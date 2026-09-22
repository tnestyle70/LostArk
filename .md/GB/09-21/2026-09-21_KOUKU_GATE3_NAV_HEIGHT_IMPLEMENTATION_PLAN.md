# 2026-09-21 쿠크세이돈 3관문 시작 지점 navigation 조사 계획

## G00. 질문과 판정 기준

사용자는 3관문 시작 지점에서 낮은 전투 바닥 대신 위쪽 navigation이 선택되어
플레이어가 움직이지 않는다고 보고했다. 이 조사는 상부 셀이 실제 바닥인지,
장식 actor가 baker의 최고 높이를 오염시킨 것인지, 그리고 Server의 detail-region
선택이 낮은 바닥을 가리는지를 `navsource`, `navpaint`, placement, Server 소비자와
대조한다. 임의로 상부 셀을 차단하거나 높이를 평탄화하지 않고, 실제 authored 이동과
낙사 착지 계약을 보존할 수 있을 때만 source/paint와 runtime을 갱신한다.

## G01. 입력과 확인 순서

1. `Data/Navigation/LV_LUT_MIDNIGHTC_ED.Gate3Fine.navsource`의 높이·walkable
   footprint를 계산하고 `navpaint`의 기존 의자/장식 차단과 비교한다.
2. `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`,
   `Shared/Public/Gameplay/KoukuArenaReadyAreas.h`,
   `Server/Private/ServerNavigation.cpp`, `GameRoom_PlayerCommands.cpp`,
   `GameRoom_PlayerSimulation.cpp`의 authored entry/ground 이동 계약을 확인한다.
3. 대기 테라스 내부 경로와 테라스에서 낮은 전투장으로의 경로를 분리해 측정한다.
   서로 다른 공간을 일반 A*로 연결하는 것은 authored entry/teleport 계약을 먼저
   확인한 뒤에만 변경 후보로 삼는다.
4. 후보가 source 오선택으로 판정되면 MapTool의 stable placement와 WModel 입력을
   보존한 재베이크 후보를 만들고, 최신 저장본 CAS 확인 뒤에만 publish한다.

## G02. 현재 판정과 보류 조건

기존 fine-nav 문서는 Gate3Fine을 `중앙 전투 원 + 입장 전 고지대`로 정의하고,
두 공간은 실제 바닥 footprint가 겹치지 않으며 authored 입장 이동으로 연결한다고
기록한다. `Is_KoukuGate3EntryTerrace`도 x[-30,-5], y[23.5,28], z[947,972]의
상부 테라스를 명시한다. 따라서 이 조사에서 상부 전체를 BLOCKED로 칠하는 후보는
거부한다. 낮은 전투장으로의 진입이 실제 사용자 요구라면 navsource가 아니라
Server의 typed gate-entry/teleport 소비자를 별도 증거와 함께 고친다.

## G03. 종료 증거

원본 authoring/runtime hash 보존, source 높이 통계와 범위, 기존 paint 보존,
high-terrace 내부 및 terrace→combat 경로 결과, regular `C2S_MOVE` 소비 경로,
`git diff --check`를 기록한다. 변경 후보가 없으면 publisher를 실행하지 않고
“nav 데이터 변경 없음, 진입 전환 계약 조사 필요”를 RESULT에 명시한다.
