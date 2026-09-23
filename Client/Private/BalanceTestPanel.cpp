#include "imgui.h"
#include "BalanceTestPanel.h"
#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "NetworkPlayerCommandSink.h"
#include "ProjectDataRoot.h"

#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string_view>

using namespace Client;
namespace
{
    struct BALANCE_DOMAIN { const char* file; const char* array; const char* key; const char* label; const char* fields; const char* profileArray; const char* profileFields; };
    constexpr BALANCE_DOMAIN DOMAINS[] = {
        { "PlayerProfiles.json", "players", "characterClass", "Players", "|maximumHp|maximumResource|resourceRegenPerSecond|attackPower|defense|moveSpeed|defenseStanceMoveSpeedScale|maximumIdentity|identityRegenPerSecond|identityDrainPerSecond|identityStanceSwitchCost|", "players", "|maximumHp|maximumResource|resourceRegenPerSecond|attackPower|defense|criticalChancePercent|criticalDamagePercent|" },
        { "PlayerSkills.json", "skills", "skillId", "Skills", "|cooldownMs|resourceCost|identityCost|staggerDamage|partDamage|actionDurationMs|hitTimeMs|movementDistance|maximumRange|", "skills", "|cooldownMs|resourceCost|staggerDamage|partDamage|" },
        { "DamageProfiles.json", "profiles", "damageProfileId", "Damage", "|damageRatePercent|", "damageProfiles", "|attackCoefficientBp|damageAddend|damageSpreadPercent|" },
        { "BossProfiles.json", "bosses", "archetypeId", "Bosses", "|maximumHp|maximumHealthBars|attackPower|collisionRadius|engageDistance|moveSpeed|", "bosses", "|maximumHp|maximumHealthBars|attackPower|" }
    };
    std::string ReadText(const std::filesystem::path& path)
    {
        std::ifstream stream(path, std::ios::binary);
        return { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
    }
    std::string JsonString(const std::string& text) { return "\"" + CDataJson::Escape(text) + "\""; }
}

struct CBalanceTestPanel::JOB
{
    HANDLE process = nullptr;
    std::filesystem::path log;
    bool publish = false;
    // Closing a panel never cancels an atomic writer halfway through promotion.
    ~JOB() { if (process) CloseHandle(process); }
};

CBalanceTestPanel::CBalanceTestPanel() { Reload(); }
CBalanceTestPanel::~CBalanceTestPanel() = default;

bool CBalanceTestPanel::Is_Dirty() const
{
    for (const auto& document : m_documents)
        for (const auto& row : document.rows)
            for (const auto& field : row.fields)
                if (field.original != field.value) return true;
    return false;
}

bool CBalanceTestPanel::Reload()
{
    std::vector<DOCUMENT> staged;
    constexpr const char* profilePath = "Data/Balance/Profiles/Retail.balanceprofile.json";
    DATA_JSON_VALUE profile;
    std::string profileError;
    if (!CDataJson::Parse(ReadText(CProjectDataRoot::Resolve("Balance/Profiles/Retail.balanceprofile.json")), profile, profileError))
    { m_status = "Retail profile load failed: " + profileError; return false; }
    for (const auto& domain : DOMAINS)
    {
        DOCUMENT document;
        document.path = "Data/Balance/" + std::string(domain.file);
        document.label = domain.label;
        DATA_JSON_VALUE root;
        std::string error;
        const auto path = CProjectDataRoot::Resolve(std::filesystem::path("Balance") / domain.file);
        if (!CDataJson::Parse(ReadText(path), root, error))
        { m_status = "Load failed: " + document.path + ": " + error; return false; }
        const auto* rows = root.Find(domain.array);
        if (!rows || !rows->Is_Array()) { m_status = "Missing rows: " + document.path; return false; }
        for (const auto& source : rows->Get_Array())
        {
            const auto* identity = source.Find(domain.key);
            if (!identity || (!identity->Is_String() && !identity->Is_Number()))
            { m_status = "Missing stable identity: " + document.path; return false; }
            ROW row;
            row.id = identity->Is_String() ? identity->Get_String() : std::to_string(static_cast<std::uint32_t>(identity->Get_Number()));
            row.label = row.id;
            if (const auto* name = source.Find("displayName"); name && name->Is_String()) row.label += " | " + name->Get_String();
            if (const auto* slot = source.Find("inputSlot"); slot && slot->Is_String()) row.label += " [" + slot->Get_String() + "]";
            if (const auto* owner = source.Find("characterClass"); owner && owner->Is_String() && std::string_view(domain.key) != "characterClass")
                row.label = owner->Get_String() + " | " + row.label;
            const DATA_JSON_VALUE* overrideRow = nullptr;
            if (const auto* overrides = profile.Find(domain.profileArray); overrides && overrides->Is_Array())
                for (const auto& candidate : overrides->Get_Array())
                {
                    const auto* id = candidate.Find(domain.key);
                    if (!id || (!id->Is_String() && !id->Is_Number())) continue;
                    const auto identity = id->Is_String() ? id->Get_String() : std::to_string(static_cast<std::uint32_t>(id->Get_Number()));
                    if (identity == row.id) { overrideRow = &candidate; break; }
                }
            for (const auto& [name, value] : source.Get_Object())
            {
                if (!value.Is_Number() || std::string_view(domain.fields).find("|" + name + "|") == std::string_view::npos) continue;
                const bool integral = name != "moveSpeed" && name != "defenseStanceMoveSpeedScale" && name != "movementDistance" &&
                    name != "maximumRange" && name != "collisionRadius" && name != "engageDistance";
                const auto* effective = overrideRow && std::string_view(domain.profileFields).find("|" + name + "|") != std::string_view::npos ? overrideRow->Find(name) : nullptr;
                if (name == "damageRatePercent" && overrideRow)
                {
                    const auto* coefficient = overrideRow->Find("attackCoefficientBp");
                    const auto* addend = overrideRow->Find("damageAddend");
                    if ((coefficient && coefficient->Get_Number() > 0) || (addend && addend->Get_Number() > 0)) continue;
                }
                const auto number = effective && effective->Is_Number() ? effective->Get_Number() : value.Get_Number();
                row.fields.push_back({ name, number, number, integral,
                    effective ? profilePath : document.path, effective ? domain.profileArray : domain.array });
            }
            if (overrideRow)
                for (const auto& [name, value] : overrideRow->Get_Object())
                {
                    if (!value.Is_Number() || std::string_view(domain.profileFields).find("|" + name + "|") == std::string_view::npos ||
                        std::any_of(row.fields.begin(), row.fields.end(), [&](const FIELD& field) { return field.name == name; })) continue;
                    row.fields.push_back({ name, value.Get_Number(), value.Get_Number(), true, profilePath, domain.profileArray });
                }
            document.rows.push_back(std::move(row));
        }
        staged.push_back(std::move(document));
    }
    m_documents = std::move(staged);
    m_row = 0;
    m_status = "Saved authoring loaded. Runtime values change after Publish Server Data and a Server/Client restart.";
    return true;
}

void CBalanceTestPanel::Start_Job(const bool publish)
{
    if (m_job || (publish && Is_Dirty())) return;
    const auto repository = CProjectDataRoot::Get().parent_path();
    const auto directory = repository / "Intermediate" / "BalanceTest" /
        (std::to_string(GetCurrentProcessId()) + "-" + std::to_string(GetTickCount64()));
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) { m_status = error.message(); return; }
    const auto draft = directory / "draft.json";
    if (!publish)
    {
        std::ofstream output(draft, std::ios::binary);
        output << std::setprecision(17) << "{\"schema\":\"lostark.balance-test-draft\",\"formatVersion\":1,\"changes\":[";
        bool first = true;
        for (const auto& document : m_documents)
            for (const auto& row : document.rows)
                for (const auto& field : row.fields)
                {
                    if (field.original == field.value) continue;
                    if (!std::isfinite(field.value) || (field.integral && std::floor(field.value) != field.value))
                    { m_status = "A whole finite number is required for " + field.name; return; }
                    if (!first) output << ',';
                    first = false;
                    output << "{\"document\":" << JsonString(field.sourcePath) << ",\"domain\":" << JsonString(field.sourceArray) << ",\"id\":" << JsonString(row.id)
                        << ",\"field\":" << JsonString(field.name) << ",\"before\":" << field.original << ",\"value\":" << field.value << '}';
                }
        output << "]}\n";
        output.close();
        if (!output) { m_status = "Could not write the immutable numeric draft."; return; }
    }
    auto job = std::make_unique<JOB>();
    job->publish = publish;
    job->log = directory / "pipeline.log";
    const auto script = repository / "Tools" / "GameplayPipeline" /
        (publish ? "Publish-BalanceRuntimeSet.ps1" : "Save-BalanceTestDraft.ps1");
    std::wstring command = L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" + script.wstring() + L"\"";
    command += publish ? L" -Mode Publish -BalanceProfile Retail" : L" -DraftPath \"" + draft.wstring() + L"\"";
    SECURITY_ATTRIBUTES security{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
    const HANDLE output = CreateFileW(job->log.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE,
        &security, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    const HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (output == INVALID_HANDLE_VALUE || input == INVALID_HANDLE_VALUE)
    {
        if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
        if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
        m_status = "Could not open pipeline log handles."; return;
    }
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = input; startup.hStdOutput = startup.hStdError = output;
    PROCESS_INFORMATION process{};
    const bool started = CreateProcessW(nullptr, command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
        nullptr, repository.c_str(), &startup, &process) != FALSE;
    CloseHandle(input); CloseHandle(output);
    if (!started) { m_status = "Could not start the balance pipeline."; return; }
    CloseHandle(process.hThread);
    job->process = process.hProcess;
    m_job = std::move(job);
    m_status = publish ? "Publishing saved Server data..." : "Validating the candidate and merging changed numeric fields...";
}

void CBalanceTestPanel::Poll_Job()
{
    if (!m_job) return;
    const auto wait = WaitForSingleObject(m_job->process, 0u);
    if (wait == WAIT_TIMEOUT) return;
    DWORD code = 1;
    if (wait != WAIT_OBJECT_0 || !GetExitCodeProcess(m_job->process, &code))
    { m_status = "Pipeline observation failed. Keep the log and check the writer before retrying."; return; }
    const bool publish = m_job->publish;
    const auto log = m_job->log;
    m_job.reset();
    if (!code)
    {
        if (!publish && !Reload()) return;
        m_status = publish ? "PUBLISHED. Restart Server and Client to activate the saved balance." :
            "SAVED AND VALIDATED. Publish Server Data, then restart Server and Client. Other tool drafts were preserved.";
    }
    else
    {
        auto detail = ReadText(log);
        if (detail.size() > 6000u) detail = detail.substr(detail.size() - 6000u);
        m_status = "FAILED (" + std::to_string(code) + "): " + detail;
    }
    m_status += "\nLog: " + log.string();
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
    Poll_Job();
    ImGui::SetNextWindowSize(ImVec2(1040.f, 720.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Balance Test", &open)) { ImGui::End(); return; }
    ImGui::TextWrapped("Retail player, skill, damage and boss numbers. Source fields show their effective profile values; Save and Publish prepare the next Server start.");
    ImGui::BeginDisabled(m_job != nullptr);
    if (ImGui::Button("Reload Saved")) { if (Is_Dirty()) m_confirmReload = true; else Reload(); }
    ImGui::SameLine(); ImGui::BeginDisabled(!Is_Dirty());
    if (ImGui::Button("Save + Validate")) Start_Job(false);
    ImGui::EndDisabled(); ImGui::SameLine(); ImGui::BeginDisabled(Is_Dirty());
    if (ImGui::Button("Publish Server Data")) Start_Job(true);
    ImGui::EndDisabled();
    ImGui::SameLine(); ImGui::TextDisabled("%s", Is_Dirty() ? "UNSAVED" : "saved");
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
        ImGui::BeginChild("NumericTargets", ImVec2(400.f, -135.f), true);
        for (std::size_t i = 0; i < document.rows.size(); ++i)
            if (ImGui::Selectable(document.rows[i].label.c_str(), i == m_row)) m_row = i;
        ImGui::EndChild(); ImGui::SameLine();
        ImGui::BeginChild("NumericFields", ImVec2(0.f, -135.f), true);
        if (m_row < document.rows.size())
        {
            auto& row = document.rows[m_row];
            ImGui::TextWrapped("%s", row.label.c_str());
            for (auto& field : row.fields)
            {
                ImGui::InputDouble(field.name.c_str(), &field.value, field.integral ? 1.0 : 0.1, field.integral ? 100.0 : 1.0,
                    field.integral ? "%.0f" : "%.4f");
                if (field.value != field.original) ImGui::TextDisabled("Saved: %.6g", field.original);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", field.sourcePath.c_str());
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
