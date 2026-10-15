#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapLoadScope.h"

NS_BEGIN(Engine)

class CModel;
class CShader;
struct MESH_SCREEN_LOD_DESC;
struct MAP_VISIBILITY_SETTINGS;

NS_END

NS_BEGIN(Client)

struct MAP_CAMERA_CULL_SNAPSHOT final
{
	uint64_t revision = {};
    uint64_t optimizationRevision = 0u;
    bool frustumEnabled = true;
	float4x4_t view = {};
	float4x4_t projection = {};
	float4_t worldPlanes[6] = {};
    // Derived once per camera revision, only for CPU geometry LOD. Rendering
    // and culling keep the original view. Zero scale means unsupported.
    float4x4_t lodView = {};
    f32_t lodViewScale = 0.f;
};

// The light clip volume has its own revision and never consumes camera visibility.
struct MAP_SHADOW_CULL_SNAPSHOT final
{
	uint64_t revision = {};
	float4_t worldPlanes[6] = {};
};

// VISIBILITY_ONLY preserves visibility and hysteresis; plane diagnostics are not guaranteed.
// Diagnostic policies always request the complete plane distances and rejecting plane.
enum class MAP_FRUSTUM_CULL_DETAIL : uint8_t { FULL, VISIBILITY_ONLY };

struct MAP_FRUSTUM_CULL_DECISION final
{
	bool_t wouldBeVisible = true;
	bool_t shouldRender = true;
	bool_t largeGeometry = false;
	f32_t baseRadius = {};
	f32_t margin = {};
	f32_t effectiveRadius = {};
	f32_t planeDistances[6] = {};
	f32_t planeTolerances[6] = {};
	int32_t rejectingPlane = -1;
};

struct MAP_SURFACE_BINDING_ROW final
{
	std::string assetId;
	std::string materialName;
	Engine::MODEL_SURFACE_FAMILY family = Engine::MODEL_SURFACE_FAMILY::LEGACY;
	uint32_t activeProgram = {};
	uint64_t lastSeenTickMs = {};
	Engine::MODEL_SURFACE_PARAMETERS surface;
	Engine::MODEL_BAKED_LIGHTING_INSTANCE lighting{};
};

// Instanced map shaders consume baked transforms from VTXMESHINSTANCE.
enum class MAP_MATERIAL_BINDING_MODE : uint8_t
{
	OBJECT,
	INSTANCED
};

class CMapAssetRenderUtils final
{
public:
	static bool_t Build_CameraCullSnapshot(
		const float4x4_t& view,
		const float4x4_t& projection,
		uint64_t revision,
		MAP_CAMERA_CULL_SNAPSHOT& outSnapshot,
		std::string* outFailureReason = nullptr);
	static bool_t Capture_CameraCullSnapshot(
		MAP_CAMERA_CULL_SNAPSHOT& outSnapshot,
		std::string* outFailureReason = nullptr);
	// Render-thread view of the same validated camera cache. Consume immediately;
	// a later camera capture may replace it. Failure returns null, never stale data.
	static const MAP_CAMERA_CULL_SNAPSHOT* Capture_CameraCullSnapshotView(
		std::string* outFailureReason = nullptr);
	static bool_t Build_ShadowCullSnapshot(
		const float4x4_t& view,
		const float4x4_t& projection,
		uint64_t revision,
		MAP_SHADOW_CULL_SNAPSHOT& outSnapshot);
	static bool_t Capture_ShadowCullSnapshot(
		MAP_SHADOW_CULL_SNAPSHOT& outSnapshot);
	// Invalid inputs retain the caster; only a separated valid sphere is rejected.
	static bool_t Intersects_ShadowCullSnapshot(
		const MAP_SHADOW_CULL_SNAPSHOT& snapshot,
		const float3_t& worldCenter,
		f32_t worldRadius);
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
		MAP_FRUSTUM_CULL_DECISION& outDecision,
		std::string* outFailureReason = nullptr,
		MAP_FRUSTUM_CULL_DETAIL detail = MAP_FRUSTUM_CULL_DETAIL::FULL);
	static void Begin_FrustumDiagnostics(
		const std::string& areaId,
		const MAP_FRUSTUM_CULLING_POLICY& policy);

    // Stage/Initialize가 실제 재질 전체를 분류한다. fallback/Deploy도 같은 정책을 소비한다.
    static MAP_SCENE_VISIBILITY_KIND Classify_SceneVisibility(const Engine::CModel& model,
        const MAP_ASSET_RENDER_PROFILE& profile, const std::string& groupId = {});
    static bool Get_SceneDistancePolicy(MAP_SCENE_VISIBILITY_KIND kind, float radius,
        const Engine::MAP_VISIBILITY_SETTINGS& settings, float& distance, float& maximumPixels);
    // 최종 카메라의 구 투영과 거리 조건을 모두 만족해야 숨긴다. 잘못된 입력은 표시를 보존한다.
    static bool Is_SceneDistanceHidden(const float3_t& center, float radius, const float4_t& cameraPosition,
        const MAP_CAMERA_CULL_SNAPSHOT& camera, const float2_t& viewport, float distance,
        float maximumPixels, float margin, bool wasHidden);
    // 일반 CModel::Render 소비자도 instanced 경로와 같은 1px 베른 오차 한계를 사용한다.
    static bool Build_ObjectScreenLod(const MAP_CAMERA_CULL_SNAPSHOT& camera,
        const float3_t& center, float radius, const float4x4_t& world,
        const float2_t& viewport, float maximumPixelError, Engine::MESH_SCREEN_LOD_DESC& result);

	static uint32_t Select_Pass(
		const MAP_ASSET_RENDER_PROFILE& profile,
		bool_t mirrored);

	// Existing alpha-tested shadow passes may also be invariant under time/camera.
	// The caller must reject mutable texture overrides and morph geometry.
	static bool_t Uses_StaticShadowInputs(
		const Engine::MODEL_SURFACE_PARAMETERS* surface,
		const MAP_ASSET_RENDER_PROFILE& profile,
		bool_t useSourceMaterials);

	// Only families whose source shadow cannot discard or displace vertices.
	static bool_t Uses_OpaqueShadowPass(
		const Engine::MODEL_SURFACE_PARAMETERS* surface,
		const MAP_ASSET_RENDER_PROFILE& profile,
		bool_t useSourceMaterials);

	static HRESULT Bind_ShadowMaterial(
		const shared_ptr<Engine::CModel>& model,
		const shared_ptr<Engine::CShader>& shader,
		uint32_t meshIndex,
		const MAP_ASSET_RENDER_PROFILE& profile,
		f32_t elapsedTime,
		const Engine::MODEL_SOURCE_FOLIAGE_WIND_INSTANCE* sourceWind = nullptr);

	static HRESULT Bind_Material(
		const shared_ptr<Engine::CModel>& model,
		const shared_ptr<Engine::CShader>& shader,
		uint32_t meshIndex,
		const MAP_ASSET_RENDER_PROFILE& profile,
		f32_t elapsedTime,
		const ComPtr<ID3D11ShaderResourceView>& diffuseOverride = nullptr,
		const std::string& diagnosticAssetId = {},
        const Engine::MODEL_BAKED_LIGHTING_INSTANCE* bakedLighting = nullptr,
        const float4_t* worldCullSphere = nullptr,
        MAP_MATERIAL_BINDING_MODE bindingMode = MAP_MATERIAL_BINDING_MODE::OBJECT,
        const Engine::MODEL_SOURCE_FOLIAGE_WIND_INSTANCE* sourceWind = nullptr);

	/* Scene and transient lights, their ambient and the scene fog for a
	source-character material drawn forward after scene lighting. */
	static HRESULT Bind_SourceCharacterForwardLights(
		const shared_ptr<Engine::CShader>& shader);

	// Preserve asset-by-asset diagnostics while the workbench holds its lease.
	static bool_t Is_SurfaceBindingCollectionActive();
	static std::vector<MAP_SURFACE_BINDING_ROW> Get_RecentSurfaceBindings();
};

NS_END
