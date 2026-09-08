#include "imgui.h"

#include "EffectAuthoringV2Pane.h"
#include "Effect_Tool_V2.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <set>

namespace Client
{
namespace
{
using OWNER = EFFECT_RESOURCE_OWNER_KIND;
const char* Label(const std::string& name, const std::string& id) { return name.empty() ? id.c_str() : name.c_str(); }
bool Edit_Name(const char* label, std::string& value)
{
    char text[257]{};
    std::snprintf(text, sizeof(text), "%s", value.c_str());
    if (!ImGui::InputText(label, text, sizeof(text))) return false;
    value = text;
    return true;
}
bool Edit_Ms(const char* label, uint32_t& value)
{
    int staged = static_cast<int>(value);
    if (!ImGui::DragInt(label, &staged, 1.f, 0, 600000, "%d ms")) return false;
    value = static_cast<uint32_t>((std::clamp)(staged, 0, 600000));
    return true;
}
}

CEffectAuthoringV2Pane::CEffectAuthoringV2Pane(CEffect_Tool_V2& editor) : m_Editor(editor) {}

EFFECT_RESOURCE_KEY CEffectAuthoringV2Pane::Current_Key() const
{
    if (m_Edit.Empty()) return {};
    return { m_Edit.Resource_Kind() == EFFECT_V2_RESOURCE_KIND::LEAF ? OWNER::V2_LEAF : OWNER::V2_GROUP,
        m_Edit.Resource_Id() };
}

uint32_t CEffectAuthoringV2Pane::DurationMs() const
{
    if (m_Edit.Empty()) return 3000u;
    const bool native = m_Edit.Resource_Kind() == EFFECT_V2_RESOURCE_KIND::LEAF;
    if (!native && m_Edit.Group().iDurationMs) return m_Edit.Group().iDurationMs;
    double end = 0.0;
    for (const auto& child : m_Edit.Group().Children)
    {
        const auto found = m_Edit.Leaves().find(child.strEffectId);
        if (found == m_Edit.Leaves().end()) continue;
        const auto& document = found->second.document;
        const auto& params = document.Desc.Params;
        const double rate = (std::max)(.001, double(params.fPlayRate));
        const double emission = !native && child.iDurationMs ? child.iDurationMs :
            params.fLifetime > 0.f ? std::ceil(double(params.fLifetime) * 1000.0 / rate) : 3000.0;
        double tail = 0.0;
        if (native || child.eStop == EFFECT_V2_CHILD_STOP::DEACTIVATE)
        {
            if (document.eType == EFFECT_V2_TYPE::PARTICLE) tail = params.Particle.vLifetime.y * 1000.0 / rate;
            if (document.eType == EFFECT_V2_TYPE::TRAIL) tail = params.Trail.fPointLifetime * 1000.0 / rate;
        }
        const double childEnd = (native ? 0.0 : double(child.iStartMs)) + emission + std::ceil(tail);
        if (std::isfinite(childEnd)) end = (std::max)(end, childEnd);
    }
    return static_cast<uint32_t>((std::clamp)(std::ceil(end), 1.0, 600000.0));
}

bool CEffectAuthoringV2Pane::Open(const EFFECT_RESOURCE_KEY& key)
{
    if (key.eOwnerKind != OWNER::V2_LEAF && key.eOwnerKind != OWNER::V2_GROUP)
    { m_Status = "Open this Effect in its original V1 editor."; return false; }
    return m_Edit.Load(key.eOwnerKind == OWNER::V2_LEAF ? EFFECT_V2_RESOURCE_KIND::LEAF :
        EFFECT_V2_RESOURCE_KIND::GROUP, key.strStableId, m_Status);
}

bool CEffectAuthoringV2Pane::Create(const std::string& name, EFFECT_V2_TYPE type, OWNER owner)
{
    if (m_Edit.Dirty()) { m_Status = "Save or Revert the current Effect before creating another."; return false; }
    if (name.empty() || !CEffectV2Document::Is_ValidDisplayName(name) ||
        type < EFFECT_V2_TYPE::MESH || type >= EFFECT_V2_TYPE::END ||
        (owner != OWNER::V2_LEAF && owner != OWNER::V2_GROUP))
    { m_Status = "Choose a valid Effect name and V2 type."; return false; }
    m_Edit.Create(name, type, owner == OWNER::V2_LEAF ? EFFECT_V2_RESOURCE_KIND::LEAF : EFFECT_V2_RESOURCE_KIND::GROUP);
    m_AddType = static_cast<int>(type);
    m_Status = "Created an unsaved Effect. Bind resources and tune its details.";
    return true;
}

bool CEffectAuthoringV2Pane::Append(const EFFECT_RESOURCE_KEY& key)
{
    if (key.eOwnerKind != OWNER::V2_LEAF)
    { m_Status = "This V2 group accepts native V2 Effect elements. Other owner formats are not converted."; return false; }
    return m_Edit.Append(key.strStableId, m_Status);
}

bool CEffectAuthoringV2Pane::Snapshot(EFFECT_RESOURCE_KEY& outKey,
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>& outSnapshot, std::string& error) const
{
    if (m_Edit.Empty()) { error = "Create or open an Effect first."; return false; }
    std::vector<EFFECT_V2_DOCUMENT> documents;
    std::set<std::string> seen;
    for (const auto& child : m_Edit.Group().Children)
    {
        if (!seen.insert(child.strEffectId).second) continue;
        const auto found = m_Edit.Leaves().find(child.strEffectId);
        if (found == m_Edit.Leaves().end()) { error = "Missing selected Effect draft: " + child.strEffectId; return false; }
        documents.push_back(found->second.document);
    }
    std::vector<EFFECT_V2_GROUP> groups;
    if (m_Edit.Resource_Kind() == EFFECT_V2_RESOURCE_KIND::GROUP) groups.push_back(m_Edit.Group());
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> staged;
    if (!CEffectV2Catalog::Get().Create_ResourceSnapshot(documents, groups, staged, error)) return false;
    outSnapshot = std::move(staged);
    outKey = Current_Key();
    return true;
}

bool CEffectAuthoringV2Pane::Consume_Saved(EFFECT_RESOURCE_KEY& outKey, std::string& outName)
{
    if (!m_HasSaved) return false;
    outKey = m_SavedKey;
    outName = m_SavedName;
    m_HasSaved = false;
    return true;
}

bool CEffectAuthoringV2Pane::Consume_Play(EFFECT_RESOURCE_KEY& outKey, std::string& outChildId)
{
    if (!m_HasPlay) return false;
    outKey = m_PlayKey;
    outChildId = m_PlayChildId;
    m_HasPlay = false;
    return true;
}

void CEffectAuthoringV2Pane::Render_ToolContents()
{
    ImGui::SeparatorText("Current Effect");
    if (m_Edit.Empty())
    { ImGui::TextWrapped("Choose Open or Create Effect in the Saved Effects tab."); return; }
    const bool leaf = m_Edit.Resource_Kind() == EFFECT_V2_RESOURCE_KIND::LEAF;
    ImGui::TextDisabled("%s | %s", leaf ? "V2 Effect" : "V2 Group", m_Edit.Resource_Id().c_str());
    const auto requestPlay = [&](bool selectedOnly)
    {
        m_PlayKey = Current_Key();
        m_PlayChildId.clear();
        if (selectedOnly && !leaf)
        {
            const auto* child = m_Edit.Selected_Child();
            if (!child) return;
            m_PlayKey = { OWNER::V2_LEAF, child->strEffectId };
            m_PlayChildId = child->strChildId;
        }
        m_HasPlay = true;
    };
    if (ImGui::Button("Play All")) requestPlay(false);
    ImGui::SameLine();
    if (ImGui::Button("Play Family")) requestPlay(false);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("A V2 group is one saved Effect family.");
    ImGui::SameLine();
    ImGui::BeginDisabled(m_Edit.Selected_Child() == nullptr);
    if (ImGui::Button("Play Element")) requestPlay(true);
    ImGui::EndDisabled();
    if (ImGui::Button("Save##V2CurrentEffect"))
    {
        if (m_Edit.Save(m_Status))
        {
            m_SavedKey = Current_Key();
            const auto* document = m_Edit.Selected_Document();
            m_SavedName = leaf && document ? document->strDisplayName : m_Edit.Group().strDisplayName;
            m_HasSaved = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Revert##V2CurrentEffect")) m_Edit.Revert(m_Status);
    if (m_Edit.Dirty()) { ImGui::SameLine(); ImGui::TextUnformatted("Unsaved"); }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_Edit.Dirty());
    if (ImGui::Button("Attach / Spawn Target##V2CurrentEffect"))
        m_Status = m_Editor.Open_Attach(Current_Key()) ?
            "Attach window opened for the saved Effect." : m_Editor.Document_Status();
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("Spawn an NPC / boss / preview body target, follow a bone and save bindings. Save the Effect first.");
    if (m_Edit.Empty()) return;
    if (leaf)
    {
        if (ImGui::Button("Create Group from this Effect")) m_Edit.Promote_ToGroup(m_Status);
    }
    else
    {
        if (Edit_Name("Group name", m_Edit.Group().strDisplayName)) m_Edit.Touch();
        if (Edit_Ms("Group duration (0 = auto)", m_Edit.Group().iDurationMs)) m_Edit.Touch();
        ImGui::Combo("Add type", &m_AddType, "Mesh\0Texture\0Mesh / Sprite Particle\0Decal\0Trail\0Screen Post\0");
        ImGui::InputTextWithHint("##NewV2ElementName", "Element name", m_ElementName, sizeof(m_ElementName));
        if (ImGui::Button("Add Element"))
            m_Edit.Add_Element(static_cast<EFFECT_V2_TYPE>(m_AddType), m_ElementName[0] ? m_ElementName : "Element");
        ImGui::SameLine();
        ImGui::BeginDisabled(m_Edit.Selected_Child() == nullptr);
        if (ImGui::Button("Delete Element")) m_Edit.Remove_Selected();
        ImGui::EndDisabled();
    }
    for (const auto& child : m_Edit.Group().Children)
    {
        const auto found = m_Edit.Leaves().find(child.strEffectId);
        const std::string name = found == m_Edit.Leaves().end() ? child.strEffectId :
            Label(found->second.document.strDisplayName, child.strEffectId);
        ImGui::PushID(child.strChildId.c_str());
        if (ImGui::Selectable(name.c_str(), m_Edit.Selection() == child.strChildId)) m_Edit.Select(child.strChildId);
        ImGui::PopID();
    }
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
}

void CEffectAuthoringV2Pane::Render_DetailContents()
{
    auto* document = m_Edit.Selected_Document();
    if (!document) { ImGui::TextWrapped("Select a Current Effect element to edit its details."); return; }
    if (Edit_Name("Effect name", document->strDisplayName)) m_Edit.Touch();
    ImGui::TextDisabled("ID: %s", document->strEffectId.c_str());
    int type = static_cast<int>(document->eType);
    if (ImGui::Combo("Type", &type, "Mesh\0Texture\0Mesh / Sprite Particle\0Decal\0Trail\0Screen Post\0"))
    {
        document->eType = static_cast<EFFECT_V2_TYPE>(type);
        document->Desc.eShape = CEffectV2Document::Shape_ForType(document->eType);
        m_Edit.Touch();
    }
    if (m_Edit.Resource_Kind() == EFFECT_V2_RESOURCE_KIND::GROUP)
    {
        if (auto* child = m_Edit.Selected_Child())
        {
            ImGui::SeparatorText("Element Lifetime");
            bool changed = Edit_Ms("Start", child->iStartMs);
            changed |= Edit_Ms("Duration (0 = native lifetime)", child->iDurationMs);
            int stop = static_cast<int>(child->eStop);
            if (ImGui::Combo("End", &stop, "Kill immediately\0Stop emission; keep remaining lifetime\0"))
            { child->eStop = static_cast<EFFECT_V2_CHILD_STOP>(stop); changed = true; }
            changed |= ImGui::DragFloat3("Offset", &child->vOffset.x, .01f);
            changed |= ImGui::DragFloat("Yaw", &child->fYawDegrees, .5f);
            changed |= ImGui::DragFloat("Pitch", &child->fPitchDegrees, .5f);
            changed |= ImGui::DragFloat("Roll", &child->fRollDegrees, .5f);
            changed |= ImGui::DragFloat3("Scale", &child->vScale.x, .01f, .001f, 1000.f);
            if (changed)
            {
                child->LocalTransform.vTranslation = child->vOffset;
                child->LocalTransform.vRotation = {child->fPitchDegrees, child->fYawDegrees, child->fRollDegrees};
                child->LocalTransform.vScale = child->vScale;
                m_Edit.Touch();
            }
        }
    }
    std::array<unsigned char, sizeof(CEffectV2Object::PARAMS)> before{};
    std::memcpy(before.data(), &document->Desc.Params, before.size());
    m_Editor.Render_DraftDetail(*document);
    if (std::memcmp(before.data(), &document->Desc.Params, before.size()) != 0) m_Edit.Touch();
}

void CEffectAuthoringV2Pane::Refresh_WorldObjects()
{
    m_WorldLoaded = true;
    Read_EffectCompositionWorldResources("LV_LUT_MIDNIGHTC_ED", m_WorldObjects, m_WorldRevision, m_WorldStatus);
}

bool CEffectAuthoringV2Pane::Extend_GroupToChild(const EFFECT_V2_DOCUMENT& document,
    const EFFECT_V2_GROUP_CHILD& child, uint32_t& outDuration, std::string& error) const
{
    const auto& params = document.Desc.Params;
    const double rate = (std::max)(.001, double(params.fPlayRate));
    const double tail = document.eType == EFFECT_V2_TYPE::PARTICLE ? params.Particle.vLifetime.y / rate :
        document.eType == EFFECT_V2_TYPE::TRAIL ? params.Trail.fPointLifetime / rate : 0.0;
    const double end = double(child.iStartMs) + child.iDurationMs + std::ceil(tail * 1000.0);
    if (!std::isfinite(end) || end > 600000.0)
    { error = "Motion and remaining lifetime exceed ten minutes; the Effect draft was preserved."; return false; }
    outDuration = (std::max)(m_Edit.Group().iDurationMs, static_cast<uint32_t>((std::max)(0.0, end)));
    return true;
}

void CEffectAuthoringV2Pane::Render_WorldObjects()
{
    if (!m_WorldLoaded) Refresh_WorldObjects();
    if (ImGui::Button("Reload saved World Objects")) Refresh_WorldObjects();
    if (!m_WorldStatus.empty()) ImGui::TextWrapped("%s", m_WorldStatus.c_str());
    for (const auto& entry : m_WorldObjects)
    {
        const auto& object = entry.resource;
        ImGui::PushID(object.objectId.c_str());
        const bool open = ImGui::TreeNodeEx("##WorldObject", ImGuiTreeNodeFlags_OpenOnArrow,
            "%s", Label(object.displayName, object.objectId));
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            auto* document = m_Edit.Selected_Document();
            if (!document) m_Status = "Select a Current Effect element first.";
            else if (document->eType != EFFECT_V2_TYPE::MESH && document->eType != EFFECT_V2_TYPE::PARTICLE)
                m_Status = "Select Mesh or Particle before binding a World Object mesh.";
            else if (!entry.error.empty()) m_Status = entry.error;
            else if (object.modelAssetId.empty()) m_Status = "This World Object is a sequence alias without a mesh.";
            else
            {
                document->Desc.strMeshAssetId = object.modelAssetId;
                document->Desc.TextureAssetIds[0] = object.diffuseTextureAssetId;
                document->Desc.Params.fMeshPreScale = object.modelPreScale;
                document->Desc.Params.Scale.vStart = document->Desc.Params.Scale.vEnd = object.scale;
                m_Edit.Touch();
                m_Status = "Bound saved World Object mesh, Base texture and scale to this Effect draft.";
            }
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%s", object.modelAssetId.c_str(), entry.error.c_str());
        if (open)
        {
            for (const auto& motion : entry.motions)
            {
                ImGui::PushID(motion.instanceId.c_str());
                if (ImGui::Selectable(Label(motion.displayName, motion.instanceId)))
                {
                    auto* document = m_Edit.Selected_Document();
                    auto* child = m_Edit.Selected_Child();
                    if (!document || !child) m_Status = "Select a Current Effect element before importing Motion.";
                    else if (m_Edit.Resource_Kind() == EFFECT_V2_RESOURCE_KIND::LEAF)
                        m_Status = "Use Create Group from this Effect first to retain the Motion's child timing.";
                    else
                    {
                        auto stagedDocument = *document;
                        auto stagedChild = *child;
                        uint32_t duration = 0u;
                        if (Apply_WorldMotionToEffect(entry, motion, stagedDocument, stagedChild, m_Status) &&
                            Extend_GroupToChild(stagedDocument, stagedChild, duration, m_Status))
                        {
                            *document = std::move(stagedDocument);
                            *child = std::move(stagedChild);
                            m_Edit.Group().iDurationMs = duration;
                            m_Edit.Touch();
                        }
                    }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%u ms | %.2fx | %zu clips\n%s",
                    motion.durationMs, motion.playbackSpeed, motion.animationTracks.size(), motion.error.c_str());
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}

void CEffectAuthoringV2Pane::Render_ResourceContents()
{
    if (!ImGui::BeginTabBar("##V2EffectResourceSlots")) return;
    if (ImGui::BeginTabItem("Effect Resources"))
    {
        if (auto* document = m_Edit.Selected_Document())
        {
            const auto mesh = document->Desc.strMeshAssetId;
            const auto textures = document->Desc.TextureAssetIds;
            m_Editor.Begin_CompositionFrame();
            m_Editor.Render_CompositionResources(*document);
            if (mesh != document->Desc.strMeshAssetId || textures != document->Desc.TextureAssetIds) m_Edit.Touch();
        }
        else ImGui::TextWrapped("Select a Current Effect element to assign resource slots.");
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("World Object Resource"))
    {
        Render_WorldObjects();
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}
}
