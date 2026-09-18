"""Prepare the Mario phase-2 blade/hook correction without writing live authoring files."""
from __future__ import annotations
import copy
import math
from Tools.KoukuSaydonPipeline.world_object_collider import sample_key

ACTION_PATH = "Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json"
WORLD_PATH = "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json"
PATH_TEMPLATE = "world.object.kouku.cutting_blade.state.mario_phase2_instant"
PATH_INSTANCE = PATH_TEMPLATE + ".instance"
PHASE = "KAKULSAYDON_G1_PATTERN_33"
START = [5.96, 1.30, 950.59]
DESTINATION = [-7.07, 1.32, 934.43]


def prepare(action, world_sequences):
    """Return (action, world_sequences, evidence); stable IDs scope every mutation.

    Caller owns revision increment, byte preservation, CAS installation and domain publication.
    The authored eight-emission library and all existing WORLD placements remain intact.
    """
    action, world_sequences = copy.deepcopy((action, world_sequences))
    patterns = {row["patternId"]: row for row in action["patterns"]}
    worlds = {row["worldId"]: row for row in action["worlds"]}
    instances = {row["instanceId"]: row for row in world_sequences["instances"]}
    templates = {row["sequenceId"]: row for row in world_sequences["templates"]}
    phase = patterns[PHASE]
    normal = worlds["kakulsaydon.g1.world.19"]
    instant = worlds["kakulsaydon.g1.world.38"]
    for definition in (normal, instant):
        source = templates[instances[definition["sequenceInstanceId"]]["templateId"]]
        for effect in source.get("effectTracks", []):
            if effect.get("followObject") and effect.get("resourceKind") == "V1_EFFECT" and not effect.get("bone"):
                effect["inheritObjectRotation"] = False
    # These exact occurrences belong to P33. P91 owns dolls and is unrelated.
    hook = next(row for row in phase["worldOccurrences"] if row["occurrenceId"] == PHASE + ".world.2")
    if hook["worldId"] not in ("kakulsaydon.g1.world.36", "kakulsaydon.g1.world.37"):
        raise ValueError("Mario phase-2 hook was edited to a different World; preserve it for review")
    hook["worldId"] = "kakulsaydon.g1.world.37"
    original_blade = next(row for row in phase["worldOccurrences"] if row["occurrenceId"] == PHASE + ".world.4")
    if original_blade["worldId"] != normal["worldId"]:
        raise ValueError("Mario phase-2 normal blade reference changed; preserve it for review")
    source_instance = instances[instant["sequenceInstanceId"]]
    source = templates[source_instance["templateId"]]
    motion = source["objectMotion"]
    emissions = motion.get("emissions", [])
    if len(emissions) != 8 or motion["count"] != 8 or len({row["yawDegrees"] for row in emissions}) != 1:
        raise ValueError("Instant blade source must retain its eight parallel authored emissions")
    if any(motion.get("acceleration", [0, 0, 0])) or any(motion.get("revolutionDegreesPerSecond", [0, 0, 0])):
        raise ValueError("Instant blade has edited acceleration/orbit; no implicit path replacement")
    speed = math.sqrt(sum(value * value for value in motion["velocity"]))
    delta = [b-a for a, b in zip(START, DESTINATION)]
    distance = math.sqrt(sum(value * value for value in delta))
    if not math.isfinite(speed) or speed <= 0:
        raise ValueError("Instant blade source has no positive travel speed")
    arrival_ms = math.ceil(distance / speed * 1000)
    duration_ms = source["durationMs"]
    if arrival_ms > duration_ms:
        raise ValueError("Requested blade path exceeds the saved motion lifetime at the saved speed")
    yaw = math.radians(emissions[0]["yawDegrees"])
    local_delta = [delta[0]*math.cos(yaw)-delta[2]*math.sin(yaw), delta[1],
                   delta[0]*math.sin(yaw)+delta[2]*math.cos(yaw)]
    staged = copy.deepcopy(source)
    staged["sequenceId"] = PATH_TEMPLATE
    staged["displayName"] = "마리오2페이즈_즉사칼날_보스에서아이언메이든"
    staged["interpolation"] = "LINEAR"
    staged["objectMotion"]["velocity"] = [0, 0, 0]
    # A single interpolation mode serves position and scale. Preserve constant-speed
    # translation while approximating the source smooth-step scale below 0.0001.
    # 64 ms yields 174 keys for this 11-second source, under the runtime's 256 cap.
    times = sorted({0, arrival_ms, duration_ms, *range(0, duration_ms, 64),
                    *(key["timeMs"] for track in source["tracks"] for key in track["keys"])})
    if len(times) > 256:
        raise ValueError("Mario blade curve exceeds the WorldSequence 256-key limit")
    # Resample the original scale/upright curve; translation alone is the new user-authored route.
    for track in staged["tracks"]:
        keys = []
        for time in times:
            key = copy.deepcopy(sample_key(source, track["slotId"], time))
            key["timeMs"] = time
            key["positionOffset"] = [value * min(time / arrival_ms, 1.0) for value in local_delta]
            keys.append(key)
        track["keys"] = keys
    for row in staged.get("effectTracks", []):
        row["effectTrackId"] = PATH_TEMPLATE + ".effect.attached"
        row["inheritObjectRotation"] = False
    for row in staged.get("colliderTracks", []):
        row["colliderTrackId"] = PATH_TEMPLATE + ".collider.instant_death"
        if row["behavior"] != "INSTANT_DEATH":
            raise ValueError("Instant blade collider must explicitly use INSTANT_DEATH")
    new_instance = copy.deepcopy(source_instance)
    new_instance.update(instanceId=PATH_INSTANCE, templateId=PATH_TEMPLATE, position=list(START), motionEnd="STOP", loopFullPresentation=False)
    def upsert(rows, identity, value):
        existing = next((row for row in rows if row[identity] == value[identity]), None)
        if existing is None: rows.append(value)
        elif existing != value:
            raise ValueError(f"Prepared Mario blade identity {value[identity]} already has different edits")
    upsert(world_sequences["templates"], "sequenceId", staged)
    upsert(world_sequences["instances"], "instanceId", new_instance)
    definition = next((row for row in action["worlds"] if row["sequenceInstanceId"] == PATH_INSTANCE), None)
    if definition is None:
        identity = f"kakulsaydon.g1.world.{action['nextWorldOrdinal']}"
        if identity in worlds: raise ValueError("World ordinal already used")
        action["nextWorldOrdinal"] += 1
        definition = copy.deepcopy(instant)
        definition.update(worldId=identity, displayName=staged["displayName"], sequenceInstanceId=PATH_INSTANCE)
        action["worlds"].append(definition)
    existing = next((row for row in phase["worldOccurrences"] if row["worldId"] == definition["worldId"]), None)
    if existing is None:
        ordinal = phase["nextWorldOccurrenceOrdinal"]
        identity = PHASE + f".world.{ordinal}"
        if any(row["occurrenceId"] == identity for row in phase["worldOccurrences"]): raise ValueError("World occurrence ordinal already used")
        phase["nextWorldOccurrenceOrdinal"] += 1
        phase["worldOccurrences"].append(dict(occurrenceId=identity, worldId=definition["worldId"],
            startMs=original_blade["startMs"], durationMs=duration_ms, playbackSpeed=1,
            placement=dict(position=list(START), rotationDegrees=[0, 0, 0], scale=[1, 1, 1])))
    evidence = dict(patternId=PHASE, preservedPatternId="KAKULSAYDON_G1_PATTERN_91", sourceWorldId=instant["worldId"],
        targetWorldId=definition["worldId"], start=START, destination=DESTINATION, emissionCount=len(emissions),
        sourceSpeedMps=speed, actualSpeedMps=distance/(arrival_ms*.001), arrivalMs=arrival_ms,
        lifetimeMs=duration_ms, playCount=1, transformKeyCount=len(times), scaleSampleStepMs=64,
        patternColliderWindowCount=62, tuningBasis="PROJECT_TUNED user endpoints; source speed, lifetime, spin and emission shape retained",
        sourceRevision=action["revision"], worldRevision=world_sequences["revision"])
    return action, world_sequences, evidence
