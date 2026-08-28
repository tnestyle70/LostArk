#include "MapAssetRenderUtils.h"

#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"

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
	std::mutex g_DiagnosticMutex;
	std::ofstream g_DiagnosticStream;
	std::string g_DiagnosticAreaId;
	uint64_t g_CameraMatrixRevision = {};
	bool_t g_HasCameraMatrices = false;
	float4x4_t g_LastView = {};
	float4x4_t g_LastProjection = {};
	Client::MAP_CAMERA_CULL_SNAPSHOT g_LastCameraSnapshot{};
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
	if (nullptr != outFailureReason)
		outFailureReason->clear();
	const float4x4_t* view = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
	const float4x4_t* projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
	if (nullptr == view || nullptr == projection)
		return ReportCullFailure(outFailureReason, "camera matrix unavailable");

	const float4x4_t stagedView = *view;
	const float4x4_t stagedProjection = *projection;
	if (g_HasCameraMatrices &&
		0 == std::memcmp(&g_LastView, &stagedView, sizeof(float4x4_t)) &&
		0 == std::memcmp(&g_LastProjection, &stagedProjection, sizeof(float4x4_t)))
	{
		outSnapshot = g_LastCameraSnapshot;
		return true;
	}
	const uint64_t nextRevision =
		(std::numeric_limits<uint64_t>::max)() == g_CameraMatrixRevision ?
		1u : g_CameraMatrixRevision + 1u;
	MAP_CAMERA_CULL_SNAPSHOT candidate{};
	if (!Build_CameraCullSnapshot(stagedView, stagedProjection, nextRevision,
		candidate, outFailureReason))
	{
		return false;
	}
	g_LastView = candidate.view;
	g_LastProjection = candidate.projection;
	g_CameraMatrixRevision = candidate.revision;
	g_LastCameraSnapshot = candidate;
	g_HasCameraMatrices = true;
	outSnapshot = candidate;
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

HRESULT Client::CMapAssetRenderUtils::Bind_Material(
	const shared_ptr<Engine::CModel>& model,
	const shared_ptr<Engine::CShader>& shader,
	uint32_t meshIndex,
	const MAP_ASSET_RENDER_PROFILE& profile,
	f32_t elapsedTime)
{
	if (nullptr == model ||
		nullptr == shader ||
		meshIndex >= model->Get_NumMeshes())
	{
		return E_INVALIDARG;
	}

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

	const float2_t uvOffset(
		profile.uvSpeed.x * elapsedTime,
		profile.uvSpeed.y * elapsedTime);

	if (FAILED(model->Bind_Material(
		shader,
		"g_DiffuseTexture",
		meshIndex,
		aiTextureType_DIFFUSE)) ||

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

	return S_OK;
}
