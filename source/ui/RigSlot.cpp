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
    repaint();
}

void RigSlot::paint (juce::Graphics& g)
{
    SlotWindow::drawTitle (g, layout, "RIG");

    // Status LED: lit while the rig matches what's saved, dim once it's changed.
    const auto titleWidth = juce::GlyphArrangement::getStringWidthInt (juce::FontOptions (14.0f, juce::Font::bold), "RIG");
    const auto led = juce::Rectangle<float> (8.0f, 8.0f).withCentre ({ (float) layout.title.getCentreX() + (float) titleWidth / 2.0f + 14.0f,
                                                                        (float) layout.title.getCentreY() });
    g.setColour (ParagonColours::glow.withAlpha (modified ? 0.25f : 1.0f));
    g.fillEllipse (led);

    SlotWindow::drawButton (g, layout.button, Icons::Type::rig, ParagonColours::text,
                            slot.isEmpty() ? name : slot + "  " + name, buttonHover, false);
    SlotWindow::drawCaption (g, layout.caption, modified ? "Changed since saved: Load / Save Full Rig" : "Load / Save Full Rig", false);
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
