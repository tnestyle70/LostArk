#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <map>
#include <string>
#include <string_view>

NS_BEGIN(Client)

struct NPC_PLACEMENT_ACTION_BINDING final
{
	std::string strActionId;
	std::string strClipName;
	bool_t isLoop = false;
	f32_t fPlaybackRate = 1.f;
	f32_t fBlendSeconds = 0.f;
};

struct NPC_PLACEMENT_PRESENTATION_ENTRY final
{
	std::string strPlacementId;
	/* Empty means the publisher wrote JSON null and the actor catalog owns the
	default. Empty clip names are rejected while loading, so the two meanings do
	not overlap. */
	std::string strIdleClip;
	std::string strWalkClip;
	std::map<std::string, NPC_PLACEMENT_ACTION_BINDING, std::less<>>
		ActionBindings;
};

class CNpcPlacementPresentationService final
{
public:
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static HRESULT Load(uint32_t iLevelIndex, const char_t* pWorldId);
	static bool_t Try_Get_Presentation(
		uint32_t iLevelIndex,
		std::string_view placementId,
		NPC_PLACEMENT_PRESENTATION_ENTRY& outEntry);
	static const std::string& Get_Status();
};

NS_END
