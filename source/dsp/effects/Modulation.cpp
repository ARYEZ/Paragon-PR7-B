#include "Modulation.h"

namespace effects
{
    namespace
    {
        constexpr float twoPi = juce::MathConstants<float>::twoPi;

        // Delay times in milliseconds: the centre of the sweep and how far
        // full depth swings either side of it.
        struct DelaySweep
        {
            float centreMs, sweepMs, feedback;
        };
        constexpr DelaySweep chorusSweep { 14.0f, 6.0f, 0.0f };
        constexpr DelaySweep flangerSweep { 3.0f, 2.5f, 0.6f };   // feedback gives the flanger its "jet" ring

        constexpr float maxDelayMs = 25.0f;
    }

    Modulation::Modulation() : delayLine (4096) {}

    void Modulation::prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, (juce::uint32) maxChannels };

        delayLine.setMaximumDelayInSamples ((int) std::ceil (maxDelayMs * 0.001 * sampleRate) + 4);
        delayLine.prepare (spec);
        phaser.prepare (spec);
        phaser.setCentreFrequency (900.0f);
        phaser.setFeedback (0.5f);

        depth.reset (sampleRate, 0.05);
        mix.reset (sampleRate, 0.05);
        lfoPhase = 0.0f;
        std::fill (std::begin (feedbackSample), std::end (feedbackSample), 0.0f);
        bypass.prepare (sampleRate, maxBlockSize);
    }

    float Modulation::advanceLfo (float rateHz) noexcept
    {
        lfoPhase += rateHz / (float) sampleRate;
        lfoPhase -= std::floor (lfoPhase);
        return lfoPhase;
    }

    void Modulation::process (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        bypass.process (block, settings.on, [&] (juce::dsp::AudioBlock<float> wet)
        {
            depth.setTargetValue (settings.depth);
            mix.setTargetValue (settings.mix);

            switch (settings.type)
            {
                case Type::chorus:
                case Type::flanger:
                    processDelayBased (wet, settings);
                    break;

                case Type::phaser:
                {
                    phaser.setRate (settings.rateHz);
                    phaser.setDepth (settings.depth);
                    phaser.setMix (settings.mix);
                    juce::dsp::ProcessContextReplacing<float> context (wet);
                    phaser.process (context);
                    depth.skip ((int) wet.getNumSamples());
                    mix.skip ((int) wet.getNumSamples());
                    break;
                }

                case Type::tremolo:
                    processTremolo (wet, settings);
                    break;
            }
        });
    }

    void Modulation::processDelayBased (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        const auto& sweep = settings.type == Type::flanger ? flangerSweep : chorusSweep;
        const auto msToSamples = (float) sampleRate * 0.001f;
        const auto numChannels = (int) block.getNumChannels();

        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            const auto phase = advanceLfo (settings.rateHz);
            const auto depthNow = depth.getNextValue();
            const auto mixNow = mix.getNextValue();

            for (int ch = 0; ch < numChannels; ++ch)
            {
                // Right channel's LFO runs a quarter-cycle behind the left.
                const auto lfo = std::sin (twoPi * (phase + 0.25f * (float) ch));
                const auto delayMs = sweep.centreMs + sweep.sweepMs * depthNow * lfo;

                auto* samples = block.getChannelPointer ((size_t) ch);
                const auto dry = samples[i];
                delayLine.pushSample (ch, dry + feedbackSample[ch] * sweep.feedback);
                const auto delayed = delayLine.popSample (ch, delayMs * msToSamples);
                feedbackSample[ch] = delayed;

                samples[i] = dry * (1.0f - mixNow) + delayed * mixNow;
            }
        }
    }

    void Modulation::processTremolo (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            const auto lfo = 0.5f + 0.5f * std::sin (twoPi * advanceLfo (settings.rateHz));
            const auto gain = 1.0f - depth.getNextValue() * lfo;
            mix.skip (1);

            for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
                block.getChannelPointer (ch)[i] *= gain;
        }
    }
}
