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
}

namespace ParamRanges
{
    inline constexpr float gainMinDb = -24.0f;
    inline constexpr float gainMaxDb = 24.0f;
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
