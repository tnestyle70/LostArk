import hashlib
import json
import math
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT_ROOT = Path(__file__).resolve().parent
if str(SCRIPT_ROOT) not in sys.path:
    sys.path.insert(0, str(SCRIPT_ROOT))

from build_four_class_combat_source_intake import (  # noqa: E402
    CLASS_CONFIGS,
    EXPECTED_SKILL_COUNTS,
    EXPECTED_STAGE_COUNTS,
    MATERIALIZED_CLASS_CONFIGS,
    binding_stages,
    build_class_stage_contract,
    build_pending_source_inventory,
    exact_timeline,
    normalize_json_for_serialization,
    parse_exact_clip_catalog,
)


class FourClassCombatSourceIntakeTests(unittest.TestCase):
    @staticmethod
    def sha256(path: Path) -> str:
        digest = hashlib.sha256()
        with path.open("rb") as source:
            for block in iter(lambda: source.read(1024 * 1024), b""):
                digest.update(block)
        return digest.hexdigest()

    @staticmethod
    def canonical_json_sha256(value: dict) -> str:
        return hashlib.sha256(
            json.dumps(
                value,
                ensure_ascii=False,
                sort_keys=True,
                separators=(",", ":"),
            ).encode("utf-8")
        ).hexdigest()

    def write_contract(
        self,
        root: Path,
        player_rows: list[dict],
        binding_rows: list[dict],
        animnotify_blocks: list[tuple[str, int]],
    ):
        player_path = root / "PlayerSkills.json"
        binding_path = root / "DimensionMaster.skillbindings.json"
        animnotify_path = root / "DimensionMaster.animnotify"
        player_path.write_text(
            json.dumps({"skills": player_rows}), encoding="utf-8"
        )
        binding_path.write_text(
            json.dumps(
                {
                    "animationAssetId": "DimensionMaster",
                    "characterClass": "DIMENSIONMASTER",
                    "bindings": binding_rows,
                }
            ),
            encoding="utf-8",
        )
        animnotify_path.write_text(
            "\n".join(
                f'"{clip}" skill={source_skill_id} len=1.0 name="test"'
                for clip, source_skill_id in animnotify_blocks
            )
            + "\n",
            encoding="utf-8",
        )
        return player_path, binding_path, animnotify_path

    @staticmethod
    def player(skill_id: int, slot: str) -> dict:
        return {
            "skillId": skill_id,
            "characterClass": "DIMENSIONMASTER",
            "inputSlot": slot,
            "skillKind": "ACTIVE",
            "setsStance": "NONE",
        }

    def test_flat_action_and_nested_combo_grouping_are_distinct(self):
        self.assertEqual(binding_stages(["a", {"clip": "b"}], 1), [["a", "b"]])
        self.assertEqual(
            binding_stages([["a"], [{"clip": "b"}, "c"]], 1),
            [["a"], ["b", "c"]],
        )
        with self.assertRaisesRegex(ValueError, "mixes flat clips"):
            binding_stages(["a", ["b"]], 1)

    def test_non_finite_source_values_use_valid_json_string_sentinels(self):
        normalized = normalize_json_for_serialization(
            {
                "positive": math.inf,
                "negative": -math.inf,
                "notANumber": math.nan,
                "nested": [1.0, math.inf],
            }
        )
        self.assertEqual(normalized["positive"], "Infinity")
        self.assertEqual(normalized["negative"], "-Infinity")
        self.assertEqual(normalized["notANumber"], "NaN")
        self.assertEqual(normalized["nested"], [1.0, "Infinity"])
        serialized = json.dumps(normalized, allow_nan=False)
        self.assertNotIn(": Infinity", serialized)

    def test_numeric_source_skill_alias_is_provenance_not_join_key(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            player_path, binding_path, animnotify_path = self.write_contract(
                root,
                [self.player(900, "Q")],
                [{"skillId": 900, "clips": ["exact_clip"]}],
                [("exact_clip", 777)],
            )
            result = build_class_stage_contract(
                CLASS_CONFIGS[0],
                json.loads(player_path.read_text(encoding="utf-8")),
                binding_path,
                animnotify_path,
                player_path,
                enforce_expected_counts=False,
            )
            skill = result["skills"][0]
            self.assertEqual(skill["productSkillId"], 900)
            self.assertEqual(skill["sourceSkillIds"], [777])
            self.assertEqual(skill["stages"][0]["clips"][0]["clip"], "exact_clip")
            self.assertEqual(result["summary"]["numericAliasClipOccurrenceCount"], 1)

    def test_duplicate_product_clip_owner_fails_closed(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            player_path, binding_path, animnotify_path = self.write_contract(
                root,
                [self.player(900, "Q"), self.player(901, "W")],
                [
                    {"skillId": 900, "clips": ["same_clip"]},
                    {"skillId": 901, "clips": ["same_clip"]},
                ],
                [("same_clip", 777)],
            )
            with self.assertRaisesRegex(ValueError, "duplicate product owners"):
                build_class_stage_contract(
                    CLASS_CONFIGS[0],
                    json.loads(player_path.read_text(encoding="utf-8")),
                    binding_path,
                    animnotify_path,
                    player_path,
                    enforce_expected_counts=False,
                )

    def test_missing_clip_and_contradictory_stage_source_owner_fail_closed(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            player_path, binding_path, animnotify_path = self.write_contract(
                root,
                [self.player(900, "Q")],
                [{"skillId": 900, "clips": ["a", "b"]}],
                [("a", 1), ("b", 2)],
            )
            with self.assertRaisesRegex(ValueError, "contradictory source ownership"):
                build_class_stage_contract(
                    CLASS_CONFIGS[0],
                    json.loads(player_path.read_text(encoding="utf-8")),
                    binding_path,
                    animnotify_path,
                    player_path,
                    enforce_expected_counts=False,
                )

            _, missing_binding, existing_notify = self.write_contract(
                root,
                [self.player(900, "Q")],
                [{"skillId": 900, "clips": ["missing"]}],
                [("other", 900)],
            )
            with self.assertRaisesRegex(ValueError, "absent from animnotify"):
                build_class_stage_contract(
                    CLASS_CONFIGS[0],
                    json.loads(player_path.read_text(encoding="utf-8")),
                    missing_binding,
                    existing_notify,
                    player_path,
                    enforce_expected_counts=False,
                )

    def test_repository_contract_is_exactly_56_skills_and_78_stages(self):
        repository = SCRIPT_ROOT.parent.parent
        data = repository / "Data"
        player_path = data / "Balance" / "PlayerSkills.json"
        players = json.loads(player_path.read_text(encoding="utf-8-sig"))
        total_skills = 0
        total_stages = 0
        for config in MATERIALIZED_CLASS_CONFIGS:
            result = build_class_stage_contract(
                config,
                players,
                data
                / "Animation"
                / "Authored"
                / config.asset_id
                / f"{config.asset_id}.skillbindings.json",
                data
                / "Animation"
                / "Reference"
                / config.asset_id
                / f"{config.asset_id}.animnotify",
                player_path,
            )
            self.assertEqual(
                result["summary"]["skillCount"],
                EXPECTED_SKILL_COUNTS[config.asset_id],
            )
            self.assertEqual(
                result["summary"]["stageCount"],
                EXPECTED_STAGE_COUNTS[config.asset_id],
            )
            for skill in result["skills"]:
                for stage in skill["stages"]:
                    self.assertEqual(len(stage["sourceSkillIds"]), 1)
            total_skills += result["summary"]["skillCount"]
            total_stages += result["summary"]["stageCount"]
        self.assertEqual(total_skills, 56)
        self.assertEqual(total_stages, 78)

    def test_lancemaster_d_f_use_current_exact_clip_source_events(self):
        repository = SCRIPT_ROOT.parent.parent
        data = repository / "Data"
        player_path = data / "Balance" / "PlayerSkills.json"
        players = json.loads(player_path.read_text(encoding="utf-8-sig"))
        config = next(
            row for row in MATERIALIZED_CLASS_CONFIGS if row.asset_id == "LanceMaster"
        )
        bindings_path = (
            data
            / "Animation"
            / "Authored"
            / "LanceMaster"
            / "LanceMaster.skillbindings.json"
        )
        animnotify_path = (
            data
            / "Animation"
            / "Reference"
            / "LanceMaster"
            / "LanceMaster.animnotify"
        )
        manifest = build_class_stage_contract(
            config,
            players,
            bindings_path,
            animnotify_path,
            player_path,
        )
        catalog = parse_exact_clip_catalog(animnotify_path)
        skills = {row["productSkillId"]: row for row in manifest["skills"]}
        expected = {
            34110: {
                "inputSlot": "D",
                "clip": "flm_sk_crescentsweep",
                "eventCount": 50,
                "particleCount": 32,
                "anchorAssets": {
                    "FX_PC_FLM_01.Par_M_FLM_Sasun_MTrail_04",
                    "FX_PC_FLM_01.Par_M_FLM_Sasun_UpImpact_01",
                },
            },
            34150: {
                "inputSlot": "F",
                "clip": "flm_sk_crushingblow",
                "eventCount": 92,
                "particleCount": 75,
                "anchorAssets": {
                    "FX_PC_FLM_03.Par_N_FLM_DragonSwing_03",
                    "FX_PC_FLM_03.Par_N_FLM_HurricaneSwing_Re_010",
                },
            },
        }
        for skill_id, evidence in expected.items():
            skill = skills[skill_id]
            self.assertEqual(skill["inputSlot"], evidence["inputSlot"])
            self.assertEqual(skill["sourceSkillIds"], [skill_id])
            self.assertEqual(len(skill["stages"]), 1)
            self.assertEqual(
                [
                    clip["clip"]
                    for stage in skill["stages"]
                    for clip in stage["clips"]
                ],
                [evidence["clip"]],
            )
            _, events, _ = exact_timeline(skill, catalog)
            particle_events = [
                row
                for row in events
                if row["kind"] == "EFFECT"
                and row["sourceType"] == "PlayParticleEffect"
            ]
            self.assertEqual(len(events), evidence["eventCount"])
            self.assertEqual(len(particle_events), evidence["particleCount"])
            self.assertTrue(
                all(row["clip"] == evidence["clip"] for row in particle_events)
            )
            self.assertTrue(
                evidence["anchorAssets"]
                <= {row["sourceAsset"] for row in particle_events}
            )

    def test_pending_source_inventory_is_pinned_and_keeps_38180_cueless(self):
        repository = SCRIPT_ROOT.parent.parent
        result = build_pending_source_inventory(repository / "Data")
        self.assertEqual(
            result["schema"],
            "lostark.combat-effect-pending-source-inventory",
        )
        self.assertFalse(result["policy"]["writesImportedArtifacts"])
        self.assertEqual(result["policy"]["genericFallback"], "FORBIDDEN")
        self.assertEqual(result["summary"]["classCount"], 2)
        self.assertEqual(result["summary"]["skillCount"], 23)
        self.assertEqual(result["summary"]["stageCount"], 28)

        classes = {row["animationAssetId"]: row for row in result["classes"]}
        self.assertEqual(set(classes), {"GunSlinger", "Slayer"})
        self.assertEqual(
            classes["GunSlinger"]["source"]["skillBindingsSha256"],
            "b068c2bb14c47730540d4eee9720ad82efc4a6d4e0652e9f9f48b4eb5de97d24",
        )
        self.assertEqual(
            classes["GunSlinger"]["source"]["animationNotifySha256"],
            "d84fa2c3218739f559fb2aef6f042ef604faf1ab3c67e48625114a5eeeadea08",
        )
        self.assertEqual(
            classes["Slayer"]["source"]["skillBindingsSha256"],
            "e2504a561eab0b57a4b2c4ffa16b6348b2b4f65dce1414fac8b262254657a63d",
        )
        self.assertEqual(
            classes["Slayer"]["source"]["animationNotifySha256"],
            "d651a13dd64c786d7bae18a374e0222bdebc76ce9b172a38253c0571895c26c7",
        )

        gunslinger_skills = {
            row["productSkillId"]: row for row in classes["GunSlinger"]["skills"]
        }
        spiral_chaser = gunslinger_skills[38180]
        self.assertEqual(spiral_chaser["inputSlot"], "S")
        self.assertEqual(spiral_chaser["status"], "SOURCE_CUE_ABSENT")
        self.assertEqual(spiral_chaser["genericFallback"], "FORBIDDEN")
        self.assertEqual(spiral_chaser["exactParticleOccurrenceCount"], 0)
        self.assertEqual(spiral_chaser["unresolvedEffectNotifyCount"], 4)
        self.assertEqual(spiral_chaser["exactSourceAssets"], [])

    def test_materialized_stage_manifests_have_exact_source_artifact_closure(self):
        repository = SCRIPT_ROOT.parent.parent
        imported = repository / "Data" / "Effects" / "Imported"
        hash_cache = {}
        total_skills = 0
        total_stages = 0
        for config in MATERIALIZED_CLASS_CONFIGS:
            manifest_path = (
                imported
                / config.asset_id
                / f"{config.asset_id}.combat-source-stage-manifest.json"
            )
            manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
            self.assertEqual(
                manifest["schema"],
                "lostark.combat-effect-source-stage-manifest",
            )
            self.assertEqual(manifest["version"], 1)
            total_skills += len(manifest["skills"])
            for skill in manifest["skills"]:
                product_skill_id = skill["productSkillId"]
                total_stages += len(skill["stages"])
                for stage in skill["stages"]:
                    self.assertNotEqual(stage["status"], "BLOCKED")
                    self.assertEqual(len(stage["sourceSkillIds"]), 1)
                    self.assertEqual(len(stage["sourceArtifacts"]), 1)
                    source = stage["sourceArtifacts"][0]
                    self.assertEqual(
                        source["sourceSkillId"], stage["sourceSkillIds"][0]
                    )
                    for name in (
                        "sourceReceipt",
                        "generatedSourceReceipt",
                        "normalizedGraph",
                        "externalModuleClosure",
                        "importedDocument",
                        "conversionReceipt",
                    ):
                        descriptor = source[name]
                        artifact_path = repository / descriptor["path"]
                        self.assertTrue(artifact_path.is_file(), descriptor["path"])
                        if descriptor["path"] not in hash_cache:
                            hash_cache[descriptor["path"]] = self.sha256(artifact_path)
                        self.assertEqual(
                            descriptor["sha256"], hash_cache[descriptor["path"]]
                        )

                    receipt_path = repository / source["sourceReceipt"]["path"]
                    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
                    self.assertEqual(
                        receipt["schema"],
                        "lostark.combat-effect-product-source-receipt",
                    )
                    self.assertEqual(receipt["version"], 1)
                    self.assertEqual(receipt["productSkillId"], product_skill_id)
                    self.assertEqual(receipt["sourceSkillIds"], skill["sourceSkillIds"])
                    stage_sequences = {
                        clip["sequenceIndex"] for clip in stage["clips"]
                    }
                    expected_events = {
                        event["eventId"]
                        for event in receipt["timeline"]["events"]
                        if event["clipSequenceIndex"] in stage_sequences
                    }
                    self.assertEqual(set(stage["sourceEventIds"]), expected_events)

                    closure = json.loads(
                        (
                            repository / source["externalModuleClosure"]["path"]
                        ).read_text(encoding="utf-8")
                    )
                    self.assertEqual(
                        closure["summary"]["unresolvedRequestCount"], 0
                    )
                    self.assertEqual(closure["summary"]["propertyErrorCount"], 0)
                    document = json.loads(
                        (
                            repository / source["importedDocument"]["path"]
                        ).read_text(encoding="utf-8")
                    )
                    self.assertEqual(
                        source["importedDocument"]["effectAssetId"],
                        document["effectAssetId"],
                    )

        matrix = json.loads(
            (imported / "FourClassCombat.source-blocker-matrix.json").read_text(
                encoding="utf-8"
            )
        )
        self.assertEqual(total_skills, 51)
        self.assertEqual(total_stages, 74)
        self.assertEqual(matrix["summary"]["skillCount"], 51)
        self.assertEqual(matrix["summary"]["stageCount"], 74)
        self.assertEqual(matrix["summary"]["blockedSkillCount"], 0)
        self.assertEqual(matrix["summary"]["sourceReceiptCount"], 51)
        self.assertEqual(matrix["summary"]["closureCount"], 51)
        self.assertEqual(matrix["summary"]["importedDocumentCount"], 51)

    def test_artist_31210_repeated_occurrence_and_silent_stage_are_preserved(self):
        repository = SCRIPT_ROOT.parent.parent
        artist_root = repository / "Data" / "Effects" / "Imported" / "Artist"
        manifest = json.loads(
            (artist_root / "Artist.combat-source-stage-manifest.json").read_text(
                encoding="utf-8"
            )
        )
        skill = next(
            row for row in manifest["skills"] if row["productSkillId"] == 31210
        )
        self.assertEqual(
            [stage["clips"][0]["clip"] for stage in skill["stages"]],
            [
                "sdm_sk_skykongkong_01",
                "sdm_sk_skykongkong_03",
                "sdm_sk_skykongkong_01",
                "sdm_sk_skykongkong_02",
            ],
        )
        self.assertEqual(
            [stage["clips"][0]["sequenceIndex"] for stage in skill["stages"]],
            [0, 1, 2, 3],
        )
        self.assertTrue(skill["stages"][0]["sourceEventIds"])
        self.assertFalse(skill["stages"][1]["sourceEventIds"])
        self.assertTrue(skill["stages"][2]["sourceEventIds"])
        self.assertTrue(
            set(skill["stages"][0]["sourceEventIds"]).isdisjoint(
                skill["stages"][2]["sourceEventIds"]
            )
        )
        silent_decision = skill["stages"][1]["completionDecision"]
        self.assertEqual(
            silent_decision["decision"], "sourceIntentionallySilent"
        )
        self.assertEqual(silent_decision["evidence"]["effectNotifyCount"], 0)
        self.assertEqual(silent_decision["evidence"]["shakeNotifyCount"], 0)
        self.assertEqual(
            silent_decision["evidence"]["observedNotifyKinds"],
            ["CANCEL", "SUPERARMOR"],
        )

        document_path = (
            artist_root
            / "CurrentCombat"
            / "Converted"
            / "effect.artist.skill.31210.imported.effect.json"
        )
        document = json.loads(document_path.read_text(encoding="utf-8"))
        conversion_path = (
            artist_root
            / "CurrentCombat"
            / "Converted"
            / "skill.31210.element-conversion-receipt.json"
        )
        conversion = json.loads(conversion_path.read_text(encoding="utf-8"))
        final_event_ids = {f"source-event-{index:03d}" for index in range(45, 51)}
        final_rows = [
            row
            for row in conversion["elementConversions"]
            if final_event_ids
            & {event["eventId"] for event in row["eventOccurrences"]}
        ]
        self.assertEqual(len(final_rows), 3)
        for row in final_rows:
            self.assertEqual(row["status"], "SOURCE_RECIPE_RUNTIME_PENDING")
            self.assertEqual(row["rendererShape"], "sprite")
            self.assertIn(
                "particlemoduletypedataribbon",
                {module["className"] for module in row["unrepresentedModules"]},
            )

        approval = skill["stages"][3]["rendererApproximationApproval"]
        self.assertEqual(approval["decision"], "reviewedRendererApproximation")
        self.assertEqual(approval["sourceKind"], "spriteParticle")
        self.assertEqual(approval["targetKind"], "sprite")
        self.assertEqual(approval["sourceDocumentSha256"], self.sha256(document_path))
        elements = {row["id"]: row for row in document["elements"]}
        self.assertEqual(
            set(approval["sourceElementIds"]),
            set(approval["sourceElementSha256"]),
        )
        self.assertEqual(len(approval["sourceElementIds"]), 6)
        for element_id in approval["sourceElementIds"]:
            self.assertEqual(
                approval["sourceElementSha256"][element_id],
                self.canonical_json_sha256(elements[element_id]),
            )


if __name__ == "__main__":
    unittest.main()
