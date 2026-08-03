#pragma once

#include "ClientLaunchOptions.h"
#include "Client_Defines.h"

#include <string>

NS_BEGIN(Client)

struct SCENE_TRANSITION_REQUEST final
{
	LEVEL eTargetLevel = LEVEL::END;
	CLIENT_SCENARIO eScenario = CLIENT_SCENARIO::END;
	std::string strSource;
};

class CSceneTransitionService final
{
public:
	static bool_t Request(
		LEVEL eTargetLevel,
		CLIENT_SCENARIO eScenario,
		const char_t* pSource);
	static bool_t Try_Consume(SCENE_TRANSITION_REQUEST& outRequest);
	static bool_t Is_Pending();
	static const std::string& Get_Status();
	static void Report_LoadFailure(HRESULT result);
	static bool_t Try_ConsumeLoadFailure(HRESULT& outResult);

private:
	static bool_t Is_Compatible(
		LEVEL eTargetLevel,
		CLIENT_SCENARIO eScenario);
};

NS_END
