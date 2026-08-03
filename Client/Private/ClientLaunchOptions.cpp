#include "ClientLaunchOptions.h"
#include "LevelCatalog.h"

#include <charconv>
#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")

namespace
{
	using namespace Client;

	CLIENT_LAUNCH_OPTIONS g_Options;
	std::wstring g_Error;
	bool_t g_isInitialized = false;

	bool_t StartsWith(const std::wstring& value, const wchar_t* pPrefix)
	{
		const std::wstring prefix = pPrefix;
		return value.size() >= prefix.size() &&
			0 == value.compare(0, prefix.size(), prefix);
	}

	bool_t TryNarrowStableId(
		const std::wstring& value,
		std::string& outValue)
	{
		outValue.clear();
		outValue.reserve(value.size());
		for (const wchar_t codeUnit : value)
		{
			if (codeUnit > 0x7f)
				return false;
			outValue.push_back(static_cast<char_t>(codeUnit));
		}
		return true;
	}

	bool_t ApplyScenario(
		const std::wstring& scenarioId,
		CLIENT_LAUNCH_OPTIONS& options)
	{
		std::string stableId;
		if (!TryNarrowStableId(scenarioId, stableId))
			return false;
		const LEVEL_CATALOG_ENTRY* pEntry =
			CLevelCatalog::Find(stableId);
		if (nullptr == pEntry)
			return false;

		options.eScenario = pEntry->eScenario;
		options.eStartLevel = LEVEL::LOBBY;
		options.strScenarioId = pEntry->strStableId;
		return true;
	}

	bool_t ParseUnsigned(
		const std::wstring& value,
		uint32_t& outValue)
	{
		if (value.empty())
			return false;

		std::string narrow;
		if (!TryNarrowStableId(value, narrow))
			return false;
		uint32_t parsedValue = {};
		const auto result = std::from_chars(
			narrow.data(),
			narrow.data() + narrow.size(),
			parsedValue);
		if (std::errc{} != result.ec ||
			result.ptr != narrow.data() + narrow.size())
		{
			return false;
		}

		outValue = parsedValue;
		return true;
	}
}

HRESULT CClientLaunchOptions::Initialize()
{
	if (g_isInitialized)
		return S_OK;

	int32_t argumentCount = {};
	wchar_t** ppArguments = CommandLineToArgvW(
		GetCommandLineW(),
		&argumentCount);
	if (nullptr == ppArguments)
	{
		g_Error = L"CommandLineToArgvW failed.";
		return E_FAIL;
	}

	CLIENT_LAUNCH_OPTIONS stagedOptions;
	std::wstring stagedError;
	const bool_t parsed = Parse(
		argumentCount,
		ppArguments,
		stagedOptions,
		stagedError);
	LocalFree(ppArguments);

	if (!parsed)
	{
		g_Error = std::move(stagedError);
		return E_INVALIDARG;
	}

	g_Options = std::move(stagedOptions);
	g_Error.clear();
	g_isInitialized = true;
	return S_OK;
}

const CLIENT_LAUNCH_OPTIONS& CClientLaunchOptions::Get()
{
	return g_Options;
}

const std::wstring& CClientLaunchOptions::Get_Error()
{
	return g_Error;
}

bool_t CClientLaunchOptions::Is_Initialized()
{
	return g_isInitialized;
}

bool_t CClientLaunchOptions::Select_RuntimeScenario(
	const CLIENT_SCENARIO eScenario)
{
	std::wstring scenarioId;
	switch (eScenario)
	{
	case CLIENT_SCENARIO::FRONT_LOBBY:
		scenarioId = L"front.lobby";
		break;
	case CLIENT_SCENARIO::WORLD_BERN:
		scenarioId = L"world.bern";
		break;
	case CLIENT_SCENARIO::RAID_VALTAN_ARENA:
		scenarioId = L"raid.valtan.arena";
		break;
	case CLIENT_SCENARIO::DEVELOPMENT_TRAINING_GROUND:
		scenarioId = L"dev.training.ground";
		break;
	case CLIENT_SCENARIO::DEVELOPMENT_MAP:
		scenarioId = L"dev.map.active";
		break;
	case CLIENT_SCENARIO::DEVELOPMENT_CHARACTER:
		scenarioId = L"asset.character.lance-master";
		break;
	case CLIENT_SCENARIO::DEVELOPMENT_HDR:
		scenarioId = L"render.hdr-readback";
		break;
	case CLIENT_SCENARIO::DEVELOPMENT_EFFECT:
		scenarioId = L"effect.preview";
		break;
	case CLIENT_SCENARIO::DEVELOPMENT_UI:
		scenarioId = L"ui.hud.layout";
		break;
	default:
		return false;
	}

	CLIENT_LAUNCH_OPTIONS stagedOptions = g_Options;
	if (!ApplyScenario(scenarioId, stagedOptions))
		return false;

	g_Options = std::move(stagedOptions);
	return true;
}

bool_t CClientLaunchOptions::Parse(
	const int32_t argumentCount,
	wchar_t** ppArguments,
	CLIENT_LAUNCH_OPTIONS& outOptions,
	std::wstring& outError)
{
	bool_t hasScenario = false;
	bool_t hasTimeout = false;
	bool_t hasReport = false;

	for (int32_t index = 1; index < argumentCount; ++index)
	{
		const std::wstring argument = ppArguments[index];

		if (StartsWith(argument, L"--scenario="))
		{
			if (hasScenario)
			{
				outError = L"--scenario may be specified only once.";
				return false;
			}

			const std::wstring scenarioId =
				argument.substr(std::wstring(L"--scenario=").size());
			if (!ApplyScenario(scenarioId, outOptions))
			{
				outError = L"Unknown scenario: " + scenarioId;
				return false;
			}
			hasScenario = true;
			continue;
		}
		if (L"--smoke" == argument)
		{
			outOptions.isSmokeRun = true;
			continue;
		}
		if (StartsWith(argument, L"--timeout-ms="))
		{
			if (hasTimeout)
			{
				outError = L"--timeout-ms may be specified only once.";
				return false;
			}
			const std::wstring value =
				argument.substr(std::wstring(L"--timeout-ms=").size());
			if (!ParseUnsigned(value, outOptions.iTimeoutMs) ||
				0 == outOptions.iTimeoutMs)
			{
				outError = L"--timeout-ms must be a positive integer.";
				return false;
			}
			hasTimeout = true;
			continue;
		}
		if (StartsWith(argument, L"--report="))
		{
			if (hasReport)
			{
				outError = L"--report may be specified only once.";
				return false;
			}
			const std::wstring value =
				argument.substr(std::wstring(L"--report=").size());
			if (value.empty())
			{
				outError = L"--report requires a path.";
				return false;
			}
			outOptions.ReportPath = std::filesystem::path(value);
			hasReport = true;
			continue;
		}
		if (L"--effect-auto-exit" == argument)
		{
			outOptions.isEffectAutoExit = true;
			continue;
		}
		if (L"--effect-profile" == argument)
		{
			outOptions.isEffectProfile = true;
			continue;
		}
		if (L"--effect-open" == argument)
		{
			if (index + 1 >= argumentCount)
			{
				outError = L"--effect-open requires an asset ID.";
				return false;
			}
			outOptions.EffectAssetId = ppArguments[++index];
			continue;
		}
		if (StartsWith(argument, L"--effect-open="))
		{
			outOptions.EffectAssetId =
				argument.substr(std::wstring(L"--effect-open=").size());
			continue;
		}

		outError = L"Unknown client option: " + argument;
		return false;
	}

	return true;
}
