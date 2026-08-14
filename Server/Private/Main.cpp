#include "ServerApp.h"
#include "ServerGameplayContractTests.h"

#include <charconv>
#include <cstdint>
#include <iostream>
#include <string>
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
	std::uint32_t serverPort = 7777u;
	std::string bindAddress = "0.0.0.0";
	bool hasSmokeTimeout = false;
	bool hasBindAddress = false;
	bool headless = false;
	bool hasPort = false;
	for (int index = 1; index < argumentCount; ++index)
	{
		const std::string_view argument(arguments[index]);
		if ("--smoke-timeout-ms" == argument)
		{
			if (hasSmokeTimeout || index + 1 >= argumentCount)
			{
				std::cerr << "--smoke-timeout-ms requires one value.\n";
				return 2;
			}
			const std::string_view value(arguments[++index]);
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
			hasSmokeTimeout = true;
			continue;
		}
		if ("--bind-address" == argument)
		{
			if (hasBindAddress || index + 1 >= argumentCount)
			{
				std::cerr << "--bind-address requires one IPv4 value.\n";
				return 2;
			}
			bindAddress = arguments[++index];
			if (bindAddress.empty() || bindAddress.size() > 63u)
			{
				std::cerr << "Bind address is invalid.\n";
				return 2;
			}
			hasBindAddress = true;
			continue;
		}
		if ("--headless" == argument)
		{
			if (headless)
			{
				std::cerr << "--headless may be specified only once.\n";
				return 2;
			}
			headless = true;
			continue;
		}
		if ("--port" == argument)
		{
			if (hasPort || index + 1 >= argumentCount)
			{
				std::cerr << "--port requires one value.\n";
				return 2;
			}
			const std::string_view value(arguments[++index]);
			const auto result = std::from_chars(
				value.data(), value.data() + value.size(), serverPort);
			if (result.ec != std::errc{} ||
				result.ptr != value.data() + value.size() ||
				0u == serverPort || serverPort > 65535u)
			{
				std::cerr << "Port must be an integer from 1 to 65535.\n";
				return 2;
			}
			hasPort = true;
			continue;
		}

		std::cerr << "Usage: Server [--bind-address IPv4] [--port 1..65535] "
			"[--smoke-timeout-ms 100..60000]\n";
		return 2;
	}
	LostArk::Server::CServerApp serverApp;
	return serverApp.Run(
		automaticShutdownMilliseconds,
		bindAddress,
		static_cast<std::uint16_t>(serverPort),
		headless);
}
