#include "imgui.h"
#include "CharacterModelWorkbench.h"
#include "ActorCatalog.h"
#include "AnimationEffectCueDocument.h"
#include "AnimationTargetService.h"
#include "CharacterPreviewPanel.h"
#include "CharacterActionCombatDocument.h"
#include "EffectAuthoringSequencer.h"
#include "EffectEditingSession.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <io.h>
#include <iterator>
#include <locale>
#include <set>
#include <sstream>
#include <stdexcept>

namespace Client
{
namespace
{
using Json = DATA_JSON_VALUE;
std::string Text(const Json& value, const char* key)
{ const auto* field = value.Find(key); return field && field->Is_String() ? field->Get_String() : std::string{}; }
uint32_t Number(const Json& value, const char* key)
{
    const auto* field = value.Find(key);
    if (!field || !field->Is_Number() || !std::isfinite(field->Get_Number()) || field->Get_Number() < 0.0 ||
        field->Get_Number() > UINT32_MAX || std::floor(field->Get_Number()) != field->Get_Number())
        throw std::runtime_error(std::string("Invalid unsigned integer: ") + key);
    return static_cast<uint32_t>(field->Get_Number());
}
const Json::ARRAY& Array(const Json& value, const char* key)
{ const auto* field = value.Find(key); if (!field || !field->Is_Array()) throw std::runtime_error(std::string("Missing array: ") + key); return field->Get_Array(); }
void Set(Json& value, const char* key, Json replacement)
{
    auto fields = value.Get_Object(); auto order = value.Get_ObjectInsertionOrder();
    if (!fields.contains(key)) order.push_back(key);
    fields[key] = std::move(replacement); value = Json::Object(std::move(fields), std::move(order));
}
std::string Serialize(const Json& value)
{
    std::ostringstream out; out.imbue(std::locale::classic()); out.precision(17);
    const auto write = [&](const auto& self, const Json& item) -> void {
        switch (item.Get_Type())
        {
        case DATA_JSON_TYPE::NULL_VALUE: out << "null"; break;
        case DATA_JSON_TYPE::BOOLEAN: out << (item.Get_Boolean() ? "true" : "false"); break;
        case DATA_JSON_TYPE::NUMBER: out << item.Get_Number(); break;
        case DATA_JSON_TYPE::STRING: out << '"' << CDataJson::Escape(item.Get_String()) << '"'; break;
        case DATA_JSON_TYPE::ARRAY:
            out << '['; for (size_t i = 0; i < item.Get_Array().size(); ++i) { if (i) out << ','; self(self, item.Get_Array()[i]); } out << ']'; break;
        case DATA_JSON_TYPE::OBJECT:
            out << '{'; { bool first = true; for (const auto& [key, child] : item.Get_Object()) { if (!first) out << ','; first = false; out << '"' << CDataJson::Escape(key) << "\":"; self(self, child); } } out << '}'; break;
        }
    };
    write(write, value); out << '\n'; return out.str();
}
bool Read(const std::filesystem::path& path, std::string& bytes, Json& document, std::string& status)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) { status = "Cannot read Product owner: " + path.string(); return false; }
    bytes.assign(std::istreambuf_iterator<char>(input), {});
    return !input.bad() && CDataJson::Parse(bytes, document, status) && document.Is_Object();
}
bool Commit(const std::filesystem::path& path, const std::string& baseline, const Json& document, std::string& status)
{
    const auto text = Serialize(document); Json parsed;
    if (!CDataJson::Parse(text, parsed, status) || Serialize(parsed) != text) return false;
    const auto lockPath = path.wstring() + L".writer.lock";
    HANDLE handle = CreateFileW(lockPath.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) { status = "Another writer owns this Product document; sequence preserved"; return false; }
    struct Guard { HANDLE handle; ~Guard() { CloseHandle(handle); } } guard{handle};
    std::string current; Json currentDocument;
    if (!Read(path, current, currentDocument, status) || current != baseline)
    { status = "Product document changed during Save; sequence preserved"; return false; }
    const auto temp = path.wstring() + L".tmp." + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(GetTickCount64());
    FILE* file = nullptr;
    if (_wfopen_s(&file, temp.c_str(), L"wb") || !file) { status = "Cannot stage Product changes"; return false; }
    const bool wrote = fwrite(text.data(), 1u, text.size(), file) == text.size();
    const bool flushed = fflush(file) == 0 && _commit(_fileno(file)) == 0;
    const bool closed = fclose(file) == 0;
    std::error_code error;
    std::string staged; Json stage;
    if (!wrote || !flushed || !closed || !Read(temp, staged, stage, status) || staged != text ||
        !Read(path, current, currentDocument, status) || current != baseline ||
        !CopyFileW(path.c_str(), (path.wstring() + L".previous").c_str(), FALSE) ||
        !MoveFileExW(temp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    { std::filesystem::remove(temp, error); status = "Product atomic save/freshness check failed; prior document preserved"; return false; }
    return true;
}
bool IdentityTransform(const ANIMATION_EFFECT_CUE& cue)
{
    const auto& t = cue.LocalTransform;
    return cue.strAnchorSlotId == "root" && t.vPosition.x == 0.f && t.vPosition.y == 0.f && t.vPosition.z == 0.f &&
        t.vRotationDegrees.x == 0.f && t.vRotationDegrees.y == 0.f && t.vRotationDegrees.z == 0.f &&
        t.vScale.x == 1.f && t.vScale.y == 1.f && t.vScale.z == 1.f;
}
}
CCharacterModelWorkbench::CCharacterModelWorkbench(std::shared_ptr<CCharacterPreviewPanel> panel,
    std::shared_ptr<CEffectAuthoringSequencer> sequencer) : m_Panel(std::move(panel)), m_Sequencer(std::move(sequencer)) {}
bool CCharacterModelWorkbench::Is_Dirty() const { return !m_Selected.id.empty() && m_Sequencer && m_Sequencer->Is_Dirty(); }
std::filesystem::path CCharacterModelWorkbench::Owner_Path(const ACTION& action) const
{
    if (action.sourceAction) return CProjectDataRoot::Resolve(std::filesystem::path("Animation/Authored") / action.asset / (action.asset + ".modelactions.json"));
    if (action.vehicleId) return CProjectDataRoot::Resolve("Actors/VehicleCatalog.json");
    if (action.asset == "MN_RPCZ_00-1") return CProjectDataRoot::Resolve("Animation/Authored/KoukuSaydon/Clown.interactionbindings.json");
    return CProjectDataRoot::Resolve(std::filesystem::path("Animation/Authored") / action.asset / (action.asset + ".interactionbindings.json"));
}
const Json* CCharacterModelWorkbench::Find_Action(const Json& root, const ACTION& action) const
{
    if (action.sourceAction)
    {
        for (const auto& value : Array(root, "actions")) if (Text(value, "actionId") == action.mode) return &value;
        return nullptr;
    }
    if (action.vehicleId)
    {
        for (const auto& vehicle : Array(root, "vehicles"))
            if (Number(vehicle, "vehicleId") == action.vehicleId)
            {
                if (action.locomotion || action.lifetime) return vehicle.Find(action.mode.c_str());
                for (const auto& skill : Array(vehicle, "skills")) if (Number(skill, "skillId") == action.skillId) return &skill;
            }
    }
    else for (const auto& mode : Array(root, "modes"))
        if (Text(mode, "mode") == action.mode)
        { const auto& skills = Array(mode, "skills"); return action.modeSlot < skills.size() ? &skills[action.modeSlot] : nullptr; }
    return nullptr;
}
bool CCharacterModelWorkbench::Replace_Action(Json& root, const ACTION& action, Json replacement) const
{
    if (action.sourceAction)
    {
        auto actions = Array(root, "actions");
        for (auto& value : actions) if (Text(value, "actionId") == action.mode)
        { value = std::move(replacement); Set(root, "actions", Json::Array(std::move(actions))); return true; }
        return false;
    }
    auto parents = Array(root, action.vehicleId ? "vehicles" : "modes");
    for (auto& parent : parents)
    {
        if (action.vehicleId ? Number(parent, "vehicleId") != action.vehicleId : Text(parent, "mode") != action.mode) continue;
        if (action.locomotion || action.lifetime) Set(parent, action.mode.c_str(), std::move(replacement));
        else
        {
            auto skills = Array(parent, "skills"); bool found = false;
            for (size_t index = 0; index < skills.size(); ++index)
                if (action.vehicleId ? Number(skills[index], "skillId") == action.skillId : index == action.modeSlot)
                { skills[index] = std::move(replacement); found = true; break; }
            if (!found) return false;
            Set(parent, "skills", Json::Array(std::move(skills)));
        }
        Set(root, action.vehicleId ? "vehicles" : "modes", Json::Array(std::move(parents))); return true;
    }
    return false;
}
bool CCharacterModelWorkbench::Refresh()
{
    try
    {
        std::vector<ACTION> actions;
        for (const auto& vehicle : CActorCatalog::Get_Vehicles())
        {
            if (vehicle.runtimeStatus != "supported") continue;
            const std::string id = "vehicle." + std::to_string(vehicle.vehicleId);
            const std::string asset = "Vehicle_" + std::to_string(vehicle.vehicleId);
            const std::string category = "Mounts / " + vehicle.archetypeId;
            actions.push_back({id + ".idle", category, "Idle", asset, "vehicleIdleClip", vehicle.vehicleId, 0u, 0u, true});
            actions.push_back({id + ".run", category, "Run / flight", asset, "vehicleRunClip", vehicle.vehicleId, 0u, 0u, true});
            for (const auto& skill : vehicle.skills)
                actions.push_back({id + ".skill." + std::to_string(skill.skillId), category,
                    skill.inputSlot + " | " + std::to_string(skill.skillId), asset, {}, vehicle.vehicleId, skill.skillId});
            Json vehicleDocument; std::string vehicleBytes, vehicleStatus;
            ACTION vehicleOwner; vehicleOwner.vehicleId = vehicle.vehicleId;
            if (Read(Owner_Path(vehicleOwner), vehicleBytes, vehicleDocument, vehicleStatus))
                for (const auto& value : Array(vehicleDocument, "vehicles"))
                    if (Number(value, "vehicleId") == vehicle.vehicleId)
                        for (const char* kind : {"ambientEffectCues", "mountEffectCues"})
                            if (const auto* rows = value.Find(kind); rows && rows->Is_Array() && !rows->Get_Array().empty())
                            {
                                ACTION lifetime; lifetime.id = id + "." + kind; lifetime.asset = asset;
                                lifetime.category = category + " / Mount effects"; lifetime.mode = kind;
                                lifetime.label = std::string(kind) == "ambientEffectCues" ? "Ambient / while mounted" : "Mount spawn / idle pose preview";
                                lifetime.vehicleId = vehicle.vehicleId; lifetime.lifetime = true; actions.push_back(std::move(lifetime));
                            }
            ACTION external; external.asset = asset; external.vehicleId = vehicle.vehicleId; external.sourceAction = true;
            Json source; std::string sourceBytes, sourceStatus;
            if (Read(Owner_Path(external), sourceBytes, source, sourceStatus))
            {
                if (Text(source, "schema") != "lostark.model-action-catalog" || Number(source, "formatVersion") != 1u ||
                    Text(source, "assetName") != asset || Text(source, "soundOwner") != "Vehicle")
                    throw std::runtime_error("Invalid model action catalog identity for " + asset);
                std::set<std::string> sourceIds;
                for (const auto& row : Array(source, "actions"))
                {
                    ACTION entry = external; entry.mode = Text(row, "actionId");
                    if (entry.mode.empty() || !sourceIds.insert(entry.mode).second) throw std::runtime_error("Duplicate source action ID");
                    entry.id = id + ".source." + entry.mode; entry.label = Text(row, "label");
                    entry.category = category + " / " + Text(row, "category"); actions.push_back(std::move(entry));
                }
            }
        }
        const char* owners[] = {"MN_RPCZ_00-1", "LanceMaster", "GunSlinger", "Slayer", "Artist", "DimensionMaster", "Warlord", "GuardianKnight"};
        for (const char* owner : owners)
        {
            ACTION base; base.asset = owner;
            Json document; std::string bytes, status;
            if (!Read(Owner_Path(base), bytes, document, status)) continue;
            for (const auto& mode : Array(document, "modes"))
            {
                base.mode = Text(mode, "mode");
                base.category = base.asset == "MN_RPCZ_00-1" ? "Clown / " + base.mode : base.asset + " / " + base.mode;
                const auto& skills = Array(mode, "skills");
                for (size_t index = 0; index < skills.size(); ++index)
                {
                    ACTION action = base; action.modeSlot = static_cast<uint32_t>(index);
                    action.id = "character." + action.asset + "." + action.mode + "." + std::to_string(index);
                    static const char* slots[] = {"Q", "W", "E", "R"};
                    action.label = (action.mode == "MAZE" ? "LMB / Q" : slots[(std::min)(index, size_t{3u})]) + std::string(" | ") + Text(skills[index], "clip");
                    actions.push_back(std::move(action));
                }
            }
        }
        m_Actions = std::move(actions); m_Loaded = true; return true;
    }
    catch (const std::exception& error) { m_Status = error.what(); return false; }
}
bool CCharacterModelWorkbench::Render_Actions(const bool locked)
{
    if (!m_Loaded) Refresh();
    ImGui::SeparatorText("Clown / interaction / mount actions");
    ImGui::BeginDisabled(locked);
    if (ImGui::SmallButton("Refresh interaction catalogs")) Refresh();
    bool selected = false; std::set<std::string> categories;
    for (const auto& action : m_Actions) categories.insert(action.category);
    for (const auto& category : categories)
        if (ImGui::TreeNode(category.c_str()))
        {
            for (const auto& action : m_Actions)
                if (action.category == category && ImGui::Selectable((action.label + "##" + action.id).c_str(), m_Selected.id == action.id))
                    selected = Select(action);
            ImGui::TreePop();
        }
    ImGui::EndDisabled();
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
    return selected;
}
bool CCharacterModelWorkbench::Select(const ACTION& action)
{
    try
    {
        if (Is_Dirty()) { m_Status = "Save this model sequence before switching actions"; return false; }
        Json root; std::string bytes;
        if (!Read(Owner_Path(action), bytes, root, m_Status)) return false;
        const auto* source = Find_Action(root, action);
        if (!source) { m_Status = "Product action is missing"; return false; }
        const Json baseline = *source;
        ANIMATION_SKILL_BINDING binding; binding.iSkillId = action.skillId; binding.Stages.push_back({});
        ANIMATION_EFFECT_CUE_DOCUMENT cues; cues.strAnimationAssetId = action.asset;
        ANIMATION_EFFECT_CUE_DOCUMENT externalCues; externalCues.strAnimationAssetId = action.asset;
        const Json* vehicleOwner = nullptr;
        if (action.vehicleId && !action.sourceAction)
            for (const auto& value : Array(root, "vehicles"))
                if (Number(value, "vehicleId") == action.vehicleId) vehicleOwner = &value;
        const auto appendLifetime = [&](const Json& rows, const std::string& clip, bool ambient, ANIMATION_EFFECT_CUE_DOCUMENT& target) {
            if (!rows.Is_Array()) throw std::runtime_error("Invalid mount lifetime cue array");
            for (const auto& value : rows.Get_Array())
            {
                ANIMATION_EFFECT_CUE cue; cue.strClipName = clip; cue.strEffectAssetId = Text(value, "effectAssetId");
                cue.iStartMs = Number(value, "startMs"); cue.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
                cue.eStopPolicy = ambient ? EFFECT_STOP_POLICY::CUE_END : EFFECT_STOP_POLICY::NATURAL;
                target.Cues.push_back(std::move(cue));
            }
        };
        auto& clips = binding.Stages.front().Clips;
        if (action.lifetime)
        {
            if (!vehicleOwner) throw std::runtime_error("Mount lifetime owner is missing");
            const std::string name = Text(*vehicleOwner, "vehicleIdleClip");
            clips.push_back({name, 0u, 1.f, 0u, CEffectEditingSession::New_Id("vehicle.lifetime.clip.")});
            appendLifetime(*source, name, action.mode == "ambientEffectCues", cues);
        }
        else if (action.vehicleId && !action.locomotion)
        {
            for (const auto& clip : Array(*source, action.sourceAction ? "clips" : "vehicleClips"))
            {
                if (action.sourceAction)
                {
                    const auto* rate = clip.Find("playRate");
                    if (!rate || !rate->Is_Number()) throw std::runtime_error("Invalid source action rate");
                    clips.push_back({Text(clip, "clip"), Number(clip, "playMs"), static_cast<float>(rate->Get_Number()),
                        Number(clip, "sourceStartMs"), CEffectEditingSession::New_Id("vehicle.clip.")});
                }
                else { if (!clip.Is_String()) throw std::runtime_error("Invalid mount clip"); clips.push_back({clip.Get_String(), 0u, 1.f, 0u, CEffectEditingSession::New_Id("vehicle.clip.")}); }
            }
            if (const auto* effects = source->Find("effectCues"))
                for (const auto& value : effects->Get_Array())
                {
                    const auto index = Number(value, "clipIndex"); if (index >= clips.size()) throw std::runtime_error("Mount effect clip index is invalid");
                    ANIMATION_EFFECT_CUE cue; cue.strClipName = clips[index].strClipName; cue.strEffectAssetId = Text(value, "effectAssetId"); cue.iStartMs = Number(value, "startMs");
                    cue.eFollowPolicy = Text(value, "followPolicy") == "SNAPSHOT" ? EFFECT_FOLLOW_POLICY::SNAPSHOT : EFFECT_FOLLOW_POLICY::FOLLOW;
                    cue.eStopPolicy = Text(value, "stopPolicy") == "CUE_END" ? EFFECT_STOP_POLICY::CUE_END : EFFECT_STOP_POLICY::NATURAL;
                    cues.Cues.push_back(std::move(cue));
                }
            if (const auto* sounds = source->Find("soundCues"))
                for (const auto& value : sounds->Get_Array())
                { const auto index = Number(value, "clipIndex"); if (index >= clips.size()) throw std::runtime_error("Mount sound clip index is invalid"); cues.Sounds.push_back({clips[index].strClipName, Number(value, "startMs"), Text(value, "event")}); }
        }
        else
        {
            const std::string name = action.locomotion ? (source->Is_String() ? source->Get_String() : "") : Text(*source, "clip");
            float rate = 1.f; if (const auto* value = source->Find("playRate")) { if (!value->Is_Number()) throw std::runtime_error("Invalid play rate"); rate = static_cast<float>(value->Get_Number()); }
            clips.push_back({name, 0u, rate, 0u, CEffectEditingSession::New_Id("interaction.clip.")});
            const std::string effect = Text(*source, "effectAssetId");
            if (!effect.empty()) { ANIMATION_EFFECT_CUE cue; cue.strClipName = name; cue.strEffectAssetId = effect; cues.Cues.push_back(std::move(cue)); }
        }
        if (clips.empty()) throw std::runtime_error("This action has no model clips");
        if (action.locomotion && vehicleOwner)
            if (const auto* rows = vehicleOwner->Find("ambientEffectCues")) appendLifetime(*rows, clips.front().strClipName, true, externalCues);
        if (!m_Panel || !m_Sequencer || !m_Panel->Select_TargetAsset(action.asset))
        { m_Status = m_Panel ? m_Panel->Get_Status() : "Preview is unavailable"; return false; }
        if (const auto model = CAnimationTargetService::Resolve_Model())
            for (auto* document : {&cues, &externalCues}) for (auto& cue : document->Cues) if (cue.eStopPolicy == EFFECT_STOP_POLICY::CUE_END)
                for (uint32_t index = 0u; index < model->Get_NumAnimations(); ++index)
                    if (cue.strClipName == model->Get_AnimationName(index))
                    {
                        float position = 0.f, ticks = 0.f; const auto rate = model->Get_AnimationTickPerSecond(index);
                        if (rate > 0.f && model->Get_AnimationProgress(index, position, ticks))
                            cue.iEndMs = static_cast<uint32_t>(std::ceil(ticks / rate * 1000.f));
                    }
        if (!m_Sequencer->Stage_CharacterAction(action.asset, binding, cues, {}, {}, action.vehicleId ? "Vehicle" : "KoukuSaydon", &externalCues))
        { m_Status = m_Sequencer->Status(); return false; }
        m_Selected = action; m_BaselineAction = baseline;
        if (!m_Sequencer->Open_CharacterModelSequence("action." + action.id)) m_Status = m_Sequencer->Status();
        else m_Status = "Product clips loaded. The shared sequencer saves animation, Effect, Sound, Collider and Camera rows.";
        return true;
    }
    catch (const std::exception& error) { m_Status = error.what(); return false; }
}
bool CCharacterModelWorkbench::Save_Product()
{
    try
    {
        ANIMATION_SKILL_BINDING binding; binding.iSkillId = m_Selected.skillId;
        ANIMATION_EFFECT_CUE_DOCUMENT cues;
        if (!m_Sequencer->Export_CharacterModelAction(binding, cues, m_Selected.vehicleId ? "Vehicle" : "KoukuSaydon", m_Status)) return false;
        const auto& clips = binding.Stages.front().Clips;
        if (clips.empty() || clips.size() > 16u) throw std::runtime_error("Product action requires 1 to 16 clips");
        if (!m_Selected.sourceAction)
            for (const auto& clip : clips)
                if (clip.iSourceStartMs || clip.iPlayMs || (m_Selected.vehicleId && clip.fPlayRate != 1.f))
                    throw std::runtime_error("Bake source trim/rate into an authored bone clip before assigning it to this Product owner");
        std::set<std::string> names;
        for (const auto& clip : clips)
            if (!names.insert(clip.strClipName).second) throw std::runtime_error("Product cue mapping needs unique clip names; combine repeated source stages into one authored clip");
        const auto indexOf = [&](const std::string& name) {
            const auto it = std::find_if(clips.begin(), clips.end(), [&](const auto& clip) { return clip.strClipName == name; });
            if (it == clips.end()) throw std::runtime_error("Cue has no exact clip occurrence");
            return static_cast<uint32_t>(it - clips.begin());
        };
        Json replacement = m_BaselineAction;
        if (m_Selected.lifetime)
        {
            if (clips.size() != 1u || !cues.Sounds.empty()) throw std::runtime_error("Mount lifetime owner stores only Effect rows; save the full sequence separately");
            Json::ARRAY effects;
            std::set<std::pair<std::string, uint32_t>> unique;
            for (const auto& cue : cues.Cues)
            {
                if (!IdentityTransform(cue) || cue.eFollowPolicy != EFFECT_FOLLOW_POLICY::FOLLOW)
                    throw std::runtime_error("Mount lifetime Effects follow the source model root; tune their internal anchors in the Effect editor");
                if (cue.iStartMs > 60000u || !unique.emplace(cue.strEffectAssetId, cue.iStartMs).second)
                    throw std::runtime_error("Mount lifetime cues require unique asset/start pairs and start <= 60000 ms");
                effects.push_back(Json::Object({{"effectAssetId", Json::String(cue.strEffectAssetId)}, {"startMs", Json::Number(cue.iStartMs)},
                    {"stopPolicy", Json::String(m_Selected.mode == "ambientEffectCues" ? "MOUNT_END" : "NATURAL")}}));
            }
            if (effects.size() > 256u) throw std::runtime_error("Too many mount lifetime cues");
            replacement = Json::Array(std::move(effects));
        }
        else if (m_Selected.locomotion)
        {
            if (clips.size() != 1u || !cues.Cues.empty() || !cues.Sounds.empty())
                throw std::runtime_error("Locomotion binding owns one animation; retain Effect/Sound rows with Save Effect Sequence");
            replacement = Json::String(clips.front().strClipName);
        }
        else if (!m_Selected.vehicleId)
        {
            if (clips.size() != 1u || cues.Cues.size() > 1u || !cues.Sounds.empty())
                throw std::runtime_error("Interaction binding owns one animation and one optional start Effect; retain additional rows with Save Effect Sequence");
            if (!cues.Cues.empty() && (cues.Cues.front().iStartMs || !IdentityTransform(cues.Cues.front())))
                throw std::runtime_error("Interaction start Effect requires the model root and zero start offset");
            Set(replacement, "clip", Json::String(clips.front().strClipName));
            Set(replacement, "playRate", Json::Number(clips.front().fPlayRate));
            auto fields = replacement.Get_Object(); fields.erase("effectAssetId");
            replacement = Json::Object(std::move(fields));
            if (!cues.Cues.empty()) Set(replacement, "effectAssetId", Json::String(cues.Cues.front().strEffectAssetId));
        }
        else
        {
            Json::ARRAY values;
            for (const auto& clip : clips)
            {
                if (m_Selected.sourceAction)
                    values.push_back(Json::Object({{"clip", Json::String(clip.strClipName)}, {"playRate", Json::Number(clip.fPlayRate)},
                        {"sourceStartMs", Json::Number(clip.iSourceStartMs)}, {"playMs", Json::Number(clip.iPlayMs)}}));
                else values.push_back(Json::String(clip.strClipName));
            }
            Set(replacement, m_Selected.sourceAction ? "clips" : "vehicleClips", Json::Array(std::move(values)));
            Json::ARRAY effects, sounds;
            for (const auto& cue : cues.Cues)
            {
                if (!IdentityTransform(cue)) throw std::runtime_error("Mount Product Effects use source root transforms; tune group transforms in the Effect document or save this sequence");
                effects.push_back(Json::Object({{"clipIndex", Json::Number(indexOf(cue.strClipName))}, {"effectAssetId", Json::String(cue.strEffectAssetId)},
                    {"startMs", Json::Number(cue.iStartMs)}, {"stopPolicy", Json::String(cue.eStopPolicy == EFFECT_STOP_POLICY::NATURAL ? "NATURAL" : "CUE_END")},
                    {"followPolicy", Json::String(cue.eFollowPolicy == EFFECT_FOLLOW_POLICY::SNAPSHOT ? "SNAPSHOT" : "FOLLOW")}}));
            }
            for (const auto& cue : cues.Sounds)
                sounds.push_back(Json::Object({{"clipIndex", Json::Number(indexOf(cue.strClipName))}, {"startMs", Json::Number(cue.iStartMs)}, {"event", Json::String(cue.strEventName)}}));
            if (effects.size() > 256u || sounds.size() > 256u) throw std::runtime_error("Too many Product cues");
            Set(replacement, "effectCues", Json::Array(std::move(effects))); Set(replacement, "soundCues", Json::Array(std::move(sounds)));
        }
        Json current; std::string currentBytes;
        if (!Read(Owner_Path(m_Selected), currentBytes, current, m_Status)) return false;
        const auto* currentAction = Find_Action(current, m_Selected);
        if (!currentAction || Serialize(*currentAction) != Serialize(m_BaselineAction))
            throw std::runtime_error("This action changed externally; saved owner and current sequence are preserved");
        if (!Replace_Action(current, m_Selected, replacement)) throw std::runtime_error("The selected stable action no longer exists");
        if (!Commit(Owner_Path(m_Selected), currentBytes, current, m_Status)) return false;
        m_BaselineAction = std::move(replacement);
        m_Status = m_Selected.sourceAction ? "Saved source action clips and cues; Save Effect Sequence retains all editable rows." :
            "Saved Product presentation binding. It is read on the next Character spawn / Client catalog load. Save Effect Sequence also retains the full authoring arrangement.";
        return true;
    }
    catch (const std::exception& error) { m_Status = error.what(); return false; }
}
void CCharacterModelWorkbench::Render(const COMPOSITION_WORKBENCH_PANE pane)
{
    if (!m_Sequencer || m_Selected.id.empty()) { ImGui::TextDisabled("Select a clown or mount action."); return; }
    switch (pane)
    {
    case COMPOSITION_WORKBENCH_PANE::SEQUENCER: m_Sequencer->Render_Sequencer("Character model sequence", false, true); break;
    case COMPOSITION_WORKBENCH_PANE::RESOURCES: m_Sequencer->Render_WorkbenchResources(); break;
    case COMPOSITION_WORKBENCH_PANE::DETAILS:
        ImGui::TextWrapped("%s / %s", m_Selected.category.c_str(), m_Selected.label.c_str());
        if (ImGui::Button(m_Selected.sourceAction ? "Save Source Action" : "Save Product Binding")) Save_Product();
        if (m_Selected.locomotion) ImGui::TextWrapped("Ambient rows preview the mount lifetime owner. Save Product Binding changes this animation only; edit Ambient / while mounted to save its Effects.");
        if (m_Selected.lifetime) ImGui::TextWrapped("Ambient ends with riding; Mount spawn plays once. This view uses the idle pose and ends ambient with the preview. Save Product Binding stores the lifetime cues.");
        ImGui::TextWrapped("%s", m_Status.c_str());
        m_Sequencer->Render_WorkbenchDetail(); break;
    case COMPOSITION_WORKBENCH_PANE::TOOLBAR:
    case COMPOSITION_WORKBENCH_PANE::PREVIEW:
        ImGui::TextWrapped("%s", m_Selected.asset.c_str());
        if (ImGui::Button("Preview Play")) m_Sequencer->Play();
        ImGui::SameLine(); if (ImGui::Button("Stop Preview")) m_Sequencer->Stop();
        ImGui::TextWrapped("%s", m_Sequencer->Status().c_str()); break;
    default: break;
    }
}
}
