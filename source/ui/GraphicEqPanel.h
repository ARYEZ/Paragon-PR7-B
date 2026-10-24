#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "dsp/effects/GraphicEq.h"

// The graphic EQ's knobs, shown in place of the rig summary when the EQ
// block is selected: nine vertical faders standing at their own
// frequencies on a log scale, over a live drawing of the EQ's response on
// the same scale (so the curve passes through the faders' caps), plus LEVEL,
// an ON light, the ULTRA LO and ULTRA HI tone switches, a PRESETS menu of
// common curves, and FLAT to reset every band.
class GraphicEqPanel final : public juce::Component
{
public:
    explicit GraphicEqPanel (juce::AudioProcessorValueTreeState&);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    float xForFrequency (float frequency) const;
    float yForDb (float db) const;
    void flatten();
    void applyPreset (int index);
    void showMatchingPreset();   // names the preset the faders are set to, if any

    juce::AudioProcessorValueTreeState& parameters;
    std::array<juce::Slider, effects::GraphicEq::numBands> faders;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    juce::Slider level { juce::Slider::LinearVertical, juce::Slider::NoTextBox };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> levelAttachment;
    juce::ToggleButton onButton { "ON" };
    juce::AudioProcessorValueTreeState::ButtonAttachment onAttachment;
    juce::ToggleButton ultraLo { "ULTRA LO" }, ultraHi { "ULTRA HI" };
    juce::AudioProcessorValueTreeState::ButtonAttachment ultraLoAttachment, ultraHiAttachment;
    juce::ComboBox presets;
    juce::TextButton flat { "FLAT" };
    juce::Rectangle<float> graph, faderTrack;
};
