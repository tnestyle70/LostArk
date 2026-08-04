#include "NetworkWorldEntityCommandSink.h"

#include "NetworkManager.h"

bool Client::CNetworkWorldEntityCommandSink::Request_SpawnWorldEntity(
	const std::string_view placementId)
{
	return CNetworkManager::Get().Send_SpawnWorldEntity(placementId);
}
