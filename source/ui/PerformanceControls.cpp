#include "PerformanceControls.h"

#include "Parameters.h"
#include "ui/ParagonLookAndFeel.h"

namespace
{
    const Icons::Type targetIcons[] = { Icons::Type::volume, Icons::Type::wah, Icons::Type::rig };
    const char* pedalNames[] = { "VOLUME", "WAH / FILTER", "MORPH / FX" };

    juce::RangedAudioParameter& parameter (juce::AudioProcessorValueTreeState& state, const char* id)
    {
        return *state.getParameter (id);
    }
}

//==============================================================================
IconButton::IconButton (const juce::String& name, Icons::Type iconType, const juce::String& buttonText,
                        juce::Justification textJustification)
    : juce::Button (name), icon (iconType), text (buttonText), justification (textJustification)
{
    setTooltip (name);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void IconButton::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    auto bounds = getLocalBounds().toFloat().reduced (0.5f);
    const auto on = getToggleState();

    if (on || outlined || down)
    {
        g.setColour (on ? ParagonColours::accentDim.withAlpha (0.55f) : juce::Colour (0xff161619));
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (on ? ParagonColours::accent.withAlpha (0.8f) : ParagonColours::panelEdge);
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }

    auto colour = on ? ParagonColours::accent : (highlighted ? ParagonColours::text : ParagonColours::text.withAlpha (0.85f));
    if (! isEnabled())
        colour = ParagonColours::textDim.withAlpha (0.45f);

    auto content = bounds.reduced (10.0f, 0.0f);
    const auto iconSize = juce::jmin (20.0f, bounds.getHeight() * 0.55f);

    if (text.isEmpty())
    {
        Icons::draw (g, icon, content.withSizeKeepingCentre (iconSize, iconSize), colour, 1.6f, on);
        return;
    }

    const auto font = juce::FontOptions (juce::jmin (15.0f, bounds.getHeight() * 0.42f), on ? juce::Font::bold : juce::Font::plain);
    const auto textWidth = (float) juce::GlyphArrangement::getStringWidthInt (font, text);
    const auto gap = iconSize * 0.6f;

    // Centred: icon and text together in the middle. Otherwise from the left.
    auto row = justification == juce::Justification::centred
                 ? content.withSizeKeepingCentre (juce::jmin (content.getWidth(), iconSize + gap + textWidth), content.getHeight())
                 : content;
    Icons::draw (g, icon, row.removeFromLeft (iconSize).withSizeKeepingCentre (iconSize, iconSize), colour, 1.6f, on);
    row.removeFromLeft (gap);
    g.setColour (colour);
    g.setFont (font);
    g.drawText (text, row, juce::Justification::centredLeft, true);
}

//==============================================================================
ExpressionAssign::ExpressionAssign (juce::AudioProcessorValueTreeState& state, Style lookStyle)
    : style (lookStyle),
      attachment (parameter (state, ParamIDs::expressionTarget), [this] (float value)
      {
          currentTarget = juce::roundToInt (value);
          for (size_t i = 0; i < buttons.size(); ++i)
              buttons[i]->setToggleState ((int) i + 1 == currentTarget, juce::dontSendNotification);
          repaint();
      })
{
    for (int target = ExpressionTargets::volume; target <= ExpressionTargets::morph; ++target)
    {
        const auto index = (size_t) (target - 1);
        std::unique_ptr<juce::Button> button;

        if (style == Style::list)
        {
            auto iconButton = std::make_unique<IconButton> (ExpressionTargets::names[target], targetIcons[index],
                                                            pedalNames[index],
                                                            juce::Justification::centredLeft);
            iconButton->setOutlined (true);
            button = std::move (iconButton);
        }
        else
        {
            auto textButton = std::make_unique<juce::TextButton> (pedalNames[index]);
            textButton->getProperties().set (ParagonLookAndFeel::pedalButtonProperty, true);
            button = std::move (textButton);
        }

        button->setTooltip ("Assign the expression pedal to " + ExpressionTargets::names[target]
                            + ". Click again to unassign it.");
        button->onClick = [this, target] { select (target); };
        addAndMakeVisible (*button);
        buttons.push_back (std::move (button));
    }

    attachment.sendInitialUpdate();
}

void ExpressionAssign::select (int target)
{
    // Clicking the lit one again turns the pedal off.
    attachment.setValueAsCompleteGesture ((float) (target == currentTarget ? ExpressionTargets::off : target));
}

void ExpressionAssign::paint (juce::Graphics& g)
{
    if (style != Style::pedal)
        return;

    // "EXPRESSION / ASSIGN" printed on the metal, with an LED that lights
    // while the pedal is assigned to something.
    auto title = getLocalBounds().removeFromTop (34).toFloat();
    g.setColour (ParagonColours::ink);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("EXPRESSION", title.removeFromTop (17.0f), juce::Justification::centred, false);
    g.setColour (ParagonColours::ink.withAlpha (0.25f));
    g.drawHorizontalLine ((int) title.getY(), title.getX() + 30.0f, title.getRight() - 30.0f);

    g.setColour (ParagonColours::inkDim);
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.drawText ("ASSIGN", title, juce::Justification::centred, false);

    const auto on = currentTarget != ExpressionTargets::off;
    const auto led = juce::Rectangle<float> (7.0f, 7.0f).withCentre ({ title.getCentreX() + 32.0f, title.getCentreY() });
    if (on)
    {
        g.setColour (ParagonColours::glow.withAlpha (0.35f));
        g.fillEllipse (led.expanded (3.0f));
    }
    g.setColour (on ? ParagonColours::glow : ParagonColours::glow.withAlpha (0.2f));
    g.fillEllipse (led);
}

void ExpressionAssign::resized()
{
    auto area = getLocalBounds();
    if (style == Style::pedal)
    {
        area.removeFromTop (40);
        const auto rowHeight = area.getHeight() / (int) buttons.size();
        for (auto& button : buttons)
            button->setBounds (area.removeFromTop (rowHeight).withSizeKeepingCentre (juce::jmin (124, area.getWidth()), 24));
    }
    else
    {
        const auto rowHeight = area.getHeight() / (int) buttons.size();
        for (auto& button : buttons)
            button->setBounds (area.removeFromTop (rowHeight).reduced (0, 3));
    }
}

//==============================================================================
ExpressionPedal::ExpressionPedal (ParagonX1AudioProcessor& processorToUse)
    : processor (processorToUse),
      attachment (parameter (processorToUse.getParameters(), ParamIDs::expression), [this] (float) { timerCallback(); })
{
    setTooltip ("Expression pedal. Drag up and down (or use the mouse wheel) to rock it from heel to toe; "
                "a MIDI expression pedal on CC 11 moves it too. Assign it with the EXPRESSION buttons.");
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    startTimerHz (30);   // follows a MIDI pedal smoothly
    timerCallback();
}

void ExpressionPedal::timerCallback()
{
    const auto newPosition = processor.getExpressionPosition();
    const auto newTarget = (int) processor.getParameters().getRawParameterValue (ParamIDs::expressionTarget)->load();
    if (std::abs (newPosition - position) > 1.0e-4f || newTarget != target)
    {
        position = newPosition;
        target = newTarget;
        repaint();
    }
}

juce::Rectangle<float> ExpressionPedal::trackArea() const
{
    return getLocalBounds().toFloat().reduced (24.0f, 30.0f).withTrimmedBottom (34.0f);
}

void ExpressionPedal::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced (6.0f);

    // Housing: the same silver metal and red-orange rim light as the unit.
    for (int i = 4; i >= 1; --i)
    {
        g.setColour (ParagonColours::glow.withAlpha (0.05f * (float) (5 - i)));
        g.drawRoundedRectangle (bounds.expanded ((float) i * 1.5f), 22.0f + (float) i, 3.0f);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe9e4e6), bounds.getTopLeft(),
                                             juce::Colour (0xff8c8589), bounds.getBottomLeft(), false));
    g.fillRoundedRectangle (bounds, 22.0f);
    const auto face = bounds.reduced (10.0f);
    g.setGradientFill (juce::ColourGradient (ParagonColours::metalLight, face.getTopLeft(),
                                             ParagonColours::metalDark, face.getBottomRight(), false));
    g.fillRoundedRectangle (face, 16.0f);
    g.setColour (ParagonColours::glow.withAlpha (0.55f));
    g.drawRoundedRectangle (bounds.reduced (1.0f), 22.0f, 1.5f);

    // The well the treadle rocks in.
    auto track = trackArea();
    g.setColour (juce::Colour (0xff0d0d0f));
    g.fillRoundedRectangle (track.expanded (6.0f), 12.0f);
    g.setColour (juce::Colours::white.withAlpha (0.5f));
    g.drawRoundedRectangle (track.expanded (6.0f).translated (0.0f, 1.0f), 12.0f, 1.0f);

    // The treadle. Seen from above, a toe-down pedal shows its whole length
    // and a heel-down one is tilted towards you, so it looks shorter, its
    // far end lit by the light on the front of the unit.
    const auto tilt = 1.0f - position;   // 0 = toe down
    auto treadle = track.withTrimmedTop (track.getHeight() * 0.28f * tilt);
    juce::Path pad;
    const auto inset = treadle.getWidth() * 0.08f * tilt;   // perspective: the far end narrows
    pad.startNewSubPath (treadle.getX() + inset, treadle.getY());
    pad.lineTo (treadle.getRight() - inset, treadle.getY());
    pad.lineTo (treadle.getRight(), treadle.getBottom());
    pad.lineTo (treadle.getX(), treadle.getBottom());
    pad.closeSubPath();
    pad = pad.createPathWithRoundedCorners (10.0f);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2e2d32).brighter (0.25f * tilt), treadle.getTopLeft(),
                                             juce::Colour (0xff101012), treadle.getBottomLeft(), false));
    g.fillPath (pad);

    // Rubber grip: angled ridges like the mockup's two-part pad.
    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (pad);
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        for (float y = treadle.getY() + 14.0f; y < treadle.getBottom(); y += 12.0f)
            g.drawLine (treadle.getX(), y + 10.0f, treadle.getRight(), y - 10.0f, 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.05f));
        for (float y = treadle.getY() + 16.0f; y < treadle.getBottom(); y += 12.0f)
            g.drawLine (treadle.getX(), y + 10.0f, treadle.getRight(), y - 10.0f, 1.0f);
    }
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.strokePath (pad, juce::PathStrokeType (1.0f));

    // Position LEDs in a dark slot down the left of the well, lit from the heel up.
    const auto active = target != ExpressionTargets::off;
    const int numLeds = 12;
    g.setColour (juce::Colour (0xff0d0d0f));
    g.fillRoundedRectangle (juce::Rectangle<float> (9.0f, track.getHeight() + 8.0f).withCentre ({ track.getX() - 12.0f, track.getCentreY() }), 4.0f);
    for (int i = 0; i < numLeds; ++i)
    {
        const auto fraction = (float) (i + 1) / (float) numLeds;
        const auto y = track.getBottom() - track.getHeight() * ((float) i + 0.5f) / (float) numLeds;
        const auto led = juce::Rectangle<float> (3.0f, track.getHeight() / (float) numLeds - 4.0f)
                             .withCentre ({ track.getX() - 12.0f, y });
        const auto lit = position >= fraction - 0.5f / (float) numLeds;
        g.setColour (lit ? (active ? ParagonColours::accent : ParagonColours::textDim) : ParagonColours::accent.withAlpha (0.1f));
        g.fillRoundedRectangle (led, 1.5f);
    }

    // Embossed name at the foot, as "RPX" in the mockup, and what it controls.
    auto label = getLocalBounds().toFloat().reduced (16.0f).removeFromBottom (36.0f);
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.setColour (active ? ParagonColours::glow.darker (0.2f) : ParagonColours::inkDim);
    const auto targetName = active ? juce::String (pedalNames[juce::jlimit (0, 2, target - 1)]) : juce::String ("NOT ASSIGNED");
    g.drawText (targetName + "  " + juce::String (juce::roundToInt (position * 100.0f)) + "%",
                label.removeFromTop (16.0f), juce::Justification::centred, false);
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold | juce::Font::italic));
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.drawText ("X1", label.translated (0.0f, 1.0f), juce::Justification::centred, false);
    g.setColour (ParagonColours::ink.withAlpha (0.45f));
    g.drawText ("X1", label, juce::Justification::centred, false);
}

void ExpressionPedal::mouseDown (const juce::MouseEvent&)
{
    dragStartPosition = position;
    attachment.beginGesture();
}

void ExpressionPedal::mouseDrag (const juce::MouseEvent& event)
{
    // Dragging the full height of the well rocks it heel to toe.
    const auto travel = juce::jmax (1.0f, trackArea().getHeight());
    attachment.setValueAsPartOfGesture (juce::jlimit (0.0f, 1.0f, dragStartPosition - (float) event.getDistanceFromDragStartY() / travel));
}

void ExpressionPedal::mouseUp (const juce::MouseEvent&)
{
    attachment.endGesture();
}

void ExpressionPedal::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    attachment.setValueAsCompleteGesture (juce::jlimit (0.0f, 1.0f, position + wheel.deltaY * 0.25f));
}

//==============================================================================
ExpressionPanel::ExpressionPanel (juce::AudioProcessorValueTreeState& state)
    : assign (state, ExpressionAssign::Style::list),
      minAttachment (parameter (state, ParamIDs::expressionMin), [this] (float value)
      {
          range.setMinValue (value, juce::dontSendNotification, true);
          updateRangeText();
      }),
      maxAttachment (parameter (state, ParamIDs::expressionMax), [this] (float value)
      {
          range.setMaxValue (value, juce::dontSendNotification, true);
          updateRangeText();
      })
{
    addAndMakeVisible (assign);

    range.setRange (0.0, 1.0, 0.01);
    range.setColour (juce::Slider::trackColourId, ParagonColours::accent);
    range.setColour (juce::Slider::backgroundColourId, ParagonColours::panelEdge);
    range.setColour (juce::Slider::thumbColourId, ParagonColours::accent);
    range.setTooltip ("Where the pedal's heel (MIN) and toe (MAX) land, so it can sweep just part of the range.");

    // Each drag is one edit (and one undo step), whichever end moves.
    range.onDragStart = [this] { minAttachment.beginGesture(); maxAttachment.beginGesture(); };
    range.onValueChange = [this]
    {
        minAttachment.setValueAsPartOfGesture ((float) range.getMinValue());
        maxAttachment.setValueAsPartOfGesture ((float) range.getMaxValue());
    };
    range.onDragEnd = [this] { minAttachment.endGesture(); maxAttachment.endGesture(); };
    addAndMakeVisible (range);

    for (auto* label : { &minLabel, &maxLabel })
    {
        label->setFont (juce::FontOptions (13.0f));
        label->setColour (juce::Label::textColourId, ParagonColours::textDim);
        addAndMakeVisible (*label);
    }
    maxLabel.setJustificationType (juce::Justification::centredRight);

    minAttachment.sendInitialUpdate();
    maxAttachment.sendInitialUpdate();
}

void ExpressionPanel::updateRangeText()
{
    const auto percent = [] (double value) { return juce::String (juce::roundToInt (value * 100.0)) + "%"; };
    minLabel.setText ("MIN   " + percent (range.getMinValue()), juce::dontSendNotification);
    maxLabel.setText ("MAX   " + percent (range.getMaxValue()), juce::dontSendNotification);
}

void ExpressionPanel::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setColour (ParagonColours::panel);
    g.fillRoundedRectangle (bounds, 8.0f);
    g.setColour (ParagonColours::panelEdge);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 8.0f, 1.0f);

    g.setColour (ParagonColours::text);
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("EXPRESSION ASSIGN", getLocalBounds().reduced (18, 0).removeFromTop (44), juce::Justification::centredLeft, false);
}

void ExpressionPanel::resized()
{
    auto area = getLocalBounds().reduced (16, 12);
    area.removeFromTop (34);
    auto bottom = area.removeFromBottom (52);
    assign.setBounds (area);

    auto labels = bottom.removeFromTop (24);
    minLabel.setBounds (labels.removeFromLeft (labels.getWidth() / 2));
    maxLabel.setBounds (labels);
    range.setBounds (bottom);
}

//==============================================================================
TunerReader::TunerReader (ParagonX1AudioProcessor& processorToUse) : processor (processorToUse) {}

void TunerReader::setActive (bool shouldBeActive)
{
    if (shouldBeActive == isActive())
        return;

    if (shouldBeActive)
    {
        smoothedNote = -1.0;
        startTimerHz (30);
    }
    else
    {
        stopTimer();
    }
}

void TunerReader::timerCallback()
{
    const auto sampleRate = processor.getInputTap().getSampleRate();
    if (sampleRate <= 0.0)
        return;

    if (! juce::exactlyEqual (sampleRate, preparedRate))
    {
        detector.prepare (sampleRate);
        preparedRate = sampleRate;
        window.resize ((size_t) juce::jmin (detector.windowSizeAtHostRate(), processor.getInputTap().capacity()));
    }

    processor.getInputTap().readLatest (window.data(), (int) window.size());
    const auto now = juce::Time::getMillisecondCounter();

    if (const auto result = detector.detect (window.data(), (int) window.size()))
    {
        const auto note = 69.0 + 12.0 * std::log2 (result->frequency / 440.0);

        // Steady the needle: ease towards small changes, but jump straight
        // to a new string rather than sliding across to it.
        if (smoothedNote < 0.0 || std::abs (note - smoothedNote) > 0.5)
            smoothedNote = note;
        else
            smoothedNote += (note - smoothedNote) * 0.35;
        lastHeard = now;
    }
    else if (now - lastHeard > 600)
    {
        smoothedNote = -1.0;   // the note has died away
    }

    Reading reading;
    if (smoothedNote >= 0.0)
    {
        reading.hasNote = true;
        reading.frequency = 440.0 * std::pow (2.0, (smoothedNote - 69.0) / 12.0);
        reading.note = PitchDetector::toNote (reading.frequency);
    }

    if (onReading)
        onReading (reading);
}
