#!/usr/bin/env python3

from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch


sys.path.insert(0, str(Path(__file__).resolve().parent))

import materialize_representative_authored_baselines as materializer


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
TEST_MESH_ASSET = "Effect/DimensionMaster/Meshes/fm_h_swing_02.wmodel"
TEST_TEXTURE_ASSET = (
    "Effect/DimensionMaster/Textures/FX_TEX_05/fx_l_environment_001.dds"
)


def _fixed_recipe(renderer_shape: str) -> dict:
    return {
        "enabled": True,
        "rendererShape": renderer_shape,
        "emitterDelaySeconds": 0.0,
        "emitterDurationSeconds": 0.0,
        "emitterLoopCount": 1,
        "bursts": [
            {
                "timeSeconds": 0.0,
                "countMinimum": 1,
                "countMaximum": 1,
            }
        ],
        "modules": [
            {
                "stableId": "test:required",
                "className": "particlemodulerequired",
                "objectPath": "Test.Required",
                "literals": [],
                "distributions": [
                    {
                        "propertyPath": "spawnrate",
                        "sourceClass": "",
                        "sourceObjectPath": "",
                        "componentCount": 1,
                        "operation": 1,
                        "randomLockAxes": 0,
                        "lookupTableChunkSize": 0,
                        "lookupTableNumElements": 0,
                        "lookupTableTimeScale": 0.0,
                        "lookupTableStartTime": 0.0,
                        "defaultMinimum": [0.0, 0.0, 0.0, 0.0],
                        "defaultMaximum": [0.0, 0.0, 0.0, 0.0],
                        "lookupTable": [],
                        "keys": [],
                    }
                ],
            }
        ],
    }


def _disabled_recipe() -> dict:
    return {
        "enabled": False,
        "rendererShape": "",
        "emitterDelaySeconds": 0.0,
        "emitterDurationSeconds": 0.0,
        "emitterLoopCount": 0,
        "bursts": [],
        "modules": [],
    }


def _source_element(
    element_id: str,
    classification: str,
    start_delay: float = 1.25,
) -> dict:
    if classification == materializer.STANDALONE_MESH:
        kind = "mesh"
        recipe = _disabled_recipe()
        renderer_shape = "mesh"
    elif classification == materializer.STANDALONE_SPRITE:
        kind = "sprite"
        recipe = _disabled_recipe()
        renderer_shape = "sprite"
    elif classification == materializer.MESH_PARTICLE:
        kind = "particle"
        recipe = _fixed_recipe("mesh")
        recipe["bursts"][0]["countMaximum"] = 2
        renderer_shape = "mesh"
    elif classification == materializer.SPRITE_PARTICLE:
        kind = "particle"
        recipe = _fixed_recipe("sprite")
        recipe["modules"][0]["className"] = "particlemodulevelocity"
        renderer_shape = "sprite"
    elif classification == "convertibleMeshEmitter":
        kind = "particle"
        recipe = _fixed_recipe("mesh")
        renderer_shape = "mesh"
    elif classification == "convertibleSpriteEmitter":
        kind = "particle"
        recipe = _fixed_recipe("sprite")
        renderer_shape = "sprite"
    else:
        raise AssertionError(f"Unknown fixture classification: {classification}")

    resources = [{"slotId": "base", "assetId": TEST_TEXTURE_ASSET}]
    if renderer_shape == "mesh":
        resources.insert(
            0,
            {"slotId": "meshModel", "assetId": TEST_MESH_ASSET},
        )
    return {
        "id": element_id,
        "displayName": element_id,
        "groupId": "source.group",
        "sourceNode": f"source:{element_id}",
        "visible": True,
        "kind": kind,
        "resources": resources,
        "material": {
            "templateId": "material.template.test",
            "sourceMaterialPath": "Test.Material",
            "renderProfile": "source-exact",
            "sourceProfile": {
                "enabled": True,
                "profileId": "source.profile.test",
                "runtimeShaderProfileId": "effect.source-profile",
                "semanticStatus": "reconstructed_profile",
                "textures": [
                    {
                        "name": "base",
                        "sourceObjectPath": "Test.Texture",
                        "assetId": TEST_TEXTURE_ASSET,
                    }
                ],
            },
        },
        "actionCueAttachment": {
            "enabled": True,
            "follow": False,
            "sourceAnchorSlotId": "root",
            "runtimeAnchorSlotId": "root",
            "runtimeBoneName": "",
            "socketLocalTransform": {
                "position": [0.5, 0.25, 1.0],
                "rotationDegrees": [0.0, 20.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        },
        "sourceRecipe": recipe,
        "sourcePresentation": {
            "enabled": True,
            "schema": "lostark.effect-source-presentation",
            "version": 1,
            "profileId": "source.presentation.test",
            "status": "resolved",
            "sourceObjectPath": f"Test.{element_id}",
            "sourceActionCueId": "test.action",
            "sourceEventId": "test.event",
            "sourceOccurrenceIndex": 0,
            "sourceTimeSeconds": start_delay,
            "parameters": [],
        },
        "detail": {
            "timing": {
                "startDelaySeconds": start_delay,
                "lifeTimeSeconds": 0.5,
                "afterImageSeconds": 0.0,
                "dissolveStartNormalized": 0.8,
            },
            "transform": {
                "position": [1.0, 2.0, 3.0],
                "rotationDegrees": [4.0, 5.0, 6.0],
                "revolutionDegreesPerSecond": [0.0, 0.0, 0.0],
                "scale": [0.2, 0.3, 0.4],
                "velocityPerSecond": [0.0, 0.0, 0.0],
            },
            "mesh": {"useModelMaterial": True},
            "sprite": {
                "billboard": False,
                "billboardRollDegrees": 0.0,
            },
            "particle": {
                "spawnRatePerSecond": 0.0,
                "burstCount": 1,
                "lifeTimeSeconds": [0.5, 0.5],
                "billboard": True,
            },
        },
    }


def _source_document(elements: list[dict]) -> dict:
    return {
        "schema": "lostark.effect-authoring",
        "version": 12,
        "effectAssetId": "effect.test.skill.1",
        "displayName": "Synthetic Source",
        "particleSystem": {
            "uniformScaleMultiplier": 1.0,
            "yawOffsetDegrees": 0.0,
            "directionYawDegrees": 0.0,
            "initialSpeedMultiplier": 1.0,
        },
        "modelCues": [],
        "elements": elements,
    }


def _ready_manifest() -> dict:
    return {
        "source": {
            "effectAssetId": "effect.test.skill.1",
        }
    }


def _reviewed_layer(source_element: dict, role: str) -> dict:
    analysis = materializer.analyze_source_element(source_element)
    target_candidates = analysis["targetKindCandidates"]
    fallback_target = (
        "mesh" if analysis["rendererShape"] == "mesh" else "particle"
    )
    target_kind = target_candidates[0] if target_candidates else fallback_target
    removed_module_dispositions = []
    if source_element["kind"] == "particle" and target_kind in {"mesh", "sprite"}:
        removed_module_dispositions = [
            {
                "sourceModuleStableId": module["stableId"],
                "disposition": "bakedIntoAuthoredDetail",
                "targetFields": ["detail.timing", "detail.transform"],
            }
            for module in source_element["sourceRecipe"]["modules"]
        ]
    return {
        "role": role,
        "sourceElementId": source_element["id"],
        "sourceElementSha256": materializer.source_element_sha256(source_element),
        "sourceDocumentKind": analysis["sourceDocumentKind"],
        "sourceKind": analysis["sourceKind"],
        "sourceClassification": analysis["sourceClassification"],
        "conversionEligibility": analysis["conversionEligibility"],
        "conversionReasonCodes": analysis["reasonCodes"],
        "fidelityWarningCodes": analysis["fidelityWarningCodes"],
        "fidelityWarnings": analysis["fidelityWarnings"],
        "targetKind": target_kind,
        "targetAnchorPolicy": (
            "PRESERVE_SOURCE_ATTACHMENT"
            if target_kind == "particle"
            else "AUTHORED_ROOT_SNAPSHOT"
        ),
        "removedModuleDisposition": analysis[
            "requiredRemovedModuleDisposition"
        ],
        "removedModuleDispositions": removed_module_dispositions,
    }


def _stage(layers: list[dict], *, offset: float = 1.0) -> dict:
    return {
        "stageIndex": 0,
        "clip": "test_clip",
        "status": "ready",
        "sourceTimelineOffsetSeconds": offset,
        "targetEffectAssetId": "effect.test.skill.1.ba1",
        "targetAuthoringPath": "Effects/Authored/effect.test.skill.1.ba1.effect.json",
        "displayName": "Synthetic BA1",
        "occurrences": [
            {
                "occurrenceId": "hit01",
                "groupId": "test.ba1.hit01",
                "authoredAnchorPolicy": {
                    "provenance": "AUTHORED_POLICY",
                    "runtimeAnchorSlotId": "root",
                    "follow": False,
                    "socketLocalTransform": {
                        "position": [0.0, 0.0, 0.0],
                        "rotationDegrees": [0.0, 0.0, 0.0],
                        "scale": [1.0, 1.0, 1.0],
                    },
                },
                "layers": layers,
            }
        ],
    }


def _ready_skill_manifest(source_path: Path, source_sha256: str, stages: list[dict]) -> dict:
    return {
        "schema": materializer.SKILL_SCHEMA,
        "version": materializer.SKILL_VERSION,
        "materializationId": "test.skill.1",
        "characterClass": "TEST",
        "skillId": 1,
        "inputSlot": "A",
        "status": materializer.READY,
        "spriteBillboardRollDegrees": -90.0,
        "source": {
            "effectAssetId": "effect.test.skill.1",
            "authoringPath": source_path.as_posix(),
            "expectedState": "present",
            "sha256": source_sha256,
        },
        "blockers": [],
        "evidence": [],
        "stages": stages,
    }


def _stage_for_index(
    source_element: dict, stage_index: int, offset: float
) -> dict:
    stage = _stage(
        [_reviewed_layer(source_element, f"stage-{stage_index:02d}")],
        offset=offset,
    )
    stage["stageIndex"] = stage_index
    stage["clip"] = f"test_clip_{stage_index:02d}"
    stage["targetEffectAssetId"] = f"effect.test.skill.1.ba{stage_index + 1}"
    stage["targetAuthoringPath"] = (
        f"Effects/Authored/effect.test.skill.1.ba{stage_index + 1}.effect.json"
    )
    stage["displayName"] = f"Synthetic BA{stage_index + 1}"
    stage["occurrences"][0]["occurrenceId"] = f"hit-{stage_index:02d}"
    stage["occurrences"][0]["groupId"] = f"test.ba{stage_index + 1}.hit01"
    return stage


def _evaluate_ready_in_temporary_data_root(
    source: dict, stages: list[dict]
) -> tuple[dict, list[tuple[Path, dict]]]:
    with tempfile.TemporaryDirectory() as temporary_directory:
        data_root = Path(temporary_directory) / "Data"
        authored_root = data_root / "Effects/Authored"
        authored_root.mkdir(parents=True)
        source_relative_path = Path(
            "Effects/Authored/effect.test.skill.1.effect.json"
        )
        source_path = data_root / source_relative_path
        source_path.write_text(
            json.dumps(source, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        manifest = _ready_skill_manifest(
            source_relative_path,
            hashlib.sha256(source_path.read_bytes()).hexdigest(),
            stages,
        )
        with (
            patch.object(materializer, "DATA_ROOT", data_root.resolve()),
            patch.object(materializer, "AUTHORED_ROOT", authored_root.resolve()),
        ):
            return materializer.evaluate_skill_manifest(manifest, -90.0)


class RepresentativeAuthoredMaterializerTests(unittest.TestCase):
    def test_classifier_keeps_source_kind_separate_from_conversion_target(self) -> None:
        cases = (
            (materializer.STANDALONE_MESH, materializer.STANDALONE_MESH),
            (materializer.MESH_PARTICLE, materializer.MESH_PARTICLE),
            (materializer.STANDALONE_SPRITE, materializer.STANDALONE_SPRITE),
            (materializer.SPRITE_PARTICLE, materializer.SPRITE_PARTICLE),
            ("convertibleMeshEmitter", materializer.MESH_PARTICLE),
            ("convertibleSpriteEmitter", materializer.SPRITE_PARTICLE),
        )
        for fixture_classification, expected in cases:
            with self.subTest(fixture_classification=fixture_classification):
                element = _source_element("source.element", fixture_classification)
                self.assertEqual(
                    materializer.classify_source_element(element), expected
                )
                analysis = materializer.analyze_source_element(element)
                if fixture_classification == "convertibleMeshEmitter":
                    self.assertEqual(
                        analysis["conversionEligibility"], materializer.CERTIFIED
                    )
                    self.assertEqual(analysis["targetKindCandidates"], ["mesh"])
                elif fixture_classification == "convertibleSpriteEmitter":
                    self.assertEqual(
                        analysis["conversionEligibility"], materializer.CERTIFIED
                    )
                    self.assertEqual(analysis["targetKindCandidates"], ["sprite"])

    def test_classifier_rejects_each_particle_dependent_semantic(self) -> None:
        cases = {}

        multi_spawn = _source_element("multi", "convertibleMeshEmitter")
        multi_spawn["sourceRecipe"]["bursts"][0]["countMaximum"] = 2
        cases["multi-spawn"] = multi_spawn

        spawn_rate = _source_element("rate", "convertibleMeshEmitter")
        spawn_rate_distribution = spawn_rate["sourceRecipe"]["modules"][0][
            "distributions"
        ][0]
        spawn_rate_distribution["defaultMinimum"][0] = 1.0
        spawn_rate_distribution["defaultMaximum"][0] = 1.0
        cases["spawn-rate"] = spawn_rate

        random_distribution = _source_element("random", "convertibleMeshEmitter")
        random_distribution["sourceRecipe"]["modules"][0]["distributions"][0][
            "operation"
        ] = 2
        cases["random"] = random_distribution

        for module_fragment in ("velocity", "orbit"):
            element = _source_element(
                module_fragment, "convertibleMeshEmitter"
            )
            element["sourceRecipe"]["modules"].append(
                {
                    "stableId": f"test:{module_fragment}",
                    "className": f"particlemodule{module_fragment}",
                    "objectPath": f"Test.{module_fragment}",
                    "literals": [],
                    "distributions": [],
                }
            )
            cases[module_fragment] = element

        for dependency, element in cases.items():
            with self.subTest(dependency=dependency):
                self.assertEqual(
                    materializer.classify_source_element(element),
                    materializer.MESH_PARTICLE,
                )

    def test_nonzero_burst_time_is_not_certified_for_standalone_conversion(
        self,
    ) -> None:
        mesh = _source_element("delayed.mesh", "convertibleMeshEmitter")
        mesh["sourceRecipe"]["bursts"][0]["timeSeconds"] = 0.1
        mesh_analysis = materializer.analyze_source_element(mesh)
        self.assertEqual(
            mesh_analysis["conversionEligibility"], materializer.PARTICLE_REQUIRED
        )
        self.assertEqual(mesh_analysis["sourceClassification"], materializer.MESH_PARTICLE)
        self.assertEqual(mesh_analysis["targetKindCandidates"], [])
        self.assertIn(
            "burst-time-particle-dependent", mesh_analysis["reasonCodes"]
        )

        sprite = _source_element("delayed.sprite", "convertibleSpriteEmitter")
        sprite["sourceRecipe"]["bursts"][0]["timeSeconds"] = 0.1
        sprite_analysis = materializer.analyze_source_element(sprite)
        self.assertEqual(
            sprite_analysis["conversionEligibility"],
            materializer.PARTICLE_REQUIRED,
        )
        self.assertEqual(
            sprite_analysis["sourceClassification"], materializer.SPRITE_PARTICLE
        )
        self.assertEqual(sprite_analysis["targetKindCandidates"], ["particle"])

    def test_nonzero_emitter_delay_is_not_certified_without_delay_folding(
        self,
    ) -> None:
        element = _source_element("delayed.emitter", "convertibleMeshEmitter")
        element["sourceRecipe"]["emitterDelaySeconds"] = 0.1

        analysis = materializer.analyze_source_element(element)
        self.assertEqual(
            analysis["conversionEligibility"], materializer.PARTICLE_REQUIRED
        )
        self.assertEqual(analysis["sourceKind"], materializer.MESH_PARTICLE)
        self.assertEqual(analysis["targetKindCandidates"], [])
        self.assertIn(
            "emitter-delay-particle-dependent", analysis["reasonCodes"]
        )

    def test_nonzero_rate_scale_is_particle_required(self) -> None:
        element = _source_element("rate.scale", "convertibleSpriteEmitter")
        rate_scale = copy.deepcopy(
            element["sourceRecipe"]["modules"][0]["distributions"][0]
        )
        rate_scale["propertyPath"] = "Spawn.RateScale"
        rate_scale["defaultMinimum"] = [1.0, 0.0, 0.0, 0.0]
        rate_scale["defaultMaximum"] = [1.0, 0.0, 0.0, 0.0]
        element["sourceRecipe"]["modules"][0]["distributions"].append(
            rate_scale
        )

        analysis = materializer.analyze_source_element(element)
        self.assertEqual(
            analysis["conversionEligibility"], materializer.PARTICLE_REQUIRED
        )
        self.assertEqual(analysis["sourceClassification"], materializer.SPRITE_PARTICLE)
        self.assertEqual(analysis["targetKindCandidates"], ["particle"])
        self.assertIn("rate-property-particle-dependent", analysis["reasonCodes"])

    def test_unknown_zero_distribution_module_is_not_certified(self) -> None:
        element = _source_element("unknown.module", "convertibleMeshEmitter")
        element["sourceRecipe"]["modules"].append(
            {
                "stableId": "test:kill-box",
                "className": "particlemodulekillbox",
                "objectPath": "Test.KillBox",
                "literals": [],
                "distributions": [],
            }
        )

        analysis = materializer.analyze_source_element(element)
        self.assertEqual(analysis["conversionEligibility"], materializer.UNKNOWN)
        self.assertEqual(analysis["sourceClassification"], materializer.MESH_PARTICLE)
        self.assertEqual(analysis["targetKindCandidates"], [])
        self.assertIn("source-module-class-unsupported", analysis["reasonCodes"])

    def test_deterministic_curve_requires_review_without_erasing_eligibility(
        self,
    ) -> None:
        element = _source_element("deterministic.curve", "convertibleMeshEmitter")
        distribution = copy.deepcopy(
            element["sourceRecipe"]["modules"][0]["distributions"][0]
        )
        distribution["propertyPath"] = "ColorOverLife.Color"
        distribution["lookupTable"] = [0.0, 1.0]
        element["sourceRecipe"]["modules"].append(
            {
                "stableId": "test:color-over-life",
                "className": "particlemodulecoloroverlife",
                "objectPath": "Test.ColorOverLife",
                "literals": [{"propertyPath": "ClampAlpha", "value": True}],
                "distributions": [distribution],
            }
        )

        analysis = materializer.analyze_source_element(element)
        self.assertEqual(analysis["conversionEligibility"], materializer.CERTIFIED)
        self.assertEqual(analysis["sourceKind"], materializer.MESH_PARTICLE)
        self.assertEqual(analysis["targetKindCandidates"], ["mesh"])
        self.assertNotIn(
            "source-distribution-curve-particle-dependent",
            analysis["reasonCodes"],
        )
        self.assertEqual(
            analysis["fidelityWarnings"],
            [
                {
                    "sourceModuleStableId": "test:color-over-life",
                    "warningCodes": [
                        "deterministic-distribution-curve-review-required",
                        "deterministic-module-fidelity-review-required",
                        "source-module-literals-review-required",
                    ],
                }
            ],
        )

    def test_dimensionmaster_selected_six_keep_cascade_source_kind_regression(
        self,
    ) -> None:
        source_path = REPOSITORY_ROOT / (
            "Data/Effects/Authored/"
            "effect.dimensionmaster.skill.2050210.effect.json"
        )
        correction_path = REPOSITORY_ROOT / (
            "Data/Effects/AuthoredCorrections/DimensionMaster/"
            "effect.dimensionmaster.skill.2050210.authored-baseline.correction.json"
        )
        source = json.loads(source_path.read_text(encoding="utf-8"))
        correction = json.loads(correction_path.read_text(encoding="utf-8"))
        by_id = {element["id"]: element for element in source["elements"]}
        self.assertEqual(len(correction["layers"]), 6)
        for layer in correction["layers"]:
            with self.subTest(role=layer["role"]):
                source_analysis = materializer.analyze_source_element(
                    by_id[layer["sourceElementBaseId"]]
                )
                expected_source_kind = (
                    materializer.MESH_PARTICLE
                    if layer["carrierKind"] == materializer.STANDALONE_MESH
                    else materializer.SPRITE_PARTICLE
                )
                self.assertEqual(
                    source_analysis["sourceClassification"], expected_source_kind
                )
                self.assertEqual(source_analysis["sourceKind"], expected_source_kind)
                self.assertEqual(source_analysis["sourceDocumentKind"], "particle")

    def test_builds_six_source_backed_layers_without_particle_output(self) -> None:
        source_elements = []
        layers = []
        roles = (
            "white-echo",
            "core-01",
            "core-02",
            "core-03",
            "core-04",
        )
        for index, role in enumerate(roles):
            element_id = f"source.mesh.{index}"
            source_elements.append(
                _source_element(
                    element_id,
                    "convertibleMeshEmitter",
                    start_delay=1.25,
                )
            )
            layers.append(_reviewed_layer(source_elements[-1], role))
        source_elements.append(
            _source_element(
                "source.sprite",
                "convertibleSpriteEmitter",
                start_delay=1.25,
            )
        )
        layers.append(_reviewed_layer(source_elements[-1], "sprite"))
        source = _source_document(source_elements)
        source_before = copy.deepcopy(source)
        result = materializer.build_stage_document(
            source,
            _ready_manifest(),
            _stage(layers),
            -90.0,
        )

        self.assertEqual(len(result["elements"]), 6)
        self.assertEqual(
            [element["kind"] for element in result["elements"]],
            ["mesh", "mesh", "mesh", "mesh", "mesh", "sprite"],
        )
        self.assertFalse(any(e["kind"] == "particle" for e in result["elements"]))
        self.assertTrue(
            all(not e["sourceRecipe"]["enabled"] for e in result["elements"])
        )
        self.assertEqual(
            result["elements"][0]["material"], source["elements"][0]["material"]
        )
        self.assertEqual(
            result["elements"][0]["resources"], source["elements"][0]["resources"]
        )
        self.assertEqual(
            result["elements"][0]["actionCueAttachment"]["runtimeAnchorSlotId"],
            "root",
        )
        self.assertFalse(result["elements"][0]["actionCueAttachment"]["follow"])
        self.assertIn(
            "anchor-policy:authored-root-snapshot",
            result["elements"][0]["sourceNode"],
        )
        self.assertEqual(
            result["elements"][0]["detail"]["transform"],
            source["elements"][0]["detail"]["transform"],
        )
        self.assertAlmostEqual(
            result["elements"][0]["detail"]["timing"]["startDelaySeconds"],
            0.25,
        )
        self.assertEqual(
            result["elements"][-1]["detail"]["sprite"][
                "billboardRollDegrees"
            ],
            -90.0,
        )
        self.assertEqual(source, source_before)

    def test_per_element_sprite_roll_overrides_default(self) -> None:
        source = _source_document(
            [_source_element("source.sprite", "convertibleSpriteEmitter")]
        )
        layers = [_reviewed_layer(source["elements"][0], "sprite")]
        layers[0]["spriteBillboardRollDegrees"] = 17.5
        result = materializer.build_stage_document(
            source, _ready_manifest(), _stage(layers), -90.0
        )
        self.assertEqual(
            result["elements"][0]["detail"]["sprite"][
                "billboardRollDegrees"
            ],
            17.5,
        )

    def test_stage_owned_role_name_and_layer_count_are_not_dimensionmaster_shaped(
        self,
    ) -> None:
        source_elements = []
        layers = []
        expected_kinds = []
        for index in range(7):
            classification = (
                "convertibleMeshEmitter"
                if index % 2 == 0
                else "convertibleSpriteEmitter"
            )
            source_element_id = f"source.stage-owned.{index}"
            source_elements.append(
                _source_element(source_element_id, classification)
            )
            layers.append(
                _reviewed_layer(
                    source_elements[-1], f"stage-layer-{index:02d}"
                )
            )
            expected_kinds.append("mesh" if index % 2 == 0 else "sprite")

        result = materializer.build_stage_document(
            _source_document(source_elements),
            _ready_manifest(),
            _stage(layers),
            -90.0,
        )

        self.assertEqual(len(result["elements"]), 7)
        self.assertEqual(
            [element["kind"] for element in result["elements"]], expected_kinds
        )
        self.assertEqual(
            [element["id"].rsplit(".", 1)[1] for element in result["elements"]],
            [f"stage-layer-{index:02d}" for index in range(7)],
        )

    def test_ready_stage_rejects_source_from_later_timeline_window(self) -> None:
        later = _source_element(
            "source.later-window", "convertibleMeshEmitter", start_delay=1.25
        )
        second = _source_element(
            "source.second-stage", "convertibleMeshEmitter", start_delay=1.5
        )
        source = _source_document([later, second])
        stages = [
            _stage_for_index(later, 0, 0.0),
            _stage_for_index(second, 1, 1.0),
        ]

        with self.assertRaisesRegex(ValueError, "outside its stage timeline window"):
            _evaluate_ready_in_temporary_data_root(source, stages)

    def test_ready_stages_reject_cross_stage_source_element_reuse(self) -> None:
        source_element = _source_element(
            "source.reused", "convertibleMeshEmitter", start_delay=0.25
        )
        source = _source_document([source_element])
        stages = [
            _stage_for_index(source_element, 0, 0.0),
            _stage_for_index(source_element, 1, 1.0),
        ]

        with self.assertRaisesRegex(
            ValueError, "reuse a source Element without explicit reuse provenance"
        ):
            _evaluate_ready_in_temporary_data_root(source, stages)

    def test_ready_stages_accept_distinct_interval_owned_elements(self) -> None:
        first = _source_element(
            "source.first-window", "convertibleMeshEmitter", start_delay=0.25
        )
        second = _source_element(
            "source.second-window", "convertibleMeshEmitter", start_delay=1.25
        )
        source = _source_document([first, second])
        stages = [
            _stage_for_index(first, 0, 0.0),
            _stage_for_index(second, 1, 1.0),
        ]

        status, outputs = _evaluate_ready_in_temporary_data_root(source, stages)
        self.assertEqual(status["materializedTargetCount"], 2)
        self.assertEqual(len(outputs), 2)
        self.assertEqual(
            [
                document["elements"][0]["detail"]["timing"][
                    "startDelaySeconds"
                ]
                for _, document in outputs
            ],
            [0.25, 0.25],
        )

    def test_diagnostic_admits_certified_cascade_mesh_as_target_mesh(self) -> None:
        source = _source_document(
            [_source_element("source.certified.mesh", "convertibleMeshEmitter")]
        )
        source["elements"][0].pop("actionCueAttachment")
        diagnostic = materializer._diagnose_source_document(
            source,
            {"effectAssetId": "effect.test.skill.1"},
            [{"stageIndex": 0, "sourceTimelineOffsetSeconds": 1.0}],
        )

        row = diagnostic["sourceElements"][0]
        self.assertEqual(row["sourceClassification"], materializer.MESH_PARTICLE)
        self.assertEqual(row["sourceKind"], materializer.MESH_PARTICLE)
        self.assertEqual(row["sourceDocumentKind"], "particle")
        self.assertEqual(
            row["conversionEligibility"]["status"], materializer.CERTIFIED
        )
        self.assertEqual(row["targetKindCandidates"], ["mesh"])
        self.assertEqual(row["admittedTargetKind"], "mesh")
        self.assertEqual(row["productAdmission"], "ADMITTED")
        self.assertEqual(
            row["requiredTargetAnchorPolicy"], "AUTHORED_ROOT_SNAPSHOT"
        )
        self.assertEqual(diagnostic["fullyAdmittedStandaloneCount"], 1)
        self.assertEqual(diagnostic["fullyAdmittedSpriteParticleCount"], 0)

    def test_optional_missing_layers_are_not_fabricated(self) -> None:
        source = _source_document(
            [_source_element("source.core", "convertibleMeshEmitter")]
        )
        result = materializer.build_stage_document(
            source,
            _ready_manifest(),
            _stage(
                [
                    _reviewed_layer(source["elements"][0], "core-01")
                ]
            ),
            -90.0,
        )
        self.assertEqual(len(result["elements"]), 1)
        self.assertEqual(result["elements"][0]["id"], "test.ba1.hit01.core-01")

    def test_mesh_particle_cannot_be_promoted_by_manifest_claim(self) -> None:
        source = _source_document(
            [_source_element("source.mesh.particle", materializer.MESH_PARTICLE)]
        )
        layers = [_reviewed_layer(source["elements"][0], "core-01")]
        with self.assertRaisesRegex(ValueError, "no admitted product target"):
            materializer.build_stage_document(
                source, _ready_manifest(), _stage(layers), -90.0
            )

    def test_sprite_particle_preserves_renderer_recipe_and_authored_roll(self) -> None:
        source = _source_document(
            [_source_element("source.sprite.particle", materializer.SPRITE_PARTICLE)]
        )
        source_attachment = source["elements"][0]["actionCueAttachment"]
        source_attachment["follow"] = True
        source_attachment["sourceAnchorSlotId"] = "weapon-trail"
        source_attachment["runtimeAnchorSlotId"] = "bone"
        source_attachment["runtimeBoneName"] = "bip001-r-hand"
        source_attachment["socketLocalTransform"] = {
            "position": [0.25, 0.5, 0.75],
            "rotationDegrees": [10.0, 20.0, 30.0],
            "scale": [0.5, 0.75, 1.25],
        }
        source_before = copy.deepcopy(source)
        layers = [_reviewed_layer(source["elements"][0], "ambient-streaks")]
        result = materializer.build_stage_document(
            source, _ready_manifest(), _stage(layers), -90.0
        )

        self.assertEqual(result["elements"][0]["kind"], "particle")
        self.assertEqual(
            result["elements"][0]["sourceRecipe"],
            source["elements"][0]["sourceRecipe"],
        )
        self.assertEqual(
            result["elements"][0]["sourcePresentation"],
            source["elements"][0]["sourcePresentation"],
        )
        self.assertEqual(
            result["elements"][0]["actionCueAttachment"], source_attachment
        )
        self.assertIn(
            "anchor-policy:source-preserved",
            result["elements"][0]["sourceNode"],
        )
        self.assertEqual(
            result["elements"][0]["detail"]["sprite"][
                "billboardRollDegrees"
            ],
            -90.0,
        )
        self.assertEqual(source, source_before)

    def test_ready_layer_requires_exact_element_evidence_and_module_disposition(
        self,
    ) -> None:
        source = _source_document(
            [_source_element("source.reviewed", "convertibleMeshEmitter")]
        )
        layer = _reviewed_layer(source["elements"][0], "reviewed-layer")
        cases = {
            "source Element evidence drifted": ("sourceElementSha256", "0" * 64),
            "sourceDocumentKind drifted": ("sourceDocumentKind", "mesh"),
            "sourceKind drifted": ("sourceKind", materializer.STANDALONE_MESH),
            "fidelity warning evidence drifted": (
                "fidelityWarningCodes",
                ["invented-fidelity-warning"],
            ),
            "removed-module disposition drifted": (
                "removedModuleDisposition",
                materializer.MODULES_NOT_APPLICABLE,
            ),
            "targetKind is not certified": ("targetKind", "sprite"),
        }
        for expected_error, (field, value) in cases.items():
            candidate = copy.deepcopy(layer)
            candidate[field] = value
            with self.subTest(field=field), self.assertRaisesRegex(
                ValueError, expected_error
            ):
                materializer.build_stage_document(
                    source,
                    _ready_manifest(),
                    _stage([candidate]),
                    -90.0,
                )

    def test_ready_conversion_dispositions_every_source_module(self) -> None:
        source_element = _source_element(
            "source.module-dispositions", "convertibleMeshEmitter"
        )
        source_element["sourceRecipe"]["modules"].append(
            {
                "stableId": "test:camera-offset",
                "className": "particlemodulecameraoffset",
                "objectPath": "Test.CameraOffset",
                "literals": [],
                "distributions": [],
            }
        )
        source = _source_document([source_element])
        layer = _reviewed_layer(source_element, "reviewed-layer")

        missing_disposition = copy.deepcopy(layer)
        missing_disposition["removedModuleDispositions"].pop()
        with self.assertRaisesRegex(ValueError, "disposition every removed"):
            materializer.build_stage_document(
                source,
                _ready_manifest(),
                _stage([missing_disposition]),
                -90.0,
            )

        warning_drift = copy.deepcopy(layer)
        warning_drift["fidelityWarnings"][0]["sourceModuleStableId"] = (
            "test:different-module"
        )
        with self.assertRaisesRegex(ValueError, "per-module fidelity evidence"):
            materializer.build_stage_document(
                source,
                _ready_manifest(),
                _stage([warning_drift]),
                -90.0,
            )

    def test_root_snapshot_is_explicit_authored_policy(self) -> None:
        source = _source_document(
            [_source_element("source.anchor-policy", "convertibleMeshEmitter")]
        )
        stage = _stage(
            [_reviewed_layer(source["elements"][0], "anchor-policy-layer")]
        )
        stage["occurrences"][0]["authoredAnchorPolicy"]["provenance"] = (
            "SOURCE_EXACT"
        )
        with self.assertRaisesRegex(ValueError, "AUTHORED_POLICY"):
            materializer.build_stage_document(
                source, _ready_manifest(), stage, -90.0
            )

    def test_authored_root_snapshot_replaces_missing_source_attachment(self) -> None:
        source = _source_document(
            [_source_element("source.authored-anchor", "convertibleMeshEmitter")]
        )
        source["elements"][0].pop("actionCueAttachment")
        result = materializer.build_stage_document(
            source,
            _ready_manifest(),
            _stage([_reviewed_layer(source["elements"][0], "authored-anchor")]),
            -90.0,
        )

        attachment = result["elements"][0]["actionCueAttachment"]
        self.assertTrue(attachment["enabled"])
        self.assertFalse(attachment["follow"])
        self.assertEqual(attachment["sourceAnchorSlotId"], "root")
        self.assertEqual(attachment["runtimeAnchorSlotId"], "root")
        self.assertEqual(attachment["runtimeBoneName"], "")
        self.assertEqual(
            attachment["socketLocalTransform"],
            _stage([])["occurrences"][0]["authoredAnchorPolicy"][
                "socketLocalTransform"
            ],
        )
        self.assertAlmostEqual(
            result["elements"][0]["detail"]["timing"]["startDelaySeconds"],
            0.25,
        )

    def test_unresolved_material_resource_or_anchor_fails_closed(self) -> None:
        cases = {}

        missing_resource = _source_element("missing.resource", "convertibleMeshEmitter")
        missing_resource["resources"][0]["assetId"] = "Effect/Missing/nope.wmodel"
        cases["resource"] = missing_resource

        unresolved_material = _source_element(
            "unresolved.material", "convertibleMeshEmitter"
        )
        unresolved_material["material"]["sourceProfile"]["enabled"] = False
        cases["material"] = unresolved_material

        fallback_blocked_material = _source_element(
            "fallback.blocked.material", "convertibleMeshEmitter"
        )
        fallback_blocked_material["material"]["sourceProfile"][
            "runtimeShaderProfileId"
        ] = "effect.ue3.fallback-blocked.v1"
        cases["fallback-blocked-material"] = fallback_blocked_material

        unresolved_anchor = _source_element(
            "unresolved.anchor", materializer.SPRITE_PARTICLE
        )
        unresolved_anchor["actionCueAttachment"]["enabled"] = False
        cases["anchor"] = unresolved_anchor

        for reason, element in cases.items():
            with self.subTest(reason=reason), self.assertRaises(ValueError):
                materializer.build_stage_document(
                    _source_document([element]),
                    _ready_manifest(),
                    _stage(
                        [
                            _reviewed_layer(element, f"{reason}-carrier")
                        ]
                    ),
                    -90.0,
                )

    def test_repository_set_is_one_preserved_and_three_blocked(self) -> None:
        protected_path = REPOSITORY_ROOT / (
            "Data/Effects/Authored/"
            "effect.dimensionmaster.skill.2050210.authored-baseline.effect.json"
        )
        before_hash = hashlib.sha256(protected_path.read_bytes()).hexdigest()
        status, outputs = materializer.materialize_set()
        after_hash = hashlib.sha256(protected_path.read_bytes()).hexdigest()

        self.assertEqual(before_hash, after_hash)
        self.assertEqual(outputs, [])
        self.assertEqual(
            status["summary"],
            {
                "skillCount": 4,
                "readyCount": 0,
                "blockedCount": 3,
                "preservedCount": 1,
                "pendingOutputCount": 0,
            },
        )
        self.assertEqual(
            [skill["targetCount"] for skill in status["skills"]],
            [1, 4, 4, 3],
        )
        self.assertEqual(
            status["skills"][0]["productGates"][0]["status"], "passed"
        )
        self.assertEqual(
            status["skills"][0]["productGates"][0]["kindCounts"],
            {"mesh": 20, "sprite": 4, "particle": 0},
        )
        self.assertEqual(
            [
                len(skill.get("externalApproximations", []))
                for skill in status["skills"]
            ],
            [0, 4, 4, 3],
        )
        self.assertTrue(
            all(
                row["status"] == "externalApproximationPresent"
                and row["strictAdmissionStatus"] == "blocked"
                for skill in status["skills"]
                for row in skill.get("externalApproximations", [])
            )
        )
        lance = status["skills"][1]["sourceDiagnostics"]
        self.assertEqual(
            lance["classificationCounts"],
            {
                "standaloneMesh": 0,
                "meshParticle": 5,
                "standaloneSprite": 0,
                "spriteParticle": 16,
                "unsupported": 4,
            },
        )
        self.assertEqual(
            [stage["fullyAdmittedStandaloneCount"] for stage in lance["stages"]],
            [0, 0, 0, 4],
        )
        self.assertEqual(lance["fullyAdmittedStandaloneCount"], 4)
        artist = status["skills"][2]["sourceDiagnostics"]
        self.assertEqual(
            artist["classificationCounts"],
            {
                "standaloneMesh": 0,
                "meshParticle": 11,
                "standaloneSprite": 0,
                "spriteParticle": 16,
                "unsupported": 6,
            },
        )
        self.assertEqual(
            [stage["fullyAdmittedStandaloneCount"] for stage in artist["stages"]],
            [2, 0, 0, 2],
        )
        self.assertEqual(artist["fullyAdmittedStandaloneCount"], 4)
        self.assertEqual(len(lance["sourceElements"]), lance["elementCount"])
        self.assertTrue(
            all(
                {
                    "sourceDocumentKind",
                    "sourceKind",
                    "conversionEligibility",
                    "fidelityWarningCodes",
                    "fidelityWarnings",
                    "targetKindCandidates",
                    "requiredTargetAnchorPolicy",
                    "requiredRemovedModuleDisposition",
                }
                <= set(row)
                for row in lance["sourceElements"]
            )
        )
        warlord = status["skills"][3]["sourceDiagnostics"]
        self.assertEqual(
            warlord["classificationCounts"],
            {
                "standaloneMesh": 0,
                "meshParticle": 10,
                "standaloneSprite": 0,
                "spriteParticle": 18,
                "unsupported": 3,
            },
        )
        self.assertEqual(
            [stage["fullyAdmittedStandaloneCount"] for stage in warlord["stages"]],
            [0, 0, 0],
        )
        self.assertEqual(
            sum(
                row["conversionEligibility"]["status"] == materializer.CERTIFIED
                for row in lance["sourceElements"]
            ),
            6,
        )
        self.assertEqual(
            sum(
                row["conversionEligibility"]["status"] == materializer.CERTIFIED
                for row in artist["sourceElements"]
            ),
            5,
        )
        self.assertEqual(
            sum(
                row["conversionEligibility"]["status"] == materializer.CERTIFIED
                for row in warlord["sourceElements"]
            ),
            1,
        )

    def test_preserve_existing_requires_the_separate_product_gate(self) -> None:
        manifest_path = REPOSITORY_ROOT / (
            "Data/Effects/AuthoredCorrections/DimensionMaster/"
            "effect.dimensionmaster.skill.2050210.authored-baseline.materialization.json"
        )
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        manifest["stages"][0].pop("productGate")
        with self.assertRaisesRegex(ValueError, "productGate"):
            materializer.evaluate_skill_manifest(manifest, -90.0)

    def test_evidence_hash_drift_is_rejected(self) -> None:
        manifest_path = REPOSITORY_ROOT / (
            "Data/Effects/AuthoredCorrections/LanceMaster/"
            "effect.lancemaster.skill.34010.materialization.json"
        )
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        manifest["evidence"][0]["sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "Evidence changed"):
            materializer.evaluate_skill_manifest(manifest, -90.0)

    def test_stage_order_drift_is_rejected(self) -> None:
        manifest_path = REPOSITORY_ROOT / (
            "Data/Effects/AuthoredCorrections/LanceMaster/"
            "effect.lancemaster.skill.34010.materialization.json"
        )
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        manifest["stages"] = list(reversed(manifest["stages"]))
        with self.assertRaisesRegex(ValueError, "Stage order"):
            materializer.evaluate_skill_manifest(manifest, -90.0)

    def test_ready_source_hash_drift_is_rejected_before_materialization(self) -> None:
        manifest_path = REPOSITORY_ROOT / (
            "Data/Effects/AuthoredCorrections/DimensionMaster/"
            "effect.dimensionmaster.skill.2050210.authored-baseline.materialization.json"
        )
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        manifest["status"] = "ready"
        manifest["source"]["sha256"] = "0" * 64
        manifest["stages"][0]["status"] = "ready"
        with self.assertRaisesRegex(ValueError, "Canonical source changed"):
            materializer.evaluate_skill_manifest(manifest, -90.0)

    def test_writer_refuses_to_overwrite_authored_output(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            output_path = Path(temporary_directory) / "existing.effect.json"
            output_path.write_text("existing\n", encoding="utf-8")
            with self.assertRaises(FileExistsError):
                materializer.write_documents([(output_path, {"new": True})])
            self.assertEqual(
                output_path.read_text(encoding="utf-8"), "existing\n"
            )

    def test_blocked_stage_accepts_only_hash_matching_external_approximation(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            data_root = Path(temporary_directory) / "Data"
            authored_root = data_root / "Effects/Authored"
            correction_root = data_root / "Effects/AuthoredCorrections"
            authored_root.mkdir(parents=True)
            target_id = "effect.lancemaster.skill.34010.ba1"
            target_path = authored_root / f"{target_id}.effect.json"
            document = {
                "schema": "lostark.effect-authoring",
                "version": 12,
                "effectAssetId": target_id,
                "displayName": target_id,
                "particleSystem": {},
                "modelCues": [],
                "elements": [
                    {
                        "id": "mesh.1",
                        "kind": "mesh",
                        "sourceRecipe": {"enabled": False, "modules": []},
                    }
                ],
            }
            target_path.write_text(
                json.dumps(document, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            receipt_path = (
                correction_root
                / "Generated/LanceMaster"
                / f"{target_id}.approximation-receipt.json"
            )
            receipt_path.parent.mkdir(parents=True)
            receipt = {
                "schema": materializer.EXTERNAL_APPROXIMATION_SCHEMA,
                "version": materializer.EXTERNAL_APPROXIMATION_VERSION,
                "targetEffectAssetId": target_id,
                "targetAuthoringPath": f"Effects/Authored/{target_id}.effect.json",
                "characterClass": "LANCE_MASTER",
                "productSkillId": 34010,
                "stageIndex": 0,
                "selectionPolicy": {
                    "particleOutputAllowed": False,
                    "genericPlaceholderAllowed": False,
                    "crossSkillBorrowingAllowed": False,
                },
                "output": {
                    "elementCount": 1,
                    "particleCount": 0,
                    "documentSha256": materializer._canonical_json_sha256(document),
                },
            }
            receipt_path.write_text(
                json.dumps(receipt, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            with (
                patch.object(materializer, "DATA_ROOT", data_root.resolve()),
                patch.object(materializer, "AUTHORED_ROOT", authored_root.resolve()),
                patch.object(
                    materializer, "CORRECTION_ROOT", correction_root.resolve()
                ),
            ):
                diagnostic = materializer._validate_external_authored_approximation(
                    character_class="LANCE_MASTER",
                    skill_id=34010,
                    stage_index=0,
                    target_effect_id=target_id,
                    target_path=target_path,
                )
                self.assertEqual("externalApproximationPresent", diagnostic["status"])

                receipt_path.unlink()
                with self.assertRaisesRegex(ValueError, "no matching"):
                    materializer._validate_external_authored_approximation(
                        character_class="LANCE_MASTER",
                        skill_id=34010,
                        stage_index=0,
                        target_effect_id=target_id,
                        target_path=target_path,
                    )
                receipt["output"]["documentSha256"] = "0" * 64
                receipt_path.write_text(
                    json.dumps(receipt, ensure_ascii=False, indent=2) + "\n",
                    encoding="utf-8",
                )
                with self.assertRaisesRegex(ValueError, "hash drifted"):
                    materializer._validate_external_authored_approximation(
                        character_class="LANCE_MASTER",
                        skill_id=34010,
                        stage_index=0,
                        target_effect_id=target_id,
                        target_path=target_path,
                    )


if __name__ == "__main__":
    unittest.main()
