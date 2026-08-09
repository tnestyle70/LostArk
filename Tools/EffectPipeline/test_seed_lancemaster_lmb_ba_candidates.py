from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock


SCRIPT_PATH = Path(__file__).with_name("seed_lancemaster_lmb_ba_candidates.py")
SPEC = importlib.util.spec_from_file_location(
    "seed_lancemaster_lmb_ba_candidates", SCRIPT_PATH
)
assert SPEC is not None and SPEC.loader is not None
seed = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(seed)


def resource(element: dict, slot_id: str) -> str:
    for binding in element["resources"]:
        if binding["slotId"] == slot_id:
            return binding["assetId"]
    return ""


class LanceMasterLmbCandidateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.long_source = seed.read_json(seed.LONG_IMPORTED_PATH)
        cls.short_source = seed.read_json(seed.SHORT_IMPORTED_PATH)

    def test_builds_four_long_and_three_short_stage_candidates(self) -> None:
        self.assertEqual(4, len(seed.LONG_STAGES))
        self.assertEqual(3, len(seed.SHORT_STAGES))
        for stage in seed.LONG_STAGES:
            document, receipt = seed.build_long_candidate(self.long_source, stage)
            self.assertEqual(seed.candidate_id(stage["target"]), document["effectAssetId"])
            self.assertEqual(13, document["version"])
            expected_count = 7 if stage["ba"] == 4 else 3
            self.assertEqual(expected_count, len(document["elements"]))
            self.assertTrue(
                all(
                    element["material"]["sourceProfile"]["enabled"] is False
                    for element in document["elements"]
                    if element["visible"]
                )
            )
            ring, trail, ribbon_element = document["elements"][:3]
            self.assertEqual("mesh", ring["kind"])
            self.assertEqual("ring-master", ring["displayName"])
            self.assertTrue(ring["visible"])
            self.assertEqual(seed.RING_PARTICLE_COLOR, ring["detail"]["color"]["multiply"])
            self.assertFalse(ring["transformInheritance"]["enabled"])
            self.assertEqual("trail", trail["kind"])
            self.assertEqual("animtrail-companion", trail["displayName"])
            self.assertTrue(trail["visible"])
            self.assertFalse(trail["transformInheritance"]["enabled"])
            self.assertTrue(trail["actionCueAttachment"]["follow"])
            self.assertEqual(
                "WP_FLM_1_Battle",
                trail["actionCueAttachment"]["runtimeAnchorSlotId"],
            )
            self.assertEqual(
                "b_weapon_rhand", trail["actionCueAttachment"]["runtimeBoneName"]
            )
            ribbon = receipt["anchorCompanions"][1]
            self.assertTrue(ribbon["renderElementCreated"])
            self.assertFalse(ribbon["visible"])
            self.assertEqual(stage["ribbonStart"], ribbon["sourceStartDelaySeconds"])
            self.assertEqual(stage["ribbonDuration"], ribbon["sourceDurationSeconds"])
            self.assertEqual("trail", ribbon_element["kind"])
            self.assertFalse(ribbon_element["visible"])
            self.assertEqual([], ribbon_element["resources"])
            self.assertEqual(
                "effect.ue3.fallback-blocked.v1",
                ribbon_element["material"]["sourceProfile"][
                    "runtimeShaderProfileId"
                ],
            )
            if stage["ba"] == 4:
                kinds = [element["kind"] for element in document["elements"]]
                self.assertEqual(2, kinds.count("mesh"))
                self.assertEqual(3, kinds.count("sprite"))
                self.assertEqual(2, kinds.count("trail"))
                impact = document["elements"][3:]
                impact_master = impact[0]
                self.assertFalse(impact_master["transformInheritance"]["enabled"])
                for companion in impact[1:]:
                    self.assertTrue(companion["transformInheritance"]["enabled"])
                    self.assertEqual(
                        impact_master["id"],
                        companion["transformInheritance"]["masterElementId"],
                    )
                    self.assertEqual(impact_master["groupId"], companion["groupId"])
                    self.assertEqual(
                        impact_master["actionCueAttachment"],
                        companion["actionCueAttachment"],
                    )
                    self.assertEqual(
                        impact_master["detail"]["timing"]["startDelaySeconds"],
                        companion["detail"]["timing"]["startDelaySeconds"],
                    )
            self.assertEqual("BLOCKED_UNTIL_MANUAL_VISUAL_APPROVAL", receipt["productAdmission"])
            self.assertEqual(
                "SOURCE_EXTRACTED",
                receipt["restorationStatus"]["current"],
            )
            self.assertEqual(
                "MANUAL_VISUAL_PENDING",
                receipt["restorationStatus"]["manualAssembly"],
            )
            self.assertEqual(
                "NOT_VISUAL_APPROVED",
                receipt["restorationStatus"]["visualApproval"],
            )
            self.assertFalse(
                receipt["candidateExecutionBoundary"][
                    "executableInCurrentWorktree"
                ]
            )
            self.assertFalse(
                receipt["candidateExecutionBoundary"][
                    "currentWorktreeResourceRootPresent"
                ]
            )
            self.assertTrue(
                receipt["candidateExecutionBoundary"][
                    "codecPlaybackValidatedWithCanonicalResourceRoot"
                ]
            )
            self.assertEqual(
                13,
                receipt["candidateExecutionBoundary"][
                    "currentWorktreeRuntimeFormatVersion"
                ],
            )
            self.assertEqual(
                "V13_CANDIDATE_VALIDATED_NOT_PRODUCT",
                receipt["candidateExecutionBoundary"]["status"],
            )
            self.assertFalse(
                receipt["candidateExecutionBoundary"][
                    "standaloneHarnessSelfRootIntegratedInCurrentWorktree"
                ]
            )
            self.assertTrue(
                receipt["candidateExecutionBoundary"][
                    "standaloneHarnessRequiresExplicitResourceRoot"
                ]
            )
            self.assertTrue(
                receipt["candidateExecutionBoundary"][
                    "upstreamHarnessSelfRootValidated"
                ]
            )
            self.assertEqual(
                "SOURCE_EXTRACTED_V13_CANDIDATE_MANUAL_VISUAL_PENDING",
                receipt["manualRestorationWorklist"]["candidateStatus"],
            )
            self.assertEqual(
                "CARRIER_INVENTORY_VISUAL_UNVERIFIED",
                receipt["manualRestorationWorklist"]["productVisualStatus"],
            )
            self.assertEqual(
                seed.EXPECTED_WORKLIST_SUMMARY,
                receipt["manualRestorationWorklist"]["globalSummary"],
            )
        for stage in seed.SHORT_STAGES:
            document, receipt = seed.build_short_candidate(self.short_source, stage)
            self.assertEqual(13, document["version"])
            kinds = [element["kind"] for element in document["elements"]]
            self.assertEqual(3, kinds.count("mesh"))
            self.assertEqual(3, kinds.count("sprite"))
            self.assertTrue(
                all(
                    element["material"]["sourceProfile"]["enabled"] is False
                    for element in document["elements"]
                    if element["visible"]
                )
            )
            master = document["elements"][0]
            self.assertFalse(master["transformInheritance"]["enabled"])
            for companion in document["elements"][1:]:
                self.assertTrue(companion["transformInheritance"]["enabled"])
                self.assertEqual(
                    master["id"], companion["transformInheritance"]["masterElementId"]
                )
                self.assertEqual(master["groupId"], companion["groupId"])
                self.assertEqual(
                    master["actionCueAttachment"], companion["actionCueAttachment"]
                )
            self.assertTrue(
                all(
                    element["detail"]["timing"]["startDelaySeconds"] == 0.08
                    for element in document["elements"]
                )
            )
            self.assertEqual(2, len(receipt["blockedCarriers"]))

    def test_long_trail_preserves_exact_texture_roles_without_false_emissive_boost(self) -> None:
        stage = seed.LONG_STAGES[1]
        document, receipt = seed.build_long_candidate(self.long_source, stage)
        trail = next(
            element
            for element in document["elements"]
            if element["displayName"] == "animtrail-companion"
        )
        self.assertEqual(stage["trailStart"], trail["detail"]["timing"]["startDelaySeconds"])
        self.assertEqual(stage["trailDuration"], trail["detail"]["timing"]["lifeTimeSeconds"])
        self.assertEqual(0.0, trail["detail"]["color"]["emissiveIntensity"])
        self.assertTrue(resource(trail, "mask").endswith("fx_m_trail_006.dds"))
        self.assertTrue(resource(trail, "emissive").endswith("fx_h_atypical_01_1.dds"))
        self.assertTrue(resource(trail, "dissolve").endswith("fx_k_caustictile_01.dds"))
        self.assertTrue(resource(trail, "noise").endswith("fx_d_noise_030.dds"))
        decisions = {
            row["parameter"]: row["decision"]
            for row in receipt["anchorCompanions"][0]["sourceExact"]["textureRoles"]
        }
        self.assertEqual("EXACT_ASSET_AND_ROLE", decisions["alpha_texture"])
        self.assertIn("NOT_SOURCE_GRAPH_EXACT", decisions["candidate-preview-base"])

    def test_short_candidate_uses_master_frame_and_rejects_cross_emitter_proxy(self) -> None:
        stage = seed.SHORT_STAGES[1]
        document, receipt = seed.build_short_candidate(self.short_source, stage)
        by_name = {element["displayName"]: element for element in document["elements"]}
        self.assertEqual([0.0, 0.0, -0.3], by_name["short-flow-master"]["detail"]["transform"]["position"])
        self.assertEqual(0.08, by_name["short-flow-master"]["detail"]["timing"]["startDelaySeconds"])
        self.assertEqual(0.08, by_name["short-fragment-manual-position"]["detail"]["timing"]["startDelaySeconds"])
        self.assertEqual([0.0, 0.0, -0.3], by_name["short-fragment-manual-position"]["detail"]["transform"]["position"])
        self.assertTrue(
            by_name["short-fragment-manual-position"]["transformInheritance"]["enabled"]
        )
        self.assertEqual(
            by_name["short-flow-master"]["id"],
            by_name["short-fragment-manual-position"]["transformInheritance"][
                "masterElementId"
            ],
        )
        fragment_receipt = next(
            row
            for row in receipt["companionCarriers"]
            if row["roleId"] == "short-fragment-manual-position"
        )
        self.assertEqual(0.1, fragment_receipt["sourceEmitterStartOffsetNotApplied"])
        self.assertEqual(0.08, fragment_receipt["commonPhaseStartDelaySeconds"])
        self.assertTrue(
            all(element["detail"]["sprite"]["billboardRollDegrees"] == 0.0 for element in document["elements"])
        )
        for blocked in receipt["blockedCarriers"]:
            self.assertFalse(blocked["renderElementCreated"])
            self.assertEqual(
                "Effect/LanceMaster/Textures/fx_m_spatter_001_xyclamp.dds",
                blocked["forbiddenProxyAssetId"],
            )
        self.assertEqual(
            "MISSING_OUTSIDE_CURRENT_MASTER_STACK_SLICE",
            receipt["unrestoredSourceCarriers"][0]["status"],
        )

    def test_directx_row_vector_rotation_composes_back_to_flat_rotation(self) -> None:
        for role in seed.SHORT_ROLES:
            row = seed.short_role_receipt(role, "test")
            actual = row["sourceRelativeTransformEvidence"][
                "decodedFlatRotationMatrix"
            ]
            self.assertEqual(
                "EVIDENCE_ONLY_NOT_FINAL_LAYOUT",
                row["sourceRelativeTransformEvidence"]["executionStatus"],
            )
            self.assertEqual(
                (
                    "TERMINAL_MASTER_FINAL_MATRIX"
                    if role["master"]
                    else "MASTER_FINAL_MATRIX_INHERITED"
                ),
                row["runtimeTransformExecution"],
            )
            expected = seed.clean(seed.rotation_matrix(role["rotation"]))
            for actual_row, expected_row in zip(actual, expected):
                for actual_value, expected_value in zip(actual_row, expected_row):
                    self.assertAlmostEqual(actual_value, expected_value)

    def test_decal_inventory_separates_source_and_converted_evidence(self) -> None:
        inventory = seed.build_decal_inventory()
        self.assertEqual("lostark.effect-decal-heavy-inventory", inventory["schema"])
        self.assertEqual(
            seed.EXPECTED_WORKLIST_SUMMARY,
            inventory["manualRestorationWorklist"]["globalSummary"],
        )
        by_key = {
            (row["characterClass"], row["skillId"]): row
            for row in inventory["skills"]
        }
        self.assertIn(("LANCE_MASTER", 34590), by_key)
        self.assertIn(("ARTIST", 31000), by_key)
        self.assertGreater(by_key[("LANCE_MASTER", 34590)]["convertedDecalElementCount"], 0)
        self.assertGreater(by_key[("ARTIST", 31000)]["sourceTypeDataDecalReferenceCount"], 0)
        self.assertEqual([34010, 34510], [row["skillId"] for row in inventory["lmbAbsence"]])

    def test_checked_in_outputs_match_initial_seed(self) -> None:
        seed.check_outputs(seed.desired_outputs())

    def test_writer_refuses_existing_output(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "candidate.effect.json"
            desired = {path: b"{}\n"}
            seed.write_new_outputs(desired)
            with self.assertRaisesRegex(FileExistsError, "refuses existing"):
                seed.write_new_outputs(desired)

    def test_visible_candidate_fails_closed_without_required_resources(self) -> None:
        document = {
            "elements": [
                {
                    "id": "missing-base",
                    "visible": True,
                    "kind": "mesh",
                    "resources": [
                        {"slotId": "meshModel", "assetId": "Effect/Test/test.wmodel"}
                    ],
                }
            ]
        }
        with self.assertRaisesRegex(ValueError, "Base fallback"):
            seed.validate_candidate_no_fallback(document)

        document["elements"][0]["resources"].append(
            {"slotId": "base", "assetId": "Effect/Test/test.dds"}
        )
        seed.validate_candidate_no_fallback(document)

        hidden = {
            "id": "blocked-hidden",
            "visible": False,
            "kind": "trail",
            "resources": [],
            "material": {
                "sourceProfile": {
                    "enabled": True,
                    "runtimeShaderProfileId": "effect.ue3.fallback-blocked.v1",
                }
            },
        }
        seed.validate_candidate_no_fallback({"elements": [hidden]})
        hidden["material"]["sourceProfile"]["runtimeShaderProfileId"] = ""
        with self.assertRaisesRegex(ValueError, "not explicitly fail-closed"):
            seed.validate_candidate_no_fallback({"elements": [hidden]})

    def test_v13_migration_requires_exact_complete_legacy_seed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            temporary_root = Path(directory)
            stage = {"target": "effect.lancemaster.test"}
            legacy_payload = b'{"version":12}\n'
            desired_payload = b'{"version":13}\n'
            metadata = temporary_root / "receipt.json"
            with (
                mock.patch.object(seed, "AUTHORED_ROOT", temporary_root / "Authored"),
                mock.patch.object(seed, "LONG_STAGES", (stage,)),
                mock.patch.object(seed, "SHORT_STAGES", ()),
                mock.patch.object(
                    seed,
                    "LEGACY_V12_CANDIDATE_SHA256",
                    {stage["target"]: seed.sha256_bytes(legacy_payload)},
                ),
            ):
                candidate = seed.candidate_path(stage["target"])
                candidate.parent.mkdir(parents=True)
                candidate.write_bytes(legacy_payload)
                metadata.write_bytes(b'{"status":"old"}\n')
                desired = {
                    candidate: desired_payload,
                    metadata: b'{"status":"new"}\n',
                }
                seed.migrate_v12_candidates_to_v13(desired)
                self.assertEqual(desired_payload, candidate.read_bytes())
                self.assertEqual({"status": "new"}, seed.read_json(metadata))

                candidate.write_bytes(b'{"version":13,"userTuned":true}\n')
                metadata.write_bytes(b'{"status":"preserve"}\n')
                with self.assertRaisesRegex(ValueError, "refusing migration"):
                    seed.migrate_v12_candidates_to_v13(desired)
                self.assertEqual(
                    b'{"version":13,"userTuned":true}\n', candidate.read_bytes()
                )
                self.assertEqual({"status": "preserve"}, seed.read_json(metadata))

    def test_v13_migration_preflights_every_candidate_before_replace(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            temporary_root = Path(directory)
            first = {"target": "effect.lancemaster.first"}
            second = {"target": "effect.lancemaster.second"}
            legacy_payload = b'{"version":12}\n'
            with (
                mock.patch.object(seed, "AUTHORED_ROOT", temporary_root / "Authored"),
                mock.patch.object(seed, "LONG_STAGES", (first, second)),
                mock.patch.object(seed, "SHORT_STAGES", ()),
                mock.patch.object(
                    seed,
                    "LEGACY_V12_CANDIDATE_SHA256",
                    {
                        first["target"]: seed.sha256_bytes(legacy_payload),
                        second["target"]: seed.sha256_bytes(legacy_payload),
                    },
                ),
                mock.patch.object(seed, "replace_outputs_atomically") as replace,
            ):
                first_path = seed.candidate_path(first["target"])
                second_path = seed.candidate_path(second["target"])
                first_path.parent.mkdir(parents=True)
                first_path.write_bytes(legacy_payload)
                second_path.write_bytes(b'{"version":12,"userTuned":true}\n')
                with self.assertRaisesRegex(ValueError, "refusing migration"):
                    seed.migrate_v12_candidates_to_v13(
                        {
                            first_path: b'{"version":13}\n',
                            second_path: b'{"version":13}\n',
                        }
                    )
                replace.assert_not_called()
                self.assertEqual(legacy_payload, first_path.read_bytes())

    def test_metadata_refresh_preserves_and_guards_candidate_documents(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            temporary_root = Path(directory)
            stage = {"target": "effect.lancemaster.test"}
            with (
                mock.patch.object(seed, "AUTHORED_ROOT", temporary_root / "Authored"),
                mock.patch.object(seed, "LONG_STAGES", (stage,)),
                mock.patch.object(seed, "SHORT_STAGES", ()),
            ):
                candidate = seed.candidate_path(stage["target"])
                candidate.parent.mkdir(parents=True)
                candidate_payload = b'{"effectAssetId":"manual-candidate"}\n'
                candidate.write_bytes(candidate_payload)
                metadata = temporary_root / "receipt.json"
                desired = {
                    candidate: candidate_payload,
                    metadata: b'{"status":"pending"}\n',
                }
                seed.refresh_generated_metadata(desired)
                self.assertEqual(candidate_payload, candidate.read_bytes())
                self.assertEqual({"status": "pending"}, seed.read_json(metadata))

                candidate.write_text('{"effectAssetId":"user-tuned"}\n', encoding="utf-8")
                with self.assertRaisesRegex(ValueError, "manually edited"):
                    seed.refresh_generated_metadata(desired)


if __name__ == "__main__":
    unittest.main()
