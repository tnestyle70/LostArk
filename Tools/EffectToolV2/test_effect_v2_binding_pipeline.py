from __future__ import annotations

import copy
import json
import shutil
import tempfile
import unittest
from pathlib import Path

try:
    from . import effect_v2_binding_pipeline as pipeline
except ImportError:  # pragma: no cover - direct script execution
    import effect_v2_binding_pipeline as pipeline


class EffectV2BindingPipelineTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory(prefix="effect-v2-binding-")
        self.root = Path(self.temporary.name).resolve()
        self.authored = self.root / "Data/Effects/V2/Authored"
        self.groups = self.root / "Data/Effects/V2/Groups"
        self.authored.mkdir(parents=True)
        self.groups.mkdir(parents=True)
        self.write_json(
            self.authored / "boss.valtan.leaf-a.effectv2.json",
            self.leaf("boss.valtan.leaf-a", lifetime=1.0),
        )
        self.write_json(
            self.authored / "boss.valtan.leaf-b.effectv2.json",
            self.leaf("boss.valtan.leaf-b", lifetime=2.0),
        )
        self.write_json(
            self.groups / "boss.valtan.group.effectv2group.json",
            {
                "schema": "lostark.effect-v2-group",
                "formatVersion": 2,
                "groupId": "boss.valtan.group",
                "durationMs": 0,
                "children": [
                    self.group_child("child.a", "LEAF", "boss.valtan.leaf-a", 0),
                    self.group_child("child.b", "LEAF", "boss.valtan.leaf-b", 100),
                ],
            },
        )
        self.gameplay = {
            "patterns": [
                {
                    "patternId": "PATTERN_A",
                    "stages": [
                        {
                            "stageId": "STAGE_A",
                            "actionId": "action.a",
                            "durationMs": 2000,
                        },
                        {
                            "stageId": "STAGE_B",
                            "actionId": "action.b",
                            "durationMs": 2000,
                        },
                    ],
                }
            ]
        }
        self.legacy_compatibility = {
            "patternEntries": [
                {
                    "patternId": "PATTERN_LEGACY",
                    "runtimePattern": {
                        "patternId": "PATTERN_LEGACY",
                        "stages": [
                            {
                                "stageId": "LEGACY_STAGE",
                                "actionId": "action.legacy",
                                "durationMs": 2000,
                            }
                        ],
                    },
                }
            ]
        }
        self.animation = {
            "bindings": [
                {
                    "actionId": "action.a",
                    "clips": [
                        {
                            "clipOccurrenceId": "action.a.clip.01",
                            "clip": "clip_shared",
                            "sourceStartMs": 0,
                            "playMs": 2000,
                            "loop": False,
                        }
                    ],
                },
                {
                    "actionId": "action.b",
                    "clips": [
                        {
                            "clipOccurrenceId": "action.b.clip.01",
                            "clip": "clip_b",
                            "sourceStartMs": 0,
                            "playMs": 0,
                            "loop": True,
                        }
                    ],
                },
                {
                    "actionId": "action.legacy",
                    "clips": [
                        {
                            "clipOccurrenceId": "action.legacy.clip.01",
                            "clip": "clip_shared",
                            "sourceStartMs": 0,
                            "playMs": 0,
                            "loop": True,
                        }
                    ],
                },
            ]
        }

    def tearDown(self) -> None:
        self.temporary.cleanup()

    @staticmethod
    def write_json(path: Path, value: object) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")

    @staticmethod
    def leaf(
        effect_id: str,
        *,
        effect_type: str = "Decal",
        lifetime: float = 1.0,
        play_rate: float = 1.0,
        loop: bool = False,
    ) -> dict:
        params: dict = {
            "lifetime": lifetime,
            "playRate": play_rate,
            "loop": loop,
        }
        if effect_type == "Particle":
            params["particle"] = {"lifetime": [0.25, 0.75]}
        if effect_type == "Trail":
            params["trail"] = {"pointLifetime": 0.5}
        return {
            "schema": "lostark.effect-v2",
            "formatVersion": 1,
            "effectId": effect_id,
            "effectType": effect_type,
            "slots": {
                "mesh": "",
                "base": "",
                "noise": "",
                "mask": "",
                "emissive": "",
                "dissolve": "",
            },
            "params": params,
            "parts": [],
        }

    @staticmethod
    def group_child(
        child_id: str,
        kind: str,
        resource_id: str,
        start_ms: int,
        *,
        duration_ms: int = 0,
    ) -> dict:
        return {
            "childId": child_id,
            "resource": {"kind": kind, "id": resource_id},
            "startMs": start_ms,
            "durationMs": duration_ms,
            "stop": "Deactivate",
            "localTransform": {
                "translation": [0.0, 0.0, 0.0],
                "rotation": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        }

    @staticmethod
    def row(
        binding_id: str = "binding.valtan.a",
        *,
        resource_kind: str = "LEAF",
        resource_id: str = "boss.valtan.leaf-a",
        pattern_id: str = "PATTERN_A",
        stage_id: str = "STAGE_A",
        action_id: str = "action.a",
        basis: str = "CLIP_OCCURRENCE",
        occurrence_id: str | None = "action.a.clip.01",
        start_ms: int = 0,
    ) -> dict:
        return {
            "bindingId": binding_id,
            "resource": {"kind": resource_kind, "id": resource_id},
            "scope": {
                "patternId": pattern_id,
                "stageId": stage_id,
                "actionId": action_id,
            },
            "clock": {
                "basis": basis,
                "clipOccurrenceId": occurrence_id,
                "startMs": start_ms,
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

    def document(self, rows: list[dict] | None = None) -> dict:
        return {
            "schema": "lostark.effect-v2-bindings",
            "formatVersion": 2,
            "archetypeId": "BOSS_VALTAN",
            "bindings": [self.row()] if rows is None else rows,
        }

    def validate(self, document: dict) -> dict:
        return pipeline.validate_binding_document(
            self.root,
            document,
            self.gameplay,
            self.animation,
            self.legacy_compatibility,
        )

    def test_schema_files_are_strict_v2_and_ephemeral_read_set_contracts(self) -> None:
        schema_root = Path(__file__).with_name("Schemas")
        bindings = json.loads(
            (schema_root / "lostark.effect-v2-bindings.v2.schema.json").read_text(
                encoding="utf-8"
            )
        )
        read_set = json.loads(
            (
                schema_root
                / "lostark.effect-v2-binding-read-set.v1.schema.json"
            ).read_text(encoding="utf-8")
        )
        groups = json.loads(
            (schema_root / "lostark.effect-v2-group.v2.schema.json").read_text(
                encoding="utf-8"
            )
        )
        self.assertEqual(2, bindings["properties"]["formatVersion"]["const"])
        self.assertFalse(bindings["additionalProperties"])
        self.assertEqual(
            ["bindingId", "resource", "scope", "clock", "anchor", "stopPolicy"],
            bindings["$defs"]["binding"]["required"],
        )
        self.assertNotIn("sha256", json.dumps(bindings, sort_keys=True))
        self.assertEqual("lostark.effect-v2-binding-read-set", read_set["properties"]["schema"]["const"])
        self.assertIn("pinned", read_set["$comment"])
        self.assertFalse(read_set["additionalProperties"])
        self.assertEqual(2, groups["properties"]["formatVersion"]["const"])
        self.assertEqual(
            ["childId", "resource", "startMs", "durationMs", "stop", "localTransform"],
            groups["$defs"]["child"]["required"],
        )

    def test_valid_stable_binding_and_transitive_group_read_set_pass(self) -> None:
        document = self.document(
            [
                self.row(
                    resource_kind="GROUP",
                    resource_id="boss.valtan.group",
                )
            ]
        )
        self.assertEqual(document, self.validate(document))
        snapshot = pipeline.build_resource_read_set(self.root, document)
        self.assertEqual(
            [
                ("GROUP", "boss.valtan.group"),
                ("LEAF", "boss.valtan.leaf-a"),
                ("LEAF", "boss.valtan.leaf-b"),
            ],
            [(row["kind"], row["id"]) for row in snapshot["resources"]],
        )
        self.assertEqual(snapshot, pipeline.validate_resource_read_set(snapshot))
        self.assertEqual(
            snapshot,
            pipeline.assert_resource_read_set_current(self.root, document, snapshot),
        )

    def test_json_object_key_order_is_not_semantic(self) -> None:
        row = self.row()
        reordered = {key: row[key] for key in reversed(list(row))}
        resource = reordered["resource"]
        reordered["resource"] = {key: resource[key] for key in reversed(list(resource))}
        self.assertEqual(
            "binding.valtan.a", self.validate(self.document([reordered]))["bindings"][0]["bindingId"]
        )

    def test_duplicate_identity_invalid_scope_and_bad_trs_fail_closed(self) -> None:
        duplicate = self.document([self.row(), self.row()])
        with self.assertRaisesRegex(pipeline.BindingContractError, "duplicate.*bindingId"):
            self.validate(duplicate)
        missing_scope = self.document([self.row(action_id="action.missing")])
        with self.assertRaisesRegex(pipeline.BindingContractError, "does not resolve one exact"):
            self.validate(missing_scope)
        bad_trs = self.document()
        bad_trs["bindings"][0]["anchor"]["localTransform"]["scale"] = [1, 0, 1]
        with self.assertRaisesRegex(pipeline.BindingContractError, "non-zero"):
            self.validate(bad_trs)

    def test_duplicate_move_update_delete_are_targeted_only_by_binding_id(self) -> None:
        baseline = self.document()
        duplicated = pipeline.apply_binding_mutations(
            baseline,
            [
                {
                    "op": "DUPLICATE_BINDING",
                    "bindingId": "binding.valtan.a",
                    "newBindingId": "binding.valtan.b",
                }
            ],
        )
        self.assertEqual(
            ["binding.valtan.a", "binding.valtan.b"],
            [row["bindingId"] for row in duplicated["bindings"]],
        )
        moved_scope = {
            "patternId": "PATTERN_A",
            "stageId": "STAGE_B",
            "actionId": "action.b",
        }
        moved_clock = {
            "basis": "CLIP_OCCURRENCE",
            "clipOccurrenceId": "action.b.clip.01",
            "startMs": 25,
            "repeatPolicy": "EACH_LOOP",
        }
        moved = pipeline.apply_binding_mutations(
            baseline,
            [
                {
                    "op": "MOVE_BINDING",
                    "bindingId": "binding.valtan.a",
                    "scope": moved_scope,
                    "clock": moved_clock,
                }
            ],
        )
        self.assertEqual(moved_scope, moved["bindings"][0]["scope"])
        self.assertEqual(moved_clock, moved["bindings"][0]["clock"])
        self.validate(moved)
        updated_row = self.row(resource_id="boss.valtan.leaf-b")
        updated = pipeline.apply_binding_mutations(
            baseline,
            [{"op": "UPDATE_BINDING", **updated_row}],
        )
        self.assertEqual("binding.valtan.a", updated["bindings"][0]["bindingId"])
        self.assertEqual("boss.valtan.leaf-b", updated["bindings"][0]["resource"]["id"])
        deleted = pipeline.apply_binding_mutations(
            baseline,
            [{"op": "DELETE_BINDING", "bindingId": "binding.valtan.a"}],
        )
        self.assertEqual([], deleted["bindings"])
        with self.assertRaisesRegex(pipeline.BindingContractError, "unknown.*bindingId"):
            pipeline.apply_binding_mutations(
                baseline,
                [{"op": "DELETE_BINDING", "bindingId": "binding.valtan.missing"}],
            )

    def test_legacy_clip_name_fans_out_exact_occurrences_with_loop_semantics(self) -> None:
        legacy = {
            "schema": "lostark.effect-v2-bindings",
            "formatVersion": 1,
            "archetypeId": "BOSS_VALTAN",
            "bindings": [
                {
                    "effectId": "boss.valtan.leaf-a",
                    "clip": "clip_shared",
                    "startMs": 10,
                    "bone": "b_effectroot",
                    "followBone": True,
                    "rotation": "TargetYaw",
                    "stopWithClip": False,
                }
            ],
        }
        migrated, report = pipeline.migrate_v1_document(
            legacy,
            self.gameplay,
            self.animation,
            self.legacy_compatibility,
        )
        self.assertEqual(2, report["migratedBindingCount"])
        self.assertEqual([], report["rejectedRows"])
        expansions = report["rows"][0]["expandedScopes"]
        self.assertEqual(
            {
                "Data/Valtan/Valtan.gameplay.json",
                "Data/Valtan/Valtan.legacy-compatibility.json",
            },
            {row["sourceOwner"] for row in expansions},
        )
        self.assertEqual(
            ["EACH_LOOP", "ONCE"],
            sorted(row["clock"]["repeatPolicy"] for row in migrated["bindings"]),
        )
        self.assertEqual(
            migrated,
            pipeline.migrate_v1_document(
                legacy,
                self.gameplay,
                self.animation,
                self.legacy_compatibility,
            )[0],
        )
        self.validate(migrated)

    def test_clip_migration_converts_absolute_source_clock_to_occurrence_local(self) -> None:
        animation = copy.deepcopy(self.animation)
        animation["bindings"][0]["clips"][0]["sourceStartMs"] = 400
        legacy = {
            "schema": "lostark.effect-v2-bindings",
            "formatVersion": 1,
            "archetypeId": "BOSS_VALTAN",
            "bindings": [
                {
                    "effectId": "boss.valtan.leaf-a",
                    "clip": "clip_shared",
                    "startMs": 650,
                    "bone": "b_effectroot",
                    "followBone": True,
                    "rotation": "TargetYaw",
                    "stopWithClip": False,
                }
            ],
        }
        migrated, _report = pipeline.migrate_v1_document(
            legacy, self.gameplay, animation, self.legacy_compatibility
        )
        by_occurrence = {
            row["clock"]["clipOccurrenceId"]: row["clock"]["startMs"]
            for row in migrated["bindings"]
        }
        self.assertEqual(250, by_occurrence["action.a.clip.01"])
        self.assertEqual(650, by_occurrence["action.legacy.clip.01"])

    def test_group_v1_migration_preserves_order_stable_id_and_full_trs(self) -> None:
        legacy = {
            "schema": "lostark.effect-v2-group",
            "formatVersion": 1,
            "groupId": "boss.valtan.legacy-group",
            "durationMs": 0,
            "children": [
                {
                    "effectId": "boss.valtan.leaf-a",
                    "startMs": 50,
                    "durationMs": 0,
                    "stop": "Deactivate",
                    "offset": [1.0, 2.0, 3.0],
                    "pitchDegrees": 10.0,
                    "yawDegrees": 20.0,
                    "rollDegrees": 30.0,
                    "scale": [1.0, 2.0, 3.0],
                },
                {
                    "effectId": "boss.valtan.leaf-b",
                    "startMs": 0,
                    "durationMs": 100,
                    "stop": "Kill",
                    "offset": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
            ],
        }
        migrated, report = pipeline.migrate_v1_group_document(legacy)
        repeated, _ = pipeline.migrate_v1_group_document(legacy)
        self.assertEqual(migrated, repeated)
        self.assertEqual(2, report["migratedChildCount"])
        self.assertEqual(
            ["boss.valtan.leaf-a", "boss.valtan.leaf-b"],
            [row["resource"]["id"] for row in migrated["children"]],
        )
        self.assertEqual(
            [10.0, 20.0, 30.0],
            migrated["children"][0]["localTransform"]["rotation"],
        )
        self.assertNotEqual(
            migrated["children"][0]["childId"],
            migrated["children"][1]["childId"],
        )

    def test_group_zero_duration_resolves_particle_and_trail_tails(self) -> None:
        self.write_json(
            self.authored / "boss.valtan.particle.effectv2.json",
            self.leaf(
                "boss.valtan.particle",
                effect_type="Particle",
                lifetime=1.0,
                play_rate=2.0,
            ),
        )
        self.write_json(
            self.authored / "boss.valtan.trail.effectv2.json",
            self.leaf(
                "boss.valtan.trail",
                effect_type="Trail",
                lifetime=2.0,
                play_rate=2.0,
            ),
        )
        nested = {
            "schema": "lostark.effect-v2-group",
            "formatVersion": 2,
            "groupId": "boss.valtan.nested",
            "durationMs": 0,
            "children": [
                self.group_child(
                    "child.particle", "LEAF", "boss.valtan.particle", 100
                ),
                self.group_child("child.trail", "LEAF", "boss.valtan.trail", 0),
            ],
        }
        self.write_json(
            self.groups / "boss.valtan.nested.effectv2group.json", nested
        )
        outer = {
            "schema": "lostark.effect-v2-group",
            "formatVersion": 2,
            "groupId": "boss.valtan.outer",
            "durationMs": 0,
            "children": [
                self.group_child(
                    "child.nested", "GROUP", "boss.valtan.nested", 25
                )
            ],
        }
        self.write_json(self.groups / "boss.valtan.outer.effectv2group.json", outer)
        authored, groups = pipeline._load_resource_documents(self.root)
        leaves, span = pipeline._resolve_group(
            "boss.valtan.outer", authored, groups, require_v2=True
        )
        self.assertEqual(
            [("boss.valtan.particle", 125), ("boss.valtan.trail", 25)],
            [(effect_id, start_ms) for effect_id, start_ms, _ids, _trs in leaves],
        )
        self.assertEqual(
            ("child.nested", "child.particle"), leaves[0][2]
        )
        # Particle: (1.0 + .75) / 2 = 875ms (+100); trail: (2 + .5)/2 = 1250ms.
        self.assertEqual(1275, span)

    def test_leaf_parser_subset_rejects_id_version_duplicate_key_and_missing_asset(self) -> None:
        accepted_id = "e" * 80
        accepted_path = self.authored / f"{accepted_id}.effectv2.json"
        accepted = self.leaf(accepted_id)
        self.write_json(accepted_path, accepted)
        self.assertEqual(
            1000,
            pipeline._validate_leaf_resource(accepted_id, accepted_path, accepted),
        )

        rejected_id = "e" * 81
        rejected_path = self.authored / f"{rejected_id}.effectv2.json"
        rejected = self.leaf(rejected_id)
        with self.assertRaisesRegex(pipeline.BindingContractError, "1..80"):
            pipeline._validate_leaf_resource(rejected_id, rejected_path, rejected)

        boolean_version = self.leaf("boss.valtan.bool-version")
        boolean_version["formatVersion"] = True
        boolean_path = self.authored / "boss.valtan.bool-version.effectv2.json"
        with self.assertRaisesRegex(pipeline.BindingContractError, "header/body"):
            pipeline._validate_leaf_resource(
                "boss.valtan.bool-version", boolean_path, boolean_version
            )

        with self.assertRaisesRegex(pipeline.BindingContractError, "duplicate JSON"):
            pipeline.read_json_bytes(b'{"schema":"a","schema":"b"}', "duplicate")

        asset_leaf = self.leaf("boss.valtan.asset")
        asset_leaf["slots"]["base"] = "Effect/Test/missing.dds"
        asset_path = self.authored / "boss.valtan.asset.effectv2.json"
        temporary_resource_root = self.root / "Client/Bin/Resources"
        with self.assertRaisesRegex(pipeline.BindingContractError, "resource is missing"):
            pipeline._validate_leaf_resource(
                "boss.valtan.asset",
                asset_path,
                asset_leaf,
                resource_root=temporary_resource_root,
            )
        physical = temporary_resource_root / "Effect/Test/missing.dds"
        physical.parent.mkdir(parents=True)
        physical.write_bytes(b"dds")
        self.assertEqual(
            1000,
            pipeline._validate_leaf_resource(
                "boss.valtan.asset",
                asset_path,
                asset_leaf,
                resource_root=temporary_resource_root,
            ),
        )
        asset_leaf["slots"]["base"] = "Effect/Test/missing.png"
        with self.assertRaisesRegex(pipeline.BindingContractError, r"\.dds"):
            pipeline._validate_leaf_resource(
                "boss.valtan.asset",
                asset_path,
                asset_leaf,
                resource_root=temporary_resource_root,
            )

    def test_missing_optional_particle_and_trail_payload_uses_runtime_tail_defaults(self) -> None:
        particle = self.leaf("boss.valtan.default-particle", effect_type="Particle")
        del particle["params"]["particle"]
        particle_path = self.authored / "boss.valtan.default-particle.effectv2.json"
        self.assertEqual(
            2000,
            pipeline._validate_leaf_resource(
                "boss.valtan.default-particle", particle_path, particle
            ),
        )
        trail = self.leaf("boss.valtan.default-trail", effect_type="Trail")
        del trail["params"]["trail"]
        trail_path = self.authored / "boss.valtan.default-trail.effectv2.json"
        self.assertEqual(
            1350,
            pipeline._validate_leaf_resource(
                "boss.valtan.default-trail", trail_path, trail
            ),
        )

    def test_group_duplicate_child_and_unbounded_natural_child_fail_closed(self) -> None:
        group_path = self.groups / "boss.valtan.group.effectv2group.json"
        group = json.loads(group_path.read_text(encoding="utf-8"))
        group["children"][1]["childId"] = group["children"][0]["childId"]
        self.write_json(group_path, group)
        with self.assertRaisesRegex(pipeline.BindingContractError, "duplicate.*childId"):
            self.validate(
                self.document(
                    [self.row(resource_kind="GROUP", resource_id="boss.valtan.group")]
                )
            )
        group["children"][1] = copy.deepcopy(group["children"][0])
        group["children"][1]["childId"] = "child.b"
        self.write_json(group_path, group)
        with self.assertRaisesRegex(
            pipeline.BindingContractError, "duplicate.*semantic child"
        ):
            self.validate(
                self.document(
                    [self.row(resource_kind="GROUP", resource_id="boss.valtan.group")]
                )
            )
        group["children"][1]["localTransform"]["translation"] = [1.0, 0.0, 0.0]
        self.write_json(group_path, group)
        self.validate(
            self.document(
                [self.row(resource_kind="GROUP", resource_id="boss.valtan.group")]
            )
        )
        authored, groups = pipeline._load_resource_documents(self.root)
        leaves, _span_ms = pipeline._resolve_group(
            "boss.valtan.group", authored, groups, require_v2=True
        )
        self.assertEqual(2, len(leaves))
        self.assertEqual(leaves[0][:2], leaves[1][:2])
        self.assertNotEqual(leaves[0][3], leaves[1][3])
        stable_ids = [child["childId"] for child in group["children"]]
        group["children"].reverse()
        self.write_json(group_path, group)
        self.validate(
            self.document(
                [self.row(resource_kind="GROUP", resource_id="boss.valtan.group")]
            )
        )
        self.assertEqual(
            stable_ids,
            list(reversed([child["childId"] for child in group["children"]])),
        )
        self.write_json(
            self.authored / "boss.valtan.leaf-a.effectv2.json",
            self.leaf("boss.valtan.leaf-a", loop=True),
        )
        with self.assertRaisesRegex(pipeline.BindingContractError, "unbounded natural child"):
            self.validate(
                self.document(
                    [self.row(resource_kind="GROUP", resource_id="boss.valtan.group")]
                )
            )

    def test_legacy_orphan_or_non_unique_owner_rejects_with_report(self) -> None:
        legacy = {
            "schema": "lostark.effect-v2-bindings",
            "formatVersion": 1,
            "archetypeId": "BOSS_VALTAN",
            "bindings": [
                {
                    "effectId": "boss.valtan.leaf-a",
                    "stage": "action.orphan",
                    "startMs": 0,
                    "bone": "",
                    "followBone": False,
                    "rotation": "World",
                    "stopWithClip": False,
                }
            ],
        }
        with self.assertRaises(pipeline.BindingMigrationAmbiguityError) as raised:
            pipeline.migrate_v1_document(
                legacy,
                self.gameplay,
                self.animation,
                self.legacy_compatibility,
            )
        report = raised.exception.report
        self.assertEqual(0, report["migratedBindingCount"])
        self.assertEqual(1, len(report["rejectedRows"]))
        self.assertIn("does not resolve one", report["rejectedRows"][0]["reason"])

    def test_resource_body_drift_requires_reload_and_does_not_accept_old_hash(self) -> None:
        document = self.document(
            [
                self.row(
                    resource_kind="GROUP",
                    resource_id="boss.valtan.group",
                )
            ]
        )
        snapshot = pipeline.build_resource_read_set(self.root, document)
        binding_path = (
            self.root
            / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"
        )
        self.write_json(binding_path, document)
        binding_baseline = binding_path.read_bytes()
        group_path = self.groups / "boss.valtan.group.effectv2group.json"
        group_path.write_bytes(group_path.read_bytes() + b" ")
        with self.assertRaisesRegex(pipeline.BindingReadSetStaleError, "Reload"):
            pipeline.assert_resource_read_set_current(self.root, document, snapshot)
        self.assertEqual(binding_baseline, binding_path.read_bytes())
        tampered = copy.deepcopy(snapshot)
        tampered["resources"][0]["sha256"] = "0" * 64
        with self.assertRaisesRegex(pipeline.BindingContractError, "hash is invalid"):
            pipeline.validate_resource_read_set(tampered)

    def test_malformed_leaf_and_legacy_group_are_not_admitted_by_v2_save(self) -> None:
        leaf_path = self.authored / "boss.valtan.leaf-a.effectv2.json"
        leaf = json.loads(leaf_path.read_text(encoding="utf-8"))
        leaf["unexpected"] = True
        self.write_json(leaf_path, leaf)
        with self.assertRaisesRegex(pipeline.BindingContractError, "fields must be exactly"):
            self.validate(self.document())
        malformed_cases = []
        missing_slot = self.leaf("boss.valtan.leaf-a")
        del missing_slot["slots"]["base"]
        malformed_cases.append((missing_slot, "slots.*fields must be exactly"))
        escaped_asset = self.leaf("boss.valtan.leaf-a")
        escaped_asset["slots"]["base"] = "../escape.dds"
        malformed_cases.append((escaped_asset, "Resources-relative"))
        bad_param = self.leaf("boss.valtan.leaf-a")
        bad_param["params"]["meshPreScale"] = 0.0
        malformed_cases.append((bad_param, "out of range"))
        bad_part = self.leaf("boss.valtan.leaf-a")
        bad_part["parts"] = [
            {"index": 256, "visible": True, "base": "Effect/Test/base.dds"}
        ]
        malformed_cases.append((bad_part, "unique integer in.*255"))
        for malformed, diagnostic in malformed_cases:
            with self.subTest(diagnostic=diagnostic):
                self.write_json(leaf_path, malformed)
                with self.assertRaisesRegex(pipeline.BindingContractError, diagnostic):
                    self.validate(self.document())
        self.write_json(leaf_path, self.leaf("boss.valtan.leaf-a"))

        group_path = self.groups / "boss.valtan.group.effectv2group.json"
        self.write_json(
            group_path,
            {
                "schema": "lostark.effect-v2-group",
                "formatVersion": 1,
                "groupId": "boss.valtan.group",
                "durationMs": 0,
                "children": [
                    {
                        "effectId": "boss.valtan.leaf-a",
                        "startMs": 0,
                        "durationMs": 0,
                        "stop": "Deactivate",
                    }
                ],
            },
        )
        with self.assertRaisesRegex(pipeline.BindingContractError, "explicit formatVersion 2"):
            self.validate(
                self.document(
                    [self.row(resource_kind="GROUP", resource_id="boss.valtan.group")]
                )
            )

    def test_migration_cli_cannot_overwrite_canonical_group(self) -> None:
        group_path = self.groups / "boss.valtan.group.effectv2group.json"
        legacy = {
            "schema": "lostark.effect-v2-group",
            "formatVersion": 1,
            "groupId": "boss.valtan.group",
            "durationMs": 100,
            "children": [
                {
                    "effectId": "boss.valtan.leaf-a",
                    "startMs": 0,
                    "durationMs": 100,
                    "stop": "Deactivate",
                }
            ],
        }
        self.write_json(group_path, legacy)
        baseline = group_path.read_bytes()
        result = pipeline.main(
            [
                "--repository-root",
                str(self.root),
                "migrate-group-v1",
                "--input",
                str(group_path),
                "--output",
                str(group_path),
                "--report",
                str(self.root / "migration-report.json"),
            ]
        )
        self.assertEqual(1, result)
        self.assertEqual(baseline, group_path.read_bytes())
        self.assertFalse((self.root / "migration-report.json").exists())

        staged_input = self.root / "staged-group-v1.json"
        self.write_json(staged_input, legacy)
        canonical_binding = (
            self.root
            / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"
        )
        self.write_json(canonical_binding, self.document())
        binding_baseline = canonical_binding.read_bytes()
        result = pipeline.main(
            [
                "--repository-root",
                str(self.root),
                "migrate-group-v1",
                "--input",
                str(staged_input),
                "--output",
                str(self.root / "staged-group-v2.json"),
                "--report",
                str(canonical_binding),
            ]
        )
        self.assertEqual(1, result)
        self.assertEqual(binding_baseline, canonical_binding.read_bytes())
        result = pipeline.main(
            [
                "--repository-root",
                str(self.root),
                "snapshot",
                "--bindings",
                str(canonical_binding),
                "--output",
                str(group_path),
            ]
        )
        self.assertEqual(1, result)
        self.assertEqual(baseline, group_path.read_bytes())

    def test_nested_group_cycle_fails_closed_before_read_set_hash(self) -> None:
        self.write_json(
            self.groups / "boss.valtan.outer.effectv2group.json",
            {
                "schema": "lostark.effect-v2-group",
                "formatVersion": 2,
                "groupId": "boss.valtan.outer",
                "durationMs": 0,
                "children": [
                    self.group_child(
                        "child.inner", "GROUP", "boss.valtan.inner", 0
                    )
                ],
            },
        )
        self.write_json(
            self.groups / "boss.valtan.inner.effectv2group.json",
            {
                "schema": "lostark.effect-v2-group",
                "formatVersion": 2,
                "groupId": "boss.valtan.inner",
                "durationMs": 0,
                "children": [
                    self.group_child(
                        "child.outer", "GROUP", "boss.valtan.outer", 0
                    )
                ],
            },
        )
        document = self.document(
            [
                self.row(
                    resource_kind="GROUP",
                    resource_id="boss.valtan.outer",
                )
            ]
        )
        with self.assertRaisesRegex(pipeline.BindingContractError, "group cycle"):
            pipeline.build_resource_read_set(self.root, document)


class BossValtanLegacyBindingDryRunTests(unittest.TestCase):
    def test_high_jump_axe_impact_cluster_is_rebased_to_the_combat_object_pivot(self) -> None:
        repository_root = Path(__file__).resolve().parents[2]
        group_path = (
            repository_root
            / "Data/Effects/V2/Groups/boss.valtan.axe.effectv2group.json"
        )
        group = pipeline.read_json(group_path)
        effective_z: dict[str, float] = {}
        for child in group["children"]:
            leaf_id = child["resource"]["id"]
            leaf = pipeline.read_json(
                repository_root
                / "Data/Effects/V2/Authored"
                / f"{leaf_id}.effectv2.json"
            )
            effective_z[leaf_id] = (
                child["localTransform"]["translation"][2]
                + leaf["params"]["position"]["start"][2]
            )

        impact_cluster = {
            leaf_id: offset
            for leaf_id, offset in effective_z.items()
            if leaf_id != "boss.valtan.axe_fall_1"
        }
        self.assertEqual(8, len(impact_cluster))
        for leaf_id, offset in impact_cluster.items():
            with self.subTest(leaf=leaf_id):
                self.assertLessEqual(abs(offset), 0.71)
        for leaf_id in (
            "boss.valtan.axe_stuck_1",
            "boss.valtan.hit_3",
            "boss.valtan.blur_2",
        ):
            self.assertAlmostEqual(0.0, effective_z[leaf_id], places=6)

    def test_repository_binding_owner_is_canonical_v2_and_fully_admitted(self) -> None:
        repository_root = Path(__file__).resolve().parents[2]
        canonical = pipeline.read_json(
            repository_root
            / "Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json"
        )
        admitted = pipeline.validate_binding_document(
            repository_root,
            canonical,
            pipeline.read_json(repository_root / "Data/Valtan/Valtan.gameplay.json"),
            pipeline.read_json(
                repository_root
                / "Data/Animation/Authored/Valtan/Valtan.patternbindings.json"
            ),
            pipeline.read_json(
                repository_root / "Data/Valtan/Valtan.legacy-compatibility.json"
            ),
            repository_root / "Client/Bin/Resources",
        )
        self.assertEqual(2, canonical["formatVersion"])
        # 2026-09-04: clip-template parity adds twelve authored impact,
        # landing, stomp, and finisher rows, plus the reviewed THREE STEP_03
        # legacy first pulse, STAGGER wipe, and the CHANNEL aura, on top of the
        # 86-row portal owner.
        # Every repository row must still be admitted; none may be dropped.
        self.assertEqual(101, len(canonical["bindings"]))
        self.assertEqual(len(canonical["bindings"]), len(admitted["bindings"]))
        binding_ids = [row["bindingId"] for row in admitted["bindings"]]
        self.assertEqual(sorted(binding_ids), binding_ids)
        self.assertEqual(len(binding_ids), len(set(binding_ids)))
        self.assertFalse(
            any(
                row["scope"]["actionId"]
                in {
                    "valtan.sequence.rush-success.step-01",
                    "valtan.sequence.rush-if.step-01",
                }
                for row in admitted["bindings"]
            )
        )

    def test_repository_group_migration_is_dry_run_and_resolves_natural_tails(self) -> None:
        repository_root = Path(__file__).resolve().parents[2]
        group_root = repository_root / "Data/Effects/V2/Groups"
        baselines = {path: path.read_bytes() for path in sorted(group_root.glob("*.json"))}
        with tempfile.TemporaryDirectory(prefix="effect-v2-group-dry-run-") as temporary:
            staged_root = Path(temporary)
            shutil.copytree(
                repository_root / "Data/Effects/V2",
                staged_root / "Data/Effects/V2",
            )
            child_counts: dict[str, int] = {}
            for source in baselines:
                document = pipeline.read_json(source)
                child_counts[document["groupId"]] = len(document["children"])
                self.assertEqual(2, document["formatVersion"])
            authored, groups = pipeline._load_resource_documents(staged_root)
            spans = {
                group_id: pipeline._resolve_group(
                    group_id,
                    authored,
                    groups,
                    require_v2=True,
                    resource_root=repository_root / "Client/Bin/Resources",
                )[1]
                for group_id in groups
            }
        self.assertEqual(
            {
                "boss.valtan.axe": 9,
                "boss.valtan.blackhole": 1,
                "boss.valtan.breathe": 5,
                "boss.valtan.breathe.red": 5,
                "boss.valtan.impact": 17,
                "boss.valtan.magicball": 4,
                "boss.valtan.magicball.aura": 2,
                "boss.valtan.portal": 2,
                "boss.valtan.pounding": 2,
                "boss.valtan.pounding.chase": 4,
                "boss.valtan.project-tuned.sequence.trash.pulse-group": 1,
                "boss.valtan.rock-pillar.sequence": 2,
                "boss.valtan.shout": 20,
                "boss.valtan.shout.burst": 6,
                "boss.valtan.six.sonic": 3,
                "boss.valtan.twohand": 9,
            },
            child_counts,
        )
        self.assertEqual(
            {
                "boss.valtan.axe": 5600,
                "boss.valtan.blackhole": 10000,
                "boss.valtan.breathe": 1500,
                "boss.valtan.breathe.red": 1500,
                "boss.valtan.impact": 5000,
                "boss.valtan.magicball": 12000,
                "boss.valtan.magicball.aura": 12000,
                "boss.valtan.portal": 1900,
                "boss.valtan.pounding": 2000,
                "boss.valtan.pounding.chase": 4100,
                "boss.valtan.project-tuned.sequence.trash.pulse-group": 600,
                "boss.valtan.rock-pillar.sequence": 6200,
                "boss.valtan.shout": 3000,
                "boss.valtan.shout.burst": 1000,
                "boss.valtan.six.sonic": 300,
                "boss.valtan.twohand": 2000,
            },
            spans,
        )
        for path, baseline in baselines.items():
            self.assertEqual(baseline, path.read_bytes())


if __name__ == "__main__":
    unittest.main()
