#include "Drive.h"

namespace effects
{
    namespace
    {
        // HALO's output: so OUTPUT at 0 dB matches the dry level at FUZZ noon
        // (the original's output is a low tap), as in PR7-G.
        constexpr float haloOutputScale = 1.04f;
    }

    void Drive::prepare (double sampleRate, int maxBlockSize)
    {
        obsidian.prepare (sampleRate, maxBlockSize);
        gotham.prepare (sampleRate, maxBlockSize);
        haloOversampling.initProcessing ((size_t) maxBlockSize);
        halo.prepare (sampleRate * (double) haloOversampling.getOversamplingFactor());
        haloLevel.reset (sampleRate, 0.02);
        bypass.prepare (sampleRate, maxBlockSize);

        crossover.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1 });
        appliedSplitHz = 0.0f;
        splitAmount.reset (sampleRate, 0.02);
        splitAmount.setCurrentAndTargetValue (0.0f);
        lowsGain.reset (sampleRate, 0.02);
        lows.assign ((size_t) maxBlockSize, 0.0f);
    }

    void Drive::process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept
    {
        // Switched back on after fading out, or to the other model: start
        // from silence, not from whatever the filters last held.
        if ((settings.on && bypass.isSilent()) || settings.model != lastModel)
        {
            obsidian.reset();
            gotham.reset();
            halo.reset();
            haloOversampling.reset();
            crossover.reset();
            lastModel = settings.model;
        }

        if (! juce::exactlyEqual (settings.splitHz, appliedSplitHz))
        {
            crossover.setCutoffFrequency (settings.splitHz);
            appliedSplitHz = settings.splitHz;
        }

        const auto outputDb = settings.model == Model::gotham ? settings.gotham.outputDb
                            : settings.model == Model::halo   ? settings.halo.outputDb
                                                              : settings.obsidian.outputDb;
        lowsGain.setTargetValue (juce::Decibels::decibelsToGain (settings.splitLowsDb + outputDb));

        bypass.process (monoBlock, settings.on, [&] (juce::dsp::AudioBlock<float> block)
        {
            auto* samples = block.getChannelPointer (0);
            const auto numSamples = (int) block.getNumSamples();

            // Switched on from fully off: start the crossover from silence.
            if (settings.split && splitAmount.getCurrentValue() <= 0.0f)
                crossover.reset();

            splitAmount.setTargetValue (settings.split ? 1.0f : 0.0f);
            const auto splitting = settings.split || splitAmount.isSmoothing();

            // The pedal gets the band above the split; the lows wait, clean.
            // While SPLIT fades, the pedal gets a mix of that and the full signal.
            if (splitting)
            {
                for (int i = 0; i < numSamples; ++i)
                {
                    float low = 0.0f, high = 0.0f;
                    crossover.processSample (0, samples[i], low, high);
                    const auto amount = splitAmount.getNextValue();
                    lows[(size_t) i] = low * amount;
                    samples[i] += amount * (high - samples[i]);
                }
            }

            if (settings.model == Model::gotham)
                gotham.process (block, settings.gotham);
            else if (settings.model == Model::halo)
                processHalo (block, settings.halo);
            else
                obsidian.process (block, settings.obsidian);

            if (splitting)
                for (int i = 0; i < numSamples; ++i)
                    samples[i] += lows[(size_t) i] * lowsGain.getNextValue();
            else
                lowsGain.skip (numSamples);
        });
    }

    void Drive::processHalo (juce::dsp::AudioBlock<float> block, const HaloSettings& settings) noexcept
    {
        // The whole circuit at 8x the rate: FUZZ is its 1k pot (Halo glides
        // it), OUTPUT stands in for its volume pot. The original has no tone
        // control. Volts in are the samples, as for the other models.
        halo.setFuzz (juce::jlimit (0.0f, 10.0f, settings.fuzz) / 10.0f);
        haloLevel.setTargetValue (haloOutputScale * juce::Decibels::decibelsToGain (settings.outputDb));
        auto upsampled = haloOversampling.processSamplesUp (block);
        auto* fast = upsampled.getChannelPointer (0);
        for (size_t i = 0; i < upsampled.getNumSamples(); ++i)
            fast[i] = (float) halo.process (fast[i]);
        haloOversampling.processSamplesDown (block);

        auto* samples = block.getChannelPointer (0);
        for (int i = 0; i < (int) block.getNumSamples(); ++i)
            samples[i] *= haloLevel.getNextValue();
    }
}
