from __future__ import annotations

import copy
import importlib.util
import json
from pathlib import Path
import unittest


SCRIPT_PATH = Path(__file__).with_name(
    "materialize_artist_31490_black_tiger_stroke.py"
)
SPEC = importlib.util.spec_from_file_location("artist_31490_tiger", SCRIPT_PATH)
assert SPEC is not None and SPEC.loader is not None
module = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(module)


class Artist31490BlackTigerStrokeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.outputs, cls.receipt = module.build_projection()
        cls.document = json.loads(cls.outputs[module.TARGET_PATH])
        cls.role_manifest = module.load_json(module.ROLE_MANIFEST_PATH)
        cls.role_receipt = module.load_json(module.ROLE_RECEIPT_PATH)

    def test_exact_twelve_source_order_rows_and_child_split(self) -> None:
        elements = self.document["elements"]
        by_id = {row["id"]: row for row in elements}
        self.assertEqual(len(elements), 68)
        self.assertEqual(set(module.TIGER_IDS), set(by_id) & module.TIGER_ID_SET)
        receipt_rows = {row["targetElementId"]: row for row in self.receipt["rows"]}
        self.assertEqual(
            [receipt_rows[element_id]["sourceOrder"] for element_id in module.TIGER_IDS],
            [row[1] for row in module.TIGER_ROWS],
        )
        self.assertEqual(
            {
                path: sum(
                    by_id[element_id]["material"]["sourceMaterialPath"] == path
                    for element_id in module.TIGER_IDS
                )
                for path in module.VARIANTS
            },
            {module.CHILD5_PATH: 4, module.CHILD6_PATH: 8},
        )

    def test_existing_fifty_six_rows_are_deep_equal_after_filter(self) -> None:
        current = module.load_json(module.TARGET_PATH)
        baseline = [
            copy.deepcopy(row)
            for row in current["elements"]
            if row["id"] not in module.TIGER_ID_SET
        ]
        filtered = [
            row
            for row in self.document["elements"]
            if row["id"] not in module.TIGER_ID_SET
        ]
        self.assertEqual(len(baseline), 56)
        self.assertEqual(filtered, baseline)
        self.assertEqual(
            self.receipt["preservedBaselineCanonicalSha256"],
            module.canonical_sha256(baseline),
        )

        tuned = copy.deepcopy(current)
        ordinary = next(
            row for row in tuned["elements"] if row["id"] not in module.TIGER_ID_SET
        )
        ordinary["displayName"] += " preserved-sentinel"
        tuned_outputs, _ = module.build_projection(target_document=tuned)
        tuned_document = json.loads(tuned_outputs[module.TARGET_PATH])
        tuned_filtered = [
            row
            for row in tuned_document["elements"]
            if row["id"] not in module.TIGER_ID_SET
        ]
        expected_filtered = [
            row for row in tuned["elements"] if row["id"] not in module.TIGER_ID_SET
        ]
        self.assertEqual(tuned_filtered, expected_filtered)

    def test_existing_fifty_six_raw_object_blocks_are_byte_preserved(self) -> None:
        source_text = module.TARGET_PATH.read_text(encoding="utf-8-sig")
        output_text = self.outputs[module.TARGET_PATH]
        _, _, source_by_id, source_blocks = module.extract_element_raw_blocks(
            source_text
        )
        _, _, output_by_id, output_blocks = module.extract_element_raw_blocks(
            output_text
        )
        baseline_ids = [
            row["id"]
            for row in self.document["elements"]
            if row["id"] not in module.TIGER_ID_SET
        ]
        self.assertEqual(len(baseline_ids), 56)
        for element_id in baseline_ids:
            self.assertEqual(output_by_id[element_id], source_by_id[element_id])
            self.assertEqual(output_blocks[element_id], source_blocks[element_id])
        self.assertEqual(
            self.receipt["preservedBaselineRawObjectSequenceSha256"],
            module.raw_block_sequence_sha256(baseline_ids, output_blocks),
        )
        self.assertIs(
            self.receipt["policy"]["preservedBaselineRawObjectBytes"], True
        )

    def test_fresh_fifty_six_row_input_uses_raw_preserving_stable_insert(self) -> None:
        current_text = self.outputs[module.TARGET_PATH]
        head, tail, _, current_blocks = module.extract_element_raw_blocks(
            current_text
        )
        baseline_elements = [
            row
            for row in self.document["elements"]
            if row["id"] not in module.TIGER_ID_SET
        ]
        baseline_text = head + ",\n    ".join(
            current_blocks[row["id"]] for row in baseline_elements
        ) + tail
        baseline_document = json.loads(baseline_text)
        self.assertEqual(len(baseline_document["elements"]), 56)
        inserted = module.serialize_preserving_untouched_elements(
            self.document, baseline_text
        )
        _, _, _, baseline_blocks = module.extract_element_raw_blocks(
            baseline_text
        )
        _, _, _, inserted_blocks = module.extract_element_raw_blocks(inserted)
        for row in baseline_elements:
            self.assertEqual(
                inserted_blocks[row["id"]], baseline_blocks[row["id"]]
            )
        self.assertEqual(json.loads(inserted), self.document)

    def test_packet_variants_pin_channels_color_and_lifetime_policy(self) -> None:
        by_id = {row["id"]: row for row in self.document["elements"]}
        for element_id in module.TIGER_IDS:
            execution = by_id[element_id]["material"]["execution"]
            child = by_id[element_id]["material"]["sourceMaterialPath"]
            expected_count = 28 if child == module.CHILD5_PATH else 24
            self.assertEqual(execution["opcode"], 18)
            self.assertEqual(execution["passIndex"], 2)
            self.assertEqual(execution["textureMask"], 7)
            self.assertEqual(
                [row["sourceChannel"] for row in execution["textureLanes"]],
                ["RGB", "RG", "R"],
            )
            self.assertEqual(
                [row["colorSpace"] for row in execution["textureLanes"]],
                ["linear", "linear", "linear"],
            )
            self.assertEqual(execution["scalarCount"], expected_count)
            self.assertEqual(execution["inputCount"], expected_count)
            self.assertEqual(execution["dynamicConsumedMask"], 15)
            self.assertEqual(execution["particleColorConsumedMask"], 15)
            self.assertEqual(execution["staticInputCount"], 0)
            self.assertEqual(execution["renderConsumedMask"], 47)
            self.assertEqual(execution["renderSuppressedMask"], 16)
        color_contract = self.receipt["policy"]["colorContract"]
        self.assertEqual(color_contract["mainRgb"], "LINEAR_BASE_RADIANCE")
        self.assertEqual(color_contract["mainR"], "EXPLICIT_COVERAGE")
        self.assertEqual(color_contract["mainAlpha"], "SUPPRESSED_DXT1_OPAQUE_ALPHA")
        self.assertEqual(
            color_contract["particleColorAlpha"],
            "INDEPENDENT_LIFETIME_ENVELOPE",
        )
        self.assertEqual(
            color_contract["particleColorRgb"],
            "SOURCE_OWNED_SIGNED_MAGNITUDE_NEUTRAL_ZERO",
        )

    def test_idempotent_replay_is_byte_stable(self) -> None:
        replay_outputs, replay_receipt = module.build_projection(
            target_document=self.document
        )
        self.assertEqual(
            replay_outputs[module.TARGET_PATH], self.outputs[module.TARGET_PATH]
        )
        self.assertEqual(
            replay_receipt["outputCanonicalSha256"],
            self.receipt["outputCanonicalSha256"],
        )

    def test_stale_role_manifest_and_receipt_fail_closed(self) -> None:
        stale_manifest = copy.deepcopy(self.role_manifest)
        stale_manifest["visibleRoleAllowlists"][module.ROLE].pop()
        with self.assertRaises(module.TigerMaterializationError):
            module.build_projection(role_manifest=stale_manifest)

        stale_receipt = copy.deepcopy(self.role_receipt)
        row = next(
            row
            for row in stale_receipt["particleRows"]
            if row["targetElementId"] == module.TIGER_IDS[0]
        )
        row["sourceOrder"] += 1
        with self.assertRaises(module.TigerMaterializationError):
            module.build_projection(role_receipt=stale_receipt)

    def test_partial_insertion_fails_closed(self) -> None:
        partial = copy.deepcopy(self.document)
        partial["elements"] = [
            row for row in partial["elements"] if row["id"] != module.TIGER_IDS[-1]
        ]
        with self.assertRaises(module.TigerMaterializationError):
            module.build_projection(target_document=partial)

    def test_receipt_does_not_claim_native_source_exactness(self) -> None:
        provenance = self.receipt["provenance"]
        self.assertEqual(provenance["fidelity"], "TYPED_SOURCE_RECONSTRUCTION")
        self.assertEqual(provenance["sourceExactness"], "PARTIAL")
        self.assertEqual(provenance["nativeShaderStatus"], "PENDING")
        self.assertEqual(provenance["nativeDxbcStatus"], "PENDING")
        self.assertIs(provenance["nativeDxbcExecuted"], False)


if __name__ == "__main__":
    unittest.main()
