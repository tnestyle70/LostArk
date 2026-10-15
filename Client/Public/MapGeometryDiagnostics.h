#pragma once

#include "Client_Defines.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace Client
{

// Load_Area의 선택적 대체 준비 결과다. 성공한 Level이 불변 공유 소유하며,
// F7 저장 시 기존 비동기 writer가 이 값을 차용하지 않고 shared_ptr로 보유한다.
struct MAP_COLOR_PROXY_PREPARATION_DIAGNOSTICS final
{
    struct FAILURE final
    {
        std::string assetId, reason;
        int32_t hresult = 0;
    };
    static constexpr size_t MaximumFailures = 128u;
    uint64_t totalBatches = 0u, unsupportedBatches = 0u, supportedBatches = 0u;
    uint64_t preparedBatches = 0u, rejectedBatches = 0u, failedBatches = 0u, droppedFailures = 0u;
    std::string indexError, stagingError;
    std::vector<FAILURE> failures;

    // 진단 메모리 부족이 맵 로드·원본 표시의 성공을 실패로 바꾸지 않도록 손실만 기록한다.
    void RecordFailure(const std::string& assetId, HRESULT result, const std::string& reason) noexcept
    {
        if (failures.size() >= MaximumFailures)
        {
            ++droppedFailures;
            return;
        }
        try
        {
            failures.push_back({ assetId, reason.substr(0u, 2048u), static_cast<int32_t>(result) });
        }
        catch (...)
        {
            ++droppedFailures;
        }
    }
};

// Profiler의 명시적 표본 프레임에만 생성한다. Level의 static batch frame state가
// main thread에서 소유하며 Begin/End 사이의 실제 제출을 수집한다. 렌더 선택을 바꾸지 않는다.
struct MAP_GEOMETRY_DIAGNOSTIC_DRAW final
{
    uint64_t drawId = 0u;
    std::string path, assetId;
    uint32_t meshIndex = 0u, lod = 0u;
    uint64_t indexCount = 0u, instanceCount = 0u, vertexBufferCount = 0u;
    // GPU indirect는 CPU가 instanceCount를 모른다. 0은 pending 기본값이며
    // instanceCountKnown=false일 때 측정값 0으로 표시하지 않는다.
    bool instanceCountKnown = true;
    uint32_t argsByteOffset = UINT32_MAX, cohortId = UINT32_MAX, bin = UINT32_MAX;
    uint64_t gpuFrameId = 0u;
    uint64_t geometryFrameNumber = 0u;
    uint32_t indirectBufferOrdinal = UINT32_MAX;
    // visible index 추적이 실제 제출 payload와 일치할 때만 source ID를 확정한다.
    bool sourceIdsKnown = false;
    std::vector<uint64_t> sourcePlacementIds;
};

// distance는 submesh 표면 최단거리가 아니라 placement/model bounding sphere 기준이다.
// lodDiagnostic은 현재 CPU envelope를 읽어 얻은 진단값이며 GPU 선택 결과가 아니다.
struct MAP_GEOMETRY_DIAGNOSTIC_SOURCE final
{
    uint64_t placementId = 0u;
    std::string assetId, route;
    uint32_t meshIndex = 0u;
    uint32_t gpuCohortId = UINT32_MAX;
    std::array<float, 3> boundsCenter{};
    float radius = 0.f;
    double centerDistance = 0., nearDistance = 0.;
    bool authoredVisible = false, suppressed = false, cameraPreviewSuppressed = false;
    bool cpuVisibilityPrepared = false, cpuVisibleKnown = false, cpuVisible = false;
    bool gpuSubmitted = false, hlodClaimed = false;
    bool lodDiagnosticKnown = false;
    uint32_t lodDiagnostic = 0u;
    uint64_t baseIndexCount = 0u, diagnosticIndexCount = 0u, vertexBufferCount = 0u;
};

struct MAP_GEOMETRY_DIAGNOSTIC_SNAPSHOT final
{
    bool captured = false, complete = false, cameraValid = false, distanceRangeValid = false;
    uint64_t frameNumber = 0u, cameraRevision = 0u;
    uint32_t levelId = 0u;
    std::string areaId;
    std::array<float, 3> cameraPosition{};
    std::array<float, 16> view{}, projection{};
    std::array<float, 2> viewport{};
    float minDistance = 0.f, maxDistance = 0.f;
    uint64_t staticBatchCount = 0u, staticPlacementCount = 0u, fallbackPlacementCount = 0u;
    uint64_t consideredSources = 0u, distanceFilteredSources = 0u, invalidBoundsSources = 0u;
    uint64_t droppedSources = 0u, droppedDraws = 0u;
    std::vector<MAP_GEOMETRY_DIAGNOSTIC_SOURCE> sources;
    std::vector<MAP_GEOMETRY_DIAGNOSTIC_DRAW> draws;
};

struct MAP_GEOMETRY_DIAGNOSTIC_FRAME final
{
    static constexpr size_t MaximumSources = 20000u;
    static constexpr size_t MaximumDraws = 20000u;
    uint64_t droppedDraws = 0u;
    std::vector<MAP_GEOMETRY_DIAGNOSTIC_DRAW> draws;
    // 진단 할당 실패가 제품 draw의 성공을 실패로 바꾸지 않도록 손실만 기록한다.
    void Record(MAP_GEOMETRY_DIAGNOSTIC_DRAW&& draw) noexcept
    {
        if (draws.size() >= MaximumDraws) { ++droppedDraws; return; }
        try { draw.drawId = draws.size(); draws.push_back(std::move(draw)); }
        catch (...) { ++droppedDraws; }
    }
};

}
