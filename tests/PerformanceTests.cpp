// Phase 6: the "performance" features: tap tempo, undo/redo, the tuner's
// mute and the expression pedal.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "Parameters.h"
#include "PluginProcessor.h"
#include "dsp/TapTempo.h"
#include "rigs/UndoHistory.h"

using Catch::Matchers::WithinAbs;

namespace
{
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 256;

    struct TestRig
    {
        TestRig()
        {
            folder.createDirectory();
            processor.useRigFolder (folder);
            processor.setPlayConfigDetails (1, 2, sampleRate, blockSize);
            processor.prepareToPlay (sampleRate, blockSize);
        }
        ~TestRig() { folder.deleteRecursively(); }

        juce::RangedAudioParameter& parameter (const char* id) { return *processor.getParameters().getParameter (id); }
        float value (const char* id) { return parameter (id).convertFrom0to1 (parameter (id).getValue()); }

        // A knob turned with the mouse: a gesture around the change, as the UI does.
        void turn (const char* id, float plainValue)
        {
            auto& p = parameter (id);
            p.beginChangeGesture();
            p.setValueNotifyingHost (p.convertTo0to1 (plainValue));
            p.endChangeGesture();
            processor.flushUndoStep();
        }

        // Plays a 220 Hz tone for `seconds`; returns the peak of the last block.
        float play (double seconds, juce::MidiBuffer midi = {})
        {
            juce::AudioBuffer<float> buffer (2, blockSize);
            float peak = 0.0f;
            for (int b = 0; b < (int) (seconds * sampleRate / blockSize); ++b)
            {
                buffer.clear();
                for (int i = 0; i < blockSize; ++i)
                    buffer.setSample (0, i, 0.25f * (float) std::sin (juce::MathConstants<double>::twoPi * 220.0 * (phase++) / sampleRate));
                processor.processBlock (buffer, midi);
                midi.clear();
                peak = buffer.getMagnitude (0, 0, blockSize);
            }
            return peak;
        }

        juce::File folder { juce::File::getSpecialLocation (juce::File::tempDirectory)
                                .getChildFile ("ParagonX1-perf-" + juce::String::toHexString (juce::Random::getSystemRandom().nextInt64())) };
        ParagonX1AudioProcessor processor;
        long phase = 0;
    };
}

TEST_CASE ("Tap tempo averages the taps and starts again after a pause", "[tap]")
{
    TapTempo taps;
    CHECK_FALSE (taps.tap (0.0).has_value());   // one tap is not a tempo

    auto bpm = taps.tap (500.0);
    REQUIRE (bpm.has_value());
    CHECK_THAT (*bpm, WithinAbs (120.0, 1.0e-9));

    // One late tap moves the average only a little.
    taps.tap (1000.0);
    taps.tap (1500.0);
    bpm = taps.tap (2060.0);
    CHECK (*bpm > 113.0);
    CHECK (*bpm < 120.0);

    // A long pause: the next tap is the first of a new count.
    CHECK_FALSE (taps.tap (6000.0).has_value());
    CHECK_THAT (*taps.tap (6600.0), WithinAbs (100.0, 1.0e-9));

    // Absurdly fast tapping is limited to something a delay can use.
    TapTempo fast;
    fast.tap (0.0);
    CHECK_THAT (*fast.tap (10.0), WithinAbs (TapTempo::maxBpm, 1.0e-9));
}

TEST_CASE ("Undo history steps back and forward and forgets redo after a new edit", "[undo]")
{
    UndoHistory<int> history;
    history.record (1);
    CHECK_FALSE (history.canUndo());
    history.record (2);
    history.record (2);   // no change: no extra step
    history.record (3);

    CHECK (*history.undo() == 2);
    CHECK (*history.undo() == 1);
    CHECK_FALSE (history.undo().has_value());
    CHECK (*history.redo() == 2);

    history.record (9);   // a new edit after undoing
    CHECK_FALSE (history.canRedo());
    CHECK (*history.undo() == 2);

    for (int i = 0; i < 500; ++i)
        history.record (100 + i);
    int steps = 0;
    while (history.undo())
        ++steps;
    CHECK (steps == (int) UndoHistory<int>::maxSteps - 1);
}

TEST_CASE ("Undo and redo take back whole knob moves", "[undo]")
{
    TestRig rig;
    rig.turn (ParamIDs::driveGain, 8.0f);
    rig.turn (ParamIDs::delayMix, 0.6f);
    REQUIRE (rig.processor.canUndo());

    rig.processor.undo();
    CHECK_THAT (rig.value (ParamIDs::delayMix), WithinAbs (0.25, 1.0e-4));   // back to the default
    CHECK_THAT (rig.value (ParamIDs::driveGain), WithinAbs (8.0, 1.0e-4));   // untouched

    rig.processor.undo();
    CHECK_THAT (rig.value (ParamIDs::driveGain), WithinAbs (5.0, 1.0e-4));
    CHECK_FALSE (rig.processor.canUndo());

    rig.processor.redo();
    rig.processor.redo();
    CHECK_THAT (rig.value (ParamIDs::driveGain), WithinAbs (8.0, 1.0e-4));
    CHECK_THAT (rig.value (ParamIDs::delayMix), WithinAbs (0.6, 1.0e-4));
    CHECK_FALSE (rig.processor.canRedo());
}

TEST_CASE ("Undo doesn't take back the global knobs or the pedal position", "[undo]")
{
    TestRig rig;
    rig.turn (ParamIDs::driveGain, 7.0f);
    rig.turn (ParamIDs::outputGain, -6.0f);
    rig.turn (ParamIDs::expression, 0.3f);

    rig.processor.undo();
    CHECK_THAT (rig.value (ParamIDs::driveGain), WithinAbs (5.0, 1.0e-4));
    CHECK_THAT (rig.value (ParamIDs::outputGain), WithinAbs (-6.0, 1.0e-4));
    CHECK_THAT (rig.value (ParamIDs::expression), WithinAbs (0.3, 1.0e-3));
}

TEST_CASE ("Undoing a rig change brings back the previous rig and its name", "[undo][rigs]")
{
    TestRig rig;
    rig.turn (ParamIDs::reverbMix, 0.5f);

    const auto* surf = rig.processor.getRigLibrary().findFactory ("Surf Tremolo");
    REQUIRE (surf != nullptr);
    rig.processor.loadRig (*surf);
    rig.processor.flushUndoStep();
    CHECK (rig.processor.getCurrentRig().name == "Surf Tremolo");
    CHECK_FALSE (rig.processor.isRigModified());

    rig.processor.undo();
    CHECK (rig.processor.getCurrentRig().name == "Untitled");
    CHECK_THAT (rig.value (ParamIDs::reverbMix), WithinAbs (0.5, 1.0e-4));
    CHECK (rig.processor.isRigModified());   // it was edited before the rig change

    rig.processor.redo();
    CHECK (rig.processor.getCurrentRig().name == "Surf Tremolo");
    CHECK_FALSE (rig.processor.isRigModified());
}

TEST_CASE ("Tapped tempo replaces the DAW's until cleared, and is saved", "[tap]")
{
    TestRig rig;
    CHECK_THAT (rig.processor.getTempoBpm(), WithinAbs (120.0, 1.0e-9));   // no DAW tempo: 120

    rig.processor.tapTempo();
    juce::Thread::sleep (400);
    rig.processor.tapTempo();
    const auto tapped = rig.processor.getTappedBpm();
    CHECK (tapped > 100.0);    // ~150, allowing for a slow test machine
    CHECK (tapped < 160.0);
    CHECK_THAT (rig.processor.getTempoBpm(), WithinAbs (tapped, 1.0e-9));

    juce::MemoryBlock state;
    rig.processor.getStateInformation (state);
    rig.processor.clearTappedTempo();
    CHECK_THAT (rig.processor.getTempoBpm(), WithinAbs (120.0, 1.0e-9));

    rig.processor.setStateInformation (state.getData(), (int) state.getSize());
    CHECK_THAT (rig.processor.getTappedBpm(), WithinAbs (tapped, 1.0e-9));
}

TEST_CASE ("The tuner mutes the output, unless told not to", "[tuner]")
{
    TestRig rig;
    const auto normal = rig.play (0.3);
    REQUIRE (normal > 0.05f);

    rig.turn (ParamIDs::tunerOn, 1.0f);
    CHECK (rig.play (0.1) == 0.0f);

    rig.turn (ParamIDs::tunerMute, 0.0f);
    CHECK_THAT (rig.play (0.1), WithinAbs (normal, 0.01));

    // And the tuner can hear the guitar while muted.
    rig.turn (ParamIDs::tunerMute, 1.0f);
    rig.play (0.3);
    std::vector<float> heard (8192);
    rig.processor.getInputTap().readLatest (heard.data(), (int) heard.size());
    CHECK (*std::max_element (heard.begin(), heard.end()) > 0.2f);
}

TEST_CASE ("Expression pedal as a volume pedal, within its MIN and MAX", "[expression]")
{
    TestRig rig;
    const auto full = rig.play (0.3);

    rig.turn (ParamIDs::expressionTarget, (float) ExpressionTargets::volume);
    CHECK_THAT (rig.play (0.2), WithinAbs (full, 0.005));   // toe down by default: no change

    rig.turn (ParamIDs::expression, 0.0f);
    CHECK (rig.play (0.2) < 1.0e-4f);                       // heel down: silent

    rig.turn (ParamIDs::expression, 0.5f);
    CHECK_THAT (rig.play (0.2), WithinAbs (full * 0.25, 0.01));   // audio taper: half way is -12 dB

    rig.turn (ParamIDs::expressionMin, 0.5f);
    rig.turn (ParamIDs::expression, 0.0f);
    CHECK_THAT (rig.play (0.2), WithinAbs (full * 0.25, 0.01));   // heel now stops at half volume
}

TEST_CASE ("MIDI CC 11 moves the expression pedal", "[expression]")
{
    TestRig rig;
    rig.turn (ParamIDs::expressionTarget, (float) ExpressionTargets::volume);
    const auto full = rig.play (0.3);

    juce::MidiBuffer midi;
    midi.addEvent (juce::MidiMessage::controllerEvent (1, 11, 0), 0);
    CHECK (rig.play (0.2, midi) < 1.0e-4f);
    CHECK_THAT (rig.processor.getExpressionPosition(), WithinAbs (0.0, 1.0e-6));

    // Other controllers are ignored.
    midi.addEvent (juce::MidiMessage::controllerEvent (1, 7, 127), 0);
    CHECK (rig.play (0.1, midi) < 1.0e-4f);

    midi.addEvent (juce::MidiMessage::controllerEvent (1, 11, 127), 0);
    CHECK_THAT (rig.play (0.2, midi), WithinAbs (full, 0.005));
}

TEST_CASE ("Expression pedal as WAH and MORPH FX", "[expression]")
{
    TestRig rig;
    const auto clean = rig.play (0.3);

    // A 220 Hz tone is below the wah's sweep: loud at the heel, cut at the toe.
    rig.turn (ParamIDs::expressionTarget, (float) ExpressionTargets::wah);
    rig.turn (ParamIDs::expression, 0.0f);
    const auto heel = rig.play (0.3);
    rig.turn (ParamIDs::expression, 1.0f);
    const auto toe = rig.play (0.3);
    CHECK (heel > 2.0f * toe);

    // MORPH at the heel: the time effects are dry, so the output is the clean tone.
    rig.turn (ParamIDs::expressionTarget, (float) ExpressionTargets::morph);
    rig.turn (ParamIDs::reverbOn, 1.0f);
    rig.turn (ParamIDs::reverbMix, 1.0f);
    rig.turn (ParamIDs::expression, 0.0f);
    CHECK_THAT (rig.play (0.5), WithinAbs (clean, 0.005));
    rig.turn (ParamIDs::expression, 1.0f);
    CHECK (std::abs (rig.play (0.5) - clean) > 0.02f);
}
