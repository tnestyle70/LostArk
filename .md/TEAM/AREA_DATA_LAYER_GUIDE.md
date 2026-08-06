# LostArk Area 데이터 레이어 가이드

## 1. 결론

Area별 visual, gameplay placement, navigation은 분리 저장된다. 어떤 entity placement가 없으면 해당 entity는 생성되지 않는다. 그러나 모든 레이어가 완전한 plug-in 구조인 것은 아니다. 현재 일반 몬스터, wave/증분 spawn, trigger runtime, Area별 balance override, NPC client presentation은 제품 계약이 없다.

편집·추출 정본은 전부 repository root의 `Data` 아래에 둔다. `Data/Maps/Imported`는
재추출한 catalog와 shard 기준, `Data/Maps/Authoring`은 현재 visual placement,
`Data/Worlds`는 gameplay placement, `Data/Navigation`은 bake/paint/blocker를 소유한다.
`Client/Bin/DataFiles`와 `Server/Bin/DataFiles`는 publisher 생성물이므로 직접 수정하지 않는다.

```text
LevelCatalog scenario
  -> MapCatalog area
     -> visual asset admission / placement
     -> optional deploy asset / placement pair
     -> Gameplay.world.json v2 (actor placement + gated trigger/destroyable authoring)
     -> navigation authoring -> runtime navgrid
     -> stable actor / encounter / balance ID 참조
```

## 2. 현재 Area 실측

| Area | Visual | Gameplay | Navigation | 추가 데이터 |
|---|---|---|---|---|
| `LV_BER_BERNCASTLE` | shard-set, 50,017 placements | class-neutral player spawn 4 | Server 제품 navgrid 없음 | NPC/boss 없음 |
| `LV_LUT_HEARTRB_ED` | 275 assets / 13,115 placements | player spawn 4 + `BOSS_VALTAN` 1 | 62×63, `Data/Navigation/LV_LUT_HEARTRB_ED.*` | deploy pair, BossProfile, ValtanEncounter |
| `LV_DEV_TRAINING_GROUND` | RCArena 10 assets / 18 placements | class-neutral player spawn 4 | uniform 32×32 | NPC/boss/monster/trigger 없음 |
| `LV_LOBBY_CLASSSELECT_SL00` | 55 assets / 803 placements | class-neutral player spawn 4 | Server uniform 42×60 + MapTool source/paint bootstrap | Character Select Arena gameplay |
| `LV_SHS_RCARENA_D` | 302 assets / 7,856 placements | 없음 | 없음 | 원본 Training Map 편집 대상 |

수련장은 Lobby의 `Enter Training`에서 Server 승인을 받은 뒤 `LEVEL::DEVELOPMENT`로 진입한다. Debug/Release network smoke는 map load, player spawn, Q command, Server action 승인, cooldown HUD 반영까지 검사한다.

## 3. 레이어별 생략 규칙

| 레이어 | 없을 때 | 불완전할 때 |
|---|---|---|
| visual map | scenario가 Map domain을 요구하면 load 실패 | catalog/placement 참조 오류는 rollback |
| deploy | `.deployassets`와 `.deployplacements`가 모두 없으면 skip | 둘 중 하나만 있으면 오류 |
| NPC/boss placement | 해당 kind 행이 없으면 spawn하지 않음 | unknown archetype/encounter는 publish 실패 |
| navigation | 현재 Bern처럼 Server nav를 사용하지 않는 world는 생략 가능 | nav를 요구하는 Valtan/Training/Character Select Arena는 누락·손상 시 room 기동 실패 |
| balance definition | 사용하지 않는 actor/skill 정의는 runtime state를 만들지 않음 | placement/action이 없는 stable ID를 참조하면 publish 또는 Server load 실패 |

`Gameplay.world.json` 자체는 Server가 여는 world마다 필요하다. 접속 가능한 world는 최소 하나의 활성 `playerSpawn`이 필요하므로, 빈 placements 문서를 제품 world의 정상값으로 취급하지 않는다.

## 4. MapTool이 지금 편집하는 것

Debug Lobby에서 `Test`를 누르면 기존 `TRAINING_GROUND` 서버 승인을 거친 뒤
`LEVEL::DEVELOPMENT`를 socket, player, replication이 없는 Map Editor Workspace shell로
연다. 진입 후 F1은 공통 Developer Tools만 토글하며, 그 안의 `Map Tool`에서 다음 네 Area
중 하나를 선택한다. F1이나 Map Tool 버튼 자체는 Level을 전환하지 않는다.

| 선택 | visual source | navigation | gameplay |
|---|---|---|---|
| Character Select | `LV_LOBBY_CLASSSELECT_SL00` | source/paint, Nav Bounds bootstrap 허용 | exact `gameplayDocument` 필수 |
| Bern | `LV_BER_BERNCASTLE` | disabled | exact `gameplayDocument` 필수 |
| Valtan | `LV_LUT_HEARTRB_ED` | source/paint/blockers 필수 | exact `gameplayDocument` 필수 |
| Training Map | `LV_SHS_RCARENA_D` | disabled | disabled |

Training Map은 원본 302 assets / 7,856 placements인 `LV_SHS_RCARENA_D`다. Release 제품
Test가 사용하는 10 assets / 18 placements `LV_DEV_TRAINING_GROUND`와 다른 데이터다.

Area selector는 `Data/Maps/MapCatalog.json`의 exact `sourceCatalog`와
`sourcePlacements`를 읽는다. `Client/Bin/DataFiles/Map` fallback은 없다. Area 전환 전에
visual/gameplay/navigation dirty를 검사하고 `Save and Continue / Discard and Continue /
Cancel` 중 하나를 요구한다. 전환 stage가 실패하면 기존 Area 객체와 문서를 유지한다.

MapTool의 저장 대상은 Data 원본뿐이다.

- visual: active descriptor의 `Data/Maps/Authoring/...mapplacements`
- gameplay: Character Select/Bern/Valtan의 exact `Data/Worlds/.../Gameplay.world.json`
- navigation: 정책이 허용한 `Data/Navigation/*.navsource/.navpaint/.navblockers`

Navigation `Walkability` 브러시는 높이가 해석된 셀에 대해 `Block`, `Force Walkable`,
`Reset` 세 명령을 제공한다. `Block`과 `Force Walkable`은 bake 결과보다 우선하는 수동
override이며 `Reset`은 해당 셀을 bake 결과 상속으로 되돌린다. `.navpaint` version 2는
각 행을 `x z BLOCKED|WALKABLE`로 저장하고, 기존 version 1의 `x z` 행은 `BLOCKED`로
호환 로드한다. 높이가 없는 `NO_SURFACE` 셀은 강제로 이동 가능하게 만들 수 없다.

MapTool은 Client `.navgrid`를 export하거나 제품 Navigation runtime blocker를 등록하지 않는다.
Visual runtime은 `Publish-MapAuthoring.ps1`, world bootstrap은
`Publish-WorldGameplay.ps1`, Server navigation은 `Publish-ServerNavigation.ps1`만 교체한다.
`ACTIVE.maparea`도 selector 변경 때 자동 저장하지 않는다.

Valtan DeployProp은 이번 workspace 전환에서 명시적으로 제외했다. deploy source/catalog,
transactional stage와 save 계약을 한 번에 닫기 전까지 editor에서 load/save하지 않는다.
Bern 50,017 placements는 현재 동기 stage이므로 Area 선택 직후 창이 오래 응답하지 않을 수
있다. 중복 선택은 하지 말고 status가 commit될 때까지 기다린다.

`NpcCatalog.json`은 현재 `NPC_BEDA` 한 archetype을 지원한다. Bern의 enabled NPC placement는
Server world entity로 생성되고 `CClientReplication`이 catalog → on-demand model prototype → `CNpc`
경로로 표현한다. 다른 NPC model은 catalog row와 검증을 추가하기 전까지 fail-closed다.

일반 몬스터, spawn wave, 증분 생산은 저장 schema와 Server consumer가 없다. `triggerBox`는
world formatVersion 4의 strict parse/save 구조와 Debug Development MapTool 배치·선택·크기·목적지
편집·저장/재로드 UI를 제공한다. typed action이 없는 draft는 `enabled: false`, `events: []`로만 저장한다.
제품 publisher/runtime가 admission하는 action은 정확히 하나의 `movePlayer` 또는 `changeLevel`이다. movePlayer는
`targetPosition[3]`, `durationSeconds(0.05..10)`, `arcHeight(0..1000)`를 소유하며 Server가 yaw가 적용된
OBB 진입과 포물선 이동을 판정하고 player snapshot으로 복제한다. `triggerOnce=true`는 room 전체에서
첫 성공 진입 한 번, `false`는 player가 완전히 나간 뒤 다시 들어오는 edge마다 재실행한다. 저작용 wire
box는 판정 권위가 아니다. `destroyable`, `setCondition`, `setDestroyableState`, 파티 대기, 컷신,
몬스터 생성 event는 계속 publisher가 fail-closed로 거부한다. 수업용 `CMonster`를 재사용하거나
`npc`/`boss`로 위장하지 않는다.

`collisionBox`는 transform, half extents, enabled만 저장한다. MapTool의 파란 wire box는 authoring
표시이고 Server가 player OBB 크기를 포함한 swept OBB 판정으로 일반 보행을 차단한다. Client
`CCharacter`의 같은 크기 `CCollider::OBB`는 presentation/debug이며 이동 정답을 만들지 않는다.

`changeLevel`은 Bern과 Valtan Arena 사이에서만 허용한다. Server가 OBB enter edge에서 source room leave와
target room enter를 처리하고 Client는 새 `S2C_ENTER_ACCEPTED`를 소비한 뒤에만 level transition을 요청한다.

## 5. Balance 소유권

Balance는 현재 Area별 파일이 아니라 전역 stable definition이다.

| 정본 | 소유 값 |
|---|---|
| `Data/Balance/PlayerProfiles.json` | class HP/resource/move speed |
| `Data/Balance/PlayerSkills.json` | skill slot/timing/cost/range/damage ID |
| `Data/Balance/DamageProfiles.json` | Server damage |
| `Data/Balance/BossProfiles.json` | boss HP/range/speed/phase threshold |
| `Data/Encounters/<Boss>/...json` | stage/encounter state와 pattern timeline |

Area는 수치를 복사하지 않고 stable actor/encounter ID를 참조한다. Debug F1 `Balance Tool`에서 JSON
저작 → provenance 동기화 → Validate/Publish를 수행하고 Server를 재시작해 적용한다. 레벨별 override가
필요하면 별도 schema와 우선순위/rollback 계약부터 정해야 한다.

## 6. 다섯 캐릭터 roster 상태

Lobby는 Lance Master, Gunslinger, Slayer, Artist, DimensionMaster 다섯 slot을 모두 보여주고 선택할 수 있다. 실제 world 진입은 `Is_Supported_Playable_Character_Class`가 승인한 class만 가능하다.

| Class | Lobby 선택 | Resource pack | Client Loader/Spec | Server profile | World 진입 |
|---|---:|---:|---:|---:|---:|
| Lance Master | 가능 | 있음 | 완료 | 완료 | 가능 |
| Gunslinger | 가능 | 있음 | 완료 | training baseline | 가능 |
| Slayer | 가능 | 있음 | 완료 | training baseline | 가능 |
| Artist | 가능 | 있음 | 완료 | training baseline | 가능 |
| DimensionMaster | 가능 | `.3`에는 없음, 로컬만 | 완료 | training baseline | 가능(로컬 payload 필요) |

앞의 네 class는 body·equipment·weapon `.wmodel`과 texture를 6-root resource pack에 admission한다. DimensionMaster는 combined body `Character/DimensionMaster/DimensionMaster_Character.wmodel`과 `Character/WP_WSWP_M_06`의 L/S/P/E 네 정적 기본 무기 파츠를 로컬 payload로 사용한다. 다섯 class 모두 `CharacterCatalog`, `CCharacterCatalog`, Loader prototype, Server `PlayerProfiles`, class parser, spawn/remote presentation까지 연결한다. 현재 비-Lance class의 profile 수치는 class별 스킬 계약 전까지 명시적인 training baseline이며, LanceMaster로 조용히 대체하는 identity fallback은 금지한다.

Area 진입 시 다섯 class binary를 모두 선로드하지 않는다. Lobby 선택 class만 먼저 준비하고 다른 class는 실제 remote spawn에서 최초 한 번만 같은 `CPlayableCharacterAssetService`로 admission한다. 이 규칙은 mixed-class 표현을 유지하면서 불필요한 Level 로딩 증가를 막는 고정 경계다.

## 7. 새 Area 추가 체크리스트

1. `LevelCatalog.json` stable scenario와 기존 Engine Level 매핑
2. `MapCatalog.json` Area 및 실제 사용하는 visual admission만 등록
3. authoring placement → 원자 publish → runtime placement
4. class-neutral player spawn과 필요한 NPC/boss placement
5. navigation authoring, spawn/boss cell·height 검증
6. 필요한 actor/encounter/balance stable ID 연결
7. Loader/registry/publisher/ProjectAudit 등록
8. Debug/Release scenario smoke와 process cleanup
