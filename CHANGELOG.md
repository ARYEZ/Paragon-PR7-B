# Changelog

Each version's notes appear on its GitHub Release page.

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
