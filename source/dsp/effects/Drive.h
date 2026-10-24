#pragma once

#include "Gotham.h"
#include "Obsidian.h"
#include "EffectUtils.h"

namespace effects
{
    // Footswitch 2, OD / DISTORTION: the bass distortion, before the amp,
    // in two models, each built from a trace of the pedal's circuit:
    //   OBSIDIAN, modelled on the Darkglass Alpha·Omega (see Obsidian.h);
    //   GOTHAM, modelled on the Tech 21 SansAmp Bass Driver DI (see Gotham.h).
    // Faded in and out when switched. PR7-G's guitar drives (Classic,
    // Screamer, Muff Fuzz, Face Fuzz) were removed in PR7-B 0.4.0.
    class Drive
    {
    public:
        // Saved by number in rigs and projects: only ever add to the end.
        enum class Model { obsidian, gotham };
        static inline const juce::StringArray modelNames { "Obsidian", "Gotham" };

        struct Settings
        {
            bool on = false;
            Model model = Model::obsidian;
            Obsidian::Settings obsidian;
            Gotham::Settings gotham;
        };

        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept;

    private:
        Obsidian obsidian;
        Gotham gotham;
        Model lastModel = Model::obsidian;
        BypassFade bypass;
    };
}
