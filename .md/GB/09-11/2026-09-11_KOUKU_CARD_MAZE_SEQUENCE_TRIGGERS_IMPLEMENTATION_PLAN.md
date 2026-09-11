# 쿠크 카드미로 Sequence Trigger 연결 구현 계획

## G00. 현재 데이터와 목표

`KAKULSAYDON_G1_PATTERN_28`의 `logic.43` occurrence 네 개는 3744/4182/4529/4824ms이고 `logic.40` 진입은 5136ms다. 두 정의는 TRIGGER 이름만 있어 실행 의미가 없다. 기존 occurrence ID와 시간을 보존하고 `CARD_MAZE_HIDE_NEXT`, `CARD_MAZE_ENTER` typed trigger로 연결한다.

첫 hide에서 살아 있고 전투 준비가 된 플레이어를 월드 X 내림차순, 동률 PlayerId 오름차순으로 확정한다. 이후 한 occurrence가 한 명을 숨긴다. 고정 2관문 카메라에 대한 오른쪽 순서이며 카메라 회전에 따라 서버 순서를 바꾸지 않는다. 마지막 trigger는 기존 미로 중앙 `(0.28, -0.01, 1351.65)`의 서버 navigation과 collision을 검증한 뒤 참가자 전원을 같은 tick에 이동·표시하고 MAZE HUD를 켠다. 이후 중앙 Q 망원경·문양·행진 경로는 기존 서버 로직을 소비한다.

## G01. Data, composition 저장과 publish

기존 `KoukuSaydonCompositionDocument.cpp`, `KoukuSaydonActionWorkbench.cpp`, `project_kouku_saydon_composition.py`, `Publish-GameplayBalance.ps1`에서 두 trigger를 읽고 검증·저장·projection한다. ENTER의 `teleportPosition`만 데이터로 소유하며 HIDE에는 타 종류 값이 허용되지 않는다. 같은 bundle의 플레이어 상태 소유자 충돌 검사에도 두 종류를 포함한다. 기존 Gate1 composition 두 logic 정의를 국소 수정하고 기존 publisher로 실제 실행 데이터를 만든다.

## G02. Server 상태와 실패 보존

기존 `GameplayCatalog` mechanic trigger enum/parser를 확장한다. `KoukuSaydonLogicRuntime` ledger가 참가자 순서와 이미 소비한 순번을 소유한다. cue는 시간순으로 처리해 늦은 tick에서도 hide→enter 순서를 보존한다. ENTER는 목적지/참가자를 먼저 stage하고 하나라도 실패하면 기존 위치를 모두 유지하며 hidden을 해제하고 실패 이유를 출력한다. Stop/abort/discard에서도 이 ledger 참가자의 hidden을 해제한다. 중간 사망·퇴장자는 다른 참가자의 ordinal을 바꾸지 않는다.

## G03. Shared와 Client 소비

`CARD_MAZE_PRESENTATION.flags`의 bit16을 진입 연출 hidden으로 정의한다. wire 크기는 유지하되 구버전이 snapshot을 거부하므로 protocol을 79로 갱신하고 writer/reader의 유효 flag 범위를 함께 변경한다. `ClientReplication`은 승인 snapshot hidden을 `Character`에 적용하며 Character는 숨겨진 프레임의 body/equipment render queue를 만들지 않는다. 표시 상태는 장비 개별 visibility를 변경하지 않아 다시 나타날 때 기존 장비 설정을 보존한다.

## G04. 검증과 사용자 확인

새 C++ 파일이 없어 project/filter 등록은 필요 없다. 기존 composition test와 Server card-maze contract, NetworkProtocolHarness에 실제 새 계약 검사를 더한다. 최소 Debug 컴파일, 관련 테스트, JSON parse와 `git diff --check`를 실행한다. 전체 빌드는 다른 에이전트와 중복하지 않는다. Client/UI 실행·조작·캡처는 하지 않는다. 사용자 확인 경로는 Lobby → KoukuSaydon → 2관문 → Action Composition Workbench의 카드미로 패턴 Complete Play이며 4명 숨김 순서와 미로 중앙 진입 후 Q 진행을 확인한다.

## G05. 기존 dissolve 구간과 사용자 저작 보존

publish에 함께 들어가는 기존 pattern15에는 dissolve 1/1 값이 있다. 실제 `CEffectV2Object::Dissolve_Amount`는 같은 정규화 lifetime 값을 이미 안전하게 처리한다. 1/1은 끝까지 dissolve-out 없음이고 같은 1 미만 값은 해당 시점의 즉시 전환이다. parser, Workbench, composition projector와 group playback 검증을 이 기존 계산식에 맞춰 equality를 허용하고 역전 구간만 거부한다. 사용자 저작 값을 교정하거나 runtime dissolve 계산식을 바꾸지 않는다.
