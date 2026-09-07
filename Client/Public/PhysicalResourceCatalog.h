#pragma once

#include <filesystem>
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

// Explicit refresh only. IDs retain every physical folder and never merge
// different assets merely because their file names happen to be equal.
bool Scan_PhysicalResources(const std::vector<std::string>& relativeRoots,
    std::vector<PHYSICAL_RESOURCE_ASSET>& outAssets, std::string& outStatus);
}
