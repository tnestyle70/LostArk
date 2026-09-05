#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <string_view>

NS_BEGIN(Client)

struct KOUKU_SAYDON_ACTION_PRESENTATION final
{
	std::string strActionId;
	std::string strOccurrenceId;
	std::string strClip;
	std::uint32_t iPlayMs = 0u;
	f32_t fPlayRate = 1.f;
};

/* Loads only the embedded-body Kouku Product presentation. It deliberately
does not share Valtan's weapon/armour prototype or joined presentation graph. */
class CKoukuSaydonPresentationAssetService final
{
public:
	static void Begin_LevelLoad(std::uint32_t iLevelIndex);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::uint32_t iLevelIndex,
		std::string_view archetypeId);
	static std::wstring Get_ModelPrototypeTag(std::string_view archetypeId);
	static const wchar_t* Get_GameObjectPrototypeTag();
	static bool_t Try_Resolve_Action(
		std::string_view actionId,
		KOUKU_SAYDON_ACTION_PRESENTATION& outPresentation);
	static const std::string& Get_Status();
};

NS_END
