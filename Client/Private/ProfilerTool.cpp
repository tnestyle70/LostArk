#include "imgui.h"
#include "ProfilerTool.h"

#include <algorithm>
#include <cctype>

namespace
{
    // SCOPE_CATALOG: kept in sync with actual CProfilerScope/GpuScope call sites.
    constexpr const char* CPU_SCOPE_CATALOG[] = {
        "Animation.Blend",
        "Animation.Bones.Combine",
        "Animation.Channels.Sample",
        "Animation.Channels.Update",
        "Animation.History.DebugVerify",
        "Animation.History.Sample",
        "Animation.Play",
        "Animation.SkinPalette.Bind",
        "Animation.SkinPalette.Build",
        "Animation.Transition.Build",
        "Catalog.PlayerSkills.Initialize",
        "Character.EffectCues.Admit",
        "Character.FaceMorph.Admit",
        "Character.FaceSliders.Admit",
        "Character.Initialize",
        "Character.SkillBindings.Admit",
        "CharacterAssets.Authoring.Capture",
        "CharacterAssets.Authoring.Prepare",
        "CharacterAssets.Commit",
        "CharacterAssets.FaceMorph.Prepare",
        "CharacterAssets.LevelTransition.Drain",
        "CharacterAssets.Prepare.Worker",
        "CharacterAssets.Retire.Worker",
        "Client.Render",
        "Client.Update",
        "Effect.Decal.Render",
        "Effect.FollowAnchors.Update",
        "Effect.Material.Bind",
        "Effect.Mesh.BindAndDraw",
        "Effect.Mesh.DrawSubmission",
        "Effect.Mesh.InstanceBuild",
        "Effect.Mesh.InstanceUpload",
        "Effect.Mesh.Render",
        "Effect.Occurrence.LateUpdate",
        "Effect.Occurrence.Render",
        "Effect.Occurrence.Update",
        "Effect.Particle.Render",
        "Effect.Particle.Spawn",
        "Effect.Particle.Update",
        "Effect.Playback.FixedStep",
        "Effect.Playback.FrameRebuild",
        "Effect.Playback.HistoryUpdate",
        "Effect.Playback.Update",
        "Effect.Prepare.WorkerTarget",
        "Effect.Prewarm.Advance",
        "Effect.ProductGroups.Update",
        "Effect.Rect.Render",
        "Effect.Service.Update",
        "Effect.Spawn.Commit",
        "Effect.Spawn.CommitWorldRoots",
        "Effect.Sprite.DepthSort",
        "Effect.Sprite.DrawSubmission",
        "Effect.Sprite.InstanceBuild",
        "Effect.Sprite.InstanceUpload",
        "Effect.Trail.Render",
        "Effect.TrailAfterimage.Simulate",
        "EffectSequencer.LaneLayout",
        "EffectSequencer.Render",
        "EffectTool.AllEffectsWindow",
        "EffectTool.ArtistF.MaterialPreparation",
        "EffectTool.ArtistF.SourcePreparation",
        "EffectTool.AuthoringWindow",
        "EffectTool.DataFilesWindow",
        "EffectTool.DetailWindow",
        "EffectTool.DocumentLoad",
        "EffectTool.DocumentLoad.CanonicalBaseline",
        "EffectTool.DocumentLoad.Parse",
        "EffectTool.DocumentLoad.ValidateDrawable",
        "EffectTool.InitialIndexStep",
        "EffectTool.ModelViewWindow",
        "EffectTool.Render",
        "EffectTool.ResourceGrid",
        "EffectTool.ThumbnailTrim",
        "EffectTool.UnifiedEffectTree",
        "EffectTool.UnifiedFamilyRows",
        "Engine.Camera.Update",
        "Engine.Input.Update",
        "Engine.LateUpdate",
        "Engine.LevelUpdate",
        "Engine.ObjectUpdate",
        "Engine.Physics",
        "Engine.PostPhysicsUpdate",
        "Engine.PriorityUpdate",
        "Engine.Sound.Update",
        "ImGui.BackendSubmit",
        "ImGui.BuildAndSubmit",
        "ImGui.DX11.BackupState",
        "ImGui.DX11.DeviceObjects",
        "ImGui.DX11.DrawSubmission",
        "ImGui.DX11.GrowBuffers",
        "ImGui.DX11.MapBuffers",
        "ImGui.DX11.RestoreState",
        "ImGui.DX11.SetupState",
        "ImGui.DX11.TextureUpdate",
        "ImGui.DX11.UploadBuffers",
        "ImGui.DeveloperTools",
        "ImGui.FinalizeDrawData",
        "ImGui.Hub.Build",
        "ImGui.Hub.CameraAndPlayer",
        "ImGui.Hub.FollowCamera",
        "ImGui.Hub.KoukuArena",
        "ImGui.Hub.KoukuCompletePlay",
        "ImGui.Hub.KoukuUIPreview",
        "ImGui.Hub.LevelNavigation",
        "ImGui.Hub.SequenceViewer.Build",
        "ImGui.Hub.SequenceViewer.Refresh",
        "ImGui.Hub.SequenceViewer.Update",
        "ImGui.Hub.ServerArena",
        "ImGui.Hub.ValtanCompletePlay",
        "ImGui.NewFrame",
        "ImGui.NewFrame.Core",
        "ImGui.NewFrame.DX11",
        "ImGui.NewFrame.Win32",
        "ImGui.PlatformRender",
        "ImGui.PlatformUpdate",
        "ImGui.PlatformViewport.Present",
        "ImGui.PlatformViewport.Render",
        "ImGui.ProfilerDetails",
        "ImGui.ProfilerOverlay",
        "ImGui.RenderDrawData",
        "ImGui.Tool.Animation.Build",
        "ImGui.Tool.Animation.Update",
        "ImGui.Tool.Balance.Build",
        "ImGui.Tool.Balance.Update_ServerRuntimeSetPublishJob",
        "ImGui.Tool.Balance.Update_ValtanSaveJob",
        "ImGui.Tool.Camera.Build",
        "ImGui.Tool.Camera.Update",
        "ImGui.Tool.Chat.Build",
        "ImGui.Tool.Composition.Build",
        "ImGui.Tool.CompositionProfiler.Build",
        "ImGui.Tool.EffectV1.Build",
        "ImGui.Tool.EffectV1.Update",
        "ImGui.Tool.EffectV1.Update_AuthoringWorkspace",
        "ImGui.Tool.EffectV2.Build",
        "ImGui.Tool.EffectV2.Update_AuthoringWorkspace",
        "ImGui.Tool.Equipment.Build",
        "ImGui.Tool.HUDLayout.Build",
        "ImGui.Tool.KoukuBoss.Build",
        "ImGui.Tool.Map.Build",
        "ImGui.Tool.Map.Update",
        "ImGui.Tool.Open",
        "ImGui.Tool.Party.Build",
        "ImGui.Tool.Rendering.Build",
        "ImGui.Tool.SequenceBenchmark.Build",
        "ImGui.Tool.ValtanBoss.Build",
        "ImGui.Tool.ValtanBoss.Render_LogicPatternWindow",
        "ImGui.Tool.ValtanBoss.Update",
        "ImGui.Tool.ValtanComposition.Update_SaveState",
        "ImGui.Tool.WorldObjects.Build",
        "ImGui.Tool.WorldObjects.Update",
        "Level.Kouku.Camera.Create",
        "Level.Kouku.CameraShots.Load",
        "Level.Kouku.Controller.Prepare",
        "Level.Kouku.DebugTriggers.Load",
        "Level.Kouku.DeployCommit",
        "Level.Kouku.InitialVisibility",
        "Level.Kouku.Initialize",
        "Level.Kouku.MapLights.Load",
        "Level.Kouku.MapPlacementCommit",
        "Level.Kouku.Replication.Initialize",
        "Level.Kouku.SelfMotion.Load",
        "Level.Kouku.StageMarkers.Load",
        "Level.Kouku.UI.Create",
        "Level.Kouku.WorldSequence.Load",
        "Loader.EffectPreparation",
        "Loader.LevelLoad",
        "MainApp.DebugTools.Update",
        "MainApp.Engine.Update",
        "MainApp.InputAndUI.Update",
        "MainApp.LevelAndEnvironment.Update",
        "MainApp.Presentation.Prepare",
        "Map.Batch.BindAndDraw",
        "Map.Batch.CullAndPack",
        "Map.Batch.InstanceUpload",
        "Map.Batch.ShadowPrepare",
        "Map.Batch.ShadowUpload",
        "Map.Batch.Visibility",
        "Model.Load.Animations",
        "Model.Load.Binary",
        "Model.Load.Bones",
        "Model.Load.Decode",
        "Model.Load.Materials",
        "Model.Load.MaterialWorker",
        "Model.Load.MaterialJoin",
        "Model.Load.Meshes",
        "Texture.Cache.Lookup",
        "Texture.Cache.SameKeyWait",
        "Texture.Load.FileAndUpload",
        "Navigation.AStar",
        "Navigation.FindPath",
        "Navigation.Follow",
        "Navigation.Request",
        "Navigation.RoundCorners",
        "Navigation.Simplify",
        "Network.DrainAndDispatch",
        "Network.PlayerAssets.Advance",
        "Network.PlayerPresentation.CommitSpawn",
        "Network.PlayerPresentation.Replace",
        "Picking.CopyPixel",
        "Picking.MapWait",
        "Picking.ReadPixel",
        "Picking.Readback",
        "Profiler.Capture.Snapshot",
        "Profiler.Panel.Refresh",
        "Map.Batch.Material.Bind",
        "Map.Batch.Pass.Apply",
        "Map.Batch.Mesh.Submit",
        "Map.Shadow.Material.Bind",
        "Map.Shadow.Pass.Apply",
        "Map.Shadow.Mesh.Submit",
        "Effect.Particle.Update.Worker",
        "Effect.Particle.Update.Join",
        "Render.BeginFrame",
        "Render.Blend",
        "Render.Bloom",
        "Render.BossShowcase",
        "Render.Combined",
        "Render.Debug",
        "Render.DisplayOverlays",
        "Render.Draw",
        "Render.Final",
        "Render.Lights",
        "Render.NonBlend",
        "Render.NonLight",
        "Render.Portraits",
        "Render.Present",
        "Render.Priority",
        "Render.SSAO",
        "Render.SceneColorSnapshot",
        "Render.SceneHDR",
        "Render.ScreenPosts",
        "Render.Shadow",
        "Render.SubmitFrameProviders",
        "Render.UI",
        "Render.UIText",
        "Render.World",
        "Replication.Update",
        "Shader.BuildBindings",
        "Shader.CreateEffect",
        "Shader.CreateInputLayouts",
        "Shader.Load",
        "Shader.ReadBytecode",
        "Tool.Composition.ReloadCanonical",
        "Tool.Composition.SaveReload",
        "Tool.ValtanBossTool.ReloadCanonicalGraph",
        "UI.Runtime.BossHealthBar.Update",
        "UI.Runtime.BossImmuneGauge.Update",
        "UI.Runtime.CharacterSelectWindow.Update",
        "UI.Runtime.ChargeGauge.Update",
        "UI.Runtime.CombatHUD.Update",
        "UI.Runtime.EstherGauge.Update",
        "UI.Runtime.ItemQuickSlots.Update",
        "UI.Runtime.ItemUpgrade.Update",
        "UI.Runtime.LanceMasterIdentityGauge.Update",
        "UI.Runtime.LobbyButtons.Update",
        "UI.Runtime.Minimap.Update",
        "UI.Runtime.PlayerHealthManaBar.Update",
        "UI.Runtime.QuickSlotFlash.Update",
        "UI.Runtime.SkillCooldowns.Update",
        "UI.Runtime.SkillIcons.Update",
        "WorldSequence.CollectLoadTargets",
        "WorldSequence.CommitPrepared",
        "WorldSequence.Document.Load",
        "WorldSequence.Document.Parse",
        "WorldSequence.Document.Validate",
        "WorldSequence.PrepareArea",
    };
    constexpr const char* GPU_SCOPE_CATALOG[] = {
        "ImGui.BackendSubmit",
        "ImGui.PlatformViewport.Present",
        "ImGui.PlatformViewport.Render",
        "ImGui.RenderDrawData",
        "Picking.CopyPixel",
        "Render.BeginFrame",
        "Render.Blend",
        "Render.Bloom",
        "Render.BossShowcase",
        "Render.Combined",
        "Render.Debug",
        "Render.DisplayOverlays",
        "Render.Draw",
        "Render.Final",
        "Render.Lights",
        "Render.NonBlend",
        "Render.NonLight",
        "Render.Portraits",
        "Render.Priority",
        "Render.SSAO",
        "Render.SceneColorCopy",
        "Render.SceneHDR",
        "Render.ScreenPosts",
        "Render.Shadow",
        "Render.UI",
        "Render.UIText",
    };
    constexpr ImGuiTableFlags TABLE_FLAGS = ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;

    constexpr std::array<const char*, static_cast<size_t>(Engine::EProfilerCounter::Count)>
        COUNTER_LABELS = {
        "Draw calls", "Instanced draw calls", "Instances", "Indices",
        "Submissions: priority", "Submissions: shadow", "Submissions: non-blend", "Submissions: blend",
        "Map placements", "Map visible instances", "Map batches", "Map fallback objects",
        "Texture requests", "Texture path hits", "Texture content hits", "Texture unique SRVs",
        "Texture estimated GPU bytes", "Navigation queries (Client)", "Navigation expanded nodes (Client)",
        "Navigation query microseconds (Client)", "Navigation path cells (Client)",
        "Scene color copies", "Scene color copy bytes (source size)",
        "ImGui draw lists",
        "ImGui vertices",
        "ImGui indices",
        "ImGui draw commands",
        "ImGui actual draw calls",
        "ImGui callbacks",
        "ImGui rendered windows",
        "ImGui active windows",
        "ImGui platform viewports",
        "ImGui vertex upload bytes",
        "ImGui index upload bytes",
        "ImGui constant upload bytes",
        "ImGui texture upload bytes",
        "ImGui buffer growths",
        "ImGui texture creates",
        "ImGui texture updates",
        "ImGui device object builds",
        "ImGui buffer Map failures",
        "Picking readbacks",
        "Picking readback bytes",
        "Indirect draw calls",
        "Indirect indices (LOD0 upper bound)",
    };

    bool Contains_CaseInsensitive(std::string_view text, const char* query)
    {
        if (!query || !query[0]) return true;
        const std::string_view needle(query);
        return std::search(text.begin(), text.end(), needle.begin(), needle.end(),
            [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) ==
                std::tolower(static_cast<unsigned char>(b)); }) != text.end();
    }

    std::string Capture_PathLabel(const std::filesystem::path& path)
    {
        const auto text = path.u8string();
        return std::string(text.begin(), text.end());
    }

    const char* Gpu_Status(Engine::EProfilerGpuFrameStatus status)
    {
        switch (status)
        {
        case Engine::EProfilerGpuFrameStatus::Unsupported: return "unsupported";
        case Engine::EProfilerGpuFrameStatus::Pending: return "pending";
        case Engine::EProfilerGpuFrameStatus::Valid: return "valid";
        case Engine::EProfilerGpuFrameStatus::Disjoint: return "disjoint";
        case Engine::EProfilerGpuFrameStatus::Dropped: return "dropped";
        case Engine::EProfilerGpuFrameStatus::Error: return "query error";
        }
        return "unknown";
    }

    void Unobserved_Row(const char* name, int columns)
    {
        ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextDisabled("%s", name);
        for (int i = 1; i < columns; ++i)
        {
            ImGui::TableNextColumn(); ImGui::TextDisabled(i == 1 ? "not observed" : "--");
        }
    }
}

void Client::CProfilerTool::Refresh(Engine::CProfiler& profiler)
{
    if (!m_bCatalogRegistered)
    {
        for (const char* name : CPU_SCOPE_CATALOG) profiler.Register_ScopeName(name);
        for (const char* name : GPU_SCOPE_CATALOG) profiler.Register_ScopeName(name);
        m_bCatalogRegistered = true;
    }
    Engine::CProfilerScope scope(&profiler, "Profiler.Panel.Refresh");
    m_iMainThreadId = profiler.Get_MainThreadId();
    m_iHistoryFrames = profiler.Get_HistoryFrameCount();
    profiler.Get_ScopeNames(m_ScopeNames);
    const size_t window = static_cast<size_t>((std::max)(m_iWindowFrameInput, 1));
    profiler.Get_ScopeAggregates(window, m_Aggregates);
    profiler.Get_GpuScopeAggregates(window, m_GpuAggregates, m_iGpuValidFrames, m_iGpuPartialFrames);
    profiler.Get_WindowFrameStats(window, m_fWindowCpuAvgMs, m_fWindowCpuMaxMs,
        m_fWindowGpuAvgMs, m_fWindowGpuMaxMs, m_iWindowFrames, &m_iGpuFrameValidFrames);
    profiler.Get_LongOperations(m_LongOperations);
    m_bLiveValid = profiler.Get_LiveStats(m_Live);
    m_bCpuRowsDirty = true;
}

const char_t* Client::CProfilerTool::Scope_Name(uint32_t id) const
{
    return id < m_ScopeNames.size() ? m_ScopeNames[id].c_str() : "<unknown>";
}

std::string Client::CProfilerTool::Thread_Label(uint32_t id) const
{
    return id == m_iMainThreadId ? "main" : "worker " + std::to_string(id);
}

void Client::CProfilerTool::Request_Save(Engine::CProfiler& profiler)
{
    if (m_Exporter.IsSaving()) return;
    std::string error;
    if (!CProfilerCaptureIO::Validate_Name(m_CaptureName.data(), &error))
    { m_strCaptureStatus = error; return; }
    Engine::FProfilerCaptureSnapshot snapshot;
    {
        Engine::CProfilerScope scope(&profiler, "Profiler.Capture.Snapshot");
        snapshot = profiler.Snapshot();
    }
    const uint64_t frame = snapshot.Frames.empty() ? 0 : snapshot.Frames.back().FrameNumber;
    std::filesystem::path output;
    if (!CProfilerCaptureIO::Make_NamedPath(m_CaptureName.data(), frame, output, &error))
    { m_strCaptureStatus = error; return; }
    m_strCaptureStatus = m_Exporter.BeginSave(std::move(snapshot), output, &error) ?
        "Saving JSON in background..." : error;
}

void Client::CProfilerTool::Update_SaveState()
{
    FProfilerCaptureSaveResult saveResult;
    if (!m_Exporter.Poll(saveResult)) return;
    m_strCaptureStatus = saveResult.Succeeded ?
        "Saved " + Capture_PathLabel(saveResult.OutputPath) : saveResult.Error;
    if (saveResult.Succeeded && Refresh_CaptureFiles())
        for (const auto& file : m_CaptureFiles)
            if (file.FileName == saveResult.OutputPath.filename())
            { m_strSelectedCaptureId = file.StableId; break; }
}

void Client::CProfilerTool::Render(Engine::CProfiler* profiler)
{
    if (!m_bOpen) return;
    ImGui::SetNextWindowSize(ImVec2(1060.f, 720.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Composition Profiler###LostArkProfilerToolV1", &m_bOpen))
    {
        ImGui::End(); return;
    }
    if (!profiler)
    {
        ImGui::TextUnformatted("Engine profiler is unavailable."); ImGui::End(); return;
    }

    if (!m_bCaptureFilesLoaded) Refresh_CaptureFiles();
    ImGui::SetNextItemWidth(300.f);
    ImGui::InputTextWithHint("Save name", "Optional name (Korean supported)", m_CaptureName.data(), m_CaptureName.size());
    ImGui::SameLine(); ImGui::TextDisabled("Each save creates a new JSON file.");
    bool enabled = profiler->Is_Enabled();
    if (ImGui::Checkbox("Capture", &enabled))
    {
        profiler->Set_Enabled(enabled); m_fLastRefreshTime = -1.0;
    }
    ImGui::SameLine(); ImGui::SetNextItemWidth(100.f);
    if (ImGui::DragInt("Frames", &m_iWindowFrameInput, 1.f, 1,
        static_cast<int>(Engine::CProfiler::MAX_HISTORY_FRAMES), "%d", ImGuiSliderFlags_AlwaysClamp))
        m_fLastRefreshTime = -1.0;
    ImGui::SameLine();
    if (ImGui::Button("Reset")) { profiler->Reset_History(); m_fLastRefreshTime = -1.0; }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_Exporter.IsSaving());
    if (ImGui::Button("Save JSON")) Request_Save(*profiler);
    ImGui::EndDisabled(); ImGui::SameLine();
    ImGui::TextDisabled("%zu / %zu history frames", m_iHistoryFrames, Engine::CProfiler::MAX_HISTORY_FRAMES);

    const double now = ImGui::GetTime();
    if (m_fLastRefreshTime < 0.0 ||
        ((enabled || (m_bLiveValid && m_Live.LatestFrameGpuStatus == Engine::EProfilerGpuFrameStatus::Pending)) &&
            now - m_fLastRefreshTime >= m_fRefreshIntervalSeconds))
    {
        Refresh(*profiler); m_fLastRefreshTime = now;
    }
    if (ImGui::BeginTable("##FrameSummary", 3, ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableNextColumn(); ImGui::TextDisabled("FRAME INTERVAL / FPS (latest)");
        if (m_bLiveValid && m_Live.FrameIntervalMs > 0.0)
            ImGui::Text("%.2f ms / %.1f FPS", m_Live.FrameIntervalMs, 1000.0 / m_Live.FrameIntervalMs);
        else ImGui::TextDisabled("--");
        ImGui::TableNextColumn(); ImGui::TextDisabled("CPU FRAME (window average / peak)");
        if (m_iWindowFrames) ImGui::Text("%.2f / %.2f ms", m_fWindowCpuAvgMs, m_fWindowCpuMaxMs);
        else ImGui::TextDisabled("--");
        ImGui::TableNextColumn(); ImGui::TextDisabled("GPU INTERVAL (valid average / peak)");
        if (m_iGpuFrameValidFrames > 0)
            ImGui::Text("%.2f / %.2f ms", m_fWindowGpuAvgMs, m_fWindowGpuMaxMs);
        else ImGui::TextDisabled("-- (no resolved results in window)");
        ImGui::EndTable();
    }
    if (!enabled) ImGui::TextDisabled("Capture is paused. Recorded results stay visible.");
    if (m_bLiveValid && (m_Live.TotalDroppedCpuScopes || m_Live.TotalDroppedGpuFrames ||
        m_Live.TotalDroppedGpuScopes || m_Live.TotalDroppedModelAnimationSamples))
        ImGui::TextWrapped("Incomplete capture since reset: omitted CPU scopes %llu / GPU frames %llu / GPU scopes %llu / animation samples %llu",
            static_cast<unsigned long long>(m_Live.TotalDroppedCpuScopes), static_cast<unsigned long long>(m_Live.TotalDroppedGpuFrames),
            static_cast<unsigned long long>(m_Live.TotalDroppedGpuScopes), static_cast<unsigned long long>(m_Live.TotalDroppedModelAnimationSamples));
    if (!m_strCaptureStatus.empty()) ImGui::TextWrapped("%s", m_strCaptureStatus.c_str());
    ImGui::Separator();
    ImGui::SetNextItemWidth(280.f);
    ImGui::InputTextWithHint("##ProfilerFilter", "Filter section (Animation, Map, Render...)", m_Filter.data(), m_Filter.size());
    ImGui::SameLine(); ImGui::Checkbox("Show unobserved sections", &m_bShowUnobserved);
    if (ImGui::BeginTabBar("##ProfilerTabs"))
    {
        if (ImGui::BeginTabItem("ImGui")) { Render_ImGui(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("CPU sections")) { Render_Bottlenecks(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("GPU passes")) { Render_Gpu(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Workload")) { Render_Counters(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Long operations")) { Render_LongOperations(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Saved JSON")) { Render_CaptureFiles(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Client::CProfilerTool::Render_Bottlenecks(bool_t bImGuiOnly)
{
    Rebuild_CpuRows(bImGuiOnly);
    ImGui::TextWrapped("Avg includes child sections; Self excludes them on the same thread. Do not add parent and child times. Peak is one call. Unobserved means no completed sample in this window.");
    const double frames = static_cast<double>((std::max)(m_iWindowFrames, size_t{1}));
    if (!ImGui::BeginTable("##CpuSections", 6, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Section", ImGuiTableColumnFlags_WidthStretch, 3.f);
    for (const char* label : { "Thread", "Avg ms/frame", "Self ms/frame", "Peak call ms", "Calls/frame" })
        ImGui::TableSetupColumn(label);
    ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_VisibleCpuRows.size()));
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& visible = m_VisibleCpuRows[static_cast<size_t>(i)];
            if (!visible.Aggregate) { Unobserved_Row(visible.Name, 6); continue; }
            const auto& row = *visible.Aggregate;
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(visible.Name);
            ImGui::TableNextColumn();
            if (row.ThreadId == m_iMainThreadId) ImGui::TextUnformatted("main");
            else ImGui::Text("worker %u", row.ThreadId);
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.InclusiveMs / frames);
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.SelfMs / frames);
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.MaxMs);
            ImGui::TableNextColumn(); ImGui::Text("%.2f", static_cast<double>(row.Calls) / frames);
        }
    ImGui::EndTable();
}

void Client::CProfilerTool::Rebuild_CpuRows(bool_t bImGuiOnly)
{
    if (!m_bCpuRowsDirty && m_strCpuRowFilter == m_Filter.data() &&
        m_bCpuRowsImGuiOnly == bImGuiOnly && m_bCpuRowsShowUnobserved == m_bShowUnobserved)
        return;
    m_VisibleCpuRows.clear();
    const auto matches = [&](const char* name)
    {
        const std::string_view text(name);
        const bool isImGui = text.starts_with("ImGui.") || text.starts_with("Profiler.Panel.") ||
            text.starts_with("Profiler.Capture.") || text.starts_with("EffectTool.") || text.starts_with("EffectSequencer.");
        return (!bImGuiOnly || isImGui) && Contains_CaseInsensitive(text, m_Filter.data());
    };
    for (const auto& row : m_Aggregates)
        if (matches(Scope_Name(row.NameId)))
            m_VisibleCpuRows.push_back({Scope_Name(row.NameId), &row});
    if (m_bShowUnobserved)
        for (const char* name : CPU_SCOPE_CATALOG)
            if (matches(name) && std::none_of(m_Aggregates.begin(), m_Aggregates.end(),
                [&](const auto& row) { return std::string_view(name) == Scope_Name(row.NameId); }))
                m_VisibleCpuRows.push_back({name, nullptr});
    m_strCpuRowFilter = m_Filter.data();
    m_bCpuRowsImGuiOnly = bImGuiOnly;
    m_bCpuRowsShowUnobserved = m_bShowUnobserved;
    m_bCpuRowsDirty = false;
}

void Client::CProfilerTool::Render_ImGui()
{
    ImGui::TextWrapped("Build = tool code and widget layout. DX11 = upload, state changes and drawing. Platform Present can include OS/driver waits. Compare the same scene with F1 closed, one tool open, then detached windows.");
    if (m_bLiveValid)
    {
        const auto count = [&](Engine::EProfilerCounter counter)
        { return static_cast<unsigned long long>(m_Live.Counters[static_cast<size_t>(counter)]); };
        ImGui::Text("CPU frame %llu | windows %llu / active %llu | viewports %llu | draws %llu / commands %llu",
            static_cast<unsigned long long>(m_Live.FrameNumber), count(Engine::EProfilerCounter::ImGuiRenderWindows),
            count(Engine::EProfilerCounter::ImGuiActiveWindows), count(Engine::EProfilerCounter::ImGuiPlatformViewports),
            count(Engine::EProfilerCounter::ImGuiDrawCalls), count(Engine::EProfilerCounter::ImGuiDrawCommands));
        ImGui::Text("Vertices %llu / indices %llu | vertex + index upload %.1f KiB | buffer growths %llu | Map failures %llu",
            count(Engine::EProfilerCounter::ImGuiVertices), count(Engine::EProfilerCounter::ImGuiIndices),
            (count(Engine::EProfilerCounter::ImGuiVertexUploadBytes) + count(Engine::EProfilerCounter::ImGuiIndexUploadBytes)) / 1024.0,
            count(Engine::EProfilerCounter::ImGuiBufferGrowths), count(Engine::EProfilerCounter::ImGuiBufferMapFailures));
    }
    for (const auto& row : m_GpuAggregates)
        if (std::string_view(Scope_Name(row.NameId)) == "ImGui.RenderDrawData")
            ImGui::Text("GPU drawing interval: avg %.3f ms / P95 %.3f ms (%zu complete frames; all viewports)",
                row.InclusiveMs / static_cast<double>((std::max)(m_iGpuValidFrames, size_t{1})), row.P95FrameMs, m_iGpuValidFrames);
    if (ImGui::TreeNode("Backend workload counters"))
    {
        if (ImGui::BeginTable("##ImGuiWorkload", 2, TABLE_FLAGS, ImVec2(0.f, 160.f)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Counter"); ImGui::TableSetupColumn("Latest CPU frame"); ImGui::TableHeadersRow();
            for (size_t i = static_cast<size_t>(Engine::EProfilerCounter::ImGuiDrawLists); i <= static_cast<size_t>(Engine::EProfilerCounter::ImGuiBufferMapFailures); ++i)
            {
                ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(COUNTER_LABELS[i]); ImGui::TableNextColumn();
                if (m_bLiveValid) ImGui::Text("%llu", static_cast<unsigned long long>(m_Live.Counters[i]));
                else ImGui::TextDisabled("--");
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
    Render_Bottlenecks(true);
}

void Client::CProfilerTool::Render_Gpu()
{
    ImGui::TextWrapped("GPU timestamps measure elapsed intervals, including possible waits for CPU submission. They are not GPU utilization. Nested passes overlap; do not add them. CPU-only animation/navigation have no GPU duration.");
    ImGui::Text("Complete GPU frames: %zu / %zu | partial frames excluded: %zu", m_iGpuValidFrames, m_iWindowFrames, m_iGpuPartialFrames);
    if (m_bLiveValid && !m_Live.GpuScopesSupported)
        ImGui::TextDisabled("GPU pass queries are unavailable. Whole-frame timing may still be available.");
    if (m_bLiveValid)
    {
        ImGui::Text("Latest CPU frame %llu: GPU %s", static_cast<unsigned long long>(m_Live.FrameNumber), Gpu_Status(m_Live.LatestFrameGpuStatus));
        if (m_Live.GpuValid) ImGui::Text("Last resolved GPU frame %llu | readback %u frames | omitted scopes %u",
            static_cast<unsigned long long>(m_Live.GpuFrameNumber), m_Live.GpuLatencyFrames, m_Live.DroppedGpuScopes);
    }
    if (m_bLiveValid && m_Live.GpuValid && ImGui::TreeNode("Last resolved frame intervals"))
    {
        if (ImGui::BeginTable("##GpuIntervals", 4, TABLE_FLAGS, ImVec2(0, 180.f)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Pass", ImGuiTableColumnFlags_WidthStretch, 3.f);
            for (const char* label : {"Begin ms", "End ms", "Elapsed ms"}) ImGui::TableSetupColumn(label);
            ImGui::TableHeadersRow();
            for (const auto& row : m_Live.GpuScopes)
            {
                if (!Contains_CaseInsensitive(Scope_Name(row.NameId), m_Filter.data())) continue;
                ImGui::TableNextRow(); ImGui::TableNextColumn();
                ImGui::Text("%*s%s", static_cast<int>(row.Depth * 2), "", Scope_Name(row.NameId));
                ImGui::TableNextColumn(); ImGui::Text("%.3f", row.BeginMs);
                ImGui::TableNextColumn(); ImGui::Text("%.3f", row.EndMs);
                ImGui::TableNextColumn(); ImGui::Text("%.3f", row.DurationMs);
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
    ImGui::TextWrapped("PS / VS count shader invocations, not arithmetic instructions. Selected passes only; -- means unavailable or incomplete. Nested counts must not be added.");
    if (!ImGui::BeginTable("##GpuPasses", 7, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Pass", ImGuiTableColumnFlags_WidthStretch, 3.f);
    for (const char* label : {"Avg ms/frame", "Peak frame ms", "P95 frame ms", "Calls/frame", "PS / frame", "VS / frame"}) ImGui::TableSetupColumn(label);
    ImGui::TableHeadersRow();
    const double frames = static_cast<double>((std::max)(m_iGpuValidFrames, size_t{1}));
    for (const auto& row : m_GpuAggregates)
    {
        if (!Contains_CaseInsensitive(Scope_Name(row.NameId), m_Filter.data())) continue;
        ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(Scope_Name(row.NameId));
        ImGui::TableNextColumn(); ImGui::Text("%.3f", row.InclusiveMs / frames);
        ImGui::TableNextColumn(); ImGui::Text("%.3f", row.MaxFrameMs);
        ImGui::TableNextColumn(); ImGui::Text("%.3f", row.P95FrameMs);
        ImGui::TableNextColumn(); ImGui::Text("%.2f", static_cast<double>(row.Calls) / frames);
        ImGui::TableNextColumn();
        if (row.PipelineSamples != 0 && row.PipelineSamples == row.Calls)
            ImGui::Text("%.0f", static_cast<double>(row.PSInvocations) / frames);
        else ImGui::TextDisabled("--");
        ImGui::TableNextColumn();
        if (row.PipelineSamples != 0 && row.PipelineSamples == row.Calls)
            ImGui::Text("%.0f", static_cast<double>(row.VSInvocations) / frames);
        else ImGui::TextDisabled("--");
    }
    if (m_bShowUnobserved)
        for (const char* name : GPU_SCOPE_CATALOG)
            if (Contains_CaseInsensitive(name, m_Filter.data()) &&
                std::none_of(m_GpuAggregates.begin(), m_GpuAggregates.end(),
                    [&](const auto& row) { return name == std::string_view(Scope_Name(row.NameId)); }))
                Unobserved_Row(name, 7);
    ImGui::EndTable();
}

void Client::CProfilerTool::Render_Counters() const
{
    if (!m_bLiveValid) { ImGui::TextDisabled("No captured frame yet."); return; }
    if (!ImGui::BeginChild("##WorkloadScroll")) { ImGui::EndChild(); return; }
    const auto& animation = m_Live.Animation;
    ImGui::Text("CPU frame %llu", static_cast<unsigned long long>(m_Live.FrameNumber));
    ImGui::Text("Animation evaluation: %.3f ms | %llu calls / %llu models", animation.CpuMs,
        static_cast<unsigned long long>(animation.UpdateCalls), static_cast<unsigned long long>(animation.UpdatedModels));
    ImGui::Text("Updated, not submitted: %llu models / %.3f ms", static_cast<unsigned long long>(animation.NotSubmittedUpdatedModels), animation.NotSubmittedCpuMs);
    if (animation.DroppedSamples) ImGui::Text("Animation sample limit: %llu omitted in this frame", static_cast<unsigned long long>(animation.DroppedSamples));
    ImGui::TextWrapped("Not submitted means no successful model draw in this frame. It can include hidden actors, tool previews or offscreen objects; it is not a frustum test. History sampling is measured separately in CPU sections.");
    if (m_Live.GpuValid)
        ImGui::Text("GPU frame %llu: IA vertices %llu | VS %llu | PS %llu | primitives %llu",
            static_cast<unsigned long long>(m_Live.GpuFrameNumber),
            static_cast<unsigned long long>(m_Live.Pipeline.IAVertices), static_cast<unsigned long long>(m_Live.Pipeline.VSInvocations),
            static_cast<unsigned long long>(m_Live.Pipeline.PSInvocations), static_cast<unsigned long long>(m_Live.Pipeline.IAPrimitives));
    ImGui::TextWrapped("Navigation timings below are Client-side queries. Authoritative Server navigation runs in another process. Texture cache counters have no producer yet and are shown as N/A.");
    if (ImGui::BeginTable("##WorkCounters", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableSetupColumn("Counter"); ImGui::TableSetupColumn("Latest CPU frame"); ImGui::TableHeadersRow();
        for (size_t i = 0; i < COUNTER_LABELS.size(); ++i)
        {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(COUNTER_LABELS[i]);
            ImGui::TableNextColumn();
            if (i >= static_cast<size_t>(Engine::EProfilerCounter::TextureRequests) && i <= static_cast<size_t>(Engine::EProfilerCounter::TextureEstimatedGpuBytes))
                ImGui::TextDisabled("N/A");
            else ImGui::Text("%llu", static_cast<unsigned long long>(m_Live.Counters[i]));
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void Client::CProfilerTool::Render_LongOperations()
{
    ImGui::TextWrapped("Completed calls of %.0f ms or more, newest first. Worker calls spanning frames belong to the frame in which they ended. They cannot be summed as main-thread frame time.", Engine::CProfiler::LONG_OPERATION_THRESHOLD_MS);
    if (!ImGui::BeginTable("##LongOperations", 4, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Section", ImGuiTableColumnFlags_WidthStretch, 3.f);
    for (const char* label : {"Thread", "Duration ms", "Completion frame"}) ImGui::TableSetupColumn(label);
    ImGui::TableHeadersRow();
    for (auto row = m_LongOperations.rbegin(); row != m_LongOperations.rend(); ++row)
    {
        if (!Contains_CaseInsensitive(Scope_Name(row->NameId), m_Filter.data())) continue;
        ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(Scope_Name(row->NameId));
        ImGui::TableNextColumn(); ImGui::TextUnformatted(Thread_Label(row->ThreadId).c_str());
        ImGui::TableNextColumn(); ImGui::Text("%.3f", row->DurationMs);
        ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(row->FrameNumber));
    }
    ImGui::EndTable();
}


bool_t Client::CProfilerTool::Refresh_CaptureFiles()
{
    m_bCaptureFilesLoaded = true;
    if (!CProfilerCaptureIO::List_JsonFiles(CProfilerCaptureIO::Get_CaptureDirectory(),
        m_CaptureFiles, &m_strCaptureFilesStatus)) return false;
    if (std::none_of(m_CaptureFiles.begin(), m_CaptureFiles.end(),
        [&](const auto& file) { return file.StableId == m_strSelectedCaptureId; }))
        m_strSelectedCaptureId.clear();
    return true;
}

void Client::CProfilerTool::Render_CaptureFiles()
{
    if (ImGui::Button("Refresh files")) Refresh_CaptureFiles();
    ImGui::SameLine();
    const auto selected = std::find_if(m_CaptureFiles.begin(), m_CaptureFiles.end(),
        [&](const auto& file) { return file.StableId == m_strSelectedCaptureId; });
    ImGui::BeginDisabled(selected == m_CaptureFiles.end() || m_Exporter.IsSaving());
    if (ImGui::Button("Delete selected JSON") && selected != m_CaptureFiles.end())
    {
        const FProfilerCaptureFile file = *selected;
        std::string error;
        if (CProfilerCaptureIO::Delete_JsonFile(CProfilerCaptureIO::Get_CaptureDirectory(), file, &error))
        {
            m_strSelectedCaptureId.clear();
            std::erase_if(m_CaptureFiles, [&](const auto& row) { return row.StableId == file.StableId; });
            const bool refreshed = Refresh_CaptureFiles();
            m_strCaptureFilesStatus = "Deleted " + file.DisplayName +
                (refreshed ? std::string{} : ". Refresh failed: " + m_strCaptureFilesStatus);
        }
        else m_strCaptureFilesStatus = error;
    }
    ImGui::EndDisabled();
    ImGui::SameLine(); ImGui::TextDisabled("%zu files", m_CaptureFiles.size());
    ImGui::TextWrapped("%s", Capture_PathLabel(CProfilerCaptureIO::Get_CaptureDirectory()).c_str());
    if (!m_strCaptureFilesStatus.empty()) ImGui::TextWrapped("%s", m_strCaptureFilesStatus.c_str());
    if (!ImGui::BeginTable("##SavedCaptures", 3, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch, 5.f);
    ImGui::TableSetupColumn("Size (KiB)"); ImGui::TableSetupColumn("Modified"); ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_CaptureFiles.size()));
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& file = m_CaptureFiles[static_cast<size_t>(i)];
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            ImGui::PushID(file.StableId.c_str());
            const ImVec2 labelPosition = ImGui::GetCursorScreenPos();
            if (ImGui::Selectable("##CaptureFile", file.StableId == m_strSelectedCaptureId,
                ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.f, ImGui::GetTextLineHeight())))
                m_strSelectedCaptureId = file.StableId;
            ImGui::GetWindowDrawList()->AddText(labelPosition, ImGui::GetColorU32(ImGuiCol_Text), file.DisplayName.c_str());
            ImGui::PopID();
            ImGui::TableNextColumn(); ImGui::Text("%.1f", file.SizeBytes / 1024.0);
            ImGui::TableNextColumn();
            const FILETIME utc{static_cast<DWORD>(file.LastWriteTicks), static_cast<DWORD>(file.LastWriteTicks >> 32u)};
            FILETIME local{}; SYSTEMTIME time{};
            if (FileTimeToLocalFileTime(&utc, &local) && FileTimeToSystemTime(&local, &time))
                ImGui::Text("%04u-%02u-%02u %02u:%02u:%02u", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
            else ImGui::TextDisabled("--");
        }
    ImGui::EndTable();
}
