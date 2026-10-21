#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// Parameter IDs are saved in DAW projects and presets, so never rename one
// once it has shipped. Add new parameters with a new ID instead.
namespace ParamIDs
{
    inline constexpr auto inputGain    = "inputGain";
    inline constexpr auto outputGain   = "outputGain";
    inline constexpr auto ampOn        = "ampOn";
    inline constexpr auto ampNormalise = "ampNormalise";
    inline constexpr auto cabOn        = "cabOn";

    // Phase 3
    inline constexpr auto presence       = "presence";

    inline constexpr auto compOn         = "compOn";
    inline constexpr auto compThreshold  = "compThreshold";
    inline constexpr auto compRatio      = "compRatio";
    inline constexpr auto compLevel      = "compLevel";
    inline constexpr auto gateThreshold  = "gateThreshold";

    inline constexpr auto driveOn        = "driveOn";
    inline constexpr auto driveGain      = "driveGain";
    inline constexpr auto driveTone      = "driveTone";
    inline constexpr auto driveLevel     = "driveLevel";

    inline constexpr auto modOn          = "modOn";
    inline constexpr auto modType        = "modType";
    inline constexpr auto modRate        = "modRate";
    inline constexpr auto modDepth       = "modDepth";
    inline constexpr auto modMix         = "modMix";

    inline constexpr auto delayOn        = "delayOn";
    inline constexpr auto delayTime      = "delayTime";
    inline constexpr auto delaySync      = "delaySync";
    inline constexpr auto delayFeedback  = "delayFeedback";
    inline constexpr auto delayTone      = "delayTone";
    inline constexpr auto delayMix       = "delayMix";
    inline constexpr auto delayPingPong  = "delayPingPong";

    inline constexpr auto reverbOn       = "reverbOn";
    inline constexpr auto reverbSize     = "reverbSize";
    inline constexpr auto reverbTone     = "reverbTone";
    inline constexpr auto reverbMix      = "reverbMix";

    // Phase 4
    inline constexpr auto rigLevel       = "rigLevel";

    // Phase 5
    inline constexpr auto bypass         = "bypass";

    // Global settings: kept when switching rigs. Everything else is part of a rig.
    inline const juce::StringArray notInRigs { inputGain, outputGain, bypass };
}

namespace ParamRanges
{
    inline constexpr float gainMinDb = -24.0f;
    inline constexpr float gainMaxDb = 24.0f;
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
