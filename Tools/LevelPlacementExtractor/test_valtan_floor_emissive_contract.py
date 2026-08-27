from __future__ import annotations

import argparse
import math
import os
import re
import shlex
import struct
import sys
import unittest
from pathlib import Path

import build_valtan_floor_collapse as floor_builder
import build_valtan_floor_emissive_fill as emissive_fill_builder


REPO_ROOT = Path(__file__).resolve().parents[2]
AREA_ID = "LV_LUT_HEARTRB_ED"
DEPLOY_CATALOG = (
    REPO_ROOT
    / "Data"
    / "Maps"
    / "Imported"
    / AREA_ID
    / f"{AREA_ID}.deployassets"
)
DEFAULT_RESOURCE_ROOT = REPO_ROOT / "Client" / "Bin" / "Resources"
RESOURCE_ROOT_ENV = "LOSTARK_RESOURCE_ROOT"


def resolve_resource_root(explicit: Path | None = None) -> Path:
    candidate = explicit
    if candidate is None:
        environment_value = os.environ.get(RESOURCE_ROOT_ENV, "").strip()
        candidate = (
            Path(environment_value) if environment_value else DEFAULT_RESOURCE_ROOT
        )
    if not candidate.is_absolute():
        candidate = Path.cwd() / candidate
    return candidate.resolve()


RESOURCE_ROOT = resolve_resource_root()

FLOOR_ASSETS = {
    "VALTAN_FLOOR_BRICK_A": "BG_RAD_VALTAN_FLOOR01A_SM",
    "VALTAN_FLOOR_BRICK_B": "BG_RAD_VALTAN_FLOOR01B_SM",
}

FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4s7I")
SECTION_DESC = struct.Struct("<IIQQ40s")
MATERIAL_META = struct.Struct("<4sI")
MATERIAL_PREFIX = struct.Struct("<IQ64s")
MATERIAL_PATH_NAMES = (
    "baseColor",
    "normal",
    "specular",
    "emissive",
    "opacity",
    "orm",
    "metallic",
    "roughness",
    "ambientOcclusion",
)
FIXED_WCHAR_PATH_BYTES = 260 * 2
MATERIAL_V2_ENTRY_SIZE = MATERIAL_PREFIX.size + (
    len(MATERIAL_PATH_NAMES) * FIXED_WCHAR_PATH_BYTES
)


def read_source(relative: str) -> str:
    payload = (REPO_ROOT / relative).read_bytes()
    try:
        text = payload.decode("utf-8")
    except UnicodeDecodeError:
        # Existing Engine headers preserve their original Korean Windows codepage.
        text = payload.decode("cp949")
    return text.replace("\r\n", "\n")


def braced_block(source: str, marker: str) -> str:
    marker_position = source.find(marker)
    if marker_position < 0:
        raise AssertionError(f"source marker is missing: {marker}")
    opening = source.find("{", marker_position)
    if opening < 0:
        raise AssertionError(f"source marker has no body: {marker}")
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[marker_position : index + 1]
    raise AssertionError(f"source marker has an unterminated body: {marker}")


def decode_fixed_wchar(value: bytes) -> str:
    decoded = value.decode("utf-16le")
    return decoded.split("\0", 1)[0]


def parse_wmodel_materials(path: Path) -> dict[int, dict[str, str]]:
    data = path.read_bytes()
    if len(data) < FILE_HEADER.size + MODEL_HEADER.size:
        raise AssertionError(f"WModel is truncated: {path}")

    magic, major, _minor, flags, content_size = FILE_HEADER.unpack_from(data, 0)
    if (
        magic != b"WINT"
        or major != 1
        or flags != 0
        or content_size != len(data) - FILE_HEADER.size
    ):
        raise AssertionError(f"WModel outer header is invalid: {path}")

    content_offset = FILE_HEADER.size
    model_magic, section_count, _animation_count, _model_flags, *_reserved = (
        MODEL_HEADER.unpack_from(data, content_offset)
    )
    if model_magic != b"WMOD" or section_count == 0:
        raise AssertionError(f"WModel metadata is invalid: {path}")

    section_table = content_offset + MODEL_HEADER.size
    if section_table + section_count * SECTION_DESC.size > len(data):
        raise AssertionError(f"WModel section table is truncated: {path}")

    material_sections: list[bytes] = []
    for row in range(section_count):
        type_id, _index, offset, size, _name = SECTION_DESC.unpack_from(
            data, section_table + row * SECTION_DESC.size
        )
        if offset > content_size or size > content_size - offset:
            raise AssertionError(f"WModel section is out of range: {path}")
        if type_id == 2:
            start = content_offset + offset
            material_sections.append(data[start : start + size])
    if len(material_sections) != 1:
        raise AssertionError(f"WModel must contain one material section: {path}")

    material = material_sections[0]
    if len(material) < FILE_HEADER.size + MATERIAL_META.size:
        raise AssertionError(f"WModel material section is truncated: {path}")
    mat_outer = FILE_HEADER.unpack_from(material, 0)
    mat_magic, material_count = MATERIAL_META.unpack_from(material, FILE_HEADER.size)
    expected_size = FILE_HEADER.size + MATERIAL_META.size + (
        material_count * MATERIAL_V2_ENTRY_SIZE
    )
    if (
        mat_outer[0] != b"WINT"
        or mat_outer[1] != 1
        or mat_outer[3] != 0
        or mat_outer[4] != len(material) - FILE_HEADER.size
        or mat_magic != b"WMA2"
        or len(material) != expected_size
    ):
        raise AssertionError(f"WModel WMA2 material container is invalid: {path}")

    result: dict[int, dict[str, str]] = {}
    cursor = FILE_HEADER.size + MATERIAL_META.size
    for _row in range(material_count):
        material_index, _material_hash, _name = MATERIAL_PREFIX.unpack_from(
            material, cursor
        )
        cursor += MATERIAL_PREFIX.size
        paths: dict[str, str] = {}
        for slot_name in MATERIAL_PATH_NAMES:
            paths[slot_name] = decode_fixed_wchar(
                material[cursor : cursor + FIXED_WCHAR_PATH_BYTES]
            )
            cursor += FIXED_WCHAR_PATH_BYTES
        if material_index in result:
            raise AssertionError(f"WModel material index is duplicated: {path}")
        result[material_index] = paths
    return result


def parse_deploy_catalog() -> dict[str, list[str]]:
    lines = [
        line
        for line in DEPLOY_CATALOG.read_text(encoding="utf-8").splitlines()
        if line.strip()
    ]
    header = shlex.split(lines[0])
    if header[:3] != ["LOSTARK_DEPLOY_PROP_CATALOG", "2", AREA_ID]:
        raise AssertionError("DeployProp catalog must use the Area schema-v2 header")
    if len(header) != 4 or int(header[3]) != len(lines) - 1:
        raise AssertionError("DeployProp catalog row count differs from its header")

    result: dict[str, list[str]] = {}
    for index, line in enumerate(lines[1:]):
        tokens = shlex.split(line)
        if len(tokens) != 10:
            raise AssertionError(
                f"DeployProp catalog v2 row {index} must have 10 fields"
            )
        if tokens[0] in result:
            raise AssertionError(f"DeployProp asset is duplicated: {tokens[0]}")
        result[tokens[0]] = tokens
    return result


class ValtanFloorEmissiveContractTests(unittest.TestCase):
    def test_catalog_profiles_only_the_two_crack_floor_assets(self) -> None:
        rows = parse_deploy_catalog()
        self.assertTrue(FLOOR_ASSETS.keys() <= rows.keys())
        for asset_id, tokens in rows.items():
            intensity = float(tokens[7])
            overlay = int(tokens[8])
            self.assertTrue(math.isfinite(intensity) and intensity >= 0.0)
            self.assertIn(overlay, (0, 1))
            if asset_id in FLOOR_ASSETS:
                self.assertEqual(1.5, intensity, asset_id)
                self.assertEqual(1, overlay, asset_id)
                self.assertEqual("STATIC", tokens[1], asset_id)
            else:
                self.assertEqual(1.0, intensity, asset_id)
                self.assertEqual(0, overlay, asset_id)

        for asset_id, model_name in FLOOR_ASSETS.items():
            self.assertEqual(
                f"Map/BG_RAD_VALTAN_A/{model_name}/{model_name}.wmodel",
                rows[asset_id][3],
            )

    def test_generator_migrates_v1_profiles_and_rejects_bad_v2_flags(self) -> None:
        v1_rows = [
            '"DEPLOY_ITR_02306" STATIC "ordinary" "ordinary.wmodel" '
            '"ordinary.prototype" "" "" "ordinary evidence"',
            '"VALTAN_FLOOR_BRICK_A" STATIC "floor A" "floor-a.wmodel" '
            '"floor-a.prototype" "" "" "floor evidence"',
        ]
        migrated = floor_builder.normalize_deploy_asset_rows(
            ["LOSTARK_DEPLOY_PROP_CATALOG", "1", AREA_ID, "2"], v1_rows
        )
        ordinary = shlex.split(migrated[0])
        floor_a = shlex.split(migrated[1])
        self.assertEqual(["1", "0"], ordinary[7:9])
        self.assertEqual(["1.5", "1"], floor_a[7:9])
        self.assertEqual("ordinary evidence", ordinary[9])
        self.assertEqual("floor evidence", floor_a[9])

        invalid = [
            '"ANIM_OVERLAY" ANIM "bad" "bad.wmodel" "bad.prototype" '
            '"" "" 1 1 "bad evidence"'
        ]
        with self.assertRaises(floor_builder.BuildError):
            floor_builder.normalize_deploy_asset_rows(
                ["LOSTARK_DEPLOY_PROP_CATALOG", "2", AREA_ID, "1"], invalid
            )

    def test_floor_wmodels_bind_exact_material_one_crack_resources(self) -> None:
        for model_name in FLOOR_ASSETS.values():
            with self.subTest(model=model_name):
                model_relative = Path(
                    "Map/BG_RAD_VALTAN_A"
                ) / model_name / f"{model_name}.WModel"
                model_path = RESOURCE_ROOT / model_relative
                self.assertTrue(
                    model_path.is_file(),
                    f"runtime WModel is missing under {RESOURCE_ROOT}; pass "
                    f"--resource-root or set {RESOURCE_ROOT_ENV}: {model_path}",
                )
                materials = parse_wmodel_materials(model_path)
                self.assertIn(1, materials)

                prefix = (
                    f"Resource/LostArk/Map/BG_RAD_VALTAN_A/{model_name}/textures/"
                )
                expected = {
                    "baseColor": prefix + "bg_rad_valtan_crack_floor01_d_lsj.png",
                    "normal": prefix + "bg_rad_valtan_crack_floor01_n_lsj.png",
                    "emissive": prefix
                    + "bg_rad_valtan_crack_floor01_em_reconstruction.png",
                }
                for slot_name, stored_path in expected.items():
                    self.assertEqual(stored_path, materials[1][slot_name])
                    relative = stored_path.removeprefix("Resource/LostArk/")
                    resource = RESOURCE_ROOT / relative
                    self.assertTrue(resource.is_file(), resource)
                    self.assertEqual(b"\x89PNG\r\n\x1a\n", resource.read_bytes()[:8])

    def test_renderer_runs_overlay_after_opaque_inside_game_object_mrt(self) -> None:
        enum_source = read_source("Engine/Public/Engine_Enum.h")
        render_group = re.search(
            r"enum class RENDERGROUP\s*\{(?P<body>[^}]*)\}", enum_source
        )
        self.assertIsNotNone(render_group)
        ordered_groups = [
            value.strip()
            for value in render_group.group("body").split(",")
            if value.strip()
        ]
        self.assertLess(
            ordered_groups.index("NONBLEND"),
            ordered_groups.index("DEFERRED_OVERLAY"),
        )
        self.assertLess(
            ordered_groups.index("DEFERRED_OVERLAY"),
            ordered_groups.index("NONLIGHT"),
        )

        game_object = read_source("Engine/Public/GameObject.h")
        self.assertIn("virtual HRESULT Render_DeferredOverlay();", game_object)
        default_overlay = braced_block(
            read_source("Engine/Private/GameObject.cpp"),
            "HRESULT CGameObject::Render_DeferredOverlay()",
        )
        self.assertIn("return S_OK;", default_overlay)

        renderer = braced_block(
            read_source("Engine/Private/Renderer.cpp"),
            "HRESULT CRenderer::Render_NonBlend()",
        )
        ordered_tokens = (
            'Begin_MRT(TEXT("MRT_GameObject"))',
            "for (auto& pRenderObject : NonBlendObjects)",
            "pRenderObject->Render();",
            "for (auto& pRenderObject : DeferredOverlayObjects)",
            "pRenderObject->Render_DeferredOverlay();",
            "End_MRT()",
        )
        positions = [renderer.find(token) for token in ordered_tokens]
        self.assertNotIn(-1, positions)
        self.assertEqual(sorted(positions), positions)

    def test_shader_pass_18_writes_only_emissive_with_read_only_depth(self) -> None:
        shader = read_source("Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl")
        pass_marker = "pass DeferredEmissiveOverlayPass"
        pass_position = shader.find(pass_marker)
        self.assertGreaterEqual(pass_position, 0)
        passes_before = re.findall(r"(?m)^\s*pass\s+\w+", shader[:pass_position])
        self.assertEqual(18, len(passes_before))

        shader_pass = braced_block(shader, pass_marker)
        self.assertIn("SetRasterizerState(RS_DeferredEmissiveOverlay)", shader_pass)
        self.assertIn("SetDepthStencilState(DSS_ReadOnly, 0)", shader_pass)
        self.assertIn("SetBlendState(BS_DeferredEmissiveOverlay", shader_pass)
        self.assertIn("PS_MAIN_DEFERRED_EMISSIVE_OVERLAY()", shader_pass)

        pixel_contract_start = shader.find("struct PS_OUT_DEFERRED_EMISSIVE_OVERLAY")
        raster_start = shader.find("RasterizerState RS_DeferredEmissiveOverlay")
        self.assertGreater(raster_start, pixel_contract_start)
        pixel_contract = shader[pixel_contract_start:raster_start]
        self.assertIn("SV_TARGET4", pixel_contract)
        self.assertNotRegex(pixel_contract, r"SV_TARGET[0-3]")
        self.assertIn("g_EmissiveIntensity", pixel_contract)
        self.assertRegex(pixel_contract, r"clip\s*\(\s*max\(")
        self.assertIn("1.f / 255.f", pixel_contract)

        raster = braced_block(shader, "RasterizerState RS_DeferredEmissiveOverlay")
        depth_bias = re.search(r"DepthBias\s*=\s*(-?\d+)\s*;", raster)
        slope_bias = re.search(
            r"SlopeScaledDepthBias\s*=\s*(-?[0-9.]+f?)\s*;", raster
        )
        self.assertIsNotNone(depth_bias)
        self.assertIsNotNone(slope_bias)
        self.assertLess(int(depth_bias.group(1)), 0)
        self.assertLess(float(slope_bias.group(1).rstrip("f")), 0.0)

    def test_dynamic_map_shader_keeps_water_and_valtan_vortex_abi_aligned(
        self,
    ) -> None:
        shader = read_source("Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl")
        map_object = read_source("Client/Private/MapAssetObject.cpp")
        render_utils = read_source("Client/Private/MapAssetRenderUtils.cpp")
        deploy = read_source("Client/Private/DeployPropObject.cpp")

        water_bind = braced_block(
            map_object,
            "HRESULT CMapAssetObject::Bind_WaterShaderResources(",
        )
        bound_names = set(re.findall(r'"(g_[A-Za-z0-9_]+)"', water_bind))
        self.assertGreaterEqual(len(bound_names), 17)
        shader_globals = shader[: shader.index("struct VS_IN")]
        for binding_name in sorted(bound_names):
            self.assertRegex(shader_globals, rf"\b{re.escape(binding_name)}\b")

        passes = re.findall(r"(?m)^\s*pass\s+(\w+)", shader)
        self.assertEqual(
            [
                "WaterBackPass",
                "WaterFrontPass",
                "WaterTwoSidedPass",
                "DeferredEmissiveOverlayPass",
            ],
            passes[15:19],
        )
        self.assertRegex(
            render_utils,
            r"MAP_ASSET_RENDER_MODE::WATER\s*==\s*profile\.renderMode\s*\?\s*15u",
        )
        self.assertIn("DEFERRED_EMISSIVE_OVERLAY_PASS = 18u", deploy)

        # The green/blue square suppression is a separate Valtan presentation
        # contract and must survive the water ABI repair unchanged.
        self.assertIn("g_PresentationVortexProfile", shader_globals)
        self.assertIn("g_PresentationVortexStrength", shader_globals)
        for profile in (1, 2, 3):
            self.assertRegex(
                shader,
                rf"(?:if|else if)\s*\(\s*{profile}\s*==\s*"
                r"g_PresentationVortexProfile",
            )

        blend = braced_block(shader, "BlendState BS_DeferredEmissiveOverlay")
        for target in range(4):
            self.assertRegex(
                blend, rf"RenderTargetWriteMask\[{target}\]\s*=\s*0x00\s*;"
            )
        target_four = re.search(
            r"RenderTargetWriteMask\[4\]\s*=\s*(0x[0-9A-Fa-f]+)\s*;", blend
        )
        self.assertIsNotNone(target_four)
        self.assertIn(int(target_four.group(1), 16), (0x07, 0x0F))

    def test_deploy_runtime_queues_only_intact_overlay_and_suppresses_base_emission(
        self,
    ) -> None:
        catalog_header = read_source("Client/Public/DeployPropCatalog.h")
        catalog_source = read_source("Client/Private/DeployPropCatalog.cpp")
        runtime = read_source("Client/Private/DeployPropRuntime.cpp")
        deploy = read_source("Client/Private/DeployPropObject.cpp")
        canonical_producer = read_source(
            "Tools/LevelPlacementExtractor/build_deployprop_runtime.py"
        )

        self.assertIn("f32_t emissiveIntensity = 1.f;", catalog_header)
        self.assertIn("bool_t deferredEmissiveOverlay = false;", catalog_header)
        self.assertIn("constexpr uint32_t CATALOG_FORMAT_VERSION = 2;", catalog_source)
        self.assertIn("entry.emissiveIntensity", catalog_source)
        self.assertIn("deferredEmissiveOverlay > 1u", catalog_source)
        self.assertIn("desc.emissiveIntensity = asset->emissiveIntensity;", runtime)
        self.assertIn(
            "desc.deferredEmissiveOverlay = asset->deferredEmissiveOverlay;",
            runtime,
        )
        self.assertIn("LOSTARK_DEPLOY_PROP_CATALOG 2", canonical_producer)
        self.assertRegex(
            canonical_producer,
            r'quoted\(fractured_prototype\),\s*"1",\s*"0",\s*'
            r'quoted\(str\(definition\["evidence"\]\)\)',
        )

        late_update = braced_block(deploy, "void CDeployPropObject::Late_Update")
        self.assertIn("if (Should_RenderDeferredEmissiveOverlay())", late_update)
        self.assertIn("RENDERGROUP::DEFERRED_OVERLAY", late_update)

        should_render = braced_block(
            deploy, "bool_t CDeployPropObject::Should_RenderDeferredEmissiveOverlay()"
        )
        self.assertIn("m_bDeferredEmissiveOverlay", should_render)
        self.assertIn("DEPLOY_PROP_MODEL_KIND::STATIC == m_ModelKind", should_render)
        self.assertIn("DEPLOY_PROP_STATE::INTACT == m_State", should_render)
        self.assertIn("!Is_BasePresentationSuppressed()", should_render)

        base_render = braced_block(deploy, "HRESULT CDeployPropObject::Render()")
        self.assertRegex(
            base_render,
            r"m_bDeferredEmissiveOverlay\s*&&\s*"
            r"DEPLOY_PROP_STATE::INTACT\s*==\s*m_State",
        )

        static_render = braced_block(deploy, "HRESULT CDeployPropObject::Render_Static(")
        self.assertIn("suppressDeferredEmissive", static_render)
        self.assertRegex(
            static_render,
            r"!\(suppressDeferredEmissive\s*&&\s*1u\s*==\s*index\)",
        )

        overlay_render = braced_block(
            deploy, "HRESULT CDeployPropObject::Render_DeferredEmissiveOverlay()"
        )
        mesh_indices_match = re.search(
            r"EMISSIVE_MESH_INDICES\[\]\s*=\s*\{(?P<body>[^}]*)\}",
            overlay_render,
        )
        self.assertIsNotNone(mesh_indices_match)
        mesh_indices = [
            int(value)
            for value in re.findall(
                r"(?<![A-Za-z0-9_])(\d+)u", mesh_indices_match.group("body")
            )
        ]
        self.assertEqual(0, emissive_fill_builder.FILL_SUBMESH_INDEX)
        self.assertEqual(1, emissive_fill_builder.CRACK_RUBBLE_SUBMESH_INDEX)
        self.assertEqual(
            [
                emissive_fill_builder.FILL_SUBMESH_INDEX,
                emissive_fill_builder.CRACK_RUBBLE_SUBMESH_INDEX,
            ],
            mesh_indices,
        )
        self.assertIn("EMISSIVE_MESH_INDEX = 1u", overlay_render)
        self.assertIn("DEFERRED_EMISSIVE_OVERLAY_PASS = 18u", overlay_render)
        self.assertRegex(
            overlay_render,
            r"Get_NumMeshes\(\)\s*<=\s*EMISSIVE_MESH_INDEX",
        )
        self.assertIn("aiTextureType_EMISSIVE", overlay_render)
        # Surface transitions own the live color/intensity. Binding the original
        # catalog scalar would discard floor phase/transition presentation.
        self.assertRegex(
            overlay_render,
            r"const f32_t emissiveIntensity\s*=\s*"
            r"m_SurfacePresentation\.fEmissiveIntensity\s*\*\s*"
            r"m_SurfacePresentation\.fTransitionMultiplier\s*;",
        )
        self.assertIn('"g_EmissiveIntensity", &emissiveIntensity', overlay_render)
        self.assertIn('"g_EmissiveColor", &emissiveColor', overlay_render)
        self.assertIn('"g_EmissiveMaskPower", &emissiveMaskPower', overlay_render)
        self.assertIn(
            "for (const uint32_t meshIndex : EMISSIVE_MESH_INDICES)",
            overlay_render,
        )
        self.assertRegex(
            overlay_render,
            r"FAILED\(m_pShaderCom->Begin\(DEFERRED_EMISSIVE_OVERLAY_PASS\)\)\s*\|\|\s*"
            r"FAILED\(m_pIntactModelCom->Render\(meshIndex\)\)",
        )
        self.assertNotIn("Render(EMISSIVE_MESH_INDEX)", overlay_render)


if __name__ == "__main__":
    argument_parser = argparse.ArgumentParser(add_help=False)
    argument_parser.add_argument("--resource-root", type=Path)
    arguments, unittest_arguments = argument_parser.parse_known_args()
    if arguments.resource_root is not None:
        RESOURCE_ROOT = resolve_resource_root(arguments.resource_root)
    unittest.main(argv=[sys.argv[0], *unittest_arguments])
