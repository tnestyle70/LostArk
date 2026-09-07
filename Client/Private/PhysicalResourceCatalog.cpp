#include "PhysicalResourceCatalog.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <chrono>

bool Client::CPhysicalResourceScan::Begin(const std::vector<std::string>& relativeRoots,
    std::string& outStatus)
{
    for (const auto& folder : relativeRoots)
        if (folder != "Effect" && folder != "Map" && folder != "Deploy" && folder != "Character")
        {
            outStatus = "Unsupported physical resource folder: " + folder;
            return false;
        }
    m_Root = CRuntimeAssetRoot::Get();
    m_RelativeRoots = relativeRoots;
    m_Iterator = {};
    m_Assets.clear(); m_Ids.clear(); m_Unavailable.clear();
    m_NextRoot = 0; m_AvailableFolders = 0; m_Complete = false;
    outStatus = "Scanning physical files; saved objects and Save/Publish remain available.";
    return true;
}

bool Client::CPhysicalResourceScan::Advance()
{
    if (m_Complete) return true;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(2);
    const std::filesystem::recursive_directory_iterator end;
    std::size_t visited = 0;
    do
    {
        if (m_Iterator == end)
        {
            if (m_NextRoot == m_RelativeRoots.size())
            {
                std::sort(m_Assets.begin(), m_Assets.end(), [](const auto& left, const auto& right) {
                    return left.assetId < right.assetId;
                });
                m_Complete = true;
                return true;
            }
            const auto& folder = m_RelativeRoots[m_NextRoot++];
            const auto path = m_Root / folder;
            std::error_code error;
            if (!std::filesystem::is_directory(path, error))
                m_Unavailable += (m_Unavailable.empty() ? "" : ", ") + folder;
            else
            {
                ++m_AvailableFolders;
                m_Iterator = std::filesystem::recursive_directory_iterator(path,
                    std::filesystem::directory_options::skip_permission_denied, error);
            }
        }
        else
        {
            const auto entry = *m_Iterator;
            std::error_code error;
            // Junctions/symlinks are not resource identities; do not follow them.
            const bool symlink = entry.is_symlink(error);
            if (symlink) m_Iterator.disable_recursion_pending();
            if (!error && !symlink && entry.is_regular_file(error) && !error)
            {
                auto extension = entry.path().extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (extension == ".wmodel" || extension == ".dds")
                {
                    const auto id = entry.path().lexically_relative(m_Root).generic_string();
                    if (m_Ids.insert(id).second)
                        m_Assets.push_back({id, entry.path().filename().string(),
                            extension == ".wmodel" ? PHYSICAL_RESOURCE_KIND::MODEL : PHYSICAL_RESOURCE_KIND::TEXTURE});
                }
            }
            m_Iterator.increment(error);
        }
        ++visited;
    } while (visited < 128u && std::chrono::steady_clock::now() < deadline);
    return false;
}

bool Client::CPhysicalResourceScan::Commit(std::vector<PHYSICAL_RESOURCE_ASSET>& outAssets,
    std::string& outStatus)
{
    if (!m_Complete) { outStatus = "Physical resource scan is still running; previous files preserved."; return false; }
    if (m_AvailableFolders == 0)
    {
        outStatus = "Resource folders unavailable: " + m_Unavailable;
        return false;
    }
    outAssets = std::move(m_Assets);
    outStatus = std::to_string(outAssets.size()) + " physical WModel/DDS files";
    if (!m_Unavailable.empty()) outStatus += "; folders unavailable: " + m_Unavailable;
    return true;
}

bool Client::Scan_PhysicalResources(const std::vector<std::string>& relativeRoots,
    std::vector<PHYSICAL_RESOURCE_ASSET>& outAssets, std::string& outStatus)
{
    CPhysicalResourceScan scan;
    if (!scan.Begin(relativeRoots, outStatus)) return false;
    while (!scan.Advance()) {}
    return scan.Commit(outAssets, outStatus);
}
