# Paragon X1

A guitar multi-effects VST3 plugin built around [Neural Amp Modeler](https://github.com/sdatkinson/NeuralAmpModelerCore)
(`.nam`) amp captures, with a cabinet IR loader and built-in effects. Built with [JUCE](https://juce.com).

![Paragon X1 mockup](Paragon%20x1.png)

*Design mockup. The plugin doesn't look like this yet; see the roadmap.*

## Roadmap

| Phase | Scope | Status |
|---|---|---|
| 1 | Project skeleton, CI builds for Windows/macOS/Linux, input/output gain, level meters | ✅ Done |
| 2 | Load `.nam` amp models and cabinet IRs | Planned |
| 3 | Effect blocks: comp/gate, overdrive, modulation, delay, reverb, with footswitches | Planned |
| 4 | Rigs, presets, banks, tags | Planned |
| 5 | Full pedalboard UI from the mockup | Planned |
| 6 | Expression pedal/morph, tap tempo, undo/redo, tuner | Planned |

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

"Neural Amp Modeler" is Steven Atkinson's project. Paragon X1 is an independent player for `.nam`
files and isn't affiliated with or endorsed by it.
