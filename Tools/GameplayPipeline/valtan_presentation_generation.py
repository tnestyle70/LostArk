#!/usr/bin/env python3
"""Build the immutable receipt for the Client artifacts consumed by CValtan.

The receipt is not a runtime data owner.  It hashes the existing typed Product
documents and the exact direct-authored Effect documents reached from the
generated Pattern Effect cue Product.  Gameplay.bootstrap commits the receipt
SHA so a Server gameplay revision cannot be paired with arbitrary Client
presentation bytes.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
import uuid
from dataclasses import dataclass
from pathlib import Path
from typing import Any


SCHEMA = "lostark.valtan-presentation-generation"
FORMAT_VERSION = 1
MAX_ARTIFACT_BYTES = 64 * 1024 * 1024
GENERATION_DIRECTORY = "ValtanPresentationGenerations"

BINDINGS_REL = "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
EFFECT_CUES_REL = "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
EFFECT_V1_ALIASES_REL = (
    "Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json"
)
SHAKE_CUES_REL = "Data/Animation/Authored/Valtan/Valtan.patternshakecues.json"
COMBAT_OBJECT_SOUND_CUES_REL = (
    "Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json"
)
CHARACTER_SOUND_CATALOG_REL = "Data/Sound/CharacterSoundCatalog.json"
EFFECT_CATALOG_REL = "Data/Effects/EffectCatalog.json"
EFFECT_V2_BINDINGS_REL = (
    "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"
)
EFFECT_V2_AUTHORED_ROOT_REL = "Data/Effects/V2/Authored"
EFFECT_V2_GROUP_ROOT_REL = "Data/Effects/V2/Groups"
EFFECT_V2_DOCUMENT_SUFFIX = ".effectv2.json"
EFFECT_V2_GROUP_SUFFIX = ".effectv2group.json"
ENCOUNTER_REL = "Data/Encounters/Valtan/ValtanEncounter.json"
COMBAT_OBJECTS_REL = "Data/Encounters/Valtan/ValtanCombatObjects.json"
BOSS_CATALOG_REL = "Data/Actors/BossCatalog.json"
CAMERA_REL = "Data/Encounters/Valtan/ValtanCinematicCamera.json"
WORLD_EVENTS_REL = "Data/Encounters/Valtan/ValtanWorldEvents.json"

FIXED_ARTIFACTS: tuple[tuple[str, str], ...] = (
    ("ANIMATION", BINDINGS_REL),
    ("EFFECT", EFFECT_CUES_REL),
    ("EFFECT", EFFECT_V1_ALIASES_REL),
    ("CAMERA", SHAKE_CUES_REL),
    # Pattern Sound cues intentionally remain an independently writable typed
    # source and are pinned by their own S receipt at playback admission.  The
    # catalog they resolve through and the non-live combat-object cue source
    # are immutable members of M so an exact CValtan reload cannot mix them
    # across presentation generations.
    ("COMBAT_VISUAL", COMBAT_OBJECT_SOUND_CUES_REL),
    ("COMBAT_VISUAL", CHARACTER_SOUND_CATALOG_REL),
    ("EFFECT", EFFECT_CATALOG_REL),
    ("EFFECT", EFFECT_V2_BINDINGS_REL),
    ("COMBAT_VISUAL", ENCOUNTER_REL),
    ("COMBAT_VISUAL", COMBAT_OBJECTS_REL),
    ("COMBAT_VISUAL", BOSS_CATALOG_REL),
    ("CAMERA", CAMERA_REL),
    ("WORLD_EVENT_SET", WORLD_EVENTS_REL),
)
LANES = frozenset(
    ("ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA", "WORLD_EVENT_SET")
)
LOWER_SHA256 = re.compile(r"[0-9a-f]{64}")
EFFECT_V2_ID = re.compile(r"[A-Za-z0-9_.-]{1,80}")


class PresentationGenerationError(RuntimeError):
    pass


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise PresentationGenerationError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def _canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def _sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _is_safe_relative(relative: str) -> bool:
    if (
        not relative
        or "\\" in relative
        or relative.startswith("/")
        or re.match(r"^[A-Za-z]:", relative)
    ):
        return False
    parts = relative.split("/")
    return all(part not in ("", ".", "..") for part in parts)


def _resolve_input(
    repository_root: Path,
    overlay_root: Path | None,
    relative: str,
) -> Path:
    if not _is_safe_relative(relative):
        raise PresentationGenerationError(
            f"presentation artifact path is unsafe: {relative}"
        )
    root = repository_root.resolve(strict=True)
    candidates: list[Path] = []
    if overlay_root is not None:
        candidates.append(overlay_root / relative)
    candidates.append(root / relative)
    for candidate in candidates:
        try:
            resolved = candidate.resolve(strict=True)
        except (FileNotFoundError, OSError):
            continue
        if not resolved.is_file() or resolved.is_symlink():
            continue
        allowed_root = (
            overlay_root.resolve(strict=True)
            if overlay_root is not None and candidate.is_relative_to(overlay_root)
            else root
        )
        try:
            resolved.relative_to(allowed_root)
        except ValueError as exc:
            raise PresentationGenerationError(
                f"presentation artifact escaped its input root: {relative}"
            ) from exc
        return resolved
    raise PresentationGenerationError(f"missing presentation artifact: {relative}")


def _read_json(path: Path, context: str) -> dict[str, Any]:
    data = path.read_bytes()
    if data.startswith(b"\xef\xbb\xbf"):
        raise PresentationGenerationError(f"{context} has a UTF-8 BOM")
    try:
        value = json.loads(
            data.decode("utf-8", errors="strict"),
            object_pairs_hook=_reject_duplicate_pairs,
            parse_constant=lambda token: (_ for _ in ()).throw(
                PresentationGenerationError(
                    f"{context} contains non-finite JSON: {token}"
                )
            ),
        )
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise PresentationGenerationError(f"{context} is invalid JSON") from exc
    if not isinstance(value, dict):
        raise PresentationGenerationError(f"{context} root is not an object")
    return value


@dataclass(frozen=True)
class PresentationArtifact:
    lane: str
    path: str
    sha256: str
    bytes: int
    source_path: Path

    def manifest_row(self) -> dict[str, Any]:
        return {
            "bytes": self.bytes,
            "lane": self.lane,
            "path": self.path,
            "sha256": self.sha256,
        }


@dataclass(frozen=True)
class PresentationGeneration:
    generation_id: str
    manifest_bytes: bytes
    artifacts: tuple[PresentationArtifact, ...]


def _effect_artifact_paths(
    repository_root: Path,
    overlay_root: Path | None,
) -> set[str]:
    cues_path = _resolve_input(repository_root, overlay_root, EFFECT_CUES_REL)
    catalog_path = _resolve_input(repository_root, overlay_root, EFFECT_CATALOG_REL)
    cues = _read_json(cues_path, "Valtan Pattern Effect cue Product")
    catalog = _read_json(catalog_path, "Effect catalog")
    cue_rows = cues.get("cues")
    catalog_rows = catalog.get("effects")
    if not isinstance(cue_rows, list) or not isinstance(catalog_rows, list):
        raise PresentationGenerationError(
            "Valtan Effect cue/catalog arrays are unavailable"
        )

    by_id: dict[str, dict[str, Any]] = {}
    for row in catalog_rows:
        if not isinstance(row, dict):
            raise PresentationGenerationError("Effect catalog row is not an object")
        effect_id = row.get("effectAssetId")
        if not isinstance(effect_id, str) or not effect_id or effect_id in by_id:
            raise PresentationGenerationError(
                "Effect catalog has an invalid or duplicate effectAssetId"
            )
        by_id[effect_id] = row

    result: set[str] = set()
    for cue in cue_rows:
        if not isinstance(cue, dict):
            raise PresentationGenerationError(
                "Valtan Pattern Effect cue row is not an object"
            )
        effect_id = cue.get("effectAssetId")
        if not isinstance(effect_id, str) or not effect_id:
            raise PresentationGenerationError(
                "Valtan Pattern Effect cue has no stable effectAssetId"
            )
        entry = by_id.get(effect_id)
        if entry is None or entry.get("payloadKind") != "DIRECT_AUTHORED_DOCUMENT":
            raise PresentationGenerationError(
                f"Valtan Pattern Effect cue is not direct-authored: {effect_id}"
            )
        authoring_path = entry.get("authoringPath")
        if (
            not isinstance(authoring_path, str)
            or not authoring_path.startswith("Effects/Authored/")
            or not authoring_path.endswith(".effect.json")
        ):
            raise PresentationGenerationError(
                f"Valtan Effect catalog authoringPath is invalid: {effect_id}"
            )
        relative = "Data/" + authoring_path
        if not _is_safe_relative(relative):
            raise PresentationGenerationError(
                f"Valtan Effect authoringPath is unsafe: {effect_id}"
            )
        result.add(relative)
    if not result:
        raise PresentationGenerationError(
            "Valtan Pattern Effect Product has no direct-authored closure"
        )
    return result


def _effect_v2_relative(
    root_relative: str,
    stable_id: Any,
    suffix: str,
    context: str,
) -> str:
    if not isinstance(stable_id, str) or EFFECT_V2_ID.fullmatch(stable_id) is None:
        raise PresentationGenerationError(f"{context} is not a stable Effect V2 ID")
    relative = f"{root_relative}/{stable_id}{suffix}"
    if not _is_safe_relative(relative):
        raise PresentationGenerationError(f"{context} produced an unsafe path")
    return relative


def _require_exact_properties(
    value: dict[str, Any], expected: frozenset[str], context: str
) -> None:
    if frozenset(value) != expected:
        raise PresentationGenerationError(f"{context} properties are invalid")


def _effect_v2_artifact_paths(
    repository_root: Path,
    overlay_root: Path | None,
) -> set[str]:
    """Resolve the exact BOSS_VALTAN Effect V2 binding closure.

    Effect V2 is not addressed through the direct-authored semantic Effect
    catalog.  Its runtime entry point is the strict formatVersion 2 archetype
    binding document.  Bindings and group children carry typed resource
    references and groups may nest, so walk only the reachable, acyclic
    closure.  Unrelated character/Esther authoring remains independently
    writable without changing Valtan's immutable presentation generation.
    """

    binding_path = _resolve_input(
        repository_root, overlay_root, EFFECT_V2_BINDINGS_REL
    )
    binding = _read_json(binding_path, "BOSS_VALTAN Effect V2 bindings")
    _require_exact_properties(
        binding,
        frozenset(("schema", "formatVersion", "archetypeId", "bindings")),
        "BOSS_VALTAN Effect V2 bindings",
    )
    if (
        binding.get("schema") != "lostark.effect-v2-bindings"
        or type(binding.get("formatVersion")) is not int
        or binding["formatVersion"] != 2
        or binding.get("archetypeId") != "BOSS_VALTAN"
        or not isinstance(binding.get("bindings"), list)
        or not binding["bindings"]
    ):
        raise PresentationGenerationError(
            "BOSS_VALTAN Effect V2 binding header/rows are invalid"
        )

    leaf_ids: set[str] = set()
    group_ids: set[str] = set()

    boss_catalog_path = _resolve_input(
        repository_root, overlay_root, BOSS_CATALOG_REL
    )
    boss_catalog = _read_json(boss_catalog_path, "BossCatalog Effect V2 owners")
    _require_exact_properties(
        boss_catalog,
        frozenset(("schema", "formatVersion", "bosses")),
        "BossCatalog Effect V2 owners",
    )
    if (
        boss_catalog.get("schema") != "lostark.boss-catalog"
        or type(boss_catalog.get("formatVersion")) is not int
        or boss_catalog["formatVersion"] != 8
        or not isinstance(boss_catalog.get("bosses"), list)
    ):
        raise PresentationGenerationError("BossCatalog Effect V2 owner header is invalid")
    valtan_rows = [
        row for row in boss_catalog["bosses"]
        if isinstance(row, dict) and row.get("archetypeId") == "BOSS_VALTAN"
    ]
    if len(valtan_rows) != 1 or not isinstance(
        valtan_rows[0].get("combatObjectVisuals"), list
    ):
        raise PresentationGenerationError("BossCatalog has no unique Valtan visual owner")
    for ordinal, visual in enumerate(valtan_rows[0]["combatObjectVisuals"]):
        if not isinstance(visual, dict):
            raise PresentationGenerationError(
                f"BossCatalog combatObjectVisuals[{ordinal}] is not an object"
            )
        v2_group = visual.get("effectV2Group")
        if v2_group is None:
            continue
        if not isinstance(v2_group, dict):
            raise PresentationGenerationError(
                f"BossCatalog combatObjectVisuals[{ordinal}].effectV2Group is invalid"
            )
        group_fields = frozenset(v2_group)
        base_group_fields = frozenset(("groupId", "playbackRate"))
        hit_sync_fields = frozenset(("visualHitMs", "serverHitId"))
        if group_fields not in (
            base_group_fields,
            base_group_fields | hit_sync_fields,
        ):
            raise PresentationGenerationError(
                f"BossCatalog combatObjectVisuals[{ordinal}].effectV2Group "
                "must contain either the exact presentation-only fields or "
                "the complete hit-sync pair"
            )
        _require_exact_properties(
            v2_group,
            group_fields,
            f"BossCatalog combatObjectVisuals[{ordinal}].effectV2Group",
        )
        _effect_v2_relative(
            EFFECT_V2_GROUP_ROOT_REL,
            v2_group.get("groupId"),
            EFFECT_V2_GROUP_SUFFIX,
            f"BossCatalog combatObjectVisuals[{ordinal}].effectV2Group.groupId",
        )
        group_ids.add(v2_group["groupId"])

    binding_ids: set[str] = set()
    previous_binding_id = ""
    for ordinal, row in enumerate(binding["bindings"]):
        context = f"BOSS_VALTAN Effect V2 bindings[{ordinal}]"
        if not isinstance(row, dict):
            raise PresentationGenerationError(f"{context} is not an object")
        _require_exact_properties(
            row,
            frozenset(("bindingId", "resource", "scope", "clock", "anchor", "stopPolicy")),
            context,
        )
        binding_id = row["bindingId"]
        if (
            not isinstance(binding_id, str)
            or EFFECT_V2_ID.fullmatch(binding_id) is None
            or binding_id in binding_ids
            or (previous_binding_id and previous_binding_id >= binding_id)
        ):
            raise PresentationGenerationError(
                f"{context}.bindingId must be unique and ordinally sorted"
            )
        binding_ids.add(binding_id)
        previous_binding_id = binding_id
        resource = row["resource"]
        if not isinstance(resource, dict):
            raise PresentationGenerationError(f"{context}.resource is not an object")
        _require_exact_properties(resource, frozenset(("kind", "id")), f"{context}.resource")
        kind = resource["kind"]
        resource_id = resource["id"]
        if kind not in ("LEAF", "GROUP"):
            raise PresentationGenerationError(f"{context}.resource.kind is invalid")
        _effect_v2_relative(
            EFFECT_V2_AUTHORED_ROOT_REL if kind == "LEAF" else EFFECT_V2_GROUP_ROOT_REL,
            resource_id,
            EFFECT_V2_DOCUMENT_SUFFIX if kind == "LEAF" else EFFECT_V2_GROUP_SUFFIX,
            f"{context}.resource.id",
        )
        (leaf_ids if kind == "LEAF" else group_ids).add(resource_id)

    collisions = leaf_ids & group_ids
    if collisions:
        raise PresentationGenerationError(
            "BOSS_VALTAN Effect V2 IDs collide between a leaf and group: "
            + sorted(collisions)[0]
        )

    result: set[str] = set()
    group_edges: dict[str, set[str]] = {}
    pending_groups = list(sorted(group_ids))
    visited_groups: set[str] = set()
    while pending_groups:
        group_id = pending_groups.pop(0)
        if group_id in visited_groups:
            continue
        visited_groups.add(group_id)
        relative = _effect_v2_relative(
            EFFECT_V2_GROUP_ROOT_REL,
            group_id,
            EFFECT_V2_GROUP_SUFFIX,
            "BOSS_VALTAN Effect V2 groupId",
        )
        group_path = _resolve_input(repository_root, overlay_root, relative)
        group = _read_json(group_path, f"BOSS_VALTAN Effect V2 group {group_id}")
        _require_exact_properties(
            group,
            frozenset(("schema", "formatVersion", "groupId", "durationMs", "children")),
            f"BOSS_VALTAN Effect V2 group {group_id}",
        )
        if (
            group.get("schema") != "lostark.effect-v2-group"
            or type(group.get("formatVersion")) is not int
            or group["formatVersion"] != 2
            or group.get("groupId") != group_id
            or not isinstance(group.get("children"), list)
            or not group["children"]
        ):
            raise PresentationGenerationError(
                f"BOSS_VALTAN Effect V2 group identity/children are invalid: {group_id}"
            )
        result.add(relative)
        group_edges[group_id] = set()
        child_ids: set[str] = set()
        for ordinal, child in enumerate(group["children"]):
            context = f"BOSS_VALTAN Effect V2 group {group_id}.children[{ordinal}]"
            if not isinstance(child, dict):
                raise PresentationGenerationError(f"{context} is not an object")
            _require_exact_properties(
                child,
                frozenset(("childId", "resource", "startMs", "durationMs", "stop", "localTransform")),
                context,
            )
            child_id = child["childId"]
            if (
                not isinstance(child_id, str)
                or EFFECT_V2_ID.fullmatch(child_id) is None
                or child_id in child_ids
            ):
                raise PresentationGenerationError(f"{context}.childId is invalid or duplicated")
            child_ids.add(child_id)
            resource = child["resource"]
            if not isinstance(resource, dict):
                raise PresentationGenerationError(f"{context}.resource is not an object")
            _require_exact_properties(resource, frozenset(("kind", "id")), f"{context}.resource")
            kind = resource["kind"]
            resource_id = resource["id"]
            if kind not in ("LEAF", "GROUP"):
                raise PresentationGenerationError(f"{context}.resource.kind is invalid")
            _effect_v2_relative(
                EFFECT_V2_AUTHORED_ROOT_REL if kind == "LEAF" else EFFECT_V2_GROUP_ROOT_REL,
                resource_id,
                EFFECT_V2_DOCUMENT_SUFFIX if kind == "LEAF" else EFFECT_V2_GROUP_SUFFIX,
                f"{context}.resource.id",
            )
            if kind == "LEAF":
                leaf_ids.add(resource_id)
            else:
                group_ids.add(resource_id)
                group_edges[group_id].add(resource_id)
                pending_groups.append(resource_id)

    visiting: set[str] = set()
    visited: set[str] = set()

    def visit_group(group_id: str) -> None:
        if group_id in visiting:
            raise PresentationGenerationError(
                f"BOSS_VALTAN Effect V2 group cycle is invalid: {group_id}"
            )
        if group_id in visited:
            return
        visiting.add(group_id)
        for child_group_id in sorted(group_edges.get(group_id, set())):
            visit_group(child_group_id)
        visiting.remove(group_id)
        visited.add(group_id)

    for group_id in sorted(group_ids):
        visit_group(group_id)

    for effect_id in sorted(leaf_ids):
        relative = _effect_v2_relative(
            EFFECT_V2_AUTHORED_ROOT_REL,
            effect_id,
            EFFECT_V2_DOCUMENT_SUFFIX,
            "BOSS_VALTAN Effect V2 leaf effectId",
        )
        document_path = _resolve_input(repository_root, overlay_root, relative)
        document = _read_json(
            document_path, f"BOSS_VALTAN Effect V2 leaf {effect_id}"
        )
        _require_exact_properties(
            document,
            frozenset(
                (
                    "schema",
                    "formatVersion",
                    "effectId",
                    "effectType",
                    "slots",
                    "params",
                    "parts",
                )
            ),
            f"BOSS_VALTAN Effect V2 leaf {effect_id}",
        )
        if (
            document.get("schema") != "lostark.effect-v2"
            or type(document.get("formatVersion")) is not int
            or document["formatVersion"] != 1
            or document.get("effectId") != effect_id
        ):
            raise PresentationGenerationError(
                f"BOSS_VALTAN Effect V2 leaf identity is invalid: {effect_id}"
            )
        result.add(relative)
    return result


def build_presentation_generation(
    repository_root: Path,
    overlay_root: Path | None = None,
) -> PresentationGeneration:
    repository_root = repository_root.resolve(strict=True)
    if overlay_root is not None:
        overlay_root = overlay_root.resolve(strict=True)

    rows: list[tuple[str, str]] = list(FIXED_ARTIFACTS)
    rows.extend(
        ("EFFECT", relative)
        for relative in _effect_artifact_paths(repository_root, overlay_root)
    )
    rows.extend(
        ("EFFECT", relative)
        for relative in _effect_v2_artifact_paths(repository_root, overlay_root)
    )
    if len({relative for _, relative in rows}) != len(rows):
        raise PresentationGenerationError(
            "Valtan presentation artifact closure contains duplicate paths"
        )

    artifacts: list[PresentationArtifact] = []
    for lane, relative in sorted(rows, key=lambda item: (item[1], item[0])):
        if lane not in LANES:
            raise PresentationGenerationError(
                f"Valtan presentation artifact lane is invalid: {lane}"
            )
        source_path = _resolve_input(repository_root, overlay_root, relative)
        data = source_path.read_bytes()
        if not data or len(data) > MAX_ARTIFACT_BYTES:
            raise PresentationGenerationError(
                f"Valtan presentation artifact size is invalid: {relative}"
            )
        digest = _sha256(data)
        if LOWER_SHA256.fullmatch(digest) is None:
            raise PresentationGenerationError(
                f"Valtan presentation artifact SHA-256 is invalid: {relative}"
            )
        artifacts.append(
            PresentationArtifact(lane, relative, digest, len(data), source_path)
        )

    manifest = {
        "artifacts": [artifact.manifest_row() for artifact in artifacts],
        "formatVersion": FORMAT_VERSION,
        "schema": SCHEMA,
    }
    manifest_bytes = _canonical_bytes(manifest)
    return PresentationGeneration(
        generation_id=_sha256(manifest_bytes),
        manifest_bytes=manifest_bytes,
        artifacts=tuple(artifacts),
    )


def publish_generation_manifest(
    generation: PresentationGeneration,
    output_root: Path,
) -> Path:
    output_root = output_root.resolve()
    directory = output_root / GENERATION_DIRECTORY
    directory.mkdir(parents=True, exist_ok=True)
    destination = directory / f"{generation.generation_id}.json"
    if destination.exists():
        if destination.is_symlink() or destination.read_bytes() != generation.manifest_bytes:
            raise PresentationGenerationError(
                "presentation generation ID collides with different bytes"
            )
        return destination

    temporary = directory / (
        f".{generation.generation_id}.stage.{uuid.uuid4().hex}"
    )
    try:
        with temporary.open("xb") as output:
            output.write(generation.manifest_bytes)
            output.flush()
            os.fsync(output.fileno())
        try:
            os.rename(temporary, destination)
        except FileExistsError:
            if destination.is_symlink() or destination.read_bytes() != generation.manifest_bytes:
                raise PresentationGenerationError(
                    "presentation generation race produced different bytes"
                )
    finally:
        try:
            temporary.unlink(missing_ok=True)
        except OSError:
            pass
    return destination


def _parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repository-root", type=Path, required=True)
    parser.add_argument("--input-overlay-root", type=Path)
    parser.add_argument("--output-root", type=Path)
    parser.add_argument("--mode", choices=("Validate", "Publish"), default="Validate")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = _parse_args(sys.argv[1:] if argv is None else argv)
    try:
        generation = build_presentation_generation(
            args.repository_root, args.input_overlay_root
        )
        manifest_path: Path | None = None
        if args.mode == "Publish":
            if args.output_root is None:
                raise PresentationGenerationError(
                    "Publish requires --output-root"
                )
            manifest_path = publish_generation_manifest(
                generation, args.output_root
            )
        summary = {
            "artifactCount": len(generation.artifacts),
            "generationId": generation.generation_id,
            "manifestPath": "" if manifest_path is None else str(manifest_path),
        }
        sys.stdout.write(_canonical_bytes(summary).decode("utf-8") + "\n")
        return 0
    except (OSError, PresentationGenerationError) as exc:
        sys.stderr.write(f"Valtan presentation generation failed: {exc}\n")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
