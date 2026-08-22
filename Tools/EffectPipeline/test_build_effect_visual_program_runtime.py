#!/usr/bin/env python3
"""Regression tests for the executable V6 visual-program runtime sidecar."""

from __future__ import annotations

import copy
import importlib.util
import tempfile
import unittest
from pathlib import Path, PurePosixPath


SCRIPT_PATH = Path(__file__).resolve().with_name(
    "build_effect_visual_program_runtime.py"
)
SPEC = importlib.util.spec_from_file_location(
    "build_effect_visual_program_runtime",
    SCRIPT_PATH,
)
assert SPEC is not None and SPEC.loader is not None
builder = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(builder)


def reseal(value: dict, field: str) -> None:
    value.pop(field, None)
    value[field] = builder.canonical_json_sha256(value)


def reseal_runtime_row(runtime: dict, program: dict, row: dict) -> None:
    reseal(row, "rowSha256")
    reseal(program, "programSha256")
    reseal(runtime, "artifactSha256")


class EffectVisualProgramRuntimeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.repository_root = builder.REPOSITORY_ROOT
        cls.runtime = builder.build_runtime(cls.repository_root)
        cls.corpus, _ = builder._load_source_corpus(cls.repository_root)

    def _first_overlay_context(self) -> tuple[dict, dict, list[dict]]:
        program = next(
            item for item in self.runtime["programs"]
            if item["projectionKind"] == "SOURCE_RECIPE_OVERLAY_V1"
        )
        admission = builder.load_json(
            self.repository_root / builder.ADMISSION_RELATIVE_PATH
        )
        stage = next(
            item for item in admission["stages"]
            if item["productEffectAssetId"] == program["effectAssetId"]
        )
        base = builder.load_json(
            self.repository_root
            / PurePosixPath(stage["currentProduct"]["authoringPath"])
        )
        rows = [
            item for item in self.corpus["visualRows"]
            if item["selector"]["effectAssetId"] == program["effectAssetId"]
        ]
        return program, base, rows

    def test_schema_declares_closed_runtime_projection_shape(self) -> None:
        schema = builder.load_json(
            self.repository_root / builder.SCHEMA_RELATIVE_PATH
        )
        self.assertEqual(
            schema["$defs"]["selector"]["required"],
            ["effectAssetId", "occurrenceId"],
        )
        self.assertIn(
            "localDecalPacket",
            schema["$defs"]["runtimeRow"]["required"],
        )
        packet = schema["$defs"]["localDecalPacket"]
        self.assertEqual(
            packet["properties"]["inputDispositions"]["minItems"], 33
        )
        self.assertEqual(
            packet["properties"]["staticDispositions"]["minItems"], 18
        )
        self.assertEqual(
            packet["properties"]["renderDispositions"]["minItems"], 6
        )
        self.assertIn(
            "projectedDocumentCanonicalByteCount",
            schema["$defs"]["program"]["required"],
        )
        self.assertIn(
            "projectedDocumentTypedCodecSha256",
            schema["$defs"]["program"]["required"],
        )
        self.assertIn(
            "typedCodecSha256",
            schema["$defs"]["baseDocumentIdentity"]["required"],
        )
        self.assertIn(
            "supplementalElements",
            schema["$defs"]["program"]["required"],
        )

    def test_exact_denominators_and_projected_document_bytes_sha(self) -> None:
        runtime = self.runtime
        self.assertEqual(runtime["denominators"], {
            "programCount": 17,
            "sourceRecipeOverlayProgramCount": 16,
            "adapterPacketProgramCount": 1,
            "visualRowCount": 135,
            "sourceRecipeOverlayCount": 66,
            "localDecalAdapterPacketCount": 2,
            "cascadeRibbonVisualRowCount": 4,
            "supplementalElementCount": 16,
            "artistFCascadeRibbonElementCount": 1,
            "artistTCascadeRibbonElementCount": 1,
            "animationTrailElementCount": 13,
            "bakedEdgeLightElementCount": 1,
            "failClosedCount": 67,
            "extensionCanaryCount": 2,
            "productMutationCount": 0,
        })
        overlays = [
            item for item in runtime["programs"]
            if item["projectionKind"] == "SOURCE_RECIPE_OVERLAY_V1"
        ]
        self.assertEqual(len(overlays), 16)
        for program in overlays:
            document = program["projectedDocument"]
            canonical = builder.canonical_json_bytes(document)
            self.assertEqual(
                len(canonical), program["projectedDocumentCanonicalByteCount"]
            )
            self.assertEqual(
                builder.canonical_json_sha256(document),
                program["projectedDocumentSha256"],
            )
            self.assertRegex(
                program["baseDocumentIdentity"]["typedCodecSha256"],
                builder.SHA256_RE,
            )
            self.assertRegex(
                program["projectedDocumentTypedCodecSha256"],
                builder.SHA256_RE,
            )
            builder.validate_standard_document(
                document, program["effectAssetId"]
            )

    def test_whirlwind_legacy_lf_base_identity_is_the_only_raw_sha_exception(self) -> None:
        path = (
            self.repository_root
            / PurePosixPath(builder.VALTAN_WHIRLWIND_DOCUMENT_RELATIVE_PATH)
        )
        builder._validate_overlay_base_raw_sha(
            self.repository_root,
            path,
            builder.phase1.VALTAN_WHIRLWIND_LEGACY_DOCUMENT_PAYLOAD_RAW_SHA256,
            builder.VALTAN_WHIRLWIND_EFFECT_ASSET_ID,
            self.corpus,
        )
        with self.assertRaisesRegex(
            builder.ContractError, "BA base document raw SHA changed"
        ):
            builder._validate_overlay_base_raw_sha(
                self.repository_root,
                path,
                "0" * 64,
                builder.VALTAN_WHIRLWIND_EFFECT_ASSET_ID,
                self.corpus,
            )
        changed = copy.deepcopy(self.corpus)
        input_row = next(
            item
            for item in changed["inputArtifacts"]
            if item["path"] == builder.VALTAN_WHIRLWIND_DOCUMENT_RELATIVE_PATH
        )
        input_row["rawSha256"] = "0" * 64
        with self.assertRaisesRegex(
            builder.ContractError, "physical authoring canary diverged"
        ):
            builder._validate_overlay_base_raw_sha(
                self.repository_root,
                path,
                builder.phase1.VALTAN_WHIRLWIND_LEGACY_DOCUMENT_PAYLOAD_RAW_SHA256,
                builder.VALTAN_WHIRLWIND_EFFECT_ASSET_ID,
                changed,
            )

    def test_ba_overlay_preserves_carrier_and_executes_exact_source_recipe(self) -> None:
        admission = builder.load_json(
            self.repository_root / builder.ADMISSION_RELATIVE_PATH
        )
        stage_by_id = {
            item["productEffectAssetId"]: item for item in admission["stages"]
        }
        admitted_count = 0
        for program in self.runtime["programs"]:
            if program["projectionKind"] != "SOURCE_RECIPE_OVERLAY_V1":
                continue
            effect_asset_id = program["effectAssetId"]
            phase1_rows = [
                row
                for row in self.corpus["visualRows"]
                if row["selector"]["effectAssetId"] == effect_asset_id
            ]
            phase1_supplemental = [
                row
                for row in self.corpus["supplementalElements"]
                if row["selector"]["effectAssetId"] == effect_asset_id
            ]
            base_path, _ = builder._resolve_overlay_base_document(
                self.repository_root,
                effect_asset_id,
                phase1_rows,
                phase1_supplemental,
                stage_by_id,
            )
            base = builder.load_json(base_path)
            base_by_id = {item["id"]: item for item in base["elements"]}
            projected_by_id = {
                item["id"]: item
                for item in program["projectedDocument"]["elements"]
            }
            supplemental_targets = {
                item["targetPayload"]["recordId"]: item["family"]
                for item in phase1_supplemental
            }
            admitted_targets = set()
            for row in program["visualRows"]:
                if row["disposition"] != "ADMITTED_BOUNDED":
                    self.assertIsNone(row["targetIdentity"])
                    self.assertIsNone(row["localDecalPacket"])
                    continue
                admitted_count += 1
                target_id = row["targetIdentity"]["targetElementId"]
                admitted_targets.add(target_id)
                after = copy.deepcopy(projected_by_id[target_id])
                source_recipe = after.pop("sourceRecipe")
                if target_id in base_by_id:
                    before = copy.deepcopy(base_by_id[target_id])
                    before.pop("sourceRecipe")
                    self.assertEqual(before, after)
                else:
                    self.assertEqual(row["family"], "CASCADE_RIBBON")
                    self.assertEqual(after["kind"], "trail")
                    self.assertEqual(source_recipe["rendererShape"], "ribbon")
                self.assertEqual(
                    builder.canonical_json_sha256(source_recipe),
                    row["sourceIdentity"]["sourceRecipeSha256"],
                )
                self.assertEqual(
                    builder.canonical_json_sha256(source_recipe["modules"]),
                    row["sourceIdentity"]["moduleClosureSha256"],
                )
                self.assertEqual(
                    len(source_recipe["modules"]),
                    row["sourceIdentity"]["moduleCount"],
                )
            for element_id in (
                set(base_by_id) - admitted_targets - set(supplemental_targets)
            ):
                self.assertEqual(base_by_id[element_id], projected_by_id[element_id])
            for element_id in supplemental_targets:
                self.assertIn(element_id, projected_by_id)
                if element_id in base_by_id:
                    self.assertEqual(
                        base_by_id[element_id]["kind"],
                        projected_by_id[element_id]["kind"],
                    )
                else:
                    self.assertIn(
                        supplemental_targets[element_id],
                        {"ANIMATION_TRAIL", "CASCADE_RIBBON"},
                    )
                    self.assertEqual(projected_by_id[element_id]["kind"], "trail")
        self.assertEqual(admitted_count, 66)

    def test_cascade_rows_fail_closed_and_animation_trail_packets_are_stable_elements(self) -> None:
        cascade_rows = [
            row
            for program in self.runtime["programs"]
            for row in program["visualRows"]
            if row["family"] == "CASCADE_RIBBON"
        ]
        self.assertEqual(len(cascade_rows), 4)
        self.assertTrue(all(
            row["sourceIdentity"]["moduleCount"] == 10
            and row["disposition"] == "FAIL_CLOSED"
            and row["packetLayout"] == "NONE"
            and row["targetIdentity"] is None
            and row["admissionBlockers"]
            for row in cascade_rows
        ))
        for row in cascade_rows:
            program = next(
                item for item in self.runtime["programs"]
                if item["effectAssetId"] == row["selector"]["effectAssetId"]
            )
            self.assertFalse(any(
                item["id"].endswith("ribbon-companion-blocked")
                for item in program["projectedDocument"]["elements"]
            ))

        supplemental = [
            item
            for program in self.runtime["programs"]
            for item in program["supplementalElements"]
        ]
        self.assertEqual(len(supplemental), 16)
        self.assertEqual(
            sorted(item["family"] for item in supplemental),
            ["ANIMATION_TRAIL"] * 13
            + ["CASCADE_RIBBON"] * 2
            + ["LIGHT_PARTICLE"],
        )
        for item in supplemental:
            self.assertEqual(
                set(item["selector"]), {"effectAssetId", "occurrenceId"}
            )
            self.assertTrue(item["tuningEligibleTransform"])
            self.assertEqual(item["disposition"], "ADMITTED_BOUNDED")
            if item["family"] == "ANIMATION_TRAIL":
                packet = item["animationTrailPacket"]
                self.assertIsNone(item["cascadeRibbonPacket"])
                self.assertEqual(packet["sourceNotifyType"], "Trails")
                self.assertGreater(packet["durationSeconds"], 0.0)
                self.assertEqual(
                    packet["targetElementId"],
                    item["targetIdentity"]["targetElementId"],
                )
            elif item["family"] == "CASCADE_RIBBON":
                packet = item["cascadeRibbonPacket"]
                self.assertIsNone(item["animationTrailPacket"])
                if item["selector"]["effectAssetId"] == "effect.artist.skill.31470":
                    self.assertEqual(
                        (
                            packet["typeDataStableId"],
                            packet["tilingDistance"],
                            packet["distanceTessellationStepSize"],
                            packet["operationalMaxPoints"],
                        ),
                        ("FX_PC_SDM_07:export:1293@ref:6", 6.0, 0.05, 500),
                    )
                else:
                    self.assertEqual(
                        item["selector"],
                        {
                            "effectAssetId": "effect.artist.skill.31950.unified",
                            "occurrenceId":
                                "authored.source-particle.29868adeb040d5a35e2f213c",
                        },
                    )
                    self.assertEqual(
                        (
                            packet["typeDataStableId"],
                            packet["tilingDistance"],
                            packet["distanceTessellationStepSize"],
                        ),
                        (
                            "FX_PC_SDM_01:export:1495@ref:6",
                            3.0,
                            0.05,
                        ),
                    )
                    self.assertEqual(
                        packet["preservedLimitations"],
                        [
                            "CASCADE_RIBBON_BOUNDED_RECONSTRUCTION_NOT_NATIVE_SOURCE_EXACT",
                            *builder.phase1.ARTIST_T_RIBBON_MATERIAL_LIMITATIONS,
                        ],
                    )
            else:
                packet = item["bakedEdgeLightPacket"]
                self.assertEqual(item["family"], "LIGHT_PARTICLE")
                self.assertIsNone(item["cascadeRibbonPacket"])
                self.assertIsNone(item["animationTrailPacket"])
                self.assertEqual(
                    packet["runtimeCarrier"],
                    "EFFECT_TYPED_LIGHT_BAKED_EDGE_ATTACHMENT_V1",
                )
                self.assertEqual(packet["lane"], "FIRST_EDGE")

    def test_projection_rejects_stale_source_recipe_module_and_target(self) -> None:
        _, base, rows = self._first_overlay_context()
        admitted = next(
            item for item in rows
            if item["executionProjection"]["disposition"] == "ADMITTED_BOUNDED"
        )
        mutations = (
            ("SourceRecipe SHA", "recipeSha256"),
            ("module closure", "moduleClosureSha256"),
        )
        for expected, field in mutations:
            with self.subTest(field=field):
                changed = copy.deepcopy(rows)
                row = next(
                    item for item in changed
                    if item["selector"] == admitted["selector"]
                )
                row["sourceRecipe"][field] = "0" * 64
                with self.assertRaisesRegex(builder.ContractError, expected):
                    builder.project_ba_document(
                        self.repository_root, base, changed
                    )
        changed = copy.deepcopy(rows)
        row = next(
            item for item in changed if item["selector"] == admitted["selector"]
        )
        row["targetPayload"]["recordSha256"] = "0" * 64
        with self.assertRaisesRegex(builder.ContractError, "record SHA changed"):
            builder.project_ba_document(self.repository_root, base, changed)

    def test_projection_failure_rolls_back_input_document(self) -> None:
        _, base, rows = self._first_overlay_context()
        original = copy.deepcopy(base)
        changed = copy.deepcopy(rows)
        admitted = next(
            item for item in changed
            if item["executionProjection"]["disposition"] == "ADMITTED_BOUNDED"
        )
        admitted["sourceRecipe"]["moduleClosureSha256"] = "f" * 64
        with self.assertRaises(builder.ContractError):
            builder.project_ba_document(self.repository_root, base, changed)
        self.assertEqual(base, original)

    def test_local_decal_materializes_complete_bounded_runtime_packet(self) -> None:
        program = next(
            item for item in self.runtime["programs"]
            if item["effectAssetId"] == "effect.artist.skill.31470"
        )
        self.assertEqual(
            {row["selector"]["occurrenceId"] for row in program["visualRows"]},
            {"source-active-020", "source-active-021"},
        )
        for row in program["visualRows"]:
            self.assertEqual(
                set(row["selector"]), {"effectAssetId", "occurrenceId"}
            )
            packet = row["localDecalPacket"]
            self.assertEqual(
                (len(packet["inputDispositions"]),
                 len(packet["staticDispositions"]),
                 len(packet["renderDispositions"]),
                 len(packet["packedScalars"]),
                 len(packet["packedVectors"]),
                 len(packet["srvs"])),
                (33, 18, 6, 22, 3, 6),
            )
            self.assertEqual(
                (packet["opcode"], packet["passIndex"],
                 packet["runtimeCarrier"], packet["renderProfile"]),
                (14, 3, "EFFECT_TYPED_DECAL_PROJECTOR_RECT_V1",
                 "ALPHA_ONE_SIDED_DEPTH_READ"),
            )
            self.assertTrue(packet["boundedSemanticReplay"])
            self.assertFalse(packet["nativeExecution"])
            self.assertFalse(packet["nativeVertexFactoryAdmitted"])
            self.assertFalse(packet["nativeMrtAdmitted"])
            self.assertEqual(
                [(srv["shaderRegister"], srv["runtimeSamplerRegister"])
                 for srv in packet["srvs"]],
                [(f"t{i}", f"s{i + 5}") for i in range(6)],
            )
            self.assertEqual(
                [(srv["role"], srv["linearFormat"], srv["sourceChannel"])
                 for srv in packet["srvs"]],
                [
                    ("HEIGHT", "BC1_UNORM", "B"),
                    ("DIFFUSE", "BC3_UNORM", "RGBA"),
                    ("DISSOLVE", "BC1_UNORM", "G"),
                    ("NORMAL", "BC5_UNORM", "RG"),
                    ("SPECULAR", "BC1_UNORM", "RGB"),
                    ("EMISSIVE", "BC1_UNORM", "R"),
                ],
            )
            self.assertEqual(
                [srv["sourceSamplerEvidence"] for srv in packet["srvs"]],
                ["s0", None, "s5", None, None, None],
            )
            self.assertEqual(
                [(srv["width"], srv["height"], srv["mipCount"],
                  srv["arraySize"]) for srv in packet["srvs"]],
                [(256, 256, 1, 1)] * 6,
            )
            self.assertEqual(
                packet["inputConsumedMask"], [0x820EC1FF, 1]
            )
            self.assertEqual(
                packet["inputSuppressedMask"], [0x7DF13E00, 0]
            )

    def test_selector_leakage_duplicate_and_packet_mutation_fail_closed(self) -> None:
        runtime = copy.deepcopy(self.runtime)
        program = runtime["programs"][0]
        row = program["visualRows"][0]
        row["selector"]["skillId"] = 31000
        row["selectorSha256"] = builder.canonical_json_sha256(row["selector"])
        reseal_runtime_row(runtime, program, row)
        with self.assertRaisesRegex(builder.ContractError, "selector keys mismatch"):
            builder.validate_runtime(runtime, self.repository_root)

        runtime = copy.deepcopy(self.runtime)
        program = next(item for item in runtime["programs"] if len(item["visualRows"]) > 1)
        program["visualRows"][1]["selector"] = copy.deepcopy(
            program["visualRows"][0]["selector"]
        )
        program["visualRows"][1]["selectorSha256"] = builder.canonical_json_sha256(
            program["visualRows"][1]["selector"]
        )
        reseal(program["visualRows"][1], "rowSha256")
        program["visualRows"].sort(key=lambda item: item["selector"]["occurrenceId"])
        reseal(program, "programSha256")
        reseal(runtime, "artifactSha256")
        with self.assertRaisesRegex(builder.ContractError, "duplicate runtime selector"):
            builder.validate_runtime(runtime, self.repository_root)

        runtime = copy.deepcopy(self.runtime)
        program = next(
            item for item in runtime["programs"]
            if item["effectAssetId"] == "effect.artist.skill.31470"
        )
        row = program["visualRows"][0]
        row["localDecalPacket"]["passIndex"] = 4
        reseal(row["localDecalPacket"], "packetSha256")
        reseal_runtime_row(runtime, program, row)
        with self.assertRaisesRegex(builder.ContractError, "identity diverged"):
            builder.validate_runtime(runtime, self.repository_root)

    def test_warlord_valtan_canaries_remain_non_product(self) -> None:
        self.assertEqual(
            {(item["domain"], item["fidelity"], item["disposition"])
             for item in self.runtime["extensionCanaries"]},
            {
                ("WARLORD", "EVIDENCE_ONLY", "FAIL_CLOSED"),
                ("VALTAN", "EVIDENCE_ONLY", "FAIL_CLOSED"),
            },
        )
        for item in self.runtime["extensionCanaries"]:
            self.assertEqual(
                set(item["selector"]), {"effectAssetId", "occurrenceId"}
            )
            self.assertFalse(item["productCountContribution"])

    def test_builder_is_deterministic_and_check_detects_stale_output(self) -> None:
        first = builder.build_runtime(self.repository_root)
        second = builder.build_runtime(self.repository_root)
        self.assertEqual(
            builder.pretty_json_bytes(first), builder.pretty_json_bytes(second)
        )
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "runtime.json"
            builder.write_runtime_transactionally(
                first, output, self.repository_root
            )
            builder.build_and_write(
                self.repository_root, output, check=True
            )
            output.write_bytes(output.read_bytes() + b" ")
            with self.assertRaisesRegex(builder.ContractError, "is stale"):
                builder.build_and_write(
                    self.repository_root, output, check=True
                )

    def test_published_artifact_gate_is_closed_and_rejects_resealed_counts(self) -> None:
        builder.validate_published_artifact(self.runtime)

        changed = copy.deepcopy(self.runtime)
        changed["denominators"]["visualRowCount"] += 1
        reseal(changed, "artifactSha256")
        with self.assertRaisesRegex(
            builder.ContractError, "publish denominators are stale"
        ):
            builder.validate_published_artifact(changed)

        changed = copy.deepcopy(self.runtime)
        row = changed["programs"][0]["visualRows"][0]
        row["selector"]["skillId"] = 31000
        row["selectorSha256"] = builder.canonical_json_sha256(
            row["selector"]
        )
        reseal(row, "rowSha256")
        reseal(changed["programs"][0], "programSha256")
        reseal(changed, "artifactSha256")
        with self.assertRaisesRegex(
            builder.ContractError, "selector keys mismatch"
        ):
            builder.validate_published_artifact(changed)

    def test_invalid_stage_preserves_previous_output(self) -> None:
        invalid = copy.deepcopy(self.runtime)
        program = invalid["programs"][0]
        row = program["visualRows"][0]
        row["selector"]["characterClass"] = "ARTIST"
        row["selectorSha256"] = builder.canonical_json_sha256(row["selector"])
        reseal_runtime_row(invalid, program, row)
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "previous.json"
            previous = b"previous-valid-runtime\n"
            output.write_bytes(previous)
            with self.assertRaises(builder.ContractError):
                builder.write_runtime_transactionally(
                    invalid, output, self.repository_root
                )
            self.assertEqual(output.read_bytes(), previous)


if __name__ == "__main__":
    unittest.main()
