#include "CurrentRigPanel.h"

#include "ParagonLookAndFeel.h"

namespace
{
    const juce::String filledStar { juce::CharPointer_UTF8 ("\xe2\x98\x85") };
    const juce::String emptyStar { juce::CharPointer_UTF8 ("\xe2\x98\x86") };
}

CurrentRigPanel::CurrentRigPanel (ParagonX1AudioProcessor& p) : processor (p)
{
    title.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, ParagonColours::textDim);
    nameLabel.setFont (juce::FontOptions (24.0f));
    slotLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    slotLabel.setColour (juce::Label::textColourId, ParagonColours::textDim);
    tagsLabel.setFont (juce::FontOptions (12.0f));
    tagsLabel.setColour (juce::Label::textColourId, ParagonColours::textDim);
    tagsLabel.setJustificationType (juce::Justification::topLeft);

    favourite.setTooltip ("Favourite");
    favourite.setColour (juce::TextButton::textColourOffId, ParagonColours::accent);
    favourite.onClick = [this]
    {
        if (const auto* entry = processor.findCurrentRigEntry())
        {
            processor.getRigLibrary().setFavourite (*entry, ! entry->rig.favourite);
            refresh();
        }
    };

    save.setColour (juce::TextButton::buttonColourId, ParagonColours::accentDim);
    save.setColour (juce::TextButton::textColourOffId, ParagonColours::accent);
    save.onClick = [this] { if (onSave) onSave(); };
    saveAs.onClick = [this] { if (onSaveAs) onSaveAs(); };
    browse.onClick = [this] { if (onBrowse) onBrowse(); };

    for (auto* component : std::initializer_list<juce::Component*> { &title, &nameLabel, &slotLabel, &tagsLabel,
                                                                      &favourite, &save, &saveAs, &browse })
        addAndMakeVisible (component);

    refresh();
}

void CurrentRigPanel::refresh()
{
    const auto current = processor.getCurrentRig();
    const auto* entry = processor.findCurrentRigEntry();

    nameLabel.setText (current.name + (processor.isRigModified() ? " *" : ""), juce::dontSendNotification);
    favourite.setVisible (entry != nullptr);
    favourite.setButtonText (entry != nullptr && entry->rig.favourite ? filledStar : emptyStar);

    if (entry != nullptr)
    {
        slotLabel.setText ("BANK " + Rig::bankName (entry->rig.bank) + "    PRESET " + juce::String (entry->rig.number).paddedLeft ('0', 3)
                               + (entry->isFactory ? "    FACTORY" : ""),
                           juce::dontSendNotification);
        tagsLabel.setText (entry->rig.tags.isEmpty() ? juce::String ("No tags") : entry->rig.tags.joinIntoString ("   "),
                           juce::dontSendNotification);
    }
    else
    {
        slotLabel.setText ("NOT SAVED", juce::dontSendNotification);
        tagsLabel.setText ("Press SAVE to keep this sound as a rig.", juce::dontSendNotification);
    }
}

void CurrentRigPanel::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setColour (ParagonColours::panel);
    g.fillRoundedRectangle (bounds, 8.0f);
    g.setColour (ParagonColours::panelEdge);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 8.0f, 1.0f);
}

void CurrentRigPanel::resized()
{
    auto area = getLocalBounds().reduced (16, 12);
    title.setBounds (area.removeFromTop (18));
    area.removeFromTop (4);

    auto nameRow = area.removeFromTop (40);
    favourite.setBounds (nameRow.removeFromRight (40).reduced (2));
    nameLabel.setBounds (nameRow);

    slotLabel.setBounds (area.removeFromTop (22));
    area.removeFromTop (4);

    auto buttons = area.removeFromBottom (34);
    const auto third = (buttons.getWidth() - 12) / 3;
    save.setBounds (buttons.removeFromLeft (third));
    buttons.removeFromLeft (6);
    saveAs.setBounds (buttons.removeFromLeft (third));
    buttons.removeFromLeft (6);
    browse.setBounds (buttons);

    area.removeFromBottom (8);
    tagsLabel.setBounds (area);
}
