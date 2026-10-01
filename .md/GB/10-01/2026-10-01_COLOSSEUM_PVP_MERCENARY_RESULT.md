# 콜로세움 PvP · 용병 결과

## G00. 경기와 레이드 격리

인간 네 명을 한 transaction으로 입장시켜 무작위 두 팀과 자동 파티를 만든다.
각 팀은 입장점 앞의 차원술사·창술사·워로드·가디언나이트·도화가 후보 중 두 명을 모집한다.
인간2명+용병2명씩 참가하며 미선택 후보는 공격·피격 참가자가 아니다.
각 경기는 별도 CGameRoom을 소유한다. 직접 저작 미리보기에는 PvP 권한이 없다.

PvP 적중은 COLOSSEUM world, ACTIVE phase, 같은 유효 match ID, 양쪽 participant,
서로 다른 team, 생존·전투 준비를 모두 검사한다. 조건이 없는 보스·일반 월드는 기존 경로다.
원본 Balance/Gameplay 데이터의 피해·HP·보스 기믹은 수정하지 않았다.

최대 HP는 사용자 최종 요청에 따라 활성 발탄160줄 HP의40/160을 올림한 값이다.
현재 설치 정본741285439 기준185321360 HP다. 원래 full HP를 iColosseumDamageReferenceHp에
따로 보관해 HP를 줄여도 스킬 한 번의 피해량을 줄이지 않는다. 1줄 피해는4633034를 유지한다.
두 기준은 경기 중 numeric reload에서도 고정하고 새 경기부터 최신 기준을 읽는다.
도화가 T의1/5는 PvP 분기에만 적용한다. 보스전 도화가 T는 유지한다.

## G01. 피해와 넉백 판정

기존 캐스터 시간창, projectile contact, projectile timed, fallback 적중 경로를 사용한다.
스킬 원본의 원·고리·박스·부채꼴 범위와 높이를 적팀 player body에 대조한다.
player body는 XZ 반경0.45m, 높이1.8m이며 스킬에 명시된 hit height를 우선한다.
원래 contact ledger와 최대 피격 수, 다단 히트 분배를 유지한다.

같은 유효 적중에서 damage와 CC를 처리한다. 별도 중복 피해 collider를 만들지 않는다.
V는5.1m/2161ms, ALT_V는16m/1500ms의 PvP 전용 탄도 밀치기이며 아레나 밖 이탈을 허용하지 않는다.
다른 스킬은 기존 authored push/pull을 사용한다. 워로드의 원본4초 stun도 실제 LANDED일 때만
기존 ActiveBuffs에 적용한다. 무적·시간정지·shield·사망은 기존 공용 피격 판정을 따른다.
가디언 Z의 범위 정의가 없는 부분에 전장 전체 debuff를 임의로 추가하지 않았다.

## G02. 준비와 귀환

새 경기의 파일·navigation 로드는 별도 worker에서 수행한다. room thread는 future를0ms로
조회하고 준비 완료 후 source session/binding/queue/party/revision을 다시 검사한다.
초기 reliable 송신을 전부 예약하기 전에는 원래 방·파티·큐를 변경하지 않는다.
실패 시 원본을 유지한다. 완료된 준비의 입장 commit 비용과 경기 simulation 비용은 남으며
실제 동시 레이드 지연0으로 주장하지 않는다.

FINISHED의 사망 인간도 기존 atomic Bern 이동으로 돌아간다. ACTIVE의 사망자는 조기 귀환을
할 수 없다. Bern은 일반 class HP를 복구하고 match·용병·자동 팀 파티 상태를 남기지 않는다.
마지막 인간의 binding과 cleanup이 없어지면 private room과 AI를 폐기한다.

## G03. 실제 검증과 최신 변경 경계

160줄 구현 시점의 Debug Product 전체 build는 PASS했다.
out/BuildPipeline/runs/20260930T184933055Z-debug-product.json, elapsed64983ms다.
실제 Bern8인 fixture에서 두 private 경기, reliable capacity 실패 원인과 rollback,
열 후보 navigation admission, 모집·전멸·사망 귀환·다섯 hit shape 회피를 검사하여
--colosseum-match-contract-test failures0/exit0을 확인했다.
증거는 out/ColosseumPvP20261001/match-debug-recovered.log와 대응 JSON이다.
PvP 네 적중 경로와 raid/null parity는 combat-debug-final.log에서 exit0이었다.

이후40줄 HP/피해 기준 분리·numeric reload·고정 용병 콤보·개인 Guide 변경이 추가됐다.
그 최종 소스의 Debug/Release와 계약 실행 결과는 검증 후 아래에 기록한다.
현재 위의 이전 PASS를 최신 소스 전체 PASS로 대신하지 않는다.
실제4인 Client/LAN·HUD·스킬 표현·넉백 체감은 사용자 화면 확인 범위다.


## G04. 40줄 HP와 최종 직업별 용병 선택 계약

입장 시 인간 네 명과 열 후보의 maxHP는 active BOSS_VALTAN HP / 4의 올림,
iColosseumDamageReferenceHp는 같은 profile의 나누기 전 full HP다. numeric reload는
matched COLOSSEUM의 현재·최대 HP와 damage reference를 유지한다. 일반 레이드의
class profile 비율 migration은 그대로이며 Bern 귀환의 fresh STAGED_PLAYER_ENTRY는
일반 class HP와 match/reference=0을 받는다. 새 경기는 새 active generation을 사용한다.

사용자의 최종 교정에 따라 직업별 선택 계약을 다음과 같이 분리한다.

| 직업 | 최종 스킬 선택 계약 |
|---|---|
| 차원술사 | Guide 정본의 W → A → S → D → F → V → T → ALT_V. LMB 금지 |
| 창술사 | 현재 active catalog와 현재 stance의 전체 스킬 및 LMB |
| 워로드 | 현재 active catalog와 현재 stance의 전체 스킬 및 LMB |
| 가디언나이트 | 현재 active catalog와 현재 stance의 전체 스킬 및 LMB |
| 도화가 | 현재 active catalog의 전체 스킬 및 LMB |

차원술사는 후보 staging에서 고정 순서를 resolve하며 다음 스킬 하나가 승인·종료될 때까지
기다린다. 한 단계의 사용 불가 대기는 5초, 전체 rotation은 45초 뒤 첫 단계로 재시도한다.
실제 authored cooldown은 단축하거나 재설정하지 않는다. 이동·CC가 반복되는 전투에서 모든
rotation이 매번 완주한다고 보장하지 않으며 timeout은 순서를 중단하고 다시 시작한다.

나머지 네 직업은 5Hz 판단에서 현재 class/stance의 전체 SKILL 목록을 재사용 vector에 갱신하고
LMB→Q→W→E→R→A→S→D→F→T→V→ALT_V→Z→X 순서로 마지막 승인 스킬의 다음부터 검토한다.
무작위 선택은 없으며 없는/사용 불가 슬롯은 건너뛰고 한 바퀴 뒤 처음으로 돌아온다. 다른 스킬이
모두 막혀 있으면 LMB를 다시 사용한다. 기존 executor의 쿨다운·자원·거리·상태 승인을 우회하지 않는다.
SPACE는 위험 회피에서만, STANDUP은 실제 KNOCKDOWN에서만 제출한다.
창술사·워로드·가디언 Z와 도화가 Z/X도 published skill ID 경로다. native stance 변경 후 목록을
다시 갱신하되 다음 입력 슬롯으로 cursor를 대응시켜 목록 길이 변화에도 순서를 유지한다.
차원술사의 5초/45초 순서 timeout은 적용하지 않는다.

현재 SKILL이 COMBO이면 매 Server fixed tick에 published input window 안에서 같은 skill을
한 번 제출한다. 차원술사 LMB는 금지하고 나머지 LMB 내부 BA chain은 지원한다. 자동 단계와
이미 buffered인 단계에는 추가 입력하지 않으며 buffer의 false 반환으로 다음 스킬을 승인하지 않는다.
native Update가 마지막 stage를 끝내어 action이 NONE이 된 뒤 다른 스킬을 선택한다.
일반 플레이어·보스의 공용 executor는 수정하지 않았다.

회피는 실행 중 적 caster의 향후 400ms hit window를 circle/ring/box/cone geometry로 검사하고
navigation·collision·LOS를 통과한 낮은 위험 위치를 고른다. 나머지 네 직업은 그 방향으로
SPACE를 먼저 시도하고 사용 불가하면 기존 navigation 회피를 사용한다. 진행 중 COMBO는 공용
경계의 무조건 예약을 즉시 회피 승인으로 오해하지 않도록 SPACE를 보류하고 navigation 회피를
유지한다. projectile 미래 궤적 예측은 포함하지 않는다.
실제 4인 Client/LAN·UI·실전 선택 및 넉백 체감은 사용자 확인 범위다.

## G05. 직업별 선택 교정 전 실제 Debug 계약 검증

최종 Debug Product 후 실제 `--colosseum-match-contract-test`는 PASS 100개/failures=0,
`--colosseum-combat-contract-test`는 PASS 89개/failures=0을 확인했다.
`out/GuidePersonal20261001/contracts-debug-first.json`에서 두 명령의 exitCode=0도 확인했다.
로그는 같은 폴더의 `colosseum-match-debug-first.log`, `colosseum-combat-debug-first.log`다.

40줄 HP와 full 피해 기준, 세 phase의 numeric 보존과 일반 Valtan HP migration,
다섯 native COMBO의 최종 stage·정확한 manual buffer 횟수·비용/쿨다운 1회,
260~290ms 입력창과 다음 스킬 지연까지 실제 검사했다. 피해 네 경로의 기존 full-HP 기준,
missing-reference 거절, Artist T 1/5 및 raid/null 경로 유지도 통과했다.
Release Product와 실제 4인 Client/LAN·화면 확인은 이 Debug 결과에 포함하지 않는다.

## G06. 차원술사 한정 고정 순서 교정 소스

G05 뒤 사용자 교정을 반영하여 차원술사만 고정 순서·LMB 금지를 유지하고 나머지 네 직업은
전체 사용 가능 스킬·LMB로 변경했다. 제품 변경은 GameRoom_Colosseum.cpp의 AI 입력과
ServerGameplayContractTests_ColosseumMatch.cpp에 한정하며 인간·보스 executor는 유지한다.

회귀도 차원술사 고정 순서/대기/timeout과 다른 네 직업 전체 목록/LMB로 분리했다.
기존 다섯 manual COMBO 및 좁은 input window 검사에 네 직업 native LMB 전체 단계,
전 스킬이 준비된 각 직업의 실제 LMB→Q→W 순환, 창술사 Z stance commit 뒤 짧아진 목록의
LMB→Q 선택과 KNOCKDOWN→STANDUP, 위험 시 SPACE/사용 불가 navigation fallback을 추가했다.
첫 경기 인간을 가디언으로 구성하여 FINISHED 사망 귀환이 일반 profile HP(현재 132000)와
match/reference=0을 복구하는지도 검사한다. 132000이나 기존 boss HP를 전역 수정하지 않았다.
소스 동결 시점에는 실행 대기였으며, 이후 실제 Debug 결과는 G07에 기록한다.
G05의 match 100개/combat 89개 PASS를 이번 교정 전체 PASS로 대신하지 않는다.

## G07. 직업별 순환 교정 후 실제 검증

`out/GuidePersonal20261001/colosseum-match-debug-delivery.log`의 최신 Debug 검사는
PASS 134개, FAIL 0개, 최종 failures=0이다. 네 직업 native LMB와 LMB→Q→W 순환,
창술사 stance 변경 뒤 새 LMB→Q, 조건부 SPACE/STANDUP, 가디언 일반 HP 귀환과
40줄 HP/reference 및 세 phase numeric 보존을 포함한다.

동일 폴더의 `contracts-release-delivery.json`은 아래 여섯 Release 검사 모두 exitCode=0을
기록한다. 실제 `*-release-delivery.log`의 PASS/FAIL 집계도 각각 대조했다.

| 실제 Release 검사 | PASS | FAIL |
|---|---:|---:|
| colosseum-combat | 89 | 0 |
| kouku-object-overlap | 1102 | 0 |
| maharaka-ai | 199 | 0 |
| npc-raid-return | 54 | 0 |
| skill-stages | 95 | 0 |
| valtan-lifecycle | 152 | 0 |

최신 Server Release 빌드 뒤 `colosseum-match-release-complete.log`도 PASS 134개,
FAIL 0개, 최종 failures=0을 확인했다. `contracts-release-complete.json`의
colosseum-match exitCode=0과도 대조했다. 따라서 직업별 순환 교정 후 매칭 계약 검사는
Debug/Release 모두 134개를 통과했다.

이동 소스 교정 이후 최종 Client 포함 Product Debug/Release는 아직 검증 대기다.
Server 빌드와 headless 로그를 최종 Product 성공으로 대신하지 않는다.
실제 4인 LAN과 Client 화면 확인도 미검증으로 유지한다.

## G08. 최종 PvP 제품 빌드

이동 보완과 모든 최신 Client/Server 소스를 포함한 정상 Product Debug/Release가 모두 성공했다.
앞선 EXE 잠금·최종 링크/제품 빌드 대기는 해소됐으며 실행 파일과 실제 로그는
`../09-27/2026-09-27_GUIDE_AI_TOOL_IMPLEMENTATION_RESULT.md`의 G09에 기록했다.
이 결과는 기존 기능별 검증을 대체하거나 실제 Client 화면·다인 플레이·성능 확인으로 확대하지 않는다.


## G09. 넉백10%·용병 ALT_V30초·참가자 HP 절반

2026-10-01 사용자 추가 요청의 현재 소스 반영 결과다. G08 이전 제품 빌드 완료를 아래 추가
변경의 제품 배포 완료로 해석하지 않는다.

- PvP 피해 adapter가 V/ALT_V override를 정한 뒤 기존 signed push 거리와 이동 시간을10%로
  줄인다. V0.51m/217ms, ALT_V1.6m/150ms이며 일반 push/pull도 같은 비율이다. 거리 또는 시간이
  없는 스킬에는 새 넉백을 만들지 않는다. 별도 stun/down 시간과 공용 landing recovery는 유지한다.
- 실제 collider hit·같은 ACTIVE match·적 participant 확인 뒤에만 적용한다. ally/miss/무적/
  시간정지/경계 처리는 유지하며 공용 PvE 넉백 경로는 바꾸지 않았다.
- 용병별 마지막 ALT_V 승인 tick을 match runtime이 보관한다. 두 AI 선택 경로에서900tick을
  검사하고 실제 Execute 성공 때만 갱신한다. 사망·부활·순환 초기화로 우회하지 않고 기존 더 긴
  cooldown도 검사한다. 인간 입력에는 추가 제한을 넣지 않았다.
- 입장 HP는 활성 발탄160줄 profile의 ceil(/8), 현재92,660,680(20줄)이다. 기존40줄의 절반이며
  스킬 피해용 full160줄 reference는741,285,439로 유지한다. 원본 Balance와 일반132000 profile은
  유지하고 이름표만20줄로 맞췄다. AGENTS/CLAUDE/팀 사용서의 public 수치도 일치시켰다.

### 실제 검증

실행 중 제품을 유지하면서 MSVC14.44.35207 / SDK10.0.26100.0의 기존 Release 옵션을 사용해
Server101 translation unit 전체를 out에 컴파일·링크했다. 변경 GameRoom.h 의존61개를 모두
재컴파일하여 서로 다른 struct layout의 object를 섞지 않았다. 그 검증용 EXE로 실행한 결과:

| 명령 | PASS | FAIL | exit |
|---|---:|---:|---:|
| --colosseum-combat-contract-test | 95 | 0 | 0 |
| --colosseum-match-contract-test | 318 | 0 | 0 |
| --colosseum-contract-test | 25 | 0 | 0 |

20줄/피해 reference 보존과 일반 HP 귀환, 다섯 직업 ALT_V의899/900tick 경계·실패 시각 미갱신·
실제 부활·개별 용병·긴 cooldown·phase·tick wrap을 검사했다. 기존 실행 데이터174개와 제품
EXE/OBJ/tlog/lib는 전후 같았다. 증거는 `out/ColosseumBalance20261001/run-091221-f5e10a6a/`
의 `build-receipt.json`, `contract-receipt.json`, 세 contract log다. 검증 뒤 GameRoom.cpp의
설명 주석40-bar만20-bar로 교정했고 실행 코드는 바꾸지 않았다.

현재 Product EXE 교체와 실제 Client 화면/실전 체감은 미검증이다. 실행 중 Release Client/Server는
종료하지 않았고 out의 검증 EXE를 제품 폴더로 복사하지 않았다.

사용자가 최종 확인에서 "지금은 계속 켜둘게요"를 선택하여 현재 Release Client/Server를 유지한다.
정상 Product 빌드·실행 파일 교체는 사용자 선택에 따라 보류했다. 현재 실행 중 세션에는 새 C++
동작이 적용되지 않았으며, 소스·격리 검증 결과와 다음 재빌드 경계를 구분한다.
