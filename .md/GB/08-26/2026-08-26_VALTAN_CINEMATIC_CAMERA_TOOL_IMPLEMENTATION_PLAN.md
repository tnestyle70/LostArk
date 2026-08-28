# 2026-08-26 발탄 Cinematic Camera Tool 구현 계획

## 1. 목표와 완료 조건

현재 제품 카메라 정본인
`Data/Encounters/Valtan/ValtanCinematicCamera.json ->
CValtanCinematicCameraDocument -> CValtanCinematicCameraController ->
CCamera::Begin/Apply/End_PresentationOverride`를 그대로 사용한다.
별도 `CameraCuts.json`, 별도 컷 재생기, Client local encounter replay는 만들지 않는다.

완료 조건은 다음과 같다.

1. F1 허브의 Camera Tool에서 기존 cue와 keyframe을 stable `cueId`로 선택한다.
2. cue duration/easing/tracking/shake와 key time/eye/lookAt/FOV를 draft로 편집한다.
3. 현재 `CCamera_Free`의 위치·방향·FOV를 선택 key에 Capture할 수 있다.
4. Play/Pause/Stop/Scrub이 제품 Controller와 같은 sampler를 사용하며, Tool 종료·숨김·다른 Tool 전환 시 presentation override를 반환한다.
5. Save는 camera-only formatVersion 5 전체 draft를 strict parse/validate한 뒤 source byte CAS와 sibling temporary atomic replace를 통과할 때만 정본을 교체한다. Reload 실패와 Save 충돌은 기존 draft/파일을 보존한다.
6. Boss Tool camera lane의 `Open Camera Tool`이 MainApp typed intent를 통해 같은 cue를 연다.
7. focused source/roundtrip harness, JSON strict parse, Engine/UpdateLib/Client Debug·Release build와 `git diff --check`를 검증한다. Client는 에이전트가 실행하지 않는다.

## 2. 현재 실측과 폐기 경계

- `CValtanCinematicCameraDocument`는 기존 formatVersion 4에서 camera와 sky payload를 함께 소유했다. World Effect가 Map Effect 정본으로 이전되므로 camera-only formatVersion 5로 breaking migration한다.
- `CValtanCinematicCameraController`는 Server action age, easing, tracking, shake와 late seek를 이미 소유한다.
- `CCamera` presentation override는 시작 시 world/FOV를 저장하고 owner가 종료할 때 복원한다.
- Map Tool Camera panel은 Follow/Free와 offset만 편집하며 cinematic cue authoring은 아니다.
- 2026-08-03의 `CameraCuts.json`/Catmull-Rom 계획은 현재 제품 정본 이전 설계다. 이번 구현에서 파일이나 두 번째 runtime으로 만들지 않는다.

## 3. 수직 흐름

```text
Boss Tool camera cue button
  -> CAMERA_TOOL_OPEN_REQUEST(cueId)
  -> CMainApp::EnsureDebugTool(CAMERA)
  -> CCameraTool::Open_Cue(cueId)

Camera Tool Reload
  -> ProjectDataRoot camera/encounter source read
  -> strict CEncounterPatternReference + CValtanCinematicCameraDocument stage
  -> source byte baseline + draft commit

Edit/Capture
  -> in-memory cue/key draft only
  -> Validate_Draft builds the same cinematic document and strict reparses it

Preview
  -> CValtanCinematicCameraController::Sample_Cue
  -> Level_ValtanArena가 현재 replicated boss/player pose를 typed Debug context로 publish
  -> CValtanCinematicCameraController::Apply_CueTracking
  -> CCamera::Begin_PresentationOverride(tool owner)
  -> Apply_PresentationPose
  -> hide/close/switch/level change/Free: End_PresentationOverride

Save
  -> strict stage
  -> current source byte == baseline CAS
  -> sibling temporary write/flush
  -> strict reparse temporary
  -> MoveFileExW atomic replace
  -> saved document/baseline/draft commit
```

## 4. 변경 파일

```text
Client/Public/CameraTool.h                         신규 Tool/session/typed open 계약
Client/Private/CameraTool.cpp                      ImGui 편집, preview, CAS save/reload
Client/Public/ValtanCinematicCameraDocument.h      같은 문서의 authoring stage/serialize 계약
Client/Private/ValtanCinematicCameraDocument.cpp   strict canonical serializer와 staged validation
Client/Public/ValtanCinematicCameraController.h    제품 sampler의 public static 재사용 입구
Client/Private/ValtanCinematicCameraController.cpp 같은 sampler로 runtime/Tool 일원화
Engine/Public/Camera.h
Engine/Private/Camera.cpp                          owner priority와 선점 체인의 최초 pose 복구
Client/Private/Level_ValtanArena.cpp                Reference/Server cinematic priority 지정
Client/Public/BossTool.h
Client/Private/BossTool.cpp                        typed camera deep-link request 생성
Client/Public/MainApp.h
Client/Private/MainApp.cpp                         F1 Camera Tool 수명, intent routing, deactivation
Client/Default/Client.vcxproj(.filters)             신규 H/CPP 등록
Tools/ValtanPipeline/test_valtan_camera_tool_contract.py
```

formatVersion 5의 정확한 root는 `schema, formatVersion, encounterId, provenance, cues, deathCue`다.
기존 `skyCues`와 여섯 map proxy의 Product controller/Level 경로는 제거한다. Debug reference camera가
실제로 소비하는 SpaceHole 세 장은 `DEBUG_REFERENCE_VALTAN_PHASE_SPACEHOLE` identity로 분리해
Product sky 권위가 아님을 데이터와 코드에서 명시한다.

## 5. 실패·롤백 불변식

- parse/validation 실패는 loaded document와 draft를 교체하지 않는다.
- disk byte가 Reload 당시 baseline과 다르면 Save를 거부하며 외부 변경을 덮어쓰지 않는다.
- temporary validation 또는 atomic replace 실패는 destination을 유지하고 temporary를 정리한다.
- cue/key vector index는 UI 선택 cache일 뿐 저장 identity가 아니다. Reload/deep-link 선택은 `cueId`로 다시 resolve한다.
- preview는 Server pattern을 시작하거나 Camera document runtime cache를 hot swap하지 않는다. 저장 결과는 다음 reload/level entry 입력이다.
- 동적 tracking preview는 Level이 공급한 현재 replicated actor frame만 허용한다. 해당 level/frame 또는 PLAYER_BOSS_FRAME의 local player sample이 없으면 authored base pose를 대신 보여 주지 않고 preview를 거부한다.
- Tool owner가 획득하지 못한 presentation override는 종료하지 않는다.
- Tool이 visible active 상태를 잃으면 다음 프레임까지 기다리지 않고 override를 반환한다.

## 6. 검증

```text
python Tools/ValtanPipeline/test_valtan_camera_tool_contract.py
msbuild Engine/Default/Engine.vcxproj /m /p:Configuration=Debug|Release /p:Platform=x64
Engine/UpdateLib.bat Debug|Release
msbuild Tools/ActionPresentationTimelineHarness/ActionPresentationTimelineHarness.vcxproj /m /p:Configuration=Debug|Release /p:Platform=x64
Tools/ActionPresentationTimelineHarness/Bin/Debug|Release/ActionPresentationTimelineHarness.exe
msbuild Client/Default/Client.vcxproj /m /p:Configuration=Debug|Release /p:Platform=x64
git diff --check
```

수동 화면 검증은 사용자가 Server-approved Valtan Arena에서 수행한다.

```text
F1 -> Boss Tool -> camera cue -> Open Camera Tool
선택 cue 일치 확인
Capture -> key 수치 변경 확인
Scrub/Play/Pause/Stop과 카메라 복구 확인
외부 파일 변경 뒤 Save CAS 거부 확인
정상 Save/Reload 뒤 같은 cue/key 유지 확인
```

사용자 관찰 전에는 camera visual PASS를 기록하지 않는다.

## 7. 2026-08-27 장면 저작 확장

기존 camera-only 정본과 제품 sampler를 유지하면서 다음 기능을 같은 수직 슬라이스에 추가한다.
별도 범용 `CameraCuts.json`이나 두 번째 재생기는 만들지 않는다.

1. camera 문서를 formatVersion 6으로 올리고 모든 keyframe에 stable `sceneId`를 저장한다.
2. cue에 `interpolation: LINEAR | CATMULL_ROM`을 저장한다. 기존 cue는 LINEAR로 이관해 현재 화면을 보존한다.
3. 제품 `CValtanCinematicCameraController::Sample_Cue`가 LINEAR와 clamped Catmull-Rom을 모두 처리한다.
   Eye와 LookAt은 같은 곡선/시간을 사용하고 FOV는 같은 eased alpha로 선형 보간한다.
4. Camera Tool에 debug-only LookAt Dummy sphere를 제공한다. Dummy는 gameplay collision이나 Server
   authority에 등록하지 않고 현재 Level의 debug collider prototype을 복제해 위치만 시각화한다.
5. 선택 scene의 LookAt을 Dummy로 보내거나 Dummy 위치를 scene LookAt으로 적용하고, 현재 pipeline
   camera의 Eye/FOV와 Dummy LookAt을 한 번에 Capture할 수 있게 한다.
6. stable scene list에서 선택, 장면 이동, 현재 pose 교체, 장면 삽입과 interior 장면 삭제를 제공한다.
7. 이동 속도는 authoring-only playback multiplier로 저장하지 않는다. 선택 장면으로 들어오는 구간의
   Eye 이동거리와 목표 world-unit/sec로 `timeMs`를 재계산해 실제 runtime timing에 반영한다.
8. Save는 기존 strict stage, source-byte CAS, temporary reparse, atomic replace 계약을 그대로 사용한다.

추가 검증은 format 6 strict property/scene ID uniqueness, LINEAR 회귀 동일성, Catmull-Rom endpoint와
중간 샘플의 유한성, Tool dummy/capture/speed source contract, publisher Validate와 Debug/Release
ActionPresentationTimelineHarness 및 Client build다. 최종 카메라 경로와 LookAt framing은 사용자가
직접 F1 Camera Tool에서 확인한다.
