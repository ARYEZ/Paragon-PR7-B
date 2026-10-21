#pragma once

#include "EffectUtils.h"

namespace effects
{
    // The PRESENCE knob on the mockup's pedal face: a treble shelf after the
    // cabinet, like the presence control on a real amp's power section.
    // Cuts or boosts everything above about 3.5 kHz; 0 dB is no change.
    class Presence
    {
    public:
        void prepare (double newSampleRate, int maxBlockSize)
        {
            sampleRate = newSampleRate;
            filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, shelfHz, 0.707f, 1.0f);
            filter.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1 });
            gainDb.reset (sampleRate, 0.05);
            gainDb.setCurrentAndTargetValue (0.0f);
            lastGainDb = 0.0f;
        }

        void process (juce::dsp::AudioBlock<float> monoBlock, float presenceDb) noexcept
        {
            gainDb.setTargetValue (presenceDb);
            if (! gainDb.isSmoothing() && std::abs (gainDb.getCurrentValue()) < 0.01f)
                return;   // flat: nothing to do

            // Retune once per block, in place (no allocation). Turning the knob
            // glides over 50 ms, which is smooth enough not to zip.
            const auto current = gainDb.skip ((int) monoBlock.getNumSamples());
            if (! juce::approximatelyEqual (current, lastGainDb))
            {
                *filter.coefficients = juce::dsp::IIR::ArrayCoefficients<float>::makeHighShelf (
                    sampleRate, shelfHz, 0.707f, juce::Decibels::decibelsToGain (current));
                lastGainDb = current;
            }

            filter.process (juce::dsp::ProcessContextReplacing<float> (monoBlock));
        }

    private:
        static constexpr float shelfHz = 3500.0f;
        double sampleRate = 48000.0;
        juce::dsp::IIR::Filter<float> filter;
        juce::SmoothedValue<float> gainDb;
        float lastGainDb = 0.0f;
    };
}
