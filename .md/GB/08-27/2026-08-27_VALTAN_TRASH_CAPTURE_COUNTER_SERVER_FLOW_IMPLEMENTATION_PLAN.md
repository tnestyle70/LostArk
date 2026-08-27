# 발탄 버러지 돌진 포획·카운터·그로기 Server Flow 구현 계획

## 0. 결론

이번 후속 작업은 새 Valtan 전용 상태 머신을 만드는 작업이 아니다. 이미 반영된 아래 typed 경로를
그대로 사용한다.

```text
Pattern Stage hit
  -> Server BOX overlap / damage LANDED
  -> CAPTURE + BOSS_LEFT_HAND
  -> SERVER_PLAYER.GRABBED / isCombatReady=false
  -> Shared attachment snapshot
  -> Client left-hand presentation

Player counter skill hit
  -> COUNTERABLE + counter proxy 검사
  -> COUNTER_HIT outcome
  -> GROGGY stage ENTER
  -> RELEASE_GRABBED_PLAYERS
  -> PLAYER_ACTION_STATE::NONE / isCombatReady=true
```

현재 구현에는 이 기반이 모두 있다. 실제 후속 구현의 핵심은 다음 일곱 가지다.

1. 잡기 전에 열리는 현재 카운터 구간을 제거하고, **돌진 충돌·포획 뒤의 준비 동작**에 카운터를 연다.
2. 원본 notify가 없는 `mesh_att_battle_13_02-1`이 아니라, 실제 `COUNTER 0~200ms`가 있는
   `mesh_att_battle_13_05-1`에 카운터 window를 맞춘다.
3. 상태 이름만 GROGGY인 `mesh_att_battle_13_03`을 실제
   `mesh_abn_groggy_1_start -> loop -> end` presentation으로 교체한다.
4. 포획 성공 여부와 무관하게 다음 단계로 넘어가지 않도록, rush 종료 시점에 Server가
   committed attachment 기준의 `ANY_PLAYER_GRABBED`를 판정한다.
5. 현재 충돌 위치를 보존하는 left-hand parent를 실제 왼손 grip snap 의미로 교정하고,
   Client도 `GRABBED` snapshot 동안 명령 제출 자체를 막는다.
6. `VALTAN_TRASH_CATCH_SUCCESS/FAIL/IF`를 Product ordered sequence와 기본 Boss Tool Flow에서 빼고,
   그 clip/effect 역할을 `VALTAN_TRASH` 내부 분기로 사용한다.
7. 한 명 플레이 또는 모든 생존 플레이어가 잡힌 경우에는 카운터 주체가 없으므로, 기존
   `ALL_PLAYERS_GRABBED`를 안전 해제 terminal로 연결해 영구 구속을 막는다.

목표 Product 흐름은 다음 하나로 고정한다.

```text
도입/구속 준비
  -> 대상 재선정
  -> 돌진 + 이동하는 BOX 판정
     -> 아무도 못 잡음: rush miss(13_05-2) -> 대상 재선정 -> 재돌진
     -> 한 명 이상 잡음: 왼손 snap + 입력 잠금 -> 카운터 준비
        -> COUNTER_HIT: 실제 GROGGY + 같은 전이 tick에 전원 해제
        -> TIMEOUT: 13_05-1의 남은 잡기 동작을 끝까지 재생
           -> non-grabbed 생존자가 남음: 잡은 상태 유지 -> 재돌진 -> 다시 카운터 준비
           -> 전원 포획: 안전 해제 terminal -> 패턴 종료
```

이 계획은 문서만 작성한다. 현재 Product JSON, C++, Tool 또는 binary asset을 수정하지 않는다.
구현은 다른 세션 PR이 `main`에 반영된 뒤 별도 `codex/valtan-trash-capture-counter-flow` 브랜치에서
시작한다.

### 0.1 이번 종합의 근거

- `.md/GB/08-21/2026-08-21_VALTAN_PATTERN_MASTER_SPEC_PLAN.md`: source action/notify와 전체 pattern lineage
- `.md/GB/08-26/2026-08-26_VALTAN_PHASE_TWO_PATTERN_MECHANICS_IMPLEMENTATION_PLAN.md` 및 RESULT:
  현재 Product stage 승격과 Server pattern branch 기반
- `.md/GB/08-26/2026-08-26_VALTAN_CENTER_GRAB_LEFT_HAND_ATTACHMENT_PLAN.md` 및 RESULT:
  `CAPTURE/BOSS_LEFT_HAND/GRABBED/release`가 실제 반영된 범위
- `.md/GB/08-27/2026-08-27_VALTAN_WALL_BREAK_HALF_DONUT_AND_RESETLESS_LIVE_FLOW_IMPLEMENTATION_PLAN.md`:
  별도 PR 뒤 소비할 Boss Tool resetless Play 계약
- 현재 `Data/Valtan/*.json`, authored binding/root motion, `CValtanBrain`, `CGameRoom`,
  `CBossCombatRuntime`, `CClientReplication`, `CPlayerController`, `CCharacter` 실측

## 1. 요청을 Server 계약으로 해석한 최종 동작

### 1.1 성공 경로

1. `VALTAN_TRASH`가 도입 동작을 끝낸다.
2. 발탄이 non-grabbed 생존 플레이어를 향해 회전한 뒤 root motion으로 돌진한다.
3. 돌진 중 Server의 전방 BOX와 player body circle이 겹치고 damage 결과가 `LANDED`이면
   `CAPTURE/BOSS_LEFT_HAND` request가 생성된다.
4. `CGameRoom`이 request를 commit한 플레이어만 `GRABBED`가 된다.
5. 포획 commit은 진행 중 skill, projectile, combat object, combo buffer, pending move/skill,
   knockback과 move path를 한 번에 취소한다.
6. 플레이어를 발탄의 canonical left-hand grip pose로 옮기고, 이후 매 fixed tick owner를 따라가게 한다.
7. rush가 끝났을 때 발탄이 한 명 이상의 유효한 left-hand attachment를 소유하면 카운터 준비 단계로 간다.
8. 준비 단계 진입 다음 fixed tick부터 source 0~200ms window 안에 다른 non-grabbed 플레이어가
   유효한 counter-power 공격을 맞힌다.
9. Server가 `COUNTER_HIT`을 publish하고, timeout보다 먼저 성공 branch를 소비한다.
10. 유효 hit를 받은 `Try_TriggerCounter()`가 즉시 counterable flag를 닫고, 다음 boss update의
    stage transition이 이를 다시 fail-safe로 닫으면서 groggy flag와
    `RELEASE_GRABBED_PLAYERS/HOLD`를 하나의 staged commit으로 적용한다.
11. 해제된 플레이어는 `NONE`, `isCombatReady=true`가 되어 그 snapshot부터 다시 입력할 수 있다.
12. 발탄은 실제 groggy start/loop/end animation을 재생한 뒤 패턴을 끝낸다.

### 1.2 카운터 실패 경로

- 카운터 window가 끝날 때까지 유효한 `COUNTER_HIT`이 없으면 groggy로 가지 않는다.
- 잡힌 플레이어를 해제하지 않는다.
- `mesh_att_battle_13_05-1`의 나머지 200~3000ms를 `CATCH_SLAM`으로 계속 재생한다. 0~200ms만
  재생한 뒤 곧바로 다른 clip으로 건너뛰지 않는다.
- `CATCH_SLAM`이 끝났을 때 non-grabbed 생존자가 남아 있으면 다른 target을
  재선정하고 다시 돌진한다.
- 새 rush에서 추가 플레이어가 잡힐 수 있다. 이미 잡힌 플레이어는 damage/target 후보에서 제외된다.
- rush 종료 시 이미 소유한 grabbed player가 한 명이라도 있으면 새 포획이 없더라도 다시 카운터 window로 간다.
- 모든 생존 플레이어가 잡혔다면 다른 counter 주체가 없으므로 `ALL_GRABBED_RELEASE`에서 안전 해제하고
  패턴을 끝낸다. `ALL_PLAYERS_GRABBED`는 성공 판정이 아니라 soft-lock 방지 판정이다.
- 그 외 반복은 counter 성공, boss/pattern abort, boss death, player death/disconnect 또는 arena reset 중
  하나가 발생할 때까지 Server가 소유한다.

### 1.3 돌진이 빗나간 경로

- 첫 rush에서 포획 commit이 한 건도 없으면 카운터 window를 열지 않는다.
- capture-fail presentation을 거쳐 다시 target을 고르고 rush를 반복한다.
- `ApplyPatternHit()`이 request를 만들었다는 사실은 성공 근거가 아니다.
  실제 `Capture_PlayerAttachment()` commit 뒤의 Server attachment state만 분기 근거다.

### 1.4 한 명과 여러 명의 의미

- 현재 BOX pulse는 충돌한 모든 eligible player를 검사하므로 multi-capture를 허용한다.
- multi-capture presentation 정책은 `STACK_AT_CANONICAL_GRIP`으로 고정한다. 여러 모델이 같은 왼손 grip에
  겹치는 것은 의도된 단순 정책이며, packet에 capture ordinal이나 player별 slot을 추가하지 않는다.
- `ANY_PLAYER_GRABBED`는 “이 boss가 살아 있는 플레이어 한 명 이상을 `BOSS_LEFT_HAND`에 소유한다”는 뜻이다.
- 기존 `ALL_PLAYERS_GRABBED`는 “생존 참가자가 한 명 이상이고 그 전원이 이 boss의
  `BOSS_LEFT_HAND`에 잡힘”으로 정의한다. 빈 생존 집합은 `true`로 취급하지 않는다.
- 잡힌 플레이어 본인은 Server에서 `GRABBED/isCombatReady=false`이므로 카운터할 수 없다.
  자연스러운 성공 검증에는 최소 두 플레이어가 필요하다.
- 한 명 플레이에서는 capture -> counter timeout -> catch slam -> safe release까지 검증한다. counter 성공은
  두 번째 Client 또는 별도의 Server contract fixture로 검증한다.

## 2. 현재 반영 상태

| 계약 | 현재 상태 | 이번 후속 작업 |
|---|---|---|
| Server XZ BOX overlap/damage admission | 반영됨 | `13_04` pulse와 권위 유지 |
| `CAPTURE/BOSS_LEFT_HAND -> GRABBED` | 반영됨 | collision offset을 canonical slot snap으로 교정 |
| Server move/skill/action 취소·거부 | 반영됨 | 회귀 검증 |
| attachment snapshot/fallback follow | 반영됨 | gameplay/presentation slot metadata로 pose 분리 |
| release/cleanup primitive | 반영됨 | groggy 전이에서 boss/player non-partial commit 추가 |
| counter flag/proxy/mailbox/timeout 우선순위 | 반영됨 | 실제 `13_05-1` 0~200ms stage로 이동 |
| Trash stage graph | 잘못 연결됨 | rush→capture state→counter/slam/rerush/groggy로 재배치 |
| deadline capture-state branch | 미반영 | `ANY` 추가, 기존 `ALL` 실제 평가, tick-gap validate |
| Client grabbed command submission gate | 미반영 | packet 0건 + held-input rearm 추가 |
| Boss Tool resetless Play | 선행 계획 | merge 뒤 기존 계약을 소비, 맵 복구 없음 |

### 2.1 이미 구현된 `VALTAN_TRASH`

현재 `Data/Valtan/Valtan.gameplay.json`에는 다음 그래프가 있다.

| Stage | 현재 gameplay | 현재 presentation | 현재 문제 |
|---|---|---|---|
| `STEP_01..05` | 도입 sequence | `12_01..13_02` | 유지 |
| `STEP_06` | 4100ms WINDUP, counter flag, proxy, `COUNTER_HIT -> 07`, `TIMEOUT -> 08` | `mesh_att_battle_13_02-1` | 포획 전 counter가 열리고 원본 counter notify도 없음 |
| `STEP_07` | 1000ms GROGGY, ENTER release | `mesh_att_battle_13_03` | 이 clip은 change-target/종료 계열이지 실제 groggy가 아님 |
| `STEP_08` | 667ms BOX, 7 pulse, CAPTURE/LEFT_HAND, `TIMEOUT -> 06` | `mesh_att_battle_13_04` | 포획 commit 여부를 보지 않고 항상 counter 준비로 감 |

`STEP_08.motion = null`은 정지 의미가 아니다. 별도 root-motion 정본에 같은 stage/occurrence의
667ms, 전방 최종 약 7.4608m sample이 있고 publisher가 Server bootstrap의 stage root motion으로
투영한다. 새 `FORWARD_MOTION` 필드나 두 번째 이동 경로를 만들지 않는다.

BOX는 `lengthM=6.0`, `halfWidthM=2.5`이며 0, 100, 200, 300, 400, 500, 600ms에 평가된다.
발탄의 root pose가 fixed tick마다 이동하므로 결과는 이동하는 BOX의 반복 sample이다. PhysX Client
collider는 시각 mirror일 뿐 판정 권위가 아니다.

### 2.2 원본 animation notify와 현재 mapping의 불일치

원본 `Data/Animation/Reference/Valtan/Valtan.animnotify` 실측은 다음과 같다.

| Clip | 길이 | 핵심 notify | 올바른 역할 |
|---|---:|---|---|
| `mesh_att_battle_13_02-1` | 4100ms | counter 없음, 4000ms next | 구속/돌진 준비 cast |
| `mesh_att_battle_13_03` | 1000ms | hit, 1000ms change-target | target 전환/반복 연결 |
| `mesh_att_battle_13_04` | 667ms | 300~500ms hit, 550ms skill-effect-hit 조건 | 실제 rush/capture |
| `mesh_att_battle_13_05-1` | 3000ms | **0~200ms COUNTER** | 포획 성공 뒤 counter 준비 donor |
| `mesh_att_battle_13_05-2` | 1000ms | 800~1000ms hit | 포획/counter 실패 donor |
| `mesh_abn_groggy_1_start` | 1833ms | counter-hit/groggy 표현 | groggy 시작 |
| `mesh_abn_groggy_1_loop` | clip 800ms | **600ms `MonsterMoveNextStage`** | groggy 유지 600ms slice |
| `mesh_abn_groggy_1_end` | 2000ms | recovery | groggy 종료 |

따라서 current `STEP_06`에 flag만 붙여 4.1초 전체를 counter window로 쓰는 것은 source occurrence와
맞지 않는다. current `STEP_07`도 `stageKind=GROGGY`라는 이유만으로 groggy animation이 되지 않는다.
true groggy gameplay duration은 full loop clip 800ms가 아니라 source notify edge를 따라
`1833 + 600 + 2000 = 4433ms`로 고정한다.

`13_03`, `13_05-1`, `13_05-2`에는 별도의 source hit notify도 있지만 이번 수정의 권위 범위는
`13_04` rush capture와 player counter/release다. damage profile과 Server shape 근거가 없는 hit notify는
이번 PR에서 Product damage로 승격하지 않는다. 이 세 stage는 presentation/root-motion donor로만 쓰며,
그래서 전체 occurrence를 “source-exact gameplay”라고 부르지 않는다.

### 2.3 이미 구현된 Server capture/input/release

다음 기반은 재작성하지 않는다.

- `CValtanBrain::ApplyPatternHit()`은 Shared XZ primitive로 overlap을 검사하고, damage가 실제
  `LANDED`인 `CAPTURE/BOSS_LEFT_HAND` hit만 capture request로 만든다.
- `CGameRoom::Capture_PlayerAttachment()`은 stable player/boss entity를 재검증하고 action transaction을
  취소한 뒤 `GRABBED`, `isCombatReady=false`와 attachment owner/slot을 commit한다.
- `Update_Players()`는 이미 grabbed player의 attachment만 갱신하고 movement/skill update를 건너뛴다.
- `Handle_Move()`는 action이 `NONE`이 아니면 이동을 commit하지 않는다.
- `CPlayerSkillSystem`도 grabbed action에서 skill을 시작하지 않는다.
- `Release_PlayerAttachment()`은 attachment를 지우고 생존 플레이어를 `NONE/combatReady`로 되돌린다.
- boss death, despawn, pattern 교체, action abort, transition 실패와 Server owner entity 소실 또는
  non-finite gameplay transform에는 fail-closed cleanup이 있다.
- Shared snapshot에는 owner entity, slot, boss-root local fallback pose가 이미 있다. packet shape를 늘릴 필요가 없다.

Client에서 hand bone이나 matrix를 못 찾는 경우는 Server attachment 해제 조건이 아니다. attachment는 유지하고
Client만 snapshot의 boss-root fallback pose를 사용한다.

즉 별도 JSON `inputBlocked`, Client local grab state 또는 Valtan 전용 player state를 추가하지 않는다.

### 2.4 이미 구현된 counter 우선순위

room tick은 command 처리와 `Update_Players()`를 boss brain보다 먼저 실행한다. 일반 player skill 또는
projectile hit가 유효하면 `CBossCombatRuntime`이 `COUNTER_HIT`을 pending outcome에 넣는다.

이후 `CValtanBrain::Update()`는 현재 stage의 hit와 timeout을 처리하기 전에
`ApplyPublishedOutcomeBranch()`를 호출한다. 따라서 counter window의 마지막 fixed tick에
`COUNTER_HIT`과 deadline이 같이 성립해도 counter가 이긴다. 유효 counter hit를 받은
`Try_TriggerCounter()`는 publish 직후 `COUNTERABLE`을 즉시 닫고, stage EXIT의 flag close는 fail-safe다.

다만 새 stage ENTER는 그 tick의 player/projectile update가 끝난 뒤 적용된다. 즉 counter window 진입 tick에
이미 처리된 공격은 새 window에 들어오지 않으며, 최초 허용 공격은 다음 fixed tick부터다. 최초/최종 허용
tick과 timeout 동시 tick 우선순위를 Server harness로 고정한다.

### 2.5 `VALTAN_CATCH_BREATH` 재사용 경계

“뒤돌아서 잡기 후 포효” 계열 `VALTAN_CATCH_BREATH`는 이미 다음 계약을 사용한다.

```text
LOCK_RANDOM_ALIVE_BEHIND_ON_START
  -> rear CONE 120deg / 8m / CAPTURE / BOSS_LEFT_HAND
  -> 4초 hold
  -> RELEASE_GRABBED_PLAYERS / OPPOSITE_KNOCKBACK / 12mps / 500ms
```

이번 Trash는 같은 capture, attachment, input block, release primitive를 재사용한다. 차이는 release trigger가
시간 종료가 아니라 `COUNTER_HIT -> GROGGY ENTER`라는 점뿐이다.

## 3. 현재 남은 실제 결함

### 3.1 잘못된 stage 순서와 조건 없는 진입

현재는 `STEP_05 -> STEP_06(counter) -> STEP_08(rush)`라서 누구도 잡히기 전에 패턴을 성공시킬 수 있다.
또 rush가 빗나가도 `STEP_08 TIMEOUT -> STEP_06`이라 counter가 열린다.

필요한 수정은 단순히 `STEP_05 -> STEP_08`만 바꾸는 것이 아니다. 그렇게 하면 4100ms 준비 clip,
change-target clip, 실제 success/fail donor와 true groggy가 계속 잘못된 역할로 남는다. 아래 4절의 내부
분기 graph로 역할을 다시 배치한다.

### 3.2 capture request와 commit의 구분이 없음

현재 GameRoom은 capture request마다 `Capture_PlayerAttachment()`을 호출하면서 반환값을 버린다.
hit 직후 player death, owner 교체, invalid state 등으로 commit이 실패해도 진단이 없다.

- branch는 request 생성 수가 아니라 committed attachment state를 본다.
- capture commit 실패는 정상 race라면 패턴 miss로 남는다.
- 이번 분기 구현에 새 capture-result enum을 필수 계약으로 만들지 않는다. 기존 bool과 최종 attachment state로
  correctness를 닫고, 운영 진단 소비자가 생길 때만 structured reason을 별도 추가한다.
- contract test는 request가 생겨도 commit이 실패하면 `ANY_PLAYER_GRABBED`가 false임을 검증한다.

### 3.3 왼손 follow는 있지만 왼손 snap은 없음

현재 Server는 충돌 순간의 player world pose를 boss-root local offset으로 저장한다. Client도 첫 presentation
frame에 `playerWorld * inverse(leftHandWorld)`를 저장한다. 그 결과 플레이어는 충돌 위치와 손 사이의
간격을 보존한 채 손 bone을 따라갈 뿐, 손바닥으로 이동하지 않는다.

`BOSS_LEFT_HAND` slot의 의미를 다음처럼 교정한다.

- `Valtan.gameplay.json` root의 typed `attachmentSlots`에는 `BOSS_LEFT_HAND`, frame
  `BOSS_ROOT_LOCAL`, `fallbackPositionM[3]`, `fallbackYawDegrees`와
  `STACK_AT_CANONICAL_GRIP` 정책을 한 번만 둔다.
- `Valtan.presentation.json` root의 같은 stable slot에는 Valtan asset, `bip001-l-hand`, frame
  `BONE_LOCAL`, `gripPositionM[3]`, `gripRotationDegrees[3]`를 한 번만 둔다.
- Server fallback과 Client grip은 서로 다른 좌표계의 서로 다른 transform이다. 같은 숫자를 복사하지 않고
  stable slot ID로 join한다.
- Server capture는 active catalog가 아니라 owner boss의 `PinnedDefinitionRevision`으로 catalog를 resolve하고,
  그 revision의 fallback 값을 player attachment에 한 번 저장한다. 이미 잡힌 player를 active revision 값으로
  다시 계산하지 않는다.
- Client는 최초 player world를 역산하지 않고 hand-local grip을 사용한다. bone/matrix가 없으면 attachment를
  유지한 채 Server fallback presentation으로 내린다.
- Client도 owner snapshot의 pinned revision과 CValtan이 staged한 presentation-slot revision을 attachment edge에
  고정한다. 둘이 다르거나 그 revision의 slot view가 없으면 active 문서로 대체하지 않고 fallback을 쓴다.
- 두 transform은 승인된 grab/counter reference occurrence에서 각각 bake하고 finite/frame/reference-pose parity를
  validator로 확인한다. `VALTAN_TRASH`와 `VALTAN_CATCH_BREATH`가 같은 slot record를 소비한다.

현 packet의 owner/slot/local offset 필드는 그대로 사용한다. Server는 bone animation을 계산하지 않으므로
animated hand pose와 release snapshot 사이의 zero-pop을 자동 보장한다고 쓰지 않는다. 완료 기준은 canonical
grip snap, Server fail-safe follow, release reconciliation 오차 계측과 사용자 visual 승인이다.

### 3.4 Client 명령 제출은 아직 차단되지 않음

Server는 grabbed command를 거부하지만 `CPlayerController`의 상위 enable gate는 현재 camera/input block만
본다. 따라서 Client가 이동/skill packet을 계속 보낼 수 있고, 잡힌 동안 누른 LMB/RMB/hold key가 release
뒤 지연 명령으로 살아날 위험이 있다.

남은 Client 보강은 다음과 같다.

- `CPlayerController`가 이미 읽는 `CCharacter::Try_Get_NetworkActionState()`의
  `eAction == GRABBED`를 command-enable gate에 포함한다. Level에 별도 grab bool을 만들지 않는다.
- grab 진입 edge에서 ground-target preview, held skill/aim과 move resend state를 로컬 clear한다.
- blocked frame에는 `Move`, `UseSkill`, `ReleaseSkill`, `UpdateAim`, Esther command packet을 하나도 제출하지
  않는다. 특히 `Poll_SkillSlots()`가 만드는 hold-skill release ID도 packet으로 보내지 않는다.
- LMB basic-attack gate, RMB move resend와 keyboard key edge를 “물리 release 전 재무장 금지”로 만든다.
- release snapshot은 새 입력부터 받으며 grab 전에 buffer됐거나 grab 중 눌린 명령을 재생하지 않는다.
- `Character::Apply_NetworkAction()`에 중복된 두 `GRABBED` 진입 분기와 도달 불가능한 release 분기를
  하나의 경로로 합친다.

Server 거부 규칙은 그대로 남겨 Client gate가 빠지거나 악성 packet이 와도 권위를 유지한다.

### 3.5 counter 판정의 현재 의미

현재 counter proxy는 player attack collider와 left-hand circle의 교차가 아니다. boss hit가 먼저 성립한 뒤
공격의 authoritative source XZ가 boss-local `(forward +1.0, right -1.5, radius 2.25)` 안인지 검사한다.

이번 변경은 이 의미를 유지한다. proxy 안에서 boss에 실제 landed한 `counterPower > 0` 공격만 성공한다.
현재 balance 정본에서 그 조건을 가진 playable skill은 Lance Master `34580 / A / 절룡세` 한 건이다.
다른 class의 일반 공격을 몰래 counter로 취급하지 않는다. roster 전체 counter skill 추가는 별도 balance
수직 슬라이스다.

### 3.6 groggy release는 현재 cross-player atomic이 아님

현재 `Apply_BossPatternStageTransition()`은 combat/world transaction을 commit하고 boss flag를 갱신한 뒤
`Commit_BossPatternPlayerStageActions()`에서 attachment release를 수행한다. 같은 함수와 같은 tick이지만,
player release 실패 시 이미 바뀐 boss state와 world state를 되돌리는 단일 transaction은 아니다.

목표 구현은 특정 GROGGY stage ID를 특례 처리하지 않고 typed
`RELEASE_GRABBED_PLAYERS` action 전체를 player-copy staging 대상으로 바꾸는 것이다. `HOLD`와
`OPPOSITE_KNOCKBACK` 모두 preflight에서 affected `SERVER_PLAYER` 사본에 결과를 만들고 boss combat state와
non-partial transition commit한다. disconnect/death cleanup 때문에 대상이 0명이 된 것은 유효한 idempotent
결과지만, 여러 대상 중 일부만 정상 transition 결과로 release되는 것은 금지한다.

preflight/commit failure의 최종 보장은 “process 전체 mutation 0건”이 아니다. 부분 **transition state**를
snapshot으로 내보내지 않은 뒤 existing caller가 전 attachment를 fail-closed release하고, boss를
`bossBeforeBrain`으로 rollback하고, room을 not-ready로 끝낸다. counter outcome/즉시 flag close처럼 Brain 전에
생긴 edge도 있을 수 있으므로 fault harness는 `전원 cleanup + boss rollback + snapshot 없음`을 검사한다.

## 4. 목표 `VALTAN_TRASH` 내부 graph

### 4.1 권장 graph

```text
STEP_01..05                    현재 project-authored 도입 순서 유지
  -> SETUP_CAST                13_02-1, 4100ms, counterable 아님
  -> RETARGET                  13_03, 1000ms
  -> RUSH_CAPTURE              13_04, 667ms root motion + BOX CAPTURE
       deadline + ANY_PLAYER_GRABBED
         -> CATCH_COUNTER      13_05-1 source 0~200ms slice
              COUNTER_HIT -> GROGGY
              TIMEOUT     -> CATCH_SLAM
       deadline + no owned grabbed player
         -> RUSH_MISS

RUSH_MISS                     13_05-2, 1000ms + 원본 root motion, release 없음
  -> RETARGET -> RUSH_CAPTURE

CATCH_SLAM                    13_05-1 source 200~3000ms slice, release 없음
  deadline + ALL_PLAYERS_GRABBED -> ALL_GRABBED_RELEASE
  deadline + otherwise           -> RETARGET -> RUSH_CAPTURE

ALL_GRABBED_RELEASE           13_05-2 recovery 1000ms, Product cue 없음
  ENTER = RELEASE_GRABBED_PLAYERS/HOLD
  -> terminal

GROGGY                        4433ms
  presentation = groggy_1_start 1833ms
               + groggy_1_loop   600ms source notify slice
               + groggy_1_end   2000ms
  ENTER = RELEASE_GRABBED_PLAYERS/HOLD + groggy flag true
  EXIT  = groggy flag false
  -> terminal
```

기존 `STEP_06/07/08` stable action ID는 가능한 범위에서 유지하되 역할을 바로잡는다.

- `STEP_06`: counter event/proxy/branch를 제거하고 setup cast로 사용한다.
- `STEP_07`: release/groggy event를 제거하고 retarget 연결 stage로 사용한다.
- `STEP_08`: root motion과 BOX capture를 그대로 유지한다.
- 새 semantic action ID는 catch counter, catch slam, rush miss, all-grab safe release와 groggy에만 추가한다.
- `STEP_01..05`는 현재 프로젝트가 승격한 도입 순서를 보존하는 것이며 source occurrence exact 복원이라고
  재분류하지 않는다.

### 4.2 `ANY_PLAYER_GRABBED`의 정확한 Server 의미

새 JSON object나 Valtan 전용 script block을 만들지 않고 기존 branch outcome vocabulary에 값 하나만
추가한다.

```json
{
  "branches": [
    {
      "outcome": "ANY_PLAYER_GRABBED",
      "nextActionId": "valtan.sequence.center-trash-rush-if.catch-counter"
    },
    {
      "outcome": "TIMEOUT",
      "nextActionId": "valtan.sequence.center-trash-rush-if.rush-miss"
    }
  ]
}
```

이 outcome은 mailbox event가 아니라 deadline state predicate다. 기존 enum
`ALL_PLAYERS_GRABBED`도 같은 state-predicate 경로에서 실제 평가되게 닫는다.

```text
stage duration과 모든 due hit pulse가 끝남
  -> 이 stage의 deadline state predicate를 Server player attachment에서 평가
     predicate true  -> typed state branch
     predicate false -> 유일한 TIMEOUT fallback
```

매 tick이나 stage ENTER에서 검사하면 이미 한 명을 잡은 재돌진을 건너뛰므로 금지한다. 반드시 rush
deadline에서만 검사한다. Brain은 같은 tick에 capture request를 만든 뒤 deadline branch를 먼저 정하고,
GameRoom은 Brain 반환 뒤 request를 commit한다. 따라서 이 구현은 two-phase deadline을 새로 만들지 않고,
아래 quantized authoring 규칙으로 capture commit tick과 branch tick을 분리한다.

```text
ceil(lastCaptureOffsetMs * SERVER_TICK_HZ / 1000)
  < ceil(stageDurationMs * SERVER_TICK_HZ / 1000)
```

현재 `600ms @ 30Hz = tick 18`, `667ms @ 30Hz = tick 21`이라 조건을 만족한다. 마지막 capture pulse와
deadline이 같은 tick이 되는 데이터는 publisher가 거부한다. Server harness에는 same-tick request가 그
deadline의 `ANY`를 충족하지 못한다는 negative fixture도 두어 런타임 순서를 고정한다.

`PLAYER_CAPTURED` 같은 순간 edge도 사용하지 않는다. counter 실패 뒤 재돌진에서 새 포획이 없어도 기존
grab을 근거로 counter window를 다시 열어야 하므로 current state predicate가 정확하다.

predicate의 strict rule은 다음과 같다.

- 한 stage에는 state predicate가 최대 하나이고, 정확히 하나의 `TIMEOUT` fallback이 있어야 한다.
- `ANY_PLAYER_GRABBED`는 그 **current stage 자체**에 non-NONE hit,
  `CAPTURE/BOSS_LEFT_HAND`가 있고 위 pulse/deadline gap을 만족할 때만 허용한다.
- `ALL_PLAYERS_GRABBED`는 같은 pattern의 capture path 뒤에만 허용하고, true target은 reachable한
  `RELEASE_GRABBED_PLAYERS` terminal이어야 한다.
- valid attachment는 live player의 `GRABBED`, owner=boss, slot=`BOSS_LEFT_HAND`가 모두 일치해야 한다.
  `ALL`은 live participant count가 0이면 false다.
- `ANY`, `ALL`, `TIMEOUT`은 mailbox publish 대상이 아니다. `Publish_PatternOutcome()`은 이를 거부하고
  `COUNTER_HIT` 같은 edge outcome만 받는다.
- branch target은 기존 규칙대로 같은 pattern의 stable action ID여야 한다.

### 4.3 counter window authoring

```json
{
  "stageId": "CATCH_COUNTER",
  "actionId": "valtan.sequence.center-trash-rush-if.catch-counter",
  "stageKind": "WINDUP",
  "durationMs": 200,
  "events": [
    {
      "trigger": "ENTER",
      "kind": "SET_BOSS_FLAG",
      "flagId": "boss.flag.counterable",
      "enabled": true
    },
    {
      "trigger": "EXIT",
      "kind": "SET_BOSS_FLAG",
      "flagId": "boss.flag.counterable",
      "enabled": false
    }
  ],
  "branches": [
    {
      "outcome": "COUNTER_HIT",
      "nextActionId": "valtan.sequence.center-trash-rush-if.groggy"
    },
    {
      "outcome": "TIMEOUT",
      "nextActionId": "valtan.sequence.center-trash-rush-if.catch-slam"
    }
  ],
  "counterProxy": {
    "space": "BOSS_LOCAL",
    "forwardOffsetM": 1.0,
    "rightOffsetM": -1.5,
    "radiusM": 2.25
  }
}
```

presentation occurrence는 `mesh_att_battle_13_05-1`, `sourceStartMs=0`, `playMs=200`으로 맞춘다.
outgoing gameplay hit는 저작하지 않는다. 4.1초 setup clip 전체에 counter flag를 열지 않으며, stage ENTER가
적용된 다음 fixed tick부터만 player counter가 허용된다. source presentation은 0~200ms지만 30Hz gameplay
admission은 entry tick을 제외하고 deadline tick에 먼저 들어온 `COUNTER_HIT`을 포함하므로 약 33~200ms다.

### 4.4 실패 동작과 root-motion slice

`mesh_att_battle_13_05-1`의 root motion은 source 전체 3000ms에서 전방 약 2.5035m다. 약 198ms sample에서
이미 전방 약 1.7156m이므로 0~200ms counter window만 떼어 쓰고 나머지를 버리거나, 두 stage에 원본
absolute sample을 그대로 복제하면 이동이 누락되거나 중복된다.

generator는 source 200ms 경계 pose를 보간한 뒤 다음처럼 두 개의 0-based slice를 만든다.

```text
CATCH_COUNTER samples(t) = source(t) - source(0),       0 <= t <= 200
CATCH_SLAM samples(t)   = source(t + 200) - source(200), 0 <= t <= 2800
```

두 generated curve 자체는 source 0~3000ms를 겹침 없이 표현한다. boundary sample/yaw continuity,
sample time 0/final 포함, finite와 monotonic time은 root-motion check가 검증한다. 다만 이것이 현재 30Hz
Server가 source 최종 displacement를 전부 commit한다는 뜻은 아니다.

`CATCH_COUNTER` deadline은 tick 6에서 먼저 `CATCH_SLAM`으로 전이되고 GameRoom은 그 뒤 새 stage motion을
적용한다. 따라서 old stage는 약 166.7ms까지만 이동하고 source 166.7~200ms가 빠진다. current curve의
165ms=1.5090m, 198ms=1.7156m를 보간하면 누락은 약 0.207m이고, collision/navigation clamp가 없는 focused
fixture에서 두 slice의 실제 합산 이동 승인값은 약 `2.30m ± 0.02m`다.

이번 버그 수정은 이 양자화 값을 명시적으로 승인하고 전 패턴 공통 root-motion 적용 순서는 바꾸지 않는다.
authoring parity는 2.5035m source curve, runtime parity는 30Hz actual 약 2.30m로 따로 검사한다. 추후 old-stage
terminal delta를 branch 전에 commit하려면 모든 root-motion pattern 회귀가 필요한 별도 Engine/Server 계약으로
다룬다.

`RUSH_MISS`와 `ALL_GRABBED_RELEASE`가 쓰는 `mesh_att_battle_13_05-2`는 source 1000ms/root motion 전방
약 2.1307m를 보존한다. Product fail effect cue는 `RUSH_MISS` 한 occurrence로만 옮기고,
`ALL_GRABBED_RELEASE` 재사용 occurrence에는 같은 cue ID를 복제하지 않는다.

기존 `RUSH_CAPTURE/13_04`의 authored final sample은 약 7.4608m지만 branch tick에는 새 stage identity가
먼저 적용되어 old-stage 마지막 motion delta가 실행되지 않을 수 있다. harness는 raw final sample을 그대로
assert하지 않고 30Hz stage clock에서 실제 commit된 이동 거리와 pulse coverage를 계측한다.

### 4.5 groggy/release authoring

GROGGY gameplay stage는 하나로 두고 presentation occurrence 세 개를 순서대로 연결한다. release는
첫 occurrence cue가 아니라 gameplay stage ENTER action이 소유한다.

```json
{
  "stageKind": "GROGGY",
  "durationMs": 4433,
  "events": [
    {
      "trigger": "ENTER",
      "kind": "RELEASE_GRABBED_PLAYERS",
      "releaseMode": "HOLD",
      "speedMps": 0.0,
      "durationMs": 0
    },
    {
      "trigger": "ENTER",
      "kind": "SET_BOSS_FLAG",
      "flagId": "boss.flag.groggy",
      "enabled": true
    },
    {
      "trigger": "EXIT",
      "kind": "SET_BOSS_FLAG",
      "flagId": "boss.flag.groggy",
      "enabled": false
    }
  ]
}
```

presentation은 `mesh_abn_groggy_1_start/1833ms`, `mesh_abn_groggy_1_loop/0..600ms`,
`mesh_abn_groggy_1_end/2000ms`이고 합이 정확히 4433ms여야 한다. source action 420631을
`VALTAN_TRASH.sourceActionIds`, presentation source와 provenance coverage에 추가한다.

release와 groggy flag는 current runtime처럼 순차 commit하지 않는다. generic typed release-action preflight가
affected player 사본과 boss combat state를 모두 stage하고 성공 시 한 번에 swap한다. 대상 0명은 idempotent
성공, 부분 player release는 정상 commit으로 허용하지 않는다. 실패하면 caller cleanup/rollback 뒤 snapshot을
내보내지 않는다. release snapshot에서 old buffer를 재생하거나 한 tick 늦게 control을 돌려주지 않는다.

같은 generic seam을 `VALTAN_CATCH_BREATH`의 `OPPOSITE_KNOCKBACK`에도 적용해 HOLD 전용 두 번째 commit
semantics를 만들지 않는다.

## 5. Server fixed-tick 분기

### 5.1 정상 tick 순서

```text
1. Room command queue 처리
   - grabbed player의 move/skill은 Server admission에서 거부

2. Update_Players
   - 기존 GRABBED: attachment follow 후 continue
   - non-grabbed player의 새 skill command/action 처리

3. Combat object/projectile update
   - 이미 살아 있는 player attack/projectile이 counter 조건을 만족하면 COUNTER_HIT publish

4. Update_WorldEntities / CValtanBrain::Update
   a. 현재 stage의 published outcome을 먼저 소비
   b. due pattern hit pulse를 평가
   c. capture request를 수집
   d. deadline이면 이전 tick까지 commit된 attachment로 state branch 또는 TIMEOUT을 평가

5. GameRoom capture commit
   - player/boss identity와 state를 다시 검증
   - 성공한 player만 GRABBED/LEFT_HAND

6. stage identity가 바뀌었으면 EXIT/ENTER transition action 적용
   - 일반 stage: 기존 combat/world transaction
   - typed RELEASE_GRABBED_PLAYERS action: mode와 무관하게 boss state/affected player copy를 함께 stage한 뒤
     non-partial transition commit

7. 현재 stage identity의 Server boss root motion commit

8. grabbed player fallback transform을 새 boss pose에서 한 번 더 갱신

9. 한 world snapshot에 boss stage/flag와 player action/attachment를 함께 broadcast
```

Brain hit 판정은 그 tick의 boss motion commit보다 먼저이고, root motion은 그 뒤 적용된다. 따라서 이번
계약은 continuous PhysX sweep가 아니라 authored pulse마다 직전 committed boss pose의 BOX를 평가하는
방식이다. 0~600ms 7 pulse와 6m BOX가 이동 구간을 충분히 덮는지 focused harness로 검증하고, 공백이
발견되면 pulse schedule을 보강한다. Client collider로 보완하지 않는다.

같은 tick에 Brain이 stage branch를 확정하면 GameRoom의 motion 시점에는 이미 새 stage identity다. 따라서
old stage의 마지막 root-motion delta가 branch tick에 적용된다고 가정하지 않는다. 모든 root-motion 기대값은
raw clip final과 별개로 이 tick 순서를 실행한 결과를 기록한다.

### 5.2 rush deadline 분기

```text
if stage == RUSH_CAPTURE and deadline reached:
    if HasOwnedLiveGrabbedPlayer(boss, BOSS_LEFT_HAND):
        branch ANY_PLAYER_GRABBED -> CATCH_COUNTER
    else:
        branch TIMEOUT -> RUSH_MISS
```

- 마지막 due hit보다 deadline을 먼저 처리하지 않는다.
- 마지막 capture due tick은 deadline due tick보다 엄격히 작아야 한다.
- capture request 자체는 attachment가 아니다.
- `Capture_PlayerAttachment()` 실패를 성공으로 세지 않는다.
- 이미 같은 owner/slot에 잡혀 있는 플레이어의 idempotent request는 중복 capture edge를 만들지 않는다.
- grabbed player가 death/disconnect로 사라지면 predicate에서 제외한다.

`CATCH_SLAM` deadline은 별도로 다음 분기를 가진다.

```text
if stage == CATCH_SLAM and deadline reached:
    if HasAtLeastOneLivingPlayer()
       and EveryLivingPlayerIsOwnedGrabbed(boss, BOSS_LEFT_HAND):
        branch ALL_PLAYERS_GRABBED -> ALL_GRABBED_RELEASE
    else:
        branch TIMEOUT -> RETARGET
```

이 안전 branch는 카운터 성공으로 기록하지 않으며 groggy flag도 열지 않는다. player만 HOLD release하고
패턴을 정상 종료한다.

### 5.3 counter window 분기

```text
player hit boss
  -> damage/hit admission 성공
  -> counterPower > 0
  -> boss COUNTERABLE
  -> attack source가 boss-local proxy 안
  -> COUNTER_HIT을 patternSequence/patternId/actionId identity로 한 번 publish
  -> Try_TriggerCounter가 COUNTERABLE 즉시 close

next boss update
  -> 같은 identity의 COUNTER_HIT을 timeout보다 먼저 consume
  -> CATCH_COUNTER EXIT에서 counterable close 재확인
  -> GROGGY ENTER
  -> staged player release + groggy flag non-partial commit
```

다음은 실패로 남아야 한다.

- window 밖의 hit
- `counterPower=0` 공격
- proxy 밖 source에서 맞힌 공격
- grabbed player가 보낸 공격
- 이전 stage 또는 이전 pattern sequence의 stale outcome
- 이미 성공해 counterable이 닫힌 뒤의 두 번째 hit

`CATCH_COUNTER` TIMEOUT은 `RUSH_MISS`로 가지 않고 반드시 `CATCH_SLAM`으로 간다. 이 branch가
`13_05-1`의 200~3000ms와 잡힌 player attachment를 이어 받는다.

### 5.4 cleanup 분기

다음 edge에서는 authoring release action이 없어도 attachment를 정리한다.

- boss death/despawn
- pattern replacement 또는 action abort
- stage transition preflight/commit 실패
- Server owner entity 소실 또는 non-finite gameplay transform
- player death/disconnect
- arena/audition reset

카운터 TIMEOUT과 정상 rerush는 cleanup edge가 아니다. 여기서 release하면 요구사항을 위반한다.
Client hand bone/matrix 누락도 cleanup edge가 아니며 Server fallback presentation으로만 내려간다.

### 5.5 Server 분기 요약표

| 현재 stage/edge | Server가 읽는 권위 | true/성공 | false/실패 |
|---|---|---|---|
| `RUSH_CAPTURE` deadline | committed live left-hand attachment | `CATCH_COUNTER` | `RUSH_MISS` |
| `CATCH_COUNTER` hit | landed counter-power hit + flag + proxy + identity | `GROGGY` | hit 무시, stage 계속 |
| `CATCH_COUNTER` deadline | pending `COUNTER_HIT` 우선 소비 | `GROGGY` | `CATCH_SLAM` |
| `CATCH_SLAM` deadline | non-empty living set 전원이 owned grabbed인지 | safe release terminal | `RETARGET`/rerush |
| GROGGY ENTER | staged boss/player transition | groggy + 전원 release | cleanup + boss rollback + snapshot 없음 |
| death/despawn/abort/reset | lifecycle cleanup | attachment 제거 | 해당 없음 |

## 6. Product sequence와 Boss Tool 정리

현재 ordered Product sequence와 `flow.valtan.boss-tool.default`에는 다음 네 row가 연속으로 들어 있다.

```text
000021 = VALTAN_TRASH
000022 = VALTAN_TRASH_CATCH_SUCCESS
000023 = VALTAN_TRASH_CATCH_FAIL
000024 = VALTAN_TRASH_CATCH_IF
000025 = VALTAN_CATCH_BREATH
000026 = VALTAN_COUNTER
```

후속 구현 뒤 Product와 기본 Flow에서 실행할 row는 `VALTAN_TRASH` 하나다. success/fail/if는 독립 보스
기믹이 아니라 내부 branch animation/effect donor다.

- `decisionModel.scriptedSequence.patternIds`에서 auxiliary 세 ID만 제거한다. 다른 순서와
  `selectionSets`는 건드리지 않는다.
- `Data/Encounters/Valtan/ValtanBossAuditionFlows.json`에서는 exact slot
  `000022/000023/000024` object만 제거한다.
- `000021`, `000025`, `000026`과 그 뒤 모든 stable slot ID를 renumber/regenerate하지 않는다.
  `nextSlotOrdinal=29`도 유지한다.
- animation-chain promotion lineage와 Manual Audition 진단 row는 이 PR에서 억지로 삭제하지 않아도 된다.
  aux pattern definition은 animation lineage 확인용으로 남기되 Product auto sequence/default Flow가 자동
  실행하지 않게 한다.
- 기존 success/fail/if Product effect cue는 각각 internal `CATCH_COUNTER`, `RUSH_MISS`, `RETARGET`
  occurrence로 **이동**한다. 같은 cue ID를 aux와 internal occurrence 양쪽에 복제하지 않으며 aux 진단 row는
  cue 없는 animation lineage만 남긴다.
- 완전 retirement는 promotion receipt와 source-chain one-to-one 계약을 함께 바꾸는 별도 정리 단위로 둔다.

앞서 작성한
`.md/GB/08-27/2026-08-27_VALTAN_WALL_BREAK_HALF_DONUT_AND_RESETLESS_LIVE_FLOW_IMPLEMENTATION_PLAN.md`의
resetless Live Flow가 구현된 뒤 Boss Tool의 `Play`는 현재 전투를 초기화하지 않고 stable
`VALTAN_TRASH` occurrence를 queue한다. Tool은 capture나 `COUNTER_HIT`을 Client에서 강제로 만들지 않는다.
자연 분기는 실제 Server collider와 player counter skill로 검증한다.

Tool에서 필요한 read-only 상태는 다음이다.

- current pattern/stage/action stable ID
- pattern sequence
- current counterable 여부와 counter proxy
- boss가 소유한 grabbed player count
- 마지막 branch/outcome과 reject reason
- lifecycle `QUEUED/ACTIVE/COMPLETED/ABORTED`

맵, boss HP, player 위치, 파괴된 벽·바닥은 Play 시작과 완료 때 복구하지 않는다.

## 7. 데이터·projection 저작 가이드

### 7.1 정본과 생성물

| 역할 | 파일 | 편집 원칙 |
|---|---|---|
| split gameplay 정본 | `Data/Valtan/Valtan.gameplay.json` | stage/branch/hit/action과 Server attachment slot source |
| split presentation 정본 | `Data/Valtan/Valtan.presentation.json` | clip occurrence/effect/camera와 Client grip slot source |
| phase-two authoring projection | `Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py` | 같은 graph를 idempotent하게 재생성 |
| split→Product 정본 명령 | `Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2` | 아래 tracked Product를 staged commit |
| Product pattern | `Data/Encounters/Valtan/ValtanEncounter.json` | 생성물, 직접 편집 금지 |
| Product rotation | `Data/Encounters/Valtan/ValtanPatternRotations.json` | 생성물, 직접 편집 금지 |
| Product binding/cue | `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`, `Valtan.patterneffectcues.json` | 생성물, 직접 편집 금지 |
| root motion | `Data/Animation/RootMotion/Valtan.rootmotion.json` | binding occurrence로 bake, split continuity 보존 |
| Boss Tool default Flow | `Data/Encounters/Valtan/ValtanBossAuditionFlows.json` | stable slot object 22~24만 제거 |
| provenance | `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json` | PublishV2 coverage 결과 |
| Server bootstrap | `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` | publisher 생성물, 직접 편집/commit 금지 |
| legacy migration fixture | `Data/Valtan/Valtan.pattern.json` | frozen, 이번 작업에서 수정 금지 |

authoring script가 current wrong graph를 다시 만들지 않도록 generator를 먼저 수정한다. 갱신 순서는
`author ... --mode Apply -> --mode Validate -> Project-ValtanPatternMaster.ps1 -Mode PublishV2 -> root-motion bake/check
-> split/Product/publisher validation`으로 고정한다. Product JSON과 authored binding/cue 생성물을 손으로
고친 뒤 source를 나중에 맞추는 순서는 금지한다.

`Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json`은 internal action/occurrence ID가 바뀌므로
확정 tracked migration 대상이다. 현재 `build_valtan_pattern_sound_cues.py`는 scoped check 없이 전체 문서를
rewrite하므로 먼저 repeatable `--pattern-id`와 `--check`를 추가한다. Trash와 auxiliary 세 ID만 scope로 주고
exact old/new tuple 외의 diff가 0인지 입증한다. bare 전체 regeneration은 실행하지 않는다.

### 7.2 presentation 이관

- `VALTAN_TRASH_CATCH_SUCCESS`의 `mesh_att_battle_13_05-1` occurrence를
  `CATCH_COUNTER 0~200ms + CATCH_SLAM 200~3000ms` 두 donor slice로 사용한다.
- `VALTAN_TRASH_CATCH_FAIL`의 `mesh_att_battle_13_05-2`를 `RUSH_MISS`와 cue-free safe-release recovery에 사용한다.
- `VALTAN_TRASH_CATCH_IF`의 `13_03 -> 13_04`는 retarget/rerush source lineage로 사용한다.
- true groggy는 source action 420631의 canonical `mesh_abn_groggy_1_start/loop 600ms/end`를 재사용한다.
- auxiliary Product effect cue는 success→`CATCH_COUNTER`, fail→`RUSH_MISS`, if→`RETARGET`으로 move한다.
  aux와 internal occurrence가 같은 cue ID를 동시에 소유하지 않도록 global uniqueness를 검사한다.
- sound cue도 source slice로 이관한다. success 0~200ms cue는 `CATCH_COUNTER`, 이후 cue는 start time에서
  200ms를 뺀 `CATCH_SLAM`, fail cue는 `RUSH_MISS`, IF retarget cue는 `RETARGET`이 소유한다.
  `ALL_GRABBED_RELEASE`와 aux diagnostic occurrence는 sound/effect cue-free다.
- donor clip의 source hit notify는 이번 gameplay hit로 승격하지 않고 reference-only로 남긴다.
- sound/effect/root-motion join은 새 action/occurrence stable ID로 다시 projection하고 stale old tuple을 남기지 않는다.

### 7.3 schema 확장 최소 경계

pattern branch vocabulary에 새로 필요한 값은 `ANY_PLAYER_GRABBED` 하나다. 별도로 left-hand snap을 닫기 위해
split source 두 root에 stable attachment-slot metadata를 한 번씩 둔다. stage마다 grip 값을 복제하거나 Boss
balance profile에 asset bone을 섞지 않는다.

| source | `BOSS_LEFT_HAND` record의 필드/좌표계 | 소비자 |
|---|---|---|
| `Valtan.gameplay.json` | `slotId`, `frame=BOSS_ROOT_LOCAL`, `fallbackPositionM[3]`, `fallbackYawDegrees`, `multiAttachPolicy=STACK_AT_CANONICAL_GRIP` | Server capture/follow/fallback |
| `Valtan.presentation.json` | 같은 `slotId`, Valtan presentation asset, `frame=BONE_LOCAL`, `boneName=bip001-l-hand`, `gripPositionM[3]`, `gripRotationDegrees[3]` | Client attachment presentation |

두 transform은 승인된 reference occurrence에서 각 좌표계로 bake하고 같은 숫자여야 한다고 검사하지 않는다.
대신 slot set one-to-one, stable ID/known frame, finite transform, canonical degree range, known bone과 Product projection
coverage를 검사한다.

projection과 runtime 소비 경계는 다음 하나로 고정한다.

```text
Valtan.gameplay.json attachmentSlots
  -> ValtanEncounter.json attachmentSlots
  -> Gameplay.bootstrap BOSSATTACHMENTSLOT row
  -> CGameplayCatalog
  -> CGameRoom::Capture_PlayerAttachment / fallback follow

Valtan.presentation.json attachmentSlots
  -> Valtan.patternbindings.json attachmentSlots
  -> CValtanPatternAnimationBindingDocument
  -> CValtan presentation slot snapshot
  -> CClientReplication hand-bone composition
```

compiled key는 양쪽 모두 `(bossArchetypeId, PLAYER_ATTACHMENT_SLOT)`이다. Server definition은 root-local
X/Y/Z/yaw와 multi policy만 들고, Client definition은 bone name과 hand-local X/Y/Z/pitch/yaw/roll만 든다.
pointer, pattern stage index, Prototype tag를 저장 key로 쓰지 않는다. `Capture_PlayerAttachment()`은 boss의
pinned catalog에서 Server definition을 찾아 현재 `SERVER_PLAYER` attachment 필드에 복사하므로 wire shape는
그대로다.

새 root property를 구버전 reader가 조용히 무시하지 않도록 split gameplay/presentation `formatVersion 1→2`,
joined source `2→3`, Product encounter `4→5`, pattern binding `3→4`, Gameplay bootstrap `24→25`를 같은
변경 단위에서 올린다. 기존 v1/v2/v3/v4 문서의 migration/validation 경계는 명시적으로 유지하거나 거부하며
unknown property를 허용해 우회하지 않는다.

Server slot 누락·중복·frame 불일치는 publisher/catalog admission 실패다. Client presentation slot이 runtime에
없거나 corrupt하면 Valtan spawn/Server gameplay를 막지 않고 해당 attachment만 replicated boss-root fallback으로
표시하며 진단을 남긴다. slot 문서를 매 frame 읽거나 Client-only hot reload하지 않는다.

- branch row shape는 바꾸지 않는다.
- Shared packet과 network protocol version은 바꾸지 않는다.
- 새로운 C++ 파일이나 `.vcxproj/.vcxproj.filters` 등록은 필요 없다.
- 기존 parser/runtime 파일에서 branch outcome과 attachment-slot root allowlist를 같은 변경 단위에서 갱신한다.
- `CAPTURE`는 계속 non-NONE hit, `BOSS_LEFT_HAND`, zero push/down과 reachable release를 요구한다.
- `ANY_PLAYER_GRABBED`는 current stage의 capture pulse/deadline gap과 paired TIMEOUT을 요구한다.
- 기존 `ALL_PLAYERS_GRABBED`는 non-empty living set과 reachable safe release terminal을 요구한다.
- `ANY/ALL` branch target과 TIMEOUT fallback은 같은 pattern의 stable action ID여야 한다.
- attachment metadata는 slot reference일 뿐 capture 여부나 player state를 Client data로 결정하지 않는다.

### 7.4 canonical grip bake 입력

“reference에서 적당히 추출”로 남기지 않고 다음 입력을 정본으로 고정한다.

| 항목 | 고정값 |
|---|---|
| model asset | `Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel` |
| primary occurrence | `VALTAN_TRASH/CATCH_COUNTER`, `mesh_att_battle_13_05-1`, source 0ms |
| boss root/hand | `b_root` / `bip001-l-hand` |
| initial hand-local grip | `gripPositionM=[0,0,0]`, `gripRotationDegrees=[0,0,0]` |
| engine frame | meters, +X right, +Y up, +Z forward |
| rotation order | `[pitchX, yawY, rollZ]`, DirectX `XMMatrixRotationRollPitchYaw` 의미 |
| composition | row-vector `CharacterScale * GripLocal * HandWorld` |

새 authoring helper `Tools/ValtanActionExtractor/build_valtan_attachment_slots.py`는 existing root-motion/model
extractor의 asset pre-transform과 axis conversion을 재사용한다. presentation grip을 hand bone의 source 0ms
matrix에 합성하고 boss root local로 환산해 gameplay의 `fallbackPositionM/fallbackYawDegrees`를 생성한다.
full pitch/roll은 Client presentation에만 남기고 Server fallback은 packet이 이미 표현하는 X/Y/Z+yaw로
투영한다.

grip data에는 character scale을 넣지 않는다. Client는 attachment entry에서 기존 character presentation scale만
보존하고 collision world translation/rotation은 버린다. `Apply` 뒤 `Check`는 reference pose에서 presentation
grip을 다시 boss-root로 투영했을 때 gameplay fallback과 위치 0.01m, yaw 0.5도 안에서 일치하는지 검사한다.

사용자 visual 조정이 필요하면 hand-local grip 두 배열만 바꾸고 helper를 다시 실행해 Server fallback을 함께
재생성한다. `VALTAN_CATCH_BREATH`도 같은 slot을 쓰지만 static fallback의 bake 기준은 이번 요청의
`13_05-1 @ 0ms` 하나다. 다른 clip 전체에서 animated hand와 fallback이 항상 일치한다고 보장하지 않는다.

## 8. 구현 단위

### G00 — 선행 PR merge gate

1. 다른 세션 PR이 `main`에 merge된 뒤 새 branch를 만든다.
2. `git status --short`, `git fetch`, current HEAD와 대응 PLAN/RESULT를 확인한다.
3. 팀 LAN sync를 실행하고 role만 기록한다. Client/UI는 에이전트가 실행하지 않는다.
4. phase-two authoring, Valtan split join, gameplay publisher와 Server contract baseline을 실행한다.
5. 기존 실패와 이번 변경 회귀를 구분하지 못한 상태에서 Product JSON을 Apply하지 않는다.

### G01 — source occurrence에 맞는 내부 stage graph

- `STEP_06`에서 counter flag/proxy/branch를 제거한다.
- `STEP_07`을 retarget 연결 stage로 바꾸고 release/groggy event를 제거한다.
- `STEP_08` root motion, BOX, hit schedule, damage, CAPTURE/LEFT_HAND를 유지한다.
- `CATCH_COUNTER`, `CATCH_SLAM`, `RUSH_MISS`, `ALL_GRABBED_RELEASE`, `GROGGY` semantic stage를 추가한다.
- `13_05-1`을 0~200/200~3000ms로 나누고 slice root motion을 boundary-rebase한다.
- `_13_05-2` root motion은 보존하고 fail cue는 `RUSH_MISS`만 소유한다.
- source 420631의 start/loop 600ms/end를 4433ms groggy로 연결한다.
- donor의 reference hit notify는 새 gameplay damage로 만들지 않는다.

### G02 — `ANY_PLAYER_GRABBED` deadline predicate

- Server catalog outcome enum/parser를 확장한다.
- publisher와 split/Product strict validator의 allowlist/semantic rule을 확장한다.
- Brain은 deadline에서만 owner+slot+live player state를 검사하고 기존 `ALL_PLAYERS_GRABBED`도 실제
  state predicate로 구현한다.
- state branch가 없거나 false면 기존 TIMEOUT fallback으로 간다.
- `ANY/ALL/TIMEOUT` mailbox publish를 거부한다.
- `ANY` current stage의 capture pulse tick이 deadline tick보다 엄격히 앞서는지 validate한다.
- request count나 Client attachment presentation을 분기 근거로 사용하지 않는다.

### G03 — capture commit과 left-hand slot 의미 교정

- `BOSS_LEFT_HAND`를 collision-offset preserve가 아니라 snap-to-slot 의미로 바꾼다.
- gameplay root에는 boss-root fallback, presentation root에는 bone-local grip을 같은 stable slot ID로 저작한다.
- exact `mesh_att_battle_13_05-1 @ 0ms` hand pose와 presentation grip에서 authoring helper가 Server fallback을
  생성하고 0.01m/0.5도 parity를 검사한다.
- `Capture_PlayerAttachment()`은 player-owner delta를 계산하지 않고 owner의
  `PinnedDefinitionRevision`으로 resolve한 `CGameplayCatalog` slot의 fallback X/Y/Z/yaw를 attachment에
  고정 저장한다.
- `CValtanPatternAnimationBindingDocument`가 grip record를 stage하고 `CValtan`이 immutable view를 제공하며,
  `CClientReplication`은 owner revision과 일치하는 view만 고정해 최초 player matrix 역산 대신 그
  hand-local matrix를 매 frame 합성한다.
- multi-capture는 `STACK_AT_CANONICAL_GRIP`으로 고정하고 packet shape를 늘리지 않는다.
- 기존 distinct collision-offset 보존 contract test를 canonical grip/fallback contract로 교체한다.
- `VALTAN_CATCH_BREATH`도 같은 slot을 소비하므로 함께 회귀 검증한다.
- capture request와 commit race는 기존 bool/final state로 검증하며 새 result enum을 완료 조건으로 만들지 않는다.

### G04 — Client input/presentation gate

- `Try_Get_NetworkActionState()==GRABBED`를 controller command-enable gate에 연결한다.
- 잡힘 진입 때 ground target, held skill/aim, move resend와 local key edge를 정리한다.
- blocked frame에는 Move/UseSkill/ReleaseSkill/UpdateAim/Esther packet을 0건 제출한다.
- 잡힌 동안 눌린 LMB/RMB/keyboard는 물리 release 후에만 재무장한다.
- 중복 `Character::Apply_NetworkAction(GRABBED)` branch를 통합한다.
- missing left-hand bone은 캐릭터 spawn이나 Server gameplay를 막지 않고 fallback presentation만 유지한다.

### G05 — counter/groggy transaction

- `COUNTER_HIT`의 existing identity, proxy, flag와 timeout 우선순위를 유지한다.
- valid hit 시 `Try_TriggerCounter()`가 flag를 즉시 닫고 EXIT close를 fail-safe로 유지한다.
- 모든 typed `RELEASE_GRABBED_PLAYERS` action에서 affected player copy를 preflight하고 boss state와
  non-partial transition commit한다.
- release 대상 0명은 성공, 일부만 release는 실패로 고정한다.
- failure는 전원 attachment cleanup, boss rollback, room not-ready와 no-snapshot으로 닫는다.
- `VALTAN_CATCH_BREATH`의 OPPOSITE_KNOCKBACK도 같은 generic seam으로 회귀 검증한다.
- `COUNTER_HIT` 뒤 stale TIMEOUT이나 두 번째 counter가 다음 stage에 새지 않게 mailbox를 정리한다.
- true groggy occurrence 세 개의 합과 gameplay duration을 parity 검사한다.

### G06 — Tool/Flow 연결

- resetless Live Flow가 먼저 반영된 current contract를 재사용한다.
- default Flow object `000022~000024`만 제거하고 `000025+`, `nextSlotOrdinal=29`를 보존한다.
- scriptedSequence에서 auxiliary 세 ID만 제거하고 selectionSets는 유지한다.
- Play는 stable `VALTAN_TRASH`만 Server에 queue하고 자연 branch를 기다린다.
- stage/grabbed count/outcome은 read-only 진단으로 보여 주고 Client fake capture/counter 버튼은 만들지 않는다.

### G07 — focused end-to-end harness

단편적인 parser test가 아니라 실제 stage graph를 순서대로 실행한다.

1. 도입과 `SETUP_CAST` 중 counter hit가 와도 성공하지 않는다.
2. 첫 `RUSH_CAPTURE`가 root motion을 타고 0~600ms BOX pulse를 만든다.
3. no-overlap과 capture-request/commit-failure 모두 counter window 없이 `RUSH_MISS/RETARGET/RUSH`로 간다.
4. same-tick final capture pulse/deadline fixture는 validator가 거부되고 runtime predicate도 새 request를 보지 않는다.
5. current 600ms/tick18 capture commit은 667ms/tick21 deadline의 `ANY`에 보인다.
6. overlap/commit 뒤 player가 canonical `GRABBED/LEFT_HAND/combatReady=false`가 된다. active catalog가 바뀌어도
   owner pinned revision에서 고정한 fallback은 변하지 않는다.
7. grabbed 중 Server command가 거부되고 Client packet/held input도 남지 않는다.
8. counter window entry tick 공격은 거부되고 다음 tick의 valid attack부터 허용된다.
9. counterPower 0, proxy 밖, window 밖과 grabbed player hit는 실패한다.
10. proxy 안 valid counter는 한 번만 publish하고 즉시 counterable을 닫는다.
11. counter와 deadline이 같은 tick이면 counter가 우선한다.
12. GROGGY ENTER에 boss flag와 모든 attachment release가 함께 commit된다. injected fault는 partial transition을
    broadcast하지 않고 전원 cleanup/boss rollback/room not-ready/no-snapshot으로 끝난다.
13. counter TIMEOUT은 release하지 않고 `CATCH_SLAM` 200~3000ms를 재생한 뒤 rerush한다.
14. `13_05-1` authored slice 합 2.5035m와 30Hz actual `2.30m ± 0.02m`를 분리해 검사한다.
15. 1-player roster 또는 모든 생존 참가자 포획은 `ALL_GRABBED_RELEASE`로 빠져 groggy 성공 없이 안전 해제된다.
16. boss death/despawn, player death/disconnect, transition failure와 reset은 attachment를 정리한다.
17. missing Server owner/non-finite transform은 release하지만 missing Client hand bone 또는 presentation revision
    mismatch는 pinned fallback을 유지한다.
18. aux effect/sound cue ID가 internal/aux 양쪽에 중복되지 않고 source-slice time이 보존된다.
19. exact Boss Tool slot 22~24만 사라지고 21, 25+, next ordinal과 selection set이 보존된다.
20. split/Product/bootstrap과 Tool joined view의 graph/occurrence/effect/root-motion tuple이 일치한다.

## 9. 예상 수정 파일

### 데이터와 pipeline

- `Data/Valtan/Valtan.gameplay.json`
- `Data/Valtan/Valtan.presentation.json`
- `Data/Valtan/Valtan.animation-chain-promotions.json` 또는 promotion projection 코드
  - internal donor lineage를 바꿀 때만 수정
- `Data/Encounters/Valtan/ValtanEncounter.json` — generated/tracked
- `Data/Encounters/Valtan/ValtanPatternRotations.json` — generated/tracked
- `Data/Encounters/Valtan/ValtanBossAuditionFlows.json`
- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json` — generated/tracked
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json` — generated/tracked
- `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json` — scoped generated/tracked migration
- `Data/Animation/RootMotion/Valtan.rootmotion.json` — generated/tracked
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json` — generated coverage
- `Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py`
- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`
- `Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1`
  - 새 attachment root와 strict projection이 필요할 때만 수정
- `Tools/ValtanPipeline/build_valtan_pattern_sound_cues.py`
  - repeatable exact pattern scope와 `--check` 추가
- `Tools/ValtanActionExtractor/build_valtan_rootmotion.py`
  - source slice boundary interpolation/rebase가 current builder로 닫히지 않을 때 수정
- `Tools/ValtanActionExtractor/test_build_valtan_rootmotion.py`
- `Tools/ValtanActionExtractor/build_valtan_attachment_slots.py` — 신규 authoring/check helper
- `Tools/ValtanActionExtractor/test_build_valtan_attachment_slots.py`
- `Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py`
- `Tools/ValtanPipeline/test_valtan_pattern_master_v2.py`
- `Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py`
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`
- `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1`
  - mirrored strict outcome allowlist를 쓸 때만 수정

### Server

- `Server/Public/GameplayCatalog.h`
- `Server/Private/GameplayCatalog.cpp`
- `Server/Private/ValtanBrain.cpp`
- `Server/Private/GameRoom.cpp`
- `Server/Private/BossCombatRuntime.cpp`
  - state outcome publish 거부와 existing counter identity/cleanup 보강
- `Server/Private/ServerGameplayContractTests.cpp`

### Client/Shared

- `Client/Public/EncounterPatternReference.h`
- `Client/Private/EncounterPatternReference.cpp`
- `Client/Public/ValtanPatternTree.h`
- `Client/Private/ValtanPatternTree.cpp`
- `Client/Public/AnimationSkillBindingDocument.h`
- `Client/Private/AnimationSkillBindingDocument.cpp`
- `Client/Public/Valtan.h`
- `Client/Private/Valtan.cpp`
- `Client/Public/ClientReplication.h`
- `Client/Private/ClientReplication.cpp`
- `Client/Public/PlayerController.h`
- `Client/Private/PlayerController.cpp`
- `Client/Private/Character.cpp`
- `Shared/Public/Network/PacketMessages.h`는 packet shape를 유지하므로 원칙적으로 수정하지 않는다.

새 C++ source/header 파일은 만들지 않는다. 신규 Python authoring helper는 Visual Studio
`.vcxproj/.vcxproj.filters` 등록 대상이 아니므로 project/filter 변경도 없다.

## 10. 자동 검증

### 10.1 데이터·projection

구현 branch에서 source를 고친 뒤 tracked 생성물은 다음 순서로 갱신한다.

```powershell
python -B Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py --mode Apply
python -B Tools/ValtanActionExtractor/build_valtan_attachment_slots.py --mode Apply
python -B Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py --mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2
python -B Tools/ValtanActionExtractor/build_valtan_rootmotion.py
python -B Tools/ValtanPipeline/build_valtan_pattern_sound_cues.py --pattern-id VALTAN_TRASH --pattern-id VALTAN_TRASH_CATCH_SUCCESS --pattern-id VALTAN_TRASH_CATCH_FAIL --pattern-id VALTAN_TRASH_CATCH_IF
```

그 뒤 read-only/check 검증을 실행한다.

```powershell
python -B Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py --mode Validate
python -B Tools/ValtanActionExtractor/build_valtan_attachment_slots.py --mode Check
python -B Tools/ValtanActionExtractor/test_build_valtan_attachment_slots.py
python -B Tools/ValtanPipeline/valtan_tuning_pipeline.py validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
python -B Tools/ValtanPipeline/test_valtan_pattern_master_v2.py
python -B Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py
python -B Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py
python -B Tools/ValtanActionExtractor/build_valtan_rootmotion.py --check
python -B Tools/ValtanActionExtractor/test_build_valtan_rootmotion.py
python -B Tools/ValtanPipeline/build_valtan_pattern_sound_cues.py --pattern-id VALTAN_TRASH --pattern-id VALTAN_TRASH_CATCH_SUCCESS --pattern-id VALTAN_TRASH_CATCH_FAIL --pattern-id VALTAN_TRASH_CATCH_IF --check
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
```

검사 항목은 다음과 같다.

- `SETUP_CAST`에 counter flag/proxy가 없음
- RUSH 전에 counter window가 reachable하지 않음
- RUSH에 root motion, BOX/CAPTURE/LEFT_HAND가 그대로 있음
- RUSH deadline의 `ANY_PLAYER_GRABBED/TIMEOUT` 두 branch
- 마지막 capture due tick 18 < deadline due tick 21
- counter window가 200ms이고 source occurrence도 `13_05-1/0..200`
- timeout이 `13_05-1/200..3000 CATCH_SLAM`으로 이어짐
- authored 두 root-motion slice는 source 2.5035m를 표현하고 30Hz runtime actual은 `2.30m ± 0.02m`임
- counter success만 GROGGY로 감
- GROGGY가 release를 소유하고 `1833+600+2000=4433ms`와 source action 420631 coverage가 일치
- rerush 경로에는 release가 없고 `ALL_PLAYERS_GRABBED`만 safe-release terminal로 감
- donor source hit notify가 Product damage row로 새로 생기지 않음
- gameplay/presentation attachment slot의 ID/frame/transform/policy join이 유효함
- `13_05-1 @ 0ms` bake를 역투영한 위치/yaw가 0.01m/0.5도 허용 오차 안임
- split/joined/Product/binding/bootstrap format version과 new slot row reader가 함께 올라감
- invalid/duplicate/missing slot, unknown frame와 non-finite transform이 fail-closed됨
- Product cue ID가 aux/internal occurrence에 중복되지 않음
- sound cue가 4개 pattern exact scope 밖을 rewrite하지 않고 slice-relative start time을 보존함
- Product ordered sequence에 auxiliary 세 ID가 없고 default Flow의 exact slot 22~24만 없음
- slot 21, 25+, `nextSlotOrdinal=29`와 selectionSets가 그대로임

### 10.2 Server/Client

- Debug와 Release의 Shared + NetworkProtocolHarness
  - Shared packet을 실제로 바꾸지 않았음을 확인한다.
- Server Debug/Release build와 `Server.exe --contract-test`
- typed release-action fault fixture에서 HOLD/OPPOSITE_KNOCKBACK 모두 success non-partial commit과
  failure cleanup/rollback/no-snapshot 확인
- Client Debug/Release compile/link
- focused Client command-sink test에서 grabbed frame packet 0건과 physical-release rearm 확인
- `git diff --check`
- 모든 변경 JSON parse와 project/filter Check

counter skill balance나 packet shape를 실제로 바꾸게 되면 Gameplay revision/receipt와 NetworkProtocolHarness를
같은 변경 단위에서 확장한다. 이번 권장안은 둘 다 피한다.

## 11. 사용자 수동 검증

자연스러운 성공 경로는 두 플레이어로 검증한다.

1. 사용자가 Server와 Client 두 개를 팀 profile로 실행한다.
2. 두 Client 모두 Valtan Arena에 들어가고, counter 담당은 Lance Master short-spear stance의 `A/절룡세`를 준비한다.
3. Boss Tool Flow/slot에서 `VALTAN_TRASH`를 선택하고 resetless `Play`를 누른다.
4. 첫 플레이어만 rush BOX 안에 두고 두 번째 플레이어는 밖에 둔다.
5. 첫 플레이어가 실제 왼손 grip으로 이동하고 이동/skill/LMB/RMB가 먹지 않는지 확인한다.
6. 다음 `13_05-1` 준비 동작의 짧은 counter window에 두 번째 플레이어가 proxy 안에서 절룡세를 맞힌다.
7. 발탄이 즉시 true groggy animation으로 전환하고, 잡힌 플레이어가 같은 시점에 풀려 다시 움직이는지 본다.
8. 다시 Play하여 counter를 늦게 치거나 proxy 밖에서 친다. 잡힌 상태로 `13_05-1`의 남은 catch slam을
   끝까지 재생한 뒤 retarget/재돌진하는지 본다.
9. 첫 rush를 빗나가게 하여 counter window 없이 fail/재돌진하는지 본다.
10. Client 한 개만 둔 별도 실행에서는 capture/counter timeout/catch slam 뒤 safe release하고 패턴이
    종료되어 영구 입력 잠금이 남지 않는지 본다.
11. 잡힌 상태에서 boss death, player disconnect와 Stop/abort를 각각 실행해 attachment가 남지 않는지 본다.
12. release snapshot reconciliation 순간의 Server fallback↔animated hand 위치 오차를 기록하고, 허용 가능한
    시각 pop인지는 사용자가 판정한다. 자동 검사에서 zero-pop으로 과장하지 않는다.

한 Client만으로는 “그 Client가 잡힌 뒤 다른 플레이어가 자연 counter”하는 전체 성공 경로를 증명할 수 없다.
한 Client smoke는 miss, capture/input block, counter timeout/catch slam과 all-grab safe release까지 확인하고,
counter 성공만 두 번째 Client 또는 Server fixture로 검증한다.

에이전트는 Client/UI를 자율 실행하거나 visual PASS를 대신 판정하지 않는다. 사용자가 left-hand grip,
animation 전환과 release timing을 직접 관찰해 승인한 뒤에만 visual 완료로 기록한다.

## 12. 완료 조건

- 첫 counter window는 실제 rush/capture 뒤에만 열린다.
- miss는 counter window를 열지 않고 rerush한다.
- counter 실패는 `13_05-1`의 남은 2800ms/root motion과 attachment를 유지한 뒤 rerush한다.
- 1-player roster 또는 모든 생존 참가자 포획은 groggy 성공으로 위장하지 않고 safe release terminal로 끝난다.
- counter 성공은 timeout보다 우선하고 같은 transition tick에 boss groggy와 모든 player release를
  non-partial commit한다.
- grabbed player는 Server와 Client 양쪽에서 입력이 차단되고 old input이 release 뒤 재생되지 않는다.
- blocked Client frame의 gameplay packet은 0건이고 held key는 physical release 뒤 재무장한다.
- player는 충돌 간격 보존이 아니라 canonical left-hand grip으로 이동하며 multi-capture는 같은 grip에 stack된다.
- Server fallback과 hand-local grip은 좌표계를 분리하고 release reconciliation 오차를 계측한다.
- attachment fallback/grip은 owner pinned revision에 고정되고 active revision 변경으로 중간 교체되지 않는다.
- generic typed release staging은 Trash HOLD와 Catch Breath OPPOSITE_KNOCKBACK에 같은 commit semantics를 적용한다.
- 실제 groggy start/loop 600ms/end가 4433ms로 재생된다.
- `VALTAN_TRASH` 하나가 Product 내부 success/fail/retry를 소유한다.
- default Product sequence와 Boss Tool Flow가 auxiliary 패턴을 연속 재생하지 않는다.
- Boss Tool은 slot 22~24만 제거되고 기존 stable slot ordinal/selection set을 보존한다.
- split source, Product encounter, bootstrap, Server runtime, Client strict view와 Tool view가 같은 graph를 본다.
- split root-motion source 2.5035m와 30Hz runtime 승인값 약 2.30m가 각각 일치한다.
- generated binding/effect/sound/provenance와 source action 420631 coverage가 일치한다.
- source donor hit notify는 근거 없는 Server damage로 승격되지 않는다.
- 정상, 잘못된 ID/outcome, quantized pulse/deadline, capture commit 실패, counter miss,
  partial-release fault와 cleanup harness가 모두 통과한다.
- Debug/Release Server·Client 검증과 사용자 수동 visual 확인이 분리되어 RESULT에 기록된다.

## 13. 비범위

- Client-local Valtan AI 또는 fake counter outcome
- PhysX/Client collider를 gameplay 권위로 승격
- 모든 class에 counter skill 추가
- counter-capable class가 없는 다인 roster를 자동 교체하는 balance 정책
- 새로운 map reset/restore 경로
- 새 attachment runtime 또는 Valtan 전용 player grab state
- player별 capture ordinal/분리 grip packet
- `13_03/13_05-1/13_05-2` source hit notify의 Product damage 승격
- animated hand와 Server root fallback 사이의 수학적 zero-pop 보장
- `Data/Valtan/Valtan.pattern.json` 수정
- 생성된 Product encounter/bootstrap 직접 편집
- auxiliary source-chain lineage 전체 retirement

이번 변경은 이미 반영된 범용 capture/counter/release 파이프라인을 보존하면서, 잘못 연결된 Trash의
Product stage graph와 실제 입력·presentation 의미를 닫는 한 수직 슬라이스로 끝낸다.
