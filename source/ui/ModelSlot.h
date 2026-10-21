#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"

// The amp or cabinet slot from the mockup: a title with a power LED, a
// display showing the loaded file (click it to choose a file), arrows that
// step through the other files in the same folder, and a clear button.
// Files can also be dragged onto the slot.
class ModelSlot final : public juce::Component,
                        public juce::FileDragAndDropTarget
{
public:
    ModelSlot (const juce::String& title, const juce::String& wildcard, const juce::String& emptyText,
               juce::AudioProcessorValueTreeState& parameters, const juce::String& powerParameterId);

    std::function<void (const juce::File&)> onLoad;
    std::function<void()> onClear;

    void setStatus (const SlotStatus& newStatus);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void fileDragEnter (const juce::StringArray&, int, int) override;
    void fileDragExit (const juce::StringArray&) override;
    void filesDropped (const juce::StringArray& files, int, int) override;

private:
    void chooseFile();
    void step (int direction);
    bool matchesWildcard (const juce::File& file) const;

    juce::String title, wildcard, emptyText;
    SlotStatus status;
    bool dragHover = false;

    juce::ToggleButton power;
    juce::AudioProcessorValueTreeState::ButtonAttachment powerAttachment;
    juce::TextButton previous { "<" }, next { ">" }, clear { "x" };
    juce::Rectangle<int> titleArea, displayArea;
    std::unique_ptr<juce::FileChooser> chooser;
};
