#include "EquipmentPresentationService.h"

#include "Character.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"

#include <array>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace
{
	using namespace Client;

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

	std::string Make_RuntimePartId(
		const EQUIPMENT_VISUAL_SET& set,
		const EQUIPMENT_PRESENTATION_PART& part)
	{
		return std::string(To_String(set.primarySlot)) + "." +
			set.visualSetId + "." + part.partId;
	}

	bool_t Try_MapPresentationSlot(
		const EQUIPMENT_SLOT_ID catalogSlot,
		EQUIPMENT_PRESENTATION_SLOT& outSlot)
	{
		switch (catalogSlot)
		{
		case EQUIPMENT_SLOT_ID::HEAD:
			outSlot = EQUIPMENT_PRESENTATION_SLOT::HEAD;
			return true;
		case EQUIPMENT_SLOT_ID::SHOULDER:
			outSlot = EQUIPMENT_PRESENTATION_SLOT::SHOULDER;
			return true;
		case EQUIPMENT_SLOT_ID::UPPER:
			outSlot = EQUIPMENT_PRESENTATION_SLOT::UPPER;
			return true;
		case EQUIPMENT_SLOT_ID::LOWER:
			outSlot = EQUIPMENT_PRESENTATION_SLOT::LOWER;
			return true;
		case EQUIPMENT_SLOT_ID::HANDS:
			outSlot = EQUIPMENT_PRESENTATION_SLOT::HANDS;
			return true;
		case EQUIPMENT_SLOT_ID::WEAPON:
			outSlot = EQUIPMENT_PRESENTATION_SLOT::WEAPON;
			return true;
		default:
			return false;
		}
	}
}

Client::CEquipmentPresentationService::CEquipmentPresentationService(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice(std::move(pDevice)), m_pContext(std::move(pContext))
{
}

bool_t Client::CEquipmentPresentationService::Apply_Preview(
	CCharacter& character,
	const CEquipmentPresentationCatalog& catalog,
	const std::array<std::string,
		ETOI(EQUIPMENT_SLOT_ID::END)>& selectedVisualSetIds,
	std::string& outError)
{
	outError.clear();
	if (nullptr == m_pDevice || nullptr == m_pContext ||
		nullptr == character.Get_Spec() ||
		nullptr == character.Get_BodyModel())
	{
		outError = "Equipment presentation service or target is not initialized.";
		return false;
	}
	const LostArk::Shared::CHARACTER_CLASS_ID targetClass =
		character.Get_Spec()->eCharacterClass;
	if (!LostArk::Shared::Is_Supported_Playable_Character_Class(targetClass))
	{
		outError = "Equipment preview target has an unsupported class.";
		return false;
	}

	std::array<bool_t, ETOI(EQUIPMENT_SLOT_ID::END)> occupiedSlots{};
	uint32_t occupiedSlotsMask = 0u;
	std::vector<const EQUIPMENT_VISUAL_SET*> selectedSets;
	selectedSets.reserve(selectedVisualSetIds.size());
	std::set<std::string> uniqueSetIds;
	for (size_t slotIndex = 0u;
		slotIndex < selectedVisualSetIds.size(); ++slotIndex)
	{
		const std::string& visualSetId = selectedVisualSetIds[slotIndex];
		if (visualSetId.empty())
			continue;
		const EQUIPMENT_VISUAL_SET* set = catalog.Find_Set(visualSetId);
		if (nullptr == set)
		{
			outError = "Selected equipment visual set is unknown: " + visualSetId;
			return false;
		}
		if (set->classId != targetClass)
		{
			outError = "Selected equipment visual set belongs to another class: " +
				visualSetId;
			return false;
		}
		if (set->primarySlot != static_cast<EQUIPMENT_SLOT_ID>(slotIndex))
		{
			outError = "Selected equipment visual set is assigned to the wrong slot: " +
				visualSetId;
			return false;
		}
		if (!uniqueSetIds.insert(visualSetId).second)
		{
			outError = "Selected equipment visual set is duplicated: " + visualSetId;
			return false;
		}
		for (const EQUIPMENT_SLOT_ID occupied : set->occupiedSlots)
		{
			const size_t occupiedIndex = static_cast<size_t>(occupied);
			EQUIPMENT_PRESENTATION_SLOT presentationSlot =
				EQUIPMENT_PRESENTATION_SLOT::END;
			if (occupiedIndex >= occupiedSlots.size() ||
				occupiedSlots[occupiedIndex] ||
				!Try_MapPresentationSlot(occupied, presentationSlot))
			{
				outError = "Selected equipment visual sets occupy the same slot: " +
					visualSetId;
				return false;
			}
			occupiedSlots[occupiedIndex] = true;
			occupiedSlotsMask |=
				EquipmentPresentationSlotMask(presentationSlot);
		}
		selectedSets.push_back(set);
	}

	for (const EQUIPMENT_VISUAL_SET* set : selectedSets)
	{
		for (const EQUIPMENT_PRESENTATION_PART& part : set->parts)
		{
			if (EQUIPMENT_ATTACHMENT_MODE::SOCKETED == part.attachmentMode &&
				!character.Get_BodyModel()->Has_Bone(part.socketBoneId.c_str()))
			{
				outError = "Equipment socket does not exist on the target rig: " +
					part.socketBoneId;
				return false;
			}
		}
	}

	const uint32_t prototypeLevelIndex = character.Get_PrototypeLevelIndex();
	if (prototypeLevelIndex >= ETOUI(LEVEL::END))
	{
		outError = "Equipment preview target has an invalid prototype level.";
		return false;
	}
	if (m_iPrototypeLevelIndex != prototypeLevelIndex)
	{
		m_iPrototypeLevelIndex = prototypeLevelIndex;
		m_AdmittedModelAssetIds.clear();
	}

	std::map<std::string, EQUIPMENT_ATTACHMENT_MODE> modelsToAdmit;
	for (const EQUIPMENT_VISUAL_SET* set : selectedSets)
	{
		for (const EQUIPMENT_PRESENTATION_PART& part : set->parts)
		{
			const auto [iter, inserted] = modelsToAdmit.emplace(
				part.modelAssetId, part.attachmentMode);
			if (!inserted && iter->second != part.attachmentMode)
			{
				outError = "One equipment model is declared with two attachment modes: " +
					part.modelAssetId;
				return false;
			}
		}
	}

	std::vector<std::pair<wstring_t, unique_ptr<CPrototype>>> stagedPrototypes;
	stagedPrototypes.reserve(modelsToAdmit.size());
	for (const auto& [assetId, attachmentMode] : modelsToAdmit)
	{
		if (m_AdmittedModelAssetIds.contains(assetId))
			continue;
		const std::filesystem::path path = CRuntimeAssetRoot::Resolve(assetId);
		std::error_code fileError;
		if (path.empty() ||
			!std::filesystem::is_regular_file(path, fileError) || fileError)
		{
			outError = "Equipment model is unavailable at preview time: " + assetId;
			return false;
		}

		const bool_t isSkinned =
			EQUIPMENT_ATTACHMENT_MODE::SKINNED == attachmentMode;
		const f32_t characterScale =
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER == targetClass ?
			0.01f : 0.0001f;
		const matrix_t preTransform = isSkinned ?
			XMMatrixScaling(characterScale, characterScale, characterScale) *
				XMMatrixRotationY(XMConvertToRadians(-90.f)) :
			XMMatrixIdentity();
		unique_ptr<CPrototype> model = CModel::Create(
			m_pDevice,
			m_pContext,
			isSkinned ? MODEL::ANIM : MODEL::NONANIM,
			path.string().c_str(),
			preTransform);
		if (nullptr == model)
		{
			outError = "Equipment model decode failed: " + assetId;
			return false;
		}
		stagedPrototypes.emplace_back(
			Make_ModelPrototypeTag(assetId), std::move(model));
	}

	if (!stagedPrototypes.empty() &&
		FAILED(CGameInstance::Get().Add_Prototypes(
			prototypeLevelIndex, std::move(stagedPrototypes))))
	{
		outError = "Equipment model prototype batch commit failed.";
		return false;
	}
	for (const auto& [assetId, attachmentMode] : modelsToAdmit)
	{
		(void)attachmentMode;
		m_AdmittedModelAssetIds.insert(assetId);
	}

	std::vector<CCharacter::EQUIPMENT_PREVIEW_PART_DESC> previewParts;
	for (const EQUIPMENT_VISUAL_SET* set : selectedSets)
	{
		previewParts.reserve(previewParts.size() + set->parts.size());
		for (const EQUIPMENT_PRESENTATION_PART& part : set->parts)
		{
			CCharacter::EQUIPMENT_PREVIEW_PART_DESC previewPart;
			previewPart.runtimePartId = Make_RuntimePartId(*set, part);
			previewPart.modelPrototypeTag =
				Make_ModelPrototypeTag(part.modelAssetId);
			previewPart.isSocketed =
				EQUIPMENT_ATTACHMENT_MODE::SOCKETED == part.attachmentMode;
			previewPart.socketBoneId = part.socketBoneId;
			previewPart.socketYawDegrees = part.socketYawDegrees;
			previewPart.requiredStance = part.requiredStance;
			previewPart.hiddenMeshMask = part.hiddenMeshMask;
			previewParts.push_back(std::move(previewPart));
		}
	}
	return character.Apply_EquipmentPreview(
		previewParts, occupiedSlotsMask, outError);
}

bool_t Client::CEquipmentPresentationService::Reset_Preview(
	CCharacter& character,
	std::string& outError)
{
	return character.Reset_EquipmentPreview(outError);
}

void Client::CEquipmentPresentationService::On_LevelChanged()
{
	m_iPrototypeLevelIndex = ETOUI(LEVEL::END);
	m_AdmittedModelAssetIds.clear();
}
