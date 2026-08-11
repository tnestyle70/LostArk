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
기대 SHA가 현재 tracked byte SHA와 다르다. 첫 변경 단위는 exact denominator와 package/object 요청을
재현하되 provenance failure 18개를 PASS로 승격하지 않는 frozen checkpoint로 완료했다. 둘째 변경
단위는 checkpoint SHA를 이름에 넣은 fresh empty staging에서 1,212 request를 모두 추출·보존했다.
1,208개는 DDS/WModel 구조 검사까지 성공했고 네 texture는 UModel 원형 TGA를 재인코딩하지 않은
`UNSUPPORTED_NON_DDS_TEXTURE_PRIMARY` blocker로 남겼다. 최종 inventory는 기존 18개와 raw blocker
4개를 합친 `FROZEN_WITH_BLOCKERS` 22개이며 GPU binding은 생성하지 않았다.

`Client/Bin/Resources`, runtime Catalog/DataFiles, shared C++, M0/GPU/Product 파일은 수정하지 않는다.
candidate ID는 case-sensitive Resources-relative 문자열일 뿐 실제 canonical Resources에 publish하지 않는다.

## 추가·수정할 파일

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/.codex/worktrees/0109/LostArk/Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py` | R8 분모와 portable provenance를 봉인하고 fresh export, extraction receipt 검증, raw DDS/WModel 재검사, dedup/collision report와 final offline inventory를 생성한다. |
| 추가 | `C:/Users/user/.codex/worktrees/0109/LostArk/Tools/LevelPlacementExtractor/test_prepare_r8_raw_resource_inventory.py` | pin/path false promotion, fresh staging, package preflight, DDS/WModel 구조, receipt/final projection 변조, collision과 deterministic serialization을 검증한다. |
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

`extract` mode는 checkpoint raw SHA를 staging identity에 포함하고 target이 존재하거나 부모가 비어 있지 않으면
write 전에 실패한다. 59개 package pin을 current bytes로 재검증하고 direct raw object package는 UModel
`-list -nameresolve`가 출력한 physical filename까지 exact-case로 대조한다. UModel은 71개 exact object batch를
`-dds -gltf -nooverwrite`로 실행한다. TGA를 DDS로 재인코딩하지 않는다. mesh는 hydrated
ModelAssetConverter와 companion DLL identity를 pin하고 `--pretransform --scale 1 --no-auto-textures`로만
canonical Resources 밖의 WModel staging에 쓴다. 1,212 request, process log, payload identity와 구조 검사,
denominator 밖 88개 fresh sidecar/dependency를 `lostark.raw-resource-extraction-run-v1` receipt에 남긴다.

`validate-extraction` mode는 receipt의 offline flags, exact field set, checkpoint SHA, tool/conversion boundary,
corpus별 request ID uniqueness와 self digest를 검증한다. 동결 receipt raw SHA는
`cf58ad0b26b7f24e0d0cd91e713f14bf96a3c9c14d4284a60415512977980137`이다.

`finalize` mode는 receipt가 가리키는 fresh stage 1,961 files / 119,564,185 bytes를 path, size, SHA로 다시
검증하고 각 request의 raw payload를 다시 연다. DDS는 magic/header/format/dimension/mip/block payload와
top-level linear size를, WModel은 WINT/WMOD/WMSH/WMA2/WMAT, section consumption, submesh range,
vertex/index stride·range, finite channel, local index와 embedded/derived bounds를 검사한다. checkpoint
projection을 reconstruction해 denominator가 바뀌지 않았음을 확인한 뒤 request 1,212개에 `rawResource`를
결합하고 shared-byte/case/candidate/basename report를 생성한다. alias payload는 DDS 1,026, glTF 182,
glTF buffer 182, WModel 182, TGA 4이고 structural success는 1,208개다.

`validate-final` mode는 receipt, raw-resource projection
`c7d9b7d7a8fc9b82627accf34b5ce6b2f21e20eb76b6458b81ba31c67a800e4b`, raw blocker projection
`c08c2f3db4204da5d453bb9050dcfa901477faf6a3bd0684028ea76c84b0694b`, summary와 22개 blocker를 다시
도출해 coordinated false promotion을 거부한다. `verify-final-stage` mode는 final manifest validation 뒤
receipt와 외부 stage 모든 identity를 다시 대조한다. 두 mode 모두 sampler와 SRV colourspace를 결정하지
않고 scale 1 변환을 final `geometryPreScale`로 봉인하지 않는다.

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
import struct
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
EXPECTED_CHECKPOINT_RAW_SHA256 = (
    "42b437d59bf56c713aa5e53dcd73d7d2ad20fb510dfca73a4c2cacccabe16db5"
)
EXPECTED_CHECKPOINT_SELF_DIGEST = (
    "8b90b1cfa917d4fc102d619468016d812fce6c73f21b0a1547eda504b31a07fb"
)
EXPECTED_CHECKPOINT_BYTE_SIZE = 21_107_545
EXPECTED_EXTRACTION_RECEIPT_SHA256 = (
    "cf58ad0b26b7f24e0d0cd91e713f14bf96a3c9c14d4284a60415512977980137"
)
EXPECTED_RAW_BLOCKER_PROJECTION_SHA256 = (
    "c08c2f3db4204da5d453bb9050dcfa901477faf6a3bd0684028ea76c84b0694b"
)
EXPECTED_RAW_RESOURCE_PROJECTION_SHA256 = (
    "c7d9b7d7a8fc9b82627accf34b5ce6b2f21e20eb76b6458b81ba31c67a800e4b"
)
EXTRACTION_RECEIPT_SCHEMA = "lostark.raw-resource-extraction-run-v1"
FINAL_PHASE = "RAW_BYTES_INSPECTED"
DEFAULT_EXPORT_CHUNK_SIZE = 32
DDS_HEADER_SIZE = 124
WMODEL_FILE_HEADER = struct.Struct("<4sHHII")
WMODEL_MODEL_HEADER = struct.Struct("<4sIII4I")
WMODEL_SECTION_DESC = struct.Struct("<IIQQ40s")
WMODEL_MESH_HEADER = struct.Struct("<4sIIIIIIIB3s")
WMODEL_SUBMESH_DESC = struct.Struct("<IIIIIQ20s")
WMODEL_BOUNDS = struct.Struct("<10f")
WMODEL_STATIC_VERTEX_STRIDE = 48
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


def _read_u32(data: bytes, offset: int, field: str) -> int:
    if offset < 0 or offset + 4 > len(data):
        raise InventoryError(f"{field} is outside the payload")
    return struct.unpack_from("<I", data, offset)[0]


def _dds_block_bytes(four_cc: str, dxgi_format: int | None) -> int | None:
    if four_cc == "DX10":
        if dxgi_format in (70, 71, 72, 79, 80, 81):
            return 8
        if dxgi_format in (
            73,
            74,
            75,
            76,
            77,
            78,
            82,
            83,
            84,
            94,
            95,
            96,
            97,
            98,
            99,
        ):
            return 16
        return None
    if four_cc in ("DXT1", "ATI1", "BC4U", "BC4S"):
        return 8
    if four_cc in ("DXT2", "DXT3", "DXT4", "DXT5", "ATI2", "BC5U", "BC5S"):
        return 16
    return None


def inspect_dds_bytes(data: bytes) -> dict[str, Any]:
    """Return parser-neutral DDS header and mechanically derived payload facts."""

    if len(data) < 128:
        raise InventoryError("DDS payload is shorter than magic plus DDS_HEADER")
    if data[:4] != b"DDS ":
        raise InventoryError("DDS magic is invalid")
    header_size = _read_u32(data, 4, "DDS header size")
    pixel_format_size = _read_u32(data, 76, "DDS pixel format size")
    if header_size != DDS_HEADER_SIZE or pixel_format_size != 32:
        raise InventoryError(
            f"DDS header sizes are invalid: {header_size}/{pixel_format_size}"
        )
    flags = _read_u32(data, 8, "DDS flags")
    height = _read_u32(data, 12, "DDS height")
    width = _read_u32(data, 16, "DDS width")
    pitch_or_linear = _read_u32(data, 20, "DDS pitch or linear size")
    depth = _read_u32(data, 24, "DDS depth")
    declared_mips = _read_u32(data, 28, "DDS mip count")
    pixel_format_flags = _read_u32(data, 80, "DDS pixel format flags")
    four_cc_bytes = data[84:88]
    four_cc = four_cc_bytes.decode("latin-1")
    rgb_bit_count = _read_u32(data, 88, "DDS RGB bit count")
    masks = [_read_u32(data, offset, "DDS channel mask") for offset in (92, 96, 100, 104)]
    caps = _read_u32(data, 108, "DDS caps")
    caps2 = _read_u32(data, 112, "DDS caps2")
    caps3 = _read_u32(data, 116, "DDS caps3")
    caps4 = _read_u32(data, 120, "DDS caps4")
    reserved2 = _read_u32(data, 124, "DDS reserved2")
    reserved1 = [
        _read_u32(data, 32 + index * 4, "DDS reserved1") for index in range(11)
    ]
    if width <= 0 or height <= 0:
        raise InventoryError("DDS dimensions must be positive")

    data_offset = 128
    dx10: dict[str, Any] | None = None
    dxgi_format: int | None = None
    if four_cc == "DX10":
        if len(data) < 148:
            raise InventoryError("DDS DX10 header is truncated")
        dxgi_format, resource_dimension, misc_flag, array_size, misc_flags2 = (
            struct.unpack_from("<5I", data, 128)
        )
        if array_size == 0:
            raise InventoryError("DDS DX10 array size must be positive")
        dx10 = {
            "dxgiFormat": dxgi_format,
            "resourceDimension": resource_dimension,
            "miscFlag": misc_flag,
            "arraySize": array_size,
            "miscFlags2": misc_flags2,
        }
        if resource_dimension not in (2, 3, 4):
            raise InventoryError("DDS DX10 resource dimension is invalid")
        if resource_dimension == 2 and height != 1:
            raise InventoryError("DDS DX10 1D resource must have height 1")
        if resource_dimension == 4 and (array_size != 1 or depth <= 0):
            raise InventoryError("DDS DX10 3D resource shape is invalid")
        if misc_flag & 0x4 and resource_dimension != 3:
            raise InventoryError("DDS DX10 cube must be a 2D resource")
        data_offset = 148

    block_bytes = _dds_block_bytes(four_cc, dxgi_format)
    if block_bytes is not None:
        if flags & 0x00081007 != 0x00081007:
            raise InventoryError("DDS supported block payload lacks required header flags")
        if not (caps & 0x1000):
            raise InventoryError("DDS supported block payload lacks DDSCAPS_TEXTURE")
        if dx10 and dx10["resourceDimension"] != 4 and depth not in (0, 1):
            raise InventoryError("DDS non-3D DX10 resource has a depth value")
    mip_count = max(1, declared_mips)
    is_legacy_cube = bool(caps2 & 0x200)
    legacy_face_bits = (0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000)
    legacy_face_count = sum(bool(caps2 & bit) for bit in legacy_face_bits)
    is_dx10_cube = bool(dx10 and dx10["miscFlag"] & 0x4)
    if is_dx10_cube:
        face_count = 6
    elif is_legacy_cube:
        face_count = legacy_face_count
    else:
        face_count = 1
    array_size = int(dx10["arraySize"]) if dx10 else 1
    if face_count <= 0:
        raise InventoryError("DDS cube declares no faces")
    is_volume = bool(caps2 & 0x200000) or bool(
        dx10 and dx10["resourceDimension"] == 4
    )
    if is_volume and depth <= 0:
        raise InventoryError("DDS volume resource must have positive depth")
    base_depth = max(1, depth) if is_volume else 1
    maximum_mips = int(math.floor(math.log2(max(width, height, base_depth)))) + 1
    if mip_count > maximum_mips:
        raise InventoryError(
            f"DDS mip count {mip_count} exceeds dimension-derived maximum {maximum_mips}"
        )
    if block_bytes is not None and not (pixel_format_flags & 0x4):
        raise InventoryError("DDS block-compressed FourCC lacks DDPF_FOURCC")
    calculated_mips: list[dict[str, Any]] = []
    expected_payload: int | None = 0 if block_bytes is not None else None
    payload_cursor = data_offset
    for level in range(mip_count):
        mip_width = max(1, width >> level)
        mip_height = max(1, height >> level)
        mip_depth = max(1, base_depth >> level) if is_volume else 1
        if block_bytes is None:
            mip_bytes = None
        else:
            surface_bytes = (
                max(1, (mip_width + 3) // 4)
                * max(1, (mip_height + 3) // 4)
                * block_bytes
            )
            mip_bytes = surface_bytes * mip_depth * face_count * array_size
            expected_payload += mip_bytes
        calculated_mips.append(
            {
                "level": level,
                "width": mip_width,
                "height": mip_height,
                "depth": mip_depth,
                "offset": (
                    payload_cursor
                    if mip_bytes is not None and face_count * array_size == 1
                    else None
                ),
                "byteSize": mip_bytes,
            }
        )
        if mip_bytes is not None:
            payload_cursor += mip_bytes

    actual_payload = len(data) - data_offset
    top_level_bytes = calculated_mips[0]["byteSize"]
    if top_level_bytes is not None:
        top_level_bytes //= face_count * array_size
    truncated = (
        max(0, int(expected_payload) - actual_payload)
        if expected_payload is not None
        else None
    )
    trailing = (
        max(0, actual_payload - int(expected_payload))
        if expected_payload is not None
        else None
    )
    linear_size_matches = (
        pitch_or_linear == top_level_bytes if top_level_bytes is not None else None
    )
    status = (
        "STRUCTURALLY_VALID"
        if expected_payload is not None
        and expected_payload == actual_payload
        and linear_size_matches is True
        else "UNSUPPORTED_FORMAT_FACTS_ONLY"
        if expected_payload is None
        else "TRUNCATED_PAYLOAD"
        if actual_payload < expected_payload
        else "TRAILING_PAYLOAD"
        if actual_payload > expected_payload
        else "TOP_LEVEL_LINEAR_SIZE_MISMATCH"
    )
    return {
        "inspectionStatus": status,
        "byteSize": len(data),
        "header": {
            "headerSize": header_size,
            "flags": flags,
            "width": width,
            "height": height,
            "depth": depth,
            "declaredMipCount": declared_mips,
            "effectiveMipCount": mip_count,
            "pitchOrLinearSize": pitch_or_linear,
            "pixelFormatSize": pixel_format_size,
            "pixelFormatFlags": pixel_format_flags,
            "fourCC": four_cc,
            "fourCCBytesHex": four_cc_bytes.hex(),
            "rgbBitCount": rgb_bit_count,
            "channelMasks": masks,
            "reserved1": reserved1,
            "caps": caps,
            "caps2": caps2,
            "caps3": caps3,
            "caps4": caps4,
            "reserved2": reserved2,
            "dx10": dx10,
        },
        "layoutFacts": {
            "dataOffset": data_offset,
            "blockByteSize": block_bytes,
            "isCube": is_legacy_cube or is_dx10_cube,
            "faceCount": face_count,
            "isVolume": is_volume,
            "arraySize": array_size,
            "calculatedMips": calculated_mips,
            "topLevelCalculatedByteSize": top_level_bytes,
            "topLevelLinearSizeMatches": linear_size_matches,
            "expectedPayloadByteSize": expected_payload,
            "actualPayloadByteSize": actual_payload,
            "truncatedByteCount": truncated,
            "trailingByteCount": trailing,
            "exactPayloadConsumption": expected_payload == actual_payload,
        },
    }


def inspect_dds_file(path: Path) -> dict[str, Any]:
    return inspect_dds_bytes(path.read_bytes())


def _fixed_name(value: bytes) -> str:
    return value.split(b"\0", 1)[0].decode("utf-8", errors="replace")


def _float_close(left: float, right: float) -> bool:
    return math.isclose(left, right, rel_tol=1e-5, abs_tol=1e-5)


def _inspect_wmodel_mesh(payload: bytes) -> dict[str, Any]:
    if len(payload) < WMODEL_FILE_HEADER.size + WMODEL_MESH_HEADER.size:
        raise InventoryError("WModel mesh section is truncated")
    outer = WMODEL_FILE_HEADER.unpack_from(payload, 0)
    if outer[:4] != (b"WINT", 1, 0, 0) or outer[4] != len(payload) - WMODEL_FILE_HEADER.size:
        raise InventoryError("WModel mesh WINT header is invalid")
    header = WMODEL_MESH_HEADER.unpack_from(payload, WMODEL_FILE_HEADER.size)
    (
        mesh_magic,
        submesh_count,
        bone_count,
        vertex_flags,
        vertex_stride,
        total_vertices,
        total_indices,
        index_stride,
        has_bounds,
        reserved,
    ) = header
    if mesh_magic != b"WMSH" or not (1 <= submesh_count <= 2048):
        raise InventoryError("WModel mesh header is invalid")
    if (
        bone_count != 0
        or vertex_flags != 0x0F
        or vertex_stride != WMODEL_STATIC_VERTEX_STRIDE
        or total_vertices <= 0
        or total_vertices > 10_000_000
        or total_indices <= 0
        or index_stride not in (2, 4)
        or has_bounds != 1
        or reserved != b"\0\0\0"
    ):
        raise InventoryError("WModel mesh static layout contract is invalid")
    descriptor_offset = WMODEL_FILE_HEADER.size + WMODEL_MESH_HEADER.size
    descriptor_end = descriptor_offset + submesh_count * WMODEL_SUBMESH_DESC.size
    if descriptor_end > len(payload):
        raise InventoryError("WModel submesh descriptor table is truncated")
    descriptors = [
        WMODEL_SUBMESH_DESC.unpack_from(
            payload, descriptor_offset + row * WMODEL_SUBMESH_DESC.size
        )
        for row in range(submesh_count)
    ]
    vertex_bytes = total_vertices * vertex_stride
    index_bytes = total_indices * index_stride
    bounds_bytes = submesh_count * WMODEL_BOUNDS.size
    expected_end = descriptor_end + vertex_bytes + index_bytes + bounds_bytes
    if expected_end > len(payload):
        raise InventoryError("WModel vertex/index/bounds payload is truncated")
    trailing_bytes = len(payload) - expected_end
    vertex_blob = payload[descriptor_end : descriptor_end + vertex_bytes]
    index_blob = payload[
        descriptor_end + vertex_bytes : descriptor_end + vertex_bytes + index_bytes
    ]
    bounds_blob = payload[
        descriptor_end + vertex_bytes + index_bytes : expected_end
    ]
    layout_supported = True
    submeshes: list[dict[str, Any]] = []
    derived_global_min = [math.inf, math.inf, math.inf]
    derived_global_max = [-math.inf, -math.inf, -math.inf]
    expected_vertex_offset = 0
    expected_index_offset = 0
    for row_index, descriptor in enumerate(descriptors):
        (
            vertex_offset,
            vertex_count,
            index_offset,
            index_count,
            material_index,
            material_hash,
            name_bytes,
        ) = descriptor
        vertex_end = vertex_offset + vertex_count * vertex_stride
        index_end = index_offset + index_count * index_stride
        if vertex_count <= 0 or index_count <= 0:
            raise InventoryError("WModel submesh has an empty vertex or index range")
        if vertex_offset != expected_vertex_offset or index_offset != expected_index_offset:
            raise InventoryError("WModel submesh ranges are not contiguous")
        if vertex_end > len(vertex_blob) or index_end > len(index_blob):
            raise InventoryError("WModel submesh range is outside aggregate payload")
        if index_count % 3:
            raise InventoryError("WModel submesh index count is not triangle-aligned")
        if material_index >= 4096:
            raise InventoryError("WModel submesh material index exceeds structural limit")
        positions: list[tuple[float, float, float]] = []
        for vertex_index in range(vertex_count):
            channels = struct.unpack_from(
                "<12f", vertex_blob, vertex_offset + vertex_index * vertex_stride
            )
            if not all(math.isfinite(value) for value in channels):
                raise InventoryError("WModel vertex channel is non-finite")
            positions.append(channels[:3])
        index_format = "<H" if index_stride == 2 else "<I"
        indices = [
            struct.unpack_from(
                index_format, index_blob, index_offset + index * index_stride
            )[0]
            for index in range(index_count)
        ]
        if any(value >= vertex_count for value in indices):
            raise InventoryError("WModel submesh index is outside its vertex range")
        expected_vertex_offset = vertex_end
        expected_index_offset = index_end
        minimum = [min(value[axis] for value in positions) for axis in range(3)]
        maximum = [max(value[axis] for value in positions) for axis in range(3)]
        for axis in range(3):
            derived_global_min[axis] = min(derived_global_min[axis], minimum[axis])
            derived_global_max[axis] = max(derived_global_max[axis], maximum[axis])
        embedded_bounds: list[float] | None = None
        bounds_consistent: bool | None = None
        if has_bounds:
            bounds = WMODEL_BOUNDS.unpack_from(
                bounds_blob, row_index * WMODEL_BOUNDS.size
            )
            if not all(math.isfinite(value) for value in bounds):
                raise InventoryError("WModel embedded bounds are non-finite")
            embedded_bounds = list(bounds)
            center = [(minimum[axis] + maximum[axis]) * 0.5 for axis in range(3)]
            radius = max(
                math.sqrt(
                    sum((position[axis] - center[axis]) ** 2 for axis in range(3))
                )
                for position in positions
            )
            bounds_consistent = (
                all(_float_close(bounds[axis], minimum[axis]) for axis in range(3))
                and all(
                    _float_close(bounds[axis + 3], maximum[axis])
                    for axis in range(3)
                )
                and all(
                    _float_close(bounds[axis + 6], center[axis])
                    for axis in range(3)
                )
                and bounds[9] >= 0.0
                and _float_close(bounds[9], radius)
            )
            if not bounds_consistent:
                raise InventoryError("WModel embedded bounds differ from derived bounds")
        submeshes.append(
            {
                "submeshIndex": row_index,
                "name": _fixed_name(name_bytes),
                "materialIndexFact": material_index,
                "materialHashFact": material_hash,
                "vertexOffset": vertex_offset,
                "vertexCount": vertex_count,
                "indexOffset": index_offset,
                "indexCount": index_count,
                "minimum": minimum,
                "maximum": maximum,
                "embeddedBounds": embedded_bounds,
                "embeddedBoundsConsistent": bounds_consistent,
            }
        )
    if expected_vertex_offset != len(vertex_blob) or expected_index_offset != len(index_blob):
        raise InventoryError("WModel aggregate vertex or index counts differ from submeshes")
    return {
        "outer": {
            "magic": outer[0].decode("ascii"),
            "majorVersion": outer[1],
            "minorVersion": outer[2],
            "flags": outer[3],
            "declaredContentByteSize": outer[4],
            "actualContentByteSize": len(payload) - WMODEL_FILE_HEADER.size,
        },
        "mesh": {
            "magic": mesh_magic.decode("ascii"),
            "submeshCount": submesh_count,
            "boneCount": bone_count,
            "vertexFlags": vertex_flags,
            "vertexStride": vertex_stride,
            "totalVertexCount": total_vertices,
            "totalIndexCount": total_indices,
            "indexStride": index_stride,
            "hasEmbeddedBounds": bool(has_bounds),
            "reservedHex": reserved.hex(),
            "layoutPositionDecodable": layout_supported,
            "maximumReferencedMaterialIndex": max(
                row["materialIndexFact"] for row in submeshes
            ),
            "derivedMinimum": derived_global_min,
            "derivedMaximum": derived_global_max,
            "submeshes": submeshes,
        },
        "trailingByteCount": trailing_bytes,
        "exactPayloadConsumption": trailing_bytes == 0,
    }


def _inspect_wmodel_material(payload: bytes, required_material_count: int) -> dict[str, Any]:
    if len(payload) < WMODEL_FILE_HEADER.size + 8:
        raise InventoryError("WModel material section is truncated")
    outer = WMODEL_FILE_HEADER.unpack_from(payload, 0)
    if outer[:4] != (b"WINT", 1, 0, 0) or outer[4] != len(payload) - WMODEL_FILE_HEADER.size:
        raise InventoryError("WModel material WINT header is invalid")
    magic, material_count = struct.unpack_from("<4sI", payload, WMODEL_FILE_HEADER.size)
    entry_sizes = {b"WMAT": 596, b"WMA2": 4756}
    if magic not in entry_sizes or material_count > 4096:
        raise InventoryError("WModel material container header is invalid")
    expected_size = WMODEL_FILE_HEADER.size + 8 + material_count * entry_sizes[magic]
    if expected_size != len(payload):
        raise InventoryError("WModel material container payload size is invalid")
    if material_count < required_material_count:
        raise InventoryError("WModel mesh references a missing material entry")
    entry_size = entry_sizes[magic]
    entries_offset = WMODEL_FILE_HEADER.size + 8
    material_indices = [
        struct.unpack_from("<I", payload, entries_offset + row * entry_size)[0]
        for row in range(material_count)
    ]
    if any(value >= material_count for value in material_indices) or len(
        set(material_indices)
    ) != material_count:
        raise InventoryError("WModel material indices are out of range or duplicated")
    return {
        "outer": {
            "magic": outer[0].decode("ascii"),
            "majorVersion": outer[1],
            "minorVersion": outer[2],
            "flags": outer[3],
            "declaredContentByteSize": outer[4],
            "actualContentByteSize": len(payload) - WMODEL_FILE_HEADER.size,
        },
        "containerMagic": magic.decode("ascii"),
        "declaredMaterialCount": material_count,
        "entryByteSize": entry_sizes[magic],
        "materialIndices": material_indices,
        "minimumRequiredMaterialCount": required_material_count,
        "exactPayloadConsumption": True,
    }


def inspect_wmodel_bytes(data: bytes) -> dict[str, Any]:
    """Inspect WModel structure without assigning runtime material or scale policy."""

    if len(data) < WMODEL_FILE_HEADER.size + WMODEL_MODEL_HEADER.size:
        raise InventoryError("WModel is truncated")
    outer = WMODEL_FILE_HEADER.unpack_from(data, 0)
    if outer[:4] != (b"WINT", 1, 0, 0) or outer[4] != len(data) - WMODEL_FILE_HEADER.size:
        raise InventoryError("WModel outer WINT header is invalid")
    model = WMODEL_MODEL_HEADER.unpack_from(data, WMODEL_FILE_HEADER.size)
    if (
        model[0] != b"WMOD"
        or model[1] != 2
        or model[2] != 0
        or model[3] != 0
        or any(model[4:])
    ):
        raise InventoryError("WModel model header is invalid")
    section_count = model[1]
    table_offset = WMODEL_FILE_HEADER.size + WMODEL_MODEL_HEADER.size
    table_end = table_offset + section_count * WMODEL_SECTION_DESC.size
    if table_end > len(data):
        raise InventoryError("WModel section table is truncated")
    content_size = outer[4]
    minimum_content_offset = WMODEL_MODEL_HEADER.size + section_count * WMODEL_SECTION_DESC.size
    sections: list[dict[str, Any]] = []
    intervals: list[tuple[int, int, int]] = []
    mesh_inspections: list[dict[str, Any]] = []
    material_payloads: list[bytes] = []
    for row in range(section_count):
        type_id, index, offset, size, name_bytes = WMODEL_SECTION_DESC.unpack_from(
            data, table_offset + row * WMODEL_SECTION_DESC.size
        )
        if index != 0:
            raise InventoryError("WModel static section index must be zero")
        if offset < minimum_content_offset or size <= 0 or offset + size > content_size:
            raise InventoryError("WModel section range is invalid")
        begin = WMODEL_FILE_HEADER.size + offset
        end = begin + size
        intervals.append((offset, offset + size, row))
        section_payload = data[begin:end]
        row_result: dict[str, Any] = {
            "sectionRow": row,
            "typeId": type_id,
            "index": index,
            "offset": offset,
            "byteSize": size,
            "name": _fixed_name(name_bytes),
            "payloadMagicHex": section_payload[:4].hex(),
        }
        if type_id == 1:
            mesh = _inspect_wmodel_mesh(section_payload)
            row_result["meshInspectionIndex"] = len(mesh_inspections)
            mesh_inspections.append(mesh)
        elif type_id == 2:
            row_result["materialInspectionIndex"] = len(material_payloads)
            material_payloads.append(section_payload)
        else:
            raise InventoryError("WModel contains an unknown section type")
        sections.append(row_result)
    intervals.sort()
    expected = minimum_content_offset
    for begin, end, _ in intervals:
        if begin != expected:
            raise InventoryError("WModel sections overlap or contain gaps")
        expected = end
    if expected != content_size:
        raise InventoryError("WModel contains trailing unowned section bytes")
    if len(mesh_inspections) != 1 or len(material_payloads) != 1:
        raise InventoryError("WModel must contain exactly one mesh and one material section")
    mesh = mesh_inspections[0]
    if not mesh["exactPayloadConsumption"]:
        raise InventoryError("WModel mesh section contains trailing bytes")
    material = _inspect_wmodel_material(
        material_payloads[0],
        mesh["mesh"]["maximumReferencedMaterialIndex"] + 1,
    )
    return {
        "inspectionStatus": "STRUCTURALLY_VALID",
        "byteSize": len(data),
        "outer": {
            "magic": outer[0].decode("ascii"),
            "majorVersion": outer[1],
            "minorVersion": outer[2],
            "flags": outer[3],
            "declaredContentByteSize": outer[4],
            "actualContentByteSize": len(data) - WMODEL_FILE_HEADER.size,
        },
        "model": {
            "magic": model[0].decode("ascii"),
            "sectionCount": section_count,
            "animationCount": model[2],
            "flags": model[3],
            "reserved": list(model[4:]),
        },
        "sections": sections,
        "meshInspection": mesh,
        "materialInspection": material,
        "exactSectionConsumption": True,
    }


def inspect_wmodel_file(path: Path) -> dict[str, Any]:
    return inspect_wmodel_bytes(path.read_bytes())


def _is_within(path: Path, root: Path) -> bool:
    try:
        path.resolve().relative_to(root.resolve())
        return True
    except ValueError:
        return False


def _require_fresh_external_stage(
    staging_root: Path, repo_root: Path, checkpoint_sha256: str, run_id: str
) -> Path:
    if not SAFE_SEGMENT.fullmatch(run_id):
        raise InventoryError(f"run ID contains a non-portable segment: {run_id}")
    resolved = staging_root.resolve()
    if resolved.exists():
        raise InventoryError(f"fresh staging root already exists: {resolved}")
    if _is_within(resolved, repo_root):
        raise InventoryError("raw extraction staging must remain outside the repository")
    marker = checkpoint_sha256[:16]
    if marker not in resolved.name or run_id not in resolved.name:
        raise InventoryError(
            "staging basename must contain checkpoint SHA prefix and run ID"
        )
    resolved.parent.mkdir(parents=True, exist_ok=True)
    resolved.mkdir(exist_ok=False)
    return resolved


def _stage_identity(path: Path, staging_root: Path, kind: str) -> dict[str, Any]:
    resolved = path.resolve()
    try:
        relative = resolved.relative_to(staging_root.resolve()).as_posix()
    except ValueError as error:
        raise InventoryError(f"stage artifact escapes staging root: {path}") from error
    validate_portable_strings({"relativePath": relative})
    if not path.is_file():
        raise InventoryError(f"stage artifact is not a regular file: {path}")
    return {
        "kind": kind,
        "relativePath": relative,
        "fileName": path.name,
        "byteSize": path.stat().st_size,
        "sha256": sha256_file(path),
    }


def _portable_stage_error(error: BaseException, staging_root: Path) -> str:
    value = str(error).replace(str(staging_root), "$STAGING")
    value = value.replace(str(staging_root).replace("\\", "/"), "$STAGING")
    return value.replace("\\", "/")


def _portable_tool_identity(path: Path, locator: str, kind: str) -> dict[str, Any]:
    if not path.is_file():
        raise InventoryError(f"missing extraction tool {kind}: {path}")
    return {
        "locator": locator,
        "kind": kind,
        "fileName": path.name,
        "byteSize": path.stat().st_size,
        "sha256": sha256_file(path),
    }


def _converter_toolchain_identity(converter: Path) -> dict[str, Any]:
    companions = [
        _portable_tool_identity(
            path,
            f"external:Tools/ModelAssetConverter/{path.name}",
            "MODEL_ASSET_CONVERTER_COMPANION",
        )
        for path in sorted(
            converter.parent.glob("*.dll"), key=lambda value: value.name.casefold()
        )
        if path.is_file()
    ]
    return {
        "executable": _portable_tool_identity(
            converter,
            "external:Tools/ModelAssetConverter/ModelAssetConverter.exe",
            "MODEL_ASSET_CONVERTER",
        ),
        "companions": companions,
    }


def _write_process_log(
    completed: subprocess.CompletedProcess[str], log_path: Path
) -> None:
    log_path.parent.mkdir(parents=True, exist_ok=True)
    log_path.write_text(
        completed.stdout + "\n" + completed.stderr,
        encoding="utf-8",
        newline="\n",
    )


def _run_process(command: list[str], cwd: Path) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        command,
        cwd=cwd,
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
        check=False,
        creationflags=(subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0),
    )


def _exact_case_file(root: Path, expected_name: str) -> tuple[Path | None, list[str]]:
    matches = sorted(
        (path for path in root.iterdir() if path.name.casefold() == expected_name.casefold()),
        key=lambda value: value.name,
    )
    exact = [path for path in matches if path.name == expected_name and path.is_file()]
    return (exact[0] if len(exact) == 1 else None, [path.name for path in matches])


def _preflight_package_rows(
    corpus: str,
    package_inventory: dict[str, Any],
    requested_logical_packages: set[str],
    umodel: Path,
    package_root: Path,
    logs_root: Path,
    staging_root: Path,
    region: str,
) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for package in package_inventory.get("packages", []):
        logical_package = str(package["logicalPackage"])
        expected_name = str(package["physicalPackageFileName"])
        physical, case_matches = _exact_case_file(package_root, expected_name)
        row: dict[str, Any] = {
            "corpus": corpus,
            "logicalPackage": logical_package,
            "expectedPhysicalPackageFileName": expected_name,
            "expectedByteSize": package.get("byteSize"),
            "expectedSha256": package.get("sha256"),
            "casefoldInstalledMatches": case_matches,
            "requestedForRawExport": logical_package.casefold()
            in requested_logical_packages,
        }
        if physical is None:
            row["preflightStatus"] = "PHYSICAL_PACKAGE_EXACT_CASE_MISSING_OR_AMBIGUOUS"
            results.append(row)
            continue
        row["observedByteSize"] = physical.stat().st_size
        row["observedSha256"] = sha256_file(physical)
        if (
            row["observedByteSize"] != row["expectedByteSize"]
            or row["observedSha256"] != row["expectedSha256"]
        ):
            row["preflightStatus"] = "PHYSICAL_PACKAGE_BYTES_DRIFTED"
            results.append(row)
            continue
        if logical_package.casefold() not in requested_logical_packages:
            row["preflightStatus"] = "PIN_MATCH_NOT_DIRECTLY_EXPORTED"
            results.append(row)
            continue
        log_path = logs_root / "resolve" / f"{logical_package}.log"
        command = [
            str(umodel),
            "-list",
            "-game=lostark",
            f"-{region}",
            "-nameresolve",
            f"-path={package_root}",
            logical_package,
        ]
        completed = _run_process(command, umodel.parent)
        _write_process_log(completed, log_path)
        output = completed.stdout + "\n" + completed.stderr
        match = re.search(r"Loading package:\s+([^\r\n]+?\.upk)\s+Ver:", output)
        row["resolveExitCode"] = completed.returncode
        row["resolveLog"] = _stage_identity(
            log_path, staging_root, "UMODEL_RESOLVE_LOG"
        )
        if match is None:
            row["preflightStatus"] = "UMODEL_PHYSICAL_RESOLUTION_MISSING"
        else:
            observed = Path(match.group(1).strip()).name
            row["observedLoadedPhysicalPackageFileName"] = observed
            row["preflightStatus"] = (
                "PIN_AND_UMODEL_RESOLUTION_MATCH"
                if completed.returncode == 0 and observed == expected_name
                else "UMODEL_PHYSICAL_RESOLUTION_MISMATCH"
            )
        results.append(row)
    return results


def _chunked(rows: list[dict[str, Any]], size: int) -> Iterable[list[dict[str, Any]]]:
    for index in range(0, len(rows), size):
        yield rows[index : index + size]


def _run_umodel_exports(
    corpus: str,
    requests: list[dict[str, Any]],
    preflights: list[dict[str, Any]],
    umodel: Path,
    package_root: Path,
    raw_root: Path,
    logs_root: Path,
    staging_root: Path,
    region: str,
    chunk_size: int,
) -> tuple[list[dict[str, Any]], dict[str, dict[str, Any]]]:
    accepted = {
        row["logicalPackage"].casefold(): row
        for row in preflights
        if row["preflightStatus"] == "PIN_AND_UMODEL_RESOLUTION_MATCH"
    }
    grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for request in requests:
        grouped[str(request["logicalPackage"])].append(request)
    invocations: list[dict[str, Any]] = []
    request_invocations: dict[str, dict[str, Any]] = {}
    for logical_package in sorted(grouped, key=str.casefold):
        rows = sorted(
            grouped[logical_package],
            key=lambda value: (
                str(value["sourceAssetPath"]).casefold(),
                str(value["sourceAssetPath"]),
            ),
        )
        if logical_package.casefold() not in accepted:
            for row in rows:
                request_invocations[row["requestId"]] = {
                    "status": "PACKAGE_PREFLIGHT_FAILED"
                }
            continue
        for chunk_index, batch in enumerate(_chunked(rows, chunk_size), start=1):
            invocation_id = f"{corpus}:{logical_package}:{chunk_index:03d}"
            log_path = logs_root / "export" / f"{logical_package}.{chunk_index:03d}.log"
            command = [
                str(umodel),
                "-export",
                "-game=lostark",
                f"-{region}",
                "-nameresolve",
                f"-path={package_root}",
                f"-out={raw_root}",
                "-dds",
                "-gltf",
                "-nooverwrite",
            ]
            command.extend(
                f"-obj={str(row['sourceAssetPath']).rsplit('.', 1)[-1]}"
                for row in batch
            )
            command.append(logical_package)
            completed = _run_process(command, umodel.parent)
            _write_process_log(completed, log_path)
            invocation = {
                "invocationId": invocation_id,
                "corpus": corpus,
                "logicalPackage": logical_package,
                "chunkIndex": chunk_index,
                "requestedObjectCount": len(batch),
                "requestIds": [row["requestId"] for row in batch],
                "exitCode": completed.returncode,
                "options": [
                    "export",
                    "game=lostark",
                    f"region={region}",
                    "nameresolve",
                    "dds",
                    "gltf",
                    "nooverwrite-fresh-root",
                ],
                "log": _stage_identity(log_path, staging_root, "UMODEL_EXPORT_LOG"),
            }
            invocations.append(invocation)
            for row in batch:
                request_invocations[row["requestId"]] = {
                    "status": (
                        "PROCESS_COMPLETED"
                        if completed.returncode == 0
                        else "PROCESS_FAILED"
                    ),
                    "invocationId": invocation_id,
                    "exitCode": completed.returncode,
                }
    return invocations, request_invocations


def _package_scoped_files(raw_root: Path, logical_package: str) -> tuple[list[Path], list[str]]:
    if not raw_root.is_dir():
        return [], []
    directories = sorted(
        (
            path
            for path in raw_root.iterdir()
            if path.is_dir() and path.name.casefold() == logical_package.casefold()
        ),
        key=lambda value: value.name,
    )
    files = sorted(
        (path for directory in directories for path in directory.rglob("*") if path.is_file()),
        key=lambda value: (value.as_posix().casefold(), value.as_posix()),
    )
    return files, [path.name for path in directories]


def _gltf_buffer_identities(
    gltf_path: Path, staging_root: Path
) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    document = load_json(gltf_path)
    buffers = document.get("buffers")
    if not isinstance(buffers, list) or not buffers:
        raise InventoryError("glTF has no external buffer descriptors")
    identities: list[dict[str, Any]] = []
    for index, descriptor in enumerate(buffers):
        if not isinstance(descriptor, dict) or not isinstance(descriptor.get("uri"), str):
            raise InventoryError("glTF buffer descriptor is invalid")
        uri = descriptor["uri"]
        validate_portable_strings({"uri": uri})
        if ":" in uri or uri.startswith("data:"):
            raise InventoryError("glTF buffer URI is not a portable relative file")
        path = (gltf_path.parent / Path(uri)).resolve()
        if not _is_within(path, gltf_path.parent):
            raise InventoryError("glTF buffer URI escapes the glTF directory")
        identity = _stage_identity(path, staging_root, "GLTF_BUFFER")
        expected_size = descriptor.get("byteLength")
        identity["bufferIndex"] = index
        identity["declaredByteLength"] = expected_size
        identity["declaredByteLengthMatches"] = (
            type(expected_size) is int and expected_size == identity["byteSize"]
        )
        if not identity["declaredByteLengthMatches"]:
            raise InventoryError("glTF buffer byteLength differs from actual bytes")
        identities.append(identity)
    return identities, {
        "assetVersion": document.get("asset", {}).get("version")
        if isinstance(document.get("asset"), dict)
        else None,
        "sceneCount": len(document.get("scenes", []))
        if isinstance(document.get("scenes"), list)
        else None,
        "meshCount": len(document.get("meshes", []))
        if isinstance(document.get("meshes"), list)
        else None,
        "bufferCount": len(identities),
    }


def _mesh_destination(
    wmodel_root: Path, source_asset_path: str
) -> Path:
    segments = source_asset_path.split(".")
    if len(segments) < 2 or any(not SAFE_SEGMENT.fullmatch(value) for value in segments):
        raise InventoryError(f"unsafe mesh source path: {source_asset_path}")
    return wmodel_root.joinpath(*segments[:-1], segments[-1] + ".wmodel")


def _associate_and_inspect_requests(
    corpus: str,
    requests: list[dict[str, Any]],
    request_invocations: dict[str, dict[str, Any]],
    raw_root: Path,
    wmodel_root: Path,
    logs_root: Path,
    staging_root: Path,
    converter: Path,
) -> list[dict[str, Any]]:
    package_cache: dict[str, tuple[list[Path], list[str]]] = {}
    results: list[dict[str, Any]] = []
    for request in sorted(requests, key=lambda value: value["requestId"]):
        request_id = str(request["requestId"])
        logical_package = str(request["logicalPackage"])
        source_asset_path = str(request["sourceAssetPath"])
        role = str(request["role"])
        object_name = source_asset_path.rsplit(".", 1)[-1]
        process = request_invocations.get(
            request_id, {"status": "PACKAGE_PREFLIGHT_FAILED"}
        )
        result: dict[str, Any] = {
            "requestId": request_id,
            "corpus": corpus,
            "role": role,
            "sourceAssetPath": source_asset_path,
            "logicalPackage": logical_package,
            "expectedPhysicalPackageFileName": request[
                "physicalPackageFileName"
            ],
            "resourcesRelativeCandidateId": request[
                "resourcesRelativeCandidateId"
            ],
            "invocation": process,
            "payloads": [],
            "bindingStatus": "CANDIDATE_ONLY_GPU_SCHEMA_PENDING",
        }
        if process["status"] != "PROCESS_COMPLETED":
            result["extractionStatus"] = process["status"]
            results.append(result)
            continue
        if logical_package.casefold() not in package_cache:
            package_cache[logical_package.casefold()] = _package_scoped_files(
                raw_root, logical_package
            )
        package_files, directory_names = package_cache[logical_package.casefold()]
        result["observedPackageDirectories"] = directory_names
        if len(directory_names) != 1:
            result["extractionStatus"] = "OUTPUT_PACKAGE_DIRECTORY_MISSING_OR_AMBIGUOUS"
            results.append(result)
            continue
        result["observedPackageDirectoryCaseMatchesLogicalPackage"] = (
            directory_names[0] == logical_package
        )
        suffixes = {".dds", ".tga"} if role == "texture" else {".gltf"}
        casefold_matches = [
            path
            for path in package_files
            if path.stem.casefold() == object_name.casefold()
            and path.suffix.casefold() in suffixes
        ]
        exact_matches = [path for path in casefold_matches if path.stem == object_name]
        if not exact_matches:
            result["extractionStatus"] = (
                "OUTPUT_OBJECT_CASE_MISMATCH"
                if casefold_matches
                else "MISSING_EXACT_PRIMARY_OUTPUT"
            )
            result["casefoldPrimaryCandidates"] = [
                _stage_identity(path, staging_root, "CASEFOLD_PRIMARY_CANDIDATE")
                for path in casefold_matches
            ]
            results.append(result)
            continue
        if len(exact_matches) != 1:
            result["extractionStatus"] = "AMBIGUOUS_EXACT_PRIMARY_OUTPUT"
            result["primaryCandidates"] = [
                _stage_identity(path, staging_root, "AMBIGUOUS_PRIMARY")
                for path in exact_matches
            ]
            results.append(result)
            continue
        primary = exact_matches[0]
        if role == "texture":
            kind = "DDS" if primary.suffix.casefold() == ".dds" else "TGA"
            identity = _stage_identity(primary, staging_root, kind)
            result["payloads"].append(identity)
            if kind == "TGA":
                result["extractionStatus"] = "UNSUPPORTED_NON_DDS_TEXTURE_PRIMARY"
                results.append(result)
                continue
            try:
                inspection = inspect_dds_file(primary)
            except (OSError, InventoryError) as error:
                result["extractionStatus"] = "INVALID_DDS_STRUCTURE"
                result["inspectionError"] = _portable_stage_error(error, staging_root)
            else:
                result["ddsInspection"] = inspection
                result["extractionStatus"] = (
                    "EXPORTED_AND_STRUCTURALLY_INSPECTED"
                    if inspection["inspectionStatus"] == "STRUCTURALLY_VALID"
                    else "DDS_PAYLOAD_OR_FORMAT_BLOCKER"
                )
            results.append(result)
            continue

        gltf_identity = _stage_identity(primary, staging_root, "GLTF")
        result["payloads"].append(gltf_identity)
        try:
            buffers, gltf_facts = _gltf_buffer_identities(primary, staging_root)
        except (OSError, InventoryError) as error:
            result["extractionStatus"] = "INVALID_GLTF_OR_BUFFER_SET"
            result["inspectionError"] = _portable_stage_error(error, staging_root)
            results.append(result)
            continue
        result["payloads"].extend(buffers)
        result["gltfInspection"] = gltf_facts
        destination = _mesh_destination(wmodel_root, source_asset_path)
        if destination.exists():
            raise InventoryError(f"fresh WModel destination already exists: {destination}")
        destination.parent.mkdir(parents=True, exist_ok=True)
        log_path = logs_root / "converter" / f"{request_id}.log"
        completed = _run_process(
            [
                str(converter),
                str(primary),
                "-o",
                str(destination),
                "--pretransform",
                "--scale",
                "1",
                "--no-auto-textures",
            ],
            converter.parent,
        )
        _write_process_log(completed, log_path)
        result["offlineTransientCook"] = {
            "classification": "OFFLINE_TRANSIENT_COOK",
            "exitCode": completed.returncode,
            "pretransform": True,
            "conversionScale": 1.0,
            "autoTextures": False,
            "runtimeBindingGenerated": False,
            "log": _stage_identity(
                log_path, staging_root, "MODEL_ASSET_CONVERTER_LOG"
            ),
        }
        if completed.returncode != 0 or not destination.is_file():
            result["extractionStatus"] = "OFFLINE_WMODEL_CONVERSION_FAILED"
            results.append(result)
            continue
        wmodel_identity = _stage_identity(destination, staging_root, "WMODEL")
        result["payloads"].append(wmodel_identity)
        try:
            result["wmodelInspection"] = inspect_wmodel_file(destination)
        except (OSError, InventoryError) as error:
            result["extractionStatus"] = "INVALID_WMODEL_STRUCTURE"
            result["inspectionError"] = _portable_stage_error(error, staging_root)
        else:
            result["extractionStatus"] = "EXPORTED_AND_STRUCTURALLY_INSPECTED"
        results.append(result)
    return results


def _all_stage_files(staging_root: Path) -> list[Path]:
    return sorted(
        (path for path in staging_root.rglob("*") if path.is_file()),
        key=lambda value: (value.as_posix().casefold(), value.as_posix()),
    )


def build_extraction_run(
    repo_root: Path,
    checkpoint_path: Path,
    package_root: Path,
    umodel: Path,
    converter: Path,
    staging_root: Path,
    run_id: str,
    region: str,
    chunk_size: int,
) -> tuple[dict[str, Any], Path]:
    if chunk_size <= 0 or chunk_size > 128:
        raise InventoryError("export chunk size must be between 1 and 128")
    checkpoint_raw_sha = sha256_file(checkpoint_path)
    if checkpoint_raw_sha != EXPECTED_CHECKPOINT_RAW_SHA256:
        raise InventoryError("extraction checkpoint raw SHA differs from frozen checkpoint")
    checkpoint = load_json(checkpoint_path)
    validate_checkpoint(checkpoint)
    stage = _require_fresh_external_stage(
        staging_root, repo_root, checkpoint_raw_sha, run_id
    )
    umodel_identity = _portable_tool_identity(
        umodel,
        "external:Tools/UEViewerLostArk/umodel_lostark_v7.exe",
        "UMODEL_LOSTARK",
    )
    converter_identity = _converter_toolchain_identity(converter)
    corpora: dict[str, Any] = {}
    for corpus, checkpoint_key in (("FourClass", "fourClass"), ("Valtan", "valtan")):
        source = checkpoint[checkpoint_key]
        requests = source["assetRequests"]
        requested_packages = {
            str(row["logicalPackage"]).casefold() for row in requests
        }
        corpus_root = stage / corpus
        raw_root = corpus_root / "raw"
        wmodel_root = corpus_root / "wmodel"
        logs_root = corpus_root / "logs"
        raw_root.mkdir(parents=True, exist_ok=False)
        wmodel_root.mkdir(parents=True, exist_ok=False)
        logs_root.mkdir(parents=True, exist_ok=False)
        preflights = _preflight_package_rows(
            corpus,
            source["packageInventory"],
            requested_packages,
            umodel,
            package_root,
            logs_root,
            stage,
            region,
        )
        invocations, request_invocations = _run_umodel_exports(
            corpus,
            requests,
            preflights,
            umodel,
            package_root,
            raw_root,
            logs_root,
            stage,
            region,
            chunk_size,
        )
        results = _associate_and_inspect_requests(
            corpus,
            requests,
            request_invocations,
            raw_root,
            wmodel_root,
            logs_root,
            stage,
            converter,
        )
        status_counts = Counter(row["extractionStatus"] for row in results)
        corpora[corpus] = {
            "packagePreflights": preflights,
            "invocations": invocations,
            "requests": results,
            "summary": {
                "requestCount": len(results),
                "statusCounts": dict(sorted(status_counts.items())),
                "packagePreflightCount": len(preflights),
                "packagePreflightFailureCount": sum(
                    row["preflightStatus"]
                    not in (
                        "PIN_AND_UMODEL_RESOLUTION_MATCH",
                        "PIN_MATCH_NOT_DIRECTLY_EXPORTED",
                    )
                    for row in preflights
                ),
                "invocationCount": len(invocations),
                "processFailureCount": sum(row["exitCode"] != 0 for row in invocations),
            },
        }
    document: dict[str, Any] = {
        "schema": EXTRACTION_RECEIPT_SCHEMA,
        "formatVersion": 1,
        "mode": "OFFLINE_INVENTORY_ONLY",
        "sourceCommit": SOURCE_COMMIT,
        "runId": run_id,
        "stagingLocator": f"external:RawResourceInventory/Runs/{stage.name}",
        "checkpoint": {
            "locator": "Data/Effects/Imported/RawResourceInventory/"
            "R8.raw-resource-denominator.checkpoint.json",
            "fileName": checkpoint_path.name,
            "byteSize": checkpoint_path.stat().st_size,
            "sha256": checkpoint_raw_sha,
        },
        "tools": {
            "umodel": umodel_identity,
            "modelAssetConverter": converter_identity,
        },
        "conversionBoundary": {
            "classification": "OFFLINE_TRANSIENT_COOK",
            "pretransform": True,
            "conversionScale": 1.0,
            "autoTextures": False,
            "runtimeBindingGenerated": False,
            "gpuSchema": "UNFINALIZED_GPU_SCHEMA_PENDING",
        },
        "runtimeAuthority": False,
        "rendererReady": False,
        "admission": {
            "Execute": False,
            "Submit": False,
            "Render": False,
            "Product": False,
        },
        "corpora": corpora,
    }
    known_paths = {
        payload["relativePath"]
        for corpus in corpora.values()
        for request in corpus["requests"]
        for payload in request.get("payloads", [])
    }
    known_paths.update(
        row["log"]["relativePath"]
        for corpus in corpora.values()
        for row in corpus["invocations"]
    )
    known_paths.update(
        row["resolveLog"]["relativePath"]
        for corpus in corpora.values()
        for row in corpus["packagePreflights"]
        if "resolveLog" in row
    )
    known_paths.update(
        request["offlineTransientCook"]["log"]["relativePath"]
        for corpus in corpora.values()
        for request in corpus["requests"]
        if "offlineTransientCook" in request
    )
    unassociated = []
    for path in _all_stage_files(stage):
        relative = path.relative_to(stage).as_posix()
        if relative not in known_paths:
            unassociated.append(
                _stage_identity(path, stage, "UNASSOCIATED_FRESH_EXPORT_OUTPUT")
            )
    document["unassociatedFreshOutputs"] = unassociated
    document["summary"] = {
        "requestCount": sum(value["summary"]["requestCount"] for value in corpora.values()),
        "successfulRequestCount": sum(
            row["extractionStatus"] == "EXPORTED_AND_STRUCTURALLY_INSPECTED"
            for value in corpora.values()
            for row in value["requests"]
        ),
        "unassociatedFreshOutputCount": len(unassociated),
    }
    document = add_self_digest(document)
    validate_extraction_receipt(document)
    receipt_path = stage / "raw-resource-extraction-run-v1.json"
    write_json(receipt_path, document, replace=False)
    return document, receipt_path


def validate_extraction_receipt(document: dict[str, Any]) -> dict[str, Any]:
    if document.get("schema") != EXTRACTION_RECEIPT_SCHEMA:
        raise InventoryError("unexpected raw extraction receipt schema")
    if document.get("formatVersion") != 1 or isinstance(document.get("formatVersion"), bool):
        raise InventoryError("raw extraction receipt formatVersion must be exact integer 1")
    if document.get("mode") != "OFFLINE_INVENTORY_ONLY":
        raise InventoryError("raw extraction receipt is not offline-only")
    if document.get("sourceCommit") != SOURCE_COMMIT:
        raise InventoryError("raw extraction receipt source commit mismatch")
    if document.get("runtimeAuthority") is not False or document.get("rendererReady") is not False:
        raise InventoryError("raw extraction receipt runtime flags must be false")
    if document.get("admission") != {
        "Execute": False,
        "Submit": False,
        "Render": False,
        "Product": False,
    }:
        raise InventoryError("raw extraction receipt admission flags must be false")
    boundary = document.get("conversionBoundary", {})
    if boundary != {
        "classification": "OFFLINE_TRANSIENT_COOK",
        "pretransform": True,
        "conversionScale": 1.0,
        "autoTextures": False,
        "runtimeBindingGenerated": False,
        "gpuSchema": "UNFINALIZED_GPU_SCHEMA_PENDING",
    }:
        raise InventoryError("offline transient conversion boundary changed")
    if document.get("checkpoint", {}).get("sha256") != EXPECTED_CHECKPOINT_RAW_SHA256:
        raise InventoryError("raw extraction receipt checkpoint SHA mismatch")
    if document.get("selfDigest") != compute_self_digest(document):
        raise InventoryError("raw extraction receipt self digest mismatch")
    validate_portable_strings(document)
    expected_counts = {"FourClass": 835, "Valtan": 377}
    for corpus, expected in expected_counts.items():
        rows = document.get("corpora", {}).get(corpus, {}).get("requests", [])
        if len(rows) != expected:
            raise InventoryError(f"{corpus} extraction request denominator changed")
        request_ids = [row.get("requestId") for row in rows]
        if len(request_ids) != len(set(request_ids)):
            raise InventoryError(f"{corpus} extraction receipt has duplicate request IDs")
    return {
        "runId": document["runId"],
        "requestCount": document["summary"]["requestCount"],
        "successfulRequestCount": document["summary"]["successfulRequestCount"],
        "selfDigest": document["selfDigest"],
    }


def _iter_stage_identities(value: Any) -> Iterable[dict[str, Any]]:
    if isinstance(value, dict):
        if {
            "relativePath",
            "byteSize",
            "sha256",
        }.issubset(value) and isinstance(value.get("relativePath"), str):
            yield value
        for child in value.values():
            yield from _iter_stage_identities(child)
    elif isinstance(value, list):
        for child in value:
            yield from _iter_stage_identities(child)


def _resolve_stage_relative(staging_root: Path, relative: str) -> Path:
    validate_portable_strings({"relativePath": relative})
    candidate = (staging_root / Path(relative)).resolve()
    if not _is_within(candidate, staging_root):
        raise InventoryError(f"stage-relative path escapes staging root: {relative}")
    return candidate


def verify_extraction_stage(
    receipt: dict[str, Any], receipt_path: Path, staging_root: Path
) -> dict[str, Any]:
    validate_extraction_receipt(receipt)
    if sha256_file(receipt_path) != EXPECTED_EXTRACTION_RECEIPT_SHA256:
        raise InventoryError("extraction receipt raw SHA differs from frozen run")
    expected_stage_name = str(receipt["stagingLocator"]).rsplit("/", 1)[-1]
    if staging_root.name != expected_stage_name:
        raise InventoryError("staging root basename differs from portable staging locator")
    identities_by_path: dict[str, dict[str, Any]] = {}
    for identity in _iter_stage_identities(receipt):
        relative = identity["relativePath"]
        prior = identities_by_path.get(relative)
        projection = {
            "byteSize": identity["byteSize"],
            "sha256": identity["sha256"],
        }
        if prior is not None and prior != projection:
            raise InventoryError(f"conflicting stage identity for {relative}")
        identities_by_path[relative] = projection
    for relative, expected in identities_by_path.items():
        path = _resolve_stage_relative(staging_root, relative)
        if not path.is_file():
            raise InventoryError(f"missing frozen stage artifact: {relative}")
        if path.stat().st_size != expected["byteSize"] or sha256_file(path) != expected["sha256"]:
            raise InventoryError(f"frozen stage artifact bytes changed: {relative}")
    actual_paths = {
        path.relative_to(staging_root).as_posix()
        for path in _all_stage_files(staging_root)
        if path.resolve() != receipt_path.resolve()
    }
    expected_paths = set(identities_by_path)
    if actual_paths != expected_paths:
        missing = sorted(expected_paths - actual_paths)
        unexpected = sorted(actual_paths - expected_paths)
        raise InventoryError(
            f"frozen stage file set changed: missing={missing[:5]} unexpected={unexpected[:5]}"
        )
    return {
        "verifiedArtifactCount": len(expected_paths),
        "verifiedArtifactByteSize": sum(
            int(row["byteSize"]) for row in identities_by_path.values()
        ),
        "receiptRawSha256": sha256_file(receipt_path),
    }


def _reinspect_raw_request(
    request: dict[str, Any], staging_root: Path
) -> dict[str, Any]:
    result = copy.deepcopy(request)
    result.pop("ddsInspection", None)
    result.pop("wmodelInspection", None)
    result.pop("gltfInspection", None)
    result.pop("inspectionError", None)
    payloads = result.get("payloads", [])
    by_kind: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for payload in payloads:
        by_kind[str(payload["kind"])].append(payload)
    if result["role"] == "texture":
        if len(by_kind.get("DDS", [])) == 1:
            path = _resolve_stage_relative(
                staging_root, by_kind["DDS"][0]["relativePath"]
            )
            try:
                inspection = inspect_dds_file(path)
            except (OSError, InventoryError) as error:
                result["extractionStatus"] = "INVALID_DDS_STRUCTURE_AT_FINALIZATION"
                result["inspectionError"] = _portable_stage_error(error, staging_root)
            else:
                result["ddsInspection"] = inspection
                result["extractionStatus"] = (
                    "EXPORTED_AND_STRUCTURALLY_INSPECTED"
                    if inspection["inspectionStatus"] == "STRUCTURALLY_VALID"
                    else "DDS_PAYLOAD_OR_FORMAT_BLOCKER"
                )
        elif len(by_kind.get("TGA", [])) == 1 and not by_kind.get("DDS"):
            result["extractionStatus"] = "UNSUPPORTED_NON_DDS_TEXTURE_PRIMARY"
        else:
            result["extractionStatus"] = "MISSING_OR_AMBIGUOUS_TEXTURE_PAYLOAD"
        return result

    if (
        len(by_kind.get("GLTF", [])) != 1
        or not by_kind.get("GLTF_BUFFER")
        or len(by_kind.get("WMODEL", [])) != 1
    ):
        result["extractionStatus"] = "MISSING_OR_AMBIGUOUS_MESH_PAYLOAD_SET"
        return result
    gltf_path = _resolve_stage_relative(
        staging_root, by_kind["GLTF"][0]["relativePath"]
    )
    wmodel_path = _resolve_stage_relative(
        staging_root, by_kind["WMODEL"][0]["relativePath"]
    )
    try:
        observed_buffers, gltf_facts = _gltf_buffer_identities(
            gltf_path, staging_root
        )
        expected_buffers = sorted(
            (
                {
                    key: row[key]
                    for key in (
                        "kind",
                        "relativePath",
                        "fileName",
                        "byteSize",
                        "sha256",
                        "bufferIndex",
                        "declaredByteLength",
                        "declaredByteLengthMatches",
                    )
                }
                for row in by_kind["GLTF_BUFFER"]
            ),
            key=lambda value: value["bufferIndex"],
        )
        if canonical_bytes(observed_buffers) != canonical_bytes(expected_buffers):
            raise InventoryError("glTF buffer identity set differs from extraction receipt")
        wmodel_inspection = inspect_wmodel_file(wmodel_path)
    except (OSError, InventoryError) as error:
        result["extractionStatus"] = "INVALID_MESH_PAYLOAD_SET_AT_FINALIZATION"
        result["inspectionError"] = _portable_stage_error(error, staging_root)
    else:
        result["gltfInspection"] = gltf_facts
        result["wmodelInspection"] = wmodel_inspection
        result["extractionStatus"] = "EXPORTED_AND_STRUCTURALLY_INSPECTED"
    return result


def _primary_candidate_payload(raw_resource: dict[str, Any]) -> dict[str, Any] | None:
    desired = "WMODEL" if raw_resource.get("role") == "mesh" else "DDS"
    payloads = [
        row for row in raw_resource.get("payloads", []) if row.get("kind") == desired
    ]
    if not payloads and raw_resource.get("role") == "texture":
        payloads = [
            row for row in raw_resource.get("payloads", []) if row.get("kind") == "TGA"
        ]
    return payloads[0] if len(payloads) == 1 else None


def derive_raw_blocker_evidence(document: dict[str, Any]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for corpus_key, corpus_name in (("fourClass", "FourClass"), ("valtan", "Valtan")):
        for request in document.get(corpus_key, {}).get("assetRequests", []):
            raw = request.get("rawResource")
            if not isinstance(raw, dict):
                rows.append(
                    {
                        "code": "RAW_RESOURCE_RESULT_MISSING",
                        "corpus": corpus_name,
                        "requestId": request.get("requestId"),
                        "role": request.get("role"),
                        "sourceAssetPath": request.get("sourceAssetPath"),
                        "resourcesRelativeCandidateId": request.get(
                            "resourcesRelativeCandidateId"
                        ),
                        "payloads": [],
                    }
                )
                continue
            if raw.get("extractionStatus") == "EXPORTED_AND_STRUCTURALLY_INSPECTED":
                continue
            rows.append(
                {
                    "code": raw.get("extractionStatus"),
                    "corpus": corpus_name,
                    "requestId": request.get("requestId"),
                    "role": request.get("role"),
                    "sourceAssetPath": request.get("sourceAssetPath"),
                    "resourcesRelativeCandidateId": request.get(
                        "resourcesRelativeCandidateId"
                    ),
                    "payloads": [
                        {
                            key: payload[key]
                            for key in (
                                "kind",
                                "relativePath",
                                "byteSize",
                                "sha256",
                            )
                        }
                        for payload in raw.get("payloads", [])
                    ],
                }
            )
    return sorted(
        rows,
        key=lambda row: (
            str(row["corpus"]).casefold(),
            str(row["requestId"]),
        ),
    )


def raw_blocker_evidence_sha256(rows: list[dict[str, Any]]) -> str:
    return hashlib.sha256(canonical_bytes(rows)).hexdigest()


def raw_resource_projection(document: dict[str, Any]) -> dict[str, Any]:
    extraction = document.get("rawExtraction", {})
    receipt = extraction.get("receipt", {})
    return {
        "runId": extraction.get("runId"),
        "stagingLocator": extraction.get("stagingLocator"),
        "receipt": {
            key: receipt.get(key)
            for key in ("locator", "fileName", "byteSize", "sha256", "selfDigest")
        },
        "tools": extraction.get("tools"),
        "conversionBoundary": extraction.get("conversionBoundary"),
        "stageVerification": extraction.get("stageVerification"),
        "corpora": extraction.get("corpora"),
        "unassociatedFreshOutputs": extraction.get("unassociatedFreshOutputs"),
        "requests": [
            {
                "corpus": corpus_name,
                "requestId": request.get("requestId"),
                "rawResource": request.get("rawResource"),
            }
            for corpus_key, corpus_name in (
                ("fourClass", "FourClass"),
                ("valtan", "Valtan"),
            )
            for request in document.get(corpus_key, {}).get("assetRequests", [])
        ],
    }


def raw_resource_projection_sha256(document: dict[str, Any]) -> str:
    return hashlib.sha256(canonical_bytes(raw_resource_projection(document))).hexdigest()


def reconstruct_checkpoint_projection(document: dict[str, Any]) -> dict[str, Any]:
    """Remove final-only evidence and reconstruct the exact frozen checkpoint document."""

    projection = copy.deepcopy(document)
    for key in (
        "checkpointProvenance",
        "rawBlockerEvidence",
        "rawBlockerEvidenceSha256",
        "rawExtraction",
        "rawResourceProjectionSha256",
        "inventoryStatus",
    ):
        projection.pop(key, None)
    projection["phase"] = "DENOMINATOR_CHECKPOINT"
    projection["blockerCount"] = len(projection.get("blockerEvidence", []))
    reports = projection.get("reports", {})
    for key in (
        "rawResourceBlockers",
        "rawStructuralFailures",
        "unsupportedNonDdsTextures",
        "caseOnlyCandidateIdCollisions",
        "candidateNameCollisions",
        "basenameCollisions",
    ):
        reports.pop(key, None)
    reports["rawExtractionMissing"] = []
    reports["rawOutputAmbiguities"] = []
    reports["sharedByteGroups"] = []
    for corpus_key in ("fourClass", "valtan"):
        for request in projection.get(corpus_key, {}).get("assetRequests", []):
            request.pop("rawResource", None)
    projection["selfDigest"] = EXPECTED_CHECKPOINT_SELF_DIGEST
    return projection


def _build_shared_byte_groups(document: dict[str, Any]) -> list[dict[str, Any]]:
    grouped: dict[str, list[dict[str, Any]]] = defaultdict(list)
    byte_sizes: dict[str, set[int]] = defaultdict(set)
    for corpus_key, corpus_name in (("fourClass", "FourClass"), ("valtan", "Valtan")):
        for request in document[corpus_key]["assetRequests"]:
            raw = request["rawResource"]
            for payload in raw.get("payloads", []):
                sha = str(payload["sha256"])
                byte_sizes[sha].add(int(payload["byteSize"]))
                grouped[sha].append(
                    {
                        "corpus": corpus_name,
                        "requestId": request["requestId"],
                        "role": request["role"],
                        "sourceAssetPath": request["sourceAssetPath"],
                        "kind": payload["kind"],
                        "relativePath": payload["relativePath"],
                    }
                )
    results = []
    for sha, aliases in grouped.items():
        request_ids = {row["requestId"] for row in aliases}
        if len(request_ids) <= 1:
            continue
        if len(byte_sizes[sha]) != 1:
            raise InventoryError("identical SHA has conflicting byte sizes")
        aliases.sort(
            key=lambda row: (
                row["corpus"].casefold(),
                row["requestId"],
                row["kind"],
                row["relativePath"].casefold(),
            )
        )
        results.append(
            {
                "groupId": f"rawsha.{sha[:24]}",
                "sha256": sha,
                "byteSize": next(iter(byte_sizes[sha])),
                "requestAliasCount": len(aliases),
                "distinctRequestCount": len(request_ids),
                "payloadKinds": sorted({row["kind"] for row in aliases}),
                "aliases": aliases,
            }
        )
    return sorted(results, key=lambda row: row["sha256"])


def _build_candidate_collision_reports(document: dict[str, Any]) -> dict[str, Any]:
    rows = []
    for corpus_key, corpus_name in (("fourClass", "FourClass"), ("valtan", "Valtan")):
        for request in document[corpus_key]["assetRequests"]:
            primary = _primary_candidate_payload(request["rawResource"])
            rows.append(
                {
                    "corpus": corpus_name,
                    "requestId": request["requestId"],
                    "role": request["role"],
                    "sourceAssetPath": request["sourceAssetPath"],
                    "resourcesRelativeCandidateId": request[
                        "resourcesRelativeCandidateId"
                    ],
                    "primarySha256": primary["sha256"] if primary else None,
                }
            )
    casefold_groups: dict[str, list[dict[str, Any]]] = defaultdict(list)
    exact_groups: dict[str, list[dict[str, Any]]] = defaultdict(list)
    basename_groups: dict[tuple[str, str, str], list[dict[str, Any]]] = defaultdict(list)
    for row in rows:
        candidate = row["resourcesRelativeCandidateId"]
        casefold_groups[candidate.casefold()].append(row)
        exact_groups[candidate].append(row)
        basename = row["sourceAssetPath"].rsplit(".", 1)[-1]
        basename_groups[(row["corpus"], row["role"], basename.casefold())].append(row)
    case_only = [
        {
            "casefoldCandidateId": key,
            "rows": value,
        }
        for key, value in casefold_groups.items()
        if len({row["resourcesRelativeCandidateId"] for row in value}) > 1
    ]
    name_collisions = [
        {
            "resourcesRelativeCandidateId": key,
            "rows": value,
        }
        for key, value in exact_groups.items()
        if len({row["primarySha256"] for row in value}) > 1
    ]
    basename = [
        {
            "corpus": key[0],
            "role": key[1],
            "basenameCasefold": key[2],
            "resolutionStatus": "FULL_SOURCE_PATH_CANDIDATE_DISAMBIGUATES",
            "rows": value,
        }
        for key, value in basename_groups.items()
        if len({row["sourceAssetPath"].casefold() for row in value}) > 1
    ]
    return {
        "caseOnlyCandidateIdCollisions": sorted(
            case_only, key=lambda row: row["casefoldCandidateId"]
        ),
        "candidateNameCollisions": sorted(
            name_collisions,
            key=lambda row: row["resourcesRelativeCandidateId"].casefold(),
        ),
        "basenameCollisions": sorted(
            basename,
            key=lambda row: (
                row["corpus"].casefold(),
                row["role"],
                row["basenameCasefold"],
            ),
        ),
    }


def build_final_inventory(
    repo_root: Path,
    checkpoint_path: Path,
    receipt_path: Path,
    staging_root: Path,
) -> dict[str, Any]:
    if sha256_file(checkpoint_path) != EXPECTED_CHECKPOINT_RAW_SHA256:
        raise InventoryError("finalization checkpoint raw SHA changed")
    checkpoint = load_json(checkpoint_path)
    validate_checkpoint(checkpoint)
    receipt = load_json(receipt_path)
    validate_extraction_receipt(receipt)
    stage_verification = verify_extraction_stage(receipt, receipt_path, staging_root)
    document = copy.deepcopy(checkpoint)
    document["phase"] = FINAL_PHASE
    document["checkpointProvenance"] = {
        "locator": "Data/Effects/Imported/RawResourceInventory/"
        "R8.raw-resource-denominator.checkpoint.json",
        "fileName": checkpoint_path.name,
        "byteSize": checkpoint_path.stat().st_size,
        "sha256": EXPECTED_CHECKPOINT_RAW_SHA256,
        "selfDigest": checkpoint["selfDigest"],
        "blockerEvidenceSha256": checkpoint["blockerEvidenceSha256"],
    }
    receipt_requests = {
        row["requestId"]: row
        for corpus in receipt["corpora"].values()
        for row in corpus["requests"]
    }
    expected_request_ids = {
        row["requestId"]
        for key in ("fourClass", "valtan")
        for row in document[key]["assetRequests"]
    }
    if set(receipt_requests) != expected_request_ids:
        raise InventoryError("extraction receipt request IDs differ from checkpoint denominator")
    for corpus_key in ("fourClass", "valtan"):
        for request in document[corpus_key]["assetRequests"]:
            raw = receipt_requests[request["requestId"]]
            for key in (
                "role",
                "sourceAssetPath",
                "logicalPackage",
                "resourcesRelativeCandidateId",
            ):
                if raw.get(key) != request.get(key):
                    raise InventoryError(
                        f"raw extraction request field differs for {request['requestId']}: {key}"
                    )
            if raw.get("expectedPhysicalPackageFileName") != request.get(
                "physicalPackageFileName"
            ):
                raise InventoryError("raw extraction physical package expectation changed")
            request["rawResource"] = _reinspect_raw_request(raw, staging_root)
    shared_groups = _build_shared_byte_groups(document)
    collisions = _build_candidate_collision_reports(document)
    raw_blockers = derive_raw_blocker_evidence(document)
    raw_blocker_sha = raw_blocker_evidence_sha256(raw_blockers)
    if (
        EXPECTED_RAW_BLOCKER_PROJECTION_SHA256 is not None
        and raw_blocker_sha != EXPECTED_RAW_BLOCKER_PROJECTION_SHA256
    ):
        raise InventoryError("raw blocker evidence differs from frozen extraction baseline")
    reports = document["reports"]
    reports["rawResourceBlockers"] = copy.deepcopy(raw_blockers)
    reports["rawExtractionMissing"] = [
        row
        for row in raw_blockers
        if any(token in str(row["code"]) for token in ("MISSING", "FAILED", "PREFLIGHT"))
    ]
    reports["rawOutputAmbiguities"] = [
        row
        for row in raw_blockers
        if any(token in str(row["code"]) for token in ("AMBIGUOUS", "CASE_MISMATCH"))
    ]
    reports["rawStructuralFailures"] = [
        row
        for row in raw_blockers
        if any(token in str(row["code"]) for token in ("INVALID", "PAYLOAD_OR_FORMAT"))
    ]
    reports["unsupportedNonDdsTextures"] = [
        row
        for row in raw_blockers
        if row["code"] == "UNSUPPORTED_NON_DDS_TEXTURE_PRIMARY"
    ]
    reports["sharedByteGroups"] = shared_groups
    reports.update(collisions)
    document["rawBlockerEvidence"] = raw_blockers
    document["rawBlockerEvidenceSha256"] = raw_blocker_sha
    document["blockerCount"] = len(document["blockerEvidence"]) + len(raw_blockers)
    document["inventoryStatus"] = (
        "FROZEN_WITH_BLOCKERS" if document["blockerCount"] else "FROZEN_COMPLETE"
    )
    payload_kind_counts = Counter(
        payload["kind"]
        for key in ("fourClass", "valtan")
        for request in document[key]["assetRequests"]
        for payload in request["rawResource"].get("payloads", [])
    )
    payloads = [
        payload
        for key in ("fourClass", "valtan")
        for request in document[key]["assetRequests"]
        for payload in request["rawResource"].get("payloads", [])
    ]
    status_counts = Counter(
        request["rawResource"]["extractionStatus"]
        for key in ("fourClass", "valtan")
        for request in document[key]["assetRequests"]
    )
    document["rawExtraction"] = {
        "runId": receipt["runId"],
        "stagingLocator": receipt["stagingLocator"],
        "receipt": {
            "locator": f"{receipt['stagingLocator']}/{receipt_path.name}",
            "fileName": receipt_path.name,
            "byteSize": receipt_path.stat().st_size,
            "sha256": sha256_file(receipt_path),
            "selfDigest": receipt["selfDigest"],
        },
        "tools": receipt["tools"],
        "conversionBoundary": receipt["conversionBoundary"],
        "stageVerification": stage_verification,
        "corpora": {
            key: {
                "packagePreflights": value["packagePreflights"],
                "invocations": value["invocations"],
                "summary": value["summary"],
            }
            for key, value in receipt["corpora"].items()
        },
        "unassociatedFreshOutputs": receipt["unassociatedFreshOutputs"],
        "summary": {
            "requestCount": len(expected_request_ids),
            "statusCounts": dict(sorted(status_counts.items())),
            "successfulRequestCount": status_counts[
                "EXPORTED_AND_STRUCTURALLY_INSPECTED"
            ],
            "rawBlockerCount": len(raw_blockers),
            "payloadCount": len(payloads),
            "payloadKindCounts": dict(sorted(payload_kind_counts.items())),
            "payloadByteSizeWithAliases": sum(int(row["byteSize"]) for row in payloads),
            "uniquePayloadSha256Count": len({row["sha256"] for row in payloads}),
            "sharedByteGroupCount": len(shared_groups),
            "unassociatedFreshOutputCount": len(
                receipt["unassociatedFreshOutputs"]
            ),
        },
    }
    document["rawResourceProjectionSha256"] = raw_resource_projection_sha256(
        document
    )
    if (
        EXPECTED_RAW_RESOURCE_PROJECTION_SHA256 is not None
        and document["rawResourceProjectionSha256"]
        != EXPECTED_RAW_RESOURCE_PROJECTION_SHA256
    ):
        raise InventoryError("raw resource projection differs from frozen extraction")
    document = add_self_digest(document)
    validate_final_inventory(document)
    return document


def validate_final_inventory(document: dict[str, Any]) -> dict[str, Any]:
    if document.get("schema") != "lostark.raw-resource-inventory-v1":
        raise InventoryError("unexpected final raw inventory schema")
    if document.get("formatVersion") != 1 or isinstance(document.get("formatVersion"), bool):
        raise InventoryError("final raw inventory formatVersion must be exact integer 1")
    if document.get("phase") != FINAL_PHASE:
        raise InventoryError("expected RAW_BYTES_INSPECTED final phase")
    if document.get("mode") != "OFFLINE_INVENTORY_ONLY":
        raise InventoryError("final raw inventory must remain offline-only")
    if document.get("sourceCommit") != SOURCE_COMMIT:
        raise InventoryError("final raw inventory source commit mismatch")
    if document.get("runtimeAuthority") is not False or document.get("rendererReady") is not False:
        raise InventoryError("final raw inventory runtime flags must be false")
    if document.get("admission") != {
        "Execute": False,
        "Submit": False,
        "Render": False,
        "Product": False,
    }:
        raise InventoryError("final raw inventory admission flags must be false")
    if document.get("gpuSchemaBoundary") != GPU_SCHEMA_BOUNDARY:
        raise InventoryError("final raw inventory GPU schema boundary changed")
    if document.get("selfDigest") != compute_self_digest(document):
        raise InventoryError("final raw inventory self digest mismatch")
    validate_portable_strings(document)
    expected_checkpoint_provenance = {
        "locator": "Data/Effects/Imported/RawResourceInventory/"
        "R8.raw-resource-denominator.checkpoint.json",
        "fileName": "R8.raw-resource-denominator.checkpoint.json",
        "byteSize": EXPECTED_CHECKPOINT_BYTE_SIZE,
        "sha256": EXPECTED_CHECKPOINT_RAW_SHA256,
        "selfDigest": EXPECTED_CHECKPOINT_SELF_DIGEST,
        "blockerEvidenceSha256": EXPECTED_CHECKPOINT_BLOCKER_PROJECTION_SHA256,
    }
    if document.get("checkpointProvenance") != expected_checkpoint_provenance:
        raise InventoryError("final raw inventory checkpoint provenance changed")
    checkpoint_projection = reconstruct_checkpoint_projection(document)
    if compute_self_digest(checkpoint_projection) != EXPECTED_CHECKPOINT_SELF_DIGEST:
        raise InventoryError("final embedded checkpoint corpus differs from frozen checkpoint")
    extraction = document.get("rawExtraction", {})
    expected_extraction_keys = {
        "runId",
        "stagingLocator",
        "receipt",
        "tools",
        "conversionBoundary",
        "stageVerification",
        "corpora",
        "unassociatedFreshOutputs",
        "summary",
    }
    if set(extraction) != expected_extraction_keys:
        raise InventoryError("final raw extraction field set changed")
    expected_receipt_keys = {
        "locator",
        "fileName",
        "byteSize",
        "sha256",
        "selfDigest",
    }
    if set(extraction.get("receipt", {})) != expected_receipt_keys:
        raise InventoryError("final raw extraction receipt field set changed")
    if extraction.get("receipt", {}).get("sha256") != EXPECTED_EXTRACTION_RECEIPT_SHA256:
        raise InventoryError("final raw inventory extraction receipt changed")
    raw_projection_sha = raw_resource_projection_sha256(document)
    if document.get("rawResourceProjectionSha256") != raw_projection_sha:
        raise InventoryError("final raw resource projection SHA mismatch")
    if (
        EXPECTED_RAW_RESOURCE_PROJECTION_SHA256 is not None
        and raw_projection_sha != EXPECTED_RAW_RESOURCE_PROJECTION_SHA256
    ):
        raise InventoryError("final raw resource projection differs from frozen baseline")
    for key, expected in FOUR_CLASS_EXPECTED.items():
        if document.get("fourClass", {}).get("summary", {}).get(key) != expected:
            raise InventoryError(f"final four-class summary mismatch for {key}")
    for key, expected in VALTAN_EXPECTED.items():
        if document.get("valtan", {}).get("summary", {}).get(key) != expected:
            raise InventoryError(f"final Valtan summary mismatch for {key}")
    checkpoint_blockers = derive_blocker_evidence(document)
    if blocker_evidence_sha256(checkpoint_blockers) != EXPECTED_CHECKPOINT_BLOCKER_PROJECTION_SHA256:
        raise InventoryError("final inventory checkpoint blocker evidence changed")
    if canonical_bytes(document.get("blockerEvidence", [])) != canonical_bytes(
        checkpoint_blockers
    ):
        raise InventoryError("final checkpoint blocker projection mismatch")
    raw_blockers = derive_raw_blocker_evidence(document)
    if canonical_bytes(document.get("rawBlockerEvidence", [])) != canonical_bytes(
        raw_blockers
    ):
        raise InventoryError("final raw blocker projection mismatch")
    raw_sha = raw_blocker_evidence_sha256(raw_blockers)
    if document.get("rawBlockerEvidenceSha256") != raw_sha:
        raise InventoryError("final raw blocker evidence SHA mismatch")
    if (
        EXPECTED_RAW_BLOCKER_PROJECTION_SHA256 is not None
        and raw_sha != EXPECTED_RAW_BLOCKER_PROJECTION_SHA256
    ):
        raise InventoryError("final raw blocker evidence differs from frozen baseline")
    expected_blockers = len(checkpoint_blockers) + len(raw_blockers)
    if document.get("blockerCount") != expected_blockers:
        raise InventoryError("final raw inventory blockerCount mismatch")
    expected_status = "FROZEN_WITH_BLOCKERS" if expected_blockers else "FROZEN_COMPLETE"
    if document.get("inventoryStatus") != expected_status:
        raise InventoryError("final raw inventory status mismatch")
    if canonical_bytes(document.get("reports", {}).get("rawResourceBlockers", [])) != canonical_bytes(
        raw_blockers
    ):
        raise InventoryError("final raw blocker report differs from evidence")
    expected_report_subsets = {
        "rawExtractionMissing": [
            row
            for row in raw_blockers
            if any(
                token in str(row["code"])
                for token in ("MISSING", "FAILED", "PREFLIGHT")
            )
        ],
        "rawOutputAmbiguities": [
            row
            for row in raw_blockers
            if any(
                token in str(row["code"])
                for token in ("AMBIGUOUS", "CASE_MISMATCH")
            )
        ],
        "rawStructuralFailures": [
            row
            for row in raw_blockers
            if any(
                token in str(row["code"])
                for token in ("INVALID", "PAYLOAD_OR_FORMAT")
            )
        ],
        "unsupportedNonDdsTextures": [
            row
            for row in raw_blockers
            if row["code"] == "UNSUPPORTED_NON_DDS_TEXTURE_PRIMARY"
        ],
    }
    for key, expected in expected_report_subsets.items():
        if canonical_bytes(document["reports"].get(key, [])) != canonical_bytes(expected):
            raise InventoryError(f"final {key} report differs from evidence")
    requests = [
        request
        for key in ("fourClass", "valtan")
        for request in document[key]["assetRequests"]
    ]
    if len(requests) != 1212 or any(
        request.get("rawResource", {}).get("requestId") != request.get("requestId")
        for request in requests
    ):
        raise InventoryError("final raw request denominator or join changed")
    candidates = [request["resourcesRelativeCandidateId"] for request in requests]
    if len(candidates) != len(set(candidates)) or len(candidates) != len(
        {value.casefold() for value in candidates}
    ):
        raise InventoryError("final raw candidate IDs collide")
    collision_reports = _build_candidate_collision_reports(document)
    for key, expected in collision_reports.items():
        if canonical_bytes(document["reports"].get(key, [])) != canonical_bytes(expected):
            raise InventoryError(f"final {key} report changed")
    shared = _build_shared_byte_groups(document)
    if canonical_bytes(document["reports"].get("sharedByteGroups", [])) != canonical_bytes(shared):
        raise InventoryError("final shared-byte groups changed")
    payloads = [
        payload
        for request in requests
        for payload in request["rawResource"].get("payloads", [])
    ]
    status_counts = Counter(
        request["rawResource"]["extractionStatus"] for request in requests
    )
    payload_kind_counts = Counter(payload["kind"] for payload in payloads)
    expected_raw_summary = {
        "requestCount": len(requests),
        "statusCounts": dict(sorted(status_counts.items())),
        "successfulRequestCount": status_counts[
            "EXPORTED_AND_STRUCTURALLY_INSPECTED"
        ],
        "rawBlockerCount": len(raw_blockers),
        "payloadCount": len(payloads),
        "payloadKindCounts": dict(sorted(payload_kind_counts.items())),
        "payloadByteSizeWithAliases": sum(int(row["byteSize"]) for row in payloads),
        "uniquePayloadSha256Count": len({row["sha256"] for row in payloads}),
        "sharedByteGroupCount": len(shared),
        "unassociatedFreshOutputCount": len(
            document["rawExtraction"].get("unassociatedFreshOutputs", [])
        ),
    }
    if canonical_bytes(document["rawExtraction"].get("summary", {})) != canonical_bytes(
        expected_raw_summary
    ):
        raise InventoryError("final raw extraction summary differs from request evidence")
    return {
        "phase": document["phase"],
        "inventoryStatus": document["inventoryStatus"],
        "blockerCount": document["blockerCount"],
        "rawBlockerCount": len(raw_blockers),
        "requestCount": len(requests),
        "successfulRequestCount": sum(
            request["rawResource"]["extractionStatus"]
            == "EXPORTED_AND_STRUCTURALLY_INSPECTED"
            for request in requests
        ),
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
    extract = subparsers.add_parser("extract")
    extract.add_argument("--repo-root", required=True, type=Path)
    extract.add_argument("--checkpoint", required=True, type=Path)
    extract.add_argument("--package-root", required=True, type=Path)
    extract.add_argument("--umodel", required=True, type=Path)
    extract.add_argument("--converter", required=True, type=Path)
    extract.add_argument("--staging-root", required=True, type=Path)
    extract.add_argument("--run-id", required=True)
    extract.add_argument(
        "--region", default="kr", choices=("kr", "na", "ru", "jp", "tw", "cn")
    )
    extract.add_argument("--chunk-size", type=int, default=DEFAULT_EXPORT_CHUNK_SIZE)
    validate_extraction = subparsers.add_parser("validate-extraction")
    validate_extraction.add_argument("--receipt", required=True, type=Path)
    finalize = subparsers.add_parser("finalize")
    finalize.add_argument("--repo-root", required=True, type=Path)
    finalize.add_argument("--checkpoint", required=True, type=Path)
    finalize.add_argument("--receipt", required=True, type=Path)
    finalize.add_argument("--staging-root", required=True, type=Path)
    finalize.add_argument("--output", required=True, type=Path)
    finalize.add_argument("--replace", action="store_true")
    validate_final = subparsers.add_parser("validate-final")
    validate_final.add_argument("--manifest", required=True, type=Path)
    verify_final_stage = subparsers.add_parser("verify-final-stage")
    verify_final_stage.add_argument("--manifest", required=True, type=Path)
    verify_final_stage.add_argument("--staging-root", required=True, type=Path)
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
        elif args.command == "validate":
            document = load_json(args.manifest.resolve())
            summary = validate_checkpoint(document)
            summary["rawFileSha256"] = sha256_file(args.manifest.resolve())
        elif args.command == "extract":
            document, receipt = build_extraction_run(
                args.repo_root.resolve(),
                args.checkpoint.resolve(),
                args.package_root.resolve(),
                args.umodel.resolve(),
                args.converter.resolve(),
                args.staging_root,
                args.run_id,
                args.region,
                args.chunk_size,
            )
            summary = validate_extraction_receipt(document)
            summary["receipt"] = receipt.name
            summary["rawFileSha256"] = sha256_file(receipt)
        elif args.command == "validate-extraction":
            document = load_json(args.receipt.resolve())
            summary = validate_extraction_receipt(document)
            summary["rawFileSha256"] = sha256_file(args.receipt.resolve())
        elif args.command == "finalize":
            document = build_final_inventory(
                args.repo_root.resolve(),
                args.checkpoint.resolve(),
                args.receipt.resolve(),
                args.staging_root.resolve(),
            )
            summary = validate_final_inventory(document)
            write_json(args.output.resolve(), document, args.replace)
            summary["rawFileSha256"] = sha256_file(args.output.resolve())
        elif args.command == "validate-final":
            document = load_json(args.manifest.resolve())
            summary = validate_final_inventory(document)
            summary["rawFileSha256"] = sha256_file(args.manifest.resolve())
        else:
            document = load_json(args.manifest.resolve())
            summary = validate_final_inventory(document)
            staging_root = args.staging_root.resolve()
            receipt_path = staging_root / document["rawExtraction"]["receipt"]["fileName"]
            receipt = load_json(receipt_path)
            summary.update(
                {
                    f"stage{key[0].upper()}{key[1:]}": value
                    for key, value in verify_extraction_stage(
                        receipt, receipt_path, staging_root
                    ).items()
                }
            )
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
import hashlib
import importlib.util
import json
import math
import struct
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
FINAL_INVENTORY = (
    REPO_ROOT
    / "Data/Effects/Imported/RawResourceInventory/"
    "R8.raw-resource-inventory-v1.json"
)
EXPECTED_FINAL_RAW_SHA256 = (
    "578fadda10903e4935bb633947843aeefe709c55dfa4767fa2fb62ad4817e500"
)


def build_dds(
    width: int = 4,
    height: int = 4,
    four_cc: bytes = b"DXT1",
    mip_count: int = 0,
    payload: bytes = b"\0" * 8,
) -> bytes:
    header = bytearray(128)
    header[:4] = b"DDS "
    struct.pack_into("<I", header, 4, 124)
    struct.pack_into("<I", header, 8, 0x00081007)
    struct.pack_into("<I", header, 12, height)
    struct.pack_into("<I", header, 16, width)
    struct.pack_into("<I", header, 20, 8)
    struct.pack_into("<I", header, 28, mip_count)
    struct.pack_into("<I", header, 76, 32)
    struct.pack_into("<I", header, 80, 0x4)
    header[84:88] = four_cc
    struct.pack_into("<I", header, 108, 0x1000)
    return bytes(header) + payload


def build_legacy_wmodel() -> bytes:
    vertices = b"".join(
        struct.pack(
            "<3f3f2f3ff",
            *position,
            0.0,
            1.0,
            0.0,
            0.0,
            0.0,
            1.0,
            0.0,
            0.0,
            1.0,
        )
        for position in ((0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0))
    )
    indices = struct.pack("<3H", 0, 1, 2)
    center = (0.5, 0.5, 0.0)
    radius = math.sqrt(0.5)
    bounds = struct.pack(
        "<10f",
        0.0,
        0.0,
        0.0,
        1.0,
        1.0,
        0.0,
        *center,
        radius,
    )
    mesh_header = MODULE.WMODEL_MESH_HEADER.pack(
        b"WMSH", 1, 0, 15, 48, 3, 3, 2, 1, b"\0\0\0"
    )
    descriptor = MODULE.WMODEL_SUBMESH_DESC.pack(
        0, 3, 0, 3, 0, 0, b"triangle\0".ljust(20, b"\0")
    )
    mesh_content = mesh_header + descriptor + vertices + indices + bounds
    mesh = MODULE.WMODEL_FILE_HEADER.pack(
        b"WINT", 1, 0, 0, len(mesh_content)
    ) + mesh_content
    material_content = struct.pack("<4sI", b"WMA2", 1) + b"\0" * 4756
    material = MODULE.WMODEL_FILE_HEADER.pack(
        b"WINT", 1, 0, 0, len(material_content)
    ) + material_content
    section_offset = (
        MODULE.WMODEL_MODEL_HEADER.size + 2 * MODULE.WMODEL_SECTION_DESC.size
    )
    model_header = MODULE.WMODEL_MODEL_HEADER.pack(b"WMOD", 2, 0, 0, 0, 0, 0, 0)
    table = b"".join(
        (
            MODULE.WMODEL_SECTION_DESC.pack(
                1, 0, section_offset, len(mesh), b"mesh\0".ljust(40, b"\0")
            ),
            MODULE.WMODEL_SECTION_DESC.pack(
                2,
                0,
                section_offset + len(mesh),
                len(material),
                b"material\0".ljust(40, b"\0"),
            ),
        )
    )
    content = model_header + table + mesh + material
    return MODULE.WMODEL_FILE_HEADER.pack(b"WINT", 1, 0, 0, len(content)) + content


class RawResourceCheckpointTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = MODULE.load_json(CHECKPOINT)
        cls.final_document = MODULE.load_json(FINAL_INVENTORY)

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

    def test_dds_inspection_checks_exact_block_payload(self) -> None:
        inspected = MODULE.inspect_dds_bytes(build_dds())
        self.assertEqual(inspected["inspectionStatus"], "STRUCTURALLY_VALID")
        self.assertEqual(inspected["header"]["fourCC"], "DXT1")
        self.assertEqual(inspected["header"]["effectiveMipCount"], 1)
        self.assertEqual(
            inspected["layoutFacts"]["expectedPayloadByteSize"], 8
        )
        self.assertTrue(inspected["layoutFacts"]["exactPayloadConsumption"])
        truncated = MODULE.inspect_dds_bytes(build_dds(payload=b"\0" * 7))
        self.assertEqual(truncated["inspectionStatus"], "TRUNCATED_PAYLOAD")
        self.assertEqual(truncated["layoutFacts"]["truncatedByteCount"], 1)
        wrong_linear = bytearray(build_dds())
        struct.pack_into("<I", wrong_linear, 20, 7)
        self.assertEqual(
            MODULE.inspect_dds_bytes(bytes(wrong_linear))["inspectionStatus"],
            "TOP_LEVEL_LINEAR_SIZE_MISMATCH",
        )
        for offset, value, message in (
            (28, 0xFFFFFFFF, "mip count"),
            (8, 0, "header flags"),
            (108, 0, "DDSCAPS_TEXTURE"),
        ):
            mutated = bytearray(build_dds())
            struct.pack_into("<I", mutated, offset, value)
            with self.subTest(offset=offset):
                with self.assertRaisesRegex(MODULE.InventoryError, message):
                    MODULE.inspect_dds_bytes(bytes(mutated))

    def test_dds_dx10_records_raw_fields_without_colourspace_policy(self) -> None:
        base = bytearray(build_dds(four_cc=b"DX10", payload=b""))
        base[128:128] = struct.pack("<5I", 71, 3, 0, 1, 0)
        base.extend(b"\0" * 8)
        inspected = MODULE.inspect_dds_bytes(bytes(base))
        self.assertEqual(inspected["inspectionStatus"], "STRUCTURALLY_VALID")
        self.assertEqual(inspected["header"]["dx10"]["dxgiFormat"], 71)
        self.assertNotIn("colourspace", json.dumps(inspected).casefold())
        self.assertNotIn("sampler", json.dumps(inspected).casefold())
        typeless = bytearray(build_dds(four_cc=b"DX10", payload=b""))
        typeless[128:128] = struct.pack("<5I", 70, 3, 0, 1, 0)
        typeless.extend(b"\0" * 8)
        self.assertEqual(
            MODULE.inspect_dds_bytes(bytes(typeless))["inspectionStatus"],
            "STRUCTURALLY_VALID",
        )

    def test_wmodel_inspection_checks_ranges_indices_and_bounds(self) -> None:
        payload = build_legacy_wmodel()
        inspected = MODULE.inspect_wmodel_bytes(payload)
        mesh = inspected["meshInspection"]["mesh"]
        self.assertEqual(inspected["inspectionStatus"], "STRUCTURALLY_VALID")
        self.assertEqual(mesh["submeshCount"], 1)
        self.assertEqual(mesh["totalVertexCount"], 3)
        self.assertEqual(mesh["totalIndexCount"], 3)
        self.assertTrue(mesh["submeshes"][0]["embeddedBoundsConsistent"])

        mutated = bytearray(payload)
        outer_content = MODULE.WMODEL_FILE_HEADER.size
        table = outer_content + MODULE.WMODEL_MODEL_HEADER.size
        mesh_offset = MODULE.WMODEL_SECTION_DESC.unpack_from(mutated, table)[2]
        mesh_begin = outer_content + mesh_offset
        vertex_begin = (
            mesh_begin
            + MODULE.WMODEL_FILE_HEADER.size
            + MODULE.WMODEL_MESH_HEADER.size
            + MODULE.WMODEL_SUBMESH_DESC.size
        )
        index_begin = vertex_begin + 3 * MODULE.WMODEL_STATIC_VERTEX_STRIDE
        struct.pack_into("<H", mutated, index_begin, 3)
        with self.assertRaisesRegex(MODULE.InventoryError, "index"):
            MODULE.inspect_wmodel_bytes(bytes(mutated))

        bad_version = bytearray(payload)
        struct.pack_into("<H", bad_version, 6, 1)
        with self.assertRaisesRegex(MODULE.InventoryError, "outer"):
            MODULE.inspect_wmodel_bytes(bytes(bad_version))

        bad_section = bytearray(payload)
        struct.pack_into("<I", bad_section, table, 3)
        with self.assertRaisesRegex(MODULE.InventoryError, "unknown section"):
            MODULE.inspect_wmodel_bytes(bytes(bad_section))

        nonfinite = bytearray(payload)
        struct.pack_into("<f", nonfinite, vertex_begin + 12, math.nan)
        with self.assertRaisesRegex(MODULE.InventoryError, "non-finite"):
            MODULE.inspect_wmodel_bytes(bytes(nonfinite))

        bad_bounds = bytearray(payload)
        bounds_begin = index_begin + 3 * 2
        struct.pack_into("<f", bad_bounds, bounds_begin, 2.0)
        with self.assertRaisesRegex(MODULE.InventoryError, "bounds"):
            MODULE.inspect_wmodel_bytes(bytes(bad_bounds))

        bad_material = bytearray(payload)
        material_offset = MODULE.WMODEL_SECTION_DESC.unpack_from(
            bad_material, table + MODULE.WMODEL_SECTION_DESC.size
        )[2]
        material_entry = (
            outer_content + material_offset + MODULE.WMODEL_FILE_HEADER.size + 8
        )
        struct.pack_into("<I", bad_material, material_entry, 1)
        with self.assertRaisesRegex(MODULE.InventoryError, "material indices"):
            MODULE.inspect_wmodel_bytes(bytes(bad_material))

        serialized = json.dumps(inspected)
        for forbidden in (
            "colourspace",
            "sampler",
            "geometryPreScale",
            "materialPacking",
            "rendererPacket",
        ):
            self.assertNotIn(forbidden.casefold(), serialized.casefold())

    def test_fresh_stage_rejects_existing_and_repo_local_roots(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            repo = root / "repo"
            repo.mkdir()
            marker = MODULE.EXPECTED_CHECKPOINT_RAW_SHA256[:16]
            outside = root / f"R8-{marker}-unitrun"
            created = MODULE._require_fresh_external_stage(
                outside,
                repo,
                MODULE.EXPECTED_CHECKPOINT_RAW_SHA256,
                "unitrun",
            )
            self.assertEqual(created, outside.resolve())
            with self.assertRaisesRegex(MODULE.InventoryError, "already exists"):
                MODULE._require_fresh_external_stage(
                    outside,
                    repo,
                    MODULE.EXPECTED_CHECKPOINT_RAW_SHA256,
                    "unitrun",
                )
            inside = repo / f"R8-{marker}-inside"
            with self.assertRaisesRegex(MODULE.InventoryError, "outside"):
                MODULE._require_fresh_external_stage(
                    inside,
                    repo,
                    MODULE.EXPECTED_CHECKPOINT_RAW_SHA256,
                    "inside",
                )

    def test_final_inventory_has_exact_raw_payload_denominator(self) -> None:
        summary = MODULE.validate_final_inventory(self.final_document)
        self.assertEqual(summary["requestCount"], 1212)
        self.assertEqual(summary["successfulRequestCount"], 1208)
        self.assertEqual(summary["rawBlockerCount"], 4)
        self.assertEqual(summary["blockerCount"], 22)
        self.assertEqual(summary["inventoryStatus"], "FROZEN_WITH_BLOCKERS")
        self.assertEqual(
            hashlib.sha256(FINAL_INVENTORY.read_bytes()).hexdigest(),
            EXPECTED_FINAL_RAW_SHA256,
        )
        raw = self.final_document["rawExtraction"]["summary"]
        self.assertEqual(
            raw["payloadKindCounts"],
            {
                "DDS": 1026,
                "GLTF": 182,
                "GLTF_BUFFER": 182,
                "TGA": 4,
                "WMODEL": 182,
            },
        )
        self.assertEqual(raw["uniquePayloadSha256Count"], 1277)
        self.assertEqual(raw["sharedByteGroupCount"], 273)
        self.assertEqual(raw["unassociatedFreshOutputCount"], 88)
        self.assertEqual(
            self.final_document["rawBlockerEvidenceSha256"],
            "c08c2f3db4204da5d453bb9050dcfa901477faf6a3bd0684028ea76c84b0694b",
        )
        self.assertEqual(
            self.final_document["rawResourceProjectionSha256"],
            "c7d9b7d7a8fc9b82627accf34b5ce6b2f21e20eb76b6458b81ba31c67a800e4b",
        )

    def test_final_raw_blockers_are_only_preserved_tga_payloads(self) -> None:
        blockers = self.final_document["rawBlockerEvidence"]
        self.assertEqual(len(blockers), 4)
        self.assertEqual(
            {row["code"] for row in blockers},
            {"UNSUPPORTED_NON_DDS_TEXTURE_PRIMARY"},
        )
        self.assertEqual(
            {row["sourceAssetPath"] for row in blockers},
            {
                "efmaster_material_prologue.tex.flat_gray",
                "fx_tex_nomipmap_00.fx_c_flow_004",
                "efmaster_material_prologue.normal",
                "efmaster_material_prologue.flat_red",
            },
        )
        self.assertTrue(
            all(row["payloads"][0]["kind"] == "TGA" for row in blockers)
        )
        self.assertEqual(
            len(self.final_document["reports"]["unsupportedNonDdsTextures"]), 4
        )
        for key in (
            "rawExtractionMissing",
            "rawOutputAmbiguities",
            "rawStructuralFailures",
            "caseOnlyCandidateIdCollisions",
            "candidateNameCollisions",
        ):
            self.assertEqual(self.final_document["reports"][key], [])

    def test_final_coordinated_false_promotion_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.final_document)
        for corpus_key in ("fourClass", "valtan"):
            for request in mutated[corpus_key]["assetRequests"]:
                if request["rawResource"]["extractionStatus"] != (
                    "EXPORTED_AND_STRUCTURALLY_INSPECTED"
                ):
                    request["rawResource"]["extractionStatus"] = (
                        "EXPORTED_AND_STRUCTURALLY_INSPECTED"
                    )
        mutated["rawBlockerEvidence"] = []
        mutated["rawBlockerEvidenceSha256"] = MODULE.raw_blocker_evidence_sha256([])
        for key in (
            "rawResourceBlockers",
            "rawExtractionMissing",
            "rawOutputAmbiguities",
            "rawStructuralFailures",
            "unsupportedNonDdsTextures",
        ):
            mutated["reports"][key] = []
        mutated["blockerCount"] = 18
        summary = mutated["rawExtraction"]["summary"]
        summary["statusCounts"] = {
            "EXPORTED_AND_STRUCTURALLY_INSPECTED": 1212
        }
        summary["successfulRequestCount"] = 1212
        summary["rawBlockerCount"] = 0
        mutated["rawResourceProjectionSha256"] = (
            MODULE.raw_resource_projection_sha256(mutated)
        )
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(
            MODULE.InventoryError, "frozen baseline|frozen extraction"
        ):
            MODULE.validate_final_inventory(mutated)

    def test_final_embedded_checkpoint_mutations_are_rejected(self) -> None:
        mutations = (
            lambda value: value["fourClass"]["skills"][0].__setitem__(
                "inputSlot", "MUTATED"
            ),
            lambda value: value["fourClass"]["productCues"][0].__setitem__(
                "clip", "MUTATED"
            ),
            lambda value: value["valtan"]["actions"][0].__setitem__(
                "actionId", -1
            ),
            lambda value: value["checkpointProvenance"].__setitem__(
                "selfDigest", "0" * 64
            ),
        )
        for mutate in mutations:
            with self.subTest(mutate=mutate):
                mutated = copy.deepcopy(self.final_document)
                mutate(mutated)
                mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
                with self.assertRaisesRegex(
                    MODULE.InventoryError, "checkpoint provenance|embedded checkpoint"
                ):
                    MODULE.validate_final_inventory(mutated)

    def test_final_rejects_unbound_generator_identity_field(self) -> None:
        mutated = copy.deepcopy(self.final_document)
        mutated["rawExtraction"]["inventoryGenerator"] = {
            "sha256": "0" * 64,
            "byteSize": 1,
        }
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "field set"):
            MODULE.validate_final_inventory(mutated)


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
  --valtan-extraction-manifest C:\Users\user\Desktop\LostArk\.codex_tmp\data3_reextract_20260805\manifest.json `
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

### 5-4. C:/Users/user/.codex/worktrees/0109/LostArk/Data/Effects/Imported/RawResourceInventory/R8.raw-resource-inventory-v1.json

변경 종류: 추가(생성 데이터 전체 교체)
적용 위치: `build_final_inventory()` 반환값을 `write_json()`이 UTF-8, `indent=2`, terminal LF로 직렬화한 파일 전체

final inventory는 26,308,032 byte의 생성 데이터이므로 계획서에 수동 복제하지 않는다. 5-1의 생략 없는
generator와 아래 command/schema block이 extraction receipt와 final JSON 전체를 재현·검증하는 정본이다.
`extract` 명령은 target이 존재하면 의도적으로 실패하므로 아래 historical run command는 같은 이름의 stage가
없던 시점에 정확히 한 번 실행했다. 현재는 receipt/final 검증과 stage identity 검증만 반복한다.

```powershell
Set-Location C:\Users\user\.codex\worktrees\0109\LostArk
$r8Stage = 'C:\Users\user\Desktop\Resource_LostArk\05_Reports\RawResourceInventory\R8-42b437d59bf56c71-20260811T1630KST-v1'

# Historical one-shot command. $r8Stage must be previously nonexistent.
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py extract `
  --repo-root . `
  --checkpoint Data\Effects\Imported\RawResourceInventory\R8.raw-resource-denominator.checkpoint.json `
  --package-root C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages `
  --umodel C:\Users\user\Desktop\Resource_LostArk\06_Tools\UEViewerLostArk_runtime\umodel_lostark_v7.exe `
  --converter C:\Users\user\Desktop\LostArk\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  --staging-root $r8Stage `
  --run-id 20260811T1630KST-v1 `
  --region kr `
  --chunk-size 32

python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py validate-extraction `
  --receipt "$r8Stage\raw-resource-extraction-run-v1.json"
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py finalize `
  --repo-root . `
  --checkpoint Data\Effects\Imported\RawResourceInventory\R8.raw-resource-denominator.checkpoint.json `
  --receipt "$r8Stage\raw-resource-extraction-run-v1.json" `
  --staging-root $r8Stage `
  --output Data\Effects\Imported\RawResourceInventory\R8.raw-resource-inventory-v1.json `
  --replace
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py validate-final `
  --manifest Data\Effects\Imported\RawResourceInventory\R8.raw-resource-inventory-v1.json
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py verify-final-stage `
  --manifest Data\Effects\Imported\RawResourceInventory\R8.raw-resource-inventory-v1.json `
  --staging-root $r8Stage
```

external extraction receipt `raw-resource-extraction-run-v1.json`의 exact schema는 다음과 같다.

| receipt 경로 | exact key 또는 row field |
|---|---|
| root | `schema, formatVersion, mode, sourceCommit, runId, stagingLocator, checkpoint, tools, conversionBoundary, runtimeAuthority, rendererReady, admission, corpora, unassociatedFreshOutputs, summary, selfDigest` |
| `checkpoint` | `locator, fileName, byteSize, sha256` |
| `tools` | `umodel, modelAssetConverter`; tool identity는 `locator, kind, fileName, byteSize, sha256`, converter는 `executable, companions` |
| `conversionBoundary` | `classification, pretransform, conversionScale, autoTextures, runtimeBindingGenerated, gpuSchema` |
| `corpora.<Corpus>` | `packagePreflights, invocations, requests, summary` |
| `packagePreflights[]` | `corpus, logicalPackage, expectedPhysicalPackageFileName, expectedByteSize, expectedSha256, casefoldInstalledMatches, requestedForRawExport, observedByteSize, observedSha256, resolveExitCode, resolveLog, observedLoadedPhysicalPackageFileName, preflightStatus` |
| `invocations[]` | `invocationId, corpus, logicalPackage, chunkIndex, requestedObjectCount, requestIds, exitCode, options, log` |
| `requests[]` | common `requestId, corpus, role, sourceAssetPath, logicalPackage, expectedPhysicalPackageFileName, resourcesRelativeCandidateId, invocation, payloads, bindingStatus, observedPackageDirectories, observedPackageDirectoryCaseMatchesLogicalPackage, extractionStatus`; texture success는 `ddsInspection`, mesh success는 `offlineTransientCook, gltfInspection, wmodelInspection`을 추가한다. |
| `payloads[]`, `unassociatedFreshOutputs[]` | `kind, relativePath, fileName, byteSize, sha256` |
| `corpora.<Corpus>.summary` | `requestCount, statusCounts, packagePreflightCount, packagePreflightFailureCount, invocationCount, processFailureCount` |
| `summary` | `requestCount, successfulRequestCount, unassociatedFreshOutputCount` |

final `lostark.raw-resource-inventory-v1`의 checkpoint 공통 schema는 5-3과 같고, 다음 field가 정확히
추가·교체된다.

| final 경로 | exact key 또는 row field |
|---|---|
| root | checkpoint root 뒤 `checkpointProvenance, rawBlockerEvidence, rawBlockerEvidenceSha256, inventoryStatus, rawExtraction, rawResourceProjectionSha256`를 추가하며 `phase=RAW_BYTES_INSPECTED`다. |
| `checkpointProvenance` | `locator, fileName, byteSize, sha256, selfDigest, blockerEvidenceSha256` |
| `fourClass.assetRequests[]`, `valtan.assetRequests[]` | checkpoint request row의 field 전부와 `rawResource` |
| `rawResource` | common `requestId, corpus, role, sourceAssetPath, logicalPackage, expectedPhysicalPackageFileName, resourcesRelativeCandidateId, invocation, payloads, bindingStatus, observedPackageDirectories, observedPackageDirectoryCaseMatchesLogicalPackage, extractionStatus`; DDS는 `ddsInspection`, mesh는 `offlineTransientCook, gltfInspection, wmodelInspection`을 가진다. |
| `ddsInspection` | `inspectionStatus, byteSize, header, layoutFacts`; `header`는 DDS/DX10 raw header fact, `layoutFacts`는 block layout와 exact payload consumption fact다. |
| `gltfInspection` | `assetVersion, sceneCount, meshCount, bufferCount` |
| `wmodelInspection` | `inspectionStatus, byteSize, outer, model, sections, meshInspection, materialInspection, exactSectionConsumption` |
| `rawExtraction` | `runId, stagingLocator, receipt, tools, conversionBoundary, stageVerification, corpora, unassociatedFreshOutputs, summary` |
| `rawExtraction.receipt` | `locator, fileName, byteSize, sha256, selfDigest` |
| `rawExtraction.stageVerification` | `verifiedArtifactCount, verifiedArtifactByteSize, receiptRawSha256` |
| `rawExtraction.corpora.<Corpus>` | `packagePreflights, invocations, summary` |
| `rawExtraction.summary` | `requestCount, statusCounts, successfulRequestCount, rawBlockerCount, payloadCount, payloadKindCounts, payloadByteSizeWithAliases, uniquePayloadSha256Count, sharedByteGroupCount, unassociatedFreshOutputCount` |
| `reports` 추가 field | `rawResourceBlockers, rawStructuralFailures, unsupportedNonDdsTextures, caseOnlyCandidateIdCollisions, candidateNameCollisions, basenameCollisions`; `sharedByteGroups`도 final raw payload alias projection으로 교체한다. |
| `rawBlockerEvidence[]` | `code, corpus, requestId, role, sourceAssetPath, resourcesRelativeCandidateId, payloads` |

동결 run은 1,212 request 중 1,208 structural success다. alias payload는 DDS 1,026, glTF 182,
glTF buffer 182, WModel 182, TGA 4로 1,576개이며 1,277 unique SHA와 273 shared-byte group을 가진다.
receipt raw SHA-256은 `cf58ad0b26b7f24e0d0cd91e713f14bf96a3c9c14d4284a60415512977980137`,
raw-resource projection SHA-256은 `c7d9b7d7a8fc9b82627accf34b5ce6b2f21e20eb76b6458b81ba31c67a800e4b`,
raw blocker projection SHA-256은 `c08c2f3db4204da5d453bb9050dcfa901477faf6a3bd0684028ea76c84b0694b`다.
final raw file SHA-256은 `578fadda10903e4935bb633947843aeefe709c55dfa4767fa2fb62ad4817e500`,
canonical self digest는 `d6ec9db6dc7542b45eaecd9afb31e55aa817c25382b86362dbdb532843f14a82`이며
inventory status는 corpus/provenance 18개와 non-DDS TGA 4개를 합친 `FROZEN_WITH_BLOCKERS` 22개다.


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
- 동결 receipt의 1,212 request를 모두 유지하고 DDS 1,026 / glTF 182 / buffer 182 / WModel 182 /
  TGA 4 payload identity를 보존한다. 네 TGA는 DDS로 재인코딩하지 않고 raw blocker다.
- checkpoint blocker 18개와 raw blocker 4개를 합친 final `blockerCount`는 22다.
- checkpoint와 final inventory는 deterministic JSON 직렬화 raw SHA와 canonical self digest를 각각 가진다.

## 적용 순서와 검증

첫 commit은 denominator checkpoint와 checkpoint/validate 경로를 봉인했다. 둘째 commit은 같은 checkpoint
SHA를 이름에 포함한 fresh stage의 extraction receipt, raw 검사/finalize/validator, focused test, final inventory와
RESULT를 한 검증 단위로 묶는다. 기존 stage에 `extract`를 재실행하지 않고 다음 read-only 검증을 반복한다.

```powershell
Set-Location C:\Users\user\.codex\worktrees\0109\LostArk
$r8Stage = 'C:\Users\user\Desktop\Resource_LostArk\05_Reports\RawResourceInventory\R8-42b437d59bf56c71-20260811T1630KST-v1'
python -m py_compile Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py Tools/LevelPlacementExtractor/test_prepare_r8_raw_resource_inventory.py
python -m unittest Tools.LevelPlacementExtractor.test_prepare_r8_raw_resource_inventory -v
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py validate `
  --manifest Data\Effects\Imported\RawResourceInventory\R8.raw-resource-denominator.checkpoint.json
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py validate-extraction `
  --receipt "$r8Stage\raw-resource-extraction-run-v1.json"
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py validate-final `
  --manifest Data\Effects\Imported\RawResourceInventory\R8.raw-resource-inventory-v1.json
python Tools/LevelPlacementExtractor/prepare_r8_raw_resource_inventory.py verify-final-stage `
  --manifest Data\Effects\Imported\RawResourceInventory\R8.raw-resource-inventory-v1.json `
  --staging-root $r8Stage
git diff --check
```

성공 기준은 focused test 20/20, checkpoint/receipt/final validator PASS, external stage 1,961 artifact identity
PASS, final raw SHA `578fadda10903e4935bb633947843aeefe709c55dfa4767fa2fb62ad4817e500` 재현이다.
ProjectAudit 결과는 repository baseline failure와 이번 변경의 regression을 RESULT에서 분리한다. 최종 보고는
frozen raw manifest SHA를 M0/GPU 결과와 분리하며 GPU schema 동결 전 final binding은 생성하지 않는다.
