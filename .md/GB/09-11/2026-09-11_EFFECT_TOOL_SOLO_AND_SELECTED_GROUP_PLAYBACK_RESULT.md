# Effect Tool Solo 즉시 재생과 선택 그룹 반복 재생 결과

작성일: 2026-09-11. 작업 브랜치는 `codex/kouku-gate1-sequence-playback`이다.

## G00. 구현 상태

복원 문서의 Solo와 Timeline Solo가 성공하면 Sequencer를 paused=false로 시작한다. 기존
원본 시작 시각에서 바로 재생하며 준비 작업에 소비된 첫 프레임 delta를 계속 건너뛴다.

Current Effect의 Play All 오른쪽에 Play Group을 추가했다. Ctrl/Shift 클릭으로 marked한
Element ID 목록을 복사하여 단일 transient occurrence로 즉시 반복 재생한다. 일반 클릭으로
A를 고른 뒤 Shift 클릭으로 B를 고르면 A/B가 모두 marked된다. 이후 Ctrl/Shift 클릭은 기존처럼
개별 행을 토글하고 Detail 선택과 draft를 보존한다. 재생 후 marks를 바꿔도 현재 재생 집합은
바뀌지 않는다. Play Group을 다시 누르면 새 선택으로 교체한다.

선택 preview는 기존 `CEffectObject` factory, document projection, Stage_Row, Sample_Row,
Play, Refresh_Effects, Stop을 공유한다. 별도 renderer나 저장 schema는 추가하지 않았다.
선택된 Element와 필요한 source provider를 원본 순서로 보존하고 model cue는 숨긴 anchor로
유지한다. source delay, lifetime, material, resource, attachment와 현재 draft는 그대로 소비한다.

반복 구간은 선택된 drawable Element의 최소 시작부터 최대 종료까지다. 종료 계산은 제품
`CEffectPlayback::Calculate_ElementEndSeconds`를 사용한다. 선택하지 않은 긴 Element,
provider의 긴 lifetime, 전체 model sequence 길이는 선택 반복 구간을 늘리지 않는다.
Refresh가 draft를 다시 투영할 때 시작/종료도 갱신한다. 반복과 Restart는 해당 시작으로 돌아가며
이전 반복의 source anchor history를 재사용하지 않는다.

loop는 임시 row가 소유한다. Stop이나 다른 preview가 row를 해제하면 기존 saved sequence의
loop 설정이 다시 적용된다. 선택 preview는 Append/Save 대상이 아니다. Load 성공, New,
Discard에서 선택 preview를 정리하고 다른 문서의 marks를 이월하지 않는다. 실패한 선택과
Load는 기존 문서와 미리보기를 보존한다.

## G01. 변경 파일과 호출

| 파일 | 변경 책임 |
|---|---|
| `Client/Public/Effect_Tool.h` | 선택 목록 재생, 문서 투영, 재생 구간과 V1 factory 출력 선언 |
| `Client/Private/Effect_Tool.cpp` | Play Group 버튼·marks UX, Solo 공통 진입, 선택 구간 계산, 문서 교체 정리 |
| `Client/Private/Effect_Tool_Workspace.cpp` | draft 적용 후 선택 문서와 시작/종료를 factory에서 반환 |
| `Client/Public/EffectAuthoringSequencer.h` | transient ID 목록·시작 시각·loop 상태 |
| `Client/Private/EffectAuthoringSequencer.cpp` | 즉시 재생 commit, 선택 구간 반복, Refresh와 history 초기화 |
| `Client/Private/EffectAuthoringSequencer_Timeline.cpp` | 임시 loop 제어와 선택 시작 시각 Restart, 목록 label |
| `Client/Private/EffectAuthoringSequencer_Tracks.cpp` | 선택 preview의 Append 금지 유지 |
| `Client/Private/EffectAuthoringSequencer_Resources.cpp` | 선택 목록과 임시 preview 설명 |

`Try_PlayMarkedElementGroup -> Try_PreviewElementsTimeline -> Preview_Elements -> Stage_Row ->
Create_AuthoringOccurrence -> Build_ElementsPreviewDocument/Resolve_ElementsPreviewWindow`가
성공한 뒤 원본 document clock으로 Sample_Row를 실행하고 새 transient를 commit한다.

## G02. 자동 검증

| 실행한 검증 | 결과 |
|---|---|
| 변경 6 CPP Debug 최소 compile | exit 0, 오류 0. OBJ/PDB는 `out/EffectToolSelectedGroup20260911` 전용 |
| 선택 투영·구간 임시 숫자 검사 | exit 0. 원본 순서, 숨긴 cue anchor, 선택 구간, missing/hidden 거부, source provider 보존, 원본 문서 보존 확인 |
| 도화가 31930 실제 provider 의존성 | 선택 dependent와 앞선 simulation-only provider만 유지하고 실제 Playback validator 통과 |
| 쿠크 내려치기 2306/2307 선택 구간 | 930~1830ms. 원본 문서 serialize 전후 일치 |
| 합성 긴 문서의 선택 구간 | 선택 a/b는 2000~6000ms. 선택하지 않은 290000ms 종료 요소가 구간을 늘리지 않음 |
| 인코딩·줄바꿈 | 수정 8개 H/CPP의 UTF-8 BOM 없음과 기존 LF/CRLF 유지 |
| project/filter | 기존 파일 등록 유지, 신규 C++ 파일 없음 |
| JSON/XML | 저장 계약과 해당 입력 파일 변경 없음 |
| `git diff --check` | 성공. 다른 변경의 기존 줄바꿈 변환 경고만 출력 |

컴파일 명령은 `out/EffectToolSelectedGroup20260911/compile.cmd`, 로그는 `compile.log`다.
숫자 검사는 제품의 두 순수 projection/window 함수 본문을 그대로 복사한 임시 probe이며 실제
codec와 Playback helper를 링크했다. 근거는 같은 폴더의 `projection_probe_result.json`과
`projection_probe_source_receipt.json`이다. 이 검사를 실제 Client 입력이나 scene 재생 증거로
표현하지 않는다. Resources 조회에는 `LOSTARK_RESOURCE_ROOT`를 실제 Client/Bin/Resources로
명시했다. 첫 임시 실행의 resource root 누락을 수정한 뒤 성공했다.

## G03. 실행과 사용자 확인 경계

Client가 실행 중이므로 제품 link, 재시작, UI 조작과 화면 캡처는 하지 않았다. root 통합 작업에서
제품 빌드를 마친 뒤 사용자가 F1 → Effect Tool → Current Effect로 확인한다.

1. Solo를 눌러 추가 Play 없이 시작하는지 확인한다.
2. 일반 클릭으로 A, Shift 클릭으로 B를 고르고 Play Group을 누른다.
3. 선택 요소만 원본 상대 타이밍으로 반복하는지 확인한다.
4. Stop, 다른 문서 Load, Play All에서 이전 선택 미리보기가 남지 않는지 확인한다.

실제 버튼 입력과 loop의 화면 결과는 사용자 확인 대기다. commit/push나 다른 세션의 dirty
파일 정리는 하지 않았다.

## G04. 통합 빌드 완료

사용자가 Client/Server를 종료한 뒤 Debug Product의 Engine/Shared/Server/Client compile·link·deploy를
완료했다. `out/BuildPipeline/runs/20260911T073907619Z-debug-product.json`의 모든 build step이
PASS이고 missingRuntimeInputs는0개다. 독립 읽기 전용 코드 검토에서도 즉시 재생·선택 구간·
provider·Stop/문서 교체·저장 sequence 보호의 신규 결함은 발견되지 않았다. 실제 버튼 입력과
화면 반복은 여전히 사용자 확인 대기이며, 실행 경로는
[통합 결과](2026-09-11_KOUKU_GATE1_TWO_PATTERN_FULL_RESTORE_IMPLEMENTATION_RESULT.md)의 마지막 절을 따른다.
