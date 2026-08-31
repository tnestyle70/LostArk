#include "imgui.h"

#include "EquipmentAuthoringTool.h"

#include "AnimationTargetService.h"
#include "CharacterPreviewPanel.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iterator>
#include <unordered_set>
#include <utility>

namespace
{
	using namespace Client;
	using LostArk::Shared::CHARACTER_CLASS_ID;
	using LostArk::Shared::PLAYER_STANCE_ID;

	struct EQUIPMENT_CLASS_OPTION final
	{
		CHARACTER_CLASS_ID classId;
		const char_t* pClassId;
		const char_t* pPreviewAssetId;
		const char_t* pLabel;
	};

	constexpr std::array<EQUIPMENT_CLASS_OPTION, 6u> CLASS_OPTIONS = {{
		{ CHARACTER_CLASS_ID::LANCE_MASTER, "LANCE_MASTER", "LanceMaster", "Lance Master" },
		{ CHARACTER_CLASS_ID::GUNSLINGER, "GUNSLINGER", "GunSlinger", "Gunslinger" },
		{ CHARACTER_CLASS_ID::SLAYER, "SLAYER", "Slayer", "Slayer" },
		{ CHARACTER_CLASS_ID::ARTIST, "ARTIST", "Artist", "Artist" },
		{ CHARACTER_CLASS_ID::DIMENSIONMASTER, "DIMENSIONMASTER", "DimensionMaster", "Dimension Master" },
		{ CHARACTER_CLASS_ID::WARLORD, "WARLORD", "Warlord", "Warlord" },
	}};

	const char_t* Attachment_Mode_Label(
		const EQUIPMENT_ATTACHMENT_MODE mode)
	{
		switch (mode)
		{
		case EQUIPMENT_ATTACHMENT_MODE::SKINNED:
			return "SKINNED";
		case EQUIPMENT_ATTACHMENT_MODE::SOCKETED:
			return "SOCKETED";
		default:
			return "INVALID";
		}
	}

	const char_t* Stance_Label(const PLAYER_STANCE_ID stance)
	{
		switch (stance)
		{
		case PLAYER_STANCE_ID::NONE:
			return "NONE";
		case PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR:
			return "LANCE_MASTER_LONG_SPEAR";
		case PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR:
			return "LANCE_MASTER_SHORT_SPEAR";
		case PLAYER_STANCE_ID::WARLORD_NORMAL:
			return "WARLORD_NORMAL";
		case PLAYER_STANCE_ID::WARLORD_DEFENSE:
			return "WARLORD_DEFENSE";
		default:
			return "INVALID";
		}
	}

	bool_t Is_ValidFormatVersion(const DATA_JSON_VALUE* pValue)
	{
		return nullptr != pValue && pValue->Is_Number() &&
			!pValue->Was_FloatingPointToken() &&
			1.0 == pValue->Get_Number();
	}

	void Write_JsonString(std::ostream& output, const std::string& value)
	{
		output << '"' << CDataJson::Escape(value) << '"';
	}

	size_t Find_ClassIndexById(const std::string& classId)
	{
		for (size_t index = 0u; index < CLASS_OPTIONS.size(); ++index)
		{
			if (classId == CLASS_OPTIONS[index].pClassId)
				return index;
		}
		return CLASS_OPTIONS.size();
	}

	size_t Find_ClassIndexByAssetId(const std::string& assetId)
	{
		for (size_t index = 0u; index < CLASS_OPTIONS.size(); ++index)
		{
			if (assetId == CLASS_OPTIONS[index].pPreviewAssetId)
				return index;
		}
		return CLASS_OPTIONS.size();
	}

	bool_t Set_OccupiesSlot(
		const EQUIPMENT_VISUAL_SET& visualSet,
		const EQUIPMENT_SLOT_ID slot)
	{
		if (visualSet.primarySlot == slot)
			return true;
		return std::find(
			visualSet.occupiedSlots.begin(),
			visualSet.occupiedSlots.end(), slot) !=
			visualSet.occupiedSlots.end();
	}

	uint32_t Set_OccupiedMask(const EQUIPMENT_VISUAL_SET& visualSet)
	{
		uint32_t mask = 0u;
		const auto appendSlot = [&mask](const EQUIPMENT_SLOT_ID slot)
		{
			if (slot < EQUIPMENT_SLOT_ID::END)
				mask |= 1u << ETOUI(slot);
		};
		appendSlot(visualSet.primarySlot);
		for (const EQUIPMENT_SLOT_ID slot : visualSet.occupiedSlots)
			appendSlot(slot);
		return mask;
	}
}

Client::CEquipmentAuthoringTool::CEquipmentAuthoringTool(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	std::shared_ptr<CCharacterPreviewPanel> pPreviewPanel)
	: m_pDevice{ std::move(pDevice) }
	, m_pContext{ std::move(pContext) }
	, m_pPreviewPanel{ std::move(pPreviewPanel) }
	, m_PresentationService{ m_pDevice, m_pContext }
{
}

Client::CEquipmentAuthoringTool::~CEquipmentAuthoringTool()
{
	if (nullptr != m_pPreviewPanel)
	{
		m_pPreviewPanel->Set_SessionLock(
			CHARACTER_PREVIEW_LOCK_OWNER::EQUIPMENT_TOOL,
			false, {});
	}
}

void Client::CEquipmentAuthoringTool::Render(const bool_t isInputOwner)
{
	m_bPreviewInputEnabled = isInputOwner;
	if (!m_bLoadAttempted)
	{
		m_bLoadAttempted = true;
		if (Reload_Catalog())
			(void)Reload_Presets();
	}

	ImGui::SetNextWindowSize(ImVec2(1260.f, 760.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(
		"Equipment Authoring Tool###EquipmentAuthoringToolV1"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextDisabled(
		"Preview-only authoring. This tool never writes account, Server inventory, or scene-player state.");
	if (!isInputOwner)
	{
		ImGui::TextDisabled(
			"Read-only: select Equipment Authoring Tool as the explicit preview owner in F1 Developer Tools.");
	}
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::Separator();

	ImGui::BeginDisabled(!isInputOwner || !m_bCatalogLoaded);
	Render_ClassSelector();
	ImGui::EndDisabled();

	if (ImGui::BeginTable(
		"##EquipmentAuthoringColumns", 3,
		ImGuiTableFlags_BordersInnerV |
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("Slots", ImGuiTableColumnFlags_WidthFixed, 210.f);
		ImGui::TableSetupColumn("Admitted visual sets", ImGuiTableColumnFlags_WidthStretch, 0.42f);
		ImGui::TableSetupColumn("Set diagnostics", ImGuiTableColumnFlags_WidthStretch, 0.58f);
		ImGui::TableHeadersRow();

		ImGui::TableNextColumn();
		ImGui::BeginDisabled(!isInputOwner || !m_bCatalogLoaded);
		Render_SlotList();
		ImGui::EndDisabled();

		ImGui::TableNextColumn();
		ImGui::BeginDisabled(!isInputOwner || !m_bCatalogLoaded);
		Render_VisualSetList();
		ImGui::EndDisabled();

		ImGui::TableNextColumn();
		Render_SelectedSetDiagnostics();
		ImGui::EndTable();
	}

	ImGui::Separator();
	ImGui::BeginDisabled(!isInputOwner || !m_bCatalogLoaded);
	Render_Actions();
	ImGui::EndDisabled();
	ImGui::End();
}

void Client::CEquipmentAuthoringTool::On_LevelChanged()
{
	m_PresentationService.On_LevelChanged();
	m_iObservedPreviewGeneration = 0u;
	m_strStatus = m_bDirty ?
		"Level changed. The unsaved preview preset remains locked; select Save, Reload, or Reset before changing class." :
		"Level changed. Select a class to stage its equipment preview in the new level.";
}

void Client::CEquipmentAuthoringTool::Render_ClassSelector()
{
	if (nullptr == m_pPreviewPanel)
	{
		ImGui::TextDisabled("Shared Character Preview Panel is unavailable.");
		return;
	}

	m_pPreviewPanel->Refresh_Level();
	m_pPreviewPanel->Render_Selector(
		m_bDirty,
		"Save, reload, or reset the equipment draft before changing its class target.",
		false);
	Synchronize_ClassFromPreviewTarget();

	ImGui::SeparatorText("Playable class category");
	ImGui::BeginDisabled(m_bDirty);
	for (size_t index = 0u; index < CLASS_OPTIONS.size(); ++index)
	{
		if (0u != index)
			ImGui::SameLine();
		const bool_t selected = index == m_iSelectedClass;
		if (selected)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.45f, 0.65f, 1.f));
		if (ImGui::Button(CLASS_OPTIONS[index].pLabel))
			(void)Select_Class(index);
		if (selected)
			ImGui::PopStyleColor();
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Selected stable class ID: %s%s",
		CLASS_OPTIONS[m_iSelectedClass].pClassId,
		m_bDirty ? " (unsaved preset locks target changes)" : "");
}

void Client::CEquipmentAuthoringTool::Render_SlotList()
{
	const LOADOUT& loadout = m_Loadouts[m_iSelectedClass];
	for (size_t index = 0u; index < SLOT_COUNT; ++index)
	{
		const auto slot = static_cast<EQUIPMENT_SLOT_ID>(index);
		std::string label = To_String(slot);
		const EQUIPMENT_VISUAL_SET* pOccupyingSet =
			Find_SetOccupyingSlot(loadout, slot);
		if (nullptr != pOccupyingSet)
		{
			label += "\n  ";
			label += pOccupyingSet->visualSetId;
		}
		else
		{
			label += "\n  <default>";
		}
		if (ImGui::Selectable(
			(label + "##slot" + std::to_string(index)).c_str(),
			index == m_iSelectedSlot,
			ImGuiSelectableFlags_None,
			ImVec2(0.f, 42.f)))
		{
			m_iSelectedSlot = index;
			m_strSelectedVisualSetId.clear();
		}
	}
}

void Client::CEquipmentAuthoringTool::Render_VisualSetList()
{
	const CHARACTER_CLASS_ID classId = CLASS_OPTIONS[m_iSelectedClass].classId;
	const EQUIPMENT_SLOT_ID slot =
		static_cast<EQUIPMENT_SLOT_ID>(m_iSelectedSlot);
	const std::vector<const EQUIPMENT_VISUAL_SET*> matches =
		m_Catalog.Find_ByClassAndPrimarySlot(classId, slot);

	if (matches.empty())
	{
		ImGui::TextDisabled(
			"No admitted %s set exists for %s.",
			To_String(slot), CLASS_OPTIONS[m_iSelectedClass].pClassId);
		return;
	}

	for (const EQUIPMENT_VISUAL_SET* pSet : matches)
	{
		if (nullptr == pSet)
			continue;
		const bool_t selected =
			m_strSelectedVisualSetId == pSet->visualSetId;
		const std::string label =
			pSet->visualSetId + "\n  " + pSet->categoryId +
			" / " + pSet->catalogStatus;
		if (ImGui::Selectable(
			(label + "##visualSet").c_str(), selected,
			ImGuiSelectableFlags_None, ImVec2(0.f, 42.f)))
		{
			m_strSelectedVisualSetId = pSet->visualSetId;
		}
	}
}

void Client::CEquipmentAuthoringTool::Render_SelectedSetDiagnostics() const
{
	const EQUIPMENT_VISUAL_SET* pSet = Selected_Set();
	if (nullptr == pSet)
	{
		ImGui::TextDisabled("Select one admitted visual set to inspect its parts.");
		return;
	}

	ImGui::TextWrapped("Visual set: %s", pSet->visualSetId.c_str());
	ImGui::Text("Class: %s", CLASS_OPTIONS[m_iSelectedClass].pClassId);
	ImGui::Text("Category: %s", pSet->categoryId.c_str());
	ImGui::Text("Status: %s", pSet->catalogStatus.c_str());
	ImGui::Text("Primary slot: %s", To_String(pSet->primarySlot));
	ImGui::TextUnformatted("Occupied slots:");
	for (const EQUIPMENT_SLOT_ID slot : pSet->occupiedSlots)
	{
		ImGui::SameLine();
		ImGui::Text("%s", To_String(slot));
	}

	ImGui::SeparatorText("Parts");
	for (size_t index = 0u; index < pSet->parts.size(); ++index)
	{
		const EQUIPMENT_PRESENTATION_PART& part = pSet->parts[index];
		ImGui::PushID(static_cast<int32_t>(index));
		if (ImGui::TreeNodeEx(
			part.partId.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Role: %s", part.partRole.c_str());
			ImGui::Text("Attachment: %s", Attachment_Mode_Label(part.attachmentMode));
			ImGui::TextWrapped("Model: %s", part.modelAssetId.c_str());
			ImGui::Text("Socket: %s", part.socketBoneId.empty() ?
				"<none>" : part.socketBoneId.c_str());
			ImGui::Text("Socket yaw: %.3f", part.socketYawDegrees);
			ImGui::Text("Required stance: %s", Stance_Label(part.requiredStance));
			ImGui::Text("Hidden mesh mask: 0x%08X", part.hiddenMeshMask);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void Client::CEquipmentAuthoringTool::Render_Actions()
{
	const EQUIPMENT_VISUAL_SET* pSelectedSet = Selected_Set();
	ImGui::BeginDisabled(nullptr == pSelectedSet);
	if (ImGui::Button("Equip Selected"))
		(void)Equip_SelectedSet();
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Unequip Slot"))
		(void)Unequip_SelectedSlot();
	ImGui::SameLine();
	if (ImGui::Button("Unequip All"))
		(void)Unequip_All();
	ImGui::SameLine();
	if (ImGui::Button("Reset Character Default"))
		(void)Reset_CharacterDefault();

	if (ImGui::Button("Reload Catalog"))
		(void)Reload_Catalog();
	ImGui::SameLine();
	if (ImGui::Button("Save Presets"))
		(void)Save_Presets();
	ImGui::SameLine();
	if (ImGui::Button("Reload Presets"))
		(void)Reload_Presets();
}

bool_t Client::CEquipmentAuthoringTool::Select_Class(const size_t classIndex)
{
	if (!m_bPreviewInputEnabled || nullptr == m_pPreviewPanel ||
		classIndex >= CLASS_OPTIONS.size())
	{
		m_strStatus = "Class selection requires Equipment Tool preview ownership.";
		return false;
	}
	if (m_bDirty)
	{
		m_strStatus =
			"Class selection is locked by unsaved presets. Save, reload, or reset first.";
		return false;
	}
	if (!m_pPreviewPanel->Select_TargetAsset(
		CLASS_OPTIONS[classIndex].pPreviewAssetId))
	{
		m_strStatus = "Class preview selection failed: " +
			m_pPreviewPanel->Get_Status();
		return false;
	}

	m_iSelectedClass = classIndex;
	m_strSelectedVisualSetId.clear();
	m_iObservedPreviewGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	return Apply_CurrentLoadout();
}

bool_t Client::CEquipmentAuthoringTool::Apply_CurrentLoadout()
{
	size_t previewClassIndex = CLASS_OPTIONS.size();
	std::shared_ptr<CCharacter> character;
	if (!m_bPreviewInputEnabled ||
		!Resolve_ActivePlayablePreview(previewClassIndex, character))
	{
		m_strStatus =
			"Select one of the six playable preview classes before applying equipment.";
		return false;
	}
	if (previewClassIndex != m_iSelectedClass)
	{
		m_strStatus =
			"The selected class category does not match the active playable preview.";
		return false;
	}

	std::string error;
	if (!m_PresentationService.Apply_Preview(
		*character, m_Catalog, m_Loadouts[m_iSelectedClass], error))
	{
		m_strStatus = "Equipment preview rejected; previous visual preserved: " + error;
		return false;
	}
	m_strStatus = "Applied preview preset for " +
		std::string(CLASS_OPTIONS[m_iSelectedClass].pClassId) + ".";
	return true;
}

bool_t Client::CEquipmentAuthoringTool::Commit_CurrentLoadout(
	const LOADOUT& stagedLoadout,
	const std::string& successStatus)
{
	std::string validationError;
	if (!Validate_Loadout(
		m_iSelectedClass, stagedLoadout, m_Catalog, validationError))
	{
		m_strStatus = "Equipment draft rejected: " + validationError;
		return false;
	}
	size_t previewClassIndex = CLASS_OPTIONS.size();
	std::shared_ptr<CCharacter> character;
	if (!m_bPreviewInputEnabled ||
		!Resolve_ActivePlayablePreview(previewClassIndex, character))
	{
		m_strStatus =
			"Select one of the six playable preview classes before changing equipment.";
		return false;
	}
	if (previewClassIndex != m_iSelectedClass)
	{
		m_strStatus =
			"The selected class category does not match the active playable preview.";
		return false;
	}

	std::string error;
	if (!m_PresentationService.Apply_Preview(
		*character, m_Catalog, stagedLoadout, error))
	{
		m_strStatus = "Equipment change rejected; previous visual and preset preserved: " +
			error;
		return false;
	}

	const bool_t changed = stagedLoadout != m_Loadouts[m_iSelectedClass];
	m_Loadouts[m_iSelectedClass] = stagedLoadout;
	if (changed)
		Set_Dirty(true);
	m_strStatus = successStatus;
	return true;
}

bool_t Client::CEquipmentAuthoringTool::Equip_SelectedSet()
{
	const EQUIPMENT_VISUAL_SET* pCandidate = Selected_Set();
	if (nullptr == pCandidate)
	{
		m_strStatus = "Select an admitted visual set before equipping it.";
		return false;
	}

	LOADOUT staged = m_Loadouts[m_iSelectedClass];
	const uint32_t candidateMask = Set_OccupiedMask(*pCandidate);
	for (size_t index = 0u; index < SLOT_COUNT; ++index)
	{
		if (staged[index].empty())
			continue;
		const EQUIPMENT_VISUAL_SET* pEquipped =
			m_Catalog.Find_Set(staged[index]);
		if (nullptr != pEquipped &&
			0u != (candidateMask & Set_OccupiedMask(*pEquipped)))
		{
			staged[index].clear();
		}
	}
	staged[ETOI(pCandidate->primarySlot)] = pCandidate->visualSetId;
	return Commit_CurrentLoadout(
		staged, "Equipped preview set " + pCandidate->visualSetId + ".");
}

bool_t Client::CEquipmentAuthoringTool::Unequip_SelectedSlot()
{
	LOADOUT staged = m_Loadouts[m_iSelectedClass];
	size_t primarySlotIndex = SLOT_COUNT;
	const EQUIPMENT_VISUAL_SET* pEquipped = Find_SetOccupyingSlot(
		staged, static_cast<EQUIPMENT_SLOT_ID>(m_iSelectedSlot),
		&primarySlotIndex);
	if (nullptr == pEquipped || primarySlotIndex >= SLOT_COUNT)
	{
		m_strStatus = "The selected slot already uses the character default.";
		return false;
	}
	const std::string removedSetId = pEquipped->visualSetId;
	staged[primarySlotIndex].clear();
	return Commit_CurrentLoadout(
		staged, "Unequipped preview set " + removedSetId + ".");
}

bool_t Client::CEquipmentAuthoringTool::Unequip_All()
{
	LOADOUT staged{};
	return Commit_CurrentLoadout(
		staged, "Unequipped every preview slot for the selected class.");
}

bool_t Client::CEquipmentAuthoringTool::Reset_CharacterDefault()
{
	size_t previewClassIndex = CLASS_OPTIONS.size();
	std::shared_ptr<CCharacter> character;
	if (!m_bPreviewInputEnabled ||
		!Resolve_ActivePlayablePreview(previewClassIndex, character))
	{
		m_strStatus =
			"Select one of the six playable preview classes before resetting its Character.";
		return false;
	}
	if (previewClassIndex != m_iSelectedClass)
	{
		m_strStatus =
			"The selected class category does not match the active playable preview.";
		return false;
	}

	std::string error;
	if (!m_PresentationService.Reset_Preview(*character, error))
	{
		m_strStatus = "Character default reset failed; previous visual preserved: " + error;
		return false;
	}
	const LOADOUT emptyLoadout{};
	const bool_t changed = emptyLoadout != m_Loadouts[m_iSelectedClass];
	m_Loadouts[m_iSelectedClass] = emptyLoadout;
	if (changed)
		Set_Dirty(true);
	m_strSelectedVisualSetId.clear();
	m_strStatus = "Restored the selected class Character default.";
	return true;
}

bool_t Client::CEquipmentAuthoringTool::Reload_Catalog()
{
	CEquipmentPresentationCatalog stagedCatalog;
	std::string error;
	if (!stagedCatalog.Load(error))
	{
		m_strStatus = "Equipment catalog reload failed; previous catalog preserved: " + error;
		return false;
	}
	if (!Validate_Loadouts(m_Loadouts, stagedCatalog, error))
	{
		m_strStatus =
			"Equipment catalog reload rejected by current preset references; previous catalog preserved: " +
			error;
		return false;
	}

	size_t previewClassIndex = CLASS_OPTIONS.size();
	std::shared_ptr<CCharacter> previewCharacter;
	const bool_t hasActivePlayablePreview = Resolve_ActivePlayablePreview(
		previewClassIndex, previewCharacter);
	const bool_t shouldRefreshActivePreview =
		m_bCatalogLoaded && hasActivePlayablePreview;
	if (shouldRefreshActivePreview)
	{
		if (!m_bPreviewInputEnabled)
		{
			m_strStatus =
				"Equipment catalog reload is read-only while another tool owns the active playable preview; catalog and visual were preserved.";
			return false;
		}
		if (!m_PresentationService.Apply_Preview(
			*previewCharacter, stagedCatalog,
			m_Loadouts[previewClassIndex], error))
		{
			m_strStatus =
				"Equipment catalog reload failed while staging the active preview; previous catalog, diagnostics, visual, and dirty draft were preserved: " +
				error;
			return false;
		}
	}

	m_Catalog = std::move(stagedCatalog);
	m_bCatalogLoaded = true;
	if (shouldRefreshActivePreview)
		m_iSelectedClass = previewClassIndex;
	m_strSelectedVisualSetId.clear();
	m_strStatus = shouldRefreshActivePreview ?
		"Equipment presentation catalog and active preview were committed from one staged catalog generation." :
		"Equipment presentation catalog committed; no active equipment preview required a visual refresh.";
	return true;
}

bool_t Client::CEquipmentAuthoringTool::Reload_Presets()
{
	if (!m_bCatalogLoaded)
	{
		m_strStatus = "Load the equipment presentation catalog before presets.";
		return false;
	}

	CLASS_LOADOUTS staged{};
	std::string error;
	if (!Load_PresetsFromPath(Preset_Path(), staged, error))
	{
		m_strStatus = "Equipment preset reload failed; current draft preserved: " + error;
		return false;
	}

	size_t previewClassIndex = CLASS_OPTIONS.size();
	std::shared_ptr<CCharacter> previewCharacter;
	const bool_t hasActivePlayablePreview = Resolve_ActivePlayablePreview(
		previewClassIndex, previewCharacter);
	if (hasActivePlayablePreview)
	{
		if (!m_bPreviewInputEnabled)
		{
			m_strStatus =
				"Equipment preset reload is read-only while another tool owns the active playable preview; draft, visual, and dirty state were preserved.";
			return false;
		}
		if (!m_PresentationService.Apply_Preview(
			*previewCharacter, m_Catalog,
			staged[previewClassIndex], error))
		{
			m_strStatus =
				"Equipment preset reload failed while staging the active preview; previous UI loadout, visual, and dirty state were preserved: " +
				error;
			return false;
		}
	}

	m_Loadouts = std::move(staged);
	m_bPresetsLoaded = true;
	if (hasActivePlayablePreview)
		m_iSelectedClass = previewClassIndex;
	Set_Dirty(false);
	m_strSelectedVisualSetId.clear();
	m_strStatus = hasActivePlayablePreview ?
		"Preview-only equipment presets and active preview were committed together." :
		"Preview-only equipment presets committed; they will apply when a playable class preview is selected.";
	return true;
}

bool_t Client::CEquipmentAuthoringTool::Save_Presets()
{
	if (!m_bCatalogLoaded)
	{
		m_strStatus = "Load the equipment presentation catalog before saving presets.";
		return false;
	}
	std::string error;
	if (!Validate_Loadouts(m_Loadouts, m_Catalog, error))
	{
		m_strStatus = "Equipment preset save rejected: " + error;
		return false;
	}

	const std::filesystem::path destination = Preset_Path();
	std::error_code directoryError;
	std::filesystem::create_directories(destination.parent_path(), directoryError);
	if (directoryError)
	{
		m_strStatus = "Equipment preset save failed: could not create Data/Actors.";
		return false;
	}
	const std::filesystem::path temporary =
		Make_UniqueTemporaryPath(destination);
	if (temporary.empty())
	{
		m_strStatus = "Equipment preset save failed: no unique temporary path was available.";
		return false;
	}

	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		m_strStatus = "Equipment preset save failed: could not create the temporary document.";
		return false;
	}
	output << "{\n"
		<< "  \"schema\": \"lostark.equipment-loadout-presets\",\n"
		<< "  \"formatVersion\": 1,\n"
		<< "  \"authoringOnly\": true,\n"
		<< "  \"slots\": [";
	for (size_t slotIndex = 0u; slotIndex < SLOT_COUNT; ++slotIndex)
	{
		if (0u != slotIndex)
			output << ", ";
		Write_JsonString(output, To_String(
			static_cast<EQUIPMENT_SLOT_ID>(slotIndex)));
	}
	output << "],\n  \"classPresets\": [\n";
	for (size_t classIndex = 0u; classIndex < CLASS_OPTIONS.size(); ++classIndex)
	{
		output << "    {\n      \"classId\": ";
		Write_JsonString(output, CLASS_OPTIONS[classIndex].pClassId);
		output << ",\n      \"slotSelections\": {\n";
		for (size_t slotIndex = 0u; slotIndex < SLOT_COUNT; ++slotIndex)
		{
			output << "        ";
			Write_JsonString(output, To_String(
				static_cast<EQUIPMENT_SLOT_ID>(slotIndex)));
			output << ": ";
			const std::string& visualSetId =
				m_Loadouts[classIndex][slotIndex];
			if (visualSetId.empty())
				output << "null";
			else
				Write_JsonString(output, visualSetId);
			output << (slotIndex + 1u == SLOT_COUNT ? "\n" : ",\n");
		}
		output << "      }\n    }";
		output << (classIndex + 1u == CLASS_OPTIONS.size() ? "\n" : ",\n");
	}
	output << "  ]\n}\n";
	output.flush();
	const bool_t writeSucceeded = output.good();
	output.close();
	if (!writeSucceeded)
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		m_strStatus = "Equipment preset save failed while flushing the temporary document.";
		return false;
	}

	CLASS_LOADOUTS reparsed{};
	if (!Load_PresetsFromPath(temporary, reparsed, error) ||
		reparsed != m_Loadouts)
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		m_strStatus = "Equipment preset save rejected by post-write validation: " +
			(error.empty() ? "round-trip mismatch" : error);
		return false;
	}

	if (!MoveFileExW(
		temporary.c_str(), destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		m_strStatus = "Equipment preset atomic promotion failed.";
		return false;
	}

	m_bPresetsLoaded = true;
	Set_Dirty(false);
	m_strStatus = "Preview-only equipment presets saved atomically to Data/Actors.";
	return true;
}

bool_t Client::CEquipmentAuthoringTool::Load_PresetsFromPath(
	const std::filesystem::path& path,
	CLASS_LOADOUTS& outLoadouts,
	std::string& outError) const
{
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outError = "preset JSON is missing: " + path.string();
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	DATA_JSON_VALUE root;
	if (!CDataJson::Parse(text, root, outError) || !root.Is_Object())
	{
		outError = "preset JSON parse failed: " + outError;
		return false;
	}

	const DATA_JSON_VALUE* pSchema = root.Find("schema");
	const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
	const DATA_JSON_VALUE* pAuthoringOnly = root.Find("authoringOnly");
	const DATA_JSON_VALUE* pSlots = root.Find("slots");
	const DATA_JSON_VALUE* pClassPresets = root.Find("classPresets");
	if (nullptr == pSchema || !pSchema->Is_String() ||
		"lostark.equipment-loadout-presets" != pSchema->Get_String() ||
		!Is_ValidFormatVersion(pVersion) ||
		nullptr == pAuthoringOnly || !pAuthoringOnly->Is_Boolean() ||
		!pAuthoringOnly->Get_Boolean() ||
		nullptr == pSlots || !pSlots->Is_Array() ||
		SLOT_COUNT != pSlots->Get_Array().size() ||
		nullptr == pClassPresets || !pClassPresets->Is_Array() ||
		CLASS_OPTIONS.size() != pClassPresets->Get_Array().size())
	{
		outError = "preset JSON contract mismatch";
		return false;
	}

	for (size_t slotIndex = 0u; slotIndex < SLOT_COUNT; ++slotIndex)
	{
		const DATA_JSON_VALUE& slotValue = pSlots->Get_Array()[slotIndex];
		if (!slotValue.Is_String() || slotValue.Get_String() != To_String(
			static_cast<EQUIPMENT_SLOT_ID>(slotIndex)))
		{
			outError = "preset slots must contain the six stable slot IDs in canonical order";
			return false;
		}
	}

	CLASS_LOADOUTS staged{};
	std::unordered_set<std::string> seenClasses;
	for (const DATA_JSON_VALUE& presetValue : pClassPresets->Get_Array())
	{
		if (!presetValue.Is_Object())
		{
			outError = "class preset entry is not an object";
			return false;
		}
		const DATA_JSON_VALUE* pClassId = presetValue.Find("classId");
		const DATA_JSON_VALUE* pSelections = presetValue.Find("slotSelections");
		if (nullptr == pClassId || !pClassId->Is_String() ||
			nullptr == pSelections || !pSelections->Is_Object() ||
			SLOT_COUNT != pSelections->Get_Object().size() ||
			!seenClasses.insert(pClassId->Get_String()).second)
		{
			outError = "class preset identity or slotSelections contract is invalid";
			return false;
		}
		const size_t classIndex = Find_ClassIndexById(pClassId->Get_String());
		if (classIndex >= CLASS_OPTIONS.size())
		{
			outError = "unknown equipment preset class ID: " + pClassId->Get_String();
			return false;
		}
		for (size_t slotIndex = 0u; slotIndex < SLOT_COUNT; ++slotIndex)
		{
			const char_t* pSlotId = To_String(
				static_cast<EQUIPMENT_SLOT_ID>(slotIndex));
			const DATA_JSON_VALUE* pSelection = pSelections->Find(pSlotId);
			if (nullptr == pSelection ||
				(!pSelection->Is_Null() && !pSelection->Is_String()))
			{
				outError = "slot selection must be a visualSetId string or null: " +
					std::string(pSlotId);
				return false;
			}
			if (pSelection->Is_String())
				staged[classIndex][slotIndex] = pSelection->Get_String();
		}
	}

	if (!Validate_Loadouts(staged, m_Catalog, outError))
		return false;
	outLoadouts = std::move(staged);
	outError.clear();
	return true;
}

bool_t Client::CEquipmentAuthoringTool::Validate_Loadouts(
	const CLASS_LOADOUTS& loadouts,
	const CEquipmentPresentationCatalog& catalog,
	std::string& outError) const
{
	for (size_t classIndex = 0u; classIndex < CLASS_OPTIONS.size(); ++classIndex)
	{
		if (!Validate_Loadout(classIndex, loadouts[classIndex], catalog, outError))
			return false;
	}
	outError.clear();
	return true;
}

bool_t Client::CEquipmentAuthoringTool::Validate_Loadout(
	const size_t classIndex,
	const LOADOUT& loadout,
	const CEquipmentPresentationCatalog& catalog,
	std::string& outError) const
{
	if (classIndex >= CLASS_OPTIONS.size())
	{
		outError = "loadout class index is out of range";
		return false;
	}
	uint32_t occupiedMask = 0u;
	std::unordered_set<std::string> selectedSetIds;
	for (size_t slotIndex = 0u; slotIndex < SLOT_COUNT; ++slotIndex)
	{
		const std::string& visualSetId = loadout[slotIndex];
		if (visualSetId.empty())
			continue;
		const EQUIPMENT_VISUAL_SET* pSet = catalog.Find_Set(visualSetId);
		if (nullptr == pSet ||
			pSet->classId != CLASS_OPTIONS[classIndex].classId ||
			ETOI(pSet->primarySlot) != slotIndex ||
			!selectedSetIds.insert(visualSetId).second)
		{
			outError = "preset references an unknown, duplicate, wrong-class, or wrong-slot visual set: " +
				visualSetId;
			return false;
		}
		const uint32_t setMask = Set_OccupiedMask(*pSet);
		if (0u == setMask || 0u != (occupiedMask & setMask))
		{
			outError = "preset contains overlapping occupied slots at visual set: " +
				visualSetId;
			return false;
		}
		occupiedMask |= setMask;
	}
	outError.clear();
	return true;
}

std::filesystem::path Client::CEquipmentAuthoringTool::Preset_Path() const
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Actors") / L"EquipmentLoadoutPresets.json");
}

std::filesystem::path Client::CEquipmentAuthoringTool::Make_UniqueTemporaryPath(
	const std::filesystem::path& destination) const
{
	const uint64_t tick = static_cast<uint64_t>(
		std::chrono::steady_clock::now().time_since_epoch().count());
	const uint64_t processId = static_cast<uint64_t>(GetCurrentProcessId());
	for (uint32_t attempt = 0u; attempt < 64u; ++attempt)
	{
		std::filesystem::path candidate = destination;
		candidate += L".tmp." + std::to_wstring(processId) + L"." +
			std::to_wstring(tick) + L"." + std::to_wstring(attempt);
		std::error_code existsError;
		if (!std::filesystem::exists(candidate, existsError) && !existsError)
			return candidate;
	}
	return {};
}

void Client::CEquipmentAuthoringTool::Synchronize_ClassFromPreviewTarget()
{
	const uint64_t generation =
		CAnimationTargetService::Resolve_TargetGeneration();
	if (generation == m_iObservedPreviewGeneration)
		return;
	m_iObservedPreviewGeneration = generation;
	const size_t classIndex = Find_ClassIndexByAssetId(
		CAnimationTargetService::Resolve_AssetName());
	if (classIndex >= CLASS_OPTIONS.size())
		return;
	m_iSelectedClass = classIndex;
	m_strSelectedVisualSetId.clear();
	if (!m_bDirty && m_bPreviewInputEnabled)
		(void)Apply_CurrentLoadout();
}

bool_t Client::CEquipmentAuthoringTool::Resolve_ActivePlayablePreview(
	size_t& outClassIndex,
	std::shared_ptr<CCharacter>& outCharacter) const
{
	outClassIndex = CLASS_OPTIONS.size();
	outCharacter.reset();
	if (nullptr == m_pPreviewPanel || !m_pPreviewPanel->Is_PreviewActive())
		return false;

	const size_t classIndex = Find_ClassIndexByAssetId(
		CAnimationTargetService::Resolve_AssetName());
	if (classIndex >= CLASS_OPTIONS.size())
		return false;
	const std::shared_ptr<CCharacter> character =
		CAnimationTargetService::Resolve_Character();
	if (nullptr == character)
		return false;

	outClassIndex = classIndex;
	outCharacter = character;
	return true;
}

void Client::CEquipmentAuthoringTool::Set_Dirty(const bool_t isDirty)
{
	m_bDirty = isDirty;
	if (nullptr != m_pPreviewPanel)
	{
		m_pPreviewPanel->Set_SessionLock(
			CHARACTER_PREVIEW_LOCK_OWNER::EQUIPMENT_TOOL,
			isDirty,
			isDirty ?
				"Equipment preview presets contain unsaved slot selections." :
				std::string{});
	}
}

const Client::EQUIPMENT_VISUAL_SET*
Client::CEquipmentAuthoringTool::Selected_Set() const
{
	if (m_strSelectedVisualSetId.empty())
		return nullptr;
	const EQUIPMENT_VISUAL_SET* pSet =
		m_Catalog.Find_Set(m_strSelectedVisualSetId);
	if (nullptr == pSet ||
		pSet->classId != CLASS_OPTIONS[m_iSelectedClass].classId ||
		ETOI(pSet->primarySlot) != m_iSelectedSlot)
	{
		return nullptr;
	}
	return pSet;
}

const Client::EQUIPMENT_VISUAL_SET*
Client::CEquipmentAuthoringTool::Find_SetOccupyingSlot(
	const LOADOUT& loadout,
	const EQUIPMENT_SLOT_ID slot,
	size_t* pPrimarySlotIndex) const
{
	for (size_t index = 0u; index < SLOT_COUNT; ++index)
	{
		if (loadout[index].empty())
			continue;
		const EQUIPMENT_VISUAL_SET* pSet =
			m_Catalog.Find_Set(loadout[index]);
		if (nullptr != pSet && Set_OccupiesSlot(*pSet, slot))
		{
			if (nullptr != pPrimarySlotIndex)
				*pPrimarySlotIndex = index;
			return pSet;
		}
	}
	return nullptr;
}
