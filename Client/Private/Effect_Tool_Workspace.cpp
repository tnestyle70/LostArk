#include "imgui.h"

#include "Effect_Tool.h"
#include "EffectAuthoringResourceTree.h"
#include "EffectAuthoringSequencer.h"
#include "EffectAuthoringV2Pane.h"
#include "Effect_Tool_V2.h"
#include "Effect_DocumentCodec.h"
#include "Effect_Object.h"
#include "EffectV2_Catalog.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"
#include <algorithm>
#include <cstring>
#include <cstdio>

namespace Client
{
namespace
{
constexpr const wchar_t* AUTHORING_LAYER = L"Layer_EffectAuthoringSequencer";
std::string Parent_Key(const EFFECT_RESOURCE_KEY& key)
{ return std::to_string(static_cast<int>(key.eOwnerKind)) + ":" + key.strStableId; }
}

void CEffect_Tool::Configure_AuthoringWorkspace(CEffect_Tool_V2& editor, CKoukuSaydonPresentationPlayer* player)
{
    if (!m_pAuthoringResources) m_pAuthoringResources = std::make_unique<CEffectAuthoringResourceTree>();
    if (!m_pAuthoringV2) m_pAuthoringV2 = std::make_unique<CEffectAuthoringV2Pane>(editor);
    if (!m_pAuthoringSequencer)
    {
        m_pAuthoringSequencer = std::make_unique<CEffectAuthoringSequencer>(m_pDevice, m_pContext, m_pCharacterPreviewPanel);
        m_pAuthoringSequencer->Set_V1Callbacks(
            [this](const EFFECT_RESOURCE_KEY& key, const float4x4_t& root, std::shared_ptr<CEffectObject>& object, std::string& error)
            { return Create_AuthoringOccurrence(key, root, object, error); },
            [this](const std::shared_ptr<CEffectObject>& object)
            {
                const auto found = m_AuthoringOccurrenceLevels.find(object.get());
                if (found == m_AuthoringOccurrenceLevels.end()) return;
                object->Set_Visible(false);
                CGameInstance::Get().Remove_GameObject_from_Layer(found->second, AUTHORING_LAYER, object);
                m_AuthoringOccurrenceDocuments.erase(object.get());
                m_AuthoringOccurrenceLevels.erase(found);
            });
        m_pAuthoringSequencer->Set_V1AnchorProvider(
            [this](const std::shared_ptr<CEffectObject>& object, const float4x4_t& root, bool useKouku,
                std::unordered_map<std::string, float4x4_t>& anchors, std::string& error)
            { return Resolve_AuthoringSourceAnchors(object, root, useKouku, anchors, error); });
        m_pAuthoringSequencer->Set_V2SnapshotProvider(
            [this](const EFFECT_RESOURCE_KEY& key, std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>& snapshot, std::string& error)
            {
                EFFECT_RESOURCE_KEY selected;
                std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> draft;
                if (m_pAuthoringV2 && m_pAuthoringV2->Owns_Resource(key))
                {
                    if (!m_pAuthoringV2->Snapshot(selected, draft, error)) return false;
                    snapshot = std::move(draft); return true;
                }
                return CEffectV2Catalog::Get().Load_ResourceSnapshot(
                    key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V2_GROUP ? EFFECT_V2_RESOURCE_KIND::GROUP : EFFECT_V2_RESOURCE_KIND::LEAF,
                    key.strStableId, snapshot, error);
            });
    }
    m_pAuthoringSequencer->Set_Player(player);
}

void CEffect_Tool::Set_AuthoringPlayer(CKoukuSaydonPresentationPlayer* player)
{
    if (m_pAuthoringSequencer) m_pAuthoringSequencer->Set_Player(player);
}
void CEffect_Tool::Update_AuthoringWorkspace(float dt, bool active)
{
    if (m_pAuthoringSequencer) m_pAuthoringSequencer->Update(dt, active);
    if (active && m_pAuthoringSequencer && m_pAuthoringSequencer->Is_Active() && m_pAuthoringV2 &&
        m_bAuthoringV2Selected && m_pAuthoringV2->Edit_Generation() != m_iAuthoringV2PreviewGeneration && !ImGui::IsAnyItemActive())
    {
        m_iAuthoringV2PreviewGeneration = m_pAuthoringV2->Edit_Generation();
        (void)m_pAuthoringSequencer->Refresh_Effects();
    }

    if (m_pAuthoringV2 && m_pAuthoringResources)
    {
        const auto current = m_pAuthoringV2->Current_Key();
        if (current.Is_Valid() && !(current == m_AuthoringV2PreviousKey))
        {
            const auto prior = m_AuthoringParents.find(Parent_Key(m_AuthoringV2PreviousKey));
            if (m_AuthoringV2PreviousKey.Is_Valid() && !m_AuthoringParents.contains(Parent_Key(current)) && prior != m_AuthoringParents.end())
                m_AuthoringParents.emplace(Parent_Key(current), prior->second);
            m_AuthoringV2PreviousKey = current;
        }
        EFFECT_RESOURCE_KEY play; std::string child;
        if (active && m_pAuthoringV2->Consume_Play(play, child) && m_pAuthoringSequencer)
        {
            Release_WorldPreview(true);
            (void)m_pAuthoringSequencer->Preview(play, m_pAuthoringV2->DurationMs());
        }
        EFFECT_RESOURCE_KEY saved; std::string name;
        if (m_pAuthoringV2->Consume_Saved(saved, name))
        {
            std::string status;
            (void)m_pAuthoringResources->Attach_Saved(saved.eOwnerKind, saved.strStableId, name,
                m_AuthoringParents[Parent_Key(saved)], status);
            m_pAuthoringResources->Set_Status(status);
        }
    }
}
void CEffect_Tool::Deactivate_AuthoringWorkspace()
{
    if (m_pAuthoringSequencer) m_pAuthoringSequencer->Stop();
    m_bPreviewPlaying = false;
    Release_WorldPreview(true);
}
bool CEffect_Tool::Consume_AuthoringInteraction()
{ return m_pAuthoringSequencer && m_pAuthoringSequencer->Consume_InteractionRequest(); }

bool CEffect_Tool::Open_AuthoringResource(const EFFECT_RESOURCE_KEY& key)
{
    if (key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT)
    {
        const bool opened = Try_LoadDocument(key.strStableId);
        if (opened) m_bAuthoringV2Selected = false;
        return opened;
    }
    if (!m_pAuthoringV2) return false;
    const bool opened = m_pAuthoringV2->Open(key);
    if (opened) { m_bAuthoringV2Selected = true; m_AuthoringV2PreviousKey = key; Release_WorldPreview(true); }
    return opened;
}

void CEffect_Tool::Render_AuthoringResourceTree()
{
    ImGui::SetNextWindowPos(ImVec2(1110.f, 35.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430.f, 660.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Effect Resources")) { ImGui::End(); return; }
    m_pAuthoringResources->Render();
    CEffectAuthoringResourceTree::COMMAND command;
    while (m_pAuthoringResources->Take_Command(command))
    {
        const EFFECT_RESOURCE_KEY key{command.eKind, command.strAssetId};
        if (command.eCommand == CEffectAuthoringResourceTree::COMMAND_KIND::OPEN)
        {
            m_AuthoringParents[Parent_Key(key)] = command.strParentId;
            if (Open_AuthoringResource(key)) m_strAuthoringParentId = command.strParentId;
            else m_pAuthoringResources->Set_Status(command.eKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT ? m_strDocumentStatus : m_pAuthoringV2->Status());
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
                    if (created) m_bAuthoringV2Selected = false;
                }
                m_pAuthoringResources->Set_Status(m_strDocumentStatus);
            }
            else if (m_pAuthoringV2)
            {
                created = m_pAuthoringV2->Create(command.strDisplayName, EFFECT_V2_TYPE::PARTICLE, command.eKind);
                if (created) { m_bAuthoringV2Selected = true; Release_WorldPreview(true); }
                m_pAuthoringResources->Set_Status(m_pAuthoringV2->Status());
            }
            if (created)
            {
                m_strAuthoringParentId = command.strParentId;
                const auto createdId = command.eKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT ?
                    m_ActiveDocument->strEffectAssetId : m_pAuthoringV2->Current_Key().strStableId;
                m_AuthoringParents[Parent_Key({command.eKind, createdId})] = command.strParentId;
            }
        }
        else if (m_pAuthoringSequencer)
        {
            Release_WorldPreview(true);
            const bool ok = command.eCommand == CEffectAuthoringResourceTree::COMMAND_KIND::APPEND ?
                m_pAuthoringSequencer->Append(key) : m_pAuthoringSequencer->Preview(key);
            (void)ok;
            m_pAuthoringResources->Set_Status(m_pAuthoringSequencer->Status());
        }
    }
    ImGui::End();
}

void CEffect_Tool::Render_AuthoringCommands()
{
    if (!m_pAuthoringSequencer) return;
    EFFECT_RESOURCE_KEY selected;
    if (m_bAuthoringV2Selected && m_pAuthoringV2) selected = m_pAuthoringV2->Current_Key();
    else if (m_ActiveDocument) selected = {EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, m_ActiveDocument->strEffectAssetId};
    ImGui::BeginDisabled(!selected.Is_Valid());
    if (ImGui::Button("Append##EffectSequence"))
    {
        Release_WorldPreview(true);
        const uint32_t duration = !m_bAuthoringV2Selected ?
            static_cast<uint32_t>((std::max)(0.001f, m_fPreviewDurationSeconds) * 1000.f) : m_pAuthoringV2->DurationMs();
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

bool CEffect_Tool::Create_AuthoringOccurrence(const EFFECT_RESOURCE_KEY& key, const float4x4_t& root,
    std::shared_ptr<CEffectObject>& object, std::string& error)
{
    if (key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT || !key.Is_Valid())
    { error = "Invalid V1 Effect reference."; return false; }
    EFFECT_DOCUMENT_DESC document;
    if (m_ActiveDocument && m_ActiveDocument->strEffectAssetId == key.strStableId)
    {
        document = *m_ActiveDocument;
        if (m_bDetailDraftDirty && !Apply_DetailDraft(document))
        { error = m_strDetailStatus; return false; }
    }
    else
    {
        const auto path = CProjectDataRoot::Resolve(std::filesystem::path(L"Effects") / L"Authored" /
            (key.strStableId + ".effect.json"));
        if (!CEffectDocumentCodec::Load(path, document, error)) return false;
    }
    if (document.strEffectAssetId != key.strStableId)
    { error = "Saved Effect identity does not match the selected row."; return false; }
    if (!CEffectDocumentCodec::Validate_Drawable(document, error)) return false;
    const uint32_t level = CGameInstance::Get().Get_CurrentLevelID();
    CEffectObject::EFFECT_OBJECT_DESC desc{};
    desc.RootWorld = root; desc.bAutoPlay = false;
    std::shared_ptr<CGameObject> clone;
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::STATIC),
        L"Prototype_GameObject_EffectObject", level, AUTHORING_LAYER, &desc, &clone)))
    { error = "EffectObject prototype is not available in this level."; return false; }
    auto staged = std::dynamic_pointer_cast<CEffectObject>(clone);
    if (!staged || !staged->Stage_Document(document, error))
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(level, AUTHORING_LAYER, clone);
        return false;
    }
    staged->Set_RootWorld(root); staged->Set_Playing(false); staged->Set_Visible(false);
    m_AuthoringOccurrenceLevels.emplace(staged.get(), level);
    m_AuthoringOccurrenceDocuments.emplace(staged.get(), std::make_shared<const EFFECT_DOCUMENT_DESC>(std::move(document)));
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
}
