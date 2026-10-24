#pragma once

#include "Circuit.h"
#include "EffectUtils.h"

namespace effects
{
    // OBSIDIAN: a bass preamp/distortion modelled on the Darkglass
    // Alpha·Omega, built from Aion FX's trace of its circuit (their Empyrean
    // Deluxe documentation and Alpha Omicron schematic), with the real
    // component values. Our model of that circuit, not Darkglass's code:
    //
    //   x --+-----------------------------------------------------------+
    //       |                                                           | clean (BLEND)
    //       +-> x10 -> clip -> GROWL / BITE tone stage -> clip -> DRIVE -> clip
    //                                                                   |
    //               +-------------- ALPHA filters <---------------------+
    //               |  +----------- OMEGA filters <---------------------+
    //               v  v
    //               MOD -> LEVEL -> BLEND -> 1.85 kHz notch -> BASS / TREBLE -> MID -> out
    //
    // The distortion is op-amps driven past their 9 V supply, nothing else
    // (no diodes or transistors): the input stage's fixed x10, the tone
    // stage, and the DRIVE stage (up to x91 more). ALPHA and OMEGA are the
    // same distortion through two different sets of filters: ALPHA ("tight,
    // refined") scoops 520 Hz by 20 dB, peaks at 1.9 kHz and rolls the top
    // off steeply; OMEGA ("raw") scoops 780 Hz by 6 dB and keeps more top.
    // MOD is a pot between the two, so the middle is a mix of both. The
    // clean signal is the buffered input, full range. The notch and the EQ
    // come after the blend, so they shape the clean signal too.
    //
    // GROWL (the "Low" switch) adds a capacitor before the distortion, so
    // much more bass is distorted (+7 dB at 80 Hz into the clipping). BITE
    // ("Hi-Mid") lifts about 3 kHz by 9 dB before the distortion. The MID
    // knob, per the traced values, centres near 1 kHz (Darkglass's spec
    // says 500 Hz). The EQ's op-amps clip too, if big boosts push them to
    // their rails. What isn't modelled: the op-amps' speed limits and exact
    // clipping curve (a smooth limit at each stage's rail is used).
    class Obsidian
    {
    public:
        struct Settings
        {
            float drive = 5.0f, mod = 5.0f, level = 5.0f, blend = 5.0f;   // knobs, 0 to 10
            float bass = 5.0f, mid = 5.0f, treble = 5.0f;                  // 5 is flat
            bool growl = false, bite = false;
            float outputDb = 0.0f;                                         // our addition: a trim into the amp
        };

        // Volts at the pedal's input for a full-scale sample. Real bass
        // pickups peak at roughly 0.2 to 1 V; this is our choice, and the
        // plugin's INPUT knob moves where the distortion starts.
        static constexpr double voltsPerUnit = 1.0;

        Obsidian();
        void prepare (double sampleRate, int maxBlockSize);
        void reset() noexcept;
        void process (juce::dsp::AudioBlock<float> monoBlock, const Settings&) noexcept;

        // An op-amp output pushed towards its supply: straight through, then
        // levelling off smoothly at `rail` volts either way.
        static double opAmpClip (double volts, double rail) noexcept;

        // How the MOD, LEVEL and BLEND pots mix the two engines and the clean
        // signal (their loading on each other included): out = alpha * a +
        // omega * o + clean * c. Knobs 0 to 10.
        struct Mix { double alpha, omega, clean; };
        static Mix mixWeights (float mod, float level, float blend) noexcept;

    private:
        void processChunk (float* samples, int numSamples, const Settings&) noexcept;
        void updateCircuits (const Settings&, bool force) noexcept;

        double sampleRate = 48000.0;
        juce::dsp::Oversampling<float> oversampling;
        juce::AudioBuffer<float> paths;   // 0: clean, 1: distorted, oversampled together so they stay in step

        // Before the distortion (at the oversampled rate) and after (at the plugin's rate).
        Circuit toneStage, driveStage, alpha, omega, notch, bassTreble, midStage;
        int growlCap = 0, biteShort = 0, biteParallel = 0, drivePot = 0;
        std::array<int, 2> bassPot {}, treblePot {}, midPot {};

        juce::SmoothedValue<float> drive, mod, level, blend, bass, mid, treble, output;
        float appliedDrive = -1.0f, appliedBass = -1.0f, appliedMid = -1.0f, appliedTreble = -1.0f;
        int appliedSwitches = -1;
    };
}
