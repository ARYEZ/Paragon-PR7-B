#include "FactoryRigs.h"

#include "Parameters.h"

namespace
{
    Rig make (const juce::String& name, int bank, int number, const juce::String& tags,
              std::initializer_list<std::pair<const char*, float>> changes, const juce::String& notes = {})
    {
        Rig rig;
        rig.name = name;
        rig.notes = notes;
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
    constexpr float chorus = 0, flanger = 1, phaser = 2, tremolo = 3, dimension = 4, uniVibe = 5, rotary = 6, vibrato = 7,
                    harmonicTremolo = 8, autoPan = 9, doubler = 10;
    // FILTER / PITCH types.
    constexpr float autoWah = 0, pitchShifter = 1, whammyWah = 2, talkBoxType = 3;
    constexpr float pedalTalkBox = 8;   // the expression pedal's TALK BOX setting
    constexpr float daybreakComp = 1;   // COMP / GATE's TYPE
    constexpr float glitchSequence = 1, glitchTexturizer = 3, glitchSixteenth = 3;   // GLITCH's MODE and LENGTH
    constexpr float pedalGlitch = 7;   // the expression pedal's GLITCH setting
    constexpr float pedalWah = 2;      // the expression pedal's WAH / FILTER setting
    constexpr float glitchKillswitch = 4;
    constexpr int E = 4;               // bank E: rigs inspired by particular songs
    constexpr float digitalDelay = 0;
    constexpr float xpCryWah = 1, xpUpOctave = 12, xpOctDownOctUp = 20, xpUp4thUp5th = 23;   // Whammy-Wah programs 02, 13, 21 and 24
    constexpr float pedalWhammyWah = 6;   // the expression pedal's WHAMMY-WAH setting
    // Delay types (index into effects::Delay::typeNames).
    constexpr float analogDelay = 1, tapeDelay = 2, modDelay = 3, reverseDelay = 4, multiTap = 5, shimmer = 6, ducking = 7;
    // Reverb types (index into effects::Reverb::typeNames).
    constexpr float room = 1, hall = 2, cathedral = 3, plate = 4, spring = 5, digital = 6, reverse = 7;
    // LO-FI types.
    constexpr float telephone = 0, amRadio = 1, megaphone = 2, cassette = 3, bitcrusher = 4;
    // The graphic EQ's bands, by their parameter IDs (ParamIDs::eqBands).
    constexpr auto eq33 = "eq33", eq80 = "eq80", eq150 = "eq150", eq300 = "eq300", eq500 = "eq500",
                   eq900 = "eq900", eq2k = "eq2k", eq5k = "eq5k", eq9k = "eq9k";
}

std::vector<Rig> makeFactoryRigs()
{
    using namespace ParamIDs;
    constexpr int A = 0, B = 1, C = 2;
    constexpr float obsidian = 0, gotham = 1;   // OD / DISTORTION's MODEL

    // The bass factory rigs (PR7-B 0.8.0; PR7-G's guitar rigs are gone).
    // A: clean and basics. B: drive. C: effects. E: songs.
    //
    // The song rigs follow what's documented about each recording (the
    // research notes behind them are summarised in each rig's notes): the
    // bassist's drive, EQ and effects, built from PR7-B's blocks. They're
    // our interpretation, "inspired by", not copies. Each rig's notes say
    // which amp capture and cabinet IR to load, since rigs can't ship them.
    return {
        // ---- A: clean and basics -----------------------------------------
        make ("Clean DI", A, 1, "Clean", {},
              "Nothing on: your bass, amp and cab as they are. A starting point."),

        make ("Studio Bass", A, 2, "Clean,Compressor", { { rigLevel, 1 },
            { compOn, 1 }, { compThreshold, -22 }, { compRatio, 4 }, { compLevel, 3 },
            { eqOn, 1 }, { eq33, -2 }, { eq80, 1 }, { eq300, -2 }, { eq900, 1 }, { eq2k, 1 }, { eq5k, 2 }, { eq9k, 1 }, { eqLevel, -0.5f } },
            "An evened-out, polished clean bass for recording: 4:1 compression and the Clean DI Polish EQ.\n"
            "Amp: a clean Ampeg SVT or B-15 capture, or none for a pure DI.\nCab: an 8x10 or 1x15 IR, or none."),

        make ("Fingerstyle Round", A, 3, "Clean,Compressor", { { rigLevel, -2 },
            { compOn, 1 }, { compType, daybreakComp }, { compAmount, 4 }, { compDetail, 6 },
            { eqOn, 1 }, { eq80, 2 }, { eq300, -1 }, { eq500, 2 }, { eq900, 3 }, { eq2k, 1 }, { eq9k, -1 }, { eqLevel, -1.5f } },
            "Warm lows and a woody growl for fingerstyle rock.\n"
            "Amp: a clean Ampeg SVT capture.\nCab: an SVT 8x10 IR."),

        make ("Slap Funk", A, 4, "Clean,Funk,Compressor", {
            { compOn, 1 }, { compThreshold, -26 }, { compRatio, 6 }, { compLevel, 4 },
            { eqOn, 1 }, { eq33, 2 }, { eq80, 3 }, { eq150, -1 }, { eq300, -3 }, { eq500, -2 }, { eq2k, 1 }, { eq5k, 1 },
            { eqLevel, -1 }, { eqUltraHi, 1 } },
            "Full lows, scooped mids and sparkle for slap and pop.\n"
            "Amp: a clean Gallien-Krueger or Ampeg capture.\nCab: a 4x10 IR with a tweeter."),

        make ("Motown Thump", A, 5, "Clean,Vintage", {
            { compOn, 1 }, { compThreshold, -20 }, { compRatio, 3 },
            { eqOn, 1 }, { eq33, 1 }, { eq80, 4 }, { eq150, 2 }, { eq500, -1 }, { eq900, -2 }, { eq2k, -4 }, { eq5k, -6 }, { eq9k, -8 },
            { eqLevel, -0.5f } },
            "Round, warm and dark, like flatwound strings: the highs rolled right off.\n"
            "Amp: an Ampeg B-15 capture, or none.\nCab: a 1x15 IR, or none."),

        make ("Fretless Mwah", A, 6, "Clean,Modulation", { { rigLevel, 5 },
            { compOn, 1 }, { compThreshold, -24 }, { compRatio, 3 }, { compLevel, 2 },
            { eqOn, 1 }, { eq33, -1 }, { eq80, 1 }, { eq300, -1 }, { eq500, 2 }, { eq900, 3 }, { eq2k, 2 }, { eq9k, -2 }, { eqLevel, -1.5f },
            { modOn, 1 }, { modType, chorus }, { modRate, 0.6f }, { modDepth, 0.3f }, { modMix, 0.25f } },
            "Lifts the vocal 500 Hz to 2 kHz range that makes a fretless sing, with a touch of chorus.\n"
            "Amp: a clean Ampeg or Mesa capture.\nCab: a 4x10 or 8x10 IR."),

        // ---- B: drive -----------------------------------------------------
        make ("Gotham Grit", B, 1, "Drive", { { rigLevel, 7 },
            { driveOn, 1 }, { driveType, gotham }, { gothamLevel, 6 }, { gothamBlend, 6 }, { gothamDrive, 4 }, { gothamPresence, 5 } },
            "The GOTHAM (modelled on the SansAmp Bass Driver DI) at a gentle grit, half blended with the clean bass.\n"
            "Amp: none, or a clean amp capture.\nCab: an 8x10 or 4x10 IR, or none (it imitates an amp and cab itself)."),

        make ("Gotham Cranked", B, 2, "Drive,Rock", { { rigLevel, -7.5f },
            { driveOn, 1 }, { driveType, gotham }, { gothamLevel, 4 }, { gothamBlend, 8 }, { gothamDrive, 10 }, { gothamPresence, 6 },
            { gothamBass, 6 } },
            "The GOTHAM flat out: the classic cranked SansAmp snarl.\n"
            "Amp: none, or a clean amp capture.\nCab: an 8x10 IR."),

        make ("Obsidian Tight", B, 3, "Drive,Metal", { { rigLevel, 5 },
            { compOn, 1 }, { compThreshold, -20 }, { compRatio, 4 },
            { driveOn, 1 }, { driveType, obsidian }, { driveGain, 6 }, { dualMod, 0 }, { dualBlend, 5 }, { dualLevel, 4.5f }, { dualBite, 1 } },
            "The OBSIDIAN (modelled on the Darkglass Alpha·Omega) on its ALPHA side: tight, focused grind, clean lows blended under.\n"
            "Amp: a clean Ampeg SVT capture, or none.\nCab: an SVT 8x10 IR."),

        make ("Obsidian Raw", B, 4, "Drive,Metal", { { rigLevel, 2 },
            { driveOn, 1 }, { driveType, obsidian }, { driveGain, 7 }, { dualMod, 10 }, { dualBlend, 6 }, { dualLevel, 4 }, { dualGrowl, 1 } },
            "The OBSIDIAN on its OMEGA side with GROWL: raw, brutal and fat.\n"
            "Amp: a clean Ampeg SVT capture, or none.\nCab: an SVT 8x10 IR."),

        make ("Modern Metal Clank", B, 5, "Drive,Metal", { { rigLevel, 9.5f },
            { compOn, 1 }, { compThreshold, -22 }, { compRatio, 4 },
            { driveOn, 1 }, { driveType, obsidian }, { driveGain, 6.5f }, { dualMod, 5 }, { dualBlend, 5 }, { dualLevel, 4 }, { dualBite, 1 },
            { eqOn, 1 }, { eq33, -3 }, { eq80, 3 }, { eq300, -3 }, { eq500, -2 }, { eq900, 2 }, { eq2k, 5 }, { eq5k, 4 }, { eq9k, -2 },
            { eqLevel, -2 } },
            "Tight lows, the low-mid mud cut and a bright 2-5 kHz clank: heard under heavy guitars.\n"
            "Amp: a clean Ampeg SVT capture.\nCab: an SVT 8x10 IR (MD421 style)."),

        make ("Doom Fuzz", B, 6, "Drive,Fuzz", { { rigLevel, 1 },
            { driveOn, 1 }, { driveType, obsidian }, { driveGain, 9 }, { dualMod, 10 }, { dualBlend, 7 }, { dualLevel, 3.5f }, { dualGrowl, 1 },
            { dualTreble, 3.5f },
            { eqOn, 1 }, { eq33, 3 }, { eq80, 4 }, { eq150, 3 }, { eq300, 2 }, { eq900, -1 }, { eq2k, -2 }, { eq5k, -4 }, { eq9k, -6 },
            { eqLevel, -2 } },
            "Thick, dark and heavy for doom and sludge: OMEGA side, GROWL, the top rolled off.\n"
            "Amp: an Orange or Ampeg capture.\nCab: an 8x10 or 4x12 IR."),

        // ---- C: effects ---------------------------------------------------
        make ("Envelope Funk", C, 1, "Funk,Filter", { { rigLevel, 6.5f },
            { compOn, 1 }, { compThreshold, -22 }, { compRatio, 4 }, { compLevel, 8 },
            { fpOn, 1 }, { fpType, autoWah }, { fpSensitivity, 6 }, { fpResonance, 6 }, { fpMix, 0.75f } },
            "An envelope filter that opens as you dig in: classic funk quack.\n"
            "Amp: a clean Ampeg or GK capture.\nCab: a 4x10 IR."),

        make ("Synth Octave", C, 2, "Synth,Filter,Octave", { { rigLevel, 7.5f },
            { octOn, 1 }, { octSubOne, 0.8f }, { octDry, 0.6f },
            { compOn, 1 }, { compThreshold, -20 }, { compRatio, 4 }, { compLevel, 8 },
            { fpOn, 1 }, { fpType, autoWah }, { fpSensitivity, 7 }, { fpResonance, 7 }, { fpMix, 0.7f },
            { driveOn, 1 }, { driveType, gotham }, { gothamDrive, 5 }, { gothamBlend, 4 }, { driveLevel, 8 } },
            "Octave down into a resonant envelope filter, with a little grit: a synth-bass sound. Play one note at a time.\n"
            "Amp: a clean amp capture, or none.\nCab: an 8x10 IR, or none."),

        make ("Sub Octave", C, 3, "Octave", { { rigLevel, 1 },
            { octOn, 1 }, { octSubOne, 0.7f }, { octDry, 1 },
            { compOn, 1 }, { compThreshold, -20 }, { compRatio, 3 } },
            "A sub-octave under your notes for weight. Play one note at a time.\n"
            "Amp: a clean Ampeg SVT capture.\nCab: an 8x10 IR."),

        make ("Chorus Bass", C, 4, "Modulation", { { rigLevel, 6 },
            { compOn, 1 }, { compThreshold, -22 }, { compRatio, 3 },
            { modOn, 1 }, { modType, chorus }, { modRate, 0.7f }, { modDepth, 0.45f }, { modMix, 0.4f } },
            "A shimmering bass chorus for clean, melodic parts.\n"
            "Amp: a clean amp capture.\nCab: a 4x10 IR."),

        make ("Flanged Bass", C, 5, "Modulation", { { rigLevel, 3 },
            { modOn, 1 }, { modType, flanger }, { modRate, 0.25f }, { modDepth, 0.6f }, { modMix, 0.5f } },
            "A slow jet-plane flanger sweep.\n"
            "Amp: a clean amp capture.\nCab: a 4x10 or 8x10 IR."),

        make ("Dub Delay", C, 6, "Delay,Reggae", { { rigLevel, 2 },
            { compOn, 1 }, { compThreshold, -22 }, { compRatio, 4 },
            { eqOn, 1 }, { eq80, 3 }, { eq150, 2 }, { eq2k, -3 }, { eq5k, -6 }, { eq9k, -8 }, { eqLevel, -1 },
            { delayOn, 1 }, { delayType, analogDelay }, { delaySync, dottedEighth }, { delayFeedback, 0.4f }, { delayMix, 0.3f }, { delayTone, 4 } },
            "Deep, round dub bass with dark echoes in time (tap the tempo, or follow the DAW's).\n"
            "Amp: an Ampeg B-15 or SVT capture.\nCab: a 1x15 or 8x10 IR."),

        make ("Tremolo Pulse", C, 7, "Modulation", { { rigLevel, 3 },
            { modOn, 1 }, { modType, tremolo }, { modSync, eighth }, { modDepth, 0.7f } },
            "Tremolo in time with the song: chopping eighth notes.\n"
            "Amp: a clean amp capture.\nCab: a 4x10 IR."),

        // ---- E: songs -----------------------------------------------------
        // Muse, "Hysteria": producer Rich Costey described three amps at once,
        // one clean, one through a Big Muff, one through a Japanese
        // distortion; a synth reportedly doubles it. Fuzz over a clean low end.
        make ("Hysteria (Muse)", E, 1, "Song,Fuzz,Drive", { { rigLevel, 5.5f },
            { compOn, 1 }, { compThreshold, -20 }, { compRatio, 4 },
            { driveOn, 1 }, { driveType, obsidian }, { driveGain, 8.5f }, { dualMod, 9 }, { dualBlend, 5 }, { dualLevel, 4 },
            { dualGrowl, 1 }, { dualBite, 1 }, { dualMid, 6 },
            { octOn, 1 }, { octSubOne, 0.25f }, { octDry, 1 },
            { eqOn, 1 }, { eq300, -1 }, { eq900, 3 }, { eq2k, 3 }, { eq9k, -2 }, { eqLevel, -1.5f } },
            "Chris Wolstenholme's fuzz: buzzy and mid-forward over a solid clean low end, with a hint of octave down for the synth "
            "layer. The studio blended a clean amp with a Big Muff and a Japanese distortion (as producer Rich Costey described it).\n"
            "Amp: a clean Ampeg SVT capture (Marshall bass captures are rare).\nCab: a 4x10 or 8x10 IR.\n"
            "Play: with a pick, E standard, about 94 BPM."),

        // Tool, "Forty Six & 2": Justin Chancellor on a Wal, with a pick;
        // a SansAmp in his chain (the GT2 today), Gallien-Krueger amps, clean
        // and dirty split. Mid-forward, picked, low grit under the clean.
        make ("Forty Six & 2 (Tool)", E, 2, "Song,Drive", { { rigLevel, 10 },
            { compOn, 1 }, { compThreshold, -22 }, { compRatio, 4 },
            { driveOn, 1 }, { driveType, gotham }, { gothamLevel, 6 }, { gothamBlend, 5 }, { gothamDrive, 5.5f }, { gothamPresence, 6 },
            { gothamTreble, 6 },
            { eqOn, 1 }, { eq80, 1 }, { eq300, -1 }, { eq900, 3 }, { eq2k, 3 }, { eq5k, 1 }, { eqLevel, -1.5f } },
            "Justin Chancellor's picked, mid-forward Wal tone with low grit blended under the clean. GOTHAM stands in for the SansAmp "
            "in his chain (a cousin of the GT2 he uses today; his 1996 drive isn't documented).\n"
            "Amp: a Gallien-Krueger (2001RB style) capture.\nCab: a Mesa 8x10 IR.\n"
            "Play: with a hard pick, drop D, about 80 BPM."),

        // Korn, "Blind": Fieldy's percussive clank, an Ibanez 5-string tuned
        // a step down, Mesa M-2000s, mids "all the way off" and treble up,
        // no drive.
        make ("Blind (Korn)", E, 3, "Song,Clean,Metal", { { rigLevel, -1 },
            { compOn, 1 }, { compThreshold, -26 }, { compRatio, 6 }, { compLevel, 4 },
            { gateThreshold, -60 },
            { eqOn, 1 }, { eq33, 1 }, { eq80, 3 }, { eq150, -6 }, { eq300, -8 }, { eq500, -6 }, { eq900, -2 }, { eq2k, 3 }, { eq5k, 6 },
            { eq9k, 4 }, { eqLevel, 2 }, { eqUltraHi, 1 } },
            "Fieldy's clanky, percussive slap: bright attack, a hollow middle (he's said he turns the mids all the way off) and boom "
            "underneath, no distortion.\n"
            "Amp: a clean Mesa (M-2000 style) capture.\nCab: a 4x10 IR with a tweeter.\n"
            "Play: slapped hard, 5-string tuned a whole step down (A D G C F)."),

        // RATM, "Bulls on Parade": Tim Commerford, Fender Jazz with three
        // fingers, two Ampeg SVTs, one always clean, the other switched in
        // with a Marshall Guv'nor for dirt. Warm, mids kept in.
        make ("Bulls on Parade (RATM)", E, 4, "Song,Drive,Rock", {
            { compOn, 1 }, { compThreshold, -18 }, { compRatio, 3 },
            { driveOn, 1 }, { driveType, obsidian }, { driveGain, 5 }, { dualMod, 2 }, { dualBlend, 5 }, { dualLevel, 4.5f }, { dualGrowl, 1 },
            { eqOn, 1 }, { eq80, 3 }, { eq150, 1 }, { eq500, 1 }, { eq900, 1 }, { eqLevel, -1.5f } },
            "Tim Commerford's warm, round SVT tone that's always there, with crunchy dirt on top: on record, one amp stayed clean and "
            "the other added a Guv'nor's distortion. The OBSIDIAN's blend does that split.\n"
            "Amp: an Ampeg SVT capture.\nCab: an SVT 8x10 IR.\n"
            "Play: fingers, E-flat standard (or TUNING -1), 83 BPM."),

        // Live, "Lakini's Juice": Patrick Dahlheimer's '58 P-bass, "a little
        // MXR overdrive" in line (his words, quoted). Thick, dark growl.
        make ("Lakini's Juice (Live)", E, 5, "Song,Drive,Rock", { { rigLevel, 1 },
            { driveOn, 1 }, { driveType, gotham }, { gothamLevel, 5 }, { gothamBlend, 7 }, { gothamDrive, 7 }, { gothamPresence, 4 },
            { gothamBass, 6 },
            { octOn, 1 }, { octSubOne, 0.15f }, { octDry, 1 },
            { eqOn, 1 }, { eq80, 3 }, { eq500, 2 }, { eq5k, -3 }, { eq9k, -4 }, { eqLevel, -1.5f } },
            "Patrick Dahlheimer's thick, dark overdriven P-bass with a strong low-mid push (he's quoted as having \"a little MXR "
            "overdrive\" in line). The faint octave down is our addition, for weight.\n"
            "Amp: a Trace Elliot or mildly driven Ampeg SVT capture.\nCab: a 4x10 or 8x10 IR.\n"
            "Play: tuned a half step down with the low string dropped (Db Ab Db Gb), about 91 BPM."),

        // Karnivool, "Simple Boy": Jon Stockman co-designed the Alpha·Omega
        // to recreate his Sound Awake tone, so the OBSIDIAN is the direct
        // match. Settings: our starting point (none published).
        make ("Simple Boy (Karnivool)", E, 6, "Song,Drive,Prog", { { rigLevel, 5.5f },
            { compOn, 1 }, { compThreshold, -20 }, { compRatio, 4 },
            { driveOn, 1 }, { driveType, obsidian }, { driveGain, 6 }, { dualMod, 3 }, { dualBlend, 4 }, { dualLevel, 5 }, { dualMid, 6 },
            { dualTreble, 4.5f }, { dualBite, 1 } },
            "Jon Stockman's thick clean low end with a mid-forward grind on top. He co-designed the Alpha·Omega to recreate his Sound "
            "Awake tone, so the OBSIDIAN is the direct match; no settings have been published, so these are our starting point.\n"
            "Amp: a clean Ampeg SVT capture.\nCab: an SVT 8x10 IR.\n"
            "Play: 6-string, roughly B F# B E G C (as fans report)."),

        // Metallica, "Until It Sleeps": Jason Newsted's fretless 5-string Zon
        // (sold with the studio settings still taped on), Ampeg and Mesa
        // cabs, a tube DI. Warm fretless verses, driven layer for the heavy parts.
        make ("Until It Sleeps (Metallica)", E, 7, "Song,Clean,Drive", { { rigLevel, 10 },
            { compOn, 1 }, { compThreshold, -22 }, { compRatio, 3 }, { compLevel, 2 },
            { driveOn, 1 }, { driveType, gotham }, { gothamLevel, 6 }, { gothamBlend, 3 }, { gothamDrive, 3 }, { gothamPresence, 4 },
            { eqOn, 1 }, { eq80, 2 }, { eq150, 2 }, { eq900, 2 }, { eq2k, 2 }, { eq9k, -2 }, { eqLevel, -1.5f } },
            "Jason Newsted's warm, round fretless \"mwah\": mostly clean, with a little GOTHAM grit standing in for the tube DI. "
            "For the heavy choruses, raise BLEND and DRIVE.\n"
            "Amp: an Ampeg SVT capture (or a Mesa bass head).\nCab: an SVT 8x10 IR (or a 4x10).\n"
            "Play: fretless if you have one, E-flat standard (or TUNING -1), about 90 BPM."),

        // Periphery, "Absolomb" (Nolly on bass): the OBSIDIAN at the
        // settings seen on the pedal in Nolly's Darkglass demo, which plays
        // the song (the owner confirmed GROWL off and BITE on), after
        // Nolly's own 4:1 compression.
        make ("Absolomb (Periphery)", E, 8, "Song,Bass,Distortion,Metal", { { rigLevel, 7.5f },
            { compOn, 1 }, { compThreshold, -20 }, { compRatio, 4 },
            { driveOn, 1 }, { driveType, obsidian }, { driveGain, 5.8f }, { dualBlend, 5.5f }, { dualLevel, 4 }, { dualMod, 7 },
            { dualBass, 5 }, { dualMid, 5.8f }, { dualTreble, 7.5f }, { dualGrowl, 0 }, { dualBite, 1 } },
            "Nolly's Absolomb tone: the OBSIDIAN at the settings on the pedal in his Darkglass demo, after a gentle 4:1 compressor "
            "(his own chain compresses before the drive).\n"
            "Amp: a clean Ampeg SVT capture, or none.\nCab: an SVT 8x10 IR (MD421 at the cap edge, as he used).\n"
            "Play: with a pick, on an extended-range bass (Nolly's Dingwall)."),

        // RHCP, "Higher Ground": Flea's Spector with EMGs, a GK 800RB over
        // Mesa cabs. Bright, scooped, snappy slap.
        make ("Higher Ground (RHCP)", E, 9, "Song,Funk,Clean", {
            { compOn, 1 }, { compThreshold, -26 }, { compRatio, 6 }, { compLevel, 4 },
            { eqOn, 1 }, { eq33, 1 }, { eq80, 3 }, { eq300, -3 }, { eq500, -3 }, { eq900, -1 }, { eq2k, 2 }, { eq5k, 3 }, { eq9k, 2 },
            { eqLevel, -0.5f } },
            "Flea's bright, scooped, snappy slap.\n"
            "Amp: a Gallien-Krueger 800RB style capture.\nCab: a 2x15 or 4x10 IR.\n"
            "Play: slapped, E standard, about 140 BPM."),

        // Marvin Gaye, "What's Going On": James Jamerson's Precision with
        // heavy flatwounds and a foam mute. Warm, round, no treble.
        make ("What's Going On (Jamerson)", E, 10, "Song,Clean,Vintage", { { rigLevel, -2 },
            { compOn, 1 }, { compThreshold, -18 }, { compRatio, 2.5f },
            { eqOn, 1 }, { eq80, 3 }, { eq150, 3 }, { eq300, 1 }, { eq900, -2 }, { eq2k, -5 }, { eq5k, -9 }, { eq9k, -11 }, { eqLevel, 0.5f } },
            "James Jamerson's warm, round, thumpy Motown tone with no treble. His flatwound strings and foam mute can't be "
            "modelled: a muted thumb or palm gets you closer.\n"
            "Amp: an Ampeg B-15 capture, or none (it may have gone straight to the desk).\nCab: a 1x15 IR, or none.\n"
            "Play: one finger, E standard, about 102 BPM."),

        // Joy Division, "Love Will Tear Us Apart": Peter Hook's trebly,
        // picked lead bass with chorus (an EHX Clone Theory, reported).
        make ("Love Will Tear Us Apart (Joy Division)", E, 11, "Song,Modulation,Clean", { { rigLevel, 11 },
            { compOn, 1 }, { compThreshold, -22 }, { compRatio, 3 },
            { driveOn, 1 }, { driveType, gotham }, { gothamLevel, 6 }, { gothamBlend, 3 }, { gothamDrive, 3 }, { gothamPresence, 6 },
            { gothamTreble, 7 }, { driveLevel, 8 },
            { eqOn, 1 }, { eq33, -3 }, { eq80, -1 }, { eq500, 2 }, { eq900, 3 }, { eq2k, 3 }, { eq5k, 2 }, { eqLevel, -1.5f },
            { modOn, 1 }, { modType, chorus }, { modRate, 0.8f }, { modDepth, 0.5f }, { modMix, 0.45f } },
            "Peter Hook's trebly, picked, melodic lead bass with chorus, played high on the neck.\n"
            "Amp: a Hiwatt capture at the edge of breakup.\nCab: a 4x12 or 4x15 IR.\n"
            "Play: with a pick, high on the neck, E standard, about 150 BPM."),
    };
}
