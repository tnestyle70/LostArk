```cpp
// FILE: Shared/Public/Network/PacketMessages.h
// INSERT AFTER S2C_WORLD_SNAPSHOT READ/WRITE DECLARATIONS

inline constexpr std::size_t MAX_CHAT_TEXT_BYTES = 256;

enum class CHAT_CHANNEL : std::uint8_t
{
	ROOM,
	END
};

struct C2S_CHAT
{
	std::string strText;
};

struct S2C_CHAT
{
	std::uint32_t iMessageId = 0;
	PLAYER_ID iSenderPlayerId = INVALID_PLAYER_ID;
	std::string strSenderNickname;
	CHAT_CHANNEL eChannel = CHAT_CHANNEL::END;
	std::string strText;
};

[[nodiscard]] bool Is_Valid_Chat_Text(std::string_view text);

bool Write_Message(
	CPacketWriter& writer,
	const C2S_CHAT& message);

bool Read_Message(
	CPacketReader& reader,
	C2S_CHAT& message);

bool Write_Message(
	CPacketWriter& writer,
	const S2C_CHAT& message);

bool Read_Message(
	CPacketReader& reader,
	S2C_CHAT& message);
```

```cpp
// FILE: Shared/Private/Network/PacketMessages.cpp
// ADD INCLUDE

#include <string_view>
```

```cpp
// FILE: Shared/Private/Network/PacketMessages.cpp
// ADD TO THE ANONYMOUS NAMESPACE

bool Is_Continuation_Byte(std::uint8_t value)
{
	return (value & 0xC0u) == 0x80u;
}

bool Is_Valid_Utf8_Without_Control(std::string_view text)
{
	for (std::size_t index = 0; index < text.size();)
	{
		const std::uint8_t first =
			static_cast<std::uint8_t>(text[index]);

		if (first <= 0x7Fu)
		{
			if (first < 0x20u || first == 0x7Fu)
				return false;

			++index;
			continue;
		}

		if (first >= 0xC2u && first <= 0xDFu)
		{
			if (index + 1 >= text.size() ||
				!Is_Continuation_Byte(
					static_cast<std::uint8_t>(text[index + 1])))
			{
				return false;
			}

			index += 2;
			continue;
		}

		if (first >= 0xE0u && first <= 0xEFu)
		{
			if (index + 2 >= text.size())
				return false;

			const std::uint8_t second =
				static_cast<std::uint8_t>(text[index + 1]);
			const std::uint8_t third =
				static_cast<std::uint8_t>(text[index + 2]);

			if (!Is_Continuation_Byte(second) ||
				!Is_Continuation_Byte(third) ||
				(first == 0xE0u && second < 0xA0u) ||
				(first == 0xEDu && second >= 0xA0u))
			{
				return false;
			}

			index += 3;
			continue;
		}

		if (first >= 0xF0u && first <= 0xF4u)
		{
			if (index + 3 >= text.size())
				return false;

			const std::uint8_t second =
				static_cast<std::uint8_t>(text[index + 1]);
			const std::uint8_t third =
				static_cast<std::uint8_t>(text[index + 2]);
			const std::uint8_t fourth =
				static_cast<std::uint8_t>(text[index + 3]);

			if (!Is_Continuation_Byte(second) ||
				!Is_Continuation_Byte(third) ||
				!Is_Continuation_Byte(fourth) ||
				(first == 0xF0u && second < 0x90u) ||
				(first == 0xF4u && second >= 0x90u))
			{
				return false;
			}

			index += 4;
			continue;
		}

		return false;
	}

	return true;
}
```

```cpp
// FILE: Shared/Private/Network/PacketMessages.cpp
// APPEND

bool LostArk::Shared::Is_Valid_Chat_Text(std::string_view text)
{
	return !text.empty() &&
		text.size() <= MAX_CHAT_TEXT_BYTES &&
		Is_Valid_Utf8_Without_Control(text);
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_CHAT& message)
{
	return Is_Valid_Chat_Text(message.strText) &&
		writer.Write_String(
			message.strText,
			MAX_CHAT_TEXT_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_CHAT& message)
{
	std::string text;

	if (!reader.Read_String(text, MAX_CHAT_TEXT_BYTES) ||
		!Is_Valid_Chat_Text(text))
	{
		return false;
	}

	C2S_CHAT decoded{};
	decoded.strText = std::move(text);
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_CHAT& message)
{
	const std::uint8_t rawChannel =
		static_cast<std::uint8_t>(message.eChannel);

	if (0 == message.iMessageId ||
		message.iSenderPlayerId == INVALID_PLAYER_ID ||
		message.strSenderNickname.empty() ||
		message.strSenderNickname.size() > MAX_NICKNAME_BYTES ||
		rawChannel >= static_cast<std::uint8_t>(CHAT_CHANNEL::END) ||
		!Is_Valid_Chat_Text(message.strText))
	{
		return false;
	}

	writer.Write_U32(message.iMessageId);
	writer.Write_U32(message.iSenderPlayerId);

	if (!writer.Write_String(
		message.strSenderNickname,
		MAX_NICKNAME_BYTES))
	{
		return false;
	}

	writer.Write_U8(rawChannel);
	return writer.Write_String(
		message.strText,
		MAX_CHAT_TEXT_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_CHAT& message)
{
	std::uint32_t messageId = 0;
	PLAYER_ID senderPlayerId = INVALID_PLAYER_ID;
	std::string senderNickname;
	std::uint8_t rawChannel = 0;
	std::string text;

	if (!reader.Read_U32(messageId) ||
		!reader.Read_U32(senderPlayerId) ||
		!reader.Read_String(
			senderNickname,
			MAX_NICKNAME_BYTES) ||
		!reader.Read_U8(rawChannel) ||
		!reader.Read_String(text, MAX_CHAT_TEXT_BYTES))
	{
		return false;
	}

	if (0 == messageId ||
		senderPlayerId == INVALID_PLAYER_ID ||
		senderNickname.empty() ||
		rawChannel >= static_cast<std::uint8_t>(CHAT_CHANNEL::END) ||
		!Is_Valid_Chat_Text(text))
	{
		return false;
	}

	S2C_CHAT decoded{};
	decoded.iMessageId = messageId;
	decoded.iSenderPlayerId = senderPlayerId;
	decoded.strSenderNickname = std::move(senderNickname);
	decoded.eChannel = static_cast<CHAT_CHANNEL>(rawChannel);
	decoded.strText = std::move(text);
	message = std::move(decoded);
	return true;
}
```

```cpp
// FILE: Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp
// ADD TEST FUNCTION AND CALL IT FROM main()

bool Test_Chat_RoundTrip(TEST_CONTEXT& context)
{
	using namespace LostArk::Shared;

	C2S_CHAT outgoingCommand{};
	outgoingCommand.strText = "Bern party recruitment";

	CPacketWriter commandWriter;
	context.Expect(
		Write_Message(commandWriter, outgoingCommand),
		"C2S_CHAT write succeeds");

	C2S_CHAT decodedCommand{};
	CPacketReader commandReader{ commandWriter.Get_Buffer() };
	context.Expect(
		Read_Message(commandReader, decodedCommand) &&
		0 == commandReader.Get_RemainingSize() &&
		decodedCommand.strText == outgoingCommand.strText,
		"C2S_CHAT round trip");

	S2C_CHAT outgoingEvent{};
	outgoingEvent.iMessageId = 7;
	outgoingEvent.iSenderPlayerId = 1;
	outgoingEvent.strSenderNickname = "ClientA";
	outgoingEvent.eChannel = CHAT_CHANNEL::ROOM;
	outgoingEvent.strText = outgoingCommand.strText;

	CPacketWriter eventWriter;
	context.Expect(
		Write_Message(eventWriter, outgoingEvent),
		"S2C_CHAT write succeeds");

	S2C_CHAT decodedEvent{};
	CPacketReader eventReader{ eventWriter.Get_Buffer() };
	context.Expect(
		Read_Message(eventReader, decodedEvent) &&
		0 == eventReader.Get_RemainingSize() &&
		decodedEvent.iMessageId == outgoingEvent.iMessageId &&
		decodedEvent.iSenderPlayerId == outgoingEvent.iSenderPlayerId &&
		decodedEvent.strSenderNickname == outgoingEvent.strSenderNickname &&
		decodedEvent.eChannel == outgoingEvent.eChannel &&
		decodedEvent.strText == outgoingEvent.strText,
		"S2C_CHAT round trip");

	C2S_CHAT invalid{};
	invalid.strText = "line\nbreak";
	CPacketWriter invalidWriter;
	context.Expect(
		!Write_Message(invalidWriter, invalid) &&
		invalidWriter.Get_Buffer().empty(),
		"C2S_CHAT rejects control characters before write");

	return 0 == context.Get_FailureCount();
}
```

```cpp
// FILE: Server/Public/ServerPlayer.h
// INSERT INTO SERVER_PLAYER

#include <limits>

std::uint32_t iChatWindowStartTick =
	(std::numeric_limits<std::uint32_t>::max)();
std::uint8_t iChatMessagesInWindow = 0;
```

```cpp
// FILE: Server/Public/RoomCommand.h
// REPLACE ROOM_COMMAND_TYPE AND ADD Chat MEMBER

enum class ROOM_COMMAND_TYPE
{
	REGISTER_SESSION,
	ENTER_WORLD,
	MOVE,
	CHAT,
	LEAVE
};

// ROOM_COMMAND MEMBER
LostArk::Shared::C2S_CHAT Chat;
```

```cpp
// FILE: Server/Public/GameRoom.h
// ADD PRIVATE FUNCTIONS AND MEMBER

bool Handle_Chat(
	SESSION_ID sessionId,
	const LostArk::Shared::C2S_CHAT& chat);

bool Broadcast_Chat(
	const LostArk::Shared::S2C_CHAT& chat);

std::uint32_t m_iNextChatMessageId = 1;
```

```cpp
// FILE: Server/Private/GameRoom.cpp
// ADD TO Tick() COMMAND SWITCH AFTER MOVE

case ROOM_COMMAND_TYPE::CHAT:
	if (!Handle_Chat(command.iSessionId, command.Chat))
	{
		if (const std::shared_ptr<CClientSession> session =
			Find_Session(command.iSessionId))
		{
			session->Request_Close();
		}
	}
	break;
```

```cpp
// FILE: Server/Private/GameRoom.cpp
// APPEND

bool LostArk::Server::CGameRoom::Handle_Chat(
	SESSION_ID sessionId,
	const LostArk::Shared::C2S_CHAT& chat)
{
	using namespace LostArk::Shared;

	if (!Is_Valid_Chat_Text(chat.strText))
		return false;

	const auto sessionPlayerIter =
		m_PlayerIdBySessionId.find(sessionId);
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
		return false;

	const auto playerIter =
		m_Players.find(sessionPlayerIter->second);
	if (playerIter == m_Players.end())
		return false;

	SERVER_PLAYER& player = playerIter->second;
	constexpr std::uint32_t CHAT_WINDOW_TICKS = 30;
	constexpr std::uint8_t MAX_CHAT_MESSAGES_PER_WINDOW = 4;

	if (player.iChatWindowStartTick ==
		(std::numeric_limits<std::uint32_t>::max)() ||
		m_iServerTick - player.iChatWindowStartTick >= CHAT_WINDOW_TICKS)
	{
		player.iChatWindowStartTick = m_iServerTick;
		player.iChatMessagesInWindow = 0;
	}

	if (player.iChatMessagesInWindow >= MAX_CHAT_MESSAGES_PER_WINDOW)
		return true;

	if (0 == m_iNextChatMessageId)
		++m_iNextChatMessageId;

	S2C_CHAT event{};
	event.iMessageId = m_iNextChatMessageId++;
	event.iSenderPlayerId = player.iPlayerId;
	event.strSenderNickname = player.strNickName;
	event.eChannel = CHAT_CHANNEL::ROOM;
	event.strText = chat.strText;

	if (!Broadcast_Chat(event))
		return false;

	++player.iChatMessagesInWindow;
	return true;
}

bool LostArk::Server::CGameRoom::Broadcast_Chat(
	const LostArk::Shared::S2C_CHAT& chat)
{
	using namespace LostArk::Shared;

	CPacketWriter writer;
	if (!Write_Message(writer, chat))
		return false;

	bool allSucceeded = true;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session =
			Find_Session(sessionId);

		if (nullptr == session ||
			!session->Send_Frame(
				PACKET_TYPE::S2C_CHAT,
				writer.Get_Buffer()))
		{
			allSucceeded = false;
			if (nullptr != session)
				session->Request_Close();
		}
	}

	return allSucceeded;
}
```

```cpp
// FILE: Server/Private/ServerApp.cpp
// ADD TO On_SessionFrame() SWITCH AFTER C2S_MOVE

case PACKET_TYPE::C2S_CHAT:
{
	C2S_CHAT chat{};

	if (!Read_Message(reader, chat) ||
		0 != reader.Get_RemainingSize())
	{
		Request_SessionClose(sessionId);
		return;
	}

	command.eType = ROOM_COMMAND_TYPE::CHAT;
	command.Chat = std::move(chat);
	break;
}
```

```cpp
// FILE: Client/Public/ClientReplicationEvent.h
// REPLACE ENUM AND ADD Chat MEMBER

enum class CLIENT_REPLICATION_EVENT_TYPE
{
	PLAYER_SPAWNED,
	WORLD_SNAPSHOT,
	PLAYER_DESPAWNED,
	CHAT_RECEIVED
};

// CLIENT_REPLICATION_EVENT MEMBER
LostArk::Shared::S2C_CHAT Chat;
```

```cpp
// FILE: Client/Public/NetworkManager.h
// ADD PUBLIC FUNCTION

bool Send_Chat(std::string_view text);
```

```cpp
// FILE: Client/Private/NetworkManager.cpp
// APPEND BEFORE Try_Consume_EnterAccepted()

bool CNetworkManager::Send_Chat(std::string_view text)
{
	using namespace LostArk::Shared;

	if (!Is_Connected())
		return false;

	C2S_CHAT message{};
	message.strText = std::string{ text };

	CPacketWriter payloadWriter;
	if (!Write_Message(payloadWriter, message))
		return false;

	std::vector<std::uint8_t> frameBytes;
	if (!Build_Packet_Frame(
		PACKET_TYPE::C2S_CHAT,
		payloadWriter.Get_Buffer(),
		frameBytes))
	{
		return false;
	}

	return Send_All(frameBytes);
}
```

```cpp
// FILE: Client/Private/NetworkManager.cpp
// ADD TO Handle_Frame() SWITCH

case PACKET_TYPE::S2C_CHAT:
{
	S2C_CHAT chat{};

	if (!Read_Message(reader, chat) ||
		0 != reader.Get_RemainingSize())
	{
		m_iLastErrorCode.store(WSAEINVAL);
		return;
	}

	Client::CLIENT_REPLICATION_EVENT event{};
	event.eType =
		Client::CLIENT_REPLICATION_EVENT_TYPE::CHAT_RECEIVED;
	event.Chat = std::move(chat);
	m_ReplicationEvents.push_back(std::move(event));
	break;
}
```

```cpp
// FILE: Client/Public/ChatViewModel.h

#pragma once

#include "Network/PacketMessages.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>

namespace Client
{
	struct CHAT_ENTRY
	{
		std::uint32_t iMessageId = 0;
		LostArk::Shared::PLAYER_ID iSenderPlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		std::string strSenderNickname;
		LostArk::Shared::CHAT_CHANNEL eChannel =
			LostArk::Shared::CHAT_CHANNEL::END;
		std::string strText;
	};

	class CChatViewModel final
	{
	public:
		bool Apply(const LostArk::Shared::S2C_CHAT& chat);
		void Reset();

		[[nodiscard]] const std::deque<CHAT_ENTRY>&
			Get_Entries() const;

	private:
		static constexpr std::size_t MAX_ENTRIES = 100;
		std::deque<CHAT_ENTRY> m_Entries;
		std::uint32_t m_iLastMessageId = 0;
	};
}
```

```cpp
// FILE: Client/Private/ChatViewModel.cpp

#include "ChatViewModel.h"

bool Client::CChatViewModel::Apply(
	const LostArk::Shared::S2C_CHAT& chat)
{
	if (0 == chat.iMessageId ||
		chat.iMessageId <= m_iLastMessageId ||
		chat.iSenderPlayerId == LostArk::Shared::INVALID_PLAYER_ID ||
		chat.strSenderNickname.empty() ||
		!LostArk::Shared::Is_Valid_Chat_Text(chat.strText))
	{
		return false;
	}

	CHAT_ENTRY entry{};
	entry.iMessageId = chat.iMessageId;
	entry.iSenderPlayerId = chat.iSenderPlayerId;
	entry.strSenderNickname = chat.strSenderNickname;
	entry.eChannel = chat.eChannel;
	entry.strText = chat.strText;
	m_Entries.push_back(std::move(entry));
	m_iLastMessageId = chat.iMessageId;

	while (m_Entries.size() > MAX_ENTRIES)
		m_Entries.pop_front();

	return true;
}

void Client::CChatViewModel::Reset()
{
	m_Entries.clear();
	m_iLastMessageId = 0;
}

const std::deque<Client::CHAT_ENTRY>&
Client::CChatViewModel::Get_Entries() const
{
	return m_Entries;
}
```

```cpp
// FILE: Client/Public/ClientReplication.h
// ADD INCLUDE, PUBLIC GETTER, PRIVATE APPLY, MEMBER

#include "ChatViewModel.h"

const CChatViewModel& Get_ChatViewModel() const;

bool Apply_Chat(const LostArk::Shared::S2C_CHAT& chat);

CChatViewModel m_ChatViewModel;
```

```cpp
// FILE: Client/Private/ClientReplication.cpp
// ADD TO Update() SWITCH

case CLIENT_REPLICATION_EVENT_TYPE::CHAT_RECEIVED:
	allSucceeded =
		Apply_Chat(event.Chat) && allSucceeded;
	break;
```

```cpp
// FILE: Client/Private/ClientReplication.cpp
// APPEND

const Client::CChatViewModel&
Client::CClientReplication::Get_ChatViewModel() const
{
	return m_ChatViewModel;
}

bool Client::CClientReplication::Apply_Chat(
	const LostArk::Shared::S2C_CHAT& chat)
{
	return m_ChatViewModel.Apply(chat);
}

// ADD TO Reset_World()
m_ChatViewModel.Reset();
```

```cpp
// FILE: Client/Public/Level_Baren.h
// ADD PRIVATE DEBUG FUNCTION AND MEMBER

#ifdef _DEBUG
	void Render_ChatPanel();
	char m_szChatInput[LostArk::Shared::MAX_CHAT_TEXT_BYTES + 1] = {};
#endif
```

```cpp
// FILE: Client/Private/Level_Baren.cpp
// ADD INCLUDE

#ifdef _DEBUG
#include "imgui.h"
#endif
```

```cpp
// FILE: Client/Private/Level_Baren.cpp
// ADD TO Render() BEFORE return S_OK

#ifdef _DEBUG
	Render_ChatPanel();
#endif
```

```cpp
// FILE: Client/Private/Level_Baren.cpp
// APPEND BEFORE Create()

#ifdef _DEBUG
void CLevel_Baren::Render_ChatPanel()
{
	if (!ImGui::Begin(
		"Room Chat",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	const auto& entries =
		m_Replication.Get_ChatViewModel().Get_Entries();

	ImGui::BeginChild(
		"ChatHistory",
		ImVec2(420.f, 220.f),
		true);

	for (const Client::CHAT_ENTRY& entry : entries)
	{
		ImGui::TextWrapped(
			"[%u] %s: %s",
			entry.iMessageId,
			entry.strSenderNickname.c_str(),
			entry.strText.c_str());
	}

	ImGui::EndChild();

	const bool submitted = ImGui::InputText(
		"##ChatInput",
		m_szChatInput,
		sizeof(m_szChatInput),
		ImGuiInputTextFlags_EnterReturnsTrue);

	ImGui::SameLine();
	const bool clicked = ImGui::Button("Send");

	if ((submitted || clicked) &&
		LostArk::Shared::Is_Valid_Chat_Text(m_szChatInput) &&
		CNetworkManager::Get().Send_Chat(m_szChatInput))
	{
		m_szChatInput[0] = '\0';
	}

	ImGui::End();
}
#endif
```

```xml
<!-- FILE: Client/Default/Client.vcxproj -->
<ClInclude Include="..\Public\ChatViewModel.h" />
<ClCompile Include="..\Private\ChatViewModel.cpp" />
```

```xml
<!-- FILE: Client/Default/Client.vcxproj.filters -->
<ClInclude Include="..\Public\ChatViewModel.h">
  <Filter>04. Network</Filter>
</ClInclude>
<ClCompile Include="..\Private\ChatViewModel.cpp">
  <Filter>04. Network</Filter>
</ClCompile>
```

```powershell
$msbuild = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
& $msbuild Shared\Default\Shared.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& $msbuild Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& Tools\NetworkProtocolHarness\Bin\Debug\NetworkProtocolHarness.exe
& $msbuild Server\Default\Server.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& $msbuild Client\Default\Client.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
```

```text
NetworkProtocolHarness failures: 0
Client A Send("베른 파티 모집")
Server Handle_Chat SessionId=A PlayerId=A MessageId=1
Client A ChatViewModel MessageId=1 Count=1
Client B ChatViewModel MessageId=1 Count=1
Client A/B sender nickname and text equal
empty/control/oversize/invalid-UTF8 message rejected
fifth message inside one-second window not broadcast
```
