# 2026-09-04 발탄 데미지 없는 패턴 hit collider 저작 구현 계획서

브랜치 `GB/KoukuSaydon-Main-Pattern`. 이 문서는 현재 구현·검증 중인 범위를 정의한다. 실측 근거는
`Data/Valtan/Valtan.gameplay.json`, `Data/Valtan/Valtan.combatobjects.json`, `Data/Valtan/Valtan.presentation.json`,
`Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`, `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json`과
`Server/Private/ValtanBrain.cpp`, `Server/Private/CombatObjectRuntime.cpp`, `Server/Private/GameRoom.cpp`다.

09-04 현재 G00~G11의 authoring/data/Server/Client 변경은 working tree에 반영되어 있다. 마지막 데이터 기준
Debug Product와 Server contract는 PASS했고, FullDiagnostic는 사용자가 실행한 Client/Server의 output lock에서 안전하게
중단됐다. 실행 증거와 사용자 수동 판정은 대응 RESULT에서 분리한다.

### 09-04 구현 순서 확정

이번 변경 단위는 G00~G11의 누락 collider·damage·effect·sound, 돌 폭발/엄폐, 즉시 침묵과 자동 parity gate까지 먼저 닫는다.
G12의 잡기 attachment 진단은 이 변경의 완료 gate가 아니며 후속 진단으로 남긴다. 아래 항목은 구조와 검증 축이 달라 현재 변경에
섞지 않고, 기존 계획의 RESULT까지 작성한 뒤 별도 구현 계획서로 분리한다.

- `Data/Balance/PlayerSkills.json`의 Q/W/E/R 28개는 공용 boss counter capability인 `counterPower=1`을 사용하고,
  기존 counter 스킬 A/34580의 값 1도 유지한다. 실제 damage hit가 Server overlap에 landed 된 경우에만 현재
  `COUNTERABLE` window와 기존 front/local proxy를 통과해 counter 성공으로 소비한다. window 밖·후방/외부·허공·중복은
  계속 거부하고, 성공 occurrence는 기존 one-shot `COUNTER_HIT` outcome과 authored branch를 따른다.
- 마지막 유령 발탄 패턴 안에서 부활한 본체 유령은 사용 가능한 6종 스킬을 정해진 순서로 계속 반복한다.
- 같은 마지막 패턴 안에서 보조 유령은 walkable navigation의 무작위 유효 위치에 나타나 사용 가능한 스킬 하나를 무작위로
  시전한 뒤 사라지고, 다음 위치에서 같은 사이클을 반복한다.
- 같은 마지막 패턴 안에서 삼각 포탈은 본체/보조 유령과 독립된 내부 루프로 반복한다. 정삼각형 외접 반지름은 6m이고,
  각 포탈은 사용자 승인 Warp 기준선인 `0.0s 생성 → 0.3s 돌진 시작 → 1.6s 도착 → 1.9s 완전 소멸`을 재사용한다.
- 위 세 루프는 일반 pattern rotation에 따로 등록하는 패턴 셋이 아니라, 마지막 유령 발탄 패턴이 시작·중단·reset을 함께 소유하는
  하위 상태다. 본체 순차 루프, 보조 유령 무작위 루프, 삼각 포탈 cadence는 각각 독립된 timer/cursor를 가진다.

## 1. 목표와 종료 증거

| # | 목표 | 종료 증거 |
|---|---|---|
| 1 | combat object hit 모양을 Debug wire로 볼 수 있다 | F1 `Live Combat Geometry`에 `Combat Object Hit` 항목이 있고 도넛 RING·도끼 CIRCLE이 Server pose에 그려진다 |
| 2 | 도넛 collider가 도넛 이펙트 크기와 같다 | `innerRadiusM/outerRadiusM`가 impact ring 반지름(spawnShape radius × worldScale)과 일치하고 wire가 이펙트 링 위에 겹친다 |
| 3 | 3연속 공격·twohand 계열이 V2 `boss.valtan.twohand`·Sound와 같은 ms에 판정하고 기존 THREE 3타의 첫 pulse를 잃지 않는다 | 공통 contact는 V2 `startMs`와 같고, THREE STEP_03은 기존 500ms pulse + 공통 1300ms contact를 유지하며 500ms occurrence-specific impact/Sound와 exact `EXTRA_HIT` waiver를 가진다 |
| 4 | 십자돌·돌 폭발·피자 착지가 Server 데미지를 준다 | `Server.exe --contract-test`에 각 패턴 hit case가 있고 PASS |
| 5 | 피자 패턴이 중앙 착지 → 쿵 → 사자후 → 도약 착지 → 모아치기 순서로 각각 hit·이펙트·사운드를 가지고, 도약 착지(STEP_07 250ms) 원 전체 데미지는 돌 뒤에서 막힌다 | contract test: STEP_03/04/05/07/11 hit 행이 bootstrap에 있고 STEP_07 원 안 플레이어는 LANDED, 돌 원과 교차하는 플레이어는 건너뜀 |
| 6 | 침묵 슬롯이 시전 즉시 침묵을 건다 | `SET_PLAYER_SILENCE`가 STEP_01 ENTER에서 적용되고 snapshot `iSilenceEndTick`이 시전 tick + duration이다 |
| 7 | 같은 animation clip을 쓰는 모든 stage가 같은 clip-local ms에 같은 이펙트·hit·사운드를 가진다 | `validate_valtan_clip_template_parity.py`가 `Data/Valtan/Valtan.cliptemplates.json`의 clip마다 모든 occurrence를 검사해 PASS |

## 2. 현재 실측

### 2.1 데미지 경로는 세 개뿐이다

- stage `hit.shape.kind != NONE` → `ValtanBrain::ApplyPatternHit` (매 tick, `schedule` 또는 `activation` 시각).
- combat object `hits[]` → `CombatObjectRuntime::Update` (`TIMED atMs` 또는 `CONTACT`).
- `DAMAGE_GRABBED_PLAYERS` / `EXECUTE_GRABBED_PLAYERS` (잡힌 플레이어만, shape 없음).

데미지 없는 패턴은 이 셋이 전부 비어 있다. 이펙트·사운드는 Client cue이며 판정과 무관하다.

### 2.2 데미지 없는 패턴의 시각·사운드 시계

| 패턴 | stage 시계(ms) | 이미 있는 시각/사운드 시각 | 이번에 넣을 hit |
|---|---|---|---|
| `VALTAN_THREE` | STEP_01 1800 / STEP_02 1200 / STEP_03 2067 | twohand V2 1617 / 963 / 1300, Sound Shot 1400 / 650 / 900 | STEP_01·02 신규, STEP_03은 기존 500 유지 + 1300 정렬 |
| `VALTAN_SEQUENCE_TWOHAND` | STEP_01 1500 / STEP_02 2600 | twohand V2 STEP_02 1033 | STEP_02 |
| `VALTAN_ROAR_CHARGE` | STEP_01 1400 / 02 900 / 03 4433 / 04 1400 / 05 1000 / 06 1500 | shout.burst V2 STEP_03 733, Sound Shot7 STEP_03 100, STEP_06 Shot2 1 | STEP_03(포효), STEP_06(돌진 끝) |
| `VALTAN_SEQUENCE_RUSH` | STEP_01~03 1500 / 1500 / 4733 (`mesh_att_battle_4_01`) | 없음 | 각 STEP 전방 BOX |
| `VALTAN_SEQUENCE_WHIRLWIND` | STEP_01 1333 / 02 533 / 03 1467 (`20_02~04`) | 없음 | STEP_02·03 CIRCLE |
| `VALTAN_CROSS` | STEP_01 3000 | cross cue 1617(root), Sound Shot1 1400 | STEP_01 CROSS 1617~2117 |
| `VALTAN_GROUND_ROAR` / `STRUGGLING` / `PART_BREAK` / `SIX_PIZZA_106` 돌 | 일반 돌 hit age 5000 / 피자 돌 hit age 19500, 피자 수명 20700 | primary wave가 각 hit age에 시작, Sound `ProjExp1`은 같은 `hitId`의 `HIT_PULSE`에서 즉시 재생 | 돌 archetype별 `hits`와 active terminal wave를 같은 object-local clock으로 고정 |
| `VALTAN_SIX_PIZZA_106` | STEP_01 0 → STEP_07 19200 → 끝 29900 | composite cue: 착지 layer 5.69 s·ring 5.8 s·부채꼴 11.0 s·ring/red overlay/sky-wave 19.5 s·부채꼴 23 s·whirlwind mesh 28.5 s / shout.burst V2 STEP_07 733(착지 stage에 잘못 놓임) / Sound STEP_04 Shot4 2100·STEP_05 ShotVox3 1300·STEP_07 Shot5 250·STEP_11 Shot1 1 | G05 표의 STEP_03·04·05·07·11 hit |

STEP 누적 시작: STEP_01 0, 02 1200, 03 2200(착지 travel 0~267), 04 3400, 05 6200, 06 11200, 07 19200, 08 20400, 09 21400, 10 22600, 11 28600.

### 2.3 지원되는 collider 모양

- stage hit: `CIRCLE(outerRadiusM)`, `RING(inner, outer)`, `CONE(angleDegrees, lengthM)`, `BOX(lengthM, halfWidthM)`, `CROSS(lengthM, halfWidthM)`, `SIX_DIRECTIONS(lengthM, halfWidthM)`. 앵커는 보스 pose 또는 `STAGE_ORIGIN` + forward/right/yaw 오프셋. 시계는 `schedule INTERVAL`, `schedule EXPLICIT_OFFSETS`, `activation ACTIVE_WINDOW(startMs, lifetimeMs, 대상당 1회)`.
- combat object hit: `CIRCLE`, `RING`, `FORWARD_BOX`, `CONE`. 객체 forward = 스폰 시 yaw. `TIMED(atMs, repeat)` 또는 `CONTACT`.
- 엄폐(`IsShieldedByCover`)는 stage hit에만 있고 cover 원은 `EncounterPropRuntime`의 INTACT 기둥뿐이다. combat object hit에는 엄폐가 없다.

### 2.4 제약

- 기존 `GameRoom.cpp`의 `visualCardinalRocksMayStartOffNavigation`는 `Hits.empty()`인 시각용 돌만 navigation 밖 시작을 허용했다.
  구현은 hit이 있는 돌을 최대 2m 안의 walkable point로 투영하며 실패 시 volley 전체를 commit하지 않는다. 같은 stage가 inline hit과
  `SPAWN_COMBAT_OBJECT_VOLLEY`를 함께 소유하는 예외는 `GROUND_ROAR/STEP_01/ground-roar.rock`과
  `STRUGGLING/STEP_04/struggling.rock-pillar` 두 exact tuple만 publisher와 Server parser가 허용하고 나머지는 계속 거부한다.
- `CombatObjectRuntime::Tick`은 `hits`가 있으면 `hit.strHitId`로 `HIT_PULSE`를 보내고, Client `Apply_CombatObjectPresentationEvent`는 모든 `HIT_PULSE`에 `hitEffectAssetId`를 재생한다. `presentationEvents`와 `hits`를 같이 두면 폭발 이펙트가 두 번 난다.
- combat object Sound는 `Valtan.combatobjectsoundcues.json`의 `presentationEventId`/`hitId` 키로 찾는다.
- `RADIAL_AROUND_ARENA_CENTER`는 world 절대 각도다. 피자 composite cue는 `arena.center.target-follow` 앵커라 부채꼴이 잠근 대상 방향으로 회전한다.

## 3. 변경 파일

| 영역 | 파일 | 변경 |
|---|---|---|
| Client Debug | `Client/Public/ClientReplication.h`, `Client/Private/ClientReplication.cpp`, `Client/Private/MainApp.cpp`, `Client/Private/Valtan.cpp` | `COMBAT_DEBUG_VISIBILITY_SNAPSHOT::bCombatObjectHit`, 토글, live combat object의 hit 모양 wire |
| Data | `Data/Valtan/Valtan.gameplay.json` | 2.2의 stage hit 추가·정렬, 피자 STEP_03/04/05/07/11 hit, 침묵 슬롯 event 이동 |
| Data | `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json` | 피자 shout.burst STEP_07 → STEP_05 이동, STEP_04/07 `impact`·STEP_11 `twohand` 추가, GROUND_ROAR impact 0 → 600/1300, BIND_SLOT RECOVERY burst 0 → 733, 19_04 twohand 1033 → 1000 |
| Data | `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json`, `Tools/ValtanPipeline/build_valtan_pattern_sound_cues.py`, `Tools/ValtanPipeline/test_valtan_pattern_sound_cue_contract.py` | builder 행, semantic impact 행, 잡기 종료 6 scope와 STAGGER FINAL_ATTACK Sound 등록. runtime payload가 없는 clip-native Sound는 explicit unavailable 목록으로만 제외 |
| Data / Pipeline | `Data/Valtan/Valtan.cliptemplates.json`(신규), `Tools/ValtanPipeline/validate_valtan_clip_template_parity.py`(신규), `Tools/Build/BuildDomains.json` | 10절 clip 템플릿 정본과 G11 parity validator |
| Data | `Data/Valtan/Valtan.combatobjects.json`, `Data/Effects/Authored/effect.valtan.{ground-roar,struggling,six-pizza}.rock.active.effect.json` | 돌 4종 `presentationEvents` → `hits`, `coverRadiusM`, 피자 수명/폭발 age와 active terminal wave를 archetype hit age에 정렬. part-break는 ground-roar active 문서를 재사용 |
| Data | `Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json` | 돌 pulse 키를 hitId로 |
| Pipeline | `Tools/ValtanPipeline/valtan_tuning_pipeline.py`, `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | `coverRadiusM` 필드 검증·투영 |
| Server | `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp` | `fCoverRadiusM` 파서 |
| Server | `Server/Private/GameRoom.cpp` | 돌 damaging volley의 navigation projection, stage hit cover 원에 live 돌 포함 |
| Tests | `Server/Private/ServerGameplayContractTests.cpp`, `Tools/ValtanPipeline/test_valtan_rock_pillar_group_contract.py`, `test_valtan_combat_object_hit_effect_presentation_contract.py`, `test_valtan_model_view_composition.py`, `test_valtan_status_pattern_contract.py`, `test_valtan_pattern_sound_cue_contract.py` | hit/cover/terminal wave/same-hitId Sound/즉시 침묵/잡기 6 scope/STAGGER 계약 |

## 4. G별 구현 범위

### G00 combat object hit Debug wire

09-04 실측: working tree에는 이미 `bCombatObjectHit`와 `Draw_CombatObjectHitAreaDebug`(ClientReplication.cpp 1335행)가 있고 HEAD에는 없다. 아래는 그 구현이 지켜야 할 계약이며 신규 작성 범위가 아니다.

- `COMBAT_DEBUG_VISIBILITY_SNAPSHOT`에 `bCombatObjectHit` 추가, F1 `Live Combat Geometry`에 체크박스 `Combat Object Hit`.
- `CClientReplication`이 `CCombatObjectProjectionRuntime` record마다 `Data/Encounters/Valtan/ValtanCombatObjects.json` 참조(이미 Client가 `CEncounterPatternReference`로 읽는 Product)에서 archetype의 `hits[]`를 찾아, 마지막 snapshot pose에 RING/CIRCLE/CONE/BOX wire를 그린다. 색은 pattern hit wire와 구분한다.
- 판정에는 참여하지 않는다. `Draw_PatternHitAreaDebug`와 같은 규칙(300ms pulse 창, 또는 `TIMED atMs` 전후)을 따른다.
- 검증: 도넛 패턴 재생 시 RING 8~16m wire가 impact ring 위에 보이는지 사용자가 확인.

### G01 도넛 collider 크기 맞추기

현재 데이터: `combatobject.valtan.fist-in-out.donut` RING inner 8 / outer 16, 1600ms. 이펙트 `effect.valtan.carrier-v1.attack.fist-in-out.inner.clip-01`의 impact ring particle `spawnShape.radius 8`, BossCatalog `worldScale 1.5`. 큰 도넛은 RING 16/24, impact `radius 13.333`, `worldScale 1.5`.

규칙: 이펙트 impact ring의 world 반지름 = `spawnShape.radius × worldScale`(도넛 12m, 큰 도넛 20m)이 RING의 중심선이 되게 `inner/outer`를 잡는다. 지금 값은 도넛 (8+16)/2 = 12 ✓, 큰 도넛 (16+24)/2 = 20 ✓으로 이미 중심선이 맞고, 폭 8m는 텔레그래프(`donut.telegraph.outer.red` scale 22 vs `inner.grow` 17)의 시각 폭과 맞는지 G00 wire로 확인한 뒤 조정한다. `atMs 1600`은 `donut.impact.wave.black`의 `startDelaySeconds 1.6`과 같다.

수정 절차: `Valtan.combatobjects.json` 값 변경 → Workbench `Save + Validate + Publish` 또는 `Project-ValtanPatternMaster.ps1 -Mode PublishV2` → `Publish-GameplayBalance.ps1` → Server 재시작.

### G02 twohand·포효 계열 stage hit

모두 `Valtan.gameplay.json` stage `hit`만 편집한다. 코드 변경 없음.

| 패턴 / stage | shape | schedule | damage | push / knockdown |
|---|---|---|---|---|
| `THREE` STEP_01 | `CONE 75° 15m` | `EXPLICIT_OFFSETS [1617]` | `damage.valtan.ground-wave-smash`(300%) | 0.4m/97ms, kd 2000 |
| `THREE` STEP_02 | 동일 | `[963]` | 동일 | 동일 |
| `THREE` STEP_03 | 동일(기존) | `[500, 1350]` → `[500, 1300]`. 기존 첫 pulse 500은 유지하고 두 번째만 twohand 1300에 정렬 | 동일 | 동일 |
| `SEQUENCE_TWOHAND` STEP_02 | `CONE 75° 15m` | `[1000]` (19_04 원본 HIT 1000·native Shot5 1000, V2 1033 → 1000) | 동일 | 동일 |
| `ROAR_CHARGE` STEP_03 | `CIRCLE 12m` | `[733]` (shout.burst) | `damage.valtan.ledge-roar`(400%) | 2.0m/242ms, kd 2000 |
| `ROAR_CHARGE` STEP_06 | `CONE 90° 12m` | `[200]` (`CHARGE` 패턴과 동일) | `damage.valtan.swing`(220%) | 2.0m/150ms, kd 1200 |
| `SEQUENCE_RUSH` STEP_01~03 | `BOX 6m × 2.5m` | `[0,100,…,600]` (`TRASH` rush와 동일, `playerResponse` 없음) | `damage.valtan.dash-charge`(400%) | 2.0m/150ms, kd 1000 |
| `SEQUENCE_WHIRLWIND` STEP_02, STEP_03 | `CIRCLE 10m` | STEP_02 `[0, 210, 420]`, STEP_03 `[0, 350, 700, 1050]` | `damage.valtan.jump-spin`(300%) | 3.0m/242ms, kd 2000 |

`TRIPLE_COUNTER`는 FAIL_1~3 `CIRCLE 12m [900]`이 twohand 900·Sound Shot1 900과 이미 같다. 추가 없음.
`SEQUENCE_RUSH`의 stage 이동(`motion`)이 없으면 BOX가 제자리에서만 판정되므로, 실제 돌진 motion 유무를 먼저 확인하고 없으면 `TRASH` STEP_08과 같은 stage motion을 함께 저작한다.

THREE STEP_03의 500ms는 공통 `mesh_att_battle_2_03` 템플릿을 바꾸지 않는다. 이 occurrence에만
`boss.valtan.impact`와 `G_Voltan2_Attack02_Shot2`를 500ms에 추가하고,
`Valtan.cliptemplates.json` occurrence waiver에 `EXTRA_HIT`, `extraHitOffsetsMs: [500]`를 정확히 적는다.
1300ms는 공통 B3 twohand/hit/Sound template이 계속 소유한다.

### G03 십자돌(`VALTAN_CROSS`)

돌 particle이 `velocityPerSecond 20`으로 0.5초 이동 → 팔 길이 10m, 팔 두께 scale 0.4×0.7 → 반폭 0.6m. cue는 STEP_01 1617ms root 앵커.

```json
"hit": {
  "shape": { "kind": "CROSS", "lengthM": 10.0, "halfWidthM": 0.75 },
  "activation": { "kind": "ACTIVE_WINDOW", "startMs": 1617, "lifetimeMs": 500, "perTargetPolicy": "ONCE" },
  "serverDamageProfileId": "damage.valtan.earthquake-smash",
  "pushRangeM": 2.0, "pushMs": 150, "knockdown": true, "downMs": 1200
}
```

`activation`의 정확한 필드명은 `valtan_tuning_pipeline.py` 4602행의 `("kind","startMs","lifetimeMs","perTargetPolicy")`를 따른다. 십자는 자라지 않고 1617~2117ms 동안 10m 십자로 고정 판정된다. 돌이 밖으로 퍼지는 것과 정확히 맞추려면 MISSILE combat object 4개(CONTACT)가 필요하지만 `GameplayCatalog.cpp` 4815행 join이 boss-relative missile의 direction을 `RADIAL_INWARD`/`NEXT_RADIAL_SLOT`로 제한하므로 이번 범위에서는 stage `CROSS`를 쓴다.

### G04 돌 폭발 데미지

`Valtan.combatobjects.json` 돌 4종(`ground-roar.rock`, `six-pizza.rock-pillar`, `struggling.rock-pillar`, `part-break.rock`):

```json
"hits": [
  {
    "hitId": "hit.valtan.ground-roar.rock.explode",
    "trigger": { "kind": "TIMED", "atMs": 5000 },
    "repeat": { "count": 1, "intervalMs": 0 },
    "shape": { "kind": "CIRCLE", "outerRadiusM": 3.0 },
    "serverDamageProfileId": "damage.valtan.stomp",
    "pushRangeM": 1.5, "pushMs": 150, "knockdown": true, "downMs": 1000
  }
],
"presentationEvents": []
```

- `presentationEvents`를 비우고 `hitId`가 pulse ID가 된다. `Valtan.combatobjectsoundcues.json`의 돌 4행 키를 `pulse.*.explode` → `hit.*.explode`로 바꾼다. Client `hitEffectAssetId`(패턴별 explode 문서)는 그대로 HIT_PULSE에 재생된다.
- 반지름 3.0m는 explode 문서 debris 초기 속도 5.5m/s × 수명 0.65~1.1s를 근거로 한 시작값이며 G00 wire로 조정한다.
- navigation: `GameRoom.cpp` volley 분기에서 `Hits`가 있는 돌 archetype은 실패 대신 `m_ServerNavigation.Project_PointOnSameLevel`로 가장 가까운 walkable 지점(최대 2m)에 투영해 스폰한다. 투영 실패 시에만 wave를 건너뛴다. `damagingPillarSetsRejectedAtomically` 테스트를 "투영 후 4개 스폰" 테스트로 바꾼다.
- 폭발 Sound에는 별도 5초 timer를 두지 않는다. Server가 object-local `hit.trigger.atMs`에 `HIT_PULSE(hitId)`를 보내면
  Client가 같은 `hitId`의 `G_Voltan2_Attack09_ProjExp1`을 즉시 재생한다. 따라서 damage와 Sound는 한 pulse가 정본이고,
  active effect의 primary `donut.impact.wave.black` 시작 시각을 그 hit age에 맞춘다.

| combat object | `hit.trigger.atMs` | `lifetimeMs` | active telegraph | primary wave / Sound pulse | secondary wave |
|---|---:|---:|---:|---:|---:|
| ground-roar rock | 5000 | 6200 | 4.0s | 5.0s | 5.2s |
| struggling rock | 5000 | 6200 | 4.0s | 5.0s | 5.2s |
| part-break rock | 5000 | 6200 | 4.0s | 5.0s | 5.2s |
| six-pizza rock | 19500 | 20700 | 18.5s | 19.5s | 19.7s |

part-break는 ground-roar active 문서를 공유한다. 이 수치는 "모든 돌은 5초"라는 전역 하드코딩이 아니라 각 archetype의
object-local hit age에서 파생되는 authoring 값이다. focused contract가 `primary wave startDelaySeconds * 1000 == atMs`,
combat-object Sound cue의 `hitId` 집합과 object `hits[].hitId` 집합의 exact match를 검사해 이후 drift를 막는다.

### G05 피자 패턴: 중앙 착지 → 쿵 → 사자후 → 도약 착지 → 모아치기

사용자 결정(09-04): 착지는 부채꼴이 아니라 원 전체 데미지이며 돌 뒤에서 막는다. 사자후는 땅구르기의 사자후
이펙트 group(`boss.valtan.shout.burst`)과 사자후 사운드를 그대로 쓰고, 도약 착지와 마지막 모아치기에도 collider·
사운드·데미지를 넣는다.

(a) 실측 순서. Server 위치는 `serverMotion LEAP_TO_ANCHOR`가 STEP_01 0~800 중앙 이동, 800~1100 상승, STEP_02 정점,
STEP_03 0~267 하강 한 번만 움직이고 그 뒤 모든 stage는 중앙 anchor에 고정된다(`ValtanBrain.cpp` 155~185·1765~1785행,
validator는 pattern당 `travelStageId` 하나, takeoff는 첫 stage만 허용). 따라서 사용자가 보는 "사자후 뒤 점프·착지"는
애니메이션(12_05 3594ms 도약 → STEP_06 8초 hold → 12_07 착지)이며 Server 두 번째 도약이 아니다. 아래 표의 원본
notify는 `Data/Animation/Reference/Valtan/Valtan.animevents`/`.animnotify`의 clip-local ms다.

| 단계 | stage(누적 ms) | clip | 원본 HIT / 효과 notify | 현재 이펙트·사운드 | 저작 |
|---|---|---|---|---|---|
| 중앙 이동·도약 | STEP_01(0) 1200 | 12_01 | HIT 0~200은 원본 잡기 스킬 420612의 잡기 판정, 도약 효과 800 | Sound Cast2 100·CastVox1 600·Shot1 700, 돌 volley ENTER | hit 없음(allowlist `takeoff`) |
| 공중 | STEP_02(1200) 1000 | 12_02 | HIT 0 | 없음 | 없음 |
| 중앙 착지 | STEP_03(2200) 1200, travel 0~267 | 12_03 | HIT 200~430, ZoomBlur 211 | Sound ShotVox1 1·Shot1 100 | 10절 D 착지 템플릿: `CIRCLE 8m [267]` `damage.valtan.jump-spin` 3.0m/242 kd 2000 (같은 clip의 `TERRAIN_DESTRUCTION_3/9` IMPACT와 동일 값), V2 `boss.valtan.impact` CLIP 267 |
| 쿵 찍기 | STEP_04(3400) 2800 | 12_04 | HIT 900~1100(7), 2000~2100(2); 충전 광 1098~2497, Trail 2200, 충격 광·SHAKE 2413 | Sound Shot3 1·CastVox2 500·Cast1 1100·ShotVox2 1400·Shot4 2100; composite 착지 layer 5.69 s(=2290)·ring 5.8 s(=2400) | 10절 C 발구르기 템플릿: `CIRCLE 8m [2100]` `damage.valtan.stomp` 1.5m/150 kd 1000 (원본 창 2000~2100 안, native Shot4 2100과 정확히 일치), V2 `boss.valtan.impact` CLIP 2100. composite 착지 layer/ring은 5.69/5.8 s → 5.5 s로 당긴다. 900 HIT는 충전 시작이라 넣지 않는다 |
| 사자후 | STEP_05(6200) 5000 | 12_05 | HIT 400~601, 광 1188, 도약 효과·TrailGhost·SHAKE 3594 | Sound Cast2 100·ShotVox3 1300·Shot1 3500 | 10절 A 사자후 템플릿을 clip-local T에 건다. 데이터상 포효 vocal은 ShotVox3 1300이므로 T=1300 시작값: V2 `boss.valtan.shout.burst` CLIP 1300 `b_effectroot`, `CIRCLE 12m [1300]` `damage.valtan.ledge-roar` 2.0m/242 kd 2000, semantic Sound `Attack18_Shot7` 1300·`Attack18_ShotVox4` 1300. 12_05의 실제 포효 프레임은 사용자가 Animation Clip Tool로 확인해 T를 확정한다. 3594 도약은 이펙트 없음(원본 Atk_07_01은 미이식) |
| 공중 hold | STEP_06(11200) 8000 | 12_06 loop | 원본은 착지 clip(HIT 0/0~200/200~400) | composite 부채꼴 11.0 s(7 s) | hit 없음. 12_06을 8초 loop하므로 공중처럼 보이지 않으면 hold clip 교체는 사용자 결정 |
| 도약 착지 | STEP_07(19200) 1200 | 12_07 | HIT 250~450(6), 800~1000(2); Trail 0, Atk_08_01/02 266, SHAKE 268 | Sound Cast2 1·Shot5 250·ShotVox4 250; V2 shout.burst 733(원본 notify 없음, STEP_05로 이동); composite ring/red overlay/sky-wave/center 19.5 s(=300) | 원 전체: `CIRCLE 25m [250]` `damage.valtan.super-smash`(700%) 3.0m/242 kd 2000, 돌 엄폐 (b). V2 `boss.valtan.impact` CLIP 250. native Shot5 250과 정확히 일치. 800 HIT는 넣지 않는다 |
| 후속 | STEP_08(20400) 1000 / STEP_09(21400) 1200 | 12_08 / 12_09 | 12_09 HIT 0~200, 207~408 | 12_09 Sound Cast3 1·Cast4 900 | 기본 없음. 12_09에 휘두르기 hit을 원하면 `CONE 90° 12m [0, 207]` `damage.valtan.swing` + semantic impact Sound가 필요하며 사용자 결정 |
| 붉은 검기 | STEP_10(22600) 6000 | 12_10 loop | HIT 없음(원본은 투사체), 검기 효과 0~3000, SHAKE 5 s | Sound Cast3 1·ShotVox1 500; composite 부채꼴 23 s(=400) | telegraph. hit 없음(allowlist `projectile-source`). 검기 데미지를 원하면 `damage.valtan.red-blade-wave`(350%) 전방 BOX 후보 |
| 모아치기 | STEP_11(28600) 1300 | 12_11 | HIT 150~350, 700~900, 1150~1250; Trail 56, SHAKE 162 | Sound Attack29_Shot1 1·Attack05_ShotVox2 1; composite whirlwind mesh 28.5 s(=-100) | 검격: `CONE 90° 12m` `EXPLICIT_OFFSETS [150, 700, 1150]` `damage.valtan.ground-wave-smash`(300%) 0.4m/97 kd 2000, V2 `boss.valtan.twohand` CLIP 150, semantic Sound `Attack18_Shot5` 150·700·1150. whirlwind mesh(7×1.5=10.5m 반지름) 전방위가 정본이면 `CIRCLE 10.5m`로 바꾼다 |

STEP_07 반지름 25m는 "아레나 전체"다. composite ring decal은 size 30 × cue worldScale 1.5 = 지름 45m라 원 전체와
같은 뜻이고, 돌은 중앙에서 10m에 있으므로 25m 원 안에서 돌 엄폐만이 생존 수단이다. G00 wire로 아레나 경계와
맞춰 조정한다.

(b) 돌 엄폐. stage hit은 이미 `IsShieldedByCover`를 거치므로 cover 원 목록에 살아 있는 돌만 더하면 된다.
`GameRoom.cpp` 13238행 cover 수집에 `m_CombatObjectRuntime.Get_LiveObjects()` 중 `coverRadiusM > 0`인 archetype
(돌 4종, 값 1.5m)이고 아직 첫 hit `atMs` 전인 객체의 원을 추가한다. `Valtan.combatobjects.json`에
`coverRadiusM` 필드, pipeline·bootstrap·`GameplayCatalog` 파서에 같은 필드를 추가한다. 플레이어가 중앙에서 자기
위치로 그은 선분이 돌 원과 교차하면 데미지를 받지 않는다(`Segment_IntersectsCircle`). 원 hit와 돌 배치가 모두
아레나 중앙 기준이라 안전 지대가 항상 같다. 최종 배치는 중앙 반경 10m, 시작각 45°의 네 root라
XZ offset은 각각 약 `±7.071m`이고, 폭발 collider 반지름 3m와는 별도 값이다. 구현은 STEP_01의
`firstOffsetMs 1000`을 유지했다.
피자 돌은 pattern 1000ms에 생성되어 object age 19500ms(pattern 20500ms)에 폭발하고 `lifetimeMs 20700`에 종료된다.
따라서 STEP_07 착지 hit(pattern 19450ms, object age 18450ms) 때는 살아 있고 아직 폭발 전이라 엄폐에 포함된다.
`test_valtan_rock_pillar_group_contract.py`와 `test_valtan_combat_object_hit_effect_presentation_contract.py`가
spawn offset·hit age·수명·엄폐 가능 창을 함께 고정한다.

이 엄폐 계약은 파괴 전 full arena에서 시작하는 audition/product occurrence를 기준으로 한다. 별도 collapse-state guard는
추가하지 않았고, 현재 navigation에서 4개를 원자적으로 투영할 수 없으면 volley를 commit하지 않는 기존 안전 경계만 유지한다.
따라서 Product pattern scheduler는 아레나 붕괴 뒤 또는 지연 재생으로 이 피자 occurrence를 선택하지 않아야 한다.

(c) 시각 정렬. composite cue 앵커 `arena.center.target-follow`는 부채꼴이 대상 방향으로 도는 표현이라 원 전체 데미지와
어긋난다. 원 전체 데미지가 정본이므로 cue 앵커를 `arena.center`로 바꾸고 부채꼴 particle(11.0 s·23 s)은 연출로만
둔다. STEP_07 300ms의 ring/red overlay/sky-wave는 hit 250과 1.5 tick 차이라 그대로 둔다.

(d) 사운드 규칙. `Valtan.patternsoundcues.json`의 native 행은 `build_valtan_pattern_sound_cues.py`가
`Valtan.animevents` SOUND 행 × generated `Valtan.patternbindings.json` clip × `ValtanEncounter.json`으로 결정적으로
만들고, `test_valtan_pattern_sound_cue_contract.py`가 authored 행 = builder 행(bindingId `cue.sound.<clipOccurrenceId>.NN`)을
요구한다. 그래서 native 행의 `startMs`를 손으로 옮기지 않고 hit을 원본 HIT 창 안의 native 사운드 시각에 맞춘다
(STEP_04 2100, STEP_07 250). clip에 없는 impact 사운드(STEP_05 사자후, STEP_11 검격)는 GROUND_ROAR 4행과 같은
semantic 행으로 추가한다: bindingId는 `cue.sound.valtan.semantic.<pattern>.<stage>.NN`처럼 builder 접두사와 다르게
두고, 같은 행을 테스트의 `PROJECT_AUTHORED_SEMANTIC_CUES`에 등록한다. 창 검사(`startMs < sourceStart + playMs`)는
그대로 적용된다.

### G06 데미지를 넣지 않는 패턴

| 패턴 | 이유 |
|---|---|
| `ENTRANCE_CINEMATIC` | 무적 연출 |
| `GROGGY_FOLLOWUP`, `TRASH_CATCH_FAIL` | 보스 피격·실패 회복 구간 |
| `BIND_SLOT`, `SILENCE_SLOT` | 상태 이상 슬롯. 데미지가 아니라 상태 적용이 필요하며 별도 슬라이스 |
| `GHOST_RESPAWN_AUDITION`, `GHOST_DEATH_AUDITION` | phase 전환 |

## 5. 검증

1. 데이터: `Project-ValtanPatternMaster.ps1 -Mode PublishV2` → `Publish-GameplayBalance.ps1 -Mode Publish` → bootstrap에 새 hit 행 확인.
2. Server: `ServerGameplayContractTests.cpp`에 G02~G05 case 추가 → `Server.exe --contract-test failures : 0`.
3. Python: `test_valtan_rock_pillar_group_contract`, `test_valtan_combat_object_hit_effect_presentation_contract`, `test_valtan_model_view_composition`, `test_valtan_pattern_tree_contract`에서 rock hit age = active primary wave, same-hitId Sound, cover window와 pattern topology PASS.
4. Sound: `build_valtan_pattern_sound_cues.py`를 scratch 출력으로 실행해 GROUND_ROAR `roar.clip-05` 3행을 authored 문서에 옮기고 semantic 행을 `PROJECT_AUTHORED_SEMANTIC_CUES`에 등록 → `test_valtan_pattern_sound_cue_contract` PASS.
5. G11: `validate_valtan_clip_template_parity.py --check` PASS(10절 표의 allowlist 항목만 예외).
6. `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product` 뒤 `-Profile FullDiagnostic` PASS.
7. 사용자: F1 `Combat Object Hit` wire로 도넛·돌 collider가 이펙트와 겹치는지, 피자 STEP_07 착지에서 돌 뒤 데미지가 0인지, 12_05 사자후 T와 3연속 공격이 이펙트·Sound와 같은 순간에 맞는지 확인.

## 7. 규약: 판정·이펙트·사운드의 정본과 정렬 규칙

이펙트를 보고 collider를 만드는 것이 아니라, collider(판정)를 먼저 저작하고 이펙트·사운드가 그 ms에 붙는다.
Server가 판정 권위이고 Client 이펙트는 판정을 읽을 수 없으므로 방향은 이 하나뿐이다. 다만 "공격 이펙트가 있는데
판정이 없는" 상태를 막기 위해 양방향 정렬을 validator로 강제한다.

| 규칙 | 내용 |
|---|---|
| R1 판정 정본 | 데미지·넉다운·잡기는 `Valtan.gameplay.json` stage `hit`, `Valtan.combatobjects.json` `hits`, `DAMAGE/EXECUTE_GRABBED_PLAYERS`만 만든다. 이펙트 문서의 크기·타이밍은 판정 입력이 아니다 |
| R2 시각 정본 | hit ms가 정해지면 V2 binding `clock.startMs`, V1 cue `stageOffsetMs`, particle `startDelaySeconds`를 그 ms에 맞춘다. 반대로 hit을 이펙트에 맞추는 경우는 원본 clip contact가 없는 PROJECT_TUNED 패턴뿐이며, 그때도 hit을 먼저 적고 이펙트를 그 값으로 저장한다 |
| R3 이펙트 역할표 | V2 resource와 V1 cue에 역할을 부여한다. `ATTACK`(twohand, impact, pounding, shout.burst, axe, portal, six.sonic, hand_3~6 잡기 성공), `TELEGRAPH`(shout, magicball, trash.pulse-group, decal 계열, breathe), `STATE`(blur, egg, blackhole). `ATTACK` 이펙트 하나마다 같은 stage에 ±1 tick(33ms) 안의 hit 또는 combat object hit이 있어야 한다 |
| R4 사운드 | hit ms마다 `Valtan.patternsoundcues.json`에 impact 계열 사운드(`*_Shot*`, `*_ProjExp*`) 하나가 ±1 tick 안에 있어야 한다. native 행은 builder 결정값이라 옮기지 않고 hit을 원본 HIT 창 안의 native 사운드에 맞추며, clip에 없는 impact 사운드만 semantic 행으로 추가한다(G05 d). combat object hit은 `combatobjectsoundcues.json`의 `hitId` 키로 묶는다. 시전음(`*_Cast*`, `*_Vox*`)은 hit과 무관하게 둔다 |
| R5 모양 근거 | 원본 clip의 HIT notify가 있으면 그 시각·범위를 `mappingBasis SOURCE_REVIEWED`로 쓰고, 없으면 이펙트 footprint(particle 속도×수명, decal size×worldScale, spawnShape radius×worldScale)에서 `PROJECT_TUNED`로 잡는다 |
| R6 데미지 프로파일 | 새 프로파일을 만들지 않고 `damage.valtan.*` 기존 33개에서 고른다. 사자후는 `ledge-roar`(400%), 찍기는 `ground-wave-smash`(300%), 밟기는 `stomp`(250%), 돌진은 `dash-charge`(400%) |
| R7 검증 | `validate_valtan_hit_presentation_alignment.py`를 `valtan.product` domain에 추가한다. R3·R4 위반과 hit 없는 ATTACK 이펙트를 fail-closed로 잡되, 의도적 예외(전멸 hit, 잡기 슬램)는 allowlist 파일에 stable ID로 둔다 |
| R8 clip 템플릿 | 같은 animation clip 이름을 쓰는 모든 occurrence는 `Data/Valtan/Valtan.cliptemplates.json`의 clip-local 템플릿(hit ms·shape·damage, V2 group·ms, impact 사운드·ms)을 그대로 받는다. stage hit offset = occurrence 시작(같은 stage 앞 clip `playMs` 합) + clip-local ms, V2는 `CLIP_OCCURRENCE` basis로 같은 clip-local ms다. 패턴 고유 이펙트(십자돌)가 있는 occurrence만 shape override를 allowlist에 적는다. 10절이 정본이다 |

### 7.1 사자후(shout.burst) 규칙

사자후 clip `mesh_att_battle_5_01_end`의 원본 notify는 burst 효과 740ms, HIT 900~1100(주)·1600·1800(부)이며 현재 V2 burst 733이 이와 맞는다. 10절 A 템플릿: burst CLIP 733, stage hit `CIRCLE 12m` clip-local 900, `damage.valtan.ledge-roar`, push 2.0m/242ms, 넉다운 2000ms, native Sound Cast1 1·Shot7 100·ShotVox4 100. `boss.valtan.shout`(0ms, 시전)는 TELEGRAPH라 hit을 붙이지 않는다. 피자처럼 사자후 clip이 아닌 clip에서 포효하는 stage는 같은 template을 clip-local T에 건다.

| 패턴 / stage | shout.burst ms | 현재 hit | 현재 사운드 |
|---|---|---|---|
| `GROUND_ROAR` STEP_01 `roar.clip-05`(1800 시작) | 733 | 단일 hit-track `CIRCLE 12m [600, 1300, 2700]` 중 2700(1800+900) | 1300 Shot1은 11_01 발구르기 소리로 유지하고 builder 행 Cast1 1·Shot7 100·ShotVox4 100을 clip-05에 추가 |
| `STRUGGLING` STEP_10 | 733 | 없음 → `[900]` | Cast1 1·Shot7 100·ShotVox4 100 native, hit 900은 원본 HIT 창 |
| `ROAR_CHARGE` STEP_03 | 733 | 없음 → `[900]` | 동일 |
| `SIX_PIZZA_106` STEP_07 → STEP_05 | 733 → STEP_05 CLIP 1300 | STEP_05 `[1300]` | STEP_07은 착지(G05), 사자후 semantic Sound는 STEP_05에 추가 |
| `TERRAIN_DESTRUCTION` STEP_11 | 733 | 없음 → `[900]` | 동일 |
| `BIND_SLOT` STEP_01 `clip.05`(4100 시작, playMs 900) / RECOVERY | 733 / 0 → 733 | STEP_01은 hit 5000이 stage 끝이라 allowlist `truncated` / RECOVERY `[900]` | RECOVERY에 builder 행 3개 추가 |

## 8. 전수 감사표

hit = Server 판정, FX = ATTACK 역할 이펙트, SND = hit ms의 impact 사운드. ○ 있음·정렬, △ 있으나 ms 불일치, × 없음.

| 패턴 | 모드 | hit | FX | SND | 필요한 작업 |
|---|---|---|---|---|---|
| ENTRANCE_CINEMATIC | 연출 | - | - | - | 없음 |
| WHIRLWIND | NORMAL | ○ 0/350/700/1050 | ○ cue 0 | △ 200뿐 | 350/700/1050 impact 사운드 |
| DASH_CHARGE | NORMAL | ○ BOX 0 | △ cue 2450 | △ 2450/2800/3200 | cue·사운드 stage 시계 재확인 |
| PART_BREAK | Audition | × 돌 | ○ 돌 | ○ hitId 예정 | G04 |
| FOUR_SLASH | NORMAL | ○ 1790/2560/3330, 600 | ○ cue | △ 1700/2200/3000, 500 | 사운드를 hit ms로 |
| FIST_IN_OUT / LARGE | NORMAL / Audition | ○ 객체 1600 | ○ | ○ hitId | G01 크기 확인 |
| HIGH_JUMP | NORMAL | ○ 도끼 1200, LAND 900 | △ 도끼 V2 오프셋 | △ LAND 900 없음 | 도끼 V2 오프셋 수정, LAND impact 사운드 |
| FLOOR_WIPE_130 | 130 mechanic | ○ | ○ | ○ | 없음 |
| ARENA_BREAK_109 | 109 mechanic | ○ IMPACT 0 | ○ | △ 다음 stage 1 | IMPACT 0 사운드 |
| TERRAIN_DESTRUCTION_3/9 | 84/30 mechanic | ○ IMPACT 0 | ○ | △ LANDING 1/100 | IMPACT 0 사운드 |
| GHOST_FINALE, WARP | Audition | ○ BOX 연속 | ○ portal | ○ 1/390 | 없음 |
| GHOST_PORTAL_ONCE | Audition | ○ 접촉 | ○ | ○ hitId | 없음 |
| STAGGER_SLOT | Audition | ○ 전멸 2900 | ○ `boss.valtan.six.sonic` 2900 | ○ `Attack25_Shot2` 2900 | FINAL_ATTACK 전멸 hit·V2·semantic Sound 정렬 완료 |
| BIND_SLOT | Audition | × | ○ shout.burst 733 / RECOVERY 0 | ○ clip.05 100 / RECOVERY × | 7.1 사자후 hit(RECOVERY 900), RECOVERY burst 733·builder Sound, 속박 상태는 별도 |
| SILENCE_SLOT | Audition | × | × | ○ | G10 시전 즉시 침묵. 데미지는 넣지 않음 |
| TRIPLE_COUNTER | NORMAL | ○ 900×3 | ○ twohand 900 | ○ 900 | 없음(기준 사례) |
| SIX_PIZZA_106 | Audition | × | △ shout.burst가 착지 stage에 있음 | ○ 2100/250 native, × 사자후·검격 | G05 STEP_03/04/05/07/11 hit, burst STEP_05 이동, impact·twohand 추가, semantic Sound, 돌 G04·수명 |
| ATTACK_WHIRLWIND | Audition | ○ 1800, 0/210/420 | ○ cue | △ 1000, 200 | 사운드 정렬 |
| CHARGE, CHARGE_2 | Audition | ○ 200, 250/900 | ○ cue | × | impact 사운드 |
| SEQUENCE_FOUR | Audition | △ 1790/2560/3330 | △ impact 1233/2233/3233/4200 | △ | hit을 impact 4회 ms로 재정렬 |
| ROAR_CHARGE | Audition | × | ○ shout.burst 733 | △ | G02, 7.1 |
| SEQUENCE_RUSH | Audition | × | × | ○ 돌진음 | G02 BOX + stage motion 확인 |
| THREE | Audition | ○ 1617 / 963 / 500·1300 | ○ twohand 1617/963/1300 + impact 500 | ○ 각 hit semantic impact | STEP_03 기존 500 pulse 보존, 공통 B3 외 exact `EXTRA_HIT` waiver |
| TERRAIN_DESTRUCTION | Audition | △ STEP_10 CROSS 400 | ○ sky-axe×4, shout.burst | × STEP_10, △ STEP_11 | CROSS 400 사운드, 7.1 사자후 |
| SEQUENCE_TWOHAND | Audition | × | ○ twohand 1033 | ○ 1000 | G02 |
| SEQUENCE_WHIRLWIND | Audition | × | × | △ | G02 + 이펙트·사운드 |
| TRASH, TRASH_CATCH_IF, TRASH_CATCH_SUCCESS | Audition | ○ 잡기 BOX, 슬램·처형 action | ○ hand_3~6 | ○ CATCH_SLAM / EXECUTE_TAIL 6 scope | 각 sliced clip `sourceStartMs 1500`과 같은 Sound `startMs 1500` = stage ENTER |
| TRASH_CATCH_FAIL | Audition | - | ○ | ○ | 없음 |
| CATCH_BREATH | Audition | ○ CONE 250 잡기 | ○ cue | △ 1 | 250 잡기음 |
| COUNTER | Audition | ○ 900 | ○ cue | ○ 900 | 없음 |
| CROSS | Audition | × | ○ cue 1617 | △ 1400 | G03, 사운드 1617 |
| STRUGGLING | Audition | × 전부 | ○ impact×4, pounding×2, twohand, shout.burst | △ | G07 |
| GROUND_ROAR | Audition | ○ `CIRCLE 12m [600,1300,2700]` | ○ impact 600/1300 + shout.burst clip-05 733 | ○ 발구르기 + 사자후 | 단일 hit-track 제약으로 세 contact를 `ledge-roar` response로 합치고 exact shape/response waiver |
| GROGGY_FOLLOWUP, GHOST_RESPAWN/DEATH | 전환 | - | - | - | 없음 |

## 9. 추가 G

### G07 발악(`STRUGGLING`)·땅구르기(`GROUND_ROAR`) 슬램 hit

| stage | 이펙트 | hit |
|---|---|---|
| `GROUND_ROAR` STEP_01 | impact CLIP 600, 1300(11_01 원본 HIT 600·1300) + shout.burst clip-05 733 | 확정: 단일 stage hit-track 제약 때문에 `CIRCLE 12m` `EXPLICIT_OFFSETS [600, 1300, 2700]`, `damage.valtan.ledge-roar`, 2.0m/242ms, kd 2000으로 합친다. clip template occurrence에 `HIT_SHAPE`·`HIT_RESPONSE` exact waiver와 사유를 둔다 |
| `STRUGGLING` STEP_04 | impact 1233/2233/3233/4200 | `CIRCLE 8m` `EXPLICIT_OFFSETS [1233, 2233, 3233, 4200]` `damage.valtan.stomp` kd 1000 |
| `STRUGGLING` STEP_06 | pounding 200/400 | `CONE 90° 10m` `[200, 400]` `damage.valtan.down-smash`(350%) kd 1200 |
| `STRUGGLING` STEP_07 | twohand 1033 → 1000 | `CONE 75° 15m` `[1000]` `damage.valtan.ground-wave-smash` kd 2000 (19_04 원본 HIT 1000, native Shot5 1000) |
| `STRUGGLING` STEP_10 | shout.burst 733 | 7.1 사자후 |
| `SEQUENCE_FOUR` STEP_01 | impact 1233/2233/3233/4200 | 기존 CROSS 3회 → `[1233, 2233, 3233, 4200]` 4회로 재정렬 |

한 stage에는 hit 하나(shape 하나)만 저작할 수 있다. GROUND_ROAR는 기존 stage topology를 바꾸지 않기 위해 같은 CIRCLE로 합쳤고,
그 결과 첫 두 발구르기도 12m `ledge-roar` response를 쓴다. 이 의도적 절충은 validator의 exact occurrence waiver에 고정한다.

### G08 사운드·종료 공격 정렬

- 8절의 기존 stage hit마다 ±1 tick 안에 impact Sound가 없으면 semantic 행을 추가한다. native 추출 행은 builder 결과라
  이동하지 않고, semantic 행만 `PROJECT_AUTHORED_SEMANTIC_CUES`가 소유한다.
- `VALTAN_TRASH`, `VALTAN_TRASH_CATCH_IF`, `VALTAN_TRASH_CATCH_SUCCESS`의 `CATCH_SLAM` /
  `EXECUTE_TAIL` 여섯 scope에 `G_Voltan2_Attack13_Shot1`을 추가한다. 각 clip은 원본 1500ms부터 잘라 재생하므로
  cue `startMs 1500 == sourceStartMs 1500`이 stage wall의 ENTER 0ms와 같다. 각각
  `DAMAGE_GRABBED_PLAYERS` / `EXECUTE_GRABBED_PLAYERS` ENTER action과 같은 순간이다.
- `VALTAN_STAGGER_SLOT` `FINAL_ATTACK`은 기존 Server `CIRCLE 100m` 전멸 hit 2900ms에 독립 V2 group
  `boss.valtan.six.sonic`을 `b_effectroot`, snapshot, CLIP 2900ms로 바인딩하고 semantic
  `G_Voltan2_Attack25_Shot2`를 2900ms에 둔다. 이 Sound는 같은 stagger wipe 메커닉에서 이미 사용하는 runtime 가용 event다.
- `mesh_att_battle_17_end`의 clip-native Attack15/16 계열 5 event(원본 6행)는 Character Sound catalog와 Resources payload가
  없어 composition publish를 깨므로 builder의 explicit unavailable 목록에서만 건너뛴다. 이 제외는 semantic wipe Sound를
  생략하는 fallback이 아니다.
- combat object Sound는 별도 stage ms를 갖지 않고 `hitId` exact join으로 `HIT_PULSE`와 동시에 재생한다.

### G09 정렬 validator

`Tools/ValtanPipeline/validate_valtan_hit_presentation_alignment.py`와 `Data/Effects/V2/EffectRoles.json`(resource → ATTACK/TELEGRAPH/STATE), `Data/Valtan/Valtan.hitalignment-allowlist.json`(예외)을 추가해 R3·R4를 검사한다. `BuildDomains.json` `valtan.product` tools에 등록하고 Validate 모드에서 실행한다. 이 validator는 2-a parity 계열로, 튜닝값이 아니라 "hit이 있으면 이펙트·사운드가 같은 tick에 있다"는 구조만 본다.

### G10 침묵 슬롯 시전 즉시 적용

09-04 구현값: `VALTAN_SILENCE_SLOT` STEP_01(2633ms)이
`event.valtan.silence-slot.step-01.enter`(`ENTER`, `SET_PLAYER_SILENCE`, `durationMs 7633`)를 갖고 `SILENCE_APPLY`(100ms)의
events는 비어 있다(HEAD는 반대: STEP_01 비어 있고 `SILENCE_APPLY` ENTER 5000).

| 층 | 현재 값 | 상태 |
|---|---|---|
| 정본 gameplay | STEP_01 ENTER 7633 | 완료(uncommitted) |
| 투영 `Data/Encounters/Valtan/ValtanEncounter.json` | STEP_01 `SET_PLAYER_SILENCE 7633`, `SILENCE_APPLY` actions 없음 | 투영 완료 |
| Server `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` `PATTERNSTAGEACTION` 행 | `valtan.authoring.silence-slot.step-01 … ENTER … 7633` | publish 완료 |
| Server 런타임 | 새 Debug build와 Server 재시작 전 | 적용 대기 |

authoring·projection·bootstrap은 같은 값으로 닫혔고, 실제 실행 반영에는 마지막 Debug build 뒤 Server 재시작이 필요하다.

(a) 로직 변경은 필요 없다. 이유는 코드에 있다.

- Server는 pattern 시작과 stage 전환 때 `Commit_BossPatternPlayerStageActions`(GameRoom.cpp 10579행)를 이전 stage `EXIT`,
  다음 stage `ENTER` 순으로 호출한다(9225행, 9624~9629행). 첫 stage의 ENTER도 같은 경로라 STEP_01 ENTER는 시전 tick에 실행된다.
- 적용부(10756~10783행)는 살아 있는 모든 플레이어에게 `iSilenceEndTick = 현재 tick + durationMs tick`, `iSilenceDurationTicks`를
  stage하고 pending SKILL command를 지운다. 만료는 12173행에서 tick 도달 시 `Clear_SilenceStatus()`다.
- 검증(9860~9870행)은 `targetId player.status.silence`, `ENTER`, `value 1`, `durationMs >= 소유 stage durationMs`,
  `100 <= durationMs <= 120000`이다. STEP_01은 2633 ≤ 7633이라 통과한다. Publisher(`Publish-GameplayBalance.ps1` 2580~2588행)와
  Python validator(`valtan_tuning_pipeline.py` 4813~4826행, 5053~5061행)도 같은 조건이다.
- Client HUD는 snapshot `iSilenceEndTick/iSilenceDurationTicks`(8512행)만 읽고, 스킬 제출은 Server가 2847행·2887행에서 거부한다.
- `test_valtan_status_pattern_contract.py` 199~227행은 이미 새 배치(STEP_01 ENTER 7633, `SILENCE_APPLY` events 없음)를 기대한다.

(b) 남은 절차.

1. Debug Product와 Server contract는 PASS했다. 사용자가 실행한 Client/Server를 종료한 뒤 FullDiagnostic를 다시 실행한다.
2. 새 Server에서 pattern 진입 tick부터 snapshot `iSilenceEndTick/iSilenceDurationTicks`가 설정되는지 사용자가 확인한다.
3. `SILENCE_APPLY` stage는 presentation animation이 없는 100ms hold다. 삭제하려면 gameplay·presentation·sound cue와 위 테스트의
   `[("STEP_01", 2633), ("SILENCE_APPLY", 100)]` oracle을 같은 변경에서 고친다. 유지해도 동작에는 영향이 없다.

### G11 clip 템플릿 정렬 validator

`Data/Valtan/Valtan.cliptemplates.json`(10절 표를 그대로 JSON화: clip → `hits[]{clipMs, shape, damageProfileId, push, knockdown}`,
`effects[]{groupId, clipMs, anchorSlotId}`, `sounds[]{soundEvent, clipMs}`, `allowlist[]{patternId, stageId, clipOccurrenceId, reason}`)과
`Tools/ValtanPipeline/validate_valtan_clip_template_parity.py`를 추가한다. validator는 `Valtan.presentation.json`의 모든
occurrence를 clip 이름으로 묶고, template clip마다 다음을 검사한다.

1. gameplay stage `hit`의 offset 집합이 `occurrence 시작 + clipMs`를 ±1 tick 안에 포함하고 shape·damage가 같다. `activation`
   방식이면 `startMs`를 같은 규칙으로 본다.
2. `BOSS_VALTAN.effectv2bindings.json`에 `resource.id == groupId`, `clock.basis == CLIP_OCCURRENCE`, 같은 `clipOccurrenceId`,
   `startMs == clipMs`인 binding이 있다. occurrence가 stage 시작(0)이면 `STAGE` basis + 같은 ms도 허용한다.
3. `Valtan.patternsoundcues.json`에 같은 `clipOccurrenceId`, `soundEvent`, `startMs`(±1 tick) 행이 있다.
4. `playMs < clipMs`인 잘린 occurrence, shape override, 검토 후 유지하는 추가 pulse는 allowlist에 stable ID와 이유가 있어야
   통과한다. THREE STEP_03만 `EXTRA_HIT` + `extraHitOffsetsMs [500]`를 허용하며, allowlist에 없는 누락·추가·불일치는
   fail-closed다.

`BuildDomains.json` `valtan.product` tools에 `--check`로 등록하고 `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`에서도 실행한다.
G09의 R3·R4 validator가 "ATTACK 이펙트에는 hit·사운드가 있다"를 보고, G11은 "같은 clip은 같은 값을 갖는다"를 본다.

### G12 발탄 왼손 잡기 attachment 위치 진단

현재 잡기 위치는 Server anchor와 Client 손 bone이 서로 다른 좌표를 만들고 Client가 마지막에 손 좌표로 덮어쓰는 구조다.
어느 오프셋이 틀렸는지는 아래 다섯 좌표를 같은 프레임에 같이 찍어야 알 수 있다.

(a) 실측한 현재 계약.

| # | 좌표 | 소유 코드 | 계산 |
|---|---|---|---|
| S1 | Server anchor | `GameRoom.cpp` 11775~11830행 capture, 11960~11985행 매 tick | capture 순간 플레이어가 보스에 대해 서 있던 위치를 boss-local `(localX, localY, localZ, localYaw)`로 저장하고, 매 tick `boss pos + yaw 회전(local)`을 플레이어 위치로 쓴다. 손 bone은 모른다 |
| S2 | snapshot | `GameRoom.cpp` 8486행 | `fAttachmentLocalOffsetX/Y/Z`, `fAttachmentYawOffsetDegrees`, `eAttachmentSlot BOSS_LEFT_HAND`와 S1 위치를 보낸다 |
| C1 | Server fallback | `CCharacter::Update` → `Update_NetworkTransform`(Character.cpp 1021~1100행) | GRABBED guard가 없어 매 프레임 S1을 보간해 transform에 쓴다 |
| C2 | 손 world | `ClientReplication.cpp` 2480~2500행 | `Get_BoneMatrix("bip001-l-hand") × presentationRoot`. presentationRoot = body part local × Valtan transform(Valtan.cpp 3454행). bone combined는 root bone × `bodyModelPreScale`을 포함한다(Bone.cpp 62~76행). 무기 socket(Part_Equipment.cpp 57~80행)과 같은 합성이라 무기가 손에 붙어 보이면 C2 자체는 맞다 |
| C3 | 최종 world | `ClientReplication.cpp` 2503~2560행 | 첫 프레임에 `Build_LocalOffset(player rot, hand rot)`로 회전 basis만 한 번 저장하고, 매 프레임 `local × (hand + hand의 정규화 up축 × gripLocalOffset.upM)`. `upM -0.9`는 `Valtan.gameplay.json` CAPTURE hit의 `gripLocalOffset`(TRASH STEP_08/RETRY_RUSH_02/03, TRASH_CATCH_IF 동일, CATCH_BREATH STEP_02)이 `ValtanEncounter.json`으로 투영된 값을 Valtan.cpp 765행이 읽는다 |

실행 순서는 `CGameInstance::Update_Engine`(GameInstance.cpp 173→180→195→199행)이 Object Update(C1) → Level Update(`m_Replication.Update()` →
`Update_PlayerAttachmentPresentations` = C3) → Late_Update/render라 화면에는 C3가 남는다. 그러나 nameplate, Debug wire, Server
판정은 S1을 쓴다. 따라서 "손 위치와 다르다"는 관찰은 (1) C3가 손바닥에서 벗어남, (2) S1이 손과 멀어서 S1을 쓰는 표시가 어긋남, 두
가지가 섞여 있을 수 있다.

(b) 진단 출력. 코드 추가 범위는 Client Debug뿐이다.

| 파일 | 변경 |
|---|---|
| `Client/Public/ClientReplication.h` | `COMBAT_DEBUG_VISIBILITY_SNAPSHOT::bBossGrabAnchor`(기본 true), `PLAYER_ATTACHMENT_PRESENTATION`에 마지막 프레임 진단값(S1, C2 위치·세 축, grip 점, C3 위치, `bHasLocalOffset` 캡처 시각) 저장 |
| `Client/Private/ClientReplication.cpp` | `Update_PlayerAttachmentPresentations`에서 위 값을 채우고, `Draw_PlayerAttachmentDebug()`가 `CHitAreaWire::Draw`로 S1(분홍 원 0.3m), C2(흰 원 0.3m + 축별 0.5m 지점 원 R/G/B), grip 점(노랑 원), C3(초록 원)을 그린다. capture edge와 이후 1초마다 `CClientSessionDiagnostic::Record_Event("valtan.grab.attach", detail)`로 같은 숫자를 JSONL에 남긴다 |
| `Client/Private/MainApp.cpp` | F1 `Live Combat Geometry`에 `Boss Grab Anchor` 체크박스와 읽기 전용 표: boss pos/yaw, S1, snapshot local offset 4개, C2 pos, C2 right/up/forward, grip 점, C3 pos, `C3-C2`, `C3-S1`, 첫 basis 캡처 tick |

(c) 판독 규칙. 표의 숫자가 다음 중 어디에 해당하는지로 수정 대상을 고른다.

| 관찰 | 원인 | 수정 |
|---|---|---|
| C2가 무기 socket처럼 손에 있는데 C3가 손에서 0.9m 옆·앞으로 빠짐 | bip 손 bone의 up축이 손바닥 법선이 아니다 | `gripLocalOffset`를 `forwardM`/`rightM`으로 옮긴다. 데이터 변경 + PublishV2 + Server publish. 값은 표의 C2 축으로 계산 |
| C2 자체가 눈에 보이는 손과 다르다 | presentationRoot 또는 bone 합성 문제. 무기 socket도 같이 틀려야 한다 | `Try_Get_PresentationRootMatrix`와 `Enable_RootMotionSuppression` 축 확인. Client 코드 |
| C3는 손에 맞는데 nameplate·wire·판정이 멀리 있다 | S1이 capture 상대 위치라 손과 무관 | Server capture에 authored boss-local 손 오프셋을 추가한다: gameplay CAPTURE hit에 `serverAttachmentLocalOffset {forwardM, rightM, upM}`를 두고 `GameRoom.cpp` 11775행에서 delta 대신 이 값을 local로 저장. Publisher·`GameplayCatalog` 파서·`ServerGameplayContractTests` capture case 갱신 |
| C3가 프레임마다 떨린다 | C1이 매 프레임 S1을 다시 쓰고 C3가 덮는 순서는 맞지만 basis가 첫 프레임 값 | `Update_NetworkTransform`에 GRABBED guard 추가(Client) |
| 원격 Client에서만 다르다 | Valtan animation clock이 Client마다 달라 C2가 다르다 | 설계상 허용. 큰 차이면 attachment를 Server-authoritative 손 오프셋(S1 수정)으로 통일 |

(d) 검증. 새 wire는 판정에 참여하지 않는다. `test_valtan_combat_object_hit_effect_presentation_contract.py`류의 source-text oracle이
`Update_PlayerAttachmentPresentations` 본문을 고정하는지 먼저 grep하고, JSONL 한 줄에 nickname·packet payload를 쓰지 않는
`CClientSessionDiagnostic` 규칙을 지킨다. 화면 판독은 사용자가 TRASH 잡기와 CATCH_BREATH 잡기 두 경로에서 수행한다.

## 10. clip 이름 템플릿 정본

값은 clip-local ms다. 원본 근거는 `Data/Animation/Reference/Valtan/Valtan.animevents`(HIT/SOUND)와 `.animnotify`(효과)이며
`mappingBasis SOURCE_REVIEWED`다. stage hit offset은 occurrence 시작을 더한 값이다.

### 10.1 템플릿

| 템플릿 | clip | 원본 notify | hit(clip-local) | V2 | 사운드 |
|---|---|---|---|---|---|
| A 사자후 | `mesh_att_battle_5_01_end` | 효과 Wave/Atk_07_02 740, HIT 900(주)·1600·1800 | `CIRCLE 12m [900]` `damage.valtan.ledge-roar` 2.0m/242 kd 2000 | `boss.valtan.shout.burst` 733 `b_effectroot` | native Cast1 1·Shot7 100·ShotVox4 100 |
| A′ 사자후 준비 발구름 | `mesh_att_battle_5_01_start` | HIT 1200~1300, SHAKE 1290 | `CIRCLE 8m [1200]` `damage.valtan.stomp` (선택, 사용자 결정) | `boss.valtan.impact` 1200 | native Cast1 1200 |
| B1 내려찍기 1타 | `mesh_att_battle_2_01` | HIT 1600~1800, 효과 1617 | `CONE 75° 15m [1617]` `damage.valtan.ground-wave-smash` 0.4m/97 kd 2000 | `boss.valtan.twohand` 1617 | native Shot1 1400(6 tick 전) → semantic `Attack02_Shot1` 1617 |
| B2 내려찍기 2타 | `mesh_att_battle_2_02` | HIT 950~1150, 효과 963 | `CONE 75° 15m [963]` 동일 | `twohand` 963 | native Shot1 650 → semantic 963 |
| B3 내려찍기 3타 | `mesh_att_battle_2_03` | COUNTER 0~1200, HIT 1200~1400·1300~1500, 효과 1191 | `CONE 75° 15m [1300]` 동일 | `twohand` 1300 | native Shot2 900 → semantic 1300 |
| B4 두손 찍기 | `mesh_att_battle_19_04` | HIT 600·1000·1500, 효과 1015 | `CONE 75° 15m [1000]` 동일 | `twohand` 1000 | native Shot5 1000 |
| B5 카운터 마지막 찍기 | `mesh_att_battle_14_03` | COUNTER 100~600, HIT 1000, 효과 1001 | `CIRCLE 12m [900]` `damage.valtan.triple-counter`(기존 Product 기준, 원본 1000과 3 tick) | `twohand` 900 | native Shot1 900 |
| C1 발구르기 | `mesh_att_battle_11_01` | HIT 600~800·1300~1500, 효과 595·1261 | `CIRCLE 8m [600, 1300]` `damage.valtan.stomp` 1.5m/150 kd 1000 | `boss.valtan.impact` 600, 1300 | native Cast1 600·1300 (semantic FootStep/Shot1 유지) |
| C2 쿵 찍기 | `mesh_att_battle_12_04` | HIT 900·2000~2100, 충격 광·SHAKE 2413 | `CIRCLE 8m [2100]` `damage.valtan.stomp` | `impact` 2100 | native Shot4 2100 |
| D1 중앙 착지 | `mesh_att_battle_12_03` | HIT 200~430 | `CIRCLE 8m [267 또는 stage IMPACT 0]` `damage.valtan.jump-spin` 3.0m/242 kd 2000 | `impact` 267 | native Shot1 100(allowlist `pre-landing`) |
| D2 도약 착지 | `mesh_att_battle_12_07` | HIT 250~450(6)·800, 효과 266, SHAKE 268 | `CIRCLE 25m [250]` `damage.valtan.super-smash` 3.0m/242 kd 2000 | `impact` 250 | native Shot5 250 |
| E 모아치기 | `mesh_att_battle_12_11` | HIT 150·700·1150, Trail 56 | `CONE 90° 12m [150, 700, 1150]` `damage.valtan.ground-wave-smash` | `twohand` 150 | semantic `Attack18_Shot5` 150·700·1150 |

B1~B3의 native `Shot` 사운드가 hit보다 200~400ms 앞서는 것은 원본이 "휘두르는 소리"를 먼저 내는 구조다. R4를 hit
기준으로 지키기 위해 semantic impact 행을 hit ms에 추가하고 native 행은 그대로 둔다(둘 다 재생).
공통 B3 템플릿은 1300ms만 소유한다. `VALTAN_THREE` STEP_03의 기존 500ms pulse는 그 occurrence에만 impact/Sound를 붙이고
`EXTRA_HIT` waiver로 보존하므로 같은 clip을 쓰는 다른 occurrence로 전파되지 않는다.

### 10.2 occurrence 전수표

| clip | pattern / stage / occurrence 시작 | 현재 hit | 현재 V2 | 현재 사운드 | 작업 |
|---|---|---|---|---|---|
| 5_01_end | `ROAR_CHARGE` STEP_03 / 0 | × | burst 733 | ○ | hit `[900]` |
| 5_01_end | `TERRAIN_DESTRUCTION` STEP_11 / 0 | × | burst 733 | ○ | hit `[900]` |
| 5_01_end | `STRUGGLING` STEP_10 / 0 | × | burst 733 | ○ | hit `[900]` |
| 5_01_end | `GROUND_ROAR` STEP_01 `roar.clip-05` / 1800 | `CIRCLE 12m [600,1300,2700]` 중 2700 | burst 733 | builder Sound 3행 | 단일 hit-track merge + exact shape/response waiver |
| 5_01_end | `BIND_SLOT` RECOVERY / 0 | × | burst 0 | × | burst 733, hit `[900]`, builder Sound 3행 |
| 5_01_end | `BIND_SLOT` STEP_01 `clip.05` / 4100 (playMs 900) | × | burst 733 | ○ | allowlist `truncated`(hit 5000 = stage 끝) |
| 5_01_start | `BIND_SLOT` STEP_01, `ROAR_CHARGE` STEP_01, `TERRAIN_DESTRUCTION` STEP_09, `STRUGGLING` STEP_08 / 0 | × | × | ○ Cast1 1200 | A′ 채택 시 4곳 동일 적용 |
| 2_01 | `THREE` STEP_01 / 0 | × | twohand STAGE 1617 | Shot1 1400 | hit `[1617]`, semantic 1617 |
| 2_01 | `CROSS` STEP_01 / 0 | × | cross cue 1617(V1) | Shot1 1400 | G03 `CROSS [1617]` = shape override allowlist(`cross-rock`), semantic 1617 |
| 2_02 | `THREE` STEP_02 / 0 | × | twohand 963 | Shot1 650 | hit `[963]`, semantic 963 |
| 2_03 | `THREE` STEP_03 / 0 | `[500, 1300]` | impact 500 + twohand 1300 | Shot2 900 + semantic 500/1300 | 기존 500 유지, 공통 template 밖 exact `EXTRA_HIT` waiver |
| 19_04 | `SEQUENCE_TWOHAND` STEP_02 / 0 | × | twohand 1033 | Shot5 1000 | hit `[1000]`, V2 1000 |
| 19_04 | `STRUGGLING` STEP_07 / 0 | × | twohand 1033 | Shot5 1000 | 동일 |
| 14_03 | `TRIPLE_COUNTER` FAIL_1~3 / 0 | `[900]` | twohand 900 | Shot1 900 | 기준 사례, 변경 없음 |
| 14_03 | `COUNTER` STEP_03 / 0 | `[900]` | × | Shot1 900 | V2 twohand 900 추가 |
| 11_01 | `GROUND_ROAR` STEP_01 `clip-01` / 0 | `CIRCLE 12m [600,1300,2700]` 중 600/1300 | impact 600/1300 | 600/1300 | 사자후와 단일 track으로 합쳐 shape/response waiver |
| 12_03 | `SIX_PIZZA_106` STEP_03 / 0 | × | × | Shot1 100 | G05 |
| 12_03 | `TERRAIN_DESTRUCTION_3/9` IMPACT / 0 | `CIRCLE 8m [0]` jump-spin | × | LANDING 1/100 | impact 0 추가(기존 hit 유지) |
| 12_03 | `ARENA_BREAK_109` IMPACT_HOLD / 200, `TERRAIN_DESTRUCTION` STEP_03, `TRASH` STEP_03 / 0 | × | × | Shot1 100 | 착지 연출만이면 allowlist `landing-visual`, 아니면 D1 적용(사용자 결정) |
| 12_04, 12_07, 12_11 | `SIX_PIZZA_106` STEP_04 / 07 / 11 | × | STEP_07 burst 733 | ○ / ○ / Shot1 1 | G05 |

`mesh_att_battle_12_02`는 `ARENA_BREAK_109` IMPACT(`CIRCLE 12m` `arena-destroy-109` 700%)와 피자 STEP_02 공중 hold가 같은
clip을 다른 뜻으로 쓰므로 템플릿에 넣지 않고 allowlist `mechanic-specific`으로 둔다. 잡기 계열 hand 이펙트와
`DAMAGE/EXECUTE_GRABBED_PLAYERS`도 shape가 없어 이 표 밖이다.
