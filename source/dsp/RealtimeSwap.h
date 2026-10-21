#pragma once

#include <juce_core/juce_core.h>

#include <atomic>
#include <cstdint>
#include <memory>

// Hands a heavy object (a loaded NAM model) from a loader thread to the audio
// thread without the audio thread ever blocking, allocating or freeing memory.
//
//   loader thread:  setPending()   - stages a fully prepared object
//   audio thread:   beginBlock()   - swaps the staged object in, if any
//                   endBlock()     - marks the block finished
//   other threads:  collectGarbage() - frees the object that was swapped out
//
// The audio thread only ever *tries* the lock. If a loader holds it for the
// few nanoseconds a pointer swap takes, the audio thread keeps using the
// current object and picks up the new one on the next block.
template <typename Object>
class RealtimeSwap
{
public:
    struct Block
    {
        Object* current = nullptr;
        Object* previous = nullptr;   // non-null only in the block where a swap happened
    };

    // Any non-audio thread. Replaces a staged object that hasn't been picked up yet.
    void setPending (std::unique_ptr<Object> object)
    {
        std::unique_ptr<Object> replaced;
        {
            const juce::SpinLock::ScopedLockType lock (mutex);
            replaced = std::exchange (pending, std::move (object));
            hasPending = true;
        }
        // `replaced` is destroyed here, outside the lock and off the audio thread.
    }

    // Audio thread, at the start of each block.
    Block beginBlock() noexcept
    {
        Block block;

        if (hasPending.load (std::memory_order_acquire))
        {
            const juce::SpinLock::ScopedTryLockType lock (mutex);

            // Only swap once the previously retired object has been freed, so
            // the audio thread never has to free anything itself.
            if (lock.isLocked() && retired == nullptr)
            {
                retired = std::exchange (active, std::move (pending));
                retiredAtBlock = blocksFinished.load (std::memory_order_relaxed);
                hasPending = false;
                block.previous = retired.get();
            }
        }

        block.current = active.get();
        return block;
    }

    // Audio thread, at the end of each block.
    void endBlock() noexcept { blocksFinished.fetch_add (1, std::memory_order_release); }

    // Any non-audio thread. Frees the swapped-out object once the audio thread
    // has finished the block in which it last used it.
    void collectGarbage()
    {
        std::unique_ptr<Object> toFree;
        {
            const juce::SpinLock::ScopedLockType lock (mutex);
            if (retired != nullptr && blocksFinished.load (std::memory_order_acquire) > retiredAtBlock)
                toFree = std::move (retired);
        }
    }

    // Only while audio processing is stopped (prepareToPlay, destructor, tests).
    // Visits the newest object: the staged one if there is one, else the active one.
    template <typename Function>
    void withNewestUnsafe (Function&& function)
    {
        const juce::SpinLock::ScopedLockType lock (mutex);
        if (auto* object = hasPending ? pending.get() : active.get())
            function (*object);
    }

    template <typename Function>
    void forEachUnsafe (Function&& function)
    {
        const juce::SpinLock::ScopedLockType lock (mutex);
        for (auto* object : { active.get(), pending.get(), retired.get() })
            if (object != nullptr)
                function (*object);
    }

private:
    juce::SpinLock mutex;
    std::unique_ptr<Object> active, pending, retired;
    std::atomic<bool> hasPending { false };
    std::atomic<std::uint64_t> blocksFinished { 0 };
    std::uint64_t retiredAtBlock = 0;
};
