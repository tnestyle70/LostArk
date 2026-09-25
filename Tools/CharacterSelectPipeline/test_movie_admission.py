"""Regressions for resource and phase-clock failures found by native playback."""
from pathlib import Path
import tempfile
import unittest

import numpy as np

from install_guardian_selection import validate_texture_container
from project_guardian_selection import admit_clock_keys


class MovieAdmissionContracts(unittest.TestCase):
    def test_oversized_slow_motion_clock_preserves_time_and_endpoints(self):
        times = np.arange(4478, dtype=float) * 8.
        source = times * .24 + 80. * np.sin(times / 5000.)
        keys = [dict(timeMs=float(t), sourceMs=float(s)) for t, s in zip(times, source)]
        reduced = admit_clock_keys(keys)
        self.assertLessEqual(len(reduced), 4096)
        self.assertEqual(reduced[0], keys[0])
        self.assertEqual(reduced[-1], keys[-1])
        restored = np.interp(times, [k['timeMs'] for k in reduced],
                             [k['sourceMs'] for k in reduced])
        self.assertLessEqual(np.max(np.abs(restored-source)), .001)

    def test_small_clock_keeps_authored_keys(self):
        keys = [dict(timeMs=0., sourceMs=0.), dict(timeMs=300., sourceMs=20.)]
        self.assertIs(admit_clock_keys(keys), keys)

    def test_renamed_tga_is_rejected_before_install(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'normal.dds'
            path.write_bytes(b'\x00\x00\x0a\x00' + bytes(140))
            with self.assertRaisesRegex(ValueError, 'different image container'):
                validate_texture_container(path, 'Map/normal.dds')
            path.write_bytes(b'DDS ' + bytes(140))
            validate_texture_container(path, 'Map/normal.dds')


if __name__ == '__main__':
    unittest.main()
