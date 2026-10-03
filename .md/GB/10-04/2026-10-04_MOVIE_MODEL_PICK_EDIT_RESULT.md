# World Movie 선택 모델 위치 편집 결과

## G00. 연결과 편집 소스

Scene pick 또는 Movie world models에서 선택한 actor가 기존 World Model Box Detail에 연결된다.
이전에는 inspection highlight만 바뀌었고 Workbench의 selected row/box는 바뀌지 않았다.
읽기 전용 inspection state의 process-local selection generation으로 같은 actor를 다시 고른
동작도 전달한다. stable instance/slot ID로 현재 class·Intro/Loop의 box를 찾는다.

미적용 row와 진행 중 drag가 있으면 새 선택을 대기시키고 현재 초안·선택 키를 보존한다.
Apply/Save/Revert 후 대기 선택을 처리하며 class·phase가 바뀌면 오래된 요청은 폐기한다.
scene pick의 mesh 정보를 지우지 않도록 이미 선택된 inspection에 SELECT를 다시 보내지 않는다.

World Model의 Position key와 Position offset(m) 입력, 명시적인 Nearest key to cursor를 추가했다.
스크럽은 편집 키를 자동으로 바꾸지 않는다. Translate this phase(m)의 Stage 버튼은 현재
Intro 또는 Loop track의 모든 위치 키에 같은 delta를 더한다. 회전·scale·visibility·time·slot과
다른 phase/actor를 바꾸지 않는다. 고급 quaternion 등 기존 키 입력은 All model track fields에 있다.

이 값은 기존 sequence-local positionOffset이다. Inspector의 World XYZ와 구분하며 instance와
anchor 변환 전에 적용된다. actor726은 기존 WORLD·instance position0 계약을 사용한다.
새 offset/schema나 모델 배율·재질 수정 없이 기존 Apply_AuthoringBox, Save Movie, WorldSequences
Publish를 소비한다. Apply row가 재생을 중지하는 기존 계약을 유지했다.

수정 파일은 `SequencerTool.cpp`, `ClassMovieInspection.h`, `ClassSelectionPresentation.h`,
`ClassSelectionPresentation_Inspection.cpp`다. 선택 generation은 JSON에 저장하지 않는다.
연계된 Inspector의 Drawn 표기는 별도 root 변경으로 Draw enabled가 되었으며 표시 허용과
material opacity를 구분한다.

## G01. 자동 검증

- 실제 생산 함수 EditMovieModelPosition, Sync_WorldSelection, Select_Box를 추출하고 실제
  DataJson parser와 생산 Serialize/Equal 및 World transform validation 함수를 사용한 native
  probe: **2588 checks, failures0**. 데이터 fixture는 실제 actor726 Intro281키와 Loop2키다.
- 전체 key의 같은 delta, 단일 key 수정, 무관한 key 필드와 stable slot 보존, 직렬화·parse 왕복,
  NaN/Infinity/범위 초과/없는 key/마지막 key 오류 때 부분 변경 없음이 통과했다.
- 실제 WorldSequencePlayer_Objects의 position과 world matrix 생성 식을 추출하여 instance
  offset, 회전된 anchor, identity WORLD에서 sequence-local delta가 합성되는 방향을 확인했다.
- scene 선택, 동일 actor 재선택, 일반 frame의 선택 키 유지, 다른 timeline 선택의 비침범,
  dirty/drag 중 대기와 종료 후 처리, class 변경·없는 stable ID·아직 없는 timeline 경계를 확인했다.
- SequencerTool, ClassSelectionPresentation_Inspection, ClassMovieInspector의 개별 Debug TU
  컴파일이 완료되어 OBJ를 생성했고 오류는 없다. 기존 포함 헤더의 C4819 경고는 남아 있다.
  이후 표시 함수의 매-frame 전체 key 사본을 const reference로 바꾼 최종본은 native probe에
  포함했고 root의 정식 Product 빌드가 이를 확인한다.
- 네 제품 파일의 UTF-8 BOM 없음·CRLF와 대상 git diff --check를 확인했다. 새 C++/JSON/XML
  파일 또는 project/filter 등록은 없다.

증거는 `out/MovieModelPickEdit20261004/`의 `build_probe.py`, `cases.cpp.txt`, `probe.run.log`,
`provenance.json`, `compile_tus.cmd`와 각 `*.compile.log`에 있다. provenance는 실제 source와
추출 함수의 SHA256을 보관한다. 검증용 파일만 out 아래 생성했다.

## G02. 제품 반영과 사용자 화면 경계

사용자의 EXE 종료·빌드 승인 후 root가 Debug/Release Product 통합 빌드를 진행한다.
정식 빌드·PR 결과는 통합 작업에서 별도로 기록한다. 이 문서의 native probe는 파일 저장
전체 transaction 또는 실제 GPU 화면·마우스 조작 검증을 대신하지 않는다.

이 작업에서 저작 Data와 runtime JSON을 교체하지 않았다. 사용자의 Camera·Effect·Composition·
mapplacements 편집값과 저장된 렌더링 옵션은 그대로 유지한다. Client/UI는 에이전트가 실행하거나
조작하지 않았다. 실제 actor726 위치 조절·Play/seek·Save/Publish 후 재입장 화면은 사용자 확인이다.
