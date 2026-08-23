from __future__ import annotations

import copy
import hashlib
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


TOOLS_DIRECTORY = Path(__file__).resolve().parent
REPOSITORY_ROOT = TOOLS_DIRECTORY.parents[1]
if str(TOOLS_DIRECTORY) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIRECTORY))

import build_effect_material_program_conquest_ledger as conquest
import build_effect_tuple_cohort_inventory as cohort_inventory


class EffectMaterialProgramConquestLedgerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        source_path = (
            REPOSITORY_ROOT
            / "Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json"
        )
        cls.source_inventory = json.loads(source_path.read_text(encoding="utf-8"))
        source_payload = source_path.read_bytes()
        cls.fixture_inventory_seal = {
            "conventionalArtifactPath": conquest.SOURCE_ARTIFACT_RELATIVE_PATH.as_posix(),
            "conventionalArtifactByteSize": len(source_payload),
            "conventionalArtifactRawSha256": hashlib.sha256(source_payload).hexdigest(),
            "conventionalArtifactDeclaredSha256": cls.source_inventory[
                "artifactSha256"
            ],
            "conventionalArtifactByteCurrent": True,
        }
        cls.document = conquest.build_ledger(REPOSITORY_ROOT)

    def test_denominators_and_program_counts_are_canonical(self) -> None:
        summary = self.document["summary"]
        self.assertEqual(
            summary["denominatorCounts"],
            {
                "AUTHORED": 7566,
                "PRODUCT": 2554,
                "FOUR_CHARACTER_PRODUCT": 1885,
                "VALTAN_PRODUCT": 669,
            },
        )
        self.assertEqual(summary["programRowCount"], 1325)
        self.assertEqual(summary["sourceProgramCandidateCount"], 171)
        self.assertEqual(summary["typedRuntimeProgramCount"], 18)
        self.assertEqual(summary["literalDxbcProgramCount"], 169)
        self.assertEqual(summary["opcodeUnallocatedLiteralProgramCount"], 169)
        self.assertEqual(summary["productDistinctTypedProgramCount"], 17)
        self.assertEqual(summary["productDistinctExactLiteralProgramCount"], 111)
        self.assertEqual(summary["productDistinctExactUntranslatedProgramCount"], 1)
        self.assertEqual(summary["productExactUntranslatedOccurrenceCount"], 4)
        self.assertEqual(summary["publicProgramAllocationCount"], 3)

    def test_program_identity_is_class_neutral_and_content_addressed(self) -> None:
        forbidden = {
            "class",
            "characterClass",
            "skillId",
            "filename",
            "occurrenceId",
            "effectAssetId",
            "elementId",
        }
        for row in self.document["programs"]:
            identity = row["candidateIdentity"]
            self.assertFalse(forbidden.intersection(identity))
            self.assertEqual(
                row["programLedgerId"], conquest._program_ledger_id(identity)
            )
            if row["sourceKind"] == "TYPED_RUNTIME_PROGRAM":
                self.assertEqual(
                    set(identity), {"sourceKind", "backend", "opcode"}
                )
            if row["sourceKind"] == "LITERAL_DXBC_PROGRAM":
                self.assertEqual(
                    set(identity), {"sourceKind", "dxbcSha256"}
                )
                self.assertEqual(
                    row["equation"]["opcodeAllocation"], "OPCODE_UNALLOCATED"
                )
                self.assertIsNone(row["equation"]["opcode"])
            for program_id in row["publicAllocation"]["programIds"]:
                conquest._require_public_id_is_class_neutral(
                    program_id, "test public programId"
                )
            for layout_id in row["publicAllocation"]["layoutIds"]:
                conquest._require_public_id_is_class_neutral(
                    layout_id, "test public layoutId"
                )

    def test_public_registry_allocation_is_exact_s6_m3_d14(self) -> None:
        allocated = {
            (
                row["candidateIdentity"]["backend"],
                row["candidateIdentity"]["opcode"],
            ): row["publicAllocation"]
            for row in self.document["programs"]
            if row["publicAllocation"]["status"] == "PUBLIC_PROGRAM_ALLOCATED"
        }
        self.assertEqual(
            allocated,
            {
                ("runtimeMaterialV2", 6): {
                    "status": "PUBLIC_PROGRAM_ALLOCATED",
                    "programIds": ["effect.program.runtime-material-v2.opcode-6.v1"],
                    "layoutIds": [
                        "effect.layout.runtime-material-v2.opcode-6.abi-3aafae1b4639c551.v1"
                    ],
                },
                ("runtimeMaterialV2", 3): {
                    "status": "PUBLIC_PROGRAM_ALLOCATED",
                    "programIds": ["effect.program.runtime-material-v2.opcode-3.v1"],
                    "layoutIds": [
                        "effect.layout.runtime-material-v2.opcode-3.abi-85c02e5f1f646d22.v1"
                    ],
                },
                ("localDecal", 14): {
                    "status": "PUBLIC_PROGRAM_ALLOCATED",
                    "programIds": ["effect.program.local-decal.opcode-14.v1"],
                    "layoutIds": [
                        "effect.layout.local-decal.opcode-14.abi-c6b52a791b98f0c5.v1"
                    ],
                },
            },
        )
        typed_only = [
            row
            for row in self.document["programs"]
            if row["sourceKind"] == "TYPED_RUNTIME_PROGRAM"
            and row["publicAllocation"]["status"]
            == "TYPED_OPCODE_ONLY_NOT_PUBLIC_PROGRAM"
        ]
        self.assertEqual(len(typed_only), 15)

    def test_public_registry_rejects_backend_opcode_alias(self) -> None:
        registry = conquest.material_registry.build_registry(
            REPOSITORY_ROOT / conquest.PUBLIC_REGISTRY_BASE_RELATIVE_PATH,
            REPOSITORY_ROOT / conquest.PUBLIC_EFFECT_CATALOG_RELATIVE_PATH,
            REPOSITORY_ROOT / conquest.PUBLIC_DATA_ROOT_RELATIVE_PATH,
            REPOSITORY_ROOT / conquest.PUBLIC_REGISTRY_FRAGMENT_ROOT_RELATIVE_PATH,
        )
        registry = copy.deepcopy(registry)
        alias = copy.deepcopy(registry["programs"][0])
        alias["programId"] = "effect.program.runtime-material-v2.opcode-6.alias.v1"
        registry["programs"].append(alias)
        with mock.patch.object(
            conquest.material_registry, "build_registry", return_value=registry
        ):
            with self.assertRaises(conquest.LedgerError):
                conquest._build_public_registry_projection(REPOSITORY_ROOT)

    def test_validator_rejects_domain_tokens_in_public_ids(self) -> None:
        for token in sorted(conquest.PUBLIC_ID_FORBIDDEN_TOKENS):
            for field, prefix in (
                ("programIds", "effect.program"),
                ("layoutIds", "effect.layout"),
            ):
                with self.subTest(token=token, field=field):
                    document = copy.deepcopy(self.document)
                    row = next(
                        item
                        for item in document["programs"]
                        if item["publicAllocation"]["status"]
                        == "PUBLIC_PROGRAM_ALLOCATED"
                    )
                    row["publicAllocation"][field] = [
                        f"{prefix}.{token}.opcode-6.v1"
                    ]
                    with self.assertRaises(conquest.LedgerError):
                        conquest.validate_ledger(document)

    def test_conventional_inventory_must_be_byte_current(self) -> None:
        live = {"artifactSha256": "a" * 64}
        current_payload = conquest.pretty_json_bytes(live)
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            path = root / conquest.SOURCE_ARTIFACT_RELATIVE_PATH
            path.parent.mkdir(parents=True)
            path.write_bytes(current_payload)
            with mock.patch.object(
                cohort_inventory, "validate_inventory", return_value=None
            ):
                seal = conquest._seal_current_conventional_inventory(root, live)
                self.assertTrue(seal["conventionalArtifactByteCurrent"])
                path.write_bytes(current_payload + b" ")
                with self.assertRaises(conquest.LedgerError):
                    conquest._seal_current_conventional_inventory(root, live)

    def test_all_source_candidates_project_once(self) -> None:
        candidate_ids = [
            candidate_id
            for row in self.document["programs"]
            for candidate_id in row["sourceProgramCandidateIds"]
        ]
        self.assertEqual(len(candidate_ids), 171)
        self.assertEqual(len(candidate_ids), len(set(candidate_ids)))
        self.assertEqual(len(self.document["occurrenceProjections"]), 7566)
        self.assertEqual(
            len(
                {
                    row["occurrenceId"]
                    for row in self.document["occurrenceProjections"]
                }
            ),
            7566,
        )

    def test_builder_consumes_tuple_inventory_builder(self) -> None:
        with (
            mock.patch.object(
                cohort_inventory,
                "build_inventory",
                return_value=self.source_inventory,
            ) as build_inventory,
            mock.patch.object(
                conquest,
                "_seal_current_conventional_inventory",
                return_value=self.fixture_inventory_seal,
            ),
        ):
            document = conquest.build_ledger(REPOSITORY_ROOT)
        build_inventory.assert_called_once_with(REPOSITORY_ROOT.resolve())
        self.assertEqual(document["summary"]["denominatorCounts"]["AUTHORED"], 7566)

    def test_csv_is_exact_json_derived_projection(self) -> None:
        payload = conquest.csv_bytes(self.document)
        conquest.validate_csv_bytes(self.document, payload)
        tampered = payload.replace(b"OPCODE_UNALLOCATED", b"OPCODE_ALLOCATED", 1)
        with self.assertRaises(conquest.LedgerError):
            conquest.validate_csv_bytes(self.document, tampered)

    def test_validator_rejects_class_specific_candidate_identity(self) -> None:
        document = copy.deepcopy(self.document)
        document["programs"][0]["candidateIdentity"]["effectAssetId"] = (
            "effect.artist.skill.31470"
        )
        with self.assertRaises(conquest.LedgerError):
            conquest.validate_ledger(document)

    def test_validator_rejects_literal_opcode_invention(self) -> None:
        document = copy.deepcopy(self.document)
        row = next(
            value
            for value in document["programs"]
            if value["sourceKind"] == "LITERAL_DXBC_PROGRAM"
        )
        row["equation"]["opcode"] = 1000
        row["equation"]["opcodeAllocation"] = "ALLOCATED"
        with self.assertRaises(conquest.LedgerError):
            conquest.validate_ledger(document)

    def test_validator_rejects_unknown_blocker(self) -> None:
        document = copy.deepcopy(self.document)
        document["occurrenceProjections"][0]["blockerCodes"].append(
            "UNKNOWN_FUTURE_BLOCKER"
        )
        document["occurrenceProjections"][0]["blockerCodes"].sort()
        with self.assertRaises(conquest.LedgerError):
            conquest.validate_ledger(document)

    def test_validator_rejects_projection_hash_tamper(self) -> None:
        document = copy.deepcopy(self.document)
        document["summary"]["stableProjectionSha256"] = "0" * 64
        with self.assertRaises(conquest.LedgerError):
            conquest.validate_ledger(document)

    def test_json_decoder_rejects_duplicate_keys_and_non_finite_values(self) -> None:
        with self.assertRaises(conquest.LedgerError):
            conquest._decode_json(b'{"schema":1,"schema":2}', "duplicate.json")
        with self.assertRaises(conquest.LedgerError):
            conquest._decode_json(b'{"value":NaN}', "nan.json")

    def test_write_and_check_are_byte_current_and_fail_closed(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            with mock.patch.object(
                conquest, "build_ledger", return_value=self.document
            ):
                self.assertEqual(conquest.run(root, check=False), 0)
                self.assertEqual(conquest.run(root, check=True), 0)
                csv_path = root / conquest.CSV_OUTPUT_RELATIVE_PATH
                csv_path.write_bytes(csv_path.read_bytes() + b"stale")
                self.assertEqual(conquest.run(root, check=True), 1)

    def test_second_replace_failure_rolls_back_both_outputs(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            json_path = root / "ledger.json"
            csv_path = root / "ledger.csv"
            json_path.write_bytes(b"old-json")
            csv_path.write_bytes(b"old-csv")
            call_count = 0

            def fail_second_replace(staged: Path, output: Path) -> None:
                nonlocal call_count
                call_count += 1
                if call_count == 2:
                    raise OSError("injected second replace failure")
                os.replace(staged, output)

            with mock.patch.object(
                conquest,
                "_replace_staged_output",
                side_effect=fail_second_replace,
            ):
                with self.assertRaises(OSError):
                    conquest.write_outputs(self.document, json_path, csv_path)
            self.assertEqual(json_path.read_bytes(), b"old-json")
            self.assertEqual(csv_path.read_bytes(), b"old-csv")
            self.assertEqual(list(root.glob("*.tmp")), [])

    def test_invalid_document_never_replaces_existing_outputs(self) -> None:
        document = copy.deepcopy(self.document)
        document["summary"]["denominatorCounts"]["PRODUCT"] -= 1
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            json_path = root / "ledger.json"
            csv_path = root / "ledger.csv"
            json_path.write_bytes(b"old-json")
            csv_path.write_bytes(b"old-csv")
            with self.assertRaises(conquest.LedgerError):
                conquest.write_outputs(document, json_path, csv_path)
            self.assertEqual(json_path.read_bytes(), b"old-json")
            self.assertEqual(csv_path.read_bytes(), b"old-csv")


if __name__ == "__main__":
    unittest.main()
