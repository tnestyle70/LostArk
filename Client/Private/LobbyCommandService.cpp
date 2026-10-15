#include "LobbyCommandService.h"

// token 최대값에 도달하면 접수를 거부해 다음 증가의 wrap을 피한다.
#include <limits>
// 접수·취소·소비·상태 조회가 같은 대기 명령을 동시에 바꾸지 않게 한다.
#include <mutex>
// 명령 한 개의 값과 대기 없음 상태를 함께 저장한다.
#include <optional>

namespace
{
	// 프로세스 수명 대기 칸이다. mutex는 명령과 token·상태 문자열 접근을 함께 보호한다.
	std::mutex g_CommandMutex;
	std::optional<Client::LOBBY_COMMAND> g_PendingCommand;
	Client::LOBBY_COMMAND_TOKEN g_iNextToken = 1;
	std::string g_Status = "No lobby command is pending.";
}

bool_t Client::CLobbyCommandService::Request(const LOBBY_STAGE eStage)
{
	//64 bits, 8 bytes의 LOBBY_COMMAND_TOKEN을 using으로 선언한 것을 사용해서
	//LOBBY_COMMAND_TOKEN으로 사용한다. 그냥 int64_t로 써도 무방하다.
    std::uint64_t ignoredToken = INVALID_LOBBY_COMMAND_TOKEN;
	//LOBBY_COMMAND_TOKEN ignoredToken = INVALID_LOBBY_COMMAND_TOKEN;

	return Request(
		eStage,
		LOBBY_COMMAND_PURPOSE::GAMEPLAY,
		ignoredToken);
}

bool_t Client::CLobbyCommandService::Request(
	const LOBBY_STAGE eStage,
	const LOBBY_COMMAND_PURPOSE purpose)
{
    std::uint64_t ignoredToken = INVALID_LOBBY_COMMAND_TOKEN;
	//LOBBY_COMMAND_TOKEN ignoredToken = INVALID_LOBBY_COMMAND_TOKEN;
	return Request(eStage, purpose, ignoredToken);
}

bool_t Client::CLobbyCommandService::Request(
	const LOBBY_STAGE eStage,
	LOBBY_COMMAND_TOKEN& outToken)
{
	return Request(
		eStage,
		LOBBY_COMMAND_PURPOSE::GAMEPLAY,
		outToken);
}

bool_t Client::CLobbyCommandService::Request(
	const LOBBY_STAGE eStage,
	const LOBBY_COMMAND_PURPOSE purpose,
	LOBBY_COMMAND_TOKEN& outToken)
{
	// END 표식과 TEST 이외의 편집 목적을 거부한다. 아래 접수까지 성공해야 유효 token이 나온다.
	outToken = INVALID_LOBBY_COMMAND_TOKEN;
	if (LOBBY_STAGE::END == eStage ||
		LOBBY_COMMAND_PURPOSE::END == purpose ||
		(LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE == purpose &&
			LOBBY_STAGE::TEST != eStage))
	{
		std::scoped_lock lock{ g_CommandMutex };
		g_Status = "Rejected invalid lobby command.";
		return false;
	}

	std::scoped_lock lock{ g_CommandMutex };
	// 이미 대기 중이면 교체하지 않는다. 소비 또는 일치 token의 취소 뒤에 새 요청을 받을 수 있다.
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
	// 여기서는 명령 값만 저장한다. 접속·Server 승인·Level 전환은 이후 Lobby 소비 경로가 수행한다.
	g_PendingCommand = LOBBY_COMMAND{ eStage, purpose, outToken };

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
