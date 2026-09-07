#include "imgui.h"
#include "WorldObjectTool.h"

#ifdef _DEBUG
#include "CompositionTimeline.h"
#include "Level_KakulSaydonArena.h"
#include "ProjectDataRoot.h"
#include "WorldSequencePlayer.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>

namespace
{
constexpr const char* AREA_ID = "LV_LUT_MIDNIGHTC_ED";

bool ReadSource(const std::filesystem::path& path, std::string& bytes,
    std::string& status, const bool optional = false)
{
    std::error_code error;
    if (optional && !std::filesystem::exists(path, error) && !error)
    { bytes.clear(); return true; }
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > 64u * 1024u * 1024u)
    { status = "Cannot read bounded authoring source: " + path.string(); return false; }
    std::ifstream input(path, std::ios::binary);
    if (!input) { status = "Cannot open source: " + path.string(); return false; }
    bytes.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    if (input.bad() || bytes.size() != size)
    { status = "Source changed or failed while reading: " + path.string(); return false; }
    return true;
}

std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

bool EditText(const char* label, std::string& value, const size_t maxBytes = 128)
{
    char text[1024]{};
    std::snprintf(text, sizeof(text), "%s", value.c_str());
    if (!ImGui::InputText(label, text, (std::min)(sizeof(text), maxBytes + 1))) return false;
    value = text;
    return true;
}

bool EditUInt(const char* label, uint32_t& value, const int maximum, const int minimum = 0)
{
    int number = static_cast<int>((std::min)(value, static_cast<uint32_t>(INT_MAX)));
    if (!ImGui::DragInt(label, &number, 1.f, minimum, maximum, "%d", ImGuiSliderFlags_AlwaysClamp)) return false;
    value = static_cast<uint32_t>((std::clamp)(number, minimum, maximum));
    return true;
}

float3_t QuaternionEuler(const float4_t& q)
{
    float4x4_t matrix;
    XMStoreFloat4x4(&matrix, XMMatrixRotationQuaternion(XMLoadFloat4(&q)));
    const float pitch = std::asin((std::clamp)(-matrix._32, -1.f, 1.f));
    const float cosine = std::cos(pitch);
    const float yaw = std::abs(cosine) > .00001f ? std::atan2(matrix._31, matrix._33) : std::atan2(-matrix._13, matrix._11);
    const float roll = std::abs(cosine) > .00001f ? std::atan2(matrix._12, matrix._22) : 0.f;
    return {XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll)};
}
}

using namespace Client;

CWorldObjectTool::~CWorldObjectTool()
{
    Stop_Preview();
    if (m_PublishProcess) CloseHandle(m_PublishProcess);
}

void CWorldObjectTool::Open()
{
    m_Open = true;
    m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
    if (!m_Ready) Load_Source();
}

bool CWorldObjectTool::Consume_InteractionRequest()
{
    const bool requested = m_InteractionRequested;
    m_InteractionRequested = false;
    return requested;
}

void CWorldObjectTool::Deactivate()
{
    Stop_Preview();
}

bool CWorldObjectTool::Load_Source()
{
    const auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level)
    { m_Status = "Enter KoukuSaydon, then Reload Source to edit and preview world objects."; return false; }
    const auto directory = CProjectDataRoot::Resolve(std::filesystem::path("Maps/Authoring") / AREA_ID);
    const auto sourcePath = directory / (std::string(AREA_ID) + ".worldsequences.json");
    const auto placementPath = directory / (std::string(AREA_ID) + ".mapplacements");
    const auto deployPath = directory / (std::string(AREA_ID) + ".deployplacements");
    std::string sourceBefore, mapBefore, deployBefore;
    if (!ReadSource(sourcePath, sourceBefore, m_Status) ||
        !ReadSource(placementPath, mapBefore, m_Status) ||
        !ReadSource(deployPath, deployBefore, m_Status, true)) return false;
    WORLD_SEQUENCE_PLACEMENT_MAP map;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    level->Get_WorldObjectValidationTargets(map, deploy);
    CWorldSequenceDocument staged;
    if (!staged.Load(sourcePath, AREA_ID, map, deploy, m_Status)) return false;
    std::string sourceAfter, mapAfter, deployAfter;
    if (!ReadSource(sourcePath, sourceAfter, m_Status) ||
        !ReadSource(placementPath, mapAfter, m_Status) ||
        !ReadSource(deployPath, deployAfter, m_Status, true)) return false;
    if (sourceBefore != sourceAfter || mapBefore != mapAfter || deployBefore != deployAfter)
    { m_Status = "Linked authoring source changed during Reload; existing draft preserved."; return false; }
    Stop_Preview();
    m_Document = std::move(staged);
    m_SavedDocument = m_Document;
    m_MapTargets = std::move(map);
    m_DeployTargets = std::move(deploy);
    m_SourcePath = sourcePath; m_PlacementPath = placementPath; m_DeployPath = deployPath;
    m_SourceBytes = std::move(sourceAfter); m_PlacementBytes = std::move(mapAfter); m_DeployBytes = std::move(deployAfter);
    m_Ready = true; m_Dirty = false; ++m_SavedGeneration;
    if (!m_Document.Find_ObjectResource(m_SelectedObject))
        m_SelectedObject = m_Document.Get_ObjectResources().empty() ? "" : m_Document.Get_ObjectResources().front().objectId;
    Select_Object(m_SelectedObject);
    m_Status = "Source loaded. Save updates this Area's worldsequences document; Publish applies it to Product.";
    return true;
}

bool CWorldObjectTool::Matches_SourceBaseline()
{
    std::string source, map, deploy;
    if (!ReadSource(m_SourcePath, source, m_Status) ||
        !ReadSource(m_PlacementPath, map, m_Status) ||
        !ReadSource(m_DeployPath, deploy, m_Status, true)) return false;
    if (source != m_SourceBytes || map != m_PlacementBytes || deploy != m_DeployBytes)
    { m_Status = "Save conflict: linked source changed on disk. Draft preserved; Reload Source before saving."; return false; }
    return true;
}

bool CWorldObjectTool::Save_Source()
{
    if (!m_Ready || m_PublishProcess || !Matches_SourceBaseline()) return false;
    auto stagedPath = m_SourcePath;
    stagedPath += L".world-object-" + std::to_wstring(GetCurrentProcessId()) + L".stage";
    // The document owns codec/validation. A separate staging destination permits
    // a readback and the final linked-source CAS immediately before promotion.
    if (!m_Document.Save(stagedPath, m_MapTargets, m_DeployTargets, m_Status)) return false;
    CWorldSequenceDocument verified;
    std::string stagedBytes;
    const bool ready = verified.Load(stagedPath, AREA_ID, m_MapTargets, m_DeployTargets, m_Status) &&
        m_Document.Is_Equivalent(verified) && ReadSource(stagedPath, stagedBytes, m_Status) && Matches_SourceBaseline();
    if (!ready || !MoveFileExW(stagedPath.c_str(), m_SourcePath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        std::error_code ignored; std::filesystem::remove(stagedPath, ignored);
        if (ready) m_Status = "Atomic source replacement failed; draft preserved.";
        else if (m_Status.empty()) m_Status = "Staged source readback differs; draft preserved.";
        return false;
    }
    m_SourceBytes = std::move(stagedBytes); m_Document = std::move(verified);
    m_SavedDocument = m_Document; m_Dirty = false; ++m_SavedGeneration;
    m_Status = "Object resources and states saved. Map/deploy placement files were preserved. Publish to apply to Product.";
    return true;
}

void CWorldObjectTool::Start_Publish()
{
    if (!m_Ready || m_Dirty || m_PublishProcess || !Matches_SourceBaseline()) return;
    const auto root = CProjectDataRoot::Get().parent_path();
    const auto script = root / L"Tools/MapPipeline/Publish-MapAuthoring.ps1";
    if (!std::filesystem::is_regular_file(script)) { m_Status = "Map authoring publisher is missing."; return; }
    wchar_t temporary[MAX_PATH]{};
    if (!GetTempPathW(MAX_PATH, temporary)) { m_Status = "Publisher log folder is unavailable."; return; }
    m_PublishLog = std::filesystem::path(temporary) / (L"LostArk-WorldObject-" + std::to_wstring(GetCurrentProcessId()) + L".log");
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    const HANDLE log = CreateFileW(m_PublishLog.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
        &security, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == log) { m_Status = "Cannot create publisher log."; return; }
    const HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == input) { CloseHandle(log); m_Status = "Cannot prepare publisher input."; return; }
    std::wstring command = L"powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" +
        script.wstring() + L"\" -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish";
    std::vector<wchar_t> arguments(command.begin(), command.end()); arguments.push_back(0);
    STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = log; startup.hStdError = log; startup.hStdInput = input;
    PROCESS_INFORMATION process{};
    const bool started = !!CreateProcessW(nullptr, arguments.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, root.c_str(), &startup, &process);
    CloseHandle(log); CloseHandle(input);
    if (!started) { m_Status = "Cannot start Map authoring publisher."; return; }
    CloseHandle(process.hThread); m_PublishProcess = process.hProcess;
    m_Status = "Publishing saved Area data. Log: " + m_PublishLog.string();
}

void CWorldObjectTool::Poll_Publish()
{
    if (!m_PublishProcess || WaitForSingleObject(m_PublishProcess, 0) == WAIT_TIMEOUT) return;
    DWORD code = 1; GetExitCodeProcess(m_PublishProcess, &code);
    CloseHandle(m_PublishProcess); m_PublishProcess = nullptr;
    if (code != 0)
    { m_Status = "Map publish failed (" + std::to_string(code) + "). Runtime preserved. Log: " + m_PublishLog.string(); return; }
    Stop_Preview();
    if (auto* level = CLevel_KakulSaydonArena::Get_Active())
    {
        std::string status;
        if (!level->Reload_WorldObjectRuntime(status))
        { m_Status = "Published; live runtime reload failed: " + status; return; }
    }
    m_Status = "Map publish succeeded; world object runtime reloaded for the next play. Log: " + m_PublishLog.string();
}

void CWorldObjectTool::Mark_Dirty()
{
    m_Document.Touch(); m_Dirty = true; m_PreviewDirty = m_PreviewActive;
}

void CWorldObjectTool::Stop_Preview()
{
    if (m_PreviewActive && m_PreviewLevel && m_PreviewLevel == CLevel_KakulSaydonArena::Get_Active())
        m_PreviewLevel->Debug_StopWorldObjectPreview();
    m_PreviewLevel = nullptr; m_PreviewActive = false; m_Playing = false; m_PreviewDirty = false;
}

bool CWorldObjectTool::Begin_Preview()
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level) { m_Status = "World object preview requires the active KoukuSaydon arena."; return false; }
    if (!m_Document.Find_Instance(m_SelectedInstance)) { m_Status = "Select or create an object state."; return false; }
    if (!level->Debug_BeginWorldObjectPreview(m_Document, m_SelectedInstance, m_Status, m_PreviewAtCharacter)) return false;
    m_PreviewLevel = level; m_PreviewActive = true; m_PreviewDirty = false;
    return true;
}

f32_t CWorldObjectTool::SpanMs() const
{
    const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    return !sequence ? 0.f : static_cast<float>(instance->startDelayMs) +
        static_cast<float>(sequence->durationMs) / (std::max)(.05f, instance->playbackSpeed);
}

void CWorldObjectTool::Seek(const f32_t clockMs)
{
    m_ClockMs = (std::clamp)(clockMs, 0.f, SpanMs());
    if ((!m_PreviewActive || m_PreviewDirty) && !Begin_Preview()) { m_Playing = false; return; }
    if (!m_PreviewLevel->Debug_SampleWorldObjectPreview(m_ClockMs, m_PreviewStatus))
    { m_Status = m_PreviewStatus; Stop_Preview(); }
}

void CWorldObjectTool::Update(const f32_t seconds, const bool_t active)
{
    Poll_Publish();
    if (!active || !m_Open || (m_PreviewLevel && m_PreviewLevel != CLevel_KakulSaydonArena::Get_Active()))
    { Stop_Preview(); return; }
    if (m_PreviewActive && m_PreviewDirty) Seek(m_ClockMs);
    if (!m_Playing || !m_PreviewActive) return;
    const float span = SpanMs();
    m_ClockMs += (std::max)(0.f, seconds) * 1000.f;
    if (span <= 0.f) { Stop_Preview(); return; }
    if (m_ClockMs >= span)
    {
        if (m_Loop) m_ClockMs = std::fmod(m_ClockMs, span);
        else { m_ClockMs = span; m_Playing = false; }
    }
    Seek(m_ClockMs);
}

std::vector<std::string> CWorldObjectTool::StateIds(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource) const
{
    if (!resource.sequenceInstanceId.empty()) return {resource.sequenceInstanceId};
    std::vector<std::string> ids;
    for (const auto& instance : m_Document.Get_Instances())
        if (std::any_of(instance.bindings.begin(), instance.bindings.end(), [&](const auto& binding) {
            return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE && binding.targetId == resource.objectId;
        })) ids.push_back(instance.instanceId);
    return ids;
}

void CWorldObjectTool::Select_Object(const std::string& id)
{
    Stop_Preview(); m_SelectedObject = id; m_SelectedInstance.clear(); m_ClockMs = 0.f;
    const auto* resource = m_Document.Find_ObjectResource(id);
    if (resource)
    {
        const auto states = StateIds(*resource);
        if (!states.empty()) Select_State(states.front());
    }
}

void CWorldObjectTool::Select_State(const std::string& id)
{
    Stop_Preview(); m_SelectedInstance = id; m_SelectedTrack = 0; m_SelectedKey = 0; m_ClockMs = 0.f;
}

bool CWorldObjectTool::Create_Object()
{
    if (!m_NewObjectName[0]) { m_Status = "Enter an object name."; return false; }
    WORLD_SEQUENCE_OBJECT_RESOURCE resource;
    for (uint32_t index = 1; index < UINT32_MAX; ++index)
    {
        resource.objectId = "world.object.resource." + std::to_string(index);
        if (!m_Document.Find_ObjectResource(resource.objectId)) break;
    }
    resource.displayName = m_NewObjectName.data();
    resource.anchorKind = m_NewObjectAnchor == 1 ? "PLAYER" : "WORLD";
    WORLD_SEQUENCE_TEMPLATE sequence;
    WORLD_SEQUENCE_INSTANCE instance;
    if (!Build_State(resource, "Default", sequence, instance)) return false;
    m_Document.Get_ObjectResources().push_back(resource);
    m_Document.Get_Templates().push_back(std::move(sequence));
    m_Document.Get_Instances().push_back(std::move(instance));
    Mark_Dirty(); Select_Object(resource.objectId);
    m_NewObjectName[0] = 0;
    m_NewStateName[0] = 0;
    return true;
}

void CWorldObjectTool::Create_State()
{
    auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (!resource || !resource->sequenceInstanceId.empty() || !m_NewStateName[0]) return;
    WORLD_SEQUENCE_TEMPLATE sequence;
    WORLD_SEQUENCE_INSTANCE instance;
    if (!Build_State(*resource, m_NewStateName.data(), sequence, instance)) return;
    m_Document.Get_Templates().push_back(std::move(sequence));
    m_Document.Get_Instances().push_back(instance); Mark_Dirty(); Select_State(instance.instanceId);
    m_NewStateName[0] = 0;
}

bool CWorldObjectTool::Build_State(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
    const std::string& stateName, WORLD_SEQUENCE_TEMPLATE& sequence, WORLD_SEQUENCE_INSTANCE& instance)
{
    if (m_Document.Get_Templates().size() >= CWorldSequenceDocument::MAX_TEMPLATE_COUNT ||
        m_Document.Get_Instances().size() >= CWorldSequenceDocument::MAX_INSTANCE_COUNT)
    { m_Status = "World sequence document capacity reached."; return false; }
    for (uint32_t index = 1; index < UINT32_MAX; ++index)
    {
        sequence.sequenceId = resource.objectId + ".state." + std::to_string(index);
        instance.instanceId = sequence.sequenceId + ".instance";
        if (!m_Document.Find_Template(sequence.sequenceId) && !m_Document.Find_Instance(instance.instanceId)) break;
    }
    sequence.displayName = stateName; sequence.durationMs = 2000;
    WORLD_SEQUENCE_TRACK track; track.slotId = "object"; track.keys.push_back({});
    WORLD_SEQUENCE_TRANSFORM_KEY end; end.timeMs = sequence.durationMs; track.keys.push_back(end);
    sequence.tracks.push_back(track);
    instance.templateId = sequence.sequenceId;
    instance.anchorKind = resource.anchorKind;
    instance.bindings.push_back({"object", WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE, resource.objectId});
    if (instance.anchorKind == "WORLD")
    {
        auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level)
        { m_Status = "Map state creation requires the active KoukuSaydon arena."; return false; }
        if (!level->Try_Get_AuthoringPreviewPlacement(instance.position, m_Status))
        {
            m_Status = "Map state needs the current character placement: " + m_Status;
            return false;
        }
    }
    return true;
}

void CWorldObjectTool::Change_ResourceAnchor(
    WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& anchorKind)
{
    if (resource.anchorKind == anchorKind || !resource.sequenceInstanceId.empty()) return;
    float3_t position{};
    if (anchorKind == "WORLD")
    {
        auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level || !level->Try_Get_AuthoringPreviewPlacement(position, m_Status))
        { m_Status = "Map anchor needs the current character placement: " + m_Status; return; }
    }
    resource.anchorKind = anchorKind;
    for (const auto& id : StateIds(resource))
    {
        auto* instance = m_Document.Find_Instance(id);
        if (!instance || instance->anchorKind == anchorKind) continue;
        instance->anchorKind = anchorKind;
        instance->position = position;
    }
    Mark_Dirty();
    m_Status = anchorKind == "PLAYER" ?
        "Character anchor applied to this resource's states; offsets start at the character origin." :
        "Map anchor applied to this resource's states at the current character position.";
}

void CWorldObjectTool::Render()
{
    if (!m_Open) return;
    const auto* viewport = ImGui::GetMainViewport();
    const ImVec2 origin = viewport ? viewport->WorkPos : ImVec2(0.f, 0.f);
    const ImVec2 available = viewport ? viewport->WorkSize : ImVec2(1600.f, 900.f);
    constexpr float margin = 8.f, gap = 8.f;
    const float width = (std::max)(1.f, available.x - margin * 2.f - gap * 2.f);
    const float height = (std::max)(1.f, available.y - margin * 2.f);
    const float leftWidth = width * .23f, rightWidth = width * .24f;
    const float centerWidth = width - leftWidth - rightWidth;
    const float leftX = origin.x + margin, centerX = leftX + leftWidth + gap;
    const float rightX = centerX + centerWidth + gap, topY = origin.y + margin;
    const ImGuiCond condition = m_ResetLayoutRequested ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
    m_ResetLayoutRequested = false;
    const auto beginPane = [&](const char* name, bool& visible, const ImVec2 position, const ImVec2 size)
    {
        ImGui::SetNextWindowPos(position, condition);
        ImGui::SetNextWindowSize(size, condition);
        const bool expanded = ImGui::Begin(name, &visible, ImGuiWindowFlags_MenuBar);
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsMouseClicked(0))
            m_InteractionRequested = true;
        if (expanded) Render_WindowMenu();
        return expanded;
    };

    if (m_ResourcesOpen)
    {
        if (beginPane("Object Resources###WorldObjectResourcesWindow", m_ResourcesOpen,
            {leftX, topY}, {leftWidth, height}))
        {
            if (m_Ready) Render_Resources();
            else
            {
                if (ImGui::Button("Reload Source")) Load_Source();
                ImGui::TextWrapped("%s", m_Status.c_str());
            }
            Render_PhysicalResources();
        }
        ImGui::End();
    }
    if (m_SequencerOpen)
    {
        if (beginPane("Object Sequencer###WorldObjectSequencerWindow", m_SequencerOpen,
            {centerX, topY + height * .57f}, {centerWidth, height * .43f}))
        {
            Render_Toolbar();
            auto* instance = m_Document.Find_Instance(m_SelectedInstance);
            auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
            if (sequence) Render_Sequence(*sequence);
            else ImGui::TextDisabled("Select an object and state in Object Resources.");
        }
        ImGui::End();
    }
    if (m_DetailOpen)
    {
        if (beginPane("Object Detail###WorldObjectDetailWindow", m_DetailOpen,
            {rightX, topY}, {rightWidth, height}))
        {
            if (m_Ready) Render_Detail();
            else ImGui::TextDisabled("Load Object Resources to edit an object.");
        }
        ImGui::End();
    }
    if (!m_ResourcesOpen && !m_SequencerOpen && !m_DetailOpen) m_Open = false;
    if (!m_Open) Stop_Preview();
}

void CWorldObjectTool::Render_WindowMenu()
{
    if (!ImGui::BeginMenuBar()) return;
    if (ImGui::BeginMenu("Windows"))
    {
        ImGui::MenuItem("Object Resources", nullptr, &m_ResourcesOpen);
        ImGui::MenuItem("Object Sequencer", nullptr, &m_SequencerOpen);
        ImGui::MenuItem("Object Detail", nullptr, &m_DetailOpen);
        ImGui::Separator();
        if (ImGui::MenuItem("Show All")) m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
        if (ImGui::MenuItem("Reset Window Layout"))
        {
            m_ResourcesOpen = m_SequencerOpen = m_DetailOpen = true;
            m_ResetLayoutRequested = true;
        }
        if (ImGui::MenuItem("Close World Object Tool")) m_Open = false;
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

void CWorldObjectTool::Render_Toolbar()
{
    if (ImGui::Button("Reload Source"))
    {
        if (m_Dirty) ImGui::OpenPopup("Reload object source?");
        else Load_Source();
    }
    if (ImGui::BeginPopupModal("Reload object source?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Reload discards this tool's unsaved object/state edits.");
        if (ImGui::Button("Discard and Reload")) { Load_Source(); ImGui::CloseCurrentPopup(); }
        ImGui::SameLine(); if (ImGui::Button("Keep Editing")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::SameLine(); ImGui::BeginDisabled(!m_Ready || m_PublishProcess);
    if (ImGui::Button(m_Dirty ? "Save *" : "Save")) Save_Source();
    ImGui::EndDisabled(); ImGui::SameLine();
    ImGui::BeginDisabled(!m_Ready || m_Dirty || m_PublishProcess);
    if (ImGui::Button("Publish Area")) Start_Publish();
    ImGui::SameLine();
    if (ImGui::Button("Reload Runtime"))
    {
        Stop_Preview();
        if (auto* level = CLevel_KakulSaydonArena::Get_Active()) level->Reload_WorldObjectRuntime(m_Status);
        else m_Status = "Enter KoukuSaydon to reload the active world object runtime.";
    }
    ImGui::EndDisabled();
    if (!m_Status.empty()) ImGui::TextWrapped("%s", m_Status.c_str());
}

void CWorldObjectTool::Render_Resources()
{
    if (ImGui::Button("Create Object"))
    {
        m_CreateObjectFailed = false;
        ImGui::OpenPopup("Create Object Resource");
    }
    ImGui::SameLine(); ImGui::TextDisabled("%zu resources", m_Document.Get_ObjectResources().size());
    if (ImGui::BeginPopupModal("Create Object Resource", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputTextWithHint("Name", "New object name", m_NewObjectName.data(), m_NewObjectName.size());
        ImGui::Combo("Anchor Type", &m_NewObjectAnchor, "Map\0Character\0");
        ImGui::TextUnformatted(m_NewObjectAnchor == 0 ?
            "Map: seed the state at the current character position, then keep it fixed." :
            "Character: create at each living character with a local offset.");
        ImGui::BeginDisabled(!m_NewObjectName[0]);
        if (ImGui::Button("Create"))
        {
            m_CreateObjectFailed = !Create_Object();
            if (!m_CreateObjectFailed) ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled(); ImGui::SameLine();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        if (m_CreateObjectFailed) ImGui::TextWrapped("%s", m_Status.c_str());
        ImGui::EndPopup();
    }
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##ObjectSearch", "Search object or state", m_ObjectSearch.data(), m_ObjectSearch.size());
    const float treeHeight = (std::max)(180.f, ImGui::GetContentRegionAvail().y * .48f);
    if (ImGui::BeginChild("ObjectResourceTree", ImVec2(0.f, treeHeight), true))
    {
        const auto search = Lower(m_ObjectSearch.data());
        for (const char* anchor : {"WORLD", "PLAYER"})
        {
            const char* category = std::string(anchor) == "WORLD" ? "Map" : "Character";
            size_t count = 0;
            for (const auto& resource : m_Document.Get_ObjectResources()) if (resource.anchorKind == anchor) ++count;
            const std::string categoryLabel = std::string(category) + " (" + std::to_string(count) + ")";
            if (!ImGui::TreeNodeEx(anchor, ImGuiTreeNodeFlags_DefaultOpen, "%s", categoryLabel.c_str())) continue;
            for (const auto& resource : m_Document.Get_ObjectResources())
            {
                if (resource.anchorKind != anchor) continue;
                const auto states = StateIds(resource);
                bool matches = search.empty() || Lower(resource.displayName + " " + resource.objectId).find(search) != std::string::npos;
                if (!matches)
                    for (const auto& id : states)
                    {
                        const auto* state = m_Document.Find_Instance(id);
                        const auto* sequence = state ? m_Document.Find_Template(state->templateId) : nullptr;
                        if (sequence && Lower(sequence->displayName).find(search) != std::string::npos) { matches = true; break; }
                    }
                if (!matches) continue;
                ImGui::PushID(resource.objectId.c_str());
                const auto label = resource.displayName + (resource.sequenceInstanceId.empty() && resource.modelAssetId.empty() ? " [assign model]" : "");
                const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                    ImGuiTreeNodeFlags_SpanAvailWidth | (m_SelectedObject == resource.objectId ? ImGuiTreeNodeFlags_Selected : 0);
                if (!search.empty()) ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                const bool open = ImGui::TreeNodeEx("Resource", flags, "%s", label.c_str());
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) Select_Object(resource.objectId);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", resource.objectId.c_str());
                if (open)
                {
                    for (const auto& id : states)
                    {
                        const auto* instance = m_Document.Find_Instance(id);
                        const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
                        ImGui::PushID(id.c_str());
                        if (ImGui::Selectable(sequence ? sequence->displayName.c_str() : id.c_str(), id == m_SelectedInstance))
                        { m_SelectedObject = resource.objectId; Select_State(id); }
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", id.c_str());
                        ImGui::PopID();
                    }
                    if (states.empty()) ImGui::TextDisabled("No states");
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_Detail()
{
    auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    if (!resource) { ImGui::TextUnformatted("Select an object resource."); return; }
    bool changed = EditText("Object Name", resource->displayName);
    ImGui::TextDisabled("%s", resource->objectId.c_str());
    const bool alias = !resource->sequenceInstanceId.empty();
    int resourceAnchor = resource->anchorKind == "PLAYER" ? 1 : 0;
    ImGui::BeginDisabled(alias);
    if (ImGui::Combo("Anchor Type", &resourceAnchor, "Map\0Character\0"))
        Change_ResourceAnchor(*resource, resourceAnchor == 1 ? "PLAYER" : "WORLD");
    ImGui::EndDisabled();
    if (alias) ImGui::TextDisabled("Placed objects keep their Map anchor.");
    if (alias)
        ImGui::TextWrapped("Placed object sequence: %s. This editor updates its existing tracks and bindings.", resource->sequenceInstanceId.c_str());
    else
    {
        ImGui::TextWrapped("Model: %s", resource->modelAssetId.empty() ? "Choose a WModel in Physical Resources" : resource->modelAssetId.c_str());
        ImGui::TextWrapped("Diffuse: %s", resource->diffuseTextureAssetId.empty() ? "Embedded model material" : resource->diffuseTextureAssetId.c_str());
        if (!resource->diffuseTextureAssetId.empty() && ImGui::SmallButton("Clear Diffuse Override")) { resource->diffuseTextureAssetId.clear(); changed = true; }
        changed |= ImGui::DragFloat("Model Import Scale", &resource->modelPreScale, .001f, .000001f, 1000.f, "%.6f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat3("Object Scale", &resource->scale.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::Checkbox("Animated Model", &resource->animated);
        ImGui::SetNextItemWidth((std::max)(80.f, ImGui::GetContentRegionAvail().x - 90.f));
        ImGui::InputTextWithHint("##NewState", "New state name", m_NewStateName.data(), m_NewStateName.size());
        ImGui::SameLine();
        if (ImGui::Button("Add State")) { if (changed) Mark_Dirty(); Create_State(); return; }
    }
    auto* instance = m_Document.Find_Instance(m_SelectedInstance);
    auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!sequence) { if (changed) Mark_Dirty(); return; }
    ImGui::SeparatorText("State");
    changed |= EditText("State Name", sequence->displayName);
    ImGui::TextDisabled("%s", instance->instanceId.c_str());
    changed |= ImGui::Checkbox("Enabled", &instance->enabled);
    if (!alias)
    {
        ImGui::TextDisabled("Creation Anchor: %s", instance->anchorKind == "PLAYER" ? "Character" : "Map");
        changed |= ImGui::DragFloat3(instance->anchorKind == "PLAYER" ? "Character Offset" : "Map Position", &instance->position.x, .01f);
        if (ImGui::Button("Use Current Character Position"))
        {
            if (instance->anchorKind == "PLAYER") { instance->position = {}; changed = true; }
            else if (auto* level = CLevel_KakulSaydonArena::Get_Active()) changed |= level->Try_Get_AuthoringPreviewPlacement(instance->position, m_Status);
            else m_Status = "Player placement requires the active KoukuSaydon arena.";
        }
    }
    changed |= EditUInt("Start Delay (ms)", instance->startDelayMs, CWorldSequenceDocument::MAX_DURATION_MS);
    changed |= ImGui::DragFloat("Playback Speed", &instance->playbackSpeed, .01f, .05f, 8.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    uint32_t duration = sequence->durationMs;
    if (EditUInt("Lifetime (ms)", duration, CWorldSequenceDocument::MAX_DURATION_MS, 1))
    {
        // Preserve the key order and endpoint contract even when shortening a state.
        bool valid = true;
        for (const auto& track : sequence->tracks) if (duration + 1u < track.keys.size()) valid = false;
        for (const auto& animation : sequence->animationTracks)
            if (static_cast<size_t>(duration) < std::count_if(sequence->animationTracks.begin(), sequence->animationTracks.end(),
                [&](const auto& other) { return other.slotId == animation.slotId; })) valid = false;
        if (valid)
        {
            for (auto& track : sequence->tracks)
            {
                for (size_t key = 1; key + 1 < track.keys.size(); ++key)
                {
                    const auto scaled = static_cast<uint32_t>(std::llround(static_cast<double>(track.keys[key].timeMs) * duration / sequence->durationMs));
                    track.keys[key].timeMs = (std::clamp)(scaled, track.keys[key - 1].timeMs + 1,
                        duration - static_cast<uint32_t>(track.keys.size() - key - 1));
                }
                if (!track.keys.empty()) track.keys.back().timeMs = duration;
            }
            std::unordered_map<std::string, uint32_t> previousStarts;
            for (size_t index = 0; index < sequence->animationTracks.size(); ++index)
            {
                auto& animation = sequence->animationTracks[index];
                const auto previous = previousStarts.find(animation.slotId);
                const auto remaining = std::count_if(sequence->animationTracks.begin() + index + 1, sequence->animationTracks.end(),
                    [&](const auto& other) { return other.slotId == animation.slotId; });
                const uint32_t scaled = static_cast<uint32_t>(static_cast<double>(animation.startMs) * duration / sequence->durationMs);
                animation.startMs = previous == previousStarts.end() ? 0 : (std::clamp)(scaled,
                    previous->second + 1, duration - static_cast<uint32_t>(remaining) - 1);
                previousStarts[animation.slotId] = animation.startMs;
            }
            sequence->durationMs = duration;
            if (sequence->objectMotion.count > 1)
                sequence->objectMotion.intervalMs = (std::min)(sequence->objectMotion.intervalMs, (duration - 1) / (sequence->objectMotion.count - 1));
            changed = true;
        }
        else m_Status = "Lifetime must leave at least one millisecond between every existing key or clip.";
    }
    int interpolation = sequence->interpolation == WORLD_SEQUENCE_INTERPOLATION::LINEAR ? 0 : 1;
    if (ImGui::Combo("Interpolation", &interpolation, "Linear\0Smooth Step\0"))
    { sequence->interpolation = interpolation == 0 ? WORLD_SEQUENCE_INTERPOLATION::LINEAR : WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP; changed = true; }
    if (!alias && ImGui::CollapsingHeader("Motion / Emission", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& motion = sequence->objectMotion;
        changed |= ImGui::DragFloat3("Velocity (m/s)", &motion.velocity.x, .05f);
        changed |= ImGui::DragFloat3("Acceleration (m/s2)", &motion.acceleration.x, .05f);
        changed |= ImGui::DragFloat3("Self Rotation (deg/s)", &motion.angularVelocityDegrees.x, .5f);
        changed |= ImGui::DragFloat3("Revolution (deg/s)", &motion.revolutionDegreesPerSecond.x, .5f);
        changed |= ImGui::DragFloat3("Revolution Offset (m)", &motion.revolutionOffset.x, .05f);
        changed |= EditUInt("Count", motion.count, 128, 1);
        const uint32_t maxInterval = motion.count > 1 ? (sequence->durationMs - 1) / (motion.count - 1) : CWorldSequenceDocument::MAX_DURATION_MS;
        if (motion.intervalMs > maxInterval) { motion.intervalMs = maxInterval; changed = true; }
        changed |= EditUInt("Creation Interval (ms)", motion.intervalMs, maxInterval);
        changed |= ImGui::DragFloat("Spread (deg)", &motion.spreadDegrees, .5f, 0.f, 180.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        changed |= EditUInt("Seed", motion.seed, INT_MAX);
    }
    if (changed) Mark_Dirty();
    ImGui::SeparatorText("Selected Key");
    Render_KeyEditor(*sequence);
}

void CWorldObjectTool::Render_Sequence(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    ImGui::SeparatorText(sequence.displayName.c_str());
    if (ImGui::Button(m_Playing ? "Pause" : "Play"))
    {
        if (m_Playing) m_Playing = false;
        else { if (m_ClockMs >= SpanMs()) m_ClockMs = 0.f; Seek(m_ClockMs); m_Playing = m_PreviewActive; }
    }
    ImGui::SameLine(); if (ImGui::Button("Stop / Restore")) { Stop_Preview(); m_ClockMs = 0.f; }
    ImGui::SameLine(); ImGui::Checkbox("Loop", &m_Loop);
    const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
    const auto* selectedInstance = m_Document.Find_Instance(m_SelectedInstance);
    if (resource && resource->sequenceInstanceId.empty() && selectedInstance && selectedInstance->anchorKind == "WORLD")
    {
        ImGui::SameLine();
        if (ImGui::Checkbox("Preview at Character", &m_PreviewAtCharacter))
        {
            m_PreviewDirty = m_PreviewActive;
            if (m_PreviewActive) Seek(m_ClockMs);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Preview at the current character without changing the saved Map position. Clear to preview the authored position.");
    }
    if (m_PreviewActive) ImGui::TextWrapped("%s", m_PreviewStatus.c_str());
    float clock = m_ClockMs;
    if (ImGui::SliderFloat("Clock (ms)", &clock, 0.f, (std::max)(1.f, SpanMs()), "%.0f")) Seek(clock);
    ImGui::SetNextItemWidth(180.f); ImGui::SliderFloat("Timeline Zoom", &m_Zoom, 10.f, 500.f, "%.0f px/s");
    const float rowHeight = 32.f;
    const float width = (std::max)(ImGui::GetContentRegionAvail().x - 12.f, sequence.durationMs * m_Zoom * .001f);
    const float pixelsPerMs = width / sequence.durationMs;
    const float height = 28.f + rowHeight * static_cast<float>(sequence.tracks.size() + sequence.animationTracks.size());
    if (ImGui::BeginChild("ObjectTimeline", ImVec2(0, (std::max)(110.f, ImGui::GetContentRegionAvail().y)), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const auto origin = ImGui::GetCursorScreenPos(); auto* draw = ImGui::GetWindowDrawList();
        CompositionTimeline::DrawRuler(draw, origin, ImVec2(origin.x + width, origin.y + 25.f), sequence.durationMs, pixelsPerMs * 1000.f);
        ImGui::InvisibleButton("RulerSeek", ImVec2(width, 25.f));
        if (ImGui::IsItemActive())
        {
            const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
            const float local = (std::clamp)((ImGui::GetIO().MousePos.x - origin.x) / pixelsPerMs, 0.f, static_cast<float>(sequence.durationMs));
            if (instance) Seek(instance->startDelayMs + local / instance->playbackSpeed);
        }
        for (size_t index = 0; index < sequence.tracks.size(); ++index)
        {
            auto& track = sequence.tracks[index]; ImGui::PushID(track.slotId.c_str());
            const auto row = ImVec2(origin.x, origin.y + 28.f + rowHeight * index);
            CompositionTimeline::DrawBox(draw, row, ImVec2(row.x + width, row.y + 25.f),
                IM_COL32(61, 107, 141, 255), m_SelectedTrack == index, track.slotId.c_str(), false, false);
            if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(row, ImVec2(row.x + width, row.y + 25.f)) && ImGui::IsMouseClicked(0))
            { m_SelectedTrack = index; m_SelectedKey = 0; }
            for (size_t keyIndex = 0; keyIndex < track.keys.size(); ++keyIndex)
            {
                auto& key = track.keys[keyIndex]; const float x = row.x + key.timeMs * pixelsPerMs;
                const float y = row.y + 12.f;
                const ImU32 color = m_SelectedTrack == index && m_SelectedKey == keyIndex ? IM_COL32(255, 223, 87, 255) : IM_COL32_WHITE;
                draw->AddQuadFilled(ImVec2(x, y - 6), ImVec2(x + 6, y), ImVec2(x, y + 6), ImVec2(x - 6, y), color);
                ImGui::PushID(static_cast<int>(keyIndex)); ImGui::SetCursorScreenPos(ImVec2((std::clamp)(x - 7.f, row.x, row.x + width - 14.f), row.y + 4.f));
                ImGui::InvisibleButton("Key", ImVec2(14, 18));
                if (ImGui::IsItemClicked()) { m_SelectedTrack = index; m_SelectedKey = keyIndex; }
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0) && keyIndex > 0 && keyIndex + 1 < track.keys.size())
                {
                    const auto moved = static_cast<int>((ImGui::GetIO().MousePos.x - row.x) / pixelsPerMs);
                    const uint32_t time = static_cast<uint32_t>((std::clamp)(moved, static_cast<int>(track.keys[keyIndex - 1].timeMs + 1), static_cast<int>(track.keys[keyIndex + 1].timeMs - 1)));
                    if (time != key.timeMs) { key.timeMs = time; Mark_Dirty(); }
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s / %u ms", track.slotId.c_str(), key.timeMs);
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        for (size_t index = 0; index < sequence.animationTracks.size(); ++index)
        {
            const auto& track = sequence.animationTracks[index];
            uint32_t end = sequence.durationMs;
            for (const auto& next : sequence.animationTracks) if (next.slotId == track.slotId && next.startMs > track.startMs) end = (std::min)(end, next.startMs);
            const float y = origin.y + 28.f + rowHeight * (sequence.tracks.size() + index);
            CompositionTimeline::DrawBox(draw, ImVec2(origin.x + track.startMs * pixelsPerMs, y),
                ImVec2(origin.x + end * pixelsPerMs, y + 25.f), IM_COL32(113, 82, 147, 255), false, track.clipName.c_str());
        }
        const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
        const float local = instance ? (m_ClockMs - instance->startDelayMs) * instance->playbackSpeed : 0.f;
        const float cursorX = origin.x + (std::clamp)(local, 0.f, static_cast<float>(sequence.durationMs)) * pixelsPerMs;
        draw->AddLine(ImVec2(cursorX, origin.y), ImVec2(cursorX, origin.y + height), IM_COL32(255, 217, 68, 255), 2.f);
        ImGui::SetCursorScreenPos(origin); ImGui::Dummy(ImVec2(width, height));
    }
    ImGui::EndChild();
}

void CWorldObjectTool::Render_KeyEditor(WORLD_SEQUENCE_TEMPLATE& sequence)
{
    if (!sequence.tracks.empty())
    {
        m_SelectedTrack = (std::min)(m_SelectedTrack, sequence.tracks.size() - 1);
        if (ImGui::BeginCombo("Target Track", sequence.tracks[m_SelectedTrack].slotId.c_str()))
        {
            for (size_t index = 0; index < sequence.tracks.size(); ++index)
                if (ImGui::Selectable(sequence.tracks[index].slotId.c_str(), index == m_SelectedTrack)) { m_SelectedTrack = index; m_SelectedKey = 0; }
            ImGui::EndCombo();
        }
        auto& track = sequence.tracks[m_SelectedTrack];
        if (!track.keys.empty())
        {
            m_SelectedKey = (std::min)(m_SelectedKey, track.keys.size() - 1);
            auto& key = track.keys[m_SelectedKey];
            if (ImGui::BeginCombo("Keyframe", (std::to_string(key.timeMs) + " ms").c_str()))
            {
                for (size_t index = 0; index < track.keys.size(); ++index)
                    if (ImGui::Selectable((std::to_string(track.keys[index].timeMs) + " ms").c_str(), m_SelectedKey == index)) m_SelectedKey = index;
                ImGui::EndCombo();
            }
            auto& selected = track.keys[m_SelectedKey];
            const bool endpoint = m_SelectedKey == 0 || m_SelectedKey + 1 == track.keys.size();
            bool changed = false;
            ImGui::BeginDisabled(endpoint);
            if (!endpoint) changed |= EditUInt("Key Time (ms)", selected.timeMs, track.keys[m_SelectedKey + 1].timeMs - 1, track.keys[m_SelectedKey - 1].timeMs + 1);
            else ImGui::Text("Endpoint: %u ms", selected.timeMs);
            ImGui::EndDisabled();
            changed |= ImGui::DragFloat3("Position Offset (m)", &selected.positionOffset.x, .01f);
            auto degrees = QuaternionEuler(selected.rotationQuaternion);
            if (ImGui::DragFloat3("Rotation Offset (deg)", &degrees.x, .25f))
            {
                XMStoreFloat4(&selected.rotationQuaternion, XMQuaternionRotationRollPitchYaw(
                    XMConvertToRadians(degrees.x), XMConvertToRadians(degrees.y), XMConvertToRadians(degrees.z)));
                changed = true;
            }
            changed |= ImGui::DragFloat3("Scale Multiplier", &selected.scaleMultiplier.x, .01f, .001f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::Checkbox("Visible", &selected.visible);
            if (changed) Mark_Dirty();
            ImGui::BeginDisabled(endpoint);
            if (ImGui::Button("Delete Key")) { track.keys.erase(track.keys.begin() + m_SelectedKey); m_SelectedKey = 0; Mark_Dirty(); }
            ImGui::EndDisabled(); ImGui::SameLine();
            ImGui::BeginDisabled(track.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT || sequence.durationMs < 2);
            if (ImGui::Button("Add Key at Cursor"))
            {
                const auto* instance = m_Document.Find_Instance(m_SelectedInstance);
                const float local = instance ? (m_ClockMs - instance->startDelayMs) * instance->playbackSpeed : 0.f;
                const uint32_t time = static_cast<uint32_t>((std::clamp)(local, 1.f, static_cast<float>(sequence.durationMs - 1)));
                auto at = std::lower_bound(track.keys.begin(), track.keys.end(), time, [](const auto& value, uint32_t clock) { return value.timeMs < clock; });
                m_SelectedKey = at - track.keys.begin();
                if (at == track.keys.end() || at->timeMs != time)
                {
                    auto added = CWorldSequencePlayer::Sample_Track(sequence, track, static_cast<float>(time)); added.timeMs = time;
                    track.keys.insert(at, added); Mark_Dirty();
                }
            }
            ImGui::EndDisabled();
        }
    }
    if (ImGui::CollapsingHeader("Animation Clips"))
    {
        for (size_t index = 0; index < sequence.animationTracks.size(); ++index)
        {
            auto& clip = sequence.animationTracks[index]; ImGui::PushID(static_cast<int>(index));
            ImGui::Text("Slot: %s", clip.slotId.c_str());
            bool changed = EditText("Clip Name", clip.clipName);
            uint32_t minimum = 0, maximum = sequence.durationMs - 1;
            bool first = true;
            for (size_t other = 0; other < sequence.animationTracks.size(); ++other)
            {
                const auto& next = sequence.animationTracks[other];
                if (other == index || next.slotId != clip.slotId) continue;
                if (next.startMs < clip.startMs) { minimum = (std::max)(minimum, next.startMs + 1); first = false; }
                if (next.startMs > clip.startMs) maximum = (std::min)(maximum, next.startMs - 1);
            }
            ImGui::BeginDisabled(first); changed |= EditUInt("Clip Start (ms)", clip.startMs, maximum, minimum); ImGui::EndDisabled();
            changed |= ImGui::DragFloat("Clip Speed", &clip.playbackRate, .01f, .05f, 8.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::Checkbox("Clip Loop", &clip.loop); changed |= ImGui::Checkbox("Hold Last Pose", &clip.holdLastFrame);
            if (changed) Mark_Dirty();
            if (ImGui::SmallButton("Remove Clip"))
            {
                const auto slot = clip.slotId; sequence.animationTracks.erase(sequence.animationTracks.begin() + index);
                for (auto& remaining : sequence.animationTracks) if (remaining.slotId == slot) { remaining.startMs = 0; break; }
                Mark_Dirty(); ImGui::PopID(); break;
            }
            ImGui::PopID();
        }
        const auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
        ImGui::BeginDisabled(!resource || !resource->animated || !resource->sequenceInstanceId.empty() ||
            sequence.tracks.size() + sequence.animationTracks.size() >= CWorldSequenceDocument::MAX_TRACK_COUNT);
        if (ImGui::Button("Add Animation Clip"))
        {
            uint32_t start = 0;
            for (const auto& clip : sequence.animationTracks) if (clip.slotId == "object") start = (std::max)(start, clip.startMs + 1);
            if (start < sequence.durationMs)
            {
                WORLD_SEQUENCE_ANIMATION_TRACK clip; clip.slotId = "object"; clip.startMs = start; clip.clipName = "Assign actual model clip";
                sequence.animationTracks.push_back(clip); Mark_Dirty();
            }
        }
        ImGui::EndDisabled();
    }
}

void CWorldObjectTool::Rebuild_PhysicalTree()
{
    m_PhysicalTree = {};
    const auto search = Lower(m_PhysicalSearch.data());
    for (size_t index = 0; index < m_PhysicalAssets.size(); ++index)
    {
        const auto& asset = m_PhysicalAssets[index];
        if ((m_PhysicalSlot == 0) != (asset.kind == PHYSICAL_RESOURCE_KIND::MODEL)) continue;
        if (!search.empty() && Lower(asset.assetId).find(search) == std::string::npos) continue;
        std::vector<std::string> segments;
        for (const auto& segment : std::filesystem::path(asset.assetId).parent_path()) segments.push_back(segment.string());
        InsertResourceTree(m_PhysicalTree, segments, index);
    }
    FinalizeResourceTree(m_PhysicalTree);
}

void CWorldObjectTool::Render_PhysicalResources()
{
    ImGui::SeparatorText("Physical Resources");
    if (!m_PhysicalScanned)
    {
        Scan_PhysicalResources({"Effect", "Map", "Deploy", "Character"}, m_PhysicalAssets, m_PhysicalStatus);
        m_PhysicalScanned = true; Rebuild_PhysicalTree();
    }
    if (ImGui::Button("Refresh Files"))
    { Scan_PhysicalResources({"Effect", "Map", "Deploy", "Character"}, m_PhysicalAssets, m_PhysicalStatus); Rebuild_PhysicalTree(); }
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::Combo("##AssignSlot", &m_PhysicalSlot, "Model (.wmodel)\0Diffuse (.dds)\0")) Rebuild_PhysicalTree();
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::InputTextWithHint("##PhysicalSearch", "Search full relative path", m_PhysicalSearch.data(), m_PhysicalSearch.size())) Rebuild_PhysicalTree();
    ImGui::TextDisabled("%zu matching files", m_PhysicalTree.iRecursiveLeafCount);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", m_PhysicalStatus.c_str());
    if (ImGui::BeginChild("PhysicalResourceFolders", ImVec2(0, (std::max)(120.f, ImGui::GetContentRegionAvail().y)), true))
    {
        RenderResourceTree(m_PhysicalTree, [this](size_t index) {
            if (index >= m_PhysicalAssets.size()) return;
            const auto& asset = m_PhysicalAssets[index];
            ImGui::PushID(asset.assetId.c_str());
            auto* resource = m_Document.Find_ObjectResource(m_SelectedObject);
            ImGui::BeginDisabled(!resource || !resource->sequenceInstanceId.empty());
            if (ImGui::Selectable(asset.fileName.c_str(), asset.assetId == m_SelectedPhysical))
            {
                m_SelectedPhysical = asset.assetId;
                if (asset.kind == PHYSICAL_RESOURCE_KIND::MODEL) resource->modelAssetId = asset.assetId;
                else resource->diffuseTextureAssetId = asset.assetId;
                Mark_Dirty();
                m_Status = "Assigned " + asset.assetId + ". Save stores the slot on " + resource->displayName + ".";
            }
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", asset.assetId.c_str());
            ImGui::PopID();
        });
    }
    ImGui::EndChild();
}
#endif
