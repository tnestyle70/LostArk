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
	if (nullptr == pSet || pSet->primarySlot >= EQUIPMENT_SLOT_ID::END ||
		nullptr == character.Get_Spec() || pSet->classId != character.Get_Spec()->eCharacterClass)
	{
		Log_Outfit("set is unavailable for this character: " + strSetId);
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

	/* Compose only; Apply commits costume and hair together after both validate. */
	outfit = std::move(selected);
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

	/* Compose the complete saved outfit before touching the character. A missing model
	or failed hair clone must not leave only the costume from a half-finished restore. */
	std::array<std::string, ETOI(EQUIPMENT_SLOT_ID::END)> outfit{};
	const std::vector<std::string>* const pCostumeIds =
		m_CostumeDocument.Find(pSpec->pAssetName);
	if (iCostume >= 0)
	{
		if (nullptr == pCostumeIds || static_cast<size_t>(iCostume) >= pCostumeIds->size() ||
			!Wear_Set(*pCharacter, (*pCostumeIds)[static_cast<size_t>(iCostume)], outfit))
			return false;
	}

	const std::vector<std::string>* const pHairIds =
		m_HairstyleDocument.Find(pSpec->pAssetName);
	if (nullptr != pHairIds)
	{
		const int32_t iDefaultHair = m_HairstyleDocument.Get_DefaultIndex(pSpec->pAssetName);
		if (iDefaultHair == -1 && !pSpec->isBodyHairFallback)
		{
			Log_Outfit("Body hair default is unsupported for this character.");
			return false;
		}
		const auto acceptsHair = [&](const int32_t index) {
			if (index == -1) return iDefaultHair == -1 && pSpec->isBodyHairFallback;
			if (index < 0 || static_cast<size_t>(index) >= pHairIds->size()) return false;
			const auto* set = m_EquipmentCatalog.Find_Set((*pHairIds)[static_cast<size_t>(index)]);
			return nullptr != set && set->classId == pSpec->eCharacterClass &&
				set->primarySlot == EQUIPMENT_SLOT_ID::HEAD && !set->parts.empty();
		};
		int32_t iWantedHair = acceptsHair(iHair) ? iHair : iDefaultHair;
		if (!acceptsHair(iWantedHair))
		{
			iWantedHair = -1;
			for (size_t index = 0u; index < pHairIds->size(); ++index)
			{
				if (acceptsHair(static_cast<int32_t>(index)))
				{
					iWantedHair = static_cast<int32_t>(index);
					break;
				}
			}
		}
		if (!acceptsHair(iWantedHair))
			return false;
		if (iWantedHair >= 0 &&
			!Wear_Set(*pCharacter, (*pHairIds)[static_cast<size_t>(iWantedHair)], outfit))
			return false;
	}

	const uint32_t iLevel = pCharacter->Get_PrototypeLevelIndex();
	Refresh_AdmissionMemory(iLevel);
	std::string error;
	if (!m_pService->Apply_Preview(*pCharacter, m_EquipmentCatalog, outfit, error))
	{
		Log_Outfit(error);
		return false;
	}
	for (const std::string& setId : outfit)
	{
		const auto* set = setId.empty() ? nullptr : m_EquipmentCatalog.Find_Set(setId);
		if (nullptr == set || set->parts.empty()) continue;
		m_iProbeLevelIndex = iLevel;
		m_strProbeModelAssetId = set->parts.front().modelAssetId;
		break;
	}
	return true;
}
