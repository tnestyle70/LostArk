#!/usr/bin/env python3

from __future__ import annotations

import copy
from collections import Counter
import importlib.util
import io
import json
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
import tempfile
import unittest


MODULE_PATH = Path(__file__).with_name("build_effect_material_program_registry.py")
SPEC = importlib.util.spec_from_file_location("material_program_registry", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
registry_tool = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(registry_tool)

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
SOURCE_PATH = (
    REPOSITORY_ROOT
    / "Data"
    / "Effects"
    / "MaterialPrograms"
    / "effect-material-program-registry.v1.json"
)
FRAGMENT_ROOT = SOURCE_PATH.parent / "Fragments"
ARTIST_FRAGMENT_PATH = (
    FRAGMENT_ROOT / "artist-f-golden.material-program-fragment.v1.json"
)
EFFECT_CATALOG_PATH = REPOSITORY_ROOT / "Data" / "Effects" / "EffectCatalog.json"
DATA_ROOT = REPOSITORY_ROOT / "Data"
AUTHORED_PATH = (
    DATA_ROOT
    / "Effects"
    / "Authored"
    / "effect.artist.skill.31470.unified.effect.json"
)
EFFECT_ID = "effect.artist.skill.31470.unified"
ELEMENT_ID = "sprite.2b3dc6842507e910"
SHADOW_ELEMENT_ID = "sprite.c65181324417a1a8"
PROGRAM_ID = "effect.program.runtime-material-v2.opcode-6.v1"
LAYOUT_ID = "effect.layout.runtime-material-v2.opcode-6.abi-3aafae1b4639c551.v1"
DESCRIPTOR_ID = "effect.descriptor.artist-f.sprite-2b3dc6842507e910.v1"
REPRESENTATIVE_V1_EFFECT_IDS = {
    "effect.artist.skill.31460.v1.unified",
    "effect.dimensionmaster.skill.2050180.v1.unified",
    "effect.lancemaster.skill.34110.v1.unified",
    "effect.warlord.skill.17110.clip2.v1.unified",
    "effect.warlord.skill.17110.clip3.v1.unified",
}


def make_registry() -> dict[str, object]:
    return copy.deepcopy(
        registry_tool.build_registry(SOURCE_PATH, EFFECT_CATALOG_PATH, DATA_ROOT)
    )


def make_binding(element_id: str = ELEMENT_ID) -> dict[str, object]:
    return {
        "effectAssetId": EFFECT_ID,
        "elementId": element_id,
        "programId": PROGRAM_ID,
        "layoutId": LAYOUT_ID,
        "descriptorId": DESCRIPTOR_ID,
        "adapterId": registry_tool.CANONICAL_SPRITE_ADAPTER_ID,
        "inlineMirrorPolicy": registry_tool.INLINE_MIRROR_REQUIRED,
    }


def make_bound_registry(*element_ids: str) -> dict[str, object]:
    registry = make_registry()
    registry["bindings"] = [make_binding(element_id) for element_id in element_ids]
    return registry


def make_authored_documents() -> dict[str, dict[str, object]]:
    return {
        EFFECT_ID: registry_tool.load_json(
            AUTHORED_PATH, "test authored document", require_lf=False
        )
    }


class MaterialProgramRegistryTests(unittest.TestCase):
    def test_base_owns_only_compiled_adapters_and_fragment_owns_domain_rows(self) -> None:
        base = registry_tool.load_json(SOURCE_PATH)
        fragment = registry_tool.load_json(ARTIST_FRAGMENT_PATH)
        self.assertEqual([], base["programs"])
        self.assertEqual([], base["layouts"])
        self.assertEqual([], base["descriptors"])
        self.assertEqual([], base["bindings"])
        self.assertEqual(
            list(registry_tool.COMPILED_ADAPTERS),
            [row["adapterId"] for row in base["adapters"]],
        )
        self.assertEqual(registry_tool.FRAGMENT_SCHEMA, fragment["schema"])
        self.assertEqual("effect-domain.artist-f-golden", fragment["domainId"])
        self.assertNotIn("adapters", fragment)

    def test_source_contains_exact_artist_f_sprite_mesh_decal_bindings(self) -> None:
        registry = registry_tool.build_registry(
            SOURCE_PATH, EFFECT_CATALOG_PATH, DATA_ROOT
        )
        artist_f_bindings = [
            row for row in registry["bindings"] if row["effectAssetId"] == EFFECT_ID
        ]
        self.assertEqual(make_binding(), artist_f_bindings[0])
        self.assertEqual(
            [ELEMENT_ID, "mesh.062366ee9f9655d3", "decal.f3b5c3b63b4a7e34"],
            [row["elementId"] for row in artist_f_bindings],
        )
        representative_bindings = [
            row
            for row in registry["bindings"]
            if row["effectAssetId"] in REPRESENTATIVE_V1_EFFECT_IDS
        ]
        self.assertEqual(171, len(registry["bindings"]))
        self.assertEqual(131, len(representative_bindings))
        self.assertEqual(
            {
                registry_tool.DECAL_ALPHA_TWO_SIDED_ADAPTER_ID: 8,
                registry_tool.MESH_ALPHA_ONE_SIDED_ADAPTER_ID: 3,
                registry_tool.MESH_ALPHA_TWO_SIDED_ADAPTER_ID: 22,
                registry_tool.SPRITE_ADDITIVE_ONE_SIDED_ADAPTER_ID: 23,
                registry_tool.SPRITE_ADDITIVE_TWO_SIDED_ADAPTER_ID: 24,
                registry_tool.SPRITE_ALPHA_ONE_SIDED_ADAPTER_ID: 41,
                registry_tool.CANONICAL_SPRITE_ADAPTER_ID: 10,
            },
            dict(Counter(row["adapterId"] for row in representative_bindings)),
        )
        self.assertTrue(
            all(
                row["inlineMirrorPolicy"] == registry_tool.INLINE_MIRROR_REQUIRED
                for row in artist_f_bindings
            )
        )
        output = io.StringIO()
        with redirect_stdout(output):
            self.assertEqual(
                0,
                registry_tool.main(
                    [
                        "--source",
                        str(SOURCE_PATH),
                        "--effect-catalog",
                        str(EFFECT_CATALOG_PATH),
                        "--data-root",
                        str(DATA_ROOT),
                    ]
                ),
            )
        emitted = json.loads(output.getvalue())
        self.assertEqual(tuple(registry.keys()), tuple(emitted.keys()))
        self.assertEqual(registry, emitted)

    def test_program_and_layout_ids_are_domain_neutral(self) -> None:
        registry = make_registry()
        self.assertEqual(
            set(registry_tool.COMPILED_PROGRAM_IDS.values()),
            {row["programId"] for row in registry["programs"]},
        )
        self.assertEqual(
            set(registry_tool.COMPILED_LAYOUT_IDS.values()),
            {
                row["layoutId"]
                for row in registry["layouts"]
                if not row["layoutId"].startswith(
                    "effect.layout.standard-color-v1."
                )
            },
        )

        domain_program = make_registry()
        domain_program["programs"][0]["programId"] = "effect.program.artist.opcode-6.v1"
        with self.assertRaisesRegex(registry_tool.ContractError, "canonical compiled ID"):
            registry_tool.validate_registry(domain_program)

        domain_layout = make_registry()
        domain_layout["layouts"][0]["layoutId"] = "effect.layout.valtan.opcode-6.v1"
        with self.assertRaisesRegex(registry_tool.ContractError, "canonical compiled ABI ID"):
            registry_tool.validate_registry(domain_layout)

        duplicate_program_alias = make_registry()
        alias = copy.deepcopy(duplicate_program_alias["programs"][0])
        alias["programId"] = "effect.program.runtime-material-v2.opcode-6.alias.v1"
        duplicate_program_alias["programs"].append(alias)
        with self.assertRaisesRegex(registry_tool.ContractError, "canonical compiled ID"):
            registry_tool.validate_registry(duplicate_program_alias)

        duplicate_layout_alias = make_registry()
        alias = copy.deepcopy(duplicate_layout_alias["layouts"][0])
        alias["layoutId"] = "effect.layout.runtime-material-v2.opcode-6.abi-deadbeef.v1"
        duplicate_layout_alias["layouts"].append(alias)
        with self.assertRaisesRegex(registry_tool.ContractError, "canonical compiled ABI ID"):
            registry_tool.validate_registry(duplicate_layout_alias)
    def test_representative_standard_color_packet_and_float_bits_are_exact(self) -> None:
        registry = make_registry()
        binding = next(
            row
            for row in registry["bindings"]
            if row["effectAssetId"] == "effect.artist.skill.31460.v1.unified"
            and row["adapterId"]
            == registry_tool.DECAL_ALPHA_TWO_SIDED_ADAPTER_ID
        )
        programs = {row["programId"]: row for row in registry["programs"]}
        layouts = {row["layoutId"]: row for row in registry["layouts"]}
        descriptors = {
            row["descriptorId"]: row for row in registry["descriptors"]
        }
        packet = registry_tool.materialize_binding(
            binding, programs, layouts, descriptors
        )
        self.assertEqual("standardColorV1", packet["backend"])
        self.assertEqual(1, packet["opcode"])
        self.assertEqual("DSS_ZNone", packet["renderState"]["depthStencil"])
        self.assertEqual(
            ["base_radiance", "coverage"],
            [row["role"] for row in packet["textureLanes"]],
        )
        self.assertEqual(
            {
                "packetVersion": 1,
                "baseRadianceLaneId": "lane.0",
                "baseRadianceChannel": "RGB",
                "coverageLaneId": "lane.1",
                "coverageChannel": "R",
                "emissiveMode": "baseRadiance",
                "lifetimeEnvelope": "carrierAlpha",
                "dissolveMode": "none",
                "dissolveLaneId": "",
                "dissolveChannel": "invalid",
                "dissolveSoftness": 0.0,
                "missingLanePolicy": "failClosed",
            },
            packet["standardColor"],
        )

        drifted = copy.deepcopy(registry)
        authored = registry_tool.load_binding_authored_documents(
            drifted, EFFECT_CATALOG_PATH, DATA_ROOT
        )
        target = next(
            row
            for row in authored[binding["effectAssetId"]]["elements"]
            if row["id"] == binding["elementId"]
        )
        target["material"]["execution"]["standardColor"][
            "dissolveSoftness"
        ] = -0.0
        with self.assertRaisesRegex(registry_tool.ContractError, "float-bit mismatch"):
            registry_tool.validate_registry(drifted, authored)

    def test_standard_color_carrier_resource_contract_fails_closed(self) -> None:
        registry = make_registry()
        authored = registry_tool.load_binding_authored_documents(
            registry, EFFECT_CATALOG_PATH, DATA_ROOT
        )
        cases = (
            (
                registry_tool.SPRITE_ALPHA_ONE_SIDED_ADAPTER_ID,
                [{"slotId": "base", "assetId": "Effect/invalid.dds"}],
            ),
            (
                registry_tool.MESH_ALPHA_ONE_SIDED_ADAPTER_ID,
                [
                    {"slotId": "meshModel", "assetId": "Effect/valid.wmodel"},
                    {"slotId": "base", "assetId": "Effect/invalid.dds"},
                ],
            ),
            (
                registry_tool.DECAL_ALPHA_TWO_SIDED_ADAPTER_ID,
                [{"slotId": "base", "assetId": "Effect/invalid.dds"}],
            ),
        )
        for adapter_id, resources in cases:
            with self.subTest(adapter=adapter_id):
                binding = next(
                    row for row in registry["bindings"]
                    if row["adapterId"] == adapter_id
                    and row["effectAssetId"].endswith(".v1.unified")
                )
                mutated = copy.deepcopy(authored)
                target = next(
                    row
                    for row in mutated[binding["effectAssetId"]]["elements"]
                    if row["id"] == binding["elementId"]
                )
                target["resources"] = resources
                with self.assertRaisesRegex(
                    registry_tool.ContractError, "carrier is incompatible"
                ):
                    registry_tool.validate_registry(registry, mutated)
    def test_artist_f_canary_binding_materializes_bit_exact_inline_packet(self) -> None:
        registry = make_bound_registry(ELEMENT_ID)
        registry_tool.validate_registry(registry, make_authored_documents())

        lookups = (
            {row["programId"]: row for row in registry["programs"]},
            {row["layoutId"]: row for row in registry["layouts"]},
            {row["descriptorId"]: row for row in registry["descriptors"]},
        )
        packet = registry_tool.materialize_binding(
            registry["bindings"][0], *lookups
        )
        self.assertNotIn("fidelity", packet)
        self.assertEqual(1, packet["passIndex"])
        self.assertEqual("RS_Cull_None", packet["renderState"]["rasterizer"])
        self.assertEqual(["lane.0", "lane.1"], [row["laneId"] for row in packet["textureLanes"]])
        self.assertEqual([30, 1.5, 0.100000001], [row["value"] for row in packet["scalars"]])

    def test_shadow_occurrence_resolves_the_same_bit_exact_packet(self) -> None:
        registry = make_bound_registry(ELEMENT_ID, SHADOW_ELEMENT_ID)
        authored = make_authored_documents()
        registry_tool.validate_registry(registry, authored)

        programs = {row["programId"]: row for row in registry["programs"]}
        layouts = {row["layoutId"]: row for row in registry["layouts"]}
        descriptors = {
            row["descriptorId"]: row for row in registry["descriptors"]
        }
        first_packet = registry_tool.materialize_binding(
            registry["bindings"][0], programs, layouts, descriptors
        )
        shadow_packet = registry_tool.materialize_binding(
            registry["bindings"][1], programs, layouts, descriptors
        )
        registry_tool.assert_execution_bit_exact(
            first_packet, shadow_packet, "Artist F shadow packet"
        )
        self.assertEqual(ELEMENT_ID, registry["bindings"][0]["elementId"])
        self.assertEqual(SHADOW_ELEMENT_ID, registry["bindings"][1]["elementId"])

    def test_compiled_mesh_adapter_materializes_its_pass_and_render_state(self) -> None:
        registry = make_bound_registry(ELEMENT_ID)
        registry["programs"][0]["opcode"] = 3
        registry["adapters"] = [
            {"adapterId": registry_tool.MESH_ALPHA_TWO_SIDED_ADAPTER_ID}
        ]
        binding = registry["bindings"][0]
        binding["adapterId"] = registry_tool.MESH_ALPHA_TWO_SIDED_ADAPTER_ID

        packet = registry_tool.materialize_binding(
            binding,
            {row["programId"]: row for row in registry["programs"]},
            {row["layoutId"]: row for row in registry["layouts"]},
            {row["descriptorId"]: row for row in registry["descriptors"]},
        )
        self.assertEqual(1, packet["passIndex"])
        self.assertEqual("RS_Cull_None", packet["renderState"]["rasterizer"])
        self.assertEqual("BS_EffectAlpha", packet["renderState"]["blend"])

    def test_project_tuned_base_coverage_abis_and_eight_adapters_are_compiled(self) -> None:
        sampler = copy.deepcopy(
            make_registry()["descriptors"][0]["textureLanes"][0]["sampler"]
        )
        adapter_expectations = (
            (
                registry_tool.PROJECT_TUNED_SPRITE_ALPHA_TWO_SIDED_ADAPTER_ID,
                "SPRITE_PARTICLE",
                1,
                ("RS_Cull_None", "DSS_ReadOnly", "BS_EffectAlpha"),
            ),
            (
                registry_tool.PROJECT_TUNED_SPRITE_ADDITIVE_TWO_SIDED_ADAPTER_ID,
                "SPRITE_PARTICLE",
                2,
                ("RS_Cull_None", "DSS_ReadOnly", "BS_EffectAdditive"),
            ),
            (
                registry_tool.PROJECT_TUNED_SPRITE_ALPHA_ONE_SIDED_ADAPTER_ID,
                "SPRITE_PARTICLE",
                3,
                ("RS_Default", "DSS_ReadOnly", "BS_EffectAlpha"),
            ),
            (
                registry_tool.PROJECT_TUNED_SPRITE_ADDITIVE_ONE_SIDED_ADAPTER_ID,
                "SPRITE_PARTICLE",
                4,
                ("RS_Default", "DSS_ReadOnly", "BS_EffectAdditive"),
            ),
            (
                registry_tool.PROJECT_TUNED_MESH_ALPHA_TWO_SIDED_ADAPTER_ID,
                "MESH_PARTICLE",
                1,
                ("RS_Cull_None", "DSS_ReadOnly", "BS_EffectAlpha"),
            ),
            (
                registry_tool.PROJECT_TUNED_MESH_ADDITIVE_TWO_SIDED_ADAPTER_ID,
                "MESH_PARTICLE",
                2,
                ("RS_Cull_None", "DSS_ReadOnly", "BS_EffectAdditive"),
            ),
            (
                registry_tool.PROJECT_TUNED_MESH_ALPHA_ONE_SIDED_ADAPTER_ID,
                "MESH_PARTICLE",
                3,
                ("RS_Default", "DSS_ReadOnly", "BS_EffectAlpha"),
            ),
            (
                registry_tool.PROJECT_TUNED_MESH_ADDITIVE_ONE_SIDED_ADAPTER_ID,
                "MESH_PARTICLE",
                4,
                ("RS_Default", "DSS_ReadOnly", "BS_EffectAdditive"),
            ),
        )
        for opcode, color_space, program_id, layout_id in (
            (
                1001,
                "srgb",
                registry_tool.PROJECT_TUNED_APPROX_PROGRAM_SRGB_ID,
                registry_tool.PROJECT_TUNED_APPROX_LAYOUT_SRGB_ID,
            ),
            (
                1002,
                "linear",
                registry_tool.PROJECT_TUNED_APPROX_PROGRAM_LINEAR_ID,
                registry_tool.PROJECT_TUNED_APPROX_LAYOUT_LINEAR_ID,
            ),
        ):
            with self.subTest(opcode=opcode):
                program = {
                    "programId": program_id,
                    "backend": "runtimeMaterialV2",
                    "opcode": opcode,
                    "fidelity": "PROJECT_TUNED_APPROX",
                }
                layout = {
                    "layoutId": layout_id,
                    **copy.deepcopy(
                        registry_tool.COMPILED_PROGRAM_LAYOUT_ABIS[
                            ("runtimeMaterialV2", opcode)
                        ]
                    ),
                }
                descriptor = {
                    "descriptorId": f"effect.descriptor.project-tuned-approx.{opcode}.v1",
                    "layoutId": layout_id,
                    "textureLanes": [
                        {
                            "laneId": "lane.0",
                            "assetId": "Effect/Artist/Textures/fx_d_noise_003.dds",
                            "sampler": sampler,
                        }
                    ],
                    "scalars": [{
                        "name": "coverage-channel-selector.0",
                        "value": 2.0,
                    }],
                    "vectors": [],
                    "artistParameters": [],
                    "colors": [],
                }
                registry_tool._validate_compiled_program_layout_abi(
                    program, layout, f"project-tuned-{opcode}"
                )
                self.assertEqual("base_coverage", layout["textureLanes"][0]["role"])
                self.assertEqual("RGBA", layout["textureLanes"][0]["sourceChannel"])
                self.assertEqual(color_space, layout["textureLanes"][0]["colorSpace"])
                self.assertEqual(0x0F, layout["dynamicSuppressedMask"])
                self.assertEqual(0x0F, layout["particleColorConsumedMask"])
                self.assertEqual(1, layout["scalarCount"])
                self.assertEqual(1, layout["inputCount"])
                self.assertEqual([1, 0], layout["inputConsumedMask"])
                self.assertEqual(0x2F, layout["renderConsumedMask"])
                self.assertEqual(0x10, layout["renderSuppressedMask"])

                for adapter_id, carrier, pass_index, render_state in adapter_expectations:
                    adapter = registry_tool.COMPILED_ADAPTERS[adapter_id]
                    self.assertEqual(carrier, adapter["carrier"])
                    self.assertIn(("runtimeMaterialV2", opcode), adapter["programs"])
                    binding = {
                        "effectAssetId": "effect.project-tuned-approx.test",
                        "elementId": "sprite.synthetic",
                        "programId": program_id,
                        "layoutId": layout_id,
                        "descriptorId": descriptor["descriptorId"],
                        "adapterId": adapter_id,
                        "inlineMirrorPolicy": registry_tool.INLINE_MIRROR_REQUIRED,
                    }
                    packet = registry_tool.materialize_binding(
                        binding,
                        {program_id: program},
                        {layout_id: layout},
                        {descriptor["descriptorId"]: descriptor},
                    )
                    self.assertEqual(pass_index, packet["passIndex"])
                    self.assertEqual("PROJECT_TUNED_APPROX", packet["fidelity"])
                    self.assertEqual(render_state[0], packet["renderState"]["rasterizer"])
                    self.assertEqual(render_state[1], packet["renderState"]["depthStencil"])
                    self.assertEqual(render_state[2], packet["renderState"]["blend"])
                    self.assertEqual(1, packet["textureLaneCount"])
                    self.assertEqual(
                        [{
                            "name": "coverage-channel-selector.0",
                            "packedIndex": 0,
                            "value": 2.0,
                        }],
                        packet["scalars"],
                    )
                    self.assertEqual([], packet["vectors"])

        drifted = {
            "layoutId": registry_tool.PROJECT_TUNED_APPROX_LAYOUT_SRGB_ID,
            **copy.deepcopy(
                registry_tool.COMPILED_PROGRAM_LAYOUT_ABIS[
                    ("runtimeMaterialV2", 1001)
                ]
            ),
        }
        drifted["textureLanes"][0]["colorSpace"] = "linear"
        with self.assertRaisesRegex(
            registry_tool.ContractError, "Program/Layout ABI mismatch at textureLanes"
        ):
            registry_tool._validate_compiled_program_layout_abi(
                {
                    "programId": registry_tool.PROJECT_TUNED_APPROX_PROGRAM_SRGB_ID,
                    "backend": "runtimeMaterialV2",
                    "opcode": 1001,
                    "fidelity": "PROJECT_TUNED_APPROX",
                },
                drifted,
                "project-tuned-color-space-drift",
            )

    def test_binding_carrier_mismatch_fails_closed(self) -> None:
        registry = make_bound_registry("mesh.062366ee9f9655d3")
        with self.assertRaisesRegex(
            registry_tool.ContractError, "carrier is incompatible"
        ):
            registry_tool.validate_registry(registry, make_authored_documents())

        profile_drift = make_authored_documents()
        target = next(
            element
            for element in profile_drift[EFFECT_ID]["elements"]
            if element["id"] == ELEMENT_ID
        )
        target["material"]["renderProfile"] = "additive_one_sided_depth_read"
        with self.assertRaisesRegex(
            registry_tool.ContractError, "carrier is incompatible"
        ):
            registry_tool.validate_registry(
                make_bound_registry(ELEMENT_ID), profile_drift
            )

    def test_float_bit_drift_and_signed_zero_fail_closed(self) -> None:
        registry = make_bound_registry(ELEMENT_ID)
        authored = make_authored_documents()

        drifted = copy.deepcopy(registry)
        drifted["descriptors"][0]["scalars"][2]["value"] = 0.10000002
        with self.assertRaisesRegex(registry_tool.ContractError, "float-bit mismatch"):
            registry_tool.validate_registry(drifted, authored)

        signed_zero_registry = copy.deepcopy(registry)
        signed_zero_authored = copy.deepcopy(authored)
        signed_zero_registry["descriptors"][0]["scalars"][0]["value"] = 0.0
        target = next(
            element
            for element in signed_zero_authored[EFFECT_ID]["elements"]
            if element["id"] == ELEMENT_ID
        )
        target["material"]["execution"]["scalars"][0]["value"] = -0.0
        with self.assertRaisesRegex(registry_tool.ContractError, "float-bit mismatch"):
            registry_tool.validate_registry(signed_zero_registry, signed_zero_authored)

    def test_duplicate_dangling_unknown_adapter_and_order_fail_closed(self) -> None:
        mutations: list[dict[str, object]] = []

        duplicate = make_registry()
        duplicate["programs"].append(copy.deepcopy(duplicate["programs"][0]))
        mutations.append(duplicate)

        dangling = make_registry()
        dangling["descriptors"][0]["layoutId"] = "effect.layout.missing.v1"
        mutations.append(dangling)

        unknown_adapter = make_registry()
        unknown_adapter["adapters"][0]["adapterId"] = "effect.adapter.unknown.v1"
        mutations.append(unknown_adapter)

        reordered = make_registry()
        program = reordered["programs"][0]
        reordered["programs"][0] = {
            "backend": program["backend"],
            "programId": program["programId"],
            "opcode": program["opcode"],
            "fidelity": program["fidelity"],
        }
        mutations.append(reordered)

        for index, mutation in enumerate(mutations):
            with self.subTest(index=index):
                with self.assertRaises(registry_tool.ContractError):
                    registry_tool.validate_registry(mutation)

    def test_adapter_program_opcode_mismatch_fails_closed(self) -> None:
        registry = make_bound_registry(ELEMENT_ID)
        registry["programs"][0]["opcode"] = 14
        with self.assertRaisesRegex(
            registry_tool.ContractError, "no compiled Program/Layout ABI receipt"
        ):
            registry_tool.validate_registry(registry, make_authored_documents())

    def test_program_fidelity_opcode_mismatch_fails_closed(self) -> None:
        registry = make_registry()
        registry["programs"][0]["fidelity"] = "PROJECT_TUNED_APPROX"
        with self.assertRaisesRegex(
            registry_tool.ContractError, "fidelity/opcode contract changed"
        ):
            registry_tool.validate_registry(registry)

    def test_program_layout_abi_and_unbound_metadata_fail_closed(self) -> None:
        registry = make_bound_registry(ELEMENT_ID)
        registry["programs"][0]["opcode"] = 3
        registry["bindings"][0]["adapterId"] = (
            registry_tool.MESH_ALPHA_TWO_SIDED_ADAPTER_ID
        )
        with self.assertRaisesRegex(
            registry_tool.ContractError, "canonical compiled ID"
        ):
            registry_tool.validate_registry(registry, make_authored_documents())

        drifted_role = make_bound_registry(ELEMENT_ID)
        drifted_role["layouts"][0]["textureLanes"][0]["role"] = "wrong_role"
        with self.assertRaisesRegex(
            registry_tool.ContractError, "does not match exactly one compiled ABI receipt"
        ):
            registry_tool.validate_registry(
                drifted_role, make_authored_documents()
            )

        metadata_only = make_registry()
        metadata_only["bindings"] = []
        metadata_only["programs"].append(
            {
                "programId": "effect.program.runtime-material-v2.opcode-7.unbound.v1",
                "backend": "runtimeMaterialV2",
                "opcode": 7,
                "fidelity": "SOURCE_EXACT",
            }
        )
        with self.assertRaisesRegex(
            registry_tool.ContractError, "no compiled Program/Layout ABI receipt"
        ):
            registry_tool.validate_registry(metadata_only)

    def test_binding_reference_target_and_packet_mismatch_fail_closed(self) -> None:
        registry = make_bound_registry(ELEMENT_ID)

        with self.assertRaisesRegex(registry_tool.ContractError, "requires authored"):
            registry_tool.validate_registry(registry)

        missing_target = copy.deepcopy(registry)
        missing_target["bindings"][0]["elementId"] = "sprite.missing"
        with self.assertRaisesRegex(registry_tool.ContractError, "missing or duplicate"):
            registry_tool.validate_registry(missing_target, make_authored_documents())

        mismatched_lane = copy.deepcopy(registry)
        mismatched_lane["descriptors"][0]["textureLanes"][0]["assetId"] = (
            "Effect/Artist/Textures/fx_c_noise_002.dds"
        )
        with self.assertRaisesRegex(registry_tool.ContractError, "mismatch"):
            registry_tool.validate_registry(mismatched_lane, make_authored_documents())

    def test_layout_descriptor_count_mask_and_register_mismatch_fail_closed(self) -> None:
        mutations: list[dict[str, object]] = []

        bad_mask = make_registry()
        bad_mask["layouts"][0]["textureMask"] = 1
        mutations.append(bad_mask)

        bad_register = make_registry()
        bad_register["layouts"][0]["textureLanes"][1]["textureRegister"] = 2
        mutations.append(bad_register)

        bad_sampler_register = make_registry()
        bad_sampler_register["layouts"][0]["textureLanes"][1][
            "samplerRegister"
        ] = 7
        mutations.append(bad_sampler_register)

        bad_scalar_count = make_registry()
        bad_scalar_count["layouts"][0]["scalarCount"] = 2
        mutations.append(bad_scalar_count)

        bad_descriptor_lane = make_registry()
        bad_descriptor_lane["descriptors"][0]["textureLanes"][0]["laneId"] = "lane.wrong"
        mutations.append(bad_descriptor_lane)

        long_source_channel = make_registry()
        long_source_channel["layouts"][0]["textureLanes"][0]["sourceChannel"] = (
            "R" * 33
        )
        mutations.append(long_source_channel)

        bad_particle_policy = make_registry()
        bad_particle_policy["layouts"][0]["particleColorPolicy"] = 4
        mutations.append(bad_particle_policy)

        bad_dynamic_mask = make_registry()
        bad_dynamic_mask["layouts"][0]["dynamicConsumedMask"] = 0x10
        mutations.append(bad_dynamic_mask)

        for index, mutation in enumerate(mutations):
            with self.subTest(index=index):
                with self.assertRaises(registry_tool.ContractError):
                    registry_tool.validate_registry(mutation)

    def test_duplicate_property_bom_crlf_and_nonfinite_are_rejected(self) -> None:
        source = SOURCE_PATH.read_bytes()
        fragment = ARTIST_FRAGMENT_PATH.read_bytes()
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "registry.json"
            cases = (
                source.replace(
                    b'  "formatVersion": 1,',
                    b'  "formatVersion": 1,\n  "formatVersion": 1,',
                    1,
                ),
                b"\xef\xbb\xbf" + source,
                source.replace(b"\n", b"\r\n"),
                fragment.replace(b"0.100000001", b"NaN", 1),
            )
            for index, payload in enumerate(cases):
                with self.subTest(index=index):
                    path.write_bytes(payload)
                    with self.assertRaises(registry_tool.ContractError):
                        registry_tool.load_json(path)

            path.write_bytes(cases[0])
            with redirect_stderr(io.StringIO()):
                self.assertEqual(1, registry_tool.main(["--source", str(path)]))

    def test_fragment_duplicate_domain_and_unowned_adapter_fail_closed(self) -> None:
        fragment = registry_tool.load_json(ARTIST_FRAGMENT_PATH)
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            first = root / "a.material-program-fragment.v1.json"
            second = root / "b.material-program-fragment.v1.json"
            fragment_bytes = (
                json.dumps(fragment, separators=(",", ":")) + "\n"
            ).encode("utf-8")
            first.write_bytes(fragment_bytes)
            second.write_bytes(fragment_bytes)
            with self.assertRaisesRegex(registry_tool.ContractError, "duplicate.*domainId"):
                registry_tool.build_registry(
                    SOURCE_PATH, EFFECT_CATALOG_PATH, DATA_ROOT, root
                )

            second.unlink()
            fragment_with_adapter = copy.deepcopy(fragment)
            fragment_with_adapter["adapters"] = []
            first.write_bytes(
                (
                    json.dumps(fragment_with_adapter, separators=(",", ":"))
                    + "\n"
                ).encode("utf-8")
            )
            with self.assertRaisesRegex(registry_tool.ContractError, "fields or order"):
                registry_tool.build_registry(
                    SOURCE_PATH, EFFECT_CATALOG_PATH, DATA_ROOT, root
                )

    def test_base_domain_rows_and_compiled_adapter_drift_fail_closed(self) -> None:
        base = registry_tool.load_json(SOURCE_PATH)
        with tempfile.TemporaryDirectory() as temporary:
            missing_fragment_root = Path(temporary) / "missing"

            domain_row = copy.deepcopy(base)
            domain_row["programs"] = [
                {
                    "programId": "effect.program.illegal-base-row.v1",
                    "backend": "runtimeMaterialV2",
                    "opcode": 6,
                    "fidelity": "SOURCE_EXACT",
                }
            ]
            with self.assertRaisesRegex(
                registry_tool.ContractError, "base.programs must be empty"
            ):
                registry_tool.merge_registry_fragments(
                    domain_row, missing_fragment_root
                )

            for mutation in (
                list(reversed(base["adapters"])),
                base["adapters"][:-1],
                base["adapters"]
                + [{"adapterId": "effect.adapter.uncompiled.v1"}],
            ):
                with self.subTest(adapter_count=len(mutation)):
                    drifted = copy.deepcopy(base)
                    drifted["adapters"] = mutation
                    with self.assertRaisesRegex(
                        registry_tool.ContractError,
                        "must exactly match the compiled adapter table",
                    ):
                        registry_tool.merge_registry_fragments(
                            drifted, missing_fragment_root
                        )

if __name__ == "__main__":
    unittest.main()
