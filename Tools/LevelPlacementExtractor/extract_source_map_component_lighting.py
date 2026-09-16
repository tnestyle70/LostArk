#!/usr/bin/env python3
"""Extract v868 StaticMeshComponent RNM and instance environment evidence."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import struct
from typing import Any, Callable

import extract_ue3_effect_material_closure as closure
import extract_ue3_placements as ue
from source_extraction_io import write_atomic, write_pair


FORMAT = "lostark-source-map-component-lighting"


class UnsupportedNative(ValueError):
    def __init__(self, message: str, offset: int, decoded: dict[str, Any]):
        super().__init__(message)
        self.offset, self.decoded = offset, decoded


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def finite(values: tuple | list, label: str, nonnegative: bool = False) -> list[float]:
    require(all(isinstance(value, (float, int)) and not isinstance(value, bool) and math.isfinite(value)
                and (not nonnegative or value >= 0) for value in values), f"invalid {label}")
    return [float(value) for value in values]


def tagged_property(props: dict, name: str) -> dict[str, Any]:
    matches = [item for key, item in props.items() if key.casefold() == name.casefold()]
    require(len(matches) <= 1, f"duplicate property {name}")
    return {"propertyName": name, "propertyPresent": bool(matches),
            "status": "SERIALIZED_INSTANCE_PROPERTY" if matches else "INSTANCE_PROPERTY_ABSENT",
            "tag": matches[0] if matches else None,
            "value": matches[0].get("value") if matches else None}


def decode_native_lighting(tail: bytes, reference: Callable[[int], dict]) -> dict[str, Any]:
    reader = ue.Reader(tail)
    lod_count = reader.i32()
    require(0 <= lod_count <= 64, f"invalid LOD count {lod_count}")
    header: dict[str, Any] = {"lodCount": lod_count}
    if lod_count == 0:
        require(reader.offset == len(tail), "zero LOD lighting has unexpected native tail")
        return {**header, "status": "NO_LOD_LIGHTING_DATA", "nativeTailCompletelyConsumed": True}
    if lod_count != 1:
        raise UnsupportedNative("multiple LOD lighting records are not implemented", reader.offset, header)
    shadow_count = reader.i32()
    require(0 <= shadow_count <= 4096, "invalid shadow reference count")
    shadows = [reference(reader.i32()) for _ in range(shadow_count)]
    header["shadowReferences"] = shadows
    shadow_vertices = reader.i32()
    require(0 <= shadow_vertices <= 20_000_000, "invalid vertex shadow count")
    header["vertexShadowCount"] = shadow_vertices
    if shadows or shadow_vertices:
        raise UnsupportedNative("shadow-map or vertex-shadow payload is not implemented", reader.offset, header)
    kind = reader.i32()
    header["lightMapKind"] = kind
    if kind != 2:
        raise UnsupportedNative(f"lightmap kind {kind} is not implemented", reader.offset, header)
    lighting: dict[str, Any] = {**header, "status": "RNM_TEXTURE_LIGHTMAP"}
    if kind == 2:
        guid_count = reader.i32()
        require(0 <= guid_count <= 4096, "invalid baked light GUID count")
        lighting["bakedLightGuids"] = [reader.read(16).hex() for _ in range(guid_count)]
        lighting["bakedLightGuidEncoding"] = "original serialized 16-byte order"
        coefficients = []
        for index in range(3):
            ref = reference(reader.i32())
            scales = finite(reader.unpack("<3f"), f"coefficient {index} scale", True)
            require(ref["packageIndex"] == 0 or ref["className"].casefold() == "lightmaptexture2d",
                    f"coefficient {index} is not a LightMapTexture2D")
            coefficients.append({**ref, "scale": scales})
        require(coefficients[0]["packageIndex"] != 0 and coefficients[1]["packageIndex"] != 0,
                "RNM average/directional texture is null")
        # Semantic channel names are verified from the actual LightMapTexture2D
        # exports; never guess a texture role from the material/mesh name.
        require(coefficients[0]["objectPath"].rsplit(".", 1)[-1].casefold().startswith("normalizedaveragecolor")
                and coefficients[1]["objectPath"].rsplit(".", 1)[-1].casefold().startswith("directionalmaxcomponent"),
                "unsupported RNM channel semantics")
        if coefficients[2]["packageIndex"] != 0:
            raise UnsupportedNative("third RNM coefficient texture is not implemented", reader.offset,
                                    {**header, "coefficients": coefficients, "bakedLightGuids": lighting["bakedLightGuids"]})
        coords = finite(reader.unpack("<4f"), "RNM coordinates", True)
        require(all(value <= 1 for value in coords), "RNM coordinates exceed atlas range")
        lighting.update(coefficients=coefficients, average=coefficients[0], directional=coefficients[1],
                        averageTexture=coefficients[0]["sourceObject"], directionalTexture=coefficients[1]["sourceObject"],
                        averageScale=coefficients[0]["scale"], directionalScale=coefficients[1]["scale"],
                        coordinateScale=coords[:2], coordinateBias=coords[2:])
    has_color = reader.read(1)[0]
    require(has_color in (0, 1), "invalid vertex color flag")
    color, vertices = b"", 0
    if has_color:
        stride, vertices, element_size, count = reader.unpack("<4I")
        require(stride == element_size == 4 and vertices == count and 0 < count <= 20_000_000,
                "unsupported vertex color stream layout")
        color = reader.read(count * 4)
    lighting.update(hasColor=bool(has_color), colorVertexCount=vertices,
                    colorSHA256=digest(color), colorBytes=len(color), colorEncoding="original BGRA8 bytes")
    terminal = reader.read(len(tail) - reader.offset)
    if terminal != bytes(4):
        raise UnsupportedNative("unrecognized terminal component tail", len(tail) - len(terminal), lighting)
    lighting.update(nativeTailCompletelyConsumed=True, terminalZeroBytes=4)
    return lighting


class ComponentLightingExtractor:
    def __init__(self, package_path: Path, logical_package: str, area_id: str):
        require(bool(re.fullmatch(r"[A-Za-z0-9_]+", logical_package)), "invalid logical package name")
        self.logical_package, self.area_id = logical_package, area_id
        self.package = closure.load_package(package_path.resolve(strict=True), ue.LOSTARK_KR_AES_KEY)
        require(self.package.summary.version == 868, f"unsupported package version {self.package.summary.version}")
        self.texture_evidence: dict[str, dict[str, Any]] = {}

    def reference(self, index: int) -> dict[str, Any]:
        package = self.package
        require(-len(package.imports) <= index <= len(package.exports), f"invalid package reference {index}")
        if index == 0:
            return {"packageIndex": 0, "objectPath": None, "sourceObject": None, "className": None}
        path = ue.package_ref_path(index, package.imports, package.exports)
        source = f"{self.logical_package}.{path}" if index > 0 else path
        class_name = (ue.package_ref_name(package.exports[index - 1].class_index, package.imports, package.exports)
                      if index > 0 else package.imports[-index - 1].class_name)
        return {"packageIndex": index, "objectPath": path, "sourceObject": source, "className": class_name}

    def texture(self, reference: dict[str, Any]) -> None:
        if not reference["packageIndex"] or reference["sourceObject"] in self.texture_evidence:
            return
        evidence = dict(reference)
        index = reference["packageIndex"]
        if index > 0:
            entry = self.package.exports[index - 1]
            serial = self.package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
            evidence.update(sourcePackage=str(self.package.path), sourceExportIndex0=entry.index,
                            serialOffset=entry.serial_offset, serialBytes=entry.serial_size, serialSHA256=digest(serial),
                            status="LOCAL_SOURCE_EXPORT_IDENTIFIED")
        else:
            evidence["status"] = "EXTERNAL_SOURCE_EXPORT_NOT_READ"
        self.texture_evidence[reference["sourceObject"]] = evidence

    def environment(self, props: dict) -> dict[str, Any]:
        maps = tagged_property(props, "pbrenvironmentmapoverrides")
        references = []
        if maps["propertyPresent"]:
            require(isinstance(maps["value"], list) and all(type(value) is int for value in maps["value"]),
                    "environment maps must be a reference array")
            references = [self.reference(value) for value in maps["value"]]
            require(all(row["packageIndex"] == 0 or row["className"].casefold() == "texturecube" for row in references),
                    "environment map reference is not a TextureCube")
        color = tagged_property(props, "pbrenvironmentcoloroverride")
        if color["propertyPresent"]:
            decoded = closure.struct_linear_color(color["tag"])
            require(decoded is not None, "invalid environment LinearColor")
            color["value"] = finite(decoded, "environment color")
        angle = tagged_property(props, "pbrenvironmentcubemapangleoverride")
        if angle["propertyPresent"]:
            angle["value"] = finite([angle["value"]], "environment angle")[0]
        minimum = tagged_property(props, "minroughness")
        if minimum["propertyPresent"]:
            minimum["value"] = finite([minimum["value"]], "component minroughness")[0]
        light_environment = tagged_property(props, "lightenvironment")
        if light_environment["propertyPresent"]:
            require(type(light_environment["value"]) is int, "invalid LightEnvironment reference")
            light_environment["reference"] = self.reference(light_environment["value"])
        cube_inputs = []
        for ref in references:
            if ref["packageIndex"] == 0:
                continue
            self.texture(ref)
            roughness = {"status": "EXTERNAL_TEXTURE_PROPERTY_NOT_READ", "propertyPresent": None, "value": None}
            if ref["packageIndex"] > 0:
                entry = self.package.exports[ref["packageIndex"] - 1]
                raw = self.package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
                cube_props, _ = ue.parse_tagged_properties(raw, self.package.names, self.package.summary.version)
                roughness = tagged_property(cube_props, "minroughness")
                if roughness["propertyPresent"]:
                    roughness["value"] = finite([roughness["value"]], "cube minroughness")[0]
            cube_inputs.append({"sourceObject": ref["sourceObject"], "owner": "TextureCube", "minimumRoughness": roughness})
        return {"maps": {**maps, "references": references}, "color": color, "angleDegrees": angle,
                "componentMinRoughnessTag": minimum, "cubeInputs": cube_inputs, "lightEnvironment": light_environment,
                "absenceMeaning": "No instance tag; inherited/CDO/native binder values have not been resolved",
                "runtimeTransform": "none; source angle/color preserved; no CB packing or floor inference"}

    def extract(self) -> dict[str, Any]:
        components, unsupported, failures, skipped = {}, [], [], []
        package = self.package
        for entry in package.exports:
            if ue.package_ref_name(entry.class_index, package.imports, package.exports).casefold() != "staticmeshcomponent":
                continue
            source_id = f"{self.logical_package}:export:{entry.index}"
            serial = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
            try:
                props, end = ue.parse_tagged_properties(serial, package.names, package.summary.version)
                mesh = closure.tagged_value(props, "staticmesh")
                if not isinstance(mesh, int) or mesh == 0:
                    skipped.append({"sourcePlacementId": source_id, "reason": "NO_SERIALIZED_STATICMESH_REFERENCE"})
                    continue
                actor = ue.resolve_component_actor(entry, package.imports, package.exports)
                require(actor is not None, "unsupported StaticMeshComponent owner")
                tail = serial[end:]
                row: dict[str, Any] = {"sourcePlacementId": source_id, "sourceExportIndex0": entry.index,
                                      "areaId": self.area_id, "componentObjectPath": self.reference(entry.index + 1)["sourceObject"],
                                      "actorExportIndex0": actor.index, "sourceMesh": self.reference(mesh),
                                      "sourcePackage": str(package.path), "serialSHA256": digest(serial),
                                      "propertyStreamEnd": end, "nativeTailBytes": len(tail), "nativeTailSHA256": digest(tail),
                                      "environment": self.environment(props)}
                try:
                    lighting = decode_native_lighting(tail, self.reference)
                    row["lighting"], row["status"] = lighting, lighting["status"]
                    for coefficient in lighting.get("coefficients", []):
                        self.texture(coefficient)
                except UnsupportedNative as error:
                    row.update(status="UNSUPPORTED_NATIVE_LAYOUT", lighting=None)
                    evidence = {"sourcePlacementId": source_id, "reason": str(error),
                                "offsetInNativeTail": error.offset, "decodedPrefix": error.decoded,
                                "nativeTailBytes": len(tail), "nativeTailSHA256": digest(tail), "nativeTailHex": tail.hex()}
                    row["unsupported"] = evidence
                    unsupported.append(evidence)
                components[source_id] = row
            except (ValueError, ue.ExtractionError, struct.error) as error:
                failures.append({"sourcePlacementId": source_id, "error": str(error),
                                 "serialSHA256": digest(serial), "serialHex": serial.hex()})
        require(digest(package.path.read_bytes()) == package.sha256, "source package changed during extraction")
        counts = {status: sum(row["status"] == status for row in components.values()) for status in sorted({row["status"] for row in components.values()})}
        return {"format": FORMAT, "formatVersion": 1, "logicalPackage": self.logical_package, "areaId": self.area_id,
                "source": {"path": str(package.path), "sha256": package.sha256, "packageVersion": package.summary.version},
                "components": components, "textures": list(self.texture_evidence.values()),
                "unsupported": unsupported, "failures": failures, "skipped": skipped,
                "summary": {"componentCount": len(components), "statusCounts": counts, "unsupportedCount": len(unsupported),
                            "failureCount": len(failures), "skippedCount": len(skipped)},
                "scope": "source evidence only; no texture export, runtime defaults, CB conversion, installation or visual admission"}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package", type=Path, required=True)
    parser.add_argument("--logical-package", required=True)
    parser.add_argument("--area-id", help="defaults to the logical package name")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        source, output, receipt = (path.resolve() for path in (args.package, args.output, args.receipt))
        require(len({source, output, receipt}) == 3, "source/output/receipt paths must differ")
        require(not output.is_relative_to(source.parent) and not receipt.is_relative_to(source.parent),
                "outputs must not be inside the original package directory")
        document = ComponentLightingExtractor(source, args.logical_package, args.area_id or args.logical_package).extract()
        status = "FAILED_OUTPUT_PRESERVED" if document["failures"] else "PARTIAL_UNSUPPORTED" if document["unsupported"] else "PASS"
        report = {"format": FORMAT + "-receipt", "formatVersion": 1, "status": status,
                  "source": document["source"], "summary": document["summary"], "unsupported": document["unsupported"],
                  "failures": document["failures"]}
        if not document["failures"]:
            payload = (json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + "\n").encode("utf-8")
            report["outputSHA256"] = digest(payload)
            write_pair(output, payload, receipt,
                       (json.dumps(report, ensure_ascii=False, indent=2) + "\n").encode("utf-8"))
        else:
            write_atomic(receipt, (json.dumps(report, ensure_ascii=False, indent=2) + "\n").encode("utf-8"))
        print(f"{status}: {json.dumps(document['summary'])}")
        return 1 if document["failures"] else 2 if document["unsupported"] else 0
    except (ValueError, ue.ExtractionError, OSError) as error:
        print(f"component lighting extraction failed: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
