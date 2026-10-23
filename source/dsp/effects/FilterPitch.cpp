#include "FilterPitch.h"

namespace effects
{
    namespace
    {
        constexpr float wahLowHz = 280.0f, wahHighHz = 3200.0f;
        constexpr int retuneEvery = 16;   // samples between wah filter updates

        float coefficientFor (double sampleRate, float seconds)
        {
            return 1.0f - std::exp (-1.0f / (seconds * (float) sampleRate));
        }
    }

    FilterPitch::FilterPitch() = default;

    juce::StringArray FilterPitch::whammyWahModeNames()
    {
        juce::StringArray names;
        for (const auto& mode : whammyWahModes)
            names.add (mode.name);
        return names;
    }

    float FilterPitch::wahFrequencyFor (float position) noexcept
    {
        return wahLowHz * std::pow (wahHighHz / wahLowHz, juce::jlimit (0.0f, 1.0f, position));
    }

    void FilterPitch::prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 1 };

        wahFilter.prepare (spec);
        wahFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        envelope = sweep = 0.0f;

        shifter.prepare (sampleRate);
        ratio.reset (sampleRate, 0.05);
        pedal.reset (sampleRate, 0.03);
        mix.reset (sampleRate, 0.05);
        bypass.prepare (sampleRate, maxBlockSize);
    }

    void FilterPitch::process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept
    {
        bypass.process (monoBlock, settings.on, [&] (juce::dsp::AudioBlock<float> block)
        {
            mix.setTargetValue (settings.mix);
            if (settings.type == Type::pitchShifter)
                processPitch (block, settings);
            else if (settings.type == Type::whammyWah)
                processWhammyWah (block, settings);
            else
                processAutoWah (block, settings);
        });
    }

    void FilterPitch::processAutoWah (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        // The envelope follows the guitar's level: rising in a few
        // milliseconds as you pick, falling over a fifth of a second as the
        // note fades, so each note "blooms" open and then closes. SENSITIVITY
        // is how much level it takes to open the filter fully.
        const auto attack = coefficientFor (sampleRate, 0.004f), release = coefficientFor (sampleRate, 0.2f);
        const auto sweepSmoothing = coefficientFor (sampleRate, 0.01f);
        const auto sensitivityGain = std::pow (10.0f, (juce::jlimit (0.0f, 10.0f, settings.sensitivity) - 2.0f) * 0.3f);

        // Q from 2 (smooth) to 10 (sharp, vocal). JUCE's band-pass peaks at
        // a gain of Q; scale it to a gentle boost so turning up RESONANCE
        // sharpens the sound without making it much louder.
        const auto q = 2.0f + 0.8f * juce::jlimit (0.0f, 10.0f, settings.resonance);
        wahFilter.setResonance (q);
        const auto outputScale = 2.0f / q;

        auto* samples = block.getChannelPointer (0);
        const auto numSamples = (int) block.getNumSamples();
        for (int start = 0; start < numSamples; start += retuneEvery)
        {
            const auto count = juce::jmin (retuneEvery, numSamples - start);
            for (int i = start; i < start + count; ++i)
            {
                const auto level = std::abs (samples[i]);
                envelope += (level - envelope) * (level > envelope ? attack : release);
            }
            // A square-root curve, so quieter playing still moves the filter.
            const auto target = std::sqrt (juce::jlimit (0.0f, 1.0f, envelope * sensitivityGain));
            sweep += (target - sweep) * (1.0f - std::pow (1.0f - sweepSmoothing, (float) count));
            wahFilter.setCutoffFrequency (wahFrequencyFor (sweep));

            for (int i = start; i < start + count; ++i)
            {
                const auto mixNow = mix.getNextValue();
                const auto wet = wahFilter.processSample (0, samples[i]) * outputScale;
                samples[i] = samples[i] * (1.0f - mixNow) + wet * mixNow;
            }
        }
    }

    void FilterPitch::processPitch (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        // See PitchShifter for how the shifting works.
        ratio.setTargetValue (std::exp2 (juce::jlimit (-24.0f, 24.0f, settings.semitones) / 12.0f));

        auto* samples = block.getChannelPointer (0);
        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            const auto dry = samples[i];
            const auto wet = shifter.processSample (dry, ratio.getNextValue());
            const auto mixNow = mix.getNextValue();
            samples[i] = dry * (1.0f - mixNow) + wet * mixNow;
        }
    }

    void FilterPitch::processWhammyWah (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        const auto& mode = whammyWahModes[(size_t) juce::jlimit (0, (int) whammyWahModes.size() - 1, settings.whammyWahMode)];
        pedal.setTargetValue (juce::jlimit (0.0f, 1.0f, settings.pedal));
        auto* samples = block.getChannelPointer (0);
        const auto numSamples = (int) block.getNumSamples();

        if (mode.kind == WhammyWahMode::Kind::wah)
        {
            // A resonant band-pass swept between the voicing's heel and toe
            // frequencies (spaced by ear), peaking a few dB above the dry
            // level as a real wah does.
            wahFilter.setResonance (mode.q);
            const auto outputScale = 1.5f / mode.q;
            for (int start = 0; start < numSamples; start += retuneEvery)
            {
                const auto count = juce::jmin (retuneEvery, numSamples - start);
                wahFilter.setCutoffFrequency (mode.heel * std::pow (mode.toe / mode.heel, pedal.getNextValue()));
                pedal.skip (count - 1);
                for (int i = start; i < start + count; ++i)
                {
                    const auto mixNow = mix.getNextValue();
                    samples[i] = samples[i] * (1.0f - mixNow) + wahFilter.processSample (0, samples[i]) * outputScale * mixNow;
                }
            }
            return;
        }

        // Whammy: the pedal bends the shift from none to the mode's interval,
        // the shifted guitar replacing the dry one. Harmony: the dry guitar
        // stays, and a second voice moves between the two intervals; the
        // pair is scaled so it's no louder than the guitar alone.
        const auto harmony = mode.kind == WhammyWahMode::Kind::harmony;
        for (int i = 0; i < numSamples; ++i)
        {
            const auto position = std::pow (pedal.getNextValue(), mode.curve);
            const auto semitones = juce::jlimit (-24.0f, 24.0f, mode.heel + (mode.toe - mode.heel) * position);
            const auto dry = samples[i];
            const auto wet = shifter.processSample (dry, std::exp2 (semitones / 12.0f));
            const auto mixNow = mix.getNextValue();
            samples[i] = harmony ? (dry + wet * mixNow) / std::sqrt (1.0f + mixNow * mixNow)
                                 : dry * (1.0f - mixNow) + wet * mixNow;
        }
    }
}
