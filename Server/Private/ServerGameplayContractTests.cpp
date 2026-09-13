#include "ServerGameplayContractTests.h"
#include "ServerGameplayContractTests_Runner.h"


int LostArk::Server::Run_ServerGameplayContractTests(
	const bool dimensionMasterGroundTargetOnly, const bool debugTeleportOnly,
	const bool koukuBundlesOnly, const bool worldPlaybackOnly)
{


	CONTRACT_TEST_RUN_CONTEXT context{ dimensionMasterGroundTargetOnly, debugTeleportOnly, koukuBundlesOnly, worldPlaybackOnly, 1 };
	const auto runContract = [](void* opaque)
	{
		CONTRACT_TEST_RUN_CONTEXT& context =
			*static_cast<CONTRACT_TEST_RUN_CONTEXT*>(opaque);
		context.result = CServerGameplayContractRunner::Run(context);
	};
	if (!Run_WithContractWorkerStack(runContract, &context))
	{
		std::cerr << "[FAILURE] Server gameplay contract worker did not complete\n";
		return 1;
	}
	return context.result;
}
