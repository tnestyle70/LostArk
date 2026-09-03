#pragma once

#include "Client_Defines.h"
#include "Profiler.h"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

struct RENDERING_BENCHMARK_RUN final
{
	string strLabel;
	string strTimestamp;
	string strQualitySummary;
	uint32_t iFrames = 0u;
	uint32_t iGpuFrames = 0u;
	double fCpuAvgMs = 0.0;
	double fCpuP95Ms = 0.0;
	double fCpuMaxMs = 0.0;
	double fGpuAvgMs = 0.0;
	double fGpuP95Ms = 0.0;
	double fGpuMaxMs = 0.0;
	double fDrawCallsAvg = 0.0;
	double fInstancesAvg = 0.0;
	double fIndicesAvg = 0.0;
	double fPsInvocationsAvg = 0.0;
};

/* Rendering Workbench benchmark: captures N frames through the Engine
   profiler and records CPU/GPU/draw statistics next to the quality settings
   that were active, so A/B changes to SSAO/Bloom/Shadow/Fog are measured on
   the same basis. It never changes rendering settings itself. */
class CRenderingBenchmark final
{
public:
	[[nodiscard]] bool_t Is_Capturing() const noexcept { return m_bCapturing; }
	bool_t Begin(
		Engine::CProfiler* pProfiler,
		const string& strLabel,
		uint32_t iFrames,
		const string& strQualitySummary,
		string& strOutStatus);
	/* Called once per frame. Finalizes the run when enough frames exist. */
	void Update(Engine::CProfiler* pProfiler);
	void Render_Section(
		Engine::CProfiler* pProfiler,
		const string& strQualitySummary);

private:
	bool_t Finalize(Engine::CProfiler& Profiler);
	static bool_t Save_Json(
		const vector<RENDERING_BENCHMARK_RUN>& Runs,
		const filesystem::path& OutputPath,
		string& strOutError);
	static filesystem::path Make_DefaultPath();

private:
	bool_t m_bCapturing = false;
	bool_t m_bProfilerWasEnabled = false;
	bool_t m_bSkipActivationFrame = false;
	uint64_t m_iStartFrame = 0u;
	uint32_t m_iTargetFrames = 0u;
	string m_strLabel;
	string m_strQualitySummary;
	string m_strStatus = "Idle. Capture measures the current viewport with the current quality settings.";
	array<char_t, 64> m_LabelBuffer = { "baseline" };
	int32_t m_iFrameInput = 300;
	vector<RENDERING_BENCHMARK_RUN> m_Runs;
};

NS_END
