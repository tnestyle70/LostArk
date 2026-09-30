#pragma once

#include "Client_Defines.h"
#include "EquipmentPresentationCatalog.h"

#include <array>
#include <span>
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

    bool_t Preload_VisualSets(uint32_t levelIndex, const CEquipmentPresentationCatalog& catalog,
        std::span<const std::string> visualSetIds, std::string& outError);
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
    bool_t Admit_Models(uint32_t prototypeLevelIndex, LostArk::Shared::CHARACTER_CLASS_ID targetClass,
        const std::vector<const EQUIPMENT_VISUAL_SET*>& selectedSets, std::string& outError);
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	uint32_t m_iPrototypeLevelIndex = ETOUI(LEVEL::END);
	std::unordered_set<std::string> m_AdmittedModelAssetIds;
};

NS_END
