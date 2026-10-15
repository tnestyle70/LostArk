// Level의 world 명령을 같은 의미의 NetworkManager 송신 함수로 연결한다.
#include "NetworkWorldEntityCommandSink.h"

// 연결 확인·요청 직렬화·송신과 spawn 추적 token 관리를 맡긴다.
#include "NetworkManager.h"

// spawn을 전송하고 요청 추적 token을 돌려준다. 성공해도 Server 생성 승인은 이후 응답으로 확인한다.
bool Client::CNetworkWorldEntityCommandSink::Request_SpawnWorldEntity(
	const std::string_view placementId, std::uint64_t* outRequestToken)
{
	return CNetworkManager::Get().Send_SpawnWorldEntity(placementId, outRequestToken);
}

// 해제 요청의 송신 성공 여부만 전달한다. 로컬 Registry나 Layer는 건드리지 않는다.
bool Client::CNetworkWorldEntityCommandSink::Request_DespawnAllWorldEntities(
	const std::uint32_t requestSequence)
{
	return CNetworkManager::Get().Send_DespawnAllWorldEntities(requestSequence);
}

// Debug 진입 명령을 보내고 Server 승인은 기존 Level 전환 흐름에서 기다린다.
bool Client::CNetworkWorldEntityCommandSink::Request_EnterKoukuSaydonArena(
	const std::uint32_t requestSequence)
{
	return CNetworkManager::Get().Send_DebugEnterKoukuSaydonArena(requestSequence);
}

// Server가 해석할 stable placement ID를 보낸다. string_view를 멤버에 남기지 않는다.
bool Client::CNetworkWorldEntityCommandSink::Request_StageTeleport(
	const std::uint32_t requestSequence,
	const std::string_view placementId)
{
	return CNetworkManager::Get().Send_DebugTeleportToPlacement(
		requestSequence, placementId);
}
