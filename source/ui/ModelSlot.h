#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"
#include "SlotWindow.h"

// The AMP or CABINET window on the pedal: a title with a power LED, a black
// button showing the loaded file (click it to choose a file), arrows that
// step through the other files in the same folder, and a clear button.
// Files can also be dragged onto the window.
class ModelSlot final : public juce::Component,
                        public juce::FileDragAndDropTarget
{
public:
    ModelSlot (const juce::String& title, Icons::Type icon, juce::Colour iconColour, const juce::String& wildcard,
               const juce::String& hint, juce::AudioProcessorValueTreeState& parameters, const juce::String& powerParameterId);

    // Called when the title is clicked (to show this block's settings).
    std::function<void()> onTitleClicked;

    std::function<void (const juce::File&)> onLoad;
    std::function<void()> onClear;

    void setStatus (const SlotStatus& newStatus);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void fileDragEnter (const juce::StringArray&, int, int) override;
    void fileDragExit (const juce::StringArray&) override;
    void filesDropped (const juce::StringArray& files, int, int) override;

private:
    void chooseFile();
    void step (int direction);
    bool matchesWildcard (const juce::File& file) const;

    juce::String title, wildcard, hint;
    Icons::Type icon;
    juce::Colour iconColour;
    SlotStatus status;
    bool dragHover = false, buttonHover = false;

    juce::ToggleButton power;
    juce::AudioProcessorValueTreeState::ButtonAttachment powerAttachment;
    std::unique_ptr<juce::Button> previous, next;
    juce::TextButton clear { "x" };
    SlotWindow::Layout layout;
    std::unique_ptr<juce::FileChooser> chooser;
};
