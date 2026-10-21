#include "Drive.h"

namespace effects
{
    namespace
    {
        constexpr float maxDriveDb = 40.0f;
        constexpr float lowCutHz = 120.0f;

        // Shifting the curve slightly off-centre clips the two halves of the
        // wave differently, which adds the even harmonics that make tube-style
        // overdrive sound warm rather than fizzy.
        constexpr float bias = 0.15f;
        const float biasOffset = std::tanh (bias);

        // Turning the drive up pushes the clipped signal towards full scale.
        // Taking back half the drive gain keeps the pedal at roughly the
        // guitar's own level at any drive setting, so Level 0 dB is unity
        // and the Level knob is a real boost or cut into the amp.
        constexpr float driveCompensation = 0.5f;
    }

    Drive::Drive()
        : oversampling (1, 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true)   // 3 stages = 8x, steep filters
    {
    }

    float Drive::shape (float x) noexcept
    {
        return std::tanh (x + bias) - biasOffset;
    }

    void Drive::prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 1 };

        oversampling.initProcessing ((size_t) maxBlockSize);
        oversampling.reset();

        lowCut.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (sampleRate, lowCutHz);
        toneFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 4000.0f);
        lowCut.prepare (spec);
        toneFilter.prepare (spec);
        lastTone = -1.0f;

        inputGain.reset (sampleRate, 0.02);
        outputGain.reset (sampleRate, 0.02);
        bypass.prepare (sampleRate, maxBlockSize);
    }

    void Drive::process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept
    {
        bypass.process (monoBlock, settings.on, [&] (juce::dsp::AudioBlock<float> block)
        {
            // Tone: a high cut from 1 kHz (dark) to 12 kHz (bright). Retuned
            // in place only when the knob moves: no allocation.
            if (! juce::approximatelyEqual (settings.tone, lastTone))
            {
                *toneFilter.coefficients = juce::dsp::IIR::ArrayCoefficients<float>::makeLowPass (
                    sampleRate, knobToFrequency (settings.tone, 1000.0f, 12000.0f), 0.707f);
                lastTone = settings.tone;
            }

            juce::dsp::ProcessContextReplacing<float> context (block);
            lowCut.process (context);

            const auto driveDb = maxDriveDb * juce::jlimit (0.0f, 10.0f, settings.drive) / 10.0f;
            inputGain.setTargetValue (juce::Decibels::decibelsToGain (driveDb));
            inputGain.applyGain (block.getChannelPointer (0), (int) block.getNumSamples());

            auto upsampled = oversampling.processSamplesUp (block);
            auto* samples = upsampled.getChannelPointer (0);
            for (size_t i = 0; i < upsampled.getNumSamples(); ++i)
                samples[i] = shape (samples[i]);
            oversampling.processSamplesDown (block);

            toneFilter.process (context);

            outputGain.setTargetValue (juce::Decibels::decibelsToGain (settings.levelDb - driveDb * driveCompensation));
            outputGain.applyGain (block.getChannelPointer (0), (int) block.getNumSamples());
        });
    }
}
