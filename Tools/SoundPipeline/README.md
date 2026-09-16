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
| `recover_wwise_keystream.py` | Regenerates and verifies `wwise_keystream.dat`. Needs numpy. |
| `wwise_keystream.dat` | The 435,540-byte keystream (Git LFS, `*.dat` rule). |

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

The `.wem` payloads are **Wwise Vorbis with stripped setup packets**, so they
need vgmstream (`vgmstream_cmd.exe`); this pipeline only decrypts, indexes and
extracts. vgmstream is not on PATH on this machine — the copy bundled with DSAS
under `Downloads/DSAS_4.9.9_HOTFIX_b/Res/vgmstream/` works and is what the
vehicle build defaults to.

## Usage

```powershell
$py = "C:/Program Files/Blender Foundation/Blender 5.0/5.0/python/bin/python.exe"

# which packages exist (names are deobfuscated on the fly)
& $py Tools/SoundPipeline/wwise_audio_package.py --filter SOUND_VEHICLE --list

# what a named event actually plays, and pull the .wem out
& $py Tools/SoundPipeline/wwise_audio_package.py --filter SOUND_VEHICLE `
    --event S_Vehicle_TrisionHorse_Dash1 --extract out/wem

# re-verify the keystream table against the installed game
& $py Tools/SoundPipeline/recover_wwise_keystream.py --verify-only
```

`--verify-only` runs both independent checks: the 60 seed bytes reconstructed
from the empty AKPK packages, and every LUT `languageID` field in every
installed package (209,675 samples). Both must come back with no mismatch.

Consumers: `Tools/VehiclePipeline/build_vehicle_sound_catalog.py`.
