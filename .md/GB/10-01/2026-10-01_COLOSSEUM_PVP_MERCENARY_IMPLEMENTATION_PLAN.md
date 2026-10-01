# 콜로세움 PvP 매칭·용병 구현 계획

작성일: 2026-10-01. 기준은 현재 소스와 `COLOSSEUM.worldbootstrap`이며 기존 콜로세움 월드맵 PLAN/RESULT를 이어간다.

## 목표와 현재 확인

Bern 큐의 인간 네 명을 무작위 2 대 2로 나누고, 각 팀이 다섯 직업 후보 중 두 용병을 선택하여 4 대 4 경기를 한다. Debug와 Release의 입장 인원은 모두 네 명이다. 팀별 인간 둘은 자동 파티가 되며 선택된 용병 둘을 같은 roster에 넣는다. 양 팀 모집 완료 후 ACTIVE, 한 팀 참가자 전멸 후 FINISHED다. 기존 레이드와 직접 콜로세움 저작 입장은 영향을 받지 않는다.

작업 착수 당시 `GameRoom_Inventory.cpp`는 Debug 한 명/Release 네 명이며 큐를 먼저 지운 뒤 MATCH_FOUND와 개별 이동을 송신한다. `ServerApp.cpp`는 콜로세움을 shared room으로 받으며 `SERVER_PLAYER`에는 팀/경기 식별자가 없다. 따라서 당시 loading 화면의 팀 표시는 전투 권한이나 경기 격리를 보장하지 않는다. 기존 `Transfer_PartyTo`의 Stage_PlayerEntry/RELIABLE_BATCH_TRANSACTION을 같은 기준으로 재사용한다.

## 변경 파일과 데이터 흐름

`ServerPlayer.h`에 match ID/team/participant와 immutable `iColosseumDamageReferenceHp`를 추가한다. `RoomCommand.h`, `GameRoom.cpp`, `ServerApp.cpp`는 typed recruit를 room thread로 전달한다. `GameRoom_Inventory.cpp`는 큐 네 명을 하나의 Colosseum batch로 예약하고 성공할 때만 큐를 제거한다. `ServerTriggerSystem.h`는 이 서버 내부 batch 표식을 전달한다.

새 `GameRoom_Colosseum.cpp`가 전원 입장 staging, 팀 파티, 열 명 후보, 모집, 경기 상태와 AI를 소유한다. `ServerApp.h/.cpp`는 match별 private room 수명과 tick/revision/numeric update/shutdown을 관리한다. `GameRoom_PlayerSimulation.cpp`는 해당 경기의 player pointer span과 ACTIVE 권한을 PlayerSkillSystem에 전달한다. 새 cpp는 `Server.vcxproj` 및 `.filters`에 기존 필터를 보존하여 등록한다.

Shared wire와 Client는 별도 담당자가 `C2S_COLOSSEUM_RECRUIT`(120), `S2C_COLOSSEUM_MATCH_STATE`(121), `COLOSSEUM_MERCENARY_AI`를 연결한다. Shared 담당자가 root와 조율하여 protocol 130을 한 번 변경한다. 스킬 피해·CC는 별도 담당 `ColosseumCombatPolicy.h`/PlayerSkillSystem이 동일한 match/team guard를 소비한다.

## G00 — 네 명 입장 transaction과 경기 격리

큐에는 살아 있고 이동 가능한 인간만 유지한다. 한 immutable 네 명 batch를 예약하며 다른 transfer와 중복하지 않는다. 새 private room에 source 플레이어의 인벤토리·purse·칭호·내구도를 보존하여 전부 stage한다. authored teama/teamb playerSpawn과 navigation/collision을 검증한다. 기존 MATCH_FOUND, ENTER_ACCEPTED, 전체 spawn, 팀 roster, 지속 match state를 reliable queue에 일괄 예약한다. 한 명이라도 실패하면 원래 방/파티/큐/바인딩을 유지하며 재시도할 수 있다. 송신 예약 성공 이후 source departure와 target container swap 및 app binding을 commit한다.

새 경기의 CGameRoom 생성자는 WorldBootstrap, item/vehicle/honor/rewards, navigation, collision과 entity를 읽으므로 RoomThread나 sessions mutex 안에서 실행하지 않는다. `Begin_ColosseumPreparation`은 기존 numeric worker와 같은 단일 `std::async`에 immutable active generation과 취소 플래그만 전달한다. worker는 app/session/player 컨테이너를 캡처하지 않는다. RoomThread는 0ms future poll만 하고 완료 뒤 generation pointer, 인간 네 명 binding, 현재 queue/party/vote를 다시 확인하여 입장을 commit한다. 준비 중 다른 매치가 worker를 덮어쓰지 않으며 source는 Bern에 남는다.

CGameRoom의 선택적 cancellation 인수는 기본 null이므로 기존 레이드 생성 계약은 유지한다. worker 준비는 각 로드 단계 사이에서 취소를 확인하고 실패한 객체는 worker에서 버린다. shutdown은 RoomThread 정지 후 취소를 요청하고 최대 30초를 기다리며, I/O가 응답하지 않으면 기존 numeric worker와 동일한 ERROR_TIMEOUT process fail-fast를 사용한다. `TerminateThread`는 사용하지 않는다. worker 완료 시 실제 constructor 시간을 로그에 남기며 이 값은 raid tick 시간이나 GPU 성능으로 설명하지 않는다.

`m_ColosseumMatches`는 shared preview와 별도로 소유한다. direct Lobby 입장은 기존 shared room으로 유지하며 match ID가 없으므로 PvP 권한과 40줄 HP를 주지 않는다. private room은 binding이 없고 cleanup까지 소비된 뒤 폐기한다. revision/numeric transaction은 private 경기까지 현재 room 집합으로 검증한다.

## G01 — 모집과 전투 상태

각 팀 입장점 앞에 차원술사·창술사·워로드·가디언나이트·도화가를 기존 SERVER_PLAYER/character class spawn으로 만든다. 후보 중심의 고정 가로열은 실제 navgrid 밖으로 나갈 수 있으므로 기존 Guide landing과 같은 49개/반경 3m 이하 ring을 탐색한다. 전방 1m 이상, 입장점에서 7.5m 이하, 높이차 1m 이하, 기존 collision, staged 인간·용병 0.75m 분리와 실제 navigation LOS를 모두 통과한 위치만 확정한다. 원래 배치 후보 중 z=-4.1/-5.6 네 좌표가 현재 navgrid에서 walkable=0인 것을 실측했으며 허용 지형이나 collider를 넓히지 않는다. 가짜 socket이나 session을 만들지 않는다. 미선택 후보는 participant=false이며 공격과 피격 대상이 아니다. recruit는 요청자 match/team, phase, sequence, 후보 소유 팀, 생존, 거리와 선택수 둘을 검증한다. 기존 선택 재요청은 중복 선발하지 않는다. 선택 후 roster와 지속 state를 모든 인간에게 복제한다. 인간만 party leader가 될 수 있게 인간 둘 뒤에 용병을 붙인다.

사용자의 후속 변경에 따라 전투 HP는 현재 active BOSS_VALTAN profile HP의 1/4을 올림하고 전용 표시는 40줄이다. `iColosseumDamageReferenceHp`에는 원래 160줄 full HP를 별도로 보관하여 기존 피해량을 유지한다. 현재 설치 active BOSS_VALTAN 741285439를 소비하면 경기 HP는 185321360이며 피해 기준은 741285439다. source BossProfiles의 600000이나 이 예시 숫자를 코드에 하드코딩하지 않는다. 경기의 인간·선택 용병·미선택 후보는 입장에서 두 값을 확정하고 모집/진행/종료 중 Balance Tool numeric reload에도 현재 HP 비율, maxHP와 full reference를 보존한다. 변경한 class/boss 수치는 다음 신규 경기 입장부터 사용한다. Bern 귀환은 fresh staged player의 일반 class HP와 reference=0을 사용한다. 기존 레이드의 class HP 비율 migration과 boss 데이터는 바꾸지 않는다. 양 팀 두 용병 선택 완료 후 ACTIVE로 바꾸고, 살아 있는 participant가 없는 팀이 생기면 FINISHED로 바꾼다. 모집 중 인간 이탈은 경기 종료로 처리하여 영원히 모집에 머물지 않는다.

## G02 — 직업별 기존 스킬·이동·회피 소비

선택된 용병만 상대 팀 participant를 탐색하며 전술 판단은 0.2초 주기로 수행한다. 사용자의 최종 교정에 따라 차원술사만 Guide Combat 첫 authored combo인 W→A→S→D→F→V→T→ALT_V를 고정 순서로 사용하고 LMB를 금지한다. 이 순서는 후보 staging에서 active class/stance/slot을 resolve하고, 다음 한 스킬의 승인·종료를 기다린다. 사용 불가 단계 대기 5초와 전체 45초 timeout은 차원술사에만 적용한다.

창술사·워로드·가디언나이트·도화가는 현재 class/stance의 실제 binding을 LMB→Q→W→E→R→A→S→D→F→T→V→ALT_V→Z→X 순서로 순환한다. 무작위 선택은 없으며 없는 슬롯과 쿨다운·자원·상태로 거절된 스킬은 건너뛴다. 승인한 스킬이 종료되면 그 다음부터 찾고, 나머지가 전부 사용 불가하면 LMB로 돌아온다. 각 5Hz 판단에서 저장 vector의 용량을 재사용해 현재 목록을 갱신한다. 스탠스 변경으로 목록 길이가 달라질 때는 숫자 인덱스 대신 다음 입력 슬롯의 위치를 보존한다. Z/X도 published skill ID로 같은 Execute_PlayerSkill을 사용하고 native Commit_StanceChange 후 새로운 binding을 소비한다. SPACE는 아래 위험 회피에서만, STANDUP은 KNOCKDOWN에서만 사용하며 일반 공격 순환에서는 제외한다. 별도 stance나 인간·보스 executor는 만들지 않는다.

실행 중 COMBO는 매 Server fixed tick에 현재 published input window 안에서 같은 skill command를 한 번 buffer한다. 차원술사의 LMB는 이 경로에서도 금지하지만 나머지 네 직업 LMB의 BA continuation은 허용한다. 자동 단계와 이미 buffered인 단계에는 입력하지 않으며 executor의 false 반환을 신규 승인이나 다음 스킬 선택으로 해석하지 않는다. action이 NONE이 된 뒤에만 다른 스킬을 선택한다. 실행 중 HOLD는 기존 release로 끝낸다.

이동은 Execute_PlayerMove와 Server navigation/collision을 사용한다. 적 caster의 실제 향후 hit shape를 ServerCombatGeometry로 확인하고 위험도가 낮은 navigation landing으로 회피한다. 차원술사 외 네 직업은 그 방향으로 기존 executor의 SPACE를 먼저 시도하고 거절되면 navigation 회피를 사용한다. COMBO 중 다른 스킬은 공용 경계가 가용성 검사 전에 예약하므로, 진행 중 COMBO에서는 SPACE를 예약하지 않고 기존 navigation 회피를 유지한다. 임의 teleport, 별도 damage 식, 별도 이동 simulation을 만들지 않는다. active roster의 context를 PlayerSkillSystem Update에 전달하여 동일 collider/contact ledger에서 PvP를 판정한다.

## G03 — 실제 PvP 스킬 피해와 CC

별도 전투 담당자는 기존 캐스터 shape/window, projectile contact/timed, fallback 네 hit 경로에 상대 팀 player body(XZ와 높이)를 연결한다. 기존 ledger와 maxTargets를 유지한다. 전체 cast 피해는 ceil(iColosseumDamageReferenceHp × authored bars / 160)이며 도화가 T만 1/5를 적용한 뒤 subhit으로 분배한다. 40줄 maxHP로부터 피해 기준을 곱셈 복원하지 않으며 명시 reference가 없으면 bar 피해를 거절한다. 다른 피해는 기존 buff/spread/crit/defense와 공용 HP commit을 사용한다. 별도 incoming ±10% 산식은 추가하지 않는다.

ALT_V는 16m/1.5s, V는 5.1m/2.161s이며 아레나 외 탈출 허용은 false다. 일반 authored push는 유지한다. 워로드 원본 enemy stun 4초는 LANDED일 때만 기존 ActiveBuffs/knockdown 경로로 적용한다. 가디언 Z에는 현재 hit/범위 정의가 없어 전장 전체 debuff를 임의 생성하지 않는다. null PvP context의 보스 레이드 경로는 그대로다.

## 검증

변경 파일 diff와 인코딩/개행, vcxproj/filter XML parse, git diff --check를 확인한다. 네 명/팀2명, 실패 시 원본 보존, 서로 다른 두 match의 격리, 열 후보 중 두 명만 선발, 반대팀/원거리/중복 모집 거절, ACTIVE 전후 damage guard, 경기 종료/퇴장, 기존 shared preview/레이드 비적용을 계약 테스트로 확인한다. 추가로 실제 async 준비 동안 source 보존, 단일 worker 중복 거절, 미완료 future의 비차단 poll, source binding 변경 후 취소된 completion의 commit 거절을 검증한다. FINISHED의 사망 인간은 기존 square-hole transaction으로 Bern에 돌아와 일반 class HP와 무소속 상태를 받으며 ACTIVE 사망자의 조기 귀환은 차단한다. 회피는 원/고리/박스/부채꼴/360도부채꼴 다섯 경우에 실제 Update_Colosseum과 이동 executor가 안전한 navigation goal을 고르는지 검증한다. 실제 admission의 열 후보 skill 목록이 published class/stance/slot과 대응하는지 검증한다. 차원술사만 고정 순서와 LMB 금지를, 나머지 네 직업은 현재 모든 스킬과 LMB 포함을 확인한다. 실제 Update_Colosseum으로 차원술사 다음 스킬 cooldown 대기, 승인 후 action 종료 대기, CC 후 다음 순서 재개, 단계/전체 timeout을 확인한다. 워로드 E, 창술사 R/A, 도화가 R, 가디언 W의 실제 native Update로 모든 stage 도달·정상 종료·수동 window당 buffer 1회·자동 stage 무입력·추가 비용/쿨다운 없음·다음 슬롯 대기를 검사한다. 별도 좁은 260~290ms window도 실제 fixed tick에서 검사하여 5Hz think에 종속되지 않음을 확인한다. 나머지 네 직업의 native LMB 모든 단계, 전부 준비된 상태의 실제 LMB→Q→W 순환, 창술사 Z→짧아진 short stance 목록→새 LMB→Q binding, KNOCKDOWN→STANDUP 및 위험 방향 SPACE/사용 불가 시 navigation fallback을 검사한다. 첫 경기 인간을 가디언으로 구성하여 사망 종료 뒤 Bern 귀환의 일반 active profile HP(현재 132000) 복구도 검증한다. 실제 numeric parser/stage/commit으로 세 경기 phase의 HP/reference 보존과 Valtan room 일반 class HP 비율 migration을 대조한다. 컴파일과 Product build는 root 단일 runner만 수행한다. GPU와 Client 화면은 에이전트가 조작하지 않으며 최종 UI/행동 확인은 사용자 검증으로 구분한다.

## G09. 콜로세움 넉백·용병 ALT_V·체력 조정

2026-10-01 사용자는 UI 수정과 함께 콜로세움 넉백을 기존10%로 줄이고, 각 용병 ALT_V를
최소30초 간격으로 제한하며 참가자 체력을 절반으로 줄이도록 요청했다.

`ColosseumCombatPolicy.h`의10분의1 비율을 `PlayerSkillSystem.cpp`의 기존 PvP 적중 adapter에서
소비한다. authored push/pull과 V/ALT_V 특례를 결정한 뒤 signed 거리와 이동 시간을10분의1로
만든다. 시간은1ms 이상 올림하며 push가 없는 스킬은 그대로 둔다. V는0.51m/217ms,
ALT_V는1.6m/150ms다. 공용 레이드 reaction, 별도 stun/down·착지 회복·면역·경계 검사는 유지한다.
기존 combat 계약에서 네 hit 경로, 양방향 push/pull·무넉백·아군/범위/무적 차단과 raid parity를 확인한다.

`GameRoom_Colosseum.cpp`의 입장 HP는 ceil(active BOSS_VALTAN maximumHp/8)로 정한다.
현재40줄185321360에서20줄92660680으로 줄고, 기존160줄 스킬 피해 reference는 유지한다.
일반 직업 HP132000과 실제 보스 profile은 바꾸지 않는다. 기존 match fixture의 입장·부활·귀환·
numeric reload 기대값을20줄 기준으로 대조한다.

`GameRoom.h`의 room-owned 용병 상태에 마지막 ALT_V 승인 tick을 optional로 보관한다.
용병의 두 선택 경로는 이전 승인에서900tick(30Hz,30초)이 지나야 다음 ALT_V를 제출한다.
성공한 실제 승인만 기록하고 각 용병이 독립적으로 소유한다. 죽음·부활·rotation 재시작에서
이 기록을 지우지 않는다. 더 긴 기존 스킬 cooldown·자원·상태 검증도 계속 통과해야 한다.
인간 ALT_V 입력과 다른 world의 AI는 수정하지 않는다. 기존 테스트에899/900tick 경계,
실패한 승인·부활·독립 용병·긴 cooldown·tick wrap을 추가한다.

기존 C++ 파일만 수정하므로 프로젝트/filter 항목 추가는 없다. 현재 dirty인1인 상대 자동 용병과
입장·이동 수정은 보존한다. 최소 compile, 기존 combat/match 계약과 정상 Product Build 결과를
분리해 기록한다. 실제 Client 다인 화면·체감은 사용자 확인 범위다.
이름표의 복제 HP 비율 표시도 최대20줄로 맞추며 일반 월드 HP 표시는 유지한다.
