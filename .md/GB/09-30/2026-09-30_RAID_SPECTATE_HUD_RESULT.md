# Raid 관전·HUD·미니게임 타이머 구현 결과

## G00. 반영 범위와 완료 경계

기존 미커밋 변경을 보존하며 Client presentation, Shared deadline/codec, item cooldown 복제를
연결했다. Source 수정과 아래 자동 검증은 완료했다. Server 90초 timeout 실제 판정은 같은 작업의
Kouku 담당 변경이며 최종 product Debug/Release 빌드·publish는 통합 담당 결과를 따른다.
Client/UI는 실행하거나 조작하지 않았고 실제 화면 판정은 사용자 확인으로 남긴다.

## G01. 관전 버튼과 시점

`ClientReplication.h/.cpp`가 stable NetEntityId를 관전 상태로 유지한다. 사망한 local player의
관전 클릭마다 인간 party roster 순서로 살아 있는 다른 player를 고른다. roster 밖 audition
player는 stable entity 순서다. AI companion과 사망자는 다음 대상에서 제외한다.
현재 대상의 사망은 자동 대상 변경을 일으키지 않는다. Transform을 마지막 사망 위치에 보관하고
Camera의 `OBSERVER_HOLD` override가 당시 표시된 시점(ALT_V cut 포함)을 유지한다.
같은 ID가 부활하면 다시 추적하며 자기 player가 부활하면 자기 시점으로 돌아온다.

두 Arena의 사망창 button routing과 camera follow를 연결했다. 관전 중 배경 dim/panel은 숨기고
관전·부활 버튼은 남겼다. Kouku area shot, telescope, Mario source follow, composition audience와
Mario 조명 선택도 camera subject snapshot/transform을 사용한다. Gameplay input은 계속 local
player만 소유한다. Gate3의 이동 안내는 leader의 replicated 위치와 대기 시간을 모든 player가
표시하며 실제 진입 요청은 여전히 leader만 보낸다.

`Effect_PresentationService`는 active camera follow target과 일치하는 character에 ALT_V camera,
local-only element, afterimage를 허용한다. 이미 재생 중인 효과도 관전 대상을 바꾸면 element mask를
다시 선택한다. `Character`의 skill shake는 같은 camera subject를 따른다. 원본 효과·셰이더·렌더링
설정 값은 변경하지 않았다.

## G02. Server tick 기반 90초 제품 타이머

`PLAYER_SNAPSHOT::iKoukuMinigameEndTick`은 absolute tick이며 0은 비활성이다. stage 바로 뒤
U32 writer/reader, HUD player projection과 protocol 125를 함께 적용했다. Server 담당이 설정한
입장 tick + 2700을 사용하며 Client는 snapshot 사이 경과 시간만 보간한다. 관전 중에는 대상의
해당 deadline을 표시한다. 로컬 60초 Mario/150초 maze 추정 타이머는 제거했다.

DungeonTimer text는 maze의 일반 HUD 숨김 gate와 분리되어 Debug/Release 공통으로 그린다.
노란 분:초 표시는 동일 view를 사용하며 30초 이하 빨간 소수점은 한 문자열, 동일 34px 기준과
중심으로 표시한다. Debug preview 기본값도 90초로 맞췄다.

## G03. 체력·보호막과 에스더

HP fill은 왼쪽, shield rect는 그 HP fill 끝에서 시작한다. HP+shield가 최대 HP를 넘으면 기존
비율 계산을 유지하면서 두 fill을 한 track에 나란히 배치한다. shield는 기존 흰 texture와 slot tint를 사용한다. 사용자 테스트에서 확인된 0폭 이후 표시 누락과 후속 수정은 G10에 기록한다.
Valtan에서는 매 update에 Bahuntur/Wei/Sillian portrait를 명시하여 이전 Kouku texture가 남지
않는다. Kouku의 Nineveh/Wei/Inanna 매핑은 보존했다. 새 image asset은 추가하지 않았다.

## G04. 아이템과 피해 감소 문자

실측상 Server는 포션과 배틀 아이템 모두 기존 Inventory의 수량을 성공마다 1씩 소비하며
Client slot ID는 사용 후에도 남는다. 별도의 한 번만 사용 가능한 latch는 발견하지 못했다.
명확한 결함은 배틀 아이템 30초 cooldown이 Client에 복제되지 않아 재사용 거절 이유를 화면에서
알 수 없던 점이다. Server `ItemCooldownEndTicks`를 같은 catalog skill ID의 snapshot cooldown으로
복제했다. Client `ItemCatalog`는 skill/cooldown 값을 검증하여 읽으며 quick slot은 남은 초,
재사용 중 dim, 실제 Inventory 수량을 표시한다. 수량 0에도 배정 ID를 유지하므로 다시 획득하면
같은 슬롯을 사용한다. 실제 LanceMaster cooldown 22개 + item 4개를 위해 상한을 16→32로 넓혔다.
현 저장 catalog에서 item/class skill ID 충돌은 없다.

실제 Duration 감소를 적용한 Server flag에만 숫자 아래 흰색 `피해 감소`를 72% 크기와 같은 fade로
그린다. `CRITICAL_DAMAGE_REDUCED`는 기존 노란 critical 숫자를 유지한다. 피해량 계산이나
실제 item consumption 판정을 Client로 옮기지 않았다.

## G05. 실행한 검증

- LAN sync: server-host 192.168.0.22, firewall 준비 완료. endpoint not-listening은 환경 상태로 기록했다.
- 최신 Shared `PacketMessages.cpp`와 NetworkProtocolHarness를 직접 컴파일·링크하여 전체 실행:
  `out/RaidSpectateHud20260930/protocol-all.log`, `failures : 0`. deadline 포함 모든 Mario stage
  roundtrip와 갱신한 snapshot 크기/offset fixture가 포함된다. protocol 125 및 새 damage flags가
  같은 codec으로 통과했다. 한 테스트 표시 문자열의 v124→v125 교정은 이 실행 뒤 적용했다.
- 실제 `ClientReplication` 구현 6개 함수를 추출한 native 정책 검증:
  `spectator.run.log`, 20 checks, failures 0. roster/guide/사망자 제외, 다음 순서, 생존자 부재,
  죽은 위치 유지, 동일 ID 부활, local 부활, disconnect와 fallback order를 검사했다.
- 변경 Client 9개 TU의 `/Zs /Y-` Debug/Release를 시도했다. CombatHUDViewModel, DungeonTimerView,
  Camera_Free는 양쪽 통과했고 나머지는 동시 수정 중이던 KoukuComposition header의 array19에
  20개 initializer가 들어간 오류로 멈췄다. 이 header 오류는 통합 담당이 수정했으며 이후 검증은
  중앙 product build로 일원화했다. 따라서 이 시도를 전체 Client compile 성공으로 기록하지 않는다.
- `git diff --check` 통과. C++는 원래 byte encoding/CRLF를 보존하는 부분 치환이며 새 C++ 파일이
  없어 vcxproj/filters 항목 추가는 없다. 실행 데이터·Resources는 이 영역에서 교체하지 않았다.

## G06. 사용자 화면 확인

다른 인간 player가 있는 raid에서 local 사망 후 관전 버튼을 반복해 party 순서를 확인한다.
대상이 ALT_V를 사용하면 컷과 효과가 대상의 시점을 따르는지 확인한다. 대상 사망 시 시점 유지,
동일 player 부활 시 재추적과 자기 부활 복귀를 확인한다. Release에서 Mario/card maze 입장 시
1:30 및 빨간 소수점 정렬, shield 오른쪽, Gate3 전원 안내, Valtan 초상화 순서를 확인한다.
아이템 장착 후 1~4 키 사용 시 재고 감소/30초 재사용 표시와 시간 경과 후 같은 슬롯 재사용을
확인한다. Duration 감소가 적용된 보스 피해 숫자 아래 흰색 표기를 확인한다.

## G07. 통합 검증에서 발견한 기존 Beam validator 계약 누락

통합 담당 요청으로 Effect validator 첫 실패를 조사했다. `cascadeBeamV1`은 HEAD의 native codec
Internal token/JsonPrimitives/PortableRuntime에서 이미 지원하지만 Python `CARRIER_KEYS`에
없었다. git clean인 `effect.kouku.common.trumpet.radial.lasers`의 9월 13일 문서가 HEAD/current
validator 모두 동일하게 실패했다. 기존 dirty codec의 Character/SourceMaterials 허용 및 validator의
label 검사/빈 carrier 허용과 별개의 오래된 상호계약 누락임을 확인했다.

승인된 소폭 수정은 Python validator와 그 테스트에만 적용했다. beam의 4개 identity field와
rendererShape=beam, 정확 TypeDataBeam2 class 및 stable ID의 단일 join을 확인한다. ribbon도
정확 class의 같은 join 경로를 사용한다. C++와 원작 effect JSON은 바꾸지 않았다.
신규 정상 beam/타 class/유사 문자열 class/틀린 shape/누락·중복 join을 포함하여
`python -m unittest discover -s Tools/EffectPipeline -p test_validate_effect_sources.py`의 53개가 통과했다.
기존 Beam 문서 5개의 runtime-extension 검사는 모두 통과했다.

전체 validator 재실행은 다음 기존 baked history `valtan.trail.7fcde5bbca0fbc103367e216`의
clamp/sample closure 오류에서 멈췄다. `out/RaidSpectateHud20260930/beam-validator-full.log`에
실패를 보존했으며 전체 Effect 검증 성공으로 기록하지 않는다. 추가 연쇄 수정은 진행하지 않았다.

## G08. 일반 incoming 피해의 서버 단일 표본

추가 승인 범위로 `ServerCombatHitRuntime::Apply_WorldToPlayer`에서 방어·받는피해 buff 계산
직후, shield 흡수 직전에 고정 ±10% 정수 균등 표본을 한 번 적용했다. `SERVER_PLAYER`의
server-only `iIncomingDamageSampleSerial`이 동일 tick의 서로 다른 hit를 구분한다. tick/entity와
serial을 SplitMix64로 섞고 rejection sampling으로 modulo bias 없이 정수 범위를 고른다.
보안용 RNG가 아니며 Client와 protocol에는 이 내부 상태를 보내지 않는다.

범위는 base ± floor(base/10)로 평균을 그대로 유지한다. uint32 상한 근처는 양쪽 폭을 똑같이
줄여 overflow와 한쪽 clamp 편향을 막는다. 0 피해와 사전에 차단된 hit는 serial을 소비하지 않으며
`bInstantDeath`/`bEncounterWipe`는 표본 함수를 호출하지 않는다. 기존 shield split, death-deny,
overkill 이후 실제 HP 차감량만 event에 넣는 동작은 유지한다. PlayerSkillSystem의 outgoing RNG,
outgoing HP/stagger/저작 madness gain 및 balance JSON은 변경하지 않았다.

BattleItems suite 말미에 20,000회 범위·평균·동일tick 다양성,100→90..110,동일상태 재현,
mitigation 이후 범위,shield+HP event 일치,0/무적 제외,즉사/wipe 제외,overkill actual cap 검증을
추가했다. 동일 unsigned64 산술의 별도 수치 확인은 1320 표본20,000개에서 min1188/max1452,
평균1319.24905이며 새 deterministic assertion(평균오차0.2% 이내)에 들어간다. product C++ compile
및 실제 contract suite 실행 판정은 중앙 담당이 이 수정 이후에 실행한 통합 결과를 따른다.
`git diff --check`는 세 변경 Server 파일에서 통과했다.

후속 테스트 기대 조정은 WorldDestruction/KoukuLogic 두 기존 TU의 관련 assertion에만 적용했다.
Bahuntur의 raw20은 50%감소 후9..11,효과 만료 후18..22 범위와 event=실제HP손실을 확인한다.
차단은 직전 HP/보호막 보존,Inanna 탈출은9..11 범위와 실제 손실을 확인한다. Kouku no-policy
10피해는 실제 손실/event 일치를 검증하면서 광기불변을 유지하고,무적은 직전HP·event개수와
광기불변으로 확인한다.7피해의 shield5+HP2와200%광기gain 의미는 기존 그대로다.
두 파일의 무관한 기존 dirty 부분은 보존했고 diff check를 통과했다.

## G09. 최종 통합 검증

최종 Debug/Release Product 빌드가 완료됐다. BattleItems는 양 구성 각각 145 PASS/실패0로 20,000개 난수 표본·실제 HP/흡수 event·독립 무력화·피해 감소 flag·강제 사망을 검증했다. 고정 피해를 기대하던 overlap 회귀는 대상별 개별 난수 범위 및 event 합계=실제 HP 손실로 갱신했고 Debug ObjectOverlap1099 assertion 전체가 통과했다.

최종 빌드 영수증·ZIP·한계는 [통합 RESULT](2026-09-30_RAID_GAMEPLAY_REPAIR_RESULT.md)를 따른다.

## G10. 사용자 테스트 후 보호막 채움 누락 수정

첨부 화면의 HP129116/132000,shield120000에서 검게 보인 구간을 조사했다. Shield Bar.png의
중앙 RGBA는(214,212,214,255)이며 slot tint도 흰색이다. 원인은 MainApp이 shield0에서
Set_SlotRect의 폭을0으로 만든 뒤, CUI_Sprite::Apply_Transform이 기존 RIGHT를 normalize하는
CTransform::Scale에 다시 의존하여 이후 양수 폭을 복원하지 못하는 것이었다. 검정은 새 보호막
색이 아니라 흰 quad가 표시되지 않아 남은 배경이다.

Client/Private/UI_Sprite.cpp의 Apply_Transform이 현재 rect×viewport로 RIGHT/UP/LOOK을
직접 재구성한 뒤 기존 authored rotation과 position을 적용하게 수정했다. Engine의3D Scale,
HP/보호막 계산·오른쪽 배치·texture·shader는 그대로다. 신규 Resources 및 GBResources 추가는0개다.
기존 UTF-8/CRLF를 유지했고 신규 제품 C++ 파일이나 project/filter 추가는 없다.

실제 수정 전/후 Apply_Transform 및 Engine Scale/Rotation/Get_Scaled 본문을 추출한 native
DirectXMath probe:753 checks,failures0. 기존 quad 소실80회 재현/수정후80회 복구,
4개 회전×4개 viewport×8개 보호막 상태=128개 전이를 검증했다. 기존 양수 크기 경로의 동등성,
0→부여,소진→재부여,0 높이·양축 복원,유한 행렬과 중심·크기도 포함한다.
근거는 out/HudShield20260930/build-transform-probe.log와 extraction-receipt.json이다.
이 검증은 CPU 행렬/함수 컴파일이며 제품 전체 빌드·GPU 표시 성공을 대신하지 않는다.
git diff --check는 통과했다.

후속 제품 빌드/ZIP은 대기 상태다. ProductOutputGuard가 실제 Release Client PID81032/81248와
Server PID71132의 실행을 감지했다. 프로세스를 임의 종료하지 않았고 사용자가 종료를 알리면
Debug/Release 빌드와 ZIP을 갱신한다. G09와 통합 RESULT의 기존 ZIP은 이번 수정 이전 결과다.
실제 화면은 새 Client 실행 후 사용자가 확인한다.
