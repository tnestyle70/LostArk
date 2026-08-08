#!/usr/bin/env python3
"""Extract UE3 texture address and color-space evidence from source packages."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    decompress_package,
    package_ref_name,
    package_ref_path,
    parse_export_table,
    parse_import_table,
    parse_name_table,
    parse_summary,
    parse_tagged_properties,
)


def folded(value: Any) -> str:
    return str(value or "").strip().casefold()


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def property_value(properties: dict[str, Any], name: str) -> Any:
    wanted = folded(name)
    for key, item in properties.items():
        if folded(key) != wanted:
            continue
        return item.get("value") if isinstance(item, dict) else item
    return None


def normalize_address(value: Any) -> tuple[str, str]:
    if value is None:
        return "wrap", "ue3_class_default"
    token = folded(value)
    if token in {"ta_wrap", "wrap", "0"}:
        return "wrap", "serialized_property"
    if token in {"ta_clamp", "clamp", "1"}:
        return "clamp", "serialized_property"
    raise ValueError(f"unsupported UE3 texture address mode: {value}")


def normalize_color_space(value: Any) -> tuple[str, str]:
    if value is None:
        return "srgb", "ue3_texture_class_default"
    if isinstance(value, bool):
        return ("srgb" if value else "linear"), "serialized_property"
    token = folded(value)
    if token in {"true", "1"}:
        return "srgb", "serialized_property"
    if token in {"false", "0"}:
        return "linear", "serialized_property"
    raise ValueError(f"unsupported UE3 texture sRGB value: {value}")


def sampling_row(
    source_object_path: str,
    physical_package: str,
    package_sha256: str,
    properties: dict[str, Any],
) -> dict[str, Any]:
    address_u, address_u_evidence = normalize_address(
        property_value(properties, "addressx")
    )
    address_v, address_v_evidence = normalize_address(
        property_value(properties, "addressy")
    )
    color_space, color_space_evidence = normalize_color_space(
        property_value(properties, "srgb")
    )
    return {
        "sourceObjectPath": source_object_path,
        "physicalPackage": physical_package,
        "physicalPackageSha256": package_sha256,
        "addressU": address_u,
        "addressV": address_v,
        "colorSpace": color_space,
        "addressUEvidence": address_u_evidence,
        "addressVEvidence": address_v_evidence,
        "colorSpaceEvidence": color_space_evidence,
        "samplingEvidence": "ue3_property_or_class_default.v1",
    }


def extract_package_rows(
    package_path: Path,
    wanted_paths: set[str],
    aes_key: str,
) -> list[dict[str, Any]]:
    physical = package_path.read_bytes()
    summary = parse_summary(physical)
    logical = decompress_package(physical, summary, aes_key)
    names = parse_name_table(logical, summary)
    imports = parse_import_table(logical, summary, names)
    exports = parse_export_table(logical, summary, names)
    rows = []
    for entry in exports:
        object_path = package_ref_path(entry.index + 1, imports, exports)
        matches = [
            wanted for wanted in wanted_paths
            if wanted.rsplit(".", 1)[-1] == folded(entry.object_name)
        ]
        if not matches:
            continue
        if len(matches) != 1:
            raise ValueError(
                f"ambiguous manifest texture object: {entry.object_name}"
            )
        source_object_path = matches[0]
        class_name = folded(package_ref_name(entry.class_index, imports, exports))
        if class_name not in {"texture", "texture2d"}:
            raise ValueError(
                "source texture export has invalid class: "
                f"{source_object_path}:{class_name}"
            )
        serial = logical[
            entry.serial_offset : entry.serial_offset + entry.serial_size
        ]
        properties, _ = parse_tagged_properties(serial, names, summary.version)
        rows.append(sampling_row(
            source_object_path, package_path.name, sha256_bytes(physical),
            properties,
        ))
    return rows


def selected_texture_assets(
    resource_manifest: dict[str, Any],
    skill_ids: set[int] | None = None,
) -> list[dict[str, Any]]:
    """Select manifest-admitted source texture candidates.

    The optional skill filter consumes the manifest's propagated ownership;
    it must not infer type or ownership from names such as ``*_cube``. The
    package extractor validates the actual UE export class before recording
    sampling evidence.
    """
    return [
        row for row in resource_manifest.get("assets", [])
        if "texture" in {folded(role) for role in row.get("roles", [])}
        and row.get("sourceAssetPath")
        and row.get("physicalPackage")
        and (
            not skill_ids
            or skill_ids & {int(value) for value in row.get("skillIds", [])}
        )
    ]


def extract_sampling_evidence(
    resource_manifest: dict[str, Any],
    source_root: Path,
    aes_key: str = LOSTARK_KR_AES_KEY,
    skill_ids: set[int] | None = None,
) -> dict[str, Any]:
    texture_assets = selected_texture_assets(resource_manifest, skill_ids)
    package_index: dict[str, list[Path]] = {}
    for path in source_root.rglob("*.upk"):
        package_index.setdefault(path.name.casefold(), []).append(path)

    expected_by_package: dict[str, set[str]] = {}
    for row in texture_assets:
        expected_by_package.setdefault(
            folded(row["physicalPackage"]), set()
        ).add(folded(row["sourceAssetPath"]))

    textures = []
    failures = []
    for physical_package, wanted_paths in sorted(expected_by_package.items()):
        candidates = package_index.get(physical_package, [])
        if len(candidates) != 1:
            failures.append({
                "physicalPackage": physical_package,
                "status": "MISSING_OR_AMBIGUOUS_SOURCE_PACKAGE",
                "candidateCount": len(candidates),
            })
            continue
        package_rows = extract_package_rows(
            candidates[0], wanted_paths, aes_key
        )
        found = {folded(row["sourceObjectPath"]) for row in package_rows}
        textures.extend(package_rows)
        for missing in sorted(wanted_paths - found):
            failures.append({
                "sourceObjectPath": missing,
                "physicalPackage": candidates[0].name,
                "status": "MISSING_TEXTURE_EXPORT",
            })

    textures.sort(key=lambda row: folded(row["sourceObjectPath"]))
    return {
        "schema": "lostark.ue3-texture-sampling-evidence",
        "formatVersion": 1,
        "characterClass": str(
            resource_manifest.get("characterClass") or ""
        ),
        "textures": textures,
        "failures": failures,
        "summary": {
            "textureCount": len(textures),
            "failureCount": len(failures),
            "explicitAddressUCount": sum(
                row["addressUEvidence"] == "serialized_property"
                for row in textures
            ),
            "explicitAddressVCount": sum(
                row["addressVEvidence"] == "serialized_property"
                for row in textures
            ),
            "explicitColorSpaceCount": sum(
                row["colorSpaceEvidence"] == "serialized_property"
                for row in textures
            ),
        },
    }


def write_json_atomic(path: Path, value: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    staged = path.with_suffix(path.suffix + ".tmp")
    staged.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    staged.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--resource-manifest", required=True, type=Path)
    parser.add_argument("--source-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    parser.add_argument("--skill-id", type=int, action="append", default=[])
    args = parser.parse_args()
    evidence = extract_sampling_evidence(
        json.loads(args.resource_manifest.read_text(encoding="utf-8-sig")),
        args.source_root,
        args.aes_key,
        set(args.skill_id) or None,
    )
    write_json_atomic(args.output, evidence)
    print(json.dumps(evidence["summary"], sort_keys=True))
    return 0 if not evidence["failures"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
