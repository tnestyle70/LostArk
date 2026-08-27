# 발탄 점프·도넛·돌진 충돌·전투 가속 구현 계획

## 1. 목표

- `VALTAN_SIX_PIZZA_106`, `VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK`,
  `VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK`의 느린 선형 상승을 짧은 급상승으로 교정한다.
- 3시/9시 지형 파괴 패턴은 기존 방향의 바닥 파괴를 유지하면서 발탄 본체만 아레나 중앙에 착지시킨다.
- SPACE, V/ALT_V, X/Z를 포함해 BA가 아닌 여섯 직업 스킬 87개의 쿨타임을 3초로 통일한다.
  BA인 LMB 콤보 7개만 제외한다.
- 여섯 직업의 플레이어 공격력을 1,000으로 조정해 모든 플레이어 공격의 기존 상대 비율을 유지한 채
  발탄 처치 검증을 빠르게 수행할 수 있게 한다.
- `VALTAN_FIST_IN_OUT`은 사용자가 확정한 실제 animation/action이 없으므로 현행
  `INNER + animation.mode: NONE + STAGE_CLOCK` tombstone을 보존한다. 과거 프리뷰용
  4-stage/clip 계약이나 다른 패턴 clip을 추측 연결하지 않는다.
- `fx_e_decal_007_2.dds`가 도넛·점프 소유로 섞이지 않는다는 소유권을 회귀 검증한다.
- 고공 점프 TAKEOFF가 끝난 뒤 4.51초/6.24초에 발화하던 도넛 파생 레거시 요소 두 개를 제거해
  점프 Effect가 source-native 단일 요소만 소유하도록 복원한다.
- `VALTAN_DASH_CHARGE`는 정면 진행 중 활성 파괴 receiver 또는 일반 blocking wall에 보스 몸체가
  처음 접촉한 fixed tick에 이동·가능한 벽 파괴·`WALL_CONTACT -> GROGGY`를 함께 끝낸다.

## 2. 현재 상태 실측

- 세 패턴의 `serverMotion` 상승 구간은 현재 `0..900ms`라 10m 높이를 TAKEOFF 전반에 걸쳐
  선형 보간한다. 공통 서버 계산은 `CValtanBrain::Advance_ArenaBreakLeap`가 담당한다.
- 같은 TAKEOFF 애니메이션의 저작 이벤트는 700ms 이후에 집중되어 있어, `800..1100ms`를
  급상승 구간으로 사용하면 800ms의 지상 예고 뒤 300ms 안에 정점에 도달한다.
- 고공 도끼 투하 `VALTAN_HIGH_JUMP`는 이미 `1133..1500ms`의 367ms 급상승과
  `0..267ms` 급강하를 사용하므로 수치를 다시 변경하지 않고 새 Server 빌드에 재반영한다.
- 3시/9시 착지 X는 각각 `174.03`, `138.03`이며 중앙은 `156.03`이다. 바닥 파괴는 착지 좌표가
  아니라 `worldeventset.valtan.terrain-destruction-3.floor84`와
  `worldeventset.valtan.terrain-destruction-9.floor30`으로 선택되므로 중앙 착지와 독립적이다.
- `PlayerSkills.json`에는 LMB BA 7개와 그 외 스킬 87개가 있다. LMB BA는 쿨타임 0을 유지하고
  나머지 입력 슬롯은 모두 변경 대상이다.
- 여섯 `PlayerProfiles.json` 행의 `attackPower`는 모두 100이다. 서버 피해식은
  `attackPower * damageRatePercent / 100`이므로 1,000으로 변경하면 모든 플레이어 스킬과 BA의
  원시 피해가 기존의 10배가 된다.
- 도넛 Effect cue가 보스 애니메이션을 정지시키는 것이 아니다. 현재 split presentation이
  `animation.mode: NONE`인 단일 INNER stage로 저작되어 있어 Client가 명시적으로 animation을
  pause한다. `Valtan.pattern.json`의 4-stage는 migration fixture이고, 과거 19_02/19_04 계약은
  현재 이미 재생 중인 animation이 아니므로 복원하지 않는다.
- 도넛 direct-authored Effect의 두 wave가 의도된 연속 표현인지 레거시인지 현 증거로 확정할 수
  없어 이번 변경에서 편집하지 않는다. `fx_e_decal_007_2.dds`는 도넛·고공 점프 문서에는 없고
  floor-wipe second-smash, swing clip-02, four-pillars target-cone 세 문서만 소유한다.
- high-jump TAKEOFF direct-authored Effect의 stage 길이는 1.933초인데, 도넛에서 복제된 두 요소는
  각각 4.51초와 6.24초 뒤 시작하도록 켜져 있어 다음 stage/패턴에 레거시 링을 누출한다.
- 돌진용 typed 계약은 이미 `WALL_CONTACT`, `PATTERNSTAGECHARGE`, receiver sweep과 exact destruction
  commit으로 존재한다. 현재 회귀는 Dash의 branch가 `WALL_CONTACT -> RECOVERY`로 바뀌어 접촉 즉시
  GROGGY가 아닌 짧은 part window로 진입하는 것이다. 플레이어 반격용 `COUNTER_HIT/counterProxy`는
  별도 계약이므로 벽 접촉 의미로 재사용하지 않는다.
- Valtan의 player-blocking collisionBox 99개 가운데 Dash receiver와 결합된 벽은 40개뿐이고, 나머지
  69개 일반벽은 `COLLIDER_CONTACT` binding만 가진다. 중앙 출발 Dash는 receiver보다 이 일반벽에 먼저
  닿을 수 있으므로 receiver-only sweep은 사용자 요구를 충족하지 않는다.

## 3. 변경 파일과 계약

- `Data/Valtan/Valtan.gameplay.json`
  - 피자와 3시/9시 파괴 점프의 상승 구간을 `800..1100ms`로 변경한다.
  - 3시/9시 `landingPosition`을 `[156.03, 22.99751, -122.06]`으로 통일한다.
- `Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py`
  - 재저작 시에도 같은 급상승과 중앙 착지가 생성되도록 저작 함수를 고친다.
- `Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py`
  - 세 패턴의 급상승 구간, 중앙 착지, 방향별 월드 이벤트 보존을 회귀 검증한다.
- `Data/Balance/PlayerSkills.json`
  - 입력 슬롯이 LMB가 아닌 87개 행을 `cooldownMs: 3000`으로 변경한다.
- `Data/Balance/PlayerProfiles.json`
  - 여섯 직업의 `attackPower`를 1,000으로 변경한다.
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`
  - 변경된 쿨타임과 공격력 필드를 `PROJECT_TUNED` 결과로 동기화한다.
- `Data/Valtan/Valtan.presentation.json`
  - Dash stage 순서만 갱신하고 도넛은 현행 NONE tombstone을 그대로 둔다.
- `Data/Effects/Authored/effect.valtan.carrier-v1.attack.high-jump.takeoff.clip-01.effect.json`
  - TAKEOFF 밖에서 뒤늦게 발화하는 도넛 파생 요소 두 개를 제거한다.
- `Tools/ValtanPipeline/author_valtan_requested_effect_elements.py`
  - 제거한 high-jump 요소를 donor로 쓰던 생성 Effect는 동일한 cataloged center-landing source로 옮긴다.
- `Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py`,
  `Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py`
  - TAKEOFF의 지연 요소 삭제와 decal texture 소유권을 고정하고 도넛 tombstone은 기존 계약을 유지한다.
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`, `Server/Private/GameplayCatalog.cpp`,
  `Server/Private/ServerGameplayContractTests.cpp`
  - Dash charge impact가 immediate GROGGY만 허용하도록 publish/load/runtime 회귀를 닫는다.
- `Server/Public/ServerCollisionSystem.h`, `Server/Private/ServerCollisionSystem.cpp`,
  `Server/Private/GameRoom.cpp`
  - Dash만 모든 활성 blocking collisionBox를 sweep하고 base collision과 같은 면의 receiver를 함께 보존한다.
  - exact receiver mutation을 먼저 시도하고, 일반벽이면 base contact mutation으로 한 번 fallback한다.
  - 이미 소모된 receiver도 벽면이 blocking인 동안 이동을 멈추고 같은 tick에 GROGGY로 전환한다.
- Valtan V2 projector가 관리하는 추적 Product 문서는 publisher 출력으로만 갱신한다.
  `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`은 검증용 생성물이며 커밋하지 않는다.

## 4. 구현 단계

### G1. 발탄 점프 이동 정본 수정

- 피자와 두 지형 파괴 TAKEOFF는 800ms까지 지상 원점을 유지하고 1100ms에 정점에 도달한다.
- 3시/9시 두 패턴의 안정 ID는 보존하고 착지 좌표만 중앙으로 바꾼다.
- IMPACT의 방향별 `TRIGGER_WORLD_EVENT_SET`은 수정하지 않는다.

### G2. 전투 가속 밸런스 수정

- LMB가 아닌 87행을 데이터로 선별해 모두 3,000ms인지 검산한다.
- 모든 LMB BA 7행은 쿨타임 0을 유지한다.
- 공격력 1,000과 기존 damage profile 비율의 조합으로 10배 원시 피해가 성립하는지 확인한다.

### G3. 생성·회귀 계약 고정

- phase-two author validator로 정본과 생성기 출력의 동일성을 확인한다.
- Valtan pattern tree 회귀 테스트에 상승 창·착지·월드 이벤트의 분리 계약을 추가한다.
- V2 projector를 publish/validate해 추적 Product와 Server bootstrap을 갱신한다.
- gameplay balance provenance를 동기화하고 publisher Validate/Publish를 통과시킨다.

### G4. 도넛 보류와 점프 착지 후 레거시 Effect 제거

- 도넛은 INNER 단일 stage, `animation.mode: NONE`, `STAGE_CLOCK +0ms` 계약을 유지한다.
- 과거 프리뷰용 WINDUP/INNER/OUTER/RECOVERY clip과 retired stage shell은 재연결하지 않는다.
- 도넛 authored Effect의 두 wave도 intended animation/action이 확정될 때까지 수정하지 않는다.
- `fx_e_decal_007_2.dds`의 실제 세 owner만 허용해 도넛·고공 점프 쪽 유입을 검출한다.
- high-jump TAKEOFF는 source-native 요소 하나만 남기고, 후속 생성 Effect가 필요로 하는 동일 payload는
  `effect.valtan.high-jump.center-landing.active` donor에서 가져오도록 저작 provenance를 교정한다.

### G5. 돌진 접촉 즉시 GROGGY 복원

- CHARGE의 `WALL_CONTACT` branch는 immediate next stage인 GROGGY를 가리킨다.
- TIMEOUT만 RECOVERY로 빠지고, GROGGY에서 PART_DESTROYED면 PART_BREAK, TIMEOUT이면 RECOVERY로 간다.
- Server loader와 publisher는 charge-impact target으로 RECOVERY를 허용하지 않고 GROGGY만 허용한다.
- Dash는 모든 활성 blocking wall을 swept boss-circle로 판정해 이동 방향의 첫 선단 접촉 안전 위치에서
  정지한다. 활성 receiver는 exact mutation을 한 번 commit하고, 일반벽은 collider contact mutation을
  사용하며, 소모된 receiver는 파괴를 반복하지 않는다.
- 벽 종류나 mutation 신규 여부와 무관하게 blocking surface 접촉은 같은 tick의 `WALL_CONTACT`이고,
  Dash stage clock을 끝까지 기다리지 않고 즉시 GROGGY로 전환한다.
- Armor Break Opening 등 비-Dash charge-impact는 기존 receiver-only 계약을 유지한다.

### G6. 반영 검증과 인계

- Valtan 집중 Python 하네스와 Server contract test를 실행한다.
- `Invoke-BuildAndRegression.ps1`의 Debug와 Release를 모두 실행한다.
- Client는 에이전트가 자율 실행하지 않는다. 사용자는 Server 재시작 뒤 Valtan Arena에서 피자,
  84줄/30줄 지형 파괴, 점프 착지 후 지연 Effect 부재, 돌진 첫 벽 접촉 GROGGY,
  쿨타임과 처치 속도를 직접 육안 확인한다. 도넛 animation은 이번 수동 PASS 대상에서 제외한다.

## 5. 완료 기준

- 세 패턴이 서버 정본과 생성 Product 모두 `takeoffStartMs=800`, `takeoffEndMs=1100`이다.
- 3시/9시 두 패턴의 본체 착지는 중앙이며 방향별 floor84/floor30 이벤트 세트는 그대로다.
- BA가 아닌 87개는 모두 3초이고 LMB BA 7개는 쿨타임 0이다.
- 여섯 직업 공격력은 모두 1,000이며 서버 피해 계산과 HUD 예상 피해가 같은 정본을 소비한다.
- 도넛은 현행 명시적 NONE tombstone을 유지하고 과거/타 패턴 animation을 새로 연결하지 않는다.
- 고공 점프 TAKEOFF 문서는 stage 밖 지연 도넛 요소를 소유하지 않는다.
- 도넛 Effect 문서는 `fx_e_decal_007_2.dds`를 참조하지 않으며 실제 세 owner 집합이 회귀 검증된다.
- Dash는 일반벽, 활성 파괴 receiver, 아직 blocking인 소모 receiver와 처음 접촉한 tick의 안전 위치에서
  정지하고 즉시 GROGGY stage로 진입한다. 신규 파괴 mutation은 한 번만 발생한다.
- publisher, 집중 하네스, Server contract, Debug/Release 정본 빌드 결과를 RESULT에 실제 결과로 기록한다.
