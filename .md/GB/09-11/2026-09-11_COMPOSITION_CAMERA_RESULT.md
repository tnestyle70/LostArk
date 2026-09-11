# Composition Camera와 2관문 암전 연결 결과

작성일: 2026-09-11. 브랜치: `codex/kouku-gate1-sequence-playback`.

## G00. 실제 구현 범위

`CKoukuSaydonActionWorkbench::Render_CameraWindow`에 Composition Camera 창을 연결했다.
Action과 Sequence 작업 공간에서 같은 Area shot을 선택하고 이름으로 생성한다. 현재
view를 시간별 Pos/Rot/FOV로 기록하고 `Arrival time ms`로 각 구간 이동 시간을 정한다.
Capture는 동일 시각 key를 교체하고 새 시각은 stable key ID로 추가한다. `Save Camera`,
`Play Camera`, `Append Camera at Cursor`는 기존 Level camera draft와 typed presentation
요청 경로를 소비한다. 사용자 Sequence 데이터와 기존 카메라 action을 일괄 교체하지 않았다.

`VALTAN_CINEMATIC_CAMERA_KEYFRAME`의 optional `up`과 pose의 명시 방향을 camera parser,
serializer, MapTool, publisher, sampler와 실제 camera sink까지 연결했다. 명시 방향은
quaternion 최단 경로로 회전하며 roll을 보존한다. 기존 eye/lookAt key는 기존 보간을
유지한다. 위치는 기존 Linear/Catmull-Rom, 구간 easing은 Linear/Smoothstep/Hold다.
잘못된 key와 저장 충돌은 기존 데이터를 보존한다. 새 C++ 파일은 없다.

## G01. 2관문 원본 암전

기존 `CEffectV2Object::SCREEN_POST_PARAMS`에 optional `intensityKeys`와
`intensitySmoothstep`을 추가했다. 각 key의 `timeSeconds`와 `intensity`는 유한한
float32 값이고 opacity는 0 이상이다. key는 2~64개이며 0초에서 시작해 엄격히 증가한다.
유한 lifetime이 있으면 그 안에 있어야 한다. key 없는 기존 intensityStart/End/Lerp
문서는 동일하게 동작한다. 실제 ScreenPost consumer가 effect local time으로 샘플하며
C++ parse/serialize와 Python V2 publisher validator가 같은 계약을 검사한다.

`kouku.gate2.intro.fade` group은 한 `kouku.gate2.intro.fade.black` leaf를 27000ms 재생한다.
설치된 `Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_blankwhite_01.dds`와 검정 tint를
기존 TexturedOverlay displaySpace 경로로 합성한다. 새 renderer나 camera fade 경로는
없다. native scene04A export610의 8개 fade key는 전부 CIM_CurveAutoClamped이며
arrive/leave tangent가 0이므로 기존 수학식의 zero-tangent cubic인 smoothstep을 사용한다.

| 원본 시간(ms) | opacity |
|---:|---:|
| 0 | 1 |
| 970 | 0 |
| 11660 | 0 |
| 13480 | 1 |
| 14720 | 1 |
| 16290 | 0 |
| 25590 | 0 |
| 27000 | 1 |

새 leaf/group을 V2 Independent 목록과 Client `96.DataFiles` None/project filter에 등록했다.
Sequence의 실제 cue 연결은 컷신 통합 변경이 소유하며 전체 publish/build 결과는
`2026-09-11_KOUKU_GATE_INTRO_COMPOSITION_RESTORE` 결과 문서에 기록한다.

## G02. 실행한 자동 검증

- Camera 수정 5개 TU의 MSVC `/Zs` 검사: 성공. Workbench, Level, CameraTool, MapTool,
  ValtanCinematicCameraController를 검사했다.
- 실제 camera sampler를 컴파일해 nonuniform time, shortest yaw 170→-170, roll 보존,
  entry transition roll, Hold, 기존 key 보간, 잘못된 up 거부 7항목을 통과했다.
- EffectV2_Document/EffectV2_Object MSVC `/Zs`: 성공.
- 실제 EffectV2Document, DataJson, RuntimeAssetRoot, ProjectDataRoot를 컴파일한 headless
  probe: 설치 DDS를 가진 새 leaf parse, 8key/runtime 전달, 원본 모든 key의 opacity,
  quarter sample의 smoothstep, black/clear hold, serializer 왕복, 중복 시간 거부와
  기존 문서 보존, lifetime 초과 거부, 기존 선형 곡선 유지 9항목을 통과했다.
- `python -m unittest Tools.EffectToolV2.test_effect_v2_binding_pipeline.EffectV2BindingPipelineTests -q`:
  20 tests 성공. 기존 canonical owner write 거부 검사 3건의 ERROR 출력은 예상된 거부다.
- 새 fade group의 기존 `_resolve_group` 검사: 1 leaf, 27000ms와 설치 DDS 확인 성공.
- 새 fade JSON 2개, Independent JSON과 Client project/filter XML parse: 성공.
- 해당 변경 `git diff --check`: 성공.

위치: `out/CompositionCamera20260911/verify.ps1`, `verify-fade.ps1`, sampler/fade probe와
각 compile/run log. 별도 제품 하네스를 새로 등록하지 않았다.

전체 binding test module도 한 번 실행했으나 23개 중 legacy repository group 목록의
고정 snapshot 비교 1건이 실패했다. 현재 저장소의 추가된 group들을 반영하지 않은
기존 기대 목록이며 새 fade parser/curve 실패가 아니다. 그 기대 목록이나 다른 group을
이번 변경에서 임의로 수정하지 않았다.

## G03. 사용자 확인 경계

Client와 UI를 실행·조작·캡처하지 않았다. 실제 화면, 저장·재로드 UI 조작과 컷신 visual
fidelity는 사용자 확인 대기다. 사용자는 Server + Client profile을 Ctrl+F5로 실행하고
Lobby → KoukuSaydon → F1 → Action Composition Workbench 또는 Sequencer →
Composition Camera에서 이름 생성 → F6 Free camera로 위치 조정 → Capture arrival ms /
Capture Pos + Rot → Save Camera → Append Camera at Cursor → Composition Save → Play를
사용한다. 카메라 경로의 평균 속도는 패널의 units/s로 확인할 수 있다.
