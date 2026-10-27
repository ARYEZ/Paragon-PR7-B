#include "Faceplate.h"

#include "Finish.h"
#include "LedDisplay.h"
#include "Parameters.h"
#include "ParagonLookAndFeel.h"
#include "PedalBody.h"
#include "dsp/effects/Drive.h"

// Every part here is one of the PR7's own: the unit's pearl chassis
// (PedalBody::paintUnit), its LCD screens (LedDisplay::paintScreen, the LED
// orange), big knobs, small keys with their light stripes, neon LED bars,
// and lettering stamped in ink or printed in red, all following the
// finish. (The helpers are the PR7-V's and PR7-G's.)
namespace
{
    const PedalBody::Shape unitShape { 16.0f, 10.0f, 5.0f, 0.3f };
    constexpr float glowRoom = 7.0f;        // round the unit, for its light
    constexpr float screenSize = 0.45f;     // the small screens' frame and bezel, against the main display's
    const auto orange = LedDisplay::ledOrange;

    juce::FontOptions font (float height, int style = juce::Font::bold) { return juce::FontOptions (height, style); }

    void stamped (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area, float height,
                  juce::Justification justification = juce::Justification::centred)
    {
        g.setFont (font (height));
        ParagonLookAndFeel::drawStampedText (g, text, area, justification, Finish::ink());
    }

    // The model's name, as PARAGON is on the badge, with a red printed line under it.
    void nameplate (juce::Graphics& g, juce::Rectangle<float> area, const juce::String& name, const juce::String& line)
    {
        g.setFont (font (area.getHeight() * 0.62f, juce::Font::bold | juce::Font::italic));
        ParagonLookAndFeel::drawStampedText (g, name, area.removeFromTop (area.getHeight() * 0.66f), juce::Justification::bottomLeft, Finish::ink());
        g.setColour (Finish::printedRed());
        g.setFont (font (area.getHeight() * 0.72f));
        g.drawText (line, area, juce::Justification::centredLeft, false);
    }

    // A PR7 screen: returns its glass, to draw on before `finish`.
    juce::Rectangle<float> screen (juce::Graphics& g, juce::Rectangle<float> area) { return LedDisplay::paintScreen (g, area, screenSize); }
    void finish (juce::Graphics& g, juce::Rectangle<float> glass) { LedDisplay::paintGlassOverlay (g, glass, screenSize); }

    // A meter on the glass, the tuner's way: a row of LED bars, lit up to `amount` (0 to 1).
    void bars (juce::Graphics& g, juce::Rectangle<float> area, float amount, int count)
    {
        const auto width = area.getWidth() / (float) count;
        const auto lit = juce::roundToInt (juce::jlimit (0.0f, 1.0f, amount) * (float) count);
        for (int i = 0; i < count; ++i)
        {
            const auto bar = juce::Rectangle<float> (area.getX() + width * (float) i, area.getY(), width, area.getHeight()).reduced (width * 0.22f, 0.0f);
            if (i < lit)
            {
                g.setColour (orange.withAlpha (0.3f));
                g.fillRoundedRectangle (bar.expanded (2.0f), 2.0f);
                g.setColour (orange);
            }
            else
            {
                g.setColour (orange.withAlpha (0.1f));
            }
            g.fillRoundedRectangle (bar, 1.5f);
        }
    }

    // The angle a knob points at for `position` (0 to 1), as the unit's knobs turn.
    float knobAngle (float position) { return juce::MathConstants<float>::pi * (1.2f + 1.6f * juce::jlimit (0.0f, 1.0f, position)); }

    // One of the unit's big knobs with its name stamped under it.
    // (In a cell taller than the knob, the knob and its name stay together, centred.)
    void bigKnob (juce::Graphics& g, juce::Rectangle<float> cell, const juce::String& name, float position)
    {
        const auto size = juce::jmin (cell.getWidth(), cell.getHeight() + 4.0f);
        cell = cell.withSizeKeepingCentre (cell.getWidth(), juce::jmin (cell.getHeight(), size + 8.0f));
        stamped (g, name, cell.removeFromBottom (12.0f), 8.0f);
        ParagonLookAndFeel::drawPedalKnob (g, cell.withSizeKeepingCentre (size, size).translated (0.0f, -2.0f), knobAngle (position));
    }

    // One of the unit's small keys with its name stamped under it, its stripe lit while `on`.
    void smallKey (juce::Graphics& g, juce::Rectangle<float> cell, const juce::String& name, bool on)
    {
        stamped (g, name, cell.removeFromBottom (12.0f), 8.0f);
        const auto key = cell.withSizeKeepingCentre (juce::jmin (52.0f, cell.getWidth()), juce::jmin (26.0f, cell.getHeight()));
        ParagonLookAndFeel::drawSmallKey (g, key, false, false);
        ParagonLookAndFeel::drawKeyStripe (g, key.reduced (4.0f, 2.0f), on, false);
    }
}

//==============================================================================
Faceplate::Faceplate (ParagonX1AudioProcessor& processorToShow)
    : processor (processorToShow), parameters (processorToShow.getParameters())
{
    setInterceptsMouseClicks (false, false);
    Finish::changes().addChangeListener (this);
    startTimerHz (30);
}

Faceplate::~Faceplate()
{
    Finish::changes().removeChangeListener (this);
}

void Faceplate::resized()
{
    // Twice the size, so the grain stays fine on high-resolution screens (as on the unit).
    satin = Finish::makeSatinGrain (juce::jmax (1, getWidth() * 2), juce::jmax (1, getHeight() * 2), 4321);
}

void Faceplate::changeListenerCallback (juce::ChangeBroadcaster*)
{
    resized();   // the grain differs on black
    repaint();
}

void Faceplate::setBlock (Unit newUnit, const juce::String& newTitle, Icons::Type newIcon, juce::Colour newColour,
                          const juce::String& newOnIds, const juce::String& newTypeId)
{
    unit = newUnit;
    title = newTitle;
    icon = newIcon;
    colour = newColour;
    onIds = juce::StringArray::fromTokens (newOnIds, ",", {});
    typeId = newTypeId;
    repaint();
}

Faceplate::Look Faceplate::getLook() const
{
    switch (unit)
    {
        case Unit::drive: return Look::drive;
        case Unit::amp:   return Look::amp;
        case Unit::other: break;
    }
    return Look::generic;
}

float Faceplate::raw (const juce::String& id) const { return parameters.getRawParameterValue (id)->load(); }

float Faceplate::normalised (const juce::String& id) const { return parameters.getParameter (id)->getValue(); }

juce::String Faceplate::text (const juce::String& id) const { return parameters.getParameter (id)->getCurrentValueAsText(); }

bool Faceplate::isOn() const
{
    for (const auto& id : onIds)
        if (raw (id) > 0.5f)
            return true;
    return false;
}

void Faceplate::timerCallback()
{
    if (! isShowing())
        return;
    updateMeters();
    repaint();
}

void Faceplate::updateMeters()
{
    // A meter's ballistics: up at once, falling back slowly.
    const auto peak = processor.getFaceplateMeter().takePeak();
    shownInput = juce::jmax (peak, shownInput * 0.88f);
}

//==============================================================================
void Faceplate::paint (juce::Graphics& g)
{
    // The unit: the PR7's chassis, smaller, in the current finish.
    const auto chassis = getLocalBounds().toFloat().reduced (glowRoom);
    const auto face = chassis.withTrimmedBottom (unitShape.frontEdge).reduced (9.0f);
    PedalBody::paintUnit (g, chassis, face, [this] (juce::Graphics& gg) { gg.drawImage (satin, getLocalBounds().toFloat()); }, unitShape);

    // Its name top left, its status LED top right (as over the footswitches).
    auto content = face.reduced (12.0f, 8.0f);
    auto header = content.removeFromTop (30.0f);
    ParagonLookAndFeel::drawNeonBar (g, header.removeFromRight (34.0f).withSizeKeepingCentre (30.0f, 6.0f).translated (0.0f, -6.0f), isOn());
    content.removeFromTop (6.0f);
    switch (getLook())
    {
        case Look::drive:   paintDrive (g, header, content); break;
        case Look::amp:     paintAmp (g, header, content); break;
        case Look::generic: paintGeneric (g, header, content); break;
    }
}

void Faceplate::paintGeneric (juce::Graphics& g, juce::Rectangle<float> header, juce::Rectangle<float> body)
{
    // The block's name and icon, and its TYPE on a screen.
    nameplate (g, header, title, "NEURAL MULTI-FX PROCESSOR");
    Icons::draw (g, icon, body.removeFromLeft (body.getHeight()).reduced (body.getHeight() * 0.14f), Finish::ink(), 2.2f, true);
    body.removeFromLeft (14.0f);
    const auto glass = screen (g, body.withSizeKeepingCentre (body.getWidth(), juce::jmin (body.getHeight(), 70.0f)));
    auto inside = glass.reduced (glass.getWidth() * 0.05f, glass.getHeight() * 0.16f);
    g.setColour (orange.withAlpha (0.7f));
    g.setFont (font (inside.getHeight() * 0.24f));
    g.drawText (typeId.isNotEmpty() ? "TYPE" : "STATUS", inside.removeFromTop (inside.getHeight() * 0.32f), juce::Justification::topLeft, false);
    g.setColour (orange.withAlpha (isOn() ? 1.0f : 0.45f));
    g.setFont (font (inside.getHeight() * 0.5f));
    g.drawFittedText ((typeId.isNotEmpty() ? text (typeId) : juce::String (isOn() ? "ON" : "OFF")).toUpperCase(),
                      inside.toNearestInt(), juce::Justification::centredLeft, 1, 0.8f);
    finish (g, glass);
}

void Faceplate::paintInputMeter (juce::Graphics& g, juce::Rectangle<float> area)
{
    // The bass's level going in, in LED bars (-48 to 0 dB).
    stamped (g, "INPUT", area.removeFromLeft (52.0f), 8.0f, juce::Justification::centredLeft);
    const auto glass = screen (g, area);
    const auto db = juce::Decibels::gainToDecibels (shownInput, -60.0f);
    bars (g, glass.reduced (glass.getWidth() * 0.03f, glass.getHeight() * 0.28f), (db + 48.0f) / 48.0f, 28);
    finish (g, glass);
}

void Faceplate::paintDrive (juce::Graphics& g, juce::Rectangle<float> header, juce::Rectangle<float> body)
{
    // A bass pedal: the MODEL's name in the badge's lettering and what it
    // is, its knobs and switches as on its panel, SPLIT, and the level going in.
    const auto gotham = juce::roundToInt (raw (ParamIDs::driveType)) == (int) effects::Drive::Model::gotham;
    nameplate (g, header, gotham ? "GOTHAM" : "OBSIDIAN", gotham ? "BASS PREAMP / DI" : "BASS PREAMP / DISTORTION");

    paintInputMeter (g, body.removeFromBottom (22.0f));
    body.removeFromBottom (6.0f);

    // SPLIT on the right: its key, lit while on, where it splits and how loud the clean lows are.
    {
        auto split = body.removeFromRight (92.0f);
        body.removeFromRight (8.0f);
        const auto on = raw (ParamIDs::driveSplit) > 0.5f;
        auto readings = split.removeFromBottom (26.0f);
        smallKey (g, split.withSizeKeepingCentre (split.getWidth(), juce::jmin (split.getHeight(), 52.0f)), "SPLIT", on);
        g.setColour (Finish::printedRed().withMultipliedAlpha (on ? 1.0f : 0.45f));
        g.setFont (font (8.0f));
        g.drawText (text (ParamIDs::driveSplitHz), readings.removeFromTop (13.0f), juce::Justification::centred, false);
        g.drawText ("LOWS " + text (ParamIDs::driveSplitLows), readings, juce::Justification::centred, false);
    }

    // A row of the unit's big knobs, `cells` to the row; returns what's left of it.
    const auto row = [&] (juce::Rectangle<float> area, const std::vector<std::pair<const char*, const char*>>& knobs, int cells)
    {
        const auto cellWidth = area.getWidth() / (float) cells;
        for (const auto& [id, name] : knobs)
            bigKnob (g, area.removeFromLeft (cellWidth).reduced (cellWidth > 80.0f ? cellWidth * 0.1f : 2.0f, 0.0f), name, normalised (id));
        return area;
    };

    if (gotham)
    {
        row (body, { { ParamIDs::gothamLevel, "LEVEL" }, { ParamIDs::gothamBlend, "BLEND" }, { ParamIDs::gothamTreble, "TREBLE" },
                     { ParamIDs::gothamBass, "BASS" }, { ParamIDs::gothamDrive, "DRIVE" }, { ParamIDs::gothamPresence, "PRESENCE" } }, 6);
        return;
    }

    // OBSIDIAN: BLEND, LEVEL, DRIVE and MOD (ALPHA to OMEGA) on top; BASS,
    // MID, TREBLE and the GROWL and BITE switches below.
    auto top = body.removeFromTop (body.getHeight() / 2.0f);
    const auto modCell = top.withTrimmedLeft (top.getWidth() * 0.75f);
    row (top, { { ParamIDs::dualBlend, "BLEND" }, { ParamIDs::dualLevel, "LEVEL" }, { ParamIDs::driveGain, "DRIVE" }, { ParamIDs::dualMod, "MOD" } }, 4);
    {
        // ALPHA and OMEGA printed either side of MOD's name: its two voicings.
        const auto names = modCell.withTop (modCell.getBottom() - 12.0f).reduced (2.0f, 0.0f);
        g.setColour (Finish::printedRed());
        g.setFont (font (7.0f));
        g.drawText ("ALPHA", names, juce::Justification::centredLeft, false);
        g.drawText ("OMEGA", names, juce::Justification::centredRight, false);
    }
    auto rest = row (body, { { ParamIDs::dualBass, "BASS" }, { ParamIDs::dualMid, "MID" }, { ParamIDs::dualTreble, "TREBLE" } }, 4);
    const auto half = rest.getWidth() / 2.0f;
    const auto keyHeight = juce::jmin (rest.getHeight(), 44.0f);
    smallKey (g, rest.removeFromLeft (half).withSizeKeepingCentre (half, keyHeight), "GROWL", raw (ParamIDs::dualGrowl) > 0.5f);
    smallKey (g, rest.withSizeKeepingCentre (half, keyHeight), "BITE", raw (ParamIDs::dualBite) > 0.5f);
}

void Faceplate::paintAmp (juce::Graphics& g, juce::Rectangle<float> header, juce::Rectangle<float> body)
{
    // An amp head: the neural model's name and details on a screen, its
    // knobs, NORMALISE on a key, and the level going in.
    nameplate (g, header, "NAMP", "NEURAL AMP MODEL");

    paintInputMeter (g, body.removeFromBottom (22.0f));
    body.removeFromBottom (6.0f);

    // RIG LEVEL, PRESENCE and NORMALISE on the right.
    auto controls = body.removeFromRight (body.getWidth() * 0.46f);
    body.removeFromRight (14.0f);
    auto keyCell = controls.removeFromRight (70.0f);
    stamped (g, "NORMALISE", keyCell.removeFromBottom (12.0f), 8.0f);
    const auto key = keyCell.withSizeKeepingCentre (52.0f, 26.0f);
    ParagonLookAndFeel::drawSmallKey (g, key, false, false);
    ParagonLookAndFeel::drawKeyStripe (g, key.reduced (4.0f, 2.0f), raw (ParamIDs::ampNormalise) > 0.5f, false);
    const auto half = controls.getWidth() / 2.0f;
    bigKnob (g, controls.removeFromLeft (half), "RIG LEVEL", normalised (ParamIDs::rigLevel));
    bigKnob (g, controls, "PRESENCE", normalised (ParamIDs::presence));

    // The model on a screen: its name, then its details (or what went wrong).
    const auto status = processor.getAmpStatus();
    const auto hasModel = status.file != juce::File();
    const auto glass = screen (g, body);
    auto inside = glass.reduced (glass.getWidth() * 0.06f, glass.getHeight() * 0.14f);
    g.setColour (orange.withAlpha (0.7f));
    g.setFont (font (9.0f));
    g.drawText ("MODEL", inside.removeFromTop (12.0f), juce::Justification::topLeft, false);
    g.setColour (orange.withAlpha (isOn() ? 1.0f : 0.45f));
    g.setFont (font (juce::jmin (22.0f, inside.getHeight() * 0.3f)));
    g.drawFittedText (status.loading ? juce::String ("LOADING...") : hasModel ? status.file.getFileNameWithoutExtension().toUpperCase() : juce::String ("NO MODEL LOADED"),
                      inside.removeFromTop (inside.getHeight() * 0.55f).toNearestInt(), juce::Justification::centredLeft, 2, 0.75f);
    g.setColour (orange.withAlpha (0.7f));
    g.setFont (font (10.0f));
    g.drawFittedText (status.error.isNotEmpty() ? status.error : status.detail, inside.toNearestInt(), juce::Justification::topLeft, 2, 0.8f);
    finish (g, glass);
}
