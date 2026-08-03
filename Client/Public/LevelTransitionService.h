#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>

NS_BEGIN(Client)

enum class LEVEL_TRANSITION_PHASE
{
	LOAD,
	ACTIVATE
};

struct LEVEL_TRANSITION_REQUEST final
{
	LEVEL_TRANSITION_PHASE ePhase = LEVEL_TRANSITION_PHASE::LOAD;
	LEVEL eTargetLevel = LEVEL::END;
	std::string strSource;
};

class CLevelTransitionService final
{
public:
	static bool_t Request_Load(
		LEVEL eTargetLevel,
		const char_t* pSource);
	static bool_t Request_Activation(
		LEVEL eTargetLevel,
		const char_t* pSource);
	static bool_t Try_Consume(LEVEL_TRANSITION_REQUEST& outRequest);
	static bool_t Is_Pending();
	static std::string Get_Status();
	static void Report_LoadFailure(HRESULT result);
	static bool_t Try_ConsumeLoadFailure(HRESULT& outResult);

private:
	static bool_t Request(
		LEVEL_TRANSITION_PHASE ePhase,
		LEVEL eTargetLevel,
		const char_t* pSource);
};

NS_END
