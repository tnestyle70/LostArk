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
    "Camera Cut List",
    "New Cut From Current Camera",
    "Delete Selected Cut",
    "Release Camera / Keep Time",
    "Build_CutRanges",
    "HARD_CUT_BOUNDARY_MS",
    "MAX_CAMERA_KEYFRAME_COUNT",
    "cue.Keyframes.size() - removedCount < 2u",
    "sceneId == reservedSceneId",
    "Hard cut boundary (%u ms). Select a later scene to edit movement speed.",
    "Camera Position List / Saved Scenes",
    "New Cue ID (blank = automatic)",
    "New Cue From Current Camera",
    "Delete Selected Cue",
    "No unused pattern stage",
    "Create_CueFromCurrentCamera",
    "Delete_SelectedCue",
    "Ensure_NewCueBinding",
    "Is_CueBindingUsed",
    "Make_UniqueCueId",
    "camera.cue.auto.",
    "At least one ordinary camera cue must remain.",
    "Release Camera / Keep Time, position the free camera, then create the cue.",
    "Easy Camera Path Capture",
    "Point Interval (ms)",
    "Capture First Point / Start New Path",
    "Capture Next Point",
    "Play Captured Path",
    "Each capture advances time automatically.",
    "First capture replaces this cue's current scene list.",
    "Capture_EasyPathPoint",
    "Play_EasyCapturedPath",
    "Reset_EasyPathCapture",
    "VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM",
    "Capture at least two different points before Play Captured Path.",
    "Next point rejected: the 64-scene limit was reached.",
    "the path would exceed this cue's bound stage duration.",
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
    "Render_CutEditor",
    "Insert_CapturedCut",
    "Delete_SelectedCut",
    "Create_CueFromCurrentCamera",
    "Delete_SelectedCue",
    "Render_EasyPathCapture",
    "Capture_EasyPathPoint",
    "Play_EasyCapturedPath",
    "Reset_EasyPathCapture",
    "m_iEasyPointIntervalMs",
    "m_iEasyCapturedPointCount",
    "m_bEasyPathRecording",
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

death_cue = data["deathCue"]
death_keyframes = death_cue.get("keyframes", [])
death_keyframe_times = [row.get("timeMs") for row in death_keyframes]
require(
    death_cue.get("cueId") == "camera.valtan.clear.wide" and
    death_cue.get("durationMs") == 13967 and
    death_cue.get("interpolation") == "LINEAR" and
    death_cue.get("easing") == "LINEAR" and
    death_keyframe_times == [
        0, 1200, 2400, 3266, 3267, 4700, 6100,
        7266, 7267, 8800, 10300, 11800, 13967,
    ],
    "Valtan clear camera no longer matches the authored 13.967s cut sequence",
)
for before_index, after_index in ((3, 4), (7, 8)):
    before_eye = death_keyframes[before_index]["eye"]
    after_eye = death_keyframes[after_index]["eye"]
    cut_distance_squared = sum(
        (after_eye[axis] - before_eye[axis]) ** 2 for axis in range(3)
    )
    require(
        death_keyframes[after_index]["timeMs"] -
        death_keyframes[before_index]["timeMs"] == 1 and
        cut_distance_squared > 100.0,
        "Valtan clear camera hard-cut boundary was smoothed or collapsed",
    )

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
entrance_idle_orbit = cues.get("camera.valtan.entrance-idle.orbit")
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
require(
    entrance_idle_orbit is not None and
    entrance_idle_orbit.get("patternId") ==
    "VALTAN_ENTRANCE_CINEMATIC_IDLE" and
    entrance_idle_orbit.get("stageId") == "HOLD" and
    entrance_idle_orbit.get("trackingMode") == "BOSS_FACING" and
    entrance_idle_orbit.get("durationMs") == 12000 and
    entrance_idle_orbit.get("transitionInMs") == 300 and
    entrance_idle_orbit.get("transitionOutMs") == 500 and
    entrance_idle_orbit.get("interpolation") == "CATMULL_ROM" and
    entrance_idle_orbit.get("easing") == "LINEAR" and
    [row["timeMs"] for row in entrance_idle_orbit["keyframes"]] ==
    [0, 1500, 3000, 4500, 6000, 7500, 9000, 10500, 12000],
    "second Valtan entrance Idle/orbit camera is not the exact 9-waypoint cue",
)
idle_orbit_center_x = entrance_idle_orbit["trackingOrigin"][0]
idle_orbit_center_z = entrance_idle_orbit["trackingOrigin"][2]
require(
    all(
        (row["eye"][0] - idle_orbit_center_x) ** 2 +
        (row["eye"][2] - idle_orbit_center_z) ** 2 <= 12.0 ** 2
        for row in entrance_idle_orbit["keyframes"]
    ),
    "second Valtan entrance camera leaves the arena-safe 12 m orbit",
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
    "VALTAN_ENTRANCE_CINEMATIC_IDLE",
    "Valtan idle-orbit entrance cinematic is not the first Server-authored pattern",
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
idle_entrance_gameplay = next(
    row for row in gameplay["patterns"]
    if row["patternId"] == "VALTAN_ENTRANCE_CINEMATIC_IDLE"
)
idle_entrance_presentation = next(
    row for row in presentation["patterns"]
    if row["patternId"] == "VALTAN_ENTRANCE_CINEMATIC_IDLE"
)
idle_hold = idle_entrance_presentation["stages"][0]
require(
    idle_entrance_gameplay["compatibilitySelectionWeight"] == 1 and
    idle_entrance_gameplay["invulnerableWhileRunning"] is True and
    idle_entrance_gameplay["stages"][0]["durationMs"] == 12500 and
    idle_entrance_gameplay["stages"][0]["hit"]["shape"]["kind"] == "NONE" and
    all(
        row["patternId"] != "VALTAN_ENTRANCE_CINEMATIC_IDLE"
        for row in gameplay["decisionModel"]["manualAuditions"]
    ) and
    any(
        row["patternId"] == "VALTAN_ENTRANCE_CINEMATIC" and
        row["admissionState"] == "DERIVED_SERVER_PATTERN"
        for row in gameplay["decisionModel"]["manualAuditions"]
    ) and
    idle_hold["animation"]["occurrences"][0]["clip"] ==
    "mesh_idle_battle_1" and
    idle_hold["animation"]["occurrences"][0]["repeatUntilStageEnd"] is True and
    idle_hold["cameraInvocations"][0]["cameraCueId"] ==
    "camera.valtan.entrance-idle.orbit" and
    idle_hold["cameraInvocations"][0]["durationMs"] == 12000,
    "second Valtan entrance is not the automatic Idle/Hold camera pattern",
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
