#pragma once

#include "Client_Defines.h"
#include "Profiler.h"

#include <array>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* F1 Profiler window. It only reads Engine::CProfiler aggregates and never
   owns timing data: the Engine profiler stays the single owner of scopes,
   counters and GPU queries. */
class CProfilerTool final
{
public:
	void Open() { m_bOpen = true; }
	[[nodiscard]] bool_t Is_Open() const noexcept { return m_bOpen; }
	void Render(Engine::CProfiler* pProfiler);

private:
	void Refresh(Engine::CProfiler& Profiler);
	const char_t* Scope_Name(uint32_t iNameId) const;
	std::string Thread_Label(uint32_t iThreadId) const;
	void Render_Bottlenecks();
	void Render_LongOperations();
	void Render_Counters() const;

private:
	bool_t m_bOpen = true;
	int32_t m_iWindowFrameInput = 120;
	float m_fRefreshIntervalSeconds = 0.5f;
	double m_fLastRefreshTime = -1.0;
	uint32_t m_iMainThreadId = 0u;
	size_t m_iHistoryFrames = 0u;
	double m_fWindowCpuAvgMs = 0.0;
	double m_fWindowCpuMaxMs = 0.0;
	double m_fWindowGpuAvgMs = 0.0;
	double m_fWindowGpuMaxMs = 0.0;
	size_t m_iWindowFrames = 0u;
	Engine::FProfilerLiveStats m_Live{};
	bool_t m_bLiveValid = false;
	std::vector<std::string> m_ScopeNames;
	std::vector<Engine::FProfilerScopeAggregate> m_Aggregates;
	std::vector<Engine::FProfilerLongOperation> m_LongOperations;
	std::array<char_t, 96> m_Filter = {};
	std::string m_strCaptureStatus;
};

NS_END
