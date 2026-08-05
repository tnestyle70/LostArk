#pragma once

#include "Engine_Defines.h"

#include <cstdint>
#include <string>

NS_BEGIN(Client)

enum class LOBBY_STAGE
{
	TEST,
	CHARACTER_SELECT,
	VALTAN,
	BERN,
	END
};

enum class LOBBY_COMMAND_PURPOSE
{
	GAMEPLAY,
	MAP_EDITOR_WORKSPACE,
	END
};

struct LOBBY_COMMAND final
{
	LOBBY_STAGE eStage = LOBBY_STAGE::END;
	LOBBY_COMMAND_PURPOSE ePurpose = LOBBY_COMMAND_PURPOSE::GAMEPLAY;
	std::uint64_t iToken = 0;
};

using LOBBY_COMMAND_TOKEN = std::uint64_t;
inline constexpr LOBBY_COMMAND_TOKEN INVALID_LOBBY_COMMAND_TOKEN = 0;

class CLobbyCommandService final
{
public:
	static bool_t Request(LOBBY_STAGE eStage);
	static bool_t Request(
		LOBBY_STAGE eStage,
		LOBBY_COMMAND_PURPOSE purpose);
	static bool_t Request(
		LOBBY_STAGE eStage,
		LOBBY_COMMAND_TOKEN& outToken);
	static bool_t Request(
		LOBBY_STAGE eStage,
		LOBBY_COMMAND_PURPOSE purpose,
		LOBBY_COMMAND_TOKEN& outToken);
	static bool_t Cancel(
		LOBBY_COMMAND_TOKEN token,
		const char_t* pReason);
	static bool_t Try_Consume(LOBBY_COMMAND& outCommand);
	static std::string Get_Status();
};

NS_END
