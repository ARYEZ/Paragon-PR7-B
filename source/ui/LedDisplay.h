#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// The pedal's orange LED screen: the rig number in big seven-segment digits,
// then the rig name, its bank and preset number, and a dot that lights when
// the rig has unsaved changes. Click it to open the rig browser.
class LedDisplay final : public juce::Component,
                         public juce::SettableTooltipClient
{
public:
    LedDisplay();

    struct Contents
    {
        int number = 0;              // 0 shows "---" (no saved rig)
        juce::String name;
        juce::String bank;           // "A".."D", empty when unsaved
        bool modified = false;
        bool bypassed = false;
    };

    void setContents (const Contents&);
    std::function<void()> onClick;

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;

    // Draws one seven-segment digit ('0'-'9' or '-') in `area`. Public for reuse.
    static void drawDigit (juce::Graphics&, juce::juce_wchar digit, juce::Rectangle<float> area, juce::Colour lit);

private:
    Contents contents;
};
