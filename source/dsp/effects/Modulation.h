#pragma once

#include "EffectUtils.h"

namespace effects
{
    // Footswitch 3: chorus, flanger, phaser or tremolo, after the cabinet.
    //
    // Chorus and flanger are built here rather than using juce::dsp::Chorus,
    // because JUCE's version moves both channels together. Running the right
    // channel's sweep a quarter-cycle behind the left is what makes a chorus
    // sound wide in stereo.
    class Modulation
    {
    public:
        enum class Type { chorus, flanger, phaser, tremolo };

        struct Settings
        {
            bool on = false;
            Type type = Type::chorus;
            float rateHz = 0.8f;
            float depth = 0.5f;   // 0 to 1
            float mix = 0.5f;     // 0 to 1 (tremolo ignores it)
        };

        Modulation();
        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;

    private:
        void processDelayBased (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;
        void processTremolo (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;
        float advanceLfo (float rateHz) noexcept;

        double sampleRate = 48000.0;
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine;
        juce::dsp::Phaser<float> phaser;
        float lfoPhase = 0.0f;   // 0 to 1
        float feedbackSample[maxChannels] {};
        juce::SmoothedValue<float> depth, mix;
        BypassFade bypass;
    };
}
