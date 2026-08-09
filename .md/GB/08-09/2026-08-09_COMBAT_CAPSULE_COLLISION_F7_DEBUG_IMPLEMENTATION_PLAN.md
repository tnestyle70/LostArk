# 2026-08-09 전투 Capsule Collider, Valtan Pattern, Knockback, Arena Spawn과 F7 Debug 구현 계획

## 0. 최종 결정 요약

이번 작업의 전투 판정은 다음 네 계약으로 고정한다.

1. 캐릭터, 일반 monster, Valtan의 피격 범위는 root를 따라가는 수직 `HURT_CAPSULE`이다.
2. reviewed player skill의 공격 범위는 action-relative window 동안만 활성화되는 `HIT_CAPSULE`이다. 기존
   90-skill migration은 수학적 판정 보존을 위해 같은 timeline/runtime 안의 `LEGACY_XZ_RANGE` shape로 먼저 옮긴다.
3. overlap과 damage는 Server 30 Hz fixed tick에서만 확정한다. Client mesh, `CCollider`, PhysX overlap은 damage 권위가 아니다.
4. F7은 `_DEBUG` Client에서 Server가 승인한 combat runtime revision, 같은 authoring data, Server snapshot을
   join해 collider를 그리는 읽기 전용 overlay다.

LoL처럼 단순 capsule을 쓰는 방향은 이 프로젝트에도 맞다. 다만 현재 animation pose, 무기, 머리카락,
standalone effect mesh의 render bounds를 매 frame 읽어 자동 확대하지 않는다. 공통 humanoid 비율은 최초 seed로
사용하되 최종 radius, cylinder half-height, center offset은 class/archetype profile에 명시적으로 저장한다.
맵 placement editor에서 개별 instance를 튜닝하지 않고, 같은 archetype은 모든 맵에서 같은 capsule을 소비한다.

현재 `CCharacter`가 가진 OBB는 static world `collisionBox`와의 일반 이동 판정을 위한 기존 계약이다. 이번에 추가하는
capsule은 전투 query용 hurt volume이다. actor끼리 상시 밀기/body blocking/crowd separation은 여전히 하지 않지만,
적중 반응으로 발생하는 제한된 `FORCED_MOTION`만 Server가 capsule sweep과 navigation projection으로 처리한다.
일반 이동 OBB를 capsule로 바꾸는 작업은 별도 수직 슬라이스다.

진행 중인 4직업 standalone mesh 복원과도 다음처럼 분리한다.

```text
Authored Effect Mesh                    Combat Capsule
Client presentation                    Server authority
EffectAssetId / elementId               skillId / stageIndex / hitEventId
render lifetime / evaluated World       fixed-tick start/end / local capsule
CEffectPlayback                         CPlayerSkillSystem + CCombatCollisionSystem
```

Mesh가 보이는 frame과 hit window를 맞출 수는 있지만, Mesh triangle/AABB 자체가 damage collider가 되지는 않는다.
둘의 대응은 별도 traceability 문서와 audit으로 증명한다. 따라서 이펙트 크기나 장식 layer가 바뀌어도 Server
damage 범위가 몰래 바뀌지 않는다.

이미지, screenshot, 캡처 비교는 조사와 자동 검증에서 사용하지 않는다. runtime 검증은 packet/action tick,
활성 hit ID, overlap target ID, damage event를 구조화된 log와 harness로 확인하고, 최종 화면 관찰은 사용자가
직접 수행한다.

## 1. 완료 범위

### 1.1 Actor hurt capsule

- 여섯 player class가 class profile의 capsule을 가진다.
- Valtan Arena에서 spawn되는 네 monster archetype이 monster profile capsule을 가진다.
- `BOSS_VALTAN`이 boss profile capsule을 가진다.
- Server spawn 때 profile을 runtime actor에 복사하며, 누락/0/NaN capsule은 enter 또는 spawn을 거부한다.
- NPC Beda는 combat target이 아니므로 capsule 대상에서 제외한다.

### 1.2 Skill hit volume

- `Data/Combat/Timelines/Player/<Class>.skilltimelines.json` 여섯 파일을 실제 정본으로 만든다.
- 현재 실측 90 skill, 118 effective stage를 모두 하나의 hit-volume timeline runtime으로 이관한다.
- 그중 damage-bearing inventory는 76 skill이며, no-damage stage는 hit event를 갖지 않는다.
- legacy damage-capable stage는 kind별로 고정한다. profile-empty ACTIVE는 stage 0 `NONE`, profile이 있는 ACTIVE는
  stage 0 damage, COMBO는 각 stage damage, HOLD는 stage 0/1 `NONE`·stage 2만 damage, COUNTER는 stage 0
  `NONE`·stage 1만 damage다. unknown kind/stage count는 converter가 거부한다.
- 기존 단일 `hitTimeMs + maximumRange` damage 경로와 새 capsule 경로를 동시에 운용하지 않는다.
- 기존 gameplay의 XZ 단일 target/총 damage 의미를 `LEGACY_XZ_RANGE`로 완전 migration하고, standalone mesh 근거가 승인된 stage만
  reviewed multi-window capsule로 교체한다.
- `PlayerSkills.json`의 timing/combo field를 읽던 Client catalog, Balance Tool, animation/effect pipeline, provenance,
  audit 소비자도 같은 변경에서 timeline join으로 이관한다. Server만 바꾸고 Client tool을 깨뜨린 상태를 허용하지
  않는다.

### 1.3 Damage

- `C2S_USE_SKILL`은 현재처럼 skill/aim 의도만 보낸다.
- Server가 action, stage, quantized window, caster anchor, target hurt capsule을 이용해 overlap을 계산한다.
- 후보는 `(distanceSquared, NetEntityId)`로 정렬하고 `maximumTargets`까지만 적용한다.
- dead/NPC/self/despawn target은 제외한다.
- `DAMAGE_EVENT`, HP, death action은 기존 Server snapshot 경로를 그대로 사용한다.

### 1.4 F7 debug overlay

- focused Client window에서 F7 key-down edge 한 번에 on/off한다.
- player hurt capsule, monster hurt capsule, Valtan hurt capsule, 현재 active skill hit volume과 Server가 연
  pending/active Valtan occurrence를 wire로 그린다. player legacy shape는 XZ ring, reviewed player/weapon shape는
  capsule이며 boss analytic circle/ring/cone/box/cross도 같은 debug catalog에서 그린다.
- F7은 `_DEBUG`에만 존재하고 Release binary에는 key polling, catalog load, draw submit이 없다.
- disconnect, level leave, despawn에서 debug state를 정리한다.
- debug catalog가 없거나 잘못됐으면 overlay만 unavailable로 만들고 gameplay는 계속한다.

### 1.5 4직업 standalone mesh 연결

- 우선 연결 순서는 DimensionMaster `2050210`, Lance Master `34010`, Artist `31000`, Warlord `17000`이다.
- 각 stage/occurrence에 대응하는 persisted `hitEventId`는 combat timeline이 소유하고, 별도 hit-trace 문서가
  effect occurrence를 그 ID에 단방향으로 연결한다. Effect 문서와 correction/materialization manifest에는 gameplay
  ID를 역으로 쓰지 않는다.
- 장식용 echo/flow/rim/afterimage/sprite 각각에 collider를 만들지 않는다. schema v1에서 occurrence 하나는
  reviewed gameplay capsule 하나로 귀결한다. 단일 capsule로 의도를 보존할 수 없으면 임의 compound를 만들지
  않고 `LEGACY_XZ_RANGE`를 유지하며, compound shape는 별도 schema/version 수직 슬라이스로 미룬다.
- 현재 DimensionMaster 2050210은 시각 occurrence가 4개지만 기존 Server damage는 1회다. 네 window에 같은
  damage profile을 각각 적용해 4배 damage를 만들지 않는다. 첫 단계는 `ONCE_PER_STAGE` budget으로 기존 총량을
  보존한다. 실제 4-hit damage로 바꿀 때는 per-hit damage profile 또는 검증된 분배 계약을 별도 반영한다.
- Lance Master, Artist, Warlord materialization이 `blocked`인 동안 Imported 진단물에서 collider를 자동 cook하지
  않는다. 기존 migration `LEGACY_XZ_RANGE`를 유지하고, admitted effect와 stable carrier가 생긴 뒤 reviewed
  capsule로 교체한다.

### 1.6 Valtan 조건부 공격 volume

- `ValtanEncounter.json`의 pattern stage를 explicit `hitEvents[]`와 `motion`으로 정규화한다.
- 도끼 휘두르기, root 고정 장판, target/world 고정 장판, 돌진 전방 collider를 같은 Server hit-event runtime으로
  표현한다.
- stage 진입, event anchor capture, active half-open window, repeat ordinal이 collider 생성·종료 조건이다.
- 무기 bone 궤적은 Animation Tool이 offline seed를 만들고, Balance Tool에서 승인한 root-local primitive track만
  Server gameplay data가 된다. Server가 `b_wp_r_01`이나 Client skeleton을 직접 읽지 않는다.
- F7은 pending event를 청록 wire, active event를 노랑 wire, 최근 적중 event를 주황 ghost로 보여준다.

### 1.7 Knockback와 hit reaction

- player skill이 일반 monster를 맞히거나 Valtan/monster 공격이 player를 맞히면 hit와 같은 Server transaction에서
  reaction profile을 resolve한다.
- authoritative root 이동은 PhysX rigid body가 아니라 Server 30 Hz kinematic forced-motion sweep이다.
- Client는 snapshot position에 수렴하면서 reaction animation을 재생한다. PhysX impulse는 향후 파편·ragdoll 같은
  비권위 secondary presentation에만 허용한다.
- Valtan은 기본 displacement immune, 일반 monster는 authored scale, player는 class profile scale을 사용한다.

### 1.8 Character Select Arena 수동 spawn

- Map Tool은 Development Map Editor에서 Character Select Area를 골라 Valtan placement와 monster spawn anchor를
  map surface pick으로 저장한다. 제품 Character Select 화면 안에서 Map Tool을 직접 띄우지 않는다.
- Server Arena에는 `Spawn Monsters`, `Spawn Valtan`, `Spawn All` 세 typed button을 제공한다.
- Client는 stable activation ID만 요청하고 Server가 boss placement와 SpawnGroup을 전체 preflight한 뒤 같은 room
  command에서 원자적으로 즉시 생성한다.
- 새 `CMonster`/`vector<CMonster>`를 만들지 않고 기존 `m_WorldEntities`와 `CMonsterBrain`, Client `CNpc` 경로를
  그대로 확장한다.

### 1.9 Valtan lifecycle와 HUD

- `present`, `alive`, `patternActive`, `phase`, `currentHealthBar`를 서로 다른 의미로 유지한다.
- Server entity 존재/HP/action이 권위이며 `CValtan`의 flag는 presentation mirror일 뿐이다.
- Shared snapshot이 `maximumHealthBars`를 보내고 공용 계산 함수로 현재 bar를 구한다. Valtan의 최대치는 160이며
  `x130`은 phase가 아니라 현재 130줄 표기다.
- `CCombatHUDViewModel`은 spawn/first snapshot, death `x0`, despawn/room reset clear를 모두 처리한다.

## 2. 현재 코드 실측

| 영역 | 현재 상태 | 이번 변경 |
|---|---|---|
| Player Client collider | `CCharacter` root를 따라가는 OBB `(0.9, 1.8, 0.9)`가 존재 | 이동 OBB 유지, 별도 combat capsule debug 추가 |
| Monster/Valtan Client collider | `CNpc`, `CValtan`에 combat collider 없음 | snapshot + local debug catalog로 overlay 구성 |
| Engine legacy collider | `SPHERE/AABB/OBB`, capsule 없음 | 범용 wire capsule draw만 Engine에 추가 |
| Client PhysX | capsule shape는 있으나 trigger/query/callback/stable ID 없음 | damage 판정에 사용하지 않음 |
| Server movement collision | player OBB 대 static `collisionBox` swept test | 변경하지 않음 |
| Server skill damage | hit time 뒤 XZ range 안 가장 가까운 boss/monster 1개 | hit window와 capsule overlap으로 교체 |
| Server monster body | `collisionRadius`만 존재 | complete hurt capsule spec으로 확장 |
| Server Valtan body | boss profile `collisionRadius=3.0` | complete hurt capsule spec으로 확장 |
| Snapshot | player/world transform, yaw, action, skillId, stage, actionStartTick 존재 | shape 재구성에 사용, enter 승인에 runtime revision 1필드 추가 |
| Effect runtime | Mesh/Sprite presentation 전용 | damage collider와 분리, traceability만 연결 |
| Global key | F1, F6만 허용하고 F7-F12 금지 | 사용자 요청을 public key migration으로 반영 |
| Client skill/tool consumers | `CPlayerSkillCatalog`, Balance/Animation/Effect Tool과 Python pipeline이 `comboStages`를 join | six timeline 문서의 stage metadata로 이관 |

현재 worktree에는 Effect, HDR, network, `Character.cpp`, `ClientReplication.cpp`, `MainApp.cpp`, project file,
`ProjectAudit` 변경이 함께 존재한다. 이 계획을 작성하는 단계에서는 그 파일을 수정하지 않는다. 실제 구현은
해당 세션의 변경이 commit/merge된 뒤 새 `codex/combat-capsule-f7-debug` 브랜치에서 시작하고, merge 직전
현재 파일을 다시 실측한다.

## 3. 고정 불변식

1. Client가 target ID, hit 여부, damage 값을 Server에 보내지 않는다.
2. Server는 EffectAssetId, model path, mesh index, bone index, animation clip 이름을 읽지 않는다.
3. Effect JSON과 `.animevents`에 gameplay radius, damage, maximumTargets를 저장하지 않는다.
4. 모든 `HIT_VOLUME`은 stable `hitEventId`를 갖고 vector index를 저장 ID로 쓰지 않는다.
5. hit-window authoring ms는 publisher가 current C++의 float32 누적
   `elapsed += 1.f/30.f; elapsed >= float(ms)*0.001f`를 bit-accurate하게 모사해 0-based evaluation offset으로
   한 번만 양자화한다. 단순 `ceil(ms*30/1000)-1` 식은 쓰지 않는다. root-motion sample과 input window ms는
   현재 의미를 보존한다.
6. window는 half-open `[startTick, endTick)`이며 최소 1 tick이다.
7. 같은 `(actionStartTick, stageIndex, hitEventId, repeatOrdinal, targetNetEntityId)`는 한 번만 damage를 준다.
8. `ONCE_PER_STAGE` event group은 첫 성공 적용 뒤 같은 stage의 다른 window가 총량을 중복 적용하지 않는다.
9. body capsule이 누락된 combat actor는 radius 0 또는 임의 default로 정상화하지 않는다.
10. publisher/runtime load는 `parse -> validate -> stage -> commit`을 지키며 실패하면 기존 runtime 파일과
    catalog를 유지한다.
11. F7 overlay는 gameplay state를 수정하거나 command를 제출하지 않는다.
12. Server/Client runtime revision parity는 publisher와 ProjectAudit가 build/publish 전에 강제하고,
    `S2C_ENTER_ACCEPTED.strCombatRuntimeRevision`으로 원격 Server가 실제 승인한 revision까지 확인한다. local debug
    catalog와 다르면 F7만 fail closed하며 gameplay 접속은 유지한다.
13. actor movement blocking과 combat hurt query를 같은 시스템으로 합치지 않는다.
14. blocked effect materialization은 gameplay admission 근거가 아니다.
15. skill당 stage 8, stage당 hit event 16, event당 repeat 8, repeat당 target 16, stage당 총 hit application 2048을
    public upper bound로 고정한다. publisher와 runtime이 같은 상한을 검사하며 vector는 상한 이상 성장하지 않는다.
16. repeat window끼리는 겹치지 않는다. `repeatIntervalTicks >= endTickOffset - startTickOffset`을 publisher가 강제한다.
17. `ONCE_PER_STAGE`의 damage profile은 `PlayerSkills.json.serverDamageProfileId` 하나를 참조하고,
    `maximumTargets`는 stage-owned budget 하나가 소유한다. event마다 다른 damage/profile/cap을 두지 않는다.
    `PER_EVENT_REPEAT`만 event-owned damage profile/cap을 허용하며 이 경우 skill default profile은 비워 duplicate
    authority를 만들지 않는다. schema v1은 한 skill 안에서 두 damage policy를 섞지 않는다. `NONE` stage는
    event/budget이 없고 guard/stance/no-damage를 표현한다.
18. hit tick age는 Shared `Try_GetForwardTickDistanceSkippingZero(start,current)`의 0-based 값이다. tick 0 sentinel을
    건너뛰는 wrap에서 raw unsigned subtraction을 쓰지 않는다. 새 action의 첫 Update는 age 0이고, stage transition은
    Shared `NextNonZeroTick(currentTick)`을 새 start로 예약해 0 ms event를 놓치지 않는다.
19. active hit volume은 정확한 Server tick만 노란색으로 표시한다. 렌더 프레임 사이에 사라지는 1-tick volume은
    별도 6-tick 주황색 ghost/history로 남기며, ghost는 판정 중인 collider로 오해되지 않게 role과 legend를 분리한다.

## 4. 데이터 계약

### 4.1 Actor body capsule

`PlayerProfiles.json`, `MonsterProfiles.json`, `BossProfiles.json`은 다음 object를 소유한다.

```json
{
  "combatBodyCapsule": {
    "radius": 0.45,
    "cylinderHalfHeight": 0.45,
    "centerOffsetY": 0.9
  }
}
```

`cylinderHalfHeight`는 두 hemisphere 중심 사이 segment의 절반 길이다. 따라서 전체 높이는
`2 * (cylinderHalfHeight + radius)`다. PhysX의 이름과 같은 의미를 쓰되 Server 구현은 PhysX를 링크하지 않는다.

1차 `PROJECT_TUNED` baseline은 현재 horizontal radius와 player OBB envelope를 그대로 보존한다.

| owner | radius | cylinderHalfHeight | centerOffsetY | 전체 높이 |
|---|---:|---:|---:|---:|
| 여섯 player class | 0.45 | 0.45 | 0.90 | 1.80 |
| `MONSTER_VALTAN_PADD_01` | 0.55 | 0.55 | 1.10 | 2.20 |
| `MONSTER_VALTAN_SJFC_00_4` | 0.60 | 0.60 | 1.20 | 2.40 |
| `MONSTER_VALTAN_0019_05` | 0.65 | 0.65 | 1.30 | 2.60 |
| `MINIBOSS_LUGARU` | 1.35 | 1.35 | 2.70 | 5.40 |
| `BOSS_VALTAN` | 3.00 | 3.00 | 6.00 | 12.00 |

이는 radius 대비 전체 높이 4배의 명시적 baseline이다. runtime model scale을 곱하지 않는다. 이후 body-only
bind-pose/sample union을 이용해 개선하더라도 offline 측정 결과를 profile에 저장하고 provenance를
`PROJECT_TUNED`로 갱신한다.

### 4.2 Player skill과 combat timeline 분리

`PlayerSkills.json`은 formatVersion 3으로 올리고 다음만 소유한다.

- identity: class, skillId, inputSlot, displayName, actionId, skillKind
- command admission: cooldown, resourceCost, stance
- current single-budget damage link: `serverDamageProfileId`
- `combatTimelineId`

기존 `actionDurationMs`, `hitTimeMs`, `movementDistance`, `maximumRange`, `comboStages`와
`Data/Animation/RootMotion/*.rootmotion.json`의 Server root-motion sample은 모든 90 skill이 timeline으로
이관된 같은 변경에서 제거한다. 현재 effect 세션이 소비하는 optional `effectId`는 이번 collider slice에서
제거하지 않고 presentation fallback으로 그대로 보존한다. Server는 `effectId`를 계속 무시한다.

이 제거는 Server parser만의 변경이 아니다.

- `CPlayerSkillCatalog`는 six combat timeline을 staged load해 `iComboStageCount`를 join한다. current ONCE skill의 HUD
  damage는 `serverDamageProfileId`, future PER-event skill은 event profile rate의 bounded 합계로 derive한다.
- `CAnimationSkillBindingDocument`의 public 입력은 현재처럼 `iComboStageCount`를 받되 그 수치의 source가
  PlayerSkills가 아니라 combat timeline이 된다.
- Balance Tool은 cooldown/resource/damage/profile 편집만 `PlayerSkills.json`에 저장하고 duration/hit/range/root
  motion/combo controls를 제거한다. 1차에는 timeline summary를 read-only로 보여 주며 collider editor를 새로
  만들지 않는다.
- Effect/animation materializer와 audit가 `comboStages`를 직접 읽지 않고 `(combatTimelineId, stageIndex)`를 join한다.
- provenance export/update는 이동한 field의 target path를 Data/Combat으로 갱신한다.
- 기존 네 root-motion 문서는 converter가 timeline `rootMotion` array로 합친 뒤 삭제하며, publisher가 old/new
  source를 동시에 발견하면 실패한다. authoring sample의 `timeMs`는 그대로 보존하고 hit window만 tick으로
  양자화한다.
- legacy stage에 root-motion sample이 없고 `movementDistance>0`이면 converter가
  `(0 ms, 0, 0) -> (durationMs, movementDistance, 0)` 두 sample로 materialize한다. 이는 현재 linear fallback과
  같은 보간이며 migration parity test를 통과한 뒤 legacy `movementDistance`를 제거한다.

각 class 파일은 다음 구조를 사용한다.

```json
{
  "schema": "lostark.player-combat-timelines",
  "formatVersion": 1,
  "characterClass": "DIMENSIONMASTER",
  "fixedTickHz": 30,
  "timelines": [
    {
      "timelineId": "combat.player.2050210",
      "skillId": 2050210,
      "stages": [
        {
          "stageIndex": 0,
          "durationMs": 1800,
          "inputWindow": null,
          "rootMotion": [],
          "damageApplicationPolicy": "ONCE_PER_STAGE",
          "damageBudget": {
            "damageProfileSource": "SKILL_DEFAULT",
            "maximumTargets": 1
          },
          "events": [
            {
              "eventId": "hit.01",
              "kind": "HIT_VOLUME",
              "startMs": 250,
              "endMs": 284,
              "anchorPolicy": "ACTION_ROOT_AT_WINDOW_OPEN",
              "shape": {
                "kind": "CAPSULE",
                "localStart": [0.0, 0.9, 0.4],
                "localEnd": [0.0, 0.9, 2.4],
                "radius": 0.8
              },
              "repeatCount": 1,
              "repeatIntervalMs": 0
            }
          ]
        }
      ]
    }
  ]
}
```

위 숫자는 schema 의미를 설명하는 예시이며 2050210 최종 authoring 값으로 바로 publish하지 않는다. 최종 값은
effect restoration의 admitted occurrence와 기존 Server 총 damage를 함께 검증한 뒤 기록한다.

Timeline 불변식은 다음과 같다.

- `timelineId`는 `PlayerSkills.json`에서 정확히 한 번 참조된다.
- `stageIndex`는 0부터 연속이고 binding stage 수와 일치한다.
- 모든 event는 stage duration 안에 있다.
- shape는 exact union이다. `LEGACY_XZ_RANGE`는 finite positive range와 empty capsule payload,
  `CAPSULE`은 range 0과 finite `localStart/localEnd/radius>0`을 요구한다.
- `repeatCount`는 1~8이다. `ONCE_PER_STAGE`의 stage maximumTargets 또는 `PER_EVENT_REPEAT`의 event
  maximumTargets는 1~16으로 제한한다.
- stage는 최대 8개, stage당 event는 최대 16개이다. ONCE는
  `events * repeats * stage.maximumTargets`, PER_EVENT는 `sum(event.repeats * event.maximumTargets)`가 2048
  이하여야 한다.
- repeatCount가 2 이상이면 quantized interval은 window 길이 이상이어야 한다.
- 마지막 repeat window도 stage duration을 넘지 않는다.
- `NONE` no-damage stage는 damage event와 damage budget이 모두 없다.
- damage profile ID는 `DamageProfiles.json`에 존재한다.
- `ONCE_PER_STAGE` stage는 non-null stage damage budget을 정확히 하나 가지며
  `damageProfileSource=SKILL_DEFAULT`를 사용한다. skill의 `serverDamageProfileId`는 non-empty여야 하고 event에는
  damage profile/cap이 없다.
  여러 visual window가 있어도 Server damage budget은 stage당 한 번이다.
- 실제 multi-hit은 `PER_EVENT_REPEAT`으로 명시하고 stage damage budget은 null이며 각 event가 damage profile과
  maximumTargets를 가진다. 이때 skill default profile은 empty여야 한다. 동일 target에 대한 횟수를 contract
  test로 고정한다. v1은 한 skill의 damaging stage에서 ONCE/PER_EVENT 혼합을 거부한다.
- v1 anchor는 `ACTION_ROOT_AT_WINDOW_OPEN`만 허용한다. window가 열리는 Server tick의 root/yaw를 snapshot하고
  해당 repeat가 끝날 때까지 world-fixed로 유지한다.
- projectile을 긴 capsule이나 follow collider로 위장하지 않는다.

### 4.3 전체 migration

새 runtime을 켜기 전에 converter가 현재 90 skill/118 stage와 네 root-motion 문서를 모두 timeline으로 생성한다.

- duration/input과 authored root-motion sample의 값과 순서를 보존해 옮긴다. sample 없는 linear
  `movementDistance`는 stage duration 양 끝의 두 root-motion sample로 lossless materialize한다.
- 현재 damage stage는 기존 hit 시각을 포함하는 정확히 1 tick짜리 event로 옮긴다. damage-capable 판정은
  `ACTIVE(profile 유무) / COMBO(all stages) / HOLD(stage 2 only) / COUNTER(stage 1 only)` 표만 사용하고, 나머지
  guard/charge stage는 `NONE` + event 0개로 옮긴다.
- 현재 최근접 단일 target 의미를 보존하도록 `maximumTargets=1`, `repeatCount=1`,
  `damageApplicationPolicy=ONCE_PER_STAGE`를 사용한다.
- 기존 `maximumRange`는 명시적 `LEGACY_XZ_RANGE` shape의 radius로 옮긴다. 이 shape는 caster/target root의 XZ
  squared distance와 target body radius만 사용해 기존 `maximumRange + targetRadius`를 수학적으로 그대로
  보존한다. 별도 legacy damage 함수가 아니라 같은 `CCombatCollisionSystem`의 explicit shape branch다. reviewed
  stage만 방향성/높이 분리를 가진 `CAPSULE`로 교체한다.
- root-motion `timeMs/forward/lateral` sample은 값과 순서를 보존하며, stage duration reference만 새 timeline에서
  resolve한다. old root-motion source와 timeline root-motion이 공존하는 혼합 입력은 거부한다.
- 모든 timeline이 validate되기 전에는 bootstrap version 5를 publish하지 않는다.
- v4 fallback reader를 남기지 않는다. Server와 Data를 같은 변경 단위로 v5에 올린다.

기존 hit 시각 `T ms`의 first evaluation offset은 current Server와 같은 float32 step/threshold/누적 round를
offline으로 반복해 처음 threshold를 통과한 0-based offset을 선택한다. migration pulse는 정확히
`[firstOffset, firstOffset + 1)`이다. 단순 rational ceil은 1500/1600/1700/2200 ms 등에서 current float 누적과
한 tick 어긋날 수 있으므로 금지한다. G8 admission 전 frozen v2 input을 pure legacy로 변환한 118-stage
converter golden은 각 stage를
`optional<firstFireOffset>`으로 비교해 `NONE=null`까지 고정하고 damage policy/event count도 함께 비교한다.
0/33/34/300/1500/1600/1700/2200 ms, stage transition, `uint32` zero-skipping wrap fixture가 publisher와 Server
runtime을 함께 고정한다.
최종 product graph에서는 hittrace의 reviewed allowlist를 제외한 stage만 이 legacy tuple과 같아야 한다. 현재
allowlist `(2050210, stage 0)`은 capsule event `hit.01~04`, start offset `7/17/26/38`, `ONCE_PER_STAGE`, stage 총
damage budget 1회라는 별도 golden을 통과한다. 이후 admission은 allowlist와 trace receipt를 같은 commit에서 늘린다.
duration/input/root-motion은 ms source와 기존 float clock 의미를 유지한다.

### 4.4 Effect traceability

Server가 읽지 않는 별도 문서
`Data/Combat/Reference/FourClassStandaloneMesh.hittrace.json`을 둔다.

문서는 두 exact collection을 소유한다.

- `sources[]`: class/skill stable source ID, materialization document path/hash/status. `ADMITTED` source만
  skillbinding/animevents document hash, binding stage/clip, unique EFFECT cue tuple, effectAssetId/effect document
  version/hash를 추가로 갖고, `BLOCKED` source는 admitted effect/cue field나 link를 갖지 않는다.
- `links[]`: admitted source ID, stageIndex/hitEventId, occurrence/group ID, collision carrier element ID,
  anchor/timing review 결과.

audit은 `skillId -> binding stage -> clip -> unique animevent EFFECT cue -> effectAssetId -> occurrence -> carrier`를
끝까지 join한다. materialization/binding/animevents/effect-document content hash, cue start/payload/anchor/follow,
carrier resource 존재, carrier가 standalone Mesh인지, 장식 layer가 아닌지, action-relative cue+occurrence 시간이
hit window와 같은지를 검사한다. 이 문서는 damage 수치나 capsule 크기를 소유하지 않는다.

### 4.5 Runtime 산출물

- `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` formatVersion 5
- all product world bootstrap formatVersion 7
- Valtan과 Character Select Arena spawn group bootstrap formatVersion 2
- Server/Client `Navigation/{LV_LUT_HEARTRB_ED,LV_LOBBY_CLASSSELECT_SL00,LV_DEV_TRAINING_GROUND}.navgrid` 세 쌍
- `Client/Bin/DataFiles/Gameplay/CombatColliders.debug.json` formatVersion 1
- `Server/Bin/DataFiles/RuntimeSet.balance.manifest` formatVersion 1

Server gameplay bootstrap, Valtan/Character Select spawn-group bootstrap을 포함한 모든 world bootstrap, Server/Client
navigation grid와 Client debug catalog는 같은 canonical runtime-set hash를 가진다. `Publish-BalanceRuntimeSet.ps1`이
gameplay/world/navigation/client 네 계열을 한
staging root에서 만든 뒤 한 transaction으로 promote한다. 현재 orchestrator target에서 빠진
`VALTAN_ARENA.spawngroupsbootstrap`과 `CHARACTER_SELECT_ARENA.spawngroupsbootstrap`도 promotion/rollback 목록에
반드시 포함한다. manifest artifact row는 `SERVER_REQUIRED` 10, `CLIENT_RUNTIME` 3, `CLIENT_DEBUG` 1의 정확히 14개이고
manifest-last까지 promote target은 15개다. 개별 publisher의 Publish는
staging output 생성용이고 제품 pre-build는 orchestrator만 호출한다. Client runtime은 local debug catalog의
hash가 없거나 schema가 다르면 overlay만 fail closed한다. 원격 Server revision 비교도 이번 slice의
`S2C_ENTER_ACCEPTED.strCombatRuntimeRevision`으로 수행한다. 이는 hot reload가 아니라 connect-time handshake다.
manifest 안의 `combatRuntimeRevision` full lowercase SHA-256가 packet에 들어가며, protocol version을 올리고 exact
64-hex validation과 roundtrip/truncated/trailing test를 같은 변경에 포함한다. `CServerApp::Run`은 room 생성 전에
line-based manifest를 strict parse하고 모든 `SERVER_REQUIRED` artifact의 destination/hash를 실제 파일과 대조한다.
그 뒤 승인 revision을 각 room 생성자에 주입한다. 각 room은 gameplay/world와, 해당 world에 실제 존재해야 하는
spawn-group artifact의 header revision이 승인 revision과 다르면 ready 상태가 되지 않는다. Valtan Arena와
Character Select Arena는 spawn-group artifact가 필수이고 Bern/Training Ground는 artifact 부재가 정본이다.

## 5. Runtime 흐름

```text
C2S_USE_SKILL(skillId, aim)
  -> CPlayerSkillSystem::Try_Start
  -> skillId -> combatTimelineId -> current stage
  -> Server fixed tick enters HIT_VOLUME window
  -> capture caster root/yaw for this repeat
  -> LEGACY_XZ_RANGE: root XZ range + target body radius
     CAPSULE: local capsule -> world capsule -> AABB/capsule overlap
  -> stable sort(shape-specific distanceSquared, NetEntityId)
  -> per-stage/per-event ledger check
  -> Resolve_Damage / defense / HP / death
  -> DAMAGE_EVENT + WORLD_SNAPSHOT

same snapshot + same authored debug catalog
  -> CCombatColliderDebugViewModel
  -> active body capsules / legacy hit rings / reviewed hit capsules
  -> CCombatColliderDebugRenderer
  -> F7 wire overlay
```

## 6. G별 구현 순서

## G0. 세션 분리와 baseline 고정

### 변경

1. 진행 중 effect/HDR/network 작업을 먼저 commit 또는 merge한다.
2. 현재 공유 dirty worktree에서 branch를 switch하지 않는다. effect baseline commit이 확정되면 sibling worktree에
   `git worktree add ..\LostArk-combat-collider -b codex/combat-capsule-f7-debug <baseline-commit>`으로 분리하거나,
   모든 다른 세션이 끝나고 현재 worktree가 clean임을 확인한 경우에만 같은 위치에서 branch를 만든다.
3. sibling 생성 직후 이 구현 계획서와 상세 코드 계획서 두 파일만 sibling의 같은 경로로 복사하고 SHA-256를
   원본과 비교한다. sibling에서는 구현 전부터 두 PLAN을 tracked change로 유지하되, 공유 dirty worktree의 index와
   branch에는 손대지 않는다. 첫 검증 단위에 두 PLAN을 함께 포함한다.
4. 아래 overlap 파일의 새 baseline을 다시 읽는다.
   - `Client/Private/Character.cpp`
   - `Client/Private/ClientReplication.cpp`
   - `Client/Private/MainApp.cpp`
   - `Client/Default/Client.vcxproj(.filters)`
   - `Server/Private/GameRoom.cpp`
   - `Server/Default/Server.vcxproj(.filters)`
   - `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
5. 현재 90 skill/118 stage와 actor profile 목록을 inventory fixture로 고정한다.

### 종료 증거

- collider sibling worktree 시작 시 unrelated dirty file이 없고 현재 shared dirty worktree의 branch/index가 바뀌지 않는다.
- sibling의 두 PLAN SHA-256가 이 세션 원본과 같고 구현 branch의 첫 검증 단위에 포함된다.
- effect restoration commit/revision이 trace authoring의 입력으로 기록된다.

## G1. Shared 순수 capsule geometry

### 신규 파일

- `Shared/Public/Gameplay/CombatCollisionContract.h`
- `Shared/Private/CombatCollisionContract.cpp`
- `Shared/Public/Network/NetworkTickContract.h`

### 변경 파일

- `Shared/Default/Shared.vcxproj`
- `Shared/Default/Shared.vcxproj.filters`
- `Server/Private/ServerGameplayContractTests.cpp`

### 구현

- plain float vector, body capsule spec, world capsule, AABB를 정의한다.
- finite/positive validation을 제공한다.
- local Y-yaw transform, vertical body capsule build, capsule AABB build를 제공한다.
- robust segment-segment squared distance와 inclusive tangent overlap을 제공한다.
- `NextNonZeroTick`과 `Try_GetForwardTickDistanceSkippingZero`를 header-only Shared tick 계약으로 제공해
  Server hit age, Client F7 active/ghost age, presentation timeline이 같은 0-sentinel wrap 수학을 사용하게 한다.
- Engine/DirectX/PhysX/Client type을 include하지 않는다.
- epsilon, maximum radius, maximum segment length를 public constant로 고정한다.

### 종료 증거

- identical, separated, tangent, skew, parallel, degenerate segment, vertical separation, NaN input test가 통과한다.
- Shared Debug/Release build와 Server `--contract-test`의 pure geometry section이 통과한다.

## G2. Authoring schema, 전체 migration, publisher

### 신규 파일

- `Data/Combat/Timelines/Player/Artist.skilltimelines.json`
- `Data/Combat/Timelines/Player/DimensionMaster.skilltimelines.json`
- `Data/Combat/Timelines/Player/Gunslinger.skilltimelines.json`
- `Data/Combat/Timelines/Player/LanceMaster.skilltimelines.json`
- `Data/Combat/Timelines/Player/Slayer.skilltimelines.json`
- `Data/Combat/Timelines/Player/Warlord.skilltimelines.json`
- `Tools/GameplayPipeline/Convert-PlayerSkillsToCombatTimelines.ps1`
- `Tools/GameplayPipeline/Test-CombatTimelinePublisher.ps1`
- `Tools/GameplayPipeline/Test-BalanceRuntimeSetRollback.ps1`
- `Tools/GameplayPipeline/player_combat_timelines.py`
- `Tools/GameplayPipeline/test_player_combat_timelines.py`

### 변경 파일

- `Data/Balance/PlayerSkills.json`
- `Data/Balance/PlayerProfiles.json`
- `Data/Balance/MonsterProfiles.json`
- `Data/Balance/BossProfiles.json`
- `Data/Animation/RootMotion/Artist.rootmotion.json` 삭제/이관
- `Data/Animation/RootMotion/DimensionMaster.rootmotion.json` 삭제/이관
- `Data/Animation/RootMotion/LanceMaster.rootmotion.json` 삭제/이관
- `Data/Animation/RootMotion/Warlord.rootmotion.json` 삭제/이관
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`
- `Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1`
- `Tools/GameplayPipeline/Update-BalanceProvenanceReceipt.ps1`
- `Tools/GameplayPipeline/Export-OfficialBalanceReceipt.py`
- `Tools/WorldPipeline/Publish-WorldGameplay.ps1`
- `Server/Default/Server.vcxproj`
- `Client/Public/PlayerSkillCatalog.h`
- `Client/Private/PlayerSkillCatalog.cpp`
- `Client/Public/BalanceTool.h`
- `Client/Private/BalanceTool.cpp`
- `Client/Private/Animation_Tool.cpp`
- effect/animation pipeline 중 `comboStages`, `actionDurationMs`, `hitTimeMs`를 직접 읽는 Python script와 대응 test
- `Tools/ProjectAudit/Test-EffectToolFinal.ps1`
- Client project의 `96.DataFiles` 항목

### 구현

- converter는 one-shot migration 도구이며 runtime에서 호출하지 않는다.
- publisher는 exact property set, version, stable ID, finite range, stage/event/reference graph를 모두 검증한다.
- gameplay bootstrap v5에 skill/timeline/stage/root-motion/hit-volume/body-capsule row를 deterministic order로 쓴다.
- PlayerProfiles v3, MonsterProfiles v2, BossProfiles v4로 올리고 top-level monster/boss `collisionRadius`를
  `combatBodyCapsule` exact object로 원자적으로 이관한다.
- world spawn bootstrap v2에 monster body capsule 세 필드를 쓴다.
- Client debug catalog는 같은 validated staged graph에서 생성한다.
- Client skill catalog와 Balance/Animation/Effect Tool은 timeline join 실패 시 기존 in-memory state를 유지하고
  Save를 차단한다.
- Balance Tool의 `PLAYER_EDIT`와 `BOSS_EDIT`도 `combatBodyCapsule` 세 필드를 staged parse/validate/save한다.
  PlayerProfiles v3와 BossProfiles v4 exact-property set을 함께 올리고, 기존 boss `collisionRadius` UI/writer는
  capsule radius로 대체한다. 새 body object를 모르는 구버전 writer가 저장 과정에서 필드를 지우는 경로를 남기지 않는다.
- Python effect materializer는 timeline stage count를 입력으로 받고 blocked source 정책을 그대로 유지한다.
- `Publish-BalanceRuntimeSet.ps1 -Mode Validate`는 gameplay/world/spawn-group/Server·Client navigation/client debug 전체 staged set을 strict
  reload하되 destination을 바꾸지 않는다. 별도 failure-injection harness가 각 promote index의 rollback을
  실행한다.
- 개별 publisher는 artifact와 canonical input receipt를 staged output으로만 만들고, runtime-set orchestrator는 temp
  write, flush, strict reload, gameplay/world/spawn-group/Server·Client navigation/client debug 전체 promote와
  reverse-order rollback을 수행한다. navigation publisher가 이 경로에서 destination을 선행 교체하지 않는다.

### 종료 증거

- 90 skill과 118 stage가 정확히 한 번 포함된다.
- duplicate timeline/event ID, unknown damage, wrong class, bad stage count, NaN, zero radius, short/trailing row,
  repeat overflow가 각각 실패한다.
- failure injection 뒤 Server/Client 기존 runtime hash가 유지된다.
- frozen pre-admission migration parity harness에서 118-stage legacy tuple이 같고, final graph에서는 reviewed
  allowlist를 제외한 모든 stage가 그 tuple과 같다.
- foundation commit의 reviewed allowlist는 empty라서 118-stage pure legacy graph를 publish한다. G8 trace commit 뒤
  allowlist `(2050210,0)`은 네 exact capsule offset과 `ONCE_PER_STAGE` 총 damage 1회를 지킨다.
- old root-motion 문서와 새 timeline sample의 `timeMs/forward/lateral` parity가 같다.
- legacy linear `movementDistance` fixture의 30 Hz 누적 root position이 synthetic two-sample timeline과 같다.
- orchestrator가 포착한 각 mutation index failure injection에서는 gameplay, staged manifest가 열거한 모든 world,
  Valtan/Character Select spawn-group, Server/Client navgrid와 Client debug output이 전부 이전 revision으로 복구된다.
  Bern/Training expected-absent spawn tombstone 최대 2개도 같은 reverse rollback에 포함한다. process kill/power-loss처럼
  catch가 실행되지 않는 crash는 old manifest와 mixed artifact를 남길 수 있으므로 이전 revision 복구를 주장하지 않는다.
  Server manifest hash 검증이 room/socket 전에 fail closed하고, 다음 orchestrator 실행이 canonical input에서 14 artifact를
  전부 다시 stage/promote한 뒤 manifest-last를 교체해 새 complete set으로 heal하는지 child-process kill harness로 검증한다.
- Client skill list, binding stage validation, Balance Tool save, effect materializer가 더 이상 제거된 legacy field를
  참조하지 않는다.
- Balance Tool player/boss reload-save-reload에서 capsule 세 필드와 profile version이 보존되고 invalid/partial body는
  staged state를 commit하지 않는다.

## G3. Server catalog와 actor body admission

### 변경 파일

- `Server/Public/GameplayCatalog.h`
- `Server/Private/GameplayCatalog.cpp`
- `Server/Public/BalanceRuntimeSetManifest.h` 신규
- `Server/Private/BalanceRuntimeSetManifest.cpp` 신규
- `Server/Public/SpawnGroupBootstrap.h`
- `Server/Private/SpawnGroupBootstrap.cpp`
- `Server/Public/WorldBootstrap.h`
- `Server/Private/WorldBootstrap.cpp`
- `Server/Public/GameRoom.h`
- `Server/Public/ServerApp.h`
- `Server/Public/ServerPlayer.h`
- `Server/Public/ServerWorldEntity.h`
- `Server/Private/GameRoom.cpp`
- `Server/Private/ServerApp.cpp`
- `Server/Private/MonsterBrain.cpp`
- `Server/Default/Server.vcxproj`
- `Server/Default/Server.vcxproj.filters`
- `Server/Private/ServerGameplayContractTests.cpp`
- `Server/Public/ServerGameplayContractTests.h`

### 구현

- immutable timeline/stage/hit-event definition을 catalog에 추가한다.
- runtime-set manifest는 line-based exact format으로 parse하고 repository/output root를 벗어나는 path, duplicate
  destination, unknown role, bad 64-hex revision/hash, short/trailing row를 거부한다. Windows CNG SHA-256로 모든
  `SERVER_REQUIRED` 파일의 bytes를 검증하고, 실패하면 socket/room 생성 전에 Server start를 막는다.
- `CServerApp::Run`이 검증된 manifest revision을 `CGameRoom` 생성자에 넘긴다. room은 세 artifact를 서로 비교하는
  대신 각 loader header를 이 승인 revision에 직접 비교한다.
- player/boss runtime profile과 monster spawn profile에 `COMBAT_BODY_CAPSULE`을 추가한다.
- player enter, monster spawn, boss spawn 때 complete body spec을 runtime actor에 복사한다.
- boss hit 때 catalog를 다시 찾아 radius만 읽는 기존 분기를 제거한다.
- monster chase/attack reach가 삭제된 `fCollisionRadius` 대신 `CombatBodyCapsule.fRadius`를 사용하고 기존 AI
  horizontal reach 결과를 golden fixture로 보존한다.
- catalog load는 v5 전체 graph를 stage하고 성공한 경우에만 commit한다.
- gameplay/world/spawn-group loader는 artifact header의 64-hex runtime revision과 spawn-group source artifact
  존재 여부를 보관한다. gameplay/world는 모든 room에서 필수다. Valtan과 Character Select Arena는 spawn-group
  artifact가 필수이며 승인 revision과 같아야 한다. Bern/Training Ground는 spawn-group artifact 부재만 정상이며
  예상 밖 파일이 있으면 manifest 밖 입력이므로 ready를 거부한다. packet에는 room이 받은 승인 revision만 사용한다.
- 초기 load 실패는 Server start를 막고, test reload 실패는 이전 catalog를 유지한다.

### 종료 증거

- missing/duplicate body profile과 invalid bootstrap row가 spawn 전에 거부된다.
- player/monster/Valtan runtime object가 profile과 같은 capsule을 보유한다.
- 기존 static world collision harness가 그대로 통과한다.
- old manifest + new artifact, bad artifact hash, missing required destination에서 Server가 listener를 열기 전에
  실패한다.
- Bern/Training spawn-group absent는 ready, Valtan/Character Select absent는 fail, Bern/Training unexpected artifact
  present는 revision과 무관하게 fail한다.

## G4. Server hit window, overlap, damage ledger

### 신규 파일

- `Server/Public/CombatCollisionSystem.h`
- `Server/Private/CombatCollisionSystem.cpp`
- `Server/Public/PlayerSkillHitRuntime.h`
- `Server/Private/PlayerSkillHitRuntime.cpp`

### 변경 파일

- `Server/Public/PlayerSkillSystem.h`
- `Server/Private/PlayerSkillSystem.cpp`
- `Server/Public/ServerPlayer.h`
- `Server/Private/GameRoom.cpp`
- `Server/Default/Server.vcxproj`
- `Server/Default/Server.vcxproj.filters`

### 구현

- `CCombatCollisionSystem`은 단일 hit-volume query API 안에서 `LEGACY_XZ_RANGE`와 `CAPSULE`을 명시적으로
  분기한다. legacy는 caster/target root XZ와 target body radius를 기존 식 그대로 사용하고, capsule은 alive
  target body capsule을 build해 AABB broad phase 뒤 Shared narrow phase를 호출한다.
- legacy 후보 거리는 root XZ squared distance, capsule 후보 거리는 두 segment의 shortest squared distance다.
  각각 `(distanceSquared, NetEntityId)`로 정렬하며 HP를 직접 바꾸지 않는다.
- `CPlayerSkillHitRuntime`은 event/repeat anchor, hit target ledger, stage damage budget을 bounded vector로 소유한다.
- action start, stage advance, cancel, death, disconnect, level leave에서 runtime을 reset한다.
- `CPlayerSkillSystem`은 timeline duration/root motion/input window를 소비하도록 바꾼다.
- duration/input/root motion은 기존 ms/elapsed-seconds 의미를 보존하고, hit window만 0-based fixed-tick age로
  평가한다.
- event repeat의 first tick에 Server root/yaw를 capture한다.
- active window 매 tick query하되 같은 target에는 ledger policy대로 한 번만 적용한다.
- `maximumTargets`는 repeat 전체 unique-target cap이다. query 전에 이미 맞은 target ID를 exclusion으로 넘기고
  `remaining = cap - ledger.size()`만 scan/sort 결과로 받아, multi-tick window에서 cap을 넘거나 farther 신규 후보를
  잘못 버리지 않는다.
- damage 적용과 death/damage event 생성은 기존 `CPlayerSkillSystem` 책임을 유지한다.
- 기존 `hasAppliedSkillDamage`는 제거하고 `hasEvaluatedFirstHitWindow`와 명시적 hit runtime으로 역할을 나눈다.
- combo buffered advance는 첫 damage-capable window가 평가된 뒤 가능하게 하여 기존 cancel timing을 보존한다.
- `IsInsideComboWindow`와 `Try_Counter`도 `PLAYER_SKILL_DEFINITION::ComboStages` 대신 current combat timeline
  stage/input window를 resolve한다. `Release`는 skill kind만 확인하므로 기존 public command 의미를 유지한다.
- 새 action의 first Update는 `stageTickAge=0`이다. combo/hold/counter stage transition은 새 stage를 같은 Update에서
  재평가하지 않고 `actionStartTick=NextNonZeroTick(serverTick)`으로 예약한다. stage age는
  `Try_GetForwardTickDistanceSkippingZero`로만 계산하며 0 ms event는 다음 Update age 0에서 반드시 평가한다.
- packet에는 target, collider, damage 요청 필드를 추가하지 않는다.

### 종료 증거

- before/during/after window, tangent/miss, rotated anchor, vertical separation이 고정된다.
- monster와 Valtan이 같은 query path에서 target이 된다.
- multiple target은 stable order와 cap을 지킨다.
- repeated tick, repeat event, stage change, death/despawn에서 duplicate damage가 없다.
- `ONCE_PER_STAGE` DimensionMaster fixture가 네 visual window에도 기존 총 damage를 한 번만 적용한다.
- `PER_EVENT_REPEAT` fixture는 선언한 횟수만 적용한다.
- legacy 0/33/34/300/1500/1600/1700/2200 ms와 frozen pre-admission 118-stage
  `optional<firstFireOffset> + damagePolicy + eventCount` converter golden이 일치한다. final graph는 reviewed
  allowlist를 제외한 legacy stage parity와 allowlist별 trace/capsule golden을 따로 검사한다.
- start=`UINT32_MAX-1`에서 current=`UINT32_MAX`는 age 1, current=1은 age 2이고, start=`UINT32_MAX`에서
  current=1은 age 1이다. wrap 전후에도 stage age와 exactly-once ledger가 유지된다.

## G5. Client debug catalog와 snapshot view model

### 신규 파일

- `Client/Public/CombatColliderDebugCatalog.h`
- `Client/Private/CombatColliderDebugCatalog.cpp`
- `Client/Public/CombatColliderDebugViewModel.h`
- `Client/Private/CombatColliderDebugViewModel.cpp`

### 변경 파일

- `Shared/Public/Network/PacketMessages.h`
- `Shared/Private/Network/PacketMessages.cpp`
- `Shared/Public/Network/PacketType.h`
- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`
- `Server/Private/GameRoom.cpp`
- `Client/Public/NetworkManager.h`
- `Client/Private/NetworkManager.cpp`
- `Client/Public/ClientReplication.h`
- `Client/Private/ClientReplication.cpp`
- `Client/Public/ActionPresentationTimeline.h`
- `Client/Private/ActionPresentationTimeline.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`

### 구현

- protocol version을 bump하고 `S2C_ENTER_ACCEPTED`에 exact 64-lowercase-hex
  `strCombatRuntimeRevision`을 추가한다. `C2S_USE_SKILL`은 바꾸지 않는다.
- Server는 room이 검증한 runtime-set revision을 enter 승인에 싣고, NetworkManager는 현재 승인 revision을
  connection lifetime 동안 보관한다.
- catalog는 `_DEBUG`에서만 runtime JSON을 parse/validate/stage/commit한다. 승인 revision과 local source revision이
  같을 때만 available이다. mismatch/empty는 overlay만 disable한다.
- catalog load 실패 시 view model을 explicit unavailable/null-catalog state로 초기화한다. replication의 Debug
  `Apply_*` hook은 이 상태에서 no-op이므로 missing/corrupt debug data가 Client start나 gameplay를 막지 않는다.
- view model은 player spawn의 class, world spawn의 kind/archetype, snapshot transform/action/tick을 stable
  NetEntityId로 보관한다.
- snapshot server tick과 actionStartTick으로 current stage event window를 계산한다.
- body capsule은 current root를 따라가고 hit volume은 window open tick의 root/yaw를 capture해 repeat 동안 고정한다.
- TCP snapshot에서 exact window-open tick을 관측하지 못한 repeat는 현재 transform으로 늦게 보간하지 않는다.
  해당 hit volume만 생략하고 `late anchor unavailable` diagnostic을 남긴다.
- validated network identity는 presentation GameObject 생성 성공 여부와 무관하게 먼저 등록한다. model/prototype
  presentation 생성이 실패하면 body/hit debug identity는 유지하고 `presentationMissing` diagnostic만 남긴다.
- damage event target은 한 snapshot 동안 highlight할 수 있지만 damage 판정에는 사용하지 않는다.
- 1-tick hit volume도 관찰할 수 있도록 active 종료 뒤 6 server tick 동안 별도 ghost record를 보관한다. ghost는
  damage query와 무관하고 history cap을 초과하면 oldest-first로 제거한다.
- incoming snapshot을 기존 snapshot에 commit하기 전에 old action/stage를 incoming server tick으로 한 번
  transition-carryover 평가한다. same-skill 정상 stage advance 또는 살아 있는 player의 duration-complete `NONE`에서
  old 1-tick event가 그 tick active였다면 즉시 ghost를 만든다. DEAD/early cancel/disconnect/level reset에는 만들지
  않으며 이미 관측한 event/repeat ghost와 stable key로 중복하지 않는다.
- despawn/disconnect/level reset에서 entity, active window, highlight를 모두 제거한다.
- Character, CNpc, CValtan GameObject pointer와 vector index를 debug 저장 ID로 사용하지 않는다.
- `CActionPresentationTimeline`은 Shared tick helper가 future stage start를 반환할 때 presentation age 0으로
  clamp한다. F7 active/ghost expiry도 같은 helper가 forward distance를 반환할 때만 계산한다.
- action age는 equal tick distance 0을 허용하지만 replication snapshot ordering은 distance가 positive일 때만
  accept해 equal/duplicate snapshot을 거부한다.

### 종료 증거

- out-of-order snapshot은 기존 replication tick guard와 동일하게 무시된다.
- local/remote player, monster, boss shape가 correct owner profile과 join된다.
- invalid/missing catalog는 status만 남기고 replication과 gameplay를 방해하지 않는다.
- accepted/local revision mismatch, malformed 64-hex, truncated/trailing enter packet이 각각 harness에서 고정된다.

## G6. Capsule wire renderer와 F7

### 신규 파일

- `Client/Public/CombatColliderDebugRenderer.h`
- `Client/Private/CombatColliderDebugRenderer.cpp`

### 변경 파일

- `Engine/Public/DebugDraw.h`
- `Engine/Private/DebugDraw.cpp`
- `Client/Public/MainApp.h`
- `Client/Private/MainApp.cpp`
- `Engine/Default/Engine.vcxproj(.filters)`가 새 파일을 만들 때만 등록
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`

### 구현

- Engine `DX::DrawCapsule`은 두 segment endpoint와 radius를 wire로 그리는 범용 helper만 제공한다.
- Engine `DX::DrawRing`을 재사용해 `LEGACY_XZ_RANGE`를 ground XZ ring으로 그린다. 이 ring은 target body radius가
  별도 hurt capsule로 보이는 Minkowski 기준선이며, 임의 3D capsule로 위장하지 않는다.
- Client renderer는 `PrimitiveBatch<VertexPositionColor>`와 `BasicEffect`를 소유하는 debug component다.
- MainApp은 F7 focused edge를 추적하고 overlay visible state를 소유한다.
- engine/level update와 effect update가 끝난 뒤 view model frame을 renderer에 제출하고, visible일 때만
  `Add_DebugComponent`한다.
- color 계약:
  - local player hurt: green
  - remote player hurt: blue
  - monster hurt: red
  - Valtan hurt: magenta
  - active player hit: yellow
  - expired 6-tick hit ghost: dim orange
  - current snapshot damage target highlight: cyan
- overlay의 small text legend/status는 F1 Developer Tools diagnostics에 표시할 수 있지만 F7 toggle은 독립이다.
- Release에서는 전부 compile-time 제외한다.

### 종료 증거

- key hold가 반복 toggle하지 않는다.
- 다른 process가 focused일 때 toggle하지 않는다.
- F7 off에서 debug submit 수가 0이다.
- active window 밖 hit volume은 active color로 그리지 않는다.
- active window 밖 volume은 ghost history에 있을 때만 주황색이고, 노란 active shape로 남지 않는다.
- level transition과 disconnect 뒤 stale capsule이 없다.

## G7. F7 public key 계약 migration

### 변경 파일

- `AGENTS.md`
- `CLAUDE.md`
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`
- `.md/TEAM/UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md`
- `.md/TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md`
- `.md/TEAM/BALANCE_TOOL_OWNER_HANDOFF.md`
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`

### 계약 문구

```text
공식 전역 기능키는 Debug Developer Tools의 F1, follow/free camera의 F6,
read-only combat collider overlay의 F7이다.
F2~F5와 F8~F12로 level, map, profile, authoring 또는 gameplay 상태를 바꾸지 않는다.
F7은 _DEBUG Client presentation만 바꾸며 command/network/damage authority에 영향을 주지 않는다.
```

같은 문서 변경에서 gameplay data ownership도 current 구현으로 갱신한다.

- `PlayerSkills.json`: identity/input/cooldown/resource/stance/`combatTimelineId`, 현재 effect 세션의 optional
  presentation fallback `effectId`, current single-budget `serverDamageProfileId`
- `Data/Combat/Timelines/Player`: duration/input/root motion/hit volume/damage event/stage count
- `DamageProfiles.json`: damage rate/formula
- `skillbindings.json`: ordered presentation clip
- `.animevents`: Client presentation cue

신규 skill 추가 절차는 PlayerSkills identity를 만든 뒤 combat timeline을 같은 변경에서 추가하고 publisher full
graph를 통과하는 순서로 바꾼다. Animation Tool의 combo stage count도 combat timeline source라고 명시한다.

### 종료 증거

- docs, MainApp, UI help, audit가 같은 key set을 말한다.
- docs, Client catalog/tool, publisher, Server가 같은 timeline ownership을 말한다.
- F7이 level/tool state를 바꾸는 코드가 없음을 audit한다.
- Release source/build contract에 F7 polling이 없음을 확인한다.

## G8. 4직업 standalone mesh hit trace admission

### 신규 파일

- `Data/Combat/Reference/FourClassStandaloneMesh.hittrace.json`
- `Tools/GameplayPipeline/Test-FourClassCombatHitTrace.ps1`

### effect 세션 merge 뒤 최소 변경

- effect correction/materialization manifest와 Effect JSON에는 gameplay `hitEventId`를 추가하지 않는다.
  `FourClassStandaloneMesh.hittrace.json`만 combat timeline ID와 effect occurrence/carrier를 연결하는 단방향 join
  owner다.
- DimensionMaster 2050210의 네 occurrence를 네 reviewed hit row와 연결하되 `ONCE_PER_STAGE` 총량을 유지한다.
- Lance Master 34010, Artist 31000, Warlord 17000은 각 combo stage가 admitted product cue와 carrier를 가진 뒤
  legacy migration range를 reviewed capsule로 교체한다. 현재 `BLOCKED` source record는 foundation 실패가 아니라
  admission 미완료 상태로 기록하고 link는 만들지 않으며 gameplay는 `LEGACY_XZ_RANGE`를 유지한다.
- carrier 하나의 bounds를 offline seed로 사용할 수는 있지만 local capsule 수치는 review 후 combat timeline에
  저장한다.
- sprite, echo, afterimage, flow, rim layer를 별도 damage collider로 승격하지 않는다.
- effect element lifetime을 collision end time으로 자동 복사하지 않는다.

### 종료 증거

- default trace audit은 DimensionMaster의 admitted source, skillbinding→clip→unique EFFECT cue, 네 carrier link와
  세 class의 명시적 `BLOCKED` source를 검증하며 Server foundation을 막지 않는다.
- effect 복원이 실제 admitted된 class마다 timeline+hittrace를 같은 commit에서 갱신하고
  `Test-FourClassCombatHitTrace.ps1 -RequireAllAdmitted`를 실행한다.
- `ADMITTED` source/link의 binding/animevent/effect hash 또는 cue payload/start/anchor/follow drift, missing carrier,
  decorative carrier는 audit 실패한다. `BLOCKED` source를 reviewed capsule 근거로 사용해도 실패한다.
- Effect runtime code는 Server collision dependency를 갖지 않는다.

## G9. Harness와 audit

### Shared geometry

- capsule/capsule overlap, tangent, epsilon miss
- parallel/skew/degenerate segment
- yaw transform과 vertical body build
- invalid finite/radius/length rejection
- zero-skipping tick distance: MAX-1→MAX=1, MAX-1→1=2, MAX→1=1, backward/zero rejection

### Publisher/catalog

- wrong version, unknown ID, duplicate timeline/event
- stage gap/count mismatch, invalid window/repeat, missing damage/body
- all-skill coverage와 old field absence
- temp/promotion failure rollback
- Server bootstrap/Client debug `combatRuntimeRevision` equality
- runtime-set orchestrator `-Mode Validate` destination non-mutation과 every-promotion-index failure injection
- Balance Tool player/boss capsule reload-save-reload와 partial/extra/NaN/zero staged rollback
- runtime manifest exact inventory/hash/path 검증, old manifest + partially promoted artifact의 pre-listener start failure
- Bern/Training spawn artifact absent 허용, Valtan/Character Select absent 거부, Bern/Training unexpected artifact는
  revision과 무관하게 거부

### Server gameplay

- `LEGACY_XZ_RANGE` current migration parity including tall Valtan/Lugaru and same-distance ID tie-break
- hit-time conversion 0/33/34/300/1500/1600/1700/2200 ms, frozen pre-admission 118-stage
  `optional<firstFireOffset> + damagePolicy + eventCount` golden, final legacy-stage parity, reviewed allowlist capsule
  golden, HOLD/COUNTER guard `NONE`, duration boundary, stage-transition 0 ms, zero-skipping tick wrap
- active window enter/leave
- root-motion 후 window-open anchor
- monster/Valtan hit/miss/tangent/vertical separation
- deterministic multi-target/maxTargets
- multi-tick repeat에서 기존 1명 + 신규 2명, cap 2일 때 신규 1명만 추가되고 cap을 넘지 않음
- per-target/per-repeat exactly-once
- `ONCE_PER_STAGE` total damage preservation
- combo/counter/hold stage reset
- monster chase/attack reach `fCollisionRadius -> CombatBodyCapsule.fRadius` parity
- dead/NPC/self/despawn exclusion
- damage event cap에서도 HP authority 유지

### Client frontend

- F7 focus/edge/hold/off
- Debug-only/Release absence
- entity spawn/snapshot/despawn join
- active window frame build
- one-tick active yellow + six-tick dim-orange ghost distinction
- future actionStartTick presentation clamp and F7 pending suppression
- active/ghost expiry의 Shared zero-skipping tick-distance parity
- equal snapshot reject, MAX→1 forward snapshot accept의 replication ordering parity
- buffered combo hit→future next stage snapshot과 final-tick hit→NONE snapshot은 old event ghost를 남기고,
  death/early cancel은 false ghost를 남기지 않음
- disconnect/level cleanup
- invalid debug catalog fail-closed
- enter-approved/local runtime revision match/mismatch/reset

### Network protocol

- `S2C_ENTER_ACCEPTED.strCombatRuntimeRevision` 64-hex roundtrip
- empty/non-hex/wrong-length/truncated/trailing rejection with destination object unchanged
- `C2S_USE_SKILL` remains intent-only and has no target/hit/damage/collider field

### ProjectAudit

- F1/F6/F7 key contract
- Data/Combat six-class complete graph
- all actor body capsule coverage
- Client damage/target packet 금지
- effect JSON에 gameplay collider field 금지
- project/filter registration
- `BalanceRuntimeSetManifest` project/filter 및 `bcrypt.lib` dependency 보존
- default four-class trace gate allows declared `BLOCKED`; strict admission gate is a separate explicit invocation

## 7. 충돌 회피 적용 순서

1. clean sibling worktree에서 Shared/player foundation을 먼저 구현하되 아직 final v5/v7/v13 artifact를 product publish하거나
   독립 schema commit으로 만들지 않는다.
2. 같은 branch에서 Valtan v4, reaction, navigation-bound runtime set과 Character Select activation까지 final schema를
   완성한다.
3. protocol/Server contract가 닫힌 뒤 F7 debug, Character Select controls/HUD와 presentation bridge를 연결한다.
4. effect restoration merge 뒤 hittrace join과 reviewed representative capsule admission을 넣고 `(2050210,0)` 네 event와
   final allowlist golden을 실행한다.
5. Detail PLAN의 final G order 전체가 Debug/Release/harness를 통과한 한 검증 단위로 PLAN/RESULT/code/data를 commit한다.

각 단계는 local checkpoint와 harness로 검증하지만 중간 단계가 같은 version number의 다른 의미를 publish/commit하지
않는다. 현재 dirty effect 세션 파일에 계획 단계에서 patch를 얹지 않는다. `Character.cpp`에는 새 damage 판정 코드를 넣지 않으며, `CEffectPlayback`,
`CEffectObject`, `CEffectDocumentRenderer`에도 Server collider ownership을 넣지 않는다.

## 8. 실패 처리

| 실패 | 처리 |
|---|---|
| combat timeline parse/schema 실패 | publish 실패, 기존 runtime 보존 |
| body capsule 누락/invalid | Server 초기 load 또는 해당 actor spawn 실패 |
| skill timeline 누락 | Server 초기 load 실패, legacy range fallback 없음 |
| Client debug catalog 누락/invalid | F7 unavailable, gameplay 정상 |
| accepted Server/local debug revision 불일치 | F7 unavailable + status, gameplay 정상 |
| Server/Client generated `combatRuntimeRevision` 불일치 | publish/audit 실패, runtime set 교체 금지 |
| effect trace missing/blocked | reviewed collider admission 거부, migration `LEGACY_XZ_RANGE` 유지 |
| damage event output cap | HP/death 권위 유지, overflow diagnostic |
| action cancel/death/leave | active repeat/ledger 즉시 reset |
| duplicate snapshot/despawn | stable ID 규칙에 따라 idempotent 처리 또는 명시 실패 |

## 9. 검증 명령

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-BalanceRuntimeSet.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Test-CombatTimelinePublisher.ps1
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Test-BalanceRuntimeSetRollback.ps1
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Test-FourClassCombatHitTrace.ps1
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
Server/Bin/Server.exe --contract-test
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

정본 regression 안에서 Shared/NetworkProtocolHarness/Server/Client/ClientFrontendHarness를 모두 실행한다.
`S2C_ENTER_ACCEPTED` revision field를 검증하면서 `C2S_USE_SKILL`이 intent-only로 유지되는 roundtrip test도 실행한다.

Runtime smoke는 다음 순서로 한다.

1. Lobby에서 Character Select Server Play로 진입한다.
2. 네 representative class를 각각 선택한다.
3. F7 off/on/hold 동작과 player capsule을 확인한다.
4. Valtan Arena에서 일반 monster와 Valtan hurt capsule을 확인한다.
5. skill cast log의 `hitEventId/startTick/endTick/targetId`와 `DAMAGE_EVENT`를 대조한다.
6. Character Select Server Arena에서 Monsters, Valtan, All을 각각 새 room generation으로 실행해 map-authored 위치와
   same-tick atomic spawn을 structured entity log로 확인한다.
7. monster IDLE/CHASE/ATTACK과 player→monster, Valtan→player forced-motion start/end/collision-stop log를 확인한다.
8. Valtan pending/active attack volume과 HUD `x160→x130→x0` state를 structured snapshot으로 확인한다.
9. F6 free camera 중 gameplay command가 차단된 상태에서도 F7 overlay는 read-only로만 동작하는지 확인한다.
10. disconnect와 level 복귀 뒤 stale capsule/activation/HUD state가 0개인지 diagnostic counter로 확인한다.

자동 screenshot, 이미지 비교, 이미지 판독은 수행하지 않는다.

## 10. 이번 범위에서 제외

- actor capsule끼리의 이동 blocking, push, crowd separation
- static wall/trigger OBB를 capsule movement로 교체
- projectile runtime과 lag compensation
- bone별 hurt box, frame별 skinned mesh triangle collision
- Client PhysX overlap을 Server hit로 전송
- Valtan pattern과 일반 monster 공격 외의 projectile/환경 공격을 capsule-aware outgoing 공격으로 전면 교체
- effect size 변경 시 gameplay capsule 자동 변경
- blocked Imported effect에서 자동 collider 생성
- Release collider overlay

## 11. 계획 단계 완료 조건

- 이 문서와 대응 상세 코드 계획서만 새로 작성한다.
- 기존 dirty source/data/project/audit 파일은 수정하지 않는다.
- 독립 비평으로 Server authority, all-skill migration, damage 중복, F7 public key migration, effect session 충돌을
  다시 확인한다.
- 실제 구현 전에는 별도 worktree/새 브랜치와 clean baseline을 확보한다.

## 12. 추가 범위의 최종 방향

이번 확장은 다음 네 기능을 같은 Server-authoritative combat vertical slice에 포함한다.

1. Valtan 공격 animation stage에 맞춘 무기, 장판, 돌진 전방, 향후 발판 hit volume
2. player skill에 맞은 일반 monster와 Valtan 공격에 맞은 player의 deterministic knockback/hit reaction
3. Character Select Arena의 Map Tool 배치와 Server-authoritative monster/Valtan 즉시 소환 버튼
4. Valtan spawn부터 death/despawn까지의 lifecycle과 상단 boss HP `x현재 줄` 표시

현재 `ValtanEncounter.json`은 이미 Server pattern timing, CIRCLE/RING/CONE/BOX/CROSS hit shape와 damage를 소유하고,
Balance Tool이 그 문서를 편집한다. Map Tool은 이미 boss placement와 SpawnGroups anchor/wave를 편집한다. Character
Select Server Arena에는 Server가 disabled placement ID를 검증하는 `Summon Valtan` 버튼이 있고, Server는 이미
`std::vector<SERVER_WORLD_ENTITY>`와 `CMonsterBrain`으로 monster를 보관하고 fixed tick update한다. 이 경로들을
확장하며 같은 역할의 `vector<CMonster>` 또는 Client local monster runtime을 추가하지 않는다.

## 13. Tool과 데이터 소유권 결정

| 관심사 | 편집 소유자 | 정본 | 다른 Tool의 역할 |
|---|---|---|---|
| hit shape, active window, damage, reaction, pattern motion | Balance Tool | `Data/Encounters/Valtan/ValtanEncounter.json` v4와 `Data/Combat/ReactionProfiles.json` | Animation/Effect Tool은 read-only overlay |
| pattern/stage action ID → 실제 Valtan clip, cue ID/time | Animation Tool | `Data/Animation/Authored/Valtan/Valtan.patternbindings.json` v1 | cue는 stable Effect binding ID만 참조 |
| weapon bone trajectory 측정/seed | Animation Tool | `Data/Animation/Authored/Valtan/Valtan.weapontracks.json` v1 | gameplay 수치가 아니며 Balance Tool import 입력 |
| reviewed weapon hit proxy | Balance Tool | `Data/Combat/ValtanHitTracks.json` v1 | source track/hash receipt를 보존하고 Server는 이것만 소비 |
| effect resource와 visual local transform/trigger policy | Effect Tool/Model View | `Data/Effects/Bindings/Valtan.effectbindings.json` v1과 effect asset | gameplay shape/damage를 저장하지 않음 |
| boss placement, monster spawn anchor/group, manual activation set | Map Tool | Area `Gameplay.world.json` v5와 `SpawnGroups.world.json` v1 | Character Select UI는 published stable activation ID만 요청 |
| runtime collider 확인 | F7 | Server-approved debug mirror | read-only, authoring/save 기능 없음 |

따라서 실제 collider 수치 편집은 Balance Tool에 둔다. Animation Tool에 collider 크기와 damage를 넣지 않는다.
Animation Tool은 실제 clip과 socket timing을 알고 있으므로 `patternId/stageId/actionId -> clip`, cue ID/time과 socket
trajectory bake만 소유한다. Effect Tool은 cue가 참조하는 stable effect binding의 asset/anchor/local transform/follow/
stop/trigger policy를 단독 저장한다. 두 Tool이 같은 cue row를 덮어쓰지 않으며 Server hit volume의 정본이 되지 않는다.
F7은 실행 결과 검증 전용이다.

### 13.1 Anchor 정책

Valtan runtime Server에는 skeleton이나 bone matrix가 없으므로 `b_wp_r_01`을 Server tick마다 직접 조회하지 않는다.
각 hit event는 다음 정책 중 정확히 하나를 사용한다.

- `OWNER_ROOT_AT_EVENT_OPEN`: swing, stomp처럼 event 시작 root/yaw를 snapshot한다.
- `OWNER_ROOT_FOLLOW`: charge 전방처럼 active tick마다 현재 Valtan root/yaw를 따른다.
- `WORLD_POINT_AT_EVENT_OPEN`: 장판/발판처럼 생성 순간 world point에 고정한다.
- `TARGET_POINT_AT_EVENT_OPEN`: target 위치를 snapshot한 뒤 장판을 고정한다.
- `AUTHORED_ROOT_LOCAL_TRACK`: Animation Tool의 측정 seed를 Balance Tool에서 승인한 root-local gameplay track을
  Server가 tick으로 재생한다.

Valtan 오른손 무기는 현재 `b_wp_r_01` socket에 붙는다. Animation Tool은 이를 clip 위에서 root-local로 측정해
`Valtan.weapontracks.json` seed를 만든다. 정확한 도끼 궤적이 필요한 stage만 Balance Tool이 finite sample을 검토해
`ValtanHitTracks.json`의 `AUTHORED_ROOT_LOCAL_TRACK`으로 승인한다. canonical per-clip hash를 새로 발명하지 않고 source
model content hash, clip ID, binding document hash, sequence/startOffset/playRate/sampleHz와 bone tuple 중 하나라도
달라지면 publisher가 admission을 거부한다. 기본 pattern은 root-local primitive로 먼저 닫는다.

### 13.2 Pattern별 권장 형태

| 조건 | Anchor | Shape | 비고 |
|---|---|---|---|
| 도끼 휘두르기 | root-open 또는 reviewed weapon track | capsule/cone | 여러 visual layer가 있어도 stable hit event 하나 |
| 지면 강타/장판 | world-open 또는 target-open | circle/ring/cross | effect가 끝나도 Server window는 독립 |
| 돌진 | owner-follow | forward capsule/OBB | Server pattern motion 뒤 현재 root를 매 tick 사용 |
| 향후 발판 | world-open | OBB/circle | 발판 visual 미구현 상태에서는 gameplay admission도 BLOCKED |

## 14. Valtan pattern timeline 확장

`ValtanEncounter.json`을 formatVersion 4로 올리고 stage의 단일 legacy shape 필드를 `hitEvents[]`와 `motion`으로
정규화한다. 현재 모든 pattern은 converter가 기존 CIRCLE/RING/CONE/BOX/CROSS, hit count/interval, damage 결과를
무손실 event로 바꾼다. v3 fallback reader는 남기지 않는다.

각 boss hit event는 다음 stable contract를 가진다.

- `hitEventId`, `anchorCaptureMs`, half-open `startMs/endMs`
- `anchorPolicy`, root-local `anchorLocalOffset`; track ID는 `CAPSULE_TRACK` shape union만 소유
- discriminated `shape`, `targetExtentPolicy`
- explicit pulse마다 event 하나, `maximumTargets`, v4 first cut `maximumHitsPerTarget=1`
- `damageProfileId`, optional `reactionProfileId`
- audit-only `presentationTraceId`

`anchorCaptureMs <= startMs`로 두어 장판 telegraph가 damage window보다 먼저 Server world anchor를 확정할 수 있게 한다.
v3 migration은 `anchorCaptureMs=startMs`, `targetExtentPolicy=LEGACY_ROOT_POINT_XZ`로 기존 target set과 damage를
보존한다. reviewed stage만 `HURT_CAPSULE`로 승격한다. Server는 pending/active occurrence의 stable ID, captured world
anchor/yaw와 tick range를 bounded world snapshot으로 복제하고 Client effect/F7은 이를 다시 계산하지 않는다.

stage `motion`은 first cut에서 `NONE`, `FORWARD_DISTANCE`, `TARGET_SNAPSHOT_DISTANCE` 중 하나다.
charge는 Server가 navigation/static collision을 검사하며 이동하고, `OWNER_ROOT_FOLLOW` hit volume이 이동한 root를
따른다. motion과 hit window가 서로 다른 float elapsed를 갖지 않도록 같은 zero-skipping tick age를 사용한다.

Animation Tool은 `Valtan.patternbindings.json`에서 모든 Server stage action ID를 실제 clip에 exactly one으로
연결한다. 현재 `BossCatalog.json`의 generic windup/active/recovery clip은 migration seed로만 사용하고, final runtime
presentation은 snapshot `strActionId/iActionStartTick`으로 binding을 resolve한다. clip이 없거나 hash가 다르면 Server
combat은 유지하고 그 Client action presentation만 격리한다.

## 15. Knockback과 hit reaction

PhysX rigid body를 gameplay transform authority로 사용하지 않는다. Server가 Engine/PhysX를 링크하지 않고, Client
PhysX 결과를 Server에 보내면 두 번째 collision truth와 network divergence가 생기기 때문이다. 이번 기능에서
“physics”는 Server fixed tick의 kinematic displacement, swept world collision, navigation projection과 Client
hit-reaction interpolation을 뜻한다. PhysX는 추후 ragdoll/debris/camera secondary presentation에만 사용할 수 있다.

### 15.1 Reaction profile

`Data/Combat/ReactionProfiles.json` formatVersion 1을 추가한다.

- `reactionProfileId`
- `reactionKind`: `STAGGER`, `KNOCKBACK`, `KNOCKDOWN`
- `directionPolicy`: `SOURCE_TO_TARGET`, `ATTACK_FORWARD`, `HIT_VOLUME_FORWARD`
- `distance`, `durationMs`, presentation-only `presentationArcHeight`
- `interruptPolicy`: 첫 cut은 `CANCEL_ACTION`
- `blockedPolicy`: 첫 cut은 `STOP_AT_FIRST_BLOCK`
- `presentationReactionId`

player/monster/boss profile에는 `receivedKnockbackScale`과 `reactionImmunity`를 명시한다. player와 세 regular monster는
1.0, Lugaru는 PROJECT_TUNED 0.35, Valtan은 scale 0 + `ALL` reaction immunity를 초기값으로 둔다. Valtan은 피해/피격
flash는 받되 root가 밀리거나 pattern/action이 cancel되지 않는다. pure legacy migration parity를 먼저 고정한 뒤 final
PROJECT_TUNED overlay에서 모든 damage-bearing player event→`reaction.monster.player-skill.light`, 네 monster basic
attack→`reaction.player.monster.basic`, Valtan legacy heavy→`reaction.player.valtan.heavy`, reviewed charge→
`reaction.player.valtan.charge`를 명시적으로 연결한다. 이 네 profile과 stable monster attack hit ID는 Detail G18의 exact
값을 사용하고 provenance receipt/harness로 빠짐없이 검증한다.

### 15.2 Server forced motion

player와 world entity runtime에 `SERVER_FORCED_MOTION` 하나를 둔다. 시작/목표 위치, start tick/bounded duration,
direction, reaction profile/presentation ID, kind/sequence와 active flag를 소유한다. hit 적용 순서는 damage/death 결정 뒤 reaction admission, 현재 action
cancel, forced motion 시작 순이다. death가 reaction보다 우선한다.

- player skill → 일반 monster: skill hit event의 `reactionProfileId`로 뒤로 민다.
- Valtan/monster attack → player: boss/monster attack hit event의 `reactionProfileId`로 민다.
- 같은 tick 여러 reaction은 `KNOCKDOWN > KNOCKBACK > STAGGER`, 그 뒤 source NetEntityId와 hitEventId로 결정한다.
- forced motion pending/active 동안 player move/skill input, trigger move Begin/Update와 monster AI action/path 평가를
  보류한다. admitted reaction은 기존 `TRIGGER_MOVE` payload/action을 먼저 원자 clear한다.
- command queue가 fixed update보다 먼저 실행되므로 Move는 valid sequence를 기록한 뒤 path/goal mutation 전에, UseSkill은
  공용 preflight helper로 strict skill sequence만 기록한 뒤 resource/cooldown/action mutation 전에 active `ForcedMotion`을
  거부한다. Character Select debug resource refill도 이 guard 뒤에 둔다. Release는 Client USE/RELEASE 공통 action
  sequence와 같은 `iLastSkillSequence`를 기록한 뒤 canceled hold state를 바꾸지 않는 no-op으로 만든다.
- 매 tick swept static collision과 navigation projection을 거쳐 blocked 지점에서 멈춘다.
- actor끼리 밀고 겹침을 푸는 crowd solver는 이번 범위가 아니다.

Shared snapshot에는 reaction kind/sequence/start tick/duration tick count와 normalized XZ direction을 추가하고 position은 기존
authoritative snapshot을 그대로 사용한다. Client는 snapshot 위치를 보간하고 hit reaction clip만 재생한다. 로컬
impulse 적분으로 transform을 앞서 움직이지 않는다. `presentationArcHeight`는 root/navigation을 띄우지 않고 presentation
offset에만 사용한다.

## 16. Character Select Arena 배치와 즉시 Server spawn

### 16.1 Map Tool authoring

Map Tool은 제품 Character Select 화면에서 직접 열리는 것이 아니라 Debug `LEVEL::DEVELOPMENT`의 Map Editor
workspace에서 실행된다. 그 workspace의 Area selector에서 `Character Select`를 선택하면 현재도 다음 작업이 가능하다.

- `boss.valtan.character-select.lazy` row 선택 후 position/yaw 편집과 저장
- Boss placement를 map surface pick으로 신규 배치
- SpawnGroups anchor를 map surface에 순서대로 배치
- group/wave/monster entry 저장

기존 placement를 map click 한 번으로 옮기기 쉽도록 `Relocate Selected Gameplay Placement` pick mode를 추가한다.
Valtan map의 `boss.valtan.center`와 Character Select의 Valtan placement 모두 같은 기능을 사용한다. 위치 정본은 Area
`Gameplay.world.json`이며 Client transform을 Server에 보내 저장하지 않는다.

Character Select에는 새 `SpawnGroups.world.json`을 만든다. 정확한 monster 위치가 필요하면 anchor 하나당 entry
`count=1`로 저작한다. 한 anchor의 count를 늘려 동일 좌표에 겹쳐 만들지 않는다. manual group은 single wave,
`startDelayMs=0`, 모든 `initialDelayMs=0`, `spawnIntervalMs=0`, `maxAlive=totalCount`를 publisher가 강제한다. 현재
`CSpawnGroupRuntime::Update`의 while loop는 이 계약이면 activation과 같은 Server tick에 bounded total을 전부
spawn한다.

### 16.2 Manual activation set

`Gameplay.world.json` formatVersion 5에 `manualActivationSets[]`를 추가한다. Character Select Area의 최소 set은
다음 세 개다.

- `arena.character-select.spawn.monsters`
- `arena.character-select.spawn.valtan`
- `arena.character-select.spawn.all`

각 set은 `spawnGroupIds[]`, disabled `bossPlacementIds[]`, `resetPolicy`를 가진다. Character Select audition set은
`RESET_WHEN_ROOM_EMPTY`를 사용한다. `spawn.all`은 monster group과 Valtan placement를 함께 참조한다. publisher는
same Area, known ID, duplicate, enabled boss, prerequisite, immediate-group 규칙, spawn anchor navigation/height와
baseline+동시 spawn entity 상한을 검증한다. Client-only MAP_LOAD_SCOPE/presentation coverage는 ProjectAudit와 full
Client Character Select Server Arena smoke가 검증한다. runtime world bootstrap v7은
`MANUALSET/MANUALGROUP/MANUALBOSS` row를 포함한다. v7은 아직
구현·publish되지 않은 계획 버전이므로 이번 변경을 포함한 하나의 final v7 schema로 처음 배포한다.

### 16.3 Typed Server command

기존 Valtan 전용 `C2S_SPAWN_WORLD_ENTITY`를 범용 `C2S_ACTIVATE_WORLD_SET`으로 교체한다.

```text
Client ImGui button
  -> IWorldActivationCommandSink::Request_ActivateWorldSet(setId)
  -> C2S_ACTIVATE_WORLD_SET(sequence, setId)
  -> Server room/session/world/sequence/set validation
  -> referenced boss placements와 immediate spawn group의 모든 spawn intent 전체 preflight
  -> nav projection, profile, entity/snapshot cap, NetEntityId headroom을 포함한 staged entity vector 생성
  -> 한 room command에서 boss/monster entity, resource ledger와 group state를 원자적으로 commit
  -> S2C_WORLD_SET_ACTIVATION_RESULT + entity spawn broadcasts
```

현재 `CSpawnGroupRuntime::Activate` 뒤 `Update`에서 실제 spawn이 실패할 수 있으므로 bool state만 먼저 바꾸지 않는다.
`Stage_ImmediateActivation`이 모든 monster의 projected transform과 runtime entity를 non-mutating 상태로 만들고,
성공한 경우에만 `Commit_ImmediateActivation`이 group state와 `m_WorldEntities`를 함께 바꾼다. preflight 실패 시 boss도
group도 바꾸지 않는다. 세 set이 같은 boss/group resource를 공유하므로 per-set mutable ledger를 만들지 않는다. 각 referenced
placement/group을 `DORMANT`, `ACTIVE_OR_COMPLETED`로 분류하고 missing resource만 stage한다. 예를 들어 Monsters 뒤
All을 누르면 Valtan만 추가되고 monster는 중복 생성되지 않는다. 새 resource가 하나라도 commit되면 `ACTIVATED`,
전부 이미 active/completed면 `ALREADY_ACTIVE`, preflight 실패면 typed reject reason을 가진 `REJECTED`다. 동시에 여러 Client가 눌러도 Server
room queue, per-player strict request sequence와 `(roomGeneration, resourceKind, resourceId)` resource ledger로 한 번만
생성한다. local spawn fallback은 없다.

activation 결과는 requester result에만 머물지 않는다. room snapshot/join replay에 bounded activation state를 넣어
late join Client와 다른 Client도 버튼 상태를 동일하게 복원한다. 마지막 player가 room을 떠날 때
`RESET_WHEN_ROOM_EMPTY` resource가 만든 dynamic entity, group state와 resource ledger를 정리한다. entity origin은
activation set ID가 아니라 canonical boss placement ID 또는 spawnGroup ID와 room generation으로 기록해 overlapping
set도 정확히 정리한다. baseline actor, navigation/collision과 monotonically increasing NetEntityId는 유지한다.
다른 player가 남아 있으면 reset하지 않는다.

Character Select Server Arena ImGui에는 `Spawn Monsters`, `Spawn Valtan`, `Spawn All` 세 버튼을 둔다. 버튼 label과
stable set ID는 enter 승인 뒤 Server가 current room bootstrap에서 보낸 `S2C_WORLD_ACTIVATION_CATALOG`의 read-only
Client mirror에서 읽는다. 별도 Client-local world-control catalog/Data 파일은 만들지 않는다. 버튼은 presentation prototype 준비 여부를
권위 조건으로 삼지 않는다. 대신 publisher/ProjectAudit가 activation set의 모든 archetype에 Client presentation
catalog/resource가 존재하는지 검증한다. replication spawn을 받은 뒤 기존 lazy-load path가 monster/Valtan
presentation을 준비하며, presentation 실패가 Server activation을 rollback하지는 않는다.

## 17. Monster AI와 Server container

새 `CMonster` 제품 class나 `vector<CMonster>`를 만들지 않는다. Server `CGameRoom::m_WorldEntities`가 이미
`SERVER_WORLD_ENTITY`를 보유하고 `CMonsterBrain::Update`가 다음 상태를 구현한다.

```text
combat-eligible player 없음 또는 engage range 밖 -> IDLE
가장 가까운 combat-eligible player 있음 -> CHASE
attack range 안 -> PATTERN_WINDUP
windup 종료 -> PATTERN_ACTIVE + Server damage/reaction
active 종료 -> PATTERN_RECOVERY
recovery 종료 -> IDLE
HP 0 -> DEAD -> configured delay 뒤 despawn
```

이번 변경은 Character Select spawn group도 이 경로에 들어가게 하고 body capsule/reaction/forced-motion만 추가한다.
Client monster는 `CNpc` presentation을 계속 재사용하며 snapshot action에 따라 idle/chase/attack/dead clip을 고른다.
AI target, attack hit, HP와 transform은 전부 Server 소유다.

현재 monster target filter는 HP만 보고 Valtan filter는 `isCombatReady`와 DEAD까지 본다. 새 공용
`Is_CombatTargetEligible`는 `HP>0 && isCombatReady && action!=DEAD && same room`으로 통일하고 기존 Valtan 의미를
보존한다. helper는 `Server/Public/ServerPlayer.h`의 `LostArk::Server` inline 하나가 소유하고 Monster/Valtan Brain이 같은
선언을 소비한다. Character Select Server Arena player는 입장 직후 combat-ready이므로 spawn 직후 AI가 그대로
IDLE/CHASE/ATTACK을 수행한다. DEAD monster가 despawn delay 동안 group alive-count에는 남는 기존 wave scheduling은
보존하되 damage query와 F7 hurt collider에서는 즉시 제외한다.

## 18. Valtan lifecycle와 boss HUD

서로 모순될 수 있는 독립 `active`/`live` bool 두 개를 gameplay authority로 두지 않는다.

- Server present/active: 해당 boss `SERVER_WORLD_ENTITY`가 room container에 존재한다. 별도 중복 bool을 두지 않는다.
- Server alive: `currentHp > 0`이고 action이 `DEAD`가 아니다.
- pattern active: action이 `PATTERN_ACTIVE`다. phase와도 별개다.
- Client `CValtan`: 위 snapshot의 read-only presentation mirror만 가진다.
- HUD: `CCombatHUDViewModel`이 replicated boss NetEntityId/HP/action/max bar를 소비한다.

현재 `CCombatHUDViewModel`과 상단 text fallback은 이미 boss HP, maximum bars와 phase를 소비한다. 다음을 보완한다.

- `WORLD_ENTITY_SNAPSHOT`에 boss의 `iMaximumHealthBars`를 추가하고 non-boss는 0으로 고정
- Shared `Compute_HealthBarCount(currentHp, maximumHp, maximumBars)`로 Server/Client/Balance Tool의 중복 ceil 식 제거
- `HUD_BOSS_STATE`에 boss NetEntityId, `hasBoss`, `isAlive`, `isPatternActive`, `iCurrentHealthBar` 추가
- boss spawn/첫 snapshot에서 활성화
- death snapshot에서는 `x0`과 death 상태를 표시하고 despawn에서 즉시 clear
- boss가 없는 Character Select Preview에서는 표시하지 않음
- 표기는 `발탄  x<currentHealthBar>`로 하며 최대 160에서 시작해 실제 130줄이면 `x130`을 표시
- UI 담당자가 제품 boss bar slot을 추가하면 같은 view model을 소비하고 text fallback만 제거

`CValtan`에 필요한 flag는 `m_isNetworkPresent/m_isAlivePresentation/m_isPatternActivePresentation`처럼 presentation
mirror로만 추가하고 HUD나 Server 판정이 이를 읽지 않는다. `HUD_BOSS_STATE::isValid`를 alive 의미로 재사용하지
않는다. 현재 `Apply_WorldEntityDespawn`에서 boss view model을 지우지 않는 stale 가능성도
`Clear_Boss(netEntityId)`로 닫는다. DEAD Valtan은 현재 Server에서 자동 despawn되지 않으므로 room reset 전까지
`hasBoss=true/isAlive=false/x0`을 유지한다.

## 19. Detail PLAN G 번호 crosswalk와 확장 구현 순서

이 문서의 기존 high-level G0~G9는 Detail PLAN을 다음처럼 묶은 요약이다. 실제 파일 구현·handoff 번호는 Detail PLAN의
G00~G23을 정본으로 사용한다.

| Implementation 요약 | Detail PLAN 정본 |
|---|---|
| G0 baseline/worktree | final order 1~3 |
| G1 Shared geometry | G00 |
| G2 schema/migration/publisher | G01~G03 |
| G3 Server catalog/body | G03 |
| G4 hit runtime | G04~G05 |
| G5 Client debug model | G06~G07 |
| G6 renderer/F7 | G08~G10 |
| G7 public key | G14 |
| G8 effect trace | G11 |
| G9 harness/audit | G12~G15 |

### Detail G16. Boss combat authoring schema와 Balance Tool

변경 파일:

- `Data/Encounters/Valtan/ValtanEncounter.json`
- `Data/Combat/ReactionProfiles.json` 신규
- `Data/Combat/ValtanHitTracks.json` 신규
- `Client/Public/BalanceTool.h`
- `Client/Private/BalanceTool.cpp`
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`
- provenance receipt updater와 publisher harness

v3 pattern을 v4 `hitEvents[]/motion`으로 전환하고 shape, anchor, repeat, reaction, motion을 exact-property UI로 편집한다.
save는 parse → validate → stage → temporary write → reload → commit을 따른다.

### Detail G17. Valtan Animation/Effect binding과 anchor bake

신규/변경 파일:

- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json` 신규
- `Data/Animation/Authored/Valtan/Valtan.weapontracks.json` 신규
- `Data/Effects/Bindings/Valtan.effectbindings.json` 신규
- `Data/Combat/ValtanHitTracks.json` 신규
- `Client/Public/BossPatternPresentationDocument.h` 신규
- `Client/Private/BossPatternPresentationDocument.cpp` 신규
- `Client/Public/Animation_Tool.h`
- `Client/Private/Animation_Tool.cpp`
- effect 세션 merge 뒤 `Client/Public/Effect_DocumentRenderer.h`, `Client/Private/Effect_DocumentRenderer.cpp`
- Animation/Project audit harness

Animation Tool boss mode에서 pattern/stage action을 실제 clip에 연결하고 `root`, `b_wp_r_01` preview와 측정 seed를
제공한다. Balance Tool import/review만 root-local finite 30 Hz gameplay sample을 `ValtanHitTracks.json`에 commit한다.
Animation Tool에서 collider dimension과 damage는 read-only다.

### Detail G18. Server boss hit volume, motion과 forced displacement

신규/변경 파일:

- `Shared/Public/Gameplay/CombatReactionContract.h` 신규
- `Shared/Private/CombatReactionContract.cpp` 신규
- `Shared/Public/Gameplay/BossCombatContract.h` 신규
- `Shared/Private/BossCombatContract.cpp` 신규
- `Shared/Public/Gameplay/BossHealthContract.h` 신규
- `Server/Public/CombatDisplacementSystem.h` 신규
- `Server/Private/CombatDisplacementSystem.cpp` 신규
- `Server/Public/ServerPlayer.h`
- `Server/Public/ServerWorldEntity.h`
- `Server/Public/GameplayCatalog.h`
- `Server/Private/GameplayCatalog.cpp`
- `Server/Public/ValtanBrain.h`
- `Server/Private/ValtanBrain.cpp`
- `Server/Public/MonsterBrain.h`
- `Server/Private/MonsterBrain.cpp`
- `Server/Private/PlayerSkillSystem.cpp`
- `Server/Public/ServerCollisionSystem.h`
- `Server/Private/ServerCollisionSystem.cpp`
- `Shared/Default/Shared.vcxproj`와 `.filters`
- `Server/Default/Server.vcxproj`와 `.filters`

기존 boss shape math를 G1 Shared geometry와 hit-event ledger로 교체한다. damage와 reaction은 동일 exactly-once hit 결과에서
적용한다. Valtan/monster outgoing hit의 기존 `Try_Counter` gate는 damage/reaction 전에 보존한다. pattern motion과
forced displacement는 같은 tick helper를 사용하고 static collision/navigation을 통과한다.

### Detail G20. Character Select Map Tool data와 publisher

신규/변경 파일:

- `Data/Worlds/LV_LOBBY_CLASSSELECT_SL00/SpawnGroups.world.json` 신규
- `Data/Worlds/LV_LOBBY_CLASSSELECT_SL00/Gameplay.world.json`
- `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`
- `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`
- `Data/Worlds/LV_DEV_TRAINING_GROUND/Gameplay.world.json`
- `Client/Public/WorldGameplayDocument.h`
- `Client/Private/WorldGameplayDocument.cpp`
- `Client/Public/MapTool.h`
- `Client/Private/MapTool.cpp`
- `Client/Public/AreaAuthoringTransaction.h` 신규
- `Client/Private/AreaAuthoringTransaction.cpp` 신규
- `Client/Default/Client.vcxproj`와 `.filters`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`와 `.filters`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/WorldPipeline/Publish-WorldGameplay.ps1`
- `Tools/NavigationPipeline/Publish-ServerNavigation.ps1`

Map Tool에 selected gameplay placement relocation과 manual activation set editor를 추가한다. Character Select
monster anchor/entry와 Valtan 위치는 실제 map pick으로 저장하며 authored value를 계획서에서 임의 좌표로 정하지 않는다.
Gameplay, required/absent SpawnGroups, Area navigation source/paint/blocker를 `CAreaAuthoringTransaction`의 temp overlay,
strict reload, publisher callback, flushed journal, ordered replace와 old-set recovery 한 경로로 저장한다. 기존 개별 Save는 이
경로로 수렴하고 Bern/Training에는 empty SpawnGroups를 만들지 않는다. visual Map placements 또는 destruction/simulation
authoring까지 동시에 dirty인 `Save All/Continue`는 어떤 write도 하기 전에 중단해 서로 다른 transaction domain의 partial
save를 막는다. `Save World Events`/`Save Simulations`는 기존 destruction pair wrapper, visual placement는 dedicated Save로
reroute해 자기 dirty state를 Save All에 다시 넣지 않는다. pure transaction CPP를 Client와
ClientFrontendHarness 양쪽 project/filter에 등록해 mixed-dirty 성공, ABSENT 보존, 각 replace failure rollback과 incomplete
journal startup recovery를 temp root에서 검증한다. 두 project는 CNG용 `bcrypt.lib`를 기존 dependency 보존 방식으로
링크하고 failure injection macro는 ClientFrontendHarness Debug/Release에만 정의한다.
World/navigation publisher는 activation set이 참조하는 모든 anchor의 nav projection/height와 Shared world entity
snapshot 상한을 검사한다. Client map-scope/presentation coverage는 `CLevelRegistry` descriptor를 ProjectAudit와 full
Client Character Select Server Arena smoke가 검증하고 Server preflight는 Client-only `MAP_LOAD_SCOPE`를 읽지 않는다. required Valtan/
Character Select SpawnGroups source가 빠지면 publish 실패와 기존 runtime set 보존이다. Bern/Training처럼
expected-absent인 stale generated artifact만 orchestrator가 transactional delete/rollback한다.

### Detail G19/G21. World activation protocol과 Server room commit

신규/변경 파일:

- `Shared/Public/Network/PacketType.h`
- `Shared/Public/Network/PacketMessages.h`
- `Shared/Private/Network/PacketMessages.cpp`
- `Client/Public/WorldEntityCommandSink.h`
- `Client/Public/NetworkWorldEntityCommandSink.h`
- `Client/Private/NetworkWorldEntityCommandSink.cpp`
- `Client/Public/NetworkManager.h`
- `Client/Private/NetworkManager.cpp`
- `Server/Public/RoomCommand.h`
- `Server/Public/GameRoom.h`
- `Server/Private/GameRoom.cpp`
- `Server/Private/ServerApp.cpp`
- `Server/Public/WorldBootstrap.h`
- `Server/Private/WorldBootstrap.cpp`
- `Server/Public/SpawnGroupBootstrap.h`
- `Server/Private/SpawnGroupBootstrap.cpp`
- `Server/Public/SpawnGroupRuntime.h`
- `Server/Private/SpawnGroupRuntime.cpp`

Valtan-only request를 typed set activation으로 교체하고 old packet reader/sender를 남기지 않는다. request sequence,
bounded ID, exact result enum, duplicate/truncated/trailing packet과 transactional room preflight를 검증한다.

### Detail G22. Character Select button, Valtan presentation과 HUD

변경 파일:

- `Client/Public/Level_CharacterSelect.h`
- `Client/Private/Level_CharacterSelect.cpp`
- `Client/Public/ClientReplication.h`
- `Client/Private/ClientReplication.cpp`
- `Client/Public/Valtan.h`
- `Client/Private/Valtan.cpp`
- `Client/Public/Character.h`
- `Client/Private/Character.cpp`
- `Client/Public/Npc.h`
- `Client/Private/Npc.cpp`
- `Client/Public/CombatReactionPresentationCatalog.h` 신규
- `Client/Private/CombatReactionPresentationCatalog.cpp` 신규
- `Data/Animation/Authored/CombatReactions/CombatReactionBindings.json` 신규
- `Client/Public/CombatHUDViewModel.h`
- `Client/Private/CombatHUDViewModel.cpp`
- `Client/Public/HUDRuntimeView.h`
- `Client/Private/HUDRuntimeView.cpp`
- `Client/Private/MainApp.cpp`
- `Data/Actors/BossCatalog.json`

버튼은 activation result와 entity broadcasts를 분리해 처리한다. Valtan은 actionId binding으로 clip을 재생하고
reaction/lifecycle은 snapshot mirror만 사용한다. `CCharacter`와 `CNpc`는 snapshot reaction sequence와
presentationReactionId를 catalog에서 실제 model clip으로 resolve해 hit-reaction animation을 재생하되 root는 Server
snapshot만 따른다. Client replication은 validated spawn identity를 optional GameObject pointer보다 먼저 commit해 lazy
presentation 실패에도 HUD/snapshot/despawn authority를 유지한다. HUD는 boss despawn을 명시적으로 clear한다.

### Detail G17 continuation. Effect anchor bridge

effect 세션 merge 뒤 `CEffectPresentationService`의 owner를 `CCharacter` 전용 pointer에서 read-only
`IEffectAnchorProvider`로 일반화한다. Character와 Valtan이 root/bone anchor를 제공하고 explicit Server world anchor도
typed spawn descriptor로 받을 수 있게 한다. `Data/Animation/Authored/Valtan/ValtanCombatPresentation.trace.json`이
`pattern/stage/hitEventId`와 animation cue/effect asset을 단방향 join하고 timing/hash/anchor를 audit한다. effect spawn은
trace-linked combat cue의 Server occurrence phase edge에서만 발생한다. animation/action edge는 decorative cue 또는
preview/audit에만 쓰며 같은 combat effect를 이중 spawn하지 않는다. Effect runtime, animevents, materialization 파일은 gameplay
shape나 reaction을 소유하지 않는다. 현재 dirty effect 세션 파일은 clean baseline 확보 전 수정하지 않는다.

### Detail G23. Harness, audit와 regression

public 계약 변경 파일:

- `AGENTS.md`, `CLAUDE.md`
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`
- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`
- `.md/TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md`
- `.md/TEAM/BALANCE_TOOL_OWNER_HANDOFF.md`

문서에는 Balance/Animation/Effect/Map Tool 소유권, manual activation set, Server forced-motion, Valtan
present/alive/pattern-active/current-bar 분리를 반영한다. 날짜 로그를 public 문서에 복제하지 않는다.

- boss v3→v4 모든 pattern hit count/damage target parity
- migrated `LEGACY_ROOT_POINT_XZ` Valtan shape의 current target-set parity와 reviewed `HURT_CAPSULE` boundary
- weapon track hash drift, missing bone/clip, non-finite sample rejection
- root-open/follow/world-open/target-open anchor boundary
- anchor capture가 damage보다 빠른 pending occurrence와 captured world anchor snapshot roundtrip
- charge motion과 follow collider의 same-tick 위치
- player→monster와 Valtan→player knockback, wall stop, navigation edge, death priority
- simultaneous reaction deterministic winner, repeated tick no duplicate
- Character Select immediate group same-tick full spawn, stable order, maximum bound
- invalid/duplicate set, partial preflight, concurrent request, reconnect/idempotency
- Monsters→All, Valtan→All 순서에서 missing resource만 commit하고 duplicate entity 0
- last-player leave reset, late-join activation-state replay, room generation isolation
- immediate group의 nav-invalid anchor/entity cap/NetEntityId headroom에서 zero partial commit
- Bern/Training expected-absent stale spawn artifact 제거/rollback과 required source 누락 publish 거부
- Valtan/Character Select spawn artifact required manifest policy
- monster AI idle/chase/attack/dead in Character Select
- monster/Valtan 공통 combat-ready target filter와 DEAD corpse collider 즉시 제외
- boss spawn/snapshot/death/despawn HUD lifecycle, `x160→x130(phase 1)→x80(phase 2)→x0`
- maximum-health-bar snapshot과 Shared ceil helper의 Server/Client/Balance Tool parity
- action clip/effect failure가 Server combat과 spawn을 rollback하지 않음
- NetworkProtocolHarness Debug/Release, Server contract tests, ClientFrontendHarness, ProjectAudit

자동 screenshot, 이미지 비교와 이미지 판독은 계속 금지한다.

## 20. 추가 범위 실패 처리

| 실패 | 처리 |
|---|---|
| boss pattern v4 또는 reaction profile invalid | publish 실패, 기존 runtime set 보존 |
| presentation binding/anchor track hash drift | 해당 reviewed track/effect admission만 거부, Server root-based combat 유지 |
| activation set preflight 실패 | boss/group 모두 무변경, `REJECTED` |
| entity ID exhaustion 또는 nav-invalid anchor | room activation 실패, 부분 entity commit 금지 |
| Client monster/Valtan prototype 준비 실패 | Server entity/HP 유지, presentation diagnostic 후 reconnect 가능 |
| forced motion 중 wall/nav block | 마지막 안전 위치에서 종료, 다음 tick action 복귀 |
| reaction과 lethal damage 동시 발생 | death 우선, knockback 시작 금지 |
| boss despawn event 중복 | idempotent clear, HUD invalid |

## 21. 추가 제외 범위

- Client 또는 Server PhysX dynamic rigid-body가 player/monster transform을 소유하는 구조
- ragdoll, bone별 hit reaction, 넘어짐 뒤 기상 animation 전체
- actor capsule 상호 push/crowd solver와 monster끼리 길막
- 절벽 낙사, arena 밖 추락, grab/throw/airborne 상태
- 아직 없는 발판/장판 visual asset 자체 제작
- manual arena content reset/respawn 버튼과 completed ONCE group 재활성화
- UI 담당자의 최종 boss health-bar 이미지/layout 제작
