# Encounter Sequencer · Companion AI · Account 구현 계획서

## 0. 2026-08-31 우선순위 축소 결정

사용자 실행 검증을 먼저 닫기 위해 이 문서의 G04~G07(Companion AI, Account, Login,
Dialogue)은 후속 작업으로 보류한다. 현재 변경 단위는 다음 네 결과만 소유한다.

1. Composition에서 선택한 Valtan Animation Sequence를 Valtan Arena의 명시적 preview
   target에서 바로 재생한다.
2. 도끼 Pattern의 AIRBORNE Stage duration과 Warp의 rush speed, distance, leg delay,
   portal Effect 연결을 Details에서 편집한다.
3. 사용자에게 별도 Publish 단계를 노출하지 않고 `Save & Apply` 한 번으로 source validation,
   atomic save, Product projection, Server-active revision 확인을 수행한다.
4. Encounter Sequencer가 stable Pattern node와 finite repeat를 저장·연결하고 같은 Server graph를
   실행한다.

`Publisher` 안전 경계를 코드에서 제거하지 않는다. 저작 JSON을 Server가 직접 읽게 만들거나
validation을 생략하면 source와 active runtime이 다시 갈라지므로, 단지 UI/사용 흐름에서 내부
구현 상세로 숨긴다. 이후 범위는 위 네 항목의 Debug build와 사용자 Ctrl+F5 smoke가 끝난 뒤
재개한다.

## 1. 목표

이번 작업은 세 개의 서로 다른 제품 책임을 하나의 데이터 흐름으로 연결한다.

1. `Composition Workbench`는 한 Boss Pattern 내부의 Animation, Effect, Sound, Stage,
   Logic, Collider, Camera를 완성한다.
2. `Encounter Sequencer`는 완성된 Pattern ID를 node로 놓고 실행 순서, 유한 반복,
   이후 추가될 권위 결과 분기를 연결한다.
3. `Character AI Workbench`는 Server Companion의 추종, 회피, 전투 후보와 판단 근거를
   편집하고 실제 Server trace를 읽는다.
4. Account Backend는 로그인, 회원가입, 게스트, 캐릭터 6개 슬롯과 성장/보유 정보를
   Server 권위로 저장하고 다시 실행했을 때 복원한다.
5. Companion 대화는 비동기 Backend provider로 분리하고 이동, 피격, 스킬, 보상 같은
   gameplay 권위를 절대 갖지 않는다.

본질적인 사용자 흐름은 다음과 같다.

```text
Pattern 내부 완성
  Composition Workbench
          ↓ stable patternId
Raid 순서와 반복 구성
  Encounter Sequencer
          ↓ published graph revision
Server가 Boss와 Companion을 fixed tick으로 실행
          ↓ snapshot + bounded decision trace
Client가 연출하고 Workbench가 근거를 표시

Lobby
  ├─ Login / Register → Account → 6 Character Slots → Character Select Arena
  └─ Guest            → 임시 identity → 기존 비회원 진행
```

## 2. 현재 코드 실측과 교정

| 경계 | 현재 정본 | 이번 판정 |
|---|---|---|
| Pattern 내부 | `Valtan.presentation.json`과 Composition authoring/runtime | Encounter가 lane 데이터를 복사하지 않고 `patternId`만 참조한다. |
| Boss 대단위 순서 | `ValtanBossAuditionFlows.json` v1의 29개 linear slot | 같은 문서를 v2 graph로 승격한다. 새 `sequence.json`과 세 번째 runner를 만들지 않는다. |
| Product Boss 실행 | `CGameplayCatalog::BOSS_PATTERN_SEQUENCE_DEFINITION -> CValtanBrain` | Debug와 Product가 같은 pinned graph를 소비한다. |
| Debug Flow 시작 | Client가 saved slot 전체를 packet에 다시 넣고 Server가 임시 sequence를 생성 | identity와 revision만 보내고 Server catalog가 graph를 resolve하도록 바꾼다. |
| Valtan 참가자 | 일반 `playerSpawn` 8개, 최대 인간 8명 | 이전 4 human + 4 bot 설계와 `serverBotSpawn` 전제는 폐기한다. 빈 일반 참가자 슬롯에 Companion 한 명부터 stage한다. |
| player gameplay | move/skill ingress가 session 검증과 actor 실행을 한 함수에서 수행 | human adapter와 Companion producer가 같은 actor executor를 사용하도록 분리한다. |
| party | Server 권위 최대 4명, stable `NetEntityId` 초대 | 같은 방 Companion auto-accept부터 닫고, world transfer는 control-kind-aware transaction으로 별도 확장한다. |
| chat | Server relay와 world bubble이 이미 존재 | Companion 답변도 같은 `S2C_CHAT` 표현 경로를 사용한다. |
| character identity | `CCharacterSelectionState`의 process-local 단일 nickname/class | Account identity와 6-slot roster를 별도 state로 추가하고 guest 계약은 보존한다. |
| Level | `LOBBY`, `CHARACTER_SELECT`가 이미 고정 계약 | 새 Level enum을 만들지 않는다. Lobby 안에서 Auth mode를, Character Select 안에서 Slot/Roster mode를 소유한다. |
| inventory | Server `SERVER_PLAYER`의 Debug inventory와 `ItemCatalog` | Account가 quantity/equipment instance를 소유하고 입장 때 Server가 검증해 actor state로 stage한다. |
| 기존 Backend | `C:/Users/user/Desktop/Winters/Services`의 auth/profile/PostgreSQL/Docker, bcrypt/JWT/migration | 두 번째 서비스를 만들지 않고 기존 auth/profile에 LostArk aggregate를 추가한다. dev passwordless ID API와 기존 `.env`/credential은 재사용하지 않는다. |

이미 완료된 Composition 회귀 수정은 `3bd06b02`에 고정되어 있다. Save/Preview/Blueprint와
Create New Pattern의 프레임 저하 수정은 이 계획의 선행 조건이며 다시 구현하지 않는다.

## 3. 변경하지 않을 권위 경계

### 3.1 Composition과 Encounter

- Encounter node는 `patternId`와 node 실행 정책만 저장한다.
- Animation/Effect/Sound/Camera/Collider/Stage 상세값은 Composition 정본에 남는다.
- Encounter UI에서 node를 선택하면 Composition Workbench의 해당 Pattern으로 이동한다.
- Stage 이름, 애니메이션 이름, 화면 node 좌표로 gameplay 분기를 추론하지 않는다.

### 3.2 수치와 로직

- Workbench가 편집하는 것: 거리, tick 간격, hysteresis, score weight, 허용 skill ID,
  유한 repeat, watchdog, 제공자 timeout처럼 schema로 제한된 값.
- C++/Server가 소유하는 것: 인증, anti-replay, navigation admission, damage shape,
  cooldown/resource, inventory transaction, 최대 반복 한도, 비밀키와 provider circuit breaker.
- Tool은 임의 C++ 표현식이나 script 문자열을 저장하지 않는다.

### 3.3 대화와 Gameplay

- LLM은 raid 설명과 자연어 대화만 만든다.
- Companion의 이동/회피/스킬 선택은 Server fixed tick의 결정론적 brain만 수행한다.
- API timeout, rate limit, provider 장애가 room tick을 block하지 않는다.
- API key는 Client, JSON, Git, packet에 들어가지 않고 Backend 환경 변수/secret에만 둔다.

### 3.4 Account와 Client

- Client는 password hash, level, item level, inventory 결과를 확정하지 않는다.
- 로그인 token으로 Backend가 account와 character를 resolve하고, Game Server가 검증된
  profile ticket 또는 server-to-server 조회 결과만 입장 상태로 commit한다.
- 실패한 load는 현재 roster/level을 부분 교체하지 않는다.
- Guest는 영구 account로 위장하지 않고 process-session identity만 사용한다.

## 4. 목표 데이터와 호출 흐름

### 4.1 Encounter graph v2

`Data/Encounters/Valtan/ValtanBossAuditionFlows.json`을 유일한 저작 정본으로 유지한다.

```text
flow
  flowId
  entryNodeId
  nextNodeOrdinal / nextEdgeOrdinal
  defaultPursuitMs
  maxTransitionsPerRun
  nodes[]
    nodeId, patternId, watchdogMs
  edges[]
    edgeId, fromNodeId, outcome, toNodeId, pursuitMs
    optional finite maxTraversals for a back-edge
```

첫 버전에서 admit하는 outcome은 `COMPLETED`와 watchdog을 실제로 구현한 경우의
`WATCHDOG_TIMEOUT`뿐이다. `COUNTER_SUCCESS`, `STAGGER_SUCCESS`는 Pattern runtime이 stable
exported exit receipt를 Server에 남기는 다음 수직 슬라이스 전에는 UI에도 선택지로 보이지 않는다.

outgoing `COMPLETED` edge가 없는 node가 정상 terminal이다. 검증은 node/edge ID와 ordinal, 유일 entry, reachability, dangling edge, 같은
`(fromNodeId, outcome)` 중복, terminal 경로, 모든 cycle의 유한성, transition 상한,
Pattern inventory join을 포함한다. 기존 29개 `slotId`는 그대로 `nodeId`가 된다.

G01의 finite repeat는 `maxTraversals`가 있는 back-edge를 허용하고 cap이 소진된 source node에서
정상 terminal hold한다. 반복을 마친 뒤 별도 다음 node로 빠지는 fallthrough는 동일 outcome 두 개를
모호하게 만들지 않도록 exported outcome/conditional-edge 수직 슬라이스와 함께 추가한다.

```text
Client Save
  parse → validate → serialize/reparse → source CAS → durable replace
       ↓
Publisher
  Pattern inventory join → graph projection → candidate gameplay revision
       ↓
Server commit
  immutable CGameplayCatalog generation
       ↓
Start request
  flowId + flowSourceRevision + startNodeId + expected definition revision
       ↓
CValtanBrain
  current node → Pattern terminal receipt → authoritative edge → bounded cursor
```

### 4.2 Companion profile와 trace

새 저작 정본은 `Data/AI/CompanionProfiles.json`이다.

```text
profileId, characterClass, displayNickname
partyInvitePolicy
decisionIntervalTicks
followStartDistance / followStopDistance / maxLeaderDistance
navReplanTicks / navReplanDistance
evadeHorizonMs / hazardMargin / sampleRing
skillCandidates[]
  candidateId, skillId, minRange, maxRange, baseScore, considerationWeights
traceCapacity
```

skill damage, cooldown, resource와 animation은 복제하지 않고 `PlayerSkills.json`과 기존
presentation을 stable `skillId`로 join한다. Server decision은 다음 우선순위를 사용한다.

```text
EVADE_HAZARD → FOLLOW_LEADER → COMBAT_SKILL → BASIC_ATTACK → HOLD
```

동점은 stable candidate ID로 해결하고 첫 버전은 난수를 사용하지 않는다. hazard truth는
Effect/PhysX가 아니라 Server가 실제 damage에 사용하는 XZ primitive와 combat object다.

```text
human packet → session/auth/anti-replay → normalized actor intent ┐
Companion brain → deterministic sequence → normalized intent     ├→ common executor
                                                               ┘
common executor → navigation/cooldown/resource/action/damage → snapshot
```

trace는 observed tick, profile revision, leader/target/hazard ID, 각 gate와 consideration의
input/weight/contribution, 최종 score, 선택된 destination/skill을 bounded ring으로 보존한다.

### 4.3 Account service

실행 중인 서비스 owner는 `C:/Users/user/Desktop/Winters/Services`의 기존 auth/profile과
PostgreSQL/migration이다. LostArk 저장만을 위해 두 번째 auth/profile 서비스를 만들지 않는다.
기존 shop/payment 등 무관한 도메인은 건드리지 않고 profile service 안에 LostArk aggregate를
추가한다.

```text
accounts
  account_id UUID, login_id normalized unique, password_hash, created_at
sessions
  refresh token digest, account_id, expires_at, revoked_at
characters
  character_id UUID, user_id, create_request_id, slot_index 0..5, class_id,
  nickname, character_level, item_level, revision, initialized
character_inventory
  item_instance_id, character_id, item_definition_id, quantity,
  enhancement_level, inventory_slot
character_equipment
  character_id, equipped_slot, item_instance_id
```

- password는 bcrypt/Argon2 계열 hash만 저장한다.
- `(user_id, slot_index)`와 `(user_id, create_request_id)`는 DB unique constraint로도 막는다.
- 현재 제품 계약은 nickname 중복을 허용하므로 nickname global unique를 첫 slice에 추가하지 않는다.
- 한 요청의 character/inventory/equipment 변경은 한 transaction이다.
- character revision으로 stale update를 거부한다.
- `item_definition_id`는 LostArk `ItemCatalog`와 Backend/Game Server 양쪽 publisher가 검증한다.
- entry ticket은 Redis에 digest만 두고 짧은 TTL, target world, one-time consume,
  idempotent `consume_id`에 결합한다.
- HTTP body, header, response size와 timeout은 bounded이고 password/token을 로그에 남기지 않는다.
- Client HTTP worker 종료는 cooperative cancellation + bounded join을 사용한다.

## 5. Tool 화면

### 5.1 Encounter Sequencer

```text
┌ Encounter Sequencer ───────────────────────────────────────────────────────────┐
│ Flow: Valtan Default  [Draft] [Saved] [Published] [Server Active]             │
│ [Reload] [Save + Validate + Publish] [Start Entry] [Start Here] [Stop After] │
├ Pattern Resources ─┬──────────── Graph ───────────────┬ Details ──────────────┤
│ Search...           │ [Entry] Whirlwind               │ Node ID (read-only)   │
│ Whirlwind           │        │ COMPLETED              │ Pattern: Whirlwind    │
│ High Jump           │        ▼                        │ Watchdog: Off         │
│ Warp                │      High Jump ──┐              │ [Open Composition]    │
│ Counter ...         │        ▲          │ Repeat x2    ├ Edge ────────────────┤
│                     │        └──────────┘              │ Outcome: COMPLETED    │
│ [+ Add Pattern]     │             │ COMPLETED          │ Pursuit: 1000 ms      │
│                     │             ▼                    │ Repeat: 2 traversals  │
│                     │          [Terminal]              │ [Delete Edge]         │
└─────────────────────┴──────────────────────────────────┴───────────────────────┘
```

그래프 좌표/zoom은 gameplay JSON에 저장하지 않는다. render frame은 immutable snapshot만 읽고
JSON parse와 layout rebuild는 Open/Reload/mutation 때만 수행한다.

### 5.2 Character AI Workbench

첫 버전은 topology가 없는 profile 편집이므로 node graph를 억지로 사용하지 않는다.

```text
┌ Character AI Workbench ───────────────────────────────────────────────────────┐
│ Profile [Valtan Companion]  [Reload] [Save] [Validate + Publish]             │
├ Profiles ─────┬ Follow & Survival ─┬ Combat Loadout ─┬ Live Trace / Evidence ┤
│ class         │ follow 4.0 / 2.2 m │ Skill Q  score  │ Tick 18420            │
│ nickname      │ evade 1200 ms      │ Skill W  score  │ EVADE_HAZARD 930      │
│ invite policy │ hazard margin      │ Basic     score  │ FOLLOW_LEADER 410     │
│ revision      │ replan ticks       │ gates/reasons    │ chosen destination    │
└───────────────┴─────────────────────┴──────────────────┴───────────────────────┘
```

Tool은 저장 성공과 Server 적용을 분리해 `Draft / Saved / Published / Restart Required /
Server Active Revision`을 표시한다.

### 5.3 Lobby와 6-slot Character Select

```text
Lobby Login
  Login ID [................]
  Password [................]
  [Login] [Register] [Guest]

Character Select
  [Slot 0] [Slot 1] [Slot 2]
  [Slot 3] [Slot 4] [Slot 5]

Selected Slot
  nickname / class / character level / item level
  equipped items / inventory summary
  [Enter] [Create] [Delete with confirmation]
```

이 화면은 제품 UI이므로 ImGui로 제품 런타임을 대체하지 않는다. `Data/UI` stable slot ID와
`CUIObject` 계열을 사용하고, F1 ImGui는 account/session/slot revision을 읽는 Debug Inspector만
제공한다. 비밀번호 입력·token 복사·관리자 mutation을 Debug Tool에 넣지 않는다.

## 6. G별 구현 범위

### G00. 선행 Composition 회귀 고정

- commit `3bd06b02`의 Save, Preview, Create New Pattern frame, Blueprint graph 계약을 유지한다.
- 새 작업은 별도 commit으로 분리한다.
- `VALTAN_HIGH_JUMP/AIRBORNE`의 현재 정본 6500 ms를 임의로 8000 ms로 바꾸지 않는다.

종료 증거: focused 165 tests, native graph 9 tests, Product Debug build receipt를 baseline으로 보존한다.

### G01. Encounter graph authoring v2

수정 대상:

- `Data/Encounters/Valtan/ValtanBossAuditionFlows.json`
- `Client/Public/ValtanPatternFlowDocument.h`
- `Client/Private/ValtanPatternFlowDocument.cpp`
- Pattern Flow document contract tests

구현:

- 29개 slot identity를 보존한 v1 → v2 migration
- node/edge/entry/finite repeat 모델
- strict parser와 full graph validator
- 기존 선형 Add/Move/Remove가 v2 graph를 손상하지 않는 호환 projection
- source revision CAS와 실패 rollback

종료 증거: migration identity, malformed/dangling/nondeterministic/unbounded cycle 거부,
serialize/reparse equality, external conflict 보존.

### G02. Product와 Debug의 단일 graph runtime

수정 대상:

- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`
- `Client/Private/ValtanPatternTree.cpp`
- `Server/Public|Private/GameplayCatalog.*`
- `Server/Public/ServerWorldEntity.h`
- `Server/Public|Private/ValtanBrain.*`
- `Shared/Public/Network/PacketType.h`
- `Shared/Public|Private/Network/PacketMessages.*`
- `Server/Public|Private/GameRoom.*`
- `Client/Public|Private/ValtanPatternFlowService.*`

구현:

- Product bootstrap과 Client projection에 동일 graph 저장
- `CValtanBrain`의 bounded graph cursor와 terminal receipt edge 선택
- Debug Start packet을 identity/revision/startNode만 남기고 protocol version 갱신
- Server가 active catalog의 exact graph를 resolve하고 pinned generation을 실행
- Start Here, finite loop, Stop After Current, stale revision과 owner lifecycle

종료 증거: Product/Debug 동일 node 순서, repeat 횟수, wire round-trip, retry/idempotency,
Release Debug reject, catalog rollback.

### G03. Encounter Sequencer Tool

수정 대상:

- `Client/Public|Private/BossTool.*`
- 필요한 경우 순수 graph layout/model 새 파일과 Client/harness `.vcxproj/.filters`
- `Client/Public|Private/MainApp.*`

구현:

- 기존 Pattern Flow owner를 독립 창 `Encounter Sequencer`로 표현
- resources/canvas/details 3분할
- drag add, duplicate, connect completed, set entry, delete, finite repeat
- 모든 graph mutation의 stage → validate → commit과 실패 시 기존 draft 보존
- selected node의 Composition deep-link
- Draft/Saved/Published/Server Active 네 상태와 validation reason
- per-frame disk/JSON scan 금지

종료 증거: open/reload/edit/save/publish/reload, failed mutation state preservation,
Start Entry/Here lifecycle. 화면 fidelity는 사용자가 직접 판정한다.

### G04. Server Companion과 Character AI Workbench

데이터/Publisher:

- `Data/AI/CompanionProfiles.json`
- `Tools/AIPipeline/Publish-CompanionAI.ps1`
- `Tools/Build/BuildDomains.json`

Server:

- `Server/Public|Private/CompanionAICatalog.*`
- `Server/Public|Private/CompanionBrain.*`
- `Server/Public|Private/ServerHazardQuery.*`
- `Server/Public/ServerPlayer.h`
- `Server/Public|Private/PlayerSkillSystem.*`
- `Server/Public|Private/GameRoom.*`

Shared/Client:

- bounded trace query/response codec
- `Client/Public|Private/CharacterAIWorkbench.*`
- `MainApp`, `NetworkManager`의 debug command/receipt 연결
- 새 C++ 파일의 `.vcxproj/.filters` 등록

구현 순서:

1. `SERVER_PLAYER_CONTROL_KIND`와 Session 없는 Companion stage/commit
2. human adapter와 공통 move/skill actor executor 분리
3. fixed tick `EVADE → FOLLOW → SKILL → BASIC → HOLD`
4. 같은 Server damage geometry를 쓰는 hazard query
5. party acceptance 공통 commit과 Companion auto-accept
6. last-human leave 시 Companion 취소/despawn 후 room reset
7. profile authoring과 live decision trace Workbench

첫 Companion은 빈 일반 spawn 하나를 사용하고 총 8인 capacity에 포함한다. Bern→Valtan party
transfer는 모든 멤버가 Session을 가진다는 현재 transaction을 control-kind-aware하게 바꾸는
다음 commit에서 닫는다.

종료 증거: human/AI executor parity, boss damage 1회, 추종 hysteresis, 모든 Valtan shape 회피,
unreachable destination 거부, party roster, last-human cleanup, deterministic trace digest.

### G05. LostArk Account Backend

먼저 LostArk item 권위를 닫는다.

- `Data/Items/ItemCatalog.json`에 `equipSlot`, `allowedCharacterClasses`,
  `baseItemLevel`, `maxEnhancementLevel`을 typed schema로 추가
- `Data/Items/CharacterStartingLoadouts.json`에 class별 시작 장비를 stable item ID로 선언
- `Publish-ItemCatalog.ps1`, Client/Server `ItemCatalog`, bootstrap version과 focused tests 갱신
- filename/icon/Client `m_ItemUpgradeLevels`로 equipment나 item level을 추론하지 않음

초기 account character는 ticket consume 뒤 `initialized=false` aggregate를 받고, Game Server가
published starting loadout을 구성해 Backend initialize CAS를 성공시킨 뒤에만 room entry를 commit한다.

Winters 저장소의 수정 대상:

- `C:/Users/user/Desktop/Winters/Services/internal/lostark/model.go`
- `C:/Users/user/Desktop/Winters/Services/internal/lostark/repository.go`
- `C:/Users/user/Desktop/Winters/Services/internal/lostark/service.go`
- `C:/Users/user/Desktop/Winters/Services/internal/lostark/handler.go`
- `C:/Users/user/Desktop/Winters/Services/migrations/000014_lostark_character_roster.up.sql`
- `C:/Users/user/Desktop/Winters/Services/migrations/000014_lostark_character_roster.down.sql`
- 기존 auth/profile route와 `cmd/profile/main.go`의 명시적 mount
- Backend unit/integration tests

현재 Winters checkout에는 다른 작업자의 대규모 미커밋 변경이 있으므로 그 checkout에서 자동
stage/commit하지 않는다. 해당 저장소의 `AGENTS.md`, gotcha, 계획서/비평 gate를 적용한 별도
`codex/lostark-account-backend` worktree에서 Services 변경을 만들고 독립 commit으로 검증한다.

구현:

- 기존 bcrypt/JWT register/login/refresh/logout에 LostArk product route 추가
- dev-only passwordless `/auth/id/*`는 제품 로그인에 사용하지 않음
- guest는 Backend account를 만들지 않는 Client/Server 기존 경로 유지
- list/create/update/delete 6-slot characters
- selected character snapshot과 inventory/equipment transaction
- world/expiry/account/character/revision에 묶인 opaque one-time game-entry ticket 발급과 consume
- DB constraints, password hashing, token digest, bounded HTTP, health/readiness
- migration up/down과 clean database integration test
- Docker daemon이 없는 환경에서도 Go unit test와 schema test는 실행 가능하게 분리

외부 API는 `/auth/lostark/register`, `/auth/lostark/login`,
`GET/POST /lostark/characters`, `POST /lostark/game-entry-tickets`만 첫 slice에 연다.
Game Server 전용 `/internal/lostark/game-entry-tickets/consume`과 character initialize/use-item
semantic command는 internal credential과 operation/revision CAS를 요구한다. roster 응답은 index
혼선을 막기 위해 항상 nullable element 여섯 개인 배열이다.

종료 증거: duplicate login/slot/request ID, wrong password, expired/revoked token, slot 6 reject,
stale revision, transaction rollback, restart persistence.

### G06. Product Login과 Character Slots

수정/추가 대상:

- bounded `Client` Backend HTTP transport와 Auth/Character service
- `Client/Public|Private/CharacterSelectionState.*` 또는 역할을 분리한 account session owner
- `Client/Public|Private/Level_Lobby.*`
- `Client/Public|Private/Level_CharacterSelect.*`
- `Client/Public|Private/MainApp.*`
- `Data/UI/Lobby/Lobby_Layout.json`
- Character Select 제품 layout JSON과 UI C++
- Shared/Game Server의 authenticated profile admission 계약
- 새 파일 `.vcxproj/.filters`

구현:

- Lobby Login/Register/Guest mode
- token과 account session의 process 수명 관리
- 6-slot roster load, empty slot creation, selected slot entry
- nickname/class/level/item level/equipment/inventory read model
- G02 graph wire가 올린 protocol 51→52와 `GUEST / ACCOUNT_TICKET` strict tagged
  `C2S_ENTER_WORLD` (두 payload 변경을 한 version으로 위장하지 않는다)
- Account branch는 class/nickname을 보내지 않고 opaque one-time ticket만 보냄
- Guest branch만 기존 audition class/nickname을 bounded payload로 보냄
- `ServerApp`의 room bind 전 비동기 ticket consume과 normalized
  `SERVER_WORLD_ENTRY_PROFILE` stage
- completion 시 session generation/world/disconnect/revision 재검증 후 spawn commit
- network/backend 실패 시 현재 UI와 이전 valid session 보존
- Guest direct Valtan/Character Select 계약 유지

종료 증거: login/register/guest, service restart 후 roster 복원, six-slot boundary,
wrong credential, timeout/cancel, stale ticket, Server reject와 UI rollback. 제품 화면은 사용자가 직접 판정한다.

### G07. Companion Dialogue와 Raid Guide

구현:

- Backend-only `ICompanionDialogueProvider`
- OpenAI Responses provider와 deterministic fallback provider
- bounded request queue, timeout, rate limit, circuit breaker, response size/filter
- curated raid knowledge version과 immutable room/party context
- completion 때 occurrence generation과 같은 room/party를 재검증
- 성공 답변을 기존 Server chat relay/bubble로 전달

Provider 선택은 환경 설정으로 하며 Client에는 API key와 provider credential이 없다. Claude 등
다른 provider는 같은 interface의 별도 adapter로만 추가한다.

종료 증거: provider success, timeout, rate limit, malformed response, stale room completion drop,
secret scan, fixed tick latency 불변.

### G08. 기능별 비평과 전체 검증

각 G 직후 별도 비평 에이전트가 다음을 실제 diff로 재검증한다.

- Sequencer: 별도 runtime 생성, Client-authored execution, unbounded loop, 거짓 outcome
- AI: fake session/C2S, Effect 기반 hazard, 별도 damage/cooldown, empty-room leak
- Account: Client 권위, plaintext credential/token log, partial transaction, 무한 HTTP join
- UI: ImGui 제품 승격, per-frame JSON/HTTP, stable ID 누락

지적은 코드와 실행으로 재현한 항목만 수정하고 RESULT에 실제 증거를 기록한다.

## 7. Commit과 검증 단위

한 commit에 전체 기능을 섞지 않는다.

1. Encounter graph document/migration
2. Encounter Product/Debug runtime와 protocol
3. Encounter Sequencer UI
4. Companion data/common executor/runtime/trace
5. Character AI Workbench
6. Account service/migrations
7. Product auth/6-slot UI와 Game Server admission
8. Dialogue provider

각 단위는 관련 publisher와 focused harness, `git diff --check`를 먼저 통과시킨다. 마지막에는 다음을 실행한다.

```powershell
python -B Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py
python -B Tools/ValtanPipeline/test_valtan_pattern_master_v2.py
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate

go test ./...
docker compose config

powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -Profile Product
git diff --check
```

Docker daemon이 꺼져 있으면 이를 PASS로 위장하지 않는다. Go/schema/config 검증과 실제 container
integration을 분리해 기록하고, daemon 기동 뒤 migration과 restart persistence를 다시 검증한다.

## 8. 완료 판정

다음이 모두 실제로 연결되어야 완료다.

- Encounter node/edge가 저장용 그림이 아니라 Product와 Debug Server에서 같은 revision으로 실행된다.
- Composition에서 완성한 Pattern을 Sequencer가 stable ID로 참조한다.
- Companion이 실제 `SERVER_PLAYER` snapshot, navigation, skill, damage, party 계약을 사용한다.
- Workbench가 선택 이유와 제외 이유를 Server trace로 설명한다.
- Login/Register/Guest와 6개 slot이 service restart 뒤에도 정확히 복원된다.
- 선택한 account character만 Game Server admission을 통과한다.
- LLM 장애 중에도 raid tick과 deterministic Companion 행동은 유지된다.
- 자동 검증과 사용자 수동 visual smoke를 RESULT에서 분리한다.
