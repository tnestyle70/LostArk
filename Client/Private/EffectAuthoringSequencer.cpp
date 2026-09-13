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
#include "Effect_DocumentCodec.h"
#include "GameInstance.h"
#include "Model.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "PlayerSkillCatalog.h"
#include "ProjectDataRoot.h"
#include "Transform.h"
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
// V1 and V2 retain independent documents, but only one may sample the shared model.
CEffectAuthoringSequencer* g_ModelClockOwner = nullptr;
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
    ComPtr<ID3D11DeviceContext> context, std::shared_ptr<CCharacterPreviewPanel> panel, const char* sequenceId)
    : m_Device(std::move(device)), m_Context(std::move(context)), m_Panel(std::move(panel))
{
    std::snprintf(m_SequenceId, sizeof(m_SequenceId), "%s", sequenceId);
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
    if (Is_ElementPreview()) return (std::clamp)(m_Transient->durationMs, 1u, MAX_MS);
    std::uint32_t result = !m_KoukuEffectPreview && m_UseKouku ? m_Kouku.DurationMs() : 0u;
    if (const auto* sequence = Selected_Sequence(); !m_KoukuEffectPreview && !m_UseKouku && !m_CustomAnimation && sequence) result = sequence->durationMs;
    if (!m_KoukuEffectPreview && m_CustomAnimation) for (const auto& row : m_AnimationRows) result = (std::max)(result, row.startMs + row.durationMs);
    for (const auto& row : m_TransientAnimationRows) result = (std::max)(result, row.startMs + row.durationMs);
    if (!m_Transient)
    {
        for (const auto& row : m_Sounds) result = (std::max)(result, row.startMs + row.durationMs);
        for (const auto& row : m_Colliders) result = (std::max)(result, row.startMs + row.durationMs);
    }
    if (m_Transient) result = (std::max)(result, m_Transient->startMs + m_Transient->durationMs);
    else for (const auto& row : m_Effects) result = (std::max)(result, row.startMs + row.durationMs);
    const auto& cameras = m_Transient ? m_TransientCameraRows : m_CameraRows;
    for (const auto& row : cameras) result = (std::max)(result, row.startMs + row.cue.iDurationMs);
    return (std::clamp)(result, 1u, MAX_MS);
}
bool CEffectAuthoringSequencer::Validate_AnimationRows(const std::vector<CLIP>& rows)
{
    if (rows.size() > MAX_EFFECTS) { m_Status = "A sequence supports at most 256 Animation occurrences."; return false; }
    const auto model = CAnimationTargetService::Resolve_Model();
    std::set<std::string> ids;
    std::vector<const CLIP*> enabled;
    for (const auto& row : rows)
    {
        if (!CEffectV2Document::Is_ValidEffectId(row.id) || !ids.insert(row.id).second ||
            row.label.size() > 1024u || row.memberId.size() > 256u || row.clipName.empty() || row.clipName.size() > 256u ||
            !row.durationMs || row.durationMs > MAX_MS || row.startMs > MAX_MS - row.durationMs ||
            row.sourceStartMs > MAX_MS || row.sourcePlayMs > MAX_MS ||
            !std::isfinite(row.playRate) || row.playRate <= 0.f || row.playRate > 1000.f)
        { m_Status = "Animation occurrence has invalid identity or timing: " + row.id; return false; }
        std::uint32_t index = 0u; float native = 0.f, tickRate = 0.f;
        if (!Clip_Metadata(model, row.clipName, index, native, tickRate))
        { m_Status = "Animation source is missing or ambiguous on the selected model: " + row.clipName; return false; }
        const ACTION_PRESENTATION_CLIP_TIMING timing{native / tickRate, row.sourcePlayMs, row.playRate, row.loop, row.sourceStartMs * .001f};
        float sourceDuration = 0.f, wallDuration = 0.f;
        if (!CActionPresentationTimeline::Resolve_ClipDuration(timing, sourceDuration, wallDuration))
        { m_Status = "Animation source window is invalid: " + row.id; return false; }
        if (!row.muted) enabled.push_back(&row);
    }
    std::sort(enabled.begin(), enabled.end(), [](const auto* left, const auto* right) { return left->startMs < right->startMs; });
    for (std::size_t i = 1u; i < enabled.size(); ++i)
        if (enabled[i]->startMs < enabled[i - 1u]->startMs + enabled[i - 1u]->durationMs)
        { m_Status = "Animation occurrences overlap on the same model: " + enabled[i - 1u]->id + " / " + enabled[i]->id; return false; }
    return true;
}
bool CEffectAuthoringSequencer::Append_Animation(const std::string& clipName)
{
    if (m_UseKouku || !m_Panel || !m_Panel->Is_PreviewActive())
    { m_Status = "Select one preview model before appending an Animation clip."; return false; }
    const auto model = CAnimationTargetService::Resolve_Model();
    std::uint32_t index = 0u; float native = 0.f, tickRate = 0.f;
    if (!Clip_Metadata(model, clipName, index, native, tickRate))
    { m_Status = "Animation source is missing or ambiguous: " + clipName; return false; }
    const double duration = std::ceil(static_cast<double>(native) / tickRate * 1000.0);
    if (!std::isfinite(duration) || duration < 1.0 || duration > MAX_MS || ClockMs() > MAX_MS - duration)
    { m_Status = "Animation source exceeds the sequence time limit."; return false; }
    std::vector<CLIP> staged = m_AnimationRows;
    if (!m_CustomAnimation)
        if (const auto* sequence = Selected_Sequence())
        {
            if (m_AssetName != CAnimationTargetService::Resolve_AssetName() || m_InventoryGeneration != CAnimationTargetService::Resolve_TargetGeneration())
            { m_Status = "Reload the saved model sequence before editing its Animation rows."; return false; }
            staged = sequence->clips;
        }
    CLIP row; row.id = CEffectEditingSession::New_Id("animation.occurrence."); row.label = row.clipName = clipName;
    row.startMs = ClockMs(); row.durationMs = static_cast<std::uint32_t>(duration); staged.push_back(row);
    if (!Validate_AnimationRows(staged)) return false;
    const auto previousRows = m_AnimationRows;
    const auto previousSequence = m_SelectedSequence, previousAsset = m_AssetName;
    const auto previousGeneration = m_InventoryGeneration;
    const bool previousCustom = m_CustomAnimation, previousDirty = m_Dirty;
    m_AnimationRows = std::move(staged); m_CustomAnimation = true; m_SelectedSequence.clear();
    m_AssetName = CAnimationTargetService::Resolve_AssetName(); m_InventoryGeneration = CAnimationTargetService::Resolve_TargetGeneration();
    if (!Refresh_AnimationTiming() || !Commit_TransientPreview())
    {
        const auto error = m_Status;
        m_AnimationRows = previousRows; m_CustomAnimation = previousCustom; m_SelectedSequence = previousSequence;
        m_AssetName = previousAsset; m_InventoryGeneration = previousGeneration;
        Refresh_AnimationTiming(); m_Dirty = previousDirty; m_Status = error; return false;
    }
    Select_TimelineRow(TRACK_KIND::ANIMATION, row.id);
    m_Status = "Appended Animation at the cursor. Save sequence keeps its timing."; return true;
}
bool CEffectAuthoringSequencer::Reload_ModelSequences()
{
    const auto model = CAnimationTargetService::Resolve_Model();
    if (!model || !m_Panel || !m_Panel->Is_PreviewActive())
    { m_Status = "Select a preview character or boss in Model View first."; return false; }
    const auto asset = CAnimationTargetService::Resolve_AssetName();
    const auto generation = CAnimationTargetService::Resolve_TargetGeneration();
    if (m_CustomAnimation && !m_AnimationRows.empty() && asset != m_AssetName)
    { m_Status = "The custom Animation draft belongs to " + m_AssetName + ". Restore that model or clear its Animation rows first."; return false; }
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
    Stop(); m_BoxDetailDraft.reset(); m_Sequences = std::move(staged); m_AssetName = asset; m_InventoryGeneration = generation;
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

bool CEffectAuthoringSequencer::Select_KoukuEffect(const std::string& assetId, const bool requiresSourceModel,
    const bool reusePlayerAnchor, const EFFECT_DOCUMENT_DESC* sourceDocument)
{
    return Select_SceneEffectTarget(assetId, requiresSourceModel, reusePlayerAnchor, std::nullopt, std::nullopt, sourceDocument);
}

bool CEffectAuthoringSequencer::Select_WorldEffect(const std::string& assetId, const bool reusePlayerAnchor)
{
    if (!assetId.starts_with("effect.world."))
    { m_Status = "Select a saved World Effect before preparing its scene anchor."; return false; }
    const std::optional<std::uint32_t> duration = assetId == "effect.world.mouse_click" ? std::optional<std::uint32_t>(1200u) :
        assetId == "effect.world.move_destination" ? std::optional<std::uint32_t>(7000u) : std::nullopt;
    return Select_SceneEffectTarget(assetId, false, reusePlayerAnchor, assetId == "effect.world.move_destination", duration);
}

bool CEffectAuthoringSequencer::Select_SceneEffectTarget(const std::string& assetId, const bool requiresSourceModel,
    const bool reusePlayerAnchor, const std::optional<bool> loopPolicy,
    const std::optional<std::uint32_t> previewDurationMs, const EFFECT_DOCUMENT_DESC* sourceDocument)
{
    m_PendingKoukuEffectPreview.reset();
    const bool reuseTarget = reusePlayerAnchor && m_KoukuEffectPreview && m_KoukuEffectPreview->assetId == assetId;
    const bool needsModel = requiresSourceModel || (sourceDocument && sourceDocument->SourceModelPreview);
    if (reuseTarget && m_KoukuEffectPreview->model.has_value() == needsModel &&
        m_KoukuEffectPreview->loopPolicy == loopPolicy &&
        m_KoukuEffectPreview->previewDurationMs == previewDurationMs) return true;
    const auto character = CAnimationTargetService::Resolve_SceneCharacter();
    const auto transform = character ? character->Get_Transform() : nullptr;
    if (!transform)
    { m_Status = "Enter an arena with a scene player before Play All. The player is the Effect anchor."; return false; }
    const auto& world = *transform->Get_WorldMatrixPtr();
    for (const auto& row : world.m)
        for (const auto component : row)
            if (!std::isfinite(component))
            { m_Status = "The scene player's Effect anchor is not finite."; return false; }
    const float forwardLength = world._31 * world._31 + world._33 * world._33;
    if (!std::isfinite(forwardLength) || forwardLength < 1e-12f)
    { m_Status = "The scene player's Effect anchor has no usable horizontal facing."; return false; }
    KOUKU_EFFECT_PREVIEW_TARGET target;
    target.assetId = assetId;
    target.loopPolicy = loopPolicy;
    target.previewDurationMs = previewDurationMs;
    XMStoreFloat4x4(&target.playerRoot, XMMatrixRotationY(std::atan2(world._31, world._33)) *
        XMMatrixTranslation(world._41, world._42, world._43));
    if (reuseTarget) target.playerRoot = m_KoukuEffectPreview->playerRoot;
    if (needsModel)
    {
        // Only actual source-bone attachments require a saved model binding.
        // Pure root/camera Effects retain their authored motion at the player.
        target.model.emplace();
        auto& staged = *target.model;
        staged.Set_Player(m_Player);
        if (sourceDocument && sourceDocument->SourceModelPreview)
        {
            if (!staged.Select_SourceEffect(*sourceDocument)) { m_Status = staged.Status(); return false; }
        }
        else
        {
            if (!staged.Reload()) { m_Status = staged.Status(); return false; }
            const auto& document = staged.Get_Document();
            std::set<std::string> resources;
            for (const auto& resource : document.PresentationResources)
                if (resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && resource.strAssetId == assetId &&
                    (resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT"))
                    resources.insert(resource.strResourceId);
            std::string selected;
            for (const auto& pattern : document.Patterns)
                if (std::any_of(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
                    [&](const auto& occurrence) { return resources.contains(occurrence.strResourceId); }))
                {
                    if (!selected.empty())
                    { m_Status = "Kouku source-bone Effect has multiple Composition patterns; its model binding is ambiguous."; return false; }
                    selected = pattern.strPatternId;
                }
            if (selected.empty())
            { m_Status = "Kouku source-bone Effect has no saved Composition model/animation binding."; return false; }
            if (!staged.Select_Pattern(selected)) { m_Status = staged.Status(); return false; }
        }
        if (staged.Actors().size() != 1u || !staged.Actors().front().status.empty() || staged.Rows().empty())
        { m_Status = "Kouku Effect has no valid saved model/animation target: " + staged.Status(); return false; }
    }
    m_PendingKoukuEffectPreview = std::move(target);
    m_Status = "Prepared Effect preview at the scene player's position and facing.";
    return true;
}

bool CEffectAuthoringSequencer::Select_ModelSequence(const std::string& id)
{
    const auto found = std::find_if(m_Sequences.begin(), m_Sequences.end(), [&](const auto& row) { return row.id == id; });
    if (found == m_Sequences.end() || !found->error.empty())
    { m_Status = found == m_Sequences.end() ? "Saved animation sequence is unavailable." : found->error; return false; }
    Stop(); m_BoxDetailDraft.reset(); m_UseKouku = false; m_CustomAnimation = false; m_AnimationRows.clear(); m_SelectedSequence = id; m_AnchorMember.clear(); m_ClockMs = 0;
    m_SourceModelEffectId.clear(); m_Dirty = true; m_Status = "Selected " + found->label; return true;
}
bool CEffectAuthoringSequencer::Select_Kouku(const std::string& id, const bool bundle)
{
    Stop();
    if (!(bundle ? m_Kouku.Select_Bundle(id) : m_Kouku.Select_Pattern(id))) { m_Status = m_Kouku.Status(); return false; }
    m_BoxDetailDraft.reset(); m_UseKouku = true; m_CustomAnimation = false; m_AnimationRows.clear(); m_ClockMs = 0;
    m_AnchorMember = m_Kouku.Actors().empty() ? "" : m_Kouku.Actors().front().memberId;
    m_SourceModelEffectId.clear(); m_Dirty = true; m_Status = m_Kouku.Status(); return true;
}
bool CEffectAuthoringSequencer::Begin_Model()
{
    if (m_KoukuEffectPreview && !m_KoukuEffectPreview->model) return true;
    if (m_KoukuEffectPreview)
    {
        if (g_ModelClockOwner && g_ModelClockOwner != this) g_ModelClockOwner->Stop();
        g_ModelClockOwner = this;
        auto& model = *m_KoukuEffectPreview->model;
        if (!model.Is_Active() && !model.Begin(ClockMs(), true))
        { m_Status = model.Status(); return false; }
        return Sample_Model(ClockMs());
    }
    if (g_ModelClockOwner && g_ModelClockOwner != this) g_ModelClockOwner->Stop();
    g_ModelClockOwner = this;
    if (m_UseKouku)
    {
        if (m_Kouku.Is_Active()) return Sample_Model(ClockMs());
        if (!m_Kouku.Begin(ClockMs(), true)) { m_Status = m_Kouku.Status(); return false; }
        return true;
    }
    if (m_TransientAnimationRows.empty() && !m_CustomAnimation && m_SelectedSequence.empty()) return true;
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
    if (m_KoukuEffectPreview)
    {
        if (!m_KoukuEffectPreview->model) return true;
        if (!m_KoukuEffectPreview->model->Sample(clockMs, true) ||
            !m_KoukuEffectPreview->model->Place_Root(m_KoukuEffectPreview->playerRoot))
        { m_Status = "Kouku source animation preview owner changed."; return false; }
        return true;
    }
    if (m_UseKouku)
    {
        const auto effect = std::find_if(m_Effects.begin(), m_Effects.end(), [&](const auto& row)
            { return row.key.strStableId == m_SourceModelEffectId; });
        const auto localMs = effect == m_Effects.end() ? clockMs : (clockMs > effect->startMs ? clockMs - effect->startMs : 0u);
        if (!m_Kouku.Is_Active() || !m_Kouku.Sample(localMs, true)) { m_Status = "Kouku model preview owner changed."; return false; }
        if (effect != m_Effects.end())
        {
            float4x4_t root;
            if (!Resolve_RowPivot(*effect, m_WorldRoot, root) || !m_Kouku.Place_Root(root))
            { m_Status = "The source model could not use its Effect group placement."; return false; }
        }
        return true;
    }
    const auto* sequence = Selected_Sequence();
    if (m_TransientAnimationRows.empty() && !m_CustomAnimation && (!sequence || m_SelectedSequence.empty())) return true;
    const auto model = m_Model.lock();
    if (!model || model != CAnimationTargetService::Resolve_Model() || m_ModelGeneration != CAnimationTargetService::Resolve_TargetGeneration())
    { m_Status = "Preview model target was replaced."; return false; }
    const CLIP* selected = nullptr;
    const bool recoveryAnimation = !m_TransientAnimationRows.empty();
    const auto& clips = recoveryAnimation ? m_TransientAnimationRows : (m_CustomAnimation ? m_AnimationRows : sequence->clips);
    for (const auto& clip : clips)
    {
        if (clip.muted || clip.startMs > clockMs) continue;
        if (!clip.clipName.empty() && (!selected || clip.startMs > selected->startMs)) selected = &clip;
    }
    if (!selected && m_CustomAnimation)
        for (const auto& clip : clips)
            if (!clip.muted && (!selected || clip.startMs < selected->startMs)) selected = &clip;
    if (!selected)
    {
        if (m_CustomAnimation || recoveryAnimation)
        {
            model->Set_Animation(m_PreviousClip, m_PreviousLoop); model->Skip_Blend();
            if (!model->Set_AnimTrackPosition(m_PreviousClip, m_PreviousPosition)) return false;
            model->Play_Animation(0.f); m_Panel->Synchronize_PreviewWeapon();
        }
        model->Set_AnimPaused(true); return true;
    }
    std::uint32_t index = 0u; float native = 0.f, tickRate = 0.f;
    if (!Clip_Metadata(model, selected->clipName, index, native, tickRate)) { m_Status = "Selected model clip is unavailable."; return false; }
    const ACTION_PRESENTATION_CLIP_TIMING timing{native / tickRate, selected->sourcePlayMs, selected->playRate, selected->loop, selected->sourceStartMs * .001f};
    const float budget = selected->durationMs * .001f;
    ACTION_PRESENTATION_SAMPLE sample;
    if (!CActionPresentationTimeline::Resolve_PreviewSequenceSample(std::span(&timing, 1u), std::span(&budget, 1u),
        (std::min)(clockMs > selected->startMs ? clockMs - selected->startMs : 0u, selected->durationMs) * .001f, sample))
    { m_Status = "Could not sample the Product animation source clock."; return false; }
    model->Set_Animation(index, false); model->Skip_Blend(); model->Set_AnimPaused(true);
    if (!model->Set_AnimTrackPosition(index, sample.fClipSourceTimeSeconds * tickRate)) return false;
    model->Play_Animation(0.f); m_Panel->Synchronize_PreviewWeapon(); return true;
}
bool CEffectAuthoringSequencer::Resolve_Root(float4x4_t& root)
{
    if (m_KoukuEffectPreview) { root = m_KoukuEffectPreview->playerRoot; return true; }
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

bool CEffectAuthoringSequencer::Validate_EffectPlacement(const EFFECT_ROW& row)
{
    const auto finite = [](const float3_t& value) { return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) &&
        std::abs(value.x) <= 100000.f && std::abs(value.y) <= 100000.f && std::abs(value.z) <= 100000.f; };
    if (!finite(row.offset) || !finite(row.rotation) || !finite(row.scale) ||
        row.scale.x <= 0.f || row.scale.y <= 0.f || row.scale.z <= 0.f ||
        row.scale.x > 1000.f || row.scale.y > 1000.f || row.scale.z > 1000.f ||
        (row.worldAnchor && row.anchorSlotId != "root"))
    { m_Status = "Group placement needs finite coordinates, positive scale and a matching anchor."; return false; }
    return row.worldAnchor || Validate_Anchor(row.anchorSlotId, m_ModelRoot, m_UseKouku);
}

bool CEffectAuthoringSequencer::Resolve_RowPivot(const EFFECT_ROW& row, const float4x4_t& root, float4x4_t& pivot)
{
    if (!Validate_EffectPlacement(row)) return false;
    matrix_t anchor = row.worldAnchor ? XMMatrixIdentity() : XMLoadFloat4x4(&root);
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
    XMStoreFloat4x4(&pivot, XMMatrixScaling(row.scale.x, row.scale.y, row.scale.z) *
        XMMatrixRotationRollPitchYaw(XMConvertToRadians(row.rotation.x), XMConvertToRadians(row.rotation.y), XMConvertToRadians(row.rotation.z)) *
        XMMatrixTranslation(row.offset.x, row.offset.y, row.offset.z) * anchor);
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
    const bool ownsSequence = Has_ModelSequence() || m_KoukuEffectPreview.has_value();
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
    if (!Validate_EffectPlacement(row)) return false;
    if (!row.previewElementIds.empty() && (row.key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT || row.startMs != 0u))
    { m_Status = "Element preview requires a V1 document at its original time origin."; return false; }
    if (row.screenPost && row.key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V2_LEAF)
    { m_Status = "Screen Post tracks require a saved Screen Post leaf."; return false; }
    if (row.muted) return true;
    if (row.key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT)
    {
        if (!m_V1Factory || !m_V1Release) { m_Status = "Effect document playback owner is unavailable."; return false; }
        if (!m_V1Factory(row.key, row.previewElementIds, root, row.v1, row.previewStartMs, row.durationMs, m_Status) || !row.v1)
        { Release_Row(row); return false; }
        if (row.bloomIntensityOverride &&
            !row.v1->Set_BloomIntensity(*row.bloomIntensityOverride, m_Status))
        { Release_Row(row); return false; }
        // The factory recomputes selected-element tails on Play/Refresh too.
        // Reapply only this temporary target's bounded source cycle afterward.
        if (m_KoukuEffectPreview && m_KoukuEffectPreview->assetId == row.key.strStableId &&
            m_KoukuEffectPreview->previewDurationMs)
            row.durationMs = *m_KoukuEffectPreview->previewDurationMs;
        if (!row.previewElementIds.empty() &&
            (!row.durationMs || row.durationMs > MAX_MS || row.previewStartMs >= row.durationMs))
        { m_Status = "The selected elements have no valid preview window."; Release_Row(row); return false; }
        row.v1->Set_Playing(false); row.v1->Set_Visible(false); return true;
    }
    if (!row.snapshot)
    {
        const auto kind = row.key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V2_LEAF ? EFFECT_V2_RESOURCE_KIND::LEAF : EFFECT_V2_RESOURCE_KIND::GROUP;
        if (!(m_V2Provider ? m_V2Provider(row.key, row.snapshot, m_Status) :
            CEffectV2Catalog::Get().Load_ResourceSnapshot(kind, row.key.strStableId, row.snapshot, m_Status))) return false;
    }
    if (!row.snapshot || !row.snapshot->Is_Ready()) { m_Status = "Effect resource snapshot is unavailable."; return false; }
    if (row.screenPost)
    {
        const auto* document = row.snapshot->Find_Document(row.key.strStableId);
        if (!document || document->eType != EFFECT_V2_TYPE::SCREEN_POST)
        { m_Status = "The selected Screen Post resource has a different Effect type."; return false; }
    }
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
bool CEffectAuthoringSequencer::Resolve_KoukuSourceAnchors(
    const EFFECT_DOCUMENT_DESC& document, const float4x4_t& root, const float seconds,
    std::unordered_map<std::string, float4x4_t>& anchors, std::string& error) const
{
    EFFECT_V2_TARGET target;
    EFFECT_V2_TARGET_VIEW view;
    const auto* model = m_KoukuEffectPreview ?
        (m_KoukuEffectPreview->model ? &*m_KoukuEffectPreview->model : nullptr) : &m_Kouku;
    const auto member = m_KoukuEffectPreview && model && !model->Actors().empty() ?
        model->Actors().front().memberId : m_AnchorMember;
    if (!Uses_KoukuSourceModel() || !model || !model->Resolve_Target(member, target, view))
    { error = "The selected Kouku animation target is unavailable."; return false; }
    return CKoukuSaydonPresentationPlayer::Sample_SourceAnchorWorlds(document, view, root, seconds, anchors, error);
}

bool CEffectAuthoringSequencer::Record_V1Anchors(EFFECT_ROW& row, const float4x4_t& pivot, const float age)
{
    if (!m_V1Anchors) { m_Status = "V1 source-anchor provider is unavailable."; return false; }
    std::unordered_map<std::string, float4x4_t> current;
    if (!m_V1Anchors(row.v1, pivot, Uses_KoukuSourceModel(), age, current, m_Status)) return false;
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
        const bool ownsSequence = Has_ModelSequence() || m_KoukuEffectPreview.has_value();
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
                if (!m_V1Anchors(row.v1, historicalPivot, Uses_KoukuSourceModel(), seconds, worlds, m_Status) || !record(seconds, worlds)) { success = false; break; }
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
bool CEffectAuthoringSequencer::Play(const bool paused)
{
    if (!m_Transient && m_Effects.empty() && m_CameraRows.empty() && m_Sounds.empty() && m_Colliders.empty() &&
        (!m_UseKouku && (m_CustomAnimation ? m_AnimationRows.empty() : m_SelectedSequence.empty())))
    { m_Status = "Select a model animation or append an Effect first."; return false; }
    if (!Validate_CameraRows(m_Transient ? m_TransientCameraRows : m_CameraRows)) return false;
    if (!m_TransientAnimationRows.empty())
    { if (!Validate_AnimationRows(m_TransientAnimationRows)) return false; }
    else if (!m_KoukuEffectPreview && m_CustomAnimation && !Validate_AnimationRows(m_AnimationRows)) return false;
    if (!m_Transient)
    {
        if (!Validate_SoundRows(m_Sounds) || !Validate_ColliderRows(m_Colliders)) return false;
        for (const auto& row : m_Colliders)
            if (!Validate_Anchor(row.anchorSlotId, m_ModelRoot, m_UseKouku)) return false;
    }
    const bool wasActive = m_Active;
    if (!Begin_Model()) return false;
    float4x4_t root;
    if (!Resolve_Root(root) || !Sample_Camera(ClockMs(), root)) { if (!wasActive) Stop(); return false; }
    const auto prepareRow = [&](EFFECT_ROW& row)
    {
        row.v1.reset(); row.v2 = 0u; row.sampledAge = -1.f;
        if (!wasActive || m_ClockMs == 0.0 ||
            (!row.previewElementIds.empty() && m_ClockMs == row.previewStartMs))
        { row.history.reset(); row.anchorHistory.reset(); row.recordedAge = -1.f; }
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
    m_Active = true; m_Paused = paused;
    m_SkipNextPlaybackDelta = true; m_Interaction = true;
    if (!Sample_Camera(ClockMs(), root)) { Stop(); return false; }
    if (!Sample_Sounds(true)) { Stop(); return false; }
    m_Status = "Playing all tracks on the same cursor."; return true;
}
bool CEffectAuthoringSequencer::Append(const EFFECT_RESOURCE_KEY& key, const std::uint32_t durationMs, const bool screenPost)
{
    std::optional<CEffectCompositionModelPreview> appendedModel;
    if (key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT && key.strStableId.starts_with("effect.kouku."))
    {
        if (!key.Is_Valid()) { m_Status = "Select an Effect with a stable ID."; return false; }
        EFFECT_DOCUMENT_DESC document;
        const auto path = CProjectDataRoot::Resolve(std::filesystem::path("Effects/Authored") / (key.strStableId + ".effect.json"));
        if (!CEffectDocumentCodec::Load(path, document, m_Status)) return false;
        if (document.SourceModelPreview)
        {
            appendedModel.emplace(); appendedModel->Set_Player(m_Player);
            if (!appendedModel->Select_SourceEffect(document)) { m_Status = appendedModel->Status(); return false; }
            if (m_UseKouku && !m_Effects.empty() && !m_Kouku.Actors().empty() &&
                m_Kouku.Actors().front().actorProfileId != appendedModel->Actors().front().actorProfileId)
            { m_Status = "This group needs a different model. Use a separate benchmark sequence for that actor."; return false; }
        }
    }
    const bool scenePreview = m_KoukuEffectPreview.has_value();
    const bool selectedPreview = Is_ElementPreview();
    const auto appendClock = scenePreview || selectedPreview ? 0u : ClockMs();
    if (m_Effects.size() + (m_Transient ? 1u : 0u) >= MAX_EFFECTS)
    { m_Status = "A sequence supports at most 256 Effect occurrences."; return false; }
    while (std::any_of(m_Effects.begin(), m_Effects.end(), [&](const auto& existing)
        { return existing.id == "effect.occurrence." + std::to_string(m_NextEffectOrdinal); })) ++m_NextEffectOrdinal;
    EFFECT_ROW row; row.id = "effect.occurrence." + std::to_string(m_NextEffectOrdinal);
    row.key = key; row.startMs = appendClock; row.durationMs = durationMs ? durationMs : 1u; row.screenPost = screenPost;
    row.anchorSlotId = m_DefaultAnchorSlotId;
    if (scenePreview)
    {
        const auto& world = m_KoukuEffectPreview->playerRoot;
        row.worldAnchor = true; row.anchorSlotId = "root";
        row.offset = {world._41, world._42, world._43};
        row.rotation.y = XMConvertToDegrees(std::atan2(world._31, world._33));
    }
    if (!scenePreview && key.strStableId.starts_with("effect.kouku."))
    {
        const auto character = CAnimationTargetService::Resolve_SceneCharacter();
        const auto transform = character ? character->Get_Transform() : nullptr;
        if (transform)
        {
            const auto& world = *transform->Get_WorldMatrixPtr();
            row.worldAnchor = true; row.anchorSlotId = "root"; row.offset = {world._41, world._42, world._43};
            row.rotation.y = XMConvertToDegrees(std::atan2(world._31, world._33));
        }
    }
    float4x4_t root = m_WorldRoot;
    if (m_Active && !Resolve_Root(root)) return false;
    if (!Stage_Row(row, root)) { Release_Row(row); return false; }
    if (!durationMs)
    {
        const double resolved = row.v1 ? std::ceil(static_cast<double>(row.v1->Get_PreviewDurationSeconds()) * 1000.0) : 0.0;
        if (!std::isfinite(resolved) || resolved < 1.0 || resolved > MAX_MS || row.startMs > MAX_MS - resolved)
        { m_Status = "The Effect owner did not provide a valid occurrence duration."; Release_Row(row); return false; }
        row.durationMs = static_cast<std::uint32_t>(resolved);
    }
    if (m_Active && !scenePreview && !selectedPreview && !appendedModel && !Sample_Row(row, root)) { Release_Row(row); return false; }
    if (scenePreview || selectedPreview)
    {
        std::optional<CEffectCompositionModelPreview> sourceModel;
        std::string sourceEffect;
        if (scenePreview && m_KoukuEffectPreview->model)
        { sourceModel = std::move(m_KoukuEffectPreview->model); sourceEffect = m_KoukuEffectPreview->assetId; }
        Stop();
        if (sourceModel)
        {
            m_Kouku = std::move(*sourceModel);
            m_SourceModelEffectId = m_Kouku.Selected_Id().starts_with("effect.model.") ? std::move(sourceEffect) : std::string{};
            m_UseKouku = true; m_CustomAnimation = false; m_AnimationRows.clear();
            m_AnchorMember = m_Kouku.Actors().front().memberId;
        }
        m_ClockMs = appendClock;
    }
    else if (!Commit_TransientPreview()) { Release_Row(row); return false; }
    if (appendedModel)
    {
        Stop();
        m_Kouku = std::move(*appendedModel); m_SourceModelEffectId = key.strStableId;
        m_UseKouku = true; m_CustomAnimation = false; m_AnimationRows.clear();
        m_AnchorMember = m_Kouku.Actors().front().memberId; m_ClockMs = appendClock;
    }
    Select_TimelineRow(screenPost ? TRACK_KIND::SCREEN_POST : TRACK_KIND::EFFECT, row.id);
    m_Effects.push_back(std::move(row)); ++m_NextEffectOrdinal;
    m_Dirty = true; m_Status = "Appended " + key.strStableId + " at " + std::to_string(ClockMs()) + " ms."; return true;
}
bool CEffectAuthoringSequencer::Preview(const EFFECT_RESOURCE_KEY& key, const std::uint32_t durationMs)
{
    // The Resources tree reaches this entry without an active Tool document.
    if (key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT && key.strStableId.starts_with("effect.world.") &&
        (!m_PendingKoukuEffectPreview || m_PendingKoukuEffectPreview->assetId != key.strStableId) &&
        !Select_WorldEffect(key.strStableId)) return false;
    std::vector<CAMERA_ROW> cameras; std::vector<CLIP> animations;
    if (!Read_RecoveryCameras(key, cameras, animations)) return false;
    const bool cameraOnlyV4 = m_Status.starts_with("Recovery camera preview uses");
    const bool replaceTarget = m_PendingKoukuEffectPreview.has_value() ||
        (m_KoukuEffectPreview && m_KoukuEffectPreview->assetId != key.strStableId);
    if (m_PendingKoukuEffectPreview && m_PendingKoukuEffectPreview->assetId != key.strStableId)
    { m_Status = "The staged scene player anchor belongs to another Effect."; return false; }
    std::optional<KOUKU_EFFECT_PREVIEW_TARGET> previousTarget;
    if (replaceTarget)
    {
        previousTarget = std::move(m_KoukuEffectPreview);
        m_KoukuEffectPreview = std::move(m_PendingKoukuEffectPreview);
        m_PendingKoukuEffectPreview.reset();
    }
    const bool wasActive = m_Active;
    const double previousClock = m_ClockMs;
    auto previousAnimations = std::move(m_TransientAnimationRows);
    if (m_KoukuEffectPreview) m_ClockMs = 0.0;
    const auto rollback = [&]()
    {
        const auto failure = m_Status;
        if (replaceTarget)
        {
            if (m_KoukuEffectPreview && m_KoukuEffectPreview->model) m_KoukuEffectPreview->model->Stop();
            m_KoukuEffectPreview = std::move(previousTarget);
        }
        m_ClockMs = previousClock;
        m_TransientAnimationRows = std::move(previousAnimations);
        if (!wasActive) Stop();
        else
        {
            float4x4_t priorRoot;
            if (!Begin_Model() || !Resolve_Root(priorRoot) || !Sample_Camera(ClockMs(), priorRoot)) Stop();
        }
        m_Status = failure;
    };
    for (auto& row : cameras)
    {
        if (ClockMs() > MAX_MS - row.startMs - row.cue.iDurationMs)
        { m_Status = "Recovery camera rows exceed the sequence duration at this cursor."; rollback(); return false; }
        row.startMs += ClockMs();
    }
    for (auto& row : animations)
    {
        if (ClockMs() > MAX_MS - row.startMs - row.durationMs)
        { m_Status = "Recovery animation exceeds the sequence duration at this cursor."; rollback(); return false; }
        row.startMs += ClockMs();
    }
    EFFECT_ROW staged; staged.id = "effect.preview"; staged.key = key; staged.startMs = ClockMs(); staged.durationMs = durationMs;
    staged.previewLoop = m_KoukuEffectPreview ? m_KoukuEffectPreview->loopPolicy.value_or(m_Loop) : m_Loop;
    staged.anchorSlotId = m_KoukuEffectPreview ? "root" : m_DefaultAnchorSlotId;
    m_TransientAnimationRows = std::move(animations);
    if (!Begin_Model()) { rollback(); return false; }
    float4x4_t root;
    if (!Resolve_Root(root)) { rollback(); return false; }
    if (!Sample_Camera(ClockMs(), root, &cameras) || !Stage_Row(staged, root))
    { Release_Row(staged); rollback(); return false; }
    if (m_KoukuEffectPreview && m_KoukuEffectPreview->loopPolicy.has_value() && staged.v1)
    {
        // A persistent marker loops its source emission window before the
        // bounded playback tail fades out; the source simulation stays unchanged.
        const double actualDuration = m_KoukuEffectPreview->previewDurationMs ?
            static_cast<double>(*m_KoukuEffectPreview->previewDurationMs) :
            std::ceil(static_cast<double>(staged.v1->Get_PreviewDurationSeconds()) * 1000.0);
        if (!std::isfinite(actualDuration) || actualDuration < 1.0 || actualDuration > MAX_MS)
        { m_Status = "The World Effect has no valid playback duration."; Release_Row(staged); rollback(); return false; }
        staged.durationMs = static_cast<std::uint32_t>(actualDuration);
    }
    if (!Sample_Row(staged, root)) { Release_Row(staged); rollback(); return false; }
    for (auto& row : m_Effects) Release_Row(row);
    if (m_Transient) Release_Row(*m_Transient);
    if (previousTarget && previousTarget->model) previousTarget->model->Stop();
    if (m_KoukuEffectPreview)
    {
        m_Kouku.Stop();
        if (!m_KoukuEffectPreview->model && g_ModelClockOwner == this) g_ModelClockOwner = nullptr;
    }
    m_Transient = std::move(staged); m_TransientCameraRows = std::move(cameras);
    Stop_Sounds();
    m_SelectedCamera.clear(); m_Active = true; m_Paused = false;
    m_SkipNextPlaybackDelta = true; m_Interaction = true;
    if (!Sample_Camera(ClockMs(), root)) { Stop(); return false; }
    m_Status = m_KoukuEffectPreview ? "Playing all Elements at the captured player anchor: " + key.strStableId :
        "Previewing " + key.strStableId + ". Append adds it to the saved sequence.";
    if (cameraOnlyV4) m_Status += " Load sequence restores its additional Sound and Collider tracks.";
    return true;
}

bool CEffectAuthoringSequencer::Preview_Element(const EFFECT_RESOURCE_KEY& key, const std::string& elementId,
    const std::string& label, const std::uint32_t durationMs, const std::uint32_t focusMs)
{
    return Preview_Elements(key, {elementId}, label, durationMs, focusMs, m_Loop);
}
bool CEffectAuthoringSequencer::Preview_Elements(const EFFECT_RESOURCE_KEY& key,
    const std::vector<std::string>& elementIds, const std::string& label,
    const std::uint32_t durationMs, const std::uint32_t focusMs, const bool loop)
{
    Preserve_ClockDuringAuthoring();
    if (key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT || elementIds.empty() ||
        std::any_of(elementIds.begin(), elementIds.end(), [](const auto& id) { return id.empty(); }) ||
        !durationMs || durationMs > MAX_MS || focusMs >= durationMs)
    { m_Status = "Select a document element and a focus time inside its preview duration."; return false; }

    const bool replaceTarget = m_PendingKoukuEffectPreview.has_value() ||
        (m_KoukuEffectPreview && m_KoukuEffectPreview->assetId != key.strStableId);
    if (m_PendingKoukuEffectPreview && m_PendingKoukuEffectPreview->assetId != key.strStableId)
    { m_Status = "The staged scene player anchor belongs to another Effect."; return false; }
    std::optional<KOUKU_EFFECT_PREVIEW_TARGET> previousTarget;
    if (replaceTarget)
    {
        previousTarget = std::move(m_KoukuEffectPreview);
        m_KoukuEffectPreview = std::move(m_PendingKoukuEffectPreview);
        m_PendingKoukuEffectPreview.reset();
    }
    const auto restoreTarget = [&]()
    {
        if (!replaceTarget) return;
        if (m_KoukuEffectPreview && m_KoukuEffectPreview->model) m_KoukuEffectPreview->model->Stop();
        m_KoukuEffectPreview = std::move(previousTarget);
    };
    EFFECT_ROW staged;
    staged.id = "effect.preview.element"; staged.key = key;
    staged.previewElementIds = elementIds; staged.previewElementLabel = label;
    staged.previewStartMs = focusMs;
    staged.previewLoop = m_KoukuEffectPreview ? m_KoukuEffectPreview->loopPolicy.value_or(loop) : loop;
    staged.startMs = 0u; staged.durationMs = m_KoukuEffectPreview ?
        m_KoukuEffectPreview->previewDurationMs.value_or(durationMs) : durationMs;
    staged.anchorSlotId = m_KoukuEffectPreview ? "root" : m_DefaultAnchorSlotId;
    float4x4_t root;
    // A first Kouku Solo needs its typed model before the root can resolve.
    // Existing character targets and an active Kouku preview keep their order.
    const bool startedKoukuModel = Uses_KoukuSourceModel() &&
        (m_KoukuEffectPreview ? !m_KoukuEffectPreview->model->Is_Active() : !m_Kouku.Is_Active());
    const auto releaseStaged = [&]()
    {
        const auto failure = m_Status;
        Release_Row(staged);
        restoreTarget();
        if (m_Active)
        {
            float4x4_t priorRoot;
            if (!Begin_Model() || !Resolve_Root(priorRoot) || !Sample_Camera(ClockMs(), priorRoot)) Stop();
        }
        else if (startedKoukuModel || replaceTarget) Stop();
        m_Status = failure;
    };
    if (startedKoukuModel && !Begin_Model()) { releaseStaged(); return false; }
    // Resource preparation must not release the previous preview or move its clock.
    if (!Resolve_Root(root) || !Stage_Row(staged, root)) { releaseStaged(); return false; }
    if (focusMs >= staged.durationMs)
    { m_Status = "The selected focus time exceeds the staged element duration."; releaseStaged(); return false; }

    const bool wasActive = m_Active;
    const double previousClock = m_ClockMs;
    const bool previousPaused = m_Paused, previousSkipDelta = m_SkipNextPlaybackDelta;
    const bool previousInteraction = m_Interaction;
    const std::vector<CAMERA_ROW> cameras;
    auto previousAnimations = std::move(m_TransientAnimationRows);
    m_TransientAnimationRows.clear();
    m_ClockMs = focusMs;
    // The row retains the document's time origin, including its native start delay.
    // Recovery camera rows belong to whole-document Preview, not element Solo.
    if (!Begin_Model() || !Resolve_Root(root) || !Sample_Camera(focusMs, root, &cameras) || !Sample_Row(staged, root))
    {
        const auto failure = m_Status;
        Release_Row(staged);
        m_ClockMs = previousClock;
        restoreTarget();
        m_TransientAnimationRows = std::move(previousAnimations);
        if (!wasActive) Stop();
        else if (!Begin_Model() || !Resolve_Root(root) || !Sample_Camera(ClockMs(), root))
        {
            const auto restoreFailure = m_Status;
            Stop();
            m_Status = failure + " Previous preview could not be restored: " + restoreFailure;
            return false;
        }
        m_Paused = previousPaused; m_SkipNextPlaybackDelta = previousSkipDelta; m_Interaction = previousInteraction;
        m_Status = failure; return false;
    }

    for (auto& row : m_Effects) Release_Row(row);
    if (m_Transient) Release_Row(*m_Transient);
    if (previousTarget && previousTarget->model) previousTarget->model->Stop();
    if (m_KoukuEffectPreview)
    {
        m_Kouku.Stop();
        if (!m_KoukuEffectPreview->model && g_ModelClockOwner == this) g_ModelClockOwner = nullptr;
    }
    m_Transient = std::move(staged); m_TransientCameraRows.clear();
    Stop_Sounds();
    m_Active = true; m_Paused = false; m_SkipNextPlaybackDelta = true; m_Interaction = true;
    m_BoxDetailDraft.reset();
    Select_TimelineRow(TRACK_KIND::EFFECT, m_Transient->id);
    m_Status = (loop ? "Looping selected elements: " : "Playing element: ") +
        (label.empty() ? elementIds.front() : label) + " from " + std::to_string(focusMs) +
        " ms of the original document. Stop returns to the saved sequence.";
    return true;
}

bool CEffectAuthoringSequencer::Sample(const bool forceSeekSounds)
{
    if (!Sample_Model(ClockMs())) return false;
    float4x4_t root; if (!Resolve_Root(root)) return false;
    if (!Sample_Camera(ClockMs(), root)) return false;
    if (m_Transient) { if (!Sample_Row(*m_Transient, root)) return false; }
    else for (auto& row : m_Effects) if (!Sample_Row(row, root)) return false;
    return Sample_Sounds(forceSeekSounds);
}
bool CEffectAuthoringSequencer::Seek(const std::uint32_t clockMs)
{
    m_ClockMs = (std::min)(clockMs, DurationMs()); m_Paused = true;
    if (!m_Active) return Play(true);
    if (!Sample(true)) { Stop(); return false; } return true;
}
void CEffectAuthoringSequencer::Pause(const bool paused)
{
    if (m_Paused && !paused) m_SkipNextPlaybackDelta = true;
    m_Paused = paused;
    if (m_Active && !Sample_Sounds(false)) Stop();
}
void CEffectAuthoringSequencer::Stop()
{
    Stop_Sounds();
    if (g_ModelClockOwner == this) g_ModelClockOwner = nullptr;
    Release_Camera(); m_TransientCameraRows.clear(); m_TransientAnimationRows.clear();
    for (auto& row : m_Effects) Release_Row(row);
    if (m_Transient) { Release_Row(*m_Transient); m_Transient.reset(); }
    if (m_KoukuEffectPreview && m_KoukuEffectPreview->model) m_KoukuEffectPreview->model->Stop();
    m_KoukuEffectPreview.reset(); m_PendingKoukuEffectPreview.reset();
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
        const bool elementPreview = Is_ElementPreview();
        if (Uses_TransientLoop() ? m_Transient->previewLoop : m_Loop)
        {
            Stop_Sounds();
            m_ClockMs = elementPreview ? m_Transient->previewStartMs : 0u;
            if (m_KoukuEffectPreview && m_KoukuEffectPreview->loopPolicy.has_value())
            {
                // World markers keep the staged occurrence and rewind its clock;
                // repeated GPU preparation and a skipped delta would interrupt the loop.
                if (!Sample(true)) Stop();
            }
            else if (!Play()) Stop();
        }
        else Pause(true);
    }
}

bool CEffectAuthoringSequencer::Uses_Resource(const EFFECT_RESOURCE_KEY& key) const
{
    if (m_Transient) return m_Transient->key == key;
    return std::any_of(m_Effects.begin(), m_Effects.end(),
        [&](const EFFECT_ROW& row) { return row.key == key; });
}

bool CEffectAuthoringSequencer::Set_BloomIntensity(
    const EFFECT_RESOURCE_KEY& key, const float value, std::string& error)
{
    if (key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT ||
        !key.Is_Valid() || !Is_ValidEffectBloomIntensity(value))
    { error = "Effect bloomIntensity requires a V1 document and a finite value from 0 to 16."; return false; }
    // The scalar changes existing draw state only: no stage, seek or clock reset.
    const auto apply = [&](EFFECT_ROW& row)
    {
        if (!(row.key == key)) return true;
        if (row.v1 && !row.v1->Set_BloomIntensity(value, error)) return false;
        row.bloomIntensityOverride = value;
        return true;
    };
    if (m_Transient && !apply(*m_Transient)) return false;
    for (auto& row : m_Effects) if (!apply(row)) return false;
    error.clear();
    return true;
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
        staged.bloomIntensityOverride.reset(); // Reload/Revert use the newly staged document value.
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
        row.bloomIntensityOverride.reset();
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
    if (ImGui::SmallButton("No model sequence")) { Stop(); m_SelectedSequence.clear(); m_AnimationRows.clear(); m_CustomAnimation = false; m_UseKouku = false; m_Dirty = true; }
    int source = m_UseKouku ? 1 : 0;
    if (ImGui::Combo("Pattern source", &source, "Selected character / Valtan\0KoukuSaydon Patterns\0"))
    { Stop(); m_UseKouku = source == 1; m_AnimationRows.clear(); m_CustomAnimation = false; m_ClockMs = 0; m_Dirty = true; }
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
    ImGui::TextDisabled("Saved bindings are references. Composition Resources adds editable Animation rows to this preview sequence.");
    ImGui::PopID();
}

bool CEffectAuthoringSequencer::Save_Sequence()
{
    if (m_Transient)
    {
        m_Status = m_Transient->previewElementIds.empty() ? "Append the preview and camera rows before saving the sequence." :
            "Element preview is temporary. Stop it before saving the existing sequence.";
        return false;
    }
    if (!Validate_CameraRows(m_CameraRows)) return false;
    if ((m_CustomAnimation && m_UseKouku) || (!m_CustomAnimation && !m_AnimationRows.empty()))
    { m_Status = "Custom Animation rows need their single model sequence owner."; return false; }
    if (m_CustomAnimation && (m_AssetName != CAnimationTargetService::Resolve_AssetName() || !m_Panel || !m_Panel->Is_PreviewActive()))
    { m_Status = "Restore the custom Animation draft's model before saving its source windows."; return false; }
    if (!Validate_AnimationRows(m_AnimationRows) || !Validate_SoundRows(m_Sounds) || !Validate_ColliderRows(m_Colliders)) return false;
    for (const auto& row : m_Colliders)
        if (!Validate_Anchor(row.anchorSlotId, m_ModelRoot, m_UseKouku)) return false;
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
    out << "{\n  \"schema\": \"lostark.effect-authoring-sequence\",\n  \"formatVersion\": 4,\n  \"sequenceId\": \"" << CDataJson::Escape(id)
        << "\",\n  \"model\": {\"kind\": \"" << (m_UseKouku ? (!m_SourceModelEffectId.empty() ? "KOUKU_EFFECT" : (m_Kouku.Selected_IsBundle() ? "KOUKU_BUNDLE" : "KOUKU_PATTERN")) : "MODEL_SEQUENCE")
        << "\", \"assetName\": \"" << CDataJson::Escape(m_AssetName) << "\", \"sequenceId\": \""
        << CDataJson::Escape(m_UseKouku ? (!m_SourceModelEffectId.empty() ? m_SourceModelEffectId : m_Kouku.Selected_Id()) : m_SelectedSequence) << "\", \"anchorMemberId\": \"" << CDataJson::Escape(m_AnchorMember)
        << "\"},\n  \"anchorMode\": \"" << (m_ModelRoot ? "MODEL_ROOT" : "WORLD")
        << "\",\n  \"worldPosition\": [" << m_WorldRoot._41 << ", " << m_WorldRoot._42 << ", " << m_WorldRoot._43 << "],\n  \"effects\": [";
    std::set<std::string> ids;
    for (std::size_t i = 0; i < m_Effects.size(); ++i)
    {
        const auto& row = m_Effects[i];
        if (!Validate_EffectPlacement(row)) return false;
        if (!row.key.Is_Valid() || (row.screenPost && row.key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V2_LEAF) ||
            !ids.insert(row.id).second || !CEffectV2Document::Is_ValidEffectId(row.id) ||
            !row.durationMs || row.durationMs > MAX_MS || row.startMs > MAX_MS - row.durationMs ||
            !std::isfinite(row.offset.x) || !std::isfinite(row.offset.y) || !std::isfinite(row.offset.z) ||
            std::abs(row.offset.x) > 100000.f || std::abs(row.offset.y) > 100000.f || std::abs(row.offset.z) > 100000.f)
        { m_Status = "Sequence contains an invalid Effect occurrence."; return false; }
        out << (i ? ",\n" : "\n") << "    {\"occurrenceId\": \"" << CDataJson::Escape(row.id) << "\", \"owner\": \"" << Owner_Key(row.key.eOwnerKind)
            << "\", \"effectId\": \"" << CDataJson::Escape(row.key.strStableId) << "\", \"anchorSlotId\": \"" << CDataJson::Escape(row.anchorSlotId) << "\", \"startMs\": " << row.startMs << ", \"durationMs\": " << row.durationMs
            << ", \"offset\": [" << row.offset.x << ", " << row.offset.y << ", " << row.offset.z << "], \"muted\": " << (row.muted ? "true" : "false")
            << ", \"screenPost\": " << (row.screenPost ? "true" : "false")
            << ", \"worldAnchor\": " << (row.worldAnchor ? "true" : "false")
            << ", \"rotationDegrees\": [" << row.rotation.x << ", " << row.rotation.y << ", " << row.rotation.z << "]"
            << ", \"scale\": [" << row.scale.x << ", " << row.scale.y << ", " << row.scale.z << "]}";
    }
    out << "\n  ]"; Write_CameraRows(out); Write_AdditionalRows(out); out << "\n}\n";
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
        !Json_U32(root, "formatVersion", version, 4u) || version < 1u || !Json_Text(root, "sequenceId", storedId) || storedId != id)
    { m_Status = "Invalid Effect sequence header; current timeline preserved."; return false; }
    const auto* model = root.Find("model"); const auto* effects = root.Find("effects"); float3_t world;
    if (!model || !model->Is_Object() || !Json_Text(*model, "kind", kind) || !Json_Text(*model, "assetName", asset) ||
        !Json_Text(*model, "sequenceId", sequenceId) || !Json_Text(*model, "anchorMemberId", anchor) ||
        (kind != "MODEL_SEQUENCE" && kind != "KOUKU_PATTERN" && kind != "KOUKU_BUNDLE" && kind != "KOUKU_EFFECT") || !Json_Vector(root, "worldPosition", world) ||
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
    std::vector<CLIP> animations; std::vector<SOUND_ROW> sounds; std::vector<COLLIDER_ROW> colliders; bool customAnimation = false;
    if (!Parse_AdditionalRows(root, version, animations, customAnimation, sounds, colliders)) return false;
    if (customAnimation && kind != "MODEL_SEQUENCE")
    { m_Status = "Custom Animation rows require a single model owner; current timeline preserved."; return false; }
    for (const auto& row : colliders)
    {
        if (!Validate_Anchor(row.anchorSlotId, modelRoot, kind != "MODEL_SEQUENCE")) return false;
        if (row.anchorSlotId != "root" && asset != CAnimationTargetService::Resolve_AssetName())
        { m_Status = "Load the saved character before resolving its Collider bones; current timeline preserved."; return false; }
    }
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
        if (value.Find("worldAnchor") || value.Find("rotationDegrees") || value.Find("scale"))
        {
            const auto* worldAnchor = value.Find("worldAnchor");
            if (!worldAnchor || !worldAnchor->Is_Boolean() || !Json_Vector(value, "rotationDegrees", row.rotation) ||
                !Json_Vector(value, "scale", row.scale))
            { m_Status = "Saved group placement is incomplete; current timeline preserved."; return false; }
            row.worldAnchor = worldAnchor->Get_Boolean();
        }
        if (version >= 2u && !Json_Text(value, "anchorSlotId", row.anchorSlotId))
        { m_Status = "Saved v2 Effect occurrence is missing its anchor; current timeline preserved."; return false; }
        if (version >= 4u)
        {
            const auto* post = value.Find("screenPost");
            if (!post || !post->Is_Boolean()) { m_Status = "Saved v4 Effect occurrence is missing its track kind; current timeline preserved."; return false; }
            row.screenPost = post->Get_Boolean();
        }
        const auto bounded = [](const float3_t& v, const float limit) { return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z) &&
            std::abs(v.x) <= limit && std::abs(v.y) <= limit && std::abs(v.z) <= limit; };
        if (!bounded(row.offset, 100000.f) || !bounded(row.rotation, 100000.f) || !bounded(row.scale, 1000.f) ||
            row.scale.x <= 0.f || row.scale.y <= 0.f || row.scale.z <= 0.f || (row.worldAnchor && row.anchorSlotId != "root"))
        { m_Status = "Saved group placement is out of range; current timeline preserved."; return false; }
        if (!row.worldAnchor && !Validate_Anchor(row.anchorSlotId, modelRoot, kind != "MODEL_SEQUENCE")) return false;
        if (row.anchorSlotId != "root" && asset != CAnimationTargetService::Resolve_AssetName())
        { m_Status = "Load the saved character before resolving its occurrence bones; current timeline preserved."; return false; }
        if (owner == "V1_DOCUMENT") row.key.eOwnerKind = EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT;
        else if (owner == "V2_LEAF") row.key.eOwnerKind = EFFECT_RESOURCE_OWNER_KIND::V2_LEAF;
        else if (owner == "V2_GROUP") row.key.eOwnerKind = EFFECT_RESOURCE_OWNER_KIND::V2_GROUP;
        else { m_Status = "Unknown saved Effect owner; current timeline preserved."; return false; }
        if (!row.key.Is_Valid() || (row.screenPost && row.key.eOwnerKind != EFFECT_RESOURCE_OWNER_KIND::V2_LEAF))
        { m_Status = "Invalid typed Effect ID or Screen Post owner; current timeline preserved."; return false; }
        row.muted = muted->Get_Boolean(); staged.push_back(std::move(row));
    }
    if (kind == "MODEL_SEQUENCE" && (customAnimation || !sequenceId.empty()))
    {
        if (asset != CAnimationTargetService::Resolve_AssetName() || !m_Panel || !m_Panel->Is_PreviewActive())
        { m_Status = "Select " + asset + " in Model View, reload its ordered clips, then load this sequence. Current timeline preserved."; return false; }
        if (!customAnimation && m_InventoryGeneration != CAnimationTargetService::Resolve_TargetGeneration())
        { m_Status = "Reload saved ordered clips before loading this model sequence."; return false; }
        if (!customAnimation)
        {
            const auto found = std::find_if(m_Sequences.begin(), m_Sequences.end(), [&](const auto& row) { return row.id == sequenceId; });
            if (found == m_Sequences.end() || !found->error.empty()) { m_Status = "Saved model sequence reference is unavailable."; return false; }
        }
    }
    std::optional<CEffectCompositionModelPreview> sourceModel;
    if (kind == "KOUKU_EFFECT")
    {
        if (!CEffectV2Document::Is_ValidEffectId(sequenceId))
        { m_Status = "Saved source Effect identity is malformed; current timeline preserved."; return false; }
        EFFECT_DOCUMENT_DESC effect;
        const auto effectPath = CProjectDataRoot::Resolve(std::filesystem::path("Effects/Authored") / (sequenceId + ".effect.json"));
        if (!CEffectDocumentCodec::Load(effectPath, effect, m_Status) || effect.strEffectAssetId != sequenceId) return false;
        sourceModel.emplace(); sourceModel->Set_Player(m_Player);
        if (!sourceModel->Select_SourceEffect(effect)) { m_Status = sourceModel->Status(); return false; }
        if (sourceModel->Actors().empty() || sourceModel->Actors().front().memberId != anchor)
        { m_Status = "Saved source Effect actor changed; current timeline preserved."; return false; }
    }
    else if (kind != "MODEL_SEQUENCE")
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
    Stop(); m_BoxDetailDraft.reset(); m_Effects = std::move(staged); m_CameraRows = std::move(cameras); m_SelectedCamera.clear(); m_ClockMs = 0; m_SelectedEffect.clear(); m_UseKouku = kind != "MODEL_SEQUENCE";
    m_AnimationRows = std::move(animations); m_CustomAnimation = customAnimation;
    m_Sounds = std::move(sounds); m_Colliders = std::move(colliders); m_SelectedRowId.clear();
    if (m_CustomAnimation) m_InventoryGeneration = CAnimationTargetService::Resolve_TargetGeneration();
    m_SourceModelEffectId = sourceModel ? sequenceId : std::string{};
    if (sourceModel) m_Kouku = std::move(*sourceModel);
    else if (m_UseKouku) { if (kind == "KOUKU_BUNDLE") m_Kouku.Select_Bundle(sequenceId); else m_Kouku.Select_Pattern(sequenceId); }
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
