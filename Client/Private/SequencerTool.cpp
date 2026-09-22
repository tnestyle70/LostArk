#include "imgui.h"
#include "SequencerTool.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string_view>
#include <utility>

namespace
{
    using BOSS = Client::COMPOSITION_WORKBENCH_BOSS;
    using PANE = Client::COMPOSITION_WORKBENCH_PANE;
    using TARGET = Client::COMPOSITION_WORKBENCH_TARGET;

    constexpr std::array<PANE, 6u> PANES = {
        PANE::SEQUENCER, PANE::PATTERNS, PANE::RESOURCES,
        PANE::DETAILS, PANE::PREVIEW, PANE::BOSS_PATTERN };

    const char* BossLabel(const BOSS boss)
    {
        switch (boss)
        {
        case BOSS::VALTAN: return "Valtan";
        case BOSS::KOUKU_SAYDON: return "Saydon";
        case BOSS::KOUKU_SAYDON_GATE2: return "Large Saydon, Kouku";
        case BOSS::KOUKU_SAYDON_GATE3: return "Saydon (Gate 3)";
        case BOSS::KOUKU_SAYDON_ENCORE: return "Encore Saydon";
        default: return "Unavailable boss";
        }
    }

    constexpr std::array<BOSS, 5u> BOSS_ENTRIES = {
        BOSS::VALTAN, BOSS::KOUKU_SAYDON, BOSS::KOUKU_SAYDON_GATE2,
        BOSS::KOUKU_SAYDON_GATE3, BOSS::KOUKU_SAYDON_ENCORE };

    class CClassSelectionWorkbenchSession final : public Client::ICompositionWorkbenchSession
    {
    public:
        using CALLBACKS = Client::CSequencerTool::CLASS_SELECTION_PREVIEW_CALLBACKS;
        using STATE = Client::CSequencerTool::CLASS_SELECTION_PREVIEW_STATE;

        explicit CClassSelectionWorkbenchSession(CALLBACKS callbacks = {})
            : m_Callbacks(std::move(callbacks)) {}

        void On_WorkbenchDeactivated() override
        {
            const auto state = Read_State();
            if (m_OwnsPlayback && state.active && state.ownerToken == m_OwnerToken && m_Callbacks.stop)
                m_Callbacks.stop();
            m_OwnsPlayback = false;
            m_Scrubbing = false;
            m_Pending = COMMAND::NONE;
        }

        void Begin_WorkbenchFrame() override
        {
            m_State = Read_State();
            if (!m_State.active || m_State.ownerToken != m_OwnerToken) m_OwnsPlayback = false;
            if (!m_Scrubbing) m_EditMs = static_cast<float>(m_State.clockMs);
        }

        void Render_WorkbenchPane(const PANE pane) override
        {
            switch (pane)
            {
            case PANE::PATTERNS:
                ImGui::SeparatorText("World sequences");
                if (ImGui::BeginCombo("World##ClassSelection", "Character Select"))
                {
                    ImGui::Selectable("Character Select", true);
                    ImGui::EndCombo();
                }
                ImGui::Selectable("Guardian Knight / Intro + Loop", true);
                break;
            case PANE::TOOLBAR:
                Render_Transport();
                break;
            case PANE::SEQUENCER:
                Render_Timeline();
                break;
            case PANE::DETAILS:
                ImGui::TextUnformatted("Guardian Knight");
                ImGui::Text("Intro: %.3f s", m_State.introDurationMs * .001);
                ImGui::Text("Loop: %.3f s", m_State.loopDurationMs * .001);
                ImGui::TextWrapped("The intro plays once, then the loop repeats until Stop.");
                ImGui::TextWrapped("Camera cuts, actor poses and effects use the same class-selection playback.");
                break;
            case PANE::RESOURCES:
                ImGui::TextUnformatted("Character Select / Guardian Knight");
                ImGui::BulletText("Camera cuts");
                ImGui::BulletText("Character and dragon");
                ImGui::BulletText("Stage and effects");
                break;
            case PANE::PREVIEW:
                ImGui::TextWrapped("Play shows the sequence in the current Character Select viewport.");
                ImGui::TextWrapped("Stop restores the camera. Pause holds the sequence; drag the timeline to inspect a moment.");
                break;
            default: break;
            }
        }

        void End_WorkbenchFrame() override
        {
            const auto command = std::exchange(m_Pending, COMMAND::NONE);
            switch (command)
            {
            case COMMAND::PLAY:
                if (m_Callbacks.play && m_Callbacks.play()) Claim_Playback();
                break;
            case COMMAND::STOP:
                if (m_Callbacks.stop) m_Callbacks.stop();
                m_OwnsPlayback = false;
                break;
            case COMMAND::PAUSE:
                if (m_Callbacks.setPaused) { m_Callbacks.setPaused(m_RequestedPause); Claim_Playback(); }
                break;
            case COMMAND::SEEK:
                if (m_Callbacks.setPaused) m_Callbacks.setPaused(true);
                if (m_Callbacks.seek && m_Callbacks.seek(m_SeekLoop, m_SeekMs))
                {
                    if (m_Callbacks.setPaused) m_Callbacks.setPaused(true);
                    Claim_Playback();
                }
                break;
            default: break;
            }
        }

    private:
        enum class COMMAND { NONE, PLAY, STOP, PAUSE, SEEK };

        STATE Read_State() const
        {
            if (m_Callbacks.state) return m_Callbacks.state();
            STATE state;
            state.status = "Enter Character Select from the Lobby to play this sequence.";
            return state;
        }

        void Claim_Playback()
        {
            const auto state = Read_State();
            m_OwnerToken = state.ownerToken;
            m_OwnsPlayback = state.active;
        }

        void Queue_Seek(const bool loop, const double timeMs)
        {
            m_SeekLoop = loop;
            // The slider uses floats while the source slomo duration is double.
            // Its rounded-up final tick must still request the valid phase end.
            const double durationMs = loop ? m_State.loopDurationMs : m_State.introDurationMs;
            m_SeekMs = std::clamp(timeMs, 0., (std::max)(0., durationMs));
            m_Pending = COMMAND::SEEK;
        }

        void Render_Transport()
        {
            ImGui::TextUnformatted("World / Character Select / Guardian Knight");
            ImGui::BeginDisabled(!m_State.available || !m_Callbacks.play);
            if (ImGui::Button(m_State.active ? "Restart intro" : "Play")) m_Pending = COMMAND::PLAY;
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::BeginDisabled(!m_State.active || !m_Callbacks.setPaused);
            if (ImGui::Button(m_State.paused ? "Resume" : "Pause"))
            {
                m_RequestedPause = !m_State.paused;
                m_Pending = COMMAND::PAUSE;
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::BeginDisabled(!m_State.active || !m_Callbacks.stop);
            if (ImGui::Button("Stop")) m_Pending = COMMAND::STOP;
            ImGui::EndDisabled();
            if (!m_State.status.empty()) ImGui::TextWrapped("%s", m_State.status.c_str());
        }

        void Render_Timeline()
        {
            ImGui::Text("%s / %.3f of %.3f s", m_State.looping ? "Loop" : "Intro",
                m_State.clockMs * .001, m_State.durationMs * .001);
            if (m_State.looping)
                ImGui::Text("Loop cycle %llu", static_cast<unsigned long long>(m_State.loopCycle + 1u));
            const float duration = static_cast<float>(m_State.durationMs);
            const float progress = duration > 0.f ? std::clamp(static_cast<float>(m_State.clockMs) / duration, 0.f, 1.f) : 0.f;
            ImGui::ProgressBar(progress, ImVec2(-1.f, 0.f));
            ImGui::BeginDisabled(!m_State.active || !m_Callbacks.seek || duration <= 0.f);
            ImGui::SetNextItemWidth(-1.f);
            (void)ImGui::SliderFloat("##ClassSelectionClock", &m_EditMs, 0.f, (std::max)(1.f, duration), "%.0f ms");
            if (ImGui::IsItemActivated())
            {
                m_Scrubbing = true;
                m_ScrubLoop = m_State.looping;
                m_RequestedPause = true;
                m_Pending = COMMAND::PAUSE;
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) Queue_Seek(m_ScrubLoop, m_EditMs);
            if (ImGui::IsItemDeactivated()) m_Scrubbing = false;
            if (ImGui::Button("Intro start")) Queue_Seek(false, 0.);
            ImGui::SameLine();
            if (ImGui::Button("Loop start")) Queue_Seek(true, 0.);
            ImGui::EndDisabled();
        }

        CALLBACKS m_Callbacks;
        STATE m_State;
        COMMAND m_Pending = COMMAND::NONE;
        std::uint64_t m_OwnerToken = 0;
        bool m_OwnsPlayback = false;
        bool m_RequestedPause = false;
        bool m_Scrubbing = false;
        bool m_ScrubLoop = false;
        bool m_SeekLoop = false;
        float m_EditMs = 0.f;
        double m_SeekMs = 0.;
    };

    const char* PaneLabel(const PANE pane, const bool sequenceWorkspace)
    {
        if (sequenceWorkspace && pane == PANE::PATTERNS)
            return "Sequences";
        if (sequenceWorkspace && pane == PANE::BOSS_PATTERN)
            return "Boss Sequences";
        if (sequenceWorkspace && pane == PANE::TOOLBAR)
            return "Sequencer Benchmark";
        switch (pane)
        {
        case PANE::SEQUENCER: return "Sequencer";
        case PANE::PATTERNS: return "Actions";
        case PANE::RESOURCES: return "Resources";
        case PANE::DETAILS: return "Box Detail";
        case PANE::PREVIEW: return "Preview";
        case PANE::BOSS_PATTERN: return "Boss Pattern";
        case PANE::TOOLBAR: return "Action Workbench";
        default: return "Unavailable pane";
        }
    }

    bool SameAnimationResource(const Client::COMPOSITION_ANIMATION_RESOURCE& left,
        const Client::COMPOSITION_ANIMATION_RESOURCE& right)
    {
        return left.strTargetAssetName == right.strTargetAssetName &&
            left.strModelAssetId == right.strModelAssetId &&
            left.strSourceAssetId == right.strSourceAssetId &&
            left.strRuntimeClip == right.strRuntimeClip;
    }

    bool ResourceTextMatches(const std::string_view text, const std::string_view query)
    {
        return query.empty() || std::search(text.begin(), text.end(), query.begin(), query.end(),
            [](const unsigned char left, const unsigned char right) {
                return std::tolower(left) == std::tolower(right);
            }) != text.end();
    }

    const char* PaneWindowId(const PANE pane, const bool sequenceWorkspace)
    {
        if (sequenceWorkspace)
        {
            switch (pane)
            {
            case PANE::SEQUENCER: return "Composition Sequencer###SequenceBenchmarkSequencerWindow";
            case PANE::PATTERNS: return "Composition Sequencer###SequenceBenchmarkSequencesWindow";
            case PANE::RESOURCES: return "Composition Resources###SequenceBenchmarkResourcesWindow";
            case PANE::DETAILS: return "Box Detail###SequenceBenchmarkDetailsWindow";
            case PANE::PREVIEW: return "Composition Preview###SequenceBenchmarkPreviewWindow";
            case PANE::BOSS_PATTERN: return "Composition Sequences###SequenceBenchmarkBossPatternWindow";
            case PANE::TOOLBAR: return "Sequencer Benchmark###SequenceBenchmarkSessionWindow";
            default: return "Unavailable pane###SequenceBenchmarkUnavailableWindow";
            }
        }
        switch (pane)
        {
        case PANE::SEQUENCER:
            return "Composition Sequencer###CompositionSequencerWindowResizableV3";
        case PANE::PATTERNS:
            return "Composition Actions###CompositionPatternsWindow";
        case PANE::RESOURCES:
            return "Composition Resources###CompositionResourcesWindowResizableV2";
        case PANE::DETAILS:
            return "Box Detail###CompositionDetailsWindow";
        case PANE::PREVIEW:
            return "Composition Preview###CompositionPreviewWindow";
        case PANE::BOSS_PATTERN:
            return "Boss Pattern###CompositionBossPatternWindow";
        case PANE::TOOLBAR:
            return "Action Workbench###CompositionSessionWindow";
        default: return "Unavailable pane###CompositionUnavailablePane";
        }
    }

    struct PANE_PLACEMENT final
    {
        ImVec2 position;
        ImVec2 size;
    };

    PANE_PLACEMENT PanePlacement(const PANE pane)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 origin = nullptr == viewport ? ImVec2(20.f, 20.f) : viewport->WorkPos;
        const ImVec2 available = nullptr == viewport ? ImVec2(1600.f, 900.f) : viewport->WorkSize;
        constexpr float margin = 8.f;
        constexpr float gap = 8.f;
        const float contentWidth = (std::max)(1.f, available.x - margin * 2.f - gap * 2.f);
        const float contentHeight = (std::max)(1.f, available.y - margin * 2.f);
        const float leftWidth = contentWidth * 0.18f;
        const float rightWidth = contentWidth * 0.20f;
        const float centerWidth = contentWidth - leftWidth - rightWidth;
        const float leftTopHeight = contentHeight * 0.58f;
        const float previewHeight = contentHeight * 0.30f;
        const float toolbarHeight = contentHeight * 0.18f;
        const float sequencerHeight = (std::max)(1.f, contentHeight - previewHeight - toolbarHeight - gap * 2.f);
        const float leftX = origin.x + margin;
        const float centerX = leftX + leftWidth + gap;
        const float rightX = centerX + centerWidth + gap;
        const float topY = origin.y + margin;
        switch (pane)
        {
        case PANE::PATTERNS:
            return { { leftX, topY }, { leftWidth, leftTopHeight } };
        case PANE::RESOURCES:
            return { { leftX, topY + leftTopHeight + gap },
                { leftWidth, (std::max)(1.f, contentHeight - leftTopHeight - gap) } };
        case PANE::DETAILS:
            return { { rightX, topY }, { rightWidth, contentHeight } };
        case PANE::PREVIEW:
            return { { centerX, topY }, { centerWidth, previewHeight } };
        case PANE::SEQUENCER:
            return { { centerX, topY + previewHeight + gap }, { centerWidth, sequencerHeight } };
        case PANE::TOOLBAR:
            return { { centerX, topY + previewHeight + sequencerHeight + gap * 2.f },
                { centerWidth, toolbarHeight } };
        case PANE::BOSS_PATTERN:
            return { { centerX + 24.f, topY + 24.f }, { centerWidth, contentHeight * 0.65f } };
        default: return { origin, { 320.f, 200.f } };
        }
    }
}

Client::CSequencerTool::CSequencerTool(
    ICompositionWorkbenchSession* pValtanSession,
    ICompositionWorkbenchSession* pKoukuSaydonSession,
    const bool sequenceWorkspace)
    : m_pValtanSession(pValtanSession)
    , m_pKoukuSaydonSession(pKoukuSaydonSession)
    , m_eSelectedBoss(sequenceWorkspace ? BOSS::KOUKU_SAYDON : BOSS::VALTAN)
    , m_bSequenceWorkspace(sequenceWorkspace)
{
    m_pClassSelectionSession = std::make_unique<CClassSelectionWorkbenchSession>();
}

void Client::CSequencerTool::Set_ActionSessions(ICompositionWorkbenchSession* character,
    ICompositionWorkbenchSession* object, ICompositionWorkbenchSession* sequence)
{
    m_pCharacterSession = character;
    m_pObjectSession = object;
    m_pSequenceSession = sequence;
}

void Client::CSequencerTool::Set_TargetChangedCallback(std::function<void(TARGET)> callback)
{
    m_TargetChanged = std::move(callback);
}

void Client::CSequencerTool::Set_ClassSelectionPreviewCallbacks(CLASS_SELECTION_PREVIEW_CALLBACKS callbacks)
{
    if (m_pClassSelectionSession) m_pClassSelectionSession->On_WorkbenchDeactivated();
    m_pClassSelectionSession = std::make_unique<CClassSelectionWorkbenchSession>(std::move(callbacks));
}

void Client::CSequencerTool::Select_Target(const TARGET target)
{
    if (target == m_eSelectedTarget) return;
    if (auto* previous = Selected_Session()) previous->On_WorkbenchDeactivated();
    m_eSelectedTarget = target;
    m_bAnimationPreviewPending = false;
    m_eAnimationPreviewTransport = ANIMATION_PREVIEW_TRANSPORT::NONE;
    m_AnimationPreviewState = {};
    if (m_TargetChanged) m_TargetChanged(target);
    if (target == TARGET::BOSS || target == TARGET::SEQUENCE)
        if (auto* session = Selected_Session())
            session->Select_WorkbenchBoss(Get_SelectedBoss());
}

void Client::CSequencerTool::Open(const TARGET target)
{
    if (m_bInsideFrame)
    {
        m_ePendingTarget = target;
        m_bTargetChangePending = true;
        Open();
        return;
    }
    m_bTargetChangePending = false;
    Select_Target(target);
    Open();
}

void Client::CSequencerTool::Open(const TARGET target, const BOSS boss)
{
    if (m_bInsideFrame)
    {
        m_ePendingTarget = target; m_bTargetChangePending = true;
        m_ePendingBoss = boss; m_bBossChangePending = true;
        Open();
        return;
    }
    m_bTargetChangePending = m_bBossChangePending = false;
    if (target != m_eSelectedTarget)
    {
        // Stage the requested gate before entering the session. Never briefly
        // select the previous gate and discard an exact typed deep-link selection.
        if (target == TARGET::SEQUENCE) m_eSequenceBoss = boss;
        if (target == TARGET::BOSS) m_eSelectedBoss = boss;
        Select_Target(target);
    }
    else Select_Boss(boss);
    Open();
}

void Client::CSequencerTool::Deactivate()
{
    if (auto* session = Selected_Session()) session->On_WorkbenchDeactivated();
    m_bAnimationPreviewPending = false;
    m_eAnimationPreviewTransport = ANIMATION_PREVIEW_TRANSPORT::NONE;
}

void Client::CSequencerTool::Open()
{
    m_bOpen = true;
    m_bSequencerMaximized = false;
    m_PaneVisible[static_cast<std::size_t>(PANE::PATTERNS)] = true;
    m_PaneVisible[static_cast<std::size_t>(PANE::RESOURCES)] = true;
    m_PaneVisible[static_cast<std::size_t>(PANE::DETAILS)] = true;
    m_bRestoreAuthoringPanesRequested = true;
}

void Client::CSequencerTool::Open(const COMPOSITION_WORKBENCH_BOSS boss)
{
    Open(TARGET::BOSS, boss);
}

void Client::CSequencerTool::Select_Boss(const COMPOSITION_WORKBENCH_BOSS boss)
{
    auto& selectedBoss = m_eSelectedTarget == TARGET::SEQUENCE ? m_eSequenceBoss : m_eSelectedBoss;
    if (selectedBoss != boss)
    {
        if (auto* previous = Selected_Session()) previous->On_WorkbenchDeactivated();
        if (m_TargetChanged) m_TargetChanged(m_eSelectedTarget);
    }
    selectedBoss = boss;
    if (ICompositionWorkbenchSession* session = Selected_Session())
        session->Select_WorkbenchBoss(boss);
}

Client::ICompositionWorkbenchSession* Client::CSequencerTool::Selected_Session() const noexcept
{
    switch (m_eSelectedTarget)
    {
    case TARGET::CHARACTER: return m_pCharacterSession;
    case TARGET::OBJECT: return m_pObjectSession;
    case TARGET::WORLD: return m_pClassSelectionSession.get();
    case TARGET::SEQUENCE:
        return m_eSequenceBoss == BOSS::VALTAN ? m_pValtanSession : m_pSequenceSession;
    case TARGET::BOSS: break;
    default: return nullptr;
    }
    switch (m_eSelectedBoss)
    {
    case BOSS::VALTAN: return m_pValtanSession;
    case BOSS::KOUKU_SAYDON:
    case BOSS::KOUKU_SAYDON_GATE2:
    case BOSS::KOUKU_SAYDON_GATE3:
    case BOSS::KOUKU_SAYDON_ENCORE: return m_pKoukuSaydonSession;
    default: return nullptr;
    }
}

void Client::CSequencerTool::Set_AnimationResources(
    std::vector<COMPOSITION_ANIMATION_RESOURCE> resources, std::string status)
{
    // The reader isolates missing packages and retains their last rows. Accept
    // that partial snapshot so healthy models remain available after a failure.
    if (!resources.empty())
        m_AnimationResources = std::move(resources);
    m_strAnimationResourceStatus = std::move(status);
    m_bAnimationResourceTreeDirty = true;
    if (m_bHasSelectedAnimationResource)
    {
        const auto selected = std::find_if(m_AnimationResources.begin(), m_AnimationResources.end(),
            [this](const auto& resource) { return SameAnimationResource(resource, m_SelectedAnimationResource); });
        if (selected != m_AnimationResources.end())
            m_SelectedAnimationResource = *selected;
        else
        {
            m_bHasSelectedAnimationResource = false;
            m_SelectedAnimationResource = {};
        }
    }
}

bool Client::CSequencerTool::Consume_ResourceRefreshRequest()
{
    const bool requested = m_bResourceRefreshRequested;
    m_bResourceRefreshRequested = false;
    return requested;
}

bool Client::CSequencerTool::Consume_AnimationPreviewRequest(COMPOSITION_ANIMATION_RESOURCE& resource)
{
    if (!m_bAnimationPreviewPending)
        return false;
    resource = std::move(m_PendingAnimationPreview);
    m_bAnimationPreviewPending = false;
    return true;
}

void Client::CSequencerTool::Set_AnimationPreviewStatus(std::string status)
{
    m_strAnimationBrowserStatus = std::move(status);
}

bool Client::CSequencerTool::Consume_AnimationPreviewTransportRequest(
    ANIMATION_PREVIEW_TRANSPORT& transport)
{
    transport = m_eAnimationPreviewTransport;
    m_eAnimationPreviewTransport = ANIMATION_PREVIEW_TRANSPORT::NONE;
    return transport != ANIMATION_PREVIEW_TRANSPORT::NONE;
}

void Client::CSequencerTool::Set_AnimationPreviewState(ANIMATION_PREVIEW_STATE state)
{
    m_AnimationPreviewState = std::move(state);
}

void Client::CSequencerTool::Queue_AnimationPreview(const COMPOSITION_ANIMATION_RESOURCE& resource)
{
    m_PendingAnimationPreview = resource;
    m_bAnimationPreviewPending = true;
    m_strAnimationBrowserStatus = "Preview requested: " + resource.strRuntimeClip;
}

void Client::CSequencerTool::Render_PhysicalAnimationBrowser(ICompositionWorkbenchSession& session)
{
    if (!ImGui::CollapsingHeader("Animation Library##CompositionPhysicalAnimation",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;
    if (ImGui::Button("Refresh Animation Resources##CompositionPhysicalAnimation"))
        m_bResourceRefreshRequested = true;
    ImGui::SameLine();
    ImGui::TextDisabled("%zu models / %zu clips",
        COMPOSITION_ANIMATION_TARGET_ASSET_NAMES.size(), m_AnimationResources.size());
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::InputTextWithHint("##CompositionPhysicalAnimationSearch", "Search character, designer name or clip...",
        m_AnimationResourceSearch.data(), m_AnimationResourceSearch.size())) m_bAnimationResourceTreeDirty = true;
    if (m_bAnimationResourceTreeDirty || m_AnimationResourceQuery != m_AnimationResourceSearch.data())
    {
        m_AnimationResourceQuery = m_AnimationResourceSearch.data();
        m_AnimationResourceTree = {};
        for (std::size_t index = 0u; index < m_AnimationResources.size(); ++index)
        {
            const auto& resource = m_AnimationResources[index];
            const auto category = CompositionAnimationCategory(resource.strTargetAssetName);
            const auto& query = m_AnimationResourceQuery;
            if (!ResourceTextMatches(resource.strTargetAssetName, query) &&
                !ResourceTextMatches(resource.strRuntimeClip, query) &&
                !ResourceTextMatches(resource.strDisplayName, query) &&
                !ResourceTextMatches(resource.strSourceAssetId, query) &&
                std::none_of(category.begin(), category.end(), [&](const auto& name) { return ResourceTextMatches(name, query); })) continue;
            InsertResourceTree(m_AnimationResourceTree, category, index);
        }
        FinalizeResourceTree(m_AnimationResourceTree);
        m_bAnimationResourceTreeDirty = false;
    }
    if (ImGui::BeginChild("##CompositionPhysicalAnimationTree", ImVec2(0.f, 260.f), true))
    {
        RenderResourceTree(m_AnimationResourceTree, [&](const std::size_t index)
        {
            const auto& resource = m_AnimationResources[index];
            ImGui::PushID(resource.strTargetAssetName.c_str());
            ImGui::PushID(resource.strSourceAssetId.c_str());
            ImGui::PushID(resource.strRuntimeClip.c_str());
            const bool selected = m_bHasSelectedAnimationResource &&
                SameAnimationResource(resource, m_SelectedAnimationResource);
            const auto& label = resource.strDisplayName.empty() ? resource.strRuntimeClip : resource.strDisplayName;
            if (ImGui::Selectable(label.c_str(), selected))
            {
                m_SelectedAnimationResource = resource;
                m_bHasSelectedAnimationResource = true;
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s\nModel: %s\nPackage: %s\nNative: %u ms\nDouble-click to preview",
                    resource.strRuntimeClip.c_str(), resource.strModelAssetId.c_str(),
                    resource.strSourceAssetId.c_str(), resource.iDurationMs);
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) Queue_AnimationPreview(resource);
            }
            ImGui::PopID(); ImGui::PopID(); ImGui::PopID();
        });
        if (m_AnimationResourceTree.iRecursiveLeafCount == 0u)
            ImGui::TextDisabled("No clips match. Refresh reads every installed Character and Boss model.");
    }
    ImGui::EndChild();
    if (m_bHasSelectedAnimationResource)
    {
        const auto& resource = m_SelectedAnimationResource;
        ImGui::TextWrapped("%s / %s | %u ms", resource.strTargetAssetName.c_str(),
            resource.strRuntimeClip.c_str(), resource.iDurationMs);
        if (ImGui::Button("Play Preview##CompositionPhysicalAnimation"))
            Queue_AnimationPreview(resource);
    }
    if (m_bHasSelectedAnimationResource || m_AnimationPreviewState.bPlaying)
    {
        ImGui::BeginDisabled(!m_AnimationPreviewState.bPlaying);
        if (ImGui::Button(m_AnimationPreviewState.bPaused ?
            "Resume##CompositionPhysicalAnimation" : "Pause##CompositionPhysicalAnimation"))
            m_eAnimationPreviewTransport = m_AnimationPreviewState.bPaused ?
                ANIMATION_PREVIEW_TRANSPORT::RESUME : ANIMATION_PREVIEW_TRANSPORT::PAUSE;
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Stop##CompositionPhysicalAnimation"))
            m_eAnimationPreviewTransport = ANIMATION_PREVIEW_TRANSPORT::STOP;
        if (!m_AnimationPreviewState.strPatternId.empty())
            ImGui::TextWrapped("%s: %.3f / %.3f s%s", m_AnimationPreviewState.strPatternId.c_str(),
                m_AnimationPreviewState.iClockMs / 1000.0, m_AnimationPreviewState.iDurationMs / 1000.0,
                m_AnimationPreviewState.bPaused ? " (paused)" : "");
        if (!m_AnimationPreviewState.strStatus.empty())
            ImGui::TextWrapped("%s", m_AnimationPreviewState.strStatus.c_str());
    }
    if (m_bHasSelectedAnimationResource)
    {
        const auto& resource = m_SelectedAnimationResource;
        std::string stageStatus;
        std::string rowStatus;
        const bool canStage = session.Can_AppendCompositionAnimationResource(resource, true, stageStatus);
        const bool canRow = session.Can_AppendCompositionAnimationResource(resource, false, rowStatus);
        const bool characterAction = m_eSelectedTarget == TARGET::CHARACTER;
        const char* stageLabel = characterAction ? "Replace selected Animation" : "Append as Stage";
        const char* rowLabel = characterAction ? "Append Animation" : "Add Animation Row";
        ImGui::BeginDisabled(!canStage);
        if (ImGui::Button((std::string(stageLabel) + "##CompositionPhysicalAnimation").c_str()))
            (void)session.Append_CompositionAnimationResource(resource, true, m_strAnimationBrowserStatus);
        ImGui::EndDisabled();
        ImGui::BeginDisabled(!canRow);
        if (ImGui::Button((std::string(rowLabel) + "##CompositionPhysicalAnimation").c_str()))
            (void)session.Append_CompositionAnimationResource(resource, false, m_strAnimationBrowserStatus);
        ImGui::EndDisabled();
        if (!canStage && !stageStatus.empty())
            ImGui::TextWrapped("%s: %s", stageLabel, stageStatus.c_str());
        if (!canRow && !rowStatus.empty() && rowStatus != stageStatus)
            ImGui::TextWrapped("%s: %s", rowLabel, rowStatus.c_str());
    }
    if (!m_strAnimationBrowserStatus.empty())
        ImGui::TextWrapped("%s", m_strAnimationBrowserStatus.c_str());
    if (!m_strAnimationResourceStatus.empty() && ImGui::TreeNode("Resource status##CompositionPhysicalAnimation"))
    {
        ImGui::TextWrapped("%s", m_strAnimationResourceStatus.c_str());
        ImGui::TreePop();
    }
    ImGui::Separator();
}

void Client::CSequencerTool::Render_WindowMenu()
{
    if (!ImGui::BeginMenuBar())
        return;
    if (ImGui::BeginMenu("Windows"))
    {
        for (const PANE pane : PANES)
            ImGui::MenuItem(PaneLabel(pane, m_bSequenceWorkspace), nullptr, &m_PaneVisible[static_cast<std::size_t>(pane)]);
        ImGui::MenuItem("Physical Animation Browser", nullptr, &m_bPhysicalAnimationBrowserVisible);
        ImGui::Separator();
        if (ImGui::MenuItem("Show All"))
            m_PaneVisible.fill(true);
        if (ImGui::MenuItem("Expand Resources"))
        {
            m_PaneVisible[static_cast<std::size_t>(PANE::RESOURCES)] = true;
            m_bExpandResourcesRequested = m_bFocusResourcesRequested = true;
            m_bSequencerMaximized = false;
        }
        ImGui::MenuItem("Maximize Sequencer", nullptr, &m_bSequencerMaximized);
        if (ImGui::MenuItem("Reset Window Layout"))
        {
            m_bResetLayoutRequested = true;
            m_bSequencerMaximized = false;
        }
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void Client::CSequencerTool::Render_ActionSelector()
{
    ImGui::SeparatorText("Composition Actions");
    constexpr std::array<const char*, 5> labels = { "Boss", "Character", "Object", "Sequence", "World" };
    for (std::size_t i = 0; i < labels.size(); ++i)
    {
        const auto target = static_cast<TARGET>(i);
        if (ImGui::Selectable(labels[i], m_eSelectedTarget == target))
        {
            m_ePendingTarget = target;
            m_bTargetChangePending = target != m_eSelectedTarget;
        }
    }
    ImGui::Separator();
    if (m_eSelectedTarget == TARGET::BOSS || m_eSelectedTarget == TARGET::SEQUENCE)
        Render_BossSelector();
}

void Client::CSequencerTool::Render_BossSelector()
{
    ImGui::SetNextItemWidth(200.f);
    if (ImGui::BeginCombo("Boss##CompositionWorkbenchBoss", BossLabel(Get_SelectedBoss())))
    {
        for (const BOSS boss : BOSS_ENTRIES)
        {
            const bool selected = boss == Get_SelectedBoss();
            if (ImGui::Selectable(BossLabel(boss), selected))
            {
                m_ePendingBoss = boss;
                m_bBossChangePending = true;
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

void Client::CSequencerTool::Apply_ViewRequest(ICompositionWorkbenchSession& session)
{
    const COMPOSITION_WORKBENCH_VIEW_REQUEST request = session.Consume_WorkbenchViewRequest();
    if (request.showResources || request.focusResources || request.expandResources)
        m_PaneVisible[static_cast<std::size_t>(PANE::RESOURCES)] = true;
    if (request.showPatterns || request.focusPatterns)
        m_PaneVisible[static_cast<std::size_t>(PANE::PATTERNS)] = true;
    m_bFocusResourcesRequested |= request.focusResources;
    m_bExpandResourcesRequested |= request.expandResources;
    m_bFocusPatternsRequested |= request.focusPatterns;
    if (request.maximizeSequencer)
    {
        m_bSequencerMaximized = true;
        m_PaneVisible[static_cast<std::size_t>(PANE::SEQUENCER)] = true;
    }
    if (request.restoreSequencer) m_bSequencerMaximized = false;
    m_bResetLayoutRequested |= request.resetLayout;
}

void Client::CSequencerTool::Render_Pane(
    ICompositionWorkbenchSession& session, const PANE pane)
{
    bool& visible = m_PaneVisible[static_cast<std::size_t>(pane)];
    if (!visible)
        return;
    const PANE_PLACEMENT placement = PanePlacement(pane);
    const ImGuiCond condition = m_bApplyResetLayoutThisFrame ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
    ImGui::SetNextWindowPos(placement.position, condition);
    ImGui::SetNextWindowSize(placement.size, condition);
    if (m_bRestoreAuthoringPanesRequested &&
        (pane == PANE::PATTERNS || pane == PANE::RESOURCES || pane == PANE::DETAILS))
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (pane == PANE::RESOURCES && m_bExpandResourcesRequested && nullptr != viewport)
    {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.08f,
            viewport->WorkPos.y + viewport->WorkSize.y * 0.12f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.62f,
            viewport->WorkSize.y * 0.72f), ImGuiCond_Always);
    }
    if ((pane == PANE::RESOURCES && m_bFocusResourcesRequested) ||
        (pane == PANE::PATTERNS && m_bFocusPatternsRequested))
    {
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
        ImGui::SetNextWindowFocus();
    }
    if (pane == PANE::SEQUENCER && m_bSequencerMaximized && nullptr != viewport)
    {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.02f,
            viewport->WorkPos.y + viewport->WorkSize.y * 0.02f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * 0.96f,
            viewport->WorkSize.y * 0.96f), ImGuiCond_Always);
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
    }
    const bool expanded = ImGui::Begin(PaneWindowId(pane, m_bSequenceWorkspace), &visible, ImGuiWindowFlags_MenuBar);
    Render_WindowMenu();
    if (expanded)
    {
        if (pane == PANE::PATTERNS) Render_ActionSelector();
        if (pane == PANE::RESOURCES && m_bPhysicalAnimationBrowserVisible && m_eSelectedTarget != TARGET::WORLD)
            Render_PhysicalAnimationBrowser(session);
        const bool hasBossOwner = m_eSelectedTarget == TARGET::BOSS || m_eSelectedTarget == TARGET::SEQUENCE;
        ImGui::PushID(static_cast<int>(m_eSelectedTarget) * 16 +
            (hasBossOwner ? static_cast<int>(Get_SelectedBoss()) : 0));
        session.Render_WorkbenchPane(pane);
        ImGui::PopID();
    }
    ImGui::End();
    if (pane == PANE::RESOURCES)
        m_bExpandResourcesRequested = m_bFocusResourcesRequested = false;
    if (pane == PANE::PATTERNS) m_bFocusPatternsRequested = false;
    Apply_ViewRequest(session);
}

void Client::CSequencerTool::Render()
{
    if (!m_bOpen)
        return;
    if (m_bTargetChangePending)
    {
        if (m_bBossChangePending) Open(m_ePendingTarget, m_ePendingBoss);
        else { m_bTargetChangePending = false; Select_Target(m_ePendingTarget); }
    }
    if (m_bBossChangePending)
    {
        m_bBossChangePending = false;
        Select_Boss(m_ePendingBoss);
    }
    m_bApplyResetLayoutThisFrame = m_bResetLayoutRequested;
    m_bResetLayoutRequested = false;
    const PANE_PLACEMENT placement = PanePlacement(PANE::TOOLBAR);
    const ImGuiCond condition = m_bApplyResetLayoutThisFrame ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
    ImGui::SetNextWindowPos(placement.position, condition);
    ImGui::SetNextWindowSize(placement.size, condition);
    if (m_bRestoreAuthoringPanesRequested)
    {
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
        ImGui::SetNextWindowFocus();
    }
    const bool expanded = ImGui::Begin(PaneWindowId(PANE::TOOLBAR, m_bSequenceWorkspace), &m_bOpen, ImGuiWindowFlags_MenuBar);
    Render_WindowMenu();
    if (expanded)
        ImGui::TextUnformatted("Select Boss, Character, Object, Sequence or World in Composition Actions.");
    ICompositionWorkbenchSession* const session = Selected_Session();
    if (!m_bOpen || nullptr == session)
    {
        if (expanded && nullptr == session)
            ImGui::TextDisabled("The selected action authoring session is unavailable.");
        if (!m_bOpen) Deactivate();
        ImGui::End();
        return;
    }
    /* The Map Tool may already own this session frame. Draw nothing that
       would open a second Begin/End pair for the same owner. */
    if (m_pExternallyHostedSession == session ||
        m_pExternallyHostedSessionAlt == session)
    {
        if (expanded)
            ImGui::TextDisabled("Map Tool is editing this Sequence this frame.");
        ImGui::End();
        return;
    }
    m_bInsideFrame = true;
    session->Begin_WorkbenchFrame();
    Apply_ViewRequest(*session);
    if (expanded)
    {
        ImGui::Separator();
        const bool hasBossOwner = m_eSelectedTarget == TARGET::BOSS || m_eSelectedTarget == TARGET::SEQUENCE;
        ImGui::PushID(static_cast<int>(m_eSelectedTarget) * 16 +
            (hasBossOwner ? static_cast<int>(Get_SelectedBoss()) : 0));
        session->Render_WorkbenchPane(PANE::TOOLBAR);
        ImGui::PopID();
        Apply_ViewRequest(*session);
    }
    ImGui::End();
    for (const PANE pane : PANES)
    {
        if (!m_bSequencerMaximized || pane == PANE::SEQUENCER)
            Render_Pane(*session, pane);
    }
    session->End_WorkbenchFrame();
    m_bInsideFrame = false;
    Apply_ViewRequest(*session);
    m_bApplyResetLayoutThisFrame = false;
    m_bRestoreAuthoringPanesRequested = false;
}
