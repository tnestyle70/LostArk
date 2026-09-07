#pragma once

namespace LostArk::Server
{
	int Run_ServerGameplayContractTests(
		bool dimensionMasterGroundTargetOnly = false,
		bool debugTeleportOnly = false,
		bool worldPlaybackOnly = false);
	int Run_ServerNavigationContractTests();
}
