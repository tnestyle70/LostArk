from __future__ import annotations

import json
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def _read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8-sig")


def _function(source: str, signature: str, next_signature: str) -> str:
    start = source.index(signature)
    end = source.index(next_signature, start)
    return source[start:end]


class ValtanCombatObjectHitEffectPresentationContractTests(unittest.TestCase):
    def test_rock_explosions_are_timed_damage_carriers(self) -> None:
        document = json.loads(
            _read("Data/Encounters/Valtan/ValtanCombatObjects.json")
        )
        expected = {
            "combatobject.valtan.ground-roar.rock": (5000, 6200),
            "combatobject.valtan.six-pizza.rock-pillar": (19500, 20700),
            "combatobject.valtan.struggling.rock-pillar": (5000, 6200),
            "combatobject.valtan.part-break.rock": (5000, 6200),
        }
        for archetype_id, (at_ms, life_ms) in expected.items():
            with self.subTest(archetype_id=archetype_id):
                rock = next(
                    row for row in document["objects"]
                    if row["combatObjectArchetypeId"] == archetype_id
                )
                self.assertEqual(1.5, rock["coverRadiusM"])
                self.assertEqual(life_ms, rock["lifeMs"])
                self.assertNotIn("presentationEvents", rock)
                self.assertEqual(1, len(rock["hits"]))
                hit = rock["hits"][0]
                self.assertEqual(
                    "hit." + archetype_id.removeprefix("combatobject.") +
                    ".explode",
                    hit["hitId"],
                )
                self.assertEqual("TIMED", hit["trigger"])
                self.assertEqual(at_ms, hit["atMs"])
                self.assertEqual("CIRCLE", hit["hitShape"])
                self.assertEqual(3.0, hit["hitOuterRadius"])
                self.assertEqual("damage.valtan.stomp", hit["serverDamageProfileId"])
                self.assertEqual(1, hit["repeatCount"])
                self.assertEqual(0, hit["repeatIntervalMs"])

    def test_native_consumers_admit_presentation_only_carriers(self) -> None:
        tree = _read("Client/Private/ValtanPatternTree.cpp")
        sound = _read("Client/Private/ValtanCombatObjectSoundCueDocument.cpp")
        self.assertIn('Object.Find("presentationEvents")', tree)
        self.assertIn("const size_t iEventRowCount", tree)
        self.assertIn("0u == iEventRowCount || iEventRowCount > 16u", tree)
        self.assertIn("iEventOffsetMs > Reference->second.iLifetimeMs", tree)
        self.assertNotIn("hits->Get_Array().empty()", sound)

    def test_animation_composition_joins_terminal_presentation_sound(self) -> None:
        source = _read("Client/Private/Animation_Tool.cpp")
        lanes = _function(
            source,
            "void Client::CAnimation_Tool::Render_ValtanPresentationLanes(",
            "void Client::CAnimation_Tool::Render_ValtanPatternMasterUnavailableShell(",
        )
        self.assertIn("CombatObject.PresentationEvents", lanes)
        self.assertIn("Cue.strPresentationEventId ==", lanes)
        self.assertIn("Event.strPresentationEventId", lanes)
        self.assertIn('"Server Semantic Event Sound"', lanes)
        self.assertIn("presentation-event local", lanes)
        self.assertNotIn('"Server Hit Sound"', lanes)

    def test_ground_roar_visual_owns_active_and_hit_effect_assets(self) -> None:
        catalog = json.loads(_read("Data/Actors/BossCatalog.json"))
        valtan = next(
            boss for boss in catalog["bosses"]
            if boss["archetypeId"] == "BOSS_VALTAN"
        )
        visual = next(
            row for row in valtan["combatObjectVisuals"]
            if row["combatObjectArchetypeId"]
            == "combatobject.valtan.ground-roar.rock"
        )
        self.assertEqual(
            {
                "combatObjectArchetypeId":
                    "combatobject.valtan.ground-roar.rock",
                "clientVisualId":
                    "combatobject.visual.valtan.ground-roar.rock.v1",
                "effectAssetId": "effect.valtan.ground-roar.rock.active",
                "hitEffectAssetId":
                    "effect.valtan.ground-roar.rock.explode",
            },
            visual,
        )

    def test_ground_roar_active_effect_reuses_the_authored_group_for_each_root(self) -> None:
        document = json.loads(_read(
            "Data/Effects/Authored/"
            "effect.valtan.ground-roar.rock.active.effect.json"
        ))
        self.assertEqual(
            "effect.valtan.ground-roar.rock.active",
            document["effectAssetId"],
        )
        # This whole six-element authored document is one atomic group. The
        # Server instances the group at four roots; elements are never copied.
        self.assertEqual(6, len(document["elements"]))
        self.assertEqual(
            ["mesh", "particle", "particle", "particle", "particle", "particle"],
            [row["kind"] for row in document["elements"]],
        )
        self.assertEqual(
            6, len({row["id"] for row in document["elements"]})
        )
        self.assertTrue(all(not row["id"].endswith(
            (".q0", ".q1", ".q2", ".q3")
        ) for row in document["elements"]))
        element = document["elements"][0]
        self.assertEqual("mesh", element["kind"])
        self.assertEqual("valtan.ground-roar.rock", element["groupId"])
        resources = {
            row["slotId"]: row["assetId"]
            for row in element["resources"]
        }
        self.assertEqual(
            {
                "meshModel":
                    "Effect/Valtan/Meshes/FX_SM_00/"
                    "fm_d_stoneparts_003.wmodel",
                "base":
                    "Effect/Valtan/Textures/FX_TEX_05/"
                    "fx_k_turtlespec_01.dds",
                "noise":
                    "Effect/Valtan/Textures/FX_TEX_02/"
                    "fx_d_stoneparts_002.dds",
                "mask":
                    "Effect/Valtan/Textures/FX_TEX_02/"
                    "fx_d_fluid_020.dds",
                "dissolve":
                    "Effect/Valtan/Textures/FX_TEX_04/"
                    "fx_h_noise_001.dds",
            },
            resources,
        )
        self.assertEqual(
            "fx_m_mi_n_00.fx_mi.fx_n_me_dissolve_01_04_ma",
            element["material"]["sourceMaterialPath"],
        )
        self.assertEqual(
            "opaque_back_depth_write",
            element["material"]["renderProfile"],
        )
        detail = element["detail"]
        self.assertEqual([0.0, 0.0, 0.0], detail["transform"]["position"])
        self.assertEqual(
            [0.0, 0.0, 0.0],
            detail["transform"]["velocityPerSecond"],
        )
        self.assertEqual(
            [0.600000024, 0.600000024, 0.600000024],
            detail["transform"]["scale"],
        )
        self.assertEqual(5.0, detail["timing"]["lifeTimeSeconds"])
        self.assertEqual(
            1.0,
            detail["timing"]["transformMotionDurationSeconds"],
        )
        self.assertEqual(
            0.959999979,
            detail["timing"]["dissolveStartNormalized"],
        )
        overrides = {
            row["slotId"]: (row["assetId"], row["compilerAssetId"])
            for row in element["authoringOverrides"]["resources"]
        }
        self.assertEqual(
            {
                "mask": (
                    "Effect/Valtan/Textures/FX_TEX_02/"
                    "fx_d_fluid_020.dds",
                    "Effect/Valtan/Textures/FX_TEX_04/"
                    "fx_h_noise_001.dds",
                ),
                "base": (
                    "Effect/Valtan/Textures/FX_TEX_05/"
                    "fx_k_turtlespec_01.dds",
                    "Effect/Valtan/Textures/FX_TEX_02/"
                    "fx_d_fluid_020.dds",
                ),
            },
            overrides,
        )

    def test_ground_roar_explode_effect_is_one_atomic_group_payload(
        self,
    ) -> None:
        document = json.loads(_read(
            "Data/Effects/Authored/"
            "effect.valtan.ground-roar.rock.explode.effect.json"
        ))
        self.assertEqual(
            "effect.valtan.ground-roar.rock.explode",
            document["effectAssetId"],
        )
        self.assertEqual(["particle"], [
            row["kind"] for row in document["elements"]
        ])
        self.assertTrue(all(
            row["groupId"] == "valtan.ground-roar.rock"
            for row in document["elements"]
        ))
        self.assertEqual(
            "ground-roar.rock.explode.debris",
            document["elements"][0]["id"],
        )
        self.assertEqual(
            [0.0, 0.0, 0.0],
            document["elements"][0]["detail"]["transform"]["position"],
        )

    def test_native_ground_roar_runtime_owns_four_group_root_instances(
        self,
    ) -> None:
        source = _read("Server/Private/ServerGameplayContractTests.cpp")
        body = _function(
            source,
            "/* Ground Roar owns",
            "/* Phase-three portal charges start together",
        )
        for expected in (
            "4u == groundRoarObjects.size()",
            "4u == groundRoarSpawned.size()",
            "4u == groundRoarTerminalPresentation.size()",
            "4u == groundRoarTerminalDespawned.size()",
        ):
            self.assertIn(expected, body)
        for stale in (
            "1u == groundRoarObjects.size()",
            "1u == groundRoarSpawned.size()",
            "1u == groundRoarTerminalPresentation.size()",
            "1u == groundRoarTerminalDespawned.size()",
        ):
            self.assertNotIn(stale, body)

    def test_ground_roar_independent_effect_joins_exact_cardinal_instances(self) -> None:
        presentation = json.loads(_read("Data/Valtan/Valtan.presentation.json"))
        self.assertEqual(
            [
                "valtan.independent-effect.target-axe",
                "valtan.independent-effect.donut-in-out",
                "valtan.independent-effect.donut-large",
                "valtan.independent-effect.ground-roar-cardinal-rocks",
                "valtan.independent-effect.six-pizza-rock-pillars",
                "valtan.independent-effect.struggling-rock-pillars",
                "valtan.independent-effect.ghost-portal-once",
            ],
            [row["independentEffectId"] for row in presentation["independentEffects"]],
        )
        independent = next(
            row for row in presentation["independentEffects"]
            if row["independentEffectId"]
            == "valtan.independent-effect.ground-roar-cardinal-rocks"
        )
        self.assertEqual(
            {
                "independentEffectId":
                    "valtan.independent-effect.ground-roar-cardinal-rocks",
                "displayName": "땅구르기 후 사자후 / 4방향 돌",
                "ownership": "SERVER_COMBAT_OBJECT",
                "spawnEventId": "valtan.ground-roar.cardinal-rocks",
            },
            independent,
        )

        gameplay = json.loads(_read("Data/Valtan/Valtan.gameplay.json"))
        ground_roar = next(
            row for row in gameplay["patterns"]
            if row["patternId"] == "VALTAN_GROUND_ROAR"
        )
        event = next(
            row for stage in ground_roar["stages"]
            for row in stage["events"]
            if row["eventId"] == independent["spawnEventId"]
        )
        self.assertEqual("SPAWN_COMBAT_OBJECT_VOLLEY", event["kind"])
        self.assertEqual(
            "combatobject.valtan.ground-roar.rock",
            event["combatObjectArchetypeId"],
        )
        self.assertEqual("BOSS_RELATIVE", event["volleyPolicy"])
        self.assertEqual(4, event["countPerResolvedTarget"])
        self.assertEqual("RADIAL_AROUND_BOSS", event["layout"]["kind"])
        self.assertEqual(6.3639610307, event["layout"]["radiusM"])
        self.assertEqual(45.0, event["layout"]["startAngleDegrees"])
        self.assertEqual(90.0, event["layout"]["angleStepDegrees"])
        self.assertEqual(4, event["maximumTotalObjects"])

        combat_authoring = json.loads(
            _read("Data/Valtan/Valtan.combatobjects.json")
        )
        authored_rock = next(
            row for row in combat_authoring["objects"]
            if row["combatObjectArchetypeId"]
            == event["combatObjectArchetypeId"]
        )
        self.assertEqual(6200, authored_rock["lifetimeMs"])

        combat_product = json.loads(
            _read("Data/Encounters/Valtan/ValtanCombatObjects.json")
        )
        product_rock = next(
            row for row in combat_product["objects"]
            if row["combatObjectArchetypeId"]
            == event["combatObjectArchetypeId"]
        )
        self.assertEqual(authored_rock["lifetimeMs"], product_rock["lifeMs"])

        catalog = json.loads(_read("Data/Actors/BossCatalog.json"))
        valtan = next(
            boss for boss in catalog["bosses"]
            if boss["archetypeId"] == "BOSS_VALTAN"
        )
        visual = next(
            row for row in valtan["combatObjectVisuals"]
            if row["combatObjectArchetypeId"]
            == event["combatObjectArchetypeId"]
        )
        self.assertEqual(
            "effect.valtan.ground-roar.rock.active",
            visual["effectAssetId"],
        )

    def test_actor_catalog_parses_optional_typed_hit_effect(self) -> None:
        header = _read("Client/Public/ActorCatalog.h")
        source = _read("Client/Private/ActorCatalog.cpp")
        self.assertIn("std::string hitEffectAssetId;", header)
        self.assertIn('visual.Find("hitEffectAssetId")', source)
        self.assertIn("!IsStableId(pHitEffectAssetId->Get_String())", source)
        self.assertIn(
            "entryVisual.hitEffectAssetId =\n\t\t\t\t\t\tpHitEffectAssetId->Get_String();",
            source,
        )

    def test_valtan_hit_pulse_spawns_once_at_replicated_pose_then_keeps_sound(self) -> None:
        source = _read("Client/Private/Valtan.cpp")
        body = _function(
            source,
            "bool_t CValtan::Apply_CombatObjectPresentationEvent(",
            "void CValtan::Load_PatternShakeCues()",
        )
        dedupe = body.index(
            "event.iEventSequence <= m_iLastCombatObjectPresentationEventSequence"
        )
        spawn = body.index("CEffectPresentationService::Spawn_WorldRoot(")
        sound = body.index("CGameInstance::Get().Play_Sound(")
        self.assertLess(dedupe, spawn)
        self.assertLess(spawn, sound)
        self.assertIn("desc.strEffectAssetId = visual->hitEffectAssetId;", body)
        self.assertIn(
            "float3_t(event.fPositionX, event.fPositionY, event.fPositionZ)",
            body,
        )
        self.assertIn("event.fYawDegrees", body)
        self.assertIn("std::to_string(event.iEventSequence)", body)
        self.assertNotIn("Stop_WorldRoot", body)

    def test_rock_explosion_sound_shares_the_active_wave_and_hit_visual_edge(
        self,
    ) -> None:
        catalog = json.loads(_read("Data/Actors/BossCatalog.json"))
        valtan = next(
            boss for boss in catalog["bosses"]
            if boss["archetypeId"] == "BOSS_VALTAN"
        )
        expected = {
            "combatobject.valtan.ground-roar.rock": (
                "effect.valtan.ground-roar.rock.active",
                "effect.valtan.ground-roar.rock.explode",
                5.0,
            ),
            "combatobject.valtan.six-pizza.rock-pillar": (
                "effect.valtan.six-pizza.rock.active",
                "effect.valtan.six-pizza.rock.explode",
                19.5,
            ),
            "combatobject.valtan.struggling.rock-pillar": (
                "effect.valtan.struggling.rock.active",
                "effect.valtan.struggling.rock.explode",
                5.0,
            ),
            "combatobject.valtan.part-break.rock": (
                "effect.valtan.ground-roar.rock.active",
                "effect.valtan.ground-roar.rock.explode",
                5.0,
            ),
        }
        effects = json.loads(_read("Data/Effects/EffectCatalog.json"))
        paths = {
            row["effectAssetId"]: row["authoringPath"]
            for row in effects["effects"]
        }
        for archetype_id, (active_id, hit_id, hit_seconds) in expected.items():
            with self.subTest(archetype_id=archetype_id):
                visual = next(
                    row for row in valtan["combatObjectVisuals"]
                    if row["combatObjectArchetypeId"] == archetype_id
                )
                self.assertEqual(active_id, visual["effectAssetId"])
                self.assertEqual(hit_id, visual["hitEffectAssetId"])

                active = json.loads(_read("Data/" + paths[active_id]))
                active_meshes = [
                    row for row in active["elements"]
                    if row["visible"] and row["kind"] == "mesh"
                ]
                self.assertEqual(1, len(active_meshes))
                mesh_timing = active_meshes[0]["detail"]["timing"]
                self.assertEqual(0.0, mesh_timing["startDelaySeconds"])
                self.assertAlmostEqual(
                    hit_seconds,
                    mesh_timing["startDelaySeconds"] +
                    mesh_timing["lifeTimeSeconds"],
                    places=5,
                )

                impact_waves = [
                    row for row in active["elements"]
                    if row["visible"] and row["kind"] == "particle" and
                    row["displayName"] == "donut.impact.wave.black"
                ]
                self.assertEqual(2, len(impact_waves))
                self.assertEqual(
                    [hit_seconds, hit_seconds + 0.2],
                    sorted(
                        row["detail"]["timing"]["startDelaySeconds"]
                        for row in impact_waves
                    ),
                )
                for wave in impact_waves:
                    self.assertEqual(16, wave["detail"]["particle"]["burstCount"])
                    self.assertTrue(any(
                        resource["assetId"].endswith("/fx_h_wave_04.dds")
                        for resource in wave["resources"]
                    ))

                telegraph_names = {
                    "donut.telegraph.inner.grow",
                    "sprite_particle_6",
                    "donut.telegraph.outer.red",
                }
                terminal_telegraphs = [
                    row for row in active["elements"]
                    if row["visible"] and row["kind"] == "particle" and
                    row["displayName"] in telegraph_names
                ]
                self.assertEqual(telegraph_names, {
                    row["displayName"] for row in terminal_telegraphs
                })
                self.assertTrue(all(
                    row["detail"]["timing"]["startDelaySeconds"] ==
                    hit_seconds - 1.0
                    for row in terminal_telegraphs
                ))

                document = json.loads(_read("Data/" + paths[hit_id]))
                visible_particles = [
                    row for row in document["elements"]
                    if row["visible"] and row["kind"] == "particle"
                ]
                self.assertGreater(len(visible_particles), 0)
                self.assertEqual(
                    0.0,
                    min(
                        row["detail"]["timing"]["startDelaySeconds"]
                        for row in visible_particles
                    ),
                )
                self.assertTrue(
                    any(
                        row["detail"]["particle"]["burstCount"] > 0
                        for row in visible_particles
                    )
                )
                self.assertEqual(
                    1.2,
                    max(
                        row["detail"]["timing"]["lifeTimeSeconds"]
                        for row in visible_particles
                    ),
                )

        cues = json.loads(_read(
            "Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json"
        ))
        rock_cues = [
            cue for cue in cues["cues"]
            if cue["combatObjectArchetypeId"] in expected
        ]
        self.assertEqual(4, len(rock_cues))
        self.assertTrue(all("hitId" in cue for cue in rock_cues))
        self.assertTrue(all("startMs" not in cue and "delayMs" not in cue
                            for cue in rock_cues))

        parser = _read("Client/Private/ValtanCombatObjectSoundCueDocument.cpp")
        self.assertNotIn('"delayMs"', parser)
        self.assertNotIn('"startMs"', parser)
        runtime = _function(
            _read("Client/Private/Valtan.cpp"),
            "bool_t CValtan::Apply_CombatObjectPresentationEvent(",
            "void CValtan::Load_PatternShakeCues()",
        )
        self.assertLess(
            runtime.index("CEffectPresentationService::Spawn_WorldRoot("),
            runtime.index("CGameInstance::Get().Play_Sound("),
        )

    def test_level_entry_prewarms_active_and_hit_effects(self) -> None:
        for relative in (
            "Client/Private/Level_Loading.cpp",
            "Client/Private/Level_CharacterSelect.cpp",
        ):
            with self.subTest(relative=relative):
                source = _read(relative)
                self.assertIn(
                    "EffectAssetIds.push_back(Visual.effectAssetId);", source
                )
                self.assertIn(
                    "EffectAssetIds.push_back(Visual.hitEffectAssetId);", source
                )

    def test_effect_source_closure_counts_optional_hit_effect_as_reachable(self) -> None:
        validator = _read("Tools/EffectPipeline/validate_effect_sources.py")
        self.assertIn('hit_effect_asset_id = visual.get("hitEffectAssetId")', validator)
        self.assertIn(
            "BOSS_VALTAN combatObjectVisual {index}.hitEffectAssetId",
            validator,
        )

    def test_effect_tool_indexes_hit_effect_under_the_same_combat_object_owner(self) -> None:
        source = _read("Client/Private/Effect_Tool.cpp")
        body = _function(
            source,
            "bool_t Client::CEffect_Tool::Refresh_DirectAuthoredEditableIndex(",
            "const std::filesystem::path*\nClient::CEffect_Tool::Resolve_DirectAuthoredEditablePath(",
        )
        owner = body.index(
            "const EFFECT_DIRECT_AUTHORED_BOSS_COMBAT_OBJECT_OWNER Owner{"
        )
        active = body.index(
            "BossCombatObjectOwners.emplace(Visual.effectAssetId, Owner);"
        )
        non_empty = body.index("if (!Visual.hitEffectAssetId.empty())")
        terminal = body.index(
            "BossCombatObjectOwners.emplace(\n"
            "\t\t\t\t\t\tVisual.hitEffectAssetId, Owner);"
        )
        self.assertLess(owner, active)
        self.assertLess(active, non_empty)
        self.assertLess(non_empty, terminal)

    def test_local_independent_sample_skips_boss_animation_and_product_cues(self) -> None:
        header = _read("Client/Public/Valtan.h")
        source = _read("Client/Private/Valtan.cpp")
        self.assertIn(
            "bool_t Apply_LocalCombatObjectAuthoringPreviewSample(", header
        )
        sample = _function(
            source,
            "bool_t CValtan::Apply_LocalCombatObjectAuthoringPreviewSample(",
            "void CValtan::Reset_LocalPatternPreviewTransport()",
        )
        self.assertIn("Reset_LocalPatternPreviewTransport();", sample)
        self.assertIn("Sync_LocalPatternCombatObjectPreview(", sample)
        for forbidden in (
            "Apply_PatternPresentationSample(",
            "Spawn_DuePatternEffectCues(",
            "CEffectV2Runtime::Sync_StageAuthoring(",
        ):
            self.assertNotIn(forbidden, sample)

        sync = _function(
            source,
            "bool_t CValtan::Sync_LocalPatternCombatObjectPreview(",
            "bool_t CValtan::Stage_LocalPatternAuthoringPreview(",
        )
        for token in (
            "Template.fStartAngleDegrees +",
            "Template.fAngleStepDegrees * static_cast<f32_t>(iOrdinal)",
            "fStageAgeMs <",
            "Instance.Template.iFirstSpawnOffsetMs",
            "Instance.bActiveAttempted = true;",
            "CEffectPresentationService::Stop_WorldRoot(Handle);",
            "Desc.strEffectAssetId = Visual->hitEffectAssetId;",
            "Event.iAtMs",
        ):
            self.assertIn(token, sync)
        # The active root keeps its authored NATURAL lifetime past the Server
        # lifetimeMs, mirroring the Release path taken on S2C despawn.
        self.assertNotIn(
            "fObjectAgeMs < static_cast<f32_t>(Instance.Template.iLifetimeMs)", sync
        )
        self.assertLess(
            sync.index("Instance.bActiveAttempted = true;"),
            sync.index("Desc.strEffectAssetId = Visual->hitEffectAssetId;"),
        )
        staging = _function(
            source,
            "bool_t CValtan::Stage_LocalPatternAuthoringPreview(",
            "bool_t CValtan::Apply_LocalCombatObjectAuthoringPreviewSample(",
        )
        self.assertIn("0u == Source.iLifetimeMs", staging)
        self.assertIn(
            "Template.iFirstSpawnOffsetMs = Source.iFirstSpawnOffsetMs", staging
        )
        self.assertNotIn("Source.iLifetimeMs > Stage.iDurationMs", staging)

    def test_effect_tool_plays_ground_roar_as_one_locked_multi_root_lifecycle(self) -> None:
        source = _read("Client/Private/Effect_Tool.cpp")
        play = _function(
            source,
            "bool_t Client::CEffect_Tool::Try_PlayValtanCombatObjectIndependentEffect(",
            "bool_t Client::CEffect_Tool::Sync_ValtanCombatObjectIndependentPreview(",
        )
        for token in (
            '"BOSS_RELATIVE" != pCombatObject->strVolleyPolicy',
            '"RADIAL" != pCombatObject->strVolleyLayout',
            "Prepare_ValtanStandaloneEffectTarget()",
            "Stage_LocalPatternAuthoringPreview(",
            "Preview.iInstanceCount = pCombatObject->iSpawnValue;",
            "Preview.iLifetimeMs = pCombatObject->iLifetimeMs;",
            "m_ValtanCombatObjectIndependentPreview = std::move(Preview);",
            "Set_SessionLock(",
            "Sync_ValtanCombatObjectIndependentPreview(true)",
        ):
            self.assertIn(token, play)

        effect_sync = _function(
            source,
            "bool_t Client::CEffect_Tool::Sync_ValtanCombatObjectIndependentPreview(",
            "void Client::CEffect_Tool::Clear_ValtanCombatObjectIndependentPreview()",
        )
        self.assertIn("Resolve_TargetGeneration()", effect_sync)
        self.assertIn("VALTAN_STANDALONE_STATIC_CLIP", effect_sync)
        self.assertIn("pModel->Set_AnimPaused(true);", effect_sync)
        self.assertIn(
            "Apply_LocalCombatObjectAuthoringPreviewSample(", effect_sync
        )

        render = _function(
            source,
            "void Client::CEffect_Tool::Render_ValtanIndependentEffectNode(",
            "bool_t Client::CEffect_Tool::Refresh_ValtanAreaStaticEffects()",
        )
        self.assertIn('"Play Combat Object Lifecycle"', render)
        self.assertIn("Try_PlayValtanCombatObjectIndependentEffect(", render)
        self.assertIn("Valtan IDLE", render)
        self.assertIn("fVolleyStartAngleDegrees", render)
        self.assertIn("fVolleyAngleStepDegrees", render)

        update = _function(
            source,
            "void Client::CEffect_Tool::Update(const f32_t fTimeDelta)",
            "void Client::CEffect_Tool::Render()",
        )
        self.assertIn("bCombatObjectIndependentPreviewActive", update)
        self.assertIn(
            "Sync_ValtanCombatObjectIndependentPreview(bSeekAfterLoop)", update
        )

        restart = _function(
            source,
            "void Client::CEffect_Tool::Start_WorldPreviewFromBeginning()",
            "void Client::CEffect_Tool::Synchronize_LoadedSkillPreview()",
        )
        combat_restart = restart.index(
            "m_ValtanCombatObjectIndependentPreview.has_value()"
        )
        self.assertLess(
            combat_restart,
            restart.index("Stage_WorldPreview()"),
        )
        self.assertIn(
            "Sync_ValtanCombatObjectIndependentPreview(true)", restart
        )

        release = _function(
            source,
            "void Client::CEffect_Tool::Release_WorldPreview(",
            "void Client::CEffect_Tool::Discard_ActiveDocument()",
        )
        self.assertIn("Clear_ValtanCombatObjectIndependentPreview();", release)

        clear = _function(
            source,
            "void Client::CEffect_Tool::Clear_ValtanCombatObjectIndependentPreview()",
            "bool_t Client::CEffect_Tool::Can_PlayValtanServerPattern(",
        )
        self.assertIn("Reset_LocalPatternPresentationSample();", clear)
        self.assertIn("Set_SessionLock(", clear)

    def test_hit_effect_does_not_replace_combat_object_despawn(self) -> None:
        source = _read("Client/Private/ClientReplication.cpp")
        body = _function(
            source,
            "bool Client::CClientReplication::Apply_CombatObjectDespawn(",
            "bool Client::CClientReplication::Spawn_CombatObjectPresentation(",
        )
        self.assertIn("m_CombatObjectProjectionRuntime.Apply_Despawn(", body)
        runtime = _read("Client/Public/CombatObjectProjectionRuntime.h")
        despawn = _function(runtime, "bool_t Apply_Despawn(", "size_t Remove_Source(")
        self.assertIn(
            "sink.Release(record->second.PresentationHandle);", despawn
        )
        self.assertNotIn("sink.Stop(", despawn)
        self.assertIn("m_Records.erase(record);", despawn)
        release = _function(
            source,
            "void Client::CClientReplication::Release_CombatObjectPresentation(",
            "bool Client::CClientReplication::Apply_WorldSnapshot(",
        )
        self.assertNotIn("Stop_WorldRoot(", release)
        self.assertIn("CEffectV2Runtime::Stop_Group(", release)

    def test_composition_shows_four_independent_instances_through_explosion_time(self) -> None:
        source = _read("Client/Private/ActionCompositionWorkbench.cpp")
        timeline = _function(
            source,
            "void Client::CActionCompositionWorkbench::Build_Timeline(",
            "void Client::CActionCompositionWorkbench::Pack_TimelineSubrows()",
        )
        self.assertIn(
            '"Server Combat Object (read-only)" + strSpawnSummary',
            timeline,
        )
        self.assertIn('" | waves "', timeline)
        self.assertIn('" | arena random "', timeline)
        self.assertIn("Object.iLifetimeMs", timeline)
        self.assertNotIn(
            "(std::min)(Object.iLifetimeMs, iStageDurationMs)", timeline
        )
        self.assertIn("iTimelineTailMs", timeline)
        self.assertIn("Item.iEndMs", timeline)

    def test_composition_combat_object_selection_is_visible_but_read_only(self) -> None:
        source = _read("Client/Private/ActionCompositionWorkbench.cpp")
        self.assertIn(
            "Item.eOwner == DETAIL_OWNER::COMBAT_OBJECT ||",
            source,
        )
        self.assertIn(
            "DETAIL_OWNER::COMBAT_OBJECT == SelectedTimelineBox->eOwner",
            source,
        )
        self.assertGreaterEqual(
            source.count("Item.eOwner == m_eDetailOwner"),
            2,
            "selection lookup and yellow outline must use the same owner identity",
        )
        self.assertIn("!bSelectedReadOnlyBox", source)
        self.assertIn(
            "not an Effect V2 binding",
            source,
        )


if __name__ == "__main__":
    unittest.main()
