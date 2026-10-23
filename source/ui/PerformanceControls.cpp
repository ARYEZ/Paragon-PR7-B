#include "PerformanceControls.h"

#include "Parameters.h"
#include "ui/Logo.h"
#include "ui/ParagonLookAndFeel.h"
#include "ui/SlotWindow.h"


namespace
{
    const Icons::Type targetIcons[] = { Icons::Type::volume, Icons::Type::wah, Icons::Type::rig, Icons::Type::filterPitch, Icons::Type::loFi };
    const char* pedalNames[] = { "VOLUME", "WAH / FILTER", "MORPH / FX", "PITCH / WHAMMY", "LO-FI" };

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

    const auto font = juce::FontOptions (juce::jmin (15.0f, bounds.getHeight() * 0.55f), on ? juce::Font::bold : juce::Font::plain);
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
ExpressionAssign::ExpressionAssign (juce::AudioProcessorValueTreeState& stateToUse, Style lookStyle)
    : style (lookStyle),
      state (stateToUse),
      attachment (parameter (stateToUse, ParamIDs::expressionTarget), [this] (float value)
      {
          currentTarget = juce::roundToInt (value);
          for (size_t i = 0; i < buttons.size(); ++i)
              buttons[i]->setToggleState ((int) i + 1 == currentTarget, juce::dontSendNotification);
          repaint();
      })
{
    for (int target = ExpressionTargets::volume; target <= ExpressionTargets::loFi; ++target)
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
    const auto newTarget = target == currentTarget ? ExpressionTargets::off : target;

    const auto set = [this] (const char* id, float plainValue)
    {
        auto* parameter = state.getParameter (id);
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (plainValue));
        parameter->endChangeGesture();
    };

    // PITCH / WHAMMY bends the pitch shifter, so make sure there's one
    // running: switch the FILTER / PITCH block on as a pitch shifter.
    if (newTarget == ExpressionTargets::pitch)
    {
        set (ParamIDs::fpOn, 1.0f);
        set (ParamIDs::fpType, (float) effects::FilterPitch::Type::pitchShifter);
    }

    // LO-FI fades the LO-FI block in, so switch it on.
    if (newTarget == ExpressionTargets::loFi)
        set (ParamIDs::lofiOn, 1.0f);

    attachment.setValueAsCompleteGesture ((float) newTarget);
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
            button->setBounds (area.removeFromTop (rowHeight).withSizeKeepingCentre (juce::jmin (82, area.getWidth()),
                                                                                     juce::jmin (20, rowHeight - 3)));
    }
    else
    {
        const auto rowHeight = area.getHeight() / (int) buttons.size();
        for (auto& button : buttons)
            button->setBounds (area.removeFromTop (rowHeight).reduced (0, 2));
    }
}

namespace
{
    // The pedal's layout, in units of the base's size (211 x 490, the
    // proportions of the mockup's pedal): the treadle covers the whole base,
    // just inside its chrome edge, as on a real floor unit, so the base only
    // shows where the treadle rocks away from it.
    constexpr float housingWidth = 211.0f, housingHeight = 490.0f;
    const juce::Rectangle<float> treadleInHousing { 4.0f, 4.0f, 203.0f, 482.0f };
    constexpr float baseCorner = 16.0f, treadleCorner = 13.0f;
    constexpr int treadleOversample = 2;   // the treadle's face is drawn at twice its size, so it stays sharp when rocked

    // The treadle's face, drawn once: a brushed chrome plate in the unit's
    // own rose-tinted chrome, two black rubber pads split by a diagonal
    // chrome band (the mockup's design), ribbed across like the DigiTech
    // RP6's, and the PR7 mark engraved at the heel. Drawn square and true,
    // so it rocks without any lean or warp.
    juce::Image makeTreadleFace (int w, int h)
    {
        juce::Image face (juce::Image::ARGB, w, h, true);
        juce::Graphics g (face);
        const auto bounds = face.getBounds().toFloat();
        const auto fw = (float) w, fh = (float) h;
        const auto corner = treadleCorner * fw / treadleInHousing.getWidth();

        juce::Path plate;
        plate.addRoundedRectangle (bounds.reduced (1.0f), corner);
        {
            juce::ColourGradient metal (ParagonColours::metalDark, 0.0f, 0.0f, ParagonColours::metalDark, fw, 0.0f, false);
            metal.addColour (0.14, ParagonColours::metalLight);
            metal.addColour (0.5, ParagonColours::metalMid);
            metal.addColour (0.86, ParagonColours::metalLight);
            g.setGradientFill (metal);
            g.fillPath (plate);

            // Brushed along its length: fine streaks of light and shade.
            juce::Graphics::ScopedSaveState clip (g);
            g.reduceClipRegion (plate);
            juce::Random random (7);
            for (int x = 0; x < w; ++x)
            {
                const auto shade = random.nextFloat();
                g.setColour ((shade > 0.5f ? juce::Colours::white : juce::Colours::black).withAlpha (0.05f * std::abs (shade - 0.5f) * 2.0f));
                g.fillRect ((float) x, 0.0f, 1.0f, fh);
            }
        }

        // The unit's rose tint, so it reads as the same chrome.
        g.setColour (ParagonColours::glow.withAlpha (0.07f));
        g.fillPath (plate);

        // Its bevelled edge: lit along the top and left, darker at the bottom right.
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.9f), bounds.getTopLeft(),
                                                 juce::Colour (0xff5d4c55), bounds.getBottomRight(), false));
        g.strokePath (plate, juce::PathStrokeType (fw * 0.012f));
        juce::Path seam;
        seam.addRoundedRectangle (bounds.reduced (fw * 0.03f), corner * 0.8f);
        g.setColour (juce::Colours::black.withAlpha (0.25f));
        g.strokePath (seam, juce::PathStrokeType (1.5f));

        // The two rubber pads, with the diagonal chrome band between them.
        const auto margin = fw * 0.075f;
        const auto addPad = [&] (std::initializer_list<juce::Point<float>> points)
        {
            juce::Path pad;
            auto first = true;
            for (auto point : points)
            {
                if (first)
                    pad.startNewSubPath (point);
                else
                    pad.lineTo (point);
                first = false;
            }
            pad.closeSubPath();
            pad = pad.createPathWithRoundedCorners (fw * 0.05f);

            // Set into the plate: a dark shadow line round it, deepest along the top.
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            g.strokePath (pad, juce::PathStrokeType (fw * 0.014f), juce::AffineTransform::translation (0.0f, -1.0f));
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1a1618), 0.0f, pad.getBounds().getY(),
                                                     juce::Colour (0xff090708), 0.0f, pad.getBounds().getBottom(), false));
            g.fillPath (pad);

            // Ribs across it, each with a lit top edge and a shadow under it.
            juce::Graphics::ScopedSaveState clip (g);
            g.reduceClipRegion (pad);
            const auto pitch = fh * 0.0125f;
            for (auto y = pad.getBounds().getY() + pitch * 0.5f; y < pad.getBounds().getBottom(); y += pitch)
            {
                g.setColour (juce::Colours::white.withAlpha (0.07f));
                g.fillRect (0.0f, y, fw, pitch * 0.18f);
                g.setColour (juce::Colours::black.withAlpha (0.45f));
                g.fillRect (0.0f, y + pitch * 0.18f, fw, pitch * 0.22f);
            }

            // The unit's red glow caught along the pad's edge.
            g.setColour (ParagonColours::glow.withAlpha (0.28f));
            g.strokePath (pad, juce::PathStrokeType (1.2f));
        };
        addPad ({ { margin, margin }, { fw - margin, margin }, { fw - margin, fh * 0.2f }, { margin, fh * 0.47f } });
        addPad ({ { fw - margin, fh * 0.28f }, { fw - margin, fh * 0.865f }, { margin, fh * 0.865f }, { margin, fh * 0.55f } });

        // The PR7 mark engraved on the plate at the heel.
        const auto markHeight = fh * 0.04f;
        Logo::drawPR7 (g, juce::Rectangle<float> (Logo::widthFor (markHeight), markHeight).withCentre ({ fw * 0.5f, fh * 0.928f }),
                       juce::Justification::centred,
                       { juce::Colour (0xfff7f3f5), juce::Colour (0xfff7f3f5), juce::Colours::white.withAlpha (0.6f),
                         juce::Colour (0xff3a3236), 1.6f });
        return face;
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
    treadle = makeTreadleFace (juce::roundToInt (treadleInHousing.getWidth()) * treadleOversample,
                               juce::roundToInt (treadleInHousing.getHeight()) * treadleOversample);
    setRepaintsOnMouseActivity (true);   // shows the pedal's position while you hover
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


juce::Rectangle<float> ExpressionPedal::housingArea() const
{
    // The housing fills the component's width and sits at its bottom; the
    // space above is headroom for the toe to rise into when the heel's down.
    const auto bounds = getLocalBounds().toFloat();
    return bounds.withTop (bounds.getBottom() - bounds.getWidth() * housingHeight / housingWidth);
}

juce::Rectangle<float> ExpressionPedal::trackArea() const
{
    // Where the treadle lies when it's flat.
    const auto housingBounds = housingArea();
    return (treadleInHousing * (housingBounds.getWidth() / housingWidth)) + housingBounds.getPosition();
}

void ExpressionPedal::paint (juce::Graphics& g)
{
    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);

    //==========================================================================
    // The treadle, rocking in 3D the way Neural DSP and
    // IK Multimedia show theirs, and as a real floor unit's pedal does: it
    // covers the whole base and pivots on an axle across its middle. It's
    // kept within the base's footprint, so it never spills over the unit.
    // The rocking shows as one end dropping away (narrower and darker, the
    // base showing round it) and the other coming up (lit, its shadow
    // spreading across the base).
    //
    // World units are pixels. X runs across the pedal, Y along it (toe at
    // -Y, heel at +Y), Z up towards the camera.
    const auto flat = trackArea();
    const auto scale = housingArea().getWidth() / housingWidth;
    juce::Path base;
    base.addRoundedRectangle (housingArea().reduced (1.0f), baseCorner * scale);

    // The base: dark, with a chrome lip round its edge. It shows only round
    // the treadle's edge and where the treadle rocks away from it.
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1d171a), 0.0f, housingArea().getY(),
                                             juce::Colour (0xff0b090a), 0.0f, housingArea().getBottom(), false));
    g.fillPath (base);
    g.setGradientFill (juce::ColourGradient (ParagonColours::metalLight, housingArea().getTopLeft(),
                                             ParagonColours::metalDark.darker (0.4f), housingArea().getBottomRight(), false));
    g.strokePath (base, juce::PathStrokeType (2.0f * scale));

    constexpr float maxTilt = 0.16f;                                   // about 9 degrees each way
    const auto tilt = (shownPosition * 2.0f - 1.0f) * maxTilt;         // + = toe down
    const auto length = flat.getHeight(), width = flat.getWidth();
    const auto thickness = width * 0.07f;   // the treadle's body under its face
    const auto pivotZ = thickness * 1.2f;                              // axle height above the base
    const auto cameraZ = length * 3.0f;                                // camera height: how strong the perspective is
    const auto cameraY = length * 0.6f;                                // camera a little towards the player
    const auto centre = flat.getCentre();
    const auto topZ = pivotZ + thickness * 0.5f;
    const auto flatScale = (cameraZ - topZ) / cameraZ;                 // so a flat treadle lands exactly on the artwork's place

    struct Point3 { float x, y, z; };
    const auto rotate = [&] (Point3 p)
    {
        const auto c = std::cos (tilt), s = std::sin (tilt);
        const auto dz = p.z - pivotZ;
        return Point3 { p.x, p.y * c - dz * s, pivotZ + p.y * s + dz * c };
    };
    const auto project = [&] (Point3 p)
    {
        const auto k = cameraZ / (cameraZ - p.z) * flatScale;
        return juce::Point<float> (centre.x + p.x * k, centre.y + (p.y - cameraY) * k + cameraY * flatScale);
    };
    const auto corner = [&] (float u, float v, float z)
    {
        return project (rotate ({ (u - 0.5f) * width, (v - 0.5f) * length, z }));
    };

    // The treadle's outline (0-1 across and along it): square and true.
    const std::array<juce::Point<float>, 4> outline { juce::Point<float> { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

    {
        juce::Graphics::ScopedSaveState clipToBase (g);
        g.reduceClipRegion (base);

        // The treadle's shadow on the base, spreading under whichever end lifts.
        {
            juce::Path shadow;
            const auto onBase = [&] (float u, float v)
            {
                const auto p = rotate ({ (u - 0.5f) * width, (v - 0.5f) * length, pivotZ - thickness * 0.5f });
                return project ({ p.x + p.z * 0.1f, p.y + p.z * 0.3f, 0.0f });
            };
            shadow.startNewSubPath (onBase (outline[0].x, outline[0].y));
            for (size_t i = 1; i < outline.size(); ++i)
                shadow.lineTo (onBase (outline[i].x, outline[i].y));
            shadow.closeSubPath();
            juce::DropShadow (juce::Colours::black, 14, {}).drawForPath (g, shadow);
        }

        // The treadle's sides (its thickness): chrome, lit along the top.
        for (size_t i = 0; i < outline.size(); ++i)
        {
            const auto a = outline[i], b = outline[(i + 1) % outline.size()];
            juce::Path wall;
            wall.startNewSubPath (corner (a.x, a.y, topZ));
            wall.lineTo (corner (b.x, b.y, topZ));
            wall.lineTo (corner (b.x, b.y, topZ - thickness));
            wall.lineTo (corner (a.x, a.y, topZ - thickness));
            wall.closeSubPath();
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffcdb6c1), corner (a.x, a.y, topZ),
                                                     juce::Colour (0xff3e3037), corner (a.x, a.y, topZ - thickness), false));
            g.fillPath (wall);
        }

        // Its face, drawn in thin horizontal slices: each slice is
        // placed and sized where the tilted treadle puts it, which bends the
        // whole picture through the perspective.
        const auto sourceHeight = treadle.getHeight();
        constexpr int sliceRows = 2;
        for (int row = 0; row < sourceHeight; row += sliceRows)
        {
            const auto rows = juce::jmin (sliceRows, sourceHeight - row);
            const auto v0 = (float) row / (float) sourceHeight, v1 = (float) (row + rows) / (float) sourceHeight;
            const auto topLeft = corner (0, v0, topZ), topRight = corner (1, v0, topZ);
            const auto bottomLeft = corner (0, v1, topZ), bottomRight = corner (1, v1, topZ);
            const auto x0 = (topLeft.x + bottomLeft.x) * 0.5f, x1 = (topRight.x + bottomRight.x) * 0.5f;
            const auto y0 = topLeft.y, y1 = bottomLeft.y + 0.6f;   // a hair of overlap hides the seams
            const auto slice = treadle.getClippedImage ({ 0, row, treadle.getWidth(), rows });
            g.drawImageTransformed (slice, juce::AffineTransform::scale ((x1 - x0) / (float) treadle.getWidth(), (y1 - y0) / (float) rows)
                                               .translated (x0, y0));
        }

        // Light from above and in front: the end tipped up towards it
        // brightens and the end tipped away darkens, along the treadle.
        {
            juce::Path face;
            face.startNewSubPath (corner (outline[0].x, outline[0].y, topZ));
            for (size_t i = 1; i < outline.size(); ++i)
                face.lineTo (corner (outline[i].x, outline[i].y, topZ));
            face.closeSubPath();
            const auto amount = tilt / maxTilt;   // +1 toe down (heel up), -1 heel down (toe up)
            const auto toeEnd = corner (0.5f, 0.0f, topZ), heelEnd = corner (0.5f, 1.0f, topZ);
            const auto lit = juce::Colours::white.withAlpha (0.10f * std::abs (amount));
            const auto shaded = juce::Colours::black.withAlpha (0.35f * std::abs (amount));
            g.setGradientFill (juce::ColourGradient (amount < 0.0f ? lit : shaded, toeEnd,
                                                     amount < 0.0f ? shaded : lit, heelEnd, false));
            g.fillPath (face.createPathWithRoundedCorners (treadleCorner * scale));
        }
    }

    // While you hover or drag: what the pedal controls and where it is.
    if (isMouseOverOrDragging())
    {
        const auto active = target != ExpressionTargets::off;
        const auto text = (active ? juce::String (pedalNames[juce::jlimit (0, (int) std::size (pedalNames) - 1, target - 1)]) : juce::String ("NOT ASSIGNED"))
                          + "  " + juce::String (juce::roundToInt (position * 100.0f)) + "%";
        const auto font = juce::FontOptions (11.0f, juce::Font::bold);
        const auto pill = juce::Rectangle<float> ((float) juce::GlyphArrangement::getStringWidthInt (font, text) + 18.0f, 20.0f)
                              .withCentre (flat.getCentre());
        g.setColour (juce::Colours::black.withAlpha (0.75f));
        g.fillRoundedRectangle (pill, 10.0f);
        g.setColour (active ? ParagonColours::accent : ParagonColours::text);
        g.setFont (font);
        g.drawText (text, pill, juce::Justification::centred, false);
    }
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
