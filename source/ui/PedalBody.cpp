#include "PedalBody.h"

#include "Logo.h"
#include "ParagonLookAndFeel.h"

namespace
{
    constexpr int textureScale = 2;
    constexpr float chassisRadius = 28.0f;
    constexpr float faceRadius = 16.0f;

}

PedalBody::PedalBody()
{
    // Everything here is static, so draw it once and reuse the picture.
    setBufferedToImage (true);
    setInterceptsMouseClicks (false, false);
}

void PedalBody::setGeometry (Geometry newGeometry)
{
    geometry = std::move (newGeometry);
    repaint();
}

void PedalBody::resized()
{
    makeSatinTexture();
}

void PedalBody::makeSatinTexture()
{
    // Pearl paint, not brushed metal: a fine speckle with no direction to it
    // (so no lines), and a sparse scatter of brighter flecks, like the mica
    // in a pearlescent finish catching the light. Very low alpha, so the
    // surface reads smooth and clean but not flat or plastic. A fixed random
    // seed keeps it identical every time. Twice the component's size, so
    // the grain stays fine on high-resolution screens.
    const auto width = juce::jmax (1, getWidth() * textureScale), height = juce::jmax (1, getHeight() * textureScale);
    satin = juce::Image (juce::Image::ARGB, width, height, true);
    juce::Random random (1234);
    juce::Image::BitmapData pixels (satin, juce::Image::BitmapData::writeOnly);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (random.nextInt (500) == 0)
            {
                // A fleck: white or faintly rose, a little brighter than the speckle.
                pixels.setPixelColour (x, y, (random.nextBool() ? juce::Colours::white : juce::Colour (0xffffd8e6)).withAlpha ((juce::uint8) (60 + random.nextInt (60))));
                continue;
            }
            const auto shade = random.nextFloat() * 2.0f - 1.0f;
            const auto alpha = (juce::uint8) juce::roundToInt (std::abs (shade) * 9.0f);
            pixels.setPixelColour (x, y, (shade > 0.0f ? juce::Colours::white : juce::Colour (0xff5a3a4a)).withAlpha (alpha));
        }
    }
}

void PedalBody::paint (juce::Graphics& g)
{
    auto chassis = getLocalBounds().toFloat().reduced (10.0f);
    juce::Path chassisPath;
    chassisPath.addRoundedRectangle (chassis, chassisRadius);

    // 1. Light spilling onto the dark background round the unit: soft and
    //    wide, strongest underneath, as in the mockup (no hard outline).
    juce::DropShadow (ParagonColours::glow.withAlpha (0.42f), 44, { 0, 14 }).drawForPath (g, chassisPath);
    juce::DropShadow (juce::Colour (0xffff3a5a).withAlpha (0.3f), 14, { 0, 3 }).drawForPath (g, chassisPath);

    // 2. The chassis: a thick pearl rim. Its front edge shows as a darker
    //    band along the bottom (the unit's thickness); the top surface of
    //    the rim runs from white where it faces the light to a soft rose
    //    shade lower down, enough range to read as a glossy curved finish
    //    without going grey.
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffb49ca8), 0.0f, chassis.getBottom() - 30.0f,
                                             juce::Colour (0xff5e4652), 0.0f, chassis.getBottom(), false));
    g.fillPath (chassisPath);

    const auto top = chassis.withTrimmedBottom (12.0f);
    juce::Path topPath;
    topPath.addRoundedRectangle (top, chassisRadius);
    {
        juce::ColourGradient vertical (juce::Colours::white, 0.0f, top.getY(), juce::Colour (0xffc9b2be), 0.0f, top.getBottom(), false);
        vertical.addColour (0.02, juce::Colour (0xfff3e9ee));
        vertical.addColour (0.06, juce::Colour (0xffe2d3db));
        vertical.addColour (0.45, juce::Colour (0xffeee3e9));
        vertical.addColour (0.9, juce::Colour (0xffdccbd4));
        g.setGradientFill (vertical);
        g.fillPath (topPath);

        // The rim's sides curve away: bright where they catch the light,
        // dark just inside.
        juce::ColourGradient banding (juce::Colour (0xff4a2a3a).withAlpha (0.3f), top.getX(), 0.0f,
                                      juce::Colour (0xff4a2a3a).withAlpha (0.3f), top.getRight(), 0.0f, false);
        banding.addColour (0.006, juce::Colours::white.withAlpha (0.9f));
        banding.addColour (0.018, juce::Colour (0xff4a2a3a).withAlpha (0.16f));
        banding.addColour (0.5, juce::Colours::white.withAlpha (0.12f));
        banding.addColour (0.982, juce::Colour (0xff4a2a3a).withAlpha (0.16f));
        banding.addColour (0.994, juce::Colours::white.withAlpha (0.85f));
        g.setGradientFill (banding);
        g.fillPath (topPath);

        juce::Graphics::ScopedSaveState clip (g);
        g.reduceClipRegion (topPath);
        drawSatin (g);
    }

    // Crisp machined edges: a bright line along the top of the rim, the
    // reflected red along its bottom, and a dark outline all round.
    strokeTopHighlight (g, top.reduced (1.0f), chassisRadius, 1.4f, 1.0f);
    g.setColour (juce::Colour (0xffff8a8a).withAlpha (0.7f));
    g.drawHorizontalLine ((int) top.getBottom() - 1, top.getX() + chassisRadius, top.getRight() - chassisRadius);
    g.setColour (juce::Colour (0xff2a1420).withAlpha (0.7f));
    g.drawRoundedRectangle (chassis.reduced (0.5f), chassisRadius, 1.0f);

    // 3. The face plate, sunk into the rim: bright pearl white with a satin finish.
    const auto face = geometry.face;
    if (! face.isEmpty())
    {
        juce::Path facePath;
        facePath.addRoundedRectangle (face, faceRadius);

        // The step down from the rim: a shadow just inside the rim, a dark
        // groove, and a bright lip where the rim's edge catches the light.
        juce::DropShadow (juce::Colour (0xff3a1a2a).withAlpha (0.4f), 8, { 0, -1 }).drawForPath (g, facePath);
        g.setColour (juce::Colours::white.withAlpha (0.75f));
        g.drawRoundedRectangle (face.expanded (2.5f).translated (0.0f, 1.5f), faceRadius + 2.0f, 1.0f);

        // Pearl white, lit from above: nearly pure white at the top, a soft
        // band where it reflects the room, brighter again lower down. The
        // shading stays warm (rose, not grey) so it sits with the neon.
        juce::ColourGradient pearl (juce::Colour (0xfffffdfe), face.getX(), face.getY(),
                                    juce::Colour (0xffefe3e9), face.getX(), face.getBottom(), false);
        pearl.addColour (0.18, juce::Colour (0xfffaf4f7));
        pearl.addColour (0.42, juce::Colour (0xfff1e7ec));
        pearl.addColour (0.7, juce::Colour (0xfff8f1f4));
        g.setGradientFill (pearl);
        g.fillPath (facePath);

        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (facePath);
        drawSatin (g);

        // Pearlescence: a faint shift of hue across the plate, rose at the
        // left, peach in the middle, lilac at the right, the way pearl
        // paint changes colour with the angle you see it from.
        {
            juce::ColourGradient iridescence (juce::Colour (0xffffc8dc).withAlpha (0.16f), face.getX(), face.getY(),
                                              juce::Colour (0xffe2d0ff).withAlpha (0.14f), face.getRight(), face.getBottom(), false);
            iridescence.addColour (0.5, juce::Colour (0xffffe0c8).withAlpha (0.12f));
            g.setGradientFill (iridescence);
            g.fillRect (face);
        }

        // A sharp diagonal sheen across the upper left, like a studio light
        // in a glossy finish: bright at its core, fading quickly either side.
        juce::ColourGradient sheen (juce::Colours::white.withAlpha (0.0f), face.getX(), face.getY(),
                                    juce::Colours::white.withAlpha (0.0f), face.getX() + face.getWidth() * 0.45f, face.getBottom(), false);
        sheen.addColour (0.3, juce::Colours::white.withAlpha (0.05f));
        sheen.addColour (0.38, juce::Colours::white.withAlpha (0.45f));
        sheen.addColour (0.46, juce::Colours::white.withAlpha (0.05f));
        g.setGradientFill (sheen);
        g.fillRect (face);

        // Vignette: the plate falls off a little towards its edges and
        // corners, which is what makes it look lit rather than flat. Rose,
        // not black, and light, so the corners stay pearl rather than grey.
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff8a4a66).withAlpha (0.0f), face.getCentreX(), face.getY() + face.getHeight() * 0.4f,
                                                 juce::Colour (0xff8a4a66).withAlpha (0.16f), face.getX() - face.getWidth() * 0.04f, face.getY(), true));
        g.fillRect (face);

        // The glow under the unit catches only the plate's bottom edge.
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffff9ab3).withAlpha (0.0f), 0.0f, face.getBottom() - 26.0f,
                                                 juce::Colour (0xffff9ab3).withAlpha (0.4f), 0.0f, face.getBottom(), false));
        g.fillRect (face.withTop (face.getBottom() - 26.0f));
    }

    // 4. Section panels (the slot row and the footswitch row), set down into
    //    the face: the face's edge casts a shadow along their top, the
    //    panel's own lower lip catches the light, and their pearl is a
    //    touch deeper and more even than the face around them.
    for (const auto& section : geometry.sections)
    {
        juce::Path panel;
        panel.addRoundedRectangle (section, 12.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffeee3e9), 0.0f, section.getY(),
                                                 juce::Colour (0xfff9f3f6), 0.0f, section.getBottom(), false));
        g.fillPath (panel);
        {
            juce::Graphics::ScopedSaveState state (g);
            g.reduceClipRegion (panel);
            drawSatin (g);

            // Inner shadow along the top and down the sides, from the face's edge.
            for (int ring = 1; ring <= 5; ++ring)
            {
                g.setColour (juce::Colour (0xff4a2034).withAlpha (0.11f - 0.019f * (float) ring));
                g.drawRoundedRectangle (section.translated (0.0f, (float) ring * 1.2f).reduced ((float) ring * 0.6f), 12.0f, 1.6f);
            }
        }
        // Dark crisp edge where the face drops away, bright lip at the bottom.
        g.setColour (juce::Colour (0xff3a2a32).withAlpha (0.55f));
        g.drawRoundedRectangle (section, 12.0f, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        {
            juce::Graphics::ScopedSaveState bottomOnly (g);
            g.reduceClipRegion (section.expanded (2.0f).withTrimmedTop (section.getHeight() - 6.0f).toNearestInt());
            g.drawRoundedRectangle (section.expanded (1.0f), 13.0f, 1.0f);
        }
    }

    // 5. Recessed panels behind the slot windows: engraved, with a dark
    //    groove and a light lower edge.
    for (const auto& inset : geometry.insets)
    {
        g.setColour (juce::Colour (0xff8a4a66).withAlpha (0.06f));
        g.fillRoundedRectangle (inset, 8.0f);
        g.setColour (juce::Colour (0xff2a1c24).withAlpha (0.5f));
        g.drawRoundedRectangle (inset, 8.0f, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.drawRoundedRectangle (inset.translated (0.0f, 1.0f).expanded (0.5f), 8.5f, 1.0f);
    }

    // 6. Screws in the face's corners: recessed hex heads.
    if (! face.isEmpty())
    {
        const auto inset = 14.0f;
        for (const auto centre : { face.getTopLeft().translated (inset, inset), face.getTopRight().translated (-inset, inset),
                                   face.getBottomLeft().translated (inset, -inset), face.getBottomRight().translated (-inset, -inset) })
            paintScrew (g, centre, 5.0f);
    }

    // 7. The expression pedal's bay: a deeper pearl housing, more shaded
    //    than the face because it curves away, split from it by a dark seam. The pedal component draws the pedal.
    if (const auto bay = geometry.pedalBay; ! bay.isEmpty())
    {
        juce::Path bayPath;
        bayPath.addRoundedRectangle (bay, 22.0f);
        juce::DropShadow (juce::Colours::black.withAlpha (0.55f), 8, { -2, 3 }).drawForPath (g, bayPath);
        juce::ColourGradient housing (juce::Colour (0xfff2e6ec), bay.getX(), 0.0f, juce::Colour (0xffc8b0bd), bay.getRight(), 0.0f, false);
        housing.addColour (0.05, juce::Colours::white);
        housing.addColour (0.22, juce::Colour (0xffece0e6));
        housing.addColour (0.7, juce::Colour (0xffdcc9d2));
        g.setGradientFill (housing);
        g.fillPath (bayPath);
        {
            juce::Graphics::ScopedSaveState clip (g);
            g.reduceClipRegion (bayPath);
            drawSatin (g);
        }
        g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.0f), 0.0f, bay.getY(),
                                                 juce::Colour (0xff4a2034).withAlpha (0.14f), 0.0f, bay.getBottom(), false));
        g.fillPath (bayPath);
        strokeTopHighlight (g, bay.reduced (1.5f), 20.0f, 1.0f, 1.0f);
        g.setColour (juce::Colours::black.withAlpha (0.65f));
        g.drawRoundedRectangle (bay, 22.0f, 1.0f);
    }

    g.setColour (ParagonColours::glow.darker (0.1f));
    g.strokePath (geometry.printedRed, juce::PathStrokeType (1.2f));
    g.setColour (ParagonColours::ink.withAlpha (0.8f));
    g.strokePath (geometry.printedInk, juce::PathStrokeType (1.2f));

    paintBadge (g);
}

void PedalBody::strokeTopHighlight (juce::Graphics& g, juce::Rectangle<float> area, float radius, float thickness, float alpha)
{
    // Light catching an edge from above: full strength along the top,
    // fading smoothly down the sides to nothing by about 40% of the way
    // down, rather than cut off at a hard line.
    juce::Path edge;
    edge.addRoundedRectangle (area, radius);
    juce::ColourGradient fade (juce::Colours::white.withAlpha (alpha), 0.0f, area.getY(),
                               juce::Colours::white.withAlpha (0.0f), 0.0f, area.getY() + area.getHeight() * 0.42f, false);
    fade.addColour (0.15, juce::Colours::white.withAlpha (alpha * 0.85f));
    g.setGradientFill (fade);
    g.strokePath (edge, juce::PathStrokeType (thickness));
}

void PedalBody::drawSatin (juce::Graphics& g) const
{
    g.drawImage (satin, getLocalBounds().toFloat());
}

void PedalBody::paintScrew (juce::Graphics& g, juce::Point<float> centre, float radius)
{
    // A countersunk screw: a dark ring where it sits in the plate, a domed
    // chrome head lit from the top left, and a hex socket.
    const auto head = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre);
    g.setColour (juce::Colours::white.withAlpha (0.8f));
    g.drawEllipse (head.expanded (1.2f).translated (0.0f, 0.8f), 1.0f);
    g.setColour (juce::Colour (0xff2a1c24).withAlpha (0.7f));
    g.drawEllipse (head.expanded (0.6f), 1.2f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff6eef2), head.getX(), head.getY(),
                                             juce::Colour (0xff7d6a74), head.getRight(), head.getBottom(), false));
    g.fillEllipse (head);

    juce::Path socket;
    socket.addPolygon (centre, 6, radius * 0.48f, juce::MathConstants<float>::pi / 6.0f);
    g.setColour (juce::Colour (0xff1e1418));
    g.fillPath (socket);
    g.setColour (juce::Colours::white.withAlpha (0.5f));
    g.strokePath (socket, juce::PathStrokeType (0.6f), juce::AffineTransform::translation (0.3f, 0.5f));
}

void PedalBody::paintBadge (juce::Graphics& g) const
{
    // Heavy italic PARAGON, as "CYBER RPX" in the mockup, over the PR7-G mark.
    if (geometry.badge.isEmpty())
        return;

    auto area = geometry.badge;
    const auto lineHeight = area.getHeight() * 0.38f;
    const auto font = juce::FontOptions (lineHeight * 1.12f, juce::Font::bold | juce::Font::italic);

    // A faint white line under the lettering makes it look stamped into the metal.
    const auto drawStamped = [&g] (const juce::String& text, juce::Rectangle<float> where, juce::Colour colour)
    {
        g.setColour (juce::Colours::white.withAlpha (0.7f));
        g.drawText (text, where.translated (0.0f, 1.2f), juce::Justification::centredLeft, false);
        g.setColour (colour);
        g.drawText (text, where, juce::Justification::centredLeft, false);
    };

    g.setFont (font);
    drawStamped ("PARAGON", area.removeFromTop (lineHeight), ParagonColours::ink);

    // The PR7-G mark under it, as tall as the lettering's capitals, starting
    // where they do (the slant makes the P's foot sit under PARAGON's P).
    auto secondLine = area.removeFromTop (lineHeight);
    Logo::drawMark (g, secondLine.withSizeKeepingCentre (secondLine.getWidth(), lineHeight * 0.82f).withTrimmedLeft (lineHeight * 0.04f),
                   juce::Justification::centredLeft,
                   { ParagonColours::ink, ParagonColours::glow, juce::Colours::white.withAlpha (0.7f) });

    g.setFont (juce::FontOptions (lineHeight * 0.27f, juce::Font::bold));
    g.setColour (ParagonColours::glow.darker (0.15f));
    g.drawText ("NEURAL MULTI-FX PROCESSOR", area.withTrimmedTop (4.0f).withHeight (lineHeight * 0.4f),
                juce::Justification::centredLeft, false);
}
