# LostArk 발탄 레이드 기준 자료와 JSON 튜닝 조사 결과

- 작성일: 2026-07-31
- 범위: 발탄 레이드 구조, 체력줄 기믹, 일반 패턴, 로컬 추출 애니메이션, JSON 튜닝 방향
- 현재 구현 범위: `ASSET_TEST`의 발탄 아레나와 `MN_RPBF_01` 보스
- 이번 작업: 조사와 설계만 수행한다. Navigation 작업과 겹치는 C++ 및 런타임 데이터는 수정하지 않는다.

## 결론

발탄 튜닝값을 JSON으로 분리하는 방향은 맞다. Winters에서 했던 것처럼 다음 계약을 그대로 가져오는 것이 가장 안전하다.

```text
정본 JSON
    -> Load
    -> Parse
    -> Validate
    -> Stage
    -> Commit
    -> Runtime은 검증된 구조체만 읽음

ImGui working copy
    -> Apply: 현재 실행에만 반영
    -> Save: JSON 정본 또는 draft 저장
    -> Reload: 디스크 값을 working copy로 다시 읽음
```

다만 공식 Lost Ark 문서는 세부 체력줄, 패턴별 피해량, hit frame을 공개하지 않는다. 따라서 자료를 다음 네 종류로 분리해야 한다.

| 종류 | 예 | 사용 원칙 |
|---|---|---|
| 공식 확정 | 8인, 2관문, Normal 1415, Hard 1445, 에스더 효과 | 기준 사실 |
| 공략 교차검증 | x130 전멸기, x88 한쪽 지형 파괴, x15 유령 전환 | `reference`로 저장하고 실제 영상으로 재확인 |
| 로컬 추출 확정 | 클립 이름, 원본 재생 시간, loop 여부 | 런타임 animation source truth |
| 우리 튜닝값 | 공격 거리, 선딜, hit window, 피해량, knockback | 실제 플레이를 보며 결정 |

첫 구현 기준은 **2022년 8인 Normal Gate 2**로 고정하는 것을 권장한다. Hard, Inferno, Solo, 이후 밸런스 패치는 서로 패턴과 CC 강도가 다르므로 한 JSON에 섞으면 안 된다.

## 공식 자료에서 확정되는 내용

Amazon Games의 공식 Valtan Academy는 다음을 확정한다.

- 발탄은 8인 Legion Raid다.
- 2개의 관문으로 구성된다.
- Normal 입장 아이템 레벨은 1415, Hard는 1445다.
- 발탄 레이드의 에스더는 Thirain, Wei, Balthorr다.
- Thirain은 큰 피해와 강한 Weak Point를 준다.
- Wei는 3회의 공격으로 큰 Stagger를 준다.
- Balthorr는 반경 14m 안의 파티원에게 Push 면역, 피해 감소, 일부 강력한 기술 차단 효과를 준다.

공식 문서는 의도적으로 세부 패턴과 체력줄을 공개하지 않는다고 명시한다. 따라서 아래 체력줄 표는 공식 수치가 아니라 공략 자료를 교차검증한 구현 참고값이다.

공식 Inferno 안내에는 중요한 예외도 있다.

- 다중 카운터 패턴은 고정 HP 패턴이 아니다.
- 파티 DPS에 따라 약 x90부터 x30 사이에 나타날 수 있고, 빠르면 아예 보지 않을 수 있다.

따라서 발탄 로직은 다음 두 종류를 분리해야 한다.

```text
Scripted mechanic
    HP threshold를 아래로 통과할 때 한 번 실행

Pattern pool
    현재 phase, 거리, cooldown, 이전 패턴, random weight로 선택
```

2024년부터는 Solo Mode도 추가되었고, 이후 공식 패치에서 발탄 Gate 1·2 피해량과 Gate 2 낙사/CC 조건이 완화되었다. Solo Gate 2의 공중 추적 후 내려찍기 시간도 별도 조정된 이력이 있다. 그러므로 최근 Solo 영상을 2022년 8인 Normal 기준으로 그대로 복사하면 안 된다.

## Gate 1 참고 타임라인

Gate 1은 현재 발탄 본체 아레나 구현 범위가 아니라 늑대 전투다. 향후 전체 레이드를 만들 때 참고할 수 있도록 분리 기록한다.

| 체력줄 | 기믹 | 분류 |
|---:|---|---|
| x46 | 보스 분리, Red/Blue Wolf 전환 시작 | HP scripted |
| x39 | Blue Wolf 소환, 표식 대상이 두 늑대를 분리 | HP scripted |
| x30 | 첫 Orb 순서 + Stagger | HP scripted |
| x24 | Red Wolf 재소환 | HP scripted |
| x15 | 두 번째 Orb 순서 + Stagger | HP scripted |
| x10 이후 | 가장 먼 대상 Fear 후 Stagger | HP 이후 조건부 |

Gate 1 일반 패턴은 별도 보스 정의가 필요하다.

- Red Wolf: Bleed, Backflip Slash, Roar, Blood Explosion, Slash and Spin, Charge Slash, Triple Claw, Berserker Claws, Red Tornado, Darkness, Line Attack
- Blue Wolf: Blue Slash, Green Mines, Forward Dash, Spinning Blades, Lava Eruption, Explosive Spin, Blue Explosion, Ice Explosions
- Purple Wolf: Tornado Blades, Black Circle

현재 `CValtan`에 섞지 않고 나중에 Gate 1 encounter와 늑대 actor를 따로 두는 것이 맞다.

## Gate 2 체력 페이즈 기준표

### 구현 기준

- 기준 모드: 2022 8-player Normal
- 시작 표시 체력: x160
- raw HP 참고값: Normal 약 1,074,523,848, Hard 약 1,991,561,183
- raw HP는 2022년 커뮤니티 자료이며 패치에 따라 바뀔 수 있으므로 런타임 정본으로 사용하지 않는다.
- 실제 구현은 `healthBars`와 정규화된 `hpRatio`를 사용한다.

### Body phase

| 체력줄 참고 | 정규화 비율 | 기믹 | 구현 분류 | 신뢰도와 주의점 |
|---:|---:|---|---|---|
| x160 | 1.0000 | 전투 시작, Armor 2중첩, 벽 충돌 후 Weak Point로 제거 | phase entry + condition | 여러 공략 일치 |
| x130 | 0.8125 | 바닥 2회 강타 전멸기, 보호 구슬 또는 Balthorr 대응 | HP scripted once | 여러 공략 일치 |
| x105~115 | 약 0.66~0.72 | 외곽 벽 파괴 전환, 4기둥과 cone/pizza | HP scripted once | 자료마다 x105, x110, 약 x115로 표기가 다름 |
| x88 전후 | 0.5500 | 한쪽 arena 붕괴, 기둥 안전지대와 표적 cone | HP scripted once | 자료에 따라 x85/x88/x90 |
| x64 전후 | 0.4000 | 중앙 이동, 표적 폭발 후 charge grab/counter | HP scripted once | 자료에 따라 x64/x65 |
| x34 전후 | 0.2125 | 반대쪽 arena 붕괴, 작은 platform만 남음 | HP scripted once | 자료에 따라 x30/x34/x35 |
| x15~16 | 약 0.094 | 4방향 강타, 추적 폭발, 안/밖, 기둥 후 Ghost 전환 | HP scripted once | 여러 공략이 x15 또는 x16 |

체력줄이 자료마다 1~10줄 정도 다른 이유는 공략 작성 시 표시값을 반올림하거나, 패턴 시작 애니메이션과 실제 HP lock 시점을 다르게 기록했기 때문이다. JSON에는 처음부터 확정값으로 박지 말고 `referenceBarApprox`와 우리가 영상을 보고 확정한 `triggerBar`를 분리한다.

### Ghost phase

Ghost는 Body 체력을 15에서 40으로 단순 회복시키는 것보다 **별도의 40줄 health pool**로 모델링하는 것이 명확하다.

| 항목 | 참고 동작 | 구현 방향 |
|---|---|---|
| 시작 체력 | Ghost x40 | `healthPoolId = GHOST_40`로 전환 |
| 위치 | 중앙 고정에 가까움 | 추적 이동을 끄고 전용 pattern pool 사용 |
| Armor | Normal 4중첩, Hard 6중첩 | difficulty profile로 분리 |
| Armor 제거 | 소환된 clone counter 성공 시 1중첩 제거 | counter event가 phase state를 변경 |
| Grab | 시작 직후, 약 x28, 약 x14라는 공략과 10줄마다라는 공략이 공존 | 실제 기준 영상을 보고 확정 |
| 주변 위험 | 가장자리로 돌진하는 clone과 knockback | arena edge hazard |
| 종료 | Ghost x0 | encounter clear |

현재 로컬에 cook된 것은 `MN_RPBF_01` 하나뿐이다. 원본 action 자료에는 `MN_RPBF_02-2`와 Ghost 관련 문자열이 있지만 대응 character model은 현재 cook되어 있지 않다. 그러므로 완전한 Ghost phase는 모델/애니메이션 추출과 cook을 별도 단계로 잡아야 한다.

## Gate 2 일반 공격과 조건부 패턴

아래 목록은 HP scripted mechanic과 분리된 pattern pool 후보이다.

### 기본 공격

| ID 후보 | 관찰 동작 | 핵심 튜닝값 |
|---|---|---|
| `AXE_SWIPE_SMASH_SPIN` | 좌우 axe swipe, 중앙 smash, spin과 확장 충격파 | 거리, 각 단계 hit frame, spin 반경 |
| `CROSS_SMASH` | axe를 들고 지면을 내려쳐 십자 안전/위험 구역 생성 | 선딜, 십자 폭, 잔류 시간 |
| `NORMAL_SPIN` | 원형 범위 회전 | 반경, 회전 횟수, knockback |
| `JUMP_AND_SPIN` | 한 대상을 향해 jump 후 cross, 조건부 이동 spin | target lock, jump 거리, 후속 확률 |
| `FOUR_SMASHES` | 처음 3회는 대상을 추적하고 4회째는 3회 위치에 고정 | 추적 갱신 시점, 강타 간격 |
| `IN_OUT_SMASH` | 안쪽 폭발 후 바깥 폭발, 개인 추적 폭발 동반 가능 | inner/outer timing |
| `THREE_DIRECTION_SMASH` | 짧은 jump 후 반대 방향으로 3회 cone smash | 방향 규칙, cone 각도 |
| `FRONT_BACK_FRONT` | 전방, 후방, 전방 순서 강타 | 각 구간 시간과 범위 |
| `SAFE_IN_OUT` | axe 발광 여부로 안/밖 안전 구역 결정 | telegraph 식별, 개인 폭발 지연 |

### 이동과 arena 위험 공격

| ID 후보 | 관찰 동작 | 핵심 튜닝값 |
|---|---|---|
| `PORTAL_RUSH` | 여러 portal을 통과하며 player 방향으로 돌진 | portal 수, 돌진 속도, knockback |
| `ANCHOR_RAIN` | arena 이탈 후 player 위치에 투사체를 떨구고 착지 | 투사체 수, 간격, 착지 safe zone |
| `DELAYED_EARTHQUAKE` | 표시 후 지연된 원/삼각 rock explosion | 표시 시간, 폭발 지연, knock-up |
| `NORMAL_GRAB_CHARGE` | 한 대상을 가리킨 뒤 전방으로 달려 잡아 날림 | target lock, counter 가능 여부, grab 폭 |

### 반응형 패턴

| ID 후보 | 관찰 동작 | 핵심 조건 |
|---|---|---|
| `PARRY_RETALIATE` | 방어 자세 중 공격/stagger를 받으면 큰 spin 반격 | 받은 hit/stagger event |
| `FOUR_ORBS_COUNTER` | 4개 orb를 처리하는 동안 Valtan counter | orb consume와 counter |
| `MULTI_COUNTER` | 방향을 바꾸며 여러 번 counter | HP 고정이 아닌 cooldown/pattern pool |
| `IMPRISON` | 일부 player를 가두고 남은 player가 해제 | party size와 남은 감금 수 |
| `SHIELD_DAMAGE_STAGGER` | shield를 damage로 제거한 뒤 stagger check | shield break 후 stagger phase |
| `SILENCE_ROAR` | 짧은 silence를 거는 roar | status application |

Normal과 Hard의 핵심 차이는 단순 피해량뿐 아니라 일부 공격의 반응이 stagger/push에서 knock-up으로 강화된다는 점이다. `damageScale` 하나로 난이도를 만들지 말고 `crowdControlProfile`도 분리해야 한다.

## 로컬 추출 데이터 확인 결과

### 현재 cook된 발탄 animation

경로:

```text
Client/Bin/Resources/LostArk/Character/MN_RPBF_01/anims
```

총 27개 animation 중 attack clip은 18개다.

| 묶음 | 클립과 원본 길이 |
|---|---|
| Battle 2 | `att_battle_2_01` 3.000초, `02` 1.200초, `03` 2.100초 |
| Battle 4 | `att_battle_4_01` 4.733초, `02` 2.000초 |
| Battle 7 | `att_battle_7_01` 6.067초, `02` 1.200초, `03` 2.000초 |
| Battle 19 | `01` 5.000초, `02` 1.500초, `03` 0.667초, `04` 2.600초, `05` 2.333초, `06` 0.333초 |
| Battle 20 | `01` 2.500초, `02` 1.333초, `03` 0.533초, `04` 1.467초 |

나머지 9개는 idle, run, turn, death, dance 계열이다.

중요한 확인 결과:

- 18개 attack `.wanim`의 `eventCount`는 전부 0이다.
- 즉 현재 cook된 animation에는 damage active frame, hitbox spawn, sound, counter window event가 들어 있지 않다.
- animation 길이는 확정할 수 있지만 실제 타격 시점은 영상 또는 원본 action notify를 분석해서 별도로 authoring해야 한다.
- `MN_RPBF_01-1.loa`에는 `CEFActionNotify_CounterAttack`, `Ghost`, Battle 2/4/7/19/20 clip, Valtan 공격 sound event 문자열이 존재한다.
- `.loa`는 바이너리 action 문서이므로 문자열 발견만으로 정확한 notify time을 추측하면 안 된다.

현재 코드 상태도 아직 boss combat이 아니다.

- `CValtan`은 `IDLE`과 `CHASE`만 가진다.
- `CBody_Valtan`은 idle animation을 시작하고 기존 state에서 animation을 재생한다.
- `CLoader`는 `MN_RPBF_01.wmodel` 하나만 `Prototype_Component_Model_Valtan`으로 등록한다.
- HP, stagger, attack pattern, hitbox, phase controller는 아직 없다.

## JSON 저장 구조

하나의 거대한 JSON보다 세 층을 권장한다.

```text
ValtanGate2.reference.json
    외부 자료와 로컬 실측값
    런타임에서는 읽지 않음

ValtanGate2.encounter.json
    검증된 phase, scripted mechanic, pattern 정의
    release 정본

ValtanGate2.local.tuning.json
    ImGui에서 실험하는 override
    개인 작업용, release 정본 아님
```

### 권장 schema 예시

```json
{
  "schema": "lostark.boss.encounter",
  "schemaVersion": 1,
  "encounterId": "VALTAN_GATE_2",
  "referenceProfile": {
    "mode": "NORMAL_8P_2022",
    "displayBars": 160,
    "referenceMaxHealth": 1074523848,
    "provenance": "community_2022",
    "runtimeUsesRawReferenceHealth": false
  },
  "healthPools": [
    {
      "id": "BODY_160",
      "displayBars": 160,
      "tuningMaxHealth": 100000.0
    },
    {
      "id": "GHOST_40",
      "displayBars": 40,
      "tuningMaxHealth": 25000.0
    }
  ],
  "scriptedMechanics": [
    {
      "id": "FLOOR_WIPE_130",
      "trigger": {
        "kind": "hpBarCrossedDown",
        "healthPool": "BODY_160",
        "triggerBar": 130,
        "referenceBarApprox": 130
      },
      "once": true,
      "patternId": "FLOOR_WIPE"
    },
    {
      "id": "GHOST_TRANSITION",
      "trigger": {
        "kind": "hpBarCrossedDown",
        "healthPool": "BODY_160",
        "triggerBar": 15,
        "referenceBarApprox": 15
      },
      "once": true,
      "nextHealthPool": "GHOST_40",
      "patternId": "GHOST_TRANSITION"
    }
  ],
  "patternPools": [
    {
      "id": "BODY_NORMAL_RANDOM",
      "phase": "BODY_NORMAL",
      "globalCooldownSeconds": 1.5,
      "entries": [
        {
          "patternId": "AXE_COMBO_BATTLE_2",
          "weight": 1.0,
          "minRange": 0.0,
          "maxRange": 4.0,
          "repeatLimit": 1
        }
      ]
    }
  ],
  "patterns": [
    {
      "id": "AXE_COMBO_BATTLE_2",
      "targetPolicy": "lockAtPatternStart",
      "movementPolicy": "stopNavFollower",
      "clips": [
        {
          "animation": "att_battle_2_01",
          "sourceDurationSeconds": 3.0,
          "playbackSpeed": 1.0,
          "hitWindows": []
        },
        {
          "animation": "att_battle_2_02",
          "sourceDurationSeconds": 1.2,
          "playbackSpeed": 1.0,
          "hitWindows": []
        },
        {
          "animation": "att_battle_2_03",
          "sourceDurationSeconds": 2.1,
          "playbackSpeed": 1.0,
          "hitWindows": []
        }
      ],
      "verification": {
        "animationIdentity": "local_extracted",
        "hitWindows": "unmeasured",
        "damage": "untuned"
      }
    }
  ]
}
```

예시의 `100000`, `25000`, cooldown은 실제 Lost Ark 수치가 아니라 우리 프로젝트에서 다루기 쉬운 튜닝값이다. `hitWindows`가 비어 있는 패턴은 damage 기능을 활성화하지 않고 preview만 허용해야 한다.

## Winters 방식에서 그대로 가져올 것

로컬 Winters의 실제 도구는 다음 특성을 갖는다.

- `practice_balance_overrides.json`은 정본 gameplay data를 덮어쓰는 designer scratch override다.
- `attack_speed_tuning.json`은 champion별 목표 공격 속도와 animation 배속을 저장한다.
- JSON은 nlohmann JSON으로 parse하고 version, 배열 타입, 숫자 유효성, 최대 개수 등을 검증한다.
- Load 결과는 draft에 들어가고 `Save`와 `Apply`는 분리된다.
- Save는 `.tmp`에 쓴 뒤 다시 parse/validate하고 정본으로 교체한다.
- visual timing draft는 release truth가 아니며 canonical JSON에 merge하고 codegen/build해야 한다고 UI에 명시한다.

LostArk 프로젝트에는 현재 공용 nlohmann/rapidjson dependency가 확인되지 않았다. `Effect_AssetIO`의 JSON은 구조화된 범용 parser가 아니라 `canonicalPayloadHex` 문자열을 직접 찾는 wrapper이므로 boss JSON에 재사용하면 안 된다.

따라서 구현 단계에서는 둘 중 하나를 먼저 결정해야 한다.

1. Client 전용으로 검증된 JSON library를 정식 dependency로 등록한다.
2. 기존 프로젝트 data 문서 형식을 사용하고, 나중에 JSON authoring/cook 단계를 붙인다.

사람이 읽고 수정하는 BossPattern 도구가 목표이므로 1번이 더 적합하다. 단, dependency 추가, include 경로, Debug/Release 빌드, 배포 계약을 계획서에 포함해야 한다.

## 실제 플레이를 보며 튜닝하는 순서

### 기준 영상 고정

영상마다 먼저 다음 메타데이터를 기록한다.

```text
mode: Normal / Hard / Inferno / Solo
partySize
patch 또는 영상 날짜
videoFPS
playbackSpeed
```

모드가 다른 영상의 값을 같은 패턴에 섞지 않는다.

### 패턴별 측정 항목

| 구간 | 측정값 |
|---|---|
| Tell | 자세 변화 시작부터 회피 판단이 가능한 순간 |
| Windup | pattern 시작부터 첫 타격 직전 |
| Active | 실제 피격 가능한 시작/종료 frame |
| Recovery | 마지막 타격부터 다음 판단 가능 시점 |
| Facing | 언제 target 방향을 고정하고 언제 다시 회전하는지 |
| Movement | root 이동 거리, 돌진 속도, 착지 위치 |
| Shape | circle, cone, box, cross의 크기와 offset |
| Reaction | damage, stagger, knockdown, knockback, grab |
| Phase | 어느 phase/pattern pool에서 등장하는지 |

60fps 영상이라면 `시간 = 측정 frame / 60`으로 시작하고, 원본 animation과 목표 영상 길이가 다르면 다음으로 배속을 계산한다.

```text
playbackSpeed = sourceAnimationDuration / targetObservedDuration
```

### 튜닝 우선순위

1. animation이 맞는지 확인한다.
2. 보스의 facing lock과 movement를 맞춘다.
3. telegraph와 hit window를 맞춘다.
4. hitbox 크기와 knockback을 맞춘다.
5. 마지막에 damage와 HP 소요 시간을 조절한다.

피해량부터 맞추면 애니메이션과 판정이 틀린 상태를 수치로 가리게 된다.

### ImGui 최소 패널

```text
[Boss Pattern]

Profile        Normal 8P 2022
Health Pool   BODY_160
Phase         BODY_NORMAL

Pattern       AXE_COMBO_BATTLE_2
Clip          att_battle_2_01
Source        3.000 s
Play Speed    [ 1.000 ]
Windup        [ unmeasured ]
Hit Windows   [ none ]
Recovery      [ unmeasured ]

[Preview Selected] [Stop]
[Apply] [Save Draft] [Reload]

Status: Draft only / Not release truth
```

처음부터 phase graph, sequencer, 전체 패턴 목록을 한 화면에 노출하지 않는다. 선택한 패턴 하나를 재생하고 시간을 조절하는 기능부터 닫는다.

## 권장 구현 순서

Navigation 세션이 끝난 뒤 다음 순서로 진행한다.

1. 기준 모드를 `NORMAL_8P_2022`로 고정한다.
2. `CValtan`에 HP를 바로 얹기 전에 encounter data와 runtime state의 소유권을 확정한다.
3. JSON library 또는 기존 data format dependency를 확정한다.
4. `AXE_COMBO_BATTLE_2` 하나만 encounter JSON에서 읽는다.
5. `att_battle_2_01 -> 02 -> 03` preview를 닫는다.
6. 영상으로 세 클립의 facing, hit window, recovery를 측정한다.
7. Apply/Save/Reload와 잘못된 JSON rollback을 검증한다.
8. 기본 pattern pool을 늘린다.
9. x130 scripted mechanic을 첫 체력 기믹으로 추가한다.
10. arena 파괴 trigger와 Nav blocker condition을 연결한다.
11. 마지막에 Ghost 전용 model/action extraction과 40줄 health pool을 추가한다.

이 순서라면 보스 전투, 지형 파괴, Navigation condition을 한 번에 얽지 않고 각 축을 독립 검증할 수 있다.

## 출처

### 공식

- [Lost Ark Academy - Valtan Legion Raid](https://www.playlostark.com/en-gb/news/articles/lost-ark-academy-valtan?language-picker=true)
- [Lost Ark Academy - Inferno Legion Raids](https://www.playlostark.com/en-gb/news/articles/lost-ark-academy-inferno-legion-raids)
- [Cool Retreat - Solo Mode](https://www.playlostark.com/en-us/game/releases/cool-retreat)
- [Welcome to Paradise - Solo Mode difficulty adjustments](https://www.playlostark.com/en-us/game/releases/welcome-to-paradise)
- [Rimeria Besieged - Valtan Solo Gate 2 adjustment](https://www.playlostark.com/en-us/game/releases/rimeria-besieged)

### 세부 기믹 교차검증

- [Mobalytics - Valtan Gate 1](https://mobalytics.gg/lost-ark/boss-guides/legion-raids/valtan-gate-1)
- [Mobalytics - Valtan Phase 2](https://mobalytics.gg/blog/lost-ark/lostark-valtan-phase-2-legion-raid-guide/)
- [Icy Veins - Valtan Gate 2](https://www.icy-veins.com/lost-ark/valtan-legion-raid-gate-2)
- [Dignitas - Valtan Gate 2](https://dignitas.gg/articles/entering-the-resurrected-demon-beast-lair-legion-raid-guide-valtan-gate-2)
- [KeenGamer - Valtan Phase 2](https://www.keengamer.com/articles/guides/valtan-phase-2-complete-guide-in-lost-ark/)

### 로컬 원본

- `Client/Bin/Resources/LostArk/SourceData/LPK/data3/EFGame_Extra/ClientData/XmlData/Action/MN_RPBF_01-1.loa`
- `Client/Bin/Resources/LostArk/SourceData/LPK/data3/EFGame_Extra/ClientData/XmlData/Action/MN_RPBF_02-2.loa`
- `Client/Bin/Resources/LostArk/Character/MN_RPBF_01/MN_RPBF_01.wmodel`
- `Client/Bin/Resources/LostArk/Character/MN_RPBF_01/anims/*.wanim`

