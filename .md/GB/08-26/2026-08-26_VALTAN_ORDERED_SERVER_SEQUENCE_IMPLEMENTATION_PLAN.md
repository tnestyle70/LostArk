# 발탄 벽 파괴 후 Phase 2 서버 애니메이션 시퀀스 구현 계획

## 1. 목표

발탄 일반 자동 전투를 벽 파괴 전환부터 시작해 어제 저작된 Phase 2 애니메이션 19개를 원본
`manualAuditions` 순서대로 한 번씩 재생하는 20-step 프로그램으로 고정한다.

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

`VALTAN_ARENA_BREAK_109`는 현재 Product의 중앙 이동, 외곽 벽 전환, Phase 2 포효 presentation과
`SET_GAMEPLAY_PHASE 2` event를 그대로 실행한다. Phase 3 후보
`VALTAN_SEQUENCE_WARP_JUMP_FOUR_HAND_TWOHAND_ROAR_ROAR_DEAD`는 넣지 않는다.

앞선 1페이즈 다섯 패턴은 자동 시퀀스에서 제외한다. 특히 `VALTAN_FLOOR_WIPE_130`의
`SECOND_SMASH`는 반경 100m, `damageRatePercent 100000` 전멸 판정이어서 일반 5,000~6,500 HP
플레이어를 죽이고 벽 파괴와 Phase 2 검증을 막는다. 정의와 stable-ID Debug audition은 개별 진단용으로
보존한다.

첫 벽 파괴 step은 즉시 시작한다. 이후 19개 연결부에는 이전 pattern이 끝난 뒤 1000ms 동안 기존
Server CHASE/navigation 경로로 가장 가까운 플레이어를 추적·정렬하는 연출 구간을 둔다. 이 구간은 다음
pattern의 선택 자격이 아니며 고정 순서나 HP·거리·source cooldown 조건을 되살리지 않는다.

## 2. 데이터 정본과 생성물

- `Data/Valtan/Valtan.gameplay.json`의 `decisionModel.scriptedSequence`가 sequence ID, 1회 재생 mode,
  `interStepPursuitMs: 1000`과 20개 pattern ID 순서를 소유한다.
- 두 managed selection set의 `VALTAN_DASH_CHARGE`도 `enabled: false`로 내려 기존 선택 풀에서도 자동
  선택되지 않게 한다. 정의와 강제 Debug audition은 보존한다.
- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`는 split source를 strict validate하고
  `Data/Encounters/Valtan/ValtanPatternRotations.json` format v4의 `scriptedSequence`로 투영한다.
- 기존 `rotations` 배열은 authoring/debug 호환 데이터다. Product sequence가 있는 일반 자동 전투에서는
  fallback으로 소비하지 않는다.
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`은 sequence root/step을 검증해 bootstrap v22의
  `PATTERNSEQUENCE`와 `PATTERNSEQUENCESTEP` row로 publish한다.

## 3. Server 실행 계약

- `CGameplayCatalog`은 encounter별 sequence를 `parse -> validate -> commit`하고, 누락된 pattern,
  중복/부분 ordinal, 잘못된 mode와 100~10000ms 범위 밖 pursuit 값을 거부한다.
- `CValtanBrain`은 일반 자동 전투에서 sequence를 intro, health-bar queue, weighted/legacy rotation보다 먼저
  소비한다. step cursor는 pattern의 모든 stage가 성공적으로 완료된 뒤에만 한 칸 진행한다.
- sequence는 `NORMAL`, `HEALTH_BAR`, `AUDITION_ONLY` 같은 기존 selection owner와 무관하게 stable ID로
  pattern을 실행한다. FLOOR_WIPE와 ARENA_BREAK의 기존 damage, invulnerability, motion, world event,
  phase transition 계약은 복제하지 않는다.
- sequence가 존재하는 동안 health-bar crossing queue를 만들지 않고 HP/bar/phase/armor/range/source
  cooldown/repeat/weight 조건을 조회하지 않는다.
- 성공한 step 뒤에는 30 fixed tick 동안 CHASE한 뒤 exact next ID를 실행한다. 마지막 step 뒤에는 추가
  pursuit 없이 terminal IDLE을 유지해 global weighted fallback과 돌진 재등장을 막는다.
- stable-ID audition과 entrance/health-bar Debug audition은 명시적 override로 기존 개별 점검 경로를 유지한다.

## 4. Phase 2 표현 경계

Phase 2 19개는 87개 Server stage와 동일한 Product presentation binding을 가진다. 현재 저작 범위는
animation-only이고 hit, motion, event, Effect, camera를 새로 추론하지 않는다. Server는 pattern/action/stage
clock을 권위 있게 진행하고 Client `CValtan`은 기존 presentation binding을 샘플링한다.

## 5. 검증

1. JSON parse, Python compile, PowerShell AST parse, `git diff --check`
2. Valtan V2 projector/validator와 pattern-tree focused harness
3. Gameplay publisher Validate/Publish 및 bootstrap v22 admission failure cases
4. Server contract에서 실제 5500 HP 플레이어를 두고 정확한 20개 시작 순서, 벽 파괴 후 phase 2,
   첫 연결부 30 CHASE tick, 플레이어 생존, P1/P3 0회, 실패 cursor 고정, 마지막 이후
   IDLE/terminal cursor/no fallback을 room tick으로 확인
5. Server Debug/Release build와 각 `Server.exe --contract-test`

Client/UI는 에이전트가 실행하지 않는다. 사용자는 Server 재시작 뒤 실제 아레나에서 중앙 이동·포효·벽 파괴
후 19개 Phase 2 animation 연결을 순서대로 육안 확인하며, 자동 검증은 visual PASS를 대신하지 않는다.
