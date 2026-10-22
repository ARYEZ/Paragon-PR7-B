#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Icons.h"

// Shared look for the three slot windows on the pedal (AMP, CABINET, RIG):
//
//          AMP (o)                <- title and status LED
//     <  [   icon   ]  >          <- black block, arrows either side
//      Load / Edit NAM Model      <- caption: what's loaded, the hint, or an error
namespace SlotWindow
{
    struct Layout
    {
        juce::Rectangle<int> title, led, button, previous, next, caption, corner;
    };

    Layout layout (juce::Rectangle<int> bounds);

    void drawTitle (juce::Graphics&, const Layout&, const juce::String& title);
    // The black block with the slot's glowing icon, as in the mockup.
    void drawButton (juce::Graphics&, juce::Rectangle<int> area, Icons::Type icon, juce::Colour iconColour,
                     bool hover, bool highlight);

    // The mockup's small status LED: a red ring, filled and glowing when lit.
    void drawRingLed (juce::Graphics&, juce::Point<float> centre, bool lit);
    void drawCaption (juce::Graphics&, juce::Rectangle<int> area, const juce::String& text, bool isError);

    // A dark arrow, for stepping through files or rigs.
    std::unique_ptr<juce::Button> makeArrow (bool pointsLeft);
}
