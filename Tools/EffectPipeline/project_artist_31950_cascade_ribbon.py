#!/usr/bin/env python3
"""Project one Artist 31950 Ribbon onto its typed carrier and material family.

The child MIC has no native static tail.  Its parent-default shader map does,
however, own a unique BasePass binding wire and three exact Texture2D defaults.
This projection therefore installs one bounded, class-neutral semantic replay;
it never labels the parent shader as child-native DXBC and keeps the unresolved
sampler defaults, reflection basis, and scene-color distortion explicit.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
DOCUMENT_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Authored/effect.artist.skill.31950.unified.effect.json"
)
MATERIAL_RECEIPT_PATH = REPOSITORY_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31950.ribbon-parent-default-shader.receipt.json"
)
OUTPUT_RECEIPT_PATH = REPOSITORY_ROOT / (
    "Data/Effects/AuthoredCorrections/Artist/"
    "effect.artist.skill.31950.cascade-ribbon-projection.receipt.json"
)
EFFECT_ASSET_ID = "effect.artist.skill.31950.unified"
TARGET_ELEMENT_ID = "authored.source-particle.29868adeb040d5a35e2f213c"
TARGET_SOURCE_NODE = (
    "authored-source-particle:effect.artist.skill.31950.unified|"
    "source:effect.artist.skill.31950.imported|"
    "element:fx_pc_sdm_01.par_t_sdm_dragonrising_00_wpcast_01."
    "particlespriteemitter_2"
)
TYPE_DATA_STABLE_ID = "FX_PC_SDM_01:export:1495@ref:6"
EXPECTED_ELEMENT_COUNT = 23
BASELINE_TARGET_SHA256 = (
    "8a234ff98a1006a7a232b98c477b2210b881efb51c24c8a62b4eda6dba30a3a8"
)
NON_TARGET_ELEMENTS_SHA256 = (
    "5df55dc64c788eaf2cbe750d3eaecc513f3e8f80b2e8bb341edb1d720fd058d9"
)
RUNTIME_MATERIAL_OPCODE = 20
NORMAL_ASSET_ID = "Effect/Artist/Textures/fx_d_normal_085.dds"
FLUID_ASSET_ID = "Effect/Artist/Textures/fx_a_fluid_003.dds"
AURA_ASSET_ID = "Effect/Artist/Textures/fx_k_auraline_14_ycl.dds"
EXPECTED_TEXTURES = {
    "fx_tex_01.fx_d_normal_085": (
        NORMAL_ASSET_ID,
        "617414c6ff317f0cd70723ea8ff1d10764c6b1a438b343b4b82d3d5d80dd5703",
    ),
    "fx_tex_00.fx_a_fluid_003": (
        FLUID_ASSET_ID,
        "0f8b781ccf049e7e2a034fea32d36123c2f5ec4eba74e224fdce2ee490398ab0",
    ),
    "fx_tex_05.fx_k_auraline_14_ycl": (
        AURA_ASSET_ID,
        "1d73b6debce1f3e44bb33a4c455466b06cb0bf7a04b41358fce01405f6573792",
    ),
}
PACKED_SCALARS = (
    ("normal_strength", 0.5),
    ("alpha_strength", 2.0),
    ("reflection_uv_scale", 3.0),
    ("distortion_strength", 50.0),
    ("normal_uv_scale_x", 1.0),
    ("normal_uv_scale_y", 1.0),
    ("alpha_uv_scale_x", 1.0),
    ("alpha_uv_scale_y", 1.0),
    ("normal_pan_x", 0.0),
    ("normal_pan_y", 0.0),
    ("alpha_pan_x", 0.0),
    ("alpha_pan_y", 0.0),
)
BASELINE_SOURCE_PROFILE = {
    "enabled": True,
    "profileId": (
        "ue3.material.fx.m.mi.00.fx.m.fx.d.pa.ribbonliquid.01.tr."
        "6d3d630289cd"
    ),
    "runtimeShaderProfileId": "effect.ue3.fallback-blocked.v1",
    "parentMaterialPath": "fx_m_mi_00.fx_m.fx_d_pa_ribbonliquid_01_tr",
    "semanticStatus": "reconstructed_profile",
    "textures": [],
    "scalars": [
        {"name": "02.normalmapstr", "value": 0.5, "group": ""},
        {"name": "99.alphastr", "value": 2.0, "group": ""},
    ],
    "vectors": [
        {
            "name": "10.reflectcolor",
            "value": [1.0, 1.0, 3.0, 50.0],
            "group": "",
        }
    ],
    "staticSwitches": [],
    "dynamicParameterSemantics": [
        "unbound",
        "dissolve",
        "distortion",
        "uv_pan",
    ],
    "subUVMode": "none",
}


class Artist31950RibbonProjectionError(ValueError):
    """Raised when selective projection can no longer prove its boundary."""


def require(condition: bool, message: str) -> None:
    if not condition:
        raise Artist31950RibbonProjectionError(message)


def read_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8-sig"))
    require(isinstance(value, dict), f"expected JSON object: {path}")
    return value


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(
        json.dumps(
            value,
            ensure_ascii=False,
            allow_nan=False,
            sort_keys=True,
            separators=(",", ":"),
        ).encode("utf-8")
    ).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        while payload := stream.read(1024 * 1024):
            digest.update(payload)
    return digest.hexdigest()


def output_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, allow_nan=False, indent=2) + "\n"
    ).encode("utf-8")


def replace_target_row_bytes(
    raw_document: bytes, projected_target: dict[str, Any]
) -> bytes:
    """Replace only the selected top-level element and preserve every other byte."""
    had_bom = raw_document.startswith(b"\xef\xbb\xbf")
    text = raw_document.decode("utf-8-sig")
    needle = f'"id": "{TARGET_ELEMENT_ID}"'
    identity_offset = text.find(needle)
    require(identity_offset >= 0, "Artist T raw target identity is missing")
    object_marker = text.rfind("\n    {", 0, identity_offset)
    require(object_marker >= 0, "Artist T raw target object start is missing")
    line_start = object_marker + 1
    object_start = object_marker + 5
    depth = 0
    in_string = False
    escaped = False
    end = -1
    for offset in range(object_start, len(text)):
        character = text[offset]
        if in_string:
            if escaped:
                escaped = False
            elif character == "\\":
                escaped = True
            elif character == '"':
                in_string = False
            continue
        if character == '"':
            in_string = True
        elif character == "{":
            depth += 1
        elif character == "}":
            depth -= 1
            if depth == 0:
                end = offset + 1
                break
    require(end > object_start, "Artist T raw target object end is missing")
    rendered = json.dumps(
        projected_target,
        ensure_ascii=False,
        allow_nan=False,
        indent=2,
    )
    replacement = "\n".join("    " + line for line in rendered.splitlines())
    output = (text[:line_start] + replacement + text[end:]).encode("utf-8")
    return (b"\xef\xbb\xbf" + output) if had_bom else output


def target_row(document: dict[str, Any]) -> dict[str, Any]:
    elements = document.get("elements")
    require(
        isinstance(elements, list) and len(elements) == EXPECTED_ELEMENT_COUNT,
        "Artist T element denominator changed",
    )
    matches = [
        row
        for row in elements
        if isinstance(row, dict) and row.get("id") == TARGET_ELEMENT_ID
    ]
    require(len(matches) == 1, "Artist T Ribbon target join changed")
    return matches[0]


def validate_boundary(document: dict[str, Any]) -> dict[str, Any]:
    require(
        document.get("schema") == "lostark.effect-authoring"
        and document.get("version") == 13
        and document.get("effectAssetId") == EFFECT_ASSET_ID,
        "Artist T document identity changed",
    )
    target = target_row(document)
    others = [row for row in document["elements"] if row is not target]
    require(
        canonical_sha256(others) == NON_TARGET_ELEMENTS_SHA256,
        "Artist T non-target rows changed; selective projection refuses overwrite",
    )
    require(
        target.get("sourceNode") == TARGET_SOURCE_NODE,
        "Artist T Ribbon source node changed",
    )
    require(
        target.get("material", {}).get("sourceMaterialPath")
        == "fx_m_mi_d_00.fx_mi.fx_d_pa_ribbonliquid_01_101_tr",
        "Artist T child MIC identity changed",
    )
    modules = target.get("sourceRecipe", {}).get("modules")
    require(isinstance(modules, list), "Artist T source modules are missing")
    typed = [
        module
        for module in modules
        if isinstance(module, dict)
        and str(module.get("className", "")).casefold()
        == "particlemoduletypedataribbon"
    ]
    require(
        len(typed) == 1 and typed[0].get("stableId") == TYPE_DATA_STABLE_ID,
        "Artist T TypeDataRibbon identity changed",
    )
    literals = {
        item.get("propertyPath"): item.get("value")
        for item in typed[0].get("literals", [])
        if isinstance(item, dict) and item.get("kind") == "number"
    }
    require(
        literals.get("tilingdistance") == 300.0
        and literals.get("distancetessellationstepsize") == 5.0,
        "Artist T TypeDataRibbon distance literals changed",
    )
    return target


def sampler(address_v: str) -> dict[str, Any]:
    return {
        "filter": "linear",
        "addressU": "wrap",
        "addressV": address_v,
        "addressW": "wrap",
        "mipLodBias": 0.0,
        "maxAnisotropy": 1,
        "comparison": "never",
        "borderColor": [0.0, 0.0, 0.0, 0.0],
        "minLod": 0.0,
        "maxLod": 3.40282347e38,
    }


def projected_resources() -> list[dict[str, str]]:
    return [
        {"slotId": "base", "assetId": NORMAL_ASSET_ID},
        {"slotId": "noise", "assetId": FLUID_ASSET_ID},
        {"slotId": "emissive", "assetId": AURA_ASSET_ID},
    ]


def projected_execution() -> dict[str, Any]:
    lanes = [
        ("distortion_normal", NORMAL_ASSET_ID, "RG", "linear", "wrap"),
        ("surface_normal", NORMAL_ASSET_ID, "RG", "linear", "wrap"),
        ("alpha_aura", AURA_ASSET_ID, "RGB", "srgb", "clamp"),
        ("reflection_fluid", FLUID_ASSET_ID, "RGB", "srgb", "wrap"),
    ]
    return {
        "enabled": True,
        "version": 1,
        "backend": "runtimeMaterialV2",
        "opcode": RUNTIME_MATERIAL_OPCODE,
        "passIndex": 1,
        "renderState": {
            "rasterizer": "RS_Cull_None",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAlpha",
            "stencilReference": 0,
        },
        "textureLaneCount": 4,
        "textureMask": 15,
        "textureLanes": [
            {
                "laneId": f"lane.{index}",
                "role": role,
                "assetId": asset_id,
                "textureRegister": index,
                "samplerRegister": 5 + index,
                "sourceChannel": channel,
                "colorSpace": color_space,
                "sampler": sampler(address_v),
            }
            for index, (role, asset_id, channel, color_space, address_v)
            in enumerate(lanes)
        ],
        "dynamicConsumedMask": 15,
        "dynamicSuppressedMask": 0,
        "particleColorPolicy": 2,
        "particleColorConsumedMask": 8,
        "particleColorSuppressedMask": 7,
        "scalarCount": len(PACKED_SCALARS),
        "vectorCount": 1,
        "inputCount": 17,
        # Four texture lanes precede the twelve packed scalars.  The fourth
        # scalar is 02.distortionstr and belongs to the deferred native
        # DistortionAccumulate pass, so BasePass replay must not claim it.
        "inputConsumedMask": [0x1FF7F, 0],
        "inputSuppressedMask": [0x80, 0],
        "vectorComponentConsumedMask": [15, 0, 0],
        "vectorComponentSuppressedMask": [0, 0, 0],
        "staticInputCount": 0,
        "staticSelectedMask": 0,
        "staticConsumedMask": 0,
        "staticSuppressedMask": 0,
        "renderInputCount": 6,
        "renderConsumedMask": 47,
        "renderSuppressedMask": 16,
        "scalars": [
            {"name": name, "packedIndex": index, "value": value}
            for index, (name, value) in enumerate(PACKED_SCALARS)
        ],
        "vectors": [
            {
                "name": "reflect_color_and_intensity",
                "packedIndex": 0,
                "value": [1.0, 1.0, 3.0, 50.0],
            }
        ],
        "artistParameters": [],
        "colors": [],
    }


def project_document(document: dict[str, Any]) -> dict[str, Any]:
    staged = copy.deepcopy(document)
    source = validate_boundary(document)
    target = target_row(staged)
    target["visible"] = True
    target["kind"] = "trail"
    target["sourceRecipe"]["rendererShape"] = "ribbon"
    target["detail"]["trail"]["tilingDistanceWorldUnits"] = 3.0
    target["detail"]["trail"]["distanceTessellationStepWorldUnits"] = 0.05
    target["resources"] = projected_resources()
    target["material"]["templateId"] = "effect.standard"
    target["material"]["sourceProfile"] = {"enabled": False}
    target["material"]["execution"] = projected_execution()
    return staged


def validate_material_receipt(receipt: dict[str, Any]) -> None:
    require(
        receipt.get("schema")
        == "lostark.effect-ue3-parent-default-ribbon-material-receipt"
        and receipt.get("formatVersion") == 1
        and receipt.get("occurrenceId") == TARGET_ELEMENT_ID,
        "Artist T Ribbon material receipt identity changed",
    )
    require(
        receipt.get("childMic", {}).get("childNativeDxbc") is False
        and receipt.get("scope", {}).get("runtimeMaterialAdmission") is False
        and receipt.get("runtimeAdmission", {}).get("admitted") is False,
        "Artist T material receipt overclaims native runtime or child DXBC",
    )
    passes = receipt.get("selectedRenderer", {}).get("passes")
    require(
        isinstance(passes, list)
        and [row.get("role") for row in passes]
        == ["BASE_TRANSLUCENT", "DISTORTION_ACCUMULATE"]
        and all(
            row.get("nativeBindingCandidateCount") == 1
            and row.get("nativeBindingStatus")
            == "EXACT_UNIQUE_NATIVE_BINDING_ARRAY"
            for row in passes
        ),
        "Artist T parent-default native binding closure changed",
    )
    textures = receipt.get("textureLanes")
    require(isinstance(textures, list) and len(textures) == 3,
            "Artist T texture denominator changed")
    by_path = {row.get("sourceObjectPath"): row for row in textures}
    require(set(by_path) == set(EXPECTED_TEXTURES),
            "Artist T source texture identity changed")
    for source_path, (asset_id, digest) in EXPECTED_TEXTURES.items():
        row = by_path[source_path]
        require(
            row.get("runtimeAssetId") == asset_id
            and row.get("expectedRuntimeDdsSha256") == digest
            and row.get("runtimeDeployment", {}).get("status")
            == "PRESENT_EXACT"
            and row.get("runtimeDeployment", {}).get("sha256") == digest,
            f"Artist T runtime DDS is not exact: {source_path}",
        )
    unsigned = copy.deepcopy(receipt)
    seal = unsigned.pop("receiptSha256", None)
    require(seal == canonical_sha256(unsigned), "material receipt seal changed")


def build_projection_receipt(
    baseline: dict[str, Any], projected: dict[str, Any]
) -> dict[str, Any]:
    source_target = target_row(baseline)
    projected_target = target_row(projected)
    material_receipt = read_json(MATERIAL_RECEIPT_PATH)
    validate_material_receipt(material_receipt)
    receipt = {
        "schema": "lostark.effect-artist-31950-cascade-ribbon-projection-receipt",
        "formatVersion": 1,
        "effectAssetId": EFFECT_ASSET_ID,
        "targetElementId": TARGET_ELEMENT_ID,
        "scope": {
            "selectiveElementCount": 1,
            "nonTargetElementCount": EXPECTED_ELEMENT_COUNT - 1,
            "productBulkRegeneration": False,
            "carrierAdmission": "EFFECT_TYPED_CASCADE_RIBBON_V1",
            "materialAdmission": True,
            "materialFidelity": "BOUNDED_PARENT_DEFAULT_SEMANTIC_REPLAY_PARTIAL",
            "nativeExecution": False,
            "visualFidelity": "USER_PENDING",
        },
        "input": {
            "baselineTargetCanonicalSha256": BASELINE_TARGET_SHA256,
            "nonTargetElementsCanonicalSha256": NON_TARGET_ELEMENTS_SHA256,
            "materialEvidencePath": MATERIAL_RECEIPT_PATH.relative_to(
                REPOSITORY_ROOT
            ).as_posix(),
            "materialEvidenceRawSha256": sha256_file(MATERIAL_RECEIPT_PATH),
            "materialEvidenceSeal": material_receipt["receiptSha256"],
        },
        "projection": {
            "beforeTargetCanonicalSha256": canonical_sha256(source_target),
            "afterTargetCanonicalSha256": canonical_sha256(projected_target),
            "afterDocumentCanonicalSha256": canonical_sha256(projected),
            "changes": [
                {"path": "visible", "before": False, "after": True},
                {"path": "kind", "before": "particle", "after": "trail"},
                {
                    "path": "sourceRecipe.rendererShape",
                    "before": "sprite",
                    "after": "ribbon",
                },
                {
                    "path": "detail.trail.tilingDistanceWorldUnits",
                    "before": "OMITTED_DEFAULT_ZERO",
                    "after": 3.0,
                    "basis": "TypeDataRibbon.tilingdistance 300 UE3 cm",
                },
                {
                    "path": "detail.trail.distanceTessellationStepWorldUnits",
                    "before": "OMITTED_DEFAULT_ZERO",
                    "after": 0.05,
                    "basis": "TypeDataRibbon.distancetessellationstepsize 5 UE3 cm",
                },
                {
                    "path": "resources",
                    "before": [],
                    "after": projected_resources(),
                    "basis": "exact parent ReferencedTextures and verified runtime DDS SHA",
                },
                {
                    "path": "material.templateId",
                    "before": "effect.source_material",
                    "after": "effect.standard",
                    "basis": "typed RuntimeMaterialV2 carrier bypasses fallback-blocked source evaluator",
                },
                {
                    "path": "material.sourceProfile",
                    "beforeCanonicalSha256": canonical_sha256(
                        source_target["material"]["sourceProfile"]
                    ),
                    "after": {"enabled": False},
                    "basis": "parent/default/child evidence remains sealed in material receipt",
                },
                {
                    "path": "material.execution",
                    "before": {"enabled": False, "failClosed": True},
                    "after": projected_execution(),
                    "basis": "parent-default BasePass shader map, unique native wires, child numeric overrides",
                },
            ],
            "preserved": {
                "sourceMaterialPath": source_target["material"]["sourceMaterialPath"],
                "sourceProfileCanonicalSha256": canonical_sha256(
                    source_target["material"]["sourceProfile"]
                ),
                "sourceRecipeModulesCanonicalSha256": canonical_sha256(
                    source_target["sourceRecipe"]["modules"]
                ),
            },
        },
        "limitations": [
            "CHILD_MIC_HAS_NO_NATIVE_STATIC_TAIL_PARENT_DEFAULT_ONLY",
            "SAMPLER_FILTER_ADDRESS_DEFAULTS_BOUNDED_NOT_SOURCE_REVISION_CDO_EXACT",
            "FLUID_AND_AURA_SRGB_DEFAULTS_BOUNDED_NOT_SERIALIZED_EXPLICIT",
            "PARENT_REFLECTION_BASIS_RECONSTRUCTED_ON_TYPED_RIBBON_CARRIER",
            "DISTORTION_SCENE_COLOR_ACCUMULATE_PASS_DEFERRED",
        ],
        "result": "CARRIER_AND_BOUNDED_PARENT_DEFAULT_MATERIAL_PROJECTED_USER_VISUAL_PENDING",
    }
    receipt["receiptSha256"] = canonical_sha256(receipt)
    return receipt


def write_atomic(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_bytes(payload)
    temporary.replace(path)


def run(
    write: bool = False,
    document_path: Path = DOCUMENT_PATH,
    output_receipt_path: Path = OUTPUT_RECEIPT_PATH,
) -> bool:
    current_raw = document_path.read_bytes()
    current = json.loads(current_raw.decode("utf-8-sig"))
    require(isinstance(current, dict), f"expected JSON object: {document_path}")
    baseline = copy.deepcopy(current)
    validate_boundary(baseline)
    baseline_target = target_row(baseline)
    baseline_target["visible"] = False
    baseline_target["kind"] = "particle"
    baseline_target["sourceRecipe"]["rendererShape"] = "sprite"
    baseline_target["detail"]["trail"].pop("tilingDistanceWorldUnits", None)
    baseline_target["detail"]["trail"].pop(
        "distanceTessellationStepWorldUnits", None
    )
    baseline_target["resources"] = []
    baseline_target["material"]["templateId"] = "effect.source_material"
    baseline_target["material"]["sourceProfile"] = copy.deepcopy(
        BASELINE_SOURCE_PROFILE
    )
    baseline_target["material"]["execution"] = {
        "enabled": False,
        "failClosed": True,
    }
    require(
        canonical_sha256(baseline_target) == BASELINE_TARGET_SHA256,
        "Artist T current target cannot reconstruct its sealed baseline",
    )
    final_document = project_document(baseline)
    is_projected = current == final_document
    receipt = build_projection_receipt(baseline, final_document)
    if write:
        changed = not is_projected
        if changed:
            write_atomic(
                document_path,
                replace_target_row_bytes(
                    current_raw, target_row(final_document)
                ),
            )
        write_atomic(output_receipt_path, output_bytes(receipt))
        return changed
    require(is_projected, "Artist T CascadeRibbon projection is not applied")
    require(
        output_receipt_path.is_file()
        and output_receipt_path.read_bytes() == output_bytes(receipt),
        "Artist T CascadeRibbon projection receipt is missing or stale",
    )
    return False


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--document", type=Path, default=DOCUMENT_PATH)
    arguments = parser.parse_args(argv)
    changed = run(arguments.write, arguments.document.resolve())
    print(
        ("WROTE" if changed else "PASS")
        + ": Artist 31950 one-row CascadeRibbon carrier and bounded material"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
