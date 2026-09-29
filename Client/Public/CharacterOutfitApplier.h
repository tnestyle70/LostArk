#pragma once

#include "Client_Defines.h"
#include "CustomizingCostumeDocument.h"
#include "EquipmentPresentationCatalog.h"
#include "EquipmentPresentationService.h"

#include <array>
#include <memory>
#include <string>

NS_BEGIN(Client)

class CCharacter;

/* Puts the hair and the try-on costume of a saved look on a character with no customizing screen
open. It repeats the wearing rules of CLevel_CharacterSelect::Wear_CustomizingSet on purpose (the
live screen keeps its own copy untouched) and owns the same three inputs: the equipment catalog,
the costume document and the hairstyle document. Everything loads on first use and every failure
is soft: the caller gets false and the character keeps what it is wearing. Main thread only. */
class CCharacterOutfitApplier final
{
public:
	CCharacterOutfitApplier(
		ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

	/* iCostume < 0 wears no try-on set; iHair outside the class' hairstyle list wears the class
	default hair. Costume goes on before hair, the order the screen consumes a loaded slot in.
	True when at least one set was put on. */
	bool_t Apply(const shared_ptr<CCharacter>& pCharacter,
		int32_t iHair, int32_t iCostume);

private:
	bool_t Ensure_Loaded();
	/* Adds strSetId to the local outfit (dropping what it overlaps) and previews the outfit. */
	bool_t Wear_Set(CCharacter& character, const std::string& strSetId,
		std::array<std::string, ETOI(EQUIPMENT_SLOT_ID::END)>& outfit);
	/* The service remembers which models it admitted per prototype level, but a level change
	clears the engine's prototypes behind its back; probe one admitted model and reset the
	service's memory when it is gone. */
	void Refresh_AdmissionMemory(uint32_t iPrototypeLevelIndex);

private:
	CCustomizingCostumeDocument m_CostumeDocument{
		"lostark.customizing-costumes",
		"UI/Customizing/CustomizingCostumes.json", "costume" };
	CCustomizingCostumeDocument m_HairstyleDocument{
		"lostark.customizing-hairstyles",
		"UI/Customizing/CustomizingHairstyles.json", "hairstyle" };
	CEquipmentPresentationCatalog m_EquipmentCatalog;
	unique_ptr<CEquipmentPresentationService> m_pService;
	bool_t m_isLoaded = false;
	bool_t m_isLoadFailed = false;
	uint32_t m_iProbeLevelIndex = ETOUI(LEVEL::END);
	std::string m_strProbeModelAssetId;
};

NS_END
