#include "ProfilerCaptureIO.h"

#include <array>
#include <algorithm>
#include <cwctype>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>
#include <system_error>

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
			"sceneColorCopies",
			"sceneColorCopyBytes",
			"imGuiDrawLists",
			"imGuiVertices",
			"imGuiIndices",
			"imGuiDrawCommands",
			"imGuiDrawCalls",
			"imGuiCallbacks",
			"imGuiRenderWindows",
			"imGuiActiveWindows",
			"imGuiPlatformViewports",
			"imGuiVertexUploadBytes",
			"imGuiIndexUploadBytes",
			"imGuiConstantUploadBytes",
			"imGuiTextureUploadBytes",
			"imGuiBufferGrowths",
			"imGuiTextureCreates",
			"imGuiTextureUpdates",
			"imGuiDeviceObjectBuilds",
			"imGuiBufferMapFailures",
			"pickingReadbacks",
			"pickingReadbackBytes",
            "indirectDrawCalls",
            "indirectIndexUpperBound",
	};

	const char* GpuStatusName(const Engine::EProfilerGpuFrameStatus Status)
	{
		switch (Status)
		{
		case Engine::EProfilerGpuFrameStatus::Unsupported: return "unsupported";
		case Engine::EProfilerGpuFrameStatus::Pending: return "pending";
		case Engine::EProfilerGpuFrameStatus::Valid: return "valid";
		case Engine::EProfilerGpuFrameStatus::Disjoint: return "disjoint";
		case Engine::EProfilerGpuFrameStatus::Dropped: return "dropped";
		case Engine::EProfilerGpuFrameStatus::Error: return "error";
		}
		return nullptr;
	}

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

	struct FTemporaryCapture final
	{
		filesystem::path Path;
		~FTemporaryCapture()
		{
			error_code Ignored;
			filesystem::remove(Path, Ignored);
		}
	};

	bool Cancelled(const std::atomic_bool* pCancel, string* pOutError)
	{
		if (nullptr == pCancel || !pCancel->load(std::memory_order_relaxed))
			return false;
		SetError(pOutError, "Profiler capture save cancelled during shutdown.");
		return true;
	}

bool SaveJsonImpl(
	const Engine::FProfilerCaptureSnapshot& Snapshot,
	const filesystem::path& OutputPath,
	string* pOutError, const std::atomic_bool* pCancel, bool ReplaceExisting)
{
	if (Cancelled(pCancel, pOutError))
		return false;
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
		SetError(pOutError, "Cannot create profiler capture directory: " + Error.message());
		return false;
	}

	static std::atomic_uint64_t TemporarySequence = 0;
	FTemporaryCapture Temporary{ OutputPath };
	Temporary.Path += L".tmp." + std::to_wstring(GetCurrentProcessId()) +
		L"." + std::to_wstring(TemporarySequence.fetch_add(1));

	ofstream Stream(Temporary.Path, ios::binary | ios::trunc);
	if (!Stream)
	{
		SetError(pOutError, "Cannot open profiler JSON output.");
		return false;
	}

	Stream.imbue(std::locale::classic());
	Stream << fixed << setprecision(6);
	Stream << "{\n";
	Stream << "  \"schema\": \"LostArkProfilerCapture.v3\",\n";
	Stream << "  \"droppedCpuScopes\": " << Snapshot.DroppedCpuScopes << ",\n";
	Stream << "  \"droppedGpuFrames\": " << Snapshot.DroppedGpuFrames << ",\n";
	Stream << "  \"droppedGpuScopes\": " << Snapshot.DroppedGpuScopes << ",\n";
	Stream << "  \"droppedModelAnimationSamples\": " << Snapshot.DroppedModelAnimationSamples << ",\n";
	Stream << "  \"gpuQueriesSupported\": " << (Snapshot.GpuQueriesSupported ? "true" : "false") << ",\n";
	Stream << "  \"gpuScopesSupported\": " << (Snapshot.GpuScopesSupported ? "true" : "false") << ",\n";
	Stream << "  \"mainThreadId\": " << Snapshot.MainThreadId << ",\n";
	Stream << "  \"ticksPerSecond\": " << Snapshot.TicksPerSecond << ",\n";
	Stream << "  \"scopeNames\": [";
	for (size_t i = 0; i < Snapshot.ScopeNames.size(); ++i)
	{
		if (0 == i % 256 && Cancelled(pCancel, pOutError))
			return false;
		if (0 != i)
			Stream << ", ";
		Stream << "\"" << EscapeJson(Snapshot.ScopeNames[i]) << "\"";
	}
	Stream << "],\n";
	Stream << "  \"frames\": [\n";

	for (size_t iFrame = 0; iFrame < Snapshot.Frames.size(); ++iFrame)
	{
		if (Cancelled(pCancel, pOutError))
			return false;
		const Engine::FProfilerFrame& Frame = Snapshot.Frames[iFrame];
		const char* pGpuStatus = GpuStatusName(Frame.GpuStatus);
		if (nullptr == pGpuStatus || !std::isfinite(Frame.CpuFrameMs) ||
			!std::isfinite(Frame.GpuFrameMs) || !std::isfinite(Frame.FrameIntervalMs) ||
			!std::isfinite(Frame.Animation.CpuMs) || !std::isfinite(Frame.Animation.NotSubmittedCpuMs))
		{
			SetError(pOutError, "Profiler frame has invalid GPU status or non-finite timing.");
			return false;
		}
		Stream << "    {\n";
		Stream << "      \"frameNumber\": " << Frame.FrameNumber << ",\n";
		Stream << "      \"cpuFrameMs\": " << Frame.CpuFrameMs << ",\n";
		Stream << "      \"frameIntervalMs\": " << Frame.FrameIntervalMs << ",\n";
		Stream << "      \"gpuFrameMs\": " << Frame.GpuFrameMs << ",\n";
		Stream << "      \"gpuValid\": " << (Frame.GpuValid ? "true" : "false") << ",\n";
		Stream << "      \"gpuStatus\": \"" << pGpuStatus << "\",\n";
		Stream << "      \"gpuLatencyFrames\": " << Frame.GpuLatencyFrames << ",\n";
		Stream << "      \"gpuScopesSupported\": " << (Frame.GpuScopesSupported ? "true" : "false") << ",\n";
		Stream << "      \"droppedGpuScopes\": " << Frame.DroppedGpuScopes << ",\n";
		Stream << "      \"animation\": {\n";
		Stream << "        \"updateCalls\": " << Frame.Animation.UpdateCalls << ",\n";
		Stream << "        \"updatedModels\": " << Frame.Animation.UpdatedModels << ",\n";
		Stream << "        \"submittedUpdatedModels\": " << Frame.Animation.SubmittedUpdatedModels << ",\n";
		Stream << "        \"notSubmittedUpdatedModels\": " << Frame.Animation.NotSubmittedUpdatedModels << ",\n";
		Stream << "        \"droppedSamples\": " << Frame.Animation.DroppedSamples << ",\n";
		Stream << "        \"cpuMs\": " << Frame.Animation.CpuMs << ",\n";
		Stream << "        \"notSubmittedCpuMs\": " << Frame.Animation.NotSubmittedCpuMs << "\n";
		Stream << "      },\n";
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
			if (0 == iScope % 256 && Cancelled(pCancel, pOutError))
				return false;
			const Engine::FProfilerScopeSample& Scope = Frame.CpuScopes[iScope];
			Stream << "        {\"nameId\": " << Scope.NameId
				<< ", \"depth\": " << Scope.Depth
				<< ", \"threadId\": " << Scope.ThreadId
				<< ", \"beginTick\": " << Scope.BeginTick
				<< ", \"endTick\": " << Scope.EndTick << "}"
				<< (iScope + 1 < Frame.CpuScopes.size() ? "," : "")
				<< "\n";
		}
		Stream << "      ],\n";
		Stream << "      \"gpuScopes\": [\n";
		for (size_t iScope = 0; iScope < Frame.GpuScopes.size(); ++iScope)
		{
			if (0 == iScope % 256 && Cancelled(pCancel, pOutError))
				return false;
			const Engine::FProfilerGpuScopeSample& Scope = Frame.GpuScopes[iScope];
			if (!std::isfinite(Scope.BeginMs) || !std::isfinite(Scope.EndMs) ||
				!std::isfinite(Scope.DurationMs))
			{
				SetError(pOutError, "Profiler GPU scope has non-finite timing.");
				return false;
			}
			Stream << "        {\"nameId\": " << Scope.NameId
				<< ", \"depth\": " << Scope.Depth
				<< ", \"beginMs\": " << Scope.BeginMs
				<< ", \"endMs\": " << Scope.EndMs
				<< ", \"durationMs\": " << Scope.DurationMs
				<< ", \"pipelineValid\": " << (Scope.PipelineValid ? "true" : "false")
				<< ", \"psInvocations\": " << Scope.PSInvocations
				<< ", \"vsInvocations\": " << Scope.VSInvocations << "}"
				<< (iScope + 1 < Frame.GpuScopes.size() ? "," : "") << "\n";
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
		SetError(pOutError, "Failed while writing profiler JSON.");
		return false;
	}

	if (Cancelled(pCancel, pOutError))
		return false;
	// Both paths are in the same directory. Commit only the complete file;
	// never delete an existing capture before the replacement succeeds.
	if (!MoveFileExW(Temporary.Path.c_str(), OutputPath.c_str(),
		(ReplaceExisting ? MOVEFILE_REPLACE_EXISTING : 0u) | MOVEFILE_WRITE_THROUGH))
	{
		SetError(pOutError, "Cannot finalize profiler JSON output: " +
			std::error_code(static_cast<int>(GetLastError()), std::system_category()).message());
		return false;
	}

	if (nullptr != pOutError)
		pOutError->clear();
	return true;
}

	bool SaveJsonSafely(const Engine::FProfilerCaptureSnapshot& Snapshot,
		const filesystem::path& OutputPath, string* pOutError,
		const std::atomic_bool* pCancel = nullptr, bool ReplaceExisting = true)
	{
		try
		{
			return SaveJsonImpl(Snapshot, OutputPath, pOutError, pCancel, ReplaceExisting);
		}
		catch (const std::exception& Exception)
		{
			SetError(pOutError, "Profiler capture save failed: " + string(Exception.what()));
			return false;
		}
	}
}

bool_t Client::CProfilerCaptureIO::Save_Json(
	const Engine::FProfilerCaptureSnapshot& Snapshot,
	const filesystem::path& OutputPath, string* pOutError)
{
	return SaveJsonSafely(Snapshot, OutputPath, pOutError);
}

struct Client::CProfilerCaptureExporter::FSaveJob final
{
	FProfilerCaptureSaveResult Result;
	std::atomic_bool Cancel = false;
};

Client::CProfilerCaptureExporter::~CProfilerCaptureExporter()
{
	if (!m_Worker.joinable())
		return;
	m_Job->Cancel.store(true, std::memory_order_relaxed);
	const HANDLE WorkerHandle = m_Worker.native_handle();
	CancelSynchronousIo(WorkerHandle);
	const DWORD WaitResult = WaitForSingleObject(WorkerHandle, 5000);
	if (WAIT_OBJECT_0 != WaitResult)
	{
		// Matches the Loader shutdown policy: never kill a worker and keep
		// running with partially owned state, or wait forever during exit.
		const DWORD ExitCode = WAIT_TIMEOUT == WaitResult ? ERROR_TIMEOUT : GetLastError();
		OutputDebugStringA("[Profiler] Capture writer exceeded shutdown deadline or wait failed.\n");
		if (!TerminateProcess(GetCurrentProcess(),
			ERROR_SUCCESS == ExitCode ? ERROR_INVALID_HANDLE : ExitCode))
			std::terminate();
		__assume(0);
	}
	m_Worker.join();
}

bool_t Client::CProfilerCaptureExporter::BeginSave(
	Engine::FProfilerCaptureSnapshot&& Snapshot,
	filesystem::path OutputPath, string* pOutError)
{
	if (IsSaving())
	{
		SetError(pOutError, "A profiler capture save is already in progress.");
		return false;
	}
	if (OutputPath.empty())
	{
		SetError(pOutError, "Profiler output path is empty.");
		return false;
	}
	try
	{
		auto Job = std::make_shared<FSaveJob>();
		Job->Result.OutputPath = std::move(OutputPath);
		// Capturing the snapshot in the worker also releases its large vectors
		// on that thread, rather than stalling the next main-thread Poll.
		m_Worker = std::thread([Job, Captured = std::move(Snapshot)]()
		{
			Job->Result.Succeeded = SaveJsonSafely(Captured,
				Job->Result.OutputPath, &Job->Result.Error, &Job->Cancel, false);
			OutputDebugStringA(Job->Result.Succeeded ?
				"[Profiler] JSON capture save completed.\n" :
				"[Profiler] JSON capture save failed or was cancelled.\n");
			if (!Job->Result.Succeeded)
				OutputDebugStringA(Job->Result.Error.c_str());
		});
		m_Job = std::move(Job);
	}
	catch (const std::exception& Exception)
	{
		SetError(pOutError, "Cannot start profiler capture writer: " + string(Exception.what()));
		return false;
	}
	if (nullptr != pOutError)
		pOutError->clear();
	return true;
}

bool_t Client::CProfilerCaptureExporter::IsSaving() const noexcept
{
	// Includes a finished save until Poll consumes its result.
	return m_Worker.joinable();
}

bool_t Client::CProfilerCaptureExporter::Poll(FProfilerCaptureSaveResult& OutResult)
{
	if (!m_Worker.joinable())
		return false;
	const DWORD WaitResult = WaitForSingleObject(m_Worker.native_handle(), 0);
	if (WAIT_TIMEOUT == WaitResult)
		return false;
	if (WAIT_OBJECT_0 != WaitResult)
	{
		const DWORD ExitCode = GetLastError();
		OutputDebugStringA("[Profiler] Capture writer completion wait failed.\n");
		if (!TerminateProcess(GetCurrentProcess(),
			ERROR_SUCCESS == ExitCode ? ERROR_INVALID_HANDLE : ExitCode))
			std::terminate();
		__assume(0);
	}
	m_Worker.join();
	OutResult = std::move(m_Job->Result);
	m_Job.reset();
	return true;
}

filesystem::path Client::CProfilerCaptureIO::Get_CaptureDirectory()
{
	wchar_t ModulePath[32768]{};
	const DWORD Length = GetModuleFileNameW(nullptr, ModulePath, static_cast<DWORD>(size(ModulePath)));
	const filesystem::path Base = Length && Length < size(ModulePath) ?
		filesystem::path(ModulePath).parent_path() : filesystem::current_path();
	return (Base / L".." / L"ProfilerCaptures").lexically_normal();
}

namespace
{
	struct FCaptureHandle final
	{
		HANDLE Value = INVALID_HANDLE_VALUE;
		~FCaptureHandle() { if (Value != INVALID_HANDLE_VALUE) CloseHandle(Value); }
	};

	uint64_t CaptureTicks(const FILETIME& Time)
	{ return (uint64_t(Time.dwHighDateTime) << 32u) | Time.dwLowDateTime; }

	string Utf8Path(const filesystem::path& Path)
	{
		const auto Text = Path.u8string();
		return string(Text.begin(), Text.end());
	}

	bool CaptureError(string* Error, const char* Operation)
	{
		SetError(Error, string(Operation) + ": " +
			std::error_code(static_cast<int>(GetLastError()), std::system_category()).message());
		return false;
	}

	bool CaptureFinalPath(HANDLE Handle, filesystem::path& Path, string* Error)
	{
		wchar_t Buffer[32768]{};
		const DWORD Length = GetFinalPathNameByHandleW(Handle, Buffer, static_cast<DWORD>(size(Buffer)), FILE_NAME_NORMALIZED);
		if (!Length || Length >= size(Buffer)) return CaptureError(Error, "Cannot resolve capture path");
		Path = filesystem::path(Buffer).lexically_normal();
		return true;
	}

	bool OpenCaptureDirectory(const filesystem::path& Directory, FCaptureHandle& Handle,
		filesystem::path& FinalPath, string* Error)
	{
		// Deny renaming this directory until the list/delete operation completes.
		Handle.Value = CreateFileW(Directory.c_str(), FILE_READ_ATTRIBUTES,
			FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
		if (Handle.Value == INVALID_HANDLE_VALUE) return CaptureError(Error, "Cannot open capture directory");
		BY_HANDLE_FILE_INFORMATION Info{};
		if (!GetFileInformationByHandle(Handle.Value, &Info)) return CaptureError(Error, "Cannot inspect capture directory");
		if (!(Info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || (Info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
		{ SetError(Error, "Capture directory must be a regular directory, not a link."); return false; }
		return CaptureFinalPath(Handle.Value, FinalPath, Error);
	}

	bool IsCaptureFileName(const filesystem::path& Name)
	{
		return !Name.empty() && !Name.has_root_path() && Name == Name.filename() &&
			Name.native().find_first_of(L"/\\:") == std::wstring::npos &&
			_wcsicmp(Name.extension().c_str(), L".json") == 0;
	}

	bool ReadCaptureFile(const filesystem::path& Directory, const filesystem::path& FinalDirectory,
		const filesystem::path& Name, bool ForDelete, FCaptureHandle& Handle,
		Client::FProfilerCaptureFile& Out, string* Error)
	{
		if (!IsCaptureFileName(Name)) { SetError(Error, "Select a JSON file inside the capture directory."); return false; }
		Handle.Value = CreateFileW((Directory / Name).c_str(), FILE_READ_ATTRIBUTES | (ForDelete ? DELETE : 0u),
			ForDelete ? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
		if (Handle.Value == INVALID_HANDLE_VALUE) return CaptureError(Error, "Cannot open selected capture");
		BY_HANDLE_FILE_INFORMATION Info{};
		if (!GetFileInformationByHandle(Handle.Value, &Info)) return CaptureError(Error, "Cannot inspect selected capture");
		filesystem::path FinalFile;
		if (!CaptureFinalPath(Handle.Value, FinalFile, Error)) return false;
		if ((Info.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
			_wcsicmp(FinalFile.parent_path().c_str(), FinalDirectory.c_str()) != 0)
		{ SetError(Error, "Selected capture is not a regular file inside this directory."); return false; }
		Out.FileName = Name;
		Out.DisplayName = Utf8Path(Name);
		Out.VolumeSerial = Info.dwVolumeSerialNumber;
		Out.FileIndex = (uint64_t(Info.nFileIndexHigh) << 32u) | Info.nFileIndexLow;
		Out.SizeBytes = (uint64_t(Info.nFileSizeHigh) << 32u) | Info.nFileSizeLow;
		Out.LastWriteTicks = CaptureTicks(Info.ftLastWriteTime);
		Out.StableId = std::to_string(Out.VolumeSerial) + ":" + std::to_string(Out.FileIndex) + ":" + Out.DisplayName;
		return true;
	}

	bool CaptureName(std::string_view Name, std::wstring& Wide, string* Error)
	{
		if (Name.empty()) { Wide = L"profiler"; return true; }
		if (Name.size() > 240u) { SetError(Error, "Capture name is too long (maximum 80 UTF-16 characters). "); return false; }
		const int Count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, Name.data(), static_cast<int>(Name.size()), nullptr, 0);
		if (Count <= 0 || Count > 80) { SetError(Error, "Capture name must be valid UTF-8 and at most 80 UTF-16 characters."); return false; }
		Wide.resize(static_cast<size_t>(Count));
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, Name.data(), static_cast<int>(Name.size()), Wide.data(), Count);
		if (Wide == L"." || Wide == L".." || Wide.back() == L'.' || Wide.back() == L' ' ||
			std::any_of(Wide.begin(), Wide.end(), [](wchar_t C) { return C < 32 || C == 127 || std::wstring_view(L"<>:\"/\\|?*").find(C) != std::wstring_view::npos; }))
		{ SetError(Error, "Capture name must be a file label without path separators, reserved characters, or trailing dots/spaces."); return false; }
		std::wstring Stem = Wide.substr(0, Wide.find(L'.'));
		while (!Stem.empty() && (Stem.back() == L' ' || Stem.back() == L'.')) Stem.pop_back();
		std::transform(Stem.begin(), Stem.end(), Stem.begin(), [](wchar_t C) { return static_cast<wchar_t>(std::towupper(C)); });
		const bool DeviceNumber = Stem.size() == 4u && (Stem.starts_with(L"COM") || Stem.starts_with(L"LPT")) &&
			((Stem[3] >= L'1' && Stem[3] <= L'9') || Stem[3] == L'\u00b9' || Stem[3] == L'\u00b2' || Stem[3] == L'\u00b3');
		if (Stem == L"CON" || Stem == L"PRN" || Stem == L"AUX" || Stem == L"NUL" || Stem == L"CLOCK$" ||
			Stem == L"CONIN$" || Stem == L"CONOUT$" || DeviceNumber)
		{ SetError(Error, "Capture name is reserved by Windows. Choose another name."); return false; }
		return true;
	}
}

bool_t Client::CProfilerCaptureIO::Validate_Name(std::string_view Name, string* Error)
{
	try
	{
		std::wstring Wide;
		if (!CaptureName(Name, Wide, Error)) return false;
		if (Error) Error->clear();
		return true;
	}
	catch (const std::exception& Exception) { SetError(Error, string("Invalid capture name: ") + Exception.what()); return false; }
}

bool_t Client::CProfilerCaptureIO::Make_NamedPath(std::string_view Name, uint64_t Frame,
	filesystem::path& OutPath, string* Error)
{
	try
	{
		std::wstring Label;
		if (!CaptureName(Name, Label, Error)) return false;
		const auto Now = chrono::system_clock::now();
		const time_t CalendarTime = chrono::system_clock::to_time_t(Now);
		tm LocalTime{}; localtime_s(&LocalTime, &CalendarTime);
		const auto Milliseconds = chrono::duration_cast<chrono::milliseconds>(Now.time_since_epoch()).count() % 1000;
		static std::atomic_uint64_t Sequence = 0;
		wostringstream FileName;
		FileName << Label << L"_" << put_time(&LocalTime, L"%Y%m%d_%H%M%S")
			<< L"_" << setw(3) << setfill(L'0') << Milliseconds << L"_frame" << Frame
			<< L"_" << GetCurrentProcessId() << L"_" << Sequence.fetch_add(1) << L".json";
		OutPath = Get_CaptureDirectory() / FileName.str();
		if (Error) Error->clear();
		return true;
	}
	catch (const std::exception& Exception) { SetError(Error, string("Cannot name capture: ") + Exception.what()); return false; }
}

filesystem::path Client::CProfilerCaptureIO::Make_DefaultPath(uint64_t Frame)
{
	filesystem::path Result;
	Make_NamedPath({}, Frame, Result);
	return Result;
}

bool_t Client::CProfilerCaptureIO::List_JsonFiles(const filesystem::path& Directory,
	std::vector<FProfilerCaptureFile>& OutFiles, string* Error)
{
	try
	{
		std::error_code Code;
		if (!filesystem::exists(Directory, Code) && !Code)
		{ OutFiles.clear(); if (Error) Error->clear(); return true; }
		FCaptureHandle Root; filesystem::path FinalDirectory;
		if (!OpenCaptureDirectory(Directory, Root, FinalDirectory, Error)) return false;
		std::vector<FProfilerCaptureFile> Staged;
		for (const auto& Entry : filesystem::directory_iterator(Directory))
		{
			if (!IsCaptureFileName(Entry.path().filename())) continue;
			const DWORD Attributes = GetFileAttributesW(Entry.path().c_str());
			if (Attributes != INVALID_FILE_ATTRIBUTES && (Attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))) continue;
			FCaptureHandle File; FProfilerCaptureFile Row;
			if (!ReadCaptureFile(Directory, FinalDirectory, Entry.path().filename(), false, File, Row, Error)) return false;
			Staged.push_back(std::move(Row));
		}
		std::sort(Staged.begin(), Staged.end(), [](const auto& A, const auto& B)
		{ return A.LastWriteTicks != B.LastWriteTicks ? A.LastWriteTicks > B.LastWriteTicks : A.FileName < B.FileName; });
		OutFiles = std::move(Staged);
		if (Error) Error->clear();
		return true;
	}
	catch (const std::exception& Exception) { SetError(Error, string("Cannot list captures: ") + Exception.what()); return false; }
}

bool_t Client::CProfilerCaptureIO::Delete_JsonFile(const filesystem::path& Directory,
	const FProfilerCaptureFile& Selected, string* Error)
{
	try
	{
		FCaptureHandle Root; filesystem::path FinalDirectory;
		if (!OpenCaptureDirectory(Directory, Root, FinalDirectory, Error)) return false;
		FCaptureHandle File; FProfilerCaptureFile Current;
		if (!ReadCaptureFile(Directory, FinalDirectory, Selected.FileName, true, File, Current, Error)) return false;
		if (Current.StableId != Selected.StableId || Current.SizeBytes != Selected.SizeBytes || Current.LastWriteTicks != Selected.LastWriteTicks)
		{ SetError(Error, "Selected capture changed since the last refresh. Refresh and select it again."); return false; }
		FILE_DISPOSITION_INFO Disposition{ TRUE };
		if (!SetFileInformationByHandle(File.Value, FileDispositionInfo, &Disposition, sizeof(Disposition)))
			return CaptureError(Error, "Cannot delete selected capture");
		if (Error) Error->clear();
		return true;
	}
	catch (const std::exception& Exception) { SetError(Error, string("Cannot delete capture: ") + Exception.what()); return false; }
}
