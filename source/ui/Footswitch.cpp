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
    // Status LED: a short bar above the pad, glowing pink-red when on.
    const auto led = ledArea.toFloat().withSizeKeepingCentre (26.0f, 4.0f).withX ((float) switchArea.getCentreX() - 13.0f);
    if (lastOn)
    {
        g.setColour (ParagonColours::glow.withAlpha (0.3f));
        g.fillRoundedRectangle (led.expanded (7.0f, 5.0f), 7.0f);
        g.setColour (ParagonColours::glow.withAlpha (0.45f));
        g.fillRoundedRectangle (led.expanded (3.0f, 2.0f), 4.0f);
    }
    g.setColour (lastOn ? juce::Colour (0xffff6a6a) : ParagonColours::glow.withAlpha (0.28f));
    g.fillRoundedRectangle (led, 2.0f);

    // The pad: a chunky black rubber block seen slightly from above, so its
    // front edge shows as a darker band, sitting in its own shadow.
    const auto pressed = isMouseButtonDown() && switchArea.contains (getMouseXYRelative());
    auto pad = switchArea.toFloat();
    juce::Path padPath;
    padPath.addRoundedRectangle (pad, 7.0f);
    juce::DropShadow (juce::Colours::black.withAlpha (0.55f), 10, { 0, pressed ? 2 : 6 }).drawForPath (g, padPath);
    if (pressed)
        pad = pad.translated (0.0f, 2.0f);

    // One block of black rubber: the top face fades from a soft grey sheen
    // at the back edge to near black at the front, then a narrow, darker
    // front lip shows its thickness.
    const auto frontHeight = pad.getHeight() * (pressed ? 0.06f : 0.11f);
    g.setColour (juce::Colour (0xff020203));
    g.fillRoundedRectangle (pad, 8.0f);

    const auto top = pad.withTrimmedBottom (frontHeight);
    juce::ColourGradient rubber (juce::Colour (0xff3a393e), 0.0f, top.getY(), juce::Colour (0xff0b0b0d), 0.0f, top.getBottom(), false);
    rubber.addColour (0.25, juce::Colour (0xff222125));
    g.setGradientFill (rubber);
    g.fillRoundedRectangle (top, 8.0f);
    g.setColour (juce::Colours::white.withAlpha (0.2f));
    g.drawHorizontalLine ((int) top.getY() + 1, top.getX() + 8.0f, top.getRight() - 8.0f);   // back edge catching the light

    // Pink reflection along the bottom edge, and a red outline on hover.
    g.setColour (ParagonColours::glow.withAlpha (0.35f));
    g.drawHorizontalLine ((int) pad.getBottom() - 1, pad.getX() + 8.0f, pad.getRight() - 8.0f);
    const auto hover = isMouseOver() && switchArea.contains (getMouseXYRelative());
    g.setColour (hover ? ParagonColours::glow.withAlpha (0.8f) : juce::Colours::black);
    g.drawRoundedRectangle (pad, 7.0f, 1.0f);

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
