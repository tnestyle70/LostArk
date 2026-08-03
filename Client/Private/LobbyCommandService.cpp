#include "LobbyCommandService.h"

#include <mutex>
#include <optional>

namespace
{
	std::mutex g_CommandMutex;
	std::optional<Client::LOBBY_COMMAND> g_PendingCommand;
	std::string g_Status = "No lobby command is pending.";
}

bool_t Client::CLobbyCommandService::Request(const LOBBY_STAGE eStage)
{
	if (LOBBY_STAGE::END == eStage)
	{
		std::scoped_lock lock{ g_CommandMutex };
		g_Status = "Rejected invalid lobby command.";
		return false;
	}

	std::scoped_lock lock{ g_CommandMutex };
	if (g_PendingCommand.has_value())
	{
		g_Status = "A lobby command is already pending.";
		return false;
	}

	g_PendingCommand = LOBBY_COMMAND{ eStage };
	g_Status = "Lobby command staged.";
	return true;
}

bool_t Client::CLobbyCommandService::Try_Consume(
	LOBBY_COMMAND& outCommand)
{
	std::scoped_lock lock{ g_CommandMutex };
	if (!g_PendingCommand.has_value())
		return false;

	outCommand = *g_PendingCommand;
	g_PendingCommand.reset();
	g_Status = "Lobby command consumed.";
	return true;
}

std::string Client::CLobbyCommandService::Get_Status()
{
	std::scoped_lock lock{ g_CommandMutex };
	return g_Status;
}
