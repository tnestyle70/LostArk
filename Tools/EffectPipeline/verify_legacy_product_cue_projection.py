#!/usr/bin/env python3

"""Verify the frozen legacy Product cue and non-target Catalog projection.

This command is deliberately read-only.  The baseline is an exact projection of
the implementation parent and is never updated from the current worktree.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path, PurePosixPath
import re
import subprocess
import sys
from typing import Any, Protocol


FROZEN_SOURCE_COMMIT = "18d2b48920b2a327ac59b572960325d352e77a6f"
FROZEN_BASELINE_FILE_SHA256 = (
    "261d13bbafdb51eca8e94b13862d0348112fa328f6c3ebf6c8e15dcce5a95f77"
)
PROJECTION_ID = "legacy-product-cue-projection-v1"
TARGET_EFFECT_ASSET_ID = "effect.artist.skill.31470"
PREPARED_MODE = "LEGACY_CATALOG_PREPARED_DOCUMENT"
EXPECTED_COUNTS = {
    "legacyProductCues": 101,
    "nonTargetComponents": 555,
    "nonTargetEffects": 101,
}

# The frozen legacy projection deliberately ignores only these two reviewed
# composition-policy deltas.  They are parsed and exact-checked before the new
# field is removed for legacy equality; no other v6 action-facing cue is hidden.
ACTION_FACING_CUE_DELTAS = (
    {
        "animationAssetId": "Warlord",
        "clipName": "wgl_sk_firebullet",
        "effectAssetId": "effect.warlord.skill.17060.unified",
        "previousFollowPolicy": "follow",
        "currentFollowPolicy": "follow",
    },
    {
        "animationAssetId": "DimensionMaster",
        "clipName": "pc_sp_m_00_sk_sk_willowrend",
        "effectAssetId": "effect.dimensionmaster.skill.2050210.unified",
        "previousFollowPolicy": "snapshot",
        "currentFollowPolicy": "follow",
    },
)

BASELINE_PATH = PurePosixPath(
    "Data/Effects/Baselines/legacy-product-cue-projection-v1.json"
)
RUNTIME_CATALOG_PATH = PurePosixPath(
    "Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json"
)
AUTHORING_CATALOG_PATH = PurePosixPath("Data/Effects/EffectCatalog.json")
ANIMATION_DOCUMENTS = (
    ("Artist", PurePosixPath("Data/Animation/Authored/Artist/Artist.animevents")),
    (
        "DimensionMaster",
        PurePosixPath(
            "Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents"
        ),
    ),
    (
        "LanceMaster",
        PurePosixPath(
            "Data/Animation/Authored/LanceMaster/LanceMaster.animevents"
        ),
    ),
    (
        "Warlord",
        PurePosixPath("Data/Animation/Authored/Warlord/Warlord.animevents"),
    ),
)
FROZEN_SOURCE_PATHS = (
    RUNTIME_CATALOG_PATH,
    AUTHORING_CATALOG_PATH,
    *(path for _, path in ANIMATION_DOCUMENTS),
)


class ProjectionError(RuntimeError):
    pass


class SourceReader(Protocol):
    def read_bytes(self, path: PurePosixPath) -> bytes: ...


class GitCommitReader:
    def __init__(self, repository_root: Path, commit: str) -> None:
        self.repository_root = repository_root
        self.commit = commit

    def read_bytes(self, path: PurePosixPath) -> bytes:
        result = subprocess.run(
            ["git", "show", f"{self.commit}:{path.as_posix()}"],
            cwd=self.repository_root,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        if result.returncode != 0:
            detail = result.stderr.decode("utf-8", errors="replace").strip()
            raise ProjectionError(
                f"Cannot read frozen Git blob {self.commit}:{path}: {detail}"
            )
        return result.stdout


class WorktreeReader:
    def __init__(self, repository_root: Path) -> None:
        self.repository_root = repository_root

    def read_bytes(self, path: PurePosixPath) -> bytes:
        absolute = self.repository_root.joinpath(*path.parts)
        try:
            return absolute.read_bytes()
        except OSError as exc:
            raise ProjectionError(f"Cannot read current source {path}: {exc}") from exc


def _reject_constant(value: str) -> None:
    raise ProjectionError(f"JSON contains a non-finite number: {value}")


def _reject_duplicate_pairs(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ProjectionError(f"JSON contains a duplicate key: {key}")
        result[key] = value
    return result


def _load_json_bytes(payload: bytes, path: PurePosixPath, *, require_lf: bool) -> Any:
    if payload.startswith(b"\xef\xbb\xbf"):
        raise ProjectionError(f"UTF-8 BOM is forbidden: {path}")
    if require_lf and b"\r" in payload:
        raise ProjectionError(f"Exact runtime Catalog must use LF bytes: {path}")
    try:
        text = payload.decode("utf-8", errors="strict")
    except UnicodeDecodeError as exc:
        raise ProjectionError(f"Source is not strict UTF-8: {path}: {exc}") from exc
    try:
        return json.loads(
            text,
            object_pairs_hook=_reject_duplicate_pairs,
            parse_constant=_reject_constant,
        )
    except json.JSONDecodeError as exc:
        raise ProjectionError(f"Invalid JSON {path}: {exc}") from exc


def _canonical_bytes(value: Any) -> bytes:
    try:
        return json.dumps(
            value,
            ensure_ascii=False,
            sort_keys=True,
            separators=(",", ":"),
            allow_nan=False,
        ).encode("utf-8")
    except (TypeError, ValueError) as exc:
        raise ProjectionError(f"Value cannot be canonicalized: {exc}") from exc


def _sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _canonical_sha256(value: Any) -> str:
    return _sha256_bytes(_canonical_bytes(value))


def _git_blob_id(payload: bytes) -> str:
    header = f"blob {len(payload)}\0".encode("ascii")
    return hashlib.sha1(header + payload).hexdigest()


def _tokenize(line: str) -> list[str]:
    tokens: list[str] = []
    cursor = 0
    while cursor < len(line):
        while cursor < len(line) and line[cursor].isspace():
            cursor += 1
        if cursor == len(line):
            break
        token: list[str] = []
        in_quote = False
        while cursor < len(line):
            character = line[cursor]
            cursor += 1
            if character == '"':
                in_quote = not in_quote
                continue
            if character == "\\" and in_quote and cursor < len(line):
                token.append(line[cursor])
                cursor += 1
                continue
            if not in_quote and character.isspace():
                break
            token.append(character)
        if in_quote:
            raise ProjectionError("Unterminated quote in animevents row")
        tokens.append("".join(token))
    return tokens


def _parse_uint(text: str, field: str) -> int:
    if re.fullmatch(r"[0-9]+", text) is None:
        raise ProjectionError(f"Invalid unsigned integer for {field}: {text!r}")
    value = int(text)
    if value > 0xFFFFFFFF:
        raise ProjectionError(f"Unsigned integer exceeds uint32 for {field}: {text}")
    return value


def _parse_float(text: str, field: str) -> float:
    try:
        value = float(text)
    except ValueError as exc:
        raise ProjectionError(f"Invalid float for {field}: {text!r}") from exc
    if not math.isfinite(value):
        raise ProjectionError(f"Non-finite float for {field}: {text!r}")
    return value


def _make_fields(tokens: list[str]) -> dict[str, str]:
    fields: dict[str, str] = {}
    for token in tokens:
        if "=" not in token or token.startswith("="):
            raise ProjectionError("Animation EFFECT row has an invalid field")
        key, value = token.split("=", 1)
        if key in fields:
            raise ProjectionError(f"Animation EFFECT row duplicates field: {key}")
        fields[key] = value
    return fields


def _parse_product_cues(
    payload: bytes,
    path: PurePosixPath,
    animation_asset_id: str,
) -> list[dict[str, Any]]:
    if payload.startswith(b"\xef\xbb\xbf"):
        raise ProjectionError(f"UTF-8 BOM is forbidden: {path}")
    try:
        text = payload.decode("utf-8", errors="strict")
    except UnicodeDecodeError as exc:
        raise ProjectionError(f"animevents is not strict UTF-8: {path}: {exc}") from exc
    lines = text.splitlines()
    if not lines:
        raise ProjectionError(f"Animation event document is empty: {path}")
    header = _tokenize(lines[0])
    if len(header) != 4 or header[0] != "LOSTARK_ANIM_EVENTS":
        raise ProjectionError(f"Invalid animation event header: {path}")
    version = _parse_uint(header[1], "animevents version")
    declared_rows = _parse_uint(header[3], "animevents row count")
    if version < 3 or version > 6 or header[2] != animation_asset_id:
        raise ProjectionError(f"Animation event owner/version is invalid: {path}")

    rows = [line for line in lines[1:] if line]
    if len(rows) != declared_rows:
        raise ProjectionError(
            f"Animation event row count mismatch for {path}: "
            f"declared={declared_rows} actual={len(rows)}"
        )

    cues: list[dict[str, Any]] = []
    admission_keys: set[tuple[str, int, str, str]] = set()
    for line in rows:
        tokens = _tokenize(line)
        if len(tokens) < 3:
            raise ProjectionError(f"Invalid animation event row: {path}")
        if tokens[1] != "EFFECT":
            continue
        fields = _make_fields(tokens[2:])
        payload_id = fields.get("payload", "")
        if not payload_id or fields.get("effectref") != "asset":
            continue
        if "startms" not in fields:
            raise ProjectionError(f"Product Effect cue is missing startms: {path}")
        start_ms = _parse_uint(fields["startms"], "startms")
        end_ms = _parse_uint(fields.get("endms", fields["startms"]), "endms")
        anchor = fields.get("anchor", "root")
        follow = fields.get("follow", "follow")
        stop = fields.get("stop", "natural")
        if not anchor or follow not in {"follow", "snapshot"}:
            raise ProjectionError(f"Product Effect cue has invalid anchor/follow: {path}")
        if "orientation" in fields and version < 6:
            raise ProjectionError(
                f"Product Effect cue orientation requires version 6: {path}"
            )
        orientation = fields.get("orientation", "anchor")
        if orientation not in {"anchor", "action_facing"}:
            raise ProjectionError(
                f"Product Effect cue has invalid orientation policy: {path}"
            )
        if orientation == "action_facing" and anchor != "root":
            raise ProjectionError(
                f"Product Effect cue action_facing requires root: {path}"
            )
        if stop not in {"natural", "cue_end"}:
            raise ProjectionError(f"Product Effect cue has invalid stop policy: {path}")
        if end_ms < start_ms or (stop == "cue_end" and end_ms <= start_ms):
            raise ProjectionError(f"Product Effect cue has an invalid interval: {path}")
        transform_fields = (
            ("px", 0.0), ("py", 0.0), ("pz", 0.0),
            ("rx", 0.0), ("ry", 0.0), ("rz", 0.0),
            ("sx", 1.0), ("sy", 1.0), ("sz", 1.0),
        )
        transform = {
            key: _parse_float(fields.get(key, str(default)), key)
            for key, default in transform_fields
        }
        if transform["sx"] <= 0 or transform["sy"] <= 0 or transform["sz"] <= 0:
            raise ProjectionError(f"Product Effect cue has a non-positive scale: {path}")
        admission_key = (tokens[0], start_ms, payload_id, anchor)
        if admission_key in admission_keys:
            raise ProjectionError(f"Duplicate admitted Product Effect cue: {path}")
        admission_keys.add(admission_key)
        identity = {
            "anchorSlotId": anchor,
            "animationAssetId": animation_asset_id,
            "clipName": tokens[0],
            "effectAssetId": payload_id,
            "startMs": start_ms,
        }
        cue = {
                **identity,
                "cueId": f"legacy-product-cue-{_canonical_sha256(identity)}",
                "endMs": end_ms,
                "followPolicy": follow,
                "localTransform": {
                    "position": [transform["px"], transform["py"], transform["pz"]],
                    "rotationDegrees": [
                        transform["rx"], transform["ry"], transform["rz"]
                    ],
                    "scale": [transform["sx"], transform["sy"], transform["sz"]],
                },
                "stopPolicy": stop,
            }
        if orientation == "action_facing":
            cue["orientationPolicy"] = orientation
        cues.append(cue)
    return cues


def _require_object(value: Any, label: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ProjectionError(f"{label} must be an object")
    return value


def _require_list(value: Any, label: str) -> list[Any]:
    if not isinstance(value, list):
        raise ProjectionError(f"{label} must be an array")
    return value


def _index_unique(rows: list[Any], key: str, label: str) -> dict[str, dict[str, Any]]:
    result: dict[str, dict[str, Any]] = {}
    for raw_row in rows:
        row = _require_object(raw_row, f"{label} row")
        identity = row.get(key)
        if not isinstance(identity, str) or not identity:
            raise ProjectionError(f"{label} row has an invalid {key}")
        if identity in result:
            raise ProjectionError(f"{label} contains duplicate {key}: {identity}")
        result[identity] = row
    return result


def _normalize_allowed_action_facing_cues(
    cues: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    normalized = copy.deepcopy(cues)
    allowed = {
        (
            delta["animationAssetId"],
            delta["clipName"],
            delta["effectAssetId"],
        ): delta
        for delta in ACTION_FACING_CUE_DELTAS
    }
    consumed: set[tuple[str, str, str]] = set()
    for cue in normalized:
        if "orientationPolicy" not in cue:
            continue
        identity = (
            cue.get("animationAssetId"),
            cue.get("clipName"),
            cue.get("effectAssetId"),
        )
        delta = allowed.get(identity)
        if delta is None or identity in consumed:
            raise ProjectionError(
                f"Unexpected v6 action-facing Product cue delta: {identity}"
            )
        if (
            cue.get("anchorSlotId") != "root"
            or cue.get("orientationPolicy") != "action_facing"
            or cue.get("followPolicy") != delta["currentFollowPolicy"]
        ):
            raise ProjectionError(
                f"Action-facing Product cue delta fields are invalid: {identity}"
            )
        consumed.add(identity)
        del cue["orientationPolicy"]
        cue["followPolicy"] = delta["previousFollowPolicy"]
    if consumed != set(allowed):
        missing = sorted(set(allowed) - consumed)
        raise ProjectionError(
            f"Reviewed action-facing Product cue delta is missing: {missing}"
        )
    return normalized


def _build_projection(
    reader: SourceReader,
    normalize_action_facing: bool = False,
) -> dict[str, Any]:
    runtime = _require_object(
        _load_json_bytes(
            reader.read_bytes(RUNTIME_CATALOG_PATH),
            RUNTIME_CATALOG_PATH,
            require_lf=True,
        ),
        "runtime Catalog",
    )
    if set(runtime) != {"schema", "formatVersion", "components", "effects"}:
        raise ProjectionError("Runtime Catalog envelope has unexpected fields")
    if runtime["schema"] != "lostark.effect-runtime-catalog" or runtime["formatVersion"] != 3:
        raise ProjectionError("Runtime Catalog must be exact format 3")
    effects = _index_unique(
        _require_list(runtime["effects"], "runtime effects"),
        "effectAssetId",
        "runtime effects",
    )
    components = _index_unique(
        _require_list(runtime["components"], "runtime components"),
        "componentAssetId",
        "runtime components",
    )
    if TARGET_EFFECT_ASSET_ID not in effects:
        raise ProjectionError("Target Artist F Catalog entry is missing")

    authoring = _require_object(
        _load_json_bytes(
            reader.read_bytes(AUTHORING_CATALOG_PATH),
            AUTHORING_CATALOG_PATH,
            require_lf=False,
        ),
        "authoring Catalog",
    )
    if authoring.get("formatVersion") != 1:
        raise ProjectionError("Authoring Catalog must be format 1")
    authoring_effects = _index_unique(
        _require_list(authoring.get("effects"), "authoring effects"),
        "effectAssetId",
        "authoring effects",
    )

    parsed_cues: list[dict[str, Any]] = []
    for animation_asset_id, path in ANIMATION_DOCUMENTS:
        parsed_cues.extend(
            _parse_product_cues(reader.read_bytes(path), path, animation_asset_id)
        )
    if normalize_action_facing:
        parsed_cues = _normalize_allowed_action_facing_cues(parsed_cues)
    parsed_cues = [
        cue for cue in parsed_cues if cue["effectAssetId"] != TARGET_EFFECT_ASSET_ID
    ]
    cue_ids = [cue["cueId"] for cue in parsed_cues]
    if len(cue_ids) != len(set(cue_ids)):
        raise ProjectionError("Legacy Product cue IDs are not globally unique")

    legacy_product_cues: list[dict[str, Any]] = []
    for cue in parsed_cues:
        effect_id = cue["effectAssetId"]
        runtime_row = effects.get(effect_id)
        authoring_row = authoring_effects.get(effect_id)
        if runtime_row is None or authoring_row is None:
            raise ProjectionError(f"Legacy Product cue has no Catalog row: {effect_id}")
        if runtime_row.get("payloadKind") != "LEGACY_ASSEMBLY_V1":
            raise ProjectionError(f"Legacy Product cue is not legacy assembly: {effect_id}")
        authoring_path = authoring_row.get("authoringPath")
        if not isinstance(authoring_path, str) or not authoring_path:
            raise ProjectionError(f"Legacy Product cue has no authoringPath: {effect_id}")
        assembly = _require_object(runtime_row.get("assembly"), f"assembly {effect_id}")
        required_identity = (
            assembly.get("schema") == "lostark.effect-assembly"
            and assembly.get("version") == 1
            and assembly.get("effectAssetId") == effect_id
            and assembly.get("sourceAuthoringVersion")
            == runtime_row.get("authoringFormatVersion")
            and assembly.get("sourceDocumentFileSha256")
            == runtime_row.get("contentSha256")
        )
        if not required_identity:
            raise ProjectionError(f"Authored/Assembly identity mismatch: {effect_id}")
        legacy_product_cues.append(
            {
                **cue,
                "assemblyIdentity": {
                    "canonicalSha256": _canonical_sha256(assembly),
                    "effectAssetId": assembly["effectAssetId"],
                    "schema": assembly["schema"],
                    "sourceAuthoringVersion": assembly["sourceAuthoringVersion"],
                    "sourceDocumentFileSha256": assembly["sourceDocumentFileSha256"],
                    "sourceDocumentSha256": assembly.get("sourceDocumentSha256"),
                    "version": assembly["version"],
                },
                "authoredIdentity": {
                    "authoringFormatVersion": runtime_row["authoringFormatVersion"],
                    "authoringPath": authoring_path,
                    "contentSha256": runtime_row["contentSha256"],
                },
                "catalogEntrySha256": _canonical_sha256(runtime_row),
                "payloadKind": runtime_row["payloadKind"],
                "preparedMode": PREPARED_MODE,
            }
        )

    legacy_product_cues.sort(
        key=lambda row: (
            row["animationAssetId"], row["clipName"], row["startMs"],
            row["effectAssetId"], row["anchorSlotId"],
        )
    )
    non_target_effects = [
        {
            "catalogEntrySha256": _canonical_sha256(row),
            "effectAssetId": effect_id,
            "payloadKind": row.get("payloadKind", "LEGACY_FORMAT2_DOCUMENT"),
        }
        for effect_id, row in effects.items()
        if effect_id != TARGET_EFFECT_ASSET_ID
    ]
    non_target_effects.sort(key=lambda row: row["effectAssetId"])
    non_target_components: list[dict[str, Any]] = []
    for component_id, row in components.items():
        source = _require_object(row.get("source"), f"component source {component_id}")
        source_effect_id = source.get("effectAssetId")
        if not isinstance(source_effect_id, str) or not source_effect_id:
            raise ProjectionError(f"Component source effect ID is invalid: {component_id}")
        if source_effect_id == TARGET_EFFECT_ASSET_ID:
            continue
        non_target_components.append(
            {
                "catalogEntrySha256": _canonical_sha256(row),
                "componentAssetId": component_id,
                "sourceEffectAssetId": source_effect_id,
            }
        )
    non_target_components.sort(key=lambda row: row["componentAssetId"])

    cue_effect_ids = {row["effectAssetId"] for row in legacy_product_cues}
    non_target_effect_ids = {row["effectAssetId"] for row in non_target_effects}
    if cue_effect_ids != non_target_effect_ids:
        raise ProjectionError(
            "Legacy Product cue IDs and non-target effect Catalog IDs are not exact-equal"
        )
    counts = {
        "legacyProductCues": len(legacy_product_cues),
        "nonTargetComponents": len(non_target_components),
        "nonTargetEffects": len(non_target_effects),
    }
    if counts != EXPECTED_COUNTS:
        raise ProjectionError(f"Projection denominator mismatch: {counts}")
    return {
        "catalogEnvelope": {
            "formatVersion": runtime["formatVersion"],
            "schema": runtime["schema"],
        },
        "legacyProductCues": legacy_product_cues,
        "nonTargetComponents": non_target_components,
        "nonTargetEffects": non_target_effects,
    }


def _git_tree_id(repository_root: Path, commit: str) -> str:
    result = subprocess.run(
        ["git", "rev-parse", f"{commit}^{{tree}}"],
        cwd=repository_root,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        raise ProjectionError(
            f"Cannot resolve frozen source tree: {result.stderr.strip()}"
        )
    return result.stdout.strip()


def _source_manifest(repository_root: Path, reader: GitCommitReader) -> dict[str, Any]:
    files: list[dict[str, Any]] = []
    for path in FROZEN_SOURCE_PATHS:
        payload = reader.read_bytes(path)
        files.append(
            {
                "byteCount": len(payload),
                "gitBlobId": _git_blob_id(payload),
                "path": path.as_posix(),
                "rawSha256": _sha256_bytes(payload),
            }
        )
    files.sort(key=lambda row: row["path"])
    return {
        "commitSha": FROZEN_SOURCE_COMMIT,
        "files": files,
        "treeId": _git_tree_id(repository_root, FROZEN_SOURCE_COMMIT),
    }


def _expected_frozen_document(repository_root: Path) -> dict[str, Any]:
    reader = GitCommitReader(repository_root, FROZEN_SOURCE_COMMIT)
    projection = _build_projection(reader)
    return {
        "expectedCounts": EXPECTED_COUNTS,
        "formatVersion": 1,
        "frozenSource": _source_manifest(repository_root, reader),
        "projection": projection,
        "projectionId": PROJECTION_ID,
        "projectionSha256": _canonical_sha256(projection),
        "schema": "lostark.legacy-product-cue-projection",
        "targetExclusion": {
            "effectAssetId": TARGET_EFFECT_ASSET_ID,
            "scope": "TARGET_ONLY_MUTATION_ALLOWED",
        },
    }


def _first_difference(expected: Any, actual: Any, path: str = "$.") -> str | None:
    if type(expected) is not type(actual):
        return f"{path} type expected={type(expected).__name__} actual={type(actual).__name__}"
    if isinstance(expected, dict):
        if set(expected) != set(actual):
            missing = sorted(set(expected) - set(actual))
            extra = sorted(set(actual) - set(expected))
            return f"{path} keys missing={missing} extra={extra}"
        for key in sorted(expected):
            difference = _first_difference(expected[key], actual[key], f"{path}{key}.")
            if difference is not None:
                return difference
        return None
    if isinstance(expected, list):
        if len(expected) != len(actual):
            return f"{path} length expected={len(expected)} actual={len(actual)}"
        for index, (expected_item, actual_item) in enumerate(zip(expected, actual)):
            difference = _first_difference(
                expected_item, actual_item, f"{path}[{index}]."
            )
            if difference is not None:
                return difference
        return None
    if expected != actual:
        return f"{path} expected={expected!r} actual={actual!r}"
    return None


def _assert_equal_projection(expected: Any, actual: Any, label: str) -> None:
    difference = _first_difference(expected, actual)
    if difference is not None:
        raise ProjectionError(f"{label} delta is not zero: {difference}")


def verify(repository_root: Path) -> dict[str, Any]:
    baseline_path = repository_root.joinpath(*BASELINE_PATH.parts)
    try:
        baseline_bytes = baseline_path.read_bytes()
    except OSError as exc:
        raise ProjectionError(f"Cannot read frozen baseline {BASELINE_PATH}: {exc}") from exc
    if baseline_bytes.startswith(b"\xef\xbb\xbf") or b"\r" in baseline_bytes:
        raise ProjectionError("Frozen baseline must be UTF-8 without BOM and LF-only")
    if not baseline_bytes.endswith(b"\n") or baseline_bytes.endswith(b"\n\n"):
        raise ProjectionError("Frozen baseline must have exactly one trailing LF")
    actual_file_sha = _sha256_bytes(baseline_bytes)
    if actual_file_sha != FROZEN_BASELINE_FILE_SHA256:
        raise ProjectionError(
            "Frozen baseline file SHA mismatch: "
            f"expected={FROZEN_BASELINE_FILE_SHA256} actual={actual_file_sha}"
        )
    baseline = _require_object(
        _load_json_bytes(baseline_bytes, BASELINE_PATH, require_lf=True),
        "frozen baseline",
    )
    expected_frozen = _expected_frozen_document(repository_root)
    _assert_equal_projection(expected_frozen, baseline, "Frozen 18d2 baseline")
    current = _build_projection(
        WorktreeReader(repository_root), normalize_action_facing=True
    )
    _assert_equal_projection(baseline["projection"], current, "Current non-target")
    return baseline


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
    )
    args = parser.parse_args(argv)
    try:
        baseline = verify(args.repository_root.resolve())
    except ProjectionError as exc:
        print(f"Legacy Product cue projection FAIL: {exc}", file=sys.stderr)
        return 1
    counts = baseline["expectedCounts"]
    print(
        "Legacy Product cue projection PASS: "
        f"source={FROZEN_SOURCE_COMMIT} "
        f"cues={counts['legacyProductCues']}/101 "
        f"effects={counts['nonTargetEffects']}/101 "
        f"components={counts['nonTargetComponents']}/555 "
        "old101Delta=0 nonTargetDelta=0"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
