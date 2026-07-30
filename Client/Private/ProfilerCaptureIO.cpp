#include "ProfilerCaptureIO.h"

#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{
	constexpr std::array<const char*,
		static_cast<size_t>(Engine::EProfilerCounter::Count)>
		CounterNames = {
			"drawCalls",
			"instancedDrawCalls",
			"instances",
			"indices",
			"renderSubmissionsPriority",
			"renderSubmissionsShadow",
			"renderSubmissionsNonBlend",
			"renderSubmissionsBlend",
			"mapPlacements",
			"mapVisibleInstances",
			"mapBatchCount",
			"mapFallbackObjects",
			"textureRequests",
			"texturePathHits",
			"textureContentHits",
			"textureUniqueSrvs",
			"textureEstimatedGpuBytes",
			"navigationQueries",
			"navigationExpandedNodes",
			"navigationQueryMicroseconds",
			"navigationPathCells",
	};

	string EscapeJson(const string& Value)
	{
		ostringstream Stream;
		for (const unsigned char Character : Value)
		{
			switch (Character)
			{
			case '"': Stream << "\\\""; break;
			case '\\': Stream << "\\\\"; break;
			case '\b': Stream << "\\b"; break;
			case '\f': Stream << "\\f"; break;
			case '\n': Stream << "\\n"; break;
			case '\r': Stream << "\\r"; break;
			case '\t': Stream << "\\t"; break;
			default:
				if (Character < 0x20)
				{
					Stream << "\\u"
						<< hex << setw(4) << setfill('0')
						<< static_cast<uint32_t>(Character)
						<< dec << setfill(' ');
				}
				else
				{
					Stream << Character;
				}
				break;
			}
		}
		return Stream.str();
	}

	void SetError(string* pOutError, const string& Error)
	{
		if (nullptr != pOutError)
			*pOutError = Error;
	}
}

bool_t Client::CProfilerCaptureIO::Save_Json(
	const Engine::FProfilerCaptureSnapshot& Snapshot,
	const filesystem::path& OutputPath,
	string* pOutError)
{
	if (OutputPath.empty())
	{
		SetError(pOutError, "Profiler output path is empty.");
		return false;
	}

	error_code Error;
	if (!OutputPath.parent_path().empty())
		filesystem::create_directories(OutputPath.parent_path(), Error);
	if (Error)
	{
		SetError(pOutError, Error.message());
		return false;
	}

	filesystem::path TemporaryPath = OutputPath;
	TemporaryPath += L".tmp";
	filesystem::remove(TemporaryPath, Error);
	Error.clear();

	ofstream Stream(TemporaryPath, ios::binary | ios::trunc);
	if (!Stream)
	{
		SetError(pOutError, "Cannot open profiler JSON output.");
		return false;
	}

	Stream << fixed << setprecision(6);
	Stream << "{\n";
	Stream << "  \"schema\": \"LostArkProfilerCapture.v1\",\n";
	Stream << "  \"droppedCpuScopes\": " << Snapshot.DroppedCpuScopes << ",\n";
	Stream << "  \"droppedGpuFrames\": " << Snapshot.DroppedGpuFrames << ",\n";
	Stream << "  \"scopeNames\": [";
	for (size_t i = 0; i < Snapshot.ScopeNames.size(); ++i)
	{
		if (0 != i)
			Stream << ", ";
		Stream << "\"" << EscapeJson(Snapshot.ScopeNames[i]) << "\"";
	}
	Stream << "],\n";
	Stream << "  \"frames\": [\n";

	for (size_t iFrame = 0; iFrame < Snapshot.Frames.size(); ++iFrame)
	{
		const Engine::FProfilerFrame& Frame = Snapshot.Frames[iFrame];
		Stream << "    {\n";
		Stream << "      \"frameNumber\": " << Frame.FrameNumber << ",\n";
		Stream << "      \"cpuFrameMs\": " << Frame.CpuFrameMs << ",\n";
		Stream << "      \"gpuFrameMs\": " << Frame.GpuFrameMs << ",\n";
		Stream << "      \"gpuValid\": " << (Frame.GpuValid ? "true" : "false") << ",\n";
		Stream << "      \"gpuLatencyFrames\": " << Frame.GpuLatencyFrames << ",\n";
		Stream << "      \"counters\": {\n";
		for (size_t iCounter = 0; iCounter < CounterNames.size(); ++iCounter)
		{
			Stream << "        \"" << CounterNames[iCounter] << "\": "
				<< Frame.Counters[iCounter]
				<< (iCounter + 1 < CounterNames.size() ? "," : "")
				<< "\n";
		}
		Stream << "      },\n";
		Stream << "      \"pipeline\": {\n";
		Stream << "        \"iaVertices\": " << Frame.Pipeline.IAVertices << ",\n";
		Stream << "        \"iaPrimitives\": " << Frame.Pipeline.IAPrimitives << ",\n";
		Stream << "        \"vsInvocations\": " << Frame.Pipeline.VSInvocations << ",\n";
		Stream << "        \"gsInvocations\": " << Frame.Pipeline.GSInvocations << ",\n";
		Stream << "        \"gsPrimitives\": " << Frame.Pipeline.GSPrimitives << ",\n";
		Stream << "        \"clipperInvocations\": " << Frame.Pipeline.CInvocations << ",\n";
		Stream << "        \"clipperPrimitives\": " << Frame.Pipeline.CPrimitives << ",\n";
		Stream << "        \"psInvocations\": " << Frame.Pipeline.PSInvocations << ",\n";
		Stream << "        \"hsInvocations\": " << Frame.Pipeline.HSInvocations << ",\n";
		Stream << "        \"dsInvocations\": " << Frame.Pipeline.DSInvocations << ",\n";
		Stream << "        \"csInvocations\": " << Frame.Pipeline.CSInvocations << "\n";
		Stream << "      },\n";
		Stream << "      \"cpuScopes\": [\n";
		for (size_t iScope = 0; iScope < Frame.CpuScopes.size(); ++iScope)
		{
			const Engine::FProfilerScopeSample& Scope = Frame.CpuScopes[iScope];
			Stream << "        {\"nameId\": " << Scope.NameId
				<< ", \"depth\": " << Scope.Depth
				<< ", \"beginTick\": " << Scope.BeginTick
				<< ", \"endTick\": " << Scope.EndTick << "}"
				<< (iScope + 1 < Frame.CpuScopes.size() ? "," : "")
				<< "\n";
		}
		Stream << "      ]\n";
		Stream << "    }"
			<< (iFrame + 1 < Snapshot.Frames.size() ? "," : "")
			<< "\n";
	}

	Stream << "  ]\n";
	Stream << "}\n";
	Stream.close();
	if (!Stream)
	{
		filesystem::remove(TemporaryPath, Error);
		SetError(pOutError, "Failed while writing profiler JSON.");
		return false;
	}

	filesystem::rename(TemporaryPath, OutputPath, Error);
	if (Error)
	{
		filesystem::remove(TemporaryPath, Error);
		SetError(pOutError, "Cannot finalize profiler JSON output.");
		return false;
	}

	if (nullptr != pOutError)
		pOutError->clear();
	return true;
}

filesystem::path Client::CProfilerCaptureIO::Make_DefaultPath(
	uint64_t iFrameNumber)
{
	wchar_t ModulePath[32768]{};
	const DWORD iLength = GetModuleFileNameW(
		nullptr,
		ModulePath,
		static_cast<DWORD>(size(ModulePath)));

	filesystem::path BaseDirectory =
		0 != iLength && iLength < size(ModulePath) ?
		filesystem::path(ModulePath).parent_path() :
		filesystem::current_path();

	const auto Now = chrono::system_clock::now();
	const time_t CalendarTime = chrono::system_clock::to_time_t(Now);
	tm LocalTime{};
	localtime_s(&LocalTime, &CalendarTime);
	const auto Milliseconds = chrono::duration_cast<chrono::milliseconds>(
		Now.time_since_epoch()).count() % 1000;

	wostringstream FileName;
	FileName << L"profiler_"
		<< put_time(&LocalTime, L"%Y%m%d_%H%M%S")
		<< L"_" << setw(3) << setfill(L'0') << Milliseconds
		<< L"_frame" << iFrameNumber << L".json";

	return (BaseDirectory / L".." / L"ProfilerCaptures" /
		FileName.str()).lexically_normal();
}
