#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// The silver floor unit from the mockup: brushed metal with bevelled edges,
// a red-orange glow round the rim, the PARAGON X1 badge, divider lines and
// the recessed panels the slot windows sit in. Purely visual: the controls
// on it are separate components laid on top.
class PedalBody final : public juce::Component
{
public:
    PedalBody();

    // Where to draw recessed panels and divider lines, in this component's
    // coordinates. Set by the layout code after positioning the controls.
    void setInsets (std::vector<juce::Rectangle<int>> panels, std::vector<int> dividerYs, juce::Rectangle<int> badgeArea);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void makeBrushedTexture();

    juce::Image brushed;
    std::vector<juce::Rectangle<int>> panels;
    std::vector<int> dividers;
    juce::Rectangle<int> badge;
};
