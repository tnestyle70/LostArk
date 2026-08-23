#!/usr/bin/env python3
"""Materialize the Valtan three-pattern V1 review slice.

The six V0 authored documents and the product cue document are immutable inputs.
This tool creates parallel ``.v1.unified`` documents, a data-only V0->V1 alias
table, and registry bindings for every Sprite/Mesh occurrence in the selected
patterns.  The bindings are explicitly PROJECT_TUNED_APPROX: they use one
reviewable base/coverage texture and never claim source-exact UE3 semantics.
Ribbon and Light rows remain unchanged presentation carriers in the V1 copies.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
import re
import sys
from typing import Any


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]

LEDGER_RELATIVE_PATH = Path(
    "Data/Effects/Contracts/valtan-effect-v1-horizontal-rt0-application.v1.json"
)
CUE_RELATIVE_PATH = Path(
    "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
CATALOG_RELATIVE_PATH = Path("Data/Effects/EffectCatalog.json")
ALIAS_RELATIVE_PATH = Path(
    "Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json"
)
FRAGMENT_RELATIVE_PATH = Path(
    "Data/Effects/MaterialPrograms/Fragments/"
    "valtan-three-pattern-v1.material-program-fragment.v1.json"
)
RECEIPT_RELATIVE_PATH = Path(
    "Data/Effects/Contracts/valtan-three-pattern-v1-materialization.receipt.json"
)

TARGET_PATTERNS = (
    "VALTAN_ARMOR_BREAK_OPENING",
    "VALTAN_MAGIC_CHOICE",
    "VALTAN_WHIRLWIND",
)
EXPECTED_PATTERN_STAGE_COUNTS = {
    "VALTAN_ARMOR_BREAK_OPENING/WALL_CHARGE": 10,
    "VALTAN_MAGIC_CHOICE/INNER": 4,
    "VALTAN_MAGIC_CHOICE/OUTER": 2,
    "VALTAN_MAGIC_CHOICE/RECOVERY": 13,
    "VALTAN_WHIRLWIND/RECOVERY": 3,
    "VALTAN_WHIRLWIND/SPIN": 9,
}
EXPECTED_CARRIER_COUNTS = {
    "MESH": 12,
    "PRESENTATION": 1,
    "RIBBON": 3,
    "SPRITE": 25,
}

PROGRAM_IDS = {
    "srgb": (
        "effect.program.runtime-material-v2.opcode-1001."
        "project-tuned-base-coverage-srgb.v1"
    ),
    "linear": (
        "effect.program.runtime-material-v2.opcode-1002."
        "project-tuned-base-coverage-linear.v1"
    ),
}
LAYOUT_IDS = {
    "srgb": (
        "effect.layout.runtime-material-v2.opcode-1001."
        "project-tuned-base-coverage-srgb.v1"
    ),
    "linear": (
        "effect.layout.runtime-material-v2.opcode-1002."
        "project-tuned-base-coverage-linear.v1"
    ),
}
OPCODES = {"srgb": 1001, "linear": 1002}

RESOURCE_SLOT_PRIORITY = {
    # A named mask is stronger coverage evidence than a visually plausible
    # colour texture.  Base is accepted only through the audited DDS allowlist
    # below; noise/emissive/dissolve are never guessed into coverage.
    "mask": 0,
    "base": 1,
    "emissive": 2,
    "noise": 3,
    "dissolve": 4,
}
LINEAR_TEXTURE_TOKENS = (
    "noise",
    "mask",
    "normal",
    "dissolve",
    "distort",
    "fluid",
    "cloud",
    "fragment",
)
STABLE_ID = re.compile(r"^[a-z0-9][a-z0-9._-]{0,255}$")

# Manual DDS inspection receipt for the six selected authored documents.
# Each tuple is (authored slot, sampled coverage channel, immutable raw SHA256,
# evidence basis).  RGB means Rec.601 luminance in HLSL; A means stored alpha.
# A texture absent from this table is deliberately not admitted.
COVERAGE_TEXTURE_EVIDENCE: dict[str, tuple[str, str, str, str]] = {
    "Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_glow_009.dds":
        ("base", "RGB", "f0f56b8f25f444bcce33da6280faa5663139ca7a044474c5235784be8534daaf", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_00/fx_a_cloud_005.dds":
        ("base", "A", "52cf75cf242edfe47c56f555ae004b13d6a806c7ddbb68a4faafff9330c18a78", "DDS_ALPHA_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_00/fx_a_cloud_019.dds":
        ("base", "A", "5dc0b06021d5c41a6dadecc436b8c0707d1c72441557cdb986a96cb0f6551836", "DDS_ALPHA_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_00/fx_a_fragment_007.dds":
        ("base", "RGB", "fb87b3880fd90ce60bf91cf0e3d9b383ab7dc63847d3fea71e3df39716282c6a", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_00/fx_a_hit_008.dds":
        ("base", "RGB", "131fd2a0bca13bb0cfbc90812d855ebbd1028cbcd1a80ebb1fa7a68fa16b9665", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_00/fx_a_ring_001_cl.dds":
        ("base", "RGB", "f01c9024087d5ccbfab8e74de6b91792821dd7c23c9ca2a04921c352a114a803", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_00/fx_b_fragment_004.dds":
        ("base", "RGB", "633d3bedd910e3623839d55074c12cbc4f53742bf2da366b4e762a771745a29e", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_01/fx_c_noise_001.dds":
        ("base", "A", "b5cda1a907dd417b5ff7b5405ad0604da3507d4df56f96cbd0310042e2a8f23b", "DDS_ALPHA_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_01/fx_c_ring_002.dds":
        ("base", "A", "1045f25f5cdb36e4afb66a3bb2417158c4ecbda419d1e31aa269dd620c3c07c0", "DDS_ALPHA_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_028.dds":
        ("base", "RGB", "0cda1d83c6df738528a3e32c45fd72f76a6acddf0010c76170032ae4e3469933", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_034_cl.dds":
        ("base", "RGB", "a2932ffc28a56b9ea99ec8492b6049ffd8a3f36f578333f4840337a0cb622000", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_decal_033.dds":
        ("base", "A", "ac581718f27e8d2a892ab9d6509fe6abe611a1286030c048d3a308617fdd4754", "DDS_ALPHA_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_002.dds":
        ("mask", "RGB", "6125c3c1bcea0455d3f3c9bf0c8092331cd789f9e6686d00eac45f136fe79393", "AUTHORED_MASK_DDS_RGB_LUMINANCE"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_097.dds":
        ("mask", "RGB", "46ba6a315b34e3cfde1a273786c703d15fe769542eb9a2f7749971df3a111dc0", "AUTHORED_MASK_DDS_RGB_LUMINANCE"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_009.dds":
        ("mask", "RGB", "d5e63e9f023fb9f9ffb5dc2d32f80319102b91c94c5fe7c951177d161f963c9e", "AUTHORED_MASK_DDS_RGB_LUMINANCE"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_trail_002_cl.dds":
        ("base", "RGB", "79c6c94326a2995ecf224e667931207a912cf8fbbe21ab8de7fdf9f0f86935d2", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_03/fx_e_fluid_007.dds":
        ("base", "A", "703f91ed9e11b972fab48e96f6ecfda0db70730122142ea15f123d075384269b", "DDS_ALPHA_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_03/fx_e_fluid_003.dds":
        ("mask", "A", "3339f6fd17e588fed6c11082a8a93a9b6dc55c49cb20f2a956dd6258f675a83f", "AUTHORED_MASK_DDS_ALPHA"),
    "Effect/Valtan/Textures/FX_TEX_03/fx_e_ring_001.dds":
        ("base", "A", "3c8987c8bc4bda1d3fd0f4840124e4fc1ba2eb3899307df8fe5852c4a760738e", "DDS_ALPHA_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_04/fx_f_ring_001.dds":
        ("base", "RGB", "0379a6ee48ae7f0c326bbd2982a3e157cf8ebdd3f97a1841378d4e348483d92e", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_04/fx_i_atypical_02.dds":
        ("base", "RGB", "84ebc4ff2ef7291ca3939a7887f90d7410f44f4f76d44ca0b82bee755c0f8f2e", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_05/fx_m_ring_001.dds":
        ("mask", "RGB", "662c8a752d8bdd041516c1458ced19731bddab425723d25531fefa43fb99b248", "AUTHORED_MASK_DDS_RGB_LUMINANCE"),
    "Effect/Valtan/Textures/FX_TEX_05/fx_m_trail_001_cl.dds":
        ("mask", "RGB", "11675c138cf89c50c65c368b715096e1fc53f5439338273ca4ebd0f7d036b52a", "AUTHORED_MASK_DDS_RGB_LUMINANCE"),
    "Effect/Valtan/Textures/FX_TEX_05/fx_m_wave_001_ycl.dds":
        ("base", "RGB", "25c77b81be3c5ccfedf6eaa153eb1bbe5c5b8689530afc633a698d6e2b3a2c0a", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_HIGH_00/fx_c_glow_007.dds":
        ("base", "RGB", "992669c209f95e7c7906bdaab42263036e926fe0800e2279d4a5f0f25081fed6", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_HIGH_00/fx_c_glow_008.dds":
        ("base", "RGB", "bdeebd5157937371168f031d734147290eae84e52dd30e91c86c121957bc8de9", "DDS_RGB_LUMINANCE_NONCONSTANT"),
    "Effect/Valtan/Textures/FX_TEX_HIGH_01/fx_f_electric_005.dds":
        ("mask", "A", "6fd35c8bd022f92f1306068240a72d0ab04b0e9dc1d707cf204fe6c4cf611a64", "AUTHORED_MASK_DDS_ALPHA"),
}

COVERAGE_CHANNEL_SELECTOR = {"A": 0.0, "R": 1.0, "RGB": 2.0}
COVERAGE_SELECTOR_SCALAR_NAME = "coverage-channel-selector.0"


def _load_json(path: Path, label: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise ValueError(f"{label} could not be parsed: {error}") from error
    if not isinstance(value, dict):
        raise ValueError(f"{label} must be a JSON object.")
    return value


def _json_bytes(value: Any) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def _sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def _sha256_file(path: Path) -> str:
    return _sha256_bytes(path.read_bytes())


def _stable_suffix(*values: str) -> str:
    joined = "\0".join(values).encode("utf-8")
    return hashlib.sha256(joined).hexdigest()[:20]


def _require_stable_id(value: Any, label: str) -> str:
    if not isinstance(value, str) or STABLE_ID.fullmatch(value) is None:
        raise ValueError(f"{label} must be a stable lowercase ID: {value!r}")
    return value


def _target_cues(cue_document: dict[str, Any]) -> list[dict[str, Any]]:
    if (
        cue_document.get("schema") != "lostark.valtan-pattern-effect-cues"
        or cue_document.get("formatVersion") != 2
        or cue_document.get("ownerArchetypeId") != "BOSS_VALTAN"
    ):
        raise ValueError("Valtan pattern cue identity/schema drifted.")
    cues = cue_document.get("cues")
    if not isinstance(cues, list):
        raise ValueError("Valtan pattern cues must be an array.")
    selected = [row for row in cues if row.get("patternId") in TARGET_PATTERNS]
    if len(selected) != 6:
        raise ValueError(f"Expected exactly 6 target cues; found {len(selected)}.")
    stage_keys = [f"{row.get('patternId')}/{row.get('stageId')}" for row in selected]
    if sorted(stage_keys) != sorted(EXPECTED_PATTERN_STAGE_COUNTS):
        raise ValueError(f"Target cue stage set drifted: {sorted(stage_keys)}")
    effect_ids = [_require_stable_id(row.get("effectAssetId"), "cue effectAssetId") for row in selected]
    if len(effect_ids) != len(set(effect_ids)):
        raise ValueError("Target cues must resolve to six distinct V0 Effect assets.")
    return sorted(selected, key=lambda row: row["effectAssetId"])


def _target_rows(ledger: dict[str, Any]) -> list[dict[str, Any]]:
    if (
        ledger.get("schema") != "lostark.valtan-effect-v1-horizontal-rt0-application"
        or ledger.get("formatVersion") != 1
    ):
        raise ValueError("Valtan horizontal RT0 ledger identity/schema drifted.")
    rows = ledger.get("rows")
    if not isinstance(rows, list):
        raise ValueError("Valtan horizontal RT0 ledger rows must be an array.")
    selected = [
        row for row in rows
        if row.get("composition", {}).get("patternId") in TARGET_PATTERNS
    ]
    if len(selected) != 41:
        raise ValueError(f"Expected exactly 41 target ledger rows; found {len(selected)}.")

    stage_counts: dict[str, int] = {}
    carrier_counts: dict[str, int] = {}
    identities: set[tuple[str, str]] = set()
    for row in selected:
        composition = row.get("composition", {})
        occurrence = row.get("occurrence", {})
        carrier = row.get("carrier", {}).get("carrierKind")
        stage_key = f"{composition.get('patternId')}/{composition.get('stageId')}"
        stage_counts[stage_key] = stage_counts.get(stage_key, 0) + 1
        carrier_counts[carrier] = carrier_counts.get(carrier, 0) + 1
        identity = (occurrence.get("effectAssetId"), occurrence.get("elementId"))
        if identity in identities:
            raise ValueError(f"Duplicate target occurrence identity: {identity}")
        identities.add(identity)
    if stage_counts != EXPECTED_PATTERN_STAGE_COUNTS:
        raise ValueError(f"Target pattern/stage counts drifted: {stage_counts}")
    if carrier_counts != EXPECTED_CARRIER_COUNTS:
        raise ValueError(f"Target carrier counts drifted: {carrier_counts}")
    return sorted(
        selected,
        key=lambda row: (
            row["occurrence"]["effectAssetId"],
            row["occurrence"]["elementId"],
        ),
    )


def _catalog_index(catalog: dict[str, Any]) -> dict[str, dict[str, Any]]:
    if catalog.get("formatVersion") != 1 or not isinstance(catalog.get("effects"), list):
        raise ValueError("Effect catalog identity/schema drifted.")
    result: dict[str, dict[str, Any]] = {}
    for row in catalog["effects"]:
        effect_id = _require_stable_id(row.get("effectAssetId"), "catalog effectAssetId")
        if effect_id in result:
            raise ValueError(f"Duplicate Effect catalog ID: {effect_id}")
        result[effect_id] = row
    return result


def _source_path(repository_root: Path, catalog_row: dict[str, Any]) -> Path:
    authoring_path = catalog_row.get("authoringPath")
    if not isinstance(authoring_path, str) or not authoring_path.startswith("Effects/Authored/"):
        raise ValueError(f"Unexpected target authoring path: {authoring_path!r}")
    path = (repository_root / "Data" / Path(authoring_path)).resolve()
    authored_root = (repository_root / "Data/Effects/Authored").resolve()
    if authored_root not in path.parents:
        raise ValueError(f"Target authoring path escapes Authored root: {path}")
    return path


def _v1_effect_id(source_effect_id: str) -> str:
    return f"{source_effect_id}.v1.unified"


def _v1_display_name(source_display_name: Any, source_effect_id: str) -> str:
    """Keep the authored label recognizable within the runtime UTF-8 limit."""
    base = (
        source_display_name.strip()
        if isinstance(source_display_name, str) and source_display_name.strip()
        else source_effect_id
    )
    suffix = " [V1]"
    byte_limit = 64
    base_budget = byte_limit - len(suffix.encode("utf-8"))
    encoded = base.encode("utf-8")
    if len(encoded) > base_budget:
        base = encoded[:base_budget].decode("utf-8", errors="ignore").rstrip()
    result = f"{base}{suffix}"
    if not base or len(result.encode("utf-8")) > byte_limit:
        raise ValueError(f"Could not construct bounded V1 displayName for {source_effect_id}.")
    return result


def _v1_path(source_path: Path) -> Path:
    suffix = ".effect.json"
    if not source_path.name.endswith(suffix):
        raise ValueError(f"Source Effect document does not use {suffix}: {source_path}")
    stem = source_path.name[:-len(suffix)]
    return source_path.with_name(f"{stem}.v1.unified.effect.json")


def _select_texture(
    repository_root: Path,
    element: dict[str, Any],
) -> tuple[
    dict[str, Any] | None,
    dict[str, Any] | None,
    str | None,
    list[dict[str, str]],
    list[dict[str, str]],
]:
    resources = element.get("resources")
    if not isinstance(resources, list):
        raise ValueError(f"Element {element.get('id')} resources must be an array.")
    candidates: list[dict[str, str]] = []
    for resource in resources:
        if not isinstance(resource, dict):
            raise ValueError(f"Element {element.get('id')} has a malformed resource row.")
        slot_id = resource.get("slotId")
        asset_id = resource.get("assetId")
        if slot_id == "meshModel":
            continue
        if not isinstance(slot_id, str) or not isinstance(asset_id, str):
            raise ValueError(f"Element {element.get('id')} has an invalid texture resource.")
        if not asset_id.startswith("Effect/") or not asset_id.lower().endswith(".dds"):
            continue
        candidates.append({"slotId": slot_id, "assetId": asset_id})
    candidates.sort(
        key=lambda row: (
            RESOURCE_SLOT_PRIORITY.get(row["slotId"], 100),
            row["slotId"],
            row["assetId"],
        )
    )
    rejected: list[dict[str, str]] = []
    for selected in candidates:
        if selected["slotId"] not in {"mask", "base"}:
            rejected.append({**selected, "reasonCode": "NON_COVERAGE_AUTHORED_SLOT"})
            continue
        evidence = COVERAGE_TEXTURE_EVIDENCE.get(selected["assetId"])
        if evidence is None:
            rejected.append({**selected, "reasonCode": "DDS_COVERAGE_NOT_AUDITED"})
            continue
        evidence_slot, channel, expected_sha256, basis = evidence
        if selected["slotId"] != evidence_slot:
            rejected.append({**selected, "reasonCode": "AUTHORED_SLOT_EVIDENCE_MISMATCH"})
            continue
        resource_path = (
            repository_root / "Client/Bin/Resources" / Path(selected["assetId"])
        )
        if not resource_path.is_file():
            rejected.append({**selected, "reasonCode": "DDS_RESOURCE_MISSING"})
            continue
        actual_sha256 = _sha256_file(resource_path)
        if actual_sha256 != expected_sha256:
            rejected.append({**selected, "reasonCode": "DDS_EVIDENCE_SHA256_DRIFT"})
            continue

        lower_asset = selected["assetId"].lower()
        color_space = "linear" if (
            selected["slotId"] == "mask"
            or any(token in lower_asset for token in LINEAR_TEXTURE_TOKENS)
        ) else "srgb"
        return selected, {
            "coverageChannel": channel,
            "selectorValue": COVERAGE_CHANNEL_SELECTOR[channel],
            "ddsSha256": actual_sha256,
            "basis": basis,
        }, color_space, candidates, rejected
    return None, None, None, candidates, rejected


def _render_contract(row: dict[str, Any]) -> dict[str, Any]:
    carrier = row["carrier"]["carrierKind"]
    if carrier not in {"SPRITE", "MESH"}:
        raise ValueError(f"No runtimeMaterialV2 Adapter for carrier {carrier}.")
    source_state = row.get("adapter", {}).get("sourceRenderState", {})
    blend_mode = source_state.get("blendMode")
    two_sided = source_state.get("twoSided")
    disable_depth = source_state.get("disableDepthTest")
    if blend_mode not in {"TRANSLUCENT", "ADDITIVE"}:
        raise ValueError(f"Unsupported target source blend mode: {blend_mode!r}")
    if not isinstance(two_sided, bool) or disable_depth is not False:
        raise ValueError(
            "Target generic Adapter requires resolved sidedness and depth-read state."
        )
    blend_token = "alpha" if blend_mode == "TRANSLUCENT" else "additive"
    side_token = "two" if two_sided else "one"
    carrier_token = "sprite-particle" if carrier == "SPRITE" else "mesh-particle.cmodel"
    adapter_id = (
        f"effect.adapter.{carrier_token}.scene-color-rt0.zero-distortion-rt1."
        f"project-tuned-{blend_token}-{side_token}-sided.v1"
    )
    pass_index = {
        ("alpha", "two"): 1,
        ("additive", "two"): 2,
        ("alpha", "one"): 3,
        ("additive", "one"): 4,
    }[(blend_token, side_token)]
    return {
        "adapterId": adapter_id,
        "renderProfile": f"{blend_token}_{side_token}_sided_depth_read",
        "passIndex": pass_index,
        "renderState": {
            "rasterizer": "RS_Cull_None" if two_sided else "RS_Default",
            "depthStencil": "DSS_ReadOnly",
            "blend": "BS_EffectAlpha" if blend_token == "alpha" else "BS_EffectAdditive",
            "stencilReference": 0,
        },
    }


def _sampler() -> dict[str, Any]:
    return {
        "filter": "linear",
        "addressU": "wrap",
        "addressV": "wrap",
        "addressW": "wrap",
        "mipLodBias": 0,
        "maxAnisotropy": 1,
        "comparison": "never",
        "borderColor": [0, 0, 0, 0],
        "minLod": 0,
        "maxLod": 3.40282347e38,
    }


def _layout(color_space: str) -> dict[str, Any]:
    return {
        "layoutId": LAYOUT_IDS[color_space],
        "executionVersion": 1,
        "textureLaneCount": 1,
        "textureMask": 1,
        "textureLanes": [{
            "laneId": "lane.0",
            "role": "base_coverage",
            "textureRegister": 0,
            "samplerRegister": 5,
            "sourceChannel": "RGBA",
            "colorSpace": color_space,
        }],
        "dynamicConsumedMask": 0,
        "dynamicSuppressedMask": 15,
        "particleColorPolicy": 2,
        "particleColorConsumedMask": 15,
        "particleColorSuppressedMask": 0,
        "scalarCount": 1,
        "vectorCount": 0,
        "inputCount": 1,
        "inputConsumedMask": [1, 0],
        "inputSuppressedMask": [0, 0],
        "vectorComponentConsumedMask": [0, 0, 0],
        "vectorComponentSuppressedMask": [0, 0, 0],
        "staticInputCount": 0,
        "staticSelectedMask": 0,
        "staticConsumedMask": 0,
        "staticSuppressedMask": 0,
        "renderInputCount": 6,
        "renderConsumedMask": 47,
        "renderSuppressedMask": 16,
        "scalarRows": [{
            "name": COVERAGE_SELECTOR_SCALAR_NAME,
            "packedIndex": 0,
        }],
        "vectorRows": [],
        "artistParameterRows": [],
        "colorRows": [],
    }


def _descriptor(
    descriptor_id: str,
    color_space: str,
    asset_id: str,
    coverage_selector: float,
) -> dict[str, Any]:
    return {
        "descriptorId": descriptor_id,
        "layoutId": LAYOUT_IDS[color_space],
        "textureLanes": [{
            "laneId": "lane.0",
            "assetId": asset_id,
            "sampler": _sampler(),
        }],
        "scalars": [{
            "name": COVERAGE_SELECTOR_SCALAR_NAME,
            "value": coverage_selector,
        }],
        "vectors": [],
        "artistParameters": [],
        "colors": [],
    }


def _execution(
    color_space: str,
    asset_id: str,
    coverage_selector: float,
    render_contract: dict[str, Any],
) -> dict[str, Any]:
    execution = {
        "enabled": True,
        "fidelity": "PROJECT_TUNED_APPROX",
        "version": 1,
        "backend": "runtimeMaterialV2",
        "opcode": OPCODES[color_space],
        "passIndex": render_contract["passIndex"],
        "renderState": copy.deepcopy(render_contract["renderState"]),
        "textureLaneCount": 1,
        "textureMask": 1,
        "textureLanes": [{
            "laneId": "lane.0",
            "role": "base_coverage",
            "assetId": asset_id,
            "textureRegister": 0,
            "samplerRegister": 5,
            "sourceChannel": "RGBA",
            "colorSpace": color_space,
            "sampler": _sampler(),
        }],
        "dynamicConsumedMask": 0,
        "dynamicSuppressedMask": 15,
        "particleColorPolicy": 2,
        "particleColorConsumedMask": 15,
        "particleColorSuppressedMask": 0,
        "scalarCount": 1,
        "vectorCount": 0,
        "inputCount": 1,
        "inputConsumedMask": [1, 0],
        "inputSuppressedMask": [0, 0],
        "vectorComponentConsumedMask": [0, 0, 0],
        "vectorComponentSuppressedMask": [0, 0, 0],
        "staticInputCount": 0,
        "staticSelectedMask": 0,
        "staticConsumedMask": 0,
        "staticSuppressedMask": 0,
        "renderInputCount": 6,
        "renderConsumedMask": 47,
        "renderSuppressedMask": 16,
        "scalars": [{
            "name": COVERAGE_SELECTOR_SCALAR_NAME,
            "packedIndex": 0,
            "value": coverage_selector,
        }],
        "vectors": [],
        "artistParameters": [],
        "colors": [],
    }
    return execution


def build_artifacts(repository_root: Path = REPOSITORY_ROOT) -> dict[Path, bytes]:
    repository_root = repository_root.resolve()
    ledger_path = repository_root / LEDGER_RELATIVE_PATH
    cue_path = repository_root / CUE_RELATIVE_PATH
    catalog_path = repository_root / CATALOG_RELATIVE_PATH
    ledger = _load_json(ledger_path, "Valtan horizontal RT0 ledger")
    cue_document = _load_json(cue_path, "Valtan pattern Effect cues")
    catalog = _load_json(catalog_path, "Effect catalog")
    cues = _target_cues(cue_document)
    rows = _target_rows(ledger)
    catalog_by_id = _catalog_index(catalog)

    source_effect_ids = [row["effectAssetId"] for row in cues]
    row_effect_ids = {row["occurrence"]["effectAssetId"] for row in rows}
    if row_effect_ids != set(source_effect_ids):
        raise ValueError(
            "Target ledger/cue Effect asset join drifted: "
            f"ledger={sorted(row_effect_ids)}, cues={sorted(source_effect_ids)}"
        )

    rows_by_effect: dict[str, list[dict[str, Any]]] = {}
    for row in rows:
        rows_by_effect.setdefault(row["occurrence"]["effectAssetId"], []).append(row)

    artifacts: dict[Path, bytes] = {}
    aliases: list[dict[str, str]] = []
    catalog_additions: list[dict[str, str]] = []
    descriptors: list[dict[str, Any]] = []
    bindings: list[dict[str, Any]] = []
    receipt_rows: list[dict[str, Any]] = []
    source_documents: list[dict[str, Any]] = []

    for source_effect_id in sorted(source_effect_ids):
        catalog_row = catalog_by_id.get(source_effect_id)
        if catalog_row is None:
            raise ValueError(f"Target V0 Effect is absent from catalog: {source_effect_id}")
        source_path = _source_path(repository_root, catalog_row)
        source_document = _load_json(source_path, f"source Effect {source_effect_id}")
        if (
            source_document.get("schema") != "lostark.effect-authoring"
            or source_document.get("version") != 13
            or source_document.get("effectAssetId") != source_effect_id
        ):
            raise ValueError(f"Source Effect identity/schema drifted: {source_effect_id}")
        elements = source_document.get("elements")
        if not isinstance(elements, list):
            raise ValueError(f"Source Effect elements are malformed: {source_effect_id}")
        element_by_id = {element.get("id"): element for element in elements}
        if len(element_by_id) != len(elements):
            raise ValueError(f"Source Effect has duplicate element IDs: {source_effect_id}")
        ledger_element_ids = {
            row["occurrence"]["elementId"] for row in rows_by_effect[source_effect_id]
        }
        if set(element_by_id) != ledger_element_ids:
            raise ValueError(
                f"Source Effect/ledger element set drifted for {source_effect_id}."
            )

        v1_effect_id = _v1_effect_id(source_effect_id)
        target_document = copy.deepcopy(source_document)
        target_document["effectAssetId"] = v1_effect_id
        target_document["displayName"] = _v1_display_name(
            source_document.get("displayName"), source_effect_id
        )
        target_elements = {element["id"]: element for element in target_document["elements"]}

        for row in rows_by_effect[source_effect_id]:
            element_id = row["occurrence"]["elementId"]
            carrier = row["carrier"]["carrierKind"]
            source_element = element_by_id[element_id]
            target_element = target_elements[element_id]
            base_receipt = {
                "sourceEffectAssetId": source_effect_id,
                "v1EffectAssetId": v1_effect_id,
                "elementId": element_id,
                "patternId": row["composition"]["patternId"],
                "stageId": row["composition"]["stageId"],
                "carrierKind": carrier,
                "sourceProgramFidelity": row["program"]["fidelity"],
                "sourceMaterialPath": row["occurrence"]["sourceMaterialPath"],
                "sourceMaterialEvidenceId": row["descriptor"].get(
                    "sourceMaterialEvidenceId"
                ),
                "descriptorEvidenceStatus": row["descriptor"].get(
                    "evidenceStatus"
                ),
                "sourceRenderStateId": row["adapter"]["sourceRenderState"]["renderStateId"],
            }
            if carrier not in {"SPRITE", "MESH"}:
                if carrier not in {"RIBBON", "PRESENTATION"}:
                    raise ValueError(f"Unexpected target carry carrier: {carrier}")
                if target_element != source_element:
                    raise ValueError(f"V0 presentation carry mutated unexpectedly: {element_id}")
                receipt_rows.append({
                    **base_receipt,
                    "classification": "V0_PRESENTATION_CARRY",
                    "binding": None,
                    "textureSelection": None,
                })
                continue

            selected, coverage_evidence, color_space, candidates, rejected = _select_texture(
                repository_root, source_element
            )
            if selected is None or coverage_evidence is None or color_space is None:
                source_profile = target_element["material"].get("sourceProfile")
                if isinstance(source_profile, dict):
                    source_profile["enabled"] = False
                target_element["material"]["execution"] = {
                    "enabled": False,
                    "failClosed": True,
                    "authoringApproximate": True,
                }
                receipt_rows.append({
                    **base_receipt,
                    "classification": "PROJECT_TUNED_APPROX_DEFERRED",
                    "binding": None,
                    "textureSelection": {
                        "policy": "AUDITED_COVERAGE_DDS_FAIL_CLOSED_V2",
                        "reasonCode": "NO_AUDITED_COVERAGE_DDS",
                        "candidates": candidates,
                        "rejectedCandidates": rejected,
                    },
                })
                continue
            render_contract = _render_contract(row)
            descriptor_id = (
                "effect.descriptor.valtan-three-pattern-v1."
                f"{_stable_suffix(v1_effect_id, element_id)}.v1"
            )
            descriptor = _descriptor(
                descriptor_id,
                color_space,
                selected["assetId"],
                coverage_evidence["selectorValue"],
            )
            binding = {
                "effectAssetId": v1_effect_id,
                "elementId": element_id,
                "programId": PROGRAM_IDS[color_space],
                "layoutId": LAYOUT_IDS[color_space],
                "descriptorId": descriptor_id,
                "adapterId": render_contract["adapterId"],
                "inlineMirrorPolicy": "INLINE_MIRROR_REQUIRED",
            }
            target_element["material"]["renderProfile"] = render_contract["renderProfile"]
            source_profile = target_element["material"].get("sourceProfile")
            if source_profile is not None:
                if not isinstance(source_profile, dict):
                    raise ValueError(
                        f"Element {element_id} sourceProfile must be an object."
                    )
                # Keep the imported profile payload as review evidence, but make the
                # V1 registry packet the only executable material authority.
                source_profile["enabled"] = False
            target_element["material"]["execution"] = _execution(
                color_space,
                selected["assetId"],
                coverage_evidence["selectorValue"],
                render_contract,
            )
            descriptors.append(descriptor)
            bindings.append(binding)
            receipt_rows.append({
                **base_receipt,
                "classification": "PROJECT_TUNED_APPROX",
                "binding": copy.deepcopy(binding),
                "textureSelection": {
                    "policy": "AUDITED_COVERAGE_DDS_CHANNEL_V2",
                    "selectedSlotId": selected["slotId"],
                    "selectedAssetId": selected["assetId"],
                    "colorSpace": color_space,
                    **coverage_evidence,
                    "candidates": candidates,
                    "rejectedCandidates": rejected,
                },
            })

        target_path = _v1_path(source_path)
        target_relative_path = target_path.relative_to(repository_root)
        artifacts[target_relative_path] = _json_bytes(target_document)
        aliases.append({
            "effectAssetId": source_effect_id,
            "v1EffectAssetId": v1_effect_id,
        })
        catalog_additions.append({
            "effectAssetId": v1_effect_id,
            "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
            "authoringPath": target_path.relative_to(repository_root / "Data").as_posix(),
        })
        source_documents.append({
            "effectAssetId": source_effect_id,
            "path": source_path.relative_to(repository_root).as_posix(),
            "sha256": _sha256_file(source_path),
            "v1EffectAssetId": v1_effect_id,
            "v1Path": target_relative_path.as_posix(),
        })

    bindings.sort(key=lambda row: (row["effectAssetId"], row["elementId"]))
    descriptors.sort(key=lambda row: row["descriptorId"])
    receipt_rows.sort(key=lambda row: (row["v1EffectAssetId"], row["elementId"]))

    fragment = {
        "schema": "lostark.effect-material-program-registry-fragment",
        "formatVersion": 1,
        "domainId": "effect-domain.valtan-three-pattern-v1",
        "programs": [
            {
                "programId": PROGRAM_IDS["srgb"],
                "backend": "runtimeMaterialV2",
                "opcode": OPCODES["srgb"],
                "fidelity": "PROJECT_TUNED_APPROX",
            },
            {
                "programId": PROGRAM_IDS["linear"],
                "backend": "runtimeMaterialV2",
                "opcode": OPCODES["linear"],
                "fidelity": "PROJECT_TUNED_APPROX",
            },
        ],
        "layouts": [_layout("srgb"), _layout("linear")],
        "descriptors": descriptors,
        "bindings": bindings,
    }
    alias_document = {
        "schema": "lostark.valtan-pattern-effect-v1-aliases",
        "formatVersion": 1,
        "ownerArchetypeId": "BOSS_VALTAN",
        "aliases": aliases,
    }

    target_ids = {row["effectAssetId"] for row in catalog_additions}
    new_catalog_rows = [
        copy.deepcopy(row) for row in catalog["effects"]
        if row.get("effectAssetId") not in target_ids
    ]
    new_catalog_rows.extend(catalog_additions)
    new_catalog_rows.sort(key=lambda row: row["effectAssetId"])
    updated_catalog = {
        key: copy.deepcopy(value)
        for key, value in catalog.items()
        if key != "effects"
    }
    updated_catalog["effects"] = new_catalog_rows

    classification_counts: dict[str, int] = {}
    opcode_counts: dict[str, int] = {}
    adapter_counts: dict[str, int] = {}
    for row in receipt_rows:
        classification = row["classification"]
        classification_counts[classification] = classification_counts.get(classification, 0) + 1
        binding = row["binding"]
        if binding is not None:
            opcode = str(
                OPCODES[row["textureSelection"]["colorSpace"]]
            )
            opcode_counts[opcode] = opcode_counts.get(opcode, 0) + 1
            adapter_id = binding["adapterId"]
            adapter_counts[adapter_id] = adapter_counts.get(adapter_id, 0) + 1

    receipt = {
        "schema": "lostark.valtan-three-pattern-v1-materialization-receipt",
        "formatVersion": 1,
        "ownerArchetypeId": "BOSS_VALTAN",
        "fidelityPolicy": {
            "runtimeClassification": "PROJECT_TUNED_APPROX",
            "claimSourceExact": False,
            "equation": "GENERIC_BASE_COVERAGE_TIMES_PARTICLE_COLOR",
            "excludedSourceSemantics": [
                "MULTI_TEXTURE_EQUATION",
                "SCALAR_VECTOR_RUNTIME_ABI",
                "SCENE_INPUT",
                "MRT_DISTORTION",
                "VERTEX_WPO",
            ],
            "presentationCarryPolicy": "COPY_V0_RIBBON_AND_LIGHT_UNCHANGED",
        },
        "immutableInputs": {
            "ledgerPath": LEDGER_RELATIVE_PATH.as_posix(),
            "ledgerSha256": _sha256_file(ledger_path),
            "cuePath": CUE_RELATIVE_PATH.as_posix(),
            "cueSha256": _sha256_file(cue_path),
            "sourceDocuments": source_documents,
        },
        "aliases": aliases,
        "summary": {
            "sourceEffectCount": 6,
            "v1EffectCount": 6,
            "occurrenceCount": 41,
            "bindingCount": len(bindings),
            "classificationCounts": classification_counts,
            "opcodeCounts": opcode_counts,
            "adapterCounts": dict(sorted(adapter_counts.items())),
        },
        "rows": receipt_rows,
        "manualProductReview": "PENDING_USER_VISUAL_A_B",
    }
    if classification_counts.get("V0_PRESENTATION_CARRY") != 4 or (
        classification_counts.get("PROJECT_TUNED_APPROX", 0) +
        classification_counts.get("PROJECT_TUNED_APPROX_DEFERRED", 0)
    ) != 37:
        raise ValueError(f"Target V1 classification counts drifted: {classification_counts}")
    if len(bindings) != classification_counts.get("PROJECT_TUNED_APPROX", 0) or (
        len(descriptors) != len(bindings)
    ):
        raise ValueError("Bound V1 rows must map one-to-one to descriptors.")

    artifacts[ALIAS_RELATIVE_PATH] = _json_bytes(alias_document)
    artifacts[FRAGMENT_RELATIVE_PATH] = _json_bytes(fragment)
    artifacts[RECEIPT_RELATIVE_PATH] = _json_bytes(receipt)
    artifacts[CATALOG_RELATIVE_PATH] = _json_bytes(updated_catalog)
    return artifacts


def _write_or_check(
    repository_root: Path,
    artifacts: dict[Path, bytes],
    *,
    check: bool,
) -> None:
    errors: list[str] = []
    for relative_path in sorted(artifacts, key=lambda path: path.as_posix()):
        output_path = repository_root / relative_path
        expected = artifacts[relative_path]
        if check:
            if not output_path.is_file():
                errors.append(f"missing: {relative_path.as_posix()}")
            elif output_path.read_bytes() != expected:
                errors.append(f"drifted: {relative_path.as_posix()}")
            continue
        output_path.parent.mkdir(parents=True, exist_ok=True)
        if not output_path.is_file() or output_path.read_bytes() != expected:
            output_path.write_bytes(expected)
    if errors:
        raise ValueError("Generated Valtan three-pattern V1 artifacts are stale:\n" + "\n".join(errors))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=REPOSITORY_ROOT,
        help="Repository root (defaults to the materializer's repository).",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Validate that committed generated artifacts are byte-for-byte current.",
    )
    args = parser.parse_args(argv)
    try:
        repository_root = args.repository_root.resolve()
        artifacts = build_artifacts(repository_root)
        _write_or_check(repository_root, artifacts, check=args.check)
    except ValueError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1
    action = "verified" if args.check else "materialized"
    print(f"Valtan three-pattern V1 {action}: {len(artifacts)} artifacts")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
