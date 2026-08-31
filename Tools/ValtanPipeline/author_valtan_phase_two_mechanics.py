#!/usr/bin/env python3
"""Idempotently author Server mechanics onto reviewed Valtan Phase 2 clips."""

from __future__ import annotations

import argparse
import copy
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


def author_existing_patterns(
    gameplay: dict[str, Any], presentation: dict[str, Any]
) -> None:
    gameplay_by_id = indexed(gameplay["patterns"], "patternId")
    presentation_by_id = indexed(presentation["patterns"], "patternId")

    dash = gameplay_by_id["VALTAN_DASH_CHARGE"]
    dash["targetPolicy"] = "LOCK_NEAREST_ON_START"
    dash["aimPolicy"] = "LOCK_FACING_ON_START"
    dash_charge = stage(dash, "CHARGE")
    dash_charge["branches"] = [
        {
            "outcome": "WALL_CONTACT",
            "nextActionId": "valtan.attack.dash-charge.groggy",
        },
        {
            "outcome": "TIMEOUT",
            "nextActionId": "valtan.attack.dash-charge.recovery",
        },
    ]
    dash_charge["defaultNextActionId"] = "valtan.attack.dash-charge.recovery"
    dash_recovery = stage(dash, "RECOVERY")
    dash_recovery.pop("partDamagePolicy", None)
    dash_recovery["events"] = []
    dash_recovery["branches"] = [
        {"outcome": "TIMEOUT", "nextActionId": None},
    ]
    dash_recovery["defaultNextActionId"] = None
    dash_groggy = stage(dash, "GROGGY")
    dash_groggy["partDamagePolicy"] = "DESTROY_FIRST_ELIGIBLE"
    dash_groggy["branches"] = [
        {
            "outcome": "PART_DESTROYED",
            "nextActionId": "valtan.attack.dash-charge.part-break",
        },
        {
            "outcome": "TIMEOUT",
            "nextActionId": "valtan.attack.dash-charge.recovery",
        },
    ]
    dash_groggy["defaultNextActionId"] = "valtan.attack.dash-charge.recovery"
    stage(dash, "PART_BREAK")["defaultNextActionId"] = None
    dash_stage_order = ("WINDUP", "CHARGE", "GROGGY", "RECOVERY", "PART_BREAK")
    dash_stages = indexed(dash["stages"], "stageId")
    dash["stages"] = [dash_stages[stage_id] for stage_id in dash_stage_order]
    dash_presentation = presentation_by_id["VALTAN_DASH_CHARGE"]
    dash_presentation_stages = indexed(
        dash_presentation["stages"], "stageId"
    )
    dash_presentation["stages"] = [
        dash_presentation_stages[stage_id] for stage_id in dash_stage_order
    ]

    scripted_sequence = gameplay["decisionModel"]["scriptedSequence"]
    if "flowId" not in scripted_sequence:
        # The saved Flow owns Product order; only legacy inline migrations reorder it.
        sequence = scripted_sequence["patternIds"]
        sequence[:] = [row for row in sequence if row != "VALTAN_DASH_CHARGE"]
        sequence.insert(sequence.index("VALTAN_ARENA_BREAK_109"), "VALTAN_DASH_CHARGE")

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
    charge["targetPolicy"] = "LOCK_NEAREST_ON_START"
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
        [500, 1350],
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
        gameplay_stage["durationMs"] = 2300
        gameplay_stage["hit"] = damage_hit(
            {"kind": "BOX", "lengthM": 8.0, "halfWidthM": 2.5},
            list(range(500, 1300, 50)),
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
        replace_phase_two_cues(
            leg_p,
            [
                cue(
                    f"{CUE_PREFIX}warp.step-{leg:02d}.composite",
                    "effect.valtan.project-tuned.sequence.warp.portal",
                    leg_p,
                    scale_kind="OWNER_RELATIVE",
                    position=(0.0, 0.0, 0.0),
                    follow_policy="snapshot",
                ),
            ],
        )
    recovery_p = stage(warp_p, "STEP_10")
    replace_phase_two_cues(
        recovery_p,
        [
            cue(
                f"{CUE_PREFIX}warp.step-10.composite",
                "effect.valtan.project-tuned.sequence.warp.portal",
                recovery_p,
                scale_kind="OWNER_RELATIVE",
                position=(0.0, 0.0, 0.0),
                follow_policy="snapshot",
            ),
        ],
    )

    counter = gameplay_by_id["VALTAN_COUNTER"]
    counter_p = presentation_by_id["VALTAN_COUNTER"]
    set_tracking(counter)
    counter_one = stage(counter, "STEP_01")
    counter_one["stageKind"] = "WINDUP"
    counter_two = stage(counter, "STEP_02")
    counter_two["stageKind"] = "WINDUP"
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
            "nextActionId": "valtan.sequence.counter.step-04",
        },
        {
            "outcome": "TIMEOUT",
            "nextActionId": "valtan.sequence.counter.step-03",
        },
    ]
    counter_three = stage(counter, "STEP_03")
    counter_three["stageKind"] = "ACTIVE"
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
    stage(counter, "STEP_04")["stageKind"] = "GROGGY"
    counter_slam_p = stage(counter_p, "STEP_03")
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
    replace_phase_two_cues(
        trash_p,
        [
            cue(
                f"{REQUESTED_CUE_PREFIX}trash.composite",
                "effect.valtan.project-tuned.sequence.trash",
                trash_p,
                scale_kind="OWNER_RELATIVE",
            )
        ],
    )

    catch_breath = gameplay_by_id["VALTAN_CATCH_BREATH"]
    catch_breath["targetPolicy"] = "LOCK_RANDOM_ALIVE_BEHIND_ON_START"
    catch_breath["aimPolicy"] = "LOCK_FACING_ON_START"

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

    scripted_sequence = gameplay["decisionModel"]["scriptedSequence"]
    if "flowId" not in scripted_sequence:
        sequence = scripted_sequence["patternIds"]
        sequence[:] = [row for row in sequence if row not in authored_ids]
        terrain_index = sequence.index("VALTAN_TERRAIN_DESTRUCTION")
        sequence[terrain_index:terrain_index] = [
            "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
            "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK",
        ]
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


def author_runtime_completion(gameplay: dict[str, Any], presentation: dict[str, Any]) -> None:
    """Author independent hazards and finite boss motion without editing saved Flow slots."""
    release = stage(pattern(gameplay, "VALTAN_CATCH_BREATH"), "STEP_04")["events"][0]
    release.update(
        releaseMode="ARENA_EJECTION",
        speedMps=24.0,
        durationMs=500,
        yawOffsetDegrees=180.0,
    )
    four = stage(pattern(gameplay, "VALTAN_SEQUENCE_FOUR"), "STEP_01")
    four["hit"]["shape"] = {"kind": "CROSS", "lengthM": 18.0, "halfWidthM": 2.5}

    warp = pattern(gameplay, "VALTAN_WARP")
    warp_p = pattern(presentation, "VALTAN_WARP")
    for leg in range(8):
        row = stage(warp, f"STEP_{leg + 2:02d}")
        row["durationMs"] = 2300
        row["motion"] = {
            "kind": "PORTAL_TARGET_RUSH",
            "retargetDelayMs": 500,
            "speedMps": 20.0,
            "distanceM": 16.0,
        }
        row["hit"]["schedule"] = {
            "kind": "EXPLICIT_OFFSETS",
            "offsetsMs": list(range(500, 1300, 50)),
        }
        for effect_cue in stage(warp_p, row["stageId"])["effectCues"]:
            effect_cue["followPolicy"] = "snapshot"
            effect_cue["localTransform"]["position"] = [0.0, 0.0, 0.0]
    stage(warp, "STEP_10")["events"] = [{
        "eventId": "event.valtan.warp.return-center", "trigger": "ENTER",
        "kind": "RETURN_TO_ARENA_CENTER",
    }]

    for pattern_id in ("VALTAN_SIX_PIZZA_106", "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK",
                       "VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK"):
        pattern(gameplay, pattern_id)["serverMotion"]["moveToAnchorBeforeTakeoff"] = True
        if pattern_id == "VALTAN_SIX_PIZZA_106":
            pattern(gameplay, pattern_id)["targetPolicy"] = "LOCK_RANDOM_ALIVE_ON_START"
            pattern(gameplay, pattern_id)["aimPolicy"] = "LOCK_FACING_ON_START"
        for row in pattern(presentation, pattern_id)["stages"]:
            for effect_cue in row["effectCues"]:
                effect_cue["anchorSlotId"] = (
                    "arena.center.facing"
                    if pattern_id == "VALTAN_SIX_PIZZA_106"
                    else "arena.center"
                )
                effect_cue["followPolicy"] = "snapshot"

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
            "ghostPatternIds": ["VALTAN_WHIRLWIND", "VALTAN_FOUR_SLASH", "VALTAN_SEQUENCE_FOUR"],
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
