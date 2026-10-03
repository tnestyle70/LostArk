# Boss·Object·World Movie 타임라인 양식 통일 구현 계획

## G00. 범위

사용자의 도구 촬영을 위해 Action Workbench의 Boss, Object, World Movie 타임라인 표시를 맞춘다.
Sequence는 이미 Boss와 같은 CKoukuSaydonActionWorkbench(true)를 사용한다. Object와 World Movie는
서로 다른 저작 문서·시간 변환·저장 owner를 유지한 채 공통 CompositionTimeline 표시 계약을 소비한다.

기준은 현재 Boss의 LabelWidth180, LaneHeight24, BoxHeight22와 종류별 색이다. 기존 font-aware
32/38px helper로 전체 Boss 화면을 확대하는 작업은 포함하지 않는다. Object의110px/32px/25px,
Movie의230px/24px/20px 차이를 없애고 왼쪽 종류·actor·motion 라벨과 오른쪽 시간축을 정렬한다.

## G01. 공통 표시 도우미와 Boss

`Client/Public/CompositionTimeline.h`에 라벨 열 clipping과 Fit 배율 계산을 공통 helper로 둔다.
기존 DrawBox는 화면 밖 도형 생성을 생략하고 그리기와 hit test의 기존 크기는 바꾸지 않는다.
Boss의 Fit도 공통 계산을 사용한다. 새 C++ 파일과 project/filter 등록은 필요 없다.

## G02. Object

`WorldObjectTool.cpp`의 Render_TimelineRows, Render_GroupSequence, Render_Sequence가 공통
열·행·박스 규격과 종류 색을 사용한다. 묶음에도 라벨 열을 확보하고 접기·Motion 선택은 왼쪽에 둔다.
그리기/InvisibleButton/선택 테두리/키 중심/Stage 끝 조절/스크롤 높이를 같은 좌표로 맞춘다.
줌의 배율과 canvas 최소 폭을 분리하고 Fit과 Ctrl휠 시간 위치를 일관되게 계산한다.

Motion local time에서 startDelay/playbackSpeed 변환, revision 검사, 드래그 후 검증·commit,
MOTION_END와 loop·Transform 끝 키 제한은 그대로 사용한다. 저작 JSON·폴더·stable ID를 바꾸지 않는다.

## G03. World Movie

`SequencerTool.cpp`의 CClassSelectionWorkbenchSession이 같은 열·행·박스와 색을 사용한다.
8종류 및 actor별 접기와 표시 필터를 유지하며 줌1~500, Fit, Ctrl휠, ruler 시간 선택을 연결한다.
Movie/source 시간 변환과 기존 deferred command, dirty draft와 publish guard를 유지한다.
Camera body 이동은 컷 재정렬이며 trim은 공유 cut boundary 변경이다. 이 의미를 일반 box 이동으로
바꾸지 않는다. 이번 양식 통일을 새로운 다중 선택·복사 또는 schema 통합으로 확대하지 않는다.

## G04. 검증과 인도

공통 Fit/시간 좌표, 원래 편집 함수와 guard 연결, 수정 TU Debug 컴파일을 확인한다.
이미 검증된 성능 수정과 Object 초기 로드·팀 LAN 수정까지 정식 Debug Build로 반영하고
최신 main을 통합한 PR을 생성해 사용자 요청대로 merge한다. 직접 저장한 무관한 저작 변경과
개인 파일은 보존한다. Client 자율 실행은 하지 않고 사용자 실제 화면/FPS 확인을 미완료로 구분한다.
