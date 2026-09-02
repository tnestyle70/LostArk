from __future__ import annotations

import copy
import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock


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
        self.group_root = self.root / "Data/Effects/V2/Groups"
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
        self.group = {
            "schema": "lostark.effect-v2-group",
            "formatVersion": 1,
            "groupId": "group.test.one",
            "durationMs": 0,
            "children": [
                {
                    "effectId": "effect.test.one",
                    "startMs": 100,
                    "durationMs": 500,
                    "stop": "Deactivate",
                    "offset": [0.0, 0.0, 0.0],
                    "pitchDegrees": 15.0,
                    "yawDegrees": 0.0,
                    "rollDegrees": -30.0,
                }
            ],
        }
        self._write_fixture(self.document, self.binding)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def _write_json(self, path: Path, value: object) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")

    def _write_fixture(self, document: dict, binding: dict) -> None:
        self._write_json(
            self.authored_root / "effect.test.one.effectv2.json", document
        )
        self._write_json(self.binding_root / "NPC_TEST.effectv2bindings.json", binding)

    def _write_group(self, group: dict) -> None:
        self._write_json(self.group_root / f"{group['groupId']}.effectv2group.json", group)

    def _write_boss_valtan_clip_fixture(
        self, binding_clip: str, canonical_clip: str
    ) -> None:
        (self.binding_root / "NPC_TEST.effectv2bindings.json").unlink()
        binding = copy.deepcopy(self.binding)
        binding["archetypeId"] = "BOSS_VALTAN"
        binding["bindings"][0]["clip"] = binding_clip
        self._write_json(
            self.binding_root / "BOSS_VALTAN.effectv2bindings.json", binding
        )
        self._write_json(
            self.root / "Data/Valtan/Valtan.presentation.json",
            {
                "patterns": [
                    {
                        "stages": [
                            {
                                "animation": {
                                    "occurrences": [{"clip": canonical_clip}]
                                }
                            }
                        ]
                    }
                ]
            },
        )

    def _group_binding(self) -> dict:
        binding = copy.deepcopy(self.binding)
        row = binding["bindings"][0]
        del row["effectId"]
        row["group"] = "group.test.one"
        return binding

    def _write_boss_valtan_v2_fixture(self) -> dict:
        (self.binding_root / "NPC_TEST.effectv2bindings.json").unlink()
        strict_leaf = copy.deepcopy(self.document)
        strict_leaf["params"] = {
            "lifetime": 1.0,
            "loop": False,
            "playRate": 1.0,
            "particle": {"lifetime": [0.25, 0.5]},
        }
        strict_leaf["parts"] = []
        self._write_json(
            self.authored_root / "effect.test.one.effectv2.json", strict_leaf
        )
        binding = {
            "schema": "lostark.effect-v2-bindings",
            "formatVersion": 2,
            "archetypeId": "BOSS_VALTAN",
            "bindings": [
                {
                    "bindingId": "binding.valtan.test",
                    "resource": {"kind": "LEAF", "id": "effect.test.one"},
                    "scope": {
                        "patternId": "PATTERN_TEST",
                        "stageId": "STAGE_TEST",
                        "actionId": "action.test",
                    },
                    "clock": {
                        "basis": "CLIP_OCCURRENCE",
                        "clipOccurrenceId": "action.test.clip.01",
                        "startMs": 0,
                        "repeatPolicy": "ONCE",
                    },
                    "anchor": {
                        "slotId": "b_effectroot",
                        "followPolicy": "FOLLOW_SLOT",
                        "rotationBasis": "TARGET_YAW",
                        "localTransform": {
                            "translation": [0.0, 0.0, 0.0],
                            "rotation": [0.0, 0.0, 0.0],
                            "scale": [1.0, 1.0, 1.0],
                        },
                    },
                    "stopPolicy": "NATURAL",
                }
            ],
        }
        self._write_json(
            self.binding_root / "BOSS_VALTAN.effectv2bindings.json", binding
        )
        self._write_json(
            self.root / "Data/Valtan/Valtan.gameplay.json",
            {
                "patterns": [
                    {
                        "patternId": "PATTERN_TEST",
                        "stages": [
                            {
                                "stageId": "STAGE_TEST",
                                "actionId": "action.test",
                                "durationMs": 1000,
                            }
                        ],
                    }
                ]
            },
        )
        self._write_json(
            self.root
            / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json",
            {
                "bindings": [
                    {
                        "actionId": "action.test",
                        "clips": [
                            {
                                "clipOccurrenceId": "action.test.clip.01",
                                "clip": "test_clip",
                                "sourceStartMs": 0,
                                "playMs": 1000,
                                "loop": False,
                            }
                        ],
                    }
                ]
            },
        )
        self._write_json(
            self.root / "Data/Valtan/Valtan.legacy-compatibility.json",
            {"patternEntries": []},
        )
        return binding

    def _group_and_leaf_binding(self, leaf_start_ms: int) -> dict:
        binding = self._group_binding()
        leaf = copy.deepcopy(self.binding["bindings"][0])
        leaf["startMs"] = leaf_start_ms
        binding["bindings"].append(leaf)
        return binding

    def test_exact_authored_binding_and_resource_join_passes(self) -> None:
        report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(
            report,
            {
                "authored": 1,
                "bindings": 1,
                "groups": 0,
                "independent": 0,
                "textures": 1,
                "bossBindingCompatibilityV1": 0,
            },
        )

    def test_boss_valtan_clip_binding_joins_canonical_inventory(self) -> None:
        self._write_boss_valtan_clip_fixture(
            "mesh_att_battle_19_01", "mesh_att_battle_19_01"
        )
        report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(report["bindings"], 1)
        self.assertEqual(report["bossBindingCompatibilityV1"], 1)

    def test_boss_valtan_legacy_clip_binding_fails_closed(self) -> None:
        self._write_boss_valtan_clip_fixture(
            "att_battle_19_01", "mesh_att_battle_19_01"
        )
        with self.assertRaisesRegex(
            VALIDATOR.ContractError, "absent from the canonical BOSS_VALTAN"
        ):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_boss_valtan_format_v2_stable_occurrence_binding_passes(self) -> None:
        self._write_boss_valtan_v2_fixture()
        report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(1, report["bindings"])
        self.assertEqual(0, report["bossBindingCompatibilityV1"])

    def test_boss_valtan_format_v2_explicit_resource_root_precedes_environment(self) -> None:
        self._write_boss_valtan_v2_fixture()
        unrelated_resource_root = self.root / "unrelated-resources"
        unrelated_resource_root.mkdir()
        with mock.patch.dict(
            "os.environ",
            {"LOSTARK_RESOURCE_ROOT": str(unrelated_resource_root)},
            clear=False,
        ):
            report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(1, report["bindings"])
        self.assertEqual(0, report["bossBindingCompatibilityV1"])

    def test_boss_valtan_format_v2_invalid_scope_fails_closed(self) -> None:
        binding = self._write_boss_valtan_v2_fixture()
        binding["bindings"][0]["scope"]["actionId"] = "action.missing"
        self._write_json(
            self.binding_root / "BOSS_VALTAN.effectv2bindings.json", binding
        )
        with self.assertRaisesRegex(VALIDATOR.ContractError, "does not resolve one exact"):
            VALIDATOR.validate(self.root, self.resource_root)

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

    def _write_independent(self, effects: list, groups: list) -> None:
        self._write_json(
            self.authored_root.parent / "Independent.json",
            {
                "schema": "lostark.effect-v2-independent",
                "formatVersion": 1,
                "effects": effects,
                "groups": groups,
            },
        )

    def test_independent_group_passes_without_binding(self) -> None:
        self._write_group(self.group)
        self._write_independent([], ["group.test.one"])
        binding = copy.deepcopy(self.binding)
        binding["bindings"] = []
        self._write_fixture(self.document, binding)
        report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(report["independent"], 1)
        self.assertEqual(report["bindings"], 0)

    def test_independent_unknown_id_fails_closed(self) -> None:
        self._write_independent([], ["group.test.missing"])
        with self.assertRaisesRegex(VALIDATOR.ContractError, "has no document"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_boss_catalog_group_owner_passes_when_document_exists(self) -> None:
        self._write_group(self.group)
        binding = copy.deepcopy(self.binding)
        binding["bindings"] = []
        self._write_fixture(self.document, binding)
        self._write_json(
            self.root / "Data/Actors/BossCatalog.json",
            {
                "schema": "lostark.boss-catalog",
                "formatVersion": 6,
                "bosses": [
                    {
                        "archetypeId": "BOSS_VALTAN",
                        "combatObjectVisuals": [
                            {
                                "effectV2Group": {
                                    "groupId": "group.test.one",
                                    "playbackRate": 1.0,
                                    "visualHitMs": 100,
                                    "serverHitId": "hit.test.one",
                                }
                            }
                        ],
                    }
                ],
            },
        )

        report = VALIDATOR.validate(self.root, self.resource_root)

        self.assertEqual(report["groups"], 1)
        self.assertEqual(report["bindings"], 0)
        self.assertEqual(report["independent"], 0)

    def test_present_invalid_boss_catalog_fails_closed(self) -> None:
        self._write_json(
            self.root / "Data/Actors/BossCatalog.json",
            {
                "schema": "lostark.boss-catalog",
                "formatVersion": 5,
                "bosses": [],
            },
        )

        with self.assertRaisesRegex(
            VALIDATOR.ContractError, "unsupported BossCatalog Effect V2 owner"
        ):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_binding_offset_and_yaw_pass(self) -> None:
        binding = copy.deepcopy(self.binding)
        binding["bindings"][0]["offset"] = [1.5, 0.0, -2.0]
        binding["bindings"][0]["yawDegrees"] = 90.0
        self._write_fixture(self.document, binding)
        report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(report["bindings"], 1)

    def test_same_binding_at_two_start_times_passes(self) -> None:
        binding = copy.deepcopy(self.binding)
        second = copy.deepcopy(binding["bindings"][0])
        second["startMs"] = 400
        second["offset"] = [0.0, 0.0, 3.0]
        binding["bindings"].append(second)
        self._write_fixture(self.document, binding)
        report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(report["bindings"], 2)

    def test_invalid_binding_offset_fails_closed(self) -> None:
        binding = copy.deepcopy(self.binding)
        binding["bindings"][0]["offset"] = [1.0, 2.0]
        self._write_fixture(self.document, binding)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "binding offset"):
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

    def test_group_binding_counts_children_as_bound(self) -> None:
        self._write_group(self.group)
        self._write_fixture(self.document, self._group_binding())
        report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(
            report,
            {
                "authored": 1,
                "bindings": 1,
                "groups": 1,
                "independent": 0,
                "textures": 1,
                "bossBindingCompatibilityV1": 0,
            },
        )

    def test_group_and_direct_leaf_at_same_expanded_clock_fail_closed(self) -> None:
        self._write_group(self.group)
        self._write_fixture(self.document, self._group_and_leaf_binding(100))
        with self.assertRaisesRegex(
            VALIDATOR.ContractError, "leaf overlaps the same leaf inside group"
        ):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_group_and_direct_leaf_at_different_expanded_clocks_pass(self) -> None:
        self._write_group(self.group)
        self._write_fixture(self.document, self._group_and_leaf_binding(101))
        report = VALIDATOR.validate(self.root, self.resource_root)
        self.assertEqual(report["bindings"], 2)

    def test_unbound_group_fails_closed(self) -> None:
        self._write_group(self.group)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "unbound Effect V2 groups"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_nested_group_child_fails_closed(self) -> None:
        self._write_group(self.group)
        nested = copy.deepcopy(self.group)
        nested["groupId"] = "group.test.outer"
        nested["children"][0]["effectId"] = "group.test.one"
        self._write_group(nested)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "nesting is not allowed"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_binding_with_effect_and_group_fails_closed(self) -> None:
        self._write_group(self.group)
        binding = copy.deepcopy(self.binding)
        binding["bindings"][0]["group"] = "group.test.one"
        self._write_fixture(self.document, binding)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "exactly one effectId/group"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_unknown_group_binding_fails_closed(self) -> None:
        binding = self._group_binding()
        binding["bindings"][0]["group"] = "group.test.missing"
        self._write_fixture(self.document, binding)
        with self.assertRaisesRegex(VALIDATOR.ContractError, "no group document"):
            VALIDATOR.validate(self.root, self.resource_root)

    def test_group_child_stop_value_fails_closed(self) -> None:
        group = copy.deepcopy(self.group)
        group["children"][0]["stop"] = "Fade"
        self._write_group(group)
        self._write_fixture(self.document, self._group_binding())
        with self.assertRaisesRegex(VALIDATOR.ContractError, "child stop is invalid"):
            VALIDATOR.validate(self.root, self.resource_root)


if __name__ == "__main__":
    unittest.main()
