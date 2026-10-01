#include "MapStaticBatchObject.h"
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")
#include "Engine_VertexTypes.h"

#include "GameInstance.h"
#include "MapAssetRenderUtils.h"
#include "Model.h"
#include "MeshLod.h"
#include "Profiler.h"
#include "EffectFailureDiagnostic.h"
#include "Shader.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <cmath>
#include <cfloat>
#include <sstream>

namespace
{
    // Conservative operator-norm bound for signed/nonuniform scale and shear.
    float LinearScaleBound(const float4x4_t& matrix)
    {
        double maximum = 0.;
        for (size_t i = 0u; i < 3u; ++i)
        {
            double row = 0.;
            for (size_t j = 0u; j < 3u; ++j)
            {
                double dot = 0.;
                for (size_t k = 0u; k < 3u; ++k) dot += double(matrix.m[i][k]) * matrix.m[j][k];
                row += std::abs(dot);
            }
            if (!std::isfinite(row)) return 0.f;
            maximum = (std::max)(maximum, row);
        }
        const double scale = std::sqrt(maximum);
        if (!std::isfinite(scale) || scale <= 0. || scale >= (std::numeric_limits<float>::max)()) return 0.f;
        return std::nextafter(static_cast<float>(scale), (std::numeric_limits<float>::infinity)());
    }
    struct INSTANCE_ENVELOPE final
    {
        double minimum[3] = { DBL_MAX, DBL_MAX, DBL_MAX };
        double maximum[3] = { -DBL_MAX, -DBL_MAX, -DBL_MAX };
        bool valid = true, any = false;
        float maximumScale = 0.f;
        void Add(const FMapStaticInstance& instance, const float scale)
        {
            const float* center = &instance.WorldBoundsCenter.x;
            if (!std::isfinite(instance.WorldBoundsRadius) || instance.WorldBoundsRadius <= 0.f) valid = false;
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                if (!std::isfinite(center[axis])) valid = false;
                minimum[axis] = (std::min)(minimum[axis], double(center[axis]) - instance.WorldBoundsRadius);
                maximum[axis] = (std::max)(maximum[axis], double(center[axis]) + instance.WorldBoundsRadius);
            }
            if (scale <= 0.f) valid = false;
            maximumScale = (std::max)(maximumScale, scale);
            any = true;
        }
        bool Store(float4_t& sphere) const
        {
            if (!valid || !any) return false;
            double radiusSquared = 0.;
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                const double c = (minimum[axis] + maximum[axis]) * .5;
                if (!std::isfinite(c) || std::abs(c) > (std::numeric_limits<float>::max)()) return false;
                (&sphere.x)[axis] = static_cast<float>(c);
                const double extent = (std::max)(maximum[axis] - (&sphere.x)[axis], double((&sphere.x)[axis]) - minimum[axis]);
                radiusSquared += extent * extent;
            }
            const double radius = std::sqrt(radiusSquared);
            if (!std::isfinite(radius) || radius <= 0. || radius >= (std::numeric_limits<float>::max)()) return false;
            sphere.w = std::nextafter(static_cast<float>(radius), (std::numeric_limits<float>::infinity)());
            return true;
        }
    };

    struct VIEW_LOD_ENVELOPE final
    {
        const float4x4_t* view = nullptr;
        double viewScale = 0.;
        double maximumX = 0., maximumY = 0., minimumZ = DBL_MAX;
        bool valid = false, any = false;

        explicit VIEW_LOD_ENVELOPE(const MAP_CAMERA_CULL_SNAPSHOT* camera)
        {
            if (!camera) return;
            view = &camera->view;
            if (view->_14 != 0.f || view->_24 != 0.f || view->_34 != 0.f || view->_44 != 1.f) return;
            viewScale = LinearScaleBound(*view);
            valid = viewScale > 0.;
        }

        void Add(const FMapStaticInstance& instance)
        {
            if (!valid) return;
            const double radius = double(instance.WorldBoundsRadius) * viewScale;
            if (!std::isfinite(radius) || radius <= 0.) { valid = false; return; }
            double center[3]{}, margin[3]{};
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                center[axis] = view->m[3][axis];
                double magnitude = std::abs(center[axis]);
                for (size_t source = 0u; source < 3u; ++source)
                {
                    const double term = double((&instance.WorldBoundsCenter.x)[source]) * view->m[source][axis];
                    center[axis] += term;
                    magnitude += std::abs(term);
                }
                // Enclose float matrix evaluation as well as the sphere itself,
                // including cancellation in translated/rotated view coordinates.
                margin[axis] = 16. * FLT_EPSILON * (magnitude + radius + 1.);
                if (!std::isfinite(center[axis]) || !std::isfinite(margin[axis])) { valid = false; return; }
            }
            maximumX = (std::max)(maximumX, std::abs(center[0]) + radius + margin[0]);
            maximumY = (std::max)(maximumY, std::abs(center[1]) + radius + margin[1]);
            minimumZ = (std::min)(minimumZ, center[2] - radius - margin[2]);
            any = true;
        }

        bool Store(float4_t& result) const
        {
            if (!valid || !any || maximumX >= FLT_MAX || maximumY >= FLT_MAX || std::abs(minimumZ) >= FLT_MAX)
                return false;
            result = {
                std::nextafter(static_cast<float>(maximumX), (std::numeric_limits<float>::infinity)()),
                std::nextafter(static_cast<float>(maximumY), (std::numeric_limits<float>::infinity)()),
                std::nextafter(static_cast<float>(minimumZ), -(std::numeric_limits<float>::infinity)()), 1.f };
            return true;
        }
    };

}

CMapStaticBatchObject::CMapStaticBatchObject(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject{ pDevice, pContext }
{}

CMapStaticBatchObject::CMapStaticBatchObject(
	const CMapStaticBatchObject& prototype)
	: CGameObject{ prototype }
{}

CMapStaticBatchObject::~CMapStaticBatchObject()
{}

HRESULT CMapStaticBatchObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapStaticBatchObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_INVALIDARG;

	const DESC& desc =
		*static_cast<DESC*>(pArg);

	if (desc.AssetId.empty() ||
		desc.ModelPrototypeTag.empty() ||
		desc.Instances.empty() ||
		MAP_ASSET_RENDER_MODE::DEFERRED !=
		desc.RenderProfile.renderMode)
	{
		return E_INVALIDARG;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_AssetId = desc.AssetId;
	m_AssetGroupId = desc.AssetGroupId;
	m_RenderProfile = desc.RenderProfile;
	m_FrustumCulling = desc.FrustumCulling;
	m_bMirrored = desc.Mirrored;
	m_Instances = desc.Instances;

    // Hidden gate batches become visible together at the cinematic handoff.
    // Allocate both streams while staging so their first shadow draw only uploads.
	if (FAILED(Ready_Components(
		desc.PrototypeLevelIndex,
		desc.ModelPrototypeTag)) ||
		FAILED(Rebuild_PlacementLookup()) ||
		FAILED(Ensure_InstanceCapacity(
			static_cast<uint32_t>(
				m_Instances.size()))) ||
        (m_RenderProfile.castsShadow && FAILED(Ensure_ShadowInstanceCapacity(
            static_cast<uint32_t>(m_Instances.size())))))
	{
		return E_FAIL;
	}

	return S_OK;
}

void CMapStaticBatchObject::Update(
	f32_t fTimeDelta)
{
	m_fElapsedTime += fTimeDelta;
}

void CMapStaticBatchObject::Late_Update(
	f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	m_bFinalCameraPrepared = false;

	if (Engine::CProfiler* profiler =
		CGameInstance::Get().Get_Profiler())
	{
		profiler->Add_Counter(
			Engine::EProfilerCounter::MapPlacements,
			m_Instances.size());

		profiler->Add_Counter(
			Engine::EProfilerCounter::MapBatchCount);
	}
}

void CMapStaticBatchObject::Submit_FinalCamera()
{
    auto& game = CGameInstance::Get();
    if (game.Is_SceneEnvironmentReplaced() || m_iAuthoredVisibleInstanceCount == 0u)
        return;

    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    const HRESULT visibility = Upload_VisibleInstances(camera);
    m_bFinalCameraPrepared = SUCCEEDED(visibility);
    // A failed preparation keeps the original draw callback as the error/retry path.
    if (FAILED(visibility) || !m_VisibleInstances.empty())
        game.Add_RenderObject(RENDERGROUP::NONBLEND,
            static_pointer_cast<CGameObject>(shared_from_this()));

    // Camera rejection does not reject a caster whose shadow reaches the view.
    // Providers have now committed the final shadow switch and light volume.
    if (m_RenderProfile.castsShadow && game.Is_ShadowLightEnabled())
    {
        const HRESULT shadows = Upload_ShadowInstances();
        if (FAILED(shadows) || !m_ShadowInstances.empty())
            game.Add_RenderObject(RENDERGROUP::SHADOW,
                static_pointer_cast<CGameObject>(shared_from_this()));
    }
}

HRESULT CMapStaticBatchObject::Render()
{
	Engine::CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
	Engine::CProfilerWorkScope renderWork(profiler, Engine::EProfilerWork::MapBatchRender);
	if (CGameInstance::Get().Is_SceneEnvironmentReplaced())
		return S_OK;

	const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot =
		CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
	const bool_t hasCameraSnapshot = nullptr != cameraSnapshot;
	const bool_t preparedForCamera = m_bFinalCameraPrepared &&
		m_bVisibleInstancesUsedCamera == hasCameraSnapshot &&
		m_iVisibleCameraRevision == (hasCameraSnapshot ? cameraSnapshot->revision : 0u);
	if (!preparedForCamera && FAILED(Upload_VisibleInstances(cameraSnapshot)))
	{
		return E_FAIL;
	}
	if (m_VisibleInstances.empty())
	{
		if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::MapBatchEmptyRenders);
		return S_OK;
	}
	if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibleRenders);

	const HRESULT cameraBindResult = hasCameraSnapshot ?
		CMapAssetRenderUtils::Bind_CameraCullSnapshot(
			m_pShaderCom, *cameraSnapshot) :
		(FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		 FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)) ? E_FAIL : S_OK);
	if (FAILED(cameraBindResult))
	{
		return E_FAIL;
	}

	const uint32_t passIndex =
		CMapAssetRenderUtils::Select_Pass(
			m_RenderProfile,
			m_bMirrored);

	if (passIndex > 2u)
		return E_UNEXPECTED;

	const uint32_t instanceCount =
		static_cast<uint32_t>(
			m_VisibleInstances.size());

    Engine::MESH_SCREEN_LOD_DESC screenLod{};
    const bool_t hasScreenLod = hasCameraSnapshot && m_RenderProfile.opacity >= 1.f &&
        Build_ScreenLodView(*cameraSnapshot, screenLod);
	const bool_t useSourceMaterials =
		CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;
	{
		Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.BindAndDraw");
	for (uint32_t meshIndex = 0;
		meshIndex < m_pModelCom->Get_NumMeshes();
		++meshIndex)
	{
		const auto* surface = m_pModelCom->Get_MaterialSurface(meshIndex);
        // Material variants share CMesh geometry: re-admit the current draw's
        // material instead of inheriting the source model's LOD eligibility.
        const bool_t useMeshLod = hasScreenLod && useSourceMaterials && surface &&
            surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            (surface->sourceBgFlags & 64u) == 0u &&
            (surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::INHERIT ||
             surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED) &&
            !m_pModelCom->Has_MaterialTexture(meshIndex, aiTextureType_OPACITY);
		const uint32_t meshPass = useSourceMaterials && surface &&
			surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ?
			24u + passIndex : passIndex;
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Material.Bind");
			Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchMaterial);
			if (FAILED(CMapAssetRenderUtils::Bind_Material(m_pModelCom, m_pShaderCom,
				meshIndex, m_RenderProfile, m_fElapsedTime, nullptr, m_AssetId, nullptr, nullptr,
				MAP_MATERIAL_BINDING_MODE::INSTANCED))) return E_FAIL;
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Pass.Apply");
			Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchPass);
			if (FAILED(m_pShaderCom->Begin(meshPass))) return E_FAIL;
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Mesh.Submit");
			Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchDraw);
			if (FAILED(m_pModelCom->Render_Instanced(meshIndex, m_pInstanceBuffer.Get(),
				sizeof(VTXMESHINSTANCE), instanceCount, 0u, useMeshLod ? &screenLod : nullptr))) return E_FAIL;
		}
	}
	}

	return S_OK;
}

HRESULT CMapStaticBatchObject::Render_Shadow()
{
	if (CGameInstance::Get().Is_SceneEnvironmentReplaced())
		return S_OK;

	if (!m_RenderProfile.castsShadow)
		return S_OK;
	// Keep the failing asset and operation; the renderer only knows the object type.
	const auto fail = [this](const char* stage, HRESULT result,
		uint32_t mesh = UINT_MAX, uint32_t pass = UINT_MAX) noexcept -> HRESULT
	{
		try
		{
			std::ostringstream detail;
			detail << "stage=" << stage << " asset=" << std::quoted(m_AssetId)
				<< " mesh=" << mesh << " pass=" << pass
				<< " instances=" << m_ShadowInstances.size()
				<< " hr=0x" << std::hex << static_cast<unsigned long>(result)
				<< " device_hr=0x" << static_cast<unsigned long>(m_pDevice->GetDeviceRemovedReason());
			Write_EffectFailureDiagnostic("Map.StaticBatch.Shadow", detail.str());
		}
		catch (...) { }
		return result;
	};
	// Frame providers can change the light after Late_Update queued this batch.
	HRESULT result = Upload_ShadowInstances();
	if (FAILED(result))
		return fail("UploadInstances", result);
	if (m_ShadowInstances.empty())
		return S_OK;
	const bool_t useSourceMaterials =
		CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;

	result = CGameInstance::Get().Bind_ShadowLight_ShaderResource(
		m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW);
	if (FAILED(result)) return fail("BindLightView", result);
	result = CGameInstance::Get().Bind_ShadowLight_ShaderResource(
		m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ);
	if (FAILED(result)) return fail("BindLightProjection", result);

	const uint32_t iCullPass =
		CMapAssetRenderUtils::Select_Pass(
			m_RenderProfile, m_bMirrored);
	if (iCullPass > 2u)
		return fail("SelectPass", E_UNEXPECTED);

	const uint32_t iInstanceCount =
		static_cast<uint32_t>(m_ShadowInstances.size());
	for (uint32_t iMesh = 0;
		iMesh < m_pModelCom->Get_NumMeshes(); ++iMesh)
	{
		const auto* surface = m_pModelCom->Get_MaterialSurface(iMesh);
		if (surface && !surface->castsShadow)
			continue;
		const bool_t opaqueShadow = CMapAssetRenderUtils::Uses_OpaqueShadowPass(
			surface, m_RenderProfile, useSourceMaterials);
		uint32_t shadowPassBase = opaqueShadow ? 21u : 12u;
		if (!opaqueShadow)
		{
			// Simple families retain their alpha test when source mode is off or masked.
			if (!surface || surface->family == Engine::MODEL_SURFACE_FAMILY::LEGACY ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::PBR_OPAQUE ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE)
				shadowPassBase = 18u;
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Material.Bind");
			result = CMapAssetRenderUtils::Bind_ShadowMaterial(m_pModelCom, m_pShaderCom,
				iMesh, m_RenderProfile, m_fElapsedTime);
			if (FAILED(result)) return fail("BindMaterial", result, iMesh, shadowPassBase + iCullPass);
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Pass.Apply");
			result = m_pShaderCom->Begin(shadowPassBase + iCullPass);
			if (FAILED(result)) return fail("ApplyPass", result, iMesh, shadowPassBase + iCullPass);
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Mesh.Submit");
			result = m_pModelCom->Render_Instanced(iMesh, m_pShadowInstanceBuffer.Get(),
				sizeof(VTXMESHINSTANCE), iInstanceCount);
			if (FAILED(result)) return fail("DrawMesh", result, iMesh, shadowPassBase + iCullPass);
		}
	}

	return S_OK;
}

bool_t CMapStaticBatchObject::Try_GetStaticShadowRevision(uint64_t& outRevision) const
{
	outRevision = 0u;
	if (m_iStaticShadowRevision == 0u || !m_RenderProfile.castsShadow ||
		!m_pModelCom || m_pModelCom->Get_NumMeshes() == 0u ||
		!CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials)
		return false;

	if (!m_bStaticShadowMaterialInputs)
		return false;
	// Surface/profile constants were admitted when this model was staged.
	// Mutable texture overrides and morph clones still invalidate cache use.
	for (const uint32_t mesh : m_StaticShadowCasterMeshes)
	{
		if (m_pModelCom->Has_MorphBaseVertices(mesh) ||
			m_pModelCom->Has_MaterialTextureOverrides(mesh))
			return false;
	}

	outRevision = m_iStaticShadowRevision;
	return true;
}

bool_t CMapStaticBatchObject::Try_PickMovementSurface(
	const float3_t& rayOrigin, const float3_t& rayDirection,
	const f32_t maxDistance, f32_t& outDistance) const
{
	if (!m_pModelCom || m_RenderProfile.opacity <= 0.f ||
		m_RenderProfile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED)
		return false;
	const vector_t origin = XMLoadFloat3(&rayOrigin);
	const vector_t direction = XMLoadFloat3(&rayDirection);
	// The cached envelope contains every authored-visible instance. A dirty
	// envelope cannot reject the current transform, so retain the instance scan.
	if (!m_bBatchBoundsDirty && m_bHasBatchBounds)
	{
		const BoundingBox bounds(
			float3_t(m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z),
			float3_t(m_BatchBounds.w, m_BatchBounds.w, m_BatchBounds.w));
		f32_t entry = 0.f;
		if (!bounds.Intersects(origin, direction, entry) || entry > maxDistance)
			return false;
	}
	const uint32_t cull = CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored) % 3u;
	const auto cullMode = cull == 0u ? CModel::PICK_CULL_MODE::BACK :
		cull == 1u ? CModel::PICK_CULL_MODE::FRONT : CModel::PICK_CULL_MODE::NONE;
	f32_t nearest = maxDistance;
	bool_t hit = false;
	for (const auto& instance : m_Instances)
	{
		if (!instance.Visible || instance.Suppressed || instance.CameraPreviewSuppressed)
			continue;
		const BoundingBox bounds(instance.WorldBoundsCenter, float3_t(
			instance.WorldBoundsRadius, instance.WorldBoundsRadius, instance.WorldBoundsRadius));
		f32_t boundDistance = 0.f;
		if (!bounds.Intersects(origin, direction, boundDistance) || boundDistance > nearest)
			continue;
		for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
		{
			const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
			// Masked floor geometry remains eligible; GPU alpha coverage is not
			// the movement contract. Animated foliage and shader displacement are excluded.
			if (m_pModelCom->Has_MorphBaseVertices(mesh) ||
				(surface && (surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED ||
					surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED ||
					(surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
						surface->sourceCharacter.program == 43u))))
				continue;
			f32_t distance = nearest;
			if (m_pModelCom->Try_PickStaticSurface(mesh, instance.World, rayOrigin, rayDirection,
				nearest, cullMode, distance) && distance < nearest)
			{
				nearest = distance;
				hit = true;
			}
		}
	}
	if (hit) outDistance = nearest;
	return hit;
}

HRESULT CMapStaticBatchObject::Update_Instance(
	uint64_t placementId,
	const FMapStaticInstance& instance)
{
	const auto iter =
		m_PlacementLookup.find(placementId);

	if (iter == m_PlacementLookup.end() ||
		instance.PlacementId != placementId)
	{
		return E_INVALIDARG;
	}

	FMapStaticInstance& current = m_Instances[iter->second];
	// Bounds participate in light-volume culling even when the world is unchanged.
	const bool_t shadowChanged = current.Visible != instance.Visible ||
		0 != std::memcmp(&current.World, &instance.World, sizeof(current.World)) ||
		0 != std::memcmp(&current.WorldInvTranspose, &instance.WorldInvTranspose,
			sizeof(current.WorldInvTranspose)) ||
		0 != std::memcmp(&current.WorldBoundsCenter, &instance.WorldBoundsCenter,
			sizeof(current.WorldBoundsCenter)) ||
		current.WorldBoundsRadius != instance.WorldBoundsRadius;
	if (shadowChanged && m_iStaticShadowRevision != 0u)
		++m_iStaticShadowRevision;
	if (current.Visible != instance.Visible)
	{
		if (instance.Visible)
			++m_iAuthoredVisibleInstanceCount;
		else
			--m_iAuthoredVisibleInstanceCount;
	}
	const bool_t suppressed = current.Suppressed;
	const bool_t cameraPreviewSuppressed = current.CameraPreviewSuppressed;
	current = instance;
	current.Suppressed = suppressed;
	current.CameraPreviewSuppressed = cameraPreviewSuppressed;
    m_InstanceLinearScaleBounds[iter->second] = LinearScaleBound(current.World);
    m_bBatchBoundsDirty = true;
	m_bShadowInstancesDirty = true;
	m_bVisibleInstancesDirty = true;
	m_bFinalCameraPrepared = false;
	return S_OK;
}

HRESULT CMapStaticBatchObject::Set_InstanceVisible(
	uint64_t placementId,
	bool_t visible)
{
	const auto iter =
		m_PlacementLookup.find(placementId);

	if (iter == m_PlacementLookup.end())
		return HRESULT_FROM_WIN32(
			ERROR_NOT_FOUND);

	FMapStaticInstance& instance = m_Instances[iter->second];
	if (instance.Visible != visible)
	{
		if (visible)
			++m_iAuthoredVisibleInstanceCount;
		else
			--m_iAuthoredVisibleInstanceCount;
		instance.Visible = visible;
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
        m_bBatchBoundsDirty = true;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Try_GetInstanceVisible(
	const uint64_t placementId,
	bool_t& outVisible) const
{
	const auto iter = m_PlacementLookup.find(placementId);
	if (iter == m_PlacementLookup.end() || iter->second >= m_Instances.size())
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	outVisible = m_Instances[iter->second].Visible;
	return S_OK;
}

HRESULT CMapStaticBatchObject::Set_InstanceSuppressed(
	const uint64_t placementId,
	const bool_t suppressed)
{
	const auto iter = m_PlacementLookup.find(placementId);
	if (iter == m_PlacementLookup.end() || iter->second >= m_Instances.size())
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

	FMapStaticInstance& instance = m_Instances[iter->second];
	if (instance.Suppressed != suppressed)
	{
		instance.Suppressed = suppressed;
		// Batch bounds stay conservative (they still cover this instance); only
		// the draw and shadow payloads and the cached static shadow change.
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Set_InstanceCameraPreviewSuppressed(
	const uint64_t placementId,
	const bool_t suppressed)
{
	const auto iter = m_PlacementLookup.find(placementId);
	if (iter == m_PlacementLookup.end() || iter->second >= m_Instances.size())
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

	FMapStaticInstance& instance = m_Instances[iter->second];
	if (instance.CameraPreviewSuppressed != suppressed)
	{
		instance.CameraPreviewSuppressed = suppressed;
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Ready_Components(
	uint32_t prototypeLevelIndex,
	const std::wstring& modelPrototypeTag)
{
	if (FAILED(__super::Add_Component(
		prototypeLevelIndex,
		TEXT(
			"Prototype_Component_Shader_VtxMeshMapInstance"),
		TEXT("Com_Shader"),
		m_pShaderCom)) ||

		FAILED(__super::Add_Component(
			prototypeLevelIndex,
			modelPrototypeTag,
			TEXT("Com_Model"),
			m_pModelCom)))
	{
		return E_FAIL;
	}

	m_StaticShadowCasterMeshes.clear();
	m_bStaticShadowMaterialInputs = true;
	for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
	{
		const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
		if (surface && !surface->castsShadow)
			continue;
		m_StaticShadowCasterMeshes.push_back(mesh);
		m_bStaticShadowMaterialInputs &=
			CMapAssetRenderUtils::Uses_StaticShadowInputs(surface, m_RenderProfile, true);
	}
	m_bStaticShadowMaterialInputs &= !m_StaticShadowCasterMeshes.empty();

	// Small or unsimplifiable meshes never consume tight LOD bounds. Keep the
	// ordinary world envelope/culling and profiler draw denominators unchanged.
	m_bHasStaticMeshLod = false;
	for (uint32_t meshIndex = 0u; meshIndex < m_pModelCom->Get_NumMeshes(); ++meshIndex)
	{
		if (m_pModelCom->Has_StaticMeshLod(meshIndex))
		{
			m_bHasStaticMeshLod = true;
			break;
		}
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Ensure_InstanceCapacity(
	uint32_t requiredCount)
{
	if (0 == requiredCount)
		return E_INVALIDARG;

	if (requiredCount <= m_iInstanceCapacity &&
		nullptr != m_pInstanceBuffer)
	{
		return S_OK;
	}

	uint32_t newCapacity = 1u;

	while (newCapacity < requiredCount)
		newCapacity <<= 1u;

	const uint64_t byteWidth =
		static_cast<uint64_t>(newCapacity) *
		sizeof(VTXMESHINSTANCE);

	if (byteWidth >
		(std::numeric_limits<uint32_t>::max)())
	{
		return E_OUTOFMEMORY;
	}

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth =
		static_cast<uint32_t>(byteWidth);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags =
		D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags =
		D3D11_CPU_ACCESS_WRITE;

	ComPtr<ID3D11Buffer> stagedBuffer;

	if (FAILED(m_pDevice->CreateBuffer(
		&bufferDesc,
		nullptr,
		&stagedBuffer)))
	{
		return E_FAIL;
	}

	m_pInstanceBuffer =
		std::move(stagedBuffer);

	m_iInstanceCapacity =
		newCapacity;

	m_VisibleInstances.reserve(
		newCapacity);
	m_CandidateVisibleInstances.reserve(
		newCapacity);

	return S_OK;
}

HRESULT CMapStaticBatchObject::Ensure_ShadowInstanceCapacity(
	uint32_t requiredCount)
{
	if (0 == requiredCount)
		return E_INVALIDARG;

	if (requiredCount <= m_iShadowInstanceCapacity &&
		nullptr != m_pShadowInstanceBuffer)
	{
		return S_OK;
	}

	uint32_t newCapacity = 1u;
	while (newCapacity < requiredCount)
		newCapacity <<= 1u;

	const uint64_t byteWidth =
		static_cast<uint64_t>(newCapacity) *
		sizeof(VTXMESHINSTANCE);
	if (byteWidth >
		(std::numeric_limits<uint32_t>::max)())
	{
		return E_OUTOFMEMORY;
	}

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = static_cast<uint32_t>(byteWidth);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ComPtr<ID3D11Buffer> stagedBuffer;
	if (FAILED(m_pDevice->CreateBuffer(
		&bufferDesc, nullptr, &stagedBuffer)))
	{
		return E_FAIL;
	}

	m_pShadowInstanceBuffer = std::move(stagedBuffer);
	m_iShadowInstanceCapacity = newCapacity;
	m_ShadowInstances.reserve(newCapacity);
	m_CandidateShadowInstances.reserve(newCapacity);
	return S_OK;
}

HRESULT CMapStaticBatchObject::Upload_VisibleInstances(
	const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot)
{
	Engine::CProfiler* const visibilityProfiler = CGameInstance::Get().Get_Profiler();
	Engine::CProfilerWorkScope visibilityWork(visibilityProfiler, Engine::EProfilerWork::MapBatchVisibility);
	const bool_t hasCameraSnapshot = nullptr != cameraSnapshot;
	const uint64_t cameraRevision = hasCameraSnapshot ?
		cameraSnapshot->revision : 0u;
	if (!m_bVisibleInstancesDirty &&
		m_bVisibleInstancesUsedCamera == hasCameraSnapshot &&
		m_iVisibleCameraRevision == cameraRevision)
	{
		if (Engine::CProfiler* profiler =
			CGameInstance::Get().Get_Profiler())
		{
			profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibilityCacheHits);
			profiler->Add_Counter(
				Engine::EProfilerCounter::MapVisibleInstances,
				m_VisibleInstances.size());
		}
		return S_OK;
	}

	if (visibilityProfiler) visibilityProfiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibilityRebuilds);
	Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Visibility");
	m_CandidateVisibleInstances.clear();
	bool_t requiresNextCameraTick = false;
    INSTANCE_ENVELOPE visibleEnvelope;
    VIEW_LOD_ENVELOPE visibleViewEnvelope(m_bHasStaticMeshLod ? cameraSnapshot : nullptr);
    uint64_t cullingCandidates = 0u;
    if (m_bBatchBoundsDirty) Rebuild_BatchCullBounds();
    bool_t rejectBatch = false;
    if (hasCameraSnapshot && m_bHasBatchBounds && !m_FrustumCulling.bypass)
    {
        MAP_FRUSTUM_CULL_DECISION batchDecision{};
        MAP_FRUSTUM_CULLING_POLICY broadPolicy = m_FrustumCulling;
        broadPolicy.diagnostics = false;
        const float3_t center(m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z);
        if (CMapAssetRenderUtils::Evaluate_FrustumVisibility(broadPolicy, *cameraSnapshot,
            m_AssetId, m_AssetGroupId, 0u, center, m_BatchBounds.w, m_BatchFrustumState, batchDecision))
        {
            // Diagnostics retain real placement IDs and the precise loop.
            rejectBatch = !batchDecision.shouldRender && !m_FrustumCulling.diagnostics;
            requiresNextCameraTick = !batchDecision.wouldBeVisible && batchDecision.shouldRender;
        }
    }

	if (rejectBatch && visibilityProfiler)
		visibilityProfiler->Add_Counter(Engine::EProfilerCounter::MapBatchBoundsRejected);
	if (!rejectBatch)
	{
		Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.CullAndPack");
	for (size_t index = 0u; index < m_Instances.size(); ++index)
	{
        FMapStaticInstance& instance = m_Instances[index];
		if (!instance.Visible || instance.Suppressed ||
			instance.CameraPreviewSuppressed)
			continue;

        ++cullingCandidates;
		MAP_FRUSTUM_CULL_DECISION decision{};
		const bool_t evaluated = hasCameraSnapshot &&
			CMapAssetRenderUtils::Evaluate_FrustumVisibility(
				m_FrustumCulling,
				*cameraSnapshot,
				m_AssetId,
				m_AssetGroupId,
				instance.PlacementId,
				instance.WorldBoundsCenter,
				instance.WorldBoundsRadius,
				instance.FrustumState,
				decision);
		if (evaluated && !decision.wouldBeVisible &&
			decision.shouldRender && !m_FrustumCulling.bypass)
		{
			requiresNextCameraTick = true;
		}
		if (evaluated && !decision.shouldRender)
		{
			continue;
		}

        visibleEnvelope.Add(instance, m_InstanceLinearScaleBounds[index]);
        if (m_bHasStaticMeshLod) visibleViewEnvelope.Add(instance);
		VTXMESHINSTANCE gpuInstance{};
		gpuInstance.World =
			instance.World;
		gpuInstance.WorldInvTranspose =
			instance.WorldInvTranspose;
        gpuInstance.vLightmapScaleBias = instance.BakedLighting.scaleBias;
        gpuInstance.vLightmapAverageScale = instance.BakedLighting.averageScale;
        gpuInstance.vLightmapDirectionalScale = instance.BakedLighting.directionalScale;
        gpuInstance.vStaticShadowScaleBias = instance.BakedLighting.shadowScaleBias;

		m_CandidateVisibleInstances.push_back(
			gpuInstance);
	}
	}

	if (Engine::CProfiler* profiler =
		CGameInstance::Get().Get_Profiler())
	{
        // These are actual recomputation work, separate from cached frame totals.
        profiler->Add_Counter(Engine::EProfilerCounter::MapCullingCandidates, cullingCandidates);
        profiler->Add_Counter(Engine::EProfilerCounter::MapCullingVisible, m_CandidateVisibleInstances.size());
		profiler->Add_Counter(
			Engine::EProfilerCounter::
			MapVisibleInstances,
			m_CandidateVisibleInstances.size());
	}

	// Camera motion can leave the ordered GPU payload unchanged. Preserve the
	// existing buffer in that case instead of discarding it every camera tick.
	const bool_t payloadUnchanged =
		m_CandidateVisibleInstances.size() == m_VisibleInstances.size() &&
		(m_CandidateVisibleInstances.empty() || 0 == std::memcmp(
			m_CandidateVisibleInstances.data(), m_VisibleInstances.data(),
			m_CandidateVisibleInstances.size() * sizeof(VTXMESHINSTANCE)));
    float4_t candidateLodBounds{};
    const bool_t hasLodBounds = visibleEnvelope.Store(candidateLodBounds);
    float4_t candidateTightLodBounds{};
    const bool_t hasTightLodBounds = hasLodBounds && visibleViewEnvelope.Store(candidateTightLodBounds);
	if (payloadUnchanged || m_CandidateVisibleInstances.empty())
	{
        m_VisibleLodBounds = hasLodBounds ? candidateLodBounds : float4_t{};
        m_VisibleTightLodBounds = hasTightLodBounds ? candidateTightLodBounds : float4_t{};
        m_fVisibleLodScale = hasLodBounds ? visibleEnvelope.maximumScale : 0.f;
		if (!payloadUnchanged)
			m_VisibleInstances.swap(m_CandidateVisibleInstances);
		m_bVisibleInstancesDirty = requiresNextCameraTick;
		m_bVisibleInstancesUsedCamera = hasCameraSnapshot;
		m_iVisibleCameraRevision = cameraRevision;
		return S_OK;
	}

	{
		Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.InstanceUpload");
	if (FAILED(Ensure_InstanceCapacity(
		static_cast<uint32_t>(
			m_CandidateVisibleInstances.size()))))
	{
		return E_FAIL;
	}

	D3D11_MAPPED_SUBRESOURCE mapped{};

	if (FAILED(m_pContext->Map(
		m_pInstanceBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mapped)))
	{
		return E_FAIL;
	}

	std::memcpy(
		mapped.pData,
		m_CandidateVisibleInstances.data(),
		m_CandidateVisibleInstances.size() *
		sizeof(VTXMESHINSTANCE));

	m_pContext->Unmap(
		m_pInstanceBuffer.Get(),
		0);
	if (visibilityProfiler)
		visibilityProfiler->Add_Counter(Engine::EProfilerCounter::MapBatchUploadBytes,
			m_CandidateVisibleInstances.size() * sizeof(VTXMESHINSTANCE));
	}
    m_VisibleLodBounds = hasLodBounds ? candidateLodBounds : float4_t{};
    m_VisibleTightLodBounds = hasTightLodBounds ? candidateTightLodBounds : float4_t{};
    m_fVisibleLodScale = hasLodBounds ? visibleEnvelope.maximumScale : 0.f;
	// Commit only after upload succeeds so a failed Map cannot replace the
	// CPU payload associated with the previous successful GPU upload.
	m_VisibleInstances.swap(m_CandidateVisibleInstances);
	m_bVisibleInstancesDirty = requiresNextCameraTick;
	m_bVisibleInstancesUsedCamera = hasCameraSnapshot;
	m_iVisibleCameraRevision = cameraRevision;

	return S_OK;
}

HRESULT CMapStaticBatchObject::Upload_ShadowInstances()
{
	Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.ShadowPrepare");
	MAP_SHADOW_CULL_SNAPSHOT lightSnapshot{};
	const bool_t hasLightSnapshot =
		CMapAssetRenderUtils::Capture_ShadowCullSnapshot(lightSnapshot);
	const uint64_t lightRevision = hasLightSnapshot ? lightSnapshot.revision : 0u;
	if (!m_bShadowInstancesDirty &&
		m_bShadowInstancesUsedLight == hasLightSnapshot &&
		m_iShadowLightRevision == lightRevision)
		return S_OK;

	m_CandidateShadowInstances.clear();
	for (const FMapStaticInstance& instance : m_Instances)
	{
		if (!instance.Visible || instance.Suppressed ||
			instance.CameraPreviewSuppressed ||
			(hasLightSnapshot && !CMapAssetRenderUtils::Intersects_ShadowCullSnapshot(
				lightSnapshot, instance.WorldBoundsCenter, instance.WorldBoundsRadius)))
			continue;

		VTXMESHINSTANCE gpuInstance{};
		gpuInstance.World = instance.World;
		gpuInstance.WorldInvTranspose = instance.WorldInvTranspose;
		gpuInstance.vLightmapScaleBias = instance.BakedLighting.scaleBias;
		gpuInstance.vLightmapAverageScale = instance.BakedLighting.averageScale;
		gpuInstance.vLightmapDirectionalScale = instance.BakedLighting.directionalScale;
		gpuInstance.vStaticShadowScaleBias = instance.BakedLighting.shadowScaleBias;
		m_CandidateShadowInstances.push_back(gpuInstance);
	}

	const bool_t payloadUnchanged =
		m_CandidateShadowInstances.size() == m_ShadowInstances.size() &&
		(m_CandidateShadowInstances.empty() || 0 == std::memcmp(
			m_CandidateShadowInstances.data(), m_ShadowInstances.data(),
			m_CandidateShadowInstances.size() * sizeof(VTXMESHINSTANCE)));
	if (!payloadUnchanged && !m_CandidateShadowInstances.empty())
	{
		Engine::CProfilerDetailScope uploadScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.ShadowUpload");
		HRESULT result = Ensure_ShadowInstanceCapacity(
			static_cast<uint32_t>(m_CandidateShadowInstances.size()));
		if (FAILED(result)) return result;

		D3D11_MAPPED_SUBRESOURCE mapped{};
		result = m_pContext->Map(m_pShadowInstanceBuffer.Get(), 0,
			D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		if (FAILED(result)) return result;

		std::memcpy(mapped.pData, m_CandidateShadowInstances.data(),
			m_CandidateShadowInstances.size() * sizeof(VTXMESHINSTANCE));
		m_pContext->Unmap(m_pShadowInstanceBuffer.Get(), 0);
	}
	// A failed Map leaves both the successful payload and light revision intact.
	// The next call retries even when only the light, rather than instances, moved.
	if (!payloadUnchanged)
		m_ShadowInstances.swap(m_CandidateShadowInstances);
	m_bShadowInstancesDirty = false;
	m_bShadowInstancesUsedLight = hasLightSnapshot;
	m_iShadowLightRevision = lightRevision;
	return S_OK;
}

HRESULT CMapStaticBatchObject::
Rebuild_PlacementLookup()
{
	m_PlacementLookup.clear();
	m_iAuthoredVisibleInstanceCount = 0u;
	m_PlacementLookup.reserve(
		m_Instances.size());
    m_InstanceLinearScaleBounds.resize(m_Instances.size());

	for (uint32_t index = 0;
		index < m_Instances.size();
		++index)
	{
		const FMapStaticInstance& instance =
			m_Instances[index];

		if (0 == instance.PlacementId ||
			instance.WorldBoundsRadius <= 0.f)
		{
			return E_INVALIDARG;
		}

		const auto [iter, inserted] =
			m_PlacementLookup.emplace(
				instance.PlacementId,
				index);

		UNREFERENCED_PARAMETER(iter);

		if (!inserted)
			return E_INVALIDARG;
        m_InstanceLinearScaleBounds[index] = LinearScaleBound(instance.World);
		if (instance.Visible)
			++m_iAuthoredVisibleInstanceCount;
	}

	return S_OK;
}

unique_ptr<CMapStaticBatchObject>
CMapStaticBatchObject::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance =
		unique_ptr<CMapStaticBatchObject>(
			new CMapStaticBatchObject(
				pDevice,
				pContext));

	if (FAILED(
		instance->Initialize_Prototype()))
	{
		return nullptr;
	}

	return instance;
}

shared_ptr<CPrototype>
CMapStaticBatchObject::Clone(void* pArg)
{
	auto instance =
		shared_ptr<CMapStaticBatchObject>(
			new CMapStaticBatchObject(
				*this));

	if (FAILED(instance->Initialize(pArg)))
		return nullptr;

	return instance;
}

void CMapStaticBatchObject::Rebuild_BatchCullBounds()
{
    INSTANCE_ENVELOPE envelope;
    uint32_t grace = 0u;
    for (size_t index = 0u; index < m_Instances.size(); ++index)
    {
        const auto& instance = m_Instances[index];
        if (!instance.Visible) continue;
        envelope.Add(instance, m_InstanceLinearScaleBounds[index]);
        grace = (std::max)(grace, instance.FrustumState.rejectGraceFrames);
    }
    m_bHasBatchBounds = envelope.Store(m_BatchBounds);
    m_BatchFrustumState = {};
    m_BatchFrustumState.rejectGraceFrames = grace;
    m_bBatchBoundsDirty = false;
}

bool_t CMapStaticBatchObject::Build_ScreenLodView(const MAP_CAMERA_CULL_SNAPSHOT& camera,
    Engine::MESH_SCREEN_LOD_DESC& result) const
{
    const auto& p = camera.projection;
    // Other projection conventions retain the original direct draw.
    if (p._14 != 0.f || p._24 != 0.f || p._34 != 1.f || p._44 != 0.f ||
        p._12 != 0.f || p._21 != 0.f || p._13 != 0.f || p._23 != 0.f ||
        p._41 != 0.f || p._42 != 0.f || p._11 <= 0.f || p._22 <= 0.f ||
        p._33 <= 1.f || p._43 >= 0.f || m_fVisibleLodScale <= 0.f || m_VisibleLodBounds.w <= 0.f)
        return false;
    const auto& v = camera.view;
    if (v._14 != 0.f || v._24 != 0.f || v._34 != 0.f || v._44 != 1.f) return false;
    const float viewScale = LinearScaleBound(v);
    if (viewScale <= 0.f) return false;
    const auto viewport = CGameInstance::Get().Get_ViewportSize();
    if (viewport.x <= 0.f || viewport.y <= 0.f) return false;
    const vector_t center = XMVectorSet(m_VisibleLodBounds.x, m_VisibleLodBounds.y, m_VisibleLodBounds.z, 1.f);
    Engine::MESH_SCREEN_LOD_DESC candidate{};
    XMStoreFloat4(&candidate.viewBounds, XMVector3TransformCoord(center, XMLoadFloat4x4(&v)));
    candidate.viewBounds.w = m_VisibleLodBounds.w * viewScale;
    candidate.projectionPixels = { p._11 * viewport.x * .5f, p._22 * viewport.y * .5f };
    candidate.maximumScale = m_fVisibleLodScale * viewScale;
    candidate.nearPlane = -p._43 / p._33;
    if (m_VisibleTightLodBounds.w == 1.f && m_bVisibleInstancesUsedCamera &&
        m_iVisibleCameraRevision == camera.revision)
    {
        candidate.maximumAbsViewXY = { m_VisibleTightLodBounds.x, m_VisibleTightLodBounds.y };
        candidate.minimumViewDepth = m_VisibleTightLodBounds.z;
        candidate.hasTightViewBounds = true;
    }
    const float values[] = { candidate.viewBounds.x, candidate.viewBounds.y, candidate.viewBounds.z,
        candidate.viewBounds.w, candidate.projectionPixels.x, candidate.projectionPixels.y,
        candidate.maximumScale, candidate.nearPlane };
    for (const auto value : values) if (!std::isfinite(value)) return false;
    if (candidate.nearPlane <= 0.f) return false;
    result = candidate;
    return true;
}
