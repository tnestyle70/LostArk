"""Focused light schema, runtime preservation, and map publisher contracts."""

from __future__ import annotations

import copy
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from Tools.RenderingPipeline import light_resources_pipeline as pipeline


def resource(kind: str = "SPOT", ordinal: int = 1) -> dict:
    return {
        "lightResourceId": f"light.runtime.{ordinal}", "displayName": "무대 조명",
        "kind": kind, "defaultAnchorKind": "BOSS",
        "localOffset": [0, 0 if kind == "DIRECTIONAL" else 8, 0],
        "localRotationDegrees": [90, 0, 0],
        "rangeMeters": 0 if kind == "DIRECTIONAL" else 16,
        "falloffExponent": 2, "innerConeDegrees": 10 if kind == "SPOT" else 0,
        "outerConeDegrees": 24 if kind == "SPOT" else 0,
        "color": [1, 0.85, 0.7, 1], "brightness": 6,
    }


def catalog() -> dict:
    return {"schema": "lostark.light-resources", "formatVersion": 1, "revision": 1,
            "nextLightResourceOrdinal": 4,
            "lights": [resource(kind, index + 1) for index, kind in enumerate(("SPOT", "POINT", "DIRECTIONAL"))]}


def map_document(area_id: str) -> dict:
    rows = []
    for r in catalog()["lights"]:
        r = copy.deepcopy(r)
        r["lightId"] = r.pop("lightResourceId").replace("runtime", "map")
        r["position"] = r.pop("localOffset")
        r["rotationDegrees"] = r.pop("localRotationDegrees")
        r.pop("defaultAnchorKind")
        r["groupId"] = "gate1"
        r["enabled"] = True
        rows.append(r)
    return {"schema": "lostark.map-light-presentation", "formatVersion": 2,
            "areaId": area_id, "provenance": "PROJECT_AUTHORED", "nextLightOrdinal": 4,
            "lights": rows}


class LightResourcePipelineTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory(prefix="LostArkLightPublish.")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.source = self.root / "LightResources.json"
        self.destination = self.root / "runtime" / "LightResources.runtime.json"
        self.write(catalog())

    def write(self, document: dict) -> None:
        self.source.write_text(json.dumps(document, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    def publish(self) -> dict:
        return pipeline.publish_resources(self.source, self.destination, publish=True)

    def test_validate_is_read_only_and_three_types_publish_without_changing_source(self) -> None:
        original = self.source.read_bytes()
        result = pipeline.publish_resources(self.source, self.destination, publish=False)
        self.assertEqual(3, result["lightCount"])
        self.assertFalse(self.destination.parent.exists())
        self.publish()
        self.assertEqual(original, self.source.read_bytes())
        self.assertEqual(catalog(), json.loads(self.destination.read_bytes()))

    def test_invalid_authored_fields_preserve_runtime(self) -> None:
        self.publish()
        baseline = self.destination.read_bytes()
        for field, value in (("kind", "AREA"), ("brightness", True), ("rangeMeters", 0),
                             ("outerConeDegrees", 90), ("innerConeDegrees", 25),
                             ("localOffset", [0, 0]), ("color", [1, 1, 1, 2]),
                             ("defaultAnchorKind", "WORLD"), ("displayName", "가" * 86)):
            with self.subTest(field=field, value=value):
                document = catalog()
                document["lights"][0][field] = value
                self.write(document)
                with self.assertRaises(pipeline.LightValidationError):
                    self.publish()
                self.assertEqual(baseline, self.destination.read_bytes())

    def test_unknown_keys_duplicate_ids_and_stale_ordinals_are_rejected(self) -> None:
        for mutation in (
            lambda d: d.update(unknown=1),
            lambda d: d["lights"].append(copy.deepcopy(d["lights"][0])),
            lambda d: d.update(nextLightResourceOrdinal=3),
            lambda d: d["lights"][0].update(lightResourceId="light.runtime.01"),
            lambda d: d["lights"][2].update(localOffset=[1, 0, 0]),
            lambda d: d["lights"][1].update(innerConeDegrees=1),
        ):
            document = catalog()
            mutation(document)
            with self.assertRaises(pipeline.LightValidationError):
                pipeline.validate_resources(document)

    def test_duplicate_json_keys_and_nonfinite_numbers_preserve_runtime(self) -> None:
        self.publish()
        baseline = self.destination.read_bytes()
        original = self.source.read_text(encoding="utf-8")
        for invalid in (original.replace('"revision": 1', '"revision": 1, "revision": 2'),
                        original.replace('"brightness": 6', '"brightness": NaN'),
                        original.replace('"brightness": 6', '"brightness": 1e9999')):
            self.source.write_text(invalid, encoding="utf-8")
            with self.assertRaises(pipeline.LightValidationError):
                self.publish()
            self.assertEqual(baseline, self.destination.read_bytes())

    def test_saved_binary32_boundaries_and_empty_catalog_round_trip(self) -> None:
        document = catalog()
        spot = document["lights"][0]
        spot["rangeMeters"] = spot["falloffExponent"] = float(format(pipeline._float32(0.01), ".9g"))
        spot["innerConeDegrees"] = spot["outerConeDegrees"] = float(format(pipeline._float32(89.9), ".9g"))
        self.write(document)
        self.publish()
        self.assertEqual(document, json.loads(self.destination.read_bytes()))
        document["lights"] = []
        self.write(document)
        self.publish()
        self.assertEqual([], json.loads(self.destination.read_bytes())["lights"])

    def test_failed_replace_and_changed_source_preserve_previous_runtime(self) -> None:
        self.publish()
        baseline = self.destination.read_bytes()
        with patch.object(pipeline.os, "replace", side_effect=OSError("destination locked")):
            with self.assertRaises(OSError):
                self.publish()
        self.assertEqual(baseline, self.destination.read_bytes())
        with patch.object(pipeline.os, "fsync", side_effect=lambda _: self.source.write_text("{}", encoding="utf-8")):
            with self.assertRaisesRegex(pipeline.LightValidationError, "source changed"):
                self.publish()
        self.assertEqual(baseline, self.destination.read_bytes())
        self.assertEqual([self.destination.name], [path.name for path in self.destination.parent.iterdir()])

    def test_powershell_entry_point_publishes_and_reports_failure(self) -> None:
        shell = shutil.which("powershell.exe") or shutil.which("powershell")
        if shell is None:
            self.skipTest("PowerShell is required by the entry point")
        command = [shell, "-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
                   str(Path(pipeline.__file__).with_name("Publish-LightResources.ps1")),
                   "-Mode", "Publish", "-SourcePath", str(self.source), "-DestinationPath", str(self.destination)]
        result = subprocess.run(command, capture_output=True, text=True, errors="replace", timeout=30)
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        previous = self.destination.read_bytes()
        self.source.write_text("{}", encoding="utf-8")
        result = subprocess.run(command, capture_output=True, text=True, errors="replace", timeout=30)
        self.assertNotEqual(0, result.returncode)
        self.assertEqual(previous, self.destination.read_bytes())


class MapLightV2PipelineTest(unittest.TestCase):
    def test_unbaked_receiver_roundtrips_and_unknown_receiver_is_rejected(self) -> None:
        document = map_document("AREA")
        document["lights"][0]["receiver"] = "UNBAKED"
        self.assertEqual(document, pipeline.validate_map_lights_v2(json.loads(json.dumps(document)), "AREA"))
        document["lights"][0]["receiver"] = "OTHER"
        with self.assertRaises(pipeline.LightValidationError):
            pipeline.validate_map_lights_v2(document, "AREA")

    def test_map_v2_empty_and_mixed_types_validate_with_strict_fields(self) -> None:
        document = map_document("AREA")
        pipeline.validate_map_lights_v2(document, "AREA")
        for field, value in (("enabled", 1), ("groupId", ""), ("displayName", "가" * 86),
                             ("position", [0, True, 0]), ("rangeMeters", -1)):
            invalid = copy.deepcopy(document)
            invalid["lights"][0][field] = value
            with self.assertRaises(pipeline.LightValidationError):
                pipeline.validate_map_lights_v2(invalid, "AREA")
        with self.assertRaises(pipeline.LightValidationError):
            pipeline.validate_map_lights_v2(document, "OTHER")
        document["lights"] = []
        pipeline.validate_map_lights_v2(document, "AREA")

    def test_map_publisher_preserves_v1_twenty_two_and_rejects_invalid_v2_atomically(self) -> None:
        from Tools.MapPipeline.test_map_effect_presentation_contract import AREA_ID, Fixture, POWERSHELL
        if POWERSHELL is None:
            self.skipTest("PowerShell is required by the map publisher")
        fixture = Fixture()
        self.addCleanup(fixture.close)
        source = fixture.authoring / f"{AREA_ID}.maplights.json"
        v1 = pipeline.REPOSITORY_ROOT / "Data/Maps/Authoring" / AREA_ID / f"{AREA_ID}.maplights.json"
        original_v1 = v1.read_bytes()
        source.write_bytes(original_v1)
        result = fixture.publish()
        self.assertEqual(0, result.returncode, result.stdout)
        runtime = fixture.runtime_map / source.name
        self.assertEqual(22, len(json.loads(runtime.read_bytes())["lights"]))
        self.assertEqual(json.loads(original_v1), json.loads(runtime.read_bytes()))
        document = map_document(AREA_ID)
        source.write_text(json.dumps(document), encoding="utf-8")
        result = fixture.publish()
        self.assertEqual(0, result.returncode, result.stdout)
        self.assertEqual(document, json.loads(runtime.read_bytes()))
        baseline = fixture.snapshot_runtime()
        raw = json.dumps(document).replace('"enabled": true', '"enabled": true, "enabled": false', 1)
        source.write_text(raw, encoding="utf-8")
        result = fixture.publish()
        self.assertNotEqual(0, result.returncode)
        self.assertIn("Duplicate JSON object key", result.stdout)
        self.assertEqual(baseline, fixture.snapshot_runtime())
        document["lights"][0]["innerConeDegrees"] = 30
        source.write_text(json.dumps(document), encoding="utf-8")
        result = fixture.publish()
        self.assertNotEqual(0, result.returncode)
        self.assertEqual(baseline, fixture.snapshot_runtime())
        self.assertEqual(original_v1, v1.read_bytes())


if __name__ == "__main__":
    unittest.main()
