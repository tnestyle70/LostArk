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

## G06. 관문 진입 전 광기 게이지와 저장 위치 확인 (2026-09-27)

`Level_KakulSaydonArena::Update`에서 Server가 복제한 각 player snapshot의 위치를
기존 `Is_KoukuArenaStartArea`에 전달하여 시작 발판의 자신과 동료 광기 게이지를 숨긴다.
관문으로 이동한 뒤에는 기존 madness snapshot과 미로·춤 숨김 정책을 소비한다. snapshot이
아직 없으면 숨긴다. Client 예측 위치나 별도 진입 latch는 만들지 않는다.

첫 접근의 jump.2부터는 시작 발판 영역 밖이므로 Server currentGate가 0이거나 아직
수신 전이면 계속 숨긴다. Server가 관문을 활성화한 뒤 표시하되, 보스 관문 없이 진입하는
MARIO snapshot과 Server 승인된 F1 player-only 진입의 본인은 기존 표시를 허용한다. currentGate는 시작점 복귀 때 유지되므로 각
플레이어의 발판 영역 검사도 함께 적용한다. G3 테라스의 기존 표시는 유지한다.

저장한 높이의 현재 정본은 `Data/UI/KoukuSaydon/KoukuHudModes.json`의
`madness.feetOffsetMeters`다. Save와 생성자의 Load_Config가 같은 ProjectDataRoot를
소비하는지 확인하고, 사용자 저장본 복사본으로 저장→새 인스턴스 로드와 다른 필드 보존을
검증한다. 1.3m는 character Transform의 발 위치에 더하는 월드 높이다. 기존 G02의
headOffsetMeters 설명은 이후 발 기준 전환 이전의 계약이다.

기존 C++의 관문 조건과 위치 기준 주석만 수정하며 project/filter 추가는 없다. 변경된
Client CPP를 격리 출력으로 컴파일하고 JSON parse·diff 검사를 수행한다. 실행 중인
사용자의 Debug 검증을 유지하며 최종 Product 링크와 사용자 화면 판정은 별도로 기록한다.
