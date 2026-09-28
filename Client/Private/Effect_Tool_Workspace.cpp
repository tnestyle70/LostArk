#include "imgui.h"

#include "Effect_Tool_Internal.h"
#include "ClassSelectionTimeline.h"
#include "EffectAuthoringResourceTree.h"
#include "EffectAuthoringSequencer.h"
#include "Effect_DocumentCodec.h"
#include "KoukuSaydonCompositionDocument.h"
#include "Effect_Object.h"
#include "Effect_VisualProgramCorpus.h"
#include "EffectV2_Catalog.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <utility>

namespace Client
{
namespace
{
constexpr const wchar_t* AUTHORING_LAYER = L"Layer_EffectAuthoringSequencer";
std::string Parent_Key(const EFFECT_RESOURCE_KEY& key)
{ return std::to_string(static_cast<int>(key.eOwnerKind)) + ":" + key.strStableId; }
}

bool CEffect_Tool::Build_KoukuPatternPreviewContext(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const std::string& patternId, const std::string& occurrenceId, const std::string& effectId,
    EFFECT_TOOL_KOUKU_PATTERN_PREVIEW& context, std::string& status)
{
    status.clear();
    const auto pattern = std::find_if(document.Patterns.begin(), document.Patterns.end(),
        [&](const auto& row) { return row.strPatternId == patternId; });
    if (pattern == document.Patterns.end()) return false;
    std::set<std::string> resources;
    for (const auto& resource : document.PresentationResources)
        if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && resource.strAssetId == effectId &&
            (resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT"))
            resources.insert(resource.strResourceId);
    std::vector<const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE*> matches;
    const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE* selected = nullptr;
    for (const auto& row : pattern->PresentationOccurrences)
        if (resources.contains(row.strResourceId))
        {
            matches.push_back(&row);
            if (row.strOccurrenceId == occurrenceId) selected = &row;
        }
    if (matches.empty()) return false;
    if (!selected && matches.size() == 1u) selected = matches.front();
    const auto fallback = [&](const char* reason)
    { status = std::string("Current Pattern preview unavailable: ") + reason + " Preview selection preserved."; return false; };
    if (!selected) return fallback("select one of this Effect's Pattern boxes.");
    if (!pattern->strLoadError.empty()) return fallback("the selected Pattern is invalid.");
    if (!pattern->PatternOccurrences.empty() || !pattern->AnimationBlendWindows.empty())
        return fallback("nested Patterns and animation blends require their Composition preview.");
    if (!selected->iDurationMs || selected->iStartMs > 600000u ||
        selected->iDurationMs > 600000u - selected->iStartMs)
        return fallback("the selected box has an invalid timing window.");
    EFFECT_TOOL_KOUKU_PATTERN_PREVIEW staged;
    staged.strEffectAssetId = effectId;
    staged.strPatternId = pattern->strPatternId;
    staged.strOccurrenceId = selected->strOccurrenceId;
    staged.iEffectStartMs = selected->iStartMs;
    staged.iDurationMs = selected->iDurationMs;
    staged.bLoopEffectToDuration = selected->bLoopEffectToDuration;
    auto& source = staged.SourceModelPreview;
    source.strGateId = pattern->strGateId;
    source.strActorProfileId = pattern->strActorProfileId;
    source.strTargetBossPlacementId = pattern->strTargetBossPlacementId;
    std::uint32_t stageStartMs = 0u;
    for (const auto& stage : pattern->Stages)
    {
        if (stage.iDurationMs > 600000u - stageStartMs)
            return fallback("the stage duration exceeds the animation limit.");
        for (const auto& animation : stage.AnimationOccurrences)
        {
            if (animation.iStartOffsetMs > stage.iDurationMs || !animation.iPlayMs ||
                animation.iPlayMs > stage.iDurationMs - animation.iStartOffsetMs ||
                animation.iSourceEndMs || animation.iBlendInMs || animation.iPoseStartMs != UINT32_MAX)
                return fallback("trimmed or blended animation needs its Composition preview.");
            EFFECT_SOURCE_MODEL_ANIMATION row;
            row.strRuntimeClip = animation.strRuntimeClip;
            row.iStartOffsetMs = stageStartMs + animation.iStartOffsetMs;
            row.iSourceStartMs = animation.iSourceStartMs;
            row.iPlayMs = animation.iPlayMs;
            row.fPlayRate = animation.fPlayRate;
            row.strEndPolicy = animation.strEndPolicy;
            source.Animations.push_back(std::move(row));
        }
        stageStartMs += stage.iDurationMs;
    }
    if (source.Animations.empty()) return fallback("the selected Pattern has no animation.");
    std::stable_sort(source.Animations.begin(), source.Animations.end(),
        [](const auto& left, const auto& right) { return left.iStartOffsetMs < right.iStartOffsetMs; });
    context = std::move(staged);
    status = "Current Pattern " + context.strPatternId + " / " + context.strOccurrenceId +
        ": animation at " + std::to_string(context.iEffectStartMs) + " ms, window " +
        std::to_string(context.iDurationMs) + " ms; source speed preserved.";
    return true;
}

void CEffect_Tool::Resolve_KoukuPatternPreviewContext(EFFECT_DOCUMENT_DESC& preview,
    std::optional<std::uint32_t>& durationMs, std::uint32_t& modelStartMs, bool& loopEffectToDuration)
{
    durationMs.reset(); modelStartMs = 0u; loopEffectToDuration = false;
    m_strKoukuPatternPreviewStatus.clear();
    // The resource browser opens the Effect's saved source animation. An old
    // Composition selection must not silently replace a shared Effect's clip.
    if (!m_KoukuPatternPreviewContext ||
        m_KoukuPatternPreviewContext->strEffectAssetId != preview.strEffectAssetId) return;
    const auto& context = *m_KoukuPatternPreviewContext;
    const auto savedSource = preview.SourceModelPreview;
    preview.SourceModelPreview = context.SourceModelPreview;
    std::string error;
    if (!CEffectDocumentCodec::Validate(preview, error))
    {
        preview.SourceModelPreview = savedSource;
        m_strKoukuPatternPreviewStatus = "Current Pattern animation is unsupported: " + error +
            " Using the saved source animation.";
        return;
    }
    durationMs = context.iDurationMs;
    modelStartMs = context.iEffectStartMs;
    loopEffectToDuration = context.bLoopEffectToDuration;
    m_strKoukuPatternPreviewStatus = "Selected Pattern " + context.strPatternId + " / " + context.strOccurrenceId +
        ": animation at " + std::to_string(modelStartMs) + " ms, window " +
        std::to_string(*durationMs) + " ms; source speed preserved.";
}

void CEffect_Tool::Configure_AuthoringWorkspace(CKoukuSaydonPresentationPlayer* player)
{
    if (!m_pAuthoringResources) m_pAuthoringResources = std::make_unique<CEffectAuthoringResourceTree>(EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT);
    if (!m_pAuthoringSequencer)
    {
        m_pAuthoringSequencer = Create_CompositionSequencer("effect.sequence.default");
    }
    m_pAuthoringSequencer->Set_Player(player);
}

shared_ptr<CEffectAuthoringSequencer> CEffect_Tool::Create_CompositionSequencer(const char* sequenceId)
{
        auto sequencer = std::make_shared<CEffectAuthoringSequencer>(m_pDevice, m_pContext, m_pCharacterPreviewPanel, sequenceId);
        sequencer->Set_V1Callbacks(
            [this](const EFFECT_RESOURCE_KEY& key, const std::vector<std::string>& elementIds, const float4x4_t& root, std::shared_ptr<CEffectObject>& object, uint32_t& previewStartMs, uint32_t& previewEndMs, std::string& error)
            { return Create_AuthoringOccurrence(key, elementIds, root, object, previewStartMs, previewEndMs, error); },
            [this](const std::shared_ptr<CEffectObject>& object)
            {
                const auto found = m_AuthoringOccurrenceLevels.find(object.get());
                if (found == m_AuthoringOccurrenceLevels.end()) return;
                object->Set_Visible(false);
                CGameInstance::Get().Remove_GameObject_from_Layer(found->second, AUTHORING_LAYER, object);
                m_AuthoringOccurrenceDocuments.erase(object.get());
                m_AuthoringOccurrenceLevels.erase(found);
            });
        sequencer->Set_V1AnchorProvider(
            [this](const std::shared_ptr<CEffectObject>& object, const float4x4_t& root, bool useKouku, float seconds,
                std::unordered_map<std::string, float4x4_t>& anchors, std::string& error)
            { return Resolve_AuthoringSourceAnchors(object, root, useKouku, seconds, anchors, error); });
        sequencer->Set_V2SnapshotProvider(
            [this](const EFFECT_RESOURCE_KEY& key, std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>& snapshot, std::string& error)
            {
                return CEffectV2Catalog::Get().Load_ResourceSnapshot(
                    key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V2_GROUP ? EFFECT_V2_RESOURCE_KIND::GROUP : EFFECT_V2_RESOURCE_KIND::LEAF,
                    key.strStableId, snapshot, error);
            });
    return sequencer;
}

void CEffect_Tool::Set_AuthoringPlayer(CKoukuSaydonPresentationPlayer* player)
{
    if (m_pAuthoringSequencer) m_pAuthoringSequencer->Set_Player(player);
}
void CEffect_Tool::Set_AuthoringCamera(const shared_ptr<Engine::CCamera>& camera)
{
    if (m_pAuthoringSequencer) m_pAuthoringSequencer->Set_Camera(camera);
}
void CEffect_Tool::Update_AuthoringWorkspace(float dt, bool active)
{
    if (!m_pAuthoringSequencer) return;
    const bool wasPlaying = m_pAuthoringSequencer->Is_Active();
    const std::string previousStatus = m_pAuthoringSequencer->Status();
    m_pAuthoringSequencer->Update(dt, active);
    // A failure after the initial frame must remain visible in the Effect browser too.
    if (wasPlaying && m_pAuthoringSequencer->Status() != previousStatus &&
        !m_pAuthoringSequencer->Status().empty())
        m_strPreviewStatus = m_pAuthoringSequencer->Status();
}

bool CEffect_Tool::Update_AuthoringPlacementInput(const bool active)
{ return m_pAuthoringSequencer && m_pAuthoringSequencer->Update_PreviewPlacementInput(active); }

void CEffect_Tool::Deactivate_AuthoringWorkspace()
{
    (void)End_ClassMovieEditing();
    if (m_pAuthoringSequencer) m_pAuthoringSequencer->Stop();
    m_bPreviewPlaying = false;
    Release_WorldPreview(true);
}
bool CEffect_Tool::Consume_AuthoringInteraction()
{ return m_pAuthoringSequencer && m_pAuthoringSequencer->Consume_InteractionRequest(); }

bool CEffect_Tool::Open_AuthoringResource(const EFFECT_RESOURCE_KEY& key)
{
    if (key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT && key.strStableId.starts_with("effect.kouku."))
    {
        std::filesystem::path path;
        if (!Resolve_SavedKoukuEffectSource(key.strStableId, path, m_strDocumentStatus)) return false;
        return Try_LoadDocumentPath(path, EFFECT_DOCUMENT_SOURCE::AUTHORED, key.strStableId,
            EFFECT_DOCUMENT_PREVIEW_INTENT::SYNCHRONIZED_PRODUCT);
    }
    if (key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT && key.strStableId.starts_with("effect.valtan."))
    {
        Initialize_CatalogMetadataView();
        const auto* path = Resolve_DirectAuthoredEditablePath(key.strStableId, m_strDocumentStatus);
        if (!path)
        {
            // Composition shares the physical All Effects inventory, including
            // documents saved after this Tool's first metadata snapshot.
            if (!Refresh_DataFiles()) return false;
            path = Resolve_DirectAuthoredEditablePath(key.strStableId, m_strDocumentStatus);
        }
        if (!path) return false;
        // Full Restore loads its exact source clock before the unsaved guard,
        // and carries it through the existing pending-document transaction.
        const bool opened = Try_OpenValtanStandaloneEffect(*path, key.strStableId);
        if (!opened && !m_strPreviewStatus.empty()) m_strDocumentStatus = m_strPreviewStatus;
        return opened;
    }
    if (key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT)
        return Try_LoadDocument(key.strStableId);
    if (key.Is_Valid())
    {
        m_PendingTypedEffectResourceOpen = key;
        return true;
    }
    m_strDocumentStatus = "Effect open rejected an invalid owner or ID.";
    return false;
}

bool CEffect_Tool::Preview_AuthoringResource(const EFFECT_RESOURCE_KEY& key, std::string& status)
{
    if (!key.Is_Valid() || !m_pAuthoringSequencer)
    {
        status = "Effect preview requires a valid saved resource and its authoring workspace.";
        return false;
    }
    if (key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT &&
        (key.strStableId.starts_with("effect.kouku.") || key.strStableId.starts_with("effect.valtan.")))
    {
        const bool active = m_ActiveDocument && m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED &&
            m_ActiveDocument->strEffectAssetId == key.strStableId;
        if (active || Open_AuthoringResource(key))
        {
            const bool played = Try_PlayActiveUnifiedEffect();
            status = m_strPreviewStatus;
            return played;
        }
        if (m_PendingDocumentLoad && m_PendingDocumentLoad->strSelectionId == key.strStableId)
        {
            m_PendingDocumentLoad->strElementSelectionId.clear();
            m_PendingDocumentLoad->strModelCueSelectionId.clear();
            m_PendingDocumentLoad->bPlayCompleteAfterLoad = true;
        }
        status = m_strDocumentStatus;
        return false;
    }
    const uint32_t duration = m_ActiveDocument && key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT &&
            m_ActiveDocument->strEffectAssetId == key.strStableId ?
            static_cast<uint32_t>((std::max)(0.001f, m_fPreviewDurationSeconds) * 1000.f) : 3000u;
    const bool played = m_pAuthoringSequencer->Preview(key, duration);
    status = m_pAuthoringSequencer->Status();
    if (played) Release_WorldPreview(true);
    return played;
}

void CEffect_Tool::Render_AuthoringResourceTree()
{
    ImGui::SetNextWindowPos(ImVec2(1110.f, 35.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430.f, 660.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Effect Resources")) { ImGui::End(); return; }
    if (m_pAuthoringSequencer) m_pAuthoringSequencer->Render_PreviewPlacementControls();
    m_pAuthoringResources->Set_V1CopySource(
        m_ActiveDocument ? m_ActiveDocument->strEffectAssetId : std::string{},
        m_ActiveDocument ? m_ActiveDocument->strDisplayName : std::string{});
    m_pAuthoringResources->Render();
    CEffectAuthoringResourceTree::COMMAND command;
    while (m_pAuthoringResources->Take_Command(command))
    {
        const EFFECT_RESOURCE_KEY key{command.eKind, command.strAssetId};
        if (command.eCommand == CEffectAuthoringResourceTree::COMMAND_KIND::OPEN)
        {
            m_AuthoringParents[Parent_Key(key)] = command.strParentId;
            if (Open_AuthoringResource(key)) m_strAuthoringParentId = command.strParentId;
            else m_pAuthoringResources->Set_Status(m_strDocumentStatus);
        }
        else if (command.eCommand == CEffectAuthoringResourceTree::COMMAND_KIND::CREATE_V1_COPY)
        {
            if (!m_ActiveDocument ||
                m_ActiveDocument->strEffectAssetId != command.strSourceAssetId)
                m_strDocumentStatus = "The selected V1 copy source changed; select it again before copying.";
            else
                (void)Try_SaveDocumentAs(command.strAssetId, command.strDisplayName, command.strParentId);
            m_pAuthoringResources->Set_Status(m_strDocumentStatus);
        }
        else if (command.eCommand == CEffectAuthoringResourceTree::COMMAND_KIND::CREATE_EFFECT)
        {
            bool created = false;
            if (command.eKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT)
            {
                if (Has_UnsavedWork()) m_strDocumentStatus = "Save or discard the current V1 Effect before creating another Effect.";
                else
                {
                    std::snprintf(m_NewAssetId.data(), m_NewAssetId.size(), "%s", command.strAssetId.c_str());
                    std::snprintf(m_NewDisplayName.data(), m_NewDisplayName.size(), "%s", command.strDisplayName.c_str());
                    created = Try_CreateDocument();
                }
                m_pAuthoringResources->Set_Status(m_strDocumentStatus);
            }
            if (created)
            {
                m_strAuthoringParentId = command.strParentId;
                const auto& createdId = m_ActiveDocument->strEffectAssetId;
                m_AuthoringParents[Parent_Key({command.eKind, createdId})] = command.strParentId;
            }
        }
        else if (command.eCommand == CEffectAuthoringResourceTree::COMMAND_KIND::PREVIEW)
        {
            std::string status;
            (void)Preview_AuthoringResource(key, status);
            m_pAuthoringResources->Set_Status(std::move(status));
        }
        else if (m_pAuthoringSequencer)
        {
            Release_WorldPreview(true);
            const uint32_t duration = m_ActiveDocument && key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT &&
                    m_ActiveDocument->strEffectAssetId == key.strStableId ?
                    static_cast<uint32_t>((std::max)(0.001f, m_fPreviewDurationSeconds) * 1000.f) : 3000u;
            const bool ok = command.eCommand == CEffectAuthoringResourceTree::COMMAND_KIND::APPEND ?
                m_pAuthoringSequencer->Append(key, duration) : m_pAuthoringSequencer->Preview(key, duration);
            (void)ok;
            m_pAuthoringResources->Set_Status(m_pAuthoringSequencer->Status());
        }
    }
    ImGui::End();
}



void CEffect_Tool::Render_AuthoringCommands()
{
    if (Has_ClassMovieContext()) return;
    if (!m_pAuthoringSequencer) return;
    if (m_ActiveDocument && m_ActiveDocument->strEffectAssetId.starts_with("effect.kouku."))
    {
        if (m_KoukuPatternPreviewContext &&
            m_KoukuPatternPreviewContext->strEffectAssetId == m_ActiveDocument->strEffectAssetId)
        {
            ImGui::TextWrapped("Animation: Pattern %s / %s", m_KoukuPatternPreviewContext->strPatternId.c_str(),
                m_KoukuPatternPreviewContext->strOccurrenceId.c_str());
            if (ImGui::SmallButton("Use saved source animation"))
            {
                Deactivate_AuthoringWorkspace();
                m_KoukuPatternPreviewContext.reset();
                m_strKoukuPatternPreviewStatus.clear();
                Recalculate_PreviewDuration();
                m_strPreviewStatus = "Saved source animation selected. Play All or Play Group to preview.";
            }
        }
        else
        {
            ImGui::TextDisabled("Animation: saved Effect source");
            if (m_ActiveDocument->SourceModelPreview)
                for (const auto& animation : m_ActiveDocument->SourceModelPreview->Animations)
                    ImGui::TextDisabled("  %s", animation.strRuntimeClip.c_str());
        }
        ImGui::BeginDisabled(!m_KoukuPatternPreviewProvider);
        if (ImGui::SmallButton("Use current Pattern animation"))
        {
            EFFECT_TOOL_KOUKU_PATTERN_PREVIEW context;
            std::string status;
            if (m_KoukuPatternPreviewProvider(m_ActiveDocument->strEffectAssetId, context, status))
            {
                EFFECT_DOCUMENT_DESC preview = *m_ActiveDocument;
                preview.SourceModelPreview = context.SourceModelPreview;
                if (context.strEffectAssetId == preview.strEffectAssetId && CEffectDocumentCodec::Validate(preview, status))
                {
                    Deactivate_AuthoringWorkspace();
                    m_KoukuPatternPreviewContext = std::move(context);
                    m_fPreviewDurationSeconds = static_cast<float>(m_KoukuPatternPreviewContext->iDurationMs) * .001f;
                    m_strKoukuPatternPreviewStatus.clear();
                    m_strPreviewStatus = "Pattern animation selected: " + m_KoukuPatternPreviewContext->strPatternId +
                        " / " + m_KoukuPatternPreviewContext->strOccurrenceId + ". Play All or Play Group to preview.";
                }
                else m_strPreviewStatus = "Pattern animation was not changed: " + status;
            }
            else m_strPreviewStatus = status.empty() ?
                "Select this Effect's box in a boss Pattern, then use its animation here. Current preview preserved." : status;
        }
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Use the selected Pattern box's animation and lifetime for this preview. Reopening an Effect restores its saved source animation.");
    }
    EFFECT_RESOURCE_KEY selected;
    if (m_ActiveDocument) selected = {EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, m_ActiveDocument->strEffectAssetId};
    ImGui::BeginDisabled(!selected.Is_Valid());
    if (ImGui::Button("Append##EffectSequence"))
    {
        Release_WorldPreview(true);
        const uint32_t duration = static_cast<uint32_t>((std::max)(0.001f, m_fPreviewDurationSeconds) * 1000.f);
        (void)m_pAuthoringSequencer->Append(selected, duration);
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("Append this Effect at the Sequencer cursor. Play All, Family and Element buttons keep their preview scopes.");
    if (!m_pAuthoringSequencer->Status().empty()) ImGui::TextWrapped("%s", m_pAuthoringSequencer->Status().c_str());
    ImGui::Separator();
}

void CEffect_Tool::Attach_AuthoringSaved()
{
    if (!m_pAuthoringResources || !m_ActiveDocument) return;
    std::string status;
    if (!m_pAuthoringResources->Attach_Saved(EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT,
        m_ActiveDocument->strEffectAssetId, m_ActiveDocument->strDisplayName,
        m_AuthoringParents[Parent_Key({EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, m_ActiveDocument->strEffectAssetId})], status))
        m_strDocumentStatus += " Effect saved; tree placement needs retry: " + status;
}

bool CEffect_Tool::Resolve_AuthoringOccurrenceDocument(const EFFECT_RESOURCE_KEY& key,
    EFFECT_DOCUMENT_DESC& document, std::string& error) const
{
    if (key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT || !key.Is_Valid())
    { error = "Invalid V1 Effect reference."; return false; }
    // An explicit staged document already contains the intended Detail/Model edits.
    // Reapplying the old active draft here would overwrite the transaction candidate.
    if (m_pAuthoringRefreshDocument && m_pAuthoringRefreshDocument->strEffectAssetId == key.strStableId)
        document = *m_pAuthoringRefreshDocument;
    else if (m_ActiveDocument && m_ActiveDocument->strEffectAssetId == key.strStableId)
    {
        document = *m_ActiveDocument;
        if (m_bParticleSystemDraftDirty && !Apply_ParticleSystemDraft(document))
        { error = "The particle-system draft could not be applied."; return false; }
        if (m_bDetailDraftDirty && !Apply_DetailDraft(document))
        { error = m_strDetailStatus; return false; }
        if (m_bModelCueDraftDirty && !Apply_ModelCueDraft(document))
        { error = "The model-cue draft could not be applied."; return false; }
    }
    else
    {
        const auto path = CProjectDataRoot::Resolve(std::filesystem::path(L"Effects") / L"Authored" /
            (key.strStableId + ".effect.json"));
        if (!CEffectDocumentCodec::Load(path, document, error)) return false;
    }
    if (document.strEffectAssetId != key.strStableId)
    { error = "Saved Effect identity does not match the selected row."; return false; }
    return true;
}

bool CEffect_Tool::Create_AuthoringOccurrence(const EFFECT_RESOURCE_KEY& key, const std::vector<std::string>& elementIds, const float4x4_t& root,
    std::shared_ptr<CEffectObject>& object, uint32_t& previewStartMs, uint32_t& previewEndMs, std::string& error)
{
    EFFECT_DOCUMENT_DESC document;
    if (!Resolve_AuthoringOccurrenceDocument(key, document, error)) return false;
    if (!elementIds.empty())
    {
        if (!CEffectDocumentCodec::Validate_Drawable(document, error)) return false;
        EFFECT_DOCUMENT_DESC selected;
        if (!Build_ElementsPreviewDocument(document, elementIds, selected, error)) return false;
        std::string label;
        if (!Resolve_ElementsPreviewWindow(selected, elementIds, previewStartMs, previewEndMs, label, error)) return false;
        document = std::move(selected);
        std::erase_if(document.RuntimeExtensions.BakedEdgeHistories, [&document](const auto& history)
        {
            return std::none_of(document.Elements.begin(), document.Elements.end(),
                [&history](const auto& element)
                { return element.RuntimeCarrier.strHistoryId == history.strHistoryId; });
        });
    }
    if (!CEffectDocumentCodec::Validate_Drawable(document, error)) return false;
    const auto immutableDocument = std::make_shared<const EFFECT_DOCUMENT_DESC>(std::move(document));
    std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION> projection;
    std::shared_ptr<const CEffectDocumentRenderer::PREPARED_DOCUMENT> prepared;
    // Whole-document and selected previews use the same validated payload.
    // An ordinary v15 document does not need a runtime-carrier projection.
    if (CEffectDocumentCodec::Requires_DocumentOwnedRuntimeProjection(*immutableDocument))
    {
        if (!CEffectVisualProgramCorpusCodec::Create_DocumentOwnedRuntimeProjection(
            immutableDocument, projection, error) || !projection || !projection->Is_Valid() ||
            projection->Get_DocumentShared().get() != immutableDocument.get())
        {
            if (error.empty()) error = "The authored Effect runtime projection is invalid.";
            return false;
        }
        if (!CEffectDocumentRenderer::Prepare_VisualProgramDocument(
            m_pDevice, m_pContext, projection, prepared, error) || !prepared)
        {
            if (error.empty()) error = "The authored Effect runtime resources could not be prepared.";
            return false;
        }
    }
    const uint32_t level = CGameInstance::Get().Get_CurrentLevelID();
    CEffectObject::EFFECT_OBJECT_DESC desc{};
    desc.RootWorld = root; desc.bAutoPlay = false;
    std::shared_ptr<CGameObject> clone;
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::STATIC),
        L"Prototype_GameObject_EffectObject", level, AUTHORING_LAYER, &desc, &clone)))
    { error = "EffectObject prototype is not available in this level."; return false; }
    auto staged = std::dynamic_pointer_cast<CEffectObject>(clone);
    const bool isStaged = staged && (projection ?
        staged->Stage_PrevalidatedVisualProgramDocument(projection, prepared, error) :
        staged->Stage_Document(*immutableDocument, error));
    if (!isStaged || (!elementIds.empty() && !staged->Set_SubmissionElementSet(elementIds, error)))
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(level, AUTHORING_LAYER, clone);
        return false;
    }
    staged->Set_RootWorld(root); staged->Set_Playing(false); staged->Set_Visible(false);
    m_AuthoringOccurrenceLevels.emplace(staged.get(), level);
    m_AuthoringOccurrenceDocuments.emplace(staged.get(), immutableDocument);
    object = std::move(staged);
    return true;
}

bool CEffect_Tool::Is_AuthoringWorldResource(const std::string& id, EFFECT_RESOURCE_FILE_KIND kind) const
{
    if (!m_bAuthoringWorldLoaded || id.empty()) return false;
    return std::any_of(m_AuthoringWorldObjects.begin(), m_AuthoringWorldObjects.end(), [&](const auto& row)
    {
        return row.error.empty() && ((kind == EFFECT_RESOURCE_FILE_KIND::MODEL && row.resource.modelAssetId == id) ||
            (kind == EFFECT_RESOURCE_FILE_KIND::TEXTURE && row.resource.diffuseTextureAssetId == id));
    });
}

bool CEffect_Tool::Render_WorldObjectResourceGrid(bool draft)
{
    if (!m_pAuthoringResources) return false;
    ImGui::PushID(draft ? "WorldObjectSeed" : "WorldObjectBinding");
    ImGui::Combo("Resource Category", &m_iAuthoringResourceSource, "Effect Assets\0World Object\0");
    if (m_iAuthoringResourceSource != 1) { ImGui::PopID(); return false; }
    if (!m_bAuthoringWorldLoaded || ImGui::Button("Refresh World Objects"))
    {
        uint32_t revision = 0;
        std::vector<EFFECT_COMPOSITION_WORLD_RESOURCE> staged;
        if (Read_EffectCompositionWorldResources("LV_LUT_MIDNIGHTC_ED", staged, revision, m_strAuthoringWorldStatus))
            m_AuthoringWorldObjects = std::move(staged);
        m_bAuthoringWorldLoaded = true;
    }
    ImGui::TextWrapped("%s", m_strAuthoringWorldStatus.c_str());
    ImGui::TextDisabled("Select a Mesh or texture slot, then bind the saved Object resource.");
    for (const auto& entry : m_AuthoringWorldObjects)
    {
        ImGui::PushID(entry.resource.objectId.c_str());
        if (ImGui::TreeNodeEx("Object", 0, "%s", entry.resource.displayName.c_str()))
        {
            ImGui::TextWrapped("%s", entry.resource.modelAssetId.c_str());
            const auto bind = [&](const std::string& id)
            { return draft ? Try_BindMeshAuthoringResource(id) : Try_BindResource(id); };
            ImGui::BeginDisabled(entry.resource.modelAssetId.empty());
            if (ImGui::Button("Bind Model to selected slot")) (void)bind(entry.resource.modelAssetId);
            ImGui::EndDisabled();
            ImGui::BeginDisabled(entry.resource.diffuseTextureAssetId.empty());
            if (ImGui::Button("Bind Texture to selected slot")) (void)bind(entry.resource.diffuseTextureAssetId);
            ImGui::EndDisabled();
            for (const auto& motion : entry.motions)
            {
                ImGui::Text("Motion: %s (%.3f s)", motion.displayName.c_str(), motion.durationMs / 1000.f);
                for (const auto& clip : motion.animationTracks) ImGui::BulletText("%s", clip.clipName.c_str());
            }
            if (!entry.error.empty()) ImGui::TextWrapped("%s", entry.error.c_str());
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::PopID();
    return true;
}
bool CEffect_Tool::Is_ClassMovieEffect(const std::string& assetId) const
{
    if (!Has_ClassMovieContext() || !m_ClassMovieCallbacks.timeline) return false;
    for (const bool loop : {false, true})
        if (const auto timeline = m_ClassMovieCallbacks.timeline(m_strClassMovieId, loop))
            for (const auto& row : timeline->rows)
                if (row.kind == "Effect")
                    for (const auto& box : row.boxes)
                        if (box.resource == assetId) return true;
    return false;
}

bool CEffect_Tool::End_ClassMovieEditing()
{
    if (!Has_ClassMovieContext()) return true;
    if (m_ClassMovieCallbacks.clearPreviews && !m_ClassMovieCallbacks.clearPreviews(m_strClassMovieStatus))
        return false;
    m_strClassMovieId.clear();
    m_CinematicElementDocuments.clear();
    m_CinematicElementErrors.clear();
    m_strCinematicSoloOccurrence.clear();
    m_PreviewIsolationElementIds.clear();
    m_ClassMovieFilterTargetIds.clear();
    m_strPreviewIsolationElementId.clear();
    m_strPreviewIsolationGroupId.clear();
    m_bClassMovieSelectionRepeat = false;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    m_bClassMovieScrubbing = false;
    return true;
}

bool CEffect_Tool::Open_ClassMovie(const std::string& classId, const bool loop, const std::string& effectId)
{
    const auto timeline = m_ClassMovieCallbacks.timeline ? m_ClassMovieCallbacks.timeline(classId, loop) : nullptr;
    if (!timeline)
    { m_strClassMovieStatus = classId == "valtan.entrance" ?
        "Enter Valtan Arena to open the complete entrance cinematic." :
        "This Movie is not prepared. Enter Character Select and select an available Movie."; return false; }
    const CLASS_MOVIE_TIMELINE_BOX* selected = nullptr;
    for (const auto& row : timeline->rows)
        if (row.kind == "Effect")
            for (const auto& box : row.boxes)
                if (!selected && (effectId.empty() || box.resource == effectId)) selected = &box;
    if (!selected)
    { m_strClassMovieStatus = "This Movie phase has no matching Effect. Use Timeline / Camera for other rows."; return false; }
    const bool sameEffect = m_ActiveDocument && m_ActiveDocument->strEffectAssetId == selected->resource &&
        m_eActiveDocumentSource == EFFECT_DOCUMENT_SOURCE::AUTHORED;
    if (Has_UnsavedWork() && (!sameEffect || (Has_ClassMovieContext() && m_strClassMovieId != classId)))
    { m_strClassMovieStatus = "Save or discard the current Effect changes before opening another Movie Effect."; return false; }
    if (Has_ClassMovieContext() && m_strClassMovieId != classId && !End_ClassMovieEditing()) return false;
    if (Is_ValtanCinematicEditor() && m_ActiveDocument && !Has_UnsavedWork())
        m_CinematicElementDocuments[m_ActiveDocument->strEffectAssetId] = *m_ActiveDocument;
    if (!sameEffect && !(classId == "valtan.entrance" ? Try_LoadDocument(selected->resource) :
        Open_AuthoringResource({EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, selected->resource})))
    { m_strClassMovieStatus = m_strDocumentStatus; return false; }
    if (m_pAuthoringSequencer) m_pAuthoringSequencer->Stop();
    Release_WorldPreview(true);
    Reset_SynchronizedAnimationSequence();
    m_bPreviewPlaying = false;
    m_strClassMovieId = classId;
    m_PreviewIsolationElementIds.clear();
    m_ClassMovieFilterTargetIds.clear();
    m_strPreviewIsolationElementId.clear();
    m_strPreviewIsolationGroupId.clear();
    m_bClassMovieSelectionRepeat = false;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    m_bClassMovieLoop = loop;
    m_bClassMovieScrubbing = false;
    m_bAllEffectsWorldSelected = !Is_ValtanCinematicEditor();
    m_bAllEffectsValtanBossSelected = Is_ValtanCinematicEditor();
    m_bAllEffectsKoukuBossSelected = false;
    if (Is_ValtanCinematicEditor() && m_CinematicElementDocuments.empty()) Refresh_CinematicElementList();
    const auto state = m_ClassMovieCallbacks.state ? m_ClassMovieCallbacks.state(classId) : CLASS_MOVIE_STATE{};
    if (!state.active && m_ClassMovieCallbacks.seek &&
        !m_ClassMovieCallbacks.seek(classId, loop, selected->movieStartMs, m_strClassMovieStatus)) return false;
    EFFECT_DOCUMENT_DESC document;
    if (!Resolve_AuthoringOccurrenceDocument({EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, selected->resource},
        document, m_strClassMovieStatus) || !Stage_ClassMovieEffect(document))
    {
        m_strClassMovieStatus = "Movie editor opened; previous Effect preview retained: " + m_strPreviewStatus;
        return true;
    }
    m_strClassMovieStatus = Is_ValtanCinematicEditor() ?
        "Entrance opened. All Cinematic Elements lists every source; Play All/Solo follows the original actor animation and timing." :
        "Movie opened in V1. Select an Effect and Element; Play All uses its actors, camera and full timeline.";
    return true;
}

void CEffect_Tool::Sync_ClassMovieSelection()
{
    if (!Has_ClassMovieContext() || !m_ClassMovieCallbacks.state) return;
    const auto state = m_ClassMovieCallbacks.state(m_strClassMovieId);
    m_bClassMovieSelectionRepeat = state.selectionActive && state.selectionRepeat;
    if (!state.selectionActive)
    {
        m_PreviewIsolationElementIds.clear();
        m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    }
}

bool CEffect_Tool::Stage_ClassMovieEffect(const EFFECT_DOCUMENT_DESC& document)
{
    Sync_ClassMovieSelection();
    if (!Is_ClassMovieEffect(document.strEffectAssetId) || !m_ClassMovieCallbacks.preview)
    { m_strPreviewStatus = "This Effect is not part of the selected Movie. Open its Movie Effect again."; return false; }
    EFFECT_DOCUMENT_DESC filtered;
    auto ids = m_PreviewIsolationElementIds;
    // Delete/visibility edits retire stale selection IDs only after the new target commits.
    std::erase_if(ids, [&document](const auto& id) {
        return std::none_of(document.Elements.begin(), document.Elements.end(), [&](const auto& element) {
            return element.strElementId == id && EffectToolDetail::Is_ElementPreviewAdmitted(element);
        });
    });
    uint32_t startMs = 0u, endMs = 0u;
    std::string label;
    if (!ids.empty() && (!Build_ElementsPreviewDocument(document, ids, filtered, m_strPreviewStatus) ||
        !Resolve_ElementsPreviewWindow(filtered, ids, startMs, endMs, label, m_strPreviewStatus))) return false;
    const bool applied = ids.empty() ? m_ClassMovieCallbacks.preview(document, m_strPreviewStatus) :
        m_ClassMovieCallbacks.previewSelection && m_ClassMovieCallbacks.previewSelection(document, filtered, ids, startMs, endMs, m_strPreviewStatus);
    if (!applied) return false;
    m_PreviewIsolationElementIds = std::move(ids);
    if (m_PreviewIsolationElementIds.empty()) m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    m_strClassMovieStatus = m_strPreviewStatus;
    return true;
}

bool CEffect_Tool::Play_ClassMovie()
{
    if (!Has_ClassMovieContext() || !m_ClassMovieCallbacks.play) return false;
    const auto previousFilter = m_ePreviewFilter;
    const auto previousIsolation = std::exchange(m_PreviewIsolationElementIds, {});
    if (m_ActiveDocument && Is_ClassMovieEffect(m_ActiveDocument->strEffectAssetId))
    {
        EFFECT_DOCUMENT_DESC document;
        if (!Resolve_AuthoringOccurrenceDocument({EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT,
            m_ActiveDocument->strEffectAssetId}, document, m_strClassMovieStatus) || !Stage_ClassMovieEffect(document))
        { m_PreviewIsolationElementIds = previousIsolation; m_ePreviewFilter = previousFilter;
          if (!m_strPreviewStatus.empty()) m_strClassMovieStatus = m_strPreviewStatus; return false; }
    }
    const bool played = m_ClassMovieCallbacks.play(m_strClassMovieId, m_strClassMovieStatus);
    m_strPreviewStatus = m_strClassMovieStatus;
    if (played)
    {
        m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
        m_bClassMovieLoop = false; m_bClassMovieScrubbing = false;
        m_bClassMovieSelectionRepeat = false;
    }
    else
    {
        m_PreviewIsolationElementIds = previousIsolation; m_ePreviewFilter = previousFilter;
        Sync_ClassMovieSelection();
    }
    return played;
}

bool CEffect_Tool::Restart_ClassMoviePreview()
{
    Sync_ClassMovieSelection();
    const auto ids = m_PreviewIsolationElementIds;
    return ids.empty() ? Play_ClassMovie() : Try_PreviewElementsTimeline(ids, m_bClassMovieSelectionRepeat);
}

bool CEffect_Tool::Try_SetClassMoviePreviewFilter(const EFFECT_PREVIEW_FILTER filter)
{
    Sync_ClassMovieSelection();
    if (!m_ActiveDocument || filter == EFFECT_PREVIEW_FILTER::END) return false;
    if (filter == EFFECT_PREVIEW_FILTER::COMPLETE) return Play_ClassMovie();
    if (filter == EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUE || filter == EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUES)
    { m_strPreviewStatus = "Movie actors use the WORLD animation rows. Select an Effect Element or group to audition."; return false; }
    const auto previousElement = m_strPreviewIsolationElementId;
    const auto previousGroup = m_strPreviewIsolationGroupId;
    if ((filter == EFFECT_PREVIEW_FILTER::SOLO_SELECTED || filter == EFFECT_PREVIEW_FILTER::MUTE_SELECTED) &&
        m_strPreviewIsolationElementId.empty()) m_strPreviewIsolationElementId = m_strSelectedElementId;
    if ((filter == EFFECT_PREVIEW_FILTER::SOLO_SELECTED || filter == EFFECT_PREVIEW_FILTER::MUTE_SELECTED) &&
        m_strPreviewIsolationElementId.empty())
    { m_strPreviewStatus = "Select an Element before choosing Solo or Mute."; return false; }
    if ((filter == EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP || filter == EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP) &&
        m_strPreviewIsolationGroupId.empty() && m_ClassMovieFilterTargetIds.empty())
    { m_strPreviewStatus = "Use Play Group to select the Movie group first."; return false; }
    if (filter == EFFECT_PREVIEW_FILTER::SOLO_AUTHORING_FAMILY && m_ePreviewIsolationAuthoringFamily == EFFECT_AUTHORING_FAMILY::END)
    { m_strPreviewStatus = "Use Play Family to select a Movie family first."; return false; }
    EFFECT_DOCUMENT_DESC full;
    if (!Resolve_AuthoringOccurrenceDocument({EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, m_ActiveDocument->strEffectAssetId},
        full, m_strPreviewStatus)) return false;
    const auto previousFilter = std::exchange(m_ePreviewFilter, filter);
    const auto previousScope = m_PreviewIsolationElementIds;
    auto filterTargets = m_ClassMovieFilterTargetIds;
    std::erase_if(filterTargets, [&full](const auto& id) {
        return std::none_of(full.Elements.begin(), full.Elements.end(), [&](const auto& element) {
            return element.strElementId == id && EffectToolDetail::Is_ElementPreviewAdmitted(element);
        });
    });
    if ((filter == EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP || filter == EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP) &&
        m_strPreviewIsolationGroupId.empty() && filterTargets.empty())
    { m_ePreviewFilter = previousFilter; m_strPreviewStatus = "The selected group no longer has visible Elements."; return false; }
    if (filter == EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP || filter == EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP)
        m_PreviewIsolationElementIds = filterTargets;
    std::vector<std::string> ids;
    const auto filtered = Build_PreviewDocument(full, &ids);
    m_ePreviewFilter = previousFilter;
    m_PreviewIsolationElementIds = previousScope;
    if (ids.empty())
        for (const auto& element : filtered.Elements)
            if (EffectToolDetail::Is_ElementPreviewAdmitted(element)) ids.push_back(element.strElementId);
    std::erase_if(ids, [&full](const auto& id) {
        return std::none_of(full.Elements.begin(), full.Elements.end(), [&](const auto& element) {
            return element.strElementId == id && EffectToolDetail::Is_ElementPreviewAdmitted(element);
        });
    });
    const auto selectedElement = m_strPreviewIsolationElementId;
    const auto selectedGroup = m_strPreviewIsolationGroupId;
    if (!Try_PreviewElementsTimeline(ids, filter != EFFECT_PREVIEW_FILTER::SOLO_SELECTED))
    {
        m_strPreviewIsolationElementId = previousElement;
        m_strPreviewIsolationGroupId = previousGroup;
        return false;
    }
    // Filter labels identify the user's target; the ID set identifies the resulting draw scope.
    m_ePreviewFilter = filter;
    if (filter == EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP || filter == EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP)
        m_ClassMovieFilterTargetIds = filterTargets;
    m_strPreviewIsolationElementId = selectedElement;
    m_strPreviewIsolationGroupId = selectedGroup;
    return true;
}

void CEffect_Tool::Refresh_CinematicElementList()
{
    const auto timeline = m_ClassMovieCallbacks.timeline ? m_ClassMovieCallbacks.timeline(m_strClassMovieId, false) : nullptr;
    if (!timeline) return;
    std::set<std::string> resources;
    for (const auto& row : timeline->rows)
        if (row.kind == "Effect")
            for (const auto& box : row.boxes) resources.insert(box.resource);
    for (const auto& id : resources)
    {
        if (m_ActiveDocument && m_ActiveDocument->strEffectAssetId == id)
        {
            m_CinematicElementDocuments[id] = *m_ActiveDocument;
            m_CinematicElementErrors.erase(id);
            continue;
        }
        const auto path = CProjectDataRoot::Resolve(std::filesystem::path("Effects") / "Authored" / (id + ".effect.json"));
        EFFECT_DOCUMENT_DESC document;
        std::string error;
        if (!path.empty() && CEffectDocumentCodec::Load(path, document, error) && document.strEffectAssetId == id)
        {
            m_CinematicElementDocuments[id] = std::move(document);
            m_CinematicElementErrors.erase(id);
        }
        else m_CinematicElementErrors[id] = error.empty() ? "The saved Effect ID/path does not match this occurrence." : error;
    }
}

void CEffect_Tool::Render_CinematicElementList()
{
    const auto timeline = m_ClassMovieCallbacks.timeline ? m_ClassMovieCallbacks.timeline(m_strClassMovieId, false) : nullptr;
    if (!timeline) return;
    ImGui::SeparatorText("All Cinematic Elements");
    if (ImGui::SmallButton("Refresh Element List")) Refresh_CinematicElementList();
    ImGui::SameLine();
    if (ImGui::SmallButton("Solo: All occurrences") && m_ClassMovieCallbacks.selectEffectOccurrence &&
        m_ClassMovieCallbacks.selectEffectOccurrence({}, m_strClassMovieStatus)) m_strCinematicSoloOccurrence.clear();
    if (!m_strCinematicSoloOccurrence.empty()) ImGui::TextWrapped("Solo occurrence: %s", m_strCinematicSoloOccurrence.c_str());
    ImGui::InputText("Find Element", m_CinematicElementSearch.data(), m_CinematicElementSearch.size());
    const std::string search = m_CinematicElementSearch.data();
    std::map<std::string, std::vector<const CLASS_MOVIE_TIMELINE_BOX*>> occurrences;
    for (const auto& row : timeline->rows)
        if (row.kind == "Effect")
            for (const auto& box : row.boxes) occurrences[box.resource].push_back(&box);
    size_t elementCount = 0u, occurrenceCount = 0u;
    for (const auto& [id, boxes] : occurrences)
    {
        const auto cached = m_CinematicElementDocuments.find(id);
        const auto* document = m_ActiveDocument && m_ActiveDocument->strEffectAssetId == id ? &*m_ActiveDocument :
            cached != m_CinematicElementDocuments.end() ? &cached->second : nullptr;
        if (document) { elementCount += document->Elements.size(); occurrenceCount += document->Elements.size() * boxes.size(); }
    }
    ImGui::Text("%zu Effects | %zu Elements | %zu Element occurrences", occurrences.size(), elementCount, occurrenceCount);
    ImGui::TextWrapped("Select any Element to edit it; Solo keeps the animated scene and isolates that Element. Save Changes writes its original shared Effect. Repeated occurrences share edits.");
    if (ImGui::TreeNode("Linked Animations"))
    {
        for (const auto& row : timeline->rows)
            if (row.kind == "Animation")
                for (const auto& box : row.boxes)
                    ImGui::BulletText("%s: %s (%.3f - %.3f s)", row.label.c_str(), box.label.c_str(),
                        box.movieStartMs * .001, box.movieEndMs * .001);
        ImGui::TreePop();
    }
    std::string openAsset, openElement;
    bool playSolo = false;
    ImGui::BeginChild("CinematicElementList", ImVec2(0.f, 360.f), ImGuiChildFlags_Borders);
    for (const auto& [id, boxes] : occurrences)
    {
        const auto cached = m_CinematicElementDocuments.find(id);
        const auto* document = m_ActiveDocument && m_ActiveDocument->strEffectAssetId == id ? &*m_ActiveDocument :
            cached != m_CinematicElementDocuments.end() ? &cached->second : nullptr;
        const bool bodyMatch = search.empty() || Contains_NoCase(id, search) ||
            (document && Contains_NoCase(document->strDisplayName, search));
        if (!bodyMatch && (!document || std::none_of(document->Elements.begin(), document->Elements.end(),
            [&](const auto& e) { return Contains_NoCase(e.strElementId, search) || Contains_NoCase(e.strDisplayName, search); }))) continue;
        ImGui::PushID(id.c_str());
        ImGui::SetNextItemOpen(true, search.empty() ? ImGuiCond_Once : ImGuiCond_Always);
        const bool expanded = ImGui::TreeNodeEx("Effect", ImGuiTreeNodeFlags_OpenOnArrow, "%s (%zu)",
            document ? document->strDisplayName.c_str() : id.c_str(), document ? document->Elements.size() : 0u);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", id.c_str());
        if (expanded)
        {
            for (const auto* box : boxes)
            {
                ImGui::PushID(box->id.c_str());
                if (ImGui::RadioButton("Use occurrence", m_strCinematicSoloOccurrence == box->id) &&
                    m_ClassMovieCallbacks.selectEffectOccurrence &&
                    m_ClassMovieCallbacks.selectEffectOccurrence(box->id, m_strClassMovieStatus))
                    m_strCinematicSoloOccurrence = box->id;
                ImGui::SameLine();
                ImGui::TextDisabled("%s | %.3f - %.3f s", box->label.c_str(), box->movieStartMs * .001, box->movieEndMs * .001);
                ImGui::PopID();
            }
            if (const auto error = m_CinematicElementErrors.find(id); error != m_CinematicElementErrors.end())
                ImGui::TextWrapped("Source refresh failed; previous list retained: %s", error->second.c_str());
            if (document)
                for (const auto& element : document->Elements)
                {
                    if (!bodyMatch && !Contains_NoCase(element.strElementId, search) && !Contains_NoCase(element.strDisplayName, search)) continue;
                    ImGui::PushID(element.strElementId.c_str());
                    const bool admitted = Is_ElementPreviewAdmitted(element);
                    ImGui::BeginDisabled(!admitted);
                    if (ImGui::SmallButton("Solo")) { openAsset = id; openElement = element.strElementId; playSolo = true; }
                    ImGui::EndDisabled();
                    if (!admitted && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip("This Element is hidden or failed runtime admission. Select it to inspect its details.");
                    ImGui::SameLine();
                    const bool selected = m_ActiveDocument && m_ActiveDocument->strEffectAssetId == id && m_strSelectedElementId == element.strElementId;
                    const std::string label = element.strDisplayName.empty() ? element.strElementId : element.strDisplayName;
                    if (ImGui::Selectable(label.c_str(), selected)) { openAsset = id; openElement = element.strElementId; playSolo = false; }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\nSource age %.3f s", element.strElementId.c_str(), element.Detail.Timing.fStartDelaySeconds);
                    ImGui::PopID();
                }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    // Defer document changes until no row retains references into the previous source.
    if (!openAsset.empty())
    {
        const auto& boxes = occurrences.at(openAsset);
        if (!m_strCinematicSoloOccurrence.empty() && std::none_of(boxes.begin(), boxes.end(),
            [this](const auto* box) { return box->id == m_strCinematicSoloOccurrence; }) &&
            m_ClassMovieCallbacks.selectEffectOccurrence &&
            m_ClassMovieCallbacks.selectEffectOccurrence({}, m_strClassMovieStatus)) m_strCinematicSoloOccurrence.clear();
        const bool active = m_ActiveDocument && m_ActiveDocument->strEffectAssetId == openAsset;
        if (active || Open_ClassMovie("valtan.entrance", false, openAsset))
        {
            if (Try_SelectElement(openAsset, openElement) && playSolo) (void)Try_SoloElement(openAsset, openElement);
            if (!m_strElementStatus.empty()) m_strClassMovieStatus = m_strElementStatus;
        }
    }
}

void CEffect_Tool::Render_ClassMovieControls(const bool showEffects)
{
    Sync_ClassMovieSelection();
    const auto state = m_ClassMovieCallbacks.state ? m_ClassMovieCallbacks.state(m_strClassMovieId) : CLASS_MOVIE_STATE{};
    const auto movie = std::find_if(m_ClassMovieResources.begin(), m_ClassMovieResources.end(),
        [this](const auto& row) { return row.classId == m_strClassMovieId; });
    const bool cinematic = Is_ValtanCinematicEditor();
    ImGui::SeparatorText(cinematic ? "Valtan Entrance / Animation + Effects" : "Movie / Character Animation");
    ImGui::TextWrapped("%s", cinematic ? "Valtan Entrance" :
        movie == m_ClassMovieResources.end() ? m_strClassMovieId.c_str() : movie->label.c_str());
    ImGui::BeginDisabled(!state.available);
    if (ImGui::Button("Play All")) (void)Play_ClassMovie();
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!state.active);
    if (ImGui::Button(state.paused ? "Resume" : "Pause") && m_ClassMovieCallbacks.pause)
        m_ClassMovieCallbacks.pause(!state.paused);
    ImGui::SameLine();
    if (ImGui::Button("Stop") && m_ClassMovieCallbacks.stop) m_ClassMovieCallbacks.stop();
    ImGui::EndDisabled();
    if (!cinematic)
    {
        ImGui::SameLine();
        if (ImGui::Button("Timeline / Camera")) m_PendingClassMovieEditor = m_strClassMovieId;
    }
    ImGui::TextDisabled("Actors and Effects share Movie time. Element edits preview here; Save stores the Effect.");
    ImGui::BeginDisabled(!state.available);
    float playbackRate = static_cast<float>(state.playbackRate);
    if (ImGui::SliderFloat("Movie playback rate", &playbackRate, .05f, 2.f, "%.2fx") && m_ClassMovieCallbacks.setPlaybackRate)
        (void)m_ClassMovieCallbacks.setPlaybackRate(playbackRate);
    if (!m_PreviewIsolationElementIds.empty())
    {
        if (ImGui::Button("Restart Selection")) (void)Restart_ClassMoviePreview();
        ImGui::SameLine();
        bool repeat = state.selectionActive ? state.selectionRepeat : m_bClassMovieSelectionRepeat;
        if (ImGui::Checkbox("Repeat Selection", &repeat))
        {
            const auto ids = m_PreviewIsolationElementIds;
            (void)Try_PreviewElementsTimeline(ids, repeat);
        }
        ImGui::TextDisabled("Selected occurrence: %zu Elements. Play All restores the complete Movie.", m_PreviewIsolationElementIds.size());
    }
    ImGui::EndDisabled();
    const char* scopeNames[] = { "Complete Movie", "All Particles", "Standalone Mesh", "Mesh Particles",
        "Standalone Sprite", "Sprite Particles", "Solo Element", "Mute Element", "Solo Group", "Mute Group" };
    const EFFECT_PREVIEW_FILTER scopes[] = { EFFECT_PREVIEW_FILTER::COMPLETE, EFFECT_PREVIEW_FILTER::SOLO_PARTICLE_SYSTEM,
        EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_MESHES, EFFECT_PREVIEW_FILTER::SOLO_MESH_EMITTERS,
        EFFECT_PREVIEW_FILTER::SOLO_STANDALONE_SPRITES, EFFECT_PREVIEW_FILTER::SOLO_SPRITE_EMITTERS,
        EFFECT_PREVIEW_FILTER::SOLO_SELECTED, EFFECT_PREVIEW_FILTER::MUTE_SELECTED,
        EFFECT_PREVIEW_FILTER::SOLO_SELECTED_GROUP, EFFECT_PREVIEW_FILTER::MUTE_SELECTED_GROUP };
    const char* scopeLabel = "Selected Family";
    for (size_t i = 0; i < std::size(scopes); ++i) if (scopes[i] == m_ePreviewFilter) scopeLabel = scopeNames[i];
    if (ImGui::BeginCombo("Movie preview scope", scopeLabel))
    {
        for (size_t i = 0; i < std::size(scopes); ++i)
            if (ImGui::Selectable(scopeNames[i], scopes[i] == m_ePreviewFilter)) (void)Try_SetPreviewFilter(scopes[i]);
        ImGui::EndCombo();
    }
    if (state.active) ImGui::Text("Playing %s: %.3f / %.3f s%s", state.loop ? "Loop" : "Intro",
        state.clockMs * .001, state.durationMs * .001, state.paused ? " (paused)" : "");
    if (!cinematic)
    {
        if (ImGui::RadioButton("Intro Effects", !m_bClassMovieLoop)) { m_bClassMovieLoop = false; m_bClassMovieScrubbing = false; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Loop Effects", m_bClassMovieLoop)) { m_bClassMovieLoop = true; m_bClassMovieScrubbing = false; }
    }
    const auto timeline = m_ClassMovieCallbacks.timeline ? m_ClassMovieCallbacks.timeline(m_strClassMovieId, m_bClassMovieLoop) : nullptr;
    if (timeline)
    {
        if (!m_bClassMovieScrubbing)
            m_fClassMovieSeekSeconds = state.active && state.loop == m_bClassMovieLoop ? static_cast<float>(state.clockMs * .001) : 0.f;
        ImGui::BeginDisabled(!state.available);
        ImGui::SliderFloat("Movie time", &m_fClassMovieSeekSeconds, 0.f, static_cast<float>(timeline->movieDurationMs * .001), "%.3f s");
        if (ImGui::IsItemActive()) m_bClassMovieScrubbing = true;
        if (ImGui::IsItemDeactivated())
        {
            m_bClassMovieScrubbing = false;
            if (ImGui::IsItemDeactivatedAfterEdit() && m_ClassMovieCallbacks.seek)
                (void)m_ClassMovieCallbacks.seek(m_strClassMovieId, m_bClassMovieLoop,
                    m_fClassMovieSeekSeconds * 1000., m_strClassMovieStatus);
        }
        ImGui::EndDisabled();
        if (cinematic && showEffects) Render_CinematicElementList();
        if (showEffects && !cinematic)
        {
        ImGui::SeparatorText("Movie Effects");
        std::set<std::string> shown;
        for (const auto& row : timeline->rows)
            if (row.kind == "Effect")
                for (const auto& box : row.boxes)
                {
                    if (!shown.insert(box.resource).second) continue;
                    ImGui::PushID(box.resource.c_str());
                    const bool selected = m_ActiveDocument && m_ActiveDocument->strEffectAssetId == box.resource;
                    if (ImGui::Selectable(box.resource.c_str(), selected))
                        (void)Open_ClassMovie(m_strClassMovieId, m_bClassMovieLoop, box.resource);
                    if (selected)
                    {
                        ImGui::BeginDisabled(!state.available);
                        if (ImGui::SmallButton("View in Movie") && m_ClassMovieCallbacks.seek)
                            (void)m_ClassMovieCallbacks.seek(m_strClassMovieId, m_bClassMovieLoop,
                                box.movieStartMs + (std::min)(100., (box.movieEndMs - box.movieStartMs) * .5), m_strClassMovieStatus);
                        ImGui::EndDisabled();
                        ImGui::SameLine(); ImGui::TextDisabled("First occurrence %.3f s", box.movieStartMs * .001);
                    }
                    ImGui::PopID();
                }
        }
    }
    if (!cinematic) m_ClassMovieInspector.Render(m_ClassMovieCallbacks.inspection, m_strClassMovieId, m_bClassMovieLoop);
    if (!state.status.empty()) ImGui::TextWrapped("%s", state.status.c_str());
    if (!m_strClassMovieStatus.empty()) ImGui::TextWrapped("%s", m_strClassMovieStatus.c_str());
    if (showEffects && ImGui::Button("End Movie Editing")) (void)End_ClassMovieEditing();
}


}
