// Windows 네트워크 선언을 먼저 포함해 이후 Windows 헤더와의 선언 충돌을 피한다.
#include <WinSock2.h>
// 사용자가 누르는 카메라·용 저작 controls를 그린다.
#include "imgui.h"
// Developer Tools에서 호출하는 두 패널의 MainApp 멤버를 구현한다.
#include "MainApp.h"
// 현재 카메라의 자유 이동 속도와 용 비행 구도를 편집한다.
#include "Camera_Free.h"
// 현재 Level과 카메라 layer를 조회한다.
#include "GameInstance.h"
// 복제된 로컬 플레이어의 탈것·비행 상태를 읽는다.
#include "CombatHUDViewModel.h"
// 승하차 요청을 기존 Server 명령 경계로 전달한다.
#include "PlayerController.h"
// VehicleProfiles 저작 정본과 publisher의 저장소 위치를 찾는다.
#include "ProjectDataRoot.h"
// 저작 JSON에서 Ancient Sea 속도 입력을 검증한다.
#include "DataJson.h"
// 발탄 자유 카메라 속도의 세션 보관 setter를 사용한다.
#include "Level_ValtanArena.h"
// 쿠크 자유 카메라 속도의 세션 보관 setter를 사용한다.
#include "Level_KoukuSaydonArena.h"
// 캐릭터 선택의 전용 Debug 카메라를 조회한다.
#include "Level_CharacterSelect.h"
// VehicleProfiles 원본을 바이너리 스트림으로 읽는다.
#include <fstream>
// publisher에 넘길 명령행 문자열을 구성한다.
#include <sstream>
// 속도 숫자를 사용자 로캘과 무관한 소수점 형식으로 전달한다.
#include <locale>
// 저작 속도 후보의 NaN·무한대를 거부한다.
#include <cmath>

namespace
{
// 패널이 조작할 현재 카메라를 조회한다. Character Select는 Level의 전용 카메라, 나머지는 현재 camera layer를 사용한다.
// shared_ptr로 호출 중 수명을 유지하며 카메라를 찾지 못하면 null이다.
std::shared_ptr<Client::CCamera_Free> CurrentCamera()
{
	auto &game = Engine::CGameInstance::Get();
	if (game.Get_CurrentLevelID() == ETOUI(Client::LEVEL::CHARACTER_SELECT))
	{
		const auto *level = Client::CLevel_CharacterSelect::Get_Active();
		return level ? level->Get_DebugCamera() : nullptr;
	}
	return std::dynamic_pointer_cast<Client::CCamera_Free>(
		game.Get_GameObject(game.Get_CurrentLevelID(), L"Layer_Camera", 0));
}
#ifdef _DEBUG
// Debug 패널의 세 속도는 지상·수평 비행·수직 비행 순서의 m/s다.
// baseline은 마지막 성공 로드 값으로 publisher의 동시 편집 검사에 쓰며, process HANDLE은 이 draft가 소유한다.
struct DragonSpeedDraft
{
	float speeds[3]{10.f, 16.f, 8.f};
	float baseline[3]{10.f, 16.f, 8.f};
	bool loaded = false;
	HANDLE process = nullptr;
	std::string status;
	// 패널 상태가 소멸하면 보관한 프로세스 HANDLE만 닫는다. 이미 실행 중인 publisher를 강제 종료하지 않는다.
	~DragonSpeedDraft()
	{
		if (process)
			CloseHandle(process);
	}
	// 패널 최초 열기·Reload·저장 성공 후 호출한다. 직업 외형과 무관한 Ancient Sea 속도만 검증한다.
	// 후보 세 값이 모두 유효할 때 baseline과 편집값을 함께 교체하며 실패하면 기존 draft를 유지한다.
	void Load()
	{
		const auto path = Client::CProjectDataRoot::Resolve(L"Vehicles/VehicleProfiles.json");
		std::ifstream input(path, std::ios::binary);
		const std::string text((std::istreambuf_iterator<char>(input)), {});
		Client::DATA_JSON_VALUE document;
		std::string error;
		if (!input || !Client::CDataJson::Parse(text, document, error))
		{
			status = "Cannot read VehicleProfiles.json: " + error;
			return;
		}
		// vehicleId 9523 행에서 지상 속도 override를 우선하고 flight의 수평·수직 속도를 함께 읽는다.
		const auto *vehicles = document.Find("vehicles");
		if (vehicles && vehicles->Is_Array())
			for (const auto &vehicle : vehicles->Get_Array())
			{
				const auto *id = vehicle.Find("vehicleId");
				if (!id || !id->Is_Number() || id->Get_Number() != 9523)
					continue;
				const auto *ground = vehicle.Find("moveSpeedOverride");
				if (!ground)
					ground = vehicle.Find("moveSpeed");
				const auto *flight = vehicle.Find("flight");
				const auto *speed = flight ? flight->Find("speed") : nullptr;
				const auto *vertical = flight ? flight->Find("verticalSpeed") : nullptr;
				if (!ground || !speed || !vertical || !ground->Is_Number() || !speed->Is_Number() ||
					!vertical->Is_Number())
					break;
				const float candidate[]{static_cast<float>(ground->Get_Number()),
										static_cast<float>(speed->Get_Number()),
										static_cast<float>(vertical->Get_Number())};
				if (!std::isfinite(candidate[0]) || !std::isfinite(candidate[1]) || !std::isfinite(candidate[2]) ||
					candidate[0] <= 0.f || candidate[0] > 30.f || candidate[1] <= 0.f || candidate[1] > 60.f ||
					candidate[2] <= 0.f || candidate[2] > 60.f)
					break;
				for (int i = 0; i < 3; ++i)
					baseline[i] = speeds[i] = candidate[i];
				loaded = true;
				status = "Loaded authored speeds. Server values change after Publish and restart.";
				return;
			}
		status = "Ancient Sea speed profile is invalid; the previous draft is preserved.";
	}
	// 사용자가 Save 또는 Save + Publish를 눌렀을 때 publisher를 창 없이 시작한다.
	// 새 값과 마지막 로드 baseline을 함께 넘겨 실제 검증·병합·저장·게시 책임을 기존 스크립트에 둔다.
	void Start(const bool publish)
	{
		if (process || !loaded)
			return;
		const auto root = Client::CProjectDataRoot::Resolve(L"Vehicles/VehicleProfiles.json")
							  .parent_path()
							  .parent_path()
							  .parent_path();
		const auto script = root / L"Tools/GameplayPipeline/Publish-VehicleProfiles.ps1";
		if (!std::filesystem::is_regular_file(script))
		{
			status = "Vehicle publisher is unavailable in this installation.";
			return;
		}
		// 소수점과 정밀도를 고정해 다른 로캘에서도 같은 속도 값과 expected 값을 비교하도록 한다.
		std::wostringstream command;
		command.imbue(std::locale::classic());
		command.precision(9);
		command << L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" << script.wstring() << L"\" -Mode "
				<< (publish ? L"SaveAndPublish" : L"Save") << L" -AncientSeaGroundSpeed " << speeds[0]
				<< L" -AncientSeaFlightSpeed " << speeds[1] << L" -AncientSeaVerticalSpeed " << speeds[2]
				<< L" -ExpectedGroundSpeed " << baseline[0] << L" -ExpectedFlightSpeed " << baseline[1]
				<< L" -ExpectedVerticalSpeed " << baseline[2];
		auto args = command.str();
		STARTUPINFOW startup{};
		startup.cb = sizeof(startup);
		PROCESS_INFORMATION info{};
		if (!CreateProcessW(nullptr, args.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, root.c_str(),
							&startup, &info))
		{
			status = "Could not start the vehicle publisher; no changes were made.";
			return;
		}
		CloseHandle(info.hThread);
		process = info.hProcess;
		status = publish ? "Saving and publishing dragon speeds..." : "Saving dragon speeds...";
	}
	// 매 패널 프레임에 종료 여부만 비차단 조회한다. 성공하면 디스크를 다시 읽고, 실패하면 기존 편집값을 유지한다.
	void Poll()
	{
		if (!process || WaitForSingleObject(process, 0) != WAIT_OBJECT_0)
			return;
		DWORD code = 1;
		GetExitCodeProcess(process, &code);
		CloseHandle(process);
		process = nullptr;
		if (code == 0)
		{
			Load();
			status = "Speed operation succeeded. Publish and restart Server before testing movement.";
		}
		else
			status =
				"Speed operation failed (validation or concurrent edit). Disk changes were preserved; reload the draft.";
	}
};
#endif
} // namespace

// Developer Tools가 현재 카메라의 자유 이동 속도를 m/s로 편집한다.
// 아레나는 Level setter로 세션 값을 유지하고 다른 맵은 현재 카메라 인스턴스만 바꾼다.
void CMainApp::RenderCameraSpeedControls()
{
	const auto camera = CurrentCamera();
	const auto level = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
	const bool characterSelect = level == LEVEL::CHARACTER_SELECT;
	if (!camera || !ImGui::CollapsingHeader(characterSelect ? "Character Select Camera" : "Camera",
											characterSelect ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None))
		return;
	const auto setSpeed = [&](float value)
	{
#ifdef _DEBUG
		if (level == LEVEL::VALTAN_ARENA && CLevel_ValtanArena::Get_Active())
		{
			CLevel_ValtanArena::Get_Active()->Set_DebugCameraSpeed(value);
			return;
		}
		if (level == LEVEL::KOUKUSAYDON_ARENA && CLevel_KoukuSaydonArena::Get_Active())
		{
			CLevel_KoukuSaydonArena::Get_Active()->Set_DebugCameraSpeed(value);
			return;
		}
#endif
		camera->Set_FreeMoveSpeed(value);
	};
	float speed = camera->Get_FreeMoveSpeed();
	if (ImGui::DragFloat("Free camera speed (m/s)", &speed, .5f, CCamera_Free::MIN_FREE_MOVE_SPEED,
						 CCamera_Free::MAX_FREE_MOVE_SPEED, "%.1f", ImGuiSliderFlags_AlwaysClamp))
		setSpeed(speed);
	if (ImGui::Button("Reset speed to 20 m/s"))
		setSpeed(20.f);
	ImGui::TextDisabled("F6: Follow / Free. Shift: x%.0f.", CCamera_Free::FREE_MOVE_SPRINT_MULTIPLIER);
	if (characterSelect)
		ImGui::TextDisabled("Applies to F6 / Movie free-camera movement; Movie playback timing stays authored.");
#ifdef _DEBUG
	if (level == LEVEL::VALTAN_ARENA || level == LEVEL::KOUKUSAYDON_ARENA)
		ImGui::TextDisabled("Arena speed is retained for this Client session.");
	else
#endif
		ImGui::TextDisabled("Speed applies during this map visit.");
}

// Debug Developer Tools의 승하차·비행 카메라·Server 속도 저작 패널이다.
// 승하차는 PlayerController의 typed 요청으로 보내며 카메라 preview와 파일 저장·Server 재시작 적용을 구분한다.
void CMainApp::RenderDragonControls()
{
#ifdef _DEBUG
	static DragonSpeedDraft draft;
	draft.Poll();
	if (!ImGui::CollapsingHeader("Dragon"))
		return;
	if (!draft.loaded && draft.status.empty())
		draft.Load();
	const auto &player = CCombatHUDViewModel::Get().Get_Player();
	auto *controller = Find_ActivePlayerController();
	const auto camera = CurrentCamera();
	// 추적 카메라이고 연출 override가 없을 때만 실제 승하차 명령을 허용한다.
	const bool cameraAcceptsGameplay = camera && camera->Is_FollowEnabled() && !camera->Is_PresentationOverrideActive();
	const bool canRequestRiding = controller && player.isValid && !player.isPreview && cameraAcceptsGameplay;
	ImGui::Text("Ancient Sea (9523) | flight phase: %u", static_cast<unsigned>(player.eVehicleFlightPhase));
	ImGui::BeginDisabled(!canRequestRiding);
	if (ImGui::Button(player.iVehicleId == 9523u ? "Dismount dragon" : "Mount Ancient Sea") && canRequestRiding)
		controller->Request_VehicleRiding(player.iVehicleId == 9523u ? 0u : 9523u);
	ImGui::EndDisabled();
	if (!cameraAcceptsGameplay)
		ImGui::TextDisabled("Mount / dismount requires the follow camera (F6).");
	ImGui::TextWrapped(
		"E: takeoff / land. WASD: fly, Space / Ctrl: ascend / descend. Landing requires walkable ground below; ground mount keeps the character view.");
	// 아래 구도 조절은 현재 Client 카메라만 바꾸며 Server 이동 속도나 저작 profile을 저장하지 않는다.
	if (camera)
	{
		float distance, pitch, height;
		bool enabled, heading;
		camera->Get_DragonCameraSettings(distance, pitch, height, enabled, heading);
		bool changed = ImGui::Checkbox("Flight camera", &enabled);
		changed |= ImGui::Checkbox("Follow dragon heading", &heading);
		changed |=
			ImGui::DragFloat("Flight distance (m)", &distance, .1f, 4.f, 80.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		changed |=
			ImGui::DragFloat("Flight pitch (degrees)", &pitch, .2f, -8.f, 75.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
		changed |=
			ImGui::DragFloat("Flight look height (m)", &height, .05f, 0.f, 15.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::Button("Reset flight camera"))
		{
			distance = 16.f;
			pitch = 24.f;
			height = 2.4f;
			enabled = heading = changed = true;
		}
		if (changed)
			camera->Set_DragonCameraSettings(distance, pitch, height, enabled, heading);
		ImGui::TextDisabled(
			"Camera edits apply immediately during flight; left drag orbits. F6 keeps its normal meaning.");
	}
	// 속도 draft는 사용자 Save 동작으로만 publisher에 전달한다. 진행 중에는 중복 저장·재로드를 제한한다.
	ImGui::SeparatorText("Server speed authoring");
	ImGui::BeginDisabled(draft.process != nullptr || !draft.loaded);
	ImGui::DragFloat("Ground speed (m/s)", &draft.speeds[0], .1f, .1f, 30.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Flight speed (m/s)", &draft.speeds[1], .1f, .1f, 60.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Vertical speed (m/s)", &draft.speeds[2], .1f, .1f, 60.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	if (ImGui::Button("Save dragon speeds"))
		draft.Start(false);
	ImGui::SameLine();
	if (ImGui::Button("Save + Publish dragon speeds"))
		draft.Start(true);
	ImGui::EndDisabled();
	ImGui::BeginDisabled(draft.process != nullptr);
	if (ImGui::Button("Reload dragon speeds"))
		draft.Load();
	ImGui::EndDisabled();
	ImGui::TextWrapped("%s", draft.status.c_str());
#endif
}
