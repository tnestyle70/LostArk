#include "NetworkPlayerCommandSink.h"

#include "NetworkManager.h"

bool Client::CNetworkPlayerCommandSink::Request_MoveGoal(
	std::uint32_t clientSequence,
	float goalX,
	float goalZ)
{
	return CNetworkManager::Get().Send_MoveGoal(
		clientSequence,
		goalX,
		goalZ);
}

bool Client::CNetworkPlayerCommandSink::Request_UseSkill(
	std::uint32_t clientSequence,
	LostArk::Shared::SKILL_ID skillId,
	float aimX,
	float aimZ)
{
	return CNetworkManager::Get().Send_UseSkill(
		clientSequence,
		skillId,
		aimX,
		aimZ);
}
