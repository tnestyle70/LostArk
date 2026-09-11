#include "MapStaticBatchObject.h"

#include "GameInstance.h"
#include "MapAssetRenderUtils.h"
#include "Model.h"
#include "Profiler.h"
#include "Shader.h"

#include <algorithm>
#include <cstring>
#include <limits>

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

	if (CGameInstance::Get().Is_ShadowLightEnabled() &&
		SUCCEEDED(Upload_ShadowInstances()) &&
		!m_ShadowInstances.empty())
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

	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.BindAndDraw");
	for (uint32_t meshIndex = 0;
		meshIndex < m_pModelCom->Get_NumMeshes();
		++meshIndex)
	{
		if (FAILED(
			CMapAssetRenderUtils::Bind_Material(
				m_pModelCom,
				m_pShaderCom,
				meshIndex,
				m_RenderProfile,
				m_fElapsedTime, nullptr, m_AssetId)) ||

			FAILED(m_pShaderCom->Begin(
				passIndex)) ||

			FAILED(m_pModelCom->Render_Instanced(
				meshIndex,
				m_pInstanceBuffer.Get(),
				sizeof(VTXMESHINSTANCE),
				instanceCount)))
		{
			return E_FAIL;
		}
	}
	}

	return S_OK;
}

HRESULT CMapStaticBatchObject::Render_Shadow()
{
	if (CGameInstance::Get().Is_SceneEnvironmentReplaced())
		return S_OK;

	constexpr uint32_t STATIC_SHADOW_PASS_BASE = 12u;
	if (m_ShadowInstances.empty() || !m_RenderProfile.castsShadow)
		return S_OK;

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
		if (FAILED(CMapAssetRenderUtils::Bind_Material(
				m_pModelCom, m_pShaderCom, iMesh,
				m_RenderProfile, m_fElapsedTime)) ||
			FAILED(m_pShaderCom->Begin(
				STATIC_SHADOW_PASS_BASE + iCullPass)) ||
			FAILED(m_pModelCom->Render_Instanced(
				iMesh, m_pShadowInstanceBuffer.Get(),
				sizeof(VTXMESHINSTANCE), iInstanceCount)))
		{
			return E_FAIL;
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

	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.CullAndPack");
	for (FMapStaticInstance& instance :
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
	if (payloadUnchanged || m_CandidateVisibleInstances.empty())
	{
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
	if (!m_bShadowInstancesDirty)
		return S_OK;

	m_ShadowInstances.clear();
	for (const FMapStaticInstance& instance : m_Instances)
	{
		if (!instance.Visible)
			continue;

		VTXMESHINSTANCE gpuInstance{};
		gpuInstance.World = instance.World;
		gpuInstance.WorldInvTranspose = instance.WorldInvTranspose;
        gpuInstance.vLightmapScaleBias = instance.BakedLighting.scaleBias;
        gpuInstance.vLightmapAverageScale = instance.BakedLighting.averageScale;
        gpuInstance.vLightmapDirectionalScale = instance.BakedLighting.directionalScale;
        gpuInstance.vStaticShadowScaleBias = instance.BakedLighting.shadowScaleBias;
		m_ShadowInstances.push_back(gpuInstance);
	}

	if (m_ShadowInstances.empty())
	{
		m_bShadowInstancesDirty = false;
		return S_OK;
	}
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.ShadowUpload");
	if (FAILED(Ensure_ShadowInstanceCapacity(
		static_cast<uint32_t>(m_ShadowInstances.size()))))
	{
		return E_FAIL;
	}

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if (FAILED(m_pContext->Map(
		m_pShadowInstanceBuffer.Get(), 0,
		D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		return E_FAIL;
	}

	std::memcpy(
		mapped.pData,
		m_ShadowInstances.data(),
		m_ShadowInstances.size() * sizeof(VTXMESHINSTANCE));
	m_pContext->Unmap(m_pShadowInstanceBuffer.Get(), 0);
	}
	m_bShadowInstancesDirty = false;
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
