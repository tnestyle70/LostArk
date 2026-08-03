#include "ClientLaunchOptions.h"
#include "LevelCatalog.h"

#include <algorithm>
#include <charconv>
#include <cctype>
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

	bool_t ParseCharacterClass(
		const std::wstring& value,
		LostArk::Shared::CHARACTER_CLASS_ID& outClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		if (L"lance-master" == value)
			outClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		else if (L"gunslinger" == value)
			outClass = CHARACTER_CLASS_ID::GUNSLINGER;
		else if (L"slayer" == value)
			outClass = CHARACTER_CLASS_ID::SLAYER;
		else if (L"artist" == value)
			outClass = CHARACTER_CLASS_ID::ARTIST;
		else
			return false;
		return true;
	}

	bool_t IsValidServerHost(const std::string_view host)
	{
		if (host.empty() || host.size() > 63u)
			return false;

		return std::all_of(
			host.begin(),
			host.end(),
			[](const char_t value)
			{
				return std::isalnum(static_cast<unsigned char>(value)) ||
					'.' == value || '-' == value;
			});
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

bool_t CClientLaunchOptions::Select_RuntimeCharacterClass(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(
		characterClass))
	{
		return false;
	}

	g_Options.SelectedCharacterClass = characterClass;
	return true;
}

bool_t CClientLaunchOptions::Select_RuntimeEntryMode(
	const CLIENT_ENTRY_MODE eEntryMode)
{
	if (CLIENT_ENTRY_MODE::LOCAL_PREVIEW != eEntryMode &&
		CLIENT_ENTRY_MODE::MULTIPLAYER != eEntryMode)
	{
		return false;
	}

	g_Options.eEntryMode = eEntryMode;
	g_Options.isOfflinePreview =
		CLIENT_ENTRY_MODE::LOCAL_PREVIEW == eEntryMode;
	return true;
}

bool_t CClientLaunchOptions::Set_RuntimeServerEndpoint(
	const std::string_view host,
	const uint16_t port)
{
	if (!IsValidServerHost(host) || 0u == port)
		return false;

	g_Options.strServerHost = host;
	g_Options.iServerPort = port;
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
	bool_t hasCharacterClass = false;
	bool_t hasEntryMode = false;
	bool_t hasServerHost = false;
	bool_t hasServerPort = false;

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
		if (L"--expect-disconnect-recovery" == argument)
		{
			outOptions.isDisconnectRecoverySmoke = true;
			continue;
		}
		if (L"--expect-enter-approval-timeout" == argument)
		{
			outOptions.isEnterApprovalTimeoutSmoke = true;
			continue;
		}
		if (StartsWith(argument, L"--entry-mode="))
		{
			if (hasEntryMode)
			{
				outError = L"--entry-mode may be specified only once.";
				return false;
			}
			const std::wstring value = argument.substr(
				std::wstring(L"--entry-mode=").size());
			if (L"local" == value)
			{
				outOptions.eEntryMode = CLIENT_ENTRY_MODE::LOCAL_PREVIEW;
				outOptions.isOfflinePreview = true;
			}
			else if (L"multiplayer" == value)
			{
				outOptions.eEntryMode = CLIENT_ENTRY_MODE::MULTIPLAYER;
				outOptions.isOfflinePreview = false;
			}
			else
			{
				outError = L"--entry-mode must be local or multiplayer.";
				return false;
			}
			hasEntryMode = true;
			continue;
		}
		if (StartsWith(argument, L"--server-host="))
		{
			if (hasServerHost)
			{
				outError = L"--server-host may be specified only once.";
				return false;
			}
			std::string host;
			const std::wstring value = argument.substr(
				std::wstring(L"--server-host=").size());
			if (!TryNarrowStableId(value, host) || !IsValidServerHost(host))
			{
				outError = L"--server-host is invalid.";
				return false;
			}
			outOptions.strServerHost = std::move(host);
			hasServerHost = true;
			continue;
		}
		if (StartsWith(argument, L"--server-port="))
		{
			if (hasServerPort)
			{
				outError = L"--server-port may be specified only once.";
				return false;
			}
			uint32_t port = {};
			const std::wstring value = argument.substr(
				std::wstring(L"--server-port=").size());
			if (!ParseUnsigned(value, port) || 0u == port || port > UINT16_MAX)
			{
				outError = L"--server-port must be between 1 and 65535.";
				return false;
			}
			outOptions.iServerPort = static_cast<uint16_t>(port);
			hasServerPort = true;
			continue;
		}
		if (StartsWith(argument, L"--character-class="))
		{
			if (hasCharacterClass)
			{
				outError = L"--character-class may be specified only once.";
				return false;
			}
			LostArk::Shared::CHARACTER_CLASS_ID characterClass =
				LostArk::Shared::CHARACTER_CLASS_ID::END;
			const std::wstring value = argument.substr(
				std::wstring(L"--character-class=").size());
			if (!ParseCharacterClass(value, characterClass))
			{
				outError = L"Unknown character class: " + value;
				return false;
			}
			outOptions.SelectedCharacterClass = characterClass;
			hasCharacterClass = true;
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
