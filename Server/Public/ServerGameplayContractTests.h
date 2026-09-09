#pragma once

namespace LostArk::Server
{
	int Run_ServerGameplayContractTests(
		bool dimensionMasterGroundTargetOnly = false,
		bool debugTeleportOnly = false, bool koukuBundlesOnly = false,
		bool worldPlaybackOnly = false);
	int Run_ServerNavigationContractTests();
	int Run_ServerCardMazeContractTests();
	int Run_ServerBingoContractTests();
	int Run_ServerKoukuSupportSurfaceContractTests();
	int Run_ServerKoukuObjectOverlapContractTests();
}
