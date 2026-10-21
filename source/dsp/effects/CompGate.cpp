#include "CompGate.h"

namespace effects
{
    void CompGate::prepare (double sampleRate, int maxBlockSize)
    {
        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 1 };
        gate.prepare (spec);
        compressor.prepare (spec);

        // Fixed ballistics that suit guitar: the gate opens instantly and
        // closes gently so notes don't get chopped; the compressor lets the
        // pick attack through before it clamps down.
        gate.setRatio (10.0f);
        gate.setAttack (1.0f);
        gate.setRelease (120.0f);
        compressor.setAttack (8.0f);
        compressor.setRelease (150.0f);

        makeUpGain.reset (sampleRate, 0.02);
        bypass.prepare (sampleRate, maxBlockSize);
    }

    void CompGate::process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept
    {
        bypass.process (monoBlock, settings.on, [&] (juce::dsp::AudioBlock<float> block)
        {
            juce::dsp::ProcessContextReplacing<float> context (block);

            if (settings.gateThresholdDb > gateOffDb)
            {
                gate.setThreshold (settings.gateThresholdDb);
                gate.process (context);
            }

            compressor.setThreshold (settings.thresholdDb);
            compressor.setRatio (settings.ratio);
            compressor.process (context);

            makeUpGain.setTargetValue (juce::Decibels::decibelsToGain (settings.levelDb));
            makeUpGain.applyGain (block.getChannelPointer (0), (int) block.getNumSamples());
        });
    }
}
