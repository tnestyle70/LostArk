#pragma once

#include "Client_Defines.h"
#include "ClientSessionDiagnostic.h"
#include "Engine_Defines.h"
#include "LobbyCommandService.h"
#include "Network/PacketMessages.h"

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
	/* Copy of the pending request without consuming it (false when none). Lets the frame that
	still renders the source level see where the next Apply_LevelRequest will go. */
	static bool_t Peek_Pending(LEVEL_TRANSITION_REQUEST& outRequest);
	/* Colosseum match loading: the local character's portrait, drawn once in Bern on the frame
	before the transfer, so the loading screen can show the customized 3D character after the
	live character is gone. nullptr clears it (the loading screen then keeps its 2D art). */
	static void Set_TransferPortraitSRV(ComPtr<ID3D11ShaderResourceView> pSRV);
	static ComPtr<ID3D11ShaderResourceView> Get_TransferPortraitSRV();
	/* Colosseum match loading: the Server-decided roster (S2C_COLOSSEUM_MATCH_FOUND) is stored the
	moment it arrives in Bern, ahead of the world transfer, so the loading screen can lay out the
	right number of cards. The loading screen reads it once and clears it. */
	static void Set_ColosseumMatch(const LostArk::Shared::S2C_COLOSSEUM_MATCH_FOUND& match);
	static bool_t Try_Get_ColosseumMatch(LostArk::Shared::S2C_COLOSSEUM_MATCH_FOUND& outMatch);
	static void Clear_ColosseumMatch();
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
	/* Level the last Server-approved world transfer left from (LEVEL::END when the newest
	request was not one). Lets the loading screen tell "Bern from Maharaka" from a normal
	Bern entry without adding anything to the protocol. */
	static LEVEL Get_LastWorldTransferOrigin();

private:
	static bool_t Request(
		LEVEL_TRANSITION_PHASE ePhase,
		LEVEL eTargetLevel,
		const char_t* pSource,
		LOBBY_COMMAND_TOKEN lobbyCommandToken);
};

NS_END
