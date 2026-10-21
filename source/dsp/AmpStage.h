#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "AmpModel.h"
#include "RealtimeSwap.h"

// The amp slot of the signal chain: runs the current NAM model and handles
// everything around it so that nothing the user does causes a click or a
// dropout:
//  - a new model is prepared on a background thread and swapped in between
//    blocks, crossfading from the old model over one block;
//  - turning the slot on or off fades between the amp and the dry signal;
//  - output normalisation (matching the official plugin) fades in and out.
class AmpStage
{
public:
    // Message thread, while audio is stopped. Allocates.
    void prepare (double sampleRate, int maxBlockSize);

    struct ModelInfo
    {
        double modelSampleRate = 0.0;
        bool resampling = false;
        bool hasLoudness = false;
    };

    // Background thread. Prepares `model` for the current sample rate and
    // block size, then queues it for the audio thread. Pass nullptr to unload.
    ModelInfo stage (std::unique_ptr<AmpModel> model);

    // Audio thread. Processes in place.
    void process (float* samples, int numSamples, bool enabled, bool normalise) noexcept;

    // Any non-audio thread: frees the model that was swapped out.
    void collectGarbage() { models.collectGarbage(); }

    // Latency the newest model adds (non-zero only when resampling).
    int getLatencySamples() const noexcept { return latencySamples.load(); }

private:
    void processChunk (float* samples, int numSamples, bool enabled, bool normalise) noexcept;
    static float outputGain (const AmpModel* model, float normaliseAmount) noexcept;

    juce::CriticalSection specLock;   // guards sampleRate/maxBlockSize against stage()
    double sampleRate = 48000.0;
    int maxBlockSize = 512;

    RealtimeSwap<AmpModel> models;
    std::atomic<int> latencySamples { 0 };
    juce::SmoothedValue<float> wetAmount, normaliseAmount;
    juce::AudioBuffer<float> dry, previousOutput;
};
