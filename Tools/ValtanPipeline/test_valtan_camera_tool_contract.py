import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


camera_tool = read("Client/Private/CameraTool.cpp")
camera_header = read("Client/Public/CameraTool.h")
camera_document = read("Client/Private/ValtanCinematicCameraDocument.cpp")
camera_controller = read("Client/Private/ValtanCinematicCameraController.cpp")
camera_controller_header = read("Client/Public/ValtanCinematicCameraController.h")
valtan_level = read("Client/Private/Level_ValtanArena.cpp")
valtan_runtime = read("Client/Private/Valtan.cpp")
main_app = read("Client/Private/MainApp.cpp")
boss_tool = read("Client/Private/BossTool.cpp")
camera_runtime = read("Engine/Private/Camera.cpp")
camera_runtime_header = read("Engine/Public/Camera.h")
gameplay_publisher = read("Tools/GameplayPipeline/Publish-GameplayBalance.ps1")
project = read("Client/Default/Client.vcxproj")
filters = read("Client/Default/Client.vcxproj.filters")

for forbidden in ("CameraCuts.json", "CameraCutDocument", "XMVectorCatmullRom"):
    require(forbidden not in camera_tool + camera_header,
            f"parallel legacy camera runtime leaked into Camera Tool: {forbidden}")

for required in (
    "Stage_CameraDraft",
    "Validate_Draft",
    "currentText != m_strBaselineText",
    "Reserve_UniqueSiblingPath",
    "CREATE_NEW",
    "ERROR_FILE_EXISTS",
    "ERROR_ALREADY_EXISTS",
    "FlushFileBuffers",
    "Parse_Text",
    "FILE_FLAG_DELETE_ON_CLOSE",
    "ReplaceFileW",
    'L".replaced-backup."',
    'L".conflict-recovery."',
    "replacedText != m_strBaselineText",
    "Begin_PresentationOverride",
    "Apply_PresentationPose",
    "End_PresentationOverride",
    "Is_PresentationOverrideOwnedBy",
    "CValtanCinematicCameraController::Sample_Cue",
    "CValtanCinematicCameraController::Apply_CueTracking",
    "CValtanCinematicCameraController::Remove_CueTracking",
    "g_ActorPreviewContext.iLevelIndex != currentLevel",
    "current replicated actor frame",
    "Tracking_Label",
    '"BOSS_FACING"',
    '"PLAYER_BOSS_FRAME"',
    "LookAt Dummy Collider",
    "Capture New Scene",
    "Apply Segment Speed",
    "Prototype_Component_Collider_WorldEntity",
    '"camera.scene.auto."',
    "std::isfinite(desiredMilliseconds)",
    "LookAt Dummy was disabled because",
    "Applied the moving LookAt Dummy",
    "Disable_LookAtDummy",
    "MAX_CAMERA_SHAKE_DURATION_MS",
    "minimumDuration > maximumDuration",
    "Is_ValidAuthoringPose(pose)",
    "MIN_LOOK_AT_DUMMY_RADIUS",
    "MAX_LOOK_AT_DUMMY_RADIUS",
    "Dummy Radius was rejected because it is not finite.",
    "std::isfinite(m_fLookAtDummyRadius)",
):
    require(required in camera_tool, f"Camera Tool contract missing: {required}")

require(camera_tool.count("currentText != m_strBaselineText") == 2,
        "Save must CAS-check exact source bytes before and immediately before replace")
require(camera_tool.count("ReplaceFileW(") == 2,
        "Save must atomically replace with a backup and use the same primitive for rollback")
require("Remove_Temporary(replacedBackup);" in camera_tool and
        "Concurrent source bytes were restored" in camera_tool,
        "Save must verify and recover the exact bytes displaced by atomic replace")
require("Preview pose was rejected; the prior camera pose was restored." in camera_tool,
        "unexpected preview apply failures must return the Tool-owned override")
require("NetworkManager" not in camera_tool and "Send_" not in camera_tool,
        "Camera Tool must not create a Client-local Server replay path")
require("Get_Transform()" not in camera_tool and "Valtan.h" not in camera_tool,
        "BOSS_XZ preview must not sample a local boss GameObject")
require("runtime actor transforms are not sampled by the Tool" not in camera_tool,
        "Camera Tool still advertises the retired base-pose-only preview")
for required in (
    "CAMERA_TOOL_ACTOR_PREVIEW_CONTEXT",
    "Publish_ActorPreviewContext",
    "Clear_ActorPreviewContext",
):
    require(required in camera_header + camera_tool,
            f"typed Camera Tool actor-preview seam missing: {required}")

for required in (
    '"cues"', '"deathCue"', '"PROJECT_AUTHORED"',
    "Is_StablePresentationAssetId(cue.strCueId)",
    "Is_StablePresentationAssetId(deathCue.strCueId)",
    '"BOSS_FACING"', '"PLAYER_BOSS_FRAME"', '"transitionInMs"',
    '"transitionOutMs"',
    '"sceneId"', '"interpolation"', '"CATMULL_ROM"',
    "sceneIds.insert(keyframe.strSceneId).second",
    "cueIds.insert(deathCue.strCueId).second",
    "VALTAN_CINEMATIC_CAMERA_CUE::MAX_TRANSITION_IN_MS",
    "value->Was_FloatingPointToken()",
):
    require(required in camera_document,
            f"v6 serializer/validator contract missing: {required}")

for retired in (
    "skyCues", "VALTAN_CINEMATIC_SKY_CUE", "Resolve_SkyState",
    "VALTAN_CINEMATIC_SKY_LAYER_POLICY", "Ready_ValtanSkyPresentation",
    "Apply_ValtanSkyPresentation", "Reset_ValtanSkyPresentation",
):
    require(retired not in camera_document + camera_controller +
            camera_controller_header + valtan_level,
            f"retired camera-owned sky authority remains: {retired}")

require("'schema','formatVersion','encounterId','provenance','cues','deathCue'" in
        gameplay_publisher and
        "cameraDocument.skyCues" not in gameplay_publisher and
        "$Value.transitionInMs" in gameplay_publisher and
        '"$Context transitionInMs" 0 1000' in gameplay_publisher and
        "$Value.transitionOutMs" in gameplay_publisher and
        '"$Context transitionOutMs" 0 1000' in gameplay_publisher and
        "'sceneId','timeMs','eye','lookAt','fovYDegrees'" in gameplay_publisher and
        "@('LINEAR','CATMULL_ROM')" in gameplay_publisher and
        "$cameraCueIds.Add($deathCueId)" in gameplay_publisher and
        "$deathCue -isnot [pscustomobject]" in gameplay_publisher and
        "Assert-CinematicCameraCueContract" in gameplay_publisher and
        "names an unknown pattern" in gameplay_publisher and
        "names an unknown or ambiguous stage" in gameplay_publisher and
        "duplicates an encounter stage tuple" in gameplay_publisher and
        "formatVersion' 6 6" in gameplay_publisher and
        "@($cameraDocument.cues).Count -gt 32" in gameplay_publisher and
        "$Context easing" in gameplay_publisher and
        "$Context shakeAmplitude" in gameplay_publisher and
        "shake amplitude/duration pair is invalid" in gameplay_publisher and
        "eye and lookAt must differ" in gameplay_publisher,
        "gameplay publisher is not enforcing camera-only formatVersion 6")

require("return Sample_Cue(*m_pActiveCue" in camera_controller,
        "product runtime is not consuming the same public sampler as Camera Tool")
for required in (
    "Resolve_BossFacingPoint",
    "Resolve_PlayerBossFramePoint",
    "Remove_BossFacingPoint",
    "Remove_PlayerBossFramePoint",
    "input.fBossYawDegrees",
    "input.vLocalPlayerPosition",
    "Apply_CueTracking(*m_pActiveCue, input, outPose)",
    "Apply_CueTransition",
    "m_TransitionFromPose = m_LastOutputPose",
    "m_LastOutputPose = outPose",
    "m_iStageIndex == input.iStageIndex - 1u",
    "Sample_BoundedTransition",
    "Begin_ExitTransition",
    "Update_ExitTransition",
    "m_ExitTransitionFromPose = m_LastOutputPose",
    "XMVectorCatmullRom",
    "VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM",
):
    require(required in camera_controller,
            f"dynamic camera frame runtime missing: {required}")
for required in (
    "fBossYawDegrees", "hasLocalPlayerPosition", "vLocalPlayerPosition",
    "static bool_t Apply_CueTracking",
    "static bool_t Remove_CueTracking",
    "static bool_t Sample_BoundedTransition",
    "bool_t Is_ExitTransitionActive() const",
):
    require(required in camera_controller_header,
            f"typed dynamic camera input missing: {required}")
for required in (
    "input.fBossYawDegrees = boss.fYawDegrees",
    "input.vLocalPlayerPosition",
    "input.hasLocalPlayerPosition = true",
    "CCameraTool::Publish_ActorPreviewContext(previewContext)",
    "CCameraTool::Clear_ActorPreviewContext",
    "Update_CinematicCameraExitTransition(fTimeDelta)",
    "m_ValtanCinematicCameraController.Update_ExitTransition",
    "VALTAN_GAMEPLAY_FOLLOW_LOOK_HEIGHT",
    "VALTAN_GAMEPLAY_FOV_Y_DEGREES",
):
    require(required in valtan_level,
            f"Valtan Arena did not feed dynamic camera input: {required}")

for required in (
    "Consume_CameraToolOpenRequest",
    "EnsureDebugTool(DEBUG_TOOL::CAMERA)",
    "m_pCameraTool->Open_Cue",
    "m_pCameraTool->On_LevelChanged",
    "m_pCameraTool->Deactivate",
):
    require(required in main_app, f"MainApp Camera Tool routing missing: {required}")
require(main_app.index("CGameInstance::Get().Update_Engine(fTimeDelta)") <
        main_app.index("m_pCameraTool->Update("),
        "Camera Tool samples actor context before the owning Level publishes it")
require("m_hasCameraToolOpenRequest = true" in boss_tool and
        "Open Camera Tool##" in boss_tool,
        "Boss Tool does not emit the typed camera deep link")

require("higher-priority product cinematic inherits the original saved pose" in camera_runtime,
        "Server cinematic preemption does not preserve the original restore pose")
require(camera_runtime.count("m_PresentationSavedWorld =") == 1 and
        camera_runtime.count("m_fPresentationSavedFovy =") == 1,
        "camera preemption must not replace the pose/FOV captured by the first owner")
require("XMLoadFloat4x4(&m_PresentationSavedWorld)" in camera_runtime and
        "m_fFovy = m_fPresentationSavedFovy" in camera_runtime,
        "the final presentation owner must restore the pose/FOV from before the owner chain")
for required in (
    "AUTHORING_PREVIEW = 10u",
    "REFERENCE_AUDITION = 20u",
    "SERVER_CINEMATIC = 100u",
):
    require(required in camera_runtime_header,
            f"camera presentation priority contract missing: {required}")
require("SERVER_CINEMATIC" in read("Client/Private/Level_ValtanArena.cpp"),
        "Valtan Server cinematic did not acquire the high-priority camera owner")
require("REFERENCE_AUDITION" in read("Client/Private/Level_ValtanArena.cpp"),
        "Camera Tool could preempt Valtan reference-audition state without cleanup")
exit_begin = valtan_level.index(
    "bool_t CLevel_ValtanArena::Update_CinematicCameraExitTransition")
exit_end = valtan_level.index(
    "void CLevel_ValtanArena::End_CinematicCameraOverride", exit_begin)
exit_body = valtan_level[exit_begin:exit_end]
require(exit_body.index("Update_ExitTransition") <
        exit_body.index("Apply_PresentationPose") <
        exit_body.index("End_CinematicCameraOverride"),
        "cinematic exit releases ownership before the final follow pose")

for entry in ("CameraTool.h", "CameraTool.cpp"):
    require(entry in project and entry in filters,
            f"Client project/filter registration missing: {entry}")

data = json.loads(read("Data/Encounters/Valtan/ValtanCinematicCamera.json"))
require(data.get("formatVersion") == 6, "camera source is not formatVersion 6")
require(set(data) == {
    "schema", "formatVersion", "encounterId", "provenance",
    "cues", "deathCue",
}, "camera v6 camera-only root shape changed")
require(isinstance(data["deathCue"], dict) and data["deathCue"],
        "existing death cue must be preserved")

all_cues = list(data["cues"]) + [data["deathCue"]]
scene_ids = []
for cue in all_cues:
    require(cue.get("interpolation") in {"LINEAR", "CATMULL_ROM"},
            f"camera cue interpolation is invalid: {cue.get('cueId')}")
    for scene in cue.get("keyframes", []):
        require(set(scene) == {
            "sceneId", "timeMs", "eye", "lookAt", "fovYDegrees",
        }, f"camera scene shape is invalid: {cue.get('cueId')}")
        scene_ids.append(scene["sceneId"])
require(len(scene_ids) == len(set(scene_ids)),
        "camera scene IDs must be stable and globally unique")

cues = {row["cueId"]: row for row in data["cues"]}
entrance_establish = cues.get("camera.valtan.entrance.establish")
entrance_reveal = cues.get("camera.valtan.entrance.arena-reveal")
entrance_handoff = cues.get("camera.valtan.entrance.hero-handoff")
require(
    entrance_establish is not None and
    entrance_establish.get("patternId") == "VALTAN_ENTRANCE_CINEMATIC" and
    entrance_establish.get("stageId") == "ESTABLISH" and
    entrance_establish.get("trackingMode") == "BOSS_FACING" and
    entrance_establish.get("durationMs") == 8600 and
    entrance_establish.get("interpolation") == "CATMULL_ROM" and
    entrance_establish.get("easing") == "LINEAR",
    "Valtan entrance establishing camera tuple/timing is invalid",
)
require(
    entrance_reveal is not None and
    entrance_reveal.get("patternId") == "VALTAN_ENTRANCE_CINEMATIC" and
    entrance_reveal.get("stageId") == "ARENA_REVEAL" and
    entrance_reveal.get("trackingMode") == "BOSS_FACING" and
    entrance_reveal.get("durationMs") == 5800 and
    entrance_reveal.get("interpolation") == "CATMULL_ROM" and
    entrance_reveal.get("easing") == "LINEAR",
    "Valtan entrance arena-reveal camera tuple/timing is invalid",
)
require(
    entrance_handoff is not None and
    entrance_handoff.get("patternId") == "VALTAN_ENTRANCE_CINEMATIC" and
    entrance_handoff.get("stageId") == "HERO_HANDOFF" and
    entrance_handoff.get("trackingMode") == "BOSS_FACING" and
    entrance_handoff.get("durationMs") == 4467 and
    entrance_handoff.get("transitionOutMs") == 1000 and
    entrance_handoff.get("interpolation") == "CATMULL_ROM" and
    entrance_handoff.get("easing") == "LINEAR",
    "Valtan entrance hero handoff camera tuple/timing is invalid",
)
wide_reveal = cues["camera.valtan.arena-break-109.wide-reveal"]
require(wide_reveal.get("trackingMode") == "BOSS_FACING" and
        wide_reveal.get("transitionInMs") == 250,
        "109 wide reveal is not a fast bounded move into the boss-facing frame")
recovery = cues["camera.valtan.arena-break-109.recovery"]
require(recovery.get("trackingMode") == "PLAYER_BOSS_FRAME" and
        recovery.get("transitionInMs") == 400 and
        recovery.get("transitionOutMs") == 400,
        "109 recovery is not a bounded player/boss-frame transition")
pizza = cues.get("camera.valtan.six-pizza-106.landing")
require(pizza is not None and pizza.get("patternId") == "VALTAN_SIX_PIZZA_106" and
        pizza.get("stageId") == "STEP_03" and
        pizza.get("trackingMode") == "PLAYER_BOSS_FRAME" and
        pizza.get("durationMs") == 1200 and
        "transitionInMs" not in pizza and
        "transitionOutMs" not in pizza,
        "six-pizza STEP_03 landing camera cue tuple/frame is invalid")

presentation = json.loads(read("Data/Valtan/Valtan.presentation.json"))
gameplay = json.loads(read("Data/Valtan/Valtan.gameplay.json"))
require(
    gameplay["decisionModel"]["scriptedSequence"]["patternIds"][0] ==
    "VALTAN_ENTRANCE_CINEMATIC",
    "Valtan entrance cinematic is not the first Server-authored pattern",
)
entrance_gameplay = next(
    row for row in gameplay["patterns"]
    if row["patternId"] == "VALTAN_ENTRANCE_CINEMATIC"
)
require(
    entrance_gameplay["invulnerableWhileRunning"] is True and
    [row["durationMs"] for row in entrance_gameplay["stages"]] ==
    [8600, 5800, 5467] and
    all(row["hit"]["shape"]["kind"] == "NONE"
        for row in entrance_gameplay["stages"]) and
    sum(row["durationMs"] for row in entrance_gameplay["stages"]) == 19867,
    "Valtan entrance Server gate is not an invulnerable, non-damaging 19.867s sequence",
)
entrance_presentation = next(
    row for row in presentation["patterns"]
    if row["patternId"] == "VALTAN_ENTRANCE_CINEMATIC"
)
entrance_invocations = [
    stage["cameraInvocations"][0]
    for stage in entrance_presentation["stages"]
]
require(
    [row["cameraCueId"] for row in entrance_invocations] == [
        "camera.valtan.entrance.establish",
        "camera.valtan.entrance.arena-reveal",
        "camera.valtan.entrance.hero-handoff",
    ] and
    [row["durationMs"] for row in entrance_invocations] ==
    [8600, 5800, 4467] and
    entrance_invocations[-1]["durationMs"] +
    entrance_handoff["transitionOutMs"] ==
    entrance_gameplay["stages"][-1]["durationMs"],
    "Valtan entrance camera invocation and final handoff wall do not close exactly",
)

raid_bgm_begin = valtan_runtime.index("void CValtan::Update_RaidBgm")
raid_bgm_end = valtan_runtime.index(
    "bool_t CValtan::Apply_NetworkState", raid_bgm_begin
)
raid_bgm_body = valtan_runtime[raid_bgm_begin:raid_bgm_end]
require(
    'constexpr const char_t* VALTAN_CINEMATIC_ENTRANCE_PATTERN_ID' in
    valtan_runtime and
    '"VALTAN_ENTRANCE_CINEMATIC";' in valtan_runtime and
    'constexpr const char_t* VALTAN_ENTRANCE_PATTERN_ID' in valtan_runtime and
    '"VALTAN_ENTRANCE_WHIRLWIND";' in valtan_runtime,
    "Valtan BGM must distinguish the camera-only entrance from the whirlwind entrance",
)

cinematic_check = "if (isCinematicEntrancePattern)"
entrance_check = "if (isEntrancePattern)"
late_join_check = "if (RAID_BGM_STATE::NONE == m_eRaidBgmState"
require(
    cinematic_check in raid_bgm_body and
    entrance_check in raid_bgm_body and
    late_join_check in raid_bgm_body,
    "Valtan BGM must retain cinematic, entrance, and late-join branches",
)

cinematic_begin = raid_bgm_body.index(cinematic_check)
entrance_begin = raid_bgm_body.index(entrance_check, cinematic_begin)
late_join_begin = raid_bgm_body.index(late_join_check, entrance_begin)
require(
    cinematic_begin < entrance_begin < late_join_begin,
    "The camera-only entrance must return before whirlwind and late-join handling",
)

cinematic_branch = raid_bgm_body[cinematic_begin:entrance_begin]
require(
    "return;" in cinematic_branch and
    "Transition_RaidBgm" not in cinematic_branch and
    "m_hasObservedEntrancePattern" not in cinematic_branch,
    "VALTAN_ENTRANCE_CINEMATIC must preserve Level-owned M04 without entrance state",
)

entrance_branch = raid_bgm_body[entrance_begin:late_join_begin]
require(
    "m_hasObservedEntrancePattern = true;" in entrance_branch and
    "Transition_RaidBgm(RAID_BGM_STATE::M05_INTRO);" in entrance_branch and
    "return;" in entrance_branch,
    "Only VALTAN_ENTRANCE_WHIRLWIND may mark the entrance and start M05",
)

post_late_join_check = "if (RAID_BGM_STATE::M05_INTRO == m_eRaidBgmState"
post_late_join_begin = raid_bgm_body.index(post_late_join_check, late_join_begin)
late_join_branch = raid_bgm_body[late_join_begin:post_late_join_begin]
require(
    "WORLD_ENTITY_ACTION::IDLE != action" in late_join_branch and
    "m_hasObservedEntrancePattern = false;" in late_join_branch and
    "Transition_RaidBgm(RAID_BGM_STATE::M06_PHASE_ONE);" in late_join_branch,
    "A non-idle first normal snapshot must keep late-join semantics and enter M06",
)

post_entrance_branch = raid_bgm_body[post_late_join_begin:]
require(
    "m_hasObservedEntrancePattern" in post_entrance_branch and
    "Transition_RaidBgm(RAID_BGM_STATE::M06_PHASE_ONE);" in
    post_entrance_branch,
    "The first normal snapshot after M05 must advance the sequence to M06",
)
six_pizza = next(row for row in presentation["patterns"]
                 if row["patternId"] == "VALTAN_SIX_PIZZA_106")
landing = next(row for row in six_pizza["stages"]
               if row["stageId"] == "STEP_03")
require(landing["actionId"] ==
        "valtan.sequence.center-six-pizza-charge.step-03",
        "six-pizza landing action tuple drifted")
require(landing["cameraInvocations"] == [{
    "cameraInvocationId": "camera.valtan.six-pizza-106.landing.invocation",
    "cameraCueId": "camera.valtan.six-pizza-106.landing",
    "trigger": "ENTER",
    "startOffsetMs": 0,
    "durationPolicy": "EXPLICIT",
    "durationMs": 1200,
}], "six-pizza landing presentation invocation is not exact")

overlay = json.loads(read(
    "Tools/LevelPlacementExtractor/heartrb_valtan_core_overlay.json"))
runtime_profiles = json.loads(read(
    "Tools/LevelPlacementExtractor/heartrb_environment_runtime.json"))
chaos_prefix = "VALTAN_PHASE_CHAOS_"
require(not any(row["assetId"].startswith(chaos_prefix)
                for row in overlay["assets"] + overlay["placements"]),
        "retired ChaosGate map proxy remains in the authoring manifest")
require(not any(row["assetId"].startswith(chaos_prefix)
                for row in runtime_profiles["profiles"]),
        "retired ChaosGate map proxy profile remains in the runtime manifest")
debug_rows = [row for row in overlay["placements"]
              if row["sourceLevel"] ==
              "DEBUG_REFERENCE_VALTAN_PHASE_SPACEHOLE"]
require(len(debug_rows) == 3 and
        all(row["transformSource"] == "overlay" and
            row["sourcePlacementId"].startswith("debug-reference:")
            for row in debug_rows),
        "Debug reference camera SpaceHole proxies are not exactly segregated")

for relative in (
    "Data/Maps/Imported/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapplacements",
    "Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapplacements",
    "Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapplacements",
):
    placements = read(relative)
    require(chaos_prefix not in placements,
            f"retired Product ChaosGate proxy placement remains: {relative}")
    require(placements.count("DEBUG_REFERENCE_VALTAN_PHASE_SPACEHOLE") == 3,
            f"Debug reference SpaceHole placement identity drifted: {relative}")

for relative in (
    "Data/Maps/Imported/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapassets",
    "Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapassets",
):
    assets = read(relative)
    require(chaos_prefix not in assets,
            f"retired Product ChaosGate proxy asset remains: {relative}")
    require(assets.count('"debug-reference-phase-proxy"') == 3,
            f"Debug reference SpaceHole asset classification drifted: {relative}")

print("test_valtan_camera_tool_contract: PASS")
