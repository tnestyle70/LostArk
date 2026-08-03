#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <optional>
#include <string>

NS_BEGIN(Client)

enum class CLIENT_SCENARIO
{
	FRONT_LOBBY,
	WORLD_BERN,
	RAID_VALTAN_ARENA,
	DEVELOPMENT_MAP,
	DEVELOPMENT_CHARACTER,
	DEVELOPMENT_HDR,
	DEVELOPMENT_EFFECT,
	DEVELOPMENT_UI,
	END
};

struct CLIENT_LAUNCH_OPTIONS final
{
	CLIENT_SCENARIO eScenario = CLIENT_SCENARIO::FRONT_LOBBY;
	LEVEL eStartLevel = LEVEL::LOBBY;
	std::string strScenarioId = "front.lobby";
	std::optional<std::filesystem::path> ReportPath;
	std::optional<std::wstring> EffectAssetId;
	uint32_t iTimeoutMs = 30000;
	bool_t isSmokeRun = false;
	bool_t isAutoActivate = true;
	bool_t isEffectAutoExit = false;
	bool_t isEffectProfile = false;
};

class CClientLaunchOptions final
{
public:
	static HRESULT Initialize();
	static const CLIENT_LAUNCH_OPTIONS& Get();
	static const std::wstring& Get_Error();
	static bool_t Is_Initialized();
	static bool_t Select_RuntimeScenario(CLIENT_SCENARIO eScenario);

private:
	static bool_t Parse(
		int32_t argumentCount,
		wchar_t** ppArguments,
		CLIENT_LAUNCH_OPTIONS& outOptions,
		std::wstring& outError);
};

NS_END
