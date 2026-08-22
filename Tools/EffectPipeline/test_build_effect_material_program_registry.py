#!/usr/bin/env python3

from __future__ import annotations

import copy
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
PROGRAM_ID = "effect.program.runtime-material-v2.opcode-6.artist-f-sprite.v1"
LAYOUT_ID = "effect.layout.runtime-material-v2.artist-f-sprite.v1"
DESCRIPTOR_ID = "effect.descriptor.artist-f.sprite-2b3dc6842507e910.v1"


def make_registry() -> dict[str, object]:
    return copy.deepcopy(registry_tool.load_json(SOURCE_PATH))


def make_binding(element_id: str = ELEMENT_ID) -> dict[str, object]:
    return {
        "effectAssetId": EFFECT_ID,
        "elementId": element_id,
        "programId": PROGRAM_ID,
        "layoutId": LAYOUT_ID,
        "descriptorId": DESCRIPTOR_ID,
        "adapterId": registry_tool.CANONICAL_SPRITE_ADAPTER_ID,
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
    def test_source_contains_binding_zero_registry(self) -> None:
        registry = registry_tool.build_registry(
            SOURCE_PATH, EFFECT_CATALOG_PATH, DATA_ROOT
        )
        self.assertEqual([], registry["bindings"])
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
        }
        mutations.append(reordered)

        for index, mutation in enumerate(mutations):
            with self.subTest(index=index):
                with self.assertRaises(registry_tool.ContractError):
                    registry_tool.validate_registry(mutation)

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

        for index, mutation in enumerate(mutations):
            with self.subTest(index=index):
                with self.assertRaises(registry_tool.ContractError):
                    registry_tool.validate_registry(mutation)

    def test_duplicate_property_bom_crlf_and_nonfinite_are_rejected(self) -> None:
        source = SOURCE_PATH.read_bytes()
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
                source.replace(b"0.100000001", b"NaN", 1),
            )
            for index, payload in enumerate(cases):
                with self.subTest(index=index):
                    path.write_bytes(payload)
                    with self.assertRaises(registry_tool.ContractError):
                        registry_tool.load_json(path)

            path.write_bytes(cases[0])
            with redirect_stderr(io.StringIO()):
                self.assertEqual(1, registry_tool.main(["--source", str(path)]))


if __name__ == "__main__":
    unittest.main()
