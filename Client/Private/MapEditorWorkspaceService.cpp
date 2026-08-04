#include "MapEditorWorkspaceService.h"

#include <atomic>

namespace
{
	std::atomic_bool g_isRequested = false;
	std::atomic_bool g_isActive = false;
}

void Client::CMapEditorWorkspaceService::Request()
{
	g_isRequested.store(true, std::memory_order_release);
}

void Client::CMapEditorWorkspaceService::Cancel()
{
	g_isActive.store(false, std::memory_order_release);
	g_isRequested.store(false, std::memory_order_release);
}

bool_t Client::CMapEditorWorkspaceService::Is_Requested()
{
	return g_isRequested.load(std::memory_order_acquire);
}

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
