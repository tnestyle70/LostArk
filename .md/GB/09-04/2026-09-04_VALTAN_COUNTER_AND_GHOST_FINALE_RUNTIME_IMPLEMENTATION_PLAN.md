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
| G04 | 본체·보조 유령과 독립된 반지름 7.5m 정삼각 포탈 루프를 실행한다 | 세 leg `V0→V1`, `V1→V2`, `V2→V0`의 실제 유령 러너를 동시에 발사한다. portal/red floor 0ms, rush 300~1600ms, dissolve tail 1600~1900ms, 소멸 뒤 3000ms 공백, 다음 occurrence는 4900ms에 시작한다 |
| G05 | 보조 유령과 포탈 러너의 반복 생성 hitch를 없앤다 | Arena load에서 ghost prototype과 동시 최대 4개 presentation을 미리 준비하고, immutable joined presentation을 revision/signature 기준으로 재사용한다. 랜덤 스킬 선택은 유지하되 visible spawn에서 model clone·전수 file hash/parse를 하지 않는다 |
| G06 | 시작·사망·reset·teardown 경계를 한 owner가 닫는다 | main cursor, auxiliary occurrence, portal occurrence/timer와 presentation checkout이 서로 독립이고 phase3 owner가 유효하지 않으면 child·runner·owner combat object가 남지 않는다 |
| G07 | 마력구 1000 피해 실패 전멸의 타격 표현을 닫는다 | `VALTAN_STAGGER_SLOT/FINAL_ATTACK`의 2900ms 전멸 hit에 기존 `boss.valtan.six.sonic`과 함께 `boss.valtan.twohand` V2 group 및 `G_Voltan2_Attack25_Shot2` Sound가 한 번 재생된다. 성공 Groggy branch에는 재생되지 않는다 |

정삼각형 수치는 다음으로 고정한다.

```text
circumradius = 7.5m
vertices at startAngle 30deg, step 120deg
edge = 7.5 * sqrt(3) = 12.9903810568m
travel = 1.3s
speed = edge / 1.3 = 9.9926008129m/s
portal lifetime = 1.9s
post-dissolve gap = 3.0s
occurrence start interval = 4.9s
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

## 4. G04 동시 삼각 포탈 세 leg와 실제 유령 러너

기존 구현의 arena center 기준 radial volley 3개와 `NEXT_RADIAL_SLOT` 방향 정책은 한 번에 폐회로를 만드는 데
맞으므로 유지한다. synthetic boss가 combat object만 만든 현재 구현은 실제 유령 entity가 없어 animation과
boss-root V1 red-floor를 재생할 수 없으므로, 중앙 scheduler가 damage object와 presentation runner를 함께 소유한다.

1. synthetic owner는 immutable arena center와 world yaw 0을 사용한다.
2. 반지름 7.5m, start angle 30도, step 120도의 세 vertex를 계산한다.
3. radial volley count 3을 원자적으로 spawn하고 각 ordinal은 현재 vertex에서 다음 vertex를 향한다.
4. 같은 occurrence에서 `BOSS_VALTAN_GHOST` presentation runner 3개를 세 vertex에 stage한다. runner는
   `VALTAN_GHOST_PORTAL_ONCE / ACTIVE / valtan.ghost.portal-once.active` action을 사용하고 targeting·일반 AI·nav를 타지 않는다.
5. 세 combat object와 runner transform은 300ms까지 정지하고 300~1600ms에 12.99038106m를 동시에 이동한다.
   runner body는 0~300ms 숨기고 그 뒤 표시한다.
6. 1600ms 도착에서 runner만 despawn하고 portal combat object는 1900ms lifetime까지 tail을 유지한다.
7. portal 완전 소멸 뒤 3000ms를 비운 뒤, 성공 spawn 기준 4900ms에 다음 삼각형 전체를 발사한다.

포탈은 `VALTAN_WARP`와 같은 authored world-transform 이동이며 navigation을 전혀 소비하지 않는다. boss spawn
XYZ와 world yaw 0으로 계산한 세 vertex의 finite 값, 정확한 반지름·변 길이와 volley tuple만 검사하고, navigation
load/sample, exact-walkable, same-deck, height-transition, line-of-sight 검사나 projection은 하지 않는다. 세 vertex를
모두 검증한 뒤 하나의 transaction으로만 spawn하므로 일부 leg만 남는 경로도 없다. full-footprint walkable-nav
검증은 G03의 보조 유령 spawn에만 적용한다.

combat-object movement에는 일반화된 `startDelayMs`와 `expireOnDistanceEnd`를 저작/Product/Server runtime에
추가한다. 기존 object는 기본값 0/true로 종전 의미를 유지하고 이 portal object만 300/false를 쓴다.
반복 공백은 finale의 data-owned `portalRepeatGapMs=3000`으로 두고 scheduler가 `stage duration 1900 + gap`으로
다음 due tick을 계산한다. runner는 보조 유령 최대 1개/cadence에 포함하지 않는 server-only dependent role이며,
damage를 따로 만들지 않는다. 중앙 scheduler만 volley를 한 번 실행해 3 runner가 각 3개씩 중복 생성하지 않게 한다.

Client visual은 검증을 마친 기존 `boss.valtan.portal` V2 group을 재사용한다. group root는 leg start/yaw이고,
기존 `boss.valtan.portal.black_1`, `cyan_1` leaf pair를 local Z=0에 둔다. 동시 폐회로에서는 각 leg의 destination이
다음 leg의 start와 정확히 같으므로 세 root만으로 각 변의 양 끝과 정삼각형의 세 꼭짓점을 모두 표현한다. 같은
vertex에 start/end pair를 두 번 겹쳐 밝기와 budget을 두 배로 만드는 구성은 사용하지 않는다. 세 portal은 0ms에
함께 생겨 1900ms까지 유지·dissolve한다. combat-object V2 child는 spawn 때 확정된 world root를 유지하므로 moving
damage proxy의 snapshot pivot이 portal을 끌고 가지 않는다. 기존
`effect.valtan.project-tuned.sequence.warp.portal`의 `dash-charge-red-floor` cue는 runner의
`VALTAN_GHOST_PORTAL_ONCE` occurrence에 붙여 각 start root/yaw에서 재생한다. 실제 runner는 기존 dependent boss
spawn/snapshot/despawn wire를 재사용하며 Shared에 별도 presentation packet을 만들지 않는다.

변경 파일은 portal authoring/combat-object/presentation JSON, BossCatalog, 기존 V2 portal group registry,
Valtan pipeline/publisher, GameplayCatalog parser/struct, CombatObjectRuntime과 GameRoom portal scheduler다.
기존 world-entity snapshot의 immutable `PortalRushRoute`를 소비하며, main의 raid-entry vote와 통합한 wire는
protocol 55로 고정한다.

---

## 5. G05 유령 presentation 프리웜과 재사용

첫 유령에서 body/animset을 동기 로드하고, 이후에도 entity마다 `CValtan` clone과 129개 presentation closure의
read/hash/parse를 반복하는 것이 hitch의 원인이다. 랜덤 skill 선택과 Server occurrence는 그대로 두고 visible
spawn의 Client 준비 비용만 제거한다.

1. `Loader::Ready_For_ValtanArena`가 기본 발탄과 함께 `BOSS_VALTAN_GHOST` prototype을 batch prewarm한다.
2. joined animation/effect/sound/combat-sound/shake admission 결과는
   `(level, pinned gameplay/presentation revision, archetype/model signature)` key의 immutable cache로 한 번만 준비한다.
3. phase 3 진입 전에 보조 유령 1개와 동시 portal runner 3개, 총 4개의 `CValtan` presentation instance를
   checkout 가능한 상태로 준비한다. visible spawn/despawn은 새 clone이나 disk reload 대신 checkout/checkin한다.
4. checkin은 V1/V2 owner effect, sound/effect occurrence set, animation cursor, interpolation sample, hit flash,
   action/pattern/owner/net identity를 모두 reset한다. reset이 완결되지 않으면 풀 재사용을 승인하지 않는다.

Server는 랜덤 auxiliary occurrence와 runner world entity identity/transform authority를 계속 소유한다. Client pool은
presentation 최적화일 뿐 gameplay entity를 미리 만들거나 fake snapshot을 제출하지 않는다.

---

## 6. G07 마력구 실패 전멸 타격 표현

Server 정본은 이미 `VALTAN_STAGGER_SLOT/FINAL_ATTACK`의 2900ms에 `CIRCLE 100m` 전멸 hit을 실행한다.
같은 `VALTAN_STAGGER_SLOT.FINAL_ATTACK.composition.clip.01` occurrence의 2900ms에
`boss.valtan.twohand` V2 group을 추가한다. 기존 `boss.valtan.six.sonic`은 광역 wipe layer이므로 보존하며,
두 group 모두 `b_effectroot`, snapshot, `NATURAL`, once를 사용한다.

Pattern Sound builder가 만든 `cue.sound.valtan.semantic.stagger-slot.final-attack.impact-2900`의
`G_Voltan2_Attack25_Shot2` row는 이미 같은 2900ms에 존재한다. 새 Sound 우회 경로를 만들지 않고 이 exact row와
V2/hit alignment를 focused contract로 고정한 뒤 publisher를 다시 실행한다. 1000 피해 달성 시 CHANNEL에서
`VALTAN_GROGGY_FOLLOWUP`으로 분기하므로 FINAL_ATTACK occurrence 자체가 없고 두 Effect/Sound도 재생되지 않는다.

---

## 7. 검증과 사용자 인계

새 광역 harness는 작성하지 않는다. 구현 후 다음만 수행한다.

1. author script Validate/Apply와 Valtan master projection Validate.
2. gameplay publisher Validate/Publish로 Encounter/bootstrap 동기화.
3. 기존 counter/phase3 portal/prototype admission focused contract에서 오래된 exact 수치만 새 계약으로 교체.
4. `git diff --check`와 변경 JSON parse.
5. 실행 중인 Client/Server가 없는 것을 확인한 뒤 Debug Product, Core, 마지막으로 FullDiagnostic.

에이전트는 Client를 실행하지 않는다. build가 닫히면 사용자가 직접 다음을 판정한다.

- Triple Counter 세 window와 Trash 계열 counter window에서 Q/W/E/R 실제 적중 시 authored groggy branch,
  허공 입력·proxy 밖·window 밖은 무반응.
- 부활한 본체가 여섯 스킬을 정해진 순서로 반복하고 스킬 사이 순간 이동하지 않음.
- 보조 유령은 walkable 위치에 최대 하나만 나타나 occurrence별 무작위 스킬 하나를 쓰고 사라지며,
  최초와 이후 생성 모두 눈에 띄는 frame hitch 없이 반복.
- 세 portal/red-floor와 실제 유령 runner가 반지름 7.5m 정삼각형에 동시에 생기고, 0.3초 뒤 세 runner와
  collider가 동시에 이동, 1.6초 runner 도착/소멸, 1.9초 portal dissolve, 이후 빈 3초 뒤 4.9초에 반복.
- 본체 사망/reset/방 종료 뒤 보조 유령, portal runner와 portal combat object가 남지 않음.
