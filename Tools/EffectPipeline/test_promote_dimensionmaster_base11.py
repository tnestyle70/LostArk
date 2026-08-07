from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from promote_dimensionmaster_base11 import (
    AGGREGATE_EFFECT_IDS,
    AGGREGATE_SKILL_IDS,
    COMBO_STAGE_EFFECT_IDS,
    EXCLUDED_EFFECT_IDS,
    PROMOTED_EFFECT_IDS,
    SCHEMA,
    collect_transaction,
    promote,
    sha256_file,
)


def write_json(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )


def effect_document(effect_id: str, marker: str) -> dict:
    return {
        "schema": "lostark.effect-authoring",
        "version": 12,
        "effectAssetId": effect_id,
        "displayName": marker,
        "modelCues": [],
        "elements": [
            {
                "id": f"element.{marker}",
                "kind": "particle",
            }
        ],
    }


def source_evidence(checkpoint_ids: list[int], marker: str) -> dict:
    parent_path = "fx_m.fx_fixture_parent_tr"
    parent_sha = "a" * 64
    parent_props = "FX_M/Material3/fx_fixture_parent_tr.props.txt"
    reference = f"{parent_path}@sha256:{parent_sha}"
    material_path = "fx_m_mi.fx_mi_fixture_tr"
    return {
        "schema": "lostark.effect-source-material-evidence",
        "formatVersion": 1,
        "characterClass": "DIMENSIONMASTER",
        "captureMethod": f"fixture-{marker}",
        "checkpointSkillIds": checkpoint_ids,
        "parentMaterialEvidence": {
            reference: {
                "parentMaterialPath": parent_path,
                "propsFile": parent_props,
                "propsFileSha256": parent_sha,
                "materialEvidence": {"renderState": {}},
            }
        },
        "materials": {
            material_path: [
                {
                    "material_path": material_path,
                    "propsFile": (
                        "FX_M_MI/MaterialInstanceConstant/"
                        "fx_mi_fixture_tr.props.txt"
                    ),
                    "propsFileSha256": "b" * 64,
                    "materialEvidenceStatus": "SOURCE_MATERIAL_PROPS",
                    "materialEvidencePropsFile": parent_props,
                    "materialEvidencePropsSha256": parent_sha,
                    "materialEvidenceRef": reference,
                }
            ]
        },
    }


class DimensionMasterBase11PromotionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.stage = self.root / "stage"
        self.authored = self.root / "authored"
        self.receipt = self.root / "source.receipt.json"
        self.canonical_source_receipt = (
            self.root / "canonical.source.receipt.json"
        )
        self.promotion_receipt = self.root / "promotion.receipt.json"
        self.source_evidence = self.root / "source.evidence.json"
        self.canonical_evidence = self.root / "canonical.evidence.json"
        rows = []
        for effect_id in PROMOTED_EFFECT_IDS:
            file_name = f"{effect_id}.effect.json"
            write_json(
                self.stage / file_name,
                effect_document(effect_id, "staged"),
            )
            write_json(
                self.authored / file_name,
                effect_document(effect_id, "canonical"),
            )
            rows.append(
                {
                    "effectAssetId": effect_id,
                    "path": str(self.stage / file_name),
                    "elementCount": 1,
                    "modelCueCount": 0,
                    "role": (
                        "AGGREGATE"
                        if effect_id in AGGREGATE_EFFECT_IDS
                        else "COMBO_STAGE"
                    ),
                }
            )
        for effect_id in EXCLUDED_EFFECT_IDS:
            write_json(
                self.authored / f"{effect_id}.effect.json",
                effect_document(effect_id, "excluded"),
            )
        write_json(
            self.receipt,
            {
                "schema": (
                    "lostark.dimensionmaster-base-effect-"
                    "materialization-receipt"
                ),
                "formatVersion": 1,
                "variantContract": "BASE_NO_TIME_OR_SPACE_AXIS",
                "selectedSkillIds": list(AGGREGATE_SKILL_IDS),
                "aggregateSkillCount": len(AGGREGATE_EFFECT_IDS),
                "comboStageDocumentCount": len(COMBO_STAGE_EFFECT_IDS),
                "catalogSynchronizationSkippedForSelection": True,
                "documents": rows,
            },
        )
        write_json(
            self.source_evidence,
            source_evidence(list(AGGREGATE_SKILL_IDS), "source"),
        )
        write_json(
            self.canonical_evidence,
            source_evidence(list(AGGREGATE_SKILL_IDS[:3]), "canonical"),
        )

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def canonical_hashes(self) -> dict[str, str]:
        hashes = {
            effect_id: sha256_file(
                self.authored / f"{effect_id}.effect.json"
            )
            for effect_id in (*PROMOTED_EFFECT_IDS, *EXCLUDED_EFFECT_IDS)
        }
        hashes["source-material-evidence"] = sha256_file(
            self.canonical_evidence
        )
        hashes["source-materialization-receipt"] = (
            sha256_file(self.canonical_source_receipt)
            if self.canonical_source_receipt.is_file() else "MISSING"
        )
        return hashes

    def collect(self) -> dict:
        return collect_transaction(
            self.stage,
            self.authored,
            self.receipt,
            self.canonical_source_receipt,
            self.source_evidence,
            self.canonical_evidence,
            self.promotion_receipt,
        )

    def test_validate_accepts_exact_base11_and_does_not_write(self) -> None:
        before = self.canonical_hashes()
        transaction = self.collect()
        self.assertEqual(len(PROMOTED_EFFECT_IDS), len(transaction["documents"]))
        self.assertEqual(before, self.canonical_hashes())
        self.assertFalse(self.promotion_receipt.exists())

    def test_promote_replaces_only_allowlist_and_records_hashes(self) -> None:
        before = self.canonical_hashes()
        transaction = self.collect()
        result = promote(transaction, self.promotion_receipt)
        after = self.canonical_hashes()
        self.assertEqual(SCHEMA, result["schema"])
        self.assertEqual(len(PROMOTED_EFFECT_IDS), result["promotedEffectCount"])
        self.assertEqual(len(PROMOTED_EFFECT_IDS), result["changedEffectCount"])
        for effect_id in PROMOTED_EFFECT_IDS:
            self.assertNotEqual(before[effect_id], after[effect_id])
            self.assertEqual(
                sha256_file(self.stage / f"{effect_id}.effect.json"),
                after[effect_id],
            )
        for effect_id in EXCLUDED_EFFECT_IDS:
            self.assertEqual(before[effect_id], after[effect_id])
        self.assertNotEqual(
            before["source-material-evidence"],
            after["source-material-evidence"],
        )
        self.assertEqual(
            sha256_file(self.source_evidence),
            after["source-material-evidence"],
        )
        self.assertEqual(
            sha256_file(self.receipt),
            after["source-materialization-receipt"],
        )
        persisted = json.loads(
            self.promotion_receipt.read_text(encoding="utf-8")
        )
        self.assertEqual("COMMITTED", persisted["transactionStatus"])
        self.assertEqual(
            sha256_file(self.source_evidence),
            persisted["sourceMaterialEvidence"]["promotedSha256"],
        )
        self.assertEqual(
            sha256_file(self.receipt),
            persisted["sourceMaterializationReceipt"]["promotedSha256"],
        )
        self.assertTrue(
            persisted["preservedExcludedEffects"][0]["unchanged"]
        )

    def test_rejects_manual_authored_change_after_previous_promotion(self) -> None:
        transaction = self.collect()
        promote(transaction, self.promotion_receipt)
        changed_effect_id = PROMOTED_EFFECT_IDS[0]
        changed_path = self.authored / f"{changed_effect_id}.effect.json"
        changed = json.loads(changed_path.read_text(encoding="utf-8"))
        changed["displayName"] = "Manual Restoration Override"
        write_json(changed_path, changed)
        before = self.canonical_hashes()

        with self.assertRaisesRegex(
            ValueError, "preserve the manual override before promoting"
        ):
            self.collect()

        self.assertEqual(before, self.canonical_hashes())

    def test_failure_after_evidence_restores_documents_evidence_and_receipt(
        self,
    ) -> None:
        before = self.canonical_hashes()
        old_receipt = b'{"status":"previous"}\n'
        self.promotion_receipt.write_bytes(old_receipt)
        transaction = self.collect()
        with self.assertRaisesRegex(RuntimeError, "injected promotion failure"):
            promote(
                transaction,
                self.promotion_receipt,
                failure_after_promote=len(PROMOTED_EFFECT_IDS) + 2,
            )
        self.assertEqual(before, self.canonical_hashes())
        self.assertEqual(old_receipt, self.promotion_receipt.read_bytes())
        self.assertEqual(
            [],
            list(
                self.authored.glob(
                    ".dimensionmaster-base11-promotion-*"
                )
            ),
        )

    def test_failure_after_receipt_restores_whole_transaction(self) -> None:
        before = self.canonical_hashes()
        old_receipt = b'{"status":"previous"}\n'
        self.promotion_receipt.write_bytes(old_receipt)
        transaction = self.collect()
        with self.assertRaisesRegex(RuntimeError, "injected promotion failure"):
            promote(
                transaction,
                self.promotion_receipt,
                failure_after_promote=len(PROMOTED_EFFECT_IDS) + 3,
            )
        self.assertEqual(before, self.canonical_hashes())
        self.assertEqual(old_receipt, self.promotion_receipt.read_bytes())

    def test_rejects_extra_alt_v_stage_without_touching_canonical(self) -> None:
        before = self.canonical_hashes()
        extra = self.stage / (
            f"{EXCLUDED_EFFECT_IDS[0]}.effect.json"
        )
        write_json(extra, effect_document(EXCLUDED_EFFECT_IDS[0], "extra"))
        with self.assertRaisesRegex(ValueError, "file set mismatch"):
            self.collect()
        self.assertEqual(before, self.canonical_hashes())

    def test_rejects_receipt_identity_mismatch_before_commit(self) -> None:
        before = self.canonical_hashes()
        receipt = json.loads(self.receipt.read_text(encoding="utf-8"))
        receipt["documents"][0]["effectAssetId"] = "effect.bad"
        write_json(self.receipt, receipt)
        with self.assertRaisesRegex(ValueError, "document set mismatch"):
            self.collect()
        self.assertEqual(before, self.canonical_hashes())

    def test_rejects_receipt_path_overlapping_promoted_document(self) -> None:
        before = self.canonical_hashes()
        transaction = self.collect()
        destination = self.authored / (
            f"{PROMOTED_EFFECT_IDS[0]}.effect.json"
        )
        with self.assertRaisesRegex(ValueError, "protected input"):
            promote(transaction, destination)
        self.assertEqual(before, self.canonical_hashes())

    def test_rejects_non_base11_or_absolute_path_evidence(self) -> None:
        before = self.canonical_hashes()
        evidence = json.loads(
            self.source_evidence.read_text(encoding="utf-8")
        )
        evidence["checkpointSkillIds"] = list(AGGREGATE_SKILL_IDS[:3])
        write_json(self.source_evidence, evidence)
        with self.assertRaisesRegex(ValueError, "checkpointSkillIds"):
            self.collect()
        self.assertEqual(before, self.canonical_hashes())

        evidence = source_evidence(list(AGGREGATE_SKILL_IDS), "source")
        evidence["captureMethod"] = r"C:\Users\user\AppData\Local\Temp\capture"
        write_json(self.source_evidence, evidence)
        with self.assertRaisesRegex(ValueError, "absolute path"):
            self.collect()
        self.assertEqual(before, self.canonical_hashes())


if __name__ == "__main__":
    unittest.main()
