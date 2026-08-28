# 발탄 잡기·버러지·유령·포탈·중앙 이동·독립 도넛·사망 구현 계획

## G00. 현재 작업 기준과 구현 순서

기준은 `C:/Users/user/Desktop/LostArk`의 현재 working tree다. 조사 시작 시
HEAD와 `origin/main`은 `1a9bce42ac00f75b2e1b6fa24ed0de1071d4c050`이고 브랜치는
`codex/valtan-flow-reload-next-fix`다. 사용자가 현재 작업 폴더에서 즉시 반영하도록
지정했으므로 다른 worktree로 옮기지 않는다. 기존 Flow/Next/Reload, 피자·고공 점프,
Effect·맵 튜닝 변경은 보존한다. 초기 67개 변경 파일은 Git 제외
`.codex_tmp/valtan-pattern-completion-20260828/baseline`에 별도로 보관했다.

이 문서는 구현 계획이다. 실제 구현·자동 검증·사용자 화면 검증은 대응 RESULT에서
구분한다. 기존 계획의 변경점은 아래 G별로 명시하며 과거 RESULT의 완료 문구를
현재 코드의 증거로 사용하지 않는다.

| G | 작업 단위 | 실제 소비 경로 |
|---|---|---|
| G01 | 손바닥 고정과 2배 발사 | Server attachment/release → Shared player state → Client attachment |
| G02 | 버러지 네 ID와 유한 분기 | split gameplay → publisher → GameRoom/ValtanBrain |
| G03 | 포탈 사각 모서리 이동과 복귀 | stage motion → Server transform → 기존 boss snapshot |
| G04 | 피자·3시·9시 중앙 이동 뒤 점프 | serverMotion → leap → world event 및 Effect anchor |
| G05 | 유령 공격과 본체 포탈 최종 루프 | dependent boss spawn → 기존 Brain → CValtan → despawn |
| G06 | 도넛의 독립 수명 | stage ENTER → CombatObject → 독립 timed hit/Effect |
| G07 | HP 0 사망 표현 뒤 제거 | Server death despawn → Client one-shot death clip → Layer 제거 |
| G08 | 실패 경로·회귀·인계 | publisher + protocol/Server/Client native harness |

세 비평 에이전트가 잡기/버러지/포탈, 도넛/앵커, 유령/복제 경로를 독립 조사했다.
그 결과를 실제 함수와 JSON으로 대조한 뒤 아래 설계에 반영했다. 수정 후에도 같은
경계를 다시 비평하고, 지적은 재현 가능한 테스트로 바꾼다.

## G01. 고정 손바닥 부착과 아레나 밖 발사

### 현재 기준점

`Client/Private/ClientReplication.cpp::Update_PlayerAttachmentPresentations`는
`PlayerWorld * inverse(HandWorld)`의 translation을 보존한다. 그래서 충돌 위치에 따라
손에서 떨어진 간격도 유지된다. `PlayerController`에는 이미 GRABBED 입력 차단과
release 뒤 새 press까지의 latch가 있으므로 같은 입력 경로를 추가하지 않는다.

`VALTAN_CATCH_BREATH/STEP_04`의 실제 발사 입력은 `12m/s × 500ms = 6m`다.
과거 문서의 45m/s 값은 현재 입력이 아니다. 현재 knockback은 nav/collision에서 멈춘다.

### 변경 파일과 상태

- `Client/Public/ClientReplication.h`의 `CPlayerHandGripTransform::Build_LocalOffset`,
  `Compose_World`와
  `Client/Private/ClientReplication.cpp`: 기존 손 회전·플레이어 scale 처리는 보존하고
  hand-local translation만 고정 grip origin으로 바꾼다. 실제 본체 손 basis scale 약 0.01은
  determinant 약 1e-6이므로 절대 크기가 아니라 정규화한 basis의 비퇴화를 검사한다.
  finite/determinant 실패는 해당
  부착 표현만 격리하고 Server 상태를 만들어내지 않는다.
- `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp`: 기존 release
  mode에 `ARENA_EJECTION`을 추가한다. 일반 피격 knockback과 강제 발사를 구분한다.
- `Server/Public/ServerPlayer.h`, `Server/Private/GameRoom.cpp`: 발사 중에만 적용되는
  방향·속도·남은 거리의 수명을 소유한다. nav/collision clamp를 우회하되 유한 이동과
  종료 정리를 보장한다. 아레나 밖에서는 기존 FALLING/사망 경로를 소비한다.
- `Data/Valtan/Valtan.gameplay.json`과 publisher: 속도는 정확히 2배인 24m/s,
  기본 거리는 12m로 설정한다. 그 거리로 외곽을 넘지 못하는 시작점은 실제 아레나
  경계를 벗어나는 데 필요한 거리만 유한 상한 안에서 늘린다.

호출 흐름은 기존 `Commit_BossPatternPlayerStageActions → Release_PlayerAttachment →
Advance_PlayerKnockback`을 확장한다. 두 번째 player 이동 런타임을 만들지 않는다.
발사 방향은 포획 순간의 우연한 radial offset이 아니라 발탄의 고정된 반대 방향으로
결정한다. death/revive/reset/disconnect에서는 attachment와 강제 이동을 함께 정리한다.

종료 증거는 서로 다른 포획 위치의 동일 hand-local translation, 24m/s 이동량,
nav 경계 통과, FALLING 전이, 일반 knockback clamp 보존이다. 실제 손바닥·캐릭터
몸통의 미세 높이 정렬은 사용자 화면 확인 대상이다.

## G02. 버러지 네 패턴의 공통 분기와 종료

### 현재 기준점

`VALTAN_TRASH`는 `STEP_07 → STEP_08 → RUSH_MISS → STEP_07` 및
`CATCH_SLAM → STEP_07` 순환을 가진다. 기존 테스트도 재돌진에 도달하면 검사를
끝내므로 전체 종료를 증명하지 않는다. counter는 현재 포획 뒤 200ms에만 열린다.
`VALTAN_TRASH_CATCH_SUCCESS`, `_FAIL`, `_IF`는 현재 순수 animation sequence다.

### 변경 흐름

공통 저작 fragment는 시전·돌진, 성공 tail, 실패 tail이다. main과 세 보조 ID는 같은
fragment를 소비한다. main에서 다른 patternSequence로 attachment를 넘기지 않는다.

| ID | 역할 |
|---|---|
| `VALTAN_TRASH` | 기존 도입 → 시전 counter → 돌진 중 포획 → 결과 tail → 종료 |
| `VALTAN_TRASH_CATCH_IF` | 같은 시전·돌진·결과 분기의 단독 진입 |
| `VALTAN_TRASH_CATCH_SUCCESS` | 같은 포획 성공 tail의 단독 확인; 포획을 임의 생성하지 않음 |
| `VALTAN_TRASH_CATCH_FAIL` | 같은 miss/recovery tail의 단독 확인 |

`STEP_07` WINDUP에 counter ENTER/EXIT와 `COUNTER_HIT → GROGGY`를 연결한다.
groggy가 끝나면 이 pattern occurrence를 완료하여 기존 scheduler가 다음 패턴을 고른다.
돌진 중에는 hit pulse마다 아직 잡히지 않은 생존 플레이어를 검사한다. 포획해도 돌진을
끊지 않고 종료 시점까지 계속 잡는다. `RUSH_MISS`와 부분 포획의 `CATCH_SLAM`은
재시전으로 되돌리지 않고 release 후 종료한다. 기존 전원 포획 처형 분기는 보존한다.

정면 이동이 nav 경계에서 잘리면 `NAVIGATION_BLOCKED` 결과로 같은 tick의 최종 포획
판정 후 tail로 전이한다. collider 뒤의 플레이어를 이동 전 가상의 긴 sweep로 잡지 않는다.
`CValtanBrain`의 기존 stage transition preflight/commit과 captured ANY/ALL 분류를
재사용한다. 카운터·시간 만료·nav 종료에 별도의 selector를 만들지 않는다.

수정 대상은 `author_valtan_phase_two_mechanics.py`, split gameplay/presentation,
`ValtanBrain.h/.cpp`, `GameRoom.h/.cpp`, 관련 catalog/publisher와 기존 Server contract다.
NONE/PARTIAL/ALL/COUNTER 네 경우를 terminal까지 실제 fixed tick으로 검증한다.

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

## G05. 유령 발탄과 본체 포탈 최종 패턴

### 기존 객체 경로의 확장

유령은 독립 Server `BOSS` entity다. stable archetype은 `BOSS_VALTAN_GHOST`,
소유자는 `iOwnerBossNetEntityId`로 식별한다. 별도 boss stage evaluator나 local clone
런타임을 만들지 않는다. 기존 spawn/snapshot/despawn, `CValtanBrain`, `CValtan`,
`CBody_Valtan`, `CModel → CMaterial`을 재사용한다.

유령은 하나씩 생성한다. 아레나 중앙 주변 X/Z 후보를 Server가 결정적으로 샘플링하고
현재 파괴 상태·nav·같은 높이를 검사한다. 생성된 유령은
`VALTAN_WHIRLWIND → VALTAN_FOUR_SLASH → VALTAN_SEQUENCE_FOUR`를 순서대로
실행한 뒤 소멸한다. 실패한 후보를 중앙에 조용히 대체하지 않는다. 반복 후보 수는 유한하다.
본체의 `VALTAN_GHOST_FINALE`는 이 유령 수명과 G03 포탈 순서를 병렬로 실행한다.
본체가 살아 있는 동안 유한 포탈 cycle을 반복하며 cycle마다 새 patternSequence를
발급한다. 같은 sequence에서 stage index를 0으로 되감지 않는다. 최종 패턴 자체의
완료는 본체 사망 또는 명시적 Stop/Next 경계가 결정한다. 큰 duration이나 무한 stage
back-edge로 반복을 위장하지 않는다. Boss Tool의 기존 stable slot에 이 pattern ID를
넣을 수 있게 하되 사용자의 저장 slot 목록을 자동 재작성하지 않는다.

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

Shared spawn에 owner ID를 추가하고 protocol version을 올린다. Server/Client는 같은
revision으로 다시 빌드한다. 본체 죽음·reset·room-empty·퇴장에서는 자식의 Effect와
entity를 함께 정리한다. 늦은 입장은 현재 본체와 자식 및 실제 진행 stage를 받는다.

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
고정 revision, 현재 finale를 요구한다. 초기 진입 복제는 본체를 먼저 보내고 고아·중복
엔티티를 거부한다. 이 admission은 Client의 owner wire 검증과 함께 검사한다.
최종 패턴의 두 번째 이후 cycle은 같은 Flow slot이어도 새 pattern sequence를 보고하며,
세 번째 cycle의 Next·Stop이 실제 sequence로 끝나는지 Server/Client 양쪽에서 검사한다.

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
| 왼손 | capture 상대 translation 유지, 실제 0.01 본 scale의 determinant 임계 거부 | 고정 grip + 정규화 determinant + 실제 scale fixture/실패 rollback |
| 발사 | nav 밖 VOID 판정만으로 FALLING 미발동 | 유한 ejection 종료에 명시적 FALLING, 일반 knockback 보존 |
| 버러지 4종 | miss/partial back-edge, 보조 ID animation만 존재 | 같은 finite fragment 재사용, 시전 counter, nav 종료 전 최종 포획 |
| 포탈 | tick 마지막 위치 누락, hit0으로 전체 경로 미검사 | 최종 pose 확정, 50ms pulse와 leg별 1회 피해 ledger |
| 중앙 파괴 | ARENA_ABSOLUTE가 scale만 변경 | typed center anchor + 기존 world-root handle 경로 |
| 피자 방향 | owner 시작점 기준 yaw와 center 섹터 기준점 불일치 | center→random target 서버 yaw, 같은 fixed root로 지연 섹터 재생 |
| 유령 | primary HUD/BGM 오염·무소유 spawn·무한 sequence 되감기 | owner wire 검증, dependent 필터, 유한 cycle마다 새 sequence |
| 최종 slot | Core8/Animator20 고정과 Tool·Flow publisher 허용 집합 불일치 | G15 공통 split 재생 목록, 개수 제약 제거, Derived는 표시 분류, 저장 순서 보존 |
| 도넛 | foreground와 hazard 수명 결합·worldSpace scale lerp 동결 | CombatObject 2600ms/foreground100ms, birth pose와 scale curve 분리 |
| 사망 | BOSS DEAD가 제거되지 않음, despawn보다 DEAD snapshot이 먼저 온다는 보장 없음 | typed death reason, authority 즉시 종료와 Client clip 수명 분리 |
| 데이터 확장 | 같은 finale/분기의 publisher·Client·Server 허용 범위 차이 | exact 3개 순서·damageable·extent 1~100·전체 finite graph를 각 경계에서 검증 |
| 제품 진입 | 전투 이펙트 2개 고정 로더와 구형 EncounterPatternReference 필드 목록 | 실제 catalog 순회와 새 typed parser, Load 실패 시 기존 문서 보존 |
| 유령 초기 복제 | 같은 encounter의 timeline owner 중복과 무소유 생성 허용 | dependent 제외, 생성 owner admission, 본체 우선 복제·고아/중복 rollback |
| 반복 Flow 제어 | 같은 slot의 새 cycle을 이전 sequence로 종료 | cycle ACTIVE 재보고와 세 번째 cycle Next·Stop 실행 검사 |

위 표는 설계 검토 결과이며 실행 PASS를 뜻하지 않는다. 대응 RESULT에 실제 하네스
명령과 exit code, 남은 사용자 화면 확인을 각각 기록한다.
