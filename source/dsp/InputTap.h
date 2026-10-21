#pragma once

#include <atomic>
#include <vector>

// Lets the tuner look at the guitar signal: the audio thread writes the
// input into a ring buffer, and the UI thread copies out the most recent
// samples whenever it wants to measure the pitch. Lock-free, and the audio
// thread never waits.
class InputTap
{
public:
    explicit InputTap (int capacity = 16384) : samples ((size_t) capacity) {}

    // Audio thread.
    void write (const float* data, int numSamples) noexcept
    {
        const auto capacity = (int) samples.size();
        auto position = writePosition.load (std::memory_order_relaxed);
        for (int i = 0; i < numSamples; ++i)
        {
            samples[(size_t) position].store (data[i], std::memory_order_relaxed);
            position = (position + 1) % capacity;
        }
        writePosition.store (position, std::memory_order_release);
    }

    // Any other thread: copies the newest `count` samples, oldest first.
    void readLatest (float* destination, int count) const noexcept
    {
        const auto capacity = (int) samples.size();
        const auto end = writePosition.load (std::memory_order_acquire);
        for (int i = 0; i < count; ++i)
            destination[i] = samples[(size_t) ((end - count + i + capacity * 2) % capacity)].load (std::memory_order_relaxed);
    }

    int capacity() const noexcept { return (int) samples.size(); }

private:
    std::vector<std::atomic<float>> samples;
    std::atomic<int> writePosition { 0 };
};
