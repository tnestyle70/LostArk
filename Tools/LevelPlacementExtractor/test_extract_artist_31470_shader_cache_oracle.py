#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import subprocess
import struct
import sys
import tempfile
import unittest
from pathlib import Path

from extract_artist_31470_shader_cache_oracle import (
    BLOCKERS,
    DEFAULT_MATERIAL_CONTRACT,
    DEFAULT_OUTPUT,
    MATERIAL_TARGETS,
    build_topology_completeness_matrix,
    canonical_text_sha256,
    key_variants,
    intersect_mic_tail_shader_ids,
    parse_shader_cache_serial,
    parse_shader_cache_material_maps,
    raw_sha256,
    read_json,
    scan_material_inventory_reports,
    seal_receipt,
    validate_dxbc_container,
    validate_receipt,
    verify_external_file,
)


def literal_lz4(payload: bytes) -> bytes:
    length = len(payload)
    if length < 15:
        return bytes([length << 4]) + payload
    output = bytearray([0xF0])
    remainder = length - 15
    while remainder >= 255:
        output.append(255)
        remainder -= 255
    output.append(remainder)
    output.extend(payload)
    return bytes(output)


def fake_dxbc() -> bytes:
    payload = b"\x00\x00\x00\x00"
    total_size = 36 + 8 + len(payload)
    return (
        b"DXBC"
        + b"\x00" * 16
        + struct.pack("<III", 1, total_size, 1)
        + struct.pack("<I", 36)
        + b"SHDR"
        + struct.pack("<I", len(payload))
        + payload
    )


def synthetic_cache_serial(
    *,
    dxbc: bytes | None = None,
    descriptor_count: int = 1,
    code_count: int = 1,
    compressed_size_delta: int = 0,
    corrupt_lz4: bool = False,
) -> tuple[bytes, list[str]]:
    dxbc = dxbc if dxbc is not None else fake_dxbc()
    compressed = bytearray(literal_lz4(dxbc))
    if corrupt_lz4:
        compressed[0] = 0x0F
    serial = bytearray()
    serial.extend(struct.pack("<i", -1))
    serial.extend(struct.pack("<ii", 0, 0))
    serial.extend(struct.pack("<I", 0))
    serial.append(4)
    serial.extend(struct.pack("<I", 1))
    serial.extend(struct.pack("<ii", 1, 0))
    serial.extend(struct.pack("<I", descriptor_count))
    for index in range(descriptor_count):
        serial.extend(bytes([index + 1]) * 16)
        serial.extend(struct.pack("<Q", index + 7))
    serial.extend(struct.pack("<I", code_count))
    for _ in range(code_count):
        serial.extend(
            struct.pack(
                "<II", len(dxbc), len(compressed) + compressed_size_delta
            )
        )
        serial.extend(compressed)
    serial.extend(struct.pack("<II", 4, code_count))
    return bytes(serial), ["None", "test_pixel_shader"]


def synthetic_cache_with_material_map() -> tuple[bytes, list[str], int]:
    serial, names = synthetic_cache_serial()
    code_end = len(serial) - 8
    shader_id = b"\x01" * 16
    base_material_id = bytes.fromhex("00112233445566778899aabbccddeeff")
    tail = bytearray(struct.pack("<II", 4, 1))
    tail.extend(struct.pack("<ii", 1, 0) + shader_id + b"\xa5" * 12)
    tail.extend(struct.pack("<I", 1))
    tail.extend(base_material_id)
    tail.extend(struct.pack("<IIII", 0, 0, 0, 0))
    tail.extend(struct.pack("<IIIII", 868, 16, 123, 0, 1))
    tail.extend(struct.pack("<I", 1))
    tail.extend(struct.pack("<ii", 1, 0) + shader_id + struct.pack("<ii", 1, 0))
    return serial[:code_end] + bytes(tail), names, code_end


def fake_disassemble(_bytecode: bytes) -> dict:
    return {
        "profile": "ps_5_0",
        "constantBufferDeclarations": ["dcl_constantbuffer CB0[1], immediateIndexed"],
        "samplerDeclarations": ["dcl_sampler s0, mode_default"],
        "resourceDeclarations": ["dcl_resource_texture2d (float,float,float,float) t0"],
        "uavDeclarations": [],
        "declarationSha256": "1" * 64,
    }


class ShaderCacheOracleTests(unittest.TestCase):
    def test_native_group_and_lz4_dxbc_contract(self) -> None:
        serial, names = synthetic_cache_serial()
        parsed = parse_shader_cache_serial(serial, names, fake_disassemble)
        self.assertEqual(parsed["shaderTypeGroupCount"], 1)
        self.assertEqual(len(parsed["codeRecords"]), 1)
        self.assertEqual(parsed["codeRecords"][0]["disassembly"]["profile"], "ps_5_0")
        self.assertEqual(parsed["shaderObjectTableHeader"]["shaderObjectCount"], 1)

    def test_descriptor_and_code_count_mismatch_rejected(self) -> None:
        serial, names = synthetic_cache_serial(descriptor_count=1, code_count=0)
        with self.assertRaisesRegex(ValueError, "descriptor/code count"):
            parse_shader_cache_serial(serial, names, fake_disassemble)

    def test_material_shader_map_structural_decoder_has_independent_offsets(self) -> None:
        serial, names, code_end = synthetic_cache_with_material_map()
        parsed = parse_shader_cache_serial(serial, names, fake_disassemble)
        structure = parse_shader_cache_material_maps(serial, names, parsed)
        self.assertEqual(structure["tailOffsetInSerial"], code_end)
        self.assertEqual(structure["shaderObjectTableOffset"], 8)
        self.assertEqual(structure["materialShaderMapTableCountOffset"], 44)
        self.assertEqual(structure["materialShaderMaps"][0]["offset"], 48)
        self.assertEqual(
            structure["materialShaderMaps"][0]["staticParameterSet"]["baseMaterialIdHex"],
            "00112233445566778899aabbccddeeff",
        )
        self.assertEqual(structure["shaderReferenceCount"], 1)

        mutated = bytearray(serial)
        struct.pack_into("<I", mutated, code_end + 44, 2)
        parsed = parse_shader_cache_serial(bytes(mutated), names, fake_disassemble)
        with self.assertRaisesRegex(ValueError, "header candidate|table count"):
            parse_shader_cache_material_maps(bytes(mutated), names, parsed)

        mutated = bytearray(serial)
        mutated[code_end + 112 : code_end + 128] = b"\x02" * 16
        parsed = parse_shader_cache_serial(bytes(mutated), names, fake_disassemble)
        with self.assertRaisesRegex(ValueError, "references are absent|cover"):
            parse_shader_cache_material_maps(bytes(mutated), names, parsed)

    def test_truncated_and_forged_compressed_size_rejected(self) -> None:
        serial, names = synthetic_cache_serial(compressed_size_delta=100)
        with self.assertRaisesRegex(ValueError, "compressed code size"):
            parse_shader_cache_serial(serial, names, fake_disassemble)

        valid, names = synthetic_cache_serial()
        with self.assertRaisesRegex(ValueError, "truncated|size|LZ4|native"):
            parse_shader_cache_serial(valid[:-6], names, fake_disassemble)

    def test_corrupt_lz4_rejected(self) -> None:
        serial, names = synthetic_cache_serial(corrupt_lz4=True)
        with self.assertRaises(Exception):
            parse_shader_cache_serial(serial, names, fake_disassemble)

    def test_dxbc_total_size_and_chunk_bounds_rejected(self) -> None:
        bytecode = bytearray(fake_dxbc())
        struct.pack_into("<I", bytecode, 24, len(bytecode) + 1)
        with self.assertRaisesRegex(ValueError, "total size"):
            validate_dxbc_container(bytes(bytecode))

        bytecode = bytearray(fake_dxbc())
        struct.pack_into("<I", bytecode, 32, len(bytecode))
        with self.assertRaisesRegex(ValueError, "chunk header"):
            validate_dxbc_container(bytes(bytecode))

    def test_material_key_endian_variants_are_explicit(self) -> None:
        source = bytes.fromhex("00112233445566778899aabbccddeeff")
        variants = key_variants(source)
        self.assertEqual(set(variants), {
            "DIRECT",
            "REVERSE_ALL",
            "REVERSE_U32_WORDS",
            "REVERSE_GUID_TEXT_FIELDS",
            "REVERSE_U32_WORD_ORDER",
        })
        self.assertEqual(variants["REVERSE_ALL"].hex(), "ffeeddccbbaa99887766554433221100")
        self.assertEqual(variants["REVERSE_U32_WORDS"].hex(), "3322110077665544bbaa9988ffeeddcc")

    def test_external_raw_bytes_are_not_resealable_observations(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "fixture.bin"
            path.write_bytes(b"pinned")
            expected = {
                "fileName": "fixture.bin",
                "byteSize": 6,
                "sha256": raw_sha256(b"pinned"),
            }
            self.assertEqual(verify_external_file(path, expected), b"pinned")
            path.write_bytes(b"forged")
            with self.assertRaisesRegex(ValueError, "size|SHA"):
                verify_external_file(path, expected)

    def test_tracked_text_hash_is_lf_crlf_stable(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            lf = Path(directory) / "lf.py"
            crlf = Path(directory) / "crlf.py"
            lf.write_bytes(b"a\nb\n")
            crlf.write_bytes(b"a\r\nb\r\n")
            self.assertEqual(canonical_text_sha256(lf), canonical_text_sha256(crlf))

    def test_bounded_inventory_is_explicit_and_fail_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            families = []
            for index, (family_id, source_path, _global, class_name) in enumerate(MATERIAL_TARGETS):
                families.append(
                    {
                        "familyId": family_id,
                        "materialObjectPath": source_path,
                        "materialClass": class_name,
                        "physicalPackage": f"PACKAGE{index:02d}.upk",
                        "physicalPackageSha256": f"{index + 1:064x}",
                        "materialExportIndex": index,
                        "sourceTopology": {},
                    }
                )
            for report_index in range(30):
                materials = []
                source_file = f"EMPTY{report_index:02d}.upk"
                source_sha = f"{1000 + report_index:064x}"
                if report_index < 21:
                    family = families[report_index]
                    source_file = family["physicalPackage"]
                    source_sha = family["physicalPackageSha256"]
                    materials.append(
                        {
                            "material_path": family["materialObjectPath"],
                            "object_name": family["materialObjectPath"].rsplit(".", 1)[-1],
                            "class": family["materialClass"],
                            "export_index": family["materialExportIndex"],
                        }
                    )
                    if report_index == 0:
                        materials.append(
                            {
                                "material_path": "alternate." + family["materialObjectPath"].rsplit(".", 1)[-1],
                                "object_name": family["materialObjectPath"].rsplit(".", 1)[-1],
                                "class": family["materialClass"],
                                "export_index": 999,
                            }
                        )
                document = {
                    "schema_version": 2,
                    "source": {"file": source_file, "sha256": source_sha},
                    "materials": materials,
                }
                (root / f"report-{report_index:02d}.materials.json").write_text(
                    json.dumps(document), encoding="utf-8"
                )
            evidence = scan_material_inventory_reports(root, families)
            self.assertEqual(evidence["reportCount"], 30)
            self.assertEqual(evidence["targetFamilyCoverageCount"], 21)
            self.assertEqual(evidence["targetMaterialRowCount"], 22)
            self.assertEqual(evidence["alternateObjectCandidateCount"], 1)

            path = root / "report-29.materials.json"
            mutated = json.loads(path.read_text(encoding="utf-8"))
            mutated["schema_version"] = True
            path.write_text(json.dumps(mutated), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "version"):
                scan_material_inventory_reports(root, families)

    def test_topology_improvement_requires_same_entry_denominator(self) -> None:
        families = []
        globals_ = []
        sources = []
        for index, (family_id, source_path, global_path, class_name) in enumerate(MATERIAL_TARGETS):
            source_topology = {
                "topologyStatus": "COOKED_PARTIAL",
                "expressionEntryCount": 10,
                "nonNullExpressionCount": 5,
                "nullExpressionCount": 5,
                "unresolvedInputEdgeCount": 2,
                "outputBindingCount": 1,
                "materialSerialSha256": "1" * 64,
                "materialPropertyStreamEnd": 4,
            }
            families.append({"familyId": family_id})
            sources.append(
                {
                    "familyId": family_id,
                    "candidateRole": "SOURCE_EXACT_DEPENDENCY",
                    "physicalPackage": f"S{index}.upk",
                    "physicalPackageSha256": "2" * 64,
                    "materialObjectPath": source_path,
                    "materialClass": class_name,
                    "materialExportIndex": index,
                    "topology": source_topology,
                    "fidelity": "SOURCE_EXACT_COOKED_PARTIAL_GRAPH_SHAPE",
                }
            )
            global_topology = copy.deepcopy(source_topology)
            if index == 0:
                global_topology["expressionEntryCount"] = 9
                global_topology["nonNullExpressionCount"] = 5
                global_topology["nullExpressionCount"] = 4
                global_topology["unresolvedInputEdgeCount"] = 1
            if index == 1:
                global_topology["nonNullExpressionCount"] = 6
                global_topology["nullExpressionCount"] = 4
                global_topology["unresolvedInputEdgeCount"] = 1
            globals_.append(
                {
                    "familyId": family_id,
                    "globalMaterialObjectPath": global_path,
                    "className": class_name,
                    "exportIndex": index,
                    "topology": global_topology,
                }
            )
        matrices = build_topology_completeness_matrix(families, globals_, sources)
        self.assertEqual(matrices[0]["strictParetoImprovementCandidateCount"], 0)
        self.assertEqual(matrices[1]["strictParetoImprovementCandidateCount"], 1)
        self.assertIsNotNone(matrices[1]["reconstructedEvaluatorOracleCandidate"])

    def test_mic_tail_shader_id_intersection_is_bounded(self) -> None:
        candidate_a = bytes.fromhex("00112233445566778899aabbccddeeff")
        candidate_b = bytes.fromhex("102132435465768798a9bacbdcedfe0f")
        projection = [
            {"offset": 0, "candidateHex": candidate_a.hex()},
            {"offset": 4, "candidateHex": candidate_b.hex()},
        ]
        recipe = {
            "recipeId": "recipe-a",
            "nativeTailByteCount": 20,
            "micNativeStateKeyCandidateHex": candidate_a.hex(),
            "alignedNonzero16ByteWindowCount": 2,
            "alignedNonzero16ByteWindowSha256": raw_sha256(
                json.dumps(
                    projection, sort_keys=True, separators=(",", ":"),
                    ensure_ascii=False,
                ).encode("utf-8")
            ),
        }
        records = [{"shaderIdCandidateHex": candidate_b.hex()}]
        probe = intersect_mic_tail_shader_ids(
            [recipe], {"recipe-a": [(0, candidate_a), (4, candidate_b)]}, records
        )
        self.assertEqual(probe["directShaderIdMatchCount"], 1)
        self.assertEqual(probe["matches"][0]["offsetInNativeTail"], 4)
        self.assertEqual(
            probe["status"], "MIC_TAIL_SHADER_OBJECT_ID_WINDOW_MATCH_UNBOUND"
        )

        no_match = intersect_mic_tail_shader_ids(
            [recipe],
            {"recipe-a": [(0, candidate_a), (4, candidate_b)]},
            [{"shaderIdCandidateHex": "ff" * 16}],
        )
        self.assertEqual(no_match["directShaderIdMatchCount"], 0)
        self.assertEqual(
            no_match["status"], "MIC_TAIL_CONTAINS_NO_DIRECT_SHADER_OBJECT_ID"
        )

    def test_receipt_denominator_and_blockers_fail_closed(self) -> None:
        if not DEFAULT_OUTPUT.is_file():
            self.skipTest("tracked ShaderCache receipt has not been generated")
        receipt = json.loads(DEFAULT_OUTPUT.read_text(encoding="utf-8"))
        validate_receipt(receipt, DEFAULT_MATERIAL_CONTRACT)

        mutated = copy.deepcopy(receipt)
        mutated["joinDecision"]["blockers"].remove(BLOCKERS[0])
        seal_receipt(mutated)
        with self.assertRaises(ValueError):
            validate_receipt(mutated, DEFAULT_MATERIAL_CONTRACT)

    def test_portable_receipt_resealed_attacks_are_rejected(self) -> None:
        if not DEFAULT_OUTPUT.is_file():
            self.skipTest("tracked ShaderCache receipt has not been generated")
        receipt = read_json(DEFAULT_OUTPUT)
        attacks = []
        mutated = copy.deepcopy(receipt)
        mutated["externalEvidence"]["shaderCachePackage"]["rawSha256"] = "0" * 64
        attacks.append(("external identity", mutated))

        mutated = copy.deepcopy(receipt)
        mutated["primaryShaderCache"]["codeRecords"][0]["compressedOffset"] += 1
        attacks.append(("code range", mutated))

        mutated = copy.deepcopy(receipt)
        mutated["materialNativeKeys"][0]["nativeStateKeyCandidateHex"] = "11" * 16
        attacks.append(("native key", mutated))

        mutated = copy.deepcopy(receipt)
        mutated["materialKeySearch"][0]["matches"] = [
            {"variant": "DIRECT", "logicalOffset": 1, "exportIndex": 1}
        ]
        mutated["materialKeySearch"][0]["matchCount"] = 1
        attacks.append(("direct Material match", mutated))

        mutated = copy.deepcopy(receipt)
        mic_key = next(
            row["micNativeStateKeyCandidateHex"]
            for row in receipt["recipeNativeKeys"]
            if row["micNativeStateKeyCandidateHex"]
        )
        mutated["primaryShaderCache"]["codeRecords"][0]["shaderIdCandidateHex"] = mic_key
        mutated["primaryShaderCache"]["codeRecords"][0]["descriptorSha256"] = raw_sha256(
            bytes.fromhex(mic_key)
            + bytes.fromhex(mutated["primaryShaderCache"]["codeRecords"][0]["opaqueDescriptorTailHex"])
        )
        attacks.append(("descriptor ID laundering", mutated))

        mutated = copy.deepcopy(receipt)
        forged_digest = "22" * 32
        mutated["recipeNativeKeys"][0]["alignedNonzero16ByteWindowSha256"] = forged_digest
        probe_row = next(
            row
            for row in mutated["micTailShaderObjectIdProbe"]["rows"]
            if row["recipeId"] == mutated["recipeNativeKeys"][0]["recipeId"]
        )
        probe_row["candidateDigestSha256"] = forged_digest
        attacks.append(("MIC window digest", mutated))

        mutated = copy.deepcopy(receipt)
        source = next(
            row
            for row in mutated["materialTopologyCompleteness"][0]["candidates"]
            if row["candidateRole"] == "SOURCE_EXACT_DEPENDENCY"
        )
        source["materialExportIndex"] += 1
        attacks.append(("topology export identity", mutated))

        mutated = copy.deepcopy(receipt)
        mutated["structuralShaderMapJoin"]["familyRows"][0][
            "primaryMaterialShaderMapIndices"
        ] = [0]
        mutated["structuralShaderMapJoin"]["familyRows"][0]["joinStatus"] = (
            "EXACT_BASE_MATERIAL_ID_JOIN"
        )
        attacks.append(("forged structural join", mutated))

        for label, mutated in attacks:
            seal_receipt(mutated)
            with self.subTest(label=label):
                with self.assertRaises(ValueError):
                    validate_receipt(mutated, DEFAULT_MATERIAL_CONTRACT)

    def test_validate_only_rejects_duplicate_keys_and_resealed_raw_identity(self) -> None:
        if not DEFAULT_OUTPUT.is_file():
            self.skipTest("tracked ShaderCache receipt has not been generated")
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            duplicate = root / "duplicate.json"
            duplicate.write_text(
                '{"schema":"x","schema":"y"}', encoding="utf-8"
            )
            with self.assertRaisesRegex(ValueError, "duplicate JSON key"):
                read_json(duplicate)

            receipt = read_json(DEFAULT_OUTPUT)
            receipt["externalEvidence"]["d3dcompiler"]["rawSha256"] = "0" * 64
            seal_receipt(receipt)
            forged = root / "forged.json"
            forged.write_text(json.dumps(receipt), encoding="utf-8")
            script = Path(__file__).with_name(
                "extract_artist_31470_shader_cache_oracle.py"
            )
            process = subprocess.run(
                [
                    sys.executable,
                    "-B",
                    str(script),
                    "--validate-only",
                    "--output",
                    str(forged),
                    "--material-contract",
                    str(DEFAULT_MATERIAL_CONTRACT),
                ],
                capture_output=True,
                text=True,
                check=False,
            )
            self.assertNotEqual(process.returncode, 0)

        receipt = read_json(DEFAULT_OUTPUT)
        mutated = copy.deepcopy(receipt)
        mutated["admission"]["productAdmission"] = True
        seal_receipt(mutated)
        with self.assertRaises(ValueError):
            validate_receipt(mutated, DEFAULT_MATERIAL_CONTRACT)

        mutated = copy.deepcopy(receipt)
        mutated["materialTopologyCompleteness"][0]["candidates"][0][
            "strictParetoImprovementOverSource"
        ] = True
        seal_receipt(mutated)
        with self.assertRaises(ValueError):
            validate_receipt(mutated, DEFAULT_MATERIAL_CONTRACT)

    def test_json_boolean_and_float_versions_rejected(self) -> None:
        if not DEFAULT_OUTPUT.is_file():
            self.skipTest("tracked ShaderCache receipt has not been generated")
        receipt = json.loads(DEFAULT_OUTPUT.read_text(encoding="utf-8"))
        for version in (True, 2.0, "2"):
            mutated = copy.deepcopy(receipt)
            mutated["formatVersion"] = version
            seal_receipt(mutated)
            with self.subTest(version=version):
                with self.assertRaisesRegex(ValueError, "version"):
                    validate_receipt(mutated, DEFAULT_MATERIAL_CONTRACT)


if __name__ == "__main__":
    unittest.main()
