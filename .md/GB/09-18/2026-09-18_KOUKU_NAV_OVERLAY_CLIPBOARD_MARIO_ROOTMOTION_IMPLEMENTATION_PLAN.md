# 쿠크 Show Navigation 표시·전체 lane Ctrl+C/V·마리오 부모 입장·recoil 경계 구현 계획

## 목표

사용자가 같은 세션에서 보고한 네 결함을 기존 owner 경로 안에서 고친다. 새 runtime 경로, 새 C++ 파일, `.vcxproj/.filters` 변경은 없다.

| 결함 | 종료 증거 |
|---|---|
| F1 `Show Navigation`이 `Drawn 20`을 세면서 화면에는 아무것도 없다 | main viewport background list에 직접 그리고, 패널이 frustum 밖 셀 수·카메라 위치·처음 그린 셀의 screen 좌표를 표시한다 |
| 1마리오_1페이즈(P88)에서 Ctrl+C가 `Copy supports Animation clips and Effects only`로 거절된다 | 모든 lane의 선택을 소유 연결과 함께 복사하고 1마리오_2/3/4페이즈(P91/92/93)에 Ctrl+V로 붙여넣는다. Ctrl+D 결과는 byte 동일하다 |
| 1마리오 부모 폴더의 Play가 Server에 가지 않고, 가더라도 chain 없는 ENTER_AREA→MARIO_ENTER를 Server가 열지 못한다 | chain 없는 Gate 3 부모도 projector·Client·Server·publisher가 같은 규칙으로 admission하고, Play Preview/Sequencer Play가 Server Play로 위임하며, `marioStage` 저작값으로 1~4단계에 들어간다 |
| 2관문 쿠크_슈퍼바주카(P85)·P21 recoil이 약 2m 뒤 멈춘다 | Server가 tick segment를 navigation 경계까지 bisect해 flush로 멈추고, 경계 원인(navgrid 높이 단차)은 사용자 Show Navigation 확인 뒤 별도 navpaint 후보로 다룬다 |

## 현재 실측

### Show Navigation

- `Client/Private/LevelNavigationDebug.cpp:296`은 `ImGui::GetBackgroundDrawList()`를 viewport 인자 없이 호출한다. ImGui 1.92.8 docking + `ViewportsEnable`(`Engine/Private/ImGuiLayer.cpp:36-37`)에서 이 overload는 `GImGui->CurrentWindow->Viewport`로 resolve되고, 호출 지점(`CMainApp::Render`, 모든 tool 창 End 뒤)의 current window는 암시적 `Debug##Default` 창이다.
- `Client/Default/imgui.ini` 1~5행이 `[Window][Debug##Default] ViewportPos=0,0 ViewportId=0x16723995`로 그 창을 자기 viewport에 고정한다. `0x16723995`는 `IMGUI_ENABLE_SSE4_2_CRC` 아래 `ImHashStr("Debug##Default")`다. Client 창이 `CW_USEDEFAULT`로 생성되어 main viewport rect가 (0,0)-(400,400)을 포함하지 못하면 병합이 실패하고, 숨겨진 fallback 창의 viewport는 platform window가 없어 `RenderPlatformWindowsDefault`도 `ImGui::GetDrawData()`(g.Viewports[0])도 그 background list를 그리지 않는다.
- 같은 화면의 `CHitAreaWire`(`HitAreaWire.cpp:24-25`)는 `GetBackgroundDrawList(ImGui::GetMainViewport())`를 명시해 정상 표시된다. 셀 투영·ClipLine·`Drawn` 계산 자체는 정상이다(1관문 follow 카메라 재구성: inRange 1259, drawn 24, frustum 거절 1235, 첫 셀 (488,627) h 1.30, 화면 안).
- headless 재현: `out/NavOverlay20260918/imgui_fallback_viewport_probe.exe`를 실제 ini + main (320,180)으로 실행하면 매 프레임 `window->Viewport=0x16723995 main=0x11111111 sameList=0`, `rendered in main viewport DrawData=0`; ini 없이 실행하면 `0x11111111`, rendered=1.
- 파일 인코딩: cpp는 1~4행 LF, 5행 이후 CRLF(혼합), BOM 없음. h는 LF.
- 동일 잠재 결함: `Client/Private/MainApp_WorldLevel.cpp:227`도 인자 없는 `GetBackgroundDrawList()`를 쓴다(이번 범위 밖, 기록만).

### Ctrl+C/V

- `Copy_TimelineSelection`(cpp 6810~6999)은 Stage·Animation·EFFECT box만 허용하고 `strLogicOccurrenceId`가 있는 Effect도 거절한다. `TIMELINE_CLIPBOARD`(h 969~983)는 Stages/Effects/Worlds와 ANIMATION_BLEND 정의만 담는 부분 snapshot이며 `Paste_TimelineClipboard`(7002~7130)가 축소 remap을 재구현한다.
- `Duplicate_TimelineSelection`(7132~7480)에는 이미 전체 lane 알고리즘이 있다. ownership closure(hold logic, summon occurrence, selection group, collider region, world companion), 6개 lane cloneLane, group/region remap, typed 참조를 가진 Logic 정의의 copy-on-write, stage splice와 BossMotion/lane shift. 단 source == target을 전제한다(새 row index와 usedGroupIds를 source에서 취함, pending-geometry가 targetId 키).
- P88 실측: Stage 2/animation 1, PatternOccurrences 38/40/43, `.logic.1`=logic.59 ENTER_AREA(8083/20312)에 MAP collider `presentation.7`이 `strLogicOccurrenceId`로 연결, `.logic.2`=logic.58 이름만 있는 TRIGGER, World 4개(world.35/22), Effect 3개. P91/92/93은 15000ms 빈 Parent timeline이며 같은 Gate·actor·placement다.
- `CKoukuSaydonCompositionDocument::Validate`(4145)가 pattern-scoped 참조·collider/logic 동일 시각·world owner/companion·`Validate_PatternChildren`(Parent 전용 대상, 같은 gate/actor/boss, 비겹침)을 이미 검사하므로 일반화된 paste의 실패는 `Commit_Candidate`에서 원자적으로 거절된다.
- 다른 세션의 미커밋 hunk(141-149, 1307-1386, 7882+, 7939+, 9559-9706, 11878-12039, 12465-12607, 13083-13231; h 19/565/828/915)와 이번 범위는 겹치지 않는다. 두 파일 모두 ASCII+CRLF.

### 마리오 부모 입장

- Workbench: Sequencer `Play##KoukuSequencer`(7778~7784)는 `Has_CompletionChain`(PATTERN_COMPLETION_COUNT box)일 때만 `Request_SelectedServerPlay`로 가고, transport `Play Preview`(6238~6242)는 항상 `Request_PatternPreview`(local)다. `m_strServerFollowRootPatternId`도 chain 패턴만 설정한다(4886).
- 데이터: P88 `.logic.1`=logic.59 ENTER_AREA `마리오_입장`(8083/20312, onSuccess 비어 있음), MAP collider `presentation.7` 연결. 유일한 MARIO_ENTER 결과는 logic.61이며 chain 패턴 P34가 쓴다. 저장본 formatVersion 3, revision 1669(조사 시점), nextLogicOrdinal 86.
- admission이 네 곳 모두 chain을 요구한다: projector `project_kouku_saydon_composition.py:2310-2318`("Mario entry requires one owning Gate 3 completion chain"), Client `KoukuSaydonCompositionDocument.cpp:2310-2316`, Server `GameplayCatalog.cpp:6785-6791`, Server 요청 검증 `GameRoom_KoukuAudition.cpp:322-327`. publisher는 chain 규칙이 없고 MARIO_ENTER를 10-field `PATTERNLOGICOUTCOME` 행으로 싣는다. Server parser(`GameplayCatalog.cpp:2859-2892`)는 11-field 행을 전부 FEAR로 본다.
- Server runtime: MarioEntryAnchor/strMarioEntryPatternId/iMarioEntryStage는 member 시작 때 PATTERN_COMPLETION_COUNT 창이 있을 때만 세팅(815~833, stage = `m_iNextMarioEntryStage`). `Is_InsideMarioEntry`(KoukuSaydonLogicRuntime.cpp:965-989)는 `elapsed >= startMs`만 보고 창 끝을 검사하지 않는다. Broadcast(478~495)는 chain root만 `strMarioEntryPatternId/iMarioEntryHoldMs`를 채우며 Shared writer(`PacketMessages.cpp:6547-6553`)는 hold 0에 비어 있지 않은 ID를 거절하고 Client(`KoukuSaydonPresentationPlayer.cpp:2552-2578`)는 그 패턴을 frozen hold session으로 바꾼다. 따라서 chain 없는 패턴은 `strMarioEntryPatternId`를 비워 두어야 한다.
- 저작 stage field가 없다: `BOSS_PATTERN_LOGIC_RESULT`(GameplayCatalog.h:639-656), Client RESULT struct(KoukuSaydonCompositionDocument.h:160-176), RESULT Details UI(Workbench 13306~13380) 모두 없다.
- 현재 projector에 `marioStage`를 넣으면 `logics[85] fields are invalid; unknown=['marioStage']`, 키를 빼면 P88 closure가 chain 규칙에서 거절된다(out/KoukuMarioParentPlay20260918 dry run).

### recoil 경계

- Server는 이미 P85/P21 recoil을 시각적 뒤 방향으로 적용한다. 2관문 게시 placement `boss.kakulsaydon.g2.kouku`(6.36, 10.56, 321.29, yaw 216.5)에서 약 2.04m 이동 후 `Apply_StageRootMotion`의 첫 navigation gate(`KoukuSaydonBrain.cpp:1109-1115`)가 `Has_LineOfSight/Resolve_TraversalStep` 실패로 hold한다. 원인은 arena 가장자리가 아니라 base navgrid 셀 (491,525) 10.56m 옆 셀 (492,524) 등이 2.68/3.54/6.51m인 checkerboard 높이 단차이며 `LV_LUT_MIDNIGHTC_ED.navpolicy`는 1m 단차만 허용한다. 걷기 가능 셀은 그 방향으로 11m 더 이어진다.
- 축: MN_RPCZ_00·MN_RPCT_05는 model +X를 바라본다(눈/입/발끝 +X, 손 ±Z, 머리카락 −X; `out/KoukuRootMotionProbe20260918/measure_rendered.log`). Client는 scale-only pre-transform으로 admission하고 CNpc는 `Rotation(0,yaw,0)`이므로 model +X = Transform Right = Server `lateral+` basis다. projector `lateral` 음수 = model −X = 시각 뒤. 부호 뒤집기는 필요 없다.
- probe `out/KoukuRootMotionProbe20260918/server_result.log`(변경 전): A. P85 STAGE_5가 (7.998,320.078)에서 held 308/372 tick, B. P21 (7.938,320.122) held 157, C. 경계 2m 앞 시작 시 (15.368,314.624)에서 hold(경계 (16.006,314.152)보다 0.8m 앞), D. P21 (15.976,314.174).
- 기존 계약 `ServerGameplayContractTests_KoukuBundles.cpp:418-423` 'Navigation rejects an off-grid root destination without a partial XYZ commit'은 부분 commit 없음을 요구하므로 flush 변경과 충돌한다.

## 변경할 파일

| 파일 | 변경 | 등록 |
|---|---|---|
| `Client/Private/LevelNavigationDebug.cpp` | main viewport draw list, 채움+2px 외곽선, 진단 reset/capture/표시 | 기존 |
| `Client/Public/LevelNavigationDebug.h` | `DRAWN_CELL_SAMPLE`, `m_CameraPosition`, `m_FrustumRejectedCells`, `m_FirstDrawnCell` | 기존 |
| `Client/Public/KoukuSaydonActionWorkbench.h` | full-snapshot `TIMELINE_CLIPBOARD`, `TIMELINE_CLONE_MODE/RESULT`, `Clone_TimelineSelectionInto`, `Select_ClonedTimelineRows` | 기존 |
| `Client/Private/KoukuSaydonActionWorkbench.cpp` | Copy/Paste/Duplicate를 공용 clone engine으로 통합, 안내 문구; `Owns/Has_ServerMarioEntry`, `Follows_ServerClock`, Play Preview/Sequencer Play 위임, Mario stage slider | 기존 |
| `Client/Public/KoukuSaydonCompositionDocument.h` | RESULT `iMarioEntryStage` | 기존 |
| `Client/Private/KoukuSaydonCompositionDocument.cpp` | `marioStage` parse/validate/serialize, Save 규칙에서 chain 요구 제거 | 기존 |
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | `marioStage` 허용·검증·투영, Mario 규칙 완화, 축 주석 | 기존 |
| `Tools/KoukuSaydonPipeline/test_project_kouku_saydon_composition.py` | chain 없는 Gate 3 admission·범위/kind/gate 거절 test | 기존 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | `marioStage` optional key, 1..4일 때 11번째 field | 기존 |
| `Server/Public/GameplayCatalog.h` | `BOSS_PATTERN_LOGIC_RESULT::iMarioEntryStage` | 기존 |
| `Server/Private/GameplayCatalog.cpp` | 11-field MARIO_ENTER 행 parse, admission에서 chain 요구 제거 | 기존 |
| `Server/Public/KoukuSaydonLogicRuntime.h` / `.cpp` | `Has_MarioEntry`, `Find_MarioEntryResult`, `Is_InsideMarioEntry(honorWindowEnd)` | 기존 |
| `Server/Private/GameRoom_KoukuAudition.cpp` | 요청 검증·anchor 세팅·창 끝·완료 시 portal 정리 | 기존 |
| `Server/Private/ServerGameplayContractTests_KoukuProduct.cpp` | parentMarioEntry 시나리오와 unit check | 기존 |
| `Server/Private/KoukuSaydonBrain.cpp` | 첫 navigation gate를 boundary bisect로 교체, 축 주석 | 기존 |
| `Server/Private/ServerGameplayContractTests_KoukuBundles.cpp` | 'clamps to the last navigable point' 계약 | 기존 |

Shared protocol 85와 `PacketMessages` wire는 바꾸지 않는다. 다른 세션의 미커밋 hunk가 같은 파일에 있으므로 파일 교체가 아니라 anchor 기반 최소 edit만 사용하고 파일별 인코딩·줄바꿈을 유지한다.

## 데이터와 호출 흐름

### Show Navigation

```text
CMainApp::Render (모든 tool 창 End 뒤)
→ CLevelNavigationDebug::Render_Overlay
  → m_DrawnCells/m_VisibleOmittedCells/m_SupersededCells/m_FrustumRejectedCells = 0, m_FirstDrawnCell = {}
  → camera finite 검사 뒤 m_CameraPosition 기록
  → ImGuiViewport* viewport = ImGui::GetMainViewport(); draw = GetBackgroundDrawList(viewport)
  → 셀 piece마다: near/far 사이이고 한 side plane 너머가 아니면 AddQuadFilled(반투명)
     ClipLine 통과 edge마다 AddLine 2px, 첫 edge를 m_FirstDrawnCell에 기록
     visible이면 Drawn/omitted, 아니면 ++m_FrustumRejectedCells
→ Render_Controls: 'Frustum rejected N | camera (x,y,z)' + 'First drawn cell (...) -> screen (x,y)' 또는 'No cell passed the frustum test'
→ Sync_Level: 같은 진단 reset
```

진단 값은 직전 프레임 overlay 값이며 한 프레임 지연이다.

### Ctrl+C/V

```text
Process_TimelineClipboardShortcuts (text input/popup/drag 아님)
→ Copy_TimelineSelection
  → Expand_TimelineClosure: 선택 + hold logic, summon occurrence, selection group, collider region, world companion/owner
  → blend window closure, 참조 정의(Logic/World/Summon/SceneProfile/PresentationResource) 값 capture
  → TIMELINE_CLIPBOARD{composition/area/actor/gate/boss ID, iNext* 5개, Pattern 값 snapshot, SelectedStageIds/OccurrenceIds, 정의 5종, Counts[10]}
  → 상태: Format_ClipboardSummary("Copied ", Counts) + ". Select a destination Pattern and press Ctrl+V."
→ Paste_TimelineClipboard
  → composition/area/actor/gate/boss 일치, 삭제된 정의 복원(ordinal 상향), 변경된 정의는 'changed; copy again' 거절
  → Pattern row는 Parent timeline만, 자기 자신 금지
  → Clone_TimelineSelectionInto(PASTE_APPEND): 새 row index/usedGroupIds는 destination 기준, 빈 Parent는 0ms부터, 그 외 destination 끝에 append
  → Extend_PatternLifetimeForAuthoredLanes → Commit_Candidate(Validate) → Select_ClonedTimelineRows
  → 실패: m_Draft/generation/clipboard 보존
→ Duplicate_TimelineSelection = Clone_TimelineSelectionInto(DUPLICATE_AFTER | DUPLICATE_SAME_TIME) 얇은 wrapper, 기존 Serialize 결과 byte 동일
```

### 마리오 부모 입장

```text
저작: RESULT MARIO_ENTER {marioStage 0..4} ─ ENTER_AREA box(Collider region) sole Success ─ Gate 3 Pattern
  Client Parse/Validate/Serialize(0이면 key 생략) → Save
projector: LOGIC_RESULT_VALUE_KEYS += marioStage, MARIO_ENTER만 소유, Gate 3만 요구(chain 선택)
  → _project_outcomes: marioStage != 0일 때만 투영
publisher: outcomeProperties += marioStage(MARIO_ENTER), Assert 0..4, 1..4면 PATTERNLOGICOUTCOME 11번째 field
Server parse: 11-field MARIO_ENTER → iMarioEntryStage 1..4(percent/duration 0), 다른 11-field 비FEAR 행은 거절
Server admission: Gate 3 + ENTER_AREA + sole OnSuccess + regions + stage <= 4 (chainCount 요구 제거, >1은 기존 거절 유지)
Server runtime:
  Evaluate 요청: Has_MarioEntry(chain root 또는 ENTER_AREA→MARIO_ENTER)
  Prepare_KoukuAuditionTick: Has_MarioEntry면 anchor 세팅, stage = 요청 test stage > authored 1..4 > m_iNextMarioEntryStage
  Update_KoukuMarioEntry: honorWindowEnd = !chain && no PATTERN_COMPLETION_COUNT → Is_InsideMarioEntry가 startMs+durationMs 이후 거절
  Commit_KoukuMarioEntries: Enter_MarioFromPattern, m_iNextMarioEntryStage = stage+1
  완료: chain 없는 member는 run 종료 또는 pending 없음일 때 anchor reset + pending erase; Commit은 PENDING member의 anchor를 닫음
  Build_KoukuBundleState: chain root만 hold 게시(비chain은 strMarioEntryPatternId 비움)
Workbench: Follows_ServerClock = Has_CompletionChain || Has_ServerMarioEntry(부모는 Try_ExpandPatternDocument로 자식 box 포함)
  Play Preview / Sequencer Play → Request_SelectedServerPlay(dirty/stale/revision gate) ; m_strServerFollowRootPatternId 동일 조건
```

데이터 후보 `out/KoukuMarioParentPlay20260918/prepare_mario_parent_entry.py`는 logic.86~89(`마리오_서버입장_1~4페이즈`, marioStage 1~4)를 추가하고 P88 `.logic.1` onSuccess를 logic.86에 연결한다. Client/projector/publisher가 `marioStage`를 읽는 빌드가 준비된 뒤에만 설치한다(먼저 설치하면 Workbench `Has_Properties`가 문서를 거절하고 projector가 `unknown=[marioStage]`로 실패).

### recoil 경계

```text
Update_KoukuSaydonBoss → Apply_StageRootMotion
→ destination = origin + lateral*(cos yaw, -sin yaw) + forward*(sin yaw, cos yaw)  (lateral = model X = 시각 정면 축)
→ Has_LineOfSight && Resolve_TraversalStep 실패 시
   segment [pose → destination]을 (high-low)*length <= 1mm, 최대 40회 bisect
   각 candidate를 LOS + traversal로 재검사, 가장 먼 navigable point와 그 ground를 commit
   1mm 미만이면 기존처럼 XYZ 유지 + iPatternStageRootLastTick만 갱신
→ 이후 Resolve_CircleMove와 두 번째 gate, SLAM 분기, origin-relative sampling은 그대로
```

## G별 구현 범위

### G01. Show Navigation overlay

- `Render_Overlay`: 진단 reset, finite 검사 뒤 `m_CameraPosition`, `ImGuiViewport* viewport = GetMainViewport()` + `GetBackgroundDrawList(viewport)`(why 주석), `lineColors/fillColors`, 단일 hoisted `screen` lambda, piece별 budget flag·채움·2px 외곽선·첫 셀 sample·frustum 거절 카운트.
- `Render_Controls`: `Drawn ...` 행 아래 진단 두 행.
- `Sync_Level`: 진단 reset.
- 헤더: `DRAWN_CELL_SAMPLE{valid, cellX, cellZ, worldX, worldZ, height, screenX, screenY}`, `float3_t m_CameraPosition`, `uint32_t m_FrustumRejectedCells`, `DRAWN_CELL_SAMPLE m_FirstDrawnCell`.
- 채움은 frustum 거절로 셀 수 있는 piece(한 side plane 너머 전부)에 draw work를 추가하지 않는다.

### G02. 전체 lane Ctrl+C/V

- 헤더: `TIMELINE_CLIPBOARD` 전체 값 snapshot으로 교체, `TIMELINE_CLONE_MODE{DUPLICATE_AFTER, DUPLICATE_SAME_TIME, PASTE_APPEND}`, `TIMELINE_CLONE_RESULT{NewStageIds, NewBoxIds, iDetachedBoundaryBlends, bPlacedFromZero}`, private `Clone_TimelineSelectionInto`, `Select_ClonedTimelineRows`. public 서명은 유지한다.
- cpp: 익명 helper `Expand_TimelineClosure`, `Format_ClipboardSummary`; `Copy_TimelineSelection`(closure + 정의 capture + counts), `Paste_TimelineClipboard`(정의 복원/검증, pre-check, PASTE_APPEND, Extend, commit, 선택), `Clone_TimelineSelectionInto`(기존 Duplicate 본문 이동 + mode gate), `Select_ClonedTimelineRows`, `Duplicate_TimelineSelection` wrapper(Box Detail 선택은 종전처럼 timeline 선택 유지). Details/Timeline hint/shortcut 문구 갱신.
- PASTE_APPEND에서만 `strTrackingPresentationOccurrenceId/RandomVolleyOccurrenceSets/strFixedSelectionGroupId/strSelectedEffectGroupId`를 추가 remap한다. Ctrl+D 출력은 byte 동일해야 한다.

### G03. 마리오 부모 입장

- projector [0]~[3], test [4], publisher [5]~[6], Server catalog [7]~[9], LogicRuntime [10]~[11] + `honorWindowEnd`, GameRoom [12]~[15], Server 계약 test [16], Client 문서 [17]~[22], Workbench [23]~[27]. 번호는 조사 보고서 plan 항목이다.
- 창 끝 결정: chain 없는 입장만 `startMs+durationMs`를 존중해 보이는 Collider box와 일치시키고, chain root는 자식 실행 중 입장을 위해 기존처럼 끝을 검사하지 않는다.
- 같은 tick에 queue된 entry와 PATTERN_COMPLETED가 겹치는 조기 완료(stagger/counter/external-signal `bEndPatternEarly`)는 Commit이 소비할 때까지 anchor를 유지한다.
- 데이터 후보는 out/에만 만들고 설치 순서(코드 빌드 → 설치 → Save/Publish All Patterns → Server/Client 재시작)를 RESULT에 기록한다.

### G04. recoil 경계 bisect

- `Apply_StageRootMotion` 첫 gate만 교체하고 SLAM·두 번째 gate·origin-relative sampling은 유지한다.
- `KoukuBundles` 계약을 'Navigation clamps an off-grid root destination to the last navigable point on the segment'로 바꾸고 1mm 이내 차단 시 XYZ 보존 계약을 추가한다.
- navpaint HEIGHT override는 사용자가 Show Navigation/MapTool로 실제 바닥이 10.56m 평면임을 확인한 뒤에만 설치하는 후보로 둔다. 확인 없이 Server gameplay 데이터를 바꾸지 않는다.

## 검증

| 항목 | 명령 | 기대 |
|---|---|---|
| Show Navigation 근본 원인 | `out/NavOverlay20260918/imgui_fallback_viewport_probe.exe Client/Default/imgui.ini 320 180` / `- 320 180` | ini 있음: `sameList=0`, rendered=0; ini 없음: rendered=1 |
| Show Navigation 수치 | `python out/NavOverlay20260918/nav_overlay_probe.py 1280 720` | drawn 24, rejectedFrustum 1235, 첫 셀 화면 안 |
| Nav TU 격리 컴파일 | `out/NavOverlayApply20260918/compile_real.cmd`, `compile_mainapp.cmd` | exit 0, 기존 C4828 경고만 |
| Ctrl+D baseline | 편집 전 소스로 probe 빌드 → `baseline/*.json` 5건; 편집 후 `baseline_after/*.json` | 5/5 byte 동일 |
| Clipboard probe | `python -X utf8 out/KoukuClipboardAllLanes20260918/build_probe.py` + `run_probe.ps1` | legacy 182 + 신규 step2~11 전부 PASS, 실패 시 draft/generation/clipboard 보존 |
| Workbench 소비 TU 격리 컴파일 | `out/KoukuClipboardAllLanes20260918/ui/compile_*.cmd` (Workbench, MainApp, MainApp_WorldLevel, Harness) | exit 0 |
| hotkey guard | `ui/hotkey_guard_probe.exe` | text input/popup/drag 중 무시, 새 메시지 확인 |
| projector | `PYTHONPATH=. python -m unittest Tools.KoukuSaydonPipeline.test_project_kouku_saydon_composition -k mario` | 신규 2건 포함 PASS; 전체 module의 기존 실패는 baseline과 비교 |
| publisher parse | `[Parser]::ParseFile('Tools/GameplayPipeline/Publish-GameplayBalance.ps1')` | 오류 0 |
| Server 격리 컴파일 + 계약 | `out/KoukuMarioParentPlay20260918/compile_server.ps1` → kouku/overlap/surface/nav/full suite | kouku·overlap·surface·nav failures 0; full은 기존 baseline 실패 집합과 동일 |
| Client codec probe | `out/KoukuMarioParentPlay20260918/client/codec_probe.exe` (후보 문서) | logic.86 `iMarioEntryStage 1`, Serialize에 `"marioStage": 1`, logic.61 생략, INSTANT_DEATH/5 거절 |
| recoil probe | `out/KoukuRootMotionProbe20260918/compile_server.ps1` → `server_result.log` | A: (8.0,320.0) 셀 모서리 ~2.17m, C: 경계 (16.006,314.152) 1cm 이내 |
| KoukuBundles 계약 | `out/KoukuRootMotionApply20260918/compile_contracts.ps1` | 'clamps ...' 및 '1 mm ... preserves' PASS |
| 인코딩·공백 | python byte scan(CR/LF/BOM) 전후, `git diff --check` | 삽입 줄이 이웃 줄 끝과 같음, whitespace 오류 0 |

Product MSBuild, 실행 중 Client/Server 교체·재시작, publisher 실행과 화면 판정은 이 계획의 자동 검증에 포함하지 않는다. 사용자가 Debug Product 빌드 뒤 F1 흐름으로 확인한다.
