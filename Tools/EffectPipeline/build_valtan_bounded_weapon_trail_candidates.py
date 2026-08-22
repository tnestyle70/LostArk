#!/usr/bin/env python3
"""Build three isolated, sequence-independent Valtan weapon Trail candidates.

The source Trail occurrence of each target clip has no exact baked-edge history.
This builder therefore does not claim source-exact reconstruction.  It reuses the
protected Whirlwind Par_N_MRHG_Trail_01 three-emitter visual family and attaches
it to a target-specific runtime slot that resolves Valtan's b_wp_r_01 bone.  All
new rows are PROJECT_TUNED / BOUNDED_RECONSTRUCTION.

Only candidate artifacts are writable here.  Product cues, animation bindings,
the Effect catalog, canonical Effect documents, and the Whirlwind canary are
read-only evidence.
"""

from __future__ import annotations

import argparse
import copy
from dataclasses import dataclass
import hashlib
import importlib.util
import json
import os
from pathlib import Path, PurePosixPath
import sys
import tempfile
from typing import Any


SCRIPT_PATH = Path(__file__).resolve()
DEFAULT_REPOSITORY_ROOT = SCRIPT_PATH.parents[2]


def _load_sibling_module(name: str, path: Path):
    specification = importlib.util.spec_from_file_location(name, path)
    if specification is None or specification.loader is None:
        raise RuntimeError(f"could not load {path}")
    module = importlib.util.module_from_spec(specification)
    sys.modules[name] = module
    specification.loader.exec_module(module)
    return module


wmodel_evidence = _load_sibling_module(
    "build_valtan_whirlwind_effect_canary_for_bounded_weapon_trails",
    SCRIPT_PATH.with_name("build_valtan_whirlwind_effect_canary.py"),
)


class CandidateError(RuntimeError):
    """The bounded reconstruction inputs no longer match reviewed evidence."""


@dataclass(frozen=True)
class Target:
    pattern_id: str
    stage_id: str
    action_id: str
    clip_occurrence_id: str
    cue_binding_id: str
    cue_row_sha256: str
    action_binding_sha256: str
    effect_asset_id: str
    catalog_row_sha256: str
    canonical_relative_path: PurePosixPath
    baseline_raw_sha256: str
    baseline_canonical_sha256: str
    baseline_element_count: int
    adapter_target_id: str
    adapter_row_sha256: str
    source_notify_id: str
    source_action_id: int
    source_stage_index: int
    source_time_seconds: float
    source_duration_seconds: float
    runtime_anchor_slot_id: str
    candidate_filename: str
    element_prefix: str


TARGETS = (
    Target(
        pattern_id="VALTAN_WHIRLWIND",
        stage_id="RECOVERY",
        action_id="valtan.attack.whirlwind.recovery",
        clip_occurrence_id="valtan.attack.whirlwind.recovery.clip.01",
        cue_binding_id="cue.valtan.whirlwind.recovery",
        cue_row_sha256="c6b6f9ba96156bb7418779cdda42576f24d717e59caf34757e463ebe9bf682ea",
        action_binding_sha256="e071fd5eed915090a95db3903a525b3a37c946987b5addd2ce62cf719d82fe77",
        effect_asset_id="effect.valtan.whirlwind.recovery",
        catalog_row_sha256="f4a37c77c075db5479526ba96deb15f311feea1935fe8519353d64b0108fcc04",
        canonical_relative_path=PurePosixPath(
            "Data/Effects/Authored/effect.valtan.whirlwind.recovery.effect.json"
        ),
        baseline_raw_sha256="917809de2525fd8a72f351ee7e1ef8328aac9cca395a670aafceaf781ae31865",
        baseline_canonical_sha256="692cd461485a9d5054e723a2d06318a6d0074aba8ac8e071bdc1fc1678cb7cf0",
        baseline_element_count=11,
        adapter_target_id="valtan.trail-adapter.09a79d299c00a5c089793d30",
        adapter_row_sha256="6d1e6f97423631b808ddc0355fad752c6eea962ce9c4d53c12ffc028f8cc44ec",
        source_notify_id="action-420633/stage-003/notify-001",
        source_action_id=420633,
        source_stage_index=3,
        source_time_seconds=0.0,
        source_duration_seconds=0.1965470016002655,
        runtime_anchor_slot_id="VALTAN_WHIRLWIND_RECOVERY_WEAPON_R",
        candidate_filename=(
            "effect.valtan.whirlwind.recovery.weapon-bone-trails."
            "candidate.effect.json"
        ),
        element_prefix="project-valtan-whirlwind-recovery-weapon-trail",
    ),
    Target(
        pattern_id="VALTAN_GROUND_WAVE_SMASH",
        stage_id="WINDUP",
        action_id="valtan.attack.ground-wave-smash.windup",
        clip_occurrence_id="valtan.attack.ground-wave-smash.windup.clip.01",
        cue_binding_id="cue.valtan.ground-wave-smash.windup",
        cue_row_sha256="8c2f8e189cc00c3b525731ff642a363665caea1707f3d29a5327dae7ae0582c8",
        action_binding_sha256="957dd8573f35e78d83c92f2bbc1454682efafb2c3a2c8a1d8ea7a49ce0224ba0",
        effect_asset_id="effect.valtan.ground-wave-smash.windup",
        catalog_row_sha256="dfdf3ec0c6c114c2cc280fd2ab4ac4175baa8ddb3e11584732e0bdebeab78ab4",
        canonical_relative_path=PurePosixPath(
            "Data/Effects/Authored/effect.valtan.ground-wave-smash.windup.effect.json"
        ),
        baseline_raw_sha256="1eecf319f6dc4b46facff9abba688f00af5160a39690a541a6eb33e1c7311387",
        baseline_canonical_sha256="adf215987b06509dbd3c49e53a3b9c38b39714fc0c15a81ebb429025a82ff5d5",
        baseline_element_count=13,
        adapter_target_id="valtan.trail-adapter.28064c18382b4b2ca03b4324",
        adapter_row_sha256="b30f1e26b1d615903b90dbe9b8241164beca833e725f76867bb34a5ff7a5359d",
        source_notify_id="action-420615/stage-030/notify-005",
        source_action_id=420615,
        source_stage_index=30,
        source_time_seconds=0.394663006067276,
        source_duration_seconds=0.47657299041748047,
        runtime_anchor_slot_id="VALTAN_GROUND_WAVE_SMASH_WINDUP_WEAPON_R",
        candidate_filename=(
            "effect.valtan.ground-wave-smash.windup.weapon-bone-trails."
            "candidate.effect.json"
        ),
        element_prefix="project-valtan-ground-wave-smash-windup-weapon-trail",
    ),
    Target(
        pattern_id="VALTAN_JUMP_SPIN",
        stage_id="RECOVERY",
        action_id="valtan.attack.jump-spin.recovery",
        clip_occurrence_id="valtan.attack.jump-spin.recovery.clip.01",
        cue_binding_id="cue.valtan.jump-spin.recovery",
        cue_row_sha256="27074999d44db9c3c3f06eca1f9b153ec0b963745a83d75d8a29f0b3ddbf8d14",
        action_binding_sha256="e115bb0018eb64be32e7ad1075c97f72ba4b6862ac58a259ad6fa306c43c8bde",
        effect_asset_id="effect.valtan.jump-spin.recovery",
        catalog_row_sha256="df28e1aafbcbf0c4513675375eb7a5535268b3e6c7cb8121939b301fbd93cd3c",
        canonical_relative_path=PurePosixPath(
            "Data/Effects/Authored/effect.valtan.jump-spin.recovery.effect.json"
        ),
        baseline_raw_sha256="217279b25b47880f4ea79219e8a9b328abe7acf0e74296d78ef0714d3e2c28e9",
        baseline_canonical_sha256="aeade82de9227d98d5d562e5345e9cfbc9d2a441e510e6fd99d9d628c40e3b11",
        baseline_element_count=8,
        adapter_target_id="valtan.trail-adapter.f03638659d51c5f61dbfa4d4",
        adapter_row_sha256="ebdb66d641d03c56f7a467fc42d90951f4e988361e07ceb83fcfa3371c74df66",
        source_notify_id="action-420621/stage-016/notify-004",
        source_action_id=420621,
        source_stage_index=16,
        source_time_seconds=0.0,
        source_duration_seconds=0.1965470016002655,
        runtime_anchor_slot_id="VALTAN_JUMP_SPIN_RECOVERY_WEAPON_R",
        candidate_filename=(
            "effect.valtan.jump-spin.recovery.weapon-bone-trails."
            "candidate.effect.json"
        ),
        element_prefix="project-valtan-jump-spin-recovery-weapon-trail",
    ),
)

OUTPUT_ROOT = PurePosixPath(
    "Data/Effects/Imported/Valtan/ProjectTunedBoundedWeaponTrails"
)
MANIFEST_FILENAME = "Valtan.bounded-weapon-trails.candidate-manifest.v1.json"
RECEIPT_RELATIVE_PATH = OUTPUT_ROOT / PurePosixPath(
    "Valtan.bounded-weapon-trails.application-receipt.v1.json"
)

SOURCE_DOCUMENT_RELATIVE_PATH = PurePosixPath(
    "Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json"
)
SOURCE_DOCUMENT_RAW_SHA256 = (
    "e94ddfd463da840962ddf8d8a0b604af4ef833925c2ac1cfd0f8e25522a5bea7"
)
SOURCE_DOCUMENT_CANONICAL_SHA256 = (
    "95c233b9fb9ded959a5527381f5ae0bd1a85b8c97c5a0a6b8341f92364ecd68a"
)
SOURCE_FAMILY_NAME = "FX_BS_01.Trail.Par_N_MRHG_Trail_01"
SOURCE_ELEMENTS = (
    (
        "valtan.420633.notify004.emitter5259",
        "4a61835c8fb70fd70036fd7f2b20edd3a07aee094c3e608d8da2ed8e5536fc6c",
        "04c5f4ddb0d7b6ec3135ed54773bfbdc08c9e7522d94f3f6f929b14913313264",
        "d47e73e6c7ed42e2826efe8f2a5edebf46e8d60ada40d0a2f70b4d387dc1c906",
        "2cdb50b6c44c1487fe79465cea8d4efa7b3efe2e9fd240e2bf5ea8a19240963c",
        "emitter5259",
    ),
    (
        "valtan.420633.notify004.emitter5260",
        "4a129d852250495d10a5aa28389101107fb7fd31a74fd7278eaf6dd76f6f58a8",
        "a2c565da76ede057101a9315badbdb8ec2ead0d5f697e02417d4bcbe77fc91a7",
        "9d5d502c66ed1466a95aff7225b183278c4fb4bffdde3fad33ed19eb7c94d6a6",
        "2cdb50b6c44c1487fe79465cea8d4efa7b3efe2e9fd240e2bf5ea8a19240963c",
        "emitter5260",
    ),
    (
        "valtan.420633.notify004.emitter5258",
        "8cfb762bc056f97e1ea1a99cb0c6632dd40a124dcb0915309dfa63c3abde9a5f",
        "fa9a57d8361da379c1d3964b92d3f894d2563aef3b701fae764d15f733a34260",
        "8b4436253210bcbd5312461cccfc90c36b7cf46ec6eca9eb03b8be505b6830fe",
        "2cdb50b6c44c1487fe79465cea8d4efa7b3efe2e9fd240e2bf5ea8a19240963c",
        "emitter5258",
    ),
)

BODY_MODEL_ASSET_ID = "Character/Valtan/MN_RPBF_01.wmodel"
BODY_MODEL_SHA256 = (
    "227191781b035b9aa41d60cc8c49bf1e8b2f67a749111e4afe8f6f3c997b2011"
)
RUNTIME_BONE_NAME = "b_wp_r_01"
EXPECTED_BONE_INDEX = 54
EXPECTED_BONE_PARENT_INDEX = 41
EXPECTED_BONE_NAME_HASH = "51bf337724f3b4ca"

EXPECTED_RESOURCE_EVIDENCE = {
    "Effect/Valtan/Textures/FX_TEX_00/fx_a_trail_011.dds": (32896, "ecf526f2f5849525d341e20549652736ec79ef173bdf65226f5aa314e6c89df9"),
    "Effect/Valtan/Textures/FX_TEX_01/fx_c_noise_009.dds": (32896, "d9b2b59b2657fcfe333852ef2580492fcd1e7769af2d8d5096be47050cddc65e"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_006.dds": (8320, "0eaf36a6b923cb1fdf6958716bf35598639251454b65aa90ba52a8ced7d2ee48"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_009.dds": (32896, "58a957a8d13005c12afb097c6c8d423d2c94872dfe8b76ab1d51dabbfe67749b"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_028.dds": (8320, "0cda1d83c6df738528a3e32c45fd72f76a6acddf0010c76170032ae4e3469933"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_067.dds": (32896, "8264b8a2671205799a428e244fd2068ab1300efdea0cb0f3a504e1a0c5b6e4dd"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_decal_023.dds": (65664, "57b6cf34817250e634094deda330e6360d31c5b03b289fb61e607f6067052427"),
    "Effect/Valtan/Textures/FX_TEX_02/fx_d_noise_009.dds": (8320, "d5e63e9f023fb9f9ffb5dc2d32f80319102b91c94c5fe7c951177d161f963c9e"),
    "Effect/Valtan/Textures/FX_TEX_03/fx_a_trail_003_ycl.dds": (32896, "21a6683bdd504b6c1e63b006632dd962b5be4a519b56654fff2916b29a7ab8b0"),
    "Effect/Valtan/Textures/FX_TEX_03/fx_e_fluid_003.dds": (65664, "3339f6fd17e588fed6c11082a8a93a9b6dc55c49cb20f2a956dd6258f675a83f"),
}

PATTERN_BINDINGS_RELATIVE_PATH = PurePosixPath(
    "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
)
PATTERN_CUES_RELATIVE_PATH = PurePosixPath(
    "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
)
EFFECT_CATALOG_RELATIVE_PATH = PurePosixPath("Data/Effects/EffectCatalog.json")
ADAPTER_RELATIVE_PATH = PurePosixPath(
    "Data/Effects/Imported/Valtan/Valtan.trail-adapter-packets.v1.json"
)


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def raw_sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def raw_sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=2, sort_keys=False) + "\n"
    ).encode("utf-8")


def repository_path(root: Path, relative: PurePosixPath) -> Path:
    resolved_root = root.resolve()
    candidate = resolved_root.joinpath(*relative.parts).resolve()
    if candidate != resolved_root and resolved_root not in candidate.parents:
        raise CandidateError(f"repository-relative path escaped root: {relative}")
    return candidate


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise CandidateError(f"could not read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise CandidateError(f"JSON root must be an object: {path}")
    return value


def verify_seal(value: dict[str, Any], field: str, label: str) -> None:
    expected = value.get(field)
    clone = copy.deepcopy(value)
    clone.pop(field, None)
    if not isinstance(expected, str) or canonical_sha256(clone) != expected:
        raise CandidateError(f"{label} {field} is stale")


def disabled_source_recipe() -> dict[str, Any]:
    return {
        "enabled": False,
        "rendererShape": "",
        "emitterDelaySeconds": 0,
        "emitterDurationSeconds": 0,
        "emitterLoopCount": 1,
        "bursts": [],
        "modules": [],
    }


def candidate_element_ids(target: Target) -> tuple[str, ...]:
    return tuple(f"{target.element_prefix}.{row[5]}" for row in SOURCE_ELEMENTS)


def protected_element_contract(element: dict[str, Any]) -> dict[str, Any]:
    detail = element.get("detail") or {}
    return {
        "id": element.get("id"),
        "groupId": element.get("groupId"),
        "sourceNode": element.get("sourceNode"),
        "visible": element.get("visible"),
        "kind": element.get("kind"),
        "resources": element.get("resources"),
        "material": element.get("material"),
        "actionCueAttachment": element.get("actionCueAttachment"),
        "transformInheritance": element.get("transformInheritance"),
        "initialTiming": detail.get("timing"),
        "initialTrailGeometry": detail.get("trail"),
        "sourceRecipe": element.get("sourceRecipe"),
        "sourcePresentation": element.get("sourcePresentation"),
    }


def _exact_rows(
    rows: Any, key: str, expected: str, label: str
) -> dict[str, Any]:
    matches = [
        row
        for row in rows or []
        if isinstance(row, dict) and row.get(key) == expected
    ]
    if len(matches) != 1:
        raise CandidateError(f"{label} is missing or duplicated: {expected}")
    return matches[0]


def _validate_product_joins(root: Path) -> list[dict[str, Any]]:
    bindings = read_json(repository_path(root, PATTERN_BINDINGS_RELATIVE_PATH))
    cues = read_json(repository_path(root, PATTERN_CUES_RELATIVE_PATH))
    catalog = read_json(repository_path(root, EFFECT_CATALOG_RELATIVE_PATH))
    evidence: list[dict[str, Any]] = []
    for target in TARGETS:
        binding = _exact_rows(
            bindings.get("bindings"), "actionId", target.action_id, "action binding"
        )
        cue = _exact_rows(cues.get("cues"), "bindingId", target.cue_binding_id, "cue")
        catalog_row = _exact_rows(
            catalog.get("effects"), "effectAssetId", target.effect_asset_id, "catalog row"
        )
        expected_catalog_path = target.canonical_relative_path.as_posix().replace(
            "Data/", "", 1
        )
        if (
            canonical_sha256(binding) != target.action_binding_sha256
            or canonical_sha256(cue) != target.cue_row_sha256
            or canonical_sha256(catalog_row) != target.catalog_row_sha256
            or cue.get("patternId") != target.pattern_id
            or cue.get("stageId") != target.stage_id
            or cue.get("actionId") != target.action_id
            or cue.get("clipOccurrenceId") != target.clip_occurrence_id
            or cue.get("effectAssetId") != target.effect_asset_id
            or cue.get("anchorSlotId") != "root"
            or cue.get("followPolicy") != "follow"
            or catalog_row.get("payloadKind") != "DIRECT_AUTHORED_DOCUMENT_V13"
            or catalog_row.get("authoringPath") != expected_catalog_path
            or len(binding.get("clips") or []) != 1
            or binding["clips"][0].get("clipOccurrenceId")
            != target.clip_occurrence_id
        ):
            raise CandidateError(
                f"target Product cue/binding/catalog join changed: {target.pattern_id}"
            )
        source_end_ms = cue.get("sourceEndMs")
        if source_end_ms is not None and (
            target.source_time_seconds + target.source_duration_seconds
        ) * 1000.0 > float(source_end_ms) + 0.001:
            raise CandidateError(
                f"target Trail timing exceeds cue boundary: {target.pattern_id}"
            )
        evidence.append(
            {
                "patternId": target.pattern_id,
                "stageId": target.stage_id,
                "actionId": target.action_id,
                "clipOccurrenceId": target.clip_occurrence_id,
                "cueBindingId": target.cue_binding_id,
                "cueRowSha256": canonical_sha256(cue),
                "actionBindingSha256": canonical_sha256(binding),
                "catalogRowSha256": canonical_sha256(catalog_row),
            }
        )
    return evidence


def _validate_adapter_rows(root: Path) -> list[dict[str, Any]]:
    document = read_json(repository_path(root, ADAPTER_RELATIVE_PATH))
    verify_seal(document, "artifactSha256", "Trail adapter document")
    evidence: list[dict[str, Any]] = []
    for target in TARGETS:
        row = _exact_rows(
            document.get("adapters"),
            "adapterTargetId",
            target.adapter_target_id,
            "Trail adapter row",
        )
        verify_seal(row, "rowSha256", f"Trail adapter {target.adapter_target_id}")
        source = row.get("sourceIdentity") or {}
        if (
            row.get("rowSha256") != target.adapter_row_sha256
            or row.get("family") != "ANIMATION_TRAIL"
            or row.get("projectionKind") != "ADAPTER_PACKET_V1"
            or row.get("disposition") != "UNRESOLVED_RUNTIME_ADAPTER"
            or row.get("target") is not None
            or row.get("packet") is not None
            or row.get("admissionBlockers")
            != [
                "ANIMATION_TRAIL_EXACT_BAKED_EDGE_HISTORY_MISSING",
                "ANIMATION_TRAIL_RENDERER_TARGET_MISSING",
            ]
            or source.get("patternId") != target.pattern_id
            or source.get("clipOccurrenceId") != target.clip_occurrence_id
            or source.get("sourceActionId") != target.source_action_id
            or source.get("sourceStageIndex") != target.source_stage_index
            or source.get("notifyId") != target.source_notify_id
            or source.get("sourceType") != "Trails"
            or source.get("sourceTimeSeconds") != target.source_time_seconds
            or source.get("sourceDurationSeconds")
            != target.source_duration_seconds
        ):
            raise CandidateError(
                f"reviewed Trail adapter evidence changed: {target.pattern_id}"
            )
        evidence.append(
            {
                "adapterTargetId": target.adapter_target_id,
                "rowSha256": target.adapter_row_sha256,
                "sourceNotifyId": target.source_notify_id,
                "sourceTimeSeconds": target.source_time_seconds,
                "sourceDurationSeconds": target.source_duration_seconds,
                "sourceDisposition": row.get("disposition"),
            }
        )
    return evidence


def _source_family(root: Path) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    path = repository_path(root, SOURCE_DOCUMENT_RELATIVE_PATH)
    document = read_json(path)
    if (
        raw_sha256(path) != SOURCE_DOCUMENT_RAW_SHA256
        or canonical_sha256(document) != SOURCE_DOCUMENT_CANONICAL_SHA256
        or document.get("effectAssetId") != "effect.valtan.pattern.420633.active"
        or len(document.get("elements") or []) != 9
    ):
        raise CandidateError("protected Whirlwind canary bytes changed")
    result: list[dict[str, Any]] = []
    for (
        element_id,
        element_hash,
        resource_hash,
        material_hash,
        geometry_hash,
        _,
    ) in SOURCE_ELEMENTS:
        element = _exact_rows(
            document.get("elements"), "id", element_id, "Whirlwind source element"
        )
        detail = element.get("detail") or {}
        recipe = element.get("sourceRecipe") or {}
        attachment = element.get("actionCueAttachment") or {}
        if (
            canonical_sha256(element) != element_hash
            or canonical_sha256(element.get("resources")) != resource_hash
            or canonical_sha256(element.get("material")) != material_hash
            or canonical_sha256(detail.get("trail")) != geometry_hash
            or element.get("kind") != "trail"
            or element.get("visible") is not False
            or recipe.get("enabled") is not False
            or recipe.get("rendererShape") != "sprite"
            or attachment.get("enabled") is not False
            or attachment.get("follow") is not False
        ):
            raise CandidateError(
                f"protected Whirlwind Trail family changed: {element_id}"
            )
        result.append(element)
    return document, result


def _resource_evidence(root: Path, source_rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    bindings: dict[str, set[str]] = {}
    for element in source_rows:
        for binding in element.get("resources") or []:
            asset_id = str(binding.get("assetId") or "")
            slot_id = str(binding.get("slotId") or "")
            if not asset_id or not slot_id:
                raise CandidateError("protected Whirlwind resource binding is malformed")
            bindings.setdefault(asset_id, set()).add(slot_id)
    if set(bindings) != set(EXPECTED_RESOURCE_EVIDENCE):
        raise CandidateError("protected Whirlwind resource denominator changed")
    evidence: list[dict[str, Any]] = []
    for asset_id in sorted(bindings):
        relative = PurePosixPath(asset_id)
        if relative.is_absolute() or ".." in relative.parts or "\\" in asset_id:
            raise CandidateError(f"unsafe resource asset ID: {asset_id}")
        path = repository_path(root, PurePosixPath("Client/Bin/Resources") / relative)
        expected_size, expected_sha = EXPECTED_RESOURCE_EVIDENCE[asset_id]
        if (
            not path.is_file()
            or path.stat().st_size != expected_size
            or raw_sha256(path) != expected_sha
        ):
            raise CandidateError(f"protected Whirlwind resource changed: {asset_id}")
        evidence.append(
            {
                "assetId": asset_id,
                "slotIds": sorted(bindings[asset_id]),
                "byteSize": expected_size,
                "sha256": expected_sha,
            }
        )
    return evidence


def _bone_evidence(root: Path) -> dict[str, Any]:
    path = repository_path(
        root, PurePosixPath("Client/Bin/Resources") / BODY_MODEL_ASSET_ID
    )
    if not path.is_file() or raw_sha256(path) != BODY_MODEL_SHA256:
        raise CandidateError("Valtan body WModel identity changed")
    try:
        bones = wmodel_evidence.read_wmodel_bones(path)
        evidence = wmodel_evidence.derive_bone_evidence(
            bones, RUNTIME_BONE_NAME, BODY_MODEL_ASSET_ID, BODY_MODEL_SHA256
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
    evidence["runtimeAnchorSlots"] = [
        target.runtime_anchor_slot_id for target in TARGETS
    ]
    return evidence


def _target_base_document(
    root: Path, target: Target
) -> tuple[dict[str, Any], dict[str, Any], bool]:
    path = repository_path(root, target.canonical_relative_path)
    document = read_json(path)
    expected_ids = set(candidate_element_ids(target))
    found = [
        row
        for row in document.get("elements") or []
        if isinstance(row, dict) and row.get("id") in expected_ids
    ]
    if found and {row.get("id") for row in found} != expected_ids:
        raise CandidateError(
            f"bounded weapon Trail projection is partial: {target.pattern_id}"
        )
    base = copy.deepcopy(document)
    base["elements"] = [
        row
        for row in base.get("elements") or []
        if not (isinstance(row, dict) and row.get("id") in expected_ids)
    ]
    receipt_exists = repository_path(root, RECEIPT_RELATIVE_PATH).is_file()
    if (
        document.get("schema") != "lostark.effect-authoring"
        or document.get("version") != 13
        or document.get("effectAssetId") != target.effect_asset_id
    ):
        raise CandidateError(
            f"target canonical Effect requires rebase: {target.pattern_id}"
        )
    if found:
        if not receipt_exists or len(base.get("elements") or []) < target.baseline_element_count:
            raise CandidateError(
                f"committed target document is orphaned or truncated: {target.pattern_id}"
            )
    else:
        base_payload = pretty_json_bytes(base)
        if (
            receipt_exists
            or len(base.get("elements") or []) != target.baseline_element_count
            or raw_sha256_bytes(base_payload) != target.baseline_raw_sha256
            or canonical_sha256(base) != target.baseline_canonical_sha256
        ):
            raise CandidateError(
                f"target canonical Effect requires rebase: {target.pattern_id}"
            )
    return document, base, bool(found)


def _candidate_document(
    target: Target, source_rows: list[dict[str, Any]]
) -> dict[str, Any]:
    elements: list[dict[str, Any]] = []
    for source, source_contract in zip(source_rows, SOURCE_ELEMENTS, strict=True):
        suffix = source_contract[5]
        element = copy.deepcopy(source)
        element["id"] = f"{target.element_prefix}.{suffix}"
        element["displayName"] = (
            f"{target.pattern_id.removeprefix('VALTAN_')} "
            f"{target.stage_id} Trail {suffix}"
        )
        element["groupId"] = (
            f"valtan.project.{target.pattern_id.lower()}.{target.stage_id.lower()}."
            "weapon-trails"
        )
        element["sourceNode"] = (
            f"project-tuned:bounded-reconstruction:{target.adapter_target_id}:"
            f"{SOURCE_FAMILY_NAME}:{source_contract[0]}"
        )
        element["visible"] = True
        element["actionCueAttachment"] = {
            "enabled": True,
            "follow": True,
            "sourceAnchorSlotId": RUNTIME_BONE_NAME,
            "runtimeAnchorSlotId": target.runtime_anchor_slot_id,
            "runtimeBoneName": RUNTIME_BONE_NAME,
            "snapshotRootSourceBasisYawDegrees": 0,
            "socketLocalTransform": {
                "position": [0, 0, 0],
                "rotationDegrees": [0, 0, 0],
                "scale": [1, 1, 1],
            },
        }
        element["detail"]["timing"] = {
            "startDelaySeconds": target.source_time_seconds,
            "lifeTimeSeconds": target.source_duration_seconds,
            "afterImageSeconds": 0.0,
            "dissolveStartNormalized": 1.0,
        }
        element["sourceRecipe"] = disabled_source_recipe()
        element["sourcePresentation"] = {"enabled": False}
        elements.append(element)
    return {
        "schema": "lostark.effect-authoring",
        "version": 13,
        "effectAssetId": target.effect_asset_id,
        "displayName": (
            f"{target.pattern_id.removeprefix('VALTAN_')} {target.stage_id} "
            "bounded weapon Trails"
        ),
        "particleSystem": {
            "uniformScaleMultiplier": 1,
            "yawOffsetDegrees": 0,
            "directionYawDegrees": 0,
            "initialSpeedMultiplier": 1,
        },
        "modelCues": [],
        "elements": elements,
    }


def build_outputs(root: Path) -> dict[PurePosixPath, bytes]:
    root = root.resolve()
    joins = _validate_product_joins(root)
    adapters = _validate_adapter_rows(root)
    _, source_rows = _source_family(root)
    resources = _resource_evidence(root, source_rows)
    bone = _bone_evidence(root)

    applied_states: list[bool] = []
    target_rows: list[dict[str, Any]] = []
    outputs: dict[PurePosixPath, bytes] = {}
    for target, join, adapter in zip(TARGETS, joins, adapters, strict=True):
        canonical, _, applied = _target_base_document(root, target)
        applied_states.append(applied)
        candidate = _candidate_document(target, source_rows)
        candidate_relative = OUTPUT_ROOT / PurePosixPath(target.candidate_filename)
        candidate_payload = pretty_json_bytes(candidate)
        outputs[candidate_relative] = candidate_payload
        candidate_elements = candidate["elements"]
        if len(candidate_elements) != 3 or len(
            {row["id"] for row in candidate_elements}
        ) != 3:
            raise CandidateError("candidate element denominator is not exact")
        if applied:
            canonical_by_id = {
                row.get("id"): row
                for row in canonical.get("elements") or []
                if isinstance(row, dict)
            }
            for candidate_element in candidate_elements:
                current = canonical_by_id.get(candidate_element["id"])
                if current is None or canonical_sha256(
                    protected_element_contract(current)
                ) != canonical_sha256(protected_element_contract(candidate_element)):
                    raise CandidateError(
                        f"committed protected Trail contract drifted: "
                        f"{candidate_element['id']}"
                    )
        target_rows.append(
            {
                **join,
                **adapter,
                "classification": "PROJECT_TUNED",
                "reconstructionPolicy": "BOUNDED_RECONSTRUCTION",
                "effectAssetId": target.effect_asset_id,
                "canonicalPath": target.canonical_relative_path.as_posix(),
                "baselineRawSha256": target.baseline_raw_sha256,
                "baselineCanonicalSha256": target.baseline_canonical_sha256,
                "baselineElementCount": target.baseline_element_count,
                "runtimeAnchorSlotId": target.runtime_anchor_slot_id,
                "runtimeBoneName": RUNTIME_BONE_NAME,
                "candidatePath": candidate_relative.as_posix(),
                "candidateRawSha256": raw_sha256_bytes(candidate_payload),
                "candidateCanonicalSha256": canonical_sha256(candidate),
                "candidateElementIds": [row["id"] for row in candidate_elements],
                "candidateElementSha256": [
                    canonical_sha256(row) for row in candidate_elements
                ],
                "protectedElementContractSha256": [
                    canonical_sha256(protected_element_contract(row))
                    for row in candidate_elements
                ],
            }
        )

    if any(applied_states) and not all(applied_states):
        raise CandidateError("bounded weapon Trail batch is partially committed")
    receipt_path = repository_path(root, RECEIPT_RELATIVE_PATH)
    if all(applied_states) != receipt_path.is_file():
        raise CandidateError(
            "bounded weapon Trail committed rows and receipt do not agree"
        )

    manifest: dict[str, Any] = {
        "schema": "lostark.valtan-bounded-weapon-trail-candidate-manifest",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "classification": "PROJECT_TUNED",
        "reconstructionPolicy": "BOUNDED_RECONSTRUCTION",
        "sequenceDependency": "NONE",
        "sourceLimit": (
            "Target source occurrences have no exact baked-edge history; only "
            "reviewed timing and the protected Whirlwind visual family are reused."
        ),
        "sourceFamily": {
            "familyName": SOURCE_FAMILY_NAME,
            "documentPath": SOURCE_DOCUMENT_RELATIVE_PATH.as_posix(),
            "documentRawSha256": SOURCE_DOCUMENT_RAW_SHA256,
            "documentCanonicalSha256": SOURCE_DOCUMENT_CANONICAL_SHA256,
            "elementIds": [row[0] for row in SOURCE_ELEMENTS],
            "elementSha256": [row[1] for row in SOURCE_ELEMENTS],
            "resourceClosureSha256": [row[2] for row in SOURCE_ELEMENTS],
            "materialClosureSha256": [row[3] for row in SOURCE_ELEMENTS],
            "trailGeometrySha256": [row[4] for row in SOURCE_ELEMENTS],
            "resources": resources,
        },
        "modelEvidence": bone,
        "targets": target_rows,
        "summary": {
            "targetCount": len(TARGETS),
            "candidateDocumentCount": len(TARGETS),
            "candidateElementCount": len(TARGETS) * len(SOURCE_ELEMENTS),
            "sourceEmitterCountPerTarget": len(SOURCE_ELEMENTS),
            "projectionState": "CANDIDATE_ONLY_IMMUTABLE",
        },
    }
    manifest["artifactSha256"] = canonical_sha256(manifest)
    outputs[OUTPUT_ROOT / PurePosixPath(MANIFEST_FILENAME)] = pretty_json_bytes(
        manifest
    )
    return outputs


def _write_outputs(root: Path, outputs: dict[PurePosixPath, bytes]) -> int:
    changed = 0
    staged: list[tuple[Path, Path]] = []
    try:
        for relative, payload in outputs.items():
            target = repository_path(root, relative)
            if target.is_file() and target.read_bytes() == payload:
                continue
            target.parent.mkdir(parents=True, exist_ok=True)
            descriptor, temporary_name = tempfile.mkstemp(
                prefix=target.name + ".staging.", dir=target.parent
            )
            os.close(descriptor)
            temporary = Path(temporary_name)
            temporary.write_bytes(payload)
            staged.append((temporary, target))
        for temporary, target in staged:
            os.replace(temporary, target)
            changed += 1
    finally:
        for temporary, _ in staged:
            temporary.unlink(missing_ok=True)
    return changed


def _check_outputs(root: Path, outputs: dict[PurePosixPath, bytes]) -> int:
    stale = [
        relative.as_posix()
        for relative, payload in outputs.items()
        if not repository_path(root, relative).is_file()
        or repository_path(root, relative).read_bytes() != payload
    ]
    if stale:
        raise CandidateError("candidate outputs are stale: " + ", ".join(stale))
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument(
        "--repository-root", type=Path, default=DEFAULT_REPOSITORY_ROOT
    )
    arguments = parser.parse_args(argv)
    try:
        outputs = build_outputs(arguments.repository_root)
        if arguments.check:
            _check_outputs(arguments.repository_root, outputs)
            print(
                "Valtan bounded weapon Trail candidates: check PASS "
                f"(targets={len(TARGETS)}, elements={len(TARGETS) * len(SOURCE_ELEMENTS)})"
            )
        else:
            changed = _write_outputs(arguments.repository_root, outputs)
            print(
                "Valtan bounded weapon Trail candidates: write PASS "
                f"(changed={changed}, targets={len(TARGETS)}, "
                f"elements={len(TARGETS) * len(SOURCE_ELEMENTS)})"
            )
        return 0
    except CandidateError as error:
        print(f"Valtan bounded weapon Trail candidates: FAILURE: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
