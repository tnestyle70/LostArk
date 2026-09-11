"""Read-only authoring fragment generator. Prints JSON; never edits game data."""
from __future__ import annotations
import argparse
import copy
import json
import math
from pathlib import Path
import shlex

AREA = "LV_LUT_MIDNIGHTC_ED"
PREFIX = "world.sequence.instance.kouku.g3.fire_hook"
DISPLAY = "3관문_외곽불회전_갈고리대각선_시각테스트"
# The fires make the arena busy enough that the hook could not be judged at all,
# so a second pattern runs the identical hook passes with no fires.
DISPLAY_HOOK_ONLY = "3관문_갈고리만_확인용_불없음"
DURATION = 36000
HOOK_DURATION = 8000

# The arena edge is the beaded garland (BG_EVT_CHRISTMAS_LIGHTING01), and it is a
# true circle: 72 ground-level placements at radius 13.19 +/- 0.10 around
# (0, 942.08), 1.14 m apart. That ring, not the scattered DECO24 decor, is what
# the player reads as the rim of the circus floor. The decor fires sit at radius
# 13.94 - 19.44, i.e. outside the beads, which is why driving them looked like
# fire wandering off the map.
CENTER = (0.0, 942.08)
BOUNDARY_RADIUS = 13.19
# Outermost row stays just inside the beads. The other two rows give the fire
# wall depth, without moving it outside the floor. These are authored values,
# not recovered source-game coordinates. Tuple: radius metres, phase degrees.
RADIUS = 12.6
FIRE_RING_LAYOUT = {"d": (12.6, 0.0), "e": (11.7, 6.0), "f": (10.8, 12.0)}
# Reuse the same 60 billboards: 20 per row, staggered by six degrees.
FIRE_COUNT = 60
ORBIT_DEGREES_PER_SECOND = 24.0
HOOK_WAVES = 6
HOOK_WAVE_INTERVAL_MS = 5000
HOOK_LANE_OFFSET_MS = 400
HOOK_FIRST_START_MS = 1000

# Installed WMSH vertices, not the source export axes, define this transform.
# All three are thin in X (0.01625 m) and wide in Z: D 2.25285, E 1.82747,
# F 4.49399 m at modelPreScale 0.01. Local +X must point radially outward.
# The origin Y compensates the scaled mesh minimum onto the arena floor 1.30.
FIRE_ORIGIN_Y = {"d": 1.3, "e": 1.3167, "f": 1.3134}
FIRE_ASSETS = (
    ("MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB", "d", "x", (1.2, 1.5, 1.2)),
    ("MAP_B71A2EC9D778_BG_RAD_KOUKUSATON_DECO24E_SM_KHB", "e", "x", (1.2, 1.5, 1.2)),
    ("MAP_7AC8BB3D2FEE_BG_RAD_KOUKUSATON_DECO24F_SM_KHB", "f", "x", (0.7, 2.5, 0.7)),
)


def read_json(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def key(ms, position=(0, 0, 0), yaw=0.0, visible=True):
    a = math.radians(yaw) * 0.5
    return {"timeMs": ms, "positionOffset": list(position),
            "rotationQuaternion": [0, math.sin(a), 0, math.cos(a)],
            "scaleMultiplier": [1, 1, 1], "visible": visible}


def template(sid, name, duration, tracks, angular_y=0.0, animations=None,
             revolution_y=0.0, revolution_offset=(0, 0, 0)):
    return {"sequenceId": sid, "displayName": name, "category": "KoukuGate3",
            "durationMs": duration, "interpolation": "LINEAR", "tracks": tracks,
            "animationTracks": animations or [], "effectTracks": [],
            "objectMotion": {"velocity": [0, 0, 0], "acceleration": [0, 0, 0],
                             "angularVelocityDegrees": [0, angular_y, 0],
                             "revolutionDegreesPerSecond": [0, revolution_y, 0],
                             "revolutionOffset": list(revolution_offset), "count": 1,
                             "intervalMs": 0, "spreadDegrees": 0, "seed": 1}}


def instance(iid, tid, bindings):
    return {"instanceId": iid, "templateId": tid, "enabled": True,
            "startDelayMs": 0, "playbackSpeed": 1, "bindings": bindings,
            "anchorKind": "WORLD", "position": [0, 0, 0],
            "motionEnd": "STOP", "nextMotionId": ""}


def binding(slot, kind, target):
    return {"slotId": slot, "targetKind": kind, "targetId": target}


def fit_circle(points):
    """Least-squares circle through XZ points."""
    n = len(points)
    sx = sum(p[0] for p in points) / n
    sz = sum(p[1] for p in points) / n
    suu = svv = suv = suuu = svvv = suvv = svuu = 0.0
    for x, z in points:
        u, v = x - sx, z - sz
        suu += u * u
        svv += v * v
        suv += u * v
        suuu += u ** 3
        svvv += v ** 3
        suvv += u * v * v
        svuu += v * u * u
    det = suu * svv - suv * suv
    if abs(det) < 1e-9:
        raise ValueError("Boundary ring points are degenerate.")
    b1 = (suuu + suvv) / 2.0
    b2 = (svvv + svuu) / 2.0
    cx = (b1 * svv - b2 * suv) / det + sx
    cz = (b2 * suu - b1 * suv) / det + sz
    radii = [math.hypot(x - cx, z - cz) for x, z in points]
    return cx, cz, sum(radii) / n, radii


def measure_boundary(placements):
    """Re-derive the arena circle from the beaded garland actually on the map.

    This is the one number the whole pattern hangs on, so it is measured from
    live placement data every run instead of being trusted as a constant.
    """
    points = [(float(r[5]), float(r[7])) for r in placements
              if "CHRISTMAS_LIGHTING01_SM" in r[4] and 1.0 < float(r[6]) < 5.0
              and -45.0 < float(r[5]) < 45.0 and 900.0 < float(r[7]) < 985.0]
    if len(points) < 40:
        raise ValueError(f"Expected the beaded arena garland, found {len(points)} placements.")
    cx, cz, radius, radii = fit_circle(points)
    for _ in range(6):
        median = sorted(radii)[len(radii) // 2]
        kept = [p for p, d in zip(points, radii) if abs(d - median) < 1.6]
        if len(kept) == len(points):
            break
        points = kept
        cx, cz, radius, radii = fit_circle(points)
    if math.hypot(cx - CENTER[0], cz - CENTER[1]) > 0.5 or abs(radius - BOUNDARY_RADIUS) > 0.5:
        raise ValueError(
            f"Arena boundary moved: centre ({cx:.3f}, {cz:.3f}) radius {radius:.3f}; "
            f"re-audit CENTER and BOUNDARY_RADIUS before applying.")
    if RADIUS >= radius:
        raise ValueError("The fire wall must stand inside the beaded arena edge.")
    return cx, cz, radius, len(points)


def build(root: Path):
    ws_path = root / f"Data/Maps/Authoring/{AREA}/{AREA}.worldsequences.json"
    cp_path = root / "Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json"
    ws, cp = read_json(ws_path), read_json(cp_path)
    catalog_path = root / f"Data/Maps/Imported/{AREA}/{AREA}.mapassets"
    catalog = {r[0]: r for r in
               (shlex.split(line) for line in catalog_path.read_text(encoding="utf-8-sig").splitlines()[1:])}
    placement_path = root / f"Data/Maps/Authoring/{AREA}/{AREA}.mapplacements"
    placements = [shlex.split(line) for line in
                  placement_path.read_text(encoding="utf-8-sig").splitlines()[1:]]
    boundary = measure_boundary(placements)

    if any(p.get("displayName") in (DISPLAY, DISPLAY_HOOK_ONLY) for p in cp["patterns"]):
        raise ValueError("The plan pattern already exists. Tune its saved IDs; do not generate a duplicate.")
    if any(i["instanceId"].startswith(PREFIX) for i in ws["instances"]):
        raise ValueError("Plan sequence IDs already exist; inspect and merge by ID.")

    resources, templates, instances, worlds, cues = [], [], [], [], []
    pattern_number = cp["nextPatternOrdinal"]
    pid = f"KAKULSAYDON_G1_PATTERN_{pattern_number}"
    world_number = cp["nextWorldOrdinal"]

    def add_world(name, iid, object_id=None):
        nonlocal world_number
        wid = f"kakulsaydon.g1.world.{world_number}"
        world_number += 1
        row = {"worldId": wid, "displayName": name, "sequenceInstanceId": iid,
               "positionOffset": [0, 0, 0], "anchorKind": "NONE",
               "anchorPosition": [0, 0, 0], "companionEffectResourceId": ""}
        if object_id:
            row["objectResourceId"] = object_id
        worlds.append(row)
        return wid

    def cue(wid, start, duration, placement=None):
        row = {"occurrenceId": f"{pid}.world.{len(cues) + 1}", "worldId": wid,
               "startMs": start, "durationMs": duration, "playbackSpeed": 1}
        if placement is not None:
            row["placement"] = placement
        cues.append(row)

    # One resource per flame asset, and two motions each, so neighbouring flames
    # travel the ring in opposite directions.
    fire_worlds = []
    for asset, suffix, outward, scale in FIRE_ASSETS:
        model_id = catalog[asset][2]
        if not (root / "Client/Bin/Resources" / model_id).is_file():
            raise ValueError(f"Missing physical fire model: {model_id}")
        object_id = f"world.object.kouku.g3.outer_fire.{suffix}"
        # The pivot has to be the arena centre. The centre sits one row radius along
        # the flame's own inward axis, and revolutionOffset is (object - pivot) in
        # the flame's local frame, so it is the row radius along the outward axis.
        radius = FIRE_RING_LAYOUT[suffix][0]
        offset = (radius, 0, 0) if outward == "x" else (0, 0, radius)
        first_iid = None
        for sign, tag in ((1.0, "cw"), (-1.0, "ccw")):
            iid = PREFIX + f".extra_fire_{suffix}_{tag}"
            tid = f"sequence.kouku.g3.fire_hook.extra_fire_{suffix}_{tag}"
            first_iid = first_iid or iid
            omega = ORBIT_DEGREES_PER_SECOND * sign
            # The flame spins at the revolution rate so it keeps facing out of the
            # ring while it travels.
            templates.append(template(
                tid, f"3관문_추가불_{suffix.upper()}_{tag.upper()}_외곽공전", DURATION,
                [{"slotId": "object", "keys": [key(0), key(DURATION)]}],
                angular_y=omega, revolution_y=omega, revolution_offset=offset))
            instances.append(instance(iid, tid, [binding("object", "OBJECT_RESOURCE", object_id)]))
            fire_worlds.append(add_world(
                f"3관문_추가외곽불_{suffix.upper()}_{tag.upper()}", iid, object_id))
        # Size lives on the resource. The occurrence placement scale is applied
        # after the orbit displacement, so putting it there stretches the orbit
        # itself onto a wrong, off-centre circle.
        resources.append({"objectId": object_id, "displayName": f"3관문_외곽불_{suffix.upper()}",
                          "modelAssetId": model_id, "anchorKind": "WORLD",
                          "diffuseTextureAssetId": "", "modelPreScale": 0.01,
                          "animated": False, "scale": list(scale),
                          "sequenceInstanceId": "", "defaultMotionInstanceId": first_iid})

    for i in range(FIRE_COUNT):
        world_index = i % len(fire_worlds)
        suffix = FIRE_ASSETS[world_index // 2][1]
        radius, phase = FIRE_RING_LAYOUT[suffix]
        slot_in_row = (i // len(fire_worlds)) * 2 + world_index % 2
        degrees = slot_in_row * (360.0 / (FIRE_COUNT // len(FIRE_ASSETS))) + phase
        theta = math.radians(degrees)
        x = CENTER[0] + radius * math.cos(theta)
        z = CENTER[1] + radius * math.sin(theta)
        outward = FIRE_ASSETS[world_index // 2][2]
        # Turn the billboard so its outward axis points away from the centre,
        # which lays its flat face along the ring.
        yaw = (90.0 - degrees) if outward == "z" else (-degrees)
        cue(fire_worlds[world_index], 0, DURATION,
            {"position": [x, FIRE_ORIGIN_Y[FIRE_ASSETS[world_index // 2][1]], z],
             "rotationDegrees": [0, yaw, 0], "scale": [1, 1, 1]})

    hook = next(r for r in ws["objectResources"] if r["objectId"] == "world.object.kouku.hook")
    if not (root / "Client/Bin/Resources" / hook["modelAssetId"]).is_file():
        raise ValueError("Installed hook model is missing.")
    iid, tid = PREFIX + ".hook_diagonal", "sequence.kouku.g3.fire_hook.hook_diagonal"
    keys = [key(0, (0, 3, 0)), key(400), key(7400, (0, 0, 48)),
            key(7999, (0, 3, 48)), key(8000, (0, 3, 48), visible=False)]
    animation = {"slotId": "object", "startMs": 0, "clipName": "Hook_idle_normal_1",
                 "playbackRate": 1, "loop": True, "holdLastFrame": True,
                 "displayName": "갈고리_매달린자세_경로이동은Transform만"}
    templates.append(template(tid, "3관문_갈고리_등장대각선통과퇴장", HOOK_DURATION,
                              [{"slotId": "object", "keys": keys}], animations=[animation]))
    instances.append(instance(iid, tid, [binding("object", "OBJECT_RESOURCE", hook["objectId"])]))
    hook_world = add_world("3관문_갈고리_대각선", iid, hook["objectId"])
    unit = math.sqrt(0.5)
    # More passes over a longer window so the hook is verifiable even when the
    # frame rate is poor. Each pass crosses the arena centre diagonally.
    for wave in range(HOOK_WAVES):
        for lane in range(3):
            lateral = (lane - 1) * 4.0
            start = [CENTER[0] - 24 * unit + lateral * unit, 1.2,
                     CENTER[1] - 24 * unit - lateral * unit]
            cue(hook_world,
                HOOK_FIRST_START_MS + wave * HOOK_WAVE_INTERVAL_MS + lane * HOOK_LANE_OFFSET_MS,
                HOOK_DURATION,
                {"position": start, "rotationDegrees": [0, 45, 0], "scale": [1, 1, 1]})

    reference = read_json(root / "Data/Animation/Reference/KoukuSaydon/MN_RPCT_07.actionreference.json")
    action = next(a for a in reference["actions"] if a["sourceActionId"] == 0)
    source_stage = next(s for s in action["stages"] if s["stageId"] == "stage-003")
    source_slot = next(s for s in source_stage["slots"] if s["slotId"] == "animation-000")
    if source_slot["runtimeClip"] != "rpct00_idle_normal_1" or source_slot["playMs"] != 3000:
        raise ValueError("The audited Gate 3 idle reference changed; re-audit.")
    # The boss waits out the whole show on the 3 s idle clip, so the stage count
    # follows DURATION. A pattern shorter than its own world boxes is refused by
    # the composition validator.
    stage_count = DURATION // 3000
    if stage_count * 3000 != DURATION:
        raise ValueError("DURATION must be a whole number of 3000 ms idle stages.")

    def make_pattern(pattern_id, display, occurrences):
        stages = []
        for number in range(1, stage_count + 1):
            stages.append({"stageId": f"STAGE_{number}", "actionId": f"{pattern_id}.stage.{number}",
                           "stageKind": "ACTIVE", "durationMs": 3000,
                           "animationOccurrences": [{"occurrenceId": f"{pattern_id}.animation.{number}",
                                "profileId": "MN_RPCT_07", "sourceActionId": 0,
                                "sourceStageId": "stage-003", "sourceSlotId": "animation-000",
                                "referenceRevision": reference["referenceRevision"],
                                "runtimeClip": source_slot["runtimeClip"], "startOffsetMs": 0,
                                "sourceStartMs": 0, "playMs": 3000, "playRate": 1, "endPolicy": "EXACT"}]})
        return {"patternId": pattern_id, "actorProfileId": "MN_RPCT_05", "gateId": "GATE3",
                "targetBossPlacementId": "boss.kakulsaydon.g3.saydon", "displayName": display,
                "authoringStatus": "PRODUCT", "category": "MECHANIC", "nextStageOrdinal": stage_count + 1,
                "nextAnimationOrdinal": stage_count + 1, "nextLogicOccurrenceOrdinal": 1,
                "nextSummonOccurrenceOrdinal": 1, "nextWorldOccurrenceOrdinal": len(occurrences) + 1,
                "nextSceneProfileOccurrenceOrdinal": 1, "nextPresentationOccurrenceOrdinal": 1,
                "stages": stages, "logicOccurrences": [], "summonOccurrences": [],
                "worldOccurrences": occurrences, "sceneProfileOccurrences": [],
                "presentationOccurrences": [], "resetBossToSpawn": False}

    hook_pid = f"KAKULSAYDON_G1_PATTERN_{pattern_number + 1}"
    hook_only = []
    for row in cues:
        if row["worldId"] != hook_world:
            continue
        copied = copy.deepcopy(row)
        copied["occurrenceId"] = f"{hook_pid}.world.{len(hook_only) + 1}"
        hook_only.append(copied)
    if len(hook_only) != HOOK_WAVES * 3:
        raise ValueError(f"Expected {HOOK_WAVES * 3} hook occurrences, found {len(hook_only)}.")
    patterns = [make_pattern(pid, DISPLAY, cues),
                make_pattern(hook_pid, DISPLAY_HOOK_ONLY, hook_only)]

    return {"schema": "lostark.plan-fragments-only", "notRuntimeInput": True,
            "sourceRevisions": {"worldSequences": ws["revision"], "composition": cp["revision"]},
            "worldSequenceAdditions": {"objectResources": resources, "templates": templates, "instances": instances},
            "compositionAdditions": {"worlds": worlds, "patterns": patterns},
            "rootUpdates": {"worldSequenceRevision": ws["revision"] + 1,
                            "compositionRevision": cp["revision"] + 1,
                            "nextPatternOrdinal": pattern_number + 2, "nextWorldOrdinal": world_number,
                            "playAllPatternIds": cp["playAllPatternIds"] + [pid, hook_pid]},
            "arenaBoundary": {"centreX": boundary[0], "centreZ": boundary[1],
                              "radius": boundary[2], "beadCount": boundary[3],
                              "fireRadius": RADIUS, "fireCount": FIRE_COUNT,
                              "fireSpacingMetres": 2 * math.pi * RADIUS / FIRE_COUNT}}


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-root", type=Path, required=True)
    args = parser.parse_args()
    print(json.dumps(build(args.repository_root.resolve()), ensure_ascii=False, separators=(",", ":"), allow_nan=False))
