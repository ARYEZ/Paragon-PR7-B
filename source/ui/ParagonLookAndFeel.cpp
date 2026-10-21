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
