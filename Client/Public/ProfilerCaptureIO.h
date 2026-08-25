#pragma once

#include "Client_Defines.h"
#include "Profiler.h"

#include <filesystem>

NS_BEGIN(Client)

struct PROFILER_CAPTURE_METADATA final
{
	uint32_t iProcessId = 0;
	uint32_t iClientSlot = 0;
	uint32_t iLevelId = 0;
	uint32_t iWidth = 0;
	uint32_t iHeight = 0;
	uint64_t iWarmupFrames = 0;
	uint64_t iRequestedFrames = 0;
	uint64_t iCaptureStartUtc100ns = 0;
	uint64_t iCaptureEndUtc100ns = 0;
	string strConfiguration;
	string strRunId;
	string strCharacterClass;
	string strPhase;
	string strBuildRevision;
	string strDataRevision;
};

class CProfilerCaptureIO final
{
public:
	static bool_t Save_Json(
		const Engine::FProfilerCaptureSnapshot& Snapshot,
		const filesystem::path& OutputPath,
		string* pOutError = nullptr);

	static bool_t Save_Json(
		const Engine::FProfilerCaptureSnapshot& Snapshot,
		const PROFILER_CAPTURE_METADATA& Metadata,
		const filesystem::path& OutputPath,
		string* pOutError = nullptr);

	static filesystem::path Make_DefaultPath(uint64_t iFrameNumber);
};

NS_END
