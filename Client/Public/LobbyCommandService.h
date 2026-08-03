#pragma once

#include "ClientLaunchOptions.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <cstdint>
#include <string>
#include <string_view>

NS_BEGIN(Client)

struct LOBBY_ENTER_COMMAND final
{
	int32_t iCharacterSlot = -1;
	LostArk::Shared::WORLD_ID eWorldId =
		LostArk::Shared::WORLD_ID::END;
	CLIENT_ENTRY_MODE eEntryMode = CLIENT_ENTRY_MODE::END;
	std::string strNickName;
	std::string strServerHost;
	std::uint16_t iServerPort = 0;
};

class CLobbyCommandService final
{
public:
	static bool_t Request_EnterWorld(
		int32_t iCharacterSlot,
		LostArk::Shared::WORLD_ID eWorldId,
		std::string_view nickName,
		CLIENT_ENTRY_MODE eEntryMode,
		std::string_view serverHost,
		std::uint16_t serverPort);
	static bool_t Try_Consume(LOBBY_ENTER_COMMAND& outCommand);
	static const std::string& Get_Status();
};

NS_END
