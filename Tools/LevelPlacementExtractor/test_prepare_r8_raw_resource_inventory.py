#!/usr/bin/env python3
"""Focused tests for the portable R8 offline raw-resource checkpoint."""

from __future__ import annotations

import copy
import hashlib
import importlib.util
import json
import math
import struct
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("prepare_r8_raw_resource_inventory.py")
SPEC = importlib.util.spec_from_file_location("prepare_r8_raw_resource_inventory", MODULE_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"cannot import {MODULE_PATH}")
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)
REPO_ROOT = Path(__file__).resolve().parents[2]
CHECKPOINT = (
    REPO_ROOT
    / "Data/Effects/Imported/RawResourceInventory/"
    "R8.raw-resource-denominator.checkpoint.json"
)
FINAL_INVENTORY = (
    REPO_ROOT
    / "Data/Effects/Imported/RawResourceInventory/"
    "R8.raw-resource-inventory-v1.json"
)
EXPECTED_FINAL_RAW_SHA256 = (
    "578fadda10903e4935bb633947843aeefe709c55dfa4767fa2fb62ad4817e500"
)


def build_dds(
    width: int = 4,
    height: int = 4,
    four_cc: bytes = b"DXT1",
    mip_count: int = 0,
    payload: bytes = b"\0" * 8,
) -> bytes:
    header = bytearray(128)
    header[:4] = b"DDS "
    struct.pack_into("<I", header, 4, 124)
    struct.pack_into("<I", header, 8, 0x00081007)
    struct.pack_into("<I", header, 12, height)
    struct.pack_into("<I", header, 16, width)
    struct.pack_into("<I", header, 20, 8)
    struct.pack_into("<I", header, 28, mip_count)
    struct.pack_into("<I", header, 76, 32)
    struct.pack_into("<I", header, 80, 0x4)
    header[84:88] = four_cc
    struct.pack_into("<I", header, 108, 0x1000)
    return bytes(header) + payload


def build_legacy_wmodel() -> bytes:
    vertices = b"".join(
        struct.pack(
            "<3f3f2f3ff",
            *position,
            0.0,
            1.0,
            0.0,
            0.0,
            0.0,
            1.0,
            0.0,
            0.0,
            1.0,
        )
        for position in ((0.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, 1.0, 0.0))
    )
    indices = struct.pack("<3H", 0, 1, 2)
    center = (0.5, 0.5, 0.0)
    radius = math.sqrt(0.5)
    bounds = struct.pack(
        "<10f",
        0.0,
        0.0,
        0.0,
        1.0,
        1.0,
        0.0,
        *center,
        radius,
    )
    mesh_header = MODULE.WMODEL_MESH_HEADER.pack(
        b"WMSH", 1, 0, 15, 48, 3, 3, 2, 1, b"\0\0\0"
    )
    descriptor = MODULE.WMODEL_SUBMESH_DESC.pack(
        0, 3, 0, 3, 0, 0, b"triangle\0".ljust(20, b"\0")
    )
    mesh_content = mesh_header + descriptor + vertices + indices + bounds
    mesh = MODULE.WMODEL_FILE_HEADER.pack(
        b"WINT", 1, 0, 0, len(mesh_content)
    ) + mesh_content
    material_content = struct.pack("<4sI", b"WMA2", 1) + b"\0" * 4756
    material = MODULE.WMODEL_FILE_HEADER.pack(
        b"WINT", 1, 0, 0, len(material_content)
    ) + material_content
    section_offset = (
        MODULE.WMODEL_MODEL_HEADER.size + 2 * MODULE.WMODEL_SECTION_DESC.size
    )
    model_header = MODULE.WMODEL_MODEL_HEADER.pack(b"WMOD", 2, 0, 0, 0, 0, 0, 0)
    table = b"".join(
        (
            MODULE.WMODEL_SECTION_DESC.pack(
                1, 0, section_offset, len(mesh), b"mesh\0".ljust(40, b"\0")
            ),
            MODULE.WMODEL_SECTION_DESC.pack(
                2,
                0,
                section_offset + len(mesh),
                len(material),
                b"material\0".ljust(40, b"\0"),
            ),
        )
    )
    content = model_header + table + mesh + material
    return MODULE.WMODEL_FILE_HEADER.pack(b"WINT", 1, 0, 0, len(content)) + content


class RawResourceCheckpointTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = MODULE.load_json(CHECKPOINT)
        cls.final_document = MODULE.load_json(FINAL_INVENTORY)

    def test_checked_checkpoint_has_exact_denominators_and_blockers(self) -> None:
        summary = MODULE.validate_checkpoint(self.document)
        self.assertEqual(summary["fourClassOccurrences"], 5232)
        self.assertEqual(summary["valtanOccurrences"], 21931)
        self.assertEqual(summary["fourClassAssetRequests"], 835)
        self.assertEqual(summary["valtanAssetRequests"], 377)
        self.assertEqual(self.document["fourClass"]["summary"]["sourceSystemCount"], 442)
        source_system_counts = {}
        for row in self.document["fourClass"]["sourceSystems"]:
            source_system_counts[row["characterClass"]] = (
                source_system_counts.get(row["characterClass"], 0) + 1
            )
        self.assertEqual(
            source_system_counts,
            {
                "ARTIST": 70,
                "DIMENSIONMASTER": 95,
                "LANCE_MASTER": 179,
                "WARLORD": 98,
            },
        )
        self.assertEqual(summary["checkpointStatus"], "FROZEN_WITH_BLOCKERS")
        self.assertEqual(self.document["blockerCount"], 18)
        self.assertEqual(len(self.document["reports"]["pinFailures"]), 2)
        self.assertEqual(
            {
                (row["characterClass"], row["skillId"], row["artifactKind"])
                for row in self.document["reports"]["pinFailures"]
            },
            {
                ("ARTIST", 31930, "importedDocument"),
                ("WARLORD", 17110, "normalizedGraph"),
            },
        )
        self.assertEqual(
            len(self.document["reports"]["preexistingRuntimeMissingRequests"]),
            191,
        )
        self.assertEqual(
            self.document["fourClass"]["summary"]["occurrenceSourceSystemCount"],
            424,
        )
        self.assertEqual(
            self.document["fourClass"]["summary"]["graphOnlyInactiveSourceSystemCount"],
            18,
        )
        self.assertEqual(
            self.document["fourClass"]["classAssetSummaries"][-1],
            {
                "characterClass": "WARLORD",
                "assetRequestCount": 328,
                "meshRequestCount": 47,
                "textureRequestCount": 281,
            },
        )
        self.assertEqual(
            self.document["reports"]["warlordCatalogGraphDisagreements"],
            [
                {
                    "code": "WARLORD_CATALOG_DEPENDENCY_ABSENT_FROM_NORMALIZED_GRAPH_BINDINGS",
                    "role": "texture",
                    "sourceAssetPath": "fx_tex_02.fx_d_environ_018",
                    "logicalPackage": "fx_tex_02",
                    "physicalPackageFileName": "YGI3SORGM3I1FGHA5BMJ8Y5CZ.upk",
                }
            ],
        )
        request = next(
            row
            for row in self.document["fourClass"]["assetRequests"]
            if row["sourceAssetPath"] == "fx_tex_02.fx_d_environ_018"
        )
        warlord = next(
            row for row in request["consumers"] if row["consumer"] == "WARLORD"
        )
        self.assertEqual(warlord["skillIds"], [17820])
        self.assertEqual(warlord["actionIds"], [17820])
        self.assertEqual(
            warlord["sourceSystemIds"], ["FX_PC_WGL_01.par_o_wgl_protect_02"]
        )
        self.assertEqual(
            warlord["ownershipStatuses"],
            ["ACTION_BOUND_CATALOG_OWNERSHIP", "RESOLVED_SOURCE_PACKAGE"],
        )

    def test_runtime_admission_is_strictly_false(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["admission"]["Render"] = True
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "admission"):
            MODULE.validate_checkpoint(mutated)

    def test_gpu_schema_policy_mutation_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["gpuSchemaBoundary"]["sampler"] = "FINAL_SAMPLER"
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "GPU schema boundary"):
            MODULE.validate_checkpoint(mutated)

    def test_blocker_report_and_status_mutation_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["reports"]["pinFailures"] = []
        mutated["blockerCount"] = 0
        mutated["checkpointStatus"] = "FROZEN_COMPLETE"
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "blocker|reports"):
            MODULE.validate_checkpoint(mutated)

    def test_coordinated_false_promotion_is_rejected_by_frozen_evidence(self) -> None:
        mutated = copy.deepcopy(self.document)
        for row in mutated["fourClass"]["pinnedArtifacts"]:
            row["pinStatus"] = "MATCH"
            row["pathKind"] = "REPO_RELATIVE"
        for request in mutated["fourClass"]["assetRequests"]:
            for consumer in request["consumers"]:
                if consumer["consumer"] == "WARLORD":
                    consumer["ownershipStatuses"].append("SOURCE_GRAPH_PIN_MATCH")
                    consumer["ownershipStatuses"] = sorted(
                        set(consumer["ownershipStatuses"])
                    )
        current_archive = mutated["valtan"]["currentInstalledArchiveSnapshot"]
        current_archive["matchesPinnedCorpusArchive"] = True
        current_archive["byteSize"] = MODULE.VALTAN_ARCHIVE_PIN["byteSize"]
        current_archive["sha256"] = MODULE.VALTAN_ARCHIVE_PIN["sha256"]
        for key in (
            "pinFailures",
            "weakAbsoluteSourceHints",
            "candidateIdCollisions",
            "physicalPackageIssues",
            "warlordCatalogGraphDisagreements",
            "valtanProvenanceIssues",
        ):
            mutated["reports"][key] = []
        summary = mutated["fourClass"]["summary"]
        summary["pinFailureCount"] = 0
        summary["absoluteSourceHintCount"] = 0
        summary["warlordCatalogGraphDisagreementCount"] = 0
        mutated["blockerEvidence"] = []
        mutated["blockerEvidenceSha256"] = MODULE.blocker_evidence_sha256([])
        mutated["blockerCount"] = 0
        mutated["checkpointStatus"] = "FROZEN_COMPLETE"
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "frozen baseline"):
            MODULE.validate_checkpoint(mutated)

    def test_self_digest_detects_mutation(self) -> None:
        mutated = copy.deepcopy(self.document)
        mutated["fourClass"]["summary"]["sourceOccurrenceCount"] -= 1
        with self.assertRaisesRegex(MODULE.InventoryError, "self digest"):
            MODULE.validate_checkpoint(mutated)

    def test_nonportable_paths_are_rejected(self) -> None:
        for value in (
            "C:/absolute/file.dds",
            "C:\\absolute\\file.dds",
            "C:drive-relative/file.dds",
            "/rooted/file.dds",
            "Effect/../outside.dds",
        ):
            with self.subTest(value=value):
                with self.assertRaises(MODULE.InventoryError):
                    MODULE.validate_portable_strings({"value": value})

    def test_strict_json_rejects_duplicate_and_nonfinite_values(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            duplicate = Path(directory) / "duplicate.json"
            duplicate.write_text('{"value":1,"value":2}\n', encoding="utf-8")
            with self.assertRaisesRegex(MODULE.InventoryError, "duplicate JSON key"):
                MODULE.load_json(duplicate)
            nonfinite = Path(directory) / "nonfinite.json"
            nonfinite.write_text('{"value":NaN}\n', encoding="utf-8")
            with self.assertRaisesRegex(MODULE.InventoryError, "non-finite"):
                MODULE.load_json(nonfinite)

    def test_candidate_id_preserves_full_object_path(self) -> None:
        first = MODULE.candidate_id(
            "FourClass", "texture", "fx_tex_02.sk_wgl_gdd_01_d"
        )
        second = MODULE.candidate_id(
            "FourClass", "texture", "sk_wgl_gdd_01.tex.sk_wgl_gdd_01_d"
        )
        self.assertNotEqual(first.casefold(), second.casefold())
        self.assertEqual(
            first,
            "Effect/RawCandidates/FourClass/Textures/"
            "fx_tex_02/sk_wgl_gdd_01_d.dds",
        )
        self.assertEqual(
            second,
            "Effect/RawCandidates/FourClass/Textures/"
            "sk_wgl_gdd_01/tex/sk_wgl_gdd_01_d.dds",
        )

    def test_obfuscated_package_mapping_matches_observed_install(self) -> None:
        self.assertEqual(
            MODULE.obfuscate_package_name("fx_sm_03") + ".upk",
            "XFH2RGA2R0LF04YE903X0SMQ.upk",
        )
        self.assertEqual(
            MODULE.obfuscate_package_name("mn_pmsec_00") + ".upk",
            "9G1MU9FPB1GZZEFTQWU463S.upk",
        )

    def test_manifest_serialization_is_deterministic(self) -> None:
        projection = copy.deepcopy(self.document)
        projection.pop("selfDigest")
        first = MODULE.canonical_bytes(projection)
        second = MODULE.canonical_bytes(json.loads(first))
        self.assertEqual(first, second)
        self.assertEqual(
            MODULE.compute_self_digest(self.document), self.document["selfDigest"]
        )

    def test_dds_inspection_checks_exact_block_payload(self) -> None:
        inspected = MODULE.inspect_dds_bytes(build_dds())
        self.assertEqual(inspected["inspectionStatus"], "STRUCTURALLY_VALID")
        self.assertEqual(inspected["header"]["fourCC"], "DXT1")
        self.assertEqual(inspected["header"]["effectiveMipCount"], 1)
        self.assertEqual(
            inspected["layoutFacts"]["expectedPayloadByteSize"], 8
        )
        self.assertTrue(inspected["layoutFacts"]["exactPayloadConsumption"])
        truncated = MODULE.inspect_dds_bytes(build_dds(payload=b"\0" * 7))
        self.assertEqual(truncated["inspectionStatus"], "TRUNCATED_PAYLOAD")
        self.assertEqual(truncated["layoutFacts"]["truncatedByteCount"], 1)
        wrong_linear = bytearray(build_dds())
        struct.pack_into("<I", wrong_linear, 20, 7)
        self.assertEqual(
            MODULE.inspect_dds_bytes(bytes(wrong_linear))["inspectionStatus"],
            "TOP_LEVEL_LINEAR_SIZE_MISMATCH",
        )
        for offset, value, message in (
            (28, 0xFFFFFFFF, "mip count"),
            (8, 0, "header flags"),
            (108, 0, "DDSCAPS_TEXTURE"),
        ):
            mutated = bytearray(build_dds())
            struct.pack_into("<I", mutated, offset, value)
            with self.subTest(offset=offset):
                with self.assertRaisesRegex(MODULE.InventoryError, message):
                    MODULE.inspect_dds_bytes(bytes(mutated))

    def test_dds_dx10_records_raw_fields_without_colourspace_policy(self) -> None:
        base = bytearray(build_dds(four_cc=b"DX10", payload=b""))
        base[128:128] = struct.pack("<5I", 71, 3, 0, 1, 0)
        base.extend(b"\0" * 8)
        inspected = MODULE.inspect_dds_bytes(bytes(base))
        self.assertEqual(inspected["inspectionStatus"], "STRUCTURALLY_VALID")
        self.assertEqual(inspected["header"]["dx10"]["dxgiFormat"], 71)
        self.assertNotIn("colourspace", json.dumps(inspected).casefold())
        self.assertNotIn("sampler", json.dumps(inspected).casefold())
        typeless = bytearray(build_dds(four_cc=b"DX10", payload=b""))
        typeless[128:128] = struct.pack("<5I", 70, 3, 0, 1, 0)
        typeless.extend(b"\0" * 8)
        self.assertEqual(
            MODULE.inspect_dds_bytes(bytes(typeless))["inspectionStatus"],
            "STRUCTURALLY_VALID",
        )

    def test_wmodel_inspection_checks_ranges_indices_and_bounds(self) -> None:
        payload = build_legacy_wmodel()
        inspected = MODULE.inspect_wmodel_bytes(payload)
        mesh = inspected["meshInspection"]["mesh"]
        self.assertEqual(inspected["inspectionStatus"], "STRUCTURALLY_VALID")
        self.assertEqual(mesh["submeshCount"], 1)
        self.assertEqual(mesh["totalVertexCount"], 3)
        self.assertEqual(mesh["totalIndexCount"], 3)
        self.assertTrue(mesh["submeshes"][0]["embeddedBoundsConsistent"])

        mutated = bytearray(payload)
        outer_content = MODULE.WMODEL_FILE_HEADER.size
        table = outer_content + MODULE.WMODEL_MODEL_HEADER.size
        mesh_offset = MODULE.WMODEL_SECTION_DESC.unpack_from(mutated, table)[2]
        mesh_begin = outer_content + mesh_offset
        vertex_begin = (
            mesh_begin
            + MODULE.WMODEL_FILE_HEADER.size
            + MODULE.WMODEL_MESH_HEADER.size
            + MODULE.WMODEL_SUBMESH_DESC.size
        )
        index_begin = vertex_begin + 3 * MODULE.WMODEL_STATIC_VERTEX_STRIDE
        struct.pack_into("<H", mutated, index_begin, 3)
        with self.assertRaisesRegex(MODULE.InventoryError, "index"):
            MODULE.inspect_wmodel_bytes(bytes(mutated))

        bad_version = bytearray(payload)
        struct.pack_into("<H", bad_version, 6, 1)
        with self.assertRaisesRegex(MODULE.InventoryError, "outer"):
            MODULE.inspect_wmodel_bytes(bytes(bad_version))

        bad_section = bytearray(payload)
        struct.pack_into("<I", bad_section, table, 3)
        with self.assertRaisesRegex(MODULE.InventoryError, "unknown section"):
            MODULE.inspect_wmodel_bytes(bytes(bad_section))

        nonfinite = bytearray(payload)
        struct.pack_into("<f", nonfinite, vertex_begin + 12, math.nan)
        with self.assertRaisesRegex(MODULE.InventoryError, "non-finite"):
            MODULE.inspect_wmodel_bytes(bytes(nonfinite))

        bad_bounds = bytearray(payload)
        bounds_begin = index_begin + 3 * 2
        struct.pack_into("<f", bad_bounds, bounds_begin, 2.0)
        with self.assertRaisesRegex(MODULE.InventoryError, "bounds"):
            MODULE.inspect_wmodel_bytes(bytes(bad_bounds))

        bad_material = bytearray(payload)
        material_offset = MODULE.WMODEL_SECTION_DESC.unpack_from(
            bad_material, table + MODULE.WMODEL_SECTION_DESC.size
        )[2]
        material_entry = (
            outer_content + material_offset + MODULE.WMODEL_FILE_HEADER.size + 8
        )
        struct.pack_into("<I", bad_material, material_entry, 1)
        with self.assertRaisesRegex(MODULE.InventoryError, "material indices"):
            MODULE.inspect_wmodel_bytes(bytes(bad_material))

        serialized = json.dumps(inspected)
        for forbidden in (
            "colourspace",
            "sampler",
            "geometryPreScale",
            "materialPacking",
            "rendererPacket",
        ):
            self.assertNotIn(forbidden.casefold(), serialized.casefold())

    def test_fresh_stage_rejects_existing_and_repo_local_roots(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            repo = root / "repo"
            repo.mkdir()
            marker = MODULE.EXPECTED_CHECKPOINT_RAW_SHA256[:16]
            outside = root / f"R8-{marker}-unitrun"
            created = MODULE._require_fresh_external_stage(
                outside,
                repo,
                MODULE.EXPECTED_CHECKPOINT_RAW_SHA256,
                "unitrun",
            )
            self.assertEqual(created, outside.resolve())
            with self.assertRaisesRegex(MODULE.InventoryError, "already exists"):
                MODULE._require_fresh_external_stage(
                    outside,
                    repo,
                    MODULE.EXPECTED_CHECKPOINT_RAW_SHA256,
                    "unitrun",
                )
            inside = repo / f"R8-{marker}-inside"
            with self.assertRaisesRegex(MODULE.InventoryError, "outside"):
                MODULE._require_fresh_external_stage(
                    inside,
                    repo,
                    MODULE.EXPECTED_CHECKPOINT_RAW_SHA256,
                    "inside",
                )

    def test_final_inventory_has_exact_raw_payload_denominator(self) -> None:
        summary = MODULE.validate_final_inventory(self.final_document)
        self.assertEqual(summary["requestCount"], 1212)
        self.assertEqual(summary["successfulRequestCount"], 1208)
        self.assertEqual(summary["rawBlockerCount"], 4)
        self.assertEqual(summary["blockerCount"], 22)
        self.assertEqual(summary["inventoryStatus"], "FROZEN_WITH_BLOCKERS")
        self.assertEqual(
            hashlib.sha256(FINAL_INVENTORY.read_bytes()).hexdigest(),
            EXPECTED_FINAL_RAW_SHA256,
        )
        raw = self.final_document["rawExtraction"]["summary"]
        self.assertEqual(
            raw["payloadKindCounts"],
            {
                "DDS": 1026,
                "GLTF": 182,
                "GLTF_BUFFER": 182,
                "TGA": 4,
                "WMODEL": 182,
            },
        )
        self.assertEqual(raw["uniquePayloadSha256Count"], 1277)
        self.assertEqual(raw["sharedByteGroupCount"], 273)
        self.assertEqual(raw["unassociatedFreshOutputCount"], 88)
        self.assertEqual(
            self.final_document["rawBlockerEvidenceSha256"],
            "c08c2f3db4204da5d453bb9050dcfa901477faf6a3bd0684028ea76c84b0694b",
        )
        self.assertEqual(
            self.final_document["rawResourceProjectionSha256"],
            "c7d9b7d7a8fc9b82627accf34b5ce6b2f21e20eb76b6458b81ba31c67a800e4b",
        )

    def test_final_raw_blockers_are_only_preserved_tga_payloads(self) -> None:
        blockers = self.final_document["rawBlockerEvidence"]
        self.assertEqual(len(blockers), 4)
        self.assertEqual(
            {row["code"] for row in blockers},
            {"UNSUPPORTED_NON_DDS_TEXTURE_PRIMARY"},
        )
        self.assertEqual(
            {row["sourceAssetPath"] for row in blockers},
            {
                "efmaster_material_prologue.tex.flat_gray",
                "fx_tex_nomipmap_00.fx_c_flow_004",
                "efmaster_material_prologue.normal",
                "efmaster_material_prologue.flat_red",
            },
        )
        self.assertTrue(
            all(row["payloads"][0]["kind"] == "TGA" for row in blockers)
        )
        self.assertEqual(
            len(self.final_document["reports"]["unsupportedNonDdsTextures"]), 4
        )
        for key in (
            "rawExtractionMissing",
            "rawOutputAmbiguities",
            "rawStructuralFailures",
            "caseOnlyCandidateIdCollisions",
            "candidateNameCollisions",
        ):
            self.assertEqual(self.final_document["reports"][key], [])

    def test_final_coordinated_false_promotion_is_rejected(self) -> None:
        mutated = copy.deepcopy(self.final_document)
        for corpus_key in ("fourClass", "valtan"):
            for request in mutated[corpus_key]["assetRequests"]:
                if request["rawResource"]["extractionStatus"] != (
                    "EXPORTED_AND_STRUCTURALLY_INSPECTED"
                ):
                    request["rawResource"]["extractionStatus"] = (
                        "EXPORTED_AND_STRUCTURALLY_INSPECTED"
                    )
        mutated["rawBlockerEvidence"] = []
        mutated["rawBlockerEvidenceSha256"] = MODULE.raw_blocker_evidence_sha256([])
        for key in (
            "rawResourceBlockers",
            "rawExtractionMissing",
            "rawOutputAmbiguities",
            "rawStructuralFailures",
            "unsupportedNonDdsTextures",
        ):
            mutated["reports"][key] = []
        mutated["blockerCount"] = 18
        summary = mutated["rawExtraction"]["summary"]
        summary["statusCounts"] = {
            "EXPORTED_AND_STRUCTURALLY_INSPECTED": 1212
        }
        summary["successfulRequestCount"] = 1212
        summary["rawBlockerCount"] = 0
        mutated["rawResourceProjectionSha256"] = (
            MODULE.raw_resource_projection_sha256(mutated)
        )
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(
            MODULE.InventoryError, "frozen baseline|frozen extraction"
        ):
            MODULE.validate_final_inventory(mutated)

    def test_final_embedded_checkpoint_mutations_are_rejected(self) -> None:
        mutations = (
            lambda value: value["fourClass"]["skills"][0].__setitem__(
                "inputSlot", "MUTATED"
            ),
            lambda value: value["fourClass"]["productCues"][0].__setitem__(
                "clip", "MUTATED"
            ),
            lambda value: value["valtan"]["actions"][0].__setitem__(
                "actionId", -1
            ),
            lambda value: value["checkpointProvenance"].__setitem__(
                "selfDigest", "0" * 64
            ),
        )
        for mutate in mutations:
            with self.subTest(mutate=mutate):
                mutated = copy.deepcopy(self.final_document)
                mutate(mutated)
                mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
                with self.assertRaisesRegex(
                    MODULE.InventoryError, "checkpoint provenance|embedded checkpoint"
                ):
                    MODULE.validate_final_inventory(mutated)

    def test_final_rejects_unbound_generator_identity_field(self) -> None:
        mutated = copy.deepcopy(self.final_document)
        mutated["rawExtraction"]["inventoryGenerator"] = {
            "sha256": "0" * 64,
            "byteSize": 1,
        }
        mutated["selfDigest"] = MODULE.compute_self_digest(mutated)
        with self.assertRaisesRegex(MODULE.InventoryError, "field set"):
            MODULE.validate_final_inventory(mutated)


if __name__ == "__main__":
    unittest.main()
