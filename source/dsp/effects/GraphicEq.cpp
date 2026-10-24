#include "GraphicEq.h"

namespace effects
{
    namespace
    {
        constexpr int retuneEvery = 32;   // samples between filter updates while a fader glides

        std::array<float, 6> peak (double sampleRate, float frequency, float gainDb, float q = GraphicEq::bandQ)
        {
            return juce::dsp::IIR::ArrayCoefficients<float>::makePeakFilter (sampleRate, frequency, q,
                                                                            juce::Decibels::decibelsToGain (gainDb));
        }

        std::array<float, 6> peak (double sampleRate, const GraphicEq::Shape& shape)
        {
            return peak (sampleRate, GraphicEq::safeFrequency (shape.frequency, sampleRate), shape.gainDb, shape.q);
        }

        float magnitudeDb (const std::array<float, 6>& array, float frequency, double sampleRate)
        {
            juce::dsp::IIR::Coefficients<float> coefficients;
            coefficients = array;
            return juce::Decibels::gainToDecibels ((float) coefficients.getMagnitudeForFrequency (frequency, sampleRate), -100.0f);
        }
    }

    float GraphicEq::safeFrequency (float frequency, double sampleRate) noexcept
    {
        // At very low sample rates the top band would sit past the highest
        // frequency the audio can hold, so pull it down just below that.
        return juce::jmin (frequency, (float) sampleRate * 0.45f);
    }

    float GraphicEq::bandFrequency (int band, double sampleRate) noexcept
    {
        return safeFrequency (frequencies[(size_t) band], sampleRate);
    }

    void GraphicEq::prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        for (int band = 0; band < numBands; ++band)
        {
            auto& filter = filters[(size_t) band];
            filter.coefficients = new juce::dsp::IIR::Coefficients<float>();
            *filter.coefficients = peak (sampleRate, bandFrequency (band, sampleRate), 0.0f);
            filter.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1 });
            gains[(size_t) band].reset (sampleRate, 0.05);
            gains[(size_t) band].setCurrentAndTargetValue (0.0f);
            appliedDb[(size_t) band] = 0.0f;
        }
        level.reset (sampleRate, 0.05);
        level.setCurrentAndTargetValue (1.0f);
        bypass.prepare (sampleRate, maxBlockSize);

        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 1 };
        for (size_t i = 0; i < ultraLoFilters.size(); ++i)
        {
            ultraLoFilters[i].coefficients = new juce::dsp::IIR::Coefficients<float>();
            *ultraLoFilters[i].coefficients = peak (sampleRate, ultraLo[i]);
            ultraLoFilters[i].prepare (spec);
        }
        ultraHiFilter.coefficients = new juce::dsp::IIR::Coefficients<float>();
        *ultraHiFilter.coefficients = peak (sampleRate, ultraHi);
        ultraHiFilter.prepare (spec);
        ultraLoBypass.prepare (sampleRate, maxBlockSize);
        ultraHiBypass.prepare (sampleRate, maxBlockSize);
    }

    void GraphicEq::process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept
    {
        // The tone switches first, as on the amp, where they're part of the
        // tone controls ahead of the graphic EQ. Each fades in and out, and
        // starts clean when switched back on (not from where it left off).
        if (settings.ultraLo && ultraLoBypass.isSilent())
            for (auto& filter : ultraLoFilters)
                filter.reset();
        if (settings.ultraHi && ultraHiBypass.isSilent())
            ultraHiFilter.reset();

        ultraLoBypass.process (monoBlock, settings.ultraLo, [this] (juce::dsp::AudioBlock<float> block)
        {
            for (auto& filter : ultraLoFilters)
                filter.process (juce::dsp::ProcessContextReplacing<float> (block));
        });
        ultraHiBypass.process (monoBlock, settings.ultraHi, [this] (juce::dsp::AudioBlock<float> block)
        {
            ultraHiFilter.process (juce::dsp::ProcessContextReplacing<float> (block));
        });

        bypass.process (monoBlock, settings.on, [&] (juce::dsp::AudioBlock<float> block)
        {
            for (int band = 0; band < numBands; ++band)
                gains[(size_t) band].setTargetValue (juce::jlimit (-maxGainDb, maxGainDb, settings.gainsDb[(size_t) band]));
            level.setTargetValue (juce::Decibels::decibelsToGain (settings.levelDb));

            const auto numSamples = (int) block.getNumSamples();
            for (int start = 0; start < numSamples; start += retuneEvery)
            {
                const auto count = juce::jmin (retuneEvery, numSamples - start);
                auto part = block.getSubBlock ((size_t) start, (size_t) count);

                // Retune any band whose gain moved, in place (no allocation).
                for (int band = 0; band < numBands; ++band)
                {
                    const auto gainDb = gains[(size_t) band].skip (count);
                    if (std::abs (gainDb - appliedDb[(size_t) band]) > 0.001f)
                    {
                        *filters[(size_t) band].coefficients = peak (sampleRate, bandFrequency (band, sampleRate), gainDb);
                        appliedDb[(size_t) band] = gainDb;
                    }
                    filters[(size_t) band].process (juce::dsp::ProcessContextReplacing<float> (part));
                }

                auto* samples = part.getChannelPointer (0);
                for (int i = 0; i < count; ++i)
                    samples[i] *= level.getNextValue();
            }
        });
    }

    float GraphicEq::responseDb (const std::array<float, numBands>& gainsDb, float frequency, double sampleRate,
                                 bool ultraLoOn, bool ultraHiOn)
    {
        // The bands are in series, so their responses (in dB) add up.
        float total = 0.0f;
        for (int band = 0; band < numBands; ++band)
            if (std::abs (gainsDb[(size_t) band]) >= 0.001f)
                total += magnitudeDb (peak (sampleRate, bandFrequency (band, sampleRate), gainsDb[(size_t) band]), frequency, sampleRate);

        if (ultraLoOn)
            for (const auto& shape : ultraLo)
                total += magnitudeDb (peak (sampleRate, shape), frequency, sampleRate);
        if (ultraHiOn)
            total += magnitudeDb (peak (sampleRate, ultraHi), frequency, sampleRate);
        return total;
    }
}
