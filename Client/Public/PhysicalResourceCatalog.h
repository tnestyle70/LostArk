#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <vector>

namespace Client
{
enum class PHYSICAL_RESOURCE_KIND { MODEL, TEXTURE };

struct PHYSICAL_RESOURCE_ASSET final
{
    std::string assetId;
    std::string fileName;
    PHYSICAL_RESOURCE_KIND kind = PHYSICAL_RESOURCE_KIND::MODEL;
};

// Cooperatively scans a bounded batch per Advance, keeping UI controls usable.
// Commit replaces the previous catalog only after a complete successful scan.
class CPhysicalResourceScan final
{
public:
    bool Begin(const std::vector<std::string>& relativeRoots, std::string& outStatus);
    bool Advance();
    bool Commit(std::vector<PHYSICAL_RESOURCE_ASSET>& outAssets, std::string& outStatus);

private:
    std::filesystem::path m_Root;
    std::vector<std::string> m_RelativeRoots;
    std::filesystem::recursive_directory_iterator m_Iterator;
    std::vector<PHYSICAL_RESOURCE_ASSET> m_Assets;
    std::set<std::string> m_Ids;
    std::string m_Unavailable;
    std::size_t m_NextRoot = 0;
    std::size_t m_AvailableFolders = 0;
    bool m_Complete = true;
};

// Explicit refresh only. IDs retain every physical folder and never merge
// different assets merely because their file names happen to be equal.
bool Scan_PhysicalResources(const std::vector<std::string>& relativeRoots,
    std::vector<PHYSICAL_RESOURCE_ASSET>& outAssets, std::string& outStatus);
}
