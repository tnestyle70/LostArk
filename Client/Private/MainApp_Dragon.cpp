#include <WinSock2.h>
#include "imgui.h"
#include "MainApp.h"
#include "Camera_Free.h"
#include "GameInstance.h"
#include "CombatHUDViewModel.h"
#include "PlayerController.h"
#include "ProjectDataRoot.h"
#include "DataJson.h"
#include "Level_ValtanArena.h"
#include "Level_KakulSaydonArena.h"
#include "Level_CharacterSelect.h"
#include <fstream>
#include <sstream>
#include <locale>
#include <cmath>

namespace
{
    std::shared_ptr<Client::CCamera_Free> CurrentCamera()
    {
        auto& game = Engine::CGameInstance::Get();
        if (game.Get_CurrentLevelID() == ETOUI(Client::LEVEL::CHARACTER_SELECT))
        {
            const auto* level = Client::CLevel_CharacterSelect::Get_Active();
            return level ? level->Get_DebugCamera() : nullptr;
        }
        return std::dynamic_pointer_cast<Client::CCamera_Free>(
            game.Get_GameObject(game.Get_CurrentLevelID(), L"Layer_Camera", 0));
    }
#ifdef _DEBUG
    struct DragonSpeedDraft
    {
        float speeds[3]{10.f, 16.f, 8.f};
        float baseline[3]{10.f, 16.f, 8.f};
        bool loaded = false;
        HANDLE process = nullptr;
        std::string status;
        ~DragonSpeedDraft() { if (process) CloseHandle(process); }
        void Load()
        {
            const auto path = Client::CProjectDataRoot::Resolve(L"Vehicles/VehicleProfiles.json");
            std::ifstream input(path, std::ios::binary);
            const std::string text((std::istreambuf_iterator<char>(input)), {});
            Client::DATA_JSON_VALUE document;
            std::string error;
            if (!input || !Client::CDataJson::Parse(text, document, error))
            { status = "Cannot read VehicleProfiles.json: " + error; return; }
            const auto* vehicles = document.Find("vehicles");
            if (vehicles && vehicles->Is_Array()) for (const auto& vehicle : vehicles->Get_Array())
            {
                const auto* id = vehicle.Find("vehicleId");
                if (!id || !id->Is_Number() || id->Get_Number() != 9523) continue;
                const auto* ground = vehicle.Find("moveSpeedOverride");
                if (!ground) ground = vehicle.Find("moveSpeed");
                const auto* flight = vehicle.Find("flight");
                const auto* speed = flight ? flight->Find("speed") : nullptr;
                const auto* vertical = flight ? flight->Find("verticalSpeed") : nullptr;
                if (!ground || !speed || !vertical || !ground->Is_Number() || !speed->Is_Number() || !vertical->Is_Number()) break;
                const float candidate[]{static_cast<float>(ground->Get_Number()), static_cast<float>(speed->Get_Number()),
                    static_cast<float>(vertical->Get_Number())};
                if (!std::isfinite(candidate[0]) || !std::isfinite(candidate[1]) || !std::isfinite(candidate[2]) ||
                    candidate[0] <= 0.f || candidate[0] > 30.f || candidate[1] <= 0.f || candidate[1] > 60.f ||
                    candidate[2] <= 0.f || candidate[2] > 60.f) break;
                for (int i = 0; i < 3; ++i) baseline[i] = speeds[i] = candidate[i];
                loaded = true; status = "Loaded authored speeds. Server values change after Publish and restart.";
                return;
            }
            status = "Ancient Sea speed profile is invalid; the previous draft is preserved.";
        }
        void Start(const bool publish)
        {
            if (process || !loaded) return;
            const auto root = Client::CProjectDataRoot::Resolve(L"Vehicles/VehicleProfiles.json").parent_path().parent_path().parent_path();
            const auto script = root / L"Tools/GameplayPipeline/Publish-VehicleProfiles.ps1";
            if (!std::filesystem::is_regular_file(script)) { status = "Vehicle publisher is unavailable in this installation."; return; }
            std::wostringstream command;
            command.imbue(std::locale::classic()); command.precision(9);
            command << L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" << script.wstring() << L"\" -Mode "
                << (publish ? L"SaveAndPublish" : L"Save")
                << L" -AncientSeaGroundSpeed " << speeds[0] << L" -AncientSeaFlightSpeed " << speeds[1]
                << L" -AncientSeaVerticalSpeed " << speeds[2] << L" -ExpectedGroundSpeed " << baseline[0]
                << L" -ExpectedFlightSpeed " << baseline[1] << L" -ExpectedVerticalSpeed " << baseline[2];
            auto args = command.str();
            STARTUPINFOW startup{}; startup.cb = sizeof(startup);
            PROCESS_INFORMATION info{};
            if (!CreateProcessW(nullptr, args.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                nullptr, root.c_str(), &startup, &info))
            { status = "Could not start the vehicle publisher; no changes were made."; return; }
            CloseHandle(info.hThread); process = info.hProcess;
            status = publish ? "Saving and publishing dragon speeds..." : "Saving dragon speeds...";
        }
        void Poll()
        {
            if (!process || WaitForSingleObject(process, 0) != WAIT_OBJECT_0) return;
            DWORD code = 1; GetExitCodeProcess(process, &code); CloseHandle(process); process = nullptr;
            if (code == 0) { Load(); status = "Speed operation succeeded. Publish and restart Server before testing movement."; }
            else status = "Speed operation failed (validation or concurrent edit). Disk changes were preserved; reload the draft.";
        }
    };
#endif
}

void CMainApp::RenderCameraSpeedControls()
{
    const auto camera = CurrentCamera();
    const auto level = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
    const bool characterSelect = level == LEVEL::CHARACTER_SELECT;
    if (!camera || !ImGui::CollapsingHeader(characterSelect ? "Character Select Camera" : "Camera",
        characterSelect ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None)) return;
    const auto setSpeed = [&](float value) {
#ifdef _DEBUG
        if (level == LEVEL::VALTAN_ARENA && CLevel_ValtanArena::Get_Active())
        { CLevel_ValtanArena::Get_Active()->Set_DebugCameraSpeed(value); return; }
        if (level == LEVEL::KAKULSAYDON_ARENA && CLevel_KakulSaydonArena::Get_Active())
        { CLevel_KakulSaydonArena::Get_Active()->Set_DebugCameraSpeed(value); return; }
#endif
        camera->Set_FreeMoveSpeed(value);
    };
    float speed = camera->Get_FreeMoveSpeed();
    if (ImGui::DragFloat("Free camera speed (m/s)", &speed, .5f,
        CCamera_Free::MIN_FREE_MOVE_SPEED, CCamera_Free::MAX_FREE_MOVE_SPEED, "%.1f", ImGuiSliderFlags_AlwaysClamp))
        setSpeed(speed);
    if (ImGui::Button("Reset speed to 20 m/s")) setSpeed(20.f);
    ImGui::TextDisabled("F6: Follow / Free. Shift: x%.0f.", CCamera_Free::FREE_MOVE_SPRINT_MULTIPLIER);
    if (characterSelect)
        ImGui::TextDisabled("Applies to F6 / Movie free-camera movement; Movie playback timing stays authored.");
#ifdef _DEBUG
    if (level == LEVEL::VALTAN_ARENA || level == LEVEL::KAKULSAYDON_ARENA)
        ImGui::TextDisabled("Arena speed is retained for this Client session.");
    else
#endif
        ImGui::TextDisabled("Speed applies during this map visit.");
}

void CMainApp::RenderDragonControls()
{
#ifdef _DEBUG
    static DragonSpeedDraft draft;
    draft.Poll();
    if (!ImGui::CollapsingHeader("Dragon")) return;
    if (!draft.loaded && draft.status.empty()) draft.Load();
    const auto& player = CCombatHUDViewModel::Get().Get_Player();
    auto* controller = Find_ActivePlayerController();
    const auto camera = CurrentCamera();
    const bool cameraAcceptsGameplay = camera && camera->Is_FollowEnabled() &&
        !camera->Is_PresentationOverrideActive();
    const bool canRequestRiding = controller && player.isValid && !player.isPreview && cameraAcceptsGameplay;
    ImGui::Text("Ancient Sea (9523) | flight phase: %u", static_cast<unsigned>(player.eVehicleFlightPhase));
    ImGui::BeginDisabled(!canRequestRiding);
    if (ImGui::Button(player.iVehicleId == 9523u ? "Dismount dragon" : "Mount Ancient Sea") && canRequestRiding)
        controller->Request_VehicleRiding(player.iVehicleId == 9523u ? 0u : 9523u);
    ImGui::EndDisabled();
    if (!cameraAcceptsGameplay) ImGui::TextDisabled("Mount / dismount requires the follow camera (F6).");
    ImGui::TextWrapped("E: takeoff / land. WASD: fly, Space / Ctrl: ascend / descend. Landing requires walkable ground below; ground mount keeps the character view.");
    if (camera)
    {
        float distance, pitch, height; bool enabled, heading;
        camera->Get_DragonCameraSettings(distance, pitch, height, enabled, heading);
        bool changed = ImGui::Checkbox("Flight camera", &enabled);
        changed |= ImGui::Checkbox("Follow dragon heading", &heading);
        changed |= ImGui::DragFloat("Flight distance (m)", &distance, .1f, 4.f, 80.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat("Flight pitch (degrees)", &pitch, .2f, -8.f, 75.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat("Flight look height (m)", &height, .05f, 0.f, 15.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::Button("Reset flight camera")) { distance = 16.f; pitch = 24.f; height = 2.4f; enabled = heading = changed = true; }
        if (changed) camera->Set_DragonCameraSettings(distance, pitch, height, enabled, heading);
        ImGui::TextDisabled("Camera edits apply immediately during flight; left drag orbits. F6 keeps its normal meaning.");
    }
    ImGui::SeparatorText("Server speed authoring");
    ImGui::BeginDisabled(draft.process != nullptr || !draft.loaded);
    ImGui::DragFloat("Ground speed (m/s)", &draft.speeds[0], .1f, .1f, 30.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("Flight speed (m/s)", &draft.speeds[1], .1f, .1f, 60.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("Vertical speed (m/s)", &draft.speeds[2], .1f, .1f, 60.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::Button("Save dragon speeds")) draft.Start(false);
    ImGui::SameLine(); if (ImGui::Button("Save + Publish dragon speeds")) draft.Start(true);
    ImGui::EndDisabled();
    ImGui::BeginDisabled(draft.process != nullptr);
    if (ImGui::Button("Reload dragon speeds")) draft.Load();
    ImGui::EndDisabled();
    ImGui::TextWrapped("%s", draft.status.c_str());
#endif
}
