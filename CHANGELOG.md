# Changelog

Each version's notes appear on its GitHub Release page.

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
