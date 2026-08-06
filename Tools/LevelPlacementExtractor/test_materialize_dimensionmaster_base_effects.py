#!/usr/bin/env python3

from __future__ import annotations

import json
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest

from materialize_dimensionmaster_base_effects import (
    build_combo_stage_document,
    materializable_extraction_rows,
    reconstruct_source_material_profiles,
    source_confirmed_model_cues,
    synchronize_effect_catalog,
    unsupported_model_material_cues,
)


class MaterializeDimensionMasterBaseEffectsTests(unittest.TestCase):
    def test_rebuilds_v11_profiles_without_old_authored_or_material_map(self) -> None:
        with TemporaryDirectory() as temporary:
            imported_root = Path(temporary)
            (imported_root / "Converted").mkdir()
            (imported_root / "skill.10.source-receipt.json").write_text(
                json.dumps({
                    "schema": "lostark.effect-source-receipt",
                    "characterClass": "DIMENSIONMASTER",
                    "skillId": 10,
                    "materialParameterBindings": [{
                        "sourceMaterialPath": "fx_pkg.fx_mi.child",
                        "sourcePhysicalPackage": "exact.upk",
                        "parent": "fx_m.parent",
                        "scalars": [{"name": "power", "value": 3.0}],
                    }],
                }),
                encoding="utf-8",
            )
            (imported_root / "Converted" /
             "skill.10.element-conversion-receipt.json").write_text(
                json.dumps({"elementConversions": [{
                    "targetKind": "particle", "resourceMappings": []
                }]}),
                encoding="utf-8",
            )
            imported = {
                "version": 10,
                "effectAssetId": "effect.dimensionmaster.skill.10",
                "elements": [{
                    "id": "p0",
                    "kind": "particle",
                    "resources": [],
                    "material": {
                        "templateId": "effect.source_material",
                        "sourceMaterialPath": "fx_pkg.fx_mi.child",
                    },
                    "sourceRecipe": {"modules": []},
                }],
            }
            # This stale value must be ignored because only `imported` plus the
            # source receipt enter the reconstruction function.
            stale_authored = imported_root / "effect.authored.json"
            stale_authored.write_text(json.dumps({
                "version": 11,
                "elements": [{"material": {"sourceProfile": {
                    "enabled": True, "profileId": "manual.stale"
                }}}],
            }), encoding="utf-8")

            upgraded, receipt = reconstruct_source_material_profiles(
                10, imported_root, imported,
                {
                    "schema": "lostark.class-effect-resource-source-manifest",
                    "characterClass": "DIMENSIONMASTER",
                    "assets": [],
                },
            )

            profile = upgraded["elements"][0]["material"]["sourceProfile"]
            self.assertEqual(11, upgraded["version"])
            self.assertTrue(profile["enabled"])
            self.assertNotEqual("manual.stale", profile["profileId"])
            self.assertEqual(
                [{"name": "power", "value": 3.0}], profile["scalars"]
            )
            self.assertEqual(0, receipt["summary"]["failureCount"])

    def test_builds_model_cue_only_from_exact_source_runtime_binding(self) -> None:
        action = {
            "cues": [
                {
                    "cueId": "source-model",
                    "runtimeChannel": "MODEL_CUE",
                    "executionEnabled": True,
                    "globalTimeSeconds": 0.25,
                    "typedPayload": {
                        "transformDecoded": True,
                        "sourceCueName": "SK_Test",
                        "sourceSkeletalMesh": "SK_TEST.Mesh.SK_TEST_SK",
                        "sourceAnimSet": "SK_TEST.Ani.SK_TEST_Ani",
                        "sourceMaterialInstances": ["SK_TEST.Mat.SK_TEST_MI"],
                        "localTransform": {
                            "sourcePositionUeUnits": [0.0, 0.0, 45.0],
                            "position": [0.0, 0.0, 0.45],
                            "rotationDegrees": [0.0, 0.0, 0.0],
                            "scale": [1.2, 1.2, 1.2],
                        },
                    },
                }
            ]
        }
        retime = {
            "clips": [
                {
                    "sourceClip": "sk_test",
                    "runtimeClip": "runtime_sk_test",
                    "durationSeconds": 4.0,
                }
            ]
        }
        bindings = {
            "bindings": [
                {
                    "skillId": 10,
                    "source": {
                        "cueName": "SK_Test",
                        "skeletalMesh": "SK_TEST.Mesh.SK_TEST_SK",
                        "animSet": "SK_TEST.Ani.SK_TEST_Ani",
                        "materialInstances": ["SK_TEST.Mat.SK_TEST_MI"],
                        "animationClip": "sk_test",
                    },
                    "runtime": {
                        "cueId": "test_model",
                        "modelAssetId": "Character/Test/test.wmodel",
                        "clipName": "runtime_sk_test",
                        "assetPreTransform": {
                            "scale": [0.01, 0.01, 0.01],
                            "rotationDegrees": [0.0, -90.0, 0.0],
                        },
                    },
                }
            ]
        }
        result = source_confirmed_model_cues(10, action, retime, bindings)
        self.assertEqual(1, len(result))
        self.assertEqual(0.25, result[0]["startDelaySeconds"])
        self.assertEqual([0.0, 0.0, 0.45], result[0]["localTransform"]["position"])
        self.assertEqual([1.2, 1.2, 1.2], result[0]["localTransform"]["scale"])

    def test_reports_opaque_model_material_cue_as_unsupported(self) -> None:
        action = {
            "cues": [
                {
                    "cueId": "material-at-2.9",
                    "runtimeChannel": "MODEL_MATERIAL",
                    "executionEnabled": True,
                    "typedPayload": {"semanticDecoded": False},
                }
            ]
        }
        self.assertEqual(
            ["material-at-2.9"], unsupported_model_material_cues(action)
        )

    def test_only_current_admitted_skills_are_materializable(self) -> None:
        extraction = {
            "skills": [
                {
                    "skillId": 10,
                    "effectAssetId": "effect.dimensionmaster.skill.10",
                },
                {"skillId": 99, "effectAssetId": "effect.candidate.99"},
            ]
        }
        admitted = [
            {
                "skillId": 10,
                "effectAssetId": "effect.dimensionmaster.skill.10",
                "inputSlot": "Q",
            },
            {
                "skillId": 20,
                "effectAssetId": "effect.dimensionmaster.skill.20",
                "inputSlot": "W",
            },
        ]

        selected, missing = materializable_extraction_rows(
            extraction, admitted
        )

        self.assertEqual([10], [row[0]["skillId"] for row in selected])
        self.assertEqual([20], [row["skillId"] for row in missing])

    def test_combo_stage_rebases_and_filters_element_clock(self) -> None:
        aggregate = {
            "effectAssetId": "effect.aggregate",
            "displayName": "aggregate",
            "modelCues": [{"cueId": "not-stage-owned"}],
            "elements": [
                {"id": "before", "detail": {"timing": {"startDelaySeconds": 3.9}}},
                {"id": "first", "detail": {"timing": {"startDelaySeconds": 4.1}}},
                {"id": "next", "detail": {"timing": {"startDelaySeconds": 5.5}}},
            ],
        }
        result = build_combo_stage_document(
            aggregate, "effect.ba2", "BA2", 4.0, 5.5
        )
        self.assertEqual("effect.ba2", result["effectAssetId"])
        self.assertEqual(["first"], [row["id"] for row in result["elements"]])
        self.assertAlmostEqual(
            0.1, result["elements"][0]["detail"]["timing"]["startDelaySeconds"]
        )
        self.assertEqual([], result["modelCues"])

    def test_catalog_replaces_candidates_with_only_materialized_effects(self) -> None:
        with TemporaryDirectory() as temporary:
            catalog_path = Path(temporary) / "EffectCatalog.json"
            catalog_path.write_text(json.dumps({
                "formatVersion": 1,
                "effects": [
                    {
                        "effectAssetId": "effect.other.skill.1",
                        "authoringPath": "Effects/Authored/other.json",
                    },
                    {
                        "effectAssetId": "effect.dimensionmaster.skill.999",
                        "authoringPath": "Effects/Authored/candidate.json",
                    },
                ],
            }), encoding="utf-8")
            documents = [
                {"effectAssetId": "effect.dimensionmaster.skill.20"},
                {"effectAssetId": "effect.dimensionmaster.skill.10.ba2"},
                {"effectAssetId": "effect.dimensionmaster.skill.10"},
                {"effectAssetId": "effect.dimensionmaster.skill.10.ba1"},
            ]

            result = synchronize_effect_catalog(catalog_path, documents)
            catalog = json.loads(catalog_path.read_text(encoding="utf-8"))

            self.assertEqual(1, result["preservedEffectCount"])
            self.assertEqual(4, result["dimensionMasterEffectCount"])
            self.assertEqual([
                "effect.other.skill.1",
                "effect.dimensionmaster.skill.10",
                "effect.dimensionmaster.skill.10.ba1",
                "effect.dimensionmaster.skill.10.ba2",
                "effect.dimensionmaster.skill.20",
            ], [row["effectAssetId"] for row in catalog["effects"]])


if __name__ == "__main__":
    unittest.main()
