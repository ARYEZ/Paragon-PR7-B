#include "Wah.h"

namespace effects
{
    namespace
    {
        constexpr float heelHz = 350.0f, toeHz = 2200.0f;
        constexpr float resonance = 4.0f;    // Q: how sharp and vocal the sweep sounds
        // JUCE's band-pass has a gain of `resonance` at its centre; scale that
        // to a +3.5 dB peak, the gentle boost a real wah gives.
        constexpr float outputScale = 1.5f / resonance;
        constexpr int retuneEvery = 16;      // samples between filter updates while sweeping
    }

    float Wah::frequencyFor (float position) noexcept
    {
        // Spaced by ear (exponentially), so the sweep feels even across the pedal's travel.
        return heelHz * std::pow (toeHz / heelHz, juce::jlimit (0.0f, 1.0f, position));
    }

    void Wah::prepare (double sampleRate, int maxBlockSize)
    {
        filter.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1 });
        filter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        filter.setResonance (resonance);
        sweep.reset (sampleRate, 0.03);   // smooths a stepped MIDI pedal into a continuous sweep
        bypass.prepare (sampleRate, maxBlockSize);
    }

    void Wah::process (juce::dsp::AudioBlock<float> monoBlock, bool on, float position) noexcept
    {
        bypass.process (monoBlock, on, [&] (juce::dsp::AudioBlock<float> block)
        {
            sweep.setTargetValue (position);
            auto* samples = block.getChannelPointer (0);
            const auto numSamples = (int) block.getNumSamples();

            for (int start = 0; start < numSamples; start += retuneEvery)
            {
                const auto count = juce::jmin (retuneEvery, numSamples - start);
                filter.setCutoffFrequency (frequencyFor (sweep.getNextValue()));
                sweep.skip (count - 1);

                for (int i = start; i < start + count; ++i)
                    samples[i] = filter.processSample (0, samples[i]) * outputScale;
            }
        });
    }
}
