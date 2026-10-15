// ProfilerCaptureIO.cpp의 익명 namespace에서 포함한다. 별도 저장 스레드나 파일 경로를
// 만들지 않고 기존 writer가 소유한 불변 컷신 결과를 선택적 JSON 항목으로 기록한다.
// 모든 helper는 writer 스레드에서만 호출되며 카메라·Map·GPU 자원에 접근하지 않는다.

using FCutsceneSourceKey = std::tuple<uint64_t, std::string, uint32_t>;
struct FCutscenePreviousDistance final
{
    uint64_t Tick = 0, GeometryFrame = 0;
    double ObservedWallMs = 0., CenterDistance = 0.;
};

const char* CutsceneJsonBool(bool value) noexcept { return value ? "true" : "false"; }

// 정수 ID는 JSON 숫자로 그대로 보존한다. sourcePlacementIds는 draw가 실제로 참조한
// 원본 placement의 연결 정보이며 GPU 생존 판정이나 화면 픽셀의 가시성을 뜻하지 않는다.
void WriteCutsceneIds(std::ostream& stream, const std::vector<uint64_t>& values)
{
    stream << '[';
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (i) stream << ',';
        stream << values[i];
    }
    stream << ']';
}

// 표본마다 같은 stable source key의 이전 관측을 찾는다. 거리 범위 밖에서 빠진 tick을
// 가짜 표본으로 보간하지 않고 실제 이전 관측 시각과의 차이만 worker에서 계산한다.
bool WriteCutsceneSource(std::ostream& stream,
    const Client::MAP_GEOMETRY_DIAGNOSTIC_SOURCE& source,
    const Client::FProfilerCutsceneSample& sample,
    std::map<FCutsceneSourceKey, FCutscenePreviousDistance>& previous,
    string* error)
{
    if (!std::isfinite(source.radius) || source.radius < 0.f ||
        !std::isfinite(source.centerDistance) || source.centerDistance < 0. ||
        !std::isfinite(source.nearDistance) || source.nearDistance < 0. ||
        !std::all_of(source.boundsCenter.begin(), source.boundsCenter.end(),
            [](float value) { return std::isfinite(value); }))
    { SetError(error, "Cutscene source has invalid model bounds or distance."); return false; }
    const FCutsceneSourceKey key{source.placementId, source.assetId, source.meshIndex};
    const auto prior = previous.find(key);
    const bool hasPrevious = prior != previous.end() &&
        sample.ObservedWallMs > prior->second.ObservedWallMs;
    const double deltaTime = hasPrevious ? sample.ObservedWallMs - prior->second.ObservedWallMs : 0.;
    const double distanceDelta = hasPrevious ? source.centerDistance - prior->second.CenterDistance : 0.;
    const double distanceRate = hasPrevious ? distanceDelta / (deltaTime * 0.001) : 0.;
    if (hasPrevious && (!std::isfinite(deltaTime) || !std::isfinite(distanceDelta) || !std::isfinite(distanceRate)))
    { SetError(error, "Cutscene source distance delta is not finite."); return false; }
    stream << "{\"placementId\": " << source.placementId
        << ", \"assetId\": \"" << EscapeJson(source.assetId)
        << "\", \"meshIndex\": " << source.meshIndex
        << ", \"route\": \"" << EscapeJson(source.route) << "\", \"boundsCenter\": ";
    WriteFloatArray(stream, source.boundsCenter);
    stream << ", \"radius\": " << source.radius
        << ", \"centerDistance\": " << source.centerDistance
        << ", \"nearDistance\": " << source.nearDistance
        << ", \"previousObservedTick\": ";
    if (hasPrevious) stream << prior->second.Tick; else stream << "null";
    stream << ", \"previousGeometryFrame\": ";
    if (hasPrevious) stream << prior->second.GeometryFrame; else stream << "null";
    stream << ", \"previousObservedWallMs\": ";
    if (hasPrevious) stream << prior->second.ObservedWallMs; else stream << "null";
    stream << ", \"distanceDeltaMeters\": ";
    if (hasPrevious) stream << distanceDelta; else stream << "null";
    stream << ", \"deltaTimeMs\": ";
    if (hasPrevious) stream << deltaTime; else stream << "null";
    stream << ", \"distanceRateMps\": ";
    if (hasPrevious) stream << distanceRate;
    else stream << "null";
    stream << ", \"authoredVisible\": " << CutsceneJsonBool(source.authoredVisible)
        << ", \"suppressed\": " << CutsceneJsonBool(source.suppressed)
        << ", \"cameraPreviewSuppressed\": " << CutsceneJsonBool(source.cameraPreviewSuppressed)
        << ", \"cpuVisibilityPrepared\": " << CutsceneJsonBool(source.cpuVisibilityPrepared)
        << ", \"cpuVisibleKnown\": " << CutsceneJsonBool(source.cpuVisibleKnown)
        << ", \"cpuVisible\": ";
    if (source.cpuVisibleKnown) stream << CutsceneJsonBool(source.cpuVisible); else stream << "null";
    stream << ", \"gpuSubmitted\": " << CutsceneJsonBool(source.gpuSubmitted)
        << ", \"gpuCohortId\": ";
    if (source.gpuCohortId != UINT32_MAX) stream << source.gpuCohortId; else stream << "null";
    stream << ", \"hlodClaimed\": " << CutsceneJsonBool(source.hlodClaimed)
        << ", \"lodDiagnosticKnown\": " << CutsceneJsonBool(source.lodDiagnosticKnown)
        << ", \"lodDiagnostic\": ";
    if (source.lodDiagnosticKnown) stream << source.lodDiagnostic; else stream << "null";
    stream << ", \"baseIndexCount\": " << source.baseIndexCount
        << ", \"diagnosticIndexCount\": ";
    if (source.lodDiagnosticKnown) stream << source.diagnosticIndexCount; else stream << "null";
    stream << ", \"vertexBufferCount\": " << source.vertexBufferCount << '}';
    previous[key] = {sample.Tick, sample.GeometryFrame, sample.ObservedWallMs, source.centerDistance};
    return true;
}

// 이 상태는 draw 선택 전후의 카메라와 원본 model sphere를 설명한다. observed draw는
// 별도 배열에 기록하므로 source의 CPU envelope를 실제 GPU LOD나 가시 개수로 혼동하지 않는다.
bool WriteCutsceneMap(std::ostream& stream, const Client::FProfilerCutsceneSample& sample,
    std::map<FCutsceneSourceKey, FCutscenePreviousDistance>& previous,
    const std::atomic_bool* cancel, string* error)
{
    const auto& map = sample.Map;
    if (!std::isfinite(map.minDistance) || !std::isfinite(map.maxDistance) ||
        map.minDistance < 0.f || map.maxDistance < map.minDistance)
    { SetError(error, "Cutscene map distance range is invalid."); return false; }
    stream << "{\"captured\": " << CutsceneJsonBool(map.captured)
        << ", \"complete\": " << CutsceneJsonBool(map.complete)
        << ", \"cameraValid\": " << CutsceneJsonBool(map.cameraValid)
        << ", \"distanceRangeValid\": " << CutsceneJsonBool(map.distanceRangeValid)
        << ", \"frameNumber\": " << map.frameNumber
        << ", \"cameraRevision\": " << map.cameraRevision
        << ", \"levelId\": " << map.levelId
        << ", \"areaId\": \"" << EscapeJson(map.areaId) << "\", \"cameraPosition\": ";
    WriteFloatArray(stream, map.cameraPosition);
    stream << ", \"view\": "; WriteFloatArray(stream, map.view);
    stream << ", \"projection\": "; WriteFloatArray(stream, map.projection);
    stream << ", \"viewport\": "; WriteFloatArray(stream, map.viewport);
    stream << ", \"minimumDistance\": " << map.minDistance
        << ", \"maximumDistance\": " << map.maxDistance
        << ", \"staticBatchCount\": " << map.staticBatchCount
        << ", \"staticPlacementCount\": " << map.staticPlacementCount
        << ", \"fallbackPlacementCount\": " << map.fallbackPlacementCount
        << ", \"consideredSources\": " << map.consideredSources
        << ", \"distanceFilteredSources\": " << map.distanceFilteredSources
        << ", \"invalidBoundsSources\": " << map.invalidBoundsSources
        << ", \"droppedSources\": " << map.droppedSources
        << ", \"droppedDraws\": " << map.droppedDraws
        << ", \"sources\": [\n";
    for (size_t i = 0; i < map.sources.size(); ++i)
    {
        if (i % 256 == 0 && Cancelled(cancel, error)) return false;
        stream << "          ";
        if (!WriteCutsceneSource(stream, map.sources[i], sample, previous, error)) return false;
        stream << (i + 1 < map.sources.size() ? ",\n" : "\n");
    }
    stream << "        ]}";
    return true;
}

// pipeline query의 VS 호출 수와 Engine이 계측한 indexed 제출량은 범위가 다르다.
// 두 validity를 따로 기록하고 미회수 결과를 0으로 바꾸지 않는다. pending은 GPU 대기 상태다.
void WriteCutsceneGeometryStats(std::ostream& stream, const Client::FProfilerCutsceneSample& sample)
{
    const auto& stats = sample.Geometry;
    stream << "{\"resolved\": " << CutsceneJsonBool(sample.GeometryResolved)
        << ", \"frameNumber\": " << stats.FrameNumber
        << ", \"currentFrameNumberAtResolve\": " << stats.CurrentFrameNumber
        << ", \"captureFrameNumber\": " << stats.CaptureFrameNumber
        << ", \"latencyFrames\": " << stats.LatencyFrames
        << ", \"droppedFrames\": " << stats.DroppedFrames
        << ", \"pending\": " << CutsceneJsonBool(!sample.GeometryResolved || stats.Pending)
        << ", \"vertexValid\": " << CutsceneJsonBool(sample.GeometryResolved && stats.VertexValid)
        << ", \"indexValid\": " << CutsceneJsonBool(sample.GeometryResolved && stats.IndexValid)
        << ", \"vsInvocations\": ";
    if (sample.GeometryResolved && stats.VertexValid) stream << stats.VSInvocations; else stream << "null";
    stream << ", \"directIndices\": ";
    if (sample.GeometryResolved) stream << stats.DirectIndices; else stream << "null";
    stream << ", \"indirectIndices\": ";
    if (sample.GeometryResolved && stats.IndexValid) stream << stats.IndirectIndices; else stream << "null";
    stream << ", \"totalIndices\": ";
    if (sample.GeometryResolved && stats.IndexValid) stream << stats.TotalIndices; else stream << "null";
    stream << ", \"indirectDrawCalls\": ";
    if (sample.GeometryResolved) stream << stats.IndirectDrawCalls; else stream << "null";
    stream << '}';
}

using FCutsceneIndirectKey = std::tuple<uint64_t, uint32_t, uint32_t>;

// 실제 args는 Engine이 비동기로 회수한 같은 geometry frame의 값이다. buffer ordinal은
// frame 안에서만 유효한 연결 ID이며 주소가 아니다. 하나의 args를 여러 draw가 썼으면
// DrawCount에 나타나지만 각 map draw 행은 단일 제출이므로 다시 곱하지 않는다.
bool WriteCutsceneDraws(std::ostream& stream, const Client::FProfilerCutsceneSample& sample,
    const std::atomic_bool* cancel, string* error)
{
    std::map<FCutsceneIndirectKey, const Engine::FProfilerIndirectArgument*> arguments;
    stream << "[\n";
    for (size_t i = 0; i < sample.IndirectArguments.size(); ++i)
    {
        if (i % 256 == 0 && Cancelled(cancel, error)) return false;
        const auto& argument = sample.IndirectArguments[i];
        const FCutsceneIndirectKey key{sample.GeometryFrame, argument.BufferOrdinal, argument.ArgsByteOffset};
        if (!arguments.emplace(key, &argument).second)
        { SetError(error, "Cutscene indirect argument identity is duplicated."); return false; }
        stream << "        {\"geometryFrameNumber\": " << sample.GeometryFrame
            << ", \"bufferOrdinal\": " << argument.BufferOrdinal
            << ", \"argsByteOffset\": " << argument.ArgsByteOffset
            << ", \"indexCount\": " << argument.IndexCount
            << ", \"instanceCount\": " << argument.InstanceCount
            << ", \"drawCount\": " << argument.DrawCount << '}';
        stream << (i + 1 < sample.IndirectArguments.size() ? ",\n" : "\n");
    }
    stream << "      ],\n      \"mapDraws\": [\n";
    for (size_t i = 0; i < sample.Map.draws.size(); ++i)
    {
        if (i % 256 == 0 && Cancelled(cancel, error)) return false;
        const auto& draw = sample.Map.draws[i];
        const auto argument = arguments.find(FCutsceneIndirectKey{
            draw.geometryFrameNumber, draw.indirectBufferOrdinal, draw.argsByteOffset});
        const bool fromArguments = !draw.instanceCountKnown && argument != arguments.end();
        const bool instanceKnown = draw.instanceCountKnown || fromArguments;
        const uint64_t instances = fromArguments ? argument->second->InstanceCount : draw.instanceCount;
        const uint64_t indices = fromArguments ? argument->second->IndexCount : draw.indexCount;
        if (instanceKnown && instances != 0u && indices > UINT64_MAX / instances)
        { SetError(error, "Cutscene draw index total overflowed."); return false; }
        stream << "        {\"drawId\": " << draw.drawId
            << ", \"path\": \"" << EscapeJson(draw.path)
            << "\", \"assetId\": \"" << EscapeJson(draw.assetId)
            << "\", \"meshIndex\": " << draw.meshIndex
            << ", \"lod\": ";
        if (draw.lod != UINT32_MAX) stream << draw.lod; else stream << "null";
        stream << ", \"indexCount\": " << draw.indexCount
            << ", \"vertexBufferCount\": " << draw.vertexBufferCount
            << ", \"instanceCountKnown\": " << CutsceneJsonBool(instanceKnown)
            << ", \"instanceCountSource\": \""
            << (fromArguments ? "gpu-argument-readback" : draw.instanceCountKnown ? "cpu-submission" : "unresolved")
            << "\", \"instanceCount\": ";
        if (instanceKnown) stream << instances; else stream << "null";
        stream << ", \"resolvedIndexCountPerInstance\": ";
        if (instanceKnown) stream << indices; else stream << "null";
        stream << ", \"submittedIndices\": ";
        if (instanceKnown) stream << indices * instances; else stream << "null";
        stream << ", \"argsByteOffset\": ";
        if (draw.argsByteOffset != UINT32_MAX) stream << draw.argsByteOffset; else stream << "null";
        stream << ", \"cohortId\": ";
        if (draw.cohortId != UINT32_MAX) stream << draw.cohortId; else stream << "null";
        stream << ", \"bin\": ";
        if (draw.bin != UINT32_MAX) stream << draw.bin; else stream << "null";
        stream << ", \"gpuFrameId\": " << draw.gpuFrameId
            << ", \"geometryFrameNumber\": " << draw.geometryFrameNumber
            << ", \"indirectBufferOrdinal\": ";
        if (draw.indirectBufferOrdinal != UINT32_MAX) stream << draw.indirectBufferOrdinal; else stream << "null";
        stream << ", \"sourceIdsKnown\": " << CutsceneJsonBool(draw.sourceIdsKnown)
            << ", \"sourcePlacementIds\": ";
        if (draw.sourceIdsKnown) WriteCutsceneIds(stream, draw.sourcePlacementIds); else stream << "null";
        stream << '}' << (i + 1 < sample.Map.draws.size() ? ",\n" : "\n");
    }
    stream << "      ]";
    return true;
}

// SaveJsonImpl이 기본 raw capture 항목 뒤에 호출한다. 첨부가 없으면 기존 JSON 형태를
// 유지한다. 잘못된 수치·취소·I/O 실패는 false로 상위 writer에 전파되어 temp만 정리되고
// 기존 저장본은 보존된다. 거리 변화 계산은 worker의 지역 map만 변경한다.
bool WriteCutsceneGeometry(std::ostream& stream, const Client::FProfilerCaptureContext& context,
    const std::atomic_bool* cancel, string* error)
{
    if (!context.CutsceneGeometry) return true;
    if (Cancelled(cancel, error)) return false;
    const auto& capture = *context.CutsceneGeometry;
    if (capture.IntervalMs <= 0 || !std::isfinite(capture.MinimumDistance) ||
        !std::isfinite(capture.MaximumDistance) || capture.MinimumDistance < 0.f ||
        capture.MaximumDistance < capture.MinimumDistance ||
        !std::isfinite(capture.CueDurationSeconds) || capture.CueDurationSeconds < 0. ||
        !std::isfinite(capture.ElapsedWallMs) || capture.ElapsedWallMs < 0.)
    { SetError(error, "Cutscene capture timing or distance range is invalid."); return false; }
    stream << "  \"bernCutsceneGeometry\": {\n"
        << "    \"schema\": \"LostArkBernCutsceneGeometry.v1\",\n"
        << "    \"intervalMs\": " << capture.IntervalMs
        << ",\n    \"minimumDistance\": " << capture.MinimumDistance
        << ",\n    \"maximumDistance\": " << capture.MaximumDistance
        << ",\n    \"cueDurationSeconds\": " << capture.CueDurationSeconds
        << ",\n    \"elapsedWallMs\": " << capture.ElapsedWallMs
        << ",\n    \"startedMidCue\": " << CutsceneJsonBool(capture.StartedMidCue)
        << ",\n    \"sourceRows\": " << capture.SourceRows
        << ",\n    \"skippedTicks\": " << capture.SkippedTicks
        << ",\n    \"stopReason\": \"" << EscapeJson(capture.StopReason)
        << "\",\n    \"measurementSemantics\": {\n"
        << "      \"sampling\": \"Fixed wall-time schedule, observing an actual rendered frame at or after each due tick. Missed ticks are counted, never synthesized. cueSeconds uses the existing cutscene playback clock, which can advance differently from wall time. cutsceneSamples are retained separately from the raw v3 history ring and are not restricted by the raw JSON frame window.\",\n"
        << "      \"vertices\": \"vsInvocations is the whole-frame GPU vertex shader invocation count, including passes and internal UI/DirectXTK draws within the query. It is not a unique vertex-buffer count, resident vertices, or final visible geometry.\",\n"
        << "      \"indices\": \"directIndices and indirectIndices cover instrumented Engine indexed submissions, including repeated passes and instance multiplication; totalIndices is their sum only when indexValid. They exclude uninstrumented DirectXTK and separately rendered ImGui draws. These are a different coverage from whole-frame vsInvocations.\",\n"
        << "      \"distance\": \"Meters from the observed camera to each placement/model bounding sphere: centerDistance to its center, nearDistance to its nearest sphere surface. These are not submesh surface distances. Source rows use centerDistance for the requested interval filter; mapDraws retain all instrumented static-map color submissions before that filter.\",\n"
        << "      \"distanceDelta\": \"Stable key (placementId, assetId, meshIndex). Differences use the previous observed sample of that key, which may precede skipped or filtered ticks. First observation has null previous tick, delta and rate. Negative rate means approaching the model sphere center.\",\n"
        << "      \"lod\": \"sources.lodDiagnostic and diagnosticIndexCount are CPU envelope diagnostics only. GPU source-level surviving LOD is not observed. mapDraws.lod describes the submitted draw/bin; a native HLOD path without one LOD number exports null. vertexBufferCount is the bound source VB vertex count, which need not shrink with an index-only LOD. instanceCount is actual only when known or joined with same-frame indirect args.\",\n"
        << "      \"indirectJoin\": \"geometryFrameNumber + indirectBufferOrdinal/bufferOrdinal + argsByteOffset. Ordinals are ephemeral frame-local IDs, never pointers. One map draw uses one instanceCount; argument.drawCount explains repeated use of the same args and must not be multiplied into every map row. Unknown values are null.\",\n"
        << "      \"sourceJoin\": \"sourceIdsKnown is true only when all submitted source placement IDs are observed. GPU draws leave source IDs unknown; source.gpuCohortId and meshIndex join only the cohort/bin aggregate, not individual surviving placements. draw.assetId is a representative asset and is not an exhaustive asset list for a merged draw.\",\n"
        << "      \"frameInterval\": \"frameIntervalMs measures consecutive diagnostic render boundaries before world rendering, while geometry belongs to this rendered frame. It is distinct from raw v3 profiler Begin-to-Begin timing. GPU results may resolve later; currentFrameNumberAtResolve and latencyFrames identify that delay.\"\n"
        << "    },\n    \"cutsceneSamples\": [\n";
    std::map<FCutsceneSourceKey, FCutscenePreviousDistance> previous;
    double previousWallMs = -1.;
    for (size_t i = 0; i < capture.Samples.size(); ++i)
    {
        if (Cancelled(cancel, error)) return false;
        const auto& sample = capture.Samples[i];
        if (!std::isfinite(sample.ScheduledWallMs) || sample.ScheduledWallMs < 0. ||
            !std::isfinite(sample.ObservedWallMs) || sample.ObservedWallMs < 0. ||
            sample.ObservedWallMs < previousWallMs ||
            !std::isfinite(sample.CueSeconds) || sample.CueSeconds < 0. ||
            !std::isfinite(sample.FrameIntervalMs) || sample.FrameIntervalMs < 0. ||
            !std::isfinite(sample.CollectionCpuMs) || sample.CollectionCpuMs < 0.)
        { SetError(error, "Cutscene sample timing is invalid or out of order."); return false; }
        previousWallMs = sample.ObservedWallMs;
        stream << "      {\"geometryFrame\": " << sample.GeometryFrame
            << ", \"tick\": " << sample.Tick
            << ", \"skippedTicks\": " << sample.SkippedTicks
            << ", \"scheduledWallMs\": " << sample.ScheduledWallMs
            << ", \"observedWallMs\": " << sample.ObservedWallMs
            << ", \"cueSeconds\": " << sample.CueSeconds
            << ", \"frameIntervalMs\": " << sample.FrameIntervalMs
            << ", \"collectionCpuMs\": " << sample.CollectionCpuMs
            << ",\n      \"geometry\": ";
        WriteCutsceneGeometryStats(stream, sample);
        stream << ",\n      \"map\": ";
        if (!WriteCutsceneMap(stream, sample, previous, cancel, error)) return false;
        stream << ",\n      \"indirectArguments\": ";
        if (!WriteCutsceneDraws(stream, sample, cancel, error)) return false;
        stream << "\n      }" << (i + 1 < capture.Samples.size() ? ",\n" : "\n");
    }
    stream << "    ]\n  },\n";
    if (!stream)
    { SetError(error, "Failed while writing cutscene geometry JSON."); return false; }
    return true;
}
