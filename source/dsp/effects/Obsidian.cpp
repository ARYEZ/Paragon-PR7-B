#include "Obsidian.h"

namespace effects
{
    namespace
    {
        constexpr int chunkSize = 32;   // knobs that change the circuit are re-applied this often while they glide

        // Where the op-amps run out of room on the 9 V supply (about the
        // 4.5 V bias, either way). The MC33178 (input stage and EQ) swings nearly
        // to its rails; the TL072 (tone and DRIVE stages) stops about
        // 1.5 V short. Typical datasheet figures, not measurements.
        constexpr double inputRail = 3.8, tl072Rail = 3.0;
        constexpr double inputGain = -10.0;   // IC1A: 100k / 10k, inverting

        constexpr double k = 1.0e3, n = 1.0e-9, p = 1.0e-12;
        constexpr double open = 1.0e12;   // a switch's open contact

        // Pot tapers, as fractions of the track from pin 1.
        double audioTaper (double x) noexcept { return (std::pow (10.0, 2.0 * x) - 1.0) / 99.0; }
        double knob (float value) noexcept { return juce::jlimit (0.0, 1.0, (double) value / 10.0); }

        // A pot's two halves: pin 3 to the wiper, and the wiper to pin 1,
        // for a wiper `position` of the way from pin 1 to pin 3.
        void setPot (Circuit& c, const std::array<int, 2>& halves, double total, double position) noexcept
        {
            c.setResistor (halves[0], total * (1.0 - position));
            c.setResistor (halves[1], total * position);
        }
    }

    Obsidian::Obsidian()
        : oversampling (2, 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true)   // 8x, like the other drives
    {
        // The parts lists below follow the Empyrean Deluxe schematic (Aion
        // FX's trace of the Alpha·Omega); part numbers are theirs. Each
        // op-amp stage's output is low impedance, so stages are separate
        // circuits, as they are on the board.

        // IC2B, after the input stage's x10: the Growl / Bite tone stage.
        {
            auto& c = toneStage;
            const auto in = c.node(), x = c.node(), minus = c.node(), m = c.node(), out = c.node(), pp = c.node(), q = c.node(), r10 = c.node();
            growlCap = c.capacitor (in, x, 22 * n);   // C2, plus C4 (68n) with GROWL
            c.resistor (x, minus, 220 * k);           // R5
            c.capacitor (minus, m, 1 * n);            // C5
            c.resistor (minus, out, 220 * k);         // R6
            c.opAmp (0, minus, out);
            c.resistor (x, pp, 2.2 * k);              // R7
            c.resistor (out, q, 2.2 * k);             // R8
            biteShort = c.resistor (pp, m, 47 * k);   // R11: shorted by BITE
            c.resistor (m, q, 100 * k);               // R9
            biteParallel = c.resistor (m, r10, 100 * k);   // R10: switched in parallel with R9 when BITE is off
            c.resistor (r10, q, 1.0e-3);
            c.capacitor (pp, q, 10 * n);              // C6
            c.setInput (in);
            c.setOutput (out);
        }

        // IC2A: DRIVE. Gain 1 + 300k / (3k3 + the pot), from where C8 lets the bass through.
        {
            auto& c = driveStage;
            const auto in = c.node(), minus = c.node(), out = c.node(), a = c.node(), b = c.node();
            c.opAmp (in, minus, out);
            c.resistor (minus, out, 300 * k);         // R13
            c.capacitor (minus, out, 47 * p);         // C7
            c.resistor (minus, a, 3.3 * k);           // R14
            c.capacitor (a, b, 100 * n);              // C8
            drivePot = c.resistor (b, 0, 100 * k);    // DRIVE, 100k reverse-audio, as a variable resistor
            c.setInput (in);
            c.setOutput (out);
        }

        // OMEGA: bridged-T scoop (R15, R16, C9, C10), then two Sallen-Key low-passes (IC3A, IC3B).
        {
            auto& c = omega;
            const auto in = c.node(), j = c.node(), t = c.node(), s1 = c.node(), s2 = c.node(), o1 = c.node(), s3 = c.node(), s4 = c.node(), o2 = c.node();
            c.resistor (in, j, 8.06 * k);  c.resistor (j, t, 150 * k);  c.capacitor (j, 0, 22 * n);  c.capacitor (in, t, 2.2 * n);
            c.resistor (t, s1, 10 * k);  c.capacitor (s1, o1, 1 * n);  c.resistor (s1, s2, 10 * k);  c.capacitor (s2, 0, 1 * n);  c.opAmp (s2, o1, o1);
            c.resistor (o1, s3, 10 * k);  c.capacitor (s3, o2, 1 * n);  c.resistor (s3, s4, 47 * k);  c.capacitor (s4, 0, 1 * n);  c.opAmp (s4, o2, o2);
            c.setInput (in);
            c.setOutput (o2);
        }

        // ALPHA: bridged-T scoop (R21, R22, C15, C16), Sallen-Key low-passes
        // (IC4A, IC4B), then R27 into the C21 / IC5B network that dips 520 Hz.
        {
            auto& c = alpha;
            const auto in = c.node(), j = c.node(), t = c.node(), s1 = c.node(), s2 = c.node(), o1 = c.node(), s3 = c.node(), s4 = c.node(), o2 = c.node();
            const auto s9 = c.node(), g = c.node(), h = c.node(), o5 = c.node();
            c.resistor (in, j, 47 * k);  c.resistor (j, t, 47 * k);  c.capacitor (j, 0, 22 * n);  c.capacitor (in, t, 2.2 * n);
            c.resistor (t, s1, 10 * k);  c.capacitor (s1, o1, 1 * n);  c.resistor (s1, s2, 47 * k);  c.capacitor (s2, 0, 1 * n);  c.opAmp (s2, o1, o1);
            c.resistor (o1, s3, 22 * k);  c.capacitor (s3, o2, 2.2 * n);  c.resistor (s3, s4, 47 * k);  c.capacitor (s4, 0, 1 * n);  c.opAmp (s4, o2, o2);
            c.resistor (o2, s9, 10 * k);  c.capacitor (s9, g, 6.8 * n);  c.opAmp (g, o5, o5);
            c.resistor (g, h, 10 * k);  c.resistor (h, 0, 10 * k);  c.capacitor (h, o5, 120 * n);
            c.setInput (in);
            c.setOutput (s9);   // IC5A buffers this node
        }

        // After the blend: R32 into the C23 / IC7B network, a narrow 1.85 kHz notch.
        {
            auto& c = notch;
            const auto in = c.node(), n1 = c.node(), x = c.node(), y = c.node(), o = c.node();
            c.resistor (in, n1, 56 * k);  c.capacitor (n1, x, 270 * p);  c.opAmp (x, o, o);
            c.resistor (x, y, 10 * k);  c.resistor (y, 0, 10 * k);  c.capacitor (y, o, 270 * n);
            c.setInput (in);
            c.setOutput (n1);   // IC7A buffers this node
        }

        // IC6B: BASS and TREBLE, a Baxandall stage (inverting).
        {
            auto& c = bassTreble;
            const auto in = c.node(), b1 = c.node(), wb = c.node(), b2 = c.node(), out = c.node(), t1 = c.node(), wt = c.node(), t2 = c.node(), minus = c.node();
            c.resistor (in, b1, 10 * k);                                            // R37
            bassPot = { c.resistor (b1, wb, 50 * k), c.resistor (wb, b2, 50 * k) };   // BASS, 100k linear
            c.resistor (b2, out, 10 * k);                                           // R38
            c.capacitor (b1, wb, 22 * n);  c.capacitor (wb, b2, 22 * n);            // C26, C27
            c.resistor (wb, minus, 10 * k);                                         // R39
            c.capacitor (in, t1, 10 * n);                                           // C28
            treblePot = { c.resistor (t1, wt, 50 * k), c.resistor (wt, t2, 50 * k) };   // TREBLE, 100k linear
            c.capacitor (t2, out, 10 * n);                                          // C29
            c.resistor (wt, minus, 3.3 * k);                                        // R40
            c.resistor (minus, out, 1000 * k);  c.capacitor (minus, out, 33 * p);   // R41, C30
            c.opAmp (0, minus, out);
            c.setInput (in);
            c.setOutput (out);
        }

        // IC8A: MID (inverting, so the EQ as a whole isn't).
        {
            auto& c = midStage;
            const auto in = c.node(), minus = c.node(), w = c.node(), out = c.node(), pp = c.node(), q = c.node();
            c.resistor (in, minus, 220 * k);  c.capacitor (minus, w, 2.2 * n);  c.resistor (minus, out, 220 * k);   // R42, C32, R43
            c.opAmp (0, minus, out);
            c.resistor (in, pp, 2.2 * k);  c.resistor (out, q, 2.2 * k);                                         // R44, R45
            midPot = { c.resistor (pp, w, 50 * k), c.resistor (w, q, 50 * k) };                                  // MID, 100k linear
            c.capacitor (pp, q, 22 * n);                                                                         // C33
            c.setInput (in);
            c.setOutput (out);
        }
    }

    Obsidian::Mix Obsidian::mixWeights (float modKnob, float levelKnob, float blendKnob) noexcept
    {
        // MOD (100k linear) runs from ALPHA (pin 1) to OMEGA (pin 3); its
        // wiper feeds LEVEL (100k audio, pin 1 to ground), whose wiper feeds
        // the wet end of BLEND (100k linear), whose other end is the clean
        // signal. BLEND's wiper goes into a buffer, so it's a plain divider,
        // but its track also leaks a little clean signal into LEVEL's wiper.
        constexpr double pot = 100.0e3, minimum = 1.0;
        const auto m = knob (modKnob), b = knob (blendKnob), l = audioTaper (knob (levelKnob));
        const auto gAlpha = 1.0 / std::fmax (pot * m, minimum), gOmega = 1.0 / std::fmax (pot * (1.0 - m), minimum);
        const auto gHigh = 1.0 / std::fmax (pot * (1.0 - l), minimum), gLow = 1.0 / std::fmax (pot * l, minimum), gBlend = 1.0 / pot;

        // Nodal equations for MOD's wiper and LEVEL's wiper.
        const auto a11 = gAlpha + gOmega + gHigh, a22 = gHigh + gLow + gBlend;
        const auto det = a11 * a22 - gHigh * gHigh;
        const auto wetAlpha = gHigh * gAlpha / det, wetOmega = gHigh * gOmega / det, wetClean = a11 * gBlend / det;
        return { b * wetAlpha, b * wetOmega, (1.0 - b) + b * wetClean };
    }

    void Obsidian::prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        oversampling.initProcessing ((size_t) maxBlockSize);
        paths.setSize (2, maxBlockSize);

        const auto fastRate = sampleRate * (double) oversampling.getOversamplingFactor();
        toneStage.prepare (fastRate);
        driveStage.prepare (fastRate);
        for (auto* c : { &alpha, &omega, &notch, &bassTreble, &midStage })
            c->prepare (sampleRate);

        for (auto* smoothed : { &drive, &mod, &level, &blend, &bass, &mid, &treble, &output })
            smoothed->reset (sampleRate, 0.03);
        appliedDrive = appliedBass = appliedMid = appliedTreble = -1.0f;
        appliedSwitches = -1;
        reset();
    }

    void Obsidian::reset() noexcept
    {
        oversampling.reset();
        for (auto* c : { &toneStage, &driveStage, &alpha, &omega, &notch, &bassTreble, &midStage })
            c->reset();
    }

    void Obsidian::updateCircuits (const Settings& settings, bool force) noexcept
    {
        const auto switches = (settings.growl ? 1 : 0) | (settings.bite ? 2 : 0);
        if (force || switches != appliedSwitches)
        {
            toneStage.setCapacitor (growlCap, settings.growl ? (22 + 68) * n : 22 * n);
            toneStage.setResistor (biteShort, settings.bite ? 1.0e-3 : 47 * k);
            toneStage.setResistor (biteParallel, settings.bite ? open : 100 * k);
            toneStage.update();
            appliedSwitches = switches;
        }

        const auto changed = [force] (float now, float& applied)
        {
            if (! force && std::abs (now - applied) < 1.0e-3f)
                return false;
            applied = now;
            return true;
        };
        if (changed (drive.getCurrentValue(), appliedDrive))
        {
            // Reverse-audio: most of the track's resistance goes in the first turn.
            driveStage.setResistor (drivePot, 100 * k * audioTaper (1.0 - knob (appliedDrive)));
            driveStage.update();
        }
        if (changed (bass.getCurrentValue(), appliedBass) | changed (treble.getCurrentValue(), appliedTreble))
        {
            setPot (bassTreble, bassPot, 100 * k, knob (appliedBass));
            setPot (bassTreble, treblePot, 100 * k, knob (appliedTreble));
            bassTreble.update();
        }
        if (changed (mid.getCurrentValue(), appliedMid))
        {
            setPot (midStage, midPot, 100 * k, knob (appliedMid));
            midStage.update();
        }
    }

    void Obsidian::process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept
    {
        drive.setTargetValue (settings.drive);
        mod.setTargetValue (settings.mod);
        level.setTargetValue (settings.level);
        blend.setTargetValue (settings.blend);
        bass.setTargetValue (settings.bass);
        mid.setTargetValue (settings.mid);
        treble.setTargetValue (settings.treble);
        output.setTargetValue (juce::Decibels::decibelsToGain (settings.outputDb));

        auto* samples = monoBlock.getChannelPointer (0);
        const auto numSamples = (int) monoBlock.getNumSamples();
        for (int start = 0; start < numSamples; start += chunkSize)
            processChunk (samples + start, juce::jmin (chunkSize, numSamples - start), settings);
    }

    void Obsidian::processChunk (float* samples, int numSamples, const Settings& settings) noexcept
    {
        updateCircuits (settings, appliedSwitches < 0);

        // At 8x: the input stage, tone stage and DRIVE, each clipping at its
        // op-amp's rail. The clean signal rides along untouched.
        paths.copyFrom (0, 0, samples, numSamples);
        paths.copyFrom (1, 0, samples, numSamples);
        auto block = juce::dsp::AudioBlock<float> (paths).getSubBlock (0, (size_t) numSamples);
        auto fast = oversampling.processSamplesUp (block);
        auto* distorted = fast.getChannelPointer (1);
        for (size_t i = 0; i < fast.getNumSamples(); ++i)
        {
            auto v = opAmpClip (inputGain * voltsPerUnit * (double) distorted[i], inputRail);
            v = opAmpClip (toneStage.process (v), tl072Rail);
            v = opAmpClip (driveStage.process (v), tl072Rail);
            distorted[i] = (float) v;   // volts
        }
        oversampling.processSamplesDown (block);

        // At the plugin's rate: the two engines' filters, the pots, the notch and the EQ.
        const auto* clean = paths.getReadPointer (0);
        const auto* wet = paths.getReadPointer (1);
        for (int i = 0; i < numSamples; ++i)
        {
            const auto weights = mixWeights (mod.getNextValue(), level.getNextValue(), blend.getNextValue());
            const auto cleanVolts = voltsPerUnit * (double) clean[i];
            const auto mixed = weights.alpha * alpha.process (wet[i]) + weights.omega * omega.process (wet[i]) + weights.clean * cleanVolts;
            // The EQ's op-amps run on the same 9 V, so big boosts clip there too.
            const auto eq = opAmpClip (midStage.process (opAmpClip (bassTreble.process (notch.process (mixed)), inputRail)), inputRail);
            samples[i] = (float) (eq / voltsPerUnit * output.getNextValue());
            drive.skip (1);
            bass.skip (1);
            mid.skip (1);
            treble.skip (1);
        }
    }
}
