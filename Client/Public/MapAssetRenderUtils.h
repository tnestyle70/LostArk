#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapLoadScope.h"

NS_BEGIN(Engine)

class CModel;
class CShader;

NS_END

NS_BEGIN(Client)

struct MAP_CAMERA_CULL_SNAPSHOT final
{
	uint64_t revision = {};
	float4x4_t view = {};
	float4x4_t projection = {};
	float4_t worldPlanes[6] = {};
};

struct MAP_FRUSTUM_CULL_DECISION final
{
	bool_t wouldBeVisible = true;
	bool_t shouldRender = true;
	bool_t largeGeometry = false;
	f32_t baseRadius = {};
	f32_t margin = {};
	f32_t effectiveRadius = {};
	f32_t planeDistances[6] = {};
};

class CMapAssetRenderUtils final
{
public:
	static bool_t Capture_CameraCullSnapshot(
		MAP_CAMERA_CULL_SNAPSHOT& outSnapshot);
	static HRESULT Bind_CameraCullSnapshot(
		const shared_ptr<Engine::CShader>& shader,
		const MAP_CAMERA_CULL_SNAPSHOT& snapshot);
	static bool_t Evaluate_FrustumVisibility(
		const MAP_FRUSTUM_CULLING_POLICY& policy,
		const MAP_CAMERA_CULL_SNAPSHOT& snapshot,
		const std::string& assetId,
		const std::string& assetGroupId,
		uint64_t placementId,
		const float3_t& worldCenter,
		f32_t worldRadius,
		MAP_FRUSTUM_RUNTIME_STATE& state,
		MAP_FRUSTUM_CULL_DECISION& outDecision);
	static void Begin_FrustumDiagnostics(
		const std::string& areaId,
		const MAP_FRUSTUM_CULLING_POLICY& policy);

	static uint32_t Select_Pass(
		const MAP_ASSET_RENDER_PROFILE& profile,
		bool_t mirrored);

	static HRESULT Bind_Material(
		const shared_ptr<Engine::CModel>& model,
		const shared_ptr<Engine::CShader>& shader,
		uint32_t meshIndex,
		const MAP_ASSET_RENDER_PROFILE& profile,
		f32_t elapsedTime);
};

NS_END
