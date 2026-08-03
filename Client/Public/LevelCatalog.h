#pragma once

#include "ClientLaunchOptions.h"
#include "Client_Defines.h"
#include "MapLoadScope.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

struct LEVEL_CATALOG_ENTRY final
{
	CLIENT_SCENARIO eScenario = CLIENT_SCENARIO::END;
	LEVEL eLevel = LEVEL::END;
	std::string strStableId;
	std::string strKind;
	std::string strMapAreaId;
	MAP_LOAD_SCOPE MapLoadScope;
	std::vector<std::string> AssetDomains;
	std::vector<std::string> Tools;
};

class CLevelCatalog final
{
public:
	static bool_t Initialize();
	static const LEVEL_CATALOG_ENTRY* Find(CLIENT_SCENARIO eScenario);
	static const LEVEL_CATALOG_ENTRY* Find(const std::string& stableId);
	static const std::vector<LEVEL_CATALOG_ENTRY>& Get_Entries();
	static const std::string& Get_Status();
};

NS_END
