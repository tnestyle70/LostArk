#!/usr/bin/env python3
"""Export unresolved UE3 materials and recover their parameter/resource bindings."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any


PARENT_RE = re.compile(r"^Parent\s*=\s*\w+'([^']+)'$")
VALUE_RE = re.compile(r"^ParameterValue\s*=\s*(.*)$")
NAME_RE = re.compile(r"^ParameterName\s*=\s*(.*)$")
TEXTURE_RE = re.compile(r"\w+'([^']+)'$")
VECTOR_RE = re.compile(
    r"\{\s*R=([^,]+),\s*G=([^,]+),\s*B=([^,]+),\s*A=([^}]+)\s*\}"
)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def load_inventory(path: Path) -> dict[str, str]:
    with path.open("r", encoding="utf-8-sig", newline="") as source:
        return {
            str(row["logical_name"]).casefold(): str(row["physical_file"])
            for row in csv.DictReader(source)
            if row.get("logical_name") and row.get("physical_file")
        }


def parse_props(text: str) -> dict[str, Any]:
    parent = None
    parameters: dict[str, list[dict[str, Any]]] = {
        "scalar": [],
        "texture": [],
        "vector": [],
    }
    parameter_type: str | None = None
    pending_value: str | None = None
    for raw_line in text.splitlines():
        line = raw_line.strip()
        parent_match = PARENT_RE.match(line)
        if parent_match:
            parent = parent_match.group(1)
            continue
        for prefix, kind in (
            ("ScalarParameterValues[", "scalar"),
            ("TextureParameterValues[", "texture"),
            ("VectorParameterValues[", "vector"),
        ):
            if line.startswith(prefix):
                parameter_type = kind
                pending_value = None
                break
        value_match = VALUE_RE.match(line)
        if value_match and parameter_type:
            pending_value = value_match.group(1).strip()
            continue
        name_match = NAME_RE.match(line)
        if not name_match or not parameter_type or pending_value is None:
            continue
        name = name_match.group(1).strip()
        if parameter_type == "scalar":
            try:
                value: Any = float(pending_value)
            except ValueError:
                value = pending_value
        elif parameter_type == "texture":
            texture_match = TEXTURE_RE.search(pending_value)
            value = texture_match.group(1) if texture_match else pending_value
        else:
            vector_match = VECTOR_RE.search(pending_value)
            value = (
                {
                    "r": float(vector_match.group(1)),
                    "g": float(vector_match.group(2)),
                    "b": float(vector_match.group(3)),
                    "a": float(vector_match.group(4)),
                }
                if vector_match else pending_value
            )
        parameters[parameter_type].append({"name": name, "value": value})
        pending_value = None
    return {"parent": parent, **parameters}


def texture_assets(root: Path) -> dict[str, list[dict[str, Any]]]:
    rows: dict[str, list[dict[str, Any]]] = {}
    for path in sorted(root.rglob("*"), key=lambda item: item.as_posix().casefold()):
        if not path.is_file() or path.suffix.casefold() not in {".dds", ".tga"}:
            continue
        relative = path.relative_to(root)
        if len(relative.parts) < 2:
            continue
        logical_package = relative.parts[0]
        row = {
            "objectName": path.stem,
            "texturePath": f"{logical_package.lower()}.{path.stem}",
            "relativeFile": relative.as_posix(),
            "byteSize": path.stat().st_size,
            "sha256": sha256_file(path),
        }
        rows.setdefault(path.stem.casefold(), []).append(row)
    return rows


def build_candidate(
    source_path: str,
    physical_package: str,
    output_root: Path,
    umodel_log: str = "",
    inventory: dict[str, str] | None = None,
) -> dict[str, Any]:
    object_name = source_path.rsplit(".", 1)[-1]
    props_matches = sorted(output_root.rglob(f"{object_name}.props.txt"))
    if len(props_matches) != 1:
        if "decalmaterial" in umodel_log.casefold():
            return {
                "material_path": source_path,
                "object_name": object_name,
                "source_file": physical_package,
                "resolutionStatus": "UNSUPPORTED_DECAL_MATERIAL",
                "propsCandidateCount": len(props_matches),
            }
        return {
            "material_path": source_path,
            "object_name": object_name,
            "source_file": physical_package,
            "resolutionStatus": "MISSING_OR_AMBIGUOUS_PROPS",
            "propsCandidateCount": len(props_matches),
        }
    props = parse_props(props_matches[0].read_text(encoding="utf-8-sig"))
    textures_by_name = texture_assets(output_root)
    parent_source_file = None
    normalized_parent = props["parent"]
    if props["parent"]:
        parent_name = str(props["parent"]).rsplit(".", 1)[-1]
        parent_matches = sorted(output_root.rglob(f"{parent_name}.mat"))
        instance_matches = [
            path for path in parent_matches
            if path.parent.name.casefold() == "materialinstanceconstant"
        ]
        selected_parent = (
            instance_matches[0] if len(instance_matches) == 1
            else parent_matches[0] if len(parent_matches) == 1 else None
        )
        if selected_parent is not None:
            parent_relative = selected_parent.relative_to(output_root)
            parent_logical = parent_relative.parts[0] if parent_relative.parts else ""
            parent_source_file = (inventory or {}).get(parent_logical.casefold())
            if parent_source_file is None and parent_logical.casefold() \
                    == source_path.split(".", 1)[0].casefold():
                parent_source_file = physical_package
            if parent_logical and not str(props["parent"]).casefold().startswith(
                parent_logical.casefold() + "."
            ):
                normalized_parent = f"{parent_logical.lower()}.{props['parent']}"
    textures = []
    used = set()
    for parameter in props["texture"]:
        parameter_object_name = str(parameter["value"]).rsplit(".", 1)[-1]
        matches = textures_by_name.get(parameter_object_name.casefold(), [])
        if len(matches) == 1:
            texture_path = matches[0]["texturePath"]
            used.add(texture_path.casefold())
            textures.append({"name": parameter["name"], "texture": texture_path})
        else:
            textures.append(
                {
                    "name": parameter["name"],
                    "texture": None,
                    "sourceObjectName": parameter["value"],
                    "resolutionStatus": "MISSING_OR_AMBIGUOUS_EXPORTED_TEXTURE",
                    "candidateCount": len(matches),
                }
            )
    for rows in textures_by_name.values():
        for row in rows:
            if row["texturePath"].casefold() not in used:
                textures.append(
                    {"name": "umodel_dependency", "texture": row["texturePath"]}
                )
    return {
        "material_path": source_path,
        "object_name": object_name,
        "class": "materialinstanceconstant",
        "source_file": physical_package,
        "parent": normalized_parent,
        "parent_source_file": parent_source_file,
        "textures": textures,
        "scalars": props["scalar"],
        "vectors": props["vector"],
        "resolutionStatus": "RESOLVED_UMODEL_EXPORT",
        "propsFile": props_matches[0].relative_to(output_root).as_posix(),
        "exportedTextures": [
            row for rows in textures_by_name.values() for row in rows
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--catalog", required=True, type=Path)
    parser.add_argument("--inventory-csv", required=True, type=Path)
    parser.add_argument("--umodel", required=True, type=Path)
    parser.add_argument("--package-root", required=True, type=Path)
    parser.add_argument("--output-root", required=True, type=Path)
    parser.add_argument("--output-map", required=True, type=Path)
    parser.add_argument("--receipt", required=True, type=Path)
    parser.add_argument("--region", default="kr")
    args = parser.parse_args()

    catalog = json.loads(args.catalog.read_text(encoding="utf-8-sig"))
    inventory = load_inventory(args.inventory_csv)
    unresolved = [
        row for row in catalog.get("unresolvedMaterialBindings", [])
        if row.get("sourceMaterialPath")
        and str(row.get("sourceMaterialPath")).casefold()
        != "enginematerials.defaultparticle"
    ]
    args.output_root.mkdir(parents=True, exist_ok=True)
    candidates = []
    invocations = []
    failures = []
    for row in unresolved:
        source_path = str(row["sourceMaterialPath"])
        logical_package = source_path.split(".", 1)[0]
        physical = inventory.get(logical_package.casefold())
        object_name = source_path.rsplit(".", 1)[-1]
        asset_key = hashlib.sha256(source_path.encode("utf-8")).hexdigest()[:16]
        output = args.output_root / asset_key
        output.mkdir(parents=True, exist_ok=True)
        if physical is None:
            failure = {
                "sourceMaterialPath": source_path,
                "status": "UNRESOLVED_LOGICAL_PACKAGE",
            }
            failures.append(failure)
            candidates.append(failure)
            continue
        command = [
            str(args.umodel),
            "-export",
            "-game=lostark",
            f"-{args.region}",
            "-nameresolve",
            f"-path={args.package_root}",
            f"-out={output}",
            "-dds",
            "-nooverwrite",
            f"-obj={object_name}",
            logical_package,
        ]
        completed = subprocess.run(
            command,
            cwd=args.umodel.parent,
            text=True,
            encoding="utf-8",
            errors="replace",
            capture_output=True,
            check=False,
            creationflags=(
                subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0
            ),
        )
        log = output / "umodel.log"
        log.write_text(completed.stdout + "\n" + completed.stderr, encoding="utf-8")
        umodel_log = completed.stdout + "\n" + completed.stderr
        candidate = build_candidate(
            source_path, physical, output, umodel_log, inventory
        )
        candidate["exportRoot"] = output.relative_to(args.output_root).as_posix()
        candidate["umodelExitCode"] = completed.returncode
        candidates.append(candidate)
        invocation = {
            "sourceMaterialPath": source_path,
            "logicalPackage": logical_package,
            "physicalPackage": physical,
            "exitCode": completed.returncode,
            "resolutionStatus": candidate.get("resolutionStatus"),
        }
        invocations.append(invocation)
        if completed.returncode != 0 or candidate.get("resolutionStatus") \
                not in {"RESOLVED_UMODEL_EXPORT", "UNSUPPORTED_DECAL_MATERIAL"}:
            failures.append(invocation)

    material_map: dict[str, list[dict[str, Any]]] = {}
    for candidate in candidates:
        if candidate.get("resolutionStatus") != "RESOLVED_UMODEL_EXPORT":
            continue
        keys = {
            str(candidate["material_path"]).casefold(),
            str(candidate["object_name"]).casefold(),
        }
        parts = str(candidate["material_path"]).split(".")
        if len(parts) > 1:
            keys.add(".".join(parts[1:]).casefold())
        clean = {
            key: value for key, value in candidate.items()
            if key not in {"resolutionStatus", "exportedTextures", "exportRoot", "umodelExitCode"}
        }
        for key in keys:
            material_map.setdefault(key, []).append(clean)
    args.output_map.parent.mkdir(parents=True, exist_ok=True)
    args.output_map.write_text(
        json.dumps({"materials": material_map}, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    receipt = {
        "schema": "lostark.umodel-material-dependency-receipt",
        "formatVersion": 1,
        "sourceCatalog": args.catalog.as_posix(),
        "sourceCatalogSha256": sha256_file(args.catalog),
        "candidates": candidates,
        "invocations": invocations,
        "failures": failures,
        "summary": {
            "requestedMaterialCount": len(unresolved),
            "resolvedMaterialCount": sum(
                row.get("resolutionStatus") == "RESOLVED_UMODEL_EXPORT"
                for row in candidates
            ),
            "unsupportedMaterialCount": sum(
                row.get("resolutionStatus") == "UNSUPPORTED_DECAL_MATERIAL"
                for row in candidates
            ),
            "failureCount": len(failures),
            "exportedTextureCount": sum(
                len(row.get("exportedTextures", [])) for row in candidates
            ),
        },
    }
    args.receipt.parent.mkdir(parents=True, exist_ok=True)
    args.receipt.write_text(
        json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(receipt["summary"], ensure_ascii=False, sort_keys=True))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
