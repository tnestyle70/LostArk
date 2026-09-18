# SoundPipeline

Reads the Wwise audio the game ships in
`C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/WwiseAudioPackage`
(`*.pck` packages plus the loose streamed `*.wem`). UModel cannot open these:
they are not UPK packages, and the audio lives in obfuscated-name banks rather
than as named objects.

## Container

Every file begins with the 4-byte marker `3E CE A6 74`. Everything after it is
XORed with a single keystream that is the same for every file and repeats every
**435,540 bytes**. The marker is not part of the stream.

Strip the marker and XOR, and a `.pck` is a stock Audiokinetic **AKPK** (header,
language map, soundbank LUT, streamed-file LUT, payloads) and every payload is a
stock `.bnk` (BKHD/HIRC, bank version 134) or Wwise RIFF `.wem`.

The client's own routine could not be read: `LOSTARK.exe` and `EFEngine.dll` are
wrapped by WinLicense (`.winlice`, `.vm_sec`, `.boot` sections) and the marker
does not appear anywhere in the shipped binaries. The table in
`wwise_keystream.dat` was recovered from the shipped data instead.

## Files

| File | Role |
|---|---|
| `wwise_audio_package.py` | Decrypt, AKPK/BNK parsing, event-name resolution, `.wem` extraction. Import it or run it as a CLI. |
| `wwise_vorbis_to_ogg.py` | Wwise Vorbis `.wem` -> standard Ogg Vorbis. Port of ww2ogg, with revorb's page and granule work folded in. `--wav` also writes PCM. |
| `fmod_decode.py` | Decodes to 16-bit PCM `.wav` by driving the project's own `fmod.dll`. |
| `render_events.py` | Event name -> `.wem` -> `.ogg` (and `.wav` with `--wav`) in one command. |
| `recover_wwise_keystream.py` | Regenerates and verifies `wwise_keystream.dat`. Needs numpy. |
| `wwise_keystream.dat` | The 435,540-byte keystream (Git LFS, `*.dat` rule). |
| `wwise_vorbis_codebooks_aoTuV_603.dat` | ww2ogg's `packed_codebooks_aoTuV_603.bin` (Git LFS, `*.dat` rule). |
| `WW2OGG_COPYING` | The BSD 3-clause notice the port carries. |

## Resolving a name

Wwise does not ship object names. An event object's ID is the **FNV-1 32-bit
hash of its lowercased name**, so a known event such as
`S_Vehicle_TrisionHorse_Dash1` hashes straight to its HIRC object. From there
only **Play** actions (`AkActionType` high byte `0x04`) are followed; Stop,
Pause and Resume actions point at the same containers and would pull in audio
the event never starts. The Play target is walked down through
Random/Sequence, Switch, Layer and ActorMixer containers to the Sound objects,
whose `AkBankSourceData` gives the `.wem` media id to pull out of the package
LUT.

Several `.wem` behind one event are equally-weighted variations, which is what
`CSoundCueCatalog` stores as the variant array for that event.

## Decoding

The `.wem` payloads are Wwise Vorbis (fmt codec id `0xFFFF`) with **stripped
setup packets**: the codebooks are not in the file at all, only 10-bit indices
into the fixed aoTuV 6.03 library that the Wwise runtime has compiled in.
Packets also carry a 2-byte Wwise header instead of Ogg framing, and with those
headers there is no granule position anywhere in the file.

`wwise_vorbis_to_ogg.py` does all of that here, so the pipeline no longer needs
vgmstream, ww2ogg or revorb:

- rebuilds the three Vorbis headers, expanding each indexed codebook out of
  `wwise_vorbis_codebooks_aoTuV_603.dat`
- restores the packet type and the window flags Wwise drops from modified packets
- laces packets into real Ogg pages and derives every granule position from the
  mode and blocksize sequence, which is the job ww2ogg leaves to revorb; skip it
  and a decoder reads the file as zero samples long

Output is Ogg Vorbis. FMOD opens it through the same `System::createSound`
call the engine already makes for `.wav` -- the shipped `fmod.dll` is 2.03.12 and
carries `FMOD Ogg Vorbis Codec` -- so a file dropped into
`Client/Bin/Resources/Sound/...` needs no code change.

`--wav` additionally writes 16-bit PCM, which is what the other 4,145 files
under that tree are. The decoder for that is the project's own
`Engine/ThirdPartyLib/FMOD/Bin/fmod.dll`, driven through its C API by
`fmod_decode.py`: no new third-party binary, and the samples written are exactly
the ones the client would play. It opens FMOD on the `NOSOUND` output, so it
never takes the audio device and is safe to run while the game is up.
PCM is about fourteen times the size -- 1.15 MB against 91 KB for a six-second
instrument track.

It is a port of ww2ogg (Adam Gashlin / Xiph.org, BSD 3-clause -- see
`WW2OGG_COPYING`). Not covered: the pre-2011 header-triad layout (`vorb`
0x28/0x2C) and codecs other than Vorbis; both raise instead of guessing.

### Checked against the shipped renders

The team's 2026-08-23 sound dump (11,644 WAVs plus a manifest naming each media
id) was produced by a working decoder, so it is an oracle. `verify_vorbis_against_renders.py`
pulls the same media ids out of the install, converts them here and compares the
decoded PCM.

| Sample | Result |
|---|---|
| the five `ui_pc_inst_*_return*` instrument renders | 1.000000 on both channels, identical frame counts |
| 60 random entries across every category | 60/60 converted, 59 exact |
| a further 40 random entries | 37 exact, 3 reference-clipped, 0 failed |

The entries that are not exact differ only where the reference WAV clipped at
1.0: a lossy decoder regularly passes full scale and 16-bit PCM cannot, so the
converted file is the more faithful of the two.

The game ships one variant. 250 randomly sampled loose `.wem` are all codec
`0xFFFF` with a 0x42 `fmt ` and no separate `vorb` chunk, and every media
decoded above is modified packets / 2-byte headers / no granule, 1 or 2
channels, blocksizes 256 and 2048. The 6-byte-header and standard-packet paths
are ported from ww2ogg but nothing shipped exercises them.

## Usage

```powershell
$py = "C:/Program Files/Blender Foundation/Blender 5.0/5.0/python/bin/python.exe"

# --package-root is found automatically (LOSTARK_PACKAGE_ROOT, then the known
# install roots); pass it explicitly for an install somewhere else.

# which packages exist (names are deobfuscated on the fly)
& $py Tools/SoundPipeline/wwise_audio_package.py --filter SOUND_VEHICLE --list

# what a named event actually plays, and pull the .wem out
& $py Tools/SoundPipeline/wwise_audio_package.py --filter SOUND_VEHICLE `
    --event S_Vehicle_TrisionHorse_Dash1 --extract out/wem

# event name straight to playable .ogg
& $py Tools/SoundPipeline/render_events.py --filter SOUND_UI `
    --event ui_pc_inst_warrior_return --out out/sound --name squarehole_song

# or convert .wem already on disk (one file, or a whole tree)
& $py Tools/SoundPipeline/wwise_vorbis_to_ogg.py out/wem --out-dir out/ogg

# add --wav anywhere above to get 16-bit PCM as well
& $py Tools/SoundPipeline/wwise_vorbis_to_ogg.py out/wem --out-dir out/ogg --wav

# or decode audio FMOD can already open
& $py Tools/SoundPipeline/fmod_decode.py out/ogg --out-dir out/wav

# re-verify the keystream table against the installed game
& $py Tools/SoundPipeline/recover_wwise_keystream.py --verify-only
```

`--verify-only` runs both independent checks: the 60 seed bytes reconstructed
from the empty AKPK packages, and every LUT `languageID` field in every
installed package (209,675 samples). Both must come back with no mismatch.

Consumers: `Tools/VehiclePipeline/build_vehicle_sound_catalog.py`.
