import copy
import json
import shutil
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
PUBLISHER = (
    REPOSITORY_ROOT
    / "Tools"
    / "RenderingPipeline"
    / "Publish-RenderingProfiles.ps1"
)
AUTHORED = (
    REPOSITORY_ROOT
    / "Data"
    / "Rendering"
    / "Authored"
    / "RenderingProfiles.json"
)
RUNTIME = (
    REPOSITORY_ROOT
    / "Client"
    / "Bin"
    / "DataFiles"
    / "Rendering"
    / "RenderingProfiles.runtime.json"
)
POWERSHELL = shutil.which("powershell.exe") or shutil.which("powershell")


def to_float32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", value))[0]


def previous_positive_float32(value: float) -> float:
    bits = struct.unpack("<I", struct.pack("<f", value))[0]
    if bits == 0:
        raise ValueError("zero has no positive predecessor in this test")
    return struct.unpack("<f", struct.pack("<I", bits - 1))[0]


def next_positive_float32(value: float) -> float:
    bits = struct.unpack("<I", struct.pack("<f", value))[0]
    return struct.unpack("<f", struct.pack("<I", bits + 1))[0]


class RenderingProfilePublisherTest(unittest.TestCase):
    def setUp(self) -> None:
        if POWERSHELL is None:
            self.skipTest("PowerShell is required by the rendering publisher")
        self.source_document = json.loads(AUTHORED.read_text(encoding="utf-8"))

    def run_publisher(
        self,
        source: Path,
        mode: str,
        destination: Path | None = None,
    ) -> subprocess.CompletedProcess[str]:
        command = [
            POWERSHELL,
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-File",
            str(PUBLISHER),
            "-Mode",
            mode,
            "-SourcePath",
            str(source),
        ]
        if destination is not None:
            command.extend(["-DestinationPath", str(destination)])
        return subprocess.run(
            command,
            cwd=REPOSITORY_ROOT,
            check=False,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
        )

    def write_document(self, path: Path, document: dict) -> None:
        path.write_text(
            json.dumps(document, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8",
            newline="\n",
        )

    def test_checked_in_scatter_one_round_trips_to_runtime(self) -> None:
        self.assertGreaterEqual(self.source_document["revision"], 12)
        self.assertEqual(
            1,
            self.source_document["globalQuality"]["bloomScatter"],
        )
        self.assertEqual(
            self.source_document,
            json.loads(RUNTIME.read_text(encoding="utf-8")),
            "checked-in runtime profile must match the authored document",
        )

        with tempfile.TemporaryDirectory() as temporary_directory:
            generated = Path(temporary_directory) / "RenderingProfiles.runtime.json"
            result = self.run_publisher(AUTHORED, "Publish", generated)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            generated_document = json.loads(generated.read_text(encoding="utf-8"))

        self.assertEqual(self.source_document, generated_document)
        self.assertEqual(
            1,
            generated_document["globalQuality"]["bloomScatter"],
        )

    def test_environment_round_trip_and_invalid_input_preserves_runtime(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "source.json"
            destination = Path(temporary_directory) / "runtime.json"
            document = copy.deepcopy(self.source_document)
            environment = {
                "cubeTexture": "Map/Lighting/CharacterSelect/lv_lut_valhatrond_04_hdr01.rgbm.cube.dds",
                "color": [1, 1, 1, 0],
                "rotationIntensity": [0, 1, 1, 0],
            }
            document["profiles"][0]["environment"] = environment
            self.write_document(source, document)
            result = self.run_publisher(source, "Publish", destination)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            original = destination.read_bytes()
            self.assertEqual(environment, json.loads(original)["profiles"][0]["environment"])
            for field, value in (
                ("cubeTexture", "../outside.dds"),
                ("cubeTexture", "C:/outside.dds"),
                ("cubeTexture", "Map\\outside.dds"),
                ("cubeTexture", "Map/./outside.dds"),
                ("cubeTexture", "Map/outside.tga"),
                ("color", [1, -1, 1, 0]),
                ("rotationIntensity", [0, 0, 1, 0]),
                ("rotationIntensity", [0, 1, 1, 1]),
                ("rotationIntensity", [0, 1, "1", 0]),
            ):
                with self.subTest(field=field, value=value):
                    invalid = copy.deepcopy(document)
                    invalid["profiles"][0]["environment"][field] = value
                    self.write_document(source, invalid)
                    result = self.run_publisher(source, "Publish", destination)
                    self.assertNotEqual(0, result.returncode)
                    self.assertEqual(original, destination.read_bytes())

    def test_workbench_float32_boundary_values_are_accepted(self) -> None:
        boundary_cases = (
            (to_float32(0.0312), to_float32(0.0156)),
            (to_float32(0.333), to_float32(0.0833)),
        )
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "RenderingProfiles.json"
            for edge_threshold, edge_threshold_min in boundary_cases:
                document = copy.deepcopy(self.source_document)
                global_quality = document["globalQuality"]
                global_quality["fxaaEdgeThreshold"] = edge_threshold
                global_quality["fxaaEdgeThresholdMin"] = edge_threshold_min
                self.write_document(source, document)
                result = self.run_publisher(source, "Validate")
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_value_below_runtime_float32_boundary_is_rejected(self) -> None:
        document = copy.deepcopy(self.source_document)
        runtime_minimum = to_float32(0.0312)
        document["globalQuality"]["fxaaEdgeThreshold"] = (
            previous_positive_float32(runtime_minimum)
        )
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "RenderingProfiles.json"
            self.write_document(source, document)
            result = self.run_publisher(source, "Validate")

        self.assertNotEqual(0, result.returncode)
        self.assertIn(
            "globalQuality.fxaaEdgeThreshold",
            result.stdout + result.stderr,
        )

    def test_nine_digit_float32_spelling_round_trips_without_rewriting_values(self) -> None:
        document = copy.deepcopy(self.source_document)
        kouku = next(p for p in document["profiles"] if p["profileId"] == "scene.kakulsaydon.g1.base.v1")
        kouku["exposureMultiplier"] = float(format(to_float32(0.1), ".9g"))
        kouku["fog"]["heightFalloff"] = float(format(to_float32(0.0001), ".9g"))
        kouku["environmentRegions"][0]["fog"]["heightFalloff"] = kouku["fog"]["heightFalloff"]
        document["globalQuality"]["fxaaEdgeThreshold"] = float(format(to_float32(0.0312), ".9g"))
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "source.json"
            destination = Path(temporary_directory) / "runtime.json"
            self.write_document(source, document)
            source_bytes = source.read_bytes()
            for mode in ("Validate", "Publish"):
                result = self.run_publisher(source, mode, destination)
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            self.assertEqual(source_bytes, source.read_bytes())
            self.assertEqual(document, json.loads(destination.read_text(encoding="utf-8")))

    def test_invalid_float32_values_preserve_published_runtime(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "source.json"
            destination = Path(temporary_directory) / "runtime.json"
            self.write_document(source, self.source_document)
            result = self.run_publisher(source, "Publish", destination)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            published = destination.read_bytes()
            cases = (
                ("exposure", previous_positive_float32(0.1)),
                ("exposure", next_positive_float32(4.0)),
                ("height", previous_positive_float32(0.0001)),
                ("exposure", 1e100),
                ("exposure", float("nan")),
                ("exposure", float("inf")),
                ("exposure", "0.1"),
                ("exposure", True),
                ("exposure", None),
            )
            for field, value in cases:
                with self.subTest(field=field, value=value):
                    invalid = copy.deepcopy(self.source_document)
                    profile = invalid["profiles"][0]
                    if field == "height":
                        profile["fog"]["heightFalloff"] = value
                    else:
                        profile["exposureMultiplier"] = value
                    self.write_document(source, invalid)
                    result = self.run_publisher(source, "Publish", destination)
                    self.assertNotEqual(0, result.returncode)
                    self.assertEqual(published, destination.read_bytes())

    def test_shadow_planes_that_collapse_in_float32_are_rejected(self) -> None:
        document = copy.deepcopy(self.source_document)
        document["profiles"][0]["shadow"]["near"] = 1.000000001
        document["profiles"][0]["shadow"]["far"] = 1.000000002
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "source.json"
            self.write_document(source, document)
            result = self.run_publisher(source, "Validate")
        self.assertNotEqual(0, result.returncode)
        self.assertIn("shadow.far", result.stdout + result.stderr)

    def test_vector_raw_double_limits_are_not_relaxed_by_float32_rounding(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "source.json"
            for group, field, index, value in (
                ("light", "diffuse", 0, 64.0000001),
                ("shadow", "focus", 0, 100000.001),
                ("environment", "rotationIntensity", 2, 64.0000001),
            ):
                with self.subTest(field=field):
                    document = copy.deepcopy(self.source_document)
                    profile = next(profile for profile in document["profiles"] if group in profile)
                    profile[group][field][index] = value
                    self.write_document(source, document)
                    result = self.run_publisher(source, "Validate")
                    self.assertNotEqual(0, result.returncode)
                    self.assertIn(field, result.stdout + result.stderr)

    def test_level_quality_override_round_trips_and_invalid_publish_preserves_runtime(self) -> None:
        document = copy.deepcopy(self.source_document)
        kouku = next(p for p in document["profiles"] if p["profileId"] == "scene.kakulsaydon.g1.base.v1")
        kouku["qualityOverride"] = copy.deepcopy(document["globalQuality"])
        kouku["qualityOverride"]["exposure"] = 2.0
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "RenderingProfiles.json"
            destination = Path(temporary_directory) / "RenderingProfiles.runtime.json"
            self.write_document(source, document)
            result = self.run_publisher(source, "Publish", destination)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            published = destination.read_bytes()
            self.assertEqual(document, json.loads(published))
            kouku["qualityOverride"]["exposure"] = 0.0
            self.write_document(source, document)
            result = self.run_publisher(source, "Publish", destination)
            self.assertNotEqual(0, result.returncode)
            self.assertEqual(published, destination.read_bytes())

    def test_display_name_is_optional_and_korean_name_round_trips(self) -> None:
        document = copy.deepcopy(self.source_document)
        for profile in document["profiles"]:
            profile.pop("displayName", None)
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "RenderingProfiles.json"
            destination = Path(temporary_directory) / "RenderingProfiles.runtime.json"
            self.write_document(source, document)
            result = self.run_publisher(source, "Validate")
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            dark = next(p for p in document["profiles"] if p["profileId"] == "scene.kakulsaydon.find-true-dark.v1")
            dark["displayName"] = "씬프로필_암전"
            self.write_document(source, document)
            result = self.run_publisher(source, "Publish", destination)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            self.assertEqual(document, json.loads(destination.read_bytes()))

    def test_invalid_display_name_preserves_previous_runtime(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "RenderingProfiles.json"
            destination = Path(temporary_directory) / "RenderingProfiles.runtime.json"
            self.write_document(source, self.source_document)
            result = self.run_publisher(source, "Publish", destination)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            previous = destination.read_bytes()
            for name in ("", 42, "가" * 86, "bad\u0000name"):
                with self.subTest(name=name):
                    document = copy.deepcopy(self.source_document)
                    document["profiles"][0]["displayName"] = name
                    self.write_document(source, document)
                    result = self.run_publisher(source, "Publish", destination)
                    self.assertNotEqual(0, result.returncode)
                    self.assertIn("displayName", result.stdout + result.stderr)
                    self.assertEqual(previous, destination.read_bytes())

    def test_map_light_multiplier_is_optional_and_round_trips(self) -> None:
        document = copy.deepcopy(self.source_document)
        for profile in document["profiles"]:
            profile.pop("mapLightIntensityMultiplier", None)
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "RenderingProfiles.json"
            destination = Path(temporary_directory) / "RenderingProfiles.runtime.json"
            self.write_document(source, document)
            result = self.run_publisher(source, "Validate")
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            dark = next(p for p in document["profiles"] if p["profileId"] == "scene.kakulsaydon.find-true-dark.v1")
            for multiplier in (0, 0.05, 1, 4):
                with self.subTest(multiplier=multiplier):
                    dark["mapLightIntensityMultiplier"] = multiplier
                    self.write_document(source, document)
                    result = self.run_publisher(source, "Publish", destination)
                    self.assertEqual(0, result.returncode, result.stdout + result.stderr)
                    self.assertEqual(document, json.loads(destination.read_bytes()))

    def test_invalid_map_light_multiplier_preserves_previous_runtime(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            source = Path(temporary_directory) / "RenderingProfiles.json"
            destination = Path(temporary_directory) / "RenderingProfiles.runtime.json"
            self.write_document(source, self.source_document)
            result = self.run_publisher(source, "Publish", destination)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            previous = destination.read_bytes()
            for multiplier in (-0.01, 4.01, True, "0.05", None):
                with self.subTest(multiplier=multiplier):
                    document = copy.deepcopy(self.source_document)
                    document["profiles"][0]["mapLightIntensityMultiplier"] = multiplier
                    self.write_document(source, document)
                    result = self.run_publisher(source, "Publish", destination)
                    self.assertNotEqual(0, result.returncode)
                    self.assertIn("mapLightIntensityMultiplier", result.stdout + result.stderr)
                    self.assertEqual(previous, destination.read_bytes())


if __name__ == "__main__":
    unittest.main()
