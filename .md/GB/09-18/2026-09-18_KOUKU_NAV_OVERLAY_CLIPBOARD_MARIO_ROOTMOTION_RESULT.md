# 쿠크 Show Navigation 표시·전체 lane Ctrl+C/V·마리오 부모 입장·recoil 경계 결과

## 소스 구현 상태 (Product EXE에는 아직 반영되지 않음)

네 결함의 소스 변경을 17개 파일에 anchor 기반 최소 edit으로 적용했다. 작업 중 `Client.exe`와 `Server.exe`가 실행 중이었고 종료하거나 교체하지 않았으므로, 현재 실행 중인 두 프로세스와 `Client/Bin/Debug`, `Server/Bin/Debug`의 EXE에는 이 변경이 없다. Product MSBuild는 실행하지 않았다. `git diff --stat`은 17 files, +1591/−478이며 이 수치에는 같은 파일의 다른 세션 미커밋 hunk(PATTERNTRACKMOVE, summon anchorKind/16 spawns, presentation model resolver, summon formation)가 포함된다. 새 파일·`.vcxproj/.filters` 변경·Shared protocol 변경은 없다.

### Show Navigation

| 파일 | hunk | 내용 |
|---|---|---|
| `Client/Private/LevelNavigationDebug.cpp` | `@@ -139,7 +139,8` Sync_Level; `@@ -276,6 +277,11` Render_Controls; `@@ -285,16 +291,29` Render_Overlay 머리; `@@ -346,21 +365,26` piece loop | `ImGuiViewport* viewport = GetMainViewport(); GetBackgroundDrawList(viewport)`(why 주석), `lineColors/fillColors`, hoisted `screen` lambda, 반투명 `AddQuadFilled` + 2px `AddLine`, `m_FrustumRejectedCells`/`m_CameraPosition`/`m_FirstDrawnCell` reset·capture·표시 |
| `Client/Public/LevelNavigationDebug.h` | `@@ -47,6 +47,21` | `DRAWN_CELL_SAMPLE`, `m_CameraPosition`, `m_FrustumRejectedCells`, `m_FirstDrawnCell` |

근본 원인은 인자 없는 `GetBackgroundDrawList()`가 `ViewportsEnable` 아래 암시적 `Debug##Default` 창의 viewport(`imgui.ini`가 `0x16723995`로 고정)에 그려 아무도 렌더하지 않은 것이다. 검토 지적(한 side plane 너머 piece에 채움을 넣고 frustum 거절로 세는 문제)을 반영해 `beyondSidePlane(0..3)` 검사를 채움 조건에 추가했다(cpp 368~378). 삽입 줄은 모두 CRLF이고 1~4행 LF는 유지했다(CR 359→383, LF 368→392, BOM 없음). `MainApp_WorldLevel.cpp:227`의 같은 패턴은 고치지 않았다(범위 밖, gotchas에 기록).

### 전체 lane Ctrl+C/V

| 파일 | hunk | 내용 |
|---|---|---|
| `Client/Public/KoukuSaydonActionWorkbench.h` | 362 주석; 678~693 `TIMELINE_CLONE_MODE{DUPLICATE_AFTER, DUPLICATE_SAME_TIME, PASTE_APPEND}`, `TIMELINE_CLONE_RESULT`, `Clone_TimelineSelectionInto`, `Select_ClonedTimelineRows`; 985~1003 `TIMELINE_CLIPBOARD` 전체 값 snapshot | public 서명 유지, 기존 호출자 4곳 그대로 |
| `Client/Private/KoukuSaydonActionWorkbench.cpp` | 1147~1220 `Expand_TimelineClosure`, `Format_ClipboardSummary`; 6884~7526 `Copy_TimelineSelection`, `Paste_TimelineClipboard`, `Clone_TimelineSelectionInto`, `Select_ClonedTimelineRows`, `Duplicate_TimelineSelection`; 6876/7983 안내 문구; 14935/14944 shortcut 메시지 | Copy는 모든 lane 선택의 ownership closure(hold·summon·group·region·companion·WORLD owner) + blend window closure + 참조 정의 5종 + Counts[10]; Paste는 정의 복원/`changed; copy again` 거절, Parent-only Pattern row, 자기 자신 금지, PASTE_APPEND clone, Extend, Commit, 선택; Duplicate는 같은 engine의 wrapper |

빈 Parent(P91/92/93, placeholder 15000ms)에는 0ms부터 배치한다. 검토 지적에 따라 Ctrl+D는 Box Detail을 종전처럼 timeline 선택에 유지하고 Paste만 마지막 clone presentation box를 연다. 두 파일 모두 ASCII+CRLF 유지.

### 마리오 부모 입장

| 파일 | hunk | 내용 |
|---|---|---|
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | 166; 813~818; 2323~2324; 4196~4198 | `LOGIC_RESULT_VALUE_KEYS += marioStage`; RESULT branch 0..4·MARIO_ENTER만; Mario 규칙 `gateId == GATE3`만('Mario entry requires a Gate 3 pattern'); `_project_outcomes`는 0이 아닐 때만 투영 |
| `Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py` | 1442~1504 | `mario_parent_entry_document`, chain 없는 Gate 3 부모 admission/투영 test, 범위·kind·gate 거절 test |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | 4114; 4195~4199 | `outcomeProperties += marioStage`(MARIO_ENTER); `Assert-JsonInteger 0..4`, 1..4면 `PATTERNLOGICOUTCOME` 11번째 field |
| `Server/Public/GameplayCatalog.h` | 651 | `std::uint8_t iMarioEntryStage = 0u` |
| `Server/Private/GameplayCatalog.cpp` | 2893~2902; 6796~6801 | 11-field MARIO_ENTER 행은 stage 1..4(percent/duration 0)로 parse, 다른 11-field 비FEAR 행 거절; admission에서 `chainCount != 1` 제거, `iMarioEntryStage > 4` 거절 |
| `Server/Public/KoukuSaydonLogicRuntime.h` / `.cpp` | h 149~157; cpp 965~997 | `Find_MarioEntryResult`, `Has_MarioEntry`, `Is_InsideMarioEntry(..., honorWindowEnd=false)`가 `elapsed >= Ticks_FromMs(startMs+durationMs)`면 거절 |
| `Server/Private/GameRoom_KoukuAudition.cpp` | 323~326; 482~483; 816~827; 891~896; 914~917; 1097~1108 | 요청 검증 `Has_MarioEntry`; 비chain은 hold 게시 안 함(주석); anchor 세팅과 stage 우선순위(요청 test stage > authored 1..4 > live counter); `honorWindowEnd = !chain && no PATTERN_COMPLETION_COUNT`; Commit이 PENDING 비chain member의 anchor를 닫음; 완료 시 run 종료 또는 pending 없음일 때만 anchor reset·pending erase |
| `Server/Private/ServerGameplayContractTests_KoukuProduct.cpp` | 10; 118~135; 498~660 | `Is_InsideMarioEntry` 창 끝 unit check, `Has_MarioEntry/Find_MarioEntryResult` unit check; `parentMarioEntry` 시나리오 AUTHORED_STAGE/LIVE_COUNTER/REQUEST_STAGE/AFTER_WINDOW/LAST_TICK |
| `Client/Public/KoukuSaydonCompositionDocument.h` | 169 | RESULT `std::uint32_t iMarioEntryStage` |
| `Client/Private/KoukuSaydonCompositionDocument.cpp` | 364~378 hasResultValues; 584~588 validate; 2318~2322 Save 규칙; 2864 known key; 2976 parse; 4317~4319 serialize | `marioStage` 0..4·MARIO_ENTER만, chain 요구 제거('Mario entry is the sole ENTER_AREA Success of a Gate 3 Pattern.'), 0이면 key 생략 |
| `Client/Private/KoukuSaydonActionWorkbench.cpp` | 664~697; 4994; 6388~6396; 7912; 13571~13577 | `Owns_ServerMarioEntry/Has_ServerMarioEntry/Follows_ServerClock`; `m_strServerFollowRootPatternId`; Play Preview가 `Follows_ServerClock`면 `Request_SelectedServerPlay`(tooltip: Save 뒤 Publish All Patterns 먼저); Sequencer Play 동일; RESULT Details `Mario stage (0 = live room counter)` SliderInt 0..4 |

검토 지적(같은 tick에 queue된 entry가 PATTERN_COMPLETED에서 erase되는 race)은 조기 완료(`bEndPatternEarly`)에서만 도달 가능함을 확인하고, run이 여기서 끝나거나 pending이 없을 때만 정리하며 Commit이 PENDING member의 anchor를 닫도록 고쳤다. 첫 시도(창을 lifetime 밖으로 늘린 fixture)는 Brain validator가 'logic window is out of the pattern lifetime'으로 거절해 STAGGER_WINDOW `bEndsPatternOnSuccess`로 교체했다. `GameRoom_KoukuAudition.cpp`는 혼합 줄바꿈(LF 31행 유지, CRLF 1863→1876), 다른 파일은 CRLF 유지.

### recoil 경계

| 파일 | hunk | 내용 |
|---|---|---|
| `Server/Private/KoukuSaydonBrain.cpp` | `@@ -1088,0 +1100,3`; `@@ -1102,2 +1116,31` | lateral = model X = 시각 정면 축 주석; 첫 navigation gate를 segment bisect((high−low)×length ≤ 1mm, 최대 40회, 각 candidate LOS+traversal 재검사)로 교체하고 가장 먼 navigable point와 ground를 commit, 1mm 미만이면 기존처럼 XYZ 유지·`iPatternStageRootLastTick`만 갱신 |
| `Server/Private/ServerGameplayContractTests_KoukuBundles.cpp` | `@@ -422,2 +422,19` | 'Navigation clamps an off-grid root destination to the last navigable point on the segment'(held, +X 1cm 이상 전진, Z 2mm 이내, 클램프 XZ `Is_PointWalkableExact`, +2mm traversal 실패, Y = origin + ground 차, LastTick 120)와 'A root segment blocked within 1 mm of the pose preserves the exact previous XYZ'(tick 121 bit-identical) |

축 부호는 바꾸지 않았다. MN_RPCZ_00/MN_RPCT_05는 model +X를 바라보므로 projector `lateral` 음수가 이미 시각 뒤 방향이다. 같은 파일의 다른 세션 hunk(340~360, 611~629, 824)는 건드리지 않았다. CRLF 유지(CR 1200→1232, lone LF 0), `KoukuBundles`는 혼합 파일이며 삽입 줄은 이웃과 같은 CRLF(lone LF 140 유지).

## 실행한 자동 검증

| 검증 | 명령·증거 | 결과 |
|---|---|---|
| Show Navigation 근본 원인 재현 | `out/NavOverlay20260918/imgui_fallback_viewport_probe.exe` 실제 ini + main (320,180) / ini 없음 | ini 있음: 매 프레임 `window->Viewport=0x16723995 main=0x11111111 sameList=0`, rendered=0; ini 없음: `0x11111111`, rendered=1 |
| Show Navigation 수치 재구성 | `python out/NavOverlay20260918/nav_overlay_probe.py 1280 720`(및 1920 1080) | drawn 24, rejectedFrustum 1235, nonFinite 0, 첫 셀 (488,627) h 1.30 화면 안 |
| Nav TU 격리 컴파일 | `out/NavOverlayApply20260918/compile_real.cmd`, `compile_mainapp.cmd`; 검토 반영 뒤 `out/KoukuFourFixesVerify20260918/client/compile_client.cmd` | `LevelNavigationDebug.cpp`, `MainApp.cpp` exit 0, 기존 C4828 경고만 |
| Ctrl+D baseline | 편집 전 소스(`before_sources/*.before`)로 만든 probe → `baseline/`, 편집 후 → `baseline_after/` (frozen input rev 1679) | p88_all_lanes, p88_same_time_effects, p17_hold_closure, p65_summon, p2_scene_profile 5/5 byte 동일 |
| Clipboard probe | `out/KoukuClipboardAllLanes20260918/clipboard_probe.exe`(`clipboard_probe.run.log`, `result.json`); 검토 반영 뒤 `out/ReviewFixesF20260918/clipboard_probe.direct.log` | total 386 checks PASS: baseline 1, legacy 182, step2-3 P88→P91 전체 paste·반복 61, step4 collider region 19, step5 world companion 15, step6 pattern row 규칙 14, step7 summon/cross-direction/P25 spawns 17, step8 scene profile 13, step9 hold logic 11, step10 실패 원자성 34, step11 blend 19; 검토 반영 뒤 재실행도 PASS 386, Ctrl+D 5/5 |
| out 전용 Save/Reload round trip | `out/KoukuClipboardAllLanes20260918/disk/KoukuSaydonComposition.json` | Save_Atomic → Reload 성공(Data/에는 쓰지 않음) |
| hotkey guard | `out/KoukuClipboardAllLanes20260918/ui/hotkey_guard_probe.exe` | PASS 26 cases, 새 메시지 2종 확인 |
| Workbench 소비 TU 격리 컴파일 | `ui/compile_workbench.cmd`, `compile_MainApp.cmd`, `compile_MainApp_WorldLevel.cmd`, `compile_Harness.cmd`; 최종 `out/KoukuFourFixesVerify20260918/client/` | Workbench, CompositionDocument, PresentationPlayer, MainApp, MainApp_WorldLevel, BossCompositionDocumentContractTests 모두 exit 0 |
| projector Mario test | `PYTHONPATH=. python -m unittest ... -k mario`(`out/ReviewFixesF20260918/py_mario.log`) | Ran 4, OK |
| projector 전체 module | `out/KoukuMarioParentPlay20260918/projector_tests_full.log`, `out/ReviewFixesF20260918/py_unittest.log` | Ran 231, failures 26 / errors 82~87. 실패는 저장본의 in-progress 편집('presentation occurrence exceeds the Pattern lifetime')과 다른 세션 hunk(`followSpeedScale`)에 의한 기존 실패이며 이번 변경 전 실행(head 39/88)과 같은 집합이다. 이 module을 PASS로 기록하지 않는다 |
| publisher parse | `[Parser]::ParseFile('Tools/GameplayPipeline/Publish-GameplayBalance.ps1')` | parse errors 0 |
| Client codec probe(후보 문서) | `out/KoukuMarioParentPlay20260918/client/codec_probe.exe`(`codec_probe.run.log`) | logic.86 `iMarioEntryStage 1`, Serialize `"marioStage": 1`, logic.61 생략, INSTANT_DEATH marioStage·값 5 거절, failures 0 |
| Server 격리 컴파일·계약(kouku) | `out/KoukuMarioParentPlay20260918/compile_server.ps1` → `server_result_kouku_reviewF2.log`; `out/ReviewFixesF20260918/suites/kouku.log`; `out/KoukuFourFixesVerify20260918/server2/kouku.log` | failures 0(91 PASS). parentMarioEntry AUTHORED_STAGE/LIVE_COUNTER/REQUEST_STAGE/AFTER_WINDOW/LAST_TICK 포함 |
| Server 계약(overlap/surface/nav) | 같은 driver | overlap 88 PASS, surface 191 PASS, nav 25 PASS, failures 0 |
| Server 전체 suite | `suites/full.log` | exit 1, failures 23 = 편집 전 baseline 집합(12 distinct: 완료 카운트·solo entrant·projectile collider 등 기존 실패)과 동일. 검토 반영 뒤 첫 full 실행(`full_failures.txt`)에는 신규 Mario 2건과 ghost/relocation 3건이 추가로 실패했으나 재실행(`full_failures2.txt`)과 최종 검증(`server2/full.log`)은 baseline과 동일했다. 첫 실행의 원인은 확정하지 못했다 |
| recoil probe(변경 후) | `out/KoukuRootMotionProbe20260918/compile_server.ps1` → `server_result.log`; 검증 재실행 `out/KoukuFourFixesVerify20260918/rootmotion/result.log` | A. P85 최종 (8.103, 10.560, 320.000) = 셀 모서리 flush(spawn에서 2.17m, 변경 전 7.998/320.078), destination 60 / held 312; B. P21 동일 pose, 6/162; C. 경계 2m 앞 시작 시 (16.000, 2.684, 314.157) = 경계 (16.006,314.152) 1cm 이내(변경 전 15.368/314.624), 61/311; D. 5/163 |
| KoukuBundles 계약 | `out/KoukuRootMotionApply20260918/compile_contracts.ps1` → `contract_result.log` | 'Navigation clamps an off-grid root destination to the last navigable point on the segment' PASS, 'A root segment blocked within 1 mm of the pose preserves the exact previous XYZ' PASS, failures 0. 첫 링크 시도는 LNK2019로 실패했고 전체 test TU를 포함한 재컴파일로 통과했다 |
| 축 측정 | `out/KoukuRootMotionProbe20260918/measure_rendered.py` | MN_RPCZ_00 눈/입 +X(~14cm), 손 ±Z(~52cm), rpcz00_att_battle_11_end root −4.40m model −X @3333ms; MN_RPCT_05 동일 +X |
| 인코딩·줄바꿈 | `out/KoukuFourFixesVerify20260918/eol_check.py`, `eol_hunks.py` | 17개 파일 BOM 변화 없음, UTF-8 decode, lone CR 0; 삽입 줄과 이웃 줄 끝 불일치 0(단, `project_kouku_saydon_composition.py` 986~988·2489의 lone LF 4줄은 다른 세션 summon hunk) |
| `git diff --check` | 17개 파일 | exit 0, whitespace 오류 없음(autocrlf 안내만) |
| Data/Bin 미변경 | `git status --short -- Data Client/Bin Server/Bin`, mtime·sha256 | 이 작업의 publisher/installer 실행 없음. 검증 중 관측된 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` rev 1704→1716, `Data/Encounters`, `Server/Bin/DataFiles/World|Gameplay`, `Client/Bin/DataFiles/World` 변경(19:11~19:17)은 사용자의 실행 중 Client Save/Publish이며 `nextLogicOrdinal`이 86 그대로이므로 Mario 후보는 설치되지 않았다 |

## 실행하지 않은 것

- Product MSBuild(`Invoke-BuildAndRegression.ps1`)와 `Framework.sln` Build. 위 컴파일은 모두 out/ 아래 격리 cl 호출이다.
- 실행 중 `Client.exe`/`Server.exe` 종료·교체·재시작, Client/UI 조작, 화면 캡처.
- publisher(`Publish All Patterns`, `Invoke-BuildDomainOwner`, `Publish-ServerNavigation.ps1`) 실행과 `Data/`·`Client/Bin/DataFiles`·`Server/Bin/DataFiles` 쓰기.
- `Tools/Network/Sync-TeamLanEndpoint.ps1`(Git 제외 `.vcxproj.user`를 고쳐 쓰므로 이번 read-only 규칙과 충돌, 실행 안 함).
- 사용자 화면 판정(Show Navigation 표시, Ctrl+C/V 결과, 마리오 입장, recoil 정지 위치).

## 준비했지만 설치하지 않은 데이터 후보

### 마리오 부모 입장 후보

`out/KoukuMarioParentPlay20260918/prepare_mario_parent_entry.py`가 만든 `KoukuSaydonComposition.candidate.json`(receipt: entry sha `cc28663e…` rev 1679 → candidate sha `2888b55d…` rev 1680, `nextLogicOrdinal` 86→90, logic.86~89 = `마리오_서버입장_1~4페이즈` marioStage 1~4, P88 `.logic.1` onSuccess → logic.86, `installed: false`). 이후 사용자 저장으로 live 파일은 rev 1716까지 진행했으므로 설치 시 `--install`의 SHA compare-and-swap이 거절하며 최신 저장본 기준으로 후보를 다시 만들어야 한다.

설치 순서(순서를 바꾸면 Workbench가 문서를 거절하고 projector가 `unknown=[marioStage]`로 실패한다):

1. Debug Product 빌드로 Client(문서 codec/Workbench)·projector·publisher·Server 변경을 먼저 반영한다.
2. 최신 저장본으로 후보 재생성 → `--install`(SHA CAS, backup, 원자 교체).
3. P91/92/93에 `마리오_입장` ENTER_AREA box와 연결 Collider를 Ctrl+C/V로 붙여넣은 뒤 `--link <patternId> <logicOccurrenceId> <stage>`로 logic.87/88/89를 연결하거나, Workbench RESULT Details의 `Mario stage` slider로 저작한다.
4. Save → `Publish All Patterns`(게시 revision == 저장 revision) → Server·Client 재시작. Gameplay bootstrap 행과 문서 key가 바뀌므로 둘을 같은 빌드로 함께 재시작한다(protocol 85 그대로).

### navpaint HEIGHT override 후보

`out/KoukuRootMotionProbe20260918/navpaint_candidate/`(README, `append_gate2_height_overrides.py`, dry-run `LV_LUT_MIDNIGHTC_ED.navpaint`: source 100행 + HEIGHT 10.56 8행, CRLF, byte-prefix 동일). 셀 (492,524), (493,524), (492,523), (493,523)과 mirror (489,527), (490,527), (491,527), (494,525). 이 override는 플레이어 이동·Big Saydon 높이 special case·`cardmaze.return`·`stage.kakul.sl03`에도 영향을 주는 Server gameplay 데이터이므로 사용자가 Show Navigation 또는 MapTool `Use Picked Height`로 그 셀들이 실제 바닥보다 7~8m 아래에 그려지는지(bake 오선택) 확인한 뒤에만 `--install --i-confirmed-show-navigation` → `Publish-ServerNavigation.ps1 -AreaId LV_LUT_MIDNIGHTC_ED` → Server 재시작으로 적용한다. 바닥이 실제로 꺼진 무대라면 데이터는 그대로 두고 flush 정지가 정답이다.

## 공개 문서 반영 상태

- `CLAUDE.md`: Show Navigation 문단(main viewport 채움·외곽선·진단 행)과 마리오 문단(`marioStage`, chain 선택, Play Preview/Sequencer Play Server 위임, 함께 재시작)이 검토 반영 세션의 `out/ReviewFixesF20260918/patch_docs.py`로 이미 갱신되어 worktree에 있다.
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`: Ctrl+C/V 전체 lane 문단, 마리오 `MARIO_ENTER`/`marioStage`/비chain 창 닫힘, Sequencer Play·Play Preview 위임 문장이 같은 patch로 갱신되어 있다. `ANIMATION_TOOL_OWNER_HANDOFF.md`에는 Ctrl+C·마리오 서술이 없어 수정하지 않았다.
- `.md/GB/gotchas.md`: 이 문서와 함께 네 항목(ImGui viewport, clipboard snapshot/PASTE_APPEND, marioStage 설치 순서/11-field 행, +X 정면·navgrid 단차·bisect)을 추가했다.

## 사용자 확인 절차

1. `powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`(실행 중 Client/Server가 EXE를 점유하면 먼저 종료). 헤더 변경으로 MainApp 등 소비 TU가 함께 재컴파일된다.
2. 마리오 후보 설치 → Workbench Save → `Publish All Patterns` → Server·Client 재시작.
3. F1 → `Arena Camera / Player` → `Show Navigation`: 캐릭터 주변에 녹색 채움+외곽선 4m 셀, `Frustum rejected N | camera (x, y, z)`, `First drawn cell (...) -> screen (x, y)`가 게임 창 화면 좌표 안에 있는지 확인한다. 2관문 쿠크 placement 주변에서 (492,524) 등 셀이 바닥보다 7~8m 아래인지 함께 확인해 navpaint 후보 적용 여부를 결정한다.
4. Action Workbench에서 P88(1마리오_1페이즈)의 모든 lane 선택 → Ctrl+C(`Copied 2 Stages, 1 Animation clip, 3 Pattern rows, 2 Logic, 4 World, 3 Effects, 1 Collider …`) → 1마리오_2페이즈 선택 → Ctrl+V. 0ms부터 배치되고 Save 후 Validate가 통과하는지 확인한다.
5. `1마리오_N페이즈 [Parent]` 선택 → `Play Preview`(또는 Sequencer Play). dirty/stale/revision 메시지가 없으면 Server 재생이 시작되고 8083ms부터 보이는 Collider로 걸어 들어가 Mario stage N 입장을 확인한다. 창 끝(28395ms) 이후에는 입장하지 않아야 한다.
6. 2관문에서 쿠크_슈퍼바주카(P85) 재생: recoil이 셀 모서리(spawn에서 약 2.17m)에서 flush로 멈추고 animation은 계속되는지 확인한다.

## 남은 경계와 의문

- Product EXE 미반영, 후보 데이터 미설치, 화면 미확인은 위와 같다.
- Server full suite의 첫 검토-후 실행에서만 신규 Mario 2건·ghost/relocation 3건이 실패하고 재실행·최종 검증에서는 baseline과 같았다. 순서 의존 또는 stale 산출물 가능성이 있으나 원인을 확정하지 못했다.
- projector 전체 module의 기존 실패(26/82)는 이번 변경과 무관한 저장본 편집·다른 세션 hunk 때문이며 해소하지 않았다.
- `strTrackingPresentationOccurrenceId/RandomVolleyOccurrenceSets/strFixedSelectionGroupId/strSelectedEffectGroupId` 추가 remap은 PASTE_APPEND에만 적용했다. Ctrl+D에도 적용하면 현재 Duplicate 출력이 바뀌므로 보류했다.
- typed 참조를 가진 Logic 정의는 paste마다 `(copy)` 정의가 하나씩 늘어난다(Duplicate와 같은 정책).
- `MainApp_WorldLevel.cpp:227`의 인자 없는 `GetBackgroundDrawList()`는 같은 ini 상태에서 사라질 수 있으며 별도 변경으로 남긴다.
- forward/lateral 이름이 +X 정면 rig에서 뒤바뀐 의미인 점은 BossCatalog per-actor basis 같은 후속 범위이며 이번에 바꾸지 않았다.
