#include "MapAuthoringHost.h"

#include "GameInstance.h"
#include "Level_CharacterSelect.h"
#include "Level_Bern.h"
#include "Level_ValtanArena.h"
#include "Level_KakulSaydonArena.h"

IMapAuthoringHost* Client::Find_ActiveMapAuthoringHost()
{
#ifdef _DEBUG
	const uint32_t levelIndex = CGameInstance::Get().Get_CurrentLevelID();
	if (levelIndex == ETOUI(LEVEL::KAKULSAYDON_ARENA))
		return CLevel_KakulSaydonArena::Get_Active();
	if (levelIndex == ETOUI(LEVEL::CHARACTER_SELECT))
		return CLevel_CharacterSelect::Get_Active();
	if (levelIndex == ETOUI(LEVEL::BERN))
		return CLevel_Bern::Get_Active();
	if (levelIndex == ETOUI(LEVEL::VALTAN_ARENA))
		return CLevel_ValtanArena::Get_Active();
#endif
	return nullptr;
}
