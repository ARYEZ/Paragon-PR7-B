#include "TalkBox.h"

namespace effects
{
    namespace
    {
        // A voice's first three formants for each vowel, in Hz (averages for
        // an adult male voice), heel to toe.
        constexpr std::array<std::array<float, 3>, 5> vowels { {
            { 300.0f, 870.0f, 2240.0f },    // oo
            { 450.0f, 800.0f, 2830.0f },    // oh
            { 730.0f, 1090.0f, 2440.0f },   // ah
            { 530.0f, 1840.0f, 2480.0f },   // eh
            { 270.0f, 2290.0f, 3010.0f },   // ee
        } };
        // The higher formants are quieter in a voice.
        constexpr std::array<float, 3> formantGains { 1.0f, 0.6f, 0.3f };
        constexpr int retuneEvery = 16;   // samples between filter updates while the pedal moves
    }

    std::array<float, 3> TalkBox::formantsFor (float position) noexcept
    {
        // Glide from vowel to vowel, each formant moving evenly by ear (in log frequency).
        const auto scaled = juce::jlimit (0.0f, 1.0f, position) * (float) (vowels.size() - 1);
        const auto from = juce::jmin ((int) scaled, (int) vowels.size() - 2);
        const auto blend = scaled - (float) from;
        std::array<float, 3> result {};
        for (size_t f = 0; f < 3; ++f)
            result[f] = vowels[(size_t) from][f] * std::pow (vowels[(size_t) from + 1][f] / vowels[(size_t) from][f], blend);
        return result;
    }

    void TalkBox::prepare (double sampleRate, int maxBlockSize)
    {
        for (auto& filter : formants)
        {
            filter.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1 });
            filter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        }
        mouth.reset (sampleRate, 0.03);   // smooths a stepped MIDI pedal into a glide
        mix.reset (sampleRate, 0.05);
        bypass.prepare (sampleRate, maxBlockSize);
    }

    void TalkBox::process (juce::dsp::AudioBlock<float> monoBlock, bool on, float position, float resonance, float mixAmount) noexcept
    {
        bypass.process (monoBlock, on, [&] (juce::dsp::AudioBlock<float> block)
        {
            // Q from 4 (soft, open) to 16 (sharp, very vocal). JUCE's band-pass
            // peaks at a gain of Q: scale it back so the voice sits at about
            // the guitar's level.
            const auto q = 4.0f + 1.2f * juce::jlimit (0.0f, 10.0f, resonance);
            for (auto& filter : formants)
                filter.setResonance (q);
            const auto outputScale = 2.2f / q;

            mouth.setTargetValue (juce::jlimit (0.0f, 1.0f, position));
            mix.setTargetValue (juce::jlimit (0.0f, 1.0f, mixAmount));
            auto* samples = block.getChannelPointer (0);
            const auto numSamples = (int) block.getNumSamples();

            for (int start = 0; start < numSamples; start += retuneEvery)
            {
                const auto count = juce::jmin (retuneEvery, numSamples - start);
                const auto frequencies = formantsFor (mouth.getNextValue());
                mouth.skip (count - 1);
                for (size_t f = 0; f < 3; ++f)
                    formants[f].setCutoffFrequency (frequencies[f]);

                for (int i = start; i < start + count; ++i)
                {
                    auto voiced = 0.0f;
                    for (size_t f = 0; f < 3; ++f)
                        voiced += formants[f].processSample (0, samples[i]) * formantGains[f];
                    const auto mixNow = mix.getNextValue();
                    samples[i] = samples[i] * (1.0f - mixNow) + voiced * outputScale * mixNow;
                }
            }
        });
    }
}
