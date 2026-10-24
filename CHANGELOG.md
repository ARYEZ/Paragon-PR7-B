# Changelog

Each version's notes appear on its GitHub Release page.

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
