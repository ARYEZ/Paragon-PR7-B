#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Colours taken from the Paragon X1 mockup: near-black panels with an
// orange accent.
namespace ParagonColours
{
    inline const juce::Colour background { 0xff0b0b0d };
    inline const juce::Colour panel      { 0xff141417 };
    inline const juce::Colour panelEdge  { 0xff2a2a30 };
    inline const juce::Colour accent     { 0xffff7a1a };
    inline const juce::Colour accentDim  { 0xff5a2a0a };
    inline const juce::Colour meterRed   { 0xffff3b2f };
    inline const juce::Colour text       { 0xffe8e8ea };
    inline const juce::Colour textDim    { 0xff8a8a92 };
}

class ParagonLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    ParagonLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;
};
