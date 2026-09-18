# Map Tool 통합 컷신 타임라인 — 구현 결과

2026-09-17. 입력은 `2026-09-17_MAP_TOOL_INTEGRATED_CUTSCENE_TIMELINE_PLAN.md`(사용자 전달본)와
`설명서.txt`다. 이 문서는 **소스 구현 / 자동 검사 / 빌드 / 사용자 인수**를 분리해 기록한다.

## 0. 선행 — origin/main 빌드 오류 복구

병합(`e71dd131` → `bddacace`, fast-forward) 후 Client 빌드가 실패했다. 원인은 이번 작업과 무관하다.

- `Client/Private/Level_Loading.cpp`에서 `f32_t fEffectLane = 0.f;` 선언이 사라져 있었다.
  그 변수를 247·257·265·278행이 사용해 `C2065` 4개와 `C2672` 1개가 발생했다.
- 추적 결과 `837995fb`에는 선언이 있었고 **`c3f3360b "loader optimization, bern optimization"`에서
  사라졌다.** `bddacace`(origin/main HEAD)까지 그대로 없다. 즉 **main을 받은 누구도 Client를
  빌드할 수 없는 상태였다.**
- `837995fb`에 있던 그대로 한 줄만 원래 자리에 복원했다. 팀장 쪽 main 수정이 별도로 필요하다.

병합 시 겹친 파일은 `Client/Private/Loader.cpp` 하나였다(상대 343 insert / 내 1줄). 내 한 줄을
빼두고 fast-forward한 뒤 새 파일에 다시 넣었다.

## 1. 구현한 화면

Map Tool → Camera 탭 상단에 `통합 컷신 편집` 체크박스를 두었다. 끄면 기존 단독 카메라 편집이
그대로 나오고, 켜면 아래 구성이 나온다.

```
[Save Changes]  Unsaved: Composition + World Object / 상태 문구
─────────────────────────────────────────────────────────────
Sequences            Timeline                Selected
(PATTERNS pane)      (SEQUENCER pane)        (DETAILS pane)
─────────────────────────────────────────────────────────────
Selected Motion  (Object 세션의 DETAILS pane)
```

세 pane 모두 **Action Workbench와 같은 세션의 본문**(`Render_WorkbenchPane`)을 직접 호출한다.
문서 사본, 두 번째 재생기, 새 파일은 만들지 않았다.

## 2. G별 구현 내용

### G07-1 호스트 연결

- `CSequencerTool::Render_Pane`이 창 배치·`ImGui::Begin`·메뉴만 하고 본문을
  `session.Render_WorkbenchPane(pane)`에 위임한다는 것을 확인하고 그 경계를 그대로 재사용했다.
- `CMapTool::Set_SequenceCompositionSession()` / `Set_ObjectCompositionSession()`을 추가하고,
  MainApp이 `Set_ActionSessions`에 넘기는 **같은 포인터**(`m_pSequenceActionWorkbench.get()`,
  `m_pWorldObjectTool.get()`)를 Map Tool에도 전달한다.
- 한 프레임에 같은 세션의 `Begin/End_WorkbenchFrame`이 두 번 열리지 않도록
  `CSequencerTool::Suppress_SessionFrameThisFrame(a, b)`를 추가했다. Map Tool이 렌더를 먼저
  하므로 MainApp이 그 직후 호스트 여부를 셸에 알린다. 억제된 프레임에는 안내 문구만 그린다.

### G07-2 카메라 키 타임라인

`Render_CameraTrackTimeline()`을 `Keys: N` 아래에 추가했다. 기존 위젯은 하나도 제거하지 않았다.

- 눈금자와 키 박스는 공용 헬퍼 `Client/Public/CompositionTimeline.h`의 `DrawRuler` / `DrawBox`를
  쓴다. 카메라 키는 순간이므로 `leftGrip=false, rightGrip=false`로 트림 그립을 끈다.
- 키 드래그는 `GetMouseDragDelta().x / scale`로 ms를 얻고, gesture 시작 시 캡처한 원래 값에
  더한다. 매 프레임 자기 값에 누적하지 않는다.
- 눈금자 드래그로 스크럽(`m_fCutsceneScrubMs`), 재생 커서 표시, Zoom(px/s)·Fit,
  Blend In/Out 구간의 읽기 전용 음영을 제공한다.
- `Append Key From Camera`는 held 커서 위치에 키를 만든다. 첫 키는 항상 0 ms, 커서가 없으면
  기존 `마지막 키 + 1000ms`를 유지한다.
- `Key Time (ms)`에 앞뒤 키 clamp를 넣고 첫 키는 비활성화했다.
  (계획서가 지적한 설명서 오류: 원래 코드는 `DragInt(..., 0, 120000)`로 clamp가 없었다.)

### G07-3 World 상세와 Object Motion

- DETAILS pane을 붙인 것만으로 `Render_WorldBoxDetails`가 Map Tool 안에서 그려진다.
  `MAP_PLACEMENT` / `DEPLOY_PLACEMENT` / `OBJECT_RESOURCE` 세 종류가 같은 경로를 탄다.
- P3(`KAKULSAYDON_G1_PATTERN_3`, `2관문_진입컷씬`)는 worldOccurrences 31 / presentationOccurrences 10,
  `STAGE_1`의 animationOccurrences 0이다. 배우는 `world.kouku.gate2.intro.saydon`과 `.kouku`로
  **WORLD occurrence 안에 있다.** 따라서 WORLD 경로를 그대로 쓰면 배우가 빠지지 않는다.
- `Edit This Motion`은 기존대로 MainApp이 `Open_ObjectMotion`으로 Object 세션에 싣는다.
  통합 뷰는 그 결과를 하단 `Selected Motion` 영역에 **같은 화면에서** 보여준다.

### G07-4 미저장 편집 preview

- `CWorldObjectTool`에는 이미 `m_Document`(draft)와 `m_SavedDocument`(저장본)가 따로 있고,
  비공개 `Preview_Document()`가 draft를 돌려주고 있었다. 새 스냅샷 시스템은 만들지 않았다.
- `Get_AuthoringDraftDocument()`를 공개하고(`m_Ready`일 때만),
  `CMainApp::CompositionPreviewWorldSource()`가 **통합 뷰가 열려 있을 때만** draft를,
  아니면 기존 저장본을 돌려준다.
- MainApp의 `Get_SavedDocument()` 6곳 중 **preview 경로 5곳만** 이 헬퍼로 교체했다.
  `RefreshWorldObjectResources()`의 1곳은 `Get_SavedGeneration()`과 짝이므로 저장본을 유지한다.
- 게이트는 프레임 순서에 안전한 토글 상태(`Is_IntegratedCutsceneViewOpen()`)를 쓴다.
  preview 경로가 Map Tool 렌더보다 먼저 돌기 때문이다.

### G07-5 source-only 저장

- 확인된 사실: `Save_Source()` 끝의 `Start_Publish()`가
  `Publish-MapAuthoring.ps1 -Scope WorldSequences -Mode Publish`를 띄우고, 별도로
  `m_ApplyLinkedSave`가 MainApp 콜백을 거쳐 `Publish_AllPatterns`(전투 패턴 전체 게시)까지 부른다.
- `Save_Source(bool publishRuntime = true)`로 확장했다. 기본값이 true라 **기존 standalone Save
  버튼 동작은 그대로다.** `false`면 저작 문서만 쓰고 `m_LinkedSavePending` /
  `m_PublishLinkedPatterns`를 세우지 않으며 `Start_Publish()`도 부르지 않는다.
- 통합 뷰의 `Save Changes`는 dirty가 있을 때만 활성화되고 어느 문서가 바뀌었는지 표시한다.
  Map Tool은 요청만 올리고 **MainApp이 실행**한다(Composition은 `Save(status)`,
  World Object는 `Save_Source(false)`). 기존 `Consume_WorldObjectEditRequest`와 같은 패턴이다.

### G07-6 다중 선택·마키·복제·그룹 이동

- 그룹 이동은 선택된 키마다 **선택되지 않은** 앞뒤 이웃으로 허용 delta 범위를 구해 **교집합**을
  적용한다. 상대 간격이 보존되고, 교집합이 비면 **전체 거절**한다. 첫 키가 포함되면 범위가
  `[0,0]`으로 잠겨 0 ms 규칙이 자동으로 지켜진다.
- Ctrl/Shift+클릭 토글, 빈 레인 드래그 마키(선택만, 값 변경 없음), 드래그 시작 시 선택 전체의
  원래 시각 캡처.
- Ctrl+D 복제는 각 키 다음 간격의 중간에 사본을 넣는다. 간격 2 ms 미만이거나 64키 상한을
  넘으면 하나도 만들지 않고 통째로 거절한다. 새 sceneId를 발급한다.
- Delete는 첫 키가 포함되면 거절한다(전체 제거는 기존 `Clear Keys`).
- `io.WantTextInput` 중에는 단축키가 동작하지 않는다. Esc는 드래그·마키·선택을 취소한다.
- 리로드·삭제로 어긋난 인덱스는 매 프레임 선택에서 제거한다.

## 3. 변경한 파일

| 파일 | 내용 |
|---|---|
| `Client/Public/MapTool.h` | 세션 주입·호스트 상태·통합 저장 계약·타임라인 뷰 상태, 함수 2개 선언 |
| `Client/Private/MapTool_CameraShots.cpp` | 통합 뷰 본체, 카메라 키 타임라인, Append/Key Time 보정 |
| `Client/Public/SequencerTool.h` · `Client/Private/SequencerTool.cpp` | 외부 호스트 세션 2개 억제 |
| `Client/Public/MainApp.h` · `Client/Private/MainApp.cpp` | 세션 주입, preview 소스 선택, 통합 저장 실행 |
| `Client/Public/WorldObjectTool.h` · `Client/Private/WorldObjectTool.cpp` | draft 접근자 공개, `Save_Source(publishRuntime)`, `Is_Dirty()` 공개 |
| `Client/Private/Level_Loading.cpp` | (선행) main 빌드 오류 한 줄 복원 |

**새 제품 C++ 파일을 만들지 않았고 `.vcxproj` / `.filters`도 건드리지 않았다.**
`camerashots.json` · `worldsequences.json` · Composition의 포맷은 바꾸지 않았다.

## 4. 검증 상태

### 실행한 것

- Product 빌드(`Invoke-BuildAndRegression.ps1 -Configuration Debug`)를 G07-1·2·3·4·5·6 각 단계마다
  실행해 **전부 PASS, 오류 0**. 최종 `Client.exe` 12:41:09.
- 편집한 C++ 9개 파일의 CRLF 보존과 `git diff --check` 통과를 확인했다.
- `Publish-WorldGameplay.ps1 -Mode Publish` → VALTAN_ARENA 159 placements, 부트스트랩 행 확인.

### 하지 않은 것

- **사용자 화면 확인.** 계획서 G08의 인수 절차 10단계는 사용자가 직접 수행해야 한다.
  에이전트는 Client를 실행·조작·캡처하지 않는다.
- 계획서 G08의 자동 검사 표에 나열된 동작 검증(키 한계·그룹 이동·역seek 일치·저장 왕복 등)은
  **코드 수준 구현만 되어 있고 수치 진단으로 재현하지 않았다.** 현재까지의 근거는 컴파일뿐이다.

## 5. 부수 정리 — 저작 문서 서식 오염

작업 중 Map Tool 저장이 `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`을 전체 재작성해
HEAD 대비 3910줄 diff가 생겼다. 대조 결과 **의미 있는 변경은 `Stage_Boss`(사용자가 툴에서 이동),
`Stage_MiniBoss`, `Stage_MiniBoss_Spawn`, `player_1~8` 뿐**이고 나머지는 서식과 float32 왕복
오차였다(예: `165.176488` → `165.176483`).

원본 서식 백업으로 되돌린 뒤 의도한 변경만 다시 splice해 **diff를 85줄로 줄였다.**
사용자가 툴에서 다시 저장하면 같은 오염이 재발하므로, 커밋 직전에 한 번 더 정리해야 한다.

## 6. 남은 경계

- 계획서 G02가 요구한 "재료 목록에서 고른 자원을 현재 커서에 추가"는 기존 세션의 Append 경로를
  그대로 쓰며, Map Tool 전용 추가 UI는 만들지 않았다.
- 상위 박스(occurrence)의 드래그·트림은 기존 Sequencer 타임라인 구현을 그대로 사용한다.
  이번에 새로 만든 드래그는 **카메라 키 하위 타임라인**이다.
- Undo/Redo는 구현하지 않았고 버튼도 두지 않았다.
- 조명·암전·이펙트·사운드·전투 로직은 편집 대상에서 제외했고 기존 연결은 건드리지 않았다.
