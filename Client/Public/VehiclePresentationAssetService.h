#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>

NS_BEGIN(Client)

class CVehiclePresentationAssetService final
{
public:
	static void Begin_LevelLoad(uint32_t iLevelIndex);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iLevelIndex,
		std::uint32_t vehicleId);
	static bool_t Is_Ready(uint32_t iLevelIndex, std::uint32_t vehicleId);
	static wstring_t Get_ModelPrototypeTag(std::uint32_t vehicleId);
	static const std::string& Get_Status();
};

NS_END
