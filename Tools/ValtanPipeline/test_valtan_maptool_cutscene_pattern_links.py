#!/usr/bin/env python3
"""Focused contract for Map Tool Valtan cutscenes linked to Boss patterns."""

from __future__ import annotations

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def load(relative: str) -> dict:
    return json.loads((ROOT / relative).read_text(encoding="utf-8"))


authoring = load(
    "Data/Maps/Authoring/LV_LUT_HEARTRB_ED/"
    "LV_LUT_HEARTRB_ED.camerashots.json"
)
runtime = load(
    "Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.camerashots.json"
)
gameplay = load("Data/Valtan/Valtan.gameplay.json")
presentation = load("Data/Valtan/Valtan.presentation.json")
cameras = load("Data/Encounters/Valtan/ValtanCinematicCamera.json")
world = load(
    "Data/Maps/Authoring/LV_LUT_HEARTRB_ED/"
    "LV_LUT_HEARTRB_ED.worldsequences.json"
)
level_source = (ROOT / "Client/Private/Level_ValtanArena.cpp").read_text(
    encoding="utf-8-sig"
)
level_header = (ROOT / "Client/Public/Level_ValtanArena.h").read_text(
    encoding="utf-8-sig"
)
main_app_source = (ROOT / "Client/Private/MainApp.cpp").read_text(
    encoding="utf-8-sig"
)
preview_panel_source = (ROOT / "Client/Private/CharacterPreviewPanel.cpp").read_text(
    encoding="utf-8-sig"
)
workbench_source = (ROOT / "Client/Private/ValtanActionWorkbench.cpp").read_text(
    encoding="utf-8-sig"
)
world_gameplay = load("Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json")

authoring_cutscenes = {row["cutsceneId"]: row for row in authoring["cutscenes"]}
runtime_cutscenes = {row["cutsceneId"]: row for row in runtime["cutscenes"]}
authoring_shots = {row["shotId"]: row for row in authoring["shots"]}
gameplay_patterns = {row["patternId"]: row for row in gameplay["patterns"]}
presentation_patterns = {row["patternId"]: row for row in presentation["patterns"]}
camera_cues = {row["cueId"]: row for row in cameras["cues"]}
world_instances = {row["instanceId"]: row for row in world["instances"]}
world_templates = {row["sequenceId"]: row for row in world["templates"]}


LINKS = (
    (
        "editor.cutscene.valtan.entrance",
        "VALTAN_ENTRANCE_CINEMATIC",
        "entrance",
        (
            ("ESTABLISH", 8600, "camera.valtan.entrance.establish", 0, 8600),
            ("ARENA_REVEAL", 5800, "camera.valtan.entrance.arena-reveal", 0, 5800),
            ("HERO_HANDOFF", 10308, "camera.valtan.entrance.hero-handoff", 0, 10308),
        ),
    ),
    (
        "editor.cutscene.valtan.phase2-wall-destruction",
        "VALTAN_ARENA_BREAK_109",
        "phase2",
        (
            ("IMPACT_HOLD", 1100, "camera.valtan.arena-break-109.impact-hold", 600, 500),
            ("WIDE_REVEAL", 2300, "camera.valtan.arena-break-109.wide-reveal", 0, 2300),
            ("RECOVERY", 2700, "camera.valtan.arena-break-109.recovery", 0, 2700),
        ),
    ),
    (
        "editor.cutscene.valtan.roar",
        "VALTAN_SIX_PIZZA_106",
        "roar",
        (
            ("STEP_04", 2800, "camera.valtan.source.wall-break.step-04", 0, 2800),
            ("STEP_05", 5000, "camera.valtan.source.wall-break.step-05", 0, 3924),
        ),
    ),
    (
        "editor.cutscene.valtan.trash",
        "VALTAN_TRASH",
        "trash",
        (
            ("STEP_05", 667, "camera.valtan.source.trash.step-05", 0, 667),
            ("STEP_06", 5707, "camera.valtan.source.trash.step-06", 0, 5707),
        ),
    ),
    (
        "editor.cutscene.valtan.finale",
        "VALTAN_GHOST_DEATH_AUDITION",
        "finale",
        (("STEP_01", 23000, "camera.valtan.source.finale.audition", 0, 23000),),
    ),
)


for cutscene_id, pattern_id, world_suffix, stages in LINKS:
    assert cutscene_id in authoring_cutscenes, f"missing Map Tool cutscene: {cutscene_id}"
    assert cutscene_id in runtime_cutscenes, f"missing runtime cutscene: {cutscene_id}"
    assert (
        authoring_cutscenes[cutscene_id] == runtime_cutscenes[cutscene_id]
    ), f"authoring/runtime cutscene mismatch: {cutscene_id}"

    cutscene = authoring_cutscenes[cutscene_id]
    for instance_id in cutscene["worldInstanceIds"]:
        assert instance_id in world_instances, f"missing world instance: {instance_id}"

    assert pattern_id in gameplay_patterns, f"missing gameplay pattern: {pattern_id}"
    assert pattern_id in presentation_patterns, f"missing presentation pattern: {pattern_id}"
    gameplay_stages = {
        row["stageId"]: row for row in gameplay_patterns[pattern_id]["stages"]
    }
    presentation_stages = {
        row["stageId"]: row for row in presentation_patterns[pattern_id]["stages"]
    }
    for stage_id, stage_duration, cue_id, invocation_offset, invocation_duration in stages:
        assert gameplay_stages[stage_id]["durationMs"] == stage_duration, (
            f"stage duration mismatch: {pattern_id}/{stage_id}"
        )
        invocations = presentation_stages[stage_id].get("cameraInvocations", [])
        invocation = next(
            (row for row in invocations if row["cameraCueId"] == cue_id), None
        )
        assert invocation is not None, f"missing camera invocation: {pattern_id}/{stage_id}/{cue_id}"
        assert invocation["trigger"] == "ENTER"
        assert invocation["startOffsetMs"] == 0
        assert invocation["durationPolicy"] == "EXPLICIT"
        assert invocation["durationMs"] >= invocation_offset + invocation_duration
        assert cue_id in camera_cues, f"missing camera cue: {cue_id}"
        assert camera_cues[cue_id]["durationMs"] == invocation["durationMs"]

    expected_world_id = f"world.sequence.instance.valtan.source-preview.{world_suffix}"
    assert expected_world_id in world_instances, f"missing linked world sequence: {expected_world_id}"
    source_instance = world_instances[expected_world_id]
    source_template = world_templates[source_instance["templateId"]]
    assert len(source_template.get("soundTracks", [])) == 1, (
        f"linked source cinematic has no original sound track: {expected_world_id}"
    )
    sound_asset = source_template["soundTracks"][0]["assetId"]
    assert (ROOT / "Client/Bin/Resources" / sound_asset).is_file(), (
        f"linked source cinematic sound asset is missing: {sound_asset}"
    )
    actor_track = next(
        (row for row in source_template["tracks"] if row["slotId"] == "actor"),
        None,
    )
    assert actor_track is not None and actor_track["keys"], (
        f"linked source cinematic has no actor transform track: {expected_world_id}"
    )
    # These are the authored Valtan arena roots, not player-relative preview
    # offsets.  Keeping this range explicit catches a regression back to the
    # ordinary "spawn beside the player" audition path.
    assert 150.0 <= source_instance["position"][0] <= 160.0
    assert -125.0 <= source_instance["position"][2] <= -120.0
    if world_suffix == "entrance":
        assert actor_track["keys"][0]["visible"] is False
        assert any(row["visible"] for row in actor_track["keys"])
        assert actor_track["keys"][-1]["positionOffset"][1] > 10.0
    assert f'pattern.strPatternId == "{pattern_id}"' in level_source, (
        f"Valtan Arena does not select source cinematic for {pattern_id}"
    )
    assert f'addCinema("{world_suffix}")' in level_source or f'selected = "{world_suffix}"' in level_source, (
        f"Valtan Arena does not bind {pattern_id} to {world_suffix}"
    )

for suffix, expected_subtitles in (("entrance", 4), ("trash", 2), ("finale", 2)):
    instance = world_instances[
        f"world.sequence.instance.valtan.source-preview.{suffix}"
    ]
    template = world_templates[instance["templateId"]]
    assert len(template.get("subtitleTracks", [])) == expected_subtitles


wall_cutscene = authoring_cutscenes[
    "editor.cutscene.valtan.phase2-wall-destruction"
]
assert wall_cutscene["durationMs"] == 5500
assert len(wall_cutscene["cameraCuts"]) == 1
wall_shot = authoring_shots[wall_cutscene["cameraCuts"][0]["shotId"]]
wall_keys = wall_shot["cameraTrack"]["keyframes"]
assert wall_shot["cameraTrack"]["durationMs"] == 5500
assert len(wall_keys) == 104
assert wall_keys[0]["timeMs"] == 0 and wall_keys[-1]["timeMs"] == 5500
assert all(a["timeMs"] < b["timeMs"] for a, b in zip(wall_keys, wall_keys[1:]))

# Action Workbench local Play must not depend on the user opening Camera Tool
# first.  MainApp owns the optional tool lifetime and creates its sampler on
# the first active Valtan composition preview before applying the Stage cue.
camera_create = "if (activeValtanSession && nullptr == m_pCameraTool)"
camera_sample = "m_pCameraTool->Sample_CompositionPreview(*local, cameraStatus)"
assert camera_create in main_app_source, (
    "Action Workbench camera preview still has a hidden Camera Tool prerequisite"
)
assert camera_sample in main_app_source, (
    "Action Workbench does not sample the linked Stage camera cue"
)
assert main_app_source.index(camera_create) < main_app_source.index(camera_sample)

# The original cameras use authored arena-space coordinates. Cinematic local
# editing samples the matching source actor track before the environment sample
# consumed by the camera cue.
actor_sample_call = "arena->Debug_SampleActionWorkbenchCinematic("
environment_collect = "CValtan::Collect_StageEnvironmentSamples(currentValtanSamples)"
assert actor_sample_call in main_app_source
assert environment_collect in main_app_source
assert main_app_source.index(actor_sample_call) < main_app_source.index(environment_collect)
assert "CWorldSequencePlayer::Sample_Track(*sequence, *actorTrack" in level_source
assert "previewBoss->Is_LocalPatternAuthoringPreview()" in level_source
assert "m_ActionWorkbenchCinematicPlayer.Play(instanceId, targets)" in level_source
assert "m_ActionWorkbenchCinematicPlayer.Seek_InstanceToMs(" in level_source
assert "m_bActionWorkbenchCinematicSourcePlaying);" in level_source
# The Loader owns one consumable prepared Area.  The product source player
# consumes it during Level initialization; the Workbench must clone the
# already-admitted document instead of trying to consume the stage again.
assert "m_ActionWorkbenchCinematicPlayer.Set_Document(" in level_source
assert "m_SourceCinematicPlayer.Get_Document(), targets" in level_source
assert "m_ActionWorkbenchCinematicPlayer.Load_PreparedArea(" not in level_source
# Map Tool's entrance row contains only main + colorless Valtan.  Unrelated
# Actor64 extraction instances must not appear in local or Server-exact playback.
assert 'constexpr std::array<std::string_view, 1u> entranceCompanions' in level_source
assert '"entrance.colorless" };' in level_source
assert "entrance.actor64." not in level_source
assert 'for (const char* suffix : {"entrance","entrance.colorless"})' in level_source
assert 'constexpr std::array<const char*, 6> suffixes' in level_source
# Source subtitle tracks use the shared product renderer.  Both the Server
# cinematic player and the isolated Workbench player must publish samples.
assert "m_ActionWorkbenchCinematicPlayer.Collect_Subtitles(out);" in level_header
for pattern_id, suffix, stage_id, stage_offset in (
    ("VALTAN_ENTRANCE_CINEMATIC", "entrance", "ESTABLISH", "0u"),
    ("VALTAN_ARENA_BREAK_109", "phase2", "IMPACT_HOLD", "600u"),
    ("VALTAN_SIX_PIZZA_106", "roar", "STEP_04", "0u"),
    ("VALTAN_TRASH", "trash", "STEP_05", "0u"),
    ("VALTAN_GHOST_DEATH_AUDITION", "finale", "STEP_01", "0u"),
):
    contract = f'{{ "{pattern_id}", "{suffix}", "{stage_id}", {stage_offset} }}'
    assert contract in level_source, f"missing actor-clock contract: {pattern_id}"

# Pattern-order validation must use the real Server audition path. The local
# timeline remains available for pause/seek, but it has a distinct label and
# can no longer be mistaken for authoritative verification.
assert 'ImGui::Button("Play (Server Exact)"' in workbench_source
assert 'ImGui::Button(Preview.bPlaying && !Preview.bPaused ?\n\t\t\t"Pause Local Timeline" : "Play Local Timeline")' in workbench_source
assert "Play_ServerVerification(*pPattern, Status)" in workbench_source
assert "Debug_SelectCompletePlayPattern(Pattern.strPatternId)" in workbench_source
assert "Debug_CompletePlaySelected(status)" in workbench_source

# Even the seekable local clone starts from the same stable transform that the
# Server rebuilds for PLAY_PATTERN_ID, never from the transient local player.
placements = {row["placementId"]: row for row in world_gameplay["placements"]}
center = placements["boss.valtan.center"]
assert center["kind"] == "boss"
assert center["archetypeId"] == "BOSS_VALTAN"
assert center["position"] == [156.029999, 22.9975109, -122.059998]
assert center["yawDegrees"] == 225
assert 'BOSS_PLACEMENT_ID =\n\t\t"boss.valtan.center"' in level_source
assert "document.Find(std::string(BOSS_PLACEMENT_ID))" in level_source
assert "replicated local player / camera-right" not in level_source
assert "primary replicated Valtan / camera-right fallback" not in level_source
assert "XMMatrixRotationY(XMConvertToRadians(previewYawDegrees))" in preview_panel_source

print(
    "ok: 5 Map Tool Valtan cutscenes link their source actor tracks and camera "
    "cues to the same Action Workbench clock; Server-exact Play and canonical "
    "arena local placement are wired"
)
