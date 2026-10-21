#pragma once

#include <juce_core/juce_core.h>

#include <optional>
#include <vector>

// Works out a tempo from taps on the TAP button: the average of the last
// few gaps between taps, so one slightly late tap doesn't throw it off.
// A pause of more than two seconds starts a new count.
class TapTempo
{
public:
    static constexpr double resetAfterMs = 2000.0;
    static constexpr double minBpm = 40.0, maxBpm = 300.0;
    static constexpr size_t tapsAveraged = 5;   // so 4 gaps

    // Call with the time of each tap. Returns the tempo from the second tap on.
    std::optional<double> tap (double nowMs)
    {
        if (! times.empty() && (nowMs - times.back() > resetAfterMs || nowMs <= times.back()))
            times.clear();

        times.push_back (nowMs);
        if (times.size() > tapsAveraged)
            times.erase (times.begin());

        if (times.size() < 2)
            return std::nullopt;

        const auto averageGapMs = (times.back() - times.front()) / (double) (times.size() - 1);
        return juce::jlimit (minBpm, maxBpm, 60000.0 / averageGapMs);
    }

    int numTaps() const noexcept { return (int) times.size(); }

private:
    std::vector<double> times;
};
