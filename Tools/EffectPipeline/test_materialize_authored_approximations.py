import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

import materialize_authored_approximations as materializer


def _json_sha256(value):
    return hashlib.sha256(
        json.dumps(
            value,
            ensure_ascii=False,
            sort_keys=True,
            separators=(",", ":"),
        ).encode("utf-8")
    ).hexdigest()


class AuthoredApproximationMaterializerTests(unittest.TestCase):
    def setUp(self):
        self.temporary_directory = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary_directory.name)
        self.data_root = self.root / "Data"
        self.imported_root = self.data_root / "Effects/Imported"
        self.authored_root = self.data_root / "Effects/Authored"
        self.correction_root = self.data_root / "Effects/AuthoredCorrections"
        self.receipt_root = self.correction_root / "Generated"
        self.resources_root = self.root / "Client/Bin/Resources"
        for path in (
            self.imported_root,
            self.authored_root,
            self.correction_root,
            self.resources_root,
        ):
            path.mkdir(parents=True, exist_ok=True)
        self.model_asset_id = "Effect/Test/Meshes/slash.wmodel"
        self.texture_asset_id = "Effect/Test/Textures/slash.dds"
        self._write_resource(self.model_asset_id)
        self._write_resource(self.texture_asset_id)
        self.patch = mock.patch.multiple(
            materializer,
            DATA_ROOT=self.data_root.resolve(),
            IMPORTED_ROOT=self.imported_root.resolve(),
            AUTHORED_ROOT=self.authored_root.resolve(),
            CORRECTION_ROOT=self.correction_root.resolve(),
            RECEIPT_ROOT=self.receipt_root.resolve(),
            RESOURCES_ROOT=self.resources_root.resolve(),
        )
        self.patch.start()

    def tearDown(self):
        self.patch.stop()
        self.temporary_directory.cleanup()

    def _write_resource(self, asset_id):
        path = self.resources_root / asset_id
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(b"runtime-resource")

    def _fixture_model_info_evidence(self, model_resource):
        asset_id = model_resource["assetId"]
        return {
            "decision": "embeddedFirstBaseTextureAbsent",
            "modelAssetId": asset_id,
            "modelResourceSha256": hashlib.sha256(
                (self.resources_root / asset_id).read_bytes()
            ).hexdigest(),
            "toolPath": "Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe",
            "toolSha256": "a" * 64,
            "command": "info",
            "normalizedOutputSha256": "b" * 64,
            "sectionCount": 2,
            "animationCount": 0,
            "skeletonPresent": False,
            "materialVersion": 2,
            "materialCount": 1,
            "firstMaterialName": "dummy_material_0",
            "firstBaseTexture": "",
            "embeddedFirstBaseTexturePresent": False,
        }

    def _fixed_distribution(self, property_path, values):
        component_count = len(values)
        return {
            "propertyPath": property_path,
            "operation": 1,
            "sourceClass": "DistributionFloatConstant",
            "sourceObjectPath": "fixture.distribution",
            "componentCount": component_count,
            "randomLockAxes": 0,
            "lookupTableChunkSize": component_count,
            "lookupTableNumElements": 1,
            "lookupTableTimeScale": 0.0,
            "lookupTableStartTime": 0.0,
            "defaultMinimum": list(values) + [0.0] * (4 - component_count),
            "defaultMaximum": list(values) + [0.0] * (4 - component_count),
            "lookupTable": [0.0, 0.0] + list(values),
            "keys": [],
        }

    def _vector_distribution(
        self, property_path, first, *, operation=1, maximum=None, last=None
    ):
        first = list(first)
        maximum = list(maximum if maximum is not None else first)
        component_count = len(first)
        chunk_size = component_count * (2 if operation >= 2 else 1)
        first_chunk = first + (maximum if operation >= 2 else [])
        chunks = [first_chunk]
        if last is not None:
            last = list(last)
            chunks.append(last + (last if operation >= 2 else []))
        return {
            "propertyPath": property_path,
            "operation": operation,
            "sourceClass": "DistributionVectorFixture",
            "sourceObjectPath": "fixture.vector-distribution",
            "componentCount": component_count,
            "randomLockAxes": 0,
            "lookupTableChunkSize": chunk_size,
            "lookupTableNumElements": 2 if operation >= 2 else 1,
            "lookupTableTimeScale": 1.0 if len(chunks) > 1 else 0.0,
            "lookupTableStartTime": 0.0,
            "defaultMinimum": first + [0.0] * (4 - component_count),
            "defaultMaximum": maximum + [0.0] * (4 - component_count),
            "lookupTable": [0.0, 0.0] + [value for chunk in chunks for value in chunk],
            "keys": [],
        }

    def _particle_recipe(self, renderer, *, unknown=False):
        modules = [
            {
                "stableId": "module.required",
                "className": "ParticleModuleRequired",
                "literals": [],
                "distributions": [],
            },
            {
                "stableId": "module.spawn",
                "className": "ParticleModuleSpawn",
                "literals": [],
                "distributions": (
                    []
                    if unknown
                    else [self._fixed_distribution("SpawnRate", [0.0])]
                ),
            },
            {
                "stableId": "module.velocity",
                "className": "ParticleModuleVelocity",
                "literals": [],
                "distributions": [
                    self._fixed_distribution("StartVelocity", [1.0, 0.0, 0.0])
                ],
            },
            {
                "stableId": "module.size",
                "className": "ParticleModuleSize",
                "literals": [],
                "distributions": [
                    self._fixed_distribution("StartSize", [1.0, 1.0, 1.0])
                ],
            },
        ]
        return {
            "enabled": True,
            "rendererShape": renderer,
            "emitterDelaySeconds": 0.0,
            "emitterDurationSeconds": 1.0,
            "emitterLoopCount": 1,
            "bursts": [
                {"timeSeconds": 0.0, "countMinimum": 1, "countMaximum": 1}
            ],
            "modules": modules,
        }

    def _material(self, *, fallback=False, texture_asset_id=None):
        texture_asset_id = texture_asset_id or self.texture_asset_id
        return {
            "templateId": "effect.source_material",
            "sourceMaterialPath": "fx_test.mi_slash",
            "renderProfile": "alpha_one_sided_depth_read",
            "sourceProfile": {
                "enabled": True,
                "profileId": "ue3.material.fx.test.slash",
                "runtimeShaderProfileId": (
                    materializer.FALLBACK_BLOCKED_RUNTIME_PROFILE
                    if fallback
                    else materializer.STANDARD_MATERIAL_RUNTIME_PROFILE
                ),
                "parentMaterialPath": "fx_test.m_slash",
                "semanticStatus": "reconstructed_profile",
                "textures": [
                    {
                        "name": "diffuse",
                        "sourceObjectPath": "fx_test.t_slash",
                        "assetId": texture_asset_id,
                        "addressU": "wrap",
                        "addressV": "wrap",
                        "colorSpace": "srgb",
                        "samplingEvidence": "fixture",
                        "group": "diffuse",
                    }
                ],
                "scalars": [],
                "vectors": [],
                "staticSwitches": [],
                "dynamicParameterSemantics": [],
                "subUVMode": "none",
            },
        }

    def _element(
        self,
        element_id,
        renderer,
        *,
        order=0,
        delay=0.25,
        event_id="source-event-001",
        fallback=False,
        unknown=False,
        missing_resource=False,
    ):
        missing_asset_id = "Effect/Test/Missing/missing.dds"
        resources = (
            [
                {"slotId": "meshModel", "assetId": self.model_asset_id},
                {
                    "slotId": "base",
                    "assetId": (
                        missing_asset_id if missing_resource else self.texture_asset_id
                    ),
                },
            ]
            if renderer == "mesh"
            else [
                {
                    "slotId": "base",
                    "assetId": (
                        missing_asset_id if missing_resource else self.texture_asset_id
                    ),
                }
            ]
        )
        return {
            "id": element_id,
            "displayName": element_id,
            "groupId": "source",
            "sourceNode": f"source:{element_id}",
            "visible": True,
            "kind": "particle",
            "resources": resources,
            "material": self._material(
                fallback=fallback,
                texture_asset_id=(
                    missing_asset_id if missing_resource else self.texture_asset_id
                ),
            ),
            "actionCueAttachment": {
                "enabled": False,
                "follow": False,
                "sourceAnchorSlotId": "",
                "runtimeAnchorSlotId": "",
                "runtimeBoneName": "",
                "socketLocalTransform": {
                    "position": [0.0, 0.0, 0.0],
                    "rotationDegrees": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
            },
            "detail": {
                "transform": {
                    "position": [0.0, 0.0, 1.0],
                    "rotationDegrees": [0.0, 0.0, 0.0],
                    "revolutionDegreesPerSecond": [0.0, 0.0, 0.0],
                    "scale": [float(order + 1), 1.0, 1.0],
                    "velocityPerSecond": [0.0, 0.0, 0.0],
                },
                "color": {
                    "offset": [0.0, 0.0, 0.0, 0.0],
                    "multiply": [1.0, 1.0, 1.0, 1.0],
                    "clip": 0.0,
                    "emissiveIntensity": float(order + 1),
                    "distortionIntensity": 0.0,
                    "distortionOnBaseMaterial": False,
                    "radialTime": 0.0,
                    "radialIntensity": 0.0,
                },
                "timing": {
                    "startDelaySeconds": delay,
                    "lifeTimeSeconds": 0.3,
                    "afterImageSeconds": 0.0,
                    "dissolveStartNormalized": 1.0,
                },
                "linearLerp": {
                    "position": False,
                    "endPosition": [0.0, 0.0, 0.0],
                    "rotation": False,
                    "endRotationDegrees": [0.0, 0.0, 0.0],
                    "revolution": False,
                    "endRevolutionDegreesPerSecond": [0.0, 0.0, 0.0],
                    "scale": False,
                    "endScale": [1.0, 1.0, 1.0],
                    "velocity": False,
                    "endVelocityPerSecond": [0.0, 0.0, 0.0],
                },
                "particle": {
                    "maxParticles": 1,
                    "spawnRatePerSecond": 0.0,
                    "burstCount": 1,
                    "randomSeed": 1,
                    "lifeTimeSeconds": [0.3, 0.3],
                    "initialPositionMin": [0.0, 0.0, 0.0],
                    "initialPositionMax": [0.0, 0.0, 0.0],
                    "initialVelocityMin": [0.0, 0.0, 0.0],
                    "initialVelocityMax": [0.0, 0.0, 0.0],
                    "acceleration": [0.0, 0.0, 0.0],
                    "startSize": [1.0, 1.0],
                    "endSize": [1.0, 1.0],
                    "localSpace": True,
                    "billboard": renderer == "sprite",
                },
                "mesh": {"useModelMaterial": False},
                "sprite": {"billboard": renderer == "sprite", "billboardRollDegrees": 0.0},
            },
            "sourceRecipe": self._particle_recipe(renderer, unknown=unknown),
            "sourcePresentation": {
                "enabled": True,
                "schema": "lostark.effect-source-presentation",
                "version": 1,
                "profileId": "source",
                "status": "source_exact",
                "sourceObjectPath": element_id,
                "sourceActionCueId": "",
                "sourceEventId": event_id,
                "sourceOccurrenceIndex": 0,
                "sourceTimeSeconds": delay,
                "parameters": [],
            },
        }

    def _source_document(self, elements, effect_id="effect.dimensionmaster.skill.100.imported"):
        return {
            "schema": "lostark.effect-authoring",
            "version": 12,
            "effectAssetId": effect_id,
            "displayName": effect_id,
            "particleSystem": {
                "loop": False,
                "durationSeconds": 1.0,
                "timeScale": 1.0,
                "warmupSeconds": 0.0,
                "prewarm": False,
                "randomSeed": 1,
            },
            "modelCues": [],
            "elements": elements,
        }

    def _write_source(self, document, class_directory="DimensionMaster"):
        path = (
            self.imported_root
            / class_directory
            / "Converted"
            / f"{document['effectAssetId']}.effect.json"
        )
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(
            json.dumps(document, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        return path

    def _stage(self, source_path, source_effect_id, source_hash, *, skill_id=100):
        evidence_descriptors = {}
        for evidence_name in (
            "sourceReceipt",
            "externalModuleClosure",
            "conversionReceipt",
        ):
            evidence_path = (
                self.imported_root
                / "DimensionMaster"
                / "Evidence"
                / f"{skill_id}.{evidence_name}.json"
            )
            evidence_path.parent.mkdir(parents=True, exist_ok=True)
            evidence_path.write_text(
                json.dumps({"evidence": evidence_name}) + "\n", encoding="utf-8"
            )
            evidence_descriptors[evidence_name] = {
                "path": f"Data/{evidence_path.relative_to(self.data_root).as_posix()}",
                "sha256": hashlib.sha256(evidence_path.read_bytes()).hexdigest(),
            }
        return {
            "stageIndex": 0,
            "stageId": f"skill.{skill_id}.stage.0",
            "sourceSkillIds": [skill_id],
            "timelineOffsetSeconds": 0.0,
            "durationSeconds": 1.0,
            "status": "READY",
            "blockers": [],
            "clips": [
                {
                    "clip": "test_attack_01",
                    "stageClipIndex": 0,
                    "sourceSkillId": skill_id,
                    "sourceLine": 1,
                    "lengthSeconds": 1.0,
                    "timelineOffsetSeconds": 0.0,
                }
            ],
            "sourceEventIds": ["source-event-001"],
            "sourceArtifacts": [
                {
                    "sourceSkillId": skill_id,
                    "artifactOrigin": "TEST_SOURCE_TRUTH",
                    "sourceReceipt": evidence_descriptors["sourceReceipt"],
                    "normalizedGraph": {
                        "path": "C:/fixture/normalized-graph.json",
                        "sha256": "1" * 64,
                    },
                    "externalModuleClosure": evidence_descriptors[
                        "externalModuleClosure"
                    ],
                    "importedDocument": {
                        "path": source_path.relative_to(self.data_root).as_posix(),
                        "sha256": source_hash,
                        "effectAssetId": source_effect_id,
                    },
                    "conversionReceipt": evidence_descriptors["conversionReceipt"],
                }
            ],
        }

    def _write_manifest(self, stage, *, skill_id=100, character_class="DIMENSIONMASTER"):
        class_directory = materializer.CLASS_TOKENS[character_class][1]
        manifest = {
            "schema": materializer.INTAKE_SCHEMA,
            "version": materializer.INTAKE_VERSION,
            "characterClass": character_class,
            "consumerContract": {
                "outerCueRoot": "Player",
                "outerCueFollow": True,
                "innerSnapshotFollow": False,
            },
            "skills": [
                {
                    "productSkillId": skill_id,
                    "inputSlot": "Q",
                    "skillKind": "ACTIVE",
                    "sourceSkillIds": [skill_id],
                    "stages": [stage],
                }
            ],
        }
        path = (
            self.imported_root
            / class_directory
            / f"{class_directory}.combat-source-stage-manifest.json"
        )
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(
            json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        return path

    def _materialize(
        self,
        elements,
        *,
        effect_id="effect.dimensionmaster.skill.100.imported",
        skill_id=100,
    ):
        source = self._source_document(elements, effect_id)
        source_path = self._write_source(source)
        stage = self._stage(
            source_path,
            effect_id,
            hashlib.sha256(source_path.read_bytes()).hexdigest(),
            skill_id=skill_id,
        )
        manifest_path = self._write_manifest(stage, skill_id=skill_id)
        return materializer.materialize_manifests([manifest_path])

    def _reviewed_renderer_fixture(self, *, missing_resource_index=None):
        contract = materializer.REVIEWED_RENDERER_APPROXIMATION_STAGE_CONTRACTS[
            ("ARTIST", 31210, 3)
        ]
        elements = []
        event_ids = []
        for index, element_id in enumerate(contract["sourceElementIds"]):
            event_id = element_id.rsplit("event_", 1)[-1].replace("_", "-")
            event_ids.append(event_id)
            element = self._element(
                element_id,
                "sprite",
                order=index,
                delay=4.8001,
                event_id=event_id,
                fallback=True,
                missing_resource=(missing_resource_index == index),
            )
            element["sourceRecipe"]["bursts"] = [
                {"timeSeconds": 0.0, "countMinimum": 1, "countMaximum": 1},
                {"timeSeconds": 0.0, "countMinimum": 1, "countMaximum": 1},
            ]
            element["sourceRecipe"]["modules"][2]["distributions"][0][
                "operation"
            ] = 2
            element["sourceRecipe"]["modules"].append(
                {
                    "stableId": f"module.ribbon.{index}",
                    "className": "ParticleModuleTypeDataRibbon",
                    "literals": [],
                    "distributions": [],
                }
            )
            self.assertEqual(
                list(contract["requiredReasonCodes"]),
                materializer.strict_materializer.analyze_source_element(element)[
                    "reasonCodes"
                ],
            )
            elements.append(element)
        effect_id = "effect.artist.skill.31210.imported"
        source = self._source_document(elements, effect_id)
        source_path = self._write_source(source, class_directory="Artist")
        source_hash = hashlib.sha256(source_path.read_bytes()).hexdigest()
        stage = self._stage(source_path, effect_id, source_hash, skill_id=31210)
        stage.update(
            {
                "stageIndex": 3,
                "stageId": "skill.31210.stage.3",
                "timelineOffsetSeconds": 4.8001,
                "durationSeconds": 0.5333,
                "status": "AVAILABLE_WITH_BLOCKERS",
                "blockers": [
                    {
                        "code": "IMPORTED_SOURCE_MATERIAL_RUNTIME_PENDING",
                        "count": 6,
                    },
                    {
                        "code": "IMPORTED_SOURCE_RECIPE_RUNTIME_PENDING",
                        "count": 6,
                    },
                ],
                "sourceEventIds": event_ids,
            }
        )
        stage["clips"][0].update(
            {
                "clip": contract["clip"],
                "timelineOffsetSeconds": 4.8001,
                "lengthSeconds": 0.5333,
            }
        )
        stage["rendererApproximationApproval"] = {
            "decision": "reviewedRendererApproximation",
            "sourceElementIds": list(contract["sourceElementIds"]),
            "sourceElementSha256": {
                element["id"]: materializer.strict_materializer.source_element_sha256(
                    element
                )
                for element in elements
            },
            "sourceDocumentSha256": source_hash,
            "sourceKind": contract["sourceKind"],
            "targetKind": contract["targetKind"],
            "requiredReasonCodes": list(contract["requiredReasonCodes"]),
            "rationale": "Exact fixture ribbon review.",
        }
        return stage

    def test_mesh_first_selection_is_bounded_deterministic_and_provenanced(self):
        elements = [
            self._element(f"mesh.{index}", "mesh", order=index)
            for index in range(7)
        ] + [
            self._element(f"sprite.{index}", "sprite", order=index)
            for index in range(5)
        ]
        first_status, first_outputs, first_receipts = self._materialize(elements)
        second_status, second_outputs, second_receipts = self._materialize(elements)
        self.assertEqual(first_status, second_status)
        self.assertEqual(first_outputs, second_outputs)
        self.assertEqual(first_receipts, second_receipts)
        self.assertEqual(1, first_status["summary"]["generatedStageCount"])
        document = first_outputs[0][1]
        receipt = first_receipts[0][1]
        self.assertEqual(5, sum(row["kind"] == "mesh" for row in document["elements"]))
        self.assertEqual(3, sum(row["kind"] == "sprite" for row in document["elements"]))
        self.assertTrue(all(row["kind"] != "particle" for row in document["elements"]))
        self.assertTrue(
            all(row["sourceRecipe"]["enabled"] is False for row in document["elements"])
        )
        sprites = [row for row in document["elements"] if row["kind"] == "sprite"]
        self.assertTrue(
            all(
                row["detail"]["sprite"]["billboardRollDegrees"] == -90.0
                for row in sprites
            )
        )
        selected = [
            candidate
            for candidate in receipt["occurrences"][0]["candidates"]
            if candidate.get("selectionDecision") == "selected"
        ]
        self.assertEqual(8, len(selected))
        for candidate in selected:
            self.assertEqual(
                "PARTICLE_REQUIRED", candidate["conversionEligibility"]
            )
            self.assertEqual(
                "reviewedStandaloneApproximation",
                candidate["conversionDecision"],
            )
            self.assertEqual(
                candidate["sourceModuleStableIds"],
                [
                    row["sourceModuleStableId"]
                    for row in candidate["removedModuleDispositions"]
                ],
            )
            self.assertTrue(
                all(
                    row["disposition"] == "acceptedApproximation"
                    and row["rationale"]
                    for row in candidate["removedModuleDispositions"]
                )
            )
            self.assertIn("selectionScore", candidate)
            self.assertIsInstance(candidate["sourceOrder"], int)

    def _force_budget_overflow(self, renderer_kind):
        count = 6 if renderer_kind == "mesh" else 4
        elements = [
            self._element(f"{renderer_kind}.{index}", renderer_kind, order=index)
            for index in range(count)
        ]
        _, outputs, receipts = self._materialize(elements)
        document = copy.deepcopy(outputs[0][1])
        receipt = copy.deepcopy(receipts[0][1])
        candidates = receipt["occurrences"][0]["candidates"]
        excluded = next(
            row for row in candidates if row.get("selectionDecision") == "budgetExcluded"
        )
        excluded["selectionDecision"] = "selected"
        template = copy.deepcopy(document["elements"][0])
        template["id"] = f"authored.approx.overflow.{renderer_kind}"
        template["displayName"] = template["id"]
        template["kind"] = renderer_kind
        if renderer_kind == "sprite":
            template["detail"]["sprite"]["billboard"] = True
            template["detail"]["sprite"]["billboardRollDegrees"] = -90.0
        excluded["targetElementId"] = template["id"]
        document["elements"].append(template)
        receipt["output"]["elementCount"] += 1
        receipt["output"][f"{renderer_kind}Count"] += 1
        receipt["output"]["documentSha256"] = _json_sha256(document)
        return document, receipt

    def test_validator_rejects_mesh_and_sprite_budget_overflow(self):
        for renderer_kind, expected_text in (
            ("mesh", "five-Mesh"),
            ("sprite", "three-Sprite"),
        ):
            with self.subTest(renderer_kind=renderer_kind):
                document, receipt = self._force_budget_overflow(renderer_kind)
                with self.assertRaisesRegex(ValueError, expected_text):
                    materializer.validate_materialized_stage(document, receipt)

    def test_empty_stage_is_rejected(self):
        status, outputs, receipts = self._materialize([])
        self.assertEqual(1, status["summary"]["blockedStageCount"])
        self.assertEqual([], outputs)
        self.assertEqual([], receipts)
        self.assertIn("no source Mesh/Sprite", status["stages"][0]["reason"])

    def test_unknown_conversion_is_rejected(self):
        status, outputs, _ = self._materialize(
            [self._element("mesh.unknown", "mesh", unknown=True)]
        )
        self.assertEqual([], outputs)
        self.assertEqual("blocked", status["stages"][0]["status"])
        self.assertIn("empty after UNKNOWN", status["stages"][0]["reason"])

    def test_missing_runtime_resource_is_rejected(self):
        status, outputs, _ = self._materialize(
            [self._element("mesh.missing", "mesh", missing_resource=True)]
        )
        self.assertEqual([], outputs)
        self.assertEqual("blocked", status["stages"][0]["status"])

    def test_source_document_hash_drift_is_rejected(self):
        source = self._source_document([self._element("mesh.0", "mesh")])
        source_path = self._write_source(source)
        source_hash = hashlib.sha256(source_path.read_bytes()).hexdigest()
        stage = self._stage(source_path, source["effectAssetId"], source_hash)
        manifest_path = self._write_manifest(stage)
        source["displayName"] = "changed-after-intake"
        source_path.write_text(json.dumps(source) + "\n", encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "changed after intake"):
            materializer.materialize_manifests([manifest_path])

    def test_stage_local_notify_timing_and_source_transform_are_preserved(self):
        source_element = self._element("mesh.stage-two", "mesh", delay=1.2)
        source_transform = copy.deepcopy(source_element["detail"]["transform"])
        source = self._source_document([source_element])
        source_path = self._write_source(source)
        stage = self._stage(
            source_path,
            source["effectAssetId"],
            hashlib.sha256(source_path.read_bytes()).hexdigest(),
        )
        stage["stageIndex"] = 1
        stage["stageId"] = "skill.100.stage.1"
        stage["timelineOffsetSeconds"] = 1.0
        stage["durationSeconds"] = 1.0
        stage["clips"][0]["timelineOffsetSeconds"] = 1.0
        output_path, document, _, receipt = materializer.build_stage_approximation(
            character_class="DIMENSIONMASTER",
            skill_id=100,
            stage=stage,
            stage_count=2,
            claimed_source_elements=set(),
        )
        self.assertEqual(
            "effect.dimensionmaster.skill.100.ba2.effect.json", output_path.name
        )
        self.assertAlmostEqual(
            0.2, document["elements"][0]["detail"]["timing"]["startDelaySeconds"]
        )
        expected_transform = copy.deepcopy(source_transform)
        expected_transform["scale"] = [0.01, 0.01, 0.01]
        self.assertEqual(
            expected_transform, document["elements"][0]["detail"]["transform"]
        )
        self.assertEqual(1.0, receipt["sourceTimeline"]["startSeconds"])
        self.assertEqual(2.0, receipt["sourceTimeline"]["endSeconds"])

    def test_source_element_cannot_be_borrowed_by_another_product_skill(self):
        source = self._source_document([self._element("mesh.shared", "mesh")])
        source_path = self._write_source(source)
        source_hash = hashlib.sha256(source_path.read_bytes()).hexdigest()
        first_stage = self._stage(
            source_path, source["effectAssetId"], source_hash, skill_id=100
        )
        claims = set()
        materializer.build_stage_approximation(
            character_class="DIMENSIONMASTER",
            skill_id=100,
            stage=first_stage,
            stage_count=1,
            claimed_source_elements=claims,
        )
        borrowed_stage = copy.deepcopy(first_stage)
        borrowed_stage["stageId"] = "skill.101.stage.0"
        with self.assertRaisesRegex(ValueError, "reused across stages"):
            materializer.build_stage_approximation(
                character_class="DIMENSIONMASTER",
                skill_id=101,
                stage=borrowed_stage,
                stage_count=1,
                claimed_source_elements=claims,
            )

    def test_incomplete_module_disposition_is_rejected(self):
        _, outputs, receipts = self._materialize(
            [self._element("mesh.0", "mesh")]
        )
        document = copy.deepcopy(outputs[0][1])
        receipt = copy.deepcopy(receipts[0][1])
        selected = next(
            row
            for row in receipt["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        selected["removedModuleDispositions"].pop()
        with self.assertRaisesRegex(ValueError, "cover every source module"):
            materializer.validate_materialized_stage(document, receipt)

    def test_fallback_material_requires_explicit_decision_and_preserves_raw_profile(self):
        _, outputs, receipts = self._materialize(
            [self._element("mesh.fallback", "mesh", fallback=True)]
        )
        document = outputs[0][1]
        receipt = receipts[0][1]
        selected = next(
            row
            for row in receipt["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        self.assertEqual(
            "acceptedStandardApproximation", selected["materialDecision"]
        )
        self.assertEqual(
            materializer.FALLBACK_BLOCKED_RUNTIME_PROFILE,
            selected["materialProvenance"]["sourceProfile"][
                "runtimeShaderProfileId"
            ],
        )
        self.assertEqual(
            {"enabled": False},
            document["elements"][0]["material"]["sourceProfile"],
        )
        self.assertEqual(
            "effect.standard", document["elements"][0]["material"]["templateId"]
        )
        broken_receipt = copy.deepcopy(receipt)
        broken_selected = next(
            row
            for row in broken_receipt["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        del broken_selected["materialDecision"]
        with self.assertRaisesRegex(ValueError, "materialDecision"):
            materializer.validate_materialized_stage(document, broken_receipt)

    def test_unbound_raw_material_parameter_requires_standard_approximation(self):
        element = self._element("mesh.unbound-parameter", "mesh")
        element["material"]["sourceProfile"]["textures"][0]["assetId"] = ""
        _, outputs, receipts = self._materialize([element])
        selected = next(
            row
            for row in receipts[0][1]["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        self.assertEqual(
            "acceptedStandardApproximation", selected["materialDecision"]
        )
        self.assertEqual(
            "",
            selected["materialProvenance"]["sourceProfile"]["textures"][0][
                "assetId"
            ],
        )
        self.assertEqual(
            {"enabled": False},
            outputs[0][1]["elements"][0]["material"]["sourceProfile"],
        )

    def test_no_base_mesh_aliases_its_own_first_texture_and_disables_model_material(self):
        element = self._element("mesh.own-texture-fallback", "mesh", fallback=True)
        element["resources"][1]["slotId"] = "noise"
        with mock.patch.object(
            materializer,
            "_model_info_evidence",
            side_effect=self._fixture_model_info_evidence,
        ):
            _, outputs, receipts = self._materialize([element])
            document = outputs[0][1]
            target = document["elements"][0]
            self.assertFalse(target["detail"]["mesh"]["useModelMaterial"])
            self.assertEqual(
                ["meshModel", "noise", "base"],
                [row["slotId"] for row in target["resources"]],
            )
            selected = next(
                row
                for row in receipts[0][1]["occurrences"][0]["candidates"]
                if row.get("selectionDecision") == "selected"
            )
            decision = selected["standaloneDrawableResourceDecision"]
            self.assertEqual(
                "acceptedElementTextureApproximation", decision["decision"]
            )
            self.assertEqual("sourceElement", decision["carrierScope"])
            self.assertEqual("noise", decision["promotedSourceResource"]["slotId"])
            self.assertEqual(
                "mesh.own-texture-fallback",
                decision["promotedSourceResource"]["sourceElementId"],
            )
            self.assertEqual(1, decision["promotedSourceResource"]["sourceResourceOrder"])
            self.assertEqual(
                "embeddedFirstBaseTextureAbsent",
                decision["modelInfoEvidence"]["decision"],
            )
            self.assertEqual(
                "executableBaseOverride",
                decision["runtimeDrawablePreflight"]["result"],
            )

            broken = copy.deepcopy(document)
            broken["elements"][0]["detail"]["mesh"]["useModelMaterial"] = True
            with self.assertRaisesRegex(ValueError, "explicit Base override"):
                materializer.validate_materialized_stage(broken, receipts[0][1])

            tampered_receipt = copy.deepcopy(receipts[0][1])
            tampered_selected = next(
                row
                for row in tampered_receipt["occurrences"][0]["candidates"]
                if row.get("selectionDecision") == "selected"
            )
            tampered_selected["standaloneDrawableResourceDecision"][
                "modelInfoEvidence"
            ]["materialCount"] = 99
            with self.assertRaisesRegex(ValueError, "texture approximation drifted"):
                materializer.validate_materialized_stage(document, tampered_receipt)

    def test_no_base_mesh_uses_only_the_first_exact_group_texture_donor(self):
        mesh = self._element("mesh.group-donor", "mesh", fallback=True)
        mesh["resources"] = [mesh["resources"][0]]
        donor = self._element("sprite.same-group-donor", "sprite")
        donor["groupId"] = mesh["groupId"]
        with mock.patch.object(
            materializer,
            "_model_info_evidence",
            side_effect=self._fixture_model_info_evidence,
        ):
            _, outputs, receipts = self._materialize([mesh, donor])
            target = next(
                row for row in outputs[0][1]["elements"] if row["kind"] == "mesh"
            )
            self.assertFalse(target["detail"]["mesh"]["useModelMaterial"])
            self.assertEqual(
                [
                    ("meshModel", self.model_asset_id),
                    ("base", self.texture_asset_id),
                ],
                [(row["slotId"], row["assetId"]) for row in target["resources"]],
            )
            selected = next(
                row
                for row in receipts[0][1]["occurrences"][0]["candidates"]
                if row.get("selectionDecision") == "selected"
                and row.get("sourceElementId") == mesh["id"]
            )
            decision = selected["standaloneDrawableResourceDecision"]
            promoted = decision["promotedSourceResource"]
            self.assertEqual("acceptedGroupTextureApproximation", decision["decision"])
            self.assertEqual("sourceCascadeGroup", decision["carrierScope"])
            self.assertEqual(mesh["groupId"], promoted["sourceGroupId"])
            self.assertEqual(donor["id"], promoted["sourceElementId"])
            self.assertEqual(1, promoted["sourceElementOrder"])
            self.assertEqual(0, promoted["sourceResourceOrder"])
            self.assertEqual(
                hashlib.sha256((self.resources_root / self.texture_asset_id).read_bytes()).hexdigest(),
                promoted["resourceSha256"],
            )

    def test_no_base_mesh_rejects_a_texture_from_another_group(self):
        mesh = self._element("mesh.no-cross-group", "mesh", fallback=True)
        mesh["resources"] = [mesh["resources"][0]]
        mesh["groupId"] = "mesh-group"
        other = self._element("sprite.other-group", "sprite")
        other["groupId"] = "other-group"
        _, outputs, receipts = self._materialize([mesh, other])
        self.assertEqual(["sprite"], [row["kind"] for row in outputs[0][1]["elements"]])
        rejected = next(
            row
            for row in receipts[0][1]["occurrences"][0]["candidates"]
            if row.get("sourceElementId") == mesh["id"]
        )
        self.assertEqual("rejected", rejected["selectionDecision"])
        self.assertIn("exact Cascade group", rejected["rejectionReason"])

    def test_standard_sprite_aliases_its_own_first_texture_to_base(self):
        element = self._element("sprite.source-texture-base", "sprite", fallback=True)
        element["resources"][0]["slotId"] = "noise"
        _, outputs, receipts = self._materialize([element])
        document = outputs[0][1]
        target = document["elements"][0]
        self.assertEqual(
            [("noise", self.texture_asset_id), ("base", self.texture_asset_id)],
            [(row["slotId"], row["assetId"]) for row in target["resources"]],
        )
        selected = next(
            row
            for row in receipts[0][1]["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        decision = selected["standaloneDrawableResourceDecision"]
        self.assertEqual("acceptedSourceTextureBaseAlias", decision["decision"])
        self.assertEqual("noise", decision["promotedSourceResource"]["slotId"])
        self.assertEqual(0, decision["promotedSourceResource"]["sourceOrder"])
        self.assertEqual("acceptedApproximation", decision["disposition"])
        broken = copy.deepcopy(document)
        broken["elements"][0]["resources"] = [
            row for row in broken["elements"][0]["resources"] if row["slotId"] != "base"
        ]
        with self.assertRaisesRegex(ValueError, "Base texture"):
            materializer.validate_materialized_stage(broken, receipts[0][1])

    def test_source_material_owned_sprite_does_not_alias_semantic_slots(self):
        element = self._element("sprite.source-material-owned", "sprite")
        element["resources"][0]["slotId"] = "mask"
        _, outputs, receipts = self._materialize([element])
        target = outputs[0][1]["elements"][0]
        self.assertEqual(["mask"], [row["slotId"] for row in target["resources"]])
        selected = next(
            row
            for row in receipts[0][1]["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        self.assertEqual(
            "sourceMaterialDrawableContractPreserved",
            selected["standaloneDrawableResourceDecision"]["decision"],
        )

    def test_finite_emission_uses_particle_lifetime_maximum_but_continuous_keeps_window(self):
        finite = self._element("sprite.finite-tail", "sprite")
        finite["detail"]["timing"]["lifeTimeSeconds"] = 120.0
        finite["detail"]["particle"]["lifeTimeSeconds"] = [2.0, 2.5]
        finite["sourceRecipe"]["emitterDurationSeconds"] = 120.0
        finite["sourceRecipe"]["bursts"] = [
            {"timeSeconds": 0.0, "countMinimum": 1, "countMaximum": 1}
        ]
        finite["sourceRecipe"]["modules"].append(
            {
                "stableId": "module.lifetime",
                "className": "ParticleModuleLifetime",
                "literals": [],
                "distributions": [
                    self._vector_distribution(
                        "Lifetime", [2.0], operation=2, maximum=[2.5]
                    )
                ],
            }
        )
        _, outputs, receipts = self._materialize([finite])
        self.assertEqual(
            2.5,
            outputs[0][1]["elements"][0]["detail"]["timing"]["lifeTimeSeconds"],
        )
        selected = next(
            row
            for row in receipts[0][1]["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        self.assertEqual(
            "finiteEmissionParticleLifetimeMaximum",
            selected["standaloneLifetimeDecision"]["decision"],
        )

        continuous = copy.deepcopy(finite)
        continuous["id"] = "sprite.continuous-window"
        continuous["sourceRecipe"]["modules"][1]["distributions"] = [
            self._fixed_distribution("Rate", [10.0])
        ]
        lifetime_decision = materializer._standalone_lifetime_decision(
            continuous, {"sourceDocumentKind": "particle"}
        )
        self.assertEqual(
            120.0,
            lifetime_decision["targetLifeTimeSeconds"],
        )
        self.assertEqual(
            "continuousEmitterWindowPreserved",
            lifetime_decision["decision"],
        )

    def test_seeded_particle_lifetime_alias_uses_module_tail(self):
        finite = self._element("sprite.seeded-tail", "sprite")
        finite["detail"]["timing"]["lifeTimeSeconds"] = 0.1
        finite["detail"]["particle"]["lifeTimeSeconds"] = [0.4, 0.6]
        finite["sourceRecipe"]["emitterDurationSeconds"] = 0.1
        finite["sourceRecipe"]["bursts"] = [
            {"timeSeconds": 0.0, "countMinimum": 1, "countMaximum": 1}
        ]
        finite["sourceRecipe"]["modules"].append(
            {
                "stableId": "module.lifetime-seeded",
                "className": "ParticleModuleLifetime_Seeded",
                "literals": [],
                "distributions": [
                    self._vector_distribution(
                        "Lifetime", [0.4], operation=2, maximum=[0.6]
                    )
                ],
            }
        )
        _, outputs, receipts = self._materialize([finite])
        self.assertEqual(
            0.6,
            outputs[0][1]["elements"][0]["detail"]["timing"]["lifeTimeSeconds"],
        )
        selected = next(
            row
            for row in receipts[0][1]["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        lifetime = selected["standaloneLifetimeDecision"]
        self.assertEqual("finiteEmissionParticleLifetimeMaximum", lifetime["decision"])
        self.assertEqual(
            "ParticleModuleLifetime_Seeded",
            lifetime["sourceParticleLifetimeEvidence"][0]["sourceModuleClassName"],
        )
        self.assertEqual(
            0.6,
            lifetime["sourceParticleLifetimeEvidence"][0]["maximumPositiveSeconds"],
        )

    def test_finite_burst_delay_and_loop_envelope_are_baked_into_carrier_timing(self):
        finite = self._element("sprite.delayed-loop-bursts", "sprite", delay=0.25)
        finite["detail"]["timing"]["lifeTimeSeconds"] = 0.1
        finite["detail"]["particle"]["lifeTimeSeconds"] = [0.4, 0.6]
        finite["sourceRecipe"]["emitterDelaySeconds"] = 1.0
        finite["sourceRecipe"]["emitterDurationSeconds"] = 2.0
        finite["sourceRecipe"]["emitterLoopCount"] = 2
        finite["sourceRecipe"]["bursts"] = [
            {"timeSeconds": 0.5, "countMinimum": 1, "countMaximum": 1},
            {"timeSeconds": 1.5, "countMinimum": 1, "countMaximum": 1},
        ]
        finite["sourceRecipe"]["modules"].append(
            {
                "stableId": "module.lifetime",
                "className": "ParticleModuleLifetime",
                "literals": [],
                "distributions": [
                    self._vector_distribution(
                        "Lifetime", [0.4], operation=2, maximum=[0.6]
                    )
                ],
            }
        )
        _, outputs, receipts = self._materialize([finite])
        timing = outputs[0][1]["elements"][0]["detail"]["timing"]
        self.assertEqual(1.75, timing["startDelaySeconds"])
        self.assertEqual(3.6, timing["lifeTimeSeconds"])
        selected = next(
            row
            for row in receipts[0][1]["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        decision = selected["standaloneLifetimeDecision"]
        self.assertEqual([0.5, 1.5], decision["sourceBurstTimesSeconds"])
        self.assertEqual(1.5, decision["targetStartDelayOffsetSeconds"])
        self.assertEqual(3.0, decision["sourceBurstEnvelopeSeconds"])

        continuous = copy.deepcopy(finite)
        continuous["id"] = "sprite.delayed-continuous"
        continuous["sourceRecipe"]["modules"][1]["distributions"] = [
            self._fixed_distribution("SpawnRate", [5.0])
        ]
        _, continuous_outputs, continuous_receipts = self._materialize(
            [continuous],
            effect_id="effect.dimensionmaster.skill.103.imported",
            skill_id=103,
        )
        continuous_timing = continuous_outputs[0][1]["elements"][0]["detail"][
            "timing"
        ]
        self.assertEqual(1.25, continuous_timing["startDelaySeconds"])
        self.assertEqual(0.1, continuous_timing["lifeTimeSeconds"])
        continuous_selected = next(
            row
            for row in continuous_receipts[0][1]["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        continuous_decision = continuous_selected["standaloneLifetimeDecision"]
        self.assertEqual(1.0, continuous_decision["targetStartDelayOffsetSeconds"])
        self.assertEqual(0.0, continuous_decision["sourceBurstEnvelopeSeconds"])

    def test_finite_emission_rejects_nonfinite_lifetime_fallback(self):
        element = self._element("sprite.invalid-lifetime", "sprite")
        element["detail"]["particle"]["lifeTimeSeconds"] = [float("nan"), 1.0]
        with self.assertRaisesRegex(ValueError, "finite"):
            materializer._candidate_record(element, 0, {})

    def test_particle_size_bakes_raw_sprite_and_anisotropic_mesh_samples(self):
        sprite = self._element("sprite.artist-reference", "sprite")
        sprite["sourceRecipe"]["modules"][-1]["distributions"] = [
            self._vector_distribution(
                "StartSize",
                [15.0, 1.0, 0.0],
                operation=2,
                maximum=[30.0, 2.0, 4.0],
            )
        ]
        sprite["detail"]["particle"]["startSize"] = [0.15, 0.01]
        sprite["detail"]["particle"]["endSize"] = [0.15, 0.01]
        _, outputs, receipts = self._materialize([sprite])
        self.assertEqual(
            [0.15, 0.01, 1.0],
            outputs[0][1]["elements"][0]["detail"]["transform"]["scale"],
        )
        selected = next(
            row
            for row in receipts[0][1]["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        self.assertEqual(
            [15.0, 1.0, 0.0],
            selected["standaloneSizeDecision"]["sourceInitialSizeSamples"][0][
                "rawSignedVector"
            ],
        )
        self.assertEqual(
            2,
            selected["standaloneSizeDecision"]["sourceInitialSizeSamples"][0][
                "operation"
            ],
        )
        self.assertEqual(
            "minimum",
            selected["standaloneSizeDecision"]["sourceInitialSizeSamples"][0][
                "rangeBranch"
            ],
        )

        mesh = self._element("mesh.anisotropic-reference", "mesh")
        mesh["sourceRecipe"]["modules"][-1]["distributions"] = [
            self._vector_distribution("StartSize", [2.0, 3.0, 5.0])
        ]
        mesh["sourceRecipe"]["modules"].append(
            {
                "stableId": "module.size-life",
                "className": "ParticleModuleSizeMultiplyLife",
                "literals": [],
                "distributions": [
                    self._vector_distribution(
                        "LifeMultiplier", [1.0, 1.0, 1.0], last=[2.0, 3.0, 4.0]
                    )
                ],
            }
        )
        _, outputs, receipts = self._materialize(
            [mesh],
            effect_id="effect.dimensionmaster.skill.102.imported",
            skill_id=102,
        )
        detail = outputs[0][1]["elements"][0]["detail"]
        self.assertEqual([0.02, 0.05, 0.03], detail["transform"]["scale"])
        self.assertTrue(detail["linearLerp"]["scale"])
        self.assertEqual([0.04, 0.2, 0.09], detail["linearLerp"]["endScale"])
        selected = next(
            row
            for row in receipts[0][1]["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "selected"
        )
        self.assertEqual(
            "mesh.raw-xyz-to-client-xzy.centimeters.v1",
            selected["standaloneSizeDecision"]["axisMapping"],
        )

    def test_particle_default_zero_end_does_not_create_unproven_shrink(self):
        mesh = self._element("mesh.constant-size", "mesh")
        mesh["sourceRecipe"]["modules"][-1]["distributions"] = [
            self._vector_distribution("StartSize", [2.0, 1.0, 1.0])
        ]
        mesh["detail"]["particle"]["endSize"] = [0.0, 0.0]
        _, outputs, _ = self._materialize([mesh])
        detail = outputs[0][1]["elements"][0]["detail"]
        self.assertEqual([0.02, 0.01, 0.01], detail["transform"]["scale"])
        self.assertFalse(detail["linearLerp"]["scale"])
        self.assertEqual([0.02, 0.01, 0.01], detail["linearLerp"]["endScale"])

    def test_available_source_blockers_are_explicit_approximation_provenance(self):
        source = self._source_document([self._element("mesh.blocked-source", "mesh")])
        source_path = self._write_source(source)
        stage = self._stage(
            source_path,
            source["effectAssetId"],
            hashlib.sha256(source_path.read_bytes()).hexdigest(),
        )
        stage["status"] = "AVAILABLE_WITH_BLOCKERS"
        stage["blockers"] = [
            {"code": "IMPORTED_SOURCE_MATERIAL_RUNTIME_PENDING", "count": 1}
        ]
        _, document, _, receipt = materializer.build_stage_approximation(
            character_class="DIMENSIONMASTER",
            skill_id=100,
            stage=stage,
            stage_count=1,
            claimed_source_elements=set(),
        )
        disposition = receipt["sourceIntake"]["blockerDispositions"][0]
        self.assertEqual("acceptedApproximation", disposition["disposition"])
        self.assertEqual(
            "IMPORTED_SOURCE_MATERIAL_RUNTIME_PENDING",
            disposition["sourceBlockerCode"],
        )
        self.assertEqual(stage["blockers"][0], disposition["sourceBlocker"])
        broken_receipt = copy.deepcopy(receipt)
        broken_receipt["sourceIntake"]["blockerDispositions"] = []
        with self.assertRaisesRegex(ValueError, "not dispositioned"):
            materializer.validate_materialized_stage(document, broken_receipt)

    def test_protected_dimensionmaster_a_baseline_is_never_overwritten(self):
        protected_id = "effect.dimensionmaster.skill.2050210.authored-baseline"
        protected_path = self.authored_root / f"{protected_id}.effect.json"
        protected_document = {
            "schema": "lostark.effect-authoring",
            "version": 12,
            "effectAssetId": protected_id,
            "displayName": protected_id,
            "particleSystem": {},
            "modelCues": [],
            "elements": [{"id": "protected.mesh", "kind": "mesh"}],
        }
        protected_path.write_text(
            json.dumps(protected_document) + "\n", encoding="utf-8"
        )
        stage = {
            "stageIndex": 0,
            "stageId": "skill.2050210.stage.0",
            "status": "READY",
        }
        manifest_path = self._write_manifest(stage, skill_id=2050210)
        status, outputs, receipts = materializer.materialize_manifests(
            [manifest_path]
        )
        self.assertEqual(1, status["summary"]["preservedStageCount"])
        self.assertEqual([], outputs)
        self.assertEqual([], receipts)
        self.assertEqual(
            protected_document,
            json.loads(protected_path.read_text(encoding="utf-8")),
        )

    def test_writer_replaces_only_allowlisted_legacy_particle_target(self):
        target_id = "effect.dimensionmaster.skill.2050010.ba1"
        target_path = self.authored_root / f"{target_id}.effect.json"
        legacy_document = {
            "schema": "lostark.effect-authoring",
            "version": 12,
            "effectAssetId": target_id,
            "displayName": "legacy",
            "particleSystem": {},
            "modelCues": [],
            "elements": [{"id": "legacy.particle", "kind": "particle"}],
        }
        replacement_document = {
            "schema": "lostark.effect-authoring",
            "version": 12,
            "effectAssetId": target_id,
            "displayName": "standalone",
            "particleSystem": {},
            "modelCues": [],
            "elements": [{"id": "standalone.mesh", "kind": "mesh"}],
        }
        target_path.write_text(json.dumps(legacy_document) + "\n", encoding="utf-8")
        materializer.write_outputs_transactionally(
            [(target_path, replacement_document)], []
        )
        self.assertEqual(
            replacement_document,
            json.loads(target_path.read_text(encoding="utf-8")),
        )
        self.assertEqual(
            [], list(target_path.parent.glob("*.legacy-backup"))
        )

    def test_writer_refuses_non_allowlisted_existing_authored_target(self):
        target_id = "effect.dimensionmaster.skill.100.authored-baseline"
        target_path = self.authored_root / f"{target_id}.effect.json"
        document = {
            "schema": "lostark.effect-authoring",
            "version": 12,
            "effectAssetId": target_id,
            "displayName": target_id,
            "particleSystem": {},
            "modelCues": [],
            "elements": [{"id": "mesh", "kind": "mesh"}],
        }
        target_path.write_text(json.dumps(document) + "\n", encoding="utf-8")
        with self.assertRaisesRegex(FileExistsError, "outside the legacy allowlist"):
            materializer.write_outputs_transactionally([(target_path, document)], [])

    def test_exact_reviewed_renderer_exception_selects_three_unknown_sprites(self):
        stage = self._reviewed_renderer_fixture()
        _, document, _, receipt = materializer.build_stage_approximation(
            character_class="ARTIST",
            skill_id=31210,
            stage=stage,
            stage_count=4,
            claimed_source_elements=set(),
        )
        self.assertEqual(3, len(document["elements"]))
        self.assertTrue(all(row["kind"] == "sprite" for row in document["elements"]))
        self.assertTrue(
            all(
                row["detail"]["sprite"]["billboardRollDegrees"] == -90.0
                for row in document["elements"]
            )
        )
        candidates = receipt["occurrences"][0]["candidates"]
        self.assertEqual(6, len(candidates))
        selected = [
            row for row in candidates if row.get("selectionDecision") == "selected"
        ]
        self.assertEqual(3, len(selected))
        for row in selected:
            self.assertEqual("UNKNOWN", row["conversionEligibility"])
            self.assertEqual(
                "reviewedRendererApproximation", row["conversionDecision"]
            )
            self.assertTrue(
                any(
                    module["sourceModuleClassName"].casefold()
                    == "particlemoduletypedataribbon"
                    for module in row["sourceModuleEvidence"]
                )
            )
            self.assertEqual(
                "acceptedStandardApproximation", row["materialDecision"]
            )
        self.assertTrue(
            all(
                element["material"]["sourceProfile"] == {"enabled": False}
                for element in document["elements"]
            )
        )

    def test_unknown_renderer_exception_is_exact_and_resource_fail_closed(self):
        stage = self._reviewed_renderer_fixture(missing_resource_index=0)
        _, document, _, receipt = materializer.build_stage_approximation(
            character_class="ARTIST",
            skill_id=31210,
            stage=stage,
            stage_count=4,
            claimed_source_elements=set(),
        )
        self.assertEqual(3, len(document["elements"]))
        rejected = [
            row
            for row in receipt["occurrences"][0]["candidates"]
            if row.get("selectionDecision") == "rejected"
        ]
        self.assertEqual(1, len(rejected))
        self.assertIn("unresolved", rejected[0]["rejectionReason"])

        without_approval = copy.deepcopy(stage)
        del without_approval["rendererApproximationApproval"]
        with self.assertRaisesRegex(materializer.StageBlocked, "empty after UNKNOWN"):
            materializer.build_stage_approximation(
                character_class="ARTIST",
                skill_id=31210,
                stage=without_approval,
                stage_count=4,
                claimed_source_elements=set(),
            )
        drifted = copy.deepcopy(stage)
        first_id = drifted["rendererApproximationApproval"]["sourceElementIds"][0]
        drifted["rendererApproximationApproval"]["sourceElementSha256"][first_id] = (
            "0" * 64
        )
        with self.assertRaisesRegex(ValueError, "Element hash drifted"):
            materializer.build_stage_approximation(
                character_class="ARTIST",
                skill_id=31210,
                stage=drifted,
                stage_count=4,
                claimed_source_elements=set(),
            )

    def test_exact_silent_stage_completes_without_authored_output(self):
        source = self._source_document([], "effect.artist.skill.31210.imported")
        source_path = self._write_source(source, class_directory="Artist")
        stage = self._stage(
            source_path,
            source["effectAssetId"],
            hashlib.sha256(source_path.read_bytes()).hexdigest(),
            skill_id=31210,
        )
        contract = materializer.INTENTIONALLY_SILENT_STAGE_CONTRACTS[
            ("ARTIST", 31210, 1)
        ]
        stage.update(
            {
                "stageIndex": 1,
                "stageId": "skill.31210.stage.1",
                "timelineOffsetSeconds": 1.0,
                "durationSeconds": 1.0,
                "sourceEventIds": [],
                "completionDecision": {
                    "decision": "sourceIntentionallySilent",
                    "evidence": {
                        "clip": contract["clip"],
                        "effectNotifyCount": 0,
                        "shakeNotifyCount": 0,
                        "observedNotifyKinds": sorted(
                            contract["observedNotifyKinds"]
                        ),
                    },
                    "rationale": "Control-only fixture clip.",
                },
            }
        )
        stage["clips"][0].update(
            {
                "clip": contract["clip"],
                "timelineOffsetSeconds": 1.0,
            }
        )
        completion = materializer._validate_intentionally_silent_stage(
            character_class="ARTIST",
            skill_id=31210,
            stage=stage,
            stage_count=4,
        )
        self.assertEqual("sourceIntentionallySilent", completion["decision"])
        self.assertEqual([], list(self.authored_root.glob("*.effect.json")))

    def test_repeated_clip_reuse_requires_both_equivalence_hashes(self):
        contract = materializer.REUSED_PRODUCT_STAGE_CONTRACTS[
            ("ARTIST", 31210, 2)
        ]
        reused_id = "effect.artist.skill.31210.ba1"
        nominal_id = "effect.artist.skill.31210.ba3"
        elements = [
            {"id": f"sprite.{index}", "kind": "sprite", "payload": index}
            for index in range(contract["sourceCarrierCount"])
        ]
        reused_document = {
            "schema": "lostark.effect-authoring",
            "version": 12,
            "effectAssetId": reused_id,
            "displayName": reused_id,
            "particleSystem": {},
            "modelCues": [],
            "elements": copy.deepcopy(elements),
        }
        nominal_document = copy.deepcopy(reused_document)
        nominal_document["effectAssetId"] = nominal_id
        nominal_document["displayName"] = nominal_id

        def receipt(effect_id, stage_index, document):
            return {
                "schema": materializer.RECEIPT_SCHEMA,
                "version": materializer.RECEIPT_VERSION,
                "targetEffectAssetId": effect_id,
                "targetAuthoringPath": f"Effects/Authored/{effect_id}.effect.json",
                "characterClass": "ARTIST",
                "productSkillId": 31210,
                "stageIndex": stage_index,
                "sourceTimeline": {"clips": [{"clip": contract["clip"]}]},
                "sourceCarrierEquivalence": {
                    "normalizationRule": "fixture",
                    "elementCount": contract["sourceCarrierCount"],
                    "normalizedContentSha256": contract[
                        "normalizedSourceCarrierSha256"
                    ],
                },
                "output": {"documentSha256": _json_sha256(document)},
            }

        reuse_path, reuse_receipt = materializer._build_reuse_receipt(
            character_class="ARTIST",
            skill_id=31210,
            stage_index=2,
            stage_count=4,
            nominal_document=nominal_document,
            nominal_receipt=receipt(nominal_id, 2, nominal_document),
            reused_stage_index=0,
            reused_document=reused_document,
            reused_receipt=receipt(reused_id, 0, reused_document),
        )
        self.assertEqual(
            f"{nominal_id}.approximation-receipt.json", reuse_path.name
        )
        self.assertEqual(reused_id, reuse_receipt["reusesProductTarget"])
        drifted_reused = copy.deepcopy(reused_document)
        drifted_reused["elements"][0]["payload"] = -1
        with self.assertRaisesRegex(ValueError, "not equivalent"):
            materializer._build_reuse_receipt(
                character_class="ARTIST",
                skill_id=31210,
                stage_index=2,
                stage_count=4,
                nominal_document=nominal_document,
                nominal_receipt=receipt(nominal_id, 2, nominal_document),
                reused_stage_index=0,
                reused_document=drifted_reused,
                reused_receipt=receipt(reused_id, 0, drifted_reused),
            )

    def test_generated_output_and_receipt_are_idempotently_preserved(self):
        status, outputs, receipts = self._materialize(
            [self._element("mesh.idempotent", "mesh")]
        )
        self.assertEqual(1, status["summary"]["pendingAuthoredOutputCount"])
        materializer.write_outputs_transactionally(outputs, receipts)
        source_path = next(self.imported_root.rglob("*.effect.json"))
        source = json.loads(source_path.read_text(encoding="utf-8"))
        stage = self._stage(
            source_path,
            source["effectAssetId"],
            hashlib.sha256(source_path.read_bytes()).hexdigest(),
        )
        manifest_path = self._write_manifest(stage)
        second_status, second_outputs, second_receipts = (
            materializer.materialize_manifests([manifest_path])
        )
        self.assertEqual(1, second_status["summary"]["preservedGeneratedStageCount"])
        self.assertEqual([], second_outputs)
        self.assertEqual([], second_receipts)

    def test_hash_owned_generated_pair_refreshes_only_reviewed_output_contracts(self):
        source_element = self._element("mesh.refresh-owned", "mesh", fallback=True)
        source = self._source_document([source_element])
        source_path = self._write_source(source)
        stage = self._stage(
            source_path,
            source["effectAssetId"],
            hashlib.sha256(source_path.read_bytes()).hexdigest(),
        )
        manifest_path = self._write_manifest(stage)
        _, outputs, receipts = materializer.materialize_manifests([manifest_path])
        expected_document = copy.deepcopy(outputs[0][1])
        expected_receipt = copy.deepcopy(receipts[0][1])
        legacy_document = copy.deepcopy(expected_document)
        legacy_receipt = copy.deepcopy(expected_receipt)
        selected_by_target = {
            candidate["targetElementId"]: candidate
            for occurrence in expected_receipt["occurrences"]
            for candidate in occurrence["candidates"]
            if candidate.get("selectionDecision") == "selected"
        }
        for element in legacy_document["elements"]:
            candidate = selected_by_target[element["id"]]
            size = candidate["standaloneSizeDecision"]
            lifetime = candidate["standaloneLifetimeDecision"]
            element["resources"] = copy.deepcopy(candidate["resources"])
            element["detail"]["mesh"]["useModelMaterial"] = False
            element["material"] = materializer._legacy_accepted_standard_material(
                candidate["materialProvenance"], candidate["resources"]
            )
            element["detail"]["timing"]["lifeTimeSeconds"] = lifetime[
                "sourceElementTimingSeconds"
            ]
            element["detail"]["transform"]["scale"] = copy.deepcopy(
                size["sourceTransformStartScale"]
            )
            element["detail"]["linearLerp"]["scale"] = size[
                "sourceLinearLerpScaleEnabled"
            ]
            element["detail"]["linearLerp"]["endScale"] = copy.deepcopy(
                size["sourceTransformEndScale"]
            )
        for occurrence in legacy_receipt["occurrences"]:
            for candidate in occurrence["candidates"]:
                candidate.pop("standaloneLifetimeDecision", None)
                candidate.pop("standaloneSizeDecision", None)
                candidate.pop("standaloneDrawableResourceDecision", None)
        legacy_receipt["output"]["documentSha256"] = _json_sha256(legacy_document)
        materializer.write_outputs_transactionally(
            [(outputs[0][0], legacy_document)],
            [(receipts[0][0], legacy_receipt)],
        )

        blocked, blocked_outputs, blocked_receipts = materializer.materialize_manifests(
            [manifest_path]
        )
        self.assertEqual(1, blocked["summary"]["blockedStageCount"])
        self.assertEqual([], blocked_outputs)
        self.assertEqual([], blocked_receipts)

        refreshed, refresh_outputs, refresh_receipts = (
            materializer.materialize_manifests(
                [manifest_path], allow_generated_refresh=True
            )
        )
        self.assertEqual(1, refreshed["summary"]["refreshedGeneratedStageCount"])
        materializer.write_outputs_transactionally(
            refresh_outputs,
            refresh_receipts,
            allow_generated_refresh=True,
        )
        self.assertEqual(
            expected_document,
            json.loads(outputs[0][0].read_text(encoding="utf-8")),
        )
        final_status, final_outputs, final_receipts = materializer.materialize_manifests(
            [manifest_path]
        )
        self.assertEqual(1, final_status["summary"]["preservedGeneratedStageCount"])
        self.assertEqual([], final_outputs)
        self.assertEqual([], final_receipts)

    def test_transaction_failure_rolls_back_all_new_outputs(self):
        target_id = "effect.dimensionmaster.skill.999.authored-baseline"
        target_path = self.authored_root / f"{target_id}.effect.json"
        receipt_path = self.receipt_root / "fixture.receipt.json"
        document = {
            "schema": "lostark.effect-authoring",
            "version": 12,
            "effectAssetId": target_id,
            "elements": [],
        }
        receipt = {"schema": "fixture"}
        real_replace = materializer.os.replace
        replace_count = 0

        def fail_second_commit(source, target):
            nonlocal replace_count
            replace_count += 1
            if replace_count == 2:
                raise OSError("injected second commit failure")
            return real_replace(source, target)

        with mock.patch.object(materializer.os, "replace", side_effect=fail_second_commit):
            with self.assertRaisesRegex(OSError, "injected second commit failure"):
                materializer.write_outputs_transactionally(
                    [(target_path, document)], [(receipt_path, receipt)]
                )
        self.assertFalse(target_path.exists())
        self.assertFalse(receipt_path.exists())
        self.assertEqual([], list(target_path.parent.glob("*.tmp")))
        self.assertEqual([], list(receipt_path.parent.glob("*.tmp")))


if __name__ == "__main__":
    unittest.main()
