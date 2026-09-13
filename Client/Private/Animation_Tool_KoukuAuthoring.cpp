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




void Client::CAnimation_Tool::Render_KoukuSaydonActionBindings(
	const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SeparatorText("KoukuSaydon Extracted Action Sequences");
	ImGui::TextColored(
		ImVec4(0.95f, 0.75f, 0.2f, 1.f),
		"Local Extracted Action Preview / REFERENCE_ONLY");
	ImGui::TextWrapped(
		"Planner names and source order come from the immutable extracted action reference. "
		"Create Pattern copies effective clips into a separate local REFERENCE_ONLY draft; "
		"it never creates a Server Product boss pattern.");

	ImGui::TextDisabled(
		"Categories use exact physical models. Large-named actions use the loaded BossCatalog body scale and hammer transform. MN_RPCT_07 shares the MN_RPCT_05 body:");
	for (std::size_t iProfile = 0u;
		iProfile < KOUKU_SAYDON_ACTION_PROFILES.size(); ++iProfile)
	{
		const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT& Profile =
			KOUKU_SAYDON_ACTION_PROFILES[iProfile];
		if (iProfile > 0u)
			ImGui::SameLine();
		ImGui::PushID(static_cast<int32_t>(iProfile));
		ImGui::BeginDisabled(Profile.pProfileId == m_strKoukuSaydonProfileId);
		const std::string profileButton = std::string(Profile.pCategoryLabel) +
			"##KoukuSaydonProfile_" + Profile.pProfileId;
		if (ImGui::SmallButton(profileButton.c_str()))
			(void)Open_KoukuSaydonProfile(Profile.pProfileId);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s | %s", Profile.pProfileId, Profile.pModelPolicy);
		ImGui::EndDisabled();
		ImGui::PopID();
	}

	if (!m_bKoukuSaydonActionLoadAttempted)
	{
		m_bKoukuSaydonActionLoadAttempted = true;
		(void)Load_KoukuSaydonActionBindings(pModel);
	}
	if (m_bKoukuSaydonCompositionAnimationPreviewPending ||
		m_bKoukuSaydonCompositionPatternPreviewPending)
	{
		(void)Start_PendingKoukuSaydonCompositionPreview(pModel);
	}
	if (m_KoukuSaydonActionReference.Actions.empty())
	{
		if (!m_strKoukuSaydonActionStatus.empty())
			ImGui::TextWrapped("%s", m_strKoukuSaydonActionStatus.c_str());
		return;
	}

	ImGui::Text(
		"Profile: %s | Actions: %zu | Local overrides: %zu | Local patterns: %zu",
		m_strKoukuSaydonProfileId.c_str(),
		m_KoukuSaydonActionReference.Actions.size(),
		m_KoukuSaydonActionAuthored.Bindings.size(),
		m_KoukuSaydonAnimationPatterns.Patterns.size());
	ImGui::BeginDisabled(m_bKoukuSaydonPatternPreviewPlaying);
	if (ImGui::Button("Save KoukuSaydon Action Bindings"))
		(void)Save_KoukuSaydonActionBindings(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Save KoukuSaydon Patterns"))
		(void)Save_KoukuSaydonAnimationPatterns(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Reload KoukuSaydon Animation Workspace"))
	{
		if (m_bKoukuSaydonActionDirty || m_bKoukuSaydonPatternDirty)
			m_bKoukuSaydonActionReloadConfirmationRequested = true;
		else
			(void)Load_KoukuSaydonActionBindings(pModel);
	}
	ImGui::EndDisabled();
	if (m_bKoukuSaydonActionDirty || m_bKoukuSaydonPatternDirty)
	{
		ImGui::SameLine();
		ImGui::Text("*%s%s",
			m_bKoukuSaydonActionDirty ? " action" : "",
			m_bKoukuSaydonPatternDirty ? " pattern" : "");
	}
	if (m_bKoukuSaydonActionReloadConfirmationRequested)
	{
		ImGui::OpenPopup("Discard unsaved KoukuSaydon Animation drafts?");
		m_bKoukuSaydonActionReloadConfirmationRequested = false;
	}
	if (ImGui::BeginPopupModal(
		"Discard unsaved KoukuSaydon Animation drafts?", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(
			"Reload replaces the unsaved sparse slot overrides and local REFERENCE_ONLY patterns.");
		if (ImGui::Button("Discard Drafts and Reload"))
		{
			if (Load_KoukuSaydonActionBindings(pModel))
				ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	if (!m_strKoukuSaydonActionStatus.empty())
		ImGui::TextWrapped("%s", m_strKoukuSaydonActionStatus.c_str());

	ImGui::BeginDisabled(m_bKoukuSaydonPatternPreviewPlaying);
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##KoukuSaydonActionFilter", "Filter Korean name or sourceActionId...",
		m_KoukuSaydonActionFilter, sizeof(m_KoukuSaydonActionFilter));
	const std::string strFilter = m_KoukuSaydonActionFilter;
	std::vector<int32_t> VisibleActions;
	VisibleActions.reserve(m_KoukuSaydonActionReference.Actions.size());
	for (int32_t iAction = 0;
		iAction < static_cast<int32_t>(m_KoukuSaydonActionReference.Actions.size());
		++iAction)
	{
		const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& Action =
			m_KoukuSaydonActionReference.Actions[iAction];
		if (!strFilter.empty() &&
			std::string::npos == Action.strDisplayName.find(strFilter) &&
			std::string::npos == std::to_string(
				Action.iSourceActionId).find(strFilter))
		{
			continue;
		}
		VisibleActions.push_back(iAction);
	}

	const f32_t fKoukuSaydonWorkspaceWidth =
		(std::max)(0.f, ImGui::GetContentRegionAvail().x);
	const bool_t bKoukuSaydonPanelsSideBySide =
		fKoukuSaydonWorkspaceWidth >= KOUKU_SAYDON_ACTION_LIST_MIN_WIDTH +
			KOUKU_SAYDON_ACTION_SPLITTER_WIDTH + KOUKU_SAYDON_ACTION_DETAIL_MIN_WIDTH;
	const f32_t fKoukuSaydonActionListMaximumWidth = (std::max)(
		KOUKU_SAYDON_ACTION_LIST_MIN_WIDTH,
		fKoukuSaydonWorkspaceWidth - KOUKU_SAYDON_ACTION_SPLITTER_WIDTH -
			KOUKU_SAYDON_ACTION_DETAIL_MIN_WIDTH);
	m_fKoukuSaydonActionListWidth = std::clamp(
		m_fKoukuSaydonActionListWidth,
		KOUKU_SAYDON_ACTION_LIST_MIN_WIDTH,
		fKoukuSaydonActionListMaximumWidth);
	const f32_t fKoukuSaydonActionListWidth =
		bKoukuSaydonPanelsSideBySide ? m_fKoukuSaydonActionListWidth : 0.f;
	const f32_t fKoukuSaydonActionListHeight =
		bKoukuSaydonPanelsSideBySide ? 460.f : 260.f;

	if (ImGui::BeginChild(
		"##KoukuSaydonActions",
		ImVec2(fKoukuSaydonActionListWidth, fKoukuSaydonActionListHeight),
		ImGuiChildFlags_Borders))
	{
		ImGui::TextDisabled("Korean Action / Sequence (%zu)", VisibleActions.size());
		ImGuiListClipper Clipper;
		Clipper.Begin(static_cast<int32_t>(VisibleActions.size()));
		while (Clipper.Step())
		{
			for (int32_t iVisible = Clipper.DisplayStart;
				iVisible < Clipper.DisplayEnd; ++iVisible)
			{
				const int32_t iAction = VisibleActions[iVisible];
				const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& Action =
					m_KoukuSaydonActionReference.Actions[iAction];
				char_t Label[512]{};
				snprintf(
					Label, sizeof(Label), "%u  %s  [%s]",
					Action.iSourceActionId,
					Action.strDisplayName.c_str(),
					Action.strReviewStatus.c_str());
				ImGui::PushID(iAction);
				if (ImGui::Selectable(
					Label, m_iSelectedKoukuSaydonAction == iAction))
				{
					m_iSelectedKoukuSaydonAction = iAction;
					m_iSelectedKoukuSaydonStage = 0;
					m_iSelectedKoukuSaydonSlot = 0;
					m_iSelectedKoukuSaydonActionClip = 0;
					for (int32_t iStage = 0;
						iStage < static_cast<int32_t>(Action.Stages.size());
						++iStage)
					{
						if (!Action.Stages[iStage].Slots.empty())
						{
							m_iSelectedKoukuSaydonStage = iStage;
							break;
						}
					}
				}
				ImGui::PopID();
			}
		}
	}
	ImGui::EndChild();
	if (bKoukuSaydonPanelsSideBySide)
	{
		ImGui::SameLine(0.f, 0.f);
		(void)ImGui::InvisibleButton("##KoukuSaydonActionSplitter",
			ImVec2(KOUKU_SAYDON_ACTION_SPLITTER_WIDTH, 460.f));
		if (ImGui::IsItemHovered() || ImGui::IsItemActive())
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		if (ImGui::IsItemActive())
		{
			m_fKoukuSaydonActionListWidth = std::clamp(
				m_fKoukuSaydonActionListWidth + ImGui::GetIO().MouseDelta.x,
				KOUKU_SAYDON_ACTION_LIST_MIN_WIDTH,
				fKoukuSaydonActionListMaximumWidth);
		}
		ImGui::SameLine(0.f, 0.f);
	}
	else
	{
		ImGui::TextDisabled(
			"Compact viewport: Action list is stacked above Selected Slot Detail.");
	}

	if (ImGui::BeginChild(
		"##KoukuSaydonActionDetail", ImVec2(0.f, 460.f),
		ImGuiChildFlags_Borders))
	{
		if (m_iSelectedKoukuSaydonAction < 0 ||
			m_iSelectedKoukuSaydonAction >= static_cast<int32_t>(
				m_KoukuSaydonActionReference.Actions.size()))
		{
			ImGui::TextUnformatted("Select one extracted action sequence.");
		}
		else
		{
			const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& Action =
				m_KoukuSaydonActionReference.Actions[m_iSelectedKoukuSaydonAction];
			ImGui::TextWrapped(
				"%u | %s", Action.iSourceActionId,
				Action.strDisplayName.c_str());
			if ("HOLDOUT" == Action.strReviewStatus)
			{
				ImGui::TextColored(
					ImVec4(1.f, 0.45f, 0.25f, 1.f),
					"HOLDOUT: local inspection is allowed; Product promotion is not implied.");
			}

			if (Action.Stages.empty())
			{
				ImGui::TextUnformatted("This action has no extracted stages.");
			}
			else
			{
				m_iSelectedKoukuSaydonStage = std::clamp(
					m_iSelectedKoukuSaydonStage, 0,
					static_cast<int32_t>(Action.Stages.size()) - 1);
				const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& SelectedStage =
					Action.Stages[m_iSelectedKoukuSaydonStage];
				char_t StageLabel[192]{};
				snprintf(
					StageLabel, sizeof(StageLabel), "%u / %s | %zu slot(s)",
					SelectedStage.iStageOrdinal,
					SelectedStage.strStageId.c_str(),
					SelectedStage.Slots.size());
				ImGui::SetNextItemWidth(-1.f);
				if (ImGui::BeginCombo("Stage", StageLabel))
				{
					for (int32_t iStage = 0;
						iStage < static_cast<int32_t>(Action.Stages.size());
						++iStage)
					{
						const auto& Stage = Action.Stages[iStage];
						char_t CandidateLabel[192]{};
						snprintf(
							CandidateLabel, sizeof(CandidateLabel),
							"%u / %s | %zu slot(s)%s",
							Stage.iStageOrdinal, Stage.strStageId.c_str(),
							Stage.Slots.size(),
							Stage.HoldoutClipNames.empty() ? "" : " | HOLDOUT refs");
						ImGui::PushID(iStage);
						if (ImGui::Selectable(
							CandidateLabel, iStage == m_iSelectedKoukuSaydonStage))
						{
							m_iSelectedKoukuSaydonStage = iStage;
							m_iSelectedKoukuSaydonSlot = 0;
						}
						ImGui::PopID();
					}
					ImGui::EndCombo();
				}

				const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& Stage =
					Action.Stages[m_iSelectedKoukuSaydonStage];
				if (!Stage.HoldoutClipNames.empty())
				{
					ImGui::TextDisabled("Unresolved extracted clip evidence:");
					for (const std::string& Holdout : Stage.HoldoutClipNames)
						ImGui::BulletText("%s", Holdout.c_str());
				}

				if (ImGui::BeginChild(
					"##KoukuSaydonSlots", ImVec2(0.f, 105.f),
					ImGuiChildFlags_Borders))
				{
					for (int32_t iSlot = 0;
						iSlot < static_cast<int32_t>(Stage.Slots.size()); ++iSlot)
					{
						const auto& Slot = Stage.Slots[iSlot];
						const KOUKU_SAYDON_ANIMATION_ACTION_BINDING* pOverride =
							Find_KoukuSaydonActionBinding(
								Action.iSourceActionId, Stage.strStageId,
								Slot.strSlotId);
						const std::string& EffectiveClip = nullptr != pOverride ?
							pOverride->strRuntimeClip : Slot.strRuntimeClip;
						const std::string Label = Slot.strSlotId + "  " +
							EffectiveClip + (nullptr != pOverride ? "  *" : "");
						ImGui::PushID(iSlot);
						if (ImGui::Selectable(
							Label.c_str(), iSlot == m_iSelectedKoukuSaydonSlot))
						{
							m_iSelectedKoukuSaydonSlot = iSlot;
							Select_Clip(pModel, EffectiveClip);
							Apply_KoukuSaydonPreviewScale(pModel, Resolve_ActionPreviewScale(m_KoukuSaydonActionReference, Action.iSourceActionId));
						}
						ImGui::PopID();
					}
				}
				ImGui::EndChild();

				if (Stage.Slots.empty())
				{
					ImGui::TextDisabled(
						"This stage has no exact physical WModel slot to author.");
				}
				else
				{
					m_iSelectedKoukuSaydonSlot = std::clamp(
						m_iSelectedKoukuSaydonSlot, 0,
						static_cast<int32_t>(Stage.Slots.size()) - 1);
					const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& Slot =
						Stage.Slots[m_iSelectedKoukuSaydonSlot];
					const KOUKU_SAYDON_ANIMATION_ACTION_BINDING* pOverride =
						Find_KoukuSaydonActionBinding(
							Action.iSourceActionId, Stage.strStageId,
							Slot.strSlotId);
					std::string strEditedRuntimeClip = nullptr != pOverride ?
						pOverride->strRuntimeClip : Slot.strRuntimeClip;
					int32_t iEditedSourceStartMs = static_cast<int32_t>(
						nullptr != pOverride ? pOverride->iSourceStartMs :
						Slot.iSourceStartMs);
					int32_t iEditedPlayMs = static_cast<int32_t>(
						nullptr != pOverride ? pOverride->iPlayMs : Slot.iPlayMs);
					f32_t fEditedPlayRate = nullptr != pOverride ?
						pOverride->fPlayRate : Slot.fPlayRate;
					bool_t bEditedLoop = nullptr != pOverride ?
						pOverride->bLoop : Slot.bLoop;
					bool_t bEdited = false;

					ImGui::SeparatorText("Selected Slot Detail");
					ImGui::Text("Identity: %s / %s", Stage.strStageId.c_str(),
						Slot.strSlotId.c_str());
					ImGui::TextWrapped(
						"Extracted: %s -> default runtime: %s",
						Slot.strExtractedClip.c_str(), Slot.strRuntimeClip.c_str());
					const char_t* pCurrentClip = pModel->Get_AnimationName(
						pModel->Get_CurrentAnimIndex());
					ImGui::TextDisabled(
						"Current WModel clip: %s",
						nullptr != pCurrentClip ? pCurrentClip : "(none)");

					const std::vector<std::string> AvailableClips =
						Collect_ClipNames(pModel);
					ImGui::SetNextItemWidth(-1.f);
					if (ImGui::BeginCombo(
						"Runtime Clip", strEditedRuntimeClip.c_str()))
					{
						for (const std::string& Clip : AvailableClips)
						{
							if (ImGui::Selectable(
								Clip.c_str(), Clip == strEditedRuntimeClip))
							{
								strEditedRuntimeClip = Clip;
								bEdited = true;
								Select_Clip(pModel, Clip);
								Apply_KoukuSaydonPreviewScale(pModel, Resolve_ActionPreviewScale(m_KoukuSaydonActionReference, Action.iSourceActionId));
							}
						}
						ImGui::EndCombo();
					}
					if (nullptr != pCurrentClip &&
						ImGui::Button("Assign Current WModel Clip"))
					{
						strEditedRuntimeClip = pCurrentClip;
						bEdited = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("Preview Effective Clip"))
					{
						Select_Clip(pModel, strEditedRuntimeClip);
						Apply_KoukuSaydonPreviewScale(pModel, Resolve_ActionPreviewScale(m_KoukuSaydonActionReference, Action.iSourceActionId));
					}

					if (ImGui::InputInt(
						"Source Start (ms)", &iEditedSourceStartMs))
					{
						iEditedSourceStartMs = std::clamp(
							iEditedSourceStartMs, 0, 600000);
						bEdited = true;
					}
					if (ImGui::InputInt("Play Window (ms)", &iEditedPlayMs))
					{
						iEditedPlayMs = std::clamp(iEditedPlayMs, 1, 600000);
						bEdited = true;
					}
					if (ImGui::InputFloat(
						"Play Rate", &fEditedPlayRate, 0.01f, 0.1f, "%.3f"))
					{
						fEditedPlayRate = std::clamp(fEditedPlayRate, 0.01f, 16.f);
						bEdited = true;
					}
					if (ImGui::Checkbox("Loop", &bEditedLoop))
						bEdited = true;

					bool_t bReset = false;
					if (nullptr != pOverride)
					{
						if (ImGui::Button("Reset Slot to Extracted Default"))
							bReset = true;
					}
					if (bReset)
					{
						Remove_KoukuSaydonActionBinding(
							Action.iSourceActionId, Stage.strStageId,
							Slot.strSlotId);
						Select_Clip(pModel, Slot.strRuntimeClip);
						Apply_KoukuSaydonPreviewScale(pModel, Resolve_ActionPreviewScale(m_KoukuSaydonActionReference, Action.iSourceActionId));
					}
					else if (bEdited)
					{
						Upsert_KoukuSaydonActionBinding(
							Slot, Action.iSourceActionId, Stage.strStageId,
							strEditedRuntimeClip,
							static_cast<std::uint32_t>(iEditedSourceStartMs),
							static_cast<std::uint32_t>(iEditedPlayMs),
							fEditedPlayRate, bEditedLoop);
					}
				}
			}
		}
	}
	ImGui::EndChild();
	ImGui::EndDisabled();
	Render_KoukuSaydonPatternAuthoring(pModel);
}

void Client::CAnimation_Tool::Render_KoukuSaydonPatternAuthoring(
	const shared_ptr<Engine::CModel>& pModel)
{
	struct EFFECTIVE_ACTION_CLIP final
	{
		const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE* pStage = nullptr;
		const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE* pSlot = nullptr;
		const KOUKU_SAYDON_ANIMATION_ACTION_BINDING* pOverride = nullptr;
	};

	ImGui::SeparatorText("KoukuSaydon Local Pattern Authoring");
	ImGui::TextDisabled(
		"Flat planner order -> effective physical clips -> local REFERENCE_ONLY Pattern occurrences");

	const KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE* pSelectedAction = nullptr;
	if (m_iSelectedKoukuSaydonAction >= 0 &&
		m_iSelectedKoukuSaydonAction < static_cast<int32_t>(
			m_KoukuSaydonActionReference.Actions.size()))
	{
		pSelectedAction =
			&m_KoukuSaydonActionReference.Actions[m_iSelectedKoukuSaydonAction];
	}
	std::vector<EFFECTIVE_ACTION_CLIP> EffectiveActionClips;
	if (nullptr != pSelectedAction)
	{
		for (const KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& Stage :
			pSelectedAction->Stages)
		{
			for (const KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& Slot : Stage.Slots)
			{
				EffectiveActionClips.push_back(EFFECTIVE_ACTION_CLIP{
					&Stage,
					&Slot,
					Find_KoukuSaydonActionBinding(
						pSelectedAction->iSourceActionId,
						Stage.strStageId,
						Slot.strSlotId) });
			}
		}
	}

	if (nullptr == pSelectedAction)
	{
		ImGui::TextDisabled("Select one planner action above to list its complete clip sequence.");
	}
	else
	{
		ImGui::Text(
			"Selected planner action: %u | %s | %zu effective clip occurrence(s)",
			pSelectedAction->iSourceActionId,
			pSelectedAction->strDisplayName.c_str(),
			EffectiveActionClips.size());
		const bool_t bActionPatternAdmitted =
			"REVIEW_CANDIDATE" == pSelectedAction->strReviewStatus &&
			!EffectiveActionClips.empty();
		ImGui::BeginDisabled(
			m_bKoukuSaydonPatternPreviewPlaying || !bActionPatternAdmitted);
		if (ImGui::Button("Preview Action"))
		{
			const std::string strPreviewPatternId = "kakulsaydon." +
				m_strKoukuSaydonProfileId + ".pattern." +
				std::to_string(m_KoukuSaydonAnimationPatterns.iNextPatternOrdinal);
			KOUKU_SAYDON_ANIMATION_PATTERN PreviewPattern;
			std::string Status;
			if (Build_KoukuSaydonPatternFromAction(
					pModel, *pSelectedAction, strPreviewPatternId,
					PreviewPattern, Status))
			{
				(void)Start_KoukuSaydonPatternPreview(
					pModel, PreviewPattern,
					"planner action " +
					std::to_string(pSelectedAction->iSourceActionId));
			}
			else
			{
				m_strKoukuSaydonPatternStatus = Status;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Create Pattern"))
		{
			const std::string strPatternId = "kakulsaydon." +
				m_strKoukuSaydonProfileId + ".pattern." +
				std::to_string(m_KoukuSaydonAnimationPatterns.iNextPatternOrdinal);
			KOUKU_SAYDON_ANIMATION_PATTERN Created;
			std::string Status;
			if (Build_KoukuSaydonPatternFromAction(
					pModel, *pSelectedAction, strPatternId,
					Created, Status))
			{
				KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT Candidate =
					m_KoukuSaydonAnimationPatterns;
				Candidate.Patterns.push_back(std::move(Created));
				++Candidate.iNextPatternOrdinal;
				const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* const pProfile =
					Find_KoukuSaydonActionProfile(m_strKoukuSaydonProfileId);
				if (nullptr != pProfile &&
					CKoukuSaydonAnimationPatternDocument::Validate(
						Candidate,
						m_KoukuSaydonActionReference,
						pProfile->pProfileId,
						pProfile->pModelAssetId,
						Collect_ClipNames(pModel),
						Status))
				{
					m_KoukuSaydonAnimationPatterns = std::move(Candidate);
					m_iSelectedKoukuSaydonPattern = static_cast<int32_t>(
						m_KoukuSaydonAnimationPatterns.Patterns.size() - 1u);
					m_iSelectedKoukuSaydonPatternClip = 0;
					m_bKoukuSaydonPatternDirty = true;
					m_strKoukuSaydonPatternStatus =
						"Created local Pattern " + strPatternId + " from planner action " +
						std::to_string(pSelectedAction->iSourceActionId) + ".";
				}
				else
				{
					m_strKoukuSaydonPatternStatus =
						"Create Pattern rejected; current draft preserved: " + Status;
				}
			}
			else
			{
				m_strKoukuSaydonPatternStatus = Status;
			}
		}
		ImGui::EndDisabled();
		if (!bActionPatternAdmitted)
		{
			ImGui::TextDisabled(
				"Preview Action / Create Pattern stays disabled until this action is REVIEW_CANDIDATE with at least one exact physical clip. HOLDOUT evidence remains listed below.");
		}

		m_iSelectedKoukuSaydonActionClip = EffectiveActionClips.empty() ? 0 :
			std::clamp(
				m_iSelectedKoukuSaydonActionClip, 0,
				static_cast<int32_t>(EffectiveActionClips.size()) - 1);
		if (ImGui::BeginChild(
			"##KoukuSaydonCompleteActionClips", ImVec2(0.f, 190.f),
			ImGuiChildFlags_Borders))
		{
			ImGuiListClipper Clipper;
			Clipper.Begin(static_cast<int32_t>(EffectiveActionClips.size()));
			while (Clipper.Step())
			{
				for (int32_t iClip = Clipper.DisplayStart;
					iClip < Clipper.DisplayEnd; ++iClip)
				{
					const EFFECTIVE_ACTION_CLIP& Row = EffectiveActionClips[iClip];
					const std::string& strClip = nullptr != Row.pOverride ?
						Row.pOverride->strRuntimeClip : Row.pSlot->strRuntimeClip;
					const std::uint32_t iPlayMs = nullptr != Row.pOverride ?
						Row.pOverride->iPlayMs : Row.pSlot->iPlayMs;
					char_t Label[768]{};
					snprintf(
						Label, sizeof(Label), "%04d  %s / %s  %s  %u ms%s",
						iClip + 1,
						Row.pStage->strStageId.c_str(),
						Row.pSlot->strSlotId.c_str(),
						strClip.c_str(), iPlayMs,
						nullptr != Row.pOverride ? "  *override" : "");
					ImGui::PushID(iClip);
					ImGui::BeginDisabled(m_bKoukuSaydonPatternPreviewPlaying);
					if (ImGui::Selectable(
						Label, m_iSelectedKoukuSaydonActionClip == iClip))
					{
						m_iSelectedKoukuSaydonActionClip = iClip;
						Select_Clip(pModel, strClip);
						Apply_KoukuSaydonPreviewScale(pModel,
							Resolve_ActionPreviewScale(m_KoukuSaydonActionReference, pSelectedAction->iSourceActionId));
					}
					ImGui::EndDisabled();
					ImGui::PopID();
				}
			}
		}
		ImGui::EndChild();
	}

	ImGui::SeparatorText("Created Local Patterns");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##KoukuSaydonPatternFilter", "Filter pattern name, ID, or sourceActionId...",
		m_KoukuSaydonPatternFilter, sizeof(m_KoukuSaydonPatternFilter));
	std::vector<int32_t> VisiblePatterns;
	for (int32_t iPattern = 0;
		iPattern < static_cast<int32_t>(
			m_KoukuSaydonAnimationPatterns.Patterns.size()); ++iPattern)
	{
		const KOUKU_SAYDON_ANIMATION_PATTERN& Pattern =
			m_KoukuSaydonAnimationPatterns.Patterns[iPattern];
		if ('\0' != m_KoukuSaydonPatternFilter[0] &&
			!Contains_NoCase(Pattern.strPatternId.c_str(), m_KoukuSaydonPatternFilter) &&
			!Contains_NoCase(Pattern.strDisplayName.c_str(), m_KoukuSaydonPatternFilter) &&
			!Contains_NoCase(
				std::to_string(Pattern.iSourceActionId).c_str(),
				m_KoukuSaydonPatternFilter))
		{
			continue;
		}
		VisiblePatterns.push_back(iPattern);
	}
	if (ImGui::BeginChild(
		"##KoukuSaydonPatternList", ImVec2(0.f, 150.f), ImGuiChildFlags_Borders))
	{
		for (const int32_t iPattern : VisiblePatterns)
		{
			const KOUKU_SAYDON_ANIMATION_PATTERN& Pattern =
				m_KoukuSaydonAnimationPatterns.Patterns[iPattern];
			char_t Label[768]{};
			snprintf(
				Label, sizeof(Label), "%s | action %u | %s | %zu clips",
				Pattern.strPatternId.c_str(), Pattern.iSourceActionId,
				Pattern.strDisplayName.c_str(), Pattern.Clips.size());
			ImGui::PushID(iPattern);
			ImGui::BeginDisabled(m_bKoukuSaydonPatternPreviewPlaying);
			if (ImGui::Selectable(
				Label, m_iSelectedKoukuSaydonPattern == iPattern))
			{
				m_iSelectedKoukuSaydonPattern = iPattern;
				m_iSelectedKoukuSaydonPatternClip = 0;
			}
			ImGui::EndDisabled();
			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	if (m_KoukuSaydonAnimationPatterns.Patterns.empty())
	{
		m_iSelectedKoukuSaydonPattern = -1;
		ImGui::TextDisabled(
			"No local Pattern yet. Select an action with exact clips and press Create Pattern.");
		if (!m_strKoukuSaydonPatternStatus.empty())
			ImGui::TextWrapped("%s", m_strKoukuSaydonPatternStatus.c_str());
		return;
	}
	m_iSelectedKoukuSaydonPattern = std::clamp(
		m_iSelectedKoukuSaydonPattern, 0,
		static_cast<int32_t>(m_KoukuSaydonAnimationPatterns.Patterns.size()) - 1);
	const int32_t iSelectedPattern = m_iSelectedKoukuSaydonPattern;
	const KOUKU_SAYDON_ANIMATION_PATTERN& SelectedPattern =
		m_KoukuSaydonAnimationPatterns.Patterns[iSelectedPattern];
	m_iSelectedKoukuSaydonPatternClip = std::clamp(
		m_iSelectedKoukuSaydonPatternClip, 0,
		static_cast<int32_t>(SelectedPattern.Clips.size()) - 1);
	const int32_t iSelectedClip = m_iSelectedKoukuSaydonPatternClip;

	ImGui::Text(
		"Selected Pattern: %s | %s | source action %u",
		SelectedPattern.strPatternId.c_str(),
		SelectedPattern.strDisplayName.c_str(),
		SelectedPattern.iSourceActionId);
	ImGui::BeginDisabled(m_bKoukuSaydonPatternPreviewPlaying);
	if (ImGui::Button("Play Pattern"))
		(void)Start_KoukuSaydonPatternPreview(
			pModel, SelectedPattern, SelectedPattern.strDisplayName);
	ImGui::SameLine();
	if (ImGui::Button("Preview Selected Clip Window"))
	{
		KOUKU_SAYDON_ANIMATION_PATTERN SingleClip = SelectedPattern;
		SingleClip.Clips = { SelectedPattern.Clips[iSelectedClip] };
		(void)Start_KoukuSaydonPatternPreview(
			pModel, SingleClip, "selected Pattern clip");
	}
	ImGui::EndDisabled();
	if (m_bKoukuSaydonPatternPreviewPlaying)
	{
		ImGui::SameLine();
		if (ImGui::Button(
			m_bKoukuSaydonPatternPreviewPaused ? "Resume Pattern" : "Pause Pattern"))
		{
			m_bKoukuSaydonPatternPreviewPaused = !m_bKoukuSaydonPatternPreviewPaused;
			pModel->Set_AnimPaused(m_bKoukuSaydonPatternPreviewPaused);
		}
		ImGui::SameLine();
		if (ImGui::Button("Skip Clip"))
			Advance_KoukuSaydonPatternPreview(pModel);
		ImGui::SameLine();
		if (ImGui::Button("Stop Pattern"))
			Stop_KoukuSaydonPatternPreview(
				pModel, "KoukuSaydon Pattern preview stopped; idle restored.");
	}

	bool_t bDuplicatePattern = false;
	bool_t bDeletePattern = false;
	bool_t bDuplicateClip = false;
	bool_t bDeleteClip = false;
	bool_t bMoveClipUp = false;
	bool_t bMoveClipDown = false;
	ImGui::BeginDisabled(m_bKoukuSaydonPatternPreviewPlaying);
	if (ImGui::Button("Duplicate Pattern"))
		bDuplicatePattern = true;
	ImGui::SameLine();
	if (ImGui::Button("Delete Pattern"))
		bDeletePattern = true;
	ImGui::SameLine();
	if (ImGui::Button("Duplicate Clip"))
		bDuplicateClip = true;
	ImGui::SameLine();
	ImGui::BeginDisabled(SelectedPattern.Clips.size() <= 1u);
	if (ImGui::Button("Delete Clip"))
		bDeleteClip = true;
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(0 == iSelectedClip);
	if (ImGui::Button("Move Clip Up"))
		bMoveClipUp = true;
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(
		iSelectedClip + 1 >= static_cast<int32_t>(SelectedPattern.Clips.size()));
	if (ImGui::Button("Move Clip Down"))
		bMoveClipDown = true;
	ImGui::EndDisabled();
	ImGui::EndDisabled();
	if (SelectedPattern.Clips.size() <= 1u)
		ImGui::TextDisabled("The last clip is preserved; delete the Pattern instead.");

	if (ImGui::BeginChild(
		"##KoukuSaydonPatternClips", ImVec2(0.f, 240.f), ImGuiChildFlags_Borders))
	{
		for (int32_t iClip = 0;
			iClip < static_cast<int32_t>(SelectedPattern.Clips.size()); ++iClip)
		{
			const KOUKU_SAYDON_ANIMATION_PATTERN_CLIP& Clip =
				SelectedPattern.Clips[iClip];
			char_t Label[1024]{};
			snprintf(
				Label, sizeof(Label),
				"%03d  %s | %s / %s | %s | start %u ms, play %u ms, %.3fx | %s",
				iClip + 1, Clip.strOccurrenceId.c_str(),
				Clip.strStageId.c_str(), Clip.strSlotId.c_str(),
				Clip.strRuntimeClip.c_str(), Clip.iSourceStartMs,
				Clip.iPlayMs, Clip.fPlayRate, Clip.strEndPolicy.c_str());
			ImGui::PushID(iClip);
			ImGui::BeginDisabled(m_bKoukuSaydonPatternPreviewPlaying);
			if (ImGui::Selectable(Label, iSelectedClip == iClip))
			{
				m_iSelectedKoukuSaydonPatternClip = iClip;
				Select_Clip(pModel, Clip.strRuntimeClip);
				Apply_KoukuSaydonPreviewScale(pModel, Resolve_ActionPreviewScale(m_KoukuSaydonActionReference, SelectedPattern.iSourceActionId));
			}
			ImGui::EndDisabled();
			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* const pProfile =
		Find_KoukuSaydonActionProfile(m_strKoukuSaydonProfileId);
	const auto CommitCandidate = [this, &pModel, pProfile](
		KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT&& Candidate,
		const std::string& strSuccess)
	{
		std::string Status;
		if (nullptr == pProfile ||
			!CKoukuSaydonAnimationPatternDocument::Validate(
				Candidate,
				m_KoukuSaydonActionReference,
				pProfile->pProfileId,
				pProfile->pModelAssetId,
				Collect_ClipNames(pModel),
				Status))
		{
			m_strKoukuSaydonPatternStatus =
				"Pattern edit rejected; current draft preserved: " + Status;
			return false;
		}
		m_KoukuSaydonAnimationPatterns = std::move(Candidate);
		m_bKoukuSaydonPatternDirty = true;
		m_strKoukuSaydonPatternStatus = strSuccess;
		return true;
	};

	if (bDuplicatePattern)
	{
		KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT Candidate = m_KoukuSaydonAnimationPatterns;
		KOUKU_SAYDON_ANIMATION_PATTERN Copy = Candidate.Patterns[iSelectedPattern];
		Copy.strPatternId = "kakulsaydon." + m_strKoukuSaydonProfileId + ".pattern." +
			std::to_string(Candidate.iNextPatternOrdinal++);
		Copy.strDisplayName += " Copy";
		std::uint32_t iOrdinal = 1u;
		for (KOUKU_SAYDON_ANIMATION_PATTERN_CLIP& Clip : Copy.Clips)
			Clip.strOccurrenceId = Copy.strPatternId + ".clip." +
				std::to_string(iOrdinal++);
		Copy.iNextOccurrenceOrdinal = iOrdinal;
		const std::string strCreatedId = Copy.strPatternId;
		Candidate.Patterns.push_back(std::move(Copy));
		if (CommitCandidate(
				std::move(Candidate),
				"Duplicated Pattern as " + strCreatedId + "."))
		{
			m_iSelectedKoukuSaydonPattern = static_cast<int32_t>(
				m_KoukuSaydonAnimationPatterns.Patterns.size() - 1u);
			m_iSelectedKoukuSaydonPatternClip = 0;
		}
	}
	else if (bDeletePattern)
	{
		KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT Candidate = m_KoukuSaydonAnimationPatterns;
		const std::string strDeletedId =
			Candidate.Patterns[iSelectedPattern].strPatternId;
		Candidate.Patterns.erase(
			Candidate.Patterns.begin() + iSelectedPattern);
		if (CommitCandidate(
				std::move(Candidate),
				"Deleted local Pattern " + strDeletedId + "."))
		{
			m_iSelectedKoukuSaydonPattern = m_KoukuSaydonAnimationPatterns.Patterns.empty() ?
				-1 : (std::min)(
					iSelectedPattern,
					static_cast<int32_t>(
						m_KoukuSaydonAnimationPatterns.Patterns.size()) - 1);
			m_iSelectedKoukuSaydonPatternClip = 0;
		}
	}
	else if (bDuplicateClip)
	{
		KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT Candidate = m_KoukuSaydonAnimationPatterns;
		KOUKU_SAYDON_ANIMATION_PATTERN& Pattern =
			Candidate.Patterns[iSelectedPattern];
		KOUKU_SAYDON_ANIMATION_PATTERN_CLIP Copy = Pattern.Clips[iSelectedClip];
		Copy.strOccurrenceId = Pattern.strPatternId + ".clip." +
			std::to_string(Pattern.iNextOccurrenceOrdinal++);
		const std::string strCreatedId = Copy.strOccurrenceId;
		Pattern.Clips.insert(
			Pattern.Clips.begin() + iSelectedClip + 1, std::move(Copy));
		if (CommitCandidate(
				std::move(Candidate),
				"Duplicated clip occurrence as " + strCreatedId + "."))
		{
			m_iSelectedKoukuSaydonPatternClip = iSelectedClip + 1;
		}
	}
	else if (bDeleteClip)
	{
		KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT Candidate = m_KoukuSaydonAnimationPatterns;
		KOUKU_SAYDON_ANIMATION_PATTERN& Pattern =
			Candidate.Patterns[iSelectedPattern];
		const std::string strDeletedId =
			Pattern.Clips[iSelectedClip].strOccurrenceId;
		Pattern.Clips.erase(Pattern.Clips.begin() + iSelectedClip);
		if (CommitCandidate(
				std::move(Candidate),
				"Deleted clip occurrence " + strDeletedId + "."))
		{
			m_iSelectedKoukuSaydonPatternClip = (std::min)(
				iSelectedClip,
				static_cast<int32_t>(m_KoukuSaydonAnimationPatterns.Patterns[
					iSelectedPattern].Clips.size()) - 1);
		}
	}
	else if (bMoveClipUp || bMoveClipDown)
	{
		KOUKU_SAYDON_ANIMATION_PATTERN_DOCUMENT Candidate = m_KoukuSaydonAnimationPatterns;
		KOUKU_SAYDON_ANIMATION_PATTERN& Pattern =
			Candidate.Patterns[iSelectedPattern];
		const int32_t iOther = bMoveClipUp ?
			iSelectedClip - 1 : iSelectedClip + 1;
		std::swap(Pattern.Clips[iSelectedClip], Pattern.Clips[iOther]);
		if (CommitCandidate(
				std::move(Candidate),
				"Moved the selected clip occurrence without changing its stable ID."))
		{
			m_iSelectedKoukuSaydonPatternClip = iOther;
		}
	}

	if (!m_strKoukuSaydonPatternStatus.empty())
		ImGui::TextWrapped("%s", m_strKoukuSaydonPatternStatus.c_str());
}
