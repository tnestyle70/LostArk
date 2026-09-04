#include "imgui.h"

#include "RenderingBenchmark.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace
{
	double Percentile(std::vector<double> Values, const double fPercentile)
	{
		if (Values.empty())
			return 0.0;
		std::sort(Values.begin(), Values.end());
		const double fRank = fPercentile * static_cast<double>(Values.size() - 1u);
		const size_t iLower = static_cast<size_t>(fRank);
		const size_t iUpper = (std::min)(iLower + 1u, Values.size() - 1u);
		const double fFraction = fRank - static_cast<double>(iLower);
		return Values[iLower] + (Values[iUpper] - Values[iLower]) * fFraction;
	}

	string Now_Timestamp()
	{
		const auto Now = chrono::system_clock::now();
		const time_t CalendarTime = chrono::system_clock::to_time_t(Now);
		tm LocalTime{};
		localtime_s(&LocalTime, &CalendarTime);
		ostringstream Stream;
		Stream << put_time(&LocalTime, "%Y-%m-%d %H:%M:%S");
		return Stream.str();
	}

	string Escape_Json(const string& Value)
	{
		string Escaped;
		Escaped.reserve(Value.size() + 8u);
		for (const char Character : Value)
		{
			switch (Character)
			{
			case '"': Escaped += "\\\""; break;
			case '\\': Escaped += "\\\\"; break;
			case '\n': Escaped += "\\n"; break;
			case '\r': Escaped += "\\r"; break;
			case '\t': Escaped += "\\t"; break;
			default: Escaped += Character; break;
			}
		}
		return Escaped;
	}
}

bool_t Client::CRenderingBenchmark::Begin(
	Engine::CProfiler* pProfiler,
	const string& strLabel,
	const uint32_t iFrames,
	const string& strQualitySummary,
	string& strOutStatus)
{
	if (nullptr == pProfiler)
	{
		strOutStatus = "Engine profiler is unavailable.";
		return false;
	}
	if (m_bCapturing)
	{
		strOutStatus = "A capture is already running.";
		return false;
	}
	/* A capture started from this ImGui frame must not measure the partially
	   elapsed activation frame. Keep one warm-up slot in addition to the GPU
	   readback tail inside the bounded history. */
	constexpr uint32_t MAXIMUM_CAPTURE_FRAMES =
		static_cast<uint32_t>(Engine::CProfiler::MAX_HISTORY_FRAMES) -
		Engine::CProfiler::GPU_READ_LATENCY - 1u;
	if (iFrames < 10u || iFrames > MAXIMUM_CAPTURE_FRAMES)
	{
		strOutStatus = "Frame count must be between 10 and " +
			std::to_string(MAXIMUM_CAPTURE_FRAMES) + ".";
		return false;
	}
	m_bProfilerWasEnabled = pProfiler->Is_Enabled();
	Engine::FProfilerLiveStats Live{};
	const bool_t bHadCompletedFrame = pProfiler->Get_LiveStats(Live);
	/* If profiling was already active, the frame containing this button click
	   will still be committed after Reset_History. Exclude it by identity. When
	   profiling starts here, its first measured frame begins on the next tick
	   and is a complete frame. */
	m_bSkipActivationFrame = m_bProfilerWasEnabled;
	m_iStartFrame = bHadCompletedFrame ?
		Live.FrameNumber + (m_bSkipActivationFrame ? 1u : 0u) :
		(m_bSkipActivationFrame ? 1u : 0u);
	pProfiler->Set_Enabled(true);
	pProfiler->Reset_History();
	m_iTargetFrames = iFrames;
	m_strLabel = strLabel.empty() ? "run" : strLabel;
	m_strQualitySummary = strQualitySummary;
	m_bCapturing = true;
	strOutStatus = "Capturing " + std::to_string(iFrames) + " frames for '" +
		m_strLabel + "'. Keep the camera still for a comparable result.";
	return true;
}

void Client::CRenderingBenchmark::Update(Engine::CProfiler* pProfiler)
{
	if (!m_bCapturing || nullptr == pProfiler)
		return;
	if (pProfiler->Get_HistoryFrameCount() <
		static_cast<size_t>(m_iTargetFrames) +
		Engine::CProfiler::GPU_READ_LATENCY +
		(m_bSkipActivationFrame ? 1u : 0u))
	{
		return;
	}
	(void)Finalize(*pProfiler);
}

bool_t Client::CRenderingBenchmark::Finalize(Engine::CProfiler& Profiler)
{
	m_bCapturing = false;
	const Engine::FProfilerCaptureSnapshot Snapshot = Profiler.Snapshot();
	Profiler.Set_Enabled(m_bProfilerWasEnabled);

	RENDERING_BENCHMARK_RUN Run;
	Run.strLabel = m_strLabel;
	Run.strTimestamp = Now_Timestamp();
	Run.strQualitySummary = m_strQualitySummary;
	std::vector<double> CpuMs;
	std::vector<double> GpuMs;
	double fDrawCalls = 0.0;
	double fInstances = 0.0;
	double fIndices = 0.0;
	double fPsInvocations = 0.0;
	const uint64_t iNewestFrame = Snapshot.Frames.empty() ?
		0u : Snapshot.Frames.back().FrameNumber;
	for (const Engine::FProfilerFrame& Frame : Snapshot.Frames)
	{
		/* Exclude the activation frame and the unresolved GPU-readback tail.
		   Thus every reported CPU sample covers a complete post-click frame and
		   every selected frame has reached the profiler's normal resolve age. */
		if (Frame.FrameNumber <= m_iStartFrame ||
			iNewestFrame < Frame.FrameNumber + Engine::CProfiler::GPU_READ_LATENCY)
		{
			continue;
		}
		if (Run.iFrames >= m_iTargetFrames)
			break;
		++Run.iFrames;
		CpuMs.push_back(Frame.CpuFrameMs);
		fDrawCalls += static_cast<double>(
			Frame.Counters[static_cast<size_t>(Engine::EProfilerCounter::DrawCalls)]);
		fInstances += static_cast<double>(
			Frame.Counters[static_cast<size_t>(Engine::EProfilerCounter::Instances)]);
		fIndices += static_cast<double>(
			Frame.Counters[static_cast<size_t>(Engine::EProfilerCounter::Indices)]);
		if (Frame.GpuValid)
		{
			GpuMs.push_back(Frame.GpuFrameMs);
			fPsInvocations += static_cast<double>(Frame.Pipeline.PSInvocations);
		}
	}
	if (0u == Run.iFrames)
	{
		m_strStatus = "Capture ended without any profiled frame.";
		return false;
	}
	const double fFrames = static_cast<double>(Run.iFrames);
	Run.iGpuFrames = static_cast<uint32_t>(GpuMs.size());
	Run.fCpuAvgMs = std::accumulate(CpuMs.begin(), CpuMs.end(), 0.0) / fFrames;
	Run.fCpuP95Ms = Percentile(CpuMs, 0.95);
	Run.fCpuMaxMs = *std::max_element(CpuMs.begin(), CpuMs.end());
	if (!GpuMs.empty())
	{
		Run.fGpuAvgMs = std::accumulate(GpuMs.begin(), GpuMs.end(), 0.0) /
			static_cast<double>(GpuMs.size());
		Run.fGpuP95Ms = Percentile(GpuMs, 0.95);
		Run.fGpuMaxMs = *std::max_element(GpuMs.begin(), GpuMs.end());
		Run.fPsInvocationsAvg = fPsInvocations / static_cast<double>(GpuMs.size());
	}
	Run.fDrawCallsAvg = fDrawCalls / fFrames;
	Run.fInstancesAvg = fInstances / fFrames;
	Run.fIndicesAvg = fIndices / fFrames;
	m_Runs.push_back(Run);

	string Error;
	const filesystem::path OutputPath = Make_DefaultPath();
	ostringstream Summary;
	Summary << fixed << setprecision(3)
		<< "Recorded '" << Run.strLabel << "': CPU " << Run.fCpuAvgMs
		<< " ms avg, GPU " << Run.fGpuAvgMs << " ms avg. ";
	if (Save_Json(m_Runs, OutputPath, Error))
		Summary << "Saved " << OutputPath.string();
	else
		Summary << "JSON save failed: " << Error;
	m_strStatus = Summary.str();
	return true;
}

void Client::CRenderingBenchmark::Render_Section(
	Engine::CProfiler* pProfiler,
	const string& strQualitySummary)
{
	ImGui::SeparatorText("Benchmark");
	ImGui::SetNextItemWidth(160.f);
	ImGui::InputText("Label", m_LabelBuffer.data(), m_LabelBuffer.size());
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::DragInt("Frames", &m_iFrameInput, 1.f, 10,
		static_cast<int32_t>(Engine::CProfiler::MAX_HISTORY_FRAMES -
			Engine::CProfiler::GPU_READ_LATENCY - 1u), "%d",
		ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine();
	ImGui::BeginDisabled(m_bCapturing);
	if (ImGui::Button("Capture"))
	{
		(void)Begin(pProfiler, m_LabelBuffer.data(),
			static_cast<uint32_t>(m_iFrameInput), strQualitySummary, m_strStatus);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Clear runs"))
		m_Runs.clear();
	if (m_bCapturing && nullptr != pProfiler)
	{
		ImGui::Text("Capturing: %zu / %u frames",
			pProfiler->Get_HistoryFrameCount(), m_iTargetFrames);
	}
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::TextDisabled("Current settings: %s", strQualitySummary.c_str());
	if (m_Runs.empty())
		return;
	constexpr ImGuiTableFlags TABLE_FLAGS = ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
	if (!ImGui::BeginTable("##RenderingBenchmarkRuns", 9, TABLE_FLAGS))
		return;
	ImGui::TableSetupColumn("Label");
	ImGui::TableSetupColumn("Frames");
	ImGui::TableSetupColumn("CPU avg");
	ImGui::TableSetupColumn("CPU p95");
	ImGui::TableSetupColumn("GPU avg");
	ImGui::TableSetupColumn("GPU p95");
	ImGui::TableSetupColumn("Draw calls");
	ImGui::TableSetupColumn("Instances");
	ImGui::TableSetupColumn("Settings");
	ImGui::TableHeadersRow();
	for (const RENDERING_BENCHMARK_RUN& Run : m_Runs)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(Run.strLabel.c_str());
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", Run.strTimestamp.c_str());
		ImGui::TableNextColumn();
		ImGui::Text("%u", Run.iFrames);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", Run.fCpuAvgMs);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", Run.fCpuP95Ms);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", Run.fGpuAvgMs);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", Run.fGpuP95Ms);
		ImGui::TableNextColumn();
		ImGui::Text("%.0f", Run.fDrawCallsAvg);
		ImGui::TableNextColumn();
		ImGui::Text("%.0f", Run.fInstancesAvg);
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(Run.strQualitySummary.c_str());
	}
	ImGui::EndTable();
}

bool_t Client::CRenderingBenchmark::Save_Json(
	const vector<RENDERING_BENCHMARK_RUN>& Runs,
	const filesystem::path& OutputPath,
	string& strOutError)
{
	error_code Error;
	if (!OutputPath.parent_path().empty())
		filesystem::create_directories(OutputPath.parent_path(), Error);
	if (Error)
	{
		strOutError = Error.message();
		return false;
	}
	filesystem::path TemporaryPath = OutputPath;
	TemporaryPath += L".tmp";
	ofstream Stream(TemporaryPath, ios::binary | ios::trunc);
	if (!Stream)
	{
		strOutError = "Cannot open benchmark JSON output.";
		return false;
	}
	Stream << fixed << setprecision(6);
	Stream << "{\n  \"schema\": \"LostArkRenderingBenchmark.v1\",\n  \"runs\": [\n";
	for (size_t iRun = 0u; iRun < Runs.size(); ++iRun)
	{
		const RENDERING_BENCHMARK_RUN& Run = Runs[iRun];
		Stream << "    {\n"
			<< "      \"label\": \"" << Escape_Json(Run.strLabel) << "\",\n"
			<< "      \"timestamp\": \"" << Escape_Json(Run.strTimestamp) << "\",\n"
			<< "      \"qualitySummary\": \"" << Escape_Json(Run.strQualitySummary) << "\",\n"
			<< "      \"frames\": " << Run.iFrames << ",\n"
			<< "      \"gpuFrames\": " << Run.iGpuFrames << ",\n"
			<< "      \"cpuAvgMs\": " << Run.fCpuAvgMs << ",\n"
			<< "      \"cpuP95Ms\": " << Run.fCpuP95Ms << ",\n"
			<< "      \"cpuMaxMs\": " << Run.fCpuMaxMs << ",\n"
			<< "      \"gpuAvgMs\": " << Run.fGpuAvgMs << ",\n"
			<< "      \"gpuP95Ms\": " << Run.fGpuP95Ms << ",\n"
			<< "      \"gpuMaxMs\": " << Run.fGpuMaxMs << ",\n"
			<< "      \"drawCallsAvg\": " << Run.fDrawCallsAvg << ",\n"
			<< "      \"instancesAvg\": " << Run.fInstancesAvg << ",\n"
			<< "      \"indicesAvg\": " << Run.fIndicesAvg << ",\n"
			<< "      \"psInvocationsAvg\": " << Run.fPsInvocationsAvg << "\n"
			<< "    }" << (iRun + 1u < Runs.size() ? "," : "") << "\n";
	}
	Stream << "  ]\n}\n";
	Stream.close();
	if (!Stream)
	{
		filesystem::remove(TemporaryPath, Error);
		strOutError = "Failed while writing benchmark JSON.";
		return false;
	}
	filesystem::rename(TemporaryPath, OutputPath, Error);
	if (Error)
	{
		filesystem::remove(TemporaryPath, Error);
		strOutError = "Cannot finalize benchmark JSON output.";
		return false;
	}
	return true;
}

filesystem::path Client::CRenderingBenchmark::Make_DefaultPath()
{
	wchar_t ModulePath[32768]{};
	const DWORD iLength = GetModuleFileNameW(
		nullptr, ModulePath, static_cast<DWORD>(size(ModulePath)));
	const filesystem::path BaseDirectory =
		0 != iLength && iLength < size(ModulePath) ?
		filesystem::path(ModulePath).parent_path() : filesystem::current_path();
	const auto Now = chrono::system_clock::now();
	const time_t CalendarTime = chrono::system_clock::to_time_t(Now);
	tm LocalTime{};
	localtime_s(&LocalTime, &CalendarTime);
	wostringstream FileName;
	FileName << L"benchmark_" << put_time(&LocalTime, L"%Y%m%d_%H%M%S") << L".json";
	return (BaseDirectory / L".." / L"BenchmarkCaptures" / FileName.str())
		.lexically_normal();
}
