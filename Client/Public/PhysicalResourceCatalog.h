#pragma once

// Resources 경로와 재귀 탐색 iterator를 소유한다.
#include <filesystem>
// 같은 상대 asset ID가 중복 수집되는 것을 막는다.
#include <set>
// 상대 asset ID·파일명·상태 문구를 보관한다.
#include <string>
// 요청 폴더와 탐색 후보 목록을 값으로 보관한다.
#include <vector>

namespace Client
{
enum class PHYSICAL_RESOURCE_KIND { MODEL, TEXTURE };

// WModel/DDS 파일의 경로·표시명·종류만 담는다. GPU 자원을 생성하거나 보유하지 않는다.
struct PHYSICAL_RESOURCE_ASSET final
{
    std::string assetId;
    std::string fileName;
    PHYSICAL_RESOURCE_KIND kind = PHYSICAL_RESOURCE_KIND::MODEL;
};

// Advance 호출마다 일부 directory 항목을 읽어 UI에서 나누어 진행할 수 있는 탐색 상태다.
// 완료 후 하나 이상 폴더가 있으면 결과를 교체한다. 일부 누락 폴더는 상태 문구로 알린다.
class CPhysicalResourceScan final
{
public:
    // 허용된 상위 폴더인지 먼저 검사한다. 거부하면 기존 탐색 상태를 보존하며 성공하면 새 탐색으로 초기화한다.
    bool Begin(const std::vector<std::string>& relativeRoots, std::string& outStatus);
    // 최대 128회 반복과 약 2ms 검사 기준으로 진행한다. 파일 I/O 한 번의 소요 시간을 제한하지는 않는다.
    bool Advance();
    // 미완료 또는 사용 가능 폴더 0개면 출력 목록을 보존한다. 성공 시 내부 후보를 이동하므로 한 번 소비하는 계약이다.
    bool Commit(std::vector<PHYSICAL_RESOURCE_ASSET>& outAssets, std::string& outStatus);

private:
    // 탐색 객체가 iterator와 후보를 소유한다. 소멸하면 정리되며 별도 worker thread는 없다.
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

// 명시적 새로고침에서 사용하는 동기 wrapper다. 파일명만 같아도 경로가 다르면 별도 ID로 유지한다.
// 반환 ID는 Resources 기준의 실제 하위 경로다.
bool Scan_PhysicalResources(const std::vector<std::string>& relativeRoots,
    std::vector<PHYSICAL_RESOURCE_ASSET>& outAssets, std::string& outStatus);
}
