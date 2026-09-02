#!/usr/bin/env python3
"""Read the native Valtan animation timing contract from its product WModels.

This module is an authoring validator, not a runtime or generated Product owner.
``Data/Actors/BossCatalog.json`` names the body and attached animation-set
assets, while the WModel animation headers own each clip's native duration.
Callers can use the resulting composite inventory before committing an edited
``Valtan.presentation.json`` source revision.
"""

from __future__ import annotations

import hashlib
import json
import math
import re
import struct
import sys
from collections.abc import Mapping
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from types import MappingProxyType
from typing import Any

try:
    from Tools.ModelAssetConverter.retime_wmodel_from_psa import (
        read_wmodel_animation_sections,
    )
except ModuleNotFoundError:
    # ``valtan_tuning_pipeline.py`` is also invoked directly from this folder.
    # Keep that supported entry point while sharing the one strict WModel parser.
    _REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
    if str(_REPOSITORY_ROOT) not in sys.path:
        sys.path.insert(0, str(_REPOSITORY_ROOT))
    from Tools.ModelAssetConverter.retime_wmodel_from_psa import (
        read_wmodel_animation_sections,
    )


BOSS_CATALOG_REL = "Data/Actors/BossCatalog.json"
DEFAULT_BOSS_ARCHETYPE_ID = "BOSS_VALTAN"
STABLE_CLIP_NAME = re.compile(r"^[A-Za-z0-9_.-]{1,160}$")


class NativeAnimationInventoryError(RuntimeError):
    """The product model composition or one authored source window is invalid."""


@dataclass(frozen=True)
class NativeAnimationSource:
    asset_id: str
    path: Path
    sha256: str
    byte_count: int
    clip_count: int


@dataclass(frozen=True)
class NativeClipTiming:
    clip_name: str
    source_asset_id: str
    source_animation_index: int
    duration_ticks: float
    ticks_per_second: float
    native_duration_ms: float
    rounded_native_duration_ms: int


@dataclass(frozen=True)
class ValtanCompositeAnimationInventory:
    boss_archetype_id: str
    sources: tuple[NativeAnimationSource, ...]
    clips: Mapping[str, NativeClipTiming]


@dataclass(frozen=True)
class NativeSourceWindow:
    clip: NativeClipTiming
    source_start_ms: int
    play_ms: int
    play_rate: float
    rounded_remaining_source_ms: int


@dataclass(frozen=True)
class NativePresentationValidation:
    occurrence_count: int
    unique_clip_count: int
    native_remainder_occurrence_count: int


def positive_half_away_from_zero(value: float) -> int:
    """Match the existing Valtan promotion receipt's positive-ms policy."""

    if not math.isfinite(value) or value < 0.0:
        raise NativeAnimationInventoryError(
            f"cannot round an invalid native duration: {value}"
        )
    return int(math.floor(value + 0.5))


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise NativeAnimationInventoryError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def _read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(
            path.read_text(encoding="utf-8-sig"),
            object_pairs_hook=_reject_duplicate_pairs,
        )
    except NativeAnimationInventoryError:
        raise
    except (OSError, UnicodeError, json.JSONDecodeError) as exc:
        raise NativeAnimationInventoryError(f"cannot read {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise NativeAnimationInventoryError(f"JSON root must be an object: {path}")
    return value


def _resolve_resource_asset(resource_root: Path, asset_id: Any) -> tuple[str, Path]:
    if not isinstance(asset_id, str) or not asset_id:
        raise NativeAnimationInventoryError("BossCatalog model asset ID is missing")
    posix = PurePosixPath(asset_id)
    if (
        "\\" in asset_id
        or ":" in asset_id
        or posix.is_absolute()
        or any(part in ("", ".", "..") for part in posix.parts)
        or posix.suffix.lower() != ".wmodel"
    ):
        raise NativeAnimationInventoryError(
            f"BossCatalog model asset ID is unsafe: {asset_id}"
        )
    resolved_root = resource_root.resolve()
    path = (resolved_root / Path(*posix.parts)).resolve()
    if not path.is_relative_to(resolved_root):
        raise NativeAnimationInventoryError(
            f"BossCatalog model asset escapes Resources: {asset_id}"
        )
    if not path.is_file():
        raise NativeAnimationInventoryError(
            f"required Valtan WModel is unavailable: {asset_id}"
        )
    return asset_id, path


def _boss_model_asset_ids(
    boss_catalog: Mapping[str, Any], boss_archetype_id: str
) -> tuple[str, str]:
    if (
        boss_catalog.get("schema") != "lostark.boss-catalog"
        or boss_catalog.get("formatVersion") != 6
        or not isinstance(boss_catalog.get("bosses"), list)
    ):
        raise NativeAnimationInventoryError("BossCatalog header is invalid")
    matches = [
        row
        for row in boss_catalog["bosses"]
        if isinstance(row, dict) and row.get("archetypeId") == boss_archetype_id
    ]
    if len(matches) != 1:
        raise NativeAnimationInventoryError(
            f"BossCatalog must contain exactly one {boss_archetype_id}"
        )
    boss = matches[0]
    body = boss.get("bodyModel")
    animation_set = boss.get("animationSetId")
    if not isinstance(body, str) or not isinstance(animation_set, str):
        raise NativeAnimationInventoryError(
            f"{boss_archetype_id} must name bodyModel and animationSetId"
        )
    return body, animation_set


def build_valtan_composite_animation_inventory(
    boss_catalog: Mapping[str, Any],
    resource_root: Path,
    *,
    boss_archetype_id: str = DEFAULT_BOSS_ARCHETYPE_ID,
) -> ValtanCompositeAnimationInventory:
    """Build the exact body-then-AnimSet clip table used by ``CModel``.

    Duplicate clip names are rejected because ``CModel::Attach_AnimationSet``
    rejects that composition instead of choosing one duration implicitly.
    """

    body_asset_id, animation_set_asset_id = _boss_model_asset_ids(
        boss_catalog, boss_archetype_id
    )
    sources: list[NativeAnimationSource] = []
    clips: dict[str, NativeClipTiming] = {}
    for asset_id_value in (body_asset_id, animation_set_asset_id):
        asset_id, path = _resolve_resource_asset(resource_root, asset_id_value)
        try:
            payload = path.read_bytes()
            rows = read_wmodel_animation_sections(payload)
        except (OSError, UnicodeError, ValueError, struct.error) as exc:
            raise NativeAnimationInventoryError(
                f"cannot parse native animation inventory {asset_id}: {exc}"
            ) from exc
        if not rows:
            raise NativeAnimationInventoryError(
                f"native animation inventory is empty: {asset_id}"
            )
        sources.append(
            NativeAnimationSource(
                asset_id=asset_id,
                path=path,
                sha256=hashlib.sha256(payload).hexdigest(),
                byte_count=len(payload),
                clip_count=len(rows),
            )
        )
        for row in rows:
            clip_name = row.get("name")
            duration_ticks = row.get("durationTicks")
            ticks_per_second = row.get("ticksPerSecond")
            animation_index = row.get("index")
            if (
                not isinstance(clip_name, str)
                or STABLE_CLIP_NAME.fullmatch(clip_name) is None
                or not isinstance(animation_index, int)
                or isinstance(animation_index, bool)
                or not isinstance(duration_ticks, (int, float))
                or isinstance(duration_ticks, bool)
                or not isinstance(ticks_per_second, (int, float))
                or isinstance(ticks_per_second, bool)
            ):
                raise NativeAnimationInventoryError(
                    f"native animation metadata is invalid in {asset_id}"
                )
            duration_ticks = float(duration_ticks)
            ticks_per_second = float(ticks_per_second)
            if (
                not math.isfinite(duration_ticks)
                or duration_ticks <= 0.0
                or not math.isfinite(ticks_per_second)
                or ticks_per_second <= 0.0
            ):
                raise NativeAnimationInventoryError(
                    f"native animation timing is invalid: {asset_id}/{clip_name}"
                )
            native_duration_ms = duration_ticks / ticks_per_second * 1000.0
            rounded_native_duration_ms = positive_half_away_from_zero(
                native_duration_ms
            )
            if rounded_native_duration_ms <= 0:
                raise NativeAnimationInventoryError(
                    f"native animation duration rounds empty: {asset_id}/{clip_name}"
                )
            if clip_name in clips:
                previous = clips[clip_name]
                raise NativeAnimationInventoryError(
                    "CModel::Attach_AnimationSet duplicate clip: "
                    f"{clip_name} ({previous.source_asset_id}, {asset_id})"
                )
            clips[clip_name] = NativeClipTiming(
                clip_name=clip_name,
                source_asset_id=asset_id,
                source_animation_index=animation_index,
                duration_ticks=duration_ticks,
                ticks_per_second=ticks_per_second,
                native_duration_ms=native_duration_ms,
                rounded_native_duration_ms=rounded_native_duration_ms,
            )
    return ValtanCompositeAnimationInventory(
        boss_archetype_id=boss_archetype_id,
        sources=tuple(sources),
        clips=MappingProxyType(clips),
    )


def load_valtan_composite_animation_inventory(
    repo_root: Path,
    *,
    resource_root: Path | None = None,
    boss_archetype_id: str = DEFAULT_BOSS_ARCHETYPE_ID,
) -> ValtanCompositeAnimationInventory:
    resolved_repo = repo_root.resolve()
    catalog = _read_json(resolved_repo / BOSS_CATALOG_REL)
    return build_valtan_composite_animation_inventory(
        catalog,
        resource_root or resolved_repo / "Client/Bin/Resources",
        boss_archetype_id=boss_archetype_id,
    )


def validate_native_source_window(
    inventory: ValtanCompositeAnimationInventory,
    *,
    clip_name: Any,
    source_start_ms: Any,
    play_ms: Any,
    play_rate: Any,
    context: str = "animation occurrence",
) -> NativeSourceWindow:
    if not isinstance(clip_name, str):
        raise NativeAnimationInventoryError(f"{context}.clip must be a string")
    clip = inventory.clips.get(clip_name)
    if clip is None:
        raise NativeAnimationInventoryError(
            f"{context}.clip is absent from the product model composition: {clip_name}"
        )
    for field_name, value in (
        ("sourceStartMs", source_start_ms),
        ("playMs", play_ms),
    ):
        if not isinstance(value, int) or isinstance(value, bool) or value < 0:
            raise NativeAnimationInventoryError(
                f"{context}.{field_name} must be a non-negative integer"
            )
    if (
        not isinstance(play_rate, (int, float))
        or isinstance(play_rate, bool)
        or not math.isfinite(float(play_rate))
        or float(play_rate) <= 0.0
    ):
        raise NativeAnimationInventoryError(
            f"{context}.playRate must be finite and positive"
        )
    if float(source_start_ms) >= clip.native_duration_ms:
        raise NativeAnimationInventoryError(
            f"{context}.sourceStartMs escapes native clip {clip_name}: "
            f"{source_start_ms} >= {clip.native_duration_ms:.6f} ms"
        )
    rounded_remaining_ms = positive_half_away_from_zero(
        clip.native_duration_ms - float(source_start_ms)
    )
    if play_ms != 0 and play_ms > rounded_remaining_ms:
        raise NativeAnimationInventoryError(
            f"{context}.playMs exceeds native clip remainder {clip_name}: "
            f"{play_ms} > {rounded_remaining_ms} ms"
        )
    return NativeSourceWindow(
        clip=clip,
        source_start_ms=source_start_ms,
        play_ms=play_ms,
        play_rate=float(play_rate),
        rounded_remaining_source_ms=rounded_remaining_ms,
    )


def validate_valtan_presentation_native_windows(
    presentation: Mapping[str, Any],
    inventory: ValtanCompositeAnimationInventory,
) -> NativePresentationValidation:
    if not isinstance(presentation.get("patterns"), list):
        raise NativeAnimationInventoryError(
            "Valtan presentation patterns must be an array"
        )
    occurrence_count = 0
    native_remainder_count = 0
    used_clips: set[str] = set()
    for pattern_ordinal, pattern in enumerate(presentation["patterns"]):
        if not isinstance(pattern, dict) or not isinstance(pattern.get("stages"), list):
            raise NativeAnimationInventoryError(
                f"presentation pattern[{pattern_ordinal}] is malformed"
            )
        pattern_id = pattern.get("patternId", f"pattern[{pattern_ordinal}]")
        for stage_ordinal, stage in enumerate(pattern["stages"]):
            if not isinstance(stage, dict) or not isinstance(stage.get("animation"), dict):
                raise NativeAnimationInventoryError(
                    f"{pattern_id}.stage[{stage_ordinal}].animation is malformed"
                )
            stage_id = stage.get("stageId", f"stage[{stage_ordinal}]")
            animation = stage["animation"]
            mode = animation.get("mode", "CLIP_SEQUENCE")
            if mode == "NONE":
                if animation.get("occurrences") not in (None, []):
                    raise NativeAnimationInventoryError(
                        f"{pattern_id}/{stage_id} NONE animation owns occurrences"
                    )
                continue
            if mode != "CLIP_SEQUENCE" or not isinstance(
                animation.get("occurrences"), list
            ):
                raise NativeAnimationInventoryError(
                    f"{pattern_id}/{stage_id} animation mode/occurrences are invalid"
                )
            for occurrence_ordinal, occurrence in enumerate(
                animation["occurrences"]
            ):
                context = (
                    f"{pattern_id}/{stage_id}.occurrences[{occurrence_ordinal}]"
                )
                if not isinstance(occurrence, dict):
                    raise NativeAnimationInventoryError(f"{context} is malformed")
                if not isinstance(occurrence.get("repeatUntilStageEnd"), bool):
                    raise NativeAnimationInventoryError(
                        f"{context}.repeatUntilStageEnd must be Boolean"
                    )
                window = validate_native_source_window(
                    inventory,
                    clip_name=occurrence.get("clip"),
                    source_start_ms=occurrence.get("sourceStartMs"),
                    play_ms=occurrence.get("playMs"),
                    play_rate=occurrence.get("playRate"),
                    context=context,
                )
                occurrence_count += 1
                native_remainder_count += int(window.play_ms == 0)
                used_clips.add(window.clip.clip_name)
    return NativePresentationValidation(
        occurrence_count=occurrence_count,
        unique_clip_count=len(used_clips),
        native_remainder_occurrence_count=native_remainder_count,
    )
