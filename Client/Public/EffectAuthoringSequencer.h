#pragma once
#include "EffectRecoveryCamera.h"

#include "AnimationTargetService.h"
#include "HitAreaWire.h"
#include "CharacterPreviewPanel.h"
#include "Network/PacketMessages.h"
#include "EffectCompositionModelPreview.h"
#include "Effect_AuthoringDocument.h"
#include "ValtanCinematicCameraDocument.h"
#include "CompositionResourceTree.h"
#include "CompositionAnimationResource.h"
#include "CompositionEditing.h"
#include <array>
#include <iosfwd>
#include "EffectResourceCatalog.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine { class CCamera; }
namespace Client
{
class CEffectObject;
class DATA_JSON_VALUE;
struct EFFECT_DOCUMENT_DESC;
struct ANIMATION_SKILL_BINDING;
struct ANIMATION_EFFECT_CUE_DOCUMENT;
struct CHARACTER_ACTION_COMBAT_ROW;
struct VALTAN_CLIP_OCCURRENCE_VIEW;

// One editor clock. The resource owners retain their codecs and prepare each
// independent occurrence; this adapter owns only its lifetime and sampling.
class CEffectAuthoringSequencer final
{
public:
    using V1_FACTORY = std::function<bool(const EFFECT_RESOURCE_KEY&, const std::vector<std::string>&, const float4x4_t&,
        std::shared_ptr<CEffectObject>&, std::uint32_t&, std::uint32_t&, std::string&)>;
    using V1_RELEASE = std::function<void(const std::shared_ptr<CEffectObject>&)>;
    using V1_ANCHOR_PROVIDER = std::function<bool(const std::shared_ptr<CEffectObject>&,
        const float4x4_t&, bool, float, std::unordered_map<std::string, float4x4_t>&, std::string&)>;
    using V2_SNAPSHOT_PROVIDER = std::function<bool(const EFFECT_RESOURCE_KEY&,
        std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>&, std::string&)>;

    CEffectAuthoringSequencer(ComPtr<ID3D11Device> device,
        ComPtr<ID3D11DeviceContext> context, std::shared_ptr<CCharacterPreviewPanel> panel,
        const char* sequenceId = "effect.sequence.default");
    ~CEffectAuthoringSequencer();
    void Set_Player(CKoukuSaydonPresentationPlayer* player);
    void Set_Camera(const std::shared_ptr<Engine::CCamera>& camera);
    void Set_V1Callbacks(V1_FACTORY factory, V1_RELEASE release);
    void Set_V1AnchorProvider(V1_ANCHOR_PROVIDER provider);
    bool Resolve_KoukuSourceAnchors(const EFFECT_DOCUMENT_DESC& document, const float4x4_t& root, float seconds,
        std::unordered_map<std::string, float4x4_t>& anchors, std::string& error) const;
    void Set_V2SnapshotProvider(V2_SNAPSHOT_PROVIDER provider);
    void Render_WorkbenchDetail() { Render_BoxDetail(true); }
    void Render_WorkbenchResources() { Render_CompositionResources(true); }
    bool Is_Dirty() const { return m_Dirty; }
    bool Open_CharacterModelSequence(const std::string& sequenceId, bool loadSaved = true);
    bool Export_CharacterModelAction(ANIMATION_SKILL_BINDING& binding,
        ANIMATION_EFFECT_CUE_DOCUMENT& cues, const std::string& soundOwner, std::string& status,
        bool presentationOnly = false);
    bool Can_AppendCharacterAnimation(const COMPOSITION_ANIMATION_RESOURCE& resource,
        bool replace, std::string& status) const;
    bool Append_CharacterAnimation(const COMPOSITION_ANIMATION_RESOURCE& resource,
        bool replace, std::string& status);
    bool Rebind_CharacterModel(std::string& status);
    bool Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND command, std::string& status);
    bool Insert_CompositionTransfer(const COMPOSITION_TRANSFER& transfer, std::string& status);
    void Set_WorkbenchSaveCallback(std::function<bool()> callback) { m_WorkbenchSave = std::move(callback); }
    bool Save_WorkbenchSequence() { return Save_Sequence(); }
    void Refresh_ModelResources() { m_ResourceModelGeneration = ~std::uint64_t{0u}; }

    void Render_ModelView(); // Contents inside the existing Model View window.
    void Render_PreviewPlacementControls();
    bool Update_PreviewPlacementInput(bool active);
    void Render_Sequencer(const char* title = "Sequencer##EffectAuthoring", bool integratedEffectWorkspace = false, bool embedded = false); // The existing Effect Tool calls this panel.
    void Update(float dt, bool active);
    bool Select_CharacterSkill(const std::string& asset, std::uint32_t skillId,
        std::optional<std::uint32_t> stageIndex = std::nullopt);
    bool Stage_CharacterAction(const std::string& asset, const ANIMATION_SKILL_BINDING& binding,
        const ANIMATION_EFFECT_CUE_DOCUMENT& cues, const std::vector<CHARACTER_ACTION_COMBAT_ROW>& combat,
        std::optional<std::uint32_t> stageIndex = std::nullopt, const std::string& soundOwner = {},
        const ANIMATION_EFFECT_CUE_DOCUMENT* externalOwnerCues = nullptr);
    void Render_PreviewOverlays() { Render_Colliders(); }
    std::uint32_t Preview_DurationMs() const { return DurationMs(); }
    bool Select_KoukuEffect(const std::string& assetId, bool requiresSourceModel, bool reusePlayerAnchor = false,
        const EFFECT_DOCUMENT_DESC* sourceDocument = nullptr,
        std::optional<std::uint32_t> previewDurationMs = std::nullopt, std::uint32_t modelStartMs = 0u,
        bool loopEffectToDuration = false);
    bool Select_WorldEffect(const std::string& assetId, bool reusePlayerAnchor = false);
    bool Select_ValtanEffect(const std::string& assetId,
        const std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>& clips);
    bool Preview(const EFFECT_RESOURCE_KEY& key, std::uint32_t durationMs = 3000u);
    bool Preview_Element(const EFFECT_RESOURCE_KEY& key, const std::string& elementId,
        const std::string& label, std::uint32_t durationMs, std::uint32_t focusMs);
    bool Preview_Elements(const EFFECT_RESOURCE_KEY& key, const std::vector<std::string>& elementIds,
        const std::string& label, std::uint32_t durationMs, std::uint32_t focusMs, bool loop);
    bool Append(const EFFECT_RESOURCE_KEY& key, std::uint32_t durationMs = 3000u, bool screenPost = false);
    bool Play(bool paused = false);
    void Preserve_ClockDuringAuthoring() { if (m_Active) m_SkipNextPlaybackDelta = true; }
    bool Uses_Resource(const EFFECT_RESOURCE_KEY& key) const;
    bool Refresh_Effects(const EFFECT_RESOURCE_KEY* key = nullptr,
        const std::vector<std::string>* availableElementIds = nullptr);
    bool Set_BloomIntensity(const EFFECT_RESOURCE_KEY& key, float value, std::string& error);
    bool Seek(std::uint32_t clockMs);
    void Pause(bool paused);
    void Stop();
    bool Is_Active() const { return m_Active; }
    bool Is_ElementPreview() const { return m_Transient && !m_Transient->previewElementIds.empty(); }
    bool Owns_ModelClock() const { return m_Active && Has_ModelSequence(); }
    bool Is_Paused() const { return m_Paused; }
    bool Is_ValtanSourcePreview(const std::string& assetId) const
    { return m_ValtanEffectPreview && m_ValtanEffectPreview->assetId == assetId; }
    bool Consume_InteractionRequest();
    const std::string& Status() const { return m_Status; }
    std::uint32_t ClockMs() const { return static_cast<std::uint32_t>(m_ClockMs); }

private:
    enum class TRACK_KIND { ANIMATION, EFFECT, COLLIDER, SOUND, CAMERA, SCREEN_POST };
    struct CLIP final
    {
        std::string id, label, memberId, clipName;
        std::uint32_t startMs = 0u, durationMs = 0u, sourceStartMs = 0u, sourcePlayMs = 0u;
        float playRate = 1.f;
        bool loop = false;
        bool muted = false;
    };
    struct MODEL_SEQUENCE final
    {
        std::string id, label, error;
        std::vector<CLIP> clips;
        std::uint32_t durationMs = 0u;
        std::uint32_t skillId = 0u; // Read-only Product owner for preview stance.
    };
    struct V1_ANCHOR_HISTORY final
    {
        std::unordered_map<std::string, EFFECT_V2_PIVOT_HISTORY> slots;
        float recordedAge = -1.f;
    };
    struct EFFECT_ROW final
    {
        std::string id;
        EFFECT_RESOURCE_KEY key;
        // Transient document projection only; never a saved sequence occurrence.
        std::vector<std::string> previewElementIds;
        std::string previewElementLabel;
        std::uint32_t previewStartMs = 0u;
        bool previewLoop = false;
        std::string anchorSlotId = "root";
        std::uint32_t startMs = 0u, durationMs = 3000u;
        float3_t offset{}, rotation{};
        float3_t scale{1.f, 1.f, 1.f};
        bool worldAnchor = false;
        bool muted = false;
        bool screenPost = false;
        // Product cue projection only; these values are owned by animevents.
        bool productNaturalDuration = false, productSnapshot = false, productActionFacing = false;
        bool productExternalOwner = false; // Vehicle lifetime projection; saved sequence never rewrites a skill cue owner.
        std::optional<std::uint32_t> productStopDuration;
        std::optional<float4x4_t> productSpawnPivot, productSpawnOwner;
        float productFacingDegrees = 0.f;
        std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
        std::shared_ptr<CEffectObject> v1;
        // Unsaved document scalar survives delayed starts and Play/Stop restaging.
        std::optional<float> bloomIntensityOverride;
        std::uint32_t v2 = 0u;
        std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> history;
        std::shared_ptr<V1_ANCHOR_HISTORY> anchorHistory;
        float recordedAge = -1.f;
        float sampledAge = -1.f;
        float finiteLoopSeconds = 0.f, sampledCycleStart = -1.f;
        float4x4_t recordedRoot{};
    };
    using CAMERA_ROW = EFFECT_CAMERA_ROW;
    struct SOUND_ROW final
    {
        std::string id, label, assetId;
        std::uint32_t startMs = 0u, durationMs = 3000u, sourceStartMs = 0u;
        float volume = 1.f;
        bool muted = false;
        std::uint64_t handle = 0u;
        std::int64_t sampledAge = -1;
    };
    struct COLLIDER_ROW final
    {
        std::string id, label;
        KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE resource;
        std::uint32_t startMs = 0u, durationMs = 1000u;
        float3_t offset{}, rotation{}, scale{1.f, 1.f, 1.f};
        std::string anchorSlotId = "root";
        bool muted = false, debugRender = true;
        std::optional<HIT_AREA_SHAPE> productShape;
    };
    struct ROW_TRANSFER final : COMPOSITION_EFFECT_TRANSFER
    {
        TRACK_KIND kind = TRACK_KIND::EFFECT;
        std::string asset;
        CLIP animation;
        EFFECT_ROW effect;
        SOUND_ROW sound;
        COLLIDER_ROW collider;
        CAMERA_ROW camera;
        std::string_view Type() const noexcept override { return "character.occurrence.v1"; }
    };
    static EFFECT_ROW Authoring_EffectRow(const EFFECT_ROW& source);
    COMPOSITION_TRANSFER Capture_CompositionSelection(std::string& status);
    bool Insert_EffectRows(std::vector<EFFECT_ROW> rows, std::string& status);
    bool Has_PendingBoxEdit() const;
    struct RESOURCE_ENTRY final
    {
        TRACK_KIND kind = TRACK_KIND::EFFECT;
        std::string id, label, category, status;
        EFFECT_RESOURCE_KEY key;
        std::uint32_t durationMs = 3000u;
        KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE collider;
    };
    bool Validate_AnimationRows(const std::vector<CLIP>& rows);
    bool Validate_SoundRows(const std::vector<SOUND_ROW>& rows);
    bool Validate_ColliderRows(const std::vector<COLLIDER_ROW>& rows);
    bool Parse_AdditionalRows(const DATA_JSON_VALUE& document, std::uint32_t version,
        std::vector<CLIP>& animations, bool& customAnimation,
        std::vector<SOUND_ROW>& sounds, std::vector<COLLIDER_ROW>& colliders);
    void Write_AdditionalRows(std::ostream& out) const;
    bool Append_Animation(const std::string& clipName, bool atEnd = false);
    bool Refresh_AnimationTiming();
    bool Commit_TransientPreview();
    bool Append_Sound(const std::string& assetId, std::uint32_t durationMs);
    bool Append_Collider(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource);
    bool Sample_Sounds(bool forceSeek = false);
    void Stop_Sounds();
    void Render_Colliders();
    void Render_CompositionResources(bool embedded = false);
    void Render_BoxDetail(bool embedded = false);
    bool Refresh_CompositionResourceInventory();
    void Rebuild_CompositionResourceTrees();
    void Select_TimelineRow(TRACK_KIND kind, const std::string& id);
    bool Apply_AnimationRow(const CLIP& row);
    bool Apply_EffectRow(const EFFECT_ROW& row);
    bool Apply_SoundRow(const SOUND_ROW& row);
    bool Apply_ColliderRow(const COLLIDER_ROW& row);
    bool Remove_SelectedRow();
    bool Duplicate_SelectedRow();
    bool Validate_CameraRows(const std::vector<CAMERA_ROW>& rows);
    bool Parse_CameraRows(const DATA_JSON_VALUE& document, bool required, std::vector<CAMERA_ROW>& rows);
    void Write_CameraRows(std::ostream& out) const;
    bool Read_RecoveryCameras(const EFFECT_RESOURCE_KEY& key, std::vector<CAMERA_ROW>& rows,
        std::vector<CLIP>& animations);
    bool Sample_Camera(std::uint32_t clockMs, const float4x4_t& root,
        const std::vector<CAMERA_ROW>* overrideRows = nullptr);
    bool Sort_CameraKeys(CAMERA_ROW& row);
    bool Capture_CameraKey(CAMERA_ROW& row, std::uint32_t time);
    void Release_Camera();
    void Render_CameraEditor();
    void Draw_CameraRows(float labels, float rowHeight, float width);
    bool Reload_ModelSequences();
    bool Select_ModelSequence(const std::string& id);
    bool Select_Kouku(const std::string& id, bool bundle);
    bool Select_SceneEffectTarget(const std::string& assetId, bool requiresSourceModel,
        bool reusePlayerAnchor, std::optional<bool> loopPolicy,
        std::optional<std::uint32_t> previewDurationMs = std::nullopt, const EFFECT_DOCUMENT_DESC* sourceDocument = nullptr,
        std::uint32_t modelStartMs = 0u, bool loopEffectToDuration = false);
    bool Uses_TransientLoop() const
    { return m_Transient && (Is_ElementPreview() || (m_KoukuEffectPreview && m_KoukuEffectPreview->loopPolicy.has_value())); }
    bool Uses_KoukuSourceModel() const
    { return !m_ValtanEffectPreview && (m_KoukuEffectPreview ? m_KoukuEffectPreview->model.has_value() : m_UseKouku); }
    bool Has_ModelSequence() const
    { return m_ValtanEffectPreview || !m_TransientAnimationRows.empty() || (m_KoukuEffectPreview ? m_KoukuEffectPreview->model.has_value() : (m_UseKouku || m_CustomAnimation || !m_SelectedSequence.empty())); }
    bool Begin_Model();
    bool Sample_Model(std::uint32_t clockMs);
    bool Resolve_Root(float4x4_t& root);
    bool Resolve_ScenePreviewPlacement(float4x4_t& root);
    bool Set_WorldPreviewPlacement(const float3_t& position, float yawDegrees);
    bool Resolve_RowPivot(const EFFECT_ROW& row, const float4x4_t& root, float4x4_t& pivot);
    bool Record_RowPivot(EFFECT_ROW& row, const float4x4_t& pivot, float age);
    bool Render_AnchorChoice(const char* label, std::string& anchor);
    bool Validate_EffectPlacement(const EFFECT_ROW& row);
    bool Validate_Anchor(const std::string& anchor, bool modelRoot, bool useKouku);
    bool Stage_Row(EFFECT_ROW& row, const float4x4_t& root);
    bool Sample_Row(EFFECT_ROW& row, const float4x4_t& root);
    bool Record_V1Anchors(EFFECT_ROW& row, const float4x4_t& pivot, float age);
    void Release_Row(EFFECT_ROW& row);
    bool Sample(bool forceSeekSounds = false);
    bool Apply_CharacterPreviewStance(std::uint32_t skillId);
    void Restore_CharacterPreviewStance();
    std::uint32_t DurationMs() const;
    const MODEL_SEQUENCE* Selected_Sequence() const;
    void Draw_KoukuInventory();
    bool Save_Sequence();
    bool Load_Sequence(bool discard = false);

    ComPtr<ID3D11Device> m_Device;
    std::function<bool()> m_WorkbenchSave;
    ComPtr<ID3D11DeviceContext> m_Context;
    std::shared_ptr<CCharacterPreviewPanel> m_Panel;
    CEffectCompositionModelPreview m_Kouku;
    struct KOUKU_EFFECT_PREVIEW_TARGET final
    {
        std::string assetId;
        std::string worldContextPatternId; // Preview-only saved hand props.
        std::string worldOccurrenceId; // Empty uses source bones; otherwise the exact gun supplies the root.
        std::uint64_t propReferenceRevision = 0u;
        float4x4_t playerRoot{};
        std::uint64_t placementRevision = 0u;
        std::optional<CEffectCompositionModelPreview> model;
        // Only the independent preview consumes this; saved sequence Loop is unchanged.
        std::optional<bool> loopPolicy;
        std::optional<std::uint32_t> previewDurationMs;
        std::uint32_t modelStartMs = 0u;
        bool loopEffectToDuration = false;
        std::optional<EFFECT_SOURCE_MODEL_PREVIEW> sourceModelPreview;
    };
    // Tool-only target; the saved arrangement and its dirty state never change.
    std::optional<KOUKU_EFFECT_PREVIEW_TARGET> m_KoukuEffectPreview, m_PendingKoukuEffectPreview;
    CKoukuSaydonPresentationPlayer* m_Player = nullptr;
    V1_FACTORY m_V1Factory;
    V1_RELEASE m_V1Release;
    V1_ANCHOR_PROVIDER m_V1Anchors;
    V2_SNAPSHOT_PROVIDER m_V2Provider;
    std::vector<MODEL_SEQUENCE> m_Sequences;
    std::vector<CLIP> m_AnimationRows;
    // Recovery Preview owns its clip-local animation without changing the saved arrangement.
    std::vector<CLIP> m_TransientAnimationRows;
    struct VALTAN_EFFECT_PREVIEW_TARGET final
    {
        std::string assetId;
        std::vector<CLIP> clips;
        std::uint64_t modelGeneration = 0u;
    };
    // Source animation belongs to this transient Effect, never the saved arrangement.
    std::optional<VALTAN_EFFECT_PREVIEW_TARGET> m_ValtanEffectPreview;
    bool m_CustomAnimation = false;
    std::vector<SOUND_ROW> m_Sounds;
    std::vector<COLLIDER_ROW> m_Colliders;
    std::vector<EFFECT_ROW> m_Effects;
    std::optional<EFFECT_ROW> m_Transient;
    std::weak_ptr<Engine::CCamera> m_Camera;
    std::vector<CAMERA_ROW> m_CameraRows, m_TransientCameraRows;
    std::string m_SelectedCamera;
    bool m_CameraOwned = false;
    std::string m_AssetName, m_SelectedSequence, m_AnchorMember, m_SelectedEffect, m_Status;
    std::weak_ptr<CCharacter> m_StancePreviewCharacter;
    LostArk::Shared::PLAYER_STANCE_ID m_PreviousPreviewStance = LostArk::Shared::PLAYER_STANCE_ID::NONE;
    std::uint32_t m_CharacterActionSkillId = 0u;
    std::weak_ptr<Engine::CModel> m_Model;
    std::uint64_t m_ModelGeneration = 0u, m_InventoryGeneration = 0u;
    std::uint32_t m_PreviousClip = 0u, m_NextEffectOrdinal = 1u;
    float m_PreviousPosition = 0.f;
    bool m_PreviousPaused = true, m_PreviousLoop = false;
    std::string m_SourceModelEffectId;
    std::string m_ExplicitKoukuPatternId;
    bool m_PreviewSavedHandProps = false; // Model View reference only; not saved in a sequence.
    std::vector<EFFECT_COMPOSITION_MODEL_PROP> m_PreviewPropChoices;
    std::string m_PreviewPropPatternId, m_PreviewWorldOccurrenceId;
    std::uint64_t m_PreviewPropReferenceRevision = 0u;
    float4x4_t m_WorldRoot{};
    // Explicit Play All placement is session state; Append stores it in its own occurrence.
    std::optional<float4x4_t> m_ScenePreviewWorldRoot;
    std::uint32_t m_ScenePreviewWorldLevel = UINT32_MAX, m_PreviewPlacementPickLevel = UINT32_MAX;
    std::uint64_t m_PreviewPlacementRevision = 0u;
    bool m_PreviewPlacementPickPending = false, m_PreviewPlacementLeftDown = false;
    bool m_PreviewPlacementSuppressMouse = false;
    double m_ClockMs = 0.0;
    float m_Zoom = 80.f;
    bool m_Active = false, m_Paused = false, m_Loop = false, m_UseKouku = false;
    bool m_Interaction = false;
    bool m_SkipNextPlaybackDelta = false;
    bool m_ModelRoot = true;
    std::string m_DefaultAnchorSlotId = "root";
    std::string m_DragEffect;
    float m_DragMouseX = 0.f;
    std::uint32_t m_DragStartMs = 0u, m_DragDurationMs = 0u;
    int m_DragKind = 0;
    char m_SequenceId[128] = "effect.sequence.default";
    std::string m_SequenceBaseline;
    std::string m_PersistedSequenceId;
    bool m_SequenceExisted = false, m_Dirty = false;
    TRACK_KIND m_SelectedTrack = TRACK_KIND::EFFECT;
    std::string m_SelectedRowId;
    std::array<COMPOSITION_RESOURCE_TREE_NODE, 6> m_ResourceTrees;
    std::array<std::string, 6> m_ResourceQueries, m_SelectedResourceIds;
    std::array<std::array<char, 160>, 6> m_ResourceSearch{};
    std::vector<RESOURCE_ENTRY> m_CompositionResources;
    int m_ResourceTab = 0;
    bool m_ResourcesLoaded = false, m_ResourcesOpen = true, m_BoxDetailOpen = true;
    std::uint64_t m_ResourceModelGeneration = ~std::uint64_t{0u};
    struct BOX_DETAIL_DRAFT;
    std::shared_ptr<BOX_DETAIL_DRAFT> m_BoxDetailDraft;
    std::string m_ResourceStatus;
    std::string m_DragRowId;
    TRACK_KIND m_DragTrack = TRACK_KIND::EFFECT;
    bool m_DragWasPaused = true;
    std::uint32_t m_DragPreviewStartMs = 0u, m_DragPreviewDurationMs = 0u;
};
}
