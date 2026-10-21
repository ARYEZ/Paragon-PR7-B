#pragma once

#include "EffectUtils.h"

namespace effects
{
    // Footswitch 1: noise gate then compressor, before the amp.
    // The gate comes first so the compressor doesn't bring the noise back up.
    class CompGate
    {
    public:
        struct Settings
        {
            bool on = false;
            float thresholdDb = -18.0f;
            float ratio = 4.0f;
            float levelDb = 0.0f;        // make-up gain
            float gateThresholdDb = gateOffDb;
        };

        // Gate threshold at (or below) this means the gate is off.
        static constexpr float gateOffDb = -90.0f;

        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept;

    private:
        juce::dsp::NoiseGate<float> gate;
        juce::dsp::Compressor<float> compressor;
        juce::SmoothedValue<float> makeUpGain;
        BypassFade bypass;
    };
}
