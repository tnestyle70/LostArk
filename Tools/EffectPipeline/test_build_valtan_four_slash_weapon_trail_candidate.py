#!/usr/bin/env python3

from __future__ import annotations

import copy
import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = SCRIPT_PATH.parent.parent.parent
BUILDER_PATH = (
    SCRIPT_PATH.parent / "build_valtan_four_slash_weapon_trail_candidate.py"
)


def load_module(name: str, path: Path):
    specification = importlib.util.spec_from_file_location(name, path)
    if specification is None or specification.loader is None:
        raise RuntimeError(f"could not load {path}")
    module = importlib.util.module_from_spec(specification)
    sys.modules[name] = module
    specification.loader.exec_module(module)
    return module


BUILDER = load_module(
    "build_valtan_four_slash_weapon_trail_candidate_for_test",
    BUILDER_PATH,
)


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


class ValtanFourSlashWeaponTrailCandidateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.outputs = BUILDER.build_outputs(REPOSITORY_ROOT)
        cls.output_root = (
            REPOSITORY_ROOT
            / "Data/Effects/Imported/Valtan/ProjectTunedFourSlashWeaponTrail"
        )
        cls.candidate_path = cls.output_root / BUILDER.CANDIDATE_FILENAME
        cls.manifest_path = cls.output_root / BUILDER.MANIFEST_FILENAME
        cls.schema_path = REPOSITORY_ROOT.joinpath(
            *BUILDER.SCHEMA_RELATIVE_PATH.parts
        )
        cls.candidate = read_json(cls.candidate_path)
        cls.manifest = read_json(cls.manifest_path)
        cls.source_path = (
            REPOSITORY_ROOT
            / "Data/Effects/Authored/"
            "effect.valtan.four-slash.active.clip-02.effect.json"
        )
        cls.source = read_json(cls.source_path)
        cls.source_element = next(
            row
            for row in cls.source["elements"]
            if row["id"] == BUILDER.SOURCE_ELEMENT_ID
        )

    def test_generated_files_are_current_and_deterministic(self) -> None:
        BUILDER.check_outputs(REPOSITORY_ROOT, self.outputs)
        second = BUILDER.build_outputs(REPOSITORY_ROOT)
        self.assertEqual(self.outputs, second)
        self.assertEqual(
            set(self.outputs), {self.candidate_path, self.manifest_path}
        )

    def test_candidate_is_one_missing_only_project_bounded_trail(self) -> None:
        self.assertEqual(self.candidate["schema"], "lostark.effect-authoring")
        self.assertEqual(self.candidate["version"], 13)
        self.assertEqual(
            self.candidate["effectAssetId"], BUILDER.SOURCE_EFFECT_ASSET_ID
        )
        self.assertEqual(len(self.candidate["elements"]), 1)
        element = self.candidate["elements"][0]
        self.assertEqual(element["id"], BUILDER.CANDIDATE_ELEMENT_ID)
        self.assertEqual(element["sourceNode"], BUILDER.CANDIDATE_SOURCE_NODE)
        self.assertEqual(element["kind"], "trail")
        self.assertTrue(element["visible"])
        self.assertFalse(element["sourceRecipe"]["enabled"])
        self.assertEqual(element["sourceRecipe"]["modules"], [])
        self.assertEqual(element["sourcePresentation"], {"enabled": False})
        projected = [
            row
            for row in self.source["elements"]
            if row["id"] == BUILDER.CANDIDATE_ELEMENT_ID
            or row["sourceNode"] == BUILDER.CANDIDATE_SOURCE_NODE
        ]
        receipt = self.output_root / (
            "Valtan.four-slash-weapon-trail.application-receipt.v1.json"
        )
        if receipt.is_file():
            self.assertEqual(1, len(projected))
            self.assertEqual(
                BUILDER.canonical_sha256(
                    BUILDER._protected_contract(element)
                ),
                BUILDER.canonical_sha256(
                    BUILDER._protected_contract(projected[0])
                ),
            )
        else:
            self.assertEqual([], projected)

        policy = self.manifest["policy"]
        self.assertEqual(policy["classification"], "PROJECT_TUNED")
        self.assertEqual(policy["fidelity"], "BOUNDED_RECONSTRUCTION")
        self.assertEqual(policy["reconcileMode"], "MISSING_ONLY")
        self.assertTrue(policy["candidateOnly"])
        self.assertFalse(policy["canonicalMutationPerformed"])
        self.assertFalse(policy["cueBindingMutationPerformed"])
        self.assertFalse(policy["catalogMutationPerformed"])
        self.assertFalse(policy["sourceExactClaim"])
        self.assertEqual(policy["drawableProofStatus"], "NOT_ATTEMPTED")

    def test_new_row_reuses_closure_but_keeps_old_row_immobile(self) -> None:
        candidate = self.candidate["elements"][0]
        source = self.source_element
        self.assertEqual(candidate["resources"], source["resources"])
        self.assertEqual(candidate["material"], source["material"])
        self.assertEqual(candidate["detail"]["trail"], source["detail"]["trail"])
        self.assertEqual(
            BUILDER.canonical_sha256(candidate["resources"]),
            BUILDER.SOURCE_RESOURCE_CLOSURE_SHA256,
        )
        self.assertEqual(
            BUILDER.canonical_sha256(candidate["material"]),
            BUILDER.SOURCE_MATERIAL_CLOSURE_SHA256,
        )
        self.assertEqual(
            BUILDER.canonical_sha256(candidate["detail"]["trail"]),
            BUILDER.SOURCE_TRAIL_GEOMETRY_SHA256,
        )

        source_attachment = source["actionCueAttachment"]
        self.assertFalse(source_attachment["enabled"])
        self.assertFalse(source_attachment["follow"])
        self.assertEqual(source_attachment["runtimeAnchorSlotId"], "")
        self.assertEqual(source_attachment["runtimeBoneName"], "")
        self.assertEqual(
            BUILDER.canonical_sha256(source), BUILDER.SOURCE_ELEMENT_SHA256
        )

        expected_timing = copy.deepcopy(source["detail"]["timing"])
        expected_timing["startDelaySeconds"] = (
            BUILDER.SOURCE_OCCURRENCE_TIME_SECONDS
        )
        self.assertEqual(candidate["detail"]["timing"], expected_timing)

    def test_follow_attachment_uses_new_slot_and_official_weapon_bone(self) -> None:
        attachment = self.candidate["elements"][0]["actionCueAttachment"]
        self.assertTrue(attachment["enabled"])
        self.assertTrue(attachment["follow"])
        self.assertEqual(attachment["sourceAnchorSlotId"], "b_wp_r_01")
        self.assertEqual(
            attachment["runtimeAnchorSlotId"],
            BUILDER.RUNTIME_ANCHOR_SLOT_ID,
        )
        self.assertEqual(attachment["runtimeBoneName"], "b_wp_r_01")
        self.assertEqual(
            attachment["socketLocalTransform"],
            {
                "position": [0, 0, 0],
                "rotationDegrees": [0, 0, 0],
                "scale": [1, 1, 1],
            },
        )

        evidence = self.manifest["boundedProjection"]["modelBoneEvidence"]
        self.assertEqual(evidence["runtimeModelAssetId"], BUILDER.BODY_MODEL_ASSET_ID)
        self.assertEqual(evidence["runtimeBoneName"], "b_wp_r_01")
        self.assertEqual(evidence["runtimeAnchorSlotId"], BUILDER.RUNTIME_ANCHOR_SLOT_ID)
        self.assertEqual(evidence["boneIndex"], 54)
        self.assertEqual(evidence["parentBoneIndex"], 41)
        self.assertEqual(evidence["boneNameHash"], "51bf337724f3b4ca")
        self.assertEqual(evidence["matchPolicy"], "EXACT_CASE_SENSITIVE")
        self.assertEqual(evidence["modelSha256"], BUILDER.BODY_MODEL_SHA256)
        self.assertEqual(
            evidence["admission"], "ADMITTED_EXPLICIT_RUNTIME_BONE"
        )

    def test_manifest_is_sealed_and_refuses_drawable_claim(self) -> None:
        manifest = copy.deepcopy(self.manifest)
        seal = manifest.pop("artifactSha256")
        self.assertEqual(seal, BUILDER.canonical_sha256(manifest))
        proof = self.manifest["requiredDrawableHarness"]
        self.assertEqual(
            proof["status"], "MISSING_STATIONARY_ROOT_MOVING_BONE_FIXTURE"
        )
        self.assertEqual(
            proof["rootWorldPolicy"],
            "IDENTITY_AND_STATIONARY_FOR_ALL_FIXED_STEPS",
        )
        self.assertIn(
            "candidate Trail produces at least two distinct world points",
            proof["positiveAssertions"],
        )
        self.assertIn(
            "missing runtime anchor slot fails closed without root fallback",
            proof["negativeControls"],
        )
        self.assertEqual(
            proof["proofAdmission"],
            "DO_NOT_MARK_DRAWABLE_BEFORE_FIXTURE_PASSES",
        )

    def test_manifest_schema_is_valid_json_and_matches_when_available(self) -> None:
        schema = read_json(self.schema_path)
        self.assertEqual(
            schema["$id"],
            "https://lostark.local/schemas/"
            "lostark.valtan-four-slash-weapon-trail-candidate.schema.json",
        )
        self.assertEqual(
            self.manifest["generator"]["schemaPath"],
            BUILDER.SCHEMA_RELATIVE_PATH.as_posix(),
        )
        try:
            import jsonschema
        except ImportError:
            return
        jsonschema.Draft202012Validator.check_schema(schema)
        jsonschema.validate(self.manifest, schema)

    def test_builder_write_set_cannot_touch_product_contracts(self) -> None:
        forbidden = [
            REPOSITORY_ROOT / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
            REPOSITORY_ROOT
            / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json",
            self.source_path,
            REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json",
        ]
        before = {path: path.read_bytes() for path in forbidden}
        BUILDER._validate_write_set(REPOSITORY_ROOT, self.outputs)
        BUILDER.write_outputs(REPOSITORY_ROOT, self.outputs)
        after = {path: path.read_bytes() for path in forbidden}
        self.assertEqual(before, after)
        with self.assertRaises(BUILDER.CandidateError):
            BUILDER._validate_write_set(
                REPOSITORY_ROOT,
                {self.source_path: b"forbidden"},
            )
        self.assertNotIn("--apply", BUILDER_PATH.read_text(encoding="utf-8"))

    def test_missing_only_identity_collision_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            authored = root / "Data/Effects/Authored"
            authored.mkdir(parents=True)
            collision = {
                "elements": [
                    {
                        "id": BUILDER.CANDIDATE_ELEMENT_ID,
                        "sourceNode": "different",
                        "actionCueAttachment": {"runtimeAnchorSlotId": ""},
                    }
                ]
            }
            (authored / "effect.valtan.fixture.effect.json").write_text(
                json.dumps(collision), encoding="utf-8"
            )
            with self.assertRaisesRegex(
                BUILDER.CandidateError, "identity already exists"
            ):
                BUILDER._assert_missing_only_identity(root)

    def test_source_tamper_requires_review_instead_of_overwrite(self) -> None:
        tampered = copy.deepcopy(self.source)
        row = next(
            element
            for element in tampered["elements"]
            if element["id"] == BUILDER.SOURCE_ELEMENT_ID
        )
        row["actionCueAttachment"]["enabled"] = True
        with self.assertRaisesRegex(
            BUILDER.CandidateError, "bounded projection review is required"
        ):
            BUILDER._source_element(tampered)


if __name__ == "__main__":
    unittest.main()
