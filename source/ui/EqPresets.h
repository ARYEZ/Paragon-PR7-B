#pragma once

#include <array>

// Starting points for the bass EQ, most of them for modern metal bass: the
// curves bassists and mix engineers commonly reach for (tight, controlled
// lows, the low-mid mud cut, and upper-mid "clank" and grind so the bass is
// heard under down-tuned guitars), then a few for other styles. They're our
// own starting points from common practice, not copies of any player's
// settings. Some also switch the EQ's ULTRA LO / ULTRA HI, as you would on the
// amp. Choosing one sets the faders, the two switches and LEVEL; from there
// you tweak as usual. LEVEL makes up for the loudness the curve adds or takes
// away (measured over a bass's range, 40 Hz to 5 kHz), so switching between
// presets compares tone, not volume.
namespace EqPresets
{
    struct Preset
    {
        const char* name;
        const char* description;                 // the menu's tooltip
        std::array<float, 9> gainsDb;            // 33, 80, 150, 300, 500, 900, 2k, 5k, 9k
        float levelDb;
        bool ultraLo = false, ultraHi = false;
    };

    inline constexpr std::array<Preset, 16> all { {
        // Modern metal.
        { "Modern Metal Clank",   "Tight lows, the mud cut, and a bright clank at 2-5K: the modern metal bass sound that's heard under heavy guitars.",
          { -3.0f, 3.0f, 0.0f, -3.0f, -2.0f, 2.0f, 5.0f, 4.0f, -2.0f }, -2.0f },
        { "Djent Growl",          "A deep low-mid scoop and a big, gnarly 900 Hz-2K growl, with the fizz rolled off. For down-tuned, extended-range riffs.",
          { -4.0f, 2.0f, -1.0f, -4.0f, 0.0f, 4.0f, 6.0f, 2.0f, -4.0f }, -2.5f },
        { "Tight Low End",        "The deep sub boom cut and the punch at 80 Hz lifted, so fast picking and double-kick parts stay tight and defined.",
          { -6.0f, 4.0f, 1.0f, -3.0f, -1.0f, 1.0f, 3.0f, 2.0f, -1.0f }, -1.0f },
        { "Drop-Tuned Weight",    "More sub at 33 Hz for low B, A or F# strings, with the low mids cleared so all that weight doesn't turn to mud.",
          { 4.0f, 2.0f, -2.0f, -3.0f, -1.0f, 0.0f, 2.0f, 2.0f, -2.0f }, -0.5f },
        { "Grind Through the Guitars", "Less low end, more upper mids: the bass's grind sits in the gaps between distorted guitars instead of fighting them.",
          { -4.0f, 1.0f, -2.0f, -2.0f, 2.0f, 4.0f, 5.0f, 1.0f, -3.0f }, -2.5f },
        { "Thrash Pick Attack",   "Mid-forward and bright, so every picked note in a fast riff is heard.",
          { -2.0f, 1.0f, -1.0f, 0.0f, 3.0f, 3.0f, 3.0f, 4.0f, 0.0f }, -2.5f },
        { "Classic Metal Scoop",  "The amp's ULTRA LO and ULTRA HI together: the big, hollow, scooped tone of '80s and '90s metal.",
          { 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f }, 0.0f, true, true },
        { "Doom Fuzz",            "Thick lows and low mids with the top rolled off: heavy and dark for doom and sludge fuzz.",
          { 3.0f, 4.0f, 3.0f, 2.0f, 0.0f, -1.0f, -2.0f, -4.0f, -6.0f }, -2.0f },
        { "Fizz Tamer",           "Rolls off the harsh, buzzy top end that distorted bass can have, leaving the rest alone.",
          { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, -5.0f, -9.0f }, 0.5f },
        { "Mud Cut",              "Clears the boxy, muddy low mids without thinning the tone.",
          { 0.0f, 0.0f, -3.0f, -4.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f }, 1.5f },

        // Other styles.
        { "Clean DI Polish",      "A gentle studio-style polish for clean bass: a little less boom and box, a little more definition.",
          { -2.0f, 1.0f, 0.0f, -2.0f, 0.0f, 1.0f, 1.0f, 2.0f, 1.0f }, -0.5f },
        { "Slap Funk",            "Full lows, scooped mids and ULTRA HI's sparkle, for slap and pop.",
          { 2.0f, 3.0f, -1.0f, -3.0f, -2.0f, 0.0f, 1.0f, 1.0f, 0.0f }, -1.0f, false, true },
        { "Motown Thump",         "Round, warm and dark, like flatwound strings: the lows full and the highs rolled right off.",
          { 1.0f, 4.0f, 2.0f, 0.0f, -1.0f, -2.0f, -4.0f, -6.0f, -8.0f }, -0.5f },
        { "Fingerstyle Growl",    "Warm lows and a woody 500-900 Hz growl for fingerstyle rock.",
          { 0.0f, 2.0f, 0.0f, -1.0f, 2.0f, 3.0f, 1.0f, 0.0f, -1.0f }, -1.5f },
        { "Fretless Mwah",        "Lifts the vocal 500 Hz-2K range that gives fretless its singing \"mwah\".",
          { -1.0f, 1.0f, 0.0f, -1.0f, 2.0f, 3.0f, 2.0f, 0.0f, -2.0f }, -1.5f },
        { "Radio",                "A narrow mid band, like an old AM radio or a phone: for intros and breakdowns.",
          { -12.0f, -12.0f, -11.0f, -5.5f, 2.0f, 5.5f, 3.0f, -8.0f, -12.0f }, 0.5f },
    } };
}
