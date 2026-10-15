#include "MapAuthoringHost.h"

// 현재 Level ID로 올바른 live host를 선택한다.
#include "GameInstance.h"
// 캐릭터 선택 Level의 활성 인스턴스를 host로 반환한다.
#include "Level_CharacterSelect.h"
// 베른 Level의 활성 인스턴스를 host로 반환한다.
#include "Level_Bern.h"
// 발탄 Level의 활성 인스턴스를 host로 반환한다.
#include "Level_ValtanArena.h"
// 쿠크 Level의 활성 인스턴스를 host로 반환한다.
#include "Level_KoukuSaydonArena.h"

// 현재 활성 Level을 조회할 뿐 생성하지 않는다. Level 전환 뒤에는 반환 주소를 다시 조회해야 한다.
IMapAuthoringHost* Client::Find_ActiveMapAuthoringHost()
{
#ifdef _DEBUG
	const uint32_t levelIndex = CGameInstance::Get().Get_CurrentLevelID();
	if (levelIndex == ETOUI(LEVEL::KOUKUSAYDON_ARENA))
		return CLevel_KoukuSaydonArena::Get_Active();
	if (levelIndex == ETOUI(LEVEL::CHARACTER_SELECT))
		return CLevel_CharacterSelect::Get_Active();
	if (levelIndex == ETOUI(LEVEL::BERN))
		return CLevel_Bern::Get_Active();
	if (levelIndex == ETOUI(LEVEL::VALTAN_ARENA))
		return CLevel_ValtanArena::Get_Active();
#endif
	return nullptr;
}
