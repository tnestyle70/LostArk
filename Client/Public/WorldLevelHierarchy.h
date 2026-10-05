#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace Client
{
// Per-Area editor organization only. Parents never own or inherit transforms;
// every row keeps the existing map, Deploy or sequence authoring owner.
class CWorldLevelHierarchy final
{
public:
    struct FOLDER final
    {
        std::string id, name, parent;
        bool operator==(const FOLDER&) const = default;
    };

    bool Load(const std::filesystem::path& path, const std::string& areaId, std::string& status);
    bool Save(std::string& status);
    bool Is_Dirty() const;
    const std::vector<FOLDER>& Get_Folders() const { return m_Draft.folders; }
    std::string Get_Parent(const std::string& rowKey) const;
    bool Create_Folder(const std::string& name, const std::string& parent,
        std::string& newId, std::string& status);
    bool Rename_Folder(const std::string& id, const std::string& name, std::string& status);
    bool Move_Folder(const std::string& id, const std::string& parent, std::string& status);
    // Promote children and assigned rows to the removed folder's parent.
    bool Delete_Folder(const std::string& id, std::string& status);
    bool Assign(const std::vector<std::string>& keys, const std::string& parent, std::string& status);
    // Commit a fully staged multi-selection operation as one Undo step.
    bool Commit_Edit(const CWorldLevelHierarchy& staged, std::string& status);
    // Save retains history against the new disk baseline. Reload and external
    // merge reset it, so history cannot overwrite another writer's changes.
    bool Undo(std::string& status);
    bool Redo(std::string& status);
    bool Can_Undo() const { return !m_Undo.empty(); }
    bool Can_Redo() const { return !m_Redo.empty(); }
    struct SELECTION final { std::vector<std::string> nodes; std::string primary; };
    void Set_Selection(SELECTION selection) { m_Selection = std::move(selection); }
    const SELECTION& Get_Selection() const { return m_Selection; }

private:
    struct STATE final
    {
        std::vector<FOLDER> folders;
        std::map<std::string, std::string> assignments;
        bool operator==(const STATE&) const = default;
    };

    static bool Validate(const STATE& state, std::string& status);
    static bool Parse(const std::string& bytes, const std::string& areaId, STATE& out, std::string& status);
    static std::string Serialize(const STATE& state, const std::string& areaId);
    static bool Merge(const STATE& baseline, const STATE& local, const STATE& disk,
        STATE& out, std::string& status);
    bool Commit(STATE candidate, const char* message, std::string& status);

    std::filesystem::path m_Path;
    std::string m_AreaId, m_BaselineBytes;
    STATE m_Draft, m_Baseline;
    struct HISTORY final { STATE state; SELECTION selection; };
    std::vector<HISTORY> m_Undo, m_Redo;
    SELECTION m_Selection;
    bool m_Loaded = false, m_BaselineExists = false, m_Dirty = false;
};
}
