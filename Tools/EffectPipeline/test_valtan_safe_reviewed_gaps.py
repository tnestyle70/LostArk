#!/usr/bin/env python3
"""Focused contract tests for the proof-gated Valtan safe-gap closure."""

from __future__ import annotations

from copy import deepcopy
import json
from pathlib import Path
import tempfile
import unittest

import apply_valtan_safe_reviewed_gaps as applicator
import build_valtan_safe_reviewed_gap_candidates as candidates
import build_valtan_safe_reviewed_gap_drawable_proof as drawable_proof


ROOT = Path(__file__).resolve().parents[2]
SCHEMA_ROOT = Path(__file__).parent / "Schemas"


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


class ValtanSafeReviewedGapTests(unittest.TestCase):
    def setUp(self) -> None:
        self.manifest = read_json(candidates.MANIFEST_PATH)
        self.proof = read_json(drawable_proof.PROOF_PATH)
        self.receipt = read_json(applicator.RECEIPT_PATH)
        self.boss_catalog = read_json(applicator.BOSS_CATALOG_PATH)

    def _copy_four_slash_fixture(self, temporary_root: Path) -> None:
        downstream_manifest = read_json(
            ROOT
            / applicator.FOUR_SLASH_MANIFEST_RELATIVE_PATH.as_posix()
        )
        paths = (
            applicator.FOUR_SLASH_MANIFEST_RELATIVE_PATH,
            applicator.FOUR_SLASH_PROOF_RELATIVE_PATH,
            applicator.FOUR_SLASH_RECEIPT_RELATIVE_PATH,
            applicator.four_slash_applicator.relative_from_text(
                downstream_manifest["target"]["candidatePath"],
                "candidatePath",
            ),
            applicator.four_slash_applicator.relative_from_text(
                downstream_manifest["target"]["canonicalPath"],
                "canonicalPath",
            ),
        )
        for relative in paths:
            source = ROOT.joinpath(*relative.parts)
            target = temporary_root.joinpath(*relative.parts)
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(source.read_bytes())

    def test_candidate_manifest_is_exact_and_tamper_evident(self) -> None:
        candidates.validate_manifest(self.manifest)
        by_effect = {
            row["effectAssetId"]: (
                row["elementCount"],
                row["coreProjectionCount"],
                row["trailProjectionCount"],
            )
            for row in self.manifest["candidateDocuments"]
        }
        self.assertEqual(
            {
                "effect.valtan.backstep.windup.trails": (3, 0, 3),
                "effect.valtan.four-slash.active.clip-02": (20, 19, 1),
                "effect.valtan.jump-spin.spin.trails": (3, 0, 3),
                "effect.valtan.swing.active.clip-02": (141, 141, 0),
            },
            by_effect,
        )
        self.assertEqual(160, len(self.manifest["coreProjections"]))
        self.assertEqual(7, len(self.manifest["adapterProjections"]))
        self.assertEqual(6, len(self.manifest["preservedUnresolvedTrailRows"]))

        tampered = deepcopy(self.manifest)
        tampered["summary"]["elementCount"] = 166
        with self.assertRaises(candidates.SafeGapError):
            candidates.validate_manifest(tampered)

    def test_drawable_proof_rebuilds_exactly_and_rejects_denominator_drift(self) -> None:
        drawable_proof.validate_proof(self.proof)
        rebuilt = drawable_proof.build()
        self.assertEqual(self.proof, rebuilt)
        self.assertEqual(167, self.proof["summary"]["preparedElementCount"])
        self.assertEqual(167, self.proof["summary"]["drawnElementCount"])
        self.assertEqual(0, self.proof["summary"]["failedDrawCount"])

        tampered = deepcopy(self.proof)
        tampered["summary"]["drawnElementCount"] = 166
        drawable_proof.seal(tampered, "artifactSha256")
        with self.assertRaises(drawable_proof.ProofError):
            drawable_proof.validate_proof(tampered)

    def test_applied_state_is_exact_byte_idempotent_and_tamper_evident(self) -> None:
        state, writes, rebuilt_receipt = applicator.expected_application()
        self.assertEqual("APPLIED_EXACT", state)
        self.assertEqual(self.receipt, rebuilt_receipt)
        self.assertTrue(writes)
        for path, payload in writes.items():
            self.assertTrue(path.is_file(), path)
            self.assertEqual(payload, path.read_bytes(), path)
        self.assertNotIn(
            ROOT
            / "Data/Effects/Authored/"
            "effect.valtan.four-slash.active.clip-02.effect.json",
            writes,
        )

        tampered = deepcopy(self.receipt)
        tampered["summary"]["addedElementCount"] = 166
        applicator.seal(tampered, "artifactSha256")
        with self.assertRaises(applicator.ApplyError):
            applicator.validate_receipt(tampered)

        ownership_tampered = deepcopy(self.receipt)
        ownership_tampered["combatObjectOwnershipTransfer"][
            "combatObjectVisuals"
        ][0]["effectAssetId"] = "effect.valtan.invalid"
        applicator.seal(ownership_tampered, "artifactSha256")
        with self.assertRaises(applicator.ApplyError):
            applicator.validate_receipt(ownership_tampered)

    def test_canonical_cues_catalog_and_documents_close_only_four_slices(self) -> None:
        cues = read_json(candidates.CUES_PATH)
        catalog = read_json(candidates.CATALOG_PATH)
        cue_index = {row["bindingId"]: row for row in cues["cues"]}
        catalog_index = {row["effectAssetId"]: row for row in catalog["effects"]}
        self.assertEqual(106, len(cues["cues"]))
        self.assertEqual(315, len(catalog["effects"]))

        for row in self.manifest["candidateDocuments"]:
            cue = row["cue"]
            catalog_row = row["catalogRow"]
            self.assertEqual(cue, cue_index[cue["bindingId"]])
            self.assertEqual(catalog_row, catalog_index[row["effectAssetId"]])
            self.assertEqual("once", cue["repeatPolicy"])
            self.assertEqual(0, cue["sourceStartMs"])
            self.assertIsNone(cue["sourceEndMs"])
            canonical_path = ROOT / row["canonicalPath"]
            canonical = read_json(canonical_path)
            self.assertEqual(row["effectAssetId"], canonical["effectAssetId"])
            if row["effectAssetId"] == applicator.FOUR_SLASH_EFFECT_ASSET_ID:
                downstream = applicator._validate_four_slash_composition(
                    self.manifest
                )
                self.assertIsNotNone(downstream)
                projection = downstream["projection"]
                source_view = deepcopy(canonical)
                projected = source_view["elements"].pop(
                    projection["insertedElementIndex"]
                )
                self.assertEqual(projection["elementValue"], projected)
                self.assertEqual(row["elementCount"], len(source_view["elements"]))
                self.assertEqual(
                    row["elementIds"],
                    [element["id"] for element in source_view["elements"]],
                )
                self.assertEqual(
                    row["canonicalSha256"],
                    candidates.canonical_sha256(source_view),
                )
                self.assertEqual(21, len(canonical["elements"]))
            else:
                self.assertEqual(row["elementCount"], len(canonical["elements"]))
                self.assertEqual(
                    row["elementIds"],
                    [element["id"] for element in canonical["elements"]],
                )
                self.assertEqual(
                    row["canonicalSha256"],
                    candidates.canonical_sha256(canonical),
                )

    def test_four_slash_downstream_receipts_and_exact_row_are_sealed(self) -> None:
        downstream = applicator._validate_four_slash_composition(self.manifest)
        self.assertIsNotNone(downstream)
        self.assertEqual(
            "APPLIED_PROOF_GATED_EXACT_EXTENSION", downstream["status"]
        )
        self.assertEqual(20, downstream["safeSourceView"]["elementCount"])
        self.assertEqual(21, downstream["canonicalOutput"]["elementCount"])
        self.assertEqual(20, downstream["projection"]["insertedElementIndex"])
        self.assertEqual(
            downstream["candidateDocument"]["elementSha256"],
            downstream["projection"]["elementSha256"],
        )
        self.assertEqual(
            downstream,
            self.receipt["downstreamFourSlashWeaponTrail"],
        )

    def test_four_slash_downstream_fails_closed_on_missing_or_tampered_receipt(
        self,
    ) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self._copy_four_slash_fixture(root)
            root.joinpath(
                *applicator.FOUR_SLASH_RECEIPT_RELATIVE_PATH.parts
            ).unlink()
            with self.assertRaises(applicator.ApplyError):
                applicator._validate_four_slash_composition(self.manifest, root)

        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self._copy_four_slash_fixture(root)
            receipt_path = root.joinpath(
                *applicator.FOUR_SLASH_RECEIPT_RELATIVE_PATH.parts
            )
            receipt = read_json(receipt_path)
            receipt["canonicalApply"]["postApplyRawSha256"] = "0" * 64
            applicator.four_slash_proof.seal(receipt, "artifactSha256")
            receipt_path.write_bytes(
                applicator.four_slash_applicator.pretty_bytes(receipt)
            )
            with self.assertRaises(applicator.ApplyError):
                applicator._validate_four_slash_composition(self.manifest, root)

    def test_four_slash_downstream_fails_closed_on_rebind_or_unrelated_drift(
        self,
    ) -> None:
        canonical_relative = applicator.four_slash_applicator.relative_from_text(
            applicator._four_slash_safe_candidate(self.manifest)[
                "canonicalPath"
            ],
            "canonicalPath",
        )
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self._copy_four_slash_fixture(root)
            canonical_path = root.joinpath(*canonical_relative.parts)
            payload = canonical_path.read_bytes()
            canonical = json.loads(payload.decode("utf-8"))
            canonical["elements"][-1]["sourceNode"] = "project-authored:rebound"
            canonical_path.write_bytes(
                applicator.four_slash_applicator.json_bytes_like(
                    payload, canonical
                )
            )
            with self.assertRaises(applicator.ApplyError):
                applicator._validate_four_slash_composition(self.manifest, root)

        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            self._copy_four_slash_fixture(root)
            canonical_path = root.joinpath(*canonical_relative.parts)
            payload = canonical_path.read_bytes()
            canonical = json.loads(payload.decode("utf-8"))
            canonical["elements"][1]["displayName"] += " drift"
            canonical_path.write_bytes(
                applicator.four_slash_applicator.json_bytes_like(
                    payload, canonical
                )
            )
            with self.assertRaises(applicator.ApplyError):
                applicator._validate_four_slash_composition(self.manifest, root)

    def test_applied_state_seals_exact_combat_object_ownership_transfer(
        self,
    ) -> None:
        cues = read_json(candidates.CUES_PATH)
        catalog = read_json(candidates.CATALOG_PATH)
        self.assertEqual(
            "APPLIED_EXACT",
            applicator._state(
                self.manifest, cues, catalog, self.boss_catalog
            ),
        )
        retired_bindings = {
            transfer["retiredCue"]["bindingId"]
            for transfer in applicator.COMBAT_OBJECT_OWNERSHIP_TRANSFERS
        }
        retired_actions = {
            transfer["retiredCue"]["actionId"]
            for transfer in applicator.COMBAT_OBJECT_OWNERSHIP_TRANSFERS
        }
        self.assertTrue(
            retired_bindings.isdisjoint(
                row["bindingId"] for row in cues["cues"]
            )
        )
        self.assertTrue(
            retired_actions.isdisjoint(row["actionId"] for row in cues["cues"])
        )
        valtan = next(
            row
            for row in self.boss_catalog["bosses"]
            if row["archetypeId"] == "BOSS_VALTAN"
        )
        self.assertEqual(
            [
                transfer["combatObjectVisual"]
                for transfer in applicator.COMBAT_OBJECT_OWNERSHIP_TRANSFERS
            ],
            valtan["combatObjectVisuals"],
        )

    def test_applied_state_rejects_arbitrary_removal_restore_and_rebind(
        self,
    ) -> None:
        cues = read_json(candidates.CUES_PATH)
        catalog = read_json(candidates.CATALOG_PATH)

        removed = deepcopy(cues)
        removed["cues"].pop(0)
        with self.assertRaises(applicator.ApplyError):
            applicator._state(
                self.manifest, removed, catalog, self.boss_catalog
            )

        restored = deepcopy(cues)
        restored["cues"].append(
            deepcopy(
                applicator.COMBAT_OBJECT_OWNERSHIP_TRANSFERS[0]["retiredCue"]
            )
        )
        with self.assertRaises(applicator.ApplyError):
            applicator._state(
                self.manifest, restored, catalog, self.boss_catalog
            )

        rebound = deepcopy(cues)
        rebound["cues"][0]["bindingId"] = "cue.valtan.test.rebound"
        with self.assertRaises(applicator.ApplyError):
            applicator._state(
                self.manifest, rebound, catalog, self.boss_catalog
            )

        added_catalog = deepcopy(catalog)
        added_catalog["effects"].append(
            {
                "effectAssetId": "effect.valtan.test.unrelated",
                "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
                "authoringPath": (
                    "Effects/Authored/effect.valtan.test.unrelated.effect.json"
                ),
            }
        )
        with self.assertRaises(applicator.ApplyError):
            applicator._state(
                self.manifest, cues, added_catalog, self.boss_catalog
            )

        boss_rebound = deepcopy(self.boss_catalog)
        valtan = next(
            row
            for row in boss_rebound["bosses"]
            if row["archetypeId"] == "BOSS_VALTAN"
        )
        valtan["combatObjectVisuals"][0]["effectAssetId"] = (
            "effect.valtan.invalid"
        )
        with self.assertRaises(applicator.ApplyError):
            applicator._state(self.manifest, cues, catalog, boss_rebound)

    def test_candidate_proof_and_application_artifacts_match_json_schemas(self) -> None:
        try:
            import jsonschema
        except ImportError:
            self.skipTest("jsonschema is not installed")
        fixtures = (
            (
                self.manifest,
                "lostark.valtan-safe-reviewed-gap-candidates.schema.json",
            ),
            (
                self.proof,
                "lostark.valtan-safe-reviewed-gap-drawable-proof.schema.json",
            ),
            (
                self.receipt,
                "lostark.valtan-safe-reviewed-gap-application-receipt.schema.json",
            ),
        )
        for document, schema_name in fixtures:
            with self.subTest(schema=schema_name):
                schema = read_json(SCHEMA_ROOT / schema_name)
                jsonschema.Draft202012Validator.check_schema(schema)
                jsonschema.validate(document, schema)


if __name__ == "__main__":
    unittest.main()
