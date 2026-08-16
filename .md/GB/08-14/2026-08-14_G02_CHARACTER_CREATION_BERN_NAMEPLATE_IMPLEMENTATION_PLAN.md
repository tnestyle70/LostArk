# 2026-08-14 G02 Character Creation, Bern Entry, Nameplate 구현 계획서

## 0. 문서 상태와 구현 위치

- 상태: **IMPLEMENTED / AUTOMATED PASS / PARTIAL MANUAL PENDING**
- 이 문서는 구현된 G02의 범위 정본이며 실제 결과와 남은 검증 경계는
  `.md/GB/08-15/2026-08-15_G02_CHARACTER_CREATION_BERN_NAMEPLATE_RESULT.md`가 소유한다.
- 직접 반영할 코드와 정확한 삽입 위치는 같은 폴더의 `2026-08-14_G02_CHARACTER_CREATION_BERN_NAMEPLATE_DETAIL_PLAN.md`가 소유한다.
- 구현·빌드·검증 위치는 `C:\Users\user\Desktop\LostArk` 실제 폴더 하나로 고정한다.
- `.codex\worktrees\...` 아래 Codex 분리 폴더에는 이 G의 코드·문서·빌드 결과를 만들지 않는다.
- `main`은 Git 브랜치 이름이며 폴더 이름이 아니다. 이 문서에서 `실제 폴더`는 위 절대 경로만 뜻한다.
- 이 문서 작성 자체는 C++/JSON/프로젝트 파일을 변경하지 않는다.
- 실제 폴더에는 다른 Effect 작업의 미커밋 변경이 있으므로 G02 구현자는 관련 없는 파일을 reset, clean, checkout, stash 또는 전체 파일 덮어쓰기로 정리하지 않는다.

현재 선행 상태는 다음과 같다.

| 단계 | 상태 | 기준 |
|---|---|---|
| G00 | CLOSED | 4-player Valtan baseline, 단일 boss HP bar, replay reset |
| G01 | CLOSED | Character Select session-private Server arena, Bern/Valtan/Training shared simulation |
| G02 | IMPLEMENTED / AUTOMATED PASS / PARTIAL MANUAL PENDING | Create Character -> nickname -> Server-approved Bern -> replicated nameplate |

G01 이후 정확한 world 소유권은 다음과 같다.

- `CHARACTER_SELECT_ARENA`: session-private `CGameRoom`
- `BERN`, `VALTAN_ARENA`, `TRAINING_GROUND`: world-shared `CGameRoom`

관련 완료 문서:

- `.md/GB/08-14/2026-08-14_FOUR_PLAYER_VALTAN_G00_BASELINE_RESULT.md`
- `.md/GB/08-14/2026-08-14_CHARACTER_SELECT_SESSION_PRIVATE_SERVER_ARENA_RESULT.md`

## 1. G02 최종 사용자 흐름

### 1.1 Create Character -> Bern

1. Lobby에서 `Character Select`를 누른다.
2. Lobby가 Server 승인을 받은 뒤 session-private Character Select arena를 연다.
3. 사용자는 여섯 class 중 하나를 선택한다. class 변경과 몬스터 소환·스킬·collider·damage는 계속 Server authority다.
4. 사용자가 `Create Character`를 누르면 닉네임 입력 모달이 열린다.
5. 닉네임을 입력하고 확인하면 선택 class와 닉네임을 pending identity로 stage한다.
6. Character Select는 Lobby command와 Lobby load 요청을 먼저 성공시킨 뒤에만 현재 arena를 정리한다.
7. Lobby는 hardcoded `"Player"`가 아니라 pending identity의 class/nickname으로 Bern `C2S_ENTER_WORLD`를 보낸다.
8. Server는 Bern player에 닉네임을 저장하고 기존 `S2C_PLAYER_SPAWNED`로 모든 Bern Client에 복제한다.
9. Bern resource 생성, rendering profile 활성화, `Change_Level(BERN)`까지 성공한 뒤 `CMainApp::Apply_LevelRequest()`가 pending identity를 process-session의 created identity로 commit한다.
10. Bern의 각 Client는 Server가 복제한 닉네임을 캐릭터 머리 위에 표시한다.

### 1.2 Direct Valtan test entry

`Enter Valtan Map`은 Create Character의 Bern 흐름과 분리된 빠른 전투 검증 명령으로 유지한다.

- created nickname이 있으면 현재 선택 class + created nickname으로 Valtan에 입장한다.
- created nickname이 없으면 현재 선택 class + process-local audition nickname `Test-<process-id>`로 입장한다.
- audition nickname은 표시용이며 social identity나 계정 ID가 아니다.
- 닉네임 중복은 이번 G에서 허용한다. 이후 Party는 닉네임이 아니라 별도 stable social identity를 사용한다.
- Character Select에서 Valtan 버튼을 누르는 사용 흐름은 유지하지만 현재 구조처럼 typed Lobby command를 거쳐 Server 승인을 받은 뒤 Valtan Level을 연다.

### 1.3 실패 시 보존 계약

- invalid nickname: modal을 닫지 않고 현재 draft와 기존 created identity를 보존한다.
- Lobby command 요청 실패: Character Select arena와 캐릭터를 유지하고 pending identity만 취소한다.
- Lobby load 요청 실패: command token과 pending identity를 취소하고 Character Select를 유지한다.
- Server connect/send/approval 실패: socket을 닫고 pending identity를 취소하되 기존 created identity는 유지한다.
- Bern resource load/profile activation/level 교체 실패: socket을 닫고 partial resource를 rollback하며 pending identity만 취소한다. 이전 created identity가 있었다면 그대로 유지한다.
- nameplate projection 또는 font draw 실패: gameplay·replication·damage는 유지하고 해당 nameplate만 그리지 않는다.

## 2. 현재 코드 실측

### 2.1 이미 있는 기반

- `Shared/Public/Network/PacketMessages.h`
  - `C2S_ENTER_WORLD::strNickName`
  - `S2C_PLAYER_SPAWNED::strNickName`
- `Shared/Public/Network/PacketType.h`
  - `MAX_NICKNAME_BYTES = 32`
- `Shared/Private/Network/PacketMessages.cpp`
  - nickname empty/32-byte upper bound codec
- `Server/Public/ServerPlayer.h`
  - `SERVER_PLAYER::strNickName`
- `Server/Private/GameRoom.cpp`
  - `CGameRoom::Join()`이 entry nickname을 player에 저장
  - `Send_Spawned()`가 exact nickname을 broadcast
  - G01 world transfer가 source nickname을 target entry에 전달
- `Client/Public/NetObjectRegistry.h`
  - `NET_PLAYER_RECORD::strNickName`
- `Client/Private/ClientReplication.cpp`
  - spawn nickname을 `CCharacter::DESC`까지 전달
- `Client/Public/Character.h`
  - `Get_NickName()`
- `Client/Private/Level_Loading.cpp`
  - load 실패 시 connection close와 partial resource rollback

### 2.2 실제 누락과 잘못된 기본값

- `Client/Private/Level_Lobby.cpp`
  - `PLAYER_NICKNAME = "Player"`를 모든 entry에 사용한다.
  - selection이 없으면 Lance Master를 생성 완료 상태처럼 자동 commit한다.
- `Client/Public/CharacterSelectionState.h`, `Client/Private/CharacterSelectionState.cpp`
  - class 하나만 보관하고 pending/created nickname transaction이 없다.
- `Client/Private/Level_CharacterSelect.cpp`
  - `Create Character`와 nickname input이 없다.
  - `Enter_Stage()`가 Lobby command/load 성공 전에 character, replication, socket을 먼저 정리한다.
- `Client/Private/Character.cpp`
  - `Render()`는 `S_OK`만 반환하며 nameplate가 없다.
- `CClientReplication`
  - record, character, nickname, stable IDs를 묶은 read-only player presentation view가 없다.
- Bern/Valtan Level
  - replicated player nameplate consumer가 없다.

## 3. 이번 G의 범위와 명시적 비범위

### 3.1 포함

- 1~32 UTF-8 byte nickname 검증
- selected class와 pending/created nickname의 atomic state
- Create Character modal과 Bern command
- Lobby hardcoded `"Player"` 제거
- Server in-memory player/session nickname 저장과 exact echo 검증
- Character Select 빠른 Valtan entry용 audition nickname
- Bern/Valtan의 모든 replicated player 머리 위 nameplate
- load/connect/transition 실패 rollback
- Shared, Server, Client, harness, `.vcxproj/.filters`, RESULT 갱신

### 3.2 제외

- 계정 로그인, DB, 파일 기반 캐릭터 영구 저장
- 닉네임 전역 고유성 및 중복 예약
- 캐릭터 슬롯 여러 개
- hover/RMB, Invite/Cancel, Party roster/HP
- ROOM/PARTY chat
- nameplate 가림 판정, 거리 fade, 길드/칭호
- ImGui command modal의 최종 제품 아트 교체

이번 G에서 `Server에 저장`은 다음 두 조건을 뜻한다.

1. 해당 Server player/session이 살아 있는 동안 `SERVER_PLAYER::strNickName`이 authority다.
2. Bern↔Valtan Server world transfer가 동일 닉네임을 유지한다.

Client 재실행 뒤에도 남는 영구 저장은 별도 인증/캐릭터 DB G가 필요하므로 이번 완료 주장에 포함하지 않는다.

## 4. 닉네임 계약

기존 packet layout은 바꾸지 않는다. 따라서 G02만으로 protocol version을 올리지 않는다.

`Shared/Public/Network/PacketMessages.h`에 다음 public validator를 선언하고 Client, codec, Server가 같은 함수를 사용한다.

```cpp
[[nodiscard]] bool Is_Valid_PlayerNickname(
    std::string_view nickname) noexcept;
```

검증 규칙:

- UTF-8 byte 수 `1..MAX_NICKNAME_BYTES`
- well-formed UTF-8
- embedded NUL 금지
- C0/C1 control code 금지
- 앞·뒤 ASCII whitespace 금지
- 한글은 UTF-8 byte 수로 계산하므로 32-byte 한도를 넘지 않아야 한다.

중복 nickname은 허용한다. nickname은 display text이며 다음 용도로 사용하지 않는다.

- player/session lookup key
- Party member identity
- map/vector ordering
- save key
- reconnect authentication

`Write_Message/Read_Message(C2S_ENTER_WORLD)`, `Write_Message/Read_Message(S2C_PLAYER_SPAWNED)`, `CGameRoom` entry validation은 모두 이 validator를 소비한다. malformed packet은 기존 protocol fault 경계대로 session close 대상이며 nickname 전용 신규 reject packet을 추가하지 않는다.

## 5. CharacterSelectionState 계약

### 5.1 상태

`Client/Public/CharacterSelectionState.h`에 다음 의미를 가진 DTO를 둔다.

```cpp
enum class CHARACTER_ENTRY_IDENTITY_SOURCE
{
    AUDITION,
    CREATED,
    PENDING_CREATION,
    END
};

struct CHARACTER_ENTRY_IDENTITY
{
    LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
        LostArk::Shared::CHARACTER_CLASS_ID::END;
    std::string strNickname;
    CHARACTER_ENTRY_IDENTITY_SOURCE eSource =
        CHARACTER_ENTRY_IDENTITY_SOURCE::END;
};
```

내부 상태는 한 mutex 아래 다음을 함께 소유한다.

- optional selected class
- optional committed created nickname
- optional pending creation `{selected class snapshot, nickname}`
- process lifetime 동안 한 번 만든 audition nickname

### 5.2 public 함수 책임

| 함수 | 한 줄 책임 |
|---|---|
| `Select(classId)` | 현재 audition/entry class만 바꾸며 created nickname은 보존한다. |
| `Try_Get_SelectedClass(out)` | 현재 선택 class를 읽는다. |
| `Stage_Creation(classId, nickname)` | 전체 검증 뒤 pending identity를 atomic replace한다. 실패 시 기존 상태 불변이다. |
| `Has_PendingCreation()` | Bern activation commit 대상이 있는지 반환한다. |
| `Commit_PendingCreation()` | pending class/nickname을 selected+created에 한 번에 commit하고 pending을 비운다. |
| `Cancel_PendingCreation()` | pending만 비우고 selected class와 created nickname을 보존한다. |
| `Try_Resolve_ForWorld(worldId, out)` | world 정책에 따라 pending/created/audition identity를 반환한다. |

world별 resolve 정책:

| world | class | nickname |
|---|---|---|
| Character Select | selected, 없으면 audition Lance Master | audition `Test-<process-id>` |
| Training/Test | selected, 없으면 audition Lance Master | audition `Test-<process-id>` |
| Bern | pending creation, 없으면 selected + committed created nickname | created nickname이 없으면 실패 |
| Valtan | selected, 없으면 audition Lance Master | committed created nickname, 없으면 audition nickname |

pending creation은 Bern 외 world에 암묵적으로 소비하지 않는다.

## 6. G02-A — Create Character와 Bern transaction

### 6.1 수정 파일과 정확한 기준점

| 파일 | 기준 함수/위치 | 반영 내용 |
|---|---|---|
| `Shared/Public/Network/PacketMessages.h` | message declarations 앞 | `Is_Valid_PlayerNickname` public 선언 |
| `Shared/Private/Network/PacketMessages.cpp` | `C2S_ENTER_WORLD` / `S2C_PLAYER_SPAWNED` codec | 공통 validator 구현·소비 |
| `Server/Private/GameRoom.cpp` | anonymous `Is_Valid_EnterWorld` | 중복 length check를 공통 validator 호출로 교체 |
| `Client/Public/CharacterSelectionState.h` | class 전체 | pending/created/audition identity 계약 추가 |
| `Client/Private/CharacterSelectionState.cpp` | global selection state 전체 | mutex 아래 stage/commit/cancel/resolve 구현 |
| `Client/Public/Level_CharacterSelect.h` | private functions/members | modal 함수, nickname draft, modal state 추가 |
| `Client/Private/Level_CharacterSelect.cpp` | `Enter_Stage`, `Render_SelectionPanel`, `Update_ServerArena` | Create modal, 안전한 transition 순서, modal 중 gameplay input 차단 |
| `Client/Public/Level_Lobby.h` | pending entry members | pending creation을 추적할 entry flag/identity source 추가 |
| `Client/Private/Level_Lobby.cpp` | namespace constants, `Resolve_EntryCharacterClass`, `Begin_NetworkEntry`, `Cancel_PendingEntry` | hardcoded Player/default commit 제거, world identity resolve, 실패 cancel |
| `Client/Private/Level_Loading.cpp` | `Recover_FromFailure` | target resource load 실패 시 pending creation 취소 |
| `Client/Private/MainApp.cpp` | `Apply_LevelRequest` LOAD/ACTIVATE 분기 | LOAD 시작 실패와 activation 실패 시 pending 취소, Bern `Change_Level` 성공 뒤 commit |

### 6.2 Character Select modal

현재 `Render_SelectionPanel()`은 Character Select의 typed debug/selection command surface다. G02에서는 이 기존 경로를 확장하며 두 번째 socket/UI runtime을 만들지 않는다.

추가 private 함수:

```cpp
void Open_CreateCharacterModal();
bool_t Confirm_CreateCharacter();
void Cancel_CreateCharacter();
void Render_CreateCharacterModal();
```

추가 state:

```cpp
std::array<char_t, LostArk::Shared::MAX_NICKNAME_BYTES + 1u>
    m_NicknameDraft{};
bool_t m_isCreateCharacterModalOpen = false;
```

UI 동작:

- 기존 `Enter Bern` 버튼을 `Create Character`로 바꾼다.
- click은 modal open만 수행한다.
- `InputText` buffer capacity는 terminating NUL을 포함해 `MAX_NICKNAME_BYTES + 1`이다.
- confirm은 trim으로 사용자 입력을 몰래 바꾸지 않는다. exact draft를 validator에 전달한다.
- validation 실패 시 modal과 draft를 유지하고 `m_strStatus`에 byte 한도/invalid UTF-8를 표시한다.
- stage 성공 뒤 `Enter_Stage(BERN)`이 실패하면 pending creation을 취소하고 modal/draft를 유지한다.
- Lobby load request까지 성공한 뒤에만 modal을 닫고 `MODE::RETURNING_TO_LOBBY`로 간다.
- modal이 열린 동안 `CPlayerController::Update`, class change, spawn command, Bern/Valtan/Back command를 막는다.

이 modal은 기존 Character Select command surface의 기능 확인 UI다. 최종 제품 아트로 승격하거나 UI 이미지를 ImGui widget 캡처로 만들지 않는다.

### 6.3 `Enter_Stage()` 순서 교정

현재 구현은 request 전 socket과 replication을 먼저 파괴한다. 다음 순서로 바꾼다.

1. stage와 selected class 검증
2. `CLobbyCommandService::Request(stage, token)`
3. `CLevelTransitionService::Request_Load(LOBBY, source, token)`
4. 두 요청이 성공한 뒤 `MODE::RETURNING_TO_LOBBY` commit
5. 실제 Level 교체/destructor가 animation target, socket, replication, controller를 정리

1~3 중 실패하면 현재 Character Select presentation과 Server arena connection을 그대로 유지한다.

### 6.4 Lobby entry

`Level_Lobby.cpp`에서 다음을 삭제한다.

- `PLAYER_NICKNAME = "Player"`
- `DEFAULT_ENTRY_CLASS`를 created selection처럼 commit하는 경로
- `Resolve_EntryCharacterClass(..., outUsedDefault)`

`Begin_NetworkEntry()`는 먼저 `Try_Resolve_ForWorld()`로 exact class/nickname을 stage한 뒤 connect/send한다. resolve 실패 시 socket을 건드리지 않는다.

Bern entry가 pending creation을 사용했다면 `m_hasPendingCharacterCreationEntry`를 true로 저장한다. 이후 다음 실패 소비자는 `Cancel_PendingCreation()`을 호출한다.

- connect 실패
- send 실패
- typed entry rejection
- approval timeout
- invalid approval
- approved Level load request 실패

approval 성공만으로 created identity를 commit하지 않는다.

### 6.5 Bern commit과 load rollback

`CLevel_Bern::Initialize()`는 최종 commit 지점이 아니다. Registry가 Bern 객체를 만드는 시점은 rendering profile 활성화와 실제 `Change_Level`보다 이르므로, 여기서 commit하면 뒤 단계 실패에도 생성 신원이 남는 오류가 생긴다.

정확한 소유자는 `CMainApp::Apply_LevelRequest()`다.

- LOAD 단계에서 `Start_Level()`이 실패하면 pending creation을 취소한다.
- `CLevel_Loading::Recover_FromFailure()`가 target resource load 실패를 받으면 pending creation을 취소한다.
- ACTIVATE 단계에서 profile 활성화 또는 `Change_Level()`이 실패하면 pending creation을 취소한다.
- `request.eTargetLevel == LEVEL::BERN`이고 `Change_Level()`까지 성공한 경우에만 `Commit_PendingCreation()`을 호출한다.
- Valtan, Character Select, Training, Lobby 전환은 pending creation을 commit하지 않는다.
- 취소는 pending만 비우며 이전 created identity는 유지한다.

따라서 identity 때문에 `Level_Bern.h/.cpp`를 수정하지 않는다.

## 7. G02-B — Server-replicated nameplate

### 7.1 렌더 소유권

nameplate를 `CCharacter::Render()`에 넣지 않는다. Character는 gameplay/presentation object이고 여러 Level이 같은 UI 정책을 공유해야 한다.

신규 read-only view:

- `Client/Public/WorldPlayerNameplateView.h`
- `Client/Private/WorldPlayerNameplateView.cpp`

이 view는 Server-replicated player DTO를 소비하고 `CGameInstance::Draw_Text(TEXT("Font_YG330"), ...)`로 text만 그린다. ImGui draw/widget과 image asset을 사용하지 않는다.

### 7.2 Registry/Replication DTO

`NetObjectRegistry`는 현재 `Get_LiveObjects()`에서 record와 character pairing을 잃는다. 다음 read-only pair를 추가한다.

```cpp
struct LIVE_NET_PLAYER
{
    NET_PLAYER_RECORD Record;
    std::shared_ptr<CCharacter> pCharacter;
};
```

`Get_LivePlayers()`는 occupied slot 중 살아 있는 character만 record와 함께 반환한다. vector index는 identity나 Party ordering으로 사용하지 않는다.

`ClientReplication.h`에는 Level/UI consumer용 DTO를 둔다.

```cpp
struct REPLICATED_PLAYER_VIEW
{
    LostArk::Shared::PLAYER_ID iPlayerId =
        LostArk::Shared::INVALID_PLAYER_ID;
    LostArk::Shared::NET_ENTITY_ID iNetEntityId =
        LostArk::Shared::INVALID_NET_ENTITY_ID;
    LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
        LostArk::Shared::CHARACTER_CLASS_ID::END;
    std::string strNickname;
    bool_t isLocal = false;
    std::weak_ptr<CCharacter> pCharacter;
};
```

```cpp
void Collect_PlayerViews(
    std::vector<REPLICATED_PLAYER_VIEW>& outPlayers) const;
```

규칙:

- source nickname은 `NET_PLAYER_RECORD::strNickName`뿐이다.
- local pending/created nickname을 nameplate에 직접 쓰지 않는다.
- expired character는 출력에서 제외한다.
- 안정적인 draw를 위해 `iNetEntityId`로 정렬할 수 있지만 그 순서를 Party slot으로 사용하지 않는다.

### 7.3 world-to-screen와 draw

`CWorldPlayerNameplateView::Render()`는 각 DTO에 대해 다음을 수행한다.

1. weak character와 transform resolve
2. world position에 presentation 전용 head offset 약 `2.2f` 추가
3. 현재 Engine `VIEW`, `PROJ`, viewport로 clip/screen projection
4. camera 뒤, near/far 밖, viewport 밖이면 skip
5. UTF-8 nickname을 `MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ...)`로 변환
6. `Font_YG330`, centered origin `(0.5f, 0.5f)`로 draw

projection 실패, invalid UTF-8, expired character는 해당 nameplate만 skip한다. `Font_YG330`은 기존 `CMainApp::Ready_Fonts()` 초기화 계약을 사용하며, font tag가 없을 때는 기존 Font Manager의 no-op 경계를 따른다. character, snapshot, input은 건드리지 않는다.

### 7.4 Level consumer

| 파일 | 반영 |
|---|---|
| `Client/Public/Level_Bern.h` | `CWorldPlayerNameplateView` member 추가 |
| `Client/Private/Level_Bern.cpp` | `Render()`의 3D render 성공 뒤 player DTO 수집·nameplate draw |
| `Client/Public/Level_ValtanArena.h` | 동일 member 추가 |
| `Client/Private/Level_ValtanArena.cpp` | 동일 consumer 추가 |

Character Select에는 이번 G에서 nameplate를 그리지 않는다. Bern과 Valtan에서만 모든 replicated player의 nickname을 표시한다.

## 8. 물리 파일 목록

### 8.1 신규

- `C:\Users\user\Desktop\LostArk\Client\Public\WorldPlayerNameplateView.h`
- `C:\Users\user\Desktop\LostArk\Client\Private\WorldPlayerNameplateView.cpp`
- `C:\Users\user\Desktop\LostArk\.md\GB\08-15\2026-08-15_G02_CHARACTER_CREATION_BERN_NAMEPLATE_RESULT.md`

### 8.2 수정

- `Shared/Public/Network/PacketMessages.h`
- `Shared/Private/Network/PacketMessages.cpp`
- `Server/Private/GameRoom.cpp`
- `Client/Public/CharacterSelectionState.h`
- `Client/Private/CharacterSelectionState.cpp`
- `Client/Public/Level_CharacterSelect.h`
- `Client/Private/Level_CharacterSelect.cpp`
- `Client/Public/Level_Lobby.h`
- `Client/Private/Level_Lobby.cpp`
- `Client/Private/Level_Loading.cpp`
- `Client/Private/MainApp.cpp`
- `Client/Public/Level_Bern.h`
- `Client/Private/Level_Bern.cpp`
- `Client/Public/Level_ValtanArena.h`
- `Client/Private/Level_ValtanArena.cpp`
- `Client/Public/NetObjectRegistry.h`
- `Client/Private/NetObjectRegistry.cpp`
- `Client/Public/ClientReplication.h`
- `Client/Private/ClientReplication.cpp`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`
- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj.filters`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/CharacterSelectIsolationHarness/Private/CharacterSelectIsolationHarness.cpp`
- `Tools/Network/Run-CharacterSelectIsolationHarness.ps1`
- `Tools/ValtanFourPlayerHarness/Private/ValtanFourPlayerHarness.cpp`
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`

`AGENTS.md`는 Character Select/Server authority 경계가 바뀌지 않으므로 날짜 로그를 추가하지 않는다. nickname public runtime 계약을 fixed contract에 추가해야 한다고 실제 구현 검토에서 판단된 경우에만 같은 변경에서 최소 문장을 교체한다.

## 9. Visual Studio 프로젝트 등록

신규 nameplate 파일은 다음 두 위치에 모두 등록한다.

`Client/Default/Client.vcxproj`:

```xml
<ClInclude Include="..\Public\WorldPlayerNameplateView.h" />
<ClCompile Include="..\Private\WorldPlayerNameplateView.cpp" />
```

`Client/Default/Client.vcxproj.filters`:

- H/CPP 모두 기존 `02.GameObjects\04. UI` filter
- 새 filter를 만들지 않는다.
- 기존 파일의 filter를 재배치하지 않는다.

`Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`와 `.filters`에도 `WorldPlayerNameplateView.cpp`를 기존 `Client` filter로 등록한다. projection/registry 계약을 실행형으로 검증하므로 Client project에만 등록하고 harness link를 빠뜨리는 반쪽 병합을 허용하지 않는다.

`CharacterSelectionState.cpp`가 Shared nickname validator를 직접 소비하므로 `ClientFrontendHarness.vcxproj`에는 `Shared/Default/Shared.vcxproj` `ProjectReference`도 추가한다. Shared include path만 추가하고 정적 library link를 빠뜨리지 않는다.

이번 실제 폴더 통합에서 물리 H/CPP 6개가 들어왔지만 `.vcxproj/.filters` 등록이 빠져 Client link가 막힌 회귀가 있었다. G02 종료 전에는 G02 신규 파일만 보지 않고 실제 폴더의 모든 changed/untracked C++를 전수 대조한다.

필수 확인:

1. 물리 `.h/.cpp` 존재
2. 해당 project의 `.vcxproj` 항목 존재
3. `.vcxproj.filters` 항목 존재
4. XML parse 성공
5. Debug/Release 실제 Client link 성공

문서나 Solution Explorer에 파일이 보인다는 것만으로 등록 완료 처리하지 않는다.

## 10. Harness와 검증

### 10.1 Shared protocol

`NetworkProtocolHarness`에 추가:

- ASCII nickname round trip
- 한글 nickname round trip
- exact 32-byte boundary
- 33-byte reject
- empty reject
- malformed UTF-8 reject
- leading/trailing whitespace reject
- control code reject
- failed decode가 destination message를 변경하지 않음
- spawn exact nickname echo

packet layout이 바뀌지 않았으므로 protocol version은 유지한다.

### 10.2 Client state

`ClientFrontendHarness`의 기존 `CharacterSelectionState` test를 확장한다.

- invalid stage가 old committed state를 변경하지 않음
- stage 뒤 commit 전에는 old created nickname 유지
- cancel이 selected/created를 보존
- commit이 class/nickname을 atomic replace
- Bern은 created/pending 없으면 resolve 실패
- Valtan은 created가 없으면 audition identity resolve
- created 후 Valtan은 selected class + created nickname resolve
- process-local audition nickname이 반복 호출에서 동일
- registry live pair가 record와 exact character를 함께 반환
- class replacement 뒤 동일 entity와 nickname 유지
- world position projection이 Direct3D clip 범위와 top-left 화면 좌표를 지킴

위 세 묶음은 기존 전체 Effect 회귀와 분리해
`ClientFrontendHarness.exe --g02-nameplate-fast`로 즉시 재실행할 수 있게 한다.

### 10.3 Server/live

`ServerGameplayContractTests` 또는 기존 live harness에서 다음을 검증한다.

- Bern join 후 `SERVER_PLAYER`/spawn exact nickname
- 두 Client가 서로의 exact nickname spawn을 수신
- Character Select private arena에서 각 harness Client가 제출한 nickname/player traffic 격리 유지
- Bern→Valtan transfer 뒤 nickname 유지
- disconnect/re-entry 시 Client가 다시 제출한 created nickname 사용

`CharacterSelectIsolationHarness`의 Bern shared PASS는 player count뿐 아니라 각 socket Client가 제출한 고유 nickname이 정확히 보이는지 검사한다. 한 harness 프로세스 안의 여러 socket은 같은 PID를 공유하므로 여기서는 `Test-<process-id>` 생성 정책을 검사하지 않는다. 그 정책은 `ClientFrontendHarness`에서 검사한다. `--g02-identity-fast`와 runner의 `-G02IdentityFast`는 Character Select private combat/isolation과 Bern shared exact-nickname proof까지만 실행하는 focused Debug/Release gate다. 기본 mode는 Bern→Valtan transfer와 post-transfer command까지 계속 검증한다. 현재 기본 mode의 transfer trigger navigation timeout은 RESULT에 미완료로 기록한다.

`ValtanFourPlayerHarness`는 각 Client가 제출한 label을 자기 `S2C_PLAYER_SPAWNED::strNickName`과 대조한다. 마지막 source 문구 수정 뒤 Debug/Release를 다시 빌드하고 wrapper를 각각 실행해 initial/second generation exact nickname을 포함한 4개 assertion과 `failures : 0`을 확인했다. direct Valtan four-player nickname 회귀는 automated PASS다.

### 10.4 빌드 순서

실제 폴더에서 다음 순서로 실행한다.

1. `powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1`
2. Shared x64 Debug/Release
3. NetworkProtocolHarness x64 Debug/Release + 실행
4. Server x64 Debug/Release
5. `Server.exe --contract-test` Debug/Release
6. ClientFrontendHarness x64 Debug/Release + `--g02-nameplate-fast` 실행
7. CharacterSelectIsolationHarness x64 Debug/Release + owned alternate port에서 `-G02IdentityFast` 실행
8. Client x64 Debug/Release 실제 link
9. `powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`
10. 같은 Release regression
11. `git diff --check`

에이전트는 Client/UI를 자율 실행하지 않는다. visual nameplate 판정은 사용자가 직접 한다.

### 10.5 사용자 수동 경로

1. `Framework.sln`에서 `Server + Client` profile 확인
2. `Ctrl+F5`
3. Lobby -> Character Select
4. class 선택
5. 몬스터 spawn과 skill/damage가 계속 Server 기준인지 확인
6. Create Character -> 한글 또는 영문 nickname 입력 -> 확인
7. Bern 자동 진입
8. 자기 캐릭터 머리 위 exact nickname 확인
9. 두 번째 Client로 Bern 입장 후 양쪽 nameplate 확인
10. Character Select에서 Create 없이 `Enter Valtan Map` -> `Test-<process-id>` 이름과 전투 확인
11. created nickname이 있는 Client는 Valtan에서 같은 이름 확인

수동 결과는 사용자의 서면 관찰 전까지 `visual PASS`로 기록하지 않는다.

## 11. 적용 순서

하나의 큰 수정으로 동시에 열지 않고 아래 checkpoint로 진행한다.

### G02-01 — Identity contract

- Shared nickname validator
- CharacterSelectionState pending/created/audition
- NetworkProtocolHarness + ClientFrontendHarness
- Shared/ClientFrontend Debug/Release PASS

### G02-02 — Create -> Bern transaction

- Character Select modal
- safe `Enter_Stage()` ordering
- Lobby identity consume
- Loading/Bern commit/rollback
- Server contract + live nickname echo

### G02-03 — Nameplate

- registry live pair
- replication read-only DTO
- WorldPlayerNameplateView
- Bern/Valtan consumer
- Client project/filter 등록
- Debug/Release Client link
- 사용자 manual visual

각 checkpoint는 이전 checkpoint가 build/harness PASS인 상태에서 시작한다. 마지막 RESULT에는 자동 검증과 사용자 visual 검증을 분리한다.

## 12. 다음 단계와 기존 전체 계획 교정

G02 뒤 우선순위는 다음과 같다.

1. remote player screen-space hover와 RMB input consume
2. Server-authoritative Invite/Cancel과 max-4 Party roster
3. fake Party seed 제거와 Party resource 기반 HP UI
4. ROOM/PARTY chat
5. action timing에 맞춘 Server collider/damage와 authored effect cue 교차검증
6. Valtan pattern/animation/effect/camera sequence
7. 현재 transport 4-player 회귀를 보존한 IOCP 교체
8. 4 real Client raid integration/soak

Party G에서 nameplate DTO를 재사용할 수는 있지만 G02 nameplate가 아직 존재하지 않는 Party sink나 Party protocol을 include해서는 안 된다.

기존 전체 계획을 갱신할 때는 다음을 반영한다.

- G00과 G01은 실제 폴더 기준 CLOSED
- G04의 four-player baseline은 G00 완료 기준선의 별칭이며 재구현하지 않음
- Character Select는 local simulation이 아니라 session-private Server simulation
- Bern/Valtan/Training은 world-shared
- final Party/Chat image UI는 stable layout ID와 screen-space input routing을 사용
- effect cue는 authored animation timing을 유지하고 Server collider/damage authority와 publisher에서 교차검증
- 모든 이후 G 구현 위치도 실제 폴더 하나로 고정

## 13. G02 완료 정의

다음이 모두 참일 때만 G02를 완료로 기록한다.

- Create Character confirm이 selected class + exact nickname을 stage한다.
- Server-approved Bern entry가 hardcoded `"Player"`를 사용하지 않는다.
- Bern resource 생성, profile 활성화, 실제 `Change_Level` 성공 뒤에만 created identity가 commit된다.
- 모든 실패 경로에서 이전 created identity가 보존된다.
- Server player와 spawn echo가 exact nickname을 소유한다.
- Bern↔Valtan transfer가 nickname을 유지한다.
- Bern/Valtan nameplate가 Server-replicated nickname만 표시한다.
- direct Valtan test entry가 created 또는 audition identity로 동작한다.
- 신규/변경 C++의 project/filter 등록 누락이 0개다.
- Shared, Server, Client, harness Debug/Release가 모두 link/run PASS다.
- 사용자 manual visual 결과는 자동 PASS와 별도로 기록한다.
