#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import tempfile
import unittest
from pathlib import Path

import build_valtan_trail_adapter_packets as adapters


ROOT = Path(__file__).resolve().parents[2]
SCHEMA_PATH = (
    ROOT
    / "Tools/EffectPipeline/Schemas/"
    "lostark.valtan-trail-adapter-packets.schema.json"
)


def selection(
    stage: int,
    clip_occurrence_id: str | None,
    timing: str = "REACHABLE",
) -> dict:
    return {
        "schema": "lostark.valtan-source-branch-selections",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "additiveFutureField": {"accepted": True},
        "selections": [
            {
                "patternId": "VALTAN_TEST",
                "sourceActionId": 420001,
                "profileId": "MN_RPBF_00",
                "sequenceIndex": 3,
                "sourceSequencePathSha256": "a" * 64,
                "branchId": (
                    "valtan_test.source-420001.mn_rpbf_00."
                    "sequence-003.stages-001-003"
                ),
                "status": "REVIEWED_SELECTED",
                "reviewBasis": "fixture exact sequence review",
                "futureSelectionEvidence": [1, 2, 3],
                "stageMappings": [
                    {
                        "sourceStageIndex": stage,
                        "sourceClipOrdinal": 0,
                        "clipOccurrenceId": clip_occurrence_id,
                        "timingDisposition": timing,
                        "futureMappingEvidence": "accepted",
                    }
                ],
            }
        ],
    }


def occurrence(
    *,
    stage: int,
    source_type: str,
    category: str = "trail",
    asset: dict | None = None,
    source_system_id: str | None = None,
) -> dict:
    suffix = f"{stage:02d}-{source_type.casefold()}"
    return {
        "occurrenceId": f"occurrence.{suffix}",
        "fullKey": f"occurrence-key.{suffix}",
        "patternId": "VALTAN_TEST",
        "semanticStageId": None,
        "gameplayActionId": None,
        "clipOccurrenceId": None,
        "candidateClipOccurrenceIds": [],
        "timingDisposition": "SOURCE_TIMING_REVIEW_REQUIRED",
        "mappingReviewBasis": None,
        "sourceActionId": 420001,
        "profileId": "MN_RPBF_00",
        "branchId": (
            "valtan_test.source-420001.mn_rpbf_00."
            "branch-001.stages-001-003"
        ),
        "branchSelectionStatus": "UNRESOLVED_BRANCH_SELECTION",
        "sourceStageIndex": stage,
        "sourceStagePath": f"action-420001/MN_RPBF_00/stage-{stage:03d}",
        "sourceClipOrdinal": 0,
        "sourceClip": "Att_Test_01",
        "notifyOrdinal": 0,
        "notifyId": f"action-420001/stage-{stage:03d}/notify-001",
        "sourceTimeSeconds": 0.1,
        "sourceDurationSeconds": 0.8,
        "sourceType": source_type,
        "category": category,
        "sourceResolutionStatus": "SOURCE_ASSET_EXPLICIT",
        "sourceAssetOrdinal": 0,
        "assetReference": copy.deepcopy(asset),
        "sourceSystemId": source_system_id,
        "disposition": "UNRESOLVED_RUNTIME_ADAPTER",
        "reachabilityDisposition": "UNRESOLVED_BRANCH_SELECTION",
        "expandedCarrierCount": 0,
        "expandedCarrierFullKeysSha256": "b" * 64,
    }


def inventory(rows: list[dict], systems: list[dict] | None = None) -> dict:
    return {
        "schema": "lostark.valtan-source-occurrence-inventory",
        "formatVersion": 1,
        "bossArchetypeId": "BOSS_VALTAN",
        "branches": [
            {
                "branchId": (
                    "valtan_test.source-420001.mn_rpbf_00."
                    "branch-001.stages-001-003"
                ),
                "branchOrdinal": 0,
            }
        ],
        "sourceSystems": systems or [],
        "occurrences": rows,
    }


def edge_history() -> dict:
    result = {
        "historyId": "fixture.edge-history.v1",
        "coordinateBasis": "RUNTIME_METERS_XYZ",
        "samples": [
            {
                "relativeTimeSeconds": 0.0,
                "firstEdgeMeters": [0.0, 0.0, 0.0],
                "controlPointMeters": [0.0, 0.5, 0.0],
                "secondEdgeMeters": [0.0, 1.0, 0.0],
            },
            {
                "relativeTimeSeconds": 0.1,
                "firstEdgeMeters": [1.0, 0.0, 0.0],
                "controlPointMeters": [1.0, 0.5, 0.0],
                "secondEdgeMeters": [1.0, 1.0, 0.0],
            },
        ],
    }
    result["historySha256"] = adapters.canonical_sha256(result)
    return result


def trail_target(asset_id: str) -> dict:
    return {
        "id": "valtan.trail-ghost.fixture",
        "kind": "trail",
        "resources": [{"slotId": "base", "assetId": asset_id}],
        "targetTiming": {
            "startDelaySeconds": 0.0,
            "lifeTimeSeconds": 0.8,
            "afterImageSeconds": 0.0,
            "dissolveStartNormalized": 1.0,
        },
        "attachment": {
            "enabled": False,
            "follow": False,
            "sourceAnchorSlotId": "",
            "runtimeAnchorSlotId": "",
            "runtimeBoneName": "",
            "snapshotRootSourceBasisYawDegrees": 0.0,
            "socketLocalTransform": {
                "position": [0.0, 0.0, 0.0],
                "rotationDegrees": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        },
        "trail": {
            "maxPoints": 32,
            "pointLifeTimeSeconds": 0.8,
            "sampleIntervalSeconds": 0.1,
            "minimumDistance": 0.0,
            "startWidth": 0.3,
            "endWidth": 0.0,
            "faceCamera": True,
        },
    }


def typed_ribbon_recipe(include_tiling: bool = True) -> dict:
    literals = [
        {
            "propertyPath": "distancetessellationstepsize",
            "kind": "number",
            "value": 5.0,
        },
        {"propertyPath": "lodvalidity", "kind": "number", "value": 3.0},
    ]
    if include_tiling:
        literals.append(
            {
                "propertyPath": "tilingdistance",
                "kind": "number",
                "value": 400.0,
            }
        )
    return {
        "enabled": True,
        "rendererShape": "sprite",
        "emitterDelaySeconds": 0.0,
        "emitterDurationSeconds": 0.8,
        "emitterLoopCount": 1,
        "bursts": [],
        "modules": [
            {
                "stableId": "FX_TEST:export:9@ref:4",
                "className": "particlemoduletypedataribbon",
                "objectPath": "FX_TEST.par_test.particlemoduletypedataribbon_0",
                "literals": literals,
                "distributions": [],
            }
        ],
    }


def ribbon_detail() -> dict:
    return {
        "timing": {
            "startDelaySeconds": 0.0,
            "lifeTimeSeconds": 0.8,
            "afterImageSeconds": 0.0,
            "dissolveStartNormalized": 1.0,
        },
        "trail": {
            "maxPoints": 64,
            "pointLifeTimeSeconds": 0.35,
            "sampleIntervalSeconds": 0.0166667,
            "minimumDistance": 0.01,
            "startWidth": 0.2,
            "endWidth": 0.0,
            "faceCamera": True,
        },
    }


class ValtanTrailAdapterPacketTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.repository_root = Path(self.temporary.name)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def add_resource(self, asset_id: str, payload: bytes = b"DDS fixture") -> None:
        path = self.repository_root / "Client/Bin/Resources" / Path(
            *asset_id.split("/")
        )
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(payload)

    def build(
        self,
        source_inventory: dict,
        source_selections: dict,
        *,
        ribbon_evidence: dict[str, dict] | None = None,
    ) -> dict:
        return adapters.build_document(
            self.repository_root,
            source_inventory,
            source_selections,
            ribbon_evidence=ribbon_evidence,
            enrich_source_graphs=False,
            require_whirlwind_canary=False,
        )

    def test_branch_id_drift_joins_only_reviewed_reachable_coordinate(self) -> None:
        reachable = occurrence(
            stage=2,
            source_type="Trails",
            asset={
                "className": "EFData_AnimNotify_Trails",
                "objectPath": "Pkg.Other_Trail",
            },
        )
        source_only = occurrence(
            stage=3,
            source_type="Trails",
            asset={
                "className": "EFData_AnimNotify_Trails",
                "objectPath": "Pkg.Source_Only_Trail",
            },
        )
        document = self.build(
            inventory([reachable, source_only]),
            selection(2, "valtan.test.active.clip.01"),
        )
        self.assertEqual(1, len(document["adapters"]))
        row = document["adapters"][0]
        self.assertEqual("valtan.test.active.clip.01", row["sourceIdentity"]["clipOccurrenceId"])
        self.assertEqual(adapters.UNRESOLVED, row["disposition"])
        self.assertEqual(0, document["summary"]["sourceOnlyInsertedClipCount"])
        self.assertIn(
            "sequence-003",
            row["evidence"]["reviewedSelection"]["selectedBranchId"],
        )
        self.assertIn(
            "branch-001", row["evidence"]["inventoryBranch"]["branchId"]
        )

    def test_animation_trail_without_exact_history_is_retained_unresolved(self) -> None:
        row = occurrence(
            stage=2,
            source_type="Trails",
            asset={
                "className": "EFData_AnimNotify_Trails",
                "objectPath": "MN_RPBF_00.Other_Trail",
            },
        )
        document = self.build(
            inventory([row]), selection(2, "valtan.test.active.clip.01")
        )
        adapter = document["adapters"][0]
        self.assertEqual(adapters.FAMILY_ANIMATION_TRAIL, adapter["family"])
        self.assertEqual(adapters.UNRESOLVED, adapter["disposition"])
        self.assertIn(
            "ANIMATION_TRAIL_EXACT_BAKED_EDGE_HISTORY_MISSING",
            adapter["admissionBlockers"],
        )

    def test_trail_ghost_requires_and_admits_exact_two_point_edge_history(self) -> None:
        asset_id = "Effect/Valtan/Textures/fixture_trail.dds"
        self.add_resource(asset_id)
        unresolved = occurrence(stage=2, source_type="TrailGhostEffect")
        missing_document = self.build(
            inventory([unresolved]), selection(2, "valtan.test.active.clip.01")
        )
        self.assertEqual(adapters.UNRESOLVED, missing_document["adapters"][0]["disposition"])

        admitted = copy.deepcopy(unresolved)
        admitted["runtimeAdapterEvidence"] = {
            "edgeHistory": edge_history(),
            "target": trail_target(asset_id),
        }
        document = self.build(
            inventory([admitted]), selection(2, "valtan.test.active.clip.01")
        )
        adapter = document["adapters"][0]
        self.assertEqual(adapters.FAMILY_TRAIL_GHOST, adapter["family"])
        self.assertEqual(adapters.ADMITTED, adapter["disposition"])
        self.assertEqual(
            "EFFECT_TYPED_ANIMATION_TRAIL_BAKED_EDGE_V1",
            adapter["packet"]["runtimeCarrier"],
        )
        self.assertEqual(
            "SUPPRESSED_INSUFFICIENT_POINTS",
            adapters.renderer_probe_disposition(adapter, 1),
        )
        self.assertEqual(
            "PREPARED_NONZERO_DRAW",
            adapters.renderer_probe_disposition(adapter, 2),
        )

        malformed = copy.deepcopy(admitted)
        malformed["runtimeAdapterEvidence"]["target"]["trail"]["maxPoints"] = "bad"
        rejected = self.build(
            inventory([malformed]), selection(2, "valtan.test.active.clip.01")
        )["adapters"][0]
        self.assertEqual(adapters.UNRESOLVED, rejected["disposition"])
        self.assertIn(
            "TRAIL_GHOST_TARGET_TRAIL_CONTRACT_INVALID",
            rejected["admissionBlockers"],
        )

    def test_cascade_ribbon_admits_only_typed_recipe_and_runtime_resource(self) -> None:
        asset_id = "Effect/Valtan/Textures/fixture_ribbon.dds"
        self.add_resource(asset_id)
        carrier_key = (
            "system=fx_test.par_test|emitter=FX_TEST:export:1|"
            "emitterOccurrence=0|lod=FX_TEST:export:2|moduleOrder=" + "c" * 64
        )
        system = {
            "sourceSystemId": "fx_test.par_test",
            "catalogSourceAsset": "FX_TEST.par_test",
            "explicitGenericDust": False,
            "carriers": [
                {
                    "carrierKey": carrier_key,
                    "runtimeAdapterType": "RIBBON",
                    "disposition": "UNRESOLVED_RUNTIME_ADAPTER",
                }
            ],
        }
        source = occurrence(
            stage=2,
            source_type="ParticleSystem",
            category="particle",
            asset={
                "className": "ParticleSystem",
                "objectPath": "FX_TEST.par_test",
            },
            source_system_id="fx_test.par_test",
        )
        evidence = {
            carrier_key: {
                "sourceRecipe": typed_ribbon_recipe(),
                "detail": ribbon_detail(),
                "runtimeResources": [{"slotId": "base", "assetId": asset_id}],
                "emitterPath": "FX_TEST.par_test.particlespriteemitter_0",
                "material": {
                    "templateId": "effect.standard",
                    "sourceMaterialPath": "fx_test.material_ribbon_tr",
                    "renderProfile": "alpha_two_sided_depth_read",
                    "sourceProfile": {"enabled": False},
                },
            }
        }
        document = self.build(
            inventory([source], [system]),
            selection(2, "valtan.test.active.clip.01"),
            ribbon_evidence=evidence,
        )
        adapter = document["adapters"][0]
        self.assertEqual(adapters.FAMILY_CASCADE_RIBBON, adapter["family"])
        self.assertEqual(adapters.ADMITTED, adapter["disposition"])
        self.assertEqual(4.0, adapter["packet"]["tilingDistance"])
        self.assertEqual(0.05, adapter["packet"]["distanceTessellationStepSize"])
        self.assertEqual(
            "PREPARED_NONZERO_DRAW",
            adapters.renderer_probe_disposition(adapter, 2),
        )

        missing_literal = copy.deepcopy(evidence)
        missing_literal[carrier_key]["sourceRecipe"] = typed_ribbon_recipe(False)
        unresolved = self.build(
            inventory([source], [system]),
            selection(2, "valtan.test.active.clip.01"),
            ribbon_evidence=missing_literal,
        )["adapters"][0]
        self.assertEqual(adapters.UNRESOLVED, unresolved["disposition"])
        self.assertIn(
            "CASCADE_RIBBON_TILING_DISTANCE_MISSING",
            unresolved["admissionBlockers"],
        )

    def test_light_and_explicit_dust_are_deferred_not_adapter_rows(self) -> None:
        light = occurrence(stage=2, source_type="Light", category="light")
        dust = occurrence(
            stage=2,
            source_type="ParticleSystem",
            category="particle",
            asset={"className": "ParticleSystem", "objectPath": "FX.Dust.par_dust"},
            source_system_id="fx.dust.par_dust",
        )
        system = {
            "sourceSystemId": "fx.dust.par_dust",
            "explicitGenericDust": True,
            "carriers": [],
        }
        document = self.build(
            inventory([light, dust], [system]),
            selection(2, "valtan.test.active.clip.01"),
        )
        self.assertEqual([], document["adapters"])
        self.assertEqual(
            1, document["summary"]["excludedDeferredLightOccurrenceCount"]
        )
        self.assertEqual(
            1,
            document["summary"]["excludedExplicitGenericDustOccurrenceCount"],
        )

    def test_whirlwind_canary_remains_409_1_2_and_9_of_9_byte_identical(self) -> None:
        canary = adapters.load_whirlwind_canary(ROOT)
        self.assertEqual(
            "AUTHORED_AND_RUNTIME_SLICE_BYTE_IDENTICAL", canary["status"]
        )
        self.assertEqual(409, canary["sampleCount"])
        self.assertAlmostEqual(1.2000000476837158, canary["playbackClampSeconds"])
        self.assertEqual(9, canary["effectElementCount"])
        self.assertEqual(5, canary["coreElementCount"])
        self.assertEqual(3, canary["animationTrailTargetCount"])
        self.assertEqual(1, canary["deferredLightCount"])
        self.assertEqual(
            adapters.WHIRLWIND_PROJECTED_DOCUMENT_SHA256,
            canary["runtimeProjectedDocumentSha256"],
        )
        self.assertEqual(
            adapters.WHIRLWIND_RUNTIME_PROGRAM_SHA256,
            canary["runtimeProgramSha256"],
        )

    def test_schema_parses_and_generated_document_validates(self) -> None:
        schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
        self.assertEqual(
            "lostark.valtan-trail-adapter-packets",
            schema["properties"]["schema"]["const"],
        )
        row = occurrence(
            stage=2,
            source_type="Trails",
            asset={
                "className": "EFData_AnimNotify_Trails",
                "objectPath": "Pkg.Other_Trail",
            },
        )
        document = self.build(
            inventory([row]), selection(2, "valtan.test.active.clip.01")
        )
        adapters.validate_document(document, self.repository_root)
        try:
            import jsonschema
        except ImportError:
            return
        jsonschema.Draft202012Validator.check_schema(schema)
        jsonschema.validate(document, schema)


if __name__ == "__main__":
    unittest.main()
