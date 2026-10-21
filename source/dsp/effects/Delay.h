#pragma once

#include "EffectUtils.h"

namespace effects
{
    // The delay half of footswitch 4.
    //
    // Echo time is either free (milliseconds) or locked to the DAW's tempo.
    // A high cut inside the feedback loop makes each repeat a little darker,
    // like an analogue or tape delay. Ping-pong bounces repeats between the
    // left and right speakers. Switching off lets the echoes ring out
    // ("trails") instead of cutting them dead.
    class Delay
    {
    public:
        // Note lengths, in beats (quarter notes), for tempo sync.
        // Index 0 is "Off": use the time in milliseconds instead.
        static constexpr float syncBeats[] = { 0.0f, 4.0f, 2.0f, 1.0f, 1.5f, 2.0f / 3.0f, 0.5f, 0.75f, 1.0f / 3.0f, 0.25f };
        static inline const juce::StringArray syncNames { "Off", "1/1", "1/2", "1/4", "1/4 dotted", "1/4 triplet",
                                                          "1/8", "1/8 dotted", "1/8 triplet", "1/16" };

        static constexpr float maxDelaySeconds = 2.5f;

        struct Settings
        {
            bool on = false;
            float timeMs = 375.0f;
            int syncIndex = 3;           // 1/4
            float feedback = 0.35f;      // 0 to 0.95
            float tone = 7.0f;           // 0 to 10
            float mix = 0.25f;           // 0 to 1
            bool pingPong = false;
        };

        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> block, const Settings& settings, double bpm) noexcept;

        // The echo time these settings give at this tempo, in seconds.
        static float delaySeconds (const Settings& settings, double bpm) noexcept;

    private:
        double sampleRate = 48000.0;
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 4 };
        juce::SmoothedValue<float> delaySamples, send, feedback, mix;
        float feedbackFilter[maxChannels] {};
        bool running = false;
        TailWatch tail;
        juce::AudioBuffer<float> wet;
    };
}
