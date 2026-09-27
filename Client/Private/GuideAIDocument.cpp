#include "GuideAIDocument.h"
#include "ProjectDataRoot.h"
#include <fstream>
#include <iomanip>
#include <iterator>
#include <locale>
#include <sstream>

namespace Client
{
namespace GuideJson
{
const J& Field(const J& value, const char* name) { static const J empty; const auto* field = value.Find(name); return field ? *field : empty; }
std::string String(const J& value, const char* name) { const auto& field = Field(value, name); return field.Is_String() ? field.Get_String() : std::string{}; }
double Number(const J& value, const char* name, double fallback) { const auto& field = Field(value, name); return field.Is_Number() ? field.Get_Number() : fallback; }
bool Boolean(const J& value, const char* name, bool fallback) { const auto& field = Field(value, name); return field.Is_Boolean() ? field.Get_Boolean() : fallback; }
void Set(J& value, const char* name, J field) { auto fields = value.Get_Object(); fields[name] = std::move(field); value = J::Object(std::move(fields)); }
}
using namespace GuideJson;
std::string CGuideAIDocument::Serialize(const J& value)
{
    std::ostringstream out; out.imbue(std::locale::classic()); out << std::setprecision(17);
    const auto write = [&](const auto& self, const J& item) -> void {
        switch (item.Get_Type())
        {
        case DATA_JSON_TYPE::NULL_VALUE: out << "null"; break;
        case DATA_JSON_TYPE::BOOLEAN: out << (item.Get_Boolean() ? "true" : "false"); break;
        case DATA_JSON_TYPE::NUMBER: out << item.Get_Number(); break;
        case DATA_JSON_TYPE::STRING: out << '"' << CDataJson::Escape(item.Get_String()) << '"'; break;
        case DATA_JSON_TYPE::ARRAY: out << '['; for (size_t i = 0; i < item.Get_Array().size(); ++i) { if (i) out << ','; self(self, item.Get_Array()[i]); } out << ']'; break;
        case DATA_JSON_TYPE::OBJECT: out << '{'; { bool first = true; for (const auto& [key, child] : item.Get_Object()) { if (!first) out << ','; first = false; out << '"' << CDataJson::Escape(key) << "\":"; self(self, child); } } out << '}'; break;
        }
    }; write(write, value); return out.str();
}
bool CGuideAIDocument::Read(const std::filesystem::path& path, J& value, std::string& status)
{
    std::error_code ec; const auto length = std::filesystem::file_size(path, ec);
    if (ec || length > 4u * 1024u * 1024u) { status = "Guide source is missing or exceeds 4 MiB: " + path.filename().string(); return false; }
    std::ifstream input(path, std::ios::binary); std::string bytes{std::istreambuf_iterator<char>(input), {}};
    if (!input || bytes.size() != length) { status = "Guide source changed while reading."; return false; }
    return CDataJson::Parse(bytes, value, status) && value.Is_Object();
}
CGuideAIDocument::~CGuideAIDocument() { if (m_Process) CloseHandle(m_Process); }
bool CGuideAIDocument::Is_Dirty() const { return m_Loaded && Serialize(m_Draft) != Serialize(m_Baseline); }
bool CGuideAIDocument::Load()
{
    if (Is_Busy() || Is_Dirty()) { m_Status = "Save the current draft before Load. Existing draft preserved."; return false; }
    J candidate;
    if (!Read_Source(candidate)) return false;
    m_LoadCandidate = std::move(candidate);
    return Start(L"Validate");
}
bool CGuideAIDocument::Read_Source(J& candidate)
{
    const std::pair<const char*, const char*> files[] = {{"catalog","GuideCatalog.json"},{"placement","DimensionMaster/Placement.json"},{"prompts","DimensionMaster/Prompts.json"},{"triggers","DimensionMaster/Triggers.json"},{"combat","DimensionMaster/Combat.json"}};
    J::OBJECT values;
    for (const auto& [key, file] : files)
    {
        J value;
        if (!Read(CProjectDataRoot::Resolve(std::filesystem::path("Guide") / file), value, m_Status)) return false;
        if (String(value, "schema") != std::string("lostark.guide-") + key || Number(value, "formatVersion") != 1)
        { m_Status = "Guide schema mismatch. Previous document preserved."; return false; }
        values[key] = std::move(value);
    }
    candidate = J::Object(std::move(values));
    if (!Field(Field(candidate,"catalog"),"categories").Is_Array() || Field(Field(candidate,"catalog"),"categories").Get_Array().empty() ||
        !Field(Field(candidate,"prompts"),"prompts").Is_Array() || !Field(Field(candidate,"triggers"),"triggers").Is_Array() ||
        !Field(Field(candidate,"combat"),"combos").Is_Array() || !Field(Field(candidate,"combat"),"commands").Is_Array())
    { m_Status = "Guide document collections are invalid. Previous draft preserved."; return false; }
    return true;
}
bool CGuideAIDocument::Start_Save() { return m_Loaded && Start(L"Save"); }
bool CGuideAIDocument::Start_Publish()
{
    if (Is_Dirty()) { m_Status = "Save the Guide draft before Publish."; return false; }
    J current;
    if (!Read_Source(current) || Serialize(current) != Serialize(m_Baseline))
    { m_Status = "Guide source changed since Load. Load its latest saved version before Publish."; return false; }
    return m_Loaded && Start(L"Publish");
}
void CGuideAIDocument::Read_PublishedRevision()
{
    J published; std::string ignored;
    m_PublishedRevision = Read(CProjectDataRoot::Get().parent_path()/"Client/Bin/DataFiles/Guide/Guide.runtime.json",published,ignored)
        && String(published,"schema")=="lostark.guide-runtime" ? static_cast<uint64_t>(Number(published,"revision")) : 0;
}
bool CGuideAIDocument::Start(const wchar_t* mode)
{
    if (Is_Busy()) return false;
    const auto root = CProjectDataRoot::Get().parent_path();
    const auto script = root / L"Tools/GuidePipeline/Publish-Guide.ps1";
    if (!std::filesystem::exists(script)) { m_Status = "Guide publisher script is missing."; return false; }
    wchar_t temp[MAX_PATH]{}; if (!GetTempPathW(MAX_PATH, temp)) return false;
    const std::wstring tag = L"LostArkGuide." + std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(GetTickCount64());
    m_LogPath = std::filesystem::path(temp) / (tag + L".log");
    m_BaselinePath = std::filesystem::path(temp) / (tag + L".baseline.json"); m_DraftPath = std::filesystem::path(temp) / (tag + L".draft.json");
    if (std::wstring(mode) == L"Save")
    {
        std::ofstream before(m_BaselinePath, std::ios::binary), after(m_DraftPath, std::ios::binary);
        before << Serialize(m_Baseline); after << Serialize(m_Draft); before.flush(); after.flush();
        if (!before || !after) { m_Status = "Could not stage Guide save. Draft preserved."; return false; }
    }
    wchar_t system[MAX_PATH]{}; GetSystemDirectoryW(system, MAX_PATH);
    const auto executable = std::filesystem::path(system) / L"WindowsPowerShell/v1.0/powershell.exe";
    std::wstring command = L"\"" + executable.wstring() + L"\" -NoProfile -ExecutionPolicy Bypass -File \"" + script.wstring() + L"\" -RepositoryRoot \"" + root.wstring() + L"\" -Mode " + mode;
    if (std::wstring(mode) == L"Save") command += L" -BaselinePath \"" + m_BaselinePath.wstring() + L"\" -DraftPath \"" + m_DraftPath.wstring() + L"\"";
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE output = CreateFileW(m_LogPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (output == INVALID_HANDLE_VALUE || input == INVALID_HANDLE_VALUE) { if(output != INVALID_HANDLE_VALUE) CloseHandle(output); if(input != INVALID_HANDLE_VALUE) CloseHandle(input); m_Status = "Guide publisher log could not be opened."; return false; }
    STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW; startup.wShowWindow = SW_HIDE; startup.hStdOutput = startup.hStdError = output; startup.hStdInput = input;
    PROCESS_INFORMATION process{};
    const bool started = CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, root.c_str(), &startup, &process) != FALSE;
    CloseHandle(output); CloseHandle(input);
    if (!started) { m_Status = "Guide publisher could not start; draft preserved."; return false; }
    CloseHandle(process.hThread); m_Process = process.hProcess; m_Mode = mode; m_Status = "Guide validation and transaction running..."; return true;
}
void CGuideAIDocument::Poll()
{
    if (!m_Process || WaitForSingleObject(m_Process, 0) != WAIT_OBJECT_0) return;
    DWORD exitCode = 1; GetExitCodeProcess(m_Process, &exitCode); CloseHandle(m_Process); m_Process = nullptr;
    std::ifstream log(m_LogPath, std::ios::binary); const std::string details{std::istreambuf_iterator<char>(log), {}};
    const auto mode = m_Mode;
    std::error_code ec; std::filesystem::remove(m_BaselinePath, ec); std::filesystem::remove(m_DraftPath, ec);
    if (exitCode != 0) { m_Status = "Guide transaction failed; draft preserved. " + details; return; }
    if (mode == L"Validate")
    {
        J current;
        if (!Read_Source(current) || Serialize(current) != Serialize(m_LoadCandidate))
        { m_Status = "Guide sources changed during Load validation. Previous draft preserved; retry Load."; return; }
        m_Draft = std::move(current); m_Baseline = m_Draft; m_Loaded = true;
        Read_PublishedRevision();
        m_Status = "Guide source loaded and validated. Save changes source; Publish installs the saved revision.";
    }
    else if (mode == L"Save") { m_Baseline = m_Draft; (void)Load(); }
    else { Read_PublishedRevision(); m_Status = "Guide published. Server must reload/restart to consume the new revision."; }
}
}
