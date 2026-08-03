#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>
#include <string_view>

NS_BEGIN(Client)

class CCharacter;
class CClientReplication;

class COfflinePlayerPreview final
{
public:
	static bool_t Spawn(
		CClientReplication& replication,
		std::string_view areaId,
		std::string& outStatus);
	static bool_t Matches_ActiveCharacter(
		const std::shared_ptr<CCharacter>& character);
	static const std::string& Get_ActivePlacementId();
	static void Reset();
};

NS_END
