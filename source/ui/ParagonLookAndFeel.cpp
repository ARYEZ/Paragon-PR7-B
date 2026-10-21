#include "ParagonLookAndFeel.h"

ParagonLookAndFeel::ParagonLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, ParagonColours::background);
    setColour (juce::Label::textColourId, ParagonColours::text);
    setColour (juce::Slider::textBoxTextColourId, ParagonColours::accent);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId, ParagonColours::accentDim);
    setColour (juce::TextEditor::textColourId, ParagonColours::accent);
    setColour (juce::CaretComponent::caretColourId, ParagonColours::accent);
    setColour (juce::TextButton::textColourOffId, ParagonColours::text);
    setColour (juce::ToggleButton::textColourId, ParagonColours::text);
    setColour (juce::TooltipWindow::backgroundColourId, ParagonColours::panel);
    setColour (juce::TooltipWindow::textColourId, ParagonColours::text);
    setColour (juce::TooltipWindow::outlineColourId, ParagonColours::panelEdge);
}

void ParagonLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool highlighted, bool)
{
    auto bounds = button.getLocalBounds().toFloat();
    const auto ledSize = juce::jmin (12.0f, bounds.getHeight() - 4.0f);
    const auto led = bounds.removeFromLeft (bounds.getHeight()).withSizeKeepingCentre (ledSize, ledSize);
    const auto on = button.getToggleState();

    if (on)
    {
        // Soft glow behind a lit LED.
        g.setColour (ParagonColours::accent.withAlpha (0.25f));
        g.fillEllipse (led.expanded (3.0f));
    }

    g.setColour (on ? ParagonColours::accent : ParagonColours::accentDim.darker (0.6f));
    g.fillEllipse (led);
    g.setColour (highlighted ? ParagonColours::text : juce::Colours::black);
    g.drawEllipse (led, 1.0f);

    if (button.getButtonText().isNotEmpty())
    {
        g.setColour (on ? ParagonColours::text : ParagonColours::textDim);
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText (button.getButtonText(), bounds.withTrimmedLeft (2.0f), juce::Justification::centredLeft, true);
    }
}

void ParagonLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                               bool highlighted, bool down)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    auto fill = juce::Colour (0xff1c1c20);
    if (down)
        fill = ParagonColours::accentDim;
    else if (highlighted)
        fill = fill.brighter (0.1f);

    g.setColour (button.isEnabled() ? fill : fill.withAlpha (0.4f));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (ParagonColours::panelEdge);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

void ParagonLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float startAngle, float endAngle,
                                           juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const auto centre = bounds.getCentre();
    const auto arcRadius = radius - 3.0f;
    const auto angle = startAngle + sliderPos * (endAngle - startAngle);

    // Track and value arc, like the orange ring on the mockup's knobs.
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
    g.setColour (ParagonColours::panelEdge);
    g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, angle, true);
    g.setColour (ParagonColours::accent);
    g.strokePath (value, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knob body.
    const auto knobRadius = arcRadius - 6.0f;
    const auto knob = juce::Rectangle<float> (knobRadius * 2.0f, knobRadius * 2.0f).withCentre (centre);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a3a40), knob.getTopLeft(),
                                             juce::Colour (0xff101012), knob.getBottomRight(), false));
    g.fillEllipse (knob);
    g.setColour (juce::Colours::black);
    g.drawEllipse (knob, 1.0f);

    // Pointer.
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.5f, -knobRadius + 3.0f, 3.0f, knobRadius * 0.45f, 1.5f);
    g.setColour (ParagonColours::text);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre));
}
