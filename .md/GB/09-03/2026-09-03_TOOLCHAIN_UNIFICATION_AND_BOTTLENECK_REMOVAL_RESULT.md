# 2026-09-03 저작 툴체인 통합 · 병목 제거 · 신규 도구 구현 RESULT

브랜치 `GB/valtan-bugfix-koukusaydon-pattern`, HEAD `063e1a1a`(= `origin/main`), 미커밋.
계획은 같은 폴더의 `2026-09-03_TOOLCHAIN_UNIFICATION_AND_BOTTLENECK_REMOVAL_IMPLEMENTATION_PLAN.md`가
소유한다(§16에 2차 요청 추가). 이 문서는 실제 diff와 실행 증거만 기록한다. 화면·소리·gameplay 판정은
사용자 전용이며 여기서 PASS로 기록하지 않는다.

`Client/Bin/Resources` 물리 팩은 사용자가 복구했고 직접 관리하는 런타임 입력이다. 이번 마무리의 수정·publish·
검증 범위에 포함하지 않았으며, 이 문서는 Resources payload의 완전성이나 시각 품질을 PASS로 선언하지 않는다.

## 1. 한 줄 상태

```text
Save/Restart 관련 Python source-contract                         140/140 PASS (현재 워킹트리)
Save/Restart + Effect refresh + presentation 최종 묶음           196/196 PASS (현재 워킹트리)
Run-ValtanAuthoringSaveJob.ps1 parser                            PASS
Effect exact-revision refresh source-contract                     7/7 PASS (현재 워킹트리)
Valtan split Product Validate                                    PASS (source 7cc003f40cbc...)
변경 Client 4 TU 직접 C++ 구문 검사                             PASS (C4819 기존 경고만)
Debug Product                                                     PASS, 단 최신 Save/Restart 통합 전 receipt
Debug FullDiagnostic                                              최신 통합 뒤 미실행
Client 화면·소리·gameplay occurrence                              사용자 수동 확인 대기
```

## 2. 실제로 막고 있던 원인 두 가지 (이번 세션에서 처음 드러남)

두 원인 모두 codex 세션의 데이터 변경(큰 도넛, 도끼 volley 복구)이 Python validator는 통과했지만
C++ 소비자를 갱신하지 않아 생긴 것이다. Python PASS만으로 native admission을 증명할 수 없다는
`gotchas.md`의 consumer closure matrix가 그대로 재현됐다.

### 2.1 R01 Boss Tool/All Effects/Composition 트리가 비는 진짜 원인

`ValtanPatternAuditionServiceHarness`가 현재 source/Product를 native로 읽으면
`Valtan split authoring/Product join failed: master/Product pattern projection changed: VALTAN_FIST_IN_OUT_LARGE`로
전체 canonical graph가 fail-close됐다. 세 화면이 함께 비는 바로 그 경로다.

원인: 큰 도넛 source가 작은 도넛의 `eligibility`(`minimumHealthBarInclusive 1`, `maximumHealthBarInclusive 130`,
`repeatPolicy.limit 1`)를 그대로 복사했다. Python projector는 `DERIVED_SERVER_PATTERN`을 `AUDITION_ONLY`로
투영하며 세 값을 0으로 만들지만 C++ `Equal_MasterPatternGameplay`는 source 값을 그대로 투영해 비교하므로
Product(0/0/0)와 어긋났다. 다른 DERIVED pattern 7개는 모두 source가 0/0/0이다.

조치: `Data/Valtan/Valtan.gameplay.json` 큰 도넛 pattern의 세 값을 DERIVED 관례(0/0/0)로 맞췄다. Product는
이미 0/0/0이라 PublishV2 `changed=0`. 조치 뒤 native harness에서 `Action Composition graph load`,
`Valtan Effect cue native authoring contract`가 PASS로 바뀌었다.

### 2.2 Server가 새 bootstrap을 로드하지 못하던 원인

`Server.exe --contract-test`가 `[STATUS] Valtan high jump does not own the typed AIRBORNE volley` →
`[FAILURE] Load gameplay balance bootstrap`. `Server/Private/GameplayCatalog.cpp`의
`hasExactValtanHighJumpTypedVolley`가 옛 volley(`max 4 / 1 wave / interval 0 / arena NONE`)를 exact로 고정하고
있어, 복구된 정본(`36 / 3 / 1333 / 4 / 14 m / 1 m / BOSS_SPAWN_POSITION`) bootstrap을 Server가 거부했다.
이 상태로 Server를 시작하면 catalog load 실패로 모든 입장이 막힌다.

조치: exact 검사를 복구된 정본으로 갱신. `ServerGameplayContractTests.cpp`의 `highJumpVolleyRow`/excessive
fixture, harness `VerifyMutation("capacity")` marker(`maximumTotalObjects 36 -> 37`)도 같은 값으로 맞췄다.

## 3. 반영한 변경

### 3.1 G00 파이프라인 잠금 해제·회귀 oracle 교정

| 파일 | 변경 |
|---|---|
| `Tools/Build/test_build_domain_pipeline_receipts.py` | runner의 `Publish-ValtanWorldDestruction.ps1 -Mode ContractTest` 1회 호출만 허용. baseline Product build를 첫 단계에서 막던 gate |
| `Tools/ValtanPipeline/test_valtan_camera_tool_contract.py` | scriptedSequence key set 4/5개 모두 허용(import 시점 assert가 unittest 배치 전체를 죽이던 원인) |
| `Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py` | entry-cinematic fixture의 `transitionPursuitMs` 길이 동기화 |
| `Tools/ValtanPipeline/test_valtan_pattern_master_v2.py` | `sync_transition_pursuit()` helper, insert/remove fixture 3곳, high-jump 기대값, combat companion 9개, subprocess `errors="replace"` 39곳, removal fixture가 독립 Effect spawn 소유 pattern을 제외 |
| `Tools/ValtanPipeline/test_valtan_canonical_typed_patch_transaction.py` | gameplay-only overlay 기대 changed 집합에서 presentation 제거 |
| `Tools/ValtanPipeline/test_action_composition_workbench_regression_oracles.py` | loop slot 규칙 oracle을 새 계약으로 교체 |
| `Tools/ValtanPipeline/test_action_presentation_workbench_contract.py` | Pattern Root 안내 문구 oracle을 `Pattern Timeline` 표 + 한 줄 grab-release 한계로 교체 |
| `Tools/EffectToolV2/test_effect_v2_binding_pipeline.py` | BOSS_VALTAN binding 65 → 70(3연속/카운터 6행 추가, shout loop 1행 제거), canonical == admitted 동등 검사 추가 |
| `Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py` | AIRBORNE volley 기대값을 복구 정본으로 |
| `Tools/EffectPipeline/test_valtan_model_view_composition.py` | `combatObjectVisuals` 기대 목록을 현재 9행(donut-large, six-pizza/struggling/part-break 돌기둥, ground-roar V2 group)으로 |
| `Data/Valtan/Valtan.gameplay.json` | 큰 도넛 eligibility를 DERIVED 관례(0/0/limit 0)로 (§2.1) |
| `Server/Private/GameplayCatalog.cpp`, `ServerGameplayContractTests.cpp`, harness `ValtanCanonicalGraphContractTests.cpp` | high-jump volley exact 검사·fixture·oracle을 복구 정본으로 (§2.2) |
| `Server/Private/ValtanBrain.cpp` | codex counter proxy 판정의 `Circles_Overlap({...},{...})` overload 모호성을 `CIRCLE_XZ{}`/`BODY_CIRCLE_XZ{}`로 명시(컴파일 오류) |

### 3.2 G01 Server: 연출 전용 volley의 navigation 거절을 비치명으로

`GameRoom.cpp` `Stage_BossPatternStageActions`: boss-relative root가 navigation 밖일 때 정의가
`Hits.empty() && !PresentationPulses.empty()`이면 진단 문자열을 `m_strStatus`와 `std::cerr`(`[PresentationVolleySkipped]`)에
남기고 그 wave만 `break`한다. 아무것도 stage/commit하지 않고 wave 카운터는 진행돼 매 tick 재시도하지 않는다.
damage hit가 있는 정의는 기존 strict room-failure를 유지하며 codex의 exact owner 튜플 예외 2개는 그대로다.
`ServerGameplayContractTests.cpp`에 "튜플 밖 연출 volley(count 3, radius 500 m)가 skip되고 room이 ready로 남으며
다음 tick 중복 없음" 시나리오를 추가했다.

### 3.3 G02 Composition loop slot 규칙

| 위치 | 변경 |
|---|---|
| `TIMELINE_ITEM` | `bLoopsToStageEnd` 추가 |
| `Build_Timeline` | loop slot box도 `animationEditable`이면 선택·편집. 라벨 ` [loop to Stage end]` |
| append | 마지막 slot이 loop/native-duration일 때만 거부(이전: 어느 slot이든 거부) |
| move | loop slot은 Stage 간 이동 제외 |
| right trim | loop slot 오른쪽 edge 드래그 = Stage duration 편집(`ApplyStageClockPolicy`). portal 파생 등 `durationEditable=false`면 거부 |

### 3.4 G03 Composition 구조 보강

- Pattern Root `Pattern Timeline` 표(Stage/Role/Start/End/Duration 편집/Collider, 합계). 기존 draft 경로만 사용.
- Collider lane: stage hit `DAMAGE`/`GRAB` 접두, `COUNTER proxy`(전체 Stage), hit를 가진 combat object `OBJECT …`(생존 구간) read-only 항목 추가.
- 설명 문단 제거(Workbench 5곳, Boss Tool Logic tab 2줄, Developer Tools 허브 2줄). manual pattern의 grab-release 한계는 한 줄로 유지.

### 3.5 G08 ImGui DPI

`CImGuiLayer::Enable_DpiAwareness()`(Engine public) → `Client.cpp` `wWinMain`이 창 생성 전에 호출.
`Initialize`에서 `io.ConfigDpiScaleFonts = io.ConfigDpiScaleViewports = true`.

### 3.6 G09 Profiler

Engine `CProfiler`: per-thread `thread_local` scope stack, 완료 scope를 End_Frame에서 그 frame에 귀속(worker·frame 밖
scope), `ThreadId`, ≥8 ms long-operation ring 256, `Get_ScopeAggregates`(calls/inclusive/self/max), `Get_WindowFrameStats`.
scope 추가: `Engine.*` 6, `Render.*` 11, `Loader.LevelLoad/EffectPreparation`(worker), `Tool.BossTool.ReloadCanonicalGraph`,
`Tool.Composition.ReloadCanonical`, `Replication.Update`, `ImGui.DeveloperTools`. 새 `Profiler` 창(Bottlenecks/Long operations/
Counters, Save JSON v2). `ValtanPatternTree.cpp`는 Engine 없는 harness가 컴파일하므로 scope를 넣지 않았다.

### 3.7 G10 Rendering Benchmark

`CRenderingBenchmark`(신규): Rendering Workbench `Benchmark` 섹션. N frame 캡처 → CPU/GPU avg/p95/max, draw/instance/index,
PS invocations를 당시 quality summary와 함께 run 표 + `Client/Bin/BenchmarkCaptures/benchmark_*.json`.

### 3.8 G11 Sequencer v0

`CSequencerTool`(신규, `DEBUG_TOOL::SEQUENCER`): Composition 선택 Pattern의 7 lane을 한 playhead로 표시, slider/box 클릭이
`Set_PlayheadMs`로 preview seek 공유, `Open in Composition` deep-link. Workbench에 read-only 접근자 6개 추가, timeline 타입 public.
v1 문서 계약(맵별 scene profile, camera cue, world sequence, screen post, spawn track)은 PLAN §16.1.

### 3.9 Save → exact receipt → publish 상태머신, 통합 파이프라인 (2차 요청 R23)

| 파일 | 변경 |
|---|---|
| `BalanceTool.h/.cpp` | `VALTAN_SAVE_JOB_STATE/RECEIPT`와 job ID 기반 비동기 Save 소유자. Balance patch와 Composition의 Sound/Effect owner bytes를 child 시작 전에 immutable 파일로 고정하고, canonical commit·source manifest·candidate receipt를 단계별 관찰한다. `OBSERVATION_LOST`는 child가 writer를 계속 소유할 수 있으므로 in-process consume·중복 작업을 금지한다. |
| `Tools/ValtanPipeline/Run-ValtanAuthoringSaveJob.ps1` | 입력 `ExpectedSourceRevision`을 canonical commit의 previous revision과 대조하고, irreversible commit 직후에도 durable result를 먼저 남긴다. 그 뒤 같은 SHA의 split-source join과 candidate를 검증한다. 실패 시 `COMMIT_FAILED / CANDIDATE_PENDING / STALE_REVISION / CANDIDATE_FAILED` 경계를 보존한다. |
| `MainApp.cpp` | 창 표시 여부와 무관하게 매 frame `Update_ValtanSaveJob` → Workbench `Update_SaveState` → runtime publisher poll 순서로 실행한다. Save 완료를 Composition 창 `Render()`에 묶지 않는다. |
| `ActionCompositionWorkbench.h/.cpp` | Save 요청 시 local Pattern Sound generation/bytes와 Effect V2 revision/bytes를 pending receipt로 보존한다. 성공 terminal은 `exact SHA 확인 → local owner Accept → receipt Consume → Boss/Workbench graph reload → 같은 SHA Full DataOnly 시작` 순서를 지킨다. 중간 실패와 newer draft는 이전 snapshot/draft 및 pending 선택을 보존한다. Save-and-select도 terminal success와 graph reopen 전에는 선택을 바꾸지 않는다. |
| `Effect_Tool.h/.cpp` | Save가 전달한 exact SHA와 다시 연 canonical graph SHA가 같을 때만 All Effects/Pattern Tree snapshot을 교체한다. stale/missing revision이면 기존 snapshot을 보존한다. 요청은 Effect 창이 닫혀 있어도 MainApp이 보관·전달한다. |
| `BossTool.cpp` | Save 또는 Full DataOnly가 RUNNING/OBSERVATION_LOST이면 Flow Save/Load/Discard, single-pattern/Flow Restart 등 canonical mutation·gameplay 시작을 fail-close한다. read-only 확인과 Stop은 유지한다. |
| `Tools/Build/Run-FullPipeline.ps1`, `Tools/Build/Invoke-BuildAndRegression.ps1`, 루트 `RunFullPipeline.bat` | `-ExpectedValtanSourceRevision`을 받아 PublishV2, 각 DataOnly domain, build/regression 시작·완료와 최종 receipt에서 같은 repository SHA를 검사한다. 성공 출력 `FULL_PIPELINE_SOURCE_REVISION<TAB><SHA>`도 caller가 대조한다. |

이제 Save나 runtime publish는 UI thread에서 최대 120초 동기 대기하거나 timeout에 child를 강제 종료하지 않는다. 각각
durable log를 가진 분리 process로 실행하며 180초는 경고 기준일 뿐 `TerminateProcess` 기준이 아니다. Save transaction과
Full DataOnly transaction은 서로 배타적이고, source writer entry도 `SAVE_BUSY`/`PUBLISH_BUSY`로 같은 경계를 확인한다.
따라서 버튼 disable만 우회해도 동시에 canonical source를 덮을 수 없다.

`Publish after Save`가 ON이면 exact editor reopen까지 성공한 뒤에만 `Run-FullPipeline.ps1`의
`-DataOnly -ExpectedValtanSourceRevision <committed-SHA>` 계약으로 background job을 시작한다. candidate apply는 실행 중 Server의 apply class 계약을 따르고,
생성된 on-disk bootstrap은 Server 재시작 뒤 적용되는 기존 경계를 유지한다.

### 3.9.1 Restart/재시도 안정성

- `Restart Saved Flow`는 저장된 Flow를 다시 읽고, 선택 Flow revision과 Server-active gameplay revision이 정확히 맞을 때만
  새 occurrence를 시작한다. Save/publish transaction 중에는 시작하지 않는다.
- Client가 결과를 받지 못한 `UNCONFIRMED` 재시도는 새 request sequence를 만들지 않고 같은 request identity를 재전송한다.
- Server는 pattern/Flow start receipt에 마지막 lifecycle을 저장한다. 같은 identity의 exact duplicate는 arena reset이나 두 번째
  occurrence 생성이 아니라 기존 `QUEUED`, `ACTIVE`, `COMPLETED_HOLD` lifecycle을 다시 보낸다.
- payload가 달라졌거나 predecessor/replacement revision이 stale인 요청은 duplicate recovery로 취급하지 않고 거절한다.

### 3.10 filters 정리 (2차 요청 R25), DataFiles 유지 (R21)

`03. Tools\01. Boss`(비어 있었음)에 BossTool, BossLogicFlow*, ValtanPatternFlow*, ValtanTuningCommandService, ValtanPatternTree,
ValtanPatternAuditionService. 신규 `07. Balance`(BalanceTool), `08. Camera`(CameraTool), `09. Profiler`(ProfilerTool, ProfilerCaptureIO),
`10. Rendering`(RenderingBenchmark, RenderingProfileService). SequencerTool은 `05. Sequencer`. 29개 항목 이동, 물리 파일 이동 없음.
`96.DataFiles` `None` 항목 205개는 사용자 재확인대로 유지했다(컴파일 대상이 아니라 빌드 시간과 무관).
`Client/Bin/Resources`는 프로젝트 source/filter 재배치 대상으로 삼지 않았고, 사용자가 복구한 물리 팩을 수정하지 않았다.

### 3.11 grip offset (R22) — 답

`upM -0.9`는 손목 local 축 기준으로 캐릭터 발 원점을 손 아래 0.9 m에 두는 값이다. 09-03 G01/G02 이전에는 이 값이 100배로
적용돼 손에서 멀리 떨어져 보였고, 이번 빌드부터 수정된 단위로 적용된다. 값은 Composition → 해당 CAPTURE Stage → Collider →
Grab (Capture)의 forward/up/right DragFloat에서 바로 튜닝된다. 권고 시작점: 가슴 잡기 `-0.9 ~ -1.1`, 발을 손 높이에 `0`,
양수는 캐릭터가 손 위에 서는 형태. 몸통 앵커 변경은 offset으로 같은 결과를 얻을 수 있어 권하지 않는다.

## 4. 자동 검증

| 검증 | 결과 |
|---|---|
| `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate` | PASS — managedPatterns 42, combatObjects 9 |
| `Project-ValtanPatternMaster.ps1 -Mode PublishV2` | PASS — changed=0 artifacts=7 (Product 이미 최신) |
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS — 65 patterns / 280 stages / 52 timeline rows |
| `validate_effect_v2.py` / `Validate-EffectSources.ps1` | PASS |
| build gate / camera / pattern-tree / master_v2 / typed-patch / workbench oracles(55, 49) / model-view(16) / saved-rows(35) / Effect V2 binding(19) | PASS |
| Debug Product | PASS — `20260903T055052552Z-debug-product-d80ed84a.json` (auto-publish/filters/Server exact 검사 변경 전 시점) |
| Debug FullDiagnostic 3 | 첫 Python gate 통과 후 model-view oracle에서 중단 → oracle 교정 |
| native `ValtanPatternAuditionServiceHarness.exe` (수동) | large-donut canonical join 30/30·13/13·11/11 PASS; Restart/Flow focused 13/13 PASS. 최신 Client Save UI 통합 뒤 전체 harness 재빌드는 미실행 |
| `Server.exe --contract-test` (수동) | high-jump 정본 exact 검사 교정 및 duplicate Restart lifecycle replay 뒤 failures 0. 최신 Client Save UI 통합과는 독립인 Server 증거 |
| Save/Restart 현재 source-contract (`test_valtan_balance_tool_contract`, `test_action_composition_workbench_regression_oracles`, `test_valtan_boss_tool_pattern_flow_contract`, `test_full_pipeline_entrypoint_contract`) | **140/140 PASS** — 2026-09-03 현재 워킹트리 |
| Save/Restart + Effect exact refresh + presentation 최종 묶음 | **196/196 PASS** — 2026-09-03 현재 워킹트리 |
| `Run-ValtanAuthoringSaveJob.ps1` PowerShell parser | PASS — 2026-09-03 현재 워킹트리 |
| `test_effect_tool_exact_revision_refresh_contract` | **7/7 PASS** — exact SHA 전달과 stale snapshot 보존 source-contract를 현행 함수 시그니처로 재실행 |
| `Project-ValtanPatternMaster.ps1 -Mode Validate` | PASS — source `7cc003f40cbc59cf6a1ba3a956ab955b0f3adac64cc035bf5d61d4c9fa4dd473`, managedPatterns 42, combatObjects 9, projectedArtifacts 9 |
| `validate_valtan_requested_pattern_coverage.py` | PASS — Product 42 / Encounter 65 / scripted 29 |
| `test_valtan_pattern_master_v2` | 전체 70건 중 production 계약 69건 PASS, 임시 저장소가 새 Effect V2 group 물리 closure를 누락한 fixture 1건 교정 후 해당 건 1/1 재실행 PASS |
| 변경 Client TU `/Zs` (`ActionCompositionWorkbench`, `BalanceTool`, `BossTool`, `MainApp`) | PASS — C4819 기존 인코딩 경고만, compile error 없음 |
| 변경 JSON 13개 / Client vcxproj·filters XML parse | PASS — 2026-09-03 현재 워킹트리 |
| Debug FullDiagnostic 4 (최종, 모든 교정 포함) | 미실행 — 아래 5절 |
| `git diff --check` + untracked RESULT trailing-whitespace 검사 | PASS — 2026-09-03 현재 워킹트리(LF→CRLF 경고만 출력) |

## 5. FullDiagnostic 4 결과

최신 Save/receipt/hidden-window poll/publish barrier/Restart lifecycle replay 통합 뒤 Product와 FullDiagnostic은 실행하지 않았다.
현재 남아 있는 가장 최근 전체 build receipt는
`out/BuildPipeline/runs/20260903T055052552Z-debug-product-d80ed84a.json`이며 PASS지만, 이 receipt는 위 비동기
Save/Restart 통합 전 시점이다. 따라서 최신 소스의 compile/link/regression 완료 증거로 승격하지 않는다. 사용자가 build와
runtime visual/gameplay 확인을 수행한 뒤 새 receipt로 이 절을 갱신해야 한다.

## 6. 사용자 수동 확인 경로

```text
1. Visual Studio: Server + Client profile 을 Ctrl+F5 (client 역할 PC 는 Client project)
2. Lobby -> Valtan 진입 -> F1
3. Boss Tool: CORE / ANIMATOR / DERIVED 목록 (큰 도넛 VALTAN_FIST_IN_OUT_LARGE 포함)
4. Boss Tool Pattern Flow: Restart Saved Flow (Fresh Arena) -> 두 도넛 사이 wait 100 ms
5. Composition: VALTAN_WARP STEP_02~09 Animation box 선택/우측 trim, Pattern Root -> Pattern Timeline 표,
   Collider lane 의 COUNTER proxy / OBJECT 항목, Save 뒤 Save job ID -> exact reopen -> [Apply] -> [Publish] 전이
6. Save 직후 Composition 창을 닫거나 다른 tool 로 이동해도 background job이 끝나고 All Effects/Pattern Tree가 같은 revision으로
   갱신되는지 확인. publish 중 Save/Flow Load·Save·Restart가 차단되는지도 확인
7. Queue/Active/Completed hold 상태에서 응답을 놓친 동일 Restart를 재시도해 arena reset/중복 occurrence 없이 현재 lifecycle이
   복구되는지 확인
8. Sequencer: F1 -> Sequencer -> lane 과 playhead 가 Composition 과 함께 움직이는지
9. Profiler: F1 -> Profiler -> Capture 켜고 Valtan 재진입 -> Long operations 에 Loader.LevelLoad, Bottlenecks 에 Render.* / Engine.*
10. Rendering Workbench: Benchmark Capture 300 frames -> run 표와 Client/Bin/BenchmarkCaptures JSON
11. Six Pizza / Struggling 재생 시 Server 세션 유지 (Server CMD 에 [PresentationVolleySkipped] 가 찍혀도 방은 살아 있어야 함)
12. ImGui 창을 배율이 다른 서브 모니터로 옮겨 글자 선명도 확인
13. 잡기 패턴에서 왼손 부착 위치 확인 후 Grab collider 의 up 값 튜닝
14. 루트 RunFullPipeline.bat 한 번 실행 (전체 publish + FullDiagnostic)
```

## 7. 남은 경계

```text
Stage 를 branch 전용으로 바꾸는 모델 변경 (PLAN §6 권고)                       미구현
ALL_DIE collider response                                                          schema evolution 별도
Workbench 에서 combat object(투사체/도넛/도끼) 생성·편집 typed writer (PLAN §16 R20) 미구현(가시화만)
Effect V2 append 옵션·group 자식 편집 (09-03 G13)                                  미구현
워프 leg 재정합 (09-03 G05)                                                        미구현
Sequencer v1 문서·adapter (PLAN §16.1)                                             미구현
publish 시점 nav 전수 검사 (AUDIT 방지 3)                                          미구현
PBR/CSM/TAA 등 렌더링 기능 (PLAN §12.2)                                            로드맵만
Release 구성 build/regression                                                      미실행
최신 Save/Restart 통합 뒤 Debug Product/FullDiagnostic                            미실행
Resources payload/시각 품질                                                       사용자 관리·수동 검증 범위
```
