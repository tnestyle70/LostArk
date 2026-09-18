"""Rescale the four saved Showtime shots around the source actor, not world zero.

Only camera eye/lookAt and the PATTERN_ONLY selection box center change.
The source camera remains in an exclusive backup. No runtime file is written;
Publish-MapAuthoring -Scope CameraShots performs validation and publication.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import tempfile

SHOTS = tuple(f"kouku.gate3.showtime.camera.{i}" for i in range(1, 5))
REL = Path("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json")
# SCENE02B matinee_15, group 68, move track 119; UE centimetres -> client metres.
SOURCE_ACTOR = (4.459576606750488 / 100, 130.58836364746094 / 100, 94191.4921875 / 100)
SOURCE_DRAW_SCALE = 1.2000000476837158
SHOT2_FLOOR_LIFT = 0.4


def transform(vector, ratio):
    if len(vector) != 3 or not all(math.isfinite(v) for v in vector):
        raise ValueError("Camera vector must contain three finite numbers")
    return [s + ratio * (v - s) for s, v in zip(SOURCE_ACTOR, vector)]


def rewrite(text, ratio, *, floor_clearance=False):
    before = json.loads(text)
    decoder = json.JSONDecoder()
    edits = []
    counts = {}
    for shot_id in SHOTS:
        marker = '"shotId": ' + json.dumps(shot_id)
        if text.count(marker) != 1:
            raise ValueError(f"Shot must exist exactly once: {shot_id}")
        begin = text.rfind("{", 0, text.index(marker))
        shot, size = decoder.raw_decode(text[begin:])
        if shot.get("activation") != "PATTERN_ONLY" or shot.get("sequenceInstanceId"):
            raise ValueError("Do not alter an automatic or World Sequence-bound camera")
        block = text[begin:begin + size]
        count = 0

        def replace(match):
            nonlocal count
            raw = match.group(2)
            values = json.loads(raw)
            vector = transform(values, ratio)
            if floor_clearance and shot_id == SHOTS[1]:
                vector[1] += SHOT2_FLOOR_LIFT
            changed = iter(vector)
            count += 1
            # Preserve array whitespace, indentation and line endings.
            numbers = r"-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?"
            return match.group(1) + re.sub(numbers, lambda _: format(next(changed), ".17g"), raw)

        block = re.sub(r'("(?:eye|lookAt|center)"\s*:\s*)(\[[^\]]+\])', replace, block)
        expected = 3 + 2 * len(shot["cameraTrack"]["keyframes"])
        if count != expected:
            raise ValueError(f"Unexpected vector count in {shot_id}: {count}/{expected}")
        counts[shot_id] = len(shot["cameraTrack"]["keyframes"])
        edits.append((begin, begin + size, block))
    for begin, end, block in sorted(edits, reverse=True):
        text = text[:begin] + block + text[end:]
    after = json.loads(text)
    b = {s["shotId"]: s for s in before["shots"]}
    for shot in after["shots"]:
        original = b[shot["shotId"]]
        if shot["shotId"] not in SHOTS:
            assert shot == original, "Unrelated shot changed"
            continue
        # Undo the permitted fields in the comparison document; all other
        # fields, including FOV, roll, interpolation and cut timing must match.
        shot["eye"], shot["lookAt"] = original["eye"], original["lookAt"]
        shot["box"]["center"] = original["box"]["center"]
        for key, old in zip(shot["cameraTrack"]["keyframes"], original["cameraTrack"]["keyframes"]):
            key["eye"], key["lookAt"] = old["eye"], old["lookAt"]
    assert before == after, "Non-camera-vector data changed"
    header = re.compile(r'("revision"\s*:\s*)' + str(before["revision"]) + r'(?=\s*,)')
    text, changed = header.subn(lambda m: m.group(1) + str(before["revision"] + (2 if floor_clearance else 1)), text, count=1)
    assert changed == 1
    return text, counts


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--apply", action="store_true")
    args = parser.parse_args()
    root = args.root.resolve()
    path = root / REL
    original = path.read_bytes()
    if original.startswith(b"\xef\xbb\xbf"):
        raise ValueError("Unexpected BOM; inspect source encoding before editing")
    catalog = json.loads((root / "Data/Actors/BossCatalog.json").read_text(encoding="utf-8-sig"))
    boss, = [b for b in catalog["bosses"] if b["archetypeId"] == "BOSS_KAKULSAYDON_G3_SAYDON"]
    if boss.get("presentationScale") != 1 or boss.get("bodyModelPreScale") != .017:
        raise ValueError("Boss scale changed; recalculate against the original actor")
    ratio = boss["bodyModelPreScale"] / (.01 * SOURCE_DRAW_SCALE)
    backup = root / "out/ShowtimeBernContinuation20260918/camerashots.before-scale.json"
    # Re-running must never apply a second multiplication.
    if backup.exists():
        source = backup.read_bytes().decode("utf-8")
        candidate, counts = rewrite(source, ratio, floor_clearance=True)
        if original == candidate.encode("utf-8"):
            print("Already applied; no changes")
            return
        previous, _ = rewrite(source, ratio)
        if original not in (backup.read_bytes(), previous.encode("utf-8")):
            raise ValueError("Camera changed after backup; manual rebase required")
    else:
        if json.loads(original)['revision'] != 88:
            raise ValueError('One-time migration requires original revision 88 or the original backup; refusing to rescale an already edited camera')
        candidate, counts = rewrite(original.decode("utf-8"), ratio, floor_clearance=True)
    result = candidate.encode("utf-8")
    assert original.count(b"\r\n") == result.count(b"\r\n")
    print(json.dumps(dict(ratio=ratio, keys=counts, beforeSha256=hashlib.sha256(original).hexdigest(),
                         afterSha256=hashlib.sha256(result).hexdigest(), apply=args.apply)))
    if args.apply:
        backup.parent.mkdir(parents=True, exist_ok=True)
        if not backup.exists():
            with backup.open("xb") as stream:
                stream.write(original)
        if path.read_bytes() != original:
            raise ValueError("Camera source changed during preparation")
        staged = None
        try:
            with tempfile.NamedTemporaryFile(dir=path.parent, prefix=".showtime-camera-", delete=False) as stream:
                staged = Path(stream.name)
                stream.write(result)
                stream.flush()
                os.fsync(stream.fileno())
            if path.read_bytes() != original:
                raise ValueError("Camera source changed before commit")
            os.replace(staged, path)
        finally:
            if staged is not None and staged.exists():
                staged.unlink()


if __name__ == "__main__":
    main()
