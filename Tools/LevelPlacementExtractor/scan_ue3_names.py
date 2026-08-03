#!/usr/bin/env python3
"""Search Lost Ark UE3 NameTables without exporting package payloads."""

from __future__ import annotations

import argparse
import csv
import json
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

from extract_ue3_placements import (
    LOSTARK_KR_AES_KEY,
    decompress_package,
    parse_name_table,
    parse_summary,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inventory-csv", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--prefix", action="append", default=[])
    parser.add_argument("--term", action="append", required=True)
    parser.add_argument("--workers", type=int, default=8)
    parser.add_argument("--aes-key", default=LOSTARK_KR_AES_KEY)
    return parser.parse_args()


def scan_package(row: dict[str, str], args: argparse.Namespace) -> dict:
    physical_path = args.package_root / row["physical_file"]
    physical = physical_path.read_bytes()
    summary = parse_summary(physical)
    logical = decompress_package(physical, summary, args.aes_key)
    names = parse_name_table(logical, summary)
    folded_terms = [term.casefold() for term in args.term]
    matches = sorted(
        {
            name
            for name in names
            if any(term in name.casefold() for term in folded_terms)
        },
        key=str.casefold,
    )
    return {
        "logicalPackage": row["logical_name"],
        "physicalPackage": row["physical_file"],
        "byteSize": int(row["byte_size"]),
        "nameCount": len(names),
        "matches": matches,
    }


def main() -> int:
    args = parse_args()
    prefixes = {value.casefold() for value in args.prefix}
    with args.inventory_csv.open("r", encoding="utf-8-sig", newline="") as source:
        rows = [
            row
            for row in csv.DictReader(source)
            if row.get("status") == "DONE"
            and (not prefixes or row.get("prefix", "").casefold() in prefixes)
        ]

    matches = []
    errors = []
    with ThreadPoolExecutor(max_workers=max(1, args.workers)) as executor:
        futures = {executor.submit(scan_package, row, args): row for row in rows}
        for complete_count, future in enumerate(as_completed(futures), 1):
            row = futures[future]
            try:
                result = future.result()
                if result["matches"]:
                    matches.append(result)
                    print(
                        json.dumps(
                            {
                                "package": result["logicalPackage"],
                                "matches": len(result["matches"]),
                            },
                            ensure_ascii=False,
                        )
                    )
            except Exception as error:  # preserve every per-package failure in the receipt
                errors.append(
                    {
                        "logicalPackage": row["logical_name"],
                        "physicalPackage": row["physical_file"],
                        "error": str(error),
                    }
                )
            if complete_count % 250 == 0:
                print(f"scanned {complete_count}/{len(rows)}")

    result = {
        "schemaVersion": 1,
        "terms": args.term,
        "prefixes": sorted(prefixes),
        "scannedPackageCount": len(rows),
        "matchedPackageCount": len(matches),
        "errorCount": len(errors),
        "packages": sorted(matches, key=lambda row: row["logicalPackage"].casefold()),
        "errors": sorted(errors, key=lambda row: row["logicalPackage"].casefold()),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(
        json.dumps(
            {
                "output": str(args.output),
                "scanned": len(rows),
                "matched": len(matches),
                "errors": len(errors),
            },
            ensure_ascii=False,
        )
    )
    return 0 if not errors else 1


if __name__ == "__main__":
    raise SystemExit(main())
