#include "Reverb.h"

namespace effects
{
    void Reverb::prepare (double sampleRate, int maxBlockSize)
    {
        reverb.prepare ({ sampleRate, (juce::uint32) maxBlockSize, (juce::uint32) maxChannels });
        send.reset (sampleRate, 0.02);
        mix.reset (sampleRate, 0.05);
        wet.setSize (maxChannels, maxBlockSize);
        tail.prepare (sampleRate, 0.5);
        running = false;
        lastSize = lastDamping = -1.0f;
    }

    void Reverb::process (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        if (settings.on && ! running)
        {
            reverb.reset();
            send.setCurrentAndTargetValue (1.0f);   // no tail yet, so nothing to click against
            mix.setCurrentAndTargetValue (settings.mix);
            tail.reset();
            running = true;
        }

        if (! running)
            return;

        // We do the dry/wet mixing ourselves (so the input can be muted for
        // trails), so the reverb itself outputs 100% wet.
        juce::dsp::Reverb::Parameters parameters;
        parameters.roomSize = juce::jlimit (0.0f, 1.0f, settings.size);
        parameters.damping = 1.0f - juce::jlimit (0.0f, 10.0f, settings.tone) / 10.0f;
        parameters.wetLevel = 1.0f;
        parameters.dryLevel = 0.0f;
        parameters.width = 1.0f;

        if (! juce::approximatelyEqual (parameters.roomSize, lastSize) || ! juce::approximatelyEqual (parameters.damping, lastDamping))
        {
            reverb.setParameters (parameters);
            lastSize = parameters.roomSize;
            lastDamping = parameters.damping;
        }

        send.setTargetValue (settings.on ? 1.0f : 0.0f);
        mix.setTargetValue (settings.mix);

        const auto numChannels = (int) block.getNumChannels();
        const auto numSamples = (int) block.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
            wet.copyFrom (ch, 0, block.getChannelPointer ((size_t) ch), numSamples);

        // The send ramp is the same for every channel, so apply it per sample.
        for (int i = 0; i < numSamples; ++i)
        {
            const auto gain = send.getNextValue();
            for (int ch = 0; ch < numChannels; ++ch)
                wet.setSample (ch, i, wet.getSample (ch, i) * gain);
        }

        juce::dsp::AudioBlock<float> wetBlock (wet.getArrayOfWritePointers(), (size_t) numChannels, (size_t) numSamples);
        reverb.process (juce::dsp::ProcessContextReplacing<float> (wetBlock));

        for (int i = 0; i < numSamples; ++i)
        {
            const auto mixNow = mix.getNextValue();
            for (int ch = 0; ch < numChannels; ++ch)
                block.getChannelPointer ((size_t) ch)[i] += wet.getSample (ch, i) * mixNow;
        }

        if (! settings.on && tail.isFinished (wetBlock))
            running = false;
    }
}
