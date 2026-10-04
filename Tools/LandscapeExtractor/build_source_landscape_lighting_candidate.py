"""Stage original Bern Landscape RNM/shadow mips and placement-lighting patches.

Reads each current component's native tail and its own ShadowMap2D. Does not
install, publish, change geometry, or manufacture missing texture mip levels.
The audited EFEngine CPU proof supplies the normalized grid-to-lightmap mapping;
unsupported source dimensions/resolution or a changed engine fail explicitly.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import struct
import subprocess
import sys

import extract_ue3_landscape as landscape
from build_source_landscape_candidate import ROOT, read_json, sha, write_json, write_same

sys.path.insert(0, str(ROOT / "Tools/LevelPlacementExtractor"))
import extract_ue3_texture_mips as texture_mips

PACKAGES = ("LV_BER_BERNCASTLE_T_LAND01", "LV_BER_BERNCASTLE_T_LAND02")
LIGHT_GUID = "79ad76d6208c814790c50f5f74b7b8a0"
CPU_ENGINE_SHA256 = "6d107bf77f7dee8111882bcd3309f05dd2217d4bf8c7aa730967af04ce6d2c47"
NATIVE_UV = [0.953125, 0.953125, 0.015372984111309052, 0.015372984111309052]
SHADOW_DEFAULT_PACKAGES = {
    "NE1FENCQ4UNE9ZPRENOQS.u": "ffddf056224829d8b640e35e9440f765555046691e45423db4f13f6bd89d6415",
    "9L6NC53E9WINO5FELWUN0.u": "24057399039ef5bf0edd54f0c51f68bdd6a1da7035962f3a1963c62c01fe5cd2",
}


def compose_uv(scale, bias):
    # Native VF grid/padding transform precedes the component's atlas transform.
    result = ([NATIVE_UV[i] * scale[i] for i in range(2)],
              [NATIVE_UV[i + 2] * scale[i] + bias[i] for i in range(2)])
    assert all(0 <= b < 1 and 0 < s <= 1 and s + b <= 1 for s, b in zip(*result))
    return result


def vector(props, name, default):
    if name not in props:
        return list(default)
    value = props[name]["value"]
    return [value["x"], value["y"]]


class Candidate:
    def __init__(self, args):
        self.args = args
        self.textures = {}

    def texture(self, package, physical, package_hash, logical, reference):
        assert 0 < reference <= len(package.exports)
        export = package.exports[reference - 1]
        identity = logical.lower() + "." + export.object_name
        if identity in self.textures:
            return self.textures[identity]
        raw = landscape.export_serial(package, export)
        props, end = landscape.UE3.parse_tagged_properties(raw, package.names, package.summary.version)
        pixel_format = props["format"]["value"]
        filename = logical.lower() + "_" + export.object_name + ".dds"
        resource_id = "Map/Lighting/Bern/" + filename
        target = self.args.output / "Resources" / resource_id
        receipt_path = self.args.output / "Receipts" / (filename + ".json")
        cached = self.args.texture_cache / filename if self.args.texture_cache else None
        receipt = None
        # Old G8 cache receipts can authenticate DDS bytes produced by the
        # former length-only decoder. Always decode shadows from native flags.
        if pixel_format == "pf_dxt1" and cached and cached.is_file():
            cached_receipt = cached.with_suffix(".dds.receipt.json")
            if cached_receipt.is_file():
                receipt = read_json(cached_receipt)
                assert receipt["sourceSerialSHA256"] == sha(raw), f"stale source texture: {identity}"
                cached_package_hashes = ([p["sha256"] for p in receipt["sourcePackages"]]
                                         if "sourcePackages" in receipt else [receipt["sourcePackageSHA256"]])
                assert package_hash in cached_package_hashes
                assert receipt["outputSHA256"] == sha(cached.read_bytes())
                write_same(target, cached.read_bytes())
        if receipt is None and pixel_format == "pf_dxt1":
            exported = self.args.output / "OriginalMip0" / logical
            command = [str(self.args.umodel), "-export", "-game=lostark", "-kr", "-nameresolve",
                       "-path=" + str(self.args.package_root), "-out=" + str(exported),
                       "-dds", logical, export.object_name]
            run = subprocess.run(command, cwd=self.args.umodel.parent, capture_output=True,
                                 creationflags=subprocess.CREATE_NO_WINDOW if sys.platform == "win32" else 0,
                                 timeout=90)
            write_same(self.args.output / "Logs" / (filename + ".umodel.log"), run.stdout + run.stderr)
            assert run.returncode == 0, f"UModel failed: {identity}"
            hits = list(exported.rglob(export.object_name + ".dds"))
            assert len(hits) == 1, f"ambiguous UModel mip0: {identity}"
            receipt = texture_mips.extract_texture_mips(source_object=identity, source_package=physical,
                package_root=self.args.package_root, umodel=self.args.umodel, expected_mip0=hits[0],
                output=target, scratch_root=self.args.output / "MipScratch")
        elif receipt is None and pixel_format == "pf_g8":
            records, _ = texture_mips.parse_native_mips(raw, end, export.serial_offset)
            payloads = [landscape.decompress_texture_bulk(r.packed, r.elements, r.flags) for r in records]
            width, height = records[0].width, records[0].height
            assert all(len(data) == r.width * r.height for data, r in zip(payloads, records))
            assert [(r.width, r.height) for r in records] == [
                (max(1, width >> i), max(1, height >> i)) for i in range(len(records))]
            header = [0] * 31
            header[:7] = [124, 0x2100F, height, width, width, 0, len(records)]
            header[18:26] = [32, 0x20000, 0, 8, 255, 0, 0, 0]
            header[26] = 0x401008
            data = b"DDS " + struct.pack("<31I", *header) + b"".join(payloads)
            write_same(target, data)
            receipt = dict(sourceObject=identity, sourceSerialSHA256=sha(raw), sourcePackage=str(physical),
                sourcePackageSHA256=package_hash, format="PF_G8", mipCount=len(records),
                sourceMipBytesPreserved=True, mipPayloadSHA256=[sha(p) for p in payloads],
                outputSHA256=sha(data))
        assert receipt is not None and pixel_format in ("pf_dxt1", "pf_g8"), identity
        assert sha(target.read_bytes()) == receipt["outputSHA256"]
        assert receipt["sourceSerialSHA256"] == sha(raw)
        write_json(receipt_path, {**receipt, "output": str(target)})
        result = dict(sourceObject=identity, sourceSerialSHA256=sha(raw), sourcePackage=str(physical),
            sourcePackageSHA256=package_hash, exportIndex0=reference - 1, resourceId=resource_id,
            candidatePath=str(target), sha256=sha(target.read_bytes()), bytes=target.stat().st_size,
            sourcePixelFormat=pixel_format, width=props["sizex"]["value"], height=props["sizey"]["value"],
            mipCount=receipt["mipCount"], sRGB=False, generatedMips=0, recompressed=False,
            receipt=str(receipt_path))
        self.textures[identity] = result
        return result

    def component(self, package, physical, package_hash, logical, component, resolution):
        assert (component.component_size_quads, component.subsection_size_quads,
                component.num_subsections, resolution) == (62, 31, 2, 4.0), "unaudited CPU UV inputs"
        raw = landscape.export_serial(package, package.exports[component.export_index])
        props, end = landscape.UE3.parse_tagged_properties(raw, package.names, package.summary.version)
        reader = landscape.UE3.Reader(raw[end:])
        kind, count = reader.unpack("<2i")
        assert kind == 2 and 0 < count < 64
        guids = [reader.read(16).hex() for _ in range(count)]
        coefficients = []
        for role in ("average", "directional", "unused"):
            reference, scale = reader.i32(), list(reader.unpack("<3f"))
            texture = self.texture(package, physical, package_hash, logical, reference) if reference else None
            coefficients.append(dict(role=role, reference=reference, scale=scale, texture=texture))
        coords = list(reader.unpack("<4f"))
        assert reader.offset == len(raw) - end and coefficients[2]["reference"] == 0
        assert coefficients[0]["texture"]["sourceObject"].split(".")[-1].startswith("normalizedaveragecolor")
        assert coefficients[1]["texture"]["sourceObject"].split(".")[-1].startswith("directionalmaxcomponent")
        shadow_ref, = props["shadowmaps"]["value"]
        assert package.exports[shadow_ref - 1].archetype_index == 0, "non-default ShadowMap2D archetype"
        shadow_raw = landscape.export_serial(package, package.exports[shadow_ref - 1])
        shadow, shadow_end = landscape.UE3.parse_tagged_properties(shadow_raw, package.names, package.summary.version)
        assert shadow_end == len(shadow_raw) and not shadow["bisshadowfactortexture"]["value"]
        assert shadow["lightguid"]["value"]["hex"] == LIGHT_GUID
        assert "coordinatescale" in shadow, "unsupported absent shadow coordinate scale"
        shadow_texture = self.texture(package, physical, package_hash, logical, shadow["texture"]["value"])
        shadow_scale = vector(shadow, "coordinatescale", [1.0, 1.0])
        # Source ShadowMap2D CDO has only bIsShadowFactorTexture; its superclass
        # Core.Default__Object is empty. A newly declared, zero-initialized
        # Vector2D with no instance/archetype override therefore stays (0, 0).
        shadow_bias = vector(shadow, "coordinatebias", [0.0, 0.0])
        scale, bias = compose_uv(coords[:2], coords[2:])
        shadow_uv_scale, shadow_uv_bias = compose_uv(shadow_scale, shadow_bias)
        asset_id = landscape.stable_asset_id(component)[1]
        material = dict(assetId=asset_id, bakedLighting=dict(
            averageTexture=coefficients[0]["texture"]["resourceId"],
            directionalTexture=coefficients[1]["texture"]["resourceId"], colorSpace="linear",
            staticShadow=dict(texture=shadow_texture["resourceId"], lightGuid=LIGHT_GUID, lightChannel=1,
                penumbraWidth=0.05, penumbraBasis="PROJECT_ADAPTER", shadowExponent=2.0)))
        placement = dict(sourcePlacementId=f"{logical}:landscape:export:{component.export_index}",
            assetId=asset_id, coordinateScale=scale, coordinateBias=bias,
            averageScale=coefficients[0]["scale"], directionalScale=coefficients[1]["scale"],
            shadowCoordinateScale=shadow_uv_scale, shadowCoordinateBias=shadow_uv_bias)
        evidence = dict(assetId=asset_id, logicalPackage=logical, componentExportIndex0=component.export_index,
            componentSHA256=sha(raw), sourcePlacementId=placement["sourcePlacementId"],
            grid=[component.section_base_x, component.section_base_y, component.component_size_quads,
                  component.subsection_size_quads], numSubsections=component.num_subsections,
            staticLightingResolution=resolution, nativeTailBytes=len(raw) - end, nativeTailFullyConsumed=True,
            coefficients=coefficients, bakedLightGuids=guids, coordinateScale=coords[:2], coordinateBias=coords[2:],
            staticShadow=dict(exportIndex0=shadow_ref - 1, sourceSerialSHA256=sha(shadow_raw),
                lightGuid=LIGHT_GUID, coordinateScale=shadow_scale, coordinateBias=shadow_bias,
                coordinateBiasSerialized="coordinatebias" in shadow, texture=shadow_texture),
            normalizedMeshUVScaleBias=NATIVE_UV, composedPlacementLighting=placement)
        return material, placement, evidence


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-contract", type=Path, required=True)
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--umodel", type=Path, required=True)
    parser.add_argument("--uv-proof", type=Path, required=True, help="Audited current EFEngine CPU binding/arithmetic receipt")
    parser.add_argument("--texture-cache", type=Path, help="Prior PF_DXT1 RNM DDS files with .dds.receipt.json; hashes rechecked. PF_G8 shadows always decode from source bulk flags")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    for key in ("source_contract", "package_root", "umodel", "uv_proof", "output", "texture_cache"):
        if getattr(args, key) is not None:
            setattr(args, key, getattr(args, key).resolve())
    assert args.output.is_relative_to((ROOT / "out").resolve()) and not args.output.exists(), "choose fresh repository out directory"
    contract_data, proof_data = args.source_contract.read_bytes(), args.uv_proof.read_bytes()
    contract, proof = json.loads(contract_data), json.loads(proof_data)
    assert contract["schema"] == "lostark.source-landscape-painted-contract.v1"
    wanted = {r["assetId"] for r in contract["components"]}
    assert len(wanted) == len(contract["components"]) == 42
    assert proof["status"] == "CURRENT_INSTALLED_SOURCE_CPU_DATAFLOW_VERIFIED"
    assert proof["engineDLLSHA256"] == CPU_ENGINE_SHA256 == sha(Path(proof["engineDLL"]).read_bytes())
    assert proof["arithmetic"]["normalizedMeshUVScaleBias"] == NATIVE_UV
    assert proof["inputs"] == dict(componentSizeQuads=62, subsectionSizeQuads=31,
                                    numSubsections=2, staticLightingResolution=4.0, pixelFormatBlockSize=[4, 4])
    shadow_defaults = []
    for filename, digest in SHADOW_DEFAULT_PACKAGES.items():
        path = args.package_root.parent / filename
        assert sha(path.read_bytes()) == digest, "source ShadowMap2D default chain changed"
        shadow_defaults.append(dict(path=str(path), sha256=digest))
    candidate = Candidate(args)
    materials, placements, components, packages = [], [], [], []
    for logical in PACKAGES:
        physical = landscape.UE3.resolve_physical_package(args.umodel, args.package_root, logical, "kr")
        package_hash = sha(physical.read_bytes())
        package = landscape.load_package(physical, logical, landscape.UE3.LOSTARK_KR_AES_KEY)
        proxy, source_components, _ = landscape.parse_landscape_package(package)
        proxy_raw = landscape.export_serial(package, package.exports[proxy.export_index])
        proxy_props, _ = landscape.UE3.parse_tagged_properties(proxy_raw, package.names, package.summary.version)
        resolution = proxy_props["staticlightingresolution"]["value"]
        for component in source_components:
            if landscape.stable_asset_id(component)[1] not in wanted:
                continue
            material, placement, evidence = candidate.component(package, physical, package_hash, logical, component, resolution)
            if logical == PACKAGES[0] and component.export_index == proof["componentExportIndex0"]:
                assert package_hash == proof["sourcePackageSHA256"]
                assert evidence["componentSHA256"] == proof["componentSerialSHA256"]
                assert sha(proxy_raw) == proof["proxySerialSHA256"]
            materials.append(material)
            placements.append(placement)
            components.append(evidence)
        assert sha(physical.read_bytes()) == package_hash, "source package changed during extraction"
        packages.append(dict(logicalPackage=logical, path=str(physical), sha256=package_hash,
                             proxySerialSHA256=sha(proxy_raw), staticLightingResolution=resolution))
    assert len(components) == len(wanted) and {r["assetId"] for r in components} == wanted
    assert sha(args.source_contract.read_bytes()) == sha(contract_data)
    write_json(args.output / "mapmaterials.patch.json", dict(materials=materials, placementLighting=placements))
    write_same(args.output / "cpu-uv-proof.json", proof_data)
    resources = list(candidate.textures.values())
    summary = dict(componentCount=len(components), materialPatchCount=len(materials), placementLightingCount=len(placements),
                   textureCount=len(resources), resourceBytes=sum(r["bytes"] for r in resources),
                   generatedMips=0, geometryChanges=0, installed=False)
    write_json(args.output / "candidate-manifest.json", dict(schema="source-landscape-lighting-candidate.v1",
        status="source-validated candidate only; no installation or visual validation",
        sourceContract=dict(path=str(args.source_contract), sha256=sha(contract_data)),
        uvProof=dict(path=str(args.output / "cpu-uv-proof.json"), sha256=sha(proof_data)),
        shadowDefaultSources=shadow_defaults,
        sourcePackages=packages, components=components, resources=resources, summary=summary,
        materialPatchPath=str(args.output / "mapmaterials.patch.json")))
    print(json.dumps(summary), flush=True)


if __name__ == "__main__":
    main()
