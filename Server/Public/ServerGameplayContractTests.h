#pragma once

namespace LostArk::Server
{
	int Run_ServerGameplayContractTests(
		bool dimensionMasterGroundTargetOnly = false,
		bool debugTeleportOnly = false, bool koukuBundlesOnly = false);
	int Run_ServerNavigationContractTests();
	int Run_ServerKoukuSupportSurfaceContractTests();
	int Run_ServerKoukuObjectOverlapContractTests();
}
