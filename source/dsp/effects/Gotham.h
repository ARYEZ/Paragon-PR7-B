#pragma once

#include "Circuit.h"
#include "EffectUtils.h"

namespace effects
{
    // GOTHAM: a bass preamp/DI modelled on the Tech 21 SansAmp Bass Driver
    // DI (the original, Version 1), built from madbeanpedals' SludgeHammer
    // schematic, a clone of it, with the real component values. Our model
    // of that circuit, not Tech 21's design files:
    //
    //   x --+------------------------------------------------------+
    //       |                                                      | clean (BLEND)
    //       +-> scoop network -> PRESENCE -> clip -> DRIVE -> clip -> low-pass filters -+
    //                                                                                    |
    //                       BLEND <------------------------------------------------------+
    //                         |
    //                         +-> LEVEL -> x2.2 -> BASS / TREBLE -> out
    //
    // Like the pedal, it imitates an amp and cabinet: a passive network
    // scoops about 800 Hz by 16 dB before the gain, PRESENCE lifts the top
    // going into the first clipping stage (up to +27 dB at 5 kHz), DRIVE
    // adds up to 40 dB more (most of it at the top of the knob's travel: it
    // is a linear pot used as a variable resistor; we voice it 25% hotter,
    // about 50 dB at full, to match the pedal cranked), and the filters after
    // it keep the lows big and roll the top off like a speaker. The
    // distortion is the op-amps running out of room on 9 V; there are no
    // diodes. The clean signal is the buffered input, full range; BLEND
    // brings in the drive through a capacitor that trims its deepest lows.
    // BASS and TREBLE are the same Baxandall stage as on the OBSIDIAN.
    class Gotham
    {
    public:
        struct Settings
        {
            float level = 5.0f, blend = 5.0f, treble = 5.0f, bass = 5.0f, drive = 5.0f, presence = 5.0f;   // knobs, 0 to 10; EQ flat at 5
            float outputDb = 0.0f;                                                                       // our addition: a trim into the amp
        };

        // Volts at the pedal's input for a full-scale sample (as for the OBSIDIAN).
        static constexpr double voltsPerUnit = 1.0;

        Gotham();
        void prepare (double sampleRate, int maxBlockSize);
        void reset() noexcept;
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings&) noexcept;

        // LEVEL's gain (the x2.2 stage included), knob 0 to 10.
        static double levelGain (float level) noexcept;

    private:
        void processChunk (float* samples, int numSamples) noexcept;
        void updateCircuits (bool force) noexcept;

        double sampleRate = 48000.0;
        juce::dsp::Oversampling<float> oversampling;
        juce::AudioBuffer<float> paths;   // 0: clean, 1: distorted, oversampled together so they stay in step

        // Before the clipping (oversampled) and after it (at the plugin's rate).
        // BLEND is two circuits, one per source: what reaches its wiper from
        // the clean signal, and from the drive, which add up.
        Circuit presenceStage, driveStage, lowPass, blendClean, blendDrive, eq;
        int presencePot = 0, drivePot = 0;
        std::array<int, 2> blendCleanPot {}, blendDrivePot {}, bassPot {}, treblePot {};

        juce::SmoothedValue<float> level, blend, treble, bass, drive, presence, output;
        float appliedBlend = -1.0f, appliedTreble = -1.0f, appliedBass = -1.0f, appliedDrive = -1.0f, appliedPresence = -1.0f;
        double driveBoost = 1.0;   // our voicing: see driveVoicing in Gotham.cpp
        bool needsUpdate = true;
    };
}
