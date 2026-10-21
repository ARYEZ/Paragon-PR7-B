#include <catch2/catch_test_macros.hpp>

#include "dsp/RealtimeSwap.h"

namespace
{
    // Counts live instances, so tests can see exactly when objects are freed.
    struct Tracked
    {
        explicit Tracked (int idToUse) : id (idToUse) { ++alive; }
        ~Tracked() { --alive; }
        int id;
        static inline int alive = 0;
    };
}

TEST_CASE ("RealtimeSwap hands objects to the audio thread and frees old ones safely", "[swap]")
{
    Tracked::alive = 0;
    {
        RealtimeSwap<Tracked> swap;

        SECTION ("Nothing is active until a block starts")
        {
            swap.setPending (std::make_unique<Tracked> (1));
            CHECK (Tracked::alive == 1);
            auto block = swap.beginBlock();
            REQUIRE (block.current != nullptr);
            CHECK (block.current->id == 1);
            CHECK (block.previous == nullptr);   // nothing was active before
            swap.endBlock();
        }

        SECTION ("Staging twice before a block keeps only the newest")
        {
            swap.setPending (std::make_unique<Tracked> (1));
            swap.setPending (std::make_unique<Tracked> (2));
            CHECK (Tracked::alive == 1);
            CHECK (swap.beginBlock().current->id == 2);
            swap.endBlock();
        }

        SECTION ("The old object stays alive for the rest of the swap block")
        {
            swap.setPending (std::make_unique<Tracked> (1));
            swap.beginBlock();
            swap.endBlock();

            swap.setPending (std::make_unique<Tracked> (2));
            const auto block = swap.beginBlock();
            REQUIRE (block.previous != nullptr);
            CHECK (block.previous->id == 1);
            CHECK (block.current->id == 2);

            swap.collectGarbage();          // mid-block: must not free #1
            CHECK (Tracked::alive == 2);
            CHECK (block.previous->id == 1);

            swap.endBlock();
            swap.collectGarbage();          // block finished: now it's freed
            CHECK (Tracked::alive == 1);
        }

        SECTION ("Staging nullptr unloads")
        {
            swap.setPending (std::make_unique<Tracked> (1));
            swap.beginBlock();
            swap.endBlock();

            swap.setPending (nullptr);
            const auto block = swap.beginBlock();
            CHECK (block.current == nullptr);
            CHECK (block.previous != nullptr);
            swap.endBlock();
            swap.collectGarbage();
            CHECK (Tracked::alive == 0);
        }

        SECTION ("A second swap waits until the first old object is freed")
        {
            swap.setPending (std::make_unique<Tracked> (1));
            swap.beginBlock();
            swap.endBlock();
            swap.setPending (std::make_unique<Tracked> (2));
            swap.beginBlock();
            swap.endBlock();

            // #1 is retired but not collected yet, so #3 must wait: the audio
            // thread is never allowed to free #1 itself.
            swap.setPending (std::make_unique<Tracked> (3));
            CHECK (swap.beginBlock().current->id == 2);
            swap.endBlock();

            swap.collectGarbage();
            CHECK (swap.beginBlock().current->id == 3);
            swap.endBlock();
        }
    }
    CHECK (Tracked::alive == 0);   // nothing leaks when the swap is destroyed
}
