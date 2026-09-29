# 레이드 전투·관전·실패 처리 결과

## G00. 적용 범위와 보존

사용자가 요청한 레이드 수정, publish, Debug/Release 빌드와 검증 ZIP을 같은 작업으로 처리한다.
작업 브랜치는 `codex/release-regression-20260929`다. 시작 시 존재한 다른 변경은 보존했으며
자동 commit/push는 수행하지 않았다. Client 실행·조작 및 실제 화면/음향 판정은 수행하지 않았다.

## G01. 공통 전투 판정

Retail의 `raidStaggerMaximum=40000`이 발탄과 쿠크의 단일 무력화 최대치다. Release F1 Balance
Test에서도 `STAGGER / RAID_COMMON / staggerGaugeMaximum` 한 행으로 조회·저장·적용한다.
현재 방의 진행률은 최대치 변경 전후 동일 비율로 유지한다. 일반 공격은 방어 처리 후 피해의
1/1000을 독립 무력화로 누적하며, 회오리 수류탄은 최대치의 1/3을 올림하여 누적한다.
DAMAGE/STAGGER/COUNTER 분리 collider는 자신의 채널만 적용하므로 HP·무력화 중복이 없다.

마리오의 사용자가 저장한 Duration 시간을 유지하고 `BOSS_DAMAGE_REDUCTION`으로 연결했다.
활성 창 및 retained 창 동안 HP 피해만 1/1000으로 줄이고 무력화에는 재차 나누지 않는다.
실제 HP 타격은 서버의 피해 감소 flag를 통해 숫자 아래 72% 크기의 흰색 `피해 감소`를 표시한다.

명시적 즉사·전멸과 최대 HP 100% 이상 판정은 lethal 경로에서 HP를 0으로 확정한다. 보호막,
일반 무적, 시간 정지, 부적, 사망 방지, 잡힘·낙하·notReady가 실패 대상에서 빠지는 우회를
제거했다. 빙고의 완료 줄 보상과 실제 이난나 장판은 각각의 출처·만료 tick으로 기믹 성공을
판정한다. 일반 무적을 빙고 성공으로 오인하지 않는다.

일반 피격에는 방어·buff 계산 후 보호막 차감 전에 정수 ±10% 난수를 한 번 적용한다.
100→90~110, 1320→1188~1452이며 HP 차감·흡수·피해 event는 같은 표본을 사용한다.
즉사·전멸과 플레이어의 outgoing 피해·무력화는 이 난수 범위에서 제외된다.

인형·공의 저작 광기 증가량은 보호막 흡수와 독립이다. 현재 Retail의 10% 증가·200% 배율은
초당 최대 광기의 20%, 100ms 접촉에서는 2%다. 같은 접촉에 HP 기반 광기를 중복 가산하지 않는다.

## G02. 쿠크와 발탄

쿠크 입장은 저장된 Parent timeout을 따르며 같은 tick의 피해·구속보다 유효한 마리오 접촉을
먼저 확정한다. 입장 실패는 전원 사망시키고 2페이즈를 계속 재생한다. Mario와 CardMaze의
서버 deadline은 90초이며 Mario는 해당 입장자, CardMaze는 파티 전체를 사망시킨다.
실제 저장된 Mario 1/2의 첫 접촉 tick과 P33 이동은 회귀에서 확인했다. 사용자가 경험한 간헐
Release 지연의 유일 원인을 재현한 것은 아니므로, 실제 플레이에서 재발하지 않는다는 확증과는
구분한다.

갈고리는 묶음을 구성하는 실제 15개 grip의 정지 위치를 진행 방향 반대로 1m 당겼다.
실제 WModel/애니메이션 9834ms의 15개 종료 위치가 각각 정확히 1m 이동함을 수치로 확인했다.
내릴 때 기존 navigation 투영도 함께 사용한다. 신규 경계나 순간이동으로 맵 밖을 가리는 방식은
아니며, 이동 자체의 종점이 안쪽으로 바뀐다.

대형 세이튼 등장 10개 공은 터지는 시점에 충돌을 적용한다. 조커 실패와 피자 마지막 내려치기는
2연속 마지막 내려치기의 result.522와 같은 geometry·10% HP·10m/1500ms ballistic을 사용한다.
빙고 표식은 최종 요청대로 1.5배이며 4000ms 폭발에 쇼타임 폭발 음원을 연결했다.

발탄 버러지는 카운터 실패·잡힘으로 완료되지 않고 재시도하며 실제 카운터 성공으로 끝난다.
발악 포탈은 앞쪽에 생성하고 6m/500ms 돌진 후 중앙으로 복귀한다. 발악의 빨간 예고 요소만
숨기고 폭발은 유지했다. 부위 파괴 문구는 실제 갑옷 제거 사건에만 표시하고 준비 PNG를 올렸다.

세부 근거는 같은 날짜의 `KOUKU_MECHANIC_FAILURE_TIMER`,
`VALTAN_COUNTER_LOOP_AND_STRUGGLING`, `RAID_SPECTATE_HUD` 결과에 기록했다.

## G03. 관전과 제품 HUD

본인 사망 후 관전 버튼은 파티 순서의 살아 있는 다른 인간 플레이어를 순환한다. 대상 사망·퇴장
시 카메라 pose를 고정하고 같은 entity가 부활하면 재추적한다. 본인 부활 시 본인으로 복귀한다.
복제된 행동과 게시 cue를 현재 Client에서 재생하여 ALT_V 카메라·전용 효과·잔상·흔들림을
대상에게 연결한다. 상대 PC의 카메라 행렬이나 F6 설정을 네트워크로 복제하는 계약은 아니다.
하단 HP/스킬 HUD는 본인 기준이며 카메라·쿠크 타이머가 대상 기준이다.

Release에서도 90초 제품 타이머를 표시하고 빨간 소수부를 정수부와 같은 34pt로 정렬했다.
HP 왼쪽·흰 보호막 오른쪽, 발탄의 바훈투르/웨이/실리안 초상화, 3관문 전원 이동 문구를 연결했다.
배틀 아이템은 서버 보유 수량을 소비하며 반복 장착을 요구하지 않는다. 서버의 실제 cooldown을
기존 snapshot에 실어 아이템이 한 번만 사용되는 것처럼 보이던 표시를 교정했다.

## G04. 가디언 최종 추가

ALT_V 49420의 hitTime과 DAMAGE/COUNTER/STAGGER 및 presentation HIT를 300ms에서
3800ms로 맞췄다. 실제 브레스 14개 occurrence는 3790ms, 카메라 복귀는 3800ms이며 30Hz의
같은 tick에 해당한다.

광포화 S는 실제로 source49290 clip1의 머리·목 ModelCue를 사용하며, 기존 native 110/111
수정이 연결되어 있어도 cue에 파란 rim/transcolor가 남아 있었다. 이 두 cue를 V의 107/108
프로그램·텍스처·상수로 맞추고 S 메시·14개 bone·애니메이션·배치·수명은 유지했다.
실제 CModel/CMaterial을 D3D11 WARP headless 장치로 생성하여 46개 시점의 V 상수 일치,
variant 독립/복원을 검증했다. GBResources를 assetRoot로 같은 검사를 다시 통과했다.
이 결과는 GPU 화면 색상에 대한 사용자 판정을 대신하지 않는다.

## G05. 검증과 전달 상태

최종 Gameplay publish와 Product 빌드가 완료됐다. 검증 로그는
`out/RaidFix20260930/`에 보존했다. Debug numeric 재실행과 ZIP 무결성 검사까지 완료했다.

- Product Debug `out/BuildPipeline/runs/20260929T202240665Z-debug-product.json` PASS.
- Product Release `out/BuildPipeline/runs/20260929T202300232Z-release-product.json` PASS.
- Gameplay bootstrap format 38, Network protocol 125, numeric source bindings 587 fields.
- Kouku source revision 2496: 요청 Product P8/P13/P26/P33/P88/P91/P92/P93 포함,
  전체 114개 Product/573개 stage/9개 bundle 게시. Raid sequence 183, WorldSequences 2288.
- Valtan Publish V2 → root motion → Gameplay 및 WorldSequences 게시 완료.
- BattleItems Debug/Release 각각 145 PASS: 독립 무력화·Duration 감소·즉사·반복 아이템·난수.
- KoukuProduct Debug 350 PASS, Release failures 0.
- Bingo Debug 152 PASS, ObjectOverlap Debug 1099 PASS, KoukuRaid Release 1750 PASS.
- Valtan Lifecycle Release 93 PASS, CardMaze Release 75 PASS.
- SkillStages Debug/Release 각각 95 PASS: 가디언 3.8초 타격, caster/timed/contact의 감소
  OFF/ON 6개 경로에서 HP·독립 무력화·event 정합.
- Numeric Balance Debug/Release 각각 18 PASS: 단일 STAGGER 행, 40000→41000→40000,
  모든 방 활성화와 75% 진행률 보존, 재로드·실패 rollback·source/bootstrap 원복.
- ShowtimeBomb Release 25 PASS, KoukuBundle Release failures 0.
- Protocol failures 0, 관전 production 함수 probe 20 PASS, ActionGraph Debug/Release 각 11 PASS.
- 변경 JSON 21개 및 project/filter XML 64개 parse PASS. 변경 C++의 project/filter 연결 확인.
  Rendering JSON과 Mario FXAA는 HEAD 대비 의미상 동일하다. raw bytes 차이는 Git 줄바꿈이다.
- `git diff --check` PASS.

Debug Numeric의 최초 45초 fixture는 실제 저장 완료 전 만료됐다. 동일 Release가 전체 통과하고
Debug의 durable source 저장도 확인되어 테스트 예산만 Debug 180초/Release 45초로 분리했다.
제품 로직을 바꾸지 않았으며 매 transaction elapsed를 기록한다. 최종 Debug는 실패 0으로
완료됐고 최초 저장 95.790초, 강제 파일 잠금 실패·복구 40.578초, 공통 무력화 변경/원복
51.245초와 49.449초가 관측됐다. 이는 Debug 수치 저장의 성능 한계이며 Release 동작과 구분한다. 초반 Debug 광역 KoukuRaid는
중단하고 같은 실패를 갖던 Release 전체 suite를 재실행했다. 중단된 두 로그는 성공으로 세지 않는다.

P33 갈고리의 현재 실제 grip 표본은 1667ms 최저점에서 1669ms부터 상승한다. 기존 fixture의
1701ms 기대가 한 tick 늦어 실패하던 부분을 실제 표본으로 교정했다. 설치된 66개 carrier의
floor projection·상승 전 하차·이동/전투 복귀는 ObjectOverlap 전체에서 확인했다.

기존 전체 Effect validator는 이번 범위와 무관한
`effect.valtan.action.420602.stage001.full.restore.effect.json`의 baked trail closure에서 실패한다.
실제 Client 화면·GPU 렌더링·음향 및 사용자 PC 4인 플레이를 자동 검증 성공으로 기록하지 않는다.

## G06. 리소스

빙고 음원은 Client Resources와 Desktop GBResources의 동일 상대 경로에 있다.
`Sound/KoukuSaton/Events/G_Satan1_Attack06_ProjExp1.variant01.wav`, 729164 bytes,
SHA256 `DEE470531167A8EEB3B58E96D1D4502E57586ACF0B0877BD092F0205E7A41D67`.
가디언 재질의 기존 Client 리소스 20개 경로, 27,229,408 bytes를 GBResources에 동기화하고
SHA256 일치를 확인했다. Client 신규 제작/추가 리소스는 0개다. GBResources는 사전 부재가
기록된 14개가 신규 확정이며, 추가 6개는 복사 후 일치와 생성 시각은 확인했지만 사전 존재 여부가
기록되지 않아 신규/기존을 확정하지 않는다. `out/GuardianS20260930/installation-receipt.json`에
파일별 근거가 있다.

## G07. 검증 ZIP 전달

`C:\Users\user\Desktop\LostArk-Verification-20260930.zip` 생성 완료.
크기 166,935,787 bytes(약 159 MiB), SHA256
`fa5a4239699834df725a1433aa463c8092241b43a15fefe949988704efa7d6cd`.

공식 ReleasePackaging으로 Release Client/Server, 런타임 DLL, 256개 compiled shader와 최신
게시 데이터를 묶었다. Resources는 기존 정책대로 외부 GBResources를 사용하며 ZIP에 포함하지
않는다. 전체 ZIP CRC, manifest의 파일별 SHA256/길이, source freshness, source/Product revision
일치 및 launcher `--check` preflight가 PASS다. 검사에서 Client/Server 제품 실행은 하지 않았다.
근거는 `out/RaidFix20260930/package-delivery.log`와
`out/ReleasePackaging/preflight-20260930-raid-guardian-final.json`이다.

## G08. ZIP 전달 후 보호막 표시 후속 수정

사용자 화면에서 흰 보호막 quad가 보이지 않는 0폭→양수 복원 결함을 추가 수정했다.
CUI_Sprite.cpp가 현재 rect로 transform 축을 다시 구성한다. 실제 함수 CPU 검증753개 PASS,
새 리소스0개이며 자세한 원인·검증은 RAID_SPECTATE_HUD_RESULT.md의 G10을 따른다.
현재 Release Client2개와 Server의 Product 출력 잠금 때문에 후속 제품 빌드/ZIP은 대기 중이다.
G07 ZIP은 이 보호막 후속 수정 이전 버전이며, 갱신됐다고 간주하지 않는다.

## G09. 인형·공 광기와 피해 성공 조건의 후속 분리

사용자 재검증: 인형의9/10/11 피해 숫자가 뜰 때만 광기가 오르고 피해가 없으면 오르지 않았다.
현재 실행 Server는05:31 시작/EXE05:22 빌드이며 실제 P34/P88/P89/P91/P92/P93의20 WORLD
occurrence는 authoredMadness=true,100ms repeat와 owner join이 정상이다. 현재 정책은
10%×200%/1000ms(100ms 접촉당2%,초당20%)다. Collider나 게시 누락을 원인으로 확정하지 않았다.
기존 검증은 shield가 실제로 감소하는 사례만 포함했고, Apply_Results의 damagedPlayers gate가
HP 또는 shield 감소를 요구하여 무적/부적 등으로 둘 다 유지되는 접촉은 광기도0이었다.

KoukuSaydonLogicRuntime.cpp에서 special source의 실제 내부 접촉(spatialContact+resolved center)
광기만 damage 성공 조건에서 분리했다. 범위 외 timeout에는 예외를 적용하지 않는다. Update의
범위/활성 window/World 생존/시간 정지와 생존·combatReady·일반 상태·Guide 제외 조건은 유지한다.
이 인형·공 접촉에서 새로 생성된 ABSORB event만 제거해 완전 흡수 시 피해/흡수 문구 없이
광기 snapshot이 증가한다. 부분 흡수 후 실제 HP 손실, 이전 event, 일반 공격의 ABSORB는 보존한다.
일반 브레스의 HP 기반/명시 광기는 기존 피해 성공 조건을 유지한다.

실제 Apply_Results 전후 및 Add_MadnessGauge/Can_ReceiveSpatialContact/Is_Judgeable 본문을
추출한 native CPU probe:318 checks,failures0.96가지 source/결과순서/상태 조합에서 기존
특수 광기 누락16건을 재현하고 복구했다. shield/무적/무적장판/피해0/부분흡수,제외 대상,
이전 event 보존,소수 누적/상한/광대전환 marker를 포함한다.
근거 out/MadnessContact20260930/verification.json 및 build-contact-probe.log.
이 probe의 HP sink/변신 marker는 stub이고 실제 Collider·Update 주기 통합 실행이 아니다.
기존 KoukuProduct fixture에 무적/부적/피해0/범위밖/시간정지/사망/미준비/광대 사례와
흡수 텍스트 부재 assertion을 추가했다. 이 통합 fixture 실행과 제품 빌드는 아직 대기 중이다.

사용자가 실시간 Save+Apply로 저장한 raidStaggerMaximum50000,광기 정책과 전체 Data/bootstrap은
수정하지 않았다. 새 리소스0개다. 보호막 표시 후속 수정과 함께 제품 Debug/Release 빌드·ZIP은
실행 중 Client/Server 종료 확인 후 갱신한다. 현재 게임 메모리와 기존 ZIP에는 이 수정이 없다.
