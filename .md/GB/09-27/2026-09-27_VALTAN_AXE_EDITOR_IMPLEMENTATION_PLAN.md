# 발탄 F1 손도끼 변환 구현 계획

기준: 2026-09-27, `codex/bern-dragon-camera-performance`, HEAD `8543589d`.

## G00. 현재 소비자와 저장 경계

`MainApp::RenderValtanArenaControls`의 Start Position / Before Entrance / Arena Start 아래에
손도끼 편집 영역을 붙인다. 손도끼는 `CValtan -> CPart_Equipment -> b_wp_r_01` 표현이며
Server 피해 판정은 별도 encounter 데이터가 소유한다. `BossCatalog.json`은 현재
presentation generation의 `COMBAT_VISUAL` artifact이므로 기존 게시 경로가 Server의
동일 세대 파일에도 그 값을 보존한다. 시각 offset을 Server hitbox로 승격하지 않는다.

## G01. ActorCatalog와 socket 소비

`BOSS_ACTOR_ENTRY`에 optional `weaponSocketTransform`을 추가한다. `positionMeters`는
정규화한 손본 축 기준 미터이며 boss presentationScale 이전 값이다. `rotationDegrees`는
pitch/yaw/roll이다. 저장값이 없으면 모두 0으로 기존 모습을 유지한다. 허용 범위는 각
position ±10m, rotation ±360도이며 유한성 검사 후에만 메모리 값을 교체한다.

`CPart_Equipment`는 zero-default socket 보정을 기존 부착 행렬에 합성한다.
translation은 bone preScale로 다시 축소하지 않고 정규화한 socket basis로 변환한다.
`CValtan::Update`는 현재 정상/유령 presentation archetype의 값을 같은 part에 적용한다.

## G02. MainApp 편집과 Save / Publish

새 `MainApp_ValtanAxe.cpp`는 선택한 정상/유령 발탄의 draft와 저장 baseline을 소유한다.
수치 편집은 catalog의 typed 경계로 즉시 적용한다. Save는 최신 BossCatalog를 다시 읽고
archetype stable ID로 해당 transform 필드만 병합한다. 동일 필드의 동시 변경은 거부하며
무관한 최신 필드는 보존한다. writer lock, 임시 파일 재parse, 교체 직전 bytes 확인,
backup과 원자 교체를 유지한다. 실패하면 draft와 디스크를 보존한다.

Save + Publish는 저장 성공 뒤 기존 `CBalanceTool::Publish_ServerRuntimeSet`의 receipt 기반
Data-only 비동기 게시를 호출한다. 성공 상태와 Server 재시작 필요를 별도로 표시한다.
실행 중 도구를 종료하거나 사용자 draft를 자동 Reload하지 않는다.

새 CPP는 기존 물리 MainApp 폴더 분류로 vcxproj / filters에 등록한다. 새 C++은 UTF-8
BOM 없음이며 기존 파일은 원래 인코딩을 보존한다. 리소스 파일 추가는 없다.

## G03. 검증

변경 CPP의 Debug/Release 컴파일은 통합 빌드 담당이 실행한다. JSON과 project XML parse,
PowerShell parser, optional transform schema와 정상/유령 소비 경계, `git diff --check`를
확인한다. UI 조작은 사용자가 발탄 아레나에서 F1 → Valtan Arena → Axe Transform을 열어
XYZ position / rotation 편집, Save, Save + Publish, 재진입으로 확인한다. 화면 PASS와
Server 메모리 적용은 코드/파일 저장 성공과 구분한다.

## G04. 신규 MainApp_ValtanAxe.cpp 전체 코드

`Client/Private/MainApp_ValtanAxe.cpp`는 F1 도끼 UI와 단일 catalog transform의 load/save를
소유한다. `imgui.h`를 `MainApp.h`보다 먼저 포함해 Engine Debug의 `new` macro가 ImGui의
placement-new 선언을 바꾸지 않도록 기존 `MainApp_SequenceViewer.cpp` include 순서를 따른다.

```cpp
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
```
