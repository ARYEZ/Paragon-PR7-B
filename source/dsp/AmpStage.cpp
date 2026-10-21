#include "AmpStage.h"

namespace
{
    constexpr double fadeSeconds = 0.02;
}

void AmpStage::prepare (double newSampleRate, int newMaxBlockSize)
{
    const juce::ScopedLock lock (specLock);
    sampleRate = newSampleRate;
    maxBlockSize = juce::jmax (1, newMaxBlockSize);

    dry.setSize (1, maxBlockSize);
    previousOutput.setSize (1, maxBlockSize);
    wetAmount.reset (sampleRate, fadeSeconds);
    normaliseAmount.reset (sampleRate, fadeSeconds);

    models.forEachUnsafe ([this] (AmpModel& model) { model.prepare (sampleRate, maxBlockSize); });

    latencySamples = 0;
    models.withNewestUnsafe ([this] (AmpModel& model) { latencySamples = model.getLatencySamples(); });
}

AmpStage::ModelInfo AmpStage::stage (std::unique_ptr<AmpModel> model)
{
    // Holding specLock means prepare() can't change the sample rate between
    // preparing this model and handing it to the audio thread.
    const juce::ScopedLock lock (specLock);
    ModelInfo info;

    if (model != nullptr)
    {
        model->prepare (sampleRate, maxBlockSize);
        info = { model->getModelSampleRate(), model->isResampling(), model->getNormalisationGain().has_value() };
    }

    latencySamples = model != nullptr ? model->getLatencySamples() : 0;
    models.setPending (std::move (model));
    return info;
}

float AmpStage::outputGain (const AmpModel* model, float normaliseAmount) noexcept
{
    if (model == nullptr)
        return 1.0f;
    const auto target = model->getNormalisationGain().value_or (1.0f);
    return 1.0f + (target - 1.0f) * normaliseAmount;
}

void AmpStage::process (float* samples, int numSamples, bool enabled, bool normalise) noexcept
{
    for (int start = 0; start < numSamples; start += maxBlockSize)
        processChunk (samples + start, juce::jmin (maxBlockSize, numSamples - start), enabled, normalise);
}

void AmpStage::processChunk (float* samples, int numSamples, bool enabled, bool normalise) noexcept
{
    const auto block = models.beginBlock();
    auto* current = block.current;
    auto* previous = block.previous;

    // With no model loaded the slot passes the signal through, so "enabled"
    // only means something once a model is there.
    wetAmount.setTargetValue (enabled && current != nullptr ? 1.0f : 0.0f);
    normaliseAmount.setTargetValue (normalise ? 1.0f : 0.0f);

    const auto isSwapBlock = previous != nullptr;
    const auto fullyBypassed = ! wetAmount.isSmoothing() && wetAmount.getTargetValue() < 0.5f;

    if (fullyBypassed && previous == nullptr)
    {
        // Skip the model entirely: saves CPU while the amp is off.
        normaliseAmount.skip (numSamples);
        models.endBlock();
        return;
    }

    auto* drySamples = dry.getWritePointer (0);
    std::copy (samples, samples + numSamples, drySamples);

    // New model (or the dry signal if the model was just unloaded).
    if (current != nullptr)
        current->process (samples, numSamples);

    // In the block where a model was swapped, also run the old one and
    // crossfade from it, so changing amps doesn't click.
    float* oldSamples = nullptr;
    if (isSwapBlock)
    {
        oldSamples = previousOutput.getWritePointer (0);
        std::copy (drySamples, drySamples + numSamples, oldSamples);
        previous->process (oldSamples, numSamples);
    }

    for (int i = 0; i < numSamples; ++i)
    {
        const auto normalise01 = normaliseAmount.getNextValue();
        auto wet = samples[i] * outputGain (current, normalise01);

        if (oldSamples != nullptr)
        {
            const auto t = (float) (i + 1) / (float) numSamples;
            wet = oldSamples[i] * outputGain (previous, normalise01) * (1.0f - t) + wet * t;
        }

        const auto mix = wetAmount.getNextValue();
        samples[i] = drySamples[i] + (wet - drySamples[i]) * mix;
    }

    models.endBlock();
}
