#!/usr/bin/env python3
"""Contract tests for the isolated MagicChoice SpriteParticle candidates."""

from __future__ import annotations

import copy
import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path


TEST_PATH = Path(__file__).resolve()
REPOSITORY_ROOT = TEST_PATH.parent.parent.parent
BUILDER_PATH = (
    TEST_PATH.parent / "build_valtan_magic_choice_sprite_particle_companions.py"
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
    "build_valtan_magic_choice_sprite_particle_companions_for_test",
    BUILDER_PATH,
)


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8-sig"))


class MagicChoiceSpriteParticleCandidateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.outputs = BUILDER.build_outputs(REPOSITORY_ROOT)
        cls.output_root = REPOSITORY_ROOT.joinpath(
            *BUILDER.OUTPUT_DIRECTORY_RELATIVE_PATH.parts
        )
        cls.manifest_path = cls.output_root / BUILDER.MANIFEST_FILENAME
        cls.manifest = read_json(cls.manifest_path)
        cls.schema_path = REPOSITORY_ROOT.joinpath(
            *BUILDER.SCHEMA_RELATIVE_PATH.parts
        )

        cls.sources = {}
        for spec in BUILDER.SPECS:
            path = REPOSITORY_ROOT.joinpath(*spec.source_document.parts)
            document = read_json(path)
            row = next(
                element
                for element in document["elements"]
                if element["id"] == spec.source_element_id
            )
            cls.sources[spec.candidate_element_id] = row

        cls.candidates = {}
        cls.candidate_documents = {}
        for spec in BUILDER.SPECS:
            path = cls.output_root / spec.candidate_filename
            document = read_json(path)
            cls.candidate_documents[spec.candidate_filename] = document
            row = next(
                element
                for element in document["elements"]
                if element["id"] == spec.candidate_element_id
            )
            cls.candidates[spec.candidate_element_id] = row

    def test_generated_package_is_current_deterministic_and_isolated(self) -> None:
        BUILDER.check_outputs(REPOSITORY_ROOT, self.outputs)
        second = BUILDER.build_outputs(REPOSITORY_ROOT)
        self.assertEqual(self.outputs, second)
        expected = {
            self.output_root / spec.candidate_filename
            for spec in BUILDER.SPECS
        }
        expected.add(self.manifest_path)
        self.assertEqual(set(self.outputs), expected)
        self.assertEqual(len(expected), 4)

    def test_four_new_stable_identities_are_missing_only(self) -> None:
        ids = [spec.candidate_element_id for spec in BUILDER.SPECS]
        nodes = [spec.candidate_source_node for spec in BUILDER.SPECS]
        self.assertEqual(len(ids), len(set(ids)))
        self.assertEqual(len(nodes), len(set(nodes)))
        self.assertEqual(
            ids,
            [
                "project-donut-outer-boundary.sprite-particle-v1",
                "project-donut-inner-growing-boundary.sprite-particle-v1",
                "project-donut-inner-impact.sprite-particle-v1",
                "project-donut-outer-impact.sprite-particle-v1",
            ],
        )
        canonical_ids = set()
        canonical_nodes = set()
        authored_root = REPOSITORY_ROOT / "Data/Effects/Authored"
        for path in authored_root.glob("effect.valtan.*.effect.json"):
            for row in read_json(path).get("elements", []):
                canonical_ids.add(row.get("id"))
                canonical_nodes.add(row.get("sourceNode"))
        self.assertTrue(set(ids).isdisjoint(canonical_ids))
        self.assertTrue(set(nodes).isdisjoint(canonical_nodes))

    def test_source_rows_remain_local_decals_and_are_sha_pinned(self) -> None:
        for spec in BUILDER.SPECS:
            source = self.sources[spec.candidate_element_id]
            self.assertEqual(source["kind"], "decal")
            self.assertEqual(
                BUILDER.canonical_sha256(source), spec.source_element_sha256
            )
            self.assertFalse(source["actionCueAttachment"]["enabled"])
            self.assertFalse(source["actionCueAttachment"]["follow"])
            self.assertFalse(source["sourceRecipe"]["enabled"])
            self.assertEqual(source["sourcePresentation"], {"enabled": False})

    def test_candidate_translates_only_carrier_and_preserves_authored_look(self) -> None:
        for spec in BUILDER.SPECS:
            source = self.sources[spec.candidate_element_id]
            candidate = self.candidates[spec.candidate_element_id]
            self.assertEqual(candidate["id"], spec.candidate_element_id)
            self.assertEqual(candidate["displayName"], spec.candidate_display_name)
            self.assertLessEqual(len(candidate["displayName"]), 64)
            self.assertEqual(candidate["sourceNode"], spec.candidate_source_node)
            self.assertEqual(candidate["kind"], "particle")
            self.assertTrue(candidate["visible"])
            self.assertEqual(candidate["resources"], source["resources"])
            self.assertEqual(candidate["material"], source["material"])
            self.assertEqual(
                candidate["detail"]["timing"], source["detail"]["timing"]
            )
            self.assertEqual(
                candidate["detail"]["color"], source["detail"]["color"]
            )
            self.assertEqual(candidate["detail"]["uv"], source["detail"]["uv"])
            self.assertEqual(
                candidate["actionCueAttachment"],
                source["actionCueAttachment"],
            )
            self.assertEqual(
                candidate["transformInheritance"], source["transformInheritance"]
            )
            self.assertNotIn(
                "meshModel", {row["slotId"] for row in candidate["resources"]}
            )

    def test_each_candidate_is_one_ground_plane_local_burst(self) -> None:
        for spec in BUILDER.SPECS:
            candidate = self.candidates[spec.candidate_element_id]
            detail = candidate["detail"]
            self.assertEqual(
                detail["transform"]["rotationDegrees"], [90.0, 0.0, 0.0]
            )
            self.assertFalse(detail["sprite"]["billboard"])
            particle = detail["particle"]
            self.assertEqual(particle["maxParticles"], 1)
            self.assertEqual(particle["spawnRatePerSecond"], 0)
            self.assertEqual(particle["burstCount"], 1)
            self.assertTrue(particle["localSpace"])
            self.assertFalse(particle["billboard"])
            self.assertEqual(particle["startSize"], list(spec.start_size))
            self.assertEqual(particle["endSize"], list(spec.end_size))
            lifetime = detail["timing"]["lifeTimeSeconds"]
            self.assertEqual(particle["lifeTimeSeconds"], [lifetime, lifetime])
            self.assertFalse(detail["linearLerp"]["scale"])
            self.assertEqual(
                detail["linearLerp"]["endScale"], [1.0, 1.0, 1.0]
            )
            self.assertFalse(candidate["sourceRecipe"]["enabled"])
            self.assertEqual(candidate["sourceRecipe"]["modules"], [])
            self.assertEqual(candidate["sourcePresentation"], {"enabled": False})

    def test_inner_boundary_growth_is_7_to_18_without_double_scale(self) -> None:
        candidate = self.candidates[
            "project-donut-inner-growing-boundary.sprite-particle-v1"
        ]
        particle = candidate["detail"]["particle"]
        self.assertEqual(particle["startSize"], [7.0, 7.0])
        self.assertEqual(particle["endSize"], [18.0, 18.0])
        self.assertFalse(candidate["detail"]["linearLerp"]["scale"])

    def test_policy_refuses_product_drawable_and_source_exact_claims(self) -> None:
        policy = self.manifest["policy"]
        self.assertEqual(policy["classification"], "PROJECT_TUNED")
        self.assertEqual(policy["fidelity"], "BOUNDED_CARRIER_TRANSLATION")
        self.assertEqual(policy["reconcileMode"], "MISSING_ONLY")
        self.assertTrue(policy["candidateOnly"])
        for key in (
            "canonicalMutationPerformed",
            "cueMutationPerformed",
            "bindingMutationPerformed",
            "catalogMutationPerformed",
            "commonRuntimeMutationPerformed",
            "sourceExactClaim",
        ):
            self.assertFalse(policy[key])
        self.assertEqual(
            policy["drawableProofStatus"],
            "NOT_ATTEMPTED_COMMON_COLOR_ABI_REQUIRED",
        )
        self.assertEqual(policy["productAdmission"], "BLOCKED_CANDIDATE_ONLY")
        self.assertEqual(
            self.manifest["requiredValidation"]["proofAdmission"],
            "DO_NOT_MARK_DRAWABLE_OR_SOURCE_EXACT_YET",
        )

    def test_common_color_abi_gate_is_explicit_and_machine_readable(self) -> None:
        contract = self.manifest["requiresCommonColorAbi"]
        self.assertEqual(contract["status"], "REQUIRED_BEFORE_PRODUCT_ADMISSION")
        self.assertEqual(contract["minimumContractVersion"], 1)
        lanes = {row["assetId"]: row for row in contract["lanes"]}

        ring = lanes[BUILDER.RING_002]
        self.assertEqual(ring["coverageSwizzle"], "A")
        self.assertEqual(ring["coverageSemantic"], "RING_ALPHA")
        self.assertEqual(ring["rgbDecode"], "SRGB_TO_SCENE_LINEAR")
        mask = lanes[BUILDER.RING_004]
        self.assertEqual(mask["roles"], ["MASK"])
        self.assertEqual(mask["dataDecode"], "LINEAR")
        self.assertEqual(mask["coverageSwizzle"], "R")
        self.assertEqual(
            mask["coverageSemantic"], "GRAYSCALE_LUMINANCE_MASK"
        )
        dissolve = lanes[BUILDER.ATYPICAL_011]
        self.assertEqual(dissolve["roles"], ["DISSOLVE"])
        self.assertEqual(dissolve["dataDecode"], "LINEAR")
        self.assertEqual(dissolve["coverageSwizzle"], "R")

        coverage = contract["coverage"]
        self.assertEqual(
            coverage["boundary"]["terms"],
            [
                {
                    "sourceLane": "base",
                    "assetId": BUILDER.RING_002,
                    "swizzle": "A",
                    "operator": "REPLACE",
                },
                {
                    "sourceLane": "mask",
                    "assetId": BUILDER.RING_004,
                    "swizzle": "R",
                    "operator": "MULTIPLY",
                },
                {
                    "sourceLane": "particle",
                    "assetId": "",
                    "swizzle": "A",
                    "operator": "MULTIPLY",
                },
            ],
        )
        self.assertEqual(
            coverage["impact"]["terms"],
            [
                {
                    "sourceLane": "mask",
                    "assetId": BUILDER.RING_002,
                    "swizzle": "A",
                    "operator": "REPLACE",
                },
                {
                    "sourceLane": "dissolve",
                    "assetId": BUILDER.ATYPICAL_011,
                    "swizzle": "R",
                    "operator": "DISSOLVE_MULTIPLY",
                },
                {
                    "sourceLane": "particle",
                    "assetId": "",
                    "swizzle": "A",
                    "operator": "MULTIPLY",
                },
            ],
        )
        for program in (coverage["boundary"], coverage["impact"]):
            self.assertEqual(program["threshold"], 0.0)
            self.assertEqual(program["softness"], 0.0)
            self.assertEqual(
                program["particleAlphaMode"], "MULTIPLY_COVERAGE"
            )
        self.assertTrue(coverage["forbidDxt1ImplicitAlphaCoverage"])

        radiance = contract["radiance"]
        self.assertEqual(radiance["baseRadianceSpace"], "SCENE_LINEAR")
        self.assertEqual(
            radiance["emissiveOperation"], "MULTIPLY_BASE_RADIANCE"
        )
        self.assertTrue(radiance["allowValuesAboveOne"])
        self.assertEqual(radiance["internalHdrClamp"], "FORBIDDEN")
        self.assertEqual(radiance["internalToneMap"], "FORBIDDEN")

    def test_manifest_is_sealed_and_schema_valid(self) -> None:
        manifest = copy.deepcopy(self.manifest)
        seal = manifest.pop("artifactSha256")
        self.assertEqual(seal, BUILDER.canonical_sha256(manifest))
        schema = read_json(self.schema_path)
        self.assertEqual(
            schema["$id"],
            "https://lostark.local/schemas/"
            "lostark.valtan-magic-choice-sprite-particle-companions.schema.json",
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

    def test_resource_closure_is_exact_and_content_addressed(self) -> None:
        evidence = {
            row["assetId"]: (row["byteSize"], row["sha256"])
            for row in self.manifest["resourceEvidence"]
        }
        self.assertEqual(evidence, BUILDER.EXPECTED_RESOURCE_EVIDENCE)
        for spec in BUILDER.SPECS:
            source = self.sources[spec.candidate_element_id]
            candidate = self.candidates[spec.candidate_element_id]
            self.assertEqual(candidate["resources"], source["resources"])

    def test_builder_write_set_cannot_touch_product_contracts(self) -> None:
        forbidden = [
            REPOSITORY_ROOT
            / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
            REPOSITORY_ROOT
            / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json",
            REPOSITORY_ROOT / "Data/Effects/EffectCatalog.json",
            REPOSITORY_ROOT / "Client/Private/Effect_DocumentRenderer.cpp",
            REPOSITORY_ROOT / "Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli",
            *{
                REPOSITORY_ROOT.joinpath(*spec.source_document.parts)
                for spec in BUILDER.SPECS
            },
        ]
        before = {path: path.read_bytes() for path in forbidden}
        BUILDER._validate_write_set(REPOSITORY_ROOT, self.outputs)
        BUILDER.write_outputs(REPOSITORY_ROOT, self.outputs)
        after = {path: path.read_bytes() for path in forbidden}
        self.assertEqual(before, after)
        source_path = REPOSITORY_ROOT.joinpath(
            *BUILDER.SPECS[0].source_document.parts
        )
        with self.assertRaises(BUILDER.CandidateError):
            BUILDER._validate_write_set(
                REPOSITORY_ROOT,
                {source_path: b"forbidden"},
            )
        self.assertNotIn("--apply", BUILDER_PATH.read_text(encoding="utf-8"))

    def test_missing_only_collision_and_source_tamper_fail_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            authored = root / "Data/Effects/Authored"
            authored.mkdir(parents=True)
            collision = {
                "elements": [
                    {
                        "id": BUILDER.SPECS[0].candidate_element_id,
                        "sourceNode": "different",
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

        spec = BUILDER.SPECS[0]
        source_path = REPOSITORY_ROOT.joinpath(*spec.source_document.parts)
        tampered = read_json(source_path)
        row = next(
            element
            for element in tampered["elements"]
            if element["id"] == spec.source_element_id
        )
        row["detail"]["uv"]["speed"][0] = 99.0
        with self.assertRaisesRegex(
            BUILDER.CandidateError, "bounded review is required"
        ):
            BUILDER._source_row(tampered, spec)


if __name__ == "__main__":
    unittest.main()
