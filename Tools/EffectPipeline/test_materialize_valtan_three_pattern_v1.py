#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock


sys.path.insert(0, str(Path(__file__).resolve().parent))

import materialize_valtan_three_pattern_v1 as materializer


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


def _artifact_json(artifacts: dict[Path, bytes], path: Path) -> dict:
    return json.loads(artifacts[path].decode("utf-8"))


class ValtanThreePatternV1MaterializerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.artifacts = materializer.build_artifacts(REPOSITORY_ROOT)
        cls.fragment = _artifact_json(
            cls.artifacts, materializer.FRAGMENT_RELATIVE_PATH
        )
        cls.receipt = _artifact_json(
            cls.artifacts, materializer.RECEIPT_RELATIVE_PATH
        )
        cls.aliases = _artifact_json(
            cls.artifacts, materializer.ALIAS_RELATIVE_PATH
        )
        cls.catalog = _artifact_json(
            cls.artifacts, materializer.CATALOG_RELATIVE_PATH
        )

    def test_committed_artifacts_are_byte_current(self) -> None:
        for relative_path, expected in self.artifacts.items():
            path = REPOSITORY_ROOT / relative_path
            self.assertTrue(path.is_file(), relative_path.as_posix())
            self.assertEqual(expected, path.read_bytes(), relative_path.as_posix())

    def test_exact_target_scope_and_fidelity_labels(self) -> None:
        summary = self.receipt["summary"]
        self.assertEqual(6, summary["sourceEffectCount"])
        self.assertEqual(6, summary["v1EffectCount"])
        self.assertEqual(41, summary["occurrenceCount"])
        self.assertEqual(37, summary["bindingCount"])
        self.assertEqual(
            {"PROJECT_TUNED_APPROX": 37, "V0_PRESENTATION_CARRY": 4},
            summary["classificationCounts"],
        )
        self.assertFalse(self.receipt["fidelityPolicy"]["claimSourceExact"])
        self.assertNotIn("SOURCE_EXACT", {
            row["classification"] for row in self.receipt["rows"]
        })

    def test_v0_cues_and_source_documents_are_immutable_inputs(self) -> None:
        immutable = self.receipt["immutableInputs"]
        cue_path = REPOSITORY_ROOT / immutable["cuePath"]
        self.assertEqual(
            immutable["cueSha256"], materializer._sha256_file(cue_path)
        )
        cue_document = json.loads(cue_path.read_text(encoding="utf-8"))
        cue_ids = {
            row["effectAssetId"]
            for row in cue_document["cues"]
            if row["patternId"] in materializer.TARGET_PATTERNS
        }
        alias_sources = {row["effectAssetId"] for row in self.aliases["aliases"]}
        alias_targets = {row["v1EffectAssetId"] for row in self.aliases["aliases"]}
        self.assertEqual(alias_sources, cue_ids)
        self.assertTrue(all(not value.endswith(".v1.unified") for value in cue_ids))
        self.assertTrue(all(value.endswith(".v1.unified") for value in alias_targets))

        for source in immutable["sourceDocuments"]:
            source_path = REPOSITORY_ROOT / source["path"]
            self.assertEqual(
                source["sha256"], materializer._sha256_file(source_path)
            )
            source_document = json.loads(source_path.read_text(encoding="utf-8"))
            self.assertEqual(source["effectAssetId"], source_document["effectAssetId"])
            self.assertFalse(source_document["effectAssetId"].endswith(".v1.unified"))

    def test_catalog_exposes_six_parallel_v1_documents(self) -> None:
        by_id = {row["effectAssetId"]: row for row in self.catalog["effects"]}
        self.assertEqual(len(by_id), len(self.catalog["effects"]))
        for alias in self.aliases["aliases"]:
            v1_id = alias["v1EffectAssetId"]
            entry = by_id[v1_id]
            self.assertEqual("DIRECT_AUTHORED_DOCUMENT_V13", entry["payloadKind"])
            self.assertTrue(entry["authoringPath"].endswith(".v1.unified.effect.json"))
            document = json.loads(
                (REPOSITORY_ROOT / "Data" / entry["authoringPath"]).read_text(
                    encoding="utf-8"
                )
            )
            self.assertEqual(v1_id, document["effectAssetId"])
            self.assertEqual(13, document["version"])
            self.assertTrue(document["displayName"].endswith(" [V1]"))
            self.assertLessEqual(len(document["displayName"].encode("utf-8")), 64)

    def test_v1_display_name_truncates_only_at_utf8_boundaries(self) -> None:
        result = materializer._v1_display_name("가" * 40, "effect.test.long")
        self.assertTrue(result.endswith(" [V1]"))
        self.assertLessEqual(len(result.encode("utf-8")), 64)
        self.assertNotIn("\ufffd", result)

    def test_fragment_and_inline_mirrors_are_one_to_one(self) -> None:
        programs = {row["programId"]: row for row in self.fragment["programs"]}
        layouts = {row["layoutId"]: row for row in self.fragment["layouts"]}
        descriptors = {
            row["descriptorId"]: row for row in self.fragment["descriptors"]
        }
        bindings = {
            (row["effectAssetId"], row["elementId"]): row
            for row in self.fragment["bindings"]
        }
        self.assertEqual(2, len(programs))
        self.assertEqual({1001, 1002}, {row["opcode"] for row in programs.values()})
        self.assertEqual(2, len(layouts))
        self.assertEqual(37, len(descriptors))
        self.assertEqual(37, len(bindings))

        catalog_by_id = {
            row["effectAssetId"]: row for row in self.catalog["effects"]
        }
        for identity, binding in bindings.items():
            effect_id, element_id = identity
            catalog_row = catalog_by_id[effect_id]
            document = json.loads(
                (REPOSITORY_ROOT / "Data" / catalog_row["authoringPath"]).read_text(
                    encoding="utf-8"
                )
            )
            element = next(row for row in document["elements"] if row["id"] == element_id)
            execution = element["material"]["execution"]
            self.assertFalse(
                element["material"].get("sourceProfile", {}).get("enabled", False)
            )
            program = programs[binding["programId"]]
            layout = layouts[binding["layoutId"]]
            descriptor = descriptors[binding["descriptorId"]]

            self.assertEqual("INLINE_MIRROR_REQUIRED", binding["inlineMirrorPolicy"])
            self.assertEqual(program["opcode"], execution["opcode"])
            self.assertEqual(layout["textureLaneCount"], execution["textureLaneCount"])
            self.assertEqual(layout["textureMask"], execution["textureMask"])
            self.assertEqual(0, execution["dynamicConsumedMask"])
            self.assertEqual(15, execution["dynamicSuppressedMask"])
            self.assertEqual(2, execution["particleColorPolicy"])
            self.assertEqual(15, execution["particleColorConsumedMask"])
            self.assertEqual(6, execution["renderInputCount"])
            self.assertEqual(47, execution["renderConsumedMask"])
            self.assertEqual(16, execution["renderSuppressedMask"])
            self.assertEqual(1, execution["scalarCount"])
            self.assertEqual(1, execution["inputCount"])
            self.assertEqual([1, 0], execution["inputConsumedMask"])
            self.assertEqual(descriptor["scalars"], [
                {
                    "name": "coverage-channel-selector.0",
                    "value": execution["scalars"][0]["value"],
                }
            ])
            self.assertEqual(
                descriptor["textureLanes"][0]["assetId"],
                execution["textureLanes"][0]["assetId"],
            )
            self.assertEqual(
                layout["textureLanes"][0]["colorSpace"],
                execution["textureLanes"][0]["colorSpace"],
            )

    def test_ribbon_and_light_are_preserved_without_v1_execution_invention(self) -> None:
        immutable_by_id = {
            row["effectAssetId"]: row
            for row in self.receipt["immutableInputs"]["sourceDocuments"]
        }
        carry_rows = [
            row for row in self.receipt["rows"]
            if row["classification"] == "V0_PRESENTATION_CARRY"
        ]
        self.assertEqual({"RIBBON", "PRESENTATION"}, {
            row["carrierKind"] for row in carry_rows
        })
        for row in carry_rows:
            source = immutable_by_id[row["sourceEffectAssetId"]]
            source_document = json.loads(
                (REPOSITORY_ROOT / source["path"]).read_text(encoding="utf-8")
            )
            target_document = json.loads(
                (REPOSITORY_ROOT / source["v1Path"]).read_text(encoding="utf-8")
            )
            source_element = next(
                element for element in source_document["elements"]
                if element["id"] == row["elementId"]
            )
            target_element = next(
                element for element in target_document["elements"]
                if element["id"] == row["elementId"]
            )
            self.assertEqual(source_element, target_element)
            self.assertIsNone(row["binding"])

    def test_texture_selection_is_deterministic_and_backed_by_resources(self) -> None:
        slot_counts: dict[str, int] = {}
        channel_counts: dict[str, int] = {}
        for row in self.receipt["rows"]:
            if row["classification"] != "PROJECT_TUNED_APPROX":
                continue
            selection = row["textureSelection"]
            selected_pair = {
                "slotId": selection["selectedSlotId"],
                "assetId": selection["selectedAssetId"],
            }
            self.assertEqual(selected_pair, selection["candidates"][0])
            self.assertTrue(
                (REPOSITORY_ROOT / "Client/Bin/Resources" / selection["selectedAssetId"]).is_file()
            )
            self.assertIn(selection["colorSpace"], {"linear", "srgb"})
            self.assertIn(selection["coverageChannel"], {"A", "R", "RGB"})
            self.assertEqual(
                materializer.COVERAGE_CHANNEL_SELECTOR[
                    selection["coverageChannel"]
                ],
                selection["selectorValue"],
            )
            self.assertEqual(
                selection["ddsSha256"],
                materializer._sha256_file(
                    REPOSITORY_ROOT / "Client/Bin/Resources" /
                    selection["selectedAssetId"]
                ),
            )
            self.assertIn(
                selection["basis"],
                {
                    "DDS_ALPHA_NONCONSTANT",
                    "DDS_RGB_LUMINANCE_NONCONSTANT",
                    "AUTHORED_MASK_DDS_ALPHA",
                    "AUTHORED_MASK_DDS_RGB_LUMINANCE",
                },
            )
            slot_counts[selection["selectedSlotId"]] = (
                slot_counts.get(selection["selectedSlotId"], 0) + 1
            )
            channel_counts[selection["coverageChannel"]] = (
                channel_counts.get(selection["coverageChannel"], 0) + 1
            )
        self.assertEqual({"base": 23, "mask": 14}, slot_counts)
        self.assertEqual({"A": 9, "RGB": 28}, channel_counts)

    def test_unknown_dds_coverage_evidence_fails_closed(self) -> None:
        row = next(
            row for row in self.receipt["rows"]
            if row["classification"] == "PROJECT_TUNED_APPROX"
        )
        source = next(
            item for item in self.receipt["immutableInputs"]["sourceDocuments"]
            if item["effectAssetId"] == row["sourceEffectAssetId"]
        )
        document = json.loads(
            (REPOSITORY_ROOT / source["path"]).read_text(encoding="utf-8")
        )
        element = next(
            item for item in document["elements"]
            if item["id"] == row["elementId"]
        )
        with mock.patch.dict(
            materializer.COVERAGE_TEXTURE_EVIDENCE, {}, clear=True
        ):
            selected, evidence, color_space, candidates, rejected = (
                materializer._select_texture(REPOSITORY_ROOT, element)
            )
        self.assertIsNone(selected)
        self.assertIsNone(evidence)
        self.assertIsNone(color_space)
        self.assertTrue(candidates)
        self.assertTrue(rejected)
        self.assertEqual(
            {"DDS_COVERAGE_NOT_AUDITED"},
            {item["reasonCode"] for item in rejected},
        )

    def test_check_mode_rejects_missing_or_drifted_artifact(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            relative = Path("generated/example.json")
            artifacts = {relative: b"{}\n"}
            with self.assertRaisesRegex(ValueError, "missing"):
                materializer._write_or_check(root, artifacts, check=True)
            path = root / relative
            path.parent.mkdir(parents=True)
            path.write_bytes(b"drift\n")
            with self.assertRaisesRegex(ValueError, "drifted"):
                materializer._write_or_check(root, artifacts, check=True)
            materializer._write_or_check(root, artifacts, check=False)
            materializer._write_or_check(root, artifacts, check=True)

    def test_fail_closed_on_unknown_source_render_state(self) -> None:
        source_row = next(
            row for row in self.receipt["rows"]
            if row["classification"] == "PROJECT_TUNED_APPROX"
        )
        ledger = json.loads(
            (REPOSITORY_ROOT / materializer.LEDGER_RELATIVE_PATH).read_text(
                encoding="utf-8"
            )
        )
        row = next(
            candidate for candidate in ledger["rows"]
            if candidate["occurrence"]["effectAssetId"] == source_row["sourceEffectAssetId"]
            and candidate["occurrence"]["elementId"] == source_row["elementId"]
        )
        corrupted = copy.deepcopy(row)
        corrupted["adapter"]["sourceRenderState"]["blendMode"] = "MASKED"
        with self.assertRaisesRegex(ValueError, "blend mode"):
            materializer._render_contract(corrupted)


if __name__ == "__main__":
    unittest.main()
