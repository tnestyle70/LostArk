#include "imgui.h"
#include "BalanceTestPanel.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "NetworkPlayerCommandSink.h"
#include "PlayerSkillCatalog.h"

#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <iterator>

using namespace Client;

namespace
{
const char* ClassName(LostArk::Shared::CHARACTER_CLASS_ID value)
{
    using LostArk::Shared::CHARACTER_CLASS_ID;
    switch (value)
    {
    case CHARACTER_CLASS_ID::LANCE_MASTER: return "LANCE_MASTER";
    case CHARACTER_CLASS_ID::GUNSLINGER: return "GUNSLINGER";
    case CHARACTER_CLASS_ID::SLAYER: return "SLAYER";
    case CHARACTER_CLASS_ID::ARTIST: return "ARTIST";
    case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DIMENSIONMASTER";
    case CHARACTER_CLASS_ID::WARLORD: return "WARLORD";
    case CHARACTER_CLASS_ID::GUARDIANKNIGHT: return "GUARDIANKNIGHT";
    default: return "UNKNOWN";
    }
}
const char* DomainName(LostArk::Shared::BALANCE_DOMAIN value)
{
    using LostArk::Shared::BALANCE_DOMAIN;
    switch (value)
    {
    case BALANCE_DOMAIN::PLAYER: return "Players";
    case BALANCE_DOMAIN::SKILL: return "Skills";
    case BALANCE_DOMAIN::DAMAGE: return "Damage";
    case BALANCE_DOMAIN::BOSS: return "Bosses";
    case BALANCE_DOMAIN::MADNESS: return "Madness";
    case BALANCE_DOMAIN::STAGGER: return "Stagger";
    case BALANCE_DOMAIN::PATTERN_DAMAGE: return "Pattern damage";
    default: return "Pattern damage";
    }
}
const char* FieldHelp(const std::string& field)
{
    if (field == "maxHpDamagePercent") return "Target maximum-HP damage: 10 = 10% of maximum HP. Bypasses defense; invulnerability, shields and damage-taking buffs still affect the final HP loss.";
    if (field == "fixedDamage") return "Fixed HP damage before invulnerability, shields and damage-taking buffs. Defense is bypassed; integer range 1 to 1,000,000,000.";
    if (field == "attackCoefficientBp") return "Attack coefficient: 10000 basis points = 100% of attack power. Damage = attack x coefficient / 10000 + addend.";
    if (field == "damageAddend") return "Flat raw damage added after the attack coefficient. Defense, spread and critical rules are resolved by the Server.";
    if (field == "damageRatePercent") return "Legacy attack multiplier: 100 = 100%. Consumed only when coefficient and addend are both zero.";
    if (field == "damageSpreadPercent") return "Random damage spread in percent, applied by the Server.";
    if (field == "bossHealthBarDamage") return "Total boss health bars per ACTIVE cast, divided among its hits. ALT_V uses this separately from ordinary attack-power damage; missed, invulnerable or shielded hits still follow Server rules.";
    if (field == "staggerGaugeMaximum") return "This pattern's stagger gauge threshold. A higher value needs more stagger damage; this is not a global multiplier. Some published Retail thresholds require increments of 400; invalid values are rejected without rounding.";
    if (field == "staggerDamage") return "This skill's stagger contribution per admitted hit, consumed against the active pattern gauge.";
    if (field == "partDamage") return "This skill's part-destruction contribution per admitted hit.";
    if (field.find("Percent") != std::string::npos) return "Percent units: 10 = 10%, 100 = 100%.";
    if (field.ends_with("Ms")) return "Milliseconds: 1000 = one second. Current in-flight actions keep their admitted timing.";
    if (field == "maximumRange" || field == "movementDistance" || field.ends_with("RadiusM")) return "World distance in metres.";
    return nullptr;
}
bool Matches(const std::string& label, const char* filter)
{
    std::string text = label, query = filter;
    const auto lower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
    std::transform(text.begin(), text.end(), text.begin(), lower);
    std::transform(query.begin(), query.end(), query.begin(), lower);
    return text.find(query) != std::string::npos;
}
}


CBalanceTestPanel::CBalanceTestPanel() : m_sink(std::make_unique<CNetworkPlayerCommandSink>())
{ m_status = "Connect to the Server to read its active numeric balance."; }
CBalanceTestPanel::~CBalanceTestPanel() = default;

bool CBalanceTestPanel::Is_Dirty() const
{
    for (const auto& document : m_documents)
        for (const auto& row : document.rows)
            for (const auto& field : row.fields)
                if (field.original != field.value) return true;
    return false;
}

void CBalanceTestPanel::Install_Snapshot(const LostArk::Shared::GameplayDataRevision& revision,
    const std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>& entries)
{
    using namespace LostArk::Shared;
    std::vector<DOCUMENT> documents;
    for (unsigned domain = 0; domain < static_cast<unsigned>(BALANCE_DOMAIN::END); ++domain)
    {
        DOCUMENT document; document.domain = static_cast<BALANCE_DOMAIN>(domain); document.label = DomainName(document.domain);
        for (const auto& entry : entries)
        {
            if (entry.eDomain != document.domain) continue;
            auto found = std::find_if(document.rows.begin(), document.rows.end(), [&](const auto& row) { return row.id == entry.strId; });
            if (found == document.rows.end())
            {
                ROW row; row.id = row.label = entry.strId;
                if (entry.eDomain == BALANCE_DOMAIN::SKILL)
                    for (const auto& skill : CPlayerSkillCatalog::Get_Skills())
                        if (std::to_string(skill.iSkillId) == entry.strId)
                        {
                            row.label = std::string(ClassName(skill.eCharacterClass)) + " [" + skill.strInputSlot + "] " + skill.strDisplayName + " (" + row.id + ")";
                            row.damageProfileId = skill.strDamageProfileId; row.isAltV = skill.strInputSlot == "ALT_V";
                        }
                document.rows.push_back(std::move(row)); found = std::prev(document.rows.end());
            }
            found->fields.push_back({ entry.strField, entry.fValue, entry.fValue, entry.isIntegral });
        }
        if (!document.rows.empty()) documents.push_back(std::move(document));
    }
    m_documents = std::move(documents); m_revision = revision; m_observedRevision = revision;
    if (m_document >= m_documents.size()) m_document = 0;
    m_row = 0; m_appliedRevision = {};
}

bool CBalanceTestPanel::Reload()
{
    m_documents.clear(); m_revision = {}; m_observedRevision = {}; m_appliedRevision = {};
    m_status = m_sink->Request_BalanceRefresh() ? "Reading active numbers from the Server..." : "Connect to the Server first.";
    return true;
}

void CBalanceTestPanel::Save_AndApply()
{
    using namespace LostArk::Shared;
    if (m_pending || !m_revision.Is_Valid() || !Is_Dirty()) return;
    C2S_BALANCE_PATCH request;
    if (++m_sequence == 0u || m_sequence >= 0x80000000u) m_sequence = 1u;
    request.iRequestSequence = m_sequence; request.BaseNumericRevision = m_revision;
    for (const auto& document : m_documents)
        for (const auto& row : document.rows)
            for (const auto& field : row.fields)
            {
                if (field.original == field.value) continue;
                if (!std::isfinite(field.value) || (field.integral && std::floor(field.value) != field.value))
                { m_status = "Enter a finite whole number for " + field.name; return; }
                if (request.Changes.size() == MAX_BALANCE_CHANGES) { m_status = "Apply at most 128 edited fields at once."; return; }
                BALANCE_NUMERIC_CHANGE value;
                value.eDomain = document.domain; value.strId = row.id; value.strField = field.name;
                value.fBefore = field.original; value.fValue = field.value; request.Changes.push_back(std::move(value));
            }
    if (!m_sink->Request_BalancePatch(request)) { m_status = "Save was not sent. Connect to the Server; your draft is preserved."; return; }
    m_pending = request.iRequestSequence; m_submittedAt = GetTickCount64();
    m_status = "APPLY PENDING: Server validating, saving, publishing and activating the numeric patch...";
}

void CBalanceTestPanel::Update()
{
    using namespace LostArk::Shared;
    S2C_BALANCE_RESULT result;
    while (m_sink->Consume_BalanceResult(result))
    {
        if (result.iRequestSequence != m_pending) continue;
        m_pending = 0;
        if (result.eResult == BALANCE_APPLY_RESULT::APPLIED)
        {
            m_appliedRevision = result.ActiveNumericRevision; m_observedRevision = {};
            for (auto& document : m_documents)
                for (auto& row : document.rows)
                    for (auto& field : row.fields) field.original = field.value;
            m_status = "SAVED AND APPLIED on the Server. Refreshing the shared numeric view; no restart is needed.";
        }
        else m_status = "NOT APPLIED: " + result.strReason + " Your numeric draft is preserved.";
    }
    if (m_pending && GetTickCount64() - m_submittedAt > 60000u)
    {
        m_pending = 0; m_sink->Request_BalanceRefresh();
        m_status = "Save result has not arrived. Refresh requested; your draft is preserved. Check Server diagnostics before retrying.";
    }
    auto observed = m_observedRevision;
    std::vector<BALANCE_NUMERIC_ENTRY> entries;
    if (m_sink->Copy_BalanceSnapshot(observed, entries))
    {
        m_observedRevision = observed;
        if (!Is_Dirty() || (m_appliedRevision.Is_Valid() && observed == m_appliedRevision))
        {
            const bool applied = m_appliedRevision.Is_Valid();
            Install_Snapshot(observed, entries);
            if (!m_pending) m_status = applied ? "SAVED AND APPLIED: active Server values are shared by every connected player." :
                "Active Server values loaded. Save + Apply validates and activates for every player without restarting.";
        }
        else if (!m_pending)
            m_status = "The Server values changed while this draft was edited. Draft preserved; Reload Server Values before a fresh edit.";
    }
}

void CBalanceTestPanel::Render_KillBossControl()
{
    using namespace LostArk::Shared;
    static CNetworkPlayerCommandSink sink;
    static std::uint32_t sequence = 0, pending = 0;
    static ULONGLONG submittedAt = 0;
    static std::string status;
    const auto level = CGameInstance::Get().Get_CurrentLevelID();
    const auto world = level == ETOUI(LEVEL::VALTAN_ARENA) ? WORLD_ID::VALTAN_ARENA :
        (level == ETOUI(LEVEL::KAKULSAYDON_ARENA) ? WORLD_ID::KAKULSAYDON_ARENA : WORLD_ID::END);
    const auto& boss = CCombatHUDViewModel::Get().Get_Boss();
    S2C_DEBUG_KILL_GATE_BOSSES_RESULT result;
    while (sink.Consume_DebugKillGateBossesResult(result))
    {
        if (result.iRequestSequence != pending) continue;
        pending = 0;
        constexpr const char* messages[] = { "Accepted; normal gate death/clear progression continues.", "Kill Boss is unavailable.",
            "Wrong world.", "Player is not in this room.", "Stale request ignored.", "No live boss in the current gate.",
            "The displayed boss changed; wait for the current snapshot.", "Wait until the gate is in combat." };
        const auto index = static_cast<std::size_t>(result.eResult);
        status = index < std::size(messages) ? messages[index] : "Unknown result.";
    }
    if (pending && GetTickCount64() - submittedAt > 5000u)
    { pending = 0; status = "No result received. Check the connection and current boss before retrying."; }
    ImGui::BeginDisabled(world == WORLD_ID::END || !boss.isValid || !boss.iCurrentHp || pending);
    if (ImGui::Button("Kill Current Gate Boss"))
    {
        if (++sequence == 0u) ++sequence;
        const C2S_DEBUG_KILL_GATE_BOSSES request{ sequence, world, boss.strArchetypeId };
        if (sink.Request_DebugKillGateBosses(request))
        { pending = sequence; submittedAt = GetTickCount64(); status = "Requested Server death for the current gate boss."; }
        else status = "Could not submit Kill Boss; connect to the Server first.";
    }
    ImGui::EndDisabled();
    if (!status.empty()) ImGui::TextWrapped("%s", status.c_str());
}

void CBalanceTestPanel::Render(bool& open)
{
    Update();
    ImGui::SetNextWindowSize(ImVec2(1040.f, 720.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Balance Test", &open)) { ImGui::End(); return; }
    ImGui::TextWrapped("Shared Server balance. Save + Apply validates, saves, publishes and activates numeric changes for every connected player. No restart is needed.");
    ImGui::BeginDisabled(m_pending != 0u);
    if (ImGui::Button("Reload Server Values")) { if (Is_Dirty()) m_confirmReload = true; else Reload(); }
    ImGui::SameLine(); ImGui::BeginDisabled(!Is_Dirty());
    if (ImGui::Button("Save + Apply")) Save_AndApply();
    ImGui::EndDisabled();
    ImGui::SameLine(); ImGui::TextDisabled("%s", Is_Dirty() ? "UNSAVED" : "server values");
    if (m_confirmReload) { ImGui::OpenPopup("Discard numeric draft?"); m_confirmReload = false; }
    if (ImGui::BeginPopupModal("Discard numeric draft?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Reload replaces this panel's unsaved numeric draft.");
        if (ImGui::Button("Discard and Reload")) { Reload(); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::TextWrapped("%s", m_status.c_str());
    if (!m_documents.empty())
    {
        for (std::size_t i = 0; i < m_documents.size(); ++i)
        {
            if (i) ImGui::SameLine();
            if (ImGui::Selectable(m_documents[i].label.c_str(), i == m_document, 0, ImVec2(110.f, 0.f)))
            { m_document = i; m_row = 0; }
        }
        auto& document = m_documents[m_document];
        ImGui::InputTextWithHint("##BalanceFilter", "Search class, skill, slot, ID or pattern", m_filter, sizeof(m_filter));
        if (document.domain == LostArk::Shared::BALANCE_DOMAIN::SKILL)
        { ImGui::SameLine(); ImGui::SetNextItemWidth(150.f); ImGui::Combo("##SkillFilter", &m_skillFilter, "All skills\0Normal / V\0ALT_V only\0"); }
        ImGui::BeginChild("NumericTargets", ImVec2(400.f, -135.f), true);
        for (std::size_t i = 0; i < document.rows.size(); ++i)
        {
            const auto& candidate = document.rows[i];
            if (!Matches(candidate.label, m_filter)) continue;
            if (document.domain == LostArk::Shared::BALANCE_DOMAIN::SKILL &&
                ((m_skillFilter == 1 && candidate.isAltV) || (m_skillFilter == 2 && !candidate.isAltV))) continue;
            if (ImGui::Selectable(candidate.label.c_str(), i == m_row)) m_row = i;
        }
        ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("NumericFields", ImVec2(0.f, -135.f), true);
        if (m_row < document.rows.size())
        {
            auto& row = document.rows[m_row];
            ImGui::TextWrapped("%s", row.label.c_str());
            if (document.label == "Madness")
            {
                ImGui::TextWrapped("Damage gain = maximum gauge x actual HP lost / maximum HP x damageGainPercent / 100.");
                ImGui::TextWrapped("Special pulse = maximum gauge x sourceGainPercent / 100 x sourceMultiplierPercent / 100. Interval is milliseconds; 100%% multiplier = x1.");
                ImGui::TextWrapped("Total = clamp(current + damage gain + special pulses + authored mechanic gains, 0, maximum). Fractional gains accumulate; a full gauge transforms to Clown. Clown/Mario players receive no gain.");
                ImGui::TextWrapped("Native reference: maximum 100, Clown hold 15s, NPC aura +10 per 1s in 2m. Doll breath reference: 4m / 30 degrees.");
                ImGui::TextWrapped("PROJECT_TUNED: damage conversion (100%%), doll charge (10%%), requested ball/doll x2. Doll uses two sectors in the saved object basis; this is not a bone-accurate flame collider.");
                ImGui::Separator();
            }
            const auto renderFields = [](ROW& target)
            {
                ImGui::PushID(target.id.c_str());
                for (auto& field : target.fields)
                {
                    ImGui::InputDouble(field.name.c_str(), &field.value, field.integral ? 1.0 : 0.1, field.integral ? 100.0 : 1.0,
                        field.integral ? "%.0f" : "%.4f");
                    if (const char* help = FieldHelp(field.name)) ImGui::TextWrapped("%s", help);
                    if (field.value != field.original) ImGui::TextDisabled("Server baseline: %.6g", field.original);
                }
                ImGui::PopID();
            };
            if (document.domain == LostArk::Shared::BALANCE_DOMAIN::PATTERN_DAMAGE)
                ImGui::TextWrapped("O | encounter | pattern | logic | result | ordinal; H | encounter | pattern | window | collider kind | set | hit. The selected damage mode and collider remain unchanged.");
            renderFields(row);
            if (document.domain == LostArk::Shared::BALANCE_DOMAIN::SKILL && !row.damageProfileId.empty())
            {
                ImGui::SeparatorText(row.isAltV ? "ALT_V damage profile" : "Connected damage profile");
                ImGui::TextWrapped("%s (shared profile edits affect every skill using this ID)", row.damageProfileId.c_str());
                for (auto& damageDocument : m_documents)
                    if (damageDocument.domain == LostArk::Shared::BALANCE_DOMAIN::DAMAGE)
                        for (auto& damageRow : damageDocument.rows)
                            if (damageRow.id == row.damageProfileId) renderFields(damageRow);
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndDisabled();
    ImGui::SeparatorText("Live Server / Test Actions");
    const auto& player = CCombatHUDViewModel::Get().Get_Player();
    const auto& boss = CCombatHUDViewModel::Get().Get_Boss();
    ImGui::Text("Player HP %u / %u | Boss HP %u / %u | Server tick %u", player.iCurrentHp, player.iMaximumHp,
        boss.iCurrentHp, boss.iMaximumHp, player.iServerTick);
    ImGui::Text("Server Madness: %u / %u | %s", player.iCurrentMadness, player.iMaximumMadness,
        player.eMadnessForm == LostArk::Shared::PLAYER_MADNESS_FORM::CLOWN ? "Clown" : "Normal");
    Render_CooldownControl();
    Render_KillBossControl();
    ImGui::End();
}


void CBalanceTestPanel::Render_CooldownControl()
{
    using namespace LostArk::Shared;
    static CNetworkPlayerCommandSink sink;
    static std::uint32_t sequence = 0, pending = 0;
    static ULONGLONG submittedAt = 0;
    static std::string status;
    const auto& player = CCombatHUDViewModel::Get().Get_Player();
    WORLD_ID world = WORLD_ID::END;
    switch (static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID()))
    {
    case LEVEL::BERN: world = WORLD_ID::BERN; break;
    case LEVEL::VALTAN_ARENA: world = WORLD_ID::VALTAN_ARENA; break;
    case LEVEL::KAKULSAYDON_ARENA: world = WORLD_ID::KAKULSAYDON_ARENA; break;
    case LEVEL::CHARACTER_SELECT: world = WORLD_ID::CHARACTER_SELECT_ARENA; break;
    case LEVEL::DEVELOPMENT: world = WORLD_ID::TRAINING_GROUND; break;
    case LEVEL::MAHARAKA: world = WORLD_ID::MAHARAKA; break;
    default: break;
    }
    S2C_SET_COOLDOWN_MODE_RESULT result;
    while (sink.Consume_SetCooldownModeResult(result))
    {
        if (result.iRequestSequence != pending) continue;
        pending = 0;
        status = result.eResult == SET_COOLDOWN_MODE_RESULT::ACCEPTED ?
            "Server accepted the room cooldown policy for every player." : "Server rejected the policy request; room state was preserved.";
    }
    if (pending && GetTickCount64() - submittedAt > 5000u)
    { pending = 0; status = "No policy response received. Check the current Server connection."; }
    ImGui::Text("Room cooldown: %s", !player.isValid ? "Waiting for Server" :
        (player.eCooldownMode == COOLDOWN_MODE::DEBUG_THREE_SECONDS ? "Debug (3s)" : "Release (Retail)"));
    ImGui::BeginDisabled(!player.isValid || world == WORLD_ID::END || pending);
    const auto submit = [&](const COOLDOWN_MODE mode) {
        if (++sequence == 0u) ++sequence;
        if (sink.Request_SetCooldownMode({ sequence, world, mode }))
        { pending = sequence; submittedAt = GetTickCount64(); status = "Requesting room policy..."; }
        else status = "Could not submit policy to the Server.";
    };
    if (ImGui::Button("Debug (3s)")) submit(COOLDOWN_MODE::DEBUG_THREE_SECONDS);
    ImGui::SameLine();
    if (ImGui::Button("Release (Retail)")) submit(COOLDOWN_MODE::RELEASE_AUTHORED);
    ImGui::EndDisabled();
    ImGui::TextDisabled("Runtime policy only; zero-cooldown attacks stay at zero. Retail ALT_V: 300s.");
    if (!status.empty()) ImGui::TextWrapped("%s", status.c_str());
}
