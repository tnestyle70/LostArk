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
#include "Shader.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <cmath>
#include <cfloat>

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
        void Add(const FMapStaticInstance& instance)
        {
            const float* center = &instance.WorldBoundsCenter.x;
            if (!std::isfinite(instance.WorldBoundsRadius) || instance.WorldBoundsRadius <= 0.f) valid = false;
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                if (!std::isfinite(center[axis])) valid = false;
                minimum[axis] = (std::min)(minimum[axis], double(center[axis]) - instance.WorldBoundsRadius);
                maximum[axis] = (std::max)(maximum[axis], double(center[axis]) + instance.WorldBoundsRadius);
            }
            const float scale = LinearScaleBound(instance.World);
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

	if (FAILED(Ready_Components(
		desc.PrototypeLevelIndex,
		desc.ModelPrototypeTag)) ||
		FAILED(Rebuild_PlacementLookup()) ||
		FAILED(Ensure_InstanceCapacity(
			static_cast<uint32_t>(
				m_Instances.size()))) ||
		FAILED(Ensure_ShadowInstanceCapacity(
			static_cast<uint32_t>(
				m_Instances.size()))))
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

	if (Engine::CProfiler* profiler =
		CGameInstance::Get().Get_Profiler())
	{
		profiler->Add_Counter(
			Engine::EProfilerCounter::MapPlacements,
			m_Instances.size());

		profiler->Add_Counter(
			Engine::EProfilerCounter::MapBatchCount);
	}

	if (0u != m_iAuthoredVisibleInstanceCount)
	{
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::NONBLEND,
			static_pointer_cast<CGameObject>(
				shared_from_this()));
	}

	// Frame providers can replace the shadow light after Late_Update. Queue
	// authored casters now, then cull against the final light in Render_Shadow.
	if (m_RenderProfile.castsShadow && 0u != m_iAuthoredVisibleInstanceCount)
	{
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::SHADOW,
			static_pointer_cast<CGameObject>(
				shared_from_this()));
	}
}

HRESULT CMapStaticBatchObject::Render()
{
	if (CGameInstance::Get().Is_SceneEnvironmentReplaced())
		return S_OK;

	MAP_CAMERA_CULL_SNAPSHOT cameraSnapshot{};
	const bool_t hasCameraSnapshot =
		CMapAssetRenderUtils::Capture_CameraCullSnapshot(cameraSnapshot);
	if (FAILED(Upload_VisibleInstances(
		hasCameraSnapshot ? &cameraSnapshot : nullptr)))
	{
		return E_FAIL;
	}
	if (m_VisibleInstances.empty())
		return S_OK;

	const HRESULT cameraBindResult = hasCameraSnapshot ?
		CMapAssetRenderUtils::Bind_CameraCullSnapshot(
			m_pShaderCom, cameraSnapshot) :
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
        Build_ScreenLodView(cameraSnapshot, screenLod);
	const bool_t useSourceMaterials =
		CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.BindAndDraw");
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
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Material.Bind");
			if (FAILED(CMapAssetRenderUtils::Bind_Material(m_pModelCom, m_pShaderCom,
				meshIndex, m_RenderProfile, m_fElapsedTime, nullptr, m_AssetId))) return E_FAIL;
		}
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Pass.Apply");
			if (FAILED(m_pShaderCom->Begin(meshPass))) return E_FAIL;
		}
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Mesh.Submit");
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
	// Frame providers can change the light after Late_Update queued this batch.
	if (FAILED(Upload_ShadowInstances()))
		return E_FAIL;
	if (m_ShadowInstances.empty())
		return S_OK;
	const bool_t useSourceMaterials =
		CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;

	if (FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}

	const uint32_t iCullPass =
		CMapAssetRenderUtils::Select_Pass(
			m_RenderProfile, m_bMirrored);
	if (iCullPass > 2u)
		return E_UNEXPECTED;

	const uint32_t iInstanceCount =
		static_cast<uint32_t>(m_ShadowInstances.size());
	for (uint32_t iMesh = 0;
		iMesh < m_pModelCom->Get_NumMeshes(); ++iMesh)
	{
		const auto* surface = m_pModelCom->Get_MaterialSurface(iMesh);
		if (surface && !surface->castsShadow)
			continue;
		// Existing complex families retain their complete source shadow shader.
		uint32_t shadowPassBase = 12u;
		if (!surface)
			shadowPassBase = 18u;
		else
		{
			switch (surface->family)
			{
			case Engine::MODEL_SURFACE_FAMILY::LEGACY:
			case Engine::MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION:
			case Engine::MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION:
				shadowPassBase = 18u;
				break;
			case Engine::MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE:
			case Engine::MODEL_SURFACE_FAMILY::PBR_OPAQUE:
			case Engine::MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE:
				shadowPassBase = useSourceMaterials ? 21u : 18u;
				break;
			default:
				break;
			}
		}
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Material.Bind");
			if (FAILED(CMapAssetRenderUtils::Bind_ShadowMaterial(m_pModelCom, m_pShaderCom,
				iMesh, m_RenderProfile, m_fElapsedTime))) return E_FAIL;
		}
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Pass.Apply");
			if (FAILED(m_pShaderCom->Begin(shadowPassBase + iCullPass))) return E_FAIL;
		}
		{
			Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Mesh.Submit");
			if (FAILED(m_pModelCom->Render_Instanced(iMesh, m_pShadowInstanceBuffer.Get(),
				sizeof(VTXMESHINSTANCE), iInstanceCount))) return E_FAIL;
		}
	}

	return S_OK;
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
	if (current.Visible != instance.Visible)
	{
		if (instance.Visible)
			++m_iAuthoredVisibleInstanceCount;
		else
			--m_iAuthoredVisibleInstanceCount;
	}
	current = instance;
    m_bBatchBoundsDirty = true;
	m_bShadowInstancesDirty = true;
	m_bVisibleInstancesDirty = true;
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
        m_bBatchBoundsDirty = true;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
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
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Visibility");
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
			profiler->Add_Counter(
				Engine::EProfilerCounter::MapVisibleInstances,
				m_VisibleInstances.size());
		}
		return S_OK;
	}

	m_CandidateVisibleInstances.clear();
	bool_t requiresNextCameraTick = false;
    INSTANCE_ENVELOPE visibleEnvelope;
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

	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.CullAndPack");
	if (!rejectBatch) for (FMapStaticInstance& instance :
		m_Instances)
	{
		if (!instance.Visible)
			continue;

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

        visibleEnvelope.Add(instance);
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
	if (payloadUnchanged || m_CandidateVisibleInstances.empty())
	{
        m_VisibleLodBounds = hasLodBounds ? candidateLodBounds : float4_t{};
        m_fVisibleLodScale = hasLodBounds ? visibleEnvelope.maximumScale : 0.f;
		if (!payloadUnchanged)
			m_VisibleInstances.swap(m_CandidateVisibleInstances);
		m_bVisibleInstancesDirty = requiresNextCameraTick;
		m_bVisibleInstancesUsedCamera = hasCameraSnapshot;
		m_iVisibleCameraRevision = cameraRevision;
		return S_OK;
	}

	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.InstanceUpload");
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
	}
    m_VisibleLodBounds = hasLodBounds ? candidateLodBounds : float4_t{};
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
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.ShadowPrepare");
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
		if (!instance.Visible ||
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
		Engine::CProfilerScope uploadScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.ShadowUpload");
		if (FAILED(Ensure_ShadowInstanceCapacity(
			static_cast<uint32_t>(m_CandidateShadowInstances.size()))))
			return E_FAIL;

		D3D11_MAPPED_SUBRESOURCE mapped{};
		if (FAILED(m_pContext->Map(m_pShadowInstanceBuffer.Get(), 0,
			D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
			return E_FAIL;

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
    for (const auto& instance : m_Instances)
    {
        if (!instance.Visible) continue;
        envelope.Add(instance);
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
    const float values[] = { candidate.viewBounds.x, candidate.viewBounds.y, candidate.viewBounds.z,
        candidate.viewBounds.w, candidate.projectionPixels.x, candidate.projectionPixels.y,
        candidate.maximumScale, candidate.nearPlane };
    for (const auto value : values) if (!std::isfinite(value)) return false;
    if (candidate.nearPlane <= 0.f) return false;
    result = candidate;
    return true;
}
