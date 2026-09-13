#include "VehiclePresentationAssetService.h"

#include "ActorCatalog.h"
#include "GameInstance.h"
#include "Model.h"

#include <map>
#include <mutex>
#include <set>
#include <string_view>

namespace
{
	std::mutex g_VehicleAssetMutex;
	std::map<uint32_t, std::set<std::uint32_t>> g_ReadyVehiclesByLevel;
	std::string g_VehicleAssetStatus;

	HRESULT Reject(const std::string& reason)
	{
		g_VehicleAssetStatus = reason;
		OutputDebugStringA(("[Client][VehiclePresentation] " + reason + "\n").c_str());
		return E_FAIL;
	}

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

	unique_ptr<Engine::CModel> model = Engine::CModel::Create(
		pDevice, pContext, MODEL::ANIM, load,
		XMMatrixScaling(vehicle->modelPreScale, vehicle->modelPreScale, vehicle->modelPreScale) *
		XMMatrixRotationY(XMConvertToRadians(-90.f)));
	if (nullptr == model || 0u == model->Get_NumMeshes() || !model->Has_Animations())
		return Reject("Vehicle model has no usable animated geometry: " + vehicle->modelAssetId);
	if (!model->Has_Bone(vehicle->seatBone.c_str()) ||
		!Has_Clip(*model, vehicle->vehicleIdleClip) ||
		!Has_Clip(*model, vehicle->vehicleRunClip))
	{
		return Reject("Vehicle model is missing its seat bone or idle/run clips: " + vehicle->modelAssetId);
	}
	if (FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex, Get_ModelPrototypeTag(vehicleId), std::move(model))))
	{
		return Reject("Vehicle model prototype registration failed: " + vehicle->modelAssetId);
	}

	g_ReadyVehiclesByLevel[iLevelIndex].insert(vehicleId);
	g_VehicleAssetStatus.clear();
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
