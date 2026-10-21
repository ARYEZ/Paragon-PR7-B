#pragma once

#include "EffectUtils.h"

namespace effects
{
    // Footswitch 2: an overdrive pedal, before the amp.
    //
    //   low cut (tightens the bass) -> gain -> asymmetric soft clipping
    //   (8x oversampled) -> tone (high cut) -> level
    //
    // Clipping creates new high harmonics. Without oversampling, the ones
    // above half the sample rate fold back down as harsh, unmusical tones
    // (aliasing). Running the clipper at 8x the rate keeps them out: at full
    // drive a high note is close to a square wave, whose harmonics reach far
    // enough that 4x still let audible fold-back through.
    class Drive
    {
    public:
        struct Settings
        {
            bool on = false;
            float drive = 5.0f;      // 0 to 10
            float tone = 5.0f;       // 0 to 10
            float levelDb = 0.0f;
        };

        Drive();
        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept;

        // Exposed for tests: the waveshaper itself.
        static float shape (float x) noexcept;

    private:
        double sampleRate = 48000.0;
        juce::dsp::Oversampling<float> oversampling;
        juce::dsp::IIR::Filter<float> lowCut, toneFilter;
        juce::SmoothedValue<float> inputGain, outputGain;
        float lastTone = -1.0f;
        BypassFade bypass;
    };
}
