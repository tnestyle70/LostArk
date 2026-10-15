// 정상 서버 수명과 명시적 packaged reset 진입점을 호출한다.
#include "ServerApp.h"
// 자유 함수로 노출한 기존 계약 검사 실행 경로를 선택한다.
#include "ServerGameplayContractTests.h"
// 분리된 기능별 계약 검사를 named runner의 정적 진입점으로 호출한다.
#include "ServerGameplayContractTests_Runner.h"
// 옵션 숫자를 locale 없이 읽고 입력 끝까지 소비했는지 확인한다.
#include <charconv>
// 종료 시간·포트 옵션을 고정 폭 정수로 검증한 뒤 ServerApp에 넘긴다.
#include <cstdint>
// 잘못된 옵션의 이유와 사용법을 stderr에 출력한다.
#include <iostream>
// bind 주소를 ServerApp이 사용하는 동안 유지할 소유 문자열로 보관한다.
#include <string>
// 프로세스 인자 문자열을 복사 없이 비교하고 숫자 파싱 범위로 제공한다.
#include <string_view>

#ifdef _DEBUG
// Debug CRT assertion 보고 대상을 stderr로 지정해 콘솔 실행에서 진단을 남긴다.
#include <crtdbg.h>
#endif

// 프로세스 시작 인자를 검사해 기존 계약 검사, 오프라인 reset, 정상 서버 실행 중 하나로 진입한다.
// arguments 문자열은 main 호출 동안 런타임이 유지한다. 잘못된 일반 옵션은 2를 반환하고,
// 선택된 검사/reset/ServerApp 경로의 종료 코드는 그대로 호출 환경에 돌려준다.
int main(const int argumentCount, char **arguments)
{
#ifdef _DEBUG
	// Debug assertion을 대화상자 대신 stderr로 보내 자동 실행에서도 진단을 수집할 수 있게 한다.
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
	// 계약 검사 옵션은 단독 인자일 때만 선택한다. 검사 함수가 별도 fixture를 소유하며
	// 정상 ServerApp::Run과 함께 실행하지 않고 그 검사 결과로 바로 종료한다.
	if (2 == argumentCount && std::string_view(arguments[1]) == "--contract-test")
	{
		return LostArk::Server::Run_ServerGameplayContractTests();
	}
	if (2 == argumentCount && std::string_view(arguments[1]) == "--character-admission-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_CharacterAdmissionOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--maharaka-ai-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_MaharakaAI();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--colosseum-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ColosseumOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--guide-ai-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_GuideAI();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--valtan-pattern-control-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ValtanPatternControl();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--battle-items-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_BattleItemsOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--character-state-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_CharacterStateOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--room-ping-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_RoomPing();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-draft-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_KoukuDraft();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-product-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_KoukuProductOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--valtan-presentation-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ValtanPresentationOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--numeric-balance-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_NumericBalanceOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--valtan-lifecycle-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ValtanLifecycleOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--colosseum-match-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ColosseumMatch();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--colosseum-combat-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ColosseumCombat();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--skill-stages-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_SkillStagesOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-dice-hit-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_KoukuDiceDamageContracts();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-raid-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_KoukuRaid();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--npc-raid-return-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_NpcRaidReturn();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-bundle-contract-test")
		return LostArk::Server::Run_ServerGameplayContractTests(false, false, true);
	if (2 == argumentCount && std::string_view(arguments[1]) == "--card-maze-contract-test")
		return LostArk::Server::Run_ServerCardMazeContractTests();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--bingo-contract-test")
		return LostArk::Server::Run_ServerBingoContractTests();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--vehicle-riding-contract-test")
		return LostArk::Server::Run_ServerVehicleRidingContractTests();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--debug-teleport-contract-test")
	{
		return LostArk::Server::Run_ServerGameplayContractTests(false, true);
	}
	if (2 == argumentCount && std::string_view(arguments[1]) == "--world-playback-contract-test")
	{
		return LostArk::Server::Run_ServerGameplayContractTests(false, false, false, true);
	}
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-object-overlap-contract-test")
	{
		return LostArk::Server::Run_ServerKoukuObjectOverlapContractTests();
	}
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-showtime-bomb-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ShowtimeBombs();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-support-surface-contract-test")
		return LostArk::Server::Run_ServerKoukuSupportSurfaceContractTests();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--valtan-arena-support-contract-test")
	{
		return LostArk::Server::CServerGameplayContractRunner::Run_ValtanArenaSupport();
	}
	if (2 == argumentCount && std::string_view(arguments[1]) == "--navigation-contract-test")
	{
		return LostArk::Server::Run_ServerNavigationContractTests();
	}
	if (2 == argumentCount && std::string_view(arguments[1]) == "--dimensionmaster-ground-target-contract")
	{
		return LostArk::Server::Run_ServerGameplayContractTests(true);
	}
	// 명시적 reset은 네트워크 listener를 시작하지 않는 별도 경로다.
	// 실제 Debug 허용·process mutex·디스크 세대 검증은 ServerApp의 reset 함수가 맡는다.
	if (2 == argumentCount && std::string_view(arguments[1]) == "--reset-valtan-runtime-to-packaged")
	{
		return LostArk::Server::CServerApp::Reset_ValtanRuntimeToPackaged();
	}
	// 일반 실행 기본값을 준비한다. timeout 0은 시간 제한 없음이며 bind 주소는 수신 인터페이스다.
	// 0.0.0.0을 클라이언트 접속 주소로 전달하는 설정은 이 진입점의 역할이 아니다.
	std::uint32_t automaticShutdownMilliseconds = 0;
	std::uint32_t serverPort = 7777u;
	std::string bindAddress = "0.0.0.0";
	// 값의 기본값과 옵션 지정 여부를 분리해 같은 옵션의 중복 입력을 거부한다.
	bool hasSmokeTimeout = false;
	bool hasBindAddress = false;
	bool headless = false;
	bool hasPort = false;
	// 값이 필요한 옵션은 뒤 인자 존재를 확인한 뒤 index를 한 번 더 전진시킨다.
	// 숫자 파싱은 일부 prefix만 성공한 문자열을 허용하지 않고 전체 소비와 범위를 함께 검사한다.
	for (int index = 1; index < argumentCount; ++index)
	{
		const std::string_view argument(arguments[index]);
		// 자동 종료는 밀리초 단위 100~60000만 받는다. 일반 무기한 실행과 검증용 시간 제한을 구분한다.
		if ("--smoke-timeout-ms" == argument)
		{
			if (hasSmokeTimeout || index + 1 >= argumentCount)
			{
				std::cerr << "--smoke-timeout-ms requires one value.\n";
				return 2;
			}
			const std::string_view value(arguments[++index]);
			const auto result =
			    std::from_chars(value.data(), value.data() + value.size(), automaticShutdownMilliseconds);
			if (result.ec != std::errc{} || result.ptr != value.data() + value.size() ||
			    automaticShutdownMilliseconds < 100u || automaticShutdownMilliseconds > 60000u)
			{
				std::cerr << "Smoke timeout must be an integer from 100 to 60000.\n";
				return 2;
			}
			hasSmokeTimeout = true;
			continue;
		}
		// 여기서는 빈 값·길이만 검사한다. 실제 IPv4 해석과 bind 실패는 TcpListener::Open이 진단한다.
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
		// 콘솔 Enter 대기를 사용하지 않도록 요청한다. 실제 stdin 종류와 최종 대기 방식은 Run이 결정한다.
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
		// 넓은 uint32 임시 값으로 1~65535 범위를 확인한 뒤 마지막 Run 호출에서 uint16으로 좁힌다.
		if ("--port" == argument)
		{
			if (hasPort || index + 1 >= argumentCount)
			{
				std::cerr << "--port requires one value.\n";
				return 2;
			}
			const std::string_view value(arguments[++index]);
			const auto result = std::from_chars(value.data(), value.data() + value.size(), serverPort);
			if (result.ec != std::errc{} || result.ptr != value.data() + value.size() || 0u == serverPort ||
			    serverPort > 65535u)
			{
				std::cerr << "Port must be an integer from 1 to 65535.\n";
				return 2;
			}
			hasPort = true;
			continue;
		}

		std::cerr
		    << "Usage: Server [--contract-test | "
		       "--kouku-object-overlap-contract-test | --kouku-support-surface-contract-test | --kouku-showtime-bomb-contract-test | "
		       "--kouku-bundle-contract-test | --card-maze-contract-test | "
		       "--kouku-raid-contract-test | --kouku-dice-hit-contract-test | --kouku-draft-contract-test | --kouku-product-contract-test | "
		       "--valtan-lifecycle-contract-test | --valtan-presentation-contract-test | --skill-stages-contract-test | --colosseum-combat-contract-test | "
		       "--bingo-contract-test | --vehicle-riding-contract-test | "
		       "--navigation-contract-test | --valtan-arena-support-contract-test | --maharaka-ai-contract-test | "
		       "--debug-teleport-contract-test | "
		       "--dimensionmaster-ground-target-contract | "
		       "--reset-valtan-runtime-to-packaged | "
		       "--bind-address IPv4] [--port 1..65535] "
		       "[--smoke-timeout-ms 100..60000]\n";
		return 2;
	}
	// 옵션 검증이 끝난 뒤 서버 객체를 만들고 동기 Run에 수명을 맡긴다.
	// Run이 조기에 실패해도 지역 객체 소멸자가 Shutdown 정리를 수행한다.
	LostArk::Server::CServerApp serverApp;
	return serverApp.Run(automaticShutdownMilliseconds, bindAddress, static_cast<std::uint16_t>(serverPort), headless);
}
