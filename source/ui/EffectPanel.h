#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Icons.h"

// The edit panel under the footswitches: the knobs, switches and menus for
// one effect block. Built from a description, so each block's panel is just
// a list of parameter IDs:
//   float parameter  -> knob with its value underneath
//   choice parameter -> drop-down menu
//   on/off parameter -> LED switch
// Each control sits on its own dark card, like the RIG SUMMARY cards it
// replaces; blocks with two parts (comp and gate) show each part's name.
class EffectPanel final : public juce::Component,
                          private juce::Timer
{
public:
    struct Item
    {
        juce::String parameterId, label;
        // Optional: the control is greyed out while this returns false
        // (e.g. delay TIME while the delay is synced to tempo).
        std::function<bool()> isActive = nullptr;
    };

    struct Group
    {
        juce::String title;
        std::vector<Item> items;
    };

    EffectPanel (juce::AudioProcessorValueTreeState& parameters, std::vector<Group> groups);
    ~EffectPanel() override;

    // The block's name, icon and colour, shown at the top as in the signal chain.
    void setHeader (Icons::Type icon, juce::Colour colour, const juce::String& title);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    struct Control;
    struct GroupLayout
    {
        juce::String title;
        std::vector<Control*> controls;
        juce::Rectangle<int> area;
    };

    std::vector<std::unique_ptr<Control>> controls;
    std::vector<GroupLayout> groups;
    std::vector<juce::Rectangle<int>> tiles;
    Icons::Type headerIcon = Icons::Type::comp;
    juce::Colour headerColour;
    juce::String headerTitle;
};
