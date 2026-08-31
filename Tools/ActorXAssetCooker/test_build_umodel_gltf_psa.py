from __future__ import annotations

import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest


SCRIPT = Path(__file__).with_name("build_umodel_gltf_psa.py")
CHUNK_HEADER = struct.Struct("<20siii")
ANIM_INFO = struct.Struct("<64s64s4i3f3i")
ANIM_KEY = struct.Struct("<3f4ff")


def _fixed_name(value: str, width: int) -> bytes:
    encoded = value.encode("windows-1252")
    if len(encoded) >= width:
        raise ValueError(f"fixture name is too long: {value}")
    return encoded + (b"\0" * (width - len(encoded)))


def _chunk(name: str, record_size: int, record_count: int, payload: bytes) -> bytes:
    return CHUNK_HEADER.pack(_fixed_name(name, 20), 0, record_size, record_count) + payload


def _bone_record(name: str, parent: int) -> bytes:
    record = bytearray(120)
    record[:64] = _fixed_name(name, 64)
    struct.pack_into("<i", record, 72, parent)
    return bytes(record)


def _animation_key(frame: int) -> bytes:
    return ANIM_KEY.pack(
        float(frame * 10),
        float(frame * 20),
        float(frame * 30),
        0.0,
        0.0,
        0.0,
        1.0,
        0.0,
    )


def _write_psa(
    path: Path,
    *,
    bone_names: tuple[str, ...] = ("root",),
    frame_count: int = 2,
    key_payload_count: int | None = None,
    key_header_count: int | None = None,
) -> None:
    bone_payload = b"".join(
        _bone_record(name, -1 if index == 0 else index - 1)
        for index, name in enumerate(bone_names)
    )
    info_payload = ANIM_INFO.pack(
        _fixed_name("open", 64),
        _fixed_name("fixture", 64),
        len(bone_names),
        0,
        0,
        0,
        0.0,
        float(frame_count),
        30.0,
        0,
        0,
        frame_count,
    )
    expected_keys = frame_count * len(bone_names)
    payload_count = expected_keys if key_payload_count is None else key_payload_count
    header_count = payload_count if key_header_count is None else key_header_count
    key_payload = b"".join(_animation_key(index) for index in range(payload_count))
    path.write_bytes(
        _chunk("BONENAMES", 120, len(bone_names), bone_payload)
        + _chunk("ANIMINFO", ANIM_INFO.size, 1, info_payload)
        + _chunk("ANIMKEYS", ANIM_KEY.size, header_count, key_payload)
    )


def _write_gltf(
    path: Path,
    *,
    joint_names: tuple[str, ...] = ("root",),
    animations: list[dict] | None = None,
) -> None:
    buffer_path = path.with_suffix(".bin")
    buffer_path.write_bytes(b"mesh")
    document = {
        "asset": {"version": "2.0"},
        "buffers": [{"uri": buffer_path.name, "byteLength": 4}],
        "nodes": [{"name": name} for name in joint_names],
        "skins": [{"joints": list(range(len(joint_names)))}],
    }
    if animations is not None:
        document["animations"] = animations
    path.write_text(json.dumps(document), encoding="utf-8")


def _write_scalable_gltf(path: Path) -> None:
    """A minimal but complete skinned document: mesh, bind pose and a joint."""
    buffer_path = path.with_suffix(".bin")
    inverse_bind = [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        4.0, 5.0, 6.0, 1.0,
    ]
    positions = [1.0, 2.0, 3.0, -4.0, -5.0, -6.0]
    payload = struct.pack("<16f", *inverse_bind) + struct.pack("<6f", *positions)
    buffer_path.write_bytes(payload)
    document = {
        "asset": {"version": "2.0"},
        "buffers": [{"uri": buffer_path.name, "byteLength": len(payload)}],
        "bufferViews": [
            {"buffer": 0, "byteOffset": 0, "byteLength": 64},
            {"buffer": 0, "byteOffset": 64, "byteLength": 24},
        ],
        "accessors": [
            {"bufferView": 0, "componentType": 5126, "count": 1, "type": "MAT4"},
            {
                "bufferView": 1,
                "componentType": 5126,
                "count": 2,
                "type": "VEC3",
                "min": [-4.0, -5.0, -6.0],
                "max": [1.0, 2.0, 3.0],
            },
        ],
        "meshes": [{"primitives": [{"attributes": {"POSITION": 1}}]}],
        "nodes": [{"name": "root", "translation": [1.0, 2.0, 3.0]}],
        "skins": [{"joints": [0], "inverseBindMatrices": 0}],
    }
    path.write_text(json.dumps(document), encoding="utf-8")


def _read_floats(document: dict, payload: bytes, accessor_index: int, width: int):
    accessor = document["accessors"][accessor_index]
    view = document["bufferViews"][accessor["bufferView"]]
    start = view.get("byteOffset", 0) + accessor.get("byteOffset", 0)
    return [
        struct.unpack_from("<%df" % width, payload, start + index * width * 4)
        for index in range(accessor["count"])
    ]


def _read_animation_translations(gltf_path: Path, bin_path: Path):
    document = json.loads(gltf_path.read_text(encoding="utf-8"))
    payload = bin_path.read_bytes()
    animation = document["animations"][0]
    for channel in animation["channels"]:
        if channel["target"]["path"] != "translation":
            continue
        sampler = animation["samplers"][channel["sampler"]]
        return _read_floats(document, payload, sampler["output"], 3)
    raise AssertionError("staged glTF has no translation channel")


def _sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


class UmodelGltfPsaCookerTests(unittest.TestCase):
    def setUp(self) -> None:
        self._temporary_directory = tempfile.TemporaryDirectory()
        self.root = Path(self._temporary_directory.name)
        self.gltf = self.root / "mesh.gltf"
        self.psa = self.root / "animation.psa"
        self.output_gltf = self.root / "staged.gltf"
        self.output_bin = self.root / "staged.bin"
        self.report = self.root / "stage.report.json"

    def tearDown(self) -> None:
        self._temporary_directory.cleanup()

    def run_cooker(self, *extra_arguments: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [
                sys.executable,
                str(SCRIPT),
                "--gltf",
                str(self.gltf),
                "--psa",
                str(self.psa),
                "--output-gltf",
                str(self.output_gltf),
                "--output-bin",
                str(self.output_bin),
                "--report",
                str(self.report),
                *extra_arguments,
            ],
            cwd=SCRIPT.parent,
            capture_output=True,
            text=True,
            check=False,
        )

    def assert_failed_without_outputs(
        self, result: subprocess.CompletedProcess[str], expected_error: str
    ) -> None:
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertIn("UMODEL_GLTF_PSA_ERROR", result.stderr)
        self.assertIn(expected_error, result.stderr)
        self.assertFalse(self.output_gltf.exists())
        self.assertFalse(self.output_bin.exists())
        self.assertFalse(self.report.exists())

    def test_injects_psa_clip_and_writes_receipt(self) -> None:
        _write_gltf(self.gltf)
        _write_psa(self.psa)

        result = self.run_cooker()

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("UMODEL_GLTF_PSA_OK joints=1 clips=1", result.stdout)
        staged = json.loads(self.output_gltf.read_text(encoding="utf-8"))
        self.assertEqual(staged["buffers"][0]["uri"], self.output_bin.name)
        self.assertEqual(staged["buffers"][0]["byteLength"], self.output_bin.stat().st_size)
        self.assertGreater(self.output_bin.stat().st_size, 4)
        self.assertEqual(len(staged["animations"]), 1)
        animation = staged["animations"][0]
        self.assertEqual(animation["name"], "open")
        self.assertEqual(
            [channel["target"]["path"] for channel in animation["channels"]],
            ["translation", "rotation"],
        )
        receipt = json.loads(self.report.read_text(encoding="utf-8"))
        self.assertEqual(receipt["schema"], "lostark.umodel-gltf-psa-stage")
        self.assertEqual(receipt["jointCount"], 1)
        self.assertEqual(receipt["clips"][0]["frameCount"], 2)
        self.assertAlmostEqual(receipt["clips"][0]["durationSeconds"], 1.0 / 30.0)
        self.assertEqual(receipt["clips"][0]["channelCount"], 2)
        self.assertEqual(receipt["output"]["gltfSha256"], _sha256(self.output_gltf))
        self.assertEqual(receipt["output"]["bufferSha256"], _sha256(self.output_bin))

    def test_scale_moves_mesh_bind_pose_and_translation_keys_together(self) -> None:
        # The converter's own --scale only multiplies mesh vertices, which
        # desynchronises a skinned asset from its skeleton. Staging has to move
        # positions, inverse bind translations, node translations and PSA
        # translation keys by the identical factor.
        _write_scalable_gltf(self.gltf)
        _write_psa(self.psa)
        self.assertEqual(self.run_cooker().returncode, 0)
        unscaled_translations = _read_animation_translations(
            self.output_gltf, self.output_bin
        )

        self.output_gltf.unlink()
        self.output_bin.unlink()
        self.report.unlink()
        _write_scalable_gltf(self.gltf)
        result = self.run_cooker("--scale", "100")

        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("scale=100.0", result.stdout)
        staged = json.loads(self.output_gltf.read_text(encoding="utf-8"))
        payload = self.output_bin.read_bytes()

        positions = _read_floats(staged, payload, 1, 3)
        self.assertEqual(positions[0], (100.0, 200.0, 300.0))
        self.assertEqual(positions[1], (-400.0, -500.0, -600.0))
        self.assertEqual(staged["accessors"][1]["min"], [-400.0, -500.0, -600.0])
        self.assertEqual(staged["accessors"][1]["max"], [100.0, 200.0, 300.0])

        inverse_bind = _read_floats(staged, payload, 0, 16)[0]
        self.assertEqual(inverse_bind[12:15], (400.0, 500.0, 600.0))
        # Only the translation column moves; the basis stays untouched.
        self.assertEqual(inverse_bind[0:12], (
            1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0))

        self.assertEqual(staged["nodes"][0]["translation"], [100.0, 200.0, 300.0])

        scaled_translations = _read_animation_translations(
            self.output_gltf, self.output_bin
        )
        self.assertEqual(len(scaled_translations), len(unscaled_translations))
        for scaled, unscaled in zip(scaled_translations, unscaled_translations):
            for axis in range(3):
                self.assertAlmostEqual(scaled[axis], unscaled[axis] * 100.0, places=3)

        receipt = json.loads(self.report.read_text(encoding="utf-8"))
        self.assertEqual(receipt["scale"], 100.0)
        self.assertEqual(receipt["scaled"]["positionAccessors"], 1)
        self.assertEqual(receipt["scaled"]["inverseBindMatrices"], 1)
        self.assertEqual(receipt["scaled"]["nodeTranslations"], 1)

    def test_scale_rejects_a_non_positive_factor(self) -> None:
        _write_scalable_gltf(self.gltf)
        _write_psa(self.psa)

        result = self.run_cooker("--scale", "0")

        self.assert_failed_without_outputs(
            result, "--scale must be a finite positive number"
        )

    def test_rejects_joint_name_mismatch(self) -> None:
        _write_gltf(self.gltf, joint_names=("different_root",))
        _write_psa(self.psa)

        self.assert_failed_without_outputs(
            self.run_cooker(), "glTF joint order/names differ from PSA BONENAMES"
        )

    def test_rejects_animation_key_count_mismatch(self) -> None:
        _write_gltf(self.gltf)
        _write_psa(self.psa, key_payload_count=1)

        self.assert_failed_without_outputs(
            self.run_cooker(), "PSA ANIMKEYS count mismatch: expected 2, got 1"
        )

    def test_rejects_truncated_animation_key_chunk(self) -> None:
        _write_gltf(self.gltf)
        _write_psa(self.psa, key_payload_count=1, key_header_count=2)

        self.assert_failed_without_outputs(self.run_cooker(), "Truncated PSA ANIMKEYS")

    def test_refuses_source_gltf_with_existing_animations(self) -> None:
        _write_gltf(
            self.gltf,
            animations=[{"name": "existing", "samplers": [], "channels": []}],
        )
        _write_psa(self.psa)

        self.assert_failed_without_outputs(
            self.run_cooker(), "Source glTF already contains animations"
        )

    def test_existing_outputs_are_not_overwritten_without_opt_in(self) -> None:
        _write_gltf(self.gltf)
        _write_psa(self.psa)
        first = self.run_cooker()
        self.assertEqual(first.returncode, 0, first.stderr)
        original_hashes = (
            _sha256(self.output_gltf),
            _sha256(self.output_bin),
            _sha256(self.report),
        )

        second = self.run_cooker()

        self.assertNotEqual(second.returncode, 0, second.stdout)
        self.assertIn("Output already exists", second.stderr)
        self.assertEqual(
            original_hashes,
            (
                _sha256(self.output_gltf),
                _sha256(self.output_bin),
                _sha256(self.report),
            ),
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
