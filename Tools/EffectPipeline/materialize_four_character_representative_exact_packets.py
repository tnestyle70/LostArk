#!/usr/bin/env python3
"""Materialize a check-only plan for four representative Effect documents.

This tool never writes an authored Effect document, the material-program
registry, C++, or a runtime catalog.  Its only writable target is the static
contract artifact in ``Data/Effects/Contracts``.  The plan deliberately seals
zero enabled inline packets and zero runtime admissions until a generic
Program x Layout x Adapter tuple has a compiled program and an actual draw
adapter.
"""

from __future__ import annotations

import argparse
import copy
from collections import Counter
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import re
import sys
import tempfile
from typing import Any


TOOLS_ROOT = Path(__file__).resolve().parent
if str(TOOLS_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOLS_ROOT))

import build_effect_tuple_cohort_inventory as tuple_inventory  # noqa: E402


SCHEMA = "lostark.four-character-representative-packet-plan"
FORMAT_VERSION = 1
IDENTITY = "CHECK_ONLY_PACKET_PLAN_NO_RUNTIME_OR_PRODUCT_ADMISSION"
REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
BUILDER_RELATIVE_PATH = Path(
    "Tools/EffectPipeline/materialize_four_character_representative_exact_packets.py"
)
SCHEMA_RELATIVE_PATH = Path(
    "Tools/EffectPipeline/Schemas/"
    "lostark.four-character-representative-packet-plan.schema.json"
)
OUTPUT_RELATIVE_PATH = Path(
    "Data/Effects/Contracts/four-character-representative-packet-plan.v1.json"
)
TUPLE_INVENTORY_BUILDER_PATH = Path(
    "Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py"
)
TUPLE_INVENTORY_BUILDER_ID = (
    "Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py:build_inventory"
)
SOURCE_CONTRACT_PATH = Path(
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.source-material-contract.json"
)
CHILD_PARENT_PATH = Path(
    "Data/Effects/Contracts/effect-child-parent-resolution.v1.json"
)
COOKED_PROGRAM_PATH = Path(
    "Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json"
)
TRANSLATION_PATH = Path(
    "Data/Effects/Contracts/effect-family-hlsl-translations.v1.json"
)
NAMED_ABI_PATH = Path(
    "Data/Effects/Contracts/effect-family-named-abi.v1.json"
)
REGISTRY_PATH = Path(
    "Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json"
)

EXPECTED_CARRIER_COUNTS = {"DECAL": 8, "MESH": 25, "SPRITE": 98}
EXPECTED_INLINE_STATUS_COUNTS = {
    "ABSENT": 115,
    "DISABLED_FAIL_CLOSED_APPROXIMATION": 16,
}

TARGET_DOCUMENT_SPECS = (
    {
        "effectAssetId": "effect.dimensionmaster.skill.2050180.unified",
        "domain": "DimensionMaster",
        "path": (
            "Data/Effects/Authored/"
            "effect.dimensionmaster.skill.2050180.unified.effect.json"
        ),
        "occurrenceCount": 10,
        "carrierCounts": {"DECAL": 0, "MESH": 3, "SPRITE": 7},
        "rawSha256": (
            "5ba95ca60eb7e019cc3e1b240fa6b1f9ebf57ab88258b51af017a7507eb54147"
        ),
        "canonicalJsonSha256": (
            "4b6d9b39dbc95e8de38fe1725bd764d2e7beeff8e1926088e4db8c44421d1392"
        ),
    },
    {
        "effectAssetId": "effect.artist.skill.31460.unified",
        "domain": "Artist",
        "path": (
            "Data/Effects/Authored/effect.artist.skill.31460.unified.effect.json"
        ),
        "occurrenceCount": 18,
        "carrierCounts": {"DECAL": 2, "MESH": 0, "SPRITE": 16},
        "rawSha256": (
            "b83515d3cb1705238d0e1c18b3bc18ad1458de31ecbdb951dce45ff7abcc333f"
        ),
        "canonicalJsonSha256": (
            "962179e2b9c46823d24b0b92a29e588c79a62031f1ed2c46645dc9d9381394d2"
        ),
    },
    {
        "effectAssetId": "effect.lancemaster.skill.34110.unified",
        "domain": "LanceMaster",
        "path": (
            "Data/Effects/Authored/"
            "effect.lancemaster.skill.34110.unified.effect.json"
        ),
        "occurrenceCount": 88,
        "carrierCounts": {"DECAL": 5, "MESH": 19, "SPRITE": 64},
        "rawSha256": (
            "297067f83379d1eca5bcc23d933450bd0f7c32951e96a848688d255083b79a2f"
        ),
        "canonicalJsonSha256": (
            "6bf6930079de827d275d3ecbac2e96edfe9147e73c0187300d988a8207291347"
        ),
    },
    {
        "effectAssetId": "effect.warlord.skill.17110.clip2.unified",
        "domain": "Warlord",
        "path": (
            "Data/Effects/Authored/"
            "effect.warlord.skill.17110.clip2.unified.effect.json"
        ),
        "occurrenceCount": 3,
        "carrierCounts": {"DECAL": 0, "MESH": 1, "SPRITE": 2},
        "rawSha256": (
            "4d001863fd8abf2ee8ae897b4d3682b3c8915dc97a3d1859bf20a90b2bfead82"
        ),
        "canonicalJsonSha256": (
            "2c7e2b68aa308fd134b2c58c0f9eb6ff6cc603d072b8c5eb05a4dfb0f214b0c8"
        ),
    },
    {
        "effectAssetId": "effect.warlord.skill.17110.clip3.unified",
        "domain": "Warlord",
        "path": (
            "Data/Effects/Authored/"
            "effect.warlord.skill.17110.clip3.unified.effect.json"
        ),
        "occurrenceCount": 12,
        "carrierCounts": {"DECAL": 1, "MESH": 2, "SPRITE": 9},
        "rawSha256": (
            "8403e9c1194e5cd8e1be1dbee3a226bd9b53b5386a154002662920bd4c40502c"
        ),
        "canonicalJsonSha256": (
            "1596f9722836017c69cf7b390603d290cdb28e7c00bb2fc253f685fad750cfcf"
        ),
    },
)

FAMILY_SPECS = (
    {
        "candidateFamilyId": "candidate.family.fx-mm-basic-01-ad.sprite.v1",
        "effectAssetId": "effect.dimensionmaster.skill.2050180.unified",
        "elementIds": (
            "authored.source-particle.65691ec294c8f04b9f0cad89",
            "authored.source-particle.74533d313191421e8811b60e",
            "authored.source-particle.6280dcd7d7b1a067f3de5d0e",
        ),
        "sourceMaterialPath": "fx_m_mi_o_00.fx_mi.fx_o_pa_ri_04_ad_2s",
        "parentMaterialPath": "fx_mastermaterial.fx_mm.fx_mm_basic_01_ad",
        "parentResolution": "CHILD_PARENT_KNOWN_FAMILY_EXACT",
        "childParentRowSha256": (
            "2b9278628aa61c467cb8e7097dc933fb821ad3eba337849d2f020685014c3ba2"
        ),
        "profileId": (
            "ue3.material.fx.mastermaterial.fx.mm.fx.mm.basic.01.ad.c509bec15c99"
        ),
        "renderProfile": "additive_one_sided_depth_read",
        "sourceRenderState": {
            "blendMode": "BLEND_Additive",
            "lightingModel": None,
            "twoSided": False,
            "disableDepthTest": False,
            "usesDistortion": None,
        },
        "sourceContractRowSha256": (
            "66cb6b5845196fcc7530566b75a52e622736a62a07500be9f44951828b8b3178"
        ),
        "sourceParametersSha256": (
            "b4e9d87365aab15a751e87aeeb3e6dbf17735ca67579bf40aa52565f78ccc73a"
        ),
        "authoredSourceProfileSha256": (
            "6d13e002976f1ca4f3f2de701d24bd53b3dae938b4884c0f3fc33867ec5c43f4"
        ),
        "authoredResourcesSha256": (
            "8cbd50e5bf794fd1b5090599ba83237df3ee88a8a7ebb4ad9928d241c18bf4c3"
        ),
        "programCandidateId": (
            "program.4da025b1d279eee9f3798123479f9c442da9ebf2b65441ef5b3dab79fc641705"
        ),
        "programEvidenceId": (
            "program-evidence.8a49bd9137037c1763b30b0242ece70ca795240b80101518c0cdcdce47bc632b"
        ),
        "dxbcSha256": (
            "f68b6d53c31053f8af2c18a367142c0cd480ac1fee09637c8a36884522256790"
        ),
        "dxbcByteSize": 1292,
        "cookedRowSha256": (
            "70c095b51d38c4d433efa41e8790747fe1ff87d50524fdcc478895e9a094d1af"
        ),
        "hlslSha256": (
            "1432faa2925f98f4c87b5bdfafff3c6bae6a9098726aa473842b415479f8fda8"
        ),
        "functionName": "Shade_Ue3_fx_mm_basic_01_ad",
        "translationRowSha256": (
            "5b285600d6c593424c80db87982c5afd6f4d7757a0b7a25c7156f1a136082ceb"
        ),
        "declarationsSha256": (
            "d51e6c2a4d1cc28235a182070bb5a0635d296a57d618efc64d5fd656b5f9f375"
        ),
        "layoutEvidenceId": (
            "layout-evidence.bbc2bf17e35b3a30a8852ab43f2f5032c293bd3e16b540d06a86f9ab8256bf23"
        ),
        "bindingSemanticSha256": (
            "0fc8e401d0e1ada3ee12f9c2e6ba3176faa71eea5b7d33c2f4272a2dc61d574f"
        ),
        "namedAbiRowSha256": (
            "5288a92b24d474f3e7e5b625967bc30ca4dacbeb6e76ffa2531fb9c606e228c0"
        ),
        "namedCounts": {
            "textureSlotCount": 1,
            "scalarLaneCount": 4,
            "vectorLaneCount": 1,
            "timeDependentScalarLaneCount": 0,
            "timeDependentVectorLaneCount": 0,
        },
        "adapterCandidateId": (
            "adapter.9f1dc38cca1f90aad0774bca015149b77d64886d991d7b1ef6dae570a0b3434d"
        ),
        "descriptorVariantId": (
            "descriptor.e930326e03bda0262d85cd384281665780a213c62c822e47608c8e4b3520446b"
        ),
        "familyBlockers": (
            "ACTUAL_SPRITE_ADAPTER_DRAW_UNVERIFIED",
            "COMPILED_PROGRAM_LAYOUT_ADAPTER_TUPLE_ABSENT",
            "ENGINE_PASS_OPACITY_INPUT_UNMATERIALIZED",
            "PACKET_MATERIALIZATION_PENDING",
            "SAMPLER_STATE_UNPROVEN",
            "SELECTIONCOLOR_DESCRIPTOR_VALUE_UNRESOLVED",
            "STAGE_INPUT_SEMANTICS_UNPROVEN",
            "TEXTURE_REGISTER_SAMPLER_TOPOLOGY_UNMATERIALIZED",
        ),
    },
    {
        "candidateFamilyId": "candidate.family.fx-c-pa-lensflare-01-ad.sprite.v1",
        "effectAssetId": "effect.lancemaster.skill.34110.unified",
        "elementIds": (
            "authored.source-particle.0056eabeb7ae2eb45328f928",
            "authored.source-particle.ca1e666e5d8d2fd310bb03e0",
            "authored.source-particle.97c78e48105ef4a529ed9221",
        ),
        "sourceMaterialPath": "fx_m_mi_00.fx_mi.fx_c_pa_lensflare_01_03_ad",
        "parentMaterialPath": "fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad",
        "parentResolution": "CHILD_PARENT_KNOWN_FAMILY_EXACT",
        "childParentRowSha256": (
            "5b2444b9c8895f0675e0b21d67acf821b1e94aa94ed0f334862dbbdcc8df8b60"
        ),
        "profileId": (
            "ue3.material.fx.m.mi.00.fx.m.fx.c.pa.lensflare.01.ad.2cdc706962af"
        ),
        "renderProfile": "additive_one_sided_depth_read",
        "sourceRenderState": {
            "blendMode": "BLEND_Additive",
            "lightingModel": None,
            "twoSided": False,
            "disableDepthTest": False,
            "usesDistortion": None,
        },
        "sourceContractRowSha256": (
            "dfeb36125a80f9fce31c8f0456a3adc42c057a3f9afa0c7aace80580ef5ccf21"
        ),
        "sourceParametersSha256": (
            "15843b44327d3087913d0f147e00e155fec595964f2274ff9033ff7cbfab529a"
        ),
        "authoredSourceProfileSha256": (
            "03c9ef3a5fa0dbe666869227d4d37707910f50219907774cdf6476f14614ee91"
        ),
        "authoredResourcesSha256": (
            "36d80ecab0760982a3677522444ca5bb3d1b9850db707d155a9a86b1966cf9f2"
        ),
        "programCandidateId": (
            "program.80cbee3d7af2db79bcff4ce4ee3903149b3e85b1284734c8887c7820e0b6127f"
        ),
        "programEvidenceId": (
            "program-evidence.754a5414a3576589c9f084215514cf56e881f4bb4ea637adb1464b99bea91d43"
        ),
        "dxbcSha256": (
            "b555abaef0e2477b97b333ae27fd781905ed1876db74d23aad42e3d63525e8b6"
        ),
        "dxbcByteSize": 1892,
        "cookedRowSha256": (
            "71ee26111da1b93ea5c566ad8e24e6c5b11e9f8d9321f32a9ac960fb376fb131"
        ),
        "hlslSha256": (
            "3f8f7f98af0134a63791ca2916e257f4de800f27bf8e4a7222b807fd0e89232a"
        ),
        "functionName": "Shade_Ue3_fx_c_pa_lensflare_01_ad",
        "translationRowSha256": (
            "ba4beda56393a0c36a1f6c308f3a5ade4de84f1e61dc1330d9e29f488bf2da6e"
        ),
        "declarationsSha256": (
            "076d65ca085ab2cd1833d404aa28f21c9713aa94c105ffb95d49c4898d9ebbe2"
        ),
        "layoutEvidenceId": (
            "layout-evidence.224bb5c2a3af7fe1212a68d67aae246b4810f551ea286ff38aeadb9c92e4ca5a"
        ),
        "bindingSemanticSha256": (
            "f2f90cc048d3bcd95f0f4ab90e68636ba7e0591ec1f7ccd28835167f0fdfac36"
        ),
        "namedAbiRowSha256": (
            "03c58c0381be2bb796dcd1fae1ef5663d0266daa3e61e949353eda4901edd313"
        ),
        "namedCounts": {
            "textureSlotCount": 1,
            "scalarLaneCount": 13,
            "vectorLaneCount": 1,
            "timeDependentScalarLaneCount": 10,
            "timeDependentVectorLaneCount": 0,
        },
        "adapterCandidateId": (
            "adapter.5186f9df443b8c1f54fe69d7a5769f07a47a76f3d3fd65211d36602e8d2cdbdf"
        ),
        "descriptorVariantId": (
            "descriptor.3e0bd54bda0a8323c4980b5f94c06ead32f6bd1ae3ddd40362b6a415d64bf304"
        ),
        "familyBlockers": (
            "ACTUAL_SPRITE_ADAPTER_DRAW_UNVERIFIED",
            "COMPILED_PROGRAM_LAYOUT_ADAPTER_TUPLE_ABSENT",
            "PACKET_MATERIALIZATION_PENDING",
            "SCENE_CONSTANT_BUFFER_UNMATERIALIZED",
            "SCENE_DEPTH_INPUT_UNMATERIALIZED",
            "STAGE_INPUT_SEMANTICS_UNPROVEN",
            "TIME_DEPENDENT_LANE_EVALUATOR_UNMATERIALIZED",
            "TEXTURE_REGISTER_SAMPLER_TOPOLOGY_UNMATERIALIZED",
        ),
    },
    {
        "candidateFamilyId": "candidate.family.fx-m-pa-noise-01-tr.sprite.v1",
        "effectAssetId": "effect.lancemaster.skill.34110.unified",
        "elementIds": (
            "authored.source-particle.f279654a39b89bee5ede8fd7",
            "authored.source-particle.490412cd78f2f60c9767ce04",
        ),
        "sourceMaterialPath": "fx_m_mi_03.fx_mi.fx_m_pa_noise_01_2_tr",
        "parentMaterialPath": "fx_m_mi_03.fx_m.fx_m_pa_noise_01_tr",
        "parentResolution": "AUTHORED_PARENT_EXACT",
        "childParentRowSha256": None,
        "profileId": (
            "ue3.material.fx.m.mi.03.fx.m.fx.m.pa.noise.01.tr.05a4fb7d3429"
        ),
        "renderProfile": "alpha_one_sided_depth_read",
        "sourceRenderState": {
            "blendMode": "BLEND_Translucent",
            "lightingModel": None,
            "twoSided": False,
            "disableDepthTest": False,
            "usesDistortion": None,
        },
        "sourceContractRowSha256": (
            "8af61e1754f004095ec4c074415a25ce4e3e4300f52b9d8ecb98c62dcb18973a"
        ),
        "sourceParametersSha256": (
            "c973c299c2d88cbc6baa0c95f003d83043954c504f21074471a76cd884433223"
        ),
        "authoredSourceProfileSha256": (
            "09169749aa5dc806a988b3ad364c32d28b1db6585418175cc4ccc0f363c618ac"
        ),
        "authoredResourcesSha256": (
            "0b45a82e1129728d9b98abb2ffbbcdb6f2cda5000c476038a0c0cee3464d6717"
        ),
        "programCandidateId": (
            "program.fbb4b18ca1a304d25df9d09676b40109540bfcd4604318e80818e1e7c19a3463"
        ),
        "programEvidenceId": (
            "program-evidence.ea5384a0d19b35f8f1abb613e8cc07df0284ad4f2bcd74d753be35dfa1353be3"
        ),
        "dxbcSha256": (
            "7416554f16b4cc3c196df6d86e73cd2d625f97d58a2b4597f1592be8b7e02ba7"
        ),
        "dxbcByteSize": 2876,
        "cookedRowSha256": (
            "b6f75c5b5fbbb9b8fb96f7bd6df0a0e05bfb221baf4114837dce26420ada47b3"
        ),
        "hlslSha256": (
            "38fcd60dc7aad4a16697d332ac504e09cf77f831651fe3cc29a7b64f2042d26f"
        ),
        "functionName": "Shade_Ue3_fx_m_pa_noise_01_tr",
        "translationRowSha256": (
            "b2a3e292371fc1c16787da647af24ccb4accc6f82bb2a7c0cb1a739d52a9a906"
        ),
        "declarationsSha256": (
            "192dce0ea4aa00b92b73f8c5d3c7220b9df339cf3a684cc24797b09ce92fb1b6"
        ),
        "layoutEvidenceId": (
            "layout-evidence.e80de5d1d57dca08300fa5ea1cde7def700918ea13f88315aa3b193580f822ec"
        ),
        "bindingSemanticSha256": (
            "d7c5fc0c2e344e99bdaa30ef69a05135fd3064fcd4f2ae33a6ac7dc6d83e9413"
        ),
        "namedAbiRowSha256": (
            "e02d6cc858843eb8140a5a43666f393604ef075baed7570c37368e792f8bd83b"
        ),
        "namedCounts": {
            "textureSlotCount": 3,
            "scalarLaneCount": 3,
            "vectorLaneCount": 3,
            "timeDependentScalarLaneCount": 1,
            "timeDependentVectorLaneCount": 2,
        },
        "adapterCandidateId": (
            "adapter.f9eb28d1a1bd4738cb72b4c5a842dd6856951f7c3c823c351bc51cd1aea0065e"
        ),
        "descriptorVariantId": (
            "descriptor.c2608e3aec9b588a2303c0e203de6b2534e769ef99b33a197b165d38667ae67c"
        ),
        "familyBlockers": (
            "ACTUAL_SPRITE_ADAPTER_DRAW_UNVERIFIED",
            "COMPILED_PROGRAM_LAYOUT_ADAPTER_TUPLE_ABSENT",
            "DEFAULT_TEXTURE_SLOT_IDENTITY_UNMATERIALIZED",
            "MRT_POLICY_UNMATERIALIZED",
            "PACKET_MATERIALIZATION_PENDING",
            "STAGE_INPUT_SEMANTICS_UNPROVEN",
            "TIME_DEPENDENT_LANE_EVALUATOR_UNMATERIALIZED",
            "TEXTURE_REGISTER_SAMPLER_TOPOLOGY_UNMATERIALIZED",
        ),
    },
)

SHA256_RE = re.compile(r"^[0-9a-f]{64}$")


class PlanError(ValueError):
    """A fail-closed representative packet-plan contract violation."""


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise PlanError(message)


def _reject_non_finite(token: str) -> None:
    raise PlanError("JSON contains a non-finite number: " + token)


def _object_without_duplicate_keys(
    pairs: list[tuple[str, Any]],
) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise PlanError("JSON contains a duplicate key: " + key)
        result[key] = value
    return result


def decode_json(payload: bytes, label: str) -> Any:
    try:
        text = payload.decode("utf-8-sig")
    except UnicodeError as error:
        raise PlanError(label + " is not UTF-8") from error
    try:
        return json.loads(
            text,
            object_pairs_hook=_object_without_duplicate_keys,
            parse_constant=_reject_non_finite,
        )
    except PlanError:
        raise
    except (TypeError, ValueError) as error:
        raise PlanError(label + " is not valid JSON") from error


def canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def canonical_sha256(value: Any) -> str:
    return hashlib.sha256(canonical_json_bytes(value)).hexdigest()


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def pretty_json_bytes(value: Any) -> bytes:
    return (
        json.dumps(value, ensure_ascii=False, indent=1, allow_nan=False) + "\n"
    ).encode("utf-8")


def _safe_relative_path(value: str, label: str) -> str:
    _require(isinstance(value, str) and bool(value), label + " is empty")
    normalized = value.replace("\\", "/")
    pure = PurePosixPath(normalized)
    _require(not pure.is_absolute(), label + " is absolute")
    _require(".." not in pure.parts, label + " escapes the repository")
    _require(not re.match(r"^[A-Za-z]:", normalized), label + " is drive-qualified")
    return pure.as_posix()


def _require_sha256(value: Any, label: str) -> str:
    _require(
        isinstance(value, str) and SHA256_RE.fullmatch(value) is not None,
        label + " is not SHA-256",
    )
    return value


def _require_exact_keys(value: dict[str, Any], keys: set[str], label: str) -> None:
    _require(set(value) == keys, label + " fields drifted")


def _validate_finite(value: Any, label: str = "root") -> None:
    if isinstance(value, float):
        _require(math.isfinite(value), label + " contains a non-finite number")
    elif isinstance(value, list):
        for index, item in enumerate(value):
            _validate_finite(item, f"{label}[{index}]")
    elif isinstance(value, dict):
        for key, item in value.items():
            _validate_finite(item, label + "." + str(key))


def _validate_upstream_artifact(document: Any, label: str) -> None:
    _require(isinstance(document, dict), label + " root must be an object")
    digest = _require_sha256(document.get("artifactSha256"), label + ".artifactSha256")
    unsigned = copy.deepcopy(document)
    unsigned.pop("artifactSha256", None)
    _require(canonical_sha256(unsigned) == digest, label + " self hash drifted")


class InputTracker:
    def __init__(self, root: Path) -> None:
        self.root = root
        self._rows: dict[str, dict[str, Any]] = {}

    def read_bytes(self, path: Path | str, role: str) -> bytes:
        relative = _safe_relative_path(Path(path).as_posix(), "input path")
        absolute = self.root / Path(relative)
        _require(absolute.is_file(), "required input is absent: " + relative)
        payload = absolute.read_bytes()
        self._record(relative, role, payload, None)
        return payload

    def read_json(self, path: Path | str, role: str) -> Any:
        relative = _safe_relative_path(Path(path).as_posix(), "input path")
        absolute = self.root / Path(relative)
        _require(absolute.is_file(), "required input is absent: " + relative)
        payload = absolute.read_bytes()
        document = decode_json(payload, relative)
        self._record(relative, role, payload, canonical_sha256(document))
        return document

    def _record(
        self,
        relative: str,
        role: str,
        payload: bytes,
        canonical: str | None,
    ) -> None:
        row = {
            "path": relative,
            "roles": [role],
            "rawSha256": sha256_bytes(payload),
            "byteSize": len(payload),
            "canonicalJsonSha256": canonical,
        }
        existing = self._rows.get(relative)
        if existing is None:
            self._rows[relative] = row
            return
        _require(
            existing["rawSha256"] == row["rawSha256"]
            and existing["byteSize"] == row["byteSize"]
            and existing["canonicalJsonSha256"] == canonical,
            "input identity changed during build: " + relative,
        )
        if role not in existing["roles"]:
            existing["roles"].append(role)
            existing["roles"].sort()

    def identity(self, path: Path | str) -> dict[str, Any]:
        relative = Path(path).as_posix()
        _require(relative in self._rows, "input was not tracked: " + relative)
        return copy.deepcopy(self._rows[relative])

    def rows(self) -> list[dict[str, Any]]:
        return [copy.deepcopy(self._rows[key]) for key in sorted(self._rows)]


def _index_unique(rows: list[dict[str, Any]], key: str, label: str) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for row in rows:
        value = row.get(key)
        _require(isinstance(value, str) and bool(value), label + " has an invalid " + key)
        _require(value not in result, label + " duplicates " + key + ": " + value)
        result[value] = row
    return result


def _find_one(rows: list[dict[str, Any]], predicate, label: str) -> dict[str, Any]:
    matches = [row for row in rows if predicate(row)]
    _require(len(matches) == 1, label + " must resolve exactly once")
    return matches[0]


def _carrier_for(element: dict[str, Any]) -> str:
    kind = element.get("kind")
    shape = (element.get("sourceRecipe") or {}).get("rendererShape")
    if kind == "decal" and shape == "decal":
        return "DECAL"
    if kind == "particle" and shape == "mesh":
        return "MESH"
    if kind == "particle" and shape == "sprite":
        return "SPRITE"
    raise PlanError(
        "representative occurrence is outside Sprite/Mesh/Decal scope: "
        + str(element.get("id"))
    )


def _inline_status(material: dict[str, Any]) -> str:
    if "execution" not in material:
        return "ABSENT"
    execution = material.get("execution")
    _require(isinstance(execution, dict), "inline execution must be an object")
    _require(
        execution
        == {
            "enabled": False,
            "failClosed": True,
            "authoringApproximate": True,
        },
        "representative inline execution changed or became enabled",
    )
    return "DISABLED_FAIL_CLOSED_APPROXIMATION"


def _non_priority_basis(inventory_row: dict[str, Any]) -> tuple[str, str]:
    program_status = inventory_row["program"]["status"]
    layout_status = inventory_row["layout"]["status"]
    if program_status != "DXBC_OCCURRENCE_EXACT":
        return "PROGRAM_NOT_OCCURRENCE_EXACT", "OCCURRENCE_EXACT_PROGRAM_ABSENT"
    if layout_status != "NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS":
        return "LAYOUT_NOT_WITHIN_PACKET_CAPS", "PACKET_LAYOUT_NOT_CLOSED"
    return (
        "SOURCE_STATE_OR_ADAPTER_CLOSURE_NOT_ADMITTED",
        "SOURCE_STATE_CLOSURE_NOT_ADMITTED",
    )


def _build_candidate_family(
    spec: dict[str, Any],
    source_rows: list[dict[str, Any]],
    child_rows: list[dict[str, Any]],
    cooked_rows: list[dict[str, Any]],
    translations: list[dict[str, Any]],
    named_rows: list[dict[str, Any]],
    program_candidates: dict[str, dict[str, Any]],
    program_evidence: dict[str, dict[str, Any]],
    layout_evidence: dict[str, dict[str, Any]],
    priority_elements: dict[tuple[str, str], dict[str, Any]],
    tracker: InputTracker,
) -> dict[str, Any]:
    source = _find_one(
        source_rows,
        lambda row: row.get("sourceMaterialPath") == spec["sourceMaterialPath"],
        spec["candidateFamilyId"] + " source contract row",
    )
    _require(
        canonical_sha256(source) == spec["sourceContractRowSha256"],
        spec["candidateFamilyId"] + " source contract row drifted",
    )
    _require(source.get("parentMaterialPath") == spec["parentMaterialPath"], "source parent drifted")
    _require(source.get("profileId") == spec["profileId"], "source profile id drifted")
    _require(source.get("renderState") == spec["sourceRenderState"], "source render state drifted")
    _require(
        canonical_sha256(source.get("sourceParameters"))
        == spec["sourceParametersSha256"],
        "source parameter evidence drifted",
    )

    if spec["childParentRowSha256"] is not None:
        child = _find_one(
            child_rows,
            lambda row: row.get("childMaterialPath") == spec["sourceMaterialPath"],
            spec["candidateFamilyId"] + " child-parent row",
        )
        _require(child.get("status") == "RESOLVED", "child-parent row is blocked")
        _require(
            child.get("canonicalParentMaterialPath") == spec["parentMaterialPath"],
            "canonical child parent drifted",
        )
        _require(
            child.get("rowSha256") == spec["childParentRowSha256"],
            "child-parent row identity drifted",
        )

    cooked = _find_one(
        cooked_rows,
        lambda row: row.get("parentMaterialPath") == spec["parentMaterialPath"]
        and row.get("dxbcSha256") == spec["dxbcSha256"],
        spec["candidateFamilyId"] + " cooked program row",
    )
    _require(cooked.get("status") == "EXTRACTED", "cooked program is not extracted")
    _require(cooked.get("carrier") == "sprite", "cooked carrier is not sprite")
    _require(cooked.get("dxbcByteSize") == spec["dxbcByteSize"], "DXBC size drifted")
    _require(
        canonical_sha256(cooked) == spec["cookedRowSha256"],
        "cooked program row drifted",
    )

    translation = _find_one(
        translations,
        lambda row: row.get("dxbcSha256") == spec["dxbcSha256"],
        spec["candidateFamilyId"] + " HLSL translation row",
    )
    _require(translation.get("status") == "TRANSLATED", "HLSL translation is absent")
    _require(translation.get("hlslSha256") == spec["hlslSha256"], "HLSL identity drifted")
    _require(translation.get("functionName") == spec["functionName"], "HLSL function drifted")
    _require(
        canonical_sha256(translation) == spec["translationRowSha256"],
        "HLSL translation row drifted",
    )
    declarations = translation.get("declarations")
    _require(
        canonical_sha256(declarations) == spec["declarationsSha256"],
        "HLSL declaration identity drifted",
    )

    named = _find_one(
        named_rows,
        lambda row: row.get("parentMaterialPath") == spec["parentMaterialPath"]
        and row.get("dxbcSha256") == spec["dxbcSha256"],
        spec["candidateFamilyId"] + " named ABI row",
    )
    _require(named.get("status") == "RESOLVED_NAMED_MAPPING", "named ABI is unresolved")
    _require(named.get("admits") == "NAMED_LANE_IDENTITY_ONLY", "named ABI over-admitted")
    _require(
        named.get("nativeBindingWire", {}).get("bindingSemanticSha256")
        == spec["bindingSemanticSha256"],
        "binding semantic identity drifted",
    )
    _require(
        canonical_sha256(named) == spec["namedAbiRowSha256"],
        "named ABI row drifted",
    )

    program = program_candidates[spec["programCandidateId"]]
    _require(program.get("dxbcSha256") == spec["dxbcSha256"], "program candidate DXBC drifted")
    _require(program.get("hlslSha256") == spec["hlslSha256"], "program candidate HLSL drifted")
    _require(program.get("functionName") == spec["functionName"], "program candidate function drifted")
    evidence = program_evidence[spec["programEvidenceId"]]
    _require(evidence.get("occurrenceExact") is True, "program evidence is not occurrence exact")
    _require(evidence.get("cookedDxbcSha256") == spec["dxbcSha256"], "program evidence DXBC drifted")
    layout = layout_evidence[spec["layoutEvidenceId"]]
    _require(layout.get("bindingSemanticSha256") == spec["bindingSemanticSha256"], "layout semantic drifted")
    _require(layout.get("counts") == {
        key: spec["namedCounts"][key]
        for key in ("textureSlotCount", "scalarLaneCount", "vectorLaneCount")
    }, "layout counts drifted")
    _require(layout.get("withinCountCaps") is True, "layout exceeds current count caps")
    _require(layout.get("runtimePacketTopologyMaterialized") is False, "layout unexpectedly materialized")

    elements = [
        priority_elements[(spec["effectAssetId"], element_id)]
        for element_id in spec["elementIds"]
    ]
    for element in elements:
        material = element.get("material") or {}
        _require(material.get("sourceMaterialPath") == spec["sourceMaterialPath"], "priority child material drifted")
        _require(material.get("renderProfile") == spec["renderProfile"], "priority render state drifted")
        _require(material.get("templateId") == "effect.source_material", "priority template drifted")
        _require("execution" not in material, "priority occurrence gained inline execution")
        _require(
            canonical_sha256(material.get("sourceProfile"))
            == spec["authoredSourceProfileSha256"],
            "authored source profile identity drifted",
        )
        _require(
            canonical_sha256(element.get("resources", []))
            == spec["authoredResourcesSha256"],
            "authored resource identity drifted",
        )

    dxbc_path = Path("Data/Effects/CookedShaders") / (spec["dxbcSha256"] + ".dxbc")
    hlsl_path = Path("Data/Effects/TranslatedShaders") / (spec["functionName"] + ".hlsli")
    dxbc_payload = tracker.read_bytes(dxbc_path, "PRIORITY_EXACT_DXBC")
    hlsl_payload = tracker.read_bytes(hlsl_path, "PRIORITY_LITERAL_HLSL")
    _require(sha256_bytes(dxbc_payload) == spec["dxbcSha256"], "DXBC file identity drifted")
    _require(len(dxbc_payload) == spec["dxbcByteSize"], "DXBC file size drifted")
    _require(sha256_bytes(hlsl_payload) == spec["hlslSha256"], "HLSL file identity drifted")

    family_blockers = sorted(set(spec["familyBlockers"]))
    return {
        "candidateFamilyId": spec["candidateFamilyId"],
        "disposition": "PACKET_MATERIALIZATION_PENDING",
        "targetEffectAssetId": spec["effectAssetId"],
        "targetElementIds": list(spec["elementIds"]),
        "source": {
            "sourceMaterialPath": spec["sourceMaterialPath"],
            "parentMaterialPath": spec["parentMaterialPath"],
            "parentResolution": spec["parentResolution"],
            "childParentRowSha256": spec["childParentRowSha256"],
            "profileId": spec["profileId"],
            "renderProfile": spec["renderProfile"],
            "renderState": copy.deepcopy(spec["sourceRenderState"]),
            "sourceContractRowSha256": spec["sourceContractRowSha256"],
            "sourceParametersSha256": spec["sourceParametersSha256"],
            "authoredSourceProfileSha256": spec["authoredSourceProfileSha256"],
            "authoredResourcesSha256": spec["authoredResourcesSha256"],
            "sourceContractResourceBindings": copy.deepcopy(
                source.get("currentResourceBindings", [])
            ),
        },
        "program": {
            "programCandidateId": spec["programCandidateId"],
            "programEvidenceId": spec["programEvidenceId"],
            "dxbcSha256": spec["dxbcSha256"],
            "dxbcByteSize": spec["dxbcByteSize"],
            "dxbcPath": dxbc_path.as_posix(),
            "cookedRowSha256": spec["cookedRowSha256"],
            "hlslSha256": spec["hlslSha256"],
            "hlslPath": hlsl_path.as_posix(),
            "functionName": spec["functionName"],
            "translationRowSha256": spec["translationRowSha256"],
            "declarationsSha256": spec["declarationsSha256"],
            "constantBuffers": copy.deepcopy(declarations.get("constantBuffers", {})),
            "samplers": copy.deepcopy(declarations.get("samplers", [])),
            "textures": copy.deepcopy(declarations.get("textures", [])),
            "inputs": copy.deepcopy(declarations.get("inputs", [])),
            "outputs": copy.deepcopy(declarations.get("outputs", [])),
            "compiledRuntimeProgramId": None,
        },
        "namedAbi": {
            "layoutEvidenceId": spec["layoutEvidenceId"],
            "bindingSemanticSha256": spec["bindingSemanticSha256"],
            "namedAbiRowSha256": spec["namedAbiRowSha256"],
            **copy.deepcopy(spec["namedCounts"]),
            "textureSlots": copy.deepcopy(named.get("textureSlots", [])),
            "runtimePacketTopologyMaterialized": False,
            "compiledRuntimeLayoutId": None,
        },
        "adapter": {
            "staticCandidateId": spec["adapterCandidateId"],
            "requiredCarrier": "SPRITE",
            "requiredRenderProfile": spec["renderProfile"],
            "compiledRuntimeAdapterId": None,
            "actualDrawVerified": False,
        },
        "descriptor": {
            "staticCandidateId": spec["descriptorVariantId"],
            "runtimeDescriptorId": None,
            "packetMaterialized": False,
        },
        "runtimeAdmission": False,
        "blockers": family_blockers,
    }


def build_plan(repository_root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
    root = repository_root.resolve()
    tracker = InputTracker(root)
    tracker.read_bytes(BUILDER_RELATIVE_PATH, "BUILDER")
    schema = tracker.read_json(SCHEMA_RELATIVE_PATH, "SCHEMA")
    _require(schema.get("$id") == (
        "https://lostark.local/schemas/"
        "four-character-representative-packet-plan-v1.json"
    ), "packet-plan schema identity drifted")

    documents: list[tuple[dict[str, Any], dict[str, Any], bytes]] = []
    priority_element_keys = {
        (spec["effectAssetId"], element_id)
        for spec in FAMILY_SPECS
        for element_id in spec["elementIds"]
    }
    priority_elements: dict[tuple[str, str], dict[str, Any]] = {}
    for spec in TARGET_DOCUMENT_SPECS:
        path = Path(spec["path"])
        document = tracker.read_json(path, "TARGET_AUTHORED_BYTE_STABILITY")
        payload = (root / path).read_bytes()
        tracker._record(
            path.as_posix(),
            "TARGET_AUTHORED_SEMANTICS",
            payload,
            canonical_sha256(document),
        )
        _require(sha256_bytes(payload) == spec["rawSha256"], "target authored bytes drifted: " + path.as_posix())
        _require(canonical_sha256(document) == spec["canonicalJsonSha256"], "target authored semantics drifted: " + path.as_posix())
        _require(document.get("effectAssetId") == spec["effectAssetId"], "target effectAssetId drifted")
        elements = document.get("elements")
        _require(isinstance(elements, list), "target elements must be an array")
        _require(len(elements) == spec["occurrenceCount"], "target occurrence denominator drifted")
        ids = [element.get("id") for element in elements]
        _require(all(isinstance(value, str) and value for value in ids), "target element id is invalid")
        _require(len(ids) == len(set(ids)), "target document duplicates an element id")
        carrier_counts = Counter(_carrier_for(element) for element in elements)
        _require({key: carrier_counts.get(key, 0) for key in EXPECTED_CARRIER_COUNTS} == spec["carrierCounts"], "target carrier denominator drifted")
        for element in elements:
            key = (spec["effectAssetId"], element["id"])
            if key in priority_element_keys:
                _require(key not in priority_elements, "priority element is duplicated")
                priority_elements[key] = element
        documents.append((spec, document, payload))
    _require(set(priority_elements) == priority_element_keys, "priority element set drifted")

    tracker.read_bytes(
        TUPLE_INVENTORY_BUILDER_PATH,
        "IN_MEMORY_CURRENT_HEAD_TUPLE_INVENTORY_BUILDER",
    )
    inventory = tuple_inventory.build_inventory(root)
    _validate_upstream_artifact(inventory, "in-memory current-HEAD tuple inventory")
    inventory_artifact_sha256 = inventory["artifactSha256"]
    source_contract = tracker.read_json(SOURCE_CONTRACT_PATH, "SOURCE_MATERIAL_CONTRACT")
    child_parent = tracker.read_json(CHILD_PARENT_PATH, "CHILD_PARENT_RESOLUTION")
    _validate_upstream_artifact(child_parent, CHILD_PARENT_PATH.as_posix())
    cooked = tracker.read_json(COOKED_PROGRAM_PATH, "COOKED_PIXEL_PROGRAMS")
    _validate_upstream_artifact(cooked, COOKED_PROGRAM_PATH.as_posix())
    translations = tracker.read_json(TRANSLATION_PATH, "LITERAL_HLSL_TRANSLATIONS")
    named_abi = tracker.read_json(NAMED_ABI_PATH, "NAMED_NATIVE_ABI")
    _validate_upstream_artifact(named_abi, NAMED_ABI_PATH.as_posix())
    registry = tracker.read_json(REGISTRY_PATH, "RUNTIME_MATERIAL_PROGRAM_REGISTRY")

    _require(isinstance(inventory.get("occurrences"), list), "tuple inventory occurrences are absent")
    target_asset_ids = [spec["effectAssetId"] for spec in TARGET_DOCUMENT_SPECS]
    target_inventory_rows = [
        row for row in inventory["occurrences"]
        if row.get("effectAssetId") in target_asset_ids
    ]
    _require(len(target_inventory_rows) == 131, "tuple inventory target denominator drifted")
    inventory_by_element: dict[tuple[str, str], dict[str, Any]] = {}
    for row in target_inventory_rows:
        key = (row.get("effectAssetId"), row.get("elementId"))
        _require(key not in inventory_by_element, "tuple inventory duplicates a target occurrence")
        inventory_by_element[key] = row

    program_candidates = _index_unique(
        inventory.get("programCandidates", []), "programCandidateId", "program candidates"
    )
    program_evidence = _index_unique(
        inventory.get("programEvidence", []), "programEvidenceId", "program evidence"
    )
    layout_evidence = _index_unique(
        inventory.get("layoutEvidence", []), "layoutEvidenceId", "layout evidence"
    )

    candidate_families = [
        _build_candidate_family(
            spec,
            source_contract.get("materialIdentities", []),
            child_parent.get("children", []),
            cooked.get("families", []),
            translations,
            named_abi.get("families", []),
            program_candidates,
            program_evidence,
            layout_evidence,
            priority_elements,
            tracker,
        )
        for spec in FAMILY_SPECS
    ]
    family_by_element = {
        (family["targetEffectAssetId"], element_id): family
        for family in candidate_families
        for element_id in family["targetElementIds"]
    }
    _require(len(family_by_element) == 8, "priority candidate denominator drifted")

    bindings = registry.get("bindings")
    _require(isinstance(bindings, list), "runtime registry bindings are absent")
    target_bindings = [
        row for row in bindings if row.get("effectAssetId") in target_asset_ids
    ]
    _require(not target_bindings, "representative target gained a runtime registry binding")

    occurrences: list[dict[str, Any]] = []
    target_documents: list[dict[str, Any]] = []
    for spec, document, payload in documents:
        inline_counts: Counter[str] = Counter()
        carrier_counts: Counter[str] = Counter()
        for order, element in enumerate(document["elements"]):
            key = (spec["effectAssetId"], element["id"])
            _require(key in inventory_by_element, "target occurrence is absent from tuple inventory")
            inventory_row = inventory_by_element[key]
            _require(inventory_row.get("elementOrder") == order, "tuple occurrence order drifted")
            _require(inventory_row.get("authoredPath") == spec["path"], "tuple authored path drifted")
            _require(inventory_row.get("authoredElementSha256") == canonical_sha256(element), "tuple authored element identity drifted")
            carrier = _carrier_for(element)
            _require(inventory_row.get("carrier") == carrier, "tuple carrier drifted")
            material = element.get("material") or {}
            _require(isinstance(material, dict), "element material must be an object")
            inline_status = _inline_status(material)
            inline_counts[inline_status] += 1
            carrier_counts[carrier] += 1
            family = family_by_element.get(key)
            if family is None:
                admission_basis, explicit_blocker = _non_priority_basis(inventory_row)
                disposition = "BLOCKED"
                candidate_family_id = None
                blockers = sorted(set(
                    inventory_row.get("blockers", [])
                    + ["REPRESENTATIVE_PACKET_ADMISSION_NOT_CLOSED", explicit_blocker]
                ))
            else:
                admission_basis = "SOURCE_STATE_EXACT_MATCH_STATIC_EVIDENCE_ONLY"
                disposition = "PACKET_MATERIALIZATION_PENDING"
                candidate_family_id = family["candidateFamilyId"]
                blockers = sorted(set(
                    inventory_row.get("blockers", []) + family["blockers"]
                ))
            _require(blockers, "every representative occurrence needs a blocker")
            occurrences.append({
                "occurrenceId": inventory_row["occurrenceId"],
                "effectAssetId": spec["effectAssetId"],
                "elementId": element["id"],
                "elementOrder": order,
                "authoredElementSha256": canonical_sha256(element),
                "carrier": carrier,
                "fineRendererKind": inventory_row["fineRendererKind"],
                "sourceMaterialPath": material.get("sourceMaterialPath"),
                "sourceParentMaterialPath": inventory_row.get("sourceParentMaterialPath"),
                "sourceParentResolution": inventory_row.get("sourceParentResolution"),
                "sourceParentRowSha256": inventory_row.get("sourceParentRowSha256"),
                "renderProfile": material.get("renderProfile"),
                "resourcesSha256": canonical_sha256(element.get("resources", [])),
                "sourceProfileSha256": (
                    canonical_sha256(material["sourceProfile"])
                    if isinstance(material.get("sourceProfile"), dict)
                    else None
                ),
                "inlineExecutionStatus": inline_status,
                "inlineExecutionEnabled": False,
                "programStatus": inventory_row["program"]["status"],
                "programCandidateId": inventory_row["program"].get("programCandidateId"),
                "programEvidenceId": inventory_row["program"].get("programEvidenceId"),
                "layoutStatus": inventory_row["layout"]["status"],
                "layoutEvidenceId": inventory_row["layout"].get("layoutEvidenceId"),
                "staticAdapterCandidateId": inventory_row["adapter"].get("adapterCandidateId"),
                "candidateFamilyId": candidate_family_id,
                "packetDisposition": disposition,
                "admissionBasis": admission_basis,
                "runtimeAdmission": False,
                "proposedEnabledExecution": None,
                "blockers": blockers,
            })
        identity = tracker.identity(spec["path"])
        target_documents.append({
            "effectAssetId": spec["effectAssetId"],
            "domain": spec["domain"],
            "path": spec["path"],
            "byteSize": len(payload),
            "rawSha256": identity["rawSha256"],
            "canonicalJsonSha256": identity["canonicalJsonSha256"],
            "occurrenceCount": spec["occurrenceCount"],
            "carrierCounts": {
                key: carrier_counts.get(key, 0) for key in EXPECTED_CARRIER_COUNTS
            },
            "enabledInlineExecutionCount": 0,
            "disabledInlineExecutionCount": inline_counts.get(
                "DISABLED_FAIL_CLOSED_APPROXIMATION", 0
            ),
            "runtimeRegistryBindingCount": 0,
            "authoredByteMutationAllowed": False,
        })

    _require(len(occurrences) == 131, "representative occurrence denominator drifted")
    inline_status_counts = Counter(row["inlineExecutionStatus"] for row in occurrences)
    carrier_counts = Counter(row["carrier"] for row in occurrences)
    disposition_counts = Counter(row["packetDisposition"] for row in occurrences)
    _require(dict(sorted(inline_status_counts.items())) == EXPECTED_INLINE_STATUS_COUNTS, "inline execution denominator drifted")
    _require({key: carrier_counts.get(key, 0) for key in EXPECTED_CARRIER_COUNTS} == EXPECTED_CARRIER_COUNTS, "carrier denominator drifted")
    _require(disposition_counts == {"BLOCKED": 123, "PACKET_MATERIALIZATION_PENDING": 8}, "packet disposition denominator drifted")

    artifact: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "identity": IDENTITY,
        "scope": {
            "effectAssetIds": target_asset_ids,
            "documentCount": 5,
            "occurrenceCount": 131,
            "carrierCounts": EXPECTED_CARRIER_COUNTS,
            "priorityCandidateCount": 8,
        },
        "policies": {
            "authoredWriter": False,
            "registryWriter": False,
            "cppWriter": False,
            "enabledPacketGeneration": (
                "FORBIDDEN_UNTIL_COMPILED_TUPLE_AND_ACTUAL_ADAPTER"
            ),
            "rendererDispatch": "PROGRAM_LAYOUT_ADAPTER_TUPLE_ONLY",
            "skillSpecificRendererSwitch": "FORBIDDEN",
            "visualAdmission": "USER_ONLY",
        },
        "inputs": {
            "tupleInventory": {
                "builder": TUPLE_INVENTORY_BUILDER_ID,
                "mode": "IN_MEMORY_CURRENT_HEAD",
                "checkedInArtifactUsed": False,
                "artifactSha256": inventory_artifact_sha256,
            },
            "files": tracker.rows(),
        },
        "targetDocuments": target_documents,
        "candidateFamilies": candidate_families,
        "occurrences": occurrences,
        "summary": {
            "documentCount": 5,
            "occurrenceCount": 131,
            "carrierCounts": EXPECTED_CARRIER_COUNTS,
            "inlineExecutionStatusCounts": EXPECTED_INLINE_STATUS_COUNTS,
            "enabledInlineExecutionCount": 0,
            "runtimeRegistryBindingCount": 0,
            "runtimeAdmissionCount": 0,
            "compiledTupleCount": 0,
            "actualAdapterDrawVerifiedCount": 0,
            "packetMaterializationPendingCount": 8,
            "blockedCount": 123,
            "candidateFamilyCount": 3,
            "authoredMutationCount": 0,
        },
        "admission": {
            "runtimeExecution": False,
            "productAdmission": False,
            "enabledPacketEmission": False,
            "currentProductPreserved": True,
            "requiredBeforeAnyEnabledPacket": [
                "BIT_EXACT_REGISTRY_AND_INLINE_PACKET",
                "COMPILED_PROGRAM_LAYOUT_ADAPTER_TUPLE",
                "ACTUAL_CARRIER_PASS_STATE_MRT_DRAW_PROOF",
                "FOCUSED_DEBUG_RELEASE_EVIDENCE",
            ],
        },
        "transaction": {
            "model": "PARSE_VALIDATE_STAGE_ATOMIC_PLAN_REPLACE",
            "partialCommitAllowed": False,
            "writablePath": OUTPUT_RELATIVE_PATH.as_posix(),
            "authoredDocumentsByteStable": True,
        },
    }
    artifact["artifactSha256"] = canonical_sha256(artifact)
    validate_plan(artifact)
    return artifact


def validate_plan(document: dict[str, Any]) -> None:
    _validate_finite(document)
    _require_exact_keys(document, {
        "schema", "formatVersion", "identity", "scope", "policies", "inputs",
        "targetDocuments", "candidateFamilies", "occurrences", "summary",
        "admission", "transaction", "artifactSha256",
    }, "packet plan")
    _require(document.get("schema") == SCHEMA, "packet plan schema drifted")
    _require(document.get("formatVersion") == FORMAT_VERSION, "packet plan version drifted")
    _require(document.get("identity") == IDENTITY, "packet plan identity drifted")
    digest = _require_sha256(document.get("artifactSha256"), "artifactSha256")
    unsigned = copy.deepcopy(document)
    unsigned.pop("artifactSha256", None)
    _require(canonical_sha256(unsigned) == digest, "packet plan artifactSha256 drifted")

    scope = document.get("scope")
    _require(scope == {
        "effectAssetIds": [spec["effectAssetId"] for spec in TARGET_DOCUMENT_SPECS],
        "documentCount": 5,
        "occurrenceCount": 131,
        "carrierCounts": EXPECTED_CARRIER_COUNTS,
        "priorityCandidateCount": 8,
    }, "packet plan scope drifted")
    _require(document.get("policies") == {
        "authoredWriter": False,
        "registryWriter": False,
        "cppWriter": False,
        "enabledPacketGeneration": "FORBIDDEN_UNTIL_COMPILED_TUPLE_AND_ACTUAL_ADAPTER",
        "rendererDispatch": "PROGRAM_LAYOUT_ADAPTER_TUPLE_ONLY",
        "skillSpecificRendererSwitch": "FORBIDDEN",
        "visualAdmission": "USER_ONLY",
    }, "packet plan policies drifted")

    inputs = document.get("inputs")
    _require(isinstance(inputs, dict), "packet plan inputs are absent")
    _require_exact_keys(inputs, {"tupleInventory", "files"}, "packet plan inputs")
    tuple_input = inputs.get("tupleInventory")
    _require(isinstance(tuple_input, dict), "tuple inventory generation input is absent")
    _require(tuple_input == {
        "builder": TUPLE_INVENTORY_BUILDER_ID,
        "mode": "IN_MEMORY_CURRENT_HEAD",
        "checkedInArtifactUsed": False,
        "artifactSha256": tuple_input.get("artifactSha256"),
    }, "tuple inventory generation mode drifted")
    _require_sha256(
        tuple_input.get("artifactSha256"),
        "in-memory tuple inventory artifactSha256",
    )
    file_inputs = inputs.get("files")
    _require(isinstance(file_inputs, list) and file_inputs, "packet plan file inputs are absent")
    input_paths: set[str] = set()
    for row in file_inputs:
        _require_exact_keys(row, {"path", "roles", "rawSha256", "byteSize", "canonicalJsonSha256"}, "input identity")
        path = _safe_relative_path(row.get("path"), "input identity path")
        _require(path not in input_paths, "input identity path is duplicated")
        input_paths.add(path)
        _require_sha256(row.get("rawSha256"), "input rawSha256")
        _require(isinstance(row.get("byteSize"), int) and row["byteSize"] > 0, "input byteSize is invalid")
        _require(isinstance(row.get("roles"), list) and row["roles"] == sorted(set(row["roles"])) and row["roles"], "input roles drifted")
        if row.get("canonicalJsonSha256") is not None:
            _require_sha256(row["canonicalJsonSha256"], "input canonicalJsonSha256")
    _require(
        TUPLE_INVENTORY_BUILDER_PATH.as_posix() in input_paths,
        "tuple inventory builder byte identity is absent",
    )
    _require(
        "Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json"
        not in input_paths,
        "checked-in Track B tuple inventory must not be used",
    )

    target_documents = document.get("targetDocuments")
    _require(isinstance(target_documents, list) and len(target_documents) == 5, "target document denominator drifted")
    for row, spec in zip(target_documents, TARGET_DOCUMENT_SPECS, strict=True):
        _require(row.get("effectAssetId") == spec["effectAssetId"], "target document order drifted")
        _require(row.get("path") == spec["path"], "target document path drifted")
        _require(row.get("rawSha256") == spec["rawSha256"], "target raw identity drifted")
        _require(row.get("canonicalJsonSha256") == spec["canonicalJsonSha256"], "target semantic identity drifted")
        _require(row.get("occurrenceCount") == spec["occurrenceCount"], "target occurrence count drifted")
        _require(row.get("carrierCounts") == spec["carrierCounts"], "target carrier counts drifted")
        _require(row.get("enabledInlineExecutionCount") == 0, "target gained enabled inline execution")
        _require(row.get("runtimeRegistryBindingCount") == 0, "target gained registry binding")
        _require(row.get("authoredByteMutationAllowed") is False, "target authored mutation became allowed")

    families = document.get("candidateFamilies")
    _require(isinstance(families, list) and len(families) == 3, "candidate family denominator drifted")
    family_map = _index_unique(families, "candidateFamilyId", "candidate families")
    expected_family_ids = {spec["candidateFamilyId"] for spec in FAMILY_SPECS}
    _require(set(family_map) == expected_family_ids, "candidate family identities drifted")
    expected_priority_keys: set[tuple[str, str]] = set()
    for spec in FAMILY_SPECS:
        family = family_map[spec["candidateFamilyId"]]
        _require(family.get("disposition") == "PACKET_MATERIALIZATION_PENDING", "candidate family disposition drifted")
        _require(family.get("targetEffectAssetId") == spec["effectAssetId"], "candidate target asset drifted")
        _require(family.get("targetElementIds") == list(spec["elementIds"]), "candidate target elements drifted")
        expected_priority_keys.update((spec["effectAssetId"], value) for value in spec["elementIds"])
        source = family.get("source") or {}
        _require(source.get("sourceContractRowSha256") == spec["sourceContractRowSha256"], "candidate source row identity drifted")
        _require(source.get("sourceParametersSha256") == spec["sourceParametersSha256"], "candidate source parameter identity drifted")
        _require(source.get("renderState") == spec["sourceRenderState"], "candidate source state drifted")
        program = family.get("program") or {}
        _require(program.get("programCandidateId") == spec["programCandidateId"], "candidate program id drifted")
        _require(program.get("dxbcSha256") == spec["dxbcSha256"], "candidate DXBC drifted")
        _require(program.get("hlslSha256") == spec["hlslSha256"], "candidate HLSL drifted")
        _require(program.get("functionName") == spec["functionName"], "candidate function drifted")
        _require(program.get("compiledRuntimeProgramId") is None, "candidate gained compiled runtime program")
        named = family.get("namedAbi") or {}
        _require(named.get("layoutEvidenceId") == spec["layoutEvidenceId"], "candidate layout evidence drifted")
        _require(named.get("bindingSemanticSha256") == spec["bindingSemanticSha256"], "candidate binding semantic drifted")
        _require(named.get("runtimePacketTopologyMaterialized") is False, "candidate packet topology became materialized")
        _require(named.get("compiledRuntimeLayoutId") is None, "candidate gained runtime layout")
        adapter = family.get("adapter") or {}
        _require(adapter.get("compiledRuntimeAdapterId") is None, "candidate gained runtime adapter")
        _require(adapter.get("actualDrawVerified") is False, "candidate overclaims an actual draw")
        descriptor = family.get("descriptor") or {}
        _require(descriptor.get("runtimeDescriptorId") is None, "candidate gained runtime descriptor")
        _require(descriptor.get("packetMaterialized") is False, "candidate packet became materialized")
        _require(family.get("runtimeAdmission") is False, "candidate gained runtime admission")
        blockers = family.get("blockers")
        _require(isinstance(blockers, list) and blockers == sorted(set(blockers)) and "PACKET_MATERIALIZATION_PENDING" in blockers, "candidate blockers drifted")

    occurrences = document.get("occurrences")
    _require(isinstance(occurrences, list) and len(occurrences) == 131, "occurrence denominator drifted")
    occurrence_ids: set[str] = set()
    element_keys: set[tuple[str, str]] = set()
    carrier_counts: Counter[str] = Counter()
    inline_counts: Counter[str] = Counter()
    dispositions: Counter[str] = Counter()
    runtime_admission_count = 0
    priority_keys: set[tuple[str, str]] = set()
    for row in occurrences:
        occurrence_id = row.get("occurrenceId")
        _require(isinstance(occurrence_id, str) and occurrence_id and occurrence_id not in occurrence_ids, "occurrence id is invalid or duplicated")
        occurrence_ids.add(occurrence_id)
        key = (row.get("effectAssetId"), row.get("elementId"))
        _require(key not in element_keys, "target element key is duplicated")
        element_keys.add(key)
        _require(row.get("carrier") in EXPECTED_CARRIER_COUNTS, "occurrence carrier is out of scope")
        carrier_counts[row["carrier"]] += 1
        _require(row.get("inlineExecutionEnabled") is False, "occurrence gained an enabled packet")
        _require(row.get("inlineExecutionStatus") in EXPECTED_INLINE_STATUS_COUNTS, "inline status drifted")
        inline_counts[row["inlineExecutionStatus"]] += 1
        _require(row.get("runtimeAdmission") is False, "occurrence gained runtime admission")
        runtime_admission_count += int(row["runtimeAdmission"] is True)
        _require(row.get("proposedEnabledExecution") is None, "plan contains an enabled packet payload")
        disposition = row.get("packetDisposition")
        _require(disposition in ("BLOCKED", "PACKET_MATERIALIZATION_PENDING"), "packet disposition drifted")
        dispositions[disposition] += 1
        blockers = row.get("blockers")
        _require(isinstance(blockers, list) and blockers == sorted(set(blockers)) and blockers, "occurrence blockers are absent or unstable")
        if disposition == "PACKET_MATERIALIZATION_PENDING":
            priority_keys.add(key)
            _require(row.get("candidateFamilyId") in expected_family_ids, "priority row lacks a candidate family")
            _require("PACKET_MATERIALIZATION_PENDING" in blockers, "priority row lacks pending blocker")
        else:
            _require(row.get("candidateFamilyId") is None, "blocked row gained a candidate family")
            _require("REPRESENTATIVE_PACKET_ADMISSION_NOT_CLOSED" in blockers, "blocked row lacks explicit admission blocker")
    _require(priority_keys == expected_priority_keys, "priority occurrence set drifted")
    _require({key: carrier_counts.get(key, 0) for key in EXPECTED_CARRIER_COUNTS} == EXPECTED_CARRIER_COUNTS, "occurrence carrier totals drifted")
    _require(dict(sorted(inline_counts.items())) == EXPECTED_INLINE_STATUS_COUNTS, "occurrence inline totals drifted")
    _require(dispositions == {"BLOCKED": 123, "PACKET_MATERIALIZATION_PENDING": 8}, "occurrence disposition totals drifted")
    _require(runtime_admission_count == 0, "runtime admission count is nonzero")

    summary = document.get("summary") or {}
    _require(summary == {
        "documentCount": 5,
        "occurrenceCount": 131,
        "carrierCounts": EXPECTED_CARRIER_COUNTS,
        "inlineExecutionStatusCounts": EXPECTED_INLINE_STATUS_COUNTS,
        "enabledInlineExecutionCount": 0,
        "runtimeRegistryBindingCount": 0,
        "runtimeAdmissionCount": 0,
        "compiledTupleCount": 0,
        "actualAdapterDrawVerifiedCount": 0,
        "packetMaterializationPendingCount": 8,
        "blockedCount": 123,
        "candidateFamilyCount": 3,
        "authoredMutationCount": 0,
    }, "packet plan summary drifted")
    admission = document.get("admission") or {}
    _require(admission.get("runtimeExecution") is False, "plan gained runtime execution")
    _require(admission.get("productAdmission") is False, "plan gained product admission")
    _require(admission.get("enabledPacketEmission") is False, "plan enabled packet emission")
    _require(admission.get("currentProductPreserved") is True, "plan no longer preserves Product")
    transaction = document.get("transaction") or {}
    _require(transaction.get("writablePath") == OUTPUT_RELATIVE_PATH.as_posix(), "plan writer target drifted")
    _require(transaction.get("authoredDocumentsByteStable") is True, "authored byte stability is not sealed")


def validate_input_snapshot(document: dict[str, Any], repository_root: Path) -> None:
    root = repository_root.resolve()
    for row in document.get("inputs", {}).get("files", []):
        relative = _safe_relative_path(row["path"], "snapshot input path")
        absolute = root / Path(relative)
        _require(absolute.is_file(), "snapshot input is absent: " + relative)
        payload = absolute.read_bytes()
        _require(len(payload) == row["byteSize"], "snapshot input size drifted: " + relative)
        _require(sha256_bytes(payload) == row["rawSha256"], "snapshot input bytes drifted: " + relative)
        if row["canonicalJsonSha256"] is not None:
            parsed = decode_json(payload, relative)
            _require(canonical_sha256(parsed) == row["canonicalJsonSha256"], "snapshot input semantics drifted: " + relative)


def _target_byte_snapshot(repository_root: Path) -> dict[str, str]:
    return {
        spec["path"]: sha256_bytes((repository_root / spec["path"]).read_bytes())
        for spec in TARGET_DOCUMENT_SPECS
    }


def write_plan(document: dict[str, Any], repository_root: Path) -> None:
    root = repository_root.resolve()
    validate_plan(document)
    validate_input_snapshot(document, root)
    before = _target_byte_snapshot(root)
    _require(before == {spec["path"]: spec["rawSha256"] for spec in TARGET_DOCUMENT_SPECS}, "target bytes changed before plan write")
    output = root / OUTPUT_RELATIVE_PATH
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary: Path | None = None
    try:
        descriptor, name = tempfile.mkstemp(
            prefix=output.name + ".", suffix=".tmp", dir=output.parent
        )
        temporary = Path(name)
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(pretty_json_bytes(document))
            stream.flush()
            os.fsync(stream.fileno())
        staged = decode_json(temporary.read_bytes(), temporary.name)
        _require(isinstance(staged, dict), "staged packet plan root is not an object")
        validate_plan(staged)
        validate_input_snapshot(staged, root)
        _require(_target_byte_snapshot(root) == before, "target bytes changed while staging plan")
        os.replace(temporary, output)
        temporary = None
    finally:
        if temporary is not None and temporary.exists():
            temporary.unlink()
    _require(_target_byte_snapshot(root) == before, "target bytes changed after plan write")


def run(repository_root: Path, check: bool) -> int:
    root = repository_root.resolve()
    document = build_plan(root)
    expected = pretty_json_bytes(document)
    output = root / OUTPUT_RELATIVE_PATH
    if check:
        validate_input_snapshot(document, root)
        if not output.is_file() or output.read_bytes() != expected:
            print("STALE: " + OUTPUT_RELATIVE_PATH.as_posix(), file=sys.stderr)
            return 1
        print(
            "PASS: representative packet plan is current "
            "(131 occurrences, 8 pending, 0 enabled, 0 admitted)"
        )
        return 0
    write_plan(document, root)
    print(
        "WROTE: " + OUTPUT_RELATIVE_PATH.as_posix()
        + " (plan only; authored/registry/C++ unchanged)"
    )
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check",
        action="store_true",
        help="fail unless the checked-in plan exactly matches all sealed inputs",
    )
    parser.add_argument(
        "--repository-root", type=Path, default=REPOSITORY_ROOT
    )
    arguments = parser.parse_args(argv)
    try:
        return run(arguments.repository_root, arguments.check)
    except (OSError, PlanError, ValueError) as error:
        print("ERROR: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
