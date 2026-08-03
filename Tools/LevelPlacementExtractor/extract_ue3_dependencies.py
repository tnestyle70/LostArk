#!/usr/bin/env python3
"""Recover exact UE3 Import/Export dependency tables from Lost Ark packages."""

from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path

from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    decompress_package,
    package_ref_name,
    package_ref_path,
    parse_export_table,
    parse_import_table,
    parse_name_table,
    parse_summary,
    resolve_physical_package,
)


def import_root(index: int, imports: list[object]) -> str:
    cursor = -(index + 1)
    seen: set[int] = set()
    root = ""
    while cursor < 0 and cursor not in seen:
        seen.add(cursor)
        entry_index = -cursor - 1
        if not 0 <= entry_index < len(imports):
            break
        entry = imports[entry_index]
        root = entry.object_name
        cursor = entry.package_index
    return root


def extract_package(package_path: Path, logical_name: str, aes_key: str) -> dict:
    physical = package_path.read_bytes()
    summary = parse_summary(physical)
    logical = decompress_package(physical, summary, aes_key)
    names = parse_name_table(logical, summary)
    imports = parse_import_table(logical, summary, names)
    exports = parse_export_table(logical, summary, names)

    import_rows = []
    for entry in imports:
        package_index = -(entry.index + 1)
        import_rows.append(
            {
                "index": entry.index,
                "classPackage": entry.class_package,
                "className": entry.class_name,
                "packageIndex": entry.package_index,
                "objectName": entry.object_name,
                "fullPath": package_ref_path(package_index, imports, exports),
                "rootImport": import_root(entry.index, imports),
            }
        )

    export_rows = []
    for entry in exports:
        export_rows.append(
            {
                "index": entry.index,
                "className": package_ref_name(entry.class_index, imports, exports),
                "objectName": entry.object_name,
                "fullPath": package_ref_path(entry.index + 1, imports, exports),
                "serialSize": entry.serial_size,
                "serialOffset": entry.serial_offset,
            }
        )

    dependency_packages = sorted(
        {
            row["rootImport"]
            for row in import_rows
            if row["rootImport"]
            and row["rootImport"].casefold() not in {"core", "engine"}
        },
        key=str.casefold,
    )
    return {
        "schemaVersion": 1,
        "package": logical_name,
        "physicalPackage": str(package_path),
        "source": "decrypted UE3 Name/Import/Export tables",
        "counts": {
            "names": len(names),
            "imports": len(imports),
            "exports": len(exports),
        },
        "importClassCounts": dict(
            sorted(Counter(row["className"] for row in import_rows).items())
        ),
        "exportClassCounts": dict(
            sorted(Counter(row["className"] for row in export_rows).items())
        ),
        "dependencyPackages": dependency_packages,
        "imports": import_rows,
        "exports": export_rows,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--umodel", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--region", default="kr", choices=("kr", "na", "ru", "jp", "tw", "cn"))
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    parser.add_argument("packages", nargs="+")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    package_rows = []
    for logical_name in args.packages:
        physical_path = resolve_physical_package(
            args.umodel, args.package_root, logical_name, args.region
        )
        result = extract_package(physical_path, logical_name, args.aes_key)
        output_path = args.output / f"{logical_name}.dependencies.json"
        output_path.write_text(
            json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
        )
        package_rows.append(
            {
                "logicalPackage": logical_name,
                "physicalPackage": physical_path.name,
                "dependencyFile": output_path.name,
                **result["counts"],
                "dependencyPackageCount": len(result["dependencyPackages"]),
            }
        )
        print(json.dumps(package_rows[-1], ensure_ascii=False))

    manifest = {"schemaVersion": 1, "packages": package_rows}
    manifest_path = args.output / "dependency_manifest.json"
    manifest_path.write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps({"manifest": str(manifest_path), "packageCount": len(package_rows)}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
