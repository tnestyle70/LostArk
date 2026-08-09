#include "NetworkPlayerCommandSink.h"

#include "NetworkManager.h"

std::atomic_uint32_t
	Client::CNetworkPlayerCommandSink::s_iLiveInstanceCount = 0u;

Client::CNetworkPlayerCommandSink::CNetworkPlayerCommandSink()
{
	++s_iLiveInstanceCount;
}

Client::CNetworkPlayerCommandSink::~CNetworkPlayerCommandSink()
{
	--s_iLiveInstanceCount;
}

std::uint32_t
Client::CNetworkPlayerCommandSink::Get_LiveInstanceCount()
{
	return s_iLiveInstanceCount.load();
}

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

bool Client::CNetworkPlayerCommandSink::Request_ReleaseSkill(
	std::uint32_t clientSequence,
	LostArk::Shared::SKILL_ID skillId)
{
	return CNetworkManager::Get().Send_ReleaseSkill(clientSequence, skillId);
}

bool Client::CNetworkPlayerCommandSink::Request_RevivePlayer(
	const std::uint32_t clientSequence)
{
	return CNetworkManager::Get().Send_RevivePlayer(clientSequence);
}

bool Client::CNetworkPlayerCommandSink::Request_ChangeCharacterClass(
	const std::uint32_t clientSequence,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	return CNetworkManager::Get().Send_ChangeCharacterClass(
		clientSequence, characterClass);
}
