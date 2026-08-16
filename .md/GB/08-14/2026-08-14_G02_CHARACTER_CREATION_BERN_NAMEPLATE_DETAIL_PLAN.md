# 2026-08-14 G02 Character Creation, Bern Entry, Nameplate 코드 상세 계획서

## 0. 문서 역할과 적용 경계

이 문서는 다음 구현 계획의 직접 반영용 코드 문서다.

- 개념·범위 정본: `2026-08-14_G02_CHARACTER_CREATION_BERN_NAMEPLATE_IMPLEMENTATION_PLAN.md`
- 실제 구현 위치: `C:\Users\user\Desktop\LostArk`
- 구현 상태: **IMPLEMENTED / AUTOMATED PASS / PARTIAL MANUAL PENDING**
- 실제 적용 결과와 미완료 검증은
  `.md/GB/08-15/2026-08-15_G02_CHARACTER_CREATION_BERN_NAMEPLATE_RESULT.md`를 따른다.

현재 실제 폴더에는 Effect 세션의 대규모 미커밋 변경이 있다. 특히 `MainApp.cpp`, `GameRoom.cpp`, Shared codec, 두 harness는 다른 변경과 겹친다. 따라서 기존 파일 전문을 복사해 덮어쓰지 않는다. 이 문서는 다음 형식을 사용한다.

- 신규 H/CPP: 처음부터 끝까지 전문 제공
- 짧고 현재 clean인 기존 H/CPP: 적용 후 전문 제공
- 다른 세션과 겹치는 기존 파일: 정확한 기준점과 완전한 함수 교체 블록 제공
- project/filter: 기존 항목 바로 아래에 넣을 완전한 XML block 제공

이는 다른 세션의 미커밋 변경을 보존해야 한다는 저장소 상위 규칙을 우선한 것이다. 각 block을 반영하기 직전에 기준점이 현재 파일에 한 번만 존재하는지 확인한다.

G02는 새 protocol packet을 만들지 않는다. 기존 `C2S_ENTER_WORLD::strNickName`과 `S2C_PLAYER_SPAWNED::strNickName`을 사용하므로 `NETWORK_PROTOCOL_VERSION`도 올리지 않는다. 새 harness project도 만들지 않는다.

## 1. 적용 후 호출 흐름

```text
Character Select Server Arena
  -> class 선택은 기존 Server class-change/snapshot
  -> Create Character
  -> nickname modal
  -> Shared validator
  -> CharacterSelectionState pending creation
  -> Lobby command + Lobby load 성공
  -> 기존 Character Select Level destructor가 old socket/presentation 정리
  -> Lobby가 pending class/nickname으로 C2S_ENTER_WORLD(BERN)
  -> Server GameRoom이 동일 validator로 확인하고 SERVER_PLAYER에 저장
  -> S2C_PLAYER_SPAWNED가 exact nickname을 Bern 모든 Client에 복제
  -> Loading resource 생성
  -> rendering profile 활성화
  -> Change_Level(BERN) 성공
  -> MainApp가 pending identity를 created identity로 commit
  -> ClientReplication record
  -> read-only replicated player view
  -> Bern/Valtan SpriteFont nameplate
```

Direct Valtan은 다음처럼 동작한다.

```text
created identity 있음  -> selected class + created nickname
created identity 없음  -> selected class 또는 Lance Master + Test-<process-id>
```

## 2. 물리 파일 목록

### 2.1 신규

- `Client/Public/WorldPlayerNameplateView.h`
- `Client/Private/WorldPlayerNameplateView.cpp`
- 구현·검증 후 `2026-08-14_G02_CHARACTER_CREATION_BERN_NAMEPLATE_RESULT.md`

### 2.2 수정

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
- `Client/Public/NetObjectRegistry.h`
- `Client/Private/NetObjectRegistry.cpp`
- `Client/Public/ClientReplication.h`
- `Client/Private/ClientReplication.cpp`
- `Client/Public/Level_Bern.h`
- `Client/Private/Level_Bern.cpp`
- `Client/Public/Level_ValtanArena.h`
- `Client/Private/Level_ValtanArena.cpp`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`
- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`
- `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj.filters`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/CharacterSelectIsolationHarness/Private/CharacterSelectIsolationHarness.cpp`
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`

## 3. G02-01 — 공통 nickname 계약과 process identity

### 3.1 `Shared/Public/Network/PacketMessages.h`

작업: 추가

기준점: 기존 `#include <string>` 바로 아래

```cpp
#include <string_view>
```

기준점: `class CPacketWriter;` 바로 아래

```cpp
	[[nodiscard]] bool Is_Valid_PlayerNickname(
		std::string_view nickname) noexcept;
```

이 함수는 Client modal, Shared codec, Server entry가 함께 쓰는 단일 validator다.

### 3.2 `Shared/Private/Network/PacketMessages.cpp`

기준점: anonymous namespace의 첫 validator 앞에 다음 두 helper를 추가한다.

```cpp
	bool Is_Utf8Continuation(const std::uint8_t value) noexcept
	{
		return 0x80u == (value & 0xC0u);
	}

	bool Is_AsciiWhitespace(const char value) noexcept
	{
		return ' ' == value || '\t' == value || '\n' == value ||
			'\r' == value || '\f' == value || '\v' == value;
	}
```

기준점: anonymous namespace를 닫는 `}` 바로 뒤, 첫 `Write_Message(C2S_ENTER_WORLD)` 앞에 다음 정의를 추가한다.

```cpp
bool LostArk::Shared::Is_Valid_PlayerNickname(
	const std::string_view nickname) noexcept
{
	if (nickname.empty() || nickname.size() > MAX_NICKNAME_BYTES ||
		Is_AsciiWhitespace(nickname.front()) ||
		Is_AsciiWhitespace(nickname.back()))
	{
		return false;
	}

	const auto* bytes = reinterpret_cast<const std::uint8_t*>(
		nickname.data());
	std::size_t offset = 0u;
	while (offset < nickname.size())
	{
		const std::uint8_t first = bytes[offset];
		std::uint32_t codePoint = 0u;
		std::size_t length = 0u;

		if (first <= 0x7Fu)
		{
			codePoint = first;
			length = 1u;
		}
		else if (first >= 0xC2u && first <= 0xDFu)
		{
			if (offset + 1u >= nickname.size() ||
				!Is_Utf8Continuation(bytes[offset + 1u]))
			{
				return false;
			}
			codePoint = ((first & 0x1Fu) << 6u) |
				(bytes[offset + 1u] & 0x3Fu);
			length = 2u;
		}
		else if (first >= 0xE0u && first <= 0xEFu)
		{
			if (offset + 2u >= nickname.size() ||
				!Is_Utf8Continuation(bytes[offset + 1u]) ||
				!Is_Utf8Continuation(bytes[offset + 2u]) ||
				(0xE0u == first && bytes[offset + 1u] < 0xA0u) ||
				(0xEDu == first && bytes[offset + 1u] > 0x9Fu))
			{
				return false;
			}
			codePoint = ((first & 0x0Fu) << 12u) |
				((bytes[offset + 1u] & 0x3Fu) << 6u) |
				(bytes[offset + 2u] & 0x3Fu);
			length = 3u;
		}
		else if (first >= 0xF0u && first <= 0xF4u)
		{
			if (offset + 3u >= nickname.size() ||
				!Is_Utf8Continuation(bytes[offset + 1u]) ||
				!Is_Utf8Continuation(bytes[offset + 2u]) ||
				!Is_Utf8Continuation(bytes[offset + 3u]) ||
				(0xF0u == first && bytes[offset + 1u] < 0x90u) ||
				(0xF4u == first && bytes[offset + 1u] > 0x8Fu))
			{
				return false;
			}
			codePoint = ((first & 0x07u) << 18u) |
				((bytes[offset + 1u] & 0x3Fu) << 12u) |
				((bytes[offset + 2u] & 0x3Fu) << 6u) |
				(bytes[offset + 3u] & 0x3Fu);
			length = 4u;
		}
		else
		{
			return false;
		}

		if (0u == codePoint || codePoint <= 0x1Fu ||
			(codePoint >= 0x7Fu && codePoint <= 0x9Fu) ||
			(codePoint >= 0xD800u && codePoint <= 0xDFFFu) ||
			codePoint > 0x10FFFFu)
		{
			return false;
		}
		offset += length;
	}
	return true;
}
```

다음 네 codec의 기존 empty/length 조건을 모두 같은 조건으로 교체한다.

```cpp
	if (!Is_Valid_PlayerNickname(message.strNickName))
		return false;
```

```cpp
	if (!Is_Valid_PlayerNickname(nickName))
		return false;
```

```cpp
	if (!Is_Valid_PlayerNickname(spawned.strNickName))
		return false;
```

```cpp
	if (!Is_Valid_PlayerNickname(nickName))
		return false;
```

대상 함수는 순서대로 다음과 같다.

- `Write_Message(CPacketWriter&, const C2S_ENTER_WORLD&)`
- `Read_Message(CPacketReader&, C2S_ENTER_WORLD&)`
- `Write_Message(CPacketWriter&, const S2C_PLAYER_SPAWNED&)`
- `Read_Message(CPacketReader&, S2C_PLAYER_SPAWNED&)`

Read 함수는 local 변수에 전부 읽고 validator까지 성공한 뒤 마지막에 destination message를 교체하는 현재 transaction 순서를 유지한다.

### 3.3 `Server/Private/GameRoom.cpp`

기준점: anonymous `Is_Valid_EnterWorld()` 전체 교체

```cpp
	bool Is_Valid_EnterWorld(const C2S_ENTER_WORLD& message)
	{
		return message.iProtocolVersion == NETWORK_PROTOCOL_VERSION &&
			Is_Known_World_Id(message.eWorldId) &&
			Is_Supported_Playable_Character_Class(message.eCharacterClass) &&
			Is_Valid_PlayerNickname(message.strNickName);
	}
```

별도 nickname packet이나 nickname reject packet은 만들지 않는다. 잘못된 wire payload는 기존 protocol fault/connection close 경계를 따른다.

### 3.4 `Client/Public/CharacterSelectionState.h` 적용 후 전문

```cpp
#pragma once

#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <string>
#include <string_view>

NS_BEGIN(Client)

enum class CHARACTER_ENTRY_IDENTITY_SOURCE
{
	AUDITION,
	CREATED,
	PENDING_CREATION,
	END
};

struct CHARACTER_ENTRY_IDENTITY final
{
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string strNickname;
	CHARACTER_ENTRY_IDENTITY_SOURCE eSource =
		CHARACTER_ENTRY_IDENTITY_SOURCE::END;
};

class CCharacterSelectionState final
{
public:
	static bool_t Select(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	static bool_t Has_Selection();
	static bool_t Try_Get_SelectedClass(
		LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass);
	static bool_t Stage_Creation(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass,
		std::string_view nickname);
	static bool_t Has_PendingCreation();
	static bool_t Commit_PendingCreation();
	static void Cancel_PendingCreation();
	static bool_t Try_Resolve_ForWorld(
		LostArk::Shared::WORLD_ID worldId,
		CHARACTER_ENTRY_IDENTITY& outIdentity);
};

NS_END
```

### 3.5 `Client/Private/CharacterSelectionState.cpp` 적용 후 전문

```cpp
#include "CharacterSelectionState.h"

#include "Network/PacketMessages.h"

#include <Windows.h>

#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace
{
	struct PENDING_CHARACTER_CREATION final
	{
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strNickname;
	};

	std::mutex g_SelectionMutex;
	std::optional<LostArk::Shared::CHARACTER_CLASS_ID> g_SelectedClass;
	std::optional<std::string> g_CreatedNickname;
	std::optional<PENDING_CHARACTER_CREATION> g_PendingCreation;

	const std::string& Get_AuditionNickname()
	{
		static const std::string nickname =
			"Test-" + std::to_string(GetCurrentProcessId());
		return nickname;
	}
}

bool_t Client::CCharacterSelectionState::Select(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
		characterClass))
	{
		return false;
	}

	std::scoped_lock lock{ g_SelectionMutex };
	g_SelectedClass = characterClass;
	return true;
}

bool_t Client::CCharacterSelectionState::Has_Selection()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_SelectedClass.has_value();
}

bool_t Client::CCharacterSelectionState::Try_Get_SelectedClass(
	LostArk::Shared::CHARACTER_CLASS_ID& outCharacterClass)
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (!g_SelectedClass.has_value())
		return false;

	outCharacterClass = *g_SelectedClass;
	return true;
}

bool_t Client::CCharacterSelectionState::Stage_Creation(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass,
	const std::string_view nickname)
{
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
			characterClass) ||
		!LostArk::Shared::Is_Valid_PlayerNickname(nickname))
	{
		return false;
	}

	PENDING_CHARACTER_CREATION staged{};
	staged.eCharacterClass = characterClass;
	staged.strNickname.assign(nickname);

	std::scoped_lock lock{ g_SelectionMutex };
	g_PendingCreation = std::move(staged);
	return true;
}

bool_t Client::CCharacterSelectionState::Has_PendingCreation()
{
	std::scoped_lock lock{ g_SelectionMutex };
	return g_PendingCreation.has_value();
}

bool_t Client::CCharacterSelectionState::Commit_PendingCreation()
{
	std::scoped_lock lock{ g_SelectionMutex };
	if (!g_PendingCreation.has_value())
		return false;

	g_SelectedClass = g_PendingCreation->eCharacterClass;
	g_CreatedNickname = std::move(g_PendingCreation->strNickname);
	g_PendingCreation.reset();
	return true;
}

void Client::CCharacterSelectionState::Cancel_PendingCreation()
{
	std::scoped_lock lock{ g_SelectionMutex };
	g_PendingCreation.reset();
}

bool_t Client::CCharacterSelectionState::Try_Resolve_ForWorld(
	const LostArk::Shared::WORLD_ID worldId,
	CHARACTER_ENTRY_IDENTITY& outIdentity)
{
	using namespace LostArk::Shared;

	std::scoped_lock lock{ g_SelectionMutex };
	CHARACTER_ENTRY_IDENTITY staged{};

	switch (worldId)
	{
	case WORLD_ID::CHARACTER_SELECT_ARENA:
	case WORLD_ID::TRAINING_GROUND:
		staged.eCharacterClass = g_SelectedClass.value_or(
			CHARACTER_CLASS_ID::LANCE_MASTER);
		staged.strNickname = Get_AuditionNickname();
		staged.eSource = CHARACTER_ENTRY_IDENTITY_SOURCE::AUDITION;
		break;

	case WORLD_ID::BERN:
		if (g_PendingCreation.has_value())
		{
			staged.eCharacterClass = g_PendingCreation->eCharacterClass;
			staged.strNickname = g_PendingCreation->strNickname;
			staged.eSource =
				CHARACTER_ENTRY_IDENTITY_SOURCE::PENDING_CREATION;
		}
		else if (g_SelectedClass.has_value() &&
			g_CreatedNickname.has_value())
		{
			staged.eCharacterClass = *g_SelectedClass;
			staged.strNickname = *g_CreatedNickname;
			staged.eSource = CHARACTER_ENTRY_IDENTITY_SOURCE::CREATED;
		}
		else
		{
			return false;
		}
		break;

	case WORLD_ID::VALTAN_ARENA:
		staged.eCharacterClass = g_SelectedClass.value_or(
			CHARACTER_CLASS_ID::LANCE_MASTER);
		if (g_CreatedNickname.has_value())
		{
			staged.strNickname = *g_CreatedNickname;
			staged.eSource = CHARACTER_ENTRY_IDENTITY_SOURCE::CREATED;
		}
		else
		{
			staged.strNickname = Get_AuditionNickname();
			staged.eSource = CHARACTER_ENTRY_IDENTITY_SOURCE::AUDITION;
		}
		break;

	default:
		return false;
	}

	if (!Is_Supported_Playable_Character_Class(
			staged.eCharacterClass) ||
		!Is_Valid_PlayerNickname(staged.strNickname))
	{
		return false;
	}

	outIdentity = std::move(staged);
	return true;
}
```

## 4. G02-02 — Create Character modal과 Bern entry transaction

### 4.1 `Client/Public/Level_CharacterSelect.h`

기준점: private `Enter_Stage` 선언 바로 앞

```cpp
	void Open_CreateCharacterModal();
	bool_t Confirm_CreateCharacter();
	void Cancel_CreateCharacter();
	void Render_CreateCharacterModal();
```

기준점: `m_ArenaSpawnAccepted` 바로 아래

```cpp
	std::array<char_t,
		LostArk::Shared::MAX_NICKNAME_BYTES + 1u> m_NicknameDraft{};
	bool_t m_isCreateCharacterModalOpen = false;
```

### 4.2 `Client/Private/Level_CharacterSelect.cpp`

추가 include:

```cpp
#include "Network/PacketMessages.h"
```

anonymous namespace의 미사용 `PLAYER_NICKNAME = "Player"`는 삭제한다.

다음 네 함수는 `Request_SelectedArenaSpawn()`과 `Enter_Stage()` 사이에 추가한다.

```cpp
void CLevel_CharacterSelect::Open_CreateCharacterModal()
{
	if (MODE::SERVER_ARENA != m_eMode ||
		m_iSelectedClassIndex >= SUPPORTED_CLASSES.size() ||
		m_iPendingClassIndex.has_value() ||
		CLevelTransitionService::Is_Pending())
	{
		m_strStatus =
			"Character creation is unavailable while another action is pending.";
		return;
	}

	m_isCreateCharacterModalOpen = true;
	ImGui::OpenPopup("Create Character");
}

bool_t CLevel_CharacterSelect::Confirm_CreateCharacter()
{
	if (MODE::SERVER_ARENA != m_eMode ||
		m_iSelectedClassIndex >= SUPPORTED_CLASSES.size())
	{
		m_strStatus = "The selected class is unavailable.";
		return false;
	}

	const std::string nickname{ m_NicknameDraft.data() };
	if (!LostArk::Shared::Is_Valid_PlayerNickname(nickname))
	{
		m_strStatus =
			"Nickname must be valid UTF-8, contain 1-32 bytes, and contain no control or edge whitespace.";
		return false;
	}

	if (!CCharacterSelectionState::Stage_Creation(
		SUPPORTED_CLASSES[m_iSelectedClassIndex], nickname))
	{
		m_strStatus = "The character identity could not be staged.";
		return false;
	}

	if (!Enter_Stage(LOBBY_STAGE::BERN))
	{
		CCharacterSelectionState::Cancel_PendingCreation();
		return false;
	}
	return true;
}

void CLevel_CharacterSelect::Cancel_CreateCharacter()
{
	CCharacterSelectionState::Cancel_PendingCreation();
	m_isCreateCharacterModalOpen = false;
	ImGui::CloseCurrentPopup();
}

void CLevel_CharacterSelect::Render_CreateCharacterModal()
{
	if (!m_isCreateCharacterModalOpen)
		return;

	if (!ImGui::BeginPopupModal(
		"Create Character",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings))
	{
		return;
	}

	ImGui::TextUnformatted("Enter a nickname for Bern.");
	const bool_t confirmFromEnter = ImGui::InputText(
		"Nickname",
		m_NicknameDraft.data(),
		m_NicknameDraft.size(),
		ImGuiInputTextFlags_EnterReturnsTrue);

	const bool_t confirmFromButton =
		ImGui::Button("Create and Enter Bern");
	ImGui::SameLine();
	const bool_t cancel = ImGui::Button("Cancel");

	if (cancel)
	{
		Cancel_CreateCharacter();
	}
	else if ((confirmFromEnter || confirmFromButton) &&
		Confirm_CreateCharacter())
	{
		m_isCreateCharacterModalOpen = false;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}
```

기존 `Enter_Stage()` 전체 교체:

```cpp
bool_t CLevel_CharacterSelect::Enter_Stage(const LOBBY_STAGE stage)
{
	const char_t* stageName = Get_StageName(stage);
	const char_t* transitionSource = Get_StageTransitionSource(stage);
	if (MODE::SERVER_ARENA != m_eMode || nullptr == transitionSource ||
		m_iSelectedClassIndex >= SUPPORTED_CLASSES.size() ||
		m_iPendingClassIndex.has_value() ||
		CLevelTransitionService::Is_Pending())
	{
		m_strStatus = "The selected stage is not available here.";
		return false;
	}
	if (!CCharacterSelectionState::Select(
		SUPPORTED_CLASSES[m_iSelectedClassIndex]))
	{
		m_strStatus = "The selected class could not be preserved for entry.";
		return false;
	}

	LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
	if (!CLobbyCommandService::Request(stage, token))
	{
		m_strStatus = CLobbyCommandService::Get_Status();
		return false;
	}
	if (!CLevelTransitionService::Request_Load(
		LEVEL::LOBBY, transitionSource, token))
	{
		CLobbyCommandService::Cancel(
			token, "Lobby load request was rejected");
		m_strStatus = CLevelTransitionService::Get_Status();
		return false;
	}

	m_eMode = MODE::RETURNING_TO_LOBBY;
	m_strStatus = std::string("Lobby will request ") +
		stageName + " from Server.";
	return true;
}
```

이 함수에서 기존의 다음 선행 teardown은 전부 삭제한다.

```cpp
CAnimationTargetService::Unbind(m_pActiveCharacter);
m_pActiveCharacter.reset();
CNetworkManager::Get().Close_ServerConnection();
m_Replication.Reset();
m_PlayerController.Set_LocalCharacter(nullptr);
```

Level 교체가 실제로 성공하면 기존 destructor가 동일 정리를 수행한다. request 실패 시 현재 Server arena가 살아 있어야 한다.

나머지 정확한 교체:

- `Request_ClassChange()` 첫 guard에 `m_isCreateCharacterModalOpen`을 추가한다.
- `Request_SelectedArenaSpawn()` 첫 guard에 `m_isCreateCharacterModalOpen`을 추가한다.
- `Update_ServerArena()`의 `m_PlayerController.Update(...)`는 modal이 닫힌 경우에만 호출한다.
- `Render_ClassList()`의 `bInteractable` 조건에 `!m_isCreateCharacterModalOpen`을 추가한다.
- `Render_SelectionPanel()`의 `Enter Bern` 버튼을 다음으로 교체한다.

```cpp
	if (ImGui::Button("Create Character"))
		Open_CreateCharacterModal();
```

- Bern/Valtan/Back button group의 disabled 조건에 `m_isCreateCharacterModalOpen`을 추가한다.
- `Render_SelectionPanel()`의 `ImGui::End()` 바로 전에 다음 호출을 둔다. 이 호출은 popup ID stack을 유지하기 위해 Character Select window 안에 있어야 한다.

```cpp
	Render_CreateCharacterModal();
```

### 4.3 `Client/Public/Level_Lobby.h`

기준점: `m_ePendingPurpose` 바로 아래

```cpp
	bool_t m_hasPendingCharacterCreationEntry = false;
```

### 4.4 `Client/Private/Level_Lobby.cpp`

anonymous namespace에서 다음 기존 항목을 삭제한다.

- `PLAYER_NICKNAME[] = "Player"`
- `DEFAULT_ENTRY_CLASS`
- `Resolve_EntryCharacterClass(...)`

`Update()`의 command consume block 교체:

```cpp
	LOBBY_COMMAND command{};
	if (CLobbyCommandService::Try_Consume(command) &&
		!Begin_StageRequest(command) &&
		LOBBY_STAGE::BERN == command.eStage)
	{
		CCharacterSelectionState::Cancel_PendingCreation();
	}
```

`Begin_NetworkEntry()`에서 purpose 검증 블록 뒤부터 함수 끝까지 교체:

```cpp
	CHARACTER_ENTRY_IDENTITY identity{};
	if (!CCharacterSelectionState::Try_Resolve_ForWorld(
		eWorldId, identity))
	{
		m_strStatus = LostArk::Shared::WORLD_ID::BERN == eWorldId ?
			"Create a character before entering Bern." :
			"The entry identity could not be resolved.";
		return false;
	}
	const bool_t usesPendingCreation =
		CHARACTER_ENTRY_IDENTITY_SOURCE::PENDING_CREATION ==
		identity.eSource;

	CNetworkManager& networkManager = CNetworkManager::Get();
	networkManager.Close_ServerConnection();
	const string serverHost =
		LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE == purpose ?
		CNetworkManager::Resolve_MapEditorServerHost() :
		CNetworkManager::Resolve_ServerHost();
	if (!networkManager.Connect_To_Server(
		serverHost,
		CNetworkManager::DEFAULT_SERVER_PORT))
	{
		if (usesPendingCreation)
			CCharacterSelectionState::Cancel_PendingCreation();
		m_strStatus = "Server connection failed for " +
			serverHost + ":" +
			to_string(CNetworkManager::DEFAULT_SERVER_PORT) + " (WSA " +
			to_string(networkManager.Get_LastErrorCode()) + ").";
		return false;
	}

	if (!networkManager.Send_EnterWorld(
		eWorldId,
		identity.eCharacterClass,
		identity.strNickname))
	{
		if (usesPendingCreation)
			CCharacterSelectionState::Cancel_PendingCreation();
		m_strStatus = "C2S_ENTER_WORLD send failed (WSA " +
			to_string(networkManager.Get_LastErrorCode()) + ").";
		networkManager.Close_ServerConnection();
		return false;
	}

	m_eEntryState = ENTRY_STATE::WAITING_FOR_APPROVAL;
	m_ePendingWorldId = eWorldId;
	m_ePendingLevel = eTargetLevel;
	m_ePendingPurpose = purpose;
	m_hasPendingCharacterCreationEntry = usesPendingCreation;
	m_ApprovalDeadline =
		std::chrono::steady_clock::now() + std::chrono::seconds(5);
	m_strStatus = "C2S_ENTER_WORLD sent for " +
		identity.strNickname + ". Waiting for server approval.";
	return true;
}
```

`Cancel_PendingEntry()` 전체 교체:

```cpp
void CLevel_Lobby::Cancel_PendingEntry(const string& reason)
{
	CNetworkManager::Get().Close_ServerConnection();
	if (m_hasPendingCharacterCreationEntry)
		CCharacterSelectionState::Cancel_PendingCreation();
	m_eEntryState = ENTRY_STATE::IDLE;
	m_ePendingWorldId = LostArk::Shared::WORLD_ID::END;
	m_ePendingLevel = LEVEL::END;
	m_ePendingPurpose = LOBBY_COMMAND_PURPOSE::GAMEPLAY;
	m_hasPendingCharacterCreationEntry = false;
	m_ApprovalDeadline = {};
	m_strStatus = reason;
}
```

`Consume_EnterAccepted()`의 성공 cleanup에 다음을 추가한다. 전역 pending은 여기서 commit하거나 취소하지 않는다.

```cpp
	m_hasPendingCharacterCreationEntry = false;
```

`Render_StagePanel()`의 기존 default class 표시 block 교체:

```cpp
	CHARACTER_ENTRY_IDENTITY entryIdentity{};
	const bool_t hasEntryIdentity =
		CCharacterSelectionState::Try_Resolve_ForWorld(
			LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA,
			entryIdentity);
	ImGui::Text(
		"Entry character: %s",
		hasEntryIdentity ?
			Get_CharacterClassName(entryIdentity.eCharacterClass) :
			"Unavailable");
	if (hasEntryIdentity)
	{
		ImGui::TextDisabled(
			"Entry nickname: %s",
			entryIdentity.strNickname.c_str());
	}
```

기존 `hasExplicitSelection`과 `Direct entry commits Lance Master` 문구는 삭제한다. Lobby Bern button은 created/pending identity가 없으면 `Begin_NetworkEntry()`가 실패하고 Lobby를 유지한다.

### 4.5 `Client/Private/Level_Loading.cpp`

추가 include:

```cpp
#include "CharacterSelectionState.h"
```

`Recover_FromFailure()`의 `m_isFailureReported = true;` 바로 뒤에 추가:

```cpp
	CCharacterSelectionState::Cancel_PendingCreation();
```

### 4.6 `Client/Private/MainApp.cpp`

이 파일은 Effect 세션과 겹치므로 함수 전체 덮어쓰기를 금지한다.

추가 include:

```cpp
#include "CharacterSelectionState.h"
```

`Apply_LevelRequest()` LOAD 분기의 `FAILED(result)` 첫 줄에 추가:

```cpp
			CCharacterSelectionState::Cancel_PendingCreation();
```

ACTIVATE 성공 판정 block을 다음으로 교체한다.

```cpp
	const bool_t levelChanged = profileActivated &&
		SUCCEEDED(CGameInstance::Get().Change_Level(
			ETOUI(request.eTargetLevel), move(nextLevel)));
	if (levelChanged)
	{
		CEffectPresentationService::Clear_Level(iPreviousLevel);
		if (LEVEL::BERN == request.eTargetLevel &&
			CCharacterSelectionState::Has_PendingCreation() &&
			!CCharacterSelectionState::Commit_PendingCreation())
		{
			OutputDebugStringA(
				"[MainApp] Bern identity commit invariant failed.\n");
			CNetworkManager::Get().Close_ServerConnection();
			CLevelTransitionService::Report_LoadFailure(E_FAIL);
			if (!CLevelTransitionService::Request_Load(
				LEVEL::LOBBY,
				"main-app.identity-commit-failure"))
			{
				OutputDebugStringA(
					"[MainApp] Failed to stage Lobby after identity commit failure.\n");
			}
			return;
		}
		return;
	}
```

기존 activation failure cleanup에서 `CLobbyCommandService::Cancel(...)` 앞에 추가:

```cpp
	CCharacterSelectionState::Cancel_PendingCreation();
```

`Level_Bern::Initialize()`에서는 identity를 commit하지 않는다.

## 5. G02-03 — replicated player nameplate

### 5.1 `Client/Public/NetObjectRegistry.h`

기준점: `NET_PLAYER_RECORD` 바로 아래

```cpp
	// Server record와 실제 presentation을 잃지 않고 묶어 읽는 snapshot이다.
	// Registry의 owner 상태가 아니며, 반환 vector가 살아 있는 동안만 Character 수명을 붙든다.
	struct LIVE_NET_PLAYER
	{
		NET_PLAYER_RECORD Record;
		std::shared_ptr<CCharacter> pCharacter;
	};
```

기준점: public `Get_LiveObjects() const;` 바로 아래

```cpp
		// Level/UI consumer가 stable record와 살아 있는 presentation을 함께 읽는다.
		std::vector<LIVE_NET_PLAYER> Get_LivePlayers() const;
```

### 5.2 `Client/Private/NetObjectRegistry.cpp`

기존 `<cstdint>` 바로 아래에 추가:

```cpp
#include <utility>
```

기준점: 기존 `Get_LiveObjects()` 정의 바로 아래

```cpp
std::vector<Client::LIVE_NET_PLAYER>
Client::CNetObjectRegistry::Get_LivePlayers() const
{
	std::vector<LIVE_NET_PLAYER> players;
	players.reserve(m_HandleByEntityId.size());

	for (const SLOT& slot : m_Slots)
	{
		if (!slot.isOccupied)
			continue;
		if (std::shared_ptr<CCharacter> character = slot.pCharacter.lock())
		{
			LIVE_NET_PLAYER player{};
			player.Record = slot.record;
			player.pCharacter = std::move(character);
			players.push_back(std::move(player));
		}
	}

	return players;
}
```

### 5.3 `Client/Public/ClientReplication.h`

STL include에 추가:

```cpp
#include <vector>
```

기준점: `class CValtan;` 바로 아래, `CClientReplication` 앞

```cpp
	// Level presentation이 읽는 복제 player snapshot이다. NetEntityId가 identity이며
	// nickname과 class는 Server spawn record에서만 오고 Character 수명은 소유하지 않는다.
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

기준점: public `Get_LocalCharacter() const;` 바로 아래

```cpp
		void Collect_PlayerViews(
			std::vector<REPLICATED_PLAYER_VIEW>& outPlayers) const;
```

### 5.4 `Client/Private/ClientReplication.cpp`

STL include에 추가:

```cpp
#include <algorithm>
```

기준점: `Get_LocalCharacter() const` 정의 바로 아래

```cpp
void Client::CClientReplication::Collect_PlayerViews(
	std::vector<REPLICATED_PLAYER_VIEW>& outPlayers) const
{
	const std::vector<LIVE_NET_PLAYER> livePlayers =
		m_Registry.Get_LivePlayers();
	const std::shared_ptr<CCharacter> localCharacter =
		m_Registry.Resolve(m_LocalCharacterHandle);

	outPlayers.resize(livePlayers.size());
	for (std::size_t index = 0u; index < livePlayers.size(); ++index)
	{
		const LIVE_NET_PLAYER& source = livePlayers[index];
		REPLICATED_PLAYER_VIEW& target = outPlayers[index];
		target.iPlayerId = source.Record.iPlayerId;
		target.iNetEntityId = source.Record.iNetEntityId;
		target.eCharacterClass = source.Record.eCharacterClass;
		target.strNickname = source.Record.strNickName;
		target.isLocal = nullptr != localCharacter &&
			source.pCharacter == localCharacter;
		target.pCharacter = source.pCharacter;
	}

	std::sort(
		outPlayers.begin(),
		outPlayers.end(),
		[](const REPLICATED_PLAYER_VIEW& left,
			const REPLICATED_PLAYER_VIEW& right)
		{
			return left.iNetEntityId < right.iNetEntityId;
		});
}
```

정렬은 draw determinism 전용이다. Party 순서나 social identity로 사용하지 않는다.

### 5.5 신규 `Client/Public/WorldPlayerNameplateView.h` 전문

```cpp
#pragma once

#include "ClientReplication.h"

#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

// Bern/Valtan의 Server-replicated player를 화면 공간 이름표로 투영한다.
// 이 view는 player identity, Character, font 또는 gameplay state를 소유하지 않는다.
class CWorldPlayerNameplateView final
{
public:
	static bool_t Try_ProjectWorldPosition(
		const float3_t& vWorldPosition,
		const float4x4_t& ViewMatrix,
		const float4x4_t& ProjectionMatrix,
		const float2_t& vViewportSize,
		float2_t& vOutScreenPosition);

	void Render(
		const std::vector<REPLICATED_PLAYER_VIEW>& Players) const;

private:
	static bool_t Try_ConvertUtf8(
		std::string_view Utf8,
		std::wstring& OutWide);
};

NS_END
```

### 5.6 신규 `Client/Private/WorldPlayerNameplateView.cpp` 전문

```cpp
#include "WorldPlayerNameplateView.h"

#include "Character.h"
#include "GameInstance.h"
#include "Transform.h"

#include <Windows.h>

#include <cmath>
#include <limits>
#include <utility>

namespace
{
	constexpr f32_t NAMEPLATE_HEAD_OFFSET = 2.2f;
	constexpr f32_t NAMEPLATE_TEXT_SCALE = 0.75f;

	bool_t Is_Finite(const float2_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y);
	}

	bool_t Is_Finite(const float3_t& value)
	{
		return std::isfinite(value.x) &&
			std::isfinite(value.y) &&
			std::isfinite(value.z);
	}
}

bool_t Client::CWorldPlayerNameplateView::Try_ProjectWorldPosition(
	const float3_t& vWorldPosition,
	const float4x4_t& ViewMatrix,
	const float4x4_t& ProjectionMatrix,
	const float2_t& vViewportSize,
	float2_t& vOutScreenPosition)
{
	vOutScreenPosition = {};
	if (!Is_Finite(vWorldPosition) ||
		!Is_Finite(vViewportSize) ||
		vViewportSize.x <= 0.f || vViewportSize.y <= 0.f)
	{
		return false;
	}

	const vector_t vViewPosition = XMVector3TransformCoord(
		XMLoadFloat3(&vWorldPosition),
		XMLoadFloat4x4(&ViewMatrix));
	float3_t viewPosition{};
	XMStoreFloat3(&viewPosition, vViewPosition);
	if (!Is_Finite(viewPosition) || viewPosition.z <= 0.f)
		return false;

	const vector_t vProjected = XMVector3TransformCoord(
		vViewPosition,
		XMLoadFloat4x4(&ProjectionMatrix));
	float3_t ndc{};
	XMStoreFloat3(&ndc, vProjected);
	if (!Is_Finite(ndc) ||
		ndc.x < -1.f || ndc.x > 1.f ||
		ndc.y < -1.f || ndc.y > 1.f ||
		ndc.z < 0.f || ndc.z > 1.f)
	{
		return false;
	}

	vOutScreenPosition = float2_t(
		(ndc.x * 0.5f + 0.5f) * vViewportSize.x,
		(0.5f - ndc.y * 0.5f) * vViewportSize.y);
	return Is_Finite(vOutScreenPosition);
}

bool_t Client::CWorldPlayerNameplateView::Try_ConvertUtf8(
	const std::string_view Utf8,
	std::wstring& OutWide)
{
	OutWide.clear();
	if (Utf8.empty() ||
		Utf8.size() > static_cast<std::size_t>(
			(std::numeric_limits<int>::max)()))
	{
		return false;
	}

	const int sourceLength = static_cast<int>(Utf8.size());

	const int requiredLength = MultiByteToWideChar(
		CP_UTF8,
		MB_ERR_INVALID_CHARS,
		Utf8.data(),
		sourceLength,
		nullptr,
		0);
	if (requiredLength <= 0)
		return false;

	std::wstring staged(
		static_cast<std::size_t>(requiredLength),
		L'\0');
	if (requiredLength != MultiByteToWideChar(
		CP_UTF8,
		MB_ERR_INVALID_CHARS,
		Utf8.data(),
		sourceLength,
		staged.data(),
		requiredLength))
	{
		return false;
	}

	OutWide = std::move(staged);
	return true;
}

void Client::CWorldPlayerNameplateView::Render(
	const std::vector<REPLICATED_PLAYER_VIEW>& Players) const
{
	CGameInstance& gameInstance = CGameInstance::Get();
	const float4x4_t* const pViewMatrix =
		gameInstance.Get_Transform(D3DTS::VIEW);
	const float4x4_t* const pProjectionMatrix =
		gameInstance.Get_Transform(D3DTS::PROJ);
	if (nullptr == pViewMatrix || nullptr == pProjectionMatrix)
		return;

	const float2_t vViewportSize = gameInstance.Get_ViewportSize();
	for (const REPLICATED_PLAYER_VIEW& player : Players)
	{
		if (LostArk::Shared::INVALID_PLAYER_ID == player.iPlayerId ||
			LostArk::Shared::INVALID_NET_ENTITY_ID == player.iNetEntityId ||
			player.strNickname.empty())
		{
			continue;
		}

		const std::shared_ptr<CCharacter> pCharacter =
			player.pCharacter.lock();
		if (nullptr == pCharacter)
			continue;
		const std::shared_ptr<CTransform> pTransform =
			pCharacter->Get_Transform();
		if (nullptr == pTransform)
			continue;

		float3_t vHeadPosition{};
		XMStoreFloat3(
			&vHeadPosition,
			pTransform->Get_State(STATE::POSITION));
		vHeadPosition.y += NAMEPLATE_HEAD_OFFSET;

		float2_t vScreenPosition{};
		if (!Try_ProjectWorldPosition(
			vHeadPosition,
			*pViewMatrix,
			*pProjectionMatrix,
			vViewportSize,
			vScreenPosition))
		{
			continue;
		}

		std::wstring nickname;
		if (!Try_ConvertUtf8(player.strNickname, nickname))
			continue;

		gameInstance.Draw_Text(
			TEXT("Font_YG330"),
			nickname.c_str(),
			vScreenPosition,
			Colors::White,
			0.f,
			float2_t(0.5f, 0.5f),
			NAMEPLATE_TEXT_SCALE);
	}
}
```

projection/UTF-8/expired object 실패는 해당 player만 skip한다. `Draw_Text`는 반환값이 없는 기존 Font Manager 계약이며 `Font_YG330`은 `CMainApp::Ready_Fonts()`에서 준비한다.

### 5.7 Bern/Valtan Level 연결

`Client/Public/Level_Bern.h`와 `Client/Public/Level_ValtanArena.h`에 추가:
```cpp
#include "WorldPlayerNameplateView.h"
```

각 파일의 `CClientReplication m_Replication;` 바로 아래에 추가:

```cpp
	CWorldPlayerNameplateView m_PlayerNameplateView;
	std::vector<REPLICATED_PLAYER_VIEW> m_NameplatePlayers;
```

`Client/Private/Level_Bern.cpp`의 `Render()` 전체 교체:

```cpp
HRESULT CLevel_Bern::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	m_Replication.Collect_PlayerViews(m_NameplatePlayers);
	m_PlayerNameplateView.Render(m_NameplatePlayers);

#ifdef _DEBUG
	SetWindowText(
		g_hWnd,
		TEXT("Bern Castle Network Player Test"));
#endif

	return S_OK;
}
```

`Client/Private/Level_ValtanArena.cpp`의 `Render()` 전체 교체:

```cpp
HRESULT CLevel_ValtanArena::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	m_Replication.Collect_PlayerViews(m_NameplatePlayers);
	m_PlayerNameplateView.Render(m_NameplatePlayers);

#ifdef _DEBUG
	SetWindowText(g_hWnd, TEXT("Valtan Arena Map"));
#endif

	return S_OK;
}
```

Engine render 순서는 3D renderer 뒤 현재 Level `Render()`, 그 뒤 MainApp HUD/ImGui다. 따라서 nameplate는 3D 위에 그리고 HUD 아래에 놓인다.

## 6. Visual Studio project/filter 등록

### 6.1 `Client/Default/Client.vcxproj`

`PartyWindowView.h` 바로 아래:

```xml
    <ClInclude Include="..\Public\WorldPlayerNameplateView.h" />
```

`PartyWindowView.cpp` 바로 아래:

```xml
    <ClCompile Include="..\Private\WorldPlayerNameplateView.cpp" />
```

### 6.2 `Client/Default/Client.vcxproj.filters`

`PartyWindowView.cpp` block 바로 아래:

```xml
    <ClCompile Include="..\Private\WorldPlayerNameplateView.cpp">
      <Filter>02.GameObjects\04. UI</Filter>
    </ClCompile>
```

`PartyWindowView.h` block 바로 아래:

```xml
    <ClInclude Include="..\Public\WorldPlayerNameplateView.h">
      <Filter>02.GameObjects\04. UI</Filter>
    </ClInclude>
```

`02.GameObjects\04. UI` filter는 현재 실제 filters에 존재한다. 새 filter를 만들지 않는다.

### 6.3 `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`

`CharacterSelectionState.cpp`가 Shared의 비-inline `Is_Valid_PlayerNickname()`을 호출하므로, 마지막 `ClCompile` ItemGroup 뒤와 `Microsoft.Cpp.targets` import 앞에 다음 reference를 추가한다. include path만으로는 link가 닫히지 않는다.

```xml
  <ItemGroup>
    <ProjectReference Include="..\..\..\Shared\Default\Shared.vcxproj">
      <Project>{F4CCF815-6D51-412F-A76E-84D2F1D05571}</Project>
    </ProjectReference>
  </ItemGroup>
```

`NetObjectRegistry.cpp` block 바로 아래:

```xml
	<ClCompile Include="..\..\..\Client\Private\WorldPlayerNameplateView.cpp">
	  <Link>Client\WorldPlayerNameplateView.cpp</Link>
	</ClCompile>
```

### 6.4 `Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj.filters`

`NetObjectRegistry.cpp` block 바로 아래:

```xml
	<ClCompile Include="..\..\..\Client\Private\WorldPlayerNameplateView.cpp">
	  <Filter>Client</Filter>
	</ClCompile>
```

## 7. Harness 반영 계약

### 7.1 NetworkProtocolHarness

`Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`의 기존 `Test_EnterWorldRoundTrip()` 바로 뒤에 다음 전체 함수를 추가한다.

```cpp
	void Test_PlayerNicknameContract(TEST_RUNNER& testRunner)
	{
		const std::string korean = "\xED\x95\x9C\xEA\xB8\x80";
		const std::string exactLimit(MAX_NICKNAME_BYTES, 'N');
		testRunner.Require(
			Is_Valid_PlayerNickname("Test-1234") &&
			Is_Valid_PlayerNickname(korean) &&
			Is_Valid_PlayerNickname(exactLimit),
			"Accept ASCII Korean And Exact-Limit Nicknames");

		const std::vector<std::string> invalidNicknames
		{
			{},
			std::string(MAX_NICKNAME_BYTES + 1u, 'N'),
			std::string("A\0B", 3u),
			" leading",
			"trailing\t",
			"line\nbreak",
			std::string("\xC2\x85", 2u),
			std::string("\x80", 1u),
			std::string("\xC0\xAF", 2u),
			std::string("\xED\xA0\x80", 3u),
			std::string("\xF4\x90\x80\x80", 4u)
		};
		bool allInvalidRejected = true;
		for (const std::string& nickname : invalidNicknames)
		{
			C2S_ENTER_WORLD enter{};
			enter.eWorldId = WORLD_ID::BERN;
			enter.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			enter.strNickName = nickname;
			CPacketWriter enterWriter;
			allInvalidRejected &= !Write_Message(enterWriter, enter);

			S2C_PLAYER_SPAWNED spawned{};
			spawned.iPlayerId = 1u;
			spawned.iNetEntityId = 101u;
			spawned.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
			spawned.strNickName = nickname;
			CPacketWriter spawnedWriter;
			allInvalidRejected &= !Write_Message(spawnedWriter, spawned);
		}
		testRunner.Require(allInvalidRejected,
			"C2S And Spawn Writers Share Exact Nickname Rejection Policy");

		bool allRoundTripsExact = true;
		for (const std::string& nickname :
			std::vector<std::string>{ "Test-1234", korean, exactLimit })
		{
			C2S_ENTER_WORLD enter{};
			enter.eWorldId = WORLD_ID::BERN;
			enter.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
			enter.strNickName = nickname;
			CPacketWriter enterWriter;
			C2S_ENTER_WORLD decodedEnter{};
			if (!Write_Message(enterWriter, enter))
			{
				allRoundTripsExact = false;
				continue;
			}
			CPacketReader enterReader{ enterWriter.Get_Buffer() };
			allRoundTripsExact &= Read_Message(enterReader, decodedEnter) &&
				0u == enterReader.Get_RemainingSize() &&
				decodedEnter.strNickName == nickname;

			S2C_PLAYER_SPAWNED spawned{};
			spawned.iPlayerId = 2u;
			spawned.iNetEntityId = 202u;
			spawned.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
			spawned.strNickName = nickname;
			CPacketWriter spawnedWriter;
			S2C_PLAYER_SPAWNED decodedSpawned{};
			if (!Write_Message(spawnedWriter, spawned))
			{
				allRoundTripsExact = false;
				continue;
			}
			CPacketReader spawnedReader{ spawnedWriter.Get_Buffer() };
			allRoundTripsExact &=
				Read_Message(spawnedReader, decodedSpawned) &&
				0u == spawnedReader.Get_RemainingSize() &&
				decodedSpawned.strNickName == nickname;
		}
		testRunner.Require(allRoundTripsExact,
			"C2S And Spawn Codecs Preserve Exact Nickname Bytes");

		const std::string malformed("\xC0\xAF", 2u);
		CPacketWriter rawEnter;
		rawEnter.Write_U16(NETWORK_PROTOCOL_VERSION);
		rawEnter.Write_U16(static_cast<std::uint16_t>(WORLD_ID::BERN));
		rawEnter.Write_U8(static_cast<std::uint8_t>(
			CHARACTER_CLASS_ID::ARTIST));
		const bool builtMalformedEnter = rawEnter.Write_String(
			malformed, MAX_NICKNAME_BYTES);
		CPacketReader malformedEnterReader{ rawEnter.Get_Buffer() };
		C2S_ENTER_WORLD unchangedEnter{};
		unchangedEnter.iProtocolVersion = 77u;
		unchangedEnter.eWorldId = WORLD_ID::TRAINING_GROUND;
		unchangedEnter.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
		unchangedEnter.strNickName = "keep-enter";
		testRunner.Require(
			builtMalformedEnter &&
			!Read_Message(malformedEnterReader, unchangedEnter) &&
			77u == unchangedEnter.iProtocolVersion &&
			WORLD_ID::TRAINING_GROUND == unchangedEnter.eWorldId &&
			CHARACTER_CLASS_ID::WARLORD == unchangedEnter.eCharacterClass &&
			"keep-enter" == unchangedEnter.strNickName,
			"Malformed C2S Nickname Decode Preserves Destination");

		CPacketWriter rawSpawn;
		rawSpawn.Write_U32(3u);
		rawSpawn.Write_U32(303u);
		rawSpawn.Write_U8(static_cast<std::uint8_t>(
			CHARACTER_CLASS_ID::ARTIST));
		const bool builtMalformedSpawn = rawSpawn.Write_String(
			malformed, MAX_NICKNAME_BYTES);
		rawSpawn.Write_F32(1.f);
		rawSpawn.Write_F32(2.f);
		rawSpawn.Write_F32(3.f);
		rawSpawn.Write_F32(90.f);
		CPacketReader malformedSpawnReader{ rawSpawn.Get_Buffer() };
		S2C_PLAYER_SPAWNED unchangedSpawn{};
		unchangedSpawn.iPlayerId = 9u;
		unchangedSpawn.iNetEntityId = 909u;
		unchangedSpawn.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
		unchangedSpawn.strNickName = "keep-spawn";
		testRunner.Require(
			builtMalformedSpawn &&
			!Read_Message(malformedSpawnReader, unchangedSpawn) &&
			9u == unchangedSpawn.iPlayerId &&
			909u == unchangedSpawn.iNetEntityId &&
			CHARACTER_CLASS_ID::WARLORD == unchangedSpawn.eCharacterClass &&
			"keep-spawn" == unchangedSpawn.strNickName,
			"Malformed Spawn Nickname Decode Preserves Destination");
	}
```

`main()`의 `Test_EnterWorldRoundTrip(testRunner);` 바로 뒤에 호출을 추가한다.

```cpp
	Test_PlayerNicknameContract(testRunner);
```

hardcoded protocol byte fixture를 새로 만들지 않고 `NETWORK_PROTOCOL_VERSION`을 writer로 기록한다. 이 함수가 ASCII/한글/32-byte 경계, invalid UTF-8/control/edge whitespace, C2S·spawn 동일 정책, failed-read destination 불변을 함께 닫는다.

### 7.2 ClientFrontendHarness

include에서 `#include "NetObjectRegistry.h"` 바로 뒤에 추가한다.

```cpp
#include "WorldPlayerNameplateView.h"
```

기존 `Test_CharacterSelectAuthorizedSelection()` 전체를 교체한다.

```cpp
	void Test_CharacterSelectAuthorizedSelection(TEST_RUNNER& runner)
	{
		using namespace Client;
		using namespace LostArk::Shared;

		CHARACTER_ENTRY_IDENTITY identity{};
		runner.Require(
			CCharacterSelectionState::Try_Resolve_ForWorld(
				WORLD_ID::CHARACTER_SELECT_ARENA, identity) &&
			CHARACTER_ENTRY_IDENTITY_SOURCE::AUDITION == identity.eSource &&
			CHARACTER_CLASS_ID::LANCE_MASTER == identity.eCharacterClass &&
			0u == identity.strNickname.rfind("Test-", 0u),
			"Character Select resolves process audition identity");
		runner.Require(
			!CCharacterSelectionState::Try_Resolve_ForWorld(
				WORLD_ID::BERN, identity),
			"Bern rejects entry before character creation");
		runner.Require(
			CCharacterSelectionState::Try_Resolve_ForWorld(
				WORLD_ID::VALTAN_ARENA, identity) &&
			CHARACTER_ENTRY_IDENTITY_SOURCE::AUDITION == identity.eSource,
			"Valtan resolves audition identity before creation");

		runner.Require(CCharacterSelectionState::Select(
			CHARACTER_CLASS_ID::DIMENSIONMASTER),
			"Commit Server-approved Character Select class");
		runner.Require(CCharacterSelectionState::Stage_Creation(
			CHARACTER_CLASS_ID::ARTIST, "G02-Artist"),
			"Stage character creation atomically");
		runner.Require(
			CCharacterSelectionState::Try_Resolve_ForWorld(
				WORLD_ID::BERN, identity) &&
			CHARACTER_ENTRY_IDENTITY_SOURCE::PENDING_CREATION == identity.eSource &&
			CHARACTER_CLASS_ID::ARTIST == identity.eCharacterClass &&
			"G02-Artist" == identity.strNickname,
			"Bern resolves exact pending identity");

		CCharacterSelectionState::Cancel_PendingCreation();
		CHARACTER_CLASS_ID selected = CHARACTER_CLASS_ID::END;
		runner.Require(
			CCharacterSelectionState::Try_Get_SelectedClass(selected) &&
			CHARACTER_CLASS_ID::DIMENSIONMASTER == selected &&
			!CCharacterSelectionState::Try_Resolve_ForWorld(
				WORLD_ID::BERN, identity),
			"Creation cancel preserves selected class and no created identity");

		runner.Require(
			CCharacterSelectionState::Stage_Creation(
				CHARACTER_CLASS_ID::ARTIST, "G02-Artist") &&
			CCharacterSelectionState::Commit_PendingCreation(),
			"Commit pending character identity atomically");
		runner.Require(
			CCharacterSelectionState::Try_Resolve_ForWorld(
				WORLD_ID::BERN, identity) &&
			CHARACTER_ENTRY_IDENTITY_SOURCE::CREATED == identity.eSource &&
			CHARACTER_CLASS_ID::ARTIST == identity.eCharacterClass &&
			"G02-Artist" == identity.strNickname,
			"Bern resolves exact created identity");
		runner.Require(
			CCharacterSelectionState::Try_Resolve_ForWorld(
				WORLD_ID::VALTAN_ARENA, identity) &&
			CHARACTER_ENTRY_IDENTITY_SOURCE::CREATED == identity.eSource &&
			CHARACTER_CLASS_ID::ARTIST == identity.eCharacterClass &&
			"G02-Artist" == identity.strNickname,
			"Valtan reuses exact created identity");
		runner.Require(
			!CCharacterSelectionState::Stage_Creation(
				CHARACTER_CLASS_ID::END, "invalid") &&
			!CCharacterSelectionState::Stage_Creation(
				CHARACTER_CLASS_ID::WARLORD, " invalid") &&
			CCharacterSelectionState::Try_Resolve_ForWorld(
				WORLD_ID::BERN, identity) &&
			"G02-Artist" == identity.strNickname,
			"Rejected creation preserves committed identity");

		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(
				LOBBY_STAGE::CHARACTER_SELECT, token) &&
			INVALID_LOBBY_COMMAND_TOKEN != token,
			"Character Select stages tokenized Server entry");
		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::CHARACTER_SELECT == command.eStage &&
			LOBBY_COMMAND_PURPOSE::GAMEPLAY == command.ePurpose &&
			token == command.iToken,
			"Lobby consumes exact Character Select Server entry");
		Require_NoPendingCommand(
			runner,
			"Character Select entry leaves no stale Lobby command");
	}
```

기존 `Test_NetObjectRegistryClassReplacement()` 전체를 교체한다.

```cpp
	void Test_NetObjectRegistryClassReplacement(TEST_RUNNER& runner)
	{
		using namespace Client;
		using namespace LostArk::Shared;
		const auto noDelete = [](CCharacter*) {};
		std::shared_ptr<CCharacter> first{
			reinterpret_cast<CCharacter*>(static_cast<std::uintptr_t>(1u)), noDelete };
		std::shared_ptr<CCharacter> second{
			reinterpret_cast<CCharacter*>(static_cast<std::uintptr_t>(2u)), noDelete };

		CNetObjectRegistry registry;
		NET_PLAYER_RECORD record{};
		record.iPlayerId = 7u;
		record.iNetEntityId = 107u;
		record.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		record.strNickName = "RegistryClassSwitch";
		OBJECT_HANDLE originalHandle{};
		runner.Require(registry.Register(record, first, originalHandle),
			"Registry stages original Character Select entity");

		NET_PLAYER_RECORD replacement = record;
		replacement.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		OBJECT_HANDLE replacementHandle{};
		runner.Require(
			registry.Replace(
				record.iNetEntityId, replacement, second, replacementHandle) &&
			originalHandle.iSlotIndex == replacementHandle.iSlotIndex &&
			originalHandle.iGeneration == replacementHandle.iGeneration &&
			registry.Resolve(replacementHandle) == second &&
			nullptr != registry.Find_Record(record.iNetEntityId) &&
			CHARACTER_CLASS_ID::ARTIST ==
				registry.Find_Record(record.iNetEntityId)->eCharacterClass,
			"Registry atomically replaces same-entity class presentation");

		{
			const std::vector<LIVE_NET_PLAYER> live =
				registry.Get_LivePlayers();
			runner.Require(
				1u == live.size() &&
				7u == live.front().Record.iPlayerId &&
				107u == live.front().Record.iNetEntityId &&
				CHARACTER_CLASS_ID::ARTIST ==
					live.front().Record.eCharacterClass &&
				"RegistryClassSwitch" == live.front().Record.strNickName &&
				live.front().pCharacter == second,
				"Registry returns exact record-presentation live pair");
		}

		NET_PLAYER_RECORD invalid = replacement;
		invalid.eCharacterClass = CHARACTER_CLASS_ID::END;
		OBJECT_HANDLE ignored{};
		runner.Require(
			!registry.Replace(record.iNetEntityId, invalid, first, ignored) &&
			registry.Resolve(replacementHandle) == second &&
			CHARACTER_CLASS_ID::ARTIST ==
				registry.Find_Record(record.iNetEntityId)->eCharacterClass,
			"Rejected registry replacement preserves committed presentation");

		second.reset();
		runner.Require(registry.Get_LivePlayers().empty(),
			"Registry excludes expired player presentation from live pairs");
	}
```

그 함수 바로 뒤에 projection test를 추가한다.

```cpp
	void Test_WorldPlayerNameplateProjection(TEST_RUNNER& runner)
	{
		using namespace Client;
		float4x4_t identity{};
		XMStoreFloat4x4(&identity, XMMatrixIdentity());
		const float2_t viewport(1280.f, 720.f);
		float2_t screen(77.f, 88.f);
		runner.Require(
			CWorldPlayerNameplateView::Try_ProjectWorldPosition(
				float3_t(0.f, 0.f, 0.5f), identity, identity,
				viewport, screen) &&
			std::abs(screen.x - 640.f) < 0.001f &&
			std::abs(screen.y - 360.f) < 0.001f,
			"Nameplate projects center to viewport center");
		runner.Require(
			CWorldPlayerNameplateView::Try_ProjectWorldPosition(
				float3_t(0.f, 0.5f, 0.5f), identity, identity,
				viewport, screen) &&
			std::abs(screen.y - 180.f) < 0.001f,
			"Nameplate converts Direct3D Y to top-left screen Y");
		runner.Require(
			!CWorldPlayerNameplateView::Try_ProjectWorldPosition(
				float3_t(0.f, 0.f, -0.1f), identity, identity,
				viewport, screen) && 0.f == screen.x && 0.f == screen.y,
			"Nameplate rejects camera-behind point transactionally");
		runner.Require(
			!CWorldPlayerNameplateView::Try_ProjectWorldPosition(
				float3_t(2.f, 0.f, 0.5f), identity, identity,
				viewport, screen) &&
			!CWorldPlayerNameplateView::Try_ProjectWorldPosition(
				float3_t(0.f, 0.f, 1.1f), identity, identity,
				viewport, screen) &&
			!CWorldPlayerNameplateView::Try_ProjectWorldPosition(
				float3_t(0.f, 0.f, 0.5f), identity, identity,
				float2_t(0.f, 720.f), screen),
			"Nameplate rejects viewport/far/zero-viewport points");
	}
```

기본 test 호출부의 `Test_NetObjectRegistryClassReplacement(runner);` 바로 뒤에 추가한다.

```cpp
	Test_WorldPlayerNameplateProjection(runner);
```

기존 `--lobby-endpoint-fast` mode 분기 바로 위에 focused mode를 추가한다.

```cpp
	if (Mode == "--g02-nameplate-fast")
	{
		Test_CharacterSelectAuthorizedSelection(runner);
		Test_NetObjectRegistryClassReplacement(runner);
		Test_WorldPlayerNameplateProjection(runner);
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}
```

### 7.3 CharacterSelectIsolationHarness

기존 G01 project를 재사용한다. 새 vcxproj를 만들지 않는다.

`CTestClient::Get_NetEntityId()` 바로 뒤에 추가한다.

```cpp
		[[nodiscard]] bool Has_ExactSpawnNickname(
			const NET_ENTITY_ID entityId,
			const std::string_view expected) const
		{
			const auto iter = m_ObservedSpawnedPlayerNicknames.find(entityId);
			return iter != m_ObservedSpawnedPlayerNicknames.end() &&
				std::string_view(iter->second) == expected;
		}
```

enter-accepted reset에서 `m_ObservedSpawnedPlayerEntityIds.clear();` 바로 뒤에 추가한다.

```cpp
				m_ObservedSpawnedPlayerNicknames.clear();
```

`S2C_PLAYER_SPAWNED` branch의 기존 entity insert 바로 뒤에 추가한다.

```cpp
				m_ObservedSpawnedPlayerNicknames.insert_or_assign(
					spawned.iNetEntityId, spawned.strNickName);
```

member `m_ObservedSpawnedPlayerEntityIds` 바로 뒤에 추가한다. `<map>`은 기존 include를 사용한다.

```cpp
		std::map<NET_ENTITY_ID, std::string>
			m_ObservedSpawnedPlayerNicknames;
```

`Run_CharacterSelectIsolation()` 첫 admission probe의 PASS 조건을 교체한다.

```cpp
				return clientA->Has_OwnOnlyPlayerSnapshot() &&
					clientB->Has_OwnOnlyPlayerSnapshot() &&
					clientA->Has_ExactSpawnNickname(
						clientA->Get_NetEntityId(), "CS-Isolation-A") &&
					clientB->Has_ExactSpawnNickname(
						clientB->Get_NetEntityId(), "CS-Isolation-B") ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
```

`Run_BernSharedProof()`의 PASS 조건을 교체한다.

```cpp
				return first->Has_ExactPlayerSnapshot(expected) &&
					second->Has_ExactPlayerSnapshot(expected) &&
					first->Has_ExactSpawnNickname(
						first->Get_NetEntityId(), "Bern-Shared-A") &&
					first->Has_ExactSpawnNickname(
						second->Get_NetEntityId(), "Bern-Shared-B") &&
					second->Has_ExactSpawnNickname(
						first->Get_NetEntityId(), "Bern-Shared-A") &&
					second->Has_ExactSpawnNickname(
						second->Get_NetEntityId(), "Bern-Shared-B") ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
```

`Run_BernToValtanTransferProof()`의 initial Bern admission PASS 조건을 교체한다.

```cpp
				return client->Is_Accepted() &&
					client->Has_OwnOnlyPlayerSnapshot() &&
					client->Has_ExactSpawnNickname(
						client->Get_NetEntityId(), "Bern-To-Valtan") ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
```

같은 함수의 Valtan transfer acceptance PASS 조건을 교체한다.

```cpp
				return 2u == client->Get_AcceptanceCount() &&
					WORLD_ID::VALTAN_ARENA == client->Get_CurrentWorld() &&
					client->Has_OwnOnlyPlayerSnapshot() &&
					client->Has_ExactSpawnNickname(
						client->Get_NetEntityId(), "Bern-To-Valtan") ?
					PROBE_RESULT::PASS : PROBE_RESULT::WAIT;
```

이 harness는 한 프로세스 안에서 여러 socket Client를 만들기 때문에 제품 Client의 `Test-<process-id>` 생성 자체를 검증하지 않는다. process-local audition identity는 `ClientFrontendHarness`가 검증하고, live harness는 자신이 제출한 `m_strLabel`의 Server 저장·spawn echo·private 격리·world transfer 보존을 검증한다.

## 8. 구현 순서

1. Shared validator + codec + NetworkProtocolHarness
2. CharacterSelectionState + ClientFrontendHarness identity cases
3. Character Select modal + safe transition ordering
4. Lobby entry + Loading/MainApp commit/rollback
5. GameRoom validator + Server contract/live nickname echo
6. Registry/Replication read-only view
7. 신규 nameplate H/CPP
8. Bern/Valtan consumer
9. Client/ClientFrontendHarness project/filter 등록
10. public handbook와 RESULT

각 단계에서 현재 dirty file의 기존 변경을 먼저 읽고 block 단위로 병합한다.

## 9. 빌드와 검증 명령

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
```

정본 자동화:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
```

G02 Client state/nameplate focused 실행:

```powershell
Tools/ClientFrontendHarness/Bin/Debug/ClientFrontendHarness.exe --g02-nameplate-fast
Tools/ClientFrontendHarness/Bin/Release/ClientFrontendHarness.exe --g02-nameplate-fast
```

필수 결과:

- Shared Debug/Release link
- NetworkProtocolHarness Debug/Release failures 0
- Server Debug/Release link
- `Server.exe --contract-test` Debug/Release failures 0
- ClientFrontendHarness Debug/Release failures 0
- CharacterSelectIsolationHarness Debug/Release failures 0
- Client Debug/Release 실제 link
- XML parse: Client와 ClientFrontendHarness vcxproj/filters
- 실제 폴더 changed/untracked `.h/.cpp` 전수 project/filter 등록 누락 0
- `git diff --check`

Client/UI는 에이전트가 자율 실행하지 않는다. 사용자가 직접 다음을 확인한다.

```text
Lobby -> Character Select
class 선택
Create Character
nickname 입력
Create and Enter Bern
Bern 자동 진입
자기 및 다른 replicated character 위 exact nickname
Create 전 Direct Valtan -> Test-<process-id>
Create 후 Direct Valtan -> created nickname
```

시각 결과는 사용자 확인 전 `visual PASS`로 기록하지 않는다.

## 10. 완료 후 RESULT에 기록할 경계

- 구현 완료: 실제 diff와 project/filter 등록
- 자동 검증: 실행한 Debug/Release build와 harness 결과
- 수동 검증: 사용자가 관찰한 nickname modal/Bern/nameplate/Valtan 결과
- 미완료: 영구 계정 저장, nickname 고유성, character slot, Party, hover/RMB, chat

G02의 Server 저장은 현재 접속한 `SERVER_PLAYER`와 world transfer 동안 유지되는 in-memory nickname이다. 계정 DB나 재접속 영구 저장 완료로 표현하지 않는다.
