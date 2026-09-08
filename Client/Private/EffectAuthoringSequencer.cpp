#include "imgui.h"
#include "EffectAuthoringSequencer.h"
#include "ActionPresentationTimeline.h"
#include "AnimationSkillBindingDocument.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CompositionTimeline.h"
#include "DataJson.h"
#include "EffectEditingSession.h"
#include "Effect_Object.h"
#include "GameInstance.h"
#include "Model.h"
#include "PlayerSkillCatalog.h"
#include "ProjectDataRoot.h"
#include "ValtanPatternTree.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <locale>
#include <map>
#include <set>
#include <sstream>

namespace Client
{
namespace
{
constexpr std::uint32_t MAX_MS = 600000u;
constexpr std::size_t MAX_EFFECTS = 256u;
const char* Label(const std::string& name, const std::string& id) { return name.empty() ? id.c_str() : name.c_str(); }
bool Clip_Metadata(const std::shared_ptr<Engine::CModel>& model, const std::string& name,
    std::uint32_t& index, float& duration, float& tickRate)
{
    if (!model) return false;
    index = UINT32_MAX;
    for (std::uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
        if (const auto* value = model->Get_AnimationName(i); value && name == value)
        { if (index != UINT32_MAX) return false; index = i; }
    float position = 0.f;
    if (index == UINT32_MAX || !model->Get_AnimationProgress(index, position, duration)) return false;
    tickRate = model->Get_AnimationTickPerSecond(index);
    return std::isfinite(duration) && duration > 0.f && std::isfinite(tickRate) && tickRate > 0.f;
}
bool Read_SequenceText(const std::filesystem::path& path, std::string& text, bool& exists, std::string& error)
{
    std::error_code ec; exists = std::filesystem::exists(path, ec);
    if (ec) { error = "Cannot inspect sequence: " + ec.message(); return false; }
    if (!exists) { text.clear(); return true; }
    const auto size = std::filesystem::file_size(path, ec);
    if (ec || size > 1024u * 1024u) { error = "Sequence exceeds the 1 MiB authoring limit."; return false; }
    std::ifstream input(path, std::ios::binary);
    if (!input) { error = "Cannot read " + path.generic_string(); return false; }
    text.assign(std::istreambuf_iterator<char>(input), {});
    if (input.bad()) { error = "Sequence read failed."; return false; }
    return true;
}
bool Json_Text(const DATA_JSON_VALUE& object, const char* key, std::string& value)
{
    const auto* field = object.Find(key);
    if (!field || !field->Is_String()) return false;
    value = field->Get_String(); return true;
}
bool Json_U32(const DATA_JSON_VALUE& object, const char* key, std::uint32_t& value, std::uint32_t limit = MAX_MS)
{
    const auto* field = object.Find(key);
    if (!field || !field->Is_Number() || !std::isfinite(field->Get_Number()) || field->Get_Number() < 0 ||
        field->Get_Number() > limit || std::floor(field->Get_Number()) != field->Get_Number()) return false;
    value = static_cast<std::uint32_t>(field->Get_Number()); return true;
}
bool Json_Vector(const DATA_JSON_VALUE& object, const char* key, float3_t& value)
{
    const auto* field = object.Find(key);
    if (!field || !field->Is_Array() || field->Get_Array().size() != 3u) return false;
    float* components[] = {&value.x, &value.y, &value.z};
    for (std::size_t i = 0; i < 3u; ++i)
    {
        const auto& entry = field->Get_Array()[i];
        if (!entry.Is_Number() || !std::isfinite(entry.Get_Number()) || std::abs(entry.Get_Number()) > 100000.f) return false;
        *components[i] = static_cast<float>(entry.Get_Number());
    }
    return true;
}
const char* Owner_Key(const EFFECT_RESOURCE_OWNER_KIND kind)
{
    switch (kind) { case EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT: return "V1_DOCUMENT";
    case EFFECT_RESOURCE_OWNER_KIND::V2_LEAF: return "V2_LEAF"; case EFFECT_RESOURCE_OWNER_KIND::V2_GROUP: return "V2_GROUP";
    default: return ""; }
}
}

CEffectAuthoringSequencer::CEffectAuthoringSequencer(ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context, std::shared_ptr<CCharacterPreviewPanel> panel)
    : m_Device(std::move(device)), m_Context(std::move(context)), m_Panel(std::move(panel))
{
    XMStoreFloat4x4(&m_WorldRoot, XMMatrixIdentity());
    if (const auto* world = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))
    { const auto matrix = XMLoadFloat4x4(world); XMStoreFloat4x4(&m_WorldRoot, XMMatrixTranslationFromVector(matrix.r[3] + XMVector3Normalize(matrix.r[2]) * 5.f)); }
}
CEffectAuthoringSequencer::~CEffectAuthoringSequencer() { Stop(); }
void CEffectAuthoringSequencer::Set_Player(CKoukuSaydonPresentationPlayer* player)
{
    if (player == m_Player) return;
    Stop(); m_Player = player; m_Kouku.Set_Player(player);
}
void CEffectAuthoringSequencer::Set_V1Callbacks(V1_FACTORY factory, V1_RELEASE release)
{
    Stop(); m_V1Factory = std::move(factory); m_V1Release = std::move(release);
}
void CEffectAuthoringSequencer::Set_V1AnchorProvider(V1_ANCHOR_PROVIDER provider)
{
    Stop(); m_V1Anchors = std::move(provider);
}
void CEffectAuthoringSequencer::Set_V2SnapshotProvider(V2_SNAPSHOT_PROVIDER provider) { m_V2Provider = std::move(provider); }
bool CEffectAuthoringSequencer::Consume_InteractionRequest() { const bool result = m_Interaction; m_Interaction = false; return result; }
const CEffectAuthoringSequencer::MODEL_SEQUENCE* CEffectAuthoringSequencer::Selected_Sequence() const
{
    const auto found = std::find_if(m_Sequences.begin(), m_Sequences.end(), [&](const auto& value) { return value.id == m_SelectedSequence; });
    return found == m_Sequences.end() ? nullptr : &*found;
}
std::uint32_t CEffectAuthoringSequencer::DurationMs() const
{
    std::uint32_t result = m_UseKouku ? m_Kouku.DurationMs() : 0u;
    if (const auto* sequence = Selected_Sequence(); !m_UseKouku && sequence) result = sequence->durationMs;
    if (m_Transient) result = (std::max)(result, m_Transient->startMs + m_Transient->durationMs);
    else for (const auto& row : m_Effects) result = (std::max)(result, row.startMs + row.durationMs);
    const auto& cameras = m_Transient ? m_TransientCameraRows : m_CameraRows;
    for (const auto& row : cameras) result = (std::max)(result, row.startMs + row.cue.iDurationMs);
    return (std::clamp)(result, 1u, MAX_MS);
}
bool CEffectAuthoringSequencer::Reload_ModelSequences()
{
    const auto model = CAnimationTargetService::Resolve_Model();
    if (!model || !m_Panel || !m_Panel->Is_PreviewActive())
    { m_Status = "Select a preview character or boss in Model View first."; return false; }
    const auto asset = CAnimationTargetService::Resolve_AssetName();
    const auto generation = CAnimationTargetService::Resolve_TargetGeneration();
    std::vector<std::string> names;
    for (std::uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
        if (const auto* name = model->Get_AnimationName(i)) names.emplace_back(name);
    std::vector<MODEL_SEQUENCE> staged;
    const auto addClip = [&](MODEL_SEQUENCE& sequence, CLIP clip, const std::uint32_t wallMs)
    {
        std::uint32_t index = 0u; float native = 0.f, rate = 0.f;
        if (!Clip_Metadata(model, clip.clipName, index, native, rate))
        { sequence.error = "Missing/ambiguous model clip: " + clip.clipName; return; }
        const ACTION_PRESENTATION_CLIP_TIMING timing{native / rate, clip.sourcePlayMs, clip.playRate, clip.loop, clip.sourceStartMs * .001f};
        float sourceDuration = 0.f, wallDuration = 0.f;
        if (!CActionPresentationTimeline::Resolve_ClipDuration(timing, sourceDuration, wallDuration))
        { sequence.error = "Invalid source window: " + clip.clipName; return; }
        const auto duration = wallMs ? wallMs : static_cast<std::uint32_t>(std::ceil(wallDuration * 1000.f));
        if (!duration || duration > MAX_MS || sequence.durationMs > MAX_MS - duration)
        { sequence.error = "Animation sequence exceeds 600 seconds."; return; }
        clip.startMs = sequence.durationMs; clip.durationMs = duration;
        sequence.durationMs += duration; sequence.clips.push_back(std::move(clip));
    };
    if (asset == "Valtan")
    {
        VALTAN_PATTERN_TREE_VIEW view;
        if (!CValtanPatternTree::Load(view, m_Status)) return false;
        for (const auto* collection : {&view.Rotation, &view.Gimmicks})
            for (const auto& pattern : *collection)
            {
                MODEL_SEQUENCE sequence; sequence.id = pattern.strPatternId; sequence.label = pattern.strDisplayName;
                std::vector<const VALTAN_STAGE_VIEW*> stages;
                if (!CValtanPatternTree::Build_PreviewStagePath(pattern, VALTAN_PATTERN_PREVIEW_PATH::NORMAL, stages, sequence.error))
                { staged.push_back(std::move(sequence)); continue; }
                for (const auto* stage : stages)
                {
                    const auto stageStart = sequence.durationMs;
                    if (stage->bSuppressAnimation)
                    {
                        CLIP hold; hold.id = stage->strActionId; hold.label = stage->strStageId + " (hold pose)";
                        hold.startMs = stageStart; hold.durationMs = stage->iDurationMs;
                        sequence.clips.push_back(std::move(hold));
                        sequence.durationMs += stage->iDurationMs;
                    }
                    else for (const auto& authored : stage->ClipOccurrences)
                    {
                        CLIP clip; clip.id = authored.strClipOccurrenceId; clip.label = authored.strClipName;
                        clip.clipName = authored.strClipName; clip.sourceStartMs = authored.iSourceStartMs;
                        clip.sourcePlayMs = authored.iPlayMs; clip.playRate = authored.fPlayRate; clip.loop = authored.bLoop;
                        addClip(sequence, std::move(clip), authored.iAuthoringWallMs);
                    }
                    if (sequence.durationMs - stageStart != stage->iDurationMs)
                        sequence.error = "Product clip wall durations do not cover stage " + stage->strStageId;
                    if (sequence.durationMs > MAX_MS) { sequence.error = "Pattern exceeds 600 seconds."; break; }
                }
                staged.push_back(std::move(sequence));
            }
    }
    else if (const auto character = CAnimationTargetService::Resolve_Character(); character && character->Get_Spec() && asset == character->Get_Spec()->pAssetName)
    {
        if (!CPlayerSkillCatalog::Load(m_Status)) return false;
        ANIMATION_SKILL_BINDING_DOCUMENT bindings;
        if (!CAnimationSkillBindingDocument::Load(asset, character->Get_Spec()->eCharacterClass,
            CPlayerSkillCatalog::Get_Skills(), names, bindings, m_Status)) return false;
        for (const auto& binding : bindings.Bindings)
        {
            const auto* skill = CPlayerSkillCatalog::Find_ById(binding.iSkillId);
            MODEL_SEQUENCE sequence; sequence.id = "skill." + std::to_string(binding.iSkillId);
            sequence.label = skill ? skill->strInputSlot + " / " + skill->strDisplayName : sequence.id;
            const bool combo = skill && skill->eSkillKind == LostArk::Shared::PLAYER_SKILL_KIND::COMBO;
            std::vector<MODEL_SEQUENCE> stageSequences;
            std::uint32_t stageOrdinal = 0u;
            for (const auto& stage : binding.Stages)
            {
                MODEL_SEQUENCE stageSequence;
                stageSequence.id = sequence.id + ".stage." + std::to_string(stageOrdinal);
                stageSequence.label = sequence.label + " / Stage " + std::to_string(stageOrdinal + 1u);
                std::uint32_t clipOrdinal = 0u;
                for (const auto& authored : stage.Clips)
                {
                    // The subset retains the binding's original identity; only its local clock starts at zero.
                    CLIP clip; clip.id = sequence.id + ".stage." + std::to_string(stageOrdinal) + ".clip." + std::to_string(clipOrdinal++);
                    clip.label = authored.strClipName; clip.clipName = authored.strClipName;
                    clip.sourceStartMs = authored.iSourceStartMs; clip.sourcePlayMs = authored.iPlayMs; clip.playRate = authored.fPlayRate;
                    if (combo) addClip(stageSequence, clip, 0u);
                    addClip(sequence, std::move(clip), 0u);
                }
                if (combo)
                {
                    if (stageSequence.clips.empty() && stageSequence.error.empty()) stageSequence.error = "The saved combo stage has no model clips.";
                    stageSequences.push_back(std::move(stageSequence));
                }
                ++stageOrdinal;
            }
            staged.push_back(std::move(sequence));
            for (auto& stageSequence : stageSequences) staged.push_back(std::move(stageSequence));
        }
    }
    else
    { m_Status = "This preview model has no Product skill/pattern binding. Its native clip controls remain available."; return false; }
    if (generation != CAnimationTargetService::Resolve_TargetGeneration())
    { m_Status = "Model target changed while reading its animation bindings."; return false; }
    Stop(); m_Sequences = std::move(staged); m_AssetName = asset; m_InventoryGeneration = generation;
    if (!Selected_Sequence()) m_SelectedSequence.clear();
    m_Status = "Loaded saved Product animation order. Model rows are read-only.";
    return true;
}
bool CEffectAuthoringSequencer::Select_CharacterSkill(const std::string& asset, const std::uint32_t skillId,
    const std::optional<std::uint32_t> stageIndex)
{
    if (!m_Panel || asset.empty() || !skillId)
    { m_Status = "The recovery Effect has no admitted character/skill target."; return false; }
    if ((!m_Panel->Is_PreviewActive() || CAnimationTargetService::Resolve_AssetName() != asset) &&
        !m_Panel->Select_TargetAsset(asset))
    { m_Status = m_Panel->Get_Status(); return false; }
    if (m_AssetName != asset || m_InventoryGeneration != CAnimationTargetService::Resolve_TargetGeneration() ||
        m_Sequences.empty())
        if (!Reload_ModelSequences()) return false;
    if (stageIndex)
    {
        const auto* skill = CPlayerSkillCatalog::Find_ById(skillId);
        if (!skill || skill->eSkillKind != LostArk::Shared::PLAYER_SKILL_KIND::COMBO || *stageIndex >= skill->iComboStageCount)
        { m_Status = "The requested recovery Effect combo stage is not present in the Product skill binding."; return false; }
    }
    const auto id = "skill." + std::to_string(skillId);
    return Select_ModelSequence(stageIndex ? id + ".stage." + std::to_string(*stageIndex) : id);
}

bool CEffectAuthoringSequencer::Select_ModelSequence(const std::string& id)
{
    const auto found = std::find_if(m_Sequences.begin(), m_Sequences.end(), [&](const auto& row) { return row.id == id; });
    if (found == m_Sequences.end() || !found->error.empty())
    { m_Status = found == m_Sequences.end() ? "Saved animation sequence is unavailable." : found->error; return false; }
    Stop(); m_UseKouku = false; m_SelectedSequence = id; m_AnchorMember.clear(); m_ClockMs = 0;
    m_Dirty = true; m_Status = "Selected " + found->label; return true;
}
bool CEffectAuthoringSequencer::Select_Kouku(const std::string& id, const bool bundle)
{
    Stop();
    if (!(bundle ? m_Kouku.Select_Bundle(id) : m_Kouku.Select_Pattern(id))) { m_Status = m_Kouku.Status(); return false; }
    m_UseKouku = true; m_ClockMs = 0;
    m_AnchorMember = m_Kouku.Actors().empty() ? "" : m_Kouku.Actors().front().memberId;
    m_Dirty = true; m_Status = m_Kouku.Status(); return true;
}
bool CEffectAuthoringSequencer::Begin_Model()
{
    if (m_UseKouku)
    {
        if (m_Kouku.Is_Active()) return Sample_Model(ClockMs());
        if (!m_Kouku.Begin(ClockMs(), true)) { m_Status = m_Kouku.Status(); return false; }
        return true;
    }
    if (m_SelectedSequence.empty()) return true;
    const auto model = CAnimationTargetService::Resolve_Model();
    if (!model || !m_Panel || !m_Panel->Is_PreviewActive() || m_InventoryGeneration != CAnimationTargetService::Resolve_TargetGeneration() ||
        m_AssetName != CAnimationTargetService::Resolve_AssetName())
    { m_Status = "Model target changed. Reload its saved animation bindings."; return false; }
    if (m_Model.expired())
    {
        m_Model = model; m_ModelGeneration = m_InventoryGeneration;
        m_PreviousClip = model->Get_CurrentAnimIndex(); m_PreviousPaused = model->Is_AnimPaused(); m_PreviousLoop = model->Is_AnimLoop();
        float duration = 0.f; model->Get_AnimationProgress(m_PreviousClip, m_PreviousPosition, duration);
    }
    return Sample_Model(ClockMs());
}
bool CEffectAuthoringSequencer::Sample_Model(const std::uint32_t clockMs)
{
    if (m_UseKouku)
    {
        if (!m_Kouku.Is_Active() || !m_Kouku.Sample(clockMs, true)) { m_Status = "Kouku model preview owner changed."; return false; }
        return true;
    }
    const auto* sequence = Selected_Sequence();
    if (!sequence || m_SelectedSequence.empty()) return true;
    const auto model = m_Model.lock();
    if (!model || model != CAnimationTargetService::Resolve_Model() || m_ModelGeneration != CAnimationTargetService::Resolve_TargetGeneration())
    { m_Status = "Preview model target was replaced."; return false; }
    const CLIP* selected = nullptr;
    for (const auto& clip : sequence->clips)
    {
        if (clip.startMs > clockMs) break;
        if (!clip.clipName.empty()) selected = &clip;
    }
    if (!selected) { model->Set_AnimPaused(true); return true; }
    std::uint32_t index = 0u; float native = 0.f, tickRate = 0.f;
    if (!Clip_Metadata(model, selected->clipName, index, native, tickRate)) { m_Status = "Selected model clip is unavailable."; return false; }
    const ACTION_PRESENTATION_CLIP_TIMING timing{native / tickRate, selected->sourcePlayMs, selected->playRate, selected->loop, selected->sourceStartMs * .001f};
    const float budget = selected->durationMs * .001f;
    ACTION_PRESENTATION_SAMPLE sample;
    if (!CActionPresentationTimeline::Resolve_PreviewSequenceSample(std::span(&timing, 1u), std::span(&budget, 1u),
        (std::min)(clockMs - selected->startMs, selected->durationMs) * .001f, sample))
    { m_Status = "Could not sample the Product animation source clock."; return false; }
    model->Set_Animation(index, false); model->Skip_Blend(); model->Set_AnimPaused(true);
    if (!model->Set_AnimTrackPosition(index, sample.fClipSourceTimeSeconds * tickRate)) return false;
    model->Play_Animation(0.f); m_Panel->Synchronize_PreviewWeapon(); return true;
}
bool CEffectAuthoringSequencer::Resolve_Root(float4x4_t& root)
{
    if (m_UseKouku)
    {
        EFFECT_V2_TARGET target; EFFECT_V2_TARGET_VIEW view;
        if (!m_Kouku.Resolve_Target(m_AnchorMember, target, view) ||
            !CEffectV2Object::Resolve_TargetPivot(view, "", CEffectV2Object::PIVOT_ROTATION::TARGET_YAW, root))
        { m_Status = "Select an available model member for the Effect anchor."; return false; }
        return true;
    }
    if (m_ModelRoot && m_Panel && m_Panel->Is_PreviewActive()) return CAnimationTargetService::Resolve_RootTransform(&root);
    root = m_WorldRoot; return true;
}
bool CEffectAuthoringSequencer::Validate_Anchor(const std::string& anchor, const bool modelRoot, const bool useKouku)
{
    if (anchor == "root") return true;
    float4x4_t world;
    if (anchor.empty() || anchor.size() > 128u || !modelRoot || useKouku ||
        !m_Panel || !m_Panel->Is_PreviewActive() ||
        !CAnimationTargetService::Resolve_AnchorTransform(anchor.c_str(), &world))
    { m_Status = "Named Effect anchor '" + anchor + "' requires its loaded character bone/socket and Model root mode."; return false; }
    return true;
}

bool CEffectAuthoringSequencer::Render_AnchorChoice(const char* label, std::string& anchor)
{
    bool changed = false;
    if (ImGui::BeginCombo(label, anchor.c_str()))
    {
        if (ImGui::Selectable("root", anchor == "root")) { anchor = "root"; changed = true; }
        const auto model = CAnimationTargetService::Resolve_Model();
        if (m_ModelRoot && !m_UseKouku && m_Panel && m_Panel->Is_PreviewActive() && model)
            for (const auto& bone : model->Get_BoneNames())
            {
                if (bone == "root") continue;
                if (ImGui::Selectable(bone.c_str(), anchor == bone)) { anchor = bone; changed = true; }
            }
        ImGui::EndCombo();
    }
    return changed;
}

bool CEffectAuthoringSequencer::Resolve_RowPivot(const EFFECT_ROW& row, const float4x4_t& root, float4x4_t& pivot)
{
    if (!Validate_Anchor(row.anchorSlotId, m_ModelRoot, m_UseKouku)) return false;
    matrix_t anchor = XMLoadFloat4x4(&root);
    if (row.anchorSlotId != "root")
    {
        float4x4_t modelRoot, boneWorld;
        if (!CAnimationTargetService::Resolve_RootTransform(&modelRoot) ||
            !CAnimationTargetService::Resolve_AnchorTransform(row.anchorSlotId.c_str(), &boneWorld))
        { m_Status = "The selected Effect bone/socket is unavailable."; return false; }
        vector_t determinant;
        const matrix_t inverse = XMMatrixInverse(&determinant, XMLoadFloat4x4(&modelRoot));
        if (!std::isfinite(XMVectorGetX(determinant)) || std::fabs(XMVectorGetX(determinant)) < 1e-12f)
        { m_Status = "The selected model root is singular."; return false; }
        anchor = XMLoadFloat4x4(&boneWorld) * inverse * anchor;
    }
    XMStoreFloat4x4(&pivot, XMMatrixTranslation(row.offset.x, row.offset.y, row.offset.z) * anchor);
    return true;
}

bool CEffectAuthoringSequencer::Record_RowPivot(EFFECT_ROW& row, const float4x4_t& pivot, const float age)
{
    if (!row.history) row.history = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
    if (age < row.recordedAge) return true;
    const auto record = [&](float seconds, const float4x4_t& world)
    {
        const float dx = world._41 - row.recordedRoot._41, dy = world._42 - row.recordedRoot._42, dz = world._43 - row.recordedRoot._43;
        const bool discontinuity = row.recordedAge >= 0.f && dx*dx + dy*dy + dz*dz > 2500.f;
        if (!row.history->Record(seconds, world, discontinuity, m_Status)) return false;
        row.recordedAge = seconds; row.recordedRoot = world; return true;
    };
    if (row.anchorSlotId == "root")
    {
        if (row.recordedAge < 0.f && age > 0.f && !record(0.f, pivot)) return false;
        return record(age, pivot);
    }
    // Named pivots move with animation. Reuse the source-anchor 60 Hz pose
    // sampling contract instead of seeding earlier births with today's pose.
    const bool ownsSequence = !m_UseKouku && !m_SelectedSequence.empty();
    if (age > 0.f && row.recordedAge < 0.f && !ownsSequence)
    { m_Status = "Bone-anchor seek requires recorded history or a selected saved animation sequence."; return false; }
    bool moved = false, success = true;
    if (ownsSequence)
    {
        const auto first = row.recordedAge < 0.f ? 0u : static_cast<std::uint32_t>(std::floor(double(row.recordedAge) * 60.0)) + 1u;
        for (auto step = first; double(step) / 60.0 < double(age) - 0.000001; ++step)
        {
            const float seconds = static_cast<float>(double(step) / 60.0);
            if (seconds <= row.recordedAge) continue;
            moved = true;
            float4x4_t root, past;
            if (!Sample_Model(row.startMs + static_cast<std::uint32_t>(std::llround(double(seconds) * 1000.0))) ||
                !Resolve_Root(root) || !Resolve_RowPivot(row, root, past) || !record(seconds, past))
            { success = false; break; }
        }
    }
    const auto failure = m_Status;
    if (moved && !Sample_Model(ClockMs())) return false;
    if (!success) { m_Status = failure; return false; }
    return record(age, pivot);
}

void CEffectAuthoringSequencer::Release_Row(EFFECT_ROW& row)
{
    if (row.v2) CEffectV2Runtime::Stop_Group(row.v2);
    row.v2 = 0u;
    row.sampledAge = -1.f;
    if (row.v1) { row.v1->Set_Playing(false); row.v1->Set_Visible(false); if (m_V1Release) m_V1Release(row.v1); row.v1.reset(); }
}
bool CEffectAuthoringSequencer::Stage_Row(EFFECT_ROW& row, const float4x4_t& root)
{
    if (!row.key.Is_Valid() || !row.durationMs || row.durationMs > MAX_MS || row.startMs > MAX_MS - row.durationMs)
    { m_Status = "Invalid Effect resource or occurrence timing."; return false; }
    if (!Validate_Anchor(row.anchorSlotId, m_ModelRoot, m_UseKouku)) return false;
    if (row.muted) return true;
    if (row.key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT)
    {
        if (!m_V1Factory || !m_V1Release) { m_Status = "Effect document playback owner is unavailable."; return false; }
        if (!m_V1Factory(row.key, root, row.v1, m_Status) || !row.v1)
        { Release_Row(row); return false; }
        row.v1->Set_Playing(false); row.v1->Set_Visible(false); return true;
    }
    if (!row.snapshot)
    {
        const auto kind = row.key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V2_LEAF ? EFFECT_V2_RESOURCE_KIND::LEAF : EFFECT_V2_RESOURCE_KIND::GROUP;
        if (!(m_V2Provider ? m_V2Provider(row.key, row.snapshot, m_Status) :
            CEffectV2Catalog::Get().Load_ResourceSnapshot(kind, row.key.strStableId, row.snapshot, m_Status))) return false;
    }
    if (!row.snapshot || !row.snapshot->Is_Ready()) { m_Status = "Effect resource snapshot is unavailable."; return false; }
    EFFECT_V2_GROUP leafGroup;
    const EFFECT_V2_GROUP* group = nullptr;
    if (row.key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V2_GROUP) group = row.snapshot->Find_Group(row.key.strStableId);
    else if (row.snapshot->Find_Document(row.key.strStableId))
    {
        leafGroup.strGroupId = "effect.sequencer.preview";
        auto& child = leafGroup.Children.emplace_back(); child.strChildId = "effect.sequencer.preview.child";
        child.strResourceId = child.strEffectId = row.key.strStableId; group = &leafGroup;
    }
    if (!group) { m_Status = "Typed Effect snapshot does not contain " + row.key.strStableId; return false; }
    if (!CEffectV2Runtime::Prewarm_Group(*group, row.snapshot, m_Device, m_Context))
    { m_Status = CEffectV2Runtime::Last_Error(); return false; }
    return true;
}
bool CEffectAuthoringSequencer::Sample_Row(EFFECT_ROW& row, const float4x4_t& root)
{
    const bool visible = !row.muted && m_ClockMs >= row.startMs && m_ClockMs < double(row.startMs) + row.durationMs;
    if (!visible)
    {
        if (row.v1) row.v1->Set_Visible(false);
        if (row.v2) { CEffectV2Runtime::Stop_Group(row.v2); row.v2 = 0u; }
        return true;
    }
    float4x4_t pivot;
    if (!Resolve_RowPivot(row, root, pivot)) return false;
    const float age = static_cast<float>((m_ClockMs - row.startMs) * .001);
    if (!Record_RowPivot(row, pivot, age)) return false;
    const auto history = row.history;
    if (row.key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT)
    {
        if (!row.v1 && !Stage_Row(row, pivot)) return false;
        // SourceRecipe sockets remain absolute owner-relative attachments.
        // The user occurrence bone controls free/root elements only; applying
        // that bone again to native socket worlds would double-transform them.
        float4x4_t sourceOwner;
        XMStoreFloat4x4(&sourceOwner, XMMatrixTranslation(row.offset.x, row.offset.y, row.offset.z) * XMLoadFloat4x4(&root));
        if (!Record_V1Anchors(row, sourceOwner, age)) return false;
        row.v1->Set_Playing(false);
        const auto anchors = row.anchorHistory;
        const auto provider = [history, anchors](float seconds, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& sample, std::string& error)
        {
            if (!history->Sample(seconds, sample.RootWorld, error)) return false;
            sample.SourceAnchorWorlds.clear();
            if (anchors)
                for (const auto& [slot, values] : anchors->slots)
                {
                    float4x4_t world;
                    if (!values.Sample(seconds, world, error)) { error = "Source anchor '" + slot + "': " + error; return false; }
                    sample.SourceAnchorWorlds.emplace(slot, world);
                }
            return true;
        };
        const bool sampled = row.sampledAge < 0.f || age < row.sampledAge ?
            row.v1->Set_SampleTimeWithTransformHistory(age, provider, m_Status) :
            row.v1->Advance_PreviewWithTransformHistory(age - row.sampledAge, provider, m_Status);
        if (!sampled) return false;
        row.sampledAge = age;
        row.v1->Set_Visible(true);
        if (row.v1->Is_RenderFailureIsolated()) { m_Status = row.v1->Get_Status(); return false; }
        return true;
    }
    if (!row.v2)
    {
        if (!row.snapshot && !Stage_Row(row, pivot)) return false;
        EFFECT_V2_GROUP_PLAYBACK_DESC playback; playback.PivotWorld = pivot; playback.bExternalClock = true; playback.fInitialAgeSeconds = age;
        playback.PivotSampler = [history](float seconds, float4x4_t& sampled, std::string& error) { return history->Sample(seconds, sampled, error); };
        row.v2 = row.key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V2_GROUP ?
            CEffectV2Runtime::Play_Group(*row.snapshot->Find_Group(row.key.strStableId), row.snapshot, playback, m_Device, m_Context) :
            CEffectV2Runtime::Play_Leaf(row.key.strStableId, row.snapshot, playback, m_Device, m_Context);
        if (!row.v2) { m_Status = CEffectV2Runtime::Last_Error(); return false; }
    }
    const bool sampled = CEffectV2Runtime::Sample_Group(row.v2, age, true, m_Device, m_Context);
    std::string error;
    if (CEffectV2Runtime::Consume_GroupFailure(row.v2, error) || !sampled)
    { m_Status = error.empty() ? CEffectV2Runtime::Last_Error() : error; return false; }
    return true;
}
bool CEffectAuthoringSequencer::Record_V1Anchors(EFFECT_ROW& row, const float4x4_t& pivot, const float age)
{
    if (!m_V1Anchors) { m_Status = "V1 source-anchor provider is unavailable."; return false; }
    std::unordered_map<std::string, float4x4_t> current;
    if (!m_V1Anchors(row.v1, pivot, m_UseKouku, current, m_Status)) return false;
    if (current.empty())
    {
        if (row.anchorHistory && !row.anchorHistory->slots.empty())
        { m_Status = "V1 source anchor slots changed during playback; refresh the Effect snapshot."; return false; }
        row.v1->Set_SourceAnchorWorlds(current); return true;
    }
    if (!row.anchorHistory) row.anchorHistory = std::make_shared<V1_ANCHOR_HISTORY>();
    auto& history = *row.anchorHistory;
    const auto record = [&](const float seconds, const auto& worlds)
    {
        if (history.recordedAge >= 0.f && (worlds.size() != history.slots.size() ||
            std::any_of(worlds.begin(), worlds.end(), [&](const auto& value) { return !history.slots.contains(value.first); })))
        { m_Status = "V1 source anchor slots changed during playback; refresh the Effect snapshot."; return false; }
        for (const auto& [slot, world] : worlds)
        {
            if (slot.empty()) { m_Status = "V1 source anchor has an empty runtime slot."; return false; }
            if (!history.slots[slot].Record(seconds, world, false, m_Status)) return false;
        }
        history.recordedAge = seconds; return true;
    };
    if (age >= history.recordedAge)
    {
        // Backfill the same 60 Hz steps the native V1 simulation requests.
        // Sampling the saved model clips and restoring the cursor avoids
        // substituting today's hand pose for earlier particle births.
        const bool needsPast = age > 0.f && history.recordedAge < 0.f;
        const bool ownsSequence = !m_UseKouku && !m_SelectedSequence.empty();
        if (needsPast && !ownsSequence)
        { m_Status = "Source-anchor seek requires recorded pose history or a selected saved model animation sequence."; return false; }
        bool movedModel = false, success = true;
        if (ownsSequence)
        {
            const auto first = history.recordedAge < 0.f ? 0u : static_cast<std::uint32_t>(std::floor(double(history.recordedAge) * 60.0)) + 1u;
            for (auto step = first; double(step) / 60.0 < double(age) - 0.000001; ++step)
            {
                const float seconds = static_cast<float>(double(step) / 60.0);
                if (seconds <= history.recordedAge) continue;
                movedModel = true;
                if (!Sample_Model(row.startMs + static_cast<std::uint32_t>(std::llround(double(seconds) * 1000.0)))) { success = false; break; }
                float4x4_t owner, historicalPivot;
                if (!Resolve_Root(owner) || !Sample_Camera(row.startMs + static_cast<std::uint32_t>(std::llround(double(seconds) * 1000.0)), owner))
                { success = false; break; }
                XMStoreFloat4x4(&historicalPivot, XMMatrixTranslation(row.offset.x, row.offset.y, row.offset.z) * XMLoadFloat4x4(&owner));
                std::unordered_map<std::string, float4x4_t> worlds;
                if (!m_V1Anchors(row.v1, historicalPivot, false, worlds, m_Status) || !record(seconds, worlds)) { success = false; break; }
            }
        }
        const auto failure = m_Status;
        if (movedModel)
        {
            float4x4_t currentRoot;
            if (!Sample_Model(ClockMs()) || !Resolve_Root(currentRoot) || !Sample_Camera(ClockMs(), currentRoot)) return false;
        }
        if (!success) { m_Status = failure; return false; }
        if (!record(age, current)) return false;
    }
    std::unordered_map<std::string, float4x4_t> sampled;
    for (const auto& [slot, values] : history.slots)
    {
        float4x4_t world;
        if (!values.Sample(age, world, m_Status)) return false;
        sampled.emplace(slot, world);
    }
    row.v1->Set_SourceAnchorWorlds(std::move(sampled)); return true;
}
bool CEffectAuthoringSequencer::Play()
{
    if (!m_Transient && m_Effects.empty() && m_CameraRows.empty() && (!m_UseKouku && m_SelectedSequence.empty()))
    { m_Status = "Select a model animation or append an Effect first."; return false; }
    if (!Validate_CameraRows(m_Transient ? m_TransientCameraRows : m_CameraRows)) return false;
    const bool wasActive = m_Active;
    if (!Begin_Model()) return false;
    float4x4_t root;
    if (!Resolve_Root(root) || !Sample_Camera(ClockMs(), root)) { if (!wasActive) Stop(); return false; }
    const auto prepareRow = [&](EFFECT_ROW& row)
    {
        row.v1.reset(); row.v2 = 0u; row.sampledAge = -1.f;
        if (!wasActive || m_ClockMs == 0.0) { row.history.reset(); row.anchorHistory.reset(); row.recordedAge = -1.f; }
        else if (row.history) row.history = std::make_shared<EFFECT_V2_PIVOT_HISTORY>(*row.history);
        if (row.anchorHistory) row.anchorHistory = std::make_shared<V1_ANCHOR_HISTORY>(*row.anchorHistory);
    };
    if (m_Transient)
    {
        // Play/Restart also own the Preview-only row shown in this sequencer.
        auto staged = *m_Transient;
        prepareRow(staged);
        if (!Stage_Row(staged, root) || !Sample_Row(staged, root))
        {
            Release_Row(staged);
            if (!wasActive) Stop();
            m_Status = "Preview preserved: " + m_Status; return false;
        }
        Release_Row(*m_Transient);
        m_Transient = std::move(staged);
    }
    else
    {
        auto staged = m_Effects;
        for (auto& row : staged) prepareRow(row);
        for (auto& row : staged)
            if (!Stage_Row(row, root) || !Sample_Row(row, root))
            {
                for (auto& rollback : staged) Release_Row(rollback);
                if (!wasActive) Stop();
                m_Status = "Preview preserved: " + m_Status; return false;
            }
        for (auto& row : m_Effects) Release_Row(row);
        m_Effects = std::move(staged);
    }
    m_Active = true; m_Paused = false;
    m_SkipNextPlaybackDelta = true; m_Interaction = true;
    if (!Sample_Camera(ClockMs(), root)) { Stop(); return false; }
    m_Status = "Playing model, Effects and camera on the same cursor."; return true;
}
bool CEffectAuthoringSequencer::Append(const EFFECT_RESOURCE_KEY& key, const std::uint32_t durationMs)
{
    if (m_Effects.size() >= MAX_EFFECTS) { m_Status = "A sequence supports at most 256 Effect occurrences."; return false; }
    while (std::any_of(m_Effects.begin(), m_Effects.end(), [&](const auto& existing)
        { return existing.id == "effect.occurrence." + std::to_string(m_NextEffectOrdinal); })) ++m_NextEffectOrdinal;
    EFFECT_ROW row; row.id = "effect.occurrence." + std::to_string(m_NextEffectOrdinal);
    row.key = key; row.startMs = ClockMs(); row.durationMs = durationMs;
    row.anchorSlotId = m_DefaultAnchorSlotId;
    float4x4_t root = m_WorldRoot;
    if (m_Active && !Resolve_Root(root)) return false;
    if (!Stage_Row(row, root) || (m_Active && !m_Transient && !Sample_Row(row, root))) { Release_Row(row); return false; }
    m_SelectedEffect = row.id; m_Effects.push_back(std::move(row)); ++m_NextEffectOrdinal;
    m_Dirty = true; m_Status = "Appended " + key.strStableId + " at " + std::to_string(ClockMs()) + " ms."; return true;
}
bool CEffectAuthoringSequencer::Preview(const EFFECT_RESOURCE_KEY& key, const std::uint32_t durationMs)
{
    std::vector<CAMERA_ROW> cameras;
    if (!Read_RecoveryCameras(key, cameras)) return false;
    for (auto& row : cameras)
    {
        if (ClockMs() > MAX_MS - row.startMs - row.cue.iDurationMs)
        { m_Status = "Recovery camera rows exceed the sequence duration at this cursor."; return false; }
        row.startMs += ClockMs();
    }
    EFFECT_ROW staged; staged.id = "effect.preview"; staged.key = key; staged.startMs = ClockMs(); staged.durationMs = durationMs;
    staged.anchorSlotId = m_DefaultAnchorSlotId;
    const bool wasActive = m_Active;
    if (!Begin_Model()) return false;
    float4x4_t root;
    if (!Resolve_Root(root)) { if (!wasActive) Stop(); return false; }
    if (!Sample_Camera(ClockMs(), root, &cameras) || !Stage_Row(staged, root) || !Sample_Row(staged, root))
    {
        const auto failure = m_Status; Release_Row(staged);
        if (!wasActive) Stop();
        else if (!Sample_Camera(ClockMs(), root)) Stop();
        m_Status = failure; return false;
    }
    for (auto& row : m_Effects) Release_Row(row);
    if (m_Transient) Release_Row(*m_Transient);
    m_Transient = std::move(staged); m_TransientCameraRows = std::move(cameras);
    m_SelectedCamera.clear(); m_Active = true; m_Paused = false;
    m_SkipNextPlaybackDelta = true; m_Interaction = true;
    if (!Sample_Camera(ClockMs(), root)) { Stop(); return false; }
    m_Status = "Previewing " + key.strStableId + ". Append adds it to the saved sequence."; return true;
}
bool CEffectAuthoringSequencer::Sample()
{
    if (!Sample_Model(ClockMs())) return false;
    float4x4_t root; if (!Resolve_Root(root)) return false;
    if (!Sample_Camera(ClockMs(), root)) return false;
    if (m_Transient) return Sample_Row(*m_Transient, root);
    for (auto& row : m_Effects) if (!Sample_Row(row, root)) return false;
    return true;
}
bool CEffectAuthoringSequencer::Seek(const std::uint32_t clockMs)
{
    m_ClockMs = (std::min)(clockMs, DurationMs()); m_Paused = true;
    if (!m_Active) { if (!Play()) return false; m_Paused = true; }
    if (!Sample()) { Stop(); return false; } return true;
}
void CEffectAuthoringSequencer::Pause(const bool paused)
{
    if (m_Paused && !paused) m_SkipNextPlaybackDelta = true;
    m_Paused = paused;
}
void CEffectAuthoringSequencer::Stop()
{
    Release_Camera(); m_TransientCameraRows.clear();
    for (auto& row : m_Effects) Release_Row(row);
    if (m_Transient) { Release_Row(*m_Transient); m_Transient.reset(); }
    m_Kouku.Stop();
    if (const auto model = m_Model.lock(); model && model == CAnimationTargetService::Resolve_Model() && m_ModelGeneration == CAnimationTargetService::Resolve_TargetGeneration())
    {
        model->Set_Animation(m_PreviousClip, m_PreviousLoop); model->Skip_Blend();
        model->Set_AnimTrackPosition(m_PreviousClip, m_PreviousPosition); model->Play_Animation(0.f); model->Set_AnimPaused(m_PreviousPaused);
    }
    m_Model.reset(); m_ModelGeneration = 0u; m_Active = false; m_Paused = false;
    m_SkipNextPlaybackDelta = false;
}
void CEffectAuthoringSequencer::Update(const float dt, const bool active)
{
    if (!active) { Stop(); return; }
    if (!m_Active) return;
    // The first frame delta includes synchronous work performed before Play
    // committed. It must not consume the newly prepared Effect's lifetime.
    const bool skipAdvance = m_SkipNextPlaybackDelta;
    m_SkipNextPlaybackDelta = false;
    if (!skipAdvance && !m_Paused && std::isfinite(dt) && dt >= 0.f)
        m_ClockMs = (std::min)(double(DurationMs()), m_ClockMs + dt * 1000.0);
    if (!Sample()) { Stop(); return; }
    if (!m_Paused && m_ClockMs >= DurationMs())
    {
        if (m_Loop)
        {
            m_ClockMs = 0; Play();
        }
        else m_Paused = true;
    }
}

bool CEffectAuthoringSequencer::Uses_Resource(const EFFECT_RESOURCE_KEY& key) const
{
    if (m_Transient) return m_Transient->key == key;
    return std::any_of(m_Effects.begin(), m_Effects.end(),
        [&](const EFFECT_ROW& row) { return row.key == key; });
}

bool CEffectAuthoringSequencer::Refresh_Effects(const EFFECT_RESOURCE_KEY* key)
{
    Preserve_ClockDuringAuthoring();
    if (key && !Uses_Resource(*key)) return true;
    float4x4_t root = m_WorldRoot;
    if (m_Active && !Resolve_Root(root)) return false;
    if (m_Transient)
    {
        auto staged = *m_Transient; staged.v1.reset(); staged.v2 = 0u; staged.sampledAge = -1.f; staged.snapshot.reset(); staged.anchorHistory.reset();
        if (staged.history) staged.history = std::make_shared<EFFECT_V2_PIVOT_HISTORY>(*staged.history);
        if (!Stage_Row(staged, root)) { Release_Row(staged); return false; }
        if (staged.v1 && m_Transient->v1) staged.v1->Preserve_StartingSceneCapture(*m_Transient->v1);
        if (m_Active && !Sample_Row(staged, root)) { Release_Row(staged); return false; }
        Release_Row(*m_Transient); m_Transient = std::move(staged);
        m_SkipNextPlaybackDelta = true; return true;
    }
    std::vector<std::pair<size_t, EFFECT_ROW>> staged;
    for (size_t index = 0u; index < m_Effects.size(); ++index)
    {
        if (key && !(m_Effects[index].key == *key)) continue;
        auto row = m_Effects[index];
        row.v1.reset(); row.v2 = 0u; row.sampledAge = -1.f; row.snapshot.reset(); row.anchorHistory.reset();
        if (row.history) row.history = std::make_shared<EFFECT_V2_PIVOT_HISTORY>(*row.history);
        staged.emplace_back(index, std::move(row));
    }
    for (auto& [index, row] : staged)
    {
        if (!Stage_Row(row, root))
        { for (auto& [unused, rollback] : staged) Release_Row(rollback); m_Status = "Preview preserved: " + m_Status; return false; }
        if (row.v1 && m_Effects[index].v1) row.v1->Preserve_StartingSceneCapture(*m_Effects[index].v1);
        if (m_Active && !Sample_Row(row, root))
        { for (auto& [unused, rollback] : staged) Release_Row(rollback); m_Status = "Preview preserved: " + m_Status; return false; }
    }
    for (auto& [index, row] : staged)
    {
        Release_Row(m_Effects[index]);
        m_Effects[index] = std::move(row);
    }
    m_SkipNextPlaybackDelta = true;
    m_Status = "Effect edits applied at the current cursor."; return true;
}

void CEffectAuthoringSequencer::Draw_KoukuInventory()
{
    if (ImGui::Button("Reload saved Patterns##EffectSequencer")) m_Kouku.Reload();
    if (!m_Kouku.Is_Loaded()) { ImGui::TextWrapped("Reload saved Patterns to list their complete animation order."); return; }
    const auto& document = m_Kouku.Get_Document();
    std::set<std::string> members;
    for (const auto& bundle : document.Bundles) for (const auto& member : bundle.Members) members.insert(member.strPatternId);
    const auto drawPattern = [&](const auto& pattern)
    {
        ImGui::PushID(pattern.strPatternId.c_str());
        const std::string label = std::string(Label(pattern.strDisplayName, pattern.strPatternId)) + "##Pattern";
        if (ImGui::Selectable(label.c_str(), m_UseKouku && !m_Kouku.Selected_IsBundle() && m_Kouku.Selected_Id() == pattern.strPatternId)) Select_Kouku(pattern.strPatternId, false);
        ImGui::PopID();
    };
    const auto drawBundle = [&](const auto& bundle)
    {
        ImGui::PushID(bundle.strBundleId.c_str());
        const bool open = ImGui::TreeNodeEx("Bundle", ImGuiTreeNodeFlags_OpenOnArrow, "%s", Label(bundle.strDisplayName, bundle.strBundleId));
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) Select_Kouku(bundle.strBundleId, true);
        if (open)
        {
            for (const auto& member : bundle.Members)
            {
                ImGui::PushID(member.strMemberId.c_str());
                const auto found = std::find_if(document.Patterns.begin(), document.Patterns.end(), [&](const auto& row) { return row.strPatternId == member.strPatternId; });
                if (found != document.Patterns.end()) drawPattern(*found);
                else ImGui::TextWrapped("Missing Pattern: %s", member.strPatternId.c_str());
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    };
    for (const char* gate : {"GATE1", "GATE2", "GATE3"})
        if (ImGui::TreeNode(gate))
        {
            std::set<std::string> shownBundles;
            for (const auto& folder : document.Folders)
            {
                if (folder.strGateId != gate) continue;
                ImGui::PushID(folder.strFolderId.c_str());
                if (ImGui::TreeNodeEx("Folder", ImGuiTreeNodeFlags_None, "%s", Label(folder.strDisplayName, folder.strFolderId)))
                {
                    for (const auto& bundle : document.Bundles)
                        if (bundle.strGateId == gate && bundle.strFolderId == folder.strFolderId && shownBundles.insert(bundle.strBundleId).second) drawBundle(bundle);
                    for (const auto& pattern : document.Patterns)
                        if (pattern.strGateId == gate && pattern.strFolderId == folder.strFolderId) drawPattern(pattern);
                    ImGui::TreePop();
                }
                else for (const auto& bundle : document.Bundles)
                    if (bundle.strGateId == gate && bundle.strFolderId == folder.strFolderId) shownBundles.insert(bundle.strBundleId);
                ImGui::PopID();
            }
            for (const auto& bundle : document.Bundles)
                if (bundle.strGateId == gate && !shownBundles.contains(bundle.strBundleId))
                { ImGui::TextWrapped("Saved folder unavailable: %s", bundle.strFolderId.c_str()); drawBundle(bundle); }
            for (const auto& pattern : document.Patterns)
            {
                if (pattern.strGateId != gate || members.contains(pattern.strPatternId)) continue;
                const bool parented = std::any_of(document.Folders.begin(), document.Folders.end(),
                    [&](const auto& folder) { return folder.strFolderId == pattern.strFolderId && folder.strGateId == gate; });
                if (parented) continue;
                if (!pattern.strFolderId.empty()) ImGui::TextWrapped("Saved folder unavailable: %s", pattern.strFolderId.c_str());
                drawPattern(pattern);
            }
            ImGui::TreePop();
        }
    ImGui::TextWrapped("%s", m_Kouku.Status().c_str());
    if (ImGui::BeginCombo("Effect anchor member", m_AnchorMember.empty() ? "Select member" : m_AnchorMember.c_str()))
    {
        for (const auto& actor : m_Kouku.Actors())
        {
            ImGui::PushID(actor.memberId.c_str());
            const std::string label = std::string(Label(actor.displayName, actor.patternId)) + "##AnchorMember";
            if (ImGui::Selectable(label.c_str(), actor.memberId == m_AnchorMember)) { Stop(); m_AnchorMember = actor.memberId; m_Dirty = true; }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
}

void CEffectAuthoringSequencer::Render_ModelView()
{
    ImGui::PushID("EffectAuthoringModel");
    ImGui::SeparatorText("Animation sequence");
    if (ImGui::SmallButton("No model sequence")) { Stop(); m_SelectedSequence.clear(); m_UseKouku = false; m_Dirty = true; }
    int source = m_UseKouku ? 1 : 0;
    if (ImGui::Combo("Pattern source", &source, "Selected character / Valtan\0KoukuSaydon Patterns\0"))
    { Stop(); m_UseKouku = source == 1; m_ClockMs = 0; m_Dirty = true; }
    if (m_UseKouku)
    {
        if (ImGui::BeginChild("KoukuPatternList", {0.f, 220.f}, ImGuiChildFlags_Borders)) Draw_KoukuInventory();
        ImGui::EndChild();
    }
    else
    {
        ImGui::Text("Model: %s", CAnimationTargetService::Resolve_AssetName().c_str());
        if (ImGui::Button("Reload saved ordered clips")) Reload_ModelSequences();
        if (m_InventoryGeneration != CAnimationTargetService::Resolve_TargetGeneration())
            ImGui::TextWrapped("Select a preview model, then reload its saved skill or Product pattern bindings.");
        if (ImGui::BeginChild("OrderedAnimationList", {0.f, 160.f}, ImGuiChildFlags_Borders))
            for (const auto& sequence : m_Sequences)
            {
                ImGui::PushID(sequence.id.c_str());
                const std::string label = std::string(Label(sequence.label, sequence.id)) + "##Sequence";
                if (ImGui::Selectable(label.c_str(), sequence.id == m_SelectedSequence)) Select_ModelSequence(sequence.id);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s\n%zu clips / %u ms\n%s", sequence.id.c_str(), sequence.clips.size(), sequence.durationMs, sequence.error.c_str());
                ImGui::PopID();
            }
        ImGui::EndChild();
    }
    if (!m_UseKouku)
    {
        int anchor = m_ModelRoot ? 0 : 1;
        if (ImGui::Combo("Effect anchor", &anchor, "Model root\0World\0")) { Stop(); m_ModelRoot = anchor == 0; m_Dirty = true; }
        if (!m_ModelRoot || !m_Panel || !m_Panel->Is_PreviewActive())
        {
            if (ImGui::DragFloat3("Effect world position", &m_WorldRoot._41, .05f)) m_Dirty = true;
            if (ImGui::Button("In front of camera"))
                if (const auto* world = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))
                { const auto matrix = XMLoadFloat4x4(world); XMStoreFloat4x4(&m_WorldRoot, XMMatrixTranslationFromVector(matrix.r[3] + XMVector3Normalize(matrix.r[2]) * 5.f)); m_Dirty = true; }
        }
    }
    if (Render_AnchorChoice("New occurrence bone/socket", m_DefaultAnchorSlotId)) m_Dirty = true;
    ImGui::TextWrapped("The selected bone is copied into each new Preview/Append occurrence. Source recipe sockets keep their original attachments.");
    ImGui::TextWrapped("%s", m_Status.c_str());
    ImGui::TextDisabled("Animation order is read-only. Preview keeps gameplay and source assets unchanged.");
    ImGui::PopID();
}

void CEffectAuthoringSequencer::Render_Sequencer()
{
    ImGui::SetNextWindowSize({1080.f, 430.f}, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Sequencer##EffectAuthoring")) { ImGui::End(); return; }
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
        (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))) m_Interaction = true;
    if (ImGui::Button("Play")) { if (m_ClockMs >= DurationMs()) m_ClockMs = 0; Play(); }
    ImGui::SameLine(); if (ImGui::Button(m_Paused ? "Resume" : "Pause")) Pause(!m_Paused);
    ImGui::SameLine(); if (ImGui::Button("Restart")) { m_ClockMs = 0; Play(); }
    ImGui::SameLine(); if (ImGui::Button("Stop")) { Stop(); m_ClockMs = 0; }
    ImGui::SameLine(); ImGui::Checkbox("Loop", &m_Loop);
    ImGui::SameLine(); if (ImGui::Button("Refresh Effects")) Refresh_Effects();
    ImGui::SetNextItemWidth(220.f); ImGui::InputText("Sequence ID", m_SequenceId, sizeof(m_SequenceId));
    ImGui::SameLine(); if (ImGui::Button("Save sequence")) Save_Sequence();
    ImGui::SameLine(); if (ImGui::Button("Load sequence")) Load_Sequence();
    ImGui::SameLine(); if (ImGui::Button("Revert sequence")) Load_Sequence(true);
    ImGui::SameLine(); if (ImGui::Button("New sequence"))
    {
        if (m_Dirty) m_Status = "Save the current sequence before creating another.";
        else { Stop(); m_Effects.clear(); m_CameraRows.clear(); m_SelectedCamera.clear(); m_SelectedEffect.clear(); m_ClockMs = 0; m_NextEffectOrdinal = 1u;
            std::snprintf(m_SequenceId, sizeof(m_SequenceId), "%s", CEffectEditingSession::New_Id("effect.sequence.").c_str()); m_SequenceBaseline.clear(); m_PersistedSequenceId.clear(); m_SequenceExisted = false; m_Dirty = true; }
    }
    if (m_Dirty) { ImGui::SameLine(); ImGui::TextDisabled("Unsaved sequence"); }
    int clock = static_cast<int>(ClockMs());
    ImGui::SetNextItemWidth(420.f); if (ImGui::SliderInt("Time", &clock, 0, static_cast<int>(DurationMs()), "%d ms")) Seek(clock);
    ImGui::SameLine(); ImGui::SetNextItemWidth(150.f); ImGui::SliderFloat("Zoom", &m_Zoom, 10.f, 300.f, "%.0f px/s");
    ImGui::TextWrapped("%s", m_Status.c_str());
    ImGui::TextDisabled("Save sequence stores this preview arrangement. Select the admitted Effect in Animation Tool to change a skill cue.");
    const auto changedRow = [&](EFFECT_ROW& row)
    {
        m_Dirty = true; row.history.reset(); row.anchorHistory.reset(); row.recordedAge = row.sampledAge = -1.f;
        if (row.v2) { CEffectV2Runtime::Stop_Group(row.v2); row.v2 = 0u; }
        if (m_Active && !m_Transient && !Sample()) Stop();
    };
    const auto selected = std::find_if(m_Effects.begin(), m_Effects.end(), [&](const auto& row) { return row.id == m_SelectedEffect; });
    if (selected != m_Effects.end())
    {
        int start = static_cast<int>(selected->startMs), duration = static_cast<int>(selected->durationMs);
        ImGui::PushID("OccurrenceDetail");
        ImGui::SetNextItemWidth(135.f); bool changed = ImGui::DragInt("Start", &start, 10.f, 0, MAX_MS - static_cast<int>(selected->durationMs), "%d ms");
        ImGui::SameLine(); ImGui::SetNextItemWidth(135.f); changed |= ImGui::DragInt("Duration", &duration, 10.f, 1, MAX_MS - start, "%d ms");
        ImGui::SameLine(); ImGui::SetNextItemWidth(200.f); changed |= ImGui::DragFloat3("Offset", &selected->offset.x, .05f);
        changed |= Render_AnchorChoice("Occurrence bone/socket", selected->anchorSlotId);
        if (changed) { selected->startMs = (std::clamp)(start, 0, int(MAX_MS - 1u)); selected->durationMs = (std::clamp)(duration, 1, int(MAX_MS - selected->startMs)); changedRow(*selected); }
        ImGui::SameLine(); if (ImGui::Button("Remove")) { Release_Row(*selected); m_Effects.erase(selected); m_SelectedEffect.clear(); m_Dirty = true; }
        ImGui::PopID();
    }
    Render_CameraEditor();
    constexpr float labels = 310.f, rowHeight = 27.f;
    const float width = labels + DurationMs() * m_Zoom * .001f + 25.f;
    ImGui::SetNextWindowContentSize({width, 0.f});
    if (ImGui::BeginChild("Timeline", {0.f, 0.f}, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const auto origin = ImGui::GetCursorScreenPos(); auto* draw = ImGui::GetWindowDrawList();
        CompositionTimeline::DrawRuler(draw, {origin.x + labels, origin.y}, {origin.x + width, origin.y + rowHeight}, DurationMs(), m_Zoom);
        ImGui::Dummy({width, rowHeight});
        const auto drawClip = [&](const std::string& id, const std::string& label, double start, std::uint32_t duration)
        {
            const auto top = ImGui::GetCursorScreenPos(); ImGui::PushID(id.c_str());
            ImGui::TextUnformatted(label.c_str());
            CompositionTimeline::DrawBox(draw, {top.x + labels + float(start) * m_Zoom * .001f, top.y},
                {top.x + labels + float(start + duration) * m_Zoom * .001f, top.y + rowHeight - 3.f}, IM_COL32(65, 120, 175, 230), false, label.c_str(), false, false);
            ImGui::SetCursorScreenPos({top.x, top.y + rowHeight}); ImGui::Dummy({width, 1.f}); ImGui::PopID();
        };
        if (m_UseKouku)
            for (const auto& clip : m_Kouku.Rows()) drawClip(clip.memberId + "." + clip.occurrenceId, clip.memberId + " / " + clip.runtimeClip, clip.startMs, clip.durationMs);
        else if (const auto* sequence = Selected_Sequence())
            for (const auto& clip : sequence->clips) drawClip(clip.id, clip.label, clip.startMs, clip.durationMs);
        if (m_Transient) drawClip("TransientPreview", "Preview only / " + m_Transient->key.strStableId, m_Transient->startMs, m_Transient->durationMs);
        if (m_Effects.empty() && !m_Transient && (!m_UseKouku || m_Kouku.Rows().empty()) && !Selected_Sequence())
            ImGui::TextWrapped("Choose a model sequence in Model View, or Preview / Append a selected Effect from its owner panel.");
        const auto resources = CEffectResourceCatalog::Get().Get_Snapshot();
        for (auto& row : m_Effects)
        {
            const auto top = ImGui::GetCursorScreenPos(); ImGui::PushID(row.id.c_str());
            bool mute = row.muted;
            if (ImGui::Checkbox("M", &mute)) { row.muted = mute; changedRow(row); }
            ImGui::SameLine();
            const auto* resource = resources ? resources->Find(row.key) : nullptr;
            const std::string name = resource ? resource->strDisplayLabel : row.key.strStableId;
            const std::string selectable = name + "##EffectRow";
            if (ImGui::Selectable(selectable.c_str(), row.id == m_SelectedEffect, 0, {labels - 45.f, rowHeight - 3.f})) m_SelectedEffect = row.id;
            const float start = top.x + labels + row.startMs * m_Zoom * .001f;
            const float end = start + row.durationMs * m_Zoom * .001f;
            CompositionTimeline::DrawBox(draw, {start, top.y}, {(std::max)(start + 4.f, end), top.y + rowHeight - 3.f}, IM_COL32(168, 104, 48, 225), row.id == m_SelectedEffect, name.c_str());
            ImGui::SetCursorScreenPos({start, top.y});
            ImGui::InvisibleButton("EffectTiming", {(std::max)(4.f, end - start), rowHeight - 3.f});
            if (ImGui::IsItemActivated())
            {
                m_SelectedEffect = m_DragEffect = row.id; m_DragMouseX = ImGui::GetIO().MousePos.x;
                m_DragStartMs = row.startMs; m_DragDurationMs = row.durationMs; m_Paused = true;
                m_DragKind = static_cast<int>(CompositionTimeline::HitBoxGesture(m_DragMouseX, start, end, 6.f, true, true));
            }
            if (m_DragEffect == row.id && ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                const auto delta = static_cast<std::int64_t>(std::llround((ImGui::GetIO().MousePos.x - m_DragMouseX) * 1000. / m_Zoom));
                std::int64_t newStart = m_DragStartMs, newDuration = m_DragDurationMs;
                if (m_DragKind == static_cast<int>(CompositionTimeline::BoxGesture::MOVE))
                    newStart = (std::clamp)(newStart + delta, std::int64_t(0), std::int64_t(MAX_MS - m_DragDurationMs));
                else if (m_DragKind == static_cast<int>(CompositionTimeline::BoxGesture::TRIM_START))
                {
                    newStart = (std::clamp)(newStart + delta, std::int64_t(0), std::int64_t(m_DragStartMs) + m_DragDurationMs - 1);
                    newDuration = std::int64_t(m_DragStartMs) + m_DragDurationMs - newStart;
                }
                else newDuration = (std::clamp)(newDuration + delta, std::int64_t(1), std::int64_t(MAX_MS - m_DragStartMs));
                if (row.startMs != newStart || row.durationMs != newDuration)
                { row.startMs = static_cast<std::uint32_t>(newStart); row.durationMs = static_cast<std::uint32_t>(newDuration); changedRow(row); }
            }
            ImGui::SetCursorScreenPos({top.x, top.y + rowHeight}); ImGui::Dummy({width, 1.f}); ImGui::PopID();
        }
        Draw_CameraRows(labels, rowHeight, width);
        const float cursorX = origin.x + labels + float(m_ClockMs) * m_Zoom * .001f;
        draw->AddLine({cursorX, origin.y}, {cursorX, ImGui::GetCursorScreenPos().y}, IM_COL32(255, 222, 90, 255), 2.f);
    }
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) m_DragEffect.clear();
    ImGui::EndChild(); ImGui::End();
}

bool CEffectAuthoringSequencer::Save_Sequence()
{
    if (m_Transient) { m_Status = "Append the preview and camera rows before saving the sequence."; return false; }
    if (!Validate_CameraRows(m_CameraRows)) return false;
    const std::string id = m_SequenceId;
    if (!CEffectV2Document::Is_ValidEffectId(id) || id.size() >= sizeof(m_SequenceId))
    { m_Status = "Use a stable sequence ID containing letters, digits, dots, underscores or hyphens."; return false; }
    const auto path = CProjectDataRoot::Resolve(std::filesystem::path("Effects") / "Sequences" / (id + ".effectsequence.json"));
    std::string current; bool exists = false;
    if (!Read_SequenceText(path, current, exists, m_Status)) return false;
    if ((id == m_PersistedSequenceId && (exists != m_SequenceExisted || (exists && current != m_SequenceBaseline))) ||
        (id != m_PersistedSequenceId && exists))
    { m_Status = "Sequence file changed or already exists. Load it before replacing it; the current timeline is preserved."; return false; }
    if (!std::isfinite(m_WorldRoot._41) || !std::isfinite(m_WorldRoot._42) || !std::isfinite(m_WorldRoot._43) ||
        std::abs(m_WorldRoot._41) > 100000.f || std::abs(m_WorldRoot._42) > 100000.f || std::abs(m_WorldRoot._43) > 100000.f ||
        (m_UseKouku && m_Kouku.Selected_Id().empty()))
    { m_Status = "Select a saved model Pattern/Bundle and use finite world coordinates within 100000 m."; return false; }
    if (!Validate_Anchor(m_DefaultAnchorSlotId, m_ModelRoot, m_UseKouku)) return false;
    std::ostringstream out; out.imbue(std::locale::classic()); out.precision(9);
    out << "{\n  \"schema\": \"lostark.effect-authoring-sequence\",\n  \"formatVersion\": 3,\n  \"sequenceId\": \"" << CDataJson::Escape(id)
        << "\",\n  \"model\": {\"kind\": \"" << (m_UseKouku ? (m_Kouku.Selected_IsBundle() ? "KOUKU_BUNDLE" : "KOUKU_PATTERN") : "MODEL_SEQUENCE")
        << "\", \"assetName\": \"" << CDataJson::Escape(m_AssetName) << "\", \"sequenceId\": \""
        << CDataJson::Escape(m_UseKouku ? m_Kouku.Selected_Id() : m_SelectedSequence) << "\", \"anchorMemberId\": \"" << CDataJson::Escape(m_AnchorMember)
        << "\"},\n  \"anchorMode\": \"" << (m_ModelRoot ? "MODEL_ROOT" : "WORLD")
        << "\",\n  \"worldPosition\": [" << m_WorldRoot._41 << ", " << m_WorldRoot._42 << ", " << m_WorldRoot._43 << "],\n  \"effects\": [";
    std::set<std::string> ids;
    for (std::size_t i = 0; i < m_Effects.size(); ++i)
    {
        const auto& row = m_Effects[i];
        if (!Validate_Anchor(row.anchorSlotId, m_ModelRoot, m_UseKouku)) return false;
        if (!row.key.Is_Valid() || !ids.insert(row.id).second || !CEffectV2Document::Is_ValidEffectId(row.id) ||
            !row.durationMs || row.durationMs > MAX_MS || row.startMs > MAX_MS - row.durationMs ||
            !std::isfinite(row.offset.x) || !std::isfinite(row.offset.y) || !std::isfinite(row.offset.z) ||
            std::abs(row.offset.x) > 100000.f || std::abs(row.offset.y) > 100000.f || std::abs(row.offset.z) > 100000.f)
        { m_Status = "Sequence contains an invalid Effect occurrence."; return false; }
        out << (i ? ",\n" : "\n") << "    {\"occurrenceId\": \"" << CDataJson::Escape(row.id) << "\", \"owner\": \"" << Owner_Key(row.key.eOwnerKind)
            << "\", \"effectId\": \"" << CDataJson::Escape(row.key.strStableId) << "\", \"anchorSlotId\": \"" << CDataJson::Escape(row.anchorSlotId) << "\", \"startMs\": " << row.startMs << ", \"durationMs\": " << row.durationMs
            << ", \"offset\": [" << row.offset.x << ", " << row.offset.y << ", " << row.offset.z << "], \"muted\": " << (row.muted ? "true" : "false") << "}";
    }
    out << "\n  ]"; Write_CameraRows(out); out << "\n}\n";
    const auto text = out.str(); DATA_JSON_VALUE parsed;
    if (text.size() > 1024u * 1024u) { m_Status = "Sequence exceeds the 1 MiB authoring limit; the saved file is preserved."; return false; }
    if (!CDataJson::Parse(text, parsed, m_Status)) return false;
    if (!CEffectV2Document::Write_AtomicFile(path, text, m_Status)) return false;
    m_SequenceBaseline = text; m_SequenceExisted = true; m_PersistedSequenceId = id; m_Dirty = false;
    m_Status = "Saved sequence timing and stable references. Effect assets were not changed."; return true;
}
bool CEffectAuthoringSequencer::Load_Sequence(const bool discard)
{
    if (m_Dirty && !discard) { m_Status = "Save the current timeline or use Revert sequence to discard its edits."; return false; }
    const std::string id = m_SequenceId;
    if (!CEffectV2Document::Is_ValidEffectId(id)) { m_Status = "Invalid sequence ID."; return false; }
    const auto path = CProjectDataRoot::Resolve(std::filesystem::path("Effects") / "Sequences" / (id + ".effectsequence.json"));
    std::string text; bool exists = false;
    if (!Read_SequenceText(path, text, exists, m_Status)) return false;
    if (!exists) { m_Status = "Saved sequence does not exist. The current timeline is preserved."; return false; }
    DATA_JSON_VALUE root; std::string schema, storedId, kind, asset, sequenceId, anchor;
    std::uint32_t version = 0u;
    if (!CDataJson::Parse(text, root, m_Status) || !Json_Text(root, "schema", schema) || schema != "lostark.effect-authoring-sequence" ||
        !Json_U32(root, "formatVersion", version, 3u) || (version != 1u && version != 2u && version != 3u) || !Json_Text(root, "sequenceId", storedId) || storedId != id)
    { m_Status = "Invalid Effect sequence header; current timeline preserved."; return false; }
    const auto* model = root.Find("model"); const auto* effects = root.Find("effects"); float3_t world;
    if (!model || !model->Is_Object() || !Json_Text(*model, "kind", kind) || !Json_Text(*model, "assetName", asset) ||
        !Json_Text(*model, "sequenceId", sequenceId) || !Json_Text(*model, "anchorMemberId", anchor) ||
        (kind != "MODEL_SEQUENCE" && kind != "KOUKU_PATTERN" && kind != "KOUKU_BUNDLE") || !Json_Vector(root, "worldPosition", world) ||
        !effects || !effects->Is_Array() || effects->Get_Array().size() > MAX_EFFECTS)
    { m_Status = "Invalid Effect sequence model or occurrence list; current timeline preserved."; return false; }
    bool modelRoot = true;
    if (const auto* mode = root.Find("anchorMode"))
    {
        if (!mode->Is_String() || (mode->Get_String() != "MODEL_ROOT" && mode->Get_String() != "WORLD"))
        { m_Status = "Invalid saved Effect anchor mode; current timeline preserved."; return false; }
        modelRoot = mode->Get_String() == "MODEL_ROOT";
    }
    std::vector<CAMERA_ROW> cameras;
    if (!Parse_CameraRows(root, version >= 3u, cameras)) return false;
    std::vector<EFFECT_ROW> staged; std::set<std::string> ids;
    for (const auto& value : effects->Get_Array())
    {
        EFFECT_ROW row; std::string owner;
        const auto* muted = value.Find("muted");
        if (!Json_Text(value, "occurrenceId", row.id) || !CEffectV2Document::Is_ValidEffectId(row.id) || !ids.insert(row.id).second ||
            !Json_Text(value, "owner", owner) || !Json_Text(value, "effectId", row.key.strStableId) || !Json_U32(value, "startMs", row.startMs) ||
            !Json_U32(value, "durationMs", row.durationMs) || !row.durationMs || row.startMs > MAX_MS - row.durationMs ||
            !Json_Vector(value, "offset", row.offset) || !muted || !muted->Is_Boolean())
        { m_Status = "Invalid saved Effect occurrence; current timeline preserved."; return false; }
        if (version >= 2u && !Json_Text(value, "anchorSlotId", row.anchorSlotId))
        { m_Status = "Saved v2 Effect occurrence is missing its anchor; current timeline preserved."; return false; }
        if (!Validate_Anchor(row.anchorSlotId, modelRoot, kind != "MODEL_SEQUENCE")) return false;
        if (row.anchorSlotId != "root" && asset != CAnimationTargetService::Resolve_AssetName())
        { m_Status = "Load the saved character before resolving its occurrence bones; current timeline preserved."; return false; }
        if (owner == "V1_DOCUMENT") row.key.eOwnerKind = EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT;
        else if (owner == "V2_LEAF") row.key.eOwnerKind = EFFECT_RESOURCE_OWNER_KIND::V2_LEAF;
        else if (owner == "V2_GROUP") row.key.eOwnerKind = EFFECT_RESOURCE_OWNER_KIND::V2_GROUP;
        else { m_Status = "Unknown saved Effect owner; current timeline preserved."; return false; }
        if (!row.key.Is_Valid()) { m_Status = "Invalid typed Effect ID; current timeline preserved."; return false; }
        row.muted = muted->Get_Boolean(); staged.push_back(std::move(row));
    }
    if (kind == "MODEL_SEQUENCE" && !sequenceId.empty())
    {
        if (asset != CAnimationTargetService::Resolve_AssetName() || !m_Panel || !m_Panel->Is_PreviewActive())
        { m_Status = "Select " + asset + " in Model View, reload its ordered clips, then load this sequence. Current timeline preserved."; return false; }
        if (m_InventoryGeneration != CAnimationTargetService::Resolve_TargetGeneration())
        { m_Status = "Reload saved ordered clips before loading this model sequence."; return false; }
        const auto found = std::find_if(m_Sequences.begin(), m_Sequences.end(), [&](const auto& row) { return row.id == sequenceId; });
        if (found == m_Sequences.end() || !found->error.empty()) { m_Status = "Saved model sequence reference is unavailable."; return false; }
    }
    if (kind != "MODEL_SEQUENCE")
    {
        if (!m_Kouku.Is_Loaded() && !m_Kouku.Reload()) { m_Status = m_Kouku.Status(); return false; }
        const auto& document = m_Kouku.Get_Document();
        const bool found = kind == "KOUKU_BUNDLE" ? std::any_of(document.Bundles.begin(), document.Bundles.end(), [&](const auto& row) { return row.strBundleId == sequenceId; }) :
            std::any_of(document.Patterns.begin(), document.Patterns.end(), [&](const auto& row) { return row.strPatternId == sequenceId; });
        if (!found) { m_Status = "Saved Kouku Pattern/Bundle reference is unavailable."; return false; }
        bool validAnchor = kind == "KOUKU_PATTERN" && anchor == sequenceId;
        if (kind == "KOUKU_BUNDLE")
            for (const auto& bundle : document.Bundles)
                if (bundle.strBundleId == sequenceId)
                    validAnchor = std::any_of(bundle.Members.begin(), bundle.Members.end(), [&](const auto& member)
                        { return member.strMemberId == anchor; });
        if (!validAnchor) { m_Status = "Saved Effect anchor is not a member of its Kouku Pattern/Bundle."; return false; }
    }
    Stop(); m_Effects = std::move(staged); m_CameraRows = std::move(cameras); m_SelectedCamera.clear(); m_ClockMs = 0; m_SelectedEffect.clear(); m_UseKouku = kind != "MODEL_SEQUENCE";
    if (m_UseKouku) { if (kind == "KOUKU_BUNDLE") m_Kouku.Select_Bundle(sequenceId); else m_Kouku.Select_Pattern(sequenceId); }
    else m_SelectedSequence = sequenceId;
    m_AssetName = asset; m_AnchorMember = anchor; m_ModelRoot = modelRoot;
    m_DefaultAnchorSlotId = "root";
    XMStoreFloat4x4(&m_WorldRoot, XMMatrixTranslation(world.x, world.y, world.z));
    m_NextEffectOrdinal = 1u;
    while (ids.contains("effect.occurrence." + std::to_string(m_NextEffectOrdinal))) ++m_NextEffectOrdinal;
    m_SequenceBaseline = text; m_SequenceExisted = true; m_PersistedSequenceId = id; m_Dirty = false;
    m_Status = "Loaded sequence references and timing. Press Play to prepare their existing Effect runtimes."; return true;
}
}
