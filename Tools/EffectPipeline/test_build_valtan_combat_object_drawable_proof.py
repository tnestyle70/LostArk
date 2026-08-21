from __future__ import annotations

import copy
import json
from pathlib import Path
import shutil
import tempfile
import unittest

import build_valtan_combat_object_drawable_proof as builder


REAL_ROOT = Path(__file__).resolve().parent.parent.parent
REAL_RESOURCE_ROOT = (REAL_ROOT / "Client/Bin/Resources").resolve()


class ValtanCombatObjectDrawableProofTests(unittest.TestCase):
    def setUp(self) -> None:
        self._temporary = tempfile.TemporaryDirectory()
        self.root = Path(self._temporary.name).resolve()
        self.resource_root = REAL_RESOURCE_ROOT
        relative_paths = [
            builder.BOSS_CATALOG_RELATIVE_PATH,
            builder.COMBAT_OBJECT_CATALOG_RELATIVE_PATH,
            builder.SKY_EFFECT_RELATIVE_PATH,
            builder.RED_EFFECT_RELATIVE_PATH,
            builder.SOURCE_INVENTORY_RELATIVE_PATH,
            builder.SWEEP_RELATIVE_PATH,
        ]
        for relative in relative_paths:
            source = REAL_ROOT.joinpath(*relative.parts)
            destination = self.root.joinpath(*relative.parts)
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, destination)
        self.sweep_path = self.root.joinpath(*builder.SWEEP_RELATIVE_PATH.parts)
        self._rewrite_sweep_paths()

    def tearDown(self) -> None:
        self._temporary.cleanup()

    def _path(self, relative: Path) -> Path:
        return self.root.joinpath(*relative.parts)

    def _load_sweep(self) -> dict:
        return json.loads(self.sweep_path.read_text(encoding="utf-8"))

    def _write_sweep(self, sweep: dict) -> None:
        self.sweep_path.write_bytes(builder.pretty_bytes(sweep))

    def _rewrite_sweep_paths(self) -> None:
        sweep = self._load_sweep()
        boss_path = self._path(builder.BOSS_CATALOG_RELATIVE_PATH).resolve()
        combat_path = self._path(builder.COMBAT_OBJECT_CATALOG_RELATIVE_PATH).resolve()
        sky_path = self._path(builder.SKY_EFFECT_RELATIVE_PATH).resolve()
        red_path = self._path(builder.RED_EFFECT_RELATIVE_PATH).resolve()
        sweep["resourceRoot"] = self.resource_root.as_posix()
        sweep["bossCatalog"]["path"] = boss_path.as_posix()
        sweep["combatObjectCatalog"]["path"] = combat_path.as_posix()
        sweep["targets"][0]["documentPath"] = sky_path.as_posix()
        sweep["targets"][1]["documentPath"] = red_path.as_posix()
        self._write_sweep(sweep)

    def test_write_check_seal_and_schemas(self) -> None:
        output = builder.write_proof(self.root, self.resource_root)
        self.assertTrue(output.is_file())
        proof = builder.read_json(output)
        builder.verify_proof_seal(proof)
        self.assertEqual(output, builder.check_proof(self.root, self.resource_root))
        self.assertEqual(
            proof["disposition"], "PROOF_PASS_NO_AUTHORED_MUTATION"
        )
        self.assertEqual(
            [target["rootWorldDistinctCount"] for target in proof["targets"]],
            [1, 4],
        )
        self.assertEqual(
            len(proof["targets"][1]["auditedElements"]), 5
        )
        runtime_schema = builder.read_json(
            REAL_ROOT
            / "Tools/EffectPipeline/Schemas/"
            "lostark.valtan-combat-object-runtime-sweep.schema.json"
        )
        proof_schema = builder.read_json(
            REAL_ROOT
            / "Tools/EffectPipeline/Schemas/"
            "lostark.valtan-combat-object-drawable-proof.schema.json"
        )
        self.assertEqual(
            runtime_schema["$schema"],
            "https://json-schema.org/draft/2020-12/schema",
        )
        self.assertEqual(
            proof_schema["$schema"],
            "https://json-schema.org/draft/2020-12/schema",
        )
        self.assertFalse(runtime_schema["additionalProperties"])
        self.assertFalse(proof_schema["additionalProperties"])
        try:
            import jsonschema
        except ImportError:
            self.assertTrue(
                set(runtime_schema["required"]).issubset(self._load_sweep())
            )
            self.assertTrue(set(proof_schema["required"]).issubset(proof))
        else:
            jsonschema.Draft202012Validator.check_schema(runtime_schema)
            jsonschema.Draft202012Validator.check_schema(proof_schema)
            jsonschema.Draft202012Validator(runtime_schema).validate(
                self._load_sweep()
            )
            jsonschema.Draft202012Validator(proof_schema).validate(proof)

    def test_red_late_seek_requires_each_source_draw(self) -> None:
        sweep = self._load_sweep()
        red = sweep["targets"][1]
        late = next(
            sample
            for sample in red["samples"]
            if sample["label"] == "late-initial-seek-mid-flight"
        )
        row = late["rows"][0]
        row["submitted"] = 0
        row["committed"] = 0
        row["hasSubmittedPosition"] = False
        self._write_sweep(sweep)
        with self.assertRaisesRegex(
            builder.ProofError, "RedBlade moving-root/source-5 draw boundary"
        ):
            builder.build_proof(self.root, self.resource_root)

    def test_red_moving_root_and_bounds_fail_closed(self) -> None:
        sweep = self._load_sweep()
        red = sweep["targets"][1]
        red["rootWorldDistinctCount"] = 1
        red["submittedBoundsDistinctCount"] = 1
        for sample in red["samples"]:
            sample["rootPosition"] = [0.0, 0.0, 4.0]
        self._write_sweep(sweep)
        with self.assertRaisesRegex(builder.ProofError, "root x"):
            builder.build_proof(self.root, self.resource_root)

    def test_sky_boundary_requires_mesh_decal_and_particle_transition(self) -> None:
        sweep = self._load_sweep()
        sky = sweep["targets"][0]
        impact_start = next(
            sample
            for sample in sky["samples"]
            if sample["label"] == "late-initial-seek-impact-start"
        )
        particle = next(
            row
            for row in impact_start["rows"]
            if row["elementId"] == builder.SKY_ELEMENTS[2]
        )
        particle["submitted"] = 0
        particle["committed"] = 0
        particle["hasSubmittedPosition"] = False
        self._write_sweep(sweep)
        with self.assertRaisesRegex(
            builder.ProofError, "HighJump target-axe draw boundary"
        ):
            builder.build_proof(self.root, self.resource_root)

    def test_stale_authored_document_hash_is_rejected(self) -> None:
        red_path = self._path(builder.RED_EFFECT_RELATIVE_PATH)
        red_path.write_bytes(red_path.read_bytes() + b"\n")
        with self.assertRaisesRegex(
            builder.ProofError, "runtime target identity changed"
        ):
            builder.build_proof(self.root, self.resource_root)

    def test_existing_lifecycle_failure_is_rejected(self) -> None:
        sweep = self._load_sweep()
        sweep["existingLifecycleHarness"]["failureCount"] = 1
        self._write_sweep(sweep)
        with self.assertRaisesRegex(
            builder.ProofError, "existing combat-object lifecycle proof"
        ):
            builder.build_proof(self.root, self.resource_root)

    def test_source_recipe_must_join_occurrence_inventory(self) -> None:
        inventory_path = self._path(builder.SOURCE_INVENTORY_RELATIVE_PATH)
        original = inventory_path.read_text(encoding="utf-8")
        needle = '"sourceNodeId": "FX_MN_RPBF_00_O:export:6227"'
        self.assertIn(needle, original)
        inventory_path.write_text(
            original.replace(
                needle,
                '"sourceNodeId": "FX_MN_RPBF_00_O:export:missing-6227"',
            ),
            encoding="utf-8",
        )
        with self.assertRaisesRegex(
            builder.ProofError, "absent from occurrence inventory"
        ):
            builder.build_proof(self.root, self.resource_root)

    def test_check_rejects_tampered_proof(self) -> None:
        output = builder.write_proof(self.root, self.resource_root)
        proof = builder.read_json(output)
        proof["safety"]["visualFidelityClaimed"] = True
        output.write_bytes(builder.pretty_bytes(proof))
        with self.assertRaisesRegex(
            builder.ProofError, "artifactSha256 is stale"
        ):
            builder.check_proof(self.root, self.resource_root)


if __name__ == "__main__":
    unittest.main()
