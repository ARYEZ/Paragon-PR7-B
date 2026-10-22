#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// The floor unit from the mockup, drawn as one piece of rose-tinted chrome:
// a thick bevelled rim with the red-orange light spilling round it, the
// brushed face plate, raised section panels (the slot row and the
// footswitch row), the recessed panels behind the slot windows, the
// PARAGON PR7 badge, and the bay on the right that the expression pedal sits
// in. Purely visual: the controls on it are separate components on top.
class PedalBody final : public juce::Component
{
public:
    PedalBody();

    // Positions in this component's coordinates, set by the layout code.
    struct Geometry
    {
        juce::Rectangle<float> face;                       // the main face plate
        juce::Rectangle<float> pedalBay;                   // the expression pedal's housing
        std::vector<juce::Rectangle<float>> sections;      // raised panels (slot row, footswitch row)
        std::vector<juce::Rectangle<float>> insets;        // recessed panels (behind each slot window)
        juce::Rectangle<float> badge;                      // PARAGON over the PR7 mark
        juce::Path printedRed, printedInk;                 // lines printed on the face (the tuner bracket, SAVE—SAVE AS)
    };
    void setGeometry (Geometry);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void makeBrushedTexture();
    void paintBadge (juce::Graphics&) const;

    juce::Image brushed;
    Geometry geometry;
};
