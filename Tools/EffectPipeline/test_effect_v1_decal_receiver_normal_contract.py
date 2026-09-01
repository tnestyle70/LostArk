from __future__ import annotations

import math
import pathlib
import re
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
V1_SHADER = ROOT / "Client/Bin/ShaderFiles/Shader_VtxEffectDecal.hlsl"
V2_SHADER = ROOT / "Client/Bin/ShaderFiles/Shader_EffectDecalV2.hlsl"
RENDERER = ROOT / "Engine/Private/Renderer.cpp"
GBUFFER_WRITERS = (
    ROOT / "Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl",
    ROOT / "Client/Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl",
    ROOT / "Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl",
    ROOT / "Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl",
    ROOT / "Client/Bin/ShaderFiles/Shader_VtxEstherNpc.hlsl",
)


def _normalize(value: tuple[float, float, float]) -> tuple[float, float, float]:
    length = math.sqrt(sum(component * component for component in value))
    if length <= 1.0e-6:
        return (0.0, 1.0, 0.0)
    return tuple(component / length for component in value)


def _dot(
    left: tuple[float, float, float],
    right: tuple[float, float, float],
) -> float:
    return sum(a * b for a, b in zip(left, right))


def _cross(
    left: tuple[float, float, float],
    right: tuple[float, float, float],
) -> tuple[float, float, float]:
    return (
        left[1] * right[2] - left[2] * right[1],
        left[2] * right[0] - left[0] * right[2],
        left[0] * right[1] - left[1] * right[0],
    )


def _receiver_normal(
    shading_normal: tuple[float, float, float],
    position_dx: tuple[float, float, float],
    position_dy: tuple[float, float, float],
) -> tuple[float, float, float]:
    shading = _normalize(shading_normal)
    geometric = _normalize(_cross(position_dx, position_dy))
    if _dot(geometric, shading) < 0.0:
        geometric = tuple(-component for component in geometric)
    return geometric


class EffectV1DecalReceiverNormalContractTests(unittest.TestCase):
    def test_v1_uses_depth_derived_receiver_plane_after_background_rejection(self) -> None:
        shader = V1_SHADER.read_text(encoding="utf-8-sig")
        self.assertIn("clip(0.99999f - depth.x);", shader)
        self.assertIn("clip(0.99999f - depth.y);", shader)
        self.assertIn("Resolve_DecalReceiverNormalV1", shader)
        self.assertIn("ddx(worldPosition)", shader)
        self.assertIn("ddy(worldPosition)", shader)
        self.assertIn("encodedSceneNormal * 2.f - 1.f", shader)
        self.assertIn(
            "dot(geometricNormal, normalizedShadingNormal) < 0.f", shader
        )
        self.assertIn(
            "clip(dot(receiverNormal, normalize(g_DecalUp)) -", shader
        )

    def test_flat_floor_passes_cutoff_despite_perturbed_lighting_normal(self) -> None:
        cutoff = 0.75
        up = (0.0, 1.0, 0.0)
        perturbed_shading_normal = _normalize((0.8, 0.4, 0.4472135955))
        self.assertLess(_dot(perturbed_shading_normal, up), cutoff)

        floor_receiver = _receiver_normal(
            perturbed_shading_normal,
            (1.0, 0.0, 0.0),
            (0.0, 0.0, -1.0),
        )
        self.assertGreaterEqual(_dot(floor_receiver, up), cutoff)

        wall_receiver = _receiver_normal(
            (1.0, 0.0, 0.0),
            (0.0, 1.0, 0.0),
            (0.0, 0.0, 1.0),
        )
        self.assertLess(_dot(wall_receiver, up), cutoff)

    def test_target_normal_is_encoded_lighting_normal_not_receiver_plane(self) -> None:
        renderer = RENDERER.read_text(encoding="utf-8-sig")
        self.assertIn('TEXT("Target_Normal")', renderer)
        self.assertIn("DXGI_FORMAT_R16G16B16A16_UNORM", renderer)
        for writer_path in GBUFFER_WRITERS:
            with self.subTest(writer=writer_path.name):
                writer = writer_path.read_text(encoding="utf-8-sig")
                self.assertRegex(
                    writer,
                    re.compile(
                        r"(?:output|Out)\.vNormal\s*=.*?"
                        r"\*\s*0\.5f\)?\s*\+\s*0\.5f",
                        re.DOTALL,
                    ),
                )
        map_writer = GBUFFER_WRITERS[1].read_text(encoding="utf-8-sig")
        self.assertIn("g_HasNormalTexture", map_writer)
        self.assertIn("tangentNormal", map_writer)
        self.assertIn("normal * 0.5f + 0.5f", map_writer)

    def test_v2_receiver_rule_is_not_changed_by_the_v1_fix(self) -> None:
        shader = V2_SHADER.read_text(encoding="utf-8-sig")
        self.assertNotIn("Resolve_DecalReceiverNormalV1", shader)
        self.assertIn(
            "g_NormalTexture.Sample(PointSampler, input.vTexcoord).xyz * 2.f - 1.f",
            shader,
        )
        self.assertIn(
            "clip(dot(normalize(sceneNormal), normalize(g_DecalUp))",
            shader,
        )


if __name__ == "__main__":
    unittest.main()
