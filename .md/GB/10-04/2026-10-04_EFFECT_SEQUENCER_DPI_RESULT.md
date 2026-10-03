# Effect Sequencer DPI 행 높이 수정 결과

## G00. 확인한 원인과 소스 반영

Effect Action Benchmark의 실제 owner는 `Effect_Tool.cpp`가 호출하는
`CEffectAuthoringSequencer::Render_Sequencer`다. ImGui는 현재 모니터에 맞춰 폰트를 확대하지만
이 함수의 행24px·박스19px는 고정이었다. 현재 설치에서 쓰는 Malgun Gothic으로 실제 ImGui
정점을 비교하면150%의 동일 글자 높이가 정상19px에서 기존 박스14px로 잘리는 것을 재현했다.

`CompositionTimeline.h`에 현재 font/style에서 compact box·lane·ruler 높이와 트랙 이름 열 폭을
계산하는 helper를 추가했다. Timeline은 그 결과를 draw·clip·화면 밖 생략·InvisibleButton·
행 배치·스크롤 높이에 함께 사용한다. 종류 이름과 상단 Timeline 이름은 세로 중앙에 표시한다.
라벨 열은 실제 표시 이름 폭+양쪽8px와 기존180px 중 큰 값을 사용한다.

현재 기본 폰트16px와 style에서 계산한 픽셀 높이는 다음과 같다.

| 배율 | 현재 폰트 | 박스 | 행 | 눈금 |
|---|---:|---:|---:|---:|
|100%|16|22|26|24|
|125%|20|26|30|26|
|150%|24|30|34|30|
|200%|32|38|42|38|

현재 폰트가 이미 DPI를 반영하므로 Windows 배율을 다시 곱하지 않는다. 같은 함수를 재사용하는
V2 Sequencer와 Character embedded composition에도 적용된다. 별도 Boss/Object/World Movie
editor와 Valtan의 기존32/38px helper는 바꾸지 않았다.100% Effect 행은24px에서26px로 늘었다.

## G01. 자동 검증

- 실제 공통 header와 ImGui, 설치된 Malgun Gothic을 사용한 비UI probe:255 checks, failures0.
  100/125/150/200%에서 영문·한글 glyph의 전체 높이 보존, draw/hit rect 일치, 겹친 occurrence의
  행 분리, 좌우 trim과 move 판정, ruler 입력 높이, regular6/embedded13 트랙 이름의 clip과
  시간 좌표 역변환을 확인했다. 실제 Timeline의 metrics·label·geometry·draw·hit·ruler 문장6개를
  그대로 추출하고 source/header SHA256을 기록했다.
- `EffectAuthoringSequencer_Timeline.cpp` 독립 Debug TU 컴파일 exit0. 기존 C4819 및 변경하지
  않은 occurrence duration의 C4244 경고는 남아 있다. 제품 EXE/DLL 링크·배포는 하지 않았다.
- 기존 authoring mutation 함수 전체, toolbar, occurrence 데이터 투영, ruler seek, 선택·drag,
  release 시 validate/commit의6개 코드 구간을 HEAD와 hash 비교하여 보존을 확인했다.
- 두 기존 C++ 파일의 UTF-8 BOM 없음·CRLF 유지와 대상 `git diff --check`를 확인했다.
  새 C++ 파일·JSON·XML 변경과 project/filter 추가는 없다.

Geometry 증거는 `out/DPISequencer20261004/review/run.log`, `source_receipt.json`,
`geometry_probe.cpp`, `extract_owner.py`, `compile.cmd`에 있다.
TU와 기존 코드 보존 증거는 `out/EffectSequencerDPI20261004/timeline/compile_timeline.log`,
`compile_timeline.cmd`, `preserved-contracts.json`에 있다.

## G02. 사용자 확인과 반영 경계

Client/UI 실행·조작·화면 캡처, Windows DPI 변경, imgui.ini 변경을 하지 않았다.
사용자가 편집 중인 Effect·Camera·Composition·mapplacements 등 저작 데이터와 저장된 렌더링
값은 수정하지 않았다. 이 결과는 소스 후보와 비UI 검증 완료이며 제품 빌드·반영 및 사용자의
150% 실제 화면에서 Animation/Effect 행과 드래그 확인은 별도다.

## 후속 통합 제품 반영

2026-10-04 사용자 종료·빌드 승인 뒤 Debug와 Release Product 빌드·배포를 모두 완료했다.
이 문서의 변경도 해당 실행 파일에 포함된다. 두 receipt와 검증 경계는
[Movie 통합 반영 결과 G04](2026-10-04_MOVIE_CAMERA_SAVED_POSE_REBASE_RESULT.md#g04-통합-제품-빌드)에 기록했다.
Client/UI를 자동 실행하지 않았으며 최종 화면 확인은 사용자에게 남는다.