#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "LobbyCommandService.h"

#include <string>

NS_BEGIN(Client)

enum class LEVEL_TRANSITION_PHASE
{
	LOAD,
	ACTIVATE
};

struct LEVEL_TRANSITION_REQUEST final
{
	LEVEL_TRANSITION_PHASE ePhase = LEVEL_TRANSITION_PHASE::LOAD;
	LEVEL eTargetLevel = LEVEL::END;
	std::string strSource;
	LOBBY_COMMAND_TOKEN iLobbyCommandToken =
		INVALID_LOBBY_COMMAND_TOKEN;
};

enum class SERVER_WORLD_TRANSFER_PUMP_RESULT
{
	NONE,
	REQUESTED,
	RECOVERY_REQUESTED
};

class CLevelTransitionService final
{
public:
	static bool_t Request_Load(
		LEVEL eTargetLevel,
		const char_t* pSource,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	static bool_t Request_Activation(
		LEVEL eTargetLevel,
		const char_t* pSource,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	static bool_t Try_Consume(LEVEL_TRANSITION_REQUEST& outRequest);
	static bool_t Is_Pending();
	static std::string Get_Status();
	static void Report_LoadFailure(HRESULT result);
	static bool_t Try_ConsumeLoadFailure(HRESULT& outResult);
	static SERVER_WORLD_TRANSFER_PUMP_RESULT
		Pump_ServerApprovedWorldTransfer(LEVEL currentLevel);

private:
	static bool_t Request(
		LEVEL_TRANSITION_PHASE ePhase,
		LEVEL eTargetLevel,
		const char_t* pSource,
		LOBBY_COMMAND_TOKEN lobbyCommandToken);
};

NS_END
