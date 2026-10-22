#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "Parameters.h"
#include "PluginProcessor.h"

using Catch::Matchers::WithinAbs;

namespace
{
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 256;

    // Reports a fixed tempo, like a DAW with its transport set to `bpm`.
    struct FakePlayHead final : juce::AudioPlayHead
    {
        double bpm = 120.0;
        juce::Optional<PositionInfo> getPosition() const override
        {
            PositionInfo info;
            info.setBpm (bpm);
            return info;
        }
    };

    struct EffectRig
    {
        explicit EffectRig (int outputChannels = 2) : channels (outputChannels)
        {
            processor.setPlayConfigDetails (1, channels, sampleRate, blockSize);
            processor.setPlayHead (&playHead);
            processor.prepareToPlay (sampleRate, blockSize);
        }

        void set (const char* id, float plainValue)
        {
            auto* parameter = processor.getParameters().getParameter (id);
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (plainValue));
        }

        // Runs `input` through the plugin in blocks; returns every output channel.
        std::vector<std::vector<float>> run (const std::vector<float>& input)
        {
            std::vector<std::vector<float>> output ((size_t) channels);
            juce::AudioBuffer<float> buffer (channels, blockSize);
            juce::MidiBuffer midi;

            for (size_t start = 0; start < input.size(); start += blockSize)
            {
                const auto count = (int) std::min<size_t> (blockSize, input.size() - start);
                buffer.setSize (channels, count, false, false, true);
                buffer.clear();
                std::copy (input.begin() + (long) start, input.begin() + (long) start + count, buffer.getWritePointer (0));
                processor.processBlock (buffer, midi);
                for (int ch = 0; ch < channels; ++ch)
                    output[(size_t) ch].insert (output[(size_t) ch].end(), buffer.getReadPointer (ch), buffer.getReadPointer (ch) + count);
            }
            return output;
        }

        std::vector<float> runMono (const std::vector<float>& input) { return run (input)[0]; }

        int channels;
        FakePlayHead playHead;
        ParagonX1AudioProcessor processor;
    };

    std::vector<float> sine (double hz, float amplitude, double seconds)
    {
        std::vector<float> samples ((size_t) (seconds * sampleRate));
        for (size_t i = 0; i < samples.size(); ++i)
            samples[i] = amplitude * (float) std::sin (juce::MathConstants<double>::twoPi * hz * (double) i / sampleRate);
        return samples;
    }

    std::vector<float> impulse (double seconds, float amplitude = 1.0f)
    {
        std::vector<float> samples ((size_t) (seconds * sampleRate), 0.0f);
        samples[0] = amplitude;
        return samples;
    }

    std::vector<float> silence (double seconds) { return std::vector<float> ((size_t) (seconds * sampleRate), 0.0f); }

    float rms (const std::vector<float>& samples, double fromSeconds = 0.0, double toSeconds = 1.0e9)
    {
        const auto from = (size_t) (fromSeconds * sampleRate);
        const auto to = std::min (samples.size(), (size_t) (toSeconds * sampleRate));
        double sum = 0.0;
        for (auto i = from; i < to; ++i)
            sum += samples[i] * samples[i];
        return (float) std::sqrt (sum / (double) std::max<size_t> (1, to - from));
    }

    float dB (float ratio) { return 20.0f * std::log10 (ratio); }

    // Level of one frequency in a signal (Goertzel algorithm), skipping the
    // first `fromSeconds` so filters and fades have settled.
    float levelAt (const std::vector<float>& samples, double hz, double fromSeconds = 0.1)
    {
        const auto from = (size_t) (fromSeconds * sampleRate);
        const auto n = samples.size() - from;
        const auto coefficient = 2.0 * std::cos (juce::MathConstants<double>::twoPi * hz / sampleRate);
        double s1 = 0.0, s2 = 0.0;
        for (auto i = from; i < samples.size(); ++i)
        {
            const auto s0 = samples[i] + coefficient * s1 - s2;
            s2 = s1;
            s1 = s0;
        }
        return (float) (2.0 * std::sqrt (std::max (0.0, s1 * s1 + s2 * s2 - coefficient * s1 * s2)) / (double) n);
    }

    bool allFinite (const std::vector<float>& samples)
    {
        return std::all_of (samples.begin(), samples.end(), [] (float s) { return std::isfinite (s); });
    }

    float maxAbsDifference (const std::vector<float>& a, const std::vector<float>& b, size_t from = 0)
    {
        float result = 0.0f;
        for (auto i = from; i < std::min (a.size(), b.size()); ++i)
            result = std::max (result, std::abs (a[i] - b[i]));
        return result;
    }
}

TEST_CASE ("With every effect off, both channels are the clean input", "[effects]")
{
    EffectRig rig;
    const auto input = sine (440.0, 0.3f, 0.5);
    const auto output = rig.run (input);
    CHECK (maxAbsDifference (input, output[0]) < 1.0e-6f);
    CHECK (maxAbsDifference (input, output[1]) < 1.0e-6f);
}

TEST_CASE ("Compressor turns loud playing down and leaves quiet playing alone", "[effects][comp]")
{
    // Threshold -18 dB, ratio 4:1. A peak at -6 dB is 12 dB over, so it
    // should come out 3 dB over: turned down by about 9 dB.
    EffectRig rig;
    rig.set (ParamIDs::compOn, 1.0f);
    rig.set (ParamIDs::compThreshold, -18.0f);
    rig.set (ParamIDs::compRatio, 4.0f);

    const auto loud = sine (220.0, juce::Decibels::decibelsToGain (-6.0f), 1.0);
    CHECK_THAT (dB (rms (rig.runMono (loud), 0.5) / rms (loud, 0.5)), WithinAbs (-9.0, 1.5));

    const auto quiet = sine (220.0, juce::Decibels::decibelsToGain (-30.0f), 1.0);
    CHECK_THAT (dB (rms (rig.runMono (quiet), 0.5) / rms (quiet, 0.5)), WithinAbs (0.0, 0.3));
}

TEST_CASE ("Noise gate silences signal below its threshold", "[effects][comp]")
{
    EffectRig rig;
    rig.set (ParamIDs::compOn, 1.0f);
    rig.set (ParamIDs::compThreshold, 0.0f);   // compressor out of the way
    rig.set (ParamIDs::gateThreshold, -50.0f);

    const auto hiss = sine (3000.0, juce::Decibels::decibelsToGain (-65.0f), 1.0);
    CHECK (dB (rms (rig.runMono (hiss), 0.5) / rms (hiss, 0.5)) < -20.0f);

    const auto note = sine (220.0, juce::Decibels::decibelsToGain (-20.0f), 1.0);
    CHECK_THAT (dB (rms (rig.runMono (note), 0.5) / rms (note, 0.5)), WithinAbs (0.0, 0.5));
}

TEST_CASE ("Overdrive adds harmonics, and oversampling keeps aliasing down", "[effects][drive]")
{
    EffectRig rig;
    rig.set (ParamIDs::driveOn, 1.0f);
    rig.set (ParamIDs::driveTone, 10.0f);

    SECTION ("Harmonics: odd from clipping, even from the asymmetry")
    {
        rig.set (ParamIDs::driveGain, 7.0f);
        const auto output = rig.runMono (sine (1000.0, 0.3f, 0.5));
        REQUIRE (allFinite (output));
        const auto fundamental = levelAt (output, 1000.0);
        CHECK (dB (levelAt (output, 3000.0) / fundamental) > -30.0f);
        CHECK (dB (levelAt (output, 2000.0) / fundamental) > -40.0f);
    }

    SECTION ("Aliasing")
    {
        // A 5 kHz note at full drive makes harmonics at 15, 25, 35 kHz...
        // Without oversampling, 35 kHz folds back to 13 kHz (48 - 35), a
        // frequency that isn't musically related to the note.
        rig.set (ParamIDs::driveGain, 10.0f);
        const auto output = rig.runMono (sine (5000.0, 0.5f, 0.5));
        CHECK (dB (levelAt (output, 13000.0) / levelAt (output, 5000.0)) < -45.0f);
    }

    SECTION ("Drive 0 at level 0 dB is close to unity; level adds gain")
    {
        rig.set (ParamIDs::driveGain, 0.0f);
        const auto input = sine (1000.0, 0.1f, 0.5);
        CHECK_THAT (dB (rms (rig.runMono (input), 0.2) / rms (input, 0.2)), WithinAbs (0.0, 1.0));

        rig.set (ParamIDs::driveLevel, 6.0f);
        CHECK_THAT (dB (rms (rig.runMono (input), 0.2) / rms (input, 0.2)), WithinAbs (6.0, 1.0));
    }
}

TEST_CASE ("Modulation effects", "[effects][mod]")
{
    EffectRig rig;
    rig.set (ParamIDs::modOn, 1.0f);
    const auto input = sine (440.0, 0.3f, 1.0);

    SECTION ("Chorus spreads the mono guitar across both speakers")
    {
        rig.set (ParamIDs::modType, 0.0f);
        const auto output = rig.run (input);
        REQUIRE (allFinite (output[0]));
        std::vector<float> difference (output[0].size());
        for (size_t i = 0; i < difference.size(); ++i)
            difference[i] = output[0][i] - output[1][i];
        CHECK (rms (difference, 0.2) > 0.05f * rms (output[0], 0.2));
    }

    SECTION ("Tremolo pulses the volume at the rate setting")
    {
        rig.set (ParamIDs::modType, 3.0f);
        rig.set (ParamIDs::modRate, 4.0f);
        rig.set (ParamIDs::modDepth, 1.0f);
        const auto output = rig.runMono (input);

        // Measure loudness in 10 ms windows: full depth swings from silence to full.
        float loudest = 0.0f, quietest = 1.0f;
        for (double t = 0.2; t < 0.95; t += 0.01)
        {
            const auto level = rms (output, t, t + 0.01);
            loudest = std::max (loudest, level);
            quietest = std::min (quietest, level);
        }
        CHECK (loudest > 0.18f);
        CHECK (quietest < 0.02f);
    }

    SECTION ("Flanger and phaser change the sound and stay stable")
    {
        for (const auto type : { 1.0f, 2.0f })
        {
            rig.set (ParamIDs::modType, type);
            const auto output = rig.runMono (input);
            REQUIRE (allFinite (output));
            CHECK (maxAbsDifference (input, output, (size_t) (0.2 * sampleRate)) > 0.01f);
        }
    }
}

TEST_CASE ("Delay repeats at the set time", "[effects][delay]")
{
    EffectRig rig;
    rig.set (ParamIDs::delayOn, 1.0f);
    rig.set (ParamIDs::delayMix, 1.0f);
    rig.set (ParamIDs::delayFeedback, 0.5f);

    const auto peakNear = [] (const std::vector<float>& samples, size_t at, size_t window = 64)
    {
        float peak = 0.0f;
        for (auto i = at - window; i <= at + window; ++i)
            peak = std::max (peak, std::abs (samples[i]));
        return peak;
    };

    SECTION ("Free time in milliseconds")
    {
        rig.set (ParamIDs::delaySync, 0.0f);   // Off
        rig.set (ParamIDs::delayTime, 100.0f);
        const auto output = rig.runMono (impulse (0.5, 0.5f));

        // The first echo is an exact copy 100 ms (4800 samples) later.
        CHECK_THAT (output[4800], WithinAbs (0.5, 0.01));
        CHECK (peakNear (output, 2400) < 0.001f);
        // The second has half the energy (feedback 50%). The tone filter in
        // the loop softens it (darker repeats), which spreads it over a few
        // samples, so add those up rather than looking at the peak.
        float secondEcho = 0.0f;
        for (size_t i = 9600 - 8; i < 9600 + 64; ++i)
            secondEcho += output[i];
        CHECK_THAT (secondEcho, WithinAbs (0.25, 0.01));
    }

    SECTION ("Synced to the DAW tempo: 1/4 note at 120 BPM is 500 ms")
    {
        rig.set (ParamIDs::delaySync, 3.0f);   // 1/4
        rig.playHead.bpm = 120.0;
        const auto output = rig.runMono (impulse (0.7, 0.5f));
        CHECK_THAT (output[24000], WithinAbs (0.5, 0.01));

        // Dotted eighth at 90 BPM: 0.75 beats * 60/90 s = 500 ms as well.
        EffectRig other;
        other.set (ParamIDs::delayOn, 1.0f);
        other.set (ParamIDs::delayMix, 1.0f);
        other.set (ParamIDs::delaySync, 7.0f);   // 1/8 dotted
        other.playHead.bpm = 90.0;
        CHECK_THAT (other.runMono (impulse (0.7, 0.5f))[24000], WithinAbs (0.5, 0.01));
    }

    SECTION ("Ping-pong alternates the repeats between left and right")
    {
        rig.set (ParamIDs::delaySync, 0.0f);
        rig.set (ParamIDs::delayTime, 100.0f);
        rig.set (ParamIDs::delayPingPong, 1.0f);
        const auto output = rig.run (impulse (0.5, 0.5f));
        CHECK (peakNear (output[0], 4800) > 0.1f);    // 1st echo: left
        CHECK (peakNear (output[1], 4800) < 0.001f);
        CHECK (peakNear (output[1], 9600) > 0.05f);   // 2nd echo: right
        CHECK (peakNear (output[0], 9600) < 0.001f);
    }
}

TEST_CASE ("Switching delay off lets the echoes ring out but stops new ones", "[effects][delay]")
{
    EffectRig rig;
    rig.set (ParamIDs::delayOn, 1.0f);
    rig.set (ParamIDs::delayMix, 1.0f);
    rig.set (ParamIDs::delayFeedback, 0.0f);
    rig.set (ParamIDs::delaySync, 0.0f);
    rig.set (ParamIDs::delayTime, 200.0f);

    // Play a click, then switch the delay off before its echo arrives.
    auto first = rig.runMono (impulse (0.1, 0.5f));
    rig.set (ParamIDs::delayOn, 0.0f);
    auto rest = rig.runMono (silence (0.2));
    first.insert (first.end(), rest.begin(), rest.end());
    CHECK (std::abs (first[9600]) > 0.4f);   // the echo still came, 200 ms later

    // A click played now, with the delay off, gets no echo.
    const auto later = rig.runMono (impulse (0.5, 0.5f));
    CHECK (std::abs (later[9600]) < 1.0e-4f);
}

TEST_CASE ("Reverb adds a stereo tail that fades out", "[effects][reverb]")
{
    EffectRig rig;
    rig.set (ParamIDs::reverbOn, 1.0f);
    rig.set (ParamIDs::reverbMix, 0.5f);
    const auto output = rig.run (impulse (3.0, 0.5f));

    REQUIRE (allFinite (output[0]));
    const auto early = rms (output[0], 0.05, 0.3);
    const auto late = rms (output[0], 2.5, 3.0);
    CHECK (early > 1.0e-3f);   // there is a tail
    CHECK (late < early * 0.1f);   // and it decays

    std::vector<float> difference (output[0].size());
    for (size_t i = 0; i < difference.size(); ++i)
        difference[i] = output[0][i] - output[1][i];
    CHECK (rms (difference, 0.05, 0.3) > 0.1f * early);   // left and right differ: it's wide
}

TEST_CASE ("Presence boosts the top end and leaves the lows alone", "[effects][presence]")
{
    EffectRig rig;
    rig.set (ParamIDs::presence, 10.0f);
    const auto highs = sine (9000.0, 0.1f, 0.5);
    const auto lows = sine (150.0, 0.1f, 0.5);
    CHECK (dB (rms (rig.runMono (highs), 0.2) / rms (highs, 0.2)) > 8.0f);
    CHECK_THAT (dB (rms (rig.runMono (lows), 0.2) / rms (lows, 0.2)), WithinAbs (0.0, 0.5));
}

TEST_CASE ("Turning effects on and off doesn't click, and off really is off", "[effects]")
{
    for (const auto* id : { ParamIDs::compOn, ParamIDs::driveOn, ParamIDs::modOn, ParamIDs::delayOn, ParamIDs::reverbOn })
    {
        DYNAMIC_SECTION (id)
        {
            EffectRig rig;
            const auto input = sine (220.0, 0.3f, 0.3);
            rig.set (id, 1.0f);
            rig.runMono (input);

            // Switching off mid-note: no sample may jump further than the
            // biggest step the effect makes on its own.
            const auto on = rig.runMono (input);
            rig.set (id, 0.0f);
            const auto switching = rig.runMono (sine (220.0, 0.3f, 0.05));
            float normalStep = 0.0f, switchStep = 0.0f;
            for (size_t i = 1; i < on.size(); ++i)
                normalStep = std::max (normalStep, std::abs (on[i] - on[i - 1]));
            for (size_t i = 1; i < switching.size(); ++i)
                switchStep = std::max (switchStep, std::abs (switching[i] - switching[i - 1]));
            CHECK (switchStep < std::max (normalStep, 0.03f) * 1.5f);

            // Once the fade (and any tail) is over, the output is the clean
            // input. A delay with feedback takes several seconds to die away.
            rig.runMono (silence (12.0));
            const auto after = rig.runMono (input);
            CHECK (maxAbsDifference (input, after) < 1.0e-4f);
        }
    }
}

TEST_CASE ("Every effect copes with a mono track and with oversized blocks", "[effects]")
{
    const auto turnEverythingOn = [] (EffectRig& rig)
    {
        for (const auto* id : { ParamIDs::compOn, ParamIDs::driveOn, ParamIDs::modOn, ParamIDs::delayOn, ParamIDs::reverbOn })
            rig.set (id, 1.0f);
        rig.set (ParamIDs::delayPingPong, 1.0f);
        rig.set (ParamIDs::presence, 5.0f);
    };

    SECTION ("Mono output")
    {
        EffectRig rig (1);
        turnEverythingOn (rig);
        const auto output = rig.runMono (sine (220.0, 0.3f, 0.5));
        REQUIRE (allFinite (output));
        CHECK (rms (output, 0.2) > 0.01f);
    }

    SECTION ("A host sending 3x the promised block size")
    {
        EffectRig rig;
        turnEverythingOn (rig);
        juce::AudioBuffer<float> big (2, blockSize * 3);
        juce::MidiBuffer midi;
        for (int repeat = 0; repeat < 20; ++repeat)
        {
            big.clear();
            const auto input = sine (220.0, 0.3f, (double) big.getNumSamples() / sampleRate);
            std::copy (input.begin(), input.end(), big.getWritePointer (0));
            rig.processor.processBlock (big, midi);
        }
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < big.getNumSamples(); ++i)
                REQUIRE (std::isfinite (big.getSample (ch, i)));
    }
}

TEST_CASE ("Full chain runs comfortably faster than real time", "[effects][performance]")
{
    EffectRig rig;
    rig.processor.loadAmpModel (juce::File (PARAGON_NAM_EXAMPLE_MODELS).getChildFile ("wavenet.nam"));
    REQUIRE (rig.processor.waitForLoads (20000));
    for (const auto* id : { ParamIDs::compOn, ParamIDs::driveOn, ParamIDs::modOn, ParamIDs::delayOn, ParamIDs::reverbOn })
        rig.set (id, 1.0f);

    const auto input = sine (220.0, 0.3f, 5.0);
    const auto startMs = juce::Time::getMillisecondCounterHiRes();
    rig.runMono (input);
    const auto elapsedSeconds = (juce::Time::getMillisecondCounterHiRes() - startMs) / 1000.0;
    const auto realtimeFraction = elapsedSeconds / 5.0;

    WARN ("Full chain uses " << juce::String (realtimeFraction * 100.0, 1) << "% of one core in real time");
    CHECK (realtimeFraction < 0.5);
}

TEST_CASE ("Graphic EQ: flat is transparent, each band moves only its octave", "[effects][eq]")
{
    // The peak level of a steady tone after the plugin, with the EQ set by `setUp`.
    auto levelOf = [] (float frequency, const std::function<void (EffectRig&)>& setUp)
    {
        EffectRig rig;
        setUp (rig);
        std::vector<float> tone ((size_t) (sampleRate * 0.5));
        for (size_t i = 0; i < tone.size(); ++i)
            tone[i] = 0.1f * (float) std::sin (juce::MathConstants<double>::twoPi * frequency * (double) i / sampleRate);
        const auto out = rig.run (tone)[0];
        return *std::max_element (out.end() - 4800, out.end());
    };
    const auto off = [] (EffectRig&) {};
    const auto toDb = [] (float ratio) { return juce::Decibels::gainToDecibels (ratio); };

    for (auto frequency : { 63.0f, 500.0f, 1000.0f, 4000.0f })
    {
        // Switched on but flat: no change at all.
        const auto flat = levelOf (frequency, [] (EffectRig& rig) { rig.set (ParamIDs::eqOn, 1.0f); });
        CHECK_THAT (toDb (flat / levelOf (frequency, off)), WithinAbs (0.0, 0.05));
    }

    // +12 dB at 1 kHz: a 1 kHz tone comes up by about 12 dB; far-off octaves barely
    // move (a band's skirt spills a little, as on a hardware graphic EQ: under
    // 1 dB three octaves away, under 0.5 dB four away).
    const auto boost1k = [] (EffectRig& rig) { rig.set (ParamIDs::eqOn, 1.0f); rig.set ("eq1k", 12.0f); };
    CHECK_THAT (toDb (levelOf (1000.0f, boost1k) / levelOf (1000.0f, off)), WithinAbs (12.0, 0.5));
    CHECK (std::abs (toDb (levelOf (63.0f, boost1k) / levelOf (63.0f, off))) < 0.5f);
    CHECK (std::abs (toDb (levelOf (8000.0f, boost1k) / levelOf (8000.0f, off))) < 1.0f);

    // Cuts work the same way, and LEVEL moves everything.
    const auto cut125 = [] (EffectRig& rig) { rig.set (ParamIDs::eqOn, 1.0f); rig.set ("eq125", -12.0f); };
    CHECK_THAT (toDb (levelOf (125.0f, cut125) / levelOf (125.0f, off)), WithinAbs (-12.0, 0.5));
    const auto louder = [] (EffectRig& rig) { rig.set (ParamIDs::eqOn, 1.0f); rig.set (ParamIDs::eqLevel, 6.0f); };
    CHECK_THAT (toDb (levelOf (500.0f, louder) / levelOf (500.0f, off)), WithinAbs (6.0, 0.1));

    // Off means off, whatever the faders say.
    const auto bypassed = [] (EffectRig& rig) { rig.set ("eq1k", 12.0f); };
    CHECK_THAT (toDb (levelOf (1000.0f, bypassed) / levelOf (1000.0f, off)), WithinAbs (0.0, 0.01));
}

TEST_CASE ("Graphic EQ's drawn curve matches what it does", "[effects][eq]")
{
    std::array<float, effects::GraphicEq::numBands> gains {};
    CHECK_THAT (effects::GraphicEq::responseDb (gains, 1000.0f), WithinAbs (0.0, 1.0e-6));
    gains[4] = 12.0f;   // 1 kHz
    CHECK_THAT (effects::GraphicEq::responseDb (gains, 1000.0f), WithinAbs (12.0, 0.05));
    CHECK (std::abs (effects::GraphicEq::responseDb (gains, 63.0f)) < 0.5f);
    // A band above half a low sample rate is pulled down rather than breaking.
    CHECK (effects::GraphicEq::bandFrequency (8, 22050.0) < 11025.0f);
}
