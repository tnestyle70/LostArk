# 2026-09-03 저작 툴체인 통합 · 병목 제거 · 신규 도구 구현 계획서

작성 기준 브랜치 `GB/valtan-bugfix-koukusaydon-pattern`,
HEAD `063e1a1a`(= `origin/main`), worktree에 codex 세션 미커밋 변경 48개 파일 + 미추적 2개.
이 문서는 구현 계획서다. 사용자 요구사항 전수, 현재 실측, 변경 파일, 호출 흐름, G별 범위와 검증만
소유한다. 실제 반영 결과와 실행 증거는 같은 폴더의
`2026-09-03_TOOLCHAIN_UNIFICATION_AND_BOTTLENECK_REMOVAL_RESULT.md`가 소유한다.

읽은 정본: `AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, `.md/GB/gotchas.local.md`,
`.md/GB/계획서하네스규칙.local.md`, `.md/GB/local.md`, `.md/TEAM/README.md`,
`09-02` PLAN/RESULT 6종, `09-03` PLAN/RESULT/AUDIT 4종.

## 0. 요구사항 전수표

사용자 원문을 의미 단위로 나눈 것이다. `실측`은 이 세션이 코드·데이터·실행 로그에서 직접 확인한
현재 상태이고, `처리`는 이번 변경 단위에서 어디까지 닫는지다.

| ID | 요구 | 실측 | 담당 G | 처리 |
|---|---|---|---|---|
| R01 | Boss Tool에서 pattern 트리가 안 보인다 | 09-03 codex 세션 직후는 `split authoring Product drift`로 canonical admission이 실패해 Boss/All Effects/Composition 세 화면이 함께 비었다. 현재 worktree는 PublishV2가 실행된 상태이며 `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`가 `ok:true, managedPatterns 42`다. 남은 위험은 native C++ reader가 새 `transitionPursuitMs`·`VALTAN_FIST_IN_OUT_LARGE`를 읽는지이며, Product build만으로는 증명되지 않는다 | G00, G05 | Core profile의 `ValtanPatternAuditionServiceHarness`로 현재 source/Product를 native로 읽어 PASS까지 확인 |
| R02 | Publisher V2를 돌린다 | 실행 완료. `Validate ok`, GameplayBalance Validate PASS(65 pattern / 280 stage / 52 timeline row), Effect V1 176 / V2 119·139·12·9 PASS | G00 | 완료. 문서에 명령 고정 |
| R03 | 도끼 패턴: 여러 도끼 생성·관리를 로직으로 처리 | 도끼는 Server `SPAWN_COMBAT_OBJECT_VOLLEY`가 살아 있는 플레이어별 추적 도끼 + 아레나 랜덤 4개를 1333ms 간격 3회 만든다. 09-03 G03이 source를 `3/1333/4/14m/1m/36`으로 복구했고 Product도 재투영됐다. Client는 replicated combat object presentation만 만든다. Python oracle 1건(`test_high_jump_clock_and_motion_project_losslessly`)이 구버전 `1/0/4`를 기대해 실패 중 | G00, G07 | oracle 갱신 + Server contract test 실행으로 검증 |
| R04 | Composition Workbench에서 Save 자체가 안 된다 | (a) drift admission 실패는 R01과 같은 원인이라 해소됨. (b) 구조적 병목: loop slot이 있는 42 stage는 Animation lane이 `editable=false`라 Append/이동/교체가 막힘(`ActionCompositionWorkbench.cpp:4055`). (c) canonical 11 pattern은 collider Add/Remove·stageKind가 `manualAuditions` 경계로 잠김(데이터 권한 경계, 의도된 것). (d) Save_Reload 16 gate는 정상 검증 | G02 | (b) 규칙을 "마지막 loop slot 뒤에만 append 금지"로 좁힘. (c)는 권한 경계라 유지하고 화면에 이유를 남김 |
| R05 | 어제 문서 기준 병목 구조 수정이 실제로 반영되지 않았다 | 09-02 RESULT §12는 "Product build PASS 아님"으로 끝났고, 09-03 RESULT는 "PublishV2·validator만 실행, C++ build 미실행"으로 끝났다. 즉 source는 있으나 컴파일·harness·실행 검증이 한 번도 닫히지 않았다. 이번 baseline Product build는 `test_build_domain_pipeline_receipts.py` gate에서 중단됐다(runner가 Core에서 `Publish-ValtanWorldDestruction.ps1 -Mode ContractTest`를 호출하는데 gate는 그 파일명을 중복 publisher로 금지) | G00 | gate를 ContractTest 호출만 허용하도록 교정하고 Product → Core → FullDiagnostic 순서로 실제 통과시킨다 |
| R06 | Boss Tool·Effect Tool 내부 병목, V1/V2 Effect 결합과 grouping | All Effects는 `CEffectResourceCatalog` facade로 V1 document와 V2 group을 같은 `Groups`, V2 leaf를 `Leaves`로 이미 한 화면에 보인다. Workbench Resources 탭은 `V1 Pattern Effects / V2 Authored Effects / V2 Effect Groups` 세 lane이고 group 선택 → `Append V2 Stage Binding`으로 stage에 배치된다(`EffectV2_Catalog.cpp:1171`). 부족한 것은 append 시 clock/anchor/repeat 지정과 group 자식 편집(09-03 G13) | G06 | 현재 가능한 경로를 문서로 고정. G13 확장은 별도 커밋 |
| R07 | Effect·Pattern 추가 후 Boss Tool 패턴 미노출·Restart 불가·Save 불가 등 병목 제거 | 원인은 세 갈래다. (1) drift admission(R01), (2) Save 후 graph reload 뒤 무효 포인터 use-after-free → `abort()`(codex가 `BossTool.cpp:1454`에서 수정, 미컴파일), (3) Flow `transitionPursuitMs` 추가에 따른 Server/Client/Python 계약 불일치(fixture 5건) | G00, G05 | (2)(3)을 컴파일·harness로 닫는다 |
| R08 | Restart·Flow·정본 pattern 일치, Restart로 패턴 재생을 깔끔하게 | Server는 saved canonical sequence의 edge별 wait를 사용하고(`GameRoom.cpp` savedCanonicalSequence), Client Flow 문서는 같은 배열을 저장한다. `pattern flow does not match the Server-active canonical scriptedSequence`는 Client Product와 Server bootstrap revision 불일치 증상이다 | G05 | Product→bootstrap→Server 재시작 순서를 검증 절차로 고정, contract test 통과 |
| R09 | Server entry failed(돌 생성 nav 밖) 반영 검토 | 원인 확정: Six Pizza/Struggling의 연출용 돌기둥 4개 링이 초기 벽 navblocker 안이라 `Stage_BossPatternStageActions`가 `return false` → `Mark_RuntimeFailure` → room not-ready → Server FIN → Lobby `Server entry failed`. codex는 exact owner 튜플 2개를 예외에 추가했다(반영 확인). 남은 구조 결함: 연출 전용 volley 하나의 거절이 방 전체를 죽인다 | G01 | 튜플 예외 유지(팀 gotchas 정책) + `Hits.empty()` 연출 volley의 nav 거절을 "해당 wave만 skip, 진단 보존"으로 바꾼다. Server contract test 추가 |
| R10 | Composition에서 워프 패턴 애니메이션 배치 → box 선택 → box detail로 전체 timeline 조절, Save, box 크기·양옆 화살표·본체 이동 | 타임라인은 이미 pattern-global clock이다(stage base offset 누적, `Build_Timeline`). box 이동(`bAnimationMove`), 오른쪽 trim(`bEndTrimCandidate`), collider 양끝 trim이 구현돼 있다. 워프는 `portalRushMotionEditable`이라 leg 8개 duration이 delay+distance/speed로 파생되며 그 stage 8개가 전부 loop slot이라 R04(b)에 막힌다 | G02, G03 | R04(b) 해소로 워프 STEP_02~09 Animation box가 열린다. duration은 계속 Portal 파생 정책(정본) |
| R11 | Animation/Effect/Sound/Collider/Camera Resource에서 append로 실질 추가 | Animation(Replace/Append Stage Slots), Effect V1/V2 append, Sound append, Collider `+`(BOX)는 있다. Camera는 typed 저장 owner가 없어 read-only/deep-link(gotchas 정본) | G03 | 현재 경계 유지. Camera는 Camera Tool deep-link |
| R12 | 전체 pattern timeline 탭에서 전체 timeline 설정; stage는 counter/all-die/grip 같은 분기·특수 기능만 | 현재 Stage = Server가 소유하는 하나의 action 구간(hit, motion, animation, branch의 단위). 전체 timeline은 stage duration 합이다 | G03, G04 | Pattern Root detail에 "Pattern Timeline" 표(전 stage의 global start/end/duration, duration 편집)를 추가한다. Stage를 branch 전용으로 바꾸는 것은 Server 모델 변경이라 §6에서 권고안을 제시하고 이번엔 구현하지 않는다 |
| R13 | 현재 생성되는 모든 collider를 Composition collider lane에 보이게; type(damage/grip/all-die)별 로직 | collider lane에는 stage `hit` 하나만 올라간다. 보이지 않는 것: `counterProxy`(COUNTER stage), combat-object 판정(도넛/도끼/투사체), CAPTURE의 grab 판정 표시 구분. type은 `DAMAGE/CAPTURE` 두 개가 Server 계약이고 ALL_DIE는 schema에 없다 | G03 | counterProxy·combat-object hit window·CAPTURE를 read-only collider 항목으로 lane에 추가하고 라벨에 type을 붙인다. ALL_DIE는 schema evolution matrix가 필요해 §6에 범위만 기록 |
| R14 | Boss Tool·Composition의 의미 없는 텍스트 제거 | Pattern Root의 BulletText 안내 4줄, 기본 Details의 설명 문단 다수, Boss Tool 상단 안내 | G03, G05 | 실행 판단에 필요 없는 설명 문단 제거. 오류·상태 문자열은 유지 |
| R15 | Sequencer 툴(언리얼 시퀀서 개념) | Map용 `CWorldSequenceToolPanel`(placement/deploy transform track), Camera Tool cue, Composition pattern timeline이 각각 존재하나 한 playhead가 없다 | G11 | 경계 정의(§13) + v0 shell: Composition timeline·Camera invocation·World sequence 목록을 lane으로 모아 한 playhead로 표시, 편집은 owner deep-link |
| R16 | Profiler 툴: worker thread·JSON 파싱 포함 핵심 병목을 ms와 함께 나열 | Engine `CProfiler`는 main-thread 전용 scope stack(`m_OpenScopes` 단일)이고 frame 밖(worker)의 scope는 기록되지 않는다. Client에서 Effect 계열 30여 곳만 scope가 있다. 화면은 overlay(FPS/CPU/GPU)와 Save JSON뿐 | G09 | thread-safe per-thread scope + 완료 시점 frame 귀속, Loader worker/JSON parse/Tool reload/Replication/Level/Render pass scope 추가, 새 `Profiler` 창(top-N 병목표, thread, avg/max/self ms, 장시간 작업 로그) |
| R17 | Rendering Benchmark: 현존 툴 확장, 수치 상세, 언리얼급 튜닝 | `Rendering Workbench`는 SSAO/Bloom/Tone/FXAA/Shadow/Fog 저작만 있다. 측정 수치(draw/GPU ms/p95)와 A/B 기록이 없다. "언리얼 렌더링 퀄리티"는 툴이 아니라 렌더링 기능(PBR/IBL, cascaded shadow, TAA, GI)이 필요한 별도 로드맵이다 | G10 | Benchmark 섹션(N frame 캡처 → CPU/GPU avg/p95/max, draw/instance/pipeline stats, 설정 스냅샷과 함께 기록·JSON 저장), 렌더 pass별 CPU scope. 렌더링 기능 로드맵은 §12에 기록 |
| R18 | ImGui를 서브 모니터로 옮길 때 해상도가 깨지지 않게 | 프로세스가 DPI-unaware라 Windows가 비트맵 확대. `ImGuiConfigFlags_ViewportsEnable`만 켜져 있고 `ImGui_ImplWin32_EnableDpiAwareness()`와 `io.ConfigDpiScaleFonts/Viewports`가 없다. imgui 1.92.8 docking + DX11 backend가 dynamic font atlas(`RendererHasTextures`)를 지원하므로 per-monitor 스케일이 가능하다 | G08 | Engine `CImGuiLayer::Enable_DpiAwareness()` 정적 함수 추가 → Client `wWinMain`이 창 생성 전 호출, `Initialize`에서 두 DPI flag 활성화 |
| R19 | codex 세션 반영분(큰 도넛, 3연속/카운터 이펙트, edge wait, Save abort, counter proxy 판정, 돌기둥 두 세트)을 통합 동작으로 빌드·검증 | 소스·데이터는 있으나 C++ 컴파일·Server contract·native harness·실행이 미검증 | G00 전체 | Product/Core/FullDiagnostic 실제 실행 |

### 요구사항 밖이지만 함께 고정하는 판단

- 화면·소리·gameplay 최종 판정은 사용자 전용이다. 이 문서의 PASS는 build/harness/validator까지다.
- `main` 직접 커밋 없음. 이 브랜치에서 하나의 검증 단위로 commit하고 push는 사용자 지시 후.

## 1. 현재 실측

### 1.1 파이프라인 상태

```text
Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate      ok:true  managedPatterns 42  combatObjects 9
Publish-GameplayBalance.ps1 -Mode Validate              PASS  65 patterns / 280 stages / 52 timeline rows
validate_effect_v2.py                                   PASS  119 authored / 139 bindings / 12 groups / 9 independent
Validate-EffectSources.ps1                              PASS  directSourceCount 176
Invoke-BuildAndRegression -Profile Product (baseline)   FAIL  python gate test_build_domain_pipeline_receipts.py
```

### 1.2 Python contract 실패 전수 (codex 변경이 남긴 stale oracle)

| 테스트 | 실패 | 원인 | 교정 |
|---|---|---|---|
| `test_valtan_camera_tool_contract.py:323` | module import 시 `require` 실패 → 같은 unittest 배치 전체 중단 | scriptedSequence key set이 4개로 고정 | 5번째 key `transitionPursuitMs` 허용 |
| `test_valtan_pattern_tree_contract` 1건 | `sequence[:] = [...]`로 patternIds만 바꿈 | `transitionPursuitMs` 길이 불일치 PipelineError가 먼저 발생 | fixture가 배열 길이를 함께 맞춤 |
| `test_valtan_pattern_master_v2` 2건 | `patternIds.insert/remove` | 동일 | 동일 helper |
| `test_valtan_pattern_master_v2` high-jump | 기대 `count 1 / interval 0 / arenaRandom NONE / max 4` | G03 데이터 복구 전 값 | 정본 `3 / 1333 / RANDOM_NAVIGABLE_CIRCLE 4 / 14.0 / 1.0 / 36`으로 갱신 |
| `test_valtan_pattern_master_v2` combat companion | 8개 object 기대 | `donut-large` 추가로 9개 | 기대 집합 갱신 |
| `test_valtan_pattern_master_v2` 4건 `TypeError NoneType + str` | PowerShell 자식 프로세스의 CP949 한국어 출력 | `subprocess.run(..., encoding="utf-8")`이 디코딩 실패해 stdout이 None | `errors="replace"` |
| `test_valtan_canonical_typed_patch_transaction` 1건 | gameplay-only patch 뒤 `Valtan.presentation.json`도 바뀌길 기대 | presentation은 바이트 동일이라 changed 목록에서 빠짐(정상) | 기대 집합에서 presentation 제거 + 실패 메시지에 실제 경로 출력 |
| `Tools/Build/test_build_domain_pipeline_receipts.py` | runner에 `Publish-ValtanWorldDestruction.ps1` 존재 금지 | runner는 Core에서 `-Mode ContractTest`만 호출(publish 중복 아님) | ContractTest 호출 1회만 허용 |

### 1.3 Server entry failed 인과 (확정)

```text
VALTAN_SIX_PIZZA_106/STEP_01 ENTER -> firstOffsetMs 1000 delayed wave
  -> Apply_BossPatternScheduledSpawnWave (GameRoom.cpp:9318)
  -> Stage_BossPatternStageActions BOSS_RELATIVE RADIAL root 검사 (GameRoom.cpp:9969~10047)
  -> Is_PointWalkableExact false (초기 polarity-0 벽 region 98개 활성)
  -> m_strStatus + return false
  -> world-update.pattern-scheduled-spawn-wave -> Mark_RuntimeFailure -> m_isReady=false
  -> 다음 정상 입력 REJECTED_ROOM_NOT_READY -> session FIN -> Client CLIENT_PEER_CLOSED -> Lobby
```

codex 변경으로 `six-pizza.rock-pillar`, `struggling.rock-pillar` 두 exact owner가 예외에 들어갔고
Server contract test와 Python 튜플 검사가 함께 있다. 이 부분은 반영 확인됐다.

### 1.4 Composition Save 병목 (코드 실측)

```text
ActionCompositionWorkbench.cpp:4055   editable = animationEditable && !Slot.repeatUntilStageEnd
ActionCompositionWorkbench.cpp:2364   Append: 어느 slot이든 loop/0ms면 전체 거부
ActionCompositionWorkbench.cpp:9789   bAnimationMove 는 Item.bEditable 요구
ActionCompositionWorkbench.cpp:9770   end trim 도 Item.bEditable 요구
```

loop slot(`LOOP_TO_STAGE_END`)은 stage 끝까지 늘어나므로 그 **뒤에** 이어 붙일 수 없는 것은 맞다.
그러나 현재 코드는 loop slot 자신을 이동·교체하는 것과 loop slot **앞에** 붙이는 것까지 막는다.

### 1.5 도구 인벤토리

| 영역 | 현재 | 파일 |
|---|---|---|
| Profiler | main-thread scope stack, GPU timestamp/pipeline stats 1개, 1200 frame history, JSON save, overlay | `Engine/Public/Profiler.h`, `Engine/Private/Profiler.cpp`, `Client/Private/ProfilerCaptureIO.cpp`, `MainApp.cpp:6893~6980` |
| Rendering | 품질·씬 프로파일 저작 + Save/Publish/Reload | `MainApp.cpp:6555~6890`, `RenderingProfileService.h` |
| Sequencer 유사 | Map world sequence(placement transform/deploy animation track, play/pause/loop) | `WorldSequenceToolPanel.h` |
| Composition | pattern-global 7 lane timeline(STAGE/ANIMATION/EFFECT/SOUND/LOGIC/COLLIDER/CAMERA), 이동/trim, Details, Resources append, Save 3-owner transaction | `ActionCompositionWorkbench.cpp` 11,755줄 |
| ImGui | docking+viewports, DPI 미인식 | `Engine/Private/ImGuiLayer.cpp:29~36`, `Client/Default/Client.cpp:218` |

## 2. G00 — 파이프라인 잠금 해제와 회귀 oracle 교정

### 2.1 목표와 종료 증거

codex 반영분 전체가 Product → Core → FullDiagnostic를 통과한다. 종료 증거는
`out/BuildPipeline/runs/*-debug-fulldiagnostic-*.json`과 각 harness의 `failures : 0`이다.

### 2.2 수정 파일

| 파일 | 변경 |
|---|---|
| `Tools/Build/test_build_domain_pipeline_receipts.py` | `Publish-ValtanWorldDestruction.ps1`은 `-Mode ContractTest` 1회 호출만 허용 |
| `Tools/ValtanPipeline/test_valtan_camera_tool_contract.py` | sequence key set 4/5 모두 허용 |
| `Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py` | fixture가 `transitionPursuitMs` 길이를 patternIds-1로 동기화 |
| `Tools/ValtanPipeline/test_valtan_pattern_master_v2.py` | 동일 동기화 helper, high-jump 기대값, combat companion 9개, subprocess `errors="replace"` |
| `Tools/ValtanPipeline/test_valtan_canonical_typed_patch_transaction.py` | 기대 changed 집합에서 presentation 제거 |

새 C++ 파일 없음. `.vcxproj` 변경 없음.

### 2.3 검증

```powershell
python -m unittest Tools.ValtanPipeline.test_valtan_camera_tool_contract Tools.ValtanPipeline.test_valtan_pattern_tree_contract Tools.ValtanPipeline.test_valtan_pattern_master_v2 Tools.ValtanPipeline.test_valtan_canonical_typed_patch_transaction
python Tools/Build/test_build_domain_pipeline_receipts.py
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

## 3. G01 — 연출 전용 volley의 navigation 거절을 비치명으로

### 3.1 목표와 종료 증거

`Hits`가 비어 있는 FIXED_AREA 연출 오브젝트(돌기둥·바위)의 boss-relative root가 navigation 밖이면
그 wave 하나만 생성하지 않고 진단을 남긴 채 stage는 정상 진행한다. damage hit가 있는 오브젝트는
기존대로 strict 거절(room failure)이다. 종료 증거는 Server contract test 2건이다.

```text
1. 연출 전용 + 튜플 예외 밖 + off-nav root  -> Apply_BossPatternScheduledSpawnWave true, live object 0,
   room Is_Ready true, m_strStatus에 "leaves navigable arena" 보존
2. 같은 정의에 Hits 1개 주입                 -> false, live/pending 0 (기존 테스트 유지)
```

### 3.2 수정 파일

| 파일 | 변경 |
|---|---|
| `Server/Private/GameRoom.cpp` | `Stage_BossPatternStageActions`의 boss-relative root 루프에서 off-nav 시 `definition->Hits.empty()`이면 `skipVisualVolley` 플래그 후 case `break`. `std::cerr`에 같은 문자열 기록 |
| `Server/Private/ServerGameplayContractTests.cpp` | 위 1번 시나리오 추가(count를 3으로 바꿔 튜플 예외를 빠져나가게 하고 boss를 (0,0)에 둠) |
| `Tools/ValtanPipeline/test_valtan_rock_pillar_group_contract.py` | skip 경로 존재 검사 1건 추가 |

exact owner 튜플 예외는 유지한다. `gotchas.md`의 "모든 visual object를 포괄 허용하거나 좌표를 project/clamp하지
않는다" 정책과 충돌하지 않는다. 좌표를 옮기지 않고, 생성하지 않으며, 원인을 그대로 남긴다.

### 3.3 호출 흐름

```text
Apply_BossPatternScheduledSpawnWave
  -> Apply_BossPatternStageActions(ENTER, waveOrdinal)
     -> Stage_BossPatternStageActions
        case SPAWN_COMBAT_OBJECT_VOLLEY, bossRelativeVolley
          for ordinal: off-nav
            Hits.empty()  -> m_strStatus 설정, cerr, skipVisualVolley=true, loop 탈출
            else          -> m_strStatus 설정, return false   (기존)
          skipVisualVolley -> break (Stage_BossCombatObject 미호출, 부분 commit 없음)
  -> true, ++iAppliedPatternStageSpawnWaveCount (그 wave는 소비된 것으로 처리해 매 tick 재시도하지 않음)
```

## 4. G02 — Composition loop slot 규칙 교정

### 4.1 목표와 종료 증거

loop slot 자신의 이동·교체와 loop slot 앞의 append가 열린다. append는 "마지막 slot이 loop"일 때만 거부한다.
종료 증거는 `Tools/ValtanPipeline/test_action_presentation_workbench_contract.py` 및 workbench regression
oracle 통과와 Client compile이다.

### 4.2 수정 지점

| 위치 | 기존 | 변경 |
|---|---|---|
| `ActionCompositionWorkbench.cpp:4055` | `animationEditable && !Slot.repeatUntilStageEnd` | `animationEditable` (loop slot도 선택·이동·교체 가능; 우측 trim은 9770에서 loop면 계속 제외) |
| `ActionCompositionWorkbench.cpp:2364` | 어느 slot이든 loop/0ms면 append 거부 | 마지막 slot이 loop/0ms일 때만 거부. 앞쪽 loop slot은 append된 exact clip이 그 뒤에 오지 않으므로 허용하지 않는다 → 즉 규칙은 "loop slot 뒤에는 붙일 수 없다"이며 loop slot이 마지막이 아닌 경우는 현재 데이터에 없다(loop는 항상 마지막). 따라서 실제 열리는 것은 이동·교체·우측 trim 제외다 |
| `ActionCompositionWorkbench.cpp:9770` | `Item.bEditable && bTrimOwner` | ANIMATION owner이고 slot이 loop면 end trim 후보에서 제외 |

loop slot의 종료는 Stage clock이 정본이므로 우측 trim 대신 Stage duration(Gap/Loop Stage Length)으로 조절한다.

## 5. G03 — Composition 구조 보강

### 5.1 Pattern Timeline 표

Pattern Root detail에 표를 추가한다.

```text
열: Stage | Role | Global Start | Global End | Duration(ms, 편집) | Animation End Policy | Hit
행: pattern.Stages 순서
합계: 전체 pattern duration
```

Duration 편집은 기존 `CBalanceTool::Get_ValtanStageDraft / Set_ValtanStageDraft`와 `ApplyStageClockPolicy`
(`ActionCompositionWorkbench.cpp:6186~6215`)를 그대로 호출한다. 두 번째 writer를 만들지 않는다.
`durationEditable=false`(portal 파생 등)는 read-only로 표시한다.

### 5.2 Collider lane 전수 표시

`Build_Timeline`의 collider 블록(`:3860~3910`) 뒤에 read-only 항목을 추가한다.

| 항목 | 출처 | 라벨 |
|---|---|---|
| Stage hit | 기존 | `DAMAGE` 또는 `GRAB` 접두 (`strPlayerResponse`) |
| counterProxy | `Stage.CounterProxy` | `COUNTER proxy | radius/arc` 전체 stage 구간, read-only |
| combat object hit | `Stage.CombatObjectEffects` | `OBJECT <archetype> | hit` object 생존 구간, read-only |

라벨 접두가 R13의 "type"이다. `ALL_DIE`는 Server 계약이 없어 §6.3에 범위만 기록한다.

### 5.3 텍스트 정리

제거 대상은 실행 판단에 필요 없는 설명 문단이다. 상태·오류·잠금 사유 문자열은 유지한다.

- Pattern Root: "New Pattern Authoring Coverage" BulletText 4줄과 grab release 안내 문단
- Box Detail 상단 "Pattern Root is whole-pattern metadata..." 문장
- Gameplay Detail의 "Manual Pattern only: retag..." / "EXACT means 0 ms gap..." / "Stage ENTER/EXIT state owned by..." 설명
- Boss Tool 선택 pattern의 "Connections are unverified" 문장은 진단이므로 유지
- Developer Tools 허브의 "Resizable window: drag any edge..." / "Open windows keep rendering; only this explicitly..." 문장

## 6. G04 — Stage 의미 재정의에 대한 판단 (권고, 미구현)

### 6.1 현재 모델

Stage는 Server가 소유하는 하나의 action 구간이다. hit(collider), motion, animation, ENTER/EXIT action,
branch(COUNTER_HIT/TIMEOUT/ANY_PLAYER_GRABBED)가 전부 stage에 붙는다. 전체 pattern timeline은
stage duration의 순차 합이고 Workbench는 이미 그 global clock으로 그린다.

### 6.2 사용자 제안

전체 timeline을 1급 편집 단위로 올리고, Stage는 counter / all-die / grip / falling-axe 같은 특수 분기와
기능만 담당하게 한다.

### 6.3 판단

Stage를 branch 전용으로 바꾸면 Server의 stage-단위 hit/motion/animation 소유권이 사라지고, 한 pattern
안의 여러 collider·animation 구간을 담을 새로운 track 개념이 Server bootstrap, publisher, Client reader,
Workbench에 동시에 필요하다. 이는 protocol/bootstrap version을 올리는 schema evolution이며 gotchas의
consumer closure matrix 10행 전부를 한 변경 단위로 닫아야 한다. 이번 세션 범위에서는 불가하다.

더 나은 방향으로 다음을 권고한다.

```text
1. Stage 는 Server action 구간으로 유지한다 (truth 변경 없음)
2. Workbench 는 전체 pattern timeline 을 1급 편집 화면으로 승격한다 (G03 §5.1 표 + 기존 global lane)
3. Stage 에 semantic tag 를 둔다: NORMAL / COUNTER / GRAB / ALL_DIE / FALLING_AXE / WAIT
   - 이미 있는 것: WINDUP/ACTIVE/RECOVERY/GROGGY/WAIT(sequence role), COUNTER(counterProxy+flag), GRAB(CAPTURE response)
   - 없는 것: ALL_DIE(전멸기). Server 에 "hit 대상 전원 사망" response 를 추가하는 schema evolution 이 필요
4. tag 가 Details 에 보일 필드와 허용 branch 를 결정한다 (counter 는 COUNTER_HIT/TIMEOUT, grab 은 ANY_PLAYER_GRABBED)
5. 특수 stage 는 Composition 에서 + 로 종류를 골라 삽입한다 (지금은 WINDUP/GROGGY/WAIT 삽입만 있다)
```

이렇게 하면 사용자가 원한 "전체 timeline + 특수 stage만 분기" 작업 흐름이 Server 모델을 바꾸지 않고 얻어진다.
ALL_DIE는 `stage.hit.response` enum 확장(DAMAGE/CAPTURE/LETHAL) → publisher → bootstrap v27 → Server
`Apply_PatternHit` → Client HUD 순서의 별도 수직 슬라이스다.

## 7. G05 — Boss Tool 노출·Restart·Flow 일치

- codex의 `BossTool.cpp:1454` Save-후-reload use-after-free 수정과 `transitionPursuitMs` Flow 문서를 컴파일한다.
- Server `savedCanonicalSequence` edge wait와 Client `ValtanPatternFlowDocument`의 배열이 같은 revision을 참조하는지는
  `Server.exe --contract-test`(FullDiagnostic)와 `NetworkProtocolHarness`가 검사한다.
- Boss Tool 화면 텍스트 정리(§5.3)는 이 G에 포함한다.
- 실행 절차: PublishV2 → GameplayBalance Publish → Product build → Server 재시작 → Client. 순서가 어긋나면
  `pattern flow does not match the Server-active canonical scriptedSequence`가 뜬다.

## 8. G06 — Effect V1/V2 결합·grouping 현재 경로

이미 가능한 것:

```text
All Effects        CEffectResourceCatalog facade: V1 document + V2 group -> Groups, V2 leaf -> Leaves
Workbench Resources  V1 Pattern Effects / V2 Authored Effects / V2 Effect Groups 세 lane
배치               group 선택 -> Stage box 선택 -> Stage-local start -> Append V2 Stage Binding
편집               box 드래그(startMs), Duplicate, Remove
Save               Pattern/Sound/EffectV2 3-owner 한 transaction
```

부족한 것(09-03 G13, 이번 미구현): append 시 clock basis/clip occurrence/repeat/anchor/localTransform 지정,
binding 사후 편집, group 자식 편집 패널. 세 항목 모두 저장 owner가 이미 있으므로 새 runtime 없이 확장 가능하다.

## 9. G07 — 도끼 volley 정본 확인

`Valtan.gameplay.json` `VALTAN_HIGH_JUMP/AIRBORNE`:

```text
spawnSchedule  INTERVAL count 3 firstOffsetMs 0 intervalMs 1333
arenaRandom    RANDOM_NAVIGABLE_CIRCLE anchor BOSS_SPAWN_POSITION count 4 radiusM 14.0 heightToleranceM 1.0
maximumTotalObjects 36
```

Server `GameRoom.cpp:9689/10061/9189`가 웨이브 반복, 살아있는 플레이어별 lock, 아레나 랜덤 원점을 실행한다.
Client는 combat object spawn snapshot으로 presentation을 만들 뿐 개수를 결정하지 않는다.
검증은 `test_valtan_pattern_master_v2` high-jump oracle과 FullDiagnostic Server contract다.

## 10. G08 — ImGui DPI

| 파일 | 변경 |
|---|---|
| `Engine/Public/ImGuiLayer.h` | `static void Enable_DpiAwareness();` 추가 |
| `Engine/Private/ImGuiLayer.cpp` | 위 함수가 `ImGui_ImplWin32_EnableDpiAwareness()` 호출. `Initialize`에서 `io.ConfigDpiScaleFonts = true; io.ConfigDpiScaleViewports = true;` |
| `Client/Default/Client.cpp` | `wWinMain`이 `MyRegisterClass` 전에 `Engine::CImGuiLayer::Enable_DpiAwareness()` 호출 |

Engine public header 변경이므로 Product runner로 Engine → Client SDK 반영까지 확인한다.
트레이드오프: 주 모니터가 100%가 아니면 게임 창의 물리 픽셀 크기가 이전과 달라진다(Windows가 더 이상 비트맵 확대하지 않음).
UI layout은 reference resolution 1280×720 기준 보정이 있으므로 hit test는 영향이 없다.

## 11. G09 — Profiler 도구

### 11.1 Engine `CProfiler` 확장

| 항목 | 변경 |
|---|---|
| `FProfilerScopeSample` | `ThreadId`(uint32) 추가 |
| scope stack | `thread_local` open-scope stack. `Begin_Scope`는 stack에 push, `End_Scope`는 완료 sample을 `m_Mutex` 아래 `m_PendingScopes`에 append |
| frame 귀속 | `End_Frame`이 `m_PendingScopes`를 현재 frame으로 옮긴 뒤 commit. worker scope는 끝난 frame에 귀속 |
| enabled 여부 | scope는 `m_Enabled`만 요구하고 `m_FrameActive`를 요구하지 않는다(worker는 frame 밖에서도 끝날 수 있다) |
| 장시간 작업 | duration ≥ 8ms 완료 sample을 `m_LongOperations` ring(256)에 name/thread/ms/frame으로 보존 |
| 집계 | `Get_ScopeAggregates(frames)`: name+thread별 count/inclusive avg/max/self sum |

### 11.2 scope 추가 지점

| 계층 | 위치 | 이름 |
|---|---|---|
| Engine | `CGameInstance::Update_Engine` 각 단계 | `Engine.PriorityUpdate/Update/Physics/PostPhysics/LevelUpdate/LateUpdate` |
| Engine | `CRenderer::Draw` 각 pass | `Render.Shadow/NonBlend/SSAO/Lights/Scene/ScreenPosts/Bloom/Final/UI/Debug` |
| Client | `CLoader::Start_Loading` | `Loader.LevelLoad`, `Loader.EffectPreparation` (worker thread) |
| Client | `CValtanPatternTree::Load_FromAuthoringPaths` | `Json.ValtanPatternTree.Load` |
| Client | `CBossTool::Reload_CanonicalGraph`, `CActionCompositionWorkbench::Reload_Canonical`, `CBalanceTool` reload | `Tool.BossTool.Reload`, `Tool.Composition.Reload` |
| Client | `CClientReplication::Update` | `Replication.Update` |
| Client | `CMainApp` ImGui 렌더 구간 | `ImGui.DeveloperTools` |

### 11.3 새 `Profiler` 창 (`DEBUG_TOOL::PROFILER`)

```text
상단   CPU frame ms / GPU frame ms / FPS / history frames / dropped scopes
표 1   Bottlenecks: name | thread | calls/frame | avg ms | max ms | self ms | % frame  (avg 내림차순, 필터)
표 2   Long operations: 최근 256개 ≥ 8ms (Loader/JSON/Tool reload가 여기 보인다)
표 3   Counters: draw calls / instances / indices / map / texture / navigation
버튼   Enable / Reset history / Save JSON (기존 CProfilerCaptureIO)
```

`ProfilerCaptureIO`는 sample의 `threadId`를 함께 기록한다.

## 12. G10 — Rendering Benchmark

### 12.1 Rendering Workbench에 Benchmark 섹션

```text
입력   capture frames N (기본 300), label
실행   profiler enable -> N frame 수집 -> CPU/GPU avg, p95, max, draw calls avg, instances avg, pipeline PSInvocations avg
기록   run 표 (label | 시각 | N | CPU avg/p95 | GPU avg/p95 | draw | instances | 당시 GlobalQuality 요약)
저장   JSON (`Client/Bin/BenchmarkCaptures/benchmark_*.json`)
```

### 12.2 렌더링 품질 로드맵 (툴 범위 밖)

"언리얼 렌더링 퀄리티"는 측정 도구로 얻어지지 않는다. 현재 파이프라인 `legacy_deferred_v1`
(FP16 Light → SceneHDR → Screen Post → half-res Bloom → Hable/FXAA → UI)에서 다음 기능이 각각 별도 수직 슬라이스다.

```text
1. PBR material + IBL (specular/diffuse probe)      Shader_Deferred, CMaterial, 맵 asset material 계약
2. Cascaded shadow map (현재 단일 2048 ortho)        CShadow, Renderer::Render_Shadow
3. TAA (현재 FXAA)                                    velocity buffer, history RT, jitter
4. SSR / screen-space GI 또는 baked light probe      G-buffer normal/roughness 필요
5. HDR display / ACES tone (현재 Hable)              Render_Final
```

Benchmark 섹션은 이 로드맵 각 단계의 전후 비용을 같은 기준으로 재는 도구다.

## 13. G11 — Sequencer v0와 Composition 경계

### 13.1 경계 정의

```text
Action Composition Workbench   한 boss pattern(actor 1개)의 저작 owner. Animation/Collider/Effect/Sound/Logic 을
                               stage/pattern clock 으로 저장한다. 쓰기 owner.
Sequencer                      여러 owner 의 시간을 한 playhead 로 배열하는 연출 orchestration.
                               actor(boss/player), Camera cue, World sequence(map placement/deploy), Effect 를
                               절대 시간축(ms) 위에 lane 으로 놓는다. v0 는 read-only + deep-link.
                               v1 부터 자체 문서(`Data/Sequences/<id>.sequence.json`)를 소유하고 Composition 은
                               그 안의 actor track 하나로 참조된다.
```

언리얼 비유: Composition = Anim Montage/Notify 편집, Sequencer = Level Sequence.

### 13.2 v0 구현 범위

| 파일 | 내용 |
|---|---|
| `Client/Public/SequencerTool.h`, `Client/Private/SequencerTool.cpp` (신규, `.vcxproj/.filters` 등록) | 창 하나. Source = Composition 선택 pattern. lane: Animation/Collider/Effect/Sound/Logic/Camera 를 Composition timeline item으로 표시. playhead 공유(`Set_PlayheadMs`). 버튼: Open in Composition / Camera Tool / Effect Tool |
| `ActionCompositionWorkbench.h/.cpp` | 읽기 접근자 `Get_TimelineItems()`, `Get_SelectedPatternId()`, `Get_PlayheadMs()`, `Set_PlayheadMs()`, `Get_TimelineTotalMs()` 추가 |
| `MainApp.h/.cpp` | `DEBUG_TOOL::SEQUENCER`, `DEBUG_TOOL::PROFILER` 추가, 허브 버튼, 렌더 디스패치 |

## 14. 실행 순서와 커밋 단위

```text
G00  oracle/gate 교정            -> Python 4 module + build gate PASS
G01  Server 비치명 skip           -> Server contract test 추가
G02  loop slot 규칙               -> workbench python oracle + compile
G08  DPI                          -> Engine/Client compile
G09  Profiler                     -> Engine/Client compile, Profiler 창 존재
G10  Benchmark                    -> compile
G11  Sequencer v0                 -> compile, vcxproj/filters
G03/G05 Composition/Boss 텍스트·collider lane·Pattern Timeline 표 -> compile
Product -> Core -> FullDiagnostic (직렬)
RESULT 작성, git diff --check, 한 커밋
```

## 15. 이 계획서가 하지 않는 것

```text
Client/UI 자율 실행과 화면 캡처, 사용자 서면 판정 없는 visual PASS
Stage 를 branch 전용으로 바꾸는 Server 모델 변경 (§6 권고만)
ALL_DIE response 추가 (schema evolution 별도)
Effect V2 append 옵션 확장과 group 자식 편집 (09-03 G13)
워프 leg 재정합 (09-03 G05, 사용자 A/B 결정 대기)
PBR/CSM/TAA 등 렌더링 기능 (§12.2 로드맵)
Sequencer 자체 문서 저장 (v1)
publish 시점 nav 전수 검사 (09-03 AUDIT 방지 3)
```

## 16. 2026-09-03 추가 요청 (사용자 2차 메시지)

| ID | 요구 | 실측 | 처리 |
|---|---|---|---|
| R20 | Composition collider `+`에서 위치·앵커 설정. 투사체는 투사체 기준으로 날아가야 하고 생성 시간·모양도 Box Detail에서 만들고 debug collider를 튜닝 | Stage hit는 `hit.anchor`(BOSS_CURRENT + forward/right/yaw offset)·shape·timing이 이미 Collider Detail에서 편집된다. 투사체·도넛·도끼의 판정은 stage hit가 아니라 `Valtan.combatobjects.json`의 combat object(MISSILE/FIXED_AREA)가 소유하며 object의 hit clock·shape는 combat-object pipeline이 publish한다. 그 hit는 §5.2로 Collider lane에 `OBJECT …` read-only 항목으로 보이게 했다 | 이번: 가시화(완료). 다음: Workbench에서 combat object를 생성·편집하는 typed writer(archetype, spawn origin/direction, speed, lifetime, hit schedule/shape)를 `SET_COMBAT_OBJECT` patch op로 추가하고 publisher/Server/Client reader를 같은 변경 단위로 닫는다. Server 판정 계약 변경이라 별도 수직 슬라이스 |
| R21 | `.vcxproj`에서 DataFiles 제거(빌드 시간) | `None` 항목 205개는 컴파일되지 않아 빌드 시간에 실질 영향이 없다(MSBuild 평가 수 ms). 다만 Solution Explorer 노출은 팀 계약(`AGENTS.md`, `CLAUDE.md`, gotchas, TEAM 문서 4곳)과 oracle 2건이 고정하고 있었다 | 사용자 재확인(같은 날 2차 메시지)으로 **유지**한다. 변경 없음. 빌드 시간을 실제로 줄이는 것은 `-Profile Product`(하네스 미컴파일)와 domain receipt 재사용이다 |
| R22 | 왼손 grip `upM -0.9` 튜닝: 0 또는 +0.5? 몸통 앵커? | 09-03 G01/G02가 100배 단위 오류와 actionId 조회 실패를 고치기 전에는 `-0.9`가 `-90 단위`로 적용돼 "손에서 떨어져 보이는" 증상이 났다. 수정 후 의미는 손목 local 축에서 캐릭터 발 원점을 아래 0.9 m에 두는 것(손이 가슴 높이를 잡는 형태)이다. Box Detail → 선택 CAPTURE Stage → Collider → Grab (Capture)에 forward/up/right DragFloat가 이미 있다 | 값은 사용자가 새 빌드로 Server+Client를 실행해 육안 조정한다. 권고 시작점: 손이 가슴을 잡게 하려면 `-0.9`~`-1.1`, 겨드랑이면 `-1.3`, 발을 손 높이에 두려면 `0`. 양수는 발이 손 위로 올라가 캐릭터가 손 위에 서는 형태다. 앵커를 몸통(pelvis)으로 바꾸는 것은 `attachmentSlot` 계약 확장이며 offset만으로 같은 결과를 낼 수 있어 권하지 않는다 |
| R23 | Save 시 publish 자동화, 통합 bat | Boss Tool Flow Save는 이미 candidate publish/apply까지 수행하지만 on-disk `Server/Bin/DataFiles`는 Server pre-build 또는 Balance Tool `Publish Server Data`에서만 갱신됐다 | Workbench Save 성공 뒤 `Save_ValtanProduct`(candidate publish/apply) + `Publish_ServerRuntimeSet`(runtime set publish)를 자동 실행(`Publish after Save` 체크박스, 기본 ON). Boss Tool Flow Save와 Balance Tool `Save & Apply` 성공 뒤에도 runtime set publish. 루트 `RunFullPipeline.bat` → `Tools/Build/Run-FullPipeline.ps1`: Validate → PublishV2 → GameplayBalance Publish → Effect V1/V2 validate → `Invoke-BuildAndRegression`(기본 FullDiagnostic). `-DataOnly`는 컴파일 없이 publish까지 |
| R24 | Sequencer 확장: 쿠크세이튼 맵별 셰이더/이펙트/조명 분위기, 조건별 camera 연출, map animation, 캐릭터 애니메이션, screen post, 캐릭터 생성을 하나의 통합 timeline으로 | 각 owner가 이미 존재한다: 조명·fog·post = `RenderingProfileService` scene profile(Level별 activate), map animation = `WorldSequenceDocument/Player`, camera = `ValtanCinematicCameraDocument`(Camera Tool), 캐릭터 pattern = Composition, Effect = V1/V2 catalog, spawn = Server `activateEncounter/activateSpawnGroup` trigger. 한 playhead로 묶는 문서와 재생기가 없다 | §13의 v1 문서 계약을 아래 §16.1로 구체화. 이번 세션은 v0(Composition lane 통합)까지 |
| R25 | `.filters`: `05. Sequencer` 안의 boss logic flow를 Boss Tool 쪽으로, 물리 폴더·카테고리 기준 정리 | `03. Tools\01. Boss`가 비어 있고 BossTool·Balance·Camera·Flow·Tuning 파일이 `03. Tools\03. UI`에, BossLogicFlow·ValtanPatternTree가 Sequencer/Effect 필터에 있었다 | Boss 도메인(BossTool, BossLogicFlow*, ValtanPatternFlow*, ValtanTuningCommandService, ValtanPatternTree, ValtanPatternAuditionService) → `01. Boss`; BalanceTool → `07. Balance`; CameraTool → `08. Camera`; Profiler → `09. Profiler`; Rendering → `10. Rendering`; SequencerTool → `05. Sequencer`. 물리 폴더는 `Client/Public`·`Private` 평면이라 필터가 유일한 카테고리 축이다 |

### 16.1 Sequencer v1 문서 계약 (설계, 미구현)

```text
Data/Sequences/<sequenceId>.sequence.json   schema lostark.sequence, formatVersion 1
  sequenceId, areaId(LV_LUT_MIDNIGHTC_ED 등), durationMs, tracks[]
  track.kind
    ACTOR_PATTERN    { entityRole: BOSS|PLAYER, patternId, startMs }          -> Composition Product (Server Complete Play 또는 local preview)
    CAMERA_CUE       { cueId, startMs }                                        -> ValtanCinematicCameraDocument
    WORLD_SEQUENCE   { templateId, instanceId, startMs }                       -> WorldSequenceDocument/Player (map placement/deploy animation)
    SCENE_PROFILE    { profileId, startMs, transitionMs }                      -> RenderingProfileService::Activate_Profile (맵별 조명/fog/post 분위기)
    SCREEN_POST      { effectAssetId, startMs, stopMs }                        -> Effect V1/V2 screen post occurrence
    EFFECT           { ownerKind: V1_DOCUMENT|V2_GROUP, stableId, anchor, startMs }
    SPAWN            { placementId | groupId, startMs }                        -> Server activateEncounter/activateSpawnGroup (Server 권한, Client는 요청만)
  conditions[]  { kind: HP_BAR_AT | PHASE_ENTER | TRIGGER_BOX, args }         -> Server encounter 메타데이터와 join, Client 자체 판정 금지
```

- 맵별 분위기는 `SCENE_PROFILE` track이 소유한다. 쿠크세이튼 SL01~SL05는 `Data/Rendering/Authored`에 stage별 profile을 두고 sequence가 시각에 따라 activate한다. 셰이더 기법(SSAO/Bloom/FXAA 등)은 global quality가 아니라 profile 값으로 저장한다.
- 재생기는 owner별 typed adapter 하나씩만 두고 두 번째 runtime을 만들지 않는다. Server 권한(spawn, pattern, damage)은 요청/관측만 한다.
- 검증: schema/ID/시간 범위 validator, 각 adapter의 존재 확인, 실패 시 마지막 admitted sequence 보존. Core gate에 Python contract, 실행형 하네스는 Client `CSequencePlayer`가 adapter를 순서대로 호출하는 contract test.
- 저작: v0 Sequencer 창이 lane 표시를 이미 하므로, v1은 track add/remove/move + Save(parse→validate→stage→commit) + 위 adapter를 추가한다.
