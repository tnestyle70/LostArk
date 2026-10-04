#include "ProfilerCaptureIO.h"
#include "DataJson.h"

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
#include <set>
#include <stdexcept>
#include <tuple>

namespace
{
    struct FMovementRing final
    {
        std::array<Client::FProfilerMovementSample, Client::CProfilerCaptureIO::MAX_MOVEMENT_SAMPLES> Samples{};
        size_t Next = 0, Count = 0;
        uint64_t Accepted = 0, Overwritten = 0, Rejected = 0, LastOverwrittenTick = 0;
    };
    FMovementRing MovementRing; // Only the Client main thread reads or writes this ring.

    const char* MovementKindName(Client::EProfilerMovementKind kind)
    {
        switch (kind)
        {
        case Client::EProfilerMovementKind::Frame: return "frame";
        case Client::EProfilerMovementKind::Snapshot: return "snapshot";
        case Client::EProfilerMovementKind::Command: return "command";
        }
        return nullptr;
    }

    bool ValidMovementSample(const Client::FProfilerMovementSample& sample)
    {
        const auto finite = [](const auto& values)
        { return std::all_of(values.begin(), values.end(), [](float value) { return std::isfinite(value); }); };
        return MovementKindName(sample.Kind) != nullptr && std::isfinite(sample.DeltaSeconds) &&
            sample.DeltaSeconds >= 0.f && std::isfinite(sample.MotionSeconds) && sample.MotionSeconds >= 0.f && finite(sample.Before) && finite(sample.After) &&
            finite(sample.Authority) && finite(sample.Waypoint);
    }

	constexpr std::array CounterNames = {
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
            "shadowCacheHits", "shadowCacheMisses", "shadowStaticCasters", "shadowDynamicCasters",
            "mapCullingCandidates", "mapCullingVisible", "mapLod0Draws", "mapLod1Draws", "mapLod2Draws",
            "mapLodSourceIndices", "mapLodSubmittedIndices",
            "lightRecords", "lightDrawCalls", "lightUploadBytes",
            "mapLodAvailableDraws",
            "lightCullingCandidates", "lightCullingRejected",
            "effectBoundsCandidates", "effectBoundsCulled",
            "effectMarkerSamples", "effectMarkerHistoryRequests",
            "effectAmbientSuspended", "effectAmbientAdvanced",
            "imguiPresentAttempts", "imguiPresentBusy", "imguiPresentFailures", "imguiPresentOccluded",
            "mapBatchVisibilityCacheHits", "mapBatchVisibilityRebuilds",
            "mapBatchEmptyRenders", "mapBatchVisibleRenders", "mapBatchBoundsRejected",
            "mapBatchUploadBytes", "npcAuthoredHiddenUpdates", "ambientUnboundedUpdates",
            "npcCullingCandidates", "npcCulled", "npcDeferredPoseEvaluations",
            "meshDrawCalls", "meshInstances", "meshIndices", "uniqueMeshes", "droppedMeshSamples",
            "effectAmbientClampedUpdates", "effectAmbientDiscardedMicroseconds",
            "effectAmbientFixedSteps", "effectAmbientMaxFixedSteps",
	};
    static_assert(CounterNames.size() == static_cast<size_t>(Engine::EProfilerCounter::Count),
        "Every profiler counter must have exactly one JSON key.");

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

    void WriteDistribution(std::ostream& stream, std::vector<double> values)
    {
        std::sort(values.begin(), values.end());
        if (values.empty())
        {
            stream << "{\"samples\": 0, \"meanMs\": null, \"p50Ms\": null, \"p95Ms\": null, \"p99Ms\": null, \"maxMs\": null}";
            return;
        }
        double total = 0.0;
        for (double value : values) total += value;
        const auto percentile = [&](double fraction)
        {
            const size_t rank = static_cast<size_t>(std::ceil(fraction * values.size()));
            return values[(std::max)(size_t{1}, rank) - 1];
        };
        stream << "{\"samples\": " << values.size() << ", \"meanMs\": " << total / values.size()
            << ", \"p50Ms\": " << percentile(.50) << ", \"p95Ms\": " << percentile(.95)
            << ", \"p99Ms\": " << percentile(.99) << ", \"maxMs\": " << values.back() << "}";
    }

    void WriteCaptureWindow(std::ostream& stream, const Engine::FProfilerCaptureSnapshot& snapshot)
    {
        const auto& window = snapshot.CaptureWindow;
        stream << "  \"captureWindow\": {\n    \"requestedFrames\": " << window.RequestedFrames
            << ",\n    \"retainedFrames\": " << window.RetainedFrames
            << ",\n    \"savedFrames\": " << snapshot.Frames.size()
            << ",\n    \"recordedFramesSinceReset\": " << window.RetainedFrames + window.EvictedFramesSinceReset
            << ",\n    \"excludedRetainedFrames\": " << window.ExcludedRetainedFrames
            << ",\n    \"evictedFramesSinceReset\": " << window.EvictedFramesSinceReset
            << ",\n    \"firstRetainedFrameNumber\": " << window.FirstRetainedFrameNumber
            << ",\n    \"lastRetainedFrameNumber\": " << window.LastRetainedFrameNumber
            << ",\n    \"firstSavedFrameNumber\": " << window.FirstSavedFrameNumber
            << ",\n    \"lastSavedFrameNumber\": " << window.LastSavedFrameNumber
            << ",\n    \"excludedMaxFrameIntervalMs\": " << window.ExcludedMaxFrameIntervalMs
            << ",\n    \"note\": \"Newest completed frames only. Excluded retained frames can still be saved with a larger window; evicted frames cannot. Frame N interval runs from Begin(N-1) to Begin(N), while frame N CPU scopes measure work after Begin(N).\"\n  },\n";
    }

    void WriteSummary(std::ostream& stream, const Engine::FProfilerCaptureSnapshot& snapshot)
    {
        std::vector<double> intervals, cpu, gpu;
        size_t cpuPartial = 0, gpuPartial = 0, gpuPending = 0, over60 = 0, over30 = 0, over100 = 0;
        uint64_t omittedCpuScopes = 0;
        for (const auto& frame : snapshot.Frames)
        {
            if (std::isfinite(frame.FrameIntervalMs) && frame.FrameIntervalMs > 0.0)
            {
                intervals.push_back(frame.FrameIntervalMs);
                over60 += frame.FrameIntervalMs > 1000.0 / 60.0;
                over30 += frame.FrameIntervalMs > 1000.0 / 30.0;
                over100 += frame.FrameIntervalMs > 100.0;
            }
            if (std::isfinite(frame.CpuFrameMs) && frame.CpuFrameMs >= 0.0) cpu.push_back(frame.CpuFrameMs);
            omittedCpuScopes += frame.DroppedCpuScopes;
            cpuPartial += frame.DroppedCpuScopes != 0;
            gpuPending += frame.GpuStatus == Engine::EProfilerGpuFrameStatus::Pending;
            gpuPartial += frame.DroppedGpuScopes != 0;
            if (frame.GpuValid && frame.GpuStatus == Engine::EProfilerGpuFrameStatus::Valid &&
                std::isfinite(frame.GpuFrameMs) && frame.GpuFrameMs >= 0.0) gpu.push_back(frame.GpuFrameMs);
        }
        stream << "  \"summary\": {\n    \"percentileMethod\": \"nearest-rank\",\n"
            << "    \"frameCount\": " << snapshot.Frames.size() << ",\n"
            << "    \"frameInterval\": "; WriteDistribution(stream, std::move(intervals));
        stream << ",\n    \"cpuFrame\": "; WriteDistribution(stream, std::move(cpu));
        stream << ",\n    \"gpuFrameValidOnly\": "; WriteDistribution(stream, std::move(gpu));
        stream << ",\n    \"framesOver16_667Ms\": " << over60 << ",\n    \"framesOver33_333Ms\": " << over30
            << ",\n    \"framesOver100Ms\": " << over100 << ",\n    \"cpuPartialFrames\": " << cpuPartial
            << ",\n    \"windowDroppedCpuScopes\": " << omittedCpuScopes
            << ",\n    \"gpuPendingFrames\": " << gpuPending << ",\n    \"gpuPartialScopeFrames\": " << gpuPartial
            << ",\n    \"cpuScopesWithinBudget\": " << (cpuPartial == 0 ? "true" : "false")
            << ",\n    \"note\": \"Frame durations include all frames; scope attribution is incomplete in frames with drops. GPU timestamps measure elapsed intervals, not utilization. Reset-scoped drop totals can include history outside this exported window.\"\n  },\n";
    }

    void WriteMeasurementSemantics(std::ostream& stream)
    {
        stream << "  \"measurementSemantics\": {\n"
            << "    \"drawCoverage\": \"CPU-issued draw submissions through instrumented Engine VIBuffer and CMesh paths; excludes DirectXTK internal draws and separately counted ImGui draws. Submission does not prove final pixel visibility.\",\n"
            << "    \"ambientCatchup\": \"Only admitted offscreen-pausable level-owned ambient loops use a 0.1s visual-delta limit after playback rate. effectAmbientClampedUpdates counts limited visible advances; effectAmbientDiscardedMicroseconds sums omitted visual time over effects, not frame wall time or saved CPU time. effectAmbientFixedSteps and effectAmbientMaxFixedSteps count committed 1/60 steps from integer simulation-step deltas, excluding initial Seek and hidden updates. They remain available when raw CPU scopes drop. Invalid input contributes no omitted-time sample.\",\n"
            << "    \"instances\": \"Instances counts instanced Engine submissions only; meshInstances also counts one instance for a non-instanced CMesh draw.\",\n"
            << "    \"indices\": \"Indices includes instance multiplication. indirectDrawCalls and indirectIndexUpperBound are reserved fields with no current producer, so zero is unmeasured. An upper bound must never be treated as exact submitted indices.\",\n"
            << "    \"uniqueMeshes\": \"Distinct CMesh objects submitted in one frame, including shared geometry counted once; not asset IDs, scene objects or visible meshes. Internal addresses are not exported. droppedMeshSamples counts omitted distinct-key attempts; uniqueMeshes is then a lower bound.\",\n"
            << "    \"uniqueMeshCapacity\": " << Engine::CProfiler::MAX_UNIQUE_MESHES_PER_FRAME << ",\n"
            << "    \"meshDraws\": \"Optional detailed CMesh submission trace in issue order, capped at 512 samples per frame. Mesh and pass names are display labels, not stable asset IDs. IndexCount is per instance. Per-draw GPU time is not measured. UINT32_MAX passNameId means no observed enclosing GPU scope.\",\n"
            << "    \"cpuAndGpu\": \"CPU and GPU overlap and must not be added. GPU timestamp intervals may include waits for CPU submission; they are not GPU utilization or shader ALU counts.\",\n"
            << "    \"frameInterval\": \"Previous Begin to current Begin equals previousCpuFrameMs plus frameGapMs, except the first captured frame. frameGapMs spans previous measured CPU End to current Begin and includes loop waits, message processing and profiler bookkeeping. Current cpuFrameMs belongs to the current frame and must not be substituted for previousCpuFrameMs.\",\n"
            << "    \"gpuScopeSelf\": \"selfMs subtracts direct child timestamp intervals from durationMs. Missing scopes make attribution incomplete; frames with droppedGpuScopes are excluded from complete GPU pass aggregates.\",\n"
            << "    \"gpuScopeDraw\": \"Per-scope draw counters are inclusive begin/end submission deltas retained with the original submitted frame, even when GPU results arrive later. They include child scopes and do not correspond only to selfMs.\",\n"
            << "    \"memorySampling\": \"Main-thread OS observations attempted at most once per second while Capture is ON. Frame ageMs is relative to frame End; LiveStats ageMs is relative to now. Reused frames are not independent memory samples; comparison averages deduplicate process ID, sample tick and sample frame. Stalls and between-poll peaks may be missed; this does not replace a 100-250ms loading phase sampler. Each validity flag is independent; false means unavailable, not zero.\",\n"
            << "    \"memoryMeaning\": \"Process privateCommitBytes is private committed virtual memory; workingSetBytes is currently resident process memory and can include shared pages. Peaks are OS process-lifetime peaks, not capture peaks. System commit and available RAM are different quantities. DXGI local/nonLocal CurrentUsage and Budget are process usage and OS-assigned dynamic budgets for node 0; local is not always dedicated VRAM on UMA. Do not sum process, system, local and non-local values. Allocation callstacks, resource-level bytes/lifetimes and OS wait causes are not measured.\",\n"
            << "    \"textureCache\": {\"producerScope\": \"CMaterial.LoadSharedTexture\", \"countersProduced\": [\"textureRequests\", \"texturePathHits\", \"textureUniqueSrvs\"], \"meaning\": \"Requests are load attempts, pathHits are successful weak-cache reuse, uniqueSrvs are successful new SRV creations during the frame, not current resident resources. Worker completions follow the capture frame counter boundary. contentHits and estimatedGpuBytes have no producer.\"},\n"
            << "    \"pipeline\": \"IA vertices and primitives are input-assembler counts, VS and PS are shader invocations. Read per-pass values only when pipelineValid is true; PS invocations are not final pixels or lighting arithmetic operations.\"\n"
            << "  },\n";
    }

    bool ValidMemory(const Engine::FProfilerMemoryStats& memory, uint64_t frameNumber, uint64_t frameEndTick)
    {
        if (!std::isfinite(memory.AgeMs) || memory.AgeMs < 0.0 || memory.AdapterNodeIndex != 0) return false;
        if (!memory.Sampled)
            return !memory.ProcessValid && !memory.SystemValid && !memory.Local.Valid && !memory.NonLocal.Valid &&
                !memory.AdapterIdentityValid && memory.SampleFrameNumber == 0 && memory.SampleTick == 0 && memory.AgeMs == 0.0;
        return memory.SampleFrameNumber != 0 && memory.SampleFrameNumber <= frameNumber &&
            memory.SampleTick != 0 && (frameEndTick == 0 || memory.SampleTick <= frameEndTick) && memory.ProcessId != 0;
    }

    void WriteMemory(std::ostream& stream, const Engine::FProfilerMemoryStats& memory)
    {
        stream << "      \"memory\": {\n"
            << "        \"sampled\": " << (memory.Sampled ? "true" : "false")
            << ", \"sampleFrameNumber\": " << memory.SampleFrameNumber
            << ", \"sampleTick\": " << memory.SampleTick << ", \"ageMs\": " << memory.AgeMs << ",\n"
            << "        \"processId\": " << memory.ProcessId << ", \"processValid\": " << (memory.ProcessValid ? "true" : "false") << ",\n"
            << "        \"privateCommitBytes\": " << memory.PrivateCommitBytes << ", \"workingSetBytes\": " << memory.WorkingSetBytes << ",\n"
            << "        \"peakWorkingSetBytes\": " << memory.PeakWorkingSetBytes << ", \"peakPrivateCommitBytes\": " << memory.PeakPrivateCommitBytes << ",\n"
            << "        \"systemValid\": " << (memory.SystemValid ? "true" : "false") << ", \"systemCommitBytes\": " << memory.SystemCommitBytes
            << ", \"systemCommitLimitBytes\": " << memory.SystemCommitLimitBytes << ", \"systemAvailableBytes\": " << memory.SystemAvailableBytes << ",\n"
            << "        \"adapterIdentityValid\": " << (memory.AdapterIdentityValid ? "true" : "false") << ", \"adapterLuidLow\": " << memory.AdapterLuidLow
            << ", \"adapterLuidHigh\": " << memory.AdapterLuidHigh << ", \"adapterNodeIndex\": " << memory.AdapterNodeIndex << ",\n";
        const auto segment = [&](const char* name, const Engine::FProfilerMemorySegment& value)
        {
            stream << "        \"" << name << "\": {\"valid\": " << (value.Valid ? "true" : "false")
                << ", \"currentUsageBytes\": " << value.CurrentUsageBytes << ", \"budgetBytes\": " << value.BudgetBytes << "}";
        };
        segment("local", memory.Local); stream << ",\n"; segment("nonLocal", memory.NonLocal); stream << "\n      },\n";
    }

    void WriteDrawStats(std::ostream& stream, const Engine::FProfilerDrawStats& draw)
    {
        stream << "{\"drawCalls\": " << draw.DrawCalls
            << ", \"instancedDrawCalls\": " << draw.InstancedDrawCalls
            << ", \"instances\": " << draw.Instances
            << ", \"indices\": " << draw.Indices
            << ", \"meshDrawCalls\": " << draw.MeshDrawCalls
            << ", \"meshInstances\": " << draw.MeshInstances
            << ", \"meshIndices\": " << draw.MeshIndices << "}";
    }

    template<size_t Size>
    void WriteFloatArray(std::ostream& stream, const std::array<float, Size>& values)
    {
        stream << '[';
        for (size_t i = 0; i < Size; ++i)
        {
            if (i) stream << ", ";
            if (std::isfinite(values[i])) stream << values[i]; else stream << "null";
        }
        stream << ']';
    }

    bool WriteMovement(std::ostream& stream, const Client::FProfilerCaptureContext& context,
        string* error, const std::atomic_bool* cancel)
    {
        const auto& coverage = context.MovementCoverage;
        stream << "  \"movementCoverage\": {\n    \"captured\": " << (coverage.Captured ? "true" : "false")
            << ",\n    \"capacity\": " << Client::CProfilerCaptureIO::MAX_MOVEMENT_SAMPLES
            << ",\n    \"windowBeginTick\": " << coverage.WindowBeginTick
            << ",\n    \"windowEndTick\": " << coverage.WindowEndTick
            << ",\n    \"framesWithBounds\": " << coverage.FramesWithBounds
            << ",\n    \"framesWithoutBounds\": " << coverage.FramesWithoutBounds
            << ",\n    \"windowMayBeTruncated\": " << (coverage.WindowMayBeTruncated ? "true" : "false")
            << ",\n    \"acceptedSinceReset\": " << coverage.AcceptedSinceReset
            << ",\n    \"overwrittenSinceReset\": " << coverage.OverwrittenSinceReset
            << ",\n    \"rejectedSinceReset\": " << coverage.RejectedSinceReset
            << ",\n    \"firstRetainedTick\": " << coverage.FirstRetainedTick
            << ",\n    \"lastRetainedTick\": " << coverage.LastRetainedTick
            << ",\n    \"outsideSavedFrames\": " << coverage.OutsideSavedFrames
            << ",\n    \"savedSamples\": " << context.MovementSamples.size()
            << ",\n    \"boundsSource\": \"completed-frame-main-thread-cpu-scopes\","
            << "\n    \"note\": \"Samples observe local character calls, not every rendered pixel. QPC ticks use ticksPerSecond. Only samples within a saved completed frame's main-thread scope bounds are included; scope bounds do not cover uninstrumented frame edges. Ring and rejected totals are since the last capture reset and can include history outside this window.\"\n  },\n";
        stream << "  \"movementSamples\": [\n";
        for (size_t i = 0; i < context.MovementSamples.size(); ++i)
        {
            if (i % 256 == 0 && Cancelled(cancel, error)) return false;
            const auto& sample = context.MovementSamples[i];
            if (!ValidMovementSample(sample))
            { SetError(error, "Invalid or non-finite movement capture sample."); return false; }
            stream << "    {\"kind\": \"" << MovementKindName(sample.Kind)
                << "\", \"qpcTick\": " << sample.QpcTick << ", \"frameNumber\": " << sample.FrameNumber
                << ", \"characterClass\": " << sample.CharacterClass << ", \"serverTick\": " << sample.ServerTick
                << ", \"sequence\": " << sample.Sequence << ", \"flags\": " << sample.Flags
                << ", \"disposition\": " << sample.Disposition << ", \"deltaSeconds\": " << sample.DeltaSeconds
                << ", \"motionSeconds\": " << sample.MotionSeconds
                << ", \"before\": "; WriteFloatArray(stream, sample.Before);
            stream << ", \"after\": "; WriteFloatArray(stream, sample.After);
            stream << ", \"authority\": "; WriteFloatArray(stream, sample.Authority);
            stream << ", \"waypoint\": "; WriteFloatArray(stream, sample.Waypoint);
            stream << "}" << (i + 1 < context.MovementSamples.size() ? "," : "") << "\n";
        }
        stream << "  ],\n";
        return true;
    }

    void WriteContext(std::ostream& stream, const Client::FProfilerCaptureContext& context)
    {
#ifdef _DEBUG
        constexpr const char* build = "Debug";
#else
        constexpr const char* build = "Release";
#endif
        stream << "  \"metadata\": {\n    \"sampledAtExport\": true,\n"
            << "    \"buildConfiguration\": \"" << build << "\",\n"
            << "    \"compilerMscVersion\": " << _MSC_VER << ",\n"
            << "    \"iteratorDebugLevel\": " << _ITERATOR_DEBUG_LEVEL << ",\n"
            << "    \"processId\": " << GetCurrentProcessId() << ",\n"
            << "    \"debuggerAttached\": " << (IsDebuggerPresent() ? "true" : "false") << ",\n"
            << "    \"logicalProcessors\": " << GetActiveProcessorCount(ALL_PROCESSOR_GROUPS) << ",\n"
            << "    \"runtimeContextValid\": " << (context.Valid ? "true" : "false") << ",\n"
            << "    \"adapter\": \"" << EscapeJson(context.Adapter) << "\",\n"
            << "    \"deviceCreationFlags\": " << context.DeviceCreationFlags << ",\n"
            << "    \"d3dDebugLayer\": " << ((context.DeviceCreationFlags & D3D11_CREATE_DEVICE_DEBUG) ? "true" : "false") << ",\n"
            << "    \"clientWindowForeground\": " << (context.ClientWindowForeground ? "true" : "false") << ",\n"
            << "    \"foregroundWindowOwnedByProcess\": " << (context.ProcessForeground ? "true" : "false") << ",\n"
            << "    \"windowMinimized\": " << (context.WindowMinimized ? "true" : "false") << ",\n"
            << "    \"configuredForegroundFpsLimit\": " << context.ForegroundFpsLimit << ",\n"
            << "    \"configuredBackgroundFpsLimit\": " << context.BackgroundFpsLimit << ",\n"
            << "    \"effectiveFpsLimit\": " << context.EffectiveFpsLimit << ",\n"
            << "    \"levelId\": " << context.LevelId << ",\n    \"viewport\": ";
        WriteFloatArray(stream, context.Viewport);
        stream << ",\n    \"cameraPosition\": "; WriteFloatArray(stream, context.CameraPosition);
        stream << ",\n    \"viewMatrix\": "; WriteFloatArray(stream, context.ViewMatrix);
        stream << ",\n    \"projectionMatrix\": "; WriteFloatArray(stream, context.ProjectionMatrix);
        stream << ",\n    \"shadowEnabled\": " << (context.ShadowEnabled ? "true" : "false")
            << ",\n    \"shadowWidthHeightStrength\": ";
        WriteFloatArray(stream, std::array<float, 3>{context.ShadowWidth, context.ShadowHeight, context.ShadowStrength});
        stream << ",\n    \"ssaoEnabled\": " << (context.SSAOEnabled ? "true" : "false")
            << ",\n    \"bloomEnabled\": " << (context.BloomEnabled ? "true" : "false")
            << ",\n    \"fxaaEnabled\": " << (context.FXAAEnabled ? "true" : "false")
            << ",\n    \"renderingOptions\": {";
        bool first = true;
        for (const auto& [key, value] : context.RenderingOptions)
        {
            if (!first) stream << ','; first = false;
            stream << '"' << EscapeJson(key) << "\":";
            if (std::isfinite(value)) stream << value; else stream << "null";
        }
        stream << "},\n    \"renderingAssets\": {"; first = true;
        for (const auto& [key, value] : context.RenderingAssets)
        {
            if (!first) stream << ','; first = false;
            stream << '"' << EscapeJson(key) << "\":\"" << EscapeJson(value) << '"';
        }
        stream << "},\n    \"note\": \"Current runtime context only; historical frames may have different levels, cameras, viewport sizes, focus and settings. FPS limits are checkbox-resolved user limits (0 means disabled); effectiveFpsLimit selects by foreground process ownership and excludes the separate minimized-frame message wait.\"\n  },\n";
    }

bool SaveJsonImpl(
	const Engine::FProfilerCaptureSnapshot& Snapshot,
	const filesystem::path& OutputPath,
	string* pOutError, const std::atomic_bool* pCancel, bool ReplaceExisting,
    const Client::FProfilerCaptureContext& Context)
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
    WriteMeasurementSemantics(Stream);
    WriteContext(Stream, Context);
    WriteCaptureWindow(Stream, Snapshot);
    WriteSummary(Stream, Snapshot);
    if (!WriteMovement(Stream, Context, pOutError, pCancel)) return false;
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
            !std::isfinite(Frame.PreviousCpuFrameMs) || !std::isfinite(Frame.FrameGapMs) ||
            Frame.PreviousCpuFrameMs < 0.0 || Frame.FrameGapMs < 0.0 ||
            Frame.FrameEndTick < Frame.FrameBeginTick || !ValidMemory(Frame.Memory, Frame.FrameNumber, Frame.FrameEndTick) ||
			!std::isfinite(Frame.Animation.CpuMs) || !std::isfinite(Frame.Animation.NotSubmittedCpuMs))
		{
			SetError(pOutError, "Profiler frame has invalid GPU status or non-finite timing.");
			return false;
		}
		Stream << "    {\n";
		Stream << "      \"frameNumber\": " << Frame.FrameNumber << ",\n";
        Stream << "      \"frameBeginTick\": " << Frame.FrameBeginTick << ",\n";
        Stream << "      \"frameEndTick\": " << Frame.FrameEndTick << ",\n";
        Stream << "      \"previousCpuFrameMs\": " << Frame.PreviousCpuFrameMs << ",\n";
        Stream << "      \"frameGapMs\": " << Frame.FrameGapMs << ",\n";
		Stream << "      \"cpuFrameMs\": " << Frame.CpuFrameMs << ",\n";
        Stream << "      \"droppedCpuScopes\": " << Frame.DroppedCpuScopes << ",\n";
        Stream << "      \"detailedCpuScopes\": " << (Frame.DetailedCpuScopes ? "true" : "false") << ",\n";
		Stream << "      \"frameIntervalMs\": " << Frame.FrameIntervalMs << ",\n";
		Stream << "      \"gpuFrameMs\": " << Frame.GpuFrameMs << ",\n";
		Stream << "      \"gpuValid\": " << (Frame.GpuValid ? "true" : "false") << ",\n";
		Stream << "      \"gpuStatus\": \"" << pGpuStatus << "\",\n";
		Stream << "      \"gpuLatencyFrames\": " << Frame.GpuLatencyFrames << ",\n";
		Stream << "      \"gpuScopesSupported\": " << (Frame.GpuScopesSupported ? "true" : "false") << ",\n";
		Stream << "      \"droppedGpuScopes\": " << Frame.DroppedGpuScopes << ",\n";
        WriteMemory(Stream, Frame.Memory);
		Stream << "      \"animation\": {\n";
		Stream << "        \"updateCalls\": " << Frame.Animation.UpdateCalls << ",\n";
		Stream << "        \"updatedModels\": " << Frame.Animation.UpdatedModels << ",\n";
		Stream << "        \"submittedUpdatedModels\": " << Frame.Animation.SubmittedUpdatedModels << ",\n";
		Stream << "        \"notSubmittedUpdatedModels\": " << Frame.Animation.NotSubmittedUpdatedModels << ",\n";
		Stream << "        \"droppedSamples\": " << Frame.Animation.DroppedSamples << ",\n";
		Stream << "        \"cpuMs\": " << Frame.Animation.CpuMs << ",\n";
		Stream << "        \"notSubmittedCpuMs\": " << Frame.Animation.NotSubmittedCpuMs << "\n";
		Stream << "      },\n";
        Stream << "      \"cpuWork\": [\n";
        for (size_t iWork = 0; iWork < Frame.CpuWork.size(); ++iWork)
        {
            const auto& work = Frame.CpuWork[iWork];
            if (!std::isfinite(work.CpuMs) || work.CpuMs < 0.0)
            {
                SetError(pOutError, "Profiler work category has invalid timing.");
                return false;
            }
            Stream << "        {\"name\": \""
                << Engine::CProfiler::Get_WorkName(static_cast<Engine::EProfilerWork>(iWork))
                << "\", \"calls\": " << work.Calls << ", \"cpuMs\": " << work.CpuMs
                << "}" << (iWork + 1 < Frame.CpuWork.size() ? "," : "") << "\n";
        }
        Stream << "      ],\n";
		Stream << "      \"counters\": {\n";
		for (size_t iCounter = 0; iCounter < CounterNames.size(); ++iCounter)
		{
			Stream << "        \"" << CounterNames[iCounter] << "\": "
				<< Frame.Counters[iCounter]
				<< (iCounter + 1 < CounterNames.size() ? "," : "")
				<< "\n";
		}
		Stream << "      },\n";

        Stream << "      \"droppedMeshDraws\": " << Frame.DroppedMeshDraws << ",\n";
        Stream << "      \"meshDraws\": [";
        for (size_t i = 0; i < Frame.MeshDraws.size(); ++i)
        {
            const auto& draw = Frame.MeshDraws[i];
            if (draw.MeshNameId >= Snapshot.ScopeNames.size() ||
                (draw.PassNameId != UINT32_MAX && draw.PassNameId >= Snapshot.ScopeNames.size()))
            { SetError(pOutError, "Mesh draw has an invalid display name."); return false; }
            if (i) Stream << ",";
            Stream << "{\"passNameId\":" << draw.PassNameId << ",\"meshNameId\":" << draw.MeshNameId
                << ",\"vertexCount\":" << draw.VertexCount << ",\"indexCount\":" << draw.IndexCount
                << ",\"instances\":" << draw.Instances << ",\"materialSlot\":" << draw.MaterialSlot << "}";
        }
        Stream << "],\n";
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
		Stream << "      \"droppedViewportPresents\": " << Frame.DroppedViewportPresents << ",\n";
		Stream << "      \"viewportPresents\": [";
		for (size_t i = 0; i < Frame.ViewportPresents.size(); ++i)
		{
			const auto& p = Frame.ViewportPresents[i];
			if (!std::isfinite(p.CpuMs) || !std::isfinite(p.X) || !std::isfinite(p.Y) ||
				!std::isfinite(p.Width) || !std::isfinite(p.Height))
			{ SetError(pOutError, "Non-finite viewport presentation sample."); return false; }
			if (i) Stream << ", ";
			Stream << "{\"viewportId\": " << p.ViewportId << ", \"x\": " << p.X
				<< ", \"y\": " << p.Y << ", \"width\": " << p.Width << ", \"height\": " << p.Height
				<< ", \"cpuMs\": " << p.CpuMs << ", \"syncInterval\": " << p.SyncInterval
				<< ", \"flags\": " << p.Flags << ", \"hresult\": " << p.Result << "}";
		}
		Stream << "],\n";
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
				!std::isfinite(Scope.DurationMs) || !std::isfinite(Scope.SelfMs) || Scope.SelfMs < 0.0)
			{
				SetError(pOutError, "Profiler GPU scope has non-finite timing.");
				return false;
			}
			Stream << "        {\"nameId\": " << Scope.NameId
				<< ", \"depth\": " << Scope.Depth
				<< ", \"beginMs\": " << Scope.BeginMs
				<< ", \"endMs\": " << Scope.EndMs
				<< ", \"durationMs\": " << Scope.DurationMs
                << ", \"selfMs\": " << Scope.SelfMs
				<< ", \"pipelineValid\": " << (Scope.PipelineValid ? "true" : "false")
				<< ", \"psInvocations\": " << Scope.PSInvocations
				<< ", \"vsInvocations\": " << Scope.VSInvocations
                << ", \"iaVertices\": " << Scope.IAVertices
                << ", \"iaPrimitives\": " << Scope.IAPrimitives
                << ", \"draw\": ";
            WriteDrawStats(Stream, Scope.Draw);
            Stream << "}"
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
		const std::atomic_bool* pCancel = nullptr, bool ReplaceExisting = true,
        const Client::FProfilerCaptureContext& Context = {})
	{
		try
		{
			return SaveJsonImpl(Snapshot, OutputPath, pOutError, pCancel, ReplaceExisting, Context);
		}
		catch (const std::exception& Exception)
		{
			SetError(pOutError, "Profiler capture save failed: " + string(Exception.what()));
			return false;
		}
	}
}

void Client::CProfilerCaptureIO::Record_MovementSample(const Engine::CProfiler* profiler,
    FProfilerMovementSample sample) noexcept
{
    if (!profiler || !profiler->Is_Enabled()) return;
    if (GetCurrentThreadId() != profiler->Get_MainThreadId()) return;
    if (!ValidMovementSample(sample)) { ++MovementRing.Rejected; return; }
    LARGE_INTEGER tick{};
    if (!QueryPerformanceCounter(&tick) || tick.QuadPart <= 0) { ++MovementRing.Rejected; return; }
    sample.QpcTick = static_cast<uint64_t>(tick.QuadPart);
    sample.FrameNumber = 0;
    if (MovementRing.Count == MAX_MOVEMENT_SAMPLES)
    {
        ++MovementRing.Overwritten;
        MovementRing.LastOverwrittenTick = MovementRing.Samples[MovementRing.Next].QpcTick;
    }
    else ++MovementRing.Count;
    MovementRing.Samples[MovementRing.Next] = sample;
    MovementRing.Next = (MovementRing.Next + 1) % MAX_MOVEMENT_SAMPLES;
    ++MovementRing.Accepted;
}

void Client::CProfilerCaptureIO::Reset_MovementSamples() noexcept
{
    MovementRing.Next = MovementRing.Count = 0;
    MovementRing.Accepted = MovementRing.Overwritten = MovementRing.Rejected = MovementRing.LastOverwrittenTick = 0;
}

void Client::CProfilerCaptureIO::Copy_MovementSamples(const Engine::FProfilerCaptureSnapshot& snapshot,
    FProfilerCaptureContext& context)
{
    context.MovementSamples.clear();
    context.MovementCoverage = {};
    if (GetCurrentThreadId() != snapshot.MainThreadId) return;
    auto& coverage = context.MovementCoverage;
    coverage.Captured = true;
    coverage.AcceptedSinceReset = MovementRing.Accepted;
    coverage.OverwrittenSinceReset = MovementRing.Overwritten;
    coverage.RejectedSinceReset = MovementRing.Rejected;
    struct FFrameBounds final { uint64_t Number, Begin, End; };
    std::vector<FFrameBounds> bounds;
    bounds.reserve(snapshot.Frames.size());
    for (const auto& frame : snapshot.Frames)
    {
        uint64_t begin = UINT64_MAX, end = 0;
        for (const auto& scope : frame.CpuScopes)
        {
            if (scope.ThreadId != snapshot.MainThreadId || !scope.BeginTick || scope.EndTick < scope.BeginTick) continue;
            begin = (std::min)(begin, scope.BeginTick);
            end = (std::max)(end, scope.EndTick);
        }
        if (!end) { ++coverage.FramesWithoutBounds; continue; }
        bounds.push_back({frame.FrameNumber, begin, end});
        ++coverage.FramesWithBounds;
        coverage.WindowBeginTick = coverage.WindowBeginTick ? (std::min)(coverage.WindowBeginTick, begin) : begin;
        coverage.WindowEndTick = (std::max)(coverage.WindowEndTick, end);
    }
    coverage.WindowMayBeTruncated = coverage.FramesWithoutBounds != 0 || MovementRing.Rejected != 0 ||
        (coverage.WindowBeginTick && MovementRing.LastOverwrittenTick >= coverage.WindowBeginTick);
    const size_t first = (MovementRing.Next + MAX_MOVEMENT_SAMPLES - MovementRing.Count) % MAX_MOVEMENT_SAMPLES;
    context.MovementSamples.reserve(MovementRing.Count);
    for (size_t i = 0; i < MovementRing.Count; ++i)
    {
        auto sample = MovementRing.Samples[(first + i) % MAX_MOVEMENT_SAMPLES];
        if (!i) coverage.FirstRetainedTick = sample.QpcTick;
        coverage.LastRetainedTick = sample.QpcTick;
        const auto frame = std::find_if(bounds.begin(), bounds.end(), [&](const auto& range)
        { return sample.QpcTick >= range.Begin && sample.QpcTick <= range.End; });
        if (frame == bounds.end()) { ++coverage.OutsideSavedFrames; continue; }
        sample.FrameNumber = frame->Number;
        context.MovementSamples.push_back(sample);
    }
}

bool_t Client::CProfilerCaptureIO::Save_Json(
	const Engine::FProfilerCaptureSnapshot& Snapshot,
	const filesystem::path& OutputPath, string* pOutError, const FProfilerCaptureContext& Context)
{
	return SaveJsonSafely(Snapshot, OutputPath, pOutError, nullptr, true, Context);
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
	filesystem::path OutputPath, string* pOutError, FProfilerCaptureContext Context)
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
		m_Worker = std::thread([Job, Captured = std::move(Snapshot), Context = std::move(Context)]()
		{
			Job->Result.Succeeded = SaveJsonSafely(Captured,
				Job->Result.OutputPath, &Job->Result.Error, &Job->Cancel, false, Context);
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

namespace
{
    using Client::FProfilerComparisonCapture;
    using Client::FProfilerComparisonFrame;
    using Client::FProfilerComparisonValue;
    using Client::DATA_JSON_VALUE;

    void ComparisonAdd(FProfilerComparisonFrame& frame, const std::string& key, double value, bool valid = true)
    {
        auto [it, inserted] = frame.Values.try_emplace(key, FProfilerComparisonValue{0.0, valid});
        it->second.Available = it->second.Available && valid && std::isfinite(value) && value >= 0.0;
        if (it->second.Available) it->second.Value += value;
    }

    // Sort by each thread's interval, not NameId or completion order. Only direct
    // child intervals subtract from a parent; workers never subtract from main.
    void ComparisonCpu(FProfilerComparisonFrame& target,
        const std::vector<Engine::FProfilerScopeSample>& scopes,
        const std::vector<std::string>& names, uint32_t mainThread, uint64_t frequency,
        uint64_t frameBegin, uint64_t frameEnd)
    {
        if (!frequency) { target.CpuScopesKnown = target.CpuSelfKnown = false; return; }
        // Missing legacy bounds and cross-frame completion cannot prove that all
        // children are present in this frame. Inclusive observations remain valid.
        if (frameBegin == 0 || frameEnd < frameBegin) target.CpuSelfKnown = false;
        std::map<uint32_t, std::vector<const Engine::FProfilerScopeSample*>> threads;
        for (const auto& scope : scopes) threads[scope.ThreadId].push_back(&scope);
        for (auto& [thread, samples] : threads)
        {
            std::stable_sort(samples.begin(), samples.end(), [](const auto* a, const auto* b)
            {
                if (a->BeginTick != b->BeginTick) return a->BeginTick < b->BeginTick;
                if (a->EndTick != b->EndTick) return a->EndTick > b->EndTick;
                return a->Depth < b->Depth;
            });
            std::vector<size_t> stack;
            std::vector<double> self(samples.size());
            for (size_t i = 0; i < samples.size(); ++i)
            {
                const auto& s = *samples[i];
                if (s.BeginTick < frameBegin || s.EndTick > frameEnd) target.CpuSelfKnown = false;
                if (s.NameId >= names.size() || s.EndTick < s.BeginTick) { target.CpuSelfKnown = false; continue; }
                const double duration = double(s.EndTick - s.BeginTick) * 1000.0 / double(frequency);
                self[i] = duration;
                while (!stack.empty() && samples[stack.back()]->Depth >= s.Depth)
                {
                    // Siblings on one thread cannot overlap. A malformed depth
                    // must not turn overlapping child time into a clamped zero self.
                    if (s.BeginTick < samples[stack.back()]->EndTick) target.CpuSelfKnown = false;
                    stack.pop_back();
                }
                if (!stack.empty())
                {
                    const auto& parent = *samples[stack.back()];
                    if (s.Depth == parent.Depth + 1 && s.BeginTick >= parent.BeginTick && s.EndTick <= parent.EndTick)
                        self[stack.back()] -= duration;
                    else target.CpuSelfKnown = false;
                }
                // The enclosing scope may legitimately finish in another frame.
                // Keep its inclusive observation, but this tree cannot prove self.
                else if (s.Depth != 0) target.CpuSelfKnown = false;
                stack.push_back(i);
                const std::string role = thread == mainThread ? "cpu.main.total::" : "cpu.worker.total::";
                ComparisonAdd(target, role + names[s.NameId], duration);
            }
            for (size_t i = 0; i < samples.size(); ++i)
                if (samples[i]->NameId < names.size())
                    ComparisonAdd(target, std::string(thread == mainThread ? "cpu.main.self::" : "cpu.worker.self::") +
                        names[samples[i]->NameId], (std::max)(0.0, self[i]), target.CpuSelfKnown);
        }
        if (!target.CpuSelfKnown)
            for (auto& [key, value] : target.Values)
                if (key.starts_with("cpu.") && key.find(".self::") != std::string::npos) value.Available = false;
    }

    template <size_t N> std::string ComparisonArray(const std::array<float, N>& values)
    {
        std::ostringstream stream; stream.imbue(std::locale::classic()); stream << std::setprecision(9);
        for (size_t i = 0; i < N; ++i) { if (i) stream << ", "; stream << values[i]; }
        return stream.str();
    }

    constexpr const char* ComparisonPipelineNames[] = {"iaVertices", "iaPrimitives", "vsInvocations",
        "gsInvocations", "gsPrimitives", "clipperInvocations", "clipperPrimitives", "psInvocations",
        "hsInvocations", "dsInvocations", "csInvocations"};
    constexpr const char* ComparisonDrawNames[] = {"drawCalls", "instancedDrawCalls", "instances", "indices",
        "meshDrawCalls", "meshInstances", "meshIndices"};
    constexpr const char* ComparisonPassPipelineNames[] = {"psInvocations", "vsInvocations", "iaVertices", "iaPrimitives"};
    bool ComparisonCounterMeasured(std::string_view name, const std::set<std::string>& textureProduced)
    {
        if (name.starts_with("texture")) return textureProduced.contains(std::string(name));
        return name != "indirectDrawCalls" && name != "indirectIndexUpperBound";
    }
    const std::set<std::string> TextureCountersProduced{"textureRequests", "texturePathHits", "textureUniqueSrvs"};

    // A malformed optional observation is an error; an absent observation stays absent.
    double ComparisonNumber(const DATA_JSON_VALUE& value, double maximum = 9007199254740991.0, bool integer = false)
    {
        if (!value.Is_Number() || !std::isfinite(value.Get_Number()) || value.Get_Number() < 0.0 ||
            value.Get_Number() > maximum || (integer && std::floor(value.Get_Number()) != value.Get_Number()))
            throw std::runtime_error("Capture contains an invalid or out-of-range number.");
        return value.Get_Number();
    }
    const DATA_JSON_VALUE& ComparisonRequired(const DATA_JSON_VALUE& parent, const char* name)
    {
        const auto* value = parent.Find(name);
        if (!value) throw std::runtime_error(std::string("Capture is missing required field: ") + name);
        return *value;
    }
    uint64_t ComparisonUInt(const DATA_JSON_VALUE& parent, const char* name, uint64_t maximum = 9007199254740991ull)
    { return static_cast<uint64_t>(ComparisonNumber(ComparisonRequired(parent, name), double(maximum), true)); }
    bool ComparisonBool(const DATA_JSON_VALUE& value)
    {
        if (!value.Is_Boolean()) throw std::runtime_error("Capture contains a non-boolean flag.");
        return value.Get_Boolean();
    }
    std::string ComparisonString(const DATA_JSON_VALUE& value, size_t limit = 512)
    {
        if (!value.Is_String() || value.Get_String().size() > limit ||
            value.Get_String().find('\0') != std::string::npos)
            throw std::runtime_error("Capture contains an invalid or oversized string.");
        const auto& text = value.Get_String();
        if (!text.empty() && !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), nullptr, 0))
            throw std::runtime_error("Capture contains invalid UTF-8.");
        return text;
    }
    void ComparisonObject(const DATA_JSON_VALUE& value)
    { if (!value.Is_Object()) throw std::runtime_error("Capture object has the wrong type."); }
    const DATA_JSON_VALUE::ARRAY& ComparisonArrayValue(const DATA_JSON_VALUE& value, size_t limit)
    {
        if (!value.Is_Array() || value.Get_Array().size() > limit)
            throw std::runtime_error("Capture array has the wrong type or exceeds the limit.");
        return value.Get_Array();
    }
    void ComparisonOptionalMetric(const DATA_JSON_VALUE& object, const char* field,
        FProfilerComparisonFrame& frame, const std::string& key, bool valid = true, bool integer = false)
    {
        if (const auto* value = object.Find(field))
            ComparisonAdd(frame, key, ComparisonNumber(*value, integer ? 9007199254740991.0 : 3600000.0, integer), valid);
    }
    void ComparisonMemory(FProfilerComparisonFrame& frame)
    {
        const auto& m = frame.Memory;
        const auto add = [&](const char* name, uint64_t value, bool valid)
        { ComparisonAdd(frame, std::string("memory::") + name, static_cast<double>(value), m.Sampled && valid); };
        add("privateCommitBytes", m.PrivateCommitBytes, m.ProcessValid);
        add("workingSetBytes", m.WorkingSetBytes, m.ProcessValid);
        add("peakWorkingSetBytes", m.PeakWorkingSetBytes, m.ProcessValid);
        add("peakPrivateCommitBytes", m.PeakPrivateCommitBytes, m.ProcessValid);
        add("systemCommitBytes", m.SystemCommitBytes, m.SystemValid);
        add("systemCommitLimitBytes", m.SystemCommitLimitBytes, m.SystemValid);
        add("systemAvailableBytes", m.SystemAvailableBytes, m.SystemValid);
        add("localUsageBytes", m.Local.CurrentUsageBytes, m.Local.Valid);
        add("localBudgetBytes", m.Local.BudgetBytes, m.Local.Valid);
        add("nonLocalUsageBytes", m.NonLocal.CurrentUsageBytes, m.NonLocal.Valid);
        add("nonLocalBudgetBytes", m.NonLocal.BudgetBytes, m.NonLocal.Valid);
    }

    void ComparisonReadMemory(const DATA_JSON_VALUE& source, FProfilerComparisonFrame& frame)
    {
        const auto* memory = source.Find("memory");
        if (!memory) return; // Legacy captures have no OS observation, not zero memory.
        ComparisonObject(*memory);
        auto& m = frame.Memory; frame.MemoryKnown = true;
        m.Sampled = ComparisonBool(ComparisonRequired(*memory, "sampled"));
        m.SampleFrameNumber = ComparisonUInt(*memory, "sampleFrameNumber");
        m.SampleTick = ComparisonUInt(*memory, "sampleTick");
        m.AgeMs = ComparisonNumber(ComparisonRequired(*memory, "ageMs"));
        m.ProcessId = static_cast<uint32_t>(ComparisonUInt(*memory, "processId", UINT32_MAX));
        m.ProcessValid = ComparisonBool(ComparisonRequired(*memory, "processValid"));
        m.PrivateCommitBytes = ComparisonUInt(*memory, "privateCommitBytes");
        m.WorkingSetBytes = ComparisonUInt(*memory, "workingSetBytes");
        m.PeakWorkingSetBytes = ComparisonUInt(*memory, "peakWorkingSetBytes");
        m.PeakPrivateCommitBytes = ComparisonUInt(*memory, "peakPrivateCommitBytes");
        m.SystemValid = ComparisonBool(ComparisonRequired(*memory, "systemValid"));
        m.SystemCommitBytes = ComparisonUInt(*memory, "systemCommitBytes");
        m.SystemCommitLimitBytes = ComparisonUInt(*memory, "systemCommitLimitBytes");
        m.SystemAvailableBytes = ComparisonUInt(*memory, "systemAvailableBytes");
        m.AdapterIdentityValid = ComparisonBool(ComparisonRequired(*memory, "adapterIdentityValid"));
        m.AdapterLuidLow = static_cast<uint32_t>(ComparisonUInt(*memory, "adapterLuidLow", UINT32_MAX));
        const auto& high = ComparisonRequired(*memory, "adapterLuidHigh");
        if (!high.Is_Number() || !std::isfinite(high.Get_Number()) || std::floor(high.Get_Number()) != high.Get_Number() ||
            high.Get_Number() < INT32_MIN || high.Get_Number() > INT32_MAX)
            throw std::runtime_error("Capture adapter LUID high part is invalid.");
        m.AdapterLuidHigh = static_cast<int32_t>(high.Get_Number());
        m.AdapterNodeIndex = static_cast<uint32_t>(ComparisonUInt(*memory, "adapterNodeIndex", 0));
        const auto segment = [&](const char* name, Engine::FProfilerMemorySegment& value)
        {
            const auto& object = ComparisonRequired(*memory, name); ComparisonObject(object);
            value.Valid = ComparisonBool(ComparisonRequired(object, "valid"));
            value.CurrentUsageBytes = ComparisonUInt(object, "currentUsageBytes");
            value.BudgetBytes = ComparisonUInt(object, "budgetBytes");
        };
        segment("local", m.Local); segment("nonLocal", m.NonLocal);
        const uint64_t frameEnd = source.Find("frameEndTick") ? ComparisonUInt(source, "frameEndTick") : 0;
        if (!ValidMemory(m, frame.Number, frameEnd)) throw std::runtime_error("Capture memory observation has inconsistent validity or sample bounds.");
        ComparisonMemory(frame);
    }

    std::set<std::string> ComparisonTextureCapabilities(const DATA_JSON_VALUE& root)
    {
        std::set<std::string> produced;
        const auto* semantics = root.Find("measurementSemantics");
        if (!semantics) return produced;
        ComparisonObject(*semantics);
        const auto* texture = semantics->Find("textureCache");
        if (!texture) return produced;
        ComparisonObject(*texture);
        const bool known = ComparisonString(ComparisonRequired(*texture, "producerScope")) == "CMaterial.LoadSharedTexture";
        std::set<std::string> seen;
        for (const auto& value : ComparisonArrayValue(ComparisonRequired(*texture, "countersProduced"), 32))
        {
            const auto name = ComparisonString(value);
            if (!seen.insert(name).second) throw std::runtime_error("Capture repeats a texture producer capability.");
            if (known && TextureCountersProduced.contains(name)) produced.insert(name);
        }
        return produced;
    }

    void ComparisonMetadata(const DATA_JSON_VALUE& root, FProfilerComparisonCapture& capture)
    {
        const auto* metadata = root.Find("metadata");
        if (!metadata) return;
        ComparisonObject(*metadata);
        constexpr const char* fields[] = {"sampledAtExport", "runtimeContextValid", "buildConfiguration", "adapter",
            "deviceCreationFlags", "d3dDebugLayer", "debuggerAttached", "logicalProcessors",
            "clientWindowForeground", "foregroundWindowOwnedByProcess", "windowMinimized",
            "configuredForegroundFpsLimit", "configuredBackgroundFpsLimit", "effectiveFpsLimit",
            "levelId", "viewport", "cameraPosition", "viewMatrix", "projectionMatrix",
            "shadowEnabled", "shadowWidthHeightStrength", "ssaoEnabled", "bloomEnabled", "fxaaEnabled"};
        for (const auto* field : fields)
        {
            const auto* value = metadata->Find(field); if (!value) continue;
            const std::string_view key(field);
            const bool stringField = key == "buildConfiguration" || key == "adapter";
            const bool arrayField = key == "viewport" || key == "cameraPosition" || key == "viewMatrix" ||
                key == "projectionMatrix" || key == "shadowWidthHeightStrength";
            const bool integerField = key == "deviceCreationFlags" || key == "logicalProcessors" || key == "levelId" ||
                key == "configuredForegroundFpsLimit" || key == "configuredBackgroundFpsLimit" || key == "effectiveFpsLimit";
            if ((stringField && !value->Is_String()) || (arrayField && !value->Is_Array()) ||
                (integerField && !value->Is_Number()) || (!stringField && !arrayField && !integerField && !value->Is_Boolean()))
                throw std::runtime_error("Capture metadata has the wrong field type.");
            std::ostringstream text; text.imbue(std::locale::classic()); text << std::setprecision(9);
            if (value->Is_String()) text << ComparisonString(*value, 1024);
            else if (value->Is_Boolean()) text << (value->Get_Boolean() ? "true" : "false");
            else if (value->Is_Number()) text << ComparisonNumber(*value, UINT32_MAX, true);
            else if (value->Is_Array())
            {
                const auto& array = ComparisonArrayValue(*value, 16);
                const size_t expected = std::string_view(field) == "viewport" ? 2 :
                    std::string_view(field) == "cameraPosition" ? 4 :
                    std::string_view(field) == "shadowWidthHeightStrength" ? 3 : 16;
                if (array.size() != expected) throw std::runtime_error("Capture metadata array has the wrong size.");
                for (size_t i = 0; i < array.size(); ++i)
                {
                    if (!array[i].Is_Number() || !std::isfinite(array[i].Get_Number()) || std::abs(array[i].Get_Number()) > 1.0e12)
                        throw std::runtime_error("Capture metadata contains an invalid number.");
                    if (i) text << ", "; text << array[i].Get_Number();
                }
            }
            else throw std::runtime_error("Capture metadata has the wrong type.");
            capture.Conditions[field] = text.str();
        }
        const auto valid = capture.Conditions.find("runtimeContextValid");
        for (const auto* group : {"renderingOptions", "renderingAssets"})
            if (const auto* values = metadata->Find(group))
            {
                ComparisonObject(*values);
                if (values->Get_Object().size() > 256) throw std::runtime_error("Too many rendering context fields.");
                for (const auto& [key, value] : values->Get_Object())
                {
                    if (key.empty() || key.size() > 128 || key.find('\0') != std::string::npos)
                        throw std::runtime_error("Rendering context key is invalid.");
                    std::string text;
                    if (std::string_view(group) == "renderingAssets") text = ComparisonString(value, 2048);
                    else
                    {
                        if (!value.Is_Number() || !std::isfinite(value.Get_Number()) || std::abs(value.Get_Number()) > 1.0e12)
                            throw std::runtime_error("Rendering context value is invalid.");
                        std::ostringstream number; number.imbue(std::locale::classic()); number << std::setprecision(9) << value.Get_Number(); text = number.str();
                    }
                    if (valid != capture.Conditions.end() && valid->second == "true")
                        capture.Conditions[std::string(group) + "." + key] = std::move(text);
                }
            }
        if (valid == capture.Conditions.end() || valid->second != "true")
            for (const auto* field : {"levelId", "viewport", "cameraPosition", "viewMatrix", "projectionMatrix", "shadowEnabled",
                "shadowWidthHeightStrength", "ssaoEnabled", "bloomEnabled", "fxaaEnabled", "clientWindowForeground",
                "foregroundWindowOwnedByProcess", "windowMinimized", "configuredForegroundFpsLimit", "configuredBackgroundFpsLimit", "effectiveFpsLimit"})
                capture.Conditions.erase(field);
    }
}

const char* Client::CProfilerCaptureIO::Counter_Name(size_t index)
{ return index < CounterNames.size() ? CounterNames[index] : "unknown"; }

Client::FProfilerComparisonCapture Client::CProfilerCaptureIO::Build_Comparison(
    const Engine::FProfilerCaptureSnapshot& snapshot, const FProfilerCaptureContext& context, std::string label)
{
    FProfilerComparisonCapture result;
    result.Label = std::move(label); result.Source = "session";
    auto& c = result.Conditions;
    c["sampledAtExport"] = "true"; c["runtimeContextValid"] = context.Valid ? "true" : "false";
#ifdef _DEBUG
    c["buildConfiguration"] = "Debug";
#else
    c["buildConfiguration"] = "Release";
#endif
    c["debuggerAttached"] = IsDebuggerPresent() ? "true" : "false";
    c["adapter"] = context.Adapter;
    c["deviceCreationFlags"] = std::to_string(context.DeviceCreationFlags);
    c["d3dDebugLayer"] = context.DeviceCreationFlags & D3D11_CREATE_DEVICE_DEBUG ? "true" : "false";
    if (context.Valid)
    {
        for (const auto& [key, value] : context.RenderingOptions)
        {
            if (!std::isfinite(value)) continue;
            std::ostringstream number; number.imbue(std::locale::classic()); number << std::setprecision(9) << value;
            c["renderingOptions." + key] = number.str();
        }
        for (const auto& [key, value] : context.RenderingAssets) c["renderingAssets." + key] = value;
        c["levelId"] = std::to_string(context.LevelId);
        c["viewport"] = ComparisonArray(context.Viewport);
        c["cameraPosition"] = ComparisonArray(context.CameraPosition);
        c["viewMatrix"] = ComparisonArray(context.ViewMatrix);
        c["projectionMatrix"] = ComparisonArray(context.ProjectionMatrix);
        c["shadowWidthHeightStrength"] = ComparisonArray(std::array{context.ShadowWidth, context.ShadowHeight, context.ShadowStrength});
        c["shadowEnabled"] = context.ShadowEnabled ? "true" : "false";
        c["ssaoEnabled"] = context.SSAOEnabled ? "true" : "false";
        c["bloomEnabled"] = context.BloomEnabled ? "true" : "false";
        c["fxaaEnabled"] = context.FXAAEnabled ? "true" : "false";
        c["clientWindowForeground"] = context.ClientWindowForeground ? "true" : "false";
        c["foregroundWindowOwnedByProcess"] = context.ProcessForeground ? "true" : "false";
        c["windowMinimized"] = context.WindowMinimized ? "true" : "false";
        c["configuredForegroundFpsLimit"] = std::to_string(context.ForegroundFpsLimit);
        c["configuredBackgroundFpsLimit"] = std::to_string(context.BackgroundFpsLimit);
        c["effectiveFpsLimit"] = std::to_string(context.EffectiveFpsLimit);
    }
    result.Frames.reserve(snapshot.Frames.size());
    for (const auto& source : snapshot.Frames)
    {
        FProfilerComparisonFrame frame;
        frame.Number = source.FrameNumber;
        frame.MemoryKnown = true; frame.Memory = source.Memory;
        ComparisonMemory(frame);
        frame.CpuScopesKnown = snapshot.TicksPerSecond != 0;
        frame.CpuSelfKnown = frame.CpuScopesKnown && source.DroppedCpuScopes == 0;
        frame.DetailKnown = true; frame.Detailed = source.DetailedCpuScopes;
        frame.GpuStatus = source.GpuStatus; frame.GpuValid = source.GpuValid;
        frame.MeshDrawsKnown = source.DetailedCpuScopes;
        frame.DroppedMeshDraws = source.DroppedMeshDraws;
        for (const auto& draw : source.MeshDraws)
        {
            FProfilerComparisonMeshDraw item;
            if (draw.PassNameId < snapshot.ScopeNames.size()) item.Pass = snapshot.ScopeNames[draw.PassNameId];
            if (draw.MeshNameId < snapshot.ScopeNames.size()) item.Mesh = snapshot.ScopeNames[draw.MeshNameId];
            item.VertexCount = draw.VertexCount; item.IndexCount = draw.IndexCount;
            item.Instances = draw.Instances; item.MaterialSlot = draw.MaterialSlot;
            frame.MeshDraws.push_back(std::move(item));
        }
        frame.GpuComplete = source.GpuValid && source.GpuScopesSupported && source.DroppedGpuScopes == 0;
        ComparisonAdd(frame, "frame::cpu", source.CpuFrameMs);
        ComparisonAdd(frame, "frame::interval", source.FrameIntervalMs, source.FrameIntervalMs > 0.0);
        ComparisonAdd(frame, "frame::gap", source.FrameGapMs, source.FrameIntervalMs > 0.0);
        ComparisonAdd(frame, "frame::previousCpu", source.PreviousCpuFrameMs, source.FrameIntervalMs > 0.0);
        ComparisonAdd(frame, "frame::gpu", source.GpuFrameMs, source.GpuValid);
        for (size_t i = 0; i < source.CpuWork.size(); ++i)
        {
            const std::string name = Engine::CProfiler::Get_WorkName(static_cast<Engine::EProfilerWork>(i));
            ComparisonAdd(frame, "work.time::" + name, source.CpuWork[i].CpuMs);
            ComparisonAdd(frame, "work.calls::" + name, double(source.CpuWork[i].Calls));
        }
        const auto& animation = source.Animation;
        const bool completeAnimation = animation.DroppedSamples == 0;
        ComparisonAdd(frame, "animation.time::cpuMs", animation.CpuMs, completeAnimation);
        ComparisonAdd(frame, "animation.time::notSubmittedCpuMs", animation.NotSubmittedCpuMs, completeAnimation);
        ComparisonAdd(frame, "animation.count::updateCalls", double(animation.UpdateCalls), completeAnimation);
        ComparisonAdd(frame, "animation.count::updatedModels", double(animation.UpdatedModels), completeAnimation);
        ComparisonAdd(frame, "animation.count::submittedUpdatedModels", double(animation.SubmittedUpdatedModels), completeAnimation);
        ComparisonAdd(frame, "animation.count::notSubmittedUpdatedModels", double(animation.NotSubmittedUpdatedModels), completeAnimation);
        ComparisonAdd(frame, "animation.count::droppedSamples", double(animation.DroppedSamples));
        ComparisonCpu(frame, source.CpuScopes, snapshot.ScopeNames, snapshot.MainThreadId, snapshot.TicksPerSecond,
            source.FrameBeginTick, source.FrameEndTick);
        for (size_t i = 0; i < CounterNames.size(); ++i)
            ComparisonAdd(frame, std::string("counter::") + CounterNames[i], double(source.Counters[i]), ComparisonCounterMeasured(CounterNames[i], TextureCountersProduced));
        const auto& p = source.Pipeline;
        const uint64_t pipeline[] = {p.IAVertices,p.IAPrimitives,p.VSInvocations,p.GSInvocations,p.GSPrimitives,
            p.CInvocations,p.CPrimitives,p.PSInvocations,p.HSInvocations,p.DSInvocations,p.CSInvocations};
        for (size_t i = 0; i < std::size(pipeline); ++i)
            ComparisonAdd(frame, std::string("pipeline::") + ComparisonPipelineNames[i], double(pipeline[i]), source.GpuValid);
        for (const auto& s : source.GpuScopes)
        {
            if (s.NameId >= snapshot.ScopeNames.size()) continue;
            const auto& name = snapshot.ScopeNames[s.NameId];
            ComparisonAdd(frame, "gpu.total::" + name, s.DurationMs, frame.GpuComplete);
            ComparisonAdd(frame, "gpu.self::" + name, s.SelfMs, frame.GpuComplete);
            const uint64_t draw[] = {s.Draw.DrawCalls,s.Draw.InstancedDrawCalls,s.Draw.Instances,s.Draw.Indices,
                s.Draw.MeshDrawCalls,s.Draw.MeshInstances,s.Draw.MeshIndices};
            for (size_t i = 0; i < std::size(draw); ++i)
                ComparisonAdd(frame, std::string("gpu.draw.") + ComparisonDrawNames[i] + "::" + name, double(draw[i]), frame.GpuComplete);
            const uint64_t pass[] = {s.PSInvocations,s.VSInvocations,s.IAVertices,s.IAPrimitives};
            for (size_t i = 0; i < std::size(pass); ++i)
                ComparisonAdd(frame, std::string("gpu.pipeline.") + ComparisonPassPipelineNames[i] + "::" + name,
                    double(pass[i]), frame.GpuComplete && s.PipelineValid);
        }
        result.Frames.push_back(std::move(frame));
    }
    return result;
}

bool_t Client::CProfilerCaptureIO::Parse_Comparison(std::string_view bytes, FProfilerComparisonCapture& out, std::string* error)
{
    try
    {
        DATA_JSON_VALUE root; std::string parseError;
        DATA_JSON_PARSE_LIMITS limits; limits.iMaximumBytes = MAX_COMPARISON_BYTES;
        limits.iMaximumDepth = 24; limits.iMaximumValues = 1500000;
        if (!CDataJson::Parse(bytes, root, parseError, limits))
        { SetError(error, "Cannot parse capture: " + parseError); return false; }
        ComparisonObject(root);
        const auto schema = ComparisonString(ComparisonRequired(root, "schema"));
        if (schema != "LostArkProfilerCapture.v1" && schema != "LostArkProfilerCapture.v2" && schema != "LostArkProfilerCapture.v3")
            throw std::runtime_error("Unsupported profiler capture schema.");
        FProfilerComparisonCapture staged; staged.Source = schema;
        ComparisonMetadata(root, staged);
        const auto textureProduced = ComparisonTextureCapabilities(root);
        std::vector<std::string> names;
        for (const auto& name : ComparisonArrayValue(ComparisonRequired(root, "scopeNames"), 16384))
            names.push_back(ComparisonString(name));
        const auto mainThread = static_cast<uint32_t>(ComparisonUInt(root, "mainThreadId", UINT32_MAX));
        const auto frequency = ComparisonUInt(root, "ticksPerSecond", 1000000000000ull);
        if (!frequency) throw std::runtime_error("Capture clock frequency must be positive.");
        const auto& frames = ComparisonArrayValue(ComparisonRequired(root, "frames"), Engine::CProfiler::MAX_HISTORY_FRAMES);
        if (frames.empty()) throw std::runtime_error("Capture has no completed frames.");
        size_t scopeBudget = 0;
        for (const auto& source : frames)
        {
            ComparisonObject(source); FProfilerComparisonFrame frame;
            frame.Number = ComparisonUInt(source, "frameNumber");
            const uint64_t frameBegin = source.Find("frameBeginTick") ? ComparisonUInt(source, "frameBeginTick") : 0;
            const uint64_t frameEnd = source.Find("frameEndTick") ? ComparisonUInt(source, "frameEndTick") : 0;
            if (source.Find("frameBeginTick") && source.Find("frameEndTick") && frameEnd < frameBegin)
                throw std::runtime_error("Capture frame bounds are reversed.");
            ComparisonReadMemory(source, frame);
            if (!staged.Frames.empty() && staged.Frames.back().Number >= frame.Number)
                throw std::runtime_error("Capture frame numbers must increase strictly.");
            ComparisonAdd(frame, "frame::cpu", ComparisonNumber(ComparisonRequired(source, "cpuFrameMs"), 3600000.0));
            ComparisonOptionalMetric(source, "frameIntervalMs", frame, "frame::interval");
            if (auto it = frame.Values.find("frame::interval"); it != frame.Values.end() && it->second.Value <= 0.0) it->second.Available = false;
            const bool intervalKnown = frame.Values.contains("frame::interval") && frame.Values.at("frame::interval").Available;
            ComparisonOptionalMetric(source, "frameGapMs", frame, "frame::gap", intervalKnown);
            ComparisonOptionalMetric(source, "previousCpuFrameMs", frame, "frame::previousCpu", intervalKnown);
            if (const auto* work = source.Find("cpuWork"))
            {
                std::set<std::string> seen;
                for (const auto& item : ComparisonArrayValue(*work, 256))
                {
                    ComparisonObject(item);
                    const auto name = ComparisonString(ComparisonRequired(item, "name"));
                    if (!seen.insert(name).second) throw std::runtime_error("Capture repeats a fixed CPU work category.");
                    ComparisonOptionalMetric(item, "cpuMs", frame, "work.time::" + name);
                    ComparisonOptionalMetric(item, "calls", frame, "work.calls::" + name, true, true);
                }
            }
            if (const auto* animation = source.Find("animation"))
            {
                ComparisonObject(*animation);
                const auto* dropped = animation->Find("droppedSamples");
                const bool complete = dropped && ComparisonNumber(*dropped, 9007199254740991.0, true) == 0;
                for (const auto* field : {"cpuMs", "notSubmittedCpuMs"})
                    ComparisonOptionalMetric(*animation, field, frame, std::string("animation.time::") + field, complete);
                for (const auto* field : {"updateCalls", "updatedModels", "submittedUpdatedModels", "notSubmittedUpdatedModels", "droppedSamples"})
                    ComparisonOptionalMetric(*animation, field, frame, std::string("animation.count::") + field,
                        std::string_view(field) == "droppedSamples" || complete, true);
            }
            if (const auto* detail = source.Find("detailedCpuScopes")) { frame.DetailKnown = true; frame.Detailed = ComparisonBool(*detail); }
            if (const auto* drop = source.Find("droppedMeshDraws")) frame.DroppedMeshDraws = static_cast<uint64_t>(ComparisonNumber(*drop, 9007199254740991.0, true));
            if (const auto* draws = source.Find("meshDraws"))
            {
                frame.MeshDrawsKnown = frame.DetailKnown && frame.Detailed && source.Find("droppedMeshDraws");
                for (const auto& draw : ComparisonArrayValue(*draws, Engine::CProfiler::MAX_MESH_DRAWS_PER_FRAME))
                {
                    ComparisonObject(draw); FProfilerComparisonMeshDraw item;
                    const auto pass = ComparisonUInt(draw, "passNameId", UINT32_MAX);
                    const auto mesh = ComparisonUInt(draw, "meshNameId", UINT32_MAX);
                    if (mesh >= names.size() || (pass != UINT32_MAX && pass >= names.size()))
                        throw std::runtime_error("Mesh draw display name is out of range.");
                    if (pass != UINT32_MAX) item.Pass = names[static_cast<size_t>(pass)];
                    item.Mesh = names[static_cast<size_t>(mesh)];
                    item.VertexCount = static_cast<uint32_t>(ComparisonUInt(draw, "vertexCount", UINT32_MAX));
                    item.IndexCount = static_cast<uint32_t>(ComparisonUInt(draw, "indexCount", UINT32_MAX));
                    item.Instances = static_cast<uint32_t>(ComparisonUInt(draw, "instances", UINT32_MAX));
                    item.MaterialSlot = static_cast<uint32_t>(ComparisonUInt(draw, "materialSlot", UINT32_MAX));
                    frame.MeshDraws.push_back(std::move(item));
                }
            }
            if (const auto* drop = source.Find("droppedCpuScopes")) frame.CpuSelfKnown = ComparisonNumber(*drop, 9007199254740991.0, true) == 0;
            if (const auto* valid = source.Find("gpuValid")) frame.GpuValid = ComparisonBool(*valid);
            if (const auto* status = source.Find("gpuStatus"))
            {
                const auto text = ComparisonString(*status); bool found = false;
                for (const auto state : {Engine::EProfilerGpuFrameStatus::Unsupported, Engine::EProfilerGpuFrameStatus::Pending,
                    Engine::EProfilerGpuFrameStatus::Valid, Engine::EProfilerGpuFrameStatus::Disjoint,
                    Engine::EProfilerGpuFrameStatus::Dropped, Engine::EProfilerGpuFrameStatus::Error})
                    if (text == GpuStatusName(state)) { frame.GpuStatus = state; found = true; break; }
                if (!found || (frame.GpuValid != (frame.GpuStatus == Engine::EProfilerGpuFrameStatus::Valid)))
                    throw std::runtime_error("Capture GPU validity and status disagree.");
            }
            else if (frame.GpuValid) frame.GpuStatus = Engine::EProfilerGpuFrameStatus::Valid;
            bool supported = false, undropped = false;
            if (const auto* value = source.Find("gpuScopesSupported")) supported = ComparisonBool(*value);
            if (const auto* value = source.Find("droppedGpuScopes")) undropped = ComparisonNumber(*value, UINT32_MAX, true) == 0;
            frame.GpuComplete = frame.GpuValid && supported && undropped && source.Find("gpuScopes");
            ComparisonOptionalMetric(source, "gpuFrameMs", frame, "frame::gpu", frame.GpuValid);
            if (const auto* scopes = source.Find("cpuScopes"))
            {
                frame.CpuScopesKnown = true; std::vector<Engine::FProfilerScopeSample> samples;
                for (const auto& s : ComparisonArrayValue(*scopes, 32768))
                {
                    if (++scopeBudget > 150000) throw std::runtime_error("Capture has too many scope samples for comparison.");
                    ComparisonObject(s); Engine::FProfilerScopeSample sample;
                    sample.NameId = static_cast<uint32_t>(ComparisonUInt(s, "nameId", names.empty() ? 0 : names.size() - 1));
                    if (sample.NameId >= names.size()) throw std::runtime_error("Capture scope name is out of range.");
                    sample.ThreadId = static_cast<uint32_t>(ComparisonUInt(s, "threadId", UINT32_MAX));
                    sample.Depth = static_cast<uint32_t>(ComparisonUInt(s, "depth", 1024));
                    sample.BeginTick = ComparisonUInt(s, "beginTick"); sample.EndTick = ComparisonUInt(s, "endTick");
                    if (sample.EndTick < sample.BeginTick || double(sample.EndTick - sample.BeginTick) / double(frequency) > 3600.0)
                        throw std::runtime_error("Capture CPU scope has an invalid interval.");
                    samples.push_back(sample);
                }
                ComparisonCpu(frame, samples, names, mainThread, frequency, frameBegin, frameEnd);
            }
            else frame.CpuSelfKnown = false;
            for (const auto* group : {"counters", "pipeline"})
                if (const auto* values = source.Find(group))
                {
                    ComparisonObject(*values);
                    const bool isCounter = std::string_view(group) == "counters";
                    if (isCounter)
                        for (const auto* name : CounterNames) ComparisonOptionalMetric(*values, name, frame, std::string("counter::") + name, ComparisonCounterMeasured(name, textureProduced), true);
                    else
                        for (const auto* name : ComparisonPipelineNames) ComparisonOptionalMetric(*values, name, frame, std::string("pipeline::") + name, frame.GpuValid, true);
                }
            if (const auto* scopes = source.Find("gpuScopes"))
                for (const auto& s : ComparisonArrayValue(*scopes, 2048))
                {
                    if (++scopeBudget > 150000) throw std::runtime_error("Capture has too many scope samples for comparison.");
                    ComparisonObject(s);
                    const auto id = ComparisonUInt(s, "nameId", names.empty() ? 0 : names.size() - 1);
                    if (id >= names.size()) throw std::runtime_error("Capture GPU scope name is out of range.");
                    const auto& name = names[static_cast<size_t>(id)];
                    if (const auto* depth = s.Find("depth")) ComparisonNumber(*depth, 1024, true);
                    const auto* begin = s.Find("beginMs"); const auto* end = s.Find("endMs");
                    if (begin) ComparisonNumber(*begin, 3600000.0);
                    if (end) ComparisonNumber(*end, 3600000.0);
                    if (begin && end && end->Get_Number() < begin->Get_Number())
                        throw std::runtime_error("Capture GPU scope has reversed bounds.");
                    const auto* duration = s.Find("durationMs"); const auto* self = s.Find("selfMs");
                    if (duration) ComparisonNumber(*duration, 3600000.0);
                    if (self) ComparisonNumber(*self, 3600000.0);
                    if (duration && self && self->Get_Number() > duration->Get_Number() + 0.00001)
                        throw std::runtime_error("Capture GPU self time exceeds its inclusive interval.");
                    for (const auto* field : {"durationMs", "selfMs"})
                        if (!s.Find(field)) ComparisonAdd(frame, std::string(field == std::string_view("durationMs") ? "gpu.total::" : "gpu.self::") + name, 0.0, false);
                    ComparisonOptionalMetric(s, "durationMs", frame, "gpu.total::" + name, frame.GpuComplete);
                    ComparisonOptionalMetric(s, "selfMs", frame, "gpu.self::" + name, frame.GpuComplete);
                    const auto* draw = s.Find("draw");
                    if (draw) ComparisonObject(*draw);
                    for (const auto* field : ComparisonDrawNames)
                    {
                        if (draw && draw->Find(field))
                            ComparisonOptionalMetric(*draw, field, frame, std::string("gpu.draw.") + field + "::" + name, frame.GpuComplete, true);
                        else ComparisonAdd(frame, std::string("gpu.draw.") + field + "::" + name, 0.0, false);
                    }
                    bool pipelineValid = false;
                    if (const auto* valid = s.Find("pipelineValid")) pipelineValid = ComparisonBool(*valid);
                    for (const auto* field : ComparisonPassPipelineNames)
                        if (s.Find(field)) ComparisonOptionalMetric(s, field, frame, std::string("gpu.pipeline.") + field + "::" + name, frame.GpuComplete && pipelineValid, true);
                        else ComparisonAdd(frame, std::string("gpu.pipeline.") + field + "::" + name, 0.0, false);
                }
            staged.Frames.push_back(std::move(frame));
        }
        out = std::move(staged); if (error) error->clear(); return true;
    }
    catch (const std::exception& exception) { SetError(error, std::string("Cannot import capture: ") + exception.what()); return false; }
}

std::map<std::string, Client::FProfilerComparisonMean> Client::CProfilerCaptureIO::Compare_Means(const FProfilerComparisonCapture& capture)
{
    std::map<std::string, FProfilerComparisonMean> result;
    for (const auto& frame : capture.Frames) for (const auto& [key, value] : frame.Values) result.try_emplace(key);
    for (auto& [key, mean] : result)
    {
        std::set<std::tuple<uint32_t, uint64_t, uint64_t>> memoryObservations;
        const bool memoryMetric = key.starts_with("memory::");
        const bool cpu = key.starts_with("cpu.");
        const bool cpuSelf = cpu && key.find(".self::") != std::string::npos;
        const bool gpuPass = key.starts_with("gpu.");
        const bool gpuFrame = key == "frame::gpu" || key.starts_with("pipeline::");
        for (const auto& frame : capture.Frames)
        {
            if ((gpuPass && !frame.GpuComplete) || (gpuFrame && !frame.GpuValid)) continue;
            const auto it = frame.Values.find(key);
            // First captured interval has no predecessor and is excluded, not zero.
            if ((key == "frame::interval" || key == "frame::gap" || key == "frame::previousCpu") &&
                it != frame.Values.end() && !it->second.Available) continue;
            // Reusing a 1 Hz observation for many render frames must not weight
            // the average toward the periods with higher render FPS.
            if (memoryMetric && frame.MemoryKnown && frame.Memory.Sampled &&
                !memoryObservations.emplace(frame.Memory.ProcessId, frame.Memory.SampleTick, frame.Memory.SampleFrameNumber).second) continue;
            ++mean.Expected;
            if (cpu && (!frame.CpuScopesKnown || (cpuSelf && !frame.CpuSelfKnown))) continue;
            if (it != frame.Values.end())
            {
                if (it->second.Available) { mean.Value += it->second.Value; ++mean.Samples; }
            }
            else if ((cpu && frame.DetailKnown && frame.Detailed && frame.CpuSelfKnown) || gpuPass)
            {
                // Missing field on an observed pass is unmeasured. Only an entirely
                // absent pass in a complete GPU frame has observed zero workload.
                const auto separator = key.find("::");
                const auto name = key.substr(separator + 2);
                if (cpu || !frame.Values.contains("gpu.total::" + name)) ++mean.Samples;
            }
        }
        mean.Available = mean.Expected > 0 && mean.Samples == mean.Expected;
        if (mean.Samples) mean.Value /= double(mean.Samples);
    }
    return result;
}

bool_t Client::CProfilerCaptureIO::Load_Comparison(const filesystem::path& directory,
    const FProfilerCaptureFile& selected, FProfilerComparisonCapture& out, std::string* error)
{
    try
    {
        FCaptureHandle root; filesystem::path finalDirectory;
        if (!OpenCaptureDirectory(directory, root, finalDirectory, error)) return false;
        if (!IsCaptureFileName(selected.FileName)) { SetError(error, "Select a capture filename inside the capture directory."); return false; }
        FCaptureHandle file;
        // Read and validate the same handle; exclude writers and rename until parse finishes.
        file.Value = CreateFileW((directory / selected.FileName).c_str(), GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (file.Value == INVALID_HANDLE_VALUE) return CaptureError(error, "Cannot read selected capture");
        BY_HANDLE_FILE_INFORMATION info{}; filesystem::path finalFile;
        if (!GetFileInformationByHandle(file.Value, &info) || !CaptureFinalPath(file.Value, finalFile, error)) return CaptureError(error, "Cannot inspect selected capture");
        const uint64_t size = (uint64_t(info.nFileSizeHigh) << 32u) | info.nFileSizeLow;
        const uint64_t index = (uint64_t(info.nFileIndexHigh) << 32u) | info.nFileIndexLow;
        if ((info.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
            _wcsicmp(finalFile.parent_path().c_str(), finalDirectory.c_str()) != 0 ||
            info.dwVolumeSerialNumber != selected.VolumeSerial || index != selected.FileIndex ||
            size != selected.SizeBytes || CaptureTicks(info.ftLastWriteTime) != selected.LastWriteTicks)
        { SetError(error, "Selected capture changed. Refresh the list and select it again."); return false; }
        if (size == 0 || size > MAX_COMPARISON_BYTES)
        { SetError(error, "Comparison import requires a nonempty capture of at most 32 MiB."); return false; }
        std::string bytes(static_cast<size_t>(size), '\0'); DWORD read = 0;
        if (!ReadFile(file.Value, bytes.data(), static_cast<DWORD>(size), &read, nullptr) || read != size)
            return CaptureError(error, "Cannot read entire capture");
        FProfilerComparisonCapture staged;
        if (!Parse_Comparison(bytes, staged, error)) return false;
        staged.Source = selected.DisplayName; staged.Label = selected.DisplayName;
        out = std::move(staged); if (error) error->clear(); return true;
    }
    catch (const std::exception& exception) { SetError(error, std::string("Cannot load capture: ") + exception.what()); return false; }
}
