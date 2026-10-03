# Boss·Object·World Movie 타임라인 양식 통일 결과

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
