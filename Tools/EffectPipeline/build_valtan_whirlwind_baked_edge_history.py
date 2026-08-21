#!/usr/bin/env python3
"""Materialize Valtan 420633's baked AnimationTrail edge history.

The source EFData points at one AnimNotify_Trails child export that owns a
409-row time/first/control/second edge stream.  This builder preserves that
immutable source geometry once; the three Whirlwind Trail material carriers
reference the resulting history by stable ID.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import tempfile
from pathlib import Path
from typing import Any


SCRIPT_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent
LEVEL_EXTRACTOR_ROOT = REPOSITORY_ROOT / "Tools/LevelPlacementExtractor"

import sys

if str(LEVEL_EXTRACTOR_ROOT) not in sys.path:
    sys.path.insert(0, str(LEVEL_EXTRACTOR_ROOT))

import extract_ue3_placements as ue3  # noqa: E402


SCHEMA = "lostark.valtan-baked-edge-history"
FORMAT_VERSION = 1
HISTORY_ID = "valtan.420633.animnotify-trails-479.baked-edges"
DEFAULT_OUTPUT = Path(
    "Data/Effects/Imported/Valtan/"
    "Valtan.420633.whirlwind-baked-edge-history.v1.json"
)

SOURCE_FILE_NAME = "IPAVH3D5AK88AK6PQIPWVQ5UAKVH6QBO.upk"
SOURCE_BYTE_COUNT = 153_586
SOURCE_SHA256 = "09c7968667dbedb43feb3bed18a1985a470f8a8848c468d5baf6bf0980314e91"

OUTER_CLASS = "efdata_animnotify_trails"
OUTER_OBJECT = "mn_rpbf_00_420621_0_3_0"
OUTER_EXPORT_INDEX = 77
OUTER_EXPORT_REF = 78
OUTER_SERIAL_OFFSET = 556_532
OUTER_SERIAL_SIZE = 40
OUTER_SERIAL_SHA256 = (
    "1d20a4726a1228aa0b89aa94739e776622c39792dfaa2273c0e733278812e7d2"
)

HISTORY_CLASS = "animnotify_trails"
HISTORY_OBJECT = "animnotify_trails_479"
HISTORY_OBJECT_PATH = (
    "mn_rpbf_00_420621_0_3_0.animnotify_trails_479"
)
HISTORY_EXPORT_INDEX = 35
HISTORY_EXPORT_REF = 36
HISTORY_OUTER_REF = OUTER_EXPORT_REF
HISTORY_SERIAL_OFFSET = 422_412
HISTORY_SERIAL_SIZE = 68_808
HISTORY_SERIAL_SHA256 = (
    "195754a1ae68a1ba026b31c3617c145ea6a0511cb237ac6653e41e45821ea81b"
)

PARTICLE_SYSTEM_REF = -8
PARTICLE_SYSTEM_OBJECT_PATH = "FX_BS_01.Trail.Par_N_MRHG_Trail_01"
SOURCE_END_TIME_SECONDS = 3.200000047683716
PLAYBACK_CLAMP_SECONDS = 1.2000000476837158
EXPECTED_SAMPLE_COUNT = 409
EXPECTED_FIRST_TIME_SECONDS = 0.0
EXPECTED_LAST_TIME_SECONDS = 3.199997663497925
COORDINATE_BASIS = "UE3_CM_X_Z_NEG_Y_TO_RUNTIME_METERS"


class ContractError(RuntimeError):
    """Raised when source identity or normalized history is not exact."""


def _default_source_path() -> Path:
    program_data = os.environ.get("ProgramData")
    if not program_data:
        raise ContractError("ProgramData is unavailable; pass --source-package")
    return (
        Path(program_data)
        / "Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages"
        / SOURCE_FILE_NAME
    )


def _sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def _canonical_bytes(value: Any) -> bytes:
    return json.dumps(
        value,
        ensure_ascii=False,
        sort_keys=True,
        separators=(",", ":"),
        allow_nan=False,
    ).encode("utf-8")


def _pretty_bytes(value: Any) -> bytes:
    # This pre-v2 Whirlwind canary is a frozen physical-byte fixture.  Its
    # published identity predates the repository-wide LF policy and is
    # intentionally CRLF; regenerating it must preserve that exact boundary.
    text = json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n"
    return text.replace("\n", "\r\n").encode("utf-8")


def _property(properties: dict[str, Any], name: str) -> Any:
    wanted = name.casefold()
    matches = [
        item.get("value")
        for key, item in properties.items()
        if key.casefold() == wanted and isinstance(item, dict)
    ]
    if len(matches) != 1:
        raise ContractError(f"expected one property {name}, got {len(matches)}")
    return matches[0]


def _struct_value(row: dict[str, Any], name: str) -> Any:
    wanted = name.casefold()
    matches = [
        item.get("value")
        for key, item in row.items()
        if key.casefold() == wanted and isinstance(item, dict)
    ]
    if len(matches) != 1:
        raise ContractError(
            f"baked edge sample expected one {name}, got {len(matches)}"
        )
    return matches[0]


def _finite_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ContractError(f"{label} is not numeric")
    result = float(value)
    if not math.isfinite(result):
        raise ContractError(f"{label} is not finite")
    return result


def _vector(value: Any, label: str) -> list[float]:
    if not isinstance(value, dict):
        raise ContractError(f"{label} is not a UE3 vector")
    by_name = {str(key).casefold(): item for key, item in value.items()}
    if set(by_name) != {"x", "y", "z"}:
        raise ContractError(f"{label} vector lanes changed")
    return [
        _finite_number(by_name[axis], f"{label}.{axis}")
        for axis in ("x", "y", "z")
    ]


def _one_export(
    exports: list[ue3.ExportEntry],
    imports: list[ue3.ImportEntry],
    *,
    class_name: str,
    object_name: str,
    outer_ref: int | None = None,
) -> ue3.ExportEntry:
    matches = [
        entry
        for entry in exports
        if entry.object_name.casefold() == object_name
        and ue3.package_ref_name(entry.class_index, imports, exports).casefold()
        == class_name
        and (outer_ref is None or entry.package_index == outer_ref)
    ]
    if len(matches) != 1:
        raise ContractError(
            f"expected one {class_name}/{object_name} export, got {len(matches)}"
        )
    return matches[0]


def _validate_export(
    entry: ue3.ExportEntry,
    serial: bytes,
    *,
    expected_index: int,
    expected_outer_ref: int | None,
    expected_offset: int,
    expected_size: int,
    expected_sha256: str,
    label: str,
) -> None:
    if (
        entry.index != expected_index
        or entry.serial_offset != expected_offset
        or entry.serial_size != expected_size
        or (
            expected_outer_ref is not None
            and entry.package_index != expected_outer_ref
        )
        or _sha256_bytes(serial) != expected_sha256
    ):
        raise ContractError(f"{label} export identity changed")


def build_receipt(source_package: Path) -> dict[str, Any]:
    try:
        physical = source_package.read_bytes()
    except OSError as error:
        raise ContractError(f"could not read source package: {error}") from error
    if (
        source_package.name != SOURCE_FILE_NAME
        or len(physical) != SOURCE_BYTE_COUNT
        or _sha256_bytes(physical) != SOURCE_SHA256
    ):
        raise ContractError("Whirlwind source package identity changed")

    summary = ue3.read_package_summary(source_package)
    logical = ue3.decompress_package(physical, summary, ue3.LOSTARK_KR_AES_KEY)
    names = ue3.parse_name_table(logical, summary)
    imports = ue3.parse_import_table(logical, summary, names)
    exports = ue3.parse_export_table(logical, summary, names)

    outer = _one_export(
        exports,
        imports,
        class_name=OUTER_CLASS,
        object_name=OUTER_OBJECT,
    )
    outer_serial = logical[
        outer.serial_offset : outer.serial_offset + outer.serial_size
    ]
    _validate_export(
        outer,
        outer_serial,
        expected_index=OUTER_EXPORT_INDEX,
        expected_outer_ref=None,
        expected_offset=OUTER_SERIAL_OFFSET,
        expected_size=OUTER_SERIAL_SIZE,
        expected_sha256=OUTER_SERIAL_SHA256,
        label="EFData",
    )
    outer_properties, _ = ue3.parse_tagged_properties(
        outer_serial, names, summary.version
    )
    if _property(outer_properties, "trail_default") != HISTORY_EXPORT_REF:
        raise ContractError("EFData trail_default no longer references history export")

    history = _one_export(
        exports,
        imports,
        class_name=HISTORY_CLASS,
        object_name=HISTORY_OBJECT,
        outer_ref=HISTORY_OUTER_REF,
    )
    history_serial = logical[
        history.serial_offset : history.serial_offset + history.serial_size
    ]
    _validate_export(
        history,
        history_serial,
        expected_index=HISTORY_EXPORT_INDEX,
        expected_outer_ref=HISTORY_OUTER_REF,
        expected_offset=HISTORY_SERIAL_OFFSET,
        expected_size=HISTORY_SERIAL_SIZE,
        expected_sha256=HISTORY_SERIAL_SHA256,
        label="AnimNotify_Trails",
    )
    object_path = ue3.package_ref_path(history.index + 1, imports, exports)
    if object_path.casefold() != HISTORY_OBJECT_PATH.casefold():
        raise ContractError("AnimNotify_Trails object path changed")

    properties, _ = ue3.parse_tagged_properties(
        history_serial, names, summary.version
    )
    if _property(properties, "pstemplate") != PARTICLE_SYSTEM_REF:
        raise ContractError("AnimNotify_Trails particle-system reference changed")
    source_end_time = _finite_number(
        _property(properties, "endtime"), "AnimNotify_Trails.endtime"
    )
    if abs(source_end_time - SOURCE_END_TIME_SECONDS) > 1e-7:
        raise ContractError("AnimNotify_Trails source end time changed")

    raw_samples = _property(properties, "trailsampleddata")
    if not isinstance(raw_samples, list) or len(raw_samples) != EXPECTED_SAMPLE_COUNT:
        raise ContractError("AnimNotify_Trails sample denominator changed")
    samples: list[dict[str, Any]] = []
    previous_time = -math.inf
    for index, raw in enumerate(raw_samples):
        if not isinstance(raw, dict):
            raise ContractError(f"sample[{index}] is not a tagged struct")
        relative_time = _finite_number(
            _struct_value(raw, "relativetime"),
            f"sample[{index}].relativeTime",
        )
        if relative_time < previous_time:
            raise ContractError("AnimNotify_Trails sample times are not monotonic")
        previous_time = relative_time
        samples.append(
            {
                "relativeTimeSeconds": relative_time,
                "firstEdgeUE3Cm": _vector(
                    _struct_value(raw, "firstedgesample"),
                    f"sample[{index}].firstEdge",
                ),
                "controlPointUE3Cm": _vector(
                    _struct_value(raw, "controlpointsample"),
                    f"sample[{index}].controlPoint",
                ),
                "secondEdgeUE3Cm": _vector(
                    _struct_value(raw, "secondedgesample"),
                    f"sample[{index}].secondEdge",
                ),
            }
        )
    if (
        abs(samples[0]["relativeTimeSeconds"] - EXPECTED_FIRST_TIME_SECONDS)
        > 1e-7
        or abs(
            samples[-1]["relativeTimeSeconds"] - EXPECTED_LAST_TIME_SECONDS
        )
        > 1e-7
    ):
        raise ContractError("AnimNotify_Trails sample time bounds changed")

    receipt: dict[str, Any] = {
        "schema": SCHEMA,
        "formatVersion": FORMAT_VERSION,
        "historyId": HISTORY_ID,
        "source": {
            "physicalPackageFileName": SOURCE_FILE_NAME,
            "physicalPackageByteCount": SOURCE_BYTE_COUNT,
            "physicalPackageSha256": SOURCE_SHA256,
            "outerEfData": {
                "exportIndex": OUTER_EXPORT_INDEX,
                "exportRef": OUTER_EXPORT_REF,
                "className": "EFData_AnimNotify_Trails",
                "objectPath": OUTER_OBJECT,
                "serialOffset": OUTER_SERIAL_OFFSET,
                "serialByteCount": OUTER_SERIAL_SIZE,
                "serialSha256": OUTER_SERIAL_SHA256,
                "trailDefaultExportRef": HISTORY_EXPORT_REF,
            },
            "historyExport": {
                "exportIndex": HISTORY_EXPORT_INDEX,
                "exportRef": HISTORY_EXPORT_REF,
                "outerRef": HISTORY_OUTER_REF,
                "className": "AnimNotify_Trails",
                "objectPath": HISTORY_OBJECT_PATH,
                "serialOffset": HISTORY_SERIAL_OFFSET,
                "serialByteCount": HISTORY_SERIAL_SIZE,
                "serialSha256": HISTORY_SERIAL_SHA256,
                "particleSystemRef": PARTICLE_SYSTEM_REF,
                "particleSystemObjectPath": PARTICLE_SYSTEM_OBJECT_PATH,
                "sourceEndTimeSeconds": source_end_time,
            },
        },
        "playback": {
            "clampSeconds": PLAYBACK_CLAMP_SECONDS,
            "interpolation": "LINEAR_TIME_CLAMPED",
            "coordinateBasis": COORDINATE_BASIS,
        },
        "sampleCount": len(samples),
        "samples": samples,
    }
    receipt["artifactSha256"] = _sha256_bytes(_canonical_bytes(receipt))
    return receipt


def _write_transactionally(path: Path, payload: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="wb",
            prefix=f".{path.name}.",
            suffix=".tmp",
            dir=path.parent,
            delete=False,
        ) as stream:
            temporary_path = Path(stream.name)
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary_path, path)
        temporary_path = None
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository-root", type=Path, default=REPOSITORY_ROOT)
    parser.add_argument("--source-package", type=Path)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true")
    arguments = parser.parse_args()
    try:
        source_package = (
            arguments.source_package.resolve()
            if arguments.source_package is not None
            else _default_source_path().resolve()
        )
        repository_root = arguments.repository_root.resolve()
        output = arguments.output
        if not output.is_absolute():
            output = repository_root / output
        receipt = build_receipt(source_package)
        expected = _pretty_bytes(receipt)
        if arguments.check:
            if not output.is_file() or output.read_bytes() != expected:
                raise ContractError(f"baked edge history is stale: {output}")
        else:
            _write_transactionally(output, expected)
    except (ContractError, ue3.ExtractionError, OSError) as error:
        print(f"[valtan-whirlwind-baked-edge] FAIL: {error}")
        return 1

    verb = "verified" if arguments.check else "generated"
    print(
        f"[valtan-whirlwind-baked-edge] PASS: {verb} "
        f"{receipt['historyId']} samples={receipt['sampleCount']} "
        f"sha256={receipt['artifactSha256']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
