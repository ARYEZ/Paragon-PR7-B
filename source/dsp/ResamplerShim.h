#pragma once

// AudioDSPTools' resampler was lifted from the iPlug2 framework and still
// reads two constants from it. Provide them (with iPlug2's values) so the
// resampler can be used without iPlug2.
namespace iplug
{
    inline constexpr double PI = 3.14159265358979323846;
}

#ifndef DEFAULT_BLOCK_SIZE
 #define DEFAULT_BLOCK_SIZE 1024
#endif
