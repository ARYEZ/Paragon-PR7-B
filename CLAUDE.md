# Paragon PR7-B: notes for Claude

This repository starts as a copy of **Paragon PR7-G** (the guitar processor, repo `ARYEZ/Paragon-PR7-G`), full history included, to become **Paragon PR7-B**, the **bass** version. Everything below describes how the project is built, tested and released, and how the owner likes to work.

## First: make it its own plugin (do this before anything else)

Until these are done, PR7-B *is* PR7-G as far as a DAW is concerned. The two would replace each other and share rigs.

1. **CMakeLists.txt** (`juce_add_plugin`):
   - `PRODUCT_NAME` becomes "Paragon PR7-B".
   - `BUNDLE_ID` becomes `com.paragon.paragonpr7b`.
   - `PLUGIN_CODE` becomes a new code: four characters with exactly one upper case, e.g. `Pr7b`. Never PR7-G's `Px1a`.
   - Keep `PLUGIN_MANUFACTURER_CODE Prgn` (same maker).
   - Restart the version at `0.1.0` (`project(ParagonX1 VERSION ...)`; the CMake target name can stay).
2. **Rigs folder** (`source/rigs/RigLibrary.cpp`): "Paragon PR7-B".
   - Remove the carry-over from the old guitar folders. Bass users shouldn't inherit guitar rigs.
   - Update the rename test in `tests/RigTests.cpp` to match.
3. **Logo** (`source/ui/Logo.cpp`): the mark's "-G" becomes "-B", drawn in the same slanted, speed-cut lettering.
4. **Text:** the About text and MIDI hint in `source/ui/DockPanels.cpp`, the loader thread name in `PluginProcessor.h`, and `README.md` all say PR7-B.
5. **Release workflow** (`.github/workflows/build.yml`, `.github/release-install.md`):
   - Bundle paths become `Paragon PR7-B.vst3`.
   - Zips become `ParagonPR7-B-${TAG}-...`.
   - The release title becomes "Paragon PR7-B ${TAG}".
6. **CHANGELOG.md:** start fresh at 0.1.0 ("The bass processor, based on Paragon PR7-G 0.28.0").
7. **Re-enable releases.** The `release` job's `if:` was set to `false && ...` in the seed commit so this repo didn't publish a guitar release. Put it back to `github.event_name == 'push'` in the same commit as the version bump.

## The project

- An AGPLv3 **JUCE VST3** plugin: a NAM (Neural Amp Modeler) player with a full multi-effects chain, drawn as a pearl-white floor processor with an expression pedal.
  - The chain: PITCH → (WAH pedal) → COMP/GATE → FILTER/PITCH → GLITCH (Pre) → OD/DIST → AMP (NAM) → CAB (IR) → PRESENCE → TALK BOX → GRAPHIC EQ → GLITCH (Post) → MOD → DELAY → REVERB → LO-FI.
- **Code layout:** DSP in `source/dsp/effects/`, UI in `source/ui/`, parameters in `source/Parameters.*`, rigs in `source/rigs/`, tests in `tests/`.
- **Parameters:**
  - Saved by ID and choices by **index**: only ever *append* choices, never reorder.
  - New parameters get a new version hint number.
  - Migrations for changed defaults live in `setStateInformation` and `Rig::fromXml`.
- **Shared DSP pieces:** `BypassFade` and `TailWatch` (EffectUtils.h), `PitchShifter` (correlation-spliced, used by Whammy-Wah, Shimmer, PITCH and Glitch).

## Build, test, check

- **Build:** `cmake -B build && cmake --build build -j8`.
- **Tests (Catch2):** `xvfb-run -a build/tests/ParagonX1Tests`. All must pass. New features get tests that check what the feature really does to the sound (levels, frequencies, harmonics), not just that it runs.
- **pluginval:** `xvfb-run -a <pluginval> --strictness-level 10 --validate-in-process "build/ParagonX1_artefacts/Release/VST3/<name>.vst3"`. CI runs it too.
- **Screenshots** for checking the UI, via `xvfb-run -a build/tests/ParagonX1Tests "[snapshot]"` with environment variables:
  - `PARAGON_SNAPSHOT` (output path), `PARAGON_SNAPSHOT_WIDTH` (e.g. 2560 for 2x).
  - `PARAGON_SNAPSHOT_BLOCK=n` (open block n's panel), `PARAGON_SNAPSHOT_PARAMS=id=value,...`.
  - `PARAGON_SNAPSHOT_EXPRESSION`, `PARAGON_SNAPSHOT_PAGE=presets`.
  - Render, look, crop and zoom before claiming a visual change is right.

## Releases

- Bump `project(ParagonX1 VERSION x.y.z)` in CMakeLists.txt and add a matching `## x.y.z` section at the top of CHANGELOG.md, then push.
- CI builds Windows, macOS and Linux and publishes a GitHub Release with the three zips, using that changelog section as the notes.
- After pushing, check the release published (all three zips) about 15 minutes later.
- The owner installs from GitHub Releases on **Windows**.

## How the owner likes to work

- **Explain the why**, specifically: what was wrong, what changed, and the trade-offs. Plain language; the owner is a guitarist, not a DSP engineer.
- **Be honest about limits:**
  - Say when something is our design rather than a copy ("modelled on", "inspired by").
  - Say when a detail is a guess.
  - Say what can't be checked here (audio by ear).
  - Ask for source material, such as manuals, program charts or gear rundowns, rather than invent specifics.
- **No trademarked names or branding** on our models: "Screamer (TS)", "Muff Fuzz", "Face Fuzz", "Daybreak" (compressor), "GLITCH". "Inspired by" in descriptions is fine.
- **Show it:** send a rendered screenshot of any visual change.
- **"merge it"** means: open a PR from the working branch to main and merge it, then reset the working branch to the new main.
- Commit messages explain what and why.

## Ideas for the bass version (to plan with the owner)

- **Ranges for bass:**
  - Graphic EQ bands lower (from about 31 Hz).
  - Drive and fuzz voiced for low end, with a **clean blend** so the lows stay solid under distortion.
  - Bass-friendly compressor defaults.
- **Bass staples:** amp/DI blend, envelope filter and synth-bass sounds, and sub-octave (the OCTAVER already exists).
- Bass factory rigs and song rigs.
