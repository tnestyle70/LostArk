# 쿠크 앵콜 진입과 광기 UI 구현 계획

## G00. 현재 경로와 변경 범위

`codex/kouku-timeline-local-preview`의 Server는 이미 정식 레이드와 단독 G3 처치에서
false-clear 5초 후 BINGO intro, intro 종료 후 빙고 전투를 자동 실행한다.
`Level_KakulSaydonArena::Update_GateProgress`가 이 전이 사이에 ENTER_BINGO를 선택하는
잔여 UI를 없앤다. 준비 실패를 Client 임의 입장으로 숨기지 않는다.

## G01. 진입 UI와 카운트

G3 clear부터 Bingo 전투 전까지 관문 버튼을 NONE으로 두고 기존 투표 창을 닫는다.
Server 소유의 클리어/앵콜 시퀀스와 READY 준비 계약은 유지한다.
오른쪽 G3 진입 오라 카운트를 화면 아래 중앙으로 옮기고 크기를 절반으로 줄인다.
안내 문구도 같은 묶음으로 이동한다.

## G02. 광기 게이지 위치와 미로 표시

`CKoukuMadnessGaugeView`에 화면 기준 offset X/Y와 기존 headOffsetMeters의
조회·preview·저장 경로를 추가한다. F1 Kouku UI Preview가 Level의 typed 함수로
자신과 동료 게이지를 함께 조정한다. 저장은 `KoukuHudModes.json`의 madness 위치
필드만 최신 저장본에 병합하고, 같은 필드 충돌은 거절한다. 원자 교체·백업·freshness를
확인하여 무관한 HUD mode 설정을 보존한다. 실제 gauge 값은 Server snapshot을 유지한다.

동료 게이지가 health 값만으로 임시 state를 만들면서 MAZE 정보를 버리던 경로를
수정하여 카드 미로에서는 자신과 동료 게이지 모두 숨긴다.

## G03. 검증과 파일 소유권

기존 C++ 파일만 수정하므로 project/filter 등록은 추가하지 않는다. MainApp.cpp는
RenderKoukuUiPreviewControls만 수정하고 로딩 전환 담당의 변경을 보존한다.
Client Debug/Release 컴파일은 통합 담당과 순서를 맞춘다. 구조/JSON/diff 검증을
기록하며 Client 실행과 최종 위치·연출 화면 판정은 사용자에게 남긴다.

## G04. KillBoss 이후 standalone 관문 전이 회귀 검증

통합 담당이 Server의 legacy advance/restart를 pinned raid preparation으로 연결한다.
기존 `ServerGameplayContractTests_KoukuRaid.cpp`에2~4인 각각의 G1 advance→G2,
G2 advance→G3, G1/G2/G3 restart를 추가한다. 이전 실제 boss placement와 clear
소비자를 사용하고, partial vote/READY에서 이동·spawn이 없으며 FAILED 시 기존
player·boss·gate 상태가 보존되는지 검증한다. 성공은 실제 게시된 intro ID의
CINEMATIC 진입으로 판정하며 곧장 combat 좌표로 teleport하지 않는지도 확인한다.

## G05. 후속 World Object Save 표기

사용자의 후속 요청에 따라 World Object Tool의 공통 저장 버튼과 목록 상단 저장 버튼은
dirty 여부와 관계없이 `Save`로 표시한다. 별도 `Unsaved local changes` 상태 문구와
저장·검증·자동 Publish 동작은 기존 경로를 사용한다. 같은 파일에서 진행 중인 다른
Object hierarchy 변경을 보존하고 두 버튼의 label 표현식만 바꾼다.
