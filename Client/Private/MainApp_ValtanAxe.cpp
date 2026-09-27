#include "imgui.h"
#include "MainApp.h"
#include "ActorCatalog.h"
#include "BalanceTool.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <atomic>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <locale>
#include <sstream>

namespace
{
using namespace Client;
using Json = DATA_JSON_VALUE;

struct AXE_DRAFT final
{
    bool attempted = false, loaded = false;
    BOSS_WEAPON_SOCKET_TRANSFORM saved, value;
    std::string status;
};

bool Equal(const float3_t& a, const float3_t& b)
{ return a.x == b.x && a.y == b.y && a.z == b.z; }

bool Equal(const BOSS_WEAPON_SOCKET_TRANSFORM& a, const BOSS_WEAPON_SOCKET_TRANSFORM& b)
{ return Equal(a.positionMeters, b.positionMeters) && Equal(a.rotationDegrees, b.rotationDegrees); }

std::string Serialize(const Json& value)
{
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::setprecision(17);
    const auto write = [&](const auto& self, const Json& item, size_t depth) -> void {
        const std::string indent(depth * 2u, ' '), childIndent((depth + 1u) * 2u, ' ');
        switch (item.Get_Type())
        {
        case DATA_JSON_TYPE::NULL_VALUE: out << "null"; break;
        case DATA_JSON_TYPE::BOOLEAN: out << (item.Get_Boolean() ? "true" : "false"); break;
        case DATA_JSON_TYPE::NUMBER: out << item.Get_Number(); break;
        case DATA_JSON_TYPE::STRING: out << '"' << CDataJson::Escape(item.Get_String()) << '"'; break;
        case DATA_JSON_TYPE::ARRAY:
            out << '[';
            for (size_t i = 0; i < item.Get_Array().size(); ++i)
            { out << (i ? ",\n" : "\n") << childIndent; self(self, item.Get_Array()[i], depth + 1u); }
            if (!item.Get_Array().empty()) out << '\n' << indent;
            out << ']'; break;
        case DATA_JSON_TYPE::OBJECT:
            out << '{';
            {
                bool first = true;
                for (const auto& key : item.Get_ObjectInsertionOrder())
                {
                    out << (first ? "\n" : ",\n") << childIndent << '"' << CDataJson::Escape(key) << "\": ";
                    first = false; self(self, *item.Find(key), depth + 1u);
                }
                if (!first) out << '\n' << indent;
            }
            out << '}'; break;
        }
    };
    write(write, value, 0u);
    return out.str() + '\n';
}

Json Set(const Json& object, const char* key, Json value)
{
    auto fields = object.Get_Object();
    auto order = object.Get_ObjectInsertionOrder();
    if (!fields.contains(key)) order.push_back(key);
    fields[key] = std::move(value);
    return Json::Object(std::move(fields), std::move(order));
}

bool Read(const std::filesystem::path& path, std::string& bytes, Json& document, std::string& status)
{
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input || input.tellg() < 0 || input.tellg() > 16 * 1024 * 1024)
    { status = "Cannot read bounded BossCatalog: " + path.string(); return false; }
    input.seekg(0);
    bytes.assign(std::istreambuf_iterator<char>(input), {});
    if (input.bad() || !CDataJson::Parse(bytes, document, status)) return false;
    const auto* schema = document.Find("schema");
    const auto* version = document.Find("formatVersion");
    const auto* bosses = document.Find("bosses");
    if (!document.Is_Object() || !schema || !schema->Is_String() || schema->Get_String() != "lostark.boss-catalog" ||
        !version || !version->Is_Number() || version->Get_Number() != 8.0 || !bosses || !bosses->Is_Array())
    { status = "BossCatalog header is invalid; existing draft preserved."; return false; }
    return true;
}

bool ReadTransform(const Json& root, const char* archetype, BOSS_WEAPON_SOCKET_TRANSFORM& out,
    size_t& rowIndex, std::string& status)
{
    rowIndex = SIZE_MAX;
    const auto& rows = root.Find("bosses")->Get_Array();
    for (size_t i = 0; i < rows.size(); ++i)
    {
        const auto* id = rows[i].Find("archetypeId");
        if (!id || !id->Is_String() || id->Get_String() != archetype) continue;
        if (rowIndex != SIZE_MAX) { status = "Duplicate Valtan archetype; Save rejected."; return false; }
        rowIndex = i;
    }
    if (rowIndex == SIZE_MAX) { status = "Valtan archetype is missing from BossCatalog."; return false; }
    BOSS_WEAPON_SOCKET_TRANSFORM staged;
    if (const auto* transform = rows[rowIndex].Find("weaponSocketTransform"))
    {
        if (!transform->Is_Object() || transform->Get_Object().size() != 2u)
        { status = "Axe transform requires positionMeters and rotationDegrees."; return false; }
        const auto vector = [](const Json* value, float3_t& out) {
            if (!value || !value->Is_Array() || value->Get_Array().size() != 3u) return false;
            float* axes[] = { &out.x, &out.y, &out.z };
            for (size_t i = 0u; i < 3u; ++i)
            {
                const auto& scalar = value->Get_Array()[i];
                if (!scalar.Is_Number() || !std::isfinite(scalar.Get_Number()) ||
                    std::abs(scalar.Get_Number()) > 360.0) return false;
                *axes[i] = static_cast<float>(scalar.Get_Number());
            }
            return true;
        };
        if (!vector(transform->Find("positionMeters"), staged.positionMeters) ||
            !vector(transform->Find("rotationDegrees"), staged.rotationDegrees) ||
            !CActorCatalog::Validate_BossWeaponSocketTransform(staged))
        { status = "Axe transform must be finite, position +/-10 m and rotation +/-360 degrees."; return false; }
    }
    out = staged;
    return true;
}

bool Load(const char* archetype, AXE_DRAFT& draft)
{
    draft.attempted = true;
    Json root; std::string bytes; size_t index;
    BOSS_WEAPON_SOCKET_TRANSFORM staged;
    if (!Read(CProjectDataRoot::Resolve(L"Actors/BossCatalog.json"), bytes, root, draft.status) ||
        !ReadTransform(root, archetype, staged, index, draft.status)) return false;
    draft.saved = draft.value = staged;
    draft.loaded = true;
    draft.status = "Loaded saved axe transform.";
    return true;
}

struct WRITER_LOCK final
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    OVERLAPPED overlap{};
    bool locked = false;
    ~WRITER_LOCK()
    {
        if (locked) UnlockFileEx(handle, 0u, 1u, 0u, &overlap);
        if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
    }
    bool Acquire(const std::filesystem::path& repository, std::string& status)
    {
        const auto directory = repository / L"out/ValtanPatternTransactions";
        std::error_code error;
        std::filesystem::create_directories(directory, error);
        if (error) { status = "Cannot prepare canonical writer admission: " + error.message(); return false; }
        handle = CreateFileW((directory / L"create-pattern.lock").c_str(), GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle == INVALID_HANDLE_VALUE || !LockFileEx(handle,
            LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0u, 1u, 0u, &overlap))
        { status = "Another Valtan reader/writer owns the canonical transaction; retry Save."; return false; }
        locked = true;
        const bool recovery = std::filesystem::exists(directory / L"active-generation.json", error);
        if (error || recovery)
        { status = "Valtan generation requires its owning publisher recovery; axe draft preserved."; return false; }
        return true;
    }
};

bool Save(const char* archetype, AXE_DRAFT& draft)
{
    if (!draft.loaded || !CActorCatalog::Validate_BossWeaponSocketTransform(draft.value))
    { draft.status = "Load a valid axe transform before Save."; return false; }
    const auto path = CProjectDataRoot::Resolve(L"Actors/BossCatalog.json");
    if (path.empty()) { draft.status = "Project Data root is unavailable."; return false; }
    WRITER_LOCK admission;
    if (!admission.Acquire(path.parent_path().parent_path().parent_path(), draft.status)) return false;
    std::string latestBytes; Json root; size_t index;
    BOSS_WEAPON_SOCKET_TRANSFORM latest;
    if (!Read(path, latestBytes, root, draft.status) ||
        !ReadTransform(root, archetype, latest, index, draft.status)) return false;
    BOSS_WEAPON_SOCKET_TRANSFORM merged = latest;
    // Each edited axis owns only that field. Concurrent edits of other axes,
    // another boss or an unrelated material survive this Save.
    const auto merge = [](const float3_t& saved, const float3_t& value, const float3_t& current, float3_t& out) {
        const float a[] = {saved.x, saved.y, saved.z}, b[] = {value.x, value.y, value.z}, c[] = {current.x, current.y, current.z};
        float* target[] = {&out.x, &out.y, &out.z};
        for (size_t i = 0; i < 3u; ++i)
        {
            if (a[i] == b[i]) continue;
            if (a[i] != c[i] && b[i] != c[i]) return false;
            *target[i] = b[i];
        }
        return true;
    };
    if (!merge(draft.saved.positionMeters, draft.value.positionMeters, latest.positionMeters, merged.positionMeters) ||
        !merge(draft.saved.rotationDegrees, draft.value.rotationDegrees, latest.rotationDegrees, merged.rotationDegrees))
    { draft.status = "The same axe axis changed on disk. Draft and disk preserved; Reload saved before retrying."; return false; }
    const auto vector = [](const float3_t& v) {
        return Json::Array({Json::Number(v.x), Json::Number(v.y), Json::Number(v.z)});
    };
    auto rows = root.Find("bosses")->Get_Array();
    rows[index] = Set(rows[index], "weaponSocketTransform", Json::Object({
        {"positionMeters", vector(merged.positionMeters)}, {"rotationDegrees", vector(merged.rotationDegrees)}},
        {"positionMeters", "rotationDegrees"}));
    root = Set(root, "bosses", Json::Array(std::move(rows)));
    const std::string candidate = Serialize(root);
    static std::atomic_uint64_t serial{0u};
    const auto temporary = path.wstring() + L".tmp." + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(++serial);
    HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) { draft.status = "Cannot stage axe Save; existing file preserved."; return false; }
    DWORD written = 0u;
    const bool wrote = WriteFile(file, candidate.data(), static_cast<DWORD>(candidate.size()), &written, nullptr) &&
        written == candidate.size() && FlushFileBuffers(file);
    CloseHandle(file);
    const auto reject = [&](const char* reason) {
        std::error_code ignored; std::filesystem::remove(temporary, ignored);
        draft.status = reason; return false;
    };
    if (!wrote) return reject("Axe Save staging failed; previous file preserved.");
    Json stagedRoot, currentRoot; std::string stagedBytes, currentBytes;
    BOSS_WEAPON_SOCKET_TRANSFORM staged;
    if (!Read(temporary, stagedBytes, stagedRoot, draft.status) || stagedBytes != candidate ||
        !ReadTransform(stagedRoot, archetype, staged, index, draft.status) || !Equal(staged, merged) ||
        !Read(path, currentBytes, currentRoot, draft.status) || currentBytes != latestBytes)
        return reject("Axe Save validation or freshness check failed; draft and disk preserved.");
    const auto backup = path.wstring() + L".axe.previous.bak";
    if (!CopyFileW(path.c_str(), backup.c_str(), FALSE) ||
        !Read(path, currentBytes, currentRoot, draft.status) || currentBytes != latestBytes ||
        !MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        return reject("Axe backup, freshness check or atomic replace failed; draft and existing file preserved.");
    draft.saved = draft.value = merged;
    (void)CActorCatalog::Set_ValtanWeaponSocketTransform(archetype, merged, draft.status);
    draft.status = "Saved Data/Actors/BossCatalog.json. Publish installs its Server presentation generation.";
    return true;
}
}

void CMainApp::RenderValtanAxeEditor()
{
    if (!ImGui::TreeNode("Axe Transform")) return;
    static AXE_DRAFT drafts[2];
    static int selected = 0;
    const char* ids[] = {"BOSS_VALTAN", "BOSS_VALTAN_GHOST"};
    ImGui::Combo("Axe owner", &selected, "Valtan\0Ghost Valtan\0");
    auto& draft = drafts[selected];
    const char* archetype = ids[selected];
    if (!draft.attempted) (void)Load(archetype, draft);
    const bool busy = m_pBalanceTool &&
        (m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() || m_pBalanceTool->Is_ServerRuntimeSetPublishRunning());
    ImGui::TextWrapped("Hand socket b_wp_r_01. Position uses socket axes in metres before boss scale; rotation is pitch / yaw / roll. Changes appear immediately on this Client.");
    ImGui::BeginDisabled(!draft.loaded || busy);
    auto candidate = draft.value;
    bool changed = ImGui::DragFloat3("Position XYZ (m)##ValtanAxe", &candidate.positionMeters.x,
        0.01f, -10.f, 10.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    changed |= ImGui::DragFloat3("Rotation XYZ (deg)##ValtanAxe", &candidate.rotationDegrees.x,
        0.5f, -360.f, 360.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    if (changed && CActorCatalog::Set_ValtanWeaponSocketTransform(archetype, candidate, draft.status)) draft.value = candidate;
    if (ImGui::Button("Reset axes##ValtanAxe"))
    {
        if (CActorCatalog::Set_ValtanWeaponSocketTransform(archetype, {}, draft.status)) draft.value = {};
    }
    ImGui::SameLine();
    if (ImGui::Button("Save##ValtanAxe")) (void)Save(archetype, draft);
    ImGui::SameLine();
    if (ImGui::Button("Save + Publish##ValtanAxe"))
    {
        if (Save(archetype, draft))
        {
            if (!m_pBalanceTool) m_pBalanceTool = std::make_unique<CBalanceTool>(false);
            std::string revision, publishStatus;
            if (m_pBalanceTool->Ensure_Initialized() && m_pBalanceTool->Reload_ValtanSource(publishStatus) &&
                m_pBalanceTool->Get_ValtanPublishSourceRevision(revision, publishStatus) &&
                m_pBalanceTool->Publish_ServerRuntimeSet(revision, publishStatus))
                draft.status = "Axe saved. " + publishStatus;
            else draft.status = "Axe saved; Publish did not start. " + publishStatus;
        }
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(busy);
    if (ImGui::Button("Reload saved##ValtanAxe") && Load(archetype, draft))
        (void)CActorCatalog::Set_ValtanWeaponSocketTransform(archetype, draft.value, draft.status);
    ImGui::EndDisabled();
    if (draft.loaded && !Equal(draft.value, draft.saved)) ImGui::TextDisabled("Unsaved axe changes");
    ImGui::TextWrapped("%s", draft.status.c_str());
    if (m_pBalanceTool)
    {
        std::string publishStatus; double elapsed = 0.0;
        if (m_pBalanceTool->Get_ServerRuntimeSetPublishState(publishStatus, elapsed) != CBalanceTool::SERVER_RUNTIME_PUBLISH_STATE::IDLE)
            ImGui::TextWrapped("%s", publishStatus.c_str());
    }
    ImGui::TextWrapped("Publish stores the same catalog in the Server presentation generation. Restart Server and re-enter after successful Publish. Damage and collision remain owned by encounter data.");
    ImGui::TreePop();
}
