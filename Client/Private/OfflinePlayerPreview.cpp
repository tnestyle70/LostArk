#include "OfflinePlayerPreview.h"

#include "Character.h"
#include "ClientLaunchOptions.h"
#include "ClientReplication.h"
#include "NetworkManager.h"
#include "ProjectDataRoot.h"
#include "WorldGameplayDocument.h"

#include <algorithm>
#include <filesystem>
#include <vector>

namespace
{
	std::weak_ptr<Client::CCharacter> g_ActiveCharacter;
	std::string g_ActivePlacementId;
}

bool_t Client::COfflinePlayerPreview::Spawn(
	CClientReplication& replication,
	const std::string_view areaId,
	std::string& outStatus)
{
	Reset();
	const CLIENT_LAUNCH_OPTIONS& options = CClientLaunchOptions::Get();
	if (!options.isOfflinePreview ||
		CLIENT_ENTRY_MODE::LOCAL_PREVIEW != options.eEntryMode)
	{
		outStatus = "Local preview spawn rejected outside local entry mode.";
		return false;
	}
	if (CNetworkManager::Get().Is_Connected())
	{
		outStatus = "Local preview spawn rejected while connected to a server.";
		return false;
	}
	if (areaId.empty() || !options.SelectedCharacterClass.has_value() ||
		!LostArk::Shared::Is_Supported_Playable_Character_Class(
			*options.SelectedCharacterClass))
	{
		outStatus = "Local preview area or selected class is invalid.";
		return false;
	}

	const std::filesystem::path documentPath = CProjectDataRoot::Resolve(
		std::filesystem::path("Worlds") /
		std::string(areaId) /
		"Gameplay.world.json");
	if (documentPath.empty())
	{
		outStatus = "Local preview gameplay document escaped the Data root.";
		return false;
	}

	CWorldGameplayDocument document;
	if (!document.Load(documentPath, std::string(areaId), outStatus))
		return false;

	std::vector<const WORLD_GAMEPLAY_PLACEMENT*> playerSpawns;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		document.Get_Placements())
	{
		if (placement.isEnabled &&
			WORLD_PLACEMENT_KIND::PLAYER_SPAWN == placement.eKind)
		{
			playerSpawns.push_back(&placement);
		}
	}
	std::sort(
		playerSpawns.begin(),
		playerSpawns.end(),
		[](const auto* left, const auto* right)
		{
			return left->placementId < right->placementId;
		});
	if (playerSpawns.empty())
	{
		outStatus = "Local preview requires one enabled playerSpawn.";
		return false;
	}

	const WORLD_GAMEPLAY_PLACEMENT& spawn = *playerSpawns.front();
	LOCAL_PREVIEW_PLAYER_DESC desc{};
	desc.eCharacterClass = *options.SelectedCharacterClass;
	desc.strNickName = "Player";
	desc.strPlacementId = spawn.placementId;
	desc.vPosition = spawn.position;
	desc.fYawDegrees = spawn.yawDegrees;
	if (!replication.Spawn_LocalPreview(desc))
	{
		outStatus = "Failed to create the local preview Character.";
		return false;
	}

	const std::shared_ptr<CCharacter> character =
		replication.Get_LocalCharacter();
	if (!replication.Is_LocalPreviewCharacter(character))
	{
		outStatus = "Local preview Character handle did not commit.";
		return false;
	}

	g_ActiveCharacter = character;
	g_ActivePlacementId = spawn.placementId;
	outStatus = "Local preview player ready at " + spawn.placementId;
	return true;
}

bool_t Client::COfflinePlayerPreview::Matches_ActiveCharacter(
	const std::shared_ptr<CCharacter>& character)
{
	return nullptr != character && g_ActiveCharacter.lock() == character;
}

const std::string&
Client::COfflinePlayerPreview::Get_ActivePlacementId()
{
	return g_ActivePlacementId;
}

void Client::COfflinePlayerPreview::Reset()
{
	g_ActiveCharacter.reset();
	g_ActivePlacementId.clear();
}
