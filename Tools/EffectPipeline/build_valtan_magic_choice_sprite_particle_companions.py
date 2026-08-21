#!/usr/bin/env python3
"""Build isolated MagicChoice SpriteParticle companion candidates.

The builder has no canonical apply mode.  It snapshots four existing
PROJECT_TUNED MagicChoice LocalDecals, translates only their carrier into a
ground-plane SpriteParticle, and writes candidate documents plus a sealed
manifest under one new Imported/Valtan directory.  Cue, binding, catalog,
canonical Effect, renderer, C++, and HLSL files are outside its write set.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import tempfile
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any


SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent

OUTPUT_DIRECTORY_RELATIVE_PATH = PurePosixPath(
    "Data/Effects/Imported/Valtan/ProjectTunedMagicChoiceSpriteParticleCompanions"
)
SCHEMA_RELATIVE_PATH = PurePosixPath(
    "Tools/EffectPipeline/Schemas/"
    "lostark.valtan-magic-choice-sprite-particle-companions.schema.json"
)
MANIFEST_FILENAME = (
    "Valtan.magic-choice-sprite-particle-companions.candidate-manifest.v1.json"
)

RING_002 = "Effect/Valtan/Textures/FX_TEX_01/fx_c_ring_002.dds"
RING_004 = "Effect/Valtan/Textures/FX_TEX_01/fx_c_ring_004_cl.dds"
ATYPICAL_009 = "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_009.dds"
ATYPICAL_011 = "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_011.dds"

EXPECTED_RESOURCE_EVIDENCE = {
    RING_002: (
        16_512,
        "1045f25f5cdb36e4afb66a3bb2417158c4ecbda419d1e31aa269dd620c3c07c0",
    ),
    RING_004: (
        131_200,
        "7fa5d94db7d8587ad94ee5a119119e97221cbbe8b64eda7a0bed2ac6b447f094",
    ),
    ATYPICAL_009: (
        32_896,
        "58a957a8d13005c12afb097c6c8d423d2c94872dfe8b76ab1d51dabbfe67749b",
    ),
    ATYPICAL_011: (
        8_320,
        "4d0d6a1024a7ef55c5ff469729d0f567a2dbbe941c6568f495b5dc72c2169618",
    ),
}


@dataclass(frozen=True)
class CompanionSpec:
    semantic_stage: str
    source_document: PurePosixPath
    source_effect_asset_id: str
    source_element_id: str
    source_element_sha256: str
    candidate_element_id: str
    candidate_display_name: str
    candidate_source_node: str
    candidate_filename: str
    start_size: tuple[float, float]
    end_size: tuple[float, float]


SPECS = (
    CompanionSpec(
        semantic_stage="WINDUP",
        source_document=PurePosixPath(
            "Data/Effects/Authored/effect.valtan.magic-choice.windup.effect.json"
        ),
        source_effect_asset_id="effect.valtan.magic-choice.windup",
        source_element_id="project-donut-outer-boundary",
        source_element_sha256=(
            "8a380ab060a303baa2ed4f2c946620df799f99970e1249c070068b88d928f864"
        ),
        candidate_element_id=(
            "project-donut-outer-boundary.sprite-particle-v1"
        ),
        candidate_display_name="Donut outer boundary SpriteParticle candidate",
        candidate_source_node=(
            "project-authored:valtan.magic-choice.donut.outer-boundary."
            "sprite-particle-v1"
        ),
        candidate_filename=(
            "effect.valtan.magic-choice.windup.sprite-particle-companions."
            "candidate.effect.json"
        ),
        start_size=(18.0, 18.0),
        end_size=(18.0, 18.0),
    ),
    CompanionSpec(
        semantic_stage="WINDUP",
        source_document=PurePosixPath(
            "Data/Effects/Authored/effect.valtan.magic-choice.windup.effect.json"
        ),
        source_effect_asset_id="effect.valtan.magic-choice.windup",
        source_element_id="project-donut-inner-growing-boundary",
        source_element_sha256=(
            "bfe67672071d6e1ed5ac26271a68142f794754762ca8d756e9e5a9faeebcb4b9"
        ),
        candidate_element_id=(
            "project-donut-inner-growing-boundary.sprite-particle-v1"
        ),
        candidate_display_name="Donut growing inner SpriteParticle candidate",
        candidate_source_node=(
            "project-authored:valtan.magic-choice.donut.inner-growing-boundary."
            "sprite-particle-v1"
        ),
        candidate_filename=(
            "effect.valtan.magic-choice.windup.sprite-particle-companions."
            "candidate.effect.json"
        ),
        start_size=(7.0, 7.0),
        end_size=(18.0, 18.0),
    ),
    CompanionSpec(
        semantic_stage="INNER",
        source_document=PurePosixPath(
            "Data/Effects/Authored/effect.valtan.magic-choice.inner.effect.json"
        ),
        source_effect_asset_id="effect.valtan.magic-choice.inner",
        source_element_id="project-donut-inner-impact",
        source_element_sha256=(
            "2ec2a29c72dada7a7fafc753fd29f9d8b249d24f93dee61929deb40932c934cf"
        ),
        candidate_element_id="project-donut-inner-impact.sprite-particle-v1",
        candidate_display_name="Donut inner impact SpriteParticle candidate",
        candidate_source_node=(
            "project-authored:valtan.magic-choice.donut.inner-impact."
            "sprite-particle-v1"
        ),
        candidate_filename=(
            "effect.valtan.magic-choice.inner.sprite-particle-companions."
            "candidate.effect.json"
        ),
        start_size=(8.0, 8.0),
        end_size=(8.0, 8.0),
    ),
    CompanionSpec(
        semantic_stage="OUTER",
        source_document=PurePosixPath(
            "Data/Effects/Authored/effect.valtan.magic-choice.outer.effect.json"
        ),
        source_effect_asset_id="effect.valtan.magic-choice.outer",
        source_element_id="project-donut-outer-impact",
        source_element_sha256=(
            "5ae9b11755dcd28c6a88b5a541fe8fc468361a92caafcf16757cdbe672bf4510"
        ),
        candidate_element_id="project-donut-outer-impact.sprite-particle-v1",
        candidate_display_name="Donut outer impact SpriteParticle candidate",
        candidate_source_node=(
            "project-authored:valtan.magic-choice.donut.outer-impact."
            "sprite-particle-v1"
        ),
        candidate_filename=(
            "effect.valtan.magic-choice.outer.sprite-particle-companions."
            "candidate.effect.json"
        ),
        start_size=(18.0, 18.0),
        end_size=(18.0, 18.0),
    ),
)

FORBIDDEN_WRITE_RELATIVE_PATHS = frozenset(
    {
        PurePosixPath(
            "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
        ),
        PurePosixPath(
            "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
        ),
        PurePosixPath("Data/Effects/EffectCatalog.json"),
        *(spec.source_document for spec in SPECS),
    }
)


class CandidateError(RuntimeError):
    """The isolated candidate cannot be rebuilt without new review."""


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


def _source_row(
    document: dict[str, Any], spec: CompanionSpec
) -> dict[str, Any]:
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != spec.source_effect_asset_id
    ):
        raise CandidateError(
            f"MagicChoice source document identity changed: {spec.source_document}"
        )
    matches = [
        row
        for row in document.get("elements", [])
        if isinstance(row, dict) and row.get("id") == spec.source_element_id
    ]
    if len(matches) != 1:
        raise CandidateError(
            f"MagicChoice source row is missing or duplicated: {spec.source_element_id}"
        )
    row = matches[0]
    attachment = row.get("actionCueAttachment") or {}
    recipe = row.get("sourceRecipe") or {}
    presentation = row.get("sourcePresentation") or {}
    if (
        canonical_sha256(row) != spec.source_element_sha256
        or row.get("kind") != "decal"
        or row.get("visible") is not True
        or attachment.get("enabled") is not False
        or attachment.get("follow") is not False
        or recipe.get("enabled") is not False
        or presentation.get("enabled") is not False
    ):
        raise CandidateError(
            f"MagicChoice source row changed; bounded review is required: "
            f"{spec.source_element_id}"
        )
    return row


def _assert_missing_only_identity(root: Path) -> None:
    candidate_ids = {spec.candidate_element_id for spec in SPECS}
    candidate_nodes = {spec.candidate_source_node for spec in SPECS}
    output_root = _repository_path(root, OUTPUT_DIRECTORY_RELATIVE_PATH)
    search_roots = (
        _repository_path(root, PurePosixPath("Data/Effects/Authored")),
        _repository_path(root, PurePosixPath("Data/Effects/Imported/Valtan")),
    )
    for search_root in search_roots:
        if not search_root.is_dir():
            continue
        for path in sorted(search_root.rglob("*.effect.json")):
            resolved = path.resolve()
            if resolved == output_root or output_root in resolved.parents:
                continue
            document = _read_json(path)
            for element in document.get("elements", []):
                if not isinstance(element, dict):
                    continue
                if (
                    element.get("id") in candidate_ids
                    or element.get("sourceNode") in candidate_nodes
                ):
                    raise CandidateError(
                        "MagicChoice companion identity already exists outside "
                        "the isolated candidate package"
                    )


def _resource_evidence(root: Path) -> list[dict[str, Any]]:
    evidence: list[dict[str, Any]] = []
    for asset_id, expected in EXPECTED_RESOURCE_EVIDENCE.items():
        relative = PurePosixPath(asset_id)
        if relative.is_absolute() or ".." in relative.parts or "\\" in asset_id:
            raise CandidateError(f"unsafe MagicChoice resource asset ID: {asset_id}")
        path = _repository_path(
            root, PurePosixPath("Client/Bin/Resources") / relative
        )
        if not path.is_file():
            raise CandidateError(f"MagicChoice resource is missing: {asset_id}")
        actual = (path.stat().st_size, raw_sha256(path))
        if actual != expected:
            raise CandidateError(
                f"MagicChoice resource identity changed; review is required: {asset_id}"
            )
        evidence.append(
            {
                "assetId": asset_id,
                "byteSize": actual[0],
                "sha256": actual[1],
            }
        )
    return evidence


def _candidate_element(
    source: dict[str, Any], spec: CompanionSpec
) -> dict[str, Any]:
    result = copy.deepcopy(source)
    result["id"] = spec.candidate_element_id
    result["displayName"] = spec.candidate_display_name
    result["groupId"] = "valtan.project.magic-choice.sprite-particle-companion"
    result["sourceNode"] = spec.candidate_source_node
    result["kind"] = "particle"

    detail = result["detail"]
    detail["transform"]["rotationDegrees"] = [90.0, 0.0, 0.0]
    detail["sprite"]["billboard"] = False
    detail["sprite"]["billboardRollDegrees"] = 0.0

    # Translate the decal's world footprint into SpriteParticle size.  The
    # growing inner boundary uses particle size interpolation so element scale
    # is not applied a second time by EffectPlayback.
    detail["linearLerp"]["scale"] = False
    detail["linearLerp"]["endScale"] = [1.0, 1.0, 1.0]
    particle = detail["particle"]
    particle["maxParticles"] = 1
    particle["spawnRatePerSecond"] = 0
    particle["burstCount"] = 1
    particle["lifeTimeSeconds"] = [
        detail["timing"]["lifeTimeSeconds"],
        detail["timing"]["lifeTimeSeconds"],
    ]
    particle["startSize"] = list(spec.start_size)
    particle["endSize"] = list(spec.end_size)
    particle["localSpace"] = True
    particle["billboard"] = False
    result["sourceRecipe"] = {
        "enabled": False,
        "rendererShape": "",
        "emitterDelaySeconds": 0,
        "emitterDurationSeconds": 0,
        "emitterLoopCount": 1,
        "bursts": [],
        "modules": [],
    }
    result["sourcePresentation"] = {"enabled": False}
    return result


def _candidate_document(
    source_document: dict[str, Any], elements: list[dict[str, Any]]
) -> dict[str, Any]:
    return {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": source_document["effectAssetId"],
        "displayName": (
            "Project candidate: MagicChoice ground SpriteParticle companions"
        ),
        "particleSystem": copy.deepcopy(source_document["particleSystem"]),
        "modelCues": [],
        "elements": elements,
    }


def _common_color_abi_contract() -> dict[str, Any]:
    return {
        "status": "REQUIRED_BEFORE_PRODUCT_ADMISSION",
        "minimumContractVersion": 1,
        "variantSelection": "EXPLICIT_TYPED_VARIANT_NO_FILENAME_HEURISTICS",
        "lanes": [
            {
                "assetId": RING_002,
                "roles": ["BASE_RADIANCE", "COVERAGE"],
                "rgbDecode": "SRGB_TO_SCENE_LINEAR",
                "dataDecode": "LINEAR",
                "coverageSwizzle": "A",
                "coverageSemantic": "RING_ALPHA",
            },
            {
                "assetId": RING_004,
                "roles": ["MASK"],
                "rgbDecode": "LINEAR",
                "dataDecode": "LINEAR",
                "coverageSwizzle": "R",
                "coverageSemantic": "GRAYSCALE_LUMINANCE_MASK",
            },
            {
                "assetId": ATYPICAL_009,
                "roles": ["BASE_RADIANCE"],
                "rgbDecode": "SRGB_TO_SCENE_LINEAR",
                "dataDecode": "LINEAR",
                "coverageSwizzle": "NONE",
                "coverageSemantic": "NOT_A_COVERAGE_OWNER",
            },
            {
                "assetId": ATYPICAL_011,
                "roles": ["DISSOLVE"],
                "rgbDecode": "LINEAR",
                "dataDecode": "LINEAR",
                "coverageSwizzle": "R",
                "coverageSemantic": "DISSOLVE_SCALAR",
            },
        ],
        "coverage": {
            "boundary": {
                "terms": [
                    {
                        "sourceLane": "base",
                        "assetId": RING_002,
                        "swizzle": "A",
                        "operator": "REPLACE",
                    },
                    {
                        "sourceLane": "mask",
                        "assetId": RING_004,
                        "swizzle": "R",
                        "operator": "MULTIPLY",
                    },
                    {
                        "sourceLane": "particle",
                        "assetId": "",
                        "swizzle": "A",
                        "operator": "MULTIPLY",
                    },
                ],
                "threshold": 0.0,
                "softness": 0.0,
                "particleAlphaMode": "MULTIPLY_COVERAGE",
            },
            "impact": {
                "terms": [
                    {
                        "sourceLane": "mask",
                        "assetId": RING_002,
                        "swizzle": "A",
                        "operator": "REPLACE",
                    },
                    {
                        "sourceLane": "dissolve",
                        "assetId": ATYPICAL_011,
                        "swizzle": "R",
                        "operator": "DISSOLVE_MULTIPLY",
                    },
                    {
                        "sourceLane": "particle",
                        "assetId": "",
                        "swizzle": "A",
                        "operator": "MULTIPLY",
                    },
                ],
                "threshold": 0.0,
                "softness": 0.0,
                "particleAlphaMode": "MULTIPLY_COVERAGE",
            },
            "forbidDxt1ImplicitAlphaCoverage": True,
        },
        "radiance": {
            "baseRadianceSpace": "SCENE_LINEAR",
            "authoredEmissiveIntensitySource": "DETAIL_COLOR_EMISSIVE_INTENSITY",
            "emissiveOperation": "MULTIPLY_BASE_RADIANCE",
            "allowValuesAboveOne": True,
            "internalHdrClamp": "FORBIDDEN",
            "internalToneMap": "FORBIDDEN",
            "toneMapOwner": "EXTERNAL_RENDERING_PROFILE",
        },
        "blendProfiles": {
            "boundary": "STRAIGHT_ALPHA_SRC_ALPHA_INV_SRC_ALPHA",
            "impact": "SRC_ALPHA_ADDITIVE_SRC_ALPHA_ONE",
        },
    }


def _seal(document: dict[str, Any], field: str) -> None:
    document.pop(field, None)
    document[field] = canonical_sha256(document)


def build_outputs(repository_root: Path) -> dict[Path, bytes]:
    root = repository_root.resolve()
    _assert_missing_only_identity(root)
    resource_evidence = _resource_evidence(root)

    document_cache: dict[PurePosixPath, dict[str, Any]] = {}
    source_rows: dict[str, dict[str, Any]] = {}
    candidate_rows: dict[str, dict[str, Any]] = {}
    for spec in SPECS:
        if spec.source_document not in document_cache:
            document_cache[spec.source_document] = _read_json(
                _repository_path(root, spec.source_document)
            )
        source = _source_row(document_cache[spec.source_document], spec)
        source_rows[spec.candidate_element_id] = source
        candidate_rows[spec.candidate_element_id] = _candidate_element(source, spec)

    output_root = _repository_path(root, OUTPUT_DIRECTORY_RELATIVE_PATH)
    outputs: dict[Path, bytes] = {}
    candidate_documents: dict[str, dict[str, Any]] = {}
    for filename in sorted({spec.candidate_filename for spec in SPECS}):
        grouped_specs = [spec for spec in SPECS if spec.candidate_filename == filename]
        source_document = document_cache[grouped_specs[0].source_document]
        candidate = _candidate_document(
            source_document,
            [candidate_rows[spec.candidate_element_id] for spec in grouped_specs],
        )
        candidate_documents[filename] = candidate
        outputs[output_root / filename] = pretty_json_bytes(candidate)

    source_identity = []
    candidate_identity = []
    for spec in SPECS:
        source_path = _repository_path(root, spec.source_document)
        source = source_rows[spec.candidate_element_id]
        candidate = candidate_rows[spec.candidate_element_id]
        source_identity.append(
            {
                "semanticStage": spec.semantic_stage,
                "canonicalPath": spec.source_document.as_posix(),
                "effectAssetId": spec.source_effect_asset_id,
                "sourceElementId": spec.source_element_id,
                "sourceElementSha256": canonical_sha256(source),
                "canonicalDocumentRawSha256": raw_sha256(source_path),
                "canonicalDocumentCanonicalSha256": canonical_sha256(
                    document_cache[spec.source_document]
                ),
                "resourceClosureSha256": canonical_sha256(source["resources"]),
                "materialClosureSha256": canonical_sha256(source["material"]),
                "timingSha256": canonical_sha256(source["detail"]["timing"]),
                "colorSha256": canonical_sha256(source["detail"]["color"]),
                "uvSha256": canonical_sha256(source["detail"]["uv"]),
            }
        )
        candidate_identity.append(
            {
                "semanticStage": spec.semantic_stage,
                "candidatePath": (
                    OUTPUT_DIRECTORY_RELATIVE_PATH / spec.candidate_filename
                ).as_posix(),
                "candidateElementId": spec.candidate_element_id,
                "candidateSourceNode": spec.candidate_source_node,
                "candidateElementSha256": canonical_sha256(candidate),
            }
        )

    document_identity = []
    for filename, document in sorted(candidate_documents.items()):
        payload = outputs[output_root / filename]
        document_identity.append(
            {
                "candidatePath": (
                    OUTPUT_DIRECTORY_RELATIVE_PATH / filename
                ).as_posix(),
                "effectAssetId": document["effectAssetId"],
                "rawSha256": hashlib.sha256(payload).hexdigest(),
                "canonicalSha256": canonical_sha256(document),
                "elementCount": len(document["elements"]),
            }
        )

    manifest = {
        "schema": "lostark.valtan-magic-choice-sprite-particle-companions",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "generator": {
            "script": (
                "Tools/EffectPipeline/"
                "build_valtan_magic_choice_sprite_particle_companions.py"
            ),
            "schemaPath": SCHEMA_RELATIVE_PATH.as_posix(),
            "policyVersion": 1,
        },
        "policy": {
            "classification": "PROJECT_TUNED",
            "fidelity": "BOUNDED_CARRIER_TRANSLATION",
            "reconcileMode": "MISSING_ONLY",
            "candidateOnly": True,
            "canonicalMutationPerformed": False,
            "cueMutationPerformed": False,
            "bindingMutationPerformed": False,
            "catalogMutationPerformed": False,
            "commonRuntimeMutationPerformed": False,
            "sourceExactClaim": False,
            "drawableProofStatus": "NOT_ATTEMPTED_COMMON_COLOR_ABI_REQUIRED",
            "productAdmission": "BLOCKED_CANDIDATE_ONLY",
        },
        "target": {
            "patternId": "VALTAN_MAGIC_CHOICE",
            "carrierFamily": "SPRITE_PARTICLE",
            "sourceCarrierFamily": "LOCAL_DECAL",
            "candidateCount": 4,
            "candidateDocumentCount": 3,
        },
        "sourceIdentity": source_identity,
        "candidateIdentity": candidate_identity,
        "candidateDocumentIdentity": document_identity,
        "resourceEvidence": resource_evidence,
        "carrierTranslation": {
            "kind": "particle",
            "meshModelForbidden": True,
            "groundPlaneRotationDegrees": [90.0, 0.0, 0.0],
            "billboard": False,
            "burstCount": 1,
            "localSpace": True,
            "timingPolicy": "COPY_EXACT_FROM_SOURCE_ROW",
            "colorPolicy": "COPY_EXACT_FROM_SOURCE_ROW",
            "uvPolicy": "COPY_EXACT_FROM_SOURCE_ROW",
            "resourcePolicy": "COPY_EXACT_FROM_SOURCE_ROW",
            "materialPolicy": "COPY_EXACT_FROM_SOURCE_ROW",
            "innerGrowthPolicy": "PARTICLE_SIZE_7_TO_18_NO_DOUBLE_ELEMENT_SCALE",
            "attachmentPolicy": "COPY_DISABLED_SOURCE_ATTACHMENT",
        },
        "requiresCommonColorAbi": _common_color_abi_contract(),
        "requiredValidation": {
            "status": "NOT_RUN",
            "requiredBeforeDrawableClaim": [
                "common color ABI implements every requiresCommonColorAbi field",
                "candidate-only effect document codec validation passes",
                "isolated fixed-step prepare and draw proof passes",
                "cue and binding deep hashes remain unchanged",
                "user performs visual comparison under fixed Valtan render profile",
            ],
            "proofAdmission": "DO_NOT_MARK_DRAWABLE_OR_SOURCE_EXACT_YET",
        },
    }
    _seal(manifest, "artifactSha256")
    outputs[output_root / MANIFEST_FILENAME] = pretty_json_bytes(manifest)
    return outputs


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
    resolved_root = root.resolve()
    allowed_root = _repository_path(root, OUTPUT_DIRECTORY_RELATIVE_PATH)
    for path in outputs:
        resolved = path.resolve()
        try:
            relative = PurePosixPath(*resolved.relative_to(resolved_root).parts)
        except ValueError as error:
            raise CandidateError(f"candidate write escaped repository: {resolved}") from error
        if (
            allowed_root not in resolved.parents
            or relative in FORBIDDEN_WRITE_RELATIVE_PATHS
        ):
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
        raise CandidateError(
            f"MagicChoice SpriteParticle candidate package is stale: {display}"
        )


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
            print(
                "Wrote isolated MagicChoice SpriteParticle companion candidates "
                "(no apply)."
            )
        else:
            check_outputs(root, outputs)
            print("MagicChoice SpriteParticle companion candidates are current.")
    except CandidateError as error:
        print(f"ERROR: {error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
