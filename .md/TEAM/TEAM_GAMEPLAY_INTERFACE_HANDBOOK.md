# LostArk 팀 게임플레이 인터페이스 사용서

작성일: 2026-08-03
정본 브랜치: `codex/baren-player-replication`

## 1. 한 줄 계약

제품 게임플레이는 항상 다음 한 방향으로 흐른다.

```text
Input/UI intent
-> Client command service 또는 IPlayerCommandSink
-> Shared C2S message
-> Server GameRoom 30 Hz authority
-> Shared S2C snapshot
-> ClientReplication
-> Character/Valtan presentation 또는 CombatHUDViewModel
```

Client는 입력을 빠르게 제출하지만 위치, damage, cooldown, HP, boss phase를 확정하지 않는다. Server가 확정한 snapshot만 제품 화면의 정답이다.

Lobby는 `Test`, `Character Select`, `Valtan`, `Bern` 네 명령만 제공한다. 최초 Character Select는 socket 없는 3D Preview Level이다. Character Select 창의 `Server Play`는 선택 class와 tokenized TEST만 Lobby에 제출하고, Lobby가 `WORLD_ID::CHARACTER_SELECT_ARENA` 승인 payload를 검증한 뒤 기존 socket을 one-shot handoff하여 같은 visual map을 Server gameplay로 다시 연다. Server Arena의 `Preview`는 현재 socket과 replication을 정리하고 tokenized CHARACTER_SELECT를 Lobby에 제출하여 socket 없는 Preview로 재진입한다. 재진입 Level은 직접 connect/send하지 않고 `CClientReplication`, `CNetworkPlayerCommandSink`, `CPlayerController`로 HUD·우클릭 이동·quick-slot 스킬을 Server snapshot에 연결한다. 연결 실패·거부·5초 timeout은 Lobby에 남고 disconnect는 Lobby로 복귀하며 자동 local gameplay fallback은 없다. `Summon Valtan (Lazy)`는 `IWorldEntityCommandSink`를 통해 stable placement ID만 제출하고 Server가 world template/navigation/profile을 검증한 결과를 broadcast한다. Bern/Valtan map 진입도 Lobby Server 승인이 필수다.

같은 PC 테스트는 `Framework.slnLaunch`의 `Server + Client` profile과 기본 `127.0.0.1:7777` 계약을 사용한다. LAN에서는 Server를 `--bind-address 0.0.0.0`으로 실행하고 각 Client process에 `LOSTARK_SERVER_HOST=<host 사설 IPv4>`를 준다. 개인 IP는 Git에 저장하지 않으며 endpoint 입력 UI와 자동 서버 탐색은 현재 Lobby 범위가 아니다.

## 2. 팀원이 먼저 읽을 파일

| 담당 | 시작 파일 | 데이터 정본 |
|---|---|---|
| UI | `Client/Public/CombatHUDViewModel.h`, `LobbyCommandService.h`, `LevelTransitionService.h` | `Data/UI`, `Data/Balance` |
| Player/Input | `Client/Public/PlayerController.h`, `PlayerCommandSink.h` | `Data/Balance/PlayerSkills.json` |
| Character/Animation | `Client/Public/Character.h`, `CharacterSpec.h`, `AnimationTargetService.h` | `Data/Actors`, `Data/Animation` |
| Server/Player | `Server/Public/GameRoom.h`, `PlayerSkillSystem.h`, `ServerNavigation.h` | `Data/Balance`, `Data/Navigation`, `Data/Worlds` |
| Boss | `Server/Public/ValtanBrain.h`, `ServerWorldEntity.h` | `Data/Balance/BossProfiles.json`, `Data/Encounters` |
| Map/Encounter | `Client/Public/MapTool.h`, `WorldGameplayDocument.h` | `Data/Maps/Authoring`, `Data/Worlds`, `Data/Navigation` |
| 통합/검증 | `AGENTS.md`, `CLAUDE.md` | `Tools/Build`, `Tools/ProjectAudit` |

### 2.1 기능 담당자의 수직 슬라이스 책임

<!-- team-contract: vertical-slice-feature-owner; roles-are-not-file-permissions -->

기능 담당자는 수직 슬라이스의 구현 책임자다. 위 표는 어디서 시작하고 어떤 정본을 읽는지
알려주는 표이지, 해당 행 밖의 파일을 수정하지 못하게 하는 권한표가 아니다.

```text
요청 기능
-> authoring/balance JSON과 stable ID
-> 필요한 Shared command/snapshot
-> Server validation·authority·room state
-> Client replication·presentation·ViewModel
-> 실제 UI/Character/Map 소비자
-> protocol/server/client harness와 실패 경로
```

- Player/Input 기능이 Server 판정을 요구하면 그 작업자가 Shared message와 `GameRoom`/skill
  system까지 함께 구현한다.
- Character/Animation 기능이 새 action을 요구하면 balance/action ID, Server 승인 상태,
  snapshot과 presentation mapping을 함께 닫는다.
- UI 기능에 새 runtime 값이 필요하면 UI에서 socket을 읽지 않고 Server snapshot과
  `CCombatHUDViewModel`까지 확장한다.
- Map/Encounter 기능이 spawn이나 encounter truth를 바꾸면 world authoring, publisher,
  Server bootstrap/runtime과 Client presentation을 함께 검증한다.
- 다른 담당자의 미커밋 줄을 덮어쓰거나 public interface를 우회하는 것은 계속 금지한다.
  필요한 교차 영역 수정 자체는 금지하지 않는다.

## 3. 플레이어 입력과 스킬

현재 제품 입력은 다음과 같다.

| 입력 | stable ID | 결과 |
|---|---:|---|
| 우클릭 | move sequence | `C2S_MOVE`로 목표 X/Z 제출 |
| 좌클릭 | `34010` | 평타 콤보. 누르고 있으면 100ms마다 재전송한다 |
| Q | `34120` | 연환섬 사용 의도 제출 |
| W | `34080` | 일섬각 사용 의도 제출 |
| E | `34070` | 회선창 사용 의도 제출 |
| R | `34150` | 맹룡열파 사용 의도 제출 |
| A | `34110` | 반월섬 사용 의도 제출 |
| S | `34090` | 철량추 사용 의도 제출 |
| T | `34640` | 맹룡난무 사용 의도 제출 |
| V | `34600` | 은하유성탄 사용 의도 제출 |
| Alt+V | `34620` | 은하비섬창 사용 의도 제출 |

`CPlayerController`는 edge input, quick slot, sequence, aim만 만든다. `(class, inputSlot) -> skill ID`는 `CPlayerSkillCatalog`가 `Data/Balance/PlayerSkills.json`에서 해석한다. `IPlayerCommandSink`가 전송 구현을 숨기므로 Controller에서 `CNetworkManager`를 include하지 않는다. Character를 직접 이동하거나 `Play_Skill`을 호출하지 않는다.

스킬 서버 흐름은 다음과 같다.

```text
CPlayerController::Update
-> IPlayerCommandSink::Request_UseSkill
-> CNetworkManager::Send_UseSkill
-> C2S_USE_SKILL { clientSequence, skillId, aimX, aimZ }
-> CServerApp::On_SessionFrame
-> CGameRoom::Handle_UseSkill
-> CPlayerSkillSystem::Try_Start / Update
-> S2C_WORLD_SNAPSHOT
```

Client payload에는 PlayerId와 NetEntityId가 없다. Server가 SessionId로 player를 찾고 sequence, class, 생존, 현재 action, cooldown, resource를 검사한다. 승인하면 이동 목표를 취소하고 action tick, skill ID, cooldown end tick을 Server 상태에 기록한다.

## 4. 이동, Navigation, collider 경계

우클릭 피킹은 입력 목표를 얻기 위한 Client 표현 계층이다. 제품 위치의 정답은 Server Navigation이다.

- 일반 이동: Server가 navgrid에서 시작/목표를 projection하고 8방향 A* path를 만든다.
- 높이: 각 Server nav point의 Y를 사용한다.
- 스킬 이동: `movementDistance`를 action duration에 분배하고 매 tick 다음 위치를 navgrid로 projection한다.
- 보스 이동: `CValtanBrain`이 같은 Server Navigation으로 target까지 path를 계산한다.
- Client `CNavigation`, mesh picking, animation root motion은 Server 위치를 확정하지 않는다.

현재 서버 충돌 계약은 walkable nav cell 경계다. 동적 capsule-vs-capsule, projectile, knockback obstacle collision은 아직 public 계약이 아니므로 Character collider에서 임의로 서버 판정을 대신하지 않는다. 추가할 때는 Server collision owner, shape ID, broad/narrow phase, snapshot correction, harness를 한 변경 단위로 닫는다.

## 5. Character와 Animation

Character는 Server action을 시각화한다.

```text
CClientReplication::Apply_WorldSnapshot
-> CCharacter::Apply_NetworkState
-> CCharacter::Apply_NetworkAction
-> approved skill: Play_Skill
-> locomotion: RUN / IDLE
```

`Set_Locomotion()`만으로 스킬 계약 전체가 닫히는 것은 아니다. 이 함수는 이동 중 RUN, 정지 시 IDLE을 고르는 표현 함수다. 스킬 중에는 locomotion animation 전환을 보류한다. 실제 skill 시작은 snapshot의 `action`, `skillId`, `actionStartTick`이 바뀌었을 때만 `Apply_NetworkAction()`이 수행한다.

Character/Animation 담당자는 clip mapping, part, notify, blend와 재생 결과를 소유한다. damage, cooldown, resource, hit 여부, 위치 정답은 수정하지 않는다. `Logic_*`에서 DirectInput, socket, packet을 읽거나 `Play_Skill`을 직접 호출하지 않는다.

현재 roster는 Lance Master, Gunslinger, Slayer, Artist, DimensionMaster 다섯 class다. 다섯 class의 quick slot ACTIVE 스킬과 LMB COMBO 평타는 모두 Server 승인과 snapshot presentation을 사용한다. `PlayerSkills.json`이 `(characterClass, inputSlot) -> skillId`, kind, timing, comboStages를 소유하고, `Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json`이 같은 skillId에 연결할 실제 model clip 순서를 소유한다. ACTIVE는 하나 이상의 순차 clip을 재생하고 마지막 pose를 Server `NONE`까지 유지한다. COMBO는 Server `iComboStage`가 지정한 BA 단계로 직접 이동한다. 누락되거나 잘못된 presentation 문서는 spawn/replication을 중단하지 않고 해당 action의 표현만 격리한다.

Animation Tool은 Scene Character의 현재 model에 실제 존재하는 clip만 저장할 수 있다. 작업자는 key/skill row에서 ACTIVE의 순차 clip 또는 COMBO의 BA1/BA2/BA3/BA4 clip을 지정하고 atomic Save한다. `inputSlot`, `skillId`, `skillKind`, timing, damage와 combo 단계 수는 Tool에서 바꾸지 않는다. `.skilltiming/.clipmap/.animnotify/.clipseq` 및 `Data/Animation/Reference`는 read-only 참고 자료다.

## 6. UI와 밸런스 데이터

UI가 바로 사용할 읽기 경계는 `CCombatHUDViewModel`이다.

`Get_Player()`가 제공하는 값:

- 최신 server tick
- current/max HP
- current/max resource
- server action
- skill ID, input slot, 표시 이름, action ID
- cooldown duration tick, cooldown end tick, 표시 damage

`Get_Boss()`가 제공하는 값:

- archetype ID와 표시 이름
- current/max HP
- phase
- server action과 action ID

`Get_DamageEvents()`는 최근 128개 Server `DAMAGE_EVENT`를 server tick과 함께 보관한다. 실제 적용
damage, target NetEntityId, world anchor, incoming/outgoing을 제공하며 UI가 HP 차이로 damage를
재계산하지 않는다. F1 Balance Tool은 이 경계로 최근 16개 event를 표시한다.

쿨타임 남은 tick은 `max(0, cooldownEndTick - serverTick)`이며 UI가 별도 timer를 정답으로 만들지 않는다. 표시 damage는 데이터 정의를 읽은 값이고 실제 피해 적용은 Server만 한다.

밸런스 정본:

| 파일 | 수정하는 값 | 주 소비자 |
|---|---|---|
| `Data/Balance/PlayerProfiles.json` | class별 max HP/resource/move speed | Server spawn, HUD snapshot |
| `Data/Balance/PlayerSkills.json` | slot, 이름, `skillKind`, cooldown, action/hit time, cost, 이동 거리, range, damage 참조, `effectId`, `comboStages` | Server skill, UI definition, Effect presentation |
| `Data/Balance/DamageProfiles.json` | attack power에 곱하는 damage rate percent | Server 판정, UI 예상 표시 |
| `Data/Balance/BossProfiles.json` | boss HP, engage range, speed, phase threshold | Server boss, UI 이름 |
| `Data/Encounters/Valtan/ValtanEncounter.json` | state/action/pattern timing/range/damage 참조 | Server Valtan brain |

UI 담당자는 JSON을 매 프레임 읽지 않는다. `CCombatHUDViewModel::Initialize_Definitions()`가 정의를 준비하고 `CClientReplication`이 snapshot마다 runtime 상태를 적용한다. UI 코드에서 packet, socket, Character, boss GameObject를 직접 조회하지 않는다.

Debug F1 `Balance Tool`은 다섯 class와 발탄을 선택해 stats/movement/skill/combo/pattern을 편집하고,
field provenance와 Server snapshot/damage event를 같은 화면에서 검증한다. Save는 authoring JSON을
staging한 뒤 변경 field를 `PROJECT_TUNED`로 동기화하고 publisher Validate를 수행한다. Publish 후
Server 재시작이 필요하며 runtime Hot Reload 버튼은 없다. 세부 작업법은
`BALANCE_TOOL_OWNER_HANDOFF.md`가 정본이다.

### 6.1 ImGui authoring에서 제품 이미지 UI로 전환

ImGui는 최종 제품 UI가 아니라 layout authoring과 debug command를 위한 작업면이다. UI 담당자가
ImGui로 만든 창이나 버튼을 스크린샷으로 떠서 교체하는 것이 아니다. `CHUDLayoutTool`에서 실제
이미지 asset을 slot/layer에 연결하고, 저장된 JSON을 제품 런타임이 읽어 image widget을 만드는
방식으로 전환한다.

현재 정본과 구현 상태:

| 항목 | 현재 상태 |
|---|---|
| Combat HUD layout | `Data/UI/HUD/HUD_Layout.json`, asset domain `UI/HUD/` |
| Screen UI layout | `Data/UI/ScreenUI/ScreenUI.json`, asset domain `UI/ScreenUI/` |
| ImGui authoring | asset palette, thumbnail, drag/drop, rect/rotation, layer order, hover preview, save/load 구현 |
| runtime state | `CCombatHUDViewModel`과 임시 runtime HUD overlay 구현 |
| 최종 image widget 생성 | layout JSON을 `CUIObject` 계열로 만드는 factory는 미구현 |
| 제품 UI picking | screen-space input router와 command binding schema는 미구현 |

작성에서 실행까지의 목표 흐름은 하나다.

```text
Resources/UI image asset
-> CHUDLayoutTool (ImGui authoring)
-> Data/UI/*.json (stable slot.id + geometry + draw order + image asset ID)
-> runtime layout loader (parse -> validate -> stage -> commit)
-> CUIObject image widget tree
-> screen-space UI hit test
-> stable UI command
-> CLobbyCommandService / CLevelTransitionService / IPlayerCommandSink
-> Server snapshot when authority is required
-> CCombatHUDViewModel
-> widget presentation
```

저장과 asset 규칙:

- reference resolution은 현재 1280×720이며 viewport scale/letterbox 보정 뒤 같은 좌표계로
  draw와 hit test를 수행한다.
- `slot.id`가 stable widget identity다. pointer, vector index, ImGui label, 보이는 문자열을
  저장 ID로 사용하지 않는다.
- image는 `Client/Bin/Resources/UI/<Domain>/...`에 두고 JSON에는 `UI/...` 상대 asset ID만
  저장한다. 절대 경로, drive path, `..`, `Resources/LostArk` wrapper는 거부한다.
- render order와 picking order는 같은 계약을 사용한다. 뒤에서 앞으로 그리고, 겹친 widget은
  앞에서 뒤로 검사해 최상위 하나만 pointer를 소비한다.
- display-only HUD는 기본적으로 hit test하지 않는다. interactive widget은 향후 schema version
  갱신과 함께 `enabled/visible`, hit-test shape, stable command ID, pointer capture 정책을
  명시해야 한다. 현재 format version 1에는 runtime command binding이 없으므로 임의 문자열
  필드를 끼워 넣지 않는다.
- normal/hover/pressed/disabled 표현은 widget 상태로 선택하며 hover image 존재 여부가 command
  권한을 뜻하지 않는다. alpha-mask picking은 성능과 판정 harness가 있는 별도 옵션으로만
  추가하고 기본은 transformed rectangle hit test다.

UI picking은 3D 월드 ray를 쏘는 `CPicking`과 다른 기능이다. mouse viewport 좌표를 reference
resolution으로 변환하고 회전까지 반영한 slot rect를 검사한다. visible/enabled 상태가 아니거나
크기가 0인 widget은 hit 대상이 아니다. UI가 click을 소비한 프레임에는
`CGameInstance::SetInputBlocked()` 또는 동등한 단일 input arbitration 경계로 ground move와
gameplay click을 보내지 않는다.

interaction을 구현할 때 JSON의 stable command ID는 함수 이름이나 packet opcode가 아니다.
런타임 registry가 이를 typed UI command로 해석하고 Lobby 선택은 `CLobbyCommandService`, scene
이동은 `CLevelTransitionService`, gameplay action은 `IPlayerCommandSink`로 제출한다. UI가
packet을 조립하거나 socket을 호출하고, click callback에서 `Change_Level`이나 Character 상태를
직접 변경하는 것은 금지한다.

이 수직 슬라이스의 완료 검증에는 duplicate/unknown slot ID, unsafe/missing asset, 잘못된 rect와
rotation, unknown command, 겹침 시 topmost 선택, resolution/letterbox 보정, hidden/disabled 제외,
중간 load 실패 시 기존 UI 유지, UI 소비 click의 gameplay 차단이 포함되어야 한다. runtime
factory와 router가 생기기 전까지는 authoring tool의 save 성공만으로 제품 UI 전환 완료를
선언하지 않는다.

Git 관리 대상 데이터는 Visual Studio Client 프로젝트의 `96.DataFiles` 필터에서 원본을 바로 연다. 이 항목들은 `None` 링크이며 복사본이나 runtime 배포본이 아니다. 수치 튜닝 절차와 무중단 reload를 아직 활성화하지 않은 이유는 `BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`를 따른다.

## 7. Valtan Boss

제품 발탄의 transform, target, action, phase, HP, damage는 Server authority다.

```text
CGameRoom::Tick
-> CValtanBrain::Update
-> nearest living player acquire
-> IDLE / CHASE / PATTERN_WINDUP / PATTERN_ACTIVE / PATTERN_RECOVERY / DEAD
-> damage exactly once in active window
-> S2C_WORLD_SNAPSHOT
-> CValtan presentation + CCombatHUDViewModel
```

`BossProfiles.json`은 boss 기본 수치, `ValtanEncounter.json`은 pattern timeline, `DamageProfiles.json`은 피해량을 소유한다. Client `CValtan`의 로컬 AI는 Development preview 외 제품 정답이 아니다.

플레이어 profile의 defense는 발탄 incoming damage에 실제로 사용된다. 원작 Server 공식이 client
payload에 없으므로 `raw * 100 / (100 + defense)`는 `PROJECT_TUNED` 중앙 계약이며
`CGameplayCatalog::Apply_Defense` 한 곳에서만 계산한다. boss defense/outgoing 감산은 아직 없다.

수업용 `CMonster`와 `astar/Monster`는 제거 대상 레거시다. Monster runtime/catalog/schema, placeholder enum을 다시 만들지 않는다. 잡몹 요구가 승인되면 실제 archetype, spawn, brain, replication, harness를 별도 수직 슬라이스로 설계한다.

## 8. MapTool과 gameplay 저장

정적 visual 배치와 gameplay 배치는 분리한다.

- visual import definition: `Data/Maps/Imported/<AreaId>/`
- visual authoring: `Data/Maps/Authoring/<AreaId>/`
- gameplay authoring: `Data/Worlds/<AreaId>/Gameplay.world.json`
- navigation authoring: `Data/Navigation/<AreaId>.navsource/.navpaint/.navblockers` 또는 uniform `<AreaId>.navgrid.json`
- Client map/navigation 생성물: `Client/Bin/DataFiles/Map`, `Client/Bin/DataFiles/Navigation`
- Server world 생성물: `Server/Bin/DataFiles/World/*.worldbootstrap`

`Data`만 사람이 편집하거나 재추출 결과를 반영하는 정본이다. Client/Server `Bin/DataFiles`는
publisher 출력이며 source로 다시 읽어 authoring을 갱신하지 않는다. Visual Studio의
`96.DataFiles` 필터도 `Data` 원본만 연결한다.

Debug Map Editor 사용 절차는 다음과 같다.

1. Debug Lobby에서 `Test`를 누르고 기존 Server 승인을 기다린다.
2. `LEVEL::DEVELOPMENT`의 격리된 editor shell 로드가 끝날 때까지 기다린다.
3. F1로 공통 Developer Tools를 열고 `Map Tool`을 선택한다.
4. Area selector에서 Character Select, Bern, Valtan, Training Map 중 하나를 고른다.
5. visual placement 또는 정책이 허용한 gameplay/navigation만 편집한다.
6. Area 전환·reload·Lobby 복귀 전에 dirty gate에서 Save/Discard/Cancel을 결정한다.
7. 저장 뒤 필요한 publisher를 별도로 Validate/Publish한다.

제품 `CLevelRegistry`는 제품 Level의 `MAP_LOAD_SCOPE`만 소유한다. Map Editor의 대상과
authoring policy는 별도 descriptor이며 제품 Level에서 MapTool authoring을 활성화하지 않는다.
Debug Lobby Test의 승인 소비자가 editor intent를 세운다. F1/Map Tool은 전환 권한이 없고
도구 표시와 선택만 담당한다. Release Test에는 Debug editor intent가 없으므로 기존 training
map, 서버 승인 character, replication, controller 경로를 유지한다.

Editor Area 정책은 `AREA_DATA_LAYER_GUIDE.md` 4절이 정본이다. 특히 Character Select와
원본 Training Map은 gameplay 문서를 만들지 않고, Bern/Training Map은 navigation 문서를
추측 생성하지 않는다. Valtan DeployProp 편집은 현재 제외되어 있다.

gameplay authoring은 formatVersion 2다. 제품 publisher/runtime는 현재 `playerSpawn`, `npc`, `boss`와
단일 `movePlayer` action을 가진 `triggerBox`를 admission한다. placement에는 stable placement ID, kind,
encounter ID, position, yaw, enabled를 저장한다.
NPC/boss는 stable archetype ID를 소유하지만 `playerSpawn`의 `archetypeId`는 `null`이며 실제 class는
session/player selection이 소유한다. NetEntityId, pointer, Prototype tag, vector index, runtime HP/phase를
저장하지 않는다.

v2 `CWorldGameplayDocument`에서 `triggerBox`는 half extents, once 정책, typed event를 소유한다.
현재 제품 event는 `movePlayer` 하나이며 `targetPosition`, `durationSeconds`, `arcHeight`를 저장한다.
Server는 yaw OBB enter edge를 판정하고 일반 이동/스킬을 중단한 뒤 30 Hz 직선·포물선 이동을 확정한다.
Shared `TRIGGER_MOVE` action과 player transform snapshot이 Client 표현의 유일한 입력이다. Client는 별도
jump clip 계약이 생기기 전까지 RUN locomotion으로 이동을 표현한다. `destroyable`과 나머지 trigger event는
여전히 publisher가 fail-closed로 거부하며, parser 존재만으로 제품 runtime 지원 완료라고 판단하지 않는다.

Debug Development MapTool은 action이 아직 없는 `triggerBox`를 disabled draft로 배치하고 position,
yaw, half extents, once 정책을 편집한다. 선택한 box에 `movePlayer`를 추가하고 목적지를 맵에서 pick한 뒤
duration/arc를 정해야 enabled로 저장할 수 있다. wire box는 저작용 presentation이며 overlap/action 권위는
Server에만 있다. Save 뒤 publisher와 Server 재시작 전에는 제품 월드에 적용된 것이 아니다.

Map/Encounter 담당자가 좌표를 수정하면 navigation publish가 활성 playerSpawn/boss 좌표의 walkable cell과 높이 오차를 검사한다. 생성된 Server bootstrap/navgrid를 직접 편집하지 않는다.

Area별 optional layer, 현재 Bern/Valtan/Training 데이터 보유 현황, Monster/wave/trigger와 NPC presentation의 미지원 경계는 `AREA_DATA_LAYER_GUIDE.md`를 정본으로 사용한다.

## 9. 데이터 변경 절차

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate
```

세 publisher는 parse → validate → stage → commit을 따른다. unknown field, schema/version 오류, 중복 stable ID, 잘못된 참조, non-finite 위치, navigation 밖 spawn을 정상값으로 보정해 숨기지 않고 실패시킨다.

새 스킬을 추가할 때는 다음을 함께 변경한다.

1. `PlayerSkills.json`과 참조 `DamageProfiles.json`. `effectId`는 Client 연출 참조이며
   stable ID여야 하고 Server bootstrap에는 싣지 않는다. 빈 문자열은 저작된 이펙트가
   아직 없다는 뜻이다.
2. Character presentation의 stable action/skill mapping
3. 필요한 Shared message/snapshot 확장
4. Server validation/action/damage 처리
5. `CCombatHUDViewModel` 소비 확인
6. protocol harness와 `Server.exe --contract-test`

## 10. Asset과 Git

`Client/Bin/Resources`는 `Fonts, Character, Deploy, Effect, Map, UI` 여섯 root만 허용한다. asset ID는 Resources 상대 경로이며 절대 경로, drive-qualified 경로, `..` 탈출을 금지한다.

대용량 runtime payload는 Git에 올리지 않는다. Drive의 immutable pack으로 배포하고 Git에는 다음만 올린다.

- `Data/AssetPacks.lock.json`
- `Data/AssetManifests/<immutable-pack>.manifest.json`
- source/catalog/authoring JSON과 운영 문서

팩을 바꿀 때는 `Tools/AssetPipeline/README.md`의 Snapshot → Verify → Publish → Hydrate 순서를 사용한다. 개별 파일 덮어쓰기나 팀원별 절대 경로 하드코딩을 금지한다.

팀원이 branch를 pull한 뒤 최초 실행하는 순서는 다음과 같다.

```text
git lfs pull
→ lock과 같은 version의 Resource ZIP SHA-256 확인
→ 외부 pack root에 압축 해제
→ Manage-ResourcePack Hydrate
→ Manage-ResourcePack Verify
→ Debug 전체 회귀
→ 담당 public interface에서 작업 시작
```

Git commit과 Resource ZIP은 한 쌍의 인계 단위다. commit은 `Data/AssetPacks.lock.json`으로 필요한 pack version을 선언하고, Drive에는 정확히 그 immutable version ZIP만 둔다.

기능은 `main`이 아닌 별도 branch/PR로 전달한다. 코드, 소비 데이터, project/filter 등록, harness, RESULT를 같은 검증 단위로 묶는다. build output, `EngineSDK`, `.vs`, `.codex_tmp`, `_work`, `imgui.ini`, Resources payload를 stage하지 않는다.

## 11. 완료 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -DeepAssetHash
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -DeepAssetHash
```

자동화 순서는 Engine → UpdateLib → Shared/Protocol Harness → Server build/contract test → Client build → balance/world/navigation validate → ProjectAudit이다. 실제 Level 흐름은 `Framework.slnLaunch`로 Server와 Client를 함께 실행해 검증한다.

최소 성공 증거:

- Protocol Harness `failures : 0`
- Server gameplay contract `failures : 0`
- Debug/Release Client와 Server 빌드 성공
- 실제 Server+Client에서 Lobby → Character Select → Lobby → Test/Bern/Valtan 진입 확인
- 연결 실패 시 Lobby 유지와 제품 Level disconnect 후 Lobby 복귀 확인
- ProjectAudit와 필요한 deep asset hash 성공
- `git diff --check` 성공
- 잔류 Client/Server/7777 listener 없음

## 12. 현재 완료와 다음 경계

완료:

- 서버 권위 우클릭 이동과 Navigation path
- 다섯 class 전체 quick slot과 LMB COMBO의 command, server approval, action/damage/cooldown/resource
- snapshot 기반 Character skill/locomotion 표현
- Animation Tool의 data-driven key/skill → ordered clip/BA stage authoring, atomic Save, safe action-boundary reload
- HUD용 player/boss runtime ViewModel
- Valtan 추적, pattern, damage, phase, death
- world gameplay와 navigation 배치 정합성 검사
- `dev.training.ground` 최소 Area, class-neutral player spawn, RCArena 10종 admission, 서버 navigation
- Lobby의 Lance Master/Gunslinger/Slayer/Artist/DimensionMaster 다섯 선택 slot, Character Select visual map, Enter-to-Test token handoff, 다섯 class Loader/Server profile과 runtime HUD. DimensionMaster는 combined body와 L/S/P/E 네 정적 기본 무기 파츠를 사용한다. runtime payload는 `Data/AssetPacks.lock.json`의 정확한 pack만 Hydrate/Verify한다.

별도 수직 슬라이스:

- 이동기·스탠스 전환과 skill별 weapon visibility/reattach
- `Data/UI` layout에서 `CUIObject` image widget을 생성하는 runtime factory
- 1280×720 reference 좌표 보정, draw-order 기반 2D UI picking과 input arbitration
- stable UI command binding과 Lobby/Scene/Gameplay typed command service 연결
- 추가 스킬
- party/raid admission과 roster
- 동적 collider, projectile, knockback/피격 판정
- 잡몹 및 추가 boss pattern

이 항목들은 현재 인터페이스를 우회해 임시 구현하지 않는다.
