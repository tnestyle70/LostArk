#include "MapEditorWorkspaceService.h"

// Loader와 프레임 측의 공유 플래그 읽기/쓰기에 acquire/release를 사용한다.
#include <atomic>

namespace
{
	// 프로세스 수명의 두 독립 플래그다. Level·MapTool 인스턴스를 저장하지 않는다.
	std::atomic_bool g_isRequested = false;
	std::atomic_bool g_isActive = false;
}

// 호출자가 요청만 게시한다. 여기서 Level 전환이나 자원 생성은 하지 않는다.
void Client::CMapEditorWorkspaceService::Request()
{
	g_isRequested.store(true, std::memory_order_release);
}

// 활성부터 해제한 뒤 요청을 해제한다.
void Client::CMapEditorWorkspaceService::Cancel()
{
	g_isActive.store(false, std::memory_order_release);
	g_isRequested.store(false, std::memory_order_release);
}

bool_t Client::CMapEditorWorkspaceService::Is_Requested()
{
	return g_isRequested.load(std::memory_order_acquire);
}

// Development가 초기화 성공을 게시한다. 비활성화만 하면 재진입 요청은 보존된다.
void Client::CMapEditorWorkspaceService::Set_Active(const bool_t isActive)
{
	g_isActive.store(isActive, std::memory_order_release);
	if (isActive)
		g_isRequested.store(true, std::memory_order_release);
}

bool_t Client::CMapEditorWorkspaceService::Is_Active()
{
	return g_isActive.load(std::memory_order_acquire);
}
