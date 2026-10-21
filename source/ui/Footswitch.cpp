#include "Footswitch.h"

#include "ParagonLookAndFeel.h"

Footswitch::Footswitch (int numberToShow, const juce::String& nameToShow, juce::AudioProcessorValueTreeState& parametersToUse,
                        juce::StringArray onParameterIdsToUse)
    : number (numberToShow),
      name (nameToShow),
      parameters (parametersToUse),
      onParameterIds (std::move (onParameterIdsToUse))
{
    setTitle (name + " footswitch");
    setRepaintsOnMouseActivity (true);
    // Parameters can change from the DAW (automation) as well as from here,
    // so poll them rather than only reacting to our own clicks.
    startTimerHz (15);
}

bool Footswitch::isAnyOn() const
{
    for (const auto& id : onParameterIds)
        if (parameters.getRawParameterValue (id)->load() > 0.5f)
            return true;
    return false;
}

void Footswitch::toggle()
{
    const auto newValue = isAnyOn() ? 0.0f : 1.0f;
    for (const auto& id : onParameterIds)
    {
        auto* parameter = parameters.getParameter (id);
        // Begin/end tell the DAW this is one user action, for automation recording.
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost (newValue);
        parameter->endChangeGesture();
    }
    refresh();
}

void Footswitch::setSelected (bool shouldBeSelected)
{
    selected = shouldBeSelected;
    repaint();
}

void Footswitch::refresh()
{
    const auto on = isAnyOn();
    const auto newSummary = describe ? describe() : juce::String();
    if (on != lastOn || newSummary != summary)
    {
        lastOn = on;
        summary = newSummary;
        repaint();
    }
}

void Footswitch::resized()
{
    // LED bar on top, then the big pad with the number to its left, then
    // the name and summary printed on the metal underneath.
    auto area = getLocalBounds().reduced (6);
    ledArea = area.removeFromTop (12);
    area.removeFromTop (6);
    nameArea = area.removeFromBottom (36);
    area.removeFromBottom (4);
    switchArea = area.withSizeKeepingCentre (juce::jmin (area.getWidth() - 70, 170), area.getHeight());
}

void Footswitch::paint (juce::Graphics& g)
{
    // Status LED: a short bar above the pad, glowing red-orange when on.
    const auto led = ledArea.toFloat().withSizeKeepingCentre (30.0f, 5.0f);
    if (lastOn)
    {
        g.setColour (ParagonColours::glow.withAlpha (0.35f));
        g.fillRoundedRectangle (led.expanded (5.0f, 4.0f), 6.0f);
    }
    g.setColour (lastOn ? ParagonColours::glow : ParagonColours::glow.withAlpha (0.2f));
    g.fillRoundedRectangle (led, 2.5f);

    // The pad: chunky black rubber with a soft shadow and a lit top edge.
    const auto pressed = isMouseButtonDown() && switchArea.contains (getMouseXYRelative());
    auto pad = switchArea.toFloat();
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillRoundedRectangle (pad.translated (0.0f, pressed ? 2.0f : 5.0f), 8.0f);
    if (pressed)
        pad = pad.translated (0.0f, 2.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff302f34), pad.getTopLeft(),
                                             juce::Colour (0xff060607), pad.getBottomLeft(), false));
    g.fillRoundedRectangle (pad, 8.0f);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawHorizontalLine ((int) pad.getY() + 2, pad.getX() + 8.0f, pad.getRight() - 8.0f);
    const auto hover = isMouseOver() && switchArea.contains (getMouseXYRelative());
    g.setColour (hover ? ParagonColours::glow.withAlpha (0.7f) : juce::Colours::black);
    g.drawRoundedRectangle (pad, 8.0f, 1.0f);

    // Number printed to the left of the pad, as on the mockup.
    g.setColour (ParagonColours::ink);
    g.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    g.drawText (juce::String (number), switchArea.withX (0).withWidth (switchArea.getX() - 6),
                juce::Justification::centredRight, false);

    auto nameText = nameArea;
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.setColour (selected ? ParagonColours::glow : ParagonColours::ink);
    g.drawText (name, nameText.removeFromTop (18), juce::Justification::centred, true);
    if (selected)
    {
        g.setColour (ParagonColours::glow);
        const auto width = (float) juce::GlyphArrangement::getStringWidthInt (juce::FontOptions (14.0f, juce::Font::bold), name);
        g.fillRect (juce::Rectangle<float> (width, 2.0f).withCentre ({ (float) nameArea.getCentreX(), (float) nameText.getY() + 1.0f }));
    }
    g.setFont (juce::FontOptions (11.0f));
    g.setColour (ParagonColours::inkDim);
    g.drawText (summary, nameText.withTrimmedTop (3), juce::Justification::centred, true);
}

void Footswitch::mouseUp (const juce::MouseEvent& event)
{
    if (switchArea.contains (event.getPosition()))
        toggle();
    else if (onSelect)
        onSelect();
    repaint();
}
