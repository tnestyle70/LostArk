from __future__ import annotations

import json
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).with_name("retime_wmodel_ticks.py")

FILE_HEADER = struct.Struct("<4sHHII")
MODEL_HEADER = struct.Struct("<4sIII4I")
SECTION_DESC = struct.Struct("<IIQQ40s")
ANIMATION_HEADER = struct.Struct("<4sIffIIB7s")
ANIMATION_CHANNEL = struct.Struct("<QIIIIIIiI")
VECTOR_KEY = struct.Struct("<4f")
QUATERNION_KEY = struct.Struct("<5f")


def _fixed(value: str, width: int) -> bytes:
    encoded = value.encode("ascii")
    if len(encoded) >= width:
        raise ValueError(value)
    return encoded + b"\0" * (width - len(encoded))


def _animation_section(
    name: str,
    duration: float,
    ticks_per_second: float,
    times: list[float],
    *,
    event_count: int = 0,
) -> bytes:
    """One WANM section: a single bone with position and rotation keys."""
    position_keys = b"".join(
        VECTOR_KEY.pack(time, 1.0, 2.0, 3.0) for time in times
    )
    rotation_keys = b"".join(
        QUATERNION_KEY.pack(time, 0.0, 0.0, 0.0, 1.0) for time in times
    )
    channel = ANIMATION_CHANNEL.pack(
        0x1234,
        len(times), 0,
        len(times), len(position_keys),
        0, len(position_keys) + len(rotation_keys),
        0, 0,
    )
    header = ANIMATION_HEADER.pack(
        b"WANM", 1, duration, ticks_per_second, 0, event_count, 0, b"\0" * 7
    )
    body = header + channel + position_keys + rotation_keys + b"\0" * (event_count * 32) + b"\0" * 8
    return FILE_HEADER.pack(b"WINT", 1, 0, 0, len(body)) + body


def _write_wmodel(path: Path, sections: list[tuple[str, bytes]]) -> None:
    descriptors = b""
    payload = b""
    table_size = len(sections) * SECTION_DESC.size
    for name, section in sections:
        offset = MODEL_HEADER.size + table_size + len(payload)
        descriptors += SECTION_DESC.pack(4, 0, offset, len(section), _fixed(name, 40))
        payload += section
    model = MODEL_HEADER.pack(b"WMOD", len(sections), len(sections), 3, 0, 0, 0, 0)
    content = model + descriptors + payload
    path.write_bytes(FILE_HEADER.pack(b"WINT", 1, 0, 0, len(content)) + content)


def _read_clips(path: Path) -> list[dict]:
    data = path.read_bytes()
    content = FILE_HEADER.size
    model = MODEL_HEADER.unpack_from(data, content)
    table = content + MODEL_HEADER.size
    clips = []
    for index in range(model[1]):
        type_id, _, offset, size, raw_name = SECTION_DESC.unpack_from(
            data, table + index * SECTION_DESC.size
        )
        if type_id != 4:
            continue
        base = content + offset + FILE_HEADER.size
        header = ANIMATION_HEADER.unpack_from(data, base)
        row = ANIMATION_CHANNEL.unpack_from(data, base + ANIMATION_HEADER.size)
        keys_at = base + ANIMATION_HEADER.size + ANIMATION_CHANNEL.size
        times = [
            VECTOR_KEY.unpack_from(data, keys_at + row[2] + key * VECTOR_KEY.size)[0]
            for key in range(row[1])
        ]
        rotation_times = [
            QUATERNION_KEY.unpack_from(
                data, keys_at + row[4] + key * QUATERNION_KEY.size
            )[0]
            for key in range(row[3])
        ]
        clips.append({
            "name": raw_name.split(b"\0")[0].decode("ascii"),
            "duration": header[2],
            "ticksPerSecond": header[3],
            "positionTimes": times,
            "rotationTimes": rotation_times,
        })
    return clips


class RetimeWModelTicksTests(unittest.TestCase):
    def setUp(self) -> None:
        self._temporary_directory = tempfile.TemporaryDirectory()
        self.root = Path(self._temporary_directory.name)
        self.wmodel = self.root / "asset.wmodel"
        self.report = self.root / "retime.json"

    def tearDown(self) -> None:
        self._temporary_directory.cleanup()

    def run_tool(self, *arguments: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [sys.executable, str(SCRIPT), "--wmodel", str(self.wmodel), *arguments],
            capture_output=True,
            text=True,
            check=False,
        )

    def test_retime_preserves_wall_clock_and_scales_every_key(self) -> None:
        _write_wmodel(self.wmodel, [
            ("go_off", _animation_section("go_off", 2000.0, 1000.0, [0.0, 1000.0, 2000.0])),
        ])

        result = self.run_tool(
            "--ticks-per-second", "30",
            "--expect-ticks-per-second", "1000",
            "--report", str(self.report),
        )

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("WMODEL_TICK_RETIME_OK clips=1", result.stdout)
        clip = _read_clips(self.wmodel)[0]
        self.assertAlmostEqual(clip["ticksPerSecond"], 30.0, places=5)
        self.assertAlmostEqual(clip["duration"], 60.0, places=4)
        # 2000/1000 s and 60/30 s describe the same clip.
        self.assertAlmostEqual(clip["duration"] / clip["ticksPerSecond"], 2.0, places=5)
        self.assertEqual(len(clip["positionTimes"]), 3)
        for actual, expected in zip(clip["positionTimes"], [0.0, 30.0, 60.0]):
            self.assertAlmostEqual(actual, expected, places=4)
        for actual, expected in zip(clip["rotationTimes"], [0.0, 30.0, 60.0]):
            self.assertAlmostEqual(actual, expected, places=4)
        receipt = json.loads(self.report.read_text(encoding="utf-8"))
        self.assertEqual(receipt["schema"], "lostark.wmodel-tick-retime")
        self.assertEqual(receipt["clips"][0]["scaledKeyTimes"], 6)
        self.assertAlmostEqual(receipt["clips"][0]["durationSeconds"], 2.0, places=5)

    def test_every_clip_in_the_package_is_retimed(self) -> None:
        _write_wmodel(self.wmodel, [
            ("go_off", _animation_section("go_off", 2000.0, 1000.0, [0.0, 2000.0])),
            ("go_on", _animation_section("go_on", 1000.0, 1000.0, [0.0, 1000.0])),
        ])

        self.assertEqual(self.run_tool("--ticks-per-second", "30").returncode, 0)

        clips = _read_clips(self.wmodel)
        self.assertEqual(len(clips), 2)
        for clip, seconds in zip(clips, [2.0, 1.0]):
            self.assertAlmostEqual(clip["ticksPerSecond"], 30.0, places=5)
            self.assertAlmostEqual(
                clip["duration"] / clip["ticksPerSecond"], seconds, places=5
            )

    def test_unexpected_source_rate_is_refused_without_touching_the_file(self) -> None:
        _write_wmodel(self.wmodel, [
            ("go_off", _animation_section("go_off", 48.0, 24.0, [0.0, 48.0])),
        ])
        before = self.wmodel.read_bytes()

        result = self.run_tool(
            "--ticks-per-second", "30", "--expect-ticks-per-second", "1000"
        )

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("WMODEL_TICK_RETIME_ERROR", result.stderr)
        self.assertIn("expected 1000", result.stderr)
        self.assertEqual(self.wmodel.read_bytes(), before)

    def test_clip_with_events_is_refused(self) -> None:
        _write_wmodel(self.wmodel, [
            ("go_off", _animation_section(
                "go_off", 2000.0, 1000.0, [0.0, 2000.0], event_count=2)),
        ])
        before = self.wmodel.read_bytes()

        result = self.run_tool("--ticks-per-second", "30")

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("carries 2 events", result.stderr)
        self.assertEqual(self.wmodel.read_bytes(), before)

    def test_non_positive_target_rate_is_refused(self) -> None:
        _write_wmodel(self.wmodel, [
            ("go_off", _animation_section("go_off", 2000.0, 1000.0, [0.0, 2000.0])),
        ])
        before = self.wmodel.read_bytes()

        result = self.run_tool("--ticks-per-second", "0")

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("--ticks-per-second must be a finite positive number", result.stderr)
        self.assertEqual(self.wmodel.read_bytes(), before)


if __name__ == "__main__":
    unittest.main(verbosity=2)
