#include "Drive.h"

namespace effects
{
    void Drive::prepare (double sampleRate, int maxBlockSize)
    {
        engine.prepare (sampleRate, maxBlockSize);
        bypass.prepare (sampleRate, maxBlockSize);
    }

    void Drive::process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept
    {
        // Switched back on after fading out: start from silence, not from
        // whatever the filters last held.
        if (settings.on && bypass.isSilent())
            engine.reset();
        bypass.process (monoBlock, settings.on, [&] (juce::dsp::AudioBlock<float> block) { engine.process (block, settings.engine); });
    }
}
