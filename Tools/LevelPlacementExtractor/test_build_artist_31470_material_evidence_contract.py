#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import tempfile
import unittest
from pathlib import Path

from build_artist_31470_material_evidence_contract import (
    build_contract,
    canonical_sha256,
    check_or_write_tracked_json,
    load_json,
    normalize_tracked_text_bytes,
    raw_file_sha256,
    validate_contract,
)


REPO_ROOT = Path(__file__).resolve().parents[2]
ACTIVE_INVENTORY = REPO_ROOT / (
    "Data/Effects/Imported/Artist/"
    "skill.31470.source-active-effect-inventory.receipt.json"
)
MATERIAL_CLOSURE = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.active-material-closure.json"
)
DDS_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.exact-dds-recovery.receipt.json"
)
RENDER_RECEIPT = REPO_ROOT / (
    "Data/Effects/Imported/Artist/Materials/"
    "skill.31470.material-render-state-evidence.receipt.json"
)


def seal_receipt(receipt: dict) -> None:
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = canonical_sha256(receipt)


class Artist31470MaterialEvidenceContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.active = load_json(ACTIVE_INVENTORY)
        cls.closure = load_json(MATERIAL_CLOSURE)
        cls.dds = load_json(DDS_RECEIPT)
        cls.render = load_json(RENDER_RECEIPT)
        cls.contract = build_contract(
            cls.active, cls.closure, cls.dds, cls.render
        )

    def build(
        self,
        *,
        active: dict | None = None,
        closure: dict | None = None,
        dds: dict | None = None,
        render: dict | None = None,
    ) -> dict:
        return build_contract(
            active if active is not None else copy.deepcopy(self.active),
            closure if closure is not None else copy.deepcopy(self.closure),
            dds if dds is not None else copy.deepcopy(self.dds),
            render if render is not None else copy.deepcopy(self.render),
        )

    def test_baseline_denominators_and_product_gate(self) -> None:
        summary = self.contract["summary"]
        self.assertEqual(summary["materialRecipeCount"], 27)
        self.assertEqual(summary["materialOccurrenceCount"], 34)
        self.assertEqual(summary["scalarOverrideCount"], 342)
        self.assertEqual(summary["vectorOverrideCount"], 19)
        self.assertEqual(summary["directTextureOverrideCount"], 71)
        self.assertEqual(summary["directTextureExactSamplerCount"], 3)
        self.assertEqual(summary["directTextureUnprovenSamplerCount"], 68)
        self.assertEqual(summary["parentDefaultExactSamplerCount"], 1)
        self.assertEqual(summary["arithmeticFamilyCount"], 23)
        self.assertEqual(summary["cookedStrippedNullExpressionCount"], 1803)
        self.assertEqual(summary["unresolvedGraphEdgeCount"], 502)
        self.assertEqual(self.contract["admission"]["productRecipeCount"], 0)
        self.assertEqual(self.contract["admission"]["productOccurrenceCount"], 0)

    def test_builtin_light_is_not_a_material_recipe_or_parameter(self) -> None:
        paths = {
            row["sourceMaterialPath"].casefold()
            for row in self.contract["materialRecipes"]
        }
        self.assertNotIn("enginematerials.defaultparticle", paths)
        self.assertEqual(
            self.contract["summary"]["engineBuiltinMaterialOccurrenceCount"], 1
        )

    def test_duplicate_and_blank_parameter_names_fail_closed(self) -> None:
        for replacement, pattern in (("", "blank parameter name"), (None, "duplicate parameter name")):
            with self.subTest(pattern=pattern):
                closure = copy.deepcopy(self.closure)
                material = next(
                    row["material"] for row in closure["materials"]
                    if row.get("material")
                    and len(row["material"].get("scalarParameters", [])) >= 2
                )
                if replacement == "":
                    material["scalarParameters"][0]["name"] = ""
                else:
                    material["scalarParameters"][1]["name"] = material[
                        "scalarParameters"
                    ][0]["name"]
                with self.assertRaisesRegex(ValueError, pattern):
                    self.build(closure=closure)

    def test_duplicate_material_path_and_parent_cycle_fail_closed(self) -> None:
        duplicate = copy.deepcopy(self.closure)
        rendered = [row for row in duplicate["materials"] if row.get("material")]
        rendered[1]["sourceMaterialPath"] = rendered[0]["sourceMaterialPath"]
        with self.assertRaisesRegex(ValueError, "duplicate material closure path"):
            self.build(closure=duplicate)

        cycle = copy.deepcopy(self.closure)
        rendered = [row for row in cycle["materials"] if row.get("material")]
        rendered[0]["material"]["parent"] = rendered[1]["sourceMaterialPath"]
        rendered[1]["material"]["parent"] = rendered[0]["sourceMaterialPath"]
        with self.assertRaisesRegex(ValueError, "material parent cycle"):
            self.build(closure=cycle)

    def test_package_hash_path_and_inheritance_edge_fail_closed(self) -> None:
        closure = copy.deepcopy(self.closure)
        row = next(row for row in closure["materials"] if row.get("material"))
        row["sourcePhysicalPackageSha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "raw source export identity"):
            self.build(closure=closure)

        closure = copy.deepcopy(self.closure)
        row = next(row for row in closure["materials"] if row.get("material"))
        row["material"]["objectPath"] += "_missing"
        with self.assertRaisesRegex(ValueError, "raw source export identity"):
            self.build(closure=closure)

        render = copy.deepcopy(self.render)
        binding = render["bindings"][0]
        source = next(
            row for row in render["exports"]
            if row["evidenceId"] == binding["sourceExportEvidenceId"]
        )
        source["fields"]["parent"]["resolvedObjectPath"] = "wrong.parent"
        seal_receipt(render)
        with self.assertRaisesRegex(ValueError, "exact inheritance edge"):
            self.build(render=render)

    def test_dds_hash_sampler_origin_and_shadowing_fail_closed(self) -> None:
        dds = copy.deepcopy(self.dds)
        dds["assets"][0]["dds"]["sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "DDS raw hash drift"):
            self.build(dds=dds)

        dds = copy.deepcopy(self.dds)
        dds["assets"][3]["sourceMaterialBinding"][
            "bindingEvidence"
        ] = "MATERIAL_INSTANCE_OVERRIDE_EXACT"
        with self.assertRaisesRegex(ValueError, "DDS binding drift"):
            self.build(dds=dds)

        dds = copy.deepcopy(self.dds)
        dds["assets"][0]["sourceMaterialBinding"][
            "materialPhysicalPackageSha256"
        ] = "0" * 64
        with self.assertRaisesRegex(ValueError, "DDS material package identity"):
            self.build(dds=dds)

        closure = copy.deepcopy(self.closure)
        transition = next(
            row for row in closure["materials"]
            if folded_path(row.get("sourceMaterialPath"))
            == "fx_m_mi_o_00.fx_mi.fx_o_transition_05_2_ma"
        )
        transition["material"]["textureParameters"][0][
            "name"
        ] = "dissolve_texture"
        with self.assertRaisesRegex(
            ValueError, "DDS input join is not unique|shadowed|denominator"
        ):
            self.build(closure=closure)

        contract = copy.deepcopy(self.contract)
        field = next(
            field
            for recipe in contract["materialRecipes"]
            for field in recipe["inputs"]["textureOverrides"]
            if field["sampler"]["fidelity"] == "UNRESOLVED"
        )
        field["sampler"] = {
            "bindingId": "fabricated",
            "fidelity": "SOURCE_EXACT_SAMPLER",
            "provenance": {"ddsSha256": "0" * 64},
        }
        with self.assertRaisesRegex(ValueError, "sampler field set was laundered"):
            validate_contract(contract, verify_digest=False)

    def test_cooked_stripped_edge_and_graph_exact_laundering_fail_closed(self) -> None:
        closure = copy.deepcopy(self.closure)
        row = next(row for row in closure["materials"] if row.get("material"))
        holder = row.get("materialGraph") or row.get("parentGraph")
        holder["graph"]["summary"]["nullExpressionCount"] = 0
        holder["graph"]["summary"]["unresolvedInputEdgeCount"] = 0
        with self.assertRaisesRegex(ValueError, "cooked-stripped evidence vanished"):
            self.build(closure=closure)

        contract = copy.deepcopy(self.contract)
        family = contract["graphFamilies"][0]
        family["graphProvenance"] = "SOURCE_EXACT_GRAPH"
        family["sourceExactGraph"] = True
        with self.assertRaisesRegex(ValueError, "laundered as Source exact"):
            validate_contract(contract, verify_digest=False)

    def test_static_switch_flag_is_not_a_selected_permutation(self) -> None:
        contract = copy.deepcopy(self.contract)
        recipe = next(
            row for row in contract["materialRecipes"]
            if row["staticPermutation"]["resourcePresentFlag"] is True
        )
        recipe["staticPermutation"]["selectedParameters"] = [
            {"name": "fake", "value": True}
        ]
        recipe["staticPermutation"]["fidelity"] = (
            "SOURCE_EXACT_STATIC_PERMUTATION"
        )
        recipe["staticPermutation"]["sourceExact"] = True
        with self.assertRaisesRegex(ValueError, "static permutation was laundered"):
            validate_contract(contract, verify_digest=False)

    def test_partial_two_sided_does_not_open_full_cull(self) -> None:
        contract = copy.deepcopy(self.contract)
        recipe = next(
            row for row in contract["materialRecipes"]
            if row["renderState"]["partialCullExact"]
        )
        recipe["renderState"]["fullCullModeExact"] = True
        with self.assertRaisesRegex(ValueError, "promoted to full exact"):
            validate_contract(contract, verify_digest=False)

    def test_corrupt_explicit_render_state_value_fails_closed(self) -> None:
        render = copy.deepcopy(self.render)
        export = next(
            row for row in render["exports"]
            if row["fields"].get("blendmode", {}).get("status")
            == "SERIALIZED_EXPLICIT"
        )
        export["fields"]["blendmode"]["value"] = 7
        seal_receipt(render)
        with self.assertRaisesRegex(ValueError, "render enum field is invalid"):
            self.build(render=render)

    def test_reconstructed_evaluator_cannot_be_promoted_to_exact(self) -> None:
        contract = copy.deepcopy(self.contract)
        evaluator = contract["graphFamilies"][0]["evaluator"]
        evaluator["fidelity"] = "SOURCE_EXACT_INPUT"
        evaluator["sourceExact"] = True
        evaluator["implemented"] = True
        with self.assertRaisesRegex(ValueError, "evaluator fidelity was laundered"):
            validate_contract(contract, verify_digest=False)

    def test_recipe_occurrence_and_aggregate_blocker_loss_fail_closed(self) -> None:
        contract = copy.deepcopy(self.contract)
        recipe = contract["materialRecipes"][0]
        recipe["blockers"].remove("COOKED_STRIPPED_ARITHMETIC_GRAPH")
        recipe["blockerCount"] -= 1
        with self.assertRaisesRegex(ValueError, "required recipe blocker"):
            validate_contract(contract, verify_digest=False)

        contract = copy.deepcopy(self.contract)
        contract["occurrences"][0]["blockers"] = []
        contract["occurrences"][0]["blockerCount"] = 0
        with self.assertRaisesRegex(ValueError, "occurrence blocker set diverged"):
            validate_contract(contract, verify_digest=False)

        contract = copy.deepcopy(self.contract)
        contract["admission"]["blockers"] = []
        with self.assertRaisesRegex(ValueError, "aggregate blocker set"):
            validate_contract(contract, verify_digest=False)

    def test_occurrence_join_loss_fails_closed(self) -> None:
        active = copy.deepcopy(self.active)
        active["activeElements"].pop()
        with self.assertRaisesRegex(ValueError, "active element denominator changed"):
            self.build(active=active)

    def test_tracked_json_allows_only_eol_equivalence(self) -> None:
        value = {"a": 1, "b": 1.0}
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "derived.json"
            check_or_write_tracked_json(path, value, check=False)
            path.write_bytes(path.read_bytes().replace(b"\n", b"\r\n"))
            check_or_write_tracked_json(path, value, check=True)
            path.write_text('{\n  "b": 1.0,\n  "a": 1\n}\n', encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "stale"):
                check_or_write_tracked_json(path, value, check=True)

    def test_external_artifact_hash_is_raw_not_json_normalized(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            left = Path(directory) / "external-lf.json"
            right = Path(directory) / "external-crlf.json"
            left.write_bytes(b'{"a":1}\n')
            right.write_bytes(b'{"a":1}\r\n')
            self.assertNotEqual(raw_file_sha256(left), raw_file_sha256(right))

    def test_duplicate_json_object_key_is_corrupt(self) -> None:
        with self.assertRaisesRegex(ValueError, "duplicate JSON object key"):
            normalize_tracked_text_bytes(b'{"a":1,"a":1}\n')


def folded_path(value: object) -> str:
    return str(value or "").casefold()


if __name__ == "__main__":
    unittest.main()
