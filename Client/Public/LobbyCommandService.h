#pragma once

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
	std::string strNickName;
};

class CLobbyCommandService final
{
public:
	static bool_t Request_EnterWorld(
		int32_t iCharacterSlot,
		LostArk::Shared::WORLD_ID eWorldId,
		std::string_view nickName);
	static bool_t Try_Consume(LOBBY_ENTER_COMMAND& outCommand);
	static const std::string& Get_Status();
};

NS_END
