#include "ServerApp.h"
#include "ServerGameplayContractTests.h"

#include <charconv>
#include <cstdint>
#include <iostream>
#include <string_view>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

int main(const int argumentCount, char** arguments)
{
#ifdef _DEBUG
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
	if (2 == argumentCount &&
		std::string_view(arguments[1]) == "--contract-test")
	{
		return LostArk::Server::Run_ServerGameplayContractTests();
	}
	std::uint32_t automaticShutdownMilliseconds = 0;
	if (argumentCount != 1)
	{
		if (argumentCount != 3 ||
			std::string_view(arguments[1]) != "--smoke-timeout-ms")
		{
			std::cerr << "Usage: Server [--smoke-timeout-ms 100..60000]\n";
			return 2;
		}
		const std::string_view value(arguments[2]);
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(),
			automaticShutdownMilliseconds);
		if (result.ec != std::errc{} ||
			result.ptr != value.data() + value.size() ||
			automaticShutdownMilliseconds < 100u ||
			automaticShutdownMilliseconds > 60000u)
		{
			std::cerr << "Smoke timeout must be an integer from 100 to 60000.\n";
			return 2;
		}
	}
	LostArk::Server::CServerApp serverApp;
	return serverApp.Run(automaticShutdownMilliseconds);
}
