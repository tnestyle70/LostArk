"""Stage original Kouku Matinee subtitles; never install live authoring data."""
from __future__ import annotations
import argparse
import copy
import hashlib
import html
import json
from pathlib import Path
import re
import sqlite3

ROOT = Path(__file__).resolve().parents[2]
SCENES = (
    ("kouku.gate1.authored.", "SCENE03A", 321, "gate1"),
    ("kouku.gate2.intro.camera.", "SCENE04A", 143, "full"),
    ("kouku.gate2.clear.camera.", "SCENE02A", 48, "full"),
    ("kouku.gate3.intro.camera.", "SCENE02A", 48, "gate3"),
    ("kouku.gate3.showtime.camera.", "SCENE02B", 16, "full"),
    ("kouku.bingo.ending.camera.", "SCENE01B", 29, "full"),
    ("kouku.bingo.intro.camera.", "SCENE07A", 15, "full"),
)
LANDMARKS = ((0., 0.), (8.752, 8.752), (19.741, 18.352),
             (22.244, 20.852), (26.919, 25.981), (33.583, 35.409), (41.488, 46.552))


def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def plain_text(source):
    value = re.sub(r"<br\s*/?>", "\n", source, flags=re.I)
    value = re.sub(r"</?FONT(?:\s+color=['\"]#[0-9a-fA-F]{6}['\"])?\s*>", "", value, flags=re.I)
    value = html.unescape(value)
    if not value or len(value.encode("utf-8")) > 4096 or any((ord(c) < 32 and c != "\n") or ord(c) == 127 or c in "<>" for c in value):
        raise ValueError("Unsupported source subtitle markup/control/length: " + source)
    return value


def clock_ms(seconds, mode):
    if mode == "gate3":
        return round(seconds * 1000) - 16710
    if mode == "gate1":
        for (a, x), (b, y) in zip(LANDMARKS, LANDMARKS[1:]):
            if a <= seconds <= b:
                return 12258 + round((x + (seconds-a)*(y-x)/(b-a))*1000)
        raise ValueError("Subtitle lies outside the installed Gate 1 source clock")
    return round(seconds * 1000)


def prepare(document, sources, messages):
    candidate = copy.deepcopy(document)
    resources = {r["resourceId"]: r for r in candidate["presentationResources"]}
    bindings, unmapped = [], []
    for prefix, scene, track_id, mode in SCENES:
        track = sources[scene]["rows"][str(track_id)]
        assert track["cls"] == "efinterptracksubtitle"
        if track["p"].get("bdisabletrack", False):
            continue
        patterns = [p for p in candidate["patterns"] if any(
            resources.get(box["resourceId"], {}).get("kind") == "CAMERA" and
            resources[box["resourceId"]]["assetId"].startswith(prefix)
            for box in p.get("presentationOccurrences", []))]
        if not patterns:
            unmapped.append(dict(scene=scene, trackId=track_id, cameraPrefix=prefix))
        for ordinal, key in enumerate(track["p"].get("subtitleinfoarr", [])):
            identity = key["strmsgid"]
            if identity not in messages:
                raise ValueError("Original GameMsg is missing: " + identity)
            position = key["positiontype"] if key.get("boverride_positiontype") else track["p"].get("positiontype", "cinematic_subtitle_position_type_normal")
            if position not in ("cinematic_subtitle_position_type_normal", "cinematic_subtitle_position_type_upper"):
                raise ValueError("Unsupported original subtitle position: " + position)
            resource_id = "subtitle.kouku." + identity
            text = plain_text(messages[identity])
            resource = dict(resourceId=resource_id, displayName="자막 / " + identity,
                            kind="SUBTITLE", assetId=identity, resourceKind="", defaultAnchorKind="MAP",
                            durationMs=round(key["duration"]*1000), subtitleText=text,
                            subtitlePosition="UPPER" if position.endswith("_upper") else "NORMAL")
            start = clock_ms(key["time"], mode)
            end = clock_ms(key["time"] + key["duration"], mode)
            for pattern in patterns:
                duration = pattern.get("durationMs", 0) or sum(s["durationMs"] for s in pattern["stages"])
                low, high = max(0, start), min(duration, end)
                if high <= low:
                    continue
                previous = resources.get(resource_id)
                if previous is not None and any(previous.get(k) != v for k, v in resource.items()):
                    raise ValueError("Preserve edited subtitle resource: " + resource_id)
                if previous is None:
                    candidate["presentationResources"].append(resource)
                    resources[resource_id] = resource
                # Never replace a saved row's user-edited start/duration.
                if any(r["resourceId"] == resource_id for r in pattern["presentationOccurrences"]):
                    continue
                number = pattern.get("nextPresentationOccurrenceOrdinal", 1)
                ids = {r["occurrenceId"] for r in pattern["presentationOccurrences"]}
                while f"{pattern['patternId']}.presentation.{number}" in ids:
                    number += 1
                row = dict(occurrenceId=f"{pattern['patternId']}.presentation.{number}",
                           resourceId=resource_id, startMs=low, durationMs=high-low, anchorKind="MAP", followBoss=False)
                pattern["presentationOccurrences"].append(row)
                pattern["nextPresentationOccurrenceOrdinal"] = number + 1
                bindings.append(dict(patternId=pattern["patternId"], sourceScene=scene,
                                     sourceTrack=track_id, sourceKey=ordinal, sourceStringId=identity,
                                     originalText=messages[identity], sourceStartSeconds=key["time"],
                                     sourceDurationSeconds=key["duration"], sourcePosition=position,
                                     clockMapping=mode, occurrence=row))
    if candidate != document:
        candidate["revision"] += 1
    return candidate, bindings, unmapped


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--database", type=Path, required=True)
    parser.add_argument("--source-scenes", type=Path, default=ROOT / "out/KoukuFireworks20260911")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    connection = sqlite3.connect(args.database.resolve().as_uri() + "?mode=ro", uri=True)
    messages = dict(connection.execute("SELECT KEY, MSG FROM GameMsg WHERE KEY LIKE 'cin.37081_%'"))
    connection.close()
    sources = {scene: read(args.source_scenes / f"LV_LUT_MIDNIGHTC_ED_{scene}.json") for _, scene, _, _ in SCENES}
    reports = []
    for relative in ("Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json", "Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json"):
        path = ROOT / relative
        baseline = read(path)
        candidate, bindings, unmapped = prepare(baseline, sources, messages)
        write(args.out / "baseline" / relative, baseline)
        write(args.out / "candidate" / relative, candidate)
        reports.append(dict(sourcePath=relative, sourceHash=sha(path), baselineRevision=baseline["revision"],
                            candidateRevision=candidate["revision"], bindings=bindings, unmapped=unmapped))
    # A complete source inventory distinguishes missing authoring joins from scenes with no subtitles.
    inventory = []
    for path in sorted(args.source_scenes.glob("*SCENE*.json")):
        document = read(path)
        tracks = [dict(exportId=k, source=r) for k, r in document["rows"].items() if r.get("cls") == "efinterptracksubtitle"]
        inventory.append(dict(path=str(path), sha256=sha(path), tracks=tracks))
    write(args.out / "subtitle-manifest.json", dict(database=dict(path=str(args.database), sha256=sha(args.database)),
                                                   documents=reports, sourceInventory=inventory, originalMessages=messages))
    print(json.dumps({d["sourcePath"]: len(d["bindings"]) for d in reports}))


if __name__ == "__main__":
    main()
