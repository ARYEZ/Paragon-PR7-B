#include "Drive.h"

namespace effects
{
    void Drive::prepare (double sampleRate, int maxBlockSize)
    {
        obsidian.prepare (sampleRate, maxBlockSize);
        gotham.prepare (sampleRate, maxBlockSize);
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
            crossover.reset();
            lastModel = settings.model;
        }

        if (settings.splitHz != appliedSplitHz)
        {
            crossover.setCutoffFrequency (settings.splitHz);
            appliedSplitHz = settings.splitHz;
        }

        const auto outputDb = settings.model == Model::gotham ? settings.gotham.outputDb : settings.obsidian.outputDb;
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
            else
                obsidian.process (block, settings.obsidian);

            if (splitting)
                for (int i = 0; i < numSamples; ++i)
                    samples[i] += lows[(size_t) i] * lowsGain.getNextValue();
            else
                lowsGain.skip (numSamples);
        });
    }
}
