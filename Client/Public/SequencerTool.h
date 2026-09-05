#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "CompositionWorkbenchSession.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

NS_BEGIN(Client)

// One Workbench shell over independent boss authoring sessions. It owns window
// placement, visibility and raw animation browsing; no descriptor or publisher is loaded
// to decide whether a pane can be shown.
class CSequencerTool final
{
public:
    enum class ANIMATION_PREVIEW_TRANSPORT : std::uint8_t { NONE, PAUSE, RESUME, STOP };
    struct ANIMATION_PREVIEW_STATE final
    {
        std::string strPatternId;
        std::string strStatus;
        bool bPlaying = false;
        bool bPaused = false;
        std::uint32_t iClockMs = 0u;
        std::uint32_t iDurationMs = 0u;
    };

    CSequencerTool(
        ICompositionWorkbenchSession* pValtanSession,
        ICompositionWorkbenchSession* pKoukuSaydonSession);

    void Open() { m_bOpen = true; }
    void Open(COMPOSITION_WORKBENCH_BOSS boss) {
        m_eSelectedBoss = boss;
        m_bOpen = true;
    }
    [[nodiscard]] bool_t Is_Open() const noexcept { return m_bOpen; }
    [[nodiscard]] COMPOSITION_WORKBENCH_BOSS Get_SelectedBoss() const noexcept {
        return m_eSelectedBoss;
    }
    void Render();
    void Set_AnimationResources(std::vector<COMPOSITION_ANIMATION_RESOURCE> resources,
        std::string status);
    bool Consume_ResourceRefreshRequest();
    bool Consume_AnimationPreviewRequest(COMPOSITION_ANIMATION_RESOURCE& resource);
    void Set_AnimationPreviewStatus(std::string status);
    bool Consume_AnimationPreviewTransportRequest(ANIMATION_PREVIEW_TRANSPORT& transport);
    void Set_AnimationPreviewState(ANIMATION_PREVIEW_STATE state);

private:
    void Render_WindowMenu();
    void Render_BossSelector();
    void Render_PhysicalAnimationBrowser(ICompositionWorkbenchSession& session);
    void Queue_AnimationPreview(const COMPOSITION_ANIMATION_RESOURCE& resource);
    void Apply_ViewRequest(ICompositionWorkbenchSession& session);
    void Render_Pane(
        ICompositionWorkbenchSession& session,
        COMPOSITION_WORKBENCH_PANE pane);
    [[nodiscard]] ICompositionWorkbenchSession* Selected_Session() const noexcept;

    ICompositionWorkbenchSession* m_pValtanSession = nullptr;
    ICompositionWorkbenchSession* m_pKoukuSaydonSession = nullptr;
    COMPOSITION_WORKBENCH_BOSS m_eSelectedBoss = COMPOSITION_WORKBENCH_BOSS::VALTAN;
    bool_t m_bOpen = true;
    bool_t m_bResetLayoutRequested = false;
    bool_t m_bApplyResetLayoutThisFrame = false;
    bool_t m_bSequencerMaximized = false;
    bool_t m_bExpandResourcesRequested = false;
    bool_t m_bFocusResourcesRequested = false;
    bool_t m_bFocusPatternsRequested = false;
    bool m_bResourceRefreshRequested = true;
    bool m_bHasSelectedAnimationResource = false;
    bool m_bAnimationPreviewPending = false;
    std::array<char, 160u> m_AnimationResourceSearch{};
    std::vector<COMPOSITION_ANIMATION_RESOURCE> m_AnimationResources;
    COMPOSITION_ANIMATION_RESOURCE m_SelectedAnimationResource;
    COMPOSITION_ANIMATION_RESOURCE m_PendingAnimationPreview;
    std::string m_strAnimationResourceStatus;
    std::string m_strAnimationBrowserStatus;
    ANIMATION_PREVIEW_TRANSPORT m_eAnimationPreviewTransport = ANIMATION_PREVIEW_TRANSPORT::NONE;
    ANIMATION_PREVIEW_STATE m_AnimationPreviewState;
    std::array<bool, static_cast<std::size_t>(COMPOSITION_WORKBENCH_PANE::COUNT)>
        m_PaneVisible = { true, true, true, true, true, false, true };
};

NS_END
