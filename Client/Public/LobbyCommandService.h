#pragma once

#include "Engine_Defines.h"

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

struct LOBBY_COMMAND final
{
	LOBBY_STAGE eStage = LOBBY_STAGE::END;
};

class CLobbyCommandService final
{
public:
	static bool_t Request(LOBBY_STAGE eStage);
	static bool_t Try_Consume(LOBBY_COMMAND& outCommand);
	static std::string Get_Status();
};

NS_END
