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
        auto area = bounds.reduced (12, 8);
        l.title = area.removeFromTop (20);
        l.corner = l.title.withLeft (l.title.getRight() - 22);
        l.caption = area.removeFromBottom (20);
        area.removeFromBottom (4);

        const auto buttonWidth = juce::jmin (area.getWidth() - 80, 260);
        l.button = area.withSizeKeepingCentre (buttonWidth, juce::jmin (area.getHeight(), 70));
        l.previous = l.button.withX (l.button.getX() - 34).withWidth (26);
        l.next = l.button.withX (l.button.getRight() + 8).withWidth (26);
        return l;
    }

    void drawTitle (juce::Graphics& g, const Layout& l, const juce::String& title)
    {
        g.setColour (ParagonColours::ink);
        g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        g.drawText (title, l.title, juce::Justification::centred, false);
    }

    void drawButton (juce::Graphics& g, juce::Rectangle<int> areaInt, Icons::Type icon, juce::Colour iconColour,
                     const juce::String& name, bool hover, bool highlight)
    {
        const auto area = areaInt.toFloat();

        // Shadow under the button, then the black body with a soft top highlight.
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.fillRoundedRectangle (area.translated (0.0f, 3.0f), 8.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2b2a2f), area.getTopLeft(),
                                                 juce::Colour (0xff050506), area.getBottomLeft(), false));
        g.fillRoundedRectangle (area, 8.0f);
        g.setColour (highlight ? ParagonColours::glow : hover ? ParagonColours::inkDim : juce::Colours::black);
        g.drawRoundedRectangle (area.reduced (0.5f), 8.0f, highlight ? 2.0f : 1.0f);

        auto content = area.reduced (12.0f, 8.0f);
        if (name.isEmpty())
        {
            Icons::draw (g, icon, content.withSizeKeepingCentre (content.getHeight() * 1.3f, content.getHeight()), iconColour, 1.8f, true);
            return;
        }

        Icons::draw (g, icon, content.removeFromLeft (content.getHeight() * 1.1f), iconColour, 1.8f, true);
        content.removeFromLeft (10.0f);
        g.setColour (ParagonColours::accent);
        g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        g.drawFittedText (name, content.toNearestInt(), juce::Justification::centredLeft, 2, 0.75f);
    }

    void drawCaption (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& text, bool isError)
    {
        g.setColour (isError ? juce::Colour (0xffc8231a) : ParagonColours::ink);
        g.setFont (juce::FontOptions (12.0f, isError ? juce::Font::bold : juce::Font::plain));
        g.drawFittedText (text, area, juce::Justification::centred, 1, 0.7f);
    }

    std::unique_ptr<juce::Button> makeArrow (bool pointsLeft)
    {
        return std::make_unique<ArrowButton> (pointsLeft);
    }
}
