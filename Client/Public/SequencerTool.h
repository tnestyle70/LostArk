#pragma once
// 자원 목록의 category tree를 값 멤버로 보관한다.
#include "CompositionResourceTree.h"
// movie timeline snapshot과 편집 box·history selection 타입을 callback 서명에 사용한다.
#include "ClassSelectionTimeline.h"
// Inspector callback 묶음을 값 멤버로 보관한다.
#include "ClassMovieInspection.h"

#include "Client_Defines.h"
#include "Engine_Defines.h"
// 보스별 구현 대신 공용 virtual 세션 계약과 pane enum으로 호출한다.
#include "CompositionWorkbenchSession.h"

#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

NS_BEGIN(Client)

// 독립된 저작 세션을 같은 창 구성으로 보여 주는 shell이다. 창 위치·표시 상태와 물리 Animation 목록을 소유한다.
// pane를 표시할 수 있는지 결정하기 위해 문서 descriptor나 publisher를 로드하지 않는다.
// 편집할 문서와 저장 정책은 연결된 세션에 남겨 보스마다 다른 데이터 형식을 유지한다.
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
        bool canUndoAuthoring = false, canRedoAuthoring = false;
        std::string authoringStatus;
        std::uint64_t authoringGeneration = 0;
        bool available = false;
        bool active = false;
        bool paused = false;
        bool looping = false;
        bool repeatMovie = true;
        bool completedHold = false;
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
    // 재생·카메라·자원은 Level이 소유한다. 이 세션은 같은 제품 재생 경로에
    // transport 명령을 전달하고 그 소유자가 반환하는 시계를 읽는다.
    //전부 다 call back 함수들인 건가? 이것들이 실제 sequencer에서 어떤 식으로 동작하는 구조인 거지?
    // 맞다. MainApp이 Level/재생 소유자를 연결한 std::function 묶음이다. UI 입력이 오면 이 함수를 호출하고 다음 state() 값으로 결과를 읽는다.
    // closure는 복사되어 저장되므로 참조로 잡은 대상은 세션보다 오래 살아야 한다. 모든 callback이 설정됐다고 가정하지 않는다.
    struct CLASS_SELECTION_PREVIEW_CALLBACKS final
    {
        std::function<CLASS_SELECTION_PREVIEW_STATE()> state;
        std::function<void(std::size_t)> selectCategory;
        std::function<bool()> play;
        std::function<void()> stop;
        std::function<void(bool)> setPaused;
        std::function<bool(bool, double)> seek;
        std::function<bool(double)> setPlaybackRate;
        std::function<bool(const std::string&, bool, std::string&)> setRepeatMovie;
        std::function<std::shared_ptr<const CLASS_MOVIE_TIMELINE>(const std::string&, bool)> timeline;
        std::function<bool(std::string&)> beginAuthoring, saveAuthoring, reloadAuthoring, publishAuthoring;
        std::function<void(const CLASS_MOVIE_HISTORY_SELECTION&)> setHistorySelection;
        std::function<void()> finishHistory;
        std::function<bool(CLASS_MOVIE_HISTORY_SELECTION&, std::string&)> undoAuthoring, redoAuthoring;
        std::function<double(const std::string&, bool, double, bool)> mapTime;
        std::function<bool(VALTAN_CINEMATIC_CAMERA_POSE&, std::string&)> captureFreeCamera;
        std::function<bool(const std::string&, bool, const std::string&, const std::string&, CLASS_MOVIE_AUTHORING_BOX&, std::string&)> editableBox;
        std::function<bool(const CLASS_MOVIE_AUTHORING_BOX&, const DATA_JSON_VALUE&, std::string&)> applyBox;
        std::function<bool(const CLASS_MOVIE_AUTHORING_BOX&, double, double, CLASS_MOVIE_TIMING_EDIT, std::string&)> editTiming;
        std::function<bool(const std::string&, bool, const std::string&, std::string&)> openEffectEditor;
        CLASS_MOVIE_INSPECTION_CALLBACKS inspection;
    };

    // 두 보스 세션 포인터를 차용한다. MainApp이 세션을 소유하며 shell이 이 포인터를 delete하지 않는다.
    CSequencerTool(
        ICompositionWorkbenchSession* pValtanSession,
        ICompositionWorkbenchSession* pKoukuSaydonSession,
        bool sequenceWorkspace = false);

    void Set_ActionSessions(ICompositionWorkbenchSession* character,
        ICompositionWorkbenchSession* object, ICompositionWorkbenchSession* sequence);
    void Set_ClassSelectionPreviewCallbacks(CLASS_SELECTION_PREVIEW_CALLBACKS callbacks);
    void Set_TargetChangedCallback(std::function<void(COMPOSITION_WORKBENCH_TARGET)> callback);
    // 다른 도구가 이번 프레임에 같은 세션을 호스팅할 수 있다. shell은 창을 유지하되 같은 세션의 Begin/End를 두 번 열지 않는다.
    void Suppress_SessionFrameThisFrame(ICompositionWorkbenchSession* hosted,
        ICompositionWorkbenchSession* alsoHosted = nullptr) noexcept
    { m_pExternallyHostedSession = hosted; m_pExternallyHostedSessionAlt = alsoHosted; }
    void Open(COMPOSITION_WORKBENCH_TARGET target);
    void Open(COMPOSITION_WORKBENCH_TARGET target, COMPOSITION_WORKBENCH_BOSS boss);
    void Deactivate();
    [[nodiscard]] COMPOSITION_WORKBENCH_TARGET Get_SelectedTarget() const noexcept { return m_eSelectedTarget; }
    [[nodiscard]] bool Is_BossSelected() const noexcept { return m_eSelectedTarget == COMPOSITION_WORKBENCH_TARGET::BOSS; }
    [[nodiscard]] bool Uses_ValtanSession() const noexcept {
        return (Is_BossSelected() || m_eSelectedTarget == COMPOSITION_WORKBENCH_TARGET::SEQUENCE ||
            m_eSelectedTarget == COMPOSITION_WORKBENCH_TARGET::OBJECT) &&
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
    // 연결된 세션은 외부 소유다. 아래 class-selection 세션만 이 shell의 unique_ptr가 소유한다.
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
    // ImGui 입력에는 수정 가능한 고정 char 버퍼가 필요하다. 아래 자원 vector는 외부 inventory를 값으로 받아 프레임 사이에 소유한다.
    std::array<char, 160u> m_AnimationResourceSearch{};
    std::vector<COMPOSITION_ANIMATION_RESOURCE> m_AnimationResources;
    COMPOSITION_RESOURCE_TREE_NODE m_AnimationResourceTree;
    std::string m_AnimationResourceQuery;
    bool m_bAnimationResourceTreeDirty = true;
    // 선택과 재생 대기는 행 포인터 대신 값 복사본이다. 목록 refresh로 vector가 교체되어도 요청의 자원 식별이 유지된다.
    COMPOSITION_ANIMATION_RESOURCE m_SelectedAnimationResource;
    COMPOSITION_ANIMATION_RESOURCE m_PendingAnimationPreview;
    std::string m_strAnimationResourceStatus;
    std::string m_strAnimationBrowserStatus;
    ANIMATION_PREVIEW_TRANSPORT m_eAnimationPreviewTransport = ANIMATION_PREVIEW_TRANSPORT::NONE;
    ANIMATION_PREVIEW_STATE m_AnimationPreviewState;
    // pane 종류는 고정되어 enum index로 직접 접근한다. 문서의 stable ID를 이 배열 index로 대체하는 것은 아니다.
    std::array<bool, static_cast<std::size_t>(COMPOSITION_WORKBENCH_PANE::COUNT)>
        m_PaneVisible = { true, true, true, true, true, false, true };
};

NS_END
