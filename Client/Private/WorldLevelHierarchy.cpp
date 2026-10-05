#include "WorldLevelHierarchy.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <set>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "DataJson.h"

namespace
{
constexpr size_t MaximumFolders = 4096u;
constexpr size_t MaximumAssignments = 131072u;
constexpr size_t MaximumBytes = 16u * 1024u * 1024u;
constexpr size_t MaximumUndo = 64u;
using Folder = Client::CWorldLevelHierarchy::FOLDER;
using Json = Client::DATA_JSON_VALUE;

bool StableId(const std::string& text)
{
    return !text.empty() && text.size() <= 128u &&
        std::all_of(text.begin(), text.end(), [](unsigned char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
        });
}

bool DisplayText(const std::string& text, size_t maximum)
{
    return !text.empty() && text.size() <= maximum &&
        std::none_of(text.begin(), text.end(), [](unsigned char c) { return c < 0x20u || c == 0x7fu; }) &&
        std::any_of(text.begin(), text.end(), [](unsigned char c) { return c != ' '; }) &&
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), nullptr, 0) > 0;
}

bool Shape(const Json& value, std::initializer_list<const char*> fields)
{
    if (!value.Is_Object() || value.Get_Object().size() != fields.size()) return false;
    return std::all_of(fields.begin(), fields.end(), [&](const char* field) { return value.Find(field) != nullptr; });
}

bool StringField(const Json& value, const char* field, std::string& out)
{
    const auto* member = value.Find(field);
    if (!member || !member->Is_String()) return false;
    out = member->Get_String();
    return true;
}

bool ReadSource(const std::filesystem::path& path, bool& exists, std::string& bytes, std::string& status)
{
    std::error_code error;
    exists = std::filesystem::exists(path, error);
    if (error) { status = "Hierarchy source cannot be inspected: " + error.message(); return false; }
    bytes.clear();
    if (!exists) return true;
    if (!std::filesystem::is_regular_file(path, error) || error)
    { status = "Hierarchy source is not a regular file."; return false; }
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > MaximumBytes)
    { status = "Hierarchy source cannot be read or exceeds 16 MiB."; return false; }
    std::ifstream input(path, std::ios::binary);
    if (!input) { status = "Hierarchy source cannot be opened."; return false; }
    bytes.resize(static_cast<size_t>(size));
    input.read(bytes.data(), static_cast<std::streamsize>(size));
    if (input.gcount() != static_cast<std::streamsize>(size) || input.bad() || input.peek() != std::char_traits<char>::eof())
    { status = "Hierarchy source changed while reading; existing draft preserved."; return false; }
    return true;
}

struct WriterLease final
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    ~WriterLease() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
    bool Open(const std::filesystem::path& path)
    {
        const auto lock = path.wstring() + L".writer.lock";
        handle = CreateFileW(lock.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
            CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
        return handle != INVALID_HANDLE_VALUE;
    }
};

std::string UniqueSuffix()
{
    static std::atomic<uint64_t> serial{0u};
    FILETIME time{};
    GetSystemTimeAsFileTime(&time);
    const auto ticks = (uint64_t(time.dwHighDateTime) << 32u) | time.dwLowDateTime;
    return "." + std::to_string(GetCurrentProcessId()) + "." + std::to_string(ticks) +
        "." + std::to_string(++serial);
}

struct TemporaryFile final
{
    std::filesystem::path path;
    bool owned = false;
    ~TemporaryFile()
    {
        if (owned) { std::error_code ignored; std::filesystem::remove(path, ignored); }
    }
    bool Write(const std::string& bytes)
    {
        const HANDLE handle = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle == INVALID_HANDLE_VALUE) return false;
        owned = true;
        DWORD written = 0u;
        const bool success = WriteFile(handle, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) &&
            written == bytes.size() && FlushFileBuffers(handle);
        const bool closed = CloseHandle(handle) != FALSE;
        return success && closed;
    }
};

const Folder* FindFolder(const std::vector<Folder>& folders, const std::string& id)
{
    const auto found = std::find_if(folders.begin(), folders.end(), [&](const auto& folder) { return folder.id == id; });
    return found == folders.end() ? nullptr : &*found;
}

template<class Value>
bool MergeValue(const std::optional<Value>& before, const std::optional<Value>& local,
    const std::optional<Value>& disk, std::optional<Value>& result)
{
    if (local == before) result = disk;
    else if (disk == before || local == disk) result = local;
    else return false;
    return true;
}
}

namespace Client
{
bool CWorldLevelHierarchy::Validate(const STATE& state, std::string& status)
{
    if (state.folders.size() > MaximumFolders || state.assignments.size() > MaximumAssignments)
    { status = "Hierarchy exceeds its folder or assignment limit."; return false; }
    std::unordered_map<std::string, const FOLDER*> folders;
    for (const auto& folder : state.folders)
    {
        if (!StableId(folder.id) || !DisplayText(folder.name, 128u) ||
            (!folder.parent.empty() && !StableId(folder.parent)) || !folders.emplace(folder.id, &folder).second)
        { status = "Hierarchy has an invalid or duplicate folder: " + folder.id; return false; }
    }
    for (const auto& folder : state.folders)
    {
        std::unordered_set<std::string> visited{folder.id};
        std::string parent = folder.parent;
        size_t depth = 0u;
        while (!parent.empty())
        {
            const auto found = folders.find(parent);
            if (found == folders.end())
            { status = "Hierarchy parent does not exist: " + parent; return false; }
            if (!visited.insert(parent).second || ++depth > 64u)
            { status = "Hierarchy contains a cycle or more than 64 parents: " + folder.id; return false; }
            parent = found->second->parent;
        }
    }
    for (const auto& [key, parent] : state.assignments)
    {
        if (!DisplayText(key, 512u) || parent.empty() || !folders.contains(parent))
        { status = "Hierarchy has an invalid row assignment: " + key; return false; }
    }
    return true;
}

bool CWorldLevelHierarchy::Parse(const std::string& bytes, const std::string& areaId,
    STATE& out, std::string& status)
{
    Json root;
    if (!CDataJson::Parse(bytes, root, status)) return false;
    if (!Shape(root, {"schema", "version", "areaId", "folders", "assignments"}))
    { status = "Hierarchy root fields are invalid."; return false; }
    std::string schema, area;
    const auto* version = root.Find("version");
    const auto* folders = root.Find("folders");
    const auto* assignments = root.Find("assignments");
    if (!StringField(root, "schema", schema) || schema != "lostark.world-level-hierarchy" ||
        !StringField(root, "areaId", area) || area != areaId || !version->Is_Number() || version->Get_Number() != 1. ||
        !folders->Is_Array() || !assignments->Is_Array() ||
        folders->Get_Array().size() > MaximumFolders || assignments->Get_Array().size() > MaximumAssignments)
    { status = "Hierarchy schema, version, Area or array limits are invalid."; return false; }
    STATE staged;
    for (const auto& row : folders->Get_Array())
    {
        FOLDER folder;
        if (!Shape(row, {"id", "name", "parent"}) || !StringField(row, "id", folder.id) ||
            !StringField(row, "name", folder.name) || !StringField(row, "parent", folder.parent))
        { status = "Hierarchy folder fields are invalid."; return false; }
        staged.folders.push_back(std::move(folder));
    }
    std::unordered_set<std::string> keys;
    for (const auto& row : assignments->Get_Array())
    {
        std::string key, parent;
        if (!Shape(row, {"key", "parent"}) || !StringField(row, "key", key) ||
            !StringField(row, "parent", parent) || !DisplayText(key, 512u) || !keys.insert(key).second)
        { status = "Hierarchy assignment fields or duplicate row keys are invalid."; return false; }
        if (!parent.empty()) staged.assignments.emplace(std::move(key), std::move(parent));
    }
    if (!Validate(staged, status)) return false;
    out = std::move(staged);
    return true;
}

std::string CWorldLevelHierarchy::Serialize(const STATE& state, const std::string& areaId)
{
    const auto quote = [](const std::string& value) { return "\"" + CDataJson::Escape(value) + "\""; };
    std::ostringstream output;
    output << "{\n  \"schema\": \"lostark.world-level-hierarchy\",\n  \"version\": 1,\n  \"areaId\": " << quote(areaId)
        << ",\n  \"folders\": [";
    for (size_t index = 0u; index < state.folders.size(); ++index)
    {
        const auto& folder = state.folders[index];
        output << (index ? ",\n" : "\n") << "    {\"id\": " << quote(folder.id) << ", \"name\": " << quote(folder.name)
            << ", \"parent\": " << quote(folder.parent) << "}";
    }
    output << (state.folders.empty() ? "],\n" : "\n  ],\n") << "  \"assignments\": [";
    size_t index = 0u;
    for (const auto& [key, parent] : state.assignments)
        output << (index++ ? ",\n" : "\n") << "    {\"key\": " << quote(key) << ", \"parent\": " << quote(parent) << "}";
    output << (state.assignments.empty() ? "]\n" : "\n  ]\n") << "}\n";
    return output.str();
}

bool CWorldLevelHierarchy::Load(const std::filesystem::path& path, const std::string& areaId, std::string& status)
{
    if (path.empty() || !StableId(areaId))
    { status = "Hierarchy path or Area ID is invalid; existing draft preserved."; return false; }
    bool exists = false;
    std::string bytes;
    STATE staged;
    if (!ReadSource(path, exists, bytes, status) || (exists && !Parse(bytes, areaId, staged, status))) return false;
    m_Path = path;
    m_AreaId = areaId;
    m_BaselineBytes = std::move(bytes);
    m_Draft = staged;
    m_Baseline = std::move(staged);
    m_BaselineExists = exists;
    m_Loaded = true;
    m_Dirty = false;
    m_Undo.clear(); m_Redo.clear(); m_Selection = {};
    status = exists ? "World hierarchy loaded." : "No saved World hierarchy; a new empty hierarchy is ready.";
    return true;
}

bool CWorldLevelHierarchy::Is_Dirty() const { return m_Dirty; }

std::string CWorldLevelHierarchy::Get_Parent(const std::string& rowKey) const
{
    const auto found = m_Draft.assignments.find(rowKey);
    return found == m_Draft.assignments.end() ? std::string{} : found->second;
}

bool CWorldLevelHierarchy::Commit(STATE candidate, const char* message, std::string& status)
{
    if (!m_Loaded) { status = "Load an Area hierarchy first."; return false; }
    if (!Validate(candidate, status)) return false;
    if (candidate == m_Draft) { status = "Hierarchy is unchanged."; return true; }
    m_Undo.push_back({m_Draft, m_Selection});
    m_Redo.clear();
    if (m_Undo.size() > MaximumUndo) m_Undo.erase(m_Undo.begin());
    m_Draft = std::move(candidate);
    m_Dirty = m_Draft != m_Baseline;
    status = message;
    return true;
}

bool CWorldLevelHierarchy::Create_Folder(const std::string& name, const std::string& parent,
    std::string& newId, std::string& status)
{
    auto candidate = m_Draft;
    const auto suffix = UniqueSuffix();
    const std::string id = "world.level.folder" + suffix;
    candidate.folders.push_back({id, name, parent});
    if (!Commit(std::move(candidate), "Parent created. Save hierarchy to keep the organization.", status)) return false;
    newId = id;
    return true;
}

bool CWorldLevelHierarchy::Rename_Folder(const std::string& id, const std::string& name, std::string& status)
{
    auto candidate = m_Draft;
    const auto found = std::find_if(candidate.folders.begin(), candidate.folders.end(), [&](const auto& folder) { return folder.id == id; });
    if (found == candidate.folders.end()) { status = "The selected parent no longer exists."; return false; }
    found->name = name;
    return Commit(std::move(candidate), "Parent renamed. Save hierarchy to keep the organization.", status);
}

bool CWorldLevelHierarchy::Move_Folder(const std::string& id, const std::string& parent, std::string& status)
{
    auto candidate = m_Draft;
    const auto found = std::find_if(candidate.folders.begin(), candidate.folders.end(), [&](const auto& folder) { return folder.id == id; });
    if (found == candidate.folders.end()) { status = "The selected parent no longer exists."; return false; }
    found->parent = parent;
    return Commit(std::move(candidate), "Parent moved. Object transforms are unchanged.", status);
}

bool CWorldLevelHierarchy::Delete_Folder(const std::string& id, std::string& status)
{
    const auto* existing = FindFolder(m_Draft.folders, id);
    if (!existing) { status = "The selected parent no longer exists."; return false; }
    const std::string parent = existing->parent;
    auto candidate = m_Draft;
    std::erase_if(candidate.folders, [&](const auto& folder) { return folder.id == id; });
    for (auto& folder : candidate.folders) if (folder.parent == id) folder.parent = parent;
    for (auto it = candidate.assignments.begin(); it != candidate.assignments.end();)
    {
        if (it->second != id) { ++it; continue; }
        if (parent.empty()) it = candidate.assignments.erase(it);
        else { it->second = parent; ++it; }
    }
    return Commit(std::move(candidate), "Parent removed; its children and objects were moved to its parent.", status);
}

bool CWorldLevelHierarchy::Assign(const std::vector<std::string>& keys, const std::string& parent, std::string& status)
{
    if (!parent.empty() && !FindFolder(m_Draft.folders, parent))
    { status = "The destination parent no longer exists."; return false; }
    auto candidate = m_Draft;
    for (const auto& key : keys)
    {
        if (!DisplayText(key, 512u)) { status = "The selected row has an invalid stable key."; return false; }
        if (parent.empty()) candidate.assignments.erase(key);
        else candidate.assignments[key] = parent;
    }
    return Commit(std::move(candidate), "Objects organized. Their transforms and authoring owners are unchanged.", status);
}

bool CWorldLevelHierarchy::Undo(std::string& status)
{
    if (!m_Loaded || m_Undo.empty()) { status = "There is no unsaved hierarchy edit to undo."; return false; }
    m_Redo.push_back({m_Draft, m_Selection});
    m_Draft = std::move(m_Undo.back().state);
    m_Selection = std::move(m_Undo.back().selection);
    m_Undo.pop_back();
    m_Dirty = m_Draft != m_Baseline;
    status = "Hierarchy edit undone. Object transforms are unchanged.";
    return true;
}

bool CWorldLevelHierarchy::Redo(std::string& status)
{
    if (!m_Loaded || m_Redo.empty()) { status = "There is no hierarchy edit to redo."; return false; }
    m_Undo.push_back({m_Draft, m_Selection});
    m_Draft = std::move(m_Redo.back().state);
    m_Selection = std::move(m_Redo.back().selection);
    m_Redo.pop_back();
    m_Dirty = m_Draft != m_Baseline;
    status = "Hierarchy edit redone. Selection restored; object transforms are unchanged.";
    return true;
}

bool CWorldLevelHierarchy::Commit_Edit(const CWorldLevelHierarchy& staged, std::string& status)
{
    if (!m_Loaded || !staged.m_Loaded || m_Path != staged.m_Path || m_AreaId != staged.m_AreaId ||
        m_BaselineExists != staged.m_BaselineExists || m_BaselineBytes != staged.m_BaselineBytes ||
        m_Baseline != staged.m_Baseline)
    { status = "Hierarchy edit belongs to another source or baseline; current draft preserved."; return false; }
    return Commit(staged.m_Draft, "Selected branches organized. Object transforms are unchanged.", status);
}

bool CWorldLevelHierarchy::Merge(const STATE& baseline, const STATE& local, const STATE& disk,
    STATE& out, std::string& status)
{
    STATE merged;
    // Preserve disk order; new local folders are appended in their authored order.
    std::vector<std::string> folderIds;
    std::unordered_set<std::string> seen;
    for (const auto* state : {&disk, &local, &baseline})
        for (const auto& folder : state->folders) if (seen.insert(folder.id).second) folderIds.push_back(folder.id);
    for (const auto& id : folderIds)
    {
        const auto* before = FindFolder(baseline.folders, id);
        const auto* ours = FindFolder(local.folders, id);
        const auto* theirs = FindFolder(disk.folders, id);
        if (before && ours && theirs)
        {
            FOLDER folder{id, {}, {}};
            std::optional<std::string> name, parent;
            if (!MergeValue<std::string>(before->name, ours->name, theirs->name, name) ||
                !MergeValue<std::string>(before->parent, ours->parent, theirs->parent, parent))
            { status = "Hierarchy save conflict in parent " + id + "; draft and disk preserved."; return false; }
            folder.name = *name; folder.parent = *parent;
            merged.folders.push_back(std::move(folder));
        }
        else
        {
            const auto optional = [](const FOLDER* folder) { return folder ? std::optional<FOLDER>(*folder) : std::nullopt; };
            std::optional<FOLDER> folder;
            if (!MergeValue<FOLDER>(optional(before), optional(ours), optional(theirs), folder))
            { status = "Hierarchy save conflict in added/deleted parent " + id + "; draft and disk preserved."; return false; }
            if (folder) merged.folders.push_back(std::move(*folder));
        }
    }
    std::set<std::string> keys;
    for (const auto* state : {&baseline, &local, &disk})
        for (const auto& [key, parent] : state->assignments) keys.insert(key);
    const auto assigned = [](const STATE& state, const std::string& key) -> std::optional<std::string> {
        const auto found = state.assignments.find(key);
        return found == state.assignments.end() ? std::nullopt : std::optional<std::string>(found->second);
    };
    for (const auto& key : keys)
    {
        std::optional<std::string> parent;
        if (!MergeValue<std::string>(assigned(baseline, key), assigned(local, key), assigned(disk, key), parent))
        { status = "Hierarchy save conflict in row " + key + "; draft and disk preserved."; return false; }
        if (parent) merged.assignments.emplace(key, *parent);
    }
    if (!Validate(merged, status))
    { status = "Concurrent hierarchy edits cannot be combined: " + status + ". Draft and disk preserved."; return false; }
    out = std::move(merged);
    return true;
}

bool CWorldLevelHierarchy::Save(std::string& status)
{
    if (!m_Loaded) { status = "Load an Area hierarchy first."; return false; }
    if (!Validate(m_Draft, status)) return false;
    std::error_code error;
    if (!m_Path.parent_path().empty()) std::filesystem::create_directories(m_Path.parent_path(), error);
    if (error) { status = "Hierarchy directory could not be created; draft preserved."; return false; }
    WriterLease lease;
    if (!lease.Open(m_Path))
    { status = "Another writer owns this hierarchy or its lock is unavailable; retry after it finishes. Draft preserved."; return false; }
    bool diskExists = false;
    std::string diskBytes;
    if (!ReadSource(m_Path, diskExists, diskBytes, status)) return false;
    if (m_BaselineExists && !diskExists)
    { status = "Hierarchy was deleted on disk; draft preserved. Reload explicitly before recreating it."; return false; }
    STATE disk, merged;
    if (diskExists && !Parse(diskBytes, m_AreaId, disk, status)) return false;
    if (!Merge(m_Baseline, m_Draft, disk, merged, status)) return false;
    const auto bytes = Serialize(merged, m_AreaId);
    if (bytes.size() > MaximumBytes)
    { status = "Hierarchy save exceeds 16 MiB; draft preserved."; return false; }
    STATE verified;
    if (!Parse(bytes, m_AreaId, verified, status) || verified != merged)
    { status = "Hierarchy serialization verification failed; draft preserved."; return false; }
    if (diskBytes != bytes || !diskExists)
    {
        const auto suffix = UniqueSuffix();
        const std::wstring wideSuffix(suffix.begin(), suffix.end());
        TemporaryFile staged{m_Path.wstring() + wideSuffix + L".stage"};
        if (!staged.Write(bytes))
        { status = "Hierarchy staging write failed; draft and disk preserved."; return false; }
        bool latestExists = false;
        std::string latestBytes;
        if (!ReadSource(m_Path, latestExists, latestBytes, status)) return false;
        if (latestExists != diskExists || latestBytes != diskBytes)
        { status = "Hierarchy changed immediately before replacement; draft preserved. Retry Save to merge the latest source."; return false; }
        TemporaryFile backup{m_Path.wstring() + wideSuffix + L".rollback"};
        bool replaced = false;
        if (diskExists)
        {
            // ReplaceFile creates a rollback copy as part of the atomic commit.
            replaced = ReplaceFileW(m_Path.c_str(), staged.path.c_str(), backup.path.c_str(),
                REPLACEFILE_WRITE_THROUGH, nullptr, nullptr) != FALSE;
            // A failed replacement may retain a recovery file; never delete it.
            if (replaced) backup.owned = true;
        }
        else
        {
            // No REPLACE_EXISTING: a concurrent creator must not be overwritten.
            replaced = MoveFileExW(staged.path.c_str(), m_Path.c_str(), MOVEFILE_WRITE_THROUGH) != FALSE;
        }
        if (!replaced)
        {
            const auto replaceError = GetLastError();
            // Windows may retain the old file under the backup name on a
            // partially failed ReplaceFile. Restore only that exact preimage,
            // only when the destination is still absent, without replacement.
            bool currentExists = false, backupExists = false;
            std::string currentBytes, backupBytes, recoveryStatus;
            const bool recoveryReady = diskExists &&
                ReadSource(m_Path, currentExists, currentBytes, recoveryStatus) && !currentExists &&
                ReadSource(backup.path, backupExists, backupBytes, recoveryStatus) && backupExists && backupBytes == diskBytes;
            const bool restored = recoveryReady &&
                MoveFileExW(backup.path.c_str(), m_Path.c_str(), MOVEFILE_WRITE_THROUGH) != FALSE;
            status = "Hierarchy atomic replacement failed (Windows " + std::to_string(replaceError) + "); draft preserved.";
            if (restored) status += " The previous source was restored.";
            else if (backupExists) status += " Recovery copy retained: " + backup.path.generic_string();
            return false;
        }
        staged.owned = false;
    }
    const bool combined = diskExists != m_BaselineExists || diskBytes != m_BaselineBytes;
    m_Draft = merged;
    m_Baseline = std::move(merged);
    m_BaselineBytes = bytes;
    m_BaselineExists = true;
    m_Dirty = false;
    if (combined) { m_Undo.clear(); m_Redo.clear(); }
    status = combined ? "World hierarchy saved with unrelated disk edits preserved." : "World hierarchy saved. Runtime data and transforms are unchanged.";
    return true;
}
}
