#include "FilterPitch.h"

namespace effects
{
    namespace
    {
        constexpr float wahLowHz = 280.0f, wahHighHz = 3200.0f;
        constexpr int retuneEvery = 16;   // samples between wah filter updates

        // The pitch shifter's read head jumps by about 25 ms at a time (long
        // enough that the joins are infrequent, short enough to keep the
        // delay small), searching 12.5 ms either side for the best-matching
        // point (a full cycle of the lowest guitar notes), then cross-fades
        // over 8 ms.
        constexpr double jumpSeconds = 0.025, searchSeconds = 0.0125, fadeSeconds = 0.008;
        constexpr int maxRatio = 4;   // two octaves up

        float coefficientFor (double sampleRate, float seconds)
        {
            return 1.0f - std::exp (-1.0f / (seconds * (float) sampleRate));
        }
    }

    FilterPitch::FilterPitch() = default;

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

        jumpSamples = (int) std::round (jumpSeconds * sampleRate);
        searchSamples = (int) std::round (searchSeconds * sampleRate);
        fadeSamples = (int) std::round (fadeSeconds * sampleRate);
        // Room for the head to run ahead while fading at the fastest speed,
        // plus the longest jump back.
        maxDelay = minDelay + (float) (fadeSamples * maxRatio + jumpSamples + searchSamples + 8);
        history.assign ((size_t) juce::nextPowerOfTwo ((int) maxDelay + fadeSamples * maxRatio + 16), 0.0f);
        historyMask = (int) history.size() - 1;
        writeIndex = 0;
        headDelay = fadingDelay = minDelay + (float) jumpSamples * 0.5f;
        fadeRemaining = 0;
        ratio.reset (sampleRate, 0.05);
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

    float FilterPitch::readAt (float delay) const noexcept
    {
        // Cubic (Hermite) interpolation between the samples either side.
        const auto position = (float) writeIndex - delay;
        const auto whole = (int) std::floor (position);
        const auto t = position - (float) whole;
        const auto y0 = history[(size_t) ((whole - 1) & historyMask)], y1 = history[(size_t) (whole & historyMask)],
                   y2 = history[(size_t) ((whole + 1) & historyMask)], y3 = history[(size_t) ((whole + 2) & historyMask)];
        const auto c1 = 0.5f * (y2 - y0);
        const auto c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const auto c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * t + c2) * t + c1) * t + y1;
    }

    float FilterPitch::findSplice (float fromDelay, float direction, float ratioNow) const noexcept
    {
        // Where to jump to: about a jump's length away (`direction` +1 is
        // further back in time, -1 more recent), at the point where the
        // guitar's waveform best matches what the current head is about to
        // play. Both heads then play the same shape during the cross-fade,
        // so they reinforce rather than cancel, and the pitch stays clean.
        // Scores each candidate by normalised correlation, every other sample.
        // Only guitar that has already arrived can be compared: the stretch
        // just before the current head (what it has just played) and as much
        // ahead of it as has been recorded.
        const auto ahead = juce::jmin (384, juce::jmax (0, (int) fromDelay - 2), (int) ((float) fadeSamples * ratioNow) + 1);
        constexpr int behind = 256;
        const auto from = writeIndex - (int) std::round (fromDelay);
        const auto score = [&] (int candidateDelay)
        {
            const auto start = writeIndex - candidateDelay;
            float product = 0.0f, energy = 1.0e-9f;
            for (int m = -behind; m < ahead; m += 2)
            {
                const auto b = history[(size_t) ((start + m) & historyMask)];
                product += history[(size_t) ((from + m) & historyMask)] * b;
                energy += b * b;
            }
            return product / std::sqrt (energy);
        };

        const auto centre = fromDelay + direction * (float) jumpSamples;
        const auto lowest = juce::jmax ((int) minDelay + 1, (int) (centre - (float) searchSamples));
        const auto highest = juce::jmin ((int) maxDelay - 1, (int) (centre + (float) searchSamples));

        // Coarse search in steps of 2, then refine either side of the best.
        auto best = juce::jlimit (lowest, highest, (int) centre);
        auto bestScore = -1.0e30f;
        for (auto candidate = lowest; candidate <= highest; candidate += 2)
            if (const auto value = score (candidate); value > bestScore)
            {
                bestScore = value;
                best = candidate;
            }
        for (const auto candidate : { best - 1, best + 1 })
            if (candidate >= lowest && candidate <= highest)
                if (const auto value = score (candidate); value > bestScore)
                {
                    bestScore = value;
                    best = candidate;
                }

        // Keep the fraction, so the new head starts exactly in step.
        return (float) best + (fromDelay - std::floor (fromDelay));
    }

    void FilterPitch::processPitch (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        // A read head moves through the last few tens of milliseconds of
        // guitar at the new speed: faster than it was played to go up,
        // slower to go down. Moving faster, it catches up with the present
        // and has to jump back; moving slower, it falls behind and has to
        // jump forward. Each jump lands where the waveform matches (see
        // findSplice) and cross-fades from the old position, so the joins
        // are smooth.
        ratio.setTargetValue (std::exp2 (juce::jlimit (-24.0f, 24.0f, settings.semitones) / 12.0f));

        auto* samples = block.getChannelPointer (0);
        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            const auto dry = samples[i];
            history[(size_t) writeIndex] = dry;
            writeIndex = (writeIndex + 1) & historyMask;

            const auto ratioNow = ratio.getNextValue();
            const auto step = ratioNow - 1.0f;   // how much the delay shrinks each sample
            headDelay -= step;
            if (fadeRemaining > 0)
                fadingDelay -= step;

            // Jump before the head (and the old head, still sounding while
            // it fades) runs out of room.
            if (fadeRemaining == 0)
            {
                const auto runway = (float) fadeSamples * std::abs (step) + 2.0f;
                if (step > 0.0f && headDelay < minDelay + runway)
                {
                    fadingDelay = headDelay;
                    headDelay = findSplice (headDelay, 1.0f, ratioNow);
                    fadeRemaining = fadeSamples;
                }
                else if (step < 0.0f && headDelay > maxDelay - runway)
                {
                    fadingDelay = headDelay;
                    headDelay = findSplice (headDelay, -1.0f, ratioNow);
                    fadeRemaining = fadeSamples;
                }
            }
            headDelay = juce::jlimit (minDelay, maxDelay, headDelay);

            auto wet = readAt (headDelay);
            if (fadeRemaining > 0)
            {
                // Raised-cosine fade: equal gain, as the two heads are in step.
                const auto t = 1.0f - (float) fadeRemaining / (float) fadeSamples;
                const auto in = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::pi * t);
                wet = wet * in + readAt (juce::jlimit (minDelay, maxDelay, fadingDelay)) * (1.0f - in);
                --fadeRemaining;
            }

            const auto mixNow = mix.getNextValue();
            samples[i] = dry * (1.0f - mixNow) + wet * mixNow;
        }
    }
}
