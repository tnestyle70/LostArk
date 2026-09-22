#pragma once

#include "Client_Defines.h"
#include "Profiler.h"

#include <array>
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

// Context is sampled at export on the main thread; it does not assert that every
// historical frame used the same camera, level, render settings or window size.
struct FProfilerCaptureContext final
{
    bool Valid = false;
    uint32_t LevelId = 0;
    std::array<float, 2> Viewport{};
    std::array<float, 4> CameraPosition{};
    std::array<float, 16> ViewMatrix{}, ProjectionMatrix{};
    bool ShadowEnabled = false, SSAOEnabled = false, BloomEnabled = false, FXAAEnabled = false;
    float ShadowWidth = 0.f, ShadowHeight = 0.f, ShadowStrength = 0.f;
    std::string Adapter;
    uint32_t DeviceCreationFlags = 0;
};

class CProfilerCaptureIO final
{
public:
	static bool_t Save_Json(
		const Engine::FProfilerCaptureSnapshot& Snapshot,
		const filesystem::path& OutputPath,
		string* pOutError = nullptr,
        const FProfilerCaptureContext& Context = {});

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
		filesystem::path OutputPath, string* pOutError = nullptr,
        FProfilerCaptureContext Context = {});
	[[nodiscard]] bool_t IsSaving() const noexcept;
	bool_t Poll(FProfilerCaptureSaveResult& OutResult);

private:
	struct FSaveJob;
	std::shared_ptr<FSaveJob> m_Job;
	std::thread m_Worker;
};

NS_END
