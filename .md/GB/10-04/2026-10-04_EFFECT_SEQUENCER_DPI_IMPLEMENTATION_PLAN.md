# Effect Sequencer DPI 행 높이 수정 구현 계획

## G00. 현재 실측과 목표

`Effect_Tool.cpp`의 Effect Action Benchmark는 `CEffectAuthoringSequencer::Render_Sequencer`를
호출한다. 같은 함수는 V2 Sequencer와 Character의 embedded composition에서도 사용한다.
ImGuiLayer는 기본16px 폰트와 `ConfigDpiScaleFonts`를 사용한다. DPI로 커진 현재 폰트와 달리
Timeline.cpp는 행24px, 박스·InvisibleButton19px를 사용하여150%의24px 글자가 세로로 잘린다.
현재 렌더링 해상도·Windows 설정·저작 JSON과 관계없는 저작 도구의 표시 geometry 문제다.

목표는 현재 ImGui 폰트와 style을 매 프레임 읽어 행·박스·눈금·라벨·hit test를 같은 기준으로
맞추는 것이다. 모니터 DPI를 다시 곱하지 않는다. 기존 시간값·stable ID·행 배치·드래그 후보와
release 시 검증·적용 계약은 보존한다.

## G01. 공통 높이 계산

`Client/Public/CompositionTimeline.h`에 `ROW_METRICS`와 `GetCompactRowMetrics`를 추가한다.
`boxHeight`는 기존 공통22px와 현재 글자 높이+style FramePadding 양쪽의 최댓값이다.
`laneHeight`는 기존24px와 박스+style ItemSpacing의 최댓값이며, `rulerHeight`는 기존24px와
글자 높이+현재 DrawRuler의 위아래3px 공간의 최댓값이다. 각 값은 픽셀 단위로 올림한다.
기존 상수와 Valtan에서 소비하는32/38px helper는 유지하여 다른 편집기의 높이를 바꾸지 않는다.
`GetCompactLabelWidth`는 현재 표시하는 트랙 이름의 실제 글자 폭+양쪽8px 공간과 기존180px 중
최댓값을 구한다.200%에서도 긴 종류 이름을 온전히 표시하며 시간축의 px/s는 바꾸지 않는다.

## G02. 실제 Sequencer 소비

`Client/Private/EffectAuthoringSequencer_Timeline.cpp`의 `Render_Sequencer`가 metrics를
한 번 구한다. Ruler의 draw와 입력 영역에는 rulerHeight, 겹친 occurrence의 배치와 총 스크롤
높이에는 laneHeight를 사용한다. Box는 행 안에서 세로 중앙에 놓고 같은 사각형으로 draw,
화면 밖 생략, InvisibleButton을 구성한다. 종류 이름과 빈 행 안내는 현재 폰트 높이로 중앙 정렬하고
각각 라벨 열·시간축의 rect 안에서 clip한다. 상단 Timeline 라벨도 rulerHeight 안에서 표시한다.

드래그 시간 변환·양 끝 trim·재생 cursor·선택 ID·camera 소유권·저장 코드는 변경하지 않는다.
미호출 `Draw_CameraRows`나 별도 Boss/Object/Movie editor는 이번 실제 owner 수정에 포함하지 않는다.
새 CPP/H 파일이 없으므로 project/filter 등록은 필요 없다.

## G03. 검증과 완료 경계

- 실제 공통 header와 ImGui를 사용하는 비UI geometry probe에서100/125/150/200% 폰트 크기,
  row/box/ruler 높이, 한글·영문 glyph clip, hit rect와 행 분리를 확인한다.
- 수정 Timeline TU를 별도 out 경로에 컴파일하고 기존 시간·드래그·commit 코드 보존을 비교한다.
- 파일 인코딩·줄바꿈과 `git diff --check`를 확인한다. 결과는 별도 RESULT에 실제 실행 내용만 기록한다.
- Client/UI 자율 실행, Windows DPI 변경, imgui.ini 수정, 사용자 저작 데이터 교체와 제품 바이너리
  덮어쓰기는 하지 않는다. 사용자 화면 확인과 제품 빌드·반영은 후보 검증과 구분한다.
