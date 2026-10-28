#pragma once

#include "Gotham.h"
#include "Halo.h"
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
    //
    // SPLIT (either model): a crossover sends the lows around the pedal,
    // clean, and only the rest through it, then adds them back together:
    //
    //   x --> crossover --lows (below SPLIT HZ)--> LOWS level --------+--> out
    //                   \--the rest--> OBSIDIAN / GOTHAM --------------/
    //
    // This is the multiband trick Nolly (Periphery) describes for his bass:
    // a FabFilter Saturn split at 275 Hz, the band below left alone, so the
    // low end stays tight and solid however hard the drive is pushed. The
    // crossover is a 4th-order Linkwitz-Riley, whose bands add back up
    // flat. The pedal can still make some content below the split from the
    // band above (as Saturn's band 2 can); what it never gets is the lows.
    // OUTPUT trims the clean lows too, so it stays a trim for the whole block.
    class Drive
    {
    public:
        // Saved by number in rigs and projects: only ever add to the end.
        enum class Model { obsidian, gotham, halo };
        static inline const juce::StringArray modelNames { "Obsidian", "Gotham", "Halo" };

        // HALO's knobs, as on the original: FUZZ (0 to 10) and its volume
        // (our OUTPUT trim, shared with the other models).
        struct HaloSettings
        {
            float fuzz = 5.0f;
            float outputDb = 0.0f;
        };

        struct Settings
        {
            bool on = false;
            Model model = Model::obsidian;
            bool split = false;
            float splitHz = 275.0f;      // Nolly's split
            float splitLowsDb = 0.0f;    // the clean lows' level
            Obsidian::Settings obsidian;
            Gotham::Settings gotham;
            HaloSettings halo;
        };

        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept;

    private:
        Obsidian obsidian;
        Gotham gotham;
        Model lastModel = Model::obsidian;
        BypassFade bypass;

        // HALO, solved at 8x the rate (against aliasing), like the other models.
        void processHalo (juce::dsp::AudioBlock<float> block, const HaloSettings&) noexcept;
        Halo halo;
        juce::dsp::Oversampling<float> haloOversampling { 1, 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true };
        juce::SmoothedValue<float> haloLevel;

        juce::dsp::LinkwitzRileyFilter<float> crossover;
        juce::SmoothedValue<float> splitAmount, lowsGain;   // splitAmount fades SPLIT in and out
        std::vector<float> lows;
        float appliedSplitHz = 0.0f;
    };
}
