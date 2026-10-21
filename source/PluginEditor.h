#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"
#include "ui/MainView.h"
#include "ui/ParagonLookAndFeel.h"

// The plugin window. All the content lives in MainView, laid out on a fixed
// 1600 x 900 canvas; the editor just scales it to whatever size the window
// is, keeping the mockup's 16:9 shape.
class ParagonX1AudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit ParagonX1AudioProcessorEditor (ParagonX1AudioProcessor&);
    ~ParagonX1AudioProcessorEditor() override;

    void resized() override;

    // Opens at 1280 x 720 so it fits on laptop screens; drag to resize.
    static constexpr int defaultWidth = 1280;
    static constexpr int defaultHeight = 720;

private:
    ParagonLookAndFeel lookAndFeel;
    MainView view;
    juce::TooltipWindow tooltips { this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParagonX1AudioProcessorEditor)
};
