import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


SCRIPT_ROOT = Path(__file__).resolve().parent
REPOSITORY_ROOT = SCRIPT_ROOT.parent.parent
if str(SCRIPT_ROOT) not in sys.path:
    sys.path.insert(0, str(SCRIPT_ROOT))

import build_effect_source_runtime_admission as admission  # noqa: E402


BASIS_PATH = (
    REPOSITORY_ROOT
    / "Data"
    / "Effects"
    / "Contracts"
    / "effect-import-bases.v1.json"
)


class EffectSourceRuntimeAdmissionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.index = admission.build_index(REPOSITORY_ROOT, BASIS_PATH)

    def test_positive_join_closes_exact_requested_denominators(self) -> None:
        index = self.index
        self.assertEqual(
            index["scope"]["characterClasses"],
            ["ARTIST", "DIMENSIONMASTER", "LANCE_MASTER"],
        )
        self.assertEqual(
            index["scope"]["expectedDenominators"],
            {
                "stageCount": 12,
                "particleOccurrenceCount": 35,
                "sourceRendererRowCount": 133,
                "currentLegacyComponentCount": 20,
                "currentLegacyEmitterCount": 66,
            },
        )
        self.assertEqual(index["denominators"]["particleSourceRowCount"], 114)
        self.assertEqual(index["denominators"]["nonParticleSourceRowCount"], 19)
        self.assertEqual(index["denominators"]["typedDispositionRowCount"], 133)
        self.assertEqual(index["denominators"]["sourceRuntimeAdmissionCount"], 0)
        self.assertEqual(index["denominators"]["sourceProductAdmissionCount"], 0)
        self.assertEqual(
            index["rendererFamilyCounts"],
            {
                "DECAL_PARTICLE": 2,
                "LIGHT_PARTICLE": 15,
                "MESH_PARTICLE": 41,
                "SCREEN_POST": 2,
                "SPRITE_PARTICLE": 73,
            },
        )
        self.assertNotIn("CASCADE_RIBBON", index["rendererFamilyCounts"])
        self.assertEqual(
            index["typedDispositionCounts"],
            {"FAIL_CLOSED": 67, "LEGACY_APPROXIMATION": 66},
        )
        self.assertEqual(
            index["legacySelectionCounts"],
            {
                "budgetExcluded": 9,
                "notApplicable": 19,
                "rejected": 39,
                "selected": 66,
            },
        )
        self.assertFalse(index["admission"]["runtimeExecution"])
        self.assertFalse(index["admission"]["productAdmission"])
        self.assertFalse(index["admission"]["currentProductMutation"])
        self.assertTrue(index["admission"]["currentProductPreserved"])
        admission.validate_index(index)

    def test_class_data_and_artist_permuted_clip_order_remain_data_driven(self) -> None:
        summaries = {
            row["characterClass"]: row for row in self.index["classSummaries"]
        }
        self.assertEqual(
            {
                key: (
                    value["skillId"],
                    value["particleOccurrenceCount"],
                    value["sourceRendererRowCount"],
                    value["currentLegacyComponentCount"],
                    value["currentLegacyEmitterCount"],
                )
                for key, value in summaries.items()
            },
            {
                "ARTIST": (31000, 14, 33, 6, 8),
                "DIMENSIONMASTER": (2050010, 12, 75, 9, 50),
                "LANCE_MASTER": (34010, 9, 25, 5, 8),
            },
        )
        artist_stages = [
            row
            for row in self.index["stages"]
            if row["characterClass"] == "ARTIST"
        ]
        self.assertEqual(
            [row["clip"] for row in artist_stages],
            [
                "sdm_att_battle_1_03",
                "sdm_att_battle_1_02",
                "sdm_att_battle_1_01",
                "sdm_att_battle_1_04",
            ],
        )
        self.assertEqual(
            [row["productEffectAssetId"] for row in artist_stages],
            [
                "effect.artist.skill.31000.ba1",
                "effect.artist.skill.31000.ba2",
                "effect.artist.skill.31000.ba3",
                "effect.artist.skill.31000.ba4",
            ],
        )

    def test_every_source_row_has_typed_fail_closed_or_legacy_disposition(self) -> None:
        rows = self.index["sourceRows"]
        self.assertEqual(len(rows), 133)
        self.assertTrue(all(row["sourceRecipe"]["enabled"] for row in rows))
        self.assertTrue(
            all(
                row["admission"]["disposition"]
                in {"LEGACY_APPROXIMATION", "FAIL_CLOSED"}
                for row in rows
            )
        )
        self.assertTrue(
            all(not row["admission"]["sourceRuntimeAdmission"] for row in rows)
        )
        self.assertTrue(
            all(not row["admission"]["sourceProductAdmission"] for row in rows)
        )
        selected = [
            row
            for row in rows
            if row["legacyProjection"]["selectionDecision"] == "selected"
        ]
        self.assertEqual(len(selected), 66)
        self.assertTrue(
            all(
                row["admission"]["disposition"] == "LEGACY_APPROXIMATION"
                and row["legacyProjection"]["targetElementId"]
                and row["legacyProjection"]["targetElementSha256"]
                for row in selected
            )
        )
        fail_closed = [
            row
            for row in rows
            if row["admission"]["disposition"] == "FAIL_CLOSED"
        ]
        self.assertEqual(len(fail_closed), 67)
        self.assertTrue(all(row["admission"]["blockers"] for row in fail_closed))

    def test_import_basis_has_no_unproven_rotation_numbers(self) -> None:
        contract = admission.load_json(BASIS_PATH)
        self.assertFalse(contract["runtimeAdmission"])
        self.assertEqual(len(contract["profiles"]), 3)
        for profile in contract["profiles"]:
            self.assertEqual(
                profile["assetBasis"]["status"], "UNRESOLVED_FAIL_CLOSED"
            )
            self.assertIsNone(
                profile["assetBasis"]["sourceToRuntimeRotationDegrees"]
            )
            self.assertIsNone(
                profile["assetBasis"]["meshCarrierAxisRotationDegrees"]
            )
            self.assertIsNone(
                profile["assetBasis"]["attachmentRootRotationDegrees"]
            )
            self.assertEqual(profile["assetBasis"]["evidenceArtifacts"], [])
            self.assertFalse(profile["runtimeAdmission"])

    def test_build_is_byte_deterministic(self) -> None:
        rebuilt = admission.build_index(REPOSITORY_ROOT, BASIS_PATH)
        self.assertEqual(
            admission.pretty_json_bytes(self.index),
            admission.pretty_json_bytes(rebuilt),
        )

    def assert_failed_build_preserves_previous_output(
        self,
        mutating_loader,
        expected_error: str,
    ) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            output_path = Path(temporary) / "runtime-admission.json"
            previous_bytes = b'{"previous":"validated"}\n'
            output_path.write_bytes(previous_bytes)
            with mock.patch.object(
                admission,
                "load_json",
                side_effect=mutating_loader,
            ):
                with self.assertRaisesRegex(admission.ContractError, expected_error):
                    admission.build_and_write(
                        REPOSITORY_ROOT,
                        BASIS_PATH,
                        output_path,
                    )
            self.assertEqual(output_path.read_bytes(), previous_bytes)
            self.assertEqual(list(output_path.parent.glob("*.tmp")), [])

    def test_duplicate_occurrence_candidate_fails_transactionally(self) -> None:
        original_loader = admission.load_json

        def loader(path: Path):
            value = original_loader(path)
            if path.name == "effect.artist.skill.31000.ba1.approximation-receipt.json":
                value = copy.deepcopy(value)
                value["occurrences"][0]["candidates"].append(
                    copy.deepcopy(value["occurrences"][0]["candidates"][0])
                )
            return value

        self.assert_failed_build_preserves_previous_output(
            loader,
            "duplicate particle source candidate",
        )

    def test_stale_source_element_hash_fails_transactionally(self) -> None:
        original_loader = admission.load_json

        def loader(path: Path):
            value = original_loader(path)
            if path.name == "effect.artist.skill.31000.ba1.approximation-receipt.json":
                value = copy.deepcopy(value)
                value["occurrences"][0]["candidates"][0]["sourceElementSha256"] = (
                    "0" * 64
                )
            return value

        self.assert_failed_build_preserves_previous_output(
            loader,
            "stale particle source candidate SHA",
        )

    def test_missing_source_row_fails_transactionally(self) -> None:
        original_loader = admission.load_json

        def loader(path: Path):
            value = original_loader(path)
            if path.name == "effect.artist.skill.31000.ba4.approximation-receipt.json":
                value = copy.deepcopy(value)
                value["occurrences"][0]["candidates"].pop()
            return value

        self.assert_failed_build_preserves_previous_output(
            loader,
            "selected source/current product Element coverage mismatch",
        )

    def test_bad_asset_basis_fails_transactionally(self) -> None:
        original_loader = admission.load_json

        def loader(path: Path):
            value = original_loader(path)
            if path.resolve() == BASIS_PATH.resolve():
                value = copy.deepcopy(value)
                value["commonCoordinateBasis"]["position"]["axisMatrix"][1] = [
                    0.0,
                    1.0,
                    0.0,
                ]
                hash_payload = copy.deepcopy(value)
                del hash_payload["contractSha256"]
                value["contractSha256"] = admission.canonical_json_sha256(
                    hash_payload
                )
            return value

        self.assert_failed_build_preserves_previous_output(
            loader,
            "commonCoordinateBasis.position changed without source evidence",
        )

    def test_schema_and_basis_json_are_utf8_json_objects(self) -> None:
        schema_path = REPOSITORY_ROOT / admission.SCHEMA_RELATIVE_PATH
        for path in (schema_path, BASIS_PATH):
            raw = path.read_bytes()
            self.assertFalse(raw.startswith(b"\xef\xbb\xbf"))
            parsed = json.loads(raw.decode("utf-8"))
            self.assertIsInstance(parsed, dict)
        schema = json.loads(schema_path.read_text(encoding="utf-8"))
        self.assertEqual(
            schema["properties"]["schema"]["const"],
            admission.INDEX_SCHEMA,
        )


if __name__ == "__main__":
    unittest.main()
