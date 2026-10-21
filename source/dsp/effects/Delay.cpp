#include "Delay.h"

namespace effects
{
    void Delay::prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        delayLine.setMaximumDelayInSamples ((int) std::ceil (maxDelaySeconds * sampleRate) + 4);
        delayLine.prepare ({ sampleRate, (juce::uint32) maxBlockSize, (juce::uint32) maxChannels });

        // Time changes glide over 100 ms (a short pitch bend, like turning
        // the time knob on a tape delay) instead of jumping with a click.
        delaySamples.reset (sampleRate, 0.1);
        send.reset (sampleRate, 0.02);
        feedback.reset (sampleRate, 0.05);
        mix.reset (sampleRate, 0.05);

        std::fill (std::begin (feedbackFilter), std::end (feedbackFilter), 0.0f);
        tail.prepare (sampleRate, maxDelaySeconds + 0.5);
        wet.setSize (maxChannels, maxBlockSize);
        running = false;
    }

    float Delay::delaySeconds (const Settings& settings, double bpm) noexcept
    {
        const auto index = juce::jlimit (0, (int) std::size (syncBeats) - 1, settings.syncIndex);
        const auto seconds = index == 0 || bpm <= 0.0
                               ? settings.timeMs * 0.001f
                               : syncBeats[index] * 60.0f / (float) bpm;
        return juce::jlimit (0.001f, maxDelaySeconds, seconds);
    }

    void Delay::process (juce::dsp::AudioBlock<float> block, const Settings& settings, double bpm) noexcept
    {
        if (settings.on && ! running)
        {
            // Starting from silence: jump straight to the right time and
            // levels rather than gliding up from wherever they were. The send
            // opens fully at once: there are no echoes yet to click against,
            // and fading it in would swallow a note played on the switch.
            delayLine.reset();
            std::fill (std::begin (feedbackFilter), std::end (feedbackFilter), 0.0f);
            delaySamples.setCurrentAndTargetValue (delaySeconds (settings, bpm) * (float) sampleRate);
            feedback.setCurrentAndTargetValue (settings.feedback);
            mix.setCurrentAndTargetValue (settings.mix);
            send.setCurrentAndTargetValue (1.0f);
            tail.reset();
            running = true;
        }

        if (! running)
            return;

        delaySamples.setTargetValue (delaySeconds (settings, bpm) * (float) sampleRate);
        send.setTargetValue (settings.on ? 1.0f : 0.0f);
        feedback.setTargetValue (juce::jlimit (0.0f, 0.95f, settings.feedback));
        mix.setTargetValue (settings.mix);

        // One-pole high cut in the feedback loop, 1.5 kHz (dark) to 16 kHz.
        const auto cutoff = knobToFrequency (settings.tone, 1500.0f, 16000.0f);
        const auto filterCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * cutoff / (float) sampleRate);

        const auto numChannels = (int) block.getNumChannels();
        const auto numSamples = (int) block.getNumSamples();
        const auto pingPong = settings.pingPong && numChannels == 2;

        for (int i = 0; i < numSamples; ++i)
        {
            const auto delayNow = delaySamples.getNextValue();
            const auto sendNow = send.getNextValue();
            const auto feedbackNow = feedback.getNextValue();
            const auto mixNow = mix.getNextValue();

            float delayed[maxChannels] {};
            for (int ch = 0; ch < numChannels; ++ch)
                delayed[ch] = delayLine.popSample (ch, delayNow);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto* samples = block.getChannelPointer ((size_t) ch);
                const auto input = samples[i];

                auto& filtered = feedbackFilter[ch];
                filtered += filterCoefficient * (delayed[ch] - filtered);

                float toLine = 0.0f;
                if (pingPong)
                {
                    // Feed the mono guitar into the left line only, and each
                    // line's repeats into the other: echoes alternate L, R, L...
                    const auto monoIn = 0.5f * (block.getChannelPointer (0)[i] + block.getChannelPointer (1)[i]);
                    const auto otherFiltered = feedbackFilter[1 - ch];
                    toLine = (ch == 0 ? monoIn * sendNow : 0.0f) + otherFiltered * feedbackNow;
                }
                else
                {
                    toLine = input * sendNow + filtered * feedbackNow;
                }

                delayLine.pushSample (ch, toLine);
                wet.setSample (ch, i, delayed[ch]);
            }

            for (int ch = 0; ch < numChannels; ++ch)
                block.getChannelPointer ((size_t) ch)[i] += delayed[ch] * mixNow;
        }

        if (! settings.on)
        {
            juce::dsp::AudioBlock<const float> wetBlock (wet.getArrayOfReadPointers(), (size_t) numChannels, (size_t) numSamples);
            if (tail.isFinished (wetBlock))
                running = false;
        }
    }
}
