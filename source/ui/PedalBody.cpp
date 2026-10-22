#include "PedalBody.h"

#include "ParagonLookAndFeel.h"

namespace
{
    constexpr float chassisRadius = 28.0f;
    constexpr float faceRadius = 16.0f;

    // The mockup's metal has a rose tint from the red light around it.
    const juce::Colour chromeLight { 0xfff7f1f4 }, chromeMid { 0xffd8ccd2 }, chromeDark { 0xff8f8189 }, chromeDeep { 0xff574c53 };
    const juce::Colour faceLight { 0xfff4eef1 }, faceMid { 0xffe0d6db }, faceShade { 0xffcdbfc6 };
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
    // Brushed metal is lots of fine horizontal streaks, slightly lighter or
    // darker than the base. A fixed random seed keeps it identical every time.
    const auto width = juce::jmax (1, getWidth()), height = juce::jmax (1, getHeight());
    brushed = juce::Image (juce::Image::ARGB, width, height, true);
    juce::Graphics g (brushed);
    juce::Random random (1234);

    for (int i = 0; i < height * 4; ++i)
    {
        const auto y = random.nextFloat() * (float) height;
        const auto x = random.nextFloat() * (float) width * 1.2f - (float) width * 0.1f;
        const auto length = (float) width * (0.05f + random.nextFloat() * 0.4f);
        const auto light = random.nextBool();
        g.setColour ((light ? juce::Colours::white : juce::Colour (0xff5a3a48)).withAlpha (0.02f + random.nextFloat() * 0.045f));
        g.drawHorizontalLine ((int) y, x, x + length);
    }
}

void PedalBody::paint (juce::Graphics& g)
{
    const auto chassis = getLocalBounds().toFloat().reduced (10.0f);
    juce::Path chassisPath;
    chassisPath.addRoundedRectangle (chassis, chassisRadius);

    // 1. Red-orange light spilling onto the dark background round the unit,
    //    strongest underneath, as in the mockup.
    juce::DropShadow (ParagonColours::glow.withAlpha (0.55f), 26, { 0, 6 }).drawForPath (g, chassisPath);
    juce::DropShadow (juce::Colour (0xffff2a55).withAlpha (0.35f), 10, { 0, 2 }).drawForPath (g, chassisPath);

    // 2. The chassis: polished chrome, bright on top, darkening towards the
    //    bottom edge, with the vertical banding of a curved reflective surface.
    {
        juce::ColourGradient vertical (chromeLight, 0.0f, chassis.getY(), chromeDeep, 0.0f, chassis.getBottom(), false);
        vertical.addColour (0.12, chromeLight);
        vertical.addColour (0.45, chromeMid);
        vertical.addColour (0.85, chromeDark);
        g.setGradientFill (vertical);
        g.fillPath (chassisPath);

        juce::ColourGradient banding (juce::Colours::white.withAlpha (0.0f), chassis.getX(), 0.0f,
                                      juce::Colours::white.withAlpha (0.0f), chassis.getRight(), 0.0f, false);
        banding.addColour (0.02, juce::Colours::white.withAlpha (0.35f));
        banding.addColour (0.05, juce::Colours::black.withAlpha (0.12f));
        banding.addColour (0.5, juce::Colours::white.withAlpha (0.06f));
        banding.addColour (0.95, juce::Colours::black.withAlpha (0.12f));
        banding.addColour (0.98, juce::Colours::white.withAlpha (0.3f));
        g.setGradientFill (banding);
        g.fillPath (chassisPath);
    }

    // Rim edges: a white catch-light along the top and a neon seam round it.
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.drawRoundedRectangle (chassis.reduced (1.5f).withTrimmedBottom (chassis.getHeight() * 0.5f), chassisRadius, 1.2f);
    g.setColour (ParagonColours::glow.withAlpha (0.75f));
    g.drawRoundedRectangle (chassis.reduced (0.5f), chassisRadius, 1.6f);

    // 3. The face plate, slightly sunk into the chassis.
    const auto face = geometry.face;
    if (! face.isEmpty())
    {
        juce::Path facePath;
        facePath.addRoundedRectangle (face, faceRadius);

        // Groove round it: dark above (in shadow), light below (catching light).
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawRoundedRectangle (face.expanded (2.0f), faceRadius + 2.0f, 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.drawRoundedRectangle (face.expanded (3.5f).translated (0.0f, 1.0f), faceRadius + 3.0f, 1.0f);

        juce::ColourGradient metal (faceLight, face.getX(), face.getY(), faceShade, face.getRight(), face.getBottom(), false);
        metal.addColour (0.45, faceMid);
        g.setGradientFill (metal);
        g.fillPath (facePath);

        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (facePath);
        g.drawImageAt (brushed, 0, 0);

        // Pink light reflected along the inside edges, strongest at the
        // bottom where the glow underneath bounces back up.
        g.setColour (ParagonColours::glow.withAlpha (0.16f));
        g.drawRoundedRectangle (face.reduced (3.0f), faceRadius - 2.0f, 8.0f);
        g.setGradientFill (juce::ColourGradient (ParagonColours::glow.withAlpha (0.0f), 0.0f, face.getBottom() - 70.0f,
                                                 ParagonColours::glow.withAlpha (0.22f), 0.0f, face.getBottom(), false));
        g.fillRect (face.withTop (face.getBottom() - 70.0f));
    }

    // 4. Raised section panels: a soft drop shadow, a lighter top, and an
    //    edge lit pink, like the mockup's slot and footswitch rows.
    for (const auto& section : geometry.sections)
    {
        juce::Path panel;
        panel.addRoundedRectangle (section, 12.0f);
        juce::DropShadow (juce::Colour (0xff6a3a4c).withAlpha (0.28f), 8, { 0, 3 }).drawForPath (g, panel);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff6f0f3), 0.0f, section.getY(),
                                                 juce::Colour (0xffe2d7dc), 0.0f, section.getBottom(), false));
        g.fillPath (panel);
        {
            juce::Graphics::ScopedSaveState state (g);
            g.reduceClipRegion (panel);
            g.setOpacity (0.6f);
            g.drawImageAt (brushed, 0, 0);
        }
        g.setColour (ParagonColours::glow.withAlpha (0.25f));
        g.drawRoundedRectangle (section.reduced (1.5f), 11.0f, 2.5f);
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.drawRoundedRectangle (section.reduced (0.5f), 12.0f, 1.0f);
    }

    // 5. Recessed panels behind the slot windows: engraved outlines.
    for (const auto& inset : geometry.insets)
    {
        g.setColour (juce::Colour (0xff5a3a48).withAlpha (0.05f));
        g.fillRoundedRectangle (inset, 8.0f);
        g.setColour (juce::Colour (0xff3a2a32).withAlpha (0.28f));
        g.drawRoundedRectangle (inset, 8.0f, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.drawRoundedRectangle (inset.translated (0.0f, 1.0f), 8.0f, 1.0f);
    }

    // 6. The expression pedal's bay: a deeper chrome housing split from the
    //    face by a dark seam. The pedal component draws the treadle in it.
    if (const auto bay = geometry.pedalBay; ! bay.isEmpty())
    {
        juce::Path bayPath;
        bayPath.addRoundedRectangle (bay, 20.0f);
        juce::DropShadow (juce::Colours::black.withAlpha (0.35f), 6, { -2, 2 }).drawForPath (g, bayPath);
        juce::ColourGradient housing (chromeLight, bay.getX(), 0.0f, chromeDark, bay.getRight(), 0.0f, false);
        housing.addColour (0.08, juce::Colours::white);
        housing.addColour (0.3, chromeMid);
        housing.addColour (0.85, chromeLight.darker (0.15f));
        g.setGradientFill (housing);
        g.fillPath (bayPath);
        g.setColour (ParagonColours::glow.withAlpha (0.5f));
        g.drawRoundedRectangle (bay.reduced (0.5f), 20.0f, 1.2f);
        g.setColour (juce::Colours::white.withAlpha (0.8f));
        g.drawRoundedRectangle (bay.reduced (2.0f).withTrimmedBottom (bay.getHeight() * 0.6f), 18.0f, 1.0f);
    }

    g.setColour (ParagonColours::glow.darker (0.1f));
    g.strokePath (geometry.printedRed, juce::PathStrokeType (1.2f));
    g.setColour (ParagonColours::ink.withAlpha (0.8f));
    g.strokePath (geometry.printedInk, juce::PathStrokeType (1.2f));

    paintBadge (g);
}

void PedalBody::paintBadge (juce::Graphics& g) const
{
    // Heavy italic product name, as "CYBER RPX" in the mockup.
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
    auto secondLine = area.removeFromTop (lineHeight);
    drawStamped ("X", secondLine, ParagonColours::ink);
    const auto xWidth = (float) juce::GlyphArrangement::getStringWidthInt (juce::Font (font), "X");
    drawStamped ("1", secondLine.withTrimmedLeft (xWidth + 2.0f), ParagonColours::glow);

    g.setFont (juce::FontOptions (lineHeight * 0.27f, juce::Font::bold));
    g.setColour (ParagonColours::glow.darker (0.15f));
    g.drawText ("NEURAL MULTI-FX PROCESSOR", area.withTrimmedTop (4.0f).withHeight (lineHeight * 0.4f),
                juce::Justification::centredLeft, false);
}
