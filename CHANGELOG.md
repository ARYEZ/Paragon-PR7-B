# Changelog

Each version's notes appear on its GitHub Release page.

## 0.21.0

- **MORPH, a bass envelope filter, is FILTER / PITCH's fifth TYPE.** It's modelled on the MXR M82 Bass Envelope Filter, built from the specifications in its manual. An envelope filter is a wah that your playing works instead of your foot: dig in and it opens up and quacks, play softly and it closes.
  - **Its five knobs, as on the pedal.** They all start at noon, as the manual suggests.
    - **DRY:** your clean bass, from off to +6 dB. This is the M82's big idea: the filter doesn't have to carry your low end, so the lows stay solid however wild the filter gets.
    - **FX:** the filter's level.
    - **DECAY:** where the filter rests when you're not playing, and where it falls back to as a note fades: from 76 Hz (deep and round) to 1.3 kHz (bright and vocal).
    - **Q:** how sharp and vowel-like the filter's peak is. As in the manual, its peak goes from +8 dB up to +18 dB low down and +26 dB at the top of the sweep.
    - **SENS:** how hard you have to play to open it, over the manual's 46 dB range. At full SENS a medium-strong note opens it all the way, to 3.2 kHz.
  - **Its output** rounds off near the pedal's maximum output instead of clipping hard.
  - **Its own faceplate:** the screen shows what MORPH is doing to your bass right now, the dry level plus the filter's peak, moving as you play, with the filter's frequency. A faint line marks where DECAY rests it.
- **Three new rigs, from the manual's sample settings**, read off its knob drawings, under our own names:
  - **C8 Synth Bass Filter:** almost all filter, resting low: round, synthy notes.
  - **C9 Seventies Funk Filter:** the classic funk quack, with dry bass underneath.
  - **C10 Dance Filter:** resting high, so it stays bright and squelchy between notes, for techno and dance. It has so little dry bass that it's a quiet sound, so it uses the EQ's level (flat) on top of RIG LEVEL to match the other rigs.
  - As the manual says, adjust SENS to suit your bass's output.
- **Our guesses, not from the manual:**
  - There's no schematic, so the filter type is our reading. We use a band-pass, because the manual says DRY is there to keep the low end.
  - How fast the envelope rises (3 ms) and falls (120 ms).
  - How many dB of playing take it from closed to fully open (24 dB).
  - The knob curves.
- **A name to sort out:** the expression pedal already has a **MORPH / FX** setting, which blends the delay, reverb and modulation in. It has nothing to do with this filter.

## 0.20.0

- **LUMEN, an optical bass compressor, is COMP / GATE's third TYPE**, after Classic and Daybreak. It's modelled on the Ampeg Opto Comp, from its manual. An optical compressor turns the bass down with a small lamp shining on a light-sensitive cell: the harder you play, the brighter the lamp and the more it turns down. That gives the smooth, even squeeze bass players like optos for.
  - **Its knobs, as on the pedal:**
    - **COMPRESSION** sets the ratio, from 1:1 (none) to 10:1, spread exponentially as the manual says, so noon is about 3:1.
    - **RELEASE** sets how fast it lets go, from 75 ms to 600 ms (about 210 ms at noon).
    - **LEVEL** sets the output (the compressor's shared LEVEL).
    - **PAD -15** turns the bass down 15 dB going in, for hot active basses, so it compresses and clips less.
  - **Like the pedal:** it listens to the bass before it turns it down (feed-forward) and to both halves of the wave. Its input stage stays clean up to near the pedal's headroom, then rounds off peaks rather than clipping hard.
  - **The light cell lingers:** after a long stretch of hard compression it lets go up to half as slowly again, as real light cells do.
  - **Its own faceplate:** the light cell glows on the screen as hard as it's working, with the gain reduction in dB and LED bars.
- **Our guesses, not from the manual:** the manual gives the ratio, release and level ranges but not where it starts compressing (we use -24 dB), how soft the knee is, how fast it grabs (about 10 ms), or how much the cell lingers. Those are our best reading of how optos behave.
- **Our choice:** the real PAD leaves you to make up its 15 dB with LEVEL. Ours gives the 15 dB back after the compressor, so switching it on doesn't make the bass drop in volume.
- **Footswitch 1's description now follows the TYPE:** it shows Lumen's or Daybreak's own knobs instead of always showing the Classic's threshold and ratio.

## 0.19.0

- **Amp/DI blend in the AMP (NAM) block:** the classic studio bass sound, a clean DI signal mixed with the amp and cab. The DI gives tight, solid lows and a clear attack; the amp gives the character.
  - **BLEND** (0 to 100%) sets how much DI goes in. At 0%, the default, the sound is exactly as before, so your rigs and projects don't change.
  - **POINT** sets where the DI is taken:
    - **After Pedals:** what goes into the amp, like a DI box between your pedalboard and the amp. This is the usual studio way.
    - **Before Pedals:** your bass itself, clean, whatever the drives are doing.
  - **PHASE** flips the DI, as on a mixing desk: if a blend sounds thin or hollow, the two are partly cancelling, and flipping one fixes it.
  - **In time with the amp:** when an amp model delays its sound (one made at another sample rate), the DI is delayed by the same amount, so the two line up instead of comb-filtering. The amp's screen shows the DI setting while it's in use.
- **New rig: A7 Amp + DI Blend.** A compressed clean bass with 40% DI taken after the pedals. It keeps your amp model and cab IR, like the other factory rigs.

## 0.18.0

- **HALO, a germanium fuzz, is OD / DISTORTION's third MODEL**, after Obsidian and Gotham. It's PR7-G's Halo: the original two-transistor germanium fuzz of the late '60s, solved part by part with the published component values, at 8x oversampling.
  - **Its knobs, as on the original:** FUZZ and its volume (our OUTPUT). There's no tone control, because the original has none.
  - **It reacts to how you play:** dig in for full fuzz, and play softer and it cleans up. Its uneven clipping gives the strong 2nd harmonic germanium fuzzes are known for.
- **Clean lows under the fuzz, as you chose:** HALO works with OD / DISTORTION's **SPLIT** (on the EQ / SPLIT page). With SPLIT on:
  - everything below SPLIT HZ goes around the fuzz, clean;
  - LOWS sets that clean band's level.
  - This matters more for a fuzz like this than for the other models: its circuit thins the deepest bass on its own, as the original does.
- **New rig: B8 Germanium Split Fuzz.** HALO at FUZZ 7, with SPLIT on at 200 Hz.
- **Old Face Fuzz projects now open on HALO:**
  - This covers projects and rigs saved with Face Fuzz in PR7-B 0.1.0 to 0.3.0, before the guitar drives were removed. They used to open on Obsidian.
  - They keep the same FUZZ setting.
  - Nothing else changes: rigs and projects saved since then keep their model.
- **Not modelled:** a passive bass is assumed. An active bass, with its much lower output impedance, makes the real circuit fuzz harder than this model does.

## 0.17.1

- **Our own names for the factory rigs that had trademarks, artists or song titles in them.** Every rig sounds exactly as before and stays in its slot; only the names and notes changed.
  - **A5** Motown Thump is now **Vintage Thump**, and **B7** Nolly Split is now **Split Drive**.
  - **The song rigs in bank E** are named for their sound:
    - E1 **Buzzsaw Octave Fuzz**
    - E2 **Flanged Prog Riff**
    - E3 **Hollow Clank Slap**
    - E4 **Warm Grit Groove**
    - E5 **Dark Overdrive Push**
    - E6 **Thick Prog Grind**
    - E7 **Fretless Grit**
    - E8 **Extended-Range Drive**
    - E9 **Bright Funk Slap**
    - E10 **Sixties Soul Groove**
    - E11 **Melodic Chorus Lead**
  - **The notes** in the rig browser describe the sound without naming artists or songs. The amp and cab advice still names gear (Ampeg SVT, Mesa 8x10 and so on), and the tuning and tempo tips are unchanged.
  - **Nothing to redo:** projects, saved rigs and favourites that used the old names find the renamed rigs automatically.
- **The graphic EQ's preset** Motown Thump is now **Vintage Thump**, to match.

## 0.17.0

- **GRAPHIC EQ: the fader lights now light up like the PR7's other keys.** While the EQ is on, the light on each fader's cap (all nine bands and LEVEL) is the same bright neon bar as the lit ON key, with its white-hot core and glow. Before, it was a thin pink line that looked the same whether the EQ was on or off.
  - While the EQ is off they show the dim red of an unlit key, so the faders also tell you at a glance whether the EQ is working.
  - They follow ON however it changes: a click, the stomp, a rig or the DAW's automation.
  - In every finish (Pearl White, White + Neon, Pearl Black) the caps stay black, so the dim light stays visible on them too.

## 0.16.1

- **SHOW RACK no longer makes the window too big** (as on PR7-G 0.53.1). In Cubase, showing the rack made the whole plugin about 12% bigger, running it off the right and bottom of the screen.
  - The cause: the plugin asked Cubase for the taller size while still giving it the rack-hidden window's limits and shape. Cubase checks every new size against those, so it "corrected" the size to a bigger one.
  - Now the new shape and limits go in first, then the size, so the window is exactly the width it was, only taller.
- **The window stays on the screen.** The DAW keeps the window's top left corner where it is, so the window grows downwards when the rack opens. If the full-width window wouldn't fit below where it is, it's made narrower, just enough to fit. Hiding the rack again goes back to your width.
  - To get the rack at full size, drag the window up and widen it from its corner.

## 0.16.0

- **HIDE RACK** (as on PR7-G): a button at the top right of the SIGNAL CHAIN strip hides the rack when you're not editing an effect, for a smaller, cleaner window.
  - The SIGNAL CHAIN moves up under the floor unit and the window gets shorter by the rack's height. It keeps its width, so the floor unit stays the same size.
  - **SHOW RACK** (the same button) brings it back, and so does clicking a block in the SIGNAL CHAIN, which opens the rack on that block. Footswitches don't open it.
  - It's remembered on this computer, like the finish: every PR7-B you open afterwards starts with the rack shown or hidden, the way you left it.
  - The PRESETS and SETTINGS pages fit the shorter window too.

## 0.15.0

- **THE RACK, stage 2: the GRAPHIC EQ rebuilt for the rack, at full size** (as on PR7-V and PR7-G). In 0.14.0 it sat in a screen on the rack, a little smaller; now it's on the unit's face like every other block, and nothing on the rack is scaled down.
  - **The nine faders** (33 Hz to 9 kHz): evenly spaced, as on a hardware graphic EQ, with LEVEL a little apart. Each value is on a small screen above its fader and each frequency is stamped under it.
  - **A printed red dB scale** and a 0 dB line across the faders.
  - **The keys:** ON, **ULTRA LO and ULTRA HI** are the unit's keys with light stripes; FLAT is a key and PRESETS is on a screen.
  - **The display shows the EQ's curve**, the ULTRA switches included. They work with the faders off, as on the amp, and the curve then shows just them.
  - The faders and switches are the real ones (same settings, automation and undo).

## 0.14.0

- **THE RACK, stage 1 of 2** (as on PR7-V and PR7-G): the faceplate, the block's controls and EXPRESSION ASSIGN are now **one wide rack unit** under the floorboard, the "brain" for the floorboard processor.
  - **Left: the block's display.** Only its screens and meters, since the real knobs are right beside it:
    - OBSIDIAN, GOTHAM and NAMP show the bass's level going in as big LED bars, with the SPLIT frequency when SPLIT is on;
    - Classic's needle fills the display, and Daybreak's gain reduction is big LED bars;
    - the delay's repeats, the reverb's shape and the modulation's wave each have their readout beside them;
    - PITCH's voices are LED columns.
  - **Middle: the block's real controls, at full size.** They're the same controls as before (same settings, automation and undo), drawn as the unit's own parts:
    - its big knobs, each value on a small screen;
    - lists on screens;
    - switches as small keys with a light stripe.

    A control that doesn't apply (another TYPE's, TIME while synced) leaves the face rather than greying out.
  - **OD / DIST has two pages** (keys at the top), because OBSIDIAN's twelve controls won't fit in one row at full size:
    - **PEDAL:** ON, MODEL, the pedal's knobs and switches, OUTPUT;
    - **EQ / SPLIT:** its BASS, MID and TREBLE, then SPLIT, SPLIT HZ and LOWS.

    Before, it squeezed onto two rows of smaller knobs. Every other block fits in one row.
  - **Right end: EXPRESSION ASSIGN**, behind its own printed red line. **GRAPHIC EQ** is one of the unit's keys, top right.
  - **The GRAPHIC EQ sits in a screen on the rack for now**, a little smaller. Stage 2 rebuilds it for the rack at full size, with the ULTRA LO and ULTRA HI keys.

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
