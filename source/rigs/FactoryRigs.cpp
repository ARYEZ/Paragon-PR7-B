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
    constexpr float syncOff = 0, quarter = 3, eighth = 6, dottedEighth = 7;
    // Modulation types (index into effects::Modulation::typeNames).
    constexpr float chorus = 0, flanger = 1, tremolo = 3, dimension = 4, uniVibe = 5, rotary = 6, vibrato = 7,
                    harmonicTremolo = 8, autoPan = 9, doubler = 10;
    // FILTER / PITCH types.
    constexpr float autoWah = 0, pitchShifter = 1;
    // LO-FI types.
    constexpr float telephone = 0, amRadio = 1, megaphone = 2, cassette = 3, bitcrusher = 4;
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

        make ("Leslie Spin", D, 2, "Modulation,Clean", {
            { compOn, 1 }, { compThreshold, -20 }, { compRatio, 3 },
            { modOn, 1 }, { modType, rotary }, { modRate, 6.5f }, { modDepth, 0.7f }, { modMix, 0.85f },
            { reverbOn, 1 }, { reverbSize, 0.4f }, { reverbMix, 0.2f } }),

        make ("Machine Gun Vibe", D, 3, "Modulation,Drive,Lead", {
            { driveOn, 1 }, { driveGain, 6 }, { driveTone, 5 }, { driveLevel, 3 },
            { modOn, 1 }, { modType, uniVibe }, { modRate, 1.2f }, { modDepth, 0.75f }, { modMix, 0.5f },
            { reverbOn, 1 }, { reverbMix, 0.18f } }),

        make ("Dimension Clean", D, 4, "Clean,Modulation", {
            { compOn, 1 }, { compThreshold, -22 }, { compRatio, 3 }, { compLevel, 2 },
            { modOn, 1 }, { modType, dimension }, { modRate, 0.4f }, { modDepth, 0.5f }, { modMix, 0.5f },
            { delayOn, 1 }, { delaySync, dottedEighth }, { delayFeedback, 0.25f }, { delayMix, 0.15f },
            { reverbOn, 1 }, { reverbSize, 0.6f }, { reverbMix, 0.25f } }),

        make ("Warble Vibrato", D, 5, "Modulation,Lo-Fi", {
            { modOn, 1 }, { modType, vibrato }, { modRate, 5.5f }, { modDepth, 0.35f },
            { reverbOn, 1 }, { reverbSize, 0.5f }, { reverbTone, 4 }, { reverbMix, 0.3f } }),

        make ("Octave Lead", B, 3, "Lead,Pitch,Drive", {
            { compOn, 1 }, { compThreshold, -20 }, { compRatio, 3 },
            { fpOn, 1 }, { fpType, pitchShifter }, { fpInterval, 12 }, { fpMix, 0.45f },
            { driveOn, 1 }, { driveGain, 5 }, { driveTone, 6 }, { driveLevel, 2 },
            { delayOn, 1 }, { delaySync, dottedEighth }, { delayFeedback, 0.3f }, { delayMix, 0.2f },
            { reverbOn, 1 }, { reverbMix, 0.2f } }),

        make ("Harmony Fifths", B, 4, "Lead,Pitch", {
            { fpOn, 1 }, { fpType, pitchShifter }, { fpInterval, 7 }, { fpMix, 0.4f },
            { driveOn, 1 }, { driveGain, 4 }, { driveTone, 5 },
            { reverbOn, 1 }, { reverbSize, 0.6f }, { reverbMix, 0.25f } }),

        make ("Funk Envelope", C, 6, "Funk,Filter,Clean", {
            { compOn, 1 }, { compThreshold, -26 }, { compRatio, 4 }, { compLevel, 4 },
            { fpOn, 1 }, { fpType, autoWah }, { fpSensitivity, 6 }, { fpResonance, 6 }, { fpMix, 1 },
            { reverbOn, 1 }, { reverbSize, 0.3f }, { reverbMix, 0.15f } }),

        make ("Wide Doubler", D, 6, "Rhythm,Modulation", {
            { driveOn, 1 }, { driveGain, 3 }, { driveTone, 5 },
            { modOn, 1 }, { modType, doubler }, { modRate, 0.8f }, { modDepth, 0.4f }, { modMix, 0.7f },
            { reverbOn, 1 }, { reverbSize, 0.35f }, { reverbMix, 0.15f } }),

        // LO-FI: for intros and breakdowns. Assign the expression pedal to
        // LO-FI to sweep from the full band (heel) into the effect (toe).
        make ("Phone Intro", B, 5, "Lo-Fi,Clean", {
            { compOn, 1 }, { compThreshold, -22 }, { compRatio, 3 },
            { reverbOn, 1 }, { reverbSize, 0.35f }, { reverbMix, 0.2f },
            { lofiOn, 1 }, { lofiType, telephone }, { lofiAmount, 6 }, { lofiNoise, 3 }, { lofiLevel, 2 } }),

        make ("Megaphone Verse", B, 6, "Lo-Fi,Drive", {
            { driveOn, 1 }, { driveGain, 5 }, { driveTone, 6 },
            { lofiOn, 1 }, { lofiType, megaphone }, { lofiAmount, 6 }, { lofiNoise, 2 } }),

        make ("Old Radio", C, 7, "Lo-Fi,Vintage", {
            { modOn, 1 }, { modType, tremolo }, { modRate, 4.5f }, { modDepth, 0.3f },
            { reverbOn, 1 }, { reverbSize, 0.5f }, { reverbMix, 0.3f },
            { lofiOn, 1 }, { lofiType, amRadio }, { lofiAmount, 5 }, { lofiNoise, 5 }, { lofiLevel, 1 } }),

        make ("Cassette Dreams", D, 7, "Lo-Fi,Ambient", {
            { compOn, 1 }, { compThreshold, -24 }, { compRatio, 3 },
            { modOn, 1 }, { modType, chorus }, { modRate, 0.4f }, { modDepth, 0.4f }, { modMix, 0.35f },
            { delayOn, 1 }, { delaySync, dottedEighth }, { delayFeedback, 0.35f }, { delayMix, 0.25f },
            { reverbOn, 1 }, { reverbSize, 0.8f }, { reverbMix, 0.35f },
            { lofiOn, 1 }, { lofiType, cassette }, { lofiAmount, 5 }, { lofiNoise, 4 } }),

        make ("8-Bit Crunch", D, 8, "Lo-Fi,Drive", {
            { driveOn, 1 }, { driveGain, 3 },
            { lofiOn, 1 }, { lofiType, bitcrusher }, { lofiAmount, 6 }, { lofiNoise, 0 }, { lofiMix, 0.8f } }),

        // These two follow the tempo (tapped, or the DAW's).
        make ("Brownface Trem", C, 4, "Modulation,Clean,Vintage", {
            { modOn, 1 }, { modType, harmonicTremolo }, { modSync, eighth }, { modDepth, 0.7f },
            { reverbOn, 1 }, { reverbSize, 0.55f }, { reverbTone, 6 }, { reverbMix, 0.3f } }),

        make ("Wide Pan", C, 5, "Modulation,Ambient", {
            { compOn, 1 }, { compThreshold, -24 }, { compRatio, 3 },
            { modOn, 1 }, { modType, autoPan }, { modSync, quarter }, { modDepth, 0.8f },
            { delayOn, 1 }, { delaySync, dottedEighth }, { delayFeedback, 0.35f }, { delayMix, 0.2f },
            { reverbOn, 1 }, { reverbSize, 0.8f }, { reverbMix, 0.35f } }),
    };
}
