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
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
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
        // The settings show on hover; the RIG SUMMARY cards in the dock show them too.
        setTooltip (name + (summary.isNotEmpty() ? ":  " + summary : juce::String())
                    + "\nPress the pad to switch on or off; click the name to edit.");
        repaint();
    }
}

void Footswitch::resized()
{
    // LED bar on top, the pad with its number to the left, the name underneath.
    auto area = getLocalBounds();
    ledArea = area.removeFromTop (14);
    area.removeFromTop (9);
    nameArea = area.removeFromBottom (22);
    area.removeFromBottom (5);
    switchArea = area.withTrimmedLeft (46).withTrimmedRight (10);
}

void Footswitch::paint (juce::Graphics& g)
{
    // Status LED: a short neon bar above the pad, as in the mockup.
    ParagonLookAndFeel::drawNeonBar (g, ledArea.toFloat().withSizeKeepingCentre (26.0f, 5.0f).withX ((float) switchArea.getCentreX() - 13.0f),
                                     lastOn);

    // The pad: a big piano-black key with chamfered edges.
    const auto pressed = isMouseButtonDown() && switchArea.contains (getMouseXYRelative());
    const auto hover = isMouseOver() && switchArea.contains (getMouseXYRelative());
    ParagonLookAndFeel::drawPianoBlackKey (g, switchArea.toFloat(), 7.0f, 7.0f, pressed, hover);

    // Number printed to the left of the pad, as on the mockup.
    g.setColour (ParagonColours::ink);
    g.setFont (juce::FontOptions (32.0f, juce::Font::bold));
    g.drawText (juce::String (number), switchArea.withX (0).withWidth (switchArea.getX() - 8),
                juce::Justification::centredRight, false);

    // Name under the pad; the selected block's is underlined in red.
    const auto nameBox = nameArea.withX (switchArea.getX()).withWidth (switchArea.getWidth());
    const auto font = juce::FontOptions (14.5f, juce::Font::bold);
    g.setFont (font);
    g.setColour (selected ? ParagonColours::glow : ParagonColours::ink);
    g.drawText (name, nameBox, juce::Justification::centred, false);
    if (selected)
    {
        const auto width = (float) juce::GlyphArrangement::getStringWidthInt (font, name);
        g.fillRect (juce::Rectangle<float> (width, 2.0f).withCentre ({ (float) nameBox.getCentreX(), (float) nameBox.getBottom() - 1.0f }));
    }
}

void Footswitch::mouseUp (const juce::MouseEvent& event)
{
    if (switchArea.contains (event.getPosition()))
        toggle();
    else if (onSelect)
        onSelect();
    repaint();
}
