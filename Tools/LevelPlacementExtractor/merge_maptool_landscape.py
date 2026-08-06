from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import shlex
import tempfile
from pathlib import Path
from typing import Any, Sequence


CATALOG_MAGIC = "LOSTARK_MAP_ASSET_CATALOG"
CATALOG_VERSION = 4
PLACEMENT_MAGIC = "LOSTARK_MAP_PLACEMENTS"
PLACEMENT_VERSION = 2
IMPORTED_ID_BIT = 1 << 63
EDITOR_ID_MASK = IMPORTED_ID_BIT - 1
UINT64_MAX = (1 << 64) - 1
WINT_HEADER_SIZE = 16
WMOD_META_SIZE = 32
WMOD_SECTION_DESC_SIZE = 64
MAX_MODEL_SECTIONS = 4096
MODEL_SECTION_MESH = 1
MODEL_SECTION_MATERIAL = 2
MODEL_SECTION_ANIMATION = 4


class MapDocumentMergeError(RuntimeError):
    pass


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def imported_id(source_placement_id: str) -> int:
    digest = hashlib.sha256(source_placement_id.encode("utf-8")).digest()
    return int.from_bytes(digest[:8], "big") | IMPORTED_ID_BIT


def quoted(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def path_key(path: Path) -> str:
    return os.path.normcase(str(path.resolve()))


def validate_role_paths(args: argparse.Namespace) -> None:
    sources = {
        "base catalog": path_key(args.base_catalog),
        "base placements": path_key(args.base_placements),
        "Landscape catalog": path_key(args.landscape_catalog),
        "Landscape placements": path_key(args.landscape_placements),
    }
    if len(set(sources.values())) != len(sources):
        raise MapDocumentMergeError("input document roles must use distinct paths")

    catalog_output = path_key(args.catalog_output)
    placement_output = path_key(args.placement_output)
    if catalog_output == placement_output:
        raise MapDocumentMergeError("catalog and placement outputs must be distinct")
    if catalog_output in {
        sources["base placements"],
        sources["Landscape catalog"],
        sources["Landscape placements"],
    }:
        raise MapDocumentMergeError("catalog output aliases an incompatible input role")
    if placement_output in {
        sources["base catalog"],
        sources["Landscape catalog"],
        sources["Landscape placements"],
    }:
        raise MapDocumentMergeError("placement output aliases an incompatible input role")

    if args.receipt_output is not None:
        receipt_output = path_key(args.receipt_output)
        protected = set(sources.values()) | {catalog_output, placement_output}
        if receipt_output in protected:
            raise MapDocumentMergeError(
                "receipt output must not alias an input or map document output"
            )


def parse_document_header(
    path: Path,
    expected_magic: str,
    expected_version: int,
) -> tuple[str, list[str]]:
    if not path.is_file():
        raise MapDocumentMergeError(f"input document is missing: {path}")
    lines = path.read_text(encoding="utf-8-sig").splitlines()
    if not lines:
        raise MapDocumentMergeError(f"empty document: {path}")
    try:
        header = shlex.split(lines[0], posix=True)
    except ValueError as error:
        raise MapDocumentMergeError(f"invalid document header: {path}: {error}") from error
    if (
        len(header) != 4
        or header[0] != expected_magic
        or header[1] != str(expected_version)
    ):
        raise MapDocumentMergeError(f"unsupported document header: {path}")
    try:
        declared_count = int(header[3])
    except ValueError as error:
        raise MapDocumentMergeError(f"invalid document count: {path}") from error
    rows = [line for line in lines[1:] if line.strip()]
    if declared_count != len(rows):
        raise MapDocumentMergeError(
            f"document count mismatch: {path}: {declared_count} != {len(rows)}"
        )
    return header[2], rows


def parse_catalog(path: Path) -> dict[str, Any]:
    area_id, rows = parse_document_header(path, CATALOG_MAGIC, CATALOG_VERSION)
    entries: list[dict[str, Any]] = []
    asset_ids: set[str] = set()
    prototype_tags: set[str] = set()
    for line_number, row in enumerate(rows, 2):
        try:
            fields = shlex.split(row, posix=True)
        except ValueError as error:
            raise MapDocumentMergeError(
                f"invalid catalog row {line_number}: {path}: {error}"
            ) from error
        if len(fields) != 26:
            raise MapDocumentMergeError(
                f"catalog row {line_number} must contain 26 fields: {path}"
            )
        asset_id = fields[0]
        prototype_tag = fields[3]
        if not asset_id or asset_id in asset_ids:
            raise MapDocumentMergeError(
                f"duplicate/empty catalog asset ID: {asset_id!r}: {path}"
            )
        if not prototype_tag or prototype_tag in prototype_tags:
            raise MapDocumentMergeError(
                f"duplicate/empty catalog prototype tag: {prototype_tag!r}: {path}"
            )
        numeric_indices = (*range(4, 7), *range(13, 26))
        try:
            numeric_values = [float(fields[index]) for index in numeric_indices]
        except ValueError as error:
            raise MapDocumentMergeError(
                f"invalid catalog numeric value at row {line_number}: {path}"
            ) from error
        if not all(math.isfinite(value) for value in numeric_values):
            raise MapDocumentMergeError(
                f"non-finite catalog value at row {line_number}: {path}"
            )
        asset_ids.add(asset_id)
        prototype_tags.add(prototype_tag)
        entries.append(
            {
                "text": row,
                "assetId": asset_id,
                "modelPath": fields[2],
                "prototypeTag": prototype_tag,
            }
        )
    return {
        "areaId": area_id,
        "entries": entries,
        "assetIds": asset_ids,
        "prototypeTags": prototype_tags,
    }


def parse_placements(path: Path, catalog_ids: set[str]) -> dict[str, Any]:
    area_id, rows = parse_document_header(path, PLACEMENT_MAGIC, PLACEMENT_VERSION)
    entries: list[dict[str, Any]] = []
    runtime_ids: set[int] = set()
    source_ids: set[str] = set()
    for line_number, row in enumerate(rows, 2):
        try:
            fields = shlex.split(row, posix=True)
        except ValueError as error:
            raise MapDocumentMergeError(
                f"invalid placement row {line_number}: {path}: {error}"
            ) from error
        if len(fields) != 16:
            raise MapDocumentMergeError(
                f"placement row {line_number} must contain 16 fields: {path}"
            )
        try:
            runtime_id = int(fields[0])
        except ValueError as error:
            raise MapDocumentMergeError(
                f"invalid placement runtime ID at row {line_number}: {path}"
            ) from error
        source_id = fields[1]
        transform_source = fields[3]
        asset_id = fields[4]
        if not 0 < runtime_id <= UINT64_MAX:
            raise MapDocumentMergeError(
                f"placement runtime ID is out of range: {runtime_id}: {path}"
            )
        if runtime_id in runtime_ids:
            raise MapDocumentMergeError(
                f"duplicate placement runtime ID: {runtime_id}: {path}"
            )
        if not source_id or source_id in source_ids:
            raise MapDocumentMergeError(
                f"duplicate/empty placement source ID: {source_id!r}: {path}"
            )
        if transform_source == "overlay":
            if runtime_id > EDITOR_ID_MASK:
                raise MapDocumentMergeError(
                    f"overlay placement runtime ID is outside its domain: {runtime_id}"
                )
        elif runtime_id != imported_id(source_id):
            raise MapDocumentMergeError(
                f"placement runtime ID is not stable for {source_id}: {path}"
            )
        if asset_id not in catalog_ids:
            raise MapDocumentMergeError(
                f"placement asset join is missing: {asset_id}: {path}"
            )
        try:
            numeric = tuple(float(value) for value in fields[5:15])
        except ValueError as error:
            raise MapDocumentMergeError(
                f"invalid placement transform: {source_id}: {path}"
            ) from error
        if not all(math.isfinite(value) for value in numeric):
            raise MapDocumentMergeError(
                f"non-finite placement transform: {source_id}: {path}"
            )
        if any(abs(value) < 1.0e-6 for value in numeric[7:10]):
            raise MapDocumentMergeError(f"zero placement scale: {source_id}: {path}")
        if fields[15] not in ("0", "1"):
            raise MapDocumentMergeError(
                f"invalid placement visibility: {source_id}: {path}"
            )
        runtime_ids.add(runtime_id)
        source_ids.add(source_id)
        entries.append(
            {
                "text": row,
                "runtimeId": runtime_id,
                "sourceId": source_id,
                "transformSource": transform_source,
                "assetId": asset_id,
            }
        )
    return {
        "areaId": area_id,
        "entries": entries,
        "runtimeIds": runtime_ids,
        "sourceIds": source_ids,
    }


def validate_runtime_models(entries: Sequence[dict[str, Any]], runtime_root: Path) -> None:
    root = runtime_root.resolve()
    for entry in entries:
        relative = Path(str(entry["modelPath"]))
        if relative.is_absolute() or relative.suffix.casefold() != ".wmodel":
            raise MapDocumentMergeError(
                f"invalid Landscape runtime model path: {entry['modelPath']}"
            )
        model_path = (root / relative).resolve()
        try:
            model_path.relative_to(root)
        except ValueError as error:
            raise MapDocumentMergeError(
                f"Landscape runtime model escapes runtime root: {entry['modelPath']}"
            ) from error
        if not model_path.is_file():
            raise MapDocumentMergeError(
                f"Landscape runtime model is missing: {model_path}"
            )
        model_bytes = model_path.read_bytes()
        if len(model_bytes) < WINT_HEADER_SIZE + WMOD_META_SIZE:
            raise MapDocumentMergeError(
                f"invalid Landscape WModel container: {model_path}"
            )

        version_major = int.from_bytes(model_bytes[4:6], "little")
        flags = int.from_bytes(model_bytes[8:12], "little")
        declared_payload_size = int.from_bytes(model_bytes[12:16], "little")
        content = model_bytes[WINT_HEADER_SIZE:]
        if (
            model_bytes[:4] != b"WINT"
            or version_major != 1
            or flags != 0
            or declared_payload_size != len(content)
            or content[:4] != b"WMOD"
        ):
            raise MapDocumentMergeError(
                f"invalid Landscape WModel container: {model_path}"
            )

        section_count = int.from_bytes(content[4:8], "little")
        animation_count = int.from_bytes(content[8:12], "little")
        section_table_end = (
            WMOD_META_SIZE + WMOD_SECTION_DESC_SIZE * section_count
        )
        if (
            section_count < 2
            or section_count > MAX_MODEL_SECTIONS
            or animation_count > section_count
            or section_table_end > len(content)
        ):
            raise MapDocumentMergeError(
                f"invalid Landscape WModel metadata: {model_path}"
            )

        section_type_counts: dict[int, int] = {}
        for section_index in range(section_count):
            descriptor_offset = (
                WMOD_META_SIZE + section_index * WMOD_SECTION_DESC_SIZE
            )
            descriptor = content[
                descriptor_offset:descriptor_offset + WMOD_SECTION_DESC_SIZE
            ]
            section_type = int.from_bytes(descriptor[0:4], "little")
            section_offset = int.from_bytes(descriptor[8:16], "little")
            section_size = int.from_bytes(descriptor[16:24], "little")
            if (
                section_type < MODEL_SECTION_MESH
                or section_type > MODEL_SECTION_ANIMATION
                or section_offset > len(content)
                or section_size > len(content) - section_offset
            ):
                raise MapDocumentMergeError(
                    f"invalid Landscape WModel section table: {model_path}"
                )
            section_type_counts[section_type] = (
                section_type_counts.get(section_type, 0) + 1
            )

        if (
            section_type_counts.get(MODEL_SECTION_MESH) != 1
            or section_type_counts.get(MODEL_SECTION_MATERIAL) != 1
            or section_type_counts.get(MODEL_SECTION_ANIMATION, 0)
            != animation_count
        ):
            raise MapDocumentMergeError(
                f"invalid Landscape WModel required sections: {model_path}"
            )


def merge_unique_entries(
    base_entries: Sequence[dict[str, Any]],
    incoming_entries: Sequence[dict[str, Any]],
    key: str,
    label: str,
) -> tuple[list[dict[str, Any]], int]:
    result = list(base_entries)
    by_key = {entry[key]: entry for entry in base_entries}
    added = 0
    for entry in sorted(incoming_entries, key=lambda value: str(value[key])):
        existing = by_key.get(entry[key])
        if existing is not None:
            if existing["text"] != entry["text"]:
                raise MapDocumentMergeError(
                    f"conflicting {label}: {entry[key]!r}"
                )
            continue
        result.append(entry)
        by_key[entry[key]] = entry
        added += 1
    return result, added


def stage_bytes(path: Path, value: bytes) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "wb") as output:
            output.write(value)
            output.flush()
            os.fsync(output.fileno())
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise
    return temporary


def stage_text(path: Path, value: str) -> Path:
    return stage_bytes(path, value.encode("utf-8"))


def replace_document_pair(
    catalog_output: Path,
    catalog_text: str,
    placement_output: Path,
    placement_text: str,
) -> None:
    catalog_temporary = stage_text(catalog_output, catalog_text)
    placement_temporary = stage_text(placement_output, placement_text)
    previous_catalog = catalog_output.read_bytes() if catalog_output.is_file() else None
    try:
        os.replace(catalog_temporary, catalog_output)
        os.replace(placement_temporary, placement_output)
    except BaseException:
        catalog_temporary.unlink(missing_ok=True)
        placement_temporary.unlink(missing_ok=True)
        if previous_catalog is not None:
            restore = stage_bytes(catalog_output, previous_catalog)
            os.replace(restore, catalog_output)
        else:
            catalog_output.unlink(missing_ok=True)
        raise


def atomic_write_text(path: Path, value: str) -> None:
    temporary = stage_text(path, value)
    try:
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def existing_receipt_matches(
    path: Path,
    area_id: str,
    catalog_hash: str,
    placement_hash: str,
) -> bool:
    if not path.exists():
        return False
    if not path.is_file():
        raise MapDocumentMergeError(f"receipt output is not a file: {path}")
    try:
        receipt = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise MapDocumentMergeError(
            f"existing receipt is unreadable: {path}: {error}"
        ) from error
    outputs = receipt.get("outputs")
    if (
        receipt.get("schemaVersion") != 1
        or receipt.get("areaId") != area_id
        or not isinstance(outputs, dict)
        or outputs.get("catalog") != catalog_hash
        or outputs.get("placements") != placement_hash
    ):
        raise MapDocumentMergeError(
            f"existing receipt describes different map outputs: {path}"
        )
    return True


def merge_landscape(args: argparse.Namespace) -> dict[str, Any]:
    validate_role_paths(args)
    base_catalog = parse_catalog(args.base_catalog)
    base_placements = parse_placements(
        args.base_placements, set(base_catalog["assetIds"])
    )
    landscape_catalog = parse_catalog(args.landscape_catalog)
    landscape_placements = parse_placements(
        args.landscape_placements, set(landscape_catalog["assetIds"])
    )

    area_id = str(args.area_id)
    if base_catalog["areaId"] != area_id or base_placements["areaId"] != area_id:
        raise MapDocumentMergeError("base document areaId mismatch")
    if landscape_catalog["areaId"] != landscape_placements["areaId"]:
        raise MapDocumentMergeError("Landscape document areaId mismatch")
    valid_landscape_areas = {area_id, f"{area_id}_LANDSCAPE"}
    if landscape_catalog["areaId"] not in valid_landscape_areas:
        raise MapDocumentMergeError(
            f"unexpected Landscape areaId: {landscape_catalog['areaId']}"
        )
    if any(
        entry["transformSource"] != "component"
        for entry in landscape_placements["entries"]
    ):
        raise MapDocumentMergeError(
            "Landscape placements must retain transformSource=component"
        )

    if args.expect_base_assets is not None and len(base_catalog["entries"]) != args.expect_base_assets:
        raise MapDocumentMergeError(
            f"base asset count mismatch: {len(base_catalog['entries'])}"
        )
    if args.expect_base_placements is not None and len(base_placements["entries"]) != args.expect_base_placements:
        raise MapDocumentMergeError(
            f"base placement count mismatch: {len(base_placements['entries'])}"
        )
    if args.expect_landscape_assets is not None and len(landscape_catalog["entries"]) != args.expect_landscape_assets:
        raise MapDocumentMergeError(
            f"Landscape asset count mismatch: {len(landscape_catalog['entries'])}"
        )
    if args.expect_landscape_placements is not None and len(landscape_placements["entries"]) != args.expect_landscape_placements:
        raise MapDocumentMergeError(
            f"Landscape placement count mismatch: {len(landscape_placements['entries'])}"
        )

    validate_runtime_models(landscape_catalog["entries"], args.runtime_root)

    base_prototype_to_asset = {
        entry["prototypeTag"]: entry["assetId"] for entry in base_catalog["entries"]
    }
    for entry in landscape_catalog["entries"]:
        previous_asset = base_prototype_to_asset.get(entry["prototypeTag"])
        if previous_asset is not None and previous_asset != entry["assetId"]:
            raise MapDocumentMergeError(
                f"prototype tag collision: {entry['prototypeTag']}"
            )

    catalog_entries, added_assets = merge_unique_entries(
        base_catalog["entries"], landscape_catalog["entries"], "assetId", "asset ID"
    )
    final_asset_ids = {entry["assetId"] for entry in catalog_entries}

    base_runtime_to_source = {
        entry["runtimeId"]: entry["sourceId"] for entry in base_placements["entries"]
    }
    base_source_to_runtime = {
        entry["sourceId"]: entry["runtimeId"] for entry in base_placements["entries"]
    }
    for entry in landscape_placements["entries"]:
        previous_source = base_runtime_to_source.get(entry["runtimeId"])
        if previous_source is not None and previous_source != entry["sourceId"]:
            raise MapDocumentMergeError(
                f"placement runtime ID collision: {entry['runtimeId']}"
            )
        previous_runtime = base_source_to_runtime.get(entry["sourceId"])
        if previous_runtime is not None and previous_runtime != entry["runtimeId"]:
            raise MapDocumentMergeError(
                f"placement source ID collision: {entry['sourceId']}"
            )
        if entry["assetId"] not in final_asset_ids:
            raise MapDocumentMergeError(
                f"merged placement asset join is missing: {entry['assetId']}"
            )

    placement_entries, added_placements = merge_unique_entries(
        base_placements["entries"],
        landscape_placements["entries"],
        "sourceId",
        "placement source ID",
    )

    if args.expect_output_assets is not None and len(catalog_entries) != args.expect_output_assets:
        raise MapDocumentMergeError(
            f"output asset count mismatch: {len(catalog_entries)}"
        )
    if args.expect_output_placements is not None and len(placement_entries) != args.expect_output_placements:
        raise MapDocumentMergeError(
            f"output placement count mismatch: {len(placement_entries)}"
        )

    catalog_text = (
        f"{CATALOG_MAGIC} {CATALOG_VERSION} {quoted(area_id)} {len(catalog_entries)}\n"
        + "\n".join(str(entry["text"]) for entry in catalog_entries)
        + "\n"
    )
    placement_text = (
        f"{PLACEMENT_MAGIC} {PLACEMENT_VERSION} {quoted(area_id)} {len(placement_entries)}\n"
        + "\n".join(str(entry["text"]) for entry in placement_entries)
        + "\n"
    )
    input_hashes = {
        "baseCatalog": sha256(args.base_catalog),
        "basePlacements": sha256(args.base_placements),
        "landscapeCatalog": sha256(args.landscape_catalog),
        "landscapePlacements": sha256(args.landscape_placements),
    }
    catalog_hash = hashlib.sha256(catalog_text.encode("utf-8")).hexdigest().upper()
    placement_hash = hashlib.sha256(
        placement_text.encode("utf-8")
    ).hexdigest().upper()
    receipt: dict[str, Any] = {
        "schemaVersion": 1,
        "areaId": area_id,
        "checkOnly": bool(args.check_only),
        "baseAssetCount": len(base_catalog["entries"]),
        "basePlacementCount": len(base_placements["entries"]),
        "landscapeAssetCount": len(landscape_catalog["entries"]),
        "landscapePlacementCount": len(landscape_placements["entries"]),
        "addedAssetCount": added_assets,
        "addedPlacementCount": added_placements,
        "outputAssetCount": len(catalog_entries),
        "outputPlacementCount": len(placement_entries),
        "inputs": input_hashes,
        "outputs": {
            "catalog": catalog_hash,
            "placements": placement_hash,
        },
    }

    preserve_existing_receipt = False
    if args.receipt_output is not None:
        preserve_existing_receipt = existing_receipt_matches(
            args.receipt_output,
            area_id,
            catalog_hash,
            placement_hash,
        )

    if not args.check_only:
        replace_document_pair(
            args.catalog_output,
            catalog_text,
            args.placement_output,
            placement_text,
        )

    if (
        args.receipt_output is not None
        and not args.check_only
        and not preserve_existing_receipt
    ):
        atomic_write_text(
            args.receipt_output,
            json.dumps(receipt, ensure_ascii=False, indent=2) + "\n",
        )
    return receipt


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Validate and idempotently merge exact Landscape MapTool documents "
            "into one editable area catalog/placement pair."
        )
    )
    parser.add_argument("--area-id", required=True)
    parser.add_argument("--base-catalog", type=Path, required=True)
    parser.add_argument("--base-placements", type=Path, required=True)
    parser.add_argument("--landscape-catalog", type=Path, required=True)
    parser.add_argument("--landscape-placements", type=Path, required=True)
    parser.add_argument("--runtime-root", type=Path, required=True)
    parser.add_argument("--catalog-output", type=Path, required=True)
    parser.add_argument("--placement-output", type=Path, required=True)
    parser.add_argument("--receipt-output", type=Path)
    parser.add_argument("--expect-base-assets", type=int)
    parser.add_argument("--expect-base-placements", type=int)
    parser.add_argument("--expect-landscape-assets", type=int)
    parser.add_argument("--expect-landscape-placements", type=int)
    parser.add_argument("--expect-output-assets", type=int)
    parser.add_argument("--expect-output-placements", type=int)
    parser.add_argument("--check-only", action="store_true")
    return parser.parse_args()


def main() -> int:
    receipt = merge_landscape(parse_args())
    print(json.dumps(receipt, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
