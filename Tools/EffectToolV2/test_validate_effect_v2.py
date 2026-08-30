from __future__ import annotations

import copy
import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("validate_effect_v2.py")
SPEC = importlib.util.spec_from_file_location("validate_effect_v2", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
VALIDATOR = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = VALIDATOR
SPEC.loader.exec_module(VALIDATOR)


class EffectV2ValidatorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.resource_root = self.root / "Client/Bin/Resources"
        self.authored_root = self.root / "Data/Effects/V2/Authored"
        self.binding_root = self.root / "Data/Effects/V2/Bindings"
        self.authored_root.mkdir(parents=True)
        self.binding_root.mkdir(parents=True)
        resource = self.resource_root / "Effect/Test/base.dds"
        resource.parent.mkdir(parents=True)
        resource.write_bytes(b"dds")
        self.document = {
            "schema": "lostark.effect-v2",
            "formatVersion": 1,
            "effectId": "effect.test.one",
            "effectType": "Particle",
            "slots": {
                "mesh": "",
                "base": "Effect/Test/base.dds",
                "noise": "",
                "mask": "",
                "emissive": "",
                "dissolve": "",
            },
            "params": {},
        }
        self.binding = {
            "schema": "lostark.effect-v2-bindings",
            "formatVersion": 1,
            "archetypeId": "NPC_TEST",
            "bindings": [
                {
                    "effectId": "effect.test.one",
                    "clip": "test_clip",
                    "startMs": 0,
                    "bone": "b_effectroot",
                    "followBone": False,
                    "rotation": "TargetYaw",
                    "stopWithClip": False,
                }
            ],
        }
        self._write_fixture(self.document, self.binding)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _write_json(self, path: Path, value: object) -> None:
        path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")

    def _write_fixture(self, document: dict, binding: dict) -> None:
        self._write_json(
            self.authored_root / "effect.test.one.effectv2.json", document
        )
        self._write_json(self.binding_root / "NPC_TEST.effectv2bindings.json", binding)

    def test_exact_authored_binding_and_resource_join_passes(self) -> None:
        report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(report, {"authored": 1, "bindings": 1, "textures": 1})

    def test_missing_resource_fails_closed(self) -> None:
        (self.resource_root / "Effect/Test/base.dds").unlink()
        with self.assertRaisesRegex(VALIDATOR.ContractError, "resource is missing"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_unknown_binding_effect_fails_closed(self) -> None:
        binding = copy.deepcopy(self.binding)
        binding["bindings"][0]["effectId"] = "effect.test.missing"
        self._write_fixture(self.document, binding)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "no authored effect"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_non_finite_number_fails_closed(self) -> None:
        document = copy.deepcopy(self.document)
        document["params"]["lifetime"] = float("nan")
        self._write_fixture(document, self.binding)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "non-finite JSON number"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_path_escape_fails_closed(self) -> None:
        document = copy.deepcopy(self.document)
        document["slots"]["base"] = "../outside.dds"
        self._write_fixture(document, self.binding)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "escapes Resources"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_resource_root_primary_precedes_shared_and_default(self) -> None:
        primary = self.root / "primary-resources"
        shared = self.root / "shared-resources"
        primary.mkdir()
        shared.mkdir()
        resolved = VALIDATOR.resolve_resource_root(
            self.root,
            environment={
                "LOSTARK_RESOURCE_ROOT": str(primary),
                "LOSTARK_SHARED_ASSET_ROOT": str(shared),
            },
        )
        self.assertEqual(primary.resolve(), resolved)

    def test_resource_root_shared_fallback_and_empty_default(self) -> None:
        shared = self.root / "shared-resources"
        shared.mkdir()
        self.assertEqual(
            shared.resolve(),
            VALIDATOR.resolve_resource_root(
                self.root,
                environment={
                    "LOSTARK_RESOURCE_ROOT": "",
                    "LOSTARK_SHARED_ASSET_ROOT": str(shared),
                },
            ),
        )
        self.assertEqual(
            self.resource_root.resolve(),
            VALIDATOR.resolve_resource_root(
                self.root,
                environment={
                    "LOSTARK_RESOURCE_ROOT": "",
                    "LOSTARK_SHARED_ASSET_ROOT": "",
                },
            ),
        )

    def test_invalid_configured_resource_root_cannot_fall_back(self) -> None:
        shared = self.root / "shared-resources"
        shared.mkdir()
        missing = self.root / "missing-resources"
        with self.assertRaisesRegex(
            VALIDATOR.ContractError, "resource root is unavailable"
        ):
            VALIDATOR.resolve_resource_root(
                self.root,
                environment={
                    "LOSTARK_RESOURCE_ROOT": str(missing),
                    "LOSTARK_SHARED_ASSET_ROOT": str(shared),
                },
            )

    def test_invalid_shared_resource_root_cannot_fall_back_to_default(self) -> None:
        missing = self.root / "missing-resources"
        with self.assertRaisesRegex(
            VALIDATOR.ContractError, "resource root is unavailable"
        ):
            VALIDATOR.resolve_resource_root(
                self.root,
                environment={
                    "LOSTARK_RESOURCE_ROOT": "",
                    "LOSTARK_SHARED_ASSET_ROOT": str(missing),
                },
            )


if __name__ == "__main__":
    unittest.main()
