"""Match cinematic clips to existing patterns and project Map Tool roar cameras.

No gameplay timing, source World Sequence, sound or subtitle is rewritten.
Camera keys are checked against Map Tool's shared Catmull-Rom sampler at 1ms.
"""
from __future__ import annotations

import argparse
import bisect
import copy
import json
import math
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
AREA = "LV_LUT_HEARTRB_ED"
MAP = f"Data/Maps/Authoring/{AREA}/{AREA}.camerashots.json"
WORLD = f"Data/Maps/Authoring/{AREA}/{AREA}.worldsequences.json"
PRESENTATION = "Data/Valtan/Valtan.presentation.json"
CAMERA = "Data/Encounters/Valtan/ValtanCinematicCamera.json"
LIMIT = 0.002
LINKS = {
    "entrance": "VALTAN_ENTRANCE_CINEMATIC",
    "phase2": "VALTAN_ARENA_BREAK_109",
    "roar": "VALTAN_SIX_PIZZA_106",
    "trash": "VALTAN_TRASH",
    "finale": "VALTAN_GHOST_DEATH_AUDITION",
}


def load(path: Path) -> dict:
    return json.loads(path.read_bytes())


def compact(values):
    result = []
    for value in values:
        if not result or result[-1] != value:
            result.append(value)
    return result


def audit_links(root: Path) -> list[dict]:
    world = load(root / WORLD)
    presentation = load(root / PRESENTATION)
    templates = {t["sequenceId"]: t for t in world["templates"]}
    instances = {i["instanceId"]: i for i in world["instances"]}
    report = []
    for suffix, expected in LINKS.items():
        instance = instances[f"world.sequence.instance.valtan.source-preview.{suffix}"]
        template = templates[instance["templateId"]]
        clips = compact(t["clipName"] for t in sorted(
            template["animationTracks"], key=lambda t: t["startMs"])
            if t["slotId"] == "actor")
        if not clips:
            raise ValueError(f"No actor animation: {suffix}")
        candidates = []
        for pattern in presentation["patterns"]:
            occurrences = [(s["stageId"], o) for s in pattern["stages"]
                           for o in s.get("animation", {}).get("occurrences", [])]
            chain = compact(o["clip"] for _, o in occurrences)
            if any(chain[i:i + len(clips)] == clips for i in range(len(chain))):
                candidates.append({"patternId": pattern["patternId"], "occurrences": [
                    {"stageId": stage, **o} for stage, o in occurrences if o["clip"] in clips]})
        if [p["patternId"] for p in candidates] != [expected]:
            raise ValueError(f"Ambiguous/different animation match for {suffix}: {candidates}")
        sounds = template.get("soundTracks", [])
        if not sounds:
            raise ValueError(f"Missing source sound: {suffix}")
        for sound in sounds:
            if not (root / "Client/Bin/Resources" / sound["assetId"]).is_file():
                raise ValueError(f"Missing installed sound: {sound['assetId']}")
        report.append({"source": suffix, "sourceClips": clips, "matches": candidates,
                       "sourceAnimationTracks": template["animationTracks"],
                       "soundTracks": sounds, "subtitleTracks": template.get("subtitleTracks", []),
                       "effectTrackCount": len(template.get("effectTracks", [])),
                       "proofScope": "Current project animation match, not original combat trigger proof"})
    return report


def sample(track: dict, time_ms: float) -> dict:
    keys = track["keyframes"]
    times = [k["timeMs"] for k in keys]
    right = bisect.bisect_right(times, time_ms)
    if right == 0 or right == len(keys):
        return {k: copy.deepcopy(keys[0 if right == 0 else -1][k])
                for k in ("eye", "lookAt", "fovYDegrees")}
    left = right - 1
    a, b = keys[left], keys[right]
    if b.get("cutBefore"):
        return {k: copy.deepcopy(a[k]) for k in ("eye", "lookAt", "fovYDegrees")}
    u = (time_ms - a["timeMs"]) / (b["timeMs"] - a["timeMs"])
    easing = track.get("easing", "LINEAR")
    if easing == "SMOOTHSTEP":
        u = u * u * (3 - 2 * u)
    elif easing == "HOLD":
        u = 0
    elif easing != "LINEAR":
        raise ValueError(easing)
    if any("up" in k for k in keys):
        raise ValueError("This projection requires the installed roar shots without explicit up")
    def vector(field):
        if track["interpolation"] == "LINEAR":
            return [x + (y - x) * u for x, y in zip(a[field], b[field])]
        if track["interpolation"] != "CATMULL_ROM":
            raise ValueError(track["interpolation"])
        p = keys[left if left == 0 or a.get("cutBefore") else left - 1]
        d = keys[min(right + 1, len(keys) - 1)]
        if d.get("cutBefore"):
            d = b
        return [.5 * (2*y + (-x+z)*u + (2*x-5*y+4*z-w)*u*u +
                      (-x+3*y-3*z+w)*u*u*u)
                for x, y, z, w in zip(p[field], a[field], b[field], d[field])]
    return {"eye": vector("eye"), "lookAt": vector("lookAt"),
            "fovYDegrees": a["fovYDegrees"] + (b["fovYDegrees"]-a["fovYDegrees"])*u}


def error(a, b):
    return max(math.dist(a["eye"], b["eye"]), math.dist(a["lookAt"], b["lookAt"]),
               abs(a["fovYDegrees"] - b["fovYDegrees"]))


def make_cue(shots, cuts, original, start, end):
    cue = copy.deepcopy(original)
    cue.update(durationMs=end-start, interpolation="LINEAR", easing="LINEAR",
               shakeAmplitude=0, shakeDurationMs=0)
    # Map Tool samples absolute WORLD coordinates. Retaining BOSS_XZ here
    # would add the preview actor's movement to an already-authored camera.
    for field in ("trackingMode", "trackingOrigin", "transitionInMs", "transitionOutMs"):
        cue.pop(field, None)
    rows = {}
    for cut in cuts:
        shot = shots[cut["shotId"]]
        track = shot["cameraTrack"]
        first = max(start, cut["startMs"])
        last = min(end, cut["startMs"] + track["durationMs"])
        if first >= last:
            continue
        # Keep the outgoing pose until a hard cut. Do not interpolate across it.
        if last < end:
            last -= 1
        points = {first, last}
        points.update(cut["startMs"] + k["timeMs"] for k in track["keyframes"]
                      if first < cut["startMs"] + k["timeMs"] < last)
        def pose(t):
            return sample(track, t-cut["startMs"])
        def refine(a, b):
            if b-a <= 1:
                return
            linear = {"interpolation": "LINEAR", "keyframes": [
                {"timeMs": a, **pose(a)}, {"timeMs": b, **pose(b)}]}
            worst, where = max((error(pose(t), sample(linear, t)), t)
                               for t in range(a+1, b))
            if worst > LIMIT * .8:
                points.add(where)
                refine(a, where)
                refine(where, b)
        ordered = sorted(points)
        for a, b in zip(ordered, ordered[1:]):
            refine(a, b)
        for t in sorted(points):
            key = {"sceneId": cue["cueId"] + f".maptool.{t}", "timeMs": t-start, **pose(t)}
            if t == cut["startMs"] and t > start:
                key["cutBefore"] = True
            rows[t] = key
    cue["keyframes"] = [rows[t] for t in sorted(rows)]
    if not 2 <= len(rows) <= 512:
        raise ValueError(f"Cue key limit: {cue['cueId']}: {len(rows)}")
    if cue["keyframes"][0]["timeMs"] != 0 or cue["keyframes"][-1]["timeMs"] != end-start:
        raise ValueError("Incomplete camera coverage")
    maximum = 0
    for t in range(start, end+1):
        cut = next(c for c in reversed(cuts) if c["startMs"] <= t)
        wanted = sample(shots[cut["shotId"]]["cameraTrack"], t-cut["startMs"])
        maximum = max(maximum, error(wanted, sample(cue, t-start)))
    if maximum > LIMIT:
        raise ValueError(f"Map Tool projection error: {maximum}")
    return cue, maximum


def candidates(root):
    mapping = audit_links(root)
    doc = load(root / MAP)
    camera = load(root / CAMERA)
    presentation = load(root / PRESENTATION)
    shots = {s["shotId"]: s for s in doc["shots"]}
    cutscene = next(c for c in doc["cutscenes"] if c["cutsceneId"] == "editor.cutscene.valtan.roar")
    cuts = sorted(cutscene["cameraCuts"], key=lambda c: c["startMs"])
    camera_end = max(c["startMs"] + shots[c["shotId"]]["cameraTrack"]["durationMs"] for c in cuts)
    if camera_end != 5000 or cutscene["durationMs"] != 7003:
        raise ValueError("Roar source window changed; re-evaluate stage placement")
    pattern = next(p for p in presentation["patterns"] if p["patternId"] == LINKS["roar"])
    errors = []
    for stage_id, start, end in (("STEP_04", 0, 2800), ("STEP_05", 2800, camera_end)):
        stage = next(s for s in pattern["stages"] if s["stageId"] == stage_id)
        if len(stage["cameraInvocations"]) != 1:
            raise ValueError(f"Preserve edited invocation list: {stage_id}")
        invocation = stage["cameraInvocations"][0]
        index = next(i for i,c in enumerate(camera["cues"]) if c["cueId"] == invocation["cameraCueId"])
        cue, maximum = make_cue(shots, cuts, camera["cues"][index], start, end)
        camera["cues"][index] = cue
        invocation.update(startOffsetMs=0, durationPolicy="EXPLICIT", durationMs=end-start)
        errors.append({"stageId": stage_id, "sourceStartMs": start, "sourceEndMs": end,
                       "keyCount": len(cue["keyframes"]), "maximumError": maximum})
    # Event_02 keys already match the Map Tool shot. Only the old boss-relative
    # tracking and stage blends changed their effective pose at runtime.
    for cue in camera["cues"]:
        if cue.get("patternId") == LINKS["phase2"] and cue.get("stageId") in (
                "IMPACT_HOLD", "WIDE_REVEAL", "RECOVERY"):
            for field in ("trackingMode", "trackingOrigin", "transitionInMs", "transitionOutMs"):
                cue.pop(field, None)
    return {CAMERA: camera, PRESENTATION: presentation}, {"animationMatches": mapping, "cameraProjection": errors}


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument("--repository-root", type=Path, default=ROOT)
    parser.add_argument("--evidence-root", type=Path, required=True)
    parser.add_argument("--install", action="store_true")
    args = parser.parse_args()
    root = args.repository_root.resolve()
    inputs = {p: (root/p).read_bytes() for p in (CAMERA, PRESENTATION, MAP, WORLD)}
    values, report = candidates(root)
    args.evidence_root.mkdir(parents=True, exist_ok=True)
    writes = []
    for relative, doc in values.items():
        before = inputs[relative]
        after = (json.dumps(doc, ensure_ascii=False, indent=2)+"\n").encode("utf-8")
        evidence = args.evidence_root / Path(relative).name
        backup = evidence.with_suffix(evidence.suffix+".before")
        if not backup.exists():
            backup.write_bytes(before)
        evidence.with_suffix(evidence.suffix+".candidate").write_bytes(after)
        if json.loads(before) != doc:
            writes.append((root/relative, before, after))
    if args.install:
        if any((root/p).read_bytes() != content for p,content in inputs.items()):
            raise ValueError("Source changed during preparation; retry from current saved data")
        committed = []
        try:
            for path,before,after in writes:
                if path.read_bytes() != before:
                    raise ValueError(f"Concurrent save: {path}")
                temp = path.with_name(path.name+".roar-tmp")
                if temp.exists():
                    raise ValueError(f"Previous temporary output exists: {temp}")
                temp.write_bytes(after)
                os.replace(temp, path)
                committed.append((path,before,after))
        except Exception:
            for path,before,after in reversed(committed):
                if path.read_bytes() == after:
                    path.write_bytes(before)
            raise
    report.update(installed=args.install, changedFiles=[str(p) for p,_,_ in writes], visualStatus="USER_PENDING")
    (args.evidence_root/"animation-camera-report.json").write_text(
        json.dumps(report, ensure_ascii=False, indent=2)+"\n", encoding="utf-8")
    print(json.dumps({"installed": args.install, "matchedPatterns": len(report["animationMatches"]),
                      "cameras": report["cameraProjection"], "changedFiles": report["changedFiles"]}, ensure_ascii=False))


if __name__ == "__main__":
    main()
