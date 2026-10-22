#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"
#include "ui/Icons.h"

// The panels along the bottom of the PROCESSOR page, as in the mockup:
//
//   RIG BROWSER | SELECTED RIG | TAGS + RIG SUMMARY | EXPRESSION ASSIGN
//
// plus the SETTINGS page.

// Dark rounded panel with a small grey title, shared by the dock panels.
namespace DockStyle
{
    void drawPanel (juce::Graphics&, juce::Rectangle<float> area);
    void drawTitle (juce::Graphics&, juce::Rectangle<int> area, const juce::String& title);
}

//==============================================================================
// RIG BROWSER: shortcuts into the PRESETS page. Clicking a filter or a bank
// opens the full browser showing just those rigs.
class RigNavigator final : public juce::Component
{
public:
    RigNavigator();

    // `filter`: 0 all, 1 favourites, 2 mine, 3 factory. `bank`: -1 all, 0-3 = A-D.
    std::function<void (int filter, int bank)> onShow;

    void paint (juce::Graphics&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    struct Row
    {
        juce::String text, letter;
        Icons::Type icon;
        int filter, bank;
        juce::Rectangle<int> area;
    };
    std::vector<Row> rows;
    void layoutRows();
    int rowAt (juce::Point<int>) const;

    int hovered = -1, selected = 0;
    juce::Rectangle<int> banksTitle;
};

//==============================================================================
// SELECTED RIG: the loaded rig's name and favourite star, where it's filed
// (bank and number, editable for your own rigs), and SAVE / SAVE AS.
class SelectedRigPanel final : public juce::Component
{
public:
    explicit SelectedRigPanel (ParagonX1AudioProcessor&);

    std::function<void()> onSave, onSaveAs;

    // Re-reads the current rig from the processor.
    void refresh();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    void refile();

    ParagonX1AudioProcessor& processor;
    juce::ComboBox bankBox, numberBox;
    juce::TextButton save { "SAVE" }, saveAs { "SAVE AS..." };
    juce::Rectangle<int> nameBox, starArea, bankLabel, numberLabel;
    juce::String name;
    bool hasEntry = false, favourite = false, canRefile = false, refreshing = false;
};

//==============================================================================
// TAGS and RIG SUMMARY: the rig's tags as chips (+ adds one, click a chip
// to remove it), and a card for each block of the chain showing what it's
// set to. Click a card to edit that block.
class RigSummary final : public juce::Component,
                         public juce::SettableTooltipClient,
                         private juce::Timer
{
public:
    explicit RigSummary (ParagonX1AudioProcessor&);

    std::function<void (int block)> onSelectBlock;

    void refresh();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    struct Card
    {
        juce::String title, line1, line2;
        Icons::Type icon;
        juce::Colour colour;
        bool on = false;
        juce::Rectangle<float> area;
    };

    void timerCallback() override { refresh(); }
    void addTag();
    void removeTag (const juce::String& tag);
    void setTags (const juce::StringArray& tags);
    juce::String text (const char* parameterId) const;
    float value (const char* parameterId) const;

    ParagonX1AudioProcessor& processor;
    std::vector<Card> cards;
    juce::StringArray tags;
    std::vector<juce::Rectangle<float>> tagAreas;
    juce::Rectangle<float> plusArea;
    juce::Rectangle<int> tagsTitle, summaryTitle;
    bool canEditTags = false;
    int hoveredCard = -1;
    std::unique_ptr<juce::AlertWindow> dialog;
};

//==============================================================================
// SETTINGS: tuner reference pitch and mute, the expression pedal's MIDI
// controller, where rigs are kept, and what the plugin is built from.
class SettingsPage final : public juce::Component
{
public:
    explicit SettingsPage (ParagonX1AudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;
    void visibilityChanged() override;

private:
    ParagonX1AudioProcessor& processor;
    juce::Slider reference { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::ToggleButton muteWhileTuning { "Mute the output while the tuner is on" };
    juce::AudioProcessorValueTreeState::ButtonAttachment muteAttachment;
    juce::ComboBox ccBox;
    juce::TextButton openFolder { "OPEN RIGS FOLDER" };
    juce::Rectangle<int> tunerArea, expressionArea, rigsArea, aboutArea;
};
