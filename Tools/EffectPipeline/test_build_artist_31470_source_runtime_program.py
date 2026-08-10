from __future__ import annotations

import copy
import hashlib
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / "Tools" / "EffectPipeline"
LEVEL_TOOLS = ROOT / "Tools" / "LevelPlacementExtractor"
for path in (TOOLS, LEVEL_TOOLS):
    if str(path) not in sys.path:
        sys.path.insert(0, str(path))

import build_artist_31470_source_runtime_program as compiler
from effect_source_contract_io import load_strict_json_object


SOURCE = (
    ROOT
    / "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.source-execution-semantics.receipt.json"
)
OUTPUT = (
    ROOT
    / "Data/Effects/Imported/Artist/Candidates/"
    "skill.31470.source-runtime-program.candidate.json"
)


def reseal_program(value: dict) -> None:
    unsigned = copy.deepcopy(value)
    unsigned.pop("programSha256", None)
    value["programSha256"] = compiler.canonical_sha256(unsigned)


class ArtistSourceRuntimeProgramTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source = load_strict_json_object(SOURCE)
        cls.program = compiler.build_program(cls.source, SOURCE)

    def test_denominators_and_fail_closed_admission(self) -> None:
        summary = self.program["summary"]
        self.assertEqual(summary["emitterCount"], 35)
        self.assertEqual(summary["opcodeCount"], 399)
        self.assertEqual(summary["readyOpcodeCount"], 370)
        self.assertEqual(summary["blockedOpcodeCount"], 29)
        self.assertEqual(summary["distributionCount"], 629)
        self.assertEqual(summary["silentFallbackCount"], 0)
        self.assertFalse(self.program["runtimeExecutionAdmission"])
        self.assertFalse(self.program["productAdmission"])
        self.assertFalse(self.program["executionContract"]["executionAdmission"])

    def test_all_source_rows_are_joined_in_exact_order(self) -> None:
        source_emitters, source_modules = compiler.source_index(self.source)
        self.assertEqual(len(source_emitters), 35)
        self.assertEqual(len(source_modules), 399)
        for emitter in self.program["emitters"]:
            expected = [
                row["moduleOccurrenceId"]
                for row in source_emitters[emitter["emitterId"]]["modules"]
            ]
            self.assertEqual(emitter["orderedOpcodeIds"], expected)

    def test_payload_value_coordinated_reseal_is_rejected(self) -> None:
        forged = copy.deepcopy(self.program)
        literal = forged["opcodes"][0]["payload"]["literals"][0]
        literal["value"] = not literal["value"]
        payload = forged["opcodes"][0]["payload"]
        payload["payloadSha256"] = compiler.canonical_sha256(
            compiler.payload_unsigned(payload)
        )
        reseal_program(forged)
        with self.assertRaisesRegex(compiler.ProgramError, "typedPayload drift"):
            compiler.validate_program(forged, self.source)

    def test_emitter_reassignment_is_rejected(self) -> None:
        forged = copy.deepcopy(self.program)
        forged["opcodes"][0]["emitterId"] = forged["emitters"][1]["emitterId"]
        reseal_program(forged)
        with self.assertRaisesRegex(compiler.ProgramError, "emitter reassigned"):
            compiler.validate_program(forged, self.source)

    def test_opcode_and_order_mutations_are_rejected(self) -> None:
        for field, value, reason in (
            ("opcode", "VELOCITY", "semantic mapping"),
            ("order", 99, "order drift"),
        ):
            with self.subTest(field=field):
                forged = copy.deepcopy(self.program)
                forged["opcodes"][0][field] = value
                reseal_program(forged)
                with self.assertRaisesRegex(compiler.ProgramError, reason):
                    compiler.validate_program(forged, self.source)

    def test_blocker_and_handler_receipt_laundering_are_rejected(self) -> None:
        blocked_index = next(
            index
            for index, row in enumerate(self.program["opcodes"])
            if row["decision"] == "BLOCKED"
        )
        forged = copy.deepcopy(self.program)
        forged["opcodes"][blocked_index]["blockers"] = []
        forged["executionContract"] = compiler.execution_contract(
            [compiler.FINAL_OWNER_BLOCKER]
        )
        reseal_program(forged)
        with self.assertRaises(compiler.ProgramError):
            compiler.validate_program(forged, self.source)

        forged = copy.deepcopy(self.program)
        forged["handlerReceipts"][0]["handlerSha256"] = "0" * 64
        reseal_program(forged)
        with self.assertRaisesRegex(compiler.ProgramError, "handler receipts"):
            compiler.validate_program(forged, self.source)

    def test_source_identity_and_summary_laundering_are_rejected(self) -> None:
        for field, value, reason in (
            ("sourceExecutionReceiptSha256", "0" * 64, "source receipt identity"),
            ("summary", {**self.program["summary"], "opcodeCount": 398}, "summary"),
        ):
            with self.subTest(field=field):
                forged = copy.deepcopy(self.program)
                forged[field] = value
                reseal_program(forged)
                with self.assertRaisesRegex(compiler.ProgramError, reason):
                    compiler.validate_program(forged, self.source)

    def test_exact_json_integer_and_duplicate_key_inputs_are_rejected(self) -> None:
        forged = copy.deepcopy(self.program)
        forged["formatVersion"] = 1.0
        reseal_program(forged)
        with self.assertRaisesRegex(compiler.ProgramError, "version"):
            compiler.validate_program(forged, self.source)

        raw = SOURCE.read_text(encoding="utf-8")
        duplicate = raw.replace(
            "{\n  \"schema\"",
            "{\n  \"schema\": \"FORGED\",\n  \"schema\"",
            1,
        )
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "duplicate.json"
            path.write_text(duplicate, encoding="utf-8")
            result = subprocess.run(
                [
                    sys.executable,
                    str(TOOLS / "build_artist_31470_source_runtime_program.py"),
                    "--source-execution-receipt",
                    str(path),
                    "--output",
                    str(Path(directory) / "out.json"),
                ],
                cwd=ROOT,
                capture_output=True,
                text=True,
                check=False,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("duplicate", result.stderr.lower())

    def test_checked_output_and_cli_check_are_exact(self) -> None:
        current = load_strict_json_object(OUTPUT)
        compiler.validate_program(current, self.source)
        self.assertEqual(
            compiler.canonical_bytes(current),
            compiler.canonical_bytes(self.program),
        )
        result = subprocess.run(
            [
                sys.executable,
                str(TOOLS / "build_artist_31470_source_runtime_program.py"),
                "--check",
            ],
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == "__main__":
    unittest.main()
