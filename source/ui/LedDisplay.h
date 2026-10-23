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

        // While the tuner is on, the screen shows it instead of the rig.
        struct Tuner
        {
            bool active = false;
            bool hasNote = false;   // false: nothing played yet, or the note died away
            juce::String note;      // "E", "F#"...
            int octave = 0;
            double cents = 0.0;     // -50 to +50
            double frequency = 0.0;
            bool muted = true;

            bool operator== (const Tuner&) const = default;
        } tuner;
    };

    void setContents (const Contents&);
    std::function<void()> onClick;

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;

    // Draws one seven-segment digit ('0'-'9' or '-') in `area`. Public for reuse.
    static void drawDigit (juce::Graphics&, juce::juce_wchar digit, juce::Rectangle<float> area, juce::Colour lit);

private:
    void paintTuner (juce::Graphics&, juce::Rectangle<float> content, juce::Colour colour) const;
    void paintGlassOverlay (juce::Graphics&, juce::Rectangle<float> glass) const;

    Contents contents;
};
