#pragma once

#include "Client_Defines.h"
#include "KoukuSaydonCompositionDocument.h"
#include "Network/PacketMessages.h"
#include "HitAreaWire.h"
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace Engine { class CModel; }
namespace Client
{
class CNpc;
class CCharacter;
class CRenderingProfileService;
class CLightResourceCatalog;
class EFFECT_V2_CATALOG_SNAPSHOT;

struct KOUKU_BOSS_PRESENTATION_VIEW final
{
    std::weak_ptr<CNpc> pNpc;
    LostArk::Shared::WORLD_ENTITY_SNAPSHOT Snapshot;
    std::uint32_t iServerTick = 0;
};
struct KOUKU_CARD_PRESENTATION_VIEW final
{
    std::weak_ptr<CCharacter> pCharacter;
    LostArk::Shared::PLAYER_SNAPSHOT Snapshot;
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
    void Set_LightResources(const CLightResourceCatalog* catalog) { m_pLightResources = catalog; }
    std::size_t Light_SkippedByBudget() const;
    void Update(float dt, const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
        const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players);
    bool Begin_Preview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, bool ownClock,
        std::uint32_t clockMs, bool paused, std::string& status);
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
    bool Preview_OwnsClock() const { return m_bOwnPreviewClock; }
    bool Preview_IsColliderResource() const { return m_bColliderResourcePreview; }
    bool Preview_Playing() const { return m_bPreviewPlaying; }
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
        std::uint64_t soundHandle = 0;
        float lastAge = -1.f;
        float startMs = 0.f;
        float4x4_t pivot{};
        std::string assetId;
        float3_t cameraOffset{};
        HIT_AREA_SHAPE wire{};
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
    };
    struct PRODUCT_PATTERN final
    {
        KOUKU_SAYDON_COMPOSITION_DOCUMENT document;
        KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
        std::uint32_t durationMs = 0;
    };
    struct CARD final { std::string assetId; std::uint32_t handle = 0; };
    void Sample(SESSION& session, const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, float clockMs, bool paused,
        const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model);
    void Stop_Session(SESSION& session);
    bool Ensure_Effects();
    void Restore_Scene();
    void Refresh_SharedPresentation();
    void Collect_FrameLights();
    struct FRAME_LIGHT_PROVIDER;
    std::shared_ptr<FRAME_LIGHT_PROVIDER> m_LightProvider;
    const CLightResourceCatalog* m_pLightResources = nullptr;
    std::vector<float4x4_t> m_LightPlayerPivots;
    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_Context;
    CRenderingProfileService& m_Profiles;
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> m_Effects;
    std::map<std::string, PRODUCT_PATTERN> m_Product;
    std::set<std::string> m_MissingProductPatterns;
    std::map<std::uint32_t, SESSION> m_BossSessions;
    std::map<std::uint32_t, CARD> m_Cards;
    std::map<std::string, bool> m_ColliderDebugOverrides;
    std::uint64_t m_iColliderAuthoringGeneration = UINT64_MAX;
    bool m_bProductLoaded = false, m_bProductAttempted = false, m_bEffectsAttempted = false;
    std::string m_strScenePrevious, m_strSceneOwner, m_strSceneApplied;
    bool m_bSceneUsed = false, m_bCameraUsed = false;
    SESSION m_PreviewSession;
    KOUKU_SAYDON_COMPOSITION_DOCUMENT m_PreviewDocument;
    KOUKU_SAYDON_COMPOSITION_PATTERN m_PreviewPattern;
    float4x4_t m_PreviewPivot{};
    std::weak_ptr<Engine::CModel> m_PreviewModel;
    bool m_bOwnPreviewClock = false, m_bPreviewPlaying = false, m_bPreviewPaused = false;
    bool m_bPreviewPivotReady = false;
    bool m_bColliderResourcePreview = false;
    double m_fPreviewClockMs = 0;
    std::uint32_t m_iPreviewDurationMs = 0;
    std::string m_strStatus;
};
}
