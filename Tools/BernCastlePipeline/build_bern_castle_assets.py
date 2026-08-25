from __future__ import annotations

"""Build exact Bern Castle StaticMesh source/runtime packs.

The pipeline deliberately treats the UE3 package + object path as the source of
truth.  Every asset is exported with UModel's ``-obj=`` switch and is committed
only after the exact package/object glTF, its buffer, material parameters and
referenced textures have been validated.
"""

import argparse
import concurrent.futures
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Iterable, Sequence


STATIC_CLASS_DIRS = {"staticmesh", "staticmesh2", "staticmesh3"}
TEXTURE_PARAMETER_TO_SWITCH = {
    "texture_diffuse": "--material-remap",
    "diffuse": "--material-remap",
    "texture_basecolor": "--material-remap",
    "texture_albedo": "--material-remap",
    "texture_cloud_mask": "--material-remap",
    "texture_normal": "--normal-remap",
    "normal": "--normal-remap",
    "texture_specular": "--specular-remap",
    "specular": "--specular-remap",
    "texture_emissive": "--emissive-remap",
    "emissive": "--emissive-remap",
    "texture_opacity": "--opacity-remap",
    "opacity": "--opacity-remap",
    "texture_orm": "--orm-remap",
    "orm": "--orm-remap",
    "texture_metallic": "--metallic-remap",
    "metallic": "--metallic-remap",
    "texture_roughness": "--roughness-remap",
    "roughness": "--roughness-remap",
    "texture_ao": "--ao-remap",
    "ao": "--ao-remap",
}
# Texture roles the cooked .wmodel has no slot for. MATERIAL_ENTRY_V2/V3 carry
# baseColor/normal/specular/emissive/opacity (+orm/metallic/roughness/ao/mask),
# so a water reflection or a second scrolling normal cannot be remapped into a
# material slot without lying about what that slot means. These are copied
# beside the model and recorded by role in the source receipt; the water
# presentation document is what binds them at draw time.
AUXILIARY_TEXTURE_PARAMETERS = {
    "texture_reflection": "reflection",
    "reflection": "reflection",
    "detail_texture_normal": "detailNormal",
    "texture_detail_normal": "detailNormal",
    "texture_foam": "foam",
    "foam": "foam",
}
# Scalars and vectors the original water instances author. They are read from
# the whole MIC chain with the child winning, and they are evidence only: the
# runtime value lives in the published water presentation document.
MATERIAL_SCALAR_PARAMETERS = frozenset({
    "diffuse_tiling",
    "screen_distortion_intensity",
    "fresnel_intensity",
    "fresnel_power",
    "normal_intensity",
    "normal_distortion_intensity",
    "detail_normal_intensity",
    "depth_bias",
    "opacity",
    "opacity_power",
    "reflection_intensity",
    "reflection_uv",
    "specular_intensity",
    "specular_power",
})
MATERIAL_VECTOR_PARAMETERS = frozenset({
    "diffuse_color",
    "reflection_color",
    "normal_tiling_panning",
    "detail_normal_tiling_panning",
    "reflection_tiling_panning",
})
SOURCE_RECEIPT_SCHEMA_VERSION = 2
REQUIRED_WMODEL_MAGICS = {b"WINT", b"WMOD"}
PRINT_LOCK = threading.Lock()


class PipelineError(RuntimeError):
    pass


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest().upper()


def stable_asset_id(full_path: str, object_name: str) -> str:
    digest = hashlib.sha1(full_path.casefold().encode("utf-8")).hexdigest()[:12].upper()
    label = re.sub(r"[^A-Za-z0-9_]+", "_", object_name).strip("_").upper()
    if not label:
        raise PipelineError(f"invalid object name for asset ID: {object_name!r}")
    return f"MAP_{digest}_{label}"


def load_json(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise PipelineError(f"JSON root must be an object: {path}")
    return value


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
        raise PipelineError(f"refusing to remove path outside pipeline root: {resolved}")
    if resolved.exists():
        shutil.rmtree(resolved)


def commit_directory(stage: Path, destination: Path, root: Path, force: bool) -> None:
    if destination.exists():
        if not force:
            raise PipelineError(f"destination already exists: {destination}")
        bounded_rmtree(destination, root)
    destination.parent.mkdir(parents=True, exist_ok=True)
    os.replace(stage, destination)


def placement_files(directories: Sequence[Path]) -> list[Path]:
    files: list[Path] = []
    seen: set[Path] = set()
    for directory in directories:
        if not directory.is_dir():
            raise PipelineError(f"placement directory is missing: {directory}")
        for path in sorted(directory.glob("*.placements.json")):
            resolved = path.resolve()
            if resolved not in seen:
                seen.add(resolved)
                files.append(path)
    if not files:
        raise PipelineError("no *.placements.json inputs were found")
    return files


def inventory_assets(
    directories: Sequence[Path], level_prefix: str, expect_assets: int | None,
    expect_placements: int | None,
) -> dict[str, Any]:
    by_path: dict[str, dict[str, Any]] = {}
    source_files: list[dict[str, Any]] = []
    level_counts: Counter[str] = Counter()
    placement_count = 0
    for path in placement_files(directories):
        document = load_json(path)
        if document.get("schemaVersion") != 1 or document.get("propertyErrors"):
            raise PipelineError(f"invalid placement source: {path}")
        selected = []
        for row in document.get("placements", []):
            level = str(row.get("levelPackage", ""))
            if level_prefix and not level.casefold().startswith(level_prefix.casefold()):
                continue
            selected.append(row)
            asset = row.get("asset", {})
            full_path = str(asset.get("objectPath", "")).strip().casefold()
            source_object_name = str(asset.get("objectName", "")).strip()
            if (
                not full_path
                or not source_object_name
                or full_path.split(".")[-1] != source_object_name.casefold()
            ):
                raise PipelineError(f"invalid asset reference in {path}: {asset!r}")
            object_name = full_path.split(".")[-1]
            path_parts = full_path.split(".")
            logical_package = path_parts[0]
            candidate = {
                "assetId": stable_asset_id(full_path, object_name),
                "fullPath": full_path,
                "logicalPackage": logical_package.upper(),
                "objectGroup": "/".join(path_parts[1:-1]),
                "objectName": object_name,
                "sourceCategory": "staticmesh",
            }
            previous = by_path.get(full_path)
            if previous is not None and previous != candidate:
                raise PipelineError(f"conflicting asset definition: {full_path}")
            by_path[full_path] = candidate
            level_counts[level] += 1
            placement_count += 1
        if selected:
            source_files.append(
                {
                    "path": str(path.resolve()),
                    "sha256": sha256(path),
                    "selectedPlacementCount": len(selected),
                }
            )
    assets = sorted(by_path.values(), key=lambda row: row["fullPath"])
    if len({row["assetId"] for row in assets}) != len(assets):
        raise PipelineError("stable asset ID collision")
    if expect_assets is not None and len(assets) != expect_assets:
        raise PipelineError(f"asset count mismatch: {len(assets)} != {expect_assets}")
    if expect_placements is not None and placement_count != expect_placements:
        raise PipelineError(
            f"placement count mismatch: {placement_count} != {expect_placements}"
        )
    return {
        "schemaVersion": 1,
        "areaId": "LV_BER_BERNCASTLE",
        "levelPrefix": level_prefix,
        "assetCount": len(assets),
        "placementCount": placement_count,
        "levelCounts": dict(sorted(level_counts.items())),
        "sources": source_files,
        "assets": assets,
    }


def corpus_audit(assets: Sequence[dict[str, Any]], corpus_root: Path | None) -> dict[str, Any]:
    if corpus_root is None:
        return {"enabled": False}
    if not corpus_root.is_dir():
        raise PipelineError(f"corpus root is missing: {corpus_root}")
    exact: set[tuple[str, str]] = set()
    by_name: defaultdict[str, set[str]] = defaultdict(set)
    scanned = 0
    for gltf in corpus_root.rglob("*.gltf"):
        parts = gltf.relative_to(corpus_root).parts
        class_index = next(
            (index for index, part in enumerate(parts[:-1]) if part.casefold() in STATIC_CLASS_DIRS),
            None,
        )
        if class_index is None or class_index == 0:
            continue
        package = parts[class_index - 1].casefold()
        name = gltf.stem.casefold()
        exact.add((package, name))
        by_name[name].add(package)
        scanned += 1
    exact_rows: list[str] = []
    mismatch_rows: list[dict[str, Any]] = []
    missing_rows: list[str] = []
    for asset in assets:
        package = str(asset["logicalPackage"]).casefold()
        name = str(asset["objectName"]).casefold()
        full_path = str(asset["fullPath"])
        if (package, name) in exact:
            exact_rows.append(full_path)
        elif name in by_name:
            mismatch_rows.append(
                {"fullPath": full_path, "foundPackages": sorted(by_name[name])}
            )
        else:
            missing_rows.append(full_path)
    return {
        "enabled": True,
        "root": str(corpus_root.resolve()),
        "scannedGltfCount": scanned,
        "exactCount": len(exact_rows),
        "sourceMismatchCount": len(mismatch_rows),
        "missingCount": len(missing_rows),
        "exact": exact_rows,
        "sourceMismatches": mismatch_rows,
        "missing": missing_rows,
    }


def creation_flags() -> int:
    return subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0


def run(command: Sequence[str], cwd: Path, timeout: float, label: str) -> subprocess.CompletedProcess[str]:
    try:
        completed = subprocess.run(
            [str(item) for item in command], cwd=cwd, text=True, encoding="utf-8",
            errors="replace", capture_output=True, check=False, timeout=timeout,
            creationflags=creation_flags(),
        )
    except subprocess.TimeoutExpired as error:
        raise PipelineError(f"{label} timed out after {timeout:g}s") from error
    if completed.returncode != 0:
        output = (completed.stdout + "\n" + completed.stderr)[-6000:]
        raise PipelineError(f"{label} failed ({completed.returncode})\n{output}")
    return completed


def validate_tools(umodel: Path, package_root: Path, converter: Path | None = None) -> None:
    for path, label in ((umodel, "UModel"), (package_root, "package root")):
        if not path.exists():
            raise PipelineError(f"{label} is missing: {path}")
    if converter is not None and not converter.is_file():
        raise PipelineError(f"ModelAssetConverter is missing: {converter}")


def exact_target_gltf(
    export_root: Path, logical_package: str, object_group: str, object_name: str,
) -> Path:
    expected_parts = [logical_package.casefold()]
    expected_parts.extend(part.casefold() for part in object_group.split("/") if part)
    candidates: list[Path] = []
    for path in export_root.rglob("*.gltf"):
        if path.stem.casefold() != object_name.casefold():
            continue
        relative_parts = [part.casefold() for part in path.relative_to(export_root).parts[:-1]]
        if relative_parts == expected_parts:
            candidates.append(path)
    if len(candidates) != 1:
        raise PipelineError(
            f"expected one exact StaticMesh glTF for {logical_package}.{object_name}, "
            f"found {len(candidates)}"
        )
    return candidates[0]


def gltf_contract(path: Path) -> tuple[list[str], list[Path]]:
    document = load_json(path)
    materials = document.get("materials", [])
    material_names = [str(row.get("name", "")).strip() for row in materials]
    if any(not value for value in material_names):
        raise PipelineError(f"glTF contains an unnamed material: {path}")
    buffers: list[Path] = []
    for row in document.get("buffers", []):
        uri = str(row.get("uri", ""))
        candidate = path.parent / uri
        if not uri or Path(uri).is_absolute() or not candidate.is_file():
            raise PipelineError(f"glTF buffer is missing/invalid: {uri!r}")
        buffers.append(candidate)
    if not buffers or not document.get("meshes"):
        raise PipelineError(f"glTF has no mesh/buffer: {path}")
    return material_names, buffers


def props_index(export_root: Path) -> dict[str, list[Path]]:
    index: defaultdict[str, list[Path]] = defaultdict(list)
    for path in export_root.rglob("*.props.txt"):
        name = path.name[: -len(".props.txt")].casefold()
        index[name].append(path)
    return dict(index)


def choose_props(
    index: dict[str, list[Path]], name_reference: str, logical_package: str,
) -> Path | None:
    reference_parts = [part for part in name_reference.split(".") if part]
    name = reference_parts[-1] if reference_parts else name_reference
    rows = index.get(name.casefold(), [])
    if not rows:
        return None
    primary = [
        row for row in rows if any(
            part.casefold() == logical_package.casefold() for part in row.parts
        )
    ]
    candidates = primary or rows
    if not primary and len(reference_parts) > 1:
        suffix = [part.casefold() for part in reference_parts[:-1]]
        suffix_rows = []
        for row in rows:
            parents = [part.casefold() for part in row.parts[:-1]]
            if len(parents) >= len(suffix) and parents[-len(suffix):] == suffix:
                suffix_rows.append(row)
        if suffix_rows:
            candidates = suffix_rows
    hashes = {sha256(row) for row in candidates}
    if len(hashes) != 1:
        raise PipelineError(f"ambiguous material properties for {name}: {candidates}")
    return sorted(candidates, key=lambda row: str(row).casefold())[0]


def parse_material_document(path: Path) -> dict[str, Any]:
    """One .props.txt as an unfolded record.

    Only what the file itself declares is returned; the parent chain is folded
    by material_contract. Nothing is guessed: a parameter the file does not
    declare is simply absent, so a caller can tell "declared as 0" apart from
    "not declared here".
    """
    text = path.read_text(encoding="utf-8", errors="replace")
    parent_match = re.search(r"^Parent\s*=\s*[^']*'([^']+)'", text, re.MULTILINE)
    parent = parent_match.group(1).strip() if parent_match else None

    textures: dict[str, str] = {}
    for block in re.finditer(
        r"ParameterValue\s*=\s*Texture2D'([^']+)'(?:(?!ParameterValue).)*?"
        r"ParameterName\s*=\s*([A-Za-z0-9_]+)", text, re.DOTALL,
    ):
        texture = block.group(1).split(".")[-1].strip()
        parameter = block.group(2).casefold()
        if texture:
            textures[parameter] = texture

    scalars: dict[str, float] = {}
    for block in re.finditer(
        r"ParameterValue\s*=\s*(-?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)\s*[\r\n]"
        r"(?:(?!ParameterValue).)*?ParameterName\s*=\s*([A-Za-z0-9_]+)",
        text, re.DOTALL,
    ):
        scalars[block.group(2).casefold()] = float(block.group(1))

    vectors: dict[str, list[float]] = {}
    number = r"(-?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)"
    for block in re.finditer(
        r"ParameterValue\s*=\s*\{\s*R=" + number + r",\s*G=" + number +
        r",\s*B=" + number + r",\s*A=" + number + r"\s*\}"
        r"(?:(?!ParameterValue).)*?ParameterName\s*=\s*([A-Za-z0-9_]+)",
        text, re.DOTALL,
    ):
        vectors[block.group(5).casefold()] = [float(block.group(i)) for i in range(1, 5)]

    # The master Material3 declares its own defaults here rather than as
    # TextureParameterValues. Without these the foam and opacity textures of a
    # water preset have no source at all, because no instance overrides them.
    collected: dict[str, dict[str, str]] = {}
    for block in re.finditer(
        r"Texture\s*=\s*Texture2D'([^']+)'\s*[\r\n]\s*Name\s*=\s*([A-Za-z0-9_]+)"
        r"\s*[\r\n]\s*Group\s*=\s*([A-Za-z0-9_]*)", text,
    ):
        texture = block.group(1).split(".")[-1].strip()
        if texture:
            collected[block.group(2).casefold()] = {
                "texture": texture,
                "group": block.group(3).strip().casefold(),
            }

    flags: dict[str, Any] = {}
    blend_match = re.search(r"^\s*BlendMode\s*=\s*([A-Za-z_]+)", text, re.MULTILINE)
    if blend_match:
        flags["blendMode"] = blend_match.group(1).strip()
    for name, key in (
        ("TwoSided", "twoSided"),
        ("bIsMasked", "isMasked"),
        ("bDisableDepthTest", "disableDepthTest"),
    ):
        match = re.search(rf"^\s*{name}\s*=\s*(true|false)", text, re.MULTILINE)
        if match:
            flags[key] = match.group(1) == "true"
    clip_match = re.search(
        r"^\s*OpacityMaskClipValue\s*=\s*(-?\d+(?:\.\d+)?)", text, re.MULTILINE)
    if clip_match:
        flags["opacityMaskClipValue"] = float(clip_match.group(1))

    return {
        "parent": parent,
        "textures": textures,
        "collectedTextures": collected,
        "scalars": scalars,
        "vectors": vectors,
        "flags": flags,
    }


def parse_material_props(path: Path) -> tuple[str | None, dict[str, str]]:
    document = parse_material_document(path)
    return document["parent"], document["textures"]


def material_contract(
    material_name: str, index: dict[str, list[Path]], logical_package: str,
) -> tuple[dict[str, Any], list[Path]]:
    """Fold a MaterialInstanceConstant chain into one resolved contract.

    The walk is root first so a child instance overrides its parents, which is
    what the source engine does. The master Material3 sits at the root and is
    the only node that carries CollectedTextureParameters and the render flags,
    so its textures seed the result and its BlendMode decides how the asset has
    to be drawn. Callers must not re-derive any of this from a file name.
    """
    visiting: set[str] = set()
    used: list[Path] = []

    def empty() -> dict[str, Any]:
        return {
            "textures": {},
            "collectedTextures": {},
            "scalars": {},
            "vectors": {},
            "flags": {},
            "chain": [],
        }

    def visit(name: str) -> dict[str, Any]:
        key = name.casefold()
        if key in visiting:
            raise PipelineError(f"material parent cycle: {material_name}")
        path = choose_props(index, name, logical_package)
        if path is None:
            return empty()
        visiting.add(key)
        used.append(path)
        document = parse_material_document(path)
        result = visit(document["parent"]) if document["parent"] else empty()
        for parameter, entry in document["collectedTextures"].items():
            result["collectedTextures"][parameter] = entry
            result["textures"].setdefault(parameter, entry["texture"])
        result["textures"].update(document["textures"])
        result["scalars"].update(document["scalars"])
        result["vectors"].update(document["vectors"])
        result["flags"].update(document["flags"])
        result["chain"].append(name)
        visiting.remove(key)
        return result

    return visit(material_name), used


def material_texture_roles(
    material_name: str, index: dict[str, list[Path]], logical_package: str,
) -> tuple[dict[str, str], list[Path]]:
    contract, used = material_contract(material_name, index, logical_package)
    return contract["textures"], used


def find_texture(export_root: Path, texture_name: str) -> Path:
    candidates = [
        path for path in export_root.rglob("*")
        if path.is_file()
        and path.suffix.casefold() in {".dds", ".tga", ".png"}
        and path.stem.casefold() == texture_name.casefold()
    ]
    if not candidates:
        raise PipelineError(f"referenced texture was not exported: {texture_name}")
    by_extension: defaultdict[str, list[Path]] = defaultdict(list)
    for candidate in candidates:
        by_extension[candidate.suffix.casefold()].append(candidate)
    for extension in (".dds", ".tga", ".png"):
        rows = by_extension.get(extension, [])
        if not rows:
            continue
        hashes = {sha256(path) for path in rows}
        if len(hashes) != 1:
            raise PipelineError(
                f"ambiguous referenced texture: {texture_name}: {rows}"
            )
        return sorted(rows, key=lambda row: str(row).casefold())[0]
    raise PipelineError(f"unsupported referenced texture: {texture_name}")


def receipt_is_valid(directory: Path, receipt_name: str, asset: dict[str, Any]) -> bool:
    receipt_path = directory / receipt_name
    try:
        receipt = load_json(receipt_path)
        if receipt.get("fullPath") != asset["fullPath"]:
            return False
        # A receipt written before the material contract was preserved has no
        # scalars, render flags or auxiliary textures. Resuming on it would
        # keep the gap forever, so an older schema counts as not exported.
        if receipt.get("schemaVersion") != SOURCE_RECEIPT_SCHEMA_VERSION:
            return False
        for item in receipt.get("outputs", []):
            path = directory / str(item["path"])
            if not path.is_file() or sha256(path) != item["sha256"]:
                return False
        return True
    except (OSError, ValueError, KeyError, TypeError, json.JSONDecodeError):
        return False


def export_one(
    asset: dict[str, Any], output_root: Path, umodel: Path, package_root: Path,
    region: str, timeout: float, force: bool,
) -> dict[str, Any]:
    asset_id = str(asset["assetId"])
    destination = output_root / "source" / asset_id
    if not force and receipt_is_valid(destination, "source.receipt.json", asset):
        return {"assetId": asset_id, "status": "resumed"}
    staging_parent = output_root / ".staging" / "extract"
    staging_parent.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix=asset_id + ".", dir=staging_parent))
    export_root = work / "umodel"
    pack = work / "pack"
    try:
        command = [
            str(umodel), "-export", "-game=lostark", f"-{region}", "-nameresolve",
            "-gltf", "-dds", "-uncook", "-groups", f"-path={package_root}",
            f"-out={export_root}",
            f"-obj={asset['objectName']}", str(asset["logicalPackage"]),
        ]
        completed = run(command, umodel.parent, timeout, f"UModel export {asset['fullPath']}")
        console = completed.stdout + "\n" + completed.stderr
        physical_match = re.search(r"Loading package:\s+([^\r\n]+?\.upk)\s+Ver:", console)
        if physical_match is None:
            raise PipelineError(f"UModel did not report a physical package: {asset['fullPath']}")
        target = exact_target_gltf(
            export_root, str(asset["logicalPackage"]), str(asset["objectGroup"]),
            str(asset["objectName"]),
        )
        materials, buffers = gltf_contract(target)
        pack.mkdir(parents=True)
        shutil.copy2(target, pack / target.name)
        outputs: list[dict[str, Any]] = []
        for buffer in buffers:
            shutil.copy2(buffer, pack / buffer.name)
        prop_map = props_index(export_root)
        material_receipts: list[dict[str, Any]] = []
        copied_props: dict[str, Path] = {}
        copied_textures: dict[str, tuple[str, Path]] = {}
        def copy_referenced_texture(texture_name: str) -> dict[str, str]:
            source_texture = find_texture(export_root, texture_name)
            texture_key = source_texture.name.casefold()
            destination_texture = pack / "textures" / source_texture.name
            destination_texture.parent.mkdir(parents=True, exist_ok=True)
            prior = copied_textures.get(texture_key)
            source_hash = sha256(source_texture)
            if prior is not None and prior[0] != source_hash:
                raise PipelineError(f"texture filename collision: {source_texture.name}")
            if not destination_texture.exists():
                shutil.copy2(source_texture, destination_texture)
            copied_textures[texture_key] = (source_hash, destination_texture)
            return {
                "texture": f"textures/{source_texture.name}",
                "sha256": source_hash,
            }

        for material in materials:
            contract, used_props = material_contract(
                material, prop_map, str(asset["logicalPackage"])
            )
            parameters = contract["textures"]
            for source_props in used_props:
                props_hash = sha256(source_props)
                destination_props = (
                    pack / "materials" / f"{props_hash[:12]}__{source_props.name}"
                )
                destination_props.parent.mkdir(parents=True, exist_ok=True)
                if not destination_props.exists():
                    shutil.copy2(source_props, destination_props)
                copied_props[props_hash] = destination_props
            roles: dict[str, dict[str, str]] = {}
            auxiliary: dict[str, dict[str, str]] = {}
            missing: list[dict[str, str]] = []
            for parameter, texture_name in sorted(parameters.items()):
                switch = TEXTURE_PARAMETER_TO_SWITCH.get(parameter)
                role = AUXILIARY_TEXTURE_PARAMETERS.get(parameter)
                if switch is None and role is None:
                    continue
                try:
                    copied = copy_referenced_texture(texture_name)
                except PipelineError as error:
                    # The model still cooks without it; only the water look is
                    # short. Recording the gap keeps it visible instead of
                    # letting a silently absent role look like an authored one.
                    missing.append({
                        "parameter": parameter,
                        "texture": texture_name,
                        "reason": str(error),
                    })
                    continue
                if switch is not None:
                    roles[switch] = {"parameter": parameter, **copied}
                else:
                    auxiliary[role] = {"parameter": parameter, **copied}
            material_receipt: dict[str, Any] = {"name": material, "roles": roles}
            if auxiliary:
                material_receipt["auxiliaryTextures"] = auxiliary
            if missing:
                material_receipt["missingTextures"] = missing
            if contract["scalars"]:
                material_receipt["scalars"] = {
                    name: value for name, value in sorted(contract["scalars"].items())
                    if name in MATERIAL_SCALAR_PARAMETERS
                }
            if contract["vectors"]:
                material_receipt["vectors"] = {
                    name: value for name, value in sorted(contract["vectors"].items())
                    if name in MATERIAL_VECTOR_PARAMETERS
                }
            if contract["flags"]:
                material_receipt["renderFlags"] = contract["flags"]
            if contract["chain"]:
                material_receipt["parentChain"] = contract["chain"]
            material_receipts.append(material_receipt)
        (pack / "umodel.log.txt").write_text(console, encoding="utf-8", newline="\n")
        for file in sorted((path for path in pack.rglob("*") if path.is_file())):
            if file.name == "source.receipt.json":
                continue
            outputs.append({"path": file.relative_to(pack).as_posix(), "sha256": sha256(file)})
        receipt = {
            "schemaVersion": SOURCE_RECEIPT_SCHEMA_VERSION,
            "assetId": asset_id,
            "fullPath": asset["fullPath"],
            "logicalPackage": asset["logicalPackage"],
            "physicalPackage": physical_match.group(1).strip(),
            "objectName": asset["objectName"],
            "gltf": target.name,
            "materials": material_receipts,
            "outputs": outputs,
        }
        atomic_write_json(pack / "source.receipt.json", receipt)
        commit_directory(pack, destination, output_root, force=True)
        return {"assetId": asset_id, "status": "exported", "receipt": receipt}
    finally:
        if work.exists():
            bounded_rmtree(work, output_root)


def print_progress(label: str, done: int, total: int, result: dict[str, Any]) -> None:
    summary = {
        key: result[key]
        for key in ("assetId", "status", "fullPath", "error")
        if key in result
    }
    with PRINT_LOCK:
        print(
            json.dumps(
                {"phase": label, "done": done, "total": total, **summary},
                ensure_ascii=False,
            ),
            flush=True,
        )


def parallel_assets(
    label: str, assets: Sequence[dict[str, Any]], workers: int, function,
) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    failures: list[dict[str, str]] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        future_rows = {pool.submit(function, asset): asset for asset in assets}
        done = 0
        for future in concurrent.futures.as_completed(future_rows):
            asset = future_rows[future]
            done += 1
            try:
                result = future.result()
                results.append(result)
                print_progress(label, done, len(assets), result)
            except BaseException as error:
                row = {
                    "assetId": str(asset["assetId"]),
                    "fullPath": str(asset["fullPath"]),
                    "error": str(error),
                }
                failures.append(row)
                print_progress(label, done, len(assets), {"status": "failed", **row})
    if failures:
        raise PipelineError(
            f"{label} failed for {len(failures)} assets; first failures: "
            + json.dumps(failures[:10], ensure_ascii=False)
        )
    return sorted(results, key=lambda row: str(row["assetId"]))


def source_remaps(source: Path, receipt: dict[str, Any]) -> list[str]:
    command: list[str] = []
    seen: dict[tuple[str, str], tuple[str, str]] = {}
    for material in receipt.get("materials", []):
        material_name = str(material["name"])
        for switch, role in sorted(material.get("roles", {}).items()):
            key = (material_name.casefold(), switch)
            texture = source / str(role["texture"])
            if not texture.is_file() or sha256(texture) != role["sha256"]:
                raise PipelineError(f"source texture validation failed: {texture}")
            value = (str(role["texture"]), str(role["sha256"]))
            previous = seen.get(key)
            if previous is not None:
                if previous != value:
                    raise PipelineError(
                        f"conflicting material role: {material_name} {switch}"
                    )
                continue
            seen[key] = value
            command.extend([str(switch), f"{material_name}={texture}"])
    return command


def cook_one(
    asset: dict[str, Any], output_root: Path, converter: Path, timeout: float,
    force: bool,
) -> dict[str, Any]:
    asset_id = str(asset["assetId"])
    source = output_root / "source" / asset_id
    source_receipt = load_json(source / "source.receipt.json")
    if source_receipt.get("fullPath") != asset["fullPath"]:
        raise PipelineError(f"source receipt mismatch: {asset_id}")
    destination = output_root / "runtime" / asset_id
    if not force and receipt_is_valid(destination, "runtime.receipt.json", asset):
        return {"assetId": asset_id, "status": "resumed"}
    staging_parent = output_root / ".staging" / "cook"
    staging_parent.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix=asset_id + ".", dir=staging_parent))
    pack = work / "pack"
    try:
        pack.mkdir(parents=True)
        source_gltf = source / str(source_receipt["gltf"])
        if not source_gltf.is_file():
            raise PipelineError(f"source glTF is missing: {source_gltf}")
        for source_texture in sorted((source / "textures").glob("*")):
            if source_texture.is_file():
                destination_texture = pack / "textures" / source_texture.name
                destination_texture.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source_texture, destination_texture)
        remaps = source_remaps(source, source_receipt)
        remaps_for_pack: list[str] = []
        for index in range(0, len(remaps), 2):
            switch, specification = remaps[index], remaps[index + 1]
            material, separator, source_texture = specification.partition("=")
            if not separator:
                raise PipelineError(f"invalid remap: {specification}")
            texture = pack / "textures" / Path(source_texture).name
            if not texture.is_file():
                raise PipelineError(f"staged texture is missing: {texture}")
            remaps_for_pack.extend([switch, f"{material}={texture}"])
        model = pack / f"{asset_id}.wmodel"
        command = [
            str(converter), str(source_gltf), "-o", str(model), "--pretransform",
            "--no-auto-textures", "--scale", "100", *remaps_for_pack,
        ]
        completed = run(command, pack, timeout, f"ModelAssetConverter cook {asset_id}")
        if not model.is_file() or model.read_bytes()[:4] not in REQUIRED_WMODEL_MAGICS:
            raise PipelineError(f"invalid cooked WModel: {model}")
        info = run(
            [str(converter), "info", str(model)], pack, timeout,
            f"ModelAssetConverter info {asset_id}",
        )
        info_text = info.stdout + "\n" + info.stderr
        (pack / "converter.log.txt").write_text(
            completed.stdout + "\n" + completed.stderr, encoding="utf-8", newline="\n"
        )
        (pack / "converter.info.txt").write_text(
            info_text, encoding="utf-8", newline="\n"
        )
        outputs = [
            {"path": path.relative_to(pack).as_posix(), "sha256": sha256(path)}
            for path in sorted(path for path in pack.rglob("*") if path.is_file())
        ]
        receipt = {
            "schemaVersion": 1,
            "assetId": asset_id,
            "fullPath": asset["fullPath"],
            "model": f"{asset_id}/{asset_id}.wmodel",
            "sourceReceiptSha256": sha256(source / "source.receipt.json"),
            "outputs": outputs,
        }
        atomic_write_json(pack / "runtime.receipt.json", receipt)
        commit_directory(pack, destination, output_root, force=True)
        return {"assetId": asset_id, "status": "cooked", "receipt": receipt}
    finally:
        if work.exists():
            bounded_rmtree(work, output_root)


def build_runtime_manifest(
    output_root: Path, inventory: dict[str, Any], expect_assets: int | None,
) -> dict[str, Any]:
    rows: list[dict[str, Any]] = []
    for asset in inventory["assets"]:
        asset_id = str(asset["assetId"])
        pack = output_root / "runtime" / asset_id
        receipt = load_json(pack / "runtime.receipt.json")
        model_relative = str(receipt["model"])
        model = output_root / "runtime" / model_relative
        if receipt.get("fullPath") != asset["fullPath"] or not model.is_file():
            raise PipelineError(f"invalid runtime pack: {asset_id}")
        if model.read_bytes()[:4] not in REQUIRED_WMODEL_MAGICS:
            raise PipelineError(f"invalid WModel magic: {model}")
        rows.append(
            {
                "assetId": asset_id,
                "model": model_relative.replace("\\", "/"),
                "sha256": sha256(model),
                "fullPath": asset["fullPath"],
            }
        )
    if expect_assets is not None and len(rows) != expect_assets:
        raise PipelineError(f"runtime manifest count mismatch: {len(rows)}")
    manifest = {
        "schemaVersion": 1,
        "areaId": inventory["areaId"],
        "assetCount": len(rows),
        "assets": rows,
    }
    atomic_write_json(output_root / "manifests" / "bern_castle_runtime_assets.json", manifest)
    return manifest


def add_common_inventory_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--placements-dir", type=Path, action="append", required=True)
    parser.add_argument("--level-prefix", default="LV_BER_BERNCASTLE_T_")
    parser.add_argument("--expect-assets", type=int, default=950)
    parser.add_argument("--expect-placements", type=int, default=32324)
    parser.add_argument("--corpus-root", type=Path)
    parser.add_argument("--output-root", type=Path, required=True)


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    inventory = subparsers.add_parser("inventory")
    add_common_inventory_args(inventory)

    for name in ("extract", "all"):
        child = subparsers.add_parser(name)
        add_common_inventory_args(child)
        child.add_argument("--umodel", type=Path, required=True)
        child.add_argument("--package-root", type=Path, required=True)
        child.add_argument("--region", default="kr", choices=("kr", "na", "ru", "jp", "tw", "cn"))
        child.add_argument("--workers", type=int, default=2)
        child.add_argument("--umodel-timeout", type=float, default=180.0)
        child.add_argument("--force", action="store_true")
        child.add_argument("--asset-id", action="append", default=None)
        if name == "all":
            child.add_argument("--converter", type=Path, required=True)
            child.add_argument("--converter-timeout", type=float, default=180.0)

    cook = subparsers.add_parser("cook")
    cook.add_argument("--inventory", type=Path, required=True)
    cook.add_argument("--output-root", type=Path, required=True)
    cook.add_argument("--converter", type=Path, required=True)
    cook.add_argument("--workers", type=int, default=2)
    cook.add_argument("--converter-timeout", type=float, default=180.0)
    cook.add_argument("--expect-assets", type=int, default=950)
    cook.add_argument("--force", action="store_true")
    cook.add_argument("--asset-id", action="append", default=None)
    return parser.parse_args(argv)


def select_assets(
    assets: Sequence[dict[str, Any]], asset_ids: Sequence[str] | None,
) -> list[dict[str, Any]]:
    """Narrow the work list without narrowing the inventory.

    The inventory keeps its full 950-asset gate; only the assets actually
    handed to UModel or the converter are reduced. An id that matches nothing
    is an error rather than an empty run, so a typo cannot look like success.
    """
    rows = list(assets)
    if not asset_ids:
        return rows
    wanted = {str(value).casefold() for value in asset_ids}
    selected = [row for row in rows if str(row["assetId"]).casefold() in wanted]
    found = {str(row["assetId"]).casefold() for row in selected}
    unknown = sorted(wanted - found)
    if unknown:
        raise PipelineError(f"unknown --asset-id: {', '.join(unknown)}")
    return selected


def validate_output_root(path: Path) -> Path:
    resolved = path.resolve()
    if resolved == Path(resolved.anchor):
        raise PipelineError(f"output root is too broad: {resolved}")
    resolved.mkdir(parents=True, exist_ok=True)
    return resolved


def write_inventory(args: argparse.Namespace) -> dict[str, Any]:
    output_root = validate_output_root(args.output_root)
    document = inventory_assets(
        args.placements_dir, args.level_prefix, args.expect_assets,
        args.expect_placements,
    )
    document["corpusAudit"] = corpus_audit(document["assets"], args.corpus_root)
    atomic_write_json(output_root / "manifests" / "bern_castle_assets.json", document)
    print(json.dumps({
        "phase": "inventory", "assetCount": document["assetCount"],
        "placementCount": document["placementCount"],
        "corpusAudit": {
            key: document["corpusAudit"][key]
            for key in (
                "enabled", "scannedGltfCount", "exactCount",
                "sourceMismatchCount", "missingCount",
            )
            if key in document["corpusAudit"]
        },
    }, ensure_ascii=False, indent=2), flush=True)
    return document


def main(argv: Iterable[str] | None = None) -> int:
    args = parse_args(argv)
    started = time.monotonic()
    if args.command in ("inventory", "extract", "all"):
        inventory = write_inventory(args)
        output_root = args.output_root.resolve()
    else:
        inventory = load_json(args.inventory)
        output_root = validate_output_root(args.output_root)
        if inventory.get("assetCount") != len(inventory.get("assets", [])):
            raise PipelineError("inventory assetCount mismatch")

    selected = select_assets(inventory["assets"], getattr(args, "asset_id", None))
    if args.command in ("extract", "all"):
        validate_tools(args.umodel, args.package_root)
        if not 1 <= args.workers <= 8:
            raise PipelineError("workers must be in [1, 8]")
        parallel_assets(
            "extract", selected, args.workers,
            lambda asset: export_one(
                asset, output_root, args.umodel.resolve(), args.package_root.resolve(),
                args.region, args.umodel_timeout, args.force,
            ),
        )
    if args.command in ("cook", "all"):
        validate_tools(
            Path(sys.executable), output_root,
            args.converter,
        )
        if not 1 <= args.workers <= 8:
            raise PipelineError("workers must be in [1, 8]")
        parallel_assets(
            "cook", selected, args.workers,
            lambda asset: cook_one(
                asset, output_root, args.converter.resolve(), args.converter_timeout,
                args.force,
            ),
        )
        if len(selected) != len(inventory["assets"]):
            # The runtime manifest is a whole-area document gated on the full
            # asset count. Rebuilding it from a partial run would either fail
            # that gate or publish a manifest that silently lost assets.
            print(json.dumps({
                "phase": "runtime-manifest", "status": "skipped-partial-selection",
                "selectedAssetCount": len(selected),
            }, ensure_ascii=False), flush=True)
        else:
            manifest = build_runtime_manifest(output_root, inventory, args.expect_assets)
            print(json.dumps({
                "phase": "runtime-manifest", "assetCount": manifest["assetCount"]
            }, ensure_ascii=False), flush=True)
    print(json.dumps({
        "phase": "complete", "command": args.command,
        "elapsedSeconds": round(time.monotonic() - started, 3),
    }, ensure_ascii=False), flush=True)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except PipelineError as error:
        print(json.dumps({"status": "failed", "error": str(error)}, ensure_ascii=False), file=sys.stderr)
        raise SystemExit(1)
