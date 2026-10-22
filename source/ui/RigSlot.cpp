#include "RigSlot.h"

#include "ParagonLookAndFeel.h"

RigSlot::RigSlot() : previous (SlotWindow::makeArrow (true)), next (SlotWindow::makeArrow (false))
{
    previous->setTooltip ("Previous rig");
    next->setTooltip ("Next rig");
    previous->onClick = [this] { if (onStep) onStep (-1); };
    next->onClick = [this] { if (onStep) onStep (1); };
    addAndMakeVisible (*previous);
    addAndMakeVisible (*next);
}

void RigSlot::setRig (const juce::String& newSlot, const juce::String& newName, bool isModified)
{
    if (newSlot == slot && newName == name && isModified == modified)
        return;
    slot = newSlot;
    name = newName;
    modified = isModified;
    setTooltip ((slot.isNotEmpty() ? slot + "  " : juce::String()) + name + (modified ? "  (unsaved changes)" : juce::String())
                + "\nClick to browse and save rigs; the arrows step through them.");
    repaint();
}

void RigSlot::paint (juce::Graphics& g)
{
    SlotWindow::drawTitle (g, layout, "RIG");

    // Status LED: lit while the sound has unsaved changes.
    const auto titleWidth = juce::GlyphArrangement::getStringWidthInt (juce::FontOptions (14.0f, juce::Font::bold), "RIG");
    SlotWindow::drawRingLed (g, { (float) layout.title.getCentreX() + (float) titleWidth / 2.0f + 13.0f, (float) layout.title.getCentreY() },
                             modified);

    SlotWindow::drawButton (g, layout.button, Icons::Type::rig, ParagonColours::text, buttonHover, false);
    SlotWindow::drawCaption (g, layout.caption, "Load / Save Full Rig", false);
}

void RigSlot::resized()
{
    layout = SlotWindow::layout (getLocalBounds());
    previous->setBounds (layout.previous);
    next->setBounds (layout.next);
}

void RigSlot::mouseUp (const juce::MouseEvent& event)
{
    if (event.mouseWasClicked() && layout.button.contains (event.getPosition()) && onOpen)
        onOpen();
}

void RigSlot::mouseMove (const juce::MouseEvent& event)
{
    const auto hover = layout.button.contains (event.getPosition());
    if (hover != buttonHover)
    {
        buttonHover = hover;
        repaint (layout.button.expanded (4));
    }
}

void RigSlot::mouseExit (const juce::MouseEvent&)
{
    buttonHover = false;
    repaint (layout.button.expanded (4));
}
