#include "imgui.h"
#include "Animation_Tool_Internal.h"
#include "Character.h"
#include "Model.h"
#include "SoundCueCatalog.h"
#include <charconv>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <span>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>




Client::ANIMATION_SKILL_BINDING*
Client::CAnimation_Tool::Find_SkillBinding(
	const LostArk::Shared::SKILL_ID skillId)
{
	for (ANIMATION_SKILL_BINDING& binding :
		m_SkillBindingDocument.Bindings)
	{
		if (binding.iSkillId == skillId)
			return &binding;
	}
	return nullptr;
}

bool_t Client::CAnimation_Tool::Load_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	ANIMATION_SKILL_BINDING_DOCUMENT staged;
	std::string status;
	if (!CAnimationSkillBindingDocument::Load(
		m_AssetName,
		characterClass,
		CPlayerSkillCatalog::Get_Skills(),
		Collect_ClipNames(pModel),
		staged,
		status))
	{
		m_SkillBindingStatus =
			"Load rejected; current Skill Bindings preserved: " + status;
		return false;
	}

	m_SkillBindingDocument = std::move(staged);
	m_iSelectedSkillBinding = -1;
	m_iSelectedSkillClip = 0;
	m_bSkillBindingDirty = false;
	m_SkillBindingStatus = status;
	return true;
}

bool_t Client::CAnimation_Tool::Save_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const shared_ptr<CCharacter>& pCharacter)
{
	if (nullptr == pCharacter || nullptr == pCharacter->Get_Spec() ||
		nullptr == pCharacter->Get_Spec()->pAssetName ||
		m_AssetName != pCharacter->Get_Spec()->pAssetName)
	{
		m_SkillBindingStatus =
			"Skill Bindings can only be saved for the selected Scene Character.";
		return false;
	}

	std::string status;
	if (!CAnimationSkillBindingDocument::Save_Atomic(
		m_SkillBindingDocument,
		m_AssetName,
		pCharacter->Get_Spec()->eCharacterClass,
		CPlayerSkillCatalog::Get_Skills(),
		Collect_ClipNames(pModel),
		status))
	{
		m_SkillBindingStatus =
			"Save rejected; destination and current bindings preserved: " + status;
		return false;
	}

	m_bSkillBindingDirty = false;
	if (!pCharacter->Reload_SkillAnimationBindings())
	{
		m_SkillBindingStatus = status +
			" [saved, but the live Character kept its previous binding set]";
		return true;
	}
	m_SkillBindingStatus = status + " [live Character refreshed]";
	return true;
}

bool_t Client::CAnimation_Tool::Create_SkillBindingDraft(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (nullptr == pModel || m_AssetName.empty())
		return false;
	const char_t* currentClip = pModel->Get_AnimationName(
		pModel->Get_CurrentAnimIndex());
	if (nullptr == currentClip)
	{
		m_SkillBindingStatus =
			"Select one model clip before creating a repair draft.";
		return false;
	}

	ANIMATION_SKILL_BINDING_DOCUMENT staged;
	staged.strAnimationAssetId = m_AssetName;
	staged.eCharacterClass = characterClass;
	for (const PLAYER_SKILL_DEFINITION& definition :
		CPlayerSkillCatalog::Get_Skills())
	{
		if (definition.eCharacterClass != characterClass)
			continue;
		ANIMATION_SKILL_BINDING binding;
		binding.iSkillId = definition.iSkillId;
		const bool_t isStaged =
			LostArk::Shared::PLAYER_SKILL_KIND::COMBO ==
				definition.eSkillKind ||
			LostArk::Shared::PLAYER_SKILL_KIND::HOLD == definition.eSkillKind ||
			LostArk::Shared::PLAYER_SKILL_KIND::COUNTER ==
				definition.eSkillKind;
		const std::size_t stageCount =
			isStaged ? definition.iComboStageCount : 1u;
		ANIMATION_SKILL_STAGE seedStage;
		seedStage.Clips.assign(
			1u, ANIMATION_SKILL_CLIP{ currentClip, 0u, 1.f });
		binding.Stages.assign(stageCount, seedStage);
		staged.Bindings.push_back(std::move(binding));
	}
	if (staged.Bindings.empty())
	{
		m_SkillBindingStatus =
			"PlayerSkills has no definitions for this Character class.";
		return false;
	}

	m_SkillBindingDocument = std::move(staged);
	m_iSelectedSkillBinding = 0;
	m_iSelectedSkillClip = 0;
	m_bSkillBindingDirty = true;
	m_SkillBindingStatus =
		"Created a complete repair draft. Every row currently uses " +
		std::string(currentClip) +
		"; assign intended clips before Save.";
	return true;
}

void Client::CAnimation_Tool::Render_SkillBindingReloadConfirmation(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (m_bSkillBindingReloadConfirmationRequested)
	{
		ImGui::OpenPopup("Discard unsaved Skill Animation Bindings?");
		m_bSkillBindingReloadConfirmationRequested = false;
	}
	if (!ImGui::BeginPopupModal(
		"Discard unsaved Skill Animation Bindings?",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}
	ImGui::TextUnformatted(
		"Reload replaces only the unsaved key/skill animation binding document.");
	if (ImGui::Button("Discard Bindings and Reload"))
	{
		if (Load_SkillBindings(pModel, characterClass))
			ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		ImGui::CloseCurrentPopup();
	ImGui::EndPopup();
}

void Client::CAnimation_Tool::Render_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const shared_ptr<CCharacter>& pCharacter)
{
	ImGui::SeparatorText("Key -> Skill Animation");
	if (nullptr == pCharacter || nullptr == pCharacter->Get_Spec() ||
		nullptr == pCharacter->Get_Spec()->pAssetName ||
		m_AssetName != pCharacter->Get_Spec()->pAssetName)
	{
		ImGui::TextDisabled(
			"Select Scene Character to author gameplay key bindings."
			" Reference-only preview assets are not playable classes.");
		return;
	}

	const CHARACTER_SPEC* spec = pCharacter->Get_Spec();
	if (!m_bSkillBindingLoadAttempted)
	{
		m_bSkillBindingLoadAttempted = true;
		Load_SkillBindings(pModel, spec->eCharacterClass);
	}
	if (m_SkillBindingDocument.Bindings.empty())
	{
		ImGui::TextWrapped("%s", m_SkillBindingStatus.c_str());
		ImGui::TextWrapped(
			"The Character remains available even when this document is missing or "
			"invalid. Create a complete in-memory repair draft from the currently "
			"selected model clip, then assign each skill and Save.");
		if (ImGui::Button("Create Repair Draft from Current Clip"))
			Create_SkillBindingDraft(pModel, spec->eCharacterClass);
		return;
	}

	ImGui::TextWrapped(
		"PlayerSkills owns key -> skillId and Server timing. This panel saves only "
		"the approved skillId -> ordered presentation clips. BA1/BA2/... are "
		"indexed directly by the replicated comboStage.");
	if (ImGui::Button("Save Skill Bindings"))
		Save_SkillBindings(pModel, pCharacter);
	ImGui::SameLine();
	if (ImGui::Button("Reload Skill Bindings"))
	{
		if (m_bSkillBindingDirty)
			m_bSkillBindingReloadConfirmationRequested = true;
		else
			Load_SkillBindings(pModel, spec->eCharacterClass);
	}
	if (m_bSkillBindingDirty)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted("*");
	}
	Render_SkillBindingReloadConfirmation(pModel, spec->eCharacterClass);
	if (!m_SkillBindingStatus.empty())
		ImGui::TextWrapped("%s", m_SkillBindingStatus.c_str());

	const uint32_t currentAnimationIndex = pModel->Get_CurrentAnimIndex();
	const char_t* currentClip =
		pModel->Get_AnimationName(currentAnimationIndex);
	ImGui::TextDisabled(
		"Current clip: %s",
		nullptr != currentClip ? currentClip : "(none)");

	std::vector<const PLAYER_SKILL_DEFINITION*> classSkills;
	for (const PLAYER_SKILL_DEFINITION& definition :
		CPlayerSkillCatalog::Get_Skills())
	{
		if (definition.eCharacterClass == spec->eCharacterClass)
			classSkills.push_back(&definition);
	}
	const auto slotRank = [](const std::string& slot)
	{
		constexpr const char_t* preferred[] =
		{
			"Q", "W", "E", "R", "A", "S", "D", "F",
			"T", "V", "ALT_V", "LMB"
		};
		for (int32_t index = 0; index < static_cast<int32_t>(std::size(preferred)); ++index)
		{
			if (slot == preferred[index])
				return index;
		}
		return static_cast<int32_t>(std::size(preferred));
	};
	std::sort(classSkills.begin(), classSkills.end(),
		[&](const PLAYER_SKILL_DEFINITION* left,
			const PLAYER_SKILL_DEFINITION* right)
		{
			const int32_t leftRank = slotRank(left->strInputSlot);
			const int32_t rightRank = slotRank(right->strInputSlot);
			return leftRank != rightRank ? leftRank < rightRank :
				left->strInputSlot < right->strInputSlot;
		});

	if (!ImGui::BeginChild(
		"##skillbindings",
		ImVec2(0.f, 300.f),
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		return;
	}
	for (const PLAYER_SKILL_DEFINITION* definition : classSkills)
	{
		const std::string slotLabel =
			"ALT_V" == definition->strInputSlot ? "ALT+V" :
			("LMB" == definition->strInputSlot ? "BA / LMB" :
				definition->strInputSlot);
		ANIMATION_SKILL_BINDING* binding =
			Find_SkillBinding(definition->iSkillId);
		if (nullptr == binding)
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.35f, 0.35f, 1.f),
				"%-8s  %u missing authored binding",
				slotLabel.c_str(),
				definition->iSkillId);
			continue;
		}
		const int32_t bindingIndex = static_cast<int32_t>(
			binding - m_SkillBindingDocument.Bindings.data());
		ImGui::PushID(bindingIndex);
		const bool_t isCombo =
			LostArk::Shared::PLAYER_SKILL_KIND::COMBO ==
			definition->eSkillKind;
		const bool_t isStaged = isCombo ||
			LostArk::Shared::PLAYER_SKILL_KIND::HOLD ==
				definition->eSkillKind ||
			LostArk::Shared::PLAYER_SKILL_KIND::COUNTER ==
				definition->eSkillKind;
		char_t header[192]{};
		snprintf(
			header,
			sizeof(header),
			"%s  %u  %s  [%s]",
			slotLabel.c_str(),
			definition->iSkillId,
			definition->strDisplayName.c_str(),
			isCombo ? "COMBO" : "ACTIVE");
		if (ImGui::TreeNodeEx("##binding", ImGuiTreeNodeFlags_DefaultOpen, "%s", header))
		{
			for (int32_t stageIndex = 0;
				stageIndex < static_cast<int32_t>(binding->Stages.size());
				++stageIndex)
			{
				ImGui::PushID(stageIndex);
				const ANIMATION_SKILL_STAGE& stage = binding->Stages[stageIndex];
				if (isStaged)
					ImGui::TextDisabled("BA%d", stageIndex + 1);
				for (int32_t clipIndex = 0;
					clipIndex < static_cast<int32_t>(stage.Clips.size());
					++clipIndex)
				{
					ImGui::PushID(clipIndex);
					char_t clipLabel[MAX_PATH + 32]{};
					if (isStaged)
					{
						snprintf(
							clipLabel,
							sizeof(clipLabel),
							"  BA%d.%d  %s",
							stageIndex + 1,
							clipIndex + 1,
							stage.Clips[clipIndex].strClipName.c_str());
					}
					else
					{
						snprintf(
							clipLabel,
							sizeof(clipLabel),
							"clip%d  %s",
							clipIndex + 1,
							stage.Clips[clipIndex].strClipName.c_str());
					}
					const bool_t selected =
						m_iSelectedSkillBinding == bindingIndex &&
						m_iSelectedSkillStage == stageIndex &&
						m_iSelectedSkillClip == clipIndex;
					if (ImGui::Selectable(clipLabel, selected))
					{
						m_iSelectedSkillBinding = bindingIndex;
						m_iSelectedSkillStage = stageIndex;
						m_iSelectedSkillClip = clipIndex;
						Select_Clip(pModel, stage.Clips[clipIndex].strClipName);
					}
					ImGui::PopID();
				}
				ImGui::PopID();
			}

			int32_t selectedStage =
				m_iSelectedSkillBinding == bindingIndex ?
				m_iSelectedSkillStage : 0;
			if (selectedStage < 0 ||
				selectedStage >= static_cast<int32_t>(binding->Stages.size()))
			{
				selectedStage = 0;
			}
			std::vector<ANIMATION_SKILL_CLIP>& selectedClips =
				binding->Stages[selectedStage].Clips;
			int32_t selectedClip =
				m_iSelectedSkillBinding == bindingIndex ?
				m_iSelectedSkillClip : 0;
			if (selectedClip < 0 ||
				selectedClip >= static_cast<int32_t>(selectedClips.size()))
			{
				selectedClip = 0;
			}
			std::size_t boundClips = 0u;
			for (const ANIMATION_SKILL_STAGE& stage : binding->Stages)
				boundClips += stage.Clips.size();

			ImGui::BeginDisabled(nullptr == currentClip);
			if (ImGui::Button("Assign Current Clip to Selected Step"))
			{
				selectedClips[selectedClip].strClipName = currentClip;
				m_iSelectedSkillBinding = bindingIndex;
				m_iSelectedSkillStage = selectedStage;
				m_iSelectedSkillClip = selectedClip;
				m_bSkillBindingDirty = true;
				m_SkillBindingStatus =
					"Assigned " + std::string(currentClip) + " to " +
					slotLabel + (isStaged ?
						(" BA" + std::to_string(selectedStage + 1) + "." +
							std::to_string(selectedClip + 1)) :
						(" clip" + std::to_string(selectedClip + 1)));
			}
			ImGui::SameLine();
			/* A stage may hold several clips, so this adds inside the selected
			stage. The stage count itself stays Server-owned. */
			if (ImGui::Button("Add Current Step") && boundClips < 16u)
			{
				selectedClips.insert(
					selectedClips.begin() + selectedClip + 1,
					ANIMATION_SKILL_CLIP{ currentClip, 0u, 1.f });
				m_iSelectedSkillBinding = bindingIndex;
				m_iSelectedSkillStage = selectedStage;
				m_iSelectedSkillClip = selectedClip + 1;
				m_bSkillBindingDirty = true;
			}
			ImGui::EndDisabled();

			if (selectedClips.size() > 1u)
			{
				if (ImGui::SmallButton("Up") && selectedClip > 0)
				{
					std::swap(
						selectedClips[selectedClip],
						selectedClips[selectedClip - 1]);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillStage = selectedStage;
					m_iSelectedSkillClip = selectedClip - 1;
					m_bSkillBindingDirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Down") &&
					selectedClip + 1 < static_cast<int32_t>(selectedClips.size()))
				{
					std::swap(
						selectedClips[selectedClip],
						selectedClips[selectedClip + 1]);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillStage = selectedStage;
					m_iSelectedSkillClip = selectedClip + 1;
					m_bSkillBindingDirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Remove Step"))
				{
					selectedClips.erase(selectedClips.begin() + selectedClip);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillStage = selectedStage;
					m_iSelectedSkillClip = std::min(
						selectedClip,
						static_cast<int32_t>(selectedClips.size()) - 1);
					m_bSkillBindingDirty = true;
				}
			}
			if (isStaged)
			{
				ImGui::TextDisabled(
					"BA stage count is fixed by PlayerSkills comboStages (%zu).",
					definition->iComboStageCount);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::EndChild();
}
