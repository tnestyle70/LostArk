# 발탄 집결 입장과 첫 폭탄 갑옷 파괴 결과

## G00. 반영 범위

기존 첫 폭탄은 DESTROY_FIRST_ELIGIBLE 계약으로 두 갑옷 중 하나만 제거했다. primary 발탄의
서버 생성 파괴 폭탄 provenance가 검증되고 실제 파괴 창이 열려 있을 때, 이제 같은 명중에서
남은 두 갑옷을 함께 제거한다. 일반 공격·스킬·가짜 skill ID의 갑옷 피해 차단은 보존했다.
BossCombatRuntime은 combined part mask의 PART_BROKEN edge 한 개를 만들고 legacy plate
상태와 snapshot도 같이 갱신한다. 기존 Client는 mask1/2의 원작 파편을 각각 한 번 재생한다.
이미 제거한 갑옷에 추가 폭탄을 던져도 파편과 성공 문구를 다시 발생시키지 않는다.
사용자가 첫 폭탄의 즉시 갑옷 파괴를 실제 화면에서 확인했다. 이 동작은 후속 입장 변경에서
재수정하지 않았다.

## G01. 집결과 입장

World Gameplay에 Stage_Boss_Assembly를 (123.38,23.05,-89.20), yaw48로 등록했다.
halfExtents는 쿠크 entry_aura와 같은 XZ footprint (3.610384827,3.633995132), Y1.5다.
기존 Stage_Boss와 Stage_Boss_ArenaEntry는 비활성화하여 (129.65,23.02,-96.85)의 이전
move_destination 표식을 없앴다. 새 collider는 GameRoom의 집결 상태가 소유하므로 일반
trigger의 진입/G 처리에서는 제외한다.

현재 room의 인간 참가자 최대4명이 모두 collider에 들어와10초(30Hz300tick)를 유지한다.
GUIDE_AI는 집결·투표 인원에 포함하지 않는다. Client는 replicated 위치와 Server tick으로
카운트다운을 표시한다. 최신 후속 요청 후보는 확인창·전원 수락을 제거하고 Server의300tick
완료에서 기존 encounter/cinematic을 바로 활성화한다. 후속 구현·native 검증은 G06에,
최신 전체 병합의 빌드·publish 및 재실행은 통합 Preview/Object RESULT에 기록한다.
이탈·사망·멤버 변경은 readiness를 초기화한다. 이동 직전 네 목적지의
navigation·height·collision을 모두 검사하며 실패하면 어떤 플레이어도 이동시키지 않는다.

도착 정본은 disabled valtan.entry.slot.1~4이며 각각 다음과 같다.

- (148,23,-117)
- (150,22.99,-115.73)
- (149.27,23,-119)
- (151.27,22.99,-117.73)

두 번째 쌍은 첫 쌍의 간격(2,1.27)에 직교하는 인접 행 offset(1.27,-2)을 적용했다.
반대 offset(-1.27,2)은 slot3가 frontwallA/receiver 안에 들어가 native destination 검증에서
REJECTED_COLLISION이 확인되어 사용하지 않는다. 사용자가 지정한 앞의 두 좌표는 유지했다.
원래 월드 최초 입장 spawn은 유지하여 집결 공간으로 이동할 수 있다. Client Loader의
ClickMoveEffect resource queue에 발탄 aura를 함께 연결했으며 새 C++ 파일이나 wire schema는 없다.

## G02. 기존 확인창 방식까지 실행한 검증

- 팀 LAN sync 성공: server-host, TCP7777 LocalSubnet ready, Team endpoint192.168.0.22,
  not-listening. Client/UI는 실행하지 않았다.
- World publisher Validate -WorldId VALTAN_ARENA 성공:158placements,3spawn groups,
 1prop set/4slots. 통합 담당이 같은 World source를 Publish하여 Client viewer와 Server bootstrap을
 설치했다. 실제 native consumer는 아래 Debug 계약 테스트로 확인했다.
- 변경 C++는 UTF-8 기존 인코딩과 CRLF를 유지했다. 신규 data JSON parse와 해당 파일의
 git diff --check 통과.
- 기존 Server --battle-items-contract-test에 첫 폭탄 combined mask·후속 중복 억제 기대값과
 실제4명 admission을 통한10초 이전 거절/전원 투표/이탈 취소/부분 승인 위치 보존/최종4도착과
 다음 Server brain tick의 VALTAN_ENTRANCE_CINEMATIC 시작 사례를 추가했다. 통합 담당의
 Debug Product build 후 Server/Bin/Debug/Server.exe --battle-items-contract-test를 실행하여
 failures:0, exit0을 확인했다. 근거는 out/ValtanAuthoring20260930/battle-items-gameplay-corrected.log다.
 최초 실행은 추정 slot3의 frontwallA 충돌로 실패했고, 인접 행을 보행 가능한 아레나 쪽으로
 반전한 뒤 World Publish와 같은 테스트를 다시 통과했다. 네 목적지는 typed ACCEPTED이며
 sampled Y는23.0124/22.9927/23.0202/23.0789다. 통합 담당이 Release에서도 같은
 --battle-items-contract-test를 실행하여 failures:0을 확인했다. 근거는
 out/ValtanAuthoring20260930/battle-items-release-final.log다.

## G03. Object 입력 소비자 검토

Valtan Object session이 MainApp의 공유 Resources/transport route와 AnimationTool 입력 소유권에도
연결되도록 수정했다. 이전 Kouku WorldObject interaction은 Valtan Object의 입력을 가져가지 않는다.
Sequencer의 Object boss 선택·typed Open은 선택된 boss를 먼저 저장한 뒤 owner callback에 전달하고,
Object pane의 ImGui scope도 boss별로 구분한다. 통합 Debug Product 컴파일은 통과했고
실제 Object 창 재생은 사용자 화면 확인이 필요하다.

## G04. 바위 타이밍과 연속 lifecycle 검증

기존 ServerGameplayContractTests_ValtanTimelines.cpp에 실제 CCombatObjectRuntime을 사용하는
검증을 추가했다. 3종 연쇄 바위는 cone 직접 피격/연쇄 판정의 준비 시점 0/1500ms를 구분하고,
각 준비에서1820ms 뒤 첫 허용30Hz tick에만 HP 피해와 reliable HIT_PULSE가 한 번 발생한다.
발악 바위는 준비4133ms, 피해5953ms를 검증한다. 매tick HP와 event ID/시각/횟수를 검사하며
같은 owner hit를 반복해도 추가 피해·event가 없는지 확인한다. HIT_PULSE가 사운드를 구동하는
서버 이벤트임을 검증한 것이며 실제 오디오 출력·GPU 프레임 정렬의 화면 판정은 포함하지 않는다.

통합 담당이 Debug와 Release의 --valtan-presentation-contract-test를 실행했고 각각
failures:0이다. 근거는 out/ValtanAuthoring20260930/valtan-presentation-final.log와
out/ValtanAuthoring20260930/valtan-presentation-release.log다.

--valtan-lifecycle-contract-test의 연속4인 fixture는 폐기된 Stage_Boss G 상호작용과
단일 옛 도착 marker를 전제로 하여 입장 단계에서 실패했다. 테스트 setup을 실제302 room tick의
집결과 leader ADVANCE·나머지3인 수락, valtan.entry.slot.1~4 검증으로 수정했다. 기존 보스
순환·체력 구간·무력화·카운터·동일 snapshot·클리어·보상 검증은 유지했다. 이 수정은 테스트에만
적용했고 제품 소스는 바꾸지 않았다. Debug/Release 기능 제품 빌드는 이미 완료했으며,
사용자가 Debug를 실행 중인 최종 시점의 전체 Product 재빌드는 output guard가 차단했다.
통합 담당은 Release Server가 미점유임을 확인하고 기존 Server 프로젝트를
BuildProjectReferences=false, LostArkPublishRuntimeData=false로 증분 빌드했다.
변경한 lifecycle 테스트 CPP 한 개의 컴파일·링크 PASS 근거는
out/ValtanAuthoring20260930/build-release-lifecycle.log다.

수정된 Release --valtan-lifecycle-contract-test는 failures:0, exit0으로 통과했다.
연속4인 fixture는19262개의 동일 snapshot tick,7개 체력 기믹, 발악→부활 직결,
망령화40줄 복원, 실제 플레이어 스킬 최종 처치,4인 각각의 MVP·보상을 확인했다.
근거는 out/ValtanAuthoring20260930/valtan-lifecycle-release-final.log의
[VALTAN_4P] complete=1 ticks=19262 window=7 mechanics=7 parity=1 ready=1이다.
최종 fixture의 Debug 재빌드·재실행 성공으로 확대해 기록하지 않는다.

## G05. 남은 화면 확인

자동 입장 변경은 Release native 검증을 통과했다. 최종 Debug 실행파일 적용 뒤 사용자 화면에서
발탄은 새 위치에 모여10초 → 네 위치 이동·기존
컷씬, 쿠크는 기존 leader aura10초 → 기존 입장을 확인한다. 두 아레나 모두 추가 확인창이
나오지 않아야 한다. 첫 폭탄의 즉시 갑옷 파괴는 사용자 확인이 완료됐다. 카운트다운·파편의 GPU와
실제 다인 네트워크 화면은 에이전트가 실행하거나 성공으로 판정하지 않았다.

## G06. 후속 요청 완료: 확인창 없는 자동 입장

Valtan과 Kouku3관문 모두 Server가300tick 집결 시간을 소유하고 기한 뒤 기존 완료 함수를
호출하도록 구현했다. Valtan은 기존 인간 전원 collider 조건, Kouku는 기존 leader만
entry aura에 들어가는 조건과 leader의 party/raid roster를 유지한다. roster·leader·raid epoch
변경, 참가자 이탈·사망·낙하·trigger 이동은 countdown을 초기화한다. GUIDE_AI와 관계없는
Kouku 방의 observer는 입장 인원에 포함하지 않는다.

Complete_GateProgressTransition은 기존 일반 투표의 완료 분기를 추출한 공유 함수다.
자동 입장은 열린 vote나 가짜 수락을 만들지 않고 동일 Valtan4목적지 검증/기존 Kouku 입장
commit을 재사용한다. 초기 Valtan ADVANCE와 Kouku ENTER_GATE3 수동 명령도300tick을
우회하지 못한다. 실패는 기존 위치·보스 상태를 보존하고 occupied attempt latch로한번만
통지하며, 나갔다 재집결하거나 roster/epoch가 바뀔 때만 다시 시도한다. 기존 closed
S2C_GATE_PROGRESS_STATE에 proposalId0, kindADVANCE/ENTER_GATE3,
ALL_ACCEPTED 또는 CANCELLED를 전달한다. Bern 파티 이동과 일반 관문
ADVANCE/RESTART/EXIT 투표는 보존한다.

기존 BattleItems·ValtanLifecycle·KoukuRaid의 입장 fixture를 자동 집결로 교정했다.
299tick 무진입,300tick 무명령 완료, 이탈·죽음·roster reset, invalid navigation/actor/participant의
commit 전 보존, 실패 반복 억제와4목적지·기존 cinematic을 검증하도록 작성했다.
기존 Kouku Gate3 fixture에는 Debug 전용 호출이 없어 그 함수의 과거 Debug guard만 제거하여
Release에서도 같은1~4인 실패보존·자동입장을 검사한다. 실제 Shared codec에서 proposalId0인
closed CANCELLED를 왕복하는 검증도 기존 state 사례에 포함했다. 추가 테스트의 잘못된
PLAYER_ACTION_STATE::IDLE 두 곳은 실제 enum NONE으로 수정한 뒤 아래 native 빌드를 통과했다.

통합 담당이 최종 Release Server로 다음 기존 테스트를 실행했고 세 실행 모두 failures:0,
exit0이다. 다음 로그는 자동 입장 후속 변경을 포함한 실제 실행 근거다.

- --battle-items-contract-test: out/ValtanAuthoring20260930/battle-items-autoentry-release.log.
  사망·낙하·이탈 reset,299tick 무입장,300tick 자동4인 이동과 기존 입장 cinematic을 확인했다.
- --valtan-lifecycle-contract-test: out/ValtanAuthoring20260930/valtan-autoentry-lifecycle-release.log.
  확인창 없는 실제 room 집결로 시작해 연속4인19262tick,7기믹, 발악→부활 직결,
  동일 snapshot·최종 처치·4인 보상을 확인했다.
- --kouku-raid-contract-test: out/ValtanAuthoring20260930/kouku-autoentry-release.log.
  일반1~4인 및 실행 중인 raid의 leader aura300tick 자동 입장, roster 변경 reset,
  실패 전 상태 보존·반복 실패 억제·무관한 observer 위치 보존을 확인했다.

Server Release 컴파일·링크는 out/ValtanAuthoring20260930/BindAirborne/build-after.log,
Debug ClCompile은 out/ValtanAuthoring20260930/compile-debug-autoentry-bind-server.log로
PASS를 확인했다. Client Release build와 Debug ClCompile도 통합 담당이 통과했다.
근거는 out/ValtanAuthoring20260930/build-release-autoentry-client.log와
out/ValtanAuthoring20260930/compile-debug-autoentry-client.log다. 변경 파일의
 git diff --check도 통과했다.

당시 사용자가 실행 중인 Debug Client/Server 두 EXE는 자동 종료하지 않았다. 이후 사용자가
전체 병합을 승인하고 EXE 점유가 해제된 것을 확인하여 최종 Debug/Release Product를 완료했다.
receipt는 out/BuildPipeline/runs/20260930T091228557Z-debug-product.json과
20260930T091359049Z-release-product.json이며 둘 다 PASS다. 전체 Server/Client owner publish도
완료했다. 사용자 화면 판정과 무창 native 검증은 계속 구분한다.

## G07. 진입 컷신 플레이어 숨김

입장 컷신에 HUD·이름표·원본 보스만 숨기고 실제 player presentation은 억제하지 않던
누락을 수정했다. Level_ValtanArena::Sync_CinematicPlayerVisibility가 source entrance와
그 입장 카메라의 종료 전환 동안 최신 local/remote/Guide character에 기존
Set_CinematicPresentationSuppressed를 적용한다. 본체·장비·탈것·그림자는 현재 owner
상태를 draw에서 소비하며, snapshot으로 추가된 player도 수집 직후와 MainApp의 후반
Sync_CinematicUI에서 반영된다. 개별 장비 visibility와 Server 복제 상태를 덮어쓰지 않는다.

source/camera가 모두 종료되면 표시가 복귀한다. 실패·취소·연결 해제·레벨 이탈은 기존
End_CinematicCamera를 사용하고, camera-null 분기도 같은 전체 종료로 변경해 source
상태가 남지 않도록 했다. 이름표·칭호·말풍선은 기존 HUD 가드를 소비한다. finale와
일반 전투 카메라·쿠크 대형 등장 정책은 보존했다.

변경은 기존 Level_ValtanArena.cpp/.h와 MainApp.cpp의 최소 추가다. 세 파일의 작업 전
미커밋 내용은 out/ValtanEntryPlayerVisibility20260930/*.before와 apply-receipt.json에
보존했으며 교체 직전 hash를 확인했다. UTF-8 BOM없음/CRLF와 기존 project 등록 유지,
이번 delta의 diff-check 및 정상/실패 종료 경계 검토를 완료했다.

제품 compiler 기록의 실제 옵션으로 두 TU를 out에 Debug/Release 컴파일하여 exit0을
확인했다. camera-null 마지막 수정도 두 설정에서 재컴파일했다. 근거는
out/RaidSafeZoneAndEntrance20260930/compile-client-Debug.log,
compile-client-Release.log, compile-final-arena.log와 client-compile-inputs.json이다.
기존 코드페이지 C4819 경고는 남아 있으므로 경고0이라고 기록하지 않는다.
표준 EXE의 최종 링크는 사용자 실행 파일 점유 해제 후 진행하며 아래 후속 기록을 따른다.
Client/UI 자율 실행은 하지 않았고 실제 컷신 숨김·복귀의 화면 PASS는 사용자 확인 범위다.
G07 최종 경계: 사용자가 테스트를 계속하며 소스만 준비하도록 명시했다. 표준 Debug/Release
제품 빌드·EXE 교체·데이터 publish·프로세스 종료를 수행하지 않았다. 현재 실행 중인
게임과 Server에는 이 수정이 아직 반영되지 않았다. Client2TU와 Server2TU의 Debug/Release
격리 최소 컴파일은 모두 exit0이며, Server의 실제 게시 파1빨2 회귀는101 PASS/0 FAIL이다.
Server 세부 증거는 out/KoukuBlueRedZone20260930/validation-receipt.json을 따른다.

### G07 후속: 2026-09-30 통합 Debug/Release 제품 빌드

사용자의 후속 전체 빌드 요청에 따라 현재 다른 세션 변경을 함께 포함하여 표준 Debug/Release
Engine·Shared·Server·Client 빌드와 배포를 완료했다. 앞선 소스만 준비 상태를 해소했고
진입 컷신 플레이어 숨김은 두 설정의 Client EXE에 포함됐다. 새 Release Server의
발탄 presentation 43 PASS, lifecycle 152 PASS 및 battle-items 183 PASS, 쿠크 raid
2066 PASS를 확인했으며 각 failures:0/exit0이다. Client 화면은 실행하지 않았다.

통합 빌드 receipt·파일 해시·데이터 점검·남아 있는 경고는
out/CombinedProductBuild20260930/summary.json을 따른다. 이번 빌드에서 별도 데이터
publish는 수행하지 않았으며 컷신 숨김·종료 후 복귀의 실제 화면 확인은 사용자 범위다.
