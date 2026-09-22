#!/usr/bin/env python3
"""Focused contract for Map Tool Valtan cutscenes linked to Boss patterns."""

from __future__ import annotations

import json
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]

# Execute the actual publisher loop, not a reimplementation of its predicate.
subprocess.run([
    'powershell', '-NoProfile', '-Command', r'''
$ErrorActionPreference = 'Stop'
$tokens = $null; $errors = $null
$ast = [System.Management.Automation.Language.Parser]::ParseFile(
    (Join-Path (Get-Location) 'Tools/GameplayPipeline/Publish-GameplayBalance.ps1'),
    [ref]$tokens, [ref]$errors)
if ($errors.Count) { throw 'Publisher syntax error' }
$loops = @($ast.FindAll({param($node)
    $node -is [System.Management.Automation.Language.ForEachStatementAst] -and
    $node.Variable.VariablePath.UserPath -eq 'keyframe' -and
    $node.Extent.Text.Contains('Cinematic cue does not look at its pattern landing anchor')
}, $true))
if ($loops.Count -ne 1) { throw 'Expected one landing-frame validation loop' }
$validate = [scriptblock]::Create($loops[0].Extent.Text)
$ownsMotion = $true
foreach ($mode in @('WORLD','BOSS_XZ')) {
    $tracking = @{Mode=$mode}
    foreach ($kind in @('LEAP_TO_ANCHOR','LEAP_TO_TARGET')) {
        $anchor = @{Kind=$kind; X=1; Z=2; AnchorId='test.anchor'}
        foreach ($aligned in @($true,$false)) {
            $x = if ($aligned) { 1 } else { 10 }
            $cue = @{cueId='test.cue'; keyframes=@(@{lookAt=@($x,5,2)})}
            $rejected = $false
            try { & $validate } catch {
                if (-not $_.Exception.Message.StartsWith('Cinematic cue does not look')) { throw }
                $rejected = $true
            }
            $expected = $kind -eq 'LEAP_TO_TARGET' -and -not $aligned
            if ($rejected -ne $expected) { throw "Wrong framing admission: $mode/$kind/$aligned" }
        }
    }
    $anchor.Kind = 'LEAP_TO_ANCHOR'
    $cue.keyframes = @(@{lookAt=@(1,2)})
    $rejected = $false
    try { & $validate } catch {
        if (-not $_.Exception.Message.StartsWith('Cinematic keyframe lookAt is malformed')) { throw }
        $rejected = $true
    }
    if (-not $rejected) { throw 'Malformed source lookAt was admitted' }
}
Write-Output 'ok: publisher landing-frame contract (10 cases)'
'''], cwd=ROOT, check=True)

# The source player cannot clone an actor merely because its resource is
# present. The raid must register the factory before Level activation, without
# requiring the user to open Map Tool first.
loader_source = (ROOT / "Client/Private/Loader.cpp").read_text(encoding="utf-8-sig")
valtan_loader = loader_source.split("HRESULT CLoader::Ready_For_ValtanArena()", 1)[1].split(
    "HRESULT CLoader::Ready_For_KakulSaydonArena()", 1
)[0]
assert 'CWorldSequenceObject::PROTOTYPE_TAG' in valtan_loader
assert valtan_loader.index('CWorldSequenceObject::PROTOTYPE_TAG') < valtan_loader.index(
    'CWorldSequencePlayer::Prepare_AreaLoad'
)
assert 'CWorldSequenceObject::Create(m_pDevice, m_pContext)' in valtan_loader
maptool_source = (ROOT / "Client/Private/MapTool_Cutscenes.cpp").read_text(encoding="utf-8-sig")
factory = maptool_source.split('bool_t Client::CMapTool::Ensure_WorldObjectPrototype()', 1)[1].split(
    'const Client::CMapTool::EDITOR_CUTSCENE*', 1
)[0]
assert 'ETOUI(LEVEL::VALTAN_ARENA) == m_iAuthoringLevelIndex' in factory


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
            ("STEP_05", 5000, "camera.valtan.source.wall-break.step-05", 0, 2200),
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
    "ok: 5 cutscene source-player paths and camera invocation IDs are wired; "
    "this structural test does not prove camera geometry or visual fidelity"
)

# Unlike the wiring assertions above, inspect actual source animation chains
# and every millisecond of the two projected roar camera cues.
from sync_valtan_roar_pattern_camera import candidates
expected, evidence = candidates(ROOT)
assert expected["Data/Encounters/Valtan/ValtanCinematicCamera.json"] == cameras
assert expected["Data/Valtan/Valtan.presentation.json"] == presentation
assert len(evidence["animationMatches"]) == 5
assert "sourceStartMs + sequence->durationMs" in level_source
assert "timeMs >= static_cast<f32_t>(sourceSequence->durationMs)" in level_source
assert "input.strStageId = pattern->stages[input.iStageIndex].stageId;" in level_source
source_update = level_source.index("if (!Update_SourceCinematic(input))")
camera_return = level_source.index("if (!hasCameraPose)")
assert source_update < camera_return, "source actor/FX must outlive the last camera cut"
assert "sourceOffsetMs + stageAgeSeconds * 1000.f" in level_source
assert "!changed && m_SourceCinematicPlayer.Try_GetElapsedMs(prefix + selected, previousSourceMs)" in level_source
assert "timeMs = (std::max)(timeMs, previousSourceMs)" in level_source
assert '(input.strStageId == "IMPACT_HOLD" && stageAgeSeconds >= 0.6f)' in level_source
for cue in cameras["cues"]:
    if cue.get("patternId") == "VALTAN_SIX_PIZZA_106" or (
        cue.get("patternId") == "VALTAN_ARENA_BREAK_109" and
        cue.get("stageId") in ("IMPACT_HOLD", "WIDE_REVEAL", "RECOVERY")
    ):
        assert cue.get("trackingMode", "WORLD") == "WORLD"
        assert cue.get("transitionInMs", 0) == cue.get("transitionOutMs", 0) == 0
print("ok: 5 unique animation-chain matches; roar camera geometry <= 2mm / 0.002deg at 1ms; source lifetime bounded")

local_source = level_source.split('bool_t CLevel_ValtanArena::Debug_SampleActionWorkbenchCinematic(', 1)[1].split(
    'void CLevel_ValtanArena::Debug_StopActionWorkbenchCinematic()', 1
)[0]
assert 'const bool_t paused' in local_source
assert 'm_ActionWorkbenchCinematicPlayer.Set_Paused(paused)' in local_source
assert local_source.index('Set_Paused(paused)') < local_source.index('m_ActionWorkbenchCinematicPlayer.Play(')
assert 'targets.bCommitWorldRootEffectsAfterSpawn = true' in local_source
assert 'valtanPreview.iPositionMs, valtanPreview.bPaused' in main_app_source
assert 'activeValtanSession && valtanCinematicSampleReady && local != currentValtanSamples.end()' in main_app_source
assert 'Set_CinematicPreviewStatus(cinematicStatus)' in main_app_source
assert workbench_source.count('m_strCinematicPreviewStatus.c_str()') == 2
runtime_world = load('Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.worldsequences.json')
runtime_instances = {row['instanceId']: row for row in runtime_world['instances']}
runtime_templates = {row['sequenceId']: row for row in runtime_world['templates']}
runtime_objects = {row['objectId']: row for row in runtime_world['objectResources']}
objects = {row['objectId']: row for row in world['objectResources']}
for cutscene_id, _, _, _ in LINKS:
    for instance_id in authoring_cutscenes[cutscene_id]['worldInstanceIds']:
        instance = world_instances[instance_id]
        assert runtime_instances[instance_id] == instance, instance_id
        assert runtime_templates[instance['templateId']] == world_templates[instance['templateId']], instance_id
        for binding in instance['bindings']:
            if binding['targetKind'] == 'OBJECT_RESOURCE':
                assert runtime_objects[binding['targetId']] == objects[binding['targetId']], binding['targetId']
print('ok: Valtan Loader actor factory, Map Tool reuse, pause, same-frame FX, failure camera gate and source payload parity (structural; visual/audio not run)')

# Local Play must resolve the SAME stage bindings and published emitter membership,
# without sending a pretend Server event or moving the authoritative impact time.
wall_events = load('Data/Encounters/Valtan/ValtanWorldEvents.json')
wall_profiles = {p['groupId']: p for p in load(
    'Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestructionpresentation.json')['profiles']}
wall_groups = {g['groupId']: g for g in wall_events['groups']}
wall_projection = {g['groupId']: g for g in load(
    'Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction.json')['groups']}
wall_mutations = {m['mutationId']: m for m in wall_events['mutations']}
gameplay = load('Data/Valtan/Valtan.gameplay.json')
wall_pattern = next(p for p in gameplay['patterns'] if p['patternId'] == 'VALTAN_ARENA_BREAK_109')
offsets = {}
duration = 0
for stage in wall_pattern['stages']:
    offsets[stage['stageId']] = duration
    duration += stage['durationMs']
projected = set()
wall_count = 0
for binding in wall_events['bindings']:
    if not (binding['enabled'] and binding['patternId'] == 'VALTAN_ARENA_BREAK_109'
            and binding['triggerKind'] == 'STAGE_ENTER'):
        continue
    mutation = wall_mutations[binding['mutationId']]
    group = wall_groups[mutation['groupId']]
    profile = wall_profiles[group['groupId']]
    assert profile['mutationId'] == mutation['mutationId']
    projection = wall_projection[group['groupId']]
    assert projection['mutationId'] == mutation['mutationId'] and not projection['removesGround']
    assert set(projection['memberPlacementIds'] + projection['suppressionAliasPlacementIds']) == set(group['memberPlacementIds'])
    members = set()
    for emitter in profile['emitters']:
        ids = [emitter['sourceRuntimePlacementId'], *emitter['suppressionAliasPlacementIds']]
        assert not projected.intersection(ids), binding['bindingId']
        projected.update(ids)
        members.update(ids)
        assert offsets[binding['stageId']] + binding['offsetMs'] == 1600
        assert 1600 + emitter['lifetimeSeconds'] * 1000 <= duration
        wall_count += 1
    assert members == set(group['memberPlacementIds']), binding['bindingId']
assert 0 < wall_count <= 256 and len(projected) <= 256
assert 'Debug_PrepareActionWorkbenchDestruction(Pattern, status)' in workbench_source
assert main_app_source.index('Debug_SampleActionWorkbenchDestruction(') < main_app_source.index('Debug_SampleActionWorkbenchCinematic(')
replication = (ROOT / 'Client/Private/ClientReplication.cpp').read_text(encoding='utf-8-sig')
for start, end, apply_name in [
    ('bool Client::CClientReplication::Apply_WorldDestructionFullSync(', 'bool Client::CClientReplication::Apply_EncounterPropSync(', 'Apply_Full('),
    ('bool Client::CClientReplication::Apply_WorldDestructionDelta(', 'm_WorldDestructionDiagnostics = delta.Diagnostics;', 'Apply_Delta('),
]:
    block = replication.split(start, 1)[1].split(end, 1)[0]
    assert block.index('beforeWorldDestructionProjection(status)') < block.index(apply_name)
controller = (ROOT / 'Client/Private/DestructionSimulationController.cpp').read_text(encoding='utf-8-sig')
external = controller.split('::Sample_ExternalTime(', 1)[1].split('::Step_Once()', 1)[0]
assert 'targetStep < currentStep' in external and 'currentStep < targetStep' in external
physics = (ROOT / 'Client/Private/DestructionSimulationRuntime.cpp').read_text(encoding='utf-8-sig')
assert 's_pClockOwner != this' in physics and 's_pClockOwner = nullptr' in physics
print(f'ok: local wall bindings resolve {wall_count} emitters / {len(projected)} source+alias placements at 1600ms; cleanup precedes Server projection (structural, not visual)')
