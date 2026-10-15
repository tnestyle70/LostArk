// 발탄 본체·유령이 손에 드는 무기의 소켓 보정 패널이다. 서버가 생성하는 낙하 도끼 패턴과는 별개다.
#include "imgui.h"
// 발탄 도구가 소유한 도끼 draft와 편집·저장 함수를 구현한다.
#include "ValtanBossTool.h"
// 보스 무기 소켓 변환의 검증과 현재 Client preview 적용을 사용한다.
#include "ActorCatalog.h"
// 발탄 저작 작업의 점유 상태와 기존 Server runtime publisher를 연결한다.
#include "BalanceTool.h"
// BossCatalog를 파싱하고 요청한 필드만 교체할 JSON 값을 사용한다.
#include "DataJson.h"
// Data/Actors/BossCatalog.json 정본 경로를 확인한다.
#include "ProjectDataRoot.h"

// 같은 프로세스의 임시 저장 파일 이름에 중복 없는 순번을 붙인다.
#include <atomic>
// 소켓 위치·회전 후보의 유한성과 허용 범위를 검사한다.
#include <cmath>
// 정본과 임시 파일의 bytes를 읽어 검증·freshness 비교에 사용한다.
#include <fstream>
// JSON 숫자 직렬화 정밀도를 고정한다.
#include <iomanip>
// 파일 스트림 전체를 문자열 bytes로 읽는다.
#include <iterator>
// JSON 숫자에 로캘별 소수점이 섞이지 않게 한다.
#include <locale>
// 검증한 문서의 JSON 문자열을 구성한다.
#include <sstream>
// 정본·임시 경로와 파일 오류, 문자열, JSON 값 이동에 필요한 표준 타입을 직접 포함한다.
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>

namespace
{
    using namespace Client;
    using Json = DATA_JSON_VALUE;

    // 저장·편집·재읽기 값의 축별 정확한 일치를 확인한다. 허용 오차를 도입해 변경 여부를 흐리지 않는다.
    bool Equal(const float3_t& a, const float3_t& b)
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    // 임시 파일 재검증과 미저장 표시에서 위치·회전 두 묶음의 일치를 확인한다.
    bool Equal(const BOSS_WEAPON_SOCKET_TRANSFORM& a, const BOSS_WEAPON_SOCKET_TRANSFORM& b)
    {
        return Equal(a.positionMeters, b.positionMeters) && Equal(a.rotationDegrees, b.rotationDegrees);
    }

    // 저장 후보 JSON을 기존 키 삽입 순서와 숫자 정밀도를 유지해 직렬화한다.
    // 문서의 다른 값은 그대로 읽으며 결과 문자열 생성만 담당한다.
    std::string Serialize(const Json& value)
    {
        std::ostringstream out;
        out.imbue(std::locale::classic());
        out << std::setprecision(17);
        const auto write = [&](const auto& self, const Json& item, size_t depth) -> void
        {
            const std::string indent(depth * 2u, ' '), childIndent((depth + 1u) * 2u, ' ');
            switch (item.Get_Type())
            {
            case DATA_JSON_TYPE::NULL_VALUE:
                out << "null";
                break;
            case DATA_JSON_TYPE::BOOLEAN:
                out << (item.Get_Boolean() ? "true" : "false");
                break;
            case DATA_JSON_TYPE::NUMBER:
                out << item.Get_Number();
                break;
            case DATA_JSON_TYPE::STRING:
                out << '"' << CDataJson::Escape(item.Get_String()) << '"';
                break;
            case DATA_JSON_TYPE::ARRAY:
                out << '[';
                for (size_t i = 0; i < item.Get_Array().size(); ++i)
                {
                    out << (i ? ",\n" : "\n") << childIndent;
                    self(self, item.Get_Array()[i], depth + 1u);
                }
                if (!item.Get_Array().empty())
                {
                    out << '\n' << indent;
                }
                out << ']';
                break;
            case DATA_JSON_TYPE::OBJECT:
                out << '{';
                {
                    bool first = true;
                    for (const auto& key : item.Get_ObjectInsertionOrder())
                    {
                        out << (first ? "\n" : ",\n") << childIndent << '"' << CDataJson::Escape(key) << "\": ";
                        first = false;
                        self(self, *item.Find(key), depth + 1u);
                    }
                    if (!first)
                    {
                        out << '\n' << indent;
                    }
                }
                out << '}';
                break;
            }
        };
        write(write, value, 0u);
        return out.str() + '\n';
    }

    // 기존 JSON 객체를 복사해 한 key만 바꾼다. 기존 key 순서는 보존하고 새 key만 마지막에 추가한다.
    Json Set(const Json& object, const char* key, Json value)
    {
        auto fields = object.Get_Object();
        auto order = object.Get_ObjectInsertionOrder();
        if (!fields.contains(key))
        {
            order.push_back(key);
        }
        fields[key] = std::move(value);
        return Json::Object(std::move(fields), std::move(order));
    }

    // Load와 Save가 공유하는 최대 16MiB 읽기·schema 검사다.
    // 실패해도 출력 bytes/document 일부는 바뀔 수 있으므로 호출자는 지역 임시 값으로 받는다.
    bool Read(const std::filesystem::path& path, std::string& bytes, Json& document, std::string& status)
    {
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input || input.tellg() < 0 || input.tellg() > 16 * 1024 * 1024)
        {
            status = "Cannot read bounded BossCatalog: " + path.string();
            return false;
        }
        input.seekg(0);
        bytes.assign(std::istreambuf_iterator<char>(input), {});
        if (input.bad() || !CDataJson::Parse(bytes, document, status))
        {
            return false;
        }
        const auto* schema = document.Find("schema");
        const auto* version = document.Find("formatVersion");
        const auto* bosses = document.Find("bosses");
        if (!document.Is_Object() || !schema || !schema->Is_String() ||
            schema->Get_String() != "lostark.boss-catalog" || !version || !version->Is_Number() ||
            version->Get_Number() != 8.0 || !bosses || !bosses->Is_Array())
        {
            status = "BossCatalog header is invalid; existing draft preserved.";
            return false;
        }
        return true;
    }

    // 검증한 catalog에서 정확한 archetype 행 하나를 찾아 위치 m·회전 degree를 읽는다.
    // 소켓 필드가 없으면 기본 변환을 사용한다. out은 끝에서만 교체하지만 rowIndex·status는 검색 중 바뀔 수 있다.
    bool ReadTransform(const Json& root,
                       const char* archetype,
                       BOSS_WEAPON_SOCKET_TRANSFORM& out,
                       size_t& rowIndex,
                       std::string& status)
    {
        rowIndex = SIZE_MAX;
        const auto& rows = root.Find("bosses")->Get_Array();
        for (size_t i = 0; i < rows.size(); ++i)
        {
            const auto* id = rows[i].Find("archetypeId");
            if (!id || !id->Is_String() || id->Get_String() != archetype)
            {
                continue;
            }
            if (rowIndex != SIZE_MAX)
            {
                status = "Duplicate Valtan archetype; Save rejected.";
                return false;
            }
            rowIndex = i;
        }
        if (rowIndex == SIZE_MAX)
        {
            status = "Valtan archetype is missing from BossCatalog.";
            return false;
        }
        // 위치와 회전을 임시 값으로 검사하여 한쪽만 성공한 결과가 편집값으로 노출되지 않게 한다.
        BOSS_WEAPON_SOCKET_TRANSFORM staged;
        if (const auto* transform = rows[rowIndex].Find("weaponSocketTransform"))
        {
            if (!transform->Is_Object() || transform->Get_Object().size() != 2u)
            {
                status = "Axe transform requires positionMeters and rotationDegrees.";
                return false;
            }
            const auto vector = [](const Json* value, float3_t& out)
            {
                if (!value || !value->Is_Array() || value->Get_Array().size() != 3u)
                {
                    return false;
                }
                float* axes[] = {&out.x, &out.y, &out.z};
                for (size_t i = 0u; i < 3u; ++i)
                {
                    const auto& scalar = value->Get_Array()[i];
                    if (!scalar.Is_Number() || !std::isfinite(scalar.Get_Number()) ||
                        std::abs(scalar.Get_Number()) > 360.0)
                    {
                        return false;
                    }
                    *axes[i] = static_cast<float>(scalar.Get_Number());
                }
                return true;
            };
            if (!vector(transform->Find("positionMeters"), staged.positionMeters) ||
                !vector(transform->Find("rotationDegrees"), staged.rotationDegrees) ||
                !CActorCatalog::Validate_BossWeaponSocketTransform(staged))
            {
                status = "Axe transform must be finite, position +/-10 m and rotation +/-360 degrees.";
                return false;
            }
        }
        out = staged;
        return true;
    }

    // 발탄 publisher와 같은 byte-range lock을 소유하는 저장 범위다.
    // 소멸 시 lock과 HANDLE을 정리하며 미완료 generation이 있으면 다른 writer의 복구를 요구한다.
    struct WRITER_LOCK final
    {
        HANDLE handle = INVALID_HANDLE_VALUE;
        OVERLAPPED overlap{};
        bool locked = false;
        ~WRITER_LOCK()
        {
            if (locked)
            {
                UnlockFileEx(handle, 0u, 1u, 0u, &overlap);
            }
            if (handle != INVALID_HANDLE_VALUE)
            {
                CloseHandle(handle);
            }
        }
        // Save가 정본 재읽기 전에 비차단 배타 lock을 얻는다. 기존 transaction 복구 표식이 있으면 저장을 시작하지 않는다.
        bool Acquire(const std::filesystem::path& repository, std::string& status)
        {
            const auto directory = repository / L"out/ValtanPatternTransactions";
            std::error_code error;
            std::filesystem::create_directories(directory, error);
            if (error)
            {
                status = "Cannot prepare canonical writer admission: " + error.message();
                return false;
            }
            handle = CreateFileW((directory / L"create-pattern.lock").c_str(),
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                 nullptr,
                                 OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,
                                 nullptr);
            if (handle == INVALID_HANDLE_VALUE ||
                !LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0u, 1u, 0u, &overlap))
            {
                status = "Another Valtan reader/writer owns the canonical transaction; retry Save.";
                return false;
            }
            locked = true;
            const bool recovery = std::filesystem::exists(directory / L"active-generation.json", error);
            if (error || recovery)
            {
                status = "Valtan generation requires its owning publisher recovery; axe draft preserved.";
                return false;
            }
            return true;
        }
    };

} // 파일 내부 저장 보조 함수

NS_BEGIN(Client)

// 패널 최초 열기와 Reload가 호출한다. 정본과 변환을 모두 검증한 뒤 saved/value를 함께 교체한다.
// 실패하면 attempted와 status만 갱신하며 기존 유효 편집값을 보존한다.
bool_t CValtanBossTool::Load_AxeDraft(const char_t* archetype, AXE_DRAFT& draft)
{
    draft.attempted = true;
    Json root;
    std::string bytes;
    size_t index;
    BOSS_WEAPON_SOCKET_TRANSFORM staged;
    if (!Read(CProjectDataRoot::Resolve(L"Actors/BossCatalog.json"), bytes, root, draft.status) ||
        !ReadTransform(root, archetype, staged, index, draft.status))
    {
        return false;
    }
    draft.saved = draft.value = staged;
    draft.loaded = true;
    draft.status = "Loaded saved axe transform.";
    return true;
}

// 사용자 Save 동작이 현재 디스크를 다시 읽어 편집한 축만 병합하고 임시 파일 검증·백업·원자적 교체를 수행한다.
// 정본 교체 전 실패는 기존 파일과 편집값을 유지하고 진단을 갱신한다. 교체 성공 후 saved/value를 갱신하고 Client preview 적용을 요청한다.
// preview setter의 반환값은 사용하지 않으므로 파일 저장 성공이 실행 중 preview 반영 성공까지 보장하지는 않는다.
bool_t CValtanBossTool::Save_AxeDraft(const char_t* archetype, AXE_DRAFT& draft)
{
    if (!draft.loaded || !CActorCatalog::Validate_BossWeaponSocketTransform(draft.value))
    {
        draft.status = "Load a valid axe transform before Save.";
        return false;
    }
    const auto path = CProjectDataRoot::Resolve(L"Actors/BossCatalog.json");
    if (path.empty())
    {
        draft.status = "Project Data root is unavailable.";
        return false;
    }
    WRITER_LOCK admission;
    if (!admission.Acquire(path.parent_path().parent_path().parent_path(), draft.status))
    {
        return false;
    }
    std::string latestBytes;
    Json root;
    size_t index;
    BOSS_WEAPON_SOCKET_TRANSFORM latest;
    if (!Read(path, latestBytes, root, draft.status) || !ReadTransform(root, archetype, latest, index, draft.status))
    {
        return false;
    }
    BOSS_WEAPON_SOCKET_TRANSFORM merged = latest;
    // 사용자가 바꾼 축만 이 저장의 변경 대상이다. 다른 축·다른 보스·무관한 재질의 최신 디스크 변경은 보존한다.
    const auto merge = [](const float3_t& saved, const float3_t& value, const float3_t& current, float3_t& out)
    {
        const float a[] = {saved.x, saved.y, saved.z}, b[] = {value.x, value.y, value.z},
                    c[] = {current.x, current.y, current.z};
        float* target[] = {&out.x, &out.y, &out.z};
        for (size_t i = 0; i < 3u; ++i)
        {
            if (a[i] == b[i])
            {
                continue;
            }
            if (a[i] != c[i] && b[i] != c[i])
            {
                return false;
            }
            *target[i] = b[i];
        }
        return true;
    };
    if (!merge(draft.saved.positionMeters, draft.value.positionMeters, latest.positionMeters, merged.positionMeters) ||
        !merge(draft.saved.rotationDegrees,
               draft.value.rotationDegrees,
               latest.rotationDegrees,
               merged.rotationDegrees))
    {
        draft.status = "The same axe axis changed on disk. Draft and disk preserved; Reload saved before retrying.";
        return false;
    }
    // 필드 충돌 검사 뒤 선택한 보스의 weaponSocketTransform만 새 JSON 값으로 교체한다.
    const auto vector = [](const float3_t& v)
    {
        return Json::Array({Json::Number(v.x), Json::Number(v.y), Json::Number(v.z)});
    };
    auto rows = root.Find("bosses")->Get_Array();
    rows[index] = Set(rows[index],
                      "weaponSocketTransform",
                      Json::Object({{"positionMeters", vector(merged.positionMeters)},
                                    {"rotationDegrees", vector(merged.rotationDegrees)}},
                                   {"positionMeters", "rotationDegrees"}));
    root = Set(root, "bosses", Json::Array(std::move(rows)));
    const std::string candidate = Serialize(root);
    // 정본과 같은 폴더에 프로세스 ID·순번으로 임시 파일을 만든다. 기존 파일과 충돌하면 덮어쓰지 않는다.
    static std::atomic_uint64_t serial{0u};
    const auto temporary =
        path.wstring() + L".tmp." + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(++serial);
    HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        draft.status = "Cannot stage axe Save; existing file preserved.";
        return false;
    }
    DWORD written = 0u;
    const bool wrote = WriteFile(file, candidate.data(), static_cast<DWORD>(candidate.size()), &written, nullptr) &&
                       written == candidate.size() && FlushFileBuffers(file);
    CloseHandle(file);
    const auto reject = [&](const char* reason)
    {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        draft.status = reason;
        return false;
    };
    if (!wrote)
    {
        return reject("Axe Save staging failed; previous file preserved.");
    }
    // 디스크에 쓴 후보를 다시 파싱하고 정본 bytes가 첫 읽기와 같은지 확인한다. 검증 실패는 임시 파일만 지운다.
    Json stagedRoot, currentRoot;
    std::string stagedBytes, currentBytes;
    BOSS_WEAPON_SOCKET_TRANSFORM staged;
    if (!Read(temporary, stagedBytes, stagedRoot, draft.status) || stagedBytes != candidate ||
        !ReadTransform(stagedRoot, archetype, staged, index, draft.status) || !Equal(staged, merged) ||
        !Read(path, currentBytes, currentRoot, draft.status) || currentBytes != latestBytes)
    {
        return reject("Axe Save validation or freshness check failed; draft and disk preserved.");
    }
    // 백업 후 freshness를 다시 확인하고 원자적 파일 교체를 요청한다. 실제 게시와 실행 중 Server 갱신은 별도다.
    const auto backup = path.wstring() + L".axe.previous.bak";
    if (!CopyFileW(path.c_str(), backup.c_str(), FALSE) || !Read(path, currentBytes, currentRoot, draft.status) ||
        currentBytes != latestBytes ||
        !MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        return reject("Axe backup, freshness check or atomic replace failed; draft and existing file preserved.");
    }
    draft.saved = draft.value = merged;
    (void)CActorCatalog::Set_ValtanWeaponSocketTransform(archetype, merged, draft.status);
    draft.status = "Saved Data/Actors/BossCatalog.json. Publish installs its Server presentation generation.";
    return true;
}

// Developer Tools의 발탄 패널이 본체·유령 draft를 편집한다.
// 슬라이더는 Client preview만 바꾸고 Save는 정본, Save + Publish는 기존 발탄 publisher로 명시적으로 이어진다.
void CValtanBossTool::Render_AxeEditor()
{
    // TreeNode의 반환값은 이름 비교가 아니라 펼침 여부다. 접혀 있으면 내용을 그리지 않고 반환한다.
    if (!ImGui::TreeNode("Axe Transform"))
    {
        return;
    }
    const char* ids[] = {"BOSS_VALTAN", "BOSS_VALTAN_GHOST"};
    // Combo는 선택 결과를 0(본체) 또는 1(유령)로 기록한다. 같은 index로 archetype과 두 draft 중 하나를 고른다.
    ImGui::Combo("Axe owner", &m_iSelectedAxe, "Valtan\0Ghost Valtan\0");
    auto& draft = m_AxeDrafts[m_iSelectedAxe];
    const char* archetype = ids[m_iSelectedAxe];
    // attempted는 편집 시도가 아니라 최초 로드 시도 여부다. 실패해도 매 프레임 읽지 않고 Reload를 기다린다.
    if (!draft.attempted)
    {
        (void)Load_AxeDraft(archetype, draft);
    }
    // canonical은 편집 기준이 되는 Data 정본이다. 같은 발탄 정본의 저장·게시 중에는 새 편집·재로드를 막는다.
    const bool busy = m_pBalanceTool && (m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
                                         m_pBalanceTool->Is_ServerRuntimeSetPublishRunning());
    ImGui::TextWrapped(
        "Hand socket b_wp_r_01. Position uses socket axes in metres before boss scale; rotation is pitch / yaw / roll. Changes appear immediately on this Client.");
    ImGui::BeginDisabled(!draft.loaded || busy);
    // 두 입력을 임시 값으로 받은 뒤 카탈로그가 검증·적용한 경우에만 편집값을 확정한다. saved는 Save 전까지 유지한다.
    auto candidate = draft.value;
    bool changed = ImGui::DragFloat3("Position XYZ (m)##ValtanAxe",
                                     &candidate.positionMeters.x,
                                     0.01f,
                                     -10.f,
                                     10.f,
                                     "%.3f",
                                     ImGuiSliderFlags_AlwaysClamp);
    changed |= ImGui::DragFloat3("Rotation XYZ (deg)##ValtanAxe",
                                 &candidate.rotationDegrees.x,
                                 0.5f,
                                 -360.f,
                                 360.f,
                                 "%.2f",
                                 ImGuiSliderFlags_AlwaysClamp);
    if (changed && CActorCatalog::Set_ValtanWeaponSocketTransform(archetype, candidate, draft.status))
    {
        draft.value = candidate;
    }
    // Reset은 0 보정으로 preview를 바꾼다. 디스크 값을 다시 읽는 Reload나 저장하는 Save와 다르다.
    if (ImGui::Button("Reset axes##ValtanAxe"))
    {
        if (CActorCatalog::Set_ValtanWeaponSocketTransform(archetype, {}, draft.status))
        {
            draft.value = {};
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Save##ValtanAxe"))
    {
        (void)Save_AxeDraft(archetype, draft);
    }
    ImGui::SameLine();
    // 정본 저장 성공을 확인한 뒤 최신 발탄 source revision으로 게시를 요청한다. 게시 시작 실패와 저장 실패를 구분한다.
    if (ImGui::Button("Save + Publish##ValtanAxe"))
    {
        if (Save_AxeDraft(archetype, draft))
        {
            // BalanceTool은 MainApp이 소유한다. 차용 포인터가 없으면 저장 성공을 유지하고 게시 시작만 거부한다.
            std::string revision, publishStatus;
            if (!m_pBalanceTool)
            {
                publishStatus = "Balance Tool is unavailable.";
            }
            if (m_pBalanceTool && m_pBalanceTool->Ensure_Initialized() &&
                m_pBalanceTool->Reload_ValtanSource(publishStatus) &&
                m_pBalanceTool->Get_ValtanPublishSourceRevision(revision, publishStatus) &&
                m_pBalanceTool->Publish_ServerRuntimeSet(revision, publishStatus))
            {
                draft.status = "Axe saved. " + publishStatus;
            }
            else
            {
                draft.status = "Axe saved; Publish did not start. " + publishStatus;
            }
        }
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(busy);
    if (ImGui::Button("Reload saved##ValtanAxe") && Load_AxeDraft(archetype, draft))
    {
        (void)CActorCatalog::Set_ValtanWeaponSocketTransform(archetype, draft.value, draft.status);
    }
    ImGui::EndDisabled();
    if (draft.loaded && !Equal(draft.value, draft.saved))
    {
        ImGui::TextDisabled("Unsaved axe changes");
    }
    ImGui::TextWrapped("%s", draft.status.c_str());
    if (m_pBalanceTool)
    {
        std::string publishStatus;
        double elapsed = 0.0;
        if (m_pBalanceTool->Get_ServerRuntimeSetPublishState(publishStatus, elapsed) !=
            CBalanceTool::SERVER_RUNTIME_PUBLISH_STATE::IDLE)
        {
            ImGui::TextWrapped("%s", publishStatus.c_str());
        }
    }
    ImGui::TextWrapped(
        "Publish stores the same catalog in the Server presentation generation. Restart Server and re-enter after successful Publish. Damage and collision remain owned by encounter data.");
    // 펼쳐진 TreeNode와 짝을 맞춘다. 앞의 접힘 반환 경로에는 TreePop이 필요하지 않다.
    ImGui::TreePop();
}

NS_END
