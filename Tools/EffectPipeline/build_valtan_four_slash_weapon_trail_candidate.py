#!/usr/bin/env python3
"""Build the isolated FourSlash weapon-bone Trail overlay candidate.

This tool deliberately has no canonical apply mode.  It snapshots the existing
safe-gap CascadeRibbon closure, projects one PROJECT_TUNED/BOUNDED ordinary
Trail that follows Valtan's official right-weapon bone, and writes only the
candidate document plus its manifest.  Cue, binding, catalog, canonical Effect,
VisualProgram, and renderer files are outside this tool's write set.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import importlib.util
import json
import os
import sys
import tempfile
from pathlib import Path, PurePosixPath
from typing import Any

SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent


def _load_sibling_module(name: str, path: Path):
    specification = importlib.util.spec_from_file_location(name, path)
    if specification is None or specification.loader is None:
        raise RuntimeError(f"could not load {path}")
    module = importlib.util.module_from_spec(specification)
    sys.modules[name] = module
    specification.loader.exec_module(module)
    return module


wmodel_evidence = _load_sibling_module(
    "build_valtan_whirlwind_effect_canary_for_four_slash_weapon_trail",
    SCRIPT_PATH.parent / "build_valtan_whirlwind_effect_canary.py",
)

SOURCE_EFFECT_ASSET_ID = "effect.valtan.four-slash.active.clip-02"
SOURCE_DOCUMENT_RELATIVE_PATH = PurePosixPath(
    "Data/Effects/Authored/effect.valtan.four-slash.active.clip-02.effect.json"
)
SOURCE_ELEMENT_ID = "safe-gap.trail.6dc8db3e4616f2794149d838"
SOURCE_ELEMENT_SHA256 = (
    "f55096b20690a583cac158570115315292555df23b9c67ecb13e77c4b2441ab7"
)
SOURCE_RESOURCE_CLOSURE_SHA256 = (
    "72829daad784e8883e96e3619a6ea90d3551211b93dc3f9cf7841a76f9a929c5"
)
SOURCE_MATERIAL_CLOSURE_SHA256 = (
    "d70e07b03d9ebedbb3534f3e388503afed62b6f390f850ed41aefe624e8a294d"
)
SOURCE_TRAIL_GEOMETRY_SHA256 = (
    "3aa181006b1a673de4c4f67a699bdb0614340576509609e397fce1eba121a861"
)
SOURCE_OCCURRENCE_TIME_SECONDS = 1.6400439739227295

CANDIDATE_ELEMENT_ID = "project-four-slash-weapon-bone-trail"
CANDIDATE_SOURCE_NODE = (
    "project-authored:valtan.four-slash.active.clip-02.weapon-bone-trail"
)
RUNTIME_ANCHOR_SLOT_ID = "VALTAN_FOUR_SLASH_WEAPON_R"
RUNTIME_BONE_NAME = "b_wp_r_01"

BODY_MODEL_ASSET_ID = "Character/Valtan/MN_RPBF_01.wmodel"
BODY_MODEL_SHA256 = (
    "227191781b035b9aa41d60cc8c49bf1e8b2f67a749111e4afe8f6f3c997b2011"
)
EXPECTED_BONE_INDEX = 54
EXPECTED_BONE_PARENT_INDEX = 41
EXPECTED_BONE_NAME_HASH = "51bf337724f3b4ca"

EXPECTED_RESOURCE_EVIDENCE = {
    "Effect/Valtan/Textures/FX_TEX_05/fx_k_auraline_02.dds": (
        65_664,
        "25c9e16a7ee3341ee2d98a49a5ed9ffb3fae8976db474f500f1dd403cf336f82",
    ),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_002.dds": (
        8_320,
        "6125c3c1bcea0455d3f3c9bf0c8092331cd789f9e6686d00eac45f136fe79393",
    ),
}

OUTPUT_DIRECTORY_RELATIVE_PATH = PurePosixPath(
    "Data/Effects/Imported/Valtan/ProjectTunedFourSlashWeaponTrail"
)
SCHEMA_RELATIVE_PATH = PurePosixPath(
    "Tools/EffectPipeline/Schemas/"
    "lostark.valtan-four-slash-weapon-trail-candidate.schema.json"
)
CANDIDATE_FILENAME = (
    "effect.valtan.four-slash.active.clip-02.weapon-bone-trail."
    "candidate.effect.json"
)
MANIFEST_FILENAME = "Valtan.four-slash-weapon-trail.candidate-manifest.v1.json"

FORBIDDEN_WRITE_RELATIVE_PATHS = frozenset(
    {
        PurePosixPath(
            "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
        ),
        PurePosixPath(
            "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
        ),
        SOURCE_DOCUMENT_RELATIVE_PATH,
        PurePosixPath("Data/Effects/EffectCatalog.json"),
    }
)


class CandidateError(RuntimeError):
    """The bounded candidate cannot be derived without inventing evidence."""


def _canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(_canonical_json_bytes(value)).hexdigest()


def raw_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, sort_keys=False) + "\n"
    ).encode("utf-8")


def _read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise CandidateError(f"could not read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise CandidateError(f"JSON root must be an object: {path}")
    return value


def _repository_path(root: Path, relative: PurePosixPath) -> Path:
    candidate = root.joinpath(*relative.parts).resolve()
    resolved_root = root.resolve()
    if candidate != resolved_root and resolved_root not in candidate.parents:
        raise CandidateError(f"repository-relative path escaped root: {relative}")
    return candidate


def _disabled_source_recipe() -> dict[str, Any]:
    return {
        "enabled": False,
        "rendererShape": "",
        "emitterDelaySeconds": 0,
        "emitterDurationSeconds": 0,
        "emitterLoopCount": 1,
        "bursts": [],
        "modules": [],
    }


def _source_element(document: dict[str, Any]) -> dict[str, Any]:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != SOURCE_EFFECT_ASSET_ID
    ):
        raise CandidateError("FourSlash source document identity changed")
    matches = [
        row
        for row in document.get("elements", [])
        if isinstance(row, dict) and row.get("id") == SOURCE_ELEMENT_ID
    ]
    if len(matches) != 1:
        raise CandidateError("FourSlash safe-gap source row is missing or duplicated")
    source = matches[0]
    attachment = source.get("actionCueAttachment") or {}
    recipe = source.get("sourceRecipe") or {}
    presentation = source.get("sourcePresentation") or {}
    if (
        canonical_sha256(source) != SOURCE_ELEMENT_SHA256
        or source.get("kind") != "trail"
        or source.get("visible") is not True
        or attachment.get("enabled") is not False
        or attachment.get("follow") is not False
        or recipe.get("enabled") is not True
        or recipe.get("rendererShape") != "ribbon"
        or presentation.get("sourceTimeSeconds")
        != SOURCE_OCCURRENCE_TIME_SECONDS
        or canonical_sha256(source.get("resources"))
        != SOURCE_RESOURCE_CLOSURE_SHA256
        or canonical_sha256(source.get("material"))
        != SOURCE_MATERIAL_CLOSURE_SHA256
        or canonical_sha256((source.get("detail") or {}).get("trail"))
        != SOURCE_TRAIL_GEOMETRY_SHA256
    ):
        raise CandidateError(
            "FourSlash safe-gap source row changed; bounded projection review is required"
        )
    return source


def _bone_evidence(root: Path) -> dict[str, Any]:
    body_model = _repository_path(
        root, PurePosixPath("Client/Bin/Resources") / BODY_MODEL_ASSET_ID
    )
    if not body_model.is_file() or raw_sha256(body_model) != BODY_MODEL_SHA256:
        raise CandidateError("Valtan body WModel identity changed")
    try:
        bones = wmodel_evidence.read_wmodel_bones(body_model)
        evidence = wmodel_evidence.derive_bone_evidence(
            bones,
            RUNTIME_BONE_NAME,
            BODY_MODEL_ASSET_ID,
            BODY_MODEL_SHA256,
        )
    except wmodel_evidence.ContractError as error:
        raise CandidateError(str(error)) from error
    matches = [bone for bone in bones if bone.name == RUNTIME_BONE_NAME]
    if len(matches) != 1:
        raise CandidateError("Valtan right-weapon bone is not unique")
    bone = matches[0]
    if (
        bone.index != EXPECTED_BONE_INDEX
        or bone.parent_index != EXPECTED_BONE_PARENT_INDEX
        or f"{bone.name_hash:016x}" != EXPECTED_BONE_NAME_HASH
    ):
        raise CandidateError("Valtan right-weapon bone topology changed")
    evidence["parentBoneIndex"] = bone.parent_index
    evidence["runtimeAnchorSlotId"] = RUNTIME_ANCHOR_SLOT_ID
    return evidence


def _resource_evidence(root: Path, resources: list[dict[str, Any]]) -> list[dict[str, Any]]:
    result: list[dict[str, Any]] = []
    slots: set[str] = set()
    for binding in resources:
        slot_id = str(binding.get("slotId") or "")
        asset_id = str(binding.get("assetId") or "")
        if not slot_id or slot_id in slots or not asset_id:
            raise CandidateError("ribbon resource closure is malformed")
        slots.add(slot_id)
        relative = PurePosixPath(asset_id)
        if relative.is_absolute() or ".." in relative.parts or "\\" in asset_id:
            raise CandidateError(f"unsafe ribbon resource asset ID: {asset_id}")
        path = _repository_path(root, PurePosixPath("Client/Bin/Resources") / relative)
        if not path.is_file():
            raise CandidateError(f"ribbon resource is missing: {asset_id}")
        expected = EXPECTED_RESOURCE_EVIDENCE.get(asset_id)
        actual = (path.stat().st_size, raw_sha256(path))
        if expected is None or actual != expected:
            raise CandidateError(
                f"ribbon resource identity changed; review is required: {asset_id}"
            )
        result.append(
            {
                "slotId": slot_id,
                "assetId": asset_id,
                "byteSize": actual[0],
                "sha256": actual[1],
            }
        )
    if {row["assetId"] for row in result} != set(EXPECTED_RESOURCE_EVIDENCE):
        raise CandidateError("ribbon resource denominator changed")
    return result


def _assert_missing_only_identity(root: Path) -> None:
    authored_root = _repository_path(root, PurePosixPath("Data/Effects/Authored"))
    for path in sorted(authored_root.glob("effect.valtan.*.effect.json")):
        document = _read_json(path)
        for element in document.get("elements", []):
            if not isinstance(element, dict):
                continue
            attachment = element.get("actionCueAttachment") or {}
            if (
                element.get("id") == CANDIDATE_ELEMENT_ID
                or element.get("sourceNode") == CANDIDATE_SOURCE_NODE
            ):
                raise CandidateError(
                    "FourSlash weapon Trail candidate identity already exists in canonical data"
                )
            if attachment.get("runtimeAnchorSlotId") == RUNTIME_ANCHOR_SLOT_ID:
                raise CandidateError(
                    "new FourSlash runtime anchor slot already exists in canonical data"
                )


def _candidate_element(source: dict[str, Any]) -> dict[str, Any]:
    result = copy.deepcopy(source)
    result["id"] = CANDIDATE_ELEMENT_ID
    result["displayName"] = "FourSlash weapon-bone Trail (bounded)"
    result["groupId"] = "valtan.project.four-slash.weapon-trail"
    result["sourceNode"] = CANDIDATE_SOURCE_NODE
    result["visible"] = True
    result["kind"] = "trail"
    result["actionCueAttachment"] = {
        "enabled": True,
        "follow": True,
        "sourceAnchorSlotId": RUNTIME_BONE_NAME,
        "runtimeAnchorSlotId": RUNTIME_ANCHOR_SLOT_ID,
        "runtimeBoneName": RUNTIME_BONE_NAME,
        "snapshotRootSourceBasisYawDegrees": 0,
        "socketLocalTransform": {
            "position": [0, 0, 0],
            "rotationDegrees": [0, 0, 0],
            "scale": [1, 1, 1],
        },
    }
    result["transformInheritance"] = {
        "enabled": False,
        "masterElementId": "",
    }
    timing = result["detail"]["timing"]
    timing["startDelaySeconds"] = SOURCE_OCCURRENCE_TIME_SECONDS
    result["sourceRecipe"] = _disabled_source_recipe()
    result["sourcePresentation"] = {"enabled": False}
    return result


def _candidate_document(
    source_document: dict[str, Any], element: dict[str, Any]
) -> dict[str, Any]:
    return {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": SOURCE_EFFECT_ASSET_ID,
        "displayName": "Project candidate: FourSlash weapon-bone Trail",
        "particleSystem": copy.deepcopy(source_document["particleSystem"]),
        "modelCues": [],
        "elements": [element],
    }


def _seal(document: dict[str, Any], field: str) -> None:
    document.pop(field, None)
    document[field] = canonical_sha256(document)


def build_outputs(repository_root: Path) -> dict[Path, bytes]:
    root = repository_root.resolve()
    source_path = _repository_path(root, SOURCE_DOCUMENT_RELATIVE_PATH)
    source_document = _read_json(source_path)
    source = _source_element(source_document)
    _assert_missing_only_identity(root)
    bone_evidence = _bone_evidence(root)
    element = _candidate_element(source)
    candidate = _candidate_document(source_document, element)
    resources = _resource_evidence(root, element["resources"])

    output_root = _repository_path(root, OUTPUT_DIRECTORY_RELATIVE_PATH)
    candidate_path = output_root / CANDIDATE_FILENAME
    manifest_path = output_root / MANIFEST_FILENAME
    candidate_payload = pretty_json_bytes(candidate)
    candidate_relative = PurePosixPath(
        *candidate_path.relative_to(root).parts
    ).as_posix()
    manifest = {
        "schema": "lostark.valtan-four-slash-weapon-trail-candidate",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "generator": {
            "script": (
                "Tools/EffectPipeline/"
                "build_valtan_four_slash_weapon_trail_candidate.py"
            ),
            "schemaPath": SCHEMA_RELATIVE_PATH.as_posix(),
            "policyVersion": 1,
        },
        "policy": {
            "classification": "PROJECT_TUNED",
            "fidelity": "BOUNDED_RECONSTRUCTION",
            "reconcileMode": "MISSING_ONLY",
            "candidateOnly": True,
            "canonicalMutationPerformed": False,
            "cueBindingMutationPerformed": False,
            "catalogMutationPerformed": False,
            "sourceExactClaim": False,
            "drawableProofStatus": "NOT_ATTEMPTED",
        },
        "target": {
            "patternId": "VALTAN_FOUR_SLASH",
            "stageId": "SLASHES",
            "actionId": "valtan.attack.four-slash.active",
            "clipOccurrenceId": "valtan.attack.four-slash.active.clip.02",
            "effectAssetId": SOURCE_EFFECT_ASSET_ID,
            "canonicalPath": SOURCE_DOCUMENT_RELATIVE_PATH.as_posix(),
            "candidatePath": candidate_relative,
            "candidateElementId": CANDIDATE_ELEMENT_ID,
            "candidateSourceNode": CANDIDATE_SOURCE_NODE,
        },
        "inputIdentity": {
            "canonicalDocumentRawSha256": raw_sha256(source_path),
            "canonicalDocumentCanonicalSha256": canonical_sha256(source_document),
            "preservedSourceElementId": SOURCE_ELEMENT_ID,
            "preservedSourceElementSha256": canonical_sha256(source),
            "preservedSourceElementAttachment": copy.deepcopy(
                source["actionCueAttachment"]
            ),
        },
        "boundedProjection": {
            "runtimeAnchorSlotId": RUNTIME_ANCHOR_SLOT_ID,
            "runtimeBoneName": RUNTIME_BONE_NAME,
            "sourceOccurrenceTimeSeconds": SOURCE_OCCURRENCE_TIME_SECONDS,
            "resourceClosureSha256": canonical_sha256(element["resources"]),
            "materialClosureSha256": canonical_sha256(element["material"]),
            "trailGeometrySha256": canonical_sha256(element["detail"]["trail"]),
            "sourceRecipeDisposition": "DISABLED_PROJECT_CARRIER",
            "sourcePresentationDisposition": "DISABLED_NO_SOURCE_EXACT_CLAIM",
            "resourceEvidence": resources,
            "modelBoneEvidence": bone_evidence,
        },
        "candidateIdentity": {
            "rawSha256": hashlib.sha256(candidate_payload).hexdigest(),
            "canonicalSha256": canonical_sha256(candidate),
            "elementSha256": canonical_sha256(element),
            "elementCount": 1,
        },
        "requiredDrawableHarness": {
            "status": "MISSING_STATIONARY_ROOT_MOVING_BONE_FIXTURE",
            "rootWorldPolicy": "IDENTITY_AND_STATIONARY_FOR_ALL_FIXED_STEPS",
            "boneAnchorPolicy": (
                "SUPPLY_DISTINCT_FINITE_b_wp_r_01_WORLD_PER_FIXED_STEP"
            ),
            "positiveAssertions": [
                "candidate Trail produces at least two distinct world points",
                "trail cumulative distance is finite and greater than zero",
                "trail bounds move while root bounds remain stationary",
                "prepare succeeds and draw submission is nonzero",
            ],
            "negativeControls": [
                "stationary root plus stationary bone produces no segment",
                "missing runtime anchor slot fails closed without root fallback",
                "legacy safe-gap source row remains stationary and unchanged",
            ],
            "proofAdmission": "DO_NOT_MARK_DRAWABLE_BEFORE_FIXTURE_PASSES",
        },
    }
    _seal(manifest, "artifactSha256")
    return {
        candidate_path: candidate_payload,
        manifest_path: pretty_json_bytes(manifest),
    }


def _atomic_write(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".tmp.", dir=path.parent
    )
    temporary_path = Path(temporary_name)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary_path, path)
    finally:
        if temporary_path.exists():
            temporary_path.unlink()


def _validate_write_set(root: Path, outputs: dict[Path, bytes]) -> None:
    allowed_root = _repository_path(root, OUTPUT_DIRECTORY_RELATIVE_PATH)
    for path in outputs:
        resolved = path.resolve()
        relative = PurePosixPath(*resolved.relative_to(root.resolve()).parts)
        if allowed_root not in resolved.parents or relative in FORBIDDEN_WRITE_RELATIVE_PATHS:
            raise CandidateError(f"candidate write set escaped isolation: {relative}")


def write_outputs(root: Path, outputs: dict[Path, bytes]) -> None:
    _validate_write_set(root, outputs)
    for path, payload in outputs.items():
        _atomic_write(path, payload)


def check_outputs(root: Path, outputs: dict[Path, bytes]) -> None:
    _validate_write_set(root, outputs)
    stale = [
        path
        for path, expected in outputs.items()
        if not path.is_file() or path.read_bytes() != expected
    ]
    if stale:
        display = ", ".join(str(path.relative_to(root)) for path in stale)
        raise CandidateError(f"FourSlash weapon Trail candidate is stale: {display}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=DEFAULT_REPOSITORY_ROOT,
    )
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    arguments = parser.parse_args()
    root = arguments.repository_root.resolve()
    try:
        outputs = build_outputs(root)
        if arguments.write:
            write_outputs(root, outputs)
            print("Wrote isolated FourSlash weapon-bone Trail candidate (no apply).")
        else:
            check_outputs(root, outputs)
            print("FourSlash weapon-bone Trail candidate is current.")
    except CandidateError as error:
        print(f"ERROR: {error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
