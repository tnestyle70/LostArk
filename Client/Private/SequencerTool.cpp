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

    constexpr std::array<PANE, 6u> PANES = {
        PANE::SEQUENCER, PANE::PATTERNS, PANE::RESOURCES,
        PANE::DETAILS, PANE::PREVIEW, PANE::BOSS_PATTERN };

    const char* BossLabel(const BOSS boss)
    {
        switch (boss)
        {
        case BOSS::VALTAN: return "Valtan";
        case BOSS::KOUKU_SAYDON: return "KoukuSaydon";
        default: return "Unavailable boss";
        }
    }

    const char* PaneLabel(const PANE pane)
    {
        switch (pane)
        {
        case PANE::SEQUENCER: return "Sequencer";
        case PANE::PATTERNS: return "Patterns";
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

    const char* PaneWindowId(const PANE pane)
    {
        switch (pane)
        {
        case PANE::SEQUENCER:
            return "Composition Sequencer###CompositionSequencerWindowResizableV3";
        case PANE::PATTERNS:
            return "Composition Patterns###CompositionPatternsWindow";
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
    ICompositionWorkbenchSession* pKoukuSaydonSession)
    : m_pValtanSession(pValtanSession)
    , m_pKoukuSaydonSession(pKoukuSaydonSession)
{
}

Client::ICompositionWorkbenchSession* Client::CSequencerTool::Selected_Session() const noexcept
{
    switch (m_eSelectedBoss)
    {
    case BOSS::VALTAN: return m_pValtanSession;
    case BOSS::KOUKU_SAYDON: return m_pKoukuSaydonSession;
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
    if (!ImGui::CollapsingHeader("Physical Animation##CompositionPhysicalAnimation",
        ImGuiTreeNodeFlags_DefaultOpen))
        return;
    if (ImGui::Button("Refresh##CompositionPhysicalAnimation"))
        m_bResourceRefreshRequested = true;
    ImGui::SameLine();
    ImGui::TextDisabled("6 bodies / %zu clips", m_AnimationResources.size());
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##CompositionPhysicalAnimationSearch", "Search body, clip or package...",
        m_AnimationResourceSearch.data(), m_AnimationResourceSearch.size());
    const std::string_view query = m_AnimationResourceSearch.data();
    if (ImGui::BeginChild("##CompositionPhysicalAnimationTree", ImVec2(0.f, 200.f), true))
    {
        for (const char* target : COMPOSITION_ANIMATION_TARGET_ASSET_NAMES)
        {
            std::size_t matching = 0u;
            const auto matches = [target, query](const COMPOSITION_ANIMATION_RESOURCE& resource) {
                return resource.strTargetAssetName == target &&
                    (ResourceTextMatches(target, query) || ResourceTextMatches(resource.strRuntimeClip, query) ||
                     ResourceTextMatches(resource.strSourceAssetId, query));
            };
            for (const auto& resource : m_AnimationResources)
                if (matches(resource)) ++matching;
            ImGui::PushID(target);
            if (ImGui::TreeNodeEx("##Body", ImGuiTreeNodeFlags_None, "%s (%zu)", target, matching))
            {
                for (const auto& resource : m_AnimationResources)
                {
                    if (!matches(resource)) continue;
                    ImGui::PushID(resource.strSourceAssetId.c_str());
                    ImGui::PushID(resource.strRuntimeClip.c_str());
                    const bool selected = m_bHasSelectedAnimationResource &&
                        SameAnimationResource(resource, m_SelectedAnimationResource);
                    if (ImGui::Selectable(resource.strRuntimeClip.c_str(), selected))
                    {
                        m_SelectedAnimationResource = resource;
                        m_bHasSelectedAnimationResource = true;
                        Queue_AnimationPreview(resource);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s\nModel: %s\nPackage: %s\nNative: %u ms",
                            resource.strRuntimeClip.c_str(), resource.strModelAssetId.c_str(),
                            resource.strSourceAssetId.c_str(), resource.iDurationMs);
                    ImGui::PopID();
                    ImGui::PopID();
                }
                if (matching == 0u)
                    ImGui::TextDisabled("No matching clips in the current metadata.");
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
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
        ImGui::BeginDisabled(!canStage);
        if (ImGui::Button("Append as Stage##CompositionPhysicalAnimation"))
            (void)session.Append_CompositionAnimationResource(resource, true, m_strAnimationBrowserStatus);
        ImGui::EndDisabled();
        ImGui::BeginDisabled(!canRow);
        if (ImGui::Button("Add Animation Row##CompositionPhysicalAnimation"))
            (void)session.Append_CompositionAnimationResource(resource, false, m_strAnimationBrowserStatus);
        ImGui::EndDisabled();
        if (!canStage && !stageStatus.empty())
            ImGui::TextWrapped("Append as Stage: %s", stageStatus.c_str());
        if (!canRow && !rowStatus.empty() && rowStatus != stageStatus)
            ImGui::TextWrapped("Add Animation Row: %s", rowStatus.c_str());
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
            ImGui::MenuItem(PaneLabel(pane), nullptr, &m_PaneVisible[static_cast<std::size_t>(pane)]);
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

void Client::CSequencerTool::Render_BossSelector()
{
    ImGui::SetNextItemWidth(200.f);
    if (ImGui::BeginCombo("Boss##CompositionWorkbenchBoss", BossLabel(m_eSelectedBoss)))
    {
        for (const BOSS boss : { BOSS::VALTAN, BOSS::KOUKU_SAYDON })
        {
            const bool selected = boss == m_eSelectedBoss;
            if (ImGui::Selectable(BossLabel(boss), selected))
                m_eSelectedBoss = boss;
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
    const bool expanded = ImGui::Begin(PaneWindowId(pane), &visible, ImGuiWindowFlags_MenuBar);
    Render_WindowMenu();
    if (expanded)
    {
        if (pane == PANE::RESOURCES)
            Render_PhysicalAnimationBrowser(session);
        ImGui::PushID(static_cast<int>(m_eSelectedBoss));
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
    m_bApplyResetLayoutThisFrame = m_bResetLayoutRequested;
    m_bResetLayoutRequested = false;
    const PANE_PLACEMENT placement = PanePlacement(PANE::TOOLBAR);
    const ImGuiCond condition = m_bApplyResetLayoutThisFrame ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
    ImGui::SetNextWindowPos(placement.position, condition);
    ImGui::SetNextWindowSize(placement.size, condition);
    const bool expanded = ImGui::Begin(PaneWindowId(PANE::TOOLBAR), &m_bOpen, ImGuiWindowFlags_MenuBar);
    Render_WindowMenu();
    if (expanded)
        Render_BossSelector();
    ICompositionWorkbenchSession* const session = Selected_Session();
    if (!m_bOpen || nullptr == session)
    {
        if (expanded && nullptr == session)
            ImGui::TextDisabled("The selected boss authoring session is unavailable.");
        ImGui::End();
        return;
    }
    session->Begin_WorkbenchFrame();
    Apply_ViewRequest(*session);
    if (expanded)
    {
        ImGui::Separator();
        ImGui::PushID(static_cast<int>(m_eSelectedBoss));
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
    Apply_ViewRequest(*session);
    m_bApplyResetLayoutThisFrame = false;
}
