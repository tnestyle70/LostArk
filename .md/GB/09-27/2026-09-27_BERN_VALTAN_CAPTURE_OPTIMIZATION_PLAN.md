# 베른·발탄 09-27 캡처 기반 비용 절감 계획

## G00. 실제 캡처와 수정 경계

`Client/Bin/ProfilerCaptures/*20260927*.json` 일곱 개의 각 120프레임을 사용한다.
마지막 발탄 파일은 사용자 설명에 따라 전투 캡처로 해석한다. CPU scope는 inclusive이며
GPU timestamp는 GPU 점유율이 아니다. 모든 Debug 파일은 D3D debug layer가 활성 상태다.
렌더링 품질·FXAA·SSAO·shadow·노출 저장값과 D3D 검증을 유지한다.

| 캡처 | 평균 frame interval ms | p95 ms | NonBlend CPU ms |
|---|---:|---:|---:|
| Release 베른성 | 15.094 | 17.852 | 4.857 |
| Release 베른성 근접 | 11.675 | 12.744 | 3.049 |
| Debug 베른성 일반 | 44.860 | 50.932 | 19.339 |
| Debug 베른성 근접 | 37.072 | 40.115 | 12.872 |
| Debug 베른성 컷신 | 50.810 | 78.872 | 23.716 |
| Debug 발탄 기본 | 32.951 | 36.304 | 19.765 |
| Debug 발탄 전투 | 31.056 | 35.522 | 14.933 |

## G01. MapAssetRenderUtils.cpp의 실제 재질 입력

`Bind_Material`에서 모든 draw마다 수행하던 `RENDER_ENVIRONMENT_STATE` 복사를
source indirect 재질일 때로 제한한다. 이 상태에는 cube COM 참조와 경로 문자열이 포함된다.
`BindForwardSceneLights`는 실제 count에 해당하는 초기화된 배열 prefix만 전송한다.
모든 shader consumer는 `index < g_SourceMapForwardLightCount`로 접근한다.
0개이면 count만 저장하고 이전 배열 tail은 읽지 않는다. 광원 목록·순서·예산은 유지한다.

## G02. Material.cpp의 surface texture 수명

`Bind_SurfaceTexture`의 임시 ComPtr 복사를 멤버 const 포인터로 바꾼다. CMaterial이 호출 동안
SRV를 소유하고 CShader가 성공한 바인딩을 유지한다. override·missing texture·shader reset
분기를 그대로 유지하여 draw별 불필요한 AddRef/Release만 제거한다.

## G03. Channel.cpp의 연속 애니메이션 키 검색

`FindRightKey`는 clone별 cursor에서 최대 네 구간을 앞으로 확인한 다음 같은 upper_bound로
돌아간다. 일반 30/60Hz 재생에서 매 키 경계마다 전체 track을 다시 이진 검색하지 않는다.
역재생·seek·wrap·큰 시간 이동·중복 timestamp는 기존 upper_bound와 같은 오른쪽 키를 반환한다.
보간식·time·animation 재생 빈도를 바꾸지 않는다.

## G04. 검증과 프로젝트 등록

새 C++ 파일 및 project/filter 항목은 없다. 기존 소스 인코딩과 CRLF를 유지한다.
키 검색의 동일 결과·비교 횟수, 광원 prefix의 count 변화·0·최대 용량을 검증한다.
Debug/Release 정본 Product 증분 Build, 변경 JSON/XML parse, diff-check를 수행한다.
사용자가 같은 조건으로 재캡처하기 전에는 FPS 개선이나 화면 PASS를 기록하지 않는다.

## 적용 코드 정본

### C:/Users/user/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp

```cpp
#include "MapAssetRenderUtils.h"
#include "SourceMovieMaterialPrograms.h"
#include "Engine_RenderTypes.h"

#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Presentation_Manager.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include <array>
#include <atomic>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <mutex>

namespace
{
    HRESULT BindSourceFoliageWind(const std::shared_ptr<Engine::CShader>& shader,
        const Engine::MODEL_SURFACE_PARAMETERS* surface, float time, bool skinned = false)
    {
        const uint32_t enabled = surface &&
            (surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED ||
             surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED ||
             (surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
              surface->sourceCharacter.program >= 1100u && surface->sourceCharacter.program <= 1166u)) &&
            surface->sourceFoliageWind ? 1u : 0u;
        // Animated scenery shares the BG material evaluator, but its skinned VS
        // has no foliage-wind input. Do not bind the static-only reset there.
        // An actual wind request still fails instead of silently losing motion.
        if (skinned) return enabled ? E_INVALIDARG : S_OK;
        if (FAILED(shader->Bind_RawValue("g_SourceFoliageWindEnabled", &enabled, sizeof(enabled)))) return E_FAIL;
        if (!enabled) return S_OK;
        if (!std::isfinite(time)) return E_INVALIDARG;
        if (FAILED(shader->Bind_RawValue("g_SourceFoliageWindProgram", &surface->sourceFoliageWindProgram,
            sizeof(surface->sourceFoliageWindProgram)))) return E_FAIL;
        float4_t player = surface->sourceFoliageWindPlayerPosition;
        if (surface->sourceFoliageWindProgram == 3u)
        {
            const auto character = Client::CAnimationTargetService::Resolve_SceneCharacter();
            float4x4_t root{};
            if (character && character->Try_Get_PresentationRootMatrix(&root))
            {
                if (!std::isfinite(root._41) || !std::isfinite(root._42) || !std::isfinite(root._43)) return E_INVALIDARG;
                player = float4_t(root._41 * 100.f, -root._43 * 100.f, root._42 * 100.f, 0.f);
            }
        }
        for (const auto& pair : { std::pair<const char*, const float4_t*>("g_SourceFoliageWindLocalCenter", &surface->sourceFoliageWindLocalCenter),
            { "g_SourceFoliageWindLocalBounds", &surface->sourceFoliageWindLocalBounds },
            { "g_SourceFoliageWindActorPosition", &surface->sourceFoliageWindActorPosition },
            { "g_SourceFoliageWindDirectionSpeed", &surface->sourceFoliageWindDirectionSpeed },
            { "g_SourceFoliageWindPlayerPosition", &player } })
            if (FAILED(shader->Bind_RawValue(pair.first, pair.second, sizeof(float4_t)))) return E_FAIL;
        if (FAILED(shader->Bind_RawValue("g_SourceFoliageWindScalars", surface->sourceFoliageWindScalars, sizeof(surface->sourceFoliageWindScalars))) ||
            FAILED(shader->Bind_RawValue("g_SourceFoliageWindTime", &time, sizeof(time)))) return E_FAIL;
        return S_OK;
    }

    HRESULT BindForwardSceneLights(const std::shared_ptr<Engine::CShader>& shader, bool bakedReceiver, const float4_t* worldCullSphere,
        bool withAmbient = false)
    {
        constexpr size_t capacity = 400u; // Scene 16 + existing transient budget 384.
        // Only [0,count) is consumed by the forward shaders. Each appended
        // record initializes every lane, so unused capacity needs no upload.
        std::array<float4_t, capacity> positions, directions, colors, cones, ambients;
        uint32_t count = 0u;
        bool mainDirectionalConsumed = false;
        const auto append = [&](const Engine::LIGHT_DESC& light, bool scene) -> HRESULT
        {
            const bool mainDirectional = scene && !mainDirectionalConsumed && light.eType == LIGHT::DIRECTIONAL;
            if (mainDirectional) mainDirectionalConsumed = true;
            if (bakedReceiver && (light.eReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER ||
                light.eReceiver == LIGHT_RECEIVER::UNBAKED)) return S_OK;
            if (light.vDiffuse.x == 0.f && light.vDiffuse.y == 0.f && light.vDiffuse.z == 0.f) return S_OK;
            if (worldCullSphere && light.eType != LIGHT::DIRECTIONAL)
            {
                const float x = light.vPosition.x - worldCullSphere->x;
                const float y = light.vPosition.y - worldCullSphere->y;
                const float z = light.vPosition.z - worldCullSphere->z;
                const float radius = light.fRange + worldCullSphere->w;
                if (radius <= 0.f || x*x + y*y + z*z > radius*radius) return S_OK;
            }
            if (count >= capacity) return E_BOUNDS;
            float type = 0.f;
            switch (light.eType)
            {
            case LIGHT::DIRECTIONAL: type = 1.f; break;
            case LIGHT::POINT: type = 2.f; break;
            case LIGHT::SPOT: type = 3.f; break;
            default: return E_INVALIDARG;
            }
            positions[count] = float4_t(light.vPosition.x, light.vPosition.y, light.vPosition.z, light.fRange);
            directions[count] = float4_t(light.vDirection.x, light.vDirection.y, light.vDirection.z, type);
            colors[count] = float4_t(light.vDiffuse.x, light.vDiffuse.y, light.vDiffuse.z, light.fFalloffExponent);
            cones[count] = float4_t(light.fSpotInnerCos, light.fSpotOuterCos, 0.f, light.staticShadowChannel != 0u ? float(light.staticShadowChannel) : mainDirectional ? 1.f : 0.f);
            ambients[count] = light.vAmbient;
            ++count;
            return S_OK;
        };
        for (const auto& light : Engine::CGameInstance::Get().Get_SceneLights())
            if (FAILED(append(light, true))) return E_FAIL;
        for (const auto& light : Engine::CPresentation_Manager::Get().Get_TransientLights())
            if (FAILED(append(light, false))) return E_FAIL;
        if (FAILED(shader->Bind_RawValue("g_SourceMapForwardLightCount", &count, sizeof(count)))) return E_FAIL;
        if (count == 0u) return S_OK;
        const uint32_t bytes = count * sizeof(float4_t);
        if (FAILED(shader->Bind_RawValue("g_SourceMapForwardLightPositionRange", positions.data(), bytes)) ||
            FAILED(shader->Bind_RawValue("g_SourceMapForwardLightDirectionType", directions.data(), bytes)) ||
            FAILED(shader->Bind_RawValue("g_SourceMapForwardLightColorExponent", colors.data(), bytes)) ||
            FAILED(shader->Bind_RawValue("g_SourceMapForwardLightConeShadow", cones.data(), bytes)) ||
            (withAmbient && FAILED(shader->Bind_RawValue("g_SourceMapForwardLightAmbient", ambients.data(), bytes)))) return E_FAIL;
        return S_OK;
    }

	// Only the Rendering Workbench consumes this diagnostic list. A short lease
	// lets its next draw populate the view without collecting while tools are shut.
	std::atomic<uint64_t> g_SurfaceBindingRequestedUntilMs{ 0u };
	std::mutex g_SurfaceBindingMutex;
	std::vector<Client::MAP_SURFACE_BINDING_ROW> g_SurfaceBindings;
	uint32_t g_SurfaceBindingLevelId = (std::numeric_limits<uint32_t>::max)();

	void RefreshSurfaceBindings(uint32_t levelId, uint64_t now)
	{
		if (g_SurfaceBindingLevelId != levelId)
		{
			g_SurfaceBindings.clear();
			g_SurfaceBindingLevelId = levelId;
		}
		g_SurfaceBindings.erase(std::remove_if(g_SurfaceBindings.begin(), g_SurfaceBindings.end(),
			[now](const Client::MAP_SURFACE_BINDING_ROW& row)
			{
				return now - row.lastSeenTickMs > 1000u;
			}), g_SurfaceBindings.end());
	}

	void RecordSurfaceBinding(const std::string& assetId, const std::string& materialName,
		const Engine::MODEL_SURFACE_PARAMETERS& surface, uint32_t program,
		const Engine::MODEL_BAKED_LIGHTING_INSTANCE& lighting)
	{
		const auto family = surface.family;
		const uint64_t now = GetTickCount64();
		if (now >= g_SurfaceBindingRequestedUntilMs.load(std::memory_order_relaxed))
			return;
		const uint32_t levelId = Engine::CGameInstance::Get().Get_CurrentLevelID();
		std::lock_guard<std::mutex> lock(g_SurfaceBindingMutex);
		RefreshSurfaceBindings(levelId, now);
		const auto existing = std::find_if(g_SurfaceBindings.begin(), g_SurfaceBindings.end(),
			[&](const Client::MAP_SURFACE_BINDING_ROW& row)
			{
				return row.assetId == assetId && row.materialName == materialName;
			});
		if (existing != g_SurfaceBindings.end())
		{
			existing->family = family;
			existing->activeProgram = program;
			existing->lastSeenTickMs = now;
			existing->surface = surface;
			existing->lighting = lighting;
			return;
		}
		if (g_SurfaceBindings.size() >= 32u)
		{
			g_SurfaceBindings.erase(std::min_element(g_SurfaceBindings.begin(), g_SurfaceBindings.end(),
				[](const Client::MAP_SURFACE_BINDING_ROW& left, const Client::MAP_SURFACE_BINDING_ROW& right)
				{
					return left.lastSeenTickMs < right.lastSeenTickMs;
				}));
		}
		g_SurfaceBindings.push_back({ assetId, materialName, family, program, now, surface, lighting });
	}

	std::mutex g_DiagnosticMutex;
	std::ofstream g_DiagnosticStream;
	std::string g_DiagnosticAreaId;
	uint64_t g_CameraMatrixRevision = {};
	bool_t g_HasCameraMatrices = false;
	float4x4_t g_LastView = {};
	float4x4_t g_LastProjection = {};
	Client::MAP_CAMERA_CULL_SNAPSHOT g_LastCameraSnapshot{};
	bool_t g_HasShadowMatrices = false;
	float4x4_t g_LastShadowView{};
	float4x4_t g_LastShadowProjection{};
	Client::MAP_SHADOW_CULL_SNAPSHOT g_LastShadowSnapshot{};
	uint64_t g_ValidatedPlaneRevision = {};
	bool_t g_HasValidatedPlanes = false;
	float4_t g_ValidatedPlanes[6]{};

	bool_t IsFiniteMatrix(const float4x4_t& matrix)
	{
		const f32_t* values = &matrix._11;
		for (uint32_t index = 0; index < 16u; ++index)
		{
			if (!std::isfinite(values[index]))
				return false;
		}
		return true;
	}

	bool_t ReportCullFailure(std::string* reason, const char* message)
	{
		if (nullptr != reason)
			*reason = message;
		return false;
	}

	bool_t IsInvertibleMatrix(const float4x4_t& matrix)
	{
		double values[4][4]{};
		const f32_t* source = &matrix._11;
		for (uint32_t row = 0; row < 4u; ++row)
			for (uint32_t column = 0; column < 4u; ++column)
				values[row][column] = source[row * 4u + column];
		for (uint32_t column = 0; column < 4u; ++column)
		{
			uint32_t pivot = column;
			for (uint32_t row = column + 1u; row < 4u; ++row)
				if (std::abs(values[row][column]) > std::abs(values[pivot][column]))
					pivot = row;
			if (!std::isfinite(values[pivot][column]) || 0.0 == values[pivot][column])
				return false;
			for (uint32_t entry = column; entry < 4u; ++entry)
				std::swap(values[column][entry], values[pivot][entry]);
			for (uint32_t row = column + 1u; row < 4u; ++row)
			{
				const double factor = values[row][column] / values[column][column];
				for (uint32_t entry = column + 1u; entry < 4u; ++entry)
					values[row][entry] -= factor * values[column][entry];
			}
		}
		return true;
	}

	void CacheValidatedPlanes(
		const Client::MAP_CAMERA_CULL_SNAPSHOT& snapshot)
	{
		g_ValidatedPlaneRevision = snapshot.revision;
		std::memcpy(g_ValidatedPlanes, snapshot.worldPlanes,
			sizeof(g_ValidatedPlanes));
		g_HasValidatedPlanes = true;
	}

	bool_t ValidateNormalizedPlanes(
		const Client::MAP_CAMERA_CULL_SNAPSHOT& snapshot,
		std::string* reason)
	{
		if (g_HasValidatedPlanes &&
			g_ValidatedPlaneRevision == snapshot.revision &&
			0 == std::memcmp(g_ValidatedPlanes, snapshot.worldPlanes,
				sizeof(g_ValidatedPlanes)))
		{
			return true;
		}

		for (const float4_t& plane : snapshot.worldPlanes)
		{
			const double normSquared =
				static_cast<double>(plane.x) * plane.x +
				static_cast<double>(plane.y) * plane.y +
				static_cast<double>(plane.z) * plane.z;
			if (!std::isfinite(plane.w) || !std::isfinite(normSquared) ||
				std::abs(normSquared - 1.0) >
					8.0 * std::numeric_limits<f32_t>::epsilon())
			{
				return ReportCullFailure(reason,
					"invalid normalized frustum plane");
			}
		}

		CacheValidatedPlanes(snapshot);
		return true;
	}

	std::filesystem::path GetDiagnosticPath()
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == length || length >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path() /
			L"Diagnostics" / L"BernFrustumCulling.log";
	}

	void RecordRejectedTransition(
		const Client::MAP_CAMERA_CULL_SNAPSHOT& snapshot,
		const std::string& assetId,
		const std::string& assetGroupId,
		const uint64_t placementId,
		const float3_t& worldCenter,
		const Client::MAP_FRUSTUM_CULL_DECISION& decision,
		const bool_t bypass)
	{
		std::scoped_lock lock(g_DiagnosticMutex);
		if (!g_DiagnosticStream)
			return;
		g_DiagnosticStream << std::setprecision(9)
			<< "event=VISIBLE_TO_REJECTED"
			<< " area=" << std::quoted(g_DiagnosticAreaId)
			<< " cameraRevision=" << snapshot.revision
			<< " bypass=" << (bypass ? 1 : 0)
			<< " assetId=" << std::quoted(assetId)
			<< " groupId=" << std::quoted(assetGroupId)
			<< " placementId=" << placementId
			<< " center=(" << worldCenter.x << ',' << worldCenter.y << ','
			<< worldCenter.z << ')'
			<< " baseRadius=" << decision.baseRadius
			<< " margin=" << decision.margin
			<< " effectiveRadius=" << decision.effectiveRadius
			<< " largeGeometry=" << (decision.largeGeometry ? 1 : 0)
			<< " planeDistances=[";
		for (uint32_t index = 0; index < 6u; ++index)
		{
			if (0u != index)
				g_DiagnosticStream << ',';
			g_DiagnosticStream << decision.planeDistances[index];
		}
		g_DiagnosticStream << ']';
		const matrix_t viewProjection =
			XMLoadFloat4x4(&snapshot.view) *
			XMLoadFloat4x4(&snapshot.projection);
		const vector_t clip = XMVector3Transform(
			XMVectorSetW(XMLoadFloat3(&worldCenter), 1.f), viewProjection);
		const f32_t clipW = XMVectorGetW(clip);
		if (std::isfinite(clipW) && std::abs(clipW) > 0.000001f)
		{
			const f32_t ndcX = XMVectorGetX(clip) / clipW;
			const f32_t ndcY = XMVectorGetY(clip) / clipW;
			const f32_t ndcZ = XMVectorGetZ(clip) / clipW;
			const f32_t ndcRadiusX = decision.baseRadius *
				snapshot.projection._11 / std::abs(clipW);
			const f32_t ndcRadiusY = decision.baseRadius *
				snapshot.projection._22 / std::abs(clipW);
			const bool_t onScreen = clipW > 0.f &&
				std::abs(ndcX) <= 1.f + ndcRadiusX &&
				std::abs(ndcY) <= 1.f + ndcRadiusY &&
				ndcZ >= 0.f && ndcZ <= 1.f;
			g_DiagnosticStream
				<< " ndc=(" << ndcX << ',' << ndcY << ',' << ndcZ << ')'
				<< " clipW=" << clipW
				<< " ndcRadius=(" << ndcRadiusX << ',' << ndcRadiusY << ')'
				<< " onScreen=" << (onScreen ? 1 : 0);
		}
		else
		{
			g_DiagnosticStream << " ndc=none clipW=" << clipW
				<< " ndcRadius=none onScreen=0";
		}
		g_DiagnosticStream << '\n';
		g_DiagnosticStream.flush();
	}
}

bool_t CMapAssetRenderUtils::Build_CameraCullSnapshot(
	const float4x4_t& view,
	const float4x4_t& projection,
	const uint64_t revision,
	MAP_CAMERA_CULL_SNAPSHOT& outSnapshot,
	std::string* outFailureReason)
{
	if (nullptr != outFailureReason)
		outFailureReason->clear();
	if (0u == revision)
		return ReportCullFailure(outFailureReason, "zero camera revision");
	if (!IsFiniteMatrix(view) || !IsFiniteMatrix(projection))
		return ReportCullFailure(outFailureReason, "non-finite camera matrix");
	if (!IsInvertibleMatrix(view) || !IsInvertibleMatrix(projection))
		return ReportCullFailure(outFailureReason, "singular camera matrix");

	// Extract the shader's clip half-spaces directly. A far corner plus two
	// nearby near corners loses their small edge in a float cross product.
	double clip[4][4]{};
	const f32_t* viewValues = &view._11;
	const f32_t* projectionValues = &projection._11;
	for (uint32_t row = 0; row < 4u; ++row)
	{
		for (uint32_t column = 0; column < 4u; ++column)
		{
			for (uint32_t inner = 0; inner < 4u; ++inner)
			{
				clip[row][column] +=
					static_cast<double>(viewValues[row * 4u + inner]) *
					projectionValues[inner * 4u + column];
			}
		}
	}

	MAP_CAMERA_CULL_SNAPSHOT candidate{};
	candidate.revision = revision;
	candidate.view = view;
	candidate.projection = projection;
	for (uint32_t planeIndex = 0; planeIndex < 6u; ++planeIndex)
	{
		double plane[4]{};
		for (uint32_t row = 0; row < 4u; ++row)
		{
			// Outward normals: right, left, top, bottom, far, near.
			switch (planeIndex)
			{
			case 0u: plane[row] = clip[row][0] - clip[row][3]; break;
			case 1u: plane[row] = -clip[row][0] - clip[row][3]; break;
			case 2u: plane[row] = clip[row][1] - clip[row][3]; break;
			case 3u: plane[row] = -clip[row][1] - clip[row][3]; break;
			case 4u: plane[row] = clip[row][2] - clip[row][3]; break;
			case 5u: plane[row] = -clip[row][2]; break;
			}
		}
		const double length = std::hypot(std::hypot(plane[0], plane[1]), plane[2]);
		if (!std::isfinite(length) || 0.0 == length)
			return ReportCullFailure(outFailureReason, "degenerate clip plane");
		f32_t* stored = &candidate.worldPlanes[planeIndex].x;
		for (uint32_t component = 0; component < 4u; ++component)
		{
			const double normalized = plane[component] / length;
			if (!std::isfinite(normalized) ||
				std::abs(normalized) > (std::numeric_limits<f32_t>::max)())
			{
				return ReportCullFailure(outFailureReason, "non-finite normalized clip plane");
			}
			stored[component] = static_cast<f32_t>(normalized);
		}
	}
	CacheValidatedPlanes(candidate);
	outSnapshot = candidate;
	return true;
}

bool_t CMapAssetRenderUtils::Capture_CameraCullSnapshot(
	MAP_CAMERA_CULL_SNAPSHOT& outSnapshot,
	std::string* outFailureReason)
{
	const auto* snapshot = Capture_CameraCullSnapshotView(outFailureReason);
	if (!snapshot)
		return false;
	outSnapshot = *snapshot;
	return true;
}

const MAP_CAMERA_CULL_SNAPSHOT* CMapAssetRenderUtils::Capture_CameraCullSnapshotView(
	std::string* outFailureReason)
{
	if (nullptr != outFailureReason)
		outFailureReason->clear();
	const float4x4_t* view = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
	const float4x4_t* projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
	if (nullptr == view || nullptr == projection)
	{
		ReportCullFailure(outFailureReason, "camera matrix unavailable");
		return nullptr;
	}

	const float4x4_t& stagedView = *view;
	const float4x4_t& stagedProjection = *projection;
	if (g_HasCameraMatrices &&
		0 == std::memcmp(&g_LastView, &stagedView, sizeof(float4x4_t)) &&
		0 == std::memcmp(&g_LastProjection, &stagedProjection, sizeof(float4x4_t)))
	{
		return &g_LastCameraSnapshot;
	}
	const uint64_t nextRevision =
		(std::numeric_limits<uint64_t>::max)() == g_CameraMatrixRevision ?
		1u : g_CameraMatrixRevision + 1u;
	MAP_CAMERA_CULL_SNAPSHOT candidate{};
	if (!Build_CameraCullSnapshot(stagedView, stagedProjection, nextRevision,
		candidate, outFailureReason))
	{
		return nullptr;
	}
	g_LastView = candidate.view;
	g_LastProjection = candidate.projection;
	g_CameraMatrixRevision = candidate.revision;
	g_LastCameraSnapshot = candidate;
	g_HasCameraMatrices = true;
	return &g_LastCameraSnapshot;
}

bool_t CMapAssetRenderUtils::Build_ShadowCullSnapshot(
	const float4x4_t& view, const float4x4_t& projection,
	const uint64_t revision, MAP_SHADOW_CULL_SNAPSHOT& outSnapshot)
{
	MAP_CAMERA_CULL_SNAPSHOT planes{};
	if (!Build_CameraCullSnapshot(view, projection, revision, planes))
		return false;
	MAP_SHADOW_CULL_SNAPSHOT candidate{};
	candidate.revision = revision;
	std::memcpy(candidate.worldPlanes, planes.worldPlanes, sizeof(candidate.worldPlanes));
	outSnapshot = candidate;
	return true;
}

bool_t CMapAssetRenderUtils::Capture_ShadowCullSnapshot(
	MAP_SHADOW_CULL_SNAPSHOT& outSnapshot)
{
	const auto* view = CGameInstance::Get().Get_ShadowLightTransform(D3DTS::VIEW);
	const auto* projection = CGameInstance::Get().Get_ShadowLightTransform(D3DTS::PROJ);
	if (!view || !projection)
		return false;
	const float4x4_t stagedView = *view;
	const float4x4_t stagedProjection = *projection;
	if (g_HasShadowMatrices &&
		0 == std::memcmp(&g_LastShadowView, &stagedView, sizeof(stagedView)) &&
		0 == std::memcmp(&g_LastShadowProjection, &stagedProjection, sizeof(stagedProjection)))
	{
		outSnapshot = g_LastShadowSnapshot;
		return true;
	}
	const uint64_t revision = g_LastShadowSnapshot.revision ==
		(std::numeric_limits<uint64_t>::max)() ? 1u : g_LastShadowSnapshot.revision + 1u;
	MAP_SHADOW_CULL_SNAPSHOT candidate{};
	if (!Build_ShadowCullSnapshot(stagedView, stagedProjection, revision, candidate))
		return false;
	g_LastShadowView = stagedView;
	g_LastShadowProjection = stagedProjection;
	g_LastShadowSnapshot = candidate;
	g_HasShadowMatrices = true;
	outSnapshot = candidate;
	return true;
}

bool_t CMapAssetRenderUtils::Intersects_ShadowCullSnapshot(
	const MAP_SHADOW_CULL_SNAPSHOT& snapshot,
	const float3_t& worldCenter, const f32_t worldRadius)
{
	if (snapshot.revision == 0u || !std::isfinite(worldCenter.x) ||
		!std::isfinite(worldCenter.y) || !std::isfinite(worldCenter.z) ||
		!std::isfinite(worldRadius) || worldRadius <= 0.f)
		return true;
	// Validate every plane before rejecting; a corrupt later plane cannot hide a caster.
	for (const auto& plane : snapshot.worldPlanes)
	{
		const double normSquared = static_cast<double>(plane.x) * plane.x +
			static_cast<double>(plane.y) * plane.y + static_cast<double>(plane.z) * plane.z;
		if (!std::isfinite(plane.w) || !std::isfinite(normSquared) ||
			std::abs(normSquared - 1.0) > 8.0 * std::numeric_limits<f32_t>::epsilon())
			return true;
	}
	// Placement bounds already include transform inflation; retain another 5 cm
	// and a magnitude-scaled float tolerance at all six shadow clip boundaries.
	const double radius = static_cast<double>(worldRadius) + 0.05;
	for (const auto& plane : snapshot.worldPlanes)
	{
		const double x = static_cast<double>(plane.x) * worldCenter.x;
		const double y = static_cast<double>(plane.y) * worldCenter.y;
		const double z = static_cast<double>(plane.z) * worldCenter.z;
		const double magnitude = std::abs(x) + std::abs(y) + std::abs(z) +
			std::abs(static_cast<double>(plane.w)) + radius;
		const double tolerance = 8.0 * std::numeric_limits<f32_t>::epsilon() *
			(std::max)(1.0, magnitude);
		if (x + y + z + plane.w > radius + tolerance)
			return false;
	}
	return true;
}

HRESULT CMapAssetRenderUtils::Bind_CameraCullSnapshot(
	const shared_ptr<Engine::CShader>& shader,
	const MAP_CAMERA_CULL_SNAPSHOT& snapshot)
{
	if (nullptr == shader || 0u == snapshot.revision)
		return E_INVALIDARG;
	const HRESULT viewResult = shader->Bind_Matrix("g_ViewMatrix", &snapshot.view);
	return FAILED(viewResult) ? viewResult :
		shader->Bind_Matrix("g_ProjMatrix", &snapshot.projection);
}

bool_t CMapAssetRenderUtils::Evaluate_FrustumVisibility(
	const MAP_FRUSTUM_CULLING_POLICY& policy,
	const MAP_CAMERA_CULL_SNAPSHOT& snapshot,
	const std::string& assetId,
	const std::string& assetGroupId,
	const uint64_t placementId,
	const float3_t& worldCenter,
	const f32_t worldRadius,
	MAP_FRUSTUM_RUNTIME_STATE& state,
	MAP_FRUSTUM_CULL_DECISION& outDecision,
	std::string* outFailureReason)
{
	if (nullptr != outFailureReason)
		outFailureReason->clear();
	if (0u == snapshot.revision || !std::isfinite(worldCenter.x) ||
		!std::isfinite(worldCenter.y) || !std::isfinite(worldCenter.z) ||
		!std::isfinite(worldRadius) || worldRadius <= 0.f)
	{
		return ReportCullFailure(outFailureReason, "invalid camera revision or world sphere");
	}
	const f32_t policyValues[] = { policy.baseMargin,
		policy.largeObjectRadiusThreshold, policy.largeObjectAbsoluteMargin,
		policy.largeObjectRelativeMargin };
	for (const f32_t value : policyValues)
	{
		if (!std::isfinite(value) || value < 0.f)
			return ReportCullFailure(outFailureReason, "invalid frustum margin policy");
	}
	if (!ValidateNormalizedPlanes(snapshot, outFailureReason))
		return false;

	MAP_FRUSTUM_CULL_DECISION candidate{};
	candidate.baseRadius = worldRadius;
	candidate.largeGeometry = "landscape" == assetGroupId ||
		(policy.largeObjectRadiusThreshold > 0.f &&
		 worldRadius >= policy.largeObjectRadiusThreshold);
	double margin = policy.baseMargin;
	if (candidate.largeGeometry)
	{
		margin = (std::max)({ margin,
			static_cast<double>(policy.largeObjectAbsoluteMargin),
			static_cast<double>(worldRadius) * policy.largeObjectRelativeMargin });
	}
	const double effectiveRadius = static_cast<double>(worldRadius) + margin;
	if (!std::isfinite(effectiveRadius) ||
		effectiveRadius > (std::numeric_limits<f32_t>::max)())
	{
		return ReportCullFailure(outFailureReason, "effective sphere radius overflow");
	}
	candidate.margin = static_cast<f32_t>(margin);
	candidate.effectiveRadius = static_cast<f32_t>(effectiveRadius);
	double largestSeparation = 0.0;
	const vector_t center = XMLoadFloat3(&worldCenter);
	for (uint32_t index = 0; index < 6u; ++index)
	{
		const float4_t& plane = snapshot.worldPlanes[index];
		const double x = static_cast<double>(plane.x) * worldCenter.x;
		const double y = static_cast<double>(plane.y) * worldCenter.y;
		const double z = static_cast<double>(plane.z) * worldCenter.z;
		const f32_t planeDistance = XMVectorGetX(XMPlaneDotCoord(
			XMLoadFloat4(&plane), center));
		const double distance = planeDistance;
		const double magnitude = std::abs(x) + std::abs(y) + std::abs(z) +
			std::abs(static_cast<double>(plane.w)) + effectiveRadius;
		const double tolerance = 8.0 * std::numeric_limits<f32_t>::epsilon() *
			(std::max)(1.0, magnitude);
		if (!std::isfinite(distance) ||
			std::abs(distance) > (std::numeric_limits<f32_t>::max)())
		{
			return ReportCullFailure(outFailureReason, "frustum distance overflow");
		}
		candidate.planeDistances[index] = static_cast<f32_t>(distance);
		candidate.planeTolerances[index] = static_cast<f32_t>(tolerance);
		const double separation = distance - effectiveRadius - tolerance;
		if (separation > 0.0)
		{
			candidate.wouldBeVisible = false;
			if (separation > largestSeparation)
			{
				largestSeparation = separation;
				candidate.rejectingPlane = static_cast<int32_t>(index);
			}
		}
	}

	MAP_FRUSTUM_RUNTIME_STATE nextState = state;
	nextState.initialized = true;
	nextState.lastFrustumVisible = candidate.wouldBeVisible;
	if (candidate.wouldBeVisible)
		nextState.rejectGraceFrames = policy.rejectHysteresisFrames;
	candidate.shouldRender = policy.bypass || candidate.wouldBeVisible;
	if (!candidate.shouldRender && nextState.rejectGraceFrames > 0u)
	{
		candidate.shouldRender = true;
		--nextState.rejectGraceFrames;
	}
	if (state.initialized && state.lastFrustumVisible &&
		!candidate.wouldBeVisible && policy.diagnostics)
	{
		RecordRejectedTransition(snapshot, assetId, assetGroupId,
			placementId, worldCenter, candidate, policy.bypass);
	}
	state = nextState;
	outDecision = candidate;
	return true;
}

void CMapAssetRenderUtils::Begin_FrustumDiagnostics(
	const std::string& areaId,
	const MAP_FRUSTUM_CULLING_POLICY& policy)
{
	if (!policy.diagnostics)
		return;

	const std::filesystem::path path = GetDiagnosticPath();
	std::error_code error;
	if (path.empty() ||
		(!std::filesystem::create_directories(path.parent_path(), error) && error))
	{
		OutputDebugStringA("[BernFrustum] Diagnostic directory unavailable.\n");
		return;
	}

	std::scoped_lock lock(g_DiagnosticMutex);
	g_DiagnosticStream.close();
	g_DiagnosticStream.clear();
	g_DiagnosticStream.open(path, std::ios::binary | std::ios::trunc);
	if (!g_DiagnosticStream)
	{
		OutputDebugStringA("[BernFrustum] Diagnostic log unavailable.\n");
		return;
	}
	g_DiagnosticAreaId = areaId;
	g_DiagnosticStream
		<< "LOSTARK_BERN_FRUSTUM_DIAGNOSTICS 1\n"
		<< "mode=" << (policy.bypass ? "BYPASS_AND_LOG" : "CULL_AND_LOG")
		<< " baseMargin=" << policy.baseMargin
		<< " largeRadiusThreshold=" << policy.largeObjectRadiusThreshold
		<< " largeAbsoluteMargin=" << policy.largeObjectAbsoluteMargin
		<< " largeRelativeMargin=" << policy.largeObjectRelativeMargin
		<< " rejectHysteresisFrames=" << policy.rejectHysteresisFrames
		<< "\n";
	g_DiagnosticStream.flush();
	OutputDebugStringA(("[BernFrustum] Diagnostics ready: " +
		path.string() + "\n").c_str());
}

uint32_t CMapAssetRenderUtils::Select_Pass(const MAP_ASSET_RENDER_PROFILE& profile, 
	bool_t mirrored)
{
	//map asset의 profile의 cullmode를 사용해서 culloffset 변수와 modeoffset 변수를 설정해준다
	MAP_ASSET_CULL_MODE cullMode = profile.cullMode;

	if (mirrored && MAP_ASSET_CULL_MODE::TWO_SIDED != cullMode)
	{
		cullMode = MAP_ASSET_CULL_MODE::CULL_BACK == cullMode ?
			MAP_ASSET_CULL_MODE::CULL_FRONT :
			MAP_ASSET_CULL_MODE::CULL_BACK;
	}
	//culloffset 설정
	const uint32_t  cullOfset =
		MAP_ASSET_CULL_MODE::CULL_BACK == cullMode ? 0u :
		MAP_ASSET_CULL_MODE::CULL_FRONT == cullMode ? 1u : 2u;
	//deffered translucent background
	/* Water sits after the shadow passes so adding it leaves every existing
	   pass index where it was; the three cull variants keep the +0/+1/+2 rule
	   even though the source water material is never two sided. */
	const uint32_t modeOffset =
		MAP_ASSET_RENDER_MODE::DEFERRED == profile.renderMode ? 0u :
		MAP_ASSET_RENDER_MODE::TRANSLUCENT == profile.renderMode ? 3u :
		MAP_ASSET_RENDER_MODE::BACKGROUND == profile.renderMode ? 6u :
		MAP_ASSET_RENDER_MODE::WATER == profile.renderMode ? 15u : 9u;
	
	return modeOffset + cullOfset;
}

HRESULT Client::CMapAssetRenderUtils::Bind_SourceCharacterForwardLights(
	const shared_ptr<Engine::CShader>& shader)
{
	if (nullptr == shader)
		return E_INVALIDARG;
	if (FAILED(BindForwardSceneLights(shader, false, nullptr, true)))
		return E_FAIL;
	return CGameInstance::Get().Bind_HeightFog(shader.get());
}

std::vector<Client::MAP_SURFACE_BINDING_ROW> Client::CMapAssetRenderUtils::Get_RecentSurfaceBindings()
{
	const uint64_t now = GetTickCount64();
	g_SurfaceBindingRequestedUntilMs.store(now + 1000u, std::memory_order_relaxed);
	const uint32_t levelId = CGameInstance::Get().Get_CurrentLevelID();
	std::lock_guard<std::mutex> lock(g_SurfaceBindingMutex);
	RefreshSurfaceBindings(levelId, now);
	return g_SurfaceBindings;
}

bool_t Client::CMapAssetRenderUtils::Uses_OpaqueShadowPass(
	const Engine::MODEL_SURFACE_PARAMETERS* surface,
	const MAP_ASSET_RENDER_PROFILE& profile,
	const bool_t useSourceMaterials)
{
	if (!surface || !useSourceMaterials ||
		profile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED ||
		!std::isfinite(profile.opacity) || profile.opacity < 1.f)
		return false;
	switch (surface->family)
	{
	case Engine::MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE:
	case Engine::MODEL_SURFACE_FAMILY::PBR_OPAQUE:
		return !surface->pbrAlphaMasked;
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE:
		return true;
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE:
		return (surface->sourceOverlayFlags & 64u) == 0u;
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED:
		// BG parallax changes UV only. Its sole shadow discard is mask bit 64.
		return (surface->sourceBgFlags & 64u) == 0u;
	default:
		// Foliage, native character/map and other families retain their source VS/PS.
		return false;
	}
}

bool_t Client::CMapAssetRenderUtils::Uses_StaticShadowInputs(
	const Engine::MODEL_SURFACE_PARAMETERS* surface,
	const MAP_ASSET_RENDER_PROFILE& profile,
	const bool_t useSourceMaterials)
{
	if (Uses_OpaqueShadowPass(surface, profile, useSourceMaterials))
		return true;
	if (!surface || !useSourceMaterials ||
		profile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED ||
		!std::isfinite(profile.opacity) || profile.opacity < 1.f)
		return false;

	// Preserve the current alpha test and rasterizer pass. Only its inputs decide
	// whether the already rendered depth can survive another frame.
	switch (surface->family)
	{
	case Engine::MODEL_SURFACE_FAMILY::LEGACY:
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED:
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED:
		return !surface->sourceFoliageWind && profile.uvSpeed.x == 0.f && profile.uvSpeed.y == 0.f;
	case Engine::MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION:
	case Engine::MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION:
	case Engine::MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE:
	case Engine::MODEL_SURFACE_FAMILY::PBR_OPAQUE:
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE:
		// Source alpha uses raw UV, fixed tiling or the fixed overlay transform.
		return true;
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED:
		// Parallax moves the masked sample with camera position. Panning moves it
		// with elapsed time. Neither input is part of the static light-depth key.
		return (surface->sourceBgFlags & 2u) == 0u &&
			surface->sourceBgPanning.x == 0.f && surface->sourceBgPanning.y == 0.f;
	default:
		return false;
	}
}

HRESULT Client::CMapAssetRenderUtils::Bind_ShadowMaterial(
	const shared_ptr<Engine::CModel>& model,
	const shared_ptr<Engine::CShader>& shader,
	uint32_t meshIndex,
	const MAP_ASSET_RENDER_PROFILE& profile,
	f32_t elapsedTime)
{
	if (nullptr == model || nullptr == shader ||
		meshIndex >= model->Get_NumMeshes())
		return E_INVALIDARG;

	const auto* surface = model->Get_MaterialSurface(meshIndex);
	if (surface)
	{
		switch (surface->family)
		{
		case Engine::MODEL_SURFACE_FAMILY::LEGACY:
		case Engine::MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION:
		case Engine::MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION:
		case Engine::MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE:
		case Engine::MODEL_SURFACE_FAMILY::PBR_OPAQUE:
		case Engine::MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE:
			break;
		default:
			// Native, layered and foliage families own additional shadow inputs.
			return Bind_Material(model, shader, meshIndex, profile, elapsedTime);
		}
	}

	const uint32_t noProgram = 0u;
	const uint32_t pbrMasked = surface && surface->pbrAlphaMasked ? 1u : 0u;
	if (FAILED(shader->Bind_RawValue("g_SurfacePBRMasked", &pbrMasked, sizeof(pbrMasked))) ||
		(pbrMasked && FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &surface->uvTiling, sizeof(surface->uvTiling)))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_DiffuseMirrorU", &noProgram, sizeof(noProgram))))
		return E_FAIL;
	// A preceding character draw may share the Effect. Map instances omit these
	// optional variables, while the binary shadow pass reads the character program.
	shader->Bind_RawValue("g_SourceCharacterProgram", &noProgram, sizeof(noProgram));
	shader->Bind_RawValue("g_SourceCharacterRow", &noProgram, sizeof(noProgram));

	const float2_t uvOffset(profile.uvSpeed.x * elapsedTime, profile.uvSpeed.y * elapsedTime);
	if (FAILED(model->Bind_Material(shader, "g_DiffuseTexture", meshIndex, aiTextureType_DIFFUSE)) ||
		FAILED(shader->Bind_RawValue("g_UVScale", &profile.uvScale, sizeof(profile.uvScale))) ||
		FAILED(shader->Bind_RawValue("g_UVOffset", &uvOffset, sizeof(uvOffset))) ||
		FAILED(shader->Bind_RawValue("g_ColorTint", &profile.colorTint, sizeof(profile.colorTint))) ||
		FAILED(shader->Bind_RawValue("g_Opacity", &profile.opacity, sizeof(profile.opacity))))
		return E_FAIL;

	const auto settings = CGameInstance::Get().Get_MaterialRenderSettings();
	const uint32_t program = surface &&
		surface->family != Engine::MODEL_SURFACE_FAMILY::LEGACY &&
		profile.renderMode == MAP_ASSET_RENDER_MODE::DEFERRED && settings.bUseSourceMaterials ?
		static_cast<uint32_t>(surface->family) : 0u;
	// Source diffuse can differ from the legacy texture. Preserve its alpha even
	// though the remaining source lighting textures are unused by these shadows.
	if (program != 0u &&
		FAILED(model->Bind_SurfaceTexture(shader, "g_DiffuseTexture", meshIndex, aiTextureType_DIFFUSE)))
		return E_FAIL;
	if (FAILED(BindSourceFoliageWind(shader, surface, elapsedTime, model->Is_Skinned()))) return E_FAIL;
	return shader->Bind_RawValue("g_SurfaceProgram", &program, sizeof(program));
}

HRESULT Client::CMapAssetRenderUtils::Bind_Material(
	const shared_ptr<Engine::CModel>& model,
	const shared_ptr<Engine::CShader>& shader,
	uint32_t meshIndex,
	const MAP_ASSET_RENDER_PROFILE& profile,
	f32_t elapsedTime, const ComPtr<ID3D11ShaderResourceView>& diffuseOverride,
	const std::string& diagnosticAssetId,
    const Engine::MODEL_BAKED_LIGHTING_INSTANCE* bakedLighting, const float4_t* worldCullSphere,
    MAP_MATERIAL_BINDING_MODE bindingMode)
{
	if (nullptr == model ||
		nullptr == shader ||
		meshIndex >= model->Get_NumMeshes())
	{
		return E_INVALIDARG;
	}

	// Shared shader state is reset even for legacy and diffuse-override draws.
	const uint32_t noSurfaceEmissive = 0u;
	if (FAILED(shader->Bind_RawValue("g_HasSurfaceEmissive",
		&noSurfaceEmissive, sizeof(noSurfaceEmissive))) ||
		FAILED(shader->Bind_RawValue("g_DiffuseMirrorU",
			&noSurfaceEmissive, sizeof(noSurfaceEmissive))))
		return E_FAIL;

	// Static world objects share this Effect with character equipment. An explicit
	// diffuse SRV skips CMaterial's reset, so a preceding source character draw
	// must not select its program (or leave its dye/hit tint) for this mesh.
	// The instanced map shader omits these optional character-only variables.
	if (bindingMode == MAP_MATERIAL_BINDING_MODE::OBJECT)
	{
		const float4_t identityEmissive(1.f, 1.f, 1.f, 1.f);
		for (const char_t* name : { "g_SourceCharacterProgram", "g_SourceCharacterRow",
			"g_HasDyeMask", "g_HasFullSurfaceEmissiveOverride" })
			shader->Bind_RawValue(name, &noSurfaceEmissive, sizeof(noSurfaceEmissive));
		shader->Bind_RawValue("g_EmissiveColor", &identityEmissive, sizeof(identityEmissive));
	}

	const auto* nativeSurface = model->Get_MaterialSurface(meshIndex);
    const uint32_t sourceBgUnlit = nativeSurface && nativeSurface->sourceBgUnlit &&
        nativeSurface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_SourceBgUnlit", &sourceBgUnlit, sizeof(sourceBgUnlit)))) return E_FAIL;
	const auto settings = CGameInstance::Get().Get_MaterialRenderSettings();
	const bool sourceBg = nativeSurface &&
		nativeSurface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
		profile.renderMode == MAP_ASSET_RENDER_MODE::DEFERRED &&
		!diffuseOverride && settings.bUseSourceMaterials;
	// Source BG uses its raw UV and native textures/constants. Keep the diffuse
	// binder's mirror/reset contract, but do not prepare unused legacy inputs.
	if (sourceBg)
	{
		// Non-instanced opaque props still consume opacity for presentation dither.
		if (FAILED(model->Bind_Material(shader, "g_DiffuseTexture", meshIndex, aiTextureType_DIFFUSE)) ||
			FAILED(shader->Bind_RawValue("g_Opacity", &profile.opacity, sizeof(profile.opacity))))
			return E_FAIL;
	}
	else
	{
	const uint32_t hasNormalTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_NORMALS) ? 1u : 0u;

	const uint32_t hasEmissiveTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_EMISSIVE) ? 1u : 0u;

	const uint32_t hasSpecularTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_SPECULAR) ? 1u : 0u;

	const uint32_t hasOpacityTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_OPACITY) ? 1u : 0u;

    const bool constantSource = nativeSurface &&
        nativeSurface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
        (nativeSurface->sourceCharacter.program == 64u || nativeSurface->sourceCharacter.program == 65u);
	const float2_t uvOffset(
		profile.uvSpeed.x * elapsedTime,
		profile.uvSpeed.y * elapsedTime);

	if ((!constantSource && FAILED(diffuseOverride ? shader->Bind_Texture("g_DiffuseTexture", diffuseOverride) : model->Bind_Material(
		shader,
		"g_DiffuseTexture",
		meshIndex,
		aiTextureType_DIFFUSE))) ||

		FAILED(shader->Bind_RawValue(
			"g_UVScale",
			&profile.uvScale,
			sizeof(profile.uvScale))) ||

		FAILED(shader->Bind_RawValue(
			"g_UVOffset",
			&uvOffset,
			sizeof(uvOffset))) ||

		FAILED(shader->Bind_RawValue(
			"g_Opacity",
			&profile.opacity,
			sizeof(profile.opacity))) ||

		FAILED(shader->Bind_RawValue(
			"g_OpacityPower",
			&profile.opacityPower,
			sizeof(profile.opacityPower))) ||

		FAILED(shader->Bind_RawValue(
			"g_ColorTint",
			&profile.colorTint,
			sizeof(profile.colorTint))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasNormalTexture",
			&hasNormalTexture,
			sizeof(hasNormalTexture))) ||

		(0 != hasNormalTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_NormalTexture",
				meshIndex,
				aiTextureType_NORMALS))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasEmissiveTexture",
			&hasEmissiveTexture,
			sizeof(hasEmissiveTexture))) ||

		FAILED(shader->Bind_RawValue(
			"g_EmissiveIntensity",
			&profile.emissiveIntensity,
			sizeof(profile.emissiveIntensity))) ||

		(0 != hasEmissiveTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_EmissiveTexture",
				meshIndex,
				aiTextureType_EMISSIVE))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasSpecularTexture",
			&hasSpecularTexture,
			sizeof(hasSpecularTexture))) ||

		FAILED(shader->Bind_RawValue(
			"g_SpecularIntensity",
			&profile.specularIntensity,
			sizeof(profile.specularIntensity))) ||

		FAILED(shader->Bind_RawValue(
			"g_SpecularPower",
			&profile.specularPower,
			sizeof(profile.specularPower))) ||

		FAILED(shader->Bind_RawValue(
			"g_TriplanarHeightScale",
			&profile.triplanarHeightScale,
			sizeof(profile.triplanarHeightScale))) ||

		(0 != hasSpecularTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_SpecularTexture",
				meshIndex,
				aiTextureType_SPECULAR))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasOpacityTexture",
			&hasOpacityTexture,
			sizeof(hasOpacityTexture))) ||

		(0 != hasOpacityTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_OpacityTexture",
				meshIndex,
				aiTextureType_OPACITY))))
	{
		return E_FAIL;
	}

	}

    // A static World Object may own the same native material contract as an actor.
    // Submit its real CMaterial into the shared direct-light pass after legacy resets.
    const Engine::MODEL_BAKED_LIGHTING_INSTANCE emptyShadowLighting{};
    const auto& shadowLighting = bakedLighting ? *bakedLighting : emptyShadowLighting;
    const uint32_t hasStaticShadow = nativeSurface && nativeSurface->hasStaticShadow ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_HasStaticShadow", &hasStaticShadow, sizeof(hasStaticShadow))) ||
        (bindingMode == MAP_MATERIAL_BINDING_MODE::OBJECT &&
         FAILED(shader->Bind_RawValue("g_StaticShadowScaleBias", &shadowLighting.shadowScaleBias, sizeof(shadowLighting.shadowScaleBias)))) ||
        (hasStaticShadow && FAILED(model->Bind_SurfaceLighting(shader, meshIndex)))) return E_FAIL;

    if (nativeSurface && nativeSurface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
    {
        const uint32_t noMapSurface = 0u;
        if (FAILED(shader->Bind_RawValue("g_SurfaceProgram", &noMapSurface, sizeof(noMapSurface))) ||
            FAILED(shader->Bind_RawValue("g_HasSurfaceDefinition", &noMapSurface, sizeof(noMapSurface)))) return E_FAIL;
        const uint32_t sourceProgram = nativeSurface->sourceCharacter.program;
        const bool movieStatic = SourceMovieMaterial::Is_Static(sourceProgram);
        const bool movieForward = SourceMovieMaterial::Is_Forward(sourceProgram) ||
            sourceProgram == 224u || sourceProgram == 225u || sourceProgram == 226u || sourceProgram == 237u;
        const bool forwardBakedProgram = (sourceProgram >= 40u && sourceProgram <= 63u &&
            sourceProgram != 47u && sourceProgram != 53u && sourceProgram != 55u) || sourceProgram == 209u || movieForward;
        if ((sourceProgram >= 80u && sourceProgram <= 83u) || sourceProgram == 210u ||
            (sourceProgram >= 214u && sourceProgram <= 234u) || sourceProgram == 237u || movieStatic)
        {
            const Engine::MODEL_BAKED_LIGHTING_INSTANCE emptyLighting{};
            const auto& instanceLighting = bakedLighting ? *bakedLighting : emptyLighting;
            if (FAILED(shader->Bind_RawValue("g_LightmapScaleBias", &instanceLighting.scaleBias, sizeof(instanceLighting.scaleBias))) ||
                FAILED(shader->Bind_RawValue("g_LightmapAverageScale", &instanceLighting.averageScale, sizeof(instanceLighting.averageScale))) ||
                FAILED(shader->Bind_RawValue("g_LightmapDirectionalScale", &instanceLighting.directionalScale, sizeof(instanceLighting.directionalScale)))) return E_FAIL;
        }
        if ((sourceProgram >= 33u && sourceProgram <= 63u) || sourceProgram == 65u || sourceProgram == 209u || movieForward)
        {
            float4_t ambient(0.f, 0.f, 0.f, 1.f);
            for (const auto& light : CGameInstance::Get().Get_SceneLights())
            {
                ambient.x += light.vAmbient.x;
                ambient.y += light.vAmbient.y;
                ambient.z += light.vAmbient.z;
            }
            if (FAILED(CGameInstance::Get().Bind_HeightFog(shader.get())) ||
                FAILED(shader->Bind_RawValue("g_SourceMapAmbient", &ambient, sizeof(ambient))) ||
                FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), shader,
                    "g_SourceMapSceneDepth"))) return E_FAIL;
            if (((sourceProgram >= 38u && sourceProgram <= 63u) || SourceMovieMaterial::Needs_SceneColor(sourceProgram)) &&
                FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_EffectSceneColor"), shader,
                    "g_SourceMapSceneColor"))) return E_FAIL;
        }
        if (((sourceProgram >= 38u && sourceProgram <= 63u) || sourceProgram == 209u || movieForward) &&
            FAILED(BindForwardSceneLights(shader, forwardBakedProgram && nativeSurface->hasBakedLighting && bakedLighting, worldCullSphere))) return E_FAIL;
        if ((sourceProgram >= 40u && sourceProgram <= 63u) || sourceProgram == 209u || movieForward)
        {
            const uint32_t hasBaked = forwardBakedProgram && nativeSurface->hasBakedLighting && bakedLighting ? 1u : 0u;
            const Engine::MODEL_BAKED_LIGHTING_INSTANCE emptyLighting{};
            const auto& lighting = bakedLighting ? *bakedLighting : emptyLighting;
            if (FAILED(shader->Bind_RawValue("g_HasBakedLighting", &hasBaked, sizeof(hasBaked))) ||
                FAILED(shader->Bind_RawValue("g_LightmapScaleBias", &lighting.scaleBias, sizeof(lighting.scaleBias))) ||
                FAILED(shader->Bind_RawValue("g_LightmapAverageScale", &lighting.averageScale, sizeof(lighting.averageScale))) ||
                FAILED(shader->Bind_RawValue("g_LightmapDirectionalScale", &lighting.directionalScale, sizeof(lighting.directionalScale))) ||
                (hasBaked && !hasStaticShadow && FAILED(model->Bind_SurfaceLighting(shader, meshIndex)))) return E_FAIL;
        }
        if (FAILED(BindSourceFoliageWind(shader, nativeSurface, elapsedTime, model->Is_Skinned()))) return E_FAIL;
        if (FAILED(model->Bind_SourceCharacter(shader, meshIndex))) return E_FAIL;
        return movieForward ? model->Bind_SourceCharacterForwardLight(shader, meshIndex) : S_OK;
    }

	const auto* surface = nativeSurface;
	const bool_t hasDefinition = surface &&
		surface->family != Engine::MODEL_SURFACE_FAMILY::LEGACY &&
		profile.renderMode == MAP_ASSET_RENDER_MODE::DEFERRED && !diffuseOverride;
	const uint32_t hasSurface = hasDefinition ? 1u : 0u;
	const uint32_t program = hasDefinition && settings.bUseSourceMaterials ?
		static_cast<uint32_t>(surface->family) : 0u;
	const uint32_t debugView = static_cast<uint32_t>(settings.eDebugView);
    const bool pbrComparisonActive = settings.MapPBR.Is_Active(Engine::CGameInstance::Get().Get_CurrentLevelID());
    const float4_t pbrContributions = pbrComparisonActive ? settings.MapPBR.vContributionScale : float4_t(1.f, 1.f, 1.f, 1.f);
    const float4_t pbrParameters = pbrComparisonActive ? settings.MapPBR.vSurfaceParameters : float4_t(1.f, 0.f, 0.f, 0.f);
    if (FAILED(shader->Bind_RawValue("g_MapPBRContributionScale", &pbrContributions, sizeof(pbrContributions))) ||
        FAILED(shader->Bind_RawValue("g_MapPBRDiagnosticParameters", &pbrParameters, sizeof(pbrParameters)))) return E_FAIL;
    if (FAILED(BindSourceFoliageWind(shader, surface, elapsedTime, model->Is_Skinned()))) return E_FAIL;
	// Bind last: the legacy diffuse binder resets source programs on shared shaders.
	if (FAILED(shader->Bind_RawValue("g_SurfaceProgram", &program, sizeof(program))) ||
		FAILED(shader->Bind_RawValue("g_HasSurfaceDefinition", &hasSurface, sizeof(hasSurface))) ||
		FAILED(shader->Bind_RawValue("g_SurfaceDebugView", &debugView, sizeof(debugView))))
		return E_FAIL;
    const uint32_t hasBaked = (program == 3u || program == 4u || program == 5u || program == 7u || program == 8u || program == 9u || program == 10u || (program >= 11u && program <= 13u)) && surface->hasBakedLighting ? 1u : 0u;
    // Most map surfaces have no source indirect input. Avoid copying the
    // environment's cube reference and path string for those ordinary draws.
    const bool sourceIndirectEnabled = surface && surface->hasSourceIndirect &&
        Engine::CGameInstance::Get().Get_RenderEnvironment().bUseSourcePBRIndirect;
    const uint32_t hasEnvironment = (program == 3u || program == 4u) && surface->hasEnvironmentCube &&
        (surface->environmentLegacyEnabled || sourceIndirectEnabled) ? 1u : 0u;
    const Engine::MODEL_BAKED_LIGHTING_INSTANCE emptyLighting{};
    const auto& lighting = bakedLighting ? *bakedLighting : emptyLighting;
    if (FAILED(shader->Bind_RawValue("g_HasBakedLighting", &hasBaked, sizeof(hasBaked))) ||
        FAILED(shader->Bind_RawValue("g_HasEnvironmentCube", &hasEnvironment, sizeof(hasEnvironment))) ||
        FAILED(shader->Bind_RawValue("g_HasEnvironmentBRDFLookup", &hasEnvironment, sizeof(hasEnvironment))) ||
        (bindingMode == MAP_MATERIAL_BINDING_MODE::OBJECT &&
        (FAILED(shader->Bind_RawValue("g_LightmapScaleBias", &lighting.scaleBias, sizeof(lighting.scaleBias))) ||
        FAILED(shader->Bind_RawValue("g_LightmapAverageScale", &lighting.averageScale, sizeof(lighting.averageScale))) ||
        FAILED(shader->Bind_RawValue("g_LightmapDirectionalScale", &lighting.directionalScale, sizeof(lighting.directionalScale)))))) return E_FAIL;
    // A static-shadow bind above already supplies this material's RNM and environment SRVs.
    // Reuse only within this call; sibling materials share the Effect and may replace them.
    if ((hasBaked || hasEnvironment) && !hasStaticShadow)
    {
        if (FAILED(model->Bind_SurfaceLighting(shader, meshIndex))) return E_FAIL;
    }
    if (hasEnvironment)
    {
        const auto& environmentColor = sourceIndirectEnabled ? surface->sourceIndirectColor : surface->environmentColor;
        const auto& environmentRotation = sourceIndirectEnabled ? surface->sourceIndirectRotation : surface->environmentRotation;
        if (FAILED(shader->Bind_RawValue("g_EnvironmentColor", &environmentColor, sizeof(environmentColor))) ||
            FAILED(shader->Bind_RawValue("g_EnvironmentRotation", &environmentRotation, sizeof(environmentRotation)))) return E_FAIL;
    }
    const uint32_t useSourceIndirect = hasEnvironment && sourceIndirectEnabled ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_UseSourcePBRIndirect", &useSourceIndirect, sizeof(useSourceIndirect)))) return E_FAIL;
    if (useSourceIndirect &&
        (FAILED(shader->Bind_RawValue("g_SourcePBRPackedSH", surface->sourceIndirectSH.data(), sizeof(surface->sourceIndirectSH))) ||
         FAILED(shader->Bind_RawValue("g_SourcePBRUpperSky", &surface->sourceUpperSkyColor, sizeof(surface->sourceUpperSkyColor))) ||
         FAILED(shader->Bind_RawValue("g_SourcePBRLowerSky", &surface->sourceLowerSkyColor, sizeof(surface->sourceLowerSkyColor))) ||
         FAILED(shader->Bind_RawValue("g_SourcePBRAmbientAndSkyFactor", &surface->sourceAmbientAndSkyFactor, sizeof(surface->sourceAmbientAndSkyFactor))))) return E_FAIL;
	const auto recordBinding = [&]()
	{
		if (hasDefinition && !diagnosticAssetId.empty())
			RecordSurfaceBinding(diagnosticAssetId, model->Get_MaterialName(meshIndex), *surface, program, lighting);
	};
	if (program == 0u)
	{
		recordBinding();
		return S_OK;
	}
	if ((program == 3u || program == 4u || program == 7u || program == 8u || program == 9u || program == 10u) && surface->hasEmissive)
	{
		if (!std::isfinite(elapsedTime))
			return E_INVALIDARG;
		if (FAILED(model->Bind_SurfaceTexture(shader, "g_SurfaceEmissiveTexture", meshIndex, aiTextureType_EMISSIVE)) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveColor", &surface->emissiveColor, sizeof(surface->emissiveColor))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveIntensity", &surface->emissiveIntensity, sizeof(surface->emissiveIntensity))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveUVTiling", &surface->emissiveUVTiling, sizeof(surface->emissiveUVTiling))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveFlickerMinimum", &surface->emissiveFlickerMinimum, sizeof(surface->emissiveFlickerMinimum))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveFlickerSpeed", &surface->emissiveFlickerSpeed, sizeof(surface->emissiveFlickerSpeed))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissivePhaseOffset", &surface->emissivePhaseOffset, sizeof(surface->emissivePhaseOffset))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveTime", &elapsedTime, sizeof(elapsedTime))) ||
            ((program == 3u || program == 4u) &&
             FAILED(shader->Bind_RawValue("g_SourceBgFlicker", &surface->sourceBgFlicker, sizeof(surface->sourceBgFlicker)))))
			return E_FAIL;
		const uint32_t hasSurfaceEmissive = 1u;
		if (FAILED(shader->Bind_RawValue("g_HasSurfaceEmissive", &hasSurfaceEmissive, sizeof(hasSurfaceEmissive))))
			return E_FAIL;
	}
	const auto* camera = CGameInstance::Get().Get_CamPosition();
	if (!camera || FAILED(shader->Bind_RawValue("g_vCamPosition", camera, sizeof(*camera))) ||
		FAILED(model->Bind_SurfaceTexture(shader, "g_DiffuseTexture", meshIndex, aiTextureType_DIFFUSE)) ||
        (program != 7u && program != 8u && program != 9u && program != 10u && program != 12u && FAILED(model->Bind_SurfaceTexture(shader, "g_ReflectionTexture", meshIndex, aiTextureType_REFLECTION))) ||
		((program == 1u || program == 5u) && FAILED(model->Bind_SurfaceTexture(shader, "g_SpecularTexture", meshIndex, aiTextureType_SPECULAR))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceDiffuseBrightness", &surface->diffuseBrightness, sizeof(surface->diffuseBrightness))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceNormalIntensity", &surface->normalIntensity, sizeof(surface->normalIntensity))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceSpecularIntensity", &surface->specularIntensity, sizeof(surface->specularIntensity))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceSpecularPower", &surface->specularPower, sizeof(surface->specularPower))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceReflectionIntensity", &surface->reflectionIntensity, sizeof(surface->reflectionIntensity))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceReflectionContrast", &surface->reflectionContrast, sizeof(surface->reflectionContrast))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceReflectionTiling", &surface->reflectionTiling, sizeof(surface->reflectionTiling))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceDiffuseSaturation", &surface->diffuseSaturation, sizeof(surface->diffuseSaturation))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceDiffuseColor", &surface->diffuseColor, sizeof(surface->diffuseColor))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceSpecularColor", &surface->specularColor, sizeof(surface->specularColor))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceReflectionColor", &surface->reflectionColor, sizeof(surface->reflectionColor))))
		return E_FAIL;
    if (program >= 11u && program <= 13u && FAILED(model->Bind_SourceSpecialSurface(shader, meshIndex)))
        return E_FAIL;
    if (program == 9u || program == 10u)
    {
        const uint32_t flags = surface->sourceFoliageFlags;
        if (((flags & 1u) && FAILED(model->Bind_SurfaceTexture(shader, "g_NormalTexture", meshIndex, aiTextureType_NORMALS))) ||
            ((flags & 8u) && FAILED(model->Bind_SurfaceTexture(shader, "g_SpecularTexture", meshIndex, aiTextureType_SPECULAR))) ||
            (program == 9u && FAILED(model->Bind_SurfaceTexture(shader, "g_SourceFoliageMaskTexture", meshIndex, aiTextureType_TRANSMISSION))) ||
            FAILED(shader->Bind_RawValue("g_SourceFoliageFlags", &flags, sizeof(flags))) ||
            FAILED(shader->Bind_RawValue("g_SourceFoliageTransmission", &surface->sourceFoliageTransmission,
                sizeof(surface->sourceFoliageTransmission)))) return E_FAIL;
    }
    if (program == 8u)
    {
        const uint32_t flags = surface->sourceBgFlags;
        if (!std::isfinite(elapsedTime) ||
            FAILED(shader->Bind_RawValue("g_SourceBgSubspecular", &surface->sourceBgSubspecular, sizeof(surface->sourceBgSubspecular))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgRimlight", &surface->sourceBgRimlight, sizeof(surface->sourceBgRimlight))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgSpecularSaturation", &surface->sourceBgSpecularSaturation, sizeof(surface->sourceBgSpecularSaturation))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgPanning", &surface->sourceBgPanning, sizeof(surface->sourceBgPanning))) ||
            (!surface->hasEmissive &&
             FAILED(shader->Bind_RawValue("g_SurfaceEmissiveTime", &elapsedTime, sizeof(elapsedTime))))) return E_FAIL;
        if ((flags & 32768u) &&
            (FAILED(model->Bind_SurfaceTexture(shader, "g_DetailNormalTexture", meshIndex, aiTextureType_HEIGHT)) ||
             FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalIntensity", &surface->detailNormalIntensity, sizeof(surface->detailNormalIntensity))) ||
             FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalTiling", &surface->detailNormalTiling, sizeof(surface->detailNormalTiling))))) return E_FAIL;
        if (((flags & 1u) && FAILED(model->Bind_SurfaceTexture(shader, "g_NormalTexture", meshIndex, aiTextureType_NORMALS))) ||
            ((flags & 8u) && FAILED(model->Bind_SurfaceTexture(shader, "g_SpecularTexture", meshIndex, aiTextureType_SPECULAR))) ||
            ((flags & 16u) && FAILED(model->Bind_SurfaceTexture(shader, "g_ReflectionTexture", meshIndex, aiTextureType_REFLECTION))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgFlags", &flags, sizeof(flags))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgBump", &surface->sourceBgBump, sizeof(surface->sourceBgBump))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgUV", &surface->sourceBgUV, sizeof(surface->sourceBgUV))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgFlicker", &surface->sourceBgFlicker, sizeof(surface->sourceBgFlicker))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &surface->uvTiling, sizeof(surface->uvTiling))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceReflectionOriginOffset", &surface->reflectionOriginOffset, sizeof(surface->reflectionOriginOffset))))
            return E_FAIL;
    }
    if (program == 5u)
    {
        if (FAILED(model->Bind_SurfaceTexture(shader, "g_NormalTexture", meshIndex, aiTextureType_NORMALS)) ||
            FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &surface->uvTiling, sizeof(surface->uvTiling))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceReflectionOriginOffset", &surface->reflectionOriginOffset, sizeof(surface->reflectionOriginOffset))))
            return E_FAIL;
    }
    if (program == 7u)
    {
        const uint32_t separateSpecular = surface->overlaySeparateSpecular ? 1u : 0u;
        if (FAILED(shader->Bind_RawValue("g_SourceBgSubspecular", &surface->sourceBgSubspecular, sizeof(surface->sourceBgSubspecular))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgSpecularSaturation", &surface->sourceBgSpecularSaturation, sizeof(surface->sourceBgSpecularSaturation))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgBump", &surface->sourceBgBump, sizeof(surface->sourceBgBump))) ||
            FAILED(shader->Bind_RawValue("g_SourceOverlayFlags", &surface->sourceOverlayFlags, sizeof(surface->sourceOverlayFlags))) ||
            FAILED(shader->Bind_RawValue("g_SourceOverlayDirection", &surface->sourceOverlayDirection, sizeof(surface->sourceOverlayDirection))) ||
            FAILED(shader->Bind_RawValue("g_SourceOverlayUV", &surface->sourceBgUV, sizeof(surface->sourceBgUV))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalIntensity", &surface->detailNormalIntensity, sizeof(surface->detailNormalIntensity))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalTiling", &surface->detailNormalTiling, sizeof(surface->detailNormalTiling))) ||
            ((surface->sourceOverlayFlags & 32u) != 0u && FAILED(model->Bind_SurfaceTexture(shader, "g_DetailNormalTexture", meshIndex, aiTextureType_HEIGHT)))) return E_FAIL;
        if (FAILED(shader->Bind_RawValue("g_SurfaceOverlaySeparateSpecular", &separateSpecular, sizeof(separateSpecular))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &surface->uvTiling, sizeof(surface->uvTiling))) ||
            (separateSpecular && FAILED(model->Bind_SurfaceTexture(shader, "g_SpecularTexture", meshIndex, aiTextureType_SPECULAR)))) return E_FAIL;
        if (((surface->sourceOverlayFlags & 1u) != 0u && FAILED(model->Bind_SurfaceTexture(shader, "g_NormalTexture", meshIndex, aiTextureType_NORMALS))) ||
            FAILED(model->Bind_SurfaceTexture(shader, "g_SurfaceOverlayDiffuseTexture", meshIndex, aiTextureType_BASE_COLOR)) ||
            ((surface->sourceOverlayFlags & 2u) != 0u && FAILED(model->Bind_SurfaceTexture(shader, "g_SurfaceOverlayNormalTexture", meshIndex, aiTextureType_NORMAL_CAMERA))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlayColor", &surface->overlayColor, sizeof(surface->overlayColor))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlayTiling", &surface->overlayTiling, sizeof(surface->overlayTiling))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlayNormalIntensity", &surface->overlayNormalIntensity, sizeof(surface->overlayNormalIntensity))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlaySharpness", &surface->overlaySharpness, sizeof(surface->overlaySharpness))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlayBrightness", &surface->overlayBrightness, sizeof(surface->overlayBrightness))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlaySaturation", &surface->overlaySaturation, sizeof(surface->overlaySaturation))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlaySpecularIntensity", &surface->overlaySpecularIntensity, sizeof(surface->overlaySpecularIntensity)))) return E_FAIL;
    }
	if (program == 3u || program == 4u)
	{
		const uint32_t masked = surface->pbrAlphaMasked ? 1u : 0u;
		if (FAILED(shader->Bind_RawValue("g_SurfacePBRMasked", &masked, sizeof(masked))))
			return E_FAIL;
		if (FAILED(model->Bind_SurfaceTexture(shader, "g_NormalTexture", meshIndex, aiTextureType_NORMALS)) ||
			FAILED(model->Bind_SurfaceTexture(shader, "g_DetailNormalTexture", meshIndex, aiTextureType_HEIGHT)) ||
			FAILED(model->Bind_SurfaceTexture(shader, "g_SurfaceORMTexture", meshIndex, aiTextureType_UNKNOWN)))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &surface->uvTiling, sizeof(surface->uvTiling))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalIntensity", &surface->detailNormalIntensity, sizeof(surface->detailNormalIntensity))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalTiling", &surface->detailNormalTiling, sizeof(surface->detailNormalTiling))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceMetallicIntensity", &surface->metallicIntensity, sizeof(surface->metallicIntensity))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceMetallicPower", &surface->metallicPower, sizeof(surface->metallicPower))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceRoughnessIntensity", &surface->roughnessIntensity, sizeof(surface->roughnessIntensity))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceRoughnessPower", &surface->roughnessPower, sizeof(surface->roughnessPower))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceAOIntensity", &surface->aoIntensity, sizeof(surface->aoIntensity))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceAOPower", &surface->aoPower, sizeof(surface->aoPower))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceSpecularPBRIntensity", &surface->specularPBRIntensity, sizeof(surface->specularPBRIntensity))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceNonmetallicBrightness", &surface->nonmetallicBrightness, sizeof(surface->nonmetallicBrightness))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceMetallicBrightness", &surface->metallicBrightness, sizeof(surface->metallicBrightness))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceMinimumRoughness", &surface->minimumRoughness, sizeof(surface->minimumRoughness))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceReflectionOriginOffset", &surface->reflectionOriginOffset, sizeof(surface->reflectionOriginOffset))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceVertexAlpha", &surface->vertexAlpha, sizeof(surface->vertexAlpha))))
			return E_FAIL;
		const uint32_t uvFixedNormal = surface->uvFixedNormal ? 1u : 0u;
		if (FAILED(shader->Bind_RawValue("g_SurfaceUVFixedNormal", &uvFixedNormal, sizeof(uvFixedNormal))))
			return E_FAIL;
		const uint32_t useWorldReflection = surface->useWorldReflection ? 1u : 0u;
		if (FAILED(shader->Bind_RawValue("g_SurfaceUseWorldReflection", &useWorldReflection, sizeof(useWorldReflection))))
			return E_FAIL;
	}
	recordBinding();
	return S_OK;
}

```

### C:/Users/user/Desktop/LostArk/Engine/Private/Material.cpp

```cpp
#include "Material.h"
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")
#pragma push_macro("new")
#undef new
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#pragma pop_macro("new")

#include <mutex>
#include <unordered_map>
#include "BinaryAsset/ModelAssetData.h"
#include "Shader.h"
#include "GameInstance.h"
#include "Profiler.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cwctype>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>

#pragma pack(push, 1)
namespace
{
	struct TGA_HEADER
	{
		uint8_t idLength;
		uint8_t colorMapType;
		uint8_t imageType;
		uint16_t colorMapFirst;
		uint16_t colorMapLength;
		uint8_t colorMapDepth;
		uint16_t xOrigin;
		uint16_t yOrigin;
		uint16_t width;
		uint16_t height;
		uint8_t bitsPerPixel;
		uint8_t descriptor;
	};
}
#pragma pack(pop)

namespace
{
	struct RGBA_MIP_LEVEL
	{
		uint32_t width;
		uint32_t height;
		vector<uint8_t> pixels;
	};

	vector<RGBA_MIP_LEVEL> BuildRgbaMipChain(uint32_t width, uint32_t height,
		vector<uint8_t>&& rgba, bool_t isColorSlot)
	{
		static const array<double, 256> srgbToLinear = []
		{
			array<double, 256> values{};
			for (size_t index = 0; index < values.size(); ++index)
			{
				const double value = static_cast<double>(index) / 255.0;
				values[index] = value <= 0.04045 ? value / 12.92 :
					pow((value + 0.055) / 1.055, 2.4);
			}
			return values;
		}();

		vector<RGBA_MIP_LEVEL> levels;
		// Moving the decoded image preserves every mip-zero channel byte.
		levels.push_back({ width, height, move(rgba) });
		while (levels.back().width > 1 || levels.back().height > 1)
		{
			const RGBA_MIP_LEVEL& source = levels.back();
			RGBA_MIP_LEVEL target{ max(1u, source.width / 2),
				max(1u, source.height / 2), {} };
			target.pixels.resize(static_cast<size_t>(target.width) * target.height * 4);
			for (uint32_t y = 0; y < target.height; ++y)
			{
				const double top = static_cast<double>(y) * source.height / target.height;
				const double bottom = static_cast<double>(y + 1) * source.height / target.height;
				const uint32_t firstY = static_cast<uint32_t>(top);
				const uint32_t endY = min(source.height, static_cast<uint32_t>(ceil(bottom)));
				for (uint32_t x = 0; x < target.width; ++x)
				{
					const double left = static_cast<double>(x) * source.width / target.width;
					const double right = static_cast<double>(x + 1) * source.width / target.width;
					const uint32_t firstX = static_cast<uint32_t>(left);
					const uint32_t endX = min(source.width, static_cast<uint32_t>(ceil(right)));
					double sum[4]{};
					// Area weights include the final row/column of odd-sized images.
					for (uint32_t sourceY = firstY; sourceY < endY; ++sourceY)
					{
						const double weightY = min(bottom, static_cast<double>(sourceY + 1)) -
							max(top, static_cast<double>(sourceY));
						for (uint32_t sourceX = firstX; sourceX < endX; ++sourceX)
						{
							const double weight = weightY *
								(min(right, static_cast<double>(sourceX + 1)) -
									max(left, static_cast<double>(sourceX)));
							const size_t index =
								(static_cast<size_t>(sourceY) * source.width + sourceX) * 4;
							for (size_t channel = 0; channel < 4; ++channel)
							{
								const uint8_t value = source.pixels[index + channel];
								sum[channel] += weight * ((isColorSlot && channel < 3)
									? srgbToLinear[value] : static_cast<double>(value) / 255.0);
							}
						}
					}
					const double area = (right - left) * (bottom - top);
					const size_t index = (static_cast<size_t>(y) * target.width + x) * 4;
					for (size_t channel = 0; channel < 4; ++channel)
					{
						double value = clamp(sum[channel] / area, 0.0, 1.0);
						if (isColorSlot && channel < 3)
							value = value <= 0.0031308 ? value * 12.92 :
								1.055 * pow(value, 1.0 / 2.4) - 0.055;
						target.pixels[index + channel] = static_cast<uint8_t>(
							clamp(lround(value * 255.0), 0l, 255l));
					}
				}
			}
			levels.push_back(move(target));
		}
		return levels;
	}

	HRESULT LoadTgaTexture(ComPtr<ID3D11Device> pDevice,
		const filesystem::path& path,
		bool_t isColorSlot,
		ComPtr<ID3D11ShaderResourceView>& pSRV)
	{
		ifstream input(path, ios::binary);
		if (!input)
			return E_FAIL;
		const vector<uint8_t> bytes(
			(istreambuf_iterator<char>(input)), istreambuf_iterator<char>());
		if (bytes.size() < sizeof(TGA_HEADER))
			return E_FAIL;

		TGA_HEADER header{};
		memcpy(&header, bytes.data(), sizeof(header));
		if (0 != header.colorMapType ||
			(2 != header.imageType && 10 != header.imageType) ||
			(24 != header.bitsPerPixel && 32 != header.bitsPerPixel) ||
			0 == header.width || 0 == header.height)
			return E_FAIL;

		const size_t sourceStride = header.bitsPerPixel / 8;
		const size_t pixelCount =
			static_cast<size_t>(header.width) * header.height;
		vector<uint8_t> rgba(pixelCount * 4);
		size_t cursor = sizeof(TGA_HEADER) + header.idLength;
		size_t sourcePixel = 0;

		auto writePixel = [&](const uint8_t* source) -> bool_t
		{
			if (sourcePixel >= pixelCount)
				return false;
			const size_t sourceX = sourcePixel % header.width;
			const size_t sourceY = sourcePixel / header.width;
			const size_t targetX = (header.descriptor & 0x10)
				? header.width - 1 - sourceX : sourceX;
			const size_t targetY = (header.descriptor & 0x20)
				? sourceY : header.height - 1 - sourceY;
			const size_t target = (targetY * header.width + targetX) * 4;
			rgba[target + 0] = source[2];
			rgba[target + 1] = source[1];
			rgba[target + 2] = source[0];
			rgba[target + 3] = 4 == sourceStride ? source[3] : 255;
			++sourcePixel;
			return true;
		};

		if (2 == header.imageType)
		{
			while (sourcePixel < pixelCount)
			{
				if (cursor + sourceStride > bytes.size() ||
					!writePixel(bytes.data() + cursor))
					return E_FAIL;
				cursor += sourceStride;
			}
		}
		else
		{
			while (sourcePixel < pixelCount)
			{
				if (cursor >= bytes.size())
					return E_FAIL;
				const uint8_t packet = bytes[cursor++];
				const size_t count = (packet & 0x7f) + 1;
				if (packet & 0x80)
				{
					if (cursor + sourceStride > bytes.size())
						return E_FAIL;
					const uint8_t* source = bytes.data() + cursor;
					cursor += sourceStride;
					for (size_t index = 0; index < count; ++index)
					{
						if (!writePixel(source))
							return E_FAIL;
					}
				}
				else
				{
					for (size_t index = 0; index < count; ++index)
					{
						if (cursor + sourceStride > bytes.size() ||
							!writePixel(bytes.data() + cursor))
							return E_FAIL;
						cursor += sourceStride;
					}
				}
			}
		}

		const vector<RGBA_MIP_LEVEL> mipLevels = BuildRgbaMipChain(
			header.width, header.height, move(rgba), isColorSlot);
		vector<D3D11_SUBRESOURCE_DATA> initialData(mipLevels.size());
		for (size_t index = 0; index < mipLevels.size(); ++index)
		{
			initialData[index].pSysMem = mipLevels[index].pixels.data();
			initialData[index].SysMemPitch = mipLevels[index].width * 4;
		}

		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = header.width;
		textureDesc.Height = header.height;
		textureDesc.MipLevels = static_cast<UINT>(mipLevels.size());
		textureDesc.ArraySize = 1;
		textureDesc.Format = isColorSlot ?
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		ComPtr<ID3D11Texture2D> texture;
		if (FAILED(pDevice->CreateTexture2D(&textureDesc, initialData.data(), &texture)))
			return E_FAIL;
		return pDevice->CreateShaderResourceView(texture.Get(), nullptr, &pSRV);
	}

	/* Base colour and emissive textures are authored in sRGB, while lighting runs
	   in linear space and the post pass owns the only gamma encode. Decoding them
	   on load keeps that contract. Normal, specular, ORM and mask textures carry
	   data instead of colour, so they stay linear. */
	bool_t IsColorTextureSlot(aiTextureType type)
	{
		return aiTextureType_DIFFUSE == type ||
			aiTextureType_EMISSIVE == type;
	}

	HRESULT LoadTexture(ComPtr<ID3D11Device> pDevice,
		const filesystem::path& path,
		bool_t isColorSlot,
		ComPtr<ID3D11ShaderResourceView>& pSRV,
		bool_t forceLinear = false)
	{
		if (path.empty())
			return E_FAIL;

		wstring extension = path.extension().wstring();
		transform(extension.begin(), extension.end(), extension.begin(), towlower);
		if (L".dds" == extension)
			return CreateDDSTextureFromFileEx(pDevice.Get(), path.c_str(), 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
				isColorSlot ? DDS_LOADER_FORCE_SRGB :
				(forceLinear ? DDS_LOADER_IGNORE_SRGB : DDS_LOADER_DEFAULT),
				nullptr, &pSRV);
		if (L".tga" == extension)
			return LoadTgaTexture(pDevice, path, isColorSlot, pSRV);
		return CreateWICTextureFromFileEx(pDevice.Get(), path.c_str(), 0,
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
			isColorSlot ? WIC_LOADER_FORCE_SRGB :
			(forceLinear ? WIC_LOADER_IGNORE_SRGB : WIC_LOADER_DEFAULT),
			nullptr, &pSRV);
	}

    HRESULT LoadSharedTexture(ComPtr<ID3D11Device> device,
        vector<shared_ptr<ComPtr<ID3D11ShaderResourceView>>>& owners,
        const filesystem::path& path, bool_t srgb,
        ComPtr<ID3D11ShaderResourceView>& view, bool_t forceLinear = false)
    {
        if (path.empty()) return E_INVALIDARG;
        std::error_code error;
        const auto size = filesystem::file_size(path, error);
        if (error) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        const auto modified = filesystem::last_write_time(path, error);
        if (error) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        std::wstring normalized = path.lexically_normal().wstring();
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), towlower);
        const std::wstring key = std::to_wstring(reinterpret_cast<uintptr_t>(device.Get())) + L":" +
            normalized + L":" + std::to_wstring(srgb) + L":" + std::to_wstring(forceLinear) + L":" +
            std::to_wstring(size) + L":" + std::to_wstring(modified.time_since_epoch().count());
        struct TEXTURE_LOAD_ENTRY final
        {
            std::mutex loadMutex;
            std::weak_ptr<ComPtr<ID3D11ShaderResourceView>> texture;
        };
        static std::mutex cacheMutex;
        static std::unordered_map<std::wstring, std::shared_ptr<TEXTURE_LOAD_ENTRY>> cache;
        static size_t insertions = 0u;
        std::shared_ptr<TEXTURE_LOAD_ENTRY> entry;
        {
            Engine::CProfilerScope lookupScope(CGameInstance::Get().Get_Profiler(), "Texture.Cache.Lookup");
            std::lock_guard<std::mutex> lock(cacheMutex);
            const auto found = cache.find(key);
            if (found != cache.end()) entry = found->second;
            else
            {
                // Only idle entries can be inspected or removed under the map
                // lock. An active loader owns an additional strong reference.
                if ((++insertions % 256u) == 0u)
                    std::erase_if(cache, [](const auto& pair) {
                        if (pair.second.use_count() != 1) return false;
                        std::unique_lock idleLock(pair.second->loadMutex, std::try_to_lock);
                        return idleLock.owns_lock() && pair.second->texture.expired();
                    });
                entry = std::make_shared<TEXTURE_LOAD_ENTRY>();
                cache.emplace(key, entry);
            }
        }
        std::unique_lock<std::mutex> loadLock;
        {
            Engine::CProfilerScope waitScope(CGameInstance::Get().Get_Profiler(), "Texture.Cache.SameKeyWait");
            loadLock = std::unique_lock<std::mutex>(entry->loadMutex);
        }
        auto shared = entry->texture.lock();
        if (!shared)
        {
            Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Texture.Load.FileAndUpload");
            shared = std::make_shared<ComPtr<ID3D11ShaderResourceView>>();
            const HRESULT result = LoadTexture(device, path, srgb, *shared, forceLinear);
            if (FAILED(result)) return result;
            entry->texture = shared;
        }
        view = *shared;
        owners.push_back(std::move(shared));
        return S_OK;
    }

	HRESULT AddTexture(ComPtr<ID3D11Device> pDevice,
        vector<shared_ptr<ComPtr<ID3D11ShaderResourceView>>>& owners,
		const filesystem::path& path,
		aiTextureType type,
		vector<ComPtr<ID3D11ShaderResourceView>> (&textures)[AI_TEXTURE_TYPE_MAX])
	{
		if (path.empty())
			return S_OK;
		ComPtr<ID3D11ShaderResourceView> resource;
		const HRESULT result = LoadSharedTexture(
			pDevice, owners, path, IsColorTextureSlot(type), resource);
		if (FAILED(result))
		{
			wstring message = L"[CMaterial] Texture load failed: ";
			message += path.wstring();
			message += L"\n";
			OutputDebugStringW(message.c_str());
			return result;
		}
		textures[type].push_back(resource);
		return S_OK;
	}

	HRESULT AddSolidTexture(ComPtr<ID3D11Device> pDevice,
		uint32_t rgba,
		aiTextureType type,
		vector<ComPtr<ID3D11ShaderResourceView>> (&textures)[AI_TEXTURE_TYPE_MAX])
	{
		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = 1;
		textureDesc.Height = 1;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initialData{};
		initialData.pSysMem = &rgba;
		initialData.SysMemPitch = sizeof(rgba);

		ComPtr<ID3D11Texture2D> texture;
		if (FAILED(pDevice->CreateTexture2D(&textureDesc, &initialData, &texture)))
			return E_FAIL;

		ComPtr<ID3D11ShaderResourceView> resource;
		if (FAILED(pDevice->CreateShaderResourceView(texture.Get(), nullptr, &resource)))
			return E_FAIL;
		textures[type].push_back(resource);
		return S_OK;
	}
}

CMaterial::CMaterial(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{	
}

CMaterial::~CMaterial()
{
}

HRESULT CMaterial::Initialize(const aiMaterial* pAIMaterial, const char_t* pModelFilePath)
{
	aiString materialName;
	if (AI_SUCCESS == pAIMaterial->Get(AI_MATKEY_NAME, materialName))
		m_strName = materialName.C_Str();

	char_t			szDrive[MAX_PATH] = {};
	char_t			szDir[MAX_PATH] = {};

	_splitpath_s(pModelFilePath, szDrive, MAX_PATH, szDir, MAX_PATH, nullptr, 0, nullptr, 0);

	for (uint32_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
	{
		uint32_t		iNumTextures = pAIMaterial->GetTextureCount(static_cast<aiTextureType>(i));

		m_Textures[i].reserve(iNumTextures);

		for (uint32_t j = 0; j < iNumTextures; j++)
		{
			ComPtr<ID3D11ShaderResourceView>		pSRV = { nullptr };

			aiString			strTextureFilePath = {};

			if (FAILED(pAIMaterial->GetTexture(static_cast<aiTextureType>(i), j, &strTextureFilePath)))
				return E_FAIL;

			
			char_t			szFileName[MAX_PATH] = {};
			char_t			szEXT[MAX_PATH] = {};

			_splitpath_s(strTextureFilePath.C_Str(), nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szEXT, MAX_PATH);

			char_t			szFullPath[MAX_PATH] = {};
			strcpy_s(szFullPath, szDrive);
			strcat_s(szFullPath, szDir);
			strcat_s(szFullPath, szFileName);
			strcat_s(szFullPath, szEXT);

			tchar_t			szTextureFilePath[MAX_PATH] = {};

			MultiByteToWideChar(CP_ACP, 0, szFullPath, strlen(szFullPath), szTextureFilePath, MAX_PATH);

			HRESULT		hr = {};

			if (false == strcmp(szEXT, ".dds"))
				hr = CreateDDSTextureFromFile(m_pDevice.Get(), szTextureFilePath, nullptr, &pSRV);
			else if (false == strcmp(szEXT, ".tga"))
				hr = E_FAIL;
			else
				hr = CreateWICTextureFromFile(m_pDevice.Get(), szTextureFilePath, nullptr, &pSRV);
			
			m_Textures[i].push_back(pSRV);
		}
	}

	return S_OK;
}

HRESULT CMaterial::Initialize(const MODEL_MATERIAL_DATA& material)
{
	m_strName = material.name;
	m_iNameHash = material.nameHash;
	m_iDiffuseMirrorU = material.diffuseMirrorU ? 1u : 0u;

	const filesystem::path& compatibleDiffusePath = material.diffusePath.empty()
		? material.emissivePath
		: material.diffusePath;
	if (compatibleDiffusePath.empty())
	{
		if (FAILED(AddSolidTexture(m_pDevice, 0xff4d4d4d,
			aiTextureType_DIFFUSE, m_Textures)))
			return E_FAIL;
	}
	else if (FAILED(AddTexture(m_pDevice, m_SharedTextureViews, compatibleDiffusePath,
		aiTextureType_DIFFUSE, m_Textures)))
		return E_FAIL;

	if (FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.normalPath,
			aiTextureType_NORMALS, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.specularPath,
			aiTextureType_SPECULAR, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.emissivePath,
			aiTextureType_EMISSIVE, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.opacityPath,
			aiTextureType_OPACITY, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.ormPath,
			aiTextureType_UNKNOWN, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.metallicPath,
			aiTextureType_METALNESS, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.roughnessPath,
			aiTextureType_DIFFUSE_ROUGHNESS, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.ambientOcclusionPath,
			aiTextureType_AMBIENT_OCCLUSION, m_Textures)))
		return E_FAIL;

	/* The colour mask samples as data, not colour: aiTextureType_BASE_COLOR
	is outside IsColorTextureSlot, so it loads without the sRGB flag. */
	if (FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.colorMaskPath,
		aiTextureType_BASE_COLOR, m_Textures)))
		return E_FAIL;
	m_ColorTint = material.colorTint;
	m_ColorTint.isEnabled = material.colorTint.isEnabled &&
		Has_Texture(aiTextureType_BASE_COLOR);
	m_ColorTint.isHairMask = !material.colorMaskPath.empty() &&
		material.colorMaskPath == material.diffusePath;
	m_AuthoredColorTint = m_ColorTint;
	m_Surface = material.surface;
    if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
    {
        const auto mask = m_Surface.sourceCharacter.baseTextureMask |
            m_Surface.sourceCharacter.lightTextureMask;
        for (uint32_t index = 0u; index < SOURCE_CHARACTER_TEXTURE_COUNT; ++index)
        {
            if ((mask & (1u << index)) == 0u) continue;
            const auto& input = material.sourceCharacterTextures[index];
            if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, input.path, input.srgb,
                m_SourceCharacterTextures[index], true))) return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
            m_SourceCharacterTextures[index]->GetDesc(&desc);
            if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
        }
        if (m_Surface.hasStaticShadow)
        {
            if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.staticShadowPath, false, m_StaticShadow, true))) return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC shadow{}; m_StaticShadow->GetDesc(&shadow);
            if (shadow.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D || shadow.Format != DXGI_FORMAT_R8_UNORM) return E_INVALIDARG;
        }
        if (m_Surface.hasBakedLighting)
        {
            const uint32_t program = m_Surface.sourceCharacter.program;
            const bool supportsBaked = (program >= 80u && program <= 83u) ||
                (program >= 40u && program <= 63u && program != 47u && program != 53u && program != 55u) ||
                program == 209u || program == 210u || (program >= 214u && program <= 234u) || program == 237u ||
                (program >= 1100u && program <= 1166u) || (program >= 1400u && program <= 1413u);
            if (!supportsBaked ||
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.bakedAveragePath, m_Surface.bakedLightingSRGB, m_BakedAverage, true)) ||
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.bakedDirectionalPath, m_Surface.bakedLightingSRGB, m_BakedDirectional, true))) return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC average{}, directional{};
            m_BakedAverage->GetDesc(&average); m_BakedDirectional->GetDesc(&directional);
            if (average.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
                directional.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
        }
        m_AuthoredSourceCharacter = m_Surface.sourceCharacter;
        return S_OK;
    }
	if (m_Surface.hasEmissive)
	{
		if ((m_Surface.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE &&
			m_Surface.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
            m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
            m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED) ||
			FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceEmissivePath,
				m_Surface.emissiveSRGB, m_SurfaceEmissive, true)))
		{
			OutputDebugStringA(("[CMaterial] Source emissive input load failed: " +
				m_strName + "\n").c_str());
			return E_FAIL;
		}
		D3D11_SHADER_RESOURCE_VIEW_DESC emissive{};
		m_SurfaceEmissive->GetDesc(&emissive);
		if (emissive.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D)
			return E_INVALIDARG;
	}
	if (MODEL_SURFACE_FAMILY::LEGACY != m_Surface.family)
	{
        const bool sourceSpecial = m_Surface.family >= MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE &&
            m_Surface.family <= MODEL_SURFACE_FAMILY::SOURCE_WET_OPAQUE;
		const auto& diffuse = material.surfaceDiffusePath.empty() ? material.diffusePath : material.surfaceDiffusePath;
		if (diffuse.empty() ||
            (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial &&
             m_Surface.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
             m_Surface.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE && material.normalPath.empty()) ||
            (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial && material.reflectionPath.empty()) ||
			FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, diffuse,
				m_Surface.diffuseSRGB, m_SurfaceDiffuse, true)) ||
            (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial &&
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.reflectionPath,
                    m_Surface.reflectionSRGB, m_SurfaceReflection, true))))
		{
			OutputDebugStringA(("[CMaterial] Source surface texture load failed: " +
				m_strName + "\n").c_str());
			return E_FAIL;
		}
		if (m_Surface.family == MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE ||
			m_Surface.family == MODEL_SURFACE_FAMILY::PBR_OPAQUE)
		{
			if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceNormalPath, false, m_SurfaceNormal, true)) ||
				FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.detailNormalPath, false, m_SurfaceDetailNormal, true)) ||
				FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceORMPath, m_Surface.ormSRGB, m_SurfaceORM, true)))
			{
				OutputDebugStringA(("[CMaterial] PBR input load failed: " + m_strName + "\n").c_str());
				return E_FAIL;
			}
		}
        if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED ||
            m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED)
        {
            const uint32_t flags = m_Surface.sourceFoliageFlags;
            if (((flags & 1u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceNormalPath, false, m_SurfaceNormal, true))) ||
                ((flags & 8u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceSpecularPath, m_Surface.specularSRGB, m_SurfaceSpecular, true))) ||
                (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
                    FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.sourceFoliageMaskPath, m_Surface.sourceFoliageMaskSRGB, m_SourceFoliageMask, true)))) return E_FAIL;
            for (const auto& texture : { m_SurfaceDiffuse, m_SurfaceNormal, m_SurfaceSpecular, m_SourceFoliageMask })
            {
                if (!texture) continue;
                D3D11_SHADER_RESOURCE_VIEW_DESC desc{}; texture->GetDesc(&desc);
                if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
            }
        }
        if (sourceSpecial)
        {
            const auto& special = m_Surface.sourceSpecial;
            const bool ice = m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE;
            const bool blend = m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_VERTEXBLEND_OPAQUE;
            const auto load = [&](bool required, const filesystem::path& path, bool srgb,
                ComPtr<ID3D11ShaderResourceView>& texture) -> HRESULT
            {
                if (!required) return S_OK;
                const auto hr = LoadSharedTexture(m_pDevice, m_SharedTextureViews, path, srgb, texture, true);
                if (FAILED(hr)) return hr;
                D3D11_SHADER_RESOURCE_VIEW_DESC desc{}; texture->GetDesc(&desc);
                return desc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D ? S_OK : E_INVALIDARG;
            };
            if (FAILED(load(true, material.surfaceNormalPath, false, m_SurfaceNormal)) ||
                FAILED(load(!blend, material.reflectionPath, m_Surface.reflectionSRGB, m_SurfaceReflection)) ||
                FAILED(load(!blend && (!ice || (special.flags & 2u)), material.surfaceSpecularPath, m_Surface.specularSRGB, m_SurfaceSpecular)) ||
                FAILED(load(blend || (ice && (special.flags & 1u)), material.detailNormalPath, false, m_SurfaceDetailNormal)) ||
                FAILED(load(blend, material.overlayDiffusePath, m_Surface.overlaySRGB, m_SurfaceOverlayDiffuse)) ||
                FAILED(load(blend, material.overlayNormalPath, false, m_SurfaceOverlayNormal)) ||
                FAILED(load(ice, material.sourceSpecialMaskPath, special.maskSRGB, m_SourceSpecialMask)) ||
                FAILED(load(blend && (special.flags & 1u), material.sourceBlendDiffuseGPath, special.blendGSRGB, m_SourceBlendDiffuseG)) ||
                FAILED(load(blend && (special.flags & 1u), material.sourceBlendNormalGPath, false, m_SourceBlendNormalG)) ||
                FAILED(load(blend && (special.flags & 2u), material.sourceBlendDiffuseBPath, special.blendBSRGB, m_SourceBlendDiffuseB)) ||
                FAILED(load(blend && (special.flags & 2u), material.sourceBlendNormalBPath, false, m_SourceBlendNormalB))) return E_FAIL;
        }
        if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED)
        {
            const uint32_t flags = m_Surface.sourceBgFlags;
            if ((flags & 32768u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews,
                material.detailNormalPath, false, m_SurfaceDetailNormal, true))) return E_FAIL;
            if (((flags & 1u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceNormalPath, false, m_SurfaceNormal, true))) ||
                ((flags & 8u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceSpecularPath, m_Surface.specularSRGB, m_SurfaceSpecular, true))) ||
                ((flags & 16u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.reflectionPath, m_Surface.reflectionSRGB, m_SurfaceReflection, true))))
                return E_FAIL;
        }
        if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE &&
            (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceNormalPath, false, m_SurfaceNormal, true)) ||
             FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceSpecularPath, m_Surface.specularSRGB, m_SurfaceSpecular, true))))
        {
            OutputDebugStringA(("[CMaterial] Source opaque inputs failed: " + m_strName + "\n").c_str());
            return E_FAIL;
        }
        if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE)
        {
            if (m_Surface.overlaySeparateSpecular && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews,
                material.surfaceSpecularPath, m_Surface.specularSRGB, m_SurfaceSpecular, true))) return E_FAIL;
            if (((m_Surface.sourceOverlayFlags & 1u) != 0u && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceNormalPath, false, m_SurfaceNormal, true))) ||
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.overlayDiffusePath, m_Surface.overlaySRGB, m_SurfaceOverlayDiffuse, true)) ||
                ((m_Surface.sourceOverlayFlags & 2u) != 0u && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.overlayNormalPath, false, m_SurfaceOverlayNormal, true))) ||
                ((m_Surface.sourceOverlayFlags & 32u) != 0u && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.detailNormalPath, false, m_SurfaceDetailNormal, true)))) return E_FAIL;
            for (const auto& input : { m_SurfaceDiffuse, m_SurfaceNormal, m_SurfaceOverlayDiffuse, m_SurfaceOverlayNormal })
            {
                if (!input) continue;
                D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
                input->GetDesc(&desc);
                if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
            }
        }
        if (m_Surface.hasStaticShadow)
        {
            if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.staticShadowPath, false, m_StaticShadow, true))) return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC shadow{}; m_StaticShadow->GetDesc(&shadow);
            if (shadow.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D || shadow.Format != DXGI_FORMAT_R8_UNORM) return E_INVALIDARG;
        }
        if (m_Surface.hasBakedLighting)
        {
            if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.bakedAveragePath, m_Surface.bakedLightingSRGB, m_BakedAverage, true)) ||
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.bakedDirectionalPath, m_Surface.bakedLightingSRGB, m_BakedDirectional, true)))
                return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC average{}, directional{};
            m_BakedAverage->GetDesc(&average); m_BakedDirectional->GetDesc(&directional);
            if (average.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
                directional.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
        }
        if (m_Surface.hasEnvironmentCube)
        {
            if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.environmentCubePath, false, m_EnvironmentCube, true)) ||
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.environmentBRDFPath, false, m_EnvironmentBRDF, true)))
                return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC cube{};
            m_EnvironmentCube->GetDesc(&cube);
            D3D11_SHADER_RESOURCE_VIEW_DESC brdf{};
            m_EnvironmentBRDF->GetDesc(&brdf);
            if (cube.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBE ||
                brdf.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
            if (m_Surface.hasSourceIndirect)
            {
                if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.sourceIndirectBRDFPath,
                    false, m_SourceIndirectBRDF, true)) ||
                    FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.sourceIndirectCubePath,
                        false, m_SourceIndirectCube, true))) return E_FAIL;
                D3D11_SHADER_RESOURCE_VIEW_DESC sourceCube{};
                m_SourceIndirectCube->GetDesc(&sourceCube);
                if (sourceCube.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBE) return E_INVALIDARG;
                D3D11_SHADER_RESOURCE_VIEW_DESC sourceBRDF{};
                m_SourceIndirectBRDF->GetDesc(&sourceBRDF);
                if (sourceBRDF.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
                    sourceBRDF.Format != DXGI_FORMAT_R16G16_UNORM) return E_INVALIDARG;
            }
        }
		if (MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION == m_Surface.family &&
			FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.specularPath,
				m_Surface.specularSRGB, m_SurfaceSpecular, true)))
		{
			OutputDebugStringA(("[CMaterial] Source specular load failed: " +
				m_strName + "\n").c_str());
			return E_FAIL;
		}
	}
	return S_OK;
}

bool_t CMaterial::Has_Texture(aiTextureType eType, uint32_t iTextureIndex) const
{
	return eType < AI_TEXTURE_TYPE_MAX &&
		iTextureIndex < m_Textures[eType].size();
}

namespace
{
	/* One key per (slot, index) pair; the index is small in every shipped material. */
	constexpr uint32_t Texture_OverrideKey(
		const aiTextureType eType, const uint32_t iTextureIndex)
	{
		return (static_cast<uint32_t>(eType) << 8) | (iTextureIndex & 0xFFu);
	}
}

void CMaterial::Set_TextureOverride(
	const aiTextureType eType,
	const uint32_t iTextureIndex,
	ComPtr<ID3D11ShaderResourceView> pTexture)
{
	if (eType >= AI_TEXTURE_TYPE_MAX)
		return;
	const uint32_t key = Texture_OverrideKey(eType, iTextureIndex);
	if (nullptr == pTexture)
		m_TextureOverrides.erase(key);
	else
		m_TextureOverrides[key] = pTexture;
}

void CMaterial::Clear_TextureOverrides()
{
	m_TextureOverrides.clear();
}

bool_t CMaterial::Set_SourceCharacterConstants(
	const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters)
{
	if (!Has_SourceCharacterProgram() ||
		parameters.program != m_Surface.sourceCharacter.program ||
		parameters.baseTextureMask != m_Surface.sourceCharacter.baseTextureMask ||
		parameters.lightTextureMask != m_Surface.sourceCharacter.lightTextureMask)
	{
		return false;
	}
	/* The same bound the loader holds these to. A slider that produced a NaN would otherwise
	spread through the whole lighting row rather than showing up as one wrong value. */
	for (const auto* constants : { &parameters.baseConstants, &parameters.lightConstants })
		for (const auto& value : *constants)
			for (const f32_t scalar : { value.x, value.y, value.z, value.w })
				if (!std::isfinite(scalar) || std::abs(scalar) > 1000000.f)
					return false;
	m_Surface.sourceCharacter = parameters;
	return true;
}

bool_t CMaterial::Set_SourceCharacterTextureOverride(
	const uint32_t iRegister, ComPtr<ID3D11ShaderResourceView> pTexture)
{
	const uint32_t mask = m_Surface.sourceCharacter.baseTextureMask |
		m_Surface.sourceCharacter.lightTextureMask;
	if (!Has_SourceCharacterProgram() || iRegister >= SOURCE_CHARACTER_TEXTURE_COUNT ||
		0u == (mask & (1u << iRegister)))
	{
		return false;
	}
	if (nullptr == pTexture)
		m_SourceCharacterTextureOverrides.erase(iRegister);
	else
		m_SourceCharacterTextureOverrides[iRegister] = pTexture;
	return true;
}

void CMaterial::Clear_SourceCharacterOverrides()
{
	m_SourceCharacterTextureOverrides.clear();
	if (Has_SourceCharacterProgram())
		m_Surface.sourceCharacter = m_AuthoredSourceCharacter;
}

void CMaterial::Set_DyeColorOverride(
	const float4_t& vDiffuse, const float4_t& vRegionA)
{
	if (!m_ColorTint.isEnabled)
		return;
	m_ColorTint.vDiffuse = vDiffuse;
	m_ColorTint.vRegionA = vRegionA;
}

void CMaterial::Clear_DyeColorOverride()
{
	m_ColorTint = m_AuthoredColorTint;
}

void CMaterial::Set_DyeTwoTone(const f32_t fStrength, const f32_t fRange)
{
	if (!m_ColorTint.isEnabled || !m_ColorTint.isHairMask)
		return;
	m_ColorTint.vDiffuse.w = isfinite(fStrength) ? max(0.f, min(1.f, fStrength)) : 0.f;
	m_ColorTint.vRegionA.w = isfinite(fRange) ? max(0.f, min(1.f, fRange)) : 0.f;
}

HRESULT CMaterial::Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, aiTextureType eType, uint32_t iTextureIndex)
{
	if (eType >= AI_TEXTURE_TYPE_MAX)
		return E_FAIL;
	/* An override may exist for a slot the authored material left empty -- a face that never
	shipped a decal still has to be able to wear one. */
	if (const auto found = m_TextureOverrides.find(Texture_OverrideKey(eType, iTextureIndex));
		found != m_TextureOverrides.end())
	{
		if (aiTextureType_DIFFUSE == eType)
		{
			const uint32_t disabled = 0u;
			pShader->Bind_RawValue("g_SurfaceProgram", &disabled, sizeof(disabled));
			pShader->Bind_RawValue("g_HasSurfaceDefinition", &disabled, sizeof(disabled));
		}
		return pShader->Bind_Texture(pConstantName, found->second);
	}
	if (iTextureIndex >= m_Textures[eType].size())
		return E_FAIL;

	if (aiTextureType_DIFFUSE == eType)
	{
		/* Shared shader instances must not inherit the previous surface program.
		   Shaders without this optional contract simply reject the variable. */
		const uint32_t disabled = 0u;
		pShader->Bind_RawValue("g_SurfaceProgram", &disabled, sizeof(disabled));
        pShader->Bind_RawValue("g_SourceCharacterProgram", &disabled, sizeof(disabled));
        pShader->Bind_RawValue("g_SourceCharacterRow", &disabled, sizeof(disabled));
		pShader->Bind_RawValue("g_HasSurfaceDefinition", &disabled, sizeof(disabled));
		pShader->Bind_RawValue("g_HasSurfaceEmissive", &disabled, sizeof(disabled));
		pShader->Bind_RawValue("g_DiffuseMirrorU", &m_iDiffuseMirrorU, sizeof(m_iDiffuseMirrorU));
	}
	return pShader->Bind_Texture(pConstantName, m_Textures[eType][iTextureIndex]);
}

namespace
{
    // Populated only by actual mesh draws on the rendering thread. Keeping a
    // shared reference until light accumulation finishes also closes teardown.
    thread_local std::vector<std::shared_ptr<CMaterial>> g_SourceCharacterFrame;
    thread_local std::unordered_map<const CMaterial*, uint32_t> g_SourceCharacterRows;
    // The row lives in the R32_FLOAT depth target, not an eight-bit material index.
    // Every positive integer through 2^24 is exactly representable there.
    constexpr uint32_t SOURCE_CHARACTER_MAX_EXACT_ROW =
        1u << std::numeric_limits<float>::digits;
    static_assert(std::numeric_limits<float>::is_iec559 && std::numeric_limits<float>::digits == 24);
    thread_local float g_SourceCharacterTime = 0.f;
}

void CMaterial::Reset_SourceCharacterFrame(float presentationTime)
{
    g_SourceCharacterRows.clear();
    g_SourceCharacterFrame.clear();
    g_SourceCharacterTime = presentationTime;
}

uint32_t CMaterial::Get_SourceCharacterFrameCount()
{
    return static_cast<uint32_t>(g_SourceCharacterFrame.size());
}

HRESULT CMaterial::Bind_SourceCharacterInputs(shared_ptr<CShader> shader,
    bool lightPass, uint32_t row)
{
    if (!shader) return E_INVALIDARG;
    const auto& source = m_Surface.sourceCharacter;
    const uint32_t hasBaked = m_Surface.hasBakedLighting ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_SourceMapMonsterBakedEnabled", &hasBaked, sizeof(hasBaked)))) return E_FAIL;
    if (!lightPass)
    {
        if (((source.program >= 80u && source.program <= 83u) ||
            (source.program >= 214u && source.program <= 234u) || source.program == 237u) && FAILED(Bind_StaticShadow(shader))) return E_FAIL;
        if (hasBaked && (FAILED(shader->Bind_Texture("g_SourceMapMonsterAverageTexture", m_BakedAverage)) ||
            FAILED(shader->Bind_Texture("g_SourceMapMonsterDirectionalTexture", m_BakedDirectional)))) return E_FAIL;
        const auto environment = CGameInstance::Get().Get_RenderEnvironment();
        const uint32_t enabled = environment.pCube ? 1u : 0u;
        if (FAILED(shader->Bind_RawValue("g_SourceCharacterEnvironmentEnabled", &enabled, sizeof(enabled))) ||
            FAILED(shader->Bind_RawValue("g_SourceCharacterEnvironmentColor", &environment.vColor, sizeof(float4_t))) ||
            FAILED(shader->Bind_RawValue("g_SourceCharacterEnvironmentRotation", &environment.vRotationIntensity, sizeof(float4_t))) ||
            FAILED(shader->Bind_Texture("g_SourceCharacterEnvironmentCube", environment.pCube))) return E_FAIL;
    }
    const bool forward = (source.program >= 38u && source.program <= 63u) || source.program == 209u ||
        (source.program >= 224u && source.program <= 226u) || source.program == 237u;
    if (!lightPass && forward && FAILED(shader->Bind_RawValue("g_SourceCharacterLightConstants",
        source.lightConstants.data(), sizeof(source.lightConstants)))) return E_FAIL;
    const auto& constants = lightPass ? source.lightConstants : source.baseConstants;
    const uint32_t mask = lightPass ? source.lightTextureMask :
        source.baseTextureMask | (forward ? source.lightTextureMask : 0u);
    if (FAILED(shader->Bind_RawValue("g_SourceCharacterTime", &g_SourceCharacterTime, sizeof(g_SourceCharacterTime))) ||
        FAILED(shader->Bind_RawValue("g_SourceCharacterProgram", &source.program, sizeof(source.program))) ||
        FAILED(shader->Bind_RawValue("g_SourceCharacterRow", &row, sizeof(row))) ||
        FAILED(shader->Bind_RawValue(lightPass ? "g_SourceCharacterLightConstants" :
            "g_SourceCharacterBaseConstants", constants.data(), sizeof(constants)))) return E_FAIL;
    static constexpr const char_t* textureNames[] =
    {
        "g_SourceCharacterTexture0", "g_SourceCharacterTexture1", "g_SourceCharacterTexture2", "g_SourceCharacterTexture3",
        "g_SourceCharacterTexture4", "g_SourceCharacterTexture5", "g_SourceCharacterTexture6", "g_SourceCharacterTexture7",
        "g_SourceCharacterTexture8", "g_SourceCharacterTexture9", "g_SourceCharacterTexture10", "g_SourceCharacterTexture11",
        "g_SourceCharacterTexture12", "g_SourceCharacterTexture13", "g_SourceCharacterTexture14", "g_SourceCharacterTexture15"
    };
    static_assert(std::size(textureNames) == SOURCE_CHARACTER_TEXTURE_COUNT);
    for (uint32_t index = 0u; index < SOURCE_CHARACTER_TEXTURE_COUNT; ++index)
    {
        if ((mask & (1u << index)) == 0u) continue;
        /* A creation choice repaints the register on the clone; the authored view stays put. */
        const auto repainted = m_SourceCharacterTextureOverrides.find(index);
        if (FAILED(shader->Bind_Texture(textureNames[index],
            repainted != m_SourceCharacterTextureOverrides.end() ?
            repainted->second : m_SourceCharacterTextures[index]))) return E_FAIL;
    }
    return S_OK;
}

HRESULT CMaterial::Bind_SourceCharacter(shared_ptr<CShader> shader)
{
    if (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_CHARACTER) return S_FALSE;
    const uint32_t program = m_Surface.sourceCharacter.program;
    if ((program >= 33u && program <= 65u) || program == 209u ||
        (program >= 224u && program <= 226u) || program == 237u)
        return Bind_SourceCharacterInputs(shader, false, 0u);
    const auto found = g_SourceCharacterRows.find(this);
    if (found != g_SourceCharacterRows.end())
        return Bind_SourceCharacterInputs(shader, false, found->second);
    if (g_SourceCharacterFrame.size() >= SOURCE_CHARACTER_MAX_EXACT_ROW) return E_BOUNDS;
    // Row IDs are ephemeral render indices, never serialized asset IDs.
    const uint32_t row = static_cast<uint32_t>(g_SourceCharacterFrame.size()) + 1u;
    const HRESULT result = Bind_SourceCharacterInputs(shader, false, row);
    if (FAILED(result)) return result;
    g_SourceCharacterFrame.push_back(shared_from_this());
    g_SourceCharacterRows.emplace(this, row);
    return S_OK;
}

HRESULT CMaterial::Bind_SourceCharacterForwardLight(shared_ptr<CShader> shader)
{
    if (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_CHARACTER) return E_INVALIDARG;
    return Bind_SourceCharacterInputs(shader, true, 0u);
}

HRESULT CMaterial::Bind_SourceCharacterLight(shared_ptr<CShader> shader, uint32_t index)
{
    if (index >= g_SourceCharacterFrame.size()) return E_INVALIDARG;
    return g_SourceCharacterFrame[index]->Bind_SourceCharacterInputs(shader, true, index + 1u);
}

HRESULT CMaterial::Bind_StaticShadow(shared_ptr<CShader> shader)
{
    if (!shader) return E_INVALIDARG;
    const uint32_t enabled = m_Surface.hasStaticShadow ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_HasStaticShadow", &enabled, sizeof(enabled))) ||
        FAILED(shader->Bind_RawValue("g_StaticShadowChannel", &m_Surface.staticShadowChannel, sizeof(m_Surface.staticShadowChannel))) ||
        FAILED(shader->Bind_RawValue("g_StaticShadowTransfer", &m_Surface.staticShadowTransfer, sizeof(m_Surface.staticShadowTransfer))) ||
        (enabled && FAILED(shader->Bind_Texture("g_StaticShadowTexture", m_StaticShadow)))) return E_FAIL;
    return S_OK;
}

HRESULT CMaterial::Bind_SourceSpecialSurface(shared_ptr<CShader> shader)
{
    if (!shader || m_Surface.family < MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE ||
        m_Surface.family > MODEL_SURFACE_FAMILY::SOURCE_WET_OPAQUE) return E_INVALIDARG;
    const auto& special = m_Surface.sourceSpecial;
    const bool ice = m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE;
    const bool blend = m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_VERTEXBLEND_OPAQUE;
    if (FAILED(shader->Bind_RawValue("g_SourceSpecialFlags", &special.flags, sizeof(special.flags)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceNormalTiling", &special.normalTiling, sizeof(special.normalTiling)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceIceCoreColor", &special.iceCoreColor, sizeof(special.iceCoreColor)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceIceOuterColor", &special.iceOuterColor, sizeof(special.iceOuterColor)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceIceBlend", &special.iceBlend, sizeof(special.iceBlend)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceIceBumpOffset", &special.iceBumpOffset, sizeof(special.iceBumpOffset)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceWetParameters", &special.wetParameters, sizeof(special.wetParameters)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceWetSpecularPower", &special.wetSpecularPower, sizeof(special.wetSpecularPower)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceBlendSharpness", &special.blendSharpness, sizeof(special.blendSharpness)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceBlendDiffuse", special.blendDiffuse.data(), sizeof(special.blendDiffuse)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceBlendSpecular", special.blendSpecular.data(), sizeof(special.blendSpecular)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceBlendLayers", special.blendLayers.data(), sizeof(special.blendLayers)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &m_Surface.uvTiling, sizeof(m_Surface.uvTiling)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalIntensity", &m_Surface.detailNormalIntensity, sizeof(m_Surface.detailNormalIntensity)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalTiling", &m_Surface.detailNormalTiling, sizeof(m_Surface.detailNormalTiling)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceBgRimlight", &m_Surface.sourceBgRimlight, sizeof(m_Surface.sourceBgRimlight)))) return E_FAIL;
    if (FAILED(shader->Bind_Texture("g_NormalTexture", m_SurfaceNormal)) ||
        (!blend && FAILED(shader->Bind_Texture("g_ReflectionTexture", m_SurfaceReflection))) ||
        (!blend && (!ice || (special.flags & 2u)) && FAILED(shader->Bind_Texture("g_SpecularTexture", m_SurfaceSpecular))) ||
        ((blend || (ice && (special.flags & 1u))) && FAILED(shader->Bind_Texture("g_DetailNormalTexture", m_SurfaceDetailNormal))) ||
        (blend && (FAILED(shader->Bind_Texture("g_SurfaceOverlayDiffuseTexture", m_SurfaceOverlayDiffuse)) ||
            FAILED(shader->Bind_Texture("g_SurfaceOverlayNormalTexture", m_SurfaceOverlayNormal)))) ||
        (ice && FAILED(shader->Bind_Texture("g_SourceSpecialMaskTexture", m_SourceSpecialMask))) ||
        (blend && (special.flags & 1u) && (FAILED(shader->Bind_Texture("g_SourceBlendDiffuseGTexture", m_SourceBlendDiffuseG)) ||
            FAILED(shader->Bind_Texture("g_SourceBlendNormalGTexture", m_SourceBlendNormalG)))) ||
        (blend && (special.flags & 2u) && (FAILED(shader->Bind_Texture("g_SourceBlendDiffuseBTexture", m_SourceBlendDiffuseB)) ||
            FAILED(shader->Bind_Texture("g_SourceBlendNormalBTexture", m_SourceBlendNormalB))))) return E_FAIL;
    return S_OK;
}

HRESULT CMaterial::Bind_SurfaceLighting(shared_ptr<CShader> shader)
{
    if (!shader) return E_INVALIDARG;
    if (FAILED(Bind_StaticShadow(shader))) return E_FAIL;
    if (m_Surface.hasBakedLighting &&
        (FAILED(shader->Bind_Texture("g_BakedAverageTexture", m_BakedAverage)) ||
         FAILED(shader->Bind_Texture("g_BakedDirectionalTexture", m_BakedDirectional)))) return E_FAIL;
    const bool sourceIndirect = m_Surface.hasSourceIndirect &&
        CGameInstance::Get().Get_RenderEnvironment().bUseSourcePBRIndirect;
    const auto& lookup = sourceIndirect ? m_SourceIndirectBRDF : m_EnvironmentBRDF;
    const auto& cube = sourceIndirect ? m_SourceIndirectCube : m_EnvironmentCube;
    if (m_Surface.hasEnvironmentCube && (m_Surface.environmentLegacyEnabled || sourceIndirect) &&
        (FAILED(shader->Bind_Texture("g_EnvironmentCubeTexture", cube)) ||
         FAILED(shader->Bind_Texture("g_EnvironmentBRDFLookupTexture", lookup)))) return E_FAIL;
    return S_OK;
}

HRESULT CMaterial::Bind_SurfaceTexture(shared_ptr<CShader> pShader,
	const char_t* pConstantName, aiTextureType eType)
{
	if (nullptr == pShader || nullptr == pConstantName)
		return E_INVALIDARG;
    if (eType == aiTextureType_DIFFUSE)
    {
        const uint32_t zero = 0u;
        for (const auto* name : { "g_SourceCharacterProgram", "g_SourceCharacterRow" })
            (void)pShader->Bind_RawValue(name, &zero, sizeof(zero));
    }
	const ComPtr<ID3D11ShaderResourceView>* texture = nullptr;
	/* The surface program reads slot 0 of the same type, so one override covers both paths. */
	if (const auto found = m_TextureOverrides.find(Texture_OverrideKey(eType, 0u));
		found != m_TextureOverrides.end())
	{
		return pShader->Bind_Texture(pConstantName, found->second);
	}
	switch (eType)
	{
	case aiTextureType_DIFFUSE: texture = &m_SurfaceDiffuse; break;
	case aiTextureType_SPECULAR: texture = &m_SurfaceSpecular; break;
	case aiTextureType_REFLECTION: texture = &m_SurfaceReflection; break;
	case aiTextureType_NORMALS: texture = &m_SurfaceNormal; break;
	case aiTextureType_HEIGHT: texture = &m_SurfaceDetailNormal; break;
	case aiTextureType_UNKNOWN: texture = &m_SurfaceORM; break;
	case aiTextureType_EMISSIVE: texture = &m_SurfaceEmissive; break;
    case aiTextureType_TRANSMISSION: texture = &m_SourceFoliageMask; break;
    // Surface-only roles, separate from the legacy color-mask texture array.
    case aiTextureType_BASE_COLOR: texture = &m_SurfaceOverlayDiffuse; break;
    case aiTextureType_NORMAL_CAMERA: texture = &m_SurfaceOverlayNormal; break;
	default: return E_INVALIDARG;
	}
	// The material owns the SRV throughout this call; borrowing avoids a
	// redundant COM AddRef/Release for every surface texture of every draw.
	return texture && *texture ? pShader->Bind_Texture(pConstantName, *texture) : E_FAIL;
}

shared_ptr<CMaterial> CMaterial::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const aiMaterial* pAIMaterial, const char_t* pModelFilePath)
{
	auto pInstance = shared_ptr<CMaterial>(new CMaterial(pDevice, pContext));

	if (FAILED(pInstance->Initialize(pAIMaterial, pModelFilePath)))
	{
		OutputDebugStringA("[CMaterial] Assimp material initialization failed.\n");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CMaterial> CMaterial::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext, const MODEL_MATERIAL_DATA& material)
{
	auto pInstance = shared_ptr<CMaterial>(new CMaterial(pDevice, pContext));
	if (FAILED(pInstance->Initialize(material)))
	{
		OutputDebugStringA("[CMaterial] Binary material initialization failed.\n");
		return nullptr;
	}
	return pInstance;
}

```

### C:/Users/user/Desktop/LostArk/Engine/Private/Channel.cpp

```cpp
#include "Channel.h"
#pragma push_macro("new")
#undef new
#include "Assimp/scene.h"
#pragma pop_macro("new")
#include "Engine_AnimationTypes.h"
#include "BinaryAsset/ModelAssetData.h"
#include "Bone.h"

#include <algorithm>
#include <cmath>

namespace
{
	template<typename TKey>
	const TKey* FindRightKey(const vector<TKey>& keys, const f32_t time,
		uint32_t* const leftIndex)
	{
		const TKey* const begin = keys.data();
		const size_t count = keys.size();
		if (leftIndex && count > 1u && static_cast<size_t>(*leftIndex) < count - 1u)
		{
			const TKey* left = begin + *leftIndex;
			if (left->timeTicks <= time)
			{
				// Normal playback crosses only a few keys per frame. Bound the
				// cursor walk so random seeks keep logarithmic worst-case work.
				for (size_t step = 0u; step < 4u && left + 1 < begin + count; ++step, ++left)
				{
					if (time < (left + 1)->timeTicks)
					{
						*leftIndex = static_cast<uint32_t>(left - begin);
						return left + 1;
					}
				}
			}
		}
		// A clone owns its cursors. Channels and their compact tracks stay immutable.
		// Random seeks and reversed playback use a bounded search of the same keys.
		const TKey* const right = upper_bound(begin, begin + count, time,
			[](f32_t value, const TKey& key) { return value < key.timeTicks; });
		if (leftIndex)
			*leftIndex = static_cast<uint32_t>(right - begin - 1);
		return right;
	}

	float3_t SampleVector(const vector<MODEL_VECTOR_KEY_DATA>& keys,
		f32_t time, const float3_t& fallback, uint32_t* leftIndex = nullptr)
	{
		if (keys.empty())
			return fallback;
		if (time <= keys.front().timeTicks)
			return keys.front().value;
		if (time >= keys.back().timeTicks)
			return keys.back().value;

		const auto right = FindRightKey(keys, time, leftIndex);
		const auto left = right - 1;
		const f32_t span = right->timeTicks - left->timeTicks;
		const f32_t ratio = span > 0.f ? (time - left->timeTicks) / span : 0.f;
		float3_t result{};
		XMStoreFloat3(&result, XMVectorLerp(
			XMLoadFloat3(&left->value), XMLoadFloat3(&right->value), ratio));
		return result;
	}

	float4_t SampleQuaternion(const vector<MODEL_QUAT_KEY_DATA>& keys,
		f32_t time, uint32_t* leftIndex = nullptr)
	{
		if (keys.empty())
			return float4_t(0.f, 0.f, 0.f, 1.f);
		if (time <= keys.front().timeTicks)
			return keys.front().value;
		if (time >= keys.back().timeTicks)
			return keys.back().value;

		const auto right = FindRightKey(keys, time, leftIndex);
		const auto left = right - 1;
		const f32_t span = right->timeTicks - left->timeTicks;
		const f32_t ratio = span > 0.f ? (time - left->timeTicks) / span : 0.f;
		float4_t result{};
		XMStoreFloat4(&result, XMQuaternionSlerp(
			XMLoadFloat4(&left->value), XMLoadFloat4(&right->value), ratio));
		return result;
	}
}

CChannel::CChannel()
{
}

CChannel::~CChannel()
{
}

HRESULT CChannel::Initialize(const aiNodeAnim* pAIChannel, const vector<shared_ptr<class CBone>>& Bones)
{
	strcpy_s(m_szName, pAIChannel->mNodeName.C_Str());

	m_iNumKeyFrames = max(pAIChannel->mNumScalingKeys, pAIChannel->mNumRotationKeys);
	m_iNumKeyFrames = max(m_iNumKeyFrames, pAIChannel->mNumPositionKeys);

	float3_t		vScale{};
	float4_t		vRotation{};
	float3_t		vTranslation{};


	for (uint32_t i = 0; i < m_iNumKeyFrames; i++)
	{
		KEYFRAME			KeyFrame{};

		if (i < pAIChannel->mNumScalingKeys)
		{
			memcpy(&vScale, &pAIChannel->mScalingKeys[i].mValue, sizeof(float3_t));
			KeyFrame.fTrackPosition = pAIChannel->mScalingKeys[i].mTime;
		}
		if (i < pAIChannel->mNumRotationKeys)
		{
			/*memcpy(&vRotation, &pAIChannel->mRotationKeys[i].mValue, sizeof(float4_t));*/
			vRotation.x = pAIChannel->mRotationKeys[i].mValue.x;
			vRotation.y = pAIChannel->mRotationKeys[i].mValue.y;
			vRotation.z = pAIChannel->mRotationKeys[i].mValue.z;
			vRotation.w = pAIChannel->mRotationKeys[i].mValue.w;

			KeyFrame.fTrackPosition = pAIChannel->mRotationKeys[i].mTime;
		}
		if (i < pAIChannel->mNumPositionKeys)
		{
			memcpy(&vTranslation, &pAIChannel->mPositionKeys[i].mValue, sizeof(float3_t));
			KeyFrame.fTrackPosition = pAIChannel->mPositionKeys[i].mTime;
		}

		KeyFrame.vScale = vScale;
		KeyFrame.vRotation = vRotation;
		KeyFrame.vTranslation = vTranslation;

		m_KeyFrames.push_back(KeyFrame);
	}

	auto	iter = find_if(Bones.begin(), Bones.end(), [&](shared_ptr<class CBone> pBone)->bool_t {
			++m_iBoneIndex;
			return pBone->Compare_Name(m_szName);
		});

	if (iter == Bones.end())
		return E_FAIL;

	return S_OK;
}

HRESULT CChannel::Initialize(const MODEL_ANIMATION_CHANNEL_DATA& channel,
	const vector<shared_ptr<class CBone>>& Bones)
{
	if (channel.resolvedBoneIndex < 0 ||
		channel.resolvedBoneIndex >= static_cast<int32_t>(Bones.size()))
		return E_FAIL;
	m_iBoneIndex = channel.resolvedBoneIndex;
	if (channel.positionKeys.empty() && channel.rotationKeys.empty() &&
		channel.scaleKeys.empty())
		return E_FAIL;

	// Preserve the three compact WAnimation tracks. Expanding their union into
	// full transform keyframes multiplies memory for long character packages and
	// made the 154-clip DimensionMaster body exhaust memory during level loading.
	m_PositionKeys = channel.positionKeys;
	m_RotationKeys = channel.rotationKeys;
	m_ScaleKeys = channel.scaleKeys;
	m_iNumKeyFrames = static_cast<uint32_t>((max)({
		m_PositionKeys.size(), m_RotationKeys.size(), m_ScaleKeys.size() }));
	m_bUsesSeparateTracks = true;
	return S_OK;
}

void CChannel::Update_TransformationMatrix(f32_t fCurrentTrackPosition, const vector<shared_ptr<class CBone>>& Bones, uint32_t* pLeftKeyFrameIndex, std::array<uint32_t, 3>* pSeparateTrackIndices)
{
	if (m_bUsesSeparateTracks)
	{
		const float3_t scale = SampleVector(
			m_ScaleKeys, fCurrentTrackPosition,
			float3_t(1.f, 1.f, 1.f), pSeparateTrackIndices ? &(*pSeparateTrackIndices)[0] : nullptr);
		const float4_t rotation = SampleQuaternion(
			m_RotationKeys, fCurrentTrackPosition,
			pSeparateTrackIndices ? &(*pSeparateTrackIndices)[1] : nullptr);
		const float3_t translation = SampleVector(
			m_PositionKeys, fCurrentTrackPosition,
			float3_t(0.f, 0.f, 0.f), pSeparateTrackIndices ? &(*pSeparateTrackIndices)[2] : nullptr);

		const matrix_t boneTranslationMatrix =
			XMMatrixAffineTransformation(
				XMLoadFloat3(&scale),
				XMVectorZero(),
				XMLoadFloat4(&rotation),
				XMVectorSetW(XMLoadFloat3(&translation), 1.f));
		Bones[m_iBoneIndex]->Update_TransformationMatrix(
			boneTranslationMatrix);
		return;
	}

	if (m_KeyFrames.empty() || nullptr == pLeftKeyFrameIndex)
		return;

	if (0.f == fCurrentTrackPosition)
		(*pLeftKeyFrameIndex) = 0;


	KEYFRAME		LastKeyFrame = m_KeyFrames.back();

	vector_t		vScale{}, vRotation{}, vTranslation{};

	if (1 == m_KeyFrames.size() || fCurrentTrackPosition >= LastKeyFrame.fTrackPosition) /* 선형보간이 필요 없는 상태 */
	{
		vScale = XMLoadFloat3(&LastKeyFrame.vScale);
		vRotation = XMLoadFloat4(&LastKeyFrame.vRotation);
		vTranslation = XMVectorSetW(XMLoadFloat3(&LastKeyFrame.vTranslation), 1.f);
	}

	else /* 선형보간이 필요한 상태 */
	{
		(*pLeftKeyFrameIndex) = (min)(*pLeftKeyFrameIndex,
			static_cast<uint32_t>(m_KeyFrames.size() - 2));
		while ((*pLeftKeyFrameIndex) + 1 < m_KeyFrames.size() - 1 &&
			fCurrentTrackPosition >= m_KeyFrames[(*pLeftKeyFrameIndex) + 1].fTrackPosition)
			++(*pLeftKeyFrameIndex);

		const f32_t fSpan = m_KeyFrames[(*pLeftKeyFrameIndex) + 1].fTrackPosition -
			m_KeyFrames[(*pLeftKeyFrameIndex)].fTrackPosition;
		f32_t		fRatio = fSpan > 0.f ?
			(fCurrentTrackPosition - m_KeyFrames[(*pLeftKeyFrameIndex)].fTrackPosition) / fSpan : 0.f;

		vector_t	vLeftScale = XMLoadFloat3(&m_KeyFrames[(*pLeftKeyFrameIndex)].vScale);
		vector_t	vRightScale = XMLoadFloat3(&m_KeyFrames[(*pLeftKeyFrameIndex) + 1].vScale);
		vector_t	vLeftRotation = XMLoadFloat4(&m_KeyFrames[(*pLeftKeyFrameIndex)].vRotation); 
		vector_t	vRightRotation = XMLoadFloat4(&m_KeyFrames[(*pLeftKeyFrameIndex) + 1].vRotation);
		vector_t	vLeftTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[(*pLeftKeyFrameIndex)].vTranslation), 1.f);
		vector_t	vRightTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[(*pLeftKeyFrameIndex) + 1].vTranslation), 1.f);

		vScale = XMVectorLerp(vLeftScale, vRightScale, fRatio);
		vRotation = XMQuaternionSlerp(vLeftRotation, vRightRotation, fRatio);
		vTranslation = XMVectorLerp(vLeftTranslation, vRightTranslation, fRatio);
	}

	// matrix_t	BoneTranslationMatrix = XMMatrixScaling() * XMMatrixRotationQuaternion() * XMMatrixTranslation();
	matrix_t	BoneTranslationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vTranslation);

	Bones[m_iBoneIndex]->Update_TransformationMatrix(BoneTranslationMatrix);
}

bool_t CChannel::Sample_TransformationMatrix(
	const f32_t fTrackPosition,
	uint32_t& iOutBoneIndex,
	float4x4_t& OutTransformationMatrix) const
{
	if (!std::isfinite(fTrackPosition) || m_iBoneIndex < 0)
		return false;

	matrix_t Transformation;
	if (m_bUsesSeparateTracks)
	{
		const float3_t Scale = SampleVector(
			m_ScaleKeys, fTrackPosition, float3_t(1.f, 1.f, 1.f));
		const float4_t Rotation = SampleQuaternion(
			m_RotationKeys, fTrackPosition);
		const float3_t Translation = SampleVector(
			m_PositionKeys, fTrackPosition, float3_t(0.f, 0.f, 0.f));
		Transformation = XMMatrixAffineTransformation(
			XMLoadFloat3(&Scale),
			XMVectorZero(),
			XMLoadFloat4(&Rotation),
			XMVectorSetW(XMLoadFloat3(&Translation), 1.f));
	}
	else
	{
		if (m_KeyFrames.empty())
			return false;

		const KEYFRAME& LastKeyFrame = m_KeyFrames.back();
		vector_t Scale;
		vector_t Rotation;
		vector_t Translation;
		if (1u == m_KeyFrames.size() ||
			fTrackPosition >= LastKeyFrame.fTrackPosition)
		{
			Scale = XMLoadFloat3(&LastKeyFrame.vScale);
			Rotation = XMLoadFloat4(&LastKeyFrame.vRotation);
			Translation = XMVectorSetW(
				XMLoadFloat3(&LastKeyFrame.vTranslation), 1.f);
		}
		else
		{
			size_t iLeftKeyFrame = 0u;
			while (iLeftKeyFrame + 1u < m_KeyFrames.size() - 1u &&
				fTrackPosition >=
					m_KeyFrames[iLeftKeyFrame + 1u].fTrackPosition)
			{
				++iLeftKeyFrame;
			}
			const KEYFRAME& Left = m_KeyFrames[iLeftKeyFrame];
			const KEYFRAME& Right = m_KeyFrames[iLeftKeyFrame + 1u];
			const f32_t fSpan =
				Right.fTrackPosition - Left.fTrackPosition;
			const f32_t fRatio = fSpan > 0.f ?
				(fTrackPosition - Left.fTrackPosition) / fSpan : 0.f;
			Scale = XMVectorLerp(
				XMLoadFloat3(&Left.vScale),
				XMLoadFloat3(&Right.vScale), fRatio);
			Rotation = XMQuaternionSlerp(
				XMLoadFloat4(&Left.vRotation),
				XMLoadFloat4(&Right.vRotation), fRatio);
			Translation = XMVectorLerp(
				XMVectorSetW(XMLoadFloat3(&Left.vTranslation), 1.f),
				XMVectorSetW(XMLoadFloat3(&Right.vTranslation), 1.f),
				fRatio);
		}
		Transformation = XMMatrixAffineTransformation(
			Scale, XMVectorZero(), Rotation, Translation);
	}

	float4x4_t Staged{};
	XMStoreFloat4x4(&Staged, Transformation);
	iOutBoneIndex = static_cast<uint32_t>(m_iBoneIndex);
	OutTransformationMatrix = Staged;
	return true;
}

shared_ptr<CChannel> CChannel::Create(const aiNodeAnim* pAIChannel, const vector<shared_ptr<class CBone>>& Bones)
{
	auto pInstance = shared_ptr<CChannel>(new CChannel());

	if (FAILED(pInstance->Initialize(pAIChannel, Bones)))
	{
		MSG_BOX("Failed to Created : CChannel");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CChannel> CChannel::Create(const MODEL_ANIMATION_CHANNEL_DATA& channel,
	const vector<shared_ptr<class CBone>>& Bones)
{
	auto pInstance = shared_ptr<CChannel>(new CChannel());
	if (FAILED(pInstance->Initialize(channel, Bones)))
	{
		MSG_BOX("Failed to Created : CChannel");
		return nullptr;
	}
	return pInstance;
}

```

