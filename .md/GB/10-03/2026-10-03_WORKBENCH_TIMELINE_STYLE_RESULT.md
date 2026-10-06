# Boss·Object·World Movie 타임라인 양식 통일 결과

## G03. 2026-10-06 Movie 종류별 레인 구조 통일

Movie의 객체별·배우별 제목/접기 행을 제거하고 Animation/World/Material/Effect/Sound/Camera/Light/
Time Control의 종류 이름을 왼쪽에 한 번만 표시하도록 바꿨다. 같은 종류의 시간 box는 오른쪽에
배치하며 겹치는 구간만 하위 표시 행을 사용한다. 전체 phase 동안 존재하는 모델이 많으면 World
하위 행도 그 수만큼 생기며, 모델을 합치거나 실제 유지 시간을 줄이지 않는다.

`CompositionTimeline::DrawCategoryLane`을 Movie·Valtan·Kouku와 Sequence가 함께 사용한다.
종류 라벨·배경·경계와 하위 행의 좌표가 공통이며, 행/box/ruler와 hit 영역도 같은
`GetCompactRowMetrics`를 사용한다. Valtan의 기존 최소38/32px도 공통 compact 규칙으로 바꿨다.
폰트13px의 기본 style에서는 행26/box22px, 폰트26px에서는 행36/box32px이며 DPI를 중복 적용하지 않는다.
배우·모델·리소스는 box/tooltip/Box Detail에서 구별하고 원본 row·actor/slot·box ID는 유지했다.
`World Model`은 표시 이름만 `World`로 바꾸며 실제 source kind와 inspection 연결은 그대로다.

Movie 시퀀서에 Save/Play/Pause/Stop/Reset을 같은 순서로 배치하고 기존 Movie 명령에 위임했다.
Reset은 선택 Intro/Loop의 시작에서 일시정지한다. 별도 전체 폭 slider를 제거하고 공통 ruler에서
시간을 탐색한다. Search/Active at cursor·mute/solo·Box Detail·리소스 열기와 편집 허용 범위는 유지했다.
저작 JSON, resource, 원본 timing, publisher, 실행 중 draft와 저장된 rendering option은 변경하지 않았다.

### 실행한 검증

- 실제 `CompositionTimeline.h`와 ImGui4개 CPP를 컴파일한 headless probe:43 checks PASS.
  종류 라벨1회·시간축X·서브행Y·다음 종류 시작점·13/26px 폰트, actor 혼합 비중첩 재사용/중첩 분리,
  stable ID·원본 시간·최소box 폭·hold tail·saved group을 확인했다. OS/Client/UI 창을 만들지 않았다.
- 기존 `test_valtan_composition_input_native`:37 native checks PASS.
- Movie의 `Select_Box`, `Sync_WorldSelection`, `Queue_Seek`, `End_WorkbenchFrame`,
  `Update_TimelineGesture`, `Save_Authoring` 함수 본문은 작업 시작 HEAD와 SHA2566개 전부 일치했다.
- 기존 occurrence timing 정적 검사14개 중12개 PASS,2개 FAIL이다. 변경 전 HEAD에도 동일한
  `Append to Stage Slots` 버튼 문자열과 `!Cue.bUsesStageClock` 문자열 기대가 실패함을 대조했다.
  새 실패는 없으며 전체 테스트가 통과했다고 기록하지 않는다.
- 최종 Debug Product build/deploy PASS: `20261006T032313928Z-debug-product.json`.
  Engine/Shared/Server/Client를 확인했고 Client 단계36325ms, OBJ3/EXE1 갱신, tracking 변경 없음이다.
  기존 인코딩/외부 debug PDB 경고는 남아 있으며 컴파일·링크 오류는 없다. Runtime 파일·Navigation·
  Item/Valtan catalog 검사도 통과했고 domain publish는 하지 않았다.
- 수정 C++4개 UTF-8 BOM 없음·CRLF와 `git diff --check`를 확인했다. 새 파일/project/filter/JSON/XML
  변경은 없으며 기존 사용자 untracked 자료는 보존했다.

증거는 `out/MovieSequencerLayout20261006/`의 `run.log`, `result.json`, `preserved-contracts.json`,
`timing-contract-final-comparison.json`, `boss_metrics_source_receipt.json`, `product-debug-final.log`에 있다.
이 날짜의 전체 H/CPP 반영본은 대응 PLAN의 G06에 보존한다.

### 남은 사용자 화면 확인

최신 변경은 Debug EXE에 반영했다. 이번 변경의 Release 빌드와 Client/UI 실행·조작·캡처는 하지 않았다.
사용자는 Action Workbench의 World → Character Select → Composition Sequencer에서 Intro/Loop를
열고, Boss/Sequence와 종류별 배치·행 높이·World 여러 box·선택·ruler seek·편집을 확인한다.
실제 촬영 화면 확인과 저장/재생 동작의 사용자 확인을 자동 검증 완료로 대신하지 않는다.

아래 G00~G02는 10-03 당시 반영 기록이며 현재 Movie 구조는 위 G03을 따른다.

## G00. 반영 범위

Boss/Sequence의 기존 공통 표시 기준을 Object와 World Movie에도 적용했다.
라벨 열180px, 행24px, 박스22px, ruler24px이며 Animation/World/Presentation 등 종류 색을 공유한다.
CompositionTimeline의 라벨 clipping·Fit 배율과 화면 밖 box 그리기 생략을 공통으로 소비한다.

Object의 단일 Motion과 묶음/부모 overview는 왼쪽 kind·slot·Motion 접기 열, 오른쪽 공통 시간축을
사용한다. Motion bar 선택과 Stage 끝 조절, Animation/Effect/Collider box, Transform diamond,
Physics 표시가 같은 규격을 사용한다. 줌 배율을 canvas 최소 폭과 분리해 슬라이더 값이 실제 px/s다.

World Movie는8종류와 actor별 접기를 왼쪽 열로 정렬하고 Zoom1~500, Fit, Ctrl휠 cursor 기준 확대,
ruler drag 후 release 시 시간 이동을 제공한다. 각 display row마다 전체 box를 다시 순회하던
탐색은 한 번 만든 refsByLane 조회로 바꿨다. camera cut reorder/shared boundary, source↔movie
시간 변환, row dirty/publish guard와 EndFrame의 deferred command 소비는 보존했다.

Object/Movie의 저작 문서와 저장 owner는 그대로다. 데이터 schema·stable ID·폴더, 원래 loop와
MOTION_END 편집 제한은 바꾸지 않았다. 새로운 다중 선택/복사 기능을 구현한 것으로 설명하지 않는다.

## G01. 검증

- 실제 공통 header를 사용하는 Fit/크기 native probe:18377 checks, failures0.
- Object 공통 좌표와 실제 생산 시간변환 식의 native probe:110 checks, failures0.
- WorldObjectTool.cpp 독립 Debug TU compile: exit0.
- Movie의 EndFrame·seek·gesture·selection·authoring·inspection 기존 함수 본문6개 보존을 hash 비교로 확인했다.
- 기존 Object 선택/그룹 편집 위임·linked save CAS·preview 선택 생명주기·paused live edit:4개 PASS.
  그룹 편집 source guard는 실제 Render_TimelineRows 위임을 검사하도록 낡은 직접본문 탐색을 교정했다.
- 최신 main의 project/filter XML5개와 endpoint JSON2개 parse, Debug/Release endpoint 유지 확인.
- C++ UTF-8 BOM없음/CRLF 유지와 `git diff --check` 확인.

정식 Debug Product Build는 `20261003T053410131Z-debug-product.json`에서 PASS/exit0이다.
Engine·Shared·Server·Client를 확인했으며 Client는 OBJ55개와 EXE1개를 갱신했다.
정식 Release Product Build도 `20261003T053612726Z-release-product.json`에서 PASS/exit0이다.
Release Client 단계는85597ms이며 OBJ52개·EXE1개를 갱신했다.
기존 C4819/C4828·외부 debug PDB 등 경고는 남아 있고 컴파일·링크 오류는 없다.
런타임 파일과 Navigation·Item/Valtan catalog 검사를 실행했으며 데이터 publish는 하지 않았다.

증거는 `out/WorkbenchTimelineStyle20261003/geometry_probe.log`, `product-debug.log`, `product-release.log`,
`out/WorldObjectTimelineParity20261003/timeline_geometry_probe.run.log`와 compile log다.
Movie 함수 보존 근거는 `out/WorldMovieTimelineParity20261003/preserved-contracts.json`이다.
쇼타임의 별도 CPU 병목 수정과 수치는 대응 `SHOWTIME_SEQUENCER_PERFORMANCE_RESULT.md`에 있다.

## G02. 사용자 확인

현재 Debug·Release EXE에 반영됐지만 Client/UI는 자동 실행하지 않았다. 실제 도킹 화면에서 Object 단일·묶음,
World Intro/Loop의 접기·선택·drag/trim·ruler seek·Fit을 사용자가 확인해야 한다.
쇼타임을 같은 구간에서 다시 재생해 저장한 새 Profiler capture가 최종 FPS 비교 근거다.
기존 원본 캡처·직접 저장한 Composition JSON·개인 변경과 backup 파일은 보존했다.
