#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "EquipmentPresentationCatalog.h"
#include "EquipmentPresentationService.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

NS_BEGIN(Client)

class CCharacterPreviewPanel;
class CCharacter;

class CEquipmentAuthoringTool final
{
private:
	static constexpr size_t CLASS_COUNT = 6u;
	static constexpr size_t SLOT_COUNT = ETOI(EQUIPMENT_SLOT_ID::END);
	using LOADOUT = std::array<std::string, SLOT_COUNT>;
	using CLASS_LOADOUTS = std::array<LOADOUT, CLASS_COUNT>;

public:
	CEquipmentAuthoringTool(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::shared_ptr<CCharacterPreviewPanel> pPreviewPanel);
	~CEquipmentAuthoringTool();

	void Render(bool_t isInputOwner);
	void On_LevelChanged();

private:
	void Render_ClassSelector();
	void Render_SlotList();
	void Render_VisualSetList();
	void Render_SelectedSetDiagnostics() const;
	void Render_Actions();

	bool_t Select_Class(size_t classIndex);
	bool_t Apply_CurrentLoadout();
	bool_t Commit_CurrentLoadout(
		const LOADOUT& stagedLoadout,
		const std::string& successStatus);
	bool_t Equip_SelectedSet();
	bool_t Unequip_SelectedSlot();
	bool_t Unequip_All();
	bool_t Reset_CharacterDefault();

	bool_t Reload_Catalog();
	bool_t Reload_Presets();
	bool_t Save_Presets();
	bool_t Load_PresetsFromPath(
		const std::filesystem::path& path,
		CLASS_LOADOUTS& outLoadouts,
		std::string& outError) const;
	bool_t Validate_Loadouts(
		const CLASS_LOADOUTS& loadouts,
		const CEquipmentPresentationCatalog& catalog,
		std::string& outError) const;
	bool_t Validate_Loadout(
		size_t classIndex,
		const LOADOUT& loadout,
		const CEquipmentPresentationCatalog& catalog,
		std::string& outError) const;
	std::filesystem::path Preset_Path() const;
	std::filesystem::path Make_UniqueTemporaryPath(
		const std::filesystem::path& destination) const;

	void Synchronize_ClassFromPreviewTarget();
	bool_t Resolve_ActivePlayablePreview(
		size_t& outClassIndex,
		std::shared_ptr<CCharacter>& outCharacter) const;
	void Set_Dirty(bool_t isDirty);
	const EQUIPMENT_VISUAL_SET* Selected_Set() const;
	const EQUIPMENT_VISUAL_SET* Find_SetOccupyingSlot(
		const LOADOUT& loadout,
		EQUIPMENT_SLOT_ID slot,
		size_t* pPrimarySlotIndex = nullptr) const;

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	std::shared_ptr<CCharacterPreviewPanel> m_pPreviewPanel;
	CEquipmentPresentationCatalog m_Catalog;
	CEquipmentPresentationService m_PresentationService;
	CLASS_LOADOUTS m_Loadouts{};

	size_t m_iSelectedClass = 0u;
	size_t m_iSelectedSlot = 0u;
	std::string m_strSelectedVisualSetId;
	std::string m_strStatus =
		"Equipment catalog and preview-only presets have not been loaded.";
	uint64_t m_iObservedPreviewGeneration = 0u;
	bool_t m_bLoadAttempted = false;
	bool_t m_bCatalogLoaded = false;
	bool_t m_bPresetsLoaded = false;
	bool_t m_bDirty = false;
	bool_t m_bPreviewInputEnabled = false;
};

NS_END
