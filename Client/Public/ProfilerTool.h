#pragma once

#include "Client_Defines.h"
#include "Profiler.h"
#include "ProfilerCaptureIO.h"

#include <array>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* Profiler panel presented by the current Debug F1/F7 routes. It only reads Engine::CProfiler aggregates and never
   owns timing data: the Engine profiler stays the single owner of scopes,
   counters and GPU queries. */
class CProfilerTool final
{
public:
    explicit CProfilerTool(ID3D11Device* pDevice = nullptr);
    void Begin_Capture(Engine::CProfiler& Profiler);
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
	void Render_FrameOverview();
	void Render_LongOperations();
	void Render_Counters() const;
	bool_t Refresh_CaptureFiles();
	void Render_CaptureFiles();
    FProfilerCaptureContext Sample_Context() const;
    void Render_FrameChanges(Engine::CProfiler& Profiler);
    void Render_Comparison(Engine::CProfiler& Profiler);

private:
	bool_t m_bOpen = true;
    FProfilerCaptureContext m_CaptureContext;
    bool_t m_bSaveWindowOnly = true;
	bool_t m_bShowUnobserved = true;
	bool_t m_bCatalogRegistered = false;
	int32_t m_iWindowFrameInput = 120;
	float m_fRefreshIntervalSeconds = 0.5f;
	double m_fLastRefreshTime = -1.0;
	uint32_t m_iMainThreadId = 0u;
	size_t m_iHistoryFrames = 0u;
    Engine::FProfilerCaptureWindow m_SaveWindowCoverage{};
	double m_fWindowCpuAvgMs = 0.0;
	double m_fWindowCpuMaxMs = 0.0;
	double m_fWindowGpuAvgMs = 0.0;
	double m_fWindowGpuMaxMs = 0.0;
	size_t m_iWindowFrames = 0u;
	Engine::FProfilerLiveStats m_Live{};
	Engine::FProfilerFrame m_LatestFrame{};
	double m_fUnattributedCpuMs = 0.0;
	bool_t m_bCpuSortSelf = true;
	bool_t m_bMainThreadOnly = true;
	bool_t m_bGpuSortSelf = true;
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
    FProfilerComparisonCapture m_ComparisonFrames;
    std::array<std::map<std::string, FProfilerComparisonMean>, 2> m_FrameComparisonMeans;
    std::array<FProfilerComparisonCapture, 2> m_Baselines;
    std::array<std::map<std::string, FProfilerComparisonMean>, 2> m_BaselineMeans;
    std::array<std::array<char, 241>, 2> m_BaselineLabels{};
    std::string m_ComparisonStatus;
    bool m_bComparisonRefresh = true, m_bFollowComparisonFrames = true;
    int m_iComparisonFrame = 0;
    int m_iComparedFrame = -1;
};

NS_END
