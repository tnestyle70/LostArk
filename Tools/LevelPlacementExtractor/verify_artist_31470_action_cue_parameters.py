#!/usr/bin/env python3
"""Verify source-exact Artist 31470 particle cue occurrences and overrides."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
from typing import Any


ZOOM_EXPECTED = [
    ("life", "scalar", 0.6000000238418579),
]

LIGHT_COMMON_EXPECTED = [
    ("Spawn", "scalar", 0.0),
    ("Lifetime", "scalar", 0.20000000298023224),
    ("Size", "scalar", 600.0),
]

LIGHT_TRAILING_EXPECTED = [
    ("Alpha", "scalar", 1.0),
    ("Location", "vector", [1.0, 1.0, 1.0]),
]

LIGHT_OCCURRENCES = {
    "notify-029": (True, [2.0, 2.0, 2.0]),
    "notify-030": (False, [2.0, 2.0, 2.0]),
    "notify-031": (False, [5.0, 1.0, 1.0]),
    "notify-032": (False, [5.0, 1.0, 1.0]),
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def require_number(actual: Any, expected: float, context: str) -> None:
    require(
        isinstance(actual, (int, float))
        and math.isfinite(float(actual))
        and math.isclose(float(actual), expected, rel_tol=0.0, abs_tol=1e-7),
        f"{context}: expected {expected}, got {actual!r}",
    )


def require_overrides(
    cue: dict[str, Any],
    expected: list[tuple[str, str, float | list[float]]],
) -> None:
    cue_id = str(cue.get("cueId") or "")
    typed = cue.get("typedPayload")
    require(isinstance(typed, dict), f"{cue_id}: typedPayload missing")
    require(
        typed.get("parameterOverridesDecoded") is True,
        f"{cue_id}: parameter override table was not decoded",
    )
    overrides = typed.get("parameterOverrides")
    require(isinstance(overrides, list), f"{cue_id}: parameterOverrides missing")
    require(
        len(overrides) == len(expected),
        f"{cue_id}: expected {len(expected)} overrides, got {len(overrides)}",
    )
    for index, (actual, (name, value_type, value)) in enumerate(
        zip(overrides, expected, strict=True)
    ):
        require(isinstance(actual, dict), f"{cue_id}: override {index} malformed")
        require(actual.get("sourceIndex") == index, f"{cue_id}: override order drift")
        require(actual.get("name") == name, f"{cue_id}: override name drift")
        require(actual.get("type") == value_type, f"{cue_id}: override type drift")
        if value_type == "scalar":
            require("vectorValue" not in actual, f"{cue_id}: scalar replaced by vector")
            require_number(actual.get("scalarValue"), float(value), f"{cue_id}/{name}")
        else:
            require("scalarValue" not in actual, f"{cue_id}: vector replaced by scalar")
            actual_vector = actual.get("vectorValue")
            require(
                isinstance(actual_vector, list) and len(actual_vector) == 3,
                f"{cue_id}/{name}: vector value malformed",
            )
            for component, expected_component in zip(
                actual_vector, value, strict=True
            ):
                require_number(
                    component,
                    float(expected_component),
                    f"{cue_id}/{name}",
                )


def validate_recipe(recipe: dict[str, Any]) -> dict[str, int]:
    require(
        recipe.get("schema") == "lostark.effect-action-cue-recipe",
        "action cue recipe schema mismatch",
    )
    require(recipe.get("skillId") == 31470, "expected Artist skill 31470")
    cues = recipe.get("cues")
    require(isinstance(cues, list) and cues, "action cue recipe has no cues")

    by_suffix: dict[str, dict[str, Any]] = {}
    occurrence_keys: set[tuple[int, int]] = set()
    for cue in cues:
        require(isinstance(cue, dict), "cue row must be an object")
        cue_id = str(cue.get("cueId") or "")
        suffix = cue_id.rsplit("/", 1)[-1]
        require(suffix not in by_suffix, f"duplicate cue suffix: {suffix}")
        by_suffix[suffix] = cue
        occurrence = cue.get("sourceOccurrence")
        require(isinstance(occurrence, dict), f"{cue_id}: sourceOccurrence missing")
        expected_notify_id = f"action-31470/stage-000/{suffix}"
        require(
            occurrence.get("notifyId") == expected_notify_id,
            f"{cue_id}: source notify identity drift",
        )
        require(
            occurrence.get("stageIndex") == cue.get("sourceStageIndex"),
            f"{cue_id}: source stage identity drift",
        )
        stage_notify_index = occurrence.get("stageNotifyIndex")
        require(
            isinstance(stage_notify_index, int) and stage_notify_index >= 0,
            f"{cue_id}: source stage notify index missing",
        )
        occurrence_key = (int(occurrence["stageIndex"]), stage_notify_index)
        require(
            occurrence_key not in occurrence_keys,
            f"{cue_id}: duplicate source occurrence identity",
        )
        occurrence_keys.add(occurrence_key)
        typed = cue.get("typedPayload")
        require(isinstance(typed, dict), f"{cue_id}: typedPayload missing")
        source_enabled = typed.get("enabled")
        require(
            occurrence.get("enabled") is source_enabled
            and cue.get("executionEnabled") is source_enabled,
            f"{cue_id}: source enabled flag drift",
        )

    zoom = by_suffix.get("notify-026")
    require(zoom is not None, "ZoomBlur notify-026 missing")
    require_overrides(zoom, ZOOM_EXPECTED)

    for suffix, (enabled, color) in LIGHT_OCCURRENCES.items():
        cue = by_suffix.get(suffix)
        require(cue is not None, f"Light {suffix} missing")
        occurrence = cue["sourceOccurrence"]
        require(
            occurrence["stageNotifyIndex"] == int(suffix.rsplit("-", 1)[-1]),
            f"Light {suffix}: occurrence index drift",
        )
        require(
            occurrence["enabled"] is enabled,
            f"Light {suffix}: enabled variant drift",
        )
        require_overrides(
            cue,
            LIGHT_COMMON_EXPECTED
            + [("Color", "vector", color)]
            + LIGHT_TRAILING_EXPECTED,
        )

    summary = recipe.get("summary")
    require(isinstance(summary, dict), "recipe summary missing")
    decoded_count = sum(
        len(cue.get("typedPayload", {}).get("parameterOverrides", []))
        for cue in cues
        if str(cue.get("sourceType") or "").casefold() == "playparticleeffect"
    )
    require(
        summary.get("typedParticleParameterOverrideCount") == decoded_count,
        "typed particle parameter override summary drift",
    )
    return {
        "cueCount": len(cues),
        "sourceOccurrenceCount": len(occurrence_keys),
        "typedParticleParameterOverrideCount": decoded_count,
        "verifiedLightOccurrenceCount": len(LIGHT_OCCURRENCES),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--recipe", required=True, type=Path)
    args = parser.parse_args()
    recipe = json.loads(args.recipe.read_text(encoding="utf-8-sig"))
    print(json.dumps(validate_recipe(recipe), sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
