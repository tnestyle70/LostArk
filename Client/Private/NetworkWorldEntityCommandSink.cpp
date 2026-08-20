#include "NetworkWorldEntityCommandSink.h"

#include "NetworkManager.h"

bool Client::CNetworkWorldEntityCommandSink::Request_SpawnWorldEntity(
	const std::string_view placementId)
{
	return CNetworkManager::Get().Send_SpawnWorldEntity(placementId);
}

bool Client::CNetworkWorldEntityCommandSink::Request_DespawnAllWorldEntities(
	const std::uint32_t requestSequence)
{
	return CNetworkManager::Get().Send_DespawnAllWorldEntities(requestSequence);
}
