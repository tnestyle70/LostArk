#!/usr/bin/env python3
"""Publish the five Esther strike sounds from their summons sequences.

Each Esther's CEFSequenceSummonsProjectileFixArea carries its AkEvent actions on
the same Timer clock as its FX. The runtime NPC strike clip starts with the
cameo, so a cue starts at the event time minus the cameo start. The media are
resolved from the shipped PC_COMMON banks and deployed as
Client/Bin/Resources/Sound/Asther/<event>__<mediaId>.wav. The EpicSkill voice
has no sequence time and follows the cut-in.

  python build_esther_sound_cues.py [--dry-run]
"""
from __future__ import annotations

import argparse
import json
import re
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(REPO_ROOT / "Tools" / "EffectPipeline"))
import wwise_audio_package as wwise                   # noqa: E402
import wwise_vorbis_to_ogg as vorbis                  # noqa: E402
import build_esther_bahuntur_source_effects as seq    # noqa: E402

SOUND_CATALOG = REPO_ROOT / "Data" / "Sound" / "CharacterSoundCatalog.json"
CUE_DOCUMENT = REPO_ROOT / "Data" / "Sound" / "EstherActionSoundCues.json"
RESOURCE_ROOT = REPO_ROOT / "Client" / "Bin" / "Resources" / "Sound" / "Asther"
PACKAGE_FILTER = "SOUND_PC_COMMON"
CATALOG_OWNER = "Esther"
TIMING_BASIS = "SOURCE_SUMMONS_SEQUENCE"
LATE_TOLERANCE_MS = 500

NPC_CATALOG = REPO_ROOT / "Data" / "Actors" / "NpcCatalog.json"
ESTHERS = (
    ("silian", "NPC_59030", "533000", "SK_SwordofChampion", "Silian"),
    ("wei", "NPC_58700", "532100", "SK_Dochul", "Waye"),
    ("bahuntur", "NPC_59060", "532200", "SK_BreathOfArcturus", "Bahunturr"),
    ("ninave", "NPC_59504", "532300", "SK_Parkunas", "Ninave"),
    ("inanna", "NPC_59620", "532400", "SK_MagicShield", "Inanna"),
)


def voice_cue(archetype: str, voice: str) -> tuple[int, str]:
    """EFTable_EpicSkill pairs SkillVoice with the SkillMC cut-in; it plays when the cut-in starts."""
    npc = next(row for row in json.loads(NPC_CATALOG.read_text(encoding="utf-8"))["npcs"]
               if row["archetypeId"] == archetype)
    return npc["cutinMovie"]["delayMs"], "Esther_%s1_Attack1_Vox1_1_2d" % voice


def sequence_events(projectile: str, cameo_mesh: str) -> list[tuple[int, str]]:
    raw = seq.PROJECTILE.with_name(projectile + ".loa").read_bytes()
    events, cameo = [], None
    for time, action in seq.sequence_timeline(raw):
        body = raw[action["at"]:action["until"]]
        if action["kind"] == "SkeletalMeshFX" and re.search(rb"\x00" + cameo_mesh.encode("ascii") + rb"\x00", body):
            cameo = time
        for match in re.finditer(rb"AkEvent'[^'.]+\.([^']+)'", body):
            events.append((time, match.group(1).decode("ascii")))
    assert cameo is not None, projectile
    return sorted((max(0, round((time - cameo) * 1000)), event) for time, event in events)


def render(event: str, packages, objects, decoder, scratch: Path, dry_run: bool) -> tuple[list[str], str | None]:
    sources, unresolved = wwise.resolve_event(event, objects)
    gap = "%d Play target(s) absent from every shipped bank" % len(unresolved) if unresolved else None
    assets = []
    for media_id in sources:
        payload = next((package.payload(entry) for package in packages
                        for entry in [package.stream_by_id(media_id)] if entry is not None), None)
        if payload is None:
            gap = "media %d not in any package" % media_id
            continue
        stem = "%s__%d" % (event.lower(), media_id)
        wav = RESOURCE_ROOT / (stem + ".wav")
        if not dry_run and not wav.is_file():
            wem = scratch / (stem + ".wem")
            wem.write_bytes(payload)
            vorbis.convert(wem, wem.with_suffix(".ogg"))
            wav.parent.mkdir(parents=True, exist_ok=True)
            decoder.to_wav(wem.with_suffix(".ogg"), wav)
        assets.append("Sound/Asther/%s.wav" % stem)
    return assets, gap


def write_json(path: Path, value) -> None:
    path.write_bytes((json.dumps(value, ensure_ascii=False, indent=2) + "\n").replace("\n", "\r\n").encode("utf-8"))


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--dry-run", action="store_true", help="resolve and report only")
    args = parser.parse_args(argv)

    timelines = {name: sequence_events(projectile, mesh) for name, _, projectile, mesh, _ in ESTHERS}
    voices = {name: voice_cue(archetype, voice) for name, archetype, _, _, voice in ESTHERS}
    events = sorted({event for rows in timelines.values() for _, event in rows} | {v for _, v in voices.values()})
    packages = wwise.load_packages(wwise.find_packages(wwise.DEFAULT_PACKAGE_ROOT, PACKAGE_FILTER))
    objects = wwise.merged_objects(packages)
    decoder = None if args.dry_run else vorbis.open_wav_decoder()
    if not args.dry_run and decoder is None:
        return 2

    catalog = json.loads(SOUND_CATALOG.read_text(encoding="utf-8"))
    bucket = catalog["classes"][CATALOG_OWNER]
    empty = []
    with tempfile.TemporaryDirectory() as scratch_dir:
        for event in events:
            assets, gap = render(event, packages, objects, decoder, Path(scratch_dir), args.dry_run)
            print("%-34s %d variant(s)%s" % (event, len(assets), " -- " + gap if gap else ""))
            if assets:
                bucket[event] = assets
            else:
                empty.append(event)
    if decoder is not None:
        decoder.close()

    document = json.loads(CUE_DOCUMENT.read_text(encoding="utf-8"))
    owners = {archetype for _, archetype, _, _, _ in ESTHERS}
    cues = [cue for cue in document["cues"] if cue["ownerKind"] != "NPC_ACTION" or cue["ownerId"] not in owners]
    for name, archetype, _, _, _ in ESTHERS:
        start, event = voices[name]
        if event not in empty:
            cues.append(dict(cueId="sound.esther.%s.strike.voice.cutin-edge" % name, ownerKind="NPC_ACTION",
                             ownerId=archetype, actionId="esther.strike", catalogOwnerId=CATALOG_OWNER,
                             soundEvent=event, startMs=start, lateToleranceMs=LATE_TOLERANCE_MS, volume=1.0,
                             once=True, timingBasis="PROJECT_TUNED_EDGE"))
        for number, (start, event) in enumerate(timelines[name]):
            if event in empty:
                continue
            cues.append(dict(cueId="sound.esther.%s.strike.%02d.%s.source-sequence" % (name, number, event.lower()),
                             ownerKind="NPC_ACTION", ownerId=archetype, actionId="esther.strike",
                             catalogOwnerId=CATALOG_OWNER, soundEvent=event, startMs=start,
                             lateToleranceMs=LATE_TOLERANCE_MS, volume=1.0, once=True, timingBasis=TIMING_BASIS))
        print("%-9s voice %d, %s" % (name, voices[name][0], ", ".join("%d:%s" % row for row in timelines[name])))
    if empty:
        print("no audio: " + ", ".join(empty))
    if args.dry_run:
        return 0
    write_json(SOUND_CATALOG, catalog)
    write_json(CUE_DOCUMENT, dict(document, cues=cues))
    print("wrote %d cues, %d events" % (len(cues), len(events) - len(empty)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
