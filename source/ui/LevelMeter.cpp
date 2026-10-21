#include "LevelMeter.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include "ParagonLookAndFeel.h"

namespace
{
    constexpr int refreshHz = 30;
    constexpr float fallDbPerSecond = 24.0f;   // how fast the meter drops after a peak
    constexpr int numSegments = 36;
    constexpr float scaleMarksDb[] = { 12.0f, 6.0f, 0.0f, -6.0f, -12.0f, -18.0f, -24.0f, -36.0f, -48.0f, -60.0f };
}

LevelMeter::LevelMeter (LevelMeterSource& sourceToUse) : source (sourceToUse)
{
    startTimerHz (refreshHz);
}

float LevelMeter::dbToProportion (float db) const noexcept
{
    return juce::jlimit (0.0f, 1.0f, (db - minDb) / (maxDb - minDb));
}

void LevelMeter::timerCallback()
{
    const auto peakDb = juce::Decibels::gainToDecibels (source.takePeak(), minDb);
    const auto fallenDb = displayedDb - fallDbPerSecond / (float) refreshHz;
    const auto newDb = juce::jmax (peakDb, fallenDb, minDb);

    if (! juce::approximatelyEqual (newDb, displayedDb))
    {
        displayedDb = newDb;
        repaint();
    }
}

void LevelMeter::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto scaleArea = bounds.removeFromLeft (bounds.getWidth() * 0.6f);
    auto barArea = bounds.reduced (2.0f, 0.0f).withTrimmedTop (6.0f).withTrimmedBottom (6.0f);
    scaleArea = scaleArea.withTop (barArea.getY()).withBottom (barArea.getBottom());

    g.setFont (juce::FontOptions (10.0f));
    for (auto markDb : scaleMarksDb)
    {
        const auto y = scaleArea.getBottom() - dbToProportion (markDb) * scaleArea.getHeight();
        g.setColour (ParagonColours::textDim);
        g.drawText (juce::String ((int) markDb), scaleArea.withHeight (12.0f).withCentre ({ scaleArea.getCentreX(), y }),
                    juce::Justification::centredRight, false);
    }

    const auto segmentHeight = barArea.getHeight() / (float) numSegments;
    const auto litProportion = dbToProportion (displayedDb);

    for (int i = 0; i < numSegments; ++i)
    {
        const auto segmentTop = barArea.getBottom() - (float) (i + 1) * segmentHeight;
        const auto segment = juce::Rectangle<float> (barArea.getX(), segmentTop, barArea.getWidth(), segmentHeight)
                                 .reduced (0.0f, segmentHeight * 0.2f);
        const auto proportion = (float) (i + 1) / (float) numSegments;
        const auto isOverZero = proportion > dbToProportion (0.0f);
        const auto colour = isOverZero ? ParagonColours::meterRed : ParagonColours::accent;
        const auto lit = proportion <= litProportion + 1.0e-4f;

        g.setColour (lit ? colour : colour.withAlpha (0.12f));
        g.fillRect (segment);
    }
}
