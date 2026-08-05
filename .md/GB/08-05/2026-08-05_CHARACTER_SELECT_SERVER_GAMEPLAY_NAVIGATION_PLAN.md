# Character Select Lobby 승인 Server Arena·Animation 입력·Valtan Lazy Spawn PLAN

- 작성일: 2026-08-05
- 상태: 구현 및 자동 검증 완료, 실제 UI 조작 smoke 대기
- 대응 결과: `.md/GB/08-05/2026-08-05_CHARACTER_SELECT_SERVER_GAMEPLAY_NAVIGATION_RESULT.md`
- 데이터 정본: `Data/Balance`, `Data/Animation/Authored`, `Data/Navigation`, `Data/Worlds`

## G00. 결론, 원인, 완료 계약

### 결론

Character Select preview는 socket 없는 저작/선택 화면이고 Server Arena는 선택 class와 tokenized TEST를
Lobby가 승인한 뒤 같은 visual map을 다시 여는 gameplay 실행이다. 연결과 승인 권위는 Lobby 한 곳에 둔다.

```text
PREVIEW -> Enter Test -> LOBBY WAITING_FOR_APPROVAL
  -> approved socket one-shot handoff -> CONNECTING_PRESENTATION
  -> SERVER_ARENA
  -> LOBBY (disconnect/failure/back)
```

ImGui `Character Select` 창 최상단 mode 표시는 읽기 전용이다. Preview의 `Enter Test`가 class와 command를
commit하며 Character Select가 직접 connect/send/approval을 반복하지 않는다.

`Summon Valtan`은 `SERVER_ARENA`에서만 노출한다. 버튼은 Client가 발탄을 로컬 생성하는 기능이 아니다.
Client presentation prototype 준비가 성공한 뒤 stable placement ID를 Server에 요청하고, Server가 world template과
navigation을 검증해 entity를 commit하고 모든 Client에 spawn event를 broadcast한다.

### 확인된 실패 원인

1. 기존 Preview와 Server gameplay가 별도 `CLevel_CharacterSelect` instance였다. Preview 객체,
   `CAnimationTargetService` binding, camera의 F6 follow/free 상태가 Level 전환에서 모두 소멸했다.
2. Preview `Update()`는 `m_isServerGameplay == false`에서 즉시 반환해 replication과
   `CPlayerController`가 존재해도 실행할 수 없는 구조였다.
3. Server gameplay instance는 Character Select authoring ImGui를 렌더하지 않아 Animation Tool에서
   저장한 key → skill animation과 실제 Server snapshot presentation을 같은 actor로 검증할 수 없었다.
4. `CMainApp`은 ImGui `WantCaptureKeyboard`를 `CInput_Device`의 전역 keyboard block으로 바꿨다.
   `CPlayerController`와 `CCamera_Free`가 모두 차단된 DirectInput getter만 사용하므로 F1 도구가 열린
   Character Select에서는 Q/W/E/R/A/S/D/F/T/V와 F6가 함께 사라졌다.
5. Preview camera와 gameplay camera를 별도 생성하고 target bind 때마다 follow를 강제로 켜서,
   사용자가 선택한 F6 free 상태와 camera 위치가 보존될 수 없었다.
6. Valtan presentation prototype은 Valtan Arena loader에서만 eager load한다. Character Select가 Server에서
   boss spawn event를 받아도 prototype이 없어 event 적용이 실패한다.
7. Character Select world bootstrap에는 player spawn만 있고 Server가 런타임에 승인할 boss template과
   동적 spawn command가 없다.

### 완료 계약

- Preview에서 `Enter Test`를 누르면 선택 class와 tokenized TEST가 Lobby로 전달된다.
- Server 연결 실패, 승인 거부, 5초 timeout은 Lobby에 남는다.
- 승인된 socket은 one-shot handoff로만 소비하고 local replicated character 생성 뒤 gameplay target을 commit한다.
- 진입 후 disconnect/replication failure는 replicated state를 정리하고 Lobby로 복귀한다.
- Server Arena에서도 Character Select 창과 F1 Animation Tool을 유지한다.
- ImGui 창이 열려 있거나 focused인 것만으로 gameplay keyboard를 막지 않는다. 실제 text input 중에는
  선택적 passthrough도 막는다.
- Q/W/E/R/A/S/D/F/T/V 및 데이터에 존재하는 LMB/ALT_V가
  `CPlayerController -> IPlayerCommandSink -> Server -> snapshot -> CCharacter` 경로로만 재생된다.
- F6 follow/free 전환은 같은 camera instance에서 유지된다. Free camera에서는 gameplay command를 제출하지 않는다.
- 우클릭 이동은 기존 ImGui mouse capture 규칙을 유지한다.
- `Summon Valtan`은 Character Select Server Arena에서만 동작하며 중복 요청은 entity를 중복 생성하지 않는다.
- 발탄은 Server 승인 뒤 Client presentation asset을 지연 준비하고, 실패 시 기존 player/map/tool 상태를 보존한다.
- Character Select navigation과 world placement가 publisher 및 Server contract를 통과한다.
- Debug/Release 정본 빌드, protocol/client/server harness, ProjectAudit, `git diff --check` 결과를 남긴다.

## G01. Lobby 승인 one-shot handoff 상태 머신

### 소유 구조

`CLevel_CharacterSelect` preview와 Server Arena는 같은 visual map descriptor를 사용하지만 승인 전후에 다시
열리는 별도 Level 수명이다. gameplay instance가 다음을 소유한다.

- `CMapPlacementRuntime`
- `CCamera_Free`
- preview character 또는 replicated local character 중 현재 presentation target 하나
- `CClientReplication`
- `CNetworkPlayerCommandSink`
- `CPlayerController`
- mode, connection deadline, status, Valtan request 상태

```cpp
enum class CHARACTER_SELECT_MODE
{
    CONNECTING,
    SERVER_ARENA,
    RETURNING_TO_LOBBY
};
```

Preview `Initialize()`는 map, light, preview, camera를 준비하지만 socket을 열지 않는다. `Enter Test`는
`CCharacterSelectionState`에 선택 class와 `SERVER_GAMEPLAY` handoff intent를 남기는 것이 아니라 먼저 tokenized
TEST를 Lobby로 제출한다. Lobby가 승인 payload의 protocol/world/player/entity를 검증한 다음에만
`Stage_TestEntryMode(SERVER_GAMEPLAY)`를 commit한다. gameplay `Initialize()`는 이를 원자적으로 한 번 소비한다.

### Preview -> Lobby -> Server Arena

1. Preview에서 선택 class를 `CCharacterSelectionState`에 commit한다.
2. `Enter Test`가 `CLobbyCommandService`의 tokenized TEST를 제출하고 Lobby 로드를 요청한다.
3. Lobby만 endpoint 연결과 `C2S_ENTER_WORLD(CHARACTER_SELECT_ARENA)` 전송을 소유한다.
4. Lobby가 `S2C_ENTER_ACCEPTED` 전체 payload와 5초 deadline을 검증한다.
5. 승인 socket과 queued spawn snapshot을 닫지 않고 `SERVER_GAMEPLAY` handoff를 stage한다.
6. 같은 visual map의 gameplay instance가 handoff를 한 번 소비하고 replication을 즉시 update한다.
7. local character 생성 뒤 camera/Animation Tool target과 controller를 commit한다.

승인 전 실패는 Lobby에 남는다. 승인 후 presentation 실패나 disconnect는 connection/replication을 정리하고
Lobby로 복귀한다. 같은 Level preview 또는 local gameplay fallback으로 전환하지 않는다.

### Server Arena -> Lobby

`CNetworkManager` connection, `CClientReplication`, HUD/controller/tool target을 멱등 정리하고
`CLevelTransitionService`로 Lobby를 요청한다. 새 preview를 gameplay Level 안에 재생성하지 않는다.

### ImGui 배치

```text
Character Select
  Mode: [Preview] [Server Arena (Lobby-approved)]   // read-only
  Status: Preview / Connecting 2.3s / Server Arena
  --------------------------------
  Character class list
  --------------------------------
  [Summon Valtan (Lazy)]    // Server Arena 전용
  [Enter Test] [Enter Bern] [Enter Valtan] [Back to Lobby]
```

Server Arena에서는 session class가 Server authority이므로 class list를 read-only로 표시한다. class를 바꾸려면
Lobby를 거쳐 Preview를 다시 연다. Animation Tool은 현재 `CAnimationTargetService` target을 사용한다.

## G02. 선택적 keyboard passthrough와 F6 camera 보존

### Engine input 계약

기존 `Get_DIKeyState/Get_DIKeyPressed`는 ImGui capture를 존중하는 기본 API로 유지한다. Engine에는 물리 상태를
읽는 명시적 raw API만 추가한다.

```cpp
int8_t Get_DIKeyStateRaw(uint8_t keyId) const;
bool_t Get_DIKeyPressedRaw(uint8_t keyId) const;
```

전역 block을 해제하거나 `WantCaptureKeyboard`의 의미를 바꾸지 않는다. Character Select가 선택한 controller와
camera만 raw keyboard를 소비하므로 Bern/Valtan/다른 editor의 입력 계약은 변하지 않는다.

### PlayerController

`CPlayerController`에 `Set_AllowCapturedKeyboardInput(bool)`를 추가한다.

- `SERVER_ARENA`: true
- 다른 Level 및 Preview: false 또는 controller 미호출
- `ImGui::GetIO().WantTextInput == true`: raw bypass 금지
- mouse picking/basic attack: 기존 mouse capture를 그대로 존중

키 edge state는 raw/blocked 전환 때 held key를 신규 press로 오인하지 않도록 매 frame 실제 선택된 input source로
갱신한다. skill ID는 계속 `PlayerSkills.json inputSlot`을 `CPlayerSkillCatalog`로 조회하며 코드에 하드코딩하지 않는다.

### Camera

`CCamera_Free::CAMERA_FREE_DESC`에 Character Select 전용 keyboard passthrough flag를 추가한다.
F6, Tab, free WASD는 해당 flag가 켜졌고 text input이 아닐 때 raw keyboard를 사용한다.

camera target 교체는 다음 순서를 지킨다.

```cpp
const bool wasFollowEnabled = camera->Is_FollowEnabled();
camera->Set_FollowTarget(newTargetTransform);
camera->Set_PositionOffset(serverOrPreviewOffset);
camera->Set_FollowEnabled(wasFollowEnabled);
```

최초 preview camera만 follow=true로 시작한다. Server Arena의 F6 상태를 강제로 덮지 않으며 free camera에서는
gameplay command를 제출하지 않는다.

## G03. Lobby 단일 Server 승인 경계

Character Select는 반드시 Lobby command token을 거친다. Lobby가 Test/Bern/Valtan의 endpoint 연결,
`C2S_ENTER_WORLD`, 승인 payload 검증과 5초 deadline을 단독 소유한다. Character Select gameplay는 승인된
socket을 닫거나 재연결하지 않고 handoff state와 queued replication만 소비한다.

중복된 host 환경변수 처리를 만들지 않도록 기본 endpoint 해석은 `CNetworkManager`의 public 정적 계약으로
이동한다.

```cpp
static std::string Resolve_ServerHost();
static constexpr std::uint16_t DEFAULT_SERVER_PORT = 7777;
```

- `LOSTARK_SERVER_HOST` 없음/빈 값/`0.0.0.0` -> `127.0.0.1`
- process-local 설정만 사용
- 실패 status에는 host, port, WSA error를 보존
- 자동 local gameplay fallback 없음

`CLevel_CharacterSelect` destructor와 mode exit는 연결/replication/tool binding을 멱등 정리한다. disconnect는
`RETURNING_TO_LOBBY`로 바꾸고 Lobby 전환을 요청한다. Bern/Valtan/Back도 기존 connection을 정리한 뒤 tokenized
Lobby command/transition service를 사용한다.

## G04. Valtan Server-authoritative lazy spawn

### Data 정본

`Data/Worlds/LV_LOBBY_CLASSSELECT_SL00/Gameplay.world.json` revision을 올리고 disabled boss template을 추가한다.

```json
{
  "placementId": "boss.valtan.character-select.lazy",
  "kind": "boss",
  "archetypeId": "BOSS_VALTAN",
  "encounterId": "ENCOUNTER_VALTAN",
  "position": [-772.0, -142.55, 207.0],
  "yawDegrees": 180.0,
  "enabled": false
}
```

publisher는 기존 `ValtanEncounter.json`으로 pattern/action/damage profile을 확장한다. disabled는 초기 spawn에서
제외되지만 schema/catalog/navigation 검증 대상에는 포함한다. Runtime bootstrap을 직접 편집하지 않는다.

### Protocol

protocol version을 9로 올리고 stable placement ID만 가진 intent를 추가한다.

```cpp
struct C2S_SPAWN_WORLD_ENTITY
{
    std::string strPlacementId;
};
```

`MAX_STABLE_NETWORK_ID_BYTES`, stable ID charset, trailing payload를 writer/reader/harness에서 검증한다. Client가
archetype, transform, HP, pattern을 보내지 않는다.

### Server

`ROOM_COMMAND_TYPE::SPAWN_WORLD_ENTITY`와 handler를 추가한다. handler는 다음을 모두 만족할 때만 commit한다.

- authenticated session이 해당 room의 player를 소유
- room world가 `CHARACTER_SELECT_ARENA`
- placement ID가 bootstrap에 존재
- `enabled == false`, kind가 boss, archetype이 `BOSS_VALTAN`
- 같은 placement ID의 runtime entity가 없음
- gameplay profile, encounter, damage profile 존재
- spawn point가 server navigation에 project 가능
- net entity ID가 유효

동일 placement 재요청은 실패로 session을 끊지 않고 멱등 성공으로 처리한다. 새 entity는 임시 구조체에 전부
검증한 뒤 `m_WorldEntities`에 한 번 commit하고 `S2C_WORLD_ENTITY_SPAWNED`를 broadcast한다.

초기 enabled entity 생성과 lazy entity 생성은 동일 `Build_WorldEntity` helper를 사용해 수치 적용 경로를
분기시키지 않는다.

### Client presentation lazy load

`CLoader::Ready_ValtanPresentation`의 실제 prototype 조립을 재사용 가능한
`CValtanPresentationAssetService`로 이동한다.

```cpp
static HRESULT Ensure_Prototypes(device, context, levelIndex);
static bool_t Is_Ready(levelIndex);
static void Begin_LevelLoad(levelIndex);
```

- Valtan Arena loader도 동일 service를 호출한다.
- Character Select `Summon Valtan` button은 먼저 `Ensure_Prototypes`를 호출한다.
- 성공한 경우에만 Server request를 보낸다.
- replication은 remote spawn 안전망으로 `Apply_WorldEntitySpawn` 전에 다시 `Ensure_Prototypes`한다.
- asset 실패 시 Server request를 보내지 않고 기존 map/player/tool state를 보존한다.
- 이미 ready/spawned/requesting이면 중복 load/request를 하지 않는다.

## G05. Navigation, 수치 데이터, Animation Tool 경계

Character Select navigation 정본은 다음과 같다.

```text
Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navsource
Data/Navigation/LV_LOBBY_CLASSSELECT_SL00.navpaint
  -> Tools/NavigationPipeline/Publish-ServerNavigation.ps1
  -> Client/Bin/DataFiles/Navigation/LV_LOBBY_CLASSSELECT_SL00.navgrid
```

player spawn 네 개와 disabled Valtan template을 walkable/projectable point로 검증한다. Server movement,
skill movement correction, Valtan chase는 이 `.navgrid`만 정답으로 사용한다.

수치 정본은 변경하지 않는다.

- Player profile: HP/resource/regen/attack/move
- Skill: cooldown/duration/hit time/cost/range/movement/damage profile/combo stage
- Boss: HP/attack/radius/engage/move/phase
- Damage: rate percent

이번 구현은 위 데이터가 실제 Server action/snapshot/presentation 경로에 도달하도록 연결할 뿐 Client local
damage나 local skill 재생 경로를 추가하지 않는다. `defense`가 현재 parse/validate만 되고 mitigation에 사용되지
않는 기존 경계도 별도 balance 수직 슬라이스로 남긴다.

Animation Tool은 `Data/Animation/Authored/<Class>/<Class>.skillbindings.json`을 계속 정본으로 사용한다.
Q/W/E/R/A/S/D/F/T/V 및 COMBO stage별 binding 저장/수정 계약은 mode 전환으로 복제하지 않는다.
Preview에서는 preview actor, Server Arena에서는 replicated local actor가 같은 service target이 된다.

## G06. Harness, Audit, build, 수동 smoke

### 자동 검증

1. JSON/XML parse와 publisher dry run
2. Character Select world publish: disabled Valtan template이 초기 entity에서 제외되고 runtime template에는 남음
3. Navigation publish: 4 player spawn + Valtan template project 성공
4. NetworkProtocolHarness Debug/Release
   - protocol v9 round trip
   - empty/oversize/invalid-char/trailing payload 거부
5. Server contract Debug/Release
   - Character Select room 초기 boss 없음
   - 다른 world/미입장 session/unknown placement/enabled placement 거부
   - lazy Valtan 한 개 생성, 중복 요청 멱등, spawn broadcast/snapshot 포함
   - navigation projection 및 balance profile 적용
6. ClientFrontendHarness Debug/Release
   - mode state rollback
   - keyboard passthrough가 text input에서는 차단
   - camera target swap이 follow/free 상태 보존
   - asset service 중복 prepare 멱등
7. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
8. `git diff --check`

### 정본 빌드 순서

```powershell
Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
```

Engine public input header가 바뀌므로 두 configuration 모두 Engine -> UpdateLib -> Shared/harness -> Server ->
Client 순서를 생략하지 않는다. 실행 중 Client가 출력물을 점유하면 종료한 뒤 링크를 재실행한다.

### 실제 EXE smoke

작업 디렉터리는 `Client/Default`를 사용한다.

```text
1. Server.exe 실행
2. Client.exe -> Lobby -> Character Select
3. F1 Animation Tool 열기, Preview class 변경 및 binding 확인
4. Enter Test -> Lobby 승인 -> 같은 visual map Server Arena 재진입
5. 승인 socket 재접속 없이 replicated actor와 tool target 연결 확인
6. Q W E R A S D F T V와 LMB/ALT+V -> Server snapshot -> animation 확인
7. F6 free에서 command 차단, follow 복귀 후 새 press만 제출되는지 확인
8. Animation Tool InputText 편집 중 gameplay key가 제출되지 않음 확인
9. Summon Valtan -> 최초 지연 준비 -> Server spawn -> chase/combat/damage/HUD 확인
10. 중복 클릭으로 두 번째 Valtan이 생기지 않음 확인
11. Back/disconnect -> replicated actor/boss 정리 및 Lobby 복귀 확인
12. Server 미실행/거부/timeout -> Lobby에 남는지 확인
```

Resources immutable pack이 현재 lock manifest와 다르면 DeepAssetHash 및 전체 visual smoke는 PASS로 기록하지
않는다. code build/harness 결과와 resource hydration blocker를 분리해 RESULT에 남긴다.

## G07. 변경 파일과 범위 밖

### 예정 변경 파일

| 영역 | 파일 |
|---|---|
| Engine input | `Engine/Public/Input_Device.h`, `Engine/Public/GameInstance.h`, `Engine/Private/GameInstance.cpp` |
| Shared protocol | `Shared/Public/Network/PacketType.h`, `Shared/Public/Network/PacketMessages.h`, `Shared/Private/Network/PacketMessages.cpp` |
| Client mode/network/input | `Level_CharacterSelect.*`, `NetworkManager.*`, `PlayerController.*`, `Camera_Free.*`, `ClientReplication.*` |
| Client lazy asset | `ValtanPresentationAssetService.h/.cpp`, `Loader.*`, Client project/filter |
| Server authority | `RoomCommand.h`, `ServerApp.cpp`, `GameRoom.h/.cpp` |
| Data/publisher | Character Select `Gameplay.world.json`, world/navigation publisher와 generated runtime data |
| 검증 | NetworkProtocolHarness, ClientFrontendHarness, ServerGameplayContractTests, ProjectAudit |
| public 문서 | `AGENTS.md`, `CLAUDE.md`, TEAM gameplay/animation handoff, 대응 RESULT |

새 C++ 두 파일은 물리 폴더를 정본으로 만들고 `Client.vcxproj`와 `.filters`에 필요한 항목만 등록한다.

### 범위 밖

- Client local skill/damage fallback
- hot reload revision protocol
- defense mitigation 신규 공식
- MapTool gameplay trigger 일반화
- Valtan 외 임의 archetype spawn UI
- ImGui widget을 제품 UI로 승격
- Resources payload 커밋

이 항목들은 이번 완료를 위해 placeholder나 두 번째 runtime 경로를 만들지 않는다.

## G08. 비평 검토와 반영 결론

비평 에이전트는 최초안의 파괴적 전환, prototype 부분 등록, spawn 응답 부재, remote presentation
실패, text input 경계, UI와 transport 결합, F6 의도 상태 소실을 차단점으로 지적했다. 결론을 그대로
수용하지 않고 각 항목을 실제 호출 경로와 실행 계약으로 재검증한 뒤 다음처럼 반영했다.

| 지적 | 실질 반영 |
|---|---|
| 기존 Lobby 승인 계약과 직접 접속 충돌 | 직접 접속을 제거하고 Lobby 승인 payload 검증 -> one-shot handoff -> Character Select replication 소비로 고정 |
| Preview local rollback이 제품 Server 필수를 우회 | 실패/disconnect는 replicated state를 정리하고 Lobby로 복귀 |
| Valtan prototype 일부만 live 등록될 수 있음 | Engine `Add_Prototypes` batch가 전체 tag/prototype을 사전 검증하고 한 번에 merge |
| lazy spawn 성공·거부·멱등 결과 부재 | protocol v9의 typed request/result와 `SPAWNED/ALREADY_EXISTS/REJECTED` 추가 |
| remote asset 준비 실패 뒤 불일치 | spawn 적용 전에 presentation asset을 준비하고 실패 시 Lobby로 통제 복귀 |
| InputText 종료 직후 held key 오발사 | 입력 억제 중에도 raw physical shadow를 갱신해 새로운 edge로 오인하지 않음 |
| ImGui가 network transport를 직접 호출 | `IWorldEntityCommandSink` typed 경계로 summon command 제출 |
| target 교체 공백에서 F6 follow 의도 소실 | active follow와 `follow requested` 상태를 분리해 target 교체 후 복원 |

CONNECTING 중 remote entity가 도착할 수 있다는 지적은 현재 Character Select world의 초기 boss가
disabled이고 local replicated character 준비가 완료되기 전에는 mode를 commit하지 않는 것으로 경계를
고정했다. 일반적인 다중 entity 원자 stage는 이번 단일-player arena 범위 밖이며 향후 world streaming
transaction으로 확장한다.

계획 완료 증거는 대응 RESULT의 Debug/Release 정본 빌드, 세 harness, Server contract, 실제 TCP 중복
spawn smoke, ProjectAudit 결과를 기준으로 한다. 코드와 데이터가 이 문서의 구현 정본이다.
