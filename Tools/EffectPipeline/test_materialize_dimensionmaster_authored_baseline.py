#!/usr/bin/env python3

from __future__ import annotations

from collections import Counter
import copy
import hashlib
import json
import unittest
from unittest import mock

import materialize_dimensionmaster_authored_baseline as materializer


class AuthoredBaselineMaterializerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source = json.loads(
            materializer.DEFAULT_SOURCE.read_text(encoding="utf-8")
        )
        cls.correction = json.loads(
            materializer.DEFAULT_CORRECTION.read_text(encoding="utf-8")
        )
        cls.source_by_id = {
            element["id"]: element for element in cls.source["elements"]
        }
        cls.imported_recipe = (
            materializer.REPOSITORY_ROOT
            / "Data/Effects/Imported/DimensionMaster/Converted/skill.2050210.action-cue-recipe.json"
        )

    def test_four_hits_materialize_six_standalone_layers_each(self) -> None:
        document = materializer.build_document(self.source, self.correction)
        self.assertEqual(materializer.TARGET_EFFECT_ID, document["effectAssetId"])
        self.assertEqual(24, len(document["elements"]))
        self.assertEqual(
            Counter({"mesh": 20, "sprite": 4}),
            Counter(element["kind"] for element in document["elements"]),
        )

        expected_hit_positions = (
            [-0.55, 0.15, 0.95],
            [-0.60, 0.15, 1.05],
            [-0.65, 0.20, 1.15],
            [-0.70, 0.25, 1.25],
        )
        for hit_index, (hit_contract, hit_position) in enumerate(
            zip(materializer.HIT_SNAPSHOT_CONTRACT, expected_hit_positions, strict=True)
        ):
            hit_id, suffix, source_time = hit_contract
            hit_elements = document["elements"][hit_index * 6 : (hit_index + 1) * 6]
            self.assertEqual(
                [contract[0] for contract in materializer.LAYER_CONTRACT],
                [element["id"].rsplit(".", 1)[-1] for element in hit_elements],
            )
            self.assertEqual(
                {source_time},
                {
                    element["detail"]["timing"]["startDelaySeconds"]
                    for element in hit_elements
                },
            )
            self.assertEqual(
                {tuple(hit_position)},
                {
                    tuple(element["detail"]["transform"]["position"])
                    for element in hit_elements
                },
            )
            for element, layer_contract, layer in zip(
                hit_elements,
                materializer.LAYER_CONTRACT,
                self.correction["layers"],
                strict=True,
            ):
                role, carrier_kind, source_base_id = layer_contract
                source_id = f"{source_base_id}{suffix}"
                source_element = self.source_by_id[source_id]
                expected_kind = (
                    "sprite"
                    if carrier_kind == materializer.STANDALONE_SPRITE
                    else "mesh"
                )
                self.assertEqual(
                    f"authored.baseline.a.{hit_id}.{role}", element["id"]
                )
                self.assertEqual(expected_kind, element["kind"])
                self.assertEqual(
                    f"authored-correction:{materializer.CORRECTION_ID}|source:{source_id}",
                    element["sourceNode"],
                )
                self.assertEqual(source_element["material"], element["material"])
                self.assertTrue(element["material"]["sourceProfile"]["enabled"])
                source_resources = {
                    resource["slotId"]: resource["assetId"]
                    for resource in source_element["resources"]
                }
                output_resources = {
                    resource["slotId"]: resource["assetId"]
                    for resource in element["resources"]
                }
                self.assertEqual(
                    source_resources[layer["emissiveFromSlot"]],
                    output_resources["emissive"],
                )
                self.assertFalse(element["sourceRecipe"]["enabled"])
                self.assertFalse(element["sourcePresentation"]["enabled"])
                self.assertTrue(element["actionCueAttachment"]["enabled"])
                self.assertFalse(element["actionCueAttachment"]["follow"])
                self.assertEqual(
                    "root", element["actionCueAttachment"]["sourceAnchorSlotId"]
                )
                self.assertEqual(
                    "root", element["actionCueAttachment"]["runtimeAnchorSlotId"]
                )
                self.assertEqual("", element["actionCueAttachment"]["runtimeBoneName"])

            sprite = hit_elements[-1]
            self.assertTrue(sprite["detail"]["sprite"]["billboard"])
            self.assertEqual(
                -90.0, sprite["detail"]["sprite"]["billboardRollDegrees"]
            )
        self.assertEqual(0.0, document["particleSystem"]["yawOffsetDegrees"])

    def test_build_does_not_mutate_source_or_imported_recipe(self) -> None:
        source_before = copy.deepcopy(self.source)
        imported_before = hashlib.sha256(self.imported_recipe.read_bytes()).digest()
        materializer.build_document(self.source, self.correction)
        self.assertEqual(source_before, self.source)
        self.assertEqual(
            imported_before, hashlib.sha256(self.imported_recipe.read_bytes()).digest()
        )

    def test_wrong_version_is_rejected(self) -> None:
        correction = copy.deepcopy(self.correction)
        correction["version"] = 1
        with self.assertRaisesRegex(ValueError, "version"):
            materializer.build_document(self.source, correction)

    def test_wrong_target_id_is_rejected(self) -> None:
        correction = copy.deepcopy(self.correction)
        correction["targetEffectAssetId"] = "effect.wrong"
        with self.assertRaisesRegex(ValueError, "targetEffectAssetId"):
            materializer.build_document(self.source, correction)

    def test_duplicate_hit_snapshot_is_rejected(self) -> None:
        correction = copy.deepcopy(self.correction)
        correction["hitSnapshots"][1] = copy.deepcopy(correction["hitSnapshots"][0])
        with self.assertRaisesRegex(ValueError, "Duplicate"):
            materializer.build_document(self.source, correction)

    def test_hit_suffix_drift_is_rejected(self) -> None:
        correction = copy.deepcopy(self.correction)
        correction["hitSnapshots"][1]["sourceEventSuffix"] = ".event_wrong"
        with self.assertRaisesRegex(ValueError, "sourceEventSuffix"):
            materializer.build_document(self.source, correction)

    def test_hit_order_drift_is_rejected(self) -> None:
        correction = copy.deepcopy(self.correction)
        correction["hitSnapshots"][0], correction["hitSnapshots"][1] = (
            correction["hitSnapshots"][1],
            correction["hitSnapshots"][0],
        )
        with self.assertRaisesRegex(ValueError, "order"):
            materializer.build_document(self.source, correction)

    def test_layer_draw_order_drift_is_rejected(self) -> None:
        correction = copy.deepcopy(self.correction)
        correction["layers"][0], correction["layers"][1] = (
            correction["layers"][1],
            correction["layers"][0],
        )
        with self.assertRaisesRegex(ValueError, "draw order"):
            materializer.build_document(self.source, correction)

    def test_exact_source_layer_drift_is_rejected(self) -> None:
        correction = copy.deepcopy(self.correction)
        correction["layers"][0]["sourceElementBaseId"] = (
            "fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1.particlespriteemitter_19"
        )
        with self.assertRaisesRegex(ValueError, "source layer"):
            materializer.build_document(self.source, correction)

    def test_source_resource_path_escape_is_rejected(self) -> None:
        source = copy.deepcopy(self.source)
        source_element = next(
            element
            for element in source["elements"]
            if element["id"] == materializer.LAYER_CONTRACT[0][2]
        )
        base_resource = next(
            resource
            for resource in source_element["resources"]
            if resource["slotId"] == "base"
        )
        base_resource["assetId"] = "Effect/../Map/not-admitted.dds"
        with self.assertRaisesRegex(ValueError, "asset ID"):
            materializer.build_document(source, self.correction)

    def test_hash_drift_is_rejected_before_materialization(self) -> None:
        with mock.patch.object(materializer, "_sha256_file", return_value="0" * 64):
            with self.assertRaisesRegex(ValueError, "source changed"):
                materializer.materialize(
                    materializer.DEFAULT_SOURCE,
                    materializer.DEFAULT_CORRECTION,
                    materializer.DEFAULT_OUTPUT,
                    write=False,
                )

    def test_output_path_drift_is_rejected(self) -> None:
        wrong_output = materializer.DEFAULT_OUTPUT.with_name("effect.wrong.effect.json")
        with self.assertRaisesRegex(ValueError, "stable Authored path"):
            materializer._resolve_output(wrong_output)

    def test_seed_refuses_to_overwrite_effect_tool_document(self) -> None:
        document = materializer.build_document(self.source, self.correction)
        with self.assertRaisesRegex(FileExistsError, "Effect Tool"):
            materializer.write_document_seed(materializer.DEFAULT_OUTPUT, document)


if __name__ == "__main__":
    unittest.main()
