#!/usr/bin/env python3
"""Preserve the first animation-joined Valtan family/carrier ledger.

The experiment preserves two one-element WATERTRAIL candidates as hashes and
source evidence only.  Carrier V1 physically cleared both retired authored
documents, so this tool must never repopulate them or create a second Product
draw.  The historical candidates exercised the same WATERTRAIL typed RT0
family through the ordinary document renderer:

* FRONT_BACK_FRONT Atk_01_02 sourceOrder 15, with its corrected sphere carrier.
* FOUR_SLASH source.7e08a4a792dbc4be1e1f, preserving its source recipe.

The legacy 60-row FRONT_BACK_FRONT windup document remains evidence on disk,
while its standalone Product cue and both canary Product admissions remain
retired.  The two authored paths are validated as ``elements: []`` evidence
shells.  An explicit
disposition ledger accounts for every row so an unsupported family can never
disappear merely to make the Product list smaller.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import shutil
import sys
import tempfile
from typing import Any, Iterable

import build_valtan_source_occurrence_inventory as source_inventory


ROOT = Path(__file__).resolve().parents[2]

FBF_SOURCE_PATH = PurePosixPath(
    "Data/Effects/Authored/effect.valtan.front-back-front.windup.effect.json"
)
FOUR_SLASH_SOURCE_PATH = PurePosixPath(
    "Data/Effects/Imported/Valtan/SafeReviewedGaps/"
    "effect.valtan.four-slash.active.clip-02.safe-gap-candidate.effect.json"
)
MATERIAL_EVIDENCE_PATH = PurePosixPath(
    "Data/Effects/Imported/Valtan/Valtan.source-material-evidence.json"
)
SOURCE_INVENTORY_PATH = PurePosixPath(
    "Data/Effects/Imported/Valtan/Valtan.source-occurrence-inventory.v1.json"
)
FOUR_SLASH_PROVENANCE_PATH = PurePosixPath(
    "Data/Effects/Imported/Valtan/SafeReviewedGaps/"
    "Valtan.safe-reviewed-gap-candidates.v1.json"
)
CATALOG_PATH = PurePosixPath("Data/Effects/EffectCatalog.json")
VALTAN_CUE_PATH = PurePosixPath(
    "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
MIGRATION_RECEIPT_PATH = PurePosixPath(
    "Data/Animation/Authored/Valtan/"
    "Valtan.pattern-occurrence-v2-migration.receipt.json"
)
FBF_EFFECT_ID = "effect.valtan.front-back-front.v1-watertrail-audition"
FOUR_SLASH_EFFECT_ID = "effect.valtan.four-slash.v1-watertrail-audition"
LEGACY_FBF_EFFECT_ID = "effect.valtan.front-back-front.windup"
FBF_OUTPUT_PATH = PurePosixPath(
    f"Data/Effects/Authored/{FBF_EFFECT_ID}.effect.json"
)
FOUR_SLASH_OUTPUT_PATH = PurePosixPath(
    f"Data/Effects/Authored/{FOUR_SLASH_EFFECT_ID}.effect.json"
)
LEDGER_PATH = PurePosixPath(
    "Data/Effects/Imported/Valtan/FrontBackFrontV1/"
    "Valtan.front-back-front-v1-selection-ledger.v1.json"
)
CARRIER_V1_RECEIPT_PATH = PurePosixPath(
    "Data/Effects/Imported/Valtan/CarrierV1/"
    "Valtan.carrier-v1-materialization-receipt.v1.json"
)
HISTORICAL_CARRIER_V1_RECEIPT_CANONICAL_SHA256 = (
    "a1a0515a6072c52097bef9ada0ab681fe8c8c46c4d64d902817d4e2e4e826e00"
)

FBF_SOURCE_ELEMENT_ID = "par_n_rpbf_atk_01_02.em15"
FOUR_SLASH_SOURCE_ELEMENT_ID = "source.7e08a4a792dbc4be1e1f"
SOURCE_SYSTEM_ID = "fx_mn_rpbf_00_n.par_n_rpbf_atk_01_02"
SOURCE_ORDER = 15

CHILD_MATERIAL = "fx_m_mi_m_00.fx_mi.fx_m_me_watertrail_01_46_tr"
PARENT_MATERIAL = "fx_m_mi_03.fx_m.fx_m_me_watertrail_01_tr"
PROFILE_ID = "ue3.material.fx.m.mi.03.fx.m.fx.m.me.watertrail.01.tr.afa4aeba0c50"
RUNTIME_PROFILE_ID = "effect.ue3.watertrail-01.v1"
MESH_ASSET_ID = "Effect/Valtan/Meshes/FX_SM_01/fm_m_sphere_006.wmodel"
BASE_ASSET_ID = "Effect/Valtan/Textures/FX_TEX_05/fx_m_wave_001_ycl.dds"
NOISE_ASSET_ID = "Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_030.dds"
MODEL_PRE_SCALE = 0.01
FBF_CUE_ID = "cue.valtan.front-back-front.v1-watertrail-audition"
FOUR_SLASH_CUE_ID = "cue.valtan.four-slash.v1-watertrail-audition"
LEGACY_FBF_CUE_ID = "cue.valtan.front-back-front.windup"
FBF_SOURCE_TIME_SECONDS = 3.214798927307129
FBF_SOURCE_START_MS = 3214
FBF_LOCAL_DELAY_SECONDS = FBF_SOURCE_TIME_SECONDS - (
    FBF_SOURCE_START_MS / 1000.0
)

DYNAMIC_SEMANTICS = [
    "water_alpha_pan",
    "water_noise_pan",
    "water_dissolve",
    "water_noise_strength",
]
LEDGER_DISPOSITIONS = {
    "V1_REQUIRED_VISIBLE",
    "V1_OUT_OF_SCOPE_NON_RT0",
    "USER_RETIRED",
    "BLOCKED_REQUIRED",
}
FBF_RETIREMENT_ROW = {
    "bindingId": LEGACY_FBF_CUE_ID,
    "occurrenceId": f"{LEGACY_FBF_CUE_ID}.occurrence.01",
    "reason": "LEGACY_CLIP_AGGREGATE_CARRIER_COLLAPSE",
    "replacementOwnerKind": "ANIMATION_EFFECT_CUE",
    "replacementBindingId": FBF_CUE_ID,
    "replacementOccurrenceId": f"{FBF_CUE_ID}.occurrence.01",
    "replacementEffectAssetId": FBF_EFFECT_ID,
    "effectAssetId": LEGACY_FBF_EFFECT_ID,
}


class MaterializationError(RuntimeError):
    """The source evidence cannot produce the sealed V1 audition outputs."""


def _require_object(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise MaterializationError(f"{label} must be an object")
    return value


def _require_list(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise MaterializationError(f"{label} must be an array")
    return value


def _require_string(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value:
        raise MaterializationError(f"{label} must be a non-empty string")
    return value


def _load_json(relative: PurePosixPath) -> dict[str, Any]:
    path = ROOT / Path(relative)
    try:
        payload = path.read_bytes()
    except OSError as exc:
        raise MaterializationError(f"cannot read {relative}: {exc}") from exc
    if payload.startswith(b"\xef\xbb\xbf"):
        raise MaterializationError(f"JSON must be UTF-8 without BOM: {relative}")
    try:
        value = json.loads(payload.decode("utf-8"))
    except (UnicodeError, json.JSONDecodeError) as exc:
        raise MaterializationError(f"cannot parse {relative}: {exc}") from exc
    return _require_object(value, str(relative))


def _json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    ).encode("utf-8")


def _canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def _find_element(document: dict[str, Any], element_id: str) -> dict[str, Any]:
    matches = [
        _require_object(row, f"elements[{index}]")
        for index, row in enumerate(_require_list(document.get("elements"), "elements"))
        if isinstance(row, dict) and row.get("id") == element_id
    ]
    if len(matches) != 1:
        raise MaterializationError(
            f"expected exactly one element {element_id}; found {len(matches)}"
        )
    return matches[0]


def _binding_map(element: dict[str, Any]) -> dict[str, str]:
    result: dict[str, str] = {}
    for index, raw in enumerate(_require_list(element.get("resources"), "resources")):
        row = _require_object(raw, f"resources[{index}]")
        slot = _require_string(row.get("slotId"), f"resources[{index}].slotId")
        asset = _require_string(row.get("assetId"), f"resources[{index}].assetId")
        if slot in result:
            raise MaterializationError(f"duplicate resource slot: {slot}")
        result[slot] = asset
    return result


def _find_material_evidence(evidence: dict[str, Any]) -> dict[str, Any]:
    found: list[dict[str, Any]] = []

    def walk(value: Any) -> None:
        if isinstance(value, list):
            for row in value:
                walk(row)
        elif isinstance(value, dict):
            if value.get("sourceMaterialPath") == CHILD_MATERIAL:
                found.append(value)
            for row in value.values():
                walk(row)

    walk(evidence)
    unique = {id(row): row for row in found}
    if len(unique) != 1:
        raise MaterializationError(
            f"expected one exact WATERTRAIL material row; found {len(unique)}"
        )
    row = next(iter(unique.values()))
    if row.get("parentMaterialPath") != PARENT_MATERIAL:
        raise MaterializationError("WATERTRAIL parent material drifted")
    if row.get("blockers") not in ([], None):
        raise MaterializationError("WATERTRAIL material evidence has blockers")
    return row


def _merged_named_values(
    parent_rows: Iterable[Any], instance_rows: Iterable[Any], *, vector: bool
) -> list[dict[str, Any]]:
    ordered: list[str] = []
    values: dict[str, dict[str, Any]] = {}

    def stage(raw: Any, label: str) -> None:
        row = _require_object(raw, label)
        name = _require_string(row.get("name"), f"{label}.name")
        group = row.get("group") if isinstance(row.get("group"), str) else ""
        value = row.get("value")
        if vector:
            source = _require_object(value, f"{label}.value")
            value = [source.get("r"), source.get("g"), source.get("b"), source.get("a")]
            if any(
                not isinstance(component, (int, float))
                or isinstance(component, bool)
                or not math.isfinite(float(component))
                for component in value
            ):
                raise MaterializationError(f"{label}.value must be finite float4")
        elif (
            not isinstance(value, (int, float))
            or isinstance(value, bool)
            or not math.isfinite(float(value))
        ):
            raise MaterializationError(f"{label}.value must be finite")
        if name not in values:
            ordered.append(name)
        values[name] = {"name": name, "value": value, "group": group}

    for index, raw in enumerate(parent_rows):
        stage(raw, f"parent[{index}]")
    for index, raw in enumerate(instance_rows):
        stage(raw, f"instance[{index}]")
    return [values[name] for name in ordered]


def _build_source_profile(material: dict[str, Any]) -> dict[str, Any]:
    parent = _require_object(material.get("parentDeclaration"), "parentDeclaration")
    scalars = _merged_named_values(
        _require_list(parent.get("collectedScalarParameters"), "parent scalars"),
        _require_list(material.get("instanceScalars"), "instance scalars"),
        vector=False,
    )
    vectors = _merged_named_values(
        _require_list(parent.get("collectedVectorParameters"), "parent vectors"),
        _require_list(material.get("instanceVectors"), "instance vectors"),
        vector=True,
    )

    texture_assets = {
        "maintex": BASE_ASSET_ID,
        "uv_noise_tex": NOISE_ASSET_ID,
    }
    textures: list[dict[str, Any]] = []
    seen: set[str] = set()
    for index, raw in enumerate(
        _require_list(material.get("instanceTextures"), "instance textures")
    ):
        row = _require_object(raw, f"instanceTextures[{index}]")
        name = _require_string(row.get("name"), f"instanceTextures[{index}].name")
        if name not in texture_assets or name in seen:
            raise MaterializationError(f"unexpected/duplicate WATERTRAIL texture: {name}")
        source = _require_string(
            row.get("texture"), f"instanceTextures[{index}].texture"
        )
        textures.append(
            {
                "name": name,
                "sourceObjectPath": source,
                "assetId": texture_assets[name],
                "addressU": "wrap",
                "addressV": "wrap",
                "colorSpace": "linear",
                "samplingEvidence": "legacy_default",
                "group": "maintex" if name == "maintex" else "uv_noise",
            }
        )
        seen.add(name)
    if seen != set(texture_assets):
        raise MaterializationError("WATERTRAIL named texture closure is not 2/2")

    return {
        "enabled": True,
        "profileId": PROFILE_ID,
        "runtimeShaderProfileId": RUNTIME_PROFILE_ID,
        "parentMaterialPath": PARENT_MATERIAL,
        "semanticStatus": "reconstructed_profile",
        "scalars": scalars,
        "vectors": vectors,
        "staticSwitches": [],
        "dynamicParameterSemantics": list(DYNAMIC_SEMANTICS),
        "subUVMode": "none",
        "textures": textures,
    }


def _validate_shared_carrier(element: dict[str, Any], label: str) -> None:
    if element.get("kind") != "particle":
        raise MaterializationError(f"{label} must remain a particle")
    bindings = _binding_map(element)
    expected = {
        "meshModel": MESH_ASSET_ID,
        "base": BASE_ASSET_ID,
        "noise": NOISE_ASSET_ID,
    }
    if bindings != expected:
        raise MaterializationError(f"{label} resource contract drifted: {bindings}")
    material = _require_object(element.get("material"), f"{label}.material")
    if (
        material.get("templateId") != "effect.source_material"
        or material.get("sourceMaterialPath") != CHILD_MATERIAL
        or material.get("renderProfile") != "alpha_two_sided_depth_read"
    ):
        raise MaterializationError(f"{label} material contract drifted")
    profile = _require_object(material.get("sourceProfile"), f"{label}.sourceProfile")
    if (
        profile.get("enabled") is not True
        or profile.get("profileId") != PROFILE_ID
        or profile.get("runtimeShaderProfileId") != RUNTIME_PROFILE_ID
        or profile.get("parentMaterialPath") != PARENT_MATERIAL
        or profile.get("dynamicParameterSemantics") != DYNAMIC_SEMANTICS
    ):
        raise MaterializationError(f"{label} typed family profile drifted")
    names = [row.get("name") for row in profile.get("textures", [])]
    if sorted(names) != ["maintex", "uv_noise_tex"]:
        raise MaterializationError(f"{label} named texture lanes drifted")
    detail = _require_object(element.get("detail"), f"{label}.detail")
    mesh = _require_object(detail.get("mesh"), f"{label}.detail.mesh")
    particle = _require_object(detail.get("particle"), f"{label}.detail.particle")
    if mesh.get("modelPreScale") != MODEL_PRE_SCALE:
        raise MaterializationError(f"{label} modelPreScale must remain 0.01")
    if particle.get("billboard") is not False:
        raise MaterializationError(f"{label} mesh particle must remain non-billboard")
    recipe = _require_object(element.get("sourceRecipe"), f"{label}.sourceRecipe")
    if recipe.get("rendererShape") != "mesh":
        raise MaterializationError(f"{label} source renderer shape must be mesh")


def _build_document(
    source: dict[str, Any], effect_id: str, display_name: str, element: dict[str, Any]
) -> dict[str, Any]:
    document = {
        "schema": source.get("schema"),
        "version": source.get("version"),
        "effectAssetId": effect_id,
        "displayName": display_name,
        "particleSystem": copy.deepcopy(source.get("particleSystem")),
        "modelCues": [],
        "elements": [element],
    }
    if document["schema"] != "lostark.effect-authoring" or document["version"] != 13:
        raise MaterializationError("source Effect document schema/version drifted")
    return document


def _catalog_entry(effect_id: str) -> dict[str, Any]:
    return {
        "effectAssetId": effect_id,
        "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
        "authoringPath": f"Effects/Authored/{effect_id}.effect.json",
    }


def _upsert_exact_rows(
    rows: list[Any],
    expected_rows: list[dict[str, Any]],
    *,
    identity_key: str,
    label: str,
) -> list[dict[str, Any]]:
    staged = [
        copy.deepcopy(_require_object(row, f"{label}[{index}]"))
        for index, row in enumerate(rows)
    ]
    for expected in expected_rows:
        identity = expected[identity_key]
        matches = [row for row in staged if row.get(identity_key) == identity]
        if len(matches) > 1:
            raise MaterializationError(f"duplicate {label} identity: {identity}")
        if matches and matches[0] != expected:
            raise MaterializationError(f"existing {label} row drifted: {identity}")
        if not matches:
            staged.append(copy.deepcopy(expected))
    staged.sort(key=lambda row: _require_string(row.get(identity_key), identity_key))
    return staged


def _without_identities(
    rows: list[Any], identities: set[str], *, identity_key: str, label: str
) -> list[dict[str, Any]]:
    staged = [
        copy.deepcopy(_require_object(row, f"{label}[{index}]"))
        for index, row in enumerate(rows)
    ]
    found: dict[str, int] = {identity: 0 for identity in identities}
    result: list[dict[str, Any]] = []
    for row in staged:
        identity = _require_string(row.get(identity_key), identity_key)
        if identity in identities:
            found[identity] += 1
            continue
        result.append(row)
    duplicated = [identity for identity, count in found.items() if count > 1]
    if duplicated:
        raise MaterializationError(
            f"duplicate retired {label} identities: {sorted(duplicated)}"
        )
    return result


def _build_catalog(source: dict[str, Any]) -> dict[str, Any]:
    if source.get("formatVersion") != 1 or tuple(source) != (
        "formatVersion",
        "effects",
    ):
        raise MaterializationError("Effect catalog header or property order drifted")
    catalog = copy.deepcopy(source)
    retained = _without_identities(
        _require_list(source.get("effects"), "catalog effects"),
        {LEGACY_FBF_EFFECT_ID, FBF_EFFECT_ID, FOUR_SLASH_EFFECT_ID},
        identity_key="effectAssetId",
        label="catalog effect",
    )
    catalog["effects"] = _upsert_exact_rows(
        retained,
        [_catalog_entry(FBF_EFFECT_ID)],
        identity_key="effectAssetId",
        label="catalog effect",
    )
    return catalog


def _cue_row(
    *,
    cue_id: str,
    pattern_id: str,
    action_id: str,
    clip_occurrence_id: str,
    effect_id: str,
    source_start_ms: int,
) -> dict[str, Any]:
    return {
        "bindingId": cue_id,
        "occurrenceId": f"{cue_id}.occurrence.01",
        "patternId": pattern_id,
        "stageId": "SMASHES" if pattern_id == "VALTAN_FRONT_BACK_FRONT" else "SLASHES",
        "actionId": action_id,
        "clipOccurrenceId": clip_occurrence_id,
        "effectAssetId": effect_id,
        "anchorSlotId": "root",
        "followPolicy": "follow",
        "stopPolicy": "natural",
        "repeatPolicy": "once",
        "sourceStartMs": source_start_ms,
        "sourceEndMs": None,
        "localTransform": {
            "position": [0, 0, 0],
            "rotationDegrees": [0, 0, 0],
            "scale": [1, 1, 1],
        },
    }


def _product_cues() -> list[dict[str, Any]]:
    return [
        _cue_row(
            cue_id=FBF_CUE_ID,
            pattern_id="VALTAN_FRONT_BACK_FRONT",
            action_id="valtan.attack.front-back-front.active",
            clip_occurrence_id="valtan.attack.front-back-front.active.clip.01",
            effect_id=FBF_EFFECT_ID,
            source_start_ms=FBF_SOURCE_START_MS,
        ),
    ]


def _build_valtan_cues(source: dict[str, Any]) -> dict[str, Any]:
    if (
        source.get("schema") != "lostark.valtan-pattern-effect-cues"
        or source.get("formatVersion") != 2
        or source.get("ownerArchetypeId") != "BOSS_VALTAN"
        or tuple(source) != ("schema", "formatVersion", "ownerArchetypeId", "cues")
    ):
        raise MaterializationError("Valtan pattern Effect cue header drifted")
    cues = copy.deepcopy(source)
    retained = _without_identities(
        _require_list(source.get("cues"), "Valtan cues"),
        {LEGACY_FBF_CUE_ID, FBF_CUE_ID, FOUR_SLASH_CUE_ID},
        identity_key="bindingId",
        label="Valtan cue",
    )
    cues["cues"] = _upsert_exact_rows(
        retained,
        _product_cues(),
        identity_key="bindingId",
        label="Valtan cue",
    )
    return cues


def _build_migration_receipt(source: dict[str, Any]) -> dict[str, Any]:
    if (
        source.get("schema")
        != "lostark.valtan-pattern-occurrence-v2-migration-receipt"
        or source.get("formatVersion") != 2
    ):
        raise MaterializationError("Valtan occurrence migration receipt drifted")
    baseline = _require_object(source.get("baselineIdentity"), "baselineIdentity")
    baseline_rows = _require_list(baseline.get("cues"), "baselineIdentity.cues")
    matches = [
        row
        for row in baseline_rows
        if isinstance(row, dict)
        and row.get("occurrenceId") == FBF_RETIREMENT_ROW["occurrenceId"]
    ]
    if (
        len(matches) != 1
        or matches[0].get("bindingId") != LEGACY_FBF_CUE_ID
        or matches[0].get("effectAssetId") != LEGACY_FBF_EFFECT_ID
    ):
        raise MaterializationError("legacy FBF baseline cue identity drifted")
    receipt = copy.deepcopy(source)
    receipt["retiredBaselineCues"] = _upsert_exact_rows(
        _require_list(source.get("retiredBaselineCues"), "retiredBaselineCues"),
        [FBF_RETIREMENT_ROW],
        identity_key="occurrenceId",
        label="retired baseline cue",
    )
    return receipt


def _fbf_inventory_carriers(
    inventory: dict[str, Any], fbf_source: dict[str, Any]
) -> dict[str, dict[str, Any]]:
    systems = {
        _require_string(row.get("sourceSystemId"), "sourceSystemId"): row
        for row in (
            _require_object(raw, f"sourceSystems[{index}]")
            for index, raw in enumerate(
                _require_list(inventory.get("sourceSystems"), "sourceSystems")
            )
        )
    }
    result: dict[str, dict[str, Any]] = {}
    for index, raw in enumerate(_require_list(fbf_source.get("elements"), "FBF elements")):
        element = _require_object(raw, f"FBF elements[{index}]")
        element_id = _require_string(element.get("id"), f"FBF elements[{index}].id")
        local_system, separator, order_text = element_id.rpartition(".em")
        if not separator or not order_text.isdigit():
            raise MaterializationError(f"FBF element source identity is invalid: {element_id}")
        matching_systems = [
            (source_system_id, system)
            for source_system_id, system in systems.items()
            if source_system_id.endswith(f".{local_system}")
        ]
        if len(matching_systems) != 1:
            raise MaterializationError(
                f"FBF element source system join is not exact: {element_id}"
            )
        source_system_id, system = matching_systems[0]
        source_order = int(order_text)
        carriers = [
            _require_object(carrier, f"{source_system_id}.carriers")
            for carrier in _require_list(system.get("carriers"), "carriers")
            if isinstance(carrier, dict) and carrier.get("sourceOrder") == source_order
        ]
        if len(carriers) != 1:
            raise MaterializationError(
                f"FBF element carrier join is not exact: {element_id}"
            )
        carrier = carriers[0]
        result[element_id] = {
            "sourceSystemId": source_system_id,
            "sourceOrder": source_order,
            "carrierKey": _require_string(
                carrier.get("carrierKey"), f"{element_id}.carrierKey"
            ),
            "rendererShape": _require_string(
                carrier.get("rendererShape"), f"{element_id}.rendererShape"
            ),
            "inventoryKind": _require_string(
                carrier.get("kind"), f"{element_id}.kind"
            ),
            "conversionStatus": _require_string(
                carrier.get("conversionStatus"), f"{element_id}.conversionStatus"
            ),
            "sourceRecipeSha256": _require_string(
                carrier.get("sourceRecipeSha256"), f"{element_id}.sourceRecipeSha256"
            ),
            "carrier": carrier,
        }
    if len(result) != 60:
        raise MaterializationError("FBF inventory carrier join must seal exactly 60 rows")
    return result


def _find_fbf_occurrence(inventory: dict[str, Any]) -> dict[str, Any]:
    matches = [
        _require_object(row, f"occurrences[{index}]")
        for index, row in enumerate(
            _require_list(inventory.get("occurrences"), "occurrences")
        )
        if isinstance(row, dict)
        and row.get("patternId") == "VALTAN_FRONT_BACK_FRONT"
        and row.get("semanticStageId") == "SMASHES"
        and row.get("gameplayActionId") == "valtan.attack.front-back-front.active"
        and row.get("clipOccurrenceId")
        == "valtan.attack.front-back-front.active.clip.01"
        and row.get("sourceSystemId") == SOURCE_SYSTEM_ID
        and isinstance(row.get("sourceTimeSeconds"), (int, float))
        and math.isclose(
            float(row["sourceTimeSeconds"]),
            FBF_SOURCE_TIME_SECONDS,
            rel_tol=0.0,
            abs_tol=1e-12,
        )
    ]
    if len(matches) != 1:
        raise MaterializationError("FBF WATERTRAIL source occurrence join drifted")
    occurrence = matches[0]
    source_time = float(occurrence["sourceTimeSeconds"])
    if math.floor(source_time * 1000.0) != FBF_SOURCE_START_MS:
        raise MaterializationError("FBF WATERTRAIL cue floor projection drifted")
    if not math.isclose(
        source_time,
        FBF_SOURCE_START_MS / 1000.0 + FBF_LOCAL_DELAY_SECONDS,
        rel_tol=0.0,
        abs_tol=1e-12,
    ):
        raise MaterializationError("FBF WATERTRAIL local delay projection drifted")
    return occurrence


def _find_four_slash_provenance(
    provenance: dict[str, Any], inventory: dict[str, Any]
) -> tuple[dict[str, Any], dict[str, Any]]:
    matches = [
        _require_object(row, f"coreProjections[{index}]")
        for index, row in enumerate(
            _require_list(provenance.get("coreProjections"), "coreProjections")
        )
        if isinstance(row, dict) and row.get("id") == FOUR_SLASH_SOURCE_ELEMENT_ID
    ]
    if len(matches) != 1:
        raise MaterializationError("Four Slash WATERTRAIL occurrence join drifted")
    row = matches[0]
    if (
        row.get("occurrenceFullKey")
        != "occurrence-key.2ac495461f6ed2b9c91ee9c22a254930dc82ea13cd0dd8425cc68173318fffbf"
        or row.get("sourceSystemId")
        != "fx_mn_rpbf_00_o.par_o_rpbf_atk_01_08"
        or row.get("sourceTimeSeconds") != 0.6000000238418579
        or row.get("clipOccurrenceId")
        != "valtan.attack.four-slash.active.clip.02"
    ):
        raise MaterializationError("Four Slash WATERTRAIL occurrence identity drifted")
    systems = [
        _require_object(system, f"sourceSystems[{index}]")
        for index, system in enumerate(
            _require_list(inventory.get("sourceSystems"), "sourceSystems")
        )
        if isinstance(system, dict)
        and system.get("sourceSystemId") == row.get("sourceSystemId")
    ]
    if len(systems) != 1:
        raise MaterializationError("Four Slash WATERTRAIL source system drifted")
    carriers = [
        _require_object(carrier, "Four Slash carrier")
        for carrier in _require_list(systems[0].get("carriers"), "carriers")
        if isinstance(carrier, dict) and carrier.get("carrierKey") == row.get("carrierKey")
    ]
    if len(carriers) != 1:
        raise MaterializationError("Four Slash WATERTRAIL carrier join drifted")
    carrier = carriers[0]
    if (
        carrier.get("sourceOrder") != 1
        or carrier.get("rendererShape") != "mesh"
        or carrier.get("kind") != "particle"
    ):
        raise MaterializationError("Four Slash WATERTRAIL carrier identity drifted")
    return row, carrier


def _build_ledger(
    fbf_source: dict[str, Any],
    four_source_element: dict[str, Any],
    carrier_rows: dict[str, dict[str, Any]],
    fbf_occurrence: dict[str, Any],
    four_provenance: dict[str, Any],
    four_carrier: dict[str, Any],
    fbf_document: dict[str, Any],
    four_document: dict[str, Any],
    successor_contract: dict[str, Any],
    empty_shells: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    rows: list[dict[str, Any]] = []
    for index, raw in enumerate(_require_list(fbf_source.get("elements"), "FBF elements")):
        element = _require_object(raw, f"FBF elements[{index}]")
        element_id = _require_string(element.get("id"), f"FBF elements[{index}].id")
        material = _require_object(element.get("material"), f"{element_id}.material")
        material_path = _require_string(
            material.get("sourceMaterialPath"), f"{element_id}.sourceMaterialPath"
        )
        bindings = _binding_map(element)
        carrier_row = carrier_rows.get(element_id)
        if carrier_row is None:
            raise MaterializationError(f"FBF ledger carrier is missing: {element_id}")
        if element_id == FBF_SOURCE_ELEMENT_ID:
            disposition = "V1_REQUIRED_VISIBLE"
            reason = "FIRST_SHARED_WATERTRAIL_FAMILY_CANARY"
        else:
            disposition = "BLOCKED_REQUIRED"
            reason = "NOT_YET_ADMITTED_BY_A_PERSISTENT_TYPED_FAMILY_PACKET"
        rows.append(
            {
                "sourceIndex": index,
                "elementId": element_id,
                "sourceMaterialPath": material_path,
                "legacyKind": element.get("kind"),
                "legacyMeshAssetId": bindings.get("meshModel"),
                "sourceSystemId": carrier_row["sourceSystemId"],
                "sourceOrder": carrier_row["sourceOrder"],
                "carrierKey": carrier_row["carrierKey"],
                "rendererShape": carrier_row["rendererShape"],
                "inventoryKind": carrier_row["inventoryKind"],
                "conversionStatus": carrier_row["conversionStatus"],
                "sourceRecipeSha256": carrier_row["sourceRecipeSha256"],
                "disposition": disposition,
                "reason": reason,
            }
        )
    if len(rows) != 60 or len({row["elementId"] for row in rows}) != 60:
        raise MaterializationError("FBF legacy denominator must remain exactly 60/60")
    if any(row["disposition"] not in LEDGER_DISPOSITIONS for row in rows):
        raise MaterializationError("FBF ledger contains an unknown disposition")

    counts = {
        name: sum(row["disposition"] == name for row in rows)
        for name in sorted(LEDGER_DISPOSITIONS)
    }
    if counts != {
        "BLOCKED_REQUIRED": 59,
        "USER_RETIRED": 0,
        "V1_OUT_OF_SCOPE_NON_RT0": 0,
        "V1_REQUIRED_VISIBLE": 1,
    }:
        raise MaterializationError(f"FBF first-slice disposition counts drifted: {counts}")

    normalized_four = copy.deepcopy(four_source_element)
    _require_object(normalized_four["detail"], "Four Slash detail").setdefault(
        "mesh", {}
    )["modelPreScale"] = MODEL_PRE_SCALE
    return {
        "schema": "lostark.valtan-front-back-front-v1-selection-ledger",
        "formatVersion": 1,
        "productMutation": False,
        "visualAdmission": "LEDGER_ONLY_NO_PHYSICAL_AUDITION",
        "policy": {
            "legacyAggregateProductAdmission": False,
            "standaloneWaterTrailProductAdmission": False,
            "silentDropAllowed": False,
            "automaticMaterialOrMeshDedupAllowed": False,
            "nativeVfOrFullMrtRequiredForV1": False,
        },
        "sharedFamily": {
            "childMaterialPath": CHILD_MATERIAL,
            "parentMaterialPath": PARENT_MATERIAL,
            "profileId": PROFILE_ID,
            "runtimeShaderProfileId": RUNTIME_PROFILE_ID,
            "carrierKind": "MeshParticle",
            "meshAssetId": MESH_ASSET_ID,
            "modelPreScale": MODEL_PRE_SCALE,
            "namedTextureLanes": ["maintex", "uv_noise_tex"],
            "dynamicParameterSemantics": list(DYNAMIC_SEMANTICS),
            "renderProfile": "alpha_two_sided_depth_read",
        },
        "canaries": [
            {
                "role": "FRONT_BACK_FRONT_FAMILY_AUDITION",
                "effectAssetId": FBF_EFFECT_ID,
                "authoringPath": FBF_OUTPUT_PATH.as_posix(),
                "physicalDocumentDisposition": (
                    "ELEMENTS_CLEARED_EVIDENCE_SHELL"
                ),
                "physicalDocumentCanonicalSha256": empty_shells[
                    FBF_EFFECT_ID
                ]["canonicalSha256"],
                "sourceElementId": FBF_SOURCE_ELEMENT_ID,
                "sourceOccurrenceId": fbf_occurrence.get("occurrenceId"),
                "sourceOccurrenceFullKey": fbf_occurrence.get("fullKey"),
                "sourceSystemId": SOURCE_SYSTEM_ID,
                "sourceCarrierKey": carrier_rows[FBF_SOURCE_ELEMENT_ID]["carrierKey"],
                "sourceOrder": SOURCE_ORDER,
                "sourceTimeSeconds": fbf_occurrence.get("sourceTimeSeconds"),
                "productCueId": None,
                "productAdmission": "AUTHORING_ONLY_SUCCESSOR_OWNS_CLIP",
                "clipOccurrenceId": "valtan.attack.front-back-front.active.clip.01",
                "sourceStartMs": FBF_SOURCE_START_MS,
                "elementLocalDelaySeconds": FBF_LOCAL_DELAY_SECONDS,
                "historicalCandidateElementSha256": _canonical_sha256(
                    fbf_document["elements"][0]
                ),
                "historicalCandidateDocumentSha256": _canonical_sha256(
                    fbf_document
                ),
            },
            {
                "role": "FOUR_SLASH_REUSE_AUDITION",
                "effectAssetId": FOUR_SLASH_EFFECT_ID,
                "authoringPath": FOUR_SLASH_OUTPUT_PATH.as_posix(),
                "physicalDocumentDisposition": (
                    "ELEMENTS_CLEARED_EVIDENCE_SHELL"
                ),
                "physicalDocumentCanonicalSha256": empty_shells[
                    FOUR_SLASH_EFFECT_ID
                ]["canonicalSha256"],
                "sourceElementId": FOUR_SLASH_SOURCE_ELEMENT_ID,
                "sourceOccurrenceFullKey": four_provenance.get("occurrenceFullKey"),
                "sourceSystemId": four_provenance.get("sourceSystemId"),
                "sourceCarrierKey": four_provenance.get("carrierKey"),
                "sourceOrder": four_carrier.get("sourceOrder"),
                "sourceRecipeSha256": four_carrier.get("sourceRecipeSha256"),
                "productCueId": None,
                "productAdmission": "AUTHORING_ONLY_SUCCESSOR_OWNS_CLIP",
                "clipOccurrenceId": "valtan.attack.four-slash.active.clip.02",
                "sourceStartMs": 0,
                "normalizedSourceElementSha256": _canonical_sha256(normalized_four),
                "historicalCandidateElementSha256": _canonical_sha256(
                    four_document["elements"][0]
                ),
                "historicalCandidateDocumentSha256": _canonical_sha256(
                    four_document
                ),
            },
        ],
        "carrierV1Successor": copy.deepcopy(successor_contract),
        "rows": rows,
        "summary": {
            "sourceRowCount": len(rows),
            "accountedRowCount": len(rows),
            "resolvedDecisionCount": len(rows) - counts["BLOCKED_REQUIRED"],
            "blockedRequiredCount": counts["BLOCKED_REQUIRED"],
            "unknownRowCount": 0,
            "failedButRemovedCount": 0,
            "dispositionCounts": counts,
            "auditionDocumentCount": 0,
            "emptyEvidenceShellCount": 2,
            "historicalWitnessElementCount": 2,
            "productCatalogAddedCount": 0,
            "productCatalogRetiredCount": 0,
            "productCueAddedCount": 0,
            "productCueRetiredCount": 0,
            "duplicateCueSuppressedCount": 2,
            "carrierV1SuccessorOwnerCount": 2,
            "historicalStandaloneOwnerRetiredCount": 2,
        },
    }


def _validate_carrier_v1_successor(
    receipt: dict[str, Any],
    catalog: dict[str, Any],
    cues: dict[str, Any],
    fbf_occurrence: dict[str, Any],
    fbf_carrier: dict[str, Any],
    four_provenance: dict[str, Any],
    four_carrier: dict[str, Any],
) -> dict[str, Any]:
    if (
        receipt.get("schema")
        != "lostark.valtan-carrier-v1-materialization-receipt"
        or receipt.get("formatVersion") != 1
        or receipt.get("bossArchetypeId") != "BOSS_VALTAN"
    ):
        raise MaterializationError("Carrier V1 successor receipt header drifted")
    summary = _require_object(receipt.get("summary"), "Carrier V1 summary")
    reset = _require_object(receipt.get("productReset"), "Carrier V1 productReset")
    if (
        summary.get("reviewedCoreProjectionCount") != 660
        or summary.get("materializedProjectionCount") != 657
        or reset.get("duplicateClipOccurrenceOwnerCount") != 0
        or reset.get("nonExactOldBossRootSurvivorCount") != 0
    ):
        raise MaterializationError("Carrier V1 successor denominator drifted")

    output_receipt = _require_object(receipt.get("outputs"), "Carrier V1 outputs")
    catalog_output = _require_object(output_receipt.get("catalog"), "catalog output")
    cue_output = _require_object(output_receipt.get("cues"), "cue output")
    # Preserve the one-shot Carrier V1 preimage seal.  Append-only catalog and
    # pattern-master cue successors are not part of that historical hash; the
    # two exact live WATERTRAIL owners are joined and checked below.
    if (
        catalog_output.get("path") != CATALOG_PATH.as_posix()
        or catalog_output.get("scope") != "EFFECT_ASSET_ID_PREFIX"
        or catalog_output.get("effectAssetIdPrefix") != "effect.valtan."
        or catalog_output.get("effectCount") != 46
        or catalog_output.get("canonicalSha256")
        != "123c070157e743ef467294607f104a9e5f1d90c3c99f73b6cf9c48033da093da"
        or cue_output.get("path") != VALTAN_CUE_PATH.as_posix()
        or cue_output.get("cueCount") != 44
        or cue_output.get("canonicalSha256")
        != "4ff3c88cffdbe84abb99aaee22aad86c92f1b1797dfd8706058ded489b738dc9"
    ):
        raise MaterializationError("Carrier V1 historical Product output seal drifted")

    effect_ids = {
        _require_string(row.get("effectAssetId"), "catalog effectAssetId")
        for row in catalog["effects"]
    }
    cue_ids = {
        _require_string(row.get("bindingId"), "cue bindingId")
        for row in cues["cues"]
    }
    retired_effects = {LEGACY_FBF_EFFECT_ID, FBF_EFFECT_ID, FOUR_SLASH_EFFECT_ID}
    retired_cues = {LEGACY_FBF_CUE_ID, FBF_CUE_ID, FOUR_SLASH_CUE_ID}
    if effect_ids.intersection(retired_effects) or cue_ids.intersection(retired_cues):
        raise MaterializationError(
            "standalone WATERTRAIL predecessor was restored to Product"
        )

    expected_owners = {
        "valtan.attack.front-back-front.active.clip.01": (
            "effect.valtan.carrier-v1.attack.front-back-front.active.clip-01",
            "cue.valtan.carrier-v1.attack.front-back-front.active.clip-01",
        ),
        "valtan.attack.four-slash.active.clip.02": (
            "effect.valtan.carrier-v1.attack.four-slash.active.clip-02",
            "cue.valtan.carrier-v1.attack.four-slash.active.clip-02",
        ),
    }
    groups = {
        str(row.get("clipOccurrenceId") or ""): row
        for row in _require_list(receipt.get("clipGroups"), "Carrier V1 clipGroups")
        if isinstance(row, dict)
        and row.get("clipOccurrenceId") in expected_owners
    }
    if set(groups) != set(expected_owners):
        raise MaterializationError("Carrier V1 WATERTRAIL clip owner is missing")
    for clip_id, (effect_id, cue_id) in expected_owners.items():
        group = groups[clip_id]
        if (
            group.get("effectAssetId") != effect_id
            or group.get("cueBindingId") != cue_id
            or effect_id not in effect_ids
            or cue_id not in cue_ids
        ):
            raise MaterializationError(
                f"Carrier V1 WATERTRAIL clip owner drifted: {clip_id}"
            )

    fbf_full_key = (
        f"{fbf_occurrence.get('fullKey')}|{fbf_carrier.get('carrierKey')}"
    )
    fbf_rows = [
        _require_object(raw, "Carrier V1 reviewed projection")
        for raw in _require_list(
            receipt.get("reviewedProjectionLedger"),
            "Carrier V1 reviewedProjectionLedger",
        )
        if isinstance(raw, dict)
        and raw.get("occurrenceFullKey") == fbf_occurrence.get("fullKey")
        and raw.get("carrierKey") == fbf_carrier.get("carrierKey")
    ]
    if (
        len(fbf_rows) != 1
        or fbf_rows[0].get("clipOccurrenceId")
        != "valtan.attack.front-back-front.active.clip.01"
        or fbf_rows[0].get("rendererShape") != "mesh"
        or fbf_rows[0].get("productAdmission") is not False
        or CHILD_MATERIAL not in (fbf_rows[0].get("materialObjectPaths") or [])
    ):
        raise MaterializationError("Carrier V1 FBF WATERTRAIL blocker drifted")

    four_full_key = (
        f"{four_provenance.get('occurrenceFullKey')}|{four_carrier.get('carrierKey')}"
    )
    four_rows = [
        _require_object(raw, "Carrier V1 source element")
        for raw in _require_list(receipt.get("sourceElements"), "Carrier V1 sourceElements")
        if isinstance(raw, dict) and raw.get("fullSourceKey") == four_full_key
    ]
    if (
        len(four_rows) != 1
        or four_rows[0].get("clipOccurrenceId")
        != "valtan.attack.four-slash.active.clip.02"
        or four_rows[0].get("effectAssetId")
        != expected_owners["valtan.attack.four-slash.active.clip.02"][0]
        or four_rows[0].get("rendererShape") != "mesh"
        or four_rows[0].get("originalMaterial", {}).get("sourceMaterialPath")
        != CHILD_MATERIAL
    ):
        raise MaterializationError("Carrier V1 Four Slash WATERTRAIL source drifted")

    mappings = {
        str(row.get("retiredBindingId") or ""): row
        for row in _require_list(
            receipt.get("retiredOwnerSuccessorMappings"),
            "Carrier V1 retired owner mappings",
        )
        if isinstance(row, dict)
    }
    fbf_mapping = mappings.get(FBF_CUE_ID)
    if (
        fbf_mapping is None
        or fbf_mapping.get("disposition")
        != "REPLACED_BY_EXACT_CARRIER_V1_CLIP_OWNER"
        or fbf_mapping.get("replacementEffectAssetId")
        != expected_owners["valtan.attack.front-back-front.active.clip.01"][0]
    ):
        raise MaterializationError("standalone FBF WATERTRAIL retirement drifted")

    return {
        "receiptPath": CARRIER_V1_RECEIPT_PATH.as_posix(),
        # The WATERTRAIL ledger is an immutable witness of the Carrier V1
        # receipt revision it was authored against.  Later receipt overlays are
        # validated through their exact rows above and do not rewrite history.
        "receiptCanonicalSha256": (
            HISTORICAL_CARRIER_V1_RECEIPT_CANONICAL_SHA256
        ),
        "standaloneProductAdmission": False,
        "duplicateClipOccurrenceOwnerCount": 0,
        "owners": [
            {
                "clipOccurrenceId": "valtan.attack.front-back-front.active.clip.01",
                "effectAssetId": expected_owners[
                    "valtan.attack.front-back-front.active.clip.01"
                ][0],
                "cueBindingId": expected_owners[
                    "valtan.attack.front-back-front.active.clip.01"
                ][1],
                "sourceNode": None,
                "fullSourceKey": fbf_full_key,
                "sourceAdmission": "BLOCKED_UNRESOLVED_RUNTIME_ADAPTER_WITNESS",
            },
            {
                "clipOccurrenceId": "valtan.attack.four-slash.active.clip.02",
                "effectAssetId": expected_owners[
                    "valtan.attack.four-slash.active.clip.02"
                ][0],
                "cueBindingId": expected_owners[
                    "valtan.attack.four-slash.active.clip.02"
                ][1],
                "sourceNode": four_rows[0]["sourceNode"],
                "fullSourceKey": four_rows[0]["fullSourceKey"],
                "sourceAdmission": "MATERIALIZED_EXACT_CARRIER_V1",
            },
        ],
    }


def _validate_empty_evidence_shell(
    relative: PurePosixPath, effect_asset_id: str
) -> dict[str, Any]:
    document = _load_json(relative)
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != effect_asset_id
        or document.get("elements") != []
        or not isinstance(document.get("modelCues"), list)
    ):
        raise MaterializationError(
            f"retired WATERTRAIL evidence shell regained rows: {relative}"
        )
    return {
        "path": relative.as_posix(),
        "effectAssetId": effect_asset_id,
        "elementCount": 0,
        "canonicalSha256": source_inventory.canonical_sha256(document),
    }


def build_outputs() -> dict[PurePosixPath, bytes]:
    fbf_source = _load_json(FBF_SOURCE_PATH)
    four_source = _load_json(FOUR_SLASH_SOURCE_PATH)
    evidence = _find_material_evidence(_load_json(MATERIAL_EVIDENCE_PATH))
    source_inventory = _load_json(SOURCE_INVENTORY_PATH)
    carrier_rows = _fbf_inventory_carriers(source_inventory, fbf_source)
    fbf_occurrence = _find_fbf_occurrence(source_inventory)
    carrier = carrier_rows[FBF_SOURCE_ELEMENT_ID]["carrier"]
    four_provenance, four_carrier = _find_four_slash_provenance(
        _load_json(FOUR_SLASH_PROVENANCE_PATH), source_inventory
    )
    profile = _build_source_profile(evidence)

    four_source_element = _find_element(four_source, FOUR_SLASH_SOURCE_ELEMENT_ID)
    if four_source_element.get("sourceNode") != four_provenance.get("sourceNode"):
        raise MaterializationError("Four Slash WATERTRAIL source node drifted")
    if four_source_element.get("material", {}).get("sourceMaterialPath") != CHILD_MATERIAL:
        raise MaterializationError("Four Slash WATERTRAIL child material drifted")
    source_bindings = _binding_map(four_source_element)
    if source_bindings != {
        "meshModel": MESH_ASSET_ID,
        "base": BASE_ASSET_ID,
        "noise": NOISE_ASSET_ID,
    }:
        raise MaterializationError("Four Slash WATERTRAIL source resources drifted")

    four_element = copy.deepcopy(four_source_element)
    four_element["id"] = "v1-four-slash-watertrail-reuse"
    four_element["displayName"] = "V1 / Four Slash / WATERTRAIL reuse"
    four_element["groupId"] = "v1-watertrail-reuse"
    four_element["material"] = {
        "templateId": "effect.source_material",
        "sourceMaterialPath": CHILD_MATERIAL,
        "renderProfile": "alpha_two_sided_depth_read",
        "sourceProfile": copy.deepcopy(profile),
    }
    four_detail = _require_object(four_element.get("detail"), "Four Slash detail")
    _require_object(four_detail.get("mesh"), "Four Slash mesh")[
        "modelPreScale"
    ] = MODEL_PRE_SCALE
    _require_object(four_detail.get("particle"), "Four Slash particle")[
        "billboard"
    ] = False

    fbf_source_element = _find_element(fbf_source, FBF_SOURCE_ELEMENT_ID)
    if fbf_source_element.get("material", {}).get("sourceMaterialPath") != CHILD_MATERIAL:
        raise MaterializationError("FBF WATERTRAIL child material drifted")
    fbf_element = copy.deepcopy(fbf_source_element)
    fbf_element["id"] = "v1-front-back-front-watertrail"
    fbf_element["displayName"] = "V1 / Front Back Front / WATERTRAIL"
    fbf_element["groupId"] = "v1-watertrail-reuse"
    fbf_element["sourceNode"] = carrier.get("carrierKey")
    fbf_element["resources"] = copy.deepcopy(four_element["resources"])
    fbf_element["material"] = copy.deepcopy(four_element["material"])
    fbf_detail = _require_object(fbf_element.get("detail"), "FBF detail")
    fbf_timing = _require_object(fbf_detail.get("timing"), "FBF timing")
    fbf_timing["startDelaySeconds"] = FBF_LOCAL_DELAY_SECONDS
    _require_object(fbf_detail.get("mesh"), "FBF mesh")["modelPreScale"] = (
        MODEL_PRE_SCALE
    )
    _require_object(fbf_detail.get("particle"), "FBF particle")["billboard"] = False
    fbf_element["sourceRecipe"] = {
        "enabled": False,
        "rendererShape": "mesh",
        "emitterDelaySeconds": 0.0,
        "emitterDurationSeconds": 0.0,
        "emitterLoopCount": 1,
        "bursts": [],
        "modules": [],
    }

    _validate_shared_carrier(fbf_element, "FBF WATERTRAIL audition")
    _validate_shared_carrier(four_element, "Four Slash WATERTRAIL audition")

    fbf_document = _build_document(
        fbf_source,
        FBF_EFFECT_ID,
        "Valtan / Front Back Front / V1 WATERTRAIL Audition",
        fbf_element,
    )
    four_document = _build_document(
        four_source,
        FOUR_SLASH_EFFECT_ID,
        "Valtan / Four Slash / V1 WATERTRAIL Reuse Audition",
        four_element,
    )
    successor_contract = _validate_carrier_v1_successor(
        _load_json(CARRIER_V1_RECEIPT_PATH),
        _load_json(CATALOG_PATH),
        _load_json(VALTAN_CUE_PATH),
        fbf_occurrence,
        carrier,
        four_provenance,
        four_carrier,
    )
    empty_shells = {
        FBF_EFFECT_ID: _validate_empty_evidence_shell(
            FBF_OUTPUT_PATH, FBF_EFFECT_ID
        ),
        FOUR_SLASH_EFFECT_ID: _validate_empty_evidence_shell(
            FOUR_SLASH_OUTPUT_PATH, FOUR_SLASH_EFFECT_ID
        ),
    }
    ledger = _build_ledger(
        fbf_source,
        four_source_element,
        carrier_rows,
        fbf_occurrence,
        four_provenance,
        four_carrier,
        fbf_document,
        four_document,
        successor_contract,
        empty_shells,
    )

    return {
        LEDGER_PATH: _json_bytes(ledger),
    }


def _check(outputs: dict[PurePosixPath, bytes]) -> None:
    stale: list[str] = []
    for relative, expected in outputs.items():
        path = ROOT / Path(relative)
        try:
            actual = path.read_bytes()
        except OSError:
            stale.append(f"{relative}:missing")
            continue
        if actual != expected:
            stale.append(f"{relative}:drift")
    if stale:
        raise MaterializationError("V1 WATERTRAIL outputs are stale: " + ", ".join(stale))


def _write(outputs: dict[PurePosixPath, bytes]) -> None:
    transaction = Path(tempfile.mkdtemp(prefix=".valtan-watertrail-v1.", dir=ROOT))
    staged_root = transaction / "staged"
    backup_root = transaction / "backup"
    originals: dict[PurePosixPath, bytes | None] = {}
    promoted: list[PurePosixPath] = []
    try:
        for relative, payload in outputs.items():
            target = ROOT / Path(relative)
            originals[relative] = target.read_bytes() if target.is_file() else None
            staged = staged_root / Path(relative)
            staged.parent.mkdir(parents=True, exist_ok=True)
            staged.write_bytes(payload)
            if originals[relative] is not None:
                backup = backup_root / Path(relative)
                backup.parent.mkdir(parents=True, exist_ok=True)
                backup.write_bytes(originals[relative] or b"")
        for relative in outputs:
            target = ROOT / Path(relative)
            current = target.read_bytes() if target.is_file() else None
            if current != originals[relative]:
                raise MaterializationError(f"output changed after staging: {relative}")
        for relative in outputs:
            target = ROOT / Path(relative)
            target.parent.mkdir(parents=True, exist_ok=True)
            os.replace(staged_root / Path(relative), target)
            promoted.append(relative)
    except Exception as exc:
        rollback_errors: list[str] = []
        for relative in reversed(promoted):
            target = ROOT / Path(relative)
            try:
                if originals[relative] is None:
                    target.unlink(missing_ok=True)
                else:
                    os.replace(backup_root / Path(relative), target)
            except OSError as rollback_exc:
                rollback_errors.append(f"{relative}: {rollback_exc}")
        if rollback_errors:
            raise MaterializationError(
                "materialization failed and rollback was incomplete: "
                + "; ".join(rollback_errors)
            ) from exc
        if isinstance(exc, MaterializationError):
            raise
        raise MaterializationError(
            f"materialization failed; all outputs rolled back: {exc}"
        ) from exc
    finally:
        shutil.rmtree(transaction, ignore_errors=True)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--write", action="store_true")
    args = parser.parse_args(argv)
    try:
        outputs = build_outputs()
        if args.check:
            _check(outputs)
            label = "checked"
        else:
            _write(outputs)
            label = "written"
    except (OSError, MaterializationError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1
    print(
        "Valtan WATERTRAIL family/carrier canaries "
        f"{label}: outputs={len(outputs)}, productMutation=0, "
        "standaloneProductOwners=0, carrierV1SuccessorOwners=2, "
        "emptyEvidenceShells=2, ledgerRows=60"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
