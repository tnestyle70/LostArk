"""Native color ownership, ambiguous joins and byte-preserving WModel changes."""
from pathlib import Path
import copy
import json
import struct
import tempfile
import unittest

import numpy as np
import restore_static_source_colors as colors
import cook_wmodel_geometry_contract as geometry
import test_cook_wmodel_geometry_contract as fixtures


def native_fixture(rgba=None, duplicate=False):
    points = [(0., 0., 0.), (100., 0., 0.), (0., 100., 0.)]
    uv = [(0., 0., .25, .25), (1., 0., .75, .25), (0., 1., .25, .75)]
    if duplicate:
        points.append(points[0]); uv.append(uv[0])
    count = len(points)
    raw = bytearray(b'fixture-prefix')
    raw.extend(struct.pack('<4I', 12, count, 12, count))
    raw.extend(b''.join(struct.pack('<3f', *p) for p in points))
    raw.extend(struct.pack('<6I', 2, 16, count, 0, 16, count))
    for coord in uv:
        raw.extend(bytes((255, 128, 128, 128, 128, 255, 128, 255)))
        raw.extend(struct.pack('<4e', *coord))
    if rgba is None:
        raw.extend(struct.pack('<2I', 0, 0))
    else:
        raw.extend(struct.pack('<4I', 4, count, 4, count))
        raw.extend(b''.join(bytes((b, g, r, a)) for r, g, b, a in rgba))
    raw.extend(struct.pack('<I', count))
    arrays = dict(POSITION=np.array(points, dtype=np.float32)[:, [0, 2, 1]] * np.float32(.01),
                  NORMAL=np.tile((0., 0., 1.), (count, 1)), TANGENT=np.tile((1., 0., 0., 1.), (count, 1)),
                  TEXCOORD_0=np.array(uv)[:, :2], TEXCOORD_1=np.array(uv)[:, 2:],
                  COLOR_0=np.zeros((count, 4), dtype=np.uint8))
    return bytes(raw), [arrays]


class RestoreStaticSourceColorsTests(unittest.TestCase):
    def test_absent_native_colors_disregard_exporter_bytes(self):
        raw, arrays = native_fixture()
        arrays[0]['COLOR_0'][:] = (32, 5, 255, 128)
        mapped, report = colors.join_native_colors(raw, arrays)
        self.assertFalse(report['hasNativeColors'])
        self.assertEqual(mapped, [[None, None, None]])

    def test_bgra_maps_to_rgba_even_with_shared_primitive_vertices(self):
        values = [(3, 7, 19, 47), (32, 12, 9, 255), (255, 128, 1, 0)]
        raw, arrays = native_fixture(values)
        mapped, report = colors.join_native_colors(raw, arrays + arrays)
        self.assertEqual(mapped, [[bytes(v) for v in values]] * 2)
        self.assertEqual(report['nativeVertexCount'], 3)
        self.assertEqual(report['gltfVertexCount'], 6)

    def test_differing_colors_at_identical_geometry_are_rejected(self):
        raw, arrays = native_fixture([(1, 2, 3, 4)] * 3 + [(9, 8, 7, 6)], duplicate=True)
        with self.assertRaisesRegex(ValueError, 'Ambiguous native color'):
            colors.join_native_colors(raw, arrays)

    def test_uv_and_basis_mismatch_cannot_claim_absence(self):
        raw, arrays = native_fixture()
        for field in ('TEXCOORD_1', 'NORMAL'):
            changed = copy.deepcopy(arrays)
            changed[0][field][0, 0] += .5
            with self.assertRaisesRegex(ValueError, 'No native position/UV/basis match'):
                colors.join_native_colors(raw, changed)

    def test_remove_color_preserves_existing_vertex_order_and_material(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, model, _ = fixtures._cook_fixture(root, 'colors', True)
            original = model.read_bytes()
            doc = json.loads(source.read_text())
            for p in doc['meshes'][0]['primitives']:
                del p['attributes']['COLOR_0']
            source.write_text(json.dumps(doc))
            restored, report = colors.restore_model(original, source)
            result = geometry.parse_geometry_wmodel(restored)
            self.assertFalse(result['hasColor0'])
            self.assertTrue(report['nonColorVertexBytesIdentical'])
            self.assertTrue(report['materialBytesIdentical'])
            self.assertTrue(report['topologyIndicesBoundsByteIdentical'])
            self.assertEqual(model.read_bytes(), original)

    def test_color_restore_preserves_installed_handedness_on_export_difference(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, model, _ = fixtures._cook_fixture(root, 'signs', True)
            original = model.read_bytes()
            doc = json.loads(source.read_text())
            primitive = doc['meshes'][0]['primitives'][0]
            accessor = doc['accessors'][primitive['attributes']['TANGENT']]
            view = doc['bufferViews'][accessor['bufferView']]
            buffer_path = source.parent / doc['buffers'][view['buffer']]['uri']
            data = bytearray(buffer_path.read_bytes())
            start = view.get('byteOffset', 0) + accessor.get('byteOffset', 0)
            for i in range(accessor['count']):
                at = start + i * view.get('byteStride', 16) + 12
                struct.pack_into('<f', data, at, -struct.unpack_from('<f', data, at)[0])
            buffer_path.write_bytes(data)
            restored, report = colors.restore_model(original, source)
            self.assertEqual(report['submeshes'][0]['preservedInstalledHandednessDifferingFromFreshExport'], 3)
            self.assertEqual(restored, original)
            before = geometry.parse_geometry_wmodel(original)
            after = geometry.parse_geometry_wmodel(restored)
            self.assertEqual(before['submeshes'][0]['vertexBytes'], after['submeshes'][0]['vertexBytes'])


if __name__ == '__main__':
    unittest.main()
