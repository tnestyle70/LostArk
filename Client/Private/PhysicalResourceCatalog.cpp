#include "PhysicalResourceCatalog.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <set>

bool Client::Scan_PhysicalResources(const std::vector<std::string>& relativeRoots,
    std::vector<PHYSICAL_RESOURCE_ASSET>& outAssets, std::string& outStatus)
{
    const auto root = CRuntimeAssetRoot::Get();
    std::vector<PHYSICAL_RESOURCE_ASSET> staged;
    std::set<std::string> ids;
    std::string unavailable;
    size_t availableFolders = 0;
    for (const auto& folder : relativeRoots)
    {
        if (folder != "Effect" && folder != "Map" && folder != "Deploy" && folder != "Character")
        {
            outStatus = "Unsupported physical resource folder: " + folder;
            return false;
        }
        const auto path = root / folder;
        std::error_code error;
        if (!std::filesystem::is_directory(path, error))
        {
            unavailable += (unavailable.empty() ? "" : ", ") + folder;
            continue;
        }
        ++availableFolders;
        std::filesystem::recursive_directory_iterator iterator(path,
            std::filesystem::directory_options::skip_permission_denied, error), end;
        while (iterator != end)
        {
            const auto entry = *iterator;
            std::error_code entryError;
            // Junctions/symlinks are not resource identities; do not follow them.
            const bool symlink = entry.is_symlink(entryError);
            if (symlink) iterator.disable_recursion_pending();
            if (!entryError && !symlink && entry.is_regular_file(entryError) && !entryError)
            {
                auto extension = entry.path().extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (extension == ".wmodel" || extension == ".dds")
                {
                    const auto id = entry.path().lexically_relative(root).generic_string();
                    if (ids.insert(id).second)
                        staged.push_back({id, entry.path().filename().string(),
                            extension == ".wmodel" ? PHYSICAL_RESOURCE_KIND::MODEL : PHYSICAL_RESOURCE_KIND::TEXTURE});
                }
            }
            iterator.increment(error);
            if (error) error.clear();
        }
    }
    if (availableFolders == 0)
    {
        outStatus = "Resource folders unavailable: " + unavailable;
        return false;
    }
    std::sort(staged.begin(), staged.end(), [](const auto& left, const auto& right) {
        return left.assetId < right.assetId;
    });
    outAssets = std::move(staged);
    outStatus = std::to_string(outAssets.size()) + " physical WModel/DDS files";
    if (!unavailable.empty()) outStatus += "; folders unavailable: " + unavailable;
    return true;
}
