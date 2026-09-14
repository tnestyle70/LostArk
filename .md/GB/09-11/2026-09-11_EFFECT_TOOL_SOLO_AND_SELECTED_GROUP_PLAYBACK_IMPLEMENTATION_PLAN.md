# Effect Tool Solo 즉시 재생과 선택 그룹 반복 재생 구현 계획

작성일: 2026-09-11. 기존 `codex/kouku-gate1-sequence-playback` 작업 트리의 다른 변경은 보존한다.

## G00. 현재 호출과 변경 범위

복원 문서의 Element `Solo`는 `Try_SoloElement -> Try_PreviewElementTimeline ->
CEffectAuthoringSequencer::Preview_Element`를 호출한다. 마지막 commit이 `m_Paused=true`여서
사용자가 Sequencer의 Play를 추가로 눌러야 한다. 성공한 Solo는 같은 원본 시작 시각에서
즉시 재생하도록 바꾼다.

Current Effect의 Ctrl/Shift 클릭은 이미 stable Element ID 집합 `m_MarkedElementIds`를
토글한다. Detail 선택과 draft를 보존하는 이 동작을 유지하고, 일반 클릭 후 첫 modifier 클릭은
이미 선택된 행도 marked 집합에 포함한다. Play All 바로 오른쪽에
`Play Group`을 추가한다. marked 집합을 한 번 복사하여 임시 preview의 입력으로 사용한다.
그룹 재생 중 행 선택을 바꿔도 현재 재생 집합은 바뀌지 않는다.

## G01. Tool의 선택과 문서 투영

`Client/Public/Effect_Tool.h`, `Client/Private/Effect_Tool.cpp`와
`Client/Private/Effect_Tool_Workspace.cpp`를 수정한다. 여러 Element ID를 받는 동일한
preview document projection을 Solo와 Play Group이 공유한다. 선택된 요소를 원본 순서로
보존하고 source particle provider와 model cue anchor를 함께 유지한다. 바깥 runtime history는
현재 단일 Solo처럼 제거한다. 원본 Document, visibility, 저장 ID와 schema는 변경하지 않는다.

입력 ID와 admission, 현재 draft, 원본 시작/종료 시각을 먼저 검증한다. 최소 시작 시각부터
선택 요소의 최대 종료 시각까지 같은 document clock으로 재생한다. 실패하면 기존 미리보기와
선택을 보존하고 오류 문구를 표시한다. 다른 문서 로드가 성공하면 이전 임시 preview와 marks를
정리한다. 로드 실패는 기존 상태를 보존한다.

## G02. 단일 Sequencer 재생 경로

`Client/Public/EffectAuthoringSequencer.h`와 기존 `EffectAuthoringSequencer.cpp`,
`EffectAuthoringSequencer_Tracks.cpp`, `EffectAuthoringSequencer_Timeline.cpp`,
`EffectAuthoringSequencer_Resources.cpp`의 Element preview ID를 선택 ID 목록으로 확장한다.
기존 V1 factory, Stage_Row, Sample_Row, Play, Refresh_Effects와 Stop 경로를 재사용한다.

선택 그룹은 하나의 transient occurrence이며 각 반복에서 동일한 원본 시각을 다시 샘플링한다.
V1 factory가 draft를 적용한 선택 문서의 시작/종료 시각을 반환하여 Refresh도 같은 구간을 갱신한다.
그룹의 loop 값은 임시 row가 소유하여 Stop이나 다른 preview에서 해제된다. 기존 saved sequence의
loop 설정과 내용은 보존한다. 선택 preview는 Append/Save 대상으로 승격하지 않는다.
Solo도 성공 commit 시 paused=false로 시작하고 준비에 소비된 첫 프레임 delta는 계속 건너뛴다.

## G03. 등록과 검증

새 C++ 파일은 없다. 기존 project/filter 등록을 유지한다. 수정한 C++ translation unit을
Debug 설정으로 최소 compile하고 Client가 실행 중이므로 link하지 않는다. 변경한 코드의
호출/실패/Stop/문서 교체 경계와 `git diff --check`를 확인한다. 저장 JSON/XML은 변경하지 않는다.

Client/UI 자동 실행·조작·캡처는 하지 않는다. 사용자 확인 경로는 F1 → Effect Tool → Current
Effect에서 Solo 클릭, Shift로 두 개 이상 행 표시 → Play Group 클릭, 반복 관찰, Stop,
다른 문서 열기 순서다. 실제 입력과 시각 판정은 사용자 확인으로 남긴다.

## G05. Current Effect의 본 부착 그룹과 공동 offset (2026-09-14)

Current Effect에서 `Group by anchor` 표시를 추가한다. 저장된 `ActionCueAttachment`의 enabled/follow, modelCue, runtime bone·binding, orientation과 socket basis를 기준으로 그룹을 파생하며 새 저장 그룹 ID나 runtime 부모를 만들지 않는다. 양손 발사 섬광은 b_wp_2 11개와 b_wp_1 11개로 보이고 disabled attachment/root는 별도 묶음이며 같은 독립 root basis일 때 공동 위치 편집을 허용한다. 알려진 SourceModelPreview actor `MN_RPCT_05`의 source-model b_wp_2/b_wp_1만 Left hand/Right hand를 함께 표시하고 다른 actor에는 이 의미를 추정하지 않는다. 그룹마다 Element 행을 펼쳐 기존 개별 Detail/Solo를 사용할 수 있고 Play Group은 기존 선택 Element preview pipeline으로 연결한다.

본 그룹은 `Group Center (bone-local m)`, disabled attachment의 독립 root 그룹은 `Group Center (effect-local m)`로 절대 공동 중심을 표시한다. 위치 입력은 멤버 `Detail.Transform.position`의 평균이다. 새 중심과 이전 중심의 차이를 모든 멤버 위치에 더하고, position lerp가 켜졌으면 endPosition에도 같은 delta를 더한다. 런타임의 S*R*T 뒤 공통 parent 곱을 확인했으므로 서로 다른 Element 회전·scale은 수정하지 않는다. source transform track, master transform inheritance 또는 RuntimeCarrier처럼 해당 로컬 위치를 대체하는 owner는 그룹 위치 편집을 거절하고 기존 owner에서 편집하도록 상태를 표시한다. SourceContract 문서는 읽기 전용이다.

변경 파일은 기존 Effect_Tool_Internal.h/Helpers.cpp의 파생 그룹·순수 이동, Effect_Tool_Detail.cpp의 검증·commit·그룹 입력, Effect_Tool_ResourceBrowser.cpp의 Current Effect 표시와 기존 행 렌더 공통화, Effect_Tool.h의 private 선언·UI 토글이다. 또한 Playback.cpp의 Stage_WorldPreview와 Workspace.cpp의 기존 factory 문서 선택을 연결한다. 미적용 Detail draft가 있으면 그룹 이동을 비활성화하여 Apply/Revert로 먼저 기존 입력을 보존한다. 모든 멤버를 검증한 candidate 하나만 Try_CommitDocument로 commit하고 Save Changes가 기존 문서 전체 저장 경로를 사용한다. 사용자 JSON을 에이전트가 직접 변경하지 않는다.

실제 signature 22개에 대해 그룹별 11개, 한 그룹 이동·다른 손 보존, 상대 offset·source recipe·runtime anchor ID 불변, 위치 lerp·상이한 S/R에서 공동 delta의 정확성, SourceContract/invalid 입력 실패 보존, 실제 Codec 저장·재로드를 확인한다. 변경 TU는 out 격리 Debug 최소 컴파일한다. 제품 빌드와 Client 화면 확인은 상위 작업 및 사용자 조작으로 연결한다. 새 C++/프로젝트 항목은 없다.

Stage_WorldPreview가 sequencer를 갱신하는 동안 호출자가 넘긴 candidate 문서를 scope로 지정한다. factory는 이 문서를 먼저 복사하고 이전 active Detail draft를 다시 덮어씌우지 않는다. 동일 V1 asset만 stage/sample한 뒤 성공하면 기존 Try_CommitDocument가 active 문서를 교체하며 실패·예외에는 scope가 이전 포인터를 복원한다. 일반 Play factory의 기존 draft overlay는 유지한다. 이 연결은 그룹 입력의 첫 Enter부터 새 위치를 사용하고 실패 시 기존 문서·preview를 보존하기 위한 것이다.

## G06. 본 그룹 정렬용 저장 Object 참조 (2026-09-14)

Effect의 sourceModelPreview는 actor와 animation만 담고 있으며 기존 Model Reference는 WORLD를 지운다. 따라서 총구에 맞추려면 같은 actor의 저장된 손 소품을 명시적으로 참고하는 연결이 필요하다. EffectAuthoringSequencer의 기존 Model View Pattern 선택과 새 Show saved hand props 체크를 사용한다. 이름이 중복된 Pattern은 stable ID를 함께 표시한다. 체크는 현재 세션의 Play All/Play Group에만 적용하며 Effect 문서와 sequence 저장 형식을 바꾸지 않는다.

EffectCompositionModelPreview::Select_SourceEffect는 명시 Pattern ID가 있을 때만 정본 Composition에서 같은 actor/target의 BOSS Object resource와 occurrence를 읽는다. source animation 창과 Object placement는 보존하고 Effect/Logic/맵은 재생하지 않는다. KoukuSaydonPresentationPlayer는 opt-in Model Reference에만 기존 WORLD player를 연결한다. actor pose와 preview root를 확정한 뒤 같은 시각의 소품을 갱신한다. 누락·불일치·유효하지 않은 참조는 이유를 보존하고 다른 Pattern을 추정하지 않는다.

기존 EffectAuthoringSequencer.h/.cpp, EffectCompositionModelPreview.h/.cpp, KoukuSaydonPresentationPlayer.h/.cpp를 확장한다. 새 C++ 파일과 project 등록은 없다. 개별 TU 컴파일과 실제 저장 Pattern의 소품 필터·local TRS 보존을 검사한다. 사용자 Effect JSON과 실행 중 편집 내용은 변경하지 않고 최종 제품 빌드는 저장·종료 후 수행한다.

### G06 WORLD 총구용 중립 Effect의 실제 Object anchor

추가 요청한 gun.muzzle.world도 defaultAnchorKind=WORLD이지만 특정 worldId가 없으므로 G08 Append 수정에 포함된다. 이 리소스와 signature.world는 내부 본 부착과 sourceModelPreview가 없다. 모델만 함께 표시하는 것과 Effect의 실제 발생 원점을 총으로 선택하는 것을 구분한다.

Model View의 명시 Pattern에서 저장 Object anchor 목록을 읽고 정확한 World occurrence를 선택한다. 중립 Effect의 임시 preview target은 해당 Pattern의 기존 모델·총을 준비하고, Resolve_Root가 실제 생성된 총의 sampled pivot를 소비한다. 기존 history가 과거 총 pose를 기록해 world-space 방출도 유지한다. 내부 본 부착이 있는 양손 원본을 이 총 root에 다시 붙이려는 선택은 거절한다. 원본은 Source model 선택에서 내부 본 그룹을 편집한다.

Current Effect의 root 그룹은 기존 Element 위치 전체를 공통 delta로 이동한다. 위치는 Effect JSON의 기존 local transform에만 저장하고 미리보기의 특정 총 선택은 세션 상태다. 제품에서는 Append한 Box Detail이 어떤 총 WORLD에 붙는지 소유한다. Effect 자체에 이미 반영한 총구 위치를 Box offset으로 중복 적용하지 않는다. 기존 사용자 박스 offset은 외부에서 자동 변경하지 않는다.

## G07. 양손 Play All의 저장 총 자동 참조 (2026-09-14)

사용자는 양손 원본의 본 부착을 유지하고 Effect Tool에서 Left/Right 그룹의 위치만 각각
조절하는 흐름을 최종 선택했다. 이 방식은 현재 그룹 편집과 Save Changes로 지원한다.
한 손으로 분리해야만 가능한 구조가 아니다. 기존 그룹 중심·본·회전·크기·저장 Composition은
그대로 유지하며 Play All이 총을 함께 준비하는 연결을 완성한다.

`EffectCompositionModelPreview.h/.cpp`의 `Resolve_SourcePropPattern`은 원본 typed Composition에서
정확한 Effect asset을 참조하고 source actor/Gate/target이 일치하는 Pattern만 조사한다.
기존 WorldSequence reader와 actor-bound prop 검사로 지원 소품이 있는 후보를 확인한다.
현재 양손 참조 P32에는 WORLD가 없고 P35만 좌우 총을 가지므로 P35가 선택된다.
후보 없음은 기존 source model만 재생하며, 여러 유효 후보나 잘못된 참조는 이유를 표시하고
기존 미리보기를 보존한다. 이름·asset·P35 ID 하드코딩과 별도 모델 런타임은 추가하지 않는다.

`EffectAuthoringSequencer.cpp`의 `Select_SceneEffectTarget`은 source model Effect의 자동
소품 문맥을 위 함수로 준비한 뒤 기존 `Select_SourceEffect`에 전달한다. 명시 Pattern 소품
선택은 우선한다. 내부 본 Effect는 앞서 한 손 미리보기에서 선택한 단일 총 WORLD root를
상속하지 않는다. source animation과 양손 부착은 계속 Effect 원본이 소유한다.
Model View 문구는 자동 source props와 명시 Pattern 선택의 차이를 표시하고, 재생 중
참조하는 Pattern 및 소품 수를 기존 미리보기 패널에서 확인할 수 있게 한다.

현재 `Group Center`는 입력값 변경 시 반영되므로 Enter로만 적용한다는 이전 안내를 교정한다.
신규 C++ 파일·JSON 필드·project/filter 등록은 없으며 기존 등록과 인코딩을 유지한다.
실제 선택 함수의 양손 자동 문맥, 이전 한 손 선택 잔여값, 한 손 수동 경로, 후보 없음·중복·
실패의 기존 상태 보존을 집중 검사한다. 관련 TU 컴파일 후 정규 Debug Product Build로
EXE를 반영한다. 실행 중 Client의 사용자 편집을 보존하며 화면 조작·최종 총구 정렬은
사용자가 `발사 섬광 양 손 → Play All → Current Effect → Group by anchor → Left/Right
Group Center → Save Changes`에서 확인한다.
