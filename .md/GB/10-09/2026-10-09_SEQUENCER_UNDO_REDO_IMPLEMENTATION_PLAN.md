# Sequencer의 Boss·Sequence·World 편집 이력 통일

## G00. 현재 기준과 범위

기존 `codex/bingo-authoring-controls`의 `f283d4d42`와 `origin/main`의 `0164be5a9`를
`codex/sequencer-undo-redo`의 `8f00e13a6`으로 충돌 없이 병합했다. 빙고 기능을 보존한다.
Boss Saydon과 Sequence는 기존 `CKoukuSaydonActionWorkbench`의 독립 편집 이력을 이미 사용한다.
World Movie는 `CClassSelectionWorkbenchSession`에서 같은 Sequencer 창을 사용하지만 이력이 없다.
이번 변경은 이 누락을 기존 Movie 저작 owner와 공용 `CEditorUndoHistory`에 연결한다.

## G01. Movie 저작 owner

`ClassSelectionPresentation.h`와 `ClassSelectionPresentation_Authoring.cpp`에서 manifest/world
초안과 stable 선택·Movie cursor를 한 편집 상태로 기록한다. Box Apply, 시간 drag/trim,
카메라 cut 재배치, Repeat movie, Movie 배우 제외·복원을 기록한다. 카메라 연속 입력은 입력이
끝날 때 한 단계로 확정한다. 선택만 바꾸거나 Play/Pause/Stop/Seek를 실행한 경우에는 이력을 만들지 않는다.

Undo/Redo는 기존 stable-ID 병합으로 해당 편집의 변경 필드만 현재 draft에 적용한다.
현재 world revision과 최신 저장 baseline은 보존한다. 같은 필드 충돌·유효하지 않은 문서·resource
준비 실패는 현재 draft와 두 이력 stack을 보존한다. 기존 Validate/Prepare/Commit 경로를 사용하고
별도 player나 publisher를 만들지 않는다. 일반 Save는 이력을 유지하며 Reload 성공과 Level 종료는
해당 문서 이력의 경계다. 게시 중에는 Undo/Redo를 막는다. 메모리 복원은 디스크를 쓰지 않는다.

## G02. Sequencer 입력과 선택 복원

`ClassSelectionTimeline.h`는 class/phase/kind/box/key의 stable ID와 Movie millisecond cursor를
전달하는 선택 상태를 선언한다. `SequencerTool.h/.cpp`는 World의 재생 toolbar에 Undo/Redo와
Ctrl+Z, Ctrl+Y, Ctrl+Shift+Z를 연결한다. `MainApp.cpp`의 기존 callback이 Level 소유 Movie owner에
명령을 전달한다. 미적용 row와 활성 drag·텍스트 입력은 보존하며, 처리 시점은 기존 프레임 종료다.
복원 성공 때만 선택과 cursor를 갱신한다. Effect asset 내부 편집은 기존 Effect Tool 이력이 소유한다.

## G03. 검증과 전달

기존 H/CPP만 확장하므로 project/filter 신규 등록은 없다. 기존 Movie native 검사와 공용 history
검사를 재사용하여 편집·Undo·Redo, 저장 이후 Undo, 외부 변경 보존·동일 필드 충돌, 실패 후 재시도,
Reload 이력 경계를 확인한다. 정상 Debug Product 증분 Build와 `git diff --check`를 실행한다.
실제 데이터 파일을 교체하거나 Client/UI를 실행하지 않는다. 자동 검증과 사용자 화면 확인은 RESULT에서
구분하고 Animation Tool 인계 문서에 지원 범위와 단축키를 반영한다.
