"""Stage source-painted CModel geometry and verified native Landscape inputs.

Consumes the extractor's lossless SourceRaw tree and a shader-audited contract.
Never installs, publishes, edits placements, or generates missing texture mips.
The optional geometry-only mode validates a component before its shader is ready.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys

import extract_ue3_landscape as landscape

ROOT = Path(__file__).resolve().parents[2]
BERN_PREFIX = "Map/LV_BER_BERNCASTLE_T/Landscape"


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def write_same(path: Path, data: bytes):
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists():
        if path.read_bytes() != data:
            raise ValueError(f"candidate collision; choose a fresh output: {path}")
    else:
        path.write_bytes(data)


def write_json(path: Path, value):
    write_same(path, (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode("utf8"))


def resolve_raw(source_root: Path, reference: str) -> Path:
    parts = Path(reference.replace("\\", "/")).parts
    if not parts or parts[0] != "SourceRaw" or ".." in parts:
        raise ValueError(f"not a SourceRaw-relative reference: {reference}")
    result = source_root.joinpath(*parts[1:]).resolve()
    if not result.is_relative_to(source_root):
        raise ValueError(f"SourceRaw escape: {reference}")
    return result


def load_raw_texture(folder: Path):
    doc = read_json(folder / "texture.json")
    assert doc["pixelFormat"].casefold() == "pf_a8r8g8b8"
    assert doc["rawChannelOrder"] == "BGRA"
    mips = []
    for index, row in enumerate(doc["mips"]):
        raw = (folder / row["rawFile"]).read_bytes()
        assert row["level"] == index and sha(raw) == row["sha256"].lower()
        assert len(raw) == row["width"] * row["height"] * 4
        mips.append(landscape.TextureMip(index, row["width"], row["height"],
            int(row["bulkFlags"], 16), row["elementCount"], row["sizeOnDisk"],
            int(row["logicalOffset"], 16), raw))
    art = doc["sourceArt"]
    return landscape.DecodedTexture(doc["exportIndex"], doc["objectName"], doc["pixelFormat"],
        int(art["flags"], 16), art["elementCount"], art["sizeOnDisk"],
        int(art["logicalOffset"], 16), tuple(mips))


def load_component(path: Path, source_root: Path):
    doc = read_json(path)
    assert doc["logicalPackage"] in ("LV_BER_BERNCASTLE_T_LAND01", "LV_BER_BERNCASTLE_T_LAND02"), "shader contract is Bern-specific"
    d, p, h = doc["component"], doc["proxy"], doc["heightmap"]
    weights = doc["weightmaps"]
    component = landscape.LandscapeComponent(
        doc["logicalPackage"], d["exportIndex"], d["objectName"], *d["sectionBase"],
        d["componentSizeQuads"], d["subsectionSizeQuads"], d["numSubsections"],
        h["ref"], h["name"], h["path"], tuple(h["scaleBias"]),
        [w["ref"] for w in weights], [w["name"] for w in weights], [w["path"] for w in weights],
        tuple(doc["weightmapScaleBias"]), doc["weightmapSubsectionOffset"],
        [landscape.LayerAllocation(**a) for a in doc["layerAllocations"]],
        doc["materialInstance"]["ref"], doc["materialInstance"]["name"], d["cachedLocalBox"])
    proxy = landscape.LandscapeProxy(doc["logicalPackage"], p["exportIndex"], p["objectName"],
        p["landscapeGuid"], tuple(p["location"]), p["drawScale"], tuple(p["drawScale3D"]),
        d["componentSizeQuads"], d["subsectionSizeQuads"], d["numSubsections"],
        0, p["landscapeMaterialPath"], p["landscapeMaterialPath"].split(".")[-1],
        [d["exportIndex"] + 1], p["collisionComponentRefs"])
    component.height_texture = load_raw_texture(resolve_raw(source_root, h["source"]))
    component.weight_textures = [load_raw_texture(resolve_raw(source_root, w["source"])) for w in weights]
    assert landscape.stable_asset_id(component) == (doc["canonicalId"], doc["assetId"])
    assert component.component_size_quads == 62 and component.subsection_size_quads == 31
    assert component.num_subsections == 2 and 1 <= len(weights) <= 2
    return doc, component, proxy


def bgra_dds(texture):
    mips = texture.mips
    width, height = mips[0].width, mips[0].height
    header = [124, 0x100F | 0x20000, height, width, width * 4, 0, len(mips)] + [0] * 11
    header += [32, 0x41, 0, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000,
               0x401008, 0, 0, 0, 0]
    assert len(header) == 31
    return b"DDS " + struct.pack("<31I", *header) + b"".join(m.bgra for m in mips)


def oriented(triangle):
    a, b, c = triangle
    return min((a, b, c), (b, c, a), (c, a, b))


def model_mesh(data):
    assert data[:4] == b"WINT" and data[16:20] == b"WMOD"
    count = struct.unpack_from("<I", data, 20)[0]
    sections = [struct.unpack_from("<IIQQ40s", data, 48 + i * 64) for i in range(count)]
    section, = [s for s in sections if s[0] == 1]
    offset = 32 + section[2]
    h = struct.unpack_from("<4sIIIIIIIB3s", data, offset)
    assert h[0] == b"WMSH" and h[2] == 0 and h[4] == 48 and h[7] in (2, 4)
    descriptors = [struct.unpack_from("<IIIIIQ20s", data, offset + 36 + i * 48) for i in range(h[1])]
    start = offset + 36 + h[1] * 48
    vertices = [struct.unpack_from("<12f", data, start + i * 48) for i in range(h[5])]
    index_start = start + h[5] * 48
    triangles = []
    for vo, vc, io, ic, _, _, _ in descriptors:
        assert ic % 3 == 0 and vo % 48 == 0
        indices = struct.unpack_from("<" + ("H" if h[7] == 2 else "I") * ic, data, index_start + io)
        assert all(i < vc for i in indices)
        triangles.extend(tuple(vo // 48 + k for k in indices[i:i + 3]) for i in range(0, ic, 3))
    return vertices, triangles, descriptors


def verify_geometry(data, component, proxy, baseline):
    vertices, triangles, descriptors = model_mesh(data)
    assert len(descriptors) == 1 and descriptors[0][4] == 0
    points = landscape.component_positions(component, proxy)
    grid = component.component_size_quads + 1
    normals, tangents = landscape.component_normals_and_tangents(component, points, grid)
    logical_ids = []
    errors = dict(positionMeters=0.0, normal=0.0, tangent=0.0, componentUV0=0.0)
    for v in vertices:
        x, y = (round(v[k] * (grid - 1)) for k in (6, 7))
        assert 0 <= x < grid and 0 <= y < grid
        logical = y * grid + x
        logical_ids.append(logical)
        errors["positionMeters"] = max(errors["positionMeters"], *(abs(v[k] * .01 - points[logical][k]) for k in range(3)))
        errors["normal"] = max(errors["normal"], *(abs(v[k + 3] - normals[logical][k]) for k in range(3)))
        errors["tangent"] = max(errors["tangent"], *(abs(v[k + 8] - tangents[logical][k]) for k in range(3)))
        errors["componentUV0"] = max(errors["componentUV0"], abs(v[6] - x / (grid - 1)), abs(v[7] - y / (grid - 1)))
        assert v[11] == 1.0
    assert max(errors.values()) < 2e-5, errors
    actual = Counter(oriented(tuple(logical_ids[k] for k in tri)) for tri in triangles)
    wanted, holes = Counter(), 0
    for y in range(grid - 1):
        for x in range(grid - 1):
            if landscape.quad_is_hole(component, x, y):
                holes += 1
                continue
            a = y * grid + x
            wanted.update((oriented((a, a + 1, a + grid)), oriented((a + 1, a + grid + 1, a + grid))))
    assert actual == wanted, "source topology/winding mismatch"
    before_vertices, before_triangles, _ = model_mesh(baseline)
    scale_x, scale_y = (proxy.draw_scale * proxy.draw_scale3d[k] for k in (0, 1))
    before_ids = [round(-v[2] / scale_y) * grid + round(v[0] / scale_x) for v in before_vertices]
    assert Counter(oriented(tuple(before_ids[k] for k in t)) for t in before_triangles) == wanted
    for v, index in zip(before_vertices, before_ids):
        assert max(abs(v[k] * .01 - points[index][k]) for k in range(3)) < 2e-5
    return dict(vertexCount=len(vertices), triangleCount=len(triangles), holeQuadCount=holes,
        maximumErrors=errors, sourceTopologyAndWindingExact=True, baselineTopologyAndPositionPreserved=True,
        allFacesUseComponentUV0=True, materialSlotCount=1, tangentHandedness=1.0,
        worldAnchor=landscape.component_world_anchor(component, proxy))


def typed_row(contract, component, weight_ids, height_id, textures):
    allocations = {(int(a.layer_name[-2:]) - 1): a for a in component.allocations if a.layer_name.casefold() != "__datalayer__"}
    assert len(allocations) == len(contract["layers"]) and 1 <= len(allocations) <= 6
    layers, used = [], set()
    for source in contract["layers"]:
        layer = {k: source[k] for k in ("layerIndex", "uv", "diffuse", "specular", "factors", "weight", "diffuseColorSpace")}
        layer["diffuseColorSpace"] = layer["diffuseColorSpace"].casefold()
        layer["factors"] = list(layer["factors"])
        allocation = allocations.pop(layer["layerIndex"])
        assert layer["weight"][:2] == [allocation.texture_index, allocation.channel]
        assert layer["weight"][2] in (0, 1) and layer["weight"][3] in (0, 1)
        assert layer["diffuseColorSpace"] in ("srgb", "linear")
        for key, role in (("diffuseSourceObject", "diffuseTexture"), ("normalSourceObject", "normalTexture")):
            if source.get(key):
                texture = textures[source[key].casefold()]
                layer[role] = texture["resourceId"]
                used.add(layer[role])
        assert "diffuseTexture" in layer
        assert ("normalTexture" in layer) == (layer["layerIndex"] in (2, 4, 5))
        if "normalTexture" not in layer:
            # The native compiler disabled this branch; its unused scalar is
            # retained in the source contract, never treated as an active input.
            layer["factors"][1] = 0.0
        layers.append(layer)
    assert not allocations
    assert {int(r["weight"][0]) for r in layers} == set(range(len(weight_ids)))
    row = dict(assetId=contract["assetId"], materialName=landscape.MATERIAL_NAME,
        sourceMaterial=contract["sourceMaterial"], family="bg-source-landscape-opaque",
        castsShadow=contract["castsShadow"], sourceLandscape=dict(
            grid=[component.section_base_x, component.section_base_y, 62, 31],
            weightmapScaleBias=list(component.weightmap_scale_bias),
            heightmapScaleBias=list(component.heightmap_scale_bias), weightmaps=weight_ids,
            heightmapTexture=height_id, layers=sorted(layers, key=lambda r: r["layerIndex"])))
    return row, used


def extract_source_textures(source_root, contracts, selected, package_root, umodel, output):
    """Reuse the original compressed-mip extractor instead of baking/recompressing."""
    sys.path.insert(0, str(ROOT / "Tools/LevelPlacementExtractor"))
    import extract_ue3_texture_mips as texture_mips

    wanted = {r[key].casefold() for contract in contracts.values()
              if not selected or contract["assetId"] in selected
              for r in contract["layers"] for key in ("diffuseSourceObject", "normalSourceObject") if r.get(key)}
    index = read_json(source_root / "MasterMaterial/DependencySerials/dependency_serial_index.json")
    packages = {p["logicalPackage"]: p for p in index["packages"]}
    checked_packages, resources, found = set(), [], set()
    for dep in index["dependencies"]:
        if dep["parameterPath"].casefold() not in wanted:
            continue
        package = packages[dep["logicalPackage"]]
        physical = package_root / package["physicalPackageFile"]
        if physical not in checked_packages:
            assert sha(physical.read_bytes()) == package["physicalPackageSha256"].lower(), f"source package changed: {physical}"
            checked_packages.add(physical)
        expected = source_root / "MasterMaterial/dds" / dep["logicalPackage"] / "Texture2D" / (dep["objectName"] + ".dds")
        decoded = output / "OriginalTextures" / (dep["logicalPackage"] + "_" + dep["objectName"] + ".dds")
        receipt = decoded.with_suffix(".receipt.json")
        result = texture_mips.extract_texture_mips(source_object=dep["parameterPath"], source_package=physical,
            package_root=package_root, umodel=umodel, expected_mip0=expected, output=decoded,
            scratch_root=output / "OriginalTextureScratch", receipt=receipt)
        data = decoded.read_bytes()
        digest = sha(data)
        assert digest == result["outputSHA256"] and result["mip0CompressedBytesIdentical"]
        srgb = next((p["decoded"] for p in dep["taggedProperties"] if p["name"].casefold() == "srgb"), True)
        resource_id = f"{BERN_PREFIX}/NativeLayers/Textures/{digest[:12]}_{dep['objectName'].lower()}.dds"
        resources.append(dict(sourceObject=dep["parameterPath"], sourcePackage=str(physical),
            sourcePackageSha256=package["physicalPackageSha256"].lower(), resourceId=resource_id,
            candidatePath=str(decoded), sha256=digest, bytes=len(data), format=result["fourCC"],
            width=dep["sizeX"], height=dep["sizeY"], mipCount=result["mipCount"], sRGB=srgb,
            mip0CompressedBytesIdentical=True, mips=result["mips"], receipt=str(receipt),
            recompressed=False, generatedMips=0))
        found.add(dep["parameterPath"].casefold())
    assert found == wanted, f"source texture dependency missing: {wanted - found}"
    write_json(output / "source-texture-closure.json", dict(resources=resources))
    return resources


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, required=True, help="Lossless extractor SourceRaw directory")
    parser.add_argument("--source-contract", type=Path)
    parser.add_argument("--texture-closure", type=Path, help="Original compressed-mip DDS receipt; payload hashes are checked")
    parser.add_argument("--package-root", type=Path, help="Original ReleasePC/Packages; used when --texture-closure is omitted")
    parser.add_argument("--umodel", type=Path, help="Original-compatible UModel; extracts complete compressed mip chains")
    parser.add_argument("--baseline-resources", type=Path, default=ROOT / "Client/Bin/Resources")
    parser.add_argument("--converter", type=Path, default=ROOT / "Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--only-asset", action="append", default=[])
    parser.add_argument("--geometry-only", action="store_true")
    args = parser.parse_args()
    source_root, output = args.source_root.resolve(), args.output.resolve()
    assert output.is_relative_to((ROOT / "out").resolve()), "candidates must stay under repository out"
    assert not output.exists(), "use a fresh candidate directory"
    if not args.geometry_only and (not args.source_contract or not (args.texture_closure or (args.package_root and args.umodel))):
        parser.error("native candidates require --source-contract and either --texture-closure or --package-root plus --umodel")
    contract_hash = None
    contracts = {}
    if not args.geometry_only:
        contract_data = args.source_contract.read_bytes()
        contract_hash = sha(contract_data)
        source_contract = json.loads(contract_data.decode("utf-8-sig"))
        assert source_contract["schema"] == "lostark.source-landscape-painted-contract.v1"
        assert source_contract["uvScale"] == .1 and abs(source_contract["rotationRadiansMultiplier"] - 3.140000104904175) < 1e-10
        contracts = {r["assetId"]: r for r in source_contract["components"]}
        assert len(contracts) == len(source_contract["components"]), "duplicate source contract component"
    output.mkdir(parents=True)
    texture_list = []
    if not args.geometry_only:
        texture_list = (read_json(args.texture_closure)["resources"] if args.texture_closure else
            extract_source_textures(source_root, contracts, set(args.only_asset),
                args.package_root.resolve(), args.umodel.resolve(), output))
    textures = {r["sourceObject"].casefold(): r for r in texture_list}
    resources, baseline_dependencies, rows, checks, used_textures = {}, {}, [], [], set()
    for path in sorted(source_root.glob("*/Components/component_*.json")):
        basic = read_json(path)
        asset = basic["assetId"]
        if args.only_asset and asset not in args.only_asset:
            continue
        doc, component, proxy = load_component(path, source_root)
        resource_id = f"{BERN_PREFIX}/{asset}/{asset}.wmodel"
        baseline_path = args.baseline_resources / resource_id
        baseline = baseline_path.read_bytes()
        folder = output / "Resources" / Path(resource_id).parent
        folder.mkdir(parents=True)
        gltf = output / "SourceDerived" / asset / (asset + ".gltf")
        gltf.parent.mkdir(parents=True)
        geometry = landscape.write_component_gltf(component, proxy, gltf, asset,
            dict(tiling=1, rotation=0), source_painted_surface=True)
        diffuse = baseline_path.parent / "textures/baked_diffuse.png"
        normal = baseline_path.parent / "textures/baked_normal.png"
        output_model = output / "Resources" / resource_id
        command = [str(args.converter.resolve()), landscape.repository_argument(gltf), "-o",
            landscape.repository_argument(output_model), "--pretransform", "--no-auto-textures", "--scale", "100",
            "--material-remap", f"{landscape.MATERIAL_NAME}={landscape.repository_argument(diffuse)}",
            "--normal-remap", f"{landscape.MATERIAL_NAME}={landscape.repository_argument(normal)}"]
        run = subprocess.run(command, cwd=ROOT, capture_output=True)
        (gltf.parent / "cook.log").write_bytes(run.stdout + run.stderr)
        assert run.returncode == 0, f"converter failed: {asset}"
        payload = output_model.read_bytes()
        check = verify_geometry(payload, component, proxy, baseline)
        check.update(assetId=asset, sourceComponentPath=str(path), sourceComponentSha256=sha(path.read_bytes()),
            sourcePlacementId=f"{component.logical_package}:landscape:export:{component.export_index}",
            resourceId=resource_id, baselineSha256=sha(baseline), sha256=sha(payload), geometry=geometry)
        checks.append(check)
        resources[resource_id] = dict(resourceId=resource_id, candidatePath=str(output_model), sha256=sha(payload),
            bytes=len(payload), oldSha256=sha(baseline), role="source-painted-geometry")
        for texture in (diffuse, normal):
            data = texture.read_bytes()
            assert (folder / "textures" / texture.name).read_bytes() == data
            name = f"{BERN_PREFIX}/{asset}/textures/{texture.name}"
            baseline_dependencies[name] = dict(resourceId=name, path=str(texture.resolve()), sha256=sha(data), bytes=len(data),
                role="unchanged embedded WMaterial fallback; native family uses sourceLandscape inputs")
        weight_ids, height_id = [], None
        for role, texture in [("Height", component.height_texture)] + [("Weights", w) for w in component.weight_textures]:
            data = bgra_dds(texture)
            name = f"{BERN_PREFIX}/NativeLayers/{role}/{sha(data)[:12]}_{component.logical_package.split('_')[-1].lower()}_{texture.export_index:05d}.dds"
            dst = output / "Resources" / name
            write_same(dst, data)
            resources[name] = dict(resourceId=name, candidatePath=str(dst), sha256=sha(data), bytes=len(data), role=role.lower(),
                format="A8R8G8B8", sRGB=False, mipCount=len(texture.mips), generatedMips=0,
                sourceMipPayloadsPreservedExactly=True, mips=[dict(level=m.level, width=m.width, height=m.height, sha256=sha(m.bgra)) for m in texture.mips])
            if role == "Height":
                height_id = name
            else:
                weight_ids.append(name)
        if not args.geometry_only:
            row, used = typed_row(contracts[asset], component, weight_ids, height_id, textures)
            rows.append(row)
            used_textures.update(used)
            check["sourceShaderMapKey"] = contracts[asset]["shaderMapKey"]
            check["sourcePixelShaderId"] = contracts[asset]["pixelShaderId"]
            check["disabledNormalScalarProvenance"] = [dict(layerIndex=r["layerIndex"],
                sourceUnusedNormalIntensity=r["factors"][1], emittedRuntimeNormalIntensity=0.0)
                for r in contracts[asset]["layers"] if not r.get("normalSourceObject")]
        assert baseline_path.read_bytes() == baseline, f"baseline changed during cook: {asset}"
        print("VALIDATED", asset, check["triangleCount"], "triangles", flush=True)
    assert checks and (not args.only_asset or set(args.only_asset) == {c["assetId"] for c in checks})
    if not args.geometry_only:
        assert {c["assetId"] for c in checks} == (set(args.only_asset) if args.only_asset else set(contracts))
        assert sha(args.source_contract.read_bytes()) == contract_hash, "source contract changed during cook"
    for resource in texture_list:
        if resource["resourceId"] not in used_textures:
            continue
        data = Path(resource["candidatePath"]).read_bytes()
        assert sha(data) == resource["sha256"] and not resource["recompressed"] and resource["generatedMips"] == 0
        target = output / "Resources" / resource["resourceId"]
        write_same(target, data)
        resources[resource["resourceId"]] = {**resource, "candidatePath": str(target)}
    write_json(output / "mapmaterials.rows.json", {"materials": rows})
    summary = dict(components=len(checks), materialRows=len(rows), sourceTriangles=sum(c["triangleCount"] for c in checks),
        sourceHoleQuads=sum(c["holeQuadCount"] for c in checks), resourceCount=len(resources),
        resourceBytes=sum(r["bytes"] for r in resources.values()), unchangedEmbeddedTextureCount=len(baseline_dependencies),
        allGeometryAndHolesPreserved=True, allFacesUseComponentUV0=True, placementChanges=0,
        maximumPositionErrorMeters=max(c["maximumErrors"]["positionMeters"] for c in checks))
    write_json(output / "candidate-manifest.json", dict(schema="source-landscape-runtime-candidate.v1",
        status="validated candidate only; not installed or GPU verified", sourceRoot=str(source_root),
        sourceContract=None if args.geometry_only else dict(path=str(args.source_contract.resolve()), sha256=contract_hash),
        converterSha256=sha(args.converter.read_bytes()), summary=summary, geometryChecks=checks,
        resources=list(resources.values()), unchangedDependencies=list(baseline_dependencies.values()),
        materialRowsPath=str(output / "mapmaterials.rows.json")))
    print(json.dumps(summary), flush=True)


if __name__ == "__main__":
    main()
