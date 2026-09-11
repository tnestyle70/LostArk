#pragma once

#include "Client_Defines.h"
#include "Profiler.h"
#include "ProfilerCaptureIO.h"

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
	void Request_Save(Engine::CProfiler& Profiler);
	void Update_SaveState();
	[[nodiscard]] bool_t Is_Saving() const noexcept { return m_Exporter.IsSaving(); }
	[[nodiscard]] const std::string& Get_CaptureStatus() const noexcept { return m_strCaptureStatus; }

private:
	void Refresh(Engine::CProfiler& Profiler);
	const char_t* Scope_Name(uint32_t iNameId) const;
	std::string Thread_Label(uint32_t iThreadId) const;
	void Render_Bottlenecks(bool_t bImGuiOnly = false);
	void Render_ImGui();
	void Rebuild_CpuRows(bool_t bImGuiOnly);
	void Render_Gpu();
	void Render_LongOperations();
	void Render_Counters() const;
	bool_t Refresh_CaptureFiles();
	void Render_CaptureFiles();

private:
	bool_t m_bOpen = true;
	bool_t m_bShowUnobserved = true;
	bool_t m_bCatalogRegistered = false;
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
	struct FVisibleCpuRow final
	{
		const char* Name = nullptr;
		const Engine::FProfilerScopeAggregate* Aggregate = nullptr;
	};
	std::vector<FVisibleCpuRow> m_VisibleCpuRows;
	std::string m_strCpuRowFilter;
	bool_t m_bCpuRowsDirty = true;
	bool_t m_bCpuRowsImGuiOnly = false;
	bool_t m_bCpuRowsShowUnobserved = true;
	std::vector<Engine::FProfilerGpuScopeAggregate> m_GpuAggregates;
	size_t m_iGpuValidFrames = 0u;
	size_t m_iGpuFrameValidFrames = 0u;
	size_t m_iGpuPartialFrames = 0u;
	std::vector<Engine::FProfilerLongOperation> m_LongOperations;
	std::array<char_t, 96> m_Filter = {};
	std::array<char_t, 241> m_CaptureName = {};
	std::vector<FProfilerCaptureFile> m_CaptureFiles;
	std::string m_strSelectedCaptureId;
	std::string m_strCaptureFilesStatus;
	bool_t m_bCaptureFilesLoaded = false;
	std::string m_strCaptureStatus;
	CProfilerCaptureExporter m_Exporter;
};

NS_END
