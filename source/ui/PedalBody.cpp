#include "PedalBody.h"

#include "ParagonLookAndFeel.h"

namespace
{
    constexpr float cornerRadius = 22.0f;
    constexpr float rimWidth = 14.0f;
}

PedalBody::PedalBody()
{
    // Everything here is static, so draw it once and reuse the picture.
    setBufferedToImage (true);
    setInterceptsMouseClicks (false, false);
}

void PedalBody::setInsets (std::vector<juce::Rectangle<int>> newPanels, std::vector<int> dividerYs, juce::Rectangle<int> badgeArea)
{
    panels = std::move (newPanels);
    dividers = std::move (dividerYs);
    badge = badgeArea;
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

    for (int i = 0; i < height * 3; ++i)
    {
        const auto y = random.nextFloat() * (float) height;
        const auto x = random.nextFloat() * (float) width * 1.2f - (float) width * 0.1f;
        const auto length = (float) width * (0.1f + random.nextFloat() * 0.5f);
        const auto light = random.nextBool();
        g.setColour ((light ? juce::Colours::white : juce::Colours::black).withAlpha (0.02f + random.nextFloat() * 0.05f));
        g.drawHorizontalLine ((int) y, x, x + length);
    }
}

void PedalBody::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced (6.0f);

    // Red-orange light spilling round the unit onto the dark background.
    for (int i = 4; i >= 1; --i)
    {
        g.setColour (ParagonColours::glow.withAlpha (0.05f * (float) (5 - i)));
        g.drawRoundedRectangle (bounds.expanded ((float) i * 1.5f), cornerRadius + (float) i, 3.0f);
    }

    // Outer rim: a darker bevel, lit along the top edge.
    juce::Path outline;
    outline.addRoundedRectangle (bounds, cornerRadius);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe9e4e6), bounds.getTopLeft(),
                                             juce::Colour (0xff8c8589), bounds.getBottomLeft(), false));
    g.fillPath (outline);

    // Face plate: light silver, a little warmer towards the edges where the glow reflects.
    const auto face = bounds.reduced (rimWidth);
    juce::Path facePath;
    facePath.addRoundedRectangle (face, cornerRadius - 6.0f);
    g.setGradientFill (juce::ColourGradient (ParagonColours::metalLight, face.getTopLeft(),
                                             ParagonColours::metalMid, face.getBottomRight(), false));
    g.fillPath (facePath);

    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (facePath);
        g.drawImageAt (brushed, 0, 0);

        // Soft pink reflection of the glow along the inside of the rim.
        g.setColour (ParagonColours::glow.withAlpha (0.12f));
        g.drawRoundedRectangle (face.reduced (2.0f), cornerRadius - 8.0f, 6.0f);
    }

    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.drawRoundedRectangle (face.translated (0.0f, -1.0f), cornerRadius - 6.0f, 1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.drawRoundedRectangle (face, cornerRadius - 6.0f, 1.0f);

    // Glowing seam round the outside edge, like the mockup's neon trim.
    g.setColour (ParagonColours::glow.withAlpha (0.55f));
    g.drawRoundedRectangle (bounds.reduced (1.0f), cornerRadius, 1.5f);

    // Divider lines: an engraved groove (dark line over a light one).
    for (auto y : dividers)
    {
        const auto left = face.getX() + 16.0f, right = face.getRight() - 16.0f;
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        g.drawHorizontalLine (y, left, right);
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.drawHorizontalLine (y + 1, left, right);
    }

    // Recessed panels behind the slot windows.
    for (const auto& panel : panels)
    {
        const auto area = panel.toFloat();
        g.setColour (juce::Colours::black.withAlpha (0.05f));
        g.fillRoundedRectangle (area, 6.0f);
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        g.drawRoundedRectangle (area, 6.0f, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.drawRoundedRectangle (area.translated (0.0f, 1.0f), 6.0f, 1.0f);
    }

    // Badge: heavy italic product name, as "CYBER RPX" in the mockup.
    if (! badge.isEmpty())
    {
        auto area = badge.toFloat();
        const auto lineHeight = area.getHeight() * 0.36f;
        const auto font = juce::FontOptions (lineHeight * 1.15f, juce::Font::bold | juce::Font::italic);

        g.setFont (font);
        g.setColour (ParagonColours::ink);
        g.drawText ("PARAGON", area.removeFromTop (lineHeight), juce::Justification::centredLeft, false);
        auto secondLine = area.removeFromTop (lineHeight);
        g.drawText ("X", secondLine, juce::Justification::centredLeft, false);
        const auto xWidth = (float) juce::GlyphArrangement::getStringWidthInt (juce::Font (font), "X ");
        g.setColour (ParagonColours::glow);
        g.drawText ("1", secondLine.withTrimmedLeft (xWidth * 0.75f), juce::Justification::centredLeft, false);

        g.setFont (juce::FontOptions (lineHeight * 0.3f, juce::Font::bold));
        g.setColour (ParagonColours::glow.darker (0.2f));
        g.drawText ("NEURAL MULTI-FX PROCESSOR", area.withTrimmedTop (4.0f).withHeight (lineHeight * 0.4f),
                    juce::Justification::centredLeft, false);
    }
}
