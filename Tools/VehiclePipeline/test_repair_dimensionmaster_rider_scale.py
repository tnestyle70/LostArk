"""Focused mutation-boundary regressions for the DimensionMaster rider repair."""
import struct
import unittest

import repair_dimensionmaster_rider_scale as repair

wm = repair.wm
ROOT_HASH = 123
CLIP = "pc_sp_m_00_sk_ride_horse_idle_normal_1"


def fixture(scales=(repair.SOURCE_SCALE, repair.SOURCE_SCALE)):
    header = wm.ANIMATION_HEADER.pack(b"WANM", 2, 10.0, 30.0, 3, 0, 0, bytes(7))
    root = wm.ANIMATION_CHANNEL.pack(ROOT_HASH, 0, 0, 0, 0, 2, 0, -1, 0)
    child = wm.ANIMATION_CHANNEL.pack(124, 0, 0, 0, 0, 1, 32, -1, 0)
    keys = (wm.VECTOR_KEY.pack(0.0, *scales[0]) +
            wm.VECTOR_KEY.pack(10.0, *scales[1]) +
            wm.VECTOR_KEY.pack(7.0, 3.0, 4.0, 5.0))
    return header + root + child + keys + b"preserved trailer"


class RiderScaleRepairTests(unittest.TestCase):
    def test_only_armature_scale_xyz_changes_and_repeated_repair_is_identical(self):
        before = fixture()
        after, count, already = repair.repair_keys(before, [(0, CLIP)], ROOT_HASH)
        self.assertEqual(count, 2)
        self.assertFalse(already)
        key_at = wm.ANIMATION_HEADER.size + 2 * wm.ANIMATION_CHANNEL.size
        self.assertEqual(before[:key_at], after[:key_at])
        self.assertEqual(before[key_at + 32:], after[key_at + 32:])
        for offset in (key_at, key_at + 16):
            self.assertEqual(before[offset:offset + 4], after[offset:offset + 4])
            self.assertEqual(struct.unpack_from("<3f", after, offset + 4), repair.TARGET_SCALE)
        repeated, repeated_count, already = repair.repair_keys(after, [(0, CLIP)], ROOT_HASH)
        self.assertEqual(repeated, after)
        self.assertEqual(repeated_count, 2)
        self.assertTrue(already)

    def test_authored_scale_change_is_rejected_before_mutation(self):
        before = fixture((repair.SOURCE_SCALE, (110.0, 110.0, 110.0)))
        source = bytearray(before)
        with self.assertRaisesRegex(ValueError, "Unexpected or animated"):
            repair.repair_keys(source, [(0, CLIP)], ROOT_HASH)
        self.assertEqual(source, before)

    def test_nonfinite_and_unrecognized_import_scales_are_rejected(self):
        for scale in ((float("nan"), 100.0, 100.0), (50.0, 50.0, 50.0)):
            with self.subTest(scale=scale), self.assertRaises(ValueError):
                repair.repair_keys(fixture((scale, scale)), [(0, CLIP)], ROOT_HASH)

    def test_nonriding_and_missing_armature_are_rejected(self):
        with self.assertRaisesRegex(ValueError, "Only DimensionMaster riding"):
            repair.repair_keys(fixture(), [(0, "ordinary_skill")], ROOT_HASH)
        with self.assertRaisesRegex(ValueError, "missing or duplicate"):
            repair.repair_keys(fixture(), [(0, CLIP)], 999)


if __name__ == "__main__":
    unittest.main()
