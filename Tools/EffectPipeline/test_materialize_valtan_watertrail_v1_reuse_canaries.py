#!/usr/bin/env python3
"""WATERTRAIL witnesses under the Carrier V1 Product successor."""

from __future__ import annotations

import copy
import json
from pathlib import Path
import sys
import unittest

TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import materialize_valtan_watertrail_v1_reuse_canaries as subject


class ValtanWaterTrailV1ReuseWitnessTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.outputs = subject.build_outputs()
        cls.ledger = json.loads(cls.outputs[subject.LEDGER_PATH].decode("utf-8"))
        cls.catalog = subject._load_json(subject.CATALOG_PATH)
        cls.cues = subject._load_json(subject.VALTAN_CUE_PATH)
        cls.fbf_shell = subject._load_json(subject.FBF_OUTPUT_PATH)
        cls.four_shell = subject._load_json(subject.FOUR_SLASH_OUTPUT_PATH)

    def test_only_ledger_is_an_output(self) -> None:
        self.assertEqual({subject.LEDGER_PATH}, set(self.outputs))
        self.assertNotIn(subject.FBF_OUTPUT_PATH, self.outputs)
        self.assertNotIn(subject.FOUR_SLASH_OUTPUT_PATH, self.outputs)
        self.assertNotIn(subject.CATALOG_PATH, self.outputs)
        self.assertNotIn(subject.VALTAN_CUE_PATH, self.outputs)
        self.assertNotIn(subject.MIGRATION_RECEIPT_PATH, self.outputs)

    def test_two_authored_documents_remain_empty_evidence_shells(self) -> None:
        for path, effect_id, document in (
            (subject.FBF_OUTPUT_PATH, subject.FBF_EFFECT_ID, self.fbf_shell),
            (
                subject.FOUR_SLASH_OUTPUT_PATH,
                subject.FOUR_SLASH_EFFECT_ID,
                self.four_shell,
            ),
        ):
            self.assertEqual([], document["elements"], path)
            self.assertEqual(effect_id, document["effectAssetId"], path)
            identity = subject._validate_empty_evidence_shell(path, effect_id)
            self.assertEqual(0, identity["elementCount"])
        summary = self.ledger["summary"]
        self.assertEqual(0, summary["auditionDocumentCount"])
        self.assertEqual(2, summary["emptyEvidenceShellCount"])
        self.assertEqual(2, summary["historicalWitnessElementCount"])

    def test_ledger_preserves_the_shared_typed_family_evidence(self) -> None:
        family = self.ledger["sharedFamily"]
        self.assertEqual(
            subject.CHILD_MATERIAL, family["childMaterialPath"]
        )
        self.assertEqual(subject.PARENT_MATERIAL, family["parentMaterialPath"])
        self.assertEqual(subject.RUNTIME_PROFILE_ID, family["runtimeShaderProfileId"])
        self.assertEqual(subject.PROFILE_ID, family["profileId"])
        self.assertEqual(subject.MESH_ASSET_ID, family["meshAssetId"])
        self.assertEqual(subject.MODEL_PRE_SCALE, family["modelPreScale"])
        for canary in self.ledger["canaries"]:
            self.assertEqual(
                "ELEMENTS_CLEARED_EVIDENCE_SHELL",
                canary["physicalDocumentDisposition"],
            )
            self.assertRegex(
                canary["historicalCandidateElementSha256"], r"^[0-9a-f]{64}$"
            )
            self.assertRegex(
                canary["historicalCandidateDocumentSha256"], r"^[0-9a-f]{64}$"
            )

    def test_historical_sixty_rows_remain_fully_accounted(self) -> None:
        rows = self.ledger["rows"]
        self.assertEqual(60, len(rows))
        self.assertEqual(60, len({row["elementId"] for row in rows}))
        self.assertEqual(60, self.ledger["summary"]["accountedRowCount"])
        self.assertEqual(59, self.ledger["summary"]["blockedRequiredCount"])
        self.assertEqual(48, sum(row["rendererShape"] == "sprite" for row in rows))
        self.assertEqual(10, sum(row["rendererShape"] == "mesh" for row in rows))
        self.assertEqual(2, sum(row["rendererShape"] == "decal" for row in rows))
        self.assertTrue(all(row["carrierKey"] for row in rows))

    def test_standalone_product_admission_is_retired_without_resurrection(self) -> None:
        self.assertFalse(self.ledger["productMutation"])
        self.assertFalse(
            self.ledger["carrierV1Successor"]["standaloneProductAdmission"]
        )
        summary = self.ledger["summary"]
        self.assertEqual(0, summary["productCatalogAddedCount"])
        self.assertEqual(0, summary["productCatalogRetiredCount"])
        self.assertEqual(0, summary["productCueAddedCount"])
        self.assertEqual(0, summary["productCueRetiredCount"])
        self.assertEqual(2, summary["duplicateCueSuppressedCount"])
        self.assertEqual(2, summary["carrierV1SuccessorOwnerCount"])

        effect_ids = {row["effectAssetId"] for row in self.catalog["effects"]}
        cue_ids = {row["bindingId"] for row in self.cues["cues"]}
        self.assertTrue(
            {
                subject.LEGACY_FBF_EFFECT_ID,
                subject.FBF_EFFECT_ID,
                subject.FOUR_SLASH_EFFECT_ID,
            }.isdisjoint(effect_ids)
        )
        self.assertTrue(
            {
                subject.LEGACY_FBF_CUE_ID,
                subject.FBF_CUE_ID,
                subject.FOUR_SLASH_CUE_ID,
            }.isdisjoint(cue_ids)
        )

    def test_carrier_v1_owns_both_exact_clip_occurrences_once(self) -> None:
        successor = self.ledger["carrierV1Successor"]
        self.assertEqual(0, successor["duplicateClipOccurrenceOwnerCount"])
        owners = {row["clipOccurrenceId"]: row for row in successor["owners"]}
        self.assertEqual(
            {
                "valtan.attack.front-back-front.active.clip.01",
                "valtan.attack.four-slash.active.clip.02",
            },
            set(owners),
        )
        self.assertEqual(
            "effect.valtan.carrier-v1.attack.front-back-front.active.clip-01",
            owners["valtan.attack.front-back-front.active.clip.01"]["effectAssetId"],
        )
        self.assertEqual(
            "effect.valtan.carrier-v1.attack.four-slash.active.clip-02",
            owners["valtan.attack.four-slash.active.clip.02"]["effectAssetId"],
        )
        fbf = owners["valtan.attack.front-back-front.active.clip.01"]
        self.assertIsNone(fbf["sourceNode"])
        self.assertEqual(
            "BLOCKED_UNRESOLVED_RUNTIME_ADAPTER_WITNESS",
            fbf["sourceAdmission"],
        )
        four = owners["valtan.attack.four-slash.active.clip.02"]
        self.assertTrue(four["sourceNode"].startswith("valtan.source."))
        self.assertEqual(
            "MATERIALIZED_EXACT_CARRIER_V1", four["sourceAdmission"]
        )
        for row in owners.values():
            self.assertIn("occurrence-key.", row["fullSourceKey"])

    def test_canary_rows_are_explicitly_authoring_only(self) -> None:
        canaries = {row["role"]: row for row in self.ledger["canaries"]}
        for row in canaries.values():
            self.assertIsNone(row["productCueId"])
            self.assertEqual(
                "AUTHORING_ONLY_SUCCESSOR_OWNS_CLIP", row["productAdmission"]
            )

    def test_wrong_parent_and_missing_texture_fail_closed(self) -> None:
        evidence = subject._find_material_evidence(
            subject._load_json(subject.MATERIAL_EVIDENCE_PATH)
        )
        wrong_parent = copy.deepcopy(evidence)
        wrong_parent["parentMaterialPath"] = "wrong.parent"
        with self.assertRaisesRegex(subject.MaterializationError, "parent"):
            subject._find_material_evidence(
                {"materialParameterBindings": [wrong_parent]}
            )

        missing_texture = copy.deepcopy(evidence)
        missing_texture["instanceTextures"] = missing_texture["instanceTextures"][:1]
        with self.assertRaisesRegex(subject.MaterializationError, "closure"):
            subject._build_source_profile(missing_texture)

    def test_candidate_ids_do_not_create_skill_specific_shader_branches(self) -> None:
        search_roots = [
            subject.ROOT / "Client" / "Private",
            subject.ROOT / "Client" / "Public",
            subject.ROOT / "Client" / "Bin" / "ShaderFiles",
        ]
        needles = (subject.FBF_EFFECT_ID, subject.FOUR_SLASH_EFFECT_ID)
        hits: list[str] = []
        for root in search_roots:
            for path in root.rglob("*"):
                if path.suffix.casefold() not in {".cpp", ".h", ".hlsl", ".hlsli"}:
                    continue
                try:
                    text = path.read_text(encoding="utf-8", errors="ignore")
                except OSError:
                    continue
                if any(needle in text for needle in needles):
                    hits.append(str(path.relative_to(subject.ROOT)))
        self.assertEqual([], hits)

    def test_repository_outputs_match_after_materialization(self) -> None:
        for relative, expected in self.outputs.items():
            path = subject.ROOT / Path(relative)
            self.assertTrue(path.is_file(), relative)
            self.assertEqual(expected, path.read_bytes(), relative)


if __name__ == "__main__":
    unittest.main()
