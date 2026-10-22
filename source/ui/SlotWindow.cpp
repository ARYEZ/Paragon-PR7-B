#include "SlotWindow.h"

#include "ParagonLookAndFeel.h"

namespace SlotWindow
{
    namespace
    {
        // A solid triangle printed on the metal; darkens on hover.
        class ArrowButton final : public juce::Button
        {
        public:
            explicit ArrowButton (bool left) : juce::Button (left ? "Previous" : "Next"), pointsLeft (left) {}

            void paintButton (juce::Graphics& g, bool highlighted, bool down) override
            {
                auto area = getLocalBounds().toFloat().withSizeKeepingCentre (10.0f, 16.0f);
                if (down)
                    area = area.translated (0.0f, 1.0f);

                juce::Path triangle;
                if (pointsLeft)
                    triangle.addTriangle (area.getRight(), area.getY(), area.getRight(), area.getBottom(), area.getX(), area.getCentreY());
                else
                    triangle.addTriangle (area.getX(), area.getY(), area.getX(), area.getBottom(), area.getRight(), area.getCentreY());

                g.setColour (! isEnabled() ? ParagonColours::inkDim.withAlpha (0.4f)
                                           : highlighted ? ParagonColours::glow : ParagonColours::ink);
                g.fillPath (triangle);
            }

        private:
            bool pointsLeft;
        };
    }

    Layout layout (juce::Rectangle<int> bounds)
    {
        Layout l;
        auto area = bounds.reduced (10, 6);
        l.title = area.removeFromTop (22);
        l.corner = l.title.withLeft (l.title.getRight() - 22);
        l.caption = area.removeFromBottom (22);
        area.removeFromBottom (4);

        l.button = area.withSizeKeepingCentre (juce::jmin (area.getWidth() - 64, 158), juce::jmin (area.getHeight(), 70));
        l.previous = l.button.withX (l.button.getX() - 28).withWidth (20);
        l.next = l.button.withX (l.button.getRight() + 8).withWidth (20);
        return l;
    }

    void drawTitle (juce::Graphics& g, const Layout& l, const juce::String& title)
    {
        g.setColour (ParagonColours::ink);
        g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        g.drawText (title, l.title, juce::Justification::centred, false);
    }

    void drawRingLed (juce::Graphics& g, juce::Point<float> centre, bool lit)
    {
        const auto ring = juce::Rectangle<float> (9.0f, 9.0f).withCentre (centre);
        if (lit)
        {
            g.setColour (ParagonColours::glow.withAlpha (0.3f));
            g.fillEllipse (ring.expanded (4.0f));
            g.setColour (juce::Colour (0xffff7a6a));
            g.fillEllipse (ring.reduced (2.0f));
        }
        g.setColour (lit ? ParagonColours::glow : ParagonColours::glow.withAlpha (0.45f));
        g.drawEllipse (ring, 1.6f);
    }

    void drawButton (juce::Graphics& g, juce::Rectangle<int> areaInt, Icons::Type icon, juce::Colour iconColour,
                     bool hover, bool highlight)
    {
        const auto area = areaInt.toFloat();
        juce::Path body;
        body.addRoundedRectangle (area, 8.0f);

        // A black block like the footswitches: shadow, darker front edge,
        // a sheen on top, and the icon glowing in the middle.
        juce::DropShadow (juce::Colours::black.withAlpha (0.5f), 9, { 0, 5 }).drawForPath (g, body);
        g.setColour (juce::Colour (0xff050506));
        g.fillPath (body);
        const auto top = area.withTrimmedBottom (area.getHeight() * 0.14f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a292e), 0.0f, top.getY(),
                                                 juce::Colour (0xff111113), 0.0f, top.getBottom(), false));
        g.fillRoundedRectangle (top, 7.0f);
        g.setColour (juce::Colours::white.withAlpha (0.14f));
        g.drawRoundedRectangle (top.reduced (1.0f), 6.0f, 1.0f);
        g.setColour (ParagonColours::glow.withAlpha (0.3f));
        g.drawHorizontalLine ((int) area.getBottom() - 1, area.getX() + 8.0f, area.getRight() - 8.0f);
        g.setColour (highlight ? ParagonColours::glow : hover ? ParagonColours::glow.withAlpha (0.6f) : juce::Colours::black);
        g.drawRoundedRectangle (area.reduced (0.5f), 8.0f, highlight ? 2.0f : 1.0f);

        const auto iconSize = top.getHeight() * 0.62f;
        Icons::draw (g, icon, top.withSizeKeepingCentre (iconSize * 1.3f, iconSize), iconColour, 1.8f, true);
    }

    void drawCaption (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& text, bool isError)
    {
        g.setColour (isError ? juce::Colour (0xffc8231a) : ParagonColours::ink);
        g.setFont (juce::FontOptions (12.5f, isError ? juce::Font::bold : juce::Font::plain));
        g.drawFittedText (text, area, juce::Justification::centred, 1, 0.7f);
    }

    std::unique_ptr<juce::Button> makeArrow (bool pointsLeft)
    {
        return std::make_unique<ArrowButton> (pointsLeft);
    }
}
