#pragma once

#include "EffectUtils.h"

namespace effects
{
    // The reverb half of footswitch 4: JUCE's stereo room reverb (based on
    // the classic Freeverb). The guitar is mono, but Freeverb's left and
    // right channels use slightly different internal delays, which turns it
    // into a wide stereo space. Switching off lets the tail ring out.
    class Reverb
    {
    public:
        struct Settings
        {
            bool on = false;
            float size = 0.5f;    // 0 to 1
            float tone = 6.0f;    // 0 (dark) to 10 (bright)
            float mix = 0.28f;    // 0 to 1
        };

        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;

    private:
        juce::dsp::Reverb reverb;
        float lastSize = -1.0f, lastDamping = -1.0f;   // -1: not set yet
        juce::SmoothedValue<float> send, mix;
        juce::AudioBuffer<float> wet;
        bool running = false;
        TailWatch tail;
    };
}
