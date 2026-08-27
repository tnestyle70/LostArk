# 발탄 벽 파괴 후 Phase 2 서버 애니메이션 시퀀스 구현 결과

## 1. 완료한 자동 재생 계약

일반 Server 전투에서 발탄은 벽 파괴 전환과 Phase 2 저작 애니메이션 19개를 다음 순서로 각각 한 번만
실행하고 마지막에는 `IDLE`을 유지한다.

```text
VALTAN_ARENA_BREAK_109
-> VALTAN_SEQUENCE_CENTER_SIX_PIZZA_CHARGE
-> VALTAN_SEQUENCE_ATTACK_WHIRLWIND
-> VALTAN_SEQUENCE_CHARGE
-> VALTAN_SEQUENCE_FOUR
-> VALTAN_SEQUENCE_ROAR_CHARGE
-> VALTAN_SEQUENCE_RUSH
-> VALTAN_SEQUENCE_THREE
-> VALTAN_SEQUENCE_JUMP_WHIRLWIND_ROAR_ROAR_CHARGE
-> VALTAN_SEQUENCE_FRONT_BACK_FRONT
-> VALTAN_SEQUENCE_WARP
-> VALTAN_SEQUENCE_TWOHAND
-> VALTAN_SEQUENCE_WHIRLWIND
-> VALTAN_SEQUENCE_CENTER_TRASH_RUSH_IF
-> VALTAN_SEQUENCE_RUSH_SUCCESS
-> VALTAN_SEQUENCE_RUSH_FAIL
-> VALTAN_SEQUENCE_RUSH_IF
-> VALTAN_SEQUENCE_CATCH_BREATH
-> VALTAN_SEQUENCE_COUNTER
-> VALTAN_SEQUENCE_CHARGE2
-> IDLE
```

이 경로는 HP, 체력줄, gameplay phase, armor, pattern range, engage distance, source cooldown,
repeat limit, selection weight와 기존 rotation membership을 자동 step 선택 조건으로 사용하지 않는다.
이전 step의 모든 stage가 성공적으로 끝나야 cursor가 한 칸 진행하며, 실패한 step은 건너뛰지 않고 reset
필요 상태로 닫힌다. 기존 health-bar queue, intro, weighted/legacy fallback은 일반 자동 재생에 끼어들지 않는다.

앞선 P1 다섯 패턴은 자동 프로그램에서 제외했다. 기존 순서에 있던 `VALTAN_FLOOR_WIPE_130`의
`SECOND_SMASH`가 반경 100m, `damageRatePercent 100000`의 전멸 판정이어서 실제 플레이어를 죽이고
벽 파괴 이후로 진행하지 못하기 때문이다. P1 정의와 개별 stable-ID audition은 보존한다. Phase 3 후보
`VALTAN_SEQUENCE_WARP_JUMP_FOUR_HAND_TWOHAND_ROAR_ROAR_DEAD`도 이 프로그램에는 없다.

## 2. 패턴 사이 추적 연출

첫 `ARENA_BREAK_109`는 플레이어가 전투 가능 상태이면 즉시 시작한다. 이후 19개 연결부는 이전 pattern의 authored
`RECOVERY`가 끝난 뒤 `interStepPursuitMs: 1000`, 즉 Server 30 fixed tick 동안 다음 pattern을 보류한다.
그동안 기존 `CHASE -> navigation path -> MoveAlongPath -> yaw 정렬` 경로가 가장 가까운 플레이어를 따라간다.
30 tick을 소모한 뒤 selector를 다시 돌리지 않고 정해진 다음 stable ID를 시작한다.

플레이어가 없을 때 countdown을 소모하지 않으므로 재등장 직후 공격이 튀어나오지 않는다.
마지막 `VALTAN_SEQUENCE_CHARGE2` 뒤에는 추가 추적을 시작하지 않고 바로 terminal `IDLE`을 유지한다.

## 3. 실제 재생 표현

첫 `VALTAN_ARENA_BREAK_109`는 authored center anchor `[156.03, 22.99751, -122.06]`로 이동한다.
TAKEOFF/DROP/IMPACT/HOLD는 `mesh_att_battle_12_01 -> 12_02 -> 12_03`, WIDE_REVEAL/RECOVERY는
`mesh_evt1_att_battle_5_01_start -> loop -> end`를 재생한다. IMPACT 진입에서 외곽 벽 event와
`SET_GAMEPLAY_PHASE 2`를 적용한다.

그 뒤 19개 `VALTAN_SEQUENCE_*`는 총 87개 Server stage이며, 각 저작 occurrence가
`Data/Animation/Authored/Valtan/Valtan.patternbindings.json`의 Product presentation binding과 정확히 연결된다.
현재 이 19개는 animation-only 저작이므로 새 hit, motion, event, Effect, camera는 발생시키지 않는다.
Server가 pattern/action/stage clock을 진행하고 Client `CValtan`이 같은 binding을 샘플링한다.

## 4. 데이터와 runtime 연결

- `Data/Valtan/Valtan.gameplay.json`: Arena Break + Phase 2 19개 exact sequence와 1000ms pursuit 정본
- `Data/Encounters/Valtan/ValtanPatternRotations.json`: format v4 Product projection
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`: bootstrap v22 sequence/pursuit publish
- `Server/Private/GameplayCatalog.cpp`: sequence row admission과 1000ms -> 30 tick compile
- `Server/Private/ValtanBrain.cpp`: exclusive sequence, completion cursor, pursuit gate, terminal IDLE
- `Client/Private/ValtanPatternTree.cpp`: source/Product sequence shape와 pursuit parity 검증
- `Server/Private/ServerGameplayContractTests.cpp`: 실제 5500 HP target의 phase 2 도달, 20-step exact order와 terminal 검증

P1과 Phase 3 pattern 정의, animation binding은 삭제하지 않고 자동 sequence membership만 제한했다.

## 5. 자동 검증

| 검증 | 결과 |
|---|---|
| Valtan V2 `PublishV2` / `ValidateV2` | 실행 예정 |
| focused V2 master tests | 실행 예정 |
| pattern-tree contract | 실행 예정 |
| Gameplay balance Validate / Publish | 실행 예정 |
| bootstrap sequence | 실행 예정: v22 / 1000ms / exact 20 steps |
| Debug/Release Server build | 실행 예정 |
| Debug/Release `Server.exe --contract-test` | 실행 예정 |
| JSON parse / `git diff --check` | 실행 예정 |

## 6. 수동 화면 검증 경계

Client와 UI는 실행하지 않았다. 자동 검증은 animation/Effect의 최종 visual fidelity를 판정하지 않는다.
사용자는 새 Server process를 시작한 뒤 Valtan Arena에서 다음을 순서대로 확인해야 한다.

1. 입장 후 첫 자동 pattern이 중앙 이동·포효·벽 파괴 `ARENA_BREAK_109`인지 확인한다.
2. 벽 파괴 IMPACT 이후 Phase 2 19개가 문서의 exact order로 하나씩 재생되는지 확인한다.
3. 각 pattern 뒤 약 1초 동안 발탄이 플레이어를 따라 이동·회전한 뒤 다음 animation을 시작하는지 확인한다.
4. P1 패턴이나 Phase 3 후보가 중간에 끼어들지 않는지 확인한다.
5. 마지막 `VALTAN_SEQUENCE_CHARGE2` 뒤 다른 fallback 없이 멈추는지 확인한다.

눈으로 본 결과가 빠르거나 느리면 `interStepPursuitMs`만 조정하면 된다. 1차값은 1000ms이며, 추적 이동이
약하게 보일 때 다음 후보는 1200ms다.
