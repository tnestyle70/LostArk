#pragma once

#include "AnimationTargetService.h"
#include "CharacterPreviewPanel.h"
#include "EffectCompositionModelPreview.h"
#include "ValtanCinematicCameraDocument.h"
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
    using V1_FACTORY = std::function<bool(const EFFECT_RESOURCE_KEY&, const float4x4_t&,
        std::shared_ptr<CEffectObject>&, std::string&)>;
    using V1_RELEASE = std::function<void(const std::shared_ptr<CEffectObject>&)>;
    using V1_ANCHOR_PROVIDER = std::function<bool(const std::shared_ptr<CEffectObject>&,
        const float4x4_t&, bool, std::unordered_map<std::string, float4x4_t>&, std::string&)>;
    using V2_SNAPSHOT_PROVIDER = std::function<bool(const EFFECT_RESOURCE_KEY&,
        std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>&, std::string&)>;

    CEffectAuthoringSequencer(ComPtr<ID3D11Device> device,
        ComPtr<ID3D11DeviceContext> context, std::shared_ptr<CCharacterPreviewPanel> panel);
    ~CEffectAuthoringSequencer();
    void Set_Player(CKoukuSaydonPresentationPlayer* player);
    void Set_Camera(const std::shared_ptr<Engine::CCamera>& camera);
    void Set_V1Callbacks(V1_FACTORY factory, V1_RELEASE release);
    void Set_V1AnchorProvider(V1_ANCHOR_PROVIDER provider);
    void Set_V2SnapshotProvider(V2_SNAPSHOT_PROVIDER provider);
    void Render_ModelView(); // Contents inside the existing Model View window.
    void Render_Sequencer(); // The existing Effect Tool calls this panel.
    void Update(float dt, bool active);
    bool Select_CharacterSkill(const std::string& asset, std::uint32_t skillId,
        std::optional<std::uint32_t> stageIndex = std::nullopt);
    bool Preview(const EFFECT_RESOURCE_KEY& key, std::uint32_t durationMs = 3000u);
    bool Append(const EFFECT_RESOURCE_KEY& key, std::uint32_t durationMs = 3000u);
    bool Play();
    void Preserve_ClockDuringAuthoring() { if (m_Active) m_SkipNextPlaybackDelta = true; }
    bool Uses_Resource(const EFFECT_RESOURCE_KEY& key) const;
    bool Refresh_Effects(const EFFECT_RESOURCE_KEY* key = nullptr);
    bool Seek(std::uint32_t clockMs);
    void Pause(bool paused);
    void Stop();
    bool Is_Active() const { return m_Active; }
    bool Owns_ModelClock() const { return m_Active && (m_UseKouku || !m_SelectedSequence.empty()); }
    bool Consume_InteractionRequest();
    const std::string& Status() const { return m_Status; }
    std::uint32_t ClockMs() const { return static_cast<std::uint32_t>(m_ClockMs); }

private:
    struct CLIP final
    {
        std::string id, label, memberId, clipName;
        std::uint32_t startMs = 0u, durationMs = 0u, sourceStartMs = 0u, sourcePlayMs = 0u;
        float playRate = 1.f;
        bool loop = false;
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
        std::string anchorSlotId = "root";
        std::uint32_t startMs = 0u, durationMs = 3000u;
        float3_t offset{};
        bool muted = false;
        std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
        std::shared_ptr<CEffectObject> v1;
        std::uint32_t v2 = 0u;
        std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> history;
        std::shared_ptr<V1_ANCHOR_HISTORY> anchorHistory;
        float recordedAge = -1.f;
        float sampledAge = -1.f;
        float4x4_t recordedRoot{};
    };
    struct CAMERA_ROW final
    {
        std::string id, label, source = "PROJECT_TUNED";
        std::uint32_t startMs = 0u;
        bool muted = false, modelRelative = true, horizontalFov = false;
        VALTAN_CINEMATIC_CAMERA_CUE cue;
        std::vector<float3_t> upVectors;
    };
    bool Validate_CameraRows(const std::vector<CAMERA_ROW>& rows);
    bool Parse_CameraRows(const DATA_JSON_VALUE& document, bool required, std::vector<CAMERA_ROW>& rows);
    void Write_CameraRows(std::ostream& out) const;
    bool Read_RecoveryCameras(const EFFECT_RESOURCE_KEY& key, std::vector<CAMERA_ROW>& rows);
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
    bool Sample();
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
};
}
