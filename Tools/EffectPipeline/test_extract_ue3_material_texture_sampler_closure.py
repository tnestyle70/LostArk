#!/usr/bin/env python3
from __future__ import annotations

import copy
import struct
import sys
import tempfile
import unittest
from pathlib import Path

TOOL_ROOT = Path(__file__).resolve().parent
if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

import extract_ue3_material_texture_sampler_closure as closure


def fname(index: int, number: int = 0) -> bytes:
    return struct.pack("<ii", index, number)


def source_revision_abi_fixture() -> dict:
    return {
        "effectiveSerializedDefaults": {
            "addressx": {"status": "NATIVE_CONSTRUCTOR_UNRESOLVED"},
            "addressy": {"status": "NATIVE_CONSTRUCTOR_UNRESOLVED"},
            "srgb": {
                "status": "SOURCE_REVISION_CDO_SERIALIZED",
                "declaringCdo": "Default__Texture",
                "value": True,
                "recordSha256": "srgb-record",
            },
            "filter": {
                "status": "SOURCE_REVISION_CDO_SERIALIZED",
                "declaringCdo": "Default__Texture",
                "value": "TF_Linear",
                "recordSha256": "filter-record",
            },
            "lodgroup": {"status": "NATIVE_CONSTRUCTOR_UNRESOLVED"},
        },
        "enums": {
            "TextureAddress": {
                "values": [
                    {"ordinal": 0, "name": "TA_Wrap"},
                    {"ordinal": 1, "name": "TA_Clamp"},
                    {"ordinal": 2, "name": "TA_Mirror"},
                    {"ordinal": 3, "name": "TA_MAX"},
                ]
            },
            "TextureFilter": {
                "values": [
                    {"ordinal": 0, "name": "TF_Nearest"},
                    {"ordinal": 1, "name": "TF_Linear"},
                    {"ordinal": 2, "name": "TF_MAX"},
                ]
            },
            "TextureGroup": {
                "values": [
                    {"ordinal": 0, "name": "TEXTUREGROUP_World"},
                    {"ordinal": 13, "name": "TEXTUREGROUP_Effects"},
                    {"ordinal": 32, "name": "TEXTUREGROUP_MAX"},
                ]
            },
        },
    }


class TextureSamplerClosureTests(unittest.TestCase):
    def test_class_neutral_target_filter_pins_identity_and_order(self) -> None:
        exact_targets = [
            {
                "targetId": "ground",
                "familyId": "family.ground",
                "sourceMaterialPath": "mi.ground",
                "parentMaterialPath": "m.ground",
                "rendererType": "LocalDecal",
            },
            {
                "targetId": "crack",
                "familyId": "family.crack",
                "sourceMaterialPath": "mi.crack",
                "parentMaterialPath": "m.crack",
                "rendererType": "MeshParticle",
            },
        ]
        target_filter = {
            "schema": closure.TARGET_FILTER_SCHEMA,
            "formatVersion": closure.TARGET_FILTER_FORMAT_VERSION,
            "filterId": "core-two",
            "targets": [
                {
                    "targetId": "crack",
                    "familyId": "family.crack",
                    "sourceMaterialPath": "mi.crack",
                    "parentMaterialPath": "m.crack",
                    "rendererType": "MeshParticle",
                },
                {"targetId": "ground"},
            ],
        }

        selected, normalized = closure.selected_exact_targets(
            exact_targets, target_filter
        )

        self.assertEqual([row["targetId"] for row in selected], ["crack", "ground"])
        self.assertEqual(normalized["filterId"], "core-two")
        changed = copy.deepcopy(target_filter)
        changed["targets"][0]["rendererType"] = "SpriteParticle"
        with self.assertRaisesRegex(ValueError, "rendererType changed"):
            closure.selected_exact_targets(exact_targets, changed)

    def test_source_pack_mapping_closes_parent_package_without_object_overclaim(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            source_root = Path(temporary)
            (source_root / "parent.upk").touch()
            source_pack = {
                "schemaVersion": 1,
                "packages": [
                    {
                        "logicalPackage": "FX_M_MI_00",
                        "physicalPackage": "parent.upk",
                        "resolved": True,
                    }
                ],
            }

            rows = closure.source_pack_manifest_assets(source_pack, source_root)
            assets = closure.manifest_asset_index({"assets": rows})
            parent = closure.resolve_parent_manifest_asset(
                "fx_m_mi_00.fx_m.fx_d_me_crack_01_tr", assets
            )

        self.assertEqual(parent["physicalPackage"], "parent.upk")
        self.assertEqual(
            parent["resolutionStatus"],
            "RESOLVED_BY_EXACT_LOGICAL_PACKAGE_MAPPING",
        )
        self.assertEqual(
            rows[0]["sourceAssetPath"], "fx_m_mi_00.__package_mapping__"
        )

    def test_tagged_texture_override_preserves_full_fname_number(self) -> None:
        names = [
            "None",
            "ParameterName",
            "NameProperty",
            "ParameterValue",
            "ObjectProperty",
            "ExpressionGUID",
            "StructProperty",
            "Guid",
            "MainTex",
        ]
        payload = bytearray(struct.pack("<i", 1))
        payload += fname(1) + fname(2) + struct.pack("<ii", 8, 0) + fname(8, 3)
        payload += fname(3) + fname(4) + struct.pack("<ii", 4, 0) + struct.pack("<i", -7)
        payload += (
            fname(5)
            + fname(6)
            + struct.pack("<ii", 16, 0)
            + fname(7)
            + bytes(range(16))
        )
        payload += fname(0)

        rows = closure.parse_tagged_struct_array_raw(bytes(payload), names)

        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["parametername"]["value"], "MainTex")
        self.assertEqual(rows[0]["parametername"]["valueNameNumber"], 3)
        self.assertEqual(rows[0]["parametervalue"]["value"], -7)
        self.assertEqual(
            rows[0]["expressionguid"]["valueGuidHex"], bytes(range(16)).hex()
        )

    def test_uniform_binding_uses_full_fname_override_then_fallback(self) -> None:
        expressions = [
            {
                "typeName": "fmaterialuniformexpressiontextureparameter",
                "parameterName": "MainTex",
                "parameterNameNumber": 0,
                "referencedTextureIndex": 0,
            },
            {
                "typeName": "fmaterialuniformexpressiontextureparameter",
                "parameterName": "MainTex",
                "parameterNameNumber": 2,
                "referencedTextureIndex": 1,
            },
        ]
        wires = [
            {
                "expressionIndexOrGroup": 0,
                "baseIndex": 4,
                "bufferIndexOrSamplerIndex": 1,
            },
            {
                "expressionIndexOrGroup": 1,
                "baseIndex": 7,
                "bufferIndexOrSamplerIndex": 3,
            },
        ]
        references = [
            {"objectPath": "fx_tex.fallback_a"},
            {"objectPath": "fx_tex.fallback_b"},
        ]
        overrides = [
            {
                "index": 0,
                "parameterName": "MainTex",
                "parameterNameNumber": 2,
                "sourceObjectPath": "fx_tex.override_number_2",
            }
        ]

        bindings = closure.resolve_uniform_texture_bindings(
            expressions, wires, references, overrides
        )

        self.assertFalse(bindings[0]["micOverrideApplied"])
        self.assertEqual(bindings[0]["effectiveSourceObjectPath"], "fx_tex.fallback_a")
        self.assertEqual(bindings[0]["textureRegister"], "t4")
        self.assertEqual(bindings[0]["samplerRegister"], "s1")
        self.assertTrue(bindings[1]["micOverrideApplied"])
        self.assertEqual(
            bindings[1]["effectiveSourceObjectPath"], "fx_tex.override_number_2"
        )
        self.assertEqual(bindings[1]["parameterFNameKey"], "maintex#2")

    def test_sampler_projection_inherits_source_cdo_and_keeps_native_defaults_blocked(self) -> None:
        fields = {
            name: {"status": "OMITTED_FROM_EXPORT"}
            for name in ("addressx", "addressy", "srgb", "filter", "lodgroup")
        }

        result = closure.sampler_projection(fields, source_revision_abi_fixture())

        self.assertFalse(result["sourceExactSamplerAndColorSpace"])
        self.assertIsNone(result["addressU"]["value"])
        self.assertEqual(result["colorSpace"]["value"], "srgb")
        self.assertEqual(
            result["colorSpace"]["status"], "SOURCE_REVISION_CDO_INHERITED"
        )
        self.assertTrue(result["sourceExactColorSpace"])
        self.assertEqual(result["filterSelector"]["value"], "tf_linear")
        self.assertTrue(result["sourceExactFilterSelector"])
        self.assertFalse(result["sourceExactLodGroup"])
        self.assertIn(
            "FILTER_TF_LINEAR_SOURCE_REVISION_TEXTURELODSETTINGS_UNRESOLVED",
            result["blockers"],
        )
        self.assertIn(
            "ADDRESS_U_NATIVE_CONSTRUCTOR_DEFAULT_UNRESOLVED", result["blockers"]
        )
        self.assertIn(
            "LOD_GROUP_NATIVE_CONSTRUCTOR_DEFAULT_UNRESOLVED", result["blockers"]
        )

    def test_explicit_properties_win_over_source_cdo_but_hardware_filter_stays_blocked(self) -> None:
        fields = {
            "addressx": {"status": "SERIALIZED_EXPLICIT", "value": "TA_Clamp"},
            "addressy": {"status": "SERIALIZED_EXPLICIT", "value": "TA_Wrap"},
            "srgb": {"status": "SERIALIZED_EXPLICIT", "value": False},
            "filter": {"status": "SERIALIZED_EXPLICIT", "value": "TF_Nearest"},
            "lodgroup": {
                "status": "SERIALIZED_EXPLICIT",
                "value": "TEXTUREGROUP_Effects",
            },
        }

        result = closure.sampler_projection(fields, source_revision_abi_fixture())

        self.assertFalse(result["sourceExactSamplerAndColorSpace"])
        self.assertEqual(result["filterSelector"]["value"], "tf_nearest")
        self.assertEqual(result["filterSelector"]["status"], "SERIALIZED_EXPLICIT")
        self.assertIsNone(result["resolvedFilter"])
        self.assertEqual(result["colorSpace"]["value"], "linear")
        self.assertEqual(result["colorSpace"]["status"], "SERIALIZED_EXPLICIT")
        self.assertEqual(result["sourceExactAddressAxisCount"], 2)
        self.assertTrue(result["sourceExactLodGroup"])
        self.assertIn(
            "FILTER_TF_NEAREST_SOURCE_REVISION_TEXTURELODSETTINGS_UNRESOLVED",
            result["blockers"],
        )

    def test_v975_rejects_tf_default(self) -> None:
        fields = {
            name: {"status": "OMITTED_FROM_EXPORT"}
            for name in ("addressx", "addressy", "srgb", "filter", "lodgroup")
        }
        fields["filter"] = {
            "status": "SERIALIZED_EXPLICIT",
            "value": "TF_Default",
        }

        with self.assertRaisesRegex(ValueError, "unsupported Texture2D filter for v975"):
            closure.sampler_projection(fields, source_revision_abi_fixture())

    def test_official_v975_engine_cdo_and_enums_are_exact(self) -> None:
        evidence = closure.source_revision_texture_abi_evidence(
            closure.DEFAULT_OFFICIAL_V975_MANIFEST,
            closure.DEFAULT_OFFICIAL_V975_ENGINE_PACKAGE,
        )

        closure.validate_source_revision_texture_abi(evidence)
        self.assertTrue(evidence["effectiveSerializedDefaults"]["srgb"]["value"])
        self.assertEqual(
            evidence["effectiveSerializedDefaults"]["filter"]["value"], "TF_Linear"
        )
        self.assertEqual(
            [row["name"] for row in evidence["enums"]["TextureFilter"]["values"]],
            ["TF_Nearest", "TF_Linear", "TF_MAX"],
        )
        self.assertFalse(
            evidence["textureLodSettings"]["finalHardwareFilterResolved"]
        )

    def test_tracked_receipt_is_sealed_and_keeps_runtime_visual_closed(self) -> None:
        receipt_path = (
            Path(__file__).resolve().parents[2]
            / "Data/Effects/Imported/DimensionMaster/Materials/"
            "skill.2050120.clip3.exact-texture-sampler-closure.receipt.json"
        )
        receipt = closure.read_json(receipt_path)

        closure.validate_receipt(receipt)
        self.assertEqual(receipt["summary"]["runtimeDdsParityTargetCount"], 5)
        self.assertEqual(
            receipt["summary"]["glasshole02SamplerEvidence"],
            {
                "bindingCount": 7,
                "sourceExactColorSpaceBindingCount": 7,
                "sourceExactFilterSelectorBindingCount": 7,
                "sourceExactAddressAxisCount": 1,
                "sourceExactAddressAxisDenominator": 14,
                "sourceExactLodGroupBindingCount": 6,
                "sourceExactHardwareFilterBindingCount": 0,
                "sourceExactFullSamplerTargetCount": 0,
            },
        )
        glasshole = next(
            row for row in receipt["targets"]
            if row.get("targetId") == "dimensionmaster-w-glasshole-02"
        )
        self.assertTrue(glasshole["runtimeDdsParityAdmission"])
        self.assertFalse(glasshole["sourceExactSamplerAdmission"])
        self.assertNotIn(
            "FILTER_TF_DEFAULT_TEXTURELODSETTINGS_UNRESOLVED",
            glasshole["blockers"],
        )
        self.assertIn(
            "FILTER_TF_LINEAR_SOURCE_REVISION_TEXTURELODSETTINGS_UNRESOLVED",
            glasshole["blockers"],
        )
        self.assertEqual(
            receipt["inputs"]["extractor"]["path"],
            "Tools/EffectPipeline/extract_ue3_material_texture_sampler_closure.py",
        )
        self.assertFalse(receipt["scope"]["authoredGenericResourceSlotsRead"])
        self.assertFalse(receipt["scope"]["runtimeAdmission"])
        self.assertFalse(receipt["scope"]["visualAdmission"])

    def test_receipt_rejects_cdo_or_enum_evidence_mutation_after_reseal(self) -> None:
        receipt_path = (
            Path(__file__).resolve().parents[2]
            / "Data/Effects/Imported/DimensionMaster/Materials/"
            "skill.2050120.clip3.exact-texture-sampler-closure.receipt.json"
        )
        original = closure.read_json(receipt_path)
        mutations = []

        cdo_mutation = copy.deepcopy(original)
        cdo_mutation["sourceRevisionTextureAbi"]["effectiveSerializedDefaults"][
            "srgb"
        ]["value"] = False
        mutations.append(cdo_mutation)

        enum_mutation = copy.deepcopy(original)
        enum_mutation["sourceRevisionTextureAbi"]["enums"]["TextureFilter"][
            "values"
        ][1]["name"] = "TF_Default"
        mutations.append(enum_mutation)

        for mutation in mutations:
            with self.subTest(mutation=mutations.index(mutation)):
                sealed = dict(mutation)
                sealed.pop("receiptSha256", None)
                mutation["receiptSha256"] = closure.canonical_json_sha256(sealed)
                with self.assertRaises(ValueError):
                    closure.validate_receipt(mutation)

    def test_tracked_valtan_core_three_receipt_is_exact_but_sampler_blocked(self) -> None:
        receipt_path = (
            Path(__file__).resolve().parents[2]
            / "Data/Effects/Imported/Valtan/FrontBackFrontFamilyRestoration/"
            "Valtan.front-back-front-texture-sampler-closure.receipt.v1.json"
        )
        receipt = closure.read_json(receipt_path)

        closure.validate_receipt(receipt)
        self.assertEqual(
            receipt["scope"]["selectedTargetIds"],
            [
                "valtan-front-back-front-masked-dissolve-stone",
                "valtan-front-back-front-ground-decal",
                "valtan-front-back-front-crack-translucent",
            ],
        )
        self.assertEqual(receipt["summary"]["exactTargetCount"], 3)
        self.assertEqual(receipt["summary"]["sourceExactTextureBindingCount"], 3)
        self.assertEqual(receipt["summary"]["runtimeDdsParityTargetCount"], 3)
        self.assertEqual(receipt["summary"]["sourceExactSamplerTargetCount"], 0)
        self.assertFalse(receipt["scope"]["runtimeAdmission"])
        self.assertFalse(receipt["scope"]["visualAdmission"])
        for target in receipt["targets"]:
            self.assertTrue(target["sourceExactTextureBindingAdmission"])
            self.assertTrue(target["runtimeDdsParityAdmission"])
            self.assertFalse(target["sourceExactSamplerAdmission"])
            self.assertTrue(target["blockers"])
            for binding in target["uniformTextureBindings"]:
                self.assertIn("runtimeTexture", binding["ddsIdentity"])
                self.assertNotIn("runtimeDimensionMaster", binding["ddsIdentity"])

    def test_valtan_receipt_rejects_resealed_sampler_overclaim(self) -> None:
        receipt_path = (
            Path(__file__).resolve().parents[2]
            / "Data/Effects/Imported/Valtan/FrontBackFrontFamilyRestoration/"
            "Valtan.front-back-front-texture-sampler-closure.receipt.v1.json"
        )
        mutation = closure.read_json(receipt_path)
        mutation["targets"][0]["sourceExactSamplerAdmission"] = True
        sealed = dict(mutation)
        sealed.pop("receiptSha256", None)
        mutation["receiptSha256"] = closure.canonical_json_sha256(sealed)

        with self.assertRaisesRegex(ValueError, "sampler admission overclaim"):
            closure.validate_receipt(mutation)


if __name__ == "__main__":
    unittest.main()
