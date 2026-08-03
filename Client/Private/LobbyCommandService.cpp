#include "LobbyCommandService.h"

#include <mutex>
#include <optional>

namespace
{
	std::mutex g_CommandMutex;
	std::optional<Client::LOBBY_ENTER_COMMAND> g_PendingCommand;
	std::string g_Status = "No lobby command is pending.";
}

bool_t Client::CLobbyCommandService::Request_EnterWorld(
	const int32_t iCharacterSlot,
	const LostArk::Shared::WORLD_ID eWorldId,
	const std::string_view nickName)
{
	if (iCharacterSlot < 0 ||
		!LostArk::Shared::Is_Known_World_Id(eWorldId) ||
		nickName.empty() ||
		nickName.size() > LostArk::Shared::MAX_NICKNAME_BYTES)
	{
		g_Status = "Rejected invalid lobby enter command.";
		return false;
	}

	std::scoped_lock lock{ g_CommandMutex };
	if (g_PendingCommand.has_value())
	{
		g_Status = "A lobby command is already pending.";
		return false;
	}

	g_PendingCommand = LOBBY_ENTER_COMMAND{
		iCharacterSlot,
		eWorldId,
		std::string{ nickName }
	};
	g_Status = "Lobby enter command staged.";
	return true;
}

bool_t Client::CLobbyCommandService::Try_Consume(
	LOBBY_ENTER_COMMAND& outCommand)
{
	std::scoped_lock lock{ g_CommandMutex };
	if (!g_PendingCommand.has_value())
		return false;

	outCommand = std::move(*g_PendingCommand);
	g_PendingCommand.reset();
	g_Status = "Lobby enter command consumed.";
	return true;
}

const std::string& Client::CLobbyCommandService::Get_Status()
{
	return g_Status;
}
