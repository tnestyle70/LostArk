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
            17,
            sum(row["reconcileAction"] == "APPEND_MISSING" for row in desired),
        )
        self.assertEqual(
            7,
            sum(row["reconcileAction"] == "PRESERVE_EXISTING" for row in desired),
        )
        self.assertEqual(9, len(artifacts.documents))
        self.assertEqual(
            17,
            sum(len(document["elements"]) for document in artifacts.documents.values()),
        )
        self.assertTrue(receipt["policy"]["portalRushExcluded"])
        self.assertNotIn(
            "VALTAN_PORTAL_RUSH",
            {target["patternId"] for target in receipt["targets"]},
        )
        self.assertEqual(
            "PRESERVE_EXISTING_OFFICIAL_ASSET_REUSE",
            receipt["officialAxePresentation"]["disposition"],
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
        self.assertEqual(8, len(roles))
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
        self.assertEqual(
            "PROJECT_PALETTE_REUSE_NO_SOURCE_CLAIM",
            roles["HIGH_JUMP_AXE_GROUND_IMPACT"]["evidenceStatus"],
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

    def test_high_jump_combat_object_transfer_preserves_official_axe(self) -> None:
        receipt = self.build().receipt
        transfer = receipt["highJumpCombatObjectTransfer"]
        self.assertEqual("VALTAN_HIGH_JUMP", transfer["patternId"])
        self.assertEqual("AIRBORNE", transfer["stageId"])
        self.assertEqual(
            MODULE.HIGH_JUMP_COMBAT_OBJECT_ID,
            transfer["combatObjectArchetypeId"],
        )
        self.assertEqual(MODULE.SKY_AXE_EFFECT_ID, transfer["effectAssetId"])
        self.assertEqual(
            "LOCKED_TARGET_PER_ALIVE_PLAYER",
            transfer["combatObjectContract"]["originPolicy"],
        )
        self.assertEqual(1200, transfer["combatObjectContract"]["timedImpactMs"])
        self.assertFalse(
            transfer["retiredBossRoot"]["effectCatalogRegistrationAllowed"]
        )
        self.assertFalse(
            transfer["retiredBossRoot"]["productCueRegistrationAllowed"]
        )
        self.assertTrue(transfer["authority"]["effectPresentationOnly"])
        self.assertTrue(transfer["authority"]["serverCombatObjectAuthority"])
        self.assertFalse(transfer["authority"]["serverGameplayChange"])
        self.assertEqual(
            "TIMED_COMBAT_OBJECT_HIT",
            transfer["authority"]["serverDamagePolicy"],
        )
        presentation = receipt["officialAxePresentation"]
        self.assertEqual(
            MODULE.SKY_AXE_DESCENT_ELEMENT_ID,
            presentation["presentationId"],
        )
        self.assertEqual(MODULE.ASSET_VALTAN_WEAPON, presentation["modelAssetId"])
        self.assertEqual("OFFICIAL_GEOMETRY_EXACT", presentation["geometryProvenance"])
        self.assertEqual(
            "EXISTING_HAND_TUNED_PRESERVE",
            presentation["trajectoryTimingProvenance"],
        )

        overlay = self.build().documents[MODULE.SKY_AXE_EFFECT_ID]
        self.assertEqual(2, len(overlay["elements"]))
        self.assertEqual(
            {MODULE.SKY_AXE_TARGET_DECAL_ELEMENT_ID, MODULE.SKY_AXE_IMPACT_ELEMENT_ID},
            {element["id"] for element in overlay["elements"]},
        )
        self.assertNotIn(
            MODULE.SKY_AXE_DESCENT_ELEMENT_ID,
            {element["id"] for element in overlay["elements"]},
        )
        impact = next(
            element for element in overlay["elements"]
            if element["id"] == MODULE.SKY_AXE_IMPACT_ELEMENT_ID
        )
        self.assertEqual(1.2, impact["detail"]["timing"]["startDelaySeconds"])
        self.assertEqual(MODULE.ASSET_SHOCKWAVE_02, impact["resources"][0]["assetId"])

        encounter = MODULE._load_json(
            self.repo_root / "Data/Encounters/Valtan/ValtanEncounter.json"
        )
        high_jump = next(
            pattern for pattern in encounter["patterns"]
            if pattern["patternId"] == "VALTAN_HIGH_JUMP"
        )
        stages = {stage["stageId"]: stage for stage in high_jump["stages"]}
        self.assertEqual("NONE", stages["AIRBORNE"]["hitShape"])
        self.assertEqual(0, stages["AIRBORNE"]["hitCount"])
        self.assertEqual("", stages["AIRBORNE"]["serverDamageProfileId"])
        self.assertEqual("CIRCLE", stages["LAND"]["hitShape"])
        self.assertEqual(1, stages["LAND"]["hitCount"])
        self.assertEqual(900, stages["LAND"]["hitDelayMs"])
        self.assertEqual(
            "damage.valtan.high-jump",
            stages["LAND"]["serverDamageProfileId"],
        )

    def test_circular_decal_growth_uses_xz_footprint_axes(self) -> None:
        documents = self.build().documents
        cases = (
            (
                "effect.valtan.front-back-front.active",
                "project-three-hit-down-smash-wave",
                [0.35, 1.0, 0.35],
                [1.0, 1.0, 1.0],
                (4.2, 12.0),
            ),
            (
                "effect.valtan.front-back-front.active",
                "project-three-hit-down-smash-shockwave",
                [0.25, 1.0, 0.25],
                [1.0, 1.0, 1.0],
                (2.5, 10.0),
            ),
            (
                "effect.valtan.high-jump.land",
                "project-high-jump-landing-wave",
                [0.35, 1.0, 0.35],
                [1.0, 1.0, 1.0],
                (4.9, 14.0),
            ),
            (
                "effect.valtan.magic-choice.windup",
                "project-donut-inner-growing-boundary",
                [1.0, 1.0, 1.0],
                [18.0 / 7.0, 1.0, 18.0 / 7.0],
                (7.0, 18.0),
            ),
        )

        for effect_id, element_id, start_scale, end_scale, footprint in cases:
            with self.subTest(effect_id=effect_id, element_id=element_id):
                element = next(
                    row
                    for row in documents[effect_id]["elements"]
                    if row["id"] == element_id
                )
                detail = element["detail"]
                self.assertEqual(start_scale, detail["transform"]["scale"])
                self.assertEqual(end_scale, detail["linearLerp"]["endScale"])
                self.assertTrue(detail["linearLerp"]["scale"])

                size_x, size_z = detail["decal"]["size"]
                start_xz = (
                    round(size_x * start_scale[0], 6),
                    round(size_z * start_scale[2], 6),
                )
                end_xz = (
                    round(size_x * end_scale[0], 6),
                    round(size_z * end_scale[2], 6),
                )
                self.assertEqual((footprint[0], footprint[0]), start_xz)
                self.assertEqual((footprint[1], footprint[1]), end_xz)

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
            projection_receipt = (
                destination
                / "Data/Effects/Imported/Valtan/ProjectAuthoredPriority/"
                "Valtan.project-authored-priority.projection-receipt.v1.json"
            )
            projection_receipt.write_text("{}\n", encoding="utf-8")
            projection_payload = projection_receipt.read_bytes()
            MODULE.check_artifacts(artifacts, destination)
            first = {
                relative.as_posix(): destination.joinpath(*relative.parts).read_bytes()
                for relative in artifacts.files
            }
            MODULE.write_artifacts(artifacts, destination)
            MODULE.check_artifacts(artifacts, destination)
            self.assertEqual(projection_payload, projection_receipt.read_bytes())
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

    def test_postapply_canonical_state_keeps_the_preapply_candidate_bytes(self) -> None:
        postapply = self.build()
        overrides = {}
        project_ids = {
            target.effect_asset_id: {
                element["id"]
                for element in postapply.documents[target.effect_asset_id]["elements"]
            }
            for target in MODULE._target_specs()
        }
        for target in MODULE._target_specs():
            path = MODULE._canonical_authoring_path(target.effect_asset_id)
            canonical_path = self.repo_root.joinpath(*path.parts)
            canonical = MODULE._load_json(canonical_path)
            canonical["elements"] = [
                element
                for element in canonical["elements"]
                if element["id"] not in project_ids[target.effect_asset_id]
            ]
            overrides[target.effect_asset_id] = canonical
        preapply = self.build(canonical_overrides=overrides)
        self.assertEqual(postapply.files, preapply.files)
        self.assertEqual(9, len(postapply.documents))
        self.assertEqual(
            17,
            sum(len(row["elements"]) for row in postapply.documents.values()),
        )

    def test_existing_project_element_resource_drift_fails_closed(self) -> None:
        effect_id = MODULE.SKY_AXE_EFFECT_ID
        canonical = MODULE._load_json(
            self.repo_root
            / "Data/Effects/Authored/effect.valtan.sky-axe.active.effect.json"
        )
        impact = next(
            row for row in canonical["elements"]
            if row["id"] == MODULE.SKY_AXE_IMPACT_ELEMENT_ID
        )
        impact["resources"][0]["assetId"] = (
            "Effect/Valtan/Textures/FX_TEX_04/fx_i_shockwave_03.dds"
        )
        with self.assertRaisesRegex(
            MODULE.ContractError, "projected element immutable identity drift"
        ):
            self.build(canonical_overrides={effect_id: canonical})

    def test_high_jump_combat_object_ownership_is_exact(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            relative_paths = (
                MODULE.EFFECT_CATALOG_RELATIVE_PATH,
                MODULE.VALTAN_CUES_RELATIVE_PATH,
                MODULE.BOSS_CATALOG_RELATIVE_PATH,
                MODULE.COMBAT_OBJECTS_RELATIVE_PATH,
                MODULE.LEGACY_HIGH_JUMP_AUTHORING_PATH,
            )
            for relative in relative_paths:
                source = self.repo_root.joinpath(*relative.parts)
                destination = root.joinpath(*relative.parts)
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(source.read_bytes())
            MODULE._validate_high_jump_combat_object_ownership(root)

            catalog_path = root.joinpath(*MODULE.EFFECT_CATALOG_RELATIVE_PATH.parts)
            catalog = MODULE._load_json(catalog_path)
            catalog["effects"].append({
                "effectAssetId": MODULE.LEGACY_HIGH_JUMP_EFFECT_ID,
                "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
                "authoringPath": (
                    "Effects/Authored/effect.valtan.high-jump.airborne.effect.json"
                ),
            })
            catalog_path.write_text(json.dumps(catalog) + "\n", encoding="utf-8")
            with self.assertRaisesRegex(MODULE.ContractError, "re-registered"):
                MODULE._validate_high_jump_combat_object_ownership(root)

    def test_missing_runtime_resource_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            with self.assertRaisesRegex(
                MODULE.ContractError, "required candidate resource is missing"
            ):
                MODULE.build_artifacts(
                    self.repo_root,
                    resource_root=Path(temporary),
                )

    def test_character_wmodel_is_mesh_model_only_and_fail_closed(self) -> None:
        resolved = MODULE._safe_resource_path(
            self.resource_root,
            MODULE.ASSET_VALTAN_WEAPON,
            element_kind="mesh",
            slot_id="meshModel",
        )
        self.assertTrue(resolved.is_file())
        rejected = (
            (MODULE.ASSET_VALTAN_WEAPON, "decal", "meshModel"),
            (MODULE.ASSET_VALTAN_WEAPON, "mesh", "base"),
            (
                "Character/Valtan/textures/wp_mn_rpbf_01_d.dds",
                "mesh",
                "meshModel",
            ),
            (
                "Character/Valtan/../ValtanWeapon.wmodel",
                "mesh",
                "meshModel",
            ),
            (
                "Character\\Valtan\\ValtanWeapon.wmodel",
                "mesh",
                "meshModel",
            ),
            (
                "C:/Character/Valtan/ValtanWeapon.wmodel",
                "mesh",
                "meshModel",
            ),
        )
        for asset_id, kind, slot_id in rejected:
            with self.subTest(asset_id=asset_id, kind=kind, slot_id=slot_id):
                with self.assertRaises(MODULE.ContractError):
                    MODULE._safe_resource_path(
                        self.resource_root,
                        asset_id,
                        element_kind=kind,
                        slot_id=slot_id,
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
