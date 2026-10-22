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
            g.setColour (juce::Colour (0xfffff0ec));   // white-hot core, as the mockup's lit LEDs
            g.fillEllipse (ring.reduced (2.2f));
        }
        g.setColour (lit ? ParagonColours::glow : ParagonColours::glow.withAlpha (0.45f));
        g.drawEllipse (ring, 1.6f);
    }

    void drawButton (juce::Graphics& g, juce::Rectangle<int> areaInt, Icons::Type icon, juce::Colour iconColour,
                     bool hover, bool highlight)
    {
        // A piano-black key like the footswitches, its icon glowing on top.
        const auto area = areaInt.toFloat();
        ParagonLookAndFeel::drawPianoBlackKey (g, area, 8.0f, 6.0f, false, hover || highlight);
        const auto top = area.reduced (6.0f, 0.0f).withTrimmedTop (4.0f).withTrimmedBottom (9.6f);
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
