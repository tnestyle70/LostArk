#include "RecoveryCameraAuthoringSession.h"
#include "ProjectDataRoot.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <locale>
#include <set>
#include <sstream>

namespace Client
{
namespace
{
using J = DATA_JSON_VALUE;
std::string Serialize(const J& value)
{
    std::ostringstream out; out.imbue(std::locale::classic()); out << std::setprecision(17);
    const auto write = [&](const auto& self, const J& item) -> void {
        switch (item.Get_Type())
        {
        case DATA_JSON_TYPE::NULL_VALUE: out << "null"; break;
        case DATA_JSON_TYPE::BOOLEAN: out << (item.Get_Boolean() ? "true" : "false"); break;
        case DATA_JSON_TYPE::NUMBER: out << item.Get_Number(); break;
        case DATA_JSON_TYPE::STRING: out << '"' << CDataJson::Escape(item.Get_String()) << '"'; break;
        case DATA_JSON_TYPE::ARRAY:
            out << '['; for (std::size_t i = 0; i < item.Get_Array().size(); ++i) { if (i) out << ','; self(self, item.Get_Array()[i]); } out << ']'; break;
        case DATA_JSON_TYPE::OBJECT:
            out << '{'; { bool first = true; for (const auto& [key, child] : item.Get_Object())
            { if (!first) out << ','; first = false; out << '"' << CDataJson::Escape(key) << "\":"; self(self, child); } } out << '}'; break;
        }
    }; write(write, value); return out.str();
}
bool Equal(const J* a, const J* b) { return (!a || !b) ? a == b : Serialize(*a) == Serialize(*b); }
const J* ById(const J::ARRAY& values, const std::string& id, const char* field)
{
    for (const auto& value : values) if (const auto* name = value.Find(field); name && name->Is_String() && name->Get_String() == id) return &value;
    return nullptr;
}
// Rebase changed fields only. Row/key identity replaces positional array indices.
bool MergeValue(const J* before, const J* draft, const J* current, J& out, const std::string& path, std::string& status)
{
    if (Equal(before, draft)) { out = current ? *current : J::Null(); return true; }
    if (Equal(current, before) || Equal(current, draft)) { out = draft ? *draft : J::Null(); return true; }
    if (before && draft && current && before->Is_Object() && draft->Is_Object() && current->Is_Object())
    {
        auto fields = current->Get_Object(); std::set<std::string> names;
        for (const auto& [name, value] : before->Get_Object()) names.insert(name);
        for (const auto& [name, value] : draft->Get_Object()) names.insert(name);
        for (const auto& name : names)
        {
            if (Equal(before->Find(name), draft->Find(name))) continue;
            J value;
            if (!MergeValue(before->Find(name), draft->Find(name), current->Find(name), value, path + "/" + name, status)) return false;
            if (!draft->Find(name)) fields.erase(name); else fields[name] = std::move(value);
        }
        out = J::Object(std::move(fields), current->Get_ObjectInsertionOrder()); return true;
    }
    if (before && draft && current && before->Is_Array() && draft->Is_Array() && current->Is_Array() && path.ends_with("/keys"))
    {
        auto values = current->Get_Array(); std::set<std::string> ids;
        for (const auto* source : {before, draft}) for (const auto& item : source->Get_Array())
            if (const auto* id = item.Find("keyId"); id && id->Is_String()) ids.insert(id->Get_String());
        for (const auto& id : ids)
        {
            const auto* old = ById(before->Get_Array(), id, "keyId"); const auto* next = ById(draft->Get_Array(), id, "keyId");
            if (Equal(old, next)) continue;
            const auto* now = ById(current->Get_Array(), id, "keyId"); J value;
            if (!MergeValue(old, next, now, value, path + "/" + id, status)) return false;
            auto found = std::find_if(values.begin(), values.end(), [&](const auto& item) { return item.Find("keyId")->Get_String() == id; });
            if (!next) { if (found != values.end()) values.erase(found); }
            else if (found == values.end()) values.push_back(std::move(value)); else *found = std::move(value);
        }
        std::stable_sort(values.begin(), values.end(), [](const auto& a, const auto& b) { return a.Find("timeMs")->Get_Number() < b.Find("timeMs")->Get_Number(); });
        out = J::Array(std::move(values)); return true;
    }
    status = "Camera save conflict at " + path + "; disk and draft were preserved."; return false;
}
bool Read(const std::filesystem::path& path, std::string& bytes, J& document, std::string& status)
{
    std::error_code ec; const auto size = std::filesystem::file_size(path, ec);
    if (ec || !size || size > 1024u * 1024u) { status = "Recovery camera source is missing or exceeds 1 MiB."; return false; }
    std::ifstream input(path, std::ios::binary); bytes.assign(std::istreambuf_iterator<char>(input), {});
    if (!input || bytes.size() != size) { status = "Recovery camera source changed while reading."; return false; }
    return CDataJson::Parse(bytes, document, status);
}
struct FileHandle final { HANDLE value = INVALID_HANDLE_VALUE; ~FileHandle() { if (value != INVALID_HANDLE_VALUE) CloseHandle(value); } };
}
bool CRecoveryCameraAuthoringSession::Open(const std::string& id, std::string& status)
{
    if (m_Dirty) { status = "Save the current camera draft before opening another source."; return false; }
    EFFECT_RECOVERY_CAMERA_DOCUMENT source;
    if (!CEffectRecoveryCamera::Load(id, source, status)) return false;
    if (!source.exists || source.rows.empty()) { status = "This Effect has no canonical recovery camera."; return false; }
    m_EffectId = id; m_Baseline = std::move(source.document); m_Rows = std::move(source.rows); m_Dirty = false;
    std::stable_sort(m_Rows.begin(), m_Rows.end(), [](const auto& a, const auto& b) { return a.startMs < b.startMs; });
    status = "Opened Product recovery cameras. Save writes only their changed fields."; return true;
}
bool CRecoveryCameraAuthoringSession::Apply(const EFFECT_CAMERA_ROW& row, std::string& status)
{
    auto rows = m_Rows; const auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& old) { return old.id == row.id; });
    if (found == rows.end()) { status = "Camera identity is no longer available."; return false; }
    if (Serialize(CEffectRecoveryCamera::Write_Row(*found)) == Serialize(CEffectRecoveryCamera::Write_Row(row))) return true;
    *found = row;
    if (!CEffectRecoveryCamera::Validate(rows, status)) return false;
    m_Rows = std::move(rows); m_Dirty = true; return true;
}
bool CRecoveryCameraAuthoringSession::Merge(const J& baseline, const std::vector<EFFECT_CAMERA_ROW>& rows,
    const J& current, J& merged, std::string& status)
{
    std::vector<EFFECT_CAMERA_ROW> oldRows, nowRows;
    if (!CEffectRecoveryCamera::Parse(baseline, true, oldRows, status) || !CEffectRecoveryCamera::Parse(current, true, nowRows, status) ||
        !CEffectRecoveryCamera::Validate(rows, status)) return false;
    const auto& before = baseline.Find("cameras")->Get_Array(); auto cameras = current.Find("cameras")->Get_Array();
    if (rows.size() != before.size()) { status = "Camera-only save preserves the canonical cut list."; return false; }
    for (const auto& row : rows)
    {
        const auto* old = ById(before, row.id, "cameraId");
        if (!old) { status = "Unknown canonical camera identity."; return false; }
        const auto typedOld = std::find_if(oldRows.begin(), oldRows.end(), [&](const auto& value) { return value.id == row.id; });
        const auto original = CEffectRecoveryCamera::Write_Row(*typedOld, old);
        const auto draft = CEffectRecoveryCamera::Write_Row(row, old);
        if (Equal(&original, &draft)) continue;
        const auto* now = ById(current.Find("cameras")->Get_Array(), row.id, "cameraId");
        J replacement;
        if (!MergeValue(old, &draft, now, replacement, "camera/" + row.id, status)) return false;
        auto found = std::find_if(cameras.begin(), cameras.end(), [&](const auto& item) { return item.Find("cameraId")->Get_String() == row.id; });
        if (found == cameras.end()) { status = "The edited camera was removed by another editor."; return false; }
        *found = std::move(replacement);
    }
    auto fields = current.Get_Object(); fields["cameras"] = J::Array(std::move(cameras));
    auto candidate = J::Object(std::move(fields), current.Get_ObjectInsertionOrder());
    std::vector<EFFECT_CAMERA_ROW> check;
    if (!CEffectRecoveryCamera::Parse(candidate, true, check, status)) return false;
    merged = std::move(candidate); return true;
}
bool CRecoveryCameraAuthoringSession::Save_File(const std::filesystem::path& path, const std::string& id,
    const J& baseline, const std::vector<EFFECT_CAMERA_ROW>& rows, EFFECT_RECOVERY_CAMERA_DOCUMENT& saved, std::string& status)
{
    FileHandle lock; const auto lockPath = path.wstring() + L".camera-save.lock";
    lock.value = CreateFileW(lockPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
    if (lock.value == INVALID_HANDLE_VALUE) { status = "Another camera save owns this source."; return false; }
    std::string bytes; J current, merged; EFFECT_RECOVERY_CAMERA_DOCUMENT check;
    if (!Read(path, bytes, current, status) || !CEffectRecoveryCamera::Parse_Document(id, current, check, status) ||
        !Merge(baseline, rows, current, merged, status) || !CEffectRecoveryCamera::Parse_Document(id, merged, check, status)) return false;
    const std::string text = Serialize(merged) + "\n";
    if (text.size() > 1024u * 1024u) { status = "Camera source exceeds 1 MiB."; return false; }
    J emitted;
    if (!CDataJson::Parse(text, emitted, status) || !CEffectRecoveryCamera::Parse_Document(id, emitted, check, status)) return false;
    const auto suffix = L".camera." + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(GetTickCount64());
    const std::filesystem::path temporary = path.wstring() + suffix + L".tmp", backup = path.wstring() + suffix + L".bak";
    FileHandle output; output.value = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    DWORD written = 0;
    const bool flushed = output.value != INVALID_HANDLE_VALUE && WriteFile(output.value, text.data(), static_cast<DWORD>(text.size()), &written, nullptr) &&
        written == text.size() && FlushFileBuffers(output.value);
    if (output.value != INVALID_HANDLE_VALUE) { CloseHandle(output.value); output.value = INVALID_HANDLE_VALUE; }
    std::error_code ec; const auto cleanup = [&]() { std::filesystem::remove(temporary, ec); };
    std::string latest; J latestJson;
    if (!flushed || !Read(path, latest, latestJson, status) || latest != bytes)
    { cleanup(); status = "Camera source changed before atomic replacement; draft preserved."; return false; }
    if (!ReplaceFileW(path.c_str(), temporary.c_str(), backup.c_str(), 0u, nullptr, nullptr))
    { cleanup(); status = "Camera atomic replacement failed; draft preserved."; return false; }
    std::string displaced; J ignored;
    if (!Read(backup, displaced, ignored, status) || displaced != bytes)
    {
        std::string installed;
        if (Read(path, installed, ignored, status) && installed == text)
        {
            const std::filesystem::path recovery = path.wstring() + suffix + L".recovery";
            if (ReplaceFileW(path.c_str(), backup.c_str(), recovery.c_str(), 0u, nullptr, nullptr))
            {
                std::string recovered;
                if (Read(recovery, recovered, ignored, status) && recovered == text) std::filesystem::remove(recovery, ec);
            }
        }
        status = "Concurrent write during camera replacement. Draft retained; conflict backups are beside the source."; return false;
    }
    std::filesystem::remove(backup, ec); saved = std::move(check);
    status = "Camera source saved. Publish applies the saved cameras to Client playback."; return true;
}
bool CRecoveryCameraAuthoringSession::Save(std::string& status)
{
    EFFECT_RECOVERY_CAMERA_DOCUMENT saved;
    if (!Save_File(CProjectDataRoot::Resolve(std::filesystem::path("Effects/Sequences") / (m_EffectId + ".effectsequence.json")),
        m_EffectId, m_Baseline, m_Rows, saved, status)) return false;
    m_Baseline = std::move(saved.document); m_Rows = std::move(saved.rows); m_Dirty = false;
    std::stable_sort(m_Rows.begin(), m_Rows.end(), [](const auto& a, const auto& b) { return a.startMs < b.startMs; });
    return true;
}
}
