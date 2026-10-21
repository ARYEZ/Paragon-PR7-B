# Paragon X1

A guitar multi-effects VST3 plugin built around [Neural Amp Modeler](https://github.com/sdatkinson/NeuralAmpModelerCore)
(`.nam`) amp captures, with a cabinet IR loader and built-in effects. Built with [JUCE](https://juce.com).

![Paragon X1 mockup](Paragon%20x1.png)

*Design mockup. The plugin doesn't look like this yet; see the roadmap.*

## Roadmap

| Phase | Scope | Status |
|---|---|---|
| 1 | Project skeleton, CI builds for Windows/macOS/Linux, input/output gain, level meters | ✅ Done |
| 2 | Load `.nam` amp models and cabinet IRs | ✅ Done |
| 3 | Effect blocks: comp/gate, overdrive, modulation, delay, reverb, with footswitches | ✅ Done |
| 4 | Rigs, presets, banks, tags | Planned |
| 5 | Full pedalboard UI from the mockup | Planned |
| 6 | Expression pedal/morph, tap tempo, undo/redo, tuner | Planned |

## Using it

- **Amp:** click the amp display (or drag a `.nam` file onto it) to load a model. The ◀ ▶ arrows step
  through the other models in the same folder. The LED turns the amp on and off.
- **NORMALISE** levels every model to the same loudness, like the official NAM plugin's "Normalized"
  output. It needs loudness data in the model; older models without it play at their raw level.
- **Cabinet:** load a `.wav`, `.aiff` or `.flac` impulse response the same way. Like the official
  plugin, IRs are turned down by a fixed 18 dB and not trimmed or normalised.
- Models and IRs load in the background and crossfade in, so switching while playing doesn't click.
- **Sample rate:** NAM models are captured at a fixed rate (almost always 48 kHz). At other rates the
  plugin resamples with the same resampler as the official plugin and reports the small added
  latency to your DAW.
- **PRESENCE** (under the cabinet) cuts or boosts the top end after the cab, like an amp's presence knob.
- **Footswitches 1-4** turn the effect blocks on and off. Click a footswitch's name to show its knobs
  below. Signal chain: comp/gate → overdrive → amp → cabinet → presence → modulation → delay → reverb.
  - **1 Comp / Gate:** compressor (threshold, ratio, level) and noise gate (threshold; all the way down is off).
  - **2 OD / Distortion:** overdrive (drive, tone, level), 8x oversampled to avoid harsh aliasing.
  - **3 Modulation:** chorus, flanger, phaser or tremolo. Chorus and flanger are stereo.
  - **4 Reverb / Delay:** switches both; each also has its own ON switch. The delay syncs to your
    DAW's tempo (or set SYNC to Off for milliseconds), has a tone control for darker repeats, and
    optional ping-pong. Both let their tails ring out when switched off.
- All effects start switched off, so projects made with earlier versions sound the same.
- The paths of the loaded files are saved in your DAW project. If a file is missing when the project
  opens, the slot says so and keeps the path, so moving the file back fixes it.

## Download

Every push builds the plugin on GitHub Actions. Open the **Actions** tab, pick the latest
green run, and download `ParagonX1-VST3-<your OS>` from the **Artifacts** section.

Unzip it and copy `Paragon X1.vst3` to your VST3 folder:

| OS | VST3 folder |
|---|---|
| Windows | `C:\Program Files\Common Files\VST3` |
| macOS | `~/Library/Audio/Plug-Ins/VST3` |
| Linux | `~/.vst3` |

**macOS:** the build isn't code-signed yet, so macOS will block it. After copying, run
`xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/"Paragon X1.vst3"` once.

## Building from source

Requirements: CMake 3.24+, a C++20 compiler (Visual Studio 2022, Xcode 15+, or GCC 11+/Clang 14+), and Git.
JUCE, NeuralAmpModelerCore, Eigen and Catch2 are downloaded automatically at pinned versions.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

The plugin is written to `build/ParagonX1_artefacts/Release/VST3/`.

On Linux, install JUCE's dependencies first (the list is in `.github/workflows/build.yml`).

To render the editor to an image without a DAW:

```bash
PARAGON_SNAPSHOT=editor.png ./build/tests/ParagonX1Tests "[snapshot]"
```

## License

Paragon X1 is free software under the [GNU Affero General Public License v3](LICENSE.md),
which is required by JUCE's open-source licence.

It uses:
- [JUCE](https://github.com/juce-framework/JUCE): AGPLv3
- [NeuralAmpModelerCore](https://github.com/sdatkinson/NeuralAmpModelerCore): MIT
- [Eigen](https://eigen.tuxfamily.org): MPL 2.0
- [AudioDSPTools](https://github.com/sdatkinson/AudioDSPTools) (resampler): MIT, with parts from iPlug2 under the zlib-style iPlug2 licence

"Neural Amp Modeler" is Steven Atkinson's project. Paragon X1 is an independent player for `.nam`
files and isn't affiliated with or endorsed by it.
