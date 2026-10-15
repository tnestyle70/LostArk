// Debug 도구 창·입력 점유와 월드 선택 표시를 처리한다.
#include "imgui.h"
// MainApp이 소유한 저작 도구 간 선택·입력 중재를 구현한다.
#include "MainApp.h"

#ifdef _DEBUG
// 선택한 월드 위치로 현재 저작 카메라의 구도를 맞춘다.
#include "Camera_Free.h"
// 현재 Level·카메라 행렬·화면 위치 picking을 조회한다.
#include "GameInstance.h"
// Guide trigger 박스의 일회 위치 선택을 완료하거나 취소한다.
#include "GuideAITool.h"
// Action·Sequence 저작 draft의 선택과 Map Effect 배치 요청을 연결한다.
#include "KoukuSaydonActionWorkbench.h"
// 현재 Level의 canonical Area와 렌더링 Level 연결을 조회한다.
#include "LevelRegistry.h"
// 베른의 현재 저작 카메라와 플레이어 placement 제어를 조회한다.
#include "Level_Bern.h"
// 캐릭터 선택의 현재 저작 카메라와 placement 제어를 조회한다.
#include "Level_CharacterSelect.h"
// 쿠크 아레나의 저작 카메라와 플레이어 placement 제어를 조회한다.
#include "Level_KoukuSaydonArena.h"
// 발탄 아레나의 저작 카메라와 플레이어 placement 제어를 조회한다.
#include "Level_ValtanArena.h"
// 별도 Development Map Editor와 현재 Level 저작을 구분한다.
#include "MapEditorWorkspaceService.h"
// 기존 맵 편집 소유권·미저장 상태·Area 준비와 선택 요청을 사용한다.
#include "MapTool.h"
// 월드 ray를 만들고 기존 Move Player 일회 입력을 취소한다.
#include "PlayerController.h"
// composition shell에서 대상 보스·관문과 Action/Sequence 창을 선택한다.
#include "SequencerTool.h"
// 한 클릭이 제품 UI·저작 도구·gameplay에 중복 전달되지 않게 점유한다.
#include "UIInputRouter.h"
// 통합 목록이 발행한 typed 요청과 chunk 조회 결과를 처리한다.
#include "WorldLevelTool.h"
// 현재 Level의 실제 배치·Deploy 선택과 공용 Object Details를 연결한다.
#include "WorldSceneTool.h"
// Level이 소유한 live map catalog·placement·Deploy runtime을 참조한다.
#include "MapAuthoringHost.h"
// 쿠크 world object의 기존 motion 저작 화면을 연다.
#include "WorldObjectTool.h"

// 선택 identity 검색과 카메라 focus 반경 제한에 사용한다.
#include <algorithm>
// picking·clip 좌표의 유한성과 화면 범위를 검사한다.
#include <cmath>
// 한 종류가 맞지 않았을 때 거리 비교에 FLT_MAX를 사용한다.
#include <cfloat>
// 기존 도구 구현의 표준 스트림 컴파일 의존성을 유지한다.
#include <sstream>

using namespace Client;

// Map Tool이 저작 소유권을 받기 전에 World Scene의 placement 편집 해제를 요청한다.
// 해제 실패는 기존 draft를 보존하고 해당 도구에 이유를 보여 주며 성공한 경우에만 창을 숨긴다.
bool CMainApp::Debug_PrepareMapToolAuthoring(std::string &status)
{
	if (!m_pWorldSceneTool)
		return true;
	if (!m_pWorldSceneTool->Release_PlacementEditing(status))
	{
		(void)EnsureDebugTool(DEBUG_TOOL::WORLD_SCENE);
		m_pWorldSceneTool->Set_Status(status);
		if (m_pWorldLevelTool)
			m_pWorldLevelTool->Set_Status(status);
		return false;
	}
	SetDebugToolVisible(DEBUG_TOOL::WORLD_SCENE, false);
	return true;
}

// Object Details가 같은 Area를 편집하기 전에 기존 Map Tool의 미저장 변경·preview 복원 여부를 확인한다.
// 저장되지 않은 변경이나 복원 실패가 있으면 새 편집을 시작하지 않는다.
bool CMainApp::Debug_PrepareWorldPlacementEditing(const std::string &areaId, std::string &status)
{
	if (!m_pMapTool)
		return true;
	if (m_pMapTool->Debug_GetActiveAreaId() == areaId && m_pMapTool->Debug_HasPendingAuthoring())
	{
		status = "Save the pending Map Tool edits before enabling Object Details placement editing.";
		return false;
	}
	if (m_pMapTool->IsOpen())
	{
		m_pMapTool->SetOpen(false);
		if (m_pMapTool->IsOpen())
		{
			status = "Stop / Restore the Map Tool preview before enabling Object Details editing.";
			return false;
		}
		SetDebugToolVisible(DEBUG_TOOL::MAP, false);
	}
	return true;
}

// 현재 live Area의 stable placement를 공용 Object Details로 넘긴다.
// Map Tool draft를 검사하고 preview를 닫은 뒤 선택하며, 초기화·선택 실패는 이전 창 표시 상태를 복원한다.
bool CMainApp::Debug_OpenWorldObjectDetails(const std::string &areaId, uint64_t placementId, bool deploy,
											std::string &status)
{
	const auto *host = Find_ActiveMapAuthoringHost();
	if (!host || areaId != host->Get_MapAuthoringCatalog().Get_AreaId())
	{
		status =
			"Object Details requires this Area's live Bern, Character Select, Valtan or Kouku map. The existing Map Tool inspector remains available for isolated or Movie backgrounds.";
		return false;
	}
	if (m_pMapTool && m_pMapTool->Debug_GetActiveAreaId() == areaId && m_pMapTool->Debug_HasPendingAuthoring())
	{
		status = "Save the pending Map Tool edits before transferring this placement to Object Details.";
		return false;
	}
	const bool mapWasOpen = m_pMapTool && m_pMapTool->IsOpen();
	if (mapWasOpen)
	{
		m_pMapTool->SetOpen(false);
		if (m_pMapTool->IsOpen())
		{
			status = "Map Tool could not restore its preview. Stop / Restore there before opening Object Details.";
			return false;
		}
	}
	const bool sceneWasVisible = IsDebugToolVisible(DEBUG_TOOL::WORLD_SCENE);
	// 실패 복구는 창 표시와 원래 열림 상태를 되돌린다. 실제 placement draft는 기존 도구의 소유권을 유지한다.
	const auto restoreWindows = [&]
	{
		if (mapWasOpen)
		{
			m_pMapTool->SetOpen(true);
			SetDebugToolVisible(DEBUG_TOOL::MAP, m_pMapTool->IsOpen());
		}
		if (sceneWasVisible && m_pWorldSceneTool)
		{
			m_pWorldSceneTool->Open();
			SetDebugToolVisible(DEBUG_TOOL::WORLD_SCENE, true);
		}
		if (!sceneWasVisible)
			SetDebugToolVisible(DEBUG_TOOL::WORLD_SCENE, false);
	};
	if (FAILED(EnsureDebugTool(DEBUG_TOOL::WORLD_SCENE)))
	{
		restoreWindows();
		status = "Object Details could not initialize.";
		return false;
	}
	m_pWorldSceneTool->Update(0.f, true);
	if (placementId && !m_pWorldSceneTool->Inspect_Placement(placementId, deploy))
	{
		restoreWindows();
		status = "This placement is not in the current live scope; previous selection was preserved.";
		return false;
	}
	m_pWorldSceneTool->Set_DetailsMode(true);
	if (mapWasOpen)
		SetDebugToolVisible(DEBUG_TOOL::MAP, false);
	if (m_pWorldLevelTool)
		m_pWorldLevelTool->Hide_MetadataDetails();
	status = "Object Details selected the live placement. Transform edits use its existing save owner.";
	return true;
}

// MainApp의 입력 단계가 한 번 예약한 live mesh 선택을 처리한다.
// 도구·Level·포커스가 바뀌면 예약을 취소하고 클릭을 소비한다. 선택 성공은 실제 배치 ID만 바꾸며 TRS나 Server 상태는 변경하지 않는다.
bool CMainApp::UpdateWorldMeshInspectionInput()
{
	const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	const bool rightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	if (!leftDown && !rightDown)
		m_bWorldMeshPickSuppressMouse = false;
	const auto currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (!m_bWorldMeshPickArmed)
		return m_bWorldMeshPickSuppressMouse;
	if (auto *controller = Find_ActivePlayerController(); controller && controller->Is_DebugPlayerPlacementArmed())
	{
		m_bWorldMeshPickArmed = false;
		if (m_pWorldSceneTool)
			m_pWorldSceneTool->Set_Status("Move Player owns the next world click; mesh selection was preserved.");
		return m_bWorldMeshPickSuppressMouse;
	}
	auto *host = Find_ActiveMapAuthoringHost();
	const HWND foreground = GetForegroundWindow();
	DWORD process = 0u;
	if (foreground)
		GetWindowThreadProcessId(foreground, &process);
	if (!m_pWorldSceneTool || !m_bDeveloperToolsVisible || !IsDebugToolVisible(DEBUG_TOOL::WORLD_SCENE) ||
		!m_pWorldSceneTool->Is_Open() || m_eDebugInputOwner != DEBUG_TOOL::WORLD_SCENE || !host ||
		currentLevel != m_iWorldMeshPickLevel || process != GetCurrentProcessId() || rightDown ||
		(GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
	{
		m_bWorldMeshPickArmed = false;
		m_bWorldMeshPickSuppressMouse = leftDown || rightDown;
		if (m_pWorldSceneTool)
			m_pWorldSceneTool->Set_Status("World pick cancelled; previous selection preserved.");
		return true;
	}
	const bool pressed = leftDown && !m_bWorldMeshPickLeftDown;
	m_bWorldMeshPickLeftDown = leftDown;
	if (!pressed || foreground != g_hWnd || ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput ||
		CUIInputRouter::Get().Is_MouseClaimedThisFrame() || CUIInputRouter::Get().Was_MouseClaimedLastFrame())
		return true;
	CUIInputRouter::Get().Claim_Mouse_This_Frame();
	m_bWorldMeshPickSuppressMouse = true;
	vector_t origin, direction;
	float3_t rayOrigin{}, rayDirection{};
	if (CPlayerController::Try_PickWorldRay(origin, direction))
	{
		XMStoreFloat3(&rayOrigin, origin);
		XMStoreFloat3(&rayDirection, direction);
		// ray에 맞아도 카메라 앞의 0 ≤ clip.z ≤ clip.w 범위에 들어온 위치만 선택한다.
		const auto inCameraDepth = [](const float3_t &point)
		{
			const auto *view = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
			const auto *projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
			if (!view || !projection)
				return false;
			float4_t clip{};
			XMStoreFloat4(&clip, XMVector4Transform(XMVectorSet(point.x, point.y, point.z, 1.f),
													XMLoadFloat4x4(view) * XMLoadFloat4x4(projection)));
			return std::isfinite(clip.w) && std::isfinite(clip.z) && clip.w > .0001f && clip.z >= 0.f &&
				   clip.z <= clip.w;
		};
		MAP_WORLD_MESH_PICK mapPick;
		DEPLOY_WORLD_MESH_PICK deployPick;
		const bool mapHit =
			host->Get_MapAuthoringRuntime().Try_PickInspectionSurface(rayOrigin, rayDirection, mapPick) &&
			inCameraDepth(mapPick.hitPosition);
		auto *deploy = host->Get_MapAuthoringDeployRuntime();
		const bool deployHit = deploy && deploy->Try_PickInspectionSurface(rayOrigin, rayDirection, deployPick) &&
							   inCameraDepth(deployPick.hitPosition);
		if (mapHit || deployHit)
		{
			// 두 종류가 모두 맞으면 ray 시작점에서의 거리 제곱을 비교해 더 가까운 실제 표면을 고른다. 단위는 m²다.
			const float mapDistance =
				mapHit ? XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&mapPick.hitPosition) - origin)) : FLT_MAX;
			const float deployDistance =
				deployHit ? XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&deployPick.hitPosition) - origin)) : FLT_MAX;
			if (deployHit && (!mapHit || deployDistance < mapDistance))
				m_pWorldSceneTool->Complete_DeployPick(std::move(deployPick));
			else
				m_pWorldSceneTool->Complete_MapPick(std::move(mapPick));
			uint64_t selectedId = 0;
			bool selectedDeploy = false;
			if (m_pWorldLevelTool && m_pWorldLevelTool->Is_Open() &&
				m_pWorldSceneTool->Get_SelectedPlacement(selectedId, selectedDeploy))
				m_pWorldLevelTool->Sync_LiveSelection(m_pWorldSceneTool->Get_AreaId(), selectedId, selectedDeploy,
													  true);
			m_bWorldMeshPickArmed = false;
			return true;
		}
	}
	m_pWorldSceneTool->Set_Status("No live map or Deploy triangle hit. Click again, or Esc / right-click to cancel.");
	return true;
}

// 현재 Level에 저작 host가 있을 때 World Scene 진입 버튼을 표시한다. 버튼만 공용 도구 초기화를 요청한다.
void CMainApp::RenderWorldMeshInspection()
{
	if (!Find_ActiveMapAuthoringHost())
		return;
	ImGui::SeparatorText("World Scene");
	if (ImGui::Button("Open World Scene Tool"))
	{
		if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::WORLD_SCENE)))
			m_pWorldSceneTool->Set_DetailsMode(false);
	}
	ImGui::SameLine();
	ImGui::TextDisabled("Live mesh pick / transform / map animation");
}

// World Scene 창의 표시·선택·focus·일회 picking 요청을 중재한다.
// 새 picking은 다른 저작 소비자의 예약을 해제하고, 현재 선택점은 월드→clip→화면으로 투영해 표시한다.
void CMainApp::RenderWorldSceneTool()
{
	if (!IsDebugToolVisible(DEBUG_TOOL::WORLD_SCENE) || !m_pWorldSceneTool)
		return;
	if (m_eDebugWindowFocusPending == DEBUG_TOOL::WORLD_SCENE)
	{
		ImGui::SetNextWindowFocus();
		m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
	}
	m_pWorldSceneTool->Render();
	uint64_t selectedId = 0;
	bool selectedDeploy = false;
	if (m_pWorldLevelTool && m_pWorldLevelTool->Is_Open() &&
		m_pWorldSceneTool->Get_SelectedPlacement(selectedId, selectedDeploy))
		m_pWorldLevelTool->Sync_LiveSelection(m_pWorldSceneTool->Get_AreaId(), selectedId, selectedDeploy);
	if (m_pWorldSceneTool->Consume_InteractionRequest())
		m_eDebugInputOwner = DEBUG_TOOL::WORLD_SCENE;
	if (!m_pWorldSceneTool->Is_Open())
	{
		SetDebugToolVisible(DEBUG_TOOL::WORLD_SCENE, false);
		return;
	}
	float3_t focus{};
	float radius = 8.f;
	if (m_pWorldSceneTool->Consume_FocusRequest(focus, radius))
	{
		std::string status;
		(void)FocusWorldLevelPosition(focus, radius, status);
		m_pWorldSceneTool->Set_Status(std::move(status));
	}
	if (m_pWorldSceneTool->Consume_PickRequest())
	{
		if (m_bWorldMeshPickArmed)
		{
			m_bWorldMeshPickArmed = false;
			m_pWorldSceneTool->Set_Status("World pick cancelled; previous selection preserved.");
		}
		else
		{
			m_bWorldLevelPickArmed = false;
			if (m_pGuideAITool)
				m_pGuideAITool->Cancel_PlacementPick("World Scene Tool owns the next click.");
			if (auto *bern = CLevel_Bern::Get_Active())
				bern->Get_PlayerController().Cancel_DebugPlayerPlacement();
			if (auto *select = CLevel_CharacterSelect::Get_Active())
				select->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
			if (auto *valtan = CLevel_ValtanArena::Get_Active())
				valtan->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
			if (auto *kouku = CLevel_KoukuSaydonArena::Get_Active())
				kouku->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
			if (m_pMapEffectPlacementRequest)
			{
				auto *owner = m_eMapEffectPlacementOwner == DEBUG_TOOL::SEQUENCER ? m_pKoukuSaydonActionWorkbench.get()
																				  : m_pSequenceActionWorkbench.get();
				if (owner)
					owner->Cancel_MapEffectPlacementRequest(m_pMapEffectPlacementRequest->iRequestToken);
				m_pMapEffectPlacementRequest.reset();
				m_eMapEffectPlacementOwner = DEBUG_TOOL::NONE;
			}
			m_eDebugInputOwner = DEBUG_TOOL::WORLD_SCENE;
			m_bWorldMeshPickArmed = true;
			m_bWorldMeshPickLeftDown = true;
			m_iWorldMeshPickLevel = CGameInstance::Get().Get_CurrentLevelID();
			m_pWorldSceneTool->Set_Status("Click a world mesh outside the UI. Esc / right-click cancels.");
		}
	}
	float3_t hit{};
	if (!m_pWorldSceneTool->Try_GetSelectedHit(hit))
		return;
	const auto *view = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
	const auto *projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
	if (!view || !projection)
		return;
	float4_t clip;
	XMStoreFloat4(&clip, XMVector4Transform(XMVectorSet(hit.x, hit.y, hit.z, 1.f),
											XMLoadFloat4x4(view) * XMLoadFloat4x4(projection)));
	if (!std::isfinite(clip.w) || clip.w <= .0001f || clip.z < 0.f || clip.z > clip.w)
		return;
	// clip의 w로 나눠 NDC를 얻고 ImGui viewport의 실제 위치·크기로 화면 좌표를 만든다. 화면 Y는 반전한다.
	const float nx = clip.x / clip.w, ny = clip.y / clip.w;
	if (!std::isfinite(nx) || !std::isfinite(ny) || std::abs(nx) > 1.f || std::abs(ny) > 1.f)
		return;
	const auto *viewport = ImGui::GetMainViewport();
	const ImVec2 center(viewport->Pos.x + (nx * .5f + .5f) * viewport->Size.x,
						viewport->Pos.y + (.5f - ny * .5f) * viewport->Size.y);
	auto *draw = ImGui::GetBackgroundDrawList();
	const auto color = IM_COL32(255, 220, 65, 255);
	draw->AddCircle(center, 10.f, color, 24, 2.f);
	draw->AddLine({center.x - 15.f, center.y}, {center.x + 15.f, center.y}, color, 2.f);
	draw->AddLine({center.x, center.y - 15.f}, {center.x, center.y + 15.f}, color, 2.f);
}

// Development 편집기에서는 열려 있는 Area, 제품 Level에서는 registry의 Area를 사용한다. 대응 Area가 없으면 빈 문자열이다.
std::string CMainApp::GetWorldLevelAreaId() const
{
	const auto level = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
	if (level == LEVEL::DEVELOPMENT && m_pMapTool && CMapEditorWorkspaceService::Is_Active())
		return m_pMapTool->Debug_GetActiveAreaId();
	const auto *descriptor = CLevelRegistry::Find(level);
	return descriptor && descriptor->pMapAreaId ? descriptor->pMapAreaId : "";
}

// 도구의 선택 위치(m)로 현재 저작 카메라를 맞춘다. 입력 위치를 검증하고 활성 연출 preview를 종료할 수 있는지 확인한다.
// 카메라 확보·override 해제 실패는 false이며, preview 중지 자체가 이미 수행됐을 수 있다.
bool CMainApp::FocusWorldLevelPosition(const float3_t &position, const float radius, std::string &status)
{
	if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z))
	{
		status = "The selected item has no finite world position.";
		return false;
	}
	const auto level = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
	shared_ptr<CCamera_Free> camera;
	if (level == LEVEL::KOUKUSAYDON_ARENA)
	{
		if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
			camera = arena->Get_DebugCamera();
	}
	else if (level == LEVEL::VALTAN_ARENA)
	{
		if (auto *arena = CLevel_ValtanArena::Get_Active())
			camera = arena->Get_DebugCamera();
	}
	else if (level == LEVEL::CHARACTER_SELECT)
	{
		if (auto *arena = CLevel_CharacterSelect::Get_Active())
			camera = arena->Get_DebugCamera();
	}
	else if (level == LEVEL::BERN)
	{
		if (auto *bern = CLevel_Bern::Get_Active())
			camera = bern->Get_DebugCamera();
	}
	else if (level == LEVEL::DEVELOPMENT && m_pMapTool)
		camera = m_pMapTool->Debug_GetCamera();
	if (!camera)
	{
		status = "The current Level has no authoring camera ready.";
		return false;
	}
	if (camera->Is_PresentationOverrideActive())
	{
		if (m_eCompositionPreviewOwner != DEBUG_TOOL::NONE)
			StopCompositionPreview(m_eCompositionPreviewOwner);
		if (camera->Is_PresentationOverrideActive())
		{
			status = "Stop the active cinematic camera, then Focus this position.";
			return false;
		}
	}
	camera->Frame_Area(position, std::clamp(radius, 2.f, 200.f));
	status = "Camera focused on the selected world pivot. F6 returns to the player.";
	return true;
}

// Action·Sequence workbench가 요청한 Map Effect 위치 선택을 중재한다.
// 요청 token·draft·Area·Level을 확인하고 한 클릭의 GPU 위치를 기존 workbench에 전달한다. 실제 Save는 도구의 기존 경로가 담당한다.
bool CMainApp::UpdateMapEffectPlacementInput()
{
	const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	const bool rightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	if (!leftDown && !rightDown)
		m_bMapEffectPlacementSuppressMouse = false;
	const auto currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const auto workbenchFor = [this](DEBUG_TOOL owner) -> CKoukuSaydonActionWorkbench *
	{
		if (owner == DEBUG_TOOL::SEQUENCER)
			return m_pKoukuSaydonActionWorkbench.get();
		if (owner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
			return m_pSequenceActionWorkbench.get();
		return nullptr;
	};
	const auto cancel = [&]()
	{
		if (m_pMapEffectPlacementRequest)
			if (auto *workbench = workbenchFor(m_eMapEffectPlacementOwner))
				workbench->Cancel_MapEffectPlacementRequest(m_pMapEffectPlacementRequest->iRequestToken);
		m_pMapEffectPlacementRequest.reset();
		m_eMapEffectPlacementOwner = DEBUG_TOOL::NONE;
	};
	for (const auto owner : {DEBUG_TOOL::SEQUENCER, DEBUG_TOOL::SEQUENCER_BENCHMARK})
	{
		auto *workbench = workbenchFor(owner);
		KOUKU_MAP_EFFECT_PLACEMENT_REQUEST request;
		if (!workbench || !workbench->Consume_MapEffectPlacementRequest(request))
			continue;
		cancel();
		if (!m_bDeveloperToolsVisible || !IsDebugToolVisible(owner) ||
			!workbench->Is_MapEffectPlacementRequestCurrent(request) ||
			workbench->Get_Composition().strAreaId != GetWorldLevelAreaId())
		{
			workbench->Cancel_MapEffectPlacementRequest(request.iRequestToken);
			m_strMapEffectPlacementStatus = "Placement requires this box's Area to be the active Level.";
			continue;
		}
		m_eDebugInputOwner = owner;
		if (request.eAction == KOUKU_MAP_EFFECT_PLACEMENT_ACTION::FOCUS)
		{
			const auto &p = request.Selection.Position;
			(void)FocusWorldLevelPosition({float(p[0]), float(p[1]), float(p[2])}, 8.f, m_strMapEffectPlacementStatus);
			workbench->Cancel_MapEffectPlacementRequest(request.iRequestToken);
			continue;
		}
		// Move Player가 먼저 예약돼 있었어도 viewport 클릭 하나는 한 저작 소비자에게만 전달한다.
		if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
			arena->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
		if (auto *arena = CLevel_ValtanArena::Get_Active())
			arena->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
		if (auto *bern = CLevel_Bern::Get_Active())
			bern->Get_PlayerController().Cancel_DebugPlayerPlacement();
		m_pMapEffectPlacementRequest = std::make_unique<KOUKU_MAP_EFFECT_PLACEMENT_REQUEST>(request);
		m_eMapEffectPlacementOwner = owner;
		m_iMapEffectPlacementLevel = currentLevel;
		m_bMapEffectPlacementLeftDown = true;
		m_strMapEffectPlacementStatus =
			"Click a visible surface once. Esc / right-click cancels. Apply and Save keep the position.";
	}
	if (!m_pMapEffectPlacementRequest)
		return m_bMapEffectPlacementSuppressMouse;
	auto *workbench = workbenchFor(m_eMapEffectPlacementOwner);
	const bool valid = workbench && m_bDeveloperToolsVisible && IsDebugToolVisible(m_eMapEffectPlacementOwner) &&
					   m_eDebugInputOwner == m_eMapEffectPlacementOwner && currentLevel == m_iMapEffectPlacementLevel &&
					   workbench->Get_Composition().strAreaId == GetWorldLevelAreaId() &&
					   workbench->Is_MapEffectPlacementRequestCurrent(*m_pMapEffectPlacementRequest);
	const HWND foreground = GetForegroundWindow();
	DWORD foregroundProcess = 0u;
	if (foreground)
		GetWindowThreadProcessId(foreground, &foregroundProcess);
	if (!valid || foregroundProcess != GetCurrentProcessId() || rightDown ||
		(GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
	{
		cancel();
		m_bMapEffectPlacementSuppressMouse = leftDown || rightDown;
		m_strMapEffectPlacementStatus = "Placement cancelled; the previous box position was preserved.";
		return true;
	}
	const bool pressed = leftDown && !m_bMapEffectPlacementLeftDown;
	m_bMapEffectPlacementLeftDown = leftDown;
	if (!pressed || foreground != g_hWnd || ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput ||
		CUIInputRouter::Get().Is_MouseClaimedThisFrame() || CUIInputRouter::Get().Was_MouseClaimedLastFrame())
		return true;
	CUIInputRouter::Get().Claim_Mouse_This_Frame();
	m_bMapEffectPlacementSuppressMouse = true;
	float4_t picked{};
	if (!CGameInstance::Get().Picking(picked) || !std::isfinite(picked.x) || !std::isfinite(picked.y) ||
		!std::isfinite(picked.z))
	{
		m_strMapEffectPlacementStatus =
			"No visible mesh surface at this pixel. Click a surface again, or Esc to cancel.";
		return true;
	}
	const bool completed = workbench->Complete_MapEffectPlacementRequest(
		*m_pMapEffectPlacementRequest, {picked.x, picked.y, picked.z}, m_strMapEffectPlacementStatus);
	cancel();
	if (completed)
		m_strMapEffectPlacementStatus =
			"Picked world position staged in Box Detail. Preview now; Apply and Save keep it.";
	return true;
}

// Guide trigger 박스는 보이는 표면의 월드 위치를 소비한다.
// 맵 메시 identity 선택은 UpdateWorldMeshInspectionInput의 ray 검사를 사용하며 이 GPU 위치 읽기로 대신하지 않는다.
// Guide 박스의 일회 위치 선택을 처리한다. UI가 쓰지 않은 클릭만 소비하고 보이는 표면의 월드 위치를 전달한다.
// 소유 도구·Area·Level 변경이나 취소는 이전 박스 위치를 유지하며 표면 miss는 다음 클릭을 기다린다.
bool CMainApp::UpdateWorldLevelPlacementPickInput()
{
	const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	const bool rightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	if (!leftDown && !rightDown)
		m_bWorldLevelPickSuppressMouse = false;
	if (!m_bWorldLevelPickArmed)
		return m_bWorldLevelPickSuppressMouse;
	const auto currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool targetReady = m_eWorldLevelPickOwner == DEBUG_TOOL::GUIDE_AI && m_pGuideAITool &&
							 m_pGuideAITool->Is_PlacementPickArmed() &&
							 m_pGuideAITool->Get_PlacementPickAreaId() == GetWorldLevelAreaId();
	const bool valid = targetReady && m_bDeveloperToolsVisible && IsDebugToolVisible(m_eWorldLevelPickOwner) &&
					   m_eDebugInputOwner == m_eWorldLevelPickOwner && currentLevel == m_iWorldLevelPickLevel;
	const HWND foreground = GetForegroundWindow();
	DWORD foregroundProcess = 0u;
	if (foreground)
		GetWindowThreadProcessId(foreground, &foregroundProcess);
	if (!valid || foregroundProcess != GetCurrentProcessId() || rightDown ||
		(GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
	{
		m_bWorldLevelPickArmed = false;
		if (m_pGuideAITool)
			m_pGuideAITool->Cancel_PlacementPick("Pick cancelled; the previous Guide box position was preserved.");
		m_bWorldLevelPickSuppressMouse = leftDown || rightDown;
		return true;
	}
	const bool pressed = leftDown && !m_bWorldLevelPickLeftDown;
	m_bWorldLevelPickLeftDown = leftDown;
	if (!pressed || foreground != g_hWnd || ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput ||
		CUIInputRouter::Get().Is_MouseClaimedThisFrame() || CUIInputRouter::Get().Was_MouseClaimedLastFrame())
		return true;
	CUIInputRouter::Get().Claim_Mouse_This_Frame();
	m_bWorldLevelPickSuppressMouse = true;
	float4_t picked{};
	if (!CGameInstance::Get().Picking(picked) || !std::isfinite(picked.x) || !std::isfinite(picked.y) ||
		!std::isfinite(picked.z))
	{
		// 픽셀 아래 표면이 없으면 예약을 유지하여 다음 클릭을 받을 수 있게 한다.
		const std::string status = "No visible mesh surface at this pixel. Click a surface again, or Esc to cancel.";
		m_pGuideAITool->Set_Status(status);
		return true;
	}
	m_bWorldLevelPickArmed = false;
	m_pGuideAITool->Complete_PlacementPick({picked.x, picked.y, picked.z});
	return true;
}

// 현재 workbench의 선택 위치와 1m 월드 축을 Debug overlay에 투영한다.
// 시작·종료 ms는 표시할 때 초로 환산하며 화면 밖 위치도 안내 문구로 남긴다.
void CMainApp::RenderMapEffectPlacementMarker()
{
	if (!m_bDeveloperToolsVisible)
		return;
	auto *workbench = m_eDebugInputOwner == DEBUG_TOOL::SEQUENCER			  ? m_pKoukuSaydonActionWorkbench.get()
					  : m_eDebugInputOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK ? m_pSequenceActionWorkbench.get()
																			  : nullptr;
	KOUKU_MAP_EFFECT_PLACEMENT selection;
	if (!workbench || !IsDebugToolVisible(m_eDebugInputOwner) ||
		workbench->Get_Composition().strAreaId != GetWorldLevelAreaId() ||
		!workbench->Get_SelectedMapEffectPlacement(selection))
		return;
	const auto *view = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
	const auto *projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
	if (!view || !projection)
		return;
	const auto viewProjection = XMLoadFloat4x4(view) * XMLoadFloat4x4(projection);
	const auto *viewport = ImGui::GetMainViewport();
	const auto project = [&](double x, double y, double z, ImVec2 &screen)
	{
		float4_t clip;
		XMStoreFloat4(&clip, XMVector4Transform(XMVectorSet(float(x), float(y), float(z), 1.f), viewProjection));
		if (!std::isfinite(clip.w) || clip.w <= 0.0001f || clip.z < 0.f || clip.z > clip.w)
			return false;
		const float nx = clip.x / clip.w, ny = clip.y / clip.w;
		if (!std::isfinite(nx) || !std::isfinite(ny) || std::abs(nx) > 1.f || std::abs(ny) > 1.f)
			return false;
		screen = {viewport->Pos.x + (nx * .5f + .5f) * viewport->Size.x,
				  viewport->Pos.y + (.5f - ny * .5f) * viewport->Size.y};
		return true;
	};
	auto *draw = ImGui::GetBackgroundDrawList();
	const auto &p = selection.Position;
	ImVec2 center;
	const bool onScreen = project(p[0], p[1], p[2], center);
	const ImU32 color = IM_COL32(255, 220, 65, 255);
	if (onScreen)
	{
		draw->AddCircle(center, 10.f, color, 24, 2.f);
		draw->AddLine({center.x - 16.f, center.y}, {center.x + 16.f, center.y}, color, 2.f);
		draw->AddLine({center.x, center.y - 16.f}, {center.x, center.y + 16.f}, color, 2.f);
		const ImU32 colors[] = {IM_COL32(255, 80, 80, 255), IM_COL32(80, 255, 110, 255), IM_COL32(85, 155, 255, 255)};
		const char *axes[] = {"X", "Y", "Z"};
		for (size_t axis = 0; axis < 3; ++axis)
		{
			auto end = p;
			end[axis] += 1.;
			ImVec2 endpoint;
			if (project(end[0], end[1], end[2], endpoint))
			{
				draw->AddLine(center, endpoint, colors[axis], 2.f);
				draw->AddText(endpoint, colors[axis], axes[axis]);
			}
		}
	}
	char label[256]{};
	sprintf_s(label, "MAP Effect pivot%s | XYZ %.3f / %.3f / %.3f m | box %.3f - %.3f s",
			  onScreen ? "" : " OFF SCREEN - Focus (F) in Box Detail", p[0], p[1], p[2], selection.iStartMs / 1000.,
			  (double(selection.iStartMs) + selection.iDurationMs) / 1000.);
	const ImVec2 textPosition = onScreen ? ImVec2(center.x + 18.f, center.y + 18.f)
										 : ImVec2(viewport->Pos.x + 20.f, viewport->Pos.y + viewport->Size.y - 85.f);
	draw->AddText({textPosition.x + 1.f, textPosition.y + 1.f}, IM_COL32(0, 0, 0, 255), label);
	draw->AddText(textPosition, color, label);
	if (!m_strMapEffectPlacementStatus.empty())
		draw->AddText({viewport->Pos.x + 20.f, viewport->Pos.y + viewport->Size.y - 60.f}, color,
					  m_strMapEffectPlacementStatus.c_str());
}

// MainApp 갱신이 통합 목록에 활성 Area와 현재 composition draft 세대를 전달한다.
// Map Tool 준비 요청은 60초 안에서 재조회하며 도구가 닫히거나 준비가 끝나면 예약을 해제한다.
void CMainApp::UpdateWorldLevelTool()
{
	if (!m_pWorldLevelTool)
		return;
	m_pWorldLevelTool->Set_ActiveArea(GetWorldLevelAreaId());
	for (const auto owner : {WORLD_LEVEL_COMPOSITION_OWNER::ACTION, WORLD_LEVEL_COMPOSITION_OWNER::SEQUENCE})
	{
		auto *workbench = owner == WORLD_LEVEL_COMPOSITION_OWNER::ACTION ? m_pKoukuSaydonActionWorkbench.get()
																		 : m_pSequenceActionWorkbench.get();
		m_pWorldLevelTool->Set_CompositionView(
			owner, workbench && workbench->Has_Composition() ? &workbench->Get_Composition() : nullptr,
			workbench ? workbench->Get_DraftGeneration() : 0u);
	}
	if (!m_pWorldLevelPendingMapRequest)
		return;
	if (!m_pMapTool || !m_bDeveloperToolsVisible || !IsDebugToolVisible(DEBUG_TOOL::WORLD_LEVEL) ||
		std::chrono::steady_clock::now() > m_WorldLevelMapDeadline)
	{
		m_pWorldLevelPendingMapRequest.reset();
		m_pWorldLevelTool->Set_Status("Map selection ended; existing owner drafts were preserved.");
		return;
	}
	const auto request = *m_pWorldLevelPendingMapRequest;
	std::string status;
	const int result =
		!request.sequenceInstanceId.empty() || !request.sourceItemId.empty()
			? m_pMapTool->Debug_SequenceViewer(request.areaId, request.sequenceInstanceId, request.sourceItemId, false,
											   false, nullptr, status)
			: m_pMapTool->Debug_WorldLevelSelection(request.areaId, request.placementId, request.deploy, status);
	m_pWorldLevelTool->Set_Status(status);
	if (result != 0)
	{
		m_pWorldLevelPendingMapRequest.reset();
		if (result > 0)
		{
			m_eDebugInputOwner = DEBUG_TOOL::MAP;
			m_eDebugWindowFocusPending = DEBUG_TOOL::MAP;
		}
	}
}

namespace
{
// chunk 경계선 두 끝점을 homogeneous clip 공간의 일곱 반공간으로 자른다.
// 성공한 끝점만 화면 변환에 사용한다. 마지막 유한성 검사 이전에 a/b를 갱신하므로 false의 출력 보존을 약속하지 않는다.
bool ClipChunkOverlayEdge(float4_t &a, float4_t &b)
{
	const auto finite = [](const float4_t &p)
	{ return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z) && std::isfinite(p.w); };
	if (!finite(a) || !finite(b))
		return false;
	// w > 0 여유, 좌·우·하·상, Direct3D의 z ≥ 0 및 z ≤ w를 순서대로 선형 부등식으로 표현한다.
	const double av[] = {double(a.w) - .0001, double(a.x) + a.w, double(a.w) - a.x,
						 double(a.y) + a.w,	  double(a.w) - a.y, a.z,
						 double(a.w) - a.z};
	const double bv[] = {double(b.w) - .0001, double(b.x) + b.w, double(b.w) - b.x,
						 double(b.y) + b.w,	  double(b.w) - b.y, b.z,
						 double(b.w) - b.z};
	// 원래 선분의 t∈[0,1] 중 각 평면 안쪽에 남는 구간만 누적한다. 나눗셈 전 양끝의 부호가 다른 경우만 교점을 구한다.
	double first = 0., last = 1.;
	for (size_t plane = 0; plane < 7; ++plane)
	{
		if (av[plane] < 0. && bv[plane] < 0.)
			return false;
		if ((av[plane] < 0.) != (bv[plane] < 0.))
		{
			const double t = av[plane] / (av[plane] - bv[plane]);
			if (av[plane] < 0.)
				first = (std::max)(first, t);
			else
				last = (std::min)(last, t);
		}
	}
	if (first > last)
		return false;
	const auto lerp = [&](double t)
	{
		return float4_t{float(a.x + (double(b.x) - a.x) * t), float(a.y + (double(b.y) - a.y) * t),
						float(a.z + (double(b.z) - a.z) * t), float(a.w + (double(b.w) - a.w) * t)};
	};
	const float4_t begin = lerp(first), end = lerp(last);
	a = begin;
	b = end;
	return finite(a) && finite(b) && a.w > 0.f && b.w > 0.f;
}

// World Level의 읽기 전용 chunk 경계를 화면 선으로 표시한다.
// 선택·유효성·원경 활성 상태는 색으로 구분하며 실제 chunk culling·LOD·저작 정책은 변경하지 않는다.
void DrawWorldLevelChunkBounds(const CWorldLevelTool &tool)
{
	if (!tool.Show_ChunkBounds())
		return;
	auto &game = CGameInstance::Get();
	const auto *view = game.Get_Transform(D3DTS::VIEW);
	const auto *projection = game.Get_Transform(D3DTS::PROJ);
	if (!view || !projection)
		return;
	const matrix_t vp = XMLoadFloat4x4(view) * XMLoadFloat4x4(projection);
	auto *viewport = ImGui::GetMainViewport();
	if (!viewport || viewport->Size.x <= 0.f || viewport->Size.y <= 0.f)
		return;
	auto *draw = ImGui::GetBackgroundDrawList(viewport);
	const auto screen = [viewport](const float4_t &p)
	{
		return ImVec2(viewport->Pos.x + (p.x / p.w * .5f + .5f) * viewport->Size.x,
					  viewport->Pos.y + (.5f - p.y / p.w * .5f) * viewport->Size.y);
	};
	constexpr uint8_t edges[12][2] = {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {0, 2}, {1, 3},
									  {4, 6}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};
	for (const auto &row : tool.Get_ChunkView())
	{
		const bool selected = row.chunkId == tool.Get_SelectedChunk();
		if (!tool.Show_AllChunkBounds() && !selected)
			continue;
		const auto &lo = row.minimum;
		const auto &hi = row.maximum;
		if (!std::isfinite(lo.x) || !std::isfinite(lo.y) || !std::isfinite(lo.z) || !std::isfinite(hi.x) ||
			!std::isfinite(hi.y) || !std::isfinite(hi.z) || lo.x > hi.x || lo.y > hi.y || lo.z > hi.z)
			continue;
		float4_t corners[8];
		for (size_t i = 0; i < 8; ++i)
			XMStoreFloat4(&corners[i],
						  XMVector4Transform(
							  XMVectorSet(i & 1 ? hi.x : lo.x, i & 2 ? hi.y : lo.y, i & 4 ? hi.z : lo.z, 1.f), vp));
		const ImU32 color = selected						? IM_COL32(255, 220, 65, 240)
							: !row.valid					? IM_COL32(255, 95, 95, 150)
							: row.farSelected && row.active ? IM_COL32(80, 230, 150, 150)
															: IM_COL32(90, 175, 255, 140);
		for (const auto &edge : edges)
		{
			auto a = corners[edge[0]], b = corners[edge[1]];
			if (ClipChunkOverlayEdge(a, b))
				draw->AddLine(screen(a), screen(b), color, selected ? 2.f : 1.f);
		}
	}
}
} // namespace

// 통합 World Level 창의 typed 요청을 각 기존 저작 소유자에게 연결한다.
// 현재 runtime generation과 stable ID를 확인해 선택·focus·창 열기 또는 명시적 chunk 표시 변경을 수행한다.
void CMainApp::RenderWorldLevelTool()
{
	if (!IsDebugToolVisible(DEBUG_TOOL::WORLD_LEVEL) || !m_pWorldLevelTool)
		return;
	if (m_eDebugWindowFocusPending == DEBUG_TOOL::WORLD_LEVEL)
	{
		ImGui::SetNextWindowFocus();
		m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
	}
	auto *chunkHost = Find_ActiveMapAuthoringHost();
	auto *chunkRuntime = chunkHost ? &chunkHost->Get_MapAuthoringRuntime() : nullptr;
	const std::string chunkArea = chunkHost ? chunkHost->Get_MapAuthoringCatalog().Get_AreaId() : GetWorldLevelAreaId();
	const uint64_t chunkGeneration = chunkRuntime ? chunkRuntime->Debug_GetRuntimeGeneration() : 0u;
	// runtime 주소뿐 아니라 세대가 바뀌었는지 확인해 이전 Area의 chunk 조회 결과를 재사용하지 않는다.
	if (m_pWorldLevelTool->Needs_ChunkView(chunkArea, chunkRuntime, chunkGeneration))
	{
		const auto policy = chunkRuntime ? chunkRuntime->Get_ChunkPolicy() : nullptr;
		m_pWorldLevelTool->Set_ChunkView(chunkArea, chunkRuntime, chunkGeneration,
										 chunkRuntime ? chunkRuntime->Get_ChunkDebugRows()
													  : std::vector<MAP_CHUNK_DEBUG_ROW>{},
										 policy && policy->enabled, policy && policy->hlodEnabled);
	}
	m_pWorldLevelTool->Render();
	DrawWorldLevelChunkBounds(*m_pWorldLevelTool);
	if (m_pWorldLevelTool->Consume_InteractionRequest())
		m_eDebugInputOwner = DEBUG_TOOL::WORLD_LEVEL;
	if (!m_pWorldLevelTool->Is_Open())
	{
		SetDebugToolVisible(DEBUG_TOOL::WORLD_LEVEL, false);
		return;
	}
	WORLD_LEVEL_TOOL_REQUEST request;
	if (!m_pWorldLevelTool->Consume_Request(request))
	{
		m_pWorldLevelTool->Render_MetadataDetails();
		return;
	}
	std::string status;
	if (request.kind == WORLD_LEVEL_REQUEST_KIND::INSPECT_METADATA)
	{
		if (m_pWorldSceneTool && m_pWorldSceneTool->Is_DetailsMode())
			SetDebugToolVisible(DEBUG_TOOL::WORLD_SCENE, false);
	}
	else if (request.kind == WORLD_LEVEL_REQUEST_KIND::INSPECT_PLACEMENT)
	{
		(void)Debug_OpenWorldObjectDetails(request.areaId, request.placementId, request.deploy, status);
		m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
		m_eDebugInputOwner = DEBUG_TOOL::WORLD_LEVEL;
	}
	// 사용자가 명시한 현재 세대의 표시 스위치만 바꾼다. 저장 배치·재질·렌더링 profile 파일을 쓰지 않는다.
	else if (request.kind == WORLD_LEVEL_REQUEST_KIND::SET_CHUNK_MODE)
	{
		if (!chunkRuntime || request.areaId != chunkArea || request.runtimeGeneration != chunkGeneration)
			status = "Chunk source changed; refresh the live Area before changing its display mode.";
		else if (const auto &policy = chunkRuntime->Get_ChunkPolicy())
		{
			policy->enabled = request.chunkEnabled;
			policy->hlodEnabled = request.chunkHlodEnabled;
			status = "Runtime chunk display mode changed. Saved placements and materials are unchanged.";
		}
	}
	else if (request.kind == WORLD_LEVEL_REQUEST_KIND::OPEN_GUIDE)
	{
		status = SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::GUIDE_AI)) ? "DimensionMaster Guide opened."
																  : "DimensionMaster Guide could not open.";
	}
	else if (request.kind == WORLD_LEVEL_REQUEST_KIND::FOCUS)
	{
		if (request.areaId != GetWorldLevelAreaId())
			status = "Enter this Area before focusing its world position.";
		else if (request.placementId)
		{
			if (Debug_OpenWorldObjectDetails(request.areaId, request.placementId, request.deploy, status))
				(void)m_pWorldSceneTool->Request_SelectedFocus();
			m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
		}
		else
			(void)FocusWorldLevelPosition(request.position, request.focusRadius, status);
	}
	else if (request.kind == WORLD_LEVEL_REQUEST_KIND::PICK_IN_SCENE)
	{
		if (Debug_OpenWorldObjectDetails(request.areaId, 0u, false, status))
		{
			m_pWorldSceneTool->Request_ScenePick();
			m_eDebugInputOwner = DEBUG_TOOL::WORLD_SCENE;
			m_eDebugWindowFocusPending = DEBUG_TOOL::WORLD_SCENE;
			status = "Pick in scene opened. Click a visible map or Deploy object outside the UI.";
		}
	}
	else if (request.kind == WORLD_LEVEL_REQUEST_KIND::OPEN_LIGHT)
	{
		LEVEL renderingLevel = LEVEL::END;
		for (const auto level : {LEVEL::KOUKUSAYDON_ARENA, LEVEL::VALTAN_ARENA, LEVEL::BERN, LEVEL::CHARACTER_SELECT})
			if (const auto *descriptor = CLevelRegistry::Find(level);
				descriptor && descriptor->pMapAreaId && request.areaId == descriptor->pMapAreaId)
				renderingLevel = level;
		if (request.areaId != GetWorldLevelAreaId())
			status = "Enter this Area before editing its live map lights.";
		else if (renderingLevel == LEVEL::END)
			status = "This Area has no Rendering Workbench Level binding; its lights are listed for inspection.";
		else if (m_AreaLightSession.Is_Open() && m_AreaLightSession.Get_AreaId() != request.areaId)
			status = "Rendering Workbench has another Area open. Finish its light edits before switching Areas.";
		else if (FAILED(EnsureDebugTool(DEBUG_TOOL::RENDERING)))
			status = "Rendering Workbench could not initialize.";
		else if (!m_AreaLightSession.Is_Open() && !m_AreaLightSession.Open(request.areaId, status))
		{
		}
		else
		{
			const auto &lights = m_AreaLightSession.Get_Document().Get_Lights();
			if (std::none_of(lights.begin(), lights.end(),
							 [&](const auto &light) { return light.lightId == request.sourceItemId; }))
				status = "The selected light is absent from the current Rendering Workbench draft.";
			else
			{
				m_iRenderingLastLevel = CGameInstance::Get().Get_CurrentLevelID();
				m_eRenderingSelectedLevel = renderingLevel;
				m_strRenderingLightAreaAttempt = request.areaId;
				SelectRenderingLight("map:" + request.sourceItemId);
				m_eDebugInputOwner = DEBUG_TOOL::RENDERING;
				m_eDebugWindowFocusPending = DEBUG_TOOL::RENDERING;
				status = "Selected the map light in Rendering Workbench. Its existing Save controls own changes.";
			}
		}
	}
	else if (request.kind == WORLD_LEVEL_REQUEST_KIND::OPEN_WORLD_OBJECT)
	{
		if (request.areaId != "LV_LUT_MIDNIGHTC_ED")
			status = "This Area's Object motions are edited in Map Tool.";
		else if (FAILED(EnsureDebugTool(DEBUG_TOOL::WORLD_OBJECT)))
			status = "Object Tool could not initialize.";
		else if (request.objectId.empty() && request.sequenceInstanceId.empty())
			status = "World Object Tool opened. Create an Object in its Resources pane.";
		else
			(void)m_pWorldObjectTool->Open_ObjectMotion(request.objectId, request.sequenceInstanceId, status);
	}
	else if (request.kind == WORLD_LEVEL_REQUEST_KIND::OPEN_COMPOSITION)
	{
		const bool sequence = request.compositionOwner == WORLD_LEVEL_COMPOSITION_OWNER::SEQUENCE;
		const auto owner = sequence ? DEBUG_TOOL::SEQUENCER_BENCHMARK : DEBUG_TOOL::SEQUENCER;
		auto *shell = m_pSequencerTool.get();
		if (!shell && FAILED(EnsureDebugTool(owner)))
			status = "Composition could not initialize.";
		else
		{
			auto *workbench = sequence ? m_pSequenceActionWorkbench.get() : m_pKoukuSaydonActionWorkbench.get();
			shell = m_pSequencerTool.get();
			if (!workbench->Has_Composition() && !workbench->Reload(status))
			{
			}
			else if (workbench->Get_Composition().strAreaId != request.areaId)
				status = "This Area's sequences are edited in Map Tool; this Composition belongs to another Area.";
			else
			{
				const auto &patterns = workbench->Get_Composition().Patterns;
				const auto pattern = std::find_if(patterns.begin(), patterns.end(), [&](const auto &value)
												  { return value.strPatternId == request.patternId; });
				auto boss = shell->Get_SelectedBoss();
				if (pattern != patterns.end())
				{
					const auto &gate = pattern->strGateId;
					boss = gate == "GATE2"	 ? COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE2
						   : gate == "GATE3" ? COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE3
						   : gate == "BINGO" ? COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_ENCORE
											 : COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON;
				}
				else if (boss == COMPOSITION_WORKBENCH_BOSS::VALTAN)
					boss = COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON;
				// 같은 관문 창을 열면서 현재 박스 편집이 버려지지 않도록 대상 보스·Sequence 구분을 먼저 설정한다.
				shell->Open(sequence ? COMPOSITION_WORKBENCH_TARGET::SEQUENCE : COMPOSITION_WORKBENCH_TARGET::BOSS,
							boss);
				bool selected = request.patternId.empty();
				if (!request.occurrenceId.empty())
				{
					const bool worldBox =
						pattern != patterns.end() &&
						std::any_of(pattern->WorldOccurrences.begin(), pattern->WorldOccurrences.end(),
									[&](const auto &value) { return value.strOccurrenceId == request.occurrenceId; });
					selected =
						worldBox
							? workbench->Select_WorldBoxById(request.patternId, request.occurrenceId, status)
							: workbench->Select_PresentationBoxById(request.patternId, request.occurrenceId, status);
				}
				else if (!request.patternId.empty())
					selected = workbench->Select_PatternById(request.patternId, status);
				else
					status = "Composition opened. Create a Sequence/Pattern using its existing authoring controls.";
				if (selected)
				{
					SetDebugToolVisible(owner, true);
					m_eDebugInputOwner = owner;
					m_eDebugWindowFocusPending = owner;
				}
			}
		}
	}
	else if (request.kind == WORLD_LEVEL_REQUEST_KIND::OPEN_MAP)
	{
		const auto *host = Find_ActiveMapAuthoringHost();
		if (host && request.areaId == host->Get_MapAuthoringCatalog().Get_AreaId() &&
			request.sequenceInstanceId.empty() && request.sourceItemId.empty())
		{
			if (Debug_OpenWorldObjectDetails(request.areaId, request.placementId, request.deploy, status))
			{
				m_pWorldSceneTool->Set_Status(status);
				m_eDebugInputOwner = DEBUG_TOOL::WORLD_SCENE;
				m_eDebugWindowFocusPending = DEBUG_TOOL::WORLD_SCENE;
			}
		}
		else if (!CMapEditorWorkspaceService::Is_Active() ||
				 CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::DEVELOPMENT))
			status =
				"This item needs its owning Area/editor. Scene picking remains available for the current Level; existing drafts are preserved.";
		else if (FAILED(EnsureDebugTool(DEBUG_TOOL::MAP)))
			status = "Map Tool could not initialize.";
		else
		{
			m_pWorldLevelPendingMapRequest = std::make_unique<WORLD_LEVEL_TOOL_REQUEST>(request);
			m_WorldLevelMapDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
			status = "Preparing the selected Area in Map Tool...";
		}
	}
	m_pWorldLevelTool->Set_Status(std::move(status));
	m_pWorldLevelTool->Render_MetadataDetails();
}
#endif
