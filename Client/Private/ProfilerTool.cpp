#include "imgui.h"

#include "ProfilerTool.h"
#include "ProfilerCaptureIO.h"

#include <algorithm>
#include <cctype>

namespace
{
	constexpr std::array<const char*,
		static_cast<size_t>(Engine::EProfilerCounter::Count)>
		COUNTER_LABELS = {
			"Draw calls",
			"Instanced draw calls",
			"Instances",
			"Indices",
			"Submissions: priority",
			"Submissions: shadow",
			"Submissions: non-blend",
			"Submissions: blend",
			"Map placements",
			"Map visible instances",
			"Map batches",
			"Map fallback objects",
			"Texture requests",
			"Texture path hits",
			"Texture content hits",
			"Texture unique SRVs",
			"Texture estimated GPU bytes",
			"Navigation queries",
			"Navigation expanded nodes",
			"Navigation query microseconds",
			"Navigation path cells",
	};

	bool Contains_CaseInsensitive(const std::string& Text, const char* pQuery)
	{
		if (nullptr == pQuery || '\0' == pQuery[0])
			return true;
		const std::string Query(pQuery);
		const auto Found = std::search(
			Text.begin(), Text.end(), Query.begin(), Query.end(),
			[](const char a, const char b)
			{
				return std::tolower(static_cast<unsigned char>(a)) ==
					std::tolower(static_cast<unsigned char>(b));
			});
		return Found != Text.end();
	}
}

void Client::CProfilerTool::Refresh(Engine::CProfiler& Profiler)
{
	m_iMainThreadId = Profiler.Get_MainThreadId();
	m_iHistoryFrames = Profiler.Get_HistoryFrameCount();
	Profiler.Get_ScopeNames(m_ScopeNames);
	const size_t iWindow = static_cast<size_t>((std::max)(m_iWindowFrameInput, 1));
	Profiler.Get_ScopeAggregates(iWindow, m_Aggregates);
	Profiler.Get_WindowFrameStats(
		iWindow, m_fWindowCpuAvgMs, m_fWindowCpuMaxMs,
		m_fWindowGpuAvgMs, m_fWindowGpuMaxMs, m_iWindowFrames);
	Profiler.Get_LongOperations(m_LongOperations);
	m_bLiveValid = Profiler.Get_LiveStats(m_Live);
}

const char_t* Client::CProfilerTool::Scope_Name(const uint32_t iNameId) const
{
	return iNameId < m_ScopeNames.size() ?
		m_ScopeNames[iNameId].c_str() : "<unknown>";
}

std::string Client::CProfilerTool::Thread_Label(const uint32_t iThreadId) const
{
	if (iThreadId == m_iMainThreadId)
		return "main";
	return "worker " + std::to_string(iThreadId);
}

void Client::CProfilerTool::Render(Engine::CProfiler* pProfiler)
{
	if (!m_bOpen)
		return;
	ImGui::SetNextWindowSize(ImVec2(960.f, 640.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Profiler###LostArkProfilerToolV1", &m_bOpen))
	{
		ImGui::End();
		return;
	}
	if (nullptr == pProfiler)
	{
		ImGui::TextUnformatted("Engine profiler is unavailable.");
		ImGui::End();
		return;
	}

	bool_t bEnabled = pProfiler->Is_Enabled();
	if (ImGui::Checkbox("Capture", &bEnabled))
		pProfiler->Set_Enabled(bEnabled);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::DragInt("Window (frames)", &m_iWindowFrameInput, 1.f, 1,
		static_cast<int32_t>(Engine::CProfiler::MAX_HISTORY_FRAMES),
		"%d", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
		pProfiler->Reset_History();
	ImGui::SameLine();
	if (ImGui::Button("Save JSON"))
	{
		const Engine::FProfilerCaptureSnapshot Snapshot = pProfiler->Snapshot();
		const uint64_t iFrameNumber = Snapshot.Frames.empty() ?
			0u : Snapshot.Frames.back().FrameNumber;
		const filesystem::path OutputPath =
			CProfilerCaptureIO::Make_DefaultPath(iFrameNumber);
		string Error;
		m_strCaptureStatus = CProfilerCaptureIO::Save_Json(
			Snapshot, OutputPath, &Error) ?
			"Saved " + OutputPath.string() : Error;
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(220.f);
	ImGui::InputTextWithHint("##ProfilerFilter", "Filter scope name",
		m_Filter.data(), m_Filter.size());

	const double fNow = ImGui::GetTime();
	if (m_fLastRefreshTime < 0.0 ||
		fNow - m_fLastRefreshTime >= m_fRefreshIntervalSeconds)
	{
		Refresh(*pProfiler);
		m_fLastRefreshTime = fNow;
	}

	const ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("FPS %.1f | window %zu frames: CPU avg %.3f ms max %.3f ms | GPU avg %.3f ms max %.3f ms | history %zu",
		io.Framerate, m_iWindowFrames, m_fWindowCpuAvgMs, m_fWindowCpuMaxMs,
		m_fWindowGpuAvgMs, m_fWindowGpuMaxMs, m_iHistoryFrames);
	if (!bEnabled)
		ImGui::TextDisabled("Capture is off. Turn it on before the operation you want to measure (level load, tool reload, pattern playback).");
	if (!m_strCaptureStatus.empty())
		ImGui::TextWrapped("%s", m_strCaptureStatus.c_str());

	if (ImGui::BeginTabBar("##ProfilerTabs"))
	{
		if (ImGui::BeginTabItem("Bottlenecks"))
		{
			Render_Bottlenecks();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Long operations"))
		{
			Render_LongOperations();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Counters"))
		{
			Render_Counters();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void Client::CProfilerTool::Render_Bottlenecks()
{
	const double fFrames = static_cast<double>((std::max)(m_iWindowFrames, size_t{ 1u }));
	const double fFrameTotalMs = m_fWindowCpuAvgMs * fFrames;
	constexpr ImGuiTableFlags TABLE_FLAGS = ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
		ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;
	if (!ImGui::BeginTable("##ProfilerBottlenecks", 7, TABLE_FLAGS))
		return;
	ImGui::TableSetupScrollFreeze(0, 1);
	ImGui::TableSetupColumn("Scope", ImGuiTableColumnFlags_WidthStretch, 3.f);
	ImGui::TableSetupColumn("Thread", ImGuiTableColumnFlags_WidthStretch, 1.f);
	ImGui::TableSetupColumn("Calls/frame", ImGuiTableColumnFlags_WidthStretch, 1.f);
	ImGui::TableSetupColumn("Avg ms/frame", ImGuiTableColumnFlags_WidthStretch, 1.f);
	ImGui::TableSetupColumn("Self ms/frame", ImGuiTableColumnFlags_WidthStretch, 1.f);
	ImGui::TableSetupColumn("Max ms", ImGuiTableColumnFlags_WidthStretch, 1.f);
	ImGui::TableSetupColumn("% CPU", ImGuiTableColumnFlags_WidthStretch, 1.f);
	ImGui::TableHeadersRow();
	for (const Engine::FProfilerScopeAggregate& Aggregate : m_Aggregates)
	{
		const std::string Name = Scope_Name(Aggregate.NameId);
		if (!Contains_CaseInsensitive(Name, m_Filter.data()))
			continue;
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(Name.c_str());
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(Thread_Label(Aggregate.ThreadId).c_str());
		ImGui::TableNextColumn();
		ImGui::Text("%.2f", static_cast<double>(Aggregate.Calls) / fFrames);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", Aggregate.InclusiveMs / fFrames);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", Aggregate.SelfMs / fFrames);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", Aggregate.MaxMs);
		ImGui::TableNextColumn();
		ImGui::Text("%.1f", fFrameTotalMs > 0.0 ?
			Aggregate.InclusiveMs * 100.0 / fFrameTotalMs : 0.0);
	}
	ImGui::EndTable();
}

void Client::CProfilerTool::Render_LongOperations()
{
	ImGui::TextDisabled("Completed scopes of %.0f ms or more, newest first. Worker-thread loads and JSON parses appear here even when they span many frames.",
		Engine::CProfiler::LONG_OPERATION_THRESHOLD_MS);
	constexpr ImGuiTableFlags TABLE_FLAGS = ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
		ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;
	if (!ImGui::BeginTable("##ProfilerLongOperations", 4, TABLE_FLAGS))
		return;
	ImGui::TableSetupScrollFreeze(0, 1);
	ImGui::TableSetupColumn("Scope", ImGuiTableColumnFlags_WidthStretch, 3.f);
	ImGui::TableSetupColumn("Thread", ImGuiTableColumnFlags_WidthStretch, 1.f);
	ImGui::TableSetupColumn("Duration ms", ImGuiTableColumnFlags_WidthStretch, 1.f);
	ImGui::TableSetupColumn("Frame", ImGuiTableColumnFlags_WidthStretch, 1.f);
	ImGui::TableHeadersRow();
	for (auto It = m_LongOperations.rbegin(); It != m_LongOperations.rend(); ++It)
	{
		const std::string Name = Scope_Name(It->NameId);
		if (!Contains_CaseInsensitive(Name, m_Filter.data()))
			continue;
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(Name.c_str());
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(Thread_Label(It->ThreadId).c_str());
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", It->DurationMs);
		ImGui::TableNextColumn();
		ImGui::Text("%llu", static_cast<unsigned long long>(It->FrameNumber));
	}
	ImGui::EndTable();
}

void Client::CProfilerTool::Render_Counters() const
{
	if (!m_bLiveValid)
	{
		ImGui::TextDisabled("No captured frame yet.");
		return;
	}
	ImGui::Text("Frame %llu | CPU %.3f ms | GPU %s",
		static_cast<unsigned long long>(m_Live.FrameNumber),
		m_Live.CpuFrameMs,
		m_Live.GpuValid ? "valid" : "pending");
	if (m_Live.GpuValid)
	{
		ImGui::Text("GPU %.3f ms (latency %u frames) | IA vertices %llu | VS %llu | PS %llu | primitives %llu",
			m_Live.GpuFrameMs, m_Live.GpuLatencyFrames,
			static_cast<unsigned long long>(m_Live.Pipeline.IAVertices),
			static_cast<unsigned long long>(m_Live.Pipeline.VSInvocations),
			static_cast<unsigned long long>(m_Live.Pipeline.PSInvocations),
			static_cast<unsigned long long>(m_Live.Pipeline.IAPrimitives));
	}
	if (!ImGui::BeginTable("##ProfilerCounters", 2,
			ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
			ImGuiTableFlags_SizingStretchProp))
	{
		return;
	}
	ImGui::TableSetupColumn("Counter", ImGuiTableColumnFlags_WidthStretch, 2.f);
	ImGui::TableSetupColumn("Last frame", ImGuiTableColumnFlags_WidthStretch, 1.f);
	ImGui::TableHeadersRow();
	for (size_t iCounter = 0u; iCounter < COUNTER_LABELS.size(); ++iCounter)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(COUNTER_LABELS[iCounter]);
		ImGui::TableNextColumn();
		ImGui::Text("%llu",
			static_cast<unsigned long long>(m_Live.Counters[iCounter]));
	}
	ImGui::EndTable();
}
