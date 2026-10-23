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
    makeBrushedTexture();
}

void PedalBody::makeBrushedTexture()
{
    // Brushed metal: every row of pixels is a slightly different shade (the
    // grain left by the brushing), with longer, brighter and darker streaks
    // over it and a fine speckle, all running left to right. A fixed random
    // seed keeps it identical every time. It's an overlay: white and dark
    // pixels at low alpha over whatever colour the metal is.
    // Twice the component's size, so the grain stays fine on high-resolution screens.
    const auto width = juce::jmax (1, getWidth() * textureScale), height = juce::jmax (1, getHeight() * textureScale);
    brushed = juce::Image (juce::Image::ARGB, width, height, true);
    juce::Random random (1234);
    {
        juce::Image::BitmapData pixels (brushed, juce::Image::BitmapData::writeOnly);
        for (int y = 0; y < height; ++y)
        {
            const auto rowShade = random.nextFloat() * 2.0f - 1.0f;   // the grain of this row
            for (int x = 0; x < width; ++x)
            {
                const auto shade = rowShade * 0.75f + (random.nextFloat() * 2.0f - 1.0f) * 0.25f;
                const auto alpha = (juce::uint8) juce::roundToInt (std::abs (shade) * 20.0f);
                pixels.setPixelColour (x, y, (shade > 0.0f ? juce::Colours::white : juce::Colour (0xff2a1c24)).withAlpha (alpha));
            }
        }
    }

    juce::Graphics g (brushed);
    for (int i = 0; i < height * 3; ++i)
    {
        const auto y = (float) random.nextInt (height);
        const auto x = random.nextFloat() * (float) width * 1.2f - (float) width * 0.1f;
        const auto length = (float) width * (0.03f + random.nextFloat() * 0.3f);
        const auto light = random.nextBool();
        g.setColour ((light ? juce::Colours::white : juce::Colour (0xff2a1c24)).withAlpha (0.02f + random.nextFloat() * 0.05f));
        g.drawHorizontalLine ((int) y, x, x + length);
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

    // 2. The chassis: a thick polished rim. Its front edge shows as a dark
    //    band along the bottom (the unit's thickness); the top surface of
    //    the rim is chrome with a full range from near-white to deep shadow,
    //    which is what makes metal read as polished rather than painted.
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6e5f67), 0.0f, chassis.getBottom() - 30.0f,
                                             juce::Colour (0xff2e2429), 0.0f, chassis.getBottom(), false));
    g.fillPath (chassisPath);

    const auto top = chassis.withTrimmedBottom (12.0f);
    juce::Path topPath;
    topPath.addRoundedRectangle (top, chassisRadius);
    {
        juce::ColourGradient vertical (juce::Colour (0xfff4ecf0), 0.0f, top.getY(), juce::Colour (0xff5a4c54), 0.0f, top.getBottom(), false);
        vertical.addColour (0.02, juce::Colour (0xffb8a5af));
        vertical.addColour (0.06, juce::Colour (0xff8a7580));
        vertical.addColour (0.45, juce::Colour (0xff806d77));
        vertical.addColour (0.9, juce::Colour (0xff6a5862));
        g.setGradientFill (vertical);
        g.fillPath (topPath);

        // The rim's sides curve away: bright where they catch the light,
        // dark just inside.
        juce::ColourGradient banding (juce::Colours::black.withAlpha (0.35f), top.getX(), 0.0f,
                                      juce::Colours::black.withAlpha (0.35f), top.getRight(), 0.0f, false);
        banding.addColour (0.006, juce::Colour (0xffffe3ea).withAlpha (0.75f));
        banding.addColour (0.018, juce::Colours::black.withAlpha (0.3f));
        banding.addColour (0.5, juce::Colours::white.withAlpha (0.08f));
        banding.addColour (0.982, juce::Colours::black.withAlpha (0.3f));
        banding.addColour (0.994, juce::Colour (0xffffe3ea).withAlpha (0.7f));
        g.setGradientFill (banding);
        g.fillPath (topPath);

        juce::Graphics::ScopedSaveState clip (g);
        g.reduceClipRegion (topPath);
        drawBrushed (g);
    }

    // Crisp machined edges: a bright line along the top of the rim, the
    // reflected red along its bottom, and a dark outline all round.
    g.setColour (juce::Colours::white);
    {
        juce::Graphics::ScopedSaveState topOnly (g);   // the top edge only: clip, so the corners don't curl in halfway down
        g.reduceClipRegion (top.reduced (1.0f).withHeight (top.reduced (1.0f).getHeight() * 0.45f).toNearestInt());
        g.drawRoundedRectangle (top.reduced (1.0f), chassisRadius, 1.4f);
    }
    g.setColour (juce::Colour (0xffff8a8a).withAlpha (0.7f));
    g.drawHorizontalLine ((int) top.getBottom() - 1, top.getX() + chassisRadius, top.getRight() - chassisRadius);
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawRoundedRectangle (chassis.reduced (0.5f), chassisRadius, 1.0f);

    // 3. The face plate, sunk into the rim: brushed, rose-tinted silver.
    const auto face = geometry.face;
    if (! face.isEmpty())
    {
        juce::Path facePath;
        facePath.addRoundedRectangle (face, faceRadius);

        // The step down from the rim: a shadow just inside the rim, a dark
        // groove, and a bright lip where the rim's edge catches the light.
        juce::DropShadow (juce::Colours::black.withAlpha (0.55f), 8, { 0, -1 }).drawForPath (g, facePath);
        g.setColour (juce::Colours::white.withAlpha (0.75f));
        g.drawRoundedRectangle (face.expanded (2.5f).translated (0.0f, 1.5f), faceRadius + 2.0f, 1.0f);

        // Polished metal reflects its surroundings, so it isn't one shade:
        // bright under the light at the top, a darker band where it reflects
        // the room, lighter again lower down.
        juce::ColourGradient metal (juce::Colour (0xfff0e8ec), face.getX(), face.getY(),
                                    juce::Colour (0xffcdbcc5), face.getX(), face.getBottom(), false);
        metal.addColour (0.18, juce::Colour (0xffe1d5db));
        metal.addColour (0.42, juce::Colour (0xffc7b6bf));
        metal.addColour (0.7, juce::Colour (0xffd9cbd2));
        g.setGradientFill (metal);
        g.fillPath (facePath);

        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (facePath);
        drawBrushed (g);

        // A sharp diagonal sheen across the upper left, like a studio light
        // in polished metal: bright at its core, fading quickly either side.
        juce::ColourGradient sheen (juce::Colours::white.withAlpha (0.0f), face.getX(), face.getY(),
                                    juce::Colours::white.withAlpha (0.0f), face.getX() + face.getWidth() * 0.45f, face.getBottom(), false);
        sheen.addColour (0.3, juce::Colours::white.withAlpha (0.05f));
        sheen.addColour (0.38, juce::Colours::white.withAlpha (0.28f));
        sheen.addColour (0.46, juce::Colours::white.withAlpha (0.05f));
        g.setGradientFill (sheen);
        g.fillRect (face);

        // Vignette: the plate darkens towards its edges and corners, which
        // is much of what makes it look like a lit piece of metal.
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a1420).withAlpha (0.0f), face.getCentreX(), face.getY() + face.getHeight() * 0.4f,
                                                 juce::Colour (0xff2a1420).withAlpha (0.32f), face.getX() - face.getWidth() * 0.04f, face.getY(), true));
        g.fillRect (face);

        // The glow under the unit catches only the plate's bottom edge.
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffff9ab3).withAlpha (0.0f), 0.0f, face.getBottom() - 26.0f,
                                                 juce::Colour (0xffff9ab3).withAlpha (0.4f), 0.0f, face.getBottom(), false));
        g.fillRect (face.withTop (face.getBottom() - 26.0f));
    }

    // 4. Section panels (the slot row and the footswitch row), set down into
    //    the face: the face's edge casts a shadow along their top, the
    //    panel's own lower lip catches the light, and their metal is a
    //    touch darker and more even than the face around them.
    for (const auto& section : geometry.sections)
    {
        juce::Path panel;
        panel.addRoundedRectangle (section, 12.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffd2c4cb), 0.0f, section.getY(),
                                                 juce::Colour (0xffdfd3d9), 0.0f, section.getBottom(), false));
        g.fillPath (panel);
        {
            juce::Graphics::ScopedSaveState state (g);
            g.reduceClipRegion (panel);
            drawBrushed (g);

            // Inner shadow along the top and down the sides, from the face's edge.
            for (int ring = 1; ring <= 5; ++ring)
            {
                g.setColour (juce::Colour (0xff2a1820).withAlpha (0.13f - 0.022f * (float) ring));
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
        g.setColour (juce::Colour (0xff3a2a32).withAlpha (0.08f));
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

    // 7. The expression pedal's bay: a deeper, darker chrome housing split
    //    from the face by a dark seam. The pedal component draws the pedal.
    if (const auto bay = geometry.pedalBay; ! bay.isEmpty())
    {
        juce::Path bayPath;
        bayPath.addRoundedRectangle (bay, 22.0f);
        juce::DropShadow (juce::Colours::black.withAlpha (0.55f), 8, { -2, 3 }).drawForPath (g, bayPath);
        juce::ColourGradient housing (juce::Colour (0xffe6cfd9), bay.getX(), 0.0f, juce::Colour (0xff9e7486), bay.getRight(), 0.0f, false);
        housing.addColour (0.05, juce::Colour (0xfffbeef3));
        housing.addColour (0.22, juce::Colour (0xffd2acbc));
        housing.addColour (0.7, juce::Colour (0xffb98a9d));
        g.setGradientFill (housing);
        g.fillPath (bayPath);
        {
            juce::Graphics::ScopedSaveState clip (g);
            g.reduceClipRegion (bayPath);
            drawBrushed (g);
        }
        g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.0f), 0.0f, bay.getY(),
                                                 juce::Colours::black.withAlpha (0.2f), 0.0f, bay.getBottom(), false));
        g.fillPath (bayPath);
        g.setColour (juce::Colours::white);
        {
            juce::Graphics::ScopedSaveState topOnly (g);   // the top edge only: clip, so the corners don't curl in halfway down
            g.reduceClipRegion (bay.reduced (1.5f).withHeight (bay.reduced (1.5f).getHeight() * 0.4f).toNearestInt());
            g.drawRoundedRectangle (bay.reduced (1.5f), 20.0f, 1.0f);
        }
        g.setColour (juce::Colours::black.withAlpha (0.65f));
        g.drawRoundedRectangle (bay, 22.0f, 1.0f);
    }

    g.setColour (ParagonColours::glow.darker (0.1f));
    g.strokePath (geometry.printedRed, juce::PathStrokeType (1.2f));
    g.setColour (ParagonColours::ink.withAlpha (0.8f));
    g.strokePath (geometry.printedInk, juce::PathStrokeType (1.2f));

    paintBadge (g);
}

void PedalBody::drawBrushed (juce::Graphics& g) const
{
    g.drawImage (brushed, getLocalBounds().toFloat());
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
    // Heavy italic PARAGON, as "CYBER RPX" in the mockup, over the PR7 mark.
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

    // The PR7 mark under it, as tall as the lettering's capitals, starting
    // where they do (the slant makes the P's foot sit under PARAGON's P).
    auto secondLine = area.removeFromTop (lineHeight);
    Logo::drawPR7 (g, secondLine.withSizeKeepingCentre (secondLine.getWidth(), lineHeight * 0.82f).withTrimmedLeft (lineHeight * 0.04f),
                   juce::Justification::centredLeft,
                   { ParagonColours::ink, ParagonColours::glow, juce::Colours::white.withAlpha (0.7f) });

    g.setFont (juce::FontOptions (lineHeight * 0.27f, juce::Font::bold));
    g.setColour (ParagonColours::glow.darker (0.15f));
    g.drawText ("NEURAL MULTI-FX PROCESSOR", area.withTrimmedTop (4.0f).withHeight (lineHeight * 0.4f),
                juce::Justification::centredLeft, false);
}
