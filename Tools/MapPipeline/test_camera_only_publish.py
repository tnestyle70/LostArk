"""Exercise the real CameraShots publisher in an isolated temporary project."""
import copy
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
AREA = "LV_LUT_MIDNIGHTC_ED"
SCRIPT = ROOT / "Tools/MapPipeline/Publish-MapAuthoring.ps1"


class CameraOnlyPublish(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="lostark-camera-contract-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.source = self.root / f"Data/Maps/Authoring/{AREA}/{AREA}.camerashots.json"
        self.source.parent.mkdir(parents=True)
        self.runtime = self.root / "Client/Bin/DataFiles/Map"
        self.runtime.mkdir(parents=True)
        self.target = self.runtime / self.source.name
        self.document = json.loads((ROOT / self.source.relative_to(self.root)).read_text(encoding="utf-8"))
        self.catalog = {"areas": [{"id": AREA,
            "sourceCameraShots": self.source.relative_to(self.root).as_posix(),
            "cameraShots": self.target.relative_to(self.root).as_posix()}]}
        for suffix in ("maplights.json", "mapplacements", "mapmaterials.json", "worldsequences.json"):
            (self.runtime / f"{AREA}.{suffix}").write_bytes(b"UNCHANGED DOMAIN\r\n")
        self.target.write_bytes(b"previous runtime bytes")
        self.save()

    def save(self):
        self.source.write_text(json.dumps(self.document), encoding="utf-8")
        (self.root / "Data/Maps/MapCatalog.json").write_text(json.dumps(self.catalog), encoding="utf-8")

    def run_publisher(self, mode, fail=0):
        result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
            "-File", str(SCRIPT), "-ProjectRoot", str(self.root), "-AreaId", AREA,
            "-Scope", "CameraShots", "-Mode", mode, "-FailureAfterPromote", str(fail)],
            capture_output=True, timeout=45)
        return result.returncode, result.stdout + result.stderr

    def test_publish_check_and_other_domains_unchanged(self):
        untouched = {p.name: p.read_bytes() for p in self.runtime.iterdir() if p != self.target}
        for mode in ("Validate", "Publish", "Check"):
            code, output = self.run_publisher(mode)
            self.assertEqual(code, 0, output.decode(errors="replace"))
        self.assertEqual(json.loads(self.target.read_bytes()), self.document)
        self.assertEqual(untouched, {p.name: p.read_bytes() for p in self.runtime.iterdir() if p != self.target})

    def test_failure_after_promotion_rolls_back(self):
        previous = self.target.read_bytes()
        code, _ = self.run_publisher("Publish", 1)
        self.assertNotEqual(code, 0)
        self.assertEqual(self.target.read_bytes(), previous)
        self.assertFalse(list(self.runtime.glob(".map-publish*")))

    def test_invalid_document_preserves_runtime(self):
        pristine = copy.deepcopy(self.document)
        for failure in ("version", "duplicate", "camera-vector"):
            self.document = copy.deepcopy(pristine)
            if failure == "version":
                self.document["formatVersion"] = 999
            elif failure == "duplicate":
                self.document["shots"][1]["shotId"] = self.document["shots"][0]["shotId"]
            else:
                self.document["shots"][0]["lookAt"] = self.document["shots"][0]["eye"]
            self.save()
            with self.subTest(failure=failure):
                code, _ = self.run_publisher("Publish")
                self.assertNotEqual(code, 0)
                self.assertEqual(self.target.read_bytes(), b"previous runtime bytes")

    def test_escaped_catalog_path_rejected(self):
        self.catalog["areas"][0]["cameraShots"] = "../elsewhere.json"
        self.save()
        self.assertNotEqual(self.run_publisher("Publish")[0], 0)
        self.assertEqual(self.target.read_bytes(), b"previous runtime bytes")


if __name__ == "__main__":
    unittest.main()
