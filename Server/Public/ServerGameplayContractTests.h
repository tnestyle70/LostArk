#pragma once

//왜 이 테스트들을 따로 이렇게 header로 분리를 시켜둔 걸까?
namespace LostArk::Server
{
	//dimensionMaster의 ground target only가 guide의 이동에 대한 같이 따라가는 것을
	//검증하기 위해서?
	int Run_ServerGameplayContractTests(
		bool dimensionMasterGroundTargetOnly = false,
		bool debugTeleportOnly = false, bool koukuBundlesOnly = false,
		bool worldPlaybackOnly = false);
	int Run_ServerNavigationContractTests();
	int Run_ServerCardMazeContractTests();
	int Run_ServerBingoContractTests();
	int Run_ServerKoukuSupportSurfaceContractTests();
	int Run_ServerKoukuObjectOverlapContractTests();
	int Run_ServerVehicleRidingContractTests();
}
