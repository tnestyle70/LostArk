# 쿠크 낙사면과 현재 관문 부활 구현 계획

## G00. 기존 소비자와 작업 범위

현재 `Begin_PlayerFall`/`Update_PlayerFall`은 45틱 뒤 사망시키고, 쿠크 부활은 사망 좌표를
navigation에 투영한다. 비행 넉백은 아래 배경 지형을 착지 대상으로 계속 추적할 수 있다.
사용자가 요청한 기준 바닥 아래 약5m 낙사면과 현재 관문 시작점 부활을 같은 Server 경로로 연결한다.
발탄 강제 이동의 지지면/보행 마스크 분리는 09-23 대응 PLAN/RESULT의 현재 코드를 보존한다.
1·3관문 펜스와 Mario 전용 이동 계약은 유지하며 새 Client 권위 이동이나 wire 상태를 만들지 않는다.

## G01. GameRoom_PlayerSimulation.cpp와 ServerPlayer.h

`SERVER_PLAYER`의 낙사면은 낙하 시작의 기준 높이에서5m 아래를 저장하며 snapshot에는 추가하지 않는다.
`Begin_PlayerFall`은 비행의 최초 supportY 또는 일반 지지 이탈 높이에서 이 값을 잡는다.
`Update_PlayerFall`은 쿠크에서 낙사면 통과를, 기존 다른 world에서는 기존 deadline을 소비한다.
비행 중에도 아래 지형에 닿기 전에 낙사면을 검사한다. 명시적으로 이탈 가능한 쿠크 수평 넉백은
기존 발탄 `Trace_ForcedSurface`를 공유하고 실제 충돌이 확정한 이동 구간의 지지를 검사한다.
Gate1·Gate3와 Mario는 이 확장에서 제외한다.

## G02. 현재 관문과 Revive commit

현재 관문은 승인된 raid, audition의 Scope/실제 Pattern, 기존 GateProgress 순서로 resolve한다.
`GameRoom_GateProgress.cpp`의 기존 관문 전투 시작좌표를 재사용하며 새 좌표표를 만들지 않는다.
`Handle_RevivePlayer`는 후보 위치의 navigation 검증이 성공해야 위치·HP·action을 함께 commit한다.
발탄 safe-center 부활과 실패 시 사망 상태 보존을 유지하고 쿠크 비행/낙하 잔여 상태를 정리한다.

## G03. 검증과 완료 경계

기존 `ServerGameplayContractTests_KoukuOverlap.cpp`에서5m 전후 사망, 기존 timer와 구분,
공중에서 lower deck으로 떨어지는 경우, Gate1 펜스, 현재관문 부활과 잘못된 목적지 rollback을 검사한다.
새 C++ 파일이 없으므로 project/filter 등록은 없다. Root의 통합 Server 컴파일과 focused headless
실행 결과만 RESULT에 기록한다. Client/Server 프로세스 종료·UI 실행·live 데이터 쓰기는 하지 않는다.


## G04. Albion attack template의 공통 수직 반응

기존 Shared `ATTACK_HIT_TEMPLATE`에 optional `riseHeightM`/`pushMs`를 추가한다. 둘 다0이면
기존 피해만 적용하며, 상승을 사용하면 높이0초과100m이하와 비행100~5000ms를 함께 요구한다.
`PATTERNATTACKHIT`는 기존25열과 끝에 두 값을 붙인27열을 받아 이전 게시본을 유지한다.
`CCombatObjectRuntime`의 TIMED와 CONTACT가 기존 `Apply_WorldToPlayer`에 수평거리0,
상승 높이·시간·강제 ballistic을 전달한다. 피해량과 템플릿의 Once/repeat 정책은 별도 변경하지 않는다.
Client codec/UI/projector/emitter 및 실제 저작 후보는 root 담당이며 runtime만 단독 완료로 처리하지 않는다.

사용자의 후속 변경으로1·3관문 모두 낙사를 금지한다. 강제 이동의 지지 높이를 검사하고
낮은 배경층으로 내려가는 step을 막는다. 이 두 관문에서는 void 자동 낙하도 시작하지 않는다.
기존 FALLING 상태가 들어오면 HP를 보존하여 검증된 현재 관문 시작점으로 복귀시킨다.
