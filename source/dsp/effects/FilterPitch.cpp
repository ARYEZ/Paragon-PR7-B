#include "FilterPitch.h"

namespace effects
{
    namespace
    {
        constexpr float wahLowHz = 280.0f, wahHighHz = 3200.0f;
        constexpr int retuneEvery = 16;   // samples between wah filter updates

        float coefficientFor (double sampleRate, float seconds)
        {
            return 1.0f - std::exp (-1.0f / (seconds * (float) sampleRate));
        }
    }

    namespace
    {
        // MORPH's guesses (the manual doesn't give them): how fast the
        // envelope rises and falls, and how many dB of playing take the
        // filter from rest to fully open.
        constexpr float morphAttackSeconds = 0.003f, morphReleaseSeconds = 0.12f;
        constexpr float morphSweepSpanDb = 24.0f;
        // From the manual: SENS covers 46 dB; at full SENS a 500 Hz note at
        // -23 dBV (0.1 V peak, -20 dBFS) opens it fully.
        constexpr float morphSensRangeDb = 46.0f, morphMostSensitiveDb = -20.0f;
        // From the manual: the filter's gain at its peak, Q at minimum
        // +8 dB; at maximum +18 dB at 76 Hz rising to +26 dB at 3.2 kHz.
        constexpr float morphQMinDb = 8.0f, morphQMaxLowDb = 18.0f, morphQMaxHighDb = 26.0f;

        float morphOutputStage (float sample) noexcept
        {
            // Clean up to 80% of the pedal's maximum output, then rounding
            // off towards it, never past it.
            const auto clean = 0.8f * FilterPitch::morphMaxOutput;
            const auto size = std::abs (sample);
            if (size <= clean)
                return sample;
            const auto room = FilterPitch::morphMaxOutput - clean;
            return std::copysign (clean + room * std::tanh ((size - clean) / room), sample);
        }
    }

    FilterPitch::FilterPitch() = default;

    float FilterPitch::morphStopHz (float decay) noexcept
    {
        // Exponential (our reading), so each step of the knob moves it by
        // the same musical interval.
        return morphLowestStopHz * std::pow (morphHighestStopHz / morphLowestStopHz, juce::jlimit (0.0f, 10.0f, decay) / 10.0f);
    }

    float FilterPitch::morphFrequencyFor (float position, float decay) noexcept
    {
        const auto stop = morphStopHz (decay);
        return stop * std::pow (morphTopHz / stop, juce::jlimit (0.0f, 1.0f, position));
    }

    float FilterPitch::morphPeakDb (float q, float frequencyHz) noexcept
    {
        const auto along = juce::jlimit (0.0f, 1.0f, std::log (frequencyHz / morphLowestStopHz) / std::log (morphTopHz / morphLowestStopHz));
        const auto maximumDb = morphQMaxLowDb + (morphQMaxHighDb - morphQMaxLowDb) * along;
        return morphQMinDb + (maximumDb - morphQMinDb) * juce::jlimit (0.0f, 10.0f, q) / 10.0f;
    }

    float FilterPitch::morphDryGain (float dry) noexcept
    {
        // -inf to +6 dB (the manual), on an audio-taper curve (our guess).
        return 2.0f * juce::square (juce::jlimit (0.0f, 10.0f, dry) / 10.0f);
    }

    float FilterPitch::morphFxGain (float fx) noexcept
    {
        return juce::square (juce::jlimit (0.0f, 10.0f, fx) / 10.0f);
    }

    float FilterPitch::morphFullyOpenDb (float sens) noexcept
    {
        return morphMostSensitiveDb + morphSensRangeDb * (10.0f - juce::jlimit (0.0f, 10.0f, sens)) / 10.0f;
    }

    juce::StringArray FilterPitch::xpProgramNames()
    {
        juce::StringArray names;
        for (const auto& program : xpPrograms)
            names.add (program.name);
        return names;
    }

    float FilterPitch::wahFrequencyFor (float position) noexcept
    {
        return wahLowHz * std::pow (wahHighHz / wahLowHz, juce::jlimit (0.0f, 1.0f, position));
    }

    void FilterPitch::prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 1 };

        wahFilter.prepare (spec);
        wahFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        envelope = sweep = 0.0f;

        morphFilter.prepare (spec);
        morphFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        morphEnvelope = morphSweep = 0.0f;
        morphDry.reset (sampleRate, 0.03);
        morphFx.reset (sampleRate, 0.03);

        shifter.prepare (sampleRate);
        ratio.reset (sampleRate, 0.05);
        pedal.reset (sampleRate, 0.03);
        mix.reset (sampleRate, 0.05);
        bypass.prepare (sampleRate, maxBlockSize);
    }

    void FilterPitch::process (juce::dsp::AudioBlock<float> monoBlock, const Settings& settings) noexcept
    {
        bypass.process (monoBlock, settings.on, [&] (juce::dsp::AudioBlock<float> block)
        {
            mix.setTargetValue (settings.mix);
            if (settings.type == Type::pitchShifter)
                processPitch (block, settings);
            else if (settings.type == Type::whammyWah)
                processWhammyWah (block, settings);
            else if (settings.type == Type::morph)
                processMorph (block, settings);
            else
                processAutoWah (block, settings);
        });
    }

    void FilterPitch::processAutoWah (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        // The envelope follows the guitar's level: rising in a few
        // milliseconds as you pick, falling over a fifth of a second as the
        // note fades, so each note "blooms" open and then closes. SENSITIVITY
        // is how much level it takes to open the filter fully.
        const auto attack = coefficientFor (sampleRate, 0.004f), release = coefficientFor (sampleRate, 0.2f);
        const auto sweepSmoothing = coefficientFor (sampleRate, 0.01f);
        const auto sensitivityGain = std::pow (10.0f, (juce::jlimit (0.0f, 10.0f, settings.sensitivity) - 2.0f) * 0.3f);

        // Q from 2 (smooth) to 10 (sharp, vocal). JUCE's band-pass peaks at
        // a gain of Q; scale it to a gentle boost so turning up RESONANCE
        // sharpens the sound without making it much louder.
        const auto q = 2.0f + 0.8f * juce::jlimit (0.0f, 10.0f, settings.resonance);
        wahFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);   // the Whammy-Wah may have left it low-pass
        wahFilter.setResonance (q);
        const auto outputScale = 2.0f / q;

        auto* samples = block.getChannelPointer (0);
        const auto numSamples = (int) block.getNumSamples();
        for (int start = 0; start < numSamples; start += retuneEvery)
        {
            const auto count = juce::jmin (retuneEvery, numSamples - start);
            for (int i = start; i < start + count; ++i)
            {
                const auto level = std::abs (samples[i]);
                envelope += (level - envelope) * (level > envelope ? attack : release);
            }
            // A square-root curve, so quieter playing still moves the filter.
            const auto target = std::sqrt (juce::jlimit (0.0f, 1.0f, envelope * sensitivityGain));
            sweep += (target - sweep) * (1.0f - std::pow (1.0f - sweepSmoothing, (float) count));
            wahFilter.setCutoffFrequency (wahFrequencyFor (sweep));

            for (int i = start; i < start + count; ++i)
            {
                const auto mixNow = mix.getNextValue();
                const auto wet = wahFilter.processSample (0, samples[i]) * outputScale;
                samples[i] = samples[i] * (1.0f - mixNow) + wet * mixNow;
            }
        }
    }

    void FilterPitch::processMorph (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        // The envelope follows the bass (both halves of the wave); its level
        // in dB, against SENS, says how far the filter opens: from DECAY's
        // stop frequency at rest to 3.2 kHz, morphSweepSpanDb further up.
        const auto attack = coefficientFor (sampleRate, morphAttackSeconds), release = coefficientFor (sampleRate, morphReleaseSeconds);
        const auto sweepSmoothing = coefficientFor (sampleRate, 0.004f);
        const auto openDb = morphFullyOpenDb (settings.morphSens);
        morphDry.setTargetValue (morphDryGain (settings.morphDry));
        morphFx.setTargetValue (morphFxGain (settings.morphFx));

        auto* samples = block.getChannelPointer (0);
        const auto numSamples = (int) block.getNumSamples();
        auto frequency = morphFrequency.load();
        for (int start = 0; start < numSamples; start += retuneEvery)
        {
            const auto count = juce::jmin (retuneEvery, numSamples - start);
            for (int i = start; i < start + count; ++i)
            {
                const auto level = std::abs (samples[i]);
                morphEnvelope += (level - morphEnvelope) * (level > morphEnvelope ? attack : release);
            }
            const auto levelDb = juce::Decibels::gainToDecibels (morphEnvelope, -120.0f);
            const auto target = juce::jlimit (0.0f, 1.0f, (levelDb - (openDb - morphSweepSpanDb)) / morphSweepSpanDb);
            morphSweep += (target - morphSweep) * (1.0f - std::pow (1.0f - sweepSmoothing, (float) count));
            frequency = morphFrequencyFor (morphSweep, settings.morphDecay);
            morphFilter.setCutoffFrequency (juce::jmin (frequency, 0.45f * (float) sampleRate));
            // JUCE's band-pass peaks at a gain of its resonance (Q).
            morphFilter.setResonance (juce::Decibels::decibelsToGain (morphPeakDb (settings.morphQ, frequency)));

            for (int i = start; i < start + count; ++i)
            {
                const auto dry = samples[i];
                const auto wet = morphFilter.processSample (0, dry);
                samples[i] = morphOutputStage (dry * morphDry.getNextValue() + wet * morphFx.getNextValue());
            }
        }
        morphFrequency.store (frequency);
    }

    void FilterPitch::processPitch (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        // See PitchShifter for how the shifting works.
        ratio.setTargetValue (std::exp2 (juce::jlimit (-24.0f, 24.0f, settings.semitones) / 12.0f));

        auto* samples = block.getChannelPointer (0);
        for (size_t i = 0; i < block.getNumSamples(); ++i)
        {
            const auto dry = samples[i];
            const auto wet = shifter.processSample (dry, ratio.getNextValue());
            const auto mixNow = mix.getNextValue();
            samples[i] = dry * (1.0f - mixNow) + wet * mixNow;
        }
    }

    void FilterPitch::processWhammyWah (juce::dsp::AudioBlock<float> block, const Settings& settings) noexcept
    {
        const auto& program = xpPrograms[(size_t) juce::jlimit (0, (int) xpPrograms.size() - 1, settings.xpProgram)];
        pedal.setTargetValue (juce::jlimit (0.0f, 1.0f, settings.pedal));
        auto* samples = block.getChannelPointer (0);
        const auto numSamples = (int) block.getNumSamples();

        switch (program.kind)
        {
            case XpProgram::Kind::volume:
                // A volume pedal: squared, so the swell feels even (loudness isn't linear).
                for (int i = 0; i < numSamples; ++i)
                {
                    const auto position = pedal.getNextValue();
                    samples[i] *= position * position;
                }
                return;

            case XpProgram::Kind::wah:
            {
                // A resonant band-pass swept between the voicing's heel and
                // toe frequencies (spaced by ear), peaking a few dB above the
                // dry level as a real wah does.
                wahFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
                wahFilter.setResonance (program.q);
                const auto outputScale = 1.5f / program.q;
                for (int start = 0; start < numSamples; start += retuneEvery)
                {
                    const auto count = juce::jmin (retuneEvery, numSamples - start);
                    wahFilter.setCutoffFrequency (program.heel * std::pow (program.toe / program.heel, pedal.getNextValue()));
                    pedal.skip (count - 1);
                    for (int i = start; i < start + count; ++i)
                    {
                        const auto mixNow = mix.getNextValue();
                        samples[i] = samples[i] * (1.0f - mixNow) + wahFilter.processSample (0, samples[i]) * outputScale * mixNow;
                    }
                }
                return;
            }

            case XpProgram::Kind::autoWah:
            {
                // An envelope-swept filter, as the Auto-Wah type, in the
                // program's voicing. The pedal sets either its sensitivity
                // (heel: barely opens, toe: opens on the lightest touch) or
                // its attack (heel: snaps open in 2 ms, toe: swells over a
                // quarter of a second, like a brass player's note).
                pedal.skip (numSamples);
                const auto position = pedal.getCurrentValue();
                const auto attackSeconds = program.pedalSetsAttack ? 0.002f * std::pow (125.0f, position) : program.attackSeconds;
                const auto sensitivity = program.pedalSetsAttack ? 6.0f : 10.0f * position;
                const auto attack = coefficientFor (sampleRate, attackSeconds), release = coefficientFor (sampleRate, program.releaseSeconds);
                const auto sweepSmoothing = coefficientFor (sampleRate, 0.01f);
                const auto sensitivityGain = std::pow (10.0f, (sensitivity - 2.0f) * 0.3f);

                wahFilter.setType (program.lowPass ? juce::dsp::StateVariableTPTFilterType::lowpass
                                                   : juce::dsp::StateVariableTPTFilterType::bandpass);
                wahFilter.setResonance (program.q);
                // A band-pass peaks at a gain of Q: scale it to a gentle boost.
                // A low-pass passes the lows as they are, its peak riding on top.
                const auto outputScale = program.lowPass ? 0.8f : 2.0f / program.q;

                for (int start = 0; start < numSamples; start += retuneEvery)
                {
                    const auto count = juce::jmin (retuneEvery, numSamples - start);
                    for (int i = start; i < start + count; ++i)
                    {
                        const auto level = std::abs (samples[i]);
                        envelope += (level - envelope) * (level > envelope ? attack : release);
                    }
                    const auto target = std::sqrt (juce::jlimit (0.0f, 1.0f, envelope * sensitivityGain));
                    sweep += (target - sweep) * (1.0f - std::pow (1.0f - sweepSmoothing, (float) count));
                    wahFilter.setCutoffFrequency (program.heel * std::pow (program.toe / program.heel, sweep));

                    for (int i = start; i < start + count; ++i)
                    {
                        const auto mixNow = mix.getNextValue();
                        const auto wet = wahFilter.processSample (0, samples[i]) * outputScale;
                        samples[i] = samples[i] * (1.0f - mixNow) + wet * mixNow;
                    }
                }
                return;
            }

            case XpProgram::Kind::whammy:
            case XpProgram::Kind::harmony:
                break;
        }

        // Whammy: the pedal bends the shift from none to the program's
        // interval, the shifted guitar replacing the dry one. Harmony: the
        // dry guitar stays, and a second voice moves between the two
        // intervals; the pair is scaled so it's no louder than the guitar alone.
        const auto harmony = program.kind == XpProgram::Kind::harmony;
        for (int i = 0; i < numSamples; ++i)
        {
            const auto semitones = program.heel + (program.toe - program.heel) * pedal.getNextValue();
            const auto dry = samples[i];
            const auto wet = shifter.processSample (dry, std::exp2 (semitones / 12.0f));
            const auto mixNow = mix.getNextValue();
            samples[i] = harmony ? (dry + wet * mixNow) / std::sqrt (1.0f + mixNow * mixNow)
                                 : dry * (1.0f - mixNow) + wet * mixNow;
        }
    }
}
