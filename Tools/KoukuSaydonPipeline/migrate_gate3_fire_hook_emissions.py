"""Fold the Gate 3 fire ring and hook waves into authored Object Motion emissions.

Sixty fire boxes and eighteen hook boxes were the only way to place many copies of
one World Object, so the arrangement lived in the Composition where the World
Object Tool cannot see or edit it. Each Motion now owns its own emission rows and
the Composition keeps one box per Object.

The fold is exact. With one box per copy the Client builds

    world = Scale x Rot x T(local) x RotY(boxYaw) x T(boxPos)

and with one box plus authored rows it builds

    world = Scale x Rot x T(local) x RotY(rowYaw) x T(rowOffset + basePos)

so a row that keeps the old yaw and takes the old position minus the base
position reproduces every sampled transform. Run with --check to compare both
forms across the whole lifetime without writing anything.
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SEQUENCES = ROOT / "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"
COMPOSITION = ROOT / "Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json"

FIRE_WORLD_IDS = [f"kakulsaydon.g1.world.{index}" for index in range(7, 13)]
HOOK_WORLD_ID = "kakulsaydon.g1.world.13"
PATTERN_IDS = ("KAKULSAYDON_G1_PATTERN_18", "KAKULSAYDON_G1_PATTERN_19")
SAMPLE_STEPS = 240


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def save(path: Path, document: dict) -> None:
    text = json.dumps(document, indent=2, ensure_ascii=False) + "\n"
    path.write_bytes(text.replace("\n", "\r\n").encode("utf-8"))


def rotate_y(vector, degrees):
    """XMMatrixRotationY on a row vector, the same basis the Client samples in."""
    angle = math.radians(degrees)
    return [math.cos(angle) * vector[0] + math.sin(angle) * vector[2], vector[1],
            -math.sin(angle) * vector[0] + math.cos(angle) * vector[2]]


def find(rows, key, value):
    return next((row for row in rows if row[key] == value), None)


def world_of(document, world_id):
    world = find(document["worlds"], "worldId", world_id)
    if world is None:
        raise SystemExit(f"World definition is missing: {world_id}")
    return world


def motion_of(sequences, world):
    instance = find(sequences["instances"], "instanceId", world["sequenceInstanceId"])
    if instance is None:
        raise SystemExit(f"Sequence instance is missing: {world['sequenceInstanceId']}")
    template = find(sequences["templates"], "sequenceId", instance["templateId"])
    if template is None:
        raise SystemExit(f"Template is missing: {instance['templateId']}")
    return instance, template


def sample_world(template, emission, placement, instance_position, time_ms):
    """Reproduce Sample_ObjectWorld's translation and visibility for one emission."""
    motion = template["objectMotion"]
    keys = template["tracks"][0]["keys"]
    if time_ms <= keys[0]["timeMs"]:
        key_offset, visible = list(keys[0]["positionOffset"]), keys[0].get("visible", True)
    elif time_ms >= keys[-1]["timeMs"]:
        key_offset, visible = list(keys[-1]["positionOffset"]), keys[-1].get("visible", True)
    else:
        right = next(index for index, key in enumerate(keys) if time_ms < key["timeMs"])
        left, other = keys[right - 1], keys[right]
        factor = (time_ms - left["timeMs"]) / (other["timeMs"] - left["timeMs"])
        if template.get("interpolation") == "SMOOTH_STEP":
            factor = factor * factor * (3 - 2 * factor)
        key_offset = [a + (b - a) * factor for a, b in zip(left["positionOffset"], other["positionOffset"])]
        visible = left.get("visible", True)
    seconds = time_ms / 1000.0
    orbit = motion["revolutionOffset"]
    spun = rotate_y(orbit, motion["revolutionDegreesPerSecond"][1] * seconds)
    local = [key_offset[axis]
             + motion["velocity"][axis] * seconds
             + 0.5 * motion["acceleration"][axis] * seconds * seconds
             + spun[axis] - orbit[axis]
             for axis in range(3)]
    if emission is not None:
        local = rotate_y(local, emission["yawDegrees"])
        local = [local[axis] + emission["positionOffset"][axis] for axis in range(3)]
    local = [local[axis] + instance_position[axis] for axis in range(3)]
    scaled = [local[axis] * placement["scale"][axis] for axis in range(3)]
    placed = rotate_y(scaled, placement["rotationDegrees"][1])
    return [placed[axis] + placement["position"][axis] for axis in range(3)], visible


def build_group(sequences, document, world_id, boxes):
    """Authored rows and the single surviving box for one World definition."""
    world = world_of(document, world_id)
    _, template = motion_of(sequences, world)
    orbit = template["objectMotion"]["revolutionOffset"]
    ordered = sorted(boxes, key=lambda box: (box["startMs"], box["placement"]["rotationDegrees"][1]))
    for box in ordered:
        placement = box["placement"]
        if placement["rotationDegrees"][0] or placement["rotationDegrees"][2]:
            raise SystemExit(f"{world_id}: only yaw rotation folds into an emission row")
        if placement["scale"] != [1, 1, 1]:
            raise SystemExit(f"{world_id}: box scale must stay [1,1,1] to fold into an emission row")
    if orbit != [0, 0, 0]:
        # Rows ride R(yaw)*orbit, so the surviving box sits on the shared orbit centre.
        centres = [[round(box["placement"]["position"][axis]
                          - rotate_y(orbit, box["placement"]["rotationDegrees"][1])[axis], 4)
                    for axis in range(3)] for box in ordered]
        if any(centre != centres[0] for centre in centres[1:]):
            raise SystemExit(f"{world_id}: boxes do not share one orbit centre; refusing to fold")
        base = [round(value, 6) for value in centres[0]]
    else:
        base = [0.0, ordered[0]["placement"]["position"][1], 942.080017]
    first_start = ordered[0]["startMs"]
    emissions = [{
        "positionOffset": [round(box["placement"]["position"][axis] - base[axis], 6) for axis in range(3)],
        "yawDegrees": box["placement"]["rotationDegrees"][1],
        "startDelayMs": box["startMs"] - first_start,
    } for box in ordered]
    if any(abs(value) > 100000 for row in emissions for value in row["positionOffset"]):
        raise SystemExit(f"{world_id}: emission offset is outside runtime bounds")
    survivor = dict(ordered[0])
    survivor["startMs"] = first_start
    survivor["durationMs"] = max(box["startMs"] + box["durationMs"] for box in ordered) - first_start
    survivor["placement"] = {"position": list(base), "rotationDegrees": [0, 0.0, 0], "scale": [1, 1, 1]}
    return template, emissions, survivor, ordered


def verify(sequences, document, world_id, template, emissions, survivor, originals):
    """Every sampled transform must be identical before and after the fold."""
    instance, _ = motion_of(sequences, world_of(document, world_id))
    instance_position = instance.get("position", [0, 0, 0])
    failures = 0
    for index, (row, original) in enumerate(zip(emissions, originals)):
        for step in range(SAMPLE_STEPS + 1):
            local_ms = original["durationMs"] * step / SAMPLE_STEPS
            old_position, old_visible = sample_world(template, None, original["placement"], instance_position, local_ms)
            new_position, new_visible = sample_world(template, row, survivor["placement"], instance_position, local_ms)
            if old_visible != new_visible or any(abs(a - b) > 1e-4 for a, b in zip(old_position, new_position)):
                failures += 1
                if failures <= 5:
                    print(f"  MISMATCH {world_id} row {index} at {local_ms:.1f} ms: {old_position} vs {new_position}")
    return failures


def migrate(check_only: bool) -> None:
    sequences = load(SEQUENCES)
    document = load(COMPOSITION)
    patterns = [row for row in document["patterns"] if row["patternId"] in PATTERN_IDS]
    if len(patterns) != len(PATTERN_IDS):
        raise SystemExit("Gate 3 fire/hook patterns are missing")
    if "emissions" in motion_of(sequences, world_of(document, HOOK_WORLD_ID))[1]["objectMotion"]:
        raise SystemExit("Already migrated; the hook motion owns authored emissions")

    groups, failures = [], 0
    for pattern in patterns:
        for world_id in [*FIRE_WORLD_IDS, HOOK_WORLD_ID]:
            boxes = [box for box in pattern["worldOccurrences"] if box["worldId"] == world_id]
            if not boxes:
                continue
            if any("placement" not in box for box in boxes):
                raise SystemExit(f"{world_id}: every box needs a placement to fold")
            template, emissions, survivor, ordered = build_group(sequences, document, world_id, boxes)
            failures += verify(sequences, document, world_id, template, emissions, survivor, ordered)
            groups.append({"pattern": pattern, "worldId": world_id, "template": template,
                           "emissions": emissions, "survivor": survivor, "originals": ordered})
            print(f"  {pattern['patternId']} / {world_id}: {len(emissions)} rows, "
                  f"box {survivor['startMs']}..{survivor['startMs'] + survivor['durationMs']} ms, verified")
    if failures:
        raise SystemExit(f"Refusing to write: {failures} sampled transforms differ")

    # One Motion is shared by both patterns, so its rows must agree before it is written once.
    by_template = {}
    for group in groups:
        existing = by_template.setdefault(group["template"]["sequenceId"], group)
        if existing["emissions"] != group["emissions"]:
            raise SystemExit(f"{group['worldId']}: the two patterns place this Object differently; refusing to fold")
    if check_only:
        print("check only; nothing written")
        return

    for group in by_template.values():
        template, emissions = group["template"], group["emissions"]
        last_delay = max(row["startDelayMs"] for row in emissions)
        if last_delay >= template["durationMs"]:
            # Rows outlive one lifetime, so the Motion keeps its own keys and gains
            # a hidden tail key that carries the last row to the end of the box.
            track = template["tracks"][0]
            tail = json.loads(json.dumps(track["keys"][-1]))
            tail["visible"] = False
            tail["timeMs"] = last_delay + template["durationMs"]
            track["keys"].append(tail)
            template["durationMs"] = tail["timeMs"]
        motion = template["objectMotion"]
        motion["emissions"] = emissions
        motion["count"] = len(emissions)
        motion["intervalMs"] = 0
        motion["spreadDegrees"] = 0

    for group in groups:
        pattern, survivor, originals = group["pattern"], group["survivor"], group["originals"]
        keep = survivor["occurrenceId"]
        row_of = {box["occurrenceId"]: index for index, box in enumerate(originals)}
        removed = set(row_of) - {keep}
        pattern["worldOccurrences"] = [survivor if box["occurrenceId"] == keep else box
                                       for box in pattern["worldOccurrences"]
                                       if box["occurrenceId"] not in removed]
        for box in pattern.get("presentationOccurrences", []):
            owner = box.get("worldOccurrenceId", "")
            if owner in row_of:
                box["worldEmissionIndex"] = row_of[owner]
                box["worldOccurrenceId"] = keep

    sequences["revision"] += 1
    document["revision"] += 1
    save(SEQUENCES, sequences)
    save(COMPOSITION, document)
    for pattern in patterns:
        print(f"  {pattern['patternId']}: {len(pattern['worldOccurrences'])} World boxes")
    print(f"worldsequences revision {sequences['revision']}, composition revision {document['revision']}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="verify the fold without writing")
    migrate(parser.parse_args().check)
