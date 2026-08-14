#!/usr/bin/env python3
"""Build the generic source-runtime admission index for combat BA Effects.

The index is an evidence join, not a runtime payload.  It joins gameplay LMB
COMBO definitions, authored clip order, asset Effect cues, source-stage
manifests, Imported renderer rows, approximation occurrence receipts, current
legacy product components, and an asset-scoped import basis.  Every source row
receives an explicit typed disposition.  Unknown, stale, duplicate, or missing
joins fail before the output is atomically replaced.

This module deliberately does not reuse the Artist 31470 reconstructed runtime
specialization and does not mutate EffectCatalog, Authored Effects, assemblies,
components, or runtime resources.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
import re
import tempfile
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


SCRIPT_PATH = Path(__file__).resolve()
SCRIPT_ROOT = SCRIPT_PATH.parent
REPOSITORY_ROOT = SCRIPT_ROOT.parent.parent

SCHEMA_RELATIVE_PATH = (
    "Tools/EffectPipeline/Schemas/"
    "lostark.effect-source-runtime-admission-index.schema.json"
)
DEFAULT_BASIS_RELATIVE_PATH = (
    "Data/Effects/Contracts/effect-import-bases.v1.json"
)
DEFAULT_OUTPUT_RELATIVE_PATH = (
    "Data/Effects/Imported/CombatBA/"
    "lmb-combo-3class.v1.runtime-admission.json"
)

INDEX_SCHEMA = "lostark.effect-source-runtime-admission-index"
INDEX_FORMAT_VERSION = 1
BASIS_SCHEMA = "lostark.effect-import-basis-contract"
BASIS_FORMAT_VERSION = 1
SOURCE_MANIFEST_SCHEMA = "lostark.combat-effect-source-stage-manifest"
SOURCE_MANIFEST_VERSION = 1
AUTHORING_SCHEMA = "lostark.effect-authoring"
ASSEMBLY_SCHEMA = "lostark.effect-assembly"
COMPONENT_SCHEMA = "lostark.effect-component"
APPROXIMATION_RECEIPT_SCHEMA = "lostark.effect-authored-approximation-receipt"

DEFAULT_EXPECTED_STAGE_COUNT = 12
DEFAULT_EXPECTED_OCCURRENCE_COUNT = 35
DEFAULT_EXPECTED_SOURCE_ROW_COUNT = 133
DEFAULT_EXPECTED_COMPONENT_COUNT = 20
DEFAULT_EXPECTED_LEGACY_EMITTER_COUNT = 66

SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
STABLE_ID_RE = re.compile(r"^[A-Za-z0-9_.:-]+$")
PRODUCT_EFFECT_RE = re.compile(
    r"^effect\.(?P<class_id>[a-z0-9]+)\.skill\."
    r"(?P<skill_id>[1-9][0-9]*)\.ba(?P<stage>[1-9][0-9]*)$"
)
ANIMEVENT_RE = re.compile(r'^\s*"(?P<clip>[^"]+)"\s+EFFECT\b(?P<body>.*)$')
ATTRIBUTE_RE = re.compile(r'(?P<name>[A-Za-z][A-Za-z0-9_]*)=(?P<value>"[^"]*"|\S+)')

COMMON_BASIS_BLOCKERS = (
    "ASSET_ROTATION_BASIS_UNPROVEN",
    "ATTACHMENT_ROOT_BASIS_UNPROVEN",
    "MESH_CARRIER_AXIS_BASIS_UNPROVEN",
)
COMMON_RUNTIME_BLOCKERS = (
    "OCCURRENCE_VF_PASS_BINDING_UNPROVEN",
    "SOURCE_RUNTIME_PROGRAM_NOT_PUBLISHED",
)

RENDERER_FAMILY = {
    ("particle", "mesh"): "MESH_PARTICLE",
    ("particle", "sprite"): "SPRITE_PARTICLE",
    ("particle", "ribbon"): "CASCADE_RIBBON",
    ("particle", "trail"): "CASCADE_RIBBON",
    ("decal", "decal"): "DECAL_PARTICLE",
    ("light", "light"): "LIGHT_PARTICLE",
    ("screenPost", "screenPost"): "SCREEN_POST",
}

DISPOSITIONS = frozenset(
    {
        "NATIVE_EXACT",
        "TYPED_RECONSTRUCTED",
        "LEGACY_APPROXIMATION",
        "INTENTIONAL_NONPRODUCT",
        "FAIL_CLOSED",
    }
)


class ContractError(ValueError):
    """Raised when an input cannot participate in the all-or-nothing join."""


def _reject_non_finite(value: str) -> None:
    raise ContractError(f"non-finite JSON number is forbidden: {value}")


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(
        path.read_text(encoding="utf-8-sig"),
        parse_constant=_reject_non_finite,
    )
    if not isinstance(value, dict):
        raise ContractError(f"JSON root must be an object: {path}")
    return value


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_json_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(
            value,
            ensure_ascii=False,
            indent=2,
            allow_nan=False,
        )
        + "\n"
    ).encode("utf-8")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def text_sha256_variants(path: Path) -> set[str]:
    raw = path.read_bytes()
    text = raw.decode("utf-8-sig")
    normalized_lf = text.replace("\r\n", "\n").replace("\r", "\n").encode("utf-8")
    return {
        hashlib.sha256(raw).hexdigest(),
        hashlib.sha256(normalized_lf).hexdigest(),
        hashlib.sha256(normalized_lf.replace(b"\n", b"\r\n")).hexdigest(),
    }


def _require_object(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ContractError(f"{label} must be an object")
    return value


def _require_array(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise ContractError(f"{label} must be an array")
    return value


def _require_string(value: Any, label: str, *, allow_empty: bool = False) -> str:
    if not isinstance(value, str) or (not allow_empty and not value):
        raise ContractError(f"{label} must be a non-empty string")
    return value


def _require_int(value: Any, label: str, *, minimum: int | None = None) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise ContractError(f"{label} must be an integer")
    if minimum is not None and value < minimum:
        raise ContractError(f"{label} must be >= {minimum}")
    return value


def _require_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ContractError(f"{label} must be numeric")
    result = float(value)
    if not math.isfinite(result):
        raise ContractError(f"{label} must be finite")
    return result


def _require_bool(value: Any, label: str) -> bool:
    if not isinstance(value, bool):
        raise ContractError(f"{label} must be boolean")
    return value


def _require_sha256(value: Any, label: str) -> str:
    result = _require_string(value, label)
    if not SHA256_RE.fullmatch(result):
        raise ContractError(f"{label} must be a lowercase SHA-256")
    return result


def _require_stable_id(value: Any, label: str) -> str:
    result = _require_string(value, label)
    if not STABLE_ID_RE.fullmatch(result):
        raise ContractError(f"{label} is not a stable ID: {result}")
    return result


def _require_exact_keys(value: dict[str, Any], keys: Iterable[str], label: str) -> None:
    expected = set(keys)
    actual = set(value)
    if actual != expected:
        missing = sorted(expected - actual)
        extra = sorted(actual - expected)
        raise ContractError(
            f"{label} fields mismatch; missing={missing}, extra={extra}"
        )


def _safe_relative_path(value: Any, label: str) -> str:
    text = _require_string(value, label)
    if "\\" in text or ":" in text or text.startswith("/"):
        raise ContractError(f"{label} must be repository-relative POSIX: {text}")
    path = PurePosixPath(text)
    if any(part in ("", ".", "..") for part in path.parts):
        raise ContractError(f"{label} contains an unsafe segment: {text}")
    return text


def _resolve_repository_path(root: Path, value: Any, label: str) -> Path:
    relative = _safe_relative_path(value, label)
    resolved_root = root.resolve()
    resolved = (resolved_root / Path(*PurePosixPath(relative).parts)).resolve()
    try:
        resolved.relative_to(resolved_root)
    except ValueError as error:
        raise ContractError(f"{label} escaped repository root: {relative}") from error
    if not resolved.is_file():
        raise ContractError(f"{label} does not exist: {relative}")
    return resolved


def _repository_relative(root: Path, path: Path) -> str:
    try:
        return path.resolve().relative_to(root.resolve()).as_posix()
    except ValueError as error:
        raise ContractError(f"path is outside repository root: {path}") from error


def _validate_resource_asset_id(value: Any, label: str) -> str:
    text = _safe_relative_path(value, label)
    if not text.startswith("Effect/"):
        raise ContractError(f"{label} must be Effect-relative: {text}")
    return text


class InputTracker:
    def __init__(self, repository_root: Path) -> None:
        self.repository_root = repository_root.resolve()
        self._records: dict[str, dict[str, Any]] = {}

    def _record(self, path: Path, role: str, canonical_sha256: str | None) -> None:
        relative = _repository_relative(self.repository_root, path)
        raw_sha256 = sha256_file(path)
        existing = self._records.get(relative)
        if existing is None:
            existing = {
                "path": relative,
                "rawSha256": raw_sha256,
                "canonicalJsonSha256": canonical_sha256,
                "roles": set(),
            }
            self._records[relative] = existing
        elif existing["rawSha256"] != raw_sha256:
            raise ContractError(f"input changed while staging: {relative}")
        elif existing["canonicalJsonSha256"] != canonical_sha256:
            raise ContractError(f"input JSON identity changed while staging: {relative}")
        existing["roles"].add(role)

    def json(self, path: Path, role: str) -> dict[str, Any]:
        value = load_json(path)
        self._record(path, role, canonical_json_sha256(value))
        return value

    def text(self, path: Path, role: str) -> str:
        value = path.read_text(encoding="utf-8-sig")
        self._record(path, role, None)
        return value

    def records(self) -> list[dict[str, Any]]:
        result: list[dict[str, Any]] = []
        for relative in sorted(self._records):
            record = copy.deepcopy(self._records[relative])
            record["roles"] = sorted(record["roles"])
            result.append(record)
        return result


@dataclass(frozen=True)
class BasisProfile:
    basis_id: str
    animation_asset_id: str
    character_class: str
    source_path: Path
    source_relative_path: str
    source_sha256: str
    asset_basis: dict[str, Any]


@dataclass
class StageContext:
    basis: BasisProfile
    skill_id: int
    stage_index: int
    stage_id: str
    clip: str
    timeline_offset_seconds: float
    duration_seconds: float
    source_event_ids: list[str]
    effect_asset_id: str
    manifest_stage: dict[str, Any]
    receipt_path: Path
    receipt: dict[str, Any]
    current_product: dict[str, Any]
    current_product_elements: dict[str, dict[str, Any]]
    occurrence_records: list[dict[str, Any]]
    source_row_ids: list[str]


def _matrix(value: Any, label: str) -> list[list[float]]:
    rows = _require_array(value, label)
    if len(rows) != 3:
        raise ContractError(f"{label} must have three rows")
    result: list[list[float]] = []
    for row_index, raw_row in enumerate(rows):
        row = _require_array(raw_row, f"{label}[{row_index}]")
        if len(row) != 3:
            raise ContractError(f"{label}[{row_index}] must have three columns")
        result.append(
            [
                _require_number(item, f"{label}[{row_index}][{column_index}]")
                for column_index, item in enumerate(row)
            ]
        )
    return result


def _validate_basis_contract(
    repository_root: Path,
    tracker: InputTracker,
    basis_path: Path,
) -> tuple[dict[str, Any], list[BasisProfile]]:
    contract = tracker.json(basis_path, "IMPORT_BASIS_CONTRACT")
    _require_exact_keys(
        contract,
        (
            "schema",
            "formatVersion",
            "contractId",
            "contractRole",
            "runtimeAdmission",
            "coordinateSourceContract",
            "commonCoordinateBasis",
            "profiles",
            "contractSha256",
        ),
        "import basis contract",
    )
    if contract["schema"] != BASIS_SCHEMA or contract["formatVersion"] != BASIS_FORMAT_VERSION:
        raise ContractError("import basis contract header mismatch")
    _require_stable_id(contract["contractId"], "import basis contractId")
    if contract["contractRole"] != "ASSET_SCOPED_EVIDENCE_NOT_RUNTIME_AUTHORITY":
        raise ContractError("import basis contract role mismatch")
    if _require_bool(contract["runtimeAdmission"], "import basis runtimeAdmission"):
        raise ContractError("first-slice import basis cannot grant runtime admission")
    expected_contract_sha = _require_sha256(
        contract["contractSha256"], "import basis contractSha256"
    )
    hash_payload = copy.deepcopy(contract)
    del hash_payload["contractSha256"]
    if canonical_json_sha256(hash_payload) != expected_contract_sha:
        raise ContractError("import basis contractSha256 is stale")

    source_contract_ref = _require_object(
        contract["coordinateSourceContract"], "coordinateSourceContract"
    )
    _require_exact_keys(
        source_contract_ref,
        ("path", "sha256", "profileId"),
        "coordinateSourceContract",
    )
    source_contract_path = _resolve_repository_path(
        repository_root,
        source_contract_ref["path"],
        "coordinateSourceContract.path",
    )
    source_contract = tracker.json(source_contract_path, "COORDINATE_SOURCE_CONTRACT")
    if sha256_file(source_contract_path) != _require_sha256(
        source_contract_ref["sha256"], "coordinateSourceContract.sha256"
    ):
        raise ContractError("coordinate source contract raw SHA is stale")
    if source_contract.get("profileId") != source_contract_ref["profileId"]:
        raise ContractError("coordinate source contract profileId mismatch")

    coordinate_rules = {
        item.get("ruleId"): item
        for item in _require_array(
            source_contract.get("coordinateAndScaleRules"),
            "coordinate source rules",
        )
        if isinstance(item, dict)
    }
    expected_rule_ids = {
        "ue3.position_cm_to_client_m",
        "ue3.velocity_cm_per_s_to_client_m_per_s",
        "ue3.acceleration_cm_per_s2_to_client_m_per_s2",
        "sprite_particle.size_cm_to_m",
        "decal_particle.size_cm_to_m",
        "mesh_particle.dimensionless_axis_reorder",
        "wmodel.carrier_geometry_prescale",
    }
    if not expected_rule_ids.issubset(coordinate_rules):
        raise ContractError("coordinate source contract no longer covers the import basis")

    common = _require_object(contract["commonCoordinateBasis"], "commonCoordinateBasis")
    _require_exact_keys(
        common,
        (
            "position",
            "velocity",
            "acceleration",
            "spriteAndDecalSize",
            "meshParticleDimension",
        ),
        "commonCoordinateBasis",
    )
    expected_vectors = {
        "position": (
            "centimeter",
            "meter",
            0.01,
            [[1.0, 0.0, 0.0], [0.0, 0.0, 1.0], [0.0, -1.0, 0.0]],
        ),
        "velocity": (
            "centimeter_per_second",
            "meter_per_second",
            0.01,
            [[1.0, 0.0, 0.0], [0.0, 0.0, 1.0], [0.0, -1.0, 0.0]],
        ),
        "acceleration": (
            "centimeter_per_second_squared",
            "meter_per_second_squared",
            0.01,
            [[1.0, 0.0, 0.0], [0.0, 0.0, 1.0], [0.0, -1.0, 0.0]],
        ),
        "spriteAndDecalSize": (
            "centimeter",
            "meter",
            0.01,
            [[1.0, 0.0, 0.0], [0.0, 0.0, 1.0], [0.0, 1.0, 0.0]],
        ),
    }
    for name, (source_unit, target_unit, scale, axis_matrix) in expected_vectors.items():
        row = _require_object(common[name], f"commonCoordinateBasis.{name}")
        _require_exact_keys(
            row,
            ("sourceUnit", "targetUnit", "unitScale", "axisMatrix"),
            f"commonCoordinateBasis.{name}",
        )
        if (
            row["sourceUnit"] != source_unit
            or row["targetUnit"] != target_unit
            or _require_number(row["unitScale"], f"{name}.unitScale") != scale
            or _matrix(row["axisMatrix"], f"{name}.axisMatrix") != axis_matrix
        ):
            raise ContractError(f"commonCoordinateBasis.{name} changed without source evidence")
    mesh_basis = _require_object(
        common["meshParticleDimension"], "commonCoordinateBasis.meshParticleDimension"
    )
    _require_exact_keys(
        mesh_basis,
        (
            "sourceUnit",
            "targetUnit",
            "unitScale",
            "axisMatrix",
            "carrierGeometryPreScale",
        ),
        "commonCoordinateBasis.meshParticleDimension",
    )
    if (
        mesh_basis["sourceUnit"] != "dimensionless_multiplier"
        or mesh_basis["targetUnit"] != "dimensionless_multiplier"
        or _require_number(mesh_basis["unitScale"], "mesh unitScale") != 1.0
        or _matrix(mesh_basis["axisMatrix"], "mesh axisMatrix")
        != [[1.0, 0.0, 0.0], [0.0, 0.0, 1.0], [0.0, 1.0, 0.0]]
        or _require_number(
            mesh_basis["carrierGeometryPreScale"], "carrierGeometryPreScale"
        )
        != 0.01
    ):
        raise ContractError("mesh particle common basis changed without source evidence")

    profiles: list[BasisProfile] = []
    basis_ids: set[str] = set()
    asset_ids: set[str] = set()
    character_classes: set[str] = set()
    for profile_index, raw_profile in enumerate(
        _require_array(contract["profiles"], "import basis profiles")
    ):
        profile = _require_object(raw_profile, f"profiles[{profile_index}]")
        _require_exact_keys(
            profile,
            (
                "basisId",
                "animationAssetId",
                "characterClass",
                "sourceRevision",
                "assetBasis",
                "runtimeAdmission",
            ),
            f"profiles[{profile_index}]",
        )
        basis_id = _require_stable_id(profile["basisId"], "basisId")
        animation_asset_id = _require_stable_id(
            profile["animationAssetId"], "animationAssetId"
        )
        character_class = _require_stable_id(
            profile["characterClass"], "characterClass"
        )
        if basis_id in basis_ids or animation_asset_id in asset_ids or character_class in character_classes:
            raise ContractError("duplicate import basis identity")
        basis_ids.add(basis_id)
        asset_ids.add(animation_asset_id)
        character_classes.add(character_class)
        if _require_bool(profile["runtimeAdmission"], f"{basis_id}.runtimeAdmission"):
            raise ContractError(f"{basis_id} cannot grant first-slice runtime admission")

        revision = _require_object(profile["sourceRevision"], f"{basis_id}.sourceRevision")
        _require_exact_keys(revision, ("path", "sha256"), f"{basis_id}.sourceRevision")
        source_relative = _safe_relative_path(
            revision["path"], f"{basis_id}.sourceRevision.path"
        )
        source_path = _resolve_repository_path(
            repository_root,
            source_relative,
            f"{basis_id}.sourceRevision.path",
        )
        source_sha256 = _require_sha256(
            revision["sha256"], f"{basis_id}.sourceRevision.sha256"
        )
        if sha256_file(source_path) != source_sha256:
            raise ContractError(f"{basis_id} source revision SHA is stale")

        asset_basis = _require_object(profile["assetBasis"], f"{basis_id}.assetBasis")
        _require_exact_keys(
            asset_basis,
            (
                "status",
                "sourceToRuntimeRotationDegrees",
                "meshCarrierAxisRotationDegrees",
                "attachmentRootRotationDegrees",
                "evidenceArtifacts",
                "blockers",
            ),
            f"{basis_id}.assetBasis",
        )
        status = _require_string(asset_basis["status"], f"{basis_id}.assetBasis.status")
        evidence = _require_array(
            asset_basis["evidenceArtifacts"], f"{basis_id}.evidenceArtifacts"
        )
        blockers = _require_array(asset_basis["blockers"], f"{basis_id}.blockers")
        if status == "UNRESOLVED_FAIL_CLOSED":
            for field in (
                "sourceToRuntimeRotationDegrees",
                "meshCarrierAxisRotationDegrees",
                "attachmentRootRotationDegrees",
            ):
                if asset_basis[field] is not None:
                    raise ContractError(
                        f"{basis_id}.{field} cannot carry an unproven numeric correction"
                    )
            if evidence:
                raise ContractError(f"{basis_id} unresolved basis cannot claim evidence")
            if sorted(blockers) != sorted(COMMON_BASIS_BLOCKERS):
                raise ContractError(f"{basis_id} unresolved blocker set mismatch")
        elif status == "SOURCE_EVIDENCE_PINNED":
            if not evidence:
                raise ContractError(f"{basis_id} pinned basis requires evidence artifacts")
            for field in (
                "sourceToRuntimeRotationDegrees",
                "meshCarrierAxisRotationDegrees",
                "attachmentRootRotationDegrees",
            ):
                vector = _require_array(asset_basis[field], f"{basis_id}.{field}")
                if len(vector) != 3:
                    raise ContractError(f"{basis_id}.{field} must have three values")
                for component_index, component in enumerate(vector):
                    _require_number(component, f"{basis_id}.{field}[{component_index}]")
        else:
            raise ContractError(f"unsupported asset basis status: {status}")

        profiles.append(
            BasisProfile(
                basis_id=basis_id,
                animation_asset_id=animation_asset_id,
                character_class=character_class,
                source_path=source_path,
                source_relative_path=source_relative,
                source_sha256=source_sha256,
                asset_basis=copy.deepcopy(asset_basis),
            )
        )
    if not profiles:
        raise ContractError("import basis contract has no asset profiles")
    return contract, sorted(profiles, key=lambda item: item.character_class)


def _clip_name(value: Any, label: str) -> str:
    if isinstance(value, str):
        return _require_string(value, label)
    item = _require_object(value, label)
    return _require_string(item.get("clip"), f"{label}.clip")


def _binding_stages(value: Any, label: str) -> list[list[str]]:
    clips = _require_array(value, label)
    if not clips:
        raise ContractError(f"{label} cannot be empty")
    nested = [isinstance(item, list) for item in clips]
    if any(nested) and not all(nested):
        raise ContractError(f"{label} mixes flat clips and combo stages")
    if all(nested):
        result: list[list[str]] = []
        for stage_index, raw_stage in enumerate(clips):
            stage = _require_array(raw_stage, f"{label}[{stage_index}]")
            if not stage:
                raise ContractError(f"{label}[{stage_index}] cannot be empty")
            result.append(
                [
                    _clip_name(item, f"{label}[{stage_index}][{clip_index}]")
                    for clip_index, item in enumerate(stage)
                ]
            )
        return result
    return [[_clip_name(item, f"{label}[{index}]") for index, item in enumerate(clips)]]


def _parse_asset_effect_rows(text: str) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for line_number, line in enumerate(text.splitlines(), start=1):
        match = ANIMEVENT_RE.match(line)
        if not match:
            continue
        attributes: dict[str, str] = {}
        for attribute in ATTRIBUTE_RE.finditer(match.group("body")):
            raw_value = attribute.group("value")
            if raw_value.startswith('"') and raw_value.endswith('"'):
                raw_value = raw_value[1:-1]
            attributes[attribute.group("name")] = raw_value
        if attributes.get("effectref") != "asset":
            continue
        payload = attributes.get("payload", "")
        if not payload:
            raise ContractError(f"asset Effect row has no payload at line {line_number}")
        rows.append(
            {
                "lineNumber": line_number,
                "clip": match.group("clip"),
                "payload": payload,
                "startms": attributes.get("startms"),
                "anchor": attributes.get("anchor"),
                "follow": attributes.get("follow"),
                "stop": attributes.get("stop"),
                "transform": {
                    key: attributes.get(key)
                    for key in ("px", "py", "pz", "rx", "ry", "rz", "sx", "sy", "sz")
                },
            }
        )
    return rows


def _class_effect_id(character_class: str) -> str:
    return character_class.lower().replace("_", "")


def _source_row_id(
    character_class: str,
    skill_id: int,
    source_element_id: str,
    source_element_sha256: str,
) -> str:
    identity = {
        "characterClass": character_class,
        "skillId": skill_id,
        "sourceElementId": source_element_id,
        "sourceElementSha256": source_element_sha256,
    }
    return f"source-row.{canonical_json_sha256(identity)}"


def _find_unique(values: Iterable[Any], predicate: Any, label: str) -> Any:
    matches = [item for item in values if predicate(item)]
    if len(matches) != 1:
        raise ContractError(f"{label} expected exactly one row, found {len(matches)}")
    return matches[0]


def _component_family_counts(emitters: list[Any], label: str) -> dict[str, int]:
    counter: Counter[str] = Counter()
    for index, raw_emitter in enumerate(emitters):
        emitter = _require_object(raw_emitter, f"{label}.emitters[{index}]")
        renderer = _require_string(
            emitter.get("renderer"), f"{label}.emitters[{index}].renderer"
        )
        if renderer not in ("mesh", "sprite"):
            raise ContractError(f"legacy component has unsupported renderer: {renderer}")
        counter[renderer] += 1
    return {key: counter[key] for key in sorted(counter)}


def _build_current_product(
    repository_root: Path,
    tracker: InputTracker,
    catalog_entry: dict[str, Any],
    basis: BasisProfile,
    skill_id: int,
    stage_index: int,
    effect_asset_id: str,
) -> tuple[dict[str, Any], dict[str, dict[str, Any]]]:
    authoring_relative = _safe_relative_path(
        catalog_entry.get("authoringPath"), f"{effect_asset_id}.authoringPath"
    )
    if not authoring_relative.startswith("Effects/Authored/"):
        raise ContractError(f"legacy BA authoring path is outside Effects/Authored: {effect_asset_id}")
    authoring_path = _resolve_repository_path(
        repository_root,
        f"Data/{authoring_relative}",
        f"{effect_asset_id}.authoringPath",
    )
    authoring = tracker.json(authoring_path, "CURRENT_LEGACY_AUTHORING")
    if (
        authoring.get("schema") != AUTHORING_SCHEMA
        or authoring.get("effectAssetId") != effect_asset_id
    ):
        raise ContractError(f"legacy authoring identity mismatch: {effect_asset_id}")
    elements = _require_array(authoring.get("elements"), f"{effect_asset_id}.elements")
    elements_by_id: dict[str, dict[str, Any]] = {}
    for index, raw_element in enumerate(elements):
        element = _require_object(raw_element, f"{effect_asset_id}.elements[{index}]")
        element_id = _require_stable_id(element.get("id"), f"{effect_asset_id}.elements[{index}].id")
        if element_id in elements_by_id:
            raise ContractError(f"duplicate legacy product Element ID: {effect_asset_id}/{element_id}")
        recipe = _require_object(
            element.get("sourceRecipe"), f"{effect_asset_id}/{element_id}.sourceRecipe"
        )
        if recipe.get("enabled") is not False:
            raise ContractError(
                f"current BA product must remain a standalone legacy projection: {effect_asset_id}/{element_id}"
            )
        elements_by_id[element_id] = element

    assembly_relative = (
        f"Data/Effects/Assemblies/{basis.animation_asset_id}/"
        f"{effect_asset_id}.assembly.json"
    )
    assembly_path = _resolve_repository_path(
        repository_root, assembly_relative, f"{effect_asset_id}.assembly"
    )
    assembly = tracker.json(assembly_path, "CURRENT_LEGACY_ASSEMBLY")
    if (
        assembly.get("schema") != ASSEMBLY_SCHEMA
        or assembly.get("version") != 1
        or assembly.get("effectAssetId") != effect_asset_id
    ):
        raise ContractError(f"legacy Assembly identity mismatch: {effect_asset_id}")
    if assembly.get("sourceDocumentFileSha256") not in text_sha256_variants(authoring_path):
        raise ContractError(f"legacy Assembly source document SHA is stale: {effect_asset_id}")

    component_directory = (
        repository_root
        / "Data"
        / "Effects"
        / "Components"
        / basis.animation_asset_id
        / f"skill.{skill_id}.ba{stage_index + 1}"
    )
    if not component_directory.is_dir():
        raise ContractError(f"missing legacy component directory: {component_directory}")
    component_documents: dict[str, tuple[Path, dict[str, Any]]] = {}
    for component_path in sorted(component_directory.glob("*.visual.wfx.json")):
        component = tracker.json(component_path, "CURRENT_LEGACY_COMPONENT")
        component_id = _require_stable_id(
            component.get("componentAssetId"), f"{component_path}.componentAssetId"
        )
        if component_id in component_documents:
            raise ContractError(f"duplicate componentAssetId: {component_id}")
        component_documents[component_id] = (component_path, component)

    component_cues: list[dict[str, Any]] = []
    component_element_ids: set[str] = set()
    for cue_index, raw_cue in enumerate(
        _require_array(assembly.get("componentCues"), f"{effect_asset_id}.componentCues")
    ):
        cue = _require_object(raw_cue, f"{effect_asset_id}.componentCues[{cue_index}]")
        component_id = _require_stable_id(
            cue.get("componentAssetId"),
            f"{effect_asset_id}.componentCues[{cue_index}].componentAssetId",
        )
        if component_id not in component_documents:
            raise ContractError(f"missing legacy component: {effect_asset_id}/{component_id}")
        component_path, component = component_documents[component_id]
        if (
            component.get("schema") != COMPONENT_SCHEMA
            or component.get("version") != 1
            or component.get("componentAssetId") != component_id
        ):
            raise ContractError(f"legacy component identity mismatch: {component_id}")
        document = _require_object(component.get("document"), f"{component_id}.document")
        document_elements = _require_array(document.get("elements"), f"{component_id}.document.elements")
        emitters = _require_array(component.get("emitters"), f"{component_id}.emitters")
        if len(document_elements) != len(emitters):
            raise ContractError(f"component Emitter/Element count mismatch: {component_id}")
        emitter_ids = {
            _require_stable_id(
                _require_object(item, f"{component_id}.emitters[{index}]").get("elementId"),
                f"{component_id}.emitters[{index}].elementId",
            )
            for index, item in enumerate(emitters)
        }
        document_ids = {
            _require_stable_id(
                _require_object(item, f"{component_id}.elements[{index}]").get("id"),
                f"{component_id}.elements[{index}].id",
            )
            for index, item in enumerate(document_elements)
        }
        if emitter_ids != document_ids:
            raise ContractError(f"component Emitter/Element identity mismatch: {component_id}")
        overlap = component_element_ids.intersection(document_ids)
        if overlap:
            raise ContractError(f"duplicate Element across components: {sorted(overlap)}")
        component_element_ids.update(document_ids)
        local_transform = _require_object(
            cue.get("localTransform"), f"{effect_asset_id}/{component_id}.localTransform"
        )
        component_cues.append(
            {
                "cueId": _require_stable_id(cue.get("cueId"), f"{component_id}.cueId"),
                "componentAssetId": component_id,
                "componentPath": _repository_relative(repository_root, component_path),
                "componentRawSha256": sha256_file(component_path),
                "startDelaySeconds": _require_number(
                    cue.get("startDelaySeconds"), f"{component_id}.startDelaySeconds"
                ),
                "anchor": _require_string(cue.get("anchor"), f"{component_id}.anchor"),
                "localTransformSha256": canonical_json_sha256(local_transform),
                "emitterCount": len(emitters),
                "rendererCounts": _component_family_counts(emitters, component_id),
            }
        )
    if component_element_ids != set(elements_by_id):
        raise ContractError(f"Assembly component coverage mismatch: {effect_asset_id}")

    product = {
        "payloadKind": "LEGACY_ASSEMBLY_V1",
        "authoringPath": _repository_relative(repository_root, authoring_path),
        "authoringRawSha256": sha256_file(authoring_path),
        "assemblyPath": _repository_relative(repository_root, assembly_path),
        "assemblyRawSha256": sha256_file(assembly_path),
        "componentCues": component_cues,
        "componentCount": len(component_cues),
        "emitterCount": len(elements),
        "sourceRecipeEnabledEmitterCount": 0,
    }
    return product, elements_by_id


def _renderer_family(element: dict[str, Any], label: str) -> str:
    kind = _require_string(element.get("kind"), f"{label}.kind")
    recipe = _require_object(element.get("sourceRecipe"), f"{label}.sourceRecipe")
    shape = _require_string(recipe.get("rendererShape"), f"{label}.rendererShape")
    family = RENDERER_FAMILY.get((kind, shape))
    if family is None:
        raise ContractError(f"unsupported renderer family {kind}/{shape}: {label}")
    return family


def _resource_bindings(element: dict[str, Any], label: str) -> list[dict[str, str]]:
    result: list[dict[str, str]] = []
    seen_slots: set[str] = set()
    for index, raw_resource in enumerate(
        _require_array(element.get("resources"), f"{label}.resources")
    ):
        resource = _require_object(raw_resource, f"{label}.resources[{index}]")
        slot_id = _require_stable_id(
            resource.get("slotId"), f"{label}.resources[{index}].slotId"
        )
        if slot_id in seen_slots:
            raise ContractError(f"duplicate resource slot: {label}/{slot_id}")
        seen_slots.add(slot_id)
        result.append(
            {
                "slotId": slot_id,
                "assetId": _validate_resource_asset_id(
                    resource.get("assetId"), f"{label}.resources[{index}].assetId"
                ),
            }
        )
    return result


def _material_identity(element: dict[str, Any], label: str) -> dict[str, Any]:
    material = _require_object(element.get("material"), f"{label}.material")
    source_profile = _require_object(
        material.get("sourceProfile"), f"{label}.material.sourceProfile"
    )
    enabled = _require_bool(
        source_profile.get("enabled"), f"{label}.material.sourceProfile.enabled"
    )
    static_switches = source_profile.get("staticSwitches", [])
    textures = source_profile.get("textures", [])
    if not isinstance(static_switches, list) or not isinstance(textures, list):
        raise ContractError(f"{label} material profile arrays are invalid")
    profile_id = source_profile.get("profileId") if enabled else None
    runtime_profile_id = source_profile.get("runtimeShaderProfileId") if enabled else None
    if enabled:
        _require_stable_id(profile_id, f"{label}.material.profileId")
        _require_stable_id(runtime_profile_id, f"{label}.material.runtimeShaderProfileId")
    recipe_sha256 = canonical_json_sha256(material)
    return {
        "materialRecipeId": f"material-recipe.{recipe_sha256[:16]}",
        "materialRecipeSha256": recipe_sha256,
        "sourceMaterialPath": _require_string(
            material.get("sourceMaterialPath"), f"{label}.material.sourceMaterialPath"
        ),
        "renderProfile": _require_stable_id(
            material.get("renderProfile"), f"{label}.material.renderProfile"
        ),
        "sourceProfileEnabled": enabled,
        "materialFamilyId": profile_id,
        "runtimeShaderProfileId": runtime_profile_id,
        "staticSetSha256": canonical_json_sha256(static_switches),
        "textureEvidenceSha256": canonical_json_sha256(textures),
        "shaderBinding": {
            "status": (
                "RECONSTRUCTED_PROFILE_WITHOUT_OCCURRENCE_VF_PASS"
                if enabled
                else "SOURCE_PROFILE_DISABLED"
            ),
            "vfId": None,
            "passIndex": None,
            "staticSetSha256": canonical_json_sha256(static_switches),
        },
    }


def _source_recipe_identity(element: dict[str, Any], label: str) -> dict[str, Any]:
    recipe = _require_object(element.get("sourceRecipe"), f"{label}.sourceRecipe")
    if _require_bool(recipe.get("enabled"), f"{label}.sourceRecipe.enabled") is not True:
        raise ContractError(f"Imported sourceRecipe must be enabled: {label}")
    modules = _require_array(recipe.get("modules"), f"{label}.sourceRecipe.modules")
    stable_ids: list[str] = []
    seen: set[str] = set()
    for index, raw_module in enumerate(modules):
        module = _require_object(raw_module, f"{label}.sourceRecipe.modules[{index}]")
        stable_id = _require_string(
            module.get("stableId"), f"{label}.sourceRecipe.modules[{index}].stableId"
        )
        if stable_id in seen:
            raise ContractError(f"duplicate source module stable ID: {label}/{stable_id}")
        seen.add(stable_id)
        stable_ids.append(stable_id)
    return {
        "enabled": True,
        "rendererShape": _require_string(
            recipe.get("rendererShape"), f"{label}.sourceRecipe.rendererShape"
        ),
        "sourceRecipeSha256": canonical_json_sha256(recipe),
        "moduleClosureSha256": canonical_json_sha256(modules),
        "moduleCount": len(modules),
        "moduleStableIds": stable_ids,
    }


def _attachment_identity(
    element: dict[str, Any], basis: BasisProfile, label: str
) -> dict[str, Any]:
    attachment = _require_object(
        element.get("actionCueAttachment"), f"{label}.actionCueAttachment"
    )
    return {
        "attachmentSha256": canonical_json_sha256(attachment),
        "enabled": _require_bool(attachment.get("enabled"), f"{label}.attachment.enabled"),
        "follow": _require_bool(attachment.get("follow"), f"{label}.attachment.follow"),
        "sourceAnchorSlotId": _require_string(
            attachment.get("sourceAnchorSlotId", ""),
            f"{label}.attachment.sourceAnchorSlotId",
            allow_empty=True,
        ),
        "runtimeAnchorSlotId": _require_string(
            attachment.get("runtimeAnchorSlotId", ""),
            f"{label}.attachment.runtimeAnchorSlotId",
            allow_empty=True,
        ),
        "runtimeBoneName": _require_string(
            attachment.get("runtimeBoneName", ""),
            f"{label}.attachment.runtimeBoneName",
            allow_empty=True,
        ),
        "importBasisId": basis.basis_id,
        "assetBasisStatus": basis.asset_basis["status"],
    }


def _stage_from_timeline(
    stages: list[StageContext], source_time: float, label: str
) -> StageContext:
    candidates = [
        stage
        for stage in stages
        if source_time + 1e-6 >= stage.timeline_offset_seconds
        and source_time
        <= stage.timeline_offset_seconds + stage.duration_seconds + 1e-6
    ]
    if not candidates:
        raise ContractError(f"{label} is outside every source stage: {source_time}")
    return max(candidates, key=lambda item: item.timeline_offset_seconds)


def _stage_join_for_non_particle(
    element: dict[str, Any],
    stages: list[StageContext],
    event_to_stage: dict[str, StageContext],
    label: str,
) -> tuple[StageContext, dict[str, Any]]:
    presentation = _require_object(
        element.get("sourcePresentation"), f"{label}.sourcePresentation"
    )
    source_event_id = _require_string(
        presentation.get("sourceEventId", ""),
        f"{label}.sourcePresentation.sourceEventId",
        allow_empty=True,
    )
    if source_event_id:
        stage = event_to_stage.get(source_event_id)
        if stage is None:
            raise ContractError(f"non-particle source event has no stage: {label}/{source_event_id}")
        return stage, {
            "kind": "SOURCE_EVENT_ID",
            "sourceEventId": source_event_id,
            "sourceTimelineSeconds": _require_number(
                presentation.get("sourceTimeSeconds"),
                f"{label}.sourcePresentation.sourceTimeSeconds",
            ),
        }
    detail = _require_object(element.get("detail"), f"{label}.detail")
    timing = _require_object(detail.get("timing"), f"{label}.detail.timing")
    source_time = _require_number(
        timing.get("startDelaySeconds"), f"{label}.detail.timing.startDelaySeconds"
    )
    stage = _stage_from_timeline(stages, source_time, label)
    return stage, {
        "kind": "SOURCE_TIMELINE_INTERVAL",
        "sourceEventId": None,
        "sourceTimelineSeconds": source_time,
    }


def _source_presentation_fields(element: dict[str, Any], label: str) -> dict[str, Any]:
    presentation = _require_object(
        element.get("sourcePresentation"), f"{label}.sourcePresentation"
    )
    source_event_id = presentation.get("sourceEventId", "")
    source_action_cue_id = presentation.get("sourceActionCueId", "")
    occurrence_index = presentation.get("sourceOccurrenceIndex")
    if source_event_id == "":
        source_event_id = None
    elif not isinstance(source_event_id, str):
        raise ContractError(f"{label}.sourceEventId must be a string")
    if source_action_cue_id == "":
        source_action_cue_id = None
    elif not isinstance(source_action_cue_id, str):
        raise ContractError(f"{label}.sourceActionCueId must be a string")
    if occurrence_index is not None:
        occurrence_index = _require_int(
            occurrence_index, f"{label}.sourceOccurrenceIndex", minimum=0
        )
    return {
        "sourceEventId": source_event_id,
        "sourceActionCueId": source_action_cue_id,
        "sourcePresentationOccurrenceIndex": occurrence_index,
    }


def _build_source_row(
    element: dict[str, Any],
    source_order: int,
    stage: StageContext,
    stage_join: dict[str, Any],
    particle_occurrence_id: str | None,
    candidate: dict[str, Any] | None,
) -> dict[str, Any]:
    source_element_id = _require_string(element.get("id"), "source element id")
    source_element_sha256 = canonical_json_sha256(element)
    row_id = _source_row_id(
        stage.basis.character_class,
        stage.skill_id,
        source_element_id,
        source_element_sha256,
    )
    family = _renderer_family(element, source_element_id)
    resources = _resource_bindings(element, source_element_id)
    source_recipe = _source_recipe_identity(element, source_element_id)
    material = _material_identity(element, source_element_id)
    presentation = _source_presentation_fields(element, source_element_id)

    legacy_decision = "notApplicable"
    target_element_id: str | None = None
    target_element_sha256: str | None = None
    rejection_reason: str | None = None
    evidence_reason_codes: list[str] = []
    disposition = "FAIL_CLOSED"
    disposition_blockers = list(COMMON_BASIS_BLOCKERS + COMMON_RUNTIME_BLOCKERS)
    if candidate is not None:
        legacy_decision = _require_string(
            candidate.get("selectionDecision"),
            f"{source_element_id}.selectionDecision",
        )
        evidence_reason_codes = sorted(
            {
                _require_string(item, f"{source_element_id}.reasonCodes")
                for item in candidate.get("reasonCodes", [])
            }
        )
        rejection_reason_value = candidate.get("rejectionReason")
        if rejection_reason_value is not None:
            rejection_reason = _require_string(
                rejection_reason_value, f"{source_element_id}.rejectionReason"
            )
        if legacy_decision == "selected":
            disposition = "LEGACY_APPROXIMATION"
            target_element_id = _require_stable_id(
                candidate.get("targetElementId"), f"{source_element_id}.targetElementId"
            )
            target_element = stage.current_product_elements.get(target_element_id)
            if target_element is None:
                raise ContractError(
                    f"selected source row has no current product Element: {source_element_id}/{target_element_id}"
                )
            target_element_sha256 = canonical_json_sha256(target_element)
            disposition_blockers.append("SOURCE_RECIPE_DISABLED_IN_LEGACY_PRODUCT")
        elif legacy_decision == "rejected":
            disposition_blockers.append("LEGACY_CARRIER_REJECTED")
        elif legacy_decision == "budgetExcluded":
            disposition_blockers.append("LEGACY_BUDGET_EXCLUDED")
        else:
            raise ContractError(
                f"unsupported approximation selectionDecision: {legacy_decision}"
            )
    else:
        family_blocker = {
            "DECAL_PARTICLE": "DECAL_TYPED_RUNTIME_ADAPTER_NOT_ADMITTED",
            "LIGHT_PARTICLE": "LIGHT_TYPED_RUNTIME_ADAPTER_NOT_ADMITTED",
            "SCREEN_POST": "SCREEN_POST_TYPED_RUNTIME_ADAPTER_NOT_ADMITTED",
            "CASCADE_RIBBON": "RIBBON_TYPED_RUNTIME_ADAPTER_NOT_ADMITTED",
        }.get(family, "SOURCE_FAMILY_NOT_PRESENT_IN_LEGACY_PRODUCT")
        disposition_blockers.append(family_blocker)

    if not material["sourceProfileEnabled"]:
        disposition_blockers.append("TYPED_MATERIAL_PROFILE_NOT_ADMITTED")
    if disposition not in DISPOSITIONS:
        raise ContractError(f"unknown typed disposition: {disposition}")

    return {
        "sourceRowId": row_id,
        "characterClass": stage.basis.character_class,
        "animationAssetId": stage.basis.animation_asset_id,
        "skillId": stage.skill_id,
        "stageIndex": stage.stage_index,
        "stageId": stage.stage_id,
        "clip": stage.clip,
        "productEffectAssetId": stage.effect_asset_id,
        "sourceElementId": source_element_id,
        "sourceElementSha256": source_element_sha256,
        "sourceOrder": source_order,
        "sourceKind": _require_string(element.get("kind"), f"{source_element_id}.kind"),
        "rendererFamily": family,
        **presentation,
        "particleOccurrenceId": particle_occurrence_id,
        "stageJoin": stage_join,
        "importBasisId": stage.basis.basis_id,
        "attachment": _attachment_identity(element, stage.basis, source_element_id),
        "sourceRecipe": source_recipe,
        "material": material,
        "resourceBindings": resources,
        "resourceBindingsSha256": canonical_json_sha256(resources),
        "legacyProjection": {
            "selectionDecision": legacy_decision,
            "targetElementId": target_element_id,
            "targetElementSha256": target_element_sha256,
            "rejectionReason": rejection_reason,
            "evidenceReasonCodes": evidence_reason_codes,
        },
        "admission": {
            "disposition": disposition,
            "sourceRuntimeAdmission": False,
            "sourceProductAdmission": False,
            "preservesCurrentLegacyProduct": legacy_decision == "selected",
            "blockers": sorted(set(disposition_blockers)),
        },
    }


def _validate_schema_document(schema: dict[str, Any]) -> None:
    if schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
        raise ContractError("admission-index schema draft mismatch")
    if schema.get("title") != "LostArk Effect source-runtime admission index":
        raise ContractError("admission-index schema title mismatch")
    root_properties = _require_object(schema.get("properties"), "schema.properties")
    if root_properties.get("schema", {}).get("const") != INDEX_SCHEMA:
        raise ContractError("admission-index schema identity mismatch")
    if root_properties.get("formatVersion", {}).get("const") != INDEX_FORMAT_VERSION:
        raise ContractError("admission-index schema version mismatch")


def build_index(
    repository_root: Path = REPOSITORY_ROOT,
    basis_path: Path | None = None,
    *,
    expected_stage_count: int = DEFAULT_EXPECTED_STAGE_COUNT,
    expected_occurrence_count: int = DEFAULT_EXPECTED_OCCURRENCE_COUNT,
    expected_source_row_count: int = DEFAULT_EXPECTED_SOURCE_ROW_COUNT,
    expected_component_count: int = DEFAULT_EXPECTED_COMPONENT_COUNT,
    expected_legacy_emitter_count: int = DEFAULT_EXPECTED_LEGACY_EMITTER_COUNT,
) -> dict[str, Any]:
    repository_root = repository_root.resolve()
    if basis_path is None:
        basis_path = repository_root / Path(*PurePosixPath(DEFAULT_BASIS_RELATIVE_PATH).parts)
    else:
        basis_path = basis_path.resolve()
    tracker = InputTracker(repository_root)

    schema_path = repository_root / Path(*PurePosixPath(SCHEMA_RELATIVE_PATH).parts)
    if not schema_path.is_file():
        raise ContractError(f"missing admission-index schema: {schema_path}")
    schema_document = tracker.json(schema_path, "OUTPUT_SCHEMA")
    _validate_schema_document(schema_document)
    basis_contract, basis_profiles = _validate_basis_contract(
        repository_root, tracker, basis_path
    )

    player_skills_path = repository_root / "Data" / "Balance" / "PlayerSkills.json"
    player_skills = tracker.json(player_skills_path, "GAMEPLAY_SKILL_CATALOG")
    player_rows = _require_array(player_skills.get("skills"), "PlayerSkills.skills")
    effect_catalog_path = repository_root / "Data" / "Effects" / "EffectCatalog.json"
    effect_catalog = tracker.json(effect_catalog_path, "CURRENT_EFFECT_CATALOG")
    catalog_rows = _require_array(effect_catalog.get("effects"), "EffectCatalog.effects")
    catalog_by_id: dict[str, dict[str, Any]] = {}
    for index, raw_entry in enumerate(catalog_rows):
        entry = _require_object(raw_entry, f"EffectCatalog.effects[{index}]")
        effect_id = _require_stable_id(
            entry.get("effectAssetId"), f"EffectCatalog.effects[{index}].effectAssetId"
        )
        if effect_id in catalog_by_id:
            raise ContractError(f"duplicate EffectCatalog ID: {effect_id}")
        catalog_by_id[effect_id] = entry

    stage_contexts: list[StageContext] = []
    imported_by_class: dict[str, tuple[dict[str, Any], dict[str, dict[str, Any]]]] = {}
    candidate_binding_by_element_id: dict[tuple[str, str], dict[str, Any]] = {}

    for basis in basis_profiles:
        imported = tracker.json(basis.source_path, "IMPORTED_SOURCE_DOCUMENT")
        imported_effect_id = _require_stable_id(
            imported.get("effectAssetId"), f"{basis.basis_id}.Imported effectAssetId"
        )
        imported_id_match = re.fullmatch(
            rf"effect\.{re.escape(_class_effect_id(basis.character_class))}\."
            r"skill\.(?P<skill_id>[1-9][0-9]*)\.imported",
            imported_effect_id,
        )
        if imported_id_match is None:
            raise ContractError(
                "import basis source revision has an incompatible Effect ID: "
                f"{imported_effect_id}"
            )
        source_skill_id = int(imported_id_match.group("skill_id"))
        matching_skills = [
            _require_object(row, "PlayerSkills row")
            for row in player_rows
            if isinstance(row, dict)
            and row.get("skillId") == source_skill_id
            and row.get("characterClass") == basis.character_class
            and row.get("inputSlot") == "LMB"
            and row.get("skillKind") == "COMBO"
        ]
        if len(matching_skills) != 1:
            raise ContractError(
                f"{basis.character_class} must have exactly one LMB COMBO skill; found {len(matching_skills)}"
            )
        player_skill = matching_skills[0]
        skill_id = _require_int(player_skill.get("skillId"), "PlayerSkills.skillId", minimum=1)
        combo_stages = _require_array(
            player_skill.get("comboStages"), f"PlayerSkills[{skill_id}].comboStages"
        )
        if len(combo_stages) != 4:
            raise ContractError(f"LMB COMBO closure requires four gameplay stages: {skill_id}")

        binding_path = (
            repository_root
            / "Data"
            / "Animation"
            / "Authored"
            / basis.animation_asset_id
            / f"{basis.animation_asset_id}.skillbindings.json"
        )
        bindings = tracker.json(binding_path, "AUTHORED_SKILL_BINDINGS")
        if (
            bindings.get("animationAssetId") != basis.animation_asset_id
            or bindings.get("characterClass") != basis.character_class
        ):
            raise ContractError(f"skill binding identity mismatch: {basis.animation_asset_id}")
        binding = _find_unique(
            _require_array(bindings.get("bindings"), f"{basis.animation_asset_id}.bindings"),
            lambda item: isinstance(item, dict) and item.get("skillId") == skill_id,
            f"{basis.character_class}/{skill_id} authored binding",
        )
        binding_stage_clips = _binding_stages(
            binding.get("clips"), f"{basis.character_class}/{skill_id}.clips"
        )
        if len(binding_stage_clips) != 4 or any(len(stage) != 1 for stage in binding_stage_clips):
            raise ContractError(
                f"LMB COMBO closure requires one authored clip in each of four stages: {skill_id}"
            )

        animevents_path = (
            repository_root
            / "Data"
            / "Animation"
            / "Authored"
            / basis.animation_asset_id
            / f"{basis.animation_asset_id}.animevents"
        )
        asset_effect_rows = _parse_asset_effect_rows(
            tracker.text(animevents_path, "AUTHORED_ANIMATION_EFFECT_CUES")
        )

        manifest_path = (
            repository_root
            / "Data"
            / "Effects"
            / "Imported"
            / basis.animation_asset_id
            / f"{basis.animation_asset_id}.combat-source-stage-manifest.json"
        )
        manifest = tracker.json(manifest_path, "SOURCE_STAGE_MANIFEST")
        if (
            manifest.get("schema") != SOURCE_MANIFEST_SCHEMA
            or manifest.get("version") != SOURCE_MANIFEST_VERSION
            or manifest.get("animationAssetId") != basis.animation_asset_id
            or manifest.get("characterClass") != basis.character_class
        ):
            raise ContractError(f"source-stage manifest identity mismatch: {basis.animation_asset_id}")
        manifest_skill = _find_unique(
            _require_array(manifest.get("skills"), f"{basis.animation_asset_id}.skills"),
            lambda item: isinstance(item, dict) and item.get("productSkillId") == skill_id,
            f"{basis.character_class}/{skill_id} source-stage skill",
        )
        manifest_stages = _require_array(
            manifest_skill.get("stages"), f"source manifest {skill_id}.stages"
        )
        if len(manifest_stages) != 4:
            raise ContractError(f"source-stage manifest must expose four BA stages: {skill_id}")

        expected_imported_id = (
            f"effect.{_class_effect_id(basis.character_class)}.skill.{skill_id}.imported"
        )
        if (
            imported.get("schema") != AUTHORING_SCHEMA
            or imported.get("version") != 12
            or imported.get("effectAssetId") != expected_imported_id
        ):
            raise ContractError(f"Imported source identity mismatch: {expected_imported_id}")
        imported_elements = _require_array(imported.get("elements"), f"{expected_imported_id}.elements")
        imported_elements_by_id: dict[str, dict[str, Any]] = {}
        for source_order, raw_element in enumerate(imported_elements):
            element = _require_object(raw_element, f"{expected_imported_id}.elements[{source_order}]")
            element_id = _require_string(element.get("id"), f"Imported element[{source_order}].id")
            if element_id in imported_elements_by_id:
                raise ContractError(f"duplicate Imported source Element ID: {element_id}")
            imported_elements_by_id[element_id] = element
        imported_by_class[basis.character_class] = (imported, imported_elements_by_id)

        class_stages: list[StageContext] = []
        for stage_index in range(4):
            stage_id = f"skill.{skill_id}.stage.{stage_index}"
            manifest_stage = _find_unique(
                manifest_stages,
                lambda item, stage_index=stage_index: (
                    isinstance(item, dict) and item.get("stageIndex") == stage_index
                ),
                f"source stage {stage_id}",
            )
            if manifest_stage.get("stageId") != stage_id:
                raise ContractError(f"source stage stable ID mismatch: {stage_id}")
            manifest_clips = _require_array(
                manifest_stage.get("clips"), f"{stage_id}.clips"
            )
            if len(manifest_clips) != 1:
                raise ContractError(f"{stage_id} must have one source clip")
            manifest_clip = _require_object(manifest_clips[0], f"{stage_id}.clips[0]")
            clip = binding_stage_clips[stage_index][0]
            if (
                manifest_clip.get("clip") != clip
                or manifest_clip.get("sequenceIndex") != stage_index
            ):
                raise ContractError(f"authored/source stage clip join mismatch: {stage_id}")

            class_id = _class_effect_id(basis.character_class)
            effect_asset_id = f"effect.{class_id}.skill.{skill_id}.ba{stage_index + 1}"
            product_match = PRODUCT_EFFECT_RE.fullmatch(effect_asset_id)
            if (
                product_match is None
                or int(product_match.group("skill_id")) != skill_id
                or int(product_match.group("stage")) != stage_index + 1
            ):
                raise ContractError(f"invalid product Effect stable ID: {effect_asset_id}")
            matching_cues = [
                row
                for row in asset_effect_rows
                if row["clip"] == clip and row["payload"] == effect_asset_id
            ]
            if len(matching_cues) != 1:
                raise ContractError(
                    f"{clip} must have exactly one asset cue for {effect_asset_id}; found {len(matching_cues)}"
                )
            product_cue = matching_cues[0]
            if (
                product_cue["startms"] != "0"
                or product_cue["anchor"] != "root"
                or product_cue["follow"] != "follow"
                or product_cue["stop"] != "natural"
                or product_cue["transform"]
                != {
                    "px": "0",
                    "py": "0",
                    "pz": "0",
                    "rx": "0",
                    "ry": "0",
                    "rz": "0",
                    "sx": "1",
                    "sy": "1",
                    "sz": "1",
                }
            ):
                raise ContractError(f"product asset cue contract mismatch: {effect_asset_id}")
            catalog_entry = catalog_by_id.get(effect_asset_id)
            if catalog_entry is None or "payloadKind" in catalog_entry:
                raise ContractError(f"missing generic legacy catalog entry: {effect_asset_id}")

            receipt_relative = (
                f"Data/Effects/AuthoredCorrections/Generated/{basis.animation_asset_id}/"
                f"{effect_asset_id}.approximation-receipt.json"
            )
            receipt_path = _resolve_repository_path(
                repository_root, receipt_relative, f"{effect_asset_id}.approximation receipt"
            )
            receipt = tracker.json(receipt_path, "LEGACY_APPROXIMATION_RECEIPT")
            if (
                receipt.get("schema") != APPROXIMATION_RECEIPT_SCHEMA
                or receipt.get("targetEffectAssetId") != effect_asset_id
                or receipt.get("characterClass") != basis.character_class
                or receipt.get("productSkillId") != skill_id
                or receipt.get("stageIndex") != stage_index
                or receipt.get("stageId") != stage_id
            ):
                raise ContractError(f"approximation receipt identity mismatch: {effect_asset_id}")
            if receipt.get("sourceEventIds") != manifest_stage.get("sourceEventIds"):
                raise ContractError(f"approximation/source event coverage mismatch: {effect_asset_id}")
            source_artifact = _require_object(
                receipt.get("sourceArtifact"), f"{effect_asset_id}.sourceArtifact"
            )
            imported_ref = _require_object(
                source_artifact.get("importedDocument"),
                f"{effect_asset_id}.sourceArtifact.importedDocument",
            )
            if (
                imported_ref.get("path") != basis.source_relative_path
                or imported_ref.get("sha256") != basis.source_sha256
                or imported_ref.get("effectAssetId") != expected_imported_id
            ):
                raise ContractError(f"approximation Imported source identity mismatch: {effect_asset_id}")

            current_product, current_product_elements = _build_current_product(
                repository_root,
                tracker,
                catalog_entry,
                basis,
                skill_id,
                stage_index,
                effect_asset_id,
            )
            context = StageContext(
                basis=basis,
                skill_id=skill_id,
                stage_index=stage_index,
                stage_id=stage_id,
                clip=clip,
                timeline_offset_seconds=_require_number(
                    manifest_stage.get("timelineOffsetSeconds"), f"{stage_id}.timelineOffsetSeconds"
                ),
                duration_seconds=_require_number(
                    manifest_stage.get("durationSeconds"), f"{stage_id}.durationSeconds"
                ),
                source_event_ids=[
                    _require_stable_id(item, f"{stage_id}.sourceEventIds")
                    for item in _require_array(
                        manifest_stage.get("sourceEventIds"), f"{stage_id}.sourceEventIds"
                    )
                ],
                effect_asset_id=effect_asset_id,
                manifest_stage=manifest_stage,
                receipt_path=receipt_path,
                receipt=receipt,
                current_product=current_product,
                current_product_elements=current_product_elements,
                occurrence_records=[],
                source_row_ids=[],
            )
            class_stages.append(context)
            stage_contexts.append(context)

        for context in class_stages:
            occurrence_values = _require_array(
                context.receipt.get("occurrences"),
                f"{context.effect_asset_id}.occurrences",
            )
            for expected_occurrence_index, raw_occurrence in enumerate(occurrence_values):
                occurrence = _require_object(
                    raw_occurrence,
                    f"{context.effect_asset_id}.occurrences[{expected_occurrence_index}]",
                )
                occurrence_index = _require_int(
                    occurrence.get("occurrenceIndex"),
                    f"{context.effect_asset_id}.occurrenceIndex",
                    minimum=0,
                )
                if occurrence_index != expected_occurrence_index:
                    raise ContractError(
                        f"occurrence indices must be contiguous: {context.effect_asset_id}"
                    )
                occurrence_id = (
                    f"occurrence.{context.effect_asset_id}."
                    f"{occurrence_index:03d}"
                )
                candidates = _require_array(
                    occurrence.get("candidates"), f"{occurrence_id}.candidates"
                )
                if not candidates:
                    raise ContractError(f"particle occurrence has no candidates: {occurrence_id}")
                occurrence_source_row_ids: list[str] = []
                selected_count = 0
                for candidate_index, raw_candidate in enumerate(candidates):
                    candidate = _require_object(
                        raw_candidate, f"{occurrence_id}.candidates[{candidate_index}]"
                    )
                    source_element_id = _require_string(
                        candidate.get("sourceElementId"),
                        f"{occurrence_id}.candidates[{candidate_index}].sourceElementId",
                    )
                    candidate_key = (basis.character_class, source_element_id)
                    if candidate_key in candidate_binding_by_element_id:
                        previous = candidate_binding_by_element_id[candidate_key]
                        raise ContractError(
                            "duplicate particle source candidate across occurrences: "
                            f"{source_element_id} ({previous['occurrenceId']} and {occurrence_id})"
                        )
                    element = imported_elements_by_id.get(source_element_id)
                    if element is None:
                        raise ContractError(
                            f"particle candidate is absent from Imported source: {source_element_id}"
                        )
                    if element.get("kind") != "particle":
                        raise ContractError(f"occurrence candidate is not a Particle row: {source_element_id}")
                    element_sha256 = canonical_json_sha256(element)
                    if element_sha256 != _require_sha256(
                        candidate.get("sourceElementSha256"),
                        f"{source_element_id}.sourceElementSha256",
                    ):
                        raise ContractError(f"stale particle source candidate SHA: {source_element_id}")
                    family = _renderer_family(element, source_element_id)
                    expected_source_kind = {
                        "MESH_PARTICLE": "meshParticle",
                        "SPRITE_PARTICLE": "spriteParticle",
                        "CASCADE_RIBBON": "ribbonParticle",
                    }.get(family)
                    if expected_source_kind is None or candidate.get("sourceKind") != expected_source_kind:
                        raise ContractError(f"particle candidate renderer identity mismatch: {source_element_id}")
                    decision = _require_string(
                        candidate.get("selectionDecision"),
                        f"{source_element_id}.selectionDecision",
                    )
                    if decision not in ("selected", "rejected", "budgetExcluded"):
                        raise ContractError(f"unsupported candidate decision: {decision}")
                    if decision == "selected":
                        selected_count += 1
                    row_id = _source_row_id(
                        basis.character_class,
                        skill_id,
                        source_element_id,
                        element_sha256,
                    )
                    occurrence_source_row_ids.append(row_id)
                    candidate_binding_by_element_id[candidate_key] = {
                        "stage": context,
                        "occurrenceId": occurrence_id,
                        "candidate": candidate,
                    }
                occurrence_status = _require_string(
                    occurrence.get("status"), f"{occurrence_id}.status"
                )
                if (selected_count > 0 and occurrence_status != "selected") or (
                    selected_count == 0 and occurrence_status != "blocked"
                ):
                    raise ContractError(f"occurrence selection status mismatch: {occurrence_id}")
                source_timeline_values = [
                    _require_number(
                        imported_elements_by_id[
                            _require_string(item.get("sourceElementId"), "candidate sourceElementId")
                        ]["detail"]["timing"]["startDelaySeconds"],
                        f"{occurrence_id}.sourceTimeline",
                    )
                    for item in candidates
                ]
                context.occurrence_records.append(
                    {
                        "occurrenceId": occurrence_id,
                        "characterClass": basis.character_class,
                        "skillId": skill_id,
                        "stageIndex": context.stage_index,
                        "stageId": context.stage_id,
                        "productEffectAssetId": context.effect_asset_id,
                        "receiptOccurrenceIndex": occurrence_index,
                        "sourceOccurrenceKey": occurrence.get("occurrenceKey") or None,
                        "sourceEventId": occurrence.get("sourceEventId") or None,
                        "sourceTimelineSeconds": min(source_timeline_values),
                        "stageLocalTimeSeconds": (
                            min(source_timeline_values) - context.timeline_offset_seconds
                        ),
                        "sourceRowIds": occurrence_source_row_ids,
                        "sourceRowCount": len(occurrence_source_row_ids),
                        "legacySelectedSourceRowCount": selected_count,
                        "disposition": (
                            "LEGACY_APPROXIMATION" if selected_count else "FAIL_CLOSED"
                        ),
                        "sourceRuntimeAdmission": False,
                    }
                )

            selected_target_ids = {
                _require_stable_id(
                    binding["candidate"].get("targetElementId"),
                    f"{source_element_id}.targetElementId",
                )
                for (_, source_element_id), binding in candidate_binding_by_element_id.items()
                if binding["stage"] is context
                and binding["candidate"].get("selectionDecision") == "selected"
            }
            if selected_target_ids != set(context.current_product_elements):
                raise ContractError(
                    f"selected source/current product Element coverage mismatch: {context.effect_asset_id}"
                )
            output = _require_object(
                context.receipt.get("output"), f"{context.effect_asset_id}.output"
            )
            if (
                output.get("elementCount") != context.current_product["emitterCount"]
                or output.get("particleCount") != 0
            ):
                raise ContractError(f"approximation output/current product mismatch: {context.effect_asset_id}")

    imported_particle_ids = {
        (character_class, element_id)
        for character_class, (_, elements_by_id) in imported_by_class.items()
        for element_id, element in elements_by_id.items()
        if element.get("kind") == "particle"
    }
    candidate_particle_ids = set(candidate_binding_by_element_id)
    if imported_particle_ids != candidate_particle_ids:
        missing = sorted(imported_particle_ids - candidate_particle_ids)
        extra = sorted(candidate_particle_ids - imported_particle_ids)
        raise ContractError(
            f"particle source occurrence coverage mismatch; missing={missing}, extra={extra}"
        )

    stages_by_class: dict[str, list[StageContext]] = defaultdict(list)
    event_to_stage_by_class: dict[str, dict[str, StageContext]] = defaultdict(dict)
    for stage in stage_contexts:
        stages_by_class[stage.basis.character_class].append(stage)
        for source_event_id in stage.source_event_ids:
            if source_event_id in event_to_stage_by_class[stage.basis.character_class]:
                raise ContractError(
                    f"duplicate sourceEventId across stages: {stage.basis.character_class}/{source_event_id}"
                )
            event_to_stage_by_class[stage.basis.character_class][source_event_id] = stage

    source_rows: list[dict[str, Any]] = []
    source_row_ids: set[str] = set()
    for basis in basis_profiles:
        imported, elements_by_id = imported_by_class[basis.character_class]
        elements = _require_array(imported.get("elements"), "Imported elements")
        for source_order, raw_element in enumerate(elements):
            element = _require_object(raw_element, f"Imported elements[{source_order}]")
            source_element_id = _require_string(element.get("id"), "source element id")
            candidate_binding = candidate_binding_by_element_id.get(
                (basis.character_class, source_element_id)
            )
            if candidate_binding is not None:
                stage = candidate_binding["stage"]
                occurrence_id = candidate_binding["occurrenceId"]
                candidate = candidate_binding["candidate"]
                stage_join = {
                    "kind": "APPROXIMATION_OCCURRENCE_RECEIPT",
                    "sourceEventId": (
                        _source_presentation_fields(element, source_element_id)["sourceEventId"]
                    ),
                    "sourceTimelineSeconds": _require_number(
                        element["detail"]["timing"]["startDelaySeconds"],
                        f"{source_element_id}.startDelaySeconds",
                    ),
                }
            else:
                stage, stage_join = _stage_join_for_non_particle(
                    element,
                    stages_by_class[basis.character_class],
                    event_to_stage_by_class[basis.character_class],
                    source_element_id,
                )
                occurrence_id = None
                candidate = None
            row = _build_source_row(
                element,
                source_order,
                stage,
                stage_join,
                occurrence_id,
                candidate,
            )
            if row["sourceRowId"] in source_row_ids:
                raise ContractError(f"duplicate sourceRowId: {row['sourceRowId']}")
            source_row_ids.add(row["sourceRowId"])
            source_rows.append(row)
            stage.source_row_ids.append(row["sourceRowId"])

    stage_contexts.sort(
        key=lambda item: (item.basis.character_class, item.skill_id, item.stage_index)
    )
    source_rows.sort(
        key=lambda item: (
            item["characterClass"],
            item["skillId"],
            item["stageIndex"],
            item["sourceOrder"],
            item["sourceElementId"],
        )
    )
    occurrences = sorted(
        [item for stage in stage_contexts for item in stage.occurrence_records],
        key=lambda item: (
            item["characterClass"],
            item["skillId"],
            item["stageIndex"],
            item["receiptOccurrenceIndex"],
        ),
    )

    stage_rows: list[dict[str, Any]] = []
    for stage in stage_contexts:
        stage_source_rows = [
            row for row in source_rows if row["productEffectAssetId"] == stage.effect_asset_id
        ]
        stage_occurrences = [
            item for item in occurrences if item["productEffectAssetId"] == stage.effect_asset_id
        ]
        family_counts = Counter(row["rendererFamily"] for row in stage_source_rows)
        disposition_counts = Counter(
            row["admission"]["disposition"] for row in stage_source_rows
        )
        stage_rows.append(
            {
                "characterClass": stage.basis.character_class,
                "animationAssetId": stage.basis.animation_asset_id,
                "skillId": stage.skill_id,
                "inputSlot": "LMB",
                "skillKind": "COMBO",
                "stageIndex": stage.stage_index,
                "stageId": stage.stage_id,
                "clip": stage.clip,
                "productEffectAssetId": stage.effect_asset_id,
                "importBasisId": stage.basis.basis_id,
                "timelineOffsetSeconds": stage.timeline_offset_seconds,
                "durationSeconds": stage.duration_seconds,
                "sourceEventIds": stage.source_event_ids,
                "sourceRowIds": stage.source_row_ids,
                "sourceRowCount": len(stage_source_rows),
                "particleOccurrenceIds": [
                    item["occurrenceId"] for item in stage_occurrences
                ],
                "particleOccurrenceCount": len(stage_occurrences),
                "rendererFamilyCounts": {
                    key: family_counts[key] for key in sorted(family_counts)
                },
                "typedDispositionCounts": {
                    key: disposition_counts[key] for key in sorted(disposition_counts)
                },
                "currentProduct": stage.current_product,
                "sourceRuntimeAdmission": False,
                "productMutation": False,
            }
        )

    component_count = sum(
        stage["currentProduct"]["componentCount"] for stage in stage_rows
    )
    legacy_emitter_count = sum(
        stage["currentProduct"]["emitterCount"] for stage in stage_rows
    )
    expected = {
        "stageCount": expected_stage_count,
        "particleOccurrenceCount": expected_occurrence_count,
        "sourceRendererRowCount": expected_source_row_count,
        "currentLegacyComponentCount": expected_component_count,
        "currentLegacyEmitterCount": expected_legacy_emitter_count,
    }
    actual = {
        "stageCount": len(stage_rows),
        "particleOccurrenceCount": len(occurrences),
        "sourceRendererRowCount": len(source_rows),
        "currentLegacyComponentCount": component_count,
        "currentLegacyEmitterCount": legacy_emitter_count,
    }
    if actual != expected:
        raise ContractError(f"scope closure denominator mismatch: expected={expected}, actual={actual}")

    family_counts = Counter(row["rendererFamily"] for row in source_rows)
    disposition_counts = Counter(row["admission"]["disposition"] for row in source_rows)
    selection_counts = Counter(
        row["legacyProjection"]["selectionDecision"] for row in source_rows
    )
    class_summaries: list[dict[str, Any]] = []
    for basis in basis_profiles:
        class_rows = [row for row in source_rows if row["characterClass"] == basis.character_class]
        class_stages = [row for row in stage_rows if row["characterClass"] == basis.character_class]
        class_occurrences = [
            row for row in occurrences if row["characterClass"] == basis.character_class
        ]
        class_families = Counter(row["rendererFamily"] for row in class_rows)
        class_dispositions = Counter(row["admission"]["disposition"] for row in class_rows)
        class_summaries.append(
            {
                "characterClass": basis.character_class,
                "animationAssetId": basis.animation_asset_id,
                "skillId": class_stages[0]["skillId"],
                "stageCount": len(class_stages),
                "particleOccurrenceCount": len(class_occurrences),
                "sourceRendererRowCount": len(class_rows),
                "rendererFamilyCounts": {
                    key: class_families[key] for key in sorted(class_families)
                },
                "typedDispositionCounts": {
                    key: class_dispositions[key] for key in sorted(class_dispositions)
                },
                "currentLegacyComponentCount": sum(
                    row["currentProduct"]["componentCount"] for row in class_stages
                ),
                "currentLegacyEmitterCount": sum(
                    row["currentProduct"]["emitterCount"] for row in class_stages
                ),
            }
        )

    index: dict[str, Any] = {
        "schema": INDEX_SCHEMA,
        "formatVersion": INDEX_FORMAT_VERSION,
        "indexId": "effect.source-runtime-admission.lmb-combo-3class.v1",
        "contractRole": "EVIDENCE_JOIN_NOT_RUNTIME_AUTHORITY",
        "scope": {
            "inputSlot": "LMB",
            "skillKind": "COMBO",
            "characterClasses": [
                profile.character_class for profile in basis_profiles
            ],
            "expectedDenominators": expected,
        },
        "schemaIdentity": {
            "path": SCHEMA_RELATIVE_PATH,
            "rawSha256": sha256_file(schema_path),
            "canonicalJsonSha256": canonical_json_sha256(schema_document),
        },
        "builderIdentity": {
            "path": _repository_relative(repository_root, SCRIPT_PATH),
            "rawSha256": sha256_file(SCRIPT_PATH),
        },
        "importBasisContract": {
            "path": _repository_relative(repository_root, basis_path),
            "rawSha256": sha256_file(basis_path),
            "contractId": basis_contract["contractId"],
            "contractSha256": basis_contract["contractSha256"],
            "runtimeAdmission": False,
        },
        "inputArtifacts": tracker.records(),
        "stages": stage_rows,
        "particleOccurrences": occurrences,
        "sourceRows": source_rows,
        "denominators": {
            **actual,
            "particleSourceRowCount": sum(
                1 for row in source_rows if row["sourceKind"] == "particle"
            ),
            "nonParticleSourceRowCount": sum(
                1 for row in source_rows if row["sourceKind"] != "particle"
            ),
            "typedDispositionRowCount": len(source_rows),
            "sourceRuntimeAdmissionCount": sum(
                1 for row in source_rows if row["admission"]["sourceRuntimeAdmission"]
            ),
            "sourceProductAdmissionCount": sum(
                1 for row in source_rows if row["admission"]["sourceProductAdmission"]
            ),
        },
        "rendererFamilyCounts": {
            key: family_counts[key] for key in sorted(family_counts)
        },
        "typedDispositionCounts": {
            key: disposition_counts[key] for key in sorted(disposition_counts)
        },
        "legacySelectionCounts": {
            key: selection_counts[key] for key in sorted(selection_counts)
        },
        "classSummaries": class_summaries,
        "admission": {
            "runtimeExecution": False,
            "productAdmission": False,
            "currentProductMutation": False,
            "currentProductPreserved": True,
            "blockers": [
                "ASSET_SCOPED_IMPORT_BASIS_UNRESOLVED",
                "GENERIC_SOURCE_RUNTIME_PROGRAM_NOT_PUBLISHED",
                "OCCURRENCE_VF_PASS_BINDINGS_INCOMPLETE",
                "SOURCE_FAMILY_ADAPTERS_NOT_ALL_ADMITTED",
            ],
        },
        "transactionPolicy": {
            "loadOrder": ["parse", "validate", "stage", "commit"],
            "commitMode": "ATOMIC_REPLACE_AFTER_FULL_SCOPE_VALIDATION",
            "failureAction": "PRESERVE_PREVIOUS_ARTIFACT_AND_PRODUCT_RUNTIME",
            "catalogMutation": False,
            "runtimeResourceMutation": False,
        },
        "artifactSha256": "",
    }
    hash_payload = copy.deepcopy(index)
    del hash_payload["artifactSha256"]
    index["artifactSha256"] = canonical_json_sha256(hash_payload)
    validate_index(index)
    return index


def validate_index(index: dict[str, Any]) -> None:
    if index.get("schema") != INDEX_SCHEMA or index.get("formatVersion") != INDEX_FORMAT_VERSION:
        raise ContractError("admission index header mismatch")
    if index.get("contractRole") != "EVIDENCE_JOIN_NOT_RUNTIME_AUTHORITY":
        raise ContractError("admission index role mismatch")
    artifact_sha256 = _require_sha256(index.get("artifactSha256"), "artifactSha256")
    hash_payload = copy.deepcopy(index)
    del hash_payload["artifactSha256"]
    if canonical_json_sha256(hash_payload) != artifact_sha256:
        raise ContractError("admission index artifactSha256 is stale")

    stages = _require_array(index.get("stages"), "index.stages")
    occurrences = _require_array(
        index.get("particleOccurrences"), "index.particleOccurrences"
    )
    source_rows = _require_array(index.get("sourceRows"), "index.sourceRows")
    expected = _require_object(
        _require_object(index.get("scope"), "index.scope").get("expectedDenominators"),
        "scope.expectedDenominators",
    )
    denominators = _require_object(index.get("denominators"), "index.denominators")
    actual = {
        "stageCount": len(stages),
        "particleOccurrenceCount": len(occurrences),
        "sourceRendererRowCount": len(source_rows),
        "currentLegacyComponentCount": sum(
            _require_object(stage, "stage")["currentProduct"]["componentCount"]
            for stage in stages
        ),
        "currentLegacyEmitterCount": sum(
            _require_object(stage, "stage")["currentProduct"]["emitterCount"]
            for stage in stages
        ),
    }
    if actual != expected:
        raise ContractError("admission index scope denominator mismatch")
    for key, value in actual.items():
        if denominators.get(key) != value:
            raise ContractError(f"admission index denominator is stale: {key}")

    stage_ids = [stage.get("stageId") for stage in stages if isinstance(stage, dict)]
    if len(stage_ids) != len(set(stage_ids)):
        raise ContractError("duplicate stageId in admission index")
    occurrence_ids = [
        occurrence.get("occurrenceId")
        for occurrence in occurrences
        if isinstance(occurrence, dict)
    ]
    if len(occurrence_ids) != len(set(occurrence_ids)):
        raise ContractError("duplicate occurrenceId in admission index")
    row_ids = [row.get("sourceRowId") for row in source_rows if isinstance(row, dict)]
    if len(row_ids) != len(set(row_ids)):
        raise ContractError("duplicate sourceRowId in admission index")

    rows_by_id = {
        _require_string(row["sourceRowId"], "sourceRowId"): row
        for row in source_rows
    }
    particle_row_ids = {
        row_id
        for row_id, row in rows_by_id.items()
        if row.get("sourceKind") == "particle"
    }
    occurrence_row_ids: set[str] = set()
    for occurrence in occurrences:
        item = _require_object(occurrence, "particleOccurrence")
        ids = [
            _require_string(row_id, "particleOccurrence.sourceRowId")
            for row_id in _require_array(item.get("sourceRowIds"), "particleOccurrence.sourceRowIds")
        ]
        if len(ids) != len(set(ids)):
            raise ContractError(f"duplicate row within occurrence: {item.get('occurrenceId')}")
        overlap = occurrence_row_ids.intersection(ids)
        if overlap:
            raise ContractError(f"source row belongs to multiple occurrences: {sorted(overlap)}")
        occurrence_row_ids.update(ids)
    if occurrence_row_ids != particle_row_ids:
        raise ContractError("particle occurrence/source row closure mismatch")

    family_counts = Counter()
    disposition_counts = Counter()
    runtime_admission_count = 0
    product_admission_count = 0
    for row_id, row in rows_by_id.items():
        family_counts[_require_stable_id(row.get("rendererFamily"), f"{row_id}.rendererFamily")] += 1
        admission = _require_object(row.get("admission"), f"{row_id}.admission")
        disposition = _require_string(admission.get("disposition"), f"{row_id}.disposition")
        if disposition not in DISPOSITIONS:
            raise ContractError(f"unknown row disposition: {row_id}/{disposition}")
        disposition_counts[disposition] += 1
        runtime_admission_count += int(bool(admission.get("sourceRuntimeAdmission")))
        product_admission_count += int(bool(admission.get("sourceProductAdmission")))
        if row.get("sourceKind") == "particle" and row.get("particleOccurrenceId") is None:
            raise ContractError(f"Particle source row has no occurrence: {row_id}")
        if row.get("sourceKind") != "particle" and row.get("particleOccurrenceId") is not None:
            raise ContractError(f"non-Particle source row joined a Particle occurrence: {row_id}")
    if runtime_admission_count or product_admission_count:
        raise ContractError("first-slice admission index cannot grant source runtime/product admission")
    if index.get("rendererFamilyCounts") != {
        key: family_counts[key] for key in sorted(family_counts)
    }:
        raise ContractError("renderer family summary is stale")
    if index.get("typedDispositionCounts") != {
        key: disposition_counts[key] for key in sorted(disposition_counts)
    }:
        raise ContractError("typed disposition summary is stale")
    if denominators.get("typedDispositionRowCount") != len(source_rows):
        raise ContractError("typed disposition denominator is incomplete")
    if denominators.get("sourceRuntimeAdmissionCount") != 0:
        raise ContractError("source runtime admission denominator must remain zero")
    if denominators.get("sourceProductAdmissionCount") != 0:
        raise ContractError("source product admission denominator must remain zero")
    admission = _require_object(index.get("admission"), "index.admission")
    if (
        admission.get("runtimeExecution") is not False
        or admission.get("productAdmission") is not False
        or admission.get("currentProductMutation") is not False
        or admission.get("currentProductPreserved") is not True
    ):
        raise ContractError("first-slice global admission boundary changed")


def write_index_transactionally(index: dict[str, Any], output_path: Path) -> None:
    validate_index(index)
    output_path = output_path.resolve()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    payload = pretty_json_bytes(index)
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="wb",
            prefix=f".{output_path.name}.",
            suffix=".tmp",
            dir=output_path.parent,
            delete=False,
        ) as temporary:
            temporary_path = Path(temporary.name)
            temporary.write(payload)
            temporary.flush()
            os.fsync(temporary.fileno())
        os.replace(temporary_path, output_path)
        temporary_path = None
    finally:
        if temporary_path is not None and temporary_path.exists():
            temporary_path.unlink()


def build_and_write(
    repository_root: Path,
    basis_path: Path,
    output_path: Path,
    **expected_counts: int,
) -> dict[str, Any]:
    index = build_index(
        repository_root,
        basis_path,
        **expected_counts,
    )
    write_index_transactionally(index, output_path)
    return index


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=REPOSITORY_ROOT,
    )
    parser.add_argument(
        "--basis-contract",
        type=Path,
        default=Path(DEFAULT_BASIS_RELATIVE_PATH),
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path(DEFAULT_OUTPUT_RELATIVE_PATH),
    )
    parser.add_argument("--expected-stage-count", type=int, default=DEFAULT_EXPECTED_STAGE_COUNT)
    parser.add_argument(
        "--expected-occurrence-count",
        type=int,
        default=DEFAULT_EXPECTED_OCCURRENCE_COUNT,
    )
    parser.add_argument(
        "--expected-source-row-count",
        type=int,
        default=DEFAULT_EXPECTED_SOURCE_ROW_COUNT,
    )
    parser.add_argument(
        "--expected-component-count",
        type=int,
        default=DEFAULT_EXPECTED_COMPONENT_COUNT,
    )
    parser.add_argument(
        "--expected-legacy-emitter-count",
        type=int,
        default=DEFAULT_EXPECTED_LEGACY_EMITTER_COUNT,
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Validate inputs and require the tracked output to match exactly.",
    )
    return parser.parse_args()


def main() -> int:
    args = _parse_args()
    repository_root = args.repository_root.resolve()
    basis_path = args.basis_contract
    if not basis_path.is_absolute():
        basis_path = repository_root / basis_path
    output_path = args.output
    if not output_path.is_absolute():
        output_path = repository_root / output_path
    expected_counts = {
        "expected_stage_count": args.expected_stage_count,
        "expected_occurrence_count": args.expected_occurrence_count,
        "expected_source_row_count": args.expected_source_row_count,
        "expected_component_count": args.expected_component_count,
        "expected_legacy_emitter_count": args.expected_legacy_emitter_count,
    }
    try:
        index = build_index(
            repository_root,
            basis_path,
            **expected_counts,
        )
        if args.check:
            if not output_path.is_file():
                raise ContractError(f"missing generated admission index: {output_path}")
            if output_path.read_bytes() != pretty_json_bytes(index):
                raise ContractError(f"generated admission index is stale: {output_path}")
        else:
            write_index_transactionally(index, output_path)
    except (ContractError, OSError, json.JSONDecodeError) as error:
        print(f"Effect source-runtime admission build failed: {error}")
        return 1
    denominators = index["denominators"]
    dispositions = index["typedDispositionCounts"]
    print(
        "Effect source-runtime admission validated: "
        f"stages={denominators['stageCount']}, "
        f"occurrences={denominators['particleOccurrenceCount']}, "
        f"sourceRows={denominators['sourceRendererRowCount']}, "
        f"legacyApproximation={dispositions.get('LEGACY_APPROXIMATION', 0)}, "
        f"failClosed={dispositions.get('FAIL_CLOSED', 0)}, "
        "runtimeAdmission=0"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
