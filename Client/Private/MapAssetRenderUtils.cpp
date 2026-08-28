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

	/* Frustum planes are taken from the view-projection matrix instead of from
	   unprojected corner points. A plane built through a far corner ends with
	   d = -dot(normal, corner) evaluated at the far distance, and with Bern's
	   max(2000, span * 8) far plane that subtraction cancels away tens of world
	   units in float32. Measured against exact planes at far 18837 the old
	   left plane sat 63.5 units inside the real one, which rejected a quarter
	   of the geometry the camera was looking straight at. The clip-space
	   half-spaces below never form that difference, so the error stays at
	   0.0003 units. Plane order stays right, left, top, bottom, far, near
	   because the decision and the diagnostics index it. */
	bool_t MakePlanes(fmatrix_t viewProjection, float4_t* planes)
	{
		float4x4_t matrix{};
		XMStoreFloat4x4(&matrix, viewProjection);
		if (!IsFiniteMatrix(matrix))
			return false;

		/* Row-vector convention: clip = world * viewProjection, so a clip
		   component is the dot product with the matching matrix column. */
		const float4_t columnX{ matrix._11, matrix._21, matrix._31, matrix._41 };
		const float4_t columnY{ matrix._12, matrix._22, matrix._32, matrix._42 };
		const float4_t columnZ{ matrix._13, matrix._23, matrix._33, matrix._43 };
		const float4_t columnW{ matrix._14, matrix._24, matrix._34, matrix._44 };

		const auto combine = [](const float4_t& left, const float4_t& right,
			const f32_t scale) -> float4_t
		{
			return float4_t(
				left.x + right.x * scale,
				left.y + right.y * scale,
				left.z + right.z * scale,
				left.w + right.w * scale);
		};

		/* Inside a half-space is w + x >= 0 for left, w - x >= 0 for right and
		   so on, with z >= 0 for near. */
		const float4_t inward[6] = {
			combine(columnW, columnX, -1.f),
			combine(columnW, columnX, 1.f),
			combine(columnW, columnY, -1.f),
			combine(columnW, columnY, 1.f),
			combine(columnW, columnZ, -1.f),
			columnZ,
		};

		for (uint32_t index = 0; index < 6u; ++index)
		{
			/* The decision reads a positive distance as outside, so store the
			   outward form and normalise it into world units. */
			const vector_t plane = XMVectorNegate(XMLoadFloat4(&inward[index]));
			const f32_t length = XMVectorGetX(XMVector3Length(plane));
			if (!std::isfinite(length) || length <= 0.000001f)
				return false;
			XMStoreFloat4(&planes[index], XMVectorScale(plane, 1.f / length));
		}
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

		/* The plane distances alone cannot answer the only question that
		   matters: was the dropped placement actually on screen? Project the
		   bound centre with the same snapshot the decision used, and record the
		   clip-space depth plus the screen-space radius the sphere would have
		   spanned, so an on-screen drop is identifiable by asset. */
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
			const f32_t projectionScaleX = snapshot.projection._11;
			const f32_t projectionScaleY = snapshot.projection._22;
			const f32_t ndcRadiusX = std::abs(clipW) > 0.000001f ?
				decision.baseRadius * projectionScaleX / std::abs(clipW) : 0.f;
			const f32_t ndcRadiusY = std::abs(clipW) > 0.000001f ?
				decision.baseRadius * projectionScaleY / std::abs(clipW) : 0.f;
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

bool_t CMapAssetRenderUtils::Capture_CameraCullSnapshot(
	MAP_CAMERA_CULL_SNAPSHOT& outSnapshot)
{
	const float4x4_t* view =
		CGameInstance::Get().Get_Transform(D3DTS::VIEW);
	const float4x4_t* projection =
		CGameInstance::Get().Get_Transform(D3DTS::PROJ);
	if (nullptr == view || nullptr == projection ||
		!IsFiniteMatrix(*view) || !IsFiniteMatrix(*projection))
	{
		return false;
	}

	if (!g_HasCameraMatrices ||
		0 != std::memcmp(&g_LastView, view, sizeof(float4x4_t)) ||
		0 != std::memcmp(&g_LastProjection, projection, sizeof(float4x4_t)))
	{
		g_CameraMatrixRevision =
			(std::numeric_limits<uint64_t>::max)() == g_CameraMatrixRevision ?
			1u : g_CameraMatrixRevision + 1u;
		g_LastView = *view;
		g_LastProjection = *projection;
		g_HasCameraMatrices = true;
	}

	outSnapshot = {};
	outSnapshot.revision = g_CameraMatrixRevision;
	outSnapshot.view = *view;
	outSnapshot.projection = *projection;

	if (!MakePlanes(
		XMLoadFloat4x4(&outSnapshot.view) *
		XMLoadFloat4x4(&outSnapshot.projection),
		outSnapshot.worldPlanes))
	{
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
	return FAILED(shader->Bind_Matrix("g_ViewMatrix", &snapshot.view)) ||
		FAILED(shader->Bind_Matrix("g_ProjMatrix", &snapshot.projection)) ?
		E_FAIL : S_OK;
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
	MAP_FRUSTUM_CULL_DECISION& outDecision)
{
	outDecision = {};
	outDecision.baseRadius = worldRadius;
	if (0u == snapshot.revision || !std::isfinite(worldCenter.x) ||
		!std::isfinite(worldCenter.y) || !std::isfinite(worldCenter.z) ||
		!std::isfinite(worldRadius) || worldRadius <= 0.f)
	{
		return false;
	}

	outDecision.largeGeometry = "landscape" == assetGroupId ||
		(policy.largeObjectRadiusThreshold > 0.f &&
		 worldRadius >= policy.largeObjectRadiusThreshold);
	f32_t margin = std::isfinite(policy.baseMargin) ?
		(std::max)(policy.baseMargin, 0.f) : 0.f;
	if (outDecision.largeGeometry)
	{
		const f32_t absoluteMargin =
			std::isfinite(policy.largeObjectAbsoluteMargin) ?
			(std::max)(policy.largeObjectAbsoluteMargin, 0.f) : 0.f;
		const f32_t relativeMargin =
			std::isfinite(policy.largeObjectRelativeMargin) ?
			worldRadius * (std::max)(policy.largeObjectRelativeMargin, 0.f) : 0.f;
		margin = (std::max)({ margin, absoluteMargin, relativeMargin });
	}
	outDecision.margin = margin;
	outDecision.effectiveRadius = worldRadius + margin;
	outDecision.wouldBeVisible = true;
	const vector_t center = XMLoadFloat3(&worldCenter);
	for (uint32_t index = 0; index < 6u; ++index)
	{
		const f32_t distance = XMVectorGetX(XMPlaneDotCoord(
			XMLoadFloat4(&snapshot.worldPlanes[index]), center));
		if (!std::isfinite(distance))
			return false;
		outDecision.planeDistances[index] = distance;
		if (distance >= outDecision.effectiveRadius)
			outDecision.wouldBeVisible = false;
	}

	if (state.initialized && state.lastFrustumVisible &&
		!outDecision.wouldBeVisible && policy.diagnostics)
	{
		RecordRejectedTransition(snapshot, assetId, assetGroupId,
			placementId, worldCenter, outDecision, policy.bypass);
	}
	state.initialized = true;
	state.lastFrustumVisible = outDecision.wouldBeVisible;
	if (outDecision.wouldBeVisible)
		state.rejectGraceFrames = policy.rejectHysteresisFrames;

	outDecision.shouldRender = policy.bypass || outDecision.wouldBeVisible;
	if (!outDecision.shouldRender && state.rejectGraceFrames > 0u)
	{
		outDecision.shouldRender = true;
		--state.rejectGraceFrames;
	}
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
