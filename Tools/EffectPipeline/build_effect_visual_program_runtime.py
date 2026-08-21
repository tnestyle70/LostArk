#!/usr/bin/env python3
"""Build the generic V6 visual-program runtime sidecar source artifact.

The sidecar keeps selectors class/skill/order-free.  For the three BA programs
it deterministically overlays the exact Imported SourceRecipe onto the already
admitted legacy carrier element, preserving that carrier's material, resources,
and transform.  Unsupported rows remain diagnostic and cannot mutate a
document.  Artist F LocalDecal rows remain typed adapter packets for the later
catalog/renderer integration seam.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import importlib.util
import json
import os
import re
import subprocess
import struct
import tempfile
from collections import Counter, defaultdict
from pathlib import Path, PurePosixPath
from typing import Any, Iterable


SCRIPT_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent

PHASE1_PATH = SCRIPT_PATH.with_name("build_effect_visual_program_corpus.py")
PHASE1_SPEC = importlib.util.spec_from_file_location(
    "build_effect_visual_program_corpus_runtime_input", PHASE1_PATH
)
if PHASE1_SPEC is None or PHASE1_SPEC.loader is None:
    raise RuntimeError(f"cannot import phase1 builder: {PHASE1_PATH}")
phase1 = importlib.util.module_from_spec(PHASE1_SPEC)
PHASE1_SPEC.loader.exec_module(phase1)

SCHEMA_RELATIVE_PATH = (
    "Tools/EffectPipeline/Schemas/"
    "lostark.effect-visual-program-runtime.schema.json"
)
SOURCE_CORPUS_RELATIVE_PATH = (
    "Data/Effects/VisualPrograms/effect-visual-program-corpus.v1.json"
)
ADMISSION_RELATIVE_PATH = phase1.ADMISSION_RELATIVE_PATH
DEFAULT_OUTPUT_RELATIVE_PATH = (
    "Data/Effects/VisualPrograms/effect-visual-program-runtime.v1.json"
)

RUNTIME_SCHEMA = "lostark.effect-visual-program-runtime"
RUNTIME_VERSION = 1
RUNTIME_ID = "effect.visual-program-runtime.v1"
CONTRACT_ROLE = "GENERIC_VISUAL_PROGRAM_RUNTIME_SIDECAR_STAGE_INPUT"

EXPECTED_PROGRAM_COUNT = 17
EXPECTED_BA_PROGRAM_COUNT = 16
EXPECTED_ADAPTER_PROGRAM_COUNT = 1
EXPECTED_ROW_COUNT = 135
EXPECTED_OVERLAY_COUNT = 66
EXPECTED_LOCAL_PACKET_COUNT = 2
EXPECTED_FAIL_CLOSED_COUNT = 67
EXPECTED_CASCADE_RIBBON_ROW_COUNT = 4
EXPECTED_SUPPLEMENTAL_ELEMENT_COUNT = 16
EXPECTED_ARTIST_CASCADE_RIBBON_ELEMENT_COUNT = 2
EXPECTED_ARTIST_F_CASCADE_RIBBON_ELEMENT_COUNT = 1
EXPECTED_ARTIST_T_CASCADE_RIBBON_ELEMENT_COUNT = 1
EXPECTED_ANIMATION_TRAIL_ELEMENT_COUNT = 13
EXPECTED_BAKED_EDGE_LIGHT_ELEMENT_COUNT = 1

VALTAN_WHIRLWIND_EFFECT_ASSET_ID = "effect.valtan.pattern.420633.active"
VALTAN_WHIRLWIND_DOCUMENT_RELATIVE_PATH = (
    "Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json"
)

LOCAL_DECAL_PROGRAM_RELATIVE_PATH = phase1.RUNTIME_PROGRAM_RELATIVE_PATH
LOCAL_DECAL_RECEIPT_RELATIVE_PATH = phase1.LOCAL_DECAL_RECEIPT_RELATIVE_PATH

LOCAL_DECAL_INPUT_CONSUMED_MASK = (0x820EC1FF, 0x00000001)
LOCAL_DECAL_INPUT_SUPPRESSED_MASK = (0x7DF13E00, 0x00000000)
LOCAL_DECAL_VECTOR_CONSUMED_MASK = (0x0F, 0x0F, 0x00)
LOCAL_DECAL_VECTOR_SUPPRESSED_MASK = (0x00, 0x00, 0x0F)
LOCAL_DECAL_STATIC_SELECTED_MASK = 0x3FFFB
LOCAL_DECAL_STATIC_CONSUMED_MASK = 0x3FFFF
LOCAL_DECAL_RENDER_CONSUMED_MASK = 0x03
LOCAL_DECAL_RENDER_SUPPRESSED_MASK = 0x3C

LOCAL_DECAL_SRV_FORMATS = {
    "HEIGHT": ("BC1_UNORM", False),
    "DIFFUSE": ("BC3_UNORM", True),
    "DISSOLVE": ("BC1_UNORM", False),
    "NORMAL": ("BC5_UNORM", False),
    "SPECULAR": ("BC1_UNORM", True),
    "EMISSIVE": ("BC1_UNORM", True),
}

SHA256_RE = re.compile(r"^[0-9a-f]{64}$")
STABLE_ID_RE = re.compile(r"^[A-Za-z0-9_.-]+$")

_TYPED_CODEC_SHA_CACHE: dict[tuple[str, ...], tuple[str, ...]] = {}


class ContractError(ValueError):
    """Raised when runtime staging would be partial, stale, or overclaimed."""


def _typed_codec_tool(repository_root: Path) -> Path:
    configured = os.environ.get("LOSTARK_EFFECT_DOCUMENT_CODEC_TOOL")
    candidates = []
    if configured:
        candidates.append(Path(configured))
    candidates.extend([
        repository_root
        / "Tools/ClientFrontendHarness/Bin/Debug/ClientFrontendHarness.exe",
        repository_root
        / "Tools/ClientFrontendHarness/Bin/Release/ClientFrontendHarness.exe",
    ])
    for candidate in candidates:
        resolved = candidate.resolve()
        if resolved.is_file():
            return resolved
    raise ContractError(
        "EffectDocumentCodec identity tool is missing; build "
        "Tools/ClientFrontendHarness and/or set "
        "LOSTARK_EFFECT_DOCUMENT_CODEC_TOOL"
    )


def _compute_typed_codec_sha256_batch(
    repository_root: Path,
    document_paths: list[Path],
) -> list[str]:
    resolved_paths = [path.resolve() for path in document_paths]
    cache_key = tuple(raw_sha256(path) for path in resolved_paths)
    cached = _TYPED_CODEC_SHA_CACHE.get(cache_key)
    if cached is not None:
        return list(cached)
    tool = _typed_codec_tool(repository_root)
    environment = os.environ.copy()
    environment["LOSTARK_RESOURCE_ROOT"] = str(
        (repository_root / "Client/Bin/Resources").resolve()
    )
    completed = subprocess.run(
        [str(tool), "--effect-document-codec-sha",
         *(str(path) for path in resolved_paths)],
        cwd=repository_root / "Client/Default",
        env=environment,
        text=True,
        encoding="utf-8",
        errors="strict",
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=120,
        check=False,
    )
    if completed.returncode != 0:
        raise ContractError(
            "EffectDocumentCodec typed identity tool failed: "
            f"{completed.stdout.strip()} {completed.stderr.strip()}"
        )
    hashes: list[str] = []
    for line in completed.stdout.splitlines():
        fields = line.strip().split("\t")
        candidates = [field for field in fields if SHA256_RE.fullmatch(field)]
        if len(candidates) == 1:
            hashes.append(candidates[0])
    _require(
        len(hashes) == len(resolved_paths),
        "EffectDocumentCodec identity tool output denominator changed: "
        f"expected {len(resolved_paths)}, got {len(hashes)}",
    )
    result = tuple(hashes)
    _TYPED_CODEC_SHA_CACHE[cache_key] = result
    return list(result)


def _reject_non_finite(value: str) -> None:
    raise ContractError(f"non-finite JSON number is forbidden: {value}")


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(
            path.read_text(encoding="utf-8-sig"),
            parse_constant=_reject_non_finite,
        )
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ContractError(f"cannot parse JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise ContractError(f"JSON root must be an object: {path}")
    return value


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        allow_nan=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def canonical_json_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def raw_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for block in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(block)
    except OSError as error:
        raise ContractError(f"cannot hash {path}: {error}") from error
    return digest.hexdigest()


def _read_legacy_dds_descriptor(path: Path) -> dict[str, int]:
    try:
        header = path.read_bytes()[:148]
    except OSError as error:
        raise ContractError(f"cannot read DDS descriptor {path}: {error}") from error
    _require(
        len(header) >= 128 and header[:4] == b"DDS "
        and struct.unpack_from("<I", header, 4)[0] == 124,
        f"DDS header is invalid: {path}",
    )
    height, width = struct.unpack_from("<II", header, 12)
    raw_mip_count = struct.unpack_from("<I", header, 28)[0]
    four_cc = header[84:88]
    _require(four_cc != b"DX10", f"DX10 DDS array descriptor requires typed parser: {path}")
    _require(width > 0 and height > 0, f"DDS dimensions are invalid: {path}")
    return {
        "width": width,
        "height": height,
        "mipCount": max(1, raw_mip_count),
        "arraySize": 1,
    }


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, allow_nan=False, indent=2)
        + "\n"
    ).encode("utf-8")


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise ContractError(message)


def _require_dict(value: Any, label: str) -> dict[str, Any]:
    _require(isinstance(value, dict), f"{label} must be an object")
    return value


def _require_list(value: Any, label: str) -> list[Any]:
    _require(isinstance(value, list), f"{label} must be an array")
    return value


def _require_string(value: Any, label: str) -> str:
    _require(isinstance(value, str) and bool(value), f"{label} must be a string")
    return value


def _require_sha(value: Any, label: str) -> str:
    text = _require_string(value, label)
    _require(bool(SHA256_RE.fullmatch(text)), f"{label} must be lowercase SHA-256")
    return text


def _require_stable_id(value: Any, label: str) -> str:
    text = _require_string(value, label)
    _require(bool(STABLE_ID_RE.fullmatch(text)), f"{label} is not a stable ID")
    return text


def _expect_keys(value: dict[str, Any], expected: Iterable[str], label: str) -> None:
    expected_set = set(expected)
    actual = set(value)
    _require(
        actual == expected_set,
        f"{label} keys mismatch: missing={sorted(expected_set - actual)} "
        f"extra={sorted(actual - expected_set)}",
    )


def _seal(value: dict[str, Any], field: str) -> None:
    _require(field not in value, f"value already sealed: {field}")
    value[field] = canonical_json_sha256(value)


def _verify_seal(value: dict[str, Any], field: str, label: str) -> None:
    expected = _require_sha(value.get(field), f"{label}.{field}")
    unsigned = copy.deepcopy(value)
    del unsigned[field]
    _require(canonical_json_sha256(unsigned) == expected, f"{label}.{field} is stale")


def _load_source_corpus(repository_root: Path) -> tuple[dict[str, Any], str]:
    path = repository_root / SOURCE_CORPUS_RELATIVE_PATH
    _require(path.is_file(), f"missing phase1 visual corpus: {SOURCE_CORPUS_RELATIVE_PATH}")
    corpus = load_json(path)
    try:
        phase1.validate_corpus(corpus, repository_root)
    except phase1.ContractError as error:
        raise ContractError(f"phase1 visual corpus rejected: {error}") from error
    return corpus, raw_sha256(path)


def _find_element(document: dict[str, Any], element_id: str, label: str) -> dict[str, Any]:
    matches = [
        item for item in document.get("elements", [])
        if isinstance(item, dict) and item.get("id") == element_id
    ]
    _require(len(matches) == 1, f"{label} element join is not unique: {element_id}")
    return matches[0]


def _load_payload_record(
    repository_root: Path,
    payload: dict[str, Any],
) -> dict[str, Any]:
    path_text = _require_string(payload.get("path"), "payload.path").replace("\\", "/")
    relative = PurePosixPath(path_text)
    _require(not relative.is_absolute() and ".." not in relative.parts, "payload path escapes repository")
    path = repository_root / relative
    physical_raw_sha = raw_sha256(path)
    payload_raw_sha = payload.get("rawSha256")
    if physical_raw_sha != payload_raw_sha:
        # Phase 1 owns the only bounded exception: the four legacy Whirlwind
        # supplemental rows retain their original LF payload identities while
        # the frozen authoring/history canaries are checked out as CRLF.  Reuse
        # the phase-1 validator so no other payload can bypass raw-byte drift.
        try:
            phase1._validate_payload_ref(payload, repository_root, "runtime payload")
        except phase1.ContractError as error:
            raise ContractError(f"payload raw SHA changed: {path_text}") from error
    document = load_json(path)
    record_id = _require_string(payload.get("recordId"), "payload.recordId")
    if "emitters" in document and path_text.endswith("runtime-program.candidate.json"):
        matches = [item for item in document["emitters"] if item.get("evidenceId") == record_id]
    elif "document" in document:
        matches = [item for item in document["document"].get("elements", []) if item.get("id") == record_id]
    else:
        matches = [item for item in document.get("elements", []) if item.get("id") == record_id]
    _require(len(matches) == 1, f"payload record join is not unique: {path_text}/{record_id}")
    record = _require_dict(matches[0], "payload record")
    if "rowSha256" in record:
        expected = record["rowSha256"]
        unsigned = copy.deepcopy(record)
        del unsigned["rowSha256"]
        _require(canonical_json_sha256(unsigned) == expected, f"payload sealed row is stale: {record_id}")
        record_sha = expected
    else:
        record_sha = canonical_json_sha256(record)
    _require(record_sha == payload.get("recordSha256"), f"payload record SHA changed: {record_id}")
    return record


def _resolve_overlay_base_document(
    repository_root: Path,
    effect_asset_id: str,
    rows: list[dict[str, Any]],
    supplemental_source: list[dict[str, Any]],
    stages: dict[str, dict[str, Any]],
) -> tuple[Path, str]:
    """Resolve the single sealed authoring document owned by an overlay program."""
    if effect_asset_id == VALTAN_WHIRLWIND_EFFECT_ASSET_ID:
        path = repository_root / PurePosixPath(
            VALTAN_WHIRLWIND_DOCUMENT_RELATIVE_PATH
        )
        return path, phase1.VALTAN_WHIRLWIND_LEGACY_DOCUMENT_PAYLOAD_RAW_SHA256

    stage_value = stages.get(effect_asset_id)
    if stage_value is not None:
        stage = _require_dict(stage_value, f"BA stage {effect_asset_id}")
        current = _require_dict(stage.get("currentProduct"), "stage.currentProduct")
        path_text = _require_string(current.get("authoringPath"), "authoringPath")
        path = repository_root / PurePosixPath(path_text)
        expected_raw = _require_sha(
            current.get("authoringRawSha256"), "authoringRawSha256"
        )
        return path, expected_raw

    allowed_supplemental_scopes = {
        "VALTAN_SAFE_REVIEWED_GAP_ANIMATION_TRAIL",
        "ARTIST_T_CASCADE_RIBBON",
    }
    _require(
        not rows
        and supplemental_source
        and all(
            (item.get("provenance") or {}).get("scope")
            in allowed_supplemental_scopes
            for item in supplemental_source
        ),
        f"non-BA supplemental program has no sealed ownership: {effect_asset_id}",
    )
    target_payloads = [
        _require_dict(item.get("targetPayload"), "supplemental targetPayload")
        for item in supplemental_source
    ]
    target_paths = {
        _require_string(item.get("path"), "supplemental target path")
        for item in target_payloads
    }
    target_hashes = {
        _require_sha(item.get("rawSha256"), "supplemental target raw SHA")
        for item in target_payloads
    }
    _require(
        len(target_paths) == 1 and len(target_hashes) == 1,
        f"supplemental target join is not unique: {effect_asset_id}",
    )
    path = repository_root / PurePosixPath(next(iter(target_paths)))
    return path, next(iter(target_hashes))


def _validate_overlay_base_raw_sha(
    repository_root: Path,
    path: Path,
    expected_raw_sha: str,
    effect_asset_id: str,
    corpus: dict[str, Any],
) -> None:
    physical_raw_sha = raw_sha256(path)
    if physical_raw_sha == expected_raw_sha:
        return
    _require(
        effect_asset_id == VALTAN_WHIRLWIND_EFFECT_ASSET_ID
        and expected_raw_sha
        == phase1.VALTAN_WHIRLWIND_LEGACY_DOCUMENT_PAYLOAD_RAW_SHA256,
        f"BA base document raw SHA changed: {effect_asset_id}",
    )
    relative_path = path.resolve().relative_to(repository_root.resolve()).as_posix()
    input_matches = [
        item
        for item in _require_list(corpus.get("inputArtifacts"), "corpus.inputArtifacts")
        if isinstance(item, dict) and item.get("path") == relative_path
    ]
    _require(
        len(input_matches) == 1
        and input_matches[0].get("rawSha256") == physical_raw_sha,
        "Whirlwind physical authoring canary diverged from the sealed corpus input",
    )


def validate_standard_document(document: dict[str, Any], expected_id: str) -> None:
    _require(
        document.get("schema") == "lostark.effect-authoring"
        and isinstance(document.get("version"), int)
        and 12 <= document["version"] <= 14
        and document.get("effectAssetId") == expected_id,
        f"standard Effect document header mismatch: {expected_id}",
    )
    elements = _require_list(document.get("elements"), f"{expected_id}.elements")
    element_ids = [_require_string(item.get("id"), "element.id") for item in elements if isinstance(item, dict)]
    _require(len(element_ids) == len(elements) == len(set(element_ids)), f"duplicate/invalid Effect element: {expected_id}")
    for element in elements:
        item = _require_dict(element, "Effect element")
        recipe = _require_dict(item.get("sourceRecipe"), "Effect sourceRecipe")
        _require(isinstance(recipe.get("enabled"), bool), "sourceRecipe.enabled must be boolean")
        _require(isinstance(recipe.get("modules"), list), "sourceRecipe.modules must be an array")


def project_ba_document(
    repository_root: Path,
    base_document: dict[str, Any],
    phase1_rows: list[dict[str, Any]],
    supplemental_elements: list[dict[str, Any]] | None = None,
) -> dict[str, Any]:
    """Return an all-or-nothing SourceRecipe overlay; never mutates input."""
    effect_asset_id = _require_stable_id(base_document.get("effectAssetId"), "base effectAssetId")
    validate_standard_document(base_document, effect_asset_id)
    staged = copy.deepcopy(base_document)
    seen_targets: set[str] = set()
    for row in phase1_rows:
        selector = _require_dict(row.get("selector"), "phase1 selector")
        _expect_keys(selector, ["effectAssetId", "occurrenceId"], "phase1 selector")
        _require(selector.get("effectAssetId") == effect_asset_id, "projection row/effect mismatch")
        execution = _require_dict(row.get("executionProjection"), "phase1 executionProjection")
        if execution.get("disposition") == "FAIL_CLOSED":
            _require(row.get("targetPayload") is None, "fail-closed row cannot mutate a target")
            continue
        family = execution.get("family")
        _require(
            execution.get("disposition") == "ADMITTED_BOUNDED"
            and execution.get("fidelity") in {
                "LEGACY_APPROXIMATION", "BOUNDED_RECONSTRUCTION"
            }
            and execution.get("packetLayout") == "EFFECT_DOCUMENT_ELEMENT_V12"
            and (
                execution.get("fidelity") == "LEGACY_APPROXIMATION" or
                family == "CASCADE_RIBBON"
            ),
            "BA overlay row is not a bounded carrier projection",
        )
        source_record = _load_payload_record(
            repository_root,
            _require_dict(row.get("sourcePayload"), "sourcePayload"),
        )
        target_payload = _require_dict(row.get("targetPayload"), "targetPayload")
        _load_payload_record(repository_root, target_payload)
        target_id = _require_string(target_payload.get("recordId"), "target recordId")
        _require(target_id not in seen_targets, f"duplicate SourceRecipe overlay target: {target_id}")
        seen_targets.add(target_id)
        if family == "CASCADE_RIBBON":
            target_matches = [
                item for item in staged.get("elements", [])
                if isinstance(item, dict) and item.get("id") == target_id
            ]
            _require(not target_matches, f"CascadeRibbon target already exists: {target_id}")
            target_element = copy.deepcopy(_load_payload_record(repository_root, target_payload))
            _require(target_element.get("kind") == "trail", "CascadeRibbon target is not a trail")
            # The restoration-candidate row was intentionally hidden while the
            # legacy importer classified TypeDataRibbon as a Sprite.  Admission
            # is now owned by the exact TypeDataRibbon visual-program row, so
            # materializing the projected Trail must make that stable target
            # visible.  The imported/restoration source document remains
            # immutable.
            target_element["visible"] = True
            staged["elements"].append(target_element)
        else:
            target_element = _find_element(staged, target_id, effect_asset_id)
        source_recipe = _require_dict(source_record.get("sourceRecipe"), "source record SourceRecipe")
        recipe_identity = _require_dict(row.get("sourceRecipe"), "phase1 SourceRecipe identity")
        _require(source_recipe.get("enabled") is True, f"admitted SourceRecipe is disabled: {target_id}")
        projected_source_recipe = copy.deepcopy(source_recipe)
        projected_source_recipe["rendererShape"] = recipe_identity.get(
            "resolvedRendererShape"
        )
        _require(
            canonical_json_sha256(source_recipe) ==
                recipe_identity.get("sourceRecipeEvidenceSha256") and
            canonical_json_sha256(projected_source_recipe) ==
                recipe_identity.get("recipeSha256"),
            f"SourceRecipe SHA changed: {target_id}",
        )
        modules = _require_list(source_recipe.get("modules"), "SourceRecipe.modules")
        _require(
            len(modules) == recipe_identity.get("moduleCount")
            and canonical_json_sha256(modules) == recipe_identity.get("moduleClosureSha256"),
            f"SourceRecipe module closure changed: {target_id}",
        )
        expected_kind = {
            "MESH_PARTICLE": "mesh",
            "SPRITE_PARTICLE": "sprite",
            "CASCADE_RIBBON": "trail",
        }.get(family)
        expected_shape = {
            "MESH_PARTICLE": "mesh",
            "SPRITE_PARTICLE": "sprite",
            "CASCADE_RIBBON": "ribbon",
        }.get(family)
        _require(
            family in {"MESH_PARTICLE", "SPRITE_PARTICLE", "CASCADE_RIBBON"}
            and target_element.get("kind") == expected_kind
            and projected_source_recipe.get("rendererShape") == expected_shape,
            f"SourceRecipe/carrier family mismatch: {target_id}",
        )
        if family == "CASCADE_RIBBON":
            typed_modules = [
                module for module in modules
                if isinstance(module, dict) and
                str(module.get("className", "")).lower() ==
                    "particlemoduletypedataribbon"
            ]
            _require(len(typed_modules) == 1, "CascadeRibbon TypeData module is not unique")
            literals = {
                item.get("propertyPath"): item.get("value")
                for item in typed_modules[0].get("literals", [])
                if isinstance(item, dict) and item.get("kind") == "number"
            }
            _require(
                isinstance(literals.get("tilingdistance"), (int, float)) and
                isinstance(literals.get("distancetessellationstepsize"), (int, float)),
                "CascadeRibbon typed distance literals are missing",
            )
            trail_detail = _require_dict(
                target_element.get("detail", {}).get("trail"),
                "CascadeRibbon target trail detail",
            )
            trail_detail["tilingDistanceWorldUnits"] = (
                float(literals["tilingdistance"]) * 0.01
            )
            trail_detail["distanceTessellationStepWorldUnits"] = (
                float(literals["distancetessellationstepsize"]) * 0.01
            )
        target_element["sourceRecipe"] = projected_source_recipe
    for supplemental in supplemental_elements or []:
        family = supplemental.get("family")
        _require(
            family in {"ANIMATION_TRAIL", "CASCADE_RIBBON", "LIGHT_PARTICLE"}
            and supplemental.get("disposition") == "ADMITTED_BOUNDED",
            "BA supplemental projection contains an unsupported row",
        )
        target_payload = _require_dict(
            supplemental.get("targetPayload"), "supplemental targetPayload"
        )
        target = copy.deepcopy(_load_payload_record(repository_root, target_payload))
        source_target = copy.deepcopy(target)
        target_id = _require_string(target.get("id"), "supplemental target ID")
        _require(target_id not in seen_targets, f"duplicate supplemental target: {target_id}")
        existing = [
            (index, item)
            for index, item in enumerate(staged.get("elements", []))
            if isinstance(item, dict) and item.get("id") == target_id
        ]
        source_recipe = _require_dict(
            target.get("sourceRecipe"), "supplemental target SourceRecipe"
        )
        if family == "CASCADE_RIBBON":
            source_payload = _require_dict(
                supplemental.get("sourcePayload"),
                "CascadeRibbon supplemental sourcePayload",
            )
            packet = _require_dict(
                supplemental.get("cascadeRibbonPacket"),
                "CascadeRibbon packet",
            )
            unsigned_packet = copy.deepcopy(packet)
            packet_sha = _require_sha(
                unsigned_packet.pop("packetSha256", None),
                "CascadeRibbon packetSha256",
            )
            trail = _require_dict(
                _require_dict(target.get("detail"), "CascadeRibbon detail").get(
                    "trail"
                ),
                "CascadeRibbon trail",
            )
            timing = _require_dict(
                target.get("detail", {}).get("timing"),
                "CascadeRibbon timing",
            )
            if effect_asset_id == "effect.artist.skill.31950.unified":
                phase1._validate_artist_t_ribbon_material_target(target)
                material_boundary_valid = True
            else:
                material_boundary_valid = (
                    target.get("resources") == []
                    and target.get("material", {}).get("execution")
                        == {"enabled": False, "failClosed": True}
                )
            _require(
                supplemental.get("packetLayout")
                    == "CASCADE_RIBBON_TYPED_PACKET_V1"
                and source_payload == target_payload
                and len(existing) == 1
                and existing[0][1] == source_target
                and target.get("kind") == "trail"
                and target.get("visible") is True
                and material_boundary_valid
                and source_recipe.get("enabled") is True
                and source_recipe.get("rendererShape") == "ribbon"
                and packet.get("runtimeCarrier")
                    == "EFFECT_TYPED_CASCADE_RIBBON_V1"
                and packet.get("boundedSemanticReplay") is True
                and packet.get("nativeExecution") is False
                and packet.get("resolvedRendererShape") == "ribbon"
                and packet.get("tilingDistance")
                    == trail.get("tilingDistanceWorldUnits")
                and packet.get("distanceTessellationStepSize")
                    == trail.get("distanceTessellationStepWorldUnits")
                and packet.get("targetTiming") == timing
                and packet.get("attachment")
                    == target.get("actionCueAttachment")
                and packet.get("trail") == trail
                and packet.get("sourceRecipeSha256")
                    == canonical_json_sha256(source_recipe)
                and packet.get("moduleClosureSha256")
                    == canonical_json_sha256(source_recipe.get("modules"))
                and packet.get("moduleCount")
                    == len(_require_list(
                        source_recipe.get("modules"),
                        "CascadeRibbon SourceRecipe.modules",
                    ))
                and canonical_json_sha256(unsigned_packet) == packet_sha,
                "CascadeRibbon carrier/material projection boundary changed",
            )
            seen_targets.add(target_id)
            continue
        if family == "LIGHT_PARTICLE":
            packet = _require_dict(
                supplemental.get("bakedEdgeLightPacket"),
                "baked-edge Light packet",
            )
            _require(
                supplemental.get("packetLayout")
                    == "LIGHT_BAKED_EDGE_ATTACHMENT_V1"
                and target.get("kind") == "light"
                and target.get("visible") is False
                and source_recipe.get("enabled") is False
                and source_recipe.get("modules") == []
                and len(existing) == 1
                and existing[0][1] == source_target,
                "baked-edge Light base target identity changed",
            )
            target_light = copy.deepcopy(
                _require_dict(packet.get("targetLight"), "targetLight")
            )
            base_light = copy.deepcopy(_require_dict(
                target.get("detail", {}).get("light"),
                "baked-edge Light base profile",
            ))
            base_light["enabled"] = True
            _require(
                base_light == target_light
                and target_light.get("enabled") is True,
                "baked-edge Light projected profile diverged from quarantine",
            )
            target["visible"] = True
            target["detail"]["timing"]["startDelaySeconds"] = packet.get(
                "activeStartSeconds"
            )
            target["detail"]["timing"]["lifeTimeSeconds"] = packet.get(
                "activeDurationSeconds"
            )
            target["detail"]["light"] = target_light
            staged["elements"][existing[0][0]] = target
            seen_targets.add(target_id)
            continue
        packet = _require_dict(
            supplemental.get("animationTrailPacket"),
            "AnimationTrail packet",
        )
        baked_edge = (
            supplemental.get("packetLayout")
            == "ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1"
        )
        target["detail"]["trail"].setdefault("tilingDistanceWorldUnits", 0.0)
        target["detail"]["trail"].setdefault(
            "distanceTessellationStepWorldUnits", 0.0
        )
        if baked_edge:
            module_classes = {
                str(module.get("className", "")).casefold()
                for module in _require_list(
                    source_recipe.get("modules"),
                    "baked AnimationTrail source modules",
                )
                if isinstance(module, dict)
            }
            _require(
                target.get("kind") == "trail"
                and source_recipe.get("enabled") is False
                and "particlemoduletypedataanimtrail" in module_classes
                and len(existing) == 1
                and existing[0][1] == source_target,
                "baked AnimationTrail base target identity changed",
            )
            target["visible"] = True
            target["sourceRecipe"]["modules"] = []
            target["detail"]["timing"] = copy.deepcopy(
                _require_dict(packet.get("targetTiming"), "targetTiming")
            )
            target["detail"]["trail"].update(copy.deepcopy(
                _require_dict(packet.get("trail"), "baked AnimationTrail trail")
            ))
            staged["elements"][existing[0][0]] = target
        else:
            _require(
                target.get("kind") == "trail"
                and source_recipe.get("enabled") is False
                and source_recipe.get("modules") == [],
                "AnimationTrail target is not a bounded authored trail",
            )
            _require(
                not existing,
                f"AnimationTrail target already exists: {target_id}",
            )
            staged["elements"].append(target)
        seen_targets.add(target_id)
    staged["elements"].sort(key=lambda item: item["id"])
    _require(bool(seen_targets), f"BA projection has no admitted overlay: {effect_asset_id}")
    validate_standard_document(staged, effect_asset_id)
    return staged


def _mask_bit(mask: tuple[int, ...], index: int) -> bool:
    word = index // 32
    return word < len(mask) and bool(mask[word] & (1 << (index % 32)))


def _local_decal_packet_inputs(
    program: dict[str, Any],
    recipe: dict[str, Any],
) -> tuple[list[dict[str, Any]], list[dict[str, Any]], list[dict[str, Any]]]:
    input_by_id = {
        item.get("fieldId"): item
        for item in _require_list(program.get("materialInputs"), "materialInputs")
        if isinstance(item, dict)
    }
    input_rows: list[dict[str, Any]] = []
    packed_scalars: list[dict[str, Any]] = []
    packed_vectors: list[dict[str, Any]] = []
    input_ids = _require_list(recipe.get("inputIds"), "LocalDecal recipe.inputIds")
    _require(len(input_ids) == 33, "LocalDecal input denominator changed")
    for index, input_id_value in enumerate(input_ids):
        input_id = _require_string(input_id_value, "LocalDecal inputId")
        source = _require_dict(input_by_id.get(input_id), f"LocalDecal input {input_id}")
        _verify_seal(source, "rowSha256", f"LocalDecal input {input_id}")
        _require(source.get("recipeId") == recipe.get("recipeId"), "LocalDecal input/recipe mismatch")
        consumed = _mask_bit(LOCAL_DECAL_INPUT_CONSUMED_MASK, index)
        suppressed = _mask_bit(LOCAL_DECAL_INPUT_SUPPRESSED_MASK, index)
        _require(consumed != suppressed, f"LocalDecal input disposition is incomplete: {input_id}")
        variant = source.get("valueVariant")
        scalar_value: float | None = None
        vector_value: list[float] | None = None
        texture_id: str | None = None
        scalar_index: int | None = None
        vector_index: int | None = None
        if variant == "F64":
            scalar_value = source.get("valueF64")
            _require(isinstance(scalar_value, (int, float)) and not isinstance(scalar_value, bool), f"LocalDecal scalar is invalid: {input_id}")
            scalar_value = float(scalar_value)
            scalar_index = len(packed_scalars)
            packed_scalars.append({"inputId": input_id, "value": scalar_value})
        elif variant == "F64X4":
            raw_vector = source.get("valueF64x4")
            _require(
                isinstance(raw_vector, list) and len(raw_vector) == 4
                and all(isinstance(item, (int, float)) and not isinstance(item, bool) for item in raw_vector),
                f"LocalDecal vector is invalid: {input_id}",
            )
            vector_value = [float(item) for item in raw_vector]
            vector_index = len(packed_vectors)
            packed_vectors.append({"inputId": input_id, "value": vector_value})
        elif variant == "TEXTURE_ID":
            texture_id = _require_string(source.get("valueTextureId"), f"LocalDecal texture {input_id}")
        else:
            raise ContractError(f"LocalDecal input variant is unsupported: {input_id}/{variant}")
        input_rows.append({
            "inputId": input_id,
            "normalizedParameterName": _require_string(source.get("normalizedParameterName"), "LocalDecal parameter name"),
            "rowSha256": _require_sha(source.get("rowSha256"), "LocalDecal input row SHA"),
            "valueVariant": variant,
            "disposition": "CONSUMED" if consumed else "SUPPRESSED",
            "scalarValue": scalar_value,
            "vectorValue": vector_value,
            "textureId": texture_id,
            "packedScalarIndex": scalar_index,
            "packedVectorIndex": vector_index,
        })
    _require(len(packed_scalars) == 22 and len(packed_vectors) == 3, "LocalDecal packed value denominator changed")
    return input_rows, packed_scalars, packed_vectors


def _local_decal_static_dispositions(
    program: dict[str, Any],
    recipe: dict[str, Any],
) -> list[dict[str, Any]]:
    by_id = {
        item.get("fieldId"): item
        for item in _require_list(program.get("materialStaticBindings"), "materialStaticBindings")
        if isinstance(item, dict)
    }
    result: list[dict[str, Any]] = []
    selected_mask = 0
    static_ids = _require_list(recipe.get("staticBindingIds"), "LocalDecal recipe.staticBindingIds")
    _require(len(static_ids) == 18, "LocalDecal static denominator changed")
    for index, binding_id_value in enumerate(static_ids):
        binding_id = _require_string(binding_id_value, "LocalDecal static bindingId")
        source = _require_dict(by_id.get(binding_id), f"LocalDecal static {binding_id}")
        _verify_seal(source, "rowSha256", f"LocalDecal static {binding_id}")
        _require(
            source.get("recipeId") == recipe.get("recipeId")
            and isinstance(source.get("sourceValue"), bool)
            and isinstance(source.get("selectedValue"), bool),
            f"LocalDecal static binding is invalid: {binding_id}",
        )
        if source["selectedValue"]:
            selected_mask |= 1 << index
        result.append({
            "bindingId": binding_id,
            "normalizedParameterName": _require_string(source.get("normalizedParameterName"), "LocalDecal static parameter"),
            "rowSha256": _require_sha(source.get("rowSha256"), "LocalDecal static row SHA"),
            "sourceValue": source["sourceValue"],
            "selectedValue": source["selectedValue"],
            "policyRowId": _require_string(source.get("policyRowId"), "LocalDecal static policy row"),
            "disposition": "CONSUMED",
        })
    _require(selected_mask == LOCAL_DECAL_STATIC_SELECTED_MASK, "LocalDecal selected static mask changed")
    return result


def _local_decal_render_dispositions(
    program: dict[str, Any],
    recipe: dict[str, Any],
) -> list[dict[str, Any]]:
    by_id = {
        item.get("renderBindingId"): item
        for item in _require_list(program.get("materialRenderBindings"), "materialRenderBindings")
        if isinstance(item, dict)
    }
    result: list[dict[str, Any]] = []
    render_ids = _require_list(recipe.get("renderBindingIds"), "LocalDecal recipe.renderBindingIds")
    _require(len(render_ids) == 6, "LocalDecal render denominator changed")
    for index, binding_id_value in enumerate(render_ids):
        binding_id = _require_string(binding_id_value, "LocalDecal render bindingId")
        source = _require_dict(by_id.get(binding_id), f"LocalDecal render {binding_id}")
        _verify_seal(source, "rowSha256", f"LocalDecal render {binding_id}")
        consumed = bool(LOCAL_DECAL_RENDER_CONSUMED_MASK & (1 << index))
        suppressed = bool(LOCAL_DECAL_RENDER_SUPPRESSED_MASK & (1 << index))
        _require(consumed != suppressed, f"LocalDecal render disposition is incomplete: {binding_id}")
        variant = source.get("valueVariant")
        _require(variant in {"BOOL", "ENUM_STRING", "F64"}, f"LocalDecal render variant changed: {binding_id}")
        result.append({
            "bindingId": binding_id,
            "fieldName": _require_string(source.get("fieldName"), "LocalDecal render field"),
            "rowSha256": _require_sha(source.get("rowSha256"), "LocalDecal render row SHA"),
            "valueVariant": variant,
            "boolValue": source.get("boolValue"),
            "enumValue": source.get("enumValue") or None,
            "f64Value": source.get("f64Value"),
            "sourceStatus": _require_string(source.get("sourceStatus"), "LocalDecal render source status"),
            "sourceFidelity": _require_string(source.get("sourceFidelity"), "LocalDecal render source fidelity"),
            "disposition": "CONSUMED" if consumed else "SUPPRESSED",
        })
    return result


def _local_decal_srvs(
    repository_root: Path,
    resource_packet: list[dict[str, Any]],
    receipt: dict[str, Any],
) -> list[dict[str, Any]]:
    source_sampler_by_register = {
        item.get("sourceShaderRegister"): item.get("sourceShaderSampler")
        for item in _require_list(receipt.get("assets"), "LocalDecal receipt assets")
        if isinstance(item, dict)
    }
    result: list[dict[str, Any]] = []
    _require(len(resource_packet) == 6, "LocalDecal SRV denominator changed")
    for index, resource_value in enumerate(resource_packet):
        resource = _require_dict(resource_value, f"LocalDecal resource {index}")
        role = _require_string(resource.get("role"), "LocalDecal resource role")
        _require(role in LOCAL_DECAL_SRV_FORMATS, f"LocalDecal SRV role is unsupported: {role}")
        linear_format, srgb = LOCAL_DECAL_SRV_FORMATS[role]
        register = _require_string(resource.get("shaderRegister"), "LocalDecal shader register")
        _require(register == f"t{index}", "LocalDecal SRV register order changed")
        asset_id = _require_string(resource.get("assetId"), "LocalDecal SRV assetId")
        descriptor = _read_legacy_dds_descriptor(
            repository_root / "Client/Bin/Resources" / PurePosixPath(asset_id)
        )
        result.append({
            "role": role,
            "assetId": asset_id,
            "rawSha256": _require_sha(resource.get("rawSha256"), "LocalDecal SRV SHA"),
            "byteCount": resource.get("byteCount"),
            "shaderRegister": register,
            "sourceChannel": _require_string(resource.get("sourceChannel"), "LocalDecal source channel"),
            "runtimeSamplerRegister": f"s{5 + index}",
            "sourceSamplerEvidence": source_sampler_by_register.get(register),
            "samplerPolicy": "LINEAR_CLAMP_UVW_BOUNDED_V1",
            "linearFormat": linear_format,
            "srgb": srgb,
            **descriptor,
        })
    _require(
        result[0]["sourceSamplerEvidence"] == "s0"
        and result[2]["sourceSamplerEvidence"] == "s5"
        and all(result[index]["sourceSamplerEvidence"] is None for index in (1, 3, 4, 5)),
        "LocalDecal source sampler evidence boundary changed",
    )
    return result


def _build_local_decal_packet(
    repository_root: Path,
    phase1_row: dict[str, Any],
    program: dict[str, Any],
    receipt: dict[str, Any],
) -> dict[str, Any]:
    selector = _require_dict(phase1_row.get("selector"), "LocalDecal selector")
    _require(selector.get("effectAssetId") == "effect.artist.skill.31470", "LocalDecal selector asset changed")
    target_payload = _require_dict(phase1_row.get("targetPayload"), "LocalDecal targetPayload")
    occurrence_id = _require_string(selector.get("occurrenceId"), "LocalDecal occurrenceId")
    emitters = [
        item for item in _require_list(program.get("emitters"), "LocalDecal emitters")
        if isinstance(item, dict) and item.get("evidenceId") == occurrence_id
    ]
    _require(len(emitters) == 1, f"LocalDecal emitter join changed: {occurrence_id}")
    emitter = emitters[0]
    _verify_seal(emitter, "rowSha256", f"LocalDecal emitter {occurrence_id}")
    _require(
        emitter.get("rowSha256") == target_payload.get("recordSha256")
        and emitter.get("rendererType") == "DecalParticle",
        f"LocalDecal target emitter identity changed: {occurrence_id}",
    )
    recipe_id = _require_string(emitter.get("materialOccurrenceId"), "LocalDecal occurrence row id")
    occurrences = [
        item for item in _require_list(program.get("materialOccurrences"), "LocalDecal materialOccurrences")
        if isinstance(item, dict) and item.get("occurrenceId") == recipe_id
    ]
    _require(len(occurrences) == 1, f"LocalDecal material occurrence join changed: {occurrence_id}")
    occurrence = occurrences[0]
    _verify_seal(occurrence, "rowSha256", f"LocalDecal material occurrence {occurrence_id}")
    recipes = [
        item for item in _require_list(program.get("materialRecipes"), "LocalDecal materialRecipes")
        if isinstance(item, dict) and item.get("recipeId") == occurrence.get("recipeId")
    ]
    _require(len(recipes) == 1, f"LocalDecal material recipe join changed: {occurrence_id}")
    recipe = recipes[0]
    _verify_seal(recipe, "rowSha256", f"LocalDecal material recipe {occurrence_id}")
    _require(
        recipe.get("recipeId") == "material-recipe-a0a4fd34c2f220dc"
        and occurrence.get("familyId") == "material-family-f1667adae7da4bdd"
        and occurrence.get("bindingSha256") == recipe.get("bindingSha256"),
        f"LocalDecal material identity changed: {occurrence_id}",
    )
    input_rows, packed_scalars, packed_vectors = _local_decal_packet_inputs(program, recipe)
    static_rows = _local_decal_static_dispositions(program, recipe)
    render_rows = _local_decal_render_dispositions(program, recipe)
    execution = _require_dict(phase1_row.get("executionProjection"), "LocalDecal executionProjection")
    native_shader = _require_dict(receipt.get("nativeShaderEvidence"), "LocalDecal native shader evidence")
    packet = {
        "packetVersion": 1,
        "adapterId": "local-decal-rt0-bounded-v1",
        "boundedSemanticReplay": True,
        "nativeExecution": False,
        "nativeVertexFactoryAdmitted": False,
        "nativeMrtAdmitted": False,
        "runtimeCarrier": "EFFECT_TYPED_DECAL_PROJECTOR_RECT_V1",
        "nativeVertexFactoryCandidate": _require_string(native_shader.get("vertexFactoryCandidate"), "LocalDecal native VF candidate"),
        "nativeVertexShaderSha256": _require_sha(native_shader.get("vertexDxbcSha256"), "LocalDecal native VS SHA"),
        "nativePixelShaderSha256": _require_sha(native_shader.get("pixelDxbcSha256"), "LocalDecal native PS SHA"),
        "renderProfile": "ALPHA_ONE_SIDED_DEPTH_READ",
        "passIndex": 3,
        "pipelineState": {
            "rasterizer": "RS_Default",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAlpha",
            "stencilReference": 0,
        },
        "opcode": 14,
        "textureLaneCount": 6,
        "textureMask": 0x3F,
        "dynamicConsumedMask": 0,
        "dynamicSuppressedMask": 0x0F,
        "particleColorPolicy": 0,
        "particleColorConsumedMask": 0,
        "particleColorSuppressedMask": 0,
        "inputConsumedMask": list(LOCAL_DECAL_INPUT_CONSUMED_MASK),
        "inputSuppressedMask": list(LOCAL_DECAL_INPUT_SUPPRESSED_MASK),
        "vectorComponentConsumedMask": list(LOCAL_DECAL_VECTOR_CONSUMED_MASK),
        "vectorComponentSuppressedMask": list(LOCAL_DECAL_VECTOR_SUPPRESSED_MASK),
        "staticSelectedMask": LOCAL_DECAL_STATIC_SELECTED_MASK,
        "staticConsumedMask": LOCAL_DECAL_STATIC_CONSUMED_MASK,
        "staticSuppressedMask": 0,
        "renderConsumedMask": LOCAL_DECAL_RENDER_CONSUMED_MASK,
        "renderSuppressedMask": LOCAL_DECAL_RENDER_SUPPRESSED_MASK,
        "inputDispositions": input_rows,
        "staticDispositions": static_rows,
        "renderDispositions": render_rows,
        "packedScalars": packed_scalars,
        "packedVectors": packed_vectors,
        "srvs": _local_decal_srvs(
            repository_root,
            copy.deepcopy(execution.get("resourceRoles", [])), receipt),
        "preservedLimitations": copy.deepcopy(execution.get("preservedLimitations", [])),
    }
    _require(
        packet["nativeVertexFactoryCandidate"] == "flocaldecalvertexfactory"
        and receipt.get("decision", {}).get("nativeVfPassAdmission") is False
        and receipt.get("decision", {}).get("nativeMrtAdmission") is False
        and receipt.get("decision", {}).get("boundedSemanticReplayEligible") is True,
        "LocalDecal bounded/native admission boundary changed",
    )
    _seal(packet, "packetSha256")
    return packet


def _runtime_row(
    phase1_row: dict[str, Any],
    local_decal_packet: dict[str, Any] | None = None,
) -> dict[str, Any]:
    selector = copy.deepcopy(_require_dict(phase1_row.get("selector"), "selector"))
    _expect_keys(selector, ["effectAssetId", "occurrenceId"], "selector")
    provenance = _require_dict(phase1_row.get("provenance"), "provenance")
    recipe = _require_dict(phase1_row.get("sourceRecipe"), "sourceRecipe")
    source_payload = _require_dict(phase1_row.get("sourcePayload"), "sourcePayload")
    execution = _require_dict(phase1_row.get("executionProjection"), "executionProjection")
    target_payload = phase1_row.get("targetPayload")
    if target_payload is None:
        target_identity = None
    else:
        target = _require_dict(target_payload, "targetPayload")
        target_element_id = (
            provenance.get("sourceElementId")
            if execution.get("adapterId") == "local-decal-rt0-bounded-v1"
            else target.get("recordId")
        )
        target_identity = {
            "targetElementId": _require_string(target_element_id, "targetElementId"),
            "targetRecordSha256": _require_sha(target.get("recordSha256"), "target record SHA"),
            "targetPayloadRawSha256": _require_sha(target.get("rawSha256"), "target payload SHA"),
        }
    row = {
        "selector": selector,
        "selectorSha256": canonical_json_sha256(selector),
        "family": execution.get("family"),
        "adapterId": execution.get("adapterId"),
        "packetLayout": execution.get("packetLayout"),
        "fidelity": execution.get("fidelity"),
        "disposition": execution.get("disposition"),
        "tuningEligibleTransform": execution.get("tuningEligibleTransform"),
        "sourceIdentity": {
            "sourceRowId": provenance.get("sourceRowId"),
            "sourceRowSha256": recipe.get("sourceRowSha256"),
            "sourceRecordId": source_payload.get("recordId"),
            "sourceRecordSha256": source_payload.get("recordSha256"),
            "sourceRecipeSha256": recipe.get("recipeSha256"),
            "moduleClosureSha256": recipe.get("moduleClosureSha256"),
            "moduleCount": recipe.get("moduleCount"),
        },
        "targetIdentity": target_identity,
        "resourcePacket": copy.deepcopy(execution.get("resourceRoles", [])),
        "localDecalPacket": copy.deepcopy(local_decal_packet),
        "admissionBlockers": copy.deepcopy(execution.get("admissionBlockers", [])),
    }
    _seal(row, "rowSha256")
    return row


def _runtime_supplemental_element(
    phase1: dict[str, Any],
) -> dict[str, Any]:
    selector = copy.deepcopy(_require_dict(phase1.get("selector"), "supplemental selector"))
    _expect_keys(selector, ["effectAssetId", "occurrenceId"], "supplemental selector")
    source = _require_dict(phase1.get("sourcePayload"), "supplemental sourcePayload")
    target = _require_dict(phase1.get("targetPayload"), "supplemental targetPayload")
    schedule = _require_dict(phase1.get("schedule"), "supplemental schedule")
    row = {
        "selector": selector,
        "selectorSha256": canonical_json_sha256(selector),
        "family": phase1.get("family"),
        "adapterId": phase1.get("adapterId"),
        "packetLayout": phase1.get("packetLayout"),
        "fidelity": phase1.get("fidelity"),
        "disposition": phase1.get("disposition"),
        "tuningEligibleTransform": phase1.get("tuningEligibleTransform"),
        "sourceIdentity": {
            "sourceRecordId": source.get("recordId"),
            "sourceRecordSha256": source.get("recordSha256"),
            "sourcePayloadRawSha256": source.get("rawSha256"),
        },
        "targetIdentity": {
            "targetElementId": target.get("recordId"),
            "targetRecordSha256": target.get("recordSha256"),
            "targetPayloadRawSha256": target.get("rawSha256"),
        },
        "schedule": {
            "stageId": schedule.get("stageId"),
            "sourceEventId": schedule.get("sourceEventId"),
            "sourceTimelineSeconds": schedule.get("sourceTimelineSeconds"),
            "localTimeSeconds": schedule.get("localTimeSeconds"),
            "durationSeconds": schedule.get("durationSeconds"),
        },
        "resourcePacket": copy.deepcopy(phase1.get("resourcePacket", [])),
        "cascadeRibbonPacket": copy.deepcopy(phase1.get("cascadeRibbonPacket")),
        "animationTrailPacket": copy.deepcopy(phase1.get("animationTrailPacket")),
        "bakedEdgeLightPacket": copy.deepcopy(phase1.get("bakedEdgeLightPacket")),
        "admissionBlockers": copy.deepcopy(phase1.get("admissionBlockers", [])),
    }
    _seal(row, "rowSha256")
    return row


def _build_programs(
    repository_root: Path,
    corpus: dict[str, Any],
) -> list[dict[str, Any]]:
    admission = load_json(repository_root / ADMISSION_RELATIVE_PATH)
    stages = {
        stage.get("productEffectAssetId"): stage
        for stage in admission.get("stages", [])
        if isinstance(stage, dict)
    }
    grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in corpus["visualRows"]:
        grouped[row["selector"]["effectAssetId"]].append(row)
    supplemental_grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for row in corpus["supplementalElements"]:
        supplemental_grouped[row["selector"]["effectAssetId"]].append(row)
    histories_by_id = {
        history["historyId"]: history
        for history in _require_list(
            corpus.get("bakedEdgeHistories"), "corpus.bakedEdgeHistories"
        )
    }
    effect_asset_ids = sorted(set(grouped) | set(supplemental_grouped))
    local_program = None
    local_receipt = None
    if "effect.artist.skill.31470" in effect_asset_ids:
        local_program = load_json(repository_root / LOCAL_DECAL_PROGRAM_RELATIVE_PATH)
        local_receipt = load_json(repository_root / LOCAL_DECAL_RECEIPT_RELATIVE_PATH)
        _verify_seal(local_program, "programSha256", "LocalDecal runtime program")
    programs = []
    typed_codec_jobs: list[tuple[dict[str, Any], Path, Path]] = []
    projected_temporary_directory = tempfile.TemporaryDirectory(
        prefix="effect-visual-program-typed-codec-"
    )
    projected_temporary_root = Path(projected_temporary_directory.name)
    for effect_asset_id in effect_asset_ids:
        rows = grouped.get(effect_asset_id, [])
        runtime_rows = [
            _runtime_row(
                row,
                _build_local_decal_packet(repository_root, row, local_program, local_receipt)
                if effect_asset_id == "effect.artist.skill.31470" else None,
            )
            for row in rows
        ]
        runtime_rows.sort(key=lambda row: row["selector"]["occurrenceId"])
        supplemental_source = supplemental_grouped.get(effect_asset_id, [])
        runtime_supplemental = [
            _runtime_supplemental_element(row) for row in supplemental_source
        ]
        runtime_supplemental.sort(key=lambda row: row["selector"]["occurrenceId"])
        referenced_history_ids = sorted(
            {
                packet["historyId"]
                for source in supplemental_source
                for packet in [source.get("animationTrailPacket")]
                if isinstance(packet, dict) and packet.get("packetVersion") == 2
            }
            | {
                packet["historyId"]
                for source in supplemental_source
                for packet in [source.get("bakedEdgeLightPacket")]
                if isinstance(packet, dict)
            }
        )
        runtime_histories = [
            copy.deepcopy(_require_dict(
                histories_by_id.get(history_id),
                f"baked-edge history {history_id}",
            ))
            for history_id in referenced_history_ids
        ]
        if effect_asset_id == "effect.artist.skill.31470":
            _require(len(rows) == EXPECTED_LOCAL_PACKET_COUNT, "Artist F LocalDecal packet count changed")
            program = {
                "effectAssetId": effect_asset_id,
                "projectionKind": "ADAPTER_PACKET_V1",
                "baseDocumentIdentity": None,
                "projectedDocument": None,
                "projectedDocumentCanonicalByteCount": None,
                "projectedDocumentSha256": None,
                "projectedDocumentTypedCodecSha256": None,
                "visualRows": runtime_rows,
                "supplementalElements": runtime_supplemental,
                "bakedEdgeHistories": runtime_histories,
            }
        else:
            path, expected_raw = _resolve_overlay_base_document(
                repository_root,
                effect_asset_id,
                rows,
                supplemental_source,
                stages,
            )
            _validate_overlay_base_raw_sha(
                repository_root,
                path,
                expected_raw,
                effect_asset_id,
                corpus,
            )
            base = load_json(path)
            validate_standard_document(base, effect_asset_id)
            projected = project_ba_document(
                repository_root, base, rows, supplemental_source
            )
            program = {
                "effectAssetId": effect_asset_id,
                "projectionKind": "SOURCE_RECIPE_OVERLAY_V1",
                "baseDocumentIdentity": {
                    "rawSha256": expected_raw,
                    "canonicalSha256": canonical_json_sha256(base),
                    "typedCodecSha256": "0" * 64,
                },
                "projectedDocument": projected,
                "projectedDocumentCanonicalByteCount": len(canonical_json_bytes(projected)),
                "projectedDocumentSha256": canonical_json_sha256(projected),
                "projectedDocumentTypedCodecSha256": "0" * 64,
                "visualRows": runtime_rows,
                "supplementalElements": runtime_supplemental,
                "bakedEdgeHistories": runtime_histories,
            }
            projected_path = projected_temporary_root / (
                effect_asset_id + ".projected.effect.json"
            )
            projected_path.write_bytes(pretty_json_bytes(projected))
            typed_codec_jobs.append((program, path, projected_path))
        programs.append(program)
    if typed_codec_jobs:
        typed_paths = [
            path
            for _, base_path, projected_path in typed_codec_jobs
            for path in (base_path, projected_path)
        ]
        typed_hashes = _compute_typed_codec_sha256_batch(
            repository_root, typed_paths
        )
        for index, (program, _, _) in enumerate(typed_codec_jobs):
            program["baseDocumentIdentity"]["typedCodecSha256"] = (
                typed_hashes[index * 2]
            )
            program["projectedDocumentTypedCodecSha256"] = (
                typed_hashes[index * 2 + 1]
            )
    for program in programs:
        _seal(program, "programSha256")
    projected_temporary_directory.cleanup()
    _require(
        len(programs) == EXPECTED_PROGRAM_COUNT,
        "visual runtime program count changed",
    )
    return programs


def _runtime_canaries(corpus: dict[str, Any]) -> list[dict[str, Any]]:
    result = []
    for source in corpus["extensionCanaries"]:
        canary = {
            key: copy.deepcopy(source[key])
            for key in (
                "canaryId", "domain", "selector", "selectorSha256",
                "family", "adapterId", "packetLayout", "fidelity",
                "disposition", "productCountContribution", "admissionBlockers",
            )
        }
        _seal(canary, "canarySha256")
        result.append(canary)
    result.sort(key=lambda row: row["canaryId"])
    return result


def build_runtime(repository_root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    repository_root = repository_root.resolve()
    schema_path = repository_root / SCHEMA_RELATIVE_PATH
    _require(schema_path.is_file(), f"missing runtime schema: {SCHEMA_RELATIVE_PATH}")
    load_json(schema_path)
    corpus, corpus_raw_sha = _load_source_corpus(repository_root)
    runtime = {
        "schema": RUNTIME_SCHEMA,
        "formatVersion": RUNTIME_VERSION,
        "runtimeId": RUNTIME_ID,
        "contractRole": CONTRACT_ROLE,
        "sourceCorpus": {
            "corpusId": corpus["corpusId"],
            "artifactSha256": corpus["artifactSha256"],
            "rawSha256": corpus_raw_sha,
        },
        "adapterContracts": copy.deepcopy(corpus["adapterContracts"]),
        "programs": _build_programs(repository_root, corpus),
        "extensionCanaries": _runtime_canaries(corpus),
        "denominators": {
            "programCount": EXPECTED_PROGRAM_COUNT,
            "sourceRecipeOverlayProgramCount": EXPECTED_BA_PROGRAM_COUNT,
            "adapterPacketProgramCount": EXPECTED_ADAPTER_PROGRAM_COUNT,
            "visualRowCount": EXPECTED_ROW_COUNT,
            "sourceRecipeOverlayCount": EXPECTED_OVERLAY_COUNT,
            "localDecalAdapterPacketCount": EXPECTED_LOCAL_PACKET_COUNT,
            "cascadeRibbonVisualRowCount": EXPECTED_CASCADE_RIBBON_ROW_COUNT,
            "supplementalElementCount": EXPECTED_SUPPLEMENTAL_ELEMENT_COUNT,
            "artistFCascadeRibbonElementCount":
                EXPECTED_ARTIST_F_CASCADE_RIBBON_ELEMENT_COUNT,
            "artistTCascadeRibbonElementCount":
                EXPECTED_ARTIST_T_CASCADE_RIBBON_ELEMENT_COUNT,
            "animationTrailElementCount": EXPECTED_ANIMATION_TRAIL_ELEMENT_COUNT,
            "bakedEdgeLightElementCount": EXPECTED_BAKED_EDGE_LIGHT_ELEMENT_COUNT,
            "failClosedCount": EXPECTED_FAIL_CLOSED_COUNT,
            "extensionCanaryCount": 2,
            "productMutationCount": 0,
        },
        "transactionPolicy": {
            "loadOrder": ["parse", "validate", "stage", "commit"],
            "commitMode": "ATOMIC_REPLACE_AFTER_FULL_RUNTIME_VALIDATION",
            "failureAction": "PRESERVE_PREVIOUS_SIDECAR_AND_PRODUCT_RUNTIME",
            "productMutation": False,
            "catalogMutation": False,
        },
    }
    _seal(runtime, "artifactSha256")
    validate_runtime(runtime, repository_root)
    return runtime


def _phase1_by_selector(corpus: dict[str, Any]) -> dict[tuple[str, str], dict[str, Any]]:
    result = {}
    for row in corpus["visualRows"]:
        selector = row["selector"]
        key = (selector["effectAssetId"], selector["occurrenceId"])
        _require(key not in result, f"duplicate phase1 selector: {key}")
        result[key] = row
    return result


def _phase1_supplemental_by_selector(
    corpus: dict[str, Any],
) -> dict[tuple[str, str], dict[str, Any]]:
    result = {}
    for row in corpus["supplementalElements"]:
        selector = row["selector"]
        key = (selector["effectAssetId"], selector["occurrenceId"])
        _require(key not in result, f"duplicate phase1 supplemental selector: {key}")
        result[key] = row
    return result


def _expected_runtime_identity(
    repository_root: Path,
    source: dict[str, Any],
    local_program: dict[str, Any],
    local_receipt: dict[str, Any],
) -> dict[str, Any]:
    is_local = source.get("executionProjection", {}).get("adapterId") == (
        "local-decal-rt0-bounded-v1"
    )
    return _runtime_row(
        source,
        _build_local_decal_packet(repository_root, source, local_program, local_receipt)
        if is_local else None,
    )


def validate_runtime(
    runtime: dict[str, Any],
    repository_root: Path = REPOSITORY_ROOT,
) -> None:
    repository_root = repository_root.resolve()
    _expect_keys(
        runtime,
        [
            "schema", "formatVersion", "runtimeId", "contractRole",
            "sourceCorpus", "adapterContracts", "programs",
            "extensionCanaries", "denominators", "transactionPolicy",
            "artifactSha256",
        ],
        "runtime",
    )
    _require(runtime.get("schema") == RUNTIME_SCHEMA and runtime.get("formatVersion") == RUNTIME_VERSION, "runtime header mismatch")
    _require(runtime.get("runtimeId") == RUNTIME_ID and runtime.get("contractRole") == CONTRACT_ROLE, "runtime identity/role mismatch")
    _verify_seal(runtime, "artifactSha256", "runtime")
    corpus, corpus_raw = _load_source_corpus(repository_root)
    _require(runtime.get("sourceCorpus") == {
        "corpusId": corpus["corpusId"],
        "artifactSha256": corpus["artifactSha256"],
        "rawSha256": corpus_raw,
    }, "runtime source corpus identity is stale")
    adapters = runtime.get("adapterContracts")
    _require(adapters == corpus["adapterContracts"], "runtime adapter allowlist diverged from phase1")
    adapter_by_id = {row["adapterId"]: row for row in adapters}
    source_by_selector = _phase1_by_selector(corpus)
    supplemental_by_selector = _phase1_supplemental_by_selector(corpus)
    phase1_histories = {
        history["historyId"]: history
        for history in _require_list(
            corpus.get("bakedEdgeHistories"), "corpus.bakedEdgeHistories"
        )
    }
    local_program = load_json(repository_root / LOCAL_DECAL_PROGRAM_RELATIVE_PATH)
    local_receipt = load_json(repository_root / LOCAL_DECAL_RECEIPT_RELATIVE_PATH)
    _verify_seal(local_program, "programSha256", "LocalDecal runtime program")
    programs = _require_list(runtime.get("programs"), "programs")
    _require(len(programs) == EXPECTED_PROGRAM_COUNT, "runtime program denominator changed")
    _require(programs == sorted(programs, key=lambda row: row["effectAssetId"]), "runtime programs are not deterministic")
    seen_effects: set[str] = set()
    seen_selectors: set[tuple[str, str]] = set()
    seen_supplemental_selectors: set[tuple[str, str]] = set()
    counts = Counter()
    admission = load_json(repository_root / ADMISSION_RELATIVE_PATH)
    stages = {stage.get("productEffectAssetId"): stage for stage in admission.get("stages", []) if isinstance(stage, dict)}
    for program_index, program_value in enumerate(programs):
        program = _require_dict(program_value, f"programs[{program_index}]")
        _expect_keys(
            program,
            [
                "effectAssetId", "projectionKind", "baseDocumentIdentity",
                "projectedDocument", "projectedDocumentCanonicalByteCount",
                "projectedDocumentSha256", "projectedDocumentTypedCodecSha256",
                "visualRows", "supplementalElements", "bakedEdgeHistories",
                "programSha256",
            ],
            f"programs[{program_index}]",
        )
        _verify_seal(program, "programSha256", f"programs[{program_index}]")
        effect_asset_id = _require_stable_id(program.get("effectAssetId"), "program effectAssetId")
        _require(effect_asset_id not in seen_effects, f"duplicate runtime effect program: {effect_asset_id}")
        seen_effects.add(effect_asset_id)
        rows = _require_list(program.get("visualRows"), "program.visualRows")
        supplemental = _require_list(
            program.get("supplementalElements"), "program.supplementalElements"
        )
        histories = _require_list(
            program.get("bakedEdgeHistories"), "program.bakedEdgeHistories"
        )
        _require(
            rows or supplemental,
            f"runtime program has no executable rows: {effect_asset_id}",
        )
        _require(rows == sorted(rows, key=lambda row: row["selector"]["occurrenceId"]), f"program rows are not deterministic: {effect_asset_id}")
        for row_index, row_value in enumerate(rows):
            row = _require_dict(row_value, f"{effect_asset_id}.visualRows[{row_index}]")
            _expect_keys(
                row,
                [
                    "selector", "selectorSha256", "family", "adapterId",
                    "packetLayout", "fidelity", "disposition",
                    "tuningEligibleTransform", "sourceIdentity",
                    "targetIdentity", "resourcePacket", "localDecalPacket",
                    "admissionBlockers", "rowSha256",
                ],
                "runtime row",
            )
            _verify_seal(row, "rowSha256", "runtime row")
            selector = _require_dict(row.get("selector"), "runtime selector")
            _expect_keys(selector, ["effectAssetId", "occurrenceId"], "runtime selector")
            _require(selector.get("effectAssetId") == effect_asset_id, "runtime selector/program mismatch")
            key = (selector["effectAssetId"], selector["occurrenceId"])
            _require(key not in seen_selectors, f"duplicate runtime selector: {key}")
            seen_selectors.add(key)
            _require(canonical_json_sha256(selector) == row.get("selectorSha256"), "runtime selector hash is stale")
            source_row = source_by_selector.get(key)
            _require(source_row is not None, f"runtime row is absent from phase1 corpus: {key}")
            expected = _expected_runtime_identity(
                repository_root, source_row, local_program, local_receipt
            )
            _require(row == expected, f"runtime row identity diverged from phase1: {key}")
            adapter = adapter_by_id.get(row.get("adapterId"))
            _require(adapter is not None, f"unknown runtime adapter: {row.get('adapterId')}")
            _require(adapter.get("family") == row.get("family"), f"runtime adapter/family mismatch: {key}")
            _require(row.get("packetLayout") in adapter.get("packetLayouts", []), f"runtime adapter packet mismatch: {key}")
            counts[("disposition", row.get("disposition"))] += 1
            counts[("family", row.get("family"))] += 1
            if row.get("disposition") == "FAIL_CLOSED":
                _require(
                    row.get("targetIdentity") is None
                    and row.get("localDecalPacket") is None
                    and row.get("packetLayout") == "NONE",
                    f"fail-closed row can mutate: {key}",
                )
            elif row.get("adapterId") == "local-decal-rt0-bounded-v1":
                counts[("projection", "local")] += 1
                _require(row.get("packetLayout") == "LOCAL_DECAL_RT0_SIX_SRV_V1", "LocalDecal packet layout changed")
                _require([item.get("shaderRegister") for item in row.get("resourcePacket", [])] == [f"t{i}" for i in range(6)], "LocalDecal t0..t5 resource packet changed")
                packet = _require_dict(row.get("localDecalPacket"), "LocalDecal packet")
                _verify_seal(packet, "packetSha256", "LocalDecal packet")
                _require(
                    len(packet.get("inputDispositions", [])) == 33
                    and len(packet.get("staticDispositions", [])) == 18
                    and len(packet.get("renderDispositions", [])) == 6
                    and len(packet.get("packedScalars", [])) == 22
                    and len(packet.get("packedVectors", [])) == 3
                    and len(packet.get("srvs", [])) == 6
                    and packet.get("nativeExecution") is False
                    and packet.get("nativeVertexFactoryAdmitted") is False
                    and packet.get("nativeMrtAdmitted") is False
                    and packet.get("boundedSemanticReplay") is True,
                    "LocalDecal executable packet denominator/admission changed",
                )
            else:
                counts[("projection", "overlay")] += 1
                _require(
                    row.get("targetIdentity") is not None
                    and row.get("localDecalPacket") is None
                    and row.get("packetLayout") == "EFFECT_DOCUMENT_ELEMENT_V12",
                    f"BA overlay target/packet missing: {key}",
                )
        _require(
            supplemental == sorted(
                supplemental, key=lambda row: row["selector"]["occurrenceId"]
            ),
            f"supplemental elements are not deterministic: {effect_asset_id}",
        )
        for item in supplemental:
            _verify_seal(item, "rowSha256", "runtime supplemental element")
            selector = _require_dict(item.get("selector"), "supplemental selector")
            _expect_keys(selector, ["effectAssetId", "occurrenceId"], "supplemental selector")
            _require(
                selector.get("effectAssetId") == effect_asset_id and
                canonical_json_sha256(selector) == item.get("selectorSha256") and
                item.get("disposition") == "ADMITTED_BOUNDED" and
                item.get("admissionBlockers") == [],
                "runtime supplemental selector/admission is invalid",
            )
            supplemental_key = (
                selector["effectAssetId"], selector["occurrenceId"]
            )
            _require(
                supplemental_key not in seen_supplemental_selectors,
                f"duplicate runtime supplemental selector: {supplemental_key}",
            )
            seen_supplemental_selectors.add(supplemental_key)
            source_supplemental = supplemental_by_selector.get(supplemental_key)
            _require(
                source_supplemental is not None
                and item == _runtime_supplemental_element(source_supplemental),
                f"runtime supplemental identity diverged from phase1: {supplemental_key}",
            )
            if item.get("family") == "ANIMATION_TRAIL":
                _require(
                    item.get("adapterId") == "animation-trail-document-v12" and
                    item.get("packetLayout") in {
                        "ANIMATION_TRAIL_ELEMENT_V1",
                        "ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1",
                    } and
                    item.get("cascadeRibbonPacket") is None and
                    item.get("bakedEdgeLightPacket") is None and
                    isinstance(item.get("animationTrailPacket"), dict),
                    "AnimationTrail runtime packet is invalid",
                )
            elif item.get("family") == "CASCADE_RIBBON":
                _require(
                    item.get("adapterId") == "cascade-ribbon-document-v12" and
                    item.get("packetLayout") == "CASCADE_RIBBON_TYPED_PACKET_V1" and
                    isinstance(item.get("cascadeRibbonPacket"), dict) and
                    item.get("animationTrailPacket") is None and
                    item.get("bakedEdgeLightPacket") is None,
                    "CascadeRibbon supplemental packet is invalid",
                )
            else:
                packet = item.get("bakedEdgeLightPacket")
                _require(
                    item.get("family") == "LIGHT_PARTICLE" and
                    item.get("adapterId") == "light-particle-document-v12" and
                    item.get("packetLayout") == "LIGHT_BAKED_EDGE_ATTACHMENT_V1" and
                    item.get("cascadeRibbonPacket") is None and
                    item.get("animationTrailPacket") is None and
                    isinstance(packet, dict) and
                    packet.get("runtimeCarrier")
                        == "EFFECT_TYPED_LIGHT_BAKED_EDGE_ATTACHMENT_V1" and
                    packet.get("lane") == "FIRST_EDGE",
                    "baked-edge Light supplemental packet is invalid",
                )
            counts[("supplemental", item.get("family"))] += 1
            if item.get("family") == "CASCADE_RIBBON":
                if effect_asset_id == "effect.artist.skill.31470":
                    counts[("supplemental", "ARTIST_F_CASCADE_RIBBON")] += 1
                elif effect_asset_id == "effect.artist.skill.31950.unified":
                    counts[("supplemental", "ARTIST_T_CASCADE_RIBBON")] += 1
                else:
                    raise ContractError(
                        "unowned CascadeRibbon supplemental program: "
                        f"{effect_asset_id}"
                    )
        _require(
            histories == sorted(histories, key=lambda row: row["historyId"]),
            f"runtime baked-edge histories are not deterministic: {effect_asset_id}",
        )
        referenced_history_ids = {
            item["animationTrailPacket"]["historyId"]
            for item in supplemental
            if item.get("packetLayout")
                == "ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1"
        } | {
            item["bakedEdgeLightPacket"]["historyId"]
            for item in supplemental
            if item.get("packetLayout") == "LIGHT_BAKED_EDGE_ATTACHMENT_V1"
        }
        history_ids = set()
        for history in histories:
            history_id = _require_string(
                history.get("historyId"), "runtime baked-edge historyId"
            )
            _require(
                history_id not in history_ids
                and phase1_histories.get(history_id) == history,
                f"runtime baked-edge history diverged from phase1: {history_id}",
            )
            history_ids.add(history_id)
        _require(
            history_ids == referenced_history_ids,
            f"runtime baked-edge history reference closure changed: {effect_asset_id}",
        )
        if program.get("projectionKind") == "SOURCE_RECIPE_OVERLAY_V1":
            counts[("program", "overlay")] += 1
            phase1_rows = [
                source_by_selector[
                    (effect_asset_id, row["selector"]["occurrenceId"])
                ]
                for row in rows
            ]
            phase1_supplemental = [
                item
                for item in corpus["supplementalElements"]
                if item["selector"]["effectAssetId"] == effect_asset_id
            ]
            base_path, expected_raw = _resolve_overlay_base_document(
                repository_root,
                effect_asset_id,
                phase1_rows,
                phase1_supplemental,
                stages,
            )
            _validate_overlay_base_raw_sha(
                repository_root,
                base_path,
                expected_raw,
                effect_asset_id,
                corpus,
            )
            base = load_json(base_path)
            expected_base_identity = {
                "rawSha256": expected_raw,
                "canonicalSha256": canonical_json_sha256(base),
                "typedCodecSha256": program.get(
                    "baseDocumentIdentity", {}
                ).get("typedCodecSha256"),
            }
            _require_sha(
                expected_base_identity["typedCodecSha256"],
                "baseDocumentIdentity.typedCodecSha256",
            )
            _require(program.get("baseDocumentIdentity") == expected_base_identity, f"BA base document identity changed: {effect_asset_id}")
            projected = _require_dict(program.get("projectedDocument"), "projectedDocument")
            _require(
                canonical_json_sha256(projected) == program.get("projectedDocumentSha256")
                and SHA256_RE.fullmatch(
                    program.get("projectedDocumentTypedCodecSha256", "")
                ) is not None
                and len(canonical_json_bytes(projected)) ==
                    program.get("projectedDocumentCanonicalByteCount"),
                f"projected document bytes/SHA are stale: {effect_asset_id}",
            )
            expected_projected = project_ba_document(
                repository_root, base, phase1_rows, phase1_supplemental
            )
            _require(projected == expected_projected, f"SourceRecipe projection is not deterministic: {effect_asset_id}")
            for row in rows:
                if row.get("disposition") != "ADMITTED_BOUNDED":
                    continue
                target = _require_dict(row.get("targetIdentity"), "BA target identity")
                element = _find_element(
                    projected, target["targetElementId"], effect_asset_id
                )
                recipe = _require_dict(element.get("sourceRecipe"), "projected SourceRecipe")
                identity = _require_dict(row.get("sourceIdentity"), "BA source identity")
                _require(
                    len(_require_list(recipe.get("modules"), "projected modules")) ==
                        identity.get("moduleCount")
                    and canonical_json_sha256(recipe) ==
                        identity.get("sourceRecipeSha256")
                    and canonical_json_sha256(recipe["modules"]) ==
                        identity.get("moduleClosureSha256"),
                    f"projected emitter moduleCount/recipe closure changed: {key}",
                )
        else:
            counts[("program", "adapter")] += 1
            _require(
                program.get("projectionKind") == "ADAPTER_PACKET_V1"
                and effect_asset_id == "effect.artist.skill.31470"
                and program.get("baseDocumentIdentity") is None
                and program.get("projectedDocument") is None
                and program.get("projectedDocumentCanonicalByteCount") is None
                and program.get("projectedDocumentSha256") is None
                and program.get("projectedDocumentTypedCodecSha256") is None,
                "adapter-only runtime program boundary changed",
            )
    _require(len(seen_selectors) == EXPECTED_ROW_COUNT == len(source_by_selector), "runtime selector closure changed")
    _require(
        len(seen_supplemental_selectors)
            == EXPECTED_SUPPLEMENTAL_ELEMENT_COUNT
            == len(supplemental_by_selector),
        "runtime supplemental selector closure changed",
    )
    _require(counts[("program", "overlay")] == EXPECTED_BA_PROGRAM_COUNT, "BA overlay program count changed")
    _require(counts[("program", "adapter")] == EXPECTED_ADAPTER_PROGRAM_COUNT, "adapter program count changed")
    _require(counts[("projection", "overlay")] == EXPECTED_OVERLAY_COUNT, "SourceRecipe overlay count changed")
    _require(counts[("projection", "local")] == EXPECTED_LOCAL_PACKET_COUNT, "LocalDecal packet count changed")
    _require(counts[("disposition", "FAIL_CLOSED")] == EXPECTED_FAIL_CLOSED_COUNT, "fail-closed count changed")
    _require(counts[("family", "CASCADE_RIBBON")] == EXPECTED_CASCADE_RIBBON_ROW_COUNT, "CascadeRibbon row count changed")
    _require(counts[("supplemental", "CASCADE_RIBBON")] == EXPECTED_ARTIST_CASCADE_RIBBON_ELEMENT_COUNT, "Artist CascadeRibbon supplemental count changed")
    _require(counts[("supplemental", "ARTIST_F_CASCADE_RIBBON")] == EXPECTED_ARTIST_F_CASCADE_RIBBON_ELEMENT_COUNT, "Artist F CascadeRibbon supplemental count changed")
    _require(counts[("supplemental", "ARTIST_T_CASCADE_RIBBON")] == EXPECTED_ARTIST_T_CASCADE_RIBBON_ELEMENT_COUNT, "Artist T CascadeRibbon supplemental count changed")
    _require(counts[("supplemental", "ANIMATION_TRAIL")] == EXPECTED_ANIMATION_TRAIL_ELEMENT_COUNT, "AnimationTrail supplemental count changed")
    _require(counts[("supplemental", "LIGHT_PARTICLE")] == EXPECTED_BAKED_EDGE_LIGHT_ELEMENT_COUNT, "baked-edge Light supplemental count changed")
    canaries = _require_list(runtime.get("extensionCanaries"), "extensionCanaries")
    _require(len(canaries) == 2, "extension canary count changed")
    for canary in canaries:
        _verify_seal(canary, "canarySha256", "extension canary")
        _expect_keys(_require_dict(canary.get("selector"), "canary selector"), ["effectAssetId", "occurrenceId"], "canary selector")
        _require(canary.get("disposition") == "FAIL_CLOSED" and canary.get("productCountContribution") is False, "extension canary was falsely admitted")
    expected_denominators = {
        "programCount": EXPECTED_PROGRAM_COUNT,
        "sourceRecipeOverlayProgramCount": EXPECTED_BA_PROGRAM_COUNT,
        "adapterPacketProgramCount": EXPECTED_ADAPTER_PROGRAM_COUNT,
        "visualRowCount": EXPECTED_ROW_COUNT,
        "sourceRecipeOverlayCount": EXPECTED_OVERLAY_COUNT,
        "localDecalAdapterPacketCount": EXPECTED_LOCAL_PACKET_COUNT,
        "cascadeRibbonVisualRowCount": EXPECTED_CASCADE_RIBBON_ROW_COUNT,
        "supplementalElementCount": EXPECTED_SUPPLEMENTAL_ELEMENT_COUNT,
        "artistFCascadeRibbonElementCount":
            EXPECTED_ARTIST_F_CASCADE_RIBBON_ELEMENT_COUNT,
        "artistTCascadeRibbonElementCount":
            EXPECTED_ARTIST_T_CASCADE_RIBBON_ELEMENT_COUNT,
        "animationTrailElementCount": EXPECTED_ANIMATION_TRAIL_ELEMENT_COUNT,
        "bakedEdgeLightElementCount": EXPECTED_BAKED_EDGE_LIGHT_ELEMENT_COUNT,
        "failClosedCount": EXPECTED_FAIL_CLOSED_COUNT,
        "extensionCanaryCount": 2,
        "productMutationCount": 0,
    }
    _require(runtime.get("denominators") == expected_denominators, "runtime denominators are stale")
    _require(runtime.get("transactionPolicy") == {
        "loadOrder": ["parse", "validate", "stage", "commit"],
        "commitMode": "ATOMIC_REPLACE_AFTER_FULL_RUNTIME_VALIDATION",
        "failureAction": "PRESERVE_PREVIOUS_SIDECAR_AND_PRODUCT_RUNTIME",
        "productMutation": False,
        "catalogMutation": False,
    }, "runtime transaction boundary changed")


def validate_published_artifact(runtime: dict[str, Any]) -> None:
    """Validate the closed, self-sealed runtime artifact without rebuilding it.

    The full ``validate_runtime`` gate remains the source-to-runtime authority and
    is used by the builder/check flow.  The publisher uses this bounded gate on
    both its source and staged copy so a large catalog transaction does not run
    the typed document codec a second time.  The root seal covers every nested
    packet; the checks below also reject a maliciously resealed denominator or
    selector shape.
    """

    _expect_keys(
        runtime,
        [
            "schema", "formatVersion", "runtimeId", "contractRole",
            "sourceCorpus", "adapterContracts", "programs",
            "extensionCanaries", "denominators", "transactionPolicy",
            "artifactSha256",
        ],
        "runtime",
    )
    _require(
        runtime.get("schema") == RUNTIME_SCHEMA
        and runtime.get("formatVersion") == RUNTIME_VERSION
        and runtime.get("runtimeId") == RUNTIME_ID
        and runtime.get("contractRole") == CONTRACT_ROLE,
        "runtime publish artifact header mismatch",
    )
    _verify_seal(runtime, "artifactSha256", "runtime publish artifact")

    programs = _require_list(runtime.get("programs"), "programs")
    _require(
        len(programs) == EXPECTED_PROGRAM_COUNT
        and programs == sorted(programs, key=lambda row: row["effectAssetId"]),
        "runtime publish program denominator/order changed",
    )
    seen_effects: set[str] = set()
    seen_selectors: set[tuple[str, str]] = set()
    counts = Counter()
    for program_index, program_value in enumerate(programs):
        program = _require_dict(
            program_value, f"programs[{program_index}]"
        )
        _expect_keys(
            program,
            [
                "effectAssetId", "projectionKind", "baseDocumentIdentity",
                "projectedDocument", "projectedDocumentCanonicalByteCount",
                "projectedDocumentSha256",
                "projectedDocumentTypedCodecSha256", "visualRows",
                "supplementalElements", "bakedEdgeHistories", "programSha256",
            ],
            f"programs[{program_index}]",
        )
        _verify_seal(
            program, "programSha256", f"programs[{program_index}]"
        )
        effect_asset_id = _require_stable_id(
            program.get("effectAssetId"), "program effectAssetId"
        )
        _require(
            effect_asset_id not in seen_effects,
            f"duplicate runtime publish program: {effect_asset_id}",
        )
        seen_effects.add(effect_asset_id)
        projection_kind = program.get("projectionKind")
        _require(
            projection_kind in {
                "SOURCE_RECIPE_OVERLAY_V1", "ADAPTER_PACKET_V1"
            },
            f"unsupported runtime publish projection: {effect_asset_id}",
        )
        counts[("program", projection_kind)] += 1

        rows = _require_list(program.get("visualRows"), "program.visualRows")
        _require(
            rows == sorted(
                rows, key=lambda row: row["selector"]["occurrenceId"]
            ),
            f"runtime publish rows are not deterministic: {effect_asset_id}",
        )
        for row_index, row_value in enumerate(rows):
            row = _require_dict(
                row_value, f"{effect_asset_id}.visualRows[{row_index}]"
            )
            _verify_seal(row, "rowSha256", "runtime publish row")
            selector = _require_dict(
                row.get("selector"), "runtime publish selector"
            )
            _expect_keys(
                selector,
                ["effectAssetId", "occurrenceId"],
                "runtime publish selector",
            )
            _require(
                selector.get("effectAssetId") == effect_asset_id
                and canonical_json_sha256(selector)
                == row.get("selectorSha256"),
                "runtime publish selector identity changed",
            )
            key = (effect_asset_id, selector.get("occurrenceId"))
            _require(
                key not in seen_selectors,
                f"duplicate runtime publish selector: {key}",
            )
            seen_selectors.add(key)
            counts[("row", row.get("family"))] += 1
            counts[("disposition", row.get("disposition"))] += 1
            if row.get("localDecalPacket") is not None:
                _verify_seal(
                    _require_dict(
                        row.get("localDecalPacket"),
                        "runtime publish LocalDecal packet",
                    ),
                    "packetSha256",
                    "runtime publish LocalDecal packet",
                )
                counts[("packet", "LOCAL_DECAL")] += 1

        supplemental = _require_list(
            program.get("supplementalElements"),
            "program.supplementalElements",
        )
        histories = _require_list(
            program.get("bakedEdgeHistories"),
            "program.bakedEdgeHistories",
        )
        _require(
            rows or supplemental,
            f"runtime publish program has no executable rows: {effect_asset_id}",
        )
        _require(
            supplemental == sorted(
                supplemental,
                key=lambda row: row["selector"]["occurrenceId"],
            ),
            f"runtime publish supplemental rows are not deterministic: "
            f"{effect_asset_id}",
        )
        for supplemental_value in supplemental:
            item = _require_dict(
                supplemental_value, "runtime publish supplemental element"
            )
            _verify_seal(
                item, "rowSha256", "runtime publish supplemental element"
            )
            selector = _require_dict(
                item.get("selector"), "runtime publish supplemental selector"
            )
            _expect_keys(
                selector,
                ["effectAssetId", "occurrenceId"],
                "runtime publish supplemental selector",
            )
            _require(
                selector.get("effectAssetId") == effect_asset_id
                and canonical_json_sha256(selector)
                == item.get("selectorSha256"),
                "runtime publish supplemental selector identity changed",
            )
            key = (effect_asset_id, selector.get("occurrenceId"))
            _require(
                key not in seen_selectors,
                f"duplicate runtime publish supplemental selector: {key}",
            )
            seen_selectors.add(key)
            counts[("supplemental", item.get("family"))] += 1
            if item.get("family") == "CASCADE_RIBBON":
                _verify_seal(
                    _require_dict(
                        item.get("cascadeRibbonPacket"),
                        "runtime publish CascadeRibbon packet",
                    ),
                    "packetSha256",
                    "runtime publish CascadeRibbon packet",
                )
                if effect_asset_id == "effect.artist.skill.31470":
                    counts[("supplemental", "ARTIST_F_CASCADE_RIBBON")] += 1
                elif effect_asset_id == "effect.artist.skill.31950.unified":
                    counts[("supplemental", "ARTIST_T_CASCADE_RIBBON")] += 1
                else:
                    raise ContractError(
                        "runtime publish has unowned CascadeRibbon supplemental "
                        f"program: {effect_asset_id}"
                    )
            packet = item.get("animationTrailPacket")
            if item.get("packetLayout") == "ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1":
                _verify_seal(
                    _require_dict(packet, "runtime publish baked-edge packet"),
                    "packetSha256",
                    "runtime publish baked-edge packet",
                )
            elif item.get("packetLayout") == "LIGHT_BAKED_EDGE_ATTACHMENT_V1":
                _verify_seal(
                    _require_dict(
                        item.get("bakedEdgeLightPacket"),
                        "runtime publish baked-edge Light packet",
                    ),
                    "packetSha256",
                    "runtime publish baked-edge Light packet",
                )

        referenced_history_ids = {
            item["animationTrailPacket"]["historyId"]
            for item in supplemental
            if item.get("packetLayout")
                == "ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1"
        } | {
            item["bakedEdgeLightPacket"]["historyId"]
            for item in supplemental
            if item.get("packetLayout") == "LIGHT_BAKED_EDGE_ATTACHMENT_V1"
        }
        history_ids = set()
        for history_value in histories:
            history = _require_dict(
                history_value, "runtime publish baked-edge history"
            )
            _verify_seal(
                history,
                "historySha256",
                "runtime publish baked-edge history",
            )
            history_id = _require_stable_id(
                history.get("historyId"), "runtime publish historyId"
            )
            samples = _require_list(
                history.get("samples"), "runtime publish baked-edge samples"
            )
            _require(
                history_id not in history_ids
                and history.get("sourceKind")
                    == "UE3_ANIMTRAIL_BAKED_EDGE_HISTORY_V1"
                and history.get("coordinateBasis")
                    == "UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS"
                and history.get("sampleCount") == len(samples) == 409
                and canonical_json_sha256(samples)
                    == history.get("samplesSha256"),
                "runtime publish baked-edge history closure changed",
            )
            history_ids.add(history_id)
        _require(
            history_ids == referenced_history_ids,
            f"runtime publish baked-edge history reference changed: {effect_asset_id}",
        )

    canaries = _require_list(
        runtime.get("extensionCanaries"), "extensionCanaries"
    )
    for canary in canaries:
        _verify_seal(
            _require_dict(canary, "runtime publish extension canary"),
            "canarySha256",
            "runtime publish extension canary",
        )

    expected_denominators = {
        "programCount": EXPECTED_PROGRAM_COUNT,
        "sourceRecipeOverlayProgramCount": EXPECTED_BA_PROGRAM_COUNT,
        "adapterPacketProgramCount": EXPECTED_ADAPTER_PROGRAM_COUNT,
        "visualRowCount": EXPECTED_ROW_COUNT,
        "sourceRecipeOverlayCount": EXPECTED_OVERLAY_COUNT,
        "localDecalAdapterPacketCount": EXPECTED_LOCAL_PACKET_COUNT,
        "cascadeRibbonVisualRowCount": EXPECTED_CASCADE_RIBBON_ROW_COUNT,
        "supplementalElementCount": EXPECTED_SUPPLEMENTAL_ELEMENT_COUNT,
        "artistFCascadeRibbonElementCount":
            EXPECTED_ARTIST_F_CASCADE_RIBBON_ELEMENT_COUNT,
        "artistTCascadeRibbonElementCount":
            EXPECTED_ARTIST_T_CASCADE_RIBBON_ELEMENT_COUNT,
        "animationTrailElementCount": EXPECTED_ANIMATION_TRAIL_ELEMENT_COUNT,
        "bakedEdgeLightElementCount": EXPECTED_BAKED_EDGE_LIGHT_ELEMENT_COUNT,
        "failClosedCount": EXPECTED_FAIL_CLOSED_COUNT,
        "extensionCanaryCount": 2,
        "productMutationCount": 0,
    }
    _require(
        runtime.get("denominators") == expected_denominators,
        "runtime publish denominators are stale",
    )
    _require(
        len(seen_effects) == EXPECTED_PROGRAM_COUNT
        and counts[("program", "SOURCE_RECIPE_OVERLAY_V1")]
        == EXPECTED_BA_PROGRAM_COUNT
        and counts[("program", "ADAPTER_PACKET_V1")]
        == EXPECTED_ADAPTER_PROGRAM_COUNT
        and sum(
            count for (kind, _), count in counts.items() if kind == "row"
        ) == EXPECTED_ROW_COUNT
        and counts[("row", "CASCADE_RIBBON")]
        == EXPECTED_CASCADE_RIBBON_ROW_COUNT
        and counts[("packet", "LOCAL_DECAL")]
        == EXPECTED_LOCAL_PACKET_COUNT
        and counts[("disposition", "FAIL_CLOSED")]
        == EXPECTED_FAIL_CLOSED_COUNT
        and counts[("supplemental", "CASCADE_RIBBON")]
        == EXPECTED_ARTIST_CASCADE_RIBBON_ELEMENT_COUNT
        and counts[("supplemental", "ARTIST_F_CASCADE_RIBBON")]
        == EXPECTED_ARTIST_F_CASCADE_RIBBON_ELEMENT_COUNT
        and counts[("supplemental", "ARTIST_T_CASCADE_RIBBON")]
        == EXPECTED_ARTIST_T_CASCADE_RIBBON_ELEMENT_COUNT
        and counts[("supplemental", "ANIMATION_TRAIL")]
        == EXPECTED_ANIMATION_TRAIL_ELEMENT_COUNT
        and counts[("supplemental", "LIGHT_PARTICLE")]
        == EXPECTED_BAKED_EDGE_LIGHT_ELEMENT_COUNT
        and len(canaries) == 2,
        "runtime publish internal denominators changed",
    )
    _require(runtime.get("transactionPolicy") == {
        "loadOrder": ["parse", "validate", "stage", "commit"],
        "commitMode": "ATOMIC_REPLACE_AFTER_FULL_RUNTIME_VALIDATION",
        "failureAction": "PRESERVE_PREVIOUS_SIDECAR_AND_PRODUCT_RUNTIME",
        "productMutation": False,
        "catalogMutation": False,
    }, "runtime publish transaction boundary changed")


def write_runtime_transactionally(
    runtime: dict[str, Any],
    output_path: Path,
    repository_root: Path = REPOSITORY_ROOT,
) -> None:
    validate_runtime(runtime, repository_root)
    output_path = output_path.resolve()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    payload = pretty_json_bytes(runtime)
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="wb",
            prefix=f".{output_path.name}.",
            suffix=".tmp",
            dir=output_path.parent,
            delete=False,
        ) as stream:
            temporary_path = Path(stream.name)
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary_path, output_path)
        temporary_path = None
    finally:
        if temporary_path is not None and temporary_path.exists():
            temporary_path.unlink()


def build_and_write(
    repository_root: Path,
    output_path: Path,
    *,
    check: bool = False,
) -> dict[str, Any]:
    runtime = build_runtime(repository_root)
    expected = pretty_json_bytes(runtime)
    if check:
        _require(output_path.is_file(), f"missing generated visual runtime sidecar: {output_path}")
        _require(output_path.read_bytes() == expected, f"generated visual runtime sidecar is stale: {output_path}")
    else:
        write_runtime_transactionally(runtime, output_path, repository_root)
    return runtime


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    parser.add_argument("--output", type=Path, default=Path(DEFAULT_OUTPUT_RELATIVE_PATH))
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--artifact-check", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = _parse_args()
    repository_root = args.repository_root.resolve()
    output_path = args.output
    if not output_path.is_absolute():
        output_path = repository_root / output_path
    try:
        if args.artifact_check:
            runtime = load_json(output_path)
            validate_published_artifact(runtime)
        else:
            runtime = build_and_write(
                repository_root, output_path, check=args.check
            )
    except (ContractError, phase1.ContractError) as error:
        print(f"FAIL: {error}")
        return 1
    print(
        f"PASS: "
        f"{'ARTIFACT-CHECK' if args.artifact_check else ('CHECK' if args.check else 'WRITE')} "
        f"{output_path} "
        f"programs={len(runtime['programs'])} rows={sum(len(p['visualRows']) for p in runtime['programs'])} "
        f"sha256={runtime['artifactSha256']} productMutation=false"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
