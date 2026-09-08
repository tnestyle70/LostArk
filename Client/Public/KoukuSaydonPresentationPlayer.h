#pragma once

#include "Client_Defines.h"
#include "KoukuSaydonCompositionDocument.h"
#include "Network/PacketMessages.h"
#include "HitAreaWire.h"
#include <array>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace Engine { class CModel; }
namespace Client
{
class CNpc;
class CWorldSequencePlayer;
class CWorldSequenceDocument;
class CCharacter;
class CRenderingProfileService;
class CLightResourceCatalog;
class EFFECT_V2_CATALOG_SNAPSHOT;
class EFFECT_V2_PIVOT_HISTORY;
struct EFFECT_V2_TARGET;
struct EFFECT_V2_TARGET_VIEW;
struct ANIMATION_MODEL_TARGET_VIEW;

struct KOUKU_BOSS_PRESENTATION_VIEW final
{
    std::weak_ptr<CNpc> pNpc;
    LostArk::Shared::WORLD_ENTITY_SNAPSHOT Snapshot;
    std::uint32_t iServerTick = 0;
    LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
    std::string strArchetypeId;
};
struct KOUKU_CARD_PRESENTATION_VIEW final
{
    std::weak_ptr<CCharacter> pCharacter;
    LostArk::Shared::PLAYER_SNAPSHOT Snapshot;
};
struct KOUKU_MAZE_TARGET_VIEW final
{
    std::weak_ptr<CNpc> npc;
    std::uint32_t entityId = 0u;
    std::string archetypeId;
};

// The Server supplies identity and time. This owner only samples presentation
// resources and releases its own effects, audio and temporary scene/camera state.
class CKoukuSaydonPresentationPlayer final
{
public:
    CKoukuSaydonPresentationPlayer(ComPtr<ID3D11Device> device,
        ComPtr<ID3D11DeviceContext> context, CRenderingProfileService& profiles);
    ~CKoukuSaydonPresentationPlayer();
    bool Reload_Product(std::string& status);
    using WORLD_EMISSION_ANCHOR = std::function<bool_t(f32_t, float4x4_t&)>;
    static WORLD_EMISSION_ANCHOR Make_WorldEmissionAnchor(
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
        const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world,
        const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& occurrence);
    bool Resolve_ProductWorldEmissionAnchor(std::uint32_t sourceRevision, std::string_view patternId,
        std::string_view occurrenceId, WORLD_EMISSION_ANCHOR& out) const;
    void Set_LightResources(const CLightResourceCatalog* catalog) { m_pLightResources = catalog; }
    std::size_t Light_SkippedByBudget() const;
    void Update(float dt, const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
        const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players);
    bool Begin_Preview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, bool ownClock,
        std::uint32_t clockMs, bool paused, std::string& status);
    bool Begin_BundlePreview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const std::string& bundleId, std::uint32_t clockMs, bool paused, std::string& status,
        const CWorldSequenceDocument* sourceDocument = nullptr);
    // Optional Effect Workbench reference: existing actors and model sampler,
    // with all Pattern presentation/WORLD disabled and an external master clock.
    bool Begin_ModelReferencePreview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const std::string& selectionId, bool bundle, std::uint32_t clockMs, bool paused, std::string& status);
    void Sample_ModelReferencePreview(std::uint32_t clockMs, bool paused);
    bool Resolve_ModelReferenceTarget(const std::string& memberId,
        EFFECT_V2_TARGET& target, EFFECT_V2_TARGET_VIEW& view) const;
    bool Preview_IsModelReference() const { return m_bModelReferencePreview; }
    std::uint64_t Preview_Generation() const { return m_iPreviewGeneration; }
    bool Preview_IsBundle() const { return !m_PreviewBundleId.empty(); }
    void Sample_Preview(std::uint32_t clockMs, bool playing, bool paused,
        const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model);
    void Set_PreviewPivot(const float4x4_t& pivot,
        const std::shared_ptr<Engine::CModel>& model);
    void Pause_Preview(bool paused);
    void Seek_Preview(std::uint32_t clockMs);
    void Stop_Preview();
    void Reset();
    // Called after ImGui NewFrame; sampling never draws from a loader/update thread.
    void Render_Debug() const;
    void Refresh_ColliderAuthoring(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, std::uint64_t generation);
    // Only the selected Collider/Effect placement changes; clocks and unrelated cues remain live.
    bool Preview_PresentationGeometry(const std::string& patternId,
        const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence);
    bool Preview_OwnsClock() const { return m_bOwnPreviewClock; }
    bool Preview_IsColliderResource() const { return m_bColliderResourcePreview; }
    bool Preview_Playing() const { return m_bPreviewPlaying; }
    bool Preview_HasActiveWorldBox(std::string_view occurrenceId) const;
    bool Preview_Paused() const { return m_bPreviewPaused; }
    std::uint32_t Preview_ClockMs() const { return static_cast<std::uint32_t>(m_fPreviewClockMs); }
    std::uint32_t Preview_DurationMs() const { return m_iPreviewDurationMs; }
    const std::string& Preview_PatternId() const { return m_PreviewPattern.strPatternId; }
    const std::string& Status() const { return m_strStatus; }
private:
    struct PLAYING_ROW final
    {
        KOUKU_SAYDON_PRESENTATION_KIND kind = KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
        std::uint32_t effectHandle = 0;
        std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> effectPivotHistory;
        float4x4_t effectLastPivot{};
        float effectRecordedSeconds = -1.f;
        std::uint64_t soundHandle = 0;
        float lastAge = -1.f;
        float startMs = 0.f;
        std::uint32_t cameraDurationMs = 0u;
        float4x4_t pivot{};
        std::string assetId;
        float3_t cameraOffset{};
        HIT_AREA_SHAPE wire{};
        float4x4_t placementAnchor{};
        std::array<double, 3u> placementAnchorScale{1.0, 1.0, 1.0};
        bool hasPlacementAnchor = false;
        KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE lightBox;
        float lightWeight = 1.f;
        bool failed = false;
        bool waitingForAnchor = false;
        bool debugRender = true;
    };
    struct SESSION final
    {
        std::string key;
        float lastClockMs = -1.f;
        std::map<std::string, PLAYING_ROW> rows;
        std::uint32_t runEpoch = 0;
        std::string memberId;
        std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> rootHistory;
        float rootRecordedSeconds = -1.f;
        float4x4_t rootRecordedPivot{};
        std::map<std::string, std::shared_ptr<CWorldSequencePlayer>> previewWorlds;
    };
    struct PRODUCT_PATTERN final
    {
        KOUKU_SAYDON_COMPOSITION_DOCUMENT document;
        KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
        std::uint32_t durationMs = 0;
        std::map<std::string, WORLD_EMISSION_ANCHOR> worldEmissionAnchors;
    };
    struct PRODUCT_BUNDLE final
    {
        PRODUCT_PATTERN common;
        std::vector<std::string> patternIds;
    };
    struct BUNDLE_PREVIEW_MEMBER final
    {
        std::string memberId;
        std::uint32_t offsetTicks = 0, durationMs = 0;
        KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
        std::shared_ptr<CNpc> actor;
        SESSION session;
        std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE> animations;
        std::map<std::string, float3_t> worldOffsets;
        std::uint32_t initialAnimation = 0;
        float initialTicks = 0.f;
        float initialYawDegrees = 0.f;
        std::map<std::string, float> stageFacingYawDegrees;
    };
    void Sample_BundlePreview();
    void Sample_BundlePreviewFacing(BUNDLE_PREVIEW_MEMBER& member, double localMs);
    void Refresh_WorldPlacementAuthoring(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document);
    void Release_BundlePreviewMembers(std::vector<BUNDLE_PREVIEW_MEMBER>& members);
    struct CARD final { std::string assetId; std::uint32_t handle = 0; };
    void Sync_MazeMark(CARD& mark, const std::string& asset, const float4x4_t& pivot);
    void Update_MazeMarks(const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players);
    void Sample(SESSION& session, const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, float clockMs, bool paused,
        const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model,
        const ANIMATION_MODEL_TARGET_VIEW* weaponView = nullptr);
    void Stop_Session(SESSION& session);
    bool Ensure_EffectResource(const std::string& kind, const std::string& asset,
        std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>& snapshot);
    void Restore_Scene();
    void Refresh_SharedPresentation();
    void Collect_FrameLights();
    struct FRAME_LIGHT_PROVIDER;
    std::shared_ptr<FRAME_LIGHT_PROVIDER> m_LightProvider;
    const CLightResourceCatalog* m_pLightResources = nullptr;
    std::vector<float4x4_t> m_LightPlayerPivots;
    struct LIGHT_BOSS_FOLLOWER final
    {
        std::uint32_t entityId = 0u;
        std::weak_ptr<CNpc> npc;
    };
    std::map<std::uint32_t, std::vector<LIGHT_BOSS_FOLLOWER>> m_LightBossFollowers;
    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_Context;
    CRenderingProfileService& m_Profiles;
    std::map<std::string, std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>> m_EffectResources;
    std::map<std::string, std::string> m_EffectResourceFailures;
    std::uint64_t m_iEffectCacheGeneration = 0u;
    std::map<std::string, PRODUCT_PATTERN> m_Product;
    std::map<std::string, PRODUCT_BUNDLE> m_ProductBundles;
    SESSION m_ProductBundleSession;
    std::uint32_t m_iProductSourceRevision = 0u;
    std::uint32_t m_iProductReloadRunEpoch = 0u;
    std::set<std::string> m_MissingProductPatterns;
    std::map<std::uint32_t, SESSION> m_BossSessions;
    std::map<std::uint32_t, CARD> m_Cards;
    std::map<std::uint32_t, CARD> m_MazeExits;
    std::map<std::uint32_t, CARD> m_MazePlayerMarks;
    std::map<std::uint32_t, CARD> m_MazeTargetMarks;
    std::map<std::string, bool> m_ColliderDebugOverrides;
    std::uint64_t m_iColliderAuthoringGeneration = UINT64_MAX;
    bool m_bProductLoaded = false, m_bProductAttempted = false;
    std::string m_strScenePrevious, m_strSceneOwner, m_strSceneApplied;
    bool m_bSceneUsed = false, m_bCameraUsed = false;
    SESSION m_PreviewSession;
    std::string m_PreviewBundleId;
    std::vector<BUNDLE_PREVIEW_MEMBER> m_BundlePreviewMembers;
    KOUKU_SAYDON_COMPOSITION_DOCUMENT m_PreviewDocument;
    KOUKU_SAYDON_COMPOSITION_PATTERN m_PreviewPattern;
    float4x4_t m_PreviewPivot{};
    std::weak_ptr<Engine::CModel> m_PreviewModel;
    bool m_bOwnPreviewClock = false, m_bPreviewPlaying = false, m_bPreviewPaused = false;
    bool m_bPreviewPivotReady = false;
    bool m_bColliderResourcePreview = false;
    bool m_bModelReferencePreview = false;
    std::uint64_t m_iPreviewGeneration = 0u;
    double m_fPreviewClockMs = 0;
    std::uint32_t m_iPreviewDurationMs = 0;
    std::string m_strStatus;
};
}
