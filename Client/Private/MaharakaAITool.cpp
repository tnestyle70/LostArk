#include "MaharakaAITool.h"
#ifdef _DEBUG
#include "PlayerCommandSink.h"
#include "imgui.h"
#include <utility>

namespace Client
{
using namespace LostArk::Shared;
namespace
{
bool SameValues(const MAHARAKA_AI_TUNING& a, const MAHARAKA_AI_TUNING& b)
{
    return a.iBotCount == b.iBotCount && a.iDecisionTicks == b.iDecisionTicks &&
        a.iMoveRetargetTicks == b.iMoveRetargetTicks && a.iSkillIntervalTicks == b.iSkillIntervalTicks &&
        a.fTargetRangeM == b.fTargetRangeM && a.fMoveProbability == b.fMoveProbability &&
        a.fAggression == b.fAggression && a.fKnockbackRangeM == b.fKnockbackRangeM &&
        a.iKnockbackMs == b.iKnockbackMs;
}

void NumberRow(const char* label, std::uint32_t& draft, const std::uint32_t applied,
    const int minimum, const int maximum, const bool hasServer)
{
    ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1); ImGui::PushID(label); ImGui::SetNextItemWidth(-1.f);
    int value = static_cast<int>(draft);
    if (ImGui::SliderInt("##draft", &value, minimum, maximum)) draft = static_cast<std::uint32_t>(value);
    ImGui::PopID(); ImGui::TableSetColumnIndex(2);
    if (hasServer) ImGui::Text("%u", applied); else ImGui::TextUnformatted("--");
}

void NumberRow(const char* label, float& draft, const float applied,
    const float minimum, const float maximum, const bool hasServer)
{
    ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1); ImGui::PushID(label); ImGui::SetNextItemWidth(-1.f);
    ImGui::SliderFloat("##draft", &draft, minimum, maximum, "%.2f");
    ImGui::PopID(); ImGui::TableSetColumnIndex(2);
    if (hasServer) ImGui::Text("%.2f", applied); else ImGui::TextUnformatted("--");
}
}

CMaharakaAITool::CMaharakaAITool(std::shared_ptr<IPlayerCommandSink> sink) : m_sink(std::move(sink)) {}

bool CMaharakaAITool::Is_Dirty() const
{
    return m_hasDraft && !SameValues(m_base, m_draft);
}

bool CMaharakaAITool::Is_Stale() const
{
    return !m_hasServer || m_base.iRevision != m_server.iRevision || !SameValues(m_base, m_server);
}

void CMaharakaAITool::Set_Active(const bool active)
{
    if (active == m_active) return;
    m_active = active;
    m_hasServer = false;
    m_pending = 0u;
    m_status = active ? "Refresh to read the Server's AI settings. Your draft is preserved." :
        "Maharaka Server connection is unavailable. Your draft is preserved.";
    if (active && m_open) Submit(MAHARAKA_AI_OPERATION::GET);
}

void CMaharakaAITool::Open()
{
    m_open = true;
    if (m_active && !m_pending) Submit(MAHARAKA_AI_OPERATION::GET);
}

bool CMaharakaAITool::Consume_InteractionRequest()
{
    const bool result = m_interaction;
    m_interaction = false;
    return result;
}

void CMaharakaAITool::Submit(const MAHARAKA_AI_OPERATION operation)
{
    if (!m_active || m_pending || !m_sink) return;
    if (operation != MAHARAKA_AI_OPERATION::GET &&
        (!m_hasDraft || Is_Stale() || !Is_Valid_MaharakaAITuning(m_draft)))
    {
        m_status = "Refresh and review the draft and its revision before applying. Draft preserved.";
        return;
    }
    C2S_MAHARAKA_AI_TUNING request{};
    if (0u == ++m_sequence) ++m_sequence;
    request.iRequestSequence = m_sequence;
    request.iExpectedRevision = m_hasDraft ? m_base.iRevision : 0u;
    request.eOperation = operation;
    request.Tuning = m_draft;
    if (!m_sink->Request_MaharakaAITuning(request))
    {
        m_status = "Request was not sent. Check the Maharaka Server connection; draft preserved.";
        return;
    }
    m_pending = request.iRequestSequence;
    m_operation = operation;
    m_submittedAt = std::chrono::steady_clock::now();
    m_status = operation == MAHARAKA_AI_OPERATION::GET ? "Reading active settings from the Server..." :
        operation == MAHARAKA_AI_OPERATION::SAVE ? "Server Save + Apply pending..." : "Server Apply pending...";
}

void CMaharakaAITool::Update()
{
    S2C_MAHARAKA_AI_TUNING result{};
    while (m_sink && m_sink->Consume_MaharakaAITuning(result))
    {
        if (!m_pending || result.iRequestSequence != m_pending) continue;
        m_pending = 0u;
        m_hasServer = result.eResult != MAHARAKA_AI_RESULT::WRONG_WORLD;
        if (m_hasServer) m_server = result.Tuning;
        if (result.eResult == MAHARAKA_AI_RESULT::ACCEPTED)
        {
            if (m_operation != MAHARAKA_AI_OPERATION::GET || !Is_Dirty())
            {
                m_base = m_draft = m_server;
                m_hasDraft = true;
            }
            m_status = result.strStatus;
            if (m_operation == MAHARAKA_AI_OPERATION::GET && Is_Dirty())
                m_status += " Edited draft preserved.";
        }
        else
        {
            m_status = result.eResult == MAHARAKA_AI_RESULT::REVISION_CONFLICT ? "Revision conflict: " : "Not applied: ";
            m_status += result.strStatus + " Draft preserved.";
        }
    }
    if (m_pending && std::chrono::steady_clock::now() - m_submittedAt > std::chrono::seconds(10))
    {
        m_pending = 0u;
        m_hasServer = false;
        m_status = "Response timed out. The outcome is unknown; Refresh before retrying. Draft preserved.";
    }
}

void CMaharakaAITool::Rebase_Draft()
{
    if (!m_hasServer || !m_hasDraft || m_pending) return;
    const auto merge = [&](auto member)
    {
        if (m_draft.*member == m_base.*member) m_draft.*member = m_server.*member;
    };
    merge(&MAHARAKA_AI_TUNING::iBotCount);
    merge(&MAHARAKA_AI_TUNING::iDecisionTicks);
    merge(&MAHARAKA_AI_TUNING::iMoveRetargetTicks);
    merge(&MAHARAKA_AI_TUNING::iSkillIntervalTicks);
    merge(&MAHARAKA_AI_TUNING::fTargetRangeM);
    merge(&MAHARAKA_AI_TUNING::fMoveProbability);
    merge(&MAHARAKA_AI_TUNING::fAggression);
    merge(&MAHARAKA_AI_TUNING::fKnockbackRangeM);
    merge(&MAHARAKA_AI_TUNING::iKnockbackMs);
    m_base = m_server;
    m_draft.iRevision = m_server.iRevision;
    m_status = "Rebased onto the observed Server revision. Edited fields kept; review before applying.";
}

void CMaharakaAITool::Render()
{
    if (!m_open) return;
    ImGui::SetNextWindowSize(ImVec2(750.f, 560.f), ImGuiCond_FirstUseEver);
    const bool expanded = ImGui::Begin("Waterpang AI Tool", &m_open);
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
        ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) m_interaction = true;
    if (!expanded) { ImGui::End(); return; }
    ImGui::TextWrapped("AI settings are owned by the Maharaka Server. Apply changes the active room; Save + Apply also saves the Server source.");
    ImGui::Text("Server revision: %u | Draft base: %u | %s", m_hasServer ? m_server.iRevision : 0u,
        m_hasDraft ? m_base.iRevision : 0u, Is_Dirty() ? "Unsaved draft" : "No draft changes");
    ImGui::BeginDisabled(!m_active || m_pending != 0u);
    if (ImGui::Button("Refresh")) Submit(MAHARAKA_AI_OPERATION::GET);
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_hasServer || !m_hasDraft || Is_Stale() || !Is_Valid_MaharakaAITuning(m_draft));
    if (ImGui::Button("Apply")) Submit(MAHARAKA_AI_OPERATION::APPLY);
    ImGui::SameLine();
    if (ImGui::Button("Save + Apply")) Submit(MAHARAKA_AI_OPERATION::SAVE);
    ImGui::EndDisabled();
    ImGui::BeginDisabled(!m_hasServer);
    if (ImGui::Button("Discard draft / Use Server"))
    {
        m_base = m_draft = m_server;
        m_hasDraft = true;
        m_status = "Draft reset to the observed Server values.";
    }
    if (m_hasDraft && Is_Stale() && m_hasServer)
    {
        ImGui::SameLine();
        if (ImGui::Button("Rebase edited fields")) Rebase_Draft();
        ImGui::TextWrapped("Server values changed. Refresh preserves your draft; Rebase keeps your edited fields and updates the other fields from the Server.");
    }
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    ImGui::Separator();
    ImGui::BeginDisabled(!m_active || !m_hasDraft || m_pending != 0u);
    if (ImGui::BeginTable("aiValues", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Setting", ImGuiTableColumnFlags_WidthStretch, 1.2f);
        ImGui::TableSetupColumn("Draft", ImGuiTableColumnFlags_WidthStretch, 1.5f);
        ImGui::TableSetupColumn("Applied on Server", ImGuiTableColumnFlags_WidthStretch, 1.f);
        ImGui::TableHeadersRow();
        NumberRow("AI players", m_draft.iBotCount, m_server.iBotCount, 0, 20, m_hasServer);
        NumberRow("Decision interval (ticks)", m_draft.iDecisionTicks, m_server.iDecisionTicks, 1, 300, m_hasServer);
        NumberRow("Move retarget (ticks)", m_draft.iMoveRetargetTicks, m_server.iMoveRetargetTicks, 1, 900, m_hasServer);
        NumberRow("Skill interval (ticks)", m_draft.iSkillIntervalTicks, m_server.iSkillIntervalTicks, 6, 1800, m_hasServer);
        NumberRow("Target range (m)", m_draft.fTargetRangeM, m_server.fTargetRangeM, 1.f, 30.f, m_hasServer);
        NumberRow("Move probability", m_draft.fMoveProbability, m_server.fMoveProbability, 0.f, 1.f, m_hasServer);
        NumberRow("Attack probability", m_draft.fAggression, m_server.fAggression, 0.f, 1.f, m_hasServer);
        NumberRow("Knockback range (m)", m_draft.fKnockbackRangeM, m_server.fKnockbackRangeM, 0.f, 12.f, m_hasServer);
        NumberRow("Knockback duration (ms)", m_draft.iKnockbackMs, m_server.iKnockbackMs, 1, 3000, m_hasServer);
        ImGui::EndTable();
    }
    ImGui::EndDisabled();
    if (m_hasDraft && m_draft.iMoveRetargetTicks < m_draft.iDecisionTicks)
        ImGui::TextWrapped("Move retarget interval must be at least the decision interval.");
    ImGui::TextWrapped("%s", m_status.c_str());
    ImGui::End();
}
}
#endif
