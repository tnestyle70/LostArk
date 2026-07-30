#pragma once

#include "Client_Defines.h"
#include "Profiler.h"

#include <filesystem>

NS_BEGIN(Client)

class CProfilerCaptureIO final
{
public:
	static bool_t Save_Json(
		const Engine::FProfilerCaptureSnapshot& Snapshot,
		const filesystem::path& OutputPath,
		string* pOutError = nullptr);

	static filesystem::path Make_DefaultPath(uint64_t iFrameNumber);
};

NS_END
