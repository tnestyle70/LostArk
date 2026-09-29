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

## G10. PR487·488 통합과 현재 프로토콜

PR487의 Waterpang·Ancient Sea와 PR488의 아바타 상점·착용 변경을 통합한 소스를 기준으로
Network protocol은 126이다. G05의 protocol125와 G01의 최초40000은 당시 검증 기록이다.
현재 사용자가 저장한 Retail 공통 무력화 최대치50000을 유지하며, 검사 통과를 위해 과거
수치나 저작 패턴 시간을 되돌리지 않았다.

통합된 ProtocolHarness는 Debug/Release 각각1400 PASS, failures0이다. 상점 금화와 인벤토리,
아바타 두 슬롯·snapshot ID, 비행·Esther·마리오·World occurrence의 현재 packet 계약을
검증했다. PR487의 기존 Server 검사도 Release에서 Waterpang34 PASS, VehicleRiding99 PASS,
NPC raid return34 PASS, 각각 exit0으로 완료됐다. UI 화면·아바타 외형·음향 확인은 포함하지 않는다.

근거는 `out/ValtanFinalRepair20260930/protocol-{debug,release}-test.log`,
`release-native-results.json`과 각 `release-*-contract.log`다. GitHub의 최종 merge 상태와
최종 배포 완료는 이 실행 결과만으로 확정하지 않는다.

PR 리소스 통합 receipt에는399개 파일·341,771,794 bytes의 hash/길이가 기록되어 있으며,
그중389개는 GBResources로 복사한 것으로 기록됐다. 별도로 아바타 아이콘30개가
`missingAssets`에 남아 있다. 리소스 복사 완료를 해당 아이콘이나 실제 제품 화면 확인 완료로
확대하지 않는다. 근거는 `GBResources-pr-integration-receipt.json`이다.

## G11. 파괴폭탄 전용 갑옷과 아바타 실제 소비 검증

발탄 primary의 일반 몸체 갑옷은 서버가 확인한 파괴폭탄 projectile impact에서만 파괴된다.
일반 공격·스킬·파괴폭탄과 같은 skill ID만 넣은 hit는 HP 피해를 주더라도 갑옷 내구도를
차감하지 않는다. 방벽 충돌은 DASH의 무력화 가능 구간을 열며 갑옷을 자동으로 제거하거나
복원하지 않는다. 다른 보스의 기존 part-damage 경로는 유지한다.

Release `--battle-items-contract-test`는174 PASS, failures0, exit0으로 완료됐다.
이 숫자는 아래 항목을 포함하는 suite 전체이며 갑옷 검사만174개라는 뜻은 아니다.

- 실제 입장·인벤토리·사용 handler·projectile contact를 거쳐 두 번의 groggy 구간에서
  갑옷 두 부위의 typed/legacy 상태가 제거되는 것을 확인했다.
- 범위 밖·중복·cooldown 요청은 추가 소비나 projectile을 만들지 않았다. 갑옷이 모두 제거된
  뒤 네 번째 실제 파괴폭탄을 던져도 추가 파괴 사건·복원이 발생하지 않았다.
- 반복 방벽 impact에서도 갑옷 수치가 변하지 않았고, 일반 스킬·잘못된 출처의 hit 차단과
  일반 몸체 phase 전환 후 폭탄 전용 조건, 다른 보스의 part-damage 보존을 확인했다.
- 네 관찰자의 실제 packet decode에서 비행·폭발·제거·갑옷 snapshot이 일치했다.
- 아바타는 지원5클래스에 template 두 개가 정확한 클래스 ID로 지급되고 금화가 차감됐다.
  미지원 Gunslinger/Slayer는 구매 전체를 거부하면서 인벤토리·금화를 보존했다.
- 아바타 머리·의상 장착은 일반 HELMET/TOP을 유지했다. 잘못된 클래스 장착은 거부하고,
  네 관찰자에게 소유자의 같은 entity ID로 두 아바타 ID를 복제했다. 머리만 해제하면 의상과
  일반 장비는 유지되고 머리의 복제 필드만 비워졌다.

최초 BattleItems157 PASS/1 FAIL은 `commonMaximum == 40000`이라는 고정 기대값이었다.
최종 검사는 현재 catalog의 공통 정책·보스 maximum·표시 gauge의 일치를 검사하고,
일부러 다른 authored fallback을 두어 정책 우선 적용을 확인한다. 회오리의 skill/divisor는
게시 ItemCatalog를 소비하며 1/3 누적·3회 완료 조건을 유지했다. 50000으로 문자열이나
기대 상수만 교체하지 않았다.

근거는 `release-targeted-battle-items-contract.log`와
`release-targeted-native-results.json`이다.

## G12. 쿠크 후속 광기와 피해 판정 검증

기존 `--kouku-product-contract-test`에 기존 Logic runtime 검사를 연결해 Release96 PASS,
failures0, exit0을 확인했다. 새 CLI나 별도 제품 경로를 만들지 않았다. G09에 대기로 남겼던
인형·공의 실제 Logic Update 통합 검사는 이제 보호막 흡수·무적·부적·피해0 접촉의 광기 증가와
흡수 문구 부재를 확인했다. 범위 밖·시간 정지·사망·미준비·광대 상태의 제외 조건도 유지했다.
Mario/CardMaze90초 deadline·대상별 사망 범위와 탈출 시 deadline 제거를 같은 suite에서
확인했다. 이 결과는 사용자 실시간 화면에서 간헐 문제가 완전히 사라졌다는 보증은 아니다.

룰렛·Collider·Circle·이동 World trigger·sweep·QWER의 기존7개 실패는 일반 피격의±10%
변동 이후에도 HP900/800/500을 고정 요구한 fixture였다. 현재 검사는 허용 범위와 target별
정확한 NORMAL event 개수·event 합과 실제 HP 차감의 일치를 함께 요구한다. 첫 타격 이후
HP 불변, 닫힌 창 재평가 시 중복 타격 부재, 성공·실패·timeout 판정은 보존했다.

근거는 `release-targeted-kouku-product-contract.log`와
`release-targeted-native-results.json`이다. 이 Release96개 결과를 과거 Debug350개와
동일한 전체 실행 범위로 표현하지 않는다.

## G13. 마력구 독립 무력화 연결의 현재 상태

추가 조사에서 `VALTAN_STAGGER_SLOT/CHANNEL`은 아직 누적 HP 피해 response를 사용하고
있었다. 소스 threshold10000과 generator의 과거1000 모두 기존 독립 무력화 경로에 연결되지
않아 일반 피해/1000과 회오리1/3로 해당 창을 완료할 수 없었다. 이는 단순히 오래된 fixture의
기대값만 잘못된 경우와 구분되는 제품 연결 누락이다.

적용 receipt 기준11개 소스·검사 파일에 기존 ENTER/EXIT의 `SET_STAGGER_GAUGE`와
`STAGGER_BROKEN -> VALTAN_GROGGY_FOLLOWUP` 연결을 반영했다. 별도 HP response나
두 번째 damage runtime을 만들지 않았으며, 현재 공통 최대치50000과 이후 F1 저장값을
기존 catalog에서 소비한다. 12000ms channel, +0.5m 높이, 3000ms 실패 stage 및1000ms
실패 타격 시점은 유지한다. Bind 검사는 현재 저작 hold4107ms를 읽도록 교정하며 저작 시간을
5000ms로 되돌리지 않는다.

후보 Python4/4, V2 join/projector8개 산출물, 구조·hash·인코딩 검사와11개 파일 적용에 이어
최종 Release 기존 presentation suite가27 PASS, failures0, exit0으로 완료됐다. 이 검사는
마력구만이 아니라 속박·단독 속박·침묵·돌진·발악·부위 파괴 회복을 함께 포함한다.
Lifecycle94개와 실제 BattleItems174개도 같은 최종 Release 집계에서 실패0이다.

속박 fixture의 남은 실패는 실제 EXIT 복원과 같은 tick의 플레이어 넉백 갱신을 혼동한 위치
검사였다. 제품은 살아 있는 속박 대상의 피격 가능 상태를 유지하고 `bPatternBound`로 입력을
차단한다. 테스트는 기존 단계 전환 직후·플레이어 갱신 전의 읽기 전용 관찰 지점에서 저장XYZ,
owner/endTick 해제, HP와 남은 넉백 불변을 확인한 뒤, 실제 플레이어 갱신의 넉백 진행을
별도로 확인한다. 위치 조건을 삭제하거나 피해·무적·넉백 정책을 바꾸지 않았다. 저장 시간
4107/3533ms도 유지했다. tick2000 입장은2123 RECOVERY,2229 완료이며 단독 fixture는
같은 검사를600tick 뒤에서 수행했다. 자세한 실측은 `bind-result-candidate.md`에 기록했다.

최초 추가 게시의 world.destruction 실패는 후속 보완과 재게시로 해소됐다.
`full-publish-final2.log`가 전체 DataOnly 완료를 기록하며 source revision은
`128d68d30634ff0c51eaab5f6e4669434842014cc795997abfe803e2c55eedc4`다.
마력구·Bind의 최종 Release 결과는 `release-final-native-results.json`,
`release-targeted-valtan-presentation-contract.log`,
`bind-exit-boundary-candidate/final-source-verification.json`을 따른다.

## G14. 광역 검사 실패와 최종 전달의 남은 경계

통합 후 최초 Release `--contract-test`는1459 PASS/91 FAIL, exit1로 완료됐다.
`RoomRuntimeFailure`를 의도적으로 발생시키는 rollback 검사는 실패 개수에 넣지 않았으며,
실제 `[FAILURE]`와 종료 코드를 기준으로 기록했다. 이 전체 suite를 PASS로 간주하지 않는다.

대표적인 구형 fixture 불일치는 다음과 같다.

- 은퇴한 갑옷 입장 HEALTH_BAR159·managed rotation 행을 문자열 그대로 찾지만 현재 Product는
  AUDITION_ONLY와 저장된 ORDERED_LOOP를 사용한다. 해당 과거 행을 복원하지 않는다.
- 고공 도끼는 현재 AIRBORNE와 object lifetime이 모두7984ms인데 fixture는8000ms를
  고정 요구한다. object-local1200ms hit와 LAND3200ms는 유지되어 있다.
- DASH WINDUP/GROGGY의 저장값7350/6897ms와 fixture의3650/6833ms가 다르다.
- 과거 SpawnGroups 검사는 protocol101을 요구하지만 현재 통합 계약은126이다.

91개 전부가 fixture 문제라고 결론 내리지는 않았다. queued generation 이후 재선택 등
복합 실패와 범위 밖 스킬·Bern navigation·wall climb 검사는 개별 원인을 끝까지 분리하지
않았다. 이 남은 검사를 이번 수정 성공으로 바꾸거나 판정을 약화하지 않았다. 자세한 실제
실패 목록·로그 위치·관련 범위 및 대표 근거는 `release-native-failure-audit.json`과
`native-fixture-drift-audit.json`에 남겼다.

최종 Release는 새 CLI 없이 기존7개 범위를 실행해 합558 PASS, failures0으로 완료됐다.

| 기존 suite | PASS | 결과 |
|---|---:|---|
| BattleItems | 174 | exit0 |
| KoukuProduct | 96 | exit0 |
| ValtanLifecycle | 94 | exit0 |
| ValtanPresentation | 27 | exit0 |
| WorldPlayback | 34 | exit0 |
| VehicleRiding | 99 | exit0 |
| NPCRaidReturn | 34 | exit0 |

`release-final-native-results.json`이 이 집계의 정본이다. 앞서 기록한 presentation10개는
Timelines만 연결했을 때의 중간 결과이며, 최종27개에는 Magic/Bind를 포함한 추가 기존 검사도
들어 있다. 부위 파괴 recovery의155tick 유지/156tick 종료,5183ms clock과 돌 definition·live
object·피해·object lifecycle 부재는 유지했다. PNG는 갑옷이 남은 파괴 가능 구간을 알리는
ready 표시이며, 실제 `PART_BROKEN`의 갑옷 제거·파편·성공 text와 서로 다른 조건이다.

전체 DataOnly는 위 final2 로그로 완료를 확인했다. Debug Product도
`out/BuildPipeline/runs/20260929T233149354Z-debug-product.json`에 PASS가 기록됐다.
그 뒤 Bind fixture만 바뀌었으므로 해당 최신 테스트 소스를 포함한 Debug 재빌드·7범위 검증은
아직 별도 완료 확인이 필요하다. 현재 Debug 중간 기록의 BattleItems174 PASS를7범위 전체
완료로 확대하지 않는다.

최종 Release Product 재빌드는 실행 중인 검증용 Debug Server PID45512 때문에 output guard에서
컴파일 시작 전에 차단됐다(`product-release-final.log`). 따라서 초기 통합 Release 빌드나
최종 Server native558개 통과를 전체 최신 Client/Server Release Product 완료로 대체하지 않는다.
최신 Debug·Release 빌드, Debug native 최종 집계, GitHub merge와 새 ZIP hash·CRC·preflight는
담당자의 완료 확인 후 추가한다. G07 기존 ZIP은 이 후속 변경을 포함한 새 ZIP이 아니다.
