"""Exercise the publisher through its public entry point with source variant IDs."""
import copy
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]

class VehicleVariantPublisherTests(unittest.TestCase):
    def run_case(self, rows):
        with tempfile.TemporaryDirectory(prefix="lostark-vehicle-variant-") as temp:
            root = Path(temp)
            script = root / "Tools/GameplayPipeline/Publish-VehicleProfiles.ps1"
            script.parent.mkdir(parents=True)
            shutil.copyfile(ROOT / script.relative_to(root), script)
            doc = root / "Data/Vehicles/VehicleProfiles.json"
            doc.parent.mkdir(parents=True)
            doc.write_text(json.dumps({"schema": "lostark.vehicle-profiles", "formatVersion": 2, "vehicles": rows}), encoding="utf8")
            return subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(script), "-Mode", "Validate"], capture_output=True)

    def row(self, vehicle_id):
        source = json.loads((ROOT / "Data/Vehicles/VehicleProfiles.json").read_text(encoding="utf8"))
        row = copy.deepcopy(next(v for v in source["vehicles"] if v["vehicleId"] == 9524))
        row["vehicleId"] = row["source"]["primaryKey"] = vehicle_id
        return row

    def test_shared_original_skill_ids_across_variants(self):
        result = self.run_case([self.row(9523), self.row(9524)])
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_duplicate_skill_inside_one_variant_is_rejected(self):
        row = self.row(9523)
        row["skills"][1]["skillId"] = row["skills"][0]["skillId"]
        self.assertNotEqual(self.run_case([row]).returncode, 0)

    def test_duplicate_vehicle_is_rejected(self):
        self.assertNotEqual(self.run_case([self.row(9523), self.row(9523)]).returncode, 0)

if __name__ == "__main__":
    unittest.main()
