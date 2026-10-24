#include "Gotham.h"

namespace effects
{
    namespace
    {
        constexpr int chunkSize = 32;   // knobs that change the circuit are re-applied this often while they glide

        // The TL074 / TL072 on 9 V stop about 1.5 V short of each rail
        // (typical datasheet figures, as for the OBSIDIAN).
        constexpr double tl07xRail = 3.0;
        constexpr double levelStageR20 = 10.0e3, levelStageR21 = 22.0e3;   // IC1B, inverting x2.2 (its 33 pF only acts far above hearing)

        constexpr double k = 1.0e3, n = 1.0e-9, p = 1.0e-12;

        // Our voicing, by ear against the real pedal (not from the circuit):
        // DRIVE gets 25% more gain, in dB, at every setting (40 dB at full
        // becomes about 50), as a gain step into the DRIVE stage so it clips
        // harder. Cranked, the circuit's own gain sounded tamer than the pedal.
        constexpr double driveVoicing = 1.25;
        constexpr double driveR9 = 330.0e3, driveR10 = 3.3e3;   // the DRIVE stage's gain resistors

        double knob (float value) noexcept { return juce::jlimit (0.0, 1.0, (double) value / 10.0); }

        // A pot's two halves: pin 3 to the wiper, and the wiper to pin 1,
        // for a wiper `position` of the way from pin 1 to pin 3.
        void setPot (Circuit& c, const std::array<int, 2>& halves, double total, double position) noexcept
        {
            c.setResistor (halves[0], total * (1.0 - position));
            c.setResistor (halves[1], total * position);
        }
    }

    Gotham::Gotham()
        : oversampling (2, 3, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true)   // 8x, like the OBSIDIAN
    {
        // Parts lists from madbeanpedals' SludgeHammer schematic (their part
        // numbers). Every pot is 100k linear. Each op-amp output is low
        // impedance, so stages are separate circuits, as on the board.

        // The scoop network (R3, R4, R5, C2-C5, bias R6), then IC2C: PRESENCE.
        // Gain 1 + 330k / (10n + 3k3 + the pot): more of the top as it's turned up.
        {
            auto& c = presenceStage;
            const auto in = c.node(), p1 = c.node(), l = c.node(), r = c.node(), plus = c.node(), minus = c.node(), out = c.node(), a = c.node(), b = c.node();
            c.resistor (in, p1, 100 * k);  c.capacitor (in, l, 22 * n);  c.capacitor (p1, r, 22 * n);  c.capacitor (l, r, 22 * n);   // R3, C2, C3, C4
            c.resistor (l, 0, 2.2 * k);  c.resistor (r, 0, 22 * k);                                                                // R4, R5
            c.capacitor (p1, plus, 22 * n);  c.resistor (plus, 0, 100 * k);                                                        // C5, R6
            c.opAmp (plus, minus, out);
            c.resistor (minus, out, 330 * k);  c.capacitor (minus, out, 220 * p);                                                  // R7, C6
            c.capacitor (minus, a, 10 * n);  c.resistor (a, b, 3.3 * k);                                                           // C7, R8
            presencePot = c.resistor (b, 0, 50 * k);                                                                               // PRESENCE, as a variable resistor
            c.setInput (in);
            c.setOutput (out);
        }

        // IC2D: DRIVE. Gain 1 + 330k / (3k3 + the pot), flat but for the 220 pF.
        {
            auto& c = driveStage;
            const auto in = c.node(), minus = c.node(), out = c.node(), b = c.node();
            c.opAmp (in, minus, out);
            c.resistor (minus, out, 330 * k);  c.capacitor (minus, out, 220 * p);   // R9, C8
            c.resistor (minus, b, 3.3 * k);                                         // R10
            drivePot = c.resistor (b, 0, 50 * k);                                   // DRIVE, as a variable resistor
            c.setInput (in);
            c.setOutput (out);
        }

        // After the drive: IC2A's filter (R11-R15, C9-C11), IC2B's low-pass
        // (R16, R17, C12, C13), and the R18 / C14 / C15 / R19 network into IC1A.
        {
            auto& c = lowPass;
            const auto in = c.node(), a1 = c.node(), a2 = c.node(), q = c.node(), a3 = c.node(), oa = c.node(), b1 = c.node(), b2 = c.node(), ob = c.node(), m = c.node(), out = c.node();
            c.resistor (in, a1, 10 * k);  c.resistor (a1, 0, 22 * k);  c.resistor (a1, a2, 22 * k);           // R11, R12, R13
            c.resistor (a2, q, 10 * k);  c.capacitor (q, 0, 47 * n);                                         // R14, C10
            c.resistor (a2, a3, 22 * k);  c.capacitor (a3, 0, 470 * p);  c.capacitor (a2, oa, 10 * n);        // R15, C11, C9
            c.opAmp (a3, oa, oa);
            c.resistor (oa, b1, 33 * k);  c.resistor (b1, b2, 33 * k);  c.capacitor (b2, 0, 1.2 * n);  c.capacitor (b1, ob, 2.2 * n);   // R16, R17, C13, C12
            c.opAmp (b2, ob, ob);
            c.resistor (ob, out, 22 * k);  c.capacitor (ob, m, 22 * n);  c.capacitor (out, m, 47 * n);  c.resistor (m, 0, 6.2 * k);   // R18, C14, C15, R19
            c.setInput (in);
            c.setOutput (out);   // IC1A buffers this node
        }

        // BLEND: pin 1 the clean signal, pin 3 the drive through C16 (47n),
        // the wiper into a buffer (IC1C). One circuit per source, the other at 0 V.
        {
            auto& c = blendClean;
            const auto in = c.node(), w = c.node(), p3 = c.node();
            blendCleanPot = { c.resistor (p3, w, 50 * k), c.resistor (w, in, 50 * k) };
            c.capacitor (p3, 0, 47 * n);
            c.setInput (in);
            c.setOutput (w);
        }
        {
            auto& c = blendDrive;
            const auto in = c.node(), w = c.node(), p3 = c.node();
            c.capacitor (in, p3, 47 * n);
            blendDrivePot = { c.resistor (p3, w, 50 * k), c.resistor (w, 0, 50 * k) };
            c.setInput (in);
            c.setOutput (w);
        }

        // C18 from IC1B, then IC3A: BASS and TREBLE, a Baxandall stage (inverting).
        //
        // Where LEVEL sits: the SludgeHammer schematic puts it after IC1B,
        // straight into the EQ, which would load it, so LEVEL would also tilt
        // the tone and the EQ would reach only +5 to +7 dB of boost. Bass21's
        // analysis of the original (github.com/jpcima/Bass21) puts it before
        // IC1B, so an op-amp drives the EQ and it gets the full +-12 dB that
        // Tech 21 specifies. We follow the original.
        {
            auto& c = eq;
            const auto in = c.node(), i = c.node(), b1 = c.node(), wb = c.node(), b2 = c.node(), out = c.node(), t1 = c.node(), wt = c.node(), t2 = c.node(), minus = c.node();
            c.capacitor (in, i, 100 * n);                                             // C18
            c.resistor (i, b1, 10 * k);                                               // R25
            bassPot = { c.resistor (b1, wb, 50 * k), c.resistor (wb, b2, 50 * k) };   // BASS
            c.resistor (b2, out, 10 * k);                                             // R26
            c.capacitor (b1, wb, 22 * n);  c.capacitor (wb, b2, 22 * n);              // C22, C23
            c.resistor (wb, minus, 10 * k);                                           // R24
            c.capacitor (i, t1, 10 * n);                                              // C19
            treblePot = { c.resistor (t1, wt, 50 * k), c.resistor (wt, t2, 50 * k) };   // TREBLE
            c.capacitor (t2, out, 10 * n);                                            // C20
            c.resistor (wt, minus, 3.3 * k);                                          // R23
            c.resistor (minus, out, 1000 * k);  c.capacitor (minus, out, 33 * p);     // R22, C21
            c.opAmp (0, minus, out);
            c.setInput (in);
            c.setOutput (out);
        }
    }

    double Gotham::levelGain (float levelKnob) noexcept
    {
        // LEVEL (100k linear) into IC1B's 10k input resistor, whose other
        // end is held at 0 V by the op-amp: the 10k loads the wiper, which
        // makes the linear pot feel like a volume control, and IC1B turns
        // the current through it into its x2.2 (inverted) output.
        const auto position = knob (levelKnob);
        const auto lower = 100 * k * position, upper = 100 * k * (1.0 - position);
        const auto loaded = lower * levelStageR20 / (lower + levelStageR20);
        const auto wiper = loaded / (loaded + upper);
        return -levelStageR21 / levelStageR20 * wiper;
    }

    void Gotham::prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        oversampling.initProcessing ((size_t) maxBlockSize);
        paths.setSize (2, maxBlockSize);

        const auto fastRate = sampleRate * (double) oversampling.getOversamplingFactor();
        presenceStage.prepare (fastRate);
        driveStage.prepare (fastRate);
        for (auto* c : { &lowPass, &blendClean, &blendDrive, &eq })
            c->prepare (sampleRate);

        for (auto* smoothed : { &level, &blend, &treble, &bass, &drive, &presence, &output })
            smoothed->reset (sampleRate, 0.03);
        needsUpdate = true;
        reset();
    }

    void Gotham::reset() noexcept
    {
        oversampling.reset();
        for (auto* c : { &presenceStage, &driveStage, &lowPass, &blendClean, &blendDrive, &eq })
            c->reset();
    }

    void Gotham::updateCircuits (bool force) noexcept
    {
        const auto changed = [force] (const juce::SmoothedValue<float>& smoothed, float& applied)
        {
            if (! force && std::abs (smoothed.getCurrentValue() - applied) < 1.0e-3f)
                return false;
            applied = smoothed.getCurrentValue();
            return true;
        };

        // PRESENCE and DRIVE are linear pots used as variable resistors: turning
        // up takes resistance out, so the gain climbs fastest near the top.
        if (changed (presence, appliedPresence))
        {
            presenceStage.setResistor (presencePot, 100 * k * (1.0 - knob (appliedPresence)));
            presenceStage.update();
        }
        if (changed (drive, appliedDrive))
        {
            const auto potR = 100 * k * (1.0 - knob (appliedDrive));
            driveStage.setResistor (drivePot, potR);
            driveStage.update();
            const auto circuitGain = 1.0 + driveR9 / (driveR10 + potR);
            driveBoost = std::pow (circuitGain, driveVoicing - 1.0);   // the extra 25% (in dB) of the circuit's gain
        }
        if (changed (blend, appliedBlend))
        {
            setPot (blendClean, blendCleanPot, 100 * k, knob (appliedBlend));
            setPot (blendDrive, blendDrivePot, 100 * k, knob (appliedBlend));
            blendClean.update();
            blendDrive.update();
        }
        if (changed (bass, appliedBass) | changed (treble, appliedTreble))
        {
            setPot (eq, bassPot, 100 * k, knob (appliedBass));
            setPot (eq, treblePot, 100 * k, knob (appliedTreble));
            eq.update();
        }
    }

    void Gotham::process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept
    {
        level.setTargetValue (settings.level);
        blend.setTargetValue (settings.blend);
        treble.setTargetValue (settings.treble);
        bass.setTargetValue (settings.bass);
        drive.setTargetValue (settings.drive);
        presence.setTargetValue (settings.presence);
        output.setTargetValue (juce::Decibels::decibelsToGain (settings.outputDb));

        auto* samples = monoBlock.getChannelPointer (0);
        const auto numSamples = (int) monoBlock.getNumSamples();
        for (int start = 0; start < numSamples; start += chunkSize)
            processChunk (samples + start, juce::jmin (chunkSize, numSamples - start));
    }

    void Gotham::processChunk (float* samples, int numSamples) noexcept
    {
        updateCircuits (needsUpdate);
        needsUpdate = false;

        // At 8x: PRESENCE and DRIVE, each clipping at its op-amp's rail. The
        // clean signal rides along untouched.
        paths.copyFrom (0, 0, samples, numSamples);
        paths.copyFrom (1, 0, samples, numSamples);
        auto block = juce::dsp::AudioBlock<float> (paths).getSubBlock (0, (size_t) numSamples);
        auto fast = oversampling.processSamplesUp (block);
        auto* distorted = fast.getChannelPointer (1);
        for (size_t i = 0; i < fast.getNumSamples(); ++i)
        {
            auto v = opAmpClip (presenceStage.process (voltsPerUnit * (double) distorted[i]), tl07xRail);
            v = opAmpClip (driveStage.process (v * driveBoost), tl07xRail);
            distorted[i] = (float) v;   // volts
        }
        oversampling.processSamplesDown (block);

        // At the plugin's rate: the filters after the drive, BLEND, the x2.2
        // stage, LEVEL and the EQ (whose op-amps clip too if pushed).
        const auto* clean = paths.getReadPointer (0);
        const auto* wet = paths.getReadPointer (1);
        for (int i = 0; i < numSamples; ++i)
        {
            const auto mixed = blendClean.process (voltsPerUnit * (double) clean[i]) + blendDrive.process (lowPass.process (wet[i]));
            const auto boosted = opAmpClip (levelGain (level.getCurrentValue()) * mixed, tl07xRail);
            const auto out = opAmpClip (eq.process (boosted), tl07xRail);
            samples[i] = (float) (out / voltsPerUnit * output.getNextValue());
            for (auto* smoothed : { &level, &blend, &treble, &bass, &drive, &presence })
                smoothed->skip (1);
        }
    }
}
