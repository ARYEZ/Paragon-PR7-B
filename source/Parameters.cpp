#include "Parameters.h"

namespace
{
    std::unique_ptr<juce::AudioParameterFloat> makeGainParameter (const char* id, const juce::String& name)
    {
        // Version hint 1: tells hosts which plugin version introduced the parameter.
        return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 },
            name,
            juce::NormalisableRange<float> { ParamRanges::gainMinDb, ParamRanges::gainMaxDb, 0.1f },
            0.0f,
            juce::AudioParameterFloatAttributes()
                .withLabel ("dB")
                .withStringFromValueFunction ([] (float value, int) { return juce::String (value, 1) + " dB"; }));
    }

    std::unique_ptr<juce::AudioParameterBool> makeSwitch (const char* id, const juce::String& name, int versionHint)
    {
        return std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, versionHint }, name, true);
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
    return layout;
}
