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
