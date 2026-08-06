#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>
#include <string_view>

NS_BEGIN(Client)

class CMonsterPresentationAssetService final
{
public:
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		std::string_view archetypeId);
	static std::wstring Get_ModelPrototypeTag(std::string_view archetypeId);
	static const wchar_t* Get_GameObjectPrototypeTag();
};

NS_END
