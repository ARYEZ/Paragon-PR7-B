#include "CabStage.h"

namespace
{
    constexpr double fadeSeconds = 0.02;

    // The official NAM plugin turns every IR down by 18 dB so a typical cab
    // IR lands at a sensible level:
    // https://github.com/sdatkinson/NeuralAmpModelerPlugin/issues/100#issuecomment-1455273839
    constexpr float irGainDb = -18.0f;
}

CabStage::CabStage()
{
    formats.registerBasicFormats();   // WAV, AIFF, FLAC and others
}

void CabStage::prepare (double sampleRate, int newMaxBlockSize)
{
    maxBlockSize = juce::jmax (1, newMaxBlockSize);
    convolution.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1 });
    wetAmount.reset (sampleRate, fadeSeconds);
    dry.setSize (1, maxBlockSize);
}

CabStage::LoadResult CabStage::load (const juce::File& file)
{
    if (! file.existsAsFile())
        return { "File not found." };

    std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));
    if (reader == nullptr)
        return { "Not an audio file this plugin can read (use WAV, AIFF or FLAC)." };

    if (reader->lengthInSamples <= 0)
        return { "The file is empty." };
    if ((double) reader->lengthInSamples > maxIrSeconds * reader->sampleRate)
        return { "That's too long for a cabinet IR (over 10 seconds)." };

    // Cabinet IRs are mono. For a stereo file, use the left channel.
    juce::AudioBuffer<float> ir (1, (int) reader->lengthInSamples);
    reader->read (&ir, 0, ir.getNumSamples(), 0, true, false);
    ir.applyGain (juce::Decibels::decibelsToGain (irGainDb));

    // Normalise::no also makes JUCE scale the IR by (IR rate / host rate),
    // which keeps the level the same at any host sample rate.
    convolution.loadImpulseResponse (std::move (ir), reader->sampleRate,
                                     juce::dsp::Convolution::Stereo::no,
                                     juce::dsp::Convolution::Trim::no,
                                     juce::dsp::Convolution::Normalise::no);
    hasImpulseResponse = true;
    return { {}, (double) reader->lengthInSamples / reader->sampleRate };
}

void CabStage::process (float* samples, int numSamples, bool enabled) noexcept
{
    wetAmount.setTargetValue (enabled && hasImpulseResponse ? 1.0f : 0.0f);

    if (! wetAmount.isSmoothing() && wetAmount.getTargetValue() < 0.5f)
    {
        // Off, or no IR: skip the convolution and leave the audio untouched.
        // A newly loaded IR still gets installed: JUCE does that inside
        // process(), so it happens as soon as the cab is switched back on.
        return;
    }

    for (int start = 0; start < numSamples; start += maxBlockSize)
    {
        const auto count = juce::jmin (maxBlockSize, numSamples - start);
        auto* chunk = samples + start;
        auto* drySamples = dry.getWritePointer (0);
        std::copy (chunk, chunk + count, drySamples);

        float* channels[] = { chunk };
        juce::dsp::AudioBlock<float> block (channels, 1, (size_t) count);
        convolution.process (juce::dsp::ProcessContextReplacing<float> (block));

        for (int i = 0; i < count; ++i)
        {
            const auto mix = wetAmount.getNextValue();
            chunk[i] = drySamples[i] + (chunk[i] - drySamples[i]) * mix;
        }
    }
}
