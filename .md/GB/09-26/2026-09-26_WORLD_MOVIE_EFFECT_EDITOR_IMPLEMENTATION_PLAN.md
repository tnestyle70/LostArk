# World Movie의 Effect Tool V1 편집과 런타임 카메라

## G00. 목표와 현재 연결

`All Effects → World → Character Selection Movies`의 Open Editor는 현재 WORLD Sequencer만
열고, Effect 박스에서 V1을 열면 Movie를 Stop한다. V1은 Movie 배우와 독립적인 모델 preview를
사용한다. Camera Box Detail의 key와 실제 pose는 읽기 전용 표시이고 일반 Apply row는 재생을
중지한다. 이 작업은 같은 Movie owner를 V1 element 편집과 camera key 편집에서도 유지한다.

정본은 기존 `Data/Camera/ClassSelection.cinematics.json`, SL00 WorldSequences와
`Data/Effects/Authored/*.effect.json`이다. 원본 데이터의 외부 교체나 별도 Movie player를 추가하지
않는다. 기존 09-25 FOUR_CLASS_SELECTION_MOVIES PLAN/RESULT의 admission·저장 계약을 따른다.

## G01. V1 Movie 편집 진입과 재생

`Effect_Tool.h`, `Effect_Tool_Workspace.cpp`, `Effect_Tool_ResourceBrowser.cpp`, `Effect_Tool.cpp`와
`MainApp.cpp`에서 typed class/phase/Effect ID를 연결한다. Open Editor는 선택 Movie의 실제
Effect 문서를 V1 Current Effect에 열고 element 목록·Detail을 노출한다. Movie의 Intro/Loop
Effect 목록을 선택해 같은 V1 문서를 전환한다. 기존 미저장 문서 보호를 유지한다.

Play All·Pause·Stop·seek는 Level 소유 `CClassSelectionPresentation`에 전달한다. V1 Model View는
해당 Movie의 재생 상태와 Effect 목록을 표시하고 기존 WORLD 배우·애니메이션을 함께 사용한다.
카메라/다른 row 편집은 같은 WORLD Sequencer를 연다. 서로 다른 모델 preview가 Movie를 덮지 않는다.

## G02. Element draft의 Movie 반영

`Effect_Tool_Playback.cpp`의 stage와 기존 ClassSelection/Effect presentation 경로를 연결한다.
검증된 V1 draft는 해당 Movie의 Effect target에만 임시 적용하며 배우·카메라·시계는 유지한다.
실패는 현재 문서와 재생을 보존하고 이유를 표시한다. Save는 기존 V1 저장·prepared target 갱신을
사용한다. Movie 편집 종료·다른 문서 이동과 Level 종료의 임시 override 정리를 확인한다.

`Effect_DocumentRenderer`의 기존 prepared resource 빌더로 catalog에 게시하지 않는 불변 target을
준비한다. `Effect_PresentationService`는 기존 level-owned active/pending handle을 현재 transform
history에서 검증한 뒤 교체한다. `ClassSelectionPresentation`이 target 수명을 소유하고 다음
Intro/Loop·seek spawn에 전달한다. active 교체는 기존 visibility와 handle을 유지하고 scene 비용은
교체 후 집합으로 한 번 계산한다. `Effect_Object.h`의 visibility 조회는 이 보존에만 사용한다.

## G03. Camera Box Detail

`SequencerTool.cpp/.h`, `ClassSelectionPresentation.cpp/.h`,
`ClassSelectionPresentation_Authoring.cpp`에서 선택한 camera key의 Eye·LookAt·FOV를 편집한다.
Camera-only Apply는 유효한 동일 stable row를 기존 scene/draft에 반영하고 현재 Movie 시각에서
카메라를 다시 평가한다. 재생 token·pause·phase를 유지하며 Save movie는 기존 freshness 병합을 쓴다.
다른 row의 full admission과 실패 보존 경로는 유지한다.

## G04. 검증과 인계

기존 파일의 인코딩을 유지한다. 새 C++ 파일을 만들지 않으므로 project/filter 등록 변경은 없다.
정상 Debug Product 증분 Build, 관련 native 편집 검증의 재사용 가능 범위, `git diff --check`를
확인한다. 원본 JSON을 직접 수정하지 않으며 저장 검증은 임시 사본에서 한다. RESULT에는 실제
실행한 검사와 미실행한 UI 확인을 분리한다. 사용자 화면 경로는 World Movie Open Editor →
Effect/element 선택 → Detail 변경 → Play All 및 Camera Box Detail 위치 변경 → Save movie다.
