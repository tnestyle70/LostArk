# World Movie 선택 모델 위치 편집 구현 계획

## G00. 실제 대상과 누락 경계

현재 선택한 무기는 `world.object.classselect.warlord.a726.p0`이며 모델은
`Character/Warlord/Cinematics/ClassSelect/Original/Parts/warrior_sword.slot0.wmodel`이다.
독립 WORLD actor의 Intro281개·Loop2개 transform key를 기존 WorldSequences 문서가 소유한다.
기존 `Get_AuthoringBox`는 World Model의 stable instance/slot ID를 transform track으로 resolve하고
`Apply_AuthoringBox`는 검증·준비 후 기존 Movie 초안에 적용한다.

Scene pick과 Inspector 선택은 `Inspection.selectedId`와 노란 highlight만 바꾼다. Timeline click과
달리 Workbench의 selected kind/row/box에 전달되지 않아 Box Detail 편집 연결이 빠져 있다.
현재 raw key editor의 positionOffset 입력은 있으나 특정 키만 고치면 다음 키에서 다시 이동한다.

## G01. 선택과 미적용 초안

`ClassMovieInspection.h`의 읽기 전용 state와 `ClassSelectionPresentation.h`의 process-local state에
선택 generation을 둔다. `ClassSelectionPresentation_Inspection.cpp`의 SELECT/SOLO/FOCUS가
성공한 새 선택을 표시한다. 동일 actor를 다시 고르는 동작도 이벤트로 구분하며 저장하지 않는다.

`SequencerTool.cpp`는 새 inspection 선택의 stable ID를 같은 class·Intro/Loop의 World Model box로
resolve하여 기존 `Select_Box`를 사용한다. 미적용 row나 drag가 있으면 선택 요청을 보관하고
현재 값·선택 키를 유지한다. Apply/Save/Revert 후 요청을 이어서 처리하며 class·phase가 달라진
요청은 적용하지 않는다. 조회만으로 모델·시간·저장값을 바꾸지 않는다.

## G02. 위치 편집 UI

같은 `SequencerTool.cpp`의 World Model row editor에 source key 시간, 선택 키의
`Position offset (m)` XYZ와 명시적인 `Nearest key to cursor` 선택을 제공한다. 스크럽 중
자동으로 키를 바꾸지 않는다. 값은 World XYZ 관측값이 아니라 기존 sequence transform key의
positionOffset임을 표시한다. 현재 actor는 WORLD anchor와 instance position0을 사용한다.

`Translate this phase (m)` XYZ는 현재 track의 모든 positionOffset에 같은 delta를 적용한 row
후보를 만든다. 다음 키에서도 상대적인 보정이 유지되고 Intro/Loop는 서로 별도다. 시간·회전·
scale·visibility·slot ID와 다른 actor는 그대로 보존한다. 새 model offset/schema와 pivot 회전은
추가하지 않는다. 회전과 나머지 고급 키 입력은 기존 raw editor에 남긴다.

기존 Apply row와 Save Movie, WorldSequences Publish를 재사용한다. Apply는 현재 계약대로
재생을 중지하며 Play/seek로 확인한다. 파일 병합·동시 저장 충돌·generation 검증을 우회하지 않는다.

## G03. 검증과 반영 경계

실제 위치 편집 helper와 선택 처리 코드를 추출한 비UI 검증으로 actor726의281/2키에서 delta,
키 한 개 수정, 무관한 필드 보존, 잘못된 입력 거절, dirty/drag 중 선택 보존과 stable ID 전환을
확인한다. 실제 CDataJson 왕복과 현재 WorldSequence parser 계약, 수정 TU 컴파일 및 diff check를
확인한다. 새 CPP/H 파일은 없으므로 project/filter 등록은 필요 없다.

Data·runtime JSON, 현재 사용자 초안, Client/UI와 git 상태는 에이전트가 변경하지 않는다.
소스 후보·컴파일·비UI 검증과 실제 사용자 화면 확인 및 제품 반영을 RESULT에서 분리한다.
