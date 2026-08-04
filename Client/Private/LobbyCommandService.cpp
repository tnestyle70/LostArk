#include "LobbyCommandService.h"

#include <limits>
#include <mutex>
#include <optional>

namespace
{
	std::mutex g_CommandMutex;
	std::optional<Client::LOBBY_COMMAND> g_PendingCommand;
	Client::LOBBY_COMMAND_TOKEN g_iNextToken = 1;
	std::string g_Status = "No lobby command is pending.";
}

bool_t Client::CLobbyCommandService::Request(const LOBBY_STAGE eStage)
{
	LOBBY_COMMAND_TOKEN ignoredToken = INVALID_LOBBY_COMMAND_TOKEN;
	return Request(eStage, ignoredToken);
}

bool_t Client::CLobbyCommandService::Request(
	const LOBBY_STAGE eStage,
	LOBBY_COMMAND_TOKEN& outToken)
{
	outToken = INVALID_LOBBY_COMMAND_TOKEN;
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
	if ((std::numeric_limits<LOBBY_COMMAND_TOKEN>::max)() == g_iNextToken)
	{
		g_Status = "Lobby command token space is exhausted.";
		return false;
	}

	outToken = g_iNextToken++;
	g_PendingCommand = LOBBY_COMMAND{ eStage, outToken };
	g_Status = "Lobby command staged with token " +
		std::to_string(outToken) + ".";
	return true;
}

bool_t Client::CLobbyCommandService::Cancel(
	const LOBBY_COMMAND_TOKEN token,
	const char_t* pReason)
{
	if (INVALID_LOBBY_COMMAND_TOKEN == token ||
		nullptr == pReason || '\0' == *pReason)
	{
		std::scoped_lock lock{ g_CommandMutex };
		g_Status = "Rejected invalid lobby command cancellation.";
		return false;
	}

	std::scoped_lock lock{ g_CommandMutex };
	if (!g_PendingCommand.has_value() ||
		g_PendingCommand->iToken != token)
	{
		g_Status = "Lobby command cancellation token did not match.";
		return false;
	}

	g_PendingCommand.reset();
	g_Status = std::string("Lobby command cancelled: ") + pReason;
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
