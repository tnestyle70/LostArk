#!/usr/bin/env python3
from __future__ import annotations

import copy
import json
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from Tools.CompositionPipeline import composition_pipeline as pipeline
from Tools.ValtanPipeline import valtan_tuning_pipeline as valtan


ROOT = Path(__file__).resolve().parents[2]


class CompositionPipelineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.validated = pipeline.load_and_validate_all(ROOT)

    def test_repository_closes_valtan_and_kouku_saydon_coverage(self) -> None:
        valtan_product = self.validated[pipeline.BOSS_AUTHORING[0]]["resolved"]
        self.assertEqual(42, len(valtan_product["joinedPatternMaster"]["patterns"]))
        self.assertEqual(
            194,
            sum(
                len(row["stages"])
                for row in valtan_product["joinedPatternMaster"]["patterns"]
            ),
        )
        kouku_saydon = self.validated[pipeline.BOSS_AUTHORING[1]]["resolved"]
        self.assertEqual(4, len(kouku_saydon["referenceProfiles"]))
        self.assertEqual(
            349,
            sum(
                len(row["actionReference"]["actions"])
                for row in kouku_saydon["referenceProfiles"]
            ),
        )

    def test_valtan_source_manifest_covers_sound_and_effect_v2(self) -> None:
        manifest = valtan.source_manifest(ROOT)
        paths = {row["path"] for row in manifest["files"]}
        self.assertIn(valtan.PATTERN_SOUND_CUES_REL, paths)
        self.assertIn(valtan.EFFECT_V2_BINDINGS_REL, paths)
        self.assertIn(valtan.COMPOSITION_DESCRIPTOR_REL, paths)

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            selected = (valtan.PATTERN_SOUND_CUES_REL, valtan.EFFECT_V2_BINDINGS_REL)
            for relative in selected:
                target = root / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes((ROOT / relative).read_bytes())
            before = pipeline.source_manifest(root, selected)
            sound = root / valtan.PATTERN_SOUND_CUES_REL
            sound.write_bytes(sound.read_bytes() + b"\n")
            after = pipeline.source_manifest(root, selected)
            self.assertNotEqual(
                before["sourceManifestId"], after["sourceManifestId"]
            )

        products, receipt = pipeline.build_products(ROOT)
        del products
        composition_paths = {
            row["path"] for row in receipt["sourceManifest"]["files"]
        }
        self.assertIn(pipeline.SOUND_CATALOG_REL, composition_paths)
        self.assertIn(valtan.LEGACY_REL, composition_paths)
        self.assertIn(valtan.EFFECT_CATALOG_REL, composition_paths)
        self.assertIn(valtan.CAMERA_REL, composition_paths)
        self.assertTrue(
            any(
                path.startswith("Data/Effects/V2/Authored/")
                for path in composition_paths
            )
        )
        self.assertTrue(
            any(
                path.startswith("Data/Effects/V2/Groups/")
                for path in composition_paths
            )
        )
        self.assertTrue(
            any(
                path.startswith("Data/Effects/Authored/effect.valtan.")
                for path in composition_paths
            )
        )

    def test_pattern_index_and_join_digest_reject_drift(self) -> None:
        path = ROOT / pipeline.BOSS_AUTHORING[0]
        document = pipeline.read_json(path)
        reordered = copy.deepcopy(document)
        reordered["patterns"][0], reordered["patterns"][1] = (
            reordered["patterns"][1],
            reordered["patterns"][0],
        )
        with self.assertRaisesRegex(
            pipeline.CompositionError, "pattern index order/closure drift"
        ):
            pipeline.validate_boss_document(ROOT, reordered, "valtan")

        bad_digest = copy.deepcopy(document)
        bad_digest["coverage"]["expectedStageCount"] -= 1
        with self.assertRaisesRegex(
            pipeline.CompositionError, "composition coverage drift"
        ):
            pipeline.validate_boss_document(ROOT, bad_digest, "valtan")

    def test_shadow_index_projection_is_minimal_and_revisioned(self) -> None:
        path = ROOT / pipeline.BOSS_AUTHORING[0]
        document = pipeline.read_json(path)
        joined = self.validated[pipeline.BOSS_AUTHORING[0]]["resolved"][
            "joinedPatternMaster"
        ]

        unchanged = pipeline.project_valtan_shadow_index(document, joined)
        self.assertEqual(document, unchanged)

        changed_join = copy.deepcopy(joined)
        changed_join["patterns"] = changed_join["patterns"][:-1]
        changed = pipeline.project_valtan_shadow_index(document, changed_join)
        self.assertEqual(document["revision"] + 1, changed["revision"])
        self.assertEqual(len(document["patterns"]) - 1, len(changed["patterns"]))
        self.assertNotIn("stages", changed["patterns"][0])

        malformed = copy.deepcopy(document)
        malformed["unexpected"] = True
        with self.assertRaisesRegex(pipeline.CompositionError, "fields mismatch"):
            pipeline.project_valtan_shadow_index(malformed, joined)

        malformed_source = copy.deepcopy(document)
        malformed_source["sourceDocuments"][0]["unexpected"] = True
        with self.assertRaisesRegex(pipeline.CompositionError, "fields mismatch"):
            pipeline.project_valtan_shadow_index(malformed_source, joined)

        malformed_coverage = copy.deepcopy(document)
        malformed_coverage["coverage"]["unexpected"] = True
        with self.assertRaisesRegex(pipeline.CompositionError, "fields mismatch"):
            pipeline.project_valtan_shadow_index(malformed_coverage, joined)

    def test_kouku_saydon_reference_count_and_revision_reject_drift(self) -> None:
        document = pipeline.read_json(ROOT / pipeline.BOSS_AUTHORING[1])
        bad_count = copy.deepcopy(document)
        bad_count["coverage"]["profiles"][0]["expectedActionCount"] += 1
        with self.assertRaisesRegex(pipeline.CompositionError, "action count drift"):
            pipeline.validate_boss_document(ROOT, bad_count, "kouku_saydon")

        bad_revision = copy.deepcopy(document)
        bad_revision["coverage"]["profiles"][0]["expectedReferenceRevision"] = "0" * 64
        with self.assertRaisesRegex(
            pipeline.CompositionError, "reference revision drift"
        ):
            pipeline.validate_boss_document(ROOT, bad_revision, "kouku_saydon")

    def test_kouku_saydon_owner_headers_and_authority_fail_closed(self) -> None:
        descriptor = pipeline.read_json(ROOT / pipeline.BOSS_AUTHORING[1])
        original_read = pipeline.read_json
        cases = (
            (
                ROOT
                / "Data/Animation/Reference/KoukuSaydon/MN_RPCZ_00.actionreference.json",
                "schema",
                "WRONG",
            ),
            (
                ROOT
                / "Data/Animation/Authored/KoukuSaydon/MN_RPCZ_00.actionbindings.json",
                "authority",
                "PRODUCT",
            ),
            (
                ROOT
                / "Data/Animation/Authored/KoukuSaydon/MN_RPCZ_00.patternbindings.json",
                "schema",
                "WRONG",
            ),
            (
                ROOT
                / "Data/Animation/Authored/KoukuSaydon/MN_RPCZ_00.patternbindings.json",
                "authority",
                "PRODUCT",
            ),
        )
        for raw_path, field, value in cases:
            owner_path = raw_path.resolve()
            corrupted = original_read(owner_path)
            corrupted[field] = value

            def injected(path: Path, *, _corrupted=corrupted) -> dict[str, object]:
                return (
                    copy.deepcopy(_corrupted)
                    if path.resolve() == owner_path
                    else original_read(path)
                )

            with self.subTest(path=owner_path.name, field=field), mock.patch.object(
                pipeline, "read_json", injected
            ):
                with self.assertRaisesRegex(
                    pipeline.CompositionError,
                    "KoukuSaydon reference-only document is invalid",
                ):
                    pipeline.validate_boss_document(ROOT, descriptor, "kouku_saydon")

    def test_external_valtan_owner_contracts_fail_closed(self) -> None:
        animation = pipeline.read_json(ROOT / valtan.BINDINGS_REL)
        sound_events = pipeline._load_valtan_sound_events(ROOT)
        pattern_sound = pipeline.read_json(ROOT / valtan.PATTERN_SOUND_CUES_REL)
        unknown_sound = copy.deepcopy(pattern_sound)
        unknown_sound["cues"][0]["soundEvent"] = "NO_SUCH_EVENT"
        with self.assertRaisesRegex(pipeline.CompositionError, "does not resolve"):
            pipeline._validate_pattern_sound_document(
                unknown_sound, animation, sound_events
            )
        wrong_bank = copy.deepcopy(pattern_sound)
        wrong_bank["cues"][0]["soundBank"] = "WRONG_BANK"
        with self.assertRaisesRegex(pipeline.CompositionError, "soundBank"):
            pipeline._validate_pattern_sound_document(
                wrong_bank, animation, sound_events
            )
        bad_time = copy.deepcopy(pattern_sound)
        bad_time["cues"][0]["startMs"] = pipeline.UINT32_MAX
        with self.assertRaisesRegex(pipeline.CompositionError, "exceeds 60000"):
            pipeline._validate_pattern_sound_document(bad_time, animation, sound_events)

        effect_v2 = pipeline.read_json(ROOT / valtan.EFFECT_V2_BINDINGS_REL)
        effect_v2["bindings"][0]["stopPolicy"] = "INVALID"
        with self.assertRaisesRegex(Exception, "stopPolicy is invalid"):
            pipeline.effect_v2_pipeline.validate_binding_document(
                ROOT,
                effect_v2,
                pipeline.read_json(ROOT / valtan.GAMEPLAY_AUTHORING_REL),
                animation,
                pipeline.read_json(ROOT / valtan.LEGACY_REL),
            )

        effect_v1 = pipeline.read_json(ROOT / valtan.CUES_REL)
        effect_v1["cues"][0]["effectAssetId"] = "effect.valtan.no-such-owner"
        with self.assertRaisesRegex(pipeline.CompositionError, "resolve exactly one"):
            pipeline._validate_v1_effect_owners(ROOT, effect_v1)

        invalid_transform = pipeline.read_json(ROOT / valtan.CUES_REL)
        invalid_transform["cues"][0]["localTransform"]["position"] = "not-a-vector"
        with self.assertRaisesRegex(pipeline.CompositionError, "must be a float3"):
            pipeline._validate_v1_effect_owners(ROOT, invalid_transform)

        aliases = pipeline.read_json(ROOT / valtan.EFFECT_V1_ALIASES_REL)
        aliases["schema"] = "WRONG"
        with self.assertRaisesRegex(pipeline.CompositionError, "contract is invalid"):
            pipeline._validate_v1_alias_owners(ROOT, aliases)

        self_alias = pipeline.read_json(ROOT / valtan.EFFECT_V1_ALIASES_REL)
        self_alias["aliases"][0]["v1EffectAssetId"] = self_alias["aliases"][0][
            "effectAssetId"
        ]
        with self.assertRaisesRegex(pipeline.CompositionError, "distinct .v1.unified"):
            pipeline._validate_v1_alias_owners(ROOT, self_alias)

        missing_source_alias = pipeline.read_json(ROOT / valtan.EFFECT_V1_ALIASES_REL)
        missing_source_alias["aliases"][0]["effectAssetId"] = missing_source_alias[
            "aliases"
        ][0]["v1EffectAssetId"]
        with self.assertRaisesRegex(pipeline.CompositionError, "distinct .v1.unified"):
            pipeline._validate_v1_alias_owners(ROOT, missing_source_alias)

        shakes = pipeline.read_json(ROOT / valtan.SHAKE_CUES_REL)
        shakes["ownerArchetypeId"] = "WRONG"
        with self.assertRaisesRegex(pipeline.CompositionError, "contract is invalid"):
            pipeline._validate_pattern_shake_document(shakes, animation)

        combat_sound = pipeline.read_json(ROOT / valtan.COMBAT_OBJECT_SOUND_CUES_REL)
        combat_sound["cues"][0]["combatObjectArchetypeId"] = "combatobject.NO_SUCH"
        with self.assertRaisesRegex(pipeline.CompositionError, "does not resolve"):
            pipeline._validate_combat_object_sound_document(
                combat_sound,
                sound_events,
                pipeline.read_json(ROOT / valtan.COMBAT_AUTHORING_REL),
            )
        wrong_combat_bank = pipeline.read_json(
            ROOT / valtan.COMBAT_OBJECT_SOUND_CUES_REL
        )
        wrong_combat_bank["cues"][0]["soundBank"] = "WRONG_BANK"
        with self.assertRaisesRegex(pipeline.CompositionError, "bank/event"):
            pipeline._validate_combat_object_sound_document(
                wrong_combat_bank,
                sound_events,
                pipeline.read_json(ROOT / valtan.COMBAT_AUTHORING_REL),
            )

    def test_valtan_camera_and_arena_owner_headers_fail_closed(self) -> None:
        camera_path = (ROOT / valtan.CAMERA_REL).resolve()
        original_read = pipeline.read_json
        camera_cases = []
        bad_schema = original_read(camera_path)
        bad_schema["schema"] = "WRONG"
        camera_cases.append(bad_schema)
        bad_interpolation = original_read(camera_path)
        bad_interpolation["cues"][0]["interpolation"] = "INVALID"
        camera_cases.append(bad_interpolation)
        bad_keyframe = original_read(camera_path)
        bad_keyframe["cues"][0]["keyframes"] = [None]
        camera_cases.append(bad_keyframe)
        bad_death = original_read(camera_path)
        bad_death["deathCue"] = None
        camera_cases.append(bad_death)

        for bad_camera in camera_cases:
            def camera_injected(
                path: Path, *, _bad_camera=bad_camera
            ) -> dict[str, object]:
                return (
                    copy.deepcopy(_bad_camera)
                    if path.resolve() == camera_path
                    else original_read(path)
                )

            with self.subTest(camera=bad_camera.get("schema")), mock.patch.object(
                pipeline, "read_json", camera_injected
            ):
                with self.assertRaises(pipeline.CompositionError):
                    pipeline._load_valtan_camera_index(ROOT)

        arena = pipeline.read_json(ROOT / pipeline.SEQUENCER_AUTHORING[1])
        boss_documents = {
            value["document"]["compositionId"]: value["document"]
            for relative, value in self.validated.items()
            if relative in pipeline.BOSS_AUTHORING
        }
        world_path = (
            ROOT
            / "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/"
            "LV_LUT_MIDNIGHTC_ED.worldsequences.json"
        ).resolve()
        bad_world = original_read(world_path)
        bad_world["instances"].append(copy.deepcopy(bad_world["instances"][0]))

        def world_injected(path: Path) -> dict[str, object]:
            return (
                copy.deepcopy(bad_world)
                if path.resolve() == world_path
                else original_read(path)
            )

        with mock.patch.object(pipeline, "read_json", world_injected):
            with self.assertRaisesRegex(
                pipeline.CompositionError, "duplicate World Sequence instanceId"
            ):
                pipeline.validate_sequencer_document(
                    ROOT, arena, "kouku_saydonArena", boss_documents
                )

    def test_kouku_saydon_arena_rejects_malformed_map_owner_values_before_publish(
        self,
    ) -> None:
        original_read = pipeline.read_json
        world_path = (
            ROOT /
            pipeline.KOUKU_SAYDON_ARENA_SOURCE_DOCUMENTS["WORLD_SEQUENCES"]
        ).resolve()
        camera_path = (
            ROOT /
            pipeline.KOUKU_SAYDON_ARENA_SOURCE_DOCUMENTS["CAMERA_SHOTS"]
        ).resolve()

        world_cases: list[tuple[str, dict[str, object], str]] = []
        negative_speed = original_read(world_path)
        negative_speed["instances"][0]["playbackSpeed"] = -999
        world_cases.append(("negative playback speed", negative_speed, "playbackSpeed"))
        text_delay = original_read(world_path)
        text_delay["instances"][0]["startDelayMs"] = "100"
        world_cases.append(("text start delay", text_delay, "startDelayMs"))
        text_duration = original_read(world_path)
        text_duration["templates"][0]["durationMs"] = "3067"
        world_cases.append(("text duration", text_duration, "durationMs"))

        for label, malformed, error_field in world_cases:
            def inject_world(
                path: Path, *, _malformed=malformed
            ) -> dict[str, object]:
                return (
                    copy.deepcopy(_malformed)
                    if path.resolve() == world_path
                    else original_read(path)
                )

            with self.subTest(owner=label), mock.patch.object(
                pipeline, "read_json", inject_world
            ):
                with self.assertRaisesRegex(pipeline.CompositionError, error_field):
                    # Validate and Publish both enter through this exact full join.
                    pipeline.load_and_validate_all(ROOT)

        camera_cases: list[tuple[str, dict[str, object], str]] = []
        negative_fov = original_read(camera_path)
        negative_fov["shots"][0]["fovYDegrees"] = -1
        camera_cases.append(("negative fov", negative_fov, "fovYDegrees"))
        null_eye = original_read(camera_path)
        null_eye["shots"][0]["eye"] = None
        camera_cases.append(("null eye", null_eye, r"\.eye"))
        oversized_blend = original_read(camera_path)
        oversized_blend["shots"][0]["blendInMs"] = (
            pipeline.CAMERA_SHOT_MAX_BLEND_MS + 1
        )
        camera_cases.append(("oversized blend", oversized_blend, "blendInMs"))

        for label, malformed, error_field in camera_cases:
            def inject_camera(
                path: Path, *, _malformed=malformed
            ) -> dict[str, object]:
                return (
                    copy.deepcopy(_malformed)
                    if path.resolve() == camera_path
                    else original_read(path)
                )

            with self.subTest(owner=label), mock.patch.object(
                pipeline, "read_json", inject_camera
            ):
                with self.assertRaisesRegex(pipeline.CompositionError, error_field):
                    pipeline.load_and_validate_all(ROOT)

    def test_fixed_document_identity_status_and_source_closure_fail_closed(self) -> None:
        boss_documents = {
            value["document"]["compositionId"]: value["document"]
            for relative, value in self.validated.items()
            if relative in pipeline.BOSS_AUTHORING
        }
        arena = pipeline.read_json(ROOT / pipeline.SEQUENCER_AUTHORING[1])
        arena_cases = []
        renamed = copy.deepcopy(arena)
        renamed["sequencerId"] = "arena.sequencer.renamed"
        arena_cases.append(renamed)
        wrong_status = copy.deepcopy(arena)
        wrong_status["status"] = "REFERENCE_ONLY"
        arena_cases.append(wrong_status)
        wrong_boss_path = copy.deepcopy(arena)
        wrong_boss_path["sourceDocuments"][0]["path"] = pipeline.BOSS_AUTHORING[0]
        arena_cases.append(wrong_boss_path)
        missing_role = copy.deepcopy(arena)
        missing_role["sourceDocuments"].pop()
        arena_cases.append(missing_role)
        extra_role = copy.deepcopy(arena)
        extra_role["sourceDocuments"].append(
            {"role": "UNUSED", "path": "Data/Items/ItemCatalog.json"}
        )
        arena_cases.append(extra_role)
        wrong_scene_profiles = copy.deepcopy(arena)
        wrong_scene_profiles["sourceDocuments"][-1]["path"] = (
            "Data/Items/ItemCatalog.json"
        )
        arena_cases.append(wrong_scene_profiles)
        for ordinal, candidate in enumerate(arena_cases):
            with self.subTest(arena_case=ordinal), self.assertRaises(
                pipeline.CompositionError
            ):
                pipeline.validate_sequencer_document(
                    ROOT, candidate, "kouku_saydonArena", boss_documents
                )

        kouku_saydon = pipeline.read_json(ROOT / pipeline.BOSS_AUTHORING[1])
        kouku_saydon["sourceDocuments"].append(
            {"role": "UNUSED", "path": "Data/Items/ItemCatalog.json"}
        )
        with self.assertRaisesRegex(pipeline.CompositionError, "role/path closure"):
            pipeline.validate_boss_document(ROOT, kouku_saydon, "kouku_saydon")

        original_read = pipeline.read_json
        valtan_path = (ROOT / pipeline.BOSS_AUTHORING[0]).resolve()
        kouku_saydon_path = (ROOT / pipeline.BOSS_AUTHORING[1]).resolve()
        valtan_document = original_read(valtan_path)
        kouku_saydon_document = original_read(kouku_saydon_path)

        def swapped_boss_documents(path: Path) -> dict[str, object]:
            if path.resolve() == valtan_path:
                return copy.deepcopy(kouku_saydon_document)
            if path.resolve() == kouku_saydon_path:
                return copy.deepcopy(valtan_document)
            return original_read(path)

        with mock.patch.object(pipeline, "read_json", swapped_boss_documents):
            with self.assertRaisesRegex(pipeline.CompositionError, "fixed authoring path"):
                pipeline.load_and_validate_all(ROOT)

    def test_invalid_source_path_and_duplicate_json_key_fail_closed(self) -> None:
        document = pipeline.read_json(ROOT / pipeline.BOSS_AUTHORING[0])
        escaped = copy.deepcopy(document)
        escaped["sourceDocuments"][0]["path"] = "Data/../outside.json"
        with self.assertRaisesRegex(
            pipeline.CompositionError, "escapes|repository-relative"
        ):
            pipeline.validate_boss_document(ROOT, escaped, "valtan")

        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "duplicate.json"
            path.write_text('{"schema":"one","schema":"two"}', encoding="utf-8")
            with self.assertRaisesRegex(pipeline.CompositionError, "duplicate JSON key"):
                pipeline.read_json(path)

    def test_arena_refs_and_zero_duration_contract_fail_closed(self) -> None:
        boss_documents = {
            value["document"]["compositionId"]: value["document"]
            for relative, value in self.validated.items()
            if relative in pipeline.BOSS_AUTHORING
        }
        document = pipeline.read_json(ROOT / pipeline.SEQUENCER_AUTHORING[1])
        unknown = copy.deepcopy(document)
        unknown["tracks"][0]["payload"]["instanceId"] = "world.sequence.missing"
        with self.assertRaisesRegex(pipeline.CompositionError, "unknown world sequence"):
            pipeline.validate_sequencer_document(
                ROOT, unknown, "kouku_saydonArena", boss_documents
            )

        zero = copy.deepcopy(document)
        zero["durationMs"] = 0
        zero["tracks"][0]["startMs"] = 0
        zero["tracks"].pop()
        with self.assertRaisesRegex(
            pipeline.CompositionError, "zero only when tracks is empty"
        ):
            pipeline.validate_sequencer_document(
                ROOT, zero, "kouku_saydonArena", boss_documents
            )

    def test_arena_rejects_unimplemented_kind_payload_drift_and_unknown_pattern(self) -> None:
        boss_documents = {
            value["document"]["compositionId"]: value["document"]
            for relative, value in self.validated.items()
            if relative in pipeline.BOSS_AUTHORING
        }
        document = pipeline.read_json(ROOT / pipeline.SEQUENCER_AUTHORING[1])

        unsupported = copy.deepcopy(document)
        unsupported["tracks"][0]["kind"] = "SOUND"
        with self.assertRaisesRegex(
            pipeline.CompositionError, "not implemented in formatVersion 1"
        ):
            pipeline.validate_sequencer_document(
                ROOT, unsupported, "kouku_saydonArena", boss_documents
            )

        extra_payload = copy.deepcopy(document)
        extra_payload["tracks"][0]["payload"]["unexpected"] = True
        with self.assertRaisesRegex(pipeline.CompositionError, "field mismatch"):
            pipeline.validate_sequencer_document(
                ROOT, extra_payload, "kouku_saydonArena", boss_documents
            )

        valtan_document = pipeline.read_json(ROOT / pipeline.SEQUENCER_AUTHORING[0])
        valtan_document["durationMs"] = 1000
        valtan_document["tracks"] = [
            {
                "trackId": "track.valtan.unknown-pattern",
                "kind": "ACTOR_PATTERN",
                "startMs": 0,
                "payload": {
                    "bossCompositionId": "boss.composition.valtan",
                    "patternId": "VALTAN_NOT_A_PATTERN",
                },
            }
        ]
        with self.assertRaisesRegex(pipeline.CompositionError, "unknown boss pattern"):
            pipeline.validate_sequencer_document(
                ROOT, valtan_document, "valtanArena", boss_documents
            )

        kouku_saydon_actor = copy.deepcopy(document)
        kouku_saydon_actor["tracks"] = [
            {
                "trackId": "track.kouku_saydon.reference-only-pattern",
                "kind": "ACTOR_PATTERN",
                "startMs": 0,
                "payload": {
                    "bossCompositionId": "boss.composition.kakulsaydon",
                    "patternId": "KAKUL_FAKE_PATTERN",
                },
            }
        ]
        with self.assertRaisesRegex(pipeline.CompositionError, "unknown boss pattern"):
            pipeline.validate_sequencer_document(
                ROOT, kouku_saydon_actor, "kouku_saydonArena", boss_documents
            )

    def test_numeric_tokens_and_native_duration_bound_match_reader(self) -> None:
        boss_documents = {
            value["document"]["compositionId"]: value["document"]
            for relative, value in self.validated.items()
            if relative in pipeline.BOSS_AUTHORING
        }
        document = pipeline.read_json(ROOT / pipeline.SEQUENCER_AUTHORING[1])

        floating_revision = copy.deepcopy(document)
        floating_revision["revision"] = 1.0
        with self.assertRaisesRegex(pipeline.CompositionError, "uint32"):
            pipeline.validate_sequencer_document(
                ROOT, floating_revision, "kouku_saydonArena", boss_documents
            )

        too_long = copy.deepcopy(document)
        too_long["durationMs"] = pipeline.MAX_ARENA_DURATION_MS + 1
        with self.assertRaisesRegex(pipeline.CompositionError, "exceeds 3600000"):
            pipeline.validate_sequencer_document(
                ROOT, too_long, "kouku_saydonArena", boss_documents
            )

        too_large_revision = copy.deepcopy(document)
        too_large_revision["revision"] = pipeline.UINT32_MAX + 1
        with self.assertRaisesRegex(pipeline.CompositionError, "uint32"):
            pipeline.validate_sequencer_document(
                ROOT, too_large_revision, "kouku_saydonArena", boss_documents
            )

        floating_version = copy.deepcopy(document)
        floating_version["formatVersion"] = 1.0
        with self.assertRaisesRegex(pipeline.CompositionError, "formatVersion"):
            pipeline.validate_sequencer_document(
                ROOT, floating_version, "kouku_saydonArena", boss_documents
            )

        null_end = copy.deepcopy(document)
        null_end["tracks"][0]["endMs"] = None
        with self.assertRaisesRegex(pipeline.CompositionError, "uint32"):
            pipeline.validate_sequencer_document(
                ROOT, null_end, "kouku_saydonArena", boss_documents
            )

    def test_native_path_and_array_bounds_match_reader(self) -> None:
        existing = pipeline.BOSS_AUTHORING[0]
        dotted = existing.replace("Bosses/", "Bosses/./")
        with self.assertRaisesRegex(pipeline.CompositionError, "separators"):
            pipeline._safe_source_path(ROOT, dotted, "dotted path")
        with self.assertRaisesRegex(pipeline.CompositionError, "<= 64"):
            pipeline._validate_source_documents(
                ROOT,
                [{"role": f"ROLE_{index}", "path": existing} for index in range(65)],
                "sources",
            )

        boss_documents = {
            value["document"]["compositionId"]: value["document"]
            for relative, value in self.validated.items()
            if relative in pipeline.BOSS_AUTHORING
        }
        arena = pipeline.read_json(ROOT / pipeline.SEQUENCER_AUTHORING[1])
        arena["tracks"] = [copy.deepcopy(arena["tracks"][0])] * (
            pipeline.MAX_TRACKS + 1
        )
        with self.assertRaisesRegex(pipeline.CompositionError, "<= 8192"):
            pipeline.validate_sequencer_document(
                ROOT, arena, "kouku_saydonArena", boss_documents
            )

    def test_products_are_non_runtime_shadow_read_models(self) -> None:
        products, receipt = pipeline.build_products(ROOT)
        self.assertEqual(pipeline.RECEIPT_SCHEMA, receipt["schema"])
        valtan_product = json.loads(
            products["Bosses/Valtan.bosscomposition.json"].decode("utf-8")
        )
        self.assertEqual(pipeline.BOSS_PRODUCT_SCHEMA, valtan_product["schema"])
        self.assertFalse(valtan_product["runtimeEligible"])
        manifest_paths = {
            row["path"] for row in valtan_product["sourceManifest"]["files"]
        }
        self.assertIn(valtan.PATTERN_SOUND_CUES_REL, manifest_paths)
        self.assertIn(valtan.EFFECT_V2_BINDINGS_REL, manifest_paths)

        graph = valtan_product["resolved"]["unifiedPatternGraph"]
        dash = next(row for row in graph if row["patternId"] == "VALTAN_DASH_CHARGE")
        windup = next(row for row in dash["stages"] if row["stageId"] == "WINDUP")
        animation = [row for row in windup["cues"] if row["kind"] == "ANIMATION"]
        self.assertEqual([0, 600, 1200], [row["clock"]["startMs"] for row in animation])
        self.assertEqual([600, 600, 2450], [row["payload"]["wallDurationMs"] for row in animation])

        cue_kinds = {
            cue["kind"]
            for pattern in graph
            for stage in pattern["stages"]
            for cue in stage["cues"]
        }
        for kind in (
            "ANIMATION",
            "EFFECT_V1",
            "EFFECT_V2",
            "SOUND",
            "CAMERA",
            "CAMERA_SHAKE",
            "HIT",
            "COMBAT_OBJECT",
            "LOGIC",
        ):
            self.assertIn(kind, cue_kinds)

        v1_source = pipeline.read_json(
            ROOT
            / "Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json"
        )
        visible_v1_ids = {
            cue["cueId"]
            for pattern in graph
            for stage in pattern["stages"]
            for cue in stage["cues"]
            if cue["kind"] == "EFFECT_V1"
        }
        visible_v1_ids.update(
            cue["cueId"]
            for cue in valtan_product["resolved"]["detachedCues"]
            if cue["kind"] == "EFFECT_V1"
        )
        self.assertEqual(
            {row["bindingId"] for row in v1_source["cues"]}, visible_v1_ids
        )

    def test_publish_rollback_restores_every_previous_byte(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            output_root = Path(temporary) / "Compositions"
            pipeline.publish_products(ROOT, output_root)
            before = {
                path.relative_to(output_root).as_posix(): path.read_bytes()
                for path in output_root.rglob("*")
                if path.is_file()
            }
            with self.assertRaisesRegex(pipeline.CompositionError, "injected failure"):
                pipeline.publish_products(
                    ROOT, output_root, fail_at="after-first-promote"
                )
            after = {
                path.relative_to(output_root).as_posix(): path.read_bytes()
                for path in output_root.rglob("*")
                if path.is_file()
            }
            self.assertEqual(before, after)
            self.assertFalse(
                any(".stage." in path.name or ".rollback." in path.name for path in output_root.rglob("*"))
            )

    def test_publish_retires_pre_migration_product_names_only_after_commit(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            output_root = Path(temporary) / "Compositions"
            legacy_paths = [
                output_root / Path(*Path(relative).parts)
                for relative in pipeline.LEGACY_PRODUCT_RELATIVE_PATHS
            ]
            for path in legacy_paths:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b"legacy generated product\n")

            with self.assertRaisesRegex(pipeline.CompositionError, "injected failure"):
                pipeline.publish_products(
                    ROOT, output_root, fail_at="after-first-promote"
                )
            self.assertTrue(all(path.is_file() for path in legacy_paths))

            pipeline.publish_products(ROOT, output_root)
            self.assertTrue(all(not path.exists() for path in legacy_paths))
            self.assertTrue(
                (output_root / "Bosses/KoukuSaydonGate1.bosscomposition.json").is_file()
            )
            self.assertTrue(
                (output_root / "Sequences/KoukuSaydonArena.sequencer.json").is_file()
            )

    def test_post_commit_backup_cleanup_failure_keeps_new_products(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            output_root = Path(temporary) / "Compositions"
            pipeline.publish_products(ROOT, output_root)
            destination = output_root / "Bosses/Valtan.bosscomposition.json"
            destination.write_bytes(b"previous product\n")
            original_unlink = Path.unlink
            failed = False

            def fail_one_backup(path: Path, *args: object, **kwargs: object) -> None:
                nonlocal failed
                if ".rollback." in path.name and not failed:
                    failed = True
                    raise PermissionError("injected backup cleanup lock")
                original_unlink(path, *args, **kwargs)

            with mock.patch.object(Path, "unlink", fail_one_backup):
                with self.assertRaisesRegex(
                    pipeline.CompositionError, "Products committed"
                ):
                    pipeline.publish_products(ROOT, output_root)

            self.assertNotEqual(b"previous product\n", destination.read_bytes())
            self.assertFalse(any(".stage." in path.name for path in output_root.rglob("*")))
            self.assertTrue(any(".rollback." in path.name for path in output_root.rglob("*")))

    def test_source_drift_during_product_promotion_rolls_back(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            output_root = Path(temporary) / "Compositions"
            pipeline.publish_products(ROOT, output_root)
            before = {
                path.relative_to(output_root).as_posix(): path.read_bytes()
                for path in output_root.rglob("*")
                if path.is_file()
            }
            original_manifest = pipeline.source_manifest
            manifest_calls = 0

            def drift_after_promotion(root: Path, paths: object) -> dict[str, object]:
                nonlocal manifest_calls
                manifest_calls += 1
                result = original_manifest(root, paths)
                # build_products uses six snapshots and publish performs one
                # pre-commit snapshot.  The next snapshot is the optimistic
                # CAS immediately after every destination was promoted.
                if manifest_calls >= 8:
                    result = copy.deepcopy(result)
                    result["sourceManifestId"] = "0" * 64
                return result

            with mock.patch.object(
                pipeline, "source_manifest", drift_after_promotion
            ):
                with self.assertRaisesRegex(
                    pipeline.CompositionError, "changed during Product commit"
                ):
                    pipeline.publish_products(ROOT, output_root)

            after = {
                path.relative_to(output_root).as_posix(): path.read_bytes()
                for path in output_root.rglob("*")
                if path.is_file()
            }
            self.assertEqual(before, after)

    def test_receipt_is_last_visible_product_commit(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            output_root = (Path(temporary) / "Compositions").resolve()
            expected_destinations = {
                (output_root / Path(*Path(relative).parts)).resolve()
                for relative in (
                    *pipeline.PRODUCT_RELATIVE_PATHS.values(),
                    pipeline.PUBLISH_RECEIPT_REL,
                )
            }
            promoted: list[Path] = []
            original_replace = pipeline.os.replace

            def track_replace(source: object, destination: object) -> None:
                destination_path = Path(destination).resolve()
                if destination_path in expected_destinations and ".stage." in Path(source).name:
                    promoted.append(destination_path)
                original_replace(source, destination)

            with mock.patch.object(pipeline.os, "replace", track_replace):
                pipeline.publish_products(ROOT, output_root)
            self.assertEqual(len(expected_destinations), len(promoted))
            self.assertEqual(
                (output_root / pipeline.PUBLISH_RECEIPT_REL).resolve(), promoted[-1]
            )

    def test_same_output_publish_lock_fails_closed(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            output_root = (Path(temporary) / "Compositions").resolve()
            output_root.mkdir(parents=True)
            with pipeline._exclusive_publish_lock(ROOT, output_root, 1.0):
                with self.assertRaisesRegex(pipeline.CompositionError, "is locked"):
                    pipeline.publish_products(
                        ROOT, output_root, lock_timeout_seconds=0.025
                    )

    def test_interrupted_publish_is_recovered_before_next_stage(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            output_root = (Path(temporary) / "Compositions").resolve()
            pipeline.publish_products(ROOT, output_root)
            destination = output_root / "Bosses/Valtan.bosscomposition.json"
            destination.write_bytes(b"previous local generation\n")
            before = {
                path.relative_to(output_root).as_posix(): path.read_bytes()
                for path in output_root.rglob("*")
                if path.is_file() and not path.name.startswith(".")
            }
            with self.assertRaises(KeyboardInterrupt):
                pipeline.publish_products(
                    ROOT, output_root, fail_at="interrupt-after-first-promote"
                )
            self.assertTrue((output_root / pipeline.PUBLISH_JOURNAL_NAME).is_file())
            with self.assertRaisesRegex(pipeline.CompositionError, "after stage"):
                pipeline.publish_products(ROOT, output_root, fail_at="after-stage")
            after = {
                path.relative_to(output_root).as_posix(): path.read_bytes()
                for path in output_root.rglob("*")
                if path.is_file() and not path.name.startswith(".")
            }
            self.assertEqual(before, after)
            self.assertFalse((output_root / pipeline.PUBLISH_JOURNAL_NAME).exists())


if __name__ == "__main__":
    unittest.main()
