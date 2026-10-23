#pragma once

#include <array>

// Starting points for the graphic EQ: the curves guitarists most often dial
// into a 9/10-band octave EQ (63 Hz to 16 kHz, as on an MXR 10-band, or the
// graphic EQ blocks in amp sims) after an amp and cab. Choosing one sets the
// faders and LEVEL; from there you tweak as usual. LEVEL makes up for the
// loudness the curve adds or takes away (measured over a guitar's range, 80 Hz
// to 5 kHz), so switching between presets compares tone, not volume.
namespace EqPresets
{
    struct Preset
    {
        const char* name;
        const char* description;                 // the menu's tooltip
        std::array<float, 9> gainsDb;            // 63, 125, 250, 500, 1k, 2k, 4k, 8k, 16k
        float levelDb;
    };

    inline constexpr std::array<Preset, 12> all { {
        { "Metal Scoop",        "The classic V: big lows and highs, mids pulled right down.",
          { 3.0f, 2.0f, 0.0f, -4.0f, -6.0f, -3.0f, 2.0f, 3.0f, 1.0f }, 1.0f },
        { "Tight Low End",      "Cuts the boom under high gain so palm mutes stay tight; adds bite.",
          { -6.0f, -4.0f, -1.0f, 0.0f, 1.0f, 2.0f, 2.0f, 1.0f, 0.0f }, -0.5f },
        { "Mud Cut",            "Clears the boxy, muddy low mids without thinning the tone.",
          { 0.0f, -1.0f, -4.0f, -3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, 1.5f },
        { "Fizz Tamer",         "Rolls off the harsh top end that high-gain captures can have.",
          { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -2.0f, -6.0f, -9.0f }, 0.5f },
        { "Lead Mid Boost",     "Pushes the mids so solos sit on top of the band.",
          { -2.0f, -1.0f, 0.0f, 3.0f, 5.0f, 3.0f, 0.0f, -1.0f, -2.0f }, -3.0f },
        { "Cut Through the Mix", "Less low end, more upper mids: heard clearly in a dense band.",
          { -4.0f, -2.0f, -2.0f, 0.0f, 2.0f, 4.0f, 3.0f, 0.0f, -3.0f }, -1.5f },
        { "Fat Rhythm",         "Thick, full lows and low mids for big rhythm parts.",
          { 3.0f, 4.0f, 3.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f }, -2.5f },
        { "British Crunch",     "A forward upper-mid bark, like a cranked British stack.",
          { 0.0f, 1.0f, 0.0f, -1.0f, 2.0f, 3.0f, 1.0f, 0.0f, -1.0f }, -1.5f },
        { "Bright Clean",       "Sparkle and air for clean and chorus tones.",
          { -2.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 3.0f, 4.0f, 3.0f }, -1.0f },
        { "Funk Scoop",         "Scooped low mids and a crisp top for clean rhythm.",
          { 0.0f, 1.0f, -1.0f, -3.0f, -2.0f, 0.0f, 3.0f, 4.0f, 2.0f }, 0.0f },
        { "Warm Jazz",          "Round and dark: the highs rolled off, the lows full.",
          { 2.0f, 3.0f, 2.0f, 1.0f, 0.0f, -2.0f, -4.0f, -6.0f, -8.0f }, -1.0f },
        { "Radio",              "A narrow mid band, like an old AM radio or a phone.",
          { -12.0f, -12.0f, -8.0f, 2.0f, 6.0f, 3.0f, -6.0f, -12.0f, -12.0f }, -0.5f },
    } };
}
