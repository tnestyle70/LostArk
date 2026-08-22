from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock


ROOT = Path(__file__).resolve().parents[2]
SCRIPT_PATH = (
    ROOT
    / "Tools/EffectPipeline/"
    "materialize_four_character_representative_exact_packets.py"
)
SPEC = importlib.util.spec_from_file_location(
    "four_character_representative_packet_plan", SCRIPT_PATH
)
assert SPEC is not None and SPEC.loader is not None
PLAN = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PLAN)
ARTIFACT_PATH = ROOT / PLAN.OUTPUT_RELATIVE_PATH
SCHEMA_PATH = ROOT / PLAN.SCHEMA_RELATIVE_PATH


class FourCharacterRepresentativePacketPlanTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.document = PLAN.build_plan(ROOT)

    @staticmethod
    def _rehash(document: dict) -> None:
        document.pop("artifactSha256", None)
        document["artifactSha256"] = PLAN.canonical_sha256(document)

    def test_real_artifact_is_deterministic_and_byte_current(self) -> None:
        rebuilt = self.document
        self.assertEqual(PLAN.pretty_json_bytes(rebuilt), ARTIFACT_PATH.read_bytes())
        self.assertEqual(
            rebuilt["artifactSha256"],
            "498e5c64e0a02a0ec1288c6fc35c99d646e7f67f9738dea1f1deeedc43916f91",
        )
        PLAN.validate_plan(rebuilt)
        PLAN.validate_input_snapshot(rebuilt, ROOT)

    def test_five_documents_cover_all_131_sprite_mesh_decal_occurrences(self) -> None:
        summary = self.document["summary"]
        self.assertEqual(summary["documentCount"], 5)
        self.assertEqual(summary["occurrenceCount"], 131)
        self.assertEqual(
            summary["carrierCounts"], {"DECAL": 8, "MESH": 25, "SPRITE": 98}
        )
        self.assertEqual(len(self.document["occurrences"]), 131)
        self.assertEqual(
            len(
                {
                    (row["effectAssetId"], row["elementId"])
                    for row in self.document["occurrences"]
                }
            ),
            131,
        )
        self.assertEqual(
            [row["occurrenceCount"] for row in self.document["targetDocuments"]],
            [10, 18, 88, 3, 12],
        )

    def test_current_enabled_packet_and_runtime_admission_are_sealed_at_zero(self) -> None:
        summary = self.document["summary"]
        self.assertEqual(
            summary["inlineExecutionStatusCounts"],
            {"ABSENT": 115, "DISABLED_FAIL_CLOSED_APPROXIMATION": 16},
        )
        self.assertEqual(summary["enabledInlineExecutionCount"], 0)
        self.assertEqual(summary["runtimeRegistryBindingCount"], 0)
        self.assertEqual(summary["runtimeAdmissionCount"], 0)
        self.assertEqual(summary["compiledTupleCount"], 0)
        self.assertEqual(summary["actualAdapterDrawVerifiedCount"], 0)
        for occurrence in self.document["occurrences"]:
            with self.subTest(element=occurrence["elementId"]):
                self.assertFalse(occurrence["inlineExecutionEnabled"])
                self.assertFalse(occurrence["runtimeAdmission"])
                self.assertIsNone(occurrence["proposedEnabledExecution"])
        self.assertFalse(self.document["admission"]["enabledPacketEmission"])

    def test_priority_is_exactly_dimensionmaster_r_three_and_lance_d_five(self) -> None:
        pending = [
            row
            for row in self.document["occurrences"]
            if row["packetDisposition"] == "PACKET_MATERIALIZATION_PENDING"
        ]
        self.assertEqual(len(pending), 8)
        counts: dict[str, int] = {}
        for row in pending:
            counts[row["effectAssetId"]] = counts.get(row["effectAssetId"], 0) + 1
            self.assertIn("PACKET_MATERIALIZATION_PENDING", row["blockers"])
            self.assertEqual(
                row["admissionBasis"],
                "SOURCE_STATE_EXACT_MATCH_STATIC_EVIDENCE_ONLY",
            )
        self.assertEqual(
            counts,
            {
                "effect.dimensionmaster.skill.2050180.unified": 3,
                "effect.lancemaster.skill.34110.unified": 5,
            },
        )

    def test_priority_program_hlsl_named_abi_and_input_identities_are_exact(self) -> None:
        expected = {
            "candidate.family.fx-mm-basic-01-ad.sprite.v1": {
                "dxbc": "f68b6d53c31053f8af2c18a367142c0cd480ac1fee09637c8a36884522256790",
                "hlsl": "1432faa2925f98f4c87b5bdfafff3c6bae6a9098726aa473842b415479f8fda8",
                "abi": "0fc8e401d0e1ada3ee12f9c2e6ba3176faa71eea5b7d33c2f4272a2dc61d574f",
                "inputs": ["v2", "v3", "v4", "v5"],
                "outputs": ["o0"],
            },
            "candidate.family.fx-c-pa-lensflare-01-ad.sprite.v1": {
                "dxbc": "b555abaef0e2477b97b333ae27fd781905ed1876db74d23aad42e3d63525e8b6",
                "hlsl": "3f8f7f98af0134a63791ca2916e257f4de800f27bf8e4a7222b807fd0e89232a",
                "abi": "f2f90cc048d3bcd95f0f4ab90e68636ba7e0591ec1f7ccd28835167f0fdfac36",
                "inputs": ["v2", "v3", "v5", "v7"],
                "outputs": ["o0"],
            },
            "candidate.family.fx-m-pa-noise-01-tr.sprite.v1": {
                "dxbc": "7416554f16b4cc3c196df6d86e73cd2d625f97d58a2b4597f1592be8b7e02ba7",
                "hlsl": "38fcd60dc7aad4a16697d332ac504e09cf77f831651fe3cc29a7b64f2042d26f",
                "abi": "d7c5fc0c2e344e99bdaa30ef69a05135fd3064fcd4f2ae33a6ac7dc6d83e9413",
                "inputs": ["v0", "v1", "v2", "v3", "v4", "v5"],
                "outputs": ["o0", "o2", "o3", "o4", "o5"],
            },
        }
        for family in self.document["candidateFamilies"]:
            with self.subTest(family=family["candidateFamilyId"]):
                sealed = expected[family["candidateFamilyId"]]
                self.assertEqual(family["program"]["dxbcSha256"], sealed["dxbc"])
                self.assertEqual(family["program"]["hlslSha256"], sealed["hlsl"])
                self.assertEqual(
                    family["namedAbi"]["bindingSemanticSha256"], sealed["abi"]
                )
                self.assertEqual(
                    [row["register"] for row in family["program"]["inputs"]],
                    sealed["inputs"],
                )
                self.assertEqual(family["program"]["outputs"], sealed["outputs"])
                self.assertIsNone(family["program"]["compiledRuntimeProgramId"])
                self.assertIsNone(
                    family["namedAbi"]["compiledRuntimeLayoutId"]
                )
                self.assertIsNone(
                    family["adapter"]["compiledRuntimeAdapterId"]
                )
                self.assertFalse(family["adapter"]["actualDrawVerified"])

    def test_artist_a_and_warlord_r_remain_explicitly_blocked(self) -> None:
        blocked_assets = {
            "effect.artist.skill.31460.unified",
            "effect.warlord.skill.17110.clip2.unified",
            "effect.warlord.skill.17110.clip3.unified",
        }
        rows = [
            row
            for row in self.document["occurrences"]
            if row["effectAssetId"] in blocked_assets
        ]
        self.assertEqual(len(rows), 33)
        for row in rows:
            with self.subTest(element=row["elementId"]):
                self.assertEqual(row["packetDisposition"], "BLOCKED")
                self.assertIsNone(row["candidateFamilyId"])
                self.assertIn(
                    "REPRESENTATIVE_PACKET_ADMISSION_NOT_CLOSED",
                    row["blockers"],
                )

    def test_parser_rejects_duplicate_keys_and_non_finite_numbers(self) -> None:
        with self.assertRaisesRegex(PLAN.PlanError, "duplicate key"):
            PLAN.decode_json(b'{"value":1,"value":2}', "duplicate.json")
        for token in (b"NaN", b"Infinity", b"-Infinity"):
            with self.subTest(token=token):
                with self.assertRaisesRegex(PLAN.PlanError, "non-finite"):
                    PLAN.decode_json(b'{"value":' + token + b"}", "finite.json")

    def test_duplicate_occurrence_is_rejected_even_after_rehash(self) -> None:
        tampered = copy.deepcopy(self.document)
        tampered["occurrences"][1]["occurrenceId"] = tampered["occurrences"][0][
            "occurrenceId"
        ]
        self._rehash(tampered)
        with self.assertRaisesRegex(PLAN.PlanError, "duplicated"):
            PLAN.validate_plan(tampered)

    def test_runtime_admission_and_evidence_tamper_are_rejected_after_rehash(self) -> None:
        admitted = copy.deepcopy(self.document)
        admitted["occurrences"][0]["runtimeAdmission"] = True
        self._rehash(admitted)
        with self.assertRaisesRegex(PLAN.PlanError, "runtime admission"):
            PLAN.validate_plan(admitted)

        wrong_dxbc = copy.deepcopy(self.document)
        wrong_dxbc["candidateFamilies"][0]["program"]["dxbcSha256"] = "0" * 64
        self._rehash(wrong_dxbc)
        with self.assertRaisesRegex(PLAN.PlanError, "DXBC"):
            PLAN.validate_plan(wrong_dxbc)

        wrong_self_hash = copy.deepcopy(self.document)
        wrong_self_hash["artifactSha256"] = "f" * 64
        with self.assertRaisesRegex(PLAN.PlanError, "artifactSha256"):
            PLAN.validate_plan(wrong_self_hash)

    def test_stale_input_snapshot_and_stale_output_are_rejected(self) -> None:
        stale_input = copy.deepcopy(self.document)
        tuple_input = next(
            row
            for row in stale_input["inputs"]["files"]
            if row["path"] == PLAN.TUPLE_INVENTORY_BUILDER_PATH.as_posix()
        )
        tuple_input["rawSha256"] = "0" * 64
        self._rehash(stale_input)
        PLAN.validate_plan(stale_input)
        with self.assertRaisesRegex(PLAN.PlanError, "snapshot input bytes drifted"):
            PLAN.validate_input_snapshot(stale_input, ROOT)

        with tempfile.TemporaryDirectory() as directory:
            stale_output = Path(directory) / "stale-plan.json"
            stale_output.write_text("{}\n", encoding="utf-8")
            generated = copy.deepcopy(self.document)
            generated["transaction"]["writablePath"] = stale_output.as_posix()
            self._rehash(generated)
            with mock.patch.object(PLAN, "OUTPUT_RELATIVE_PATH", stale_output), mock.patch.object(
                PLAN, "build_plan", return_value=generated
            ):
                self.assertEqual(PLAN.run(ROOT, check=True), 1)

    def test_plan_writer_only_replaces_contract_and_preserves_target_bytes(self) -> None:
        before = PLAN._target_byte_snapshot(ROOT)
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "representative-plan.json"
            generated = copy.deepcopy(self.document)
            generated["transaction"]["writablePath"] = output.as_posix()
            self._rehash(generated)
            with mock.patch.object(PLAN, "OUTPUT_RELATIVE_PATH", output):
                PLAN.write_plan(generated, ROOT)
                self.assertTrue(output.is_file())
                self.assertEqual(
                    PLAN.pretty_json_bytes(generated), output.read_bytes()
                )
        self.assertEqual(before, PLAN._target_byte_snapshot(ROOT))
        self.assertFalse(self.document["policies"]["authoredWriter"])
        self.assertFalse(self.document["policies"]["registryWriter"])
        self.assertFalse(self.document["policies"]["cppWriter"])
        self.assertEqual(
            self.document["policies"]["rendererDispatch"],
            "PROGRAM_LAYOUT_ADAPTER_TUPLE_ONLY",
        )
        self.assertEqual(
            self.document["policies"]["skillSpecificRendererSwitch"],
            "FORBIDDEN",
        )

    def test_tuple_inventory_is_rebuilt_in_memory_and_checked_in_track_b_is_unused(self) -> None:
        tuple_input = self.document["inputs"]["tupleInventory"]
        self.assertEqual(
            tuple_input["builder"],
            "Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py:build_inventory",
        )
        self.assertEqual(tuple_input["mode"], "IN_MEMORY_CURRENT_HEAD")
        self.assertFalse(tuple_input["checkedInArtifactUsed"])
        self.assertEqual(
            tuple_input["artifactSha256"],
            "e53323119cf5fe9cede0ee40bf1ac71384f8447647a92063f1e5edf6de21b61c",
        )
        checked_in = json.loads(
            (
                ROOT
                / "Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json"
            ).read_text(encoding="utf-8")
        )
        self.assertNotEqual(
            tuple_input["artifactSha256"], checked_in["artifactSha256"]
        )
        file_paths = {row["path"] for row in self.document["inputs"]["files"]}
        self.assertIn(PLAN.TUPLE_INVENTORY_BUILDER_PATH.as_posix(), file_paths)
        self.assertNotIn(
            "Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json",
            file_paths,
        )

        invalid_inventory = {"artifactSha256": "0" * 64}
        with mock.patch.object(
            PLAN.tuple_inventory,
            "build_inventory",
            return_value=invalid_inventory,
        ):
            with self.assertRaisesRegex(PLAN.PlanError, "self hash"):
                PLAN.build_plan(ROOT)

    def test_schema_is_strict_and_check_cli_passes(self) -> None:
        schema = PLAN.decode_json(SCHEMA_PATH.read_bytes(), SCHEMA_PATH.as_posix())
        self.assertEqual(
            schema["$id"],
            "https://lostark.local/schemas/"
            "four-character-representative-packet-plan-v1.json",
        )
        self.assertFalse(schema["additionalProperties"])
        self.assertEqual(
            schema["properties"]["inputs"]["properties"]["tupleInventory"][
                "$ref"
            ],
            "#/$defs/tupleInventoryInput",
        )
        self.assertEqual(
            schema["$defs"]["tupleInventoryInput"]["properties"]["mode"][
                "const"
            ],
            "IN_MEMORY_CURRENT_HEAD",
        )
        self.assertEqual(schema["properties"]["summary"]["properties"][
            "enabledInlineExecutionCount"
        ]["const"], 0)
        self.assertEqual(schema["properties"]["occurrences"]["minItems"], 131)

        result = subprocess.run(
            [sys.executable, str(SCRIPT_PATH), "--check"],
            cwd=ROOT,
            text=True,
            capture_output=True,
            check=False,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("0 enabled, 0 admitted", result.stdout)


if __name__ == "__main__":
    unittest.main()
