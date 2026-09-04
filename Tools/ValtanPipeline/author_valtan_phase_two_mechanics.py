#!/usr/bin/env python3
"""Idempotently author Server mechanics onto reviewed Valtan Phase 2 clips."""

from __future__ import annotations

import argparse
import copy
import math
import sys
from pathlib import Path
from typing import Any

sys.path.insert(0, str(Path(__file__).resolve().parent))
import promote_valtan_animation_chains as promotion


ROOT = Path(__file__).resolve().parents[2]
GAMEPLAY = ROOT / "Data/Valtan/Valtan.gameplay.json"
PRESENTATION = ROOT / "Data/Valtan/Valtan.presentation.json"
CUE_PREFIX = "cue.valtan.phase2."
REQUESTED_CUE_PREFIX = "cue.valtan.requested.20260827."
GHOST_PORTAL_CIRCUMRADIUS_M = 7.5
GHOST_PORTAL_EDGE_LENGTH_M = GHOST_PORTAL_CIRCUMRADIUS_M * math.sqrt(3.0)
WARP_PORTAL_DISTANCE_M = 16.0
WARP_PORTAL_TRAVEL_MS = 1300
# Canonical C++ storage is float32; this decimal is that stable 16 m / 1.3 s value.
WARP_PORTAL_SPEED_MPS = 12.3076925
WARP_FIRST_LEG_DELAY_MS = 300
WARP_REPEAT_LEG_DELAY_MS = 600


class AuthoringError(RuntimeError):
    pass


def indexed(rows: list[dict[str, Any]], field: str) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        identity = row.get(field)
        if not isinstance(identity, str) or not identity or identity in result:
            raise AuthoringError(f"invalid/duplicate {field}: {identity!r}")
        result[identity] = row
    return result


def pattern(document: dict[str, Any], pattern_id: str) -> dict[str, Any]:
    row = indexed(document["patterns"], "patternId").get(pattern_id)
    if row is None:
        raise AuthoringError(f"missing pattern: {pattern_id}")
    return row


def stage(row: dict[str, Any], stage_id: str) -> dict[str, Any]:
    result = indexed(row["stages"], "stageId").get(stage_id)
    if result is None:
        raise AuthoringError(f"missing stage: {row['patternId']}/{stage_id}")
    return result


def none_hit() -> dict[str, Any]:
    return {"shape": {"kind": "NONE"}}


def damage_hit(
    shape: dict[str, Any],
    offsets_ms: list[int],
    damage_profile_id: str,
    *,
    push_range_m: float = 2.0,
    push_ms: int = 150,
    knockdown: bool = True,
    down_ms: int = 1200,
) -> dict[str, Any]:
    return {
        "shape": shape,
        "schedule": {
            "kind": "EXPLICIT_OFFSETS",
            "offsetsMs": offsets_ms,
        },
        "serverDamageProfileId": damage_profile_id,
        "pushRangeM": push_range_m,
        "pushMs": push_ms,
        "knockdown": knockdown,
        "downMs": down_ms,
    }


def active_window_hit(
    shape: dict[str, Any],
    start_ms: int,
    lifetime_ms: int,
    damage_profile_id: str,
    *,
    push_range_m: float = 2.0,
    push_ms: int = 150,
    knockdown: bool = True,
    down_ms: int = 1200,
) -> dict[str, Any]:
    return {
        "shape": shape,
        "activation": {
            "kind": "ACTIVE_WINDOW",
            "startMs": start_ms,
            "lifetimeMs": lifetime_ms,
            "perTargetPolicy": "ONCE",
        },
        "serverDamageProfileId": damage_profile_id,
        "pushRangeM": push_range_m,
        "pushMs": push_ms,
        "knockdown": knockdown,
        "downMs": down_ms,
    }


def local_transform(
    yaw_degrees: float = 0.0,
    position: tuple[float, float, float] = (0.0, 0.0, 0.0),
) -> dict[str, Any]:
    return {
        "position": list(position),
        "rotationDegrees": [0.0, yaw_degrees, 0.0],
        "scale": [1.0, 1.0, 1.0],
    }


def cue(
    cue_id: str,
    effect_asset_id: str,
    presentation_stage: dict[str, Any],
    *,
    scale_kind: str,
    yaw_degrees: float = 0.0,
    position: tuple[float, float, float] = (0.0, 0.0, 0.0),
    follow_policy: str = "follow",
) -> dict[str, Any]:
    animation = presentation_stage["animation"]
    occurrences = animation.get("occurrences", [])
    if len(occurrences) != 1:
        raise AuthoringError(
            f"Phase 2 cue requires one clip occurrence: {cue_id}"
        )
    occurrence = occurrences[0]
    scale_policy: dict[str, Any] = {"kind": scale_kind}
    if scale_kind == "GAMEPLAY_FOOTPRINT":
        scale_policy["worldScale"] = [1.5, 1.5, 1.5]
    return {
        "cueId": cue_id,
        "scalePolicy": scale_policy,
        "occurrenceId": f"{cue_id}.occurrence.01",
        "effectAssetId": effect_asset_id,
        "clipOccurrenceId": occurrence["clipOccurrenceId"],
        "sourceStartMs": occurrence["sourceStartMs"],
        "sourceEndMs": None,
        "anchorSlotId": "root",
        "followPolicy": follow_policy,
        "stopPolicy": "natural",
        "repeatPolicy": "once",
        "localTransform": local_transform(yaw_degrees, position),
        "mappingBasis": "PROJECT_AUTHORED",
    }


def replace_phase_two_cues(
    presentation_stage: dict[str, Any], authored: list[dict[str, Any]]
) -> None:
    presentation_stage["effectCues"] = [
        row
        for row in presentation_stage["effectCues"]
        if not str(row.get("cueId", "")).startswith(
            (CUE_PREFIX, REQUESTED_CUE_PREFIX)
        )
    ] + authored


def set_tracking(row: dict[str, Any], *, random_lock: bool = False) -> None:
    row["targetPolicy"] = (
        "LOCK_RANDOM_ALIVE_ON_START" if random_lock else "NEAREST_EACH_TICK"
    )
    row["aimPolicy"] = (
        "LOCK_FACING_ON_START" if random_lock else "TRACK_TARGET_EACH_TICK"
    )


def audition_only_eligibility() -> dict[str, Any]:
    return {
        "armorRequirement": "ANY",
        "phaseRequirement": "ANY",
        "minimumGameplayPhase": 1,
        "maximumGameplayPhase": 3,
        "minimumHealthBarInclusive": 0,
        "maximumHealthBarInclusive": 0,
        "minimumRangeM": 0.0,
        "maximumRangeM": 1.0,
        "cooldownPolicy": "DERIVED_SOURCE_ACTION",
        "selectionCooldownMs": None,
        "cooldownGroupId": None,
        "repeatPolicy": {
            "kind": "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE",
            "limit": 0,
        },
    }


def replace_pattern_after(
    rows: list[dict[str, Any]], anchor_id: str, replacement: dict[str, Any]
) -> None:
    replacement_id = replacement["patternId"]
    filtered = [row for row in rows if row["patternId"] != replacement_id]
    anchor_index = next(
        index for index, row in enumerate(filtered)
        if row["patternId"] == anchor_id
    )
    filtered.insert(anchor_index + 1, replacement)
    rows[:] = filtered


def author_existing_patterns(
    gameplay: dict[str, Any], presentation: dict[str, Any]
) -> None:
    gameplay_by_id = indexed(gameplay["patterns"], "patternId")
    presentation_by_id = indexed(presentation["patterns"], "patternId")

    dash = gameplay_by_id["VALTAN_DASH_CHARGE"]
    dash["targetPolicy"] = "LOCK_NEAREST_ON_START"
    dash["aimPolicy"] = "LOCK_FACING_ON_START"
    dash_presentation = presentation_by_id["VALTAN_DASH_CHARGE"]
    saved_dash_groggy = gameplay_by_id.get("VALTAN_DASH_CHARGE_GROGGY")
    saved_dash_groggy_p = presentation_by_id.get("VALTAN_DASH_CHARGE_GROGGY")
    dash_groggy_stage = copy.deepcopy(
        stage(saved_dash_groggy, "GROGGY")
        if saved_dash_groggy is not None
        else stage(dash, "GROGGY")
    )
    dash_groggy_p_stage = copy.deepcopy(
        stage(saved_dash_groggy_p, "GROGGY")
        if saved_dash_groggy_p is not None
        else stage(dash_presentation, "GROGGY")
    )
    dash_charge = stage(dash, "CHARGE")
    dash_charge["branches"] = [
        {
            "outcome": "WALL_CONTACT",
            "nextActionId": dash_groggy_stage["actionId"],
        },
        {
            "outcome": "TIMEOUT",
            "nextActionId": dash_groggy_stage["actionId"],
        },
    ]
    dash_charge["defaultNextActionId"] = dash_groggy_stage["actionId"]
    dash_groggy_stage["stageId"] = "GROGGY"
    dash_groggy_stage["stageKind"] = "GROGGY"
    dash_groggy_stage["defaultNextActionId"] = None
    dash_groggy_stage["partDamagePolicy"] = "DESTROY_FIRST_ELIGIBLE"
    dash_groggy_stage["events"] = [
        {
            "eventId": "event.valtan.dash-charge.groggy.enter",
            "trigger": "ENTER",
            "kind": "SET_BOSS_FLAG",
            "flagId": "boss.flag.groggy",
            "enabled": True,
        },
        {
            "eventId": "event.valtan.dash-charge.groggy.exit",
            "trigger": "EXIT",
            "kind": "SET_BOSS_FLAG",
            "flagId": "boss.flag.groggy",
            "enabled": False,
        },
    ]
    dash_groggy_stage["branches"] = [
        {
            "outcome": "PART_DESTROYED",
            "nextActionId": None,
            "nextPatternId": "VALTAN_PART_BREAK",
        },
        {"outcome": "TIMEOUT", "nextActionId": None},
    ]
    dash["reactions"] = []
    dash["sourceActionIds"] = [420604, 400430]
    dash_stage_order = ("WINDUP", "CHARGE", "GROGGY")
    dash_stages = indexed(dash["stages"], "stageId")
    dash_stages["GROGGY"] = dash_groggy_stage
    dash["stages"] = [dash_stages[stage_id] for stage_id in dash_stage_order]
    dash_presentation["presentationSources"] = [
        {
            "sourceActionId": 420604,
            "sequenceIndex": 2,
            "role": "PRIMARY",
        },
        {
            "sourceActionId": 400430,
            "sequenceIndex": 0,
            "role": "REFERENCE_400430_0",
        },
    ]
    dash_presentation_stages = indexed(
        dash_presentation["stages"], "stageId"
    )
    dash_presentation_stages["GROGGY"] = dash_groggy_p_stage
    dash_presentation["stages"] = [
        dash_presentation_stages[stage_id] for stage_id in dash_stage_order
    ]
    dash_groggy_p_stage["stageId"] = "GROGGY"
    dash_groggy_p_stage["sequenceRole"] = "GROGGY"
    dash_part_break_stage = {
        "stageId": "PART_BREAK",
        "actionId": "valtan.attack.dash-charge.part-break",
        "stageKind": "PART_BREAK",
        "durationMs": 1800,
        "defaultNextActionId": "valtan.reaction.part-break.recovery",
        "hit": none_hit(),
        "motion": None,
        "events": [],
        "branches": [],
    }
    part_break_recovery_stage = {
        "stageId": "PART_BREAK_RECOVERY",
        "actionId": "valtan.reaction.part-break.recovery",
        "stageKind": "RECOVERY",
        "durationMs": 5183,
        "defaultNextActionId": None,
        "hit": none_hit(),
        "motion": None,
        "events": [{
            "eventId": "valtan.part-break.cardinal-rocks",
            "trigger": "ENTER",
            "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
            "combatObjectArchetypeId": "combatobject.valtan.part-break.rock",
            "volleyPolicy": "BOSS_RELATIVE",
            "countPerResolvedTarget": 4,
            "layout": {
                "kind": "RADIAL_AROUND_BOSS",
                "radiusM": 4.9497475,
                "startAngleDegrees": 45.0,
                "angleStepDegrees": 90.0,
                "mappingBasis": "PROJECT_TUNED",
            },
            "spawnSchedule": {
                "kind": "INTERVAL",
                "count": 1,
                "firstOffsetMs": 0,
                "intervalMs": 0,
            },
            "arenaRandom": {"kind": "NONE"},
            "allowOverlap": False,
            "maximumTotalObjects": 4,
        }],
        "branches": [],
    }
    dash_part_break_p_stage = {
        "stageId": "PART_BREAK",
        "actionId": "valtan.attack.dash-charge.part-break",
        "sequenceRole": "PART_BREAK",
        "animation": {
            "endPolicy": "EXACT",
            "repeatCount": 1,
            "occurrences": [
                {
                    "clipOccurrenceId":
                        "valtan.attack.dash-charge.part-break.clip.01",
                    "clip": "mesh_dmg_parts_start_1",
                    "mappingBasis": "PATTERN_PR_REFERENCE",
                    "sourceStartMs": 0,
                    "playMs": 1400,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
                {
                    "clipOccurrenceId":
                        "valtan.attack.dash-charge.part-break.clip.02",
                    "clip": "mesh_dmg_parts_loop_1",
                    "mappingBasis": "PATTERN_PR_REFERENCE",
                    "sourceStartMs": 0,
                    "playMs": 400,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
            ],
        },
        "effectCues": [],
        "cameraInvocations": [],
    }
    part_break_recovery_p_stage = {
        "stageId": "PART_BREAK_RECOVERY",
        "actionId": "valtan.reaction.part-break.recovery",
        "sequenceRole": "RECOVERY",
        "animation": {
            "endPolicy": "EXACT",
            "repeatCount": 1,
            "occurrences": [
                {
                    "clipOccurrenceId":
                        "valtan.reaction.part-break.recovery.clip.01",
                    "clip": "mesh_dmg_parts_end_1",
                    "mappingBasis": "PATTERN_PR_REFERENCE",
                    "sourceStartMs": 0,
                    "playMs": 2850,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
                {
                    "clipOccurrenceId":
                        "valtan.reaction.part-break.recovery.clip.02",
                    "clip": "mesh_idle_battle_1",
                    "mappingBasis": "PATTERN_PR_REFERENCE",
                    "sourceStartMs": 0,
                    "playMs": 2333,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
            ],
        },
        "effectCues": [],
        "cameraInvocations": [],
    }
    part_break = {
        "patternId": "VALTAN_PART_BREAK",
        "displayName": "부위 파괴",
        "category": "NORMAL",
        "compatibilitySelectionWeight": 0,
        "actionId": "valtan.reaction.part-break",
        "entryActionId": dash_part_break_stage["actionId"],
        "targetPolicy": "NONE",
        "aimPolicy": "NONE",
        "eligibility": audition_only_eligibility(),
        "invulnerableWhileRunning": False,
        "sourceActionIds": [420627],
        "serverMotion": None,
        "reactions": [],
        "stages": [dash_part_break_stage, part_break_recovery_stage],
    }
    part_break_p = {
        "patternId": "VALTAN_PART_BREAK",
        "sourceSequenceIndex": 1,
        "presentationSources": [{
            "sourceActionId": 420627,
            "sequenceIndex": 1,
            "role": "PRIMARY",
        }],
        "stages": [dash_part_break_p_stage, part_break_recovery_p_stage],
    }
    gameplay["patterns"][:] = [
        row for row in gameplay["patterns"]
        if row["patternId"] != "VALTAN_DASH_CHARGE_GROGGY"
    ]
    presentation["patterns"][:] = [
        row for row in presentation["patterns"]
        if row["patternId"] != "VALTAN_DASH_CHARGE_GROGGY"
    ]
    replace_pattern_after(
        gameplay["patterns"], "VALTAN_DASH_CHARGE", part_break
    )
    replace_pattern_after(
        presentation["patterns"], "VALTAN_DASH_CHARGE", part_break_p
    )

    attack_whirlwind = gameplay_by_id["VALTAN_ATTACK_WHIRLWIND"]
    attack_whirlwind_p = presentation_by_id["VALTAN_ATTACK_WHIRLWIND"]
    attack_whirlwind["targetPolicy"] = "LOCK_NEAREST_ON_START"
    attack_whirlwind["aimPolicy"] = "LOCK_FACING_ON_START"
    stage(attack_whirlwind, "STEP_04")["events"] = [
        {
            "eventId": "event.valtan.attack-whirlwind.reaim",
            "trigger": "ENTER",
            "kind": "RETARGET_RANDOM_ALIVE",
        }
    ]
    stage(attack_whirlwind, "STEP_01")["hit"] = damage_hit(
        {"kind": "CIRCLE", "outerRadiusM": 8.0},
        [1800],
        "damage.valtan.jump-spin",
        push_range_m=3.0,
        push_ms=242,
        down_ms=2000,
    )
    stage(attack_whirlwind, "STEP_03")["hit"] = damage_hit(
        {"kind": "CIRCLE", "outerRadiusM": 10.0},
        [0, 210, 420],
        "damage.valtan.jump-spin",
        push_range_m=3.0,
        push_ms=242,
        down_ms=2000,
    )
    attack_land_p = stage(attack_whirlwind_p, "STEP_01")
    replace_phase_two_cues(
        attack_land_p,
        [
            cue(
                f"{REQUESTED_CUE_PREFIX}attack-whirlwind.composite",
                "effect.valtan.project-tuned.sequence.attack-whirlwind",
                attack_land_p,
                scale_kind="GAMEPLAY_FOOTPRINT",
            )
        ],
    )
    attack_spin_p = stage(attack_whirlwind_p, "STEP_03")
    replace_phase_two_cues(attack_spin_p, [])

    charge = gameplay_by_id["VALTAN_CHARGE"]
    charge_p = presentation_by_id["VALTAN_CHARGE"]
    charge["targetPolicy"] = "NEAREST_EACH_TICK"
    charge["aimPolicy"] = "TRACK_TARGET_EACH_TICK"
    stage(charge, "STEP_03")["hit"] = damage_hit(
        {"kind": "CONE", "angleDegrees": 90.0, "lengthM": 12.0},
        [200],
        "damage.valtan.swing",
    )
    for charge_stage_p in charge_p["stages"]:
        replace_phase_two_cues(charge_stage_p, [])
    charge_start_p = stage(charge_p, "STEP_01")
    replace_phase_two_cues(
        charge_start_p,
        [
            cue(
                f"{REQUESTED_CUE_PREFIX}charge.axe-follow",
                "effect.valtan.sequence.charge",
                charge_start_p,
                scale_kind="OWNER_RELATIVE",
            ),
        ],
    )

    charge2 = gameplay_by_id["VALTAN_CHARGE_2"]
    charge2_p = presentation_by_id["VALTAN_CHARGE_2"]
    set_tracking(charge2)
    stage(charge2, "STEP_03")["hit"] = damage_hit(
        {"kind": "CONE", "angleDegrees": 110.0, "lengthM": 14.0},
        [250, 900],
        "damage.valtan.swing",
    )
    charge2_end_p = stage(charge2_p, "STEP_03")
    replace_phase_two_cues(
        charge2_end_p,
        [
            cue(
                f"{REQUESTED_CUE_PREFIX}charge2.red-fan",
                "effect.valtan.sequence.charge2",
                charge2_end_p,
                scale_kind="GAMEPLAY_FOOTPRINT",
            )
        ],
    )

    four = gameplay_by_id["VALTAN_SEQUENCE_FOUR"]
    four_p = presentation_by_id["VALTAN_SEQUENCE_FOUR"]
    set_tracking(four)
    stage(four, "STEP_01")["hit"] = damage_hit(
        {"kind": "CONE", "angleDegrees": 110.0, "lengthM": 9.0},
        [1790, 2560, 3330],
        "damage.valtan.four-slash",
    )
    four_stage = stage(four_p, "STEP_01")
    replace_phase_two_cues(
        four_stage,
        [cue(
            "cue.valtan.phase2.four.slashes",
            "effect.valtan.project-tuned.sequence.four",
            four_stage,
            scale_kind="GAMEPLAY_FOOTPRINT",
        )],
    )

    three = gameplay_by_id["VALTAN_THREE"]
    three_p = presentation_by_id["VALTAN_THREE"]
    set_tracking(three)
    stage(three, "STEP_03")["hit"] = damage_hit(
        {"kind": "CONE", "angleDegrees": 75.0, "lengthM": 15.0},
        [500, 1300],
        "damage.valtan.ground-wave-smash",
        push_range_m=0.4,
        push_ms=97,
        down_ms=2000,
    )
    three_start_p = stage(three_p, "STEP_01")
    replace_phase_two_cues(
        three_start_p,
        [
            cue(
                f"{REQUESTED_CUE_PREFIX}three.composite",
                "effect.valtan.project-tuned.sequence.three",
                three_start_p,
                scale_kind="GAMEPLAY_FOOTPRINT",
            ),
        ],
    )
    replace_phase_two_cues(stage(three_p, "STEP_03"), [])

    warp = gameplay_by_id["VALTAN_WARP"]
    warp_p = presentation_by_id["VALTAN_WARP"]
    set_tracking(warp, random_lock=True)
    portal_p = stage(warp_p, "STEP_01")
    replace_phase_two_cues(portal_p, [])
    for leg in range(2, 10):
        stage_id = f"STEP_{leg:02d}"
        gameplay_stage = stage(warp, stage_id)
        retarget_delay_ms = (
            WARP_FIRST_LEG_DELAY_MS if leg == 2 else WARP_REPEAT_LEG_DELAY_MS
        )
        gameplay_stage["durationMs"] = retarget_delay_ms + WARP_PORTAL_TRAVEL_MS
        gameplay_stage["motion"] = {
            "kind": "PORTAL_TARGET_RUSH",
            "retargetDelayMs": retarget_delay_ms,
            "speedMps": WARP_PORTAL_SPEED_MPS,
            "distanceM": WARP_PORTAL_DISTANCE_M,
        }
        gameplay_stage["hit"] = damage_hit(
            {"kind": "BOX", "lengthM": 8.0, "halfWidthM": 2.5},
            list(range(retarget_delay_ms, gameplay_stage["durationMs"], 50)),
            "damage.valtan.portal-rush",
            push_range_m=3.0,
            push_ms=180,
            down_ms=1500,
        )
        gameplay_stage["events"] = [
            {
                "eventId": f"event.valtan.phase2.warp.leg-{leg - 1:02d}.retarget",
                "trigger": "ENTER",
                "kind": "RETARGET_RANDOM_ALIVE",
            }
        ]
        leg_p = stage(warp_p, stage_id)
        leg_p["bodyVisibility"] = {
            "hiddenFromMs": 0,
            "hiddenToMs": retarget_delay_ms,
        }
        replace_phase_two_cues(leg_p, [])
    recovery_p = stage(warp_p, "STEP_10")
    recovery_p["bodyVisibility"] = {
        "hiddenFromMs": 0,
        "hiddenToMs": 300,
    }
    replace_phase_two_cues(recovery_p, [])

    counter = gameplay_by_id["VALTAN_COUNTER"]
    counter_p = presentation_by_id["VALTAN_COUNTER"]
    set_tracking(counter)
    counter_one = stage(counter, "STEP_01")
    counter_one["stageKind"] = "WINDUP"
    counter_one["durationMs"] = 2000
    counter_two = stage(counter, "STEP_02")
    counter_two["stageKind"] = "WINDUP"
    counter_two["durationMs"] = 1800
    counter_two["events"] = [
        {
            "eventId": "event.valtan.phase2.counter.window.enter",
            "trigger": "ENTER",
            "kind": "SET_BOSS_FLAG",
            "flagId": "boss.flag.counterable",
            "enabled": True,
        },
        {
            "eventId": "event.valtan.phase2.counter.window.exit",
            "trigger": "EXIT",
            "kind": "SET_BOSS_FLAG",
            "flagId": "boss.flag.counterable",
            "enabled": False,
        },
    ]
    counter_two["branches"] = [
        {
            "outcome": "COUNTER_HIT",
            "nextActionId": None,
            "nextPatternId": "VALTAN_GROGGY_FOLLOWUP",
        },
        {
            "outcome": "TIMEOUT",
            "nextActionId": "valtan.sequence.counter.step-03",
        },
    ]
    counter_three = stage(counter, "STEP_03")
    counter_three["stageKind"] = "ACTIVE"
    counter_three["durationMs"] = 1667
    counter_three["hit"] = damage_hit(
        {"kind": "CIRCLE", "outerRadiusM": 12.0},
        [900],
        "damage.valtan.triple-counter",
        push_range_m=0.0,
        push_ms=0,
        down_ms=2000,
    )
    counter_three["defaultNextActionId"] = None
    counter_three["branches"] = [
        {"outcome": "TIMEOUT", "nextActionId": None}
    ]
    stage(counter_p, "STEP_01")["animation"] = {
        "endPolicy": "EXACT",
        "repeatCount": 1,
        "occurrences": [{
            "clipOccurrenceId": "valtan.sequence.counter.step-01.clip-01",
            "clip": "mesh_att_battle_14_01",
            "mappingBasis": "PROJECT_AUTHORED",
            "sourceStartMs": 0,
            "playMs": 2000,
            "playRate": 1.0,
            "repeatUntilStageEnd": False,
        }],
    }
    stage(counter_p, "STEP_02")["animation"] = {
        "endPolicy": "EXACT",
        "repeatCount": 1,
        "occurrences": [
            {
                "clipOccurrenceId": "valtan.sequence.counter.step-02.clip-01",
                "clip": "mesh_att_battle_14_02",
                "mappingBasis": "PROJECT_AUTHORED",
                "sourceStartMs": 0,
                "playMs": 1000,
                "playRate": 1.0,
                "repeatUntilStageEnd": False,
            },
            {
                "clipOccurrenceId": "valtan.sequence.counter.step-02.clip-02",
                "clip": "mesh_att_battle_14_02",
                "mappingBasis": "PROJECT_AUTHORED",
                "sourceStartMs": 0,
                "playMs": 800,
                "playRate": 1.0,
                "repeatUntilStageEnd": False,
            },
        ],
    }
    counter_slam_p = stage(counter_p, "STEP_03")
    counter_slam_p["animation"] = {
        "endPolicy": "EXACT",
        "repeatCount": 1,
        "occurrences": [{
            "clipOccurrenceId": "valtan.sequence.counter.step-03.clip-01",
            "clip": "mesh_att_battle_14_03",
            "mappingBasis": "PROJECT_AUTHORED",
            "sourceStartMs": 0,
            "playMs": 1667,
            "playRate": 1.0,
            "repeatUntilStageEnd": False,
        }],
    }
    replace_phase_two_cues(
        counter_slam_p,
        [
            cue(
                f"{REQUESTED_CUE_PREFIX}counter.cyan-roar-ring",
                "effect.valtan.project-tuned.sequence.counter",
                counter_slam_p,
                scale_kind="GAMEPLAY_FOOTPRINT",
            )
        ],
    )
    counter["sourceActionIds"] = [420642, 420643]
    counter["stages"] = [
        row for row in counter["stages"] if row["stageId"] != "STEP_04"
    ]
    counter_p["presentationSources"] = [
        source for source in counter_p["presentationSources"]
        if source["sourceActionId"] != 420644
    ]
    counter_p["stages"] = [
        row for row in counter_p["stages"] if row["stageId"] != "STEP_04"
    ]
    gameplay["patterns"][:] = [
        row for row in gameplay["patterns"]
        if row["patternId"] != "VALTAN_COUNTER_GROGGY"
    ]
    presentation["patterns"][:] = [
        row for row in presentation["patterns"]
        if row["patternId"] != "VALTAN_COUNTER_GROGGY"
    ]
    retired = gameplay.setdefault("retiredPatternIds", [])
    if "VALTAN_COUNTER_GROGGY" not in retired:
        retired.append("VALTAN_COUNTER_GROGGY")
    scripted = gameplay["decisionModel"]["scriptedSequence"]["patternIds"]
    scripted[:] = [
        pattern_id for pattern_id in scripted
        if pattern_id != "VALTAN_COUNTER_GROGGY"
    ]
    reaction_pattern_ids = {
        "VALTAN_DASH_CHARGE_GROGGY",
        "VALTAN_PART_BREAK",
        "VALTAN_COUNTER_GROGGY",
    }
    gameplay["decisionModel"]["manualAuditions"] = [
        row for row in gameplay["decisionModel"]["manualAuditions"]
        if row["patternId"] not in reaction_pattern_ids
    ] + [
        {
            "patternId": "VALTAN_PART_BREAK",
            "sourceChainId": "derived.part-break",
            "authoringPhase": 1,
            "admissionState": "DERIVED_SERVER_PATTERN",
        },
    ]

    terrain = gameplay_by_id["VALTAN_TERRAIN_DESTRUCTION"]
    terrain_p = presentation_by_id["VALTAN_TERRAIN_DESTRUCTION"]
    set_tracking(terrain)
    terrain_axe = stage(terrain, "STEP_10")
    terrain_axe["hit"] = damage_hit(
        {"kind": "CROSS", "lengthM": 12.0, "halfWidthM": 2.0},
        [400],
        "damage.valtan.jump-spin",
        push_range_m=3.0,
        push_ms=242,
        down_ms=2000,
    )
    terrain_axe_p = stage(terrain_p, "STEP_10")
    replace_phase_two_cues(
        terrain_axe_p,
        [
            cue(
                f"cue.valtan.phase2.terrain.four-axe.{index + 1:02d}",
                "effect.valtan.sky-axe.active",
                terrain_axe_p,
                scale_kind="GAMEPLAY_FOOTPRINT",
                yaw_degrees=float(index * 90),
            )
            for index in range(4)
        ],
    )

    pizza = gameplay_by_id["VALTAN_SIX_PIZZA_106"]
    pizza["targetPolicy"] = "LOCK_RANDOM_ALIVE_ON_START"
    pizza["aimPolicy"] = "TRACK_TARGET_EACH_TICK"
    pizza["serverMotion"] = {
        "kind": "LEAP_TO_ANCHOR",
        "anchorId": "anchor.valtan.six-pizza-106.landing",
        "landingPosition": [156.03, 22.99751, -122.06],
        "apexHeight": 10.0,
        "travelStageId": "STEP_03",
        "takeoffStartMs": 800,
        "takeoffEndMs": 1100,
        "travelStartMs": 0,
        "travelEndMs": 267,
    }
    pizza_start_p = stage(presentation_by_id["VALTAN_SIX_PIZZA_106"], "STEP_01")
    replace_phase_two_cues(
        pizza_start_p,
        [
            cue(
                f"{REQUESTED_CUE_PREFIX}six-pizza.composite",
                "effect.valtan.project-tuned.sequence.six-pizza-106",
                pizza_start_p,
                scale_kind="GAMEPLAY_FOOTPRINT",
            )
        ],
    )

    roar_charge_p = presentation_by_id["VALTAN_ROAR_CHARGE"]
    # The composite's Element delays are authored from pattern zero.
    replace_phase_two_cues(stage(roar_charge_p, "STEP_03"), [])
    roar_p = stage(roar_charge_p, "STEP_01")
    replace_phase_two_cues(
        roar_p,
        [
            cue(
                f"{REQUESTED_CUE_PREFIX}roar-charge.composite",
                "effect.valtan.sequence.roar-charge",
                roar_p,
                scale_kind="OWNER_RELATIVE",
            )
        ],
    )

    trash_p = stage(presentation_by_id["VALTAN_TRASH"], "STEP_01")
    replace_phase_two_cues(trash_p, [])

    catch_breath = gameplay_by_id["VALTAN_CATCH_BREATH"]
    catch_breath["targetPolicy"] = "LOCK_RANDOM_ALIVE_BEHIND_ON_START"
    catch_breath["aimPolicy"] = "LOCK_FACING_ON_START"
    catch_grab = stage(catch_breath, "STEP_02")
    catch_grab["defaultNextActionId"] = None
    catch_grab["branches"] = [
        {
            "outcome": "ANY_PLAYER_GRABBED",
            "nextActionId": stage(catch_breath, "STEP_03")["actionId"],
        },
        {"outcome": "TIMEOUT", "nextActionId": None},
    ]

    for pattern_id, cue_suffix, effect_asset_id in (
        (
            "VALTAN_TRASH_CATCH_SUCCESS",
            "trash-catch-success.composite",
            "effect.valtan.project-tuned.sequence.trash-catch-success",
        ),
        (
            "VALTAN_TRASH_CATCH_FAIL",
            "trash-catch-fail.composite",
            "effect.valtan.project-tuned.sequence.trash-catch-fail",
        ),
        (
            "VALTAN_TRASH_CATCH_IF",
            "trash-catch-if.composite",
            "effect.valtan.project-tuned.sequence.trash-catch-if",
        ),
        (
            "VALTAN_CATCH_BREATH",
            "catch-breath.composite",
            "effect.valtan.project-tuned.sequence.catch-breath",
        ),
    ):
        pattern_p = presentation_by_id[pattern_id]
        for presentation_stage in pattern_p["stages"]:
            replace_phase_two_cues(presentation_stage, [])
        first_stage = pattern_p["stages"][0]
        replace_phase_two_cues(
            first_stage,
            [
                cue(
                    f"{REQUESTED_CUE_PREFIX}{cue_suffix}",
                    effect_asset_id,
                    first_stage,
                    scale_kind="OWNER_RELATIVE",
                )
            ],
        )


def author_trash_capture_flow(
    gameplay: dict[str, Any], presentation: dict[str, Any], *,
    audition_pattern_ids: tuple[str, ...] = (
        "VALTAN_TRASH_CATCH_IF", "VALTAN_TRASH_CATCH_SUCCESS", "VALTAN_TRASH_CATCH_FAIL"
    ),
) -> None:
    """Keep user-authored flow slots intact; only this pattern owns its branches."""
    trash = pattern(gameplay, "VALTAN_TRASH")
    trash_p = pattern(presentation, "VALTAN_TRASH")
    action_root = trash["actionId"]
    base_g = [copy.deepcopy(stage(trash, f"STEP_{i:02d}")) for i in range(1, 9)]
    base_p = [copy.deepcopy(stage(trash_p, f"STEP_{i:02d}")) for i in range(1, 9)]
    actions = {
        key: f"{action_root}.{key.lower().replace('_', '-')}"
        for key in (
            "CATCH_COUNTER", "CATCH_PRE_IMPACT", "CATCH_SLAM",
            "EXECUTE_TAIL", "RUSH_MISS",
            "RECHARGE_WAIT_02", "RETRY_WINDUP_02", "RETRY_RUSH_02",
            "RETRY_MISS_02", "RECHARGE_WAIT_03", "RETRY_WINDUP_03",
            "RETRY_RUSH_03", "RETRY_EXHAUSTED", "GROGGY",
        )
    }
    reaim = base_g[6]["actionId"]
    rush = base_g[7]["actionId"]
    for row in base_g[5:]:
        row["events"] = []
        row["branches"] = []
        row.pop("counterProxy", None)
    base_g[5]["stageKind"] = "WINDUP"
    base_g[5]["defaultNextActionId"] = reaim
    base_g[6]["stageKind"] = "WINDUP"
    base_g[6]["defaultNextActionId"] = rush
    base_g[6]["events"] = [{
        "eventId": "event.valtan.trash.reaim",
        "trigger": "ENTER", "kind": "RETARGET_RANDOM_ALIVE",
    }]
    base_g[7]["defaultNextActionId"] = actions["RUSH_MISS"]
    base_g[7]["branches"] = [
        {"outcome": "ANY_PLAYER_GRABBED", "nextActionId": actions["CATCH_COUNTER"]},
        {"outcome": "NAVIGATION_BLOCKED", "nextActionId": actions["RUSH_MISS"]},
        {"outcome": "TIMEOUT", "nextActionId": actions["RUSH_MISS"]},
    ]

    def flag(flag_id: str, event_root: str) -> list[dict[str, Any]]:
        return [{
            "eventId": f"event.valtan.trash.{event_root}.{trigger.lower()}",
            "trigger": trigger, "kind": "SET_BOSS_FLAG",
            "flagId": flag_id, "enabled": trigger == "ENTER",
        } for trigger in ("ENTER", "EXIT")]

    def gameplay_stage(key: str, duration_ms: int, next_action: str | None,
                       *, kind: str = "ACTIVE", events: list | None = None,
                       branches: list | None = None) -> dict[str, Any]:
        return {
            "stageId": key, "actionId": actions[key], "stageKind": kind,
            "durationMs": duration_ms, "defaultNextActionId": next_action,
            "hit": none_hit(), "motion": None, "events": events or [],
            "branches": branches or [{"outcome": "TIMEOUT", "nextActionId": next_action}],
        }

    base_g[6]["events"] += flag("boss.flag.counterable", "counter-window")
    base_g[6]["branches"] = [
        {"outcome": "COUNTER_HIT", "nextActionId": actions["GROGGY"]},
        {"outcome": "TIMEOUT", "nextActionId": rush},
    ]
    base_g[6]["counterProxy"] = {
        "space": "BOSS_LOCAL", "forwardOffsetM": 1.0,
        "rightOffsetM": 0.0, "radiusM": 2.25,
    }

    def cloned_gameplay_stage(
        source: dict[str, Any], key: str, next_action: str | None
    ) -> dict[str, Any]:
        result = copy.deepcopy(source)
        result["stageId"] = key
        result["actionId"] = actions[key]
        result["defaultNextActionId"] = next_action
        return result

    def recharge_stage(key: str, next_action: str) -> dict[str, Any]:
        result = cloned_gameplay_stage(base_g[5], key, next_action)
        result["events"] = []
        result["branches"] = [
            {"outcome": "TIMEOUT", "nextActionId": next_action}
        ]
        result.pop("counterProxy", None)
        return result

    def retry_windup_stage(
        key: str, rush_action: str, attempt: int
    ) -> dict[str, Any]:
        result = cloned_gameplay_stage(base_g[6], key, rush_action)
        event_root = f"retry-{attempt:02d}"
        result["events"] = [{
            "eventId": f"event.valtan.trash.{event_root}.reaim",
            "trigger": "ENTER", "kind": "RETARGET_RANDOM_ALIVE",
        }] + flag("boss.flag.counterable", f"{event_root}.counter-window")
        result["branches"] = [
            {"outcome": "COUNTER_HIT", "nextActionId": actions["GROGGY"]},
            {"outcome": "TIMEOUT", "nextActionId": rush_action},
        ]
        result["counterProxy"] = copy.deepcopy(base_g[6]["counterProxy"])
        return result

    def retry_rush_stage(key: str, miss_action: str) -> dict[str, Any]:
        result = cloned_gameplay_stage(base_g[7], key, miss_action)
        result["events"] = []
        result["branches"] = [
            {"outcome": "ANY_PLAYER_GRABBED", "nextActionId": actions["CATCH_COUNTER"]},
            {"outcome": "NAVIGATION_BLOCKED", "nextActionId": miss_action},
            {"outcome": "TIMEOUT", "nextActionId": miss_action},
        ]
        result.pop("counterProxy", None)
        return result

    # Keep the stable catch clip slice, but the counter window now belongs to
    # the rush cast. Capturing a player never re-opens the cast counter.
    counter = gameplay_stage("CATCH_COUNTER", 200, actions["CATCH_PRE_IMPACT"])
    retry_g = [
        gameplay_stage(
            "RUSH_MISS", 1000, actions["RECHARGE_WAIT_02"], kind="RECOVERY"
        ),
        recharge_stage("RECHARGE_WAIT_02", actions["RETRY_WINDUP_02"]),
        retry_windup_stage("RETRY_WINDUP_02", actions["RETRY_RUSH_02"], 2),
        retry_rush_stage("RETRY_RUSH_02", actions["RETRY_MISS_02"]),
        gameplay_stage(
            "RETRY_MISS_02", 1000, actions["RECHARGE_WAIT_03"], kind="RECOVERY"
        ),
        recharge_stage("RECHARGE_WAIT_03", actions["RETRY_WINDUP_03"]),
        retry_windup_stage("RETRY_WINDUP_03", actions["RETRY_RUSH_03"], 3),
        retry_rush_stage("RETRY_RUSH_03", actions["RETRY_EXHAUSTED"]),
        gameplay_stage("RETRY_EXHAUSTED", 1000, None, kind="RECOVERY"),
    ]
    new_g = [
        *retry_g,
        counter,
        gameplay_stage("CATCH_PRE_IMPACT", 1300, actions["CATCH_SLAM"], branches=[
            {"outcome": "ALL_PLAYERS_GRABBED", "nextActionId": actions["EXECUTE_TAIL"]},
            {"outcome": "TIMEOUT", "nextActionId": actions["CATCH_SLAM"]},
        ]),
        gameplay_stage("CATCH_SLAM", 1500, None, events=[{
            "eventId": "event.valtan.trash.captured-slam", "trigger": "ENTER",
            "kind": "DAMAGE_GRABBED_PLAYERS",
            "damageProfileId": "damage.valtan.charge-grab-roar",
        }]),
        gameplay_stage("EXECUTE_TAIL", 1500, None, kind="RECOVERY", events=[{
            "eventId": "event.valtan.trash.execute-grabbed", "trigger": "ENTER",
            "kind": "EXECUTE_GRABBED_PLAYERS",
        }]),
        gameplay_stage("GROGGY", 4433, None, kind="GROGGY", events=[{
            "eventId": "event.valtan.trash.counter-release", "trigger": "ENTER",
            "kind": "RELEASE_GRABBED_PLAYERS", "releaseMode": "HOLD",
            "speedMps": 0.0, "durationMs": 0, "yawOffsetDegrees": 0.0,
        }] + flag("boss.flag.groggy", "groggy")),
    ]

    def presentation_stage(key: str, clips: list[tuple[str, int, int]]) -> dict[str, Any]:
        return {
            "stageId": key, "actionId": actions[key], "sequenceRole": "STEP",
            "animation": {"endPolicy": "EXACT", "repeatCount": 1, "occurrences": [{
                "clipOccurrenceId": f"{actions[key]}.clip-{index + 1:02d}",
                "clip": clip, "mappingBasis": "PROJECT_AUTHORED",
                "sourceStartMs": start, "playMs": duration, "playRate": 1.0,
                "repeatUntilStageEnd": False,
            } for index, (clip, start, duration) in enumerate(clips)]},
            "effectCues": [], "cameraInvocations": [],
        }

    catch_clip = "mesh_att_battle_13_05-1"
    new_p = [
        presentation_stage("RUSH_MISS", [("mesh_att_battle_13_05-2", 0, 1000)]),
        presentation_stage("RECHARGE_WAIT_02", [("mesh_att_battle_13_02-1", 0, 4100)]),
        presentation_stage("RETRY_WINDUP_02", [("mesh_att_battle_13_03", 0, 1000)]),
        presentation_stage("RETRY_RUSH_02", [("mesh_att_battle_13_04", 0, 667)]),
        presentation_stage("RETRY_MISS_02", [("mesh_att_battle_13_05-2", 0, 1000)]),
        presentation_stage("RECHARGE_WAIT_03", [("mesh_att_battle_13_02-1", 0, 4100)]),
        presentation_stage("RETRY_WINDUP_03", [("mesh_att_battle_13_03", 0, 1000)]),
        presentation_stage("RETRY_RUSH_03", [("mesh_att_battle_13_04", 0, 667)]),
        presentation_stage("RETRY_EXHAUSTED", [("mesh_att_battle_13_05-2", 0, 1000)]),
        presentation_stage("CATCH_COUNTER", [(catch_clip, 0, 200)]),
        presentation_stage("CATCH_PRE_IMPACT", [(catch_clip, 200, 1300)]),
        presentation_stage("CATCH_SLAM", [(catch_clip, 1500, 1500)]),
        presentation_stage("EXECUTE_TAIL", [(catch_clip, 1500, 1500)]),
        presentation_stage("GROGGY", [
            ("mesh_abn_groggy_1_start", 0, 1833),
            ("mesh_abn_groggy_1_loop", 0, 600),
            ("mesh_abn_groggy_1_end", 0, 2000),
        ]),
    ]
    trash["stages"] = base_g + new_g
    trash_p["stages"] = base_p + new_p
    if 420631 not in trash["sourceActionIds"]:
        trash["sourceActionIds"].append(420631)
    if not any(row["sourceActionId"] == 420631 for row in trash_p["presentationSources"]):
        trash_p["presentationSources"].append({
            "sourceActionId": 420631, "sequenceIndex": 1, "role": "REFERENCE",
        })

    # Audition IDs reuse the same finite graph fragments. Stable action/event
    # identities are namespaced to their owning pattern; no attachment is
    # transferred across a patternSequence boundary.
    for pattern_id, stage_ids in (
        ("VALTAN_TRASH_CATCH_IF", ["STEP_07", "STEP_08"] + [s["stageId"] for s in new_g]),
        ("VALTAN_TRASH_CATCH_SUCCESS", ["CATCH_COUNTER", "CATCH_PRE_IMPACT", "CATCH_SLAM", "EXECUTE_TAIL"]),
        ("VALTAN_TRASH_CATCH_FAIL", ["RUSH_MISS"]),
    ):
        if pattern_id not in audition_pattern_ids:
            continue
        destination = pattern(gameplay, pattern_id)
        destination_p = pattern(presentation, pattern_id)
        saved_cues = copy.deepcopy(destination_p["stages"][0]["effectCues"])
        action_map = {
            s["actionId"]: f"{destination['actionId']}.{s['stageId'].lower().replace('_', '-')}"
            for s in trash["stages"] if s["stageId"] in stage_ids
        }
        replacements = dict(action_map)
        replacements["event.valtan.trash."] = f"event.{destination['actionId']}."

        def remap(value: Any) -> Any:
            if isinstance(value, dict):
                return {key: remap(item) for key, item in value.items()}
            if isinstance(value, list):
                return [remap(item) for item in value]
            if isinstance(value, str):
                for before, after in replacements.items():
                    if value == before or value.startswith(before + ".") or before.endswith("."):
                        value = value.replace(before, after)
            return value

        destination["stages"] = [remap(copy.deepcopy(stage(trash, key))) for key in stage_ids]
        destination_p["stages"] = [remap(copy.deepcopy(stage(trash_p, key))) for key in stage_ids]
        if pattern_id.endswith("_FAIL"):
            destination["stages"][0]["defaultNextActionId"] = None
            destination["stages"][0]["branches"] = [
                {"outcome": "TIMEOUT", "nextActionId": None}
            ]
        destination["entryActionId"] = destination["stages"][0]["actionId"]
        destination["targetPolicy"] = "LOCK_RANDOM_ALIVE_ON_START" if pattern_id.endswith("_IF") else "NONE"
        destination["aimPolicy"] = "LOCK_FACING_ON_START" if pattern_id.endswith("_IF") else "NONE"
        if pattern_id.endswith("_IF") and 420631 not in destination["sourceActionIds"]:
            destination["sourceActionIds"].append(420631)
            destination_p["presentationSources"].append({
                "sourceActionId": 420631, "sequenceIndex": 1, "role": "REFERENCE",
            })
        first_p = destination_p["stages"][0]
        for saved_cue in saved_cues:
            if "clipOccurrenceId" in saved_cue:
                saved_cue["clipOccurrenceId"] = first_p["animation"]["occurrences"][0]["clipOccurrenceId"]
        first_p["effectCues"] = saved_cues


def renamed_occurrence(
    source: dict[str, Any],
    action_id: str,
    *,
    source_start_ms: int | None = None,
    play_ms: int | None = None,
    repeat: bool | None = None,
) -> dict[str, Any]:
    result = copy.deepcopy(source)
    result["clipOccurrenceId"] = f"{action_id}.clip-01"
    if source_start_ms is not None:
        result["sourceStartMs"] = source_start_ms
    if play_ms is not None:
        result["playMs"] = play_ms
    if repeat is not None:
        result["repeatUntilStageEnd"] = repeat
    result["mappingBasis"] = "PROJECT_AUTHORED"
    return result


def build_terrain_pair(
    direction: str,
    source_gameplay: dict[str, Any],
    source_presentation: dict[str, Any],
) -> tuple[dict[str, Any], dict[str, Any]]:
    suffix = "3" if direction == "3_OCLOCK" else "9"
    pattern_id = f"VALTAN_TERRAIN_DESTRUCTION_{direction}"
    action_root = f"valtan.mechanic.terrain-destruction-{suffix}"
    set_id = (
        "worldeventset.valtan.terrain-destruction-3.floor84"
        if suffix == "3"
        else "worldeventset.valtan.terrain-destruction-9.floor30"
    )
    actions = {
        "TAKEOFF": f"{action_root}.takeoff",
        "AIRBORNE": f"{action_root}.airborne",
        "LANDING": f"{action_root}.landing",
        "IMPACT": f"{action_root}.impact",
    }
    gameplay = {
        "patternId": pattern_id,
        "displayName": f"점프후지형파괴{suffix}시",
        "category": "IMPORTANT",
        "compatibilitySelectionWeight": 0,
        "actionId": action_root,
        "entryActionId": actions["TAKEOFF"],
        "targetPolicy": "NONE",
        "aimPolicy": "NONE",
        "eligibility": {
            "armorRequirement": "ANY",
            "phaseRequirement": "PHASE_TWO",
            "minimumGameplayPhase": 2,
            "maximumGameplayPhase": 3,
            "minimumHealthBarInclusive": 1,
            "maximumHealthBarInclusive": 109,
            "minimumRangeM": 0.0,
            "maximumRangeM": 100.0,
            "cooldownPolicy": "DERIVED_SOURCE_ACTION",
            "selectionCooldownMs": None,
            "cooldownGroupId": None,
            "repeatPolicy": {
                "kind": "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE",
                "limit": 1,
            },
        },
        "invulnerableWhileRunning": True,
        "sourceActionIds": copy.deepcopy(source_gameplay["sourceActionIds"]),
        "serverMotion": {
            "kind": "LEAP_TO_ANCHOR",
            "anchorId": f"anchor.valtan.terrain-destruction-{suffix}.landing",
            "landingPosition": [156.03, 22.99751, -122.06],
            "apexHeight": 10.0,
            "travelStageId": "LANDING",
            "takeoffStartMs": 800,
            "takeoffEndMs": 1100,
            "travelStartMs": 0,
            "travelEndMs": 200,
        },
        "reactions": [],
        "stages": [
            {
                "stageId": "TAKEOFF",
                "actionId": actions["TAKEOFF"],
                "stageKind": "WINDUP",
                "durationMs": 1200,
                "defaultNextActionId": actions["AIRBORNE"],
                "hit": none_hit(),
                "motion": None,
                "events": [],
                "branches": [],
            },
            {
                "stageId": "AIRBORNE",
                "actionId": actions["AIRBORNE"],
                "stageKind": "ACTIVE",
                "durationMs": 2000,
                "defaultNextActionId": actions["LANDING"],
                "hit": none_hit(),
                "motion": None,
                "events": [],
                "branches": [],
            },
            {
                "stageId": "LANDING",
                "actionId": actions["LANDING"],
                "stageKind": "ACTIVE",
                "durationMs": 200,
                "defaultNextActionId": actions["IMPACT"],
                "hit": none_hit(),
                "motion": None,
                "events": [],
                "branches": [],
            },
            {
                "stageId": "IMPACT",
                "actionId": actions["IMPACT"],
                "stageKind": "ACTIVE",
                "durationMs": 1000,
                "defaultNextActionId": None,
                "hit": damage_hit(
                    {"kind": "CIRCLE", "outerRadiusM": 8.0},
                    [0],
                    "damage.valtan.jump-spin",
                    push_range_m=3.0,
                    push_ms=242,
                    down_ms=2000,
                ),
                "motion": None,
                "events": [
                    {
                        "eventId": f"event.valtan.terrain-destruction-{suffix}.impact.world",
                        "trigger": "ENTER",
                        "kind": "TRIGGER_WORLD_EVENT_SET",
                        "worldEventSetId": set_id,
                    }
                ],
                "branches": [],
            },
        ],
    }
    source_stages = indexed(source_presentation["stages"], "stageId")
    source_takeoff = source_stages["STEP_01"]["animation"]["occurrences"][0]
    source_airborne = source_stages["STEP_02"]["animation"]["occurrences"][0]
    source_landing = source_stages["STEP_03"]["animation"]["occurrences"][0]
    presentation = {
        "patternId": pattern_id,
        "sourceSequenceIndex": source_presentation["sourceSequenceIndex"],
        "presentationSources": copy.deepcopy(
            source_presentation["presentationSources"]
        ),
        "stages": [
            {
                "stageId": "TAKEOFF",
                "actionId": actions["TAKEOFF"],
                "sequenceRole": "TAKEOFF",
                "animation": {
                    "endPolicy": "EXACT",
                    "repeatCount": 1,
                    "occurrences": [
                        renamed_occurrence(
                            source_takeoff, actions["TAKEOFF"], play_ms=1200
                        )
                    ],
                },
                "effectCues": [],
                "cameraInvocations": [],
            },
            {
                "stageId": "AIRBORNE",
                "actionId": actions["AIRBORNE"],
                "sequenceRole": "AIRBORNE",
                "animation": {
                    "endPolicy": "LOOP_TO_STAGE_END",
                    "repeatCount": 1,
                    "occurrences": [
                        renamed_occurrence(
                            source_airborne,
                            actions["AIRBORNE"],
                            play_ms=0,
                            repeat=True,
                        )
                    ],
                },
                "effectCues": [],
                "cameraInvocations": [],
            },
            {
                "stageId": "LANDING",
                "actionId": actions["LANDING"],
                "sequenceRole": "LANDING",
                "animation": {
                    "endPolicy": "EXACT",
                    "repeatCount": 1,
                    "occurrences": [
                        renamed_occurrence(
                            source_landing,
                            actions["LANDING"],
                            source_start_ms=0,
                            play_ms=200,
                            repeat=False,
                        )
                    ],
                },
                "effectCues": [],
                "cameraInvocations": [],
            },
            {
                "stageId": "IMPACT",
                "actionId": actions["IMPACT"],
                "sequenceRole": "IMPACT",
                "animation": {
                    "endPolicy": "EXACT",
                    "repeatCount": 1,
                    "occurrences": [
                        renamed_occurrence(
                            source_landing,
                            actions["IMPACT"],
                            source_start_ms=200,
                            play_ms=1000,
                            repeat=False,
                        )
                    ],
                },
                "effectCues": [],
                "cameraInvocations": [],
            },
        ],
    }
    # Both user-edited composites measure Element Start Delay from pattern zero.
    effect_owner_p = stage(presentation, "TAKEOFF")
    effect_owner_p["effectCues"] = [
        cue(
            f"{REQUESTED_CUE_PREFIX}terrain-{suffix}.semicircle",
            f"effect.valtan.project-tuned.terrain-destruction-{suffix}.semicircle",
            effect_owner_p,
            scale_kind="GAMEPLAY_FOOTPRINT",
            follow_policy="snapshot",
        )
    ]
    return gameplay, presentation


def author_terrain_pairs(
    gameplay: dict[str, Any], presentation: dict[str, Any]
) -> None:
    source_gameplay = pattern(gameplay, "VALTAN_TERRAIN_DESTRUCTION")
    source_presentation = pattern(
        presentation, "VALTAN_TERRAIN_DESTRUCTION"
    )
    authored = [
        build_terrain_pair("3_OCLOCK", source_gameplay, source_presentation),
        build_terrain_pair("9_OCLOCK", source_gameplay, source_presentation),
    ]
    authored_ids = {row[0]["patternId"] for row in authored}
    for gameplay_pattern, presentation_pattern in authored:
        for document, replacement in ((gameplay, gameplay_pattern), (presentation, presentation_pattern)):
            matching = next((index for index, row in enumerate(document["patterns"])
                             if row["patternId"] == replacement["patternId"]), None)
            if matching is None:
                document["patterns"].append(replacement)
            else:
                document["patterns"][matching] = replacement

    manual_rows = gameplay["decisionModel"]["manualAuditions"]
    manual_rows[:] = [
        row for row in manual_rows if row["patternId"] not in authored_ids
    ]
    mechanics = gameplay["decisionModel"]["mechanics"]
    mechanics[:] = [
        row for row in mechanics if row["patternId"] not in authored_ids
    ]
    mechanics.extend(
        [
            {
                "mechanicId": "mechanic.valtan-terrain-destruction-3-oclock",
                "patternId": "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
                "trigger": {"kind": "HEALTH_BAR_CROSSING", "healthBar": 84},
                "triggerOrder": 1,
                "oncePerEncounter": True,
                "failurePolicy": "ABORT_ENCOUNTER_REQUIRE_RESET",
            },
            {
                "mechanicId": "mechanic.valtan-terrain-destruction-9-oclock",
                "patternId": "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
                "trigger": {"kind": "HEALTH_BAR_CROSSING", "healthBar": 30},
                "triggerOrder": 1,
                "oncePerEncounter": True,
                "failurePolicy": "ABORT_ENCOUNTER_REQUIRE_RESET",
            },
        ]
    )


def author_clip_aligned_stage_hits(
    gameplay: dict[str, Any], presentation: dict[str, Any]
) -> None:
    """Keep source-reviewed attack contacts identical across matching clips."""
    cone_smash = {"kind": "CONE", "angleDegrees": 75.0, "lengthM": 15.0}
    for pattern_id, stage_id, offsets_ms in (
        ("VALTAN_THREE", "STEP_01", [1617]),
        ("VALTAN_THREE", "STEP_02", [963]),
        # Keep the source-reviewed 1300ms clip-template contact and the
        # pre-existing 500ms first pulse reviewed for this occurrence only.
        ("VALTAN_THREE", "STEP_03", [500, 1300]),
        ("VALTAN_SEQUENCE_TWOHAND", "STEP_02", [1000]),
        ("VALTAN_STRUGGLING", "STEP_07", [1000]),
    ):
        stage(pattern(gameplay, pattern_id), stage_id)["hit"] = damage_hit(
            copy.deepcopy(cone_smash),
            offsets_ms,
            "damage.valtan.ground-wave-smash",
            push_range_m=0.4,
            push_ms=97,
            down_ms=2000,
        )

    stomp = lambda offsets: damage_hit(
        {"kind": "CIRCLE", "outerRadiusM": 8.0},
        offsets,
        "damage.valtan.stomp",
        push_range_m=1.5,
        push_ms=150,
        down_ms=1000,
    )
    for pattern_id, stage_id in (
        ("VALTAN_BIND_SLOT", "STEP_01"),
        ("VALTAN_ROAR_CHARGE", "STEP_01"),
        ("VALTAN_TERRAIN_DESTRUCTION", "STEP_09"),
        ("VALTAN_STRUGGLING", "STEP_08"),
    ):
        stage(pattern(gameplay, pattern_id), stage_id)["hit"] = stomp([1200])

    # GROUND_ROAR contains both the stomp and roar clips in one canonical stage.
    # The one-hit-per-stage schema cannot retain two damage profiles, and splitting
    # this action would invalidate the rock owner action ID. Preserve topology and
    # use the stronger roar response for the three source-reviewed contacts.
    stage(pattern(gameplay, "VALTAN_GROUND_ROAR"), "STEP_01")["hit"] = damage_hit(
        {"kind": "CIRCLE", "outerRadiusM": 12.0},
        [600, 1300, 2700],
        "damage.valtan.ledge-roar",
        push_range_m=2.0,
        push_ms=242,
        down_ms=2000,
    )

    roar = lambda: damage_hit(
        {"kind": "CIRCLE", "outerRadiusM": 12.0},
        [900],
        "damage.valtan.ledge-roar",
        push_range_m=2.0,
        push_ms=242,
        down_ms=2000,
    )
    for pattern_id, stage_id in (
        ("VALTAN_BIND_SLOT", "RECOVERY"),
        ("VALTAN_ROAR_CHARGE", "STEP_03"),
        ("VALTAN_TERRAIN_DESTRUCTION", "STEP_11"),
        ("VALTAN_STRUGGLING", "STEP_10"),
    ):
        stage(pattern(gameplay, pattern_id), stage_id)["hit"] = roar()

    stage(pattern(gameplay, "VALTAN_ROAR_CHARGE"), "STEP_06")["hit"] = damage_hit(
        {"kind": "CONE", "angleDegrees": 90.0, "lengthM": 12.0},
        [200],
        "damage.valtan.swing",
        push_range_m=2.0,
        push_ms=150,
        down_ms=1200,
    )
    stage(pattern(gameplay, "VALTAN_CROSS"), "STEP_01")["hit"] = active_window_hit(
        {"kind": "CROSS", "lengthM": 10.0, "halfWidthM": 0.75},
        1617,
        500,
        "damage.valtan.earthquake-smash",
        push_range_m=2.0,
        push_ms=150,
        down_ms=1200,
    )
    # STEP_01/02 only play the pre-contact slice of mesh_att_battle_4_01.
    # STEP_03 owns the complete clip and its published root-motion curve, so its
    # source HIT contacts can follow the authoritative moving boss pose.
    stage(pattern(gameplay, "VALTAN_SEQUENCE_RUSH"), "STEP_03")["hit"] = damage_hit(
        {"kind": "BOX", "lengthM": 6.0, "halfWidthM": 2.5},
        [2450, 2650, 2850, 3050, 3250, 4600],
        "damage.valtan.dash-charge",
        push_range_m=2.0,
        push_ms=150,
        down_ms=1000,
    )

    stage(pattern(gameplay, "VALTAN_SEQUENCE_FOUR"), "STEP_01")["hit"][
        "schedule"
    ] = {"kind": "EXPLICIT_OFFSETS", "offsetsMs": [1233, 2233, 3233, 4200]}
    stage(pattern(gameplay, "VALTAN_STRUGGLING"), "STEP_04")["hit"] = stomp(
        [1233, 2233, 3233, 4200]
    )
    stage(pattern(gameplay, "VALTAN_STRUGGLING"), "STEP_06")["hit"] = damage_hit(
        {"kind": "CONE", "angleDegrees": 90.0, "lengthM": 10.0},
        [200, 400],
        "damage.valtan.down-smash",
        push_range_m=2.0,
        push_ms=150,
        down_ms=1200,
    )

    for stage_id, offsets_ms in (
        ("STEP_02", [0, 210, 420]),
        ("STEP_03", [0, 350, 700, 1050]),
    ):
        stage(pattern(gameplay, "VALTAN_SEQUENCE_WHIRLWIND"), stage_id)[
            "hit"
        ] = damage_hit(
            {"kind": "CIRCLE", "outerRadiusM": 10.0},
            offsets_ms,
            "damage.valtan.jump-spin",
            push_range_m=3.0,
            push_ms=242,
            down_ms=2000,
        )

    sequence_whirlwind_p = pattern(presentation, "VALTAN_SEQUENCE_WHIRLWIND")
    for stage_id in ("STEP_02", "STEP_03"):
        presentation_stage = stage(sequence_whirlwind_p, stage_id)
        replace_phase_two_cues(
            presentation_stage,
            [cue(
                f"{CUE_PREFIX}sequence-whirlwind.{stage_id.lower()}.active",
                "effect.valtan.pattern.420633.active",
                presentation_stage,
                scale_kind="GAMEPLAY_FOOTPRINT",
            )],
        )

    # The complete rush clip reaches its first reviewed contact at raw 2450ms.
    # Reuse the Product dash-charge active carrier from that source point; the
    # two 1500ms prep slices remain intentionally free of hit/effect rows.
    rush_presentation_stage = stage(
        pattern(presentation, "VALTAN_SEQUENCE_RUSH"), "STEP_03"
    )
    rush_effect = cue(
        f"{CUE_PREFIX}sequence-rush.step-03.active",
        "effect.valtan.project-tuned.dash-charge.active-shield",
        rush_presentation_stage,
        scale_kind="GAMEPLAY_FOOTPRINT",
    )
    rush_effect["sourceStartMs"] = 2450
    replace_phase_two_cues(rush_presentation_stage, [rush_effect])

    landing = lambda offsets: damage_hit(
        {"kind": "CIRCLE", "outerRadiusM": 8.0},
        offsets,
        "damage.valtan.jump-spin",
        push_range_m=3.0,
        push_ms=242,
        down_ms=2000,
    )
    # These occurrences share the landing clip, but adding a new area hit to
    # their established mechanics was not approved. Restore the non-damaging
    # authored contract on every pass so an older generated document cannot
    # retain the experimental same-clip contact.
    for pattern_id, stage_id in (
        ("VALTAN_TERRAIN_DESTRUCTION", "STEP_03"),
        ("VALTAN_TRASH", "STEP_03"),
        ("VALTAN_ARENA_BREAK_109", "IMPACT_HOLD"),
    ):
        stage(pattern(gameplay, pattern_id), stage_id)["hit"] = {
            "shape": {"kind": "NONE"}
        }
    for pattern_id, stage_id, offsets_ms in (
        ("VALTAN_SIX_PIZZA_106", "STEP_03", [267]),
        ("VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK", "IMPACT", [67]),
        ("VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK", "IMPACT", [67]),
    ):
        stage(pattern(gameplay, pattern_id), stage_id)["hit"] = landing(offsets_ms)

    pizza = pattern(gameplay, "VALTAN_SIX_PIZZA_106")
    stage(pizza, "STEP_04")["hit"] = stomp([2100])
    stage(pizza, "STEP_05")["hit"] = damage_hit(
        {"kind": "CIRCLE", "outerRadiusM": 12.0},
        [1300],
        "damage.valtan.ledge-roar",
        push_range_m=2.0,
        push_ms=242,
        down_ms=2000,
    )
    stage(pizza, "STEP_07")["hit"] = damage_hit(
        {"kind": "CIRCLE", "outerRadiusM": 25.0},
        [250],
        "damage.valtan.super-smash",
        push_range_m=3.0,
        push_ms=242,
        down_ms=2000,
    )
    stage(pizza, "STEP_11")["hit"] = damage_hit(
        {"kind": "CONE", "angleDegrees": 90.0, "lengthM": 12.0},
        [150, 700, 1150],
        "damage.valtan.ground-wave-smash",
        push_range_m=0.4,
        push_ms=97,
        down_ms=2000,
    )

def author_runtime_completion(gameplay: dict[str, Any], presentation: dict[str, Any]) -> None:
    """Author independent hazards and finite boss motion without reordering the canonical sequence."""
    release = stage(pattern(gameplay, "VALTAN_CATCH_BREATH"), "STEP_04")["events"][0]
    release.update(
        releaseMode="ARENA_EJECTION",
        speedMps=24.0,
        durationMs=500,
        yawOffsetDegrees=180.0,
    )
    four = stage(pattern(gameplay, "VALTAN_SEQUENCE_FOUR"), "STEP_01")
    four["hit"]["shape"] = {"kind": "CROSS", "lengthM": 18.0, "halfWidthM": 2.5}

    silence = pattern(gameplay, "VALTAN_SILENCE_SLOT")
    silence["stages"] = [
        {
            "stageId": "STEP_01",
            "actionId": "valtan.authoring.silence-slot.step-01",
            "stageKind": "ACTIVE",
            "durationMs": 2633,
            "defaultNextActionId": "valtan.authoring.silence-slot.apply",
            "hit": none_hit(),
            "motion": None,
            "events": [{
                "eventId": "event.valtan.silence-slot.step-01.enter",
                "trigger": "ENTER",
                "kind": "SET_PLAYER_SILENCE",
                "durationMs": 7633,
            }],
            "branches": [],
        },
        {
            "stageId": "SILENCE_APPLY",
            "actionId": "valtan.authoring.silence-slot.apply",
            "stageKind": "ACTIVE",
            "durationMs": 100,
            "defaultNextActionId": None,
            "hit": none_hit(),
            "motion": None,
            "events": [],
            "branches": [],
        },
    ]
    silence_p = pattern(presentation, "VALTAN_SILENCE_SLOT")
    silence_p["stages"] = [
        {
            "stageId": "STEP_01",
            "actionId": "valtan.authoring.silence-slot.step-01",
            "sequenceRole": "ROAR",
            "animation": {
                "endPolicy": "EXACT",
                "repeatCount": 1,
                "occurrences": [{
                    "clipOccurrenceId":
                        "VALTAN_SILENCE_SLOT.STEP_01.composition.clip.01",
                    "clip": "mesh_evt1_att_battle_5_01_end",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 2633,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                }],
            },
            "effectCues": [],
            "cameraInvocations": [],
        },
        {
            "stageId": "SILENCE_APPLY",
            "actionId": "valtan.authoring.silence-slot.apply",
            "sequenceRole": "SILENCE_APPLY",
            "animation": {"mode": "NONE"},
            "effectCues": [],
            "cameraInvocations": [],
        },
    ]

    bind = pattern(gameplay, "VALTAN_BIND_SLOT")
    bind["stages"] = [
        {
            "stageId": "STEP_01",
            "actionId": "valtan.authoring.bind-slot.step-01",
            "stageKind": "ACTIVE",
            "durationMs": 5000,
            "defaultNextActionId": "valtan.authoring.bind-slot.recovery",
            "hit": none_hit(),
            "motion": None,
            "events": [
                {
                    "eventId": "event.valtan.bind-slot.step-01.enter",
                    "trigger": "ENTER",
                    "kind": "SET_PLAYER_BIND",
                    "heightM": 5.0,
                    "durationMs": 5000,
                },
                {
                    "eventId": "event.valtan.bind-slot.step-01.exit",
                    "trigger": "EXIT",
                    "kind": "SET_PLAYER_BIND",
                    "heightM": 0.0,
                    "durationMs": 0,
                },
            ],
            "branches": [],
        },
        {
            "stageId": "RECOVERY",
            "actionId": "valtan.authoring.bind-slot.recovery",
            "stageKind": "RECOVERY",
            "durationMs": 3533,
            "defaultNextActionId": None,
            "hit": none_hit(),
            "motion": None,
            "events": [],
            "branches": [],
        },
    ]
    bind_p = pattern(presentation, "VALTAN_BIND_SLOT")
    bind_p["stages"] = [
        {
            "stageId": "STEP_01",
            "actionId": "valtan.authoring.bind-slot.step-01",
            "sequenceRole": "BIND_HOLD",
            "animation": {
                "endPolicy": "EXACT",
                "repeatCount": 1,
                "occurrences": [
                    {
                        "clipOccurrenceId":
                            "VALTAN_BIND_SLOT.STEP_01.composition.clip.01",
                        "clip": "mesh_att_battle_5_01_start",
                        "mappingBasis": "PROJECT_AUTHORED",
                        "sourceStartMs": 0, "playMs": 1400,
                        "playRate": 1.0, "repeatUntilStageEnd": False,
                    },
                    *[
                        {
                            "clipOccurrenceId":
                                f"VALTAN_BIND_SLOT.STEP_01.composition.clip.{ordinal:02d}",
                            "clip": "mesh_att_battle_5_01_loop",
                            "mappingBasis": "PROJECT_AUTHORED",
                            "sourceStartMs": 0, "playMs": 900,
                            "playRate": 1.0, "repeatUntilStageEnd": False,
                        }
                        for ordinal in range(2, 5)
                    ],
                    {
                        "clipOccurrenceId":
                            "VALTAN_BIND_SLOT.STEP_01.composition.clip.05",
                        "clip": "mesh_att_battle_5_01_end",
                        "mappingBasis": "PROJECT_AUTHORED",
                        "sourceStartMs": 0, "playMs": 900,
                        "playRate": 1.0, "repeatUntilStageEnd": False,
                    },
                ],
            },
            "effectCues": [],
            "cameraInvocations": [],
        },
        {
            "stageId": "RECOVERY",
            "actionId": "valtan.authoring.bind-slot.recovery",
            "sequenceRole": "RECOVERY",
            "animation": {
                "endPolicy": "EXACT",
                "repeatCount": 1,
                "occurrences": [{
                    "clipOccurrenceId":
                        "VALTAN_BIND_SLOT.RECOVERY.composition.clip.01",
                    "clip": "mesh_att_battle_5_01_end",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 3533,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                }],
            },
            "effectCues": [],
            "cameraInvocations": [],
        },
    ]

    groggy_id = "VALTAN_GROGGY_FOLLOWUP"
    stagger = pattern(gameplay, "VALTAN_STAGGER_SLOT")
    stagger.update(
        displayName="마력구 파괴 패턴",
        entryActionId="valtan.authoring.stagger-slot.channel",
        sourceActionIds=[420617],
    )
    stagger.pop("verticalOffsetM", None)
    stagger["stages"] = [
        {
            "stageId": "CHANNEL",
            "actionId": "valtan.authoring.stagger-slot.channel",
            "stageKind": "ACTIVE",
            "durationMs": 12000,
            "defaultNextActionId": "valtan.authoring.stagger-slot.final-attack",
            "hit": none_hit(),
            "motion": None,
            "events": [],
            "branches": [
                {
                    "outcome": "HEALTH_DAMAGE_THRESHOLD_REACHED",
                    "nextActionId": None,
                    "nextPatternId": groggy_id,
                },
                {
                    "outcome": "TIMEOUT",
                    "nextActionId": "valtan.authoring.stagger-slot.final-attack",
                },
            ],
            "verticalOffsetM": 0.5,
            "bossResponse": {
                "kind": "ACCUMULATED_HEALTH_DAMAGE",
                "threshold": 1000,
            },
        },
        {
            "stageId": "FINAL_ATTACK",
            "actionId": "valtan.authoring.stagger-slot.final-attack",
            "stageKind": "ACTIVE",
            "durationMs": 3000,
            "defaultNextActionId": None,
            "hit": {
                "shape": {"kind": "CIRCLE", "outerRadiusM": 100.0},
                "schedule": {
                    "kind": "INTERVAL", "count": 1,
                    "firstOffsetMs": 1000, "intervalMs": 0,
                },
                "serverDamageProfileId":
                    "damage.valtan.omnidirectional-wipe-130",
                "pushRangeM": 0.0,
                "pushMs": 0,
                "knockdown": True,
                "downMs": 2000,
            },
            "motion": None,
            "events": [],
            "branches": [],
        },
    ]
    stagger_p = pattern(presentation, "VALTAN_STAGGER_SLOT")
    stagger_p.update(
        sourceSequenceIndex=1,
        presentationSources=[{
            "sourceActionId": 420617,
            "sequenceIndex": 1,
            "role": "PRIMARY",
        }],
    )
    stagger_p["stages"] = [
        {
            "stageId": "CHANNEL",
            "actionId": "valtan.authoring.stagger-slot.channel",
            "sequenceRole": "CHANNEL",
            "animation": {
                "endPolicy": "LOOP_TO_STAGE_END",
                "repeatCount": 1,
                "occurrences": [
                    {
                        "clipOccurrenceId":
                            "VALTAN_STAGGER_SLOT.CHANNEL.composition.clip.01",
                        "clip": "mesh_att_battle_17_start",
                        "mappingBasis": "PROJECT_AUTHORED",
                        "sourceStartMs": 0, "playMs": 2000,
                        "playRate": 1.0, "repeatUntilStageEnd": False,
                    },
                    {
                        "clipOccurrenceId":
                            "VALTAN_STAGGER_SLOT.CHANNEL.composition.clip.02",
                        "clip": "mesh_att_battle_17_loop",
                        "mappingBasis": "PROJECT_AUTHORED",
                        "sourceStartMs": 0, "playMs": 0,
                        "playRate": 1.0, "repeatUntilStageEnd": True,
                    },
                ],
            },
            "effectCues": [],
            "cameraInvocations": [],
        },
        {
            "stageId": "FINAL_ATTACK",
            "actionId": "valtan.authoring.stagger-slot.final-attack",
            "sequenceRole": "FINAL_ATTACK",
            "animation": {
                "endPolicy": "EXACT",
                "repeatCount": 1,
                "occurrences": [{
                    "clipOccurrenceId":
                        "VALTAN_STAGGER_SLOT.FINAL_ATTACK.composition.clip.01",
                    "clip": "mesh_att_battle_17_end",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0, "playMs": 3000,
                    "playRate": 1.0, "repeatUntilStageEnd": False,
                }],
            },
            "effectCues": [],
            "cameraInvocations": [],
        },
    ]

    groggy = {
        "patternId": groggy_id,
        "displayName": "발탄 공용 그로기 후속",
        "category": "NORMAL",
        "compatibilitySelectionWeight": 0,
        "actionId": "valtan.followup.groggy",
        "entryActionId": "valtan.followup.groggy.active",
        "targetPolicy": "NONE",
        "aimPolicy": "NONE",
        "eligibility": copy.deepcopy(stagger["eligibility"]),
        "invulnerableWhileRunning": False,
        "sourceActionIds": [420618],
        "serverMotion": None,
        "reactions": [],
        "stages": [{
            "stageId": "GROGGY",
            "actionId": "valtan.followup.groggy.active",
            "stageKind": "GROGGY",
            "durationMs": 6833,
            "defaultNextActionId": None,
            "hit": none_hit(),
            "motion": None,
            "events": [
                {
                    "eventId": "event.valtan.followup.groggy.enter",
                    "trigger": "ENTER", "kind": "SET_BOSS_FLAG",
                    "flagId": "boss.flag.groggy", "enabled": True,
                },
                {
                    "eventId": "event.valtan.followup.groggy.exit",
                    "trigger": "EXIT", "kind": "SET_BOSS_FLAG",
                    "flagId": "boss.flag.groggy", "enabled": False,
                },
            ],
            "branches": [
                {
                    "outcome": "PART_DESTROYED",
                    "nextActionId": None,
                    "nextPatternId": "VALTAN_PART_BREAK",
                },
                {"outcome": "TIMEOUT", "nextActionId": None},
            ],
            "partDamagePolicy": "DESTROY_FIRST_ELIGIBLE",
        }],
    }
    groggy_p = {
        "patternId": groggy_id,
        "sourceSequenceIndex": 0,
        "presentationSources": [{
            "sourceActionId": 420618,
            "sequenceIndex": 0,
            "role": "PRIMARY",
        }],
        "stages": [{
            "stageId": "GROGGY",
            "actionId": "valtan.followup.groggy.active",
            "sequenceRole": "GROGGY",
            "animation": {
                "endPolicy": "EXACT",
                "repeatCount": 1,
                "occurrences": [
                    {
                        "clipOccurrenceId":
                            f"{groggy_id}.GROGGY.composition.clip.{ordinal:02d}",
                        "clip": clip,
                        "mappingBasis": "PROJECT_AUTHORED",
                        "sourceStartMs": 0,
                        "playMs": play_ms,
                        "playRate": 1.0,
                        "repeatUntilStageEnd": False,
                    }
                    for ordinal, (clip, play_ms) in enumerate((
                        ("mesh_abn_groggy_1_start", 1833),
                        ("mesh_abn_groggy_1_loop", 1333),
                        ("mesh_abn_groggy_1_loop", 1333),
                        ("mesh_abn_groggy_1_loop", 334),
                        ("mesh_abn_groggy_1_end", 2000),
                    ), start=1)
                ],
            },
            "effectCues": [],
            "cameraInvocations": [],
        }],
    }
    for document, replacement in ((gameplay, groggy), (presentation, groggy_p)):
        matching = next((index for index, row in enumerate(document["patterns"])
                         if row["patternId"] == groggy_id), None)
        if matching is None:
            stagger_index = next(
                index for index, row in enumerate(document["patterns"])
                if row["patternId"] == "VALTAN_STAGGER_SLOT"
            )
            document["patterns"].insert(stagger_index + 1, replacement)
        else:
            document["patterns"][matching] = replacement
    manual_rows = gameplay["decisionModel"]["manualAuditions"]
    manual_rows[:] = [row for row in manual_rows if row["patternId"] != groggy_id]
    stagger_manual_index = next(
        index for index, row in enumerate(manual_rows)
        if row["patternId"] == "VALTAN_STAGGER_SLOT"
    )
    manual_rows.insert(stagger_manual_index + 1, {
        "patternId": groggy_id,
        "sourceChainId": "derived.valtan-groggy-followup",
        "authoringPhase": 3,
        "admissionState": "DERIVED_SERVER_PATTERN",
    })

    triple = pattern(gameplay, "VALTAN_TRIPLE_COUNTER")
    triple_p = pattern(presentation, "VALTAN_TRIPLE_COUNTER")
    single_counter = pattern(gameplay, "VALTAN_COUNTER")
    single_counter_p = pattern(presentation, "VALTAN_COUNTER")
    triple["displayName"] = "3연속 내려치기 - 카운터"
    triple["entryActionId"] = "valtan.reactive.triple-counter.setup"
    setup = next(
        (row for row in triple["stages"] if row["stageId"] == "SETUP"),
        copy.deepcopy(stage(single_counter, "STEP_01")),
    )
    setup.update(
        stageId="SETUP",
        actionId="valtan.reactive.triple-counter.setup",
        stageKind="WINDUP",
        durationMs=2000,
        defaultNextActionId="valtan.reactive.triple-counter.first",
        hit=none_hit(),
        motion=None,
        events=[],
        branches=[{
            "outcome": "TIMEOUT",
            "nextActionId": "valtan.reactive.triple-counter.first",
        }],
    )
    setup.pop("counterProxy", None)

    authored_stages = [setup]
    authored_presentation_stages: list[dict[str, Any]] = []
    setup_p = next(
        (row for row in triple_p["stages"] if row["stageId"] == "SETUP"),
        copy.deepcopy(stage(single_counter_p, "STEP_01")),
    )
    setup_p.update(
        stageId="SETUP",
        actionId="valtan.reactive.triple-counter.setup",
        sequenceRole="SETUP",
    )
    setup_p["animation"] = {
        "endPolicy": "EXACT",
        "repeatCount": 1,
        "occurrences": [{
            "clipOccurrenceId":
                "valtan.reactive.triple-counter.setup.clip.01",
            "clip": "mesh_att_battle_14_01",
            "mappingBasis": "PROJECT_AUTHORED",
            "sourceStartMs": 0,
            "playMs": 2000,
            "playRate": 1.0,
            "repeatUntilStageEnd": False,
        }],
    }
    setup_p["effectCues"] = []
    setup_p["cameraInvocations"] = []
    authored_presentation_stages.append(setup_p)

    # Each mesh_att_battle_14_02 occurrence is the complete counter window.
    # The paired 14_03 stage is the committed slam and cannot consume counter.
    topology = (
        (1, "COUNTER_1", "valtan.reactive.triple-counter.first",
         "FAIL_1", "valtan.reactive.triple-counter.first-fail",
         "valtan.reactive.triple-counter.second"),
        (2, "COUNTER_2", "valtan.reactive.triple-counter.second",
         "FAIL_2", "valtan.reactive.triple-counter.second-fail",
         "valtan.reactive.triple-counter.third"),
        (3, "COUNTER_3", "valtan.reactive.triple-counter.third",
         "FAIL_3", "valtan.reactive.triple-counter.third-fail", None),
    )
    for ordinal, counter_stage_id, counter_action_id, fail_stage_id, \
            fail_action_id, next_action_id in topology:
        counter_stage = stage(triple, counter_stage_id)
        event_root = ("first", "second", "third")[ordinal - 1]
        fail_stage = stage(triple, fail_stage_id)
        counter_stage.update(
            actionId=counter_action_id,
            stageKind="WINDUP",
            durationMs=1800,
            defaultNextActionId=fail_action_id,
            hit=none_hit(),
            motion=None,
            events=[
                {
                    "eventId": f"event.valtan.triple-counter.{event_root}.counter-window.enter",
                    "trigger": "ENTER",
                    "kind": "SET_BOSS_FLAG",
                    "flagId": "boss.flag.counterable",
                    "enabled": True,
                },
                {
                    "eventId": f"event.valtan.triple-counter.{event_root}.counter-window.exit",
                    "trigger": "EXIT",
                    "kind": "SET_BOSS_FLAG",
                    "flagId": "boss.flag.counterable",
                    "enabled": False,
                },
            ],
        )
        counter_stage["branches"] = [
            {
                "outcome": "COUNTER_HIT",
                "nextActionId": None,
                "nextPatternId": groggy_id,
            },
            {"outcome": "TIMEOUT", "nextActionId": fail_action_id},
        ]
        counter_stage["counterProxy"] = {
            "kind": "BOSS_FORWARD_ARC",
            "forwardOffsetM": 0.0,
            "rightOffsetM": 0.0,
            "radiusM": 0.0,
            "arcDegrees": 180.0,
        }
        fail_stage.update(
            actionId=fail_action_id,
            stageKind="ACTIVE",
            durationMs=1667,
            defaultNextActionId=next_action_id,
            hit=damage_hit(
                {"kind": "CIRCLE", "outerRadiusM": 12.0},
                [900],
                "damage.valtan.triple-counter",
                push_range_m=0.0,
                push_ms=0,
                down_ms=2000,
            ),
            motion=None,
            events=[],
            branches=[{
                "outcome": "TIMEOUT",
                "nextActionId": next_action_id,
            }],
        )
        fail_stage.pop("counterProxy", None)
        authored_stages.extend((counter_stage, fail_stage))

        counter_stage_p = stage(triple_p, counter_stage_id)
        counter_stage_p.update(
            actionId=counter_action_id,
            sequenceRole=counter_stage_id,
        )
        occurrence_root = ("first", "second", "third")[ordinal - 1]
        counter_stage_p["animation"] = {
            "endPolicy": "EXACT",
            "repeatCount": 1,
            "occurrences": [
                {
                    "clipOccurrenceId":
                        f"valtan.reactive.triple-counter.{occurrence_root}.clip.01",
                    "clip": "mesh_att_battle_14_02",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 1000,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
                {
                    "clipOccurrenceId":
                        f"valtan.reactive.triple-counter.{occurrence_root}.clip.02",
                    "clip": "mesh_att_battle_14_02",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0,
                    "playMs": 800,
                    "playRate": 1.0,
                    "repeatUntilStageEnd": False,
                },
            ],
        }
        fail_stage_p = stage(triple_p, fail_stage_id)
        fail_stage_p.update(
            actionId=fail_action_id,
            sequenceRole=fail_stage_id,
        )
        fail_stage_p["animation"] = {
            "endPolicy": "EXACT",
            "repeatCount": 1,
            "occurrences": [{
                "clipOccurrenceId":
                    f"valtan.reactive.triple-counter.{occurrence_root}-fail.clip.01",
                "clip": "mesh_att_battle_14_03",
                "mappingBasis": "PROJECT_AUTHORED",
                "sourceStartMs": 0,
                "playMs": 1667,
                "playRate": 1.0,
                "repeatUntilStageEnd": False,
            }],
        }
        authored_presentation_stages.extend((counter_stage_p, fail_stage_p))
    triple["stages"] = authored_stages
    triple_p["stages"] = authored_presentation_stages

    warp = pattern(gameplay, "VALTAN_WARP")
    warp_p = pattern(presentation, "VALTAN_WARP")
    for leg in range(8):
        row = stage(warp, f"STEP_{leg + 2:02d}")
        retarget_delay_ms = (
            WARP_FIRST_LEG_DELAY_MS if leg == 0 else WARP_REPEAT_LEG_DELAY_MS
        )
        row["durationMs"] = retarget_delay_ms + WARP_PORTAL_TRAVEL_MS
        row["motion"] = {
            "kind": "PORTAL_TARGET_RUSH",
            "retargetDelayMs": retarget_delay_ms,
            "speedMps": WARP_PORTAL_SPEED_MPS,
            "distanceM": WARP_PORTAL_DISTANCE_M,
        }
        row["hit"]["schedule"] = {
            "kind": "EXPLICIT_OFFSETS",
            "offsetsMs": list(
                range(retarget_delay_ms, row["durationMs"], 50)
            ),
        }
        for effect_cue in stage(warp_p, row["stageId"])["effectCues"]:
            effect_cue["followPolicy"] = "snapshot"
            effect_cue["localTransform"]["position"] = [0.0, 0.0, 0.0]
    stage(warp, "STEP_10")["events"] = [{
        "eventId": "event.valtan.warp.return-center", "trigger": "ENTER",
        "kind": "RETURN_TO_ARENA_CENTER",
    }]
    stage(warp_p, "STEP_10")["bodyVisibility"] = {
        "hiddenFromMs": 0,
        "hiddenToMs": 300,
    }

    for pattern_id in ("VALTAN_SIX_PIZZA_106", "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
                       "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK"):
        pattern(gameplay, pattern_id)["serverMotion"]["moveToAnchorBeforeTakeoff"] = True
        if pattern_id == "VALTAN_SIX_PIZZA_106":
            pattern(gameplay, pattern_id)["targetPolicy"] = "LOCK_RANDOM_ALIVE_ON_START"
            pattern(gameplay, pattern_id)["aimPolicy"] = "TRACK_TARGET_EACH_TICK"
        for row in pattern(presentation, pattern_id)["stages"]:
            for effect_cue in row["effectCues"]:
                effect_cue["anchorSlotId"] = (
                    "arena.center.target-follow"
                    if pattern_id == "VALTAN_SIX_PIZZA_106"
                    else "arena.center"
                )
                effect_cue["followPolicy"] = (
                    "follow"
                    if pattern_id == "VALTAN_SIX_PIZZA_106"
                    else "snapshot"
                )

    def rock_pillar_volley(
        event_id: str,
        combat_object_id: str,
        radius_m: float,
        first_offset_ms: int,
        volley_policy: str = "BOSS_RELATIVE",
        layout_kind: str = "RADIAL_AROUND_BOSS",
    ) -> dict[str, Any]:
        return {
            "eventId": event_id,
            "trigger": "ENTER",
            "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
            "combatObjectArchetypeId": combat_object_id,
            "volleyPolicy": volley_policy,
            "countPerResolvedTarget": 4,
            "layout": {
                "kind": layout_kind,
                "radiusM": radius_m,
                "startAngleDegrees": 45.0,
                "angleStepDegrees": 90.0,
                "mappingBasis": "PROJECT_TUNED",
            },
            "spawnSchedule": {
                "kind": "INTERVAL",
                "count": 1,
                "firstOffsetMs": first_offset_ms,
                "intervalMs": 0,
            },
            "arenaRandom": {"kind": "NONE"},
            "allowOverlap": False,
            "maximumTotalObjects": 4,
        }

    stage(pattern(gameplay, "VALTAN_SIX_PIZZA_106"), "STEP_01")["events"] = [
        rock_pillar_volley(
            "event.valtan.six-pizza.rock-pillars",
            "combatobject.valtan.six-pizza.rock-pillar",
            10.0,
            1000,
            "ARENA_CENTER",
            "RADIAL_AROUND_ARENA_CENTER",
        )
    ]
    stage(pattern(gameplay, "VALTAN_STRUGGLING"), "STEP_04")["events"] = [
        rock_pillar_volley(
            "event.valtan.struggling.rock-pillars",
            "combatobject.valtan.struggling.rock-pillar",
            6.3639610307,
            833,
        )
    ]
    rock_independent_effects = (
        {
            "independentEffectId":
                "valtan.independent-effect.six-pizza-rock-pillars",
            "displayName": "피자 패턴 / 1초 후 아레나 중앙 반경 10m 대각 돌 기둥 4개",
            "ownership": "SERVER_COMBAT_OBJECT",
            "spawnEventId": "event.valtan.six-pizza.rock-pillars",
        },
        {
            "independentEffectId":
                "valtan.independent-effect.struggling-rock-pillars",
            "displayName": "발악 패턴 / 5초 지점 ±4.5m 돌 기둥 4개",
            "ownership": "SERVER_COMBAT_OBJECT",
            "spawnEventId": "event.valtan.struggling.rock-pillars",
        },
    )
    rock_independent_ids = {
        row["independentEffectId"] for row in rock_independent_effects
    }
    presentation["independentEffects"] = [
        row for row in presentation["independentEffects"]
        if row["independentEffectId"] not in rock_independent_ids
    ]
    presentation["independentEffects"].extend(rock_independent_effects)

    donut = stage(pattern(gameplay, "VALTAN_FIST_IN_OUT"), "INNER")
    donut["durationMs"] = 100
    donut["hit"] = none_hit()
    donut["events"] = [{
        "eventId": "event.valtan.fist-in-out.spawn-donut", "trigger": "ENTER",
        "kind": "SPAWN_COMBAT_OBJECT",
        "combatObjectArchetypeId": "combatobject.valtan.fist-in-out.donut", "count": 1,
    }]
    stage(pattern(presentation, "VALTAN_FIST_IN_OUT"), "INNER")["effectCues"] = []
    independent = next(row for row in presentation["independentEffects"]
                       if row["independentEffectId"] == "valtan.independent-effect.donut-in-out")
    independent.pop("cueId", None)
    independent["ownership"] = "SERVER_COMBAT_OBJECT"
    independent["spawnEventId"] = "event.valtan.fist-in-out.spawn-donut"

    finale_id = "VALTAN_GHOST_FINALE"
    if not any(row["patternId"] == finale_id for row in gameplay["patterns"]):
        def remap_finale(value: Any) -> Any:
            if isinstance(value, dict):
                return {key: remap_finale(item) for key, item in value.items()}
            if isinstance(value, list):
                return [remap_finale(item) for item in value]
            if isinstance(value, str):
                return value.replace("valtan.sequence.warp", "valtan.sequence.ghost-finale").replace(
                    "cue.valtan.phase2.warp.", "cue.valtan.finale.warp.").replace(
                    "event.valtan.phase2.warp.", "event.valtan.finale.warp.").replace(
                    "event.valtan.warp.", "event.valtan.finale.warp.")
            return value
        finale = remap_finale(copy.deepcopy(warp))
        finale["patternId"] = finale_id
        finale["displayName"] = "유령 발탄 · 포탈 최종 패턴"
        finale["finale"] = {
            "kind": "GHOST_PORTAL_LOOP", "ghostArchetypeId": "BOSS_VALTAN_GHOST",
            "ghostPatternIds": [
                "VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH", "VALTAN_SEQUENCE_FOUR",
                "VALTAN_CROSS", "VALTAN_CHARGE", "VALTAN_CHARGE_2",
            ],
            "spawnHalfExtentsM": [10.0, 10.0], "maximumActiveGhosts": 1,
        }
        finale_p = remap_finale(copy.deepcopy(warp_p))
        finale_p["patternId"] = finale_id
        gameplay["patterns"].append(finale)
        presentation["patterns"].append(finale_p)
        gameplay["decisionModel"]["manualAuditions"].append({
            "patternId": finale_id, "sourceChainId": "derived.warp-ghost-finale", "authoringPhase": 3,
            "admissionState": "DERIVED_SERVER_PATTERN",
        })

    # The finale intentionally retains the arena-corner portal choreography.
    # Patch it on every authoring pass so a pre-existing derived row cannot
    # inherit the normal WARP target-rush policy from its original clone.
    finale = pattern(gameplay, finale_id)
    finale_p = pattern(presentation, finale_id)
    finale["finale"]["ghostPatternIds"] = [
        "VALTAN_WHIRLWIND",
        "VALTAN_FOUR_SLASH",
        "VALTAN_SEQUENCE_FOUR",
        "VALTAN_CROSS",
        "VALTAN_CHARGE",
        "VALTAN_CHARGE_2",
    ]
    for leg in range(8):
        row = stage(finale, f"STEP_{leg + 2:02d}")
        row["motion"] = {
            "kind": "PORTAL_CROSS_ARENA",
            "cornerIndex": leg % 4,
            "halfExtentsM": [22.0, 22.0],
        }
        row["hit"]["schedule"] = {
            "kind": "EXPLICIT_OFFSETS",
            "offsetsMs": list(range(0, 900, 50)),
        }
        for effect_cue in stage(finale_p, row["stageId"])["effectCues"]:
            effect_cue["followPolicy"] = "snapshot"
            effect_cue["localTransform"]["position"] = [0.0, 0.0, 0.0]

    respawn = stage(pattern(gameplay, "VALTAN_GHOST_RESPAWN_AUDITION"), "STEP_01")
    respawn["events"] = [{
        "eventId": "event.valtan.ghost-respawn.phase-3",
        "trigger": "ENTER",
        "kind": "SET_GAMEPLAY_PHASE",
        "gameplayPhase": 3,
    }]

    portal_id = "VALTAN_GHOST_PORTAL_ONCE"
    portal = {
        "patternId": portal_id,
        "displayName": "망령 포탈 돌진 1회",
        "category": "NORMAL",
        "compatibilitySelectionWeight": 0,
        "actionId": "valtan.ghost.portal-once",
        "entryActionId": "valtan.ghost.portal-once.active",
        "targetPolicy": "NONE",
        "aimPolicy": "NONE",
        "eligibility": {
            "armorRequirement": "ANY", "phaseRequirement": "ANY",
            "minimumGameplayPhase": 1, "maximumGameplayPhase": 3,
            "minimumHealthBarInclusive": 0, "maximumHealthBarInclusive": 0,
            "minimumRangeM": 0.0, "maximumRangeM": 1.0,
            "cooldownPolicy": "DERIVED_SOURCE_ACTION",
            "selectionCooldownMs": None, "cooldownGroupId": None,
            "repeatPolicy": {
                "kind": "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE", "limit": 0,
            },
        },
        "invulnerableWhileRunning": False,
        "sourceActionIds": [420622],
        "serverMotion": None,
        "reactions": [],
        "stages": [{
            "stageId": "ACTIVE",
            "actionId": "valtan.ghost.portal-once.active",
            "stageKind": "ACTIVE",
            "durationMs": 1900,
            "defaultNextActionId": None,
            "hit": none_hit(),
            "motion": None,
            "events": [{
                "eventId": "event.valtan.ghost.portal-once.volley",
                "trigger": "ENTER",
                "kind": "SPAWN_COMBAT_OBJECT_VOLLEY",
                "combatObjectArchetypeId":
                    "combatobject.valtan.ghost.portal-charge",
                "volleyPolicy": "BOSS_RELATIVE",
                "countPerResolvedTarget": 3,
                "layout": {
                    "kind": "RADIAL_AROUND_BOSS",
                    "radiusM": GHOST_PORTAL_CIRCUMRADIUS_M,
                    "startAngleDegrees": 30.0,
                    "angleStepDegrees": 120.0,
                    "mappingBasis": "PROJECT_TUNED",
                },
                "spawnSchedule": {
                    "kind": "INTERVAL", "count": 1,
                    "firstOffsetMs": 0, "intervalMs": 0,
                },
                "arenaRandom": {"kind": "NONE"},
                "allowOverlap": False,
                "maximumTotalObjects": 3,
            }],
            "branches": [],
        }],
    }
    portal_p = {
        "patternId": portal_id,
        "sourceSequenceIndex": 1,
        "presentationSources": [{
            "sourceActionId": 420622, "sequenceIndex": 1, "role": "PRIMARY",
        }],
        "stages": [{
            "stageId": "ACTIVE",
            "actionId": "valtan.ghost.portal-once.active",
            "sequenceRole": "STEP",
            "animation": {
                "endPolicy": "LOOP_TO_STAGE_END", "repeatCount": 1,
                "occurrences": [{
                    "clipOccurrenceId": "valtan.ghost.portal-once.active.clip-01",
                    "clip": "mesh_att_battle_18_02",
                    "mappingBasis": "PROJECT_AUTHORED",
                    "sourceStartMs": 0, "playMs": 0, "playRate": 1.0,
                    "repeatUntilStageEnd": True,
                }],
            },
            "bodyVisibility": {
                "hiddenFromMs": 0,
                "hiddenToMs": 300,
            },
            "effectCues": [{
                "cueId": "cue.valtan.ghost.portal-once.dash-floor",
                "scalePolicy": {"kind": "OWNER_RELATIVE"},
                "occurrenceId":
                    "cue.valtan.ghost.portal-once.dash-floor.occurrence.01",
                "effectAssetId":
                    "effect.valtan.project-tuned.sequence.warp.portal",
                "clipOccurrenceId":
                    "valtan.ghost.portal-once.active.clip-01",
                "sourceStartMs": 0,
                "sourceEndMs": None,
                "anchorSlotId": "root",
                "followPolicy": "snapshot",
                "stopPolicy": "natural",
                "repeatPolicy": "once",
                "localTransform": {
                    "position": [0.0, 0.0, 0.0],
                    "rotationDegrees": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
                "mappingBasis": "PROJECT_AUTHORED",
            }],
            "cameraInvocations": [],
        }],
    }
    for document, replacement in ((gameplay, portal), (presentation, portal_p)):
        matching = next((index for index, row in enumerate(document["patterns"])
                         if row["patternId"] == portal_id), None)
        if matching is None:
            finale_index = next(index for index, row in enumerate(document["patterns"])
                                if row["patternId"] == finale_id)
            document["patterns"].insert(finale_index + 1, replacement)
        else:
            document["patterns"][matching] = replacement
    manual_rows = gameplay["decisionModel"]["manualAuditions"]
    manual_rows[:] = [row for row in manual_rows if row["patternId"] != portal_id]
    finale_manual_index = next(index for index, row in enumerate(manual_rows)
                               if row["patternId"] == finale_id)
    manual_rows.insert(finale_manual_index + 1, {
        "patternId": portal_id,
        "sourceChainId": "derived.ghost-portal-once",
        "authoringPhase": 3,
        "admissionState": "DERIVED_SERVER_PATTERN",
    })
    independent_id = "valtan.independent-effect.ghost-portal-once"
    presentation["independentEffects"] = [
        row for row in presentation["independentEffects"]
        if row["independentEffectId"] != independent_id
    ]
    presentation["independentEffects"].append({
        "independentEffectId": independent_id,
        "displayName": "망령 포탈 동시 돌진 / 외접반지름 7.5m 정삼각형",
        "ownership": "SERVER_COMBAT_OBJECT",
        "spawnEventId": "event.valtan.ghost.portal-once.volley",
    })
    author_clip_aligned_stage_hits(gameplay, presentation)


def build(pattern_id: str | None = None) -> tuple[dict[str, Any], dict[str, Any]]:
    gameplay = promotion._read_json(GAMEPLAY)
    presentation = promotion._read_json(PRESENTATION)
    if pattern_id == "VALTAN_TRASH":
        author_trash_capture_flow(gameplay, presentation)
        return gameplay, presentation
    saved_presentation = copy.deepcopy(presentation)
    author_existing_patterns(gameplay, presentation)
    author_trash_capture_flow(gameplay, presentation)
    author_terrain_pairs(gameplay, presentation)
    saved_patterns = indexed(saved_presentation["patterns"], "patternId")
    for authored in presentation["patterns"]:
        # Seed cues for new patterns, but retain the exact authored cue list
        # (including Unlink's empty list) once a pattern has been saved.
        # Dash is mutated in-place from that saved document above and owns its
        # GROGGY stage directly. Its existing occurrences/cues are already
        # preserved, while the generic promotion helper intentionally rejects
        # any stage-closure change.
        if authored["patternId"] in {
            "VALTAN_DASH_CHARGE",
            "VALTAN_PART_BREAK",
            "VALTAN_COUNTER",
        }:
            continue
        promotion._preserve_manual_presentation_enrichment(
            authored, saved_patterns.get(authored["patternId"])
        )
    author_runtime_completion(gameplay, presentation)
    return gameplay, presentation


def serialized(document: dict[str, Any]) -> bytes:
    return (
        promotion.json.dumps(document, ensure_ascii=False, indent=2) + "\n"
    ).encode("utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=("Apply", "Validate"), default="Validate")
    parser.add_argument("--pattern-id", choices=("VALTAN_TRASH",), default=None)
    args = parser.parse_args()
    gameplay, presentation = build(args.pattern_id)
    outputs = {
        GAMEPLAY: serialized(gameplay),
        PRESENTATION: serialized(presentation),
    }
    drift = [path for path, value in outputs.items() if path.read_bytes() != value]
    if args.mode == "Validate":
        if drift:
            raise AuthoringError(
                "Phase 2 mechanic authoring drift: "
                + ", ".join(str(path.relative_to(ROOT)) for path in drift)
            )
        print("Valtan Phase 2 mechanic authoring: VALID")
        return 0
    promotion._atomic_commit(outputs)
    print(
        "Valtan Phase 2 mechanic authoring: APPLIED "
        f"({len(gameplay['patterns'])} gameplay patterns)"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AuthoringError, promotion.PromotionError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        raise SystemExit(1)
