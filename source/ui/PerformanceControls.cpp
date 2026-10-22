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
    return getLocalBounds().toFloat().reduced (10.0f, 8.0f).withTrimmedBottom (50.0f);
}

void ExpressionPedal::paint (juce::Graphics& g)
{
    const auto well = trackArea();
    const auto active = target != ExpressionTargets::off;

    // The base the treadle rocks on: a dark recess in the chrome bay.
    juce::Path wellPath;
    wellPath.addRoundedRectangle (well, 14.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1a1417), 0.0f, well.getY(),
                                             juce::Colour (0xff3a2c33), 0.0f, well.getBottom(), false));
    g.fillPath (wellPath);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawRoundedRectangle (well, 14.0f, 1.5f);
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.drawRoundedRectangle (well.expanded (1.5f).translated (0.0f, 1.0f), 15.0f, 1.0f);

    //==========================================================================
    // The treadle, in 3D, the way Neural DSP and IK Multimedia show theirs:
    // a slab pivoting on an axle across its middle, seen from above through
    // a perspective camera. As it rocks, the end that lifts comes towards
    // you (larger, and moving outwards), the end that drops goes away
    // (smaller), its side walls show, and its shadow shifts on the base.
    //
    // World units are pixels. X runs across the pedal, Y along it (toe at
    // -Y, heel at +Y), Z up towards the camera.
    constexpr float maxTilt = 0.22f;                                   // about 13 degrees each way
    const auto tilt = (shownPosition * 2.0f - 1.0f) * maxTilt;         // + = toe down
    const auto length = well.getHeight() * 0.82f, width = well.getWidth() * 0.86f;
    const auto thickness = width * 0.2f;
    const auto pivotZ = thickness * 1.6f;                              // axle height above the base
    const auto cameraZ = length * 1.9f;                                // camera height: sets how strong the perspective is
    const auto centre = well.getCentre().translated (0.0f, well.getHeight() * 0.035f);   // down a little: the lifted toe rises towards the top
    const auto flatScale = (cameraZ - (pivotZ + thickness * 0.5f)) / cameraZ;   // a flat treadle's top fills its space

    struct Point3 { float x, y, z; };
    const auto rotate = [&] (Point3 p)
    {
        // Rock about the axle (the X axis at height pivotZ): toe down = toe's Z falls.
        const auto c = std::cos (tilt), s = std::sin (tilt);
        const auto dz = p.z - pivotZ;
        return Point3 { p.x, p.y * c - dz * s, pivotZ + p.y * s + dz * c };
    };
    // The camera sits above and a little in front of the pedal (towards
    // the player), as the mockup is drawn, so the treadle's front wall and
    // thickness show. Scaled so a flat treadle's top fills its space.
    const auto cameraY = length * 0.85f;
    const auto project = [&] (Point3 p)
    {
        const auto k = cameraZ / (cameraZ - p.z) * flatScale;
        return juce::Point<float> (centre.x + p.x * k, centre.y + (p.y - cameraY) * k + cameraY * flatScale);
    };
    // A point on the treadle's top (u across 0..1, v along 0..1 from toe to heel).
    const auto topPoint = [&] (float u, float v, float zOffset = 0.0f)
    {
        return project (rotate ({ (u - 0.5f) * width, (v - 0.5f) * length, pivotZ + thickness * 0.5f + zOffset }));
    };
    // A straight-edged shape in (u, v), sampled along its edges so perspective bends it correctly.
    const auto mapPolygon = [&] (std::initializer_list<juce::Point<float>> corners, float zOffset = 0.0f)
    {
        std::vector<juce::Point<float>> uv (corners);
        juce::Path path;
        for (size_t i = 0; i < uv.size(); ++i)
        {
            const auto a = uv[i], b = uv[(i + 1) % uv.size()];
            for (int step = 0; step < 8; ++step)
            {
                const auto t = (float) step / 8.0f;
                const auto p = topPoint (a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, zOffset);
                if (i == 0 && step == 0)
                    path.startNewSubPath (p);
                else
                    path.lineTo (p);
            }
        }
        path.closeSubPath();
        return path;
    };

    // Shadow on the base, cast by light from above and slightly in front:
    // it spreads further under whichever end is lifted.
    {
        juce::Path shadow;
        const auto onBase = [&] (float u, float v)
        {
            const auto p = rotate ({ (u - 0.5f) * width, (v - 0.5f) * length, pivotZ - thickness * 0.5f });
            return project ({ p.x + p.z * 0.15f, p.y + p.z * 0.35f, 0.0f });
        };
        shadow.startNewSubPath (onBase (0, 0));
        shadow.lineTo (onBase (1, 0));
        shadow.lineTo (onBase (1, 1));
        shadow.lineTo (onBase (0, 1));
        shadow.closeSubPath();
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (wellPath);
        juce::DropShadow (juce::Colours::black.withAlpha (0.85f), 14, {}).drawForPath (g, shadow);
    }

    // Side walls: each a quad between the top's edge and the bottom's edge.
    // Drawn before the top, which covers whatever faces away from the camera.
    const auto corner = [&] (float u, float v, bool topFace)
    {
        return project (rotate ({ (u - 0.5f) * width, (v - 0.5f) * length, pivotZ + (topFace ? 0.5f : -0.5f) * thickness }));
    };
    const std::array<std::pair<juce::Point<float>, juce::Point<float>>, 4> edges {
        std::pair<juce::Point<float>, juce::Point<float>> { { 0, 0 }, { 1, 0 } },   // toe end
        { { 1, 0 }, { 1, 1 } },                                                     // right side
        { { 1, 1 }, { 0, 1 } },                                                     // heel end
        { { 0, 1 }, { 0, 0 } },                                                     // left side
    };
    for (size_t i = 0; i < edges.size(); ++i)
    {
        const auto [a, b] = edges[i];
        juce::Path wall;
        wall.startNewSubPath (corner (a.x, a.y, true));
        wall.lineTo (corner (b.x, b.y, true));
        wall.lineTo (corner (b.x, b.y, false));
        wall.lineTo (corner (a.x, a.y, false));
        wall.closeSubPath();
        // Chrome walls: the ends catch a little more light than the sides.
        const auto shade = i % 2 == 0 ? juce::Colour (0xff6e5f67) : juce::Colour (0xff4a3e45);
        g.setGradientFill (juce::ColourGradient (shade.brighter (0.3f), corner (a.x, a.y, true), shade.darker (0.6f), corner (a.x, a.y, false), false));
        g.fillPath (wall);
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.strokePath (wall, juce::PathStrokeType (0.8f));
    }

    // The top: a chrome plate. It brightens as it tips its face towards the
    // light (toe down) and darkens tipped away.
    const auto plate = mapPolygon ({ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } }).createPathWithRoundedCorners (10.0f);
    const auto light = 0.5f + tilt / maxTilt * 0.5f;
    {
        const auto plateBounds = plate.getBounds();
        juce::ColourGradient chrome (juce::Colour (0xffe8d6de).darker (0.35f - 0.25f * light), plateBounds.getX(), 0.0f,
                                     juce::Colour (0xff8d7380).darker (0.25f - 0.2f * light), plateBounds.getRight(), 0.0f, false);
        chrome.addColour (0.12, juce::Colours::white.withAlpha (1.0f).darker (0.15f - 0.1f * light));
        chrome.addColour (0.45, juce::Colour (0xffc9b2bd).darker (0.3f - 0.2f * light));
        g.setGradientFill (chrome);
        g.fillPath (plate);
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.strokePath (plate, juce::PathStrokeType (1.0f));
    }

    // Two piano-black rubber pads on the plate, split by a diagonal strip of
    // chrome (as in the mockup), mapped through the same perspective.
    const auto toe = mapPolygon ({ { 0.07f, 0.04f }, { 0.93f, 0.04f }, { 0.93f, 0.2f }, { 0.07f, 0.37f } }, 1.0f).createPathWithRoundedCorners (8.0f);
    const auto heel = mapPolygon ({ { 0.07f, 0.45f }, { 0.93f, 0.28f }, { 0.93f, 0.96f }, { 0.07f, 0.96f } }, 1.0f).createPathWithRoundedCorners (8.0f);
    for (const auto* pad : { &toe, &heel })
    {
        const auto box = pad->getBounds();
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1d1c21), 0.0f, box.getY(), juce::Colour (0xff040405), 0.0f, box.getBottom(), false));
        g.fillPath (*pad);

        // Gloss: a soft reflection that slides along the pad as it tilts,
        // which sells the movement as much as the shape does.
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (*pad);
        const auto glossY = box.getY() + box.getHeight() * (0.15f + 0.35f * (1.0f - light));
        juce::ColourGradient band (juce::Colours::white.withAlpha (0.0f), 0.0f, glossY - box.getHeight() * 0.22f,
                                   juce::Colours::white.withAlpha (0.0f), 0.0f, glossY + box.getHeight() * 0.22f, false);
        band.addColour (0.5, juce::Colours::white.withAlpha (0.1f));
        g.setGradientFill (band);
        g.fillRect (box);
    }
    for (const auto* pad : { &toe, &heel })
    {
        g.setColour (juce::Colours::white.withAlpha (0.25f));
        g.strokePath (*pad, juce::PathStrokeType (1.0f));
    }

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
