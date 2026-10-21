#include "FactoryRigs.h"

#include "Parameters.h"

namespace
{
    Rig make (const juce::String& name, int bank, int number, const juce::String& tags,
              std::initializer_list<std::pair<const char*, float>> changes)
    {
        Rig rig;
        rig.name = name;
        rig.bank = bank;
        rig.number = number;
        rig.tags = juce::StringArray::fromTokens (tags, ",", {});
        rig.keepAmpAndCab = true;
        // Only the changes from the defaults: loading a rig resets every
        // parameter it doesn't mention to its default (every effect off).
        for (const auto& [id, value] : changes)
            rig.parameters[id] = value;
        return rig;
    }

    // Delay sync choices (index into effects::Delay::syncNames).
    constexpr float syncOff = 0, quarter = 3, dottedEighth = 7;
    // Modulation types.
    constexpr float chorus = 0, flanger = 1, tremolo = 3;
}

std::vector<Rig> makeFactoryRigs()
{
    using namespace ParamIDs;
    constexpr int A = 0, B = 1, C = 2, D = 3;

    return {
        make ("Clean Slate", A, 1, "Clean", {}),

        make ("Studio Clean", A, 2, "Clean", {
            { compOn, 1 }, { compThreshold, -24 }, { compRatio, 3 }, { compLevel, 3 },
            { reverbOn, 1 }, { reverbSize, 0.4f }, { reverbMix, 0.2f } }),

        make ("Chorus Clean", A, 3, "Clean,Modulation", {
            { compOn, 1 }, { compThreshold, -22 }, { compRatio, 3 }, { compLevel, 2 },
            { modOn, 1 }, { modType, chorus }, { modRate, 0.6f }, { modDepth, 0.5f }, { modMix, 0.5f },
            { reverbOn, 1 }, { reverbMix, 0.25f } }),

        make ("Edge of Breakup", A, 4, "Crunch,Drive", {
            { driveOn, 1 }, { driveGain, 2 }, { driveTone, 6 }, { driveLevel, 4 },
            { reverbOn, 1 }, { reverbMix, 0.2f } }),

        make ("Lead Boost", B, 1, "Lead,Drive", {
            { compThreshold, -20 }, { gateThreshold, -60 }, { compOn, 1 }, { compRatio, 2 },
            { driveOn, 1 }, { driveGain, 4 }, { driveTone, 6 }, { driveLevel, 6 },
            { delayOn, 1 }, { delaySync, dottedEighth }, { delayFeedback, 0.3f }, { delayMix, 0.2f },
            { reverbOn, 1 }, { reverbMix, 0.25f }, { rigLevel, 2 } }),

        // A low-gain overdrive in front of a high-gain amp tightens the bass
        // and sharpens the attack: the classic metal "boost" trick.
        make ("High Gain Tight", B, 2, "High Gain,Rhythm", {
            { compOn, 1 }, { compThreshold, 0 }, { gateThreshold, -55 },
            { driveOn, 1 }, { driveGain, 1 }, { driveTone, 6 }, { driveLevel, 8 },
            { presence, 2 } }),

        make ("Ambient Swells", C, 1, "Ambient,Clean", {
            { compOn, 1 }, { compThreshold, -26 }, { compRatio, 4 }, { compLevel, 4 },
            { modOn, 1 }, { modType, chorus }, { modRate, 0.25f }, { modDepth, 0.6f }, { modMix, 0.35f },
            { delayOn, 1 }, { delaySync, quarter }, { delayFeedback, 0.6f }, { delayMix, 0.35f },
            { delayTone, 5 }, { delayPingPong, 1 },
            { reverbOn, 1 }, { reverbSize, 0.9f }, { reverbTone, 5 }, { reverbMix, 0.45f } }),

        make ("Slapback", C, 2, "Rockabilly,Delay", {
            { delayOn, 1 }, { delaySync, syncOff }, { delayTime, 110 }, { delayFeedback, 0.1f },
            { delayMix, 0.3f }, { delayTone, 5 },
            { reverbOn, 1 }, { reverbSize, 0.25f }, { reverbMix, 0.15f } }),

        make ("Surf Tremolo", C, 3, "Surf,Modulation", {
            { modOn, 1 }, { modType, tremolo }, { modRate, 6 }, { modDepth, 0.6f },
            { reverbOn, 1 }, { reverbSize, 0.7f }, { reverbTone, 7 }, { reverbMix, 0.4f } }),

        make ("Jet Flanger", D, 1, "Modulation,Drive", {
            { driveOn, 1 }, { driveGain, 3 },
            { modOn, 1 }, { modType, flanger }, { modRate, 0.2f }, { modDepth, 0.8f }, { modMix, 0.5f } }),
    };
}
