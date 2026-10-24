#pragma once

#include "Obsidian.h"
#include "EffectUtils.h"

namespace effects
{
    // Footswitch 2, OD / DISTORTION: the bass distortion, before the amp.
    // It's the OBSIDIAN (modelled on the Darkglass Alpha·Omega's
    // circuit, see Obsidian.h), faded in and out when switched. PR7-G's
    // guitar drives (Classic, Screamer, Muff Fuzz, Face Fuzz) were removed
    // in PR7-B 0.4.0.
    class Drive
    {
    public:
        struct Settings
        {
            bool on = false;
            Obsidian::Settings engine;
        };

        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept;

    private:
        Obsidian engine;
        BypassFade bypass;
    };
}
