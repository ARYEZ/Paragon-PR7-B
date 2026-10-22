#include "Parameters.h"

#include "dsp/effects/CompGate.h"
#include "dsp/effects/Delay.h"
#include "dsp/effects/GraphicEq.h"

namespace
{
    using Range = juce::NormalisableRange<float>;
    using ToText = std::function<juce::String (float, int)>;

    const ToText asDb = [] (float value, int) { return juce::String (value, 1) + " dB"; };
    const ToText asKnob = [] (float value, int) { return juce::String (value, 1); };
    const ToText asPercent = [] (float value, int) { return juce::String (juce::roundToInt (value * 100.0f)) + "%"; };
    const ToText asHz = [] (float value, int) { return juce::String (value, value < 1.0f ? 2 : 1) + " Hz"; };
    const ToText asMs = [] (float value, int) { return juce::String (juce::roundToInt (value)) + " ms"; };
    const ToText asRatio = [] (float value, int) { return juce::String (value, 1) + ":1"; };
    const ToText asGate = [] (float value, int)
    {
        return value <= effects::CompGate::gateOffDb ? juce::String ("Off") : juce::String (juce::roundToInt (value)) + " dB";
    };

    // Range with its centre value at the middle of the knob's travel, for
    // ranges where the useful settings are bunched at one end (rate, time).
    Range skewed (float min, float max, float centre, float step = 0.0f)
    {
        Range range { min, max, step };
        range.setSkewForCentre (centre);
        return range;
    }

    std::unique_ptr<juce::AudioParameterFloat> makeFloat (const char* id, const juce::String& name, Range range,
                                                          float defaultValue, int versionHint, const ToText& toText)
    {
        return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, versionHint }, name, range, defaultValue,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (toText));
    }

    // Version hint 1: tells hosts which plugin version introduced the parameter.
    std::unique_ptr<juce::AudioParameterFloat> makeGainParameter (const char* id, const juce::String& name)
    {
        return makeFloat (id, name, { ParamRanges::gainMinDb, ParamRanges::gainMaxDb, 0.1f }, 0.0f, 1, asDb);
    }

    std::unique_ptr<juce::AudioParameterBool> makeSwitch (const char* id, const juce::String& name, int versionHint,
                                                          bool defaultValue = true)
    {
        return std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, versionHint }, name, defaultValue);
    }

    std::unique_ptr<juce::AudioParameterChoice> makeChoice (const char* id, const juce::String& name, int versionHint,
                                                            const juce::StringArray& choices, int defaultIndex)
    {
        return std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { id, versionHint }, name, choices, defaultIndex);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (makeGainParameter (ParamIDs::inputGain, "Input"));
    layout.add (makeGainParameter (ParamIDs::outputGain, "Output"));

    // Added in plugin version 2 (Phase 2).
    layout.add (makeSwitch (ParamIDs::ampOn, "Amp On", 2));
    layout.add (makeSwitch (ParamIDs::ampNormalise, "Amp Normalise", 2));
    layout.add (makeSwitch (ParamIDs::cabOn, "Cab On", 2));

    // Added in plugin version 3 (Phase 3). Effects default to off, so
    // projects saved with earlier versions sound exactly the same.
    constexpr int v3 = 3;
    layout.add (makeFloat (ParamIDs::presence, "Presence", { -10.0f, 10.0f, 0.1f }, 0.0f, v3, asDb));

    layout.add (makeSwitch (ParamIDs::compOn, "Comp On", v3, false));
    layout.add (makeFloat (ParamIDs::compThreshold, "Comp Threshold", { -40.0f, 0.0f, 0.1f }, -18.0f, v3, asDb));
    layout.add (makeFloat (ParamIDs::compRatio, "Comp Ratio", skewed (1.0f, 20.0f, 4.0f, 0.1f), 4.0f, v3, asRatio));
    layout.add (makeFloat (ParamIDs::compLevel, "Comp Level", { -12.0f, 24.0f, 0.1f }, 0.0f, v3, asDb));
    layout.add (makeFloat (ParamIDs::gateThreshold, "Gate Threshold", { effects::CompGate::gateOffDb, -20.0f, 1.0f },
                           effects::CompGate::gateOffDb, v3, asGate));

    layout.add (makeSwitch (ParamIDs::driveOn, "Drive On", v3, false));
    layout.add (makeFloat (ParamIDs::driveGain, "Drive", { 0.0f, 10.0f, 0.1f }, 5.0f, v3, asKnob));
    layout.add (makeFloat (ParamIDs::driveTone, "Drive Tone", { 0.0f, 10.0f, 0.1f }, 5.0f, v3, asKnob));
    layout.add (makeFloat (ParamIDs::driveLevel, "Drive Level", { -24.0f, 12.0f, 0.1f }, 0.0f, v3, asDb));

    layout.add (makeSwitch (ParamIDs::modOn, "Mod On", v3, false));
    layout.add (makeChoice (ParamIDs::modType, "Mod Type", v3, { "Chorus", "Flanger", "Phaser", "Tremolo" }, 0));
    layout.add (makeFloat (ParamIDs::modRate, "Mod Rate", skewed (0.05f, 10.0f, 1.0f, 0.01f), 0.8f, v3, asHz));
    layout.add (makeFloat (ParamIDs::modDepth, "Mod Depth", { 0.0f, 1.0f, 0.01f }, 0.45f, v3, asPercent));
    layout.add (makeFloat (ParamIDs::modMix, "Mod Mix", { 0.0f, 1.0f, 0.01f }, 0.5f, v3, asPercent));

    layout.add (makeSwitch (ParamIDs::delayOn, "Delay On", v3, false));
    layout.add (makeFloat (ParamIDs::delayTime, "Delay Time", skewed (1.0f, effects::Delay::maxDelaySeconds * 1000.0f, 400.0f, 1.0f),
                           375.0f, v3, asMs));
    layout.add (makeChoice (ParamIDs::delaySync, "Delay Sync", v3, effects::Delay::syncNames, 3));   // 1/4, as in the mockup
    layout.add (makeFloat (ParamIDs::delayFeedback, "Delay Feedback", { 0.0f, 0.95f, 0.01f }, 0.35f, v3, asPercent));
    layout.add (makeFloat (ParamIDs::delayTone, "Delay Tone", { 0.0f, 10.0f, 0.1f }, 7.0f, v3, asKnob));
    layout.add (makeFloat (ParamIDs::delayMix, "Delay Mix", { 0.0f, 1.0f, 0.01f }, 0.25f, v3, asPercent));
    layout.add (makeSwitch (ParamIDs::delayPingPong, "Delay Ping-Pong", v3, false));

    layout.add (makeSwitch (ParamIDs::reverbOn, "Reverb On", v3, false));
    layout.add (makeFloat (ParamIDs::reverbSize, "Reverb Size", { 0.0f, 1.0f, 0.01f }, 0.5f, v3, asPercent));
    layout.add (makeFloat (ParamIDs::reverbTone, "Reverb Tone", { 0.0f, 10.0f, 0.1f }, 6.0f, v3, asKnob));
    layout.add (makeFloat (ParamIDs::reverbMix, "Reverb Mix", { 0.0f, 1.0f, 0.01f }, 0.28f, v3, asPercent));

    // Added in plugin version 4 (Phase 4): the OUTPUT LEVEL knob on the
    // mockup's pedal face, saved with each rig to balance rigs' volumes.
    layout.add (makeFloat (ParamIDs::rigLevel, "Rig Level", { -24.0f, 12.0f, 0.1f }, 0.0f, 4, asDb));

    // Added in plugin version 5 (Phase 5): the pedal's BYPASS button. It's
    // also the plugin's official bypass, so the DAW's bypass uses it too.
    layout.add (makeSwitch (ParamIDs::bypass, "Bypass", 5, false));

    // Added in plugin version 6 (Phase 6): the tuner and the expression pedal.
    constexpr int v6 = 6;
    layout.add (makeSwitch (ParamIDs::tunerOn, "Tuner", v6, false));
    layout.add (makeSwitch (ParamIDs::tunerMute, "Tuner Mutes Output", v6, true));
    // Toe down by default, so assigning the pedal to VOLUME doesn't silence the rig.
    layout.add (makeFloat (ParamIDs::expression, "Expression", { 0.0f, 1.0f, 0.001f }, 1.0f, v6, asPercent));
    layout.add (makeChoice (ParamIDs::expressionTarget, "Expression Target", v6, ExpressionTargets::names, ExpressionTargets::off));
    layout.add (makeFloat (ParamIDs::expressionMin, "Expression Min", { 0.0f, 1.0f, 0.01f }, 0.0f, v6, asPercent));
    layout.add (makeFloat (ParamIDs::expressionMax, "Expression Max", { 0.0f, 1.0f, 0.01f }, 1.0f, v6, asPercent));

    // Added in plugin version 8: the graphic EQ. Off by default, so existing
    // projects and rigs sound the same.
    constexpr int v8 = 8;
    layout.add (makeSwitch (ParamIDs::eqOn, "EQ On", v8, false));
    for (int band = 0; band < ParamIDs::eqBands.size(); ++band)
        layout.add (makeFloat (ParamIDs::eqBands[band].toRawUTF8(), "EQ " + ParamIDs::eqBandNames[band] + " Hz",
                               { -effects::GraphicEq::maxGainDb, effects::GraphicEq::maxGainDb, 0.1f }, 0.0f, v8, asDb));
    layout.add (makeFloat (ParamIDs::eqLevel, "EQ Level", { -12.0f, 12.0f, 0.1f }, 0.0f, v8, asDb));
    return layout;
}
