# 발탄 잡기·버러지·유령·포탈·중앙 이동·독립 도넛·사망 구현 계획

## G00. 현재 작업 기준과 구현 순서

기준은 `C:/Users/user/Desktop/LostArk`의 현재 working tree다. 2026-08-28 마지막 계획 갱신
시점의 브랜치는 `codex/team-lan-host-10-207-18-103`, HEAD는 `a20d625f`다. 병렬 세션이
merge를 계속하는 동안 브랜치와 HEAD가 앞선 조사 기준에서 이미 바뀌었으므로, 이 값은 구현
base가 아니라 계획 작성 snapshot이다. 구현 시작 시 HEAD·upstream·ahead/behind와 dirty
file을 다시 실측하고, 선행 PR이 실제 포함된 사용자가 지정한 최신 base에서 별도 bug-fix
브랜치를 만든다. 기존 Flow/Next/Reload, 피자·고공 점프, Effect·맵 튜닝 변경은 보존한다.

이 문서는 구현 계획이다. 실제 구현·자동 검증·사용자 화면 검증은 대응 RESULT에서
구분한다. 기존 계획의 변경점은 아래 G별로 명시하며 과거 RESULT의 완료 문구를
현재 코드의 증거로 사용하지 않는다.

| G | 작업 단위 | 실제 소비 경로 |
|---|---|---|
| G01 | 손바닥 고정과 제한 거리 정면 발사 | Server attachment/release → Shared player state → Client attachment |
| G02 | 버러지 네 ID와 유한 분기 | split gameplay → publisher → GameRoom/ValtanBrain |
| G03 | 포탈 사각 모서리 이동과 복귀 | stage motion → Server transform → 기존 boss snapshot |
| G04 | 피자·3시·9시 중앙 이동 뒤 점프 | serverMotion → leap → world event 및 Effect anchor |
| G05 | 본체·유령 공통 6공격 최종 루프 | dependent boss spawn → 공통 Brain pattern → CValtan → owner 종료 정리 |
| G06 | 도넛의 독립 수명 | stage ENTER → CombatObject → 독립 timed hit/Effect |
| G07 | HP 0 사망 표현 뒤 제거 | Server death despawn → Client one-shot death clip → Layer 제거 |
| G08 | 실패 경로·회귀·인계 | publisher + protocol/Server/Client native harness |

세 비평 에이전트가 잡기/버러지/포탈, 도넛/앵커, 유령/복제 경로를 독립 조사했다.
그 결과를 실제 함수와 JSON으로 대조한 뒤 아래 설계에 반영했다. 수정 후에도 같은
경계를 다시 비평하고, 지적은 재현 가능한 테스트로 바꾼다.

### G00-1. 구현 중지와 사용자 재현 입장 조건

사용자의 2026-08-28 지시에 따라 이 문서 갱신 뒤에는 C++·JSON·생성물·project 파일을
수정하지 않고 commit/push/PR/merge도 하지 않는다. 먼저 현재 merged SHA를 Server +
Client profile로 사용자가 직접 재현한다. 사용자 관찰은 현재 결함의 기준선이며 자동
PASS를 대신하지 않는다. 구현 재개는 사용자가 재현 결과와 함께 수정 진행을 다시
명시한 뒤에만 허용한다.

재개 시에는 다른 사람의 PR이 `origin/main` 또는 사용자가 지정한 통합 base에 들어간
SHA를 먼저 확인한다. 현재 snapshot은 HEAD와 `origin/main`이 모두 `a20d625f`이고
ahead/behind는 0/0이지만 병렬 merge 뒤에도 같다고 가정하지 않는다. 최신 base에서 새
`codex/valtan-ghost-trash-grab-fix` 브랜치를 만들고 이 계획의 변경만 적용한다.

## G01. 고정 손바닥 부착과 제한 거리 정면 발사

### 현재 기준점

현재 Client의 Trash와 `VALTAN_CATCH_BREATH`는 모두
`ApplyPatternHit → Capture_PlayerAttachment → player snapshot →
Update_PlayerAttachmentPresentations → bip001-l-hand` 경로를 사용한다.
`CPlayerHandGripTransform::Build_LocalOffset`도 player/hand translation을 제거하므로
Client presentation에는 패턴별 거리 보존 분기가 없다. 반면 Server의
`Capture_PlayerAttachment`는 포획 시점의 boss-local X/Y/Z와 yaw 차이를 저장하고
`Update_PlayerAttachment`가 매 tick 다시 적용한다. 뒤쪽 넓은 cone에서 잡는 Catch가
접촉 capture인 Trash보다 손에서 멀어 보일 수 있는 차이는 이 authoritative snapshot
history와 fallback/release seam에 남아 있다.

현재 `VALTAN_CATCH_BREATH` release는 `ARENA_EJECTION`, `24m/s × 500ms`다. 바로 이전
정본은 `OPPOSITE_KNOCKBACK`, `12m/s × 500ms`였고, 후속 변경에서 속도가 두 배가 되면서
아레나 밖 강제 이탈 정책으로 바뀌었다. 최신 요청의 "이전 값 기준 1.5"는 우선
`12m/s × 1.5 = 18m/s`로 해석한다. 500ms 동안 명목 이동 거리는 9m다. 다만 사용자가
`1.5m`의 고정 이동 거리를 뜻했을 가능성은 현재 문장만으로 확정할 수 없으므로 구현 전
기준선 테스트에서 확인한다. `1.5m`가 맞다면 500ms 기준 속도는 3m/s이며 18m/s를 적용하지
않는다.

속도만 24에서 18로 낮추는 것으로는 요구를 만족하지 못한다. `Prepare_ArenaEjection`은
속도와 무관하게 최종 위치를 `마지막 아레나 지면 + 외부 여백` 이상으로 늘려 강제
FALLING을 만들기 때문이다. 따라서 Catch는 `ARENA_EJECTION`을 재사용하거나 의미를
바꾸지 않고, 발탄 현재 정면으로 유한 이동한 뒤 nav 경계에서 clamp되는 명시적
`BOSS_FORWARD_KNOCKBACK` release mode를 사용한다. 방향도 현재
`(-sin(yaw), -cos(yaw))`에서 `(+sin(yaw), +cos(yaw))`로 바로잡는다.
`PlayerController`의 GRABBED 입력 차단과 release 뒤 새 press latch는 그대로 재사용한다.

### 변경 파일과 상태

- `Server/Private/GameRoom.cpp`: `BOSS_LEFT_HAND` 포획의 Server local
  X/Y/Z/yaw를 모두 0으로 정규화한다. 새 `Prepare_BossForwardKnockback` typed helper가
  release owner boss yaw의 `(+sin,+cos)`와 선택된 speed/duration으로 velocity를 직접
  stage하고, 기존 유한 player knockback/nav clamp advance에 commit한다. generic
  `Release_PlayerAttachment`의 radial/fallback 방향을 정면 방향으로 오인해 재사용하지 않는다.
- `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp`,
  `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`,
  `Tools/ValtanPipeline/valtan_tuning_pipeline.py`와 Client strict parser:
  새 release token을 같은 grammar로 검증하고 unknown token을 거부한다.
- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`와
  `ServerGameplayContractTests.cpp`: 서로 다른 Catch/Trash 포획 거리가 같은 zero snapshot을
  Server에서 만드는지 검증한다. Shared v45 codec은 mixed peer 호환을 위해 기존 finite
  bounded nonzero offset admission을 유지하고 zero/nonzero roundtrip을 모두 받되 NaN과
  invalid slot은 계속 거부한다. packet shape, header 의미와 protocol version은 바꾸지 않는다.
- `ClientReplication` 구현에는 Catch 전용 분기를 추가하지 않는다. 현재 공통 손 본 합성을
  유지하고 `EffectRenderContractHarness`에서 먼 포획 위치·180도 회전·0.01/1.0 scale을
  같은 손 원점으로 합성하는 기존 회귀를 제품 두 pattern join까지 확장한다.
- `Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py`와
  `Data/Valtan/Valtan.gameplay.json`: Apply가 Catch를 다시 `ARENA_EJECTION/24`로 덮지 않도록
  author helper부터 고치고 Catch terminal release를
  `BOSS_FORWARD_KNOCKBACK`, 잠정 `18m/s × 500ms`로 바꾼다. 이는 사용자 확인 전 적용하지
  않는 계획값이며, `1.5m` 의미가 확인되면 `3m/s × 500ms`로 계획부터 교정한다.

호출 흐름은 `Commit_BossPatternPlayerStageActions`의 preflight에서 boss transform과 선택된
speed/duration을 `Prepare_BossForwardKnockback`에 전달하고, attachment release와 explicit
velocity를 같은 transaction으로 commit한 뒤 기존 `Advance_PlayerKnockback`을 사용한다.
두 번째 player 이동 런타임을 만들지 않는다.
발사 방향은 포획 순간의 우연한 radial offset이 아니라 발탄의 현재 정면 방향으로
결정한다. yaw 0/90/180/270은 각각 +Z/+X/-Z/-X다. ordinary bounded knockback이므로
Catch 자체가 `isCombatReady=false` 또는 강제 FALLING을 만들지 않는다. 실제 nav 밖으로
떨어지거나 다른 피해로 죽은 경우의 death/revive/reset/disconnect 정리는 기존 경로를
유지한다.

종료 증거는 서로 다른 포획 위치의 동일 hand-local translation, 사용자 확인 뒤 선택된
속도/거리, 발탄 정면 방향, nav clamp, Catch가 강제 FALLING을 만들지 않음이다. 1.5배
해석을 선택한 경우에만 18m/s와 500ms의 명목 9m를 exact test 값으로 쓴다. 실제
손바닥·캐릭터 몸통의 미세 높이 정렬과 체감 이동 거리는 사용자 화면 확인 대상이다.

## G02. 버러지 네 패턴의 공통 분기와 종료

### 현재 기준점

현재 merged 정본의 `VALTAN_TRASH`는 순환 graph가 아니다. 도입 `STEP_01..06` 뒤
`STEP_07` counterable windup과 `STEP_08` rush를 한 번 실행하고 NONE/PARTIAL/ALL/COUNTER
결과가 모두 terminal인 14-stage 유한 DAG다. 따라서 현재 문제는 무한 반복이 아니라
사용자가 원하는 총 3회 시도가 아직 없다는 점이다. `_CATCH_IF`는 같은 capture fragment,
`_CATCH_SUCCESS/_FAIL`은 결과 tail 확인용 단발 wrapper다.

### 변경 흐름

공통 저작 fragment는 시전·돌진, 성공 tail, 실패 tail이다. main과 세 보조 ID는 같은
fragment를 소비한다. main에서 다른 patternSequence로 attachment를 넘기지 않는다.

| ID | 역할 |
|---|---|
| `VALTAN_TRASH` | 기존 도입 1회 → 시전 counter/돌진/결과를 최대 3회 → 종료 |
| `VALTAN_TRASH_CATCH_IF` | 도입 없이 같은 최대 3회 시전·돌진·결과 분기의 단독 진입 |
| `VALTAN_TRASH_CATCH_SUCCESS` | 같은 포획 성공 tail의 단독 확인; 포획을 임의 생성하지 않음 |
| `VALTAN_TRASH_CATCH_FAIL` | 같은 miss/recovery tail의 단독 확인 |

`STEP_07` WINDUP의 기존 counter ENTER/EXIT와 `COUNTER_HIT → GROGGY`를 첫 시도로
보존한다. 도입은 한 번만 실행하고 rush attempt를 정확히 세 번 유한 전개한다.
1·2회차의 `RUSH_MISS`와 partial `CATCH_SLAM`은 release/damage transaction이 성공한
뒤 다음 windup으로 가고 3회차 결과만 terminal이다. 어느 회차든 counter는 공통
`GROGGY`, 전원 포획은 공통 `EXECUTE_TAIL`로 끝난다. 돌진 중에는 기존 hit pulse마다
아직 잡히지 않은 생존 플레이어를 검사하고 포획해도 해당 rush 종료까지 계속 잡는다.

정면 이동이 nav 경계에서 잘리면 `NAVIGATION_BLOCKED` 결과로 같은 tick의 최종 포획
판정 후 tail로 전이한다. collider 뒤의 플레이어를 이동 전 가상의 긴 sweep로 잡지 않는다.
`CValtanBrain`의 기존 stage transition preflight/commit과 captured ANY/ALL 분류를
재사용한다. 카운터·시간 만료·nav 종료에 별도의 selector를 만들지 않는다.

수정 대상은 `author_valtan_phase_two_mechanics.py`,
`promote_valtan_animation_chains.py`, split gameplay/presentation, 생성 Product,
관련 catalog/publisher와 기존 Server/Client contract다. runtime loop counter나 Brain 전용
selector는 추가하지 않는다. NONE/PARTIAL/ALL/COUNTER를 세 회차 각각 terminal까지 실제
fixed tick으로 검증한다.

## G03. 포탈의 독립 transform과 중앙 복귀

### 데이터와 Server 책임

현재 2초 준비·0.9초 8회 돌진·1.667초 마무리 순서는 유지한다. 각 돌진은 arena
중앙과 저작된 사각 반폭을 기준으로 모서리를 정하고, 생존 대상과 출구 방향을 stage
진입 때 고정한다. 전체 navgrid bounds를 arena 크기로 간주하지 않는다.

stage motion의 `PORTAL_CROSS_ARENA`는 `cornerIndex`, X/Z 반폭을 소유한다.
Server는 진입 위치와 출구를 계산해 기존 boss transform을 보간한다. 이 stage에서는
clip root motion과 일반 nav 이동을 함께 적용하지 않는다. collider/hit은 같은 실제
이동 경로를 소비한다. 8회 동안 같은 대상이 다시 선택되는 것은 허용한다.

마무리 stage의 `RETURN_TO_ARENA_CENTER`는 중앙의 같은 높이 walkable 위치를
먼저 검증한 뒤 boss pose를 commit한다. 실패하면 기존 pose와 실패 이유를 보존하고,
다음 일반 패턴으로 진행하지 않는다. 정상 복귀까지가 포탈 한 occurrence다.

종료 증거는 8개 leg의 모서리 배치, nav 밖 이동, 중복 root motion 부재,
마지막 walkable 복귀 및 잘못된 범위/복귀 실패 rollback이다.

## G04. 중앙에 도착한 뒤 점프·지형 파괴

현재 피자와 3시·9시에는 중앙 착지만 있다. takeoff는 패턴 시작 위치에서 시작한다.
중앙 정본은 기존 `serverMotion.landingPosition = [156.03, 22.99751, -122.06]`이다.

`serverMotion.moveToAnchorBeforeTakeoff`를 명시한 패턴은 첫 stage의 takeoff 이전
구간에 중앙으로 이동하고, takeoff window부터 중앙에서 수직 점프한다. travel stage와
기존 action/stage ID는 유지한다. 이동 목적지와 높이는 preflight에서 검증한다.

3시·9시 Effect는 중앙의 고정 world anchor와 기준 yaw를 사용한다. 사용자가 저장한
element 회전·scale·delay는 보존한다. 특히 현재 9시의 yaw/3.4초 튜닝을 되돌리지 않는다.
파괴 대상은 `worldeventset.valtan.terrain-destruction-3.floor84`와
`worldeventset.valtan.terrain-destruction-9.floor30`을 그대로 사용한다. Effect 회전이
Server 파괴 대상의 identity를 바꾸지 않는다.

### 추가 요청: 피자 섹터 두 종류와 착지 facing

피자는 생존 플레이어 최대 네 명 중 한 명을 패턴 시작 때 무작위 선택한다.
`LOCK_RANDOM_ALIVE_ON_START + LOCK_FACING_ON_START`로 선택과 회전을 고정하고,
방향 벡터는 시작 위치가 아니라 **중앙 landingPosition → 선택한 플레이어**다.
서버가 확정한 이 yaw를 중앙 접근·점프·착지 동안 유지한다. 중간 target 재선택이나
매 tick 추적을 하지 않으므로 두 Client가 서로 다른 방향을 만들 수 없다.

피자 composite cue의 `arena.center.facing`은 중앙 위치와 최초 Server patternSequence
yaw를 사용한다. `fx_o_sector_05.dds`, `fx_o_sector_04.dds`의 element와 delayed spawn은
그 world root를 공유한다. 기존 element의 상대 회전·크기·11초 지연·후속 섹터 회전은
그대로 보존한다. 3/9시의 `arena.center`는 yaw 0을 사용해 사용자 rot 튜닝을 유지한다.
단순 `ARENA_ABSOLUTE` scale 정책만으로는 위치가 고정되지 않는다는 비평을 반영해,
`CValtan`이 PatternTree의 `serverMotion.landingPosition`을 검증·캐시하고 기존
`CEffectPresentationService::Spawn_WorldRoot` allocator/commit을 재사용한다.

중앙 이동 → takeoff → landing → world event 순서를 수치로 검증한다. 정확히 부서지는
화면 영역과 사용자의 rot 튜닝은 수동 검증으로 남긴다.

## G05. 유령 발탄과 본체 공통 6공격 최종 패턴

### 기존 객체 경로의 확장

유령은 독립 Server `BOSS` entity다. stable archetype은 `BOSS_VALTAN_GHOST`,
소유자는 `iOwnerBossNetEntityId`로 식별한다. 별도 boss stage evaluator나 local clone
런타임을 만들지 않는다. 기존 spawn/snapshot/despawn, `CValtanBrain`, `CValtan`,
`CBody_Valtan`, `CModel → CMaterial`을 재사용한다.

유령은 하나만 transactionally 생성한다. 아레나 중앙 주변 X/Z 후보를 Server가
결정적으로 샘플링하고 현재 파괴 상태·nav·같은 높이를 검사한다. spawn/payload/entity
commit이 모두 성공한 뒤에만 해당 finale activation의 spawn-consumed latch와 child ID를
확정한다. 생성 뒤 같은 activation에서 한 ghost entity가 유지되며 6번째 공격 뒤
소멸·재생성하지 않는다. 예상 밖 child 소실이나 reset-required는 재생성으로 숨기지 않고
fail-closed한다.

`VALTAN_GHOST_FINALE`의 public metadata는
`BOSS_PATTERN_FINALE_KIND::GHOST_ATTACK_LOOP`, JSON key `kind: GHOST_ATTACK_LOOP`다.
기존 호환을 위해 JSON/C++ 목록 이름은 `ghostPatternIds`/`GhostPatternIds`를 유지하되
본문과 유령이 함께 소비하는 exact six attack list로 정의한다.

1. `VALTAN_WHIRLWIND`
2. `VALTAN_FOUR_SLASH`
3. `VALTAN_SEQUENCE_FOUR`
4. `VALTAN_HIGH_JUMP`
5. `VALTAN_CATCH_BREATH`
6. `VALTAN_ATTACK_WHIRLWIND`

finale 자체의 finite graph는 기존 `STEP_01` 2000ms 하나만 유지한다. 이 stage는
`hit=NONE`, `motion=null`, event 없음, `defaultNextActionId=null`, `TIMEOUT → null`인
spawn/preparation occurrence다. 기존 finale `STEP_02..10`의 portal gameplay,
presentation binding, effect cue, sound/root-motion 파생 row를 제거한다. 범용
`PORTAL_CROSS_ARENA` 지원과 `VALTAN_WARP`의 8회 target-rush 계약은 이번 변경에서
그대로 유지한다.

본체와 유령은 각자 독립 cursor를 가지되 같은 finale-loop selector와 기존
`CValtanBrain::BeginPattern` 경로로 위 여섯 실제 정의를 실행한다. 개별 pattern graph는
항상 유한하고 반복은 pattern 사이 cursor만 `(index + 1) % 6`으로 감는다. 각 실제 공격은
새 patternSequence를 발급하며 본체의 outer Product/Debug Flow cursor를 내부 cursor로
재사용하지 않는다. Saved Flow의 마지막 slot은 계속 `VALTAN_GHOST_FINALE` 하나인
composite slot이고 여섯 공격을 사용자 slot 뒤에 자동 추가하거나 저장 순서를 바꾸지 않는다.
최종 composite 수명은 본체 사망 또는 명시적 Stop/Next seam이 결정한다.

### Asset·Client 책임

실제 ghost body는 `Character/Valtan/Ghost/MN_RPBF_02.wmodel`, animation donor는
`Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel`, 도끼는 기존
`Character/Valtan/ValtanWeapon.wmodel`이다. ghost body/weapon admission scale은
각각 `0.01/1.0`, 기존 본체는 `0.0001/100.0`으로 구분한다. presentationScale 1.0은
바꾸지 않는다. catalog가 variant와 admission scale을 소유한다.

`ActorCatalog`, `ValtanPresentationAssetService`, `CValtan`/`CBody_Valtan`은
archetype별 model prototype을 선택한다. shared GameObject prototype은 재사용한다.
ghost는 본체 HUD, `ValtanPresentationState`, Boss Tool target, BGM 시작/정리를
소유하지 않는다. ghost despawn이 본체 HUD/BGM을 지우지 않게 primary/dependent를
구분한다. 본체의 owner 0은 정상이며 유령의 owner 없음·자기 소유·다른 방 소유·
종속 entity의 소유·잘못된 duplicate는 commit 이전에 거부한다. 유령은 본체와 같은
높이 및 body radius만큼 유효한 nav 영역에서 생성하며 플레이어 공격 대상이 아니다.
ghost donor 실패는 해당 자식의 생성 표현만 실패시키며 본체 표현은 유지한다.

Shared spawn의 owner ID와 primary-before-dependent admission은 이미 현재 경로에 있다.
이번 후속 수정은 그 wire shape나 Network protocol version을 올리지 않는다. 본체
죽음·reset·room-empty·퇴장·Next·Stop에서는 자식 Effect/entity, 양쪽 combat object와
attachment를 함께 정리한다. 늦은 입장은 현재 본체를 먼저 받고 같은 activation/revision의
자식과 실제 진행 stage를 이어서 받는다. ghost는 계속 플레이어 공격 대상, HUD/BGM owner,
Boss Tool primary target으로 승격하지 않는다.

플레이어 외형 복제는 이번 요청에 구체적인 공격·소유권·수명 정의가 없으므로 유령
발탄 구현과 혼동해 완료로 기록하지 않는다. 이번 구현의 분신은 위 ghost boss다.

## G06. 도넛 Effect와 발탄 foreground 수명 분리

### 현재 기준점과 변경

`VALTAN_FIST_IN_OUT/INNER`는 `2600ms + animation NONE`으로 발탄을 붙잡고
1600ms에 RING hit를 판정한다. NATURAL Effect 보존 자체는 이미 존재한다.
`CCombatObjectRuntime`은 spawning action보다 오래 사는 room 소유 object 목록,
transaction, definition pin, timed RING, late join과 cleanup을 이미 제공한다.

도넛을 `combatobject.valtan.fist-in-out.donut`의 FIXED_AREA/STATIC/BOSS_POSITION으로
저작한다. 수명 2600ms와 1600ms RING(8~16m)을 object가 소유한다. INNER는 짧은
ENTER trigger이며 object commit 뒤 foreground를 끝낸다. 기존 stage hit와 boss cue를
제거하여 damage와 Effect의 중복 발생을 막는다. 다음 패턴은 독립 도넛 수명에 묶이지 않는다.
완료된 foreground의 patternSequence가 소유한 독립 object가 아직 살아 있으면 기존
inter-step pursuit 대기도 0으로 소비한다. 특정 도넛 ID를 scheduler에 하드코딩하지 않는다.

`Valtan.combatobjects.json`의 optional explicit lifetime과 일반
`SPAWN_COMBAT_OBJECT` owner를 split validator/compiler에 연결한다. 기존 volley의
기본 수명·owner 계약은 유지한다. `BossCatalog.combatObjectVisuals`의 optional worldScale이
기존 도넛 scale 1.5를 보존한다. independent Effect tree도 같은 combat-object owner를
가리킨다. Effect 파일을 다른 폴더로 publish하지 않는다.

도넛의 다섯 element는 `localSpace=false`로 바꾼다. birth world root를 모든 element가
공유하고, delayed spawn도 발탄의 나중 transform을 다시 읽지 않는다. 기존 wave의
`groundTangentClockwise` 방향을 보존하기 위해 `Effect_DocumentCodec`의 local-only
제약과 `Effect_Playback`의 birth orientation 조건을 함께 수정한다. sourceRecipe의
소유권을 끄거나 unrelated element 설정을 덮지 않는다.

`Effect_Tool`도 ring 방향 선택 시 localSpace를 강제로 켜거나 선택을 잠그지 않는다.
world root를 동결하면 inner.grow의 scale 17→41.3 lerp까지 멎는 문제를 비평에서
확인했다. manual world-space particle은 출생/현재 element scale 비율만 적용하고,
위치·yaw는 고정한다. sourceRecipe 동작을 바꾸지 않으며 0/NaN scale 실패도 검증한다.

종료 증거는 도넛 생존 중 다음 패턴 시작, 이동 후 원래 중심의 피해, 다중 occurrence,
late join, 1회 hit, owner death/reset 정리, world-space wave 방향이다.

## G07. HP 0 사망 애니메이션과 제거

### 현재 기준점과 소유권

BossCatalog의 본체·유령 `presentationClips.dead`는 `mesh_dead_1`이다. 기존
`CValtan`에는 DEAD snapshot의 clip 선택 코드가 있지만 `CBody_Valtan::Update`는 DEAD의
pose advance를 차단한다. Server의 사망 sweep도 MONSTER만 제거하므로 BOSS가 남는다.
두 donor의 실제 `mesh_dead_1`은 110ticks/30Hz(약 3.667초)이며 non-loop다.
Server에 model clip 길이를 중복 저장하지 않는다.

`Shared`의 기존 `S2C_WORLD_ENTITY_DESPAWNED`에 일반 제거와 사망을 구분하는 typed
reason을 추가한다. 버전이 다른 packet과 알 수 없는 reason은 거부한다. Server는 HP 0에서
패턴·포획·독립 피해·종속 유령을 정리하고 사망 reason으로 authoritative boss를 제거한다.
DEAD snapshot이 먼저 도착할 것이라고 가정하지 않는다.

Client는 사망 despawn을 받으면 해당 boss의 gameplay registry/HUD 연결을 즉시 끊고,
기존 Layer의 presentation만 사망 clip 완료까지 보존한다. `CBody_Valtan`의 실제 model
clip 조회와 animation completion을 재사용한다. clip이 없거나 재생을 시작할 수 없으면
즉시 제거한다. DEAD snapshot과 despawn을 모두 받아도 clip은 한 번만 시작한다.
죽는 중 새 보스가 등장해도 이전 객체가 새 singleton/HUD/BGM을 지우지 않는다.
room reset·level leave·disconnect는 남은 사망 presentation도 즉시 정리한다.

수정 파일은 `Shared/Public/Network/PacketMessages.h`, 대응 cpp와 protocol harness,
`Server/Private/GameRoom.cpp`, `ClientReplication.h/.cpp`, `Valtan.h/.cpp`,
`Body_Valtan.h/.cpp`, `ActionPresentationTimeline.h`의 `CDeathPresentationClock`와
기존 presentation harness다. 새 GameObject/모델 런타임이나
새 C++ 파일을 만들지 않으므로 project/filter에 새 entry를 추가할 필요는 없다.

종료 증거는 HP 0 뒤 authoritative entity/자식/피해 제거, death reason roundtrip과
unknown reason 거부, 사망 clip one-shot·없음·재전송·중도 reset 정리다. 사망 애니메이션의
실제 화면 모양은 사용자 확인 대상으로 둔다. 사용자 요청 범위는 이 항목으로 닫는다.

## G08. 비평·검증·인계

기존 하네스를 확장한다. `ServerGameplayContractTests`는 실제 room tick과 실패
transaction을, `NetworkProtocolHarness`는 owner wire roundtrip/오류를,
`EffectRenderContractHarness`는 world-space particle와 grip 계산을 검사한다.
`Tools/ValtanPipeline`과 `Tools/EffectPipeline`의 기존 suite에 schema/owner/분기
검사를 추가한다. 파일 추가가 필요하면 실제 소비자와 `.vcxproj/.filters` 등록을 함께 둔다.

확장 형식을 읽는 소비자도 같은 변경에서 갱신한다. `EncounterPatternReference.cpp`의
구형 optional-field 목록과 event/motion parser를 finale·portal·ejection·center anchor
계약에 맞춘다. `Level_Loading.cpp`, `Level_CharacterSelect.cpp`,
`CharacterSelectArenaSpawnGate.h`의 전투 이펙트 2개 고정 조건은 검증된 catalog 목록을
순회하도록 바꾼다. `ClientPartyRegression.cpp`의 기존 실제 Load fixture에서 정상
제품 문서와 잘못된 필드·순환 분기·복귀 단계의 거부, 기존 문서 보존을 검사한다.
`Publish-WorldGameplay.ps1`도 finale 필드를 받아 기존 encounter-owner 배치 검증을
유지하며, 유령의 정적 배치 거부는 임시 입력을 사용하는 실행 테스트로 확인한다.

본체와 유령은 같은 encounter를 사용하므로 timeline owner 수는 dependent profile을
제외해 계산한다. `Build_WorldEntity`는 유령에 살아 있는 본체 owner와 같은 encounter,
고정 revision, 같은 finale activation을 요구한다. 초기 진입 복제는 본체를 먼저 보내고
고아·중복 엔티티를 거부한다. 이 admission은 Client의 owner wire 검증과 함께 검사한다.
같은 composite Flow slot 안의 각 실제 6공격은 새 patternSequence를 보고하지만 outer
slot/cursor는 유지한다. 세 번째 loop의 Next·Stop이 마지막 실제 공격 sequence를
predecessor로 사용해 successor 1회 또는 terminal hold로 끝나는지 Server/Client 양쪽에서
검사한다.

추가 사용자 지시에 따라 `Fix next pattern selection flow` 작업의 G15 계약을 통합 기준으로
삼는다. 등록 패턴 29개나 Core 8개·Animator 20개를 요구하지 않는다. 검증된 split
gameplay/presentation 정의를 `Build_PlayablePatternInventory`가 하나의 재생 목록으로
만들고 Boss Tool·Play Selected·Next·All Effects·저장 Flow와 publisher가 같은 집합을
사용한다. Core/Animator/Derived는 표시 분류다. 현재 31개 정의라는 관찰은 승인 제한이
아니며 새 유효 패턴의 추가·제거를 허용한다. 유령 finale와 독립 도넛도 이 경로로 선택한다.
Flow의 1~255슬롯 U8 전송 용량과 256슬롯 거부, 알 수 없는 ID·중복 소유자·손상된 조인 거부는 별도 안전 계약으로
유지한다. 기존 저장 순서를 자동 재배열하지 않고 Reload는 화면의 첫 슬롯부터 시작한다.
상세 G15 규격과 기존 resetless Next/Save 적용 계약은 대응
`2026-08-28_VALTAN_BOSS_TOOL_RESETLESS_NEXT_PATTERN_IMPLEMENTATION_PLAN.md`를 따른다.

검증 순서는 Engine Debug/Release → UpdateLib → Shared/Network harness → Server
contract → Client Debug/Release → Effect/Action 하네스 → 변경 domain Validate/Check →
`Invoke-BuildAndRegression.ps1` → JSON/XML parse → `git diff --check`다. 다른 실행 중인
빌드 결과를 이번 변경의 PASS로 사용하지 않으며 최종 소스로 다시 검사한다.

Client/UI는 에이전트가 실행·조작·캡처하지 않는다. 사용자는 Server + Client profile로
Ctrl+F5 → Lobby → Valtan → F1 Boss Tool에서 각 pattern ID를 직접 확인한다.
Effect Tool에서는 기존 저장 회전과 중앙 anchor, 도넛 이동 독립성을 확인한다.
구현, 자동 검증, 미실행 수동 검증을 RESULT에서 분리하고 large dirty worktree를
자동 stage/commit하지 않는다.

### 요구별 비평에서 발견해 반영할 계약

| 요구 | 코드로 확인한 문제 | 반영·검증 경계 |
|---|---|---|
| 왼손 | Server snapshot이 capture 상대 translation을 유지, 실제 0.01 본 scale 경계 존재 | Server zero grip + 기존 Client 본 원점 + 실제 scale fixture/실패 rollback |
| 발사 | 현재 ejection은 반대 방향이며 속도와 무관하게 아레나 밖을 강제함 | boss-yaw `(+sin,+cos)` bounded knockback, Catch no-forced-FALLING, 기존 ejection grammar와 일반 knockback 보존 |
| 버러지 4종 | 현재 14-stage 1회 유한 DAG라 요구한 총 3회가 없음 | 26/20-stage finite fragment, 회차별 counter·nav 종료 전 최종 포획 |
| 포탈 | tick 마지막 위치 누락, hit0으로 전체 경로 미검사 | 최종 pose 확정, 50ms pulse와 leg별 1회 피해 ledger |
| 중앙 파괴 | ARENA_ABSOLUTE가 scale만 변경 | typed center anchor + 기존 world-root handle 경로 |
| 피자 방향 | owner 시작점 기준 yaw와 center 섹터 기준점 불일치 | center→random target 서버 yaw, 같은 fixed root로 지연 섹터 재생 |
| 유령 | primary HUD/BGM 오염·무소유 spawn·outer cursor 되감기 | owner/activation wire 검증, dependent 필터, 공통 6공격별 새 sequence와 별도 cursor |
| 최종 slot | Core8/Animator20 고정과 Tool·Flow publisher 허용 집합 불일치 | G15 공통 split 재생 목록, 개수 제약 제거, Derived는 표시 분류, 저장 순서 보존 |
| 도넛 | foreground와 hazard 수명 결합·worldSpace scale lerp 동결 | CombatObject 2600ms/foreground100ms, birth pose와 scale curve 분리 |
| 사망 | BOSS DEAD가 제거되지 않음, despawn보다 DEAD snapshot이 먼저 온다는 보장 없음 | typed death reason, authority 즉시 종료와 Client clip 수명 분리 |
| 데이터 확장 | 같은 finale/분기의 publisher·Client·Server 허용 범위 차이 | `GHOST_ATTACK_LOOP`, exact 6개 순서·damageable·extent 1~100·전체 finite graph를 각 경계에서 검증 |
| 제품 진입 | 전투 이펙트 2개 고정 로더와 구형 EncounterPatternReference 필드 목록 | 실제 catalog 순회와 새 typed parser, Load 실패 시 기존 문서 보존 |
| 유령 초기 복제 | 같은 encounter의 timeline owner 중복과 무소유 생성 허용 | dependent 제외, owner+activation admission, 본체 우선 복제·고아/중복 rollback |
| 반복 Flow 제어 | 내부 공격 완료가 composite slot을 조기 완료 | outer slot 소유 유지, 실제 공격 sequence 재결합, 세 번째 loop Next·Stop 실행 검사 |

위 표는 설계 검토 결과이며 실행 PASS를 뜻하지 않는다. 대응 RESULT에 실제 하네스
명령과 exit code, 남은 사용자 화면 확인을 각각 기록한다.

## G09. 2026-08-28 후속 버그 수정 계약

이 절은 이번 사용자 재현을 반영한 후속 계약이며, G01의 포획 범위·거리·발사 방향, G02의
버러지 종료, G05의 유령 finale 반복 설명과 충돌하는 부분을 대체한다. 구현 전에 현재
merge 결과를 다시 조사한 결과 `VALTAN_TRASH`는 무한 graph가 아니라 한 번만 시도하고
끝나는 유한 DAG였고, Client의 Trash/Catch 부착은 이미 같은
`CPlayerHandGripTransform`과 `bip001-l-hand`를 사용한다. 반면 Server는 두 패턴 모두
포획 순간의 boss-local 거리를 계속 snapshot position으로 사용한다. 뒤쪽 8m cone에서
잡는 `VALTAN_CATCH_BREATH`는 이 값이 크고 접촉 capture인 Trash는 작으므로, 보간·fallback
경계에서 서로 다른 위치가 다시 드러날 수 있다.

추가 조사에서는 Catch의 표시와 판정 방향도 서로 다른 정본을 쓰는 것이 확인됐다.
`STEP_02`는 `CONE 120도 / 8m / CAPTURE`만 저장하고 방향을 저장하지 않으며, Server
`ContainsPatternHit()`는 모든 cone을 현재 `boss.fYawDegrees`의 정면으로만 평가한다. 반면
root-follow Catch Effect에는 서로 약 180도 반대인 sector 요소가 있고, 사용자는 그중 빨간
sector만 포획 범위여야 한다고 지정했다. raw Effect 요소의 Euler `122.25도`는 texture의
기준축과 quad 합성까지 포함한 값이므로 Server yaw로 그대로 복사하지 않는다.

이번 후속 bug-fix PR의 구현 범위는 G09-1 Catch, G09-2 Trash, G09-3 finale 세 계약뿐이다.
상단의 장기 계획 G03 포탈, G04 중앙 이동/파괴, G06 도넛, G07 사망 및 기존 G08 확장은
이번 PR에서 구현하지 않는다. 해당 항목은 별도 재개 승인과 검증 단위가 필요하다.
에스더와 발탄 타깃 도끼·도넛 사운드 추출 인계·runtime 결선은
`2026-08-28_ESTHER_VALTAN_AXE_DONUT_SOUND_INTAKE_IMPLEMENTATION_PLAN.md`가 소유하며,
이 bug-fix PR에 WAV/catalog/sound runtime 변경을 섞지 않는다.

### G09-0. 코드 변경 전 사용자 기준선 테스트

이 단계에서는 코드·데이터·생성물을 수정하지 않는다. 사용자는 실제 실행한 SHA를
`git rev-parse HEAD`로 기록하고 Server + Client profile을 `Ctrl+F5`로 시작한 뒤 Lobby →
Valtan → F1 Boss Tool에서 아래를 직접 확인한다. 에이전트는 Client를 실행·조작·캡처하지
않고 사용자가 전달한 관찰만 기준선 RESULT에 기록한다.

| 대상 | 현재 정본에서 실행할 재현 | 기록할 관찰 |
|---|---|---|
| Catch 빨간 sector | 패턴 시작 직전/`STEP_02` 판정 순간의 발탄 yaw를 기록하고 빨간 중심, 빨간 경계 안·밖, 같은 거리의 반대쪽 sector에 각각 선다 | 빨간 중심선이 판정 순간 발탄 정면 기준 0/180도 또는 다른 offset인지, boss turn 전후 어느 yaw를 따라가는지, 빨간 영역 밖의 정면에서 현재 오포획되는지 |
| Catch 손 부착 | `VALTAN_CATCH_BREATH`의 빨간 sector 안에서 발탄 가까이와 최대 포획 거리로 각각 맞는다 | 포획 즉시/발탄이 뒤도는 중/던지기 직전 중 언제 손과 거리가 생기는지, 왼손의 어느 점에 붙는지 |
| Trash 비교 | `VALTAN_TRASH` rush 접촉으로 잡힌다 | Catch와 같은 왼손 경로인지, 동일 시점에서 거리 차이가 있는지 |
| 방출 방향·거리 | yaw 0 기준 정면을 확인하고 Catch로 날아간다 | 발탄 정면(+Z)인지 후면(-Z)인지, 회전 뒤에도 현재 정면 기준인지, "이전 값의 1.5배"와 "1.5m" 중 의도한 체감이 무엇인지 |
| Catch 뒤 Flow | clean/applied Saved Flow에서 22를 선택해 `Start Here`로 시작하거나 `Reload Flow`의 01부터 실제 22까지 도달하고, player 생존/사망을 각각 본다 | 정상은 `[LIVE]` 23 `VALTAN_CHARGE_2`; `[LIVE]` 22 + `REQUEST_PENDING` + Live IDLE이면 Balance Tool `Server decision trace`의 `MECHANIC_RESET_REQUIRED`로 확정 |
| Trash 횟수 | counter하지 않고 NONE, PARTIAL, ALL을 각각 만든다 | 실제 rush 횟수, partial damage/release, all execute, 네 번째 시도 여부 |
| Trash counter | rush 준비 `STEP_07`에서 counter한다 | 즉시 GROGGY인지, groggy 뒤 다음 Product pattern으로 넘어가는지 |
| Finale | `VALTAN_GHOST_FINALE`를 시작하고 본체/유령을 두 cycle 이상 본다 | 본체 portal 여부, 실제 공격 ID 순서, ghost 교체 체감 여부, Stop/Next 반응 |

결과에는 SHA, party 인원, pattern ID, 발탄 yaw/플레이어 대략 위치, 발생 시점과 반복 횟수를
적는다. 영상·스크린샷은 선택적 보조 자료이며 사용자 서면 관찰이 최종 수동 판정이다.
관찰이 현재 조사와 다르면 구현을 시작하지 않고 해당 SHA의 코드·데이터를 다시 감사해
이 계획부터 교정한다. 조사와 일치하고 사용자가 구현을 다시 승인하면 G09-1부터 진행한다.

### G09-1. 잡기후날리기 빨간 sector·손 고정·제한 거리 정면 발사·후속 정지 복구

#### G09-1-0. 빨간 sector와 Server capture collider의 단일 방향 계약

현재 `Data/Valtan/Valtan.gameplay.json`의 Catch `STEP_02`는 `CONE`, `120도`, `8m`,
`CAPTURE/BOSS_LEFT_HAND`까지만 선언한다. `LOCK_RANDOM_ALIVE_BEHIND_ON_START`는 패턴 시작 전
발탄 뒤의 대상 후보를 고르는 정책이고, `LOCK_FACING_ON_START`는 그 대상을 향해 발탄을 다시
회전시키는 정책이다. 둘 중 어느 것도 hit cone의 방향 계약은 아니다. Server는 stage 진입 때
길이와 각도만 runtime state로 복사하고 판정 시점의 발탄 정면 벡터 `(+sin,+cos)`를 그대로
`Circle_IntersectsCone()`에 넘기므로, 현재 화면의 빨간 sector가 아닌 정면에도 capture request가
생긴다.

08-26의 과거 PLAN/RESULT는 당시 Effect 기준으로 `rear yellow cone / yaw 180 reversed`를
계약으로 적었다. 최신 사용자는 yellow가 아니라 현재 user-tuned Product의 red sector를
권위 범위로 명시했으므로 G09가 그 과거 색상 계약을 대체한다. 과거 문서를 소급 수정하거나
author helper Apply로 현재 수동 Effect element를 다시 생성하지 않는다.

수정은 `VALTAN_CATCH_BREATH` 문자열을 `ContainsPatternHit()`에서 비교하는 특례로 만들지 않는다.
방향성 shape `CONE/BOX/CROSS/SIX_DIRECTIONS`가 실제 소비하는 typed boss-local
`yawOffsetDegrees`를 stage hit shape 계약에 추가한다. 기존 stage는 명시적 0도로 유지하고,
Catch `STEP_02`만 G09-0 사용자 기준선에서 확정된 빨간 sector 중심값을 저작한다. publisher와
Python projection은 finite 범위와 shape별 허용 여부를 검사하고, Server catalog/runtime은
`boss.fYawDegrees + yawOffsetDegrees`에서 forward를 한 번 계산해 판정한다. 원형 shape와 일반
정면 cone의 동작은 바꾸지 않는다.

빨간 요소와 반대 요소가 시각 데이터상 약 180도 차이라는 사실만으로 Catch 값을 180도로
확정하지 않는다. Effect element의 raw Y rotation `122.25도`도 texture/quad 기준축 때문에
Server 값으로 사용하지 않는다. 기준선에서 빨간 중심이 판정 시점의 current boss yaw를
따르는지, 패턴 시작 직전 yaw를 유지하는지 먼저 기록한다. current boss 기준이면 위
boss-local offset 하나로 닫고, start-facing 기준임이 확인되면 패턴 시작 yaw snapshot을
명시적 shape basis로 추가해 Effect와 hit이 같은 snapshot을 소비한다. 이 확인 전에는 임의의
0/180도나 basis를 data에 쓰지 않는다.

target selection, aim과 collider는 함께 검증한다. offset 적용 뒤 선택된 rear target이 빨간
sector 밖으로 나간다면 target을 억지로 capture하거나 cone을 넓히지 않는다. 선택 시점 yaw,
aim 완료 yaw, hit basis를 같은 occurrence ledger에 고정하고, 선택된 target을 빨간 중심에 두는
typed aim 정책이 필요한지 기준선으로 결정한다. 어떤 경우에도 대상 선택 결과 자체를 collider
통과로 간주하지 않고, 모든 player가 실제 120도/8m boss-relative primitive를 따로 통과해야만
capture request를 만든다. Client Debug collider mirror도 같은 offset/basis를 읽어 빨간 sector와
겹쳐 보여야 하며 gameplay 판정 권위는 계속 Server에만 있다.

split gameplay source의 shape 필드는 `yawOffsetDegrees`, projected Encounter/runtime/bootstrap의
flat field는 `hitYawOffsetDegrees`로 고정한다. 이 새 Product field 때문에
`Data/Encounters/Valtan/ValtanEncounter.json`은 formatVersion 4에서 5로 올리고
`EncounterPatternReference`도 v5만 수용한다. split authoring formatVersion은 현행 1을
유지한다. G09-1의 release token과 같은 bootstrap v27 원자 단위에서 row shape, parser,
publisher와 생성물을 함께 바꾸며 v26 parser/v27 data 또는 Encounter v4/v5 consumer가 섞인
중간 commit을 만들지 않는다.

`Capture_PlayerAttachment`와 `Update_PlayerAttachment`는
`BOSS_LEFT_HAND` 포획의 authoritative gameplay-root 위치를 고정 grip anchor로
정규화한다. canonical 값은 local `X=0`, `Y=0`, `Z=0`, `Yaw=0`으로 고정한다. 포획
순간의 거리와 회전 차이를 attachment 수명에 저장하지 않고 Trash와 Catch가 같은
Server snapshot history를 만든다. wire field와 Shared v45의 기존 finite bounded admission은
mixed peer 호환을 위해 유지한다. `NetworkProtocolHarness`는 zero와 legacy finite nonzero
roundtrip을 모두 허용하고 NaN/invalid slot만 계속 거부한다. exact zero는 codec admission이
아니라 Server producer/domain invariant로 검증하므로 Network protocol version과 packet
shape를 올리지 않는다.

Client는 Server gameplay-root에 이 offset을 다시 합성하지 않으며, 현재 공통 경로에서
실제 `bip001-l-hand` world origin을 최종 player position으로 계속 사용한다. Catch 전용
보정이나 첫 피격 거리 분기를 추가하지 않는다. 서로 다른 먼 포획 위치, 180도 회전한 손,
normal 0.01/ghost 1.0 scale에서도 동일 손 원점이 되는 native harness를 유지·확장한다.

과거 값은 `OPPOSITE_KNOCKBACK`, `12m/s × 500ms`, 현재 값은 `ARENA_EJECTION`,
`24m/s × 500ms`다. 최신 요청의 "이전 값 기준 1.5"는 이전 속도의 1.5배인
`18m/s × 500ms`, 명목 9m를 잠정 후보로 둔다. 다만 이 표현이 고정 거리 `1.5m`를 뜻했다면
500ms 기준 3m/s가 되어야 하므로 사용자 기준선 결과에서 의미를 확인하기 전에는 data를
수정하지 않는다. 이 불명확성은 숫자만 임의 선택해 숨기지 않고 RESULT의 미확정 항목으로
유지한다.

`ARENA_EJECTION`은 속도와 무관하게 마지막 아레나 지면보다 밖의 endpoint를 계산하므로
18m/s로만 낮춰도 너무 멀리 날아가 죽는 현상은 남는다. 기존 token의 뜻을 조용히
재정의하지 않고 `BOSS_FORWARD_KNOCKBACK`을 release grammar에 추가한다. 이 mode는
`Prepare_BossForwardKnockback`이 발탄 현재 yaw의 `(+sin(yaw), +cos(yaw))`와 선택된
speed/duration으로 velocity를 직접 stage한 뒤 기존 `Advance_PlayerKnockback`의 유한
duration과 nav clamp를 소비한다. canonical zero에서 generic release의 player-yaw
`(-sin,-cos)` fallback을 타지 않는다. Catch 자체로 `isCombatReady=false`, 강제
FALLING 또는 arena-outside minimum을 만들지 않는다. yaw 0/90/180/270은 각각
`+Z/+X/-Z/-X`다. 일반 radial knockback과 기존 `ARENA_EJECTION` grammar/legacy 동작은
그대로 둔다.

#### G09-1-1. Catch 종료 뒤 발탄이 멈추는 원인과 소유권 수정

저장 Flow에서 `VALTAN_CATCH_BREATH`는 22번이고 다음은 23번
`VALTAN_CHARGE_2`다. 따라서 Play Selected/Preview Isolated의 단일 occurrence 종료와
사용자가 재현한 Saved Flow 정지를 구분한다. 현재 `Prepare_ArenaEjection`은 release
commit 즉시 마지막 player를 살아 있는 상태에서 `isCombatReady=false`로 만든다. 다음
fixed tick에는 engageable target도, grabbed target도, DEAD/FALLING revive 대상도 없어
terminal `STEP_04`가 `NO_VALID_TARGET`으로 abort된다. `FinishPattern`은 자동 Flow cursor를
올리지 않고 mechanic-reset-required를 남겨 IDLE로 가므로, 플레이어가 이후 떨어져 죽으면
22번에서 발탄이 멈춘 것처럼 보인다. 이 경로는 코드상 확인됐고, party 구성이나 실제
낙하 tick 순서에 따른 화면 occurrence의 세부 timing만 아직 수동 확인 대상이다.

수정은 pattern ID 특례가 아니라 기존 `iGrabExecutionCommittedPatternSequence/StageIndex`를
execute/release kind까지 표현하는 grab resolution commit ledger로 일반화한다. 전체
`RELEASE_GRABBED_PLAYERS` transaction이 성공하고, current owner/slot/patternSequence와
일치하는 attachment를 실제로 한 명 이상 release한 경우에만 exact patternSequence/stage에
terminal release committed marker를 기록한다. 대상 0명의 no-op success, preflight/commit
실패에는 marker를 쓰지 않고 기존 rollback/abort를 유지한다.
`CanContinueTargetlessOwnedGrabPattern`은 그 marker의 terminal stage를 끝까지 소유한다.
marker는 `FinishPattern`, reset, death, room-empty에서 정확히 한 번 지운다.

성공 계약은 다음과 같다.

1. Catch `STEP_04`는 마지막 player가 순간적으로 engageable하지 않아도 2초 terminal을
   `COMPLETED`로 끝낸다.
2. Saved Flow cursor는 22에서 23으로 정확히 한 번 증가한다.
3. bounded forward knockback 뒤 player가 살아 있으면 1초 pursuit 후
   `VALTAN_CHARGE_2`를 시작한다.
4. 다음 step 시작 전에 다른 원인으로 player가 DEAD/FALLING이면 cursor 23의
   `REQUEST_PENDING`에서 valid target을 기다리고, typed revive 뒤 23을 시작한다. 현재
   `PAUSED_FOR_REVIVE`는 이미 running인 automatic step에서만 쓰므로 새 pause state를
   만들지 않는다. 자동 부활도 추가하지 않는다.
5. Play Selected, Preview Isolated, Stop After Current의 의도된 hold는 그대로 유지한다.

### G09-2. 버러지 총 세 번 시도

`VALTAN_TRASH`는 도입 `STEP_01..06`을 한 번만 실행하고, 현재 stable 첫 시도
`STEP_07/STEP_08`부터의 counterable windup → rush → capture result를 총 세 번만
시도한다. runtime loop counter나 stage back-edge를 추가하지 않고 두 번째·세 번째
시도의 고유 stage/action/event ID를 authoring helper가 유한 DAG로 전개한다.

stable ID는 병렬 구현자가 임의로 달리 만들지 않도록 다음으로 고정한다. 첫 시도는 기존
`STEP_07`, `STEP_08`, `CATCH_COUNTER`, `CATCH_PRE_IMPACT`, `CATCH_SLAM`, `RUSH_MISS`와
`valtan.sequence.center-trash-rush-if.*` action/event ID를 그대로 보존한다.

| 회차 | stage ID | action ID suffix |
|---|---|---|
| 2 | `ATTEMPT_02_WINDUP` | `attempt-02.windup` |
| 2 | `ATTEMPT_02_RUSH` | `attempt-02.rush` |
| 2 | `ATTEMPT_02_CATCH_COUNTER` | `attempt-02.catch-counter` |
| 2 | `ATTEMPT_02_CATCH_PRE_IMPACT` | `attempt-02.catch-pre-impact` |
| 2 | `ATTEMPT_02_CATCH_SLAM` | `attempt-02.catch-slam` |
| 2 | `ATTEMPT_02_RUSH_MISS` | `attempt-02.rush-miss` |
| 3 | `ATTEMPT_03_WINDUP` | `attempt-03.windup` |
| 3 | `ATTEMPT_03_RUSH` | `attempt-03.rush` |
| 3 | `ATTEMPT_03_CATCH_COUNTER` | `attempt-03.catch-counter` |
| 3 | `ATTEMPT_03_CATCH_PRE_IMPACT` | `attempt-03.catch-pre-impact` |
| 3 | `ATTEMPT_03_CATCH_SLAM` | `attempt-03.catch-slam` |
| 3 | `ATTEMPT_03_RUSH_MISS` | `attempt-03.rush-miss` |

action prefix는 `valtan.sequence.center-trash-rush-if.`다. 회차별 reaim,
counter-window enter/exit, captured-slam event는 각각
`event.valtan.trash.attempt-02.*`, `event.valtan.trash.attempt-03.*` prefix를 쓴다.
공통 terminal ID `EXECUTE_TAIL`/`GROGGY`와 그 기존 action/event ID는 한 벌만 둔다.
따라서 main은 도입 6 + attempt 18 + 공통 terminal 2 = 정확히 26 stages,
`_CATCH_IF`는 attempt 18 + terminal 2 = 20 stages다.

- 어느 회차든 `COUNTER_HIT`은 공통 `GROGGY`를 거쳐 즉시 pattern을 끝내고 다음
  Product pattern으로 전환한다.
- 어느 회차든 전원 포획은 `EXECUTE_TAIL`로 끝난다.
- 1·2회차의 miss 또는 partial `CATCH_SLAM`은 attachment damage/release transaction을
  commit한 뒤 다음 시도로 간다. 3회차 결과는 terminal이다.
- `VALTAN_TRASH_CATCH_IF`는 같은 세 시도 fragment를 사용한다.
  `_CATCH_SUCCESS`는 기존 4-stage success tail,
  `_CATCH_FAIL`은 기존 1-stage `RUSH_MISS` 단발 wrapper로 유지한다. wrapper로 복사한
  `CATCH_SLAM`/`RUSH_MISS`의 default와 TIMEOUT은 반드시 null로 다시 써 외부 attempt
  action을 참조하지 않는다.

`author_valtan_phase_two_mechanics.py`와 animation promotion closure가 이 stage 집합을
항상 명시 재구성한다. split gameplay/presentation을 Apply한 뒤 PublishV2로 Encounter,
pattern binding/effect cue를 transactionally 투영하고 sound/shake/root-motion builder를
다시 실행한다. Server/Client의 기존 finite graph validator는 그대로 유지하며 cycle,
unknown/missing/duplicate action과 중간 impact/release 실패 rollback을 계속 거부한다.
대상이 모두 죽거나 combat-ready가 아니면 기존 pause 계약을 사용해 남은 attempt를
시간만으로 소비하지 않는다.

### G09-3. 본체와 유령의 동일 여섯 공격 death loop

`VALTAN_GHOST_FINALE`는 더 이상 `VALTAN_WARP`의 8회 portal charge를 반복하지 않는다.
짧은 spawn/activation occurrence만 소유하고 다음 exact list를 본체와 유령이 같은 순서로
죽을 때까지 반복한다.

1. `VALTAN_WHIRLWIND`
2. `VALTAN_FOUR_SLASH`
3. `VALTAN_SEQUENCE_FOUR`
4. `VALTAN_HIGH_JUMP`
5. `VALTAN_CATCH_BREATH`
6. `VALTAN_ATTACK_WHIRLWIND`

개별 공격 graph는 계속 유한이다. `SERVER_WORLD_ENTITY`에 outer Product/Debug Flow의
`strRotationId/iRotationStepIndex/bAutomaticPatternSequenceStepRunning`과 분리된
`FinaleLoopState`를 둔다. 본체와 종속 유령이 같은 구조와 selector를 소비하며 각자의
attack cursor와 clock을 갖는다.

| 상태/필드 | 계약 |
|---|---|
| phase | `INACTIVE → SPAWN_PENDING → PREPARING → ATTACK_PENDING ↔ ATTACK_RUNNING` |
| source identity | `VALTAN_GHOST_FINALE` ID, 최초 source patternSequence, encounter와 owner NetEntityId |
| generation | active gameplay의 pinned revision과 wrap되지 않는 activation identity |
| attack definition | exact six `GhostPatternIds`, expected count 6, 0..5 cursor |
| ownership | main만 outer Product/Flow step 소유, dependent ghost는 outer step 없음 |
| spawn ledger | spawn-consumed와 committed child NetEntityId; 전체 spawn commit 전에는 둘 다 비어 있음 |
| control | attack-step-running, stop-after-current, reset-required |

source finale를 시작할 때 main은 `SPAWN_PENDING`과 outer-step ownership을 잡는다.
nav/profile/payload/broadcast preflight를 통과해 child가 commit되면 두 entity에 같은
activation/revision과 exact six를 넣고 spawn-consumed를 확정한다. source `STEP_01`이
끝나면 ordinary `PatternTerminalReceipt::COMPLETED`를 정상 기록하되, active composite
runtime이 이를 `ATTACK_PENDING` 내부 전환으로 소비한다. 이 tick에는 첫 공격을 시작하지
않으며 ordinary Product selector, weighted fallback, Flow terminal hold도 실행하지 않는다.
다음 tick `Begin_NextFinaleAttack`이 저장된 pin/cursor로 기존 `BeginPattern`을 호출한다.

각 내부 공격의 `FinishPattern`은 실제 patternSequence의 terminal receipt를 남긴 뒤 outer
cursor가 아니라 finale cursor만 증가시킨다. `ATTACK_PENDING`에서 다음 tick 새 실제
patternSequence를 발급한다. 여섯 번째 다음은 0으로 감고, 일반 chase/weighted pattern이
사이에 끼지 않는다. 내부 공격 실패·정의 pin 소실·child reset-required는 다음 ID로
건너뛰거나 ghost를 재생성하지 않고 encounter reset-required로 닫는다. target 부재 pause도
현재 pattern의 finale metadata가 아니라 active `FinaleLoopState`를 보고 cursor와 stage
clock을 보존한다. 본체의 outer cursor를 되감는 `Restart_FinaleCycle` 특례는 제거한다.

Pattern-ID audition은 내부 공격 시작마다 expected pattern ID와 expected patternSequence를
실제 공격으로 재결합하되 UI의 composite source는 `VALTAN_GHOST_FINALE`로 유지한다.
Saved Flow lifecycle과 `Adopt_ValtanLiveNextPattern`도 current slot finale + world snapshot
actual six-attack 조합을 정상 상태로 인정한다. Next/Stop이 예약되면 현재 내부 공격 하나만
완료하고 cursor wrap과 다음 `BeginPattern`을 금지한다. 그 seam에서 main의 outer step을
정확히 한 번 완료하고 마지막 actual patternSequence를 predecessor로 사용한다. Next는
`predecessor + 1` successor를 한 번만 시작하고 Stop은 idle terminal hold로 간다.

종속 유령 생존 조건은 더 이상 owner의 현재 `strPatternId == VALTAN_GHOST_FINALE`가
아니다. 살아 있는 owner, 같은 encounter/revision/activation identity와 active latch로
검증한다. child는 같은 NetEntityId로 최소 두 wrap 이상 유지되고 spawn-consumed latch로
같은 finale occurrence에서 재생성하지 않는다. 본체 death/reset/room-empty 또는 명시적인
Debug Next/Stop은 loop state, 종속 유령, 양쪽 포획과 combat object를 같은 bounded commit
seam에서 함께 정리한다. ghost는 계속 dependent presentation이며 별도 HUD/BGM/damage
target으로 승격하지 않는다. late join은 main-before-child 순서와 같은 pin/activation을
검증하고 진행 중 hot reload는 이미 시작한 여섯 정의를 바꾸지 않는다.

finale bootstrap row는 3개 attack에서 6개 attack으로 늘고 kind도 portal 의미가 없는
이름으로 바뀌므로 `PATTERNFINALE`는 정확히 14 fields가 된다. G09-1의 directional hit와
새 release token이
v27에서 먼저 admission되므로 finale commit은 gameplay bootstrap을 v27에서 v28로 올린다.
Encounter profile은 G09-1의 directional hit 계약에서 이미 v5가 되며 finale commit은 이를 더
올리지 않는다. split authoring JSON formatVersion과 Shared Network protocol version도
finale 때문에 추가로 올리지 않는다.
PowerShell publisher, Python projector, Server parser, Client strict view가 exact 6개 순서와
non-recursive finite definition을 같은 규칙으로 검증한다. 최종 commit은 v28만 받고
v26/v27/v29, 13/15 fields, 구 kind, 3/5/7개 목록, 순서 변경·중복·unknown/self/terrain/cycle,
finale portal motion, owner generation 불일치와 부분 spawn activation은 commit 전에
거부한다. `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`은 생성물이므로 publisher로
v28을 다시 만든 뒤 Server를 재시작하며 기존 v26/v27 파일을 조용히 수용하지 않는다.

### G09-4. 구현 파일과 소비자 폐쇄

새 파일은 만들지 않는 것을 기본으로 한다. 실제 소비자가 없어 새 파일이 필요해지면
구현 전에 이 문서에 소유자와 `.vcxproj/.filters` 등록·검증을 추가한다.

| 계약 | 정본/구현 파일 | 필수 소비자·하네스 |
|---|---|---|
| 손 canonical zero | `Server/Private/GameRoom.cpp`; Shared v45 codec은 현행 유지 | Server exact-zero producer test, v45 zero/finite-nonzero compatibility, `EffectRenderContractHarness.cpp` |
| 빨간 sector collider/v27 | split gameplay/Encounter, `author_valtan_phase_two_mechanics.py`, `valtan_tuning_pipeline.py`, `Publish-GameplayBalance.ps1`, `GameplayCatalog.h/.cpp`, `ServerWorldEntity.h`, `ValtanBrain.cpp`, `ValtanPatternTree.cpp`, `EncounterPatternReference.cpp` | 빨간 중심 offset/basis projection, front miss/red capture fixed-tick matrix, Debug mirror join, invalid/missing/nonfinite/out-of-range rejection |
| 제한 거리 정면 release/v27 | `author_valtan_phase_two_mechanics.py`, split gameplay, `GameplayCatalog.h/.cpp`, `GameRoom.cpp`, PowerShell/Python parser | typed boss-yaw helper, 선택된 speed/duration, nav clamp, token/version rejection |
| Catch terminal 소유권 | `ServerWorldEntity.h`, `ValtanBrain.h/.cpp`, `GameRoom.h/.cpp` | 실제 capture→release→STEP_04 완료→Flow 22→23, zero-target/failure rollback, slot23 request-pending |
| Trash 3회 DAG | `author_valtan_phase_two_mechanics.py`, `promote_valtan_animation_chains.py`, split gameplay/presentation | pattern master/promotion/tree Python tests, Server fixed-tick contract, `ClientPartyRegression.cpp` |
| finale authoring/v28 | split gameplay/presentation, `valtan_tuning_pipeline.py`, `Publish-GameplayBalance.ps1` | `GameplayCatalog.h/.cpp`, `ValtanPatternTree.h/.cpp`, `EncounterPatternReference.cpp` |
| finale runtime | `ServerWorldEntity.h`, `ValtanBrain.h/.cpp`, `GameRoom.h/.cpp` | Server contract, `ValtanPatternAuditionServiceHarness`, Boss Tool Flow Python tests |
| dependent presentation | 기존 Shared owner wire, `CValtan`/`CBody_Valtan`/replication 경로 | model-view clip closure, `ClientPartyRegression`, late-join/primary-before-child |
| 문서 | 이 PLAN, 대응 RESULT, `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`, `발탄인수인계서.md` | v27 release·v28 finale·6공격·26/20-stage 설명과 실제 코드/생성물 대조 |

생성 Product closure는 다음이다.

- `Data/Valtan/Valtan.gameplay.json`
- `Data/Valtan/Valtan.presentation.json`
- `Data/Encounters/Valtan/ValtanEncounter.json`
- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`
- `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json`
- `Data/Animation/Authored/Valtan/Valtan.patternshakecues.json`
- `Data/Animation/RootMotion/Valtan.rootmotion.json`
- Server pre-build가 다시 만드는 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`

shake, rotations, combat objects, world events, provenance가 byte-identical이어도 builder와
publisher가 join을 검증한다. 생성물이 바뀌지 않았다는 이유로 검증을 생략하지 않는다.
`ClientReplication`의 공통 손 본 경로와 Shared spawn wire는 현재 계약을 재사용하므로,
실측 없이 Catch 전용 분기나 protocol version bump를 추가하지 않는다.

### G09-5. 구현 순서와 commit barrier

1. 사용자 기준선 결과와 구현 재개 승인을 받는다.
2. 다른 세션 PR이 병합된 최신 base를 fetch하고, clean한 새 bug-fix branch를 만든다.
3. 의도한 실패 fixture를 먼저 추가해 현재 구현에서 RED를 확인한다.
4. 빨간 sector collider, 손 canonical zero, 제한 거리 정면 release, Catch terminal 소유권을
   v27 원자 단위로
   구현·검증·commit한다. "1.5" 의미가 확인되지 않았으면 이 단계에 들어가지 않는다.
5. Trash author helper만 3회 DAG로 바꾸고 Apply/Publish/검증 후 생성 closure와 함께
   commit한다. 이 barrier 전에는 finale authoring을 바꾸지 않는다.
6. finale authoring/schema/runtime/client strict view를 v28 원자 단위로 바꾸고 전체
   생성 closure·harness·문서를 함께 commit한다. v28 parser만 또는 데이터만 따로
   commit하지 않는다.
7. 최신 통합 HEAD에서 focused GREEN과 전체 Debug/Release regression을 다시 실행하고
   RESULT에 실제 명령, exit code, SHA, 미실행 수동 검증을 분리해 기록한다.

권장 commit 단위는 다음 세 개다.

1. `fix(valtan): align catch sector, bound release, and resume flow`
2. `fix(valtan): bound trash rush to three attempts`
3. `fix(valtan): repeat shared six-attack ghost finale`

각 commit은 대응 테스트와 필요한 데이터/문서를 포함해 독립적으로 검증 가능해야 한다.
Trash와 finale가 같은 authoring/generated 파일을 수정하므로 2번을 완전히 생성·검증·commit한
뒤 3번을 시작한다. 오래된 `origin/codex/valtan-authoring-data-contract`를 wholesale
merge하지 않고 필요한 최신 base 변경만 실제 diff로 대조한다.

첫 commit은 directional hit yaw/basis와 release token admission, gameplay bootstrap v27을
함께 포함한다. 두 번째는
v27을 유지하고, 세 번째만 v28로 올린다. 따라서 각 commit을 checkout했을 때 parser와
bootstrap이 같은 exact version이어야 하며 v27 parser/v28 data 같은 중간 상태를 만들지 않는다.
첫째·둘째 commit에서는 v27 accept와 v26/v28 reject를 실행하고, 셋째 commit에서는 v28
accept와 v26/v27/v29 reject를 별도로 기록한다.

### G09-6. authoring·publisher 실행 순서

각 데이터 commit에서 해당 단계에 맞는 author helper를 수정한 뒤 다음 순서로 실행한다.

```powershell
python -B Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py --mode Apply
python -B Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py --mode Validate

powershell -NoProfile -ExecutionPolicy Bypass `
  -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2

python -B Tools/ValtanPipeline/build_valtan_pattern_sound_cues.py
python -B Tools/ValtanPipeline/build_valtan_pattern_shake_cues.py

python -B Tools/ValtanActionExtractor/build_valtan_rootmotion.py `
  --resource-root C:\Users\user\Desktop\LostArk\Client\Bin\Resources
python -B Tools/ValtanActionExtractor/build_valtan_rootmotion.py `
  --resource-root C:\Users\user\Desktop\LostArk\Client\Bin\Resources --check

powershell -NoProfile -ExecutionPolicy Bypass `
  -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
powershell -NoProfile -ExecutionPolicy Bypass `
  -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass `
  -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
```

author helper는 기존 finale가 이미 있어도 `finale`, `stages`, presentation stages를 매번
완전히 재구성해야 한다. 없을 때만 생성하거나 WARP motion 일부만 patch하면 과거 portal
row가 남으므로 idempotent가 아니다. Publish 뒤 split 정본과 Product projection을 다시
비교하고 제거한 finale `STEP_02..10`의 binding/effect/sound/shake/root-motion 참조가 0개인지
검사한다. Catch release commit은 v27, 최종 finale commit은 v28 bootstrap을 publish한 뒤
각각 Server를 재시작해야 하며 재시작 전을 적용 완료로
기록하지 않는다.

### G09-7. 자동 검증 matrix

Trash contract는 다음을 모두 fixed tick terminal까지 실행한다.

- 1회 miss/partial → 2회, 2회 miss/partial → 3회, 3회 miss/partial → terminal
- 각 회차 full capture → 공통 `EXECUTE_TAIL`, 각 회차 counter → 공통 `GROGGY`
- 네 번째 windup/rush가 존재하지 않음
- nav blocked tick의 최종 capture가 miss 분기보다 먼저 commit
- damage/release preflight 실패 시 다음 회차로 이동하지 않고 attachment도 부분 해제하지 않음
- `_CATCH_IF` 20-stage closure, `_SUCCESS` 4-stage와 `_FAIL` 1-stage의 외부 action 참조 0개
- combat-ready 대상 부재 pause가 남은 회차를 소비하지 않음

Catch sector contract는 yaw 0/90에서 사용자 기준선으로 확정한 빨간 중심 안의 player만
capture하고, 같은 거리의 현재 정면/반대 sector player는 capture하지 않는지 검사한다. 빨간
중심과 정면에 두 player가 동시에 있어도 capture request는 빨간 쪽 한 명뿐이어야 한다.
각도 경계 `+/-60도`의 body-circle 안/밖, 8m 길이 안/밖, boss rotation 뒤 같은 boss-local
결과를 fixed tick으로 검증한다. target selection이 rear player를 골랐다는 사실만으로
primitive 밖 대상을 통과시키지 않는다. missing/nonfinite/out-of-range offset 또는 허용되지
않은 shape/basis 조합은 authoring projection, Encounter v5와 bootstrap stage를 commit 전에
거부하며, Encounter v4/v6도 exact-version admission에서 거부한다. 현재 front cone을 쓰는
다른 패턴은 offset 0 회귀를 통과한다.

손/release contract는 서로 다른 Catch/Trash 포획 위치에서 Server가 같은 zero snapshot을
만들고, Shared v45가 zero와 기존 finite bounded nonzero를 모두 roundtrip하며 NaN/invalid
slot을 거부하는지 검사한다. typed helper의 yaw 0/90/180/270은 각각 +Z/+X/-Z/-X이며 generic
release의 player-yaw fallback을 호출하지 않는다. 사용자가 1.5배 해석을 확정한 경우에만
18m/s × 500ms의 명목 9m를 exact 값으로 검사한다. Catch release는 arena-outside minimum,
`isCombatReady=false`, 강제 FALLING을 만들지 않는다. invalid speed/nav preflight는 GRABBED
상태를 그대로 보존하고 일반 knockback nav clamp, 기존 ARENA_EJECTION grammar,
FALLING/death/revive/reset/disconnect cleanup은 회귀하지 않는다.

실제 한 명의 player를 `VALTAN_CATCH_BREATH`에 capture한 fixed-tick test는 release transaction,
terminal `STEP_04`, receipt `COMPLETED`, automatic Flow cursor 22→23을 순서대로 확인한다.
player가 살아 있으면 1초 pursuit 뒤 `VALTAN_CHARGE_2`가 시작되어야 한다. sole player가
release seam에서 잠시 non-engageable이어도 abort/reset-required가 되지 않아야 하며, 별도
낙하·사망을 주입하면 cursor 23의 `REQUEST_PENDING`에서 typed revive 뒤 23을 시작해야 한다.
release 대상 0명의 no-op success와 preflight/commit 실패는 committed marker를 쓰지 않고,
실패 시 attachment를 보존한다. 두 번째 engageable player가 있는 경우, Play Selected의
`COMPLETED_HOLD`, Stop After Current의 hold도 각각 회귀하지 않는다.

finale contract는 다음을 검사한다.

- source finale `STEP_01` 정확히 한 번 뒤 main 첫 12 occurrence가 exact six 두 wrap
- main에서 `PORTAL_CROSS_ARENA`와 `bPortalMotionActive`가 한 번도 활성화되지 않음
- 같은 ghost NetEntityId가 exact six를 두 wrap 이상 실행하고 main/ghost 독립 cursor가
  각자의 순서를 보존함
- source 완료 tick에는 첫 내부 공격을 시작하지 않고 정확히 다음 tick 시작함
- outer finale receipt/slot ownership은 한 번이며 normal selector가 사이에 끼지 않음
- `[VALTAN_GHOST_FINALE, VALTAN_FIST_IN_OUT]` Flow의 세 번째 loop에서 Pattern-ID Next,
  Flow Live Next, Flow Stop이 successor 1회 또는 idle hold를 정확히 만듦
- no-spawn-clearance가 entity ID, broadcast, latch, child, cursor를 하나도 소비하지 않음
- owner ID 재사용·stale revision/activation·internal failure가 silent skip/respawn을 만들지 않음
- main과 ghost가 동시에 Catch할 때 이미 잡힌 player owner를 빼앗지 않는 기존 결정 규칙
- death/reset/room-empty/Next/Stop이 child, 양쪽 attachment/combat object를 함께 정리함
- late join이 main-before-child와 같은 activation/revision을 받음
- 최종 v28만 수용하고 v26/v27/v29, 잘못된 row shape/kind/list/child/portal 재삽입을 거부함

focused 명령은 `Test-ValtanPatternMaster.ps1`, 관련 Python suite,
`NetworkProtocolHarness`, `Server.exe --contract-test`,
`ActionPresentationTimelineHarness`, `EffectRenderContractHarness`,
`ValtanPatternAuditionServiceHarness`, `ClientPartyRegression`이다. Python suite에는 최소
`test_valtan_pattern_master_v2.py`, `test_valtan_animation_chain_promotion.py`,
`test_valtan_pattern_tree_contract.py`, `test_valtan_boss_tool_pattern_flow_contract.py`,
`test_effect_tool_valtan_saved_rows.py`를 포함한다.

focused GREEN 뒤 정본 `Tools/Build/Invoke-BuildAndRegression.ps1`을 Debug와 Release로
각각 실행한다. Engine → UpdateLib → Shared/Network → Server contract → Client 순서를
지키고 JSON/XML parse와 `git diff --check`를 마지막에 실행한다. 실패 fixture RED,
구현 후 focused GREEN, 전체 regression의 실제 결과만 RESULT에 기록한다. 에이전트가
Client/UI를 실행하거나 visual PASS를 대신 기록하지 않는다.

### G09-8. PR·merge·사후 수동 검증

현재 snapshot의 HEAD와 `origin/main`은 `a20d625f`, ahead/behind 0/0이다. 다만 다른 사람의
선행 PR merge가 계속되므로 구현 직전에 `git fetch`로 최신 포함 SHA를 다시 확인하고,
사용자가 지정한 최종 base에서 fresh bug-fix branch를 만든다. 선행 commit이 아직 main에
없다면 현재 통합 브랜치를 base로 하는 내부 PR인지 최종 main PR인지 명시적으로 구분하며
두 diff를 같은 bug-fix PR로 위장하지 않는다.

PR 전에는 다음을 확인한다.

- `git status --short`, upstream, `origin/main...HEAD` ahead/behind
- `git diff --name-status <base>...HEAD`에 위 세 검증 단위와 문서만 존재함
- 다른 세션의 미커밋/staged 파일과 build/intermediate 산출물이 0개임
- 최신 통합 HEAD에서 focused/full regression이 다시 통과함
- 대응 RESULT가 구현 완료, 자동 검증, 사용자 미검증을 분리함

bug-fix branch를 push하고 PR checks를 확인한 뒤에만 merge한다. merge 뒤 fetch하여 merge
SHA가 `origin/main` 또는 사용자가 정한 최종 통합 branch에 실제 포함됐는지 확인하고,
그 merged SHA에서 Server bootstrap v28을 publish한 뒤 Server + Client profile을 사용자에게
  안내한다. 사용자는 Catch 빨간 sector 안에서만 포획되는지, 손 위치·정면 발사·체감 거리,
  Catch 뒤 Saved Flow의
`VALTAN_CHARGE_2` 재개, Trash 정확히 세 시도와 각 회차 counter, 본체·같은 유령의 여섯
공격 wrap을 직접 판정한다.

사용자 관찰 전에는 visual PASS나 전체 완료로 기록하지 않는다. 사후 관찰이 실패하면
실패 occurrence와 merged SHA를 RESULT에 남기고, 기존 merge를 완료 상태로 유지하지 않은
채 별도 follow-up bug-fix PR로 닫는다. 다른 사람의 변경과 함께 병합한 최종 HEAD에서도
focused harness와 Debug/Release regression을 한 번 더 실행해야 이 수직 슬라이스를 닫는다.
