#include "VIBuffer_Instance_Rect.h"
#include "GameInstance.h"

CVIBuffer_Instance_Rect::CVIBuffer_Instance_Rect(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer_Instance { pDevice, pContext }
{
}

CVIBuffer_Instance_Rect::~CVIBuffer_Instance_Rect()
{
}

HRESULT CVIBuffer_Instance_Rect::Initialize_Prototype(INSTANCE_DESC* pArg)
{
	auto			pDesc = static_cast<CVIBuffer_Instance_Rect::INSTANCE_PARTICLE_DESC*>(pArg);

#pragma region PUBLIC_DATA
	m_iVertexStride = sizeof(VTXTEX);
	m_iNumVertices = 4;
	m_iIndexStride = 2;
	m_iNumIndices = 6;
	m_iNumVertexBuffers = 2;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_iNumInstances = pDesc->iNumInstance;
	m_iNumIndexPerInstance = m_iNumIndices;
	m_iVertexInstanceStride = sizeof(VTXINSTANCE_PARTICLE);
#pragma endregion

#pragma region VERTEX_BUFFER
	D3D11_BUFFER_DESC		VertexBufferDesc{};
	VertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = m_iVertexStride;	


	// unique_ptr<VTXTEX[]> pVertices = unique_ptr<VTXTEX[]>(new VTXTEX[m_iNumVertices]());
	unique_ptr<VTXTEX[]> pVertices = make_unique<VTXTEX[]>(m_iNumVertices);
		
	pVertices[0].vPosition = float3_t(-0.5f, 0.5f, 0.f);
	pVertices[0].vTexcoord = float2_t(0.f, 0.f);

	pVertices[1].vPosition = float3_t(0.5f, 0.5f, 0.f);
	pVertices[1].vTexcoord = float2_t(1.f, 0.f);

	pVertices[2].vPosition = float3_t(0.5f, -0.5f, 0.f);
	pVertices[2].vTexcoord = float2_t(1.f, 1.f);

	pVertices[3].vPosition = float3_t(-0.5f, -0.5f, 0.f);
	pVertices[3].vTexcoord = float2_t(0.f, 1.f);

	D3D11_SUBRESOURCE_DATA			VertexInitialData{};
	VertexInitialData.pSysMem = pVertices.get();

	if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
		return E_FAIL;

#pragma endregion

#pragma region INDEX_BUFFER
	D3D11_BUFFER_DESC		IndexBufferDesc{};
	IndexBufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	IndexBufferDesc.StructureByteStride = m_iIndexStride;

	unique_ptr<uint16_t[]>		pIndices = make_unique<uint16_t[]>(m_iNumIndices);
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;

	pIndices[3] = 0;
	pIndices[4] = 2;
	pIndices[5] = 3;

	D3D11_SUBRESOURCE_DATA			IndexInitialData{};
	IndexInitialData.pSysMem = pIndices.get();

	if (FAILED(m_pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, &m_pIB)))
		return E_FAIL;

#pragma endregion


#pragma region INSTANCE_BUFFER
	
	m_InstanceBufferDesc.ByteWidth = m_iVertexInstanceStride * m_iNumInstances;
	m_InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_InstanceBufferDesc.MiscFlags = 0;
	m_InstanceBufferDesc.StructureByteStride = m_iVertexInstanceStride;
	
	m_pInstances = make_shared<VTXINSTANCE_PARTICLE[]>(m_iNumInstances);
	m_pSpeeds = make_shared<f32_t[]>(m_iNumInstances);
	m_isLoop = pDesc->isLoop;

	for (uint32_t i = 0; i < m_iNumInstances; i++)
	{
		f32_t		fSize = CGameInstance::Get().Random(pDesc->vSize.x, pDesc->vSize.y);
		m_pSpeeds[i] = CGameInstance::Get().Random(pDesc->vSpeed.x, pDesc->vSpeed.y);

		/*XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw());		*/

		XMStoreFloat4(&m_pInstances[i].vRight, XMVectorSet(1.f, 0.f, 0.f, 0.f) * fSize);
		XMStoreFloat4(&m_pInstances[i].vUp, XMVectorSet(0.f, 1.f, 0.f, 0.f) * fSize);
		XMStoreFloat4(&m_pInstances[i].vLook, XMVectorSet(0.f, 0.f, 1.f, 0.f) * fSize);

		m_pInstances[i].vTranslation = float4_t(
			CGameInstance::Get().Random(pDesc->vCenter.x - pDesc->vRange.x * 0.5f, pDesc->vCenter.x + pDesc->vRange.x * 0.5f),
			CGameInstance::Get().Random(pDesc->vCenter.y - pDesc->vRange.y * 0.5f, pDesc->vCenter.y + pDesc->vRange.y * 0.5f),
			CGameInstance::Get().Random(pDesc->vCenter.z - pDesc->vRange.z * 0.5f, pDesc->vCenter.z + pDesc->vRange.z * 0.5f),
			1.f);

		m_pInstances[i].vLifeTime = float2_t(0.f, CGameInstance::Get().Random(pDesc->vLifeTime.x, pDesc->vLifeTime.y));
	}

	
#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_Instance_Rect::Initialize(void* pArg)
{
	D3D11_SUBRESOURCE_DATA			InstanceInitialData{};
	InstanceInitialData.pSysMem = m_pInstances.get();

	if (FAILED(m_pDevice->CreateBuffer(&m_InstanceBufferDesc, &InstanceInitialData, &m_pVBInstance)))
		return E_FAIL;


	return S_OK;
}

void CVIBuffer_Instance_Rect::Drop(f32_t fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance.Get(), 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	auto		pVertices = static_cast<VTXINSTANCE_PARTICLE*>(SubResource.pData);

	for (uint32_t i = 0; i < m_iNumInstances; i++)
	{
		pVertices[i].vTranslation.y -= m_pSpeeds[i] * fTimeDelta;
		pVertices[i].vLifeTime.x += fTimeDelta;

		if (true == m_isLoop 
			&& pVertices[i].vLifeTime.x >= pVertices[i].vLifeTime.y)
		{
			pVertices[i].vLifeTime.x = 0.f;
			pVertices[i].vTranslation = m_pInstances[i].vTranslation;
		}
	}

	m_pContext->Unmap(m_pVBInstance.Get(), 0);
}

unique_ptr<CVIBuffer_Instance_Rect> CVIBuffer_Instance_Rect::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, INSTANCE_DESC* pArg)
{
	auto pInstance = unique_ptr<CVIBuffer_Instance_Rect>(new CVIBuffer_Instance_Rect(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pArg)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Instance_Rect");
		return nullptr;
	}

	return pInstance;
}


shared_ptr<CPrototype> CVIBuffer_Instance_Rect::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CVIBuffer_Instance_Rect>(new CVIBuffer_Instance_Rect(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CVIBuffer_Instance_Rect");
		return nullptr;
	}

	return pInstance;
}

