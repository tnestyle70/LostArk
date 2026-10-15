#pragma once

#include "Profiler.h"
#include "MapGeometryDiagnostics.h"
#include <string>
#include <vector>

namespace Client
{
// ProfilerTool이 메인 스레드에서 완성하고 기존 exporter에 불변 소유권으로 넘긴다.
// 예약 tick은 관측 주기다. 늦은 프레임에서 가짜 중간 pose를 재생하거나 보간하지 않는다.
struct FProfilerCutsceneSample final
{
    uint64_t GeometryFrame = 0, Tick = 0, SkippedTicks = 0;
    double ScheduledWallMs = 0, ObservedWallMs = 0, CueSeconds = 0;
    double FrameIntervalMs = 0, CollectionCpuMs = 0;
    bool GeometryResolved = false;
    Engine::FProfilerGeometryStats Geometry{};
    std::vector<Engine::FProfilerIndirectArgument> IndirectArguments;
    MAP_GEOMETRY_DIAGNOSTIC_SNAPSHOT Map;
};

struct FProfilerCutsceneCapture final
{
    int IntervalMs = 100;
    float MinimumDistance = 0.f, MaximumDistance = 250.f;
    double CueDurationSeconds = 0, ElapsedWallMs = 0;
    bool StartedMidCue = false;
    uint64_t SourceRows = 0, SkippedTicks = 0;
    std::string StopReason;
    std::vector<FProfilerCutsceneSample> Samples;
};
}
