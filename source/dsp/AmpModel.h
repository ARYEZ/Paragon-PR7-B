#pragma once

#include <juce_core/juce_core.h>

#include <memory>
#include <optional>
#include <vector>

#include "NAM/dsp.h"
#include "ResamplerShim.h"   // must come before the resampler
#include "ResamplingContainer/ResamplingContainer.h"

// One loaded .nam model, ready to run at the host's sample rate.
//
// NAM models only sound right at the sample rate they were captured at
// (almost always 48 kHz). When the host runs at a different rate, audio is
// resampled to the model's rate and back with the same Lanczos resampler the
// official NAM plugin uses, so a model sounds the same in both plugins.
//
// Construct and prepare() off the audio thread: both allocate. process() is
// real-time safe.
class AmpModel
{
public:
    // Throws std::runtime_error if the file can't be loaded or isn't a mono model.
    explicit AmpModel (const juce::File& file);

    void prepare (double hostSampleRate, int maxBlockSize);

    // Processes `numSamples` in place. Real-time safe.
    void process (float* samples, int numSamples) noexcept;

    const juce::File& getFile() const noexcept { return file; }
    double getModelSampleRate() const noexcept { return modelSampleRate; }
    bool isResampling() const noexcept { return resampling; }
    int getLatencySamples() const noexcept;

    // Gain (linear) that brings the model to the same loudness as the
    // official plugin's "Normalized" output mode, if the model knows its
    // loudness.
    std::optional<float> getNormalisationGain() const noexcept { return normalisationGain; }

private:
    void processChunk (float* samples, int numSamples) noexcept;

    juce::File file;
    std::unique_ptr<nam::DSP> model;
    double modelSampleRate = 48000.0;
    std::optional<float> normalisationGain;

    // NCHANS = 1 (guitar is mono); A = 12 is the official plugin's filter size.
    using Resampler = dsp::ResamplingContainer<float, 1, 12>;
    std::unique_ptr<Resampler> resampler;
    Resampler::BlockProcessFunc runModel;
    bool resampling = false;

    int maxBlockSize = 0;
    std::vector<float> output;
};
