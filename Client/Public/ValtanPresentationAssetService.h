#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CValtanPresentationAssetService final
{
public:
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex);
	static bool_t Is_Ready(uint32_t iLevelIndex);
};

NS_END
