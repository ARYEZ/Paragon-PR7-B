#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Icons.h"

// Shared look for the three slot windows on the pedal (AMP, CABINET, RIG):
//
//          AMP (o)                <- title and status LED
//     <  [ icon  Name  ]  >       <- black button, arrows either side
//      Load / Edit NAM Model      <- caption (or what's loaded / an error)
namespace SlotWindow
{
    struct Layout
    {
        juce::Rectangle<int> title, led, button, previous, next, caption, corner;
    };

    Layout layout (juce::Rectangle<int> bounds);

    void drawTitle (juce::Graphics&, const Layout&, const juce::String& title);
    void drawButton (juce::Graphics&, juce::Rectangle<int> area, Icons::Type icon, juce::Colour iconColour,
                     const juce::String& name, bool hover, bool highlight);
    void drawCaption (juce::Graphics&, juce::Rectangle<int> area, const juce::String& text, bool isError);

    // A dark arrow, for stepping through files or rigs.
    std::unique_ptr<juce::Button> makeArrow (bool pointsLeft);
}
