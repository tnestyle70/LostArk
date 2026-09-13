#pragma once

#include "ServerWorldEntity.h"
#include "ServerPlayer.h"
#include "ValtanBrain.h"
#include <map>
#include <vector>

namespace ServerGameplayContractDetail
{


	/* Run_ServerGameplayContractTests is intentionally one broad executable
	contract and is already close to the Windows default stack budget. Keep the
	hot-reload occurrence simulation on the heap so extending that contract does
	not make test startup depend on compiler stack-slot reuse. */
	struct PINNED_MECHANIC_GENERATION_FIXTURE final
	{
		LostArk::Server::SERVER_WORLD_ENTITY Boss;
		LostArk::Server::SERVER_WORLD_ENTITY ReconcileBoss;
		LostArk::Server::SERVER_WORLD_ENTITY ThresholdBoss;
		std::map<LostArk::Shared::PLAYER_ID,
			LostArk::Server::SERVER_PLAYER> Players;
		std::map<LostArk::Shared::PLAYER_ID,
			LostArk::Server::SERVER_PLAYER> ReconcilePlayers;
		LostArk::Server::SERVER_PLAYER Target;
		LostArk::Server::CValtanBrain Brain;
		std::vector<LostArk::Shared::DAMAGE_EVENT> DamageEvents;
		std::vector<LostArk::Shared::GameplayDataRevision> RequiredPins;
	};
}
