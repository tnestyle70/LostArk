# 2026-08-11 R8 Offline Raw Resource Preparation Plan

## 현재 실제 반영 상태와 이번 경계

작업 기준선은 `codex/artist-f-reconstructed-integration-v1`의
`7d3e957f4d93bfd1416fa6a05d5d7fa8f46c12a2`다. Artist F M0 구현 task
`019fef62-5979-7ab2-8b1b-13b6d6891ac8`와 GPU sampler/state schema는 별도 lane이며,
이번 결과는 어느 쪽의 prerequisite 또는 runtime authority가 아니다.

Artist 31470을 제외한 4-class R8 분모는 51 skill, 74 stage, 113 clip occurrence,
5,232 imported source occurrence이고 product cue는 별도 101개다. Valtan 분모는 170 action,
2,464 stage, 2,378 clip, 21,931 notify다. Valtan typed raw occurrence는 Particle 6,159,
Decal 536, Trail 430, Material 606, Camera 1,022이고 generic Effect 3,787은 quarantine으로
유지한다.

clean baseline 조사에서 Artist 31930 imported document와 Warlord 17110 normalized graph의
기대 SHA가 현재 tracked byte SHA와 다르다. 따라서 첫 변경 단위는 exact denominator와 package/object
요청을 재현하되 이 두 provenance failure를 PASS로 승격하지 않는 frozen checkpoint다. 그 다음 변경
단위만 fresh empty staging에서 raw DDS와 scale-1 offline WModel을 만들고 구조를 검사한다.

`Client/Bin/Resources`, runtime Catalog/DataFiles, shared C++, M0/GPU/Product 파일은 수정하지 않는다.
candidate ID는 case-sensitive Resources-relative 문자열일 뿐 실제 canonical Resources에 publish하지 않는다.

## 추가·수정할 파일

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/.codex/worktrees/0109/LostArk/Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py` | R8 분모, portable provenance, logical object/package mapping, fresh export, raw DDS/WModel 검사와 dedup report를 생성한다. |
| 추가 | `C:/Users/user/.codex/worktrees/0109/LostArk/Tools/LevelPlacementExtractor/test_prepare_r8_raw_resource_inventory.py` | pin mismatch, absolute path, stale staging, DDS payload, WModel index/bounds, collision과 deterministic serialization을 검증한다. |
| 추가 | `C:/Users/user/.codex/worktrees/0109/LostArk/Data/Effects/Imported/RawResourceInventory/R8.raw-resource-denominator.checkpoint.json` | exact denominator, occurrence/source-system/cue projection, physical UPK identity와 현재 missing/provenance failure를 봉인한다. |
| 추가 | `C:/Users/user/.codex/worktrees/0109/LostArk/Data/Effects/Imported/RawResourceInventory/R8.raw-resource-inventory-v1.json` | fresh staging raw byte identity, strict DDS/WModel inspection, candidate ID와 dedup/collision/missing 결과를 담는 최종 offline input이다. |
| 추가 | `C:/Users/user/.codex/worktrees/0109/LostArk/.md/GB/08-11/2026-08-11_R8_OFFLINE_RAW_RESOURCE_PREPARATION_RESULT.md` | 두 commit의 실제 검증과 frozen manifest SHA, 남은 GPU binding 경계를 기록한다. |

Python/JSON/Markdown만 추가하므로 `.vcxproj`, `.vcxproj.filters`, MSBuild lease는 해당하지 않는다.

## `prepare_r8_raw_resource_inventory.py`의 책임과 흐름

`checkpoint` mode는 네 class stage manifest에서 현재 skill/stage/clip 분모를 읽고, 각 stage가 pin한
conversion receipt와 imported document를 exact path/SHA로 검증한다. source occurrence는 conversion
receipt의 `elementIds`를 source system/emitter/LOD와 결합해 세며 `summary.emittedElementCount`와 다르면
거부한다. 101 product cue는 FourClass rollout에서 별도 projection으로 보존한다.

DimensionMaster, LanceMaster, Artist, Warlord의 current product receipt가 pin한 normalized graph에서
mesh/texture binding을 수집한다. Warlord는 이 graph projection만으로 ownership을 확정하지 않고 frozen
계획의 정본인 action-bound resource catalog를 98개 occurrence-backed source system으로 다시 join한다.
catalog가 소유하지만 graph binding에 없는 `fx_tex_02.fx_d_environ_018`은 Warlord 17820 consumer를 보존하고
catalog/graph disagreement blocker로 남긴다. 따라서 class request 분모는 1,327, casefold union은 835다.
Valtan은 Action-only resource catalog의 377개 요청만 사용한다. logical package는 source object path와
일치해야 하고 physical package basename은 resolution/catalog row에서 가져온다. installed Packages의
exact-case basename, byteSize, SHA-256을 기록하되 절대 package root는 출력하지 않는다.

4-class source system은 5,232 occurrence를 실제로 소유한 424개와 normalized graph에만 있는 inactive 18개를
분리하고 graph 전체 442개도 보존한다. raw object physical UPK는 27개, Valtan action graph/direct UPK는
32개다. 두 pin mismatch, 14 absolute source hint, Warlord catalog/graph disagreement 1개, current installed
`data3.lpk` drift 1개를 evidence-derived blocker 18개로 봉인한다. validator는 report 배열을 신뢰하지 않고
pinned artifact/path kind, per-consumer ownership, package status와 Valtan raw provenance에서 같은 blocker
projection을 다시 계산한다.

Valtan occurrence는 세 pinned Action source document의 action/stage/clip/notify projection을 합산한다.
PlayParticleEffect와 quarantined DefaultParticle, Decal, Trails/TrailGhostEffect,
PawnMaterialChange/PawnMaterialParam, ViewShake를 원형 source type으로 유지한다. generic Effect는 typed로
변환하지 않고 quarantine reason과 함께 유지한다. raw LOA와 extraction manifest는 portable basename,
byteSize, SHA-256만 출력한다.

`extract` mode는 checkpoint raw SHA를 staging identity에 포함하고 corpus별 target이 존재하면 write 전에
실패한다. UModel은 exact object batches를 `-dds -gltf -nooverwrite`로 실행하지만 empty-root 검사가 먼저라
stale tree를 재사용하지 않는다. TGA를 DDS로 재인코딩하지 않는다. mesh는 pinned hydrated
ModelAssetConverter를 `--pretransform --scale 1 --no-auto-textures`로만 실행하고 canonical Resources 밖의
offline staging에 둔다.

`inspect` mode는 DDS magic/header/format/dimension/mip/payload length만 검증하며 sampler와 SRV colourspace를
결정하지 않는다. WModel은 outer/model/mesh/submesh header, vertex/index stride/count/range, finite position,
derived bounds를 검사한다. scale 1 변환 사실은 provenance일 뿐 final `geometryPreScale` 결정으로 봉인하지
않는다. 동일 SHA raw payload는 한 blob으로 dedup하고 source object consumer를 모두 연결한다. exact-case,
casefold, candidate-name collision과 basename ambiguity는 별도 report이며 자동 정상화하지 않는다.

## 파일별 전체 구현 코드

### 5-1. C:/Users/user/.codex/worktrees/0109/LostArk/Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py

변경 종류: 추가
적용 위치: 새 Python 파일 전체

```python
#!/usr/bin/env python3
"""Prepare portable, offline-only R8 raw resource inventory evidence.

The checkpoint path deliberately does not export, cook, publish, or bind runtime
resources.  It freezes exact corpus denominators, source/object requests, physical
package byte identities, and every provenance/missing condition needed by a later
fresh-staging extraction run.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import re
import subprocess
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Iterable


SOURCE_COMMIT = "7d3e957f4d93bfd1416fa6a05d5d7fa8f46c12a2"
ROLLOUT_PATH = Path(
    "Data/Effects/AuthoredCorrections/Generated/"
    "FourClassCombat.authored-product-rollout.json"
)
CLASS_NAMES = ("DimensionMaster", "LanceMaster", "Artist", "Warlord")
CLASS_ENUMS = {
    "DimensionMaster": "DIMENSIONMASTER",
    "LanceMaster": "LANCE_MASTER",
    "Artist": "ARTIST",
    "Warlord": "WARLORD",
}
CLASS_EXPECTED = {
    "DimensionMaster": (12, 15, 22, 872, 19, 414, 58, 356),
    "LanceMaster": (17, 27, 45, 2199, 41, 332, 59, 273),
    "Artist": (9, 15, 16, 841, 14, 253, 31, 222),
    "Warlord": (13, 17, 30, 1320, 27, 328, 47, 281),
}
CLASS_SOURCE_SYSTEM_EXPECTED = {
    "DimensionMaster": 95,
    "LanceMaster": 179,
    "Artist": 70,
    "Warlord": 98,
}
FOUR_CLASS_EXPECTED = {
    "skillCount": 51,
    "stageCount": 74,
    "clipOccurrenceCount": 113,
    "sourceOccurrenceCount": 5232,
    "productCueCount": 101,
    "classAssetRequestCount": 1327,
    "uniqueAssetRequestCount": 835,
    "physicalPackageCount": 27,
    "sourceSystemCount": 442,
}
VALTAN_EXPECTED = {
    "actionCount": 170,
    "stageCount": 2464,
    "clipOccurrenceCount": 2378,
    "notifyOccurrenceCount": 21931,
    "sourceSystemCount": 165,
    "assetRequestCount": 377,
    "meshRequestCount": 47,
    "textureRequestCount": 330,
    "physicalPackageCount": 32,
    "typedOccurrenceCount": 8753,
    "genericEffectQuarantineCount": 3787,
}
VALTAN_DOCUMENTS = {
    "MN_RPBF_00": "MN_RPBF_00.action-effects.json",
    "MN_RPBF_01_1": "MN_RPBF_01_1.action-effects.json",
    "MN_RPBF_02_2": "MN_RPBF_02_2.action-effects.json",
}
VALTAN_DOCUMENT_SHA = {
    "MN_RPBF_00": "6b80d5b3fbe033239abe2c612f880f083f08952e89f87a1a583f084d4b8dfd59",
    "MN_RPBF_01_1": "5d8fd0d83b1bc4eb58309b4b1cfd1345974e3bd32512ca082bf276a93b74d278",
    "MN_RPBF_02_2": "44ff4828986a96129d802556ef5b9ed5853d0a78504c4a4b5220b03a8441a9a3",
}
VALTAN_LOA = {
    "MN_RPBF_00": (
        "MN_RPBF_00.loa",
        7450184,
        "f61df383bf20634ccdc5b0db3eb9dde1bc62c78717ca763a325e56285e370797",
    ),
    "MN_RPBF_01_1": (
        "MN_RPBF_01-1.loa",
        366062,
        "6fea6fb228d95a019fa9c6d42e2a298ca334be20ed186a84c98027015846fc2a",
    ),
    "MN_RPBF_02_2": (
        "MN_RPBF_02-2.loa",
        645481,
        "60b6b1b17633e76a7a02a3c2514f2e54eebdbc7625354da697d46b924488ea4d",
    ),
}
VALTAN_EXTRACTION_MANIFEST_SHA = (
    "02d104c24b5df625b3bf9bf021513f734e763a666042726eeafd62192effff54"
)
VALTAN_ARCHIVE_PIN = {
    "fileName": "data3.lpk",
    "byteSize": 53879640,
    "sha256": "52fbca9b982f8394a6fe492de122715c4dc8b0f07442f8e9d864b406141e2ff5",
}
RAW_NOTIFY_TYPES = {
    "PlayParticleEffect": "PARTICLE",
    "DefaultParticle": "PARTICLE",
    "PlayDecalEffect": "DECAL",
    "Trails": "TRAIL",
    "TrailGhostEffect": "TRAIL",
    "PawnMaterialChange": "MATERIAL",
    "PawnMaterialParam": "MATERIAL",
    "ViewShake": "CAMERA",
}
EXPECTED_TYPED_GROUPS = {
    "PARTICLE": 6159,
    "DECAL": 536,
    "TRAIL": 430,
    "MATERIAL": 606,
    "CAMERA": 1022,
}
FOUR_CLASS_OCCURRENCE_SOURCE_SYSTEM_COUNT = 424
FOUR_CLASS_GRAPH_ONLY_SOURCE_SYSTEM_COUNT = 18
EXPECTED_CHECKPOINT_BLOCKER_PROJECTION_SHA256 = (
    "1c8ed35f509e16281c8214d6920881714bdffcbaac451c2ddb3b05c1acdf8a4d"
)
GPU_SCHEMA_BOUNDARY = {
    "sampler": "UNFINALIZED_GPU_SCHEMA_PENDING",
    "rasterizerState": "UNFINALIZED_GPU_SCHEMA_PENDING",
    "depthStencilState": "UNFINALIZED_GPU_SCHEMA_PENDING",
    "srvColourspace": "UNFINALIZED_GPU_SCHEMA_PENDING",
    "neutralProvider": "UNFINALIZED_GPU_SCHEMA_PENDING",
    "shaderPermutation": "UNFINALIZED_GPU_SCHEMA_PENDING",
    "materialPacking": "UNFINALIZED_GPU_SCHEMA_PENDING",
    "recipeSlot": "UNFINALIZED_GPU_SCHEMA_PENDING",
    "geometryPreScale": "UNFINALIZED_GPU_SCHEMA_PENDING",
    "cache": "UNFINALIZED_GPU_SCHEMA_PENDING",
    "rendererPacket": "UNFINALIZED_GPU_SCHEMA_PENDING",
    "finalBindingGenerated": False,
}
SAFE_SEGMENT = re.compile(r"^[A-Za-z0-9_+\-]+$")
DRIVE_PATH = re.compile(r"^[A-Za-z]:")
PACKAGE_SUBSTITUTIONS = {
    "Q": ("QP", "QD", "QW", "Q4"),
    "-": ("QL", "QB", "QO", "Q5"),
    "_": ("QC", "QN", "QT", "Q9"),
    "X": ("XU", "XN", "XH", "X3"),
    "!": ("XW", "XS", "XZ", "X0"),
}


class InventoryError(RuntimeError):
    """Raised when an exact offline inventory invariant is violated."""


def _strict_object(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise InventoryError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(
            path.read_text(encoding="utf-8-sig"),
            object_pairs_hook=_strict_object,
            parse_constant=lambda value: (_ for _ in ()).throw(
                InventoryError(f"non-finite JSON token {value} in {path}")
            ),
        )
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise InventoryError(f"cannot read strict JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise InventoryError(f"JSON root must be an object: {path}")
    return value


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def canonical_bytes(value: Any) -> bytes:
    return (
        json.dumps(
            value,
            ensure_ascii=False,
            allow_nan=False,
            sort_keys=True,
            separators=(",", ":"),
        )
        + "\n"
    ).encode("utf-8")


def compute_self_digest(document: dict[str, Any]) -> str:
    projection = copy.deepcopy(document)
    projection.pop("selfDigest", None)
    return hashlib.sha256(canonical_bytes(projection)).hexdigest()


def add_self_digest(document: dict[str, Any]) -> dict[str, Any]:
    result = copy.deepcopy(document)
    result["selfDigest"] = compute_self_digest(result)
    return result


def file_identity(path: Path, locator: str, kind: str) -> dict[str, Any]:
    if not path.is_file():
        raise InventoryError(f"missing input file for {locator}: {path}")
    return {
        "locator": locator,
        "kinds": [kind],
        "fileName": path.name,
        "byteSize": path.stat().st_size,
        "sha256": sha256_file(path),
    }


def ensure_repo_path(repo_root: Path, raw: str) -> tuple[Path, str]:
    candidate = Path(raw)
    if candidate.is_absolute():
        return candidate, "ABSOLUTE_HINT"
    path = (repo_root / candidate).resolve()
    try:
        relative = path.relative_to(repo_root.resolve()).as_posix()
    except ValueError as error:
        raise InventoryError(f"repo path escapes root: {raw}") from error
    if ".." in Path(relative).parts:
        raise InventoryError(f"repo path contains parent traversal: {raw}")
    return path, relative


def register_input(
    inputs: dict[str, dict[str, Any]], path: Path, locator: str, kind: str
) -> dict[str, Any]:
    identity = file_identity(path, locator, kind)
    previous = inputs.get(locator)
    if previous is not None:
        comparable_previous = {key: value for key, value in previous.items() if key != "kinds"}
        comparable_identity = {key: value for key, value in identity.items() if key != "kinds"}
        if comparable_previous != comparable_identity:
            raise InventoryError(f"input locator changed identity: {locator}")
        previous["kinds"] = sorted(set(previous["kinds"]) | {kind})
        return previous
    inputs[locator] = identity
    return identity


def candidate_id(corpus: str, role: str, source_asset_path: str) -> str:
    segments = source_asset_path.split(".")
    if len(segments) < 2 or any(not SAFE_SEGMENT.fullmatch(row) for row in segments):
        raise InventoryError(f"unsafe source object path: {source_asset_path}")
    if role == "mesh":
        directory, extension = "Meshes", "wmodel"
    elif role == "texture":
        directory, extension = "Textures", "dds"
    else:
        raise InventoryError(f"unsupported raw role: {role}")
    return "/".join(
        ["Effect", "RawCandidates", corpus, directory, *segments[:-1], segments[-1] + "." + extension]
    )


def stable_request_id(corpus: str, role: str, source_asset_path: str) -> str:
    payload = f"{corpus}\0{role}\0{source_asset_path}".encode("utf-8")
    return f"rawreq.{corpus.casefold()}.{hashlib.sha256(payload).hexdigest()[:20]}"


def escape_package_name(value: str) -> str:
    result = ""
    for character in value.upper():
        choices = PACKAGE_SUBSTITUTIONS.get(character)
        result += choices[len(result) % 4] if choices else character
    return result


def obfuscate_package_name(logical_name: str) -> str:
    """Mirror Lost Ark's observed logical-to-physical package-name mapping."""

    unpadded_length = len(logical_name)
    escaped = escape_package_name(logical_name)
    if unpadded_length < 20:
        escaped = escape_package_name(
            logical_name + "!" + "." * (20 - unpadded_length - 1)
        )
    characters = list(escaped)
    output = []
    length = len(characters)
    for index, character in enumerate(characters):
        value = ord(character)
        if "0" <= character <= "9":
            value += 43
        encoded = (length + 7 * (value - 65)) % 36 + 65
        if encoded >= 91:
            encoded -= 43
        encoded_character = chr(encoded)
        output.append(encoded_character)
        padding_index = index + unpadded_length
        if padding_index < length and characters[padding_index] == ".":
            characters[padding_index] = encoded_character
    return "".join(output)


def compact_event_occurrences(rows: Iterable[dict[str, Any]]) -> list[dict[str, Any]]:
    result = []
    for row in rows:
        result.append(
            {
                "eventId": str(row.get("eventId", "")),
                "globalTimeSeconds": float(row.get("globalTimeSeconds", 0.0)),
                "durationSeconds": float(row.get("durationSeconds", 0.0)),
            }
        )
    return result


def collect_four_class_stage_corpus(
    repo_root: Path,
    inputs: dict[str, dict[str, Any]],
) -> tuple[dict[str, Any], list[dict[str, Any]], list[dict[str, Any]]]:
    class_summaries = []
    skill_inventory = []
    stage_inventory = []
    clip_inventory = []
    occurrence_inventory = []
    source_systems: dict[tuple[str, str], dict[str, Any]] = {}
    pinned_artifacts = []
    pin_failures = []
    absolute_hints = []

    for class_name in CLASS_NAMES:
        class_enum = CLASS_ENUMS[class_name]
        manifest_rel = Path(
            f"Data/Effects/Imported/{class_name}/"
            f"{class_name}.combat-source-stage-manifest.json"
        )
        manifest_path = repo_root / manifest_rel
        manifest_identity = register_input(
            inputs, manifest_path, manifest_rel.as_posix(), "FOUR_CLASS_STAGE_MANIFEST"
        )
        document = load_json(manifest_path)
        skills = document.get("skills", [])
        if not isinstance(skills, list):
            raise InventoryError(f"skills must be an array: {manifest_rel}")
        artifact_by_skill: dict[int, dict[str, Any]] = {}
        event_to_stages: dict[int, dict[str, set[str]]] = defaultdict(
            lambda: defaultdict(set)
        )
        class_stage_count = 0
        class_clip_count = 0

        for skill in skills:
            skill_id = int(skill["productSkillId"])
            stages = skill.get("stages", [])
            skill_inventory.append(
                {
                    "characterClass": class_enum,
                    "skillId": skill_id,
                    "inputSlot": str(skill.get("inputSlot", "")),
                    "skillKind": str(skill.get("skillKind", "")),
                    "stageCount": len(stages),
                    "clipOccurrenceCount": sum(len(row.get("clips", [])) for row in stages),
                }
            )
            for stage in stages:
                stage_id = str(stage["stageId"])
                clips = stage.get("clips", [])
                class_stage_count += 1
                class_clip_count += len(clips)
                event_ids = sorted(str(row) for row in stage.get("sourceEventIds", []))
                for event_id in event_ids:
                    event_to_stages[skill_id][event_id].add(stage_id)
                stage_inventory.append(
                    {
                        "characterClass": class_enum,
                        "skillId": skill_id,
                        "stageId": stage_id,
                        "stageIndex": int(stage["stageIndex"]),
                        "sourceEventIds": event_ids,
                        "clipOccurrenceCount": len(clips),
                    }
                )
                for clip in clips:
                    clip_inventory.append(
                        {
                            "characterClass": class_enum,
                            "skillId": skill_id,
                            "stageId": stage_id,
                            "stageClipIndex": int(clip["stageClipIndex"]),
                            "sequenceIndex": int(clip["sequenceIndex"]),
                            "clip": str(clip["clip"]),
                            "sourceSkillId": int(clip["sourceSkillId"]),
                        }
                    )
                for artifact in stage.get("sourceArtifacts", []):
                    source_skill_id = int(artifact["sourceSkillId"])
                    previous = artifact_by_skill.get(source_skill_id)
                    if previous is not None and previous != artifact:
                        raise InventoryError(
                            f"conflicting source artifact for {class_name}/{source_skill_id}"
                        )
                    artifact_by_skill[source_skill_id] = artifact

        if len(artifact_by_skill) != len(skills):
            raise InventoryError(
                f"expected one unique source artifact per skill for {class_name}: "
                f"{len(artifact_by_skill)} != {len(skills)}"
            )

        class_occurrence_count = 0
        seen_artifact_refs: set[tuple[int, str, str, str]] = set()
        for source_skill_id, artifact in sorted(artifact_by_skill.items()):
            for artifact_kind, reference in sorted(artifact.items()):
                if not isinstance(reference, dict):
                    continue
                if "path" not in reference or "sha256" not in reference:
                    continue
                raw_path = str(reference["path"])
                expected_sha = str(reference["sha256"]).casefold()
                dedup_key = (source_skill_id, artifact_kind, raw_path, expected_sha)
                if dedup_key in seen_artifact_refs:
                    continue
                seen_artifact_refs.add(dedup_key)
                resolved, portable = ensure_repo_path(repo_root, raw_path)
                locator = (
                    f"external:FourClass/{class_name}/skill.{source_skill_id}/"
                    f"{artifact_kind}/{resolved.name}"
                    if portable == "ABSOLUTE_HINT"
                    else portable
                )
                actual_sha = sha256_file(resolved) if resolved.is_file() else None
                status = (
                    "MISSING"
                    if actual_sha is None
                    else "MATCH"
                    if actual_sha.casefold() == expected_sha
                    else "MISMATCH"
                )
                row = {
                    "characterClass": class_enum,
                    "skillId": source_skill_id,
                    "artifactKind": artifact_kind,
                    "locator": locator,
                    "pathKind": "EXTERNAL_ABSOLUTE_HINT" if portable == "ABSOLUTE_HINT" else "REPO_RELATIVE",
                    "fileName": resolved.name,
                    "expectedSha256": expected_sha,
                    "actualByteSize": resolved.stat().st_size if resolved.is_file() else None,
                    "actualSha256": actual_sha,
                    "pinStatus": status,
                }
                pinned_artifacts.append(row)
                if resolved.is_file():
                    register_input(inputs, resolved, locator, f"FOUR_CLASS_{artifact_kind.upper()}")
                if status != "MATCH":
                    pin_failures.append(
                        {
                            "code": "SOURCE_ARTIFACT_PIN_" + status,
                            "characterClass": class_enum,
                            "skillId": source_skill_id,
                            "artifactKind": artifact_kind,
                            "locator": locator,
                            "expectedSha256": expected_sha,
                            "actualSha256": actual_sha,
                        }
                    )
                if portable == "ABSOLUTE_HINT":
                    absolute_hints.append(
                        {
                            "code": "SOURCE_STAGE_ABSOLUTE_PATH_HINT_NOT_PORTABLE",
                            "characterClass": class_enum,
                            "skillId": source_skill_id,
                            "artifactKind": artifact_kind,
                            "portableLocator": locator,
                            "pinStatus": status,
                        }
                    )

            conversion_ref = artifact.get("conversionReceipt")
            if not isinstance(conversion_ref, dict):
                raise InventoryError(
                    f"missing conversionReceipt for {class_name}/{source_skill_id}"
                )
            conversion_path, conversion_portable = ensure_repo_path(
                repo_root, str(conversion_ref["path"])
            )
            if conversion_portable == "ABSOLUTE_HINT":
                raise InventoryError("conversion receipts must be repo-relative")
            conversion = load_json(conversion_path)
            imported_reference = artifact.get("importedDocument")
            if not isinstance(imported_reference, dict):
                raise InventoryError(
                    f"missing importedDocument for {class_name}/{source_skill_id}"
                )
            imported_path, imported_portable = ensure_repo_path(
                repo_root, str(imported_reference["path"])
            )
            if imported_portable == "ABSOLUTE_HINT":
                raise InventoryError("imported effect documents must be repo-relative")
            imported_document = load_json(imported_path)
            imported_element_ids = [
                str(row["id"]) for row in imported_document.get("elements", [])
            ]
            if len(imported_element_ids) != len(set(imported_element_ids)):
                raise InventoryError(
                    f"duplicate imported element ID in {class_name}/{source_skill_id}"
                )
            local_ids: set[str] = set()
            for conversion_index, row in enumerate(conversion.get("elementConversions", [])):
                element_ids = [str(value) for value in row.get("elementIds", [])]
                event_occurrences = compact_event_occurrences(row.get("eventOccurrences", []))
                event_ids = sorted({event["eventId"] for event in event_occurrences if event["eventId"]})
                stage_ids = sorted(
                    {
                        stage_id
                        for event_id in event_ids
                        for stage_id in event_to_stages[source_skill_id].get(event_id, set())
                    }
                )
                for element_id in element_ids:
                    if element_id in local_ids:
                        raise InventoryError(
                            f"duplicate element ID in {class_name}/{source_skill_id}: {element_id}"
                        )
                    local_ids.add(element_id)
                    occurrence_inventory.append(
                        {
                            "occurrenceId": (
                                f"four-class/{class_enum.casefold()}/"
                                f"skill.{source_skill_id}/element.{len(local_ids) - 1:04d}"
                            ),
                            "characterClass": class_enum,
                            "skillId": source_skill_id,
                            "elementId": element_id,
                            "conversionIndex": conversion_index,
                            "sourceSystemId": str(row.get("sourceSystemId", "")),
                            "sourceEmitter": str(row.get("sourceEmitter", "")),
                            "sourceLod": str(row.get("sourceLod", "")),
                            "sourceEventOccurrences": event_occurrences,
                            "stageIds": stage_ids,
                        }
                    )
                    system_key = (class_enum, str(row.get("sourceSystemId", "")))
                    system = source_systems.setdefault(
                        system_key,
                        {
                            "characterClass": class_enum,
                            "sourceSystemId": system_key[1],
                            "skillIds": set(),
                            "sourceEmitters": set(),
                            "sourceLods": set(),
                            "occurrenceCount": 0,
                        },
                    )
                    system["skillIds"].add(source_skill_id)
                    system["sourceEmitters"].add(str(row.get("sourceEmitter", "")))
                    system["sourceLods"].add(str(row.get("sourceLod", "")))
                    system["occurrenceCount"] += 1
            emitted = int(conversion.get("summary", {}).get("emittedElementCount", -1))
            if emitted != len(local_ids):
                raise InventoryError(
                    f"emitted element mismatch for {class_name}/{source_skill_id}: "
                    f"{emitted} != {len(local_ids)}"
                )
            if set(imported_element_ids) != local_ids:
                missing_from_receipt = sorted(set(imported_element_ids) - local_ids)
                missing_from_document = sorted(local_ids - set(imported_element_ids))
                raise InventoryError(
                    f"imported document/conversion element mismatch for "
                    f"{class_name}/{source_skill_id}: "
                    f"documentOnly={missing_from_receipt[:5]} "
                    f"receiptOnly={missing_from_document[:5]}"
                )
            class_occurrence_count += len(local_ids)

        expected = CLASS_EXPECTED[class_name]
        actual = (
            len(skills),
            class_stage_count,
            class_clip_count,
            class_occurrence_count,
        )
        if actual != expected[:4]:
            raise InventoryError(
                f"R8 denominator mismatch for {class_name}: {actual} != {expected[:4]}"
            )
        class_summaries.append(
            {
                "characterClass": class_enum,
                "skillCount": actual[0],
                "stageCount": actual[1],
                "clipOccurrenceCount": actual[2],
                "sourceOccurrenceCount": actual[3],
                "productCueCount": expected[4],
                "stageManifest": manifest_identity,
            }
        )

    serialized_systems = []
    for key in sorted(source_systems):
        row = source_systems[key]
        serialized_systems.append(
            {
                "characterClass": row["characterClass"],
                "sourceSystemId": row["sourceSystemId"],
                "skillIds": sorted(row["skillIds"]),
                "sourceEmitterCount": len(row["sourceEmitters"]),
                "sourceLodCount": len(row["sourceLods"]),
                "occurrenceCount": row["occurrenceCount"],
            }
        )

    result = {
        "classSummaries": class_summaries,
        "skills": sorted(skill_inventory, key=lambda row: (row["characterClass"], row["skillId"])),
        "stages": sorted(stage_inventory, key=lambda row: (row["characterClass"], row["skillId"], row["stageIndex"])),
        "clips": sorted(clip_inventory, key=lambda row: (row["characterClass"], row["skillId"], row["sequenceIndex"], row["stageClipIndex"])),
        "occurrenceSourceSystems": serialized_systems,
        "occurrences": sorted(occurrence_inventory, key=lambda row: row["occurrenceId"]),
        "pinnedArtifacts": sorted(
            pinned_artifacts,
            key=lambda row: (row["characterClass"], row["skillId"], row["artifactKind"], row["locator"]),
        ),
    }
    return result, pin_failures, absolute_hints


def collect_product_cues(
    repo_root: Path, inputs: dict[str, dict[str, Any]]
) -> list[dict[str, Any]]:
    path = repo_root / ROLLOUT_PATH
    register_input(inputs, path, ROLLOUT_PATH.as_posix(), "FOUR_CLASS_PRODUCT_ROLLOUT")
    document = load_json(path)
    cues = []
    for row in document.get("productTargets", []):
        cues.append(
            {
                "cueId": str(row["effectAssetId"]),
                "characterClass": str(row["characterClass"]),
                "skillId": int(row["productSkillId"]),
                "stageIndex": int(row["stageIndex"]),
                "stageClipIndex": int(row["stageClipIndex"]),
                "clip": str(row["clip"]),
                "sourceEventIds": sorted(str(value) for value in row.get("sourceEventIds", [])),
                "sourceElementsSha256": str(row["sourceElementsSha256"]).casefold(),
            }
        )
    if len(cues) != FOUR_CLASS_EXPECTED["productCueCount"]:
        raise InventoryError(f"four-class product cue mismatch: {len(cues)}")
    counts = Counter(row["characterClass"] for row in cues)
    for class_name in CLASS_NAMES:
        expected = CLASS_EXPECTED[class_name][4]
        if counts[CLASS_ENUMS[class_name]] != expected:
            raise InventoryError(
                f"product cue mismatch for {class_name}: "
                f"{counts[CLASS_ENUMS[class_name]]} != {expected}"
            )
    return sorted(cues, key=lambda row: row["cueId"])


def load_package_resolution(
    repo_root: Path,
    class_name: str,
    inputs: dict[str, dict[str, Any]],
) -> dict[str, tuple[str, str]]:
    relative = Path(
        f"Data/Effects/Imported/{class_name}/CurrentCombat/"
        f"{class_name}.source-package-resolution.json"
    )
    path = repo_root / relative
    if not path.is_file():
        return {}
    register_input(inputs, path, relative.as_posix(), "SOURCE_PACKAGE_RESOLUTION")
    document = load_json(path)
    result: dict[str, tuple[str, str]] = {}
    for row in document.get("packages", []):
        logical = str(row["logicalPackage"])
        physical = Path(str(row["physicalPackage"])).name
        key = logical.casefold()
        previous = result.get(key)
        value = (logical, physical)
        if previous is not None and previous != value:
            raise InventoryError(f"duplicate logical package mapping in {relative}: {logical}")
        result[key] = value
    return result


def merge_class_request(
    requests: dict[tuple[str, str], dict[str, Any]],
    class_enum: str,
    role: str,
    source_asset_path: str,
    logical_package: str,
    physical_package: str,
    skill_ids: Iterable[int],
    action_ids: Iterable[int],
    source_system_ids: Iterable[str],
    resolution_statuses: Iterable[str],
) -> None:
    key = (role, source_asset_path.casefold())
    row = requests.get(key)
    if row is None:
        row = {
            "role": role,
            "sourceAssetPath": source_asset_path,
            "logicalPackage": logical_package,
            "physicalPackageFileName": physical_package,
            "classConsumers": {},
            "sourceResolutionStatuses": set(),
        }
        requests[key] = row
    else:
        if row["sourceAssetPath"] != source_asset_path:
            raise InventoryError(
                f"case-only source object collision: {row['sourceAssetPath']} / {source_asset_path}"
            )
        if row["physicalPackageFileName"].casefold() != physical_package.casefold():
            raise InventoryError(
                f"one object maps to multiple physical packages: {source_asset_path}"
            )
    consumer = row["classConsumers"].setdefault(
        class_enum,
        {
            "skillIds": set(),
            "actionIds": set(),
            "sourceSystemIds": set(),
            "ownershipStatuses": set(),
        },
    )
    consumer["skillIds"].update(int(value) for value in skill_ids)
    consumer["actionIds"].update(int(value) for value in action_ids)
    consumer["sourceSystemIds"].update(str(value) for value in source_system_ids if value)
    serialized_statuses = {str(value) for value in resolution_statuses if value}
    consumer["ownershipStatuses"].update(serialized_statuses)
    row["sourceResolutionStatuses"].update(serialized_statuses)


def collect_graph_class_requests(
    repo_root: Path,
    class_name: str,
    skill_ids: list[int],
    inputs: dict[str, dict[str, Any]],
) -> tuple[
    dict[tuple[str, str], dict[str, Any]],
    dict[str, tuple[str, str]],
    list[dict[str, Any]],
]:
    package_map = load_package_resolution(repo_root, class_name, inputs)
    used_package_map: dict[str, tuple[str, str]] = {}
    requests: dict[tuple[str, str], dict[str, Any]] = {}
    graph_systems: dict[str, dict[str, Any]] = {}
    class_enum = CLASS_ENUMS[class_name]
    for skill_id in skill_ids:
        receipt_relative = Path(
            f"Data/Effects/Imported/{class_name}/CurrentCombat/ProductReceipts/"
            f"skill.{skill_id}.product-source-receipt.json"
        )
        receipt_path = repo_root / receipt_relative
        register_input(inputs, receipt_path, receipt_relative.as_posix(), "PRODUCT_SOURCE_RECEIPT")
        receipt = load_json(receipt_path)
        graph_reference = receipt.get("normalizedGraph", {})
        graph_path, graph_portable = ensure_repo_path(repo_root, str(graph_reference["path"]))
        graph_locator = (
            f"external:FourClass/{class_name}/CurrentProductGraphs/"
            f"skill.{skill_id}/{graph_path.name}"
            if graph_portable == "ABSOLUTE_HINT"
            else graph_portable
        )
        identity = register_input(
            inputs, graph_path, graph_locator, "NORMALIZED_EFFECT_GRAPH"
        )
        graph_pin_match = (
            identity["sha256"].casefold()
            == str(graph_reference["sha256"]).casefold()
        )
        graph = load_json(graph_path)
        object_systems: dict[str, set[str]] = defaultdict(set)
        for system in graph.get("sourceSystems", []):
            system_id = str(system.get("sourceSystemId", ""))
            system_key = system_id.casefold()
            existing_system = graph_systems.get(system_key)
            if existing_system is None:
                existing_system = {
                    "characterClass": class_enum,
                    "sourceSystemId": system_id,
                    "sourceAsset": str(system.get("sourceAsset", "")),
                    "logicalPackage": str(system.get("logicalPackage", "")),
                    "skillIds": set(),
                }
                graph_systems[system_key] = existing_system
            elif existing_system["sourceSystemId"] != system_id:
                raise InventoryError(
                    f"case-only source system collision for {class_name}: "
                    f"{existing_system['sourceSystemId']} / {system_id}"
                )
            existing_system["skillIds"].add(skill_id)
            for binding in system.get("resourceBindings", []):
                if binding.get("role") in {"mesh", "texture"} and binding.get("objectPath"):
                    object_systems[str(binding["objectPath"]).casefold()].add(system_id)
        for binding in graph.get("runtimeResourceBindings", []):
            role = str(binding.get("role", ""))
            source_path = str(binding.get("sourceObjectPath", ""))
            if role not in {"mesh", "texture"} or not source_path:
                continue
            source_logical = source_path.split(".", 1)[0]
            mapping = package_map.get(source_logical.casefold())
            if mapping is None:
                mapping = (
                    source_logical,
                    obfuscate_package_name(source_logical) + ".upk",
                )
                package_map[source_logical.casefold()] = mapping
            used_package_map[source_logical.casefold()] = mapping
            merge_class_request(
                requests,
                class_enum,
                role,
                source_path,
                mapping[0],
                mapping[1],
                [skill_id],
                [],
                object_systems.get(source_path.casefold(), set()),
                [
                    str(binding.get("resolutionStatus", "")),
                    "SOURCE_GRAPH_PIN_MATCH"
                    if graph_pin_match
                    else "SOURCE_GRAPH_PIN_MISMATCH",
                ],
            )
    serialized_systems = []
    for key in sorted(graph_systems):
        row = graph_systems[key]
        serialized_systems.append(
            {
                "characterClass": row["characterClass"],
                "sourceSystemId": row["sourceSystemId"],
                "sourceAsset": row["sourceAsset"],
                "logicalPackage": row["logicalPackage"],
                "skillIds": sorted(row["skillIds"]),
            }
        )
    return requests, used_package_map, serialized_systems


def collect_warlord_catalog_requests(
    repo_root: Path,
    inputs: dict[str, dict[str, Any]],
    selected_action_ids: set[int],
    selected_source_system_ids: set[str],
) -> tuple[
    dict[tuple[str, str], dict[str, Any]],
    dict[str, tuple[str, str]],
]:
    relative = Path(
        "Data/Effects/Imported/Warlord/Warlord.action-particle-resource-catalog.json"
    )
    path = repo_root / relative
    register_input(inputs, path, relative.as_posix(), "WARLORD_ACTION_RESOURCE_CATALOG")
    document = load_json(path)
    requests: dict[tuple[str, str], dict[str, Any]] = {}
    package_map: dict[str, tuple[str, str]] = {}
    selected_folded = {value.casefold() for value in selected_source_system_ids}
    for asset in document.get("assets", []):
        owned_systems = sorted(
            {
                str(value)
                for value in asset.get("sourceSystems", [])
                if str(value).casefold() in selected_folded
            },
            key=str.casefold,
        )
        if not owned_systems:
            continue
        roles = sorted(
            set(str(value) for value in asset.get("roles", [])) & {"mesh", "texture"}
        )
        if not roles:
            continue
        source_path = str(asset["sourceAssetPath"])
        logical = str(asset["logicalPackage"])
        physical = Path(str(asset["physicalPackage"])).name
        key = logical.casefold()
        previous = package_map.get(key)
        mapping = (logical, physical)
        if previous is not None and previous[1].casefold() != physical.casefold():
            raise InventoryError(f"Warlord catalog physical conflict for {logical}")
        package_map[key] = mapping
        action_ids = {
            int(value) for value in asset.get("actionIds", [])
        } & selected_action_ids
        for role in roles:
            merge_class_request(
                requests,
                "WARLORD",
                role,
                source_path,
                logical,
                physical,
                action_ids,
                action_ids,
                owned_systems,
                [str(asset.get("resolutionStatus", "")), "ACTION_BOUND_CATALOG_OWNERSHIP"],
            )
    return requests, package_map


def graph_header_mapping(path: Path) -> tuple[str, str]:
    with path.open("rb") as source:
        prefix = source.read(128 * 1024).decode("utf-8-sig")
    package_match = re.search(r'"package"\s*:\s*"((?:\\.|[^"\\])*)"', prefix)
    physical_match = re.search(
        r'"physicalPackage"\s*:\s*"((?:\\.|[^"\\])*)"', prefix
    )
    if package_match is None or physical_match is None:
        raise InventoryError(f"particle graph header lacks package mapping: {path}")
    logical = json.loads('"' + package_match.group(1) + '"')
    physical_path = json.loads('"' + physical_match.group(1) + '"')
    return str(logical), Path(str(physical_path)).name


def collect_catalog_requests(
    repo_root: Path,
    relative: Path,
    corpus: str,
    inputs: dict[str, dict[str, Any]],
    selected_action_ids: set[int] | None,
) -> tuple[
    dict[tuple[str, str], dict[str, Any]],
    dict[str, tuple[str, str]],
    list[dict[str, Any]],
]:
    path = repo_root / relative
    register_input(inputs, path, relative.as_posix(), f"{corpus.upper()}_ACTION_RESOURCE_CATALOG")
    document = load_json(path)
    selected_systems = []
    selected_system_ids = set()
    for system in document.get("sourceSystems", []):
        action_ids = {int(value) for value in system.get("actionIds", [])}
        if selected_action_ids is not None and not (action_ids & selected_action_ids):
            continue
        system_id = str(system["sourceAsset"])
        selected_system_ids.add(system_id.casefold())
        selected_systems.append(
            {
                "corpus": corpus,
                "sourceSystemId": system_id,
                "logicalPackage": str(system["logicalPackage"]),
                "actionIds": sorted(
                    action_ids if selected_action_ids is None else action_ids & selected_action_ids
                ),
                "occurrenceCount": int(system.get("occurrenceCount", 0)),
            }
        )

    package_map: dict[str, tuple[str, str]] = {}
    for graph_index, graph in enumerate(document.get("sourcePackageGraphs", [])):
        graph_path = Path(str(graph["graphFile"]))
        logical, physical = graph_header_mapping(graph_path)
        locator = f"external:{corpus}/ParticleGraphs/{graph_path.name}"
        register_input(inputs, graph_path, locator, f"{corpus.upper()}_PARTICLE_GRAPH")
        declared = str(graph["logicalPackage"])
        if declared.casefold() != logical.casefold():
            raise InventoryError(
                f"particle graph logical mismatch at index {graph_index}: {declared} / {logical}"
            )
        key = logical.casefold()
        previous = package_map.get(key)
        mapping = (logical, physical)
        if previous is not None and previous != mapping:
            raise InventoryError(f"conflicting particle graph package mapping: {logical}")
        package_map[key] = mapping

    requests: dict[tuple[str, str], dict[str, Any]] = {}
    class_enum = "WARLORD" if corpus == "FourClass" else "VALTAN"
    for asset in document.get("assets", []):
        action_ids = {int(value) for value in asset.get("actionIds", [])}
        if selected_action_ids is not None and not (action_ids & selected_action_ids):
            continue
        roles = sorted(set(str(value) for value in asset.get("roles", [])) & {"mesh", "texture"})
        if not roles:
            continue
        source_path = str(asset["sourceAssetPath"])
        logical = str(asset["logicalPackage"])
        physical = Path(str(asset["physicalPackage"])).name
        package_key = logical.casefold()
        previous = package_map.get(package_key)
        if previous is not None and previous[1].casefold() != physical.casefold():
            raise InventoryError(f"catalog physical mapping conflict for {logical}")
        if previous is None:
            package_map[package_key] = (logical, physical)
        source_system_ids = [
            str(value)
            for value in asset.get("sourceSystems", [])
            if str(value).casefold() in selected_system_ids
        ]
        selected_actions = action_ids if selected_action_ids is None else action_ids & selected_action_ids
        for role in roles:
            merge_class_request(
                requests,
                class_enum,
                role,
                source_path,
                package_map[package_key][0],
                physical,
                [],
                selected_actions,
                source_system_ids,
                [str(asset.get("resolutionStatus", ""))],
            )
    return requests, package_map, sorted(selected_systems, key=lambda row: row["sourceSystemId"].casefold())


def serialize_requests(
    corpus: str, requests: dict[tuple[str, str], dict[str, Any]]
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    result = []
    case_collisions = []
    candidate_owners: dict[str, str] = {}
    for key in sorted(requests):
        row = requests[key]
        consumers = []
        for class_name in sorted(row["classConsumers"]):
            consumer = row["classConsumers"][class_name]
            consumers.append(
                {
                    "consumer": class_name,
                    "skillIds": sorted(consumer["skillIds"]),
                    "actionIds": sorted(consumer["actionIds"]),
                    "sourceSystemIds": sorted(consumer["sourceSystemIds"], key=str.casefold),
                    "ownershipStatuses": sorted(consumer["ownershipStatuses"]),
                }
            )
        candidate = candidate_id(corpus, row["role"], row["sourceAssetPath"])
        folded = candidate.casefold()
        previous = candidate_owners.get(folded)
        if previous is not None and previous != candidate:
            case_collisions.append(
                {
                    "code": "CASE_ONLY_CANDIDATE_ID_COLLISION",
                    "first": previous,
                    "second": candidate,
                }
            )
        candidate_owners[folded] = candidate
        result.append(
            {
                "requestId": stable_request_id(corpus, row["role"], row["sourceAssetPath"]),
                "role": row["role"],
                "sourceAssetPath": row["sourceAssetPath"],
                "logicalPackage": row["logicalPackage"],
                "physicalPackageFileName": row["physicalPackageFileName"],
                "resourcesRelativeCandidateId": candidate,
                "bindingStatus": "CANDIDATE_ONLY_GPU_SCHEMA_PENDING",
                "consumers": consumers,
                "sourceResolutionStatuses": sorted(row["sourceResolutionStatuses"]),
            }
        )
    return result, case_collisions


def package_identities(
    package_root: Path,
    package_maps: Iterable[dict[str, tuple[str, str]]],
    corpus: str,
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    merged: dict[str, dict[str, Any]] = {}
    aliases: dict[str, set[str]] = defaultdict(set)
    for package_map in package_maps:
        for key, (logical, physical) in package_map.items():
            aliases[key].add(logical)
            previous = merged.get(key)
            if previous is not None and previous["physicalPackageFileName"].casefold() != physical.casefold():
                raise InventoryError(f"logical package maps to multiple physical packages: {logical}")
            if previous is None:
                merged[key] = {
                    "logicalPackage": logical,
                    "physicalPackageFileName": physical,
                }
    by_casefold: dict[str, list[Path]] = defaultdict(list)
    for path in package_root.iterdir():
        if path.is_file():
            by_casefold[path.name.casefold()].append(path)
    rows = []
    reports = []
    for key in sorted(merged):
        row = merged[key]
        physical = row["physicalPackageFileName"]
        matches = by_casefold.get(physical.casefold(), [])
        exact = [path for path in matches if path.name == physical]
        status = "PRESENT_EXACT_CASE"
        selected = exact[0] if len(exact) == 1 and len(matches) == 1 else None
        if not matches:
            status = "MISSING_PHYSICAL_PACKAGE"
        elif len(matches) != 1:
            status = "AMBIGUOUS_CASEFOLD_PHYSICAL_PACKAGE"
        elif not exact:
            status = "PHYSICAL_PACKAGE_CASE_MISMATCH"
        package_row = {
            "corpus": corpus,
            "logicalPackage": row["logicalPackage"],
            "logicalPackageAliases": sorted(aliases[key], key=str.casefold),
            "physicalPackageFileName": physical,
            "resolutionStatus": status,
            "byteSize": selected.stat().st_size if selected is not None else None,
            "sha256": sha256_file(selected) if selected is not None else None,
        }
        rows.append(package_row)
        if status != "PRESENT_EXACT_CASE":
            reports.append(
                {
                    "code": status,
                    "corpus": corpus,
                    "logicalPackage": row["logicalPackage"],
                    "physicalPackageFileName": physical,
                    "casefoldMatches": sorted(path.name for path in matches),
                }
            )
        if len(aliases[key]) > 1:
            reports.append(
                {
                    "code": "LOGICAL_PACKAGE_CASE_ALIASES",
                    "corpus": corpus,
                    "canonicalLogicalPackage": row["logicalPackage"],
                    "aliases": sorted(aliases[key], key=str.casefold),
                }
            )
    return rows, reports


def collect_valtan_occurrences(
    action_root: Path,
    extraction_manifest_path: Path,
    installed_data3: Path,
    inputs: dict[str, dict[str, Any]],
) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    extraction_identity = register_input(
        inputs,
        extraction_manifest_path,
        "external:Valtan/data3-extraction-manifest.json",
        "VALTAN_LOA_EXTRACTION_MANIFEST",
    )
    extraction_document = load_json(extraction_manifest_path)
    extraction_reports = []
    if extraction_identity["sha256"] != VALTAN_EXTRACTION_MANIFEST_SHA:
        extraction_reports.append(
            {
                "code": "VALTAN_EXTRACTION_MANIFEST_PIN_MISMATCH",
                "expectedSha256": VALTAN_EXTRACTION_MANIFEST_SHA,
                "actualSha256": extraction_identity["sha256"],
            }
        )
    archives = extraction_document.get("archives", [])
    archive = next((row for row in archives if str(row.get("name", "")).casefold() == "data3.lpk"), None)
    if archive is None:
        raise InventoryError("data3.lpk archive missing from Valtan extraction manifest")
    archive_provenance = {
        "fileName": str(archive["name"]),
        "byteSize": int(archive["sourceBytes"]),
        "sha256": str(archive["sourceSha256"]).casefold(),
        "expectedEntries": int(archive["expectedEntries"]),
        "extractedEntries": int(archive["extractedEntries"]),
        "failureCount": int(extraction_document.get("failureCount", -1)),
    }
    if {
        "fileName": archive_provenance["fileName"],
        "byteSize": archive_provenance["byteSize"],
        "sha256": archive_provenance["sha256"],
    } != VALTAN_ARCHIVE_PIN:
        extraction_reports.append(
            {
                "code": "VALTAN_SOURCE_ARCHIVE_PIN_MISMATCH",
                "expected": VALTAN_ARCHIVE_PIN,
                "actual": {
                    "fileName": archive_provenance["fileName"],
                    "byteSize": archive_provenance["byteSize"],
                    "sha256": archive_provenance["sha256"],
                },
            }
        )

    current_archive = file_identity(
        installed_data3, "installed-snapshot:data3.lpk", "CURRENT_INSTALLED_VALTAN_ARCHIVE"
    )
    current_archive["matchesPinnedCorpusArchive"] = (
        current_archive["byteSize"] == VALTAN_ARCHIVE_PIN["byteSize"]
        and current_archive["sha256"] == VALTAN_ARCHIVE_PIN["sha256"]
    )
    if not current_archive["matchesPinnedCorpusArchive"]:
        extraction_reports.append(
            {
                "code": "CURRENT_INSTALLED_DATA3_DRIFT_FROM_PINNED_VALTAN_CORPUS",
                "expectedByteSize": VALTAN_ARCHIVE_PIN["byteSize"],
                "expectedSha256": VALTAN_ARCHIVE_PIN["sha256"],
                "actualByteSize": current_archive["byteSize"],
                "actualSha256": current_archive["sha256"],
            }
        )

    entry_by_name = {
        Path(str(row["FullPath"])).name.casefold(): row
        for row in archive.get("entries", [])
        if "/Action/" in str(row.get("FullPath", "")).replace("\\", "/")
    }
    source_documents = []
    raw_loas = []
    actions = []
    stages = []
    clips = []
    occurrences = []
    notify_counts: Counter[str] = Counter()
    typed_groups: Counter[str] = Counter()

    for profile_id, file_name in VALTAN_DOCUMENTS.items():
        document_path = action_root / file_name
        identity = register_input(
            inputs,
            document_path,
            f"external:Valtan/ActionEffects/{file_name}",
            "VALTAN_ACTION_EFFECT_DOCUMENT",
        )
        expected_document_sha = VALTAN_DOCUMENT_SHA[profile_id]
        document_status = "MATCH" if identity["sha256"] == expected_document_sha else "MISMATCH"
        if document_status != "MATCH":
            extraction_reports.append(
                {
                    "code": "VALTAN_ACTION_DOCUMENT_PIN_MISMATCH",
                    "profileId": profile_id,
                    "expectedSha256": expected_document_sha,
                    "actualSha256": identity["sha256"],
                }
            )
        document = load_json(document_path)
        if str(document.get("profileId")) != profile_id:
            raise InventoryError(f"Valtan profile mismatch in {file_name}")
        source_documents.append(
            {
                "profileId": profile_id,
                "locator": identity["locator"],
                "fileName": file_name,
                "byteSize": identity["byteSize"],
                "sha256": identity["sha256"],
                "expectedSha256": expected_document_sha,
                "pinStatus": document_status,
            }
        )
        loa_name, loa_size, loa_sha = VALTAN_LOA[profile_id]
        source = document.get("source", {})
        source_path = Path(str(source.get("path", "")))
        source_actual = file_identity(
            source_path,
            f"external:Valtan/RawAction/{loa_name}",
            "VALTAN_RAW_LOA",
        )
        entry = entry_by_name.get(loa_name.casefold())
        if entry is None:
            raise InventoryError(f"raw LOA entry missing from extraction manifest: {loa_name}")
        loa_status = "MATCH"
        if (
            source_actual["fileName"] != loa_name
            or source_actual["byteSize"] != loa_size
            or source_actual["sha256"] != loa_sha
            or int(source.get("byteSize", -1)) != loa_size
            or str(source.get("sha256", "")).casefold() != loa_sha
            or int(entry.get("outputBytes", -1)) != loa_size
            or str(entry.get("outputSha256", "")).casefold() != loa_sha
        ):
            loa_status = "MISMATCH"
            extraction_reports.append(
                {
                    "code": "VALTAN_RAW_LOA_PIN_MISMATCH",
                    "profileId": profile_id,
                    "fileName": loa_name,
                    "expectedByteSize": loa_size,
                    "expectedSha256": loa_sha,
                    "actualByteSize": source_actual["byteSize"],
                    "actualSha256": source_actual["sha256"],
                }
            )
        raw_loas.append(
            {
                "profileId": profile_id,
                "locator": source_actual["locator"],
                "fileName": loa_name,
                "byteSize": source_actual["byteSize"],
                "sha256": source_actual["sha256"],
                "pinStatus": loa_status,
                "archiveEntry": {
                    "logicalPath": str(entry["FullPath"]).replace("\\", "/"),
                    "index": int(entry["Index"]),
                    "offset": int(entry["Offset"]),
                    "storedSize": int(entry["StoredSize"]),
                    "unpackedSize": int(entry["UnpackedSize"]),
                },
            }
        )
        for action in document.get("actions", []):
            action_id = int(action["actionId"])
            action_key = f"{profile_id}/action.{action_id}"
            actions.append(
                {
                    "actionKey": action_key,
                    "profileId": profile_id,
                    "sourceActionIndex": int(action["sourceActionIndex"]),
                    "actionId": action_id,
                    "sourceOffset": int(action["sourceOffset"]),
                }
            )
            for stage in action.get("stages", []):
                stage_index = int(stage["stageIndex"])
                stage_key = f"{action_key}/stage.{stage_index:03d}"
                animation_clips = stage.get("animationClips", [])
                notifies = stage.get("notifies", [])
                stages.append(
                    {
                        "stageKey": stage_key,
                        "actionKey": action_key,
                        "stageIndex": stage_index,
                        "sourceOffset": int(stage["sourceOffset"]),
                        "clipOccurrenceCount": len(animation_clips),
                        "notifyOccurrenceCount": len(notifies),
                    }
                )
                for clip_index, clip in enumerate(animation_clips):
                    clips.append(
                        {
                            "clipOccurrenceId": f"{stage_key}/clip.{clip_index:03d}",
                            "stageKey": stage_key,
                            "clipName": str(clip["clipName"]),
                            "lengthSeconds": float(clip["lengthSeconds"]),
                            "notifyId": str(clip["notifyId"]),
                        }
                    )
                for notify in notifies:
                    source_type = str(notify["sourceType"])
                    notify_counts[source_type] += 1
                    typed_group = RAW_NOTIFY_TYPES.get(source_type)
                    if typed_group is not None:
                        typed_groups[typed_group] += 1
                    classification = "REFERENCE_ONLY"
                    if source_type == "Effect":
                        classification = "QUARANTINED_GENERIC_EFFECT"
                    elif source_type == "DefaultParticle":
                        classification = "QUARANTINED_DEFAULT_PARTICLE_TYPED_DENOMINATOR"
                    elif typed_group is not None:
                        classification = "RAW_TYPED_INPUT"
                    references = []
                    for reference in notify.get("assetReferences", []):
                        references.append(
                            {
                                "className": str(reference.get("className", "")),
                                "objectPath": str(reference.get("objectPath", "")),
                            }
                        )
                    occurrences.append(
                        {
                            "occurrenceId": f"valtan/{profile_id}/{notify['notifyId']}",
                            "stageKey": stage_key,
                            "sourceType": source_type,
                            "category": str(notify.get("category", "")),
                            "classification": classification,
                            "resolutionStatus": str(notify.get("resolutionStatus", "")),
                            "localTimeSeconds": float(notify.get("localTimeSeconds", 0.0)),
                            "durationSeconds": float(notify.get("durationSeconds", 0.0)),
                            "sourceOffset": int(notify["sourceOffset"]),
                            "assetReferences": references,
                        }
                    )

    if len(actions) != VALTAN_EXPECTED["actionCount"]:
        raise InventoryError(f"Valtan action denominator mismatch: {len(actions)}")
    if len(stages) != VALTAN_EXPECTED["stageCount"]:
        raise InventoryError(f"Valtan stage denominator mismatch: {len(stages)}")
    if len(clips) != VALTAN_EXPECTED["clipOccurrenceCount"]:
        raise InventoryError(f"Valtan clip denominator mismatch: {len(clips)}")
    if len(occurrences) != VALTAN_EXPECTED["notifyOccurrenceCount"]:
        raise InventoryError(f"Valtan notify denominator mismatch: {len(occurrences)}")
    if dict(sorted(typed_groups.items())) != EXPECTED_TYPED_GROUPS:
        raise InventoryError(
            f"Valtan typed denominator mismatch: {dict(typed_groups)} != {EXPECTED_TYPED_GROUPS}"
        )
    if notify_counts["Effect"] != VALTAN_EXPECTED["genericEffectQuarantineCount"]:
        raise InventoryError("Valtan generic Effect quarantine denominator mismatch")

    result = {
        "sourceArchive": archive_provenance,
        "currentInstalledArchiveSnapshot": current_archive,
        "sourceDocuments": sorted(source_documents, key=lambda row: row["profileId"]),
        "rawLoas": sorted(raw_loas, key=lambda row: row["profileId"]),
        "actions": sorted(actions, key=lambda row: row["actionKey"]),
        "stages": sorted(stages, key=lambda row: row["stageKey"]),
        "clips": sorted(clips, key=lambda row: row["clipOccurrenceId"]),
        "occurrences": sorted(occurrences, key=lambda row: row["occurrenceId"]),
        "notifyTypeCounts": dict(sorted(notify_counts.items())),
        "typedGroupCounts": dict(sorted(typed_groups.items())),
    }
    return result, extraction_reports


def serialize_package_maps(
    package_rows: list[dict[str, Any]], expected_count: int, corpus: str
) -> dict[str, Any]:
    if len(package_rows) != expected_count:
        raise InventoryError(
            f"{corpus} physical package denominator mismatch: "
            f"{len(package_rows)} != {expected_count}"
        )
    return {
        "packageCount": len(package_rows),
        "totalByteSize": sum(int(row["byteSize"] or 0) for row in package_rows),
        "packages": package_rows,
    }


def compute_report_blocker_count(reports: dict[str, Any]) -> int:
    physical_blockers = sum(
        1
        for row in reports.get("physicalPackageIssues", [])
        if row.get("code") != "LOGICAL_PACKAGE_CASE_ALIASES"
    )
    return (
        len(reports.get("pinFailures", []))
        + len(reports.get("weakAbsoluteSourceHints", []))
        + physical_blockers
        + len(reports.get("candidateIdCollisions", []))
        + len(reports.get("warlordCatalogGraphDisagreements", []))
        + len(reports.get("valtanProvenanceIssues", []))
    )


def report_blocker_evidence(reports: dict[str, Any]) -> list[dict[str, Any]]:
    rows = []
    for key in (
        "pinFailures",
        "weakAbsoluteSourceHints",
        "candidateIdCollisions",
        "warlordCatalogGraphDisagreements",
        "valtanProvenanceIssues",
    ):
        rows.extend(copy.deepcopy(reports.get(key, [])))
    rows.extend(
        copy.deepcopy(row)
        for row in reports.get("physicalPackageIssues", [])
        if row.get("code") != "LOGICAL_PACKAGE_CASE_ALIASES"
    )
    return sorted(rows, key=lambda row: canonical_bytes(row))


def derive_blocker_evidence(document: dict[str, Any]) -> list[dict[str, Any]]:
    blockers = []
    four = document.get("fourClass", {})
    valtan = document.get("valtan", {})
    for row in four.get("pinnedArtifacts", []):
        if row.get("pinStatus") != "MATCH":
            blockers.append(
                {
                    "code": "SOURCE_ARTIFACT_PIN_" + str(row.get("pinStatus", "UNKNOWN")),
                    "characterClass": row.get("characterClass"),
                    "skillId": row.get("skillId"),
                    "artifactKind": row.get("artifactKind"),
                    "locator": row.get("locator"),
                    "expectedSha256": row.get("expectedSha256"),
                    "actualSha256": row.get("actualSha256"),
                }
            )
        if row.get("pathKind") == "EXTERNAL_ABSOLUTE_HINT":
            blockers.append(
                {
                    "code": "SOURCE_STAGE_ABSOLUTE_PATH_HINT_NOT_PORTABLE",
                    "characterClass": row.get("characterClass"),
                    "skillId": row.get("skillId"),
                    "artifactKind": row.get("artifactKind"),
                    "portableLocator": row.get("locator"),
                    "pinStatus": row.get("pinStatus"),
                }
            )
    for corpus_name, corpus in (("FourClass", four), ("Valtan", valtan)):
        for row in corpus.get("packageInventory", {}).get("packages", []):
            if row.get("resolutionStatus") != "PRESENT_EXACT_CASE":
                blockers.append(
                    {
                        "code": str(row.get("resolutionStatus", "UNKNOWN_PACKAGE_STATUS")),
                        "corpus": corpus_name,
                        "logicalPackage": row.get("logicalPackage"),
                        "physicalPackageFileName": row.get("physicalPackageFileName"),
                    }
                )
    candidate_owners: dict[str, str] = {}
    for corpus in (four, valtan):
        for row in corpus.get("assetRequests", []):
            candidate = str(row.get("resourcesRelativeCandidateId", ""))
            folded = candidate.casefold()
            previous = candidate_owners.get(folded)
            if previous is not None and previous != candidate:
                blockers.append(
                    {
                        "code": "CASE_ONLY_CANDIDATE_ID_COLLISION",
                        "first": previous,
                        "second": candidate,
                    }
                )
            candidate_owners[folded] = candidate
            for consumer in row.get("consumers", []):
                statuses = set(consumer.get("ownershipStatuses", []))
                if (
                    consumer.get("consumer") == "WARLORD"
                    and "ACTION_BOUND_CATALOG_OWNERSHIP" in statuses
                    and not any(value.startswith("SOURCE_GRAPH_PIN_") for value in statuses)
                ):
                    blockers.append(
                        {
                            "code": "WARLORD_CATALOG_DEPENDENCY_ABSENT_FROM_NORMALIZED_GRAPH_BINDINGS",
                            "role": row.get("role"),
                            "sourceAssetPath": row.get("sourceAssetPath"),
                            "logicalPackage": row.get("logicalPackage"),
                            "physicalPackageFileName": row.get("physicalPackageFileName"),
                        }
                    )
    source_archive = valtan.get("sourceArchive", {})
    if (
        source_archive.get("fileName") != VALTAN_ARCHIVE_PIN["fileName"]
        or source_archive.get("byteSize") != VALTAN_ARCHIVE_PIN["byteSize"]
        or source_archive.get("sha256") != VALTAN_ARCHIVE_PIN["sha256"]
    ):
        blockers.append(
            {
                "code": "VALTAN_SOURCE_ARCHIVE_PIN_MISMATCH",
                "expected": VALTAN_ARCHIVE_PIN,
                "actual": {
                    "fileName": source_archive.get("fileName"),
                    "byteSize": source_archive.get("byteSize"),
                    "sha256": source_archive.get("sha256"),
                },
            }
        )
    extraction_input = next(
        (
            row
            for row in document.get("inputs", [])
            if row.get("locator") == "external:Valtan/data3-extraction-manifest.json"
        ),
        None,
    )
    if extraction_input is None or extraction_input.get("sha256") != (
        VALTAN_EXTRACTION_MANIFEST_SHA
    ):
        blockers.append(
            {
                "code": "VALTAN_EXTRACTION_MANIFEST_PIN_MISMATCH",
                "expectedSha256": VALTAN_EXTRACTION_MANIFEST_SHA,
                "actualSha256": extraction_input.get("sha256") if extraction_input else None,
            }
        )
    for row in valtan.get("sourceDocuments", []):
        if row.get("pinStatus") != "MATCH":
            blockers.append(
                {
                    "code": "VALTAN_ACTION_DOCUMENT_PIN_MISMATCH",
                    "profileId": row.get("profileId"),
                    "expectedSha256": row.get("expectedSha256"),
                    "actualSha256": row.get("sha256"),
                }
            )
    for row in valtan.get("rawLoas", []):
        if row.get("pinStatus") != "MATCH":
            expected = VALTAN_LOA.get(str(row.get("profileId")))
            blockers.append(
                {
                    "code": "VALTAN_RAW_LOA_PIN_MISMATCH",
                    "profileId": row.get("profileId"),
                    "fileName": row.get("fileName"),
                    "expectedByteSize": expected[1] if expected else None,
                    "expectedSha256": expected[2] if expected else None,
                    "actualByteSize": row.get("byteSize"),
                    "actualSha256": row.get("sha256"),
                }
            )
    current_archive = valtan.get("currentInstalledArchiveSnapshot", {})
    if current_archive.get("matchesPinnedCorpusArchive") is not True:
        blockers.append(
            {
                "code": "CURRENT_INSTALLED_DATA3_DRIFT_FROM_PINNED_VALTAN_CORPUS",
                "expectedByteSize": VALTAN_ARCHIVE_PIN["byteSize"],
                "expectedSha256": VALTAN_ARCHIVE_PIN["sha256"],
                "actualByteSize": current_archive.get("byteSize"),
                "actualSha256": current_archive.get("sha256"),
            }
        )
    return sorted(
        blockers,
        key=lambda row: canonical_bytes(row),
    )


def blocker_evidence_sha256(rows: list[dict[str, Any]]) -> str:
    return hashlib.sha256(canonical_bytes(rows)).hexdigest()


def build_checkpoint(
    repo_root: Path,
    package_root: Path,
    valtan_action_root: Path,
    valtan_extraction_manifest: Path,
    installed_data3: Path,
) -> dict[str, Any]:
    inputs: dict[str, dict[str, Any]] = {}
    tool_relative = Path(
        "Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py"
    )
    register_input(
        inputs,
        repo_root / tool_relative,
        tool_relative.as_posix(),
        "RAW_INVENTORY_GENERATOR",
    )
    stage_corpus, pin_failures, absolute_hints = collect_four_class_stage_corpus(
        repo_root, inputs
    )
    product_cues = collect_product_cues(repo_root, inputs)
    skill_ids_by_class = {
        class_name: sorted(
            row["skillId"]
            for row in stage_corpus["skills"]
            if row["characterClass"] == CLASS_ENUMS[class_name]
        )
        for class_name in CLASS_NAMES
    }

    four_class_requests: dict[tuple[str, str], dict[str, Any]] = {}
    four_class_package_maps = []
    four_class_graph_systems = []
    class_asset_summaries = []
    preexisting_missing = []
    warlord_catalog_disagreements = []
    occurrence_system_keys = {
        (row["characterClass"], row["sourceSystemId"].casefold())
        for row in stage_corpus["occurrenceSourceSystems"]
    }
    for class_name in CLASS_NAMES:
        class_requests, package_map, class_graph_systems = collect_graph_class_requests(
            repo_root, class_name, skill_ids_by_class[class_name], inputs
        )
        if len(class_graph_systems) != CLASS_SOURCE_SYSTEM_EXPECTED[class_name]:
            raise InventoryError(
                f"source system denominator mismatch for {class_name}: "
                f"{len(class_graph_systems)} != "
                f"{CLASS_SOURCE_SYSTEM_EXPECTED[class_name]}"
            )
        for system in class_graph_systems:
            system["activityStatus"] = (
                "OCCURRENCE_BACKED"
                if (system["characterClass"], system["sourceSystemId"].casefold())
                in occurrence_system_keys
                else "GRAPH_ONLY_INACTIVE_FOR_SELECTED_OCCURRENCES"
            )
        if class_name == "Warlord":
            graph_request_keys = set(class_requests)
            catalog_requests, catalog_package_map = collect_warlord_catalog_requests(
                repo_root,
                inputs,
                set(skill_ids_by_class[class_name]),
                {row["sourceSystemId"] for row in class_graph_systems},
            )
            catalog_request_keys = set(catalog_requests)
            for key in sorted(catalog_request_keys - graph_request_keys):
                row = catalog_requests[key]
                warlord_catalog_disagreements.append(
                    {
                        "code": "WARLORD_CATALOG_DEPENDENCY_ABSENT_FROM_NORMALIZED_GRAPH_BINDINGS",
                        "role": row["role"],
                        "sourceAssetPath": row["sourceAssetPath"],
                        "logicalPackage": row["logicalPackage"],
                        "physicalPackageFileName": row["physicalPackageFileName"],
                    }
                )
            for key in sorted(graph_request_keys - catalog_request_keys):
                row = class_requests[key]
                warlord_catalog_disagreements.append(
                    {
                        "code": "WARLORD_GRAPH_DEPENDENCY_ABSENT_FROM_ACTION_BOUND_CATALOG",
                        "role": row["role"],
                        "sourceAssetPath": row["sourceAssetPath"],
                        "logicalPackage": row["logicalPackage"],
                        "physicalPackageFileName": row["physicalPackageFileName"],
                    }
                )
            for row in catalog_requests.values():
                consumer = row["classConsumers"]["WARLORD"]
                merge_class_request(
                    class_requests,
                    "WARLORD",
                    row["role"],
                    row["sourceAssetPath"],
                    row["logicalPackage"],
                    row["physicalPackageFileName"],
                    consumer["skillIds"],
                    consumer["actionIds"],
                    consumer["sourceSystemIds"],
                    row["sourceResolutionStatuses"],
                )
            for key, mapping in catalog_package_map.items():
                previous = package_map.get(key)
                if previous is not None and previous[1].casefold() != mapping[1].casefold():
                    raise InventoryError(
                        f"Warlord graph/catalog package conflict for {mapping[0]}"
                    )
                package_map[key] = mapping
        four_class_graph_systems.extend(class_graph_systems)
        four_class_package_maps.append(package_map)
        for key, row in class_requests.items():
            four_class_requests.setdefault(key, row)
            if four_class_requests[key] is not row:
                for consumer_name, consumer in row["classConsumers"].items():
                    merge_class_request(
                        four_class_requests,
                        consumer_name,
                        row["role"],
                        row["sourceAssetPath"],
                        row["logicalPackage"],
                        row["physicalPackageFileName"],
                        consumer["skillIds"],
                        consumer["actionIds"],
                        consumer["sourceSystemIds"],
                        row["sourceResolutionStatuses"],
                    )
        counts = Counter(row["role"] for row in class_requests.values())
        expected = CLASS_EXPECTED[class_name]
        if (len(class_requests), counts["mesh"], counts["texture"]) != expected[5:8]:
            raise InventoryError(f"raw request denominator mismatch for {class_name}")
        class_asset_summaries.append(
            {
                "characterClass": CLASS_ENUMS[class_name],
                "assetRequestCount": len(class_requests),
                "meshRequestCount": counts["mesh"],
                "textureRequestCount": counts["texture"],
            }
        )
        for row in class_requests.values():
            if "MISSING_RUNTIME_ASSET" in row["sourceResolutionStatuses"]:
                preexisting_missing.append(
                    {
                        "code": "PREEXISTING_RUNTIME_ASSET_MISSING_RAW_EXTRACTION_STILL_REQUESTED",
                        "characterClass": CLASS_ENUMS[class_name],
                        "role": row["role"],
                        "sourceAssetPath": row["sourceAssetPath"],
                    }
                )

    four_class_serialized, four_class_collisions = serialize_requests(
        "FourClass", four_class_requests
    )
    if len(four_class_serialized) != FOUR_CLASS_EXPECTED["uniqueAssetRequestCount"]:
        raise InventoryError(
            f"four-class unique raw request mismatch: {len(four_class_serialized)}"
        )
    if sum(row["assetRequestCount"] for row in class_asset_summaries) != FOUR_CLASS_EXPECTED[
        "classAssetRequestCount"
    ]:
        raise InventoryError("four-class class raw request denominator mismatch")

    valtan_relative = Path(
        "Data/Effects/Imported/Valtan/Valtan.action-particle-resource-catalog.json"
    )
    valtan_requests, valtan_package_map, valtan_systems = collect_catalog_requests(
        repo_root, valtan_relative, "Valtan", inputs, None
    )
    valtan_serialized, valtan_collisions = serialize_requests("Valtan", valtan_requests)
    valtan_role_counts = Counter(row["role"] for row in valtan_serialized)
    if (
        len(valtan_serialized) != VALTAN_EXPECTED["assetRequestCount"]
        or valtan_role_counts["mesh"] != VALTAN_EXPECTED["meshRequestCount"]
        or valtan_role_counts["texture"] != VALTAN_EXPECTED["textureRequestCount"]
    ):
        raise InventoryError("Valtan Action-only raw request denominator mismatch")
    if len(valtan_systems) != VALTAN_EXPECTED["sourceSystemCount"]:
        raise InventoryError("Valtan source system denominator mismatch")

    four_packages, four_package_reports = package_identities(
        package_root, four_class_package_maps, "FourClass"
    )
    valtan_packages, valtan_package_reports = package_identities(
        package_root, [valtan_package_map], "Valtan"
    )
    four_package_inventory = serialize_package_maps(
        four_packages, FOUR_CLASS_EXPECTED["physicalPackageCount"], "FourClass"
    )
    valtan_package_inventory = serialize_package_maps(
        valtan_packages, VALTAN_EXPECTED["physicalPackageCount"], "Valtan"
    )

    valtan_corpus, valtan_provenance_reports = collect_valtan_occurrences(
        valtan_action_root,
        valtan_extraction_manifest,
        installed_data3,
        inputs,
    )
    four_summary = {
        **FOUR_CLASS_EXPECTED,
        "occurrenceSourceSystemCount": len(stage_corpus["occurrenceSourceSystems"]),
        "graphOnlyInactiveSourceSystemCount": sum(
            row["activityStatus"] == "GRAPH_ONLY_INACTIVE_FOR_SELECTED_OCCURRENCES"
            for row in four_class_graph_systems
        ),
        "pinFailureCount": len(pin_failures),
        "absoluteSourceHintCount": len(absolute_hints),
        "preexistingRuntimeMissingRequestCount": len(preexisting_missing),
        "warlordCatalogGraphDisagreementCount": len(warlord_catalog_disagreements),
    }
    valtan_summary = {
        **VALTAN_EXPECTED,
        "meshRequestCount": valtan_role_counts["mesh"],
        "textureRequestCount": valtan_role_counts["texture"],
    }

    reports = {
        "pinFailures": sorted(
            pin_failures,
            key=lambda row: (row["characterClass"], row["skillId"], row["artifactKind"]),
        ),
        "weakAbsoluteSourceHints": sorted(
            absolute_hints,
            key=lambda row: (row["characterClass"], row["skillId"], row["artifactKind"]),
        ),
        "preexistingRuntimeMissingRequests": sorted(
            preexisting_missing,
            key=lambda row: (row["characterClass"], row["role"], row["sourceAssetPath"]),
        ),
        "physicalPackageIssues": four_package_reports + valtan_package_reports,
        "candidateIdCollisions": four_class_collisions + valtan_collisions,
        "warlordCatalogGraphDisagreements": warlord_catalog_disagreements,
        "valtanProvenanceIssues": valtan_provenance_reports,
        "rawExtractionMissing": [],
        "rawOutputAmbiguities": [],
        "sharedByteGroups": [],
    }
    blocker_count = compute_report_blocker_count(reports)

    document = {
        "schema": "lostark.raw-resource-inventory-v1",
        "formatVersion": 1,
        "phase": "DENOMINATOR_CHECKPOINT",
        "mode": "OFFLINE_INVENTORY_ONLY",
        "sourceCommit": SOURCE_COMMIT,
        "runtimeAuthority": False,
        "rendererReady": False,
        "admission": {
            "Execute": False,
            "Submit": False,
            "Render": False,
            "Product": False,
        },
        "gpuSchemaBoundary": copy.deepcopy(GPU_SCHEMA_BOUNDARY),
        "fourClass": {
            "summary": four_summary,
            "classAssetSummaries": sorted(
                class_asset_summaries, key=lambda row: row["characterClass"]
            ),
            **stage_corpus,
            "sourceSystems": sorted(
                four_class_graph_systems,
                key=lambda row: (row["characterClass"], row["sourceSystemId"].casefold()),
            ),
            "productCues": product_cues,
            "assetRequests": four_class_serialized,
            "packageInventory": four_package_inventory,
        },
        "valtan": {
            "summary": valtan_summary,
            **valtan_corpus,
            "sourceSystems": valtan_systems,
            "assetRequests": valtan_serialized,
            "packageInventory": valtan_package_inventory,
        },
        "inputs": sorted(inputs.values(), key=lambda row: row["locator"]),
        "reports": reports,
        "checkpointStatus": "FROZEN_WITH_BLOCKERS" if blocker_count else "FROZEN_COMPLETE",
        "blockerCount": blocker_count,
    }
    evidence = derive_blocker_evidence(document)
    reported_evidence = report_blocker_evidence(reports)
    if canonical_bytes(evidence) != canonical_bytes(reported_evidence):
        raise InventoryError("generated blocker reports do not match source evidence")
    if len(evidence) != blocker_count:
        raise InventoryError(
            "generated blocker count does not match source evidence: "
            f"reports={blocker_count} evidence={len(evidence)}"
        )
    document["blockerEvidence"] = evidence
    document["blockerEvidenceSha256"] = blocker_evidence_sha256(evidence)
    return add_self_digest(document)


def walk_strings(value: Any) -> Iterable[str]:
    if isinstance(value, str):
        yield value
    elif isinstance(value, list):
        for row in value:
            yield from walk_strings(row)
    elif isinstance(value, dict):
        for key, row in value.items():
            yield key
            yield from walk_strings(row)


def validate_portable_strings(document: dict[str, Any]) -> None:
    violations = []
    for value in walk_strings(document):
        if "\\" in value or DRIVE_PATH.match(value) or value.startswith("/"):
            violations.append(value)
        if value.startswith("../") or "/../" in value or value.endswith("/.."):
            violations.append(value)
    if violations:
        raise InventoryError(
            "manifest contains non-portable path tokens: " + repr(violations[:10])
        )


def validate_checkpoint(document: dict[str, Any]) -> dict[str, Any]:
    if document.get("schema") != "lostark.raw-resource-inventory-v1":
        raise InventoryError("unexpected raw inventory schema")
    if document.get("formatVersion") != 1 or isinstance(document.get("formatVersion"), bool):
        raise InventoryError("formatVersion must be exact integer 1")
    if document.get("phase") != "DENOMINATOR_CHECKPOINT":
        raise InventoryError("expected DENOMINATOR_CHECKPOINT phase")
    if document.get("mode") != "OFFLINE_INVENTORY_ONLY":
        raise InventoryError("raw inventory must be OFFLINE_INVENTORY_ONLY")
    if document.get("sourceCommit") != SOURCE_COMMIT:
        raise InventoryError("raw inventory source commit mismatch")
    if document.get("runtimeAuthority") is not False or document.get("rendererReady") is not False:
        raise InventoryError("runtime authority and rendererReady must be false")
    admission = document.get("admission", {})
    if admission != {"Execute": False, "Submit": False, "Render": False, "Product": False}:
        raise InventoryError("all runtime admission flags must be false")
    if document.get("gpuSchemaBoundary") != GPU_SCHEMA_BOUNDARY:
        raise InventoryError("GPU schema boundary must remain wholly unfinalized")
    if document.get("selfDigest") != compute_self_digest(document):
        raise InventoryError("raw inventory self digest mismatch")
    validate_portable_strings(document)

    four = document.get("fourClass", {})
    valtan = document.get("valtan", {})
    for key, expected in FOUR_CLASS_EXPECTED.items():
        if four.get("summary", {}).get(key) != expected:
            raise InventoryError(f"four-class summary mismatch for {key}")
    for key, expected in VALTAN_EXPECTED.items():
        if valtan.get("summary", {}).get(key) != expected:
            raise InventoryError(f"Valtan summary mismatch for {key}")
    reports = document.get("reports", {})
    derived_blockers = derive_blocker_evidence(document)
    reported_blockers = report_blocker_evidence(reports)
    if canonical_bytes(derived_blockers) != canonical_bytes(reported_blockers):
        raise InventoryError("checkpoint blocker reports do not match source evidence")
    if canonical_bytes(document.get("blockerEvidence", [])) != canonical_bytes(
        derived_blockers
    ):
        raise InventoryError("checkpoint frozen blocker evidence projection mismatch")
    derived_blocker_sha = blocker_evidence_sha256(derived_blockers)
    if document.get("blockerEvidenceSha256") != derived_blocker_sha:
        raise InventoryError("checkpoint blocker evidence SHA mismatch")
    if (
        EXPECTED_CHECKPOINT_BLOCKER_PROJECTION_SHA256 is not None
        and derived_blocker_sha != EXPECTED_CHECKPOINT_BLOCKER_PROJECTION_SHA256
    ):
        raise InventoryError("checkpoint blocker evidence differs from frozen baseline")
    expected_blocker_count = len(derived_blockers)
    if document.get("blockerCount") != expected_blocker_count:
        raise InventoryError("checkpoint blockerCount does not match blocker reports")
    expected_checkpoint_status = (
        "FROZEN_WITH_BLOCKERS" if expected_blocker_count else "FROZEN_COMPLETE"
    )
    if document.get("checkpointStatus") != expected_checkpoint_status:
        raise InventoryError("checkpoint status does not match blocker reports")
    report_summary_fields = {
        "pinFailureCount": len(reports.get("pinFailures", [])),
        "absoluteSourceHintCount": len(reports.get("weakAbsoluteSourceHints", [])),
        "preexistingRuntimeMissingRequestCount": len(
            reports.get("preexistingRuntimeMissingRequests", [])
        ),
        "warlordCatalogGraphDisagreementCount": len(
            reports.get("warlordCatalogGraphDisagreements", [])
        ),
    }
    for key, expected in report_summary_fields.items():
        if four.get("summary", {}).get(key) != expected:
            raise InventoryError(f"four-class report summary mismatch for {key}")
    if four.get("summary", {}).get("occurrenceSourceSystemCount") != (
        FOUR_CLASS_OCCURRENCE_SOURCE_SYSTEM_COUNT
    ):
        raise InventoryError("four-class occurrence-backed source system mismatch")
    if four.get("summary", {}).get("graphOnlyInactiveSourceSystemCount") != (
        FOUR_CLASS_GRAPH_ONLY_SOURCE_SYSTEM_COUNT
    ):
        raise InventoryError("four-class graph-only source system mismatch")
    if len(four.get("skills", [])) != FOUR_CLASS_EXPECTED["skillCount"]:
        raise InventoryError("four-class skill inventory length mismatch")
    if len(four.get("stages", [])) != FOUR_CLASS_EXPECTED["stageCount"]:
        raise InventoryError("four-class stage inventory length mismatch")
    if len(four.get("clips", [])) != FOUR_CLASS_EXPECTED["clipOccurrenceCount"]:
        raise InventoryError("four-class clip inventory length mismatch")
    if len(four.get("occurrences", [])) != FOUR_CLASS_EXPECTED["sourceOccurrenceCount"]:
        raise InventoryError("four-class occurrence inventory length mismatch")
    if len(four.get("productCues", [])) != FOUR_CLASS_EXPECTED["productCueCount"]:
        raise InventoryError("four-class product cue inventory length mismatch")
    if len(four.get("sourceSystems", [])) != FOUR_CLASS_EXPECTED["sourceSystemCount"]:
        raise InventoryError("four-class source system inventory length mismatch")
    if len(four.get("occurrenceSourceSystems", [])) != (
        FOUR_CLASS_OCCURRENCE_SOURCE_SYSTEM_COUNT
    ):
        raise InventoryError("four-class occurrence source system inventory mismatch")
    graph_only_count = sum(
        row.get("activityStatus") == "GRAPH_ONLY_INACTIVE_FOR_SELECTED_OCCURRENCES"
        for row in four.get("sourceSystems", [])
    )
    if graph_only_count != FOUR_CLASS_GRAPH_ONLY_SOURCE_SYSTEM_COUNT:
        raise InventoryError("four-class graph source activity classification mismatch")
    if len(four.get("assetRequests", [])) != FOUR_CLASS_EXPECTED["uniqueAssetRequestCount"]:
        raise InventoryError("four-class raw request inventory length mismatch")
    if len(valtan.get("actions", [])) != VALTAN_EXPECTED["actionCount"]:
        raise InventoryError("Valtan action inventory length mismatch")
    if len(valtan.get("stages", [])) != VALTAN_EXPECTED["stageCount"]:
        raise InventoryError("Valtan stage inventory length mismatch")
    if len(valtan.get("clips", [])) != VALTAN_EXPECTED["clipOccurrenceCount"]:
        raise InventoryError("Valtan clip inventory length mismatch")
    if len(valtan.get("occurrences", [])) != VALTAN_EXPECTED["notifyOccurrenceCount"]:
        raise InventoryError("Valtan occurrence inventory length mismatch")
    if len(valtan.get("assetRequests", [])) != VALTAN_EXPECTED["assetRequestCount"]:
        raise InventoryError("Valtan raw request inventory length mismatch")
    if valtan.get("typedGroupCounts") != EXPECTED_TYPED_GROUPS:
        raise InventoryError("Valtan typed group counts mismatch")
    request_ids = [
        row["requestId"]
        for corpus in (four, valtan)
        for row in corpus.get("assetRequests", [])
    ]
    if len(request_ids) != len(set(request_ids)):
        raise InventoryError("duplicate raw request IDs")
    candidates = [
        row["resourcesRelativeCandidateId"]
        for corpus in (four, valtan)
        for row in corpus.get("assetRequests", [])
    ]
    if len(candidates) != len(set(candidates)):
        raise InventoryError("duplicate exact-case candidate IDs")
    return {
        "phase": document["phase"],
        "checkpointStatus": document["checkpointStatus"],
        "blockerCount": document["blockerCount"],
        "fourClassOccurrences": len(four["occurrences"]),
        "valtanOccurrences": len(valtan["occurrences"]),
        "fourClassAssetRequests": len(four["assetRequests"]),
        "valtanAssetRequests": len(valtan["assetRequests"]),
        "selfDigest": document["selfDigest"],
    }


def write_json(path: Path, document: dict[str, Any], replace: bool) -> None:
    if path.exists() and not replace:
        raise InventoryError(f"output already exists; pass --replace: {path}")
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    if temporary.exists():
        raise InventoryError(f"stale output temporary exists: {temporary}")
    payload = (
        json.dumps(document, ensure_ascii=False, allow_nan=False, indent=2) + "\n"
    ).encode("utf-8")
    temporary.write_bytes(payload)
    temporary.replace(path)


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)
    checkpoint = subparsers.add_parser("checkpoint")
    checkpoint.add_argument("--repo-root", required=True, type=Path)
    checkpoint.add_argument("--package-root", required=True, type=Path)
    checkpoint.add_argument("--valtan-action-root", required=True, type=Path)
    checkpoint.add_argument("--valtan-extraction-manifest", required=True, type=Path)
    checkpoint.add_argument("--installed-data3", required=True, type=Path)
    checkpoint.add_argument("--output", required=True, type=Path)
    checkpoint.add_argument("--replace", action="store_true")
    validate = subparsers.add_parser("validate")
    validate.add_argument("--manifest", required=True, type=Path)
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        if args.command == "checkpoint":
            document = build_checkpoint(
                args.repo_root.resolve(),
                args.package_root.resolve(),
                args.valtan_action_root.resolve(),
                args.valtan_extraction_manifest.resolve(),
                args.installed_data3.resolve(),
            )
            summary = validate_checkpoint(document)
            write_json(args.output.resolve(), document, args.replace)
            summary["rawFileSha256"] = sha256_file(args.output.resolve())
        else:
            document = load_json(args.manifest.resolve())
            summary = validate_checkpoint(document)
            summary["rawFileSha256"] = sha256_file(args.manifest.resolve())
        print(json.dumps(summary, ensure_ascii=False, sort_keys=True))
        return 0
    except InventoryError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
```

### 5-2. C:/Users/user/.codex/worktrees/0109/LostArk/Tools/LevelPlacementExtractor/test_prepare_r8_raw_resource_inventory.py

변경 종류: 추가
적용 위치: 새 Python 파일 전체

```python
#!/usr/bin/env python3
"""Focused tests for the portable R8 offline raw-resource checkpoint."""

from __future__ import annotations

import copy
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("prepare_r8_raw_resource_inventory.py")
SPEC = importlib.util.spec_from_file_location("prepare_r8_raw_resource_inventory", MODULE_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"cannot import {MODULE_PATH}")
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)
REPO_ROOT = Path(__file__).resolve().parents[2]
CHECKPOINT = (
    REPO_ROOT
    / "Data/Effects/Imported/RawResourceInventory/"
    "R8.raw-resource-denominator.checkpoint.json"
)


class RawResourceCheckpointTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = MODULE.load_json(CHECKPOINT)

    def test_checked_checkpoint_has_exact_denominators_and_blockers(self) -> None:
        summary = MODULE.validate_checkpoint(self.document)
        self.assertEqual(summary["fourClassOccurrences"], 5232)
        self.assertEqual(summary["valtanOccurrences"], 21931)
        self.assertEqual(summary["fourClassAssetRequests"], 835)
        self.assertEqual(summary["valtanAssetRequests"], 377)
        self.assertEqual(self.document["fourClass"]["summary"]["sourceSystemCount"], 442)
        source_system_counts = {}
        for row in self.document["fourClass"]["sourceSystems"]:
            source_system_counts[row["characterClass"]] = (
                source_system_counts.get(row["characterClass"], 0) + 1
            )
        self.assertEqual(
            source_system_counts,
            {
                "ARTIST": 70,
                "DIMENSIONMASTER": 95,
                "LANCE_MASTER": 179,
                "WARLORD": 98,
            },
        )
        self.assertEqual(summary["checkpointStatus"], "FROZEN_WITH_BLOCKERS")
        self.assertEqual(self.document["blockerCount"], 18)
        self.assertEqual(len(self.document["reports"]["pinFailures"]), 2)
        self.assertEqual(
            {
                (row["characterClass"], row["skillId"], row["artifactKind"])
                for row in self.document["reports"]["pinFailures"]
            },
            {
                ("ARTIST", 31930, "importedDocument"),
                ("WARLORD", 17110, "normalizedGraph"),
            },
        )
        self.assertEqual(
            len(self.document["reports"]["preexistingRuntimeMissingRequests"]),
            191,
        )
        self.assertEqual(
            self.document["fourClass"]["summary"]["occurrenceSourceSystemCount"],
            424,
        )
        self.assertEqual(
            self.document["fourClass"]["summary"]["graphOnlyInactiveSourceSystemCount"],
            18,
        )
        self.assertEqual(
            self.document["fourClass"]["classAssetSummaries"][-1],
            {
                "characterClass": "WARLORD",
                "assetRequestCount": 328,
                "meshRequestCount": 47,
                "textureRequestCount": 281,
            },
        )
        self.assertEqual(
            self.document["reports"]["warlordCatalogGraphDisagreements"],
            [
                {
                    "code": "WARLORD_CATALOG_DEPENDENCY_ABSENT_FROM_NORMALIZED_GRAPH_BINDINGS",
                    "role": "texture",
                    "sourceAssetPath": "fx_tex_02.fx_d_environ_018",
                    "logicalPackage": "fx_tex_02",
                    "physicalPackageFileName": "YGI3SORGM3I1FGHA5BMJ8Y5CZ.upk",
                }
            ],
        )
        request = next(
            row
            for row in self.document["fourClass"]["assetRequests"]
            if row["sourceAssetPath"] == "fx_tex_02.fx_d_environ_018"
        )
        warlord = next(
            row for row in request["consumers"] if row["consumer"] == "WARLORD"
        )
        self.assertEqual(warlord["skillIds"], [17820])
        self.assertEqual(warlord["actionIds"], [17820])
        self.assertEqual(
            warlord["sourceSystemIds"], ["FX_PC_WGL_01.par_o_wgl_protect_02"]
        )
        self.assertEqual(
            warlord["ownershipStatuses"],
            ["ACTION_BOUND_CATALOG_OWNERSHIP", "RESOLVED_SOURCE_PACKAGE"],
        )

    def test_runtime_admission_is_strictly_false(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["admission"]["Render"] = True
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "admission"):
            MODULE.validate_checkpoint(mutated)

    def test_gpu_schema_policy_mutation_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["gpuSchemaBoundary"]["sampler"] = "FINAL_SAMPLER"
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "GPU schema boundary"):
            MODULE.validate_checkpoint(mutated)

    def test_blocker_report_and_status_mutation_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["reports"]["pinFailures"] = []
        mutated["blockerCount"] = 0
        mutated["checkpointStatus"] = "FROZEN_COMPLETE"
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "blocker|reports"):
            MODULE.validate_checkpoint(mutated)

    def test_coordinated_false_promotion_is_rejected_by_frozen_evidence(self) -> None:
        mutated = copy.deepcopy(self.document)
        for row in mutated["fourClass"]["pinnedArtifacts"]:
            row["pinStatus"] = "MATCH"
            row["pathKind"] = "REPO_RELATIVE"
        for request in mutated["fourClass"]["assetRequests"]:
            for consumer in request["consumers"]:
                if consumer["consumer"] == "WARLORD":
                    consumer["ownershipStatuses"].append("SOURCE_GRAPH_PIN_MATCH")
                    consumer["ownershipStatuses"] = sorted(
                        set(consumer["ownershipStatuses"])
                    )
        current_archive = mutated["valtan"]["currentInstalledArchiveSnapshot"]
        current_archive["matchesPinnedCorpusArchive"] = True
        current_archive["byteSize"] = MODULE.VALTAN_ARCHIVE_PIN["byteSize"]
        current_archive["sha256"] = MODULE.VALTAN_ARCHIVE_PIN["sha256"]
        for key in (
            "pinFailures",
            "weakAbsoluteSourceHints",
            "candidateIdCollisions",
            "physicalPackageIssues",
            "warlordCatalogGraphDisagreements",
            "valtanProvenanceIssues",
        ):
            mutated["reports"][key] = []
        summary = mutated["fourClass"]["summary"]
        summary["pinFailureCount"] = 0
        summary["absoluteSourceHintCount"] = 0
        summary["warlordCatalogGraphDisagreementCount"] = 0
        mutated["blockerEvidence"] = []
        mutated["blockerEvidenceSha256"] = MODULE.blocker_evidence_sha256([])
        mutated["blockerCount"] = 0
        mutated["checkpointStatus"] = "FROZEN_COMPLETE"
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "frozen baseline"):
            MODULE.validate_checkpoint(mutated)

    def test_self_digest_detects_mutation(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["fourClass"]["summary"]["sourceOccurrenceCount"] -= 1
        with self.assertRaisesRegex(MODULE.InventoryError, "self digest"):
            MODULE.validate_checkpoint(mutated)

    def test_nonportable_paths_are_rejected(self) -> None:
        for value in (
            "C:/absolute/file.dds",
            "C:\\absolute\\file.dds",
            "C:drive-relative/file.dds",
            "/rooted/file.dds",
            "Effect/../outside.dds",
        ):
            with self.subTest(value=value):
                with self.assertRaises(MODULE.InventoryError):
                    MODULE.validate_portable_strings({"value": value})

    def test_strict_json_rejects_duplicate_and_nonfinite_values(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            duplicate = Path(directory) / "duplicate.json"
            duplicate.write_text('{"value":1,"value":2}\n', encoding="utf-8")
            with self.assertRaisesRegex(MODULE.InventoryError, "duplicate JSON key"):
                MODULE.load_json(duplicate)
            nonfinite = Path(directory) / "nonfinite.json"
            nonfinite.write_text('{"value":NaN}\n', encoding="utf-8")
            with self.assertRaisesRegex(MODULE.InventoryError, "non-finite"):
                MODULE.load_json(nonfinite)

    def test_candidate_id_preserves_full_object_path(self) -> None:
        first = MODULE.candidate_id(
            "FourClass", "texture", "fx_tex_02.sk_wgl_gdd_01_d"
        )
        second = MODULE.candidate_id(
            "FourClass", "texture", "sk_wgl_gdd_01.tex.sk_wgl_gdd_01_d"
        )
        self.assertNotEqual(first.casefold(), second.casefold())
        self.assertEqual(
            first,
            "Effect/RawCandidates/FourClass/Textures/"
            "fx_tex_02/sk_wgl_gdd_01_d.dds",
        )
        self.assertEqual(
            second,
            "Effect/RawCandidates/FourClass/Textures/"
            "sk_wgl_gdd_01/tex/sk_wgl_gdd_01_d.dds",
        )

    def test_obfuscated_package_mapping_matches_observed_install(self) -> None:
        self.assertEqual(
            MODULE.obfuscate_package_name("fx_sm_03") + ".upk",
            "XFH2RGA2R0LF04YE903X0SMQ.upk",
        )
        self.assertEqual(
            MODULE.obfuscate_package_name("mn_pmsec_00") + ".upk",
            "9G1MU9FPB1GZZEFTQWU463S.upk",
        )

    def test_manifest_serialization_is_deterministic(self) -> None:
        projection = copy.deepcopy(self.document)
        projection.pop("selfDigest")
        first = MODULE.canonical_bytes(projection)
        second = MODULE.canonical_bytes(json.loads(first))
        self.assertEqual(first, second)
        self.assertEqual(
            MODULE.compute_self_digest(self.document), self.document["selfDigest"]
        )


if __name__ == "__main__":
    unittest.main()
```

### 5-3. C:/Users/user/.codex/worktrees/0109/LostArk/Data/Effects/Imported/RawResourceInventory/R8.raw-resource-denominator.checkpoint.json

변경 종류: 추가(생성 데이터 전체 교체)
적용 위치: `build_checkpoint()` 반환값을 `write_json()`이 UTF-8, `indent=2`, terminal LF로 직렬화한 파일 전체

이 checkpoint는 21,107,545 byte의 생성 데이터이므로 계획서에 수동 복제하지 않는다. 위 5-1의 생략 없는
generator 정본과 아래 명령이 파일 전체를 재생성하는 교체 가능한 완전한 블록이다. fresh baseline 입력을
사용해 다음 명령을 그대로 실행한다.

```powershell
Set-Location C:\Users\user\.codex\worktrees\0109\LostArk
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py checkpoint `
  --repo-root . `
  --package-root C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages `
  --valtan-action-root C:\Users\user\Desktop\Resource_LostArk\05_Reports\EffectExtraction\VALTAN\all_actions `
  --valtan-extraction-manifest C:\Users\user\Desktop\Resource_LostArk\05_Reports\EffectExtraction\VALTAN\all_actions\action-effect-source-manifest.json `
  --installed-data3 C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\data3.lpk `
  --output Data\Effects\Imported\RawResourceInventory\R8.raw-resource-denominator.checkpoint.json `
  --replace
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py validate `
  --manifest Data\Effects\Imported\RawResourceInventory\R8.raw-resource-denominator.checkpoint.json
```

생성 schema의 root key 순서와 각 aggregate의 exact row shape는 다음과 같다. 배열의 값과 정렬,
blocker 재투영, self digest 계산은 5-1의 `build_checkpoint()`, `validate_checkpoint()`가 유일하게 소유한다.

| 경로 | exact key 또는 row field |
|---|---|
| root | `schema, formatVersion, phase, mode, sourceCommit, runtimeAuthority, rendererReady, admission, gpuSchemaBoundary, fourClass, valtan, inputs, reports, checkpointStatus, blockerCount, blockerEvidence, blockerEvidenceSha256, selfDigest` |
| `admission` | `Execute, Submit, Render, Product` |
| `gpuSchemaBoundary` | `sampler, rasterizerState, depthStencilState, srvColourspace, neutralProvider, shaderPermutation, materialPacking, recipeSlot, geometryPreScale, cache, rendererPacket, finalBindingGenerated` |
| `fourClass` | `summary, classAssetSummaries, classSummaries, skills, stages, clips, occurrenceSourceSystems, occurrences, pinnedArtifacts, sourceSystems, productCues, assetRequests, packageInventory` |
| `fourClass.summary` | `skillCount, stageCount, clipOccurrenceCount, sourceOccurrenceCount, productCueCount, classAssetRequestCount, uniqueAssetRequestCount, physicalPackageCount, sourceSystemCount, occurrenceSourceSystemCount, graphOnlyInactiveSourceSystemCount, pinFailureCount, absoluteSourceHintCount, preexistingRuntimeMissingRequestCount, warlordCatalogGraphDisagreementCount` |
| `fourClass.classAssetSummaries[]` | `characterClass, assetRequestCount, meshRequestCount, textureRequestCount` |
| `fourClass.classSummaries[]` | `characterClass, skillCount, stageCount, clipOccurrenceCount, sourceOccurrenceCount, productCueCount, stageManifest` |
| `fourClass.skills[]` | `characterClass, skillId, inputSlot, skillKind, stageCount, clipOccurrenceCount` |
| `fourClass.stages[]` | `characterClass, skillId, stageId, stageIndex, sourceEventIds, clipOccurrenceCount` |
| `fourClass.clips[]` | `characterClass, skillId, stageId, stageClipIndex, sequenceIndex, clip, sourceSkillId` |
| `fourClass.occurrenceSourceSystems[]` | `characterClass, sourceSystemId, skillIds, sourceEmitterCount, sourceLodCount, occurrenceCount` |
| `fourClass.occurrences[]` | `occurrenceId, characterClass, skillId, elementId, conversionIndex, sourceSystemId, sourceEmitter, sourceLod, sourceEventOccurrences, stageIds` |
| `fourClass.pinnedArtifacts[]` | `characterClass, skillId, artifactKind, locator, pathKind, fileName, expectedSha256, actualByteSize, actualSha256, pinStatus` |
| `fourClass.sourceSystems[]` | `characterClass, sourceSystemId, sourceAsset, logicalPackage, skillIds, activityStatus` |
| `fourClass.productCues[]` | `cueId, characterClass, skillId, stageIndex, stageClipIndex, clip, sourceEventIds, sourceElementsSha256` |
| `fourClass.assetRequests[]` | `requestId, role, sourceAssetPath, logicalPackage, physicalPackageFileName, resourcesRelativeCandidateId, bindingStatus, consumers, sourceResolutionStatuses`; `consumers[]` row는 `consumer, skillIds, actionIds, sourceSystemIds, ownershipStatuses` |
| `fourClass.packageInventory`, `valtan.packageInventory` | `packageCount, totalByteSize, packages`; `packages[]` row는 `corpus, logicalPackage, logicalPackageAliases, physicalPackageFileName, resolutionStatus, byteSize, sha256` |
| `valtan` | `summary, sourceArchive, currentInstalledArchiveSnapshot, sourceDocuments, rawLoas, actions, stages, clips, occurrences, notifyTypeCounts, typedGroupCounts, sourceSystems, assetRequests, packageInventory` |
| `valtan.summary` | `actionCount, stageCount, clipOccurrenceCount, notifyOccurrenceCount, sourceSystemCount, assetRequestCount, meshRequestCount, textureRequestCount, physicalPackageCount, typedOccurrenceCount, genericEffectQuarantineCount` |
| `valtan.sourceArchive` | `fileName, byteSize, sha256, expectedEntries, extractedEntries, failureCount` |
| `valtan.currentInstalledArchiveSnapshot` | `locator, kinds, fileName, byteSize, sha256, matchesPinnedCorpusArchive` |
| `valtan.sourceDocuments[]` | `profileId, locator, fileName, byteSize, sha256, expectedSha256, pinStatus` |
| `valtan.rawLoas[]` | `profileId, locator, fileName, byteSize, sha256, pinStatus, archiveEntry` |
| `valtan.actions[]` | `actionKey, profileId, sourceActionIndex, actionId, sourceOffset` |
| `valtan.stages[]` | `stageKey, actionKey, stageIndex, sourceOffset, clipOccurrenceCount, notifyOccurrenceCount` |
| `valtan.clips[]` | `clipOccurrenceId, stageKey, clipName, lengthSeconds, notifyId` |
| `valtan.occurrences[]` | `occurrenceId, stageKey, sourceType, category, classification, resolutionStatus, localTimeSeconds, durationSeconds, sourceOffset, assetReferences` |
| `valtan.sourceSystems[]` | `corpus, sourceSystemId, logicalPackage, actionIds, occurrenceCount` |
| `valtan.assetRequests[]` | `requestId, role, sourceAssetPath, logicalPackage, physicalPackageFileName, resourcesRelativeCandidateId, bindingStatus, consumers, sourceResolutionStatuses` |
| `inputs[]` | `locator, kinds, fileName, byteSize, sha256` |
| `reports` | `pinFailures, weakAbsoluteSourceHints, preexistingRuntimeMissingRequests, physicalPackageIssues, candidateIdCollisions, warlordCatalogGraphDisagreements, valtanProvenanceIssues, rawExtractionMissing, rawOutputAmbiguities, sharedByteGroups` |
| `blockerEvidence[]` | blocker code별 필요한 부분집합: `actualByteSize, actualSha256, artifactKind, characterClass, code, expectedByteSize, expectedSha256, locator, logicalPackage, physicalPackageFileName, pinStatus, portableLocator, role, skillId, sourceAssetPath` |

현재 생성 byte identity는 byteSize `21107545`, raw file SHA-256
`42b437d59bf56c713aa5e53dcd73d7d2ad20fb510dfca73a4c2cacccabe16db5`, canonical
`selfDigest.sha256` `8b90b1cfa917d4fc102d619468016d812fce6c73f21b0a1547eda504b31a07fb`,
`blockerEvidenceSha256` `1c8ed35f509e16281c8214d6920881714bdffcbaac451c2ddb3b05c1acdf8a4d`다.


## `lostark.raw-resource-inventory-v1` 불변식

- `mode`는 `OFFLINE_INVENTORY_ONLY`, `rendererReady`는 false다.
- `admission.Execute`, `Submit`, `Render`, `Product`는 모두 false다.
- sampler, RS/DSS, SRV colourspace, neutral provider, shader permutation, material packing, recipe slot,
  geometry preScale, cache와 renderer packet은 모두 `UNFINALIZED_GPU_SCHEMA_PENDING`이다.
- manifest의 어떤 문자열에도 absolute path, drive-qualified path, backslash 또는 Resources 밖 `..`가 없다.
- source input은 portable logical ID 또는 repo-relative path와 byteSize/SHA-256을 함께 가진다.
- physical package는 exact-case basename, byteSize/SHA-256만 가지며 absolute install root는 없다.
- candidate ID는 `Effect/RawCandidates/<Corpus>/<Meshes|Textures>/<logical/object path>.<ext>`이고 실제
  Resources file의 존재를 뜻하지 않는다.
- provenance mismatch, missing package/object/export, ambiguous output, invalid DDS/WModel은 blocker이며
  request row를 삭제하거나 PASS로 승격하지 않는다.
- checkpoint와 final inventory는 deterministic JSON 직렬화 raw SHA와 canonical self digest를 각각 가진다.

## 검증과 commit 단위

첫 commit은 denominator checkpoint, 도구의 checkpoint/validate 경로, focused unit test와 RESULT의 checkpoint
절만 포함한다. strict JSON parse, exact denominator assertion, portable-path scan, package hash 재검증,
`git diff --check`, 전체 ProjectAudit 뒤 push한다.

둘째 commit은 새 checkpoint SHA를 이름에 포함한 fresh four-class/Valtan staging에서 export한다. 모든 raw
DDS/WModel의 byteSize/SHA와 구조 검사, shared-byte dedup, missing/case/name collision report를 생성하고
final inventory validate, focused test, ProjectAudit, `git diff --check` 뒤 push한다. 최종 보고는 이 frozen
raw manifest의 file SHA를 M0/GPU 결과와 분리해 제공하며 GPU schema 동결 전 final binding은 생성하지 않는다.
