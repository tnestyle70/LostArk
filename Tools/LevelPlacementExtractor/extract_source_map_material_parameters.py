#!/usr/bin/env python3
"""Resolve explicit source map Material/MIC parameters without runtime defaults."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import sys
from typing import Any

import extract_ue3_effect_material_closure as closure
import extract_ue3_material_graph as graph
import extract_ue3_placements as ue
from source_extraction_io import write_atomic, write_pair

EFFECT_TOOLS = Path(__file__).resolve().parents[1] / "EffectPipeline"
if str(EFFECT_TOOLS) not in sys.path:
    sys.path.insert(0, str(EFFECT_TOOLS))
import extract_ue3_material_shader_maps as shader_maps


FORMAT = "lostark-source-map-material-parameters"
STATIC_FORMAT = "lostark-v868-normal-byte-numbered-fname"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def full_source_name(value: Any) -> str:
    require(isinstance(value, str) and bool(re.fullmatch(r"[^.\\/:\s]+(?:\.[^.\\/:\s]+)+", value)),
            f"expected full package.object path: {value!r}")
    return value.casefold()


class MapStaticSetDecoder:
    """Local v868 layout adapter; never alters shared parser globals.

    The third array is FStaticNormalParameter (one BYTE + uint32 + GUID),
    not the fourth terrain array. Names are expanded once from their FName
    pair into a private name table before invoking the existing strict parser.
    Returned offsets and raw hashes continue to describe original bytes.
    """

    def decode(self, tail: bytes, base_id: bytes, names: list[str]) -> dict[str, Any]:
        require(bool(tail), "MIC native static resource absent; effective switches are unresolved")
        require(len(base_id) == 16 and base_id != bytes(16), "invalid base Material ID")
        require(tail.count(base_id) == 1, "MIC base Material ID absent or ambiguous")
        start = tail.index(base_id)
        cursor = start + 16
        converted = bytearray(tail[:cursor])
        expanded_names = list(names)
        source_offsets: dict[int, tuple[int, int]] = {}
        for array_name, stride in (("staticSwitchParameters", 32), ("staticComponentMaskParameters", 44),
                                   ("normalParameters", 29), ("terrainLayerWeightParameters", 32)):
            require(cursor + 4 <= len(tail), f"{array_name} count truncated")
            count = struct.unpack_from("<I", tail, cursor)[0]
            require(count <= 4096, f"{array_name} count unsupported")
            converted.extend(tail[cursor:cursor + 4])
            cursor += 4
            require(cursor + count * stride <= len(tail), f"{array_name} records truncated")
            for _ in range(count):
                raw = tail[cursor:cursor + stride]
                name_index, number = struct.unpack_from("<ii", raw)
                require(0 <= name_index < len(names) and number >= 0, f"{array_name} invalid FName")
                rendered = names[name_index] + (f"_{number - 1}" if number else "")
                source_offsets[len(converted)] = cursor, number
                converted.extend(struct.pack("<ii", len(expanded_names), 0))
                expanded_names.append(rendered)
                if array_name == "normalParameters":
                    converted.extend(struct.pack("<I", raw[8]))
                    converted.extend(raw[9:])
                else:
                    converted.extend(raw[8:])
                cursor += stride
        converted.extend(tail[cursor:])
        decoded = shader_maps.decode_static_set_from_tail(
            bytes(converted), base_id, expanded_names, shader_maps.POLICY_REQUIRE_NATIVE)
        static = decoded["staticParameterSet"]
        for array_name in ("staticSwitchParameters", "staticComponentMaskParameters", "normalParameters", "terrainLayerWeightParameters"):
            for row in static[array_name]:
                row["entryOffset"], row["sourceFNameNumber"] = source_offsets[row["entryOffset"]]
        static["byteSize"] = cursor - start
        static["rawSha256"] = digest(tail[start:cursor])
        semantic = {key: static[key] for key in ("baseMaterialIdHex", "staticSwitchParameters", "staticComponentMaskParameters", "normalParameters", "terrainLayerWeightParameters")}
        static["semanticSha256"] = shader_maps.canonical_json_sha256(semantic)
        decoded["nativeTailByteCount"] = len(tail)
        decoded["sourceStaticFormat"] = STATIC_FORMAT
        return decoded


def source_reference(package: closure.PackageData, logical_name: str, reference: int) -> str | None:
    if reference == 0:
        return None
    path = ue.package_ref_path(reference, package.imports, package.exports)
    return full_source_name(f"{logical_name}.{path}" if reference > 0 else path)


def checked_float(value: Any, label: str) -> float:
    require(isinstance(value, (int, float)) and not isinstance(value, bool), f"invalid scalar {label}")
    result = float(value)
    require(math.isfinite(result), f"nonfinite scalar {label}")
    return result


def assign(result: dict[str, Any], field: str, name: str, value: Any, evidence: dict[str, Any]) -> None:
    name = name.casefold()
    result[field][name] = value
    result["parameterSources"][field][name] = evidence
    result["unresolvedDefaults"] = [row for row in result["unresolvedDefaults"]
                                    if (row["field"], row["name"]) != (field, name)]


class MaterialResolver:
    def __init__(self, package_root: Path, umodel: Path, parent_static_proof: Path | None = None):
        self.package_root = package_root.resolve(strict=True)
        self.umodel = umodel.resolve(strict=True)
        self.packages: dict[str, closure.PackageData] = {}
        self.materials: dict[str, dict[str, Any]] = {}
        self.active: list[str] = []
        self.static_decoder = MapStaticSetDecoder()
        self.parent_static_proofs: dict[str, Any] = {}
        self.parent_static_evidence: dict[str, Any] | None = None
        if parent_static_proof is not None:
            payload = parent_static_proof.read_bytes()
            document = json.loads(payload)
            require(document.get("format") == "lostark-source-parent-static-map-proof" and
                    document.get("formatVersion") == 1, "unsupported parent static map evidence")
            for key in ("sourceShaderCache", "classDefaultEvidence"):
                item = document[key]
                require(digest(Path(item["path"]).read_bytes()) == item["sha256"],
                        f"parent static map evidence changed: {key}")
            self.parent_static_proofs = document["proofs"]
            self.parent_static_evidence = {"path": str(parent_static_proof.resolve()), "sha256": digest(payload)}

    def inherited_static(self, full: str, parent: str, package: closure.PackageData,
                         serial: bytes, tail: bytes, props: dict, resolved: dict) -> dict:
        """Accept only an explicitly proved, unchanged parent-map MIC.

        Absence alone remains an error. The opt-in proof pins the original MIC,
        CDO evidence and the real cache's unique parent map. Scalar/vector/texture
        overrides are retained below; an instance static permutation is never
        replaced by this path.
        """
        proof = self.parent_static_proofs.get(full)
        require(proof is not None, "MIC native static resource absent; effective switches are unresolved")
        allowed = {"parent", "parentlightingguid", "scalarparametervalues",
                   "vectorparametervalues", "textureparametervalues"}
        keys = {key.casefold() for key in props}
        require(not tail and keys <= allowed and keys == set(proof["allowedPropertyNames"]),
                f"{full}: parent-map proof has an unreviewed instance property or native resource")
        require(proof["sourceMaterial"] == full and proof["parentMaterial"] == parent and
                proof["sourceSerialSha256"] == digest(serial) and
                proof["sourcePackageSha256"] == package.sha256 and
                proof["baseMaterialId"] == resolved["baseId"] and proof["sourceNativeTailBytes"] == 0,
                f"{full}: parent-map source identity changed")
        require(not resolved["switches"] and not any(row["field"] == "switches"
                for row in resolved["unresolvedDefaults"]), f"{full}: unproved static parent state")
        parsed = shader_maps.parse_static_parameter_set(
            bytes.fromhex(resolved["baseId"]) + bytes(16), 0, package.names)
        equality = shader_maps.canonical_json_sha256(shader_maps.engine_equivalent_static_parameter_set(parsed))
        require(equality == proof["engineEqualityStaticParameterSetSha256"] and
                proof["nativeShaderMapContext"]["engineEqualityStaticParameterSetSha256"] == equality and
                proof["sourceLocalVertexFactoryShaders"], f"{full}: parent shader-map key changed")
        return {"status": "SOURCE_PROVED_PARENT_MAP_INHERITANCE", "nativeTailByteCount": 0,
                "staticParameterSet": shader_maps.public_static_set(parsed),
                "engineEqualityStaticParameterSetSha256": equality,
                "evidence": self.parent_static_evidence, "parentMaterial": parent}

    def package(self, logical_name: str) -> closure.PackageData:
        if logical_name not in self.packages:
            path = ue.resolve_physical_package(self.umodel, self.package_root, logical_name, "kr").resolve(strict=True)
            require(path.is_relative_to(self.package_root), "resolved package escaped package root")
            self.packages[logical_name] = closure.load_package(path, ue.LOSTARK_KR_AES_KEY)
        return self.packages[logical_name]

    def resolve(self, source_material: str) -> dict[str, Any]:
        full = full_source_name(source_material)
        require(full not in self.active, f"Material parent cycle: {' -> '.join(self.active + [full])}")
        require(len(self.active) < 64, "Material parent chain exceeds 64")
        if full in self.materials:
            return self.materials[full]
        self.active.append(full)
        try:
            logical_name, relative = full.split(".", 1)
            package = self.package(logical_name)
            matches = [entry for entry in package.exports
                       if ue.package_ref_path(entry.index + 1, package.imports, package.exports).casefold() == relative]
            require(len(matches) == 1, f"missing/ambiguous Material export {full}")
            entry = matches[0]
            class_name = ue.package_ref_name(entry.class_index, package.imports, package.exports).casefold()
            serial = package.logical[entry.serial_offset:entry.serial_offset + entry.serial_size]
            props, end = ue.parse_tagged_properties(serial, package.names, package.summary.version)
            require(package.summary.version == 868, f"unsupported source material version {package.summary.version}")
            evidence = {"sourceMaterial": full, "sourcePackage": str(package.path), "sourceExportIndex0": entry.index,
                        "serialSHA256": digest(serial), "propertyStreamEnd": end}
            if class_name == "material":
                result = self.base_material(full, package, relative, serial[end:], evidence)
            elif class_name == "materialinstanceconstant":
                parent_ref = closure.tagged_value(props, "parent")
                require(isinstance(parent_ref, int) and parent_ref != 0, f"MIC has no parent: {full}")
                parent = source_reference(package, logical_name, parent_ref)
                result = copy.deepcopy(self.resolve(parent))
                result["sourceMaterial"], result["parent"] = full, parent
                result["chain"].append(evidence)
                native = (self.static_decoder.decode(serial[end:], bytes.fromhex(result["baseId"]), package.names)
                          if serial[end:] else self.inherited_static(full, parent, package, serial,
                                                                    serial[end:], props, result))
                result["static"] = native
                for row in native["staticParameterSet"]["staticSwitchParameters"]:
                    assign(result, "switches", row["parameterName"], row["value"],
                           {**evidence, "kind": "MIC_NATIVE_EFFECTIVE_STATIC_SET", "bOverride": row["bOverride"],
                            "sourceFNameNumber": row["sourceFNameNumber"]})
                instance = closure.decode_material_instance(package, relative)
                self.validate_instance_rows(props, instance, full)
                for field in ("scalarParameters", "vectorParameters"):
                    for row in instance[field]:
                        value = row["value"]
                        if field == "scalarParameters":
                            value = checked_float(value, f"{full}:{row['name']}")
                        else:
                            value = [checked_float(item, f"{full}:{row['name']}") for item in value]
                        assign(result, "values", row["name"], value, {**evidence, "kind": "MIC_SERIALIZED_OVERRIDE"})
                for row in instance["textureParameters"]:
                    assign(result, "textures", row["name"], source_reference(package, logical_name, row["packageIndex"]),
                           {**evidence, "kind": "MIC_SERIALIZED_OVERRIDE", "packageIndex": row["packageIndex"]})
            else:
                raise ValueError(f"unsupported source Material class {class_name}: {full}")
            self.materials[full] = result
            return result
        finally:
            self.active.pop()

    @staticmethod
    def validate_instance_rows(props: dict, decoded: dict, full: str) -> None:
        for property_name, field in (("scalarparametervalues", "scalarParameters"),
                                     ("vectorparametervalues", "vectorParameters"),
                                     ("textureparametervalues", "textureParameters")):
            rows = closure.tagged_value(props, property_name)
            require(rows is None or isinstance(rows, list), f"invalid MIC array {full}:{property_name}")
            require(len(rows or []) == len(decoded[field]), f"undecoded MIC parameter row {full}:{property_name}")
            names = [row["name"].casefold() for row in decoded[field]]
            require(len(names) == len(set(names)), f"duplicate MIC parameter {full}:{property_name}")

    def base_material(self, full: str, package: closure.PackageData, relative: str,
                      tail: bytes, evidence: dict) -> dict[str, Any]:
        # This offset is the verified v868 cooked Material native-resource ABI;
        # other package versions are rejected before reaching this function.
        require(len(tail) >= 32 and tail[16:32] != bytes(16), f"invalid v868 base Material ID: {full}")
        contract = graph.extract_material_contract(package.path, relative)
        result: dict[str, Any] = {"sourceMaterial": full, "parent": None, "terminal": full,
                                  "baseId": tail[16:32].hex(), "values": {}, "textures": {}, "switches": {},
                                  "static": None, "chain": [evidence], "unresolvedDefaults": [],
                                  "parameterSources": {"values": {}, "textures": {}, "switches": {}},
                                  "graphSummary": contract["summary"]}
        logical_name = full.split(".", 1)[0]
        for expr in contract["expressions"]:
            name = expr["parameterName"]
            if not name:
                continue
            name, class_name = name.casefold(), expr["className"].casefold()
            expression_entry = package.exports[expr["exportIndex"]]
            raw = package.logical[expression_entry.serial_offset:expression_entry.serial_offset + expression_entry.serial_size]
            props, _ = ue.parse_tagged_properties(raw, package.names, package.summary.version)
            detail = {**evidence, "kind": "MATERIAL_EXPRESSION_DEFAULT", "expressionExportIndex0": expr["exportIndex"],
                      "expressionObjectPath": expr["objectPath"], "expressionSerialSHA256": digest(raw)}
            if class_name == "materialexpressionstaticswitchparameter":
                field, value = "switches", expr["defaultValue"]
                require(value is None or isinstance(value, bool), f"invalid switch default {full}:{name}")
            elif class_name == "materialexpressionscalarparameter":
                field, value = "values", expr["defaultValue"]
                if value is not None:
                    value = checked_float(value, f"{full}:{name}")
            elif class_name == "materialexpressionvectorparameter":
                field, value = "values", expr["defaultValue"]
                if value is not None:
                    require(isinstance(value, dict) and len(value.get("hex", "")) == 32, f"invalid vector default {full}:{name}")
                    value = [checked_float(item, f"{full}:{name}") for item in struct.unpack("<4f", bytes.fromhex(value["hex"]))]
            elif class_name.startswith("materialexpressiontexturesampleparameter"):
                reference = closure.tagged_value(props, "texture")
                if reference is None:
                    field, value = "textures", None
                else:
                    require(isinstance(reference, int), f"invalid texture default {full}:{name}")
                    texture = source_reference(package, logical_name, reference)
                    require(name not in result["textures"] or result["textures"][name] == texture,
                            f"conflicting base texture defaults {full}:{name}")
                    assign(result, "textures", name, texture,
                           {**detail, "packageIndex": reference})
                    continue
            else:
                continue
            if value is None:
                if name not in result[field] and not any(row["field"] == field and row["name"] == name for row in result["unresolvedDefaults"]):
                    result["unresolvedDefaults"].append({"field": field, "name": name, "reason": "DEFAULT_PROPERTY_ABSENT", **detail})
                continue
            require(name not in result[field] or result[field][name] == value,
                    f"conflicting base defaults {full}:{name}")
            assign(result, field, name, value, detail)
        return result


def extract_parameters(source_materials: list[str], package_root: Path, umodel: Path,
                       parent_static_proof: Path | None = None) -> dict[str, Any]:
    require(bool(source_materials), "no source Material paths supplied")
    resolver = MaterialResolver(package_root, umodel, parent_static_proof)
    rows, failures = {}, []
    for full in sorted({full_source_name(value) for value in source_materials}):
        try:
            rows[full] = resolver.resolve(full)
        except (ValueError, ue.ExtractionError, OSError, struct.error) as error:
            failures.append({"sourceMaterial": full, "error": str(error)})
    sources = []
    for logical_name, package in resolver.packages.items():
        require(digest(package.path.read_bytes()) == package.sha256, f"source package changed during extraction: {package.path}")
        sources.append({"logicalName": logical_name, "path": str(package.path), "sha256": package.sha256})
    if resolver.parent_static_evidence:
        sources.append(resolver.parent_static_evidence)
    return {"format": FORMAT, "formatVersion": 1, "materials": rows, "sources": sources, "failures": failures,
            "sourceStaticFormat": STATIC_FORMAT,
            "scope": "source parameter resolution only; no shader-map/runtime/visual admission; absent defaults remain unresolved"}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-material", action="append", default=[])
    parser.add_argument("--source-materials-json", type=Path, help="JSON array of full package.object paths")
    parser.add_argument("--parent-static-proof", type=Path,
                        help="Opt-in source/CDO/cache proof for exact empty-tail parent-map instances")
    for name in ("package-root", "umodel", "output", "receipt"):
        parser.add_argument(f"--{name}", required=True, type=Path)
    args = parser.parse_args(argv)
    try:
        materials = list(args.source_material)
        if args.source_materials_json:
            listed = json.loads(args.source_materials_json.read_text(encoding="utf-8-sig"))
            require(isinstance(listed, list), "source-materials-json must be an array of full paths")
            materials.extend(listed)
        output, receipt = args.output.resolve(), args.receipt.resolve()
        require(output != receipt, "output and receipt must be distinct")
        for target in (output, receipt):
            require(not target.is_relative_to(args.package_root.resolve()) and target != args.umodel.resolve(),
                    "output must not overwrite source packages or decoder")
            require(args.source_materials_json is None or target != args.source_materials_json.resolve(),
                    "output must not overwrite source list")
        document = extract_parameters(materials, args.package_root, args.umodel, args.parent_static_proof)
        receipt_document = {"format": FORMAT + "-receipt", "formatVersion": 1,
                            "status": "PASS" if not document["failures"] else "FAILED_OUTPUT_PRESERVED",
                            "requested": sorted(set(materials)), "sources": document["sources"],
                            "failures": document["failures"], "materialCount": len(document["materials"]),
                            "decoder": {"path": str(args.umodel.resolve()), "sha256": digest(args.umodel.read_bytes())}}
        if not document["failures"]:
            payload = (json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + "\n").encode("utf-8")
            receipt_document["outputSHA256"] = digest(payload)
            write_pair(output, payload, receipt,
                       (json.dumps(receipt_document, ensure_ascii=False, indent=2) + "\n").encode("utf-8"))
        else:
            write_atomic(receipt, (json.dumps(receipt_document, ensure_ascii=False, indent=2) + "\n").encode("utf-8"))
        print(f"materials={len(document['materials'])} failures={len(document['failures'])}")
        return 1 if document["failures"] else 0
    except (ValueError, ue.ExtractionError, OSError) as error:
        print(f"source Material extraction failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
