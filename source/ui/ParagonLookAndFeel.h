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

    // The silver pedal unit, and dark "ink" for text printed on it.
    inline const juce::Colour metalLight { 0xfff1edef };
    inline const juce::Colour metalMid   { 0xffd5cfd2 };
    inline const juce::Colour metalDark  { 0xffa59ea2 };
    inline const juce::Colour ink        { 0xff26252a };
    inline const juce::Colour inkDim     { 0xff6b6569 };
    inline const juce::Colour glow       { 0xffff4a3a };   // the red-orange edge light

    // Each block's colour in the signal chain, as in the mockup.
    inline const juce::Colour comp       { 0xff35d6a8 };
    inline const juce::Colour drive      { 0xffffb22e };
    inline const juce::Colour amp        { 0xffff4b3e };
    inline const juce::Colour cab        { 0xffff5d6c };
    inline const juce::Colour modulation { 0xffb86dff };
    inline const juce::Colour delay      { 0xff45b4ff };
    inline const juce::Colour reverb     { 0xff4fe3dd };
}

class ParagonLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    ParagonLookAndFeel();

    // Set this property on a slider to draw it as one of the big black
    // knobs on the pedal face (OUTPUT LEVEL, PRESENCE) instead of the
    // small ringed knob.
    static constexpr const char* pedalKnobProperty = "pedalKnob";

    // Set this on a TextButton to draw it as one of the pedal's small black
    // buttons with an LED stripe (SAVE, BYPASS...). The stripe lights when
    // the button's toggle state is on.
    static constexpr const char* pedalButtonProperty = "pedalButton";

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    // Toggle buttons are drawn as the mockup's red/orange status LEDs.
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& background,
                               bool highlighted, bool down) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;

private:
    static void drawPedalKnob (juce::Graphics&, juce::Rectangle<float> bounds, float angle);
};
