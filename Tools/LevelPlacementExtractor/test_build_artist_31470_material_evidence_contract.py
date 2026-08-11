#!/usr/bin/env python3

from __future__ import annotations

import copy
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from build_artist_31470_material_evidence_contract import (
    build_contract,
    build_from_paths,
    canonical_sha256,
    check_or_write_tracked_json,
    contract_exact_input_lineage_fixture_payload,
    load_json,
    normalize_tracked_text_bytes,
    occurrence_identity_payload,
    raw_file_sha256,
    recipe_family_fixture_payload,
    recipe_composition_payload,
    recipe_identity_fixture_payload,
    stable_id,
    contract_render_field_fixture_payload,
    validate_contract,
    verify_external_artifacts,
)
from extract_artist_31470_material_render_state import build_receipt


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
SOURCE_PACKAGE_ROOT = Path(
    os.environ.get("ARTIST_F_MATERIAL_SOURCE_PACKAGE_ROOT", "__unavailable__")
)
EXACT_DDS_ROOT = Path(
    os.environ.get("ARTIST_F_MATERIAL_EXACT_DDS_ROOT", "__unavailable__")
)
SOURCE_PACK_MANIFEST = Path(
    os.environ.get("ARTIST_F_MATERIAL_SOURCE_PACK_MANIFEST", "__unavailable__")
)


def seal_receipt(receipt: dict) -> None:
    receipt.pop("receiptSha256", None)
    receipt["receiptSha256"] = canonical_sha256(receipt)


def seal_contract(contract: dict) -> None:
    contract.pop("contractSha256", None)
    contract["contractSha256"] = canonical_sha256(contract)


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
        self.assertEqual(self.contract["formatVersion"], 4)
        self.assertEqual(summary["materialRecipeCount"], 27)
        self.assertEqual(summary["materialOccurrenceCount"], 34)
        self.assertEqual(summary["scalarOverrideCount"], 342)
        self.assertEqual(summary["vectorOverrideCount"], 19)
        self.assertEqual(summary["directTextureOverrideCount"], 71)
        self.assertEqual(summary["directTextureExactSamplerCount"], 0)
        self.assertEqual(summary["directTextureUnprovenSamplerCount"], 71)
        self.assertEqual(summary["parentDefaultExactSamplerCount"], 0)
        self.assertEqual(summary["exactSamplerBindingCount"], 0)
        self.assertEqual(summary["previouslyAdmittedExactSamplerBindingCount"], 4)
        self.assertEqual(summary["rejectedSamplerBindingCount"], 4)
        self.assertEqual(summary["strictSamplerExecutionRowCount"], 72)
        self.assertEqual(
            summary["rejectedSamplerBindingSetSha256"],
            "dfc923cab4dd2155385c2c066f261cea689a863c8e8179b48d2677556a849d4c",
        )
        self.assertEqual(self.contract["exactSamplerBindings"], [])
        self.assertEqual(len(self.contract["rejectedSamplerBindings"]), 4)
        self.assertEqual(summary["arithmeticFamilyCount"], 23)
        self.assertEqual(summary["cookedStrippedNullExpressionCount"], 1803)
        self.assertEqual(summary["unresolvedGraphEdgeCount"], 502)
        self.assertEqual(summary["usedMaterialRecipeCount"], 27)
        self.assertEqual(summary["unusedMaterialRecipeCount"], 0)
        self.assertEqual(summary["unexpectedOccurrenceMaterialCount"], 0)
        self.assertEqual(
            summary["occurrenceMaterialJoinSha256"],
            "1c56ff7bf67dc94a61129372a0e71f57a74171ee47ddf57702cd88b95606b296",
        )
        parent_defaults = [
            field
            for recipe in self.contract["materialRecipes"]
            for field in recipe["inputs"]["parentDefaults"]
        ]
        self.assertEqual(len(parent_defaults), 297)
        self.assertTrue(
            all(field["fidelity"] == "SOURCE_EXACT_INPUT" for field in parent_defaults)
        )
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
                with self.assertRaisesRegex(
                    ValueError,
                    f"{pattern}|blank scalarParameters|raw instance parameter projection",
                ):
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

    def test_canonical_material_path_and_selected_parent_graph_are_bound(self) -> None:
        closure = copy.deepcopy(self.closure)
        left = next(
            row for row in closure["materials"]
            if folded_path(row.get("sourceMaterialPath")).endswith(
                "fx_e_me_ht_03_4_ma"
            )
        )
        right = next(
            row for row in closure["materials"]
            if folded_path(row.get("sourceMaterialPath")).endswith(
                "fx_e_pa_fd_18_2_tr"
            )
        )
        left["sourceMaterialPath"], right["sourceMaterialPath"] = (
            right["sourceMaterialPath"],
            left["sourceMaterialPath"],
        )
        with self.assertRaisesRegex(
            ValueError, "raw source export identity|canonical source Material"
        ):
            self.build(closure=closure)

        closure = copy.deepcopy(self.closure)
        left = next(
            row for row in closure["materials"]
            if folded_path(row.get("sourceMaterialPath")).endswith(
                "fx_e_me_ht_03_4_ma"
            )
        )
        right = next(
            row for row in closure["materials"]
            if folded_path(row.get("sourceMaterialPath")).endswith(
                "fx_e_pa_fd_18_2_tr"
            )
        )
        left["parentGraph"], right["parentGraph"] = (
            right["parentGraph"],
            left["parentGraph"],
        )
        with self.assertRaisesRegex(
            ValueError, "raw base Material identity|raw MIC Parent"
        ):
            self.build(closure=closure)

        render = copy.deepcopy(self.render)
        render["bindings"][0]["sourceMaterialIdentity"], render["bindings"][1][
            "sourceMaterialIdentity"
        ] = (
            render["bindings"][1]["sourceMaterialIdentity"],
            render["bindings"][0]["sourceMaterialIdentity"],
        )
        seal_receipt(render)
        with self.assertRaisesRegex(
            ValueError, "canonical source Material identity binding"
        ):
            self.build(render=render)

    def test_instance_parameter_value_and_order_require_raw_projection(self) -> None:
        closure = copy.deepcopy(self.closure)
        row = next(
            row
            for row in closure["materials"]
            if row.get("material") and row["material"].get("scalarParameters")
        )
        row["material"]["scalarParameters"][0]["value"] = 124.0
        with self.assertRaisesRegex(ValueError, "raw instance parameter projection"):
            self.build(closure=closure)

        render = copy.deepcopy(self.render)
        source = next(
            export
            for export in render["exports"]
            if export.get("instanceParameters", {}).get("scalar")
        )
        source["instanceParameters"]["scalar"][0]["value"] = 124.0
        seal_receipt(render)
        with self.assertRaisesRegex(ValueError, "raw instance parameter projection"):
            self.build(render=render)

        render = copy.deepcopy(self.render)
        source = next(
            export
            for export in render["exports"]
            if len(export.get("instanceParameters", {}).get("scalar", [])) >= 2
        )
        source["instanceParameters"]["scalar"][:2] = reversed(
            source["instanceParameters"]["scalar"][:2]
        )
        seal_receipt(render)
        with self.assertRaisesRegex(ValueError, "raw instance parameter projection"):
            self.build(render=render)

    def test_parent_expression_value_requires_raw_export_evidence(self) -> None:
        closure = copy.deepcopy(self.closure)
        expression = next(
            expression
            for row in closure["materials"]
            if row.get("material")
            for expression in (row.get("materialGraph") or row.get("parentGraph"))["graph"]["expressions"]
            if isinstance(expression.get("defaultValue"), (int, float))
            and not isinstance(expression.get("defaultValue"), bool)
            and float(expression["defaultValue"]) == 1.0
        )
        expression["defaultValue"] = 18.0
        with self.assertRaisesRegex(ValueError, "raw graph-expression projection"):
            self.build(closure=closure)

        render = copy.deepcopy(self.render)
        raw_expression = next(
            expression
            for expression in render["graphExpressions"]
            if expression["fields"]["defaultvalue"]["status"]
            == "SERIALIZED_EXPLICIT"
        )
        raw_expression["projection"]["defaultValue"] = 18.0
        raw_expression["fields"]["defaultvalue"]["value"] = 18.0
        seal_receipt(render)
        with self.assertRaisesRegex(ValueError, "raw graph-expression projection"):
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
            ValueError,
            "DDS input join is not unique|shadowed|denominator|raw instance parameter projection",
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
        with self.assertRaisesRegex(
            ValueError,
            "sampler field set was laundered|exact sampler provenance|"
            "omitted sampler defaults were laundered",
        ):
            validate_contract(contract, verify_digest=False)

        contract = copy.deepcopy(self.contract)
        rejected = contract["rejectedSamplerBindings"][0]
        field = next(
            field
            for recipe in contract["materialRecipes"]
            for collection in (
                recipe["inputs"]["textureOverrides"],
                [
                    field
                    for field in recipe["inputs"]["parentDefaults"]
                    if field["fieldKind"] == "texture"
                ],
            )
            for field in collection
            if field["fieldId"] == rejected["inputFieldId"]
        )
        field["sampler"]["fidelity"] = "SOURCE_EXACT_SAMPLER"
        with self.assertRaisesRegex(
            ValueError, "omitted sampler defaults were laundered"
        ):
            validate_contract(contract, verify_digest=False)

    def test_texture2d_class_export_and_serial_join_fail_closed(self) -> None:
        for field_name, value in (
            ("className", "Material"),
            ("exportIndex", 999),
            ("serialSha256", "0" * 64),
        ):
            with self.subTest(field_name=field_name):
                dds = copy.deepcopy(self.dds)
                dds["assets"][0]["sourceTexture2D"][field_name] = value
                with self.assertRaisesRegex(
                    ValueError, "DDS receipt disagrees with raw Texture2D export"
                ):
                    self.build(dds=dds)

        render = copy.deepcopy(self.render)
        render["textureSamplerExports"][0]["export"]["className"] = "Material"
        seal_receipt(render)
        with self.assertRaisesRegex(ValueError, "raw Texture2D identity"):
            self.build(render=render)

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

    def test_per_family_graph_count_redistribution_fails_closed(self) -> None:
        contract = copy.deepcopy(self.contract)
        left, right = contract["graphFamilies"][:2]
        left["cookedEvidence"]["nullExpressionCount"] += 1
        right["cookedEvidence"]["nullExpressionCount"] -= 1
        with self.assertRaisesRegex(ValueError, "count was redistributed"):
            validate_contract(contract, verify_digest=False)

        contract = copy.deepcopy(self.contract)
        left, right = contract["graphFamilies"][:2]
        for evidence_name in ("cookedEvidence", "rawEvidence"):
            left[evidence_name]["nullExpressionCount"] += 1
            left[evidence_name]["nonNullExpressionCount"] -= 1
            right[evidence_name]["nullExpressionCount"] -= 1
            right[evidence_name]["nonNullExpressionCount"] += 1
        with self.assertRaisesRegex(
            ValueError, "raw evidence fixture changed|exact-identity-derived"
        ):
            validate_contract(contract, verify_digest=False)

        closure = copy.deepcopy(self.closure)
        row = next(row for row in closure["materials"] if row.get("material"))
        graph = (row.get("materialGraph") or row.get("parentGraph"))["graph"]
        graph["summary"]["nullExpressionCount"] += 1
        with self.assertRaisesRegex(ValueError, "raw expression evidence"):
            self.build(closure=closure)

    def test_family_exact_identity_and_recipe_join_are_not_swappable(self) -> None:
        contract = copy.deepcopy(self.contract)
        contract["graphFamilies"][0]["exactIdentity"][
            "materialExportIndex"
        ] += 1
        seal_contract(contract)
        with self.assertRaisesRegex(ValueError, "exact-identity-derived"):
            validate_contract(contract)

        contract = copy.deepcopy(self.contract)
        contract["graphFamilies"][:2] = reversed(contract["graphFamilies"][:2])
        seal_contract(contract)
        with self.assertRaisesRegex(ValueError, "stable identity order"):
            validate_contract(contract)

        contract = copy.deepcopy(self.contract)
        left, right = contract["materialRecipes"][:2]
        for field_name in ("arithmeticFamilyId", "arithmeticFamilyEvidence"):
            left[field_name], right[field_name] = (
                right[field_name],
                left[field_name],
            )
        contract["summary"]["recipeFamilyJoinSha256"] = canonical_sha256(
            recipe_family_fixture_payload(contract["materialRecipes"])
        )
        seal_contract(contract)
        with self.assertRaisesRegex(ValueError, "recipe-to-family raw evidence join"):
            validate_contract(contract)

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

        contract = copy.deepcopy(self.contract)
        recipe = next(
            row for row in contract["materialRecipes"]
            if not row["renderState"]["partialCullExact"]
        )
        recipe["renderState"]["partialCullExact"] = True
        contract["summary"]["sourceExactPartialCullRecipeCount"] += 1
        with self.assertRaisesRegex(ValueError, "partial cull exactness"):
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

        render = copy.deepcopy(self.render)
        export = next(
            row for row in render["exports"]
            if row["fields"].get("blendmode", {}).get("status")
            == "SERIALIZED_EXPLICIT"
        )
        export["fields"]["blendmode"]["enumOrdinal"] += 1
        seal_receipt(render)
        with self.assertRaisesRegex(ValueError, "enum domain/ordinal"):
            self.build(render=render)

    def test_render_enum_and_bool_cannot_change_under_original_raw_hashes(self) -> None:
        render = copy.deepcopy(self.render)
        export = next(
            row for row in render["exports"]
            if row["fields"].get("blendmode", {}).get("status")
            == "SERIALIZED_EXPLICIT"
        )
        blend = export["fields"]["blendmode"]
        blend["value"] = (
            "blend_translucent"
            if folded_path(blend["value"]) != "blend_translucent"
            else "blend_additive"
        )
        blend["enumOrdinal"] = blend["enumDomain"].index(blend["value"])
        seal_receipt(render)
        with self.assertRaisesRegex(ValueError, "raw exact render-field evidence"):
            self.build(render=render)

        render = copy.deepcopy(self.render)
        export = next(
            row for row in render["exports"]
            if row["fields"].get("twosided", {}).get("status")
            == "SERIALIZED_EXPLICIT"
        )
        export["fields"]["twosided"]["value"] = not export["fields"][
            "twosided"
        ]["value"]
        seal_receipt(render)
        with self.assertRaisesRegex(ValueError, "boolean disagrees with raw tagged bytes"):
            self.build(render=render)

        contract = copy.deepcopy(self.contract)
        recipe = next(
            row for row in contract["materialRecipes"]
            if row["renderState"]["fields"]["blendmode"]["status"]
            == "SERIALIZED_EXPLICIT"
        )
        blend = recipe["renderState"]["fields"]["blendmode"]
        blend["value"] = (
            "blend_translucent"
            if folded_path(blend["value"]) != "blend_translucent"
            else "blend_additive"
        )
        contract["summary"]["renderFieldEvidenceSha256"] = canonical_sha256(
            contract_render_field_fixture_payload(contract["materialRecipes"])
        )
        seal_contract(contract)
        with self.assertRaisesRegex(
            ValueError,
            "contract identity summary|render-field|recipe composition",
        ):
            validate_contract(contract)

        contract = copy.deepcopy(self.contract)
        recipe = next(
            row for row in contract["materialRecipes"]
            if row["renderState"]["fields"]["twosided"]["status"]
            == "SERIALIZED_EXPLICIT"
        )
        recipe["renderState"]["fields"]["twosided"]["value"] = not recipe[
            "renderState"
        ]["fields"]["twosided"]["value"]
        seal_contract(contract)
        with self.assertRaisesRegex(ValueError, "bool disagrees with raw bytes"):
            validate_contract(contract)

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

    def test_sampler_and_render_default_blockers_are_field_derived(self) -> None:
        contract = copy.deepcopy(self.contract)
        recipe = next(
            recipe
            for recipe in contract["materialRecipes"]
            if "SAMPLER_BINDINGS_INCOMPLETE" in recipe["blockers"]
        )
        recipe["blockers"].remove("SAMPLER_BINDINGS_INCOMPLETE")
        recipe["blockerCount"] -= 1
        with self.assertRaisesRegex(ValueError, "sampler blocker is not field-derived"):
            validate_contract(contract, verify_digest=False)

        contract = copy.deepcopy(self.contract)
        recipe = next(
            recipe
            for recipe in contract["materialRecipes"]
            if "RENDER_STATE_DEFAULT_PROVENANCE_UNRESOLVED" in recipe["blockers"]
        )
        recipe["blockers"].remove(
            "RENDER_STATE_DEFAULT_PROVENANCE_UNRESOLVED"
        )
        recipe["blockerCount"] -= 1
        with self.assertRaisesRegex(ValueError, "render default blocker is not field-derived"):
            validate_contract(contract, verify_digest=False)

    def test_occurrence_join_loss_fails_closed(self) -> None:
        active = copy.deepcopy(self.active)
        active["activeElements"].pop()
        with self.assertRaisesRegex(ValueError, "active element denominator changed"):
            self.build(active=active)

        active = copy.deepcopy(self.active)
        active["activeElements"][0]["materialParameterEvidence"][0][
            "sourceMaterialPath"
        ] = active["activeElements"][1]["sourceMaterials"][0]
        with self.assertRaisesRegex(ValueError, "occurrence material evidence join"):
            self.build(active=active)

        active = copy.deepcopy(self.active)
        replacement = active["activeElements"][1]["sourceMaterials"][0]
        active["activeElements"][0]["sourceMaterials"][0] = replacement
        active["activeElements"][0]["materialParameterEvidence"][0][
            "sourceMaterialPath"
        ] = replacement
        with self.assertRaisesRegex(ValueError, "stable join changed"):
            self.build(active=active)

    def test_occurrence_full_identity_and_sealed_swap_fail_closed(self) -> None:
        active = copy.deepcopy(self.active)
        active["activeElements"][0]["cueId"] += "/mutated"
        with self.assertRaisesRegex(ValueError, "identity summary"):
            self.build(active=active)

        contract = copy.deepcopy(self.contract)
        contract["occurrences"][0]["sourceEmitter"] += "_mutated"
        seal_contract(contract)
        with self.assertRaisesRegex(ValueError, "occurrence blocker set diverged"):
            validate_contract(contract)

        contract = copy.deepcopy(self.contract)
        left, right = contract["occurrences"][:2]
        swapped_fields = (
            "cueId",
            "rendererType",
            "sourceSystemId",
            "sourceEmitter",
            "sourceMaterialPath",
            "materialRecipeId",
            "blockers",
            "blockerCount",
            "admission",
        )
        for field_name in swapped_fields:
            left[field_name], right[field_name] = (
                right[field_name],
                left[field_name],
            )
        recipe_by_id = {
            recipe["recipeId"]: recipe for recipe in contract["materialRecipes"]
        }
        for occurrence in (left, right):
            recipe = recipe_by_id[occurrence["materialRecipeId"]]
            identity = occurrence_identity_payload(
                active_element_id=occurrence["occurrenceId"],
                cue_id=occurrence["cueId"],
                renderer_type=occurrence["rendererType"],
                source_system_id=occurrence["sourceSystemId"],
                source_emitter=occurrence["sourceEmitter"],
                source_material_path=occurrence["sourceMaterialPath"],
                recipe=recipe,
            )
            occurrence["identity"] = identity
            occurrence["identitySha256"] = canonical_sha256(identity)
        contract["summary"]["occurrenceIdentitySha256"] = canonical_sha256(
            [row["identity"] for row in contract["occurrences"]]
        )
        seal_contract(contract)
        with self.assertRaisesRegex(
            ValueError, "identity summary|property lineage|input lineage"
        ):
            validate_contract(contract)

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

    def test_build_from_paths_and_cli_allow_only_tracked_eol_changes(self) -> None:
        temp_root = REPO_ROOT / ".codex_tmp"
        temp_root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=temp_root) as directory:
            root = Path(directory)
            inputs = {
                "active": ACTIVE_INVENTORY,
                "closure": MATERIAL_CLOSURE,
                "dds": DDS_RECEIPT,
                "render": RENDER_RECEIPT,
            }
            copied: dict[str, Path] = {}
            for name, source in inputs.items():
                path = root / f"{name}.json"
                path.write_bytes(normalize_tracked_text_bytes(source.read_bytes()))
                copied[name] = path
            baseline = build_from_paths(
                copied["active"], copied["closure"], copied["dds"], copied["render"]
            )
            for path in copied.values():
                path.write_bytes(path.read_bytes().replace(b"\n", b"\r\n"))
            crlf = build_from_paths(
                copied["active"], copied["closure"], copied["dds"], copied["render"]
            )
            self.assertEqual(baseline, crlf)

            output = root / "contract.json"
            command = [
                sys.executable,
                str(REPO_ROOT / "Tools/LevelPlacementExtractor/build_artist_31470_material_evidence_contract.py"),
                "--active-inventory",
                str(copied["active"]),
                "--material-closure",
                str(copied["closure"]),
                "--exact-dds-receipt",
                str(copied["dds"]),
                "--render-state-receipt",
                str(copied["render"]),
                "--output",
                str(output),
            ]
            subprocess.run(command, cwd=REPO_ROOT, check=True, capture_output=True)
            output.write_bytes(output.read_bytes().replace(b"\n", b"\r\n"))
            subprocess.run(
                [*command, "--check"], cwd=REPO_ROOT, check=True, capture_output=True
            )
            value = json.loads(output.read_text(encoding="utf-8"))
            output.write_text(
                json.dumps(value, ensure_ascii=False, indent=4) + "\n",
                encoding="utf-8",
            )
            with self.assertRaises(subprocess.CalledProcessError):
                subprocess.run(
                    [*command, "--check"],
                    cwd=REPO_ROOT,
                    check=True,
                    capture_output=True,
                )

    def test_direct_parser_dependency_hash_is_pinned(self) -> None:
        temp_root = REPO_ROOT / ".codex_tmp"
        temp_root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=temp_root) as directory:
            render = copy.deepcopy(self.render)
            render["source"]["materialClosureParser"][
                "canonicalTextSha256"
            ] = "0" * 64
            seal_receipt(render)
            render_path = Path(directory) / "render.json"
            render_path.write_text(
                json.dumps(render, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(ValueError, "materialClosureParser hash"):
                build_from_paths(
                    ACTIVE_INVENTORY, MATERIAL_CLOSURE, DDS_RECEIPT, render_path
                )

    def test_external_artifact_hash_is_raw_not_json_normalized(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            left = Path(directory) / "external-lf.json"
            right = Path(directory) / "external-crlf.json"
            left.write_bytes(b'{"a":1}\n')
            right.write_bytes(b'{"a":1}\r\n')
            self.assertNotEqual(raw_file_sha256(left), raw_file_sha256(right))

    def test_root_identity_and_exact_json_integer_are_fail_closed(self) -> None:
        for value in (True, 1.0, "1"):
            with self.subTest(document="closure", value=value):
                closure = copy.deepcopy(self.closure)
                closure["formatVersion"] = value
                with self.assertRaisesRegex(ValueError, "active material closure"):
                    self.build(closure=closure)
            with self.subTest(document="active", value=value):
                active = copy.deepcopy(self.active)
                active["formatVersion"] = value
                with self.assertRaisesRegex(ValueError, "active inventory"):
                    self.build(active=active)
            with self.subTest(document="dds", value=value):
                dds = copy.deepcopy(self.dds)
                dds["formatVersion"] = value
                with self.assertRaisesRegex(ValueError, "exact DDS receipt"):
                    self.build(dds=dds)

        for name, value in (
            ("characterClass", "WARLORD"),
            ("skillId", 999),
            ("inputSlot", "Q"),
        ):
            with self.subTest(root_field=name):
                closure = copy.deepcopy(self.closure)
                closure[name] = value
                with self.assertRaisesRegex(ValueError, "active material closure"):
                    self.build(closure=closure)

        contract = copy.deepcopy(self.contract)
        contract["formatVersion"] = 3.0
        seal_contract(contract)
        with self.assertRaisesRegex(ValueError, "typed Material contract"):
            validate_contract(contract)

    def test_coordinated_raw_input_reseal_and_recipe_input_swap_fail_closed(self) -> None:
        closure = copy.deepcopy(self.closure)
        render = copy.deepcopy(self.render)
        row = next(
            item
            for item in closure["materials"]
            if item.get("material")
            and item["material"].get("scalarParameters")
        )
        row["material"]["scalarParameters"][0]["value"] = 124.0
        binding = next(
            item
            for item in render["bindings"]
            if folded_path(item.get("sourceMaterialPath"))
            == folded_path(row["sourceMaterialPath"])
        )
        source_export = next(
            item
            for item in render["exports"]
            if item["evidenceId"] == binding["sourceExportEvidenceId"]
        )
        source_export["instanceParameters"]["scalar"][0]["value"] = 124.0
        source_export["fields"]["scalarparametervalues"]["value"][0][
            "parametervalue"
        ]["value"] = 124.0
        seal_receipt(render)
        with self.assertRaisesRegex(ValueError, "exact-input lineage fixture"):
            self.build(closure=closure, render=render)

        contract = copy.deepcopy(self.contract)
        candidates = [
            recipe
            for recipe in contract["materialRecipes"]
            if recipe["inputs"]["scalarOverrides"]
        ]
        left, right = candidates[:2]
        left["inputs"]["scalarOverrides"], right["inputs"]["scalarOverrides"] = (
            right["inputs"]["scalarOverrides"],
            left["inputs"]["scalarOverrides"],
        )
        for recipe in (left, right):
            for field in recipe["inputs"]["scalarOverrides"]:
                lineage = field["provenance"]["lineage"]
                lineage["owner"] = {
                    "recipeId": recipe["recipeId"],
                    "canonicalSourceMaterialPath": recipe["sourceMaterialPath"],
                    "rawMaterialExport": copy.deepcopy(
                        recipe["identity"]["rawMaterialExport"]
                    ),
                }
                lineage["propertyLineage"]["export"] = copy.deepcopy(
                    recipe["identity"]["rawMaterialExport"]
                )
                lineage_sha256 = canonical_sha256(lineage)
                field["provenance"]["lineageSha256"] = lineage_sha256
                field["provenance"]["physicalPackage"] = recipe[
                    "identity"
                ]["physicalPackage"]
                field["provenance"]["physicalPackageSha256"] = recipe[
                    "identity"
                ]["physicalPackageSha256"]
                field["provenance"]["materialObjectPath"] = recipe[
                    "identity"
                ]["materialObjectPath"]
                field["fieldId"] = stable_id(
                    "material-input",
                    recipe["sourceMaterialPath"],
                    field["fieldKind"],
                    field["serializedArrayIndex"],
                    field["parameterName"],
                    field["bindingOrigin"],
                    lineage_sha256,
                )
            recipe["compositionSha256"] = canonical_sha256(
                recipe_composition_payload(recipe)
            )
        recipe_by_id = {
            recipe["recipeId"]: recipe
            for recipe in contract["materialRecipes"]
        }
        for occurrence in contract["occurrences"]:
            recipe = recipe_by_id[occurrence["materialRecipeId"]]
            occurrence["identity"] = occurrence_identity_payload(
                active_element_id=occurrence["occurrenceId"],
                cue_id=occurrence["cueId"],
                renderer_type=occurrence["rendererType"],
                source_system_id=occurrence["sourceSystemId"],
                source_emitter=occurrence["sourceEmitter"],
                source_material_path=occurrence["sourceMaterialPath"],
                recipe=recipe,
            )
            occurrence["identitySha256"] = canonical_sha256(
                occurrence["identity"]
            )
        contract["summary"]["occurrenceIdentitySha256"] = canonical_sha256(
            [row["identity"] for row in contract["occurrences"]]
        )
        contract["summary"]["exactInputLineageSha256"] = canonical_sha256(
            contract_exact_input_lineage_fixture_payload(
                contract["materialRecipes"]
            )
        )
        contract["summary"]["recipeCompositionSha256"] = canonical_sha256(
            [
                {
                    "recipeId": recipe["recipeId"],
                    "compositionSha256": recipe["compositionSha256"],
                }
                for recipe in contract["materialRecipes"]
            ]
        )
        seal_contract(contract)
        with self.assertRaisesRegex(
            ValueError, "identity summary|property lineage|input lineage"
        ):
            validate_contract(contract)

    def test_rejected_sampler_binding_cannot_claim_a_different_recipe(self) -> None:
        contract = copy.deepcopy(self.contract)
        binding = contract["rejectedSamplerBindings"][0]
        binding["materialRecipeId"] = next(
            recipe["recipeId"]
            for recipe in contract["materialRecipes"]
            if recipe["recipeId"] != binding["materialRecipeId"]
        )
        seal_contract(contract)
        with self.assertRaisesRegex(ValueError, "rejected sampler binding join"):
            validate_contract(contract)

    def test_static_parent_expression_guid_cannot_be_resealed(self) -> None:
        contract = copy.deepcopy(self.contract)
        field = next(
            field
            for recipe in contract["materialRecipes"]
            for field in recipe["staticPermutation"]["parentDefaults"]
        )
        field["expressionGuidHex"] = "00" * 16
        with self.assertRaisesRegex(ValueError, "static ExpressionGUID lineage"):
            validate_contract(contract, verify_digest=False)

    def test_shallow_dds_manifest_provenance_cannot_be_resealed(self) -> None:
        dds = copy.deepcopy(self.dds)
        dds["sourceEvidence"]["sourcePackManifest"]["sha256"] = "42" * 32
        with self.assertRaisesRegex(ValueError, "manifest is not authenticated"):
            self.build(dds=dds)

    @unittest.skipUnless(
        SOURCE_PACKAGE_ROOT.is_dir()
        and EXACT_DDS_ROOT.is_dir()
        and SOURCE_PACK_MANIFEST.is_file(),
        "deep Artist F Material roots are unavailable",
    )
    def test_deep_raw_builder_and_verifier_reject_honestly_regenerated_mutations(self) -> None:
        temp_root = REPO_ROOT / ".codex_tmp"
        temp_root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=temp_root) as directory:
            root = Path(directory)

            canonical_closure = root / "canonical-closure.json"
            canonical_closure.write_bytes(
                normalize_tracked_text_bytes(MATERIAL_CLOSURE.read_bytes())
            )
            lf_receipt = build_receipt(
                canonical_closure,
                DDS_RECEIPT,
                SOURCE_PACKAGE_ROOT,
                SOURCE_PACK_MANIFEST,
            )
            canonical_closure.write_bytes(
                canonical_closure.read_bytes().replace(b"\n", b"\r\n")
            )
            crlf_receipt = build_receipt(
                canonical_closure,
                DDS_RECEIPT,
                SOURCE_PACKAGE_ROOT,
                SOURCE_PACK_MANIFEST,
            )
            self.assertEqual(lf_receipt, crlf_receipt)

            closure = copy.deepcopy(self.closure)
            row = next(
                item
                for item in closure["materials"]
                if item.get("material") is not None
            )
            raw_object_path = str(row["material"]["objectPath"])
            row["sourceMaterialPath"] = f"laundered_prefix.{raw_object_path}"
            closure_path = root / "laundered-canonical-prefix.json"
            closure_path.write_text(
                json.dumps(closure, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(
                ValueError, "canonical source Material path"
            ):
                build_receipt(
                    closure_path,
                    DDS_RECEIPT,
                    SOURCE_PACKAGE_ROOT,
                    SOURCE_PACK_MANIFEST,
                )

            closure = copy.deepcopy(self.closure)
            left = next(
                row for row in closure["materials"]
                if folded_path(row.get("sourceMaterialPath")).endswith(
                    "fx_e_me_ht_03_4_ma"
                )
            )
            right = next(
                row for row in closure["materials"]
                if folded_path(row.get("sourceMaterialPath")).endswith(
                    "fx_e_pa_fd_18_2_tr"
                )
            )
            left["sourceMaterialPath"], right["sourceMaterialPath"] = (
                right["sourceMaterialPath"],
                left["sourceMaterialPath"],
            )
            closure_path = root / "swapped-canonical-material-paths.json"
            closure_path.write_text(
                json.dumps(closure, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(
                ValueError, "canonical source Material path"
            ):
                build_receipt(
                    closure_path,
                    DDS_RECEIPT,
                    SOURCE_PACKAGE_ROOT,
                    SOURCE_PACK_MANIFEST,
                )

            closure = copy.deepcopy(self.closure)
            left = next(
                row for row in closure["materials"]
                if folded_path(row.get("sourceMaterialPath")).endswith(
                    "fx_e_me_ht_03_4_ma"
                )
            )
            right = next(
                row for row in closure["materials"]
                if folded_path(row.get("sourceMaterialPath")).endswith(
                    "fx_e_pa_fd_18_2_tr"
                )
            )
            left["parentGraph"], right["parentGraph"] = (
                right["parentGraph"],
                left["parentGraph"],
            )
            closure_path = root / "swapped-parent-graphs.json"
            closure_path.write_text(
                json.dumps(closure, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(ValueError, "raw MIC Parent"):
                build_receipt(
                    closure_path,
                    DDS_RECEIPT,
                    SOURCE_PACKAGE_ROOT,
                    SOURCE_PACK_MANIFEST,
                )

            closure = copy.deepcopy(self.closure)
            row = next(
                row
                for row in closure["materials"]
                if row.get("material") and row["material"].get("scalarParameters")
            )
            row["material"]["scalarParameters"][0]["value"] = 124.0
            closure_path = root / "mutated-instance.json"
            closure_path.write_text(
                json.dumps(closure, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(ValueError, "closure input disagrees with raw UPK"):
                build_receipt(
                    closure_path,
                    DDS_RECEIPT,
                    SOURCE_PACKAGE_ROOT,
                    SOURCE_PACK_MANIFEST,
                )

            closure = copy.deepcopy(self.closure)
            expression = next(
                expression
                for row in closure["materials"]
                if row.get("material")
                for expression in (row.get("materialGraph") or row.get("parentGraph"))["graph"]["expressions"]
                if isinstance(expression.get("defaultValue"), (int, float))
                and not isinstance(expression.get("defaultValue"), bool)
                and float(expression["defaultValue"]) == 1.0
            )
            expression["defaultValue"] = 18.0
            closure_path = root / "mutated-parent-expression.json"
            closure_path.write_text(
                json.dumps(closure, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(
                ValueError, "closure expression projection disagrees with raw UPK"
            ):
                build_receipt(
                    closure_path,
                    DDS_RECEIPT,
                    SOURCE_PACKAGE_ROOT,
                    SOURCE_PACK_MANIFEST,
                )

            verify_external_artifacts(
                self.dds,
                self.render,
                SOURCE_PACKAGE_ROOT,
                EXACT_DDS_ROOT,
                SOURCE_PACK_MANIFEST,
            )
            manifest_bytes = SOURCE_PACK_MANIFEST.read_bytes()
            mutated_manifest = root / "mutated-source-pack-manifest.json"
            if b"\r\n" in manifest_bytes:
                mutated_manifest.write_bytes(manifest_bytes.replace(b"\r\n", b"\n"))
            else:
                mutated_manifest.write_bytes(manifest_bytes.replace(b"\n", b"\r\n"))
            self.assertNotEqual(
                raw_file_sha256(mutated_manifest), raw_file_sha256(SOURCE_PACK_MANIFEST)
            )
            with self.assertRaisesRegex(ValueError, "manifest raw bytes changed"):
                verify_external_artifacts(
                    self.dds,
                    self.render,
                    SOURCE_PACKAGE_ROOT,
                    EXACT_DDS_ROOT,
                    mutated_manifest,
                )
            for field_name, replacement in (
                ("className", "Material"),
                ("exportIndex", 999),
                ("serialSha256", "0" * 64),
            ):
                with self.subTest(field_name=field_name):
                    dds = copy.deepcopy(self.dds)
                    dds["assets"][0]["sourceTexture2D"][field_name] = replacement
                    dds_path = root / f"forged-{field_name}.json"
                    dds_path.write_text(
                        json.dumps(dds, ensure_ascii=False, indent=2) + "\n",
                        encoding="utf-8",
                    )
                    with self.assertRaisesRegex(
                        ValueError,
                        "Texture2D|raw Texture2D|DDS receipt disagrees|"
                        "export index changed|serial SHA-256 changed|class changed",
                    ):
                        build_receipt(
                            MATERIAL_CLOSURE,
                            dds_path,
                            SOURCE_PACKAGE_ROOT,
                            SOURCE_PACK_MANIFEST,
                        )
                    with self.assertRaisesRegex(
                        ValueError, "DDS receipt disagrees with raw Texture2D export"
                    ):
                        verify_external_artifacts(
                            dds,
                            self.render,
                            SOURCE_PACKAGE_ROOT,
                            EXACT_DDS_ROOT,
                            SOURCE_PACK_MANIFEST,
                        )

    def test_duplicate_json_object_key_is_corrupt(self) -> None:
        with self.assertRaisesRegex(ValueError, "duplicate JSON object key"):
            normalize_tracked_text_bytes(b'{"a":1,"a":1}\n')


def folded_path(value: object) -> str:
    return str(value or "").casefold()


if __name__ == "__main__":
    unittest.main()
