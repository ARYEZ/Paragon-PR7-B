#include "LedDisplay.h"

#include "ParagonLookAndFeel.h"

namespace
{
    const juce::Colour ledOrange { 0xffff7a1a };

    // Which of the seven segments (a-g) light for each digit.
    //    a
    //  f   b
    //    g
    //  e   c
    //    d
    std::array<bool, 7> segmentsFor (juce::juce_wchar digit)
    {
        switch (digit)
        {
            case '0': return { 1, 1, 1, 1, 1, 1, 0 };
            case '1': return { 0, 1, 1, 0, 0, 0, 0 };
            case '2': return { 1, 1, 0, 1, 1, 0, 1 };
            case '3': return { 1, 1, 1, 1, 0, 0, 1 };
            case '4': return { 0, 1, 1, 0, 0, 1, 1 };
            case '5': return { 1, 0, 1, 1, 0, 1, 1 };
            case '6': return { 1, 0, 1, 1, 1, 1, 1 };
            case '7': return { 1, 1, 1, 0, 0, 0, 0 };
            case '8': return { 1, 1, 1, 1, 1, 1, 1 };
            case '9': return { 1, 1, 1, 1, 0, 1, 1 };
            case '-': return { 0, 0, 0, 0, 0, 0, 1 };
            default:  return { 0, 0, 0, 0, 0, 0, 0 };
        }
    }
}

LedDisplay::LedDisplay()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setTooltip ("The current rig. The dot lights when it has unsaved changes. Click to browse rigs.");
}

void LedDisplay::setContents (const Contents& newContents)
{
    const auto changed = newContents.number != contents.number || newContents.name != contents.name
                      || newContents.bank != contents.bank || newContents.modified != contents.modified
                      || newContents.bypassed != contents.bypassed;
    contents = newContents;
    if (changed)
        repaint();
}

void LedDisplay::drawDigit (juce::Graphics& g, juce::juce_wchar digit, juce::Rectangle<float> area, juce::Colour lit)
{
    const auto w = area.getWidth(), h = area.getHeight();
    const auto t = w * 0.16f;                 // segment thickness
    const auto slant = w * 0.12f;             // italic lean, like the mockup's digits
    const auto lit7 = segmentsFor (digit);

    // Segment end points in an upright cell; the slant is applied after.
    const auto top = t / 2.0f, middle = h / 2.0f, bottom = h - t / 2.0f;
    const auto left = t / 2.0f, right = w - t / 2.0f - slant;
    const std::array<juce::Line<float>, 7> segments {
        juce::Line<float> { left + t, top, right - t, top },             // a
        juce::Line<float> { right, top + t, right, middle - t * 0.6f },  // b
        juce::Line<float> { right, middle + t * 0.6f, right, bottom - t }, // c
        juce::Line<float> { left + t, bottom, right - t, bottom },       // d
        juce::Line<float> { left, middle + t * 0.6f, left, bottom - t }, // e
        juce::Line<float> { left, top + t, left, middle - t * 0.6f },    // f
        juce::Line<float> { left + t, middle, right - t, middle },       // g
    };

    const auto shear = juce::AffineTransform::shear (-slant / h, 0.0f).translated (area.getX() + slant, area.getY());

    for (size_t i = 0; i < segments.size(); ++i)
    {
        juce::Path segment;
        segment.addLineSegment (segments[i], t);
        segment = segment.createPathWithRoundedCorners (t * 0.4f);
        segment.applyTransform (shear);

        if (lit7[i])
        {
            g.setColour (lit.withAlpha (0.25f));
            g.strokePath (segment, juce::PathStrokeType (t * 0.9f));   // glow
            g.setColour (lit);
        }
        else
        {
            g.setColour (lit.withAlpha (0.06f));                       // unlit segments faintly visible
        }
        g.fillPath (segment);
    }
}

void LedDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Bezel and black glass.
    g.setColour (juce::Colours::black.withAlpha (0.3f));
    g.fillRoundedRectangle (bounds.translated (0.0f, 2.0f), 8.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a2a2e), bounds.getTopLeft(),
                                             juce::Colour (0xff0a0a0b), bounds.getBottomLeft(), false));
    g.fillRoundedRectangle (bounds, 8.0f);
    auto glass = bounds.reduced (8.0f);
    g.setColour (juce::Colour (0xff070605));
    g.fillRoundedRectangle (glass, 5.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.06f), glass.getTopLeft(),
                                             juce::Colours::transparentWhite, glass.getCentre(), false));
    g.fillRoundedRectangle (glass, 5.0f);

    auto content = glass.reduced (glass.getHeight() * 0.14f, glass.getHeight() * 0.12f);
    const auto colour = contents.bypassed ? ledOrange.withAlpha (0.45f) : ledOrange;

    // Big digits: the rig's preset number.
    const auto digits = contents.number > 0 ? juce::String (contents.number).paddedLeft ('0', 3) : juce::String ("---");
    auto digitsArea = content.removeFromLeft (content.getHeight() * 1.45f);
    const auto digitWidth = digitsArea.getWidth() / 3.0f;
    for (int i = 0; i < 3; ++i)
        drawDigit (g, digits[i], digitsArea.removeFromLeft (digitWidth).reduced (digitWidth * 0.08f, 0.0f), colour);

    content.removeFromLeft (content.getHeight() * 0.3f);

    // Name, then the bank and preset line under a thin rule.
    auto nameArea = content.removeFromTop (content.getHeight() * 0.55f);
    g.setColour (colour);
    g.setFont (juce::FontOptions (nameArea.getHeight() * 0.62f, juce::Font::bold));
    g.drawFittedText (contents.bypassed ? juce::String ("BYPASSED") : contents.name.toUpperCase(),
                      nameArea.toNearestInt(), juce::Justification::bottomLeft, 1, 0.8f);

    g.setColour (colour.withAlpha (0.35f));
    g.drawHorizontalLine ((int) nameArea.getBottom() + 3, content.getX(), content.getRight());

    g.setColour (colour);
    g.setFont (juce::FontOptions (content.getHeight() * 0.5f, juce::Font::bold));
    auto infoArea = content.withTrimmedTop (6.0f);
    const auto info = contents.bank.isEmpty() ? juce::String ("NOT SAVED")
                                              : "BANK " + contents.bank + "     PRESET " + juce::String (contents.number);
    g.drawText (info, infoArea, juce::Justification::topLeft, false);

    // Unsaved-changes dot.
    const auto dot = juce::Rectangle<float> (7.0f, 7.0f).withCentre ({ infoArea.getRight() - 6.0f, infoArea.getY() + infoArea.getHeight() * 0.3f });
    if (contents.modified)
    {
        g.setColour (ParagonColours::glow.withAlpha (0.4f));
        g.fillEllipse (dot.expanded (3.0f));
    }
    g.setColour (contents.modified ? ParagonColours::glow : ParagonColours::glow.withAlpha (0.12f));
    g.fillEllipse (dot);
}

void LedDisplay::mouseUp (const juce::MouseEvent& event)
{
    if (event.mouseWasClicked() && onClick)
        onClick();
}
