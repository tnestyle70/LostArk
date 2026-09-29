"""Stage the original Guardian selection sound; never write live authoring data.

The source Matinee fires one AkEvent at intro time zero. Its single Play action
starts two simultaneous Layer voices with a 400 ms linear fade. Keep both WAVs
as separate World sound tracks on one existing carrier, not random variants.
Only the reviewed source HIRC payloads are accepted: a changed bank must be
reviewed again instead of silently dropping a new delay, gain, state or RTPC.
"""
from __future__ import annotations

import argparse
from array import array
import hashlib
import json
import math
from pathlib import Path
import sys
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/SoundPipeline"))
sys.path.insert(0, str(ROOT / "Tools/LevelPlacementExtractor"))
import wwise_audio_package as wwise
import wwise_vorbis_to_ogg as vorbis
from fmod_decode import FmodDecoder
from extract_ue3_effect_material_closure import load_package
import extract_ue3_placements as ue3

EVENT = "pc_create_dragonknight1"
EVENT_PATH = "pc_common_create." + EVENT
CARRIER = "sequence.classselect.guardianknight.intro.a12254.p0"
WORLD = ROOT / "Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.worldsequences.json"
SCENE = Path("C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages/A89UAVCCT9OJA5NNNXAXJU9UNJXOX7E.upk")
# Inspected with wwiser v20260808: v134 Play/Layer/Sounds have no delay, pitch,
# random gain, RTPC or state. The parent mixer carries the bus routing only.
REVIEWED = {
    181606760: (3, "0304c8c9a73f00011090010000000460170090"),
    1067960776: (9, "00000000000000cc07023c00000003080004010000000000000000020000005f9eba1bb38d0f3f0000000000"),
    465215071: (2, "0100040000c97918122be201000100000000000000c8c9a73f0001070000c84200000000010000000000000000"),
    1057983923: (2, "0100040000ad5e6806b6c802000000000000000000c8c9a73f000000000000010000000000000000"),
    1006766028: (7, "000000de1577610000000000000003080000010000000000000000160000004e1da605cd32d506b0a7ae07b6d1310bbe2c390b07302d12ddd4fb17bef11b189a277c1848113a193f6d811df7115c2135c9bf219d9b6524b3aa2126f032522919038233970aee37ac1ac23acdef8f3d32d1913dc8c9a73f"),
}


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n", encoding="utf-8")


def unwrap(value):
    if isinstance(value, dict):
        if "type" in value and "value" in value:
            return unwrap(value["value"])
        if "properties" in value:
            return unwrap(value["properties"])
        return {key: unwrap(child) for key, child in value.items()}
    if isinstance(value, list):
        return [unwrap(child) for child in value]
    return value


def source_cues(path):
    package = load_package(path, ue3.LOSTARK_KR_AES_KEY)
    def row(ref):
        entry = package.exports[ref - 1]
        raw = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
        props, _ = ue3.parse_tagged_properties(raw, package.names, package.summary.version)
        return unwrap(props)
    phases = {}
    for phase, ref in (("intro", 703), ("loop", 702)):
        data = next(link["linkedvariables"][0] for link in row(ref)["variablelinks"] if link["linkdesc"] == "Data")
        cues = []
        for group in row(data)["interpgroups"]:
            for track in row(group).get("interptracks", []):
                entry = package.exports[track - 1]
                if ue3.package_ref_name(entry.class_index, package.imports, package.exports) != "interptrackakevent":
                    continue
                properties = row(track)
                if properties.get("bdisabletrack", False):
                    continue
                for cue in properties.get("akevents", []):
                    cues.append(dict(trackExportIndex0=track - 1, timeMs=cue["time"] * 1000,
                        event=ue3.package_ref_path(cue["event"], package.imports, package.exports)))
        phases[phase] = dict(matineeExportIndex0=ref - 1, cues=cues)
    expected = [dict(trackExportIndex0=3108, timeMs=0, event=EVENT_PATH)]
    if phases["intro"]["cues"] != expected or phases["loop"]["cues"]:
        raise ValueError("Guardian source sound track changed; review the Matinee before importing")
    return phases


def fade_wave(source, target):
    with wave.open(str(source), "rb") as reader:
        params = reader.getparams()
        if params.sampwidth != 2 or params.comptype != "NONE":
            raise ValueError("The FMOD decoder must produce PCM16")
        pcm = array("h", reader.readframes(params.nframes))
    if sys.byteorder != "little":
        pcm.byteswap()
    fade_frames = round(params.framerate * .4)
    for frame in range(min(fade_frames, params.nframes)):
        gain = frame / fade_frames
        for channel in range(params.nchannels):
            index = frame * params.nchannels + channel
            pcm[index] = round(pcm[index] * gain)
    peak = max(abs(value) for value in pcm)
    rms = math.sqrt(sum(value * value for value in pcm) / len(pcm))
    if not peak or not rms:
        raise ValueError("Decoded Guardian stem is silent")
    if sys.byteorder != "little":
        pcm.byteswap()
    target.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(target), "wb") as writer:
        writer.setparams(params)
        writer.writeframes(pcm.tobytes())
    return dict(channels=params.nchannels, sampleRate=params.framerate, frames=params.nframes,
        durationMs=params.nframes * 1000 // params.framerate, peakPcm16=peak,
        rmsPcm16=rms, fadeInMs=400, fadeCurve="LINEAR", rawPcmSha256=digest(source), sha256=digest(target))


def prepare(args):
    output = args.out.resolve()
    if not output.is_relative_to(ROOT / "out"):
        raise ValueError("Candidates must stay under repository out/")
    output.mkdir(parents=True, exist_ok=True)
    phases = source_cues(args.scene_package)
    baseline = json.loads(args.world.read_text(encoding="utf-8-sig"))
    carrier = next(t for t in baseline["templates"] if t["sequenceId"] == CARRIER)
    if carrier.get("soundTracks"):
        raise ValueError("Guardian carrier already has saved sound tracks; merge after review")
    packages = wwise.load_packages(wwise.find_packages(args.package_root, "SOUND_PC_COMMON"))
    objects = wwise.merged_objects(packages)
    for identity, (kind, payload) in REVIEWED.items():
        if objects.get(identity) != (kind, bytes.fromhex(payload)):
            raise ValueError(f"Reviewed Guardian HIRC node {identity} changed")
    event_id = wwise.fnv1_32(EVENT)
    plays = [action for action in wwise.event_action_ids(objects[event_id][1])
             if wwise.action_fields(objects[action][1])[0] == wwise.ACTION_PLAY]
    if plays != [181606760]:
        raise ValueError("Guardian Play action graph changed")
    media, missing = wwise.resolve_event(EVENT, objects)
    if missing or set(media) != {303593929, 107503277}:
        raise ValueError("Guardian Layer must resolve both reviewed source stems")
    rendered, tracks = [], []
    with FmodDecoder() as decoder:
        for identity in media:
            matches = [(p, p.stream_by_id(identity)) for p in packages if p.stream_by_id(identity)]
            if len(matches) != 1:
                raise ValueError(f"Guardian media {identity} has no unique source")
            package, entry = matches[0]
            raw = output / "raw" / f"{EVENT}__{identity}.wem"
            raw.parent.mkdir(parents=True, exist_ok=True)
            raw.write_bytes(package.payload(entry))
            vorbis.convert(raw, raw.with_suffix(".ogg"), vorbis.CODEBOOK_LIBRARY)
            if not vorbis.write_wav(decoder, raw.with_suffix(".ogg")):
                raise ValueError(f"Cannot decode Guardian media {identity}")
            asset = f"Sound/CharacterSelect/GuardianKnight/{EVENT}__{identity}.wav"
            candidate = output / "Resources" / asset
            info = fade_wave(raw.with_suffix(".wav"), candidate)
            track = dict(soundTrackId=f"classselect.guardianknight.source.{identity}", assetId=asset,
                startMs=0, durationMs=info["durationMs"], volume=1)
            tracks.append(track)
            rendered.append(dict(assetId=asset, candidate=str(candidate), mediaId=identity,
                package=str(package.path), packageSha256=digest(package.path), languageId=entry["language"],
                wemSha256=digest(raw), **info))
    patch = dict(sequenceId=CARRIER, expectedSoundTracks=[], soundTracks=tracks)
    write(output / "sound-track-additions.json", patch)
    receipt = dict(schema="lostark.guardian-selection-sound-candidate", sourcePackage=str(args.scene_package),
        sourcePackageSha256=digest(args.scene_package), sourcePhases=phases, eventId=event_id,
        baselineWorldSha256=digest(args.world), baselineWorldRevision=baseline["revision"],
        reviewedHirc={str(key): dict(kind=value[0], payloadHex=value[1]) for key, value in REVIEWED.items()},
        sourcePlayAction=dict(delayMs=0, transitionMs=400, fadeCurve="LINEAR"),
        renderedStems=rendered, patch=patch, liveFilesChanged=False,
        validation="PCM16 decode, non-silent stems, source Matinee/Layer exact join; no audible or Client verification")
    write(output / "candidate-receipt.json", receipt)
    print(json.dumps(dict(stems=len(rendered), sequenceId=CARRIER, patch=str(output / "sound-track-additions.json"))))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, default=ROOT / "out/ClassMovieSoundRestore20260929/guardian/candidate")
    parser.add_argument("--scene-package", type=Path, default=SCENE)
    parser.add_argument("--package-root", type=Path, default=wwise.DEFAULT_PACKAGE_ROOT)
    parser.add_argument("--world", type=Path, default=WORLD)
    prepare(parser.parse_args())


if __name__ == "__main__":
    main()
