#include "CharacterOutfitApplier.h"

#include "Character.h"
#include "GameInstance.h"

#include <algorithm>

namespace
{
	/* Same encoding the equipment presentation service registers its model prototypes under. */
	wstring_t Make_ModelPrototypeTag(const std::string& assetId)
	{
		constexpr wchar_t HEX[] = L"0123456789ABCDEF";
		wstring_t result = L"Prototype_Component_Model_EquipmentPreview_";
		result.reserve(result.size() + assetId.size() * 2u);
		for (const unsigned char character : assetId)
		{
			result.push_back(HEX[(character >> 4u) & 0x0fu]);
			result.push_back(HEX[character & 0x0fu]);
		}
		return result;
	}

	void Log_Outfit(const std::string& text)
	{
		OutputDebugStringA(("[CharacterOutfitApplier] " + text + "\n").c_str());
	}
}

Client::CCharacterOutfitApplier::CCharacterOutfitApplier(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pService(std::make_unique<CEquipmentPresentationService>(
		std::move(pDevice), std::move(pContext)))
{
}

bool_t Client::CCharacterOutfitApplier::Ensure_Loaded()
{
	if (m_isLoaded)
		return true;
	if (m_isLoadFailed)
		return false;

	std::string error;
	if (!m_EquipmentCatalog.Load(error) || !m_CostumeDocument.Load() ||
		!m_HairstyleDocument.Load())
	{
		/* One failure is final for this run so a missing document is not re-read per character. */
		m_isLoadFailed = true;
		Log_Outfit("Equipment data: " +
			(error.empty() ? m_CostumeDocument.Get_Status() : error));
		return false;
	}
	m_isLoaded = true;
	return true;
}

void Client::CCharacterOutfitApplier::Refresh_AdmissionMemory(
	const uint32_t iPrototypeLevelIndex)
{
	if (m_iProbeLevelIndex != iPrototypeLevelIndex || m_strProbeModelAssetId.empty())
		return;
	if (nullptr == CGameInstance::Get().Clone_Prototype(
		iPrototypeLevelIndex, Make_ModelPrototypeTag(m_strProbeModelAssetId)))
	{
		m_pService->On_LevelChanged();
		m_strProbeModelAssetId.clear();
	}
}

bool_t Client::CCharacterOutfitApplier::Wear_Set(
	CCharacter& character, const std::string& strSetId,
	std::array<std::string, ETOI(EQUIPMENT_SLOT_ID::END)>& outfit)
{
	const EQUIPMENT_VISUAL_SET* const pSet = m_EquipmentCatalog.Find_Set(strSetId);
	if (nullptr == pSet || pSet->primarySlot >= EQUIPMENT_SLOT_ID::END)
	{
		Log_Outfit("set not in catalog: " + strSetId);
		return false;
	}

	/* Same rule as the screen: everything worn stays on, only a piece this one physically
	overlaps comes off. */
	std::array<std::string, ETOI(EQUIPMENT_SLOT_ID::END)> selected = outfit;
	selected[ETOI(pSet->primarySlot)].clear();
	for (const EQUIPMENT_SLOT_ID occupied : pSet->occupiedSlots)
	{
		for (std::string& worn : selected)
		{
			if (worn.empty())
				continue;
			const EQUIPMENT_VISUAL_SET* const pWorn = m_EquipmentCatalog.Find_Set(worn);
			if (nullptr == pWorn ||
				pWorn->occupiedSlots.end() != std::find(pWorn->occupiedSlots.begin(),
					pWorn->occupiedSlots.end(), occupied))
			{
				worn.clear();
			}
		}
	}
	selected[ETOI(pSet->primarySlot)] = strSetId;

	const uint32_t iLevel = character.Get_PrototypeLevelIndex();
	Refresh_AdmissionMemory(iLevel);

	std::string error;
	if (!m_pService->Apply_Preview(character, m_EquipmentCatalog, selected, error))
	{
		Log_Outfit(error);
		return false;
	}
	outfit = std::move(selected);
	if (!pSet->parts.empty())
	{
		m_iProbeLevelIndex = iLevel;
		m_strProbeModelAssetId = pSet->parts.front().modelAssetId;
	}
	return true;
}

bool_t Client::CCharacterOutfitApplier::Apply(
	const shared_ptr<CCharacter>& pCharacter,
	const int32_t iHair, const int32_t iCostume)
{
	if (nullptr == pCharacter)
		return false;
	const CHARACTER_SPEC* const pSpec = pCharacter->Get_Spec();
	if (nullptr == pSpec || nullptr == pSpec->pAssetName)
		return false;
	if (!Ensure_Loaded())
		return false;

	/* A fresh restore target starts bare; Apply_Preview takes the whole outfit each time. */
	std::array<std::string, ETOI(EQUIPMENT_SLOT_ID::END)> outfit{};
	bool_t isApplied = false;

	const std::vector<std::string>* const pCostumeIds =
		m_CostumeDocument.Find(pSpec->pAssetName);
	if (nullptr != pCostumeIds && iCostume >= 0 &&
		static_cast<size_t>(iCostume) < pCostumeIds->size())
	{
		isApplied |= Wear_Set(*pCharacter, (*pCostumeIds)[static_cast<size_t>(iCostume)], outfit);
	}

	const std::vector<std::string>* const pHairIds =
		m_HairstyleDocument.Find(pSpec->pAssetName);
	if (nullptr != pHairIds)
	{
		int32_t iWantedHair = iHair;
		if (iWantedHair < 0 || static_cast<size_t>(iWantedHair) >= pHairIds->size())
			iWantedHair = m_HairstyleDocument.Get_DefaultIndex(pSpec->pAssetName);
		if (iWantedHair >= 0 && static_cast<size_t>(iWantedHair) < pHairIds->size())
		{
			isApplied |= Wear_Set(*pCharacter, (*pHairIds)[static_cast<size_t>(iWantedHair)], outfit);
		}
	}
	return isApplied;
}
