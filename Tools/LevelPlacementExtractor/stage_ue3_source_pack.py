#!/usr/bin/env python3
"""Stage an immutable UE3 source package closure with SHA-256 receipts."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import shutil
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inventory-csv", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--particle-manifest", type=Path, required=True)
    parser.add_argument("--destination", type=Path, required=True)
    parser.add_argument("--context-package", action="append", default=[])
    parser.add_argument("--report-file", type=Path, action="append", default=[])
    parser.add_argument(
        "--allow-unresolved",
        action="store_true",
        help="succeed while preserving unresolved package names in the receipt",
    )
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def copy_verified(source: Path, destination: Path, expected_hash: str) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists():
        if sha256_file(destination) != expected_hash:
            raise ValueError(f"immutable file conflict: {destination}")
        return
    temporary = destination.with_suffix(destination.suffix + ".tmp")
    shutil.copyfile(source, temporary)
    if sha256_file(temporary) != expected_hash:
        temporary.unlink(missing_ok=True)
        raise ValueError(f"copy verification failed: {source}")
    temporary.replace(destination)


def content_sha256(entries: list[dict[str, object]]) -> str:
    digest = hashlib.sha256()
    for row in sorted(entries, key=lambda item: str(item["relativePath"]).casefold()):
        digest.update(str(row["relativePath"]).encode("utf-8"))
        digest.update(b"\0")
        digest.update(str(row["byteSize"]).encode("ascii"))
        digest.update(b"\0")
        digest.update(str(row["sha256"]).encode("ascii"))
        digest.update(b"\n")
    return digest.hexdigest()


def load_inventory(path: Path) -> dict[str, dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as source:
        rows = list(csv.DictReader(source))
    result: dict[str, dict[str, str]] = {}
    for row in rows:
        key = row["logical_name"].casefold()
        if key in result:
            raise ValueError(f"duplicate logical package in inventory: {row['logical_name']}")
        result[key] = row
    return result


def resolve_package(
    inventory: dict[str, dict[str, str]], logical_name: str, role: str
) -> dict[str, object]:
    row = inventory.get(logical_name.casefold())
    if row is None:
        return {"logicalPackage": logical_name, "role": role, "resolved": False}
    return {
        "logicalPackage": row["logical_name"],
        "physicalPackage": row["physical_file"],
        "inventoryByteSize": int(row["byte_size"]),
        "role": role,
        "resolved": True,
    }


def main() -> int:
    args = parse_args()
    inventory = load_inventory(args.inventory_csv)
    particle_manifest = json.loads(args.particle_manifest.read_text(encoding="utf-8"))

    selected: dict[str, dict[str, object]] = {}

    def add(logical_name: str, role: str) -> None:
        key = logical_name.casefold()
        candidate = resolve_package(inventory, logical_name, role)
        existing = selected.get(key)
        priority = {"dependency": 0, "context": 1, "core": 2}
        if existing is None or priority[role] > priority[str(existing["role"])]:
            selected[key] = candidate

    for row in particle_manifest["dependencies"]:
        add(row["logicalPackage"], "dependency")
    for logical_name in args.context_package:
        add(logical_name, "context")
    for row in particle_manifest["sourcePackages"]:
        add(row["logicalPackage"], "core")

    unresolved = sorted(
        (row for row in selected.values() if not row["resolved"]),
        key=lambda row: str(row["logicalPackage"]).casefold(),
    )
    staged = []
    role_dirs = {"core": "Core", "context": "Context", "dependency": "Dependencies"}
    for row in sorted(
        (row for row in selected.values() if row["resolved"]),
        key=lambda row: (str(row["role"]), str(row["logicalPackage"]).casefold()),
    ):
        source = (args.package_root / str(row["physicalPackage"])).resolve()
        if not source.is_file():
            raise FileNotFoundError(f"package payload missing: {source}")
        actual_size = source.stat().st_size
        inventory_size = int(row["inventoryByteSize"])

        relative_path = Path(role_dirs[str(row["role"])]) / source.name
        destination = (args.destination / relative_path).resolve()
        source_hash = sha256_file(source)
        copy_verified(source, destination, source_hash)

        staged.append(
            {
                **row,
                "byteSize": actual_size,
                "inventorySizeMatch": actual_size == inventory_size,
                "relativePath": relative_path.as_posix(),
                "sha256": source_hash,
            }
        )

    reports = []
    report_names: set[str] = set()
    for source in args.report_file:
        source = source.resolve()
        if not source.is_file():
            raise FileNotFoundError(f"report file missing: {source}")
        if source.name.casefold() in report_names:
            raise ValueError(f"duplicate report filename: {source.name}")
        report_names.add(source.name.casefold())
        relative_path = Path("Reports") / source.name
        destination = (args.destination / relative_path).resolve()
        source_hash = sha256_file(source)
        copy_verified(source, destination, source_hash)
        reports.append(
            {
                "sourceFile": str(source),
                "relativePath": relative_path.as_posix(),
                "byteSize": source.stat().st_size,
                "sha256": source_hash,
            }
        )

    payload_entries = [*staged, *reports]
    source_manifest_relative = next(
        (
            row["relativePath"]
            for row in reports
            if Path(str(row["sourceFile"])) == args.particle_manifest.resolve()
        ),
        None,
    )

    result = {
        "schemaVersion": 1,
        "sourceManifest": source_manifest_relative,
        "inputHashes": {
            "inventorySha256": sha256_file(args.inventory_csv),
            "particleManifestSha256": sha256_file(args.particle_manifest),
            "stagingToolSha256": sha256_file(Path(__file__)),
        },
        "summary": {
            "packageCount": len(staged),
            "corePackageCount": sum(row["role"] == "core" for row in staged),
            "contextPackageCount": sum(row["role"] == "context" for row in staged),
            "dependencyPackageCount": sum(
                row["role"] == "dependency" for row in staged
            ),
            "byteSize": sum(int(row["byteSize"]) for row in staged),
            "inventorySizeMismatchCount": sum(
                not row["inventorySizeMatch"] for row in staged
            ),
            "unresolvedPackageCount": len(unresolved),
            "reportFileCount": len(reports),
            "reportBytes": sum(int(row["byteSize"]) for row in reports),
            "contentSha256": content_sha256(payload_entries),
        },
        "packages": staged,
        "reports": reports,
        "unresolvedPackages": unresolved,
    }
    args.destination.mkdir(parents=True, exist_ok=True)
    receipt = args.destination / "source_pack_manifest.json"
    serialized = json.dumps(result, ensure_ascii=False, indent=2) + "\n"
    if receipt.exists() and receipt.read_text(encoding="utf-8") != serialized:
        raise ValueError(f"immutable manifest conflict: {receipt}")
    receipt.write_text(serialized, encoding="utf-8")
    receipt_hash = sha256_file(receipt)
    sidecar = receipt.with_suffix(receipt.suffix + ".sha256")
    sidecar_text = f"{receipt_hash}  {receipt.name}\n"
    if sidecar.exists() and sidecar.read_text(encoding="ascii") != sidecar_text:
        raise ValueError(f"immutable manifest hash conflict: {sidecar}")
    sidecar.write_text(sidecar_text, encoding="ascii")
    print(json.dumps({"manifest": str(receipt), **result["summary"]}, ensure_ascii=False))
    return 0 if args.allow_unresolved or not unresolved else 1


if __name__ == "__main__":
    raise SystemExit(main())
