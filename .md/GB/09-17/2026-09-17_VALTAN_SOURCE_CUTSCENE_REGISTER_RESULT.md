# 발탄 원본 컷신 5종 등록 시도 결과

작성일: 2026-09-17. 요청: 원본 카메라 + 발탄 애니메이션 시퀀스 5종을 Camera Tool에서 Play로 확인.
동시 요청: 앞서 만든 `컷신 비교`(오리진/내거) UI 제거.

## 1. 작업 A — 컷신 비교 UI 제거 (완료)

제거 대상과 실제 제거량이다.

| 파일 | 제거 |
|---|---|
| `Client/Public/MapTool.h` | 구조체 2개(`CUTSCENE_COMPARE_CUT`, `CUTSCENE_COMPARE_TRACK`), 메서드 선언 5개, 멤버 6개 — 40줄 |
| `Client/Private/MapTool_Cutscenes.cpp` | 선점 가드 4줄 + 비교 함수 4개 343줄, 고아 include 2개(`DataJson.h`, `ProjectDataRoot.h`) |
| `Client/Private/MapTool_CameraShots.cpp` | `Render_CutsceneComparePanel` 구현 65줄 + 호출 1줄 |
| `Client/Private/MapTool.cpp` | Update 구동 3줄 |
| 삭제 | `Data/Encounters/Valtan/Reference/ValtanSourceCameraCuts.reference.json`, `Tools/ValtanPipeline/build_valtan_source_camera_reference.py` |

`grep CutsceneCompare` / `ValtanSourceCameraCuts`가 `Client/`에서 0건이다.
중괄호 균형은 제거 전후 모두 동일(`MapTool_CameraShots.cpp`의 −1은 문자열 리터럴 때문인 기존 값).

**유지한 것**: 통합 컷신 뷰의 Area별 세션 선택 수정은 별개 버그 수정이므로 그대로 뒀다
(`m_pHostedCompositionSession`, `Set_ValtanCompositionSession`, `Get_HostedCompositionSession` 전부 잔존).

## 2. Camera Tool의 Play가 실제로 하는 일

`Client/Private/CameraTool.cpp:1389` `Start` 버튼 → `Apply_PreviewPose()` → `:1119`
`CValtanCinematicCameraController::Sample_Cue`로 **카메라 포즈만** 계산해 적용한다.
보스 애니메이션은 재생하지 않는다. `Stop / Restore`와 `Time (s)` 스크럽이 함께 있다.

`:1126` 비-WORLD tracking은 복제된 발탄 actor 프레임을 요구하지만, 원본은 월드 고정이라
보스가 없어도 재생된다.

카메라와 애니메이션을 함께 재생하는 것은 Camera Tool이 아니라 Valtan Action Workbench의
`Play Authoring Timeline`이며, 그건 pattern/stage 구조를 요구한다.

## 3. 5종 판정

판정 근거가 된 제약은 전부 코드 실측이다.

- `ValtanCinematicCameraDocument.cpp:480` `cue.durationMs > stage.durationMs`면 거부.
  **Encounter 전체에서 가장 긴 stage는 12,000ms**(`VALTAN_STAGGER_SLOT/CHANNEL`).
- `:262` `Read_Fov` fovY 10 미만 거부.
- `:441/516/602` sceneId 문서 전체 유일, 첫 키 0ms, 시각 강한 증가, 키 2~64.
- `deathCue`는 패턴 바인딩이 없고 상한이 `MAX_STAGE_DURATION_MS` = 60,000ms.
- `Tools/ValtanPipeline/test_valtan_camera_tool_contract.py:277,287,297`이
  등장 3 cue의 `trackingMode == "BOSS_FACING"`을 고정.

| 컷신 | 구간 | 키 | fovY | 판정 |
|---|---|---|---|---|
| 1관문 입장(늑대) | 13.000s | 8 | 29.395 | **못 넣음** — 13,000ms > 최대 stage 12,000ms. 새 패턴 필요(금지 범위) |
| 발탄 등장 | 23.386s | 21 | 17.14~29.40 | **못 넣음** — 원본은 월드 고정인데 계약 테스트가 BOSS_FACING을 고정. 테스트 갱신 승인 필요 |
| 발탄 최후 | 23.000s | 31 | 43.00~58.72 | **넣음** — deathCue에 설치 |
| 버러지 | 5.736s | 21 | 11.33~43.00 | **못 넣음** — 현재 패턴과 잇는 근거 없음 |
| 포효 | 5.001s | 16 | **9.611** | **못 넣음** — fovY 하한 10 위반 + 매핑 근거 없음 |

## 4. 설치한 것 — 발탄 최후 → deathCue

`Data/Encounters/Valtan/ValtanCinematicCamera.json`의 `deathCue`만 교체했다.
`cues` 11개와 헤더는 바이트 동일(JSON 비교로 확인).

- `durationMs` 2090 → **23000**
- `interpolation` LINEAR → **CATMULL_ROM** (원본이 전 구간 곡선)
- `easing` SMOOTHSTEP → **LINEAR** (23초 전체에 이징이 걸리면 시간축이 왜곡된다)
- 키 3개 → **28개**, `cueId`는 `camera.valtan.clear.wide` 유지

**한계 — 하드컷이 블렌드가 된다.** 원본은 컷 경계에서 같은 시각에 키가 2개 있는 하드컷인데
v6는 같은 ms 두 키를 금지하고 1ms 흉내도 금지라, 들어오는 컷의 첫 키를 버렸다. 결과:

| 경계 | 블렌드 길이 |
|---|---|
| 3.650s → 5.670s | 2.020s |
| 7.780s → 8.840s | 1.060s |
| 15.410s → 23.000s | **7.590s** |

특히 마지막 cam6 컷은 7.59초 내내 쓸며 이동한다. **원본의 컷 느낌은 재현되지 않는다.**

**부작용**: 이 문서는 전투 중 보스 사망 시에도 쓰인다. 사망 카메라가 2.09초에서 23초로 늘어난다.
되돌리려면 `C:\Users\USER\.claude\jobs\45c8ba77\tmp\bak\ValtanCinematicCamera.json.bak`을 복사하면 된다.

## 5. 발탄 애니메이션 연결 가능성

원본 매티니의 `발탄` 그룹 활성 `interptrackanimcontrol` 클립은 고유 15종이고,
저장소 `Valtan.presentation.json`의 `mesh_` 접두 클립 79종과 대조하면 **13종이 이름 그대로 매칭**된다.

매칭: `abn_groggy_1_start/loop/end`, `idle_battle_1`, `att_battle_5_01_start/end`,
`evt1_att_battle_5_01_end`, `att_battle_13_02`, `att_battle_13_02-1`, `att_battle_17_loop`,
`att_battle_12_04`, `att_battle_12_05`, `dead_1`.
미매칭 2종: `idle_normal_1`, `walk_normal_1` (등장 컷신의 비전투 대기·보행).

즉 **클립 자체는 저장소에 있다.** 다만 Camera Tool은 애니메이션을 재생하지 않으므로
이번 설치로는 카메라만 보인다. 카메라+애니 동시 재생은 Action Workbench 경로이고
pattern/stage 구조가 필요해 위 표의 차단 사유와 같은 벽에 막힌다.

## 6. 검증

- 카메라 문서: 파싱 OK, 파서 규칙 전수 통과(첫 키 0ms, 강한 증가, 마지막==duration, 키 28개,
  fovY 42.9957~58.7155, eye≠lookAt, 키 필드 정확히 5개, sceneId 64개 전부 유일).
- 계약 테스트의 **카메라 문서 단언 전부 재현 통과**(등장 3 cue tuple/timing/tracking, handoff
  transitionOut 1000, wide-reveal 250, recovery PLAYER_BOSS_FRAME/400, 공통 루프).
- `test_valtan_camera_tool_contract.py` 전체 실행은 **195행에서 기존 실패**로 멈춘다:
  `Valtan Arena did not feed dynamic camera input: VALTAN_GAMEPLAY_FOLLOW_LOOK_HEIGHT`.
  해당 심볼은 `Level_ValtanArena.cpp`에 없고 그 파일은 이번에 수정하지 않았다(HEAD 그대로).
  **기존 실패이며 이번 변경과 무관하다.**
- **빌드하지 않았다.** 작업 내내 `devenv.exe`(PID 23600)가 떠 있어 겹쳐 돌리지 않았다.
  C++ 변경이 있으므로 **사용자가 VS에서 Build 해야 한다.**
- 화면 판정은 하지 않았다. 사용자 몫이다.

## 7. 사용자가 확인할 경로

`F1 → Action Workbench → Camera Tool → Source: Valtan` 에서 cue 목록의
`camera.valtan.clear.wide`(deathCue)를 선택 → `Start`.
`Time (s)` 슬라이더로 0~23초를 스크럽해 3.65 / 7.78 / 15.41초 부근을 보면
블렌드 구간이 어떻게 보이는지 판단할 수 있다.

## 8. 남은 결정

1. **1관문 입장**: 13초를 담으려면 새 pattern/stage가 필요하다. 만들지 않았다. 팀장 보류 상태이기도 하다.
2. **발탄 등장**: 계약 테스트의 `trackingMode == "BOSS_FACING"` 단언을 WORLD로 갱신해도 되는지.
   승인되면 3 cue를 8600/5800/4467ms로 재타이밍해 설치할 수 있다(키 21개, fovY 전부 범위 내).
3. **버러지·포효**: 원본 이벤트와 현재 패턴을 잇는 근거가 필요하다. 포효는 그에 더해 fovY 9.611이
   하한 10에 걸려 `Read_Fov`를 바꾸지 않으면 저장 자체가 거부된다.
