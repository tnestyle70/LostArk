# Valtan Phase 2 패턴 기믹 구현 계획서

## 2026-08-27 저장된 Pattern Draft Product 승격 보강

Effect Tool에서 저장한 `DRAFT_ATTACHED`는 authored Effect 파일만 보존하며 Product가 아니다.
사용자가 최종 지정한 `effect.valtan.sequence.charge` 한 건만 element를 재생성하거나 수정하지
않고 다음 transaction으로 승격한다. `effect.valtan.sequence.rush`는 별도 요청 전까지
`DRAFT_ATTACHED`로 보존한다.

- exact authored 경로를 `EffectCatalog.json`의 `DIRECT_AUTHORED_DOCUMENT`로 등록한다.
- aggregate Effect의 0초가 전체 Pattern의 0초와 일치하도록 각 Pattern의 `STEP_01` 첫 clip에
  cue 하나만 연결한다.
- `VALTAN_CHARGE`의 기존 STEP_03 `project-tuned` cue는 제거해 이중 재생을 막되, 기존 authored
  Product 파일과 Catalog row는 비교·복구 입력으로 보존한다.
- Product source와 generated cue의 검증·publish가 성공한 뒤에만 CHARGE의 `DRAFT_ATTACHED`
  row를 제거한다. 사용자가 저장한 authored 파일은 이 과정에서 쓰지 않는다.
- generator는 이 파일을 read-only CAS input으로 검사해 이후 Apply에서도 사용자 element를
  덮어쓰지 않고 같은 단일 Product cue를 유지한다.

## 2026-08-27 사용자 요구 의미 확정

이번 구현은 기존 코드와 생성된 Product의 존재 여부를 완료 기준으로 삼지 않고, 아래 실행 의미를
정본으로 사용한다.

- `VALTAN_DASH_CHARGE`는 돌진 종료 뒤 `RECOVERY` 취약 프레임에서 유효 부위 피해를 받을 때만
  `GROGGY -> PART_BREAK`로 진행한다. 벽 접촉만으로 즉시 그로기에 들어가지 않으며, 취약 프레임 밖의
  공격은 갑옷을 파괴하지 않는다.
- `VALTAN_CHARGE`는 시작 시 선택한 한 플레이어의 stable PlayerId를 끝까지 유지하면서 그 플레이어의
  현재 위치를 추적한다. 매 tick 최근접 플레이어로 target identity를 바꾸지 않는다.
- 3시/9시 지형 파괴는 각각 독립 pattern이며 2초 체공 뒤 착지 world pose에 고정된 빨간 telegraph,
  Server hit와 world-event-set을 같은 착지 edge에서 실행한다.
- `VALTAN_WARP`는 각 rush leg 진입 때 새 생존 플레이어를 선택하고, 갱신한 yaw 기준 발탄 정면의
  world pose에 포탈을 snapshot한 뒤 기존 root-motion 거리만 이동한다. 최초 한 번의 boss-follow 포탈로
  여덟 leg를 대신하지 않는다.
- `VALTAN_TRASH`는 전방 돌진 collider로 포획하되 잡힌 플레이어의 Client presentation offset을 실제
  `bip001-l-hand` world matrix 기준으로 계산한다. WINDUP 카운터 성공 전까지 돌진을 반복하고, 성공하면
  모든 attachment를 해제한 뒤 groggy 성공 경로로 진행한다. Server의 카운터 판정 범위는 bone 이름이
  아니라 저작된 boss-local left-hand gameplay proxy가 소유한다.
- `VALTAN_COUNTER`는 WINDUP의 유효 counter hit만 groggy로 보낸다. TIMEOUT은 내려찍기 판정 후 pattern을
  종료하며 groggy를 공통 후속 stage로 사용하지 않는다.
- `VALTAN_CATCH_BREATH`는 발탄 후방 sector의 플레이어만 포획하고, 왼손 부착과 지연 뒤
  boss-opposite knockback을 사용한다. Product telegraph는 같은 후방 sector를 노란색으로 표시한다.
- 중앙 2페이즈와 피자 착지 카메라는 기존 Server tuple을 보존하되 `BOSS_FACING -> PLAYER_BOSS_FRAME ->
  gameplay follow` 복귀가 끊기지 않도록 유지한다.

## 목표

어제 Animation Tool에서 승격한 Valtan Phase 2 애니메이션 체인을 Server 권위 패턴으로 완성한다. 이름만 존재하는 animation-only row에 Effect와 Camera만 붙이지 않는다. 각 패턴은 `Data/Valtan -> Server fixed tick -> Shared snapshot/event -> Client presentation -> Boss Tool 진단`의 한 경로로 판정, 이동, 지형 파괴, 카메라, Effect가 같은 stage tuple을 사용해야 한다.

사용자가 최종 육안 판정할 대상은 다음과 같다.

- `VALTAN_DASH_CHARGE`: 벽 충돌 뒤 groggy 취약 구간에서 부위 파괴 공격을 받으면 갑옷이 파괴되고 part-break 애니메이션이 재생된다.
- `VALTAN_CHARGE`: 선택된 플레이어를 추적해 회전하고 부채꼴 판정과 모아치기 Effect가 같은 방향으로 나온다.
- `VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK`, `VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK`: 점프 뒤 약 2초 체공, 각 3시/9시 착지 지점의 빨간 판정, 착지와 지형 파괴가 한 stage edge에서 실행된다.
- `VALTAN_ARENA_BREAK_109`: 중앙 이동, 포효, 외벽 파괴와 비산물, 발탄 정면 뒤 카메라, player follow 복귀가 끊기지 않는다.
- `VALTAN_SIX_PIZZA_106`: 점프 착지 구간에서 발탄과 플레이어가 함께 보이는 원거리 카메라를 사용한다.
- `VALTAN_WARP`: 포탈 생성, 살아 있는 임의 플레이어 재지정, 고정 거리 돌진을 animator의 8개 rush occurrence에 맞춰 반복한다.
- `VALTAN_TRASH`: 돌진 충돌 플레이어를 왼손 slot에 잡고 counter 성공 전까지 돌진을 반복하며, 성공 시 전원을 해제하고 groggy/성공 애니메이션으로 진행한다.
- `VALTAN_COUNTER`: WINDUP 중 유효 counter 피격만 groggy 분기로 보내고, 실패하면 내려찍기 판정을 실행한다.
- `VALTAN_CATCH_BREATH`: 후방 부채꼴로 잡은 플레이어를 왼손에 붙인 뒤 발탄 반대 방향으로 해제·넉백한다.
- `VALTAN_ATTACK_WHIRLWIND`, `VALTAN_GROUND_WAVE_SMASH` 계열, `VALTAN_CHARGE`, `VALTAN_CHARGE_2`, `VALTAN_SEQUENCE_FOUR`: 요청된 점프 찍기 후 휠윈드, wave 내려찍기, 모아치기, 검격, 4방향 도끼 Effect를 exact occurrence에 연결한다.

## 현재 실측

### 데이터 정본

- `Data/Valtan/Valtan.animation-chain-promotions.json`의 20개 수동 승격 체인은 `Valtan.gameplay.json`과 `Valtan.presentation.json`에 존재한다.
- 현재 수동 승격 체인은 총 99 stage/99 clip occurrence이지만 모두 `hit.kind=NONE`, `motion=null`, `events=[]`, `branches=[]`, `effectCues=[]`, `cameraInvocations=[]`다. 이름만으로 gameplay가 구현된 상태가 아니다.
- `VALTAN_WARP`의 `mesh_att_battle_18_02` 여덟 occurrence에는 `Data/Animation/RootMotion/Valtan.rootmotion.json`이 만든 Server root-motion 곡선이 이미 있다. 이 곡선은 보존하고 각 leg의 target/portal/hit만 추가한다.
- `VALTAN_DASH_CHARGE`는 Core 패턴으로 이미 BOX 판정, 20m FORWARD, wall-contact, GROGGY, `GROGGY_ONLY` 부위 파괴, part-break presentation을 갖는다. 새 갑옷 시스템을 만들지 않는다.
- `VALTAN_ARENA_BREAK_109`는 중앙 anchor leap, phase 2 전환, 외벽 world event, 여섯 camera cue와 presentation invocation이 이미 연결돼 있다. 이 경로를 확장한다.
- `VALTAN_TERRAIN_DESTRUCTION`, `VALTAN_SIX_PIZZA_106`, `VALTAN_WARP`, `VALTAN_TRASH`, `VALTAN_COUNTER`, `VALTAN_CATCH_BREATH`, `VALTAN_CHARGE/2`는 현재 animation-only다.
- 기존 carrier-v1 Effect 중 portal rush, ground wave, jump takeoff/land, swing, four-pillars cone은 실제 element를 가진 재사용 가능한 문서다. 이름만 남은 semantic stub 중 element가 0인 문서는 연결하지 않는다.

### Server와 Shared 계약

- `BOSS_PATTERN_STAGE_HIT`은 CIRCLE/RING/CONE/BOX/CROSS/SIX_DIRECTIONS, explicit/interval hit schedule, damage/push/knockdown을 지원한다.
- target/aim은 현재 pattern-wide이며 nearest/random one-shot lock 또는 per-tick tracking을 지원한다. `VALTAN_WARP`처럼 leg마다 random player를 다시 잡는 stage-scoped retarget은 없다.
- motion은 FORWARD, authored root-motion, LEAP_TO_TARGET/ANCHOR가 있다. portal relocation과 fixed world anchor origin은 없다.
- `ALL_PLAYERS_GRABBED` enum은 있으나 producer가 없고 `SERVER_PLAYER`, `PLAYER_ACTION_STATE`, `PLAYER_SNAPSHOT`에 grabbed/attachment 상태가 없다.
- 실제 일반 플레이어 skill hit는 legacy `ApplyPlayerHitDamage`로 들어가며 typed counter/stagger/part power를 `CBossCombatRuntime`에 전달하지 않는다. 데이터에 `COUNTER_HIT` branch가 있어도 실전 공격으로 발생하지 않는 기반 공백이다.
- 갑옷은 legacy `ArmorPlates`와 typed `BossCombat.Parts`가 병존하며 Client는 둘을 합쳐 가시성을 정한다. 이번 작업에서 세 번째 상태를 만들지 않고 player-to-boss hit adapter에서 한 번만 갱신한다.

### Client presentation과 Tool

- cinematic runtime 정본은 `ValtanCinematicCamera.json`, graph join은 `Valtan.presentation.json.cameraInvocations`다. 둘 중 하나만 수정하면 Boss Tool과 Arena가 달라진다.
- 현재 camera keyframe은 world 좌표 또는 boss XZ translation만 지원한다. 발탄 yaw 기준 뒤쪽 shot과 player/boss framing이 없다.
- Boss Tool은 direct hit/action/effect/camera token은 보지만 world event set의 member/group/mutation/nav와 combat-object의 origin/movement/hit geometry는 펼치지 못한다.
- Client는 local player의 `CCharacter`와 Valtan model bone을 모두 갖지만 Server는 model/bone/asset을 알면 안 된다. 잡기의 Server 정답은 stable attachment slot과 boss-relative gameplay offset이고, 왼손 bone 행렬 적용은 Client presentation만 담당한다.

## 변경하지 않는 경계

- Client 로컬 AI, 로컬 damage, 로컬 지형 삭제로 Server 권위를 우회하지 않는다.
- animation clip 이름, Prototype tag, vector index를 저장 ID나 wire ID로 쓰지 않는다.
- `Client/Bin/Resources` 전체 팩이나 빌드 산출물을 변경 단위에 포함하지 않는다.
- 진행 중인 Effect/World 다른 세션 diff를 되돌리거나 생성물을 정리하지 않는다.
- Client/UI는 에이전트가 실행하거나 조작하지 않는다. 자동 검증 뒤 사용자가 직접 Server+Client와 Boss Tool로 육안 확인한다.

## G00. 기준선과 실패 재현 하네스

### 수정 파일

- `Server/Private/ServerGameplayContractTests.cpp`
- `Tools/ValtanPipeline/test_valtan_pattern_master_v2.py`
- `Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py`
- 필요 시 새 focused Python contract test를 `Tools/ValtanPipeline`에 추가한다.

### 구현

현재 animation-only 상태를 실패 사례로 고정한다. 패턴별로 exact pattern/stage/action tuple, non-NONE hit 또는 motion, target/aim, required Effect cue, required camera invocation, world binding을 검사한다. grab은 Server state transition과 snapshot wire round-trip을 실행형 test로 검증한다. counter는 실제 player skill hit 진입점부터 `COUNTER_HIT` outcome까지 통과시킨다.

### 종료 증거

구현 전 새 검사는 누락 계약 때문에 실패하고, 각 G가 끝날 때 해당 실패만 순서대로 닫힌다.

## G01. Player-to-Boss hit 권위 통합과 갑옷/카운터

### 수정 파일

- `Server/Public/BossCombatRuntime.h`
- `Server/Private/BossCombatRuntime.cpp`
- `Server/Private/PlayerSkillSystem.cpp`
- `Server/Private/ServerCombatHitRuntime.cpp`
- `Server/Public/ServerWorldEntity.h`
- `Server/Private/ServerGameplayContractTests.cpp`
- `Data/Balance/ValtanBossParts.json`
- `Data/Valtan/Valtan.gameplay.json`

### 구현

일반 skill, projectile, combat object의 boss hit가 한 typed adapter를 통과하게 한다. adapter는 health damage를 중복 적용하지 않고 counterPower, staggerPower, partPower와 대상 part를 한 번 평가해 `COUNTER_HIT`, `STAGGER_BROKEN`, `PART_DESTROYED`를 publish한다. 기존 armor mask와 typed part mask가 같은 공격에서 불일치하지 않도록 기존 `ArmorPlates` 갱신을 adapter 내부 호환 단계로 제한한다.

`VALTAN_DASH_CHARGE`는 `CHARGE -> RECOVERY` 뒤 RECOVERY 취약 프레임의 유효 part hit를
`GROGGY -> PART_BREAK`로 연결한다. `mesh_dmg_parts_loop_1/end_1`과 기존 armor mask consumer는
재사용한다. `VALTAN_COUNTER`는 WINDUP에 counterable flag를 켜고 `COUNTER_HIT`은 groggy action,
TIMEOUT은 slam action으로 분기하며 slam 종료는 pattern finish다.

### 실패 보존

counter window 밖 hit, partPower 0, 이미 파괴된 part, dead/falling player 공격은 outcome을 만들지 않는다. 동일 hit token 재처리는 durability와 outcome을 중복 적용하지 않는다.

## G02. 추적 모아치기와 기본 Phase 2 판정/Effect

### 수정 파일

- `Data/Valtan/Valtan.gameplay.json`
- `Data/Valtan/Valtan.presentation.json`
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`
- `Data/Actors/BossCatalog.json`
- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`
- 관련 Effect/Valtan publisher tests

### 구현

`VALTAN_CHARGE`는 시작 시 살아 있는 target 하나를 잠그고 active 동안 같은 PlayerId의 현재 위치를
`TRACK_TARGET_EACH_TICK`으로 추적해 boss yaw를 갱신한다. end occurrence에 CONE 판정과 실제
carrier-v1 cone Effect를 같은 tuple로 연결한다. `VALTAN_CHARGE_2`는 end-2 occurrence에 검격 CONE와
swing/wave carrier를 연결한다.

`VALTAN_ATTACK_WHIRLWIND`는 jump takeoff/land와 whirlwind interval circle을 animator step에 맞춘다. `VALTAN_SEQUENCE_FOUR`는 four-direction hit schedule과 실제 보이는 Effect closure를 연결한다. 내려찍기는 ground-wave carrier와 CONE/CROSS 판정이 같은 발생 시각을 사용한다.

수동 승격 projector가 gameplay authoring을 다시 NONE으로 덮지 않도록 promotion receipt는 animation lineage만 검증하고 gameplay/presentation enrichment를 보존하도록 확장한다.

## G03. 3시/9시 점프 착지와 Server 월드 파괴

### 수정 파일

- `Data/Valtan/Valtan.gameplay.json`
- `Data/Valtan/Valtan.presentation.json`
- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`
- `Data/Encounters/Valtan/ValtanWorldEvents.json`
- `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`
- `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction.json`
- `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestructionpresentation.json`
- `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1`
- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`
- `Server/Private/WorldDestructionBootstrapContractTests.cpp`

### 구현

기존 긴 `VALTAN_TERRAIN_DESTRUCTION` animator chain은 source audition으로 보존하고, 실제 제품 graph는 다음 두 stable pattern으로 분리한다.

- `VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK`
- `VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK`

두 패턴은 `mesh_att_battle_12_01/02/03`의 takeoff, 약 2초 apex hold, landing을 각각 독립 occurrence로 가진다. Server는 명시된 3시/9시 world anchor로 leap하며 착지 telegraph는 boss-follow가 아니라 landing world pose에 snapshot한다. LAND exact hit frame에는 landing collider와 world-event-set을 같은 transaction으로 commit한다. 기존 30/84 half가 실제 9시/3시 authored floor와 일치하면 그 group/mutation/nav를 stable set으로 재사용하고, 일치하지 않으면 authoring placement를 먼저 추가한 뒤 publisher가 runtime 문서를 생성한다.

world event 실패 시 stage와 collision/navigation mutation을 부분 적용하지 않고 이전 상태를 유지한다.
`valtan_tuning_pipeline.py`의 109 단일 set/30 binding/60 placement 하드코딩은 typed
multi-set closure로 일반화한다. 3시 패턴은 기존 floor84 세 group, 9시 패턴은 floor30
세 group의 mutation/navigation을 소유권 이전해 재사용하며, legacy 84/33 flat binding을
중복 실행 경로로 남기지 않는다.

## G04. 중앙 컷씬과 피자 카메라

### 수정 파일

- `Data/Encounters/Valtan/ValtanCinematicCamera.json`
- `Data/Valtan/Valtan.presentation.json`
- `Client/Public/ValtanCinematicCameraDocument.h`
- `Client/Private/ValtanCinematicCameraDocument.cpp`
- `Client/Public/ValtanCinematicCameraController.h`
- `Client/Private/ValtanCinematicCameraController.cpp`
- `Client/Private/Level_ValtanArena.cpp`
- `Client/Private/BossTool.cpp`
- Camera/Valtan contract tests

### 구현

camera cue에 좌표 기준을 명시하는 typed frame을 추가한다.

- `WORLD`: 기존 절대 좌표.
- `BOSS_FACING`: boss yaw로 eye/look-at local offset을 회전해 발탄 정면 뒤 shot을 만든다.
- `PLAYER_BOSS_FRAME`: local player와 boss의 중점, 거리, 높이를 사용해 둘을 함께 프레이밍한다.

`VALTAN_ARENA_BREAK_109`의 wide/recovery 구간은 BOSS_FACING에서 시작해 player follow pose로 lerp한 뒤 기존 follow target을 복구한다. 외벽 파괴와 비산물은 기존 IMPACT world event를 유지한다. `VALTAN_SIX_PIZZA_106`은 실제 Server leap motion을 먼저 추가하고 landing tuple에 전용 PLAYER_BOSS_FRAME cue/invocation을 추가한다.

Camera JSON과 presentation invocation의 exact tuple join을 validator와 Boss Tool이 모두 확인한다.

## G05. 포탈과 leg별 재지정 돌진

### 수정 파일

- `Server/Public/GameplayCatalog.h`
- `Server/Private/GameplayCatalog.cpp`
- `Server/Public/ServerWorldEntity.h`
- `Server/Private/ValtanBrain.cpp`
- `Data/Valtan/Valtan.gameplay.json`
- `Data/Valtan/Valtan.presentation.json`
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`
- `Server/Private/ServerGameplayContractTests.cpp`

### 구현

pattern-wide target policy를 깨지 않고 stage entry에서만 실행되는 typed retarget action을 사용한다. 각 rush leg는 시작할 때 살아 있는 임의 플레이어의 last-known position과 yaw를 확정한다. 같은 stage entry snapshot에서 target 방향의 boss 앞 stable offset에 world-fixed portal presentation을 만들고, 각 `mesh_att_battle_18_02` occurrence는 기존 root-motion 곡선과 BOX 판정, portal-rush carrier Effect를 함께 사용한다. 거리 제한에 도달하거나 stage 시간이 끝나면 다음 leg로 넘어가며 마지막 `18_03-2`에서 복구한다.

target이 죽거나 사라지면 다음 leg에서 다시 선택하고, 현재 leg는 이미 잠근 방향을 유지한다. Client가 target 위치나 이동 결과를 Server로 보내지 않는다.

## G06. 잡기, 왼손 부착, 해제와 날리기

### 수정 파일

- `Shared/Public/Network/PacketMessages.h`
- Shared packet codec/contract harness 파일
- `Server/Public/GameplayCatalog.h`
- `Server/Private/GameplayCatalog.cpp`
- `Server/Public/ServerPlayer.h`
- `Server/Private/GameRoom.cpp`
- `Server/Private/ValtanBrain.cpp`
- `Client/Public/Character.h`
- `Client/Private/Character.cpp`
- `Client/Public/Valtan.h`
- `Client/Private/Valtan.cpp`
- `Client/Private/ClientReplication.cpp`
- `Data/Valtan/Valtan.gameplay.json`
- `Data/Valtan/Valtan.presentation.json`
- 관련 Server/Network/Client contract tests

### 구현

`PLAYER_ACTION_STATE::GRABBED`와 stable `BOSS_ATTACHMENT_SLOT::LEFT_HAND`를 wire enum 끝에 추가한다. snapshot은 attachment owner entity ID와 boss-local gameplay offset을 전달한다. Server는 grab hit가 확정될 때 player movement/skill/pending command를 취소하고 owner/slot/offset을 저장하며, fixed tick마다 boss-relative 위치를 갱신한다. 잡힌 player는 target selection, 충돌 damage 재적용, 입력 승인에서 제외한다.

Client는 GRABBED edge에서 현재 player world와 Valtan body의 authored left-hand bone world로 hand-local
offset matrix를 한 번 계산하고, 이후 `handLocalOffset * leftHandWorld`로 presentation transform만 갱신한다.
Server boss-root offset을 hand-local offset으로 재해석하지 않는다. bone/owner가 없으면 snapshot의 Server gameplay
위치를 유지하고 기존 character를 파괴하지 않는다.

typed stage action은 `RELEASE_GRABBED_PLAYERS`를 지원한다. release mode는 제자리 해제와 boss-opposite knockback을 구분하고 기존 knockback runtime을 재사용한다.

- `VALTAN_TRASH`: frontal BOX grab, 반복 rush, boss-local left-hand counter proxy window, 성공 시 모든 grabbed player 해제와 success/groggy sequence, 실패 시 다음 rush leg.
- `VALTAN_CATCH_BREATH`: yaw가 180도 반전된 rear yellow CONE grab, hold, left-hand attachment, delay 뒤 boss 반대 방향 release knockback.

stage 전환 실패, boss 사망/despawn, player death/revive, level leave/disconnect에서도 attachment를 정리한다.

## G07. Boss Tool 진단, sequence, publish와 빌드

### 수정 파일

- `Client/Private/BossTool.cpp`
- `Client/Public/BossTool.h`
- `.md/TEAM/보스툴.md`
- `Data/Valtan/Valtan.gameplay.json`의 ordered audition sequence
- `.md/GB/08-26/2026-08-26_VALTAN_PHASE_TWO_PATTERN_MECHANICS_RESULT.md`

### 구현

Boss Tool의 단순한 pattern list와 `Play Selected`는 유지한다. 선택 패턴 detail에서만 다음 read-only join을 펼친다.

- combat object origin, movement, hit trigger/shape/damage, visual Effect ID
- world event set, member binding, group/mutation, collision/nav와 placement ID
- camera invocation과 runtime cue exact tuple/frame
- grab/release attachment slot과 release mode

ordered Server audition은 기존 Core 6 뒤에 실제 Phase 2 pattern들을 순서대로 둔다. 원본 animator audit row는 All Effects/Boss Tool inventory에서 추적 가능하게 유지하되, 제품 sequence에서는 3시/9시 분리 패턴과 구현 완료 pattern을 사용한다.

## 검증 순서

1. JSON parse와 `valtan_tuning_pipeline.py --check` 및 pattern master/tree/effect contract tests.
2. `Publish-GameplayBalance.ps1 -ValidateOnly`와 Valtan world destruction publisher validation.
3. Shared + NetworkProtocolHarness Debug/Release와 packet round-trip.
4. Server Debug/Release, `Server.exe --contract-test`, focused gameplay/world destruction tests.
5. Client Debug/Release compile/link. 실행 중인 `Client.exe`가 출력 파일을 점유하면 에이전트가 종료하지 않고 compile 성공과 link blocker를 분리 보고한다.
6. `git diff --check`와 변경 JSON/XML parse.
7. 사용자가 `Server + Client`로 Valtan Arena 진입 후 Boss Tool에서 각 패턴을 `Play Selected`/Repeat로 직접 육안 검증한다. Effect, 카메라, collider 방향, 지형 파괴, grab/해제와 갑옷 mesh 결과의 visual PASS는 사용자 서면 확인 전까지 미검증으로 남긴다.

## 수동 육안 체크 순서

1. `VALTAN_DASH_CHARGE`: 벽 충돌 전 공격은 갑옷을 깨지 않고, groggy 중 부위 공격만 part-break로 간다.
2. `VALTAN_CHARGE`, `VALTAN_CHARGE_2`: target이 움직일 때 발탄, cone debug collider, Effect가 같은 방향을 본다.
3. 3시/9시 지형 파괴: takeoff 뒤 약 2초 대기, 빨간 telegraph, 착지 hit와 올바른 쪽 floor만 파괴되는지 본다.
4. `VALTAN_ARENA_BREAK_109`: 중앙 이동/포효/외벽 비산물/카메라 복귀 뒤 조작 카메라가 정상인지 본다.
5. `VALTAN_SIX_PIZZA_106`: 착지 동안 발탄과 local player가 프레임 안에 있고 종료 후 follow로 복귀하는지 본다.
6. `VALTAN_WARP`: 각 rush 전 portal과 새 target 방향, 고정 거리 이동, 여덟 occurrence의 Effect 반복을 본다.
7. `VALTAN_TRASH`: frontal collision player만 왼손에 붙고 counter 전 반복, counter 성공 즉시 해제되는지 본다.
8. `VALTAN_COUNTER`: WINDUP counter는 groggy, 늦은 공격은 slam으로 구분되는지 본다.
9. `VALTAN_CATCH_BREATH`: 후방 yellow cone 대상만 붙고 지연 뒤 발탄 반대편으로 날아가는지 본다.
