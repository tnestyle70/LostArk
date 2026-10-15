#include "VehiclePresentationAssetService.h"
// 탈것에 선택적 저작 clip을 추가한다.
#include "BoneAnimationDocument.h"
// 자원 준비의 실패 이유와 성공 모델 ID를 진단에 기록한다.
#include "EffectFailureDiagnostic.h"

// vehicle 정의·재질 입력·좌석 본·필수 idle/run clip을 조회한다.
#include "ActorCatalog.h"
// 검증한 탈것 Model 원형을 해당 Level에 등록한다.
#include "GameInstance.h"
// Model 생성과 geometry·본·clip 유효성 검사를 수행한다.
#include "Model.h"

// Level별 탈것 준비 목록을 보관한다.
#include <map>
// 준비·조회·Level 초기화의 공용 기록 접근을 잠근다.
#include <mutex>
// 준비 완료 vehicle ID를 중복 없이 기억한다.
#include <set>
// 필수 clip 이름을 복사 없이 조회 helper에 전달한다.
#include <string_view>

namespace
{
	std::mutex g_VehicleAssetMutex;
	std::map<uint32_t, std::set<std::uint32_t>> g_ReadyVehiclesByLevel;
	// 마지막 준비 실패를 저장하고 성공 시 비운다. 외부 Get_Status는 이 문자열을 잠금 없이 차용한다.
	std::string g_VehicleAssetStatus;

	// 준비 함수가 lock을 가진 상태에서 실패 이유를 저장·출력하고 E_FAIL을 반환한다.
	HRESULT Reject(const std::string& reason)
	{
		g_VehicleAssetStatus = reason;
		Client::Write_EffectFailureDiagnostic("vehicle.model", reason);
		OutputDebugStringA(("[Client][VehiclePresentation] " + reason + "\n").c_str());
		return E_FAIL;
	}

	// Model의 현재 clip 목록을 이름으로 검색한다. animation 재생 상태는 바꾸지 않는다.
	bool_t Has_Clip(const Engine::CModel& model, const std::string_view clip)
	{
		for (uint32_t index = 0u; index < model.Get_NumAnimations(); ++index)
		{
			const char_t* name = model.Get_AnimationName(index);
			if (nullptr != name && clip == name)
				return true;
		}
		return false;
	}
}

void Client::CVehiclePresentationAssetService::Begin_LevelLoad(const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_VehicleAssetMutex };
	g_ReadyVehiclesByLevel.erase(iLevelIndex);
}

// Loader의 선행 준비 또는 Debug Character의 최초 탑승이 요청한 vehicle 하나를 준비한다.
HRESULT Client::CVehiclePresentationAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex,
	const std::uint32_t vehicleId)
{
	if (nullptr == pDevice || nullptr == pContext || iLevelIndex >= ETOUI(LEVEL::END))
		return E_INVALIDARG;

	std::scoped_lock lock{ g_VehicleAssetMutex };
	if (g_ReadyVehiclesByLevel[iLevelIndex].contains(vehicleId))
		return S_FALSE;

	const VEHICLE_ACTOR_ENTRY* vehicle = CActorCatalog::Find_Vehicle(vehicleId);
	if (nullptr == vehicle)
		return Reject("Vehicle is not in the catalog: " + std::to_string(vehicleId));

	Engine::MODEL_ASSET_LOAD_DESC load;
	std::string materialStatus;
	if (!CActorCatalog::Build_ModelLoadDescription(vehicle->modelAssetId, load, materialStatus))
		return Reject("Vehicle material input failed: " + materialStatus);

	// scale→yaw→높이 보정 순서의 preTransform으로 모델을 만들고 등록 전까지 임시 소유한다.
	unique_ptr<Engine::CModel> model = Engine::CModel::Create(
		pDevice, pContext, MODEL::ANIM, load,
		XMMatrixScaling(vehicle->modelPreScale, vehicle->modelPreScale, vehicle->modelPreScale) *
		XMMatrixRotationY(XMConvertToRadians(vehicle->modelYawDegrees)) *
		XMMatrixTranslation(0.f, vehicle->modelLiftMeters, 0.f));
	if (nullptr == model || 0u == model->Get_NumMeshes() || !model->Has_Animations())
		return Reject("Vehicle model has no usable animated geometry: " + vehicle->modelAssetId);
	std::string authoredClipStatus;
	// 저작 clip 실패는 진단만 남긴다. 이후 좌석 본과 필수 idle/run 검사는 그대로 수행한다.
	if (!CBoneAnimationDocument::Load_IntoModel(*model, "Vehicle_" + std::to_string(vehicleId), authoredClipStatus))
		OutputDebugStringA(("[AuthoredAnimation] " + authoredClipStatus + "\n").c_str());
	if (!model->Has_Bone(vehicle->seatBone.c_str()) ||
		!Has_Clip(*model, vehicle->vehicleIdleClip) ||
		!Has_Clip(*model, vehicle->vehicleRunClip))
	{
		return Reject("Vehicle model is missing its seat bone or idle/run clips: " + vehicle->modelAssetId);
	}
	// 모든 필수 조건이 맞은 Model의 소유권을 Engine으로 넘긴 뒤 준비 기록을 추가한다.
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex, Get_ModelPrototypeTag(vehicleId), std::move(model))))
	{
		return Reject("Vehicle model prototype registration failed: " + vehicle->modelAssetId);
	}

	g_ReadyVehiclesByLevel[iLevelIndex].insert(vehicleId);
	g_VehicleAssetStatus.clear();
	Client::Write_EffectFailureDiagnostic("vehicle.model", "ready vehicleId=" +
		std::to_string(vehicleId) + " model=" + vehicle->modelAssetId);
	return S_OK;
}

bool_t Client::CVehiclePresentationAssetService::Is_Ready(
	const uint32_t iLevelIndex, const std::uint32_t vehicleId)
{
	std::scoped_lock lock{ g_VehicleAssetMutex };
	const auto level = g_ReadyVehiclesByLevel.find(iLevelIndex);
	return g_ReadyVehiclesByLevel.end() != level && level->second.contains(vehicleId);
}

Engine::wstring_t Client::CVehiclePresentationAssetService::Get_ModelPrototypeTag(
	const std::uint32_t vehicleId)
{
	return Engine::wstring_t(TEXT("Prototype_Component_Model_Vehicle_")) + std::to_wstring(vehicleId);
}

const std::string& Client::CVehiclePresentationAssetService::Get_Status()
{
	return g_VehicleAssetStatus;
}
