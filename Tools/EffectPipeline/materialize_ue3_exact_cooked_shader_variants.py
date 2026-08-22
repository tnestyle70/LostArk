#!/usr/bin/env python3
"""Materialize class-neutral UE3 exact cooked pixel-shader variants.

The input receipts already prove an exact Material shader map, one packed DXBC
container, native CB/SRV/sampler wires, a structural WARP replay, source-value
Material CB0 rows, and exact texture bindings.  This tool joins those receipts
by family identity and emits two source artifacts only:

* content-addressed ``Data/Effects/CookedShaders/<sha256>.dxbc`` blobs; and
* ``Data/Effects/Contracts/ue3-exact-cooked-shader-variants.v1.json``.

The reusable key is Material-family/permutation based.  Character, skill,
input-slot, clip, effect, target, and occurrence identifiers never participate
in the emitted contract.  Unknown address/native constructor/TextureLODSettings
values, engine-owned CB rows, raw vertex execution and vertex CBs, Product
runtime, and visual admission remain explicitly open.  Source-exact sampler
components and target-padding-free scalar evidence are kept separate from
full-source closure.  An authoring-preview sampler candidate is emitted only
when every unresolved component has an explicit upstream candidate.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import sys
from pathlib import Path
from typing import Any, Iterable


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
TOOL_ROOT = Path(__file__).resolve().parent
if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

from extract_ue3_material_shader_maps import (  # noqa: E402
    package_tables,
    require,
)
from extract_artist_31470_shader_cache_oracle import (  # noqa: E402
    canonical_json_sha256,
)
from replay_ue3_material_pixel_shaders import rehydrate_dxbc  # noqa: E402


SCHEMA = "lostark.effect-ue3-exact-cooked-shader-variants"
FORMAT_VERSION = 1
EXACT_SCHEMA = "lostark.effect-ue3-material-shader-map-receipt"
REPLAY_SCHEMA = "lostark.effect-ue3-material-fixed-input-replay-receipt"
UNIFORM_SCHEMA = "lostark.effect-ue3-source-value-uniform-evaluation-receipt"
TEXTURE_SCHEMA = "lostark.effect-ue3-material-texture-sampler-closure-receipt"
EXACT_STATUS = "EXACT_MATERIAL_SHADER_MAP"
NATIVE_STATUS = "EXACT_NATIVE_SHADER_OBJECT_BINDING"
UNIFORM_STATUS = "EXACT_SOURCE_VALUE_UNIFORM_CB0_CLOSURE"
TEXTURE_STATUS = "EXACT_TEXTURE_BINDING_SAMPLER_BLOCKED"
PREVIEW_FIDELITY = "PROJECT_PREVIEW_APPROXIMATE"
SCALAR_PACKING_BLOCKER = (
    "NATIVE_SCALAR_GROUP_LANE_ORDER_AND_PADDING_SOURCE_ABI_NOT_PROVEN"
)
SCALAR_PACKING_SOURCE_BLOCKER = (
    "NATIVE_SCALAR_GROUP_PACKING_SOURCE_REVISION_NOT_CLOSED"
)
TARGET_PADDING_FREE_SCALAR_FIDELITY = (
    "EPIC_PACKED_SCALAR_ABI_CORROBORATED_AND_TARGET_PADDING_FREE_NOT_SOURCE_CLOSED"
)

DEFAULT_EXACT = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.exact-material-maps.receipt.json"
)
DEFAULT_REPLAY = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.structural-fixed-input-replay.receipt.json"
)
DEFAULT_UNIFORM = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.source-value-uniform-evaluation.receipt.json"
)
DEFAULT_TEXTURE = REPOSITORY_ROOT / (
    "Data/Effects/Imported/DimensionMaster/Materials/"
    "skill.2050120.clip3.exact-texture-sampler-closure.receipt.json"
)
DEFAULT_CACHE = Path(
    "C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Effect/ARTIST/"
    "31470_TrackA_20260812/OfficialRefShaderCacheV974/"
    "EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk"
)
DEFAULT_BLOB_ROOT = REPOSITORY_ROOT / "Data/Effects/CookedShaders"
DEFAULT_OUTPUT = REPOSITORY_ROOT / (
    "Data/Effects/Contracts/ue3-exact-cooked-shader-variants.v1.json"
)
SCRIPT_PATH = Path(__file__).resolve()

FORBIDDEN_SELECTOR_KEYS = {
    "characterClass",
    "skillId",
    "inputSlot",
    "clip",
    "effectAssetId",
    "targetId",
    "occurrenceId",
    "occurrenceIds",
}


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while payload := stream.read(1024 * 1024):
            digest.update(payload)
    return digest.hexdigest()


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"))
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return value


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    os.replace(temporary, path)


def write_blob_atomic(path: Path, value: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        require(path.read_bytes() == value, f"content-addressed blob changed: {path}")
        return
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_bytes(value)
    os.replace(temporary, path)


def repository_relative(path: Path) -> str:
    return path.resolve().relative_to(REPOSITORY_ROOT.resolve()).as_posix()


def validate_sealed_receipt(
    path: Path,
    schema: str,
    format_version: int,
) -> dict[str, Any]:
    value = read_json(path)
    require(value.get("schema") == schema, f"receipt schema changed: {path}")
    require(
        value.get("formatVersion") == format_version,
        f"receipt format changed: {path}",
    )
    claimed = value.get("receiptSha256")
    unsigned = dict(value)
    unsigned.pop("receiptSha256", None)
    require(
        claimed == canonical_json_sha256(unsigned),
        f"receipt seal changed: {path}",
    )
    return value


def receipt_descriptor(path: Path, value: dict[str, Any]) -> dict[str, Any]:
    return {
        "repositoryRelativePath": repository_relative(path),
        "fileRawSha256": sha256_file(path),
        "schema": value["schema"],
        "formatVersion": value["formatVersion"],
        "receiptSha256": value["receiptSha256"],
    }


def family_index(
    rows: Iterable[dict[str, Any]],
    *,
    status: str | None = None,
) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        if status is not None and row.get("status") != status:
            continue
        family_id = row.get("familyId")
        require(isinstance(family_id, str) and family_id, "family ID is absent")
        require(family_id not in result, f"family ID is duplicated: {family_id}")
        result[family_id] = row
    return result


def preview_component(
    evidence: dict[str, Any],
    allowed: set[str],
) -> dict[str, Any] | None:
    value = evidence.get("value")
    if evidence.get("sourceExact") is True and isinstance(value, str):
        require(value in allowed, f"unsupported source-exact preview value: {value}")
        return {
            "value": value,
            "fidelity": evidence.get("fidelity"),
            "sourceExact": True,
            "provenance": "SOURCE_EXACT_VALUE",
        }
    candidate = evidence.get("valueCandidate")
    if isinstance(candidate, str):
        require(candidate in allowed, f"unsupported explicit preview candidate: {candidate}")
        return {
            "value": candidate,
            "fidelity": evidence.get("candidateFidelity", evidence.get("fidelity")),
            "sourceExact": False,
            "provenance": "EXPLICIT_UPSTREAM_PREVIEW_CANDIDATE",
        }
    return None


def preview_sampler(binding: dict[str, Any]) -> dict[str, Any] | None:
    evidence = binding["sourceTexture2D"]["samplerAndColorSpace"]
    address_u = preview_component(evidence["addressU"], {"ta_wrap", "ta_clamp"})
    address_v = preview_component(evidence["addressV"], {"ta_wrap", "ta_clamp"})
    hardware_filter = preview_component(
        evidence["hardwareFilter"], {"point", "linear", "anisotropic"}
    )
    color_space = preview_component(evidence["colorSpace"], {"linear", "srgb"})
    components = {
        "addressU": address_u,
        "addressV": address_v,
        "hardwareFilter": hardware_filter,
        "colorSpace": color_space,
    }
    if any(value is None for value in components.values()):
        return None
    assert address_u is not None
    assert address_v is not None
    assert hardware_filter is not None
    assert color_space is not None
    return {
        "fidelity": PREVIEW_FIDELITY,
        "addressU": address_u["value"].removeprefix("ta_"),
        "addressV": address_v["value"].removeprefix("ta_"),
        "hardwareFilter": hardware_filter["value"],
        "colorSpace": color_space["value"],
        "components": components,
        "sourceExact": False,
        "reason": (
            "SOURCE_EXACT_COMPONENTS_PLUS_EXPLICIT_UPSTREAM_PREVIEW_CANDIDATES_ONLY"
        ),
    }


def slim_texture_binding(binding: dict[str, Any]) -> dict[str, Any]:
    dds = binding["ddsIdentity"]
    runtime = dds["runtimeDimensionMaster"]
    sampler = binding["sourceTexture2D"]["samplerAndColorSpace"]
    return {
        "uniformExpressionIndex": binding["uniformExpressionIndex"],
        "expressionType": binding["expressionType"],
        "parameterFNameKey": binding.get("parameterFNameKey"),
        "effectiveSourceObjectPath": binding["effectiveSourceObjectPath"],
        "textureRegister": binding["textureRegister"],
        "samplerRegister": binding["samplerRegister"],
        "bindingFidelity": binding["bindingFidelity"],
        "sourceExactDds": dds["sourceExactDds"],
        "fixtureRuntimeDdsEvidence": {
            "resourceRelativePath": runtime["relativePath"],
            "status": runtime["status"],
            "byteSize": runtime["byteSize"],
            "sha256": runtime["sha256"],
            "sourceExactParity": runtime["sourceExactParity"],
        },
        "sourceSamplerAndColorSpaceEvidence": sampler,
        "sourceExactSamplerPartialEvidence": {
            "colorSpace": sampler["colorSpace"],
            "filterSelector": sampler["filterSelector"],
            "addressU": sampler["addressU"],
            "addressV": sampler["addressV"],
            "hardwareFilter": sampler["hardwareFilter"],
            "sourceExactColorSpace": sampler["sourceExactColorSpace"],
            "sourceExactFilterSelector": sampler["sourceExactFilterSelector"],
            "sourceExactAddressAxisCount": sampler["sourceExactAddressAxisCount"],
            "sourceExactHardwareFilter": sampler["sourceExactHardwareFilter"],
            "fullSamplerSourceExact": False,
        },
        "authoringPreviewSamplerCandidate": preview_sampler(binding),
    }


def engine_row_policy(
    renderer_type: str,
    native_cb0: dict[str, Any],
) -> dict[str, Any]:
    unbound = [
        row["slot"]
        for row in native_cb0["allRows"]
        if row["ownership"] == "ENGINE_OR_RENDERER_INPUT_UNBOUND_AT_G03_5"
    ]
    candidates: list[dict[str, Any]] = []
    if 0 in unbound:
        candidates.append(
            {
                "slot": 0,
                "semantic": "externalOpacity",
                "candidateValue": [1.0, 0.0, 0.0, 0.0],
                "fidelity": PREVIEW_FIDELITY,
                "evidence": "G03_4_CB0_ROW0_LANE0_EXTERNAL_OPACITY_SENSITIVITY",
            }
        )
    if renderer_type == "MeshParticle" and 1 in unbound:
        candidates.append(
            {
                "slot": 1,
                "semantic": "particleOrRendererRgba",
                "candidateValue": [1.0, 1.0, 1.0, 1.0],
                "fidelity": PREVIEW_FIDELITY,
                "evidence": "PROJECT_CANARY_CARRIER_CANDIDATE_NOT_SOURCE_EXACT",
            }
        )
    candidate_slots = {row["slot"] for row in candidates}
    unresolved = sorted(set(unbound) - candidate_slots)
    return {
        "sourceUnboundSlots": sorted(unbound),
        "authoringPreviewCandidates": candidates,
        "unresolvedSlots": unresolved,
        "sourceExact": False,
    }


def preview_cb0_rows(
    native_cb0: dict[str, Any],
    row_policy: dict[str, Any],
) -> list[dict[str, Any]]:
    packing = native_cb0.get("nativeScalarGroupPackingEvidence", {})
    target_padding_free = bool(
        packing.get("targetPaddingFreeCorroborationAdmission")
    )
    rows = [
        {
            "slot": row["slot"],
            "value": row["value"],
            "source": row["source"],
            "fidelity": (
                (
                    TARGET_PADDING_FREE_SCALAR_FIDELITY
                    if target_padding_free
                    else "PROJECT_PREVIEW_APPROXIMATE_SCALAR_GROUP_PACKING_UNPROVEN"
                )
                if row["source"] == "PIXEL_SCALAR_EXPRESSION_GROUP"
                else "SOURCE_EXACT_VECTOR_MATERIAL_UNIFORM_EXPRESSION_AT_TIME_ZERO"
            ),
        }
        for row in native_cb0["materialRows"]
    ]
    rows.extend(
        {
            "slot": row["slot"],
            "value": row["candidateValue"],
            "fidelity": row["fidelity"],
            "semantic": row["semantic"],
        }
        for row in row_policy["authoringPreviewCandidates"]
    )
    return sorted(rows, key=lambda row: row["slot"])


def scalar_packing_projection(native_cb0: dict[str, Any]) -> dict[str, Any]:
    evidence = native_cb0.get("nativeScalarGroupPackingEvidence")
    require(isinstance(evidence, dict), "native scalar-group packing evidence is absent")
    require(
        evidence.get("nativeScalarGroupPackingSourceClosed") is False,
        "native scalar-group source closure was unexpectedly admitted",
    )
    require(
        evidence.get("sourceExactAdmission") is False,
        "native scalar-group source exactness was unexpectedly admitted",
    )
    target_padding_free = bool(
        evidence.get("targetPaddingFreeCorroborationAdmission")
    )
    if target_padding_free:
        require(evidence.get("targetPaddingFree") is True, "padding-free evidence changed")
        require(evidence.get("partialGroupCount") == 0, "partial scalar group was admitted")
        require(
            evidence.get("completeGroupCount") == evidence.get("selectedGroupCount"),
            "scalar group denominator changed",
        )
        require(
            isinstance(evidence.get("packedLittleEndianSha256"), str),
            "padding-free scalar payload seal is absent",
        )
    return {
        **evidence,
        "nativeScalarGroupPackingSourceClosed": False,
        "sourceExactAdmission": False,
        "productRuntimeAdmission": False,
    }


def source_emitter_vertex_pass_evidence(
    exact: dict[str, Any],
    bytecode: bytes | None,
) -> dict[str, Any] | None:
    evidence = exact.get("sourceEmitterVertexFactoryPass")
    if evidence is None:
        require(bytecode is None, "vertex bytecode has no source emitter evidence")
        return None
    require(isinstance(evidence, dict), "source emitter VF/pass evidence changed")
    require(bytecode is not None, "source emitter exact vertex shader bytes are absent")
    shader = evidence["exactVertexShader"]
    expected = shader["dxbc"]
    require(len(bytecode) == expected["byteSize"], "vertex DXBC byte count changed")
    require(sha256_bytes(bytecode) == expected["sha256"], "vertex DXBC SHA changed")
    admission = evidence["admission"]
    selection = evidence["sourceEmitterVertexFactorySelection"]
    pass_selection = evidence["authoringVertexPassSelection"]
    require(
        isinstance(selection.get("vertexFactoryType"), str),
        "source emitter VF identity is absent",
    )
    require(
        pass_selection.get("densityPolicy") == "NO_DENSITY_AUTHORING_BOUNDED",
        "authoring pass is not the bounded NoDensity selection",
    )
    require(
        pass_selection.get("vertexShaderType") == shader["shaderType"]
        and pass_selection.get("vertexShaderIdHex") == shader["shaderIdHex"],
        "bounded authoring pass and exact vertex shader identity diverged",
    )
    require(
        evidence["vertexPixelSignatureClosure"].get("pass") is True,
        "vertex/pixel signature link failed",
    )
    require(admission["sourceEmitterVertexFactorySelection"], "source emitter VF is open")
    require(admission["exactVertexShaderBlob"], "exact vertex shader blob is open")
    require(
        admission["exactVertexPixelSignatureClosure"],
        "vertex/pixel signature closure is open",
    )
    require(admission["authoringNoDensityPass"], "bounded NoDensity pass is open")
    require(not admission["rawVertexShaderExecution"], "raw vertex shader was executed")
    require(not admission["sourceExactVertexCb"], "vertex CB was admitted")
    require(not admission["productVfPass"], "Product VF/pass was admitted")
    source_recipe = evidence["sourceRecipe"]
    required_module = source_recipe["requiredModule"]
    dynamic_module = source_recipe["dynamicModule"]
    return {
        "status": "SOURCE_EMITTER_VF_AND_BOUNDED_AUTHORING_NO_DENSITY_PASS_EVIDENCE",
        "sourceRecipeModuleEvidence": {
            "rendererShape": source_recipe["rendererShape"],
            "requiredModule": {
                "className": required_module["className"],
                "boffsetcenter": required_module["boffsetcenter"],
                "screenalignment": required_module["screenalignment"],
            },
            "dynamicModule": {
                "className": dynamic_module["className"],
                "updateflags": dynamic_module["updateflags"],
            },
        },
        "sourceEmitterVertexFactorySelection": selection,
        "authoringVertexPassSelection": pass_selection,
        "vertexShader": {
            "shaderType": shader["shaderType"],
            "shaderIdHex": shader["shaderIdHex"],
            "profile": evidence["vertexShaderDeclarationClosure"]["profile"],
            "byteCount": len(bytecode),
            "sha256": sha256_bytes(bytecode),
            "blobAssetId": f"CookedShaders/{sha256_bytes(bytecode)}.dxbc",
            "repositoryRelativePath": (
                f"Data/Effects/CookedShaders/{sha256_bytes(bytecode)}.dxbc"
            ),
            "exactDxbcContainer": True,
        },
        "vertexShaderDeclarationClosure": evidence[
            "vertexShaderDeclarationClosure"
        ],
        "vertexShaderInputSignature": evidence["vertexShaderInputSignature"],
        "vertexShaderOutputSignature": evidence["vertexShaderOutputSignature"],
        "selectedPixelShaderInputSignature": evidence[
            "selectedPixelShaderInputSignature"
        ],
        "vertexPixelSignatureClosure": evidence["vertexPixelSignatureClosure"],
        "admission": {
            **admission,
            "rawVertexShaderExecution": False,
            "sourceExactVertexCb": False,
            "productVfPass": False,
            "runtime": False,
            "visual": False,
        },
    }


def variant_key(exact: dict[str, Any]) -> dict[str, Any]:
    structural = exact["structuralVfPassCandidate"]
    selected = structural["selectedPixelPassReference"]
    vertex_factories = sorted(selected["vertexFactoryTypes"])
    return {
        "familyId": exact["familyId"],
        "baseMaterialIdHex": exact["baseMaterialIdHex"],
        "effectiveStaticParameterSetSha256": exact["mic"][
            "engineEqualityStaticParameterSetSha256"
        ],
        "shaderPlatformOrdinal": exact["materialMap"]["trailerPlatformOrdinal"],
        "rendererType": exact["rendererType"],
        "structuralVertexFactoryCandidateTypes": vertex_factories,
        "structuralVertexFactoryCandidateSetSha256": canonical_json_sha256(
            vertex_factories
        ),
        "passShaderType": selected["shaderType"],
        "pixelShaderIdHex": selected["shaderIdHex"],
    }


def structural_replay_evidence(replay: dict[str, Any]) -> dict[str, Any]:
    carrier = replay["carrierVertexShader"]
    opacity = replay["externalOpacityInputSensitivity"]
    return {
        "pixelShaderCreation": replay["pixelShaderCreation"],
        "carrier": {
            "sourceSha256": carrier["sourceSha256"],
            "compiledDxbcSha256": carrier["compiledDxbcSha256"],
            "linkageContract": carrier["linkageContract"],
            "inputSignatureSha256": canonical_json_sha256(
                carrier["inputSignature"]
            ),
            "outputSignatureSha256": canonical_json_sha256(
                carrier["outputSignature"]
            ),
            "signatureClosure": carrier["signatureClosure"],
        },
        "pixelOutputSignature": replay["outputSignature"],
        "renderTargetCount": replay["renderTargetCount"],
        "runtimeDeclarations": replay["runtimeDeclarations"],
        "nativeBinding": replay["nativeBinding"],
        "baselineRt0Nonzero": replay["baseline"]["rt0Nonzero"],
        "mrtContract": replay["baseline"]["mrtContract"],
        "externalOpacitySensitivity": {
            "mutation": opacity["mutation"],
            "pass": opacity["pass"],
        },
        "structuralFixedInputReplayAdmission": replay[
            "structuralFixedInputReplayAdmission"
        ],
        "sourceValueReplayAdmission": replay["sourceValueReplayAdmission"],
        "actualVfPassAdmission": replay["actualVfPassAdmission"],
        "runtimeAdmission": replay["runtimeAdmission"],
        "visualAdmission": replay["visualAdmission"],
    }


def native_binding_evidence(exact: dict[str, Any]) -> dict[str, Any]:
    binding = exact["nativeShaderObjectBinding"]
    return {
        "status": binding["status"],
        "shaderObject": binding["shaderObject"],
        "bindingSemanticSha256": binding["bindingSemanticSha256"],
        "scalarGroups": binding["scalarGroups"],
        "vectors": binding["vectors"],
        "textures": binding["textures"],
        "constantBufferClosure": binding["constantBufferClosure"],
        "textureSampleClosure": binding["textureSampleClosure"],
        "dxbcDeclarationClosure": binding["dxbcDeclarationClosure"],
        "wireEntryFormat": binding["wireEntryFormat"],
        "runtimeAdmission": False,
        "actualVfPassAdmission": False,
    }


def build_variant(
    exact: dict[str, Any],
    replay: dict[str, Any],
    uniform: dict[str, Any],
    texture: dict[str, Any],
    bytecode: bytes,
    vertex_bytecode: bytes | None = None,
) -> dict[str, Any]:
    require(uniform.get("status") == UNIFORM_STATUS, "uniform closure is absent")
    require(texture.get("status") == TEXTURE_STATUS, "texture closure is absent")
    require(
        texture.get("sourceExactSamplerAdmission") is False,
        "upstream full sampler exactness was admitted",
    )
    require(
        texture.get("sourceValueTextureSamplerAdmission") is False,
        "upstream source-value sampler replay was admitted",
    )
    require(
        exact["sourceMaterialPath"] == texture["sourceMaterialPath"],
        "source Material identity changed between receipts",
    )
    pixel = exact["cookedPixelShader"]
    expected_dxbc = pixel["dxbc"]
    require(len(bytecode) == expected_dxbc["byteSize"], "DXBC byte count changed")
    require(sha256_bytes(bytecode) == expected_dxbc["sha256"], "DXBC SHA changed")

    key = variant_key(exact)
    key_sha = canonical_json_sha256(key)
    evaluation = uniform["sourceValueUniformEvaluation"]
    native_cb0 = evaluation["nativeCb0"]
    scalar_packing = scalar_packing_projection(native_cb0)
    target_padding_free = bool(
        scalar_packing["targetPaddingFreeCorroborationAdmission"]
    )
    row_policy = engine_row_policy(exact["rendererType"], native_cb0)
    bindings = [slim_texture_binding(row) for row in texture["uniformTextureBindings"]]
    require(
        all(
            binding["sourceExactSamplerPartialEvidence"]["fullSamplerSourceExact"]
            is False
            and binding["sourceSamplerAndColorSpaceEvidence"].get(
                "sourceExactSamplerAndColorSpace"
            )
            is False
            for binding in bindings
        ),
        "a full source-exact sampler binding was admitted",
    )
    sampler_candidates = [
        row["authoringPreviewSamplerCandidate"]
        for row in bindings
        if row["authoringPreviewSamplerCandidate"] is not None
    ]
    vertex_pass = source_emitter_vertex_pass_evidence(exact, vertex_bytecode)
    replay_evidence = structural_replay_evidence(replay)
    single_rt0 = (
        replay_evidence["renderTargetCount"] == 1
        and [row["register"] for row in replay_evidence["pixelOutputSignature"]] == [0]
    )
    no_engine_samples = not exact["nativeShaderObjectBinding"][
        "textureSampleClosure"
    ]["unownedEngineSamplePairs"]
    preview_candidate = bool(
        single_rt0
        and no_engine_samples
        and texture["sourceExactTextureBindingAdmission"]
        and texture["runtimeDdsParityAdmission"]
        and len(sampler_candidates) == len(bindings)
        and not row_policy["unresolvedSlots"]
        and replay_evidence["structuralFixedInputReplayAdmission"]
    )
    scalar_blocker = (
        SCALAR_PACKING_SOURCE_BLOCKER
        if target_padding_free
        else SCALAR_PACKING_BLOCKER
    )
    structural_blockers = exact["structuralVfPassCandidate"]["admissionBlockers"]
    if vertex_pass is not None:
        structural_blockers = [
            blocker
            for blocker in structural_blockers
            if blocker != "NATIVE_EMITTER_VERTEX_FACTORY_ABI_UNPROVEN"
        ]
        structural_blockers.extend(
            [
                "RAW_EXACT_VERTEX_SHADER_EXECUTION_NOT_ADMITTED",
                "SOURCE_EXACT_VERTEX_CONSTANT_BUFFERS_NOT_CLOSED",
                "PRODUCT_VERTEX_FACTORY_PASS_NOT_ADMITTED",
            ]
        )

    return {
        "variantId": f"ue3.exact-cooked-ps.{key_sha[:24]}",
        "variantKey": key,
        "variantKeySha256": key_sha,
        "familyId": exact["familyId"],
        "parentMaterialPath": exact["parentMaterialPath"],
        "sourceMaterialPath": exact["sourceMaterialPath"],
        "materialInstancePaths": [exact["sourceMaterialPath"]],
        "effectiveStaticParameterSet": exact["mic"][
            "engineEqualityStaticParameterSet"
        ],
        "pixelShader": {
            "shaderType": pixel["shaderType"],
            "shaderIdHex": pixel["shaderIdHex"],
            "profile": exact["nativeShaderObjectBinding"][
                "dxbcDeclarationClosure"
            ]["profile"],
            "byteCount": len(bytecode),
            "sha256": sha256_bytes(bytecode),
            "blobAssetId": f"CookedShaders/{sha256_bytes(bytecode)}.dxbc",
            "repositoryRelativePath": (
                f"Data/Effects/CookedShaders/{sha256_bytes(bytecode)}.dxbc"
            ),
            "exactDxbcContainer": True,
        },
        "structuralVertexFactoryPass": exact["structuralVfPassCandidate"],
        "sourceEmitterVertexFactoryPassEvidence": vertex_pass,
        "nativeBinding": native_binding_evidence(exact),
        "structuralReplay": replay_evidence,
        "sourceValueUniformExpressions": {
            "evaluationContext": evaluation["evaluationContext"],
            "uniformExpressionCounts": uniform["uniformExpressionCounts"],
            "pixelVectorValuesSemanticSha256": evaluation[
                "pixelVectorValuesSemanticSha256"
            ],
            "pixelScalarValuesSemanticSha256": evaluation[
                "pixelScalarValuesSemanticSha256"
            ],
            "evaluationStats": evaluation["evaluationStats"],
            "nativeCb0": native_cb0,
            "nativeScalarGroupPackingEvidence": scalar_packing,
            "sourceValueUniformCb0ClosureAdmission": uniform[
                "sourceValueUniformCb0ClosureAdmission"
            ],
            "sourceExactNativeScalarGroupPackingAdmission": False,
            "sourceValueReplayAdmission": False,
        },
        "textureBindings": {
            "bindings": bindings,
            "bindingCount": len(bindings),
            "sourceExactTextureBindingAdmission": texture[
                "sourceExactTextureBindingAdmission"
            ],
            "fixtureRuntimeDdsParityAdmission": texture[
                "runtimeDdsParityAdmission"
            ],
            "sourceExactSamplerAdmission": False,
            "sourceExactColorSpaceBindingCount": texture[
                "sourceExactColorSpaceBindingCount"
            ],
            "sourceExactFilterSelectorBindingCount": texture[
                "sourceExactFilterSelectorBindingCount"
            ],
            "sourceExactAddressAxisCount": texture["sourceExactAddressAxisCount"],
            "sourceExactAddressAxisDenominator": texture[
                "sourceExactAddressAxisDenominator"
            ],
            "sourceExactHardwareFilterBindingCount": texture[
                "sourceExactHardwareFilterBindingCount"
            ],
            "sourceValueTextureSamplerAdmission": False,
            "blockers": texture["blockers"],
        },
        "authoringPreviewInputs": {
            "fidelity": (
                (
                    "MIXED_SOURCE_EXACT_VECTOR_ROWS_AND_TEXTURE_BINDINGS_PLUS_"
                    "TARGET_PADDING_FREE_CORROBORATED_SCALAR_ROWS_NOT_SOURCE_CLOSED_"
                    "PLUS_PROJECT_ENGINE_ROWS_AND_EXPLICIT_SAMPLER_CANDIDATES_ONLY"
                )
                if target_padding_free
                else (
                    "MIXED_SOURCE_EXACT_VECTOR_ROWS_AND_TEXTURE_BINDINGS_PLUS_"
                    "PROJECT_PREVIEW_APPROXIMATE_SCALAR_PACKING_ENGINE_ROWS_AND_"
                    "EXPLICIT_SAMPLER_CANDIDATES_ONLY"
                )
            ),
            "packingFidelity": {
                "vectorMaterialRows": (
                    "SOURCE_EXACT_MATERIAL_UNIFORM_EXPRESSION_AT_TIME_ZERO"
                ),
                "scalarExpressionValues": "SOURCE_EVALUATED_AT_TIME_ZERO",
                "nativeScalarGroupPackingSourceClosed": False,
                "targetPaddingFreeCorroborationAdmission": target_padding_free,
                "targetPaddingFreeFidelity": scalar_packing["fidelity"],
                "packedLittleEndianSha256": scalar_packing[
                    "packedLittleEndianSha256"
                ],
                "sourceExactPackedCb0": False,
                "blocker": scalar_blocker,
            },
            "engineOrRendererCb0Policy": row_policy,
            "timeZeroCb0Rows": preview_cb0_rows(native_cb0, row_policy),
            "samplers": [
                {
                    "samplerRegister": row["samplerRegister"],
                    **row["authoringPreviewSamplerCandidate"],
                }
                for row in bindings
                if row["authoringPreviewSamplerCandidate"] is not None
            ],
            "samplerCandidatePolicy": (
                "ALL_BINDINGS_HAVE_SOURCE_EXACT_OR_EXPLICIT_UPSTREAM_COMPONENT_CANDIDATES"
                if len(sampler_candidates) == len(bindings)
                else "NO_COMPLETE_EXPLICIT_PREVIEW_SAMPLER_CANDIDATE"
            ),
            "sourceExactRuntimeReplay": False,
        },
        "admission": {
            "exactPixelShaderBlob": True,
            "exactNativeBindingWire": True,
            "structuralFixedInputReplay": True,
            "sourceValueUniformCb0Closure": True,
            "sourceExactNativeScalarGroupPacking": False,
            "sourceExactTextureBindings": True,
            "sourceExactColorSpace": bool(
                texture["sourceExactColorSpaceBindingCount"]
                == len(texture["uniformTextureBindings"])
            ),
            "sourceExactFilterSelector": bool(
                texture["sourceExactFilterSelectorBindingCount"]
                == len(texture["uniformTextureBindings"])
            ),
            "sourceExactSampler": False,
            "sourceValueReplay": False,
            "actualVfPass": False,
            "authoringPreviewCandidate": preview_candidate,
            "authoringPreviewAdmission": False,
            "productRuntime": False,
            "visual": False,
        },
        "openBlockers": sorted(
            set(
                texture["blockers"]
                + structural_blockers
                + [
                    scalar_blocker,
                    "ENGINE_OR_RENDERER_CB_ROWS_NOT_SOURCE_EXACT",
                    "SOURCE_VALUE_REPLAY_NOT_CLOSED",
                    "PRODUCT_RUNTIME_NOT_ADMITTED",
                    "USER_VISUAL_ADMISSION_REQUIRED",
                ]
            )
        ),
    }


def iter_keys(value: Any) -> Iterable[str]:
    if isinstance(value, dict):
        for key, child in value.items():
            yield key
            yield from iter_keys(child)
    elif isinstance(value, list):
        for child in value:
            yield from iter_keys(child)


def validate_contract(contract: dict[str, Any], blobs: dict[str, bytes]) -> None:
    require(contract.get("schema") == SCHEMA, "contract schema changed")
    require(contract.get("formatVersion") == FORMAT_VERSION, "contract format changed")
    forbidden = FORBIDDEN_SELECTOR_KEYS.intersection(iter_keys(contract))
    require(not forbidden, f"selector keys leaked into contract: {sorted(forbidden)}")
    variants = contract.get("variants")
    require(isinstance(variants, list) and variants, "contract has no variants")
    require(
        variants == sorted(variants, key=lambda row: row["variantId"]),
        "variants are not deterministically sorted",
    )
    ids = [row["variantId"] for row in variants]
    keys = [row["variantKeySha256"] for row in variants]
    families = [row["familyId"] for row in variants]
    require(len(ids) == len(set(ids)), "variant ID is duplicated")
    require(len(keys) == len(set(keys)), "variant key is duplicated")
    require(len(families) == len(set(families)), "family variant is duplicated")
    summary = contract.get("summary")
    if summary is not None:
        require(summary["variantCount"] == 5, "five-family summary denominator changed")
        require(
            summary["uniquePixelShaderBlobCount"] == 5,
            "five-family pixel blob summary changed",
        )
        require(
            summary["exactVertexShaderSidecarCount"] == 1,
            "vertex sidecar summary changed",
        )
        require(
            summary["sourceExactSamplerAdmissionCount"] == 0,
            "sampler exactness summary was admitted",
        )
        require(
            summary["sourceExactNativeScalarGroupPackingAdmissionCount"] == 0,
            "scalar packing exactness summary was admitted",
        )
        require(
            summary["rawVertexShaderExecutionAdmissionCount"] == 0,
            "raw vertex execution summary was admitted",
        )
        require(
            summary["productRuntimeAdmissionCount"] == 0
            and summary["visualAdmissionCount"] == 0,
            "Product or visual summary was admitted",
        )
    for row in variants:
        require(
            row["variantKeySha256"] == canonical_json_sha256(row["variantKey"]),
            f"variant key seal changed: {row['variantId']}",
        )
        pixel = row["pixelShader"]
        require(pixel["sha256"] in blobs, "variant blob bytes are absent")
        blob = blobs[pixel["sha256"]]
        require(len(blob) == pixel["byteCount"], "variant blob byte count changed")
        require(sha256_bytes(blob) == pixel["sha256"], "variant blob SHA changed")
        require(pixel["repositoryRelativePath"].endswith(f"/{pixel['sha256']}.dxbc"), "blob path is not content addressed")
        vertex_pass = row["sourceEmitterVertexFactoryPassEvidence"]
        if vertex_pass is not None:
            vertex = vertex_pass["vertexShader"]
            require(vertex["sha256"] in blobs, "vertex sidecar blob bytes are absent")
            vertex_blob = blobs[vertex["sha256"]]
            require(
                len(vertex_blob) == vertex["byteCount"],
                "vertex sidecar byte count changed",
            )
            require(
                sha256_bytes(vertex_blob) == vertex["sha256"],
                "vertex sidecar SHA changed",
            )
            require(
                vertex["repositoryRelativePath"].endswith(
                    f"/{vertex['sha256']}.dxbc"
                ),
                "vertex sidecar path is not content addressed",
            )
            vertex_admission = vertex_pass["admission"]
            require(
                vertex_admission["sourceEmitterVertexFactorySelection"],
                "source emitter VF evidence is absent",
            )
            require(
                vertex_admission["exactVertexPixelSignatureClosure"],
                "VS/PS signature evidence is absent",
            )
            require(
                vertex_admission["authoringNoDensityPass"],
                "bounded NoDensity evidence is absent",
            )
            require(
                not vertex_admission["rawVertexShaderExecution"],
                "raw vertex execution was admitted",
            )
            require(
                not vertex_admission["sourceExactVertexCb"],
                "source-exact vertex CB was admitted",
            )
            require(
                not vertex_admission["productVfPass"],
                "Product vertex pass was admitted",
            )
        admission = row["admission"]
        require(not admission["authoringPreviewAdmission"], "preview was admitted")
        require(not admission["productRuntime"], "Product runtime was admitted")
        require(not admission["visual"], "visual fidelity was admitted")
        require(not admission["actualVfPass"], "actual VF/pass was admitted")
        require(not admission["sourceExactSampler"], "sampler was admitted")
        require(
            not admission["sourceExactNativeScalarGroupPacking"],
            "native scalar-group packing was admitted",
        )
        preview = row["authoringPreviewInputs"]
        require(
            not preview["packingFidelity"]["sourceExactPackedCb0"],
            "preview CB0 was mislabeled source exact",
        )
        require(
            not row["textureBindings"]["sourceExactSamplerAdmission"],
            "full sampler exactness was admitted",
        )
        packing = row["sourceValueUniformExpressions"][
            "nativeScalarGroupPackingEvidence"
        ]
        require(
            packing["nativeScalarGroupPackingSourceClosed"] is False,
            "global scalar packing source closure was admitted",
        )
        require(
            packing["sourceExactAdmission"] is False,
            "source-exact scalar packing was admitted",
        )
        if packing["targetPaddingFreeCorroborationAdmission"]:
            require(
                SCALAR_PACKING_SOURCE_BLOCKER in row["openBlockers"],
                "scalar packing source-closure blocker is absent",
            )
        else:
            require(
                SCALAR_PACKING_BLOCKER in row["openBlockers"],
                "native scalar-group packing blocker is absent",
            )
        for binding in row["textureBindings"]["bindings"]:
            partial = binding["sourceExactSamplerPartialEvidence"]
            require(
                partial["fullSamplerSourceExact"] is False,
                "partial sampler evidence was mislabeled full exact",
            )
            candidate = binding["authoringPreviewSamplerCandidate"]
            if candidate is not None:
                require(
                    all(
                        component["sourceExact"]
                        or component["provenance"]
                        == "EXPLICIT_UPSTREAM_PREVIEW_CANDIDATE"
                        for component in candidate["components"].values()
                    ),
                    "preview sampler inferred an implicit candidate",
                )
        for cb0_row in preview["timeZeroCb0Rows"]:
            if cb0_row.get("source") == "PIXEL_SCALAR_EXPRESSION_GROUP":
                expected_fidelity = (
                    TARGET_PADDING_FREE_SCALAR_FIDELITY
                    if packing["targetPaddingFreeCorroborationAdmission"]
                    else "PROJECT_PREVIEW_APPROXIMATE_SCALAR_GROUP_PACKING_UNPROVEN"
                )
                require(cb0_row["fidelity"] == expected_fidelity, "scalar row fidelity changed")
    claimed = contract.get("contractSha256")
    unsigned = dict(contract)
    unsigned.pop("contractSha256", None)
    require(claimed == canonical_json_sha256(unsigned), "contract seal changed")


def build_contract(
    exact_path: Path,
    replay_path: Path,
    uniform_path: Path,
    texture_path: Path,
    cache_path: Path,
) -> tuple[dict[str, Any], dict[str, bytes]]:
    exact = validate_sealed_receipt(exact_path, EXACT_SCHEMA, 3)
    replay = validate_sealed_receipt(replay_path, REPLAY_SCHEMA, 1)
    uniform = validate_sealed_receipt(uniform_path, UNIFORM_SCHEMA, 1)
    texture = validate_sealed_receipt(texture_path, TEXTURE_SCHEMA, 1)
    require(
        uniform["scope"]["nativeScalarGroupPackingSourceClosed"] is False,
        "global native scalar-group source closure was admitted",
    )
    require(
        uniform["scope"]["targetPaddingFreePackedScalarEvidence"] is True,
        "target-padding-free scalar evidence scope is absent",
    )
    require(
        texture["summary"]["sourceExactSamplerTargetCount"] == 0,
        "upstream full sampler exactness denominator changed",
    )
    require(
        exact["summary"]["rawVertexShaderExecutionCount"] == 0
        and exact["summary"]["productVfPassCount"] == 0,
        "upstream raw vertex or Product pass was admitted",
    )

    exact_rows = family_index(
        [
            row
            for row in exact["targets"]
            if row.get("status") == EXACT_STATUS
            and row.get("nativeShaderObjectBinding", {}).get("status")
            == NATIVE_STATUS
        ]
    )
    replay_rows = family_index(replay["targetReplays"])
    uniform_rows = family_index(uniform["targets"], status=UNIFORM_STATUS)
    texture_rows = family_index(texture["targets"], status=TEXTURE_STATUS)
    family_set = set(exact_rows)
    require(family_set == set(replay_rows), "structural replay family denominator changed")
    require(family_set == set(uniform_rows), "uniform family denominator changed")
    require(family_set == set(texture_rows), "texture family denominator changed")
    require(len(family_set) == 5, "exact five-family denominator changed")

    cache_package = exact["officialRefShaderCache"]["package"]
    require(cache_path.is_file(), f"pinned cache is missing: {cache_path}")
    require(
        cache_path.stat().st_size == cache_package["physicalByteSize"],
        "pinned cache byte count changed",
    )
    require(
        sha256_file(cache_path) == cache_package["rawSha256"],
        "pinned cache SHA changed",
    )
    cache = package_tables(cache_path)

    blobs: dict[str, bytes] = {}
    variants = []
    pixel_blob_digests: set[str] = set()
    vertex_blob_digests: set[str] = set()
    for family_id in sorted(family_set):
        exact_row = exact_rows[family_id]
        bytecode = rehydrate_dxbc(cache, exact_row)
        digest = sha256_bytes(bytecode)
        require(digest not in blobs, "exact variants unexpectedly share a DXBC blob")
        blobs[digest] = bytecode
        pixel_blob_digests.add(digest)
        vertex_bytecode = None
        source_emitter = exact_row.get("sourceEmitterVertexFactoryPass")
        if source_emitter is not None:
            vertex_bytecode = rehydrate_dxbc(
                cache,
                {
                    "targetId": family_id,
                    "cookedPixelShader": source_emitter["exactVertexShader"],
                },
            )
            vertex_digest = sha256_bytes(vertex_bytecode)
            if vertex_digest in blobs:
                require(
                    blobs[vertex_digest] == vertex_bytecode,
                    "content-addressed vertex shader collision",
                )
            else:
                blobs[vertex_digest] = vertex_bytecode
            vertex_blob_digests.add(vertex_digest)
        variants.append(
            build_variant(
                exact_row,
                replay_rows[family_id],
                uniform_rows[family_id],
                texture_rows[family_id],
                bytecode,
                vertex_bytecode,
            )
        )
    variants.sort(key=lambda row: row["variantId"])
    require(len(pixel_blob_digests) == 5, "five-family pixel blob denominator changed")
    require(
        len(vertex_blob_digests)
        == exact["summary"]["exactVertexShaderBlobCount"]
        == 1,
        "exact vertex shader sidecar denominator changed",
    )

    contract: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": {
            "scope": "CLASS_NEUTRAL_UE3_MATERIAL_FAMILY_PERMUTATION_PIXEL_SHADER_VARIANTS",
            "selectionUnit": (
                "BASE_MATERIAL_PLUS_EFFECTIVE_STATIC_PARAMETER_SET_PLUS_PLATFORM_"
                "PLUS_STRUCTURAL_VF_CANDIDATE_SET_PLUS_PASS_SHADER_IDENTITY"
            ),
            "runtimeSelectorsExcluded": sorted(FORBIDDEN_SELECTOR_KEYS),
        },
        "generator": {
            "repositoryRelativePath": repository_relative(SCRIPT_PATH),
            "sha256": sha256_file(SCRIPT_PATH),
        },
        "inputs": {
            "exactMaterialMaps": receipt_descriptor(exact_path, exact),
            "structuralFixedInputReplay": receipt_descriptor(replay_path, replay),
            "sourceValueUniformEvaluation": receipt_descriptor(uniform_path, uniform),
            "exactTextureSamplerClosure": receipt_descriptor(texture_path, texture),
            "pinnedOfficialRefShaderCache": {
                "fileName": cache_package["fileName"],
                "physicalByteSize": cache_package["physicalByteSize"],
                "rawSha256": cache_package["rawSha256"],
                "packageGuidHex": cache_package["packageGuidHex"],
                "shaderPlatformOrdinal": exact["officialRefShaderCache"][
                    "shaderCodeLayout"
                ]["platform"],
            },
        },
        "policy": {
            "blobStorage": "CONTENT_ADDRESSED_FULL_SHA256_DXBC",
            "unknownVariant": "FAIL_CLOSED_TO_EXISTING_FAMILY_LITE_PATH",
            "samplerCandidateFidelity": PREVIEW_FIDELITY,
            "sourceExactSampler": False,
            "nativeScalarGroupPackingSourceClosed": False,
            "sourceExactNativeScalarGroupPacking": False,
            "rawVertexShaderExecution": False,
            "productRuntime": False,
            "authoringPreviewCandidateIsAdmission": False,
            "publishRuntimeDataFiles": False,
        },
        "variants": variants,
        "summary": {
            "variantCount": len(variants),
            "uniqueDxbcBlobCount": len(blobs),
            "uniquePixelShaderBlobCount": len(pixel_blob_digests),
            "exactVertexShaderSidecarCount": sum(
                row["sourceEmitterVertexFactoryPassEvidence"] is not None
                for row in variants
            ),
            "uniqueVertexShaderBlobCount": len(vertex_blob_digests),
            "authoringPreviewCandidateCount": sum(
                row["admission"]["authoringPreviewCandidate"] for row in variants
            ),
            "sourceExactSamplerAdmissionCount": 0,
            "targetPaddingFreeScalarCorroborationCount": sum(
                row["sourceValueUniformExpressions"][
                    "nativeScalarGroupPackingEvidence"
                ]["targetPaddingFreeCorroborationAdmission"]
                for row in variants
            ),
            "sourceExactNativeScalarGroupPackingAdmissionCount": 0,
            "sourceEmitterVertexFactorySelectionEvidenceCount": sum(
                row["sourceEmitterVertexFactoryPassEvidence"] is not None
                for row in variants
            ),
            "exactVertexPixelSignatureClosureCount": sum(
                row["sourceEmitterVertexFactoryPassEvidence"] is not None
                and row["sourceEmitterVertexFactoryPassEvidence"]["admission"][
                    "exactVertexPixelSignatureClosure"
                ]
                for row in variants
            ),
            "authoringNoDensityPassEvidenceCount": sum(
                row["sourceEmitterVertexFactoryPassEvidence"] is not None
                and row["sourceEmitterVertexFactoryPassEvidence"]["admission"][
                    "authoringNoDensityPass"
                ]
                for row in variants
            ),
            "rawVertexShaderExecutionAdmissionCount": 0,
            "actualVfPassAdmissionCount": 0,
            "productRuntimeAdmissionCount": 0,
            "visualAdmissionCount": 0,
            "upstreamBlockedFamilyCount": exact["summary"][
                "blockedNoEffectiveStaticSetAbiEvidenceCount"
            ],
            "result": "PASS_G03_6_SOURCE_ABI_EVIDENCE_MATERIALIZED_PRODUCT_CLOSED",
        },
    }
    contract["contractSha256"] = canonical_json_sha256(contract)
    validate_contract(contract, blobs)
    return contract, blobs


def check_or_write(
    output: Path,
    blob_root: Path,
    contract: dict[str, Any],
    blobs: dict[str, bytes],
    check: bool,
) -> None:
    if check:
        require(output.is_file(), f"contract is missing: {output}")
        require(read_json(output) == contract, f"contract is stale: {output}")
        for digest, bytecode in blobs.items():
            path = blob_root / f"{digest}.dxbc"
            require(path.is_file(), f"DXBC blob is missing: {path}")
            require(path.read_bytes() == bytecode, f"DXBC blob is stale: {path}")
        return
    for digest, bytecode in blobs.items():
        write_blob_atomic(blob_root / f"{digest}.dxbc", bytecode)
    write_json_atomic(output, contract)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exact", type=Path, default=DEFAULT_EXACT)
    parser.add_argument("--replay", type=Path, default=DEFAULT_REPLAY)
    parser.add_argument("--uniform", type=Path, default=DEFAULT_UNIFORM)
    parser.add_argument("--texture", type=Path, default=DEFAULT_TEXTURE)
    parser.add_argument("--cache", type=Path, default=DEFAULT_CACHE)
    parser.add_argument("--blob-root", type=Path, default=DEFAULT_BLOB_ROOT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    arguments = parser.parse_args(argv)

    contract, blobs = build_contract(
        arguments.exact,
        arguments.replay,
        arguments.uniform,
        arguments.texture,
        arguments.cache,
    )
    check_or_write(
        arguments.output,
        arguments.blob_root,
        contract,
        blobs,
        arguments.check,
    )
    print(
        "UE3 exact cooked variants: "
        f"variants={contract['summary']['variantCount']} "
        f"blobs={contract['summary']['uniqueDxbcBlobCount']} "
        f"previewCandidates={contract['summary']['authoringPreviewCandidateCount']} "
        f"runtime={contract['summary']['productRuntimeAdmissionCount']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
