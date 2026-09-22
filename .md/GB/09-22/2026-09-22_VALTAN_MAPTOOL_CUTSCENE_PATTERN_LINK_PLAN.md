# 발탄 Map Tool 컷신과 Boss 패턴 연결 검증 계획

## 목표

Map Tool Camera의 발탄 등장, 벽·기둥 파괴, 포효, 버러지, 최후가 기존 Action Workbench Boss 패턴의 정확한 Stage에 연결되고, Action Workbench 로컬 `Play`에서도 실제 카메라가 적용되는지 하나의 실행형 계약으로 고정한다. 이미 존재하는 Camera Cue를 중복 생성하거나 gameplay Stage를 변경하지 않는다.

## 현재 런타임 경계

- Map Tool 컷신 정본: `LV_LUT_HEARTRB_ED.camerashots.json`
- 전투 카메라 정본: `ValtanCinematicCamera.json`
- Stage 호출 정본: `Valtan.presentation.json`의 `cameraInvocations`
- 보스 Stage 정본: `Valtan.gameplay.json`
- 배우·FX·사운드 정본: `LV_LUT_HEARTRB_ED.worldsequences.json`
- 실제 패턴별 World Sequence 선택: `Level_ValtanArena.cpp`
- Action Workbench 로컬 카메라 소비자: `MainApp.cpp` → `CCameraTool::Sample_CompositionPreview`

## 확인된 결함

결함은 네 개였다.

1. Action Workbench의 Stage clock과 Camera Invocation은 정상 생성되지만, `MainApp`이 `m_pCameraTool`이 이미 존재할 때만 카메라 샘플러를 호출했다. 사용자가 독립 Camera Tool 창을 먼저 열지 않으면 객체가 생성되지 않아 Action Workbench `Play`에서 카메라가 조용히 생략됐다.
2. 일반 Boss 패턴의 로컬 Preview는 편집 편의를 위해 발탄 복제본을 플레이어 옆에 둔다. 반면 다섯 원본 컷신 카메라는 발탄 아레나 절대 좌표로 저작됐다. 카메라만 연결하면 카메라는 중앙을 보지만 복제본은 플레이어 옆에 남으므로, 원본 구도와 이동을 편집할 수 없다.
3. Valtan Level 초기화에서 제품용 `m_SourceCinematicPlayer`가 Loader의 단일 prepared Area를 이미 소비한다. Action Workbench player가 같은 `Load_PreparedArea`를 다시 호출해서 항상 실패했고, 결과적으로 원본 배우·FX·원본 WAV는 시작되지 않은 채 일반 Pattern 모델과 원본 월드 카메라만 서로 다른 경로로 재생됐다.
4. Map Tool의 발탄 등장 row는 `entrance`와 `entrance.colorless` 두 instance만 소유하지만 Workbench 코드는 별도 추출물인 Actor64 body/weapon 세 instance까지 추가했다. 또한 제품 자막 수집은 제품용 source player만 읽고 Workbench player는 읽지 않았다.

따라서 컷신 패턴에서는 격리된 Action Workbench용 World Sequence player가 원본 컷신 모델·animation track·effect track·부속 배우를 같은 패턴 clock으로 재생한다. 로컬 복제본에는 원본 `actor` 위치·회전·scale 키만 적용해 Stage 카메라의 추적 root로 사용하고 화면에서는 숨긴다.

추가 실측에서 Preview 창의 기존 `Play`는 Server 패턴 실행이 아니라 `Stage_LocalPatternAuthoringPreview`가 만든 로컬 복제본의 animation/effect 타임라인이라는 사실을 확인했다. 이 경로는 위치를 플레이어 기준 camera-right 3.25m로 만들고 Server movement, combat object authority, world event를 재현하지 않으므로 패턴 순서 검증의 정답으로 사용할 수 없다. Preview 창을 다음 두 계약으로 분리한다.

- `Play (Server Exact)`: 기존 `Debug_CompletePlaySelected` → `CValtanPatternAuditionService` → `PLAY_PATTERN_ID` 경로를 호출한다. Server는 `Gameplay.world.json`의 `boss.valtan.center`에서 보스를 새로 만들고 실제 Stage 실행기를 사용한다.
- `Play Local Timeline`: 저장 전 draft의 pause/seek 편집만 담당한다. 혼동을 막기 위해 로컬임을 이름에 표시하고, 복제본도 플레이어 위치가 아니라 `boss.valtan.center`의 위치와 yaw를 읽어 시작한다. 이 결과는 Server 검증으로 승격하지 않는다.

## 연결 계약

| Map Tool 컷신 | Boss 패턴 | Stage |
|---|---|---|
| `editor.cutscene.valtan.entrance` | `VALTAN_ENTRANCE_CINEMATIC` | `ESTABLISH`, `ARENA_REVEAL`, `HERO_HANDOFF` |
| `editor.cutscene.valtan.phase2-wall-destruction` | `VALTAN_ARENA_BREAK_109` | `IMPACT_HOLD` 600ms 이후, `WIDE_REVEAL`, `RECOVERY` |
| `editor.cutscene.valtan.roar` | `VALTAN_SIX_PIZZA_106` | `STEP_04`, `STEP_05` |
| `editor.cutscene.valtan.trash` | `VALTAN_TRASH` | `STEP_05`, `STEP_06` |
| `editor.cutscene.valtan.finale` | `VALTAN_GHOST_DEATH_AUDITION` | `STEP_01` |

## 구현과 검증

1. 저작본과 게시본에 5개 컷신이 모두 존재하고 구조가 같은지 검사한다.
2. 대상 패턴과 Stage 길이를 검사한다.
3. 각 Stage의 Camera Invocation이 존재하고 실제 Camera Cue ID와 길이가 일치하는지 검사한다.
4. 각 컷신의 World Sequence instance가 존재하는지 검사한다.
5. Valtan Arena가 각 패턴에서 올바른 World Sequence suffix를 선택하는지 검사한다.
6. 신규 Event_02 카메라는 104키, 0~5500ms와 Stage 분할 500+2300+2700ms를 검사한다.
7. Valtan Action Workbench 로컬 재생이 시작되면 독립 Camera Tool 창의 사전 실행 여부와 무관하게 카메라 샘플러를 지연 생성한다.
8. 다섯 컷신 패턴은 격리된 player로 연결된 source-preview instance와 원본 모델·animation/effect track을 재생하고, `actor` track과 시작 Stage/offset을 패턴 clock에 맞춰 샘플링한다.
9. actor transform을 적용한 다음 environment sample과 Stage Camera Invocation을 계산해 보스와 카메라가 같은 월드·시간축을 사용하게 한다.
10. source actor가 실제 화면을 소유하는 동안 로컬 복제본을 숨기고, Stop 또는 일반 패턴 전환 시 source instance를 정리한 뒤 복제본의 이전 transform과 표시 상태를 복원한다.
11. 위 순서와 다섯 actor track 존재 여부를 실행형 회귀 계약으로 검사한다.
12. Preview의 검증 재생은 `Play (Server Exact)` 한 경로로 Server Complete Play를 호출하고, 로컬 편집 재생은 이름과 설명으로 분리한다.
13. 로컬 복제본의 초기 위치와 yaw는 `Gameplay.world.json / boss.valtan.center`에서 읽으며 local player 또는 현재 replicated boss pose를 fallback으로 사용하지 않는다.
14. Action Workbench player는 Loader stage를 두 번째로 소비하지 않고, 이미 검증·admit된 제품 source player 문서를 별도 player에 복제한다.
15. 발탄 등장은 로컬 타임라인·Server Exact·제품 source 재생 모두 Map Tool과 같은 main + colorless 두 instance만 재생하며 Actor64 추출물을 끼워 넣지 않는다.
16. Workbench player의 subtitle sample을 공용 제품 자막 renderer에 함께 제출한다. 원본 WAV는 같은 World Sequence sound track으로 재생한다.

사용자 화면 확인 전에는 visual PASS로 기록하지 않는다.
