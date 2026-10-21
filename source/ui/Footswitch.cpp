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
    auto area = getLocalBounds().reduced (8);
    ledArea = area.removeFromTop (12);
    area.removeFromTop (4);
    nameArea = area.removeFromBottom (36);
    switchArea = area.withSizeKeepingCentre (juce::jmin (area.getWidth() - 20, 150), juce::jmin (area.getHeight(), 70));
}

void Footswitch::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setColour (ParagonColours::panel);
    g.fillRoundedRectangle (bounds, 8.0f);
    g.setColour (selected ? ParagonColours::accent : ParagonColours::panelEdge);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 8.0f, selected ? 2.0f : 1.0f);

    // Status LED: a short bar, like the mockup's red lights above each switch.
    const auto led = ledArea.toFloat().withSizeKeepingCentre (28.0f, 4.0f);
    if (lastOn)
    {
        g.setColour (ParagonColours::accent.withAlpha (0.3f));
        g.fillRoundedRectangle (led.expanded (3.0f), 4.0f);
    }
    g.setColour (lastOn ? ParagonColours::accent : ParagonColours::accentDim.darker (0.6f));
    g.fillRoundedRectangle (led, 2.0f);

    // The switch itself: a dark rubber pad.
    const auto pad = switchArea.toFloat();
    const auto pressed = isMouseButtonDown() && switchArea.contains (getMouseXYRelative());
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a2a2e), pad.getTopLeft(),
                                             juce::Colour (0xff0a0a0b), pad.getBottomLeft(), false));
    g.fillRoundedRectangle (pressed ? pad.reduced (2.0f) : pad, 6.0f);
    g.setColour (isMouseOver() && switchArea.contains (getMouseXYRelative()) ? ParagonColours::textDim : juce::Colours::black);
    g.drawRoundedRectangle (pad, 6.0f, 1.0f);

    // Number to the left of the pad, as on the mockup.
    g.setColour (ParagonColours::text);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText (juce::String (number), switchArea.withX (getLocalBounds().getX()).withWidth (switchArea.getX() - 2),
                juce::Justification::centred, false);

    auto nameText = nameArea;
    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    g.setColour (selected ? ParagonColours::accent : ParagonColours::text);
    g.drawText (name, nameText.removeFromTop (18), juce::Justification::centred, true);
    g.setFont (juce::FontOptions (11.0f));
    g.setColour (ParagonColours::textDim);
    g.drawText (summary, nameText, juce::Justification::centred, true);
}

void Footswitch::mouseUp (const juce::MouseEvent& event)
{
    if (switchArea.contains (event.getPosition()))
        toggle();
    else if (onSelect)
        onSelect();
    repaint();
}
