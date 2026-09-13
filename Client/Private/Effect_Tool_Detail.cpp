#include "imgui.h"
#include "Effect_Tool_Internal.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentRenderer.h"
#include "EffectResourceCatalog.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Object.h"
#include "Effect_Playback.h"
#include "Effect_RuntimeAuthority.h"
#include "Effect_ThumbnailCache.h"
#include "Effect_VisualProgramCorpus.h"
#include "GameInstance.h"
#include "Logic_DimensionMaster.h"
#include "Logic_LanceMaster.h"
#include "MapEffectPresentationRuntime.h"
#include "Model.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <set>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "Transform.h"
#include "EffectAuthoringSequencer.h"

void Client::CEffect_Tool::Render_AnimationControls(
    const shared_ptr<Engine::CModel>& pModel)
{
    ImGui::SeparatorText("Animation");
    if (nullptr == pModel || 0u == pModel->Get_NumAnimations())
    {
        ImGui::TextDisabled("Select a Character model with animations.");
        return;
    }
    Refresh_AnimationClipLabels(pModel, false);
    ImGui::InputTextWithHint("##AnimationClipFilter",
        "Search clip or Valtan action...",
        m_AnimationClipFilter.data(), m_AnimationClipFilter.size());
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear##AnimationClipFilter"))
        m_AnimationClipFilter[0u] = '\0';
    uint32_t iCurrent = pModel->Get_CurrentAnimIndex();
    const char* pCurrentName = pModel->Get_AnimationName(iCurrent);
    const char* pCurrentLabel =
        iCurrent < m_AnimationClipDisplayLabels.size() ?
            m_AnimationClipDisplayLabels[iCurrent].c_str() : pCurrentName;
    if (ImGui::BeginCombo("Animation Clip",
        nullptr != pCurrentLabel ? pCurrentLabel : "Invalid"))
    {
        size_t iVisibleClipCount = 0u;
        for (uint32_t iAnimation = 0u;
            iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
        {
            const char* pName = pModel->Get_AnimationName(iAnimation);
            const char* pLabel =
                iAnimation < m_AnimationClipDisplayLabels.size() ?
                    m_AnimationClipDisplayLabels[iAnimation].c_str() : pName;
            const std::string_view Filter = m_AnimationClipFilter.data();
            const bool_t bMatches = nullptr != pName && nullptr != pLabel &&
                (Contains_NoCase(pLabel, Filter) ||
                 Contains_NoCase(pName, Filter) ||
                 (iAnimation < m_AnimationClipSearchTokens.size() &&
                  Contains_NoCase(
                    m_AnimationClipSearchTokens[iAnimation], Filter)));
            if (!bMatches)
                continue;
            ++iVisibleClipCount;
            if (nullptr != pName && nullptr != pLabel && ImGui::Selectable(
                pLabel, iAnimation == iCurrent))
            {
				Reset_BufferedComboAudition();
                Reset_SynchronizedAnimationSequence();
                pModel->Start_Animation(iAnimation, true);
                pModel->Set_AnimPaused(false);
                iCurrent = iAnimation;
            }
        }
        if (0u == iVisibleClipCount)
            ImGui::TextDisabled("No animation matches this search.");
        ImGui::EndCombo();
    }
    if (ImGui::Button("Reload Labels"))
        Refresh_AnimationClipLabels(pModel, true);
    ImGui::SameLine();
    ImGui::TextDisabled(
        CAnimationTargetService::Resolve_AssetName() == VALTAN_ANIMATION_ASSET_NAME ?
            "[Valtan] Pattern Action | Model Clip" :
            "[Input] Korean Skill Name | Model Clip");
    if (!m_strAnimationClipLabelStatus.empty())
        ImGui::TextDisabled("%s", m_strAnimationClipLabelStatus.c_str());
    const bool_t bSkillTimelineOwnsAnimation =
        !m_SynchronizedAnimationClips.empty() &&
        m_iSynchronizedAnimationTargetGeneration ==
            CAnimationTargetService::Resolve_TargetGeneration();
    if (bSkillTimelineOwnsAnimation)
    {
        ImGui::TextDisabled(
            "Bound Effect Timeline owns animation Play/Pause/Restart/Sample Time.");
    }
    ImGui::BeginDisabled(bSkillTimelineOwnsAnimation);
    if (ImGui::Button(pModel->Is_AnimPaused() ?
        "Play Animation" : "Pause Animation"))
    {
        pModel->Set_AnimPaused(!pModel->Is_AnimPaused());
    }
    ImGui::SameLine();
    if (ImGui::Button("Restart Animation"))
        pModel->Set_AnimTrackPosition(iCurrent, 0.f);
    f32_t fPosition = 0.f;
    f32_t fDuration = 0.f;
    if (pModel->Get_AnimationProgress(iCurrent, fPosition, fDuration) &&
        fDuration > 0.f && ImGui::SliderFloat(
            "Animation Frame", &fPosition, 0.f, fDuration, "%.2f"))
    {
        pModel->Set_AnimPaused(true);
        pModel->Set_AnimTrackPosition(iCurrent, fPosition);
    }
    ImGui::EndDisabled();
}

void Client::CEffect_Tool::Update_Picking()
{
    if (!m_bPendingWorldPivotPick ||
        !ImGui::IsWindowHovered() ||
        ImGui::IsAnyItemHovered() ||
        !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        return;
    float4_t Picked{};
    if (!CGameInstance::Get().Picking(Picked))
    {
        m_strPreviewStatus = "No world surface was hit.";
        return;
    }
    m_vPickedWorldPosition = { Picked.x, Picked.y, Picked.z };
    XMStoreFloat4x4(&m_PreviewWorldRoot, XMMatrixTranslation(
        Picked.x, Picked.y, Picked.z));
    m_ePreviewPivotKind = EFFECT_PREVIEW_PIVOT_KIND::WORLD;
    m_bPendingWorldPivotPick = false;
    m_strPreviewStatus = "World pivot committed from CGameInstance::Picking.";
}

void Client::CEffect_Tool::Render_SelectionPath() const
{
	if (!m_ActiveDocument.has_value())
		return;
	ImGui::SeparatorText("Selection");
	const char* pSelectionLevel = "None";
	switch (m_eDetailSelection)
	{
	case EFFECT_DETAIL_SELECTION::SKILL: pSelectionLevel = "Skill"; break;
	case EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM:
		pSelectionLevel = "Particle System"; break;
	case EFFECT_DETAIL_SELECTION::COMPONENT: pSelectionLevel = "Component"; break;
	case EFFECT_DETAIL_SELECTION::EMITTER: pSelectionLevel = "Emitter"; break;
	case EFFECT_DETAIL_SELECTION::SOURCE_MODULE: pSelectionLevel = "Module"; break;
	case EFFECT_DETAIL_SELECTION::ELEMENT: pSelectionLevel = "Element"; break;
	case EFFECT_DETAIL_SELECTION::MODEL_CUE: pSelectionLevel = "Model / Summon"; break;
	case EFFECT_DETAIL_SELECTION::NONE:
	case EFFECT_DETAIL_SELECTION::END:
	default: break;
	}
	ImGui::Text("Level: %s", pSelectionLevel);
	ImGui::TextWrapped("Skill / Document: %s",
		m_ActiveDocument->strEffectAssetId.c_str());
	if (m_ProductPreview.has_value())
	{
		const PLAYER_SKILL_DEFINITION* pSkill =
			CPlayerSkillCatalog::Find_ById(m_ProductPreview->iSkillId);
		if (nullptr != pSkill &&
			pSkill->eCharacterClass == m_ProductPreview->eCharacterClass)
		{
			const EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE& ProductCue =
				m_ProductPreview->ProductCue;
			ImGui::TextWrapped(
				"Product Skill: #%u | Input %s | Required Stance %s",
				static_cast<uint32_t>(pSkill->iSkillId),
				pSkill->strInputSlot.c_str(),
				Tool_PlayerStanceLabel(pSkill->eRequiredStance));
			ImGui::TextWrapped(
				"Product Cue: Stage %zu / Clip %zu | %s @ %u ms | %s",
				ProductCue.iStageIndex + 1u,
				ProductCue.iStageClipIndex + 1u,
				ProductCue.Cue.strClipName.c_str(),
				ProductCue.Cue.iStartMs,
				ProductCue.Cue.strEffectAssetId.c_str());
			ImGui::TextDisabled(
				"Apply / Save replaces only this Product target at the current "
				"catalog revision. An active occurrence stays immutable; Restart "
				"or the next cast consumes it.");
			if (m_bBufferedComboAuditionActive &&
				m_eBufferedComboAuditionClass == pSkill->eCharacterClass &&
				m_iBufferedComboAuditionSkillId == pSkill->iSkillId)
			{
				ImGui::TextColored(ImVec4(0.36f, 0.72f, 1.f, 1.f),
					"Animation Mode: Buffered Combo Audition | %.3f s",
					m_fBufferedComboAuditionDurationSeconds);
				ImGui::TextDisabled(
					"The selected Product Effect stays occurrence-local; Play Full Effect returns to exact stage playback.");
			}
		}
	}
	if (!m_strSelectedComponentId.empty())
		ImGui::TextWrapped("Component: %s", m_strSelectedComponentId.c_str());
	if (!m_strSelectedEmitterId.empty())
		ImGui::TextWrapped("Emitter: %s", m_strSelectedEmitterId.c_str());
	if (!m_strSelectedSourceModuleId.empty())
		ImGui::TextWrapped("Module: %s", m_strSelectedSourceModuleId.c_str());
	if (EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection &&
		!m_strSelectedElementId.empty())
	{
		if (!m_strSelectedElementGroupId.empty())
			ImGui::TextWrapped("Group: %s",
				m_strSelectedElementGroupId.c_str());
		ImGui::TextWrapped("Element: %s", m_strSelectedElementId.c_str());
	}
	if (EFFECT_DETAIL_SELECTION::MODEL_CUE == m_eDetailSelection &&
		!m_strSelectedModelCueId.empty())
	{
		ImGui::TextWrapped("Model Cue: %s", m_strSelectedModelCueId.c_str());
	}
}

void Client::CEffect_Tool::Render_ModelCueDetail()
{
	const EFFECT_MODEL_CUE_DESC* pCurrent = Find_SelectedModelCue();
	if (nullptr == pCurrent)
	{
		ImGui::TextDisabled("Select one row under Current Effect > Model / Summon.");
		return;
	}
	if (!m_ModelCueDraft.has_value() ||
		m_ModelCueDraft->strCueId != pCurrent->strCueId)
	{
		m_ModelCueDraft = *pCurrent;
		Copy_Buffer(m_ModelCueAssetIdDraft.data(),
			m_ModelCueAssetIdDraft.size(), pCurrent->strModelAssetId);
		Copy_Buffer(m_ModelCueClipNameDraft.data(),
			m_ModelCueClipNameDraft.size(), pCurrent->strClipName);
		m_bModelCueDraftDirty = false;
		m_strDetailStatus.clear();
	}

	EFFECT_MODEL_CUE_DESC& Draft = *m_ModelCueDraft;
	bool_t bChanged = ImGui::Checkbox("Visible", &Draft.bVisible);
	if (ImGui::InputText("WModel", m_ModelCueAssetIdDraft.data(),
		m_ModelCueAssetIdDraft.size()))
	{
		Draft.strModelAssetId = m_ModelCueAssetIdDraft.data();
		bChanged = true;
	}
	if (ImGui::InputText("Animation Clip", m_ModelCueClipNameDraft.data(),
		m_ModelCueClipNameDraft.size()))
	{
		Draft.strClipName = m_ModelCueClipNameDraft.data();
		bChanged = true;
	}
	bChanged |= ImGui::DragFloat("Start Delay (Seconds)",
		&Draft.fStartDelaySeconds, 0.01f, 0.f, 30.f, "%.3f");
	bChanged |= ImGui::DragFloat("Duration (Seconds)",
		&Draft.fDurationSeconds, 0.01f, 0.001f, 30.f, "%.3f");
	if (ImGui::Checkbox("Loop Animation", &Draft.bLoop))
	{
		if (Draft.bLoop) Draft.bHoldLastFrame = false;
		bChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Hold Last Frame", &Draft.bHoldLastFrame))
	{
		if (Draft.bHoldLastFrame) Draft.bLoop = false;
		bChanged = true;
	}
	ImGui::TextDisabled("Owner axes: +X right, +Y up, +Z forward. Local position is in meters.");
	bChanged |= ImGui::DragFloat3("Local Position",
		&Draft.LocalTransform.vPosition.x, 0.01f);
	bChanged |= ImGui::DragFloat3("Local Rotation (Degrees)",
		&Draft.LocalTransform.vRotationDegrees.x, 0.25f);
	bChanged |= ImGui::DragFloat3("Local Scale",
		&Draft.LocalTransform.vScale.x, 0.01f, 0.0001f, 100.f);
	bChanged |= ImGui::DragFloat3("Local Velocity Per Second",
		&Draft.LocalTransform.vVelocityPerSecond.x, 0.01f);
	bChanged |= ImGui::DragFloat3("Local Rotation Per Second",
		&Draft.LocalTransform.vRevolutionDegreesPerSecond.x, 0.25f);
	if (ImGui::CollapsingHeader("Asset Pre-Transform"))
	{
		bChanged |= ImGui::DragFloat3("Asset Pre-Scale",
			&Draft.vAssetPreScale.x, 0.0001f, 0.000001f, 100.f, "%.6f");
		bChanged |= ImGui::DragFloat3("Asset Pre-Rotation",
			&Draft.vAssetPreRotationDegrees.x, 0.25f);
	}
	if (bChanged)
	{
		m_bModelCueDraftDirty = true;
		m_strDetailStatus =
			"Live Model / Summon preview; Apply commits this draft to memory.";
		Stage_ModelCueDraftPreview();
	}

	ImGui::Separator();
	ImGui::BeginDisabled(!m_bModelCueDraftDirty);
	if (ImGui::Button("Use Model Cue"))
	{
		EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
		if (!Apply_ModelCueDraft(Staged))
		{
			m_strDetailStatus =
				"Apply failed; the selected Model Cue is no longer present.";
		}
		else if (Try_CommitDocument(std::move(Staged)))
		{
			m_bModelCueDraftDirty = false;
			if (const EFFECT_MODEL_CUE_DESC* pCommitted = Find_SelectedModelCue())
				m_ModelCueDraft = *pCommitted;
			m_strDetailStatus =
				"Applied Model / Summon to active Document memory; Save required.";
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Revert Model Cue"))
	{
		m_ModelCueDraft = *pCurrent;
		Copy_Buffer(m_ModelCueAssetIdDraft.data(),
			m_ModelCueAssetIdDraft.size(), pCurrent->strModelAssetId);
		Copy_Buffer(m_ModelCueClipNameDraft.data(),
			m_ModelCueClipNameDraft.size(), pCurrent->strClipName);
		m_bModelCueDraftDirty = false;
		Stage_WorldPreview(*m_ActiveDocument);
		m_strDetailStatus = "Reverted the Model Cue draft.";
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Audition Summon"))
		Try_SoloModelCue(m_ActiveDocument->strEffectAssetId, Draft.strCueId);
	if (!m_strDetailStatus.empty())
		ImGui::TextWrapped("%s", m_strDetailStatus.c_str());
}

void Client::CEffect_Tool::Render_SkillSelectionDetail()
{
	if (!m_ActiveDocument.has_value())
		return;
	const std::shared_ptr<const EFFECT_ASSEMBLY_DESC> Assembly =
		CEffectCatalog::Find_Assembly(m_ActiveDocument->strEffectAssetId);
	const PARTICLE_LAYER_SUMMARY Summary =
		Summarize_ParticleLayers(*m_ActiveDocument);
	size_t iEmitterCount = 0u;
	size_t iModuleCount = 0u;
	size_t iResourceCount = 0u;
	if (nullptr != Assembly)
	{
		for (const EFFECT_COMPONENT_CUE_DESC& Cue : Assembly->ComponentCues)
		{
			const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
				CEffectCatalog::Find_Component(Cue.strComponentAssetId);
			if (nullptr == Component)
				continue;
			iEmitterCount += Component->Emitters.size();
			for (const EFFECT_ELEMENT_DESC& Element : Component->Document.Elements)
			{
				iModuleCount += Element.SourceRecipe.Modules.size();
				iResourceCount += Element.ResourceBindings.size();
			}
		}
	}
	ImGui::Text("Skill Effect: %s",
		m_ActiveDocument->strDisplayName.c_str());
	ImGui::TextDisabled("Stable ID: %s",
		m_ActiveDocument->strEffectAssetId.c_str());
	ImGui::TextDisabled(
		"Timeline Cues %zu | Components %zu | Emitters %zu | Elements %zu",
		nullptr == Assembly ? 0u : Assembly->ComponentCues.size(),
		nullptr == Assembly ? 0u : Assembly->ComponentCues.size(),
		iEmitterCount, m_ActiveDocument->Elements.size());
	ImGui::TextDisabled(
		"Particle Source Systems %zu | Layers %zu | Resources %zu | Modules %zu",
		Summary.iSourceSystemCount, Summary.iLayerCount,
		iResourceCount, iModuleCount);
	ImGui::TextWrapped(
		"This is the complete Skill Effect. Select Particle System for aggregate "
		"multipliers, a Component for one source cue, an Emitter for renderer and "
		"particle controls, or a Module for the original Cascade values.");
	if (Summary.iLayerCount > 0u &&
		ImGui::Button("Select Particle System"))
	{
		Try_SelectParticleSystem(m_ActiveDocument->strEffectAssetId);
		return;
	}
	if (Summary.iLayerCount > 0u)
		ImGui::SameLine();
	ImGui::BeginDisabled(0u == iEmitterCount);
	if (ImGui::Button("Open First Emitter"))
		Try_SelectFirstEmitter(m_ActiveDocument->strEffectAssetId, {});
	ImGui::EndDisabled();
}

void Client::CEffect_Tool::Render_ComponentSelectionDetail()
{
	const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
		CEffectCatalog::Find_Component(m_strSelectedComponentId);
	if (nullptr == Component)
	{
		ImGui::TextDisabled("Selected Component is no longer admitted.");
		return;
	}
	size_t iResourceCount = 0u;
	size_t iModuleCount = 0u;
	for (const EFFECT_ELEMENT_DESC& Element : Component->Document.Elements)
	{
		iResourceCount += Element.ResourceBindings.size();
		iModuleCount += Element.SourceRecipe.Modules.size();
	}
	ImGui::Text("Component: %s", Component->strDisplayName.c_str());
	ImGui::TextDisabled("Stable ID: %s",
		Component->strComponentAssetId.c_str());
	ImGui::TextDisabled("Type: %s | Emitters %zu | Resources %zu | Modules %zu",
		Component->strComponentType.c_str(), Component->Emitters.size(),
		iResourceCount, iModuleCount);
	ImGui::TextDisabled("Source Group: %s",
		Component->strSourceGroupId.c_str());
	if (ImGui::TreeNode("Source Nodes"))
	{
		for (const std::string& Node : Component->SourceNodes)
			ImGui::BulletText("%s", Node.c_str());
		ImGui::TreePop();
	}
	ImGui::TextWrapped(
		"A Component owns the original Emitters in source order. "
		"Select an Emitter to edit its Renderer, Resource Set, and Module Stack.");
	ImGui::BeginDisabled(Component->Emitters.empty());
	if (ImGui::Button("Open First Emitter"))
	{
		Try_SelectFirstEmitter(m_ActiveDocument->strEffectAssetId,
			Component->strComponentAssetId);
		ImGui::EndDisabled();
		return;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Audition Component"))
	{
		EFFECT_DOCUMENT_DESC Preview = Component->Document;
		f32_t fCueOffset = 0.f;
		if (m_ActiveDocument.has_value())
		{
			const std::shared_ptr<const EFFECT_ASSEMBLY_DESC> Assembly =
				CEffectCatalog::Find_Assembly(
					m_ActiveDocument->strEffectAssetId);
			if (nullptr != Assembly)
			{
				const auto Cue = std::find_if(Assembly->ComponentCues.begin(),
					Assembly->ComponentCues.end(),
					[this](const EFFECT_COMPONENT_CUE_DESC& Value)
					{
						return Value.strComponentAssetId ==
							m_strSelectedComponentId;
					});
				if (Assembly->ComponentCues.end() != Cue)
					fCueOffset = Cue->fStartDelaySeconds;
			}
		}
		for (EFFECT_ELEMENT_DESC& Element : Preview.Elements)
			Element.Detail.Timing.fStartDelaySeconds += fCueOffset;
		Recalculate_PreviewDuration(Preview);
		if (Stage_WorldPreview(Preview))
		{
			m_fPreviewTimeSeconds = fCueOffset;
			m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
			m_strPreviewStatus = "Selected Component audition staged at its Assembly cue.";
		}
	}
}

void Client::CEffect_Tool::Render_EmitterSelectionDetail()
{
	const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
		CEffectCatalog::Find_Component(m_strSelectedComponentId);
	if (nullptr == Component)
		return;
	const auto Emitter = std::find_if(Component->Emitters.begin(),
		Component->Emitters.end(), [this](const EFFECT_COMPONENT_EMITTER_DESC& Value)
		{
			return Value.strEmitterId == m_strSelectedEmitterId;
		});
	if (Component->Emitters.end() == Emitter)
		return;
	ImGui::Text("Emitter: %s", Emitter->strEmitterId.c_str());
	ImGui::TextDisabled("Component: %s | Renderer: %s",
		Component->strComponentAssetId.c_str(),
		Emitter->strRendererType.c_str());
	ImGui::TextDisabled("Source order %u | Resources %u | Modules %u",
		Emitter->iSourceElementIndex, Emitter->iResourceBindingCount,
		Emitter->iModuleCount);
	if (const EFFECT_ELEMENT_DESC* pElement = Find_SelectedElement())
	{
		ImGui::TextWrapped("Source Material: %s",
			pElement->Material.strSourceMaterialPath.empty() ? "(none)" :
				pElement->Material.strSourceMaterialPath.c_str());
		ImGui::TextDisabled("Template %s | Render %s",
			pElement->Material.strTemplateId.c_str(),
			Profile_Label(pElement->Material.eRenderProfile));
	}
	ImGui::TextWrapped(
		"Emitter controls below edit the selected renderer instance. Select one "
		"Module in All Effects to focus its original source-class values.");
	ImGui::SeparatorText("Typed Emitter Detail");
}

void Client::CEffect_Tool::Render_SourceModuleSelectionDetail()
{
	const EFFECT_ELEMENT_DESC* pCurrent = Find_SelectedElement();
	if (nullptr == pCurrent)
	{
		ImGui::TextDisabled("Selected Module has no active Emitter Element.");
		return;
	}
	if (!m_DetailDraft.has_value() ||
		m_strDetailDraftElementId != pCurrent->strElementId)
	{
		m_DetailDraft = *pCurrent;
		m_strDetailDraftElementId = pCurrent->strElementId;
		Refresh_DetailDraftAdmission(*pCurrent);
		m_bDetailDraftDirty = false;
		m_bDetailDraftPreviewPending = false;
		m_bDetailDraftPreviewRestartRequested = false;
	}
	const auto Module = std::find_if(
		m_DetailDraft->SourceRecipe.Modules.begin(),
		m_DetailDraft->SourceRecipe.Modules.end(),
		[this](const EFFECT_SOURCE_MODULE_DESC& Value)
		{
			return Value.strStableId == m_strSelectedSourceModuleId;
		});
	if (m_DetailDraft->SourceRecipe.Modules.end() == Module)
	{
		ImGui::TextDisabled("Selected source Module is no longer present.");
		return;
	}
	ImGui::Text("Module: %s", Module->strClassName.c_str());
	ImGui::TextDisabled("Stable ID: %s", Module->strStableId.c_str());
	ImGui::TextDisabled("Emitter: %s", m_strSelectedEmitterId.c_str());
	if (m_bDetailDraftPortableRecipeReadOnly)
	{
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
			"Compiler-owned portable SourceRecipe. Module values are read-only in the Tool; refresh them through the source compiler/reimport path.");
	}
	bool_t bChanged = false;
	ImGui::BeginDisabled(m_bDetailDraftPortableRecipeReadOnly);
	Render_SourceModuleDetail(*Module, bChanged, true);
	ImGui::EndDisabled();
	if (bChanged)
	{
		m_bDetailDraftDirty = true;
		m_strDetailStatus =
			"Live source Module preview; Apply commits it to active Document memory.";
		m_bDetailDraftPreviewPending = true;
		m_fDetailDraftPreviewDueSeconds = ImGui::GetTime() + 0.060;
	}
	if (m_bDetailDraftPreviewPending &&
		(!ImGui::IsAnyItemActive() ||
		 ImGui::GetTime() >= m_fDetailDraftPreviewDueSeconds))
	{
		m_bDetailDraftPreviewPending = false;
		Stage_DetailDraftPreview();
	}
	ImGui::Separator();
	ImGui::BeginDisabled(
		m_bDetailDraftPortableRecipeReadOnly || !m_bDetailDraftDirty);
	if (ImGui::Button("Use Module"))
	{
		EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
		if (Apply_DetailDraft(Staged) && Try_CommitDocument(std::move(Staged)))
		{
			m_bDetailDraftDirty = false;
			m_bDetailDraftPreviewPending = false;
			m_bDetailDraftPreviewRestartRequested = false;
			if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
			{
				m_DetailDraft = *pCommitted;
				Refresh_DetailDraftAdmission(*pCommitted);
			}
			m_strDetailStatus =
				"Applied source Module to active Document memory; Save required.";
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Revert Module"))
	{
		m_DetailDraft = *pCurrent;
		Refresh_DetailDraftAdmission(*pCurrent);
		m_bDetailDraftDirty = false;
		m_bDetailDraftPreviewPending = false;
		m_bDetailDraftPreviewRestartRequested = false;
		Stage_WorldPreview();
		m_strDetailStatus = "Reverted source Module draft.";
	}
	ImGui::EndDisabled();
	if (!m_strDetailStatus.empty())
		ImGui::TextWrapped("%s", m_strDetailStatus.c_str());
}

void Client::CEffect_Tool::Render_AuthoringSessionBar()
{
	if (!m_ActiveDocument.has_value())
		return;
	const bool_t bEditableSource =
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource ||
		EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT == m_eActiveDocumentSource;
	const bool_t bMigrationReference =
		EFFECT_DOCUMENT_SOURCE::MIGRATION_REFERENCE ==
			m_eActiveDocumentSource;
	const bool_t bVisualProgramCopy =
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource;
	const bool_t bAdapterPacketVisualCopy = bVisualProgramCopy &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	const bool_t bDrawable = m_bActiveDocumentDrawable;
	const bool_t bLivePreview = bDrawable &&
		m_bPreviewVisibleRequested &&
		nullptr != m_pWorldPreviewObject.lock();
	ImGui::SeparatorText("Editing Session");
	ImGui::Text("Source: %s | Draft: %s | Document: %s | Preview: %s",
		Source_Label(m_eActiveDocumentSource),
		Has_UnappliedDetailDraft() ? "UNAPPLIED" : "committed",
		m_bDocumentDirty ? "UNSAVED" : "saved",
		bLivePreview ? "LIVE" : "HIDDEN");
	ImGui::BeginDisabled(!bEditableSource || !Has_UnsavedWork());
	if (ImGui::Button("Save"))
		Try_ApplyDraftAndSave();
	ImGui::EndDisabled();
	ImGui::SameLine();
	const bool_t bCanReloadSaved =
		(EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource ||
		 bMigrationReference) &&
		!m_ActiveDocumentPath.empty();
	ImGui::BeginDisabled(!bCanReloadSaved);
	if (ImGui::Button(Has_UnsavedWork() ?
		"Load Saved..." : "Load Saved"))
	{
		if (Has_UnsavedWork())
		{
			Try_LoadDocumentPath(m_ActiveDocumentPath,
				m_eActiveDocumentSource,
				m_ActiveDocument->strEffectAssetId);
		}
		else
			Try_ReloadActiveDocument();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	const bool_t bHasActiveDocumentDraft = m_bDocumentDirty ||
		m_bParticleSystemDraftDirty || m_bDetailDraftDirty ||
		m_bModelCueDraftDirty;
	const bool_t bDiscardBlockedByAnotherOwner =
		m_bOccurrenceTuningDirty || m_bOccurrenceTransformDraftDirty ||
		m_bValtanAreaMapEffectDirty ||
		m_UnpublishedStaticAreaWorldDraft.has_value();
	const bool_t bCanDiscardActiveDocumentDraft =
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource &&
		!m_ActiveDocumentPath.empty() && bHasActiveDocumentDraft &&
		!bDiscardBlockedByAnotherOwner;
	ImGui::BeginDisabled(!bCanDiscardActiveDocumentDraft);
	if (ImGui::Button("Discard Changes..."))
		m_bDiscardActiveDocumentDraftConfirmationRequested = true;
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(
			bDiscardBlockedByAnotherOwner ?
				"Save or discard occurrence/Area work through its typed owner first; this command discards only Current Effect changes." :
				"Reload the exact saved authored Effect and discard only Current Effect memory/detail changes. The file is not deleted.");
	}
	ImGui::SameLine();
	if (ImGui::Button("Restart Preview"))
		Start_WorldPreviewFromBeginning();
	if (m_bDiscardActiveDocumentDraftConfirmationRequested)
	{
		ImGui::OpenPopup("Discard Current Effect changes?");
		m_bDiscardActiveDocumentDraftConfirmationRequested = false;
	}
	if (ImGui::BeginPopupModal(
			"Discard Current Effect changes?", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(
			"Reload the exact saved Effect and discard only its in-memory Document/Detail changes?");
		ImGui::TextDisabled(
			"The authored file, Area draft, and occurrence tuning are not deleted or rewritten.");
		if (ImGui::Button("Discard Changes"))
		{
			if (Try_ReloadActiveDocument(true))
				ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("Last document operation: %s", m_strDocumentStatus.c_str());
	if (!bEditableSource)
	{
		if (bMigrationReference)
		{
			ImGui::TextWrapped(
				"This Legacy/Rollback document is an immutable migration reference. In-memory inspection and preview are allowed, but Save Changes cannot overwrite the original file.");
			ImGui::InputText("Migrated Authored Copy ID", m_NewAssetId.data(),
				m_NewAssetId.size());
			ImGui::BeginDisabled(Has_UnappliedDetailDraft());
			if (ImGui::Button("Save As New Authored Effect"))
				Try_SaveDocumentAs(m_NewAssetId.data());
			ImGui::EndDisabled();
		}
		else if (bVisualProgramCopy)
		{
			if (bAdapterPacketVisualCopy)
			{
				ImGui::TextWrapped(
					"This exact adapter projection is immutable. Full Details are available for inspection and temporary preview only; ordinary Save As would discard its projector/VF/resource packet and is blocked.");
				ImGui::TextDisabled(
					"Persistent position / rotation / scale: use Stable occurrences and the PROJECT_TUNED Transform Save / Reload controls.");
				ImGui::BeginDisabled(true);
				ImGui::Button("Direct Save As (adapter packet unsupported)");
				ImGui::EndDisabled();
				ImGui::InputText("Generic Starting Copy ID", m_NewAssetId.data(),
					m_NewAssetId.size());
				ImGui::BeginDisabled(Has_UnappliedDetailDraft());
				if (ImGui::Button(
					"Save Selected As Generic Authored Starting Copy"))
				{
					Try_SaveSelectedAdapterElementAsGenericAuthoredCopy(
						m_NewAssetId.data());
				}
				ImGui::EndDisabled();
				ImGui::TextDisabled(
					"The generic copy keeps this Element's Detail, Material, and resource bindings, but intentionally does not keep the exact adapter packet.");
			}
			else
			{
				ImGui::TextWrapped(
					"The extracted Visual Program is immutable. This in-memory copy exposes full Element Details and Resource Library editing; persist it only as a new Authored Effect.");
				ImGui::InputText("Authored Copy ID", m_NewAssetId.data(),
					m_NewAssetId.size());
				ImGui::BeginDisabled(Has_UnappliedDetailDraft());
				if (ImGui::Button("Save As Authored Copy"))
					Try_SaveDocumentAs(m_NewAssetId.data());
				ImGui::EndDisabled();
			}
		}
		else
		{
			ImGui::TextDisabled(
				"Imported/runtime views use Promote or Save As before authoring save.");
		}
	}
	else if (!bDrawable)
	{
		ImGui::TextWrapped(
			"Structurally valid unbound draft. Preview hidden; a gameplay-bound source must be drawable before Save: %s",
			m_strActiveDocumentDrawableError.c_str());
	}
	else if (!m_bDocumentDirty)
	{
		ImGui::TextDisabled(
			"The saved file is applied to this local preview. Server Replay updates after Server data refresh and Valtan re-entry.");
	}
	ImGui::Separator();
}

bool_t Client::CEffect_Tool::Try_SetDocumentBloomIntensity(const f32_t value)
{
    if (!m_ActiveDocument || m_ActiveDocument->bSourceContract ||
        EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource ||
        !Is_ValidEffectBloomIntensity(value))
    { m_strDocumentStatus = "Select an authored Effect and use Bloom Intensity from 0 to 16."; return false; }
    const auto& assetId = m_ActiveDocument->strEffectAssetId;
    const EFFECT_RESOURCE_KEY key{EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, assetId};
    std::string error;
    if (m_pAuthoringSequencer && !m_pAuthoringSequencer->Set_BloomIntensity(key, value, error))
    { m_strDocumentStatus = error; return false; }
    if (m_WorldPreviewDocument && m_WorldPreviewDocument->strEffectAssetId == assetId)
    {
        if (auto object = m_pWorldPreviewObject.lock(); object && !object->Set_BloomIntensity(value, error))
        { m_strDocumentStatus = error; return false; }
        m_WorldPreviewDocument->fBloomIntensity = value;
    }
    if (m_SourcePreviewDocument && m_SourcePreviewDocument->strEffectAssetId == assetId)
        m_SourcePreviewDocument->fBloomIntensity = value;
    m_ActiveDocument->fBloomIntensity = value;
    m_bDocumentDirty = true;
    m_bActiveDocumentMatchesRuntime = false;
    m_strDocumentStatus = "Skill Bloom Intensity applied to this Effect's running previews. Save Changes stores this document.";
    return true;
}

void Client::CEffect_Tool::Render_EffectDetailWindow()
{
    ImGui::SetNextWindowPos(ImVec2(1545.f, 35.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430.f, 660.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Effect Detail"))
    {
        ImGui::End();
        return;
    }
	if (EFFECT_DETAIL_SELECTION::RUNTIME_OCCURRENCE == m_eDetailSelection)
	{
		Render_RuntimeOccurrenceDetail();
		ImGui::End();
		return;
	}
    if (!m_ActiveDocument.has_value())
    {
        Reset_ParticleSystemDraft();
        Reset_DetailDraft();
		Reset_ModelCueDraft();
        m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
        ImGui::TextDisabled(
			"Load an authored Effect, then select one Element under Current Effect.");
        ImGui::End();
        return;
    }
	Render_AuthoringSessionBar();
    float bloomIntensity = m_ActiveDocument->fBloomIntensity;
    ImGui::BeginDisabled(EFFECT_DOCUMENT_SOURCE::AUTHORED != m_eActiveDocumentSource || m_ActiveDocument->bSourceContract);
    if (ImGui::SliderFloat("Skill Bloom Intensity", &bloomIntensity, 0.f, 16.f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
        (void)Try_SetDocumentBloomIntensity(bloomIntensity);
    ImGui::EndDisabled();
    ImGui::TextDisabled("Whole Effect: %s", m_ActiveDocument->strDisplayName.c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("0 removes this Effect's bloom contribution; 1.3 is the default. HDR color and other skills keep their own values.");
	Render_SelectionPath();
	if (EFFECT_DETAIL_SELECTION::SKILL == m_eDetailSelection)
	{
		Render_SkillSelectionDetail();
		ImGui::End();
		return;
	}
    if (EFFECT_DETAIL_SELECTION::PARTICLE_SYSTEM == m_eDetailSelection)
    {
        Render_ParticleSystemDetail();
        ImGui::End();
        return;
    }
	if (EFFECT_DETAIL_SELECTION::COMPONENT == m_eDetailSelection)
	{
		Render_ComponentSelectionDetail();
		ImGui::End();
		return;
	}
	if (EFFECT_DETAIL_SELECTION::SOURCE_MODULE == m_eDetailSelection)
	{
		Render_SourceModuleSelectionDetail();
		ImGui::End();
		return;
	}
	if (EFFECT_DETAIL_SELECTION::MODEL_CUE == m_eDetailSelection)
	{
		Render_ModelCueDetail();
		ImGui::End();
		return;
	}
    const EFFECT_ELEMENT_DESC* pCurrent = Find_SelectedElement();
	const bool_t bElementOrEmitter =
		EFFECT_DETAIL_SELECTION::ELEMENT == m_eDetailSelection ||
		EFFECT_DETAIL_SELECTION::EMITTER == m_eDetailSelection;
    if (!bElementOrEmitter ||
        nullptr == pCurrent)
    {
        ImGui::TextDisabled(
			"Select one Element under Effect Tool > Current Effect.");
		ImGui::End();
		return;
	}
	const bool_t bAdapterPacketInspection =
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM ==
			m_eActiveDocumentSource &&
		nullptr != m_pSelectedVisualSourceProjection &&
		m_pSelectedVisualSourceProjection->Get_ProjectionKind() ==
			EFFECT_VISUAL_PROGRAM_PROJECTION_KIND::ADAPTER_PACKET_V1;
	if (EFFECT_DETAIL_SELECTION::EMITTER == m_eDetailSelection)
		Render_EmitterSelectionDetail();
    if (!m_DetailDraft.has_value() ||
        m_strDetailDraftElementId != pCurrent->strElementId)
    {
        m_DetailDraft = *pCurrent;
        m_strDetailDraftElementId = pCurrent->strElementId;
		Refresh_DetailDraftAdmission(*pCurrent);
        m_bDetailDraftDirty = false;
		m_bDetailDraftPreviewPending = false;
		m_bDetailDraftPreviewRestartRequested = false;
        m_strDetailStatus.clear();
    }
	ImGui::TextWrapped("Selected Element Solo: %s",
		ElementPreviewAdmissionReason(*pCurrent));
    if (m_pAuthoringSequencer && !m_ProductPreview &&
        (m_ActiveDocument->strEffectAssetId.ends_with(".restore") ||
         Is_SceneAnchoredEffectAssetId(m_ActiveDocument->strEffectAssetId)))
    {
        if (ImGui::Button("Timeline Solo##SelectedElement"))
            (void)Try_PreviewElementTimeline(pCurrent->strElementId);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Play only this Element immediately from its original start time in the Sequencer.");
    }
	if (m_bDetailDraftCapabilityDeferred)
		ImGui::TextWrapped("Portable copy restriction: %s",
			m_strDetailDraftCapabilityReason.c_str());
    const f32_t fElementStart =
        m_DetailDraft->Detail.Timing.fStartDelaySeconds;
	const f32_t fNativeEmitterDelay = m_DetailDraft->SourceRecipe.bEnabled ?
		m_DetailDraft->SourceRecipe.fEmitterDelaySeconds : 0.f;
	ImGui::TextDisabled("Effect local window: %.3f - %.3f s",
		fElementStart + fNativeEmitterDelay,
		Element_PreviewEndSeconds(*m_DetailDraft));
	ImGui::TextDisabled("Emitter start: Timeline %.3f s (Effect %.3f + source %.3f)",
		Resolve_EffectTimelineTime(fElementStart + fNativeEmitterDelay),
		fElementStart, fNativeEmitterDelay);
    bool_t bChanged = false;
	ImGui::TextDisabled(bAdapterPacketInspection ?
		"Exact adapter packet inspection. Persistent transforms use Stable occurrence Save / Reload; create a generic Authored starting copy before material/resource editing." :
		"Drag numeric values for live preview; Apply updates Current Effect memory, and Save Changes writes the whole Effect.");
	ImGui::BeginDisabled(bAdapterPacketInspection);
	const bool_t bPresentationElement =
		m_DetailDraft->eKind == EFFECT_ELEMENT_KIND::LIGHT ||
		m_DetailDraft->eKind == EFFECT_ELEMENT_KIND::SCREEN_POST;
	if (bPresentationElement)
	{
		ImGui::TextDisabled(
			"Presentation carriers have no WModel/DDS material slots. Their typed payload is edited below.");
	}
	else
	{
		Render_ResourceSlots(false);
	}
	Render_Detail(*m_DetailDraft, bChanged);
    if (bChanged)
    {
        m_bDetailDraftDirty = true;
        m_strDetailStatus =
            "Live preview only; Apply Detail commits this draft to memory.";
		m_bDetailDraftPreviewPending = true;
		m_fDetailDraftPreviewDueSeconds = ImGui::GetTime() + 0.060;
    }
	if (m_bDetailDraftPreviewPending &&
		(!ImGui::IsAnyItemActive() ||
		 ImGui::GetTime() >= m_fDetailDraftPreviewDueSeconds))
	{
		m_bDetailDraftPreviewPending = false;
		Stage_DetailDraftPreview();
	}
    ImGui::Separator();
    ImGui::BeginDisabled(!m_bDetailDraftDirty);
	if (ImGui::Button("Set on Current Effect (Unsaved)"))
    {
        EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
        if (!Apply_DetailDraft(Staged))
        {
            m_strDetailStatus =
                "Apply failed; the selected Element is no longer present.";
        }
        else if (Try_CommitDocument(std::move(Staged)))
        {
            m_bDetailDraftDirty = false;
			m_bDetailDraftPreviewPending = false;
			m_bDetailDraftPreviewRestartRequested = false;
			if (const EFFECT_ELEMENT_DESC* pCommitted = Find_SelectedElement())
			{
				m_DetailDraft = *pCommitted;
				Refresh_DetailDraftAdmission(*pCommitted);
			}
            m_strDetailStatus =
                "Applied to active Document memory; Save required to persist.";
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Revert Detail"))
    {
        m_DetailDraft = *pCurrent;
		Refresh_DetailDraftAdmission(*pCurrent);
        m_bDetailDraftDirty = false;
		m_bDetailDraftPreviewPending = false;
		m_bDetailDraftPreviewRestartRequested = false;
		Recalculate_PreviewDuration(*m_ActiveDocument);
		if (Stage_WorldPreview(*m_ActiveDocument))
            m_strPreviewStatus =
                "Detail draft reverted to the active Document preview.";
        m_strDetailStatus =
            "Reverted the Detail draft to the active Document.";
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
	const bool_t bDetailExecutionTarget =
		Is_EffectElementAuthoringExecutionTarget(*m_DetailDraft);
	const bool_t bDetailPreviewLocked =
		m_bDetailDraftCapabilityDeferred || !bDetailExecutionTarget ||
		!m_DetailDraft->bVisible;
	ImGui::BeginDisabled(bDetailPreviewLocked);
    if (ImGui::Button("Audition Selected"))
        Try_AuditionSelectedElement();
	ImGui::EndDisabled();
	if (bDetailPreviewLocked && ImGui::IsItemHovered(
			ImGuiHoveredFlags_AllowWhenDisabled))
	{
		if (m_bDetailDraftCapabilityDeferred)
		{
			ImGui::SetTooltip(
				"Play is locked because this SourceRecipe is outside the supported authoring-preview capability. Editing and Save remain available.");
		}
		else if (!bDetailExecutionTarget)
		{
			ImGui::SetTooltip(
				"Play is locked by hard material/runtime fail-closed admission. Editing and Save remain available.");
		}
		else
		{
			ImGui::SetTooltip(
				"Enable Visible to audition this Element. Runtime availability remains fail-closed; editing and Save remain available.");
		}
	}
    if (m_bDetailDraftDirty)
        ImGui::TextDisabled("Detail changes are local until Save.");
	if (!m_strDetailStatus.empty())
		ImGui::TextWrapped("%s", m_strDetailStatus.c_str());
	ImGui::EndDisabled();
	ImGui::End();
}

void Client::CEffect_Tool::Render_SelectedVisualProgramEvidence() const
{
	if (EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM !=
			m_eActiveDocumentSource ||
		nullptr == m_pSelectedVisualSourceProjection ||
		m_strSelectedRuntimeOccurrenceEffectId.empty() ||
		m_strSelectedRuntimeOccurrenceId.empty() ||
		m_strSelectedRuntimeOccurrenceElementId.empty())
	{
		return;
	}

	const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION& Projection =
		*m_pSelectedVisualSourceProjection;
	const EFFECT_VISUAL_PROGRAM_ROW* pVisualRow =
		Projection.Find_RowByOccurrenceId(
			m_strSelectedRuntimeOccurrenceId);
	const EFFECT_VISUAL_PROGRAM_SUPPLEMENTAL_ELEMENT* pSupplemental =
		Projection.Find_SupplementalElementByOccurrenceId(
			m_strSelectedRuntimeOccurrenceId);
	const bool_t bSelectedTargetMatches =
		m_strSelectedElementId == m_strSelectedRuntimeOccurrenceElementId;
	const bool_t bVisualMatches = bSelectedTargetMatches &&
		nullptr != pVisualRow &&
		Projection.Get_EffectAssetId() ==
			m_strSelectedRuntimeOccurrenceEffectId &&
		pVisualRow->Selector.strEffectAssetId ==
			m_strSelectedRuntimeOccurrenceEffectId &&
		pVisualRow->Selector.strOccurrenceId ==
			m_strSelectedRuntimeOccurrenceId &&
		pVisualRow->strRowSha256 ==
			m_strSelectedRuntimeOccurrenceRowSha256 &&
		pVisualRow->TargetIdentity.has_value() &&
		pVisualRow->TargetIdentity->strTargetElementId ==
			m_strSelectedRuntimeOccurrenceElementId &&
		pVisualRow->SourceIdentity.strSourceRecordId ==
			m_strSelectedRuntimeOccurrenceEmitterPath;
	const bool_t bSupplementalMatches = bSelectedTargetMatches &&
		nullptr != pSupplemental &&
		Projection.Get_EffectAssetId() ==
			m_strSelectedRuntimeOccurrenceEffectId &&
		pSupplemental->Selector.strEffectAssetId ==
			m_strSelectedRuntimeOccurrenceEffectId &&
		pSupplemental->Selector.strOccurrenceId ==
			m_strSelectedRuntimeOccurrenceId &&
		pSupplemental->strRowSha256 ==
			m_strSelectedRuntimeOccurrenceRowSha256 &&
		pSupplemental->TargetIdentity.strTargetElementId ==
			m_strSelectedRuntimeOccurrenceElementId &&
		pSupplemental->strSourceRecordId ==
			m_strSelectedRuntimeOccurrenceEmitterPath;

	ImGui::SeparatorText("Read-only Exact Source Evidence");
	ImGui::TextDisabled(
		"Immutable Visual Program packet data. Standard authored Mesh/Base/Noise/Mask/Emissive/Dissolve slots are edited separately below.");
	if (bVisualMatches == bSupplementalMatches)
	{
		ImGui::TextWrapped(
			"Evidence unavailable: the selected occurrence/target stable identity no longer resolves to exactly one admitted packet row.");
		return;
	}

	using EVIDENCE_FIELDS =
		std::vector<std::pair<std::string, std::string>>;
	const auto AddText = [](EVIDENCE_FIELDS& Fields,
		const std::string_view strLabel, const std::string& strValue)
	{
		if (!strValue.empty())
			Fields.emplace_back(std::string(strLabel), strValue);
	};
	const auto AddUnsigned = [](EVIDENCE_FIELDS& Fields,
		const std::string_view strLabel, const uint64_t iValue)
	{
		if (0u != iValue)
			Fields.emplace_back(
				std::string(strLabel), std::to_string(iValue));
	};
	const auto AddDouble = [](EVIDENCE_FIELDS& Fields,
		const std::string_view strLabel, const double fValue)
	{
		if (0.0 == fValue)
			return;
		std::ostringstream Text;
		Text << fValue;
		Fields.emplace_back(std::string(strLabel), Text.str());
	};
	const auto AddBool = [](EVIDENCE_FIELDS& Fields,
		const std::string_view strLabel, const bool_t bValue)
	{
		Fields.emplace_back(
			std::string(strLabel), bValue ? "true" : "false");
	};
	const auto RenderFields = [](const char_t* pTableId,
		const EVIDENCE_FIELDS& Fields)
	{
		if (Fields.empty() || !ImGui::BeginTable(pTableId, 2,
			ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_SizingStretchProp))
		{
			return;
		}
		ImGui::TableSetupColumn("Typed field",
			ImGuiTableColumnFlags_WidthFixed, 124.f);
		ImGui::TableSetupColumn("Exact value",
			ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();
		for (const auto& [strLabel, strValue] : Fields)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(strLabel.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("%s", strValue.c_str());
		}
		ImGui::EndTable();
	};
	const auto RenderResources = [&](const char_t* pSectionId,
		const std::vector<EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW>& Resources)
	{
		if (Resources.empty())
			return;
		const std::string Header = std::string(pSectionId) + " (" +
			std::to_string(Resources.size()) + ")";
		if (!ImGui::TreeNodeEx(Header.c_str(),
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			return;
		}
		for (size_t iResource = 0u; iResource < Resources.size(); ++iResource)
		{
			const EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW& Resource =
				Resources[iResource];
			EVIDENCE_FIELDS Fields;
			AddText(Fields, "Role", Resource.strRole);
			AddText(Fields, "Slot ID", Resource.strSlotId);
			AddText(Fields, "Asset ID", Resource.strAssetId);
			AddText(Fields, "Resolution", Resource.strResolutionStatus);
			AddText(Fields, "Raw SHA-256", Resource.strRawSha256);
			AddUnsigned(Fields, "Byte count", Resource.iByteCount);
			AddText(Fields, "Shader register", Resource.strShaderRegister);
			AddText(Fields, "Source channel", Resource.strSourceChannel);
			if (Fields.empty())
				continue;
			ImGui::PushID(static_cast<int>(iResource));
			const std::string Label = "Resource " +
				std::to_string(iResource + 1u) +
				(Resource.strRole.empty() ? std::string{} :
					": " + Resource.strRole);
			if (ImGui::TreeNodeEx(Label.c_str(),
				ImGuiTreeNodeFlags_DefaultOpen))
			{
				RenderFields("resource_evidence", Fields);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	};
	const auto RenderLimitations = [&](
		const std::vector<std::string>& Limitations)
	{
		for (const std::string& strLimitation : Limitations)
		{
			if (!strLimitation.empty())
				ImGui::BulletText("%s", strLimitation.c_str());
		}
	};

	EVIDENCE_FIELDS Identity;
	AddText(Identity, "Effect asset ID",
		m_strSelectedRuntimeOccurrenceEffectId);
	AddText(Identity, "Occurrence ID",
		m_strSelectedRuntimeOccurrenceId);
	AddText(Identity, "Target element ID",
		m_strSelectedRuntimeOccurrenceElementId);
	AddText(Identity, "Row SHA-256",
		m_strSelectedRuntimeOccurrenceRowSha256);
	AddText(Identity, "Source record ID",
		m_strSelectedRuntimeOccurrenceEmitterPath);
	if (bVisualMatches)
	{
		AddText(Identity, "Selector SHA-256",
			pVisualRow->Selector.strSelectorSha256);
		AddText(Identity, "Source row ID",
			pVisualRow->SourceIdentity.strSourceRowId);
		AddText(Identity, "Source row SHA-256",
			pVisualRow->SourceIdentity.strSourceRowSha256);
		AddText(Identity, "Source record SHA-256",
			pVisualRow->SourceIdentity.strSourceRecordSha256);
		AddText(Identity, "Source recipe SHA-256",
			pVisualRow->SourceIdentity.strSourceRecipeSha256);
		AddText(Identity, "Module closure SHA-256",
			pVisualRow->SourceIdentity.strModuleClosureSha256);
		AddUnsigned(Identity, "Module count",
			pVisualRow->SourceIdentity.iModuleCount);
		AddText(Identity, "Target record SHA-256",
			pVisualRow->TargetIdentity->strTargetRecordSha256);
		AddText(Identity, "Target payload raw SHA-256",
			pVisualRow->TargetIdentity->strTargetPayloadRawSha256);
	}
	else
	{
		AddText(Identity, "Selector SHA-256",
			pSupplemental->Selector.strSelectorSha256);
		AddText(Identity, "Source record SHA-256",
			pSupplemental->strSourceRecordSha256);
		AddText(Identity, "Source payload raw SHA-256",
			pSupplemental->strSourcePayloadRawSha256);
		AddText(Identity, "Target record SHA-256",
			pSupplemental->TargetIdentity.strTargetRecordSha256);
		AddText(Identity, "Target payload raw SHA-256",
			pSupplemental->TargetIdentity.strTargetPayloadRawSha256);
	}
	RenderFields("selected_visual_identity", Identity);

	if (bVisualMatches)
	{
		RenderResources("Row.Resources", pVisualRow->Resources);
		if (!pVisualRow->LocalDecalPacket.has_value())
			return;
		const EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_PACKET& Packet =
			*pVisualRow->LocalDecalPacket;
		EVIDENCE_FIELDS PacketFields;
		AddUnsigned(PacketFields, "Packet version", Packet.iPacketVersion);
		AddText(PacketFields, "Adapter ID", Packet.strAdapterId);
		AddBool(PacketFields, "Bounded semantic replay",
			Packet.bBoundedSemanticReplay);
		AddBool(PacketFields, "Native execution", Packet.bNativeExecution);
		AddBool(PacketFields, "Native vertex factory admitted",
			Packet.bNativeVertexFactoryAdmitted);
		AddBool(PacketFields, "Native MRT admitted",
			Packet.bNativeMrtAdmitted);
		AddText(PacketFields, "Runtime carrier", Packet.strRuntimeCarrier);
		AddText(PacketFields, "Native VF candidate",
			Packet.strNativeVertexFactoryCandidate);
		AddText(PacketFields, "Native VS SHA-256",
			Packet.strNativeVertexShaderSha256);
		AddText(PacketFields, "Native PS SHA-256",
			Packet.strNativePixelShaderSha256);
		AddText(PacketFields, "Render profile", Packet.strRenderProfile);
		AddText(PacketFields, "Rasterizer state", Packet.strRasterizerState);
		AddText(PacketFields, "Depth stencil state",
			Packet.strDepthStencilState);
		AddText(PacketFields, "Blend state", Packet.strBlendState);
		AddUnsigned(PacketFields, "Texture lane count",
			Packet.iTextureLaneCount);
		AddUnsigned(PacketFields, "Texture mask", Packet.iTextureMask);
		AddText(PacketFields, "Packet SHA-256", Packet.strPacketSha256);
		if (ImGui::TreeNodeEx("LocalDecal packet",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderFields("local_decal_packet", PacketFields);
			for (size_t iSrv = 0u; iSrv < Packet.Srvs.size(); ++iSrv)
			{
				const EFFECT_VISUAL_PROGRAM_LOCAL_DECAL_SRV& Srv =
					Packet.Srvs[iSrv];
				EVIDENCE_FIELDS SrvFields;
				AddText(SrvFields, "Role", Srv.strRole);
				AddText(SrvFields, "Asset ID", Srv.strAssetId);
				AddText(SrvFields, "Raw SHA-256", Srv.strRawSha256);
				AddUnsigned(SrvFields, "Byte count", Srv.iByteCount);
				AddText(SrvFields, "Shader register", Srv.strShaderRegister);
				AddText(SrvFields, "Source channel", Srv.strSourceChannel);
				AddText(SrvFields, "Runtime sampler",
					Srv.strRuntimeSamplerRegister);
				AddText(SrvFields, "Source sampler evidence",
					Srv.strSourceSamplerEvidence);
				AddText(SrvFields, "Sampler policy", Srv.strSamplerPolicy);
				AddText(SrvFields, "Linear format", Srv.strLinearFormat);
				if (!SrvFields.empty())
					AddBool(SrvFields, "sRGB", Srv.bSrgb);
				AddUnsigned(SrvFields, "Width", Srv.iWidth);
				AddUnsigned(SrvFields, "Height", Srv.iHeight);
				AddUnsigned(SrvFields, "Mip count", Srv.iMipCount);
				AddUnsigned(SrvFields, "Array size", Srv.iArraySize);
				if (SrvFields.empty())
					continue;
				ImGui::PushID(static_cast<int>(iSrv));
				const std::string Label = "SRV " +
					std::to_string(iSrv) +
					(Srv.strRole.empty() ? std::string{} :
						": " + Srv.strRole);
				if (ImGui::TreeNodeEx(Label.c_str(),
					ImGuiTreeNodeFlags_DefaultOpen))
				{
					RenderFields("local_decal_srv", SrvFields);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			RenderLimitations(Packet.PreservedLimitations);
			ImGui::TreePop();
		}
		return;
	}

	RenderResources("Supplemental Resources", pSupplemental->Resources);
	EVIDENCE_FIELDS SupplementalFields;
	AddText(SupplementalFields, "Adapter ID", pSupplemental->strAdapterId);
	AddText(SupplementalFields, "Packet layout", pSupplemental->strPacketLayout);
	AddText(SupplementalFields, "Fidelity", pSupplemental->strFidelity);
	AddText(SupplementalFields, "Stage ID", pSupplemental->strStageId);
	AddText(SupplementalFields, "Source event ID",
		pSupplemental->strSourceEventId);
	AddDouble(SupplementalFields, "Source timeline seconds",
		pSupplemental->fSourceTimelineSeconds);
	AddDouble(SupplementalFields, "Local time seconds",
		pSupplemental->fLocalTimeSeconds);
	AddDouble(SupplementalFields, "Duration seconds",
		pSupplemental->fDurationSeconds);
	RenderFields("supplemental_row_packet", SupplementalFields);
	if (pSupplemental->CascadeRibbonPacket.has_value())
	{
		const EFFECT_VISUAL_PROGRAM_CASCADE_RIBBON_PACKET& Packet =
			*pSupplemental->CascadeRibbonPacket;
		EVIDENCE_FIELDS Fields;
		AddUnsigned(Fields, "Packet version", Packet.iPacketVersion);
		AddText(Fields, "Adapter ID", Packet.strAdapterId);
		AddBool(Fields, "Bounded semantic replay",
			Packet.bBoundedSemanticReplay);
		AddBool(Fields, "Native execution", Packet.bNativeExecution);
		AddText(Fields, "Runtime carrier", Packet.strRuntimeCarrier);
		AddText(Fields, "TypeData stable ID", Packet.strTypeDataStableId);
		AddText(Fields, "TypeData class", Packet.strTypeDataClassName);
		AddText(Fields, "TypeData object path", Packet.strTypeDataObjectPath);
		AddText(Fields, "TypeData module SHA-256",
			Packet.strTypeDataModuleSha256);
		AddText(Fields, "Resolved renderer", Packet.strResolvedRendererShape);
		AddDouble(Fields, "Tiling distance", Packet.fTilingDistance);
		AddDouble(Fields, "Distance tessellation step",
			Packet.fDistanceTessellationStepSize);
		AddDouble(Fields, "Tangent tessellation scalar",
			Packet.fTangentTessellationScalar);
		AddUnsigned(Fields, "Operational max points",
			Packet.iOperationalMaxPoints);
		AddText(Fields, "Source recipe SHA-256",
			Packet.strSourceRecipeSha256);
		AddText(Fields, "Module closure SHA-256",
			Packet.strModuleClosureSha256);
		AddUnsigned(Fields, "Module count", Packet.iModuleCount);
		AddText(Fields, "Packet SHA-256", Packet.strPacketSha256);
		if (ImGui::TreeNodeEx("CascadeRibbon packet",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderFields("cascade_ribbon_packet", Fields);
			RenderLimitations(Packet.PreservedLimitations);
			ImGui::TreePop();
		}
	}
	if (pSupplemental->AnimationTrailPacket.has_value())
	{
		const EFFECT_VISUAL_PROGRAM_ANIMATION_TRAIL_PACKET& Packet =
			*pSupplemental->AnimationTrailPacket;
		EVIDENCE_FIELDS Fields;
		AddUnsigned(Fields, "Packet version", Packet.iPacketVersion);
		AddText(Fields, "Adapter ID", Packet.strAdapterId);
		AddBool(Fields, "Bounded semantic replay",
			Packet.bBoundedSemanticReplay);
		AddBool(Fields, "Native execution", Packet.bNativeExecution);
		AddText(Fields, "Runtime carrier", Packet.strRuntimeCarrier);
		AddText(Fields, "Source notify type", Packet.strSourceNotifyType);
		AddText(Fields, "Source event ID", Packet.strSourceEventId);
		AddText(Fields, "Source event record SHA-256",
			Packet.strSourceEventRecordSha256);
		AddText(Fields, "Source asset", Packet.strSourceAsset);
		AddText(Fields, "Clip", Packet.strClip);
		AddDouble(Fields, "Local time seconds", Packet.fLocalTimeSeconds);
		AddDouble(Fields, "Global time seconds", Packet.fGlobalTimeSeconds);
		AddDouble(Fields, "Duration seconds", Packet.fDurationSeconds);
		AddText(Fields, "Target element ID", Packet.strTargetElementId);
		AddText(Fields, "Packet SHA-256", Packet.strPacketSha256);
		if (ImGui::TreeNodeEx("AnimationTrail packet",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderFields("animation_trail_packet", Fields);
			RenderLimitations(Packet.PreservedLimitations);
			ImGui::TreePop();
		}
	}
}

void Client::CEffect_Tool::Render_ParticleSystemDetail()
{
    if (!m_ActiveDocument.has_value())
        return;
    if (!m_ParticleSystemDraft.has_value())
    {
        m_ParticleSystemDraft = m_ActiveDocument->ParticleSystem;
        m_bParticleSystemDraftDirty = false;
        m_strDetailStatus.clear();
    }

    const PARTICLE_LAYER_SUMMARY Summary =
        Summarize_ParticleLayers(*m_ActiveDocument);
    ImGui::Text("Cascade System: %s",
        m_ActiveDocument->strDisplayName.c_str());
    ImGui::TextDisabled(
        "Source Systems %zu | Emitters %zu | Layers %zu",
        Summary.iSourceSystemCount,
        Summary.iSourceEmitterCount,
        Summary.iLayerCount);
    ImGui::TextDisabled(
        "Mesh Particles %zu | Sprite Particles %zu | Unresolved %zu | Budget %llu",
        Summary.iMeshRendererCount, Summary.iSpriteRendererCount,
        Summary.iUnresolvedRendererCount,
        static_cast<unsigned long long>(Summary.iParticleBudget));
    ImGui::TextWrapped(
        "These controls preserve every source emitter, renderer, material, burst, "
        "and lifetime. They apply only to Cascade emitters; Model Cues and Decals "
        "are unchanged.");
	if (ImGui::Button("Open First Emitter"))
	{
		Try_SelectFirstEmitter(m_ActiveDocument->strEffectAssetId, {});
		return;
	}
	ImGui::SameLine();
	ImGui::TextDisabled("Emitter selection reveals Renderer, Resources, and Modules.");

    bool_t bChanged = false;
    bChanged |= ImGui::DragFloat("Uniform Scale Multiplier",
        &m_ParticleSystemDraft->fUniformScaleMultiplier,
        0.01f, 0.001f, 100.f, "%.3f");
    bChanged |= ImGui::DragFloat("System Yaw Offset (Degrees)",
        &m_ParticleSystemDraft->fYawOffsetDegrees,
        0.25f, -360.f, 360.f, "%.3f");
    bChanged |= ImGui::DragFloat("Emission Direction Yaw (Degrees)",
        &m_ParticleSystemDraft->fDirectionYawDegrees,
        0.25f, -360.f, 360.f, "%.3f");
    bChanged |= ImGui::DragFloat("Initial Speed Multiplier",
        &m_ParticleSystemDraft->fInitialSpeedMultiplier,
        0.01f, 0.f, 100.f, "%.3f");
    ImGui::TextDisabled(
        "System Yaw rotates the whole layout; Direction Yaw rotates initial emission only.");
    if (bChanged)
    {
        m_bParticleSystemDraftDirty = true;
        m_strDetailStatus =
            "Live preview only; Apply Particle System commits this draft to memory.";
        Stage_ParticleSystemDraftPreview();
    }

    ImGui::Separator();
    ImGui::BeginDisabled(!m_bParticleSystemDraftDirty);
    if (ImGui::Button("Set Particle System"))
    {
        EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
        if (!Apply_ParticleSystemDraft(Staged))
        {
            m_strDetailStatus = "Apply failed; Particle System draft is missing.";
        }
        else if (Try_CommitDocument(std::move(Staged)))
        {
            m_bParticleSystemDraftDirty = false;
            m_ParticleSystemDraft = m_ActiveDocument->ParticleSystem;
            m_strDetailStatus =
                "Applied Particle System to active Document memory; Save required to persist.";
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Revert Particle System"))
    {
        m_ParticleSystemDraft = m_ActiveDocument->ParticleSystem;
        m_bParticleSystemDraftDirty = false;
        Recalculate_PreviewDuration();
        if (Stage_WorldPreview())
            m_strPreviewStatus =
                "Particle System draft reverted to the active Document preview.";
        m_strDetailStatus =
            "Reverted the Particle System draft to the active Document.";
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Audition Particle System"))
        Try_AuditionParticleSystem();
	ImGui::SameLine();
	if (ImGui::Button("Play Mesh Particles") &&
		Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS))
	{
		Start_WorldPreviewFromBeginning();
	}
    if (m_bParticleSystemDraftDirty)
        ImGui::TextDisabled(
            "Particle System draft is local until Apply Particle System.");
    if (!m_strDetailStatus.empty())
        ImGui::TextWrapped("%s", m_strDetailStatus.c_str());
}

void Client::CEffect_Tool::Render_Detail(
    EFFECT_ELEMENT_DESC& Element,
    bool_t& bChanged)
{
	const EFFECT_AUTHORING_FAMILY eSurface = Resolve_AuthoringFamily(Element);
	const char* pSurface = EFFECT_AUTHORING_FAMILY::END == eSurface ?
		Kind_Label(Element.eKind) : AuthoringFamily_Label(eSurface);
	const EFFECT_AUTHORING_FIDELITY eFidelity =
		Get_EffectAuthoringFidelity(Element.Material.Execution);
	bool_t bModified = !Element.AuthoringOverrides.Is_Empty();
	ImGui::Text("%s %s %s%s", pSurface, "\xC2\xB7",
		Get_EffectAuthoringFidelityLabel(eFidelity),
		bModified ? " \xC2\xB7 Modified" : "");
	const bool_t bTypedPresentationElement =
		Element.eKind == EFFECT_ELEMENT_KIND::LIGHT ||
		Element.eKind == EFFECT_ELEMENT_KIND::SCREEN_POST;
	/* Drawable source carriers keep their compiler module ownership. Typed
	   presentation carriers are different: source modules still supply their
	   lifetime/curves, while Detail.Light/ScreenPost owns the submitted output. */
	if (Element.SourceRecipe.bEnabled && !bTypedPresentationElement)
	{
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
			"SourceRecipe drives native spawn, lifetime, motion, size, and dynamic values.");
		ImGui::TextDisabled(
			"%zu source modules. Only the explicitly labelled authored overlays below remain live.",
			Element.SourceRecipe.Modules.size());
	}
	else if (bTypedPresentationElement)
	{
		ImGui::TextDisabled(
			"Typed Presentation Detail owns output; source modules retain lifetime and curve evaluation only.");
	}
	else
	{
		ImGui::TextDisabled("Authored Detail owns playback.");
	}
	if (bModified)
	{
		ImGui::SameLine();
		if (ImGui::SmallButton("Reset All to Source"))
		{
			std::string strError;
			if (Reset_AllAuthoringOverrides(Element, strError))
			{
				bChanged = true;
				bModified = false;
				m_strDetailStatus =
					"All authoring overrides reset to source values.";
			}
			else
			{
				m_strDetailStatus = "Reset to Source failed: " + strError;
			}
		}
	}
	if (m_ActiveDocument.has_value() &&
		m_ActiveDocument->strEffectAssetId == ARTIST_F_UNIFIED_EFFECT_ASSET_ID &&
		nullptr != m_pArtistFSourcePreparation &&
		nullptr != m_pArtistFSourcePreparation->Get_Program())
	{
		const auto TrackASource = std::find_if(
			m_pArtistFSourcePreparation->Get_Program()->Emitters.begin(),
			m_pArtistFSourcePreparation->Get_Program()->Emitters.end(),
			[&Element](const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter)
			{
				EFFECT_GPU_RENDER_FAMILY eFamily =
					EFFECT_GPU_RENDER_FAMILY::END;
				return Emitter.bVisible &&
					Try_ResolveArtistCoreFamily(Emitter.eRenderer, eFamily) &&
					StableUnifiedElementId(eFamily, Emitter.Row.strId) ==
						Element.strElementId;
			});
		if (TrackASource !=
			m_pArtistFSourcePreparation->Get_Program()->Emitters.end())
		{
			if (TrackASource->ActionCueAttachment.bEnabled &&
				TrackASource->ActionCueAttachment.bFollow)
			{
				ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
					"Track A Live Follow not migrated: this Element keeps its emit-start baked Transform approximation.");
			}
			const auto Snapshot = m_ArtistFMaterialExecutionSnapshots.find(
				TrackASource->strSourceElementId);
			if (Snapshot != m_ArtistFMaterialExecutionSnapshots.end() &&
				!Snapshot->second.bEnabled)
			{
				ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
					"Track A Material unresolved: no typed recipe was baked for this row; its existing authored material was preserved.");
			}
		}
	}
	const bool_t bMissingBaseSourceDecal =
		Is_MissingBaseSourceDecal(Element);
	const bool_t bAuthoringExecutionTarget =
		Is_EffectElementAuthoringExecutionTarget(Element);
	if (m_bDetailDraftCapabilityDeferred)
	{
		bool_t bLockedVisible = false;
		ImGui::BeginDisabled();
		ImGui::Checkbox("Visible", &bLockedVisible);
		ImGui::EndDisabled();
		ImGui::TextColored(ImVec4(1.f, 0.45f, 0.25f, 1.f),
			"Visibility locked OFF: this SourceRecipe carrier is capability-deferred and cannot be safely previewed or approved.");
		ImGui::TextWrapped("Reason: %s",
			m_strDetailDraftCapabilityReason.c_str());
	}
	else if (bMissingBaseSourceDecal)
	{
		bool_t bLockedVisible = false;
		ImGui::BeginDisabled();
		ImGui::Checkbox("Visible", &bLockedVisible);
		ImGui::EndDisabled();
		ImGui::TextColored(ImVec4(1.f, 0.72f, 0.22f, 1.f),
			"Visibility locked OFF: this imported Decal has no Base DDS.");
		ImGui::TextWrapped(
			"In Resources, select the empty Base input, choose one DDS, then click Bind Selected. A valid Base bind clears this exact material fail-closed marker and enables preview; Transform, rotation, scale, and color remain editable while locked.");
	}
	else if (eFidelity == EFFECT_AUTHORING_FIDELITY::APPROXIMATE)
	{
		bChanged |= ImGui::Checkbox("Visible", &Element.bVisible);
		ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.55f, 1.f),
			"PROJECT_TUNED APPROXIMATE | Preview enabled | editable and tunable");
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"The typed runtime packet and Adapter are validated, but original Material arithmetic is not SOURCE_EXACT. Overrides never change this fidelity label.");
		}
	}
	else if (eFidelity == EFFECT_AUTHORING_FIDELITY::PROJECT_TUNED_APPROX)
	{
		bChanged |= ImGui::Checkbox("Visible", &Element.bVisible);
		ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.55f, 1.f),
			"PROJECT_TUNED_APPROX | Runtime enabled | editable and tunable");
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"This admitted Program/Layout/Descriptor/Adapter packet is project-tuned, not a source-exact material replay.");
		}
	}
	else if (!bAuthoringExecutionTarget)
	{
		bool_t bLockedVisible = false;
		ImGui::BeginDisabled();
		ImGui::Checkbox("Visible", &bLockedVisible);
		ImGui::EndDisabled();
		ImGui::TextColored(ImVec4(1.f, 0.45f, 0.25f, 1.f),
			"Visibility locked OFF: this material/runtime carrier is fail-closed.");
		ImGui::TextWrapped(
			"Element editing and Save remain available, but Play/Solo stay disabled until a supported admission path explicitly clears this marker.");
	}
	else
	{
		bChanged |= ImGui::Checkbox("Visible", &Element.bVisible);
	}
	Render_CompositionDetail(Element, bChanged);
	const bool_t bEditableSourceMeshAttachment = Element.SourceRecipe.bEnabled &&
		eSurface == EFFECT_AUTHORING_FAMILY::MESH_PARTICLE &&
		Element.ActionCueAttachment.strModelCueId.empty();
	if (Element.ActionCueAttachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
		(Can_EditElementFollowAttachment(Element) || bEditableSourceMeshAttachment) &&
		Element.ActionCueAttachment.bEnabled && Element.ActionCueAttachment.bFollow &&
		Element.RuntimeCarrier.Is_Empty() &&
		m_ActiveDocument.has_value() && !m_ActiveDocument->bSourceContract &&
		ImGui::CollapsingHeader("Element Follow Attachment"))
	{
		auto& Attachment = Element.ActionCueAttachment;
		ImGui::TextWrapped("Bone: %s", Attachment.strRuntimeBoneName.c_str());
		int iOrientation = static_cast<int>(Attachment.eOrientation);
		const char* OrientationItems[] = { "Bone", "Owner yaw (unit scale)" };
		bool_t bAttachmentChanged = ImGui::Combo("Follow Orientation", &iOrientation,
			OrientationItems, static_cast<int>(std::size(OrientationItems)));
		if (bAttachmentChanged)
			Attachment.eOrientation = static_cast<EFFECT_ATTACHMENT_ORIENTATION>(iOrientation);
		bAttachmentChanged |= ImGui::DragFloat3("Socket Offset (Meters)",
			&Attachment.SocketLocalTransform.vPosition.x, 0.01f);
		bAttachmentChanged |= ImGui::DragFloat3("Socket Rotation (Degrees)",
			&Attachment.SocketLocalTransform.vRotationDegrees.x, 0.25f);
		bAttachmentChanged |= ImGui::DragFloat3("Socket Scale",
			&Attachment.SocketLocalTransform.vScale.x, 0.01f, 0.0001f, 100.f);
		if (bAttachmentChanged)
		{
			// Runtime anchor maps deduplicate by this ID. An edited socket must
			// not inherit another emitter's first Midcontrol binding.
			Attachment.strRuntimeAnchorSlotId = Element.strElementId;
			bChanged = true;
		}
		ImGui::TextWrapped(
			"Bone uses the animated socket axes. Owner yaw keeps its position with +Z forward, +X right and +Y up. Apply and Save preserve this Element's offset.");
	}
	Render_TransformDetail(Element.Detail, bChanged);
	Render_TimingDetail(Element, bChanged);
	Render_SizeDetail(Element, bChanged);
	const bool_t bParticleMasterNamedEmission =
		Is_ParticleMasterEmissionProgram(Element);
	const bool_t bLockProjectTunedCarrierColor =
		Element.Material.Execution.bEnabled &&
		Element.Material.Execution.eBackend ==
			EFFECT_MATERIAL_EXECUTION_BACKEND::RUNTIME_MATERIAL_V2 &&
		(Element.Material.Execution.iOpcode == 1003u ||
		 Element.Material.Execution.iOpcode == 1004u);
	Render_ColorDetail(Element.Detail, bChanged,
		Has_EffectiveEmissiveRadianceInput(Element),
		bParticleMasterNamedEmission,
		bLockProjectTunedCarrierColor);
	Render_LinearRevealDetail(Element, bChanged);
	if (Element.Detail.Mesh.SourceMaterialSlots.empty())
		Render_AuthoringMaterialParameters(Element, bChanged);
	if (ImGui::CollapsingHeader("Advanced Authoring"))
	{
		Render_UVDetail(Element.Detail, bChanged);
		Render_UVKeyframes(Element, bChanged);
		Render_KindDetail(Element, bChanged);
		Render_LerpDetail(Element.Detail, bChanged,
			bLockProjectTunedCarrierColor);
	}
	if (!ImGui::CollapsingHeader("Material / Render"))
		return;
	if (!Element.Detail.Mesh.SourceMaterialSlots.empty())
	{
		ImGui::TextWrapped(
			"This mesh uses the source material slots below. The primary material is retained as source evidence and is not used for drawing. Slot materials are read-only here; shared Transform, Timing and Color remain editable.");
		for (const EFFECT_SOURCE_MATERIAL_SLOT_DESC& Slot :
			Element.Detail.Mesh.SourceMaterialSlots)
		{
			ImGui::SeparatorText(
				("Source material slot " + std::to_string(Slot.iSourceMaterialIndex)).c_str());
			ImGui::TextWrapped("%s", Slot.Material.strSourceMaterialPath.c_str());
			ImGui::TextDisabled("%s", Profile_Label(Slot.Material.eRenderProfile));
		}
		return;
	}
    ImGui::TextDisabled("Material Template: %s",
        Element.Material.strTemplateId.c_str());
	const EFFECT_MATERIAL_EXECUTION_DESC& MaterialExecution =
		Element.Material.Execution;
	if (!Is_EffectElementAuthoringExecutionTarget(Element) &&
		!bMissingBaseSourceDecal)
	{
		ImGui::TextColored(ImVec4(1.f, 0.45f, 0.25f, 1.f),
			"Fail-closed source pass: generic/white fallback is disabled. This Element will not draw until a supported authored recipe replaces it.");
	}
	if (MaterialExecution.bEnabled &&
		ImGui::CollapsingHeader("Material Resources"))
	{
		ImGui::Text("Recipe: %s",
			MaterialExecutionBackendLabel(MaterialExecution.eBackend));
		ImGui::TextDisabled(
			"Each card preserves the source semantic role, t#/s# register, channel, color space, and sampler. Select a card, then bind one DDS in the Resource Library.");
		const float fCardWidth = 108.f;
		const size_t iColumns = static_cast<size_t>((std::max)(1,
			static_cast<int32_t>(
				ImGui::GetContentRegionAvail().x / fCardWidth)));
		for (size_t iLane = 0u;
			iLane < MaterialExecution.TextureLanes.size(); ++iLane)
		{
			const EFFECT_MATERIAL_TEXTURE_LANE_DESC& Lane =
				MaterialExecution.TextureLanes[iLane];
			if (0u != iLane % iColumns)
				ImGui::SameLine();
			ImGui::PushID(Lane.strLaneId.c_str());
			ImGui::BeginGroup();
			const CEffectThumbnailCache::RESULT Thumbnail =
				m_pThumbnailCache->Request(
					Lane.strAssetId, EFFECT_RESOURCE_FILE_KIND::TEXTURE);
			bool_t bClicked = false;
			if (nullptr != Thumbnail.pTextureView)
			{
				ImGui::Image(Thumbnail.pTextureView, ImVec2(82.f, 58.f));
				bClicked = ImGui::IsItemClicked();
			}
			else
			{
				bClicked = ImGui::Button(
					Lane.strAssetId.empty() ? "Empty DDS" : "DDS",
					ImVec2(82.f, 58.f));
				if (ImGui::IsItemHovered() && nullptr != Thumbnail.pError)
					ImGui::SetTooltip("%s", Thumbnail.pError->c_str());
			}
			const std::string strSlotId =
				MaterialExecutionLaneSlotId(Lane.strLaneId);
			if (m_strSelectedResourceSlotId == strSlotId)
			{
				ImGui::GetWindowDrawList()->AddRect(
					ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
					ImGui::GetColorU32(ImGuiCol_HeaderActive),
					2.f, 0, 2.f);
			}
			if (bClicked)
			{
				m_strSelectedResourceSlotId = strSlotId;
				m_strSelectedResourceAssetId.clear();
				m_eResourceLibraryFileKind =
					EFFECT_RESOURCE_FILE_KIND::TEXTURE;
			}
			std::string strRole = Lane.strRole.empty() ?
				Lane.strLaneId : Lane.strRole;
			if (strRole.size() > 14u)
				strRole = strRole.substr(0u, 12u) + "..";
			ImGui::TextUnformatted(strRole.c_str());
			ImGui::TextDisabled("t%u / s%u / %s",
				Lane.iTextureRegister, Lane.iSamplerRegister,
				Lane.strSourceChannel.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Role: %s\nLane: %s\nDDS: %s\nColor: %s\nFilter: %s",
					Lane.strRole.c_str(), Lane.strLaneId.c_str(),
					Lane.strAssetId.c_str(),
					MaterialTextureColorSpaceLabel(Lane.eColorSpace),
					MaterialTextureFilterLabel(Lane.Sampler.eFilter));
			}
			ImGui::EndGroup();
			ImGui::PopID();
		}
		if (ImGui::TreeNode("Advanced Material Packet"))
		{
			ImGui::TextDisabled(
				"Opcode %u | Pass %u | Texture mask 0x%08X",
				MaterialExecution.iOpcode,
				MaterialExecution.iPassIndex,
				MaterialExecution.iTextureMask);
			ImGui::TextWrapped("Rasterizer: %s",
				MaterialExecution.strRasterizerState.c_str());
			ImGui::TextWrapped("Depth/Stencil: %s",
				MaterialExecution.strDepthStencilState.c_str());
			ImGui::TextWrapped("Blend: %s",
				MaterialExecution.strBlendState.c_str());
			ImGui::TextDisabled(
				"Scalars %zu | Vectors %zu | Artist Params %zu | Colors %zu",
				MaterialExecution.Scalars.size(),
				MaterialExecution.Vectors.size(),
				MaterialExecution.ArtistParameters.size(),
				MaterialExecution.Colors.size());
			ImGui::TreePop();
		}
	}
	const EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial =
		Element.Material.SourceMaterial;
	if (SourceMaterial.bEnabled &&
		ImGui::CollapsingHeader("Source Material Profile"))
	{
		ImGui::TextDisabled("Read-only compiler output.");
		ImGui::Text("Status: %s",
			SourceMaterialStatus_Label(SourceMaterial.eStatus));
		ImGui::TextWrapped("Source: %s",
			Element.Material.strSourceMaterialPath.c_str());
		ImGui::TextWrapped("Parent: %s",
			SourceMaterial.strParentMaterialPath.c_str());
		ImGui::TextWrapped("Profile: %s",
			SourceMaterial.strProfileId.c_str());
		ImGui::TextWrapped("Runtime Shader: %s",
			SourceMaterial.strRuntimeShaderProfileId.c_str());
		ImGui::Text("SubUV: %s", SourceMaterial.strSubUVMode.c_str());
		ImGui::TextDisabled("Dynamic: X=%s Y=%s Z=%s W=%s",
			SourceMaterial.DynamicParameterSemantics[0].c_str(),
			SourceMaterial.DynamicParameterSemantics[1].c_str(),
			SourceMaterial.DynamicParameterSemantics[2].c_str(),
			SourceMaterial.DynamicParameterSemantics[3].c_str());
		Render_SourceMaterialParameterGroups(
			SourceMaterial, m_pThumbnailCache.get());
	}
    if (ImGui::CollapsingHeader("Runtime Sample"))
    {
        const bool_t bHasBaseBinding = std::any_of(
            Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
            [](const EFFECT_RESOURCE_BINDING_DESC& Binding)
            {
                return Binding.strSlotId == "base";
            });
        const bool_t bFallbackBlocked =
            SourceMaterial.strRuntimeShaderProfileId ==
                "effect.ue3.fallback-blocked.v1";
        const bool_t bGenericReconstructed =
            SourceMaterial.strRuntimeShaderProfileId ==
                "effect.ue3.reconstructed-standard.v1";
        const bool_t bUnsafeBaseBinding = std::any_of(
            Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
            [](const EFFECT_RESOURCE_BINDING_DESC& Binding)
            {
                return Binding.strSlotId == "base" &&
                    Is_UnsafeEffectBaseTextureAssetId(Binding.strAssetId);
            });
        const bool_t bRuntimeFallbackBlocked = bFallbackBlocked ||
            (bGenericReconstructed &&
                (!bHasBaseBinding || bUnsafeBaseBinding));
        uint32_t iMeshMaterialOverrideCount = 0u;
        for (const EFFECT_SOURCE_MODULE_DESC& Module :
            Element.SourceRecipe.Modules)
        {
            if (Module.strClassName.find("meshmaterial") == std::string::npos)
                continue;
            iMeshMaterialOverrideCount += static_cast<uint32_t>(std::count_if(
                Module.Literals.begin(), Module.Literals.end(),
                [](const EFFECT_SOURCE_LITERAL_DESC& Literal)
                {
                    return Literal.strPropertyPath.starts_with("meshmaterials[") &&
                        Literal.strPropertyPath.ends_with("].objectpath");
                }));
        }
        ImGui::TextWrapped("Source Material: %s",
            Element.Material.strSourceMaterialPath.c_str());
        ImGui::TextWrapped("Parent: %s",
            SourceMaterial.strParentMaterialPath.c_str());
        ImGui::TextWrapped("Profile: %s | Runtime Shader: %s",
            SourceMaterial.strProfileId.c_str(),
            SourceMaterial.strRuntimeShaderProfileId.c_str());
        ImGui::Text("Semantic: %s | Render: %s",
            SourceMaterialStatus_Label(SourceMaterial.eStatus),
            Profile_Label(Element.Material.eRenderProfile));
        ImGui::Text("Mesh model material: %s | Per-section overrides: %u",
            Element.Detail.Mesh.bUseModelMaterial ? "use" : "override",
            iMeshMaterialOverrideCount);
        if (ImGui::TreeNode("Resolved Runtime Resources"))
        {
            for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
                Element.ResourceBindings)
            {
                ImGui::BulletText("%s = %s [staged-loaded]",
                    Binding.strSlotId.c_str(), Binding.strAssetId.c_str());
            }
            if (Element.ResourceBindings.empty())
                ImGui::TextDisabled("(none)");
            ImGui::TreePop();
        }
        ImGui::Text("Fallback: %s",
            bRuntimeFallbackBlocked ? "FAIL-CLOSED / emitter skipped" :
            !bHasBaseBinding ? "missing Base (white fallback risk)" :
            "no missing-Base fallback");
        EFFECT_PARTICLE_RUNTIME_PROBE Probe;
        const shared_ptr<CEffectObject> pPreview =
            m_pWorldPreviewObject.lock();
        const bool_t bFound = nullptr != pPreview &&
            pPreview->Query_ParticleRuntimeProbe(
                Element.strElementId, Probe);
        if (!bFound)
        {
            ImGui::TextDisabled(
                "No staged runtime element for the current selection.");
        }
        else
        {
            ImGui::Text("Sample: %.6f s | Active: %u | Renderer: %s",
                Probe.fSampleTimeSeconds, Probe.iActiveParticleCount,
                Probe.bMeshRenderer ? "Mesh Particle" : "Sprite Particle");
            if (0u == Probe.iActiveParticleCount)
            {
                ImGui::TextDisabled(
                    "The selected emitter has no live particles at this sample.");
            }
            else
            {
                ImGui::Text("CPU pre-material alpha first/min/max: %.6g / %.6g / %.6g",
                    Probe.fFirstAlpha, Probe.fMinAlpha, Probe.fMaxAlpha);
                ImGui::TextDisabled(
                    "Final GPU Material opacity is not sampled by this probe.");
                ImGui::Text(
                    "Dynamic first: [%.6g, %.6g, %.6g, %.6g]",
                    Probe.vFirstDynamicParameter.x,
                    Probe.vFirstDynamicParameter.y,
                    Probe.vFirstDynamicParameter.z,
                    Probe.vFirstDynamicParameter.w);
                ImGui::Text(
                    "Dynamic min: [%.6g, %.6g, %.6g, %.6g]",
                    Probe.vMinDynamicParameter.x,
                    Probe.vMinDynamicParameter.y,
                    Probe.vMinDynamicParameter.z,
                    Probe.vMinDynamicParameter.w);
                ImGui::Text(
                    "Dynamic max: [%.6g, %.6g, %.6g, %.6g]",
                    Probe.vMaxDynamicParameter.x,
                    Probe.vMaxDynamicParameter.y,
                    Probe.vMaxDynamicParameter.z,
                    Probe.vMaxDynamicParameter.w);
                ImGui::Text("Life: %.6g | Raw SubImage: %.6g",
                    Probe.fFirstNormalizedLife,
                    Probe.fFirstSubImageIndex);
                ImGui::Text(
                    "Resolved SubUV current=[%.6g, %.6g, %.6g, %.6g] "
                    "next=[%.6g, %.6g, %.6g, %.6g] blend=%.6g",
                    Probe.FirstSubUV.Current.x, Probe.FirstSubUV.Current.y,
                    Probe.FirstSubUV.Current.z, Probe.FirstSubUV.Current.w,
                    Probe.FirstSubUV.Next.x, Probe.FirstSubUV.Next.y,
                    Probe.FirstSubUV.Next.z, Probe.FirstSubUV.Next.w,
                    Probe.FirstSubUV.fBlend);
            }
            if (ImGui::Button("Copy Runtime Probe"))
            {
                std::ostringstream Text;
                Text << "effect="
                    << (m_ActiveDocument.has_value() ?
                        m_ActiveDocument->strEffectAssetId : "(none)")
                    << " element=" << Element.strElementId
                    << " sample=" << Probe.fSampleTimeSeconds
                    << " active=" << Probe.iActiveParticleCount
                    << " renderer="
                    << (Probe.bMeshRenderer ? "mesh" : "sprite")
                    << " source_material="
                    << Element.Material.strSourceMaterialPath
                    << " parent=" << SourceMaterial.strParentMaterialPath
                    << " profile=" << SourceMaterial.strProfileId
                    << " runtime_shader="
                    << SourceMaterial.strRuntimeShaderProfileId
                    << " semantic="
                    << SourceMaterialStatus_Label(SourceMaterial.eStatus)
                    << " render_profile="
                    << Profile_Label(Element.Material.eRenderProfile)
                    << " mesh_use_model_material="
                    << (Element.Detail.Mesh.bUseModelMaterial ? 1 : 0)
                    << " mesh_section_overrides="
                    << iMeshMaterialOverrideCount
                    << " fallback="
                    << (bRuntimeFallbackBlocked ? "fail_closed" :
                        !bHasBaseBinding ? "missing_base_white_risk" : "none")
                    << " cpu_pre_material_alpha=" << Probe.fFirstAlpha << '/'
                    << Probe.fMinAlpha << '/' << Probe.fMaxAlpha
                    << " dynamic=[" << Probe.vFirstDynamicParameter.x << ','
                    << Probe.vFirstDynamicParameter.y << ','
                    << Probe.vFirstDynamicParameter.z << ','
                    << Probe.vFirstDynamicParameter.w << ']'
                    << " life=" << Probe.fFirstNormalizedLife
                    << " subimage=" << Probe.fFirstSubImageIndex
                    << " subuv_current=[" << Probe.FirstSubUV.Current.x << ','
                    << Probe.FirstSubUV.Current.y << ','
                    << Probe.FirstSubUV.Current.z << ','
                    << Probe.FirstSubUV.Current.w << ']'
                    << " subuv_next=[" << Probe.FirstSubUV.Next.x << ','
                    << Probe.FirstSubUV.Next.y << ','
                    << Probe.FirstSubUV.Next.z << ','
                    << Probe.FirstSubUV.Next.w << ']'
                    << " subuv_blend=" << Probe.FirstSubUV.fBlend;
                for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
                    Element.ResourceBindings)
                {
                    Text << " resource[" << Binding.strSlotId << "]="
                        << Binding.strAssetId << ":staged_loaded";
                }
                ImGui::SetClipboardText(Text.str().c_str());
            }
        }
        ImGui::TextDisabled(
            "Read-only evaluated playback data; no authoring values are changed.");
    }
	if (Element.SourceRecipe.bEnabled &&
		ImGui::TreeNode("Original Emitter / Module Stack"))
	{
		ImGui::TextDisabled("Read-only compiler output.");
		ImGui::Text("Renderer: %s",
			Element.SourceRecipe.strRendererShape.empty() ? "(unspecified)" :
				Element.SourceRecipe.strRendererShape.c_str());
		ImGui::TextDisabled("Delay %.6f | Duration %.6f | Loops %u",
			Element.SourceRecipe.fEmitterDelaySeconds,
			Element.SourceRecipe.fEmitterDurationSeconds,
			Element.SourceRecipe.iEmitterLoopCount);
		ImGui::TextDisabled("Bursts %zu | Modules %zu",
			Element.SourceRecipe.Bursts.size(),
			Element.SourceRecipe.Modules.size());
		for (const EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			ImGui::BulletText("%s | %s",
				Module.strStableId.c_str(), Module.strClassName.c_str());
			if (ImGui::IsItemHovered() && !Module.strObjectPath.empty())
				ImGui::SetTooltip("%s", Module.strObjectPath.c_str());
		}
		ImGui::TreePop();
	}
	/* The compiler derives the blend from the source material, but an Element
	   authored by hand has no source material to derive it from and simply
	   keeps the struct default of alpha two-sided. That default draws a glow
	   texture as an opaque quad, so leaving it read-only on authored Elements
	   locked the one control that fixes it. */
	const bool_t bCompilerOwnsRenderProfile =
		!Element.Material.strSourceMaterialPath.empty() ||
		Element.Material.SourceMaterial.bEnabled ||
		Element.Material.Execution.bEnabled;
	ImGui::SeparatorText(bCompilerOwnsRenderProfile ?
		"Compiler-owned Render Profile" : "Render Profile");
	if (bCompilerOwnsRenderProfile)
	{
		ImGui::TextDisabled("%s", Profile_Label(Element.Material.eRenderProfile));
		return;
	}
	static const char* const s_RenderProfileLabels[] =
	{
		"Opaque (back faces, depth write)",
		"Alpha (two sided, depth read)",
		"Additive (two sided, depth read)",
		"Alpha (one sided, depth read)",
		"Additive (one sided, depth read)"
	};
	int32_t iRenderProfile = static_cast<int32_t>(
		Element.Material.eRenderProfile);
	if (ImGui::Combo("Blend", &iRenderProfile, s_RenderProfileLabels,
		IM_ARRAYSIZE(s_RenderProfileLabels)))
	{
		Element.Material.eRenderProfile =
			static_cast<EFFECT_RENDER_PROFILE>(iRenderProfile);
		bChanged = true;
	}
	ImGui::TextDisabled(
		"Additive treats black as transparent, which is how glow and flare textures are drawn. Alpha needs the texture to carry its own alpha channel.");
}

void Client::CEffect_Tool::Render_CompositionDetail(
	EFFECT_ELEMENT_DESC& Element,
	bool_t& bChanged)
{
	if (!ImGui::CollapsingHeader("Ground / World Mark",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}
	const bool_t bEligible = Is_EffectWorldMarkCarrier(Element);
	const bool_t bBackdropEligible = Is_EffectSceneBackdropCarrier(Element);
	static const char* const s_CompositionLabels[] =
	{
		"Normal Translucency", "World Mark (before translucent FX)",
		"Scene Backdrop (replace map / sky)"
	};
	const int32_t iLayer = static_cast<int32_t>(Element.eCompositionLayer);
	if (ImGui::BeginCombo("Composition Layer", s_CompositionLabels[iLayer]))
	{
		for (int32_t iCandidate = 0; iCandidate < IM_ARRAYSIZE(s_CompositionLabels); ++iCandidate)
		{
			const bool_t bCanSelect = iCandidate == 0 ||
				(iCandidate == 1 ? bEligible : bBackdropEligible);
			ImGui::BeginDisabled(!bCanSelect);
			if (ImGui::Selectable(s_CompositionLabels[iCandidate], iLayer == iCandidate))
			{
				Element.eCompositionLayer = static_cast<EFFECT_COMPOSITION_LAYER>(iCandidate);
				bChanged = true;
			}
			ImGui::EndDisabled();
		}
		ImGui::EndCombo();
	}
	if (bBackdropEligible)
		ImGui::TextDisabled("Scene Backdrop replaces level geometry while this opaque mesh is active; characters and FX remain visible.");
	ImGui::BeginDisabled(!bEligible);
	if (ImGui::Button("Configure as Ground Mark"))
	{
		Element.eCompositionLayer = EFFECT_COMPOSITION_LAYER::WORLD_MARK;
		if (Element.eKind == EFFECT_ELEMENT_KIND::DECAL)
		{
			Element.Detail.Decal.eReceiverMode =
				EFFECT_DECAL_RECEIVER_MODE::UPWARD_SURFACES;
			Element.Detail.Decal.fNormalCutoff = 0.75f;
			Element.Detail.Decal.fDepth = 0.25f;
			Element.Detail.Decal.fEdgeFade = 0.05f;
		}
		bChanged = true;
	}
	ImGui::EndDisabled();
	if (!bEligible)
	{
		ImGui::TextDisabled(
			"World Mark supports non-Opaque Local Decal and direct-authored effect.standard Sprite/Sprite Particle carriers. Other carriers cannot use World Mark.");
		return;
	}
	if (Element.eKind == EFFECT_ELEMENT_KIND::DECAL)
	{
		ImGui::TextDisabled(
			"The Decal preset also enables Upward Surfaces, cutoff 0.75, depth 0.25, and a soft projector edge. It does not claim a world-only stencil receiver.");
	}
	else
	{
		ImGui::TextDisabled(
			"The Sprite preset changes only composition. Existing texture, transform, color, particle size, and lifetime tuning are preserved.");
	}
}

void Client::CEffect_Tool::Render_TransformDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    ImGui::TextDisabled(
        "Start values update the live preview immediately; Lerp checkboxes only enable Start-to-End interpolation.");
    bChanged |= DragFloat3(
        "Position", Detail.Transform.vPosition, 0.01f, -1000.f, 1000.f);
    bChanged |= DragFloat3("Rotation (Degrees)",
        Detail.Transform.vRotationDegrees, 0.25f, -360.f, 360.f);
    bChanged |= DragFloat3("Revolution (Degrees/Second)",
        Detail.Transform.vRevolutionDegreesPerSecond,
        0.5f, -3600.f, 3600.f);
    bChanged |= DragFloat3(
        "Scaling", Detail.Transform.vScale, 0.01f, 0.001f, 100.f);
    bChanged |= DragFloat3("Velocity",
        Detail.Transform.vVelocityPerSecond, 0.01f, -1000.f, 1000.f);
}

void Client::CEffect_Tool::Render_ColorDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged,
    const bool_t bHasEmissiveRadianceInput,
    const bool_t bParticleMasterNamedEmission,
	const bool_t bLockProjectTunedCarrierColor)
{
    if (!ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen))
        return;
	ImGui::BeginDisabled(bLockProjectTunedCarrierColor);
    bChanged |= DragFloat4(
        "Color Offset", Detail.Color.vColorOffset, 0.01f, -10.f, 10.f);
    bChanged |= DragFloat4(
        "Color Multiply", Detail.Color.vColorMultiply, 0.01f, 0.f, 10.f);
    bChanged |= ImGui::SliderFloat("Color Clip (Alpha Threshold)",
        &Detail.Color.fColorClip, 0.f, 1.f);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip(
            "Current v12 runtime clips pixels below this final alpha threshold.");
	ImGui::BeginDisabled(!bHasEmissiveRadianceInput);
	bChanged |= ImGui::DragFloat("Emissive Intensity (HDR)",
		&Detail.Color.fEmissiveIntensity, 0.05f, 0.f, 100.f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	if (!bHasEmissiveRadianceInput)
		ImGui::TextDisabled(
			"Emissive Intensity is inactive until the executable material exposes a radiance input.");
	else if (bParticleMasterNamedEmission)
		ImGui::TextDisabled(
			"ParticleMaster radiance uses source 02.map_e / 12.map_f. The generic Emissive card and 11.map_b are not emission lanes in this approximation.");
	else
		ImGui::TextDisabled(
			"Intensity scales the bound Emissive texture's final HDR radiance. Bloom still depends on the scene Threshold and artistic multiplier.");
    bChanged |= ImGui::DragFloat("Distortion Intensity",
        &Detail.Color.fDistortionIntensity, 0.01f, 0.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    bChanged |= ImGui::Checkbox("Distortion On Base Material",
        &Detail.Color.bDistortionOnBaseMaterial);
    bChanged |= ImGui::DragFloat("Radial Time",
        &Detail.Color.fRadialTime, 0.01f, -100.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
	bChanged |= ImGui::DragFloat("Radial Intensity",
		&Detail.Color.fRadialIntensity, 0.01f, -100.f, 100.f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	if (bLockProjectTunedCarrierColor)
	{
		ImGui::TextDisabled(
			"Carrier Color is locked for opcode 1003/1004. Tune radiance, coverage, edge, refraction, and emission in Project Tuned Surface so the typed packet remains the only color authority.");
	}
}

void Client::CEffect_Tool::Render_LinearRevealDetail(
	EFFECT_ELEMENT_DESC& Element,
	bool_t& bChanged)
{
	if (!ImGui::CollapsingHeader("Linear Reveal + Edge Glow",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}
	if (!Is_GenericLinearRevealCarrier(Element))
	{
		ImGui::TextDisabled(
			"Requires a direct-authored effect.standard Sprite or Sprite Particle with a non-Opaque profile. Source, typed, mesh, and decal carriers stay isolated.");
		return;
	}

	EFFECT_LINEAR_REVEAL_DESC& Reveal = Element.Detail.Sprite.LinearReveal;
	bool_t bEnabled = Reveal.bEnabled;
	if (ImGui::Checkbox("Enable Linear Reveal", &bEnabled))
	{
		Reveal = EFFECT_LINEAR_REVEAL_DESC{};
		Reveal.bEnabled = bEnabled;
		if (bEnabled)
		{
			const f32_t fSuggestedLife =
				Reveal.fStartSeconds + Reveal.fDurationSeconds + 0.2f;
			Element.Detail.Timing.fLifeTimeSeconds = (std::max)(
				Element.Detail.Timing.fLifeTimeSeconds, fSuggestedLife);
			if (Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE)
			{
				const f32_t fFixedLife = (std::max)(
					Element.Detail.Particle.vLifeTimeSeconds.y, fSuggestedLife);
				Element.Detail.Particle.vLifeTimeSeconds =
					{ fFixedLife, fFixedLife };
			}
		}
		bChanged = true;
	}
	if (!Reveal.bEnabled)
		return;

	static const char* const s_AxisLabels[] =
	{
		"U (horizontal raw UV)", "V (vertical raw UV)"
	};
	int32_t iAxis = static_cast<int32_t>(Reveal.eAxis);
	if (ImGui::Combo("Reveal Axis", &iAxis, s_AxisLabels,
		IM_ARRAYSIZE(s_AxisLabels)))
	{
		Reveal.eAxis = static_cast<EFFECT_LINEAR_REVEAL_AXIS>(iAxis);
		bChanged = true;
	}
	bChanged |= ImGui::Checkbox("Invert Reveal Axis", &Reveal.bInvert);
	bChanged |= ImGui::DragFloat("Reveal Start (Seconds)",
		&Reveal.fStartSeconds, 0.01f, 0.f, 30.f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	bChanged |= ImGui::DragFloat("Reveal Duration (Seconds)",
		&Reveal.fDurationSeconds, 0.01f, 0.01f, 30.f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	bChanged |= ImGui::DragFloat("Edge Half Width",
		&Reveal.fEdgeWidth, 0.001f, 0.f, 0.5f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	bChanged |= ImGui::DragFloat("Edge Softness",
		&Reveal.fSoftness, 0.001f, 0.f, 0.25f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	if (Reveal.fEdgeWidth + Reveal.fSoftness > 0.5f)
	{
		Reveal.fSoftness = (std::max)(0.f, 0.5f - Reveal.fEdgeWidth);
		bChanged = true;
	}
	bChanged |= ImGui::ColorEdit4("Edge Color", &Reveal.vEdgeColor.x,
		ImGuiColorEditFlags_Float);
	bChanged |= ImGui::DragFloat("Edge Emissive (HDR)",
		&Reveal.fEdgeEmissive, 0.05f, 0.f, 100.f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);

	const f32_t fRequiredLife =
		Reveal.fStartSeconds + Reveal.fDurationSeconds;
	if (Element.Detail.Timing.fLifeTimeSeconds < fRequiredLife)
	{
		Element.Detail.Timing.fLifeTimeSeconds = fRequiredLife;
		bChanged = true;
	}
	if (Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE)
	{
		const f32_t fFixedLife = (std::max)(
			Element.Detail.Particle.vLifeTimeSeconds.y, fRequiredLife);
		if (Element.Detail.Particle.vLifeTimeSeconds.x != fFixedLife ||
			Element.Detail.Particle.vLifeTimeSeconds.y != fFixedLife)
		{
			Element.Detail.Particle.vLifeTimeSeconds =
				{ fFixedLife, fFixedLife };
			bChanged = true;
		}
		ImGui::TextDisabled(
			"Sprite Particle reveal uses Element-local seconds and keeps Particle Life fixed for deterministic hold/dissolve timing.");
	}
	ImGui::TextDisabled(
		"Bottom to top preset: Axis V + Invert. Geometry, Size, and Velocity remain unchanged; only raw-UV coverage advances.");
}

void Client::CEffect_Tool::Render_UVDetail(
    EFFECT_DETAIL_DESC& Detail,
    bool_t& bChanged)
{
    if (!ImGui::CollapsingHeader("UV", ImGuiTreeNodeFlags_DefaultOpen))
        return;
    bChanged |= DragFloat2(
        "UV Start", Detail.UV.vStart, 0.001f, -100.f, 100.f);
    bChanged |= DragFloat2(
        "UV Speed", Detail.UV.vSpeed, 0.001f, -100.f, 100.f);
    bChanged |= ImGui::Checkbox("UV Wave", &Detail.UV.bWave);
    bChanged |= DragFloat2("Wave Amplitude",
        Detail.UV.vWaveAmplitude, 0.001f, -100.f, 100.f);
    bChanged |= ImGui::DragFloat("Wave Frequency",
        &Detail.UV.fWaveFrequency, 0.01f, 0.f, 100.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    if (Detail.Particle.bSubUVOverLife)
        ImGui::BeginDisabled();
    bChanged |= ImGui::Checkbox("UV Sequence", &Detail.UV.bSequence);
    bChanged |= ImGui::Checkbox("UV Loop", &Detail.UV.bLoop);
    bChanged |= ImGui::DragFloat("Sequence Term",
        &Detail.UV.fSequenceTerm, 0.001f, 0.001f, 60.f, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    if (Detail.Particle.bSubUVOverLife)
    {
        ImGui::EndDisabled();
        ImGui::TextDisabled("SubUV over particle life owns each particle's atlas frame.");
    }
    bool_t bTileChanged = ImGui::DragInt(
        "UV Tile Columns", &Detail.UV.iTileColumns, 0.1f, 1, 64, "%d",
        ImGuiSliderFlags_AlwaysClamp);
    bTileChanged |= ImGui::DragInt(
        "UV Tile Rows", &Detail.UV.iTileRows, 0.1f, 1, 64, "%d",
        ImGuiSliderFlags_AlwaysClamp);
    const int32_t iMaximumTile = (std::max)(
        0, Detail.UV.iTileColumns * Detail.UV.iTileRows - 1);
    if (Detail.Particle.bSubUVOverLife)
        ImGui::BeginDisabled();
    bTileChanged |= ImGui::DragInt(
        "UV Tile Index", &Detail.UV.iTileIndex, 0.1f, 0, iMaximumTile,
        "%d", ImGuiSliderFlags_AlwaysClamp);
    if (Detail.Particle.bSubUVOverLife)
        ImGui::EndDisabled();
    Detail.UV.iTileIndex = std::clamp(
        Detail.UV.iTileIndex, 0, iMaximumTile);
    bChanged |= bTileChanged;
}

void Client::CEffect_Tool::Render_UVKeyframes(
    EFFECT_ELEMENT_DESC& Element,
    bool_t& bChanged)
{
    EFFECT_UV_DESC& UV = Element.Detail.UV;
    if (!UV.bSequence || UV.iTileColumns <= 0 || UV.iTileRows <= 0)
        return;
    const auto Base = std::find_if(
        Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
        [](const EFFECT_RESOURCE_BINDING_DESC& Binding)
        {
            return Binding.strSlotId == "base";
        });
    if (Base == Element.ResourceBindings.end())
    {
        ImGui::TextDisabled("Keyframes require a Base texture.");
        return;
    }
    const CEffectThumbnailCache::RESULT Thumbnail =
        m_pThumbnailCache->Request(
            Base->strAssetId, EFFECT_RESOURCE_FILE_KIND::TEXTURE);
    if (nullptr == Thumbnail.pTextureView)
    {
        ImGui::TextDisabled("Keyframe thumbnail is loading.");
        return;
    }
    ImGui::SeparatorText("Keyframes");
    const int64_t iTotal = static_cast<int64_t>(UV.iTileColumns) * UV.iTileRows;
    const int32_t iVisible = static_cast<int32_t>((std::min<int64_t>)(iTotal, 64));
    for (int32_t iTile = 0; iTile < iVisible; ++iTile)
    {
        if (0 != iTile % 8)
            ImGui::SameLine();
        ImGui::PushID(iTile);
        const int32_t iColumn = iTile % UV.iTileColumns;
        const int32_t iRow = iTile / UV.iTileColumns;
        const ImVec2 UV0(
            static_cast<float>(iColumn) / UV.iTileColumns,
            static_cast<float>(iRow) / UV.iTileRows);
        const ImVec2 UV1(
            static_cast<float>(iColumn + 1) / UV.iTileColumns,
            static_cast<float>(iRow + 1) / UV.iTileRows);
        const ImVec4 Border = iTile == UV.iTileIndex ?
            ImVec4(0.2f, 0.75f, 1.f, 1.f) : ImVec4(0.f, 0.f, 0.f, 0.f);
        ImGui::Image(Thumbnail.pTextureView, ImVec2(42.f, 42.f),
            UV0, UV1, ImVec4(1.f, 1.f, 1.f, 1.f), Border);
        if (ImGui::IsItemClicked())
        {
            UV.iTileIndex = iTile;
            bool_t bCommitted = false;
            if (m_ActiveDocument.has_value())
            {
                EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
                for (EFFECT_ELEMENT_DESC& Active : Staged.Elements)
                {
                    if (Active.strElementId != Element.strElementId)
                        continue;
                    Active.Detail.UV.iTileIndex = iTile;
                    bCommitted = Try_CommitDocument(std::move(Staged));
                    break;
                }
            }
            if (bCommitted)
            {
				const bool_t bWasPlaying = m_bPreviewPlaying;
				Start_WorldPreviewFromBeginning();
				if (!bWasPlaying)
				{
					m_bPreviewPlaying = false;
					Set_SynchronizedAnimationPaused(true);
				}
            }
            else
                bChanged = true;
        }
        ImGui::PopID();
    }
    if (iTotal > iVisible)
        ImGui::TextDisabled("Showing the first 64 sequence tiles.");
}

void Client::CEffect_Tool::Render_TimingDetail(
	EFFECT_ELEMENT_DESC& Element,
	bool_t& bChanged)
{
	if (!ImGui::CollapsingHeader("Timing", ImGuiTreeNodeFlags_DefaultOpen))
		return;
	EFFECT_DETAIL_DESC& Detail = Element.Detail;
	bChanged |= ImGui::DragFloat("Life Time",
		&Detail.Timing.fLifeTimeSeconds, 0.01f, 0.001f, 60.f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	bChanged |= ImGui::DragFloat("Start Delay Timer",
		&Detail.Timing.fStartDelaySeconds, 0.01f, 0.f, 60.f, "%.3f",
		ImGuiSliderFlags_AlwaysClamp);
	ImGui::TextDisabled("Start Delay is relative to Effect 0 (Timeline %.3f s).",
		Resolve_EffectTimelineTime(0.f));
	const f32_t fNativeEmitterDelay = Element.SourceRecipe.bEnabled ?
		Element.SourceRecipe.fEmitterDelaySeconds : 0.f;
	const f32_t fEmitterStart =
		Detail.Timing.fStartDelaySeconds + fNativeEmitterDelay;
	ImGui::TextDisabled("Emitter start: Effect %.3f s / Timeline %.3f s",
		fEmitterStart, Resolve_EffectTimelineTime(fEmitterStart));
	const bool_t bSupportsSeparateTransformMotion =
		Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
		Resolve_AuthoringFamily(Element) ==
			EFFECT_AUTHORING_FAMILY::MESH_PARTICLE;
	if (bSupportsSeparateTransformMotion)
	{
		bool_t bSeparateTransformMotion =
			Detail.Timing.fTransformMotionDurationSeconds > 0.f;
		if (ImGui::Checkbox("Separate Transform Motion From Life",
			&bSeparateTransformMotion))
		{
			Detail.Timing.fTransformMotionDurationSeconds =
				bSeparateTransformMotion ?
				Detail.Timing.fLifeTimeSeconds : 0.f;
			bChanged = true;
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Freezes the Element root position, rotation, scale, velocity, and revolution when Motion Duration ends. Particle Initial Velocity and Acceleration keep their own particle-life clock.");
		}
		if (bSeparateTransformMotion)
		{
			f32_t& fMotionDuration =
				Detail.Timing.fTransformMotionDurationSeconds;
			if (fMotionDuration > Detail.Timing.fLifeTimeSeconds)
			{
				fMotionDuration = Detail.Timing.fLifeTimeSeconds;
				bChanged = true;
			}
			f32_t fHoldAfterMotion = (std::max)(0.f,
				Detail.Timing.fLifeTimeSeconds - fMotionDuration);
			if (ImGui::DragFloat("Motion Duration",
				&fMotionDuration, 0.01f, 0.001f,
				(std::max)(0.001f, 60.f - fHoldAfterMotion), "%.3f",
				ImGuiSliderFlags_AlwaysClamp))
			{
				Detail.Timing.fLifeTimeSeconds =
					fMotionDuration + fHoldAfterMotion;
				bChanged = true;
			}
			const f32_t fMaximumHold = (std::max)(
				0.f, 60.f - fMotionDuration);
			if (ImGui::DragFloat("Hold After Motion", &fHoldAfterMotion,
				0.01f, 0.f, fMaximumHold, "%.3f",
				ImGuiSliderFlags_AlwaysClamp))
			{
				Detail.Timing.fLifeTimeSeconds =
					fMotionDuration + fHoldAfterMotion;
				bChanged = true;
			}
			ImGui::TextDisabled("Motion End: %.3f s",
				Detail.Timing.fStartDelaySeconds + fMotionDuration);
		}
	}
	if (Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
		Element.eKind == EFFECT_ELEMENT_KIND::SPRITE)
	{
		bChanged |= ImGui::DragFloat("After Image Timer",
			&Detail.Timing.fAfterImageSeconds, 0.01f, 0.f, 60.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
	}
	if (Is_SourceParticleCarrier(Element))
	{
		ImGui::TextDisabled("Native emitter delay: +%.3f s", fNativeEmitterDelay);
		ImGui::TextDisabled(
			"Life Time controls root motion and the emission-window fallback; SourceRecipe owns native particle life and spawning.");
		const f32_t fMotionDuration =
			Detail.Timing.fTransformMotionDurationSeconds > 0.f ?
				Detail.Timing.fTransformMotionDurationSeconds :
				Detail.Timing.fLifeTimeSeconds;
		if (Detail.LinearLerp.bScale && fNativeEmitterDelay >= fMotionDuration)
		{
			ImGui::TextWrapped(
				"Scaling Lerp finishes before the native emitter starts. Increase Motion Duration or Life Time to keep scaling during visible playback.");
		}
	}
	else if (Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE)
	{
		ImGui::TextDisabled("Particle lifetime: %.3f - %.3f s after spawn",
			Detail.Particle.vLifeTimeSeconds.x,
			Detail.Particle.vLifeTimeSeconds.y);
		ImGui::TextDisabled(
			"Life Time controls root motion / emission; a single burst ends when its particles expire.");
	}
	bChanged |= ImGui::SliderFloat("Dissolve Start",
		&Detail.Timing.fDissolveStartNormalized, 0.f, 1.f);
}

void Client::CEffect_Tool::Render_SizeDetail(
	EFFECT_ELEMENT_DESC& Element,
	bool_t& bChanged)
{
	const auto RenderDecalReceiver = [&Element, &bChanged]()
	{
		EFFECT_DECAL_DETAIL_DESC& Decal = Element.Detail.Decal;
		static const char* const s_ReceiverLabels[] =
		{
			"All Opaque (legacy)", "Upward Surfaces"
		};
		int32_t iReceiver = static_cast<int32_t>(Decal.eReceiverMode);
		if (ImGui::Combo("Receiver", &iReceiver, s_ReceiverLabels,
			IM_ARRAYSIZE(s_ReceiverLabels)))
		{
			Decal.eReceiverMode =
				static_cast<EFFECT_DECAL_RECEIVER_MODE>(iReceiver);
			Decal.fNormalCutoff = Decal.eReceiverMode ==
				EFFECT_DECAL_RECEIVER_MODE::UPWARD_SURFACES ? 0.75f : -1.f;
			bChanged = true;
		}
		ImGui::BeginDisabled(Decal.eReceiverMode !=
			EFFECT_DECAL_RECEIVER_MODE::UPWARD_SURFACES);
		bChanged |= ImGui::SliderFloat("Upward Normal Cutoff",
			&Decal.fNormalCutoff, 0.f, 1.f, "%.2f");
		ImGui::EndDisabled();
		bChanged |= ImGui::SliderFloat("Projection Edge Fade",
			&Decal.fEdgeFade, 0.f, 1.f, "%.2f");
		ImGui::TextDisabled(
			"Upward compares the GBuffer world normal with this projector's local +Y. Tight Projection Depth is still required to avoid horizontal actor surfaces.");
	};
	const bool_t bSourceParticleCarrier =
		Is_SourceParticleCarrier(Element);
	const bool_t bHasSizeSurface =
		Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
		Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE ||
		Element.eKind == EFFECT_ELEMENT_KIND::DECAL ||
		Element.eKind == EFFECT_ELEMENT_KIND::TRAIL ||
		bSourceParticleCarrier;
	if (!bHasSizeSurface ||
		!ImGui::CollapsingHeader(bSourceParticleCarrier ?
			"Source Playback Tuning###SizeDetail" : "Size###SizeDetail",
			ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}
	if (bSourceParticleCarrier)
	{
		if (Element.SourceRecipe.strRendererShape == "mesh")
		{
			ImGui::SeparatorText("Mesh Carrier Geometry");
			bChanged |= ImGui::DragFloat("Model Import Scale",
				&Element.Detail.Mesh.fModelPreScale, 0.001f, 0.0001f, 100.f,
				"%.4f", ImGuiSliderFlags_AlwaysClamp);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Applied once to the WModel carrier. It is independent from the source particle Size multiplier below.");
			}
		}
		ImGui::SeparatorText("Evaluated Source Multipliers");
		EFFECT_PARTICLE_SOURCE_SCALE_DESC& Tuning =
			Element.Detail.Particle.SourceScale;
		bChanged |= ImGui::DragFloat("Count x", &Tuning.fCount,
			0.01f, 0.01f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Scales source rate, fixed bursts, and the particle ceiling. SpawnPerUnit and event-receiver counts remain module-owned.");
		}
		bChanged |= ImGui::DragFloat("Size x", &Tuning.fSize,
			0.01f, 0.01f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Size x is a constant source multiplier (editor range 0.01-16); zero is invalid.\n"
				"For growth, set Transform > Scaling to a positive start (editor minimum 0.001), then enable Linear Lerp > Lerp Scaling and set Scaling End.\n"
				"Keep Timing > Motion Duration (or Life Time) longer than the native emitter delay so scaling continues after particles appear.");
		}
		bChanged |= ImGui::DragFloat("Life x", &Tuning.fLifeTime,
			0.01f, 0.01f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Life x multiplies each source particle lifetime; 16 means 16 times, not 16 seconds.\n"
				"The editor and saved-document validator both allow up to 16x. Timing > Life Time is a separate duration in seconds.");
		}
		bChanged |= ImGui::DragFloat("Speed x", &Tuning.fSpeed,
			0.01f, -16.f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Scales source spawn/base velocity. A later absolute VelocityOverLife or vector-field module may still own the final velocity.");
		}
		bChanged |= ImGui::DragFloat("Rotation x", &Tuning.fRotation,
			0.01f, -16.f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		bChanged |= ImGui::DragFloat("Alpha x", &Tuning.fAlpha,
			0.01f, 0.f, 16.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Scales source base alpha. A later absolute ColorOverLife module may still own the final alpha.");
		}
		ImGui::TextDisabled(
			"These multipliers tune supported source carrier axes without replacing its modules. Use Timing > Start Delay to place the Element inside the clip; Start/End Size and raw spawn fields belong to a manual Element.");
		if (Element.eKind == EFFECT_ELEMENT_KIND::DECAL)
		{
			ImGui::SeparatorText("Decal Projection");
			bChanged |= ImGui::DragFloat("Projection Depth",
				&Element.Detail.Decal.fDepth, 0.01f, 0.001f, 1000.f, "%.3f",
				ImGuiSliderFlags_AlwaysClamp);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Projection depth is a working Decal overlay. Source Size x owns the projected width and height.");
			}
			RenderDecalReceiver();
		}
		if (!Tuning.Is_Default() &&
			ImGui::Button("Reset Source Playback Tuning"))
		{
			Tuning = EFFECT_PARTICLE_SOURCE_SCALE_DESC{};
			bChanged = true;
		}
		return;
	}
	if (Element.eKind == EFFECT_ELEMENT_KIND::MESH ||
		(Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			Resolve_AuthoringFamily(Element) ==
				EFFECT_AUTHORING_FAMILY::MESH_PARTICLE))
	{
		bChanged |= ImGui::DragFloat("Model Import Scale",
			&Element.Detail.Mesh.fModelPreScale, 0.001f, 0.0001f, 100.f,
			"%.4f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Applied once when the WModel is loaded. Valtan Effect WModels commonly use 0.01; particle size remains a separate authored or source-tuning axis.");
		}
	}
	if (Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE)
	{
		bChanged |= DragFloat2("Start Size",
			Element.Detail.Particle.vStartSize, 0.01f, 0.001f, 100.f);
		bChanged |= DragFloat2("End Size",
			Element.Detail.Particle.vEndSize, 0.01f, 0.f, 100.f);
	}
	else if (Element.eKind == EFFECT_ELEMENT_KIND::DECAL)
	{
		bChanged |= DragFloat2("Decal Size", Element.Detail.Decal.vSize,
			0.01f, 0.001f, 1000.f);
		bChanged |= ImGui::DragFloat("Projection Depth",
			&Element.Detail.Decal.fDepth, 0.01f, 0.001f, 1000.f, "%.3f",
			ImGuiSliderFlags_AlwaysClamp);
		RenderDecalReceiver();
	}
	else if (Element.eKind == EFFECT_ELEMENT_KIND::TRAIL)
	{
		bChanged |= ImGui::DragFloat("Start Width",
			&Element.Detail.Trail.fStartWidth, 0.01f, 0.001f, 100.f,
			"%.3f", ImGuiSliderFlags_AlwaysClamp);
		bChanged |= ImGui::DragFloat("End Width",
			&Element.Detail.Trail.fEndWidth, 0.01f, 0.f, 100.f,
			"%.3f", ImGuiSliderFlags_AlwaysClamp);
	}
}
