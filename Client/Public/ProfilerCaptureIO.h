#pragma once

#include "Client_Defines.h"
#include "Profiler.h"

#include <array>
#include <filesystem>
#include <memory>
#include <thread>
#include <string_view>
#include <vector>
#include <map>

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

// Observational main-thread values only. Gameplay and prediction retain ownership.
enum class EProfilerMovementKind : uint8_t { Frame, Snapshot, Command };
enum class EProfilerMovementFlags : uint32_t
{
    None = 0, PendingLocalPath = 1, PredictionActive = 2,
    Accepted = 4, Moving = 8, LocalPathCompleted = 16
};

struct FProfilerMovementSample final
{
    EProfilerMovementKind Kind = EProfilerMovementKind::Frame;
    uint64_t QpcTick = 0; // Set by Record_MovementSample, in the existing profiler clock.
    uint64_t FrameNumber = 0; // Assigned from completed-frame scope bounds at export.
    uint32_t CharacterClass = 0, ServerTick = 0, Sequence = 0;
    uint32_t Flags = 0, Disposition = 0;
    float DeltaSeconds = 0.f, MotionSeconds = 0.f;
    std::array<float, 3> Before{}, After{}, Authority{}, Waypoint{};
};

struct FProfilerMovementCoverage final
{
    bool Captured = false;
    bool WindowMayBeTruncated = false;
    uint64_t WindowBeginTick = 0, WindowEndTick = 0;
    uint64_t FirstRetainedTick = 0, LastRetainedTick = 0;
    uint64_t AcceptedSinceReset = 0, OverwrittenSinceReset = 0, RejectedSinceReset = 0;
    size_t FramesWithBounds = 0, FramesWithoutBounds = 0, OutsideSavedFrames = 0;
};

// Context is sampled at export on the main thread; it does not assert that every
// historical frame used the same camera, level, render settings, focus or window size.
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
    bool ClientWindowForeground = false, ProcessForeground = false, WindowMinimized = false;
    // Same checkbox-resolved limits as CMainApp::Limit_FrameRate; 0 means disabled.
    // Effective selects foreground/background by process ownership. It is not measured FPS
    // and excludes the separate minimized-frame message wait.
    int32_t ForegroundFpsLimit = 0, BackgroundFpsLimit = 0, EffectiveFpsLimit = 0;
    // Effective renderer values sampled at the same instant as camera/context.
    // String entries identify assets; neither map claims per-frame provenance.
    std::map<std::string, double> RenderingOptions;
    std::map<std::string, std::string> RenderingAssets;
    // Optional owned values copied before launching the existing save worker.
    FProfilerMovementCoverage MovementCoverage;
    std::vector<FProfilerMovementSample> MovementSamples;
};

// Comparison values are observations. Missing fields never become measured zero.
struct FProfilerComparisonValue final
{
    double Value = 0.0;
    bool Available = false;
};
struct FProfilerComparisonMeshDraw final
{
    std::string Pass, Mesh;
    uint32_t VertexCount = 0, IndexCount = 0, Instances = 0, MaterialSlot = 0;
};
struct FProfilerComparisonFrame final
{
    uint64_t Number = 0;
    bool MemoryKnown = false;
    Engine::FProfilerMemoryStats Memory{};
    bool CpuScopesKnown = false, CpuSelfKnown = false, DetailKnown = false, Detailed = false;
    bool GpuComplete = false;
    bool GpuValid = false;
    Engine::EProfilerGpuFrameStatus GpuStatus = Engine::EProfilerGpuFrameStatus::Unsupported;
    std::map<std::string, FProfilerComparisonValue> Values;
    bool MeshDrawsKnown = false;
    uint64_t DroppedMeshDraws = 0;
    std::vector<FProfilerComparisonMeshDraw> MeshDraws;
};
struct FProfilerComparisonCapture final
{
    std::string Label, Source;
    std::map<std::string, std::string> Conditions;
    std::vector<FProfilerComparisonFrame> Frames;
};
struct FProfilerComparisonMean final
{
    double Value = 0.0;
    size_t Samples = 0, Expected = 0;
    bool Available = false;
};

class CProfilerCaptureIO final
{
public:
    static constexpr size_t MAX_MOVEMENT_SAMPLES = 8192;
    static constexpr size_t MAX_COMPARISON_BYTES = 32u * 1024u * 1024u;
    static const char* Counter_Name(size_t Index);
    static FProfilerComparisonCapture Build_Comparison(
        const Engine::FProfilerCaptureSnapshot& Snapshot, const FProfilerCaptureContext& Context,
        std::string Label);
    // Bounded parser and file reader stage a complete replacement; failure preserves Out.
    static bool_t Parse_Comparison(std::string_view Bytes, FProfilerComparisonCapture& Out,
        std::string* Error = nullptr);
    static bool_t Load_Comparison(const filesystem::path& Directory,
        const FProfilerCaptureFile& Selected, FProfilerComparisonCapture& Out,
        std::string* Error = nullptr);
    static std::map<std::string, FProfilerComparisonMean> Compare_Means(
        const FProfilerComparisonCapture& Capture);
    // Main-thread only. Disabled capture returns without QPC, allocation or I/O.
    static void Record_MovementSample(const Engine::CProfiler* Profiler,
        FProfilerMovementSample Sample) noexcept;
    static void Reset_MovementSamples() noexcept;
    static void Copy_MovementSamples(const Engine::FProfilerCaptureSnapshot& Snapshot,
        FProfilerCaptureContext& Context);

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
