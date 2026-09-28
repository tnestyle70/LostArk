"""Numerical regression for ocean-42's translated-world PS inputs (no UI)."""
from pathlib import Path
import math
import unittest

ROOT = Path(__file__).resolve().parents[2]


def source(v):
    return (v[0] * 100.0, -v[2] * 100.0, v[1] * 100.0)


def add(a, b):
    return tuple(x + y for x, y in zip(a, b))


def sub(a, b):
    return tuple(x - y for x, y in zip(a, b))


def mul(v, matrix):
    return tuple(sum(v[i] * matrix[i][j] for i in range(4)) for j in range(4))


class OceanCameraFrameTests(unittest.TestCase):
    def assertVector(self, a, b):
        for x, y in zip(a, b):
            self.assertAlmostEqual(x, y, places=7)

    def test_world_uv_and_camera_view_are_distinct(self):
        for world, camera in [((75, 19.63, -993), (75, 80, -930)),
                              ((140, 42, -70), (125, 68, -42)),
                              ((0, 0, 0), (3, 4, 5))]:
            origin = source(camera)
            translated = sub(source(world), origin)
            native_world = add(translated, origin)
            native_view = sub(origin, native_world)
            self.assertVector(native_world, source(world))
            self.assertVector(native_view, source(sub(camera, world)))

    def test_fresnel_does_not_depend_on_map_origin(self):
        world, camera = (75, 19.63, -993), (75, 80, -930)
        shift = (10000, -700, 5000)
        def fresnel(w, c):
            view = sub(source(c), source(w))
            return (1 - view[2] / math.sqrt(sum(x*x for x in view))) ** 5
        self.assertAlmostEqual(fresnel(world, camera),
                               fresnel(add(world, shift), add(camera, shift)), places=10)
        wrong_view = tuple(-x for x in source(world))
        wrong = (1 - wrong_view[2] / math.sqrt(sum(x*x for x in wrong_view))) ** 5
        self.assertGreater(abs(wrong - fresnel(world, camera)), 0.5)

    def test_depth_projection_is_unchanged(self):
        world, camera = (75, 19.63, -993), (80, 83, -920)
        # Nontrivial rows exercise every sign/axis and homogeneous translation.
        vp = ((1.1, .2, .3, .4), (.5, 1.2, .6, .7),
              (.8, .9, 1.3, 1.4), (-70, 20, 100, 1))
        translated = sub(source(world), source(camera)) + (1,)
        projection = (vp[0], tuple(-v for v in vp[2]), vp[1],
                      tuple(v*100 for v in mul(camera+(1,), vp)))
        self.assertVector(mul(translated, projection),
                          tuple(v*100 for v in mul(world+(1,), vp)))

    def test_both_shader_paths_consume_camera_origin(self):
        water = (ROOT/'Client/Bin/ShaderFiles/Shader_SourceMapWaterPrograms.hlsli').read_text()
        for name in ('SourceMapWater42', 'SourceMapWater42Baked'):
            block = water.split(name+'(', 1)[1].split('return output;', 1)[0]
            self.assertIn('source[0]=float4(input.values[9].xyz,1.f);', block)
        client = (ROOT/'Client/Bin/ShaderFiles/Shader_SourceMapForwardPrograms.hlsli').read_text()
        engine = (ROOT/'Engine/Bin/ShaderFiles/Shader_SourceMapForwardPrograms.hlsli').read_text()
        def wrapper(text):
            return text.split('float4 EvaluateSourceMapWater(', 1)[1].split(
                '#include "Shader_SourceMapTranslucentPrograms.hlsli"', 1)[0]
        self.assertEqual(wrapper(client), wrapper(engine))
        for line in ('input.values[8]=float4(sourcePosition.xyz-sourceCamera,1.f);',
                     'input.values[9]=float4(sourceCamera,1.f);',
                     'input.projection[3]=mul(float4(cameraPosition,1.f),vp)*100.f;'):
            self.assertIn(line, wrapper(client))


if __name__ == '__main__':
    unittest.main()
