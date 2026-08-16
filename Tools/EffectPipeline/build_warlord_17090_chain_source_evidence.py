#!/usr/bin/env python3
"""Build the focused Warlord 17090 chain source-evidence receipt.

This tool deliberately does not choose a runtime Base lane or reconstruct the
cooked-out WPO graph.  It pins the exact twelve chain carriers, source package
and texture bytes, and the boundary that keeps the family approximate until a
typed artist-tuned evaluator is authored.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import re
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any, Iterator


REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "Tools" / "LevelPlacementExtractor"))
sys.path.insert(0, str(REPO_ROOT / "Tools" / "ModelAssetConverter"))

from extract_ue3_material_graph import extract_material_contract  # noqa: E402
from cook_wmodel_geometry_contract import parse_legacy_wmodel  # noqa: E402


EFFECT_ASSET_ID = "effect.warlord.skill.17090.unified"
SOURCE_EFFECT_ASSET_ID = "effect.warlord.skill.17090.imported"
CHILD_MATERIAL = "fx_m_mi_d_00.fx_mi.fx_d_me_chain_01_101_ma"
PARENT_MATERIAL = "fx_m_mi_00.fx_m.fx_d_me_chain_01_ma"
PARENT_IN_PACKAGE_PATH = "fx_m.fx_d_me_chain_01_ma"
TYPED_REASON = "SOURCE_MASKED_WPO_ARITHMETIC_UNAVAILABLE"

PACKAGE_FILES = {
    "fx_m_mi_00": "YGI3SB3OBJ3O11GUMP6QMP885.upk",
    "fx_m_mi_d_00": "ZHJ4TC4PCK4PL4J22HIXEYUX5U.upk",
    "fx_tex_02": "YGI3SORGM3I1FGHA5BMJ8Y5CZ.upk",
    "fx_tex_05": "YGI3SORGM3I10GHA5BMJ815CZ.upk",
    "fx_tex_00": "YGI3SORGM3I11GHA5BMJ885CZ.upk",
    "fx_sm_01": "XFH2RGA2R07F04YE90DX0SMQ.upk",
}

TEXTURE_ROWS = (
    ("fx_tex_02", "sk_wbk_bex_00", "FX_TEX_02"),
    ("fx_tex_02", "sk_wbk_bex_00_n", "FX_TEX_02"),
    ("fx_tex_02", "fx_d_grid_016", "FX_TEX_02"),
    ("fx_tex_02", "fx_e_symbol_064_1_n_xcl", "FX_TEX_02"),
    ("fx_tex_02", "fx_d_atypical_076_cl", "FX_TEX_02"),
    ("fx_tex_02", "fx_d_atypical_076_3_cl", "FX_TEX_02"),
    ("fx_tex_02", "fx_e_symbol_064_1_cl", "FX_TEX_02"),
    ("fx_tex_05", "fx_m_noise_003", "FX_TEX_05"),
    ("fx_tex_00", "fx_a_fluid_003", "FX_TEX_00"),
)

CHAIN_MODELS = {
    "fm_d_berchain_06.wmodel": 8,
    "fm_d_berchain_07.wmodel": 4,
}

DYNAMIC_PARAMETER_NAMES = (
    "worldpositionoffset_str",
    "worldposition_uvscale",
    "x.pan",
    "worldposition_zoffset",
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def read_json(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def canonical_sha256(value: Any) -> str:
    payload = json.dumps(
        value, ensure_ascii=False, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")
    return sha256_bytes(payload)


def iter_objects(value: Any) -> Iterator[dict[str, Any]]:
    if isinstance(value, dict):
        yield value
        for child in value.values():
            yield from iter_objects(child)
    elif isinstance(value, list):
        for child in value:
            yield from iter_objects(child)


def module_by_class(element: dict[str, Any], class_name: str) -> dict[str, Any]:
    matches = [
        row for row in element["sourceRecipe"]["modules"]
        if str(row.get("className", "")).casefold() == class_name.casefold()
    ]
    require(
        len(matches) == 1,
        f"Warlord chain requires exactly one {class_name} module.",
    )
    return matches[0]


def literal_value(module: dict[str, Any], property_path: str) -> Any:
    matches = [
        row.get("value") for row in module.get("literals", [])
        if row.get("propertyPath") == property_path
    ]
    require(
        len(matches) == 1,
        f"Warlord chain literal cardinality changed: {property_path}",
    )
    return matches[0]


def distribution(
    module: dict[str, Any], property_path: str
) -> dict[str, Any]:
    matches = [
        row for row in module.get("distributions", [])
        if row.get("propertyPath") == property_path
    ]
    require(
        len(matches) == 1,
        f"Warlord chain distribution cardinality changed: {property_path}",
    )
    return matches[0]


def finite_numbers(values: list[Any], label: str) -> list[float]:
    result = [float(value) for value in values]
    require(
        all(math.isfinite(value) for value in result),
        f"Warlord chain {label} contains a non-finite value.",
    )
    return result


def source_element_id(source_node: str) -> str:
    marker = "|element:"
    require(marker in source_node, "Authored chain sourceNode is invalid.")
    return source_node.split(marker, 1)[1]


def collect_chain_rows(
    imported: dict[str, Any], authored: dict[str, Any]
) -> list[dict[str, Any]]:
    require(
        imported.get("effectAssetId") == SOURCE_EFFECT_ASSET_ID,
        "Warlord imported effect identity changed.",
    )
    require(
        authored.get("effectAssetId") == EFFECT_ASSET_ID,
        "Warlord authored effect identity changed.",
    )
    stable_ids = {
        source_element_id(str(row.get("sourceNode", ""))): row["id"]
        for row in authored.get("elements", [])
        if "|element:" in str(row.get("sourceNode", ""))
    }
    authored_by_id = {row["id"]: row for row in authored.get("elements", [])}

    rows: list[dict[str, Any]] = []
    counts = {name: 0 for name in CHAIN_MODELS}
    for element in imported.get("elements", []):
        if element.get("material", {}).get("sourceMaterialPath") != CHILD_MATERIAL:
            continue
        meshes = [
            row["assetId"] for row in element.get("resources", [])
            if row.get("slotId") == "meshModel"
        ]
        require(len(meshes) == 1, "Warlord chain mesh cardinality changed.")
        mesh_name = Path(meshes[0]).name
        require(mesh_name in CHAIN_MODELS, "Warlord chain mesh identity changed.")
        counts[mesh_name] += 1

        dynamic = module_by_class(element, "particlemoduleparameterdynamic")
        dynamic_rows = []
        for component, expected_name in enumerate(DYNAMIC_PARAMETER_NAMES):
            name = literal_value(
                dynamic, f"dynamicparams[{component}].paramname"
            )
            require(
                name == expected_name,
                "Warlord chain DynamicParameter identity changed.",
            )
            curve = distribution(
                dynamic, f"dynamicparams[{component}].paramvalue"
            )
            table = finite_numbers(
                list(curve.get("lookupTable", [])),
                f"DynamicParameter[{component}] lookup table",
            )
            require(table, "Warlord chain DynamicParameter curve is empty.")
            dynamic_rows.append({
                "component": component,
                "name": name,
                "lookupTableCount": len(table),
                "lookupTableCanonicalSha256": canonical_sha256(table),
                "minimum": min(table),
                "maximum": max(table),
                "lookupTableTimeScale": curve.get("lookupTableTimeScale"),
                "lookupTableStartTime": curve.get("lookupTableStartTime"),
            })

        location_module = module_by_class(
            element, "particlemodulelocationdirect"
        )
        location = distribution(location_module, "location")
        location_table = finite_numbers(
            list(location.get("lookupTable", [])), "LocationDirect lookup table"
        )
        require(
            len(location_table) == 8,
            "Warlord chain LocationDirect table cardinality changed.",
        )
        type_data = module_by_class(element, "particlemoduletypedatamesh")
        rotation = []
        for axis in ("roll", "pitch", "yaw"):
            matches = [
                row.get("value") for row in type_data.get("literals", [])
                if row.get("propertyPath") == axis
            ]
            require(len(matches) <= 1, "Warlord TypeData rotation is duplicated.")
            value = float(matches[0]) if matches else 0.0
            require(math.isfinite(value), "Warlord TypeData rotation is not finite.")
            rotation.append(value)

        imported_id = element["id"]
        require(imported_id in stable_ids, "Warlord chain stable identity is missing.")
        target_id = stable_ids[imported_id]
        target = authored_by_id[target_id]
        require(
            float(target["detail"]["mesh"].get("modelPreScale", 0.0)) == 0.01,
            "Warlord chain modelPreScale changed.",
        )
        rows.append({
            "sourceElementId": imported_id,
            "targetElementId": target_id,
            "sourceGroupId": element["groupId"],
            "meshAssetId": meshes[0],
            "sourceTypeDataRotationDegrees": rotation,
            "modelPreScale": 0.01,
            "startDelaySeconds": element["detail"]["timing"][
                "startDelaySeconds"
            ],
            "particleLifetimeSeconds": element["detail"]["particle"][
                "lifeTimeSeconds"
            ],
            "locationDirect": {
                "lookupTable": location_table,
                "lookupTableCanonicalSha256": canonical_sha256(location_table),
                "lookupTableTimeScale": location.get("lookupTableTimeScale"),
                "lookupTableStartTime": location.get("lookupTableStartTime"),
            },
            "dynamicParameters": dynamic_rows,
        })

    require(len(rows) == 12, "Warlord chain row count changed; expected 12.")
    require(counts == CHAIN_MODELS, "Warlord chain 06/07 cardinality changed.")
    return rows


def collect_material_boundary(evidence: dict[str, Any]) -> dict[str, Any]:
    materials = evidence.get("materials", {})
    child_rows = materials.get(CHILD_MATERIAL)
    parent_rows = materials.get(PARENT_MATERIAL)
    require(
        isinstance(child_rows, list) and len(child_rows) == 1,
        "Warlord child Material evidence is missing or ambiguous.",
    )
    require(
        isinstance(parent_rows, list) and len(parent_rows) == 1,
        "Warlord parent Material evidence is missing or ambiguous.",
    )
    child = child_rows[0]
    parent = parent_rows[0]
    require(isinstance(child, dict), "Warlord child Material evidence is invalid.")
    require(isinstance(parent, dict), "Warlord parent Material evidence is invalid.")
    require(
        child.get("parent") == PARENT_MATERIAL,
        "Warlord child-to-parent Material identity changed.",
    )
    parent_ref = str(child.get("materialEvidenceRef", ""))
    require(
        parent_ref.startswith(PARENT_MATERIAL + "@sha256:"),
        "Warlord parent Material evidence reference changed.",
    )
    parent_details = evidence.get("parentMaterialEvidence", {}).get(parent_ref)
    require(
        isinstance(parent_details, dict),
        "Warlord parent Material declaration evidence is missing.",
    )
    declaration = parent_details["materialEvidence"]
    state = declaration["renderState"]
    require(
        state.get("blendMode") == "BLEND_Masked"
        and state.get("twoSided") is True
        and state.get("disableDepthTest") is False,
        "Warlord parent Material render state changed.",
    )
    referenced = [str(row).casefold() for row in declaration["referencedTextures"]]
    expected = [row[1] for row in TEXTURE_ROWS]
    require(
        referenced == expected,
        "Warlord parent Material referenced texture order changed.",
    )
    require(
        declaration.get("collectedTextureParameters") == [],
        "Warlord parent unexpectedly gained a named texture parameter contract.",
    )
    return {
        "childMaterialPath": CHILD_MATERIAL,
        "childSourcePhysicalPackage": child["source_file"],
        "parentMaterialPath": PARENT_MATERIAL,
        "parentSourcePhysicalPackage": parent["source_file"],
        "parentPropsFile": parent_details["propsFile"],
        "parentPropsSha256": parent_details["propsFileSha256"],
        "renderState": state,
        "referencedTextureObjectNames": referenced,
        "collectedTextureParameterCount": 0,
        "childScalarOverrides": child["scalars"],
        "childVectorOverrides": child["vectors"],
    }


def find_raw_mesh_row(raw_inventory: dict[str, Any], object_name: str) -> dict[str, Any]:
    source_path = f"fx_sm_01.{object_name.removesuffix('.wmodel')}"
    matches = []
    for row in iter_objects(raw_inventory):
        raw = row.get("rawResource")
        if (
            row.get("sourceAssetPath") == source_path
            and isinstance(raw, dict)
            and raw.get("sourceAssetPath") == source_path
            and isinstance(raw.get("payloads"), list)
        ):
            matches.append(raw)
    unique = {
        canonical_sha256(row): row for row in matches
    }
    require(len(unique) == 1, f"Raw mesh evidence is ambiguous: {source_path}")
    return next(iter(unique.values()))


def compare_runtime_mesh(
    raw_inventory: dict[str, Any],
    raw_resource_root: Path,
    runtime_resource_root: Path,
    object_name: str,
) -> dict[str, Any]:
    evidence = find_raw_mesh_row(raw_inventory, object_name)
    payloads = [
        row for row in evidence["payloads"] if row.get("kind") == "WMODEL"
    ]
    require(len(payloads) == 1, "Warlord raw WModel payload is missing.")
    payload = payloads[0]
    raw_path = raw_resource_root / Path(payload["relativePath"]).relative_to(
        "FourClass"
    )
    runtime_asset_id = (
        "Effect/Warlord/Meshes/FX_SM_01/" + object_name
    )
    runtime_path = runtime_resource_root / runtime_asset_id
    require(raw_path.is_file(), f"Raw Warlord WModel is missing: {raw_path}")
    require(
        runtime_path.is_file(), f"Runtime Warlord WModel is missing: {runtime_path}"
    )
    require(
        sha256_file(raw_path) == payload["sha256"],
        "Raw Warlord WModel SHA changed.",
    )
    _, raw_sections, raw_meshes = parse_legacy_wmodel(raw_path.read_bytes())
    _, runtime_sections, runtime_meshes = parse_legacy_wmodel(
        runtime_path.read_bytes()
    )
    require(len(raw_meshes) == len(runtime_meshes) == 1, "Warlord WModel submesh changed.")
    raw_mesh = raw_meshes[0]
    runtime_mesh = runtime_meshes[0]
    require(raw_mesh.indices == runtime_mesh.indices, "Warlord WModel indices changed.")
    require(
        raw_mesh.material_index == runtime_mesh.material_index
        and raw_mesh.material_hash == runtime_mesh.material_hash
        and raw_mesh.name_bytes == runtime_mesh.name_bytes,
        "Warlord WModel section identity changed.",
    )
    require(
        raw_sections[1].payload == runtime_sections[1].payload,
        "Warlord WModel material section changed.",
    )
    require(
        len(raw_mesh.vertices) == len(runtime_mesh.vertices),
        "Warlord WModel vertex count changed.",
    )
    maximum_position_error = 0.0
    for raw_vertex, runtime_vertex in zip(raw_mesh.vertices, runtime_mesh.vertices):
        for component in range(3):
            maximum_position_error = max(
                maximum_position_error,
                abs(runtime_vertex[component] - raw_vertex[component] * 100.0),
            )
        require(
            runtime_vertex[3:] == raw_vertex[3:],
            "Warlord WModel non-position vertex channels changed.",
        )
    require(
        maximum_position_error < 1.0e-3,
        "Warlord runtime WModel is not the exact x100 source-position projection.",
    )
    return {
        "sourceAssetPath": evidence["sourceAssetPath"],
        "sourcePhysicalPackage": evidence["expectedPhysicalPackageFileName"],
        "rawWModel": {
            "relativePath": payload["relativePath"],
            "byteSize": raw_path.stat().st_size,
            "sha256": sha256_file(raw_path),
        },
        "runtimeAssetId": runtime_asset_id,
        "runtimeWModel": {
            "byteSize": runtime_path.stat().st_size,
            "sha256": sha256_file(runtime_path),
        },
        "vertexCount": len(raw_mesh.vertices),
        "indexCount": len(raw_mesh.indices),
        "runtimePositionScaleFromRawCook": 100.0,
        "authoredModelPreScale": 0.01,
        "effectiveSourcePositionScale": 1.0,
        "maximumPositionProjectionError": maximum_position_error,
        "indicesAndNonPositionChannelsByteEquivalent": True,
        "materialSectionByteEquivalent": True,
    }


def texture_statistics(path: Path) -> dict[str, Any]:
    try:
        from PIL import Image
    except ImportError as error:
        raise ValueError("Pillow is required for DDS channel evidence.") from error
    image = Image.open(path).convert("RGBA")
    channels = []
    for name in "RGBA":
        values = list(image.getchannel(name).getdata())
        channels.append({
            "channel": name,
            "minimum": min(values),
            "maximum": max(values),
            "uniqueValueCount": len(set(values)),
            "nonZeroPixelCount": sum(value != 0 for value in values),
            "fullyOpaquePixelCount": sum(value == 255 for value in values),
            "mean": round(sum(values) / len(values), 6),
        })
    return {
        "width": image.width,
        "height": image.height,
        "channels": channels,
    }


def run_umodel_texture_exports(
    umodel: Path,
    source_package_root: Path,
    runtime_resource_root: Path,
) -> tuple[list[dict[str, Any]], list[dict[str, Any]]]:
    invocations = []
    textures = []
    with tempfile.TemporaryDirectory(prefix="warlord-17090-textures-") as temporary:
        output_root = Path(temporary)
        for logical_package, object_name, runtime_folder in TEXTURE_ROWS:
            command = [
                str(umodel), "-export", "-game=lostark", "-kr",
                "-nameresolve", f"-path={source_package_root}",
                f"-out={output_root}", "-dds", "-nooverwrite",
                f"-obj={object_name}", logical_package,
            ]
            completed = subprocess.run(
                command,
                cwd=umodel.parent,
                text=True,
                encoding="utf-8",
                errors="replace",
                capture_output=True,
                check=False,
                creationflags=(
                    subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0
                ),
            )
            require(completed.returncode == 0, f"UModel texture export failed: {object_name}")
            matches = list(output_root.rglob(f"{object_name}.dds"))
            require(len(matches) == 1, f"UModel texture output is ambiguous: {object_name}")
            extracted = matches[0]
            runtime_asset_id = (
                f"Effect/Warlord/Textures/{runtime_folder}/{object_name}.dds"
            )
            runtime_path = runtime_resource_root / runtime_asset_id
            require(runtime_path.is_file(), f"Runtime Warlord texture is missing: {runtime_asset_id}")
            require(
                extracted.read_bytes() == runtime_path.read_bytes(),
                f"Runtime Warlord texture differs from exact source export: {object_name}",
            )
            invocations.append({
                "logicalPackage": logical_package,
                "objectName": object_name,
                "exitCode": completed.returncode,
            })
            textures.append({
                "sourceObjectPath": f"{logical_package}.{object_name}",
                "sourcePhysicalPackage": PACKAGE_FILES[logical_package],
                "runtimeAssetId": runtime_asset_id,
                "byteSize": runtime_path.stat().st_size,
                "sha256": sha256_file(runtime_path),
                "sourceExportByteEquivalent": True,
                **texture_statistics(runtime_path),
            })
    return textures, invocations


def run_parent_umodel_dump(
    umodel: Path, source_package_root: Path
) -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix="warlord-17090-parent-") as temporary:
        dump_path = Path(temporary) / "parent.dump.txt"
        command = [
            str(umodel), "-dump", "-game=lostark", "-kr", "-nameresolve",
            f"-path={source_package_root}", f"-log={dump_path}",
            "-obj=fx_d_me_chain_01_ma", "fx_m_mi_00",
        ]
        completed = subprocess.run(
            command,
            cwd=umodel.parent,
            text=True,
            encoding="utf-8",
            errors="replace",
            capture_output=True,
            check=False,
            creationflags=(
                subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0
            ),
        )
        require(completed.returncode == 0, "UModel parent Material dump failed.")
        text = (
            dump_path.read_text(encoding="utf-8-sig", errors="replace")
            if dump_path.is_file()
            else completed.stdout + "\n" + completed.stderr
        )
        match = re.search(
            r"^\s*OpacityMaskClipValue\s*=\s*([-+0-9.eE]+)\s*$",
            text,
            re.MULTILINE,
        )
        require(match is not None, "Parent opacity-mask clip evidence is missing.")
        return {
            "exitCode": completed.returncode,
            "dumpSha256": sha256_bytes(text.encode("utf-8")),
            "opacityMaskClipValue": float(match.group(1)),
        }


def package_rows(source_package_root: Path) -> list[dict[str, Any]]:
    result = []
    for logical_package, file_name in PACKAGE_FILES.items():
        path = source_package_root / "Dependencies" / file_name
        require(path.is_file(), f"Warlord source package is missing: {file_name}")
        result.append({
            "logicalPackage": logical_package,
            "physicalPackage": file_name,
            "byteSize": path.stat().st_size,
            "sha256": sha256_file(path),
        })
    return result


def build_receipt(args: argparse.Namespace) -> dict[str, Any]:
    imported = read_json(args.imported_document)
    authored = read_json(args.authored_document)
    material_evidence = read_json(args.material_evidence)
    raw_inventory = read_json(args.raw_inventory)

    chain_rows = collect_chain_rows(imported, authored)
    material = collect_material_boundary(material_evidence)
    packages = package_rows(args.source_package_root)
    package_by_logical = {row["logicalPackage"]: row for row in packages}
    parent_package = args.source_package_root / "Dependencies" / PACKAGE_FILES[
        "fx_m_mi_00"
    ]
    graph = extract_material_contract(parent_package, PARENT_IN_PACKAGE_PATH)
    summary = graph["summary"]
    require(
        summary == {
            "expressionEntryCount": 132,
            "nonNullExpressionCount": 5,
            "nullExpressionCount": 127,
            "namedTextureCount": 0,
            "unresolvedInputEdgeCount": 0,
            "topologyStatus": "COOKED_PARTIAL",
            "runtimeExactEligible": False,
        },
        "Warlord parent cooked graph boundary changed.",
    )
    require(
        all(
            graph["outputs"].get(name, {}).get("packageIndex") == 0
            for name in ("emissivecolor", "opacitymask", "normal", "worldpositionoffset")
        ),
        "Warlord parent output graph unexpectedly changed.",
    )
    parent_dump = run_parent_umodel_dump(args.umodel, args.source_package_root)
    require(
        abs(parent_dump["opacityMaskClipValue"] - 0.333) < 1.0e-6,
        "Warlord parent opacity-mask clip changed.",
    )
    textures, invocations = run_umodel_texture_exports(
        args.umodel, args.source_package_root, args.runtime_resource_root
    )
    meshes = [
        compare_runtime_mesh(
            raw_inventory, args.raw_resource_root,
            args.runtime_resource_root, name,
        )
        for name in CHAIN_MODELS
    ]

    grid = next(
        row for row in textures if row["sourceObjectPath"] == "fx_tex_02.fx_d_grid_016"
    )
    alpha = next(row for row in grid["channels"] if row["channel"] == "A")
    require(
        alpha["minimum"] == alpha["maximum"] == 255,
        "fx_d_grid_016 alpha channel unexpectedly changed.",
    )
    return {
        "schema": "lostark.warlord-17090-chain-source-evidence",
        "formatVersion": 1,
        "effectAssetId": EFFECT_ASSET_ID,
        "sourceEffectAssetId": SOURCE_EFFECT_ASSET_ID,
        "classification": {
            "typedReason": TYPED_REASON,
            "exactRuntimeEligible": False,
            "requiresArtistTunedApproximation": True,
            "fullPromotionAllowed": False,
            "statement": (
                "The exact carriers and source inputs survive, but the cooked "
                "parent has no executable opacity-mask or WPO expression edges."
            ),
        },
        "packages": packages,
        "tools": {
            "umodel": {
                "fileName": args.umodel.name,
                "byteSize": args.umodel.stat().st_size,
                "sha256": sha256_file(args.umodel),
            },
            "materialGraphExtractor": {
                "repoPath": "Tools/LevelPlacementExtractor/extract_ue3_material_graph.py",
                "sha256": sha256_file(
                    REPO_ROOT / "Tools" / "LevelPlacementExtractor" /
                    "extract_ue3_material_graph.py"
                ),
            },
        },
        "material": {
            **material,
            "parentPackage": package_by_logical["fx_m_mi_00"],
            "parentOpacityMaskClipValue": parent_dump["opacityMaskClipValue"],
            "parentUModelDumpSha256": parent_dump["dumpSha256"],
            "cookedGraph": {
                "outputs": graph["outputs"],
                "summary": graph["summary"],
                "survivingExpressions": graph["expressions"],
            },
        },
        "textures": textures,
        "textureExtractionInvocations": invocations,
        "meshes": meshes,
        "chainRows": chain_rows,
        "erroneousAutoBinding": {
            "assetId": "Effect/Warlord/Textures/FX_TEX_02/fx_d_grid_016.dds",
            "wasPreviouslyDescribedAs": "exact parent Diffuse",
            "isExactBaseSemantic": False,
            "reason": (
                "The parent only lists this object among nine referenced "
                "textures, declares zero collected texture parameters, and "
                "its cooked opacity-mask/WPO outputs both target packageIndex 0. "
                "The DDS alpha channel is fully opaque, so binding it to the "
                "standard alpha lane produces an unsupported white carrier."
            ),
            "requiredAction": "REMOVE_ERRONEOUS_EXACT_BASE_AUTO_BINDING",
            "typedReason": TYPED_REASON,
        },
        "requiredGpuWitness": {
            "status": "NOT_RUN_NO_TYPED_EVALUATOR",
            "rowCount": 12,
            "perRow": (
                "Render each exact stable row with its chain06/07 WModel, typed "
                "masked/two-sided evaluator, four DynamicParameter curves, and "
                "resolved texture semantics; require nonzero non-white chroma."
            ),
            "composite": (
                "Render all twelve rows at identical camera/times and require "
                "four launch directions, four spear tips, tiled chain silhouettes, "
                "and outbound-to-return motion."
            ),
            "forbiddenSubstitute": (
                "A helix or unrelated Mesh witness cannot satisfy this contract."
            ),
        },
        "inputSha256": {
            "importedDocument": sha256_file(args.imported_document),
            "authoredDocument": sha256_file(args.authored_document),
            "materialEvidence": sha256_file(args.material_evidence),
            "rawInventory": sha256_file(args.raw_inventory),
        },
    }


def write_json_atomic(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    staged = path.with_suffix(path.suffix + ".tmp")
    staged.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    staged.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--imported-document", type=Path, required=True)
    parser.add_argument("--authored-document", type=Path, required=True)
    parser.add_argument("--material-evidence", type=Path, required=True)
    parser.add_argument("--raw-inventory", type=Path, required=True)
    parser.add_argument("--source-package-root", type=Path, required=True)
    parser.add_argument("--raw-resource-root", type=Path, required=True)
    parser.add_argument("--runtime-resource-root", type=Path, required=True)
    parser.add_argument("--umodel", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    receipt = build_receipt(args)
    if args.check:
        require(args.output.is_file(), "Warlord chain source receipt is missing.")
        require(
            read_json(args.output) == receipt,
            "Warlord chain source receipt is stale.",
        )
        print(json.dumps({
            "status": "PASS",
            "mode": "check",
            "chainRows": len(receipt["chainRows"]),
            "textures": len(receipt["textures"]),
            "typedReason": receipt["classification"]["typedReason"],
        }, sort_keys=True))
        return 0

    write_json_atomic(args.output, receipt)
    print(json.dumps({
        "status": "PASS",
        "mode": "write",
        "chainRows": len(receipt["chainRows"]),
        "textures": len(receipt["textures"]),
        "typedReason": receipt["classification"]["typedReason"],
        "output": str(args.output),
    }, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
