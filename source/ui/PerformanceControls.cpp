#include "PerformanceControls.h"

#include "Parameters.h"
#include "ui/ParagonLookAndFeel.h"
#include "ui/SlotWindow.h"

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
        g.setColour (on ? ParagonColours::accentDim.withAlpha (0.55f) : juce::Colour (0xff110e12));
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (on ? ParagonColours::accent.withAlpha (0.8f) : ParagonColours::panelEdge);
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }

    auto colour = on ? ParagonColours::accent : (highlighted ? ParagonColours::text : ParagonColours::text.withAlpha (0.85f));
    if (! isEnabled())
        colour = ParagonColours::textDim.withAlpha (0.45f);

    auto content = bounds.reduced (text.isEmpty() ? 4.0f : 8.0f, 0.0f);
    const auto iconSize = juce::jmin (18.0f, bounds.getHeight() * 0.5f);

    if (text.isEmpty())
    {
        Icons::draw (g, icon, content.withSizeKeepingCentre (iconSize, iconSize), colour, 1.6f, on);
        return;
    }

    const auto font = juce::FontOptions (juce::jmin (15.0f, bounds.getHeight() * 0.42f), on ? juce::Font::bold : juce::Font::plain);
    const auto textWidth = (float) juce::GlyphArrangement::getStringWidthInt (font, text);
    const auto gap = iconSize * 0.45f;

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
            textButton->getProperties().set (ParagonLookAndFeel::outlinedButtonProperty, true);
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

    // "EXPRESSION / ASSIGN" printed on the metal, with a ring LED that
    // lights while the pedal is assigned to something.
    auto title = getLocalBounds().removeFromTop (30).toFloat();
    g.setColour (ParagonColours::ink);
    g.setFont (juce::FontOptions (11.5f, juce::Font::bold));
    g.drawText ("EXPRESSION", title.removeFromTop (15.0f), juce::Justification::centred, false);
    g.setColour (ParagonColours::ink.withAlpha (0.3f));
    g.drawHorizontalLine ((int) title.getY(), title.getX() + 8.0f, title.getRight() - 8.0f);

    g.setColour (ParagonColours::inkDim);
    g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
    g.drawText ("ASSIGN", title, juce::Justification::centred, false);
    SlotWindow::drawRingLed (g, { title.getCentreX() + 30.0f, title.getCentreY() }, currentTarget != ExpressionTargets::off);
}

void ExpressionAssign::resized()
{
    auto area = getLocalBounds();
    if (style == Style::pedal)
    {
        area.removeFromTop (36);
        const auto rowHeight = area.getHeight() / (int) buttons.size();
        for (auto& button : buttons)
            button->setBounds (area.removeFromTop (rowHeight).withSizeKeepingCentre (juce::jmin (82, area.getWidth()), 20));
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
                "a MIDI expression pedal moves it too (CC 11, or the one chosen in SETTINGS). Assign it with the EXPRESSION buttons.");
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    startTimerHz (30);   // follows a MIDI pedal smoothly
    timerCallback();
    shownPosition = position;   // start where the pedal is, rather than gliding there
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

    // Glide the drawn treadle to the new position over a few frames (a
    // stepped MIDI pedal or a click then looks like a foot rocking it).
    if (std::abs (position - shownPosition) > 1.0e-3f)
    {
        shownPosition += (position - shownPosition) * 0.35f;
        if (std::abs (position - shownPosition) <= 1.0e-3f)
            shownPosition = position;
        repaint();
    }
}

juce::Rectangle<float> ExpressionPedal::trackArea() const
{
    // The black well the pads sit in; the label is printed below it.
    return getLocalBounds().toFloat().reduced (14.0f, 10.0f).withTrimmedBottom (52.0f);
}

void ExpressionPedal::paint (juce::Graphics& g)
{
    const auto well = trackArea();
    const auto active = target != ExpressionTargets::off;

    // The recess: darker chrome, shaded at the top where the housing's lip
    // overhangs it, with a light catch along its bottom edge.
    juce::Path wellPath;
    wellPath.addRoundedRectangle (well, 14.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff7a5664), 0.0f, well.getY(),
                                             juce::Colour (0xffd0aebd), 0.0f, well.getBottom(), false));
    g.fillPath (wellPath);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.drawRoundedRectangle (well, 14.0f, 1.5f);
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.drawRoundedRectangle (well.expanded (1.5f).translated (0.0f, 1.0f), 15.0f, 1.0f);

    // Two rubber pads split by a diagonal chrome strip, as in the mockup: a
    // short toe pad on top and a long heel pad below. As the pedal rocks
    // towards the toe the toe pad catches more light, towards the heel the
    // heel pad does.
    // The treadle rocks: towards the heel its toe end lifts towards you, so
    // seen from above it looks shorter (foreshortened) and the dark well
    // shows beyond its tip, in its shadow.
    const auto lift = 1.0f - shownPosition;
    const auto pads = well.reduced (9.0f).withTrimmedTop (well.getHeight() * 0.18f * lift);
    if (lift > 0.0f)
    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (wellPath);
        g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.0f), 0.0f, well.getY(),
                                                 juce::Colours::black.withAlpha (0.75f * lift), 0.0f, pads.getY(), false));
        g.fillRect (well.withBottom (pads.getY() + 4.0f));
    }
    const auto gap = 26.0f;
    const auto splitLeft = pads.getY() + pads.getHeight() * 0.40f;    // the strip climbs to the right
    const auto splitRight = pads.getY() + pads.getHeight() * 0.22f;

    juce::Path toe, heel;
    toe.startNewSubPath (pads.getX(), pads.getY());
    toe.lineTo (pads.getRight(), pads.getY());
    toe.lineTo (pads.getRight(), splitRight - gap * 0.5f);
    toe.lineTo (pads.getX(), splitLeft - gap * 0.5f);
    toe.closeSubPath();
    heel.startNewSubPath (pads.getX(), splitLeft + gap * 0.5f);
    heel.lineTo (pads.getRight(), splitRight + gap * 0.5f);
    heel.lineTo (pads.getRight(), pads.getBottom());
    heel.lineTo (pads.getX(), pads.getBottom());
    heel.closeSubPath();
    toe = toe.createPathWithRoundedCorners (9.0f);
    heel = heel.createPathWithRoundedCorners (9.0f);

    const auto drawPad = [&g] (const juce::Path& pad, float light)
    {
        // Each pad sits in its own shadow, with a lit bevel round the top.
        juce::DropShadow (juce::Colours::black.withAlpha (0.6f), 8, { 0, 4 }).drawForPath (g, pad);
        const auto box = pad.getBounds();
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff121115).brighter (0.1f * light), box.getX(), box.getY(),
                                                 juce::Colour (0xff020203), box.getX(), box.getBottom(), false));
        g.fillPath (pad);
        g.setColour (juce::Colours::white.withAlpha (0.12f + 0.15f * light));
        g.strokePath (pad, juce::PathStrokeType (1.4f));
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (pad);
        g.setColour (juce::Colours::white.withAlpha (0.03f));
        g.fillRect (box.withHeight (box.getHeight() * 0.25f));   // faint sheen on the upper part
    };
    drawPad (toe, shownPosition);
    drawPad (heel, 1.0f - shownPosition);

    // A soft red reflection from the unit's glow along the bottom of the heel pad.
    g.setColour (ParagonColours::glow.withAlpha (0.25f));
    g.drawHorizontalLine ((int) pads.getBottom() - 1, pads.getX() + 10.0f, pads.getRight() - 10.0f);

    // Below the well: what it controls, and the X1 badge embossed in the
    // chrome (as "RPX" in the mockup).
    auto label = getLocalBounds().toFloat().withTop (well.getBottom() + 4.0f);
    g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    g.setColour (active ? ParagonColours::glow.darker (0.1f) : ParagonColours::inkDim);
    const auto targetName = active ? juce::String (pedalNames[juce::jlimit (0, 2, target - 1)]) : juce::String ("NOT ASSIGNED");
    g.drawText (targetName + "  " + juce::String (juce::roundToInt (position * 100.0f)) + "%",
                label.removeFromTop (16.0f), juce::Justification::centred, false);

    // X1 stamped into the chrome as outlined letters, like the mockup's RPX.
    const auto badge = label.removeFromTop (34.0f);
    juce::GlyphArrangement glyphs;
    glyphs.addFittedText (juce::Font (juce::FontOptions (32.0f, juce::Font::bold | juce::Font::italic)), "X1",
                          badge.getX(), badge.getY(), badge.getWidth(), badge.getHeight(), juce::Justification::centred, 1);
    juce::Path letters;
    glyphs.createPath (letters);
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.strokePath (letters, juce::PathStrokeType (1.6f), juce::AffineTransform::translation (0.0f, 1.2f));
    g.setColour (juce::Colour (0xff8c7f87));
    g.strokePath (letters, juce::PathStrokeType (1.6f));
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
        reading.note = PitchDetector::toNote (reading.frequency, processor.getTunerReferenceHz());
    }

    if (onReading)
        onReading (reading);
}

//==============================================================================
HeelIndicator::HeelIndicator (ParagonX1AudioProcessor& processorToUse) : processor (processorToUse)
{
    setInterceptsMouseClicks (false, false);
    startTimerHz (15);
}

void HeelIndicator::timerCallback()
{
    const auto atHeel = processor.getExpressionPosition() < 0.005f;
    if (atHeel != lit)
    {
        lit = atHeel;
        repaint();
    }
}

void HeelIndicator::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    g.setColour (ParagonColours::inkDim);
    g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
    g.drawText ("MIN", area.withTrimmedRight (14.0f), juce::Justification::centredRight, false);
    SlotWindow::drawRingLed (g, { area.getRight() - 6.0f, area.getCentreY() }, lit);
}
