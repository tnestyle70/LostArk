#pragma once

#include "Client_Defines.h"
#include "ClientSessionDiagnostic.h"
#include "Engine_Defines.h"
#include "LobbyCommandService.h"

#include <string>
#include <string_view>

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

struct CLIENT_RECOVERY_DIAGNOSTIC final
{
	LostArk::Shared::SESSION_DIAGNOSTIC_REASON eReason =
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE;
	HRESULT hResult = S_OK;
	std::string strSource;
	std::string strDetail;
	std::uint64_t iOccurredUnixMs = 0u;
	CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT Session;
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
	/* detail names the stage that refused. Reporting an empty detail keeps the
	one already recorded, so the generic activation failure cannot erase it. */
	static void Report_LoadFailure(
		HRESULT result,
		std::string_view detail = {});
	static void Report_Recovery(
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
		std::string_view source,
		std::string_view detail = {},
		HRESULT result = S_OK);
	static void Report_NetworkRecovery(
		std::string_view source,
		std::string_view detail = {});
	static bool_t Try_ConsumeRecovery(
		CLIENT_RECOVERY_DIAGNOSTIC& outDiagnostic);
	static bool_t Try_ConsumeLoadFailure(
		HRESULT& outResult,
		std::string& outDetail);
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
