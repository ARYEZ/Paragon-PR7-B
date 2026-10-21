#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"

// CURRENT RIG, bottom right of the PROCESSOR page (the mockup's "SELECTED
// RIG"): the loaded rig's name, favourite star, bank and preset, tags, and
// the buttons to save it or browse for another.
class CurrentRigPanel final : public juce::Component
{
public:
    explicit CurrentRigPanel (ParagonX1AudioProcessor&);

    std::function<void()> onSave, onSaveAs, onBrowse;

    // Re-reads the current rig from the processor.
    void refresh();

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ParagonX1AudioProcessor& processor;
    juce::Label title { {}, "CURRENT RIG" }, nameLabel, slotLabel, tagsLabel;
    juce::TextButton favourite, save { "SAVE" }, saveAs { "SAVE AS..." }, browse { "BROWSE RIGS" };
};
