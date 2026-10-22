#pragma once

#include "SlotWindow.h"

// The RIG window on the pedal: shows the current rig, the arrows step
// through rigs, and clicking the button opens the rig browser
// ("Load / Save Full Rig" in the mockup).
class RigSlot final : public juce::Component,
                      public juce::SettableTooltipClient
{
public:
    RigSlot();

    std::function<void (int direction)> onStep;
    std::function<void()> onOpen;

    // `slot` is e.g. "A05", empty for an unsaved sound.
    void setRig (const juce::String& slot, const juce::String& name, bool modified);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    juce::String slot, name;
    bool modified = false, buttonHover = false;
    std::unique_ptr<juce::Button> previous, next;
    SlotWindow::Layout layout;
};
