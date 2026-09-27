#pragma once
#include "CompositionResourceTree.h"
#include "ClassSelectionTimeline.h"

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "CompositionWorkbenchSession.h"

#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
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

    struct CLASS_SELECTION_PREVIEW_OPTION final
    {
        std::string categoryId, label, classId;
    };
    struct CLASS_SELECTION_PREVIEW_STATE final
    {
        std::vector<CLASS_SELECTION_PREVIEW_OPTION> options;
        std::size_t selectedCategory = 0u;
        std::string selectedClassId, activeClassId, selectedLabel;
        bool authoringDirty = false, authoringPublishPending = false;
        std::string authoringStatus;
        bool available = false;
        bool active = false;
        bool paused = false;
        bool looping = false;
        std::uint64_t ownerToken = 0;
        std::uint64_t loopCycle = 0;
        double clockMs = 0.;
        double sourceClockMs = 0., sourceRate = 1., playbackRate = 1.;
        CLASS_MOVIE_CAMERA_SAMPLE cameraSample;
        double durationMs = 0.;
        double introDurationMs = 0.;
        double loopDurationMs = 0.;
        double activeIntroDurationMs = 0.;
        double activeLoopDurationMs = 0.;
        std::string status;
    };
    // The Level owns playback, camera and resources. This session only submits
    // transport commands to that same product presentation and reads its clock.
    struct CLASS_SELECTION_PREVIEW_CALLBACKS final
    {
        std::function<CLASS_SELECTION_PREVIEW_STATE()> state;
        std::function<void(std::size_t)> selectCategory;
        std::function<bool()> play;
        std::function<void()> stop;
        std::function<void(bool)> setPaused;
        std::function<bool(bool, double)> seek;
        std::function<bool(double)> setPlaybackRate;
        std::function<std::shared_ptr<const CLASS_MOVIE_TIMELINE>(const std::string&, bool)> timeline;
        std::function<bool(std::string&)> beginAuthoring, saveAuthoring, reloadAuthoring;
        std::function<bool(const std::string&, bool, const std::string&, const std::string&, CLASS_MOVIE_AUTHORING_BOX&, std::string&)> editableBox;
        std::function<bool(const CLASS_MOVIE_AUTHORING_BOX&, const DATA_JSON_VALUE&, std::string&)> applyBox;
        std::function<bool(const std::string&, bool, const std::string&, std::string&)> openEffectEditor;
    };

    CSequencerTool(
        ICompositionWorkbenchSession* pValtanSession,
        ICompositionWorkbenchSession* pKoukuSaydonSession,
        bool sequenceWorkspace = false);

    void Set_ActionSessions(ICompositionWorkbenchSession* character,
        ICompositionWorkbenchSession* object, ICompositionWorkbenchSession* sequence);
    void Set_ClassSelectionPreviewCallbacks(CLASS_SELECTION_PREVIEW_CALLBACKS callbacks);
    void Set_TargetChangedCallback(std::function<void(COMPOSITION_WORKBENCH_TARGET)> callback);
    /* Another tool may host the selected session for one frame. The shell
       still draws its windows but must not open a second session frame. */
    void Suppress_SessionFrameThisFrame(ICompositionWorkbenchSession* hosted,
        ICompositionWorkbenchSession* alsoHosted = nullptr) noexcept
    { m_pExternallyHostedSession = hosted; m_pExternallyHostedSessionAlt = alsoHosted; }
    void Open(COMPOSITION_WORKBENCH_TARGET target);
    void Open(COMPOSITION_WORKBENCH_TARGET target, COMPOSITION_WORKBENCH_BOSS boss);
    void Deactivate();
    [[nodiscard]] COMPOSITION_WORKBENCH_TARGET Get_SelectedTarget() const noexcept { return m_eSelectedTarget; }
    [[nodiscard]] bool Is_BossSelected() const noexcept { return m_eSelectedTarget == COMPOSITION_WORKBENCH_TARGET::BOSS; }
    [[nodiscard]] bool Uses_ValtanSession() const noexcept {
        return (Is_BossSelected() || m_eSelectedTarget == COMPOSITION_WORKBENCH_TARGET::SEQUENCE) &&
            Get_SelectedBoss() == COMPOSITION_WORKBENCH_BOSS::VALTAN;
    }
    void Open();
    void Open(COMPOSITION_WORKBENCH_BOSS boss);
    [[nodiscard]] bool_t Is_Open() const noexcept { return m_bOpen; }
    [[nodiscard]] COMPOSITION_WORKBENCH_BOSS Get_SelectedBoss() const noexcept {
        return m_eSelectedTarget == COMPOSITION_WORKBENCH_TARGET::SEQUENCE ? m_eSequenceBoss : m_eSelectedBoss;
    }
    void Render();
    void Set_AnimationResources(std::vector<COMPOSITION_ANIMATION_RESOURCE> resources,
        std::string status);
    bool Consume_ResourceRefreshRequest();
    bool Consume_AnimationPreviewRequest(COMPOSITION_ANIMATION_RESOURCE& resource);
    void Set_AnimationPreviewStatus(std::string status);
    bool Consume_AnimationPreviewTransportRequest(ANIMATION_PREVIEW_TRANSPORT& transport);
    void Set_AnimationPreviewState(ANIMATION_PREVIEW_STATE state);
    [[nodiscard]] const ANIMATION_PREVIEW_STATE& Get_AnimationPreviewState() const noexcept {
        return m_AnimationPreviewState;
    }

private:
    void Select_Boss(COMPOSITION_WORKBENCH_BOSS boss);
    void Select_Target(COMPOSITION_WORKBENCH_TARGET target);
    void Render_ActionSelector();
    void Render_WindowMenu();
    void Render_BossSelector();
    void Render_PhysicalAnimationBrowser(ICompositionWorkbenchSession& session);
    void Queue_AnimationPreview(const COMPOSITION_ANIMATION_RESOURCE& resource);
    void Apply_ViewRequest(ICompositionWorkbenchSession& session);
    void Render_Pane(
        ICompositionWorkbenchSession& session,
        COMPOSITION_WORKBENCH_PANE pane);
    [[nodiscard]] ICompositionWorkbenchSession* Selected_Session() const noexcept;

    bool m_bInsideFrame = false;
    bool m_bCompositionEditFocused = false;
    bool m_bPhysicalAnimationFocused = false;
    std::optional<COMPOSITION_EDIT_COMMAND> m_PendingCompositionEdit;
    COMPOSITION_TRANSFER m_PendingCompositionTransfer;
    std::string m_CompositionEditStatus;
    bool m_bBossChangePending = false;
    COMPOSITION_WORKBENCH_BOSS m_ePendingBoss = COMPOSITION_WORKBENCH_BOSS::VALTAN;
    ICompositionWorkbenchSession* m_pCharacterSession = nullptr;
    ICompositionWorkbenchSession* m_pObjectSession = nullptr;
    ICompositionWorkbenchSession* m_pSequenceSession = nullptr;
    std::unique_ptr<ICompositionWorkbenchSession> m_pClassSelectionSession;
    std::function<void(COMPOSITION_WORKBENCH_TARGET)> m_TargetChanged;
    COMPOSITION_WORKBENCH_TARGET m_eSelectedTarget = COMPOSITION_WORKBENCH_TARGET::BOSS;
    COMPOSITION_WORKBENCH_TARGET m_ePendingTarget = COMPOSITION_WORKBENCH_TARGET::BOSS;
    bool m_bTargetChangePending = false;
    ICompositionWorkbenchSession* m_pValtanSession = nullptr;
    ICompositionWorkbenchSession* m_pKoukuSaydonSession = nullptr;
    ICompositionWorkbenchSession* m_pExternallyHostedSession = nullptr;
    ICompositionWorkbenchSession* m_pExternallyHostedSessionAlt = nullptr;
    COMPOSITION_WORKBENCH_BOSS m_eSelectedBoss = COMPOSITION_WORKBENCH_BOSS::VALTAN;
    COMPOSITION_WORKBENCH_BOSS m_eSequenceBoss = COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON;
    const bool m_bSequenceWorkspace;
    bool_t m_bOpen = true;
    bool_t m_bRestoreAuthoringPanesRequested = false;
    bool_t m_bPhysicalAnimationBrowserVisible = true;
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
    COMPOSITION_RESOURCE_TREE_NODE m_AnimationResourceTree;
    std::string m_AnimationResourceQuery;
    bool m_bAnimationResourceTreeDirty = true;
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
