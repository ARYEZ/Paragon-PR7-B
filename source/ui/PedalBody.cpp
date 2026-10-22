#include "PedalBody.h"

#include "ParagonLookAndFeel.h"

namespace
{
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
        g.setColour ((light ? juce::Colours::white : juce::Colour (0xff4a3a42)).withAlpha (0.025f + random.nextFloat() * 0.06f));
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

    // 2. The chassis. Its front edge shows as a darker band along the bottom
    //    (the unit's thickness), then the top surface is polished chrome:
    //    bright along the top, greyer lower down, with the light and dark
    //    vertical banding of a curved reflective edge.
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6b5d65), 0.0f, chassis.getBottom() - 30.0f,
                                             juce::Colour (0xff2e252b), 0.0f, chassis.getBottom(), false));
    g.fillPath (chassisPath);

    const auto top = chassis.withTrimmedBottom (12.0f);
    juce::Path topPath;
    topPath.addRoundedRectangle (top, chassisRadius);
    {
        juce::ColourGradient vertical (juce::Colour (0xfff2edf0), 0.0f, top.getY(), juce::Colour (0xff8e8088), 0.0f, top.getBottom(), false);
        vertical.addColour (0.08, juce::Colour (0xffe4dde1));
        vertical.addColour (0.5, juce::Colour (0xffbdb2b8));
        vertical.addColour (0.88, juce::Colour (0xffa3959d));
        g.setGradientFill (vertical);
        g.fillPath (topPath);

        juce::ColourGradient banding (juce::Colours::black.withAlpha (0.18f), top.getX(), 0.0f,
                                      juce::Colours::black.withAlpha (0.18f), top.getRight(), 0.0f, false);
        banding.addColour (0.012, juce::Colours::white.withAlpha (0.55f));
        banding.addColour (0.03, juce::Colours::black.withAlpha (0.1f));
        banding.addColour (0.5, juce::Colours::white.withAlpha (0.05f));
        banding.addColour (0.97, juce::Colours::black.withAlpha (0.1f));
        banding.addColour (0.988, juce::Colours::white.withAlpha (0.45f));
        g.setGradientFill (banding);
        g.fillPath (topPath);
    }

    // Rim catch-lights: white along the top edge, pink (reflected glow)
    // along the bottom edge of the top surface.
    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.drawRoundedRectangle (top.reduced (1.0f).withTrimmedBottom (top.getHeight() * 0.55f), chassisRadius, 1.2f);
    g.setColour (juce::Colour (0xffff8a8a).withAlpha (0.55f));
    g.drawHorizontalLine ((int) top.getBottom() - 1, top.getX() + chassisRadius, top.getRight() - chassisRadius);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.drawRoundedRectangle (chassis.reduced (0.5f), chassisRadius, 1.0f);

    // 3. The face plate, sunk into the chassis: neutral brushed silver, with
    //    the red light reflected only near its lower edge.
    const auto face = geometry.face;
    if (! face.isEmpty())
    {
        juce::Path facePath;
        facePath.addRoundedRectangle (face, faceRadius);

        // Groove round it: dark (in shadow) with a light lower lip.
        juce::DropShadow (juce::Colours::black.withAlpha (0.35f), 6, { 0, -1 }).drawForPath (g, facePath);
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.drawRoundedRectangle (face.expanded (2.5f).translated (0.0f, 1.5f), faceRadius + 2.0f, 1.0f);

        juce::ColourGradient metal (juce::Colour (0xffeae5e8), face.getX(), face.getY(),
                                    juce::Colour (0xffc2b6bd), face.getX(), face.getBottom(), false);
        metal.addColour (0.5, juce::Colour (0xffd9d1d6));
        g.setGradientFill (metal);
        g.fillPath (facePath);

        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (facePath);
        g.drawImageAt (brushed, 0, 0);

        // A broad diagonal sheen across the top, like light on polished metal.
        juce::ColourGradient sheen (juce::Colours::white.withAlpha (0.0f), face.getX(), face.getY(),
                                    juce::Colours::white.withAlpha (0.0f), face.getX() + face.getWidth() * 0.5f, face.getBottom(), false);
        sheen.addColour (0.35, juce::Colours::white.withAlpha (0.22f));
        g.setGradientFill (sheen);
        g.fillRect (face);

        g.setGradientFill (juce::ColourGradient (ParagonColours::glow.withAlpha (0.0f), 0.0f, face.getBottom() - 90.0f,
                                                 ParagonColours::glow.withAlpha (0.2f), 0.0f, face.getBottom(), false));
        g.fillRect (face.withTop (face.getBottom() - 90.0f));
        g.setColour (ParagonColours::glow.withAlpha (0.1f));
        g.drawRoundedRectangle (face.reduced (2.0f), faceRadius - 2.0f, 5.0f);
    }

    // 4. Section panels: the mockup's slot row and footswitch row, raised a
    //    little: a soft shadow, a slightly lighter top, crisp light and
    //    dark edges, and a faint pink edge light.
    for (const auto& section : geometry.sections)
    {
        juce::Path panel;
        panel.addRoundedRectangle (section, 12.0f);
        juce::DropShadow (juce::Colour (0xff3a2430).withAlpha (0.25f), 10, { 0, 4 }).drawForPath (g, panel);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffece6ea), 0.0f, section.getY(),
                                                 juce::Colour (0xffd2c8ce), 0.0f, section.getBottom(), false));
        g.fillPath (panel);
        {
            juce::Graphics::ScopedSaveState state (g);
            g.reduceClipRegion (panel);
            g.drawImageAt (brushed, 0, 0);
        }
        g.setColour (ParagonColours::glow.withAlpha (0.14f));
        g.drawRoundedRectangle (section.reduced (2.0f), 10.0f, 3.0f);
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.drawRoundedRectangle (section.reduced (0.5f).withTrimmedBottom (section.getHeight() * 0.5f), 12.0f, 1.0f);
        g.setColour (juce::Colour (0xff4a3a42).withAlpha (0.3f));
        g.drawRoundedRectangle (section, 12.0f, 1.0f);
    }

    // 5. Recessed panels behind the slot windows: engraved outlines.
    for (const auto& inset : geometry.insets)
    {
        g.setColour (juce::Colour (0xff3a2a32).withAlpha (0.06f));
        g.fillRoundedRectangle (inset, 8.0f);
        g.setColour (juce::Colour (0xff3a2a32).withAlpha (0.3f));
        g.drawRoundedRectangle (inset, 8.0f, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.8f));
        g.drawRoundedRectangle (inset.translated (0.0f, 1.0f), 8.0f, 1.0f);
    }

    // 6. The expression pedal's bay: a deeper, darker chrome housing split
    //    from the face by a dark seam. The pedal component draws the pads.
    if (const auto bay = geometry.pedalBay; ! bay.isEmpty())
    {
        juce::Path bayPath;
        bayPath.addRoundedRectangle (bay, 22.0f);
        juce::DropShadow (juce::Colours::black.withAlpha (0.45f), 8, { -2, 3 }).drawForPath (g, bayPath);
        juce::ColourGradient housing (juce::Colour (0xffe9e2e6), bay.getX(), 0.0f, juce::Colour (0xff7e7078), bay.getRight(), 0.0f, false);
        housing.addColour (0.06, juce::Colours::white);
        housing.addColour (0.25, juce::Colour (0xffc4b9bf));
        housing.addColour (0.8, juce::Colour (0xffb0a4ab));
        g.setGradientFill (housing);
        g.fillPath (bayPath);
        g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.0f), 0.0f, bay.getY(),
                                                 juce::Colours::black.withAlpha (0.25f), 0.0f, bay.getBottom(), false));
        g.fillPath (bayPath);
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.drawRoundedRectangle (bay.reduced (1.5f).withTrimmedBottom (bay.getHeight() * 0.6f), 20.0f, 1.0f);
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.drawRoundedRectangle (bay, 22.0f, 1.0f);
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
