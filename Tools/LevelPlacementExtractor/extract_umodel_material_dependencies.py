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
STATIC_VALUE_RE = re.compile(r"^(?:ParameterValue|Value)\s*=\s*(.*)$")
NAME_RE = re.compile(r"^ParameterName\s*=\s*(.*)$")
TEXTURE_RE = re.compile(r"\w+'([^']+)'$")
VECTOR_RE = re.compile(
    r"\{\s*R=([^,]+),\s*G=([^,]+),\s*B=([^,]+),\s*A=([^}]+)\s*\}"
)
TEXTURE_REFERENCE_RE = re.compile(
    r"(?:Texture2D|TextureCube|Texture)'([^']+)'", re.IGNORECASE
)
EXPRESSION_ENTRY_RE = re.compile(r"^Expressions\[\d+\]\s*=\s*(.*)$")


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
        "static_switch": [],
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
            ("StaticSwitchParameters[", "static_switch"),
        ):
            if line.startswith(prefix):
                parameter_type = kind
                pending_value = None
                break
        value_match = (
            STATIC_VALUE_RE.match(line)
            if parameter_type == "static_switch"
            else VALUE_RE.match(line)
        )
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
        elif parameter_type == "vector":
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
        else:
            normalized = pending_value.casefold()
            value = normalized in {"true", "1"}
        parameters[parameter_type].append({"name": name, "value": value})
        pending_value = None
    return {"parent": parent, **parameters}


def parse_material_dump(text: str) -> dict[str, Any]:
    """Recover only explicit UModel Material evidence; never infer graph links."""
    folded = text.casefold()

    def collected_entries(collection: str) -> list[dict[str, str]]:
        """Read UModel's flattened parent parameter defaults and groups.

        These are parent parameter semantics evidence only.  They do not
        disclose a Material expression connection or evaluation order.
        """
        entry_re = re.compile(
            rf"^\s*{re.escape(collection)}\[\d+\]\s*=\s*(.*?)"
            rf"(?=^\s*{re.escape(collection)}\[\d+\]\s*=|"
            r"^\s*Collected[A-Za-z]+Parameters\[\d+\]\s*=|\Z)",
            re.IGNORECASE | re.MULTILINE | re.DOTALL,
        )
        field_re = re.compile(
            r"(?:^|[,\{\r\n])\s*(Value|Name|Group)\s*=\s*(.*?)"
            r"(?=(?:,\s*|\r?\n\s*)(?:Value|Name|Group)\s*=|\s*\}|\Z)",
            re.IGNORECASE | re.DOTALL,
        )
        result = []
        for match in entry_re.finditer(text):
            fields = {
                field.group(1).casefold(): field.group(2).strip().rstrip(",")
                for field in field_re.finditer(match.group(1))
            }
            if fields.get("name") and fields.get("group") and "value" in fields:
                fields["name"] = fields["name"].strip().strip("{}").strip()
                fields["group"] = fields["group"].strip().strip("{}").strip()
                result.append(fields)
        return result

    def scalar_value(value: str) -> float | None:
        match = re.match(
            r"\s*([-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?)",
            value,
        )
        return float(match.group(1)) if match else None

    def vector_value(value: str) -> dict[str, float] | None:
        normalized = value.strip()
        # The generic field scanner stops at the vector's closing brace.
        if normalized.startswith("{") and not normalized.endswith("}"):
            normalized += " }"
        match = VECTOR_RE.search(normalized)
        if not match:
            return None
        return {
            "r": float(match.group(1)),
            "g": float(match.group(2)),
            "b": float(match.group(3)),
            "a": float(match.group(4)),
        }

    def token(name: str) -> str | None:
        match = re.search(
            rf"^\s*{re.escape(name)}\s*=\s*([^\s]+)"
            rf"(?:\s+\([^)]*\))?\s*$",
            text,
            re.IGNORECASE | re.MULTILINE,
        )
        return match.group(1) if match else None

    def boolean(name: str) -> bool | None:
        value = token(name)
        if value is None:
            return None
        normalized = value.casefold()
        if normalized in {"true", "1"}:
            return True
        if normalized in {"false", "0"}:
            return False
        return None

    referenced_textures = []
    seen_textures = set()
    for match in TEXTURE_REFERENCE_RE.finditer(text):
        texture = match.group(1)
        key = texture.casefold()
        if key in seen_textures:
            continue
        seen_textures.add(key)
        referenced_textures.append(texture)

    expression_count = 0
    expression_non_null_count = 0
    for raw_line in text.splitlines():
        match = EXPRESSION_ENTRY_RE.match(raw_line.strip())
        if not match:
            continue
        value = match.group(1).strip().casefold()
        if not value:
            # UModel prints ``Expressions[N] =`` as the collection header.
            # It is capacity metadata, not an expression entry.
            continue
        expression_count += 1
        if value not in {"0", "none", "null", "<null>"}:
            expression_non_null_count += 1

    collected_texture_parameters = []
    for entry in re.finditer(
        r"CollectedTextureParameters\[\d+\]\s*=\s*\{([^{}]*)\}",
        text,
        re.IGNORECASE | re.DOTALL,
    ):
        fields = {}
        for field in re.finditer(
            r"(?:^|[,\r\n])\s*(Texture|Name|Group)\s*=\s*(.*?)"
            r"(?=(?:,\s*|\r?\n\s*)(?:Texture|Name|Group)\s*=|$)",
            entry.group(1),
            re.IGNORECASE | re.DOTALL,
        ):
            fields[field.group(1).casefold()] = field.group(2).strip()
        texture_value = fields.get("texture")
        texture_match = (
            TEXTURE_RE.search(texture_value) if texture_value else None
        )
        if not texture_match or not fields.get("name") or not fields.get("group"):
            continue
        collected_texture_parameters.append(
            {
                "texture": texture_match.group(1),
                "name": fields["name"],
                "group": fields["group"],
            }
        )

    collected_scalar_parameters = []
    for fields in collected_entries("CollectedScalarParameters"):
        value = scalar_value(fields["value"])
        if value is not None:
            collected_scalar_parameters.append({
                "name": fields["name"], "group": fields["group"],
                "value": value,
            })
    collected_vector_parameters = []
    for fields in collected_entries("CollectedVectorParameters"):
        value = vector_value(fields["value"])
        if value is not None:
            collected_vector_parameters.append({
                "name": fields["name"], "group": fields["group"],
                "value": value,
            })
    collected_static_switch_parameters = []
    for fields in collected_entries("CollectedStaticSwitchParameters"):
        value = fields["value"].casefold()
        if value in {"true", "false", "1", "0"}:
            collected_static_switch_parameters.append({
                "name": fields["name"], "group": fields["group"],
                "value": value in {"true", "1"},
            })

    parameters = parse_props(text)
    return {
        "renderState": {
            "blendMode": token("BlendMode"),
            "lightingModel": token("LightingModel"),
            "twoSided": boolean("TwoSided"),
            "disableDepthTest": boolean("bDisableDepthTest"),
            "usesDistortion": (
                boolean("bUsesDistortion")
                if "busesdistortion" in folded
                else boolean("UsesDistortion")
            ),
        },
        "referencedTextures": referenced_textures,
        "collectedTextureParameters": collected_texture_parameters,
        "collectedScalarParameters": collected_scalar_parameters,
        "collectedVectorParameters": collected_vector_parameters,
        "collectedStaticSwitchParameters": collected_static_switch_parameters,
        "scalars": parameters["scalar"],
        "vectors": parameters["vector"],
        "staticSwitches": parameters["static_switch"],
        "expressionCoverage": {
            "entryCount": expression_count,
            "nonNullCount": expression_non_null_count,
            "nullCount": expression_count - expression_non_null_count,
            "topologyStatus": (
                "NO_EXPRESSION_ENTRIES"
                if expression_count == 0
                else "PARTIAL_OR_COOKED_STRIPPED"
                if expression_non_null_count < expression_count
                else "NON_NULL_ENTRIES_PRESENT"
            ),
        },
    }


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


def resolve_material_props_chain(
    output_root: Path,
    instance_props: Path,
    parent_path: str | None,
) -> tuple[list[Path], int]:
    """Resolve leaf -> intermediate MI -> terminal Material props.

    Every MaterialInstanceConstant in the chain owns overrides.  Returning the
    full chain prevents a leaf-only parser from silently dropping values set by
    an intermediate instance.
    """
    if not parent_path:
        return (
            ([instance_props], 1)
            if instance_props.parent.name.casefold() in {"material", "material3"}
            else ([], 0)
        )

    visited = {instance_props.resolve()}
    chain = [instance_props]
    current_parent = str(parent_path)
    while current_parent:
        parent_name = current_parent.rsplit(".", 1)[-1]
        parent_package = current_parent.split(".", 1)[0].casefold()
        matches = [
            path
            for path in sorted(
                output_root.rglob(f"{parent_name}.props.txt"),
                key=lambda item: item.as_posix().casefold(),
            )
            if path.resolve() not in visited
        ]
        package_matches = [
            path for path in matches
            if path.relative_to(output_root).parts
            and path.relative_to(output_root).parts[0].casefold()
            == parent_package
        ]
        candidates = package_matches or matches
        if len(candidates) != 1:
            return [], len(candidates)
        selected = candidates[0]
        resolved = selected.resolve()
        if resolved in visited:
            return [], 0
        visited.add(resolved)
        chain.append(selected)
        if selected.parent.name.casefold() in {"material", "material3"}:
            return chain, 1
        if selected.parent.name.casefold() != "materialinstanceconstant":
            return [], 1
        current_props = parse_props(selected.read_text(encoding="utf-8-sig"))
        current_parent = str(current_props["parent"] or "")
    return [], 0


def select_material_evidence_props(
    output_root: Path,
    instance_props: Path,
    parent_path: str | None,
) -> tuple[Path | None, int]:
    chain, count = resolve_material_props_chain(
        output_root, instance_props, parent_path
    )
    return (chain[-1] if chain else None), count


def merge_instance_parameter_chain(
    chain: list[Path],
) -> dict[str, list[dict[str, Any]]]:
    """Merge intermediate-to-leaf MI overrides by parameter name."""
    result: dict[str, list[dict[str, Any]]] = {
        "scalar": [], "texture": [], "vector": [], "static_switch": [],
    }
    by_name: dict[str, dict[str, dict[str, Any]]] = {
        kind: {} for kind in result
    }
    instance_paths = [
        path for path in chain
        if path.parent.name.casefold() == "materialinstanceconstant"
    ]
    for path in reversed(instance_paths):
        parsed = parse_props(path.read_text(encoding="utf-8-sig"))
        for kind in result:
            for parameter in parsed[kind]:
                key = str(parameter.get("name") or "").casefold()
                if not key:
                    continue
                if key in by_name[kind]:
                    by_name[kind][key]["value"] = parameter.get("value")
                    continue
                row = dict(parameter)
                result[kind].append(row)
                by_name[kind][key] = row
    return result


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
    props_file = props_matches[0]
    props = parse_props(props_file.read_text(encoding="utf-8-sig"))
    props_chain, evidence_candidate_count = resolve_material_props_chain(
        output_root, props_file, props["parent"]
    )
    effective_parameters = (
        merge_instance_parameter_chain(props_chain)
        if props_chain else {
            "scalar": props["scalar"],
            "texture": props["texture"],
            "vector": props["vector"],
            "static_switch": props["static_switch"],
        }
    )
    textures_by_name = texture_assets(output_root)
    parent_source_file = None
    normalized_parent = props["parent"]
    if len(props_chain) >= 2:
        terminal_instance_paths = [
            path for path in props_chain[:-1]
            if path.parent.name.casefold() == "materialinstanceconstant"
        ]
        if terminal_instance_paths:
            terminal_props = parse_props(
                terminal_instance_paths[-1].read_text(encoding="utf-8-sig")
            )
            normalized_parent = terminal_props["parent"] or normalized_parent
    if props["parent"]:
        parent_logical_name = str(props["parent"]).split(".", 1)[0]
        parent_source_file = (inventory or {}).get(
            parent_logical_name.casefold()
        )
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
            parent_source_file = parent_source_file or (
                inventory or {}
            ).get(parent_logical.casefold())
            if parent_source_file is None and parent_logical.casefold() \
                    == source_path.split(".", 1)[0].casefold():
                parent_source_file = physical_package
            if parent_logical and not str(props["parent"]).casefold().startswith(
                parent_logical.casefold() + "."
            ):
                normalized_parent = f"{parent_logical.lower()}.{props['parent']}"
    if len(props_chain) >= 2:
        terminal_props_path = props_chain[-1]
        terminal_instance_path = next(
            (
                path for path in reversed(props_chain[:-1])
                if path.parent.name.casefold() == "materialinstanceconstant"
            ),
            None,
        )
        if terminal_instance_path is not None:
            terminal_props = parse_props(
                terminal_instance_path.read_text(encoding="utf-8-sig")
            )
            terminal_parent = str(terminal_props["parent"] or "")
            terminal_relative = terminal_props_path.relative_to(output_root)
            terminal_package = (
                terminal_relative.parts[0] if terminal_relative.parts else ""
            )
            if terminal_package and not terminal_parent.casefold().startswith(
                terminal_package.casefold() + "."
            ):
                terminal_parent = f"{terminal_package.lower()}.{terminal_parent}"
            normalized_parent = terminal_parent or normalized_parent
            terminal_logical = normalized_parent.split(".", 1)[0]
            parent_source_file = (inventory or {}).get(
                terminal_logical.casefold(), parent_source_file
            )
    textures = []
    used = set()
    for parameter in effective_parameters["texture"]:
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
    evidence_props = props_chain[-1] if props_chain else None
    material_evidence = parse_material_dump(
        evidence_props.read_text(encoding="utf-8-sig")
        if evidence_props is not None else ""
    )
    return {
        "material_path": source_path,
        "object_name": object_name,
        "class": "materialinstanceconstant",
        "source_file": physical_package,
        "parent": normalized_parent,
        "parent_source_file": parent_source_file,
        "textures": textures,
        "scalars": effective_parameters["scalar"],
        "vectors": effective_parameters["vector"],
        "static_switches": effective_parameters["static_switch"],
        "materialEvidence": material_evidence,
        "materialEvidenceStatus": (
            "SOURCE_MATERIAL_PROPS"
            if evidence_props is not None else
            "MISSING_OR_AMBIGUOUS_SOURCE_MATERIAL_PROPS"
        ),
        "materialEvidencePropsFile": (
            evidence_props.relative_to(output_root).as_posix()
            if evidence_props is not None else None
        ),
        "materialEvidencePropsSha256": (
            sha256_file(evidence_props) if evidence_props is not None else None
        ),
        "materialEvidencePropsCandidateCount": evidence_candidate_count,
        "materialInstanceChain": [
            {
                "propsFile": path.relative_to(output_root).as_posix(),
                "sha256": sha256_file(path),
            }
            for path in props_chain
            if path.parent.name.casefold() == "materialinstanceconstant"
        ],
        "resolutionStatus": "RESOLVED_UMODEL_EXPORT",
        "propsFile": props_file.relative_to(output_root).as_posix(),
        "propsFileSha256": sha256_file(props_file),
        "exportedTextures": [
            row for rows in textures_by_name.values() for row in rows
        ],
    }


def build_direct_material_dump_candidate(
    source_path: str,
    physical_package: str,
    dump_path: Path,
    dump_text: str,
) -> dict[str, Any] | None:
    """Build explicit evidence for a direct Material3 with no export props."""
    object_name = source_path.rsplit(".", 1)[-1]
    object_rows = re.findall(
        r"^ClassName:\s*(\w+)\s+ObjectName:\s*(\S+)\s*$",
        dump_text,
        re.IGNORECASE | re.MULTILINE,
    )
    exact_rows = [
        row for row in object_rows
        if row[0].casefold() in {"material", "material3"}
        and row[1].casefold() == object_name.casefold()
    ]
    if len(exact_rows) != 1 or len(object_rows) != 1 or not dump_path.is_file():
        return None
    evidence = parse_material_dump(dump_text)
    scalars = [
        {"name": row["name"], "value": row["value"]}
        for row in evidence["collectedScalarParameters"]
    ]
    vectors = [
        {"name": row["name"], "value": row["value"]}
        for row in evidence["collectedVectorParameters"]
    ]
    static_switches = [
        {"name": row["name"], "value": row["value"]}
        for row in evidence["collectedStaticSwitchParameters"]
    ]
    textures = [
        {"name": row["name"], "texture": row["texture"]}
        for row in evidence["collectedTextureParameters"]
    ]
    collected_paths = {
        row["texture"].casefold() for row in textures if row.get("texture")
    }
    textures.extend(
        {"name": "umodel_dependency", "texture": texture}
        for texture in evidence["referencedTextures"]
        if texture.casefold() not in collected_paths
    )
    relative_dump = dump_path.name
    dump_hash = sha256_file(dump_path)
    return {
        "material_path": source_path,
        "object_name": object_name,
        "class": "material",
        "source_file": physical_package,
        "parent": source_path,
        "parent_source_file": physical_package,
        "textures": textures,
        "scalars": scalars,
        "vectors": vectors,
        "static_switches": static_switches,
        "materialEvidence": evidence,
        "materialEvidenceStatus": "SOURCE_MATERIAL_DUMP",
        "materialEvidenceFile": relative_dump,
        "materialEvidenceSha256": dump_hash,
        "materialEvidencePropsCandidateCount": 1,
        "sourceEvidenceFile": relative_dump,
        "sourceEvidenceSha256": dump_hash,
        "resolutionStatus": "RESOLVED_UMODEL_DUMP",
        "propsFile": None,
        "propsFileSha256": None,
        "exportedTextures": [],
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
        dump_exit_code = None
        if candidate.get("resolutionStatus") == "MISSING_OR_AMBIGUOUS_PROPS" \
                and int(candidate.get("propsCandidateCount", 0)) == 0:
            dump_path = output / f"{object_name}.umodel-dump.txt"
            dump_path.unlink(missing_ok=True)
            dump_command = [
                str(args.umodel),
                "-dump",
                "-game=lostark",
                f"-{args.region}",
                "-nameresolve",
                f"-path={args.package_root}",
                f"-log={dump_path}",
                f"-obj={object_name}",
                logical_package,
            ]
            dump_completed = subprocess.run(
                dump_command,
                cwd=args.umodel.parent,
                text=True,
                encoding="utf-8",
                errors="replace",
                capture_output=True,
                check=False,
                creationflags=(
                    subprocess.CREATE_NO_WINDOW
                    if sys.platform == "win32" else 0
                ),
            )
            dump_exit_code = dump_completed.returncode
            dump_text = (
                dump_path.read_text(encoding="utf-8-sig", errors="replace")
                if dump_path.is_file()
                else dump_completed.stdout + "\n" + dump_completed.stderr
            )
            dump_candidate = build_direct_material_dump_candidate(
                source_path, physical, dump_path, dump_text
            )
            if dump_completed.returncode == 0 and dump_candidate is not None:
                candidate = dump_candidate
                relative_dump = dump_path.relative_to(
                    args.output_root
                ).as_posix()
                candidate["materialEvidenceFile"] = relative_dump
                candidate["sourceEvidenceFile"] = relative_dump
        candidate["exportRoot"] = output.relative_to(args.output_root).as_posix()
        candidate["umodelExitCode"] = completed.returncode
        if dump_exit_code is not None:
            candidate["umodelDumpExitCode"] = dump_exit_code
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
                not in {
                    "RESOLVED_UMODEL_EXPORT", "RESOLVED_UMODEL_DUMP",
                    "UNSUPPORTED_DECAL_MATERIAL",
                }:
            failures.append(invocation)

    material_map: dict[str, list[dict[str, Any]]] = {}
    for candidate in candidates:
        if candidate.get("resolutionStatus") not in {
            "RESOLVED_UMODEL_EXPORT", "RESOLVED_UMODEL_DUMP"
        }:
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
            if key not in {
                "resolutionStatus", "exportedTextures", "exportRoot",
                "umodelExitCode", "umodelDumpExitCode",
            }
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
                row.get("resolutionStatus") in {
                    "RESOLVED_UMODEL_EXPORT", "RESOLVED_UMODEL_DUMP"
                }
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
