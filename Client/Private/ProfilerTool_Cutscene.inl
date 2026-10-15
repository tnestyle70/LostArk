// ProfilerTool.cpp가 포함한다. 창을 닫아도 MainApp의 Update/Render 연결은 살아 있다.
#include "ProfilerCutsceneCapture.h"
#include <chrono>
#include <exception>

struct Client::CProfilerTool::FCutsceneState final
{
    using Clock = std::chrono::steady_clock;
    int intervalMs = 100;
    float minimumDistance = 0.f, maximumDistance = 250.f;
    bool waiting = false, recording = false, draining = false, frameArmed = false;
    bool saveRequested = false;
    // 메인 스레드 전용 상태. 단일 exporter의 현재 작업이 거리 표본을 소유하는지 구분한다.
    bool writerActive = false;
    size_t saveFrameWindow = Engine::CProfiler::MAX_HISTORY_FRAMES;
    const char* saveStatusFallback = nullptr; // 메모리 예외 때도 표시 가능한 정적 문자열만 차용한다.
    uint64_t nextTick = 0, armedTick = 0, armedSkipped = 0;
    double armedWallMs = 0, armedCue = 0, previousFrameMs = 0, frameIntervalMs = 0;
    Clock::time_point requestedAt{}, startedAt{}, stoppedAt{}, previousFrameAt{};
    std::shared_ptr<FProfilerCutsceneCapture> data;
    std::shared_ptr<const FProfilerCutsceneCapture> completed;
    std::string status;
};

bool Client::CProfilerTool::Is_CutsceneRecording() const
{
    return m_Cutscene && (m_Cutscene->waiting || m_Cutscene->recording);
}

// Profiler 버튼만 replay를 요청한다. 이 메서드는 이미 결정된 카메라를 관측한다.
// Render 직전 arm하여 같은 프레임의 실제 draw와 source distance를 한 번호로 연결한다.
void Client::CProfilerTool::Begin_CutsceneDiagnosticFrame() try
{
    if (!m_Cutscene) return;
    auto& state = *m_Cutscene;
    if (!state.waiting && !state.recording) return;
    auto* bern = CLevel_Bern::Get_Active();
    if (CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::BERN) || !bern)
    { Stop_CutsceneCapture("level_changed"); return; }
    const auto now = FCutsceneState::Clock::now();
    if (!bern->Is_EntranceCinematicActive())
    {
        if (state.recording) Stop_CutsceneCapture(
            bern->Get_EntranceCinematicSeconds() >= bern->Get_EntranceCinematicDurationSeconds() ?
                "cue_finished" : "cue_interrupted");
        else if (std::chrono::duration<double>(now - state.requestedAt).count() > 5.)
            Stop_CutsceneCapture("replay_not_started");
        return;
    }
    if (state.waiting)
    {
        state.waiting = false; state.recording = true; state.startedAt = now;
        state.previousFrameAt = now; state.frameIntervalMs = 0.;
        state.data->CueDurationSeconds = bern->Get_EntranceCinematicDurationSeconds();
        state.data->StartedMidCue = bern->Get_EntranceCinematicSeconds() > 0.1f;
    }
    state.frameIntervalMs = std::chrono::duration<double, std::milli>(now - state.previousFrameAt).count();
    state.previousFrameAt = now;
    const double elapsed = std::chrono::duration<double, std::milli>(now - state.startedAt).count();
    if (elapsed < static_cast<double>(state.nextTick) * state.data->IntervalMs) return;
    // 120초/200만source/1200표본을 넘어 메모리를 계속 늘리지 않는다. 누락을 성공으로 숨기지 않는다.
    if (elapsed > 120000. || state.data->Samples.size() >= 1200u || state.data->SourceRows >= 2'000'000u)
    { Stop_CutsceneCapture("capture_budget_reached"); return; }
    const uint64_t tick = static_cast<uint64_t>(elapsed / state.data->IntervalMs);
    state.armedTick = tick; state.armedSkipped = tick - state.nextTick;
    state.nextTick = tick + 1u; state.armedWallMs = elapsed;
    state.armedCue = bern->Get_EntranceCinematicSeconds();
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
        profiler->Request_GeometryDetailForCurrentFrame();
    bern->Get_GeometryDiagnosticRuntime().Begin_GeometryDiagnosticFrame();
    state.frameArmed = true;
}
catch (...)
{
    // 진단 메모리 실패는 월드 draw 실패로 전파하지 않는다.
    if (auto* bern = CLevel_Bern::Get_Active()) bern->Get_GeometryDiagnosticRuntime().Cancel_GeometryDiagnosticFrame();
    try { Stop_CutsceneCapture("diagnostic_allocation_failed"); } catch (...) { }
}

// world render가 끝난 뒤만 읽는다. 실패한 draw나 GPU 후보를 실제 생존 geometry로 승격하지 않는다.
void Client::CProfilerTool::End_CutsceneDiagnosticFrame(bool renderSucceeded) try
{
    if (!m_Cutscene || !m_Cutscene->frameArmed) return;
    auto& state = *m_Cutscene;
    state.frameArmed = false;
    auto* bern = CLevel_Bern::Get_Active();
    if (!bern || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::BERN))
    { Stop_CutsceneCapture("level_changed"); return; }
    if (!renderSucceeded)
    {
        bern->Get_GeometryDiagnosticRuntime().Cancel_GeometryDiagnosticFrame();
        Stop_CutsceneCapture("world_render_failed"); return;
    }
    const auto begin = FCutsceneState::Clock::now();
    FProfilerCutsceneSample sample;
    sample.Tick = state.armedTick; sample.SkippedTicks = state.armedSkipped;
    sample.ScheduledWallMs = static_cast<double>(sample.Tick) * state.data->IntervalMs;
    sample.ObservedWallMs = state.armedWallMs; sample.CueSeconds = state.armedCue;
    sample.FrameIntervalMs = state.frameIntervalMs;
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
        sample.GeometryFrame = profiler->Get_GeometryFrameNumber();
    sample.Map = bern->Get_GeometryDiagnosticRuntime().End_GeometryDiagnosticFrame(
        state.data->MinimumDistance, state.data->MaximumDistance);
    const size_t remaining = static_cast<size_t>(2'000'000u - state.data->SourceRows);
    if (sample.Map.sources.size() > remaining)
    {
        sample.Map.droppedSources += sample.Map.sources.size() - remaining;
        sample.Map.sources.resize(remaining);
    }
    sample.CollectionCpuMs = std::chrono::duration<double, std::milli>(FCutsceneState::Clock::now() - begin).count();
    state.data->SourceRows += sample.Map.sources.size();
    state.data->SkippedTicks += sample.SkippedTicks;
    state.data->Samples.push_back(std::move(sample));
    state.status = "컷신 거리 기록 중: " + std::to_string(state.data->Samples.size()) + " 표본";
}
catch (...)
{
    if (auto* bern = CLevel_Bern::Get_Active()) bern->Get_GeometryDiagnosticRuntime().Cancel_GeometryDiagnosticFrame();
    try { Stop_CutsceneCapture("diagnostic_allocation_failed"); } catch (...) { }
}

void Client::CProfilerTool::Stop_CutsceneCapture(const char* reason)
{
    if (!m_Cutscene || (!m_Cutscene->waiting && !m_Cutscene->recording)) return;
    auto& state = *m_Cutscene;
    if (auto* bern = CLevel_Bern::Get_Active())
        bern->Get_GeometryDiagnosticRuntime().Cancel_GeometryDiagnosticFrame();
    state.frameArmed = false; state.waiting = false; state.recording = false;
    state.draining = true; state.stoppedAt = FCutsceneState::Clock::now();
    state.data->StopReason = reason;
    state.data->ElapsedWallMs = state.data->Samples.empty() ? 0. :
        std::chrono::duration<double, std::milli>(state.stoppedAt - state.startedAt).count();
    state.status = "기록 종료. GPU 결과를 비동기로 회수한 뒤 자동 저장합니다.";
}

// 일반 JSON 저장과 컷신 재저장이 호출한다. 활성 표본은 복사하지 않고 기존 종료·drain으로
// 넘긴다. completed는 worker와 공유해도 불변이며, 대기 중 재입력은 새 작업을 만들지 않는다.
bool Client::CProfilerTool::Request_CutsceneSave() try
{
    if (!m_Cutscene)
    {
        return false;
    }
    auto& state = *m_Cutscene;
    if (!state.data && !state.completed && !state.waiting && !state.recording &&
        !state.draining && !state.saveRequested && !state.writerActive)
    {
        return false;
    }
    if (state.writerActive)
    {
        m_strCaptureStatus = "컷신 JSON 저장 중입니다. 완료 결과를 기다립니다.";
        return true;
    }

    // 일반 저장의 프레임 범위 선택만 기억한다. 최대 200만 거리 행은 복제하지 않는다.
    state.saveFrameWindow = Save_FrameWindow();
    if (state.waiting || state.recording)
    {
        Stop_CutsceneCapture("user_requested_save");
    }
    if (state.draining)
    {
        state.status = "거리 기록 종료. GPU 결과 회수 뒤 일반 프레임과 함께 JSON으로 저장합니다.";
    }
    else
    {
        state.saveRequested = true;
        state.status = "보관된 거리 표본과 일반 프레임의 JSON 저장을 기다립니다.";
    }
    state.saveStatusFallback = nullptr;
    m_strCaptureStatus = state.status;
    return true;
}
catch (const std::exception& exception)
{
    if (!m_Cutscene || !m_Cutscene->writerActive)
    {
        Fail_CutsceneSave(exception.what());
    }
    return true;
}
catch (...)
{
    if (!m_Cutscene || !m_Cutscene->writerActive)
    {
        Fail_CutsceneSave("거리 저장 요청 중 알 수 없는 예외가 발생했습니다.");
    }
    return true;
}

// 준비·worker 실패 후 표본을 같은 Tool 안에 보존한다. 자동 반복 재시도는 하지 않으며
// 다음 명시적 JSON 저장/재저장이 재시도한다. 종료 시 기존 exporter 취소 정책은 변경하지 않는다.
void Client::CProfilerTool::Fail_CutsceneSave(const char* reason) noexcept
{
    if (!m_Cutscene)
    {
        return;
    }
    auto& state = *m_Cutscene;
    try
    {
        Stop_CutsceneCapture("save_preparation_failed");
    }
    catch (...)
    {
        // Stop의 상태 문자열 할당이 실패해도 아래 소유권 이동은 할당하지 않는다.
    }
    state.waiting = false;
    state.recording = false;
    state.draining = false;
    state.frameArmed = false;
    state.saveRequested = false;
    state.writerActive = false;
    if (state.data)
    {
        state.completed = std::move(state.data);
    }
    state.status.clear();
    state.saveStatusFallback = "컷신 JSON 저장 실패. 표본은 현재 실행 중에 유지됩니다. JSON 저장으로 다시 시도하세요.";
    try
    {
        state.status = "컷신 JSON 저장 실패: " + std::string(reason ? reason : "원인을 확인할 수 없습니다.") +
            " (표본 유지, JSON 저장으로 다시 시도 가능)";
        m_strCaptureStatus = state.status;
    }
    catch (...)
    {
        // 동적 상태 문자열을 만들 수 없어도 거리 패널의 정적 실패 문구는 남는다.
        OutputDebugStringA("[Profiler] Cutscene JSON save failed; samples retained for explicit retry.\n");
    }
}

// Update_SaveState가 Poll로 회수한 실제 작업 결과만 소비한다. 일반 저장 완료를
// 거리 저장 성공으로 바꾸지 않고, 실패한 completed 표본도 다음 재시도까지 유지한다.
void Client::CProfilerTool::Finish_CutsceneSave(const FProfilerCaptureSaveResult& result) noexcept
{
    if (!m_Cutscene || !m_Cutscene->writerActive)
    {
        return;
    }
    auto& state = *m_Cutscene;
    state.writerActive = false;
    if (!result.Succeeded)
    {
        Fail_CutsceneSave(result.Error.c_str());
        return;
    }
    state.status.clear();
    state.saveStatusFallback = "컷신 JSON 저장 완료.";
    try
    {
        state.status = "컷신 JSON 저장 완료: " + Capture_PathLabel(result.OutputPath);
    }
    catch (...)
    {
        // 디스크 commit 성공은 표시 문자열 할당 실패와 구분한다.
    }
}

// 입력/창 표시와 무관하게 매 프레임 호출한다. 정확히 일치한 geometry frame만 결합한다.
// timeout은 pending을 그대로 저장하며 render thread에서 GPU를 기다리지 않는다.
void Client::CProfilerTool::Update_CutsceneCapture() try
{
    if (!m_Cutscene)
    {
        return;
    }
    auto& state = *m_Cutscene;
    auto* profiler = CGameInstance::Get().Get_Profiler();
    if (state.data && profiler)
    {
        for (auto& sample : state.data->Samples)
        {
            if (sample.GeometryResolved ||
                !profiler->Get_GeometryStatsForFrame(sample.GeometryFrame, sample.Geometry) || sample.Geometry.Pending)
            {
                continue;
            }
            profiler->Get_GeometryIndirectArguments(sample.GeometryFrame, sample.IndirectArguments);
            sample.GeometryResolved = true;
        }
    }
    if (state.draining)
    {
        const bool pending = std::any_of(state.data->Samples.begin(), state.data->Samples.end(),
            [](const auto& sample) { return !sample.GeometryResolved; });
        if (pending && std::chrono::duration<double>(FCutsceneState::Clock::now() - state.stoppedAt).count() < 2.)
        {
            return;
        }
        state.draining = false;
        state.completed = std::move(state.data); // worker에 넘긴 이후에는 표본을 수정하지 않는다.
        state.saveRequested = true;
    }
    if (!state.saveRequested || m_Exporter.IsSaving())
    {
        return;
    }
    if (!profiler || !state.completed)
    {
        Fail_CutsceneSave("저장할 Profiler 또는 완료된 거리 표본이 없습니다.");
        return;
    }

    auto context = Sample_Context();
    context.CutsceneGeometry = state.completed;
    // 수집을 껐어도 남아 있는 일반 프레임을 함께 저장한다. 거리 표본은 raw frame window와 독립이다.
    auto snapshot = profiler->Snapshot(state.saveFrameWindow);
    CProfilerCaptureIO::Copy_MovementSamples(snapshot, context);
    std::filesystem::path output;
    std::string error;
    std::string label = m_CaptureName[0] ? std::string(m_CaptureName.data()) + "-bern-distance" : "bern-distance";
    if (!CProfilerCaptureIO::Validate_Name(label))
    {
        label = "bern-distance";
    }
    // 문자열 준비도 worker 시작 전에 끝낸다. 시작 성공 뒤에는 noexcept 상태 교체만 수행한다.
    std::string savingCoverage = "일반 프레임 " + std::to_string(snapshot.Frames.size()) +
        "개 | 베른 컷신 거리 " + std::to_string(state.completed->Samples.size()) +
        " 표본 | 종료 사유: " + state.completed->StopReason;
    std::string captureStatus = "컷신 JSON 백그라운드 저장 중... " + savingCoverage;
    std::string cutsceneStatus = "컷신 JSON 저장 중. 디스크 저장 완료를 기다립니다.";
    if (!CProfilerCaptureIO::Make_NamedPath(label, profiler->Get_GeometryFrameNumber(), output, &error) ||
        !m_Exporter.BeginSave(std::move(snapshot), output, &error, std::move(context)))
    {
        Fail_CutsceneSave(error.c_str());
        return;
    }
    state.saveRequested = false;
    state.writerActive = true;
    state.saveStatusFallback = nullptr;
    m_strSavingCoverage.swap(savingCoverage);
    m_strCaptureStatus.swap(captureStatus);
    state.status.swap(cutsceneStatus);
}
catch (const std::exception& exception)
{
    Fail_CutsceneSave(exception.what());
}
catch (...)
{
    Fail_CutsceneSave("거리 JSON 저장 준비 중 알 수 없는 예외가 발생했습니다.");
}

void Client::CProfilerTool::Render_CutsceneControls()
{
    if (!m_Cutscene) m_Cutscene = std::make_shared<FCutsceneState>();
    auto& state = *m_Cutscene;
    const bool busy = state.waiting || state.recording || state.draining || state.saveRequested || m_Exporter.IsSaving();
    ImGui::Separator();
    ImGui::TextUnformatted("베른 컷신 거리 / 실제 geometry 기록");
    ImGui::BeginDisabled(busy);
    ImGui::SetNextItemWidth(100.f);
    ImGui::DragInt("기록 간격 (ms)", &state.intervalMs, 10.f, 20, 1000, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SameLine(); ImGui::SetNextItemWidth(100.f);
    ImGui::DragFloat("최소 거리 (m)", &state.minimumDistance, 1.f, 0.f, 5000.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SameLine(); ImGui::SetNextItemWidth(100.f);
    ImGui::DragFloat("최대 거리 (m)", &state.maximumDistance, 1.f, 1.f, 5000.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
    auto* bern = CLevel_Bern::Get_Active();
    const bool available = bern && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::BERN);
    ImGui::BeginDisabled(!available || !std::isfinite(state.minimumDistance) || !std::isfinite(state.maximumDistance) ||
        state.minimumDistance >= state.maximumDistance);
    if (ImGui::Button(available && bern->Is_EntranceCinematicActive() ? "진행 중 컷신 거리 기록" : "컷신 재생 + 거리 기록"))
    {
        if (bern->Is_EntranceCinematicActive() || bern->Request_DebugEntranceReplay())
        {
            auto data = std::make_shared<FProfilerCutsceneCapture>();
            data->IntervalMs = std::clamp(state.intervalMs, 20, 1000);
            data->MinimumDistance = state.minimumDistance; data->MaximumDistance = state.maximumDistance;
            state.data = std::move(data); state.completed.reset();
            state.saveFrameWindow = Engine::CProfiler::MAX_HISTORY_FRAMES;
            state.saveStatusFallback = nullptr;
            state.waiting = true; state.nextTick = 0; state.requestedAt = FCutsceneState::Clock::now();
            state.status = "컷신 카메라 시작 대기 중. 종료 시 JSON을 자동 저장합니다.";
        }
        else state.status = bern->Get_DebugEntranceReplayStatus();
    }
    ImGui::EndDisabled(); ImGui::EndDisabled();
    if (state.waiting || state.recording)
    {
        ImGui::SameLine();
        if (ImGui::Button("기록 종료 + 저장")) Stop_CutsceneCapture("user_stopped");
    }
    if (state.completed && !busy)
    {
        ImGui::SameLine();
        if (ImGui::Button("컷신 JSON 다시 저장")) Request_CutsceneSave();
    }
    ImGui::TextDisabled("F7로 창을 닫아도 계속 기록합니다. 거리: 배치 bounding sphere 중심 기준. 기본 0~250m / 100ms.");
    ImGui::TextDisabled("늦은 tick은 건너뛴 수를 기록하며 보간하지 않습니다. 최대 120초 / 1200표본 / 200만 source행.");
    ImGui::TextDisabled("Vtx(VS): GPU vertex shader 실행 수 / Idx: Engine indexed 제출 수. GPU 수치는 비동기 완료 프레임 기준.");
    ImGui::TextDisabled("저장 중 앱을 종료하면 작업이 취소될 수 있습니다. 저장 완료 파일 경로를 확인하세요.");
    if (!state.status.empty())
    {
        ImGui::TextWrapped("%s", state.status.c_str());
    }
    else if (state.saveStatusFallback)
    {
        ImGui::TextWrapped("%s", state.saveStatusFallback);
    }
    const auto* data = state.data ? state.data.get() : state.completed.get();
    if (data && !data->Samples.empty())
    {
        const auto& last = data->Samples.back();
        ImGui::Text("%zu 표본 | cue %.3fs | 거리 source %zu | map draw %zu | 수집 %.3fms | 건너뜀 %llu tick",
            data->Samples.size(), last.CueSeconds, last.Map.sources.size(), last.Map.draws.size(), last.CollectionCpuMs,
            static_cast<unsigned long long>(data->SkippedTicks));
        if (ImGui::CollapsingHeader("최근 표본 거리 / draw 상세"))
        {
            ImGui::TextDisabled("거리 차이는 직전 표본에도 같은 source가 있을 때 표시합니다. VB 정점은 버퍼 용량입니다.");
            if (ImGui::BeginTable("##CutsceneSources", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY,
                ImVec2(0.f, 160.f)))
            {
                for (const char* column : {"Asset / mesh", "거리 m", "거리 차이 m", "경로", "원본 Idx"}) ImGui::TableSetupColumn(column);
                ImGui::TableHeadersRow();
                ImGuiListClipper clipper; clipper.Begin(static_cast<int>(last.Map.sources.size()));
                while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
                {
                    const auto& row = last.Map.sources[static_cast<size_t>(i)];
                    const MAP_GEOMETRY_DIAGNOSTIC_SOURCE* prior = nullptr;
                    if (data->Samples.size() > 1u)
                    {
                        const auto& previous = data->Samples[data->Samples.size() - 2u].Map.sources;
                        const auto found = std::find_if(previous.begin(), previous.end(), [&](const auto& source) {
                            return source.placementId == row.placementId && source.assetId == row.assetId && source.meshIndex == row.meshIndex;
                        });
                        if (found != previous.end()) prior = &*found;
                    }
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("%s / %u", row.assetId.c_str(), row.meshIndex);
                    ImGui::TableNextColumn(); ImGui::Text("%.2f", row.centerDistance);
                    ImGui::TableNextColumn(); if (prior) ImGui::Text("%+.3f", row.centerDistance - prior->centerDistance); else ImGui::TextUnformatted("--");
                    ImGui::TableNextColumn(); ImGui::TextUnformatted(row.route.c_str());
                    ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(row.baseIndexCount));
                }
                ImGui::EndTable();
            }
            if (ImGui::BeginTable("##CutsceneDraws", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY,
                ImVec2(0.f, 160.f)))
            {
                for (const char* column : {"실제 제출 경로", "LOD", "VB 정점", "인스턴스", "제출 Idx"}) ImGui::TableSetupColumn(column);
                ImGui::TableHeadersRow();
                ImGuiListClipper clipper; clipper.Begin(static_cast<int>(last.Map.draws.size()));
                while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
                {
                    const auto& draw = last.Map.draws[static_cast<size_t>(i)];
                    bool known = draw.instanceCountKnown;
                    uint64_t instances = draw.instanceCount, indices = draw.indexCount;
                    if (!known && draw.geometryFrameNumber == last.GeometryFrame)
                        for (const auto& argument : last.IndirectArguments)
                            if (argument.BufferOrdinal == draw.indirectBufferOrdinal && argument.ArgsByteOffset == draw.argsByteOffset)
                            { known = true; instances = argument.InstanceCount; indices = argument.IndexCount; break; }
                    ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(draw.path.c_str());
                    ImGui::TableNextColumn(); if (draw.lod == UINT32_MAX) ImGui::TextUnformatted("HLOD"); else ImGui::Text("%u", draw.lod);
                    ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(draw.vertexBufferCount));
                    ImGui::TableNextColumn(); if (known) ImGui::Text("%llu", static_cast<unsigned long long>(instances)); else ImGui::TextUnformatted("pending");
                    ImGui::TableNextColumn(); if (known) ImGui::Text("%llu", static_cast<unsigned long long>(indices * instances)); else ImGui::TextUnformatted("--");
                }
                ImGui::EndTable();
            }
        }
    }
}
