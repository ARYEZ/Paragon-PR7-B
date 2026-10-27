# Changelog

Each version's notes appear on its GitHub Release page.

## 0.13.0

- **FACEPLATE stage 2: every block now has its own faceplate**, the same as PR7-G's, fitted to PR7-B's controls, in every finish.
  - **PITCH:** the octaver's four voices and FREEZE's level as faders; TUNING on a screen; OCTAVER and FREEZE on keys.
  - **COMP / GATE:**
    - **Classic:** a gain-reduction needle that moves as you play, plus THRESHOLD, RATIO, LEVEL and GATE.
    - **Daybreak:** its knobs, INTENSITY on a key, the GATE, and gain reduction in LED bars.
  - **FILTER / PITCH:**
    - **Auto-Wah and Talk Box:** the filter's peak, sharper with RESONANCE.
    - **Pitch Shifter:** the interval. **Quasar:** the program.
  - **CABINET IR:** the IR's name and length, and its **real frequency response** from 40 Hz to 20 kHz, worked out from the file when it loads.
  - **GRAPHIC EQ:** the EQ's real curve on PR7-B's bass bands (33 Hz to 9 kHz), **ULTRA LO and ULTRA HI included**, with those two on keys. The switches work with the faders off, as on the amp, and the curve shows just them then.
  - **GLITCH:** a 16-step pattern by DENSITY, with a light stepping at LENGTH.
  - **MOD:** named for its TYPE (AEON / FLANGER, NEBULA / PHASE VIBRATO...), its wobble at RATE and DEPTH, RATE in digits or as a note, and its knobs (AEON's MANUAL, DEPTH and RES).
  - **DELAY:** named for its TYPE (TAPE ECHO...), the repeats fading by FEEDBACK, and TIME in digits.
  - **REVERB:** named for its TYPE (HALL...), its pre-delay and tail (Spring ripples, Reverse swells), and DECAY in seconds.
  - **LO-FI:** a wave showing AMOUNT and NOISE.
  - **What's a drawing rather than a measurement:** the reverb's shape, the filter peak, the glitch pattern and the lo-fi wave. The cab and EQ curves and the compressor's needle are real. Moving lights keep time with the settings, not with the audio.

## 0.12.0

- **The FACEPLATE, in place of RIG BROWSER and SELECTED RIG** (the two panels at the bottom left), as on PR7-V and PR7-G. It shows the block you've selected in the signal chain as a hardware unit, built from the PR7's own parts (its chassis, LCD screens, knobs, keys and lettering) in your current finish. This is stage 1 of 2:
  - **OD / DIST:** the pedal's name on a badge and what it is, with its controls laid out as on its panel:
    - **OBSIDIAN:** BLEND, LEVEL, DRIVE and MOD (ALPHA to OMEGA) on top; BASS, MID and TREBLE below, with GROWL and BITE on keys that light when on.
    - **GOTHAM:** LEVEL, BLEND, TREBLE, BASS, DRIVE and PRESENCE.
    - **SPLIT** on a key that lights when on, with its frequency and the clean LOWS level under it.
    - An **INPUT** meter along the bottom shows your bass's level going in.
  - **NAMP:** the loaded amp model's name and details on an LCD screen (or LOADING... / NO MODEL LOADED), with RIG LEVEL, PRESENCE and NORMALISE, and the same INPUT meter.
  - **Every other block** shows a plain unit with its name and on light until stage 2.
  - **Nothing is lost:** saving is still on the header's SAVE button, the pedal's SAVE / SAVE AS and the PRESETS page, and the rig browser is the PRESETS page.
  - The faceplate is a display: you still turn the knobs in the middle panel.
- **Our own names, as in PR7-G 0.49.0.** Only the names changed: everything sounds the same, and rigs and projects load as before.
  - Whammy-Wah is now **Quasar**, Uni-Vibe is now **Nebula** and Dimension Chorus is now **Prism**.
  - The expression pedal's PITCH / WHAMMY is now **PITCH BEND**, and WHAMMY-WAH is now **QUASAR**.
  - Quasar's programs 02, 03, 07 and 10 are now **Classic Wah**, **Classic Wah 2**, **Wah Auto Sens** and **Wah Auto Attack**.

## 0.11.1

- **The signal chain in the PR7 family's colours:** each block along the bottom strip now glows in the red, hot pink and orange gradient of the new PR7-E's chain, by its place in the chain: red at PITCH, pink in the middle, orange at LO-FI. The same change as in PR7-G 0.43.1 and 0.43.2.
- **Each edit panel matches its block:** a panel's header icon is in its block's chain colour, and the graphic EQ's faders, curve and icon are in the EQ block's.
- Before, each block had its own unrelated colour (blue for delay, teal for reverb and so on). Only the colours change; the sound and the controls are the same.

## 0.11.0

- **AEON replaces the generic flanger:** a flanger modelled on the Boss BF-2, built stage by stage from its schematic with the real component values, like the OBSIDIAN and GOTHAM. It's first in a plan to model specific pedals for each modulation effect.
  - **The pedal's four knobs:** MANUAL (the delay time), DEPTH, RATE and RES (resonance). MIX is greyed out, because the pedal always mixes dry and wet 50/50, which makes the notches deep.
  - **The delay:** a bucket-brigade chip (BBD) model, 13 ms down to 1 ms across MANUAL, the pedal's specified range. The old flanger only reached 0.5 to 5.5 ms, so AEON sweeps much lower and deeper. At full DEPTH, MANUAL does nothing, as the pedal's manual says.
  - **The sweep:** a triangle LFO, from one cycle every 16 s to 10 per second. At fast RATE settings the sweep gets narrower, as on the pedal, where a capacitor smooths it.
  - **RES:** rings the peaks hardest in the treble, the BF-2's metallic "zing", without howling.
  - **For bass:** the pedal's wet path lets little through below about 40 Hz, so the low end stays solid while the mids and highs sweep.
  - **Your sound:** rigs and projects that used the old flanger now get AEON, with MANUAL and RES at 5. Expect it to be louder (up to about 6 dB on the comb's peaks), because dry and wet add at full level.
  - **Two details are estimates,** because the schematic can't show them: how much signal the BBD chip and its output stage lose, and where the factory set the RES trimmer. Both are set so full RES rings hard without howling.
- **Flanged Bass (C5) and Forty Six & 2 (E2)** now use AEON (Justin Chancellor has a BF-2 on his board), rebalanced to match the other rigs' loudness.

## 0.10.1

- **No more screws on the face plate.** The four screw heads in its corners are gone, for a cleaner front.
- **The MIN light beside the expression pedal is gone too.** It lit when the pedal was fully back on its heel; the dock's EXPRESSION ASSIGN screen already shows where the pedal sits.

## 0.10.0

- **SPLIT in OD / DIST (both models):** keeps everything below 275 Hz clean, the multiband trick Nolly (Periphery) uses on bass.
  - A crossover sends the lows around the pedal, untouched, and only the band above through the OBSIDIAN or GOTHAM, then adds them back together. The low end stays tight and solid however hard the drive is pushed.
  - Modelled on his FabFilter Saturn setup (split at 275 Hz, the band below bypassed). The crossover is a Linkwitz-Riley, whose two bands add back up flat.
  - **SPLIT HZ** (80 to 600 Hz, 275 by default) moves the split. **LOWS** (±12 dB) sets the clean lows' level against the pedal's. OUTPUT trims both.
  - Off by default, so existing rigs and projects sound the same. The footswitch shows "+ split" when it's on.
- **New rig B7, Nolly Split:** the Absolomb OBSIDIAN pushed harder, with SPLIT on. The Absolomb rig itself is unchanged.

## 0.9.0

- **14 genre rigs in bank D:** Metalcore, Nu Metal, Thrash Metal, Pop Punk, Djent, Death Metal, Stoner Rock, Hardcore Punk, Grunge, Classic Rock, Alt / Indie Rock, Prog Metal, Post-Rock and 80s / Synthwave.
  - Each one is that style's typical bass sound, as it's commonly played, recorded and mixed, built from PR7-B's blocks: our interpretation of the genre, not any one player's rig.
  - Like the song rigs, each one's notes say what it's for, which amp capture and cab IR to load, and how to play it (pick or fingers, tuning).
  - They're loudness-matched to the other rigs, within about 1 dB.

## 0.8.1

- **Forty Six & 2 (Tool) is now the intro riff:** a clean, picked, mid-forward bass with a slow flanger sweeping through it. The GOTHAM grit for the heavy part is set up but off: press **footswitch 2** when the band comes in.

## 0.8.0

- **New bass factory rigs.** All of PR7-G's guitar rigs are gone, replaced by 30 rigs made for bass:
  - **A, clean and basics:** Clean DI, Studio Bass, Fingerstyle Round, Slap Funk, Motown Thump, Fretless Mwah.
  - **B, drive:** Gotham Grit, Gotham Cranked, Obsidian Tight, Obsidian Raw, Modern Metal Clank, Doom Fuzz.
  - **C, effects:** Envelope Funk, Synth Octave, Sub Octave, Chorus Bass, Flanged Bass, Dub Delay, Tremolo Pulse.
  - **E, songs:** Hysteria (Muse), Forty Six & 2 (Tool), Blind (Korn), Bulls on Parade (RATM), Lakini's Juice (Live), Simple Boy (Karnivool), Until It Sleeps (Metallica), Absolomb (Periphery), Higher Ground (RHCP), What's Going On (Jamerson) and Love Will Tear Us Apart (Joy Division).
- **The song rigs follow what's documented about each recording:** the bassist's drive, the EQ character and the effects, built from PR7-B's blocks.
  - **Some sound right out of the box.** Bulls on Parade, for one, recreates Tim Commerford's clean amp plus distorted amp with the OBSIDIAN's blend. Simple Boy uses the OBSIDIAN, the pedal Jon Stockman co-designed.
  - **They're our interpretation**, inspired by the records, not copies. Where nothing was published (most knob settings), the settings are our starting point, and each rig's notes say so.
- **Rigs have notes.** The PRESETS page's SELECTED RIG panel shows what each rig is for. For the song rigs, it also shows **which amp capture and cabinet IR to load** (rigs can't ship them), the tuning and how to play it. Saving a copy keeps the notes.
- **Every factory rig is loudness-matched**, within about 1 dB of Clean DI on a test bass line, so switching rigs doesn't jump in volume.
- The PRESETS page's summary line now also names the octaver, filter and EQ.

## 0.7.1

- **GOTHAM's DRIVE is more aggressive:** 25% more gain (in dB) at every setting, so fully cranked it's about 50 dB instead of 40, closer to the real pedal flat out. The knob keeps its feel, with most of the drive still in the top half. It's still well short of the OBSIDIAN's gain. This is our voicing by ear; the circuit's own values are unchanged.

## 0.7.0

- **The small EXPRESSION screen beside the pedal is gone.** Assign the expression pedal on the EXPRESSION ASSIGN screen in the dock, which is bigger and scrolls.
- **AMP, CABINET and RIG share the row equally**, each window wider, now that the small screen's space at the row's end is free.

## 0.6.0

The chassis finishes from the guitar version, Paragon PR7-G (its 0.29.0 to 0.32.0), brought over to PR7-B.

- **Three finishes**, in **SETTINGS → FINISH**:
  - **Pearl White:** the original, unchanged.
  - **Pearl White + Neon:** the white unit with neon keys and knobs that run pink through red to orange. That covers the footswitches, the AMP / CABINET / RIG keys, the small keys, the RIG LEVEL and PRESENCE knobs, and the expression pedal's grip plates.
  - **Pearl Black:** a deep black pearl chassis, pedal bay and treadle, with the same neon keys, knobs and grips, and white lettering.
  - Over the neon keys, the six button names (UNDO, REDO, BYPASS, TUNER, MUTE, TAP) are printed so they stay readable.
  - The choice is saved on your computer, not in the project, so every Paragon PR7-B you open uses it. It's kept apart from PR7-G's, so each plugin remembers its own finish.
- **Opens bigger:** the window opens at **1824 × 1026** (it was 1280 × 720), or the largest size that fits on the screen *above the taskbar*, leaving room for the DAW's title bar and plugin toolbar. On a 1080p screen that's about 1706 × 959. You can still drag it to any size.
- **EXPRESSION ASSIGN is a screen:** the dock's list of small buttons is now the same orange LCD as the EXPRESSION screen beside the pedal, enlarged, with bigger lettering. **▲ / ▼** beside it scroll the list. Click a line to assign the pedal, click it again to unassign, or use the mouse wheel. MIN, the range slider and MAX share one row underneath.
- On the neon finishes, a tilted expression pedal no longer looks washed-out and grey.

## 0.5.0

- **GOTHAM, a new bass preamp modelled on the Tech 21 SansAmp Bass Driver DI** (the original, Version 1). OD / DISTORTION now has a **MODEL** menu: OBSIDIAN or GOTHAM.
  - **Built from the circuit**, like the OBSIDIAN: every stage follows the schematic of madbeanpedals' SludgeHammer, a clone of the pedal, with its real component values.
  - **What it does, as the pedal does:** it imitates an amp and cabinet. A filter scoops about 800 Hz before the gain (the classic SansAmp mid scoop). The drive stages clip like op-amps on 9 V. The filters after them keep the lows big and roll the top off like a speaker.
  - **The knobs, as on the pedal:**
    - **LEVEL:** the volume.
    - **BLEND:** the clean bass against the drive.
    - **TREBLE and BASS:** about ±12 dB at 5 kHz and 80 Hz.
    - **DRIVE:** up to 40 dB of gain, most of it in the top half of the knob, as on the pedal.
    - **PRESENCE:** lifts the top going into the distortion, up to +27 dB around 4–5 kHz.
    - **OUTPUT:** our trim into the amp, shared with OBSIDIAN.
  - **One place we chose:** the SludgeHammer puts LEVEL straight into the EQ, which would make LEVEL tilt the tone and halve the EQ's boost. An analysis of the original pedal (Bass21) puts LEVEL before the last gain stage instead, which matches Tech 21's ±12 dB EQ spec. We followed the original.
- **DUAL ENGINE is now called OBSIDIAN.** Same sound and settings; rigs and projects saved with 0.4.0 load as before.
- **The OBSIDIAN's panel uses two rows**, now that the MODEL menu is there too, so its twelve controls keep good-sized knobs and readable values.

## 0.4.0

- **DUAL ENGINE, a bass distortion modelled on the Darkglass Alpha·Omega**, the pedal Nolly demoed for Darkglass with "Absolomb". It replaces the guitar drives (Classic, Screamer, Muff Fuzz and Face Fuzz), so OD / DISTORTION is now the bass distortion.
  - **Built from the circuit itself.** Every stage follows a trace of the pedal's circuit (Aion FX's) with its real component values, so the filters and gains are the pedal's, not guesses. The distortion is what the circuit does: op-amps pushed past their 9 V supply, in three places.
  - **The knobs, as on the pedal:**
    - **BLEND:** the clean bass (full range, untouched) against the distortion.
    - **LEVEL:** the distortion's volume.
    - **DRIVE:** its gain, up to about 40 dB on top of a fixed 20 dB.
    - **MOD:** 0 is ALPHA (tight and refined: a deep scoop at 520 Hz and the top rolled off), 10 is OMEGA (raw: a gentler scoop at 780 Hz, more top). In between is a mix of both.
    - **GROWL:** much more bass into the distortion (about 7 dB at 80 Hz), for a fatter, heavier sound.
    - **BITE:** lifts about 3 kHz by 9 dB before the distortion, for clank and definition.
    - **BASS, MID, TREBLE:** after the blend, so they shape the clean bass too.
    - **OUTPUT:** our addition, a trim into the amp.
  - **What the circuit showed** that the specs don't say:
    - The MID knob centres near 1 kHz, not the advertised 500 Hz.
    - There's a fixed, narrow dip at 1.85 kHz after the blend, on the clean bass too.
    - BITE really is a boost, as Darkglass says.
- **New song rig: E9 Absolomb (Periphery)**, the DUAL ENGINE at the settings on the pedal in Nolly's demo, after a gentle 4:1 compressor. GROWL and BITE weren't visible there, so those two are our guess. Load an SVT 8x10 cabinet IR with it.
- The guitar factory rigs that used a drive now use the DUAL ENGINE until they're replaced with bass rigs.

## 0.3.0

- **Bass EQ presets.** The guitar presets are replaced by sixteen made for bass, most of them for modern metal. Open **PRESETS** in the GRAPHIC EQ:
  - **Metal:** Modern Metal Clank, Djent Growl, Tight Low End, Drop-Tuned Weight, Grind Through the Guitars, Thrash Pick Attack, Classic Metal Scoop, Doom Fuzz, Fizz Tamer and Mud Cut.
  - **Other styles:** Clean DI Polish, Slap Funk, Motown Thump, Fingerstyle Growl, Fretless Mwah and Radio.
  - The metal ones follow the usual recipe for heavy bass: keep the deepest lows in check so the low end stays tight, cut the muddy low mids (around 300 Hz), and lift the upper-mid "clank" and grind (900 Hz to 5K) so the bass is heard under down-tuned guitars.
  - Presets can now switch **ULTRA LO** and **ULTRA HI** too: Classic Metal Scoop uses both, for the amp's scooped '80s and '90s metal tone, and Slap Funk uses ULTRA HI. Choosing a preset that doesn't use them switches them off.
  - Each one's LEVEL is set so switching presets changes the tone, not the volume. Hover over the menu to read what a preset is for.

## 0.2.0

- **A bass amp's graphic EQ.** The nine bands now sit where a bass needs them: **33, 80, 150, 300, 500, 900 Hz, 2K, 5K and 9K**, the layout of the Ampeg SVT-3 PRO's graphic EQ (modelled on it, not a copy of its circuit). 33 Hz reaches the sub lows of a low B or drop D, and the bands are closer together through the low mids, where a bass's tone and punch live. Each still cuts or boosts by up to 12 dB.
- **ULTRA LO and ULTRA HI switches**, in the GRAPHIC EQ header, as on that amp:
  - **ULTRA LO:** a deep scoop in the low mids (-10 dB at 500 Hz) with a touch more deep low end (+2 dB at 40 Hz), for a big, hollow, modern tone.
  - **ULTRA HI:** a big lift in the highs (+9 dB at 8 kHz), for slap, pick attack and string zing.
  - Like the amp's, they work whether the EQ's faders are switched on or not. The EQ block in the signal chain lights while any of them is on, and the curve shows what they do.
- **The EQ presets** were carried across to the new bands, following each curve so it keeps its shape, and re-levelled over a bass's range.

## 0.1.0

- **The bass processor, based on Paragon PR7-G 0.28.0.** Everything PR7-G 0.28.0 has, under its own name, ready to be voiced for bass.
- **Its own plugin.** Paragon PR7-B has its own name, ID and plugin code, so your DAW sees it as a separate plugin: it installs alongside Paragon PR7-G instead of replacing it, and projects using either keep the right one.
- **Its own rigs.** Rigs are saved in their own Paragon PR7-B folder. Guitar rigs from PR7-G aren't copied over.
- **The PR7-B logo** on the header and the unit's badge, in the same slanted, speed-cut lettering.
