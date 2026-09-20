"""Stage original Valtan subtitles on existing cinematic WorldSequence owners."""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
from pathlib import Path
import sqlite3

ROOT = Path(__file__).resolve().parents[2]
SOURCE_PATH = Path("Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json")
SCENES = (("SCENE06A", "32", "entrance"), ("SCENE06A", "33", "trash"),
          ("SCENE04A", "19", "finale"), ("SCENE07A", "20", "gate1-entrance"),
          ("SCENE07A", "21", "gate1-entrance.black-wolf"))


def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def prepare(document, scenes, messages):
    candidate = copy.deepcopy(document)
    bindings = []
    for scene, track_id, suffix in SCENES:
        source = scenes[scene]["rows"][track_id]
        balloon = source["cls"] == "efinterptracksubtitleballoon"
        expected_class = "efinterptracksubtitleballoon" if balloon else "efinterptracksubtitle"
        if source["cls"] != expected_class:
            raise ValueError("Unexpected original subtitle track class")
        instance_id = "world.sequence.instance.valtan.source-preview." + suffix
        instance = next(x for x in candidate["instances"] if x["instanceId"] == instance_id)
        template = next(x for x in candidate["templates"] if x["sequenceId"] == instance["templateId"])
        if instance["startDelayMs"] != 0 or instance["playbackSpeed"] != 1:
            raise ValueError("Preserve an edited cinematic clock: " + instance_id)
        if balloon and not any(x["slotId"] == "actor" and x["targetKind"] == "OBJECT_RESOURCE" for x in instance["bindings"]):
            raise ValueError("Original balloon actor slot is missing: " + instance_id)
        props = source["p"]
        if props.get("bdisabletrack", False):
            continue
        for key in props["subtitleballooninfoarr" if balloon else "subtitleinfoarr"]:
            identity = key["strmsgid"]
            text = messages[identity]
            if not text or len(text.encode("utf-8")) > 4096 or any((ord(c) < 32 and c != "\n") or ord(c) == 127 or c in "<>" for c in text):
                raise ValueError("Original subtitle requires unsupported text conversion: " + identity)
            source_position = key["positiontype"] if key.get("boverride_positiontype") else props.get("positiontype", "cinematic_subtitle_position_type_normal")
            if source_position not in ("cinematic_subtitle_position_type_normal", "cinematic_subtitle_position_type_upper"):
                raise ValueError("Unknown original subtitle position")
            row = dict(subtitleTrackId="subtitle.valtan." + identity, stringId=identity, text=text,
                       position="BALLOON" if balloon else ("UPPER" if source_position.endswith("_upper") else "NORMAL"),
                       slotId="actor" if balloon else "", startMs=round(key["time"] * 1000), durationMs=round(key["duration"] * 1000))
            if row["startMs"] + row["durationMs"] > template["durationMs"]:
                raise ValueError("Original subtitle exceeds existing source cinematic owner")
            rows = template.setdefault("subtitleTracks", [])
            previous = next((x for x in rows if x["subtitleTrackId"] == row["subtitleTrackId"]), None)
            if previous is not None:
                # An authored row belongs to the editor after first import.
                continue
            rows.append(row)
            bindings.append(dict(sequenceId=template["sequenceId"], instanceId=instance_id, sourceScene=scene,
                                 sourceTrackExport=track_id, sourceTrackName=source["name"], sourceKey=key, row=row))
    if candidate != document:
        candidate["revision"] += 1
    return candidate, bindings


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--database", required=True, type=Path)
    parser.add_argument("--source-scenes", type=Path, default=ROOT / "out/FullMapRestoration20260915/ValtanSequences")
    parser.add_argument("--out", required=True, type=Path)
    args = parser.parse_args()
    connection = sqlite3.connect(args.database.resolve().as_uri() + "?mode=ro", uri=True)
    messages = dict(connection.execute("SELECT KEY, MSG FROM GameMsg WHERE KEY LIKE 'cin.37053_%'"))
    connection.close()
    scenes = {name: read(args.source_scenes / f"LV_LUT_HEARTRB_ED_{name}.json") for name, _, _ in SCENES}
    path = ROOT / SOURCE_PATH
    baseline = read(path)
    candidate, bindings = prepare(baseline, scenes, messages)
    for folder, value in (("baseline", baseline), ("candidate", candidate)):
        target = args.out / folder / SOURCE_PATH
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    receipt = dict(sourcePath=SOURCE_PATH.as_posix(), sourceHash=sha(path), baselineRevision=baseline["revision"],
                   candidateRevision=candidate["revision"], database=dict(path=str(args.database), sha256=sha(args.database)),
                   bindings=bindings, sourceScenes={name: dict(path=str(args.source_scenes / f"LV_LUT_HEARTRB_ED_{name}.json"),
                       sha256=sha(args.source_scenes / f"LV_LUT_HEARTRB_ED_{name}.json")) for name in scenes},
                   balloonPolicy="Source group62/actor25 white wolf and group63/actor26 black wolf; actor slot must be visible with a valid model. Render at actual transformed model bounds top; UI padding is not an original authored offset.",
                   productBoundary="Entrance/trash/finale already use these WorldSequence owners. Two wolves are existing MapTool source preview instances; this patch does not create an automatic gate1 intro.")
    (args.out / "subtitle-manifest.json").write_text(json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    baseline_path = args.out / "baseline" / SOURCE_PATH
    candidate_path = args.out / "candidate" / SOURCE_PATH
    files = [dict(sourcePath=SOURCE_PATH.as_posix(), baseline=str(baseline_path), candidate=str(candidate_path),
                  baselineSha256=sha(baseline_path), candidateSha256=sha(candidate_path))]
    (args.out / "candidate-files.json").write_text(json.dumps(files, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(dict(subtitleRows=len(bindings), baselineRevision=baseline["revision"], candidateRevision=candidate["revision"])))


if __name__ == "__main__":
    main()
