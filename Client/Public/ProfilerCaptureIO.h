#pragma once

#include "Client_Defines.h"
#include "Profiler.h"

#include <filesystem>
#include <memory>
#include <thread>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

struct FProfilerCaptureFile final
{
	filesystem::path FileName;
	string DisplayName;
	string StableId;
	uint64_t SizeBytes = 0u;
	uint64_t LastWriteTicks = 0u;
	uint64_t FileIndex = 0u;
	uint32_t VolumeSerial = 0u;
};

class CProfilerCaptureIO final
{
public:
	static bool_t Save_Json(
		const Engine::FProfilerCaptureSnapshot& Snapshot,
		const filesystem::path& OutputPath,
		string* pOutError = nullptr);

	static filesystem::path Get_CaptureDirectory();
	static filesystem::path Make_DefaultPath(uint64_t iFrameNumber);
	static bool_t Validate_Name(std::string_view Name, string* pOutError = nullptr);
	static bool_t Make_NamedPath(std::string_view Name, uint64_t iFrameNumber,
		filesystem::path& OutPath, string* pOutError = nullptr);
	// Only immediate regular JSON children are listed/deleted. The selected
	// file identity and last observed content metadata must still match.
	static bool_t List_JsonFiles(const filesystem::path& Directory,
		std::vector<FProfilerCaptureFile>& OutFiles, string* pOutError = nullptr);
	static bool_t Delete_JsonFile(const filesystem::path& Directory,
		const FProfilerCaptureFile& Selected, string* pOutError = nullptr);
};

struct FProfilerCaptureSaveResult final
{
	bool_t Succeeded = false;
	filesystem::path OutputPath;
	string Error;
};

/* One immutable snapshot per save. The Tool owns this exporter across window
   close/open; Poll reports each completion once and never waits for file I/O. */
class CProfilerCaptureExporter final
{
public:
	CProfilerCaptureExporter() = default;
	~CProfilerCaptureExporter();
	CProfilerCaptureExporter(const CProfilerCaptureExporter&) = delete;
	CProfilerCaptureExporter& operator=(const CProfilerCaptureExporter&) = delete;

	bool_t BeginSave(Engine::FProfilerCaptureSnapshot&& Snapshot,
		filesystem::path OutputPath, string* pOutError = nullptr);
	[[nodiscard]] bool_t IsSaving() const noexcept;
	bool_t Poll(FProfilerCaptureSaveResult& OutResult);

private:
	struct FSaveJob;
	std::shared_ptr<FSaveJob> m_Job;
	std::thread m_Worker;
};

NS_END
