#!/usr/bin/env python3
"""Recover one exact Bern Castle decal material fixture.

This probe deliberately stops before runtime rendering.  It proves the source
DecalComponent, MaterialInstanceConstant parent, texture overrides, parent
DecalMaterial graph inputs and the selected opacity branch, then copies only
the referenced files into a small auditable pack.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable, Sequence


HERE = Path(__file__).resolve().parent
LEVEL_TOOL_DIR = HERE.parent / "LevelPlacementExtractor"
for directory in (LEVEL_TOOL_DIR, HERE):
    if str(directory) not in sys.path:
        sys.path.insert(0, str(directory))

import extract_ue3_placements as ue3  # noqa: E402
import build_bern_castle_nonstatic_manifest as nonstatic  # noqa: E402
import build_bern_castle_assets as static_assets  # noqa: E402


SCHEMA_VERSION = 1
AREA_ID = "LV_BER_BERNCASTLE"
FIXTURE_ID = "BERN_NS_DECAL_0AB42A0D65BB18FF"
INSTANCE_LOGICAL_PACKAGE = "lv_decal_01"
INSTANCE_OBJECT_PATH = "lv_decal_01.mat.lv_common_decal_05_mi"
INSTANCE_LOCAL_PATH = "mat.lv_common_decal_05_mi"
INSTANCE_OBJECT_NAME = "lv_common_decal_05_mi"
PARENT_LOGICAL_PACKAGE = "efbasematerial_lv_prologue"
PARENT_OBJECT_PATH = "efbasematerial_lv_prologue.decals.decal_translucent"
PARENT_LOCAL_PATH = "decals.decal_translucent"
PARENT_OBJECT_NAME = "decal_translucent"
TEXTURE_LOGICAL_PACKAGE = "efmaster_material_prologue"
DIFFUSE_PARAMETER = "texture_diffuse"
DIFFUSE_OBJECT_PATH = "lv_decal_01.tex.lv_common_decal_05_d"
DIFFUSE_OBJECT_NAME = "lv_common_decal_05_d"
OPACITY_PARAMETER = "texture_opacity"
OPACITY_OBJECT_PATH = "efmaster_material_prologue.tex.fx_tex_a_02710"
OPACITY_OBJECT_NAME = "fx_tex_a_02710"
OPACITY_SWITCH = "1.use_opacity_texture"
OPACITY_SWITCH_GUID = "47828ed47a8eb945a3c01ebb779f5a90"
EXPECTED_SWITCH_ENTRY_COUNT = 2


class ProbeError(RuntimeError):
    pass


@dataclass(frozen=True)
class PackageContext:
    logical_name: str
    physical_path: Path
    physical_sha256: str
    summary: Any
    logical: bytes
    names: list[str]
    imports: list[Any]
    exports: list[Any]


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ProbeError(f"could not read JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise ProbeError(f"JSON root must be an object: {path}")
    return value


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest().upper()


def atomic_write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    temporary = Path(temporary_name)
    try:
        with os.fdopen(handle, "w", encoding="utf-8", newline="\n") as output:
            json.dump(value, output, ensure_ascii=False, indent=2)
            output.write("\n")
            output.flush()
            os.fsync(output.fileno())
        os.replace(temporary, path)
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise


def bounded_rmtree(path: Path, root: Path) -> None:
    resolved = path.resolve()
    resolved_root = root.resolve()
    if resolved == resolved_root or resolved_root not in resolved.parents:
        raise ProbeError(f"refusing to remove path outside probe root: {resolved}")
    if resolved.exists():
        shutil.rmtree(resolved)


def creation_flags() -> int:
    return subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0


def run(command: Sequence[str], cwd: Path, timeout: float, label: str) -> str:
    try:
        completed = subprocess.run(
            [str(item) for item in command],
            cwd=cwd,
            text=True,
            encoding="utf-8",
            errors="replace",
            capture_output=True,
            check=False,
            timeout=timeout,
            creationflags=creation_flags(),
        )
    except subprocess.TimeoutExpired as error:
        raise ProbeError(f"{label} timed out after {timeout:g}s") from error
    output = completed.stdout + "\n" + completed.stderr
    if completed.returncode != 0:
        raise ProbeError(f"{label} failed ({completed.returncode})\n{output[-6000:]}")
    if "Error creating file" in output:
        raise ProbeError(f"{label} reported an output creation failure")
    return output


def select_fixture(document: dict[str, Any]) -> dict[str, Any]:
    if document.get("areaId") != AREA_ID:
        raise ProbeError(f"wrong non-static areaId: {document.get('areaId')!r}")
    items = document.get("items")
    if not isinstance(items, list):
        raise ProbeError("non-static manifest items must be an array")
    rows = [row for row in items if isinstance(row, dict) and row.get("id") == FIXTURE_ID]
    if len(rows) != 1:
        raise ProbeError(f"expected one fixture {FIXTURE_ID}, found {len(rows)}")
    fixture = rows[0]
    if fixture.get("type") != "decal":
        raise ProbeError("fixture is not a decal")
    references = fixture.get("references")
    if not isinstance(references, list):
        raise ProbeError("fixture references must be an array")
    material_rows = [
        row for row in references
        if isinstance(row, dict) and row.get("role") == "material"
    ]
    if len(material_rows) != 1 or material_rows[0].get("objectPath") != INSTANCE_OBJECT_PATH:
        raise ProbeError("fixture does not reference the expected exact decal material")
    return fixture


def open_package(
    logical_name: str,
    umodel: Path,
    package_root: Path,
    region: str,
    aes_key: str,
) -> PackageContext:
    physical_path = ue3.resolve_physical_package(
        umodel, package_root, logical_name, region
    )
    physical = physical_path.read_bytes()
    summary = ue3.parse_summary(physical)
    logical = ue3.decompress_package(physical, summary, aes_key)
    names = ue3.parse_name_table(logical, summary)
    imports = ue3.parse_import_table(logical, summary, names)
    exports = ue3.parse_export_table(logical, summary, names)
    return PackageContext(
        logical_name=logical_name,
        physical_path=physical_path,
        physical_sha256=sha256_bytes(physical),
        summary=summary,
        logical=logical,
        names=names,
        imports=imports,
        exports=exports,
    )


def export_path(context: PackageContext, entry: Any) -> str:
    return ue3.package_ref_path(entry.index + 1, context.imports, context.exports) or ""


def export_class(context: PackageContext, entry: Any) -> str:
    return ue3.package_ref_name(entry.class_index, context.imports, context.exports)


def find_export(
    context: PackageContext, local_path: str, expected_class: str
) -> Any:
    rows = [
        entry for entry in context.exports
        if export_path(context, entry).casefold() == local_path.casefold()
    ]
    if len(rows) != 1:
        raise ProbeError(
            f"expected one export {context.logical_name}.{local_path}, found {len(rows)}"
        )
    entry = rows[0]
    actual_class = export_class(context, entry)
    if actual_class.casefold() != expected_class.casefold():
        raise ProbeError(
            f"wrong class for {local_path}: {actual_class}, expected {expected_class}"
        )
    if entry.serial_size <= 0:
        raise ProbeError(f"export has no serial payload: {local_path}")
    return entry


def serial_data(context: PackageContext, entry: Any) -> bytes:
    begin = entry.serial_offset
    end = begin + entry.serial_size
    data = context.logical[begin:end]
    if len(data) != entry.serial_size:
        raise ProbeError(f"truncated export serial: {export_path(context, entry)}")
    return data


def properties_for(
    context: PackageContext, entry: Any
) -> tuple[dict[str, Any], int, int]:
    try:
        return nonstatic.parse_properties_lossless(
            serial_data(context, entry), context.names, context.summary.version
        )
    except Exception as error:
        raise ProbeError(
            f"could not parse {export_path(context, entry)} properties: {error}"
        ) from error


def property_item(properties: dict[str, Any], name: str) -> dict[str, Any]:
    wanted = name.casefold()
    rows = [item for key, item in properties.items() if key.casefold() == wanted]
    if len(rows) != 1:
        raise ProbeError(f"expected one property {name}, found {len(rows)}")
    return rows[0]


def object_path(context: PackageContext, reference: int) -> str | None:
    return ue3.package_ref_path(reference, context.imports, context.exports)


def qualified_path(context: PackageContext, reference: int) -> str | None:
    path = object_path(context, reference)
    if path is None:
        return None
    if path.casefold().startswith(context.logical_name.casefold() + "."):
        return path
    if reference > 0:
        return f"{context.logical_name}.{path}"
    return path


def decode_single_parameter(
    context: PackageContext, item: dict[str, Any]
) -> dict[str, Any]:
    raw_hex = item.get("rawHex")
    if not isinstance(raw_hex, str):
        raise ProbeError("parameter array did not preserve rawHex")
    payload = bytes.fromhex(raw_hex)
    if len(payload) < 12:
        raise ProbeError("parameter array payload is truncated")
    count = struct.unpack_from("<i", payload, 0)[0]
    if count != 1:
        raise ProbeError(f"expected one parameter entry, found {count}")
    try:
        nested, end_offset = nonstatic._parse_properties_at(
            payload, context.names, 4
        )
    except Exception as error:
        raise ProbeError(f"could not decode parameter array entry: {error}") from error
    if end_offset != len(payload):
        raise ProbeError(
            f"parameter array has unexplained suffix: {len(payload) - end_offset} bytes"
        )
    name = property_item(nested, "parametername").get("value")
    value = property_item(nested, "parametervalue").get("value")
    guid = property_item(nested, "expressionguid").get("rawHex")
    if not isinstance(name, str) or not isinstance(value, int) or not isinstance(guid, str):
        raise ProbeError("parameter array entry has invalid fields")
    path = qualified_path(context, value)
    if path is None:
        raise ProbeError("parameter value is null")
    return {
        "parameter": name,
        "objectReference": value,
        "objectPath": path,
        "expressionGuidRawHex": guid,
    }


def decode_nested_struct(
    context: PackageContext, item: dict[str, Any]
) -> dict[str, Any]:
    raw_hex = item.get("rawHex")
    if not isinstance(raw_hex, str):
        raise ProbeError("nested struct did not preserve rawHex")
    payload = bytes.fromhex(raw_hex)
    try:
        nested, end_offset = nonstatic._parse_properties_at(
            payload, context.names, 0
        )
    except Exception as error:
        raise ProbeError(f"could not decode nested struct: {error}") from error
    if end_offset != len(payload):
        raise ProbeError(f"nested struct has unexplained suffix: {len(payload)-end_offset}")
    result: dict[str, Any] = {}
    for key, value in nested.items():
        decoded = value.get("value")
        if str(value.get("type", "")).casefold() == "objectproperty" and isinstance(decoded, int):
            result[key] = {
                "reference": decoded,
                "objectPath": qualified_path(context, decoded),
            }
        else:
            result[key] = decoded
    return result


def scan_static_switch_entries(
    serial: bytes,
    names: list[str],
    switch_name: str,
    expression_guid_hex: str,
) -> list[dict[str, Any]]:
    indices = [index for index, name in enumerate(names) if name.casefold() == switch_name.casefold()]
    if len(indices) != 1:
        raise ProbeError(f"expected one NameTable entry for {switch_name}, found {len(indices)}")
    marker = struct.pack("<ii", indices[0], 0)
    guid = bytes.fromhex(expression_guid_hex)
    if len(guid) != 16:
        raise ProbeError("static switch expression GUID is not 16 bytes")
    rows: list[dict[str, Any]] = []
    cursor = 0
    while True:
        offset = serial.find(marker, cursor)
        if offset < 0:
            break
        cursor = offset + 1
        if offset + 32 > len(serial) or serial[offset + 16 : offset + 32] != guid:
            continue
        value, override = struct.unpack_from("<ii", serial, offset + 8)
        if value not in (0, 1) or override not in (0, 1):
            raise ProbeError(
                f"invalid static switch booleans at 0x{offset:X}: {value}, {override}"
            )
        rows.append(
            {
                "offsetWithinSerial": offset,
                "value": bool(value),
                "override": bool(override),
                "expressionGuidRawHex": expression_guid_hex,
            }
        )
    return rows


def graph_parameter_rows(
    context: PackageContext, parent_local_path: str
) -> dict[str, list[dict[str, Any]]]:
    prefix = parent_local_path.casefold() + "."
    rows: dict[str, list[dict[str, Any]]] = {
        "textures": [],
        "switches": [],
    }
    for entry in context.exports:
        path = export_path(context, entry)
        if not path.casefold().startswith(prefix):
            continue
        class_name = export_class(context, entry).casefold()
        if class_name not in {
            "materialexpressiontexturesampleparameter2d",
            "materialexpressionstaticswitchparameter",
        }:
            continue
        properties, _, _ = properties_for(context, entry)
        parameter = property_item(properties, "parametername").get("value")
        if not isinstance(parameter, str):
            raise ProbeError(f"graph expression has no parameter name: {path}")
        expression_guid = property_item(properties, "expressionguid").get("rawHex")
        if not isinstance(expression_guid, str):
            raise ProbeError(f"graph expression has no GUID: {path}")
        base = {
            "exportIndex": entry.index,
            "objectPath": f"{context.logical_name}.{path}",
            "parameter": parameter,
            "expressionGuidRawHex": expression_guid,
        }
        if class_name == "materialexpressiontexturesampleparameter2d":
            reference = property_item(properties, "texture").get("value")
            if not isinstance(reference, int):
                raise ProbeError(f"texture expression has invalid reference: {path}")
            rows["textures"].append(
                {
                    **base,
                    "textureReference": reference,
                    "textureObjectPath": qualified_path(context, reference),
                }
            )
        else:
            default_item = properties.get("defaultvalue")
            default_value = bool(default_item.get("value")) if default_item else False
            rows["switches"].append(
                {
                    **base,
                    "defaultValue": default_value,
                    "defaultValuePropertySerialized": default_item is not None,
                    "a": decode_nested_struct(context, property_item(properties, "a")),
                    "b": decode_nested_struct(context, property_item(properties, "b")),
                }
            )
    return rows


def one_by_parameter(
    rows: Sequence[dict[str, Any]], parameter: str, label: str
) -> dict[str, Any]:
    matches = [row for row in rows if row.get("parameter", "").casefold() == parameter.casefold()]
    if len(matches) != 1:
        raise ProbeError(f"expected one {label} {parameter}, found {len(matches)}")
    return matches[0]


def dds_info(path: Path) -> dict[str, Any]:
    data = path.read_bytes()
    if len(data) < 128 or data[:4] != b"DDS " or struct.unpack_from("<I", data, 4)[0] != 124:
        raise ProbeError(f"invalid DDS: {path}")
    height, width = struct.unpack_from("<II", data, 12)
    mip_count = struct.unpack_from("<I", data, 28)[0]
    four_cc_bytes = data[84:88]
    four_cc = four_cc_bytes.rstrip(b"\0").decode("ascii", errors="replace")
    return {
        "width": width,
        "height": height,
        "mipCount": mip_count,
        "fourCC": four_cc,
        "byteSize": len(data),
        "sha256": sha256_bytes(data),
    }


def exact_file(root: Path, name: str) -> Path:
    rows = [
        path for path in root.rglob("*")
        if path.is_file() and path.name.casefold() == name.casefold()
    ]
    if len(rows) != 1:
        raise ProbeError(f"expected one exported {name}, found {len(rows)}")
    return rows[0]


def umodel_export(
    umodel: Path,
    package_root: Path,
    logical_package: str,
    object_name: str,
    output: Path,
    region: str,
    timeout: float,
) -> str:
    command = [
        str(umodel),
        "-export",
        "-game=lostark",
        f"-{region}",
        "-nameresolve",
        "-dds",
        "-uncook",
        "-groups",
        f"-path={package_root}",
        f"-out={output}",
        f"-obj={object_name}",
        logical_package,
    ]
    return run(command, umodel.parent, timeout, f"UModel export {logical_package}.{object_name}")


def source_evidence(
    fixture: dict[str, Any],
    instance: PackageContext,
    parent: PackageContext,
    texture: PackageContext,
) -> dict[str, Any]:
    instance_entry = find_export(instance, INSTANCE_LOCAL_PATH, "materialinstanceconstant")
    instance_properties, property_start, property_end = properties_for(instance, instance_entry)
    parent_reference = property_item(instance_properties, "parent").get("value")
    if not isinstance(parent_reference, int):
        raise ProbeError("material instance Parent is not an ObjectProperty")
    resolved_parent = object_path(instance, parent_reference)
    if resolved_parent != PARENT_OBJECT_PATH:
        raise ProbeError(f"unexpected material parent: {resolved_parent}")

    diffuse = decode_single_parameter(
        instance, property_item(instance_properties, "textureparametervalues")
    )
    if diffuse["parameter"] != DIFFUSE_PARAMETER or diffuse["objectPath"] != DIFFUSE_OBJECT_PATH:
        raise ProbeError(f"unexpected diffuse override: {diffuse}")

    parent_entry = find_export(parent, PARENT_LOCAL_PATH, "decalmaterial")
    graph = graph_parameter_rows(parent, PARENT_LOCAL_PATH)
    parent_diffuse = one_by_parameter(graph["textures"], DIFFUSE_PARAMETER, "texture expression")
    parent_opacity = one_by_parameter(graph["textures"], OPACITY_PARAMETER, "texture expression")
    opacity_switch = one_by_parameter(graph["switches"], OPACITY_SWITCH, "static switch")
    if parent_opacity["textureObjectPath"] != OPACITY_OBJECT_PATH:
        raise ProbeError(f"unexpected opacity default texture: {parent_opacity}")
    if opacity_switch["expressionGuidRawHex"].casefold() != OPACITY_SWITCH_GUID:
        raise ProbeError("opacity switch GUID changed")

    switch_entries = scan_static_switch_entries(
        serial_data(instance, instance_entry),
        instance.names,
        OPACITY_SWITCH,
        OPACITY_SWITCH_GUID,
    )
    if len(switch_entries) != EXPECTED_SWITCH_ENTRY_COUNT:
        raise ProbeError(
            f"expected {EXPECTED_SWITCH_ENTRY_COUNT} opacity switch entries, "
            f"found {len(switch_entries)}"
        )
    if any(row["value"] or row["override"] for row in switch_entries):
        raise ProbeError(f"unexpected opacity switch values: {switch_entries}")

    selected_branch = opacity_switch["b"]
    selected_expression = selected_branch.get("expression")
    if not isinstance(selected_expression, dict):
        raise ProbeError("opacity switch B branch has no expression reference")
    if selected_expression.get("objectPath") != parent_diffuse["objectPath"]:
        raise ProbeError("opacity switch B branch is not the diffuse texture expression")
    if selected_branch.get("outputindex") != 4 or selected_branch.get("maska") != 1:
        raise ProbeError(f"opacity switch B branch is not diffuse alpha: {selected_branch}")

    source = fixture.get("source", {})
    payload = fixture.get("payload", {})
    component = source.get("component", {}) if isinstance(source, dict) else {}
    return {
        "schemaVersion": SCHEMA_VERSION,
        "probeId": "BERN_DECAL_LAND01_LV_COMMON_DECAL_05",
        "areaId": AREA_ID,
        "fixture": {
            "id": fixture["id"],
            "type": fixture["type"],
            "level": source.get("level"),
            "componentObjectPath": component.get("objectPath"),
            "componentExportIndex": component.get("exportIndex"),
            "componentSerialSha256": payload.get("serial", {}).get("sha256"),
            "transform": fixture.get("transform"),
            "width": payload.get("properties", {}).get("width", {}).get("value"),
            "height": payload.get("properties", {}).get("height", {}).get("value"),
            "farPlane": payload.get("properties", {}).get("farplane", {}).get("value"),
        },
        "packages": [
            package_receipt(instance),
            package_receipt(parent),
            package_receipt(texture),
        ],
        "materialChain": [
            export_receipt(instance, instance_entry, INSTANCE_OBJECT_PATH),
            export_receipt(parent, parent_entry, PARENT_OBJECT_PATH),
        ],
        "instancePropertyStream": {
            "startWithinSerial": property_start,
            "endWithinSerial": property_end,
            "parentReference": parent_reference,
            "parentObjectPath": resolved_parent,
            "diffuseOverride": diffuse,
        },
        "parentGraph": {
            "diffuseExpression": parent_diffuse,
            "opacityExpression": parent_opacity,
            "opacitySwitch": opacity_switch,
        },
        "staticSelection": {
            "parameter": OPACITY_SWITCH,
            "masterDefaultValue": opacity_switch["defaultValue"],
            "masterDefaultValuePropertySerialized": opacity_switch[
                "defaultValuePropertySerialized"
            ],
            "instanceEntries": switch_entries,
            "resolvedValue": False,
            "override": False,
            "selectedBranch": "B",
            "selectedBranchData": selected_branch,
            "resolvedOpacityInput": {
                "objectPath": DIFFUSE_OBJECT_PATH,
                "channel": "alpha",
                "basis": "B expression -> texture_diffuse outputIndex=4, MaskA=1",
            },
        },
        "textureBindings": {
            "diffuse": {
                "parameter": DIFFUSE_PARAMETER,
                "objectPath": DIFFUSE_OBJECT_PATH,
                "source": "material-instance override",
            },
            "opacityParameterDefault": {
                "parameter": OPACITY_PARAMETER,
                "objectPath": OPACITY_OBJECT_PATH,
                "source": "parent DecalMaterial texture expression",
                "selectedByThisFixture": False,
            },
        },
        "limitations": [
            "The UModel sidecar reports Parent=None because this Lost Ark parent is a custom DecalMaterial; the original logical UPK ObjectProperty is authoritative.",
            "The parent opacity texture is exported as source evidence, but this fixture selects the diffuse alpha branch.",
            "No runtime render profile, projected decal renderer, material approximation, or placement was generated by this probe.",
        ],
    }


def package_receipt(context: PackageContext) -> dict[str, Any]:
    return {
        "logicalPackage": context.logical_name,
        "physicalPackage": context.physical_path.name,
        "physicalSha256": context.physical_sha256,
        "packageVersion": context.summary.version,
        "nameCount": context.summary.name_count,
        "exportCount": context.summary.export_count,
        "importCount": context.summary.import_count,
    }


def export_receipt(
    context: PackageContext, entry: Any, qualified_object_path: str
) -> dict[str, Any]:
    data = serial_data(context, entry)
    return {
        "objectPath": qualified_object_path,
        "class": export_class(context, entry),
        "exportIndex": entry.index,
        "serialOffset": entry.serial_offset,
        "serialSize": entry.serial_size,
        "serialSha256": sha256_bytes(data),
    }


def add_artifact(
    pack: Path, source: Path, relative: str, role: str
) -> dict[str, Any]:
    destination = pack / relative
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, destination)
    row: dict[str, Any] = {
        "role": role,
        "path": relative.replace("\\", "/"),
        "sha256": sha256_file(destination),
        "byteSize": destination.stat().st_size,
    }
    if destination.suffix.casefold() == ".dds":
        row["dds"] = dds_info(destination)
    return row


def write_pack(
    evidence: dict[str, Any],
    output_root: Path,
    umodel: Path,
    package_root: Path,
    region: str,
    timeout: float,
) -> dict[str, Any]:
    output_root = output_root.resolve()
    if output_root == Path(output_root.anchor):
        raise ProbeError(f"output root is too broad: {output_root}")
    output_root.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix="bern_decal_probe.", dir=output_root))
    pack = work / "pack"
    instance_export = work / "instance_export"
    opacity_export = work / "opacity_export"
    pack.mkdir()
    try:
        instance_log = umodel_export(
            umodel,
            package_root,
            INSTANCE_LOGICAL_PACKAGE,
            INSTANCE_OBJECT_NAME,
            instance_export,
            region,
            timeout,
        )
        opacity_log = umodel_export(
            umodel,
            package_root,
            TEXTURE_LOGICAL_PACKAGE,
            OPACITY_OBJECT_NAME,
            opacity_export,
            region,
            timeout,
        )
        mat = exact_file(instance_export, INSTANCE_OBJECT_NAME + ".mat")
        props = exact_file(instance_export, INSTANCE_OBJECT_NAME + ".props.txt")
        diffuse_dds = exact_file(instance_export, DIFFUSE_OBJECT_NAME + ".dds")
        opacity_dds = exact_file(opacity_export, OPACITY_OBJECT_NAME + ".dds")

        sidecar_text = props.read_text(encoding="utf-8", errors="replace")
        parent_line = re.search(r"^Parent\s*=\s*([^\r\n]*)$", sidecar_text, re.MULTILINE)
        _, sidecar_parameters = static_assets.parse_material_props(props)
        if (
            parent_line is None
            or parent_line.group(1).strip().casefold() != "none"
            or sidecar_parameters != {
            DIFFUSE_PARAMETER: DIFFUSE_OBJECT_NAME
            }
        ):
            raise ProbeError(
                f"unexpected UModel material sidecar: "
                f"parent={parent_line.group(1).strip() if parent_line else None}, "
                f"parameters={sidecar_parameters}"
            )
        mat_text = mat.read_text(encoding="utf-8", errors="replace").strip()
        if mat_text.casefold() != f"Diffuse={DIFFUSE_OBJECT_NAME}".casefold():
            raise ProbeError(f"unexpected UModel .mat content: {mat_text!r}")

        artifacts = [
            add_artifact(
                pack, mat, f"materials/{mat.name}", "umodel-material-summary"
            ),
            add_artifact(
                pack, props, f"materials/{props.name}", "umodel-material-properties"
            ),
            add_artifact(
                pack, diffuse_dds, f"textures/{diffuse_dds.name}", "diffuse-and-active-opacity-alpha"
            ),
            add_artifact(
                pack, opacity_dds, f"textures/{opacity_dds.name}", "parent-opacity-parameter-default"
            ),
        ]
        (pack / "logs").mkdir()
        (pack / "logs" / "instance_umodel.txt").write_text(
            instance_log, encoding="utf-8", newline="\n"
        )
        (pack / "logs" / "opacity_umodel.txt").write_text(
            opacity_log, encoding="utf-8", newline="\n"
        )
        for log in (pack / "logs").glob("*.txt"):
            artifacts.append(
                {
                    "role": "umodel-log",
                    "path": log.relative_to(pack).as_posix(),
                    "sha256": sha256_file(log),
                    "byteSize": log.stat().st_size,
                }
            )
        result = dict(evidence)
        result["artifacts"] = sorted(artifacts, key=lambda row: row["path"])
        result["validation"] = {
            "status": "exact-source-probe-pass",
            "runtimeMutation": False,
            "renderProfileGenerated": False,
        }
        atomic_write_json(pack / "bern_castle_decal_probe.json", result)

        destination = output_root / "decal_probe"
        if destination.exists():
            if not destination.is_dir():
                raise ProbeError(f"unsafe existing destination: {destination}")
            bounded_rmtree(destination, output_root)
        os.replace(pack, destination)
        return result
    finally:
        if work.exists():
            bounded_rmtree(work, output_root)


def build_evidence(args: argparse.Namespace) -> dict[str, Any]:
    for path, label in (
        (args.nonstatic_manifest, "non-static manifest"),
        (args.umodel, "UModel"),
        (args.package_root, "package root"),
    ):
        if not path.exists():
            raise ProbeError(f"{label} is missing: {path}")
    fixture = select_fixture(load_json(args.nonstatic_manifest))
    instance = open_package(
        INSTANCE_LOGICAL_PACKAGE, args.umodel, args.package_root, args.region, args.aes_key
    )
    parent = open_package(
        PARENT_LOGICAL_PACKAGE, args.umodel, args.package_root, args.region, args.aes_key
    )
    texture = open_package(
        TEXTURE_LOGICAL_PACKAGE, args.umodel, args.package_root, args.region, args.aes_key
    )
    return source_evidence(fixture, instance, parent, texture)


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--nonstatic-manifest", type=Path, required=True)
    parser.add_argument("--umodel", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--region", default="kr", choices=("kr", "na", "ru", "jp", "tw", "cn"))
    parser.add_argument("--aes-key", default=ue3.LOSTARK_KR_AES_KEY)
    parser.add_argument("--timeout", type=float, default=180.0)
    parser.add_argument("--output-root", type=Path)
    parser.add_argument(
        "--write",
        action="store_true",
        help="export the two DDS files and write an audited pack; otherwise inspect only",
    )
    args = parser.parse_args(argv)
    if args.write and args.output_root is None:
        parser.error("--write requires --output-root")
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    return args


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    evidence = build_evidence(args)
    if args.write:
        result = write_pack(
            evidence,
            args.output_root,
            args.umodel.resolve(),
            args.package_root.resolve(),
            args.region,
            args.timeout,
        )
        print(
            json.dumps(
                {
                    "status": result["validation"]["status"],
                    "output": str((args.output_root / "decal_probe").resolve()),
                    "artifactCount": len(result["artifacts"]),
                    "resolvedOpacityInput": result["staticSelection"]["resolvedOpacityInput"],
                },
                ensure_ascii=False,
                indent=2,
            )
        )
    else:
        print(json.dumps(evidence, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ProbeError, ue3.ExtractionError) as error:
        print(json.dumps({"status": "failed", "error": str(error)}, ensure_ascii=False), file=sys.stderr)
        raise SystemExit(1)
