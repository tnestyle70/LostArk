#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

NS_BEGIN(Client)

enum class CLIENT_SCENARIO
{
	FRONT_LOBBY,
	WORLD_BERN,
	RAID_VALTAN_ARENA,
	DEVELOPMENT_TRAINING_GROUND,
	DEVELOPMENT_MAP,
	DEVELOPMENT_CHARACTER,
	DEVELOPMENT_HDR,
	DEVELOPMENT_EFFECT,
	DEVELOPMENT_UI,
	END
};

enum class CLIENT_ENTRY_MODE
{
	LOCAL_PREVIEW,
	MULTIPLAYER,
	END
};

struct CLIENT_LAUNCH_OPTIONS final
{
	CLIENT_SCENARIO eScenario = CLIENT_SCENARIO::FRONT_LOBBY;
	LEVEL eStartLevel = LEVEL::LOBBY;
	std::string strScenarioId = "front.lobby";
	std::optional<std::filesystem::path> ReportPath;
	std::optional<std::wstring> EffectAssetId;
	std::optional<LostArk::Shared::CHARACTER_CLASS_ID> SelectedCharacterClass;
	CLIENT_ENTRY_MODE eEntryMode = CLIENT_ENTRY_MODE::LOCAL_PREVIEW;
	std::string strServerHost = "127.0.0.1";
	uint16_t iServerPort = 7777;
	uint32_t iTimeoutMs = 30000;
	bool_t isSmokeRun = false;
	bool_t isDisconnectRecoverySmoke = false;
	bool_t isEnterApprovalTimeoutSmoke = false;
	bool_t isAutoActivate = true;
	bool_t isOfflinePreview = true;
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
	static bool_t Select_RuntimeCharacterClass(
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	static bool_t Select_RuntimeEntryMode(CLIENT_ENTRY_MODE eEntryMode);
	static bool_t Set_RuntimeServerEndpoint(
		std::string_view host,
		uint16_t port);

private:
	static bool_t Parse(
		int32_t argumentCount,
		wchar_t** ppArguments,
		CLIENT_LAUNCH_OPTIONS& outOptions,
		std::wstring& outError);
};

NS_END
