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
        // The button's own text colour, so the same switch works on the dark
        // panels (light text) and printed on the silver pedal (dark text).
        const auto textColour = button.findColour (juce::ToggleButton::textColourId);
        g.setColour (on ? textColour : textColour.withMultipliedAlpha (0.6f));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText (button.getButtonText(), bounds.withTrimmedLeft (2.0f), juce::Justification::centredLeft, true);
    }
}

void ParagonLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                               bool highlighted, bool down)
{
    if (button.getProperties()[pedalButtonProperty])
    {
        // Small black cap with a bevel, sitting on the metal.
        auto cap = button.getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (cap.translated (0.0f, 1.5f), 3.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (down ? 0xff151517 : 0xff2c2b30), cap.getTopLeft(),
                                                 juce::Colour (0xff0b0b0c), cap.getBottomLeft(), false));
        g.fillRoundedRectangle (cap, 3.0f);
        g.setColour (highlighted ? ParagonColours::inkDim : juce::Colours::black);
        g.drawRoundedRectangle (cap, 3.0f, 1.0f);
        return;
    }

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

void ParagonLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button, bool highlighted, bool down)
{
    if (! button.getProperties()[pedalButtonProperty])
    {
        LookAndFeel_V4::drawButtonText (g, button, highlighted, down);
        return;
    }

    auto area = button.getLocalBounds().toFloat().reduced (4.0f, 2.0f);
    const auto on = button.getToggleState();

    if (button.getButtonText().isEmpty())
    {
        // Just the LED stripe, like the mockup's EXIT / MODE / BYPASS caps.
        const auto stripe = area.withSizeKeepingCentre (area.getWidth() * 0.6f, 3.0f);
        if (on)
        {
            g.setColour (ParagonColours::glow.withAlpha (0.35f));
            g.fillRoundedRectangle (stripe.expanded (2.0f, 2.5f), 3.0f);
        }
        g.setColour (on ? ParagonColours::glow : ParagonColours::glow.withAlpha (0.25f));
        g.fillRoundedRectangle (stripe, 1.5f);
        return;
    }

    g.setFont (juce::FontOptions (juce::jmin (12.0f, area.getHeight() * 0.7f), juce::Font::bold));
    g.setColour (button.isEnabled() ? (on ? ParagonColours::accent : ParagonColours::text)
                                    : ParagonColours::textDim);
    g.drawFittedText (button.getButtonText(), area.toNearestInt(), juce::Justification::centred, 1);
}

void ParagonLookAndFeel::drawPedalKnob (juce::Graphics& g, juce::Rectangle<float> bounds, float angle)
{
    const auto size = juce::jmin (bounds.getWidth(), bounds.getHeight());
    const auto body = bounds.withSizeKeepingCentre (size, size).reduced (size * 0.08f);
    const auto centre = body.getCentre();

    // Red-orange glow on the metal around the knob, as in the mockup.
    g.setGradientFill (juce::ColourGradient (ParagonColours::glow.withAlpha (0.45f), centre,
                                             ParagonColours::glow.withAlpha (0.0f), centre.translated (size * 0.55f, 0.0f), true));
    g.fillEllipse (body.expanded (size * 0.08f));

    // Shadow, then the knurled black body: a ring of fine ridges round the edge.
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillEllipse (body.translated (0.0f, size * 0.03f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3b3a40), body.getTopLeft(),
                                             juce::Colour (0xff050506), body.getBottomRight(), false));
    g.fillEllipse (body);

    const auto radius = body.getWidth() / 2.0f;
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    for (int ridge = 0; ridge < 48; ++ridge)
    {
        const auto a = (float) ridge / 48.0f * juce::MathConstants<float>::twoPi;
        g.drawLine ({ centre.getPointOnCircumference (radius * 0.86f, a), centre.getPointOnCircumference (radius * 0.99f, a) }, 1.0f);
    }

    // Smooth top cap with a highlight.
    const auto cap = body.reduced (radius * 0.18f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2f2e34), cap.getTopLeft(),
                                             juce::Colour (0xff0d0d0f), cap.getBottomRight(), false));
    g.fillEllipse (cap);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawEllipse (cap.reduced (1.0f), 1.0f);

    // Pointer line.
    g.setColour (ParagonColours::text);
    g.drawLine ({ centre.getPointOnCircumference (radius * 0.25f, angle), centre.getPointOnCircumference (radius * 0.75f, angle) }, 2.5f);
}

void ParagonLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float startAngle, float endAngle,
                                           juce::Slider& slider)
{
    if (slider.getProperties()[pedalKnobProperty])
    {
        drawPedalKnob (g, juce::Rectangle<int> (x, y, width, height).toFloat(), startAngle + sliderPos * (endAngle - startAngle));
        return;
    }

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

void ParagonLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::TwoValueHorizontal)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    const auto centreY = (float) y + (float) height * 0.5f;
    const auto left = (float) x + 8.0f, right = (float) (x + width) - 8.0f;

    g.setColour (ParagonColours::panelEdge);
    g.fillRoundedRectangle (left, centreY - 2.0f, right - left, 4.0f, 2.0f);
    g.setColour (ParagonColours::accent);
    g.fillRoundedRectangle (minSliderPos, centreY - 2.0f, maxSliderPos - minSliderPos, 4.0f, 2.0f);

    for (auto position : { minSliderPos, maxSliderPos })
    {
        const auto thumb = juce::Rectangle<float> (14.0f, 14.0f).withCentre ({ position, centreY });
        g.setColour (ParagonColours::accent.withAlpha (0.25f));
        g.fillEllipse (thumb.expanded (4.0f));
        g.setColour (ParagonColours::accent);
        g.fillEllipse (thumb);
        g.setColour (ParagonColours::background);
        g.fillEllipse (thumb.reduced (4.5f));
    }
}
