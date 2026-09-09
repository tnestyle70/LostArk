#pragma once
#include "EffectRecoveryCamera.h"

#include "AnimationTargetService.h"
#include "CharacterPreviewPanel.h"
#include "EffectCompositionModelPreview.h"
#include "ValtanCinematicCameraDocument.h"
#include "CompositionResourceTree.h"
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

// One editor clock. The resource owners retain their codecs and prepare each
// independent occurrence; this adapter owns only its lifetime and sampling.
class CEffectAuthoringSequencer final
{
public:
    using V1_FACTORY = std::function<bool(const EFFECT_RESOURCE_KEY&, const std::string&, const float4x4_t&,
        std::shared_ptr<CEffectObject>&, std::string&)>;
    using V1_RELEASE = std::function<void(const std::shared_ptr<CEffectObject>&)>;
    using V1_ANCHOR_PROVIDER = std::function<bool(const std::shared_ptr<CEffectObject>&,
        const float4x4_t&, bool, std::unordered_map<std::string, float4x4_t>&, std::string&)>;
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
    void Set_V2SnapshotProvider(V2_SNAPSHOT_PROVIDER provider);
    void Render_ModelView(); // Contents inside the existing Model View window.
    void Render_Sequencer(const char* title = "Sequencer##EffectAuthoring"); // The existing Effect Tool calls this panel.
    void Update(float dt, bool active);
    bool Select_CharacterSkill(const std::string& asset, std::uint32_t skillId,
        std::optional<std::uint32_t> stageIndex = std::nullopt);
    bool Preview(const EFFECT_RESOURCE_KEY& key, std::uint32_t durationMs = 3000u);
    bool Preview_Element(const EFFECT_RESOURCE_KEY& key, const std::string& elementId,
        const std::string& label, std::uint32_t durationMs, std::uint32_t focusMs);
    bool Append(const EFFECT_RESOURCE_KEY& key, std::uint32_t durationMs = 3000u, bool screenPost = false);
    bool Play(bool paused = false);
    void Preserve_ClockDuringAuthoring() { if (m_Active) m_SkipNextPlaybackDelta = true; }
    bool Uses_Resource(const EFFECT_RESOURCE_KEY& key) const;
    bool Refresh_Effects(const EFFECT_RESOURCE_KEY* key = nullptr);
    bool Seek(std::uint32_t clockMs);
    void Pause(bool paused);
    void Stop();
    bool Is_Active() const { return m_Active; }
    bool Is_ElementPreview() const { return m_Transient && !m_Transient->previewElementId.empty(); }
    bool Owns_ModelClock() const { return m_Active && (m_UseKouku || m_CustomAnimation || !m_SelectedSequence.empty()); }
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
        std::string previewElementId, previewElementLabel;
        std::string anchorSlotId = "root";
        std::uint32_t startMs = 0u, durationMs = 3000u;
        float3_t offset{};
        bool muted = false;
        bool screenPost = false;
        std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
        std::shared_ptr<CEffectObject> v1;
        std::uint32_t v2 = 0u;
        std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> history;
        std::shared_ptr<V1_ANCHOR_HISTORY> anchorHistory;
        float recordedAge = -1.f;
        float sampledAge = -1.f;
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
    };
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
    bool Append_Animation(const std::string& clipName);
    bool Refresh_AnimationTiming();
    bool Commit_TransientPreview();
    bool Append_Sound(const std::string& assetId, std::uint32_t durationMs);
    bool Append_Collider(const KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource);
    bool Sample_Sounds(bool forceSeek = false);
    void Stop_Sounds();
    void Render_Colliders();
    void Render_CompositionResources();
    void Render_BoxDetail();
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
    bool Begin_Model();
    bool Sample_Model(std::uint32_t clockMs);
    bool Resolve_Root(float4x4_t& root);
    bool Resolve_RowPivot(const EFFECT_ROW& row, const float4x4_t& root, float4x4_t& pivot);
    bool Record_RowPivot(EFFECT_ROW& row, const float4x4_t& pivot, float age);
    bool Render_AnchorChoice(const char* label, std::string& anchor);
    bool Validate_Anchor(const std::string& anchor, bool modelRoot, bool useKouku);
    bool Stage_Row(EFFECT_ROW& row, const float4x4_t& root);
    bool Sample_Row(EFFECT_ROW& row, const float4x4_t& root);
    bool Record_V1Anchors(EFFECT_ROW& row, const float4x4_t& pivot, float age);
    void Release_Row(EFFECT_ROW& row);
    bool Sample(bool forceSeekSounds = false);
    std::uint32_t DurationMs() const;
    const MODEL_SEQUENCE* Selected_Sequence() const;
    void Draw_KoukuInventory();
    bool Save_Sequence();
    bool Load_Sequence(bool discard = false);

    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_Context;
    std::shared_ptr<CCharacterPreviewPanel> m_Panel;
    CEffectCompositionModelPreview m_Kouku;
    CKoukuSaydonPresentationPlayer* m_Player = nullptr;
    V1_FACTORY m_V1Factory;
    V1_RELEASE m_V1Release;
    V1_ANCHOR_PROVIDER m_V1Anchors;
    V2_SNAPSHOT_PROVIDER m_V2Provider;
    std::vector<MODEL_SEQUENCE> m_Sequences;
    std::vector<CLIP> m_AnimationRows;
    // Recovery Preview owns its clip-local animation without changing the saved arrangement.
    std::vector<CLIP> m_TransientAnimationRows;
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
    std::weak_ptr<Engine::CModel> m_Model;
    std::uint64_t m_ModelGeneration = 0u, m_InventoryGeneration = 0u;
    std::uint32_t m_PreviousClip = 0u, m_NextEffectOrdinal = 1u;
    float m_PreviousPosition = 0.f;
    bool m_PreviousPaused = true, m_PreviousLoop = false;
    float4x4_t m_WorldRoot{};
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
