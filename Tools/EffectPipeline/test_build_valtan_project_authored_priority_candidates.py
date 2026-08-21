#!/usr/bin/env python3
from __future__ import annotations

from copy import deepcopy
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest


SCRIPT_PATH = Path(__file__).with_name(
    "build_valtan_project_authored_priority_candidates.py"
)
SPEC = importlib.util.spec_from_file_location(
    "valtan_project_authored_priority_candidates", SCRIPT_PATH
)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"cannot load {SCRIPT_PATH}")
MODULE = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = MODULE
SPEC.loader.exec_module(MODULE)


class ValtanProjectAuthoredPriorityCandidateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.repo_root = SCRIPT_PATH.resolve().parents[2]
        cls.resource_root = MODULE._find_resource_root(cls.repo_root, None)

    def build(self, **kwargs):
        return MODULE.build_artifacts(
            self.repo_root,
            resource_root=self.resource_root,
            **kwargs,
        )

    def test_expected_denominator_and_overlay_policy(self) -> None:
        artifacts = self.build()
        receipt = artifacts.receipt
        self.assertEqual(9, len(receipt["targets"]))
        desired = [
            row
            for target in receipt["targets"]
            for row in target["desiredElements"]
        ]
        self.assertEqual(24, len(desired))
        self.assertEqual(
            18,
            sum(row["reconcileAction"] == "APPEND_MISSING" for row in desired),
        )
        self.assertEqual(
            6,
            sum(row["reconcileAction"] == "PRESERVE_EXISTING" for row in desired),
        )
        self.assertEqual(9, len(artifacts.documents))
        self.assertEqual(
            18,
            sum(len(document["elements"]) for document in artifacts.documents.values()),
        )
        self.assertTrue(receipt["policy"]["portalRushExcluded"])
        self.assertNotIn(
            "VALTAN_PORTAL_RUSH",
            {target["patternId"] for target in receipt["targets"]},
        )
        self.assertEqual(
            ["UNRESOLVED_PROJECTILE"] * 3,
            [row["disposition"] for row in receipt["unresolved"]],
        )

        verified_assets = {
            row["assetId"] for row in receipt["resourceVerification"]
        }
        for document in artifacts.documents.values():
            MODULE.validate_candidate_document(document)
            encoded = json.dumps(document, sort_keys=True)
            self.assertNotIn("mesh_att_battle_15_03", encoded)
            self.assertNotIn("VALTAN_PORTAL_RUSH", encoded)
            for element in document["elements"]:
                self.assertTrue(
                    element["sourceNode"].startswith("project-authored:")
                )
                self.assertFalse(element["sourceRecipe"]["enabled"])
                self.assertFalse(element["sourcePresentation"]["enabled"])
                self.assertNotEqual("light", element["kind"])
                for resource in element["resources"]:
                    self.assertIn(resource["assetId"], verified_assets)

    def test_user_priority_resource_roles_stay_project_tuned(self) -> None:
        artifacts = self.build()
        roles = {
            row["requestedRole"]: row
            for row in artifacts.receipt["resourceRoleDispositions"]
        }
        self.assertEqual(7, len(roles))
        self.assertTrue(
            all(
                row["candidateDisposition"] == "PROJECT_TUNED_CANDIDATE"
                for row in roles.values()
            )
        )
        self.assertEqual(
            "EXACT_OTHER_PATTERN",
            roles["DASH_FORWARD_RED_PATH"]["evidenceStatus"],
        )
        self.assertIn(
            "FLOOR_WIPE action 420630",
            roles["DASH_FORWARD_RED_PATH"]["sourceEvidenceContext"],
        )
        self.assertEqual(
            "EXACT_SAME_PATTERN_LIMITED_STAGE",
            roles["MAGIC_DONUT_RING_BOUNDARIES"]["evidenceStatus"],
        )
        self.assertIn(
            "outer-end",
            roles["MAGIC_DONUT_RING_BOUNDARIES"]["sourceEvidenceContext"],
        )
        self.assertEqual(
            "EXACT_SAME_PATTERN_SOURCE_ONLY_OCCURRENCE",
            roles["FLOOR_WIPE_IMPACT_NOISE"]["evidenceStatus"],
        )
        self.assertIn(
            "rather than deferred",
            roles["FLOOR_WIPE_IMPACT_NOISE"]["sourceEvidenceContext"],
        )
        self.assertEqual(
            "NO_TARGET_SOURCE_JOIN",
            roles["FRONT_BACK_FRONT_FINAL_GROUND_IMPACT"]["evidenceStatus"],
        )
        self.assertTrue(
            all(
                tuple(row["handTunableFields"]) == MODULE.HAND_TUNABLE_FIELDS
                for row in roles.values()
            )
        )

        magic = artifacts.documents["effect.valtan.magic-choice.windup"]
        for element in magic["elements"]:
            resources = {
                resource["slotId"]: resource["assetId"]
                for resource in element["resources"]
            }
            self.assertEqual(MODULE.ASSET_RING_002, resources["base"])
            self.assertEqual(MODULE.ASSET_RING_004, resources["mask"])
            self.assertNotEqual([0, 0], element["detail"]["uv"]["speed"])

        floor = artifacts.documents["effect.valtan.floor-wipe-130.first-smash"]
        center = next(
            element
            for element in floor["elements"]
            if element["id"] == "project-floor-six-direction-center-impact"
        )
        self.assertIn(
            {"slotId": "noise", "assetId": MODULE.ASSET_ATYPICAL_028},
            center["resources"],
        )
        front = artifacts.documents["effect.valtan.front-back-front.active"]
        shockwave = next(
            element
            for element in front["elements"]
            if element["id"] == "project-three-hit-down-smash-shockwave"
        )
        self.assertEqual(
            MODULE.ASSET_SHOCKWAVE_02,
            shockwave["resources"][0]["assetId"],
        )

    def test_high_jump_airborne_patch_is_complete_and_presentation_only(self) -> None:
        receipt = self.build().receipt
        patch = receipt["highJumpAirbornePatch"]
        cue = patch["cueRow"]
        self.assertEqual("VALTAN_HIGH_JUMP", cue["patternId"])
        self.assertEqual("AIRBORNE", cue["stageId"])
        self.assertEqual(
            "valtan.attack.high-jump.airborne.clip.01",
            cue["clipOccurrenceId"],
        )
        self.assertEqual("snapshot", cue["followPolicy"])
        self.assertEqual("natural", cue["stopPolicy"])
        self.assertEqual("once", cue["repeatPolicy"])
        self.assertIsNone(cue["sourceEndMs"])
        self.assertTrue(patch["authority"]["presentationOnly"])
        self.assertFalse(patch["authority"]["serverGameplayChange"])
        self.assertEqual(
            "UNRESOLVED_PROJECTILE",
            patch["authority"]["projectileAuthorityStatus"],
        )
        self.assertTrue(
            all(row["candidateAssetId"] is None for row in receipt["unresolved"])
        )

    def test_existing_floor_axis_tuning_is_a_preserved_sentinel(self) -> None:
        effect_id = "effect.valtan.floor-wipe-130.windup"
        canonical_path = (
            self.repo_root
            / "Data/Effects/Authored/effect.valtan.floor-wipe-130.windup.effect.json"
        )
        canonical = MODULE._load_json(canonical_path)
        sentinel = next(
            element
            for element in canonical["elements"]
            if element["id"] == "six-direction-telegraph.axis-000"
        )
        sentinel["detail"]["color"]["multiply"] = [0.314, 0.159, 0.265, 0.358]
        before = deepcopy(canonical)
        artifacts = self.build(canonical_overrides={effect_id: canonical})
        self.assertEqual(before, canonical, "reconcile mutated the supplied document")

        target = next(
            row
            for row in artifacts.receipt["targets"]
            if row["targetEffectAssetId"] == effect_id
        )
        axis = next(
            row
            for row in target["desiredElements"]
            if row["elementId"] == "six-direction-telegraph.axis-000"
        )
        self.assertEqual("PRESERVE_EXISTING", axis["reconcileAction"])
        self.assertRegex(axis["canonicalElementSha256"], r"^[0-9a-f]{64}$")
        overlay_ids = {
            element["id"] for element in artifacts.documents[effect_id]["elements"]
        }
        self.assertNotIn("six-direction-telegraph.axis-000", overlay_ids)
        self.assertIn("project-floor-six-direction-center-guide", overlay_ids)

    def test_stable_id_collision_fails_closed(self) -> None:
        effect_id = "effect.valtan.floor-wipe-130.windup"
        canonical = MODULE._load_json(
            self.repo_root
            / "Data/Effects/Authored/effect.valtan.floor-wipe-130.windup.effect.json"
        )
        sentinel = next(
            element
            for element in canonical["elements"]
            if element["id"] == "six-direction-telegraph.axis-000"
        )
        sentinel["sourceNode"] = (
            "project-authored:valtan.floor-wipe-130.unrelated.axis-000"
        )
        with self.assertRaisesRegex(MODULE.ContractError, "stable element ID collision"):
            self.build(canonical_overrides={effect_id: canonical})

    def test_write_check_is_idempotent_and_check_does_not_mutate(self) -> None:
        artifacts = self.build()
        with tempfile.TemporaryDirectory() as temporary:
            destination = Path(temporary)
            MODULE.write_artifacts(artifacts, destination)
            MODULE.check_artifacts(artifacts, destination)
            first = {
                relative.as_posix(): destination.joinpath(*relative.parts).read_bytes()
                for relative in artifacts.files
            }
            MODULE.write_artifacts(artifacts, destination)
            MODULE.check_artifacts(artifacts, destination)
            second = {
                relative.as_posix(): destination.joinpath(*relative.parts).read_bytes()
                for relative in artifacts.files
            }
            self.assertEqual(first, second)
            MODULE.check_artifacts(artifacts, destination)
            third = {
                relative.as_posix(): destination.joinpath(*relative.parts).read_bytes()
                for relative in artifacts.files
            }
            self.assertEqual(second, third)

    def test_missing_runtime_resource_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            with self.assertRaisesRegex(
                MODULE.ContractError, "required candidate resource is missing"
            ):
                MODULE.build_artifacts(
                    self.repo_root,
                    resource_root=Path(temporary),
                )

    def test_receipt_schema_is_strict_and_matches_the_generator_contract(self) -> None:
        schema_path = (
            self.repo_root
            / "Tools/EffectPipeline/Schemas/lostark.valtan-project-authored-priority-patch-plan.schema.json"
        )
        schema = MODULE._load_json(schema_path)
        self.assertFalse(schema["additionalProperties"])
        self.assertEqual(
            MODULE.RECEIPT_SCHEMA,
            schema["properties"]["schema"]["const"],
        )
        self.assertEqual(
            9,
            schema["properties"]["targets"]["minItems"],
        )
        receipt = self.build().receipt
        MODULE.validate_receipt(receipt)
        invalid = deepcopy(receipt)
        invalid["policy"]["sourceClaimPolicy"] = "SOURCE_EXACT"
        with self.assertRaisesRegex(MODULE.ContractError, "policy changed"):
            MODULE.validate_receipt(invalid)


if __name__ == "__main__":
    unittest.main()
