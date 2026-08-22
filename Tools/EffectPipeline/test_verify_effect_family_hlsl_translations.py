#!/usr/bin/env python3
"""Negative and positive gates for checked Effect-family HLSL artifacts."""

from __future__ import annotations

import copy
import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from Tools.EffectPipeline import verify_effect_family_hlsl_translations as verifier


def raw_sha256(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def attach_artifact(document: dict) -> dict:
    result = copy.deepcopy(document)
    result.pop("artifactSha256", None)
    result["artifactSha256"] = verifier.canonical_sha256(result)
    return result


def write_json_lf(path: Path, document) -> None:
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(json.dumps(document, indent=2, ensure_ascii=False) + "\n")


class TranslationFixture:
    def __init__(self, root: Path) -> None:
        self.root = root
        self.receipt_path = root / "cooked-receipt.json"
        self.report_path = root / "translation-report.json"
        self.cooked_directory = root / "CookedShaders"
        self.translated_directory = root / "TranslatedShaders"
        self.cooked_directory.mkdir()
        self.translated_directory.mkdir()
        self.rows: list[dict] = []
        self.families: list[dict] = []

        for index in range(verifier.EXPECTED_UNIQUE_EXTRACTED_DIGEST_COUNT):
            dxbc = b"DXBC" + index.to_bytes(4, "little")
            digest = raw_sha256(dxbc)
            function_name = f"Shade_Test_{index:03d}"
            hlsl = (
                f"float4 {function_name}()\n"
                "{\n"
                "    return (float4)0;\n"
                "}\n"
            ).encode("utf-8")
            (self.cooked_directory / f"{digest}.dxbc").write_bytes(dxbc)
            (self.translated_directory / f"{function_name}.hlsli").write_bytes(hlsl)
            self.families.append(
                {
                    "parentMaterialPath": f"fx.test.family_{index:03d}",
                    "status": verifier.EXTRACTED,
                    "dxbcSha256": digest,
                    "dxbcByteSize": len(dxbc),
                }
            )
            self.rows.append(
                {
                    "dxbc": f"{digest}.dxbc",
                    "status": verifier.TRANSLATED,
                    "functionName": function_name,
                    "dxbcSha256": digest,
                    "hlslSha256": raw_sha256(hlsl),
                }
            )
        self.write_receipt()
        self.write_report()

    def receipt(self) -> dict:
        return {
            "schema": verifier.RECEIPT_SCHEMA,
            "formatVersion": verifier.FORMAT_VERSION,
            "summary": {
                "familyCount": len(self.families),
                "extractedCount": len(self.families),
                "blockedCount": 0,
            },
            "families": copy.deepcopy(self.families),
        }

    def write_receipt(self, document: dict | None = None) -> None:
        write_json_lf(
            self.receipt_path,
            attach_artifact(document if document is not None else self.receipt()),
        )

    def write_report(self) -> None:
        write_json_lf(self.report_path, self.rows)

    def verify(self) -> dict[str, int]:
        return verifier.verify(
            self.receipt_path,
            self.report_path,
            self.cooked_directory,
            self.translated_directory,
        )


class EffectFamilyHlslTranslationVerifierTests(unittest.TestCase):
    def use_fixture(self):
        temporary = tempfile.TemporaryDirectory(prefix="effect-hlsl-verify-")
        self.addCleanup(temporary.cleanup)
        return TranslationFixture(Path(temporary.name))

    def test_accepts_exact_translation_artifact_set(self) -> None:
        fixture = self.use_fixture()
        result = fixture.verify()
        self.assertEqual(
            result["translatedProgramCount"],
            verifier.EXPECTED_UNIQUE_EXTRACTED_DIGEST_COUNT,
        )

    def test_rejects_receipt_schema_even_with_fresh_artifact_hash(self) -> None:
        fixture = self.use_fixture()
        receipt = fixture.receipt()
        receipt["schema"] = "lostark.wrong-schema"
        fixture.write_receipt(receipt)
        with self.assertRaisesRegex(verifier.VerificationError, "schema"):
            fixture.verify()

    def test_rejects_receipt_artifact_hash_drift(self) -> None:
        fixture = self.use_fixture()
        receipt = json.loads(fixture.receipt_path.read_text(encoding="utf-8"))
        receipt["summary"]["familyCount"] += 1
        write_json_lf(fixture.receipt_path, receipt)
        with self.assertRaisesRegex(verifier.VerificationError, "artifactSha256 drifted"):
            fixture.verify()

    def test_rejects_non_169_unique_extracted_denominator(self) -> None:
        fixture = self.use_fixture()
        receipt = fixture.receipt()
        receipt["families"].pop()
        receipt["summary"]["familyCount"] -= 1
        receipt["summary"]["extractedCount"] -= 1
        fixture.write_receipt(receipt)
        with self.assertRaisesRegex(verifier.VerificationError, "denominator must be 169"):
            fixture.verify()

    def test_rejects_duplicate_translation_digest(self) -> None:
        fixture = self.use_fixture()
        fixture.rows.append(copy.deepcopy(fixture.rows[0]))
        duplicate_function = fixture.rows[-1]["functionName"]
        fixture.rows[-1]["functionName"] = duplicate_function + "_DuplicateRow"
        fixture.write_report()
        with self.assertRaisesRegex(verifier.VerificationError, "exactly one row"):
            fixture.verify()

    def test_rejects_missing_translation_digest(self) -> None:
        fixture = self.use_fixture()
        fixture.rows.pop()
        fixture.write_report()
        with self.assertRaisesRegex(verifier.VerificationError, "missing=1 extra=0"):
            fixture.verify()

    def test_rejects_extra_translation_digest(self) -> None:
        fixture = self.use_fixture()
        dxbc = b"DXBC-extra"
        digest = raw_sha256(dxbc)
        fixture.rows.append(
            {
                "dxbc": f"{digest}.dxbc",
                "status": verifier.TRANSLATED,
                "functionName": "Shade_Extra",
                "dxbcSha256": digest,
                "hlslSha256": "0" * 64,
            }
        )
        fixture.write_report()
        with self.assertRaisesRegex(verifier.VerificationError, "missing=0 extra=1"):
            fixture.verify()

    def test_rejects_failed_translation_row(self) -> None:
        fixture = self.use_fixture()
        fixture.rows[0]["status"] = "FAILED"
        fixture.write_report()
        with self.assertRaisesRegex(verifier.VerificationError, "not TRANSLATED"):
            fixture.verify()

    def test_rejects_duplicate_function_name(self) -> None:
        fixture = self.use_fixture()
        fixture.rows[1]["functionName"] = fixture.rows[0]["functionName"]
        fixture.write_report()
        with self.assertRaisesRegex(verifier.VerificationError, "duplicates functionName"):
            fixture.verify()

    def test_rejects_dxbc_filename_not_derived_from_digest(self) -> None:
        fixture = self.use_fixture()
        fixture.rows[0]["dxbc"] = "not-the-digest.dxbc"
        fixture.write_report()
        with self.assertRaisesRegex(verifier.VerificationError, "DXBC filename"):
            fixture.verify()

    def test_rejects_dxbc_raw_hash_drift(self) -> None:
        fixture = self.use_fixture()
        digest = fixture.rows[0]["dxbcSha256"]
        (fixture.cooked_directory / f"{digest}.dxbc").write_bytes(b"DXBC-drift")
        with self.assertRaisesRegex(verifier.VerificationError, "DXBC artifact raw SHA-256"):
            fixture.verify()

    def test_rejects_missing_hlsli_file(self) -> None:
        fixture = self.use_fixture()
        filename = fixture.rows[0]["functionName"] + ".hlsli"
        (fixture.translated_directory / filename).unlink()
        with self.assertRaisesRegex(verifier.VerificationError, "missing=1 extra=0"):
            fixture.verify()

    def test_rejects_extra_hlsli_file(self) -> None:
        fixture = self.use_fixture()
        (fixture.translated_directory / "Shade_Unreported.hlsli").write_text(
            "float4 Shade_Unreported() {}\n", encoding="utf-8", newline="\n"
        )
        with self.assertRaisesRegex(verifier.VerificationError, "missing=0 extra=1"):
            fixture.verify()

    def test_rejects_hlsli_raw_hash_drift(self) -> None:
        fixture = self.use_fixture()
        filename = fixture.rows[0]["functionName"] + ".hlsli"
        path = fixture.translated_directory / filename
        path.write_bytes(path.read_bytes() + b"// drift\n")
        with self.assertRaisesRegex(verifier.VerificationError, "HLSLI artifact raw SHA-256"):
            fixture.verify()

    def test_rejects_crlf_even_when_report_hash_matches(self) -> None:
        fixture = self.use_fixture()
        filename = fixture.rows[0]["functionName"] + ".hlsli"
        path = fixture.translated_directory / filename
        payload = path.read_bytes().replace(b"\n", b"\r\n")
        path.write_bytes(payload)
        fixture.rows[0]["hlslSha256"] = raw_sha256(payload)
        fixture.write_report()
        with self.assertRaisesRegex(verifier.VerificationError, "not LF-only"):
            fixture.verify()

    def test_rejects_missing_declared_function_even_when_hash_matches(self) -> None:
        fixture = self.use_fixture()
        row = fixture.rows[0]
        filename = row["functionName"] + ".hlsli"
        path = fixture.translated_directory / filename
        payload = b"float4 Shade_Different()\n{\n    return (float4)0;\n}\n"
        path.write_bytes(payload)
        row["hlslSha256"] = raw_sha256(payload)
        fixture.write_report()
        with self.assertRaisesRegex(verifier.VerificationError, "does not declare"):
            fixture.verify()

    def test_rejects_non_array_translation_report(self) -> None:
        fixture = self.use_fixture()
        write_json_lf(fixture.report_path, {"rows": fixture.rows})
        with self.assertRaisesRegex(verifier.VerificationError, "root must be an array"):
            fixture.verify()


if __name__ == "__main__":
    unittest.main()
