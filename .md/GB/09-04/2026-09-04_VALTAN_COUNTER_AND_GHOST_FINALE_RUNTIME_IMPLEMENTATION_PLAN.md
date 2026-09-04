# 2026-09-04 발탄 Q/W/E/R 카운터와 유령 최종전 런타임 구현 계획서

브랜치 `GB/KoukuSaydon-Main-Pattern`. 이 문서는 공유 dirty working tree의 현재 코드와
`detail_plan_template.md`를 대조한 뒤 확정한 후속 구현 범위를 소유한다. Claude 문서의 G01 잡기 C3 제거와
G02 마력구 core/aura `STAGE_END`는 별도 계획/RESULT에 이미 반영됐으며 여기서 다시 수정하지 않는다.

사용자 요청에 따라 새 하네스를 만드는 작업은 하지 않는다. 실제 Server/Data/Client presentation 연결을 먼저
반영하고, 기존 publisher·focused contract와 Product/Core 빌드만 현재 계약에 맞춰 통과시킨다.

---

## 0. 목표와 고정 결과

| G | 목표 | 고정 결과 |
|---|---|---|
| G01 | Q/W/E/R 스킬의 실제 적중을 공용 boss counter 자격으로 저작한다 | Q/W/E/R의 `counterPower=1`을 모든 `COUNTERABLE` stage가 소비한다. Triple Counter의 front proxy와 Trash 계열의 local-circle proxy 및 각 one-shot outcome은 기존 branch를 그대로 따른다 |
| G02 | 부활한 본체 유령 발탄이 지정한 여섯 스킬을 순서대로 계속 반복한다 | `WHIRLWIND → FOUR_SLASH → SEQUENCE_FOUR → CROSS → CHARGE → CHARGE_2`, 같은 본체/HP/HUD identity 유지, 스킬 사이 임의 재배치 없음 |
| G03 | 보조 유령이 live walkable nav의 무작위 위치에서 스킬 하나만 쓰고 사라지는 루프를 실행한다 | 최대 1개, 본체와 같은 여섯 스킬 풀, occurrence 기반 deterministic random, 한 스킬/follow-up 종료 후 combat object까지 정리하고 다음 tick에 새 occurrence |
| G04 | 본체·보조 유령과 독립된 반지름 7m 정삼각 포탈 루프를 실행한다 | 세 leg `V0→V1`, `V1→V2`, `V2→V0`을 동시에 발사한다. portal pair 0ms, rush 300~1600ms, dissolve tail 1600~1900ms, 300ms gap, 다음 삼각 occurrence는 2200ms에 시작한다 |
| G05 | 시작·사망·reset·teardown 경계를 한 owner가 닫는다 | main cursor, auxiliary occurrence, portal occurrence/timer가 서로 독립이고 phase3 owner가 유효하지 않으면 child와 owner combat object가 남지 않는다 |

정삼각형 수치는 다음으로 고정한다.

```text
circumradius = 7.0m
vertices at startAngle 30deg, step 120deg
edge = 7 * sqrt(3) = 12.1243556530m
travel = 1.3s
speed = edge / 1.3 = 9.3264274254m/s
portal lifetime = 1.9s
next-leg interval = 2.2s
```

---

## 1. G01 실제 적중 Q/W/E/R 카운터

현재 입력부터 outcome까지의 정본 경로는 다음이다.

```text
PlayerController inputSlot
→ PlayerSkills catalog skillId
→ C2S_USE_SKILL
→ CPlayerSkillSystem shape/projectile overlap
→ CServerCombatHitRuntime::Apply_PlayerToWorld
→ CBossCombatRuntime::Apply_PlayerHit
→ COUNTER_HIT outcome
→ CValtanBrain branch
→ VALTAN_GROGGY_FOLLOWUP
```

`TryConsumeDamageLessCounterProxy`는 counter 전용 A 스킬의 damage-less guard이며 실제 overlap이 아니다. 이 경로를
Q/W/E/R로 넓히지 않는다. 대신 여섯 클래스의 Q/W/E/R 28개 damaging skill에 기존 공용 필드
`counterPower=1`을 저작한다. 현재 `CBossCombatRuntime`은 수치의 크기를 누적하거나 threshold와 비교하지 않고
0/비0 자격으로만 소비하므로 10 같은 임의 상수는 사용하지 않는다.

direct hit, projectile, combat-object hit은 이미 같은 `CServerCombatHitRuntime::Apply_PlayerToWorld`을 거쳐
`CBossCombatRuntime::Apply_PlayerHit`으로 들어간다. 따라서 pattern ID 예외나 Valtan 전용 resolver 없이 다음 기존
경계가 그대로 적용된다.

- 실제 Server overlap이 발생한 hit만 `counterPower`를 전달한다.
- 현재 boss stage가 `COUNTERABLE`이어야 한다.
- Triple Counter는 `BOSS_FORWARD_ARC` 180도 전방 proxy를 통과해야 한다.
- `VALTAN_TRASH`와 `VALTAN_TRASH_CATCH_IF`는 STEP_07과 retry windup의 `BOSS_LOCAL_CIRCLE` proxy를 통과해야 한다.
- 성공한 occurrence는 기존 `COUNTER_HIT` one-shot outcome과 authored branch를 따른다.

Q/W/E/R은 계속 기존 `ACTIVE`/`COMBO` skill kind를 유지한다. `counterPower`를 COUNTER kind 전용으로 제한하던
publisher 규칙만 "damage-less guard는 COUNTER kind, damaging hit은 명시적 counterPower 가능"으로 넓힌다.
PlayerSkills와 공식 provenance receipt, 생성 bootstrap을 같은 변경 단위로 맞추며 Shared packet과 Client 입력
계약은 바꾸지 않는다. 먼저 들어간 Triple Counter 전용 runtime resolver와 그 때문에 추가한 catalog 인자는
제거한다.

---

## 2. G02 본체 유령 순차 루프

부활 완료 후 `Activate_ValtanGhostPhaseLoop`가 `VALTAN_GHOST_FINALE.finale.ghostPatternIds`를
`GhostPhasePatternSequence`에 복사하는 현재 구조를 유지한다. 목록만 아래 정본으로 바꾼다.

```text
VALTAN_WHIRLWIND
VALTAN_FOUR_SLASH
VALTAN_SEQUENCE_FOUR
VALTAN_CROSS
VALTAN_CHARGE
VALTAN_CHARGE_2
```

여섯 패턴은 현재 working tree에서 Server hit와 Client animation/effect/sound를 갖고 있고 유령 donor animset에도
필요 clip이 있다. ordered-once selector가 마지막 index를 완료하면 0으로 되돌아가므로 새 selector는 만들지 않는다.
현재 각 스킬 종료마다 `Begin_ValtanGhostRelocation`을 호출하는 경로는 제거해 본체가 보조 유령의 random spawn
역할을 침범하지 않게 한다.

저작 정본은 `Data/Valtan/Valtan.gameplay.json`과
`Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py`이며 Product encounter/bootstrap은 publisher가 만든다.

---

## 3. G03 보조 유령 한 스킬 루프

`Update_DependentBosses`의 기존 spawn/despawn, same-deck navigation과 full body clearance를 재사용한다. 현재
`finaleOf()`가 phase3 loop owner를 거부하는 gate를 고쳐, `bGhostPhasePatternLoopActive`인 primary owner는 pinned
catalog의 고정 `VALTAN_GHOST_FINALE` finale 확장을 읽는다.

보조 lane은 owner에 다음 server-only state를 둔다.

```text
iGhostAuxiliaryOccurrenceSequence
iGhostAuxiliaryNextSpawnTick
```

skill 선택과 위치 후보는 `ownerNetEntityId + occurrence + 서로 다른 domain salt`로 결정한다. `serverTick`과
`nextEntityId`는 RNG seed에 넣지 않는다. child의 `DependentPatternSequence.PatternIds`는 선택된 ID 하나만 갖는다.
nav 후보가 없으면 room failure나 ID/occurrence를 소비하지 않고 다음 tick에 같은 occurrence를 재시도한다.
성공 commit 뒤에만 occurrence를 전진시킨다.

child pattern과 follow-up이 끝나면 `Cancel_Source(childId)` → lifecycle/despawn을 수행한다. 같은 update에서 바로
재생성하지 않고 다음 fixed tick을 due tick으로 기록한다. dependent ghost는 damage target/HUD owner가 아니며 기존
primary-only player damage gate를 유지한다. Shared wire와 Client runtime은 기존 owner ID + spawn/snapshot/despawn을
그대로 사용한다.

---

## 4. G04 동시 삼각 포탈 세 leg

기존 구현의 arena center 기준 radial volley 3개와 `NEXT_RADIAL_SLOT` 방향 정책은 한 번에 폐회로를 만드는 데
맞으므로 유지한다. 44m 변과 5초 주기 및 V1 red-floor visual만 아래 계약으로 교체한다.

1. synthetic owner는 immutable arena center와 world yaw 0을 사용한다.
2. 반지름 7m, start angle 30도, step 120도의 세 vertex를 계산한다.
3. radial volley count 3을 원자적으로 spawn하고 각 ordinal은 현재 vertex에서 다음 vertex를 향한다.
4. 세 combat object는 300ms까지 정지하고 300~1600ms에 12.12435565m를 동시에 이동한다.
5. distance 도달은 즉시 despawn 사유가 아니며 1900ms lifetime까지 tail을 유지한다.
6. 성공 spawn 기준 2200ms 뒤 다음 삼각형 전체를 다시 발사한다.

포탈은 `VALTAN_WARP`와 같은 authored world-transform 이동이며 navigation을 전혀 소비하지 않는다. boss spawn
XYZ와 world yaw 0으로 계산한 세 vertex의 finite 값, 정확한 반지름·변 길이와 volley tuple만 검사하고, navigation
load/sample, exact-walkable, same-deck, height-transition, line-of-sight 검사나 projection은 하지 않는다. 세 vertex를
모두 검증한 뒤 하나의 transaction으로만 spawn하므로 일부 leg만 남는 경로도 없다. full-footprint walkable-nav
검증은 G03의 보조 유령 spawn에만 적용한다.

combat-object movement에는 일반화된 `startDelayMs`와 `expireOnDistanceEnd`를 저작/Product/Server runtime에
추가한다. 기존 object는 기본값 0/true로 종전 의미를 유지하고 이 portal object만 300/false를 쓴다.

Client visual은 검증을 마친 기존 `boss.valtan.portal` V2 group을 재사용한다. group root는 leg start/yaw이고,
기존 `boss.valtan.portal.black_1`, `cyan_1` leaf pair를 local Z=0에 둔다. 동시 폐회로에서는 각 leg의 destination이
다음 leg의 start와 정확히 같으므로 세 root만으로 각 변의 양 끝과 정삼각형의 세 꼭짓점을 모두 표현한다. 같은
vertex에 start/end pair를 두 번 겹쳐 밝기와 budget을 두 배로 만드는 구성은 사용하지 않는다. 세 portal은 0ms에
함께 생겨 1900ms까지 유지·dissolve한다. combat-object V2 child는 spawn 때 확정된 world root를 유지하므로 moving
damage proxy의 snapshot pivot이 portal을 끌고 가지 않는다. 이번 최소 범위에는 별도 ghost body leaf나 새 Client
world entity를 만들지 않는다.

변경 파일은 portal authoring/combat-object JSON, BossCatalog, 기존 V2 portal group registry, Valtan pipeline/publisher,
GameplayCatalog parser/struct, CombatObjectRuntime과 GameRoom portal scheduler다. Shared packet 증가는 없다.

---

## 5. 검증과 사용자 인계

새 광역 harness는 작성하지 않는다. 구현 후 다음만 수행한다.

1. author script Validate/Apply와 Valtan master projection Validate.
2. gameplay publisher Validate/Publish로 Encounter/bootstrap 동기화.
3. 기존 counter/phase3 portal focused contract에서 오래된 exact 수치만 새 계약으로 교체.
4. `git diff --check`와 변경 JSON parse.
5. 실행 중인 Client/Server가 없는 것을 확인한 뒤 Debug Product, Core, 마지막으로 FullDiagnostic.

에이전트는 Client를 실행하지 않는다. build가 닫히면 사용자가 직접 다음을 판정한다.

- Triple Counter 세 window와 Trash 계열 counter window에서 Q/W/E/R 실제 적중 시 authored groggy branch,
  허공 입력·proxy 밖·window 밖은 무반응.
- 부활한 본체가 여섯 스킬을 정해진 순서로 반복하고 스킬 사이 순간 이동하지 않음.
- 보조 유령은 walkable 위치에 최대 하나만 나타나 한 스킬 뒤 사라지고 다른 위치/스킬로 반복.
- 세 leg의 portal pair가 반지름 7m 정삼각형에 동시에 생기고 0.3초 뒤 collider 3개가 동시에 이동,
  1.6초 도착, 1.9초 dissolve, 0.3초 공백 뒤 다음 삼각형 전체를 반복.
- 본체 사망/reset/방 종료 뒤 보조 유령과 portal combat object가 남지 않음.
