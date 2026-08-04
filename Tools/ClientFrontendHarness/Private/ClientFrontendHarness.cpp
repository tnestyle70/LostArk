#include "LobbyCommandService.h"

#include <cstddef>
#include <iostream>

namespace
{
	struct TEST_RUNNER final
	{
		void Require(const bool_t condition, const char_t* pName)
		{
			if (condition)
			{
				std::cout << "[PASS] " << pName << '\n';
				return;
			}

			++iFailureCount;
			std::cout << "[FAILURE] " << pName << '\n';
		}

		std::size_t iFailureCount = 0;
	};

	void Require_NoPendingCommand(
		TEST_RUNNER& runner,
		const char_t* pName)
	{
		Client::LOBBY_COMMAND command{};
		runner.Require(
			!Client::CLobbyCommandService::Try_Consume(command),
			pName);
	}

	void Test_NormalHandoff(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::TEST, token) &&
			INVALID_LOBBY_COMMAND_TOKEN != token,
			"Normal Handoff Stages Tokenized Test Command");

		LOBBY_COMMAND_TOKEN duplicateToken = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			!CLobbyCommandService::Request(
				LOBBY_STAGE::BERN,
				duplicateToken) &&
			INVALID_LOBBY_COMMAND_TOKEN == duplicateToken,
			"Duplicate Enter Does Not Replace Pending Command");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::TEST == command.eStage &&
			token == command.iToken,
			"Lobby Consumes Exact Handoff Command Once");
		Require_NoPendingCommand(
			runner,
			"Consumed Handoff Leaves No Stale Command");
	}

	void Test_ExactCancellation(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::TEST, token),
			"Cancellation Fixture Stages Test Command");
		runner.Require(
			CLobbyCommandService::Cancel(token, "handoff owner failed"),
			"Exact Token Cancels Pending Command");
		Require_NoPendingCommand(
			runner,
			"Exact Cancellation Leaves No Stale Command");
	}

	void Test_StaleTokenCannotCancelNewCommand(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN oldToken = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::TEST, oldToken) &&
			CLobbyCommandService::Cancel(oldToken, "old handoff cancelled"),
			"Old Handoff Is Cancelled");

		LOBBY_COMMAND_TOKEN newToken = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::BERN, newToken) &&
			newToken > oldToken,
			"New Handoff Receives New Token");
		runner.Require(
			!CLobbyCommandService::Cancel(oldToken, "stale failure"),
			"Stale Failure Cannot Cancel New Handoff");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::BERN == command.eStage &&
			newToken == command.iToken,
			"New Handoff Survives Stale Cancellation");
	}

	void Test_InvalidRequestsPreservePendingCommand(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			!CLobbyCommandService::Request(LOBBY_STAGE::END, token) &&
			INVALID_LOBBY_COMMAND_TOKEN == token,
			"Invalid Stage Is Rejected");

		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::VALTAN, token),
			"Valid Command Is Staged After Invalid Stage");
		runner.Require(
			!CLobbyCommandService::Cancel(
				INVALID_LOBBY_COMMAND_TOKEN,
				"invalid token"),
			"Invalid Token Is Rejected");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::VALTAN == command.eStage &&
			token == command.iToken,
			"Invalid Cancellation Preserves Pending Command");
	}
}

int main()
{
	TEST_RUNNER runner{};

	Test_NormalHandoff(runner);
	Test_ExactCancellation(runner);
	Test_StaleTokenCannotCancelNewCommand(runner);
	Test_InvalidRequestsPreservePendingCommand(runner);

	std::cout << "failures : " << runner.iFailureCount << '\n';
	return 0 == runner.iFailureCount ? 0 : 1;
}
