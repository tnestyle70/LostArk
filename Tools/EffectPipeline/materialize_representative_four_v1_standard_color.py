#!/usr/bin/env python3
"""Materialize the representative-four StandardColorV1 audition documents.

This is an explicitly PROJECT_TUNED_APPROX migration.  It preserves each V0
occurrence carrier, transform, timing, source recipe, stable element ID, and
source-material evidence while replacing only the runtime Material packet and
ordinary texture resources.  No row produced here is SOURCE_EXACT.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile
from typing import Any, Iterable


ROOT = Path(__file__).resolve().parents[2]
AUTHORING_ROOT = Path("Data/Effects/Authored")
EFFECT_CATALOG = Path("Data/Effects/EffectCatalog.json")
FRAGMENT_PATH = Path(
    "Data/Effects/MaterialPrograms/Fragments/"
    "representative-four-v1-standard-color.material-program-fragment.v1.json"
)
RECEIPT_PATH = Path(
    "Data/Effects/AuthoredCorrections/Generated/RepresentativeFour/"
    "representative-four-v1-standard-color.receipt.json"
)

PROGRAM_ID = "effect.program.standard-color-v1.opcode-1.v1"
DOMAIN_ID = "effect-domain.representative-four-v1-standard-color"

PROJECT_TUNED_APPROX = "PROJECT_TUNED_APPROX"
REGISTRY_BOUND_AUDITION_ONLY = "REGISTRY_BOUND_AUDITION_ONLY"
INLINE_MIRROR_REQUIRED = "INLINE_MIRROR_REQUIRED"
MAX_LOD = 3.40282347e38

SPRITE_ALPHA_TWO = (
    "effect.adapter.sprite-particle.scene-color-rt0."
    "zero-distortion-rt1.alpha-two-sided.v1"
)
SPRITE_ALPHA_ONE = (
    "effect.adapter.sprite-particle.scene-color-rt0."
    "zero-distortion-rt1.alpha-one-sided.v1"
)
SPRITE_ADDITIVE_TWO = (
    "effect.adapter.sprite-particle.scene-color-rt0."
    "zero-distortion-rt1.additive-two-sided.v1"
)
SPRITE_ADDITIVE_ONE = (
    "effect.adapter.sprite-particle.scene-color-rt0."
    "zero-distortion-rt1.additive-one-sided.v1"
)
MESH_ALPHA_TWO = (
    "effect.adapter.mesh-particle.cmodel.scene-color-rt0."
    "zero-distortion-rt1.alpha-two-sided.v1"
)
MESH_ALPHA_ONE = (
    "effect.adapter.mesh-particle.cmodel.scene-color-rt0."
    "zero-distortion-rt1.alpha-one-sided.v1"
)
DECAL_ALPHA_TWO = (
    "effect.adapter.local-decal.projector.scene-color-rt0."
    "zero-distortion-rt1.alpha-two-sided.v1"
)
DECAL_ALPHA_ONE = (
    "effect.adapter.local-decal.projector.scene-color-rt0."
    "zero-distortion-rt1.alpha-one-sided.v1"
)


class RepresentativeFourV1Error(RuntimeError):
    """Raised when a sealed input or generated contract is invalid."""


SPECS: tuple[dict[str, Any], ...] = (
    {
        "v0": "effect.dimensionmaster.skill.2050180.unified",
        "v1": "effect.dimensionmaster.skill.2050180.v1.unified",
        "elements": 10,
        "rawSha256": "c1a6d50a386e292589863477d51bbbff82a674bd60ab563a6ee15d8775f82f46",
        "canonicalSha256": "4b6d9b39dbc95e8de38fe1725bd764d2e7beeff8e1926088e4db8c44421d1392",
        "fallbackAssetId": None,
    },
    {
        "v0": "effect.artist.skill.31460.unified",
        "v1": "effect.artist.skill.31460.v1.unified",
        "elements": 18,
        "rawSha256": "b83515d3cb1705238d0e1c18b3bc18ad1458de31ecbdb951dce45ff7abcc333f",
        "canonicalSha256": "962179e2b9c46823d24b0b92a29e588c79a62031f1ed2c46645dc9d9381394d2",
        "fallbackAssetId": None,
    },
    {
        "v0": "effect.lancemaster.skill.34110.unified",
        "v1": "effect.lancemaster.skill.34110.v1.unified",
        "elements": 88,
        "rawSha256": "297067f83379d1eca5bcc23d933450bd0f7c32951e96a848688d255083b79a2f",
        "canonicalSha256": "6bf6930079de827d275d3ecbac2e96edfe9147e73c0187300d988a8207291347",
        "fallbackAssetId": "Effect/LanceMaster/Textures/fx_a_glow_001.dds",
    },
    {
        "v0": "effect.warlord.skill.17110.clip2.unified",
        "v1": "effect.warlord.skill.17110.clip2.v1.unified",
        "elements": 3,
        "rawSha256": "d572b1957e5b10a068dc457b11f9ab53bef9e4124e08b60321a7a3854f832182",
        "canonicalSha256": "2c7e2b68aa308fd134b2c58c0f9eb6ff6cc603d072b8c5eb05a4dfb0f214b0c8",
        "fallbackAssetId": (
            "Effect/Warlord/Textures/FX_TEX_00/fx_a_glow_001.dds"
        ),
    },
    {
        "v0": "effect.warlord.skill.17110.clip3.unified",
        "v1": "effect.warlord.skill.17110.clip3.v1.unified",
        "elements": 12,
        "rawSha256": "6697860fc17ef912ebb55245e2d45d7fba71286f6fdfc1d6dd0209bbdb3b256a",
        "canonicalSha256": "1596f9722836017c69cf7b390603d290cdb28e7c00bb2fc253f685fad750cfcf",
        "fallbackAssetId": (
            "Effect/Warlord/Textures/FX_TEX_00/fx_a_glow_001.dds"
        ),
    },
)


PASS_STATE = {
    "alpha_two_sided_depth_read": (
        1, "RS_Cull_None", "DSS_ReadOnly", "BS_EffectAlpha"
    ),
    "additive_two_sided_depth_read": (
        2, "RS_Cull_None", "DSS_ReadOnly", "BS_EffectAdditive"
    ),
    "alpha_one_sided_depth_read": (
        3, "RS_Default", "DSS_ReadOnly", "BS_EffectAlpha"
    ),
    "additive_one_sided_depth_read": (
        4, "RS_Default", "DSS_ReadOnly", "BS_EffectAdditive"
    ),
}

ADAPTER_BY_CARRIER_PROFILE = {
    ("sprite", "alpha_two_sided_depth_read"): SPRITE_ALPHA_TWO,
    ("sprite", "alpha_one_sided_depth_read"): SPRITE_ALPHA_ONE,
    ("sprite", "additive_two_sided_depth_read"): SPRITE_ADDITIVE_TWO,
    ("sprite", "additive_one_sided_depth_read"): SPRITE_ADDITIVE_ONE,
    ("mesh", "alpha_two_sided_depth_read"): MESH_ALPHA_TWO,
    ("mesh", "alpha_one_sided_depth_read"): MESH_ALPHA_ONE,
    ("decal", "alpha_two_sided_depth_read"): DECAL_ALPHA_TWO,
    ("decal", "alpha_one_sided_depth_read"): DECAL_ALPHA_ONE,
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RepresentativeFourV1Error(message)


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return sha256_bytes(canonical_bytes(value))


def pretty_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2) + "\n"
    ).encode("utf-8")


def load_json_bytes(path: Path) -> tuple[bytes, dict[str, Any]]:
    try:
        payload = path.read_bytes()
        value = json.loads(payload.decode("utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise RepresentativeFourV1Error(f"cannot read {path}: {error}") from error
    require(isinstance(value, dict), f"JSON root must be an object: {path}")
    return payload, value


def source_document_path(effect_id: str) -> Path:
    return AUTHORING_ROOT / f"{effect_id}.effect.json"


def _validate_source_identity(
    spec: dict[str, Any], payload: bytes, document: dict[str, Any]
) -> None:
    require(
        sha256_bytes(payload) == spec["rawSha256"],
        f"sealed V0 raw identity changed: {spec['v0']}",
    )
    require(
        canonical_sha256(document) == spec["canonicalSha256"],
        f"sealed V0 canonical identity changed: {spec['v0']}",
    )
    require(
        document.get("schema") == "lostark.effect-authoring"
        and document.get("version") == 13
        and document.get("effectAssetId") == spec["v0"],
        f"sealed V0 document identity is invalid: {spec['v0']}",
    )
    elements = document.get("elements")
    require(
        isinstance(elements, list) and len(elements) == spec["elements"],
        f"sealed V0 element count changed: {spec['v0']}",
    )
    ids = [row.get("id") for row in elements if isinstance(row, dict)]
    require(
        len(ids) == len(elements) and len(set(ids)) == len(ids),
        f"sealed V0 element IDs are missing or duplicate: {spec['v0']}",
    )


def _classify_texture_role(label: str) -> str:
    normalized = re.sub(r"[^a-z0-9]+", "_", label.lower())
    if "dissolve" in normalized or "disslove" in normalized:
        return "dissolve"
    if any(token in normalized for token in ("mask", "opacity", "alpha")):
        return "mask"
    if any(
        token in normalized
        for token in (
            "base", "maintex", "main_tex", "diffuse", "diff_", "color",
        )
    ):
        return "base"
    if "emissive" in normalized or "emission" in normalized:
        return "emissive"
    if "noise" in normalized or "flow" in normalized:
        return "noise"
    return "other"


def _texture_candidates(element: dict[str, Any]) -> list[dict[str, Any]]:
    candidates: list[dict[str, Any]] = []
    resources = element.get("resources")
    require(isinstance(resources, list), f"resources is not an array: {element.get('id')}")
    for index, row in enumerate(resources):
        require(isinstance(row, dict), f"resource is not an object: {element.get('id')}")
        asset_id = row.get("assetId", "")
        slot_id = row.get("slotId", "")
        if not isinstance(asset_id, str) or not asset_id or slot_id == "meshModel":
            continue
        role = slot_id if slot_id in {
            "base", "emissive", "mask", "noise", "dissolve"
        } else _classify_texture_role(str(slot_id))
        candidates.append({
            "assetId": asset_id,
            "role": role,
            "origin": f"resources[{index}].{slot_id}",
        })

    material = element.get("material")
    source_profile = (
        material.get("sourceProfile") if isinstance(material, dict) else None
    )
    textures = (
        source_profile.get("textures", [])
        if isinstance(source_profile, dict)
        else []
    )
    require(isinstance(textures, list), f"sourceProfile.textures is invalid: {element.get('id')}")
    for index, row in enumerate(textures):
        if not isinstance(row, dict):
            raise RepresentativeFourV1Error(
                f"source texture is not an object: {element.get('id')}"
            )
        asset_id = row.get("assetId", "")
        if not isinstance(asset_id, str) or not asset_id:
            continue
        label = " ".join(
            str(row.get(field, "")) for field in ("name", "group")
        )
        candidates.append({
            "assetId": asset_id,
            "role": _classify_texture_role(label),
            "origin": f"sourceProfile.textures[{index}].{row.get('name', '')}",
        })
    return candidates


def _select_candidate(
    candidates: list[dict[str, Any]], priorities: Iterable[str]
) -> dict[str, Any] | None:
    for role in priorities:
        for candidate in candidates:
            if role == "any" or candidate["role"] == role:
                return candidate
    return None


def _source_sampling(
    element: dict[str, Any], asset_id: str
) -> dict[str, Any] | None:
    material = element.get("material")
    source_profile = (
        material.get("sourceProfile") if isinstance(material, dict) else None
    )
    textures = (
        source_profile.get("textures", [])
        if isinstance(source_profile, dict)
        else []
    )
    require(
        isinstance(textures, list),
        f"sourceProfile.textures is invalid: {element.get('id')}",
    )
    matches = [
        row for row in textures
        if isinstance(row, dict) and row.get("assetId") == asset_id
    ]
    if not matches:
        return None
    admitted: set[tuple[str, str, str]] = set()
    for row in matches:
        color_space = row.get("colorSpace")
        address_u = row.get("addressU")
        address_v = row.get("addressV")
        require(
            color_space in ("linear", "srgb")
            and address_u in ("wrap", "mirror", "clamp", "border")
            and address_v in ("wrap", "mirror", "clamp", "border"),
            "source texture sampling evidence is unsupported: "
            f"{element.get('id')}/{asset_id}",
        )
        admitted.add((str(color_space), str(address_u), str(address_v)))
    require(
        len(admitted) == 1,
        "source texture sampling evidence conflicts: "
        f"{element.get('id')}/{asset_id}",
    )
    color_space, address_u, address_v = next(iter(admitted))
    return {
        "colorSpace": color_space,
        "addressU": address_u,
        "addressV": address_v,
        "matchingSourceTextureCount": len(matches),
        "evidence": "SOURCE_PROFILE_TEXTURE_MATCH",
    }


def _texture_statistics(path: Path) -> dict[str, Any]:
    try:
        from PIL import Image
    except ImportError as error:
        raise RepresentativeFourV1Error(
            "Pillow is required for StandardColor DDS channel evidence."
        ) from error
    try:
        image = Image.open(path).convert("RGBA")
        extrema = image.getextrema()
        histograms = image.histogram()
    except (OSError, ValueError) as error:
        raise RepresentativeFourV1Error(
            f"cannot decode StandardColor DDS evidence: {path}: {error}"
        ) from error
    pixel_count = image.width * image.height
    require(pixel_count > 0, f"empty DDS evidence: {path}")
    channels: list[dict[str, Any]] = []
    for index, name in enumerate("RGBA"):
        histogram = histograms[index * 256:(index + 1) * 256]
        channels.append({
            "channel": name,
            "minimum": extrema[index][0],
            "maximum": extrema[index][1],
            "uniqueValueCount": sum(count > 0 for count in histogram),
            "nonZeroPixelCount": pixel_count - histogram[0],
            "fullyOpaquePixelCount": histogram[255],
            "mean": round(
                sum(value * count for value, count in enumerate(histogram))
                / pixel_count,
                6,
            ),
        })
    return {
        "width": image.width,
        "height": image.height,
        "channels": channels,
    }


def _resource_evidence(
    root: Path,
    asset_id: str,
    cache: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    existing = cache.get(asset_id)
    if existing is not None:
        return existing
    identity = _validate_asset(root, asset_id, "StandardColor DDS evidence")
    path = root / "Client/Bin/Resources" / Path(asset_id)
    statistics = _texture_statistics(path)
    evidence = {
        **identity,
        "textureStatistics": statistics,
        "textureStatisticsCanonicalSha256": canonical_sha256(statistics),
    }
    cache[asset_id] = evidence
    return evidence


def _sampler(
    address_u: str = "wrap", address_v: str = "wrap"
) -> dict[str, Any]:
    return {
        "filter": "linear",
        "addressU": address_u,
        "addressV": address_v,
        "addressW": "wrap",
        "mipLodBias": 0,
        "maxAnisotropy": 1,
        "comparison": "never",
        "borderColor": [0, 0, 0, 0],
        "minLod": 0,
        "maxLod": MAX_LOD,
    }


def _coverage_channel(evidence: dict[str, Any]) -> str:
    alpha = evidence["textureStatistics"]["channels"][3]
    return "A" if alpha["minimum"] < alpha["maximum"] else "R"


def _lane_policy(
    root: Path,
    element: dict[str, Any],
    candidate: dict[str, Any],
    role: str,
    evidence_cache: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    asset_id = str(candidate["assetId"])
    evidence = _resource_evidence(root, asset_id, evidence_cache)
    sampling = _source_sampling(element, asset_id)
    if sampling is None:
        sampling = {
            "colorSpace": "linear",
            "addressU": "wrap",
            "addressV": "wrap",
            "matchingSourceTextureCount": 0,
            "evidence": "PROJECT_TUNED_LINEAR_WRAP_POLICY",
        }
    if role == "base_radiance":
        semantic_role = str(candidate.get("role", "other"))
        source_channel = (
            "RGB"
            if semantic_role in ("base", "emissive")
            or sampling["colorSpace"] == "srgb"
            else "R"
        )
        color_space = sampling["colorSpace"]
        channel_evidence = (
            "SOURCE_TEXTURE_BASE_OR_EMISSIVE_RGB"
            if source_channel == "RGB"
            else "PROJECT_TUNED_SCALAR_RADIANCE_R"
        )
    else:
        source_channel = _coverage_channel(evidence)
        color_space = "linear"
        channel_evidence = (
            "DDS_VARIANT_ALPHA_COVERAGE"
            if source_channel == "A"
            else "DDS_OPAQUE_ALPHA_SCALAR_R"
        )
    return {
        "sourceChannel": source_channel,
        "colorSpace": color_space,
        "sampler": _sampler(sampling["addressU"], sampling["addressV"]),
        "samplingEvidence": sampling,
        "channelEvidence": channel_evidence,
        "ddsEvidenceCanonicalSha256": evidence[
            "textureStatisticsCanonicalSha256"
        ],
    }


def _execution_lane(
    index: int,
    role: str,
    candidate: dict[str, Any],
    policy: dict[str, Any],
) -> dict[str, Any]:
    return {
        "laneId": f"lane.{index}",
        "role": role,
        "assetId": candidate["assetId"],
        "textureRegister": index,
        "samplerRegister": 5 + index,
        "sourceChannel": policy["sourceChannel"],
        "colorSpace": policy["colorSpace"],
        "sampler": copy.deepcopy(policy["sampler"]),
    }


def _standard_color_execution(
    carrier: str,
    render_profile: str,
    base: tuple[dict[str, Any], dict[str, Any]],
    coverage: tuple[dict[str, Any], dict[str, Any]],
    dissolve: tuple[dict[str, Any], dict[str, Any]] | None,
) -> dict[str, Any]:
    require(render_profile in PASS_STATE, f"unsupported render profile: {render_profile}")
    pass_index, rasterizer, depth_stencil, blend = PASS_STATE[render_profile]
    if carrier == "decal" and render_profile == "alpha_two_sided_depth_read":
        depth_stencil = "DSS_ZNone"
    lanes = [
        _execution_lane(0, "base_radiance", *base),
        _execution_lane(1, "coverage", *coverage),
    ]
    if dissolve is not None:
        lanes.append(_execution_lane(2, "dissolve", *dissolve))
    has_dissolve = len(lanes) == 3
    return {
        "enabled": True,
        "version": 1,
        "backend": "standardColorV1",
        "opcode": 1,
        "passIndex": pass_index,
        "renderState": {
            "rasterizer": rasterizer,
            "depthStencil": depth_stencil,
            "blend": blend,
            "stencilReference": 0,
        },
        "textureLaneCount": len(lanes),
        "textureMask": (1 << len(lanes)) - 1,
        "textureLanes": lanes,
        "standardColor": {
            "packetVersion": 1,
            "baseRadianceLaneId": "lane.0",
            "baseRadianceChannel": lanes[0]["sourceChannel"],
            "coverageLaneId": "lane.1",
            "coverageChannel": lanes[1]["sourceChannel"],
            "emissiveMode": "baseRadiance",
            "lifetimeEnvelope": "carrierAlpha",
            "dissolveMode": "laneThreshold" if has_dissolve else "none",
            "dissolveLaneId": "lane.2" if has_dissolve else "",
            "dissolveChannel": (
                lanes[2]["sourceChannel"] if has_dissolve else "invalid"
            ),
            "dissolveSoftness": 0.1 if has_dissolve else 0.0,
            "missingLanePolicy": "failClosed",
        },
        "dynamicConsumedMask": 0,
        "dynamicSuppressedMask": 0,
        "particleColorPolicy": 0,
        "particleColorConsumedMask": 0,
        "particleColorSuppressedMask": 0,
        "scalarCount": 0,
        "vectorCount": 0,
        "inputCount": 0,
        "inputConsumedMask": [0, 0],
        "inputSuppressedMask": [0, 0],
        "vectorComponentConsumedMask": [0, 0, 0],
        "vectorComponentSuppressedMask": [0, 0, 0],
        "staticInputCount": 0,
        "staticSelectedMask": 0,
        "staticConsumedMask": 0,
        "staticSuppressedMask": 0,
        "renderInputCount": 0,
        "renderConsumedMask": 0,
        "renderSuppressedMask": 0,
        "scalars": [],
        "vectors": [],
        "artistParameters": [],
        "colors": [],
    }


def _carrier(element: dict[str, Any]) -> str:
    kind = element.get("kind")
    source_recipe = element.get("sourceRecipe")
    renderer_shape = (
        source_recipe.get("rendererShape")
        if isinstance(source_recipe, dict) and source_recipe.get("enabled") is True
        else ""
    )
    if kind == "decal" and renderer_shape == "decal":
        return "decal"
    if kind == "particle" and renderer_shape in ("sprite", "mesh"):
        return renderer_shape
    raise RepresentativeFourV1Error(
        f"unsupported representative carrier: {element.get('id')} ({kind}/{renderer_shape})"
    )


def _adapter_id(carrier: str, render_profile: str) -> str:
    adapter_id = ADAPTER_BY_CARRIER_PROFILE.get((carrier, render_profile))
    require(
        adapter_id is not None,
        f"no compiled adapter for {carrier}/{render_profile}",
    )
    return str(adapter_id)


def _target_resources(element: dict[str, Any], carrier: str) -> list[dict[str, str]]:
    if carrier != "mesh":
        return []
    rows = [
        copy.deepcopy(row)
        for row in element["resources"]
        if isinstance(row, dict) and row.get("slotId") == "meshModel"
    ]
    require(len(rows) == 1, f"mesh occurrence does not have exactly one meshModel: {element.get('id')}")
    require(
        set(rows[0]) == {"slotId", "assetId"} and rows[0]["assetId"],
        f"meshModel resource is invalid: {element.get('id')}",
    )
    return rows


def _descriptor_id(v1_effect_id: str, element_id: str) -> str:
    digest = hashlib.sha256(f"{v1_effect_id}\0{element_id}".encode("utf-8")).hexdigest()[:20]
    return f"effect.descriptor.representative-four-v1.{digest}.v1"


def _layout_id(texture_lanes: list[dict[str, Any]]) -> str:
    signature = [
        {
            "role": row["role"],
            "sourceChannel": row["sourceChannel"],
            "colorSpace": row["colorSpace"],
        }
        for row in texture_lanes
    ]
    digest = canonical_sha256(signature)[:16]
    return (
        f"effect.layout.standard-color-v1.{len(texture_lanes)}-lane."
        f"{digest}.v1"
    )


def _layout(execution: dict[str, Any]) -> dict[str, Any]:
    lanes = execution["textureLanes"]
    lane_count = len(lanes)
    require(lane_count in (2, 3), f"unsupported StandardColor lane count: {lane_count}")
    return {
        "layoutId": _layout_id(lanes),
        "executionVersion": 1,
        "textureLaneCount": lane_count,
        "textureMask": (1 << lane_count) - 1,
        "textureLanes": [
            {
                "laneId": row["laneId"],
                "role": row["role"],
                "textureRegister": row["textureRegister"],
                "samplerRegister": row["samplerRegister"],
                "sourceChannel": row["sourceChannel"],
                "colorSpace": row["colorSpace"],
            }
            for row in lanes
        ],
        "dynamicConsumedMask": 0,
        "dynamicSuppressedMask": 0,
        "particleColorPolicy": 0,
        "particleColorConsumedMask": 0,
        "particleColorSuppressedMask": 0,
        "scalarCount": 0,
        "vectorCount": 0,
        "inputCount": 0,
        "inputConsumedMask": [0, 0],
        "inputSuppressedMask": [0, 0],
        "vectorComponentConsumedMask": [0, 0, 0],
        "vectorComponentSuppressedMask": [0, 0, 0],
        "staticInputCount": 0,
        "staticSelectedMask": 0,
        "staticConsumedMask": 0,
        "staticSuppressedMask": 0,
        "renderInputCount": 0,
        "renderConsumedMask": 0,
        "renderSuppressedMask": 0,
        "scalarRows": [],
        "vectorRows": [],
        "artistParameterRows": [],
        "colorRows": [],
    }


def _descriptor(
    descriptor_id: str, layout_id: str, execution: dict[str, Any]
) -> dict[str, Any]:
    return {
        "descriptorId": descriptor_id,
        "layoutId": layout_id,
        "textureLanes": [
            {
                "laneId": row["laneId"],
                "assetId": row["assetId"],
                "sampler": copy.deepcopy(row["sampler"]),
            }
            for row in execution["textureLanes"]
        ],
        "scalars": [],
        "vectors": [],
        "artistParameters": [],
        "colors": [],
    }


def _non_material_projection(document: dict[str, Any]) -> dict[str, Any]:
    value = copy.deepcopy(document)
    value["effectAssetId"] = "effect.representative-four.identity-normalized"
    value["displayName"] = "effect.representative-four.identity-normalized"
    for element in value.get("elements", []):
        element.pop("material", None)
        element.pop("resources", None)
        # Authoring overrides address the replaced source-material/runtime packet.
        # They remain sealed in the V0 document and receipt, but cannot stay active
        # on the StandardColor sibling where their target parameters do not exist.
        element.pop("authoringOverrides", None)
    return value


def _resources_projection(document: dict[str, Any]) -> list[dict[str, Any]]:
    return [
        {"elementId": row["id"], "resources": copy.deepcopy(row["resources"])}
        for row in document["elements"]
    ]


def _assert_v1_invariants(
    source: dict[str, Any], target: dict[str, Any], spec: dict[str, Any]
) -> None:
    require(
        canonical_sha256(_non_material_projection(source))
        == canonical_sha256(_non_material_projection(target)),
        f"non-material occurrence state changed: {spec['v1']}",
    )
    require(
        [row["id"] for row in source["elements"]]
        == [row["id"] for row in target["elements"]],
        f"element ordering changed: {spec['v1']}",
    )
    for source_row, target_row in zip(source["elements"], target["elements"]):
        source_material = source_row["material"]
        target_material = target_row["material"]
        require(
            source_material.get("sourceMaterialPath")
            == target_material.get("sourceMaterialPath"),
            f"source material evidence changed: {spec['v1']}/{source_row['id']}",
        )
        require(
            source_material.get("renderProfile")
            == target_material.get("renderProfile"),
            f"render profile changed: {spec['v1']}/{source_row['id']}",
        )
        source_profile = copy.deepcopy(source_material.get("sourceProfile"))
        target_profile = copy.deepcopy(target_material.get("sourceProfile"))
        require(
            isinstance(source_profile, dict) and isinstance(target_profile, dict),
            f"source profile evidence is missing: {spec['v1']}/{source_row['id']}",
        )
        source_profile["enabled"] = False
        require(
            source_profile == target_profile,
            f"source profile evidence changed: {spec['v1']}/{source_row['id']}",
        )


def _validate_asset(root: Path, asset_id: str, label: str) -> dict[str, Any]:
    require(
        isinstance(asset_id, str)
        and asset_id
        and "\\" not in asset_id
        and not asset_id.startswith("/")
        and ".." not in Path(asset_id).parts,
        f"invalid runtime asset ID at {label}: {asset_id}",
    )
    path = root / "Client/Bin/Resources" / Path(asset_id)
    require(path.is_file(), f"runtime resource is missing at {label}: {asset_id}")
    payload = path.read_bytes()
    return {"assetId": asset_id, "bytes": len(payload), "sha256": sha256_bytes(payload)}


def _materialize_document(
    root: Path,
    spec: dict[str, Any],
    source: dict[str, Any],
) -> tuple[
    dict[str, Any],
    list[dict[str, Any]],
    list[dict[str, Any]],
    list[dict[str, Any]],
    list[dict[str, Any]],
]:
    target = copy.deepcopy(source)
    target["effectAssetId"] = spec["v1"]
    target["displayName"] = spec["v1"]
    descriptors: list[dict[str, Any]] = []
    bindings: list[dict[str, Any]] = []
    occurrence_rows: list[dict[str, Any]] = []
    layouts: list[dict[str, Any]] = []
    evidence_cache: dict[str, dict[str, Any]] = {}

    for source_row, target_row in zip(source["elements"], target["elements"]):
        element_id = source_row["id"]
        carrier = _carrier(source_row)
        material = source_row.get("material")
        require(isinstance(material, dict), f"material is missing: {spec['v0']}/{element_id}")
        render_profile = material.get("renderProfile")
        require(isinstance(render_profile, str), f"render profile is missing: {spec['v0']}/{element_id}")
        candidates = _texture_candidates(source_row)
        base = _select_candidate(
            candidates, ("base", "emissive", "mask", "noise", "dissolve", "any")
        )
        coverage = _select_candidate(
            candidates, ("mask", "base", "emissive", "noise", "dissolve", "any")
        )
        dissolve = _select_candidate(candidates, ("dissolve",))
        fallback = base is None or coverage is None
        if fallback:
            require(
                base is None and coverage is None and not candidates,
                f"partial lane selection unexpectedly required fallback: {spec['v0']}/{element_id}",
            )
            require(
                isinstance(spec["fallbackAssetId"], str)
                and spec["fallbackAssetId"],
                f"no admitted PROJECT_TUNED fallback: {spec['v0']}/{element_id}",
            )
            base = {
                "assetId": spec["fallbackAssetId"],
                "role": "base",
                "origin": "PROJECT_TUNED_DOMAIN_GLOW_FALLBACK",
            }
            coverage = copy.deepcopy(base)
        assert base is not None and coverage is not None

        selected = [base, coverage] + ([dissolve] if dissolve is not None else [])
        for index, row in enumerate(selected):
            _validate_asset(root, row["assetId"], f"{spec['v0']}/{element_id}/lane.{index}")

        base_policy = _lane_policy(
            root, source_row, base, "base_radiance", evidence_cache
        )
        coverage_policy = _lane_policy(
            root, source_row, coverage, "coverage", evidence_cache
        )
        dissolve_policy = (
            _lane_policy(root, source_row, dissolve, "dissolve", evidence_cache)
            if dissolve is not None
            else None
        )
        execution = _standard_color_execution(
            carrier,
            render_profile,
            (base, base_policy),
            (coverage, coverage_policy),
            (dissolve, dissolve_policy)
            if dissolve is not None and dissolve_policy is not None
            else None,
        )
        target_row["resources"] = _target_resources(source_row, carrier)
        target_material = copy.deepcopy(material)
        target_material["templateId"] = "effect.standard_color_v1"
        source_profile = target_material.get("sourceProfile")
        require(
            isinstance(source_profile, dict),
            f"sourceProfile evidence is missing: {spec['v0']}/{element_id}",
        )
        source_profile["enabled"] = False
        target_material["execution"] = execution
        target_row["material"] = target_material
        target_row.pop("authoringOverrides", None)

        descriptor_id = _descriptor_id(spec["v1"], element_id)
        layout = _layout(execution)
        layout_id = layout["layoutId"]
        adapter_id = _adapter_id(carrier, render_profile)
        layouts.append(layout)
        descriptors.append(_descriptor(descriptor_id, layout_id, execution))
        bindings.append({
            "effectAssetId": spec["v1"],
            "elementId": element_id,
            "programId": PROGRAM_ID,
            "layoutId": layout_id,
            "descriptorId": descriptor_id,
            "adapterId": adapter_id,
            "inlineMirrorPolicy": INLINE_MIRROR_REQUIRED,
        })
        occurrence_rows.append({
            "v0EffectAssetId": spec["v0"],
            "v1EffectAssetId": spec["v1"],
            "elementId": element_id,
            "fidelityClass": PROJECT_TUNED_APPROX,
            "sourceExact": False,
            "reasonCodes": [
                "SOURCE_GRAPH_NOT_EVALUATED_BY_STANDARD_COLOR_V1",
                "PROJECT_TUNED_DOMAIN_GLOW_FALLBACK" if fallback
                else "SOURCE_OCCURRENCE_TEXTURE_LANES_REUSED",
            ],
            "carrier": carrier,
            "renderProfile": render_profile,
            "adapterId": adapter_id,
            "descriptorId": descriptor_id,
            "sourceMaterialPath": material.get("sourceMaterialPath", ""),
            "sourceProfileCanonicalSha256": canonical_sha256(material["sourceProfile"]),
            "v0Resources": copy.deepcopy(source_row["resources"]),
            "v0ResourcesCanonicalSha256": canonical_sha256(source_row["resources"]),
            "v1Resources": copy.deepcopy(target_row["resources"]),
            "v1ResourcesCanonicalSha256": canonical_sha256(target_row["resources"]),
            "v0AuthoringOverrides": copy.deepcopy(
                source_row.get("authoringOverrides")
            ),
            "v0AuthoringOverridesCanonicalSha256": canonical_sha256(
                source_row.get("authoringOverrides")
            ),
            "baseRadiance": {
                **copy.deepcopy(base),
                **copy.deepcopy(base_policy),
            },
            "coverage": {
                **copy.deepcopy(coverage),
                **copy.deepcopy(coverage_policy),
            },
            "dissolve": (
                {
                    **copy.deepcopy(dissolve),
                    **copy.deepcopy(dissolve_policy),
                }
                if dissolve is not None and dissolve_policy is not None
                else None
            ),
        })

    _assert_v1_invariants(source, target, spec)
    return target, layouts, descriptors, bindings, occurrence_rows


def _build_catalog(catalog: dict[str, Any]) -> dict[str, Any]:
    require(
        catalog.get("formatVersion") == 1 and isinstance(catalog.get("effects"), list),
        "EffectCatalog identity is invalid",
    )
    target_ids = {spec["v1"] for spec in SPECS}
    rows = [
        copy.deepcopy(row)
        for row in catalog["effects"]
        if isinstance(row, dict) and row.get("effectAssetId") not in target_ids
    ]
    rows.extend(
        {
            "effectAssetId": spec["v1"],
            "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
            "authoringPath": f"Effects/Authored/{spec['v1']}.effect.json",
            "runtimeAdmission": REGISTRY_BOUND_AUDITION_ONLY,
            "fidelityClass": PROJECT_TUNED_APPROX,
            "sourceEffectAssetId": spec["v0"],
            "sourceDocumentRawSha256": spec["rawSha256"],
        }
        for spec in SPECS
    )
    rows.sort(key=lambda row: row["effectAssetId"])
    identities = [row["effectAssetId"] for row in rows]
    require(len(identities) == len(set(identities)), "EffectCatalog contains duplicate IDs")
    return {"formatVersion": 1, "effects": rows}


def _build_fragment(
    layouts: list[dict[str, Any]],
    descriptors: list[dict[str, Any]],
    bindings: list[dict[str, Any]],
) -> dict[str, Any]:
    require(len(descriptors) == 131, "representative descriptor denominator is not 131")
    require(len(bindings) == 131, "representative binding denominator is not 131")
    descriptors.sort(key=lambda row: row["descriptorId"])
    bindings.sort(key=lambda row: (row["effectAssetId"], row["elementId"]))
    require(
        len({row["descriptorId"] for row in descriptors}) == 131,
        "representative descriptor IDs are not unique",
    )
    layouts_by_id: dict[str, dict[str, Any]] = {}
    for layout in layouts:
        existing = layouts_by_id.get(layout["layoutId"])
        require(
            existing is None or existing == layout,
            f"layout identity collision: {layout['layoutId']}",
        )
        layouts_by_id[layout["layoutId"]] = layout
    return {
        "schema": "lostark.effect-material-program-registry-fragment",
        "formatVersion": 1,
        "domainId": DOMAIN_ID,
        "programs": [
            {"programId": PROGRAM_ID, "backend": "standardColorV1", "opcode": 1}
        ],
        "layouts": [layouts_by_id[key] for key in sorted(layouts_by_id)],
        "descriptors": descriptors,
        "bindings": bindings,
    }


def _document_receipt_row(
    spec: dict[str, Any], source_payload: bytes, source: dict[str, Any], target: dict[str, Any]
) -> dict[str, Any]:
    target_payload = pretty_bytes(target)
    source_non_material = canonical_sha256(_non_material_projection(source))
    target_non_material = canonical_sha256(_non_material_projection(target))
    require(source_non_material == target_non_material, f"non-material hash mismatch: {spec['v1']}")
    return {
        "v0": {
            "effectAssetId": spec["v0"],
            "path": source_document_path(spec["v0"]).as_posix(),
            "rawSha256": sha256_bytes(source_payload),
            "canonicalSha256": canonical_sha256(source),
            "nonMaterialInvariantSha256": source_non_material,
            "resourcesCanonicalSha256": canonical_sha256(_resources_projection(source)),
        },
        "v1": {
            "effectAssetId": spec["v1"],
            "path": source_document_path(spec["v1"]).as_posix(),
            "rawSha256": sha256_bytes(target_payload),
            "canonicalSha256": canonical_sha256(target),
            "nonMaterialInvariantSha256": target_non_material,
            "resourcesCanonicalSha256": canonical_sha256(_resources_projection(target)),
        },
        "elementCount": len(source["elements"]),
        "elementSequenceSha256": canonical_sha256(
            [row["id"] for row in source["elements"]]
        ),
    }


def build_outputs(root: Path) -> tuple[dict[Path, bytes], dict[str, Any]]:
    source_rows: list[tuple[dict[str, Any], bytes, dict[str, Any]]] = []
    for spec in SPECS:
        relative = source_document_path(spec["v0"])
        payload, document = load_json_bytes(root / relative)
        _validate_source_identity(spec, payload, document)
        source_rows.append((spec, payload, document))

    _, catalog = load_json_bytes(root / EFFECT_CATALOG)
    generated_documents: list[tuple[dict[str, Any], bytes, dict[str, Any], dict[str, Any]]] = []
    layouts: list[dict[str, Any]] = []
    descriptors: list[dict[str, Any]] = []
    bindings: list[dict[str, Any]] = []
    occurrences: list[dict[str, Any]] = []
    outputs: dict[Path, bytes] = {}

    for spec, source_payload, source in source_rows:
        (
            target,
            document_layouts,
            document_descriptors,
            document_bindings,
            document_occurrences,
        ) = _materialize_document(root, spec, source)
        outputs[source_document_path(spec["v1"])] = pretty_bytes(target)
        generated_documents.append((spec, source_payload, source, target))
        layouts.extend(document_layouts)
        descriptors.extend(document_descriptors)
        bindings.extend(document_bindings)
        occurrences.extend(document_occurrences)

    fragment = _build_fragment(layouts, descriptors, bindings)
    outputs[FRAGMENT_PATH] = pretty_bytes(fragment)
    generated_catalog = _build_catalog(catalog)
    outputs[EFFECT_CATALOG] = pretty_bytes(generated_catalog)

    fallback_count = sum(
        "PROJECT_TUNED_DOMAIN_GLOW_FALLBACK" in row["reasonCodes"]
        for row in occurrences
    )
    require(len(occurrences) == 131, "representative occurrence denominator is not 131")
    require(fallback_count == 11, "PROJECT_TUNED fallback denominator is not 11")
    require(
        all(row["fidelityClass"] == PROJECT_TUNED_APPROX for row in occurrences),
        "a representative occurrence is mislabeled as exact",
    )

    unique_assets = sorted({
        lane["assetId"]
        for row in occurrences
        for lane in (row["baseRadiance"], row["coverage"], row["dissolve"])
        if isinstance(lane, dict)
    })
    resource_cache: dict[str, dict[str, Any]] = {}
    resource_evidence = [
        _resource_evidence(root, asset_id, resource_cache)
        for asset_id in unique_assets
    ]
    base_channel_counts = {
        channel: sum(
            row["baseRadiance"]["sourceChannel"] == channel
            for row in occurrences
        )
        for channel in ("RGB", "R")
    }
    coverage_channel_counts = {
        channel: sum(
            row["coverage"]["sourceChannel"] == channel
            for row in occurrences
        )
        for channel in ("A", "R")
    }
    dissolve_channel_counts = {
        channel: sum(
            isinstance(row["dissolve"], dict)
            and row["dissolve"]["sourceChannel"] == channel
            for row in occurrences
        )
        for channel in ("A", "R")
    }
    base_color_space_counts = {
        color_space: sum(
            row["baseRadiance"]["colorSpace"] == color_space
            for row in occurrences
        )
        for color_space in ("srgb", "linear")
    }
    receipt: dict[str, Any] = {
        "schema": "lostark.effect-representative-four-v1-standard-color-receipt",
        "formatVersion": 1,
        "fidelityPolicy": {
            "sourceExactCount": 0,
            "projectTunedApproxCount": 131,
            "claim": PROJECT_TUNED_APPROX,
            "sourceGraphEvaluation": False,
        },
        "laneAdmissionPolicy": {
            "baseRadianceChannel": (
                "candidate role base/emissive or source sRGB -> RGB; "
                "otherwise R"
            ),
            "coverageAndDissolveChannel": (
                "decoded DDS alpha minimum < maximum -> A; otherwise R"
            ),
            "baseRadianceColorSpace": (
                "inherit a unique sourceProfile texture match; otherwise linear"
            ),
            "coverageAndDissolveColorSpace": "linear",
            "sampler": (
                "inherit unique sourceProfile addressU/addressV; linear filter "
                "and Texture2D addressW=wrap are PROJECT_TUNED policy"
            ),
            "baseRadianceChannelCounts": base_channel_counts,
            "coverageChannelCounts": coverage_channel_counts,
            "dissolveChannelCounts": dissolve_channel_counts,
            "baseRadianceColorSpaceCounts": base_color_space_counts,
        },
        "denominator": {
            "documents": 5,
            "occurrences": 131,
            "sprite": 98,
            "mesh": 25,
            "decal": 8,
            "fallback": 11,
        },
        "documents": [
            _document_receipt_row(spec, source_payload, source, target)
            for spec, source_payload, source, target in generated_documents
        ],
        "registryFragment": {
            "path": FRAGMENT_PATH.as_posix(),
            "programCount": 1,
            "layoutCount": len(fragment["layouts"]),
            "descriptorCount": 131,
            "bindingCount": 131,
            "canonicalSha256": canonical_sha256(fragment),
        },
        "resourceEvidence": resource_evidence,
        "occurrences": occurrences,
    }
    receipt["artifactSha256"] = canonical_sha256(receipt)
    outputs[RECEIPT_PATH] = pretty_bytes(receipt)
    return outputs, receipt


def _atomic_write(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary_name = ""
    try:
        with tempfile.NamedTemporaryFile(
            mode="wb", delete=False, dir=path.parent, prefix=f".{path.name}.", suffix=".tmp"
        ) as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
            temporary_name = stream.name
        os.replace(temporary_name, path)
    finally:
        if temporary_name and os.path.exists(temporary_name):
            os.unlink(temporary_name)


def run(root: Path, mode: str) -> tuple[bool, dict[str, Any]]:
    require(mode in ("check", "write"), f"unsupported mode: {mode}")
    outputs, receipt = build_outputs(root)
    changed = [
        relative
        for relative, payload in outputs.items()
        if not (root / relative).is_file() or (root / relative).read_bytes() != payload
    ]
    if mode == "write":
        for relative in sorted(changed, key=lambda path: path.as_posix()):
            _atomic_write(root / relative, outputs[relative])
    return bool(changed), receipt


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument(
        "--check", action="store_true", help="fail if checked-in outputs drift"
    )
    arguments = parser.parse_args()
    try:
        changed, receipt = run(
            arguments.root.resolve(), "check" if arguments.check else "write"
        )
    except RepresentativeFourV1Error as error:
        print(f"ERROR: {error}")
        return 1
    if arguments.check and changed:
        print("ERROR: representative-four V1 StandardColor outputs are stale")
        return 1
    print(
        "representative-four V1 StandardColor: "
        f"occurrences={receipt['denominator']['occurrences']} "
        f"fallback={receipt['denominator']['fallback']} "
        f"changed={'yes' if changed else 'no'}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
