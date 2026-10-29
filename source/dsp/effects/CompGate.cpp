#include "CompGate.h"

namespace effects
{
    namespace
    {
        constexpr float sidechainCutHz = 100.0f;   // the detector ignores the bass below this
        constexpr float makeUpReferenceDb = -12.0f;   // Daybreak's make-up restores part of what it takes at this level

        struct Curve
        {
            float thresholdDb, ratio, kneeDb;
        };

        // COMP turns the threshold down (from -8 to -40 dB) and the ratio up
        // together. INTENSITY off: a gentle ratio (1.5 to 6) and a wide, soft
        // knee; on: a hard ratio (4 to 20), a narrow knee, and a threshold
        // 6 dB lower, so it grabs earlier as well as harder.
        Curve daybreakCurve (float amount, bool intensity) noexcept
        {
            amount = juce::jlimit (0.0f, 10.0f, amount);
            return { -8.0f - 3.2f * amount - (intensity ? 6.0f : 0.0f),
                     intensity ? 4.0f + 1.6f * amount : 1.5f + 0.45f * amount,
                     intensity ? 3.0f : 10.0f };
        }

        // The gain change (0 or less, in dB) for an input level, with a soft knee.
        float gainFor (float inputDb, const Curve& curve) noexcept
        {
            const auto over = inputDb - curve.thresholdDb;
            const auto slope = 1.0f / curve.ratio - 1.0f;
            if (2.0f * over < -curve.kneeDb)
                return 0.0f;
            if (2.0f * over <= curve.kneeDb)
                return slope * juce::square (over + curve.kneeDb * 0.5f) / (2.0f * curve.kneeDb);
            return slope * over;
        }

        // LUMEN's guesses (the manual gives none of these): where it starts
        // working, how softly, how fast the light cell reacts, and how much
        // a long stretch of hard compression slows its recovery.
        constexpr float lumenThresholdDb = -24.0f, lumenKneeDb = 12.0f;
        constexpr float lumenAttackSeconds = 0.01f, lumenMemorySeconds = 1.5f;
        constexpr float lumenMemoryFullDb = -12.0f;   // at this much recent reduction, recovery takes half as long again
        constexpr float lumenMemorySlowdown = 0.5f;
        // The detector: the lamp's light follows the rectified bass a little
        // smoothed, so a low B's cycles don't make the gain wobble.
        constexpr float lumenDetectAttackSeconds = 0.005f, lumenDetectReleaseSeconds = 0.03f;
        constexpr float lumenCleanLimit = 1.6f;       // the input stage is straight wire below this

        Curve lumenCurve (float compression, bool pad) noexcept
        {
            // The PAD turns the bass down 15 dB going in (and we give it
            // back after), so it takes 15 dB more to reach the threshold.
            return { lumenThresholdDb - (pad ? CompGate::padDb : 0.0f), CompGate::lumenRatio (compression), lumenKneeDb };
        }

        float coefficientFor (double sampleRate, float seconds) noexcept
        {
            return 1.0f - std::exp (-1.0f / (seconds * (float) sampleRate));
        }
    }

    float CompGate::daybreakCurveDb (float inputDb, float amount, bool intensity) noexcept
    {
        return gainFor (inputDb, daybreakCurve (amount, intensity));
    }

    float CompGate::lumenRatio (float compression) noexcept
    {
        // The manual: 1:1 at minimum to 10:1 at maximum, exponentially, so
        // noon is about 3:1.
        return std::pow (10.0f, juce::jlimit (0.0f, 10.0f, compression) / 10.0f);
    }

    float CompGate::lumenReleaseMs (float release) noexcept
    {
        // The manual: 75 ms to 600 ms. Exponential too (our reading), so
        // noon is about 210 ms.
        return 75.0f * std::pow (8.0f, juce::jlimit (0.0f, 10.0f, release) / 10.0f);
    }

    float CompGate::lumenCurveDb (float inputDb, float compression, bool pad) noexcept
    {
        return gainFor (inputDb, lumenCurve (compression, pad));
    }

    float CompGate::lumenInputStage (float sample) noexcept
    {
        const auto size = std::abs (sample);
        if (size <= lumenCleanLimit)
            return sample;
        // Same level and slope where it starts, then flattening towards the
        // headroom, never past it.
        const auto room = lumenHeadroom - lumenCleanLimit;
        return std::copysign (lumenCleanLimit + room * std::tanh ((size - lumenCleanLimit) / room), sample);
    }

    void CompGate::prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 1 };
        gate.prepare (spec);
        compressor.prepare (spec);

        // Fixed ballistics that suit guitar: the gate opens instantly and
        // closes gently so notes don't get chopped; the compressor lets the
        // pick attack through before it clamps down.
        gate.setRatio (10.0f);
        gate.setAttack (1.0f);
        gate.setRelease (120.0f);
        compressor.setAttack (8.0f);
        compressor.setRelease (150.0f);

        makeUpGain.reset (sampleRate, 0.02);
        mix.reset (sampleRate, 0.05);
        daybreakMakeUp.reset (sampleRate, 0.05);
        envelope = sidechainState = 0.0f;
        padIn.reset (sampleRate, 0.02);
        padIn.setCurrentAndTargetValue (1.0f);
        lumenLevel = lumenGainDb = lumenMemoryDb = 0.0f;
        bypass.prepare (sampleRate, maxBlockSize);
    }

    void CompGate::process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept
    {
        if (! settings.on)
            gainReductionDb.store (0.0f);

        bypass.process (monoBlock, settings.on, [&] (juce::dsp::AudioBlock<float> block)
        {
            juce::dsp::ProcessContextReplacing<float> context (block);

            if (settings.gateThresholdDb > gateOffDb)
            {
                gate.setThreshold (settings.gateThresholdDb);
                gate.process (context);
            }

            if (settings.type == Type::daybreak)
            {
                processDaybreak (block, settings);
            }
            else if (settings.type == Type::lumen)
            {
                processLumen (block, settings);
            }
            else
            {
                // The meter: how much quieter the compressor made this block.
                const auto* samples = block.getChannelPointer (0);
                const auto numSamples = (int) block.getNumSamples();
                double inPower = 0.0, outPower = 0.0;
                for (int i = 0; i < numSamples; ++i)
                    inPower += (double) samples[i] * samples[i];

                compressor.setThreshold (settings.thresholdDb);
                compressor.setRatio (settings.ratio);
                compressor.process (context);

                for (int i = 0; i < numSamples; ++i)
                    outPower += (double) samples[i] * samples[i];
                gainReductionDb.store (inPower > 1.0e-9 ? (float) juce::jmin (0.0, 10.0 * std::log10 ((outPower + 1.0e-12) / inPower)) : 0.0f);
            }

            makeUpGain.setTargetValue (juce::Decibels::decibelsToGain (settings.levelDb));
            makeUpGain.applyGain (block.getChannelPointer (0), (int) block.getNumSamples());
        });
    }

    void CompGate::processDaybreak (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        const auto curve = daybreakCurve (settings.amount, settings.intensity);

        // DETAIL is the attack: at 0 it clamps in half a millisecond and
        // flattens the pick; at 10 it waits 30 ms, so the pick snaps through.
        // INTENSITY also lets go faster, so it grabs every note.
        const auto attackSeconds = 0.0005f * std::pow (60.0f, juce::jlimit (0.0f, 10.0f, settings.detail) / 10.0f);
        const auto attack = coefficientFor (sampleRate, attackSeconds);
        const auto release = coefficientFor (sampleRate, settings.intensity ? 0.09f : 0.25f);
        const auto sidechainCut = coefficientFor (sampleRate, 1.0f / (juce::MathConstants<float>::twoPi * sidechainCutHz));

        // Automatic make-up: give back most of what it takes from a firmly
        // played note, so turning COMP up squeezes rather than just quietens.
        daybreakMakeUp.setTargetValue (juce::Decibels::decibelsToGain (-0.6f * gainFor (makeUpReferenceDb, curve)));
        mix.setTargetValue (juce::jlimit (0.0f, 1.0f, settings.mix));

        auto* samples = block.getChannelPointer (0);
        auto mostReduction = 0.0f;
        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            const auto dry = samples[i];

            // The detector hears the guitar without its lowest bass.
            sidechainState += (dry - sidechainState) * sidechainCut;
            const auto detected = std::abs (dry - sidechainState);
            envelope += (detected - envelope) * (detected > envelope ? attack : release);

            const auto reductionDb = gainFor (juce::Decibels::gainToDecibels (envelope, -120.0f), curve);
            mostReduction = juce::jmin (mostReduction, reductionDb);

            const auto compressed = dry * juce::Decibels::decibelsToGain (reductionDb) * daybreakMakeUp.getNextValue();
            const auto mixNow = mix.getNextValue();
            samples[i] = dry * (1.0f - mixNow) + compressed * mixNow;
        }
        gainReductionDb.store (mostReduction);
    }

    void CompGate::processLumen (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        const auto curve = lumenCurve (settings.compression, false);   // the PAD is applied to the signal itself
        const auto detectAttack = coefficientFor (sampleRate, lumenDetectAttackSeconds);
        const auto detectRelease = coefficientFor (sampleRate, lumenDetectReleaseSeconds);
        const auto attack = coefficientFor (sampleRate, lumenAttackSeconds);
        const auto releaseSeconds = lumenReleaseMs (settings.release) / 1000.0f;
        const auto memory = coefficientFor (sampleRate, lumenMemorySeconds);
        padIn.setTargetValue (settings.pad ? juce::Decibels::decibelsToGain (padDb) : 1.0f);

        auto* samples = block.getChannelPointer (0);
        auto mostReduction = 0.0f;
        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            // PAD, then the input stage's headroom. The detector hears the
            // input (feed-forward), rectified both ways like the pedal's.
            const auto pad = padIn.getNextValue();
            const auto in = lumenInputStage (samples[i] * pad);
            const auto detected = std::abs (in);
            lumenLevel += (detected - lumenLevel) * (detected > lumenLevel ? detectAttack : detectRelease);
            const auto targetDb = gainFor (juce::Decibels::gainToDecibels (lumenLevel, -120.0f), curve);

            // The light cell: quick to turn the bass down; RELEASE sets how
            // fast it gives the level back, slower still the harder it has
            // been working over the last second or two.
            const auto linger = 1.0f + lumenMemorySlowdown * juce::jlimit (0.0f, 1.0f, lumenMemoryDb / lumenMemoryFullDb);
            const auto release = coefficientFor (sampleRate, releaseSeconds * linger);
            lumenGainDb += (targetDb - lumenGainDb) * (targetDb < lumenGainDb ? attack : release);
            lumenMemoryDb += (lumenGainDb - lumenMemoryDb) * memory;
            mostReduction = juce::jmin (mostReduction, lumenGainDb);

            // Give the PAD's 15 dB back after the gain cell (our choice:
            // the pedal leaves that to LEVEL).
            samples[i] = in * juce::Decibels::decibelsToGain (lumenGainDb) / pad;
        }
        gainReductionDb.store (mostReduction);
    }
}
