# Composition Camera 구현 계획

작성일: 2026-09-11. 기준 브랜치: `codex/kouku-gate1-sequence-playback`.

## G00. 현재 연결과 범위

Composition Resources의 Camera는 `CKoukuSaydonActionWorkbench`가 Area의 stable shot ID를 참조한다. `CLevel_KakulSaydonArena::Sample_CompositionCamera`가 동일 cameraTrack을 `CValtanCinematicCameraController::Sample_Cue`로 재생한다. 현재 Workbench의 Capture는 track을 정적 pose로 교체하고, `Update_CameraShot`은 track을 검증하지 않는다. 별도 Cinematic Camera Tool에는 위치 목록이 있으나 Composition 창에서 시간과 회전을 함께 편집할 수 없다.

이번 구현은 기존 Area JSON과 sampler를 확장한다. 새 제품 runtime과 camera 파일 domain은 만들지 않는다. `Client/Bin/Resources` payload와 사용자가 만든 Sequence JSON은 수정하지 않는다.

## G01. Composition Camera 창

`Client/Public/KoukuSaydonActionWorkbench.h`의 Camera authoring 선언 뒤에 창 렌더 함수를 추가한다. 창 표시, 선택 shot/key, 새 이름과 capture 시간은 Workbench session 값이다. `Client/Private/KoukuSaydonActionWorkbench.cpp`에서 toolbar와 Camera resource의 Open 명령으로 `Composition Camera`를 열고, Create Camera Action은 이름을 입력하는 popup으로 기존 Level draft에 생성한다.

선택 action은 현재 view를 시간(ms)과 함께 Capture하고, Pos/Rot(degrees)/FOV를 편집한다. 첫 key는 0ms, 이후 시간은 증가하고 마지막 key가 duration이다. 위치 간 거리/시간으로 속도를 표시한다. Play와 Append는 기존 presentation request를 사용하며 Save는 Area의 atomic camera save로 연결한다. 잘못된 key나 저장 충돌은 기존 shot과 파일을 보존한다.

## G02. 회전과 저장 계약

`VALTAN_CINEMATIC_CAMERA_KEYFRAME`은 optional `up` 방향과 명시 여부를 추가한다. 기존 eye/lookAt/FOV를 보존하며 Pos/Rot UI는 회전으로부터 lookAt/up을 산출한다. `VALTAN_CINEMATIC_CAMERA_POSE`의 up은 실제 `Apply_PresentationPoseWithUp`까지 전달한다. 명시 up을 가진 구간만 quaternion shortest-path 보간을 사용하고, 기존 key는 현재 eye/lookAt 보간을 유지한다.

`Level_KakulSaydonArena.cpp`의 parse/write/update/save는 전체 cameraTrack을 왕복시킨다. optional up은 유한한 3성분이며 view direction과 평행하지 않아야 한다. `Tools/CompositionPipeline/composition_pipeline.py`의 동일 track 검증을 맞춘다. Map publisher는 기존 source-preserving camera 복사를 유지한다.

## G03. 파일과 검증

기존 Workbench H/CPP, camera cue/controller H/CPP, CameraTool capture, Level camera parse/write/consumer와 composition publisher를 수정한다. 새 C++ 파일은 없으므로 project/filter 항목은 추가하지 않는다. 기존 UTF-8 BOM 없음과 CRLF를 유지한다.

최소 검증은 실제 sampler의 시간·회전·기존 key 회귀, publisher의 optional up/잘못된 방향/시간 검증, 변경 소스 컴파일과 `git diff --check`다. 전체 Product 빌드는 root가 한 번 수행한다. Client/UI는 실행·조작·캡처하지 않는다. 사용자 확인 경로는 F1 → Sequencer/Action Composition Workbench → Composition Camera → Create Camera Action → F6 자유 카메라에서 Capture → 시간/Pos/Rot → Save → Append Camera → Sequencer Play다. 화면과 저장·재로드의 직접 조작 결과는 RESULT에서 사용자 확인 대기로 분리한다.

## G04. 2관문 암전의 기존 Effect 연결

기존 V2 ScreenPost TexturedOverlay가 display-space full-screen quad를 소비하지만 intensity는 두 점 선형 값뿐이다. `SCREEN_POST_PARAMS`에 optional `intensityKeys`(초 단위 timeSeconds/intensity)와 `intensitySmoothstep`을 추가하여 같은 Effect clock으로 샘플한다. key가 없는 기존 문서는 기존 선형값을 유지한다. 키는 2~64개, 첫 시간 0, 엄격 증가, 유한한 0 이상 값, lifetime 이내로 검증한다. C++ parse/serialize와 기존 Python V2 검증에 같은 계약을 반영한다.

27초 `kouku.gate2.intro.fade` group과 한 TexturedOverlay leaf는 설치된 `Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_blankwhite_01.dds`에 검정 tint를 곱한다. source의 0/970/11660/13480/14720/16290/25590/27000ms와 opacity 1/0/0/1/1/0/0/1을 smoothstep으로 보간한다. Sequence 소유 agent가 이 group을 0ms에 연결한다. 새 fade camera runtime이나 무의미한 effect element는 만들지 않는다. 검증은 실제 scalar sampler 수치, V2 parser 왕복과 실패 시 문서 보존, 새 JSON의 기존 V2 publisher 검사, 수정 TU 컴파일로 한정한다.
