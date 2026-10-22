# Changelog

Each version's notes appear on its GitHub Release page.

## 0.12.0

**LO-FI: telephone, radio, megaphone, cassette and bitcrusher, for intros and breakdowns.**

- A new **LO-FI** block, last in the signal chain after the reverb, so the whole sound (echoes and reverb included) goes "through the phone", as it's done in the studio. Click LO-FI in the signal chain to open it. Five types:
  - **Telephone:** only 300 Hz to 3.4 kHz gets through, with steep edges and a little carbon-mic grit, in mono.
  - **AM Radio:** a small radio speaker: wider than a phone, a nasal midrange bump, gentle saturation, in mono.
  - **Megaphone:** a horn speaker: narrow, honky and driven hard, in mono.
  - **Cassette:** a worn tape: the pitch wobbles (slow wow, fast flutter and a random drift), the top end dulls, and the tape saturates and hisses.
  - **Bitcrusher:** fewer bits (down to 3) and a lower sample rate (down to 3 kHz), for early-sampler and video-game crunch.
- **AMOUNT** sets how strong it is: narrower, dirtier, wobblier or crunchier. **NOISE** adds line hiss, static or tape hiss. **MIX** and **LEVEL** blend it and match its volume.
- **LO-FI on the expression pedal** (new EXPRESSION ASSIGN target): fades the effect in, from the full band at the heel to full lo-fi at the toe. Start toe-down for a phone-sounding intro, then rock back to the heel as the band comes in. Assigning it switches LO-FI on.
- To switch it at a set point in a song, automate **Lo-Fi On** (or **Lo-Fi Mix**) in your DAW.
- Five new factory rigs: Phone Intro and Megaphone Verse (bank B), Old Radio (bank C), Cassette Dreams and 8-Bit Crunch (bank D).
- The EXPRESSION ASSIGN list has larger text, and the signal chain's gaps are narrower to fit the tenth block.

## 0.11.0

**Paragon X1 is now Paragon PR7, and the expression pedal can bend pitch.**

**Upgrading from Paragon X1:** the plugin file is now `Paragon PR7.vst3`. Close your DAW, delete the old `Paragon X1.vst3` from your VST3 folder, then copy in the new one. Keeping both would give your DAW two copies of the same plugin. Your projects still open: the plugin keeps the identity DAWs know it by, only the name has changed.

- **New name and logo:** PR7 is a heavy, forward-slanted mark with speed cuts, in the spirit of late-'90s floor processors. It appears in the header, on the unit's badge, and engraved on the expression pedal.
- **Your rigs come with you:** the rigs folder is now `Paragon\Paragon PR7\Rigs`. The first time the new version runs, it copies your rigs over from `Paragon\Paragon X1` and leaves the originals in place. New rigs save as `.pr7rig` files, and `.x1rig` files still load.
- **PITCH / WHAMMY** (new EXPRESSION ASSIGN target): the expression pedal bends the pitch shifter, from no shift at the heel to its INTERVAL at the toe. For example, set INTERVAL to +12 for an octave dive-up, or -24 for a two-octave drop. Assigning it switches the FILTER / PITCH block on as a pitch shifter, so it works straight away. MIN and MAX limit the sweep as usual. It sits fourth on the pedal's assign buttons and in the EXPRESSION ASSIGN panel.

## 0.10.0

**Tube Screamer drive, pitch shifter, auto-wah and doubler.**

- **Screamer drive:** the OD / DISTORTION block has a new MODEL setting, and its new default, **Screamer (TS)**, is modelled on the Ibanez TS808 Tube Screamer's circuit:
  - The op-amp only amplifies above about 720 Hz. Its diodes clip just that part, which is added back on top of the clean guitar, so the bass stays clean and tight while the mids are pushed (the famous mid "hump").
  - The diodes round the peaks off softly and symmetrically.
  - TONE works against a fixed 723 Hz treble cut, from dark to slightly bright.
  - DRIVE has the pedal's audio-taper pot.
  - It's the classic boost in front of a driven amp: try DRIVE low and LEVEL up.
- **Classic** is the original overdrive. Projects and rigs saved with earlier versions load with Classic, so they sound exactly as before.
- **New FILTER / PITCH block,** after the compressor and before the drive (where these pedals go on a real board). Click it in the signal chain to open it. Two types:
  - **Auto-Wah:** an envelope filter. A resonant wah sweeps open as you pick and closes as the note fades. SENSITIVITY sets how hard you need to play to open it fully; RESONANCE sets how sharp and vocal it is.
  - **Pitch Shifter:** two octaves down to two up, in semitones. Use MIX 100% for an all-wet pitch drop, or blend it in for octaves and harmonies (+12 for an octave up, +7 for a fifth). Each time it splices, it finds the point where the waveform matches and cross-fades there, so notes keep a clean, steady pitch instead of warbling.
- **Doubler** (new MODULATION type): two extra "takes" of your part, one each side, each a little late and drifting slightly in timing and pitch, so it sounds double-tracked. DEPTH sets how loose the takes are, RATE how fast they drift, and MIX how loud they are.
- Four new factory rigs: Octave Lead and Harmony Fifths (bank B), Funk Envelope (bank C) and Wide Doubler (bank D).
- With nine blocks now, the signal chain's gaps are a little narrower, and the RIG SUMMARY cards use short titles (hover for the full name).

## 0.9.0

**Six new modulation effects, and tempo sync.**

The MODULATION block's TYPE list grows from four to ten. The original chorus, flanger, phaser and tremolo sound exactly as before.

- **Dimension Chorus:** after the Roland Dimension D and Boss CE-1. Two chorus voices swept in opposite directions, so their pitch changes largely cancel. The result is wide and lush without the seasick wobble, and is slightly darkened like the originals' bucket-brigade chips.
- **Uni-Vibe:** after the Shin-ei Uni-Vibe. Four phase-shift stages tuned far apart, swept by a simulated lamp that brightens quickly, fades slowly and lingers dark, which gives the Vibe its lopsided throb. MIX works as the original's switch: all the way up is VIBRATO, halfway is CHORUS.
- **Rotary:** a Leslie speaker. The sound splits at 800 Hz into a treble horn and a bass drum, each spinning past two microphones with its own Doppler pitch shift and level swing, in stereo. RATE sets the speed. The heavy drum takes longer to spin up and down than the horn, so turning RATE (or assigning it to automation) gives the classic chorale-to-tremolo ramp.
- **Vibrato:** pure pitch vibrato, with no dry signal.
- **Harmonic Tremolo:** the early-'60s Fender "brownface" tremolo. Lows and highs pulse in turn, for a softer, swirling throb with a hint of phaser.
- **Auto-Pan:** sweeps the sound between the speakers, at constant loudness.
- **SYNC** (new, for every type): Off, or a note length from 1/1 to 1/16, including dotted and triplet values. The rate then follows the tapped tempo, or the DAW's. RATE greys out while synced, and the footswitch shows the note value.
- MIX greys out for types that have no dry/wet blend (tremolo, vibrato, harmonic tremolo, auto-pan). For those, MORPH / FX on the expression pedal moves DEPTH instead, as it already did for tremolo.
- Six new factory rigs to try them: Leslie Spin, Machine Gun Vibe, Dimension Clean, Warble Vibrato (bank D), and Brownface Trem and Wide Pan (bank C, tempo-synced).
- Effect panels now size their cards by contents: drop-down lists get more room and ON lights less, so long names like "Harmonic Tremolo" and "1/4 triplet" fit.

## 0.8.1

- The graphic EQ now glows in the plugin's neon orange-red (its curve, faders, icon and signal-chain block), with a white-hot core on the curve like the unit's neon LEDs.

## 0.8.0

**9-band graphic EQ.**

- A graphic EQ with one band per octave (63, 125, 250, 500 Hz, 1, 2, 4, 8 and 16 kHz), each cutting or boosting up to 12 dB, plus an output LEVEL. It sits after the cabinet and PRESENCE, before modulation, delay and reverb, where hardware modellers put theirs: it shapes the amp and cab's tone without colouring the echoes separately.
- Click the new **EQ** block in the signal chain (or its card in RIG SUMMARY) to open it: nine faders standing at their frequencies over a live drawing of the EQ's response, which passes through the faders' caps as you move them. Double-click a fader for 0 dB; **FLAT** resets every band in one step (and one undo).
- Saved with each rig. It's off by default, so existing rigs and projects sound exactly the same.

## 0.7.5

**The mockup's own expression pedal.**

- The expression pedal is now the mockup's artwork itself (with X1 in place of RPX): the chrome housing with its neon-lit rail, and the treadle with its two black pads. It still rocks in 3D: the artwork is drawn through the same perspective as it tilts, so the lifted end comes towards you and the lowered end drops away, over a dark recess with the treadle's shadow.
- Hover over the pedal (or drag it) to see what it controls and its position.

## 0.7.4

**A real rocking pedal, piano-black keys and neon LEDs.**

- **Expression pedal in 3D:** the treadle is now modelled as a solid slab rocking on an axle, seen through a perspective camera, the way Neural DSP and IK Multimedia show theirs. As it rocks, the lifted end comes towards you, the lowered end drops away, its thick edge and its shadow on the base show, and the gloss on its pads slides with the tilt. It still glides smoothly between positions.
- **Piano-black keys:** footswitches, the AMP / CABINET / RIG blocks and the small buttons are now glossy black keys with chamfered edges: a light catch along the top bevel and the red glow reflected in the side bevels, as in the mockup.
- **Neon LEDs:** lit LEDs (footswitch bars, button stripes, ring lights) have a white-hot core inside a red-orange outline, like the mockup's; unlit ones are a dimmer solid red.

## 0.7.3

**The mockup's knobs, and a moving expression pedal.**

- **OUTPUT LEVEL and PRESENCE** are now drawn like the mockup's: a black cylinder seen slightly from above with a spun-metal top, red light reflected up its sides, and a thin neon ring round its base. The ring is open at the front, with hooked ends and marks at the sides. A fine engraved line on the top shows the setting.
- **The expression pedal moves:** as it goes towards the heel the treadle tilts back, its toe lifting, so it looks shorter and the well shows beyond its tip. It glides smoothly to each new position, whether you drag it, use the mouse wheel, automate it, or play a MIDI pedal.

## 0.7.2

**Richer metal and the mockup's LCD.** Colours were measured off the mockup and matched point by point.

- **LCD screen:** proper seven-segment digits that lean like the mockup's, sized and spaced the same, with unlit segments hidden. Dark orange-tinted glass in a thick glossy black bezel, a larger rig name, a bigger BANK / PRESET line, and a small glowing red dot for unsaved changes.
- **Metal:** a deeper rose-silver face that shades towards its edges and picks up pink light at the bottom, a dark mauve chrome rim, pink chrome round the expression pedal, and pure black pedal pads. Smoother brushed finish.
- **Knobs** sit in a pool of red light, and the button cluster's red stripes glow even when off, as in the mockup.
- Darker, near-black background and panels, as in the mockup.
- Fixed stray light arcs on the unit's left edge and on the panels.

## 0.7.1

**Closer still to the mockup.**

- **Deeper chrome:** higher-contrast metal, a thicker rim with the unit's darker front edge showing along the bottom, and a soft red glow round it instead of a hard neon outline. The face plate is neutral brushed silver, with the red light reflected only near its lower edge.
- **Footswitches** are single glossy rubber blocks, as in the mockup.
- **Expression pedal:** darker bay, no dot strip, and X1 stamped in outlined letters like the mockup's RPX.
- **Block knobs** (when you open a block from the summary, a footswitch name or the signal chain) now have the block's icon and name at the top and each control on its own card, matching the rest of the bottom panels.

## 0.7.0

**The mockup's look, in detail.**

- **Inter typeface** built in, so text looks crisp and modern on every computer, with a heavy italic **PARAGON X1** badge as in the mockup.
- **The floor unit:** rose-tinted chrome with a bevelled rim and red light spilling round it. The slot row and the footswitch row sit on raised panels, and the **expression pedal is built into the unit** with two rubber pads and an embossed X1.
- **3D footswitches**, black slot blocks with glowing icons, the mockup's red ring LEDs, red labels on the button cluster with the HOLD TO TUNER bracket, light outlined VOLUME / WAH / MORPH buttons, and a **MIN** light that shows when the pedal is fully back.
- **Header:** grouped tabs (now with **SETTINGS**), the rig name between orange arrows, BPM + TAP together, and undo, redo, save and settings icons.
- **Bottom of the window, as in the mockup:** **RIG BROWSER** shortcuts, **SELECTED RIG** (name, favourite star, bank and number, save), **TAGS** (+ to add, click to remove) and **RIG SUMMARY** cards for every block, plus **EXPRESSION ASSIGN**. Click a card, a footswitch name or a signal-chain block to edit that block's knobs; RIG SUMMARY goes back.
- **SETTINGS page:** the tuner's reference pitch (A4 = 430-450 Hz), mute while tuning, which **MIDI CC** moves the expression pedal, the rigs folder, and credits.
- Slim dotted **level meters**, and a bigger **signal chain**.
- The AMP and CABINET windows show the loaded file's name underneath; hover for the full path and details. The unload (x) button appears on hover.

## 0.6.0

**Performance features: tuner, tap tempo, undo and the expression pedal.**

- **Tuner:** hold **TAP** or press **TUNER** on the pedal. The LED screen shows the note, a cents meter (green when in tune) and the frequency, accurate to under a cent from a 7-string's low B upwards. **MUTE** silences the output while you tune (on by default).
- **Tap tempo:** **TAP** in the header or on the pedal. The tapped tempo (in orange) drives the synced delay instead of your DAW's; click the BPM to go back to the DAW's tempo. The TAP light flashes on the beat, and the tapped tempo is saved with your project.
- **Undo / redo:** header arrows, **UNDO / REDO** on the pedal, or Ctrl+Z and Ctrl+Shift+Z. A whole knob turn, click, file load or rig change is one step, and undoing a rig change brings back the previous rig's name too.
- **Expression pedal** beside the unit, as in the mockup. Drag it, scroll it, or move it from a real pedal on **MIDI CC 11**. Assign it to:
  - **Volume** (with a volume pedal's feel: half way is about -12 dB)
  - **Wah / Filter** (a 350 Hz-2.2 kHz wah before the drive)
  - **Morph / FX** (fades modulation, delay and reverb in from dry at the heel).
  **MIN / MAX** in the new EXPRESSION ASSIGN panel set the heel and toe settings. The assignment and range are saved with each rig.
- Icons on the PROCESSOR and PRESETS tabs.

## 0.5.0

**The pedalboard look from the mockup.**

- **Silver floor unit:** brushed metal with the red-orange edge glow, PARAGON X1 badge, and an orange LED display showing the rig number, name, bank and preset, with a dot when there are unsaved changes. Click the display to browse rigs.
- **Amp, Cabinet and Rig windows** with icons, arrows and captions, as in the mockup. The Rig window steps through rigs; click it to open the browser.
- **Big OUTPUT LEVEL and PRESENCE knobs** on the pedal face, **SAVE / SAVE AS** under the display, and the four footswitches as rubber pads with LED bars.
- **BYPASS button:** bypasses the whole plugin. It's the same switch as your DAW's bypass, and stays in time with the rest of your mix.
- **Signal chain** strip along the bottom: each block lights in its own colour when it's on. Click a block to show its knobs; double-click to switch it on or off.
- **Edit panel** for the selected block (now one each for compressor, drive, amp, cabinet, modulation, delay and reverb), and a **Current Rig** panel with favourite, bank, tags and save buttons.
- Your DAW's **tempo** shows in the header.
- The window opens at 1280 x 720 and scales smoothly up to 1920 x 1080, staying sharp at any size.

## 0.4.0

**Rigs and presets.** Save complete sounds and switch between them.

- **Rigs:** a rig stores the amp model, cabinet IR and every effect setting. Save the current sound with **SAVE** in the header, step through rigs with the **<** and **>** arrows, and see a `*` next to the name when you've changed something since loading or saving.
- **PRESETS tab:** browse all rigs, favourites, your own rigs or factory rigs; filter by bank A-D; search by name or tag; rename, re-file, tag, favourite or delete rigs; open the rigs folder.
- **Factory rigs:** ten starter sounds (Studio Clean, Edge of Breakup, Lead Boost, High Gain Tight, Ambient Swells, Slapback and more). They set the effects and keep your own amp and cabinet.
- **OUTPUT LEVEL** knob (next to Presence): each rig's own volume, for balancing rigs. The OUTPUT knob on the right is still the overall volume, and INPUT and OUTPUT don't change when you switch rigs.
- Rigs are files in your user folder (`%APPDATA%\Paragon\Paragon X1\Rigs` on Windows), so you can back them up and share them. Your DAW project remembers which rig was loaded.

## 0.3.0

**Effects.** Four footswitches, as in the mockup.

- **Comp / Gate:** compressor and noise gate before the amp.
- **OD / Distortion:** overdrive with drive, tone and level, 8x oversampled.
- **Modulation:** stereo chorus and flanger, phaser, tremolo.
- **Reverb / Delay:** tempo-synced or free delay with tone and ping-pong, and stereo reverb; both ring out when switched off.
- **Presence** knob after the cabinet.

## 0.2.0

**Amp and cabinet.**

- Load `.nam` models and cabinet IRs (WAV, AIFF, FLAC), by clicking or drag and drop; step through a folder with the arrows.
- Normalise option to level-match models; same resampler and IR level as the official NAM plugin.
- Click-free switching; files are remembered in your DAW project.

## 0.1.0

First build: input and output gain with level meters.
