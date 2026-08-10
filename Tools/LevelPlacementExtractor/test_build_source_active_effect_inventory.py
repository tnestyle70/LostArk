#!/usr/bin/env python3

from __future__ import annotations

import hashlib
import json
import tempfile
import unittest
from pathlib import Path

from build_source_active_effect_inventory import (
    InventoryError,
    build_source_active_effect_inventory,
    check_or_write_json,
)


class SourceActiveEffectInventoryTests(unittest.TestCase):
    def _write_json(self, root: Path, name: str, value: object) -> Path:
        path = root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")
        return path

    @staticmethod
    def _attachment(mode: str) -> dict[str, object]:
        if mode == "FOLLOW_NAMED_ANCHORS":
            return {
                "mode": mode,
                "sourceAnchorNames": ["WP_TEST_R_Battle"],
                "runtimeAnchorSlotIds": ["WP_TEST_R_Battle"],
                "runtimeAnchorSlotId": "WP_TEST_R_Battle",
                "runtimeBoneName": "b_wp_1",
                "socketLocalTransform": {
                    "position": [0.0, 0.0, 0.0],
                    "rotationDegrees": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
                "runtimeResolutionStatus": "EXACT_SOURCE_SOCKET",
            }
        return {
            "mode": "SNAPSHOT_ROOT",
            "sourceAnchorNames": [],
            "runtimeAnchorSlotIds": [],
            "runtimeAnchorSlotId": "root",
            "runtimeBoneName": "",
            "socketLocalTransform": {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
            "runtimeResolutionStatus": "EXACT_ROOT_SNAPSHOT",
        }

    def _cue(
        self,
        cue_id: str,
        system: str,
        time_seconds: float,
        enabled: bool,
        mode: str,
    ) -> dict[str, object]:
        return {
            "cueId": cue_id,
            "globalTimeSeconds": time_seconds,
            "durationSeconds": 0.25,
            "sourceType": "PlayParticleEffect",
            "assetReferences": [
                {"className": "ParticleSystem", "objectPath": system}
            ],
            "typedPayload": {
                "sourceByteOffset": 47,
                "sourceByteValue": 1 if enabled else 0,
                "attachment": self._attachment(mode),
                "localTransform": {
                    "position": [1.0, 2.0, 3.0],
                    "rotationDegrees": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
            },
            "executionEnabled": enabled,
            "sourceReceiptEventIndex": int(cue_id.rsplit("-", 1)[-1]),
            "sourceExecutionStatus": "SEMANTIC_EXECUTION_AUDIT_REQUIRED",
        }

    @staticmethod
    def _node(node_id: str, class_name: str, object_path: str) -> dict[str, object]:
        return {
            "nodeId": node_id,
            "package": "FX_BASE",
            "exportIndex": int(node_id.rsplit(":", 1)[-1]),
            "className": class_name,
            "objectName": object_path.rsplit(".", 1)[-1],
            "objectPath": object_path,
            "properties": {},
        }

    @staticmethod
    def _material(path: str, renderer: str) -> dict[str, object]:
        return {
            "sourceMaterialPath": path,
            "sourceLogicalPackage": path.split(".", 1)[0],
            "rendererShapes": [renderer],
            "resolutionEvidence": "UNIT_TEST",
            "sourcePhysicalPackage": "MATERIAL.upk",
            "sourcePhysicalPackageSha256": "1" * 64,
            "material": {
                "objectPath": path.split(".", 1)[-1],
                "className": "materialinstanceconstant",
                "exportIndex": 10,
                "serialSize": 100,
                "parent": "fx_material.parent",
                "scalarParameters": [{"name": "power", "value": 2.0}],
                "textureParameters": [],
                "vectorParameters": [],
                "overrideTwoSided": True,
                "hasStaticPermutationResource": True,
            },
            "materialGraph": None,
            "parentGraph": {
                "logicalPackage": "fx_material",
                "physicalPackage": "PARENT.upk",
                "resolutionEvidence": "UNIT_TEST",
                "physicalPackageSha256": "2" * 64,
                "graph": {
                    "materialPath": "fx_material.parent",
                    "expressions": [None, {"className": "materialexpressionmultiply"}],
                    "summary": {
                        "expressionEntryCount": 2,
                        "nonNullExpressionCount": 1,
                        "nullExpressionCount": 1,
                        "unresolvedInputEdgeCount": 1,
                        "topologyStatus": "COOKED_PARTIAL",
                        "runtimeExactEligible": False,
                    },
                },
            },
            "status": "SOURCE_IDENTITY_CLOSED_SHADER_TOPOLOGY_PENDING",
        }

    def _fixture(self, root: Path) -> dict[str, Path]:
        base_system = "FX_BASE.base_system"
        disabled_system = "FX_BASE.disabled_system_04"
        light_system = "FX_LIGHT.light_system"
        action = {
            "schema": "lostark.effect-action-cue-recipe",
            "formatVersion": 2,
            "characterClass": "ARTIST",
            "skillId": 31470,
            "inputSlot": "F",
            "cues": [
                self._cue("cue-0", base_system, 0.0, True, "FOLLOW_NAMED_ANCHORS"),
                self._cue("cue-1", disabled_system, 1.0, False, "SNAPSHOT_ROOT"),
                self._cue("cue-2", light_system, 2.0, True, "SNAPSHOT_ROOT"),
                self._cue("cue-3", light_system, 2.0, False, "SNAPSHOT_ROOT"),
            ],
            "summary": {
                "referenceJoinComplete": True,
                "runtimeAnchorResolutionComplete": True,
            },
            "sourceExtractionComplete": True,
            "runtimeExecutionComplete": False,
        }

        nodes = [
            self._node("FX_BASE:1", "particlespriteemitter", "base_system.emitter_0"),
            self._node(
                "FX_BASE:2",
                "particlelodlevel",
                "base_system.emitter_0.particlelodlevel_0",
            ),
            self._node(
                "FX_BASE:3", "particlemodulerequired", "base_system.required_0"
            ),
            self._node("FX_BASE:4", "particlespriteemitter", "base_system.emitter_1"),
            self._node(
                "FX_BASE:5",
                "particlelodlevel",
                "base_system.emitter_1.particlelodlevel_0",
            ),
            self._node(
                "FX_BASE:6",
                "particlemoduletypedataribbon",
                "base_system.particlemoduletypedataribbon_0",
            ),
            self._node("FX_LIGHT:1", "particlespriteemitter", "light_system.emitter_0"),
            self._node(
                "FX_LIGHT:2",
                "particlelodlevel",
                "light_system.emitter_0.particlelodlevel_0",
            ),
            self._node(
                "FX_LIGHT:3",
                "efparticlemoduletypedatalight",
                "light_system.efparticlemoduletypedatalight_0",
            ),
        ]
        graph = {
            "schema": "lostark.normalized-effect-source-graph",
            "schemaVersion": 1,
            "characterClass": "ARTIST",
            "skillId": 31470,
            "sourceSystems": [
                {
                    "sourceSystemId": base_system,
                    "sourceAsset": f"ParticleSystem'{base_system}'",
                    "logicalPackage": "FX_BASE",
                    "objectName": "base_system",
                    "objectPath": base_system,
                    "rootNodeId": "FX_BASE:0",
                    "nodeIds": [f"FX_BASE:{value}" for value in range(1, 7)],
                },
                {
                    "sourceSystemId": disabled_system,
                    "sourceAsset": f"ParticleSystem'{disabled_system}'",
                    "logicalPackage": "FX_BASE",
                    "objectName": "disabled_system_04",
                    "objectPath": disabled_system,
                    "rootNodeId": "FX_BASE:7",
                    "nodeIds": [],
                },
                {
                    "sourceSystemId": light_system,
                    "sourceAsset": f"ParticleSystem'{light_system}'",
                    "logicalPackage": "FX_LIGHT",
                    "objectName": "light_system",
                    "objectPath": light_system,
                    "rootNodeId": "FX_LIGHT:0",
                    "nodeIds": ["FX_LIGHT:1", "FX_LIGHT:2", "FX_LIGHT:3"],
                },
            ],
            "nodes": nodes,
            "edges": [],
            "materialParameterBindings": [],
            "runtimeResourceBindings": [],
            "summary": {"sourceSystemCount": 3},
        }

        module_closure = {
            "schema": "lostark.ue3-particle-external-module-closure",
            "schemaVersion": 1,
            "characterClass": "ARTIST",
            "skillId": 31470,
            "packages": [
                {
                    "logicalPackage": "EXT",
                    "physicalPackage": "EXT.upk",
                    "resolutionSource": "UNIT_TEST",
                    "requestedReferences": [],
                    "unresolvedRequestedReferences": [],
                    "objects": [
                        {
                            "objectId": "EXT:export:1",
                            "exportIndex": 1,
                            "className": "particlemodulerotation",
                            "objectName": "particlemodulerotation_0",
                            "objectPath": "package.particlemodulerotation_0",
                            "properties": {"startrotation": {"value": 0.5}},
                            "references": [],
                            "requestedDirectly": True,
                            "propertyStreamEnd": 12,
                        }
                    ],
                    "propertyErrors": [],
                    "summary": {
                        "requestCount": 1,
                        "resolvedRequestCount": 1,
                        "unresolvedRequestCount": 0,
                        "closureObjectCount": 1,
                        "propertyErrorCount": 0,
                    },
                }
            ],
            "summary": {
                "packageCount": 1,
                "requestCount": 1,
                "resolvedRequestCount": 1,
                "unresolvedRequestCount": 0,
                "closureObjectCount": 1,
                "propertyErrorCount": 0,
            },
        }

        common_detail = [
            {"target": "timing.startDelaySeconds", "status": "EXACT", "value": 0.0}
        ]
        conversion = {
            "schema": "lostark.imported-effect-element-conversion-receipt",
            "schemaVersion": 1,
            "characterClass": "ARTIST",
            "skillId": 31470,
            "inputSlot": "R",
            "elementConversions": [
                {
                    "sourceSystemId": base_system,
                    "sourceEmitter": "FX_BASE.base_system.emitter_0",
                    "sourceLod": "FX_BASE.base_system.emitter_0.particlelodlevel_0",
                    "targetKind": "particle",
                    "rendererShape": "sprite",
                    "status": "SOURCE_RECIPE_RUNTIME_PENDING",
                    "sourceMaterialRuntimePending": True,
                    "missingResources": [],
                    "eventOccurrences": [
                        {"eventId": "event-0", "globalTimeSeconds": 0.0, "durationSeconds": 0.25}
                    ],
                    "elementIds": ["base.sprite"],
                    "resourceMappings": [
                        {
                            "slotId": "base",
                            "sourceObjectPath": "fx_tex.base",
                            "assetId": "Effect/Test/base.dds",
                            "status": "EXACT_RUNTIME_BINDING",
                        }
                    ],
                    "detailMappings": common_detail,
                    "burstSource": [],
                    "moduleEvidence": [
                        {
                            "className": "particlemodulerequired",
                            "objectPath": "FX_BASE.base_system.required_0",
                        }
                    ],
                    "unrepresentedModules": [],
                    "presentationSourceStatus": "unresolved",
                    "presentationProfileId": "",
                    "materialParameterEvidence": [
                        {"sourceMaterialPath": "MAT.sprite", "scalars": [], "vectors": []}
                    ],
                },
                {
                    "sourceSystemId": base_system,
                    "sourceEmitter": "FX_BASE.base_system.emitter_1",
                    "sourceLod": "FX_BASE.base_system.emitter_1.particlelodlevel_0",
                    "targetKind": "particle",
                    "rendererShape": "sprite",
                    "status": "SOURCE_RECIPE_RUNTIME_PENDING",
                    "sourceMaterialRuntimePending": True,
                    "missingResources": [],
                    "eventOccurrences": [
                        {"eventId": "event-0", "globalTimeSeconds": 0.0, "durationSeconds": 0.25}
                    ],
                    "elementIds": ["base.ribbon"],
                    "resourceMappings": [],
                    "detailMappings": common_detail,
                    "burstSource": [],
                    "moduleEvidence": [
                        {
                            "className": "particlemoduletypedataribbon",
                            "objectPath": "FX_BASE.base_system.particlemoduletypedataribbon_0",
                        },
                        {
                            "className": "particlemodulerotation",
                            "objectPath": "EXT.package.particlemodulerotation_0",
                        },
                    ],
                    "unrepresentedModules": [
                        {
                            "className": "particlemodulerotation",
                            "objectPath": "EXT.package.particlemodulerotation_0",
                            "reason": "CURRENT_EFFECT_DOCUMENT_HAS_NO_EQUIVALENT_MAPPING",
                        }
                    ],
                    "presentationSourceStatus": "unresolved",
                    "presentationProfileId": "",
                    "materialParameterEvidence": [
                        {"sourceMaterialPath": "MAT.ribbon", "scalars": [], "vectors": []}
                    ],
                },
                {
                    "sourceSystemId": disabled_system,
                    "sourceEmitter": "FX_BASE.disabled_system_04.emitter_0",
                    "sourceLod": "FX_BASE.disabled_system_04.emitter_0.particlelodlevel_0",
                    "targetKind": "particle",
                    "rendererShape": "mesh",
                    "status": "SOURCE_RECIPE_RUNTIME_PENDING",
                    "sourceMaterialRuntimePending": True,
                    "missingResources": [],
                    "eventOccurrences": [
                        {"eventId": "event-1", "globalTimeSeconds": 1.0, "durationSeconds": 0.25}
                    ],
                    "elementIds": ["disabled.mesh"],
                    "resourceMappings": [
                        {
                            "slotId": "meshModel",
                            "sourceObjectPath": "fx_sm.disabled",
                            "assetId": "Effect/Test/disabled.wmodel",
                            "status": "EXACT_RUNTIME_BINDING",
                        }
                    ],
                    "detailMappings": common_detail,
                    "burstSource": [],
                    "moduleEvidence": [
                        {
                            "className": "particlemoduletypedatamesh",
                            "objectPath": "FX_BASE.disabled_system_04.typedata_0",
                        }
                    ],
                    "unrepresentedModules": [],
                    "presentationSourceStatus": "unresolved",
                    "presentationProfileId": "",
                    "materialParameterEvidence": [
                        {"sourceMaterialPath": "MAT.disabled", "scalars": [], "vectors": []}
                    ],
                },
                {
                    "sourceSystemId": light_system,
                    "sourceEmitter": "FX_LIGHT.light_system.emitter_0",
                    "sourceLod": "FX_LIGHT.light_system.emitter_0.particlelodlevel_0",
                    "targetKind": "light",
                    "rendererShape": "light",
                    "status": "SOURCE_RECIPE_RUNTIME_PENDING",
                    "sourceMaterialRuntimePending": False,
                    "missingResources": [],
                    "eventOccurrences": [
                        {"eventId": "event-2", "globalTimeSeconds": 2.0, "durationSeconds": 0.25},
                        {"eventId": "event-3", "globalTimeSeconds": 2.0, "durationSeconds": 0.25},
                    ],
                    "elementIds": ["light.active", "light.disabled"],
                    "resourceMappings": [],
                    "detailMappings": common_detail,
                    "burstSource": [],
                    "moduleEvidence": [
                        {
                            "className": "efparticlemoduletypedatalight",
                            "objectPath": "FX_LIGHT.light_system.efparticlemoduletypedatalight_0",
                        }
                    ],
                    "unrepresentedModules": [
                        {
                            "className": "efparticlemoduletypedatalight",
                            "objectPath": "FX_LIGHT.light_system.efparticlemoduletypedatalight_0",
                            "reason": "CURRENT_EFFECT_DOCUMENT_HAS_NO_EQUIVALENT_MAPPING",
                        }
                    ],
                    "presentationSourceStatus": "unresolved",
                    "presentationProfileId": "light.point.reconstructed.v1",
                    "materialParameterEvidence": [
                        {
                            "sourceMaterialPath": "enginematerials.defaultparticle",
                            "scalars": [],
                            "vectors": [],
                        }
                    ],
                },
            ],
        }

        builtin = {
            "sourceMaterialPath": "enginematerials.defaultparticle",
            "sourceLogicalPackage": "enginematerials",
            "rendererShapes": ["light"],
            "status": "SOURCE_BUILTIN_NON_RENDER_MATERIAL",
            "runtimeExactEligible": True,
        }
        material_closure = {
            "schema": "lostark.ue3-effect-material-closure",
            "formatVersion": 1,
            "characterClass": "ARTIST",
            "skillId": 31470,
            "inputSlot": "F",
            "activeSourceSystemIds": [base_system, light_system],
            "materials": [
                self._material("MAT.sprite", "sprite"),
                self._material("MAT.ribbon", "sprite"),
                builtin,
            ],
            "summary": {"activeSourceSystemCount": 2, "materialCount": 3},
        }

        resource_root = root / "Resources"
        texture = resource_root / "Effect/Test/base.dds"
        texture.parent.mkdir(parents=True, exist_ok=True)
        texture.write_bytes(b"DDS test texture")

        return {
            "action": self._write_json(root, "action.json", action),
            "conversion": self._write_json(root, "conversion.json", conversion),
            "graph": self._write_json(root, "graph.json", graph),
            "modules": self._write_json(root, "modules.json", module_closure),
            "materials": self._write_json(root, "materials.json", material_closure),
            "resource_root": resource_root,
        }

    def _build(self, paths: dict[str, Path], **overrides: object) -> dict[str, object]:
        arguments: dict[str, object] = {
            "action_cue_recipe_path": paths["action"],
            "conversion_receipt_path": paths["conversion"],
            "normalized_graph_path": paths["graph"],
            "module_closure_path": paths["modules"],
            "material_closure_path": paths["materials"],
            "resource_root": paths["resource_root"],
            "expected_renderer_counts": {
                "SpriteParticle": 1,
                "CascadeRibbon": 1,
                "LightParticle": 1,
            },
            "expected_active_element_count": 3,
            "expected_excluded_legacy_element_count": 2,
            "allow_legacy_conversion_input_slot_mismatch": True,
        }
        arguments.update(overrides)
        return build_source_active_effect_inventory(**arguments)

    def test_execution_bits_select_elements_and_preserve_exact_provenance(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            paths = self._fixture(Path(directory))
            receipt = self._build(paths)

            self.assertEqual(receipt["summary"]["legacyFlattenedElementCount"], 5)
            self.assertEqual(receipt["summary"]["activeElementCount"], 3)
            self.assertEqual(
                receipt["summary"]["excludedExecutionDisabledLegacyElementCount"], 2
            )
            self.assertEqual(
                receipt["summary"]["activeRendererCounts"],
                {
                    "SpriteParticle": 1,
                    "CascadeRibbon": 1,
                    "LightParticle": 1,
                },
            )
            active = receipt["activeElements"]
            self.assertEqual(
                [item["rendererType"] for item in active],
                ["SpriteParticle", "CascadeRibbon", "LightParticle"],
            )
            self.assertFalse(
                any("disabled_system_04" in item["sourceSystemId"] for item in active)
            )
            self.assertEqual(active[0]["sourceEmitterNode"]["className"], "particlespriteemitter")
            self.assertEqual(
                receipt["activeCues"][0]["attachment"]["runtimeBoneName"], "b_wp_1"
            )
            ribbon = next(item for item in active if item["rendererType"] == "CascadeRibbon")
            unsupported = ribbon["runtimeUnsupportedModules"][0]
            self.assertEqual(
                unsupported["sourceRecord"]["sourceDocument"],
                "externalModuleClosure",
            )
            self.assertEqual(
                unsupported["sourceRecord"]["physicalPackage"], "EXT.upk"
            )
            self.assertEqual(
                {item.get("shaderGraph", {}).get("topologyStatus") for item in receipt["materialEvidence"]},
                {"COOKED_PARTIAL", None},
            )
            resource = receipt["runtimeResourceIdentities"][0]
            self.assertEqual(resource["resourceKind"], "TextureDDS")
            self.assertEqual(
                resource["sha256"], hashlib.sha256(b"DDS test texture").hexdigest()
            )
            excluded_ids = {
                item["legacyElementId"]
                for item in receipt["excludedExecutionDisabled"]["legacyElements"]
            }
            self.assertEqual(excluded_ids, {"disabled.mesh", "light.disabled"})
            self.assertFalse(receipt["productEmissionAllowed"])
            self.assertEqual(receipt["visualApprovalStatus"], "MANUAL_VISUAL_PENDING")

    def test_output_is_deterministic(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            paths = self._fixture(Path(directory))
            self.assertEqual(self._build(paths), self._build(paths))

    def test_legacy_input_slot_mismatch_requires_explicit_flag(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            paths = self._fixture(Path(directory))
            with self.assertRaisesRegex(InventoryError, "inputSlot does not match"):
                self._build(
                    paths,
                    allow_legacy_conversion_input_slot_mismatch=False,
                )

    def test_missing_active_resource_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            paths = self._fixture(Path(directory))
            (paths["resource_root"] / "Effect/Test/base.dds").unlink()
            with self.assertRaisesRegex(InventoryError, "runtime resource is missing"):
                self._build(paths)

    def test_expected_renderer_count_is_enforced(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            paths = self._fixture(Path(directory))
            with self.assertRaisesRegex(InventoryError, "MeshParticle count mismatch"):
                self._build(paths, expected_renderer_counts={"MeshParticle": 1})

    def test_check_mode_is_byte_exact_and_fails_closed_when_stale(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "receipt.json"
            document = {"schema": "test", "count": 3}
            check_or_write_json(output, document)
            check_or_write_json(output, document, check=True)
            output.write_text("{}\n", encoding="utf-8")
            with self.assertRaisesRegex(InventoryError, "receipt is stale"):
                check_or_write_json(output, document, check=True)


if __name__ == "__main__":
    unittest.main()
