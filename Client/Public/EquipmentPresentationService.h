#pragma once

#include "Client_Defines.h"
#include "EquipmentPresentationCatalog.h"

#include <array>
#include <string>
#include <unordered_set>

NS_BEGIN(Client)

class CCharacter;

class CEquipmentPresentationService final
{
public:
	CEquipmentPresentationService(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

	bool_t Apply_Preview(
		CCharacter& character,
		const CEquipmentPresentationCatalog& catalog,
		const std::array<std::string,
			ETOI(EQUIPMENT_SLOT_ID::END)>& selectedVisualSetIds,
		std::string& outError);
	bool_t Reset_Preview(
		CCharacter& character,
		std::string& outError);
	void On_LevelChanged();

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	uint32_t m_iPrototypeLevelIndex = ETOUI(LEVEL::END);
	std::unordered_set<std::string> m_AdmittedModelAssetIds;
};

NS_END
