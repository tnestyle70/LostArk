#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <Windows.h>
#include "WorldLevelHierarchy.h"

namespace
{
using Hierarchy = Client::CWorldLevelHierarchy;
int checks = 0, failures = 0;
void Check(bool result, const char* expression, int line)
{
    ++checks;
    if (!result) { ++failures; std::cerr << "FAIL line " << line << ": " << expression << '\n'; }
}
#define CHECK(...) Check(bool(__VA_ARGS__), #__VA_ARGS__, __LINE__)
std::string Read(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
void Write(const std::filesystem::path& path, const std::string& bytes)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << bytes;
}
const Hierarchy::FOLDER* Find(const Hierarchy& hierarchy, const std::string& id)
{
    for (const auto& folder : hierarchy.Get_Folders()) if (folder.id == id) return &folder;
    return nullptr;
}
std::string Document(const std::string& folders, const std::string& assignments = "[]", const char* area = "TEST_AREA")
{
    return "{\"schema\":\"lostark.world-level-hierarchy\",\"version\":1,\"areaId\":\"" + std::string(area) +
        "\",\"folders\":" + folders + ",\"assignments\":" + assignments + "}";
}
}

int wmain(int argc, wchar_t** argv)
{
    if (argc != 2) return 2;
    const std::filesystem::path root = std::filesystem::path(argv[1]) / (L"hierarchy-contracts-" + std::to_wstring(GetCurrentProcessId()));
    std::filesystem::create_directories(root);
    const auto path = root / "main.json";
    std::string status, a, b, c;
    Hierarchy h;
    CHECK(!h.Save(status));
    CHECK(!h.Create_Folder("Not loaded", {}, a, status));
    CHECK(h.Load(path, "TEST_AREA", status));
    CHECK(!std::filesystem::exists(path));
    CHECK(!h.Is_Dirty());
    CHECK(h.Create_Folder("Parent A", {}, a, status));
    CHECK(h.Create_Folder("Parent B", a, b, status));
    CHECK(h.Create_Folder("Parent C", b, c, status));
    CHECK(h.Assign({"map:1", "deploy:2", "world:motion.a", "action:pattern.a:box.b"}, c, status));
    CHECK(h.Get_Parent("map:1") == c);
    CHECK(!h.Move_Folder(a, c, status));
    CHECK(Find(h, a)->parent.empty());
    CHECK(!h.Move_Folder(c, "missing", status));
    CHECK(!h.Assign({"map:1", "bad\nkey"}, b, status));
    CHECK(h.Get_Parent("map:1") == c);
    CHECK(!h.Rename_Folder(a, "", status));
    CHECK(!h.Rename_Folder(a, "   ", status));
    CHECK(!h.Rename_Folder(a, std::string("\xC0\x80", 2), status));
    CHECK(!h.Rename_Folder(a, std::string(129, 'A'), status));
    CHECK(h.Rename_Folder(a, "\xED\x95\x9C\xEA\xB8\x80 \"quoted\" \\ name", status));
    CHECK(h.Save(status));
    CHECK(!h.Is_Dirty());
    CHECK(h.Can_Undo() && h.Undo(status) && h.Is_Dirty());
    CHECK(h.Can_Redo() && h.Redo(status) && !h.Is_Dirty());
    CHECK(h.Load(path, "TEST_AREA", status));
    CHECK(!h.Can_Undo() && !h.Can_Redo());
    CHECK(!std::filesystem::exists(path.wstring() + L".writer.lock"));
    const auto saved = Read(path);
    CHECK(saved.size() > 3 && !(static_cast<unsigned char>(saved[0]) == 0xEF && static_cast<unsigned char>(saved[1]) == 0xBB));
    Hierarchy reopened;
    CHECK(reopened.Load(path, "TEST_AREA", status));
    CHECK(reopened.Get_Folders() == h.Get_Folders());
    CHECK(reopened.Get_Parent("action:pattern.a:box.b") == c);
    // Dirty queries stay constant-time, including no-op, copied and reverted edits.
    CHECK(h.Rename_Folder(c, "Parent C", status));
    CHECK(!h.Is_Dirty() && !h.Undo(status));
    CHECK(h.Rename_Folder(c, "Temporary C", status));
    CHECK(h.Is_Dirty());
    auto dirtyCopy = h;
    CHECK(dirtyCopy.Is_Dirty());
    CHECK(h.Rename_Folder(c, "Parent C", status));
    CHECK(!h.Is_Dirty());
    CHECK(h.Undo(status) && h.Is_Dirty());
    CHECK(h.Undo(status) && !h.Is_Dirty());
    CHECK(h.Delete_Folder(b, status));
    CHECK(Find(h, c)->parent == a);
    CHECK(h.Get_Parent("map:1") == c);
    CHECK(h.Undo(status));
    CHECK(!h.Is_Dirty());
    CHECK(h.Delete_Folder(c, status));
    CHECK(h.Get_Parent("map:1") == b);
    CHECK(h.Undo(status));
    CHECK(h.Assign({"map:1"}, {}, status));
    CHECK(h.Get_Parent("map:1").empty());
    CHECK(h.Undo(status));
    CHECK(h.Get_Parent("map:1") == c);
    CHECK(h.Assign({"out-of-scope:never-loaded"}, a, status));
    CHECK(h.Save(status));
    CHECK(reopened.Load(path, "TEST_AREA", status));
    CHECK(reopened.Get_Parent("out-of-scope:never-loaded") == a);

    // Staged mixed folder/object edits commit as exactly one Undo operation.
    auto batch = h;
    std::string batchParent;
    CHECK(batch.Create_Folder("Batch Parent", {}, batchParent, status));
    CHECK(batch.Move_Folder(b, batchParent, status));
    CHECK(batch.Assign({"map:1", "deploy:2"}, batchParent, status));
    CHECK(h.Commit_Edit(batch, status));
    CHECK(h.Get_Parent("map:1") == batchParent && Find(h, b)->parent == batchParent);
    CHECK(h.Undo(status));
    CHECK(!h.Is_Dirty() && Find(h, b)->parent == a && h.Get_Parent("map:1") == c);
    CHECK(!Find(h, batchParent));
    Hierarchy otherSource;
    CHECK(otherSource.Load(root / "other.json", "TEST_AREA", status));
    CHECK(!h.Commit_Edit(otherSource, status));

    // Selection snapshots and folder deletions are restored without changing disk.
    Hierarchy history;
    const auto historyPath = root / "redo.json";
    std::string parentId, childId;
    CHECK(history.Load(historyPath, "TEST_AREA", status));
    CHECK(history.Create_Folder("Parent", {}, parentId, status));
    CHECK(history.Create_Folder("Child", parentId, childId, status));
    CHECK(history.Assign({"map:44"}, childId, status));
    history.Set_Selection({{"folder:" + childId, "row:map:44"}, "folder:" + childId});
    CHECK(history.Delete_Folder(childId, status));
    history.Set_Selection({{"folder:" + parentId}, "folder:" + parentId});
    CHECK(history.Get_Parent("map:44") == parentId);
    CHECK(history.Undo(status));
    CHECK(history.Get_Parent("map:44") == childId && Find(history, childId));
    CHECK(history.Get_Selection().nodes.size() == 2u);
    CHECK(history.Get_Selection().primary == "folder:" + childId);
    CHECK(history.Redo(status));
    CHECK(history.Get_Parent("map:44") == parentId && !Find(history, childId));
    CHECK(history.Get_Selection().primary == "folder:" + parentId);
    CHECK(history.Save(status) && !history.Is_Dirty());
    const auto historySaved = Read(historyPath);
    CHECK(history.Undo(status) && history.Is_Dirty());
    CHECK(Read(historyPath) == historySaved);
    CHECK(history.Redo(status) && !history.Is_Dirty());
    CHECK(history.Undo(status));
    CHECK(!history.Move_Folder(parentId, childId, status));
    CHECK(history.Can_Redo());
    CHECK(history.Rename_Folder(childId, "Branch", status));
    CHECK(!history.Can_Redo() && !history.Redo(status));
    CHECK(history.Save(status));
    CHECK(history.Undo(status) && Find(history, childId)->name == "Child");
    CHECK(history.Redo(status) && Find(history, childId)->name == "Branch");
    CHECK(history.Load(historyPath, "TEST_AREA", status));
    CHECK(!history.Can_Undo() && !history.Can_Redo());

    // Failed Load preserves draft, selected source path and its baseline.
    const auto invalid = root / "invalid.json";
    Write(invalid, "{not json");
    CHECK(h.Rename_Folder(c, "Unsaved C", status));
    CHECK(!h.Load(invalid, "TEST_AREA", status));
    CHECK(h.Is_Dirty() && Find(h, c)->name == "Unsaved C");
    CHECK(h.Save(status));
    CHECK(Read(invalid) == "{not json");
    CHECK(Read(path).find("Unsaved C") != std::string::npos);

    const std::vector<std::string> badDocuments = {
        Document("[{\"id\":\"a\",\"name\":\"A\",\"parent\":\"a\"}]"),
        Document("[{\"id\":\"a\",\"name\":\"A\",\"parent\":\"missing\"}]"),
        Document("[{\"id\":\"a\",\"name\":\"A\",\"parent\":\"\"},{\"id\":\"a\",\"name\":\"B\",\"parent\":\"\"}]"),
        Document("[{\"id\":\"bad/id\",\"name\":\"A\",\"parent\":\"\"}]"),
        Document("[{\"id\":\"a\",\"name\":\"\",\"parent\":\"\"}]"),
        Document("[{\"id\":\"a\",\"name\":\"A\",\"parent\":\"\",\"unknown\":true}]"),
        Document("[]", "[{\"key\":\"map:1\",\"parent\":\"missing\"}]"),
        Document("[]", "[{\"key\":\"map:1\",\"parent\":\"\"},{\"key\":\"map:1\",\"parent\":\"\"}]"),
        Document("[]", "[]", "OTHER_AREA"),
        "{\"schema\":\"lostark.world-level-hierarchy\",\"version\":2,\"areaId\":\"TEST_AREA\",\"folders\":[],\"assignments\":[]}",
        "{\"schema\":\"lostark.world-level-hierarchy\",\"version\":1,\"areaId\":\"TEST_AREA\",\"folders\":[],\"assignments\":[],\"unknown\":0}"
    };
    for (const auto& text : badDocuments)
    {
        Write(invalid, text);
        CHECK(!h.Load(invalid, "TEST_AREA", status));
        CHECK(Find(h, c) && Find(h, c)->name == "Unsaved C");
    }

    // The boundary permits 64 ancestors and rejects the 65th without mutation.
    Hierarchy deep;
    CHECK(deep.Load(root / "deep.json", "TEST_AREA", status));
    std::string parent;
    for (int i = 0; i <= 64; ++i)
    {
        std::string id;
        CHECK(deep.Create_Folder("Depth " + std::to_string(i), parent, id, status));
        parent = id;
    }
    std::string unchanged = "unchanged";
    CHECK(!deep.Create_Folder("Too deep", parent, unchanged, status));
    CHECK(unchanged == "unchanged" && deep.Get_Folders().size() == 65u);

    // Merge distinct fields of the same folder and preserve unseen assignments.
    Hierarchy left, right;
    CHECK(left.Load(path, "TEST_AREA", status));
    CHECK(right.Load(path, "TEST_AREA", status));
    CHECK(left.Rename_Folder(c, "Concurrent name", status));
    CHECK(right.Move_Folder(c, a, status));
    CHECK(right.Assign({"new.remote.row"}, b, status));
    CHECK(right.Save(status));
    CHECK(left.Save(status));
    CHECK(Find(left, c)->name == "Concurrent name" && Find(left, c)->parent == a);
    CHECK(left.Get_Parent("new.remote.row") == b);
    CHECK(left.Get_Parent("out-of-scope:never-loaded") == a);

    // Conflicting fields/assignments must keep both disk and unsaved local state.
    CHECK(left.Load(path, "TEST_AREA", status));
    CHECK(right.Load(path, "TEST_AREA", status));
    CHECK(left.Rename_Folder(c, "Local conflict", status));
    CHECK(right.Rename_Folder(c, "Remote conflict", status));
    CHECK(right.Save(status));
    const auto nameConflictDisk = Read(path);
    CHECK(!left.Save(status));
    CHECK(left.Is_Dirty() && Find(left, c)->name == "Local conflict");
    CHECK(Read(path) == nameConflictDisk);
    CHECK(left.Load(path, "TEST_AREA", status));
    CHECK(right.Load(path, "TEST_AREA", status));
    CHECK(left.Assign({"map:1"}, a, status));
    CHECK(right.Assign({"map:1"}, b, status));
    CHECK(right.Save(status));
    const auto assignmentConflictDisk = Read(path);
    CHECK(!left.Save(status));
    CHECK(left.Get_Parent("map:1") == a && left.Is_Dirty());
    CHECK(Read(path) == assignmentConflictDisk);

    // Concurrent folder deletion cannot strand a local assignment.
    CHECK(left.Load(path, "TEST_AREA", status));
    CHECK(right.Load(path, "TEST_AREA", status));
    CHECK(left.Assign({"new.local.row"}, c, status));
    CHECK(right.Delete_Folder(c, status));
    CHECK(right.Save(status));
    const auto deletionDisk = Read(path);
    CHECK(!left.Save(status));
    CHECK(Read(path) == deletionDisk && left.Get_Parent("new.local.row") == c);

    // Two editors beginning with no file can save unrelated new folders.
    const auto newPath = root / "concurrent-new.json";
    CHECK(left.Load(newPath, "TEST_AREA", status));
    CHECK(right.Load(newPath, "TEST_AREA", status));
    std::string localId, remoteId;
    CHECK(left.Create_Folder("Local", {}, localId, status));
    CHECK(right.Create_Folder("Remote", {}, remoteId, status));
    CHECK(localId != remoteId);
    CHECK(right.Save(status));
    CHECK(left.Save(status));
    CHECK(Find(left, localId) && Find(left, remoteId));

    // Writer ownership and destination file locks fail without discarding drafts.
    CHECK(left.Rename_Folder(localId, "Locked write", status));
    const auto beforeLocks = Read(newPath);
    HANDLE lease = CreateFileW((newPath.wstring() + L".writer.lock").c_str(), GENERIC_READ | GENERIC_WRITE,
        0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
    CHECK(lease != INVALID_HANDLE_VALUE);
    CHECK(!left.Save(status));
    CHECK(left.Is_Dirty() && Read(newPath) == beforeLocks);
    CloseHandle(lease);
    HANDLE held = CreateFileW(newPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    CHECK(held != INVALID_HANDLE_VALUE);
    CHECK(!left.Save(status));
    CHECK(left.Is_Dirty() && Read(newPath) == beforeLocks);
    CloseHandle(held);
    CHECK(left.Save(status));
    CHECK(!left.Is_Dirty());

    // An external delete is explicit conflict, not implicit recreation.
    CHECK(left.Rename_Folder(localId, "Before disk delete", status));
    CHECK(std::filesystem::remove(newPath));
    CHECK(!left.Save(status));
    CHECK(left.Is_Dirty() && !std::filesystem::exists(newPath));
    std::cout << "WorldLevelHierarchy contracts: checks=" << checks << " failures=" << failures << '\n';
    std::cout << "Fixture root: " << root.string() << '\n';
    return failures ? 1 : 0;
}
