#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>
#include <string_view>

NS_BEGIN(Client)

class CNpcPlacementPresentationService final
{
public:
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static HRESULT Load(uint32_t iLevelIndex, const char_t* pWorldId);
	static const std::string* Find_IdleClip(
		uint32_t iLevelIndex,
		std::string_view placementId);
	static const std::string& Get_Status();
};

NS_END
