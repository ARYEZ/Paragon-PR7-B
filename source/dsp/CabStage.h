#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>

// The cabinet slot: convolves the amp's output with a speaker cabinet
// impulse response (IR).
//
// JUCE's convolution engine loads IRs on its own background thread, resamples
// them to the host rate, and crossfades from the previous IR, so switching
// cabinets doesn't click. IR level matches the official NAM plugin: no
// trimming or normalising, just a fixed -18 dB. With no IR loaded the
// convolution is skipped and audio passes through untouched.
class CabStage
{
public:
    CabStage();

    // Message thread, while audio is stopped.
    void prepare (double sampleRate, int maxBlockSize);

    struct LoadResult
    {
        juce::String error;           // empty on success
        double lengthSeconds = 0.0;
    };

    // Background thread. Reads an IR file and hands it to the convolution engine.
    LoadResult load (const juce::File& file);

    // Any thread except the audio thread: back to pass-through.
    void clear() noexcept { hasImpulseResponse = false; }

    // Audio thread. Processes in place.
    void process (float* samples, int numSamples, bool enabled) noexcept;

    static constexpr double maxIrSeconds = 10.0;

private:
    juce::AudioFormatManager formats;
    juce::dsp::Convolution convolution;   // default: zero latency
    std::atomic<bool> hasImpulseResponse { false };
    juce::SmoothedValue<float> wetAmount;
    juce::AudioBuffer<float> dry;
    int maxBlockSize = 512;
};
