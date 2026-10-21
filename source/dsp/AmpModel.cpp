#include "AmpModel.h"

#include <cmath>
#include <stdexcept>

#include "NAM/get_dsp.h"

namespace
{
    // Models that don't record their sample rate are assumed to be 48 kHz,
    // as in the official plugin.
    constexpr double defaultModelSampleRate = 48000.0;

    // The official plugin's "Normalized" output mode targets -18 dB.
    constexpr double normalisedLoudnessDb = -18.0;

    // NAM takes a std::filesystem::path, which is UTF-16 on Windows and UTF-8
    // elsewhere. Converting explicitly keeps non-English file names working.
    std::filesystem::path toPath (const juce::File& file)
    {
       #if JUCE_WINDOWS
        return std::filesystem::path (file.getFullPathName().toWideCharPointer());
       #else
        return std::filesystem::path (file.getFullPathName().toStdString());
       #endif
    }
}

AmpModel::AmpModel (const juce::File& fileToLoad) : file (fileToLoad)
{
    model = nam::get_dsp (toPath (file));

    if (model == nullptr)
        throw std::runtime_error ("The file isn't a NAM model.");
    if (model->NumInputChannels() != 1 || model->NumOutputChannels() != 1)
        throw std::runtime_error ("Only mono (1 in, 1 out) models are supported.");

    const auto expectedRate = model->GetExpectedSampleRate();
    modelSampleRate = expectedRate > 0.0 ? expectedRate : defaultModelSampleRate;

    if (model->HasLoudness())
        normalisationGain = (float) std::pow (10.0, (normalisedLoudnessDb - model->GetLoudness()) / 20.0);

    resampler = std::make_unique<Resampler> (modelSampleRate);
    runModel = [this] (float** in, float** out, int numFrames) { model->process (in, out, numFrames); };
}

void AmpModel::prepare (double hostSampleRate, int newMaxBlockSize)
{
    maxBlockSize = juce::jmax (1, newMaxBlockSize);
    output.assign ((size_t) maxBlockSize, 0.0f);

    resampling = std::abs (hostSampleRate - modelSampleRate) > 0.5;

    if (resampling)
    {
        resampler->Reset (hostSampleRate, maxBlockSize);
        // The model runs on chunks of the resampled length, which can be
        // longer than the host block when the host rate is below the model's.
        const auto modelBlockSize = (int) std::ceil (maxBlockSize * modelSampleRate / hostSampleRate) + 1;
        model->Reset (modelSampleRate, modelBlockSize);
    }
    else
    {
        model->Reset (modelSampleRate, maxBlockSize);
    }
}

int AmpModel::getLatencySamples() const noexcept
{
    return resampling ? resampler->GetLatency() : 0;
}

void AmpModel::process (float* samples, int numSamples) noexcept
{
    // Some hosts occasionally send a bigger block than they promised in
    // prepareToPlay, so work through it in chunks the model was sized for.
    for (int start = 0; start < numSamples; start += maxBlockSize)
        processChunk (samples + start, juce::jmin (maxBlockSize, numSamples - start));
}

void AmpModel::processChunk (float* samples, int numSamples) noexcept
{
    float* in[] = { samples };
    float* out[] = { output.data() };

    if (resampling)
        resampler->ProcessBlock (in, out, numSamples, runModel);
    else
        model->process (in, out, numSamples);

    std::copy (output.begin(), output.begin() + numSamples, samples);
}
