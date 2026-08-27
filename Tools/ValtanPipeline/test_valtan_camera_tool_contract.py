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
    "Make_TemporaryPath",
    "FlushFileBuffers",
    "Parse_Text",
    "MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH",
    "Begin_PresentationOverride",
    "Apply_PresentationPose",
    "End_PresentationOverride",
    "Is_PresentationOverrideOwnedBy",
    "CValtanCinematicCameraController::Sample_Cue",
    "CValtanCinematicCameraController::Apply_CueTracking",
    "g_ActorPreviewContext.iLevelIndex != currentLevel",
    "current replicated actor frame",
    "Tracking_Label",
    '"BOSS_FACING"',
    '"PLAYER_BOSS_FRAME"',
):
    require(required in camera_tool, f"Camera Tool contract missing: {required}")

require(camera_tool.count("currentText != m_strBaselineText") == 2,
        "Save must CAS-check exact source bytes before and immediately before replace")
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
    "VALTAN_CINEMATIC_CAMERA_CUE::MAX_TRANSITION_IN_MS",
):
    require(required in camera_document,
            f"v5 serializer/validator contract missing: {required}")

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
        "[uint32]$cameraDocument.formatVersion -ne 5" in gameplay_publisher and
        "cameraDocument.skyCues" not in gameplay_publisher and
        "$Value.transitionInMs" in gameplay_publisher and
        '"$Context transitionInMs" 0 1000' in gameplay_publisher and
        "$Value.transitionOutMs" in gameplay_publisher and
        '"$Context transitionOutMs" 0 1000' in gameplay_publisher,
        "gameplay publisher is not enforcing camera-only formatVersion 5")

require("return Sample_Cue(*m_pActiveCue" in camera_controller,
        "product runtime is not consuming the same public sampler as Camera Tool")
for required in (
    "Resolve_BossFacingPoint",
    "Resolve_PlayerBossFramePoint",
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
):
    require(required in camera_controller,
            f"dynamic camera frame runtime missing: {required}")
for required in (
    "fBossYawDegrees", "hasLocalPlayerPosition", "vLocalPlayerPosition",
    "static bool_t Apply_CueTracking",
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
require(data.get("formatVersion") == 5, "camera source is not formatVersion 5")
require(set(data) == {
    "schema", "formatVersion", "encounterId", "provenance",
    "cues", "deathCue",
}, "camera v5 camera-only root shape changed")
require(isinstance(data["deathCue"], dict) and data["deathCue"],
        "existing death cue must be preserved")

cues = {row["cueId"]: row for row in data["cues"]}
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
