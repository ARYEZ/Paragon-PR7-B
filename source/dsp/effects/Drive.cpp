#include "Drive.h"

namespace effects
{
    void Drive::prepare (double sampleRate, int maxBlockSize)
    {
        obsidian.prepare (sampleRate, maxBlockSize);
        gotham.prepare (sampleRate, maxBlockSize);
        bypass.prepare (sampleRate, maxBlockSize);
    }

    void Drive::process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept
    {
        // Switched back on after fading out, or to the other model: start
        // from silence, not from whatever the filters last held.
        if ((settings.on && bypass.isSilent()) || settings.model != lastModel)
        {
            obsidian.reset();
            gotham.reset();
            lastModel = settings.model;
        }

        bypass.process (monoBlock, settings.on, [&] (juce::dsp::AudioBlock<float> block)
        {
            if (settings.model == Model::gotham)
                gotham.process (block, settings.gotham);
            else
                obsidian.process (block, settings.obsidian);
        });
    }
}
