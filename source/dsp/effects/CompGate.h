#pragma once

#include "EffectUtils.h"

namespace effects
{
    // Footswitch 1: noise gate then compressor, before the amp.
    // The gate comes first so the compressor doesn't bring the noise back up.
    // Two compressors:
    //
    //   classic   threshold and ratio, as studio compressors have
    //   daybreak  a modern pedal compressor, laid out like the boutique
    //             ones metal players use: COMP (how much, with automatic
    //             make-up gain), DETAIL (how much pick attack gets through
    //             before it clamps), MIX (parallel compression: the
    //             squashed sound blended with the dry guitar) and INTENSITY
    //             (off: smooth and gentle; on: fast and hard). Its detector
    //             ignores the lowest bass, so low palm mutes don't make it pump.
    //   lumen     an optical bass compressor modelled on the Ampeg Opto Comp,
    //             from its manual: COMPRESSION (ratio 1:1 to 10:1, set
    //             exponentially), RELEASE (75 ms to 600 ms), LEVEL and a
    //             -15 dB PAD. Feed-forward: it hears the bass before the gain
    //             cell, full-wave, and its input stage rounds off peaks near
    //             the pedal's 2.1 V headroom. The manual doesn't give the
    //             threshold, knee, attack or how the light cell lingers, so
    //             those are our guesses at an opto's feel (see CompGate.cpp).
    class CompGate
    {
    public:
        // Saved by number in rigs and projects: only ever add to the end.
        enum class Type { classic, daybreak, lumen };
        static inline const juce::StringArray typeNames { "Classic", "Daybreak", "Lumen" };

        struct Settings
        {
            bool on = false;
            float thresholdDb = -18.0f;  // classic
            float ratio = 4.0f;          // classic
            float levelDb = 0.0f;        // make-up gain (both)
            float gateThresholdDb = gateOffDb;
            Type type = Type::classic;
            float amount = 5.0f;         // daybreak: COMP, 0 to 10
            float detail = 5.0f;         // daybreak: DETAIL, 0 to 10
            float mix = 1.0f;            // daybreak: MIX, 0 to 1
            bool intensity = false;      // daybreak: INTENSITY
            float compression = 5.0f;    // lumen: COMPRESSION, 0 to 10
            float release = 5.0f;        // lumen: RELEASE, 0 to 10
            bool pad = false;            // lumen: -15 dB PAD
        };

        // Gate threshold at (or below) this means the gate is off.
        static constexpr float gateOffDb = -90.0f;

        void prepare (double sampleRate, int maxBlockSize);
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept;

        // How far the compressor turned the guitar down in the last block, in
        // dB (0 or less), for the panel's meter. 0 while it's off.
        float getGainReductionDb() const noexcept { return gainReductionDb.load(); }

        // Daybreak's gain at a steady input level, in dB, before its make-up
        // gain: exposed for tests, and the shape of its compression curve.
        static float daybreakCurveDb (float inputDb, float amount, bool intensity) noexcept;

        // LUMEN's knobs as the manual gives them, for tests and the panel.
        static float lumenRatio (float compression) noexcept;       // 1 to 10
        static float lumenReleaseMs (float release) noexcept;       // 75 to 600
        // LUMEN's gain at a steady input level, in dB (0 or less).
        static float lumenCurveDb (float inputDb, float compression, bool pad) noexcept;
        // LUMEN's input stage: clean up to 1.6, then rounding off towards
        // 2.1 (full scale is 1 V at the pedal, as for the drives).
        static float lumenInputStage (float sample) noexcept;
        static constexpr float lumenHeadroom = 2.1f, padDb = -15.0f;

    private:
        void processDaybreak (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;

        double sampleRate = 48000.0;
        juce::dsp::NoiseGate<float> gate;
        juce::dsp::Compressor<float> compressor;
        juce::SmoothedValue<float> makeUpGain;
        BypassFade bypass;
        std::atomic<float> gainReductionDb { 0.0f };

        // Daybreak
        float envelope = 0.0f;               // the detector's level
        float sidechainState = 0.0f;         // its bass cut
        juce::SmoothedValue<float> mix, daybreakMakeUp;

        // Lumen
        void processLumen (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept;
        float lumenLevel = 0.0f;             // the detector (the lamp's light)
        float lumenGainDb = 0.0f;            // the light cell's gain change (dB, 0 or less)
        float lumenMemoryDb = 0.0f;          // how hard it has worked lately (dB, 0 or less)
        juce::SmoothedValue<float> padIn;
    };
}
