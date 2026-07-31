#!/usr/bin/env python3
"""Extract Lost Ark action/particle binding hints from raw UPK packages.

This script reuses the UPK parser/decompress logic from
extract_ue3_placements.py and only scans for effect-related classes:

- efaction* classes (notify / condition / particle data)
- skeletalmeshsocket / animsequence
- particlesystem / particle system component
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

import extract_ue3_placements as ue3


DEFAULT_TARGET_CLASSES = {
    "efactionobject",
    "efactionstage",
    "efactionnotify_playparticleeffect",
    "efactionnotify_defaultparticle",
    "efactionnotify_condition",
    "efactionnotify_anim",
    "efactionparticledata",
    "efparticledata",
    "efactionconditionspawnanimindex",
    "efactionconditiondespawnanimindex",
    "animnotify_trails",
    "efanimnotify_trails",
    "skeletalmeshsocket",
    "animsequence",
    "particlesystem",
    "particlesystemcomponent",
}

# Cooked packages have no bare `particlemodule` or `particleemitter` class; the
# concrete names are particlemodulerequired, particlespriteemitter and so on.
# Matching those by prefix keeps them reachable without listing all 90+ names.
DEFAULT_TARGET_PREFIXES = (
    "particlemodule",
    "efparticlemodule",
    "particlespriteemitter",
    "particlemeshemitter",
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--package-list",
        type=Path,
        help="Optional text file, one package path per line.",
    )
    parser.add_argument(
        "--class-filter",
        nargs="*",
        default=None,
        help="Extra class names to scan (case-insensitive).",
    )
    parser.add_argument(
        "--class-prefix",
        nargs="*",
        default=None,
        help="Extra class name prefixes to scan (case-insensitive).",
    )
    parser.add_argument(
        "--skip-empty",
        action="store_true",
        help="Do not write a per-package file when nothing matched.",
    )
    parser.add_argument(
        "--max-packages",
        type=int,
        default=0,
        help="Limit for test run; 0 means no limit.",
    )
    parser.add_argument(
        "--aes-key",
        default=ue3.LOSTARK_KR_AES_KEY,
        help="AES-256 key for decrypted chunks.",
    )
    return parser.parse_args()


def _normalize_property_value(
    item: Any,
    imports: list[ue3.ImportEntry],
    exports: list[ue3.ExportEntry],
) -> Any:
    if not isinstance(item, dict):
        return item

    value = item.get("value")
    kind = item.get("type", "").casefold()
    if kind not in ("objectproperty", "componentproperty", "interfaceproperty"):
        return item

    if isinstance(value, int) and value != 0:
        return {
            "ref": value,
            "resolvedObject": ue3.package_ref_name(value, imports, exports),
            "resolvedPath": ue3.package_ref_path(value, imports, exports),
        }
    return value


def _iter_packages(package_root: Path, package_list: Path | None) -> list[Path]:
    if package_list is not None:
        # utf-8-sig: Windows PowerShell 5.1 Set-Content -Encoding UTF8 writes a
        # BOM, which would otherwise corrupt the first path and drop it silently.
        lines = [line.strip() for line in package_list.read_text(encoding="utf-8-sig").splitlines()]
        resolved: list[Path] = []
        for line in lines:
            if not line:
                continue
            path = Path(line)
            if not path.is_absolute():
                path = package_root / path
            if path.exists():
                resolved.append(path)
        return resolved

    return sorted(
        {
            p for p in package_root.rglob("*.upk") if p.is_file()
        }
    )


def extract_package(
    package_path: Path,
    classes: set[str],
    prefixes: tuple[str, ...],
    aes_key: str,
) -> tuple[dict[str, Any], list[dict[str, Any]], list[dict[str, Any]]]:
    physical = package_path.read_bytes()
    summary = ue3.parse_summary(physical)
    logical = ue3.decompress_package(physical, summary, aes_key)
    names = ue3.parse_name_table(logical, summary)
    imports = ue3.parse_import_table(logical, summary, names)
    exports = ue3.parse_export_table(logical, summary, names)

    matched: list[dict[str, Any]] = []
    property_errors: list[dict[str, Any]] = []

    for entry in exports:
        class_name = ue3.package_ref_name(entry.class_index, imports, exports).casefold()
        if entry.serial_size <= 0:
            continue
        if class_name not in classes and not class_name.startswith(prefixes):
            continue

        serial = logical[entry.serial_offset : entry.serial_offset + entry.serial_size]
        props: dict[str, Any] | None = None
        try:
            parsed, _next = ue3.parse_tagged_properties(serial, names, summary.version)
            props = {
                key: _normalize_property_value(value, imports, exports)
                for key, value in parsed.items()
            }
        except Exception as error:
            # Not only ExtractionError: a single malformed StrProperty raises
            # UnicodeDecodeError out of the shared reader, and catching just
            # ExtractionError would abort the whole package over one object.
            property_errors.append(
                {
                    "exportIndex": entry.index,
                    "class": class_name,
                    "object": entry.object_name,
                    "error": f"{type(error).__name__}: {error}",
                }
            )

        outer_index = entry.package_index
        matched.append(
            {
                "exportIndex": entry.index,
                "class": class_name,
                "objectName": entry.object_name,
                "outerRef": outer_index,
                "outerName": ue3.package_ref_name(outer_index, imports, exports)
                if outer_index != 0
                else None,
                # package_ref_* takes a 1-based package reference, while
                # ExportEntry.index is 0-based.  Passing the raw index resolves
                # to the previous export and yields a plausible but wrong path.
                "objectPath": ue3.package_ref_path(entry.index + 1, imports, exports),
                "serialSize": entry.serial_size,
                "serialOffset": entry.serial_offset,
                "properties": props,
            }
        )

    return {
        "schemaVersion": 1,
        "package": str(package_path),
        "logicalName": package_path.stem,
        "packageVersion": summary.version,
        "nameCount": summary.name_count,
        "importCount": summary.import_count,
        "exportCount": summary.export_count,
        "matchedCount": len(matched),
        "summary": {
            "propertyErrorCount": len(property_errors),
        },
    }, matched, property_errors


def main() -> int:
    args = parse_args()
    args.output.mkdir(parents=True, exist_ok=True)

    classes = {c.lower() for c in DEFAULT_TARGET_CLASSES}
    if args.class_filter:
        classes.update(c.lower() for c in args.class_filter)
    prefixes = tuple(p.lower() for p in DEFAULT_TARGET_PREFIXES)
    if args.class_prefix:
        prefixes += tuple(p.lower() for p in args.class_prefix)

    package_paths = _iter_packages(args.package_root, args.package_list)
    if args.max_packages > 0:
        package_paths = package_paths[: args.max_packages]

    manifest: dict[str, Any] = {
        "schemaVersion": 1,
        "sourceRoot": str(args.package_root),
        "classFilter": sorted(classes),
        "classPrefixFilter": sorted(prefixes),
        "packages": [],
    }

    total_hits = 0
    total_errors = 0
    for package_path in package_paths:
        try:
            source_summary, hits, errors = extract_package(
                package_path=package_path,
                classes=classes,
                prefixes=prefixes,
                aes_key=args.aes_key,
            )
            source_summary["matchedCount"] = len(hits)
            source_summary["summary"]["propertyErrorCount"] = len(errors)
            source_summary["summary"]["matchedCountByClass"] = {}
            for hit in hits:
                source_summary["summary"]["matchedCountByClass"].setdefault(
                    hit["class"], 0
                )
                source_summary["summary"]["matchedCountByClass"][hit["class"]] += 1

            output_path = None
            if hits or not args.skip_empty:
                source_summary["outputFile"] = f"{package_path.name}.effect_actions.json"
                output_path = args.output / source_summary["outputFile"]
                output_payload = {
                    **source_summary,
                    "instances": hits,
                    "propertyErrors": errors,
                }
                output_path.write_text(
                    json.dumps(output_payload, ensure_ascii=False, indent=2),
                    encoding="utf-8",
                )

            total_hits += len(hits)
            total_errors += len(errors)
            manifest["packages"].append(source_summary)

            print(
                json.dumps(
                    {
                        "package": package_path.name,
                        "matched": len(hits),
                        "errors": len(errors),
                        "output": str(output_path) if output_path else None,
                    },
                    ensure_ascii=False,
                )
            )
        except Exception as error:
            manifest["packages"].append(
                {
                    "package": str(package_path),
                    "error": str(error),
                    "matchedCount": 0,
                    "summary": {"propertyErrorCount": 1},
                }
            )
            total_errors += 1

    manifest["summary"] = {
        "packageCount": len(manifest["packages"]),
        "matchedInstanceCount": total_hits,
        "propertyErrorCount": total_errors,
    }
    manifest_path = args.output / "effect_action_manifest.json"
    manifest_path.write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    print(json.dumps({"manifest": str(manifest_path), **manifest["summary"]}, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
