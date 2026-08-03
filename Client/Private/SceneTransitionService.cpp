#include "SceneTransitionService.h"

#include "LevelRegistry.h"

#include <optional>

namespace
{
	std::optional<Client::SCENE_TRANSITION_REQUEST> g_PendingRequest;
	std::optional<HRESULT> g_LoadFailure;
	std::string g_Status = "No scene transition is pending.";
}

bool_t Client::CSceneTransitionService::Request(
	const LEVEL eTargetLevel,
	const CLIENT_SCENARIO eScenario,
	const char_t* pSource)
{
	if (nullptr == CLevelRegistry::Find(eTargetLevel) ||
		!Is_Compatible(eTargetLevel, eScenario) ||
		nullptr == pSource || '\0' == *pSource)
	{
		g_Status = "Rejected invalid scene transition request.";
		return false;
	}

	if (g_PendingRequest.has_value())
	{
		g_Status = "Rejected scene transition while another request is pending.";
		return false;
	}

	g_PendingRequest = SCENE_TRANSITION_REQUEST{
		eTargetLevel,
		eScenario,
		pSource
	};
	g_LoadFailure.reset();
	g_Status = "Scene transition request staged by " +
		g_PendingRequest->strSource + ".";
	return true;
}

bool_t Client::CSceneTransitionService::Try_Consume(
	SCENE_TRANSITION_REQUEST& outRequest)
{
	if (!g_PendingRequest.has_value())
		return false;

	outRequest = std::move(*g_PendingRequest);
	g_PendingRequest.reset();
	g_Status = "Scene transition request consumed.";
	return true;
}

bool_t Client::CSceneTransitionService::Is_Pending()
{
	return g_PendingRequest.has_value();
}

const std::string& Client::CSceneTransitionService::Get_Status()
{
	return g_Status;
}

void Client::CSceneTransitionService::Report_LoadFailure(
	const HRESULT result)
{
	g_LoadFailure = result;
	g_Status = "Level loading failed with HRESULT " +
		std::to_string(static_cast<long>(result)) + ".";
}

bool_t Client::CSceneTransitionService::Try_ConsumeLoadFailure(
	HRESULT& outResult)
{
	if (!g_LoadFailure.has_value())
		return false;
	outResult = *g_LoadFailure;
	g_LoadFailure.reset();
	return true;
}

bool_t Client::CSceneTransitionService::Is_Compatible(
	const LEVEL eTargetLevel,
	const CLIENT_SCENARIO eScenario)
{
	switch (eTargetLevel)
	{
	case LEVEL::LOBBY:
		return CLIENT_SCENARIO::FRONT_LOBBY == eScenario;

	case LEVEL::BERN:
		return CLIENT_SCENARIO::WORLD_BERN == eScenario;

	case LEVEL::VALTAN_ARENA:
		return CLIENT_SCENARIO::RAID_VALTAN_ARENA == eScenario;

	case LEVEL::DEVELOPMENT:
		return CLIENT_SCENARIO::DEVELOPMENT_TRAINING_GROUND == eScenario ||
			CLIENT_SCENARIO::DEVELOPMENT_MAP == eScenario ||
			CLIENT_SCENARIO::DEVELOPMENT_CHARACTER == eScenario ||
			CLIENT_SCENARIO::DEVELOPMENT_HDR == eScenario ||
			CLIENT_SCENARIO::DEVELOPMENT_EFFECT == eScenario ||
			CLIENT_SCENARIO::DEVELOPMENT_UI == eScenario;

	default:
		return false;
	}
}
