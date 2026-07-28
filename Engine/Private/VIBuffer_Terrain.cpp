#include "VIBuffer_Terrain.h"

#include "GameInstance.h"

CVIBuffer_Terrain::CVIBuffer_Terrain(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Terrain::~CVIBuffer_Terrain()
{
}

HRESULT CVIBuffer_Terrain::Initialize_Prototype(const tchar_t* pHeightMapFilePath)
{
	DWORD		dwByte = {};
	HANDLE		hFile = CreateFile(pHeightMapFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (0 == hFile)
		return E_FAIL;

	BITMAPFILEHEADER		fh{};
	BITMAPINFOHEADER		ih{};
	unique_ptr<uint32_t[]>	pPixels = {nullptr};

	ReadFile(hFile, &fh, sizeof fh, &dwByte, nullptr);
	ReadFile(hFile, &ih, sizeof ih, &dwByte, nullptr);

	m_iNumVerticesX = ih.biWidth;
	m_iNumVerticesZ = ih.biHeight;
	m_iNumVertices = m_iNumVerticesX * m_iNumVerticesZ;
	pPixels = make_unique<uint32_t[]>(m_iNumVertices);

	ReadFile(hFile, pPixels.get(), sizeof(uint32_t) * m_iNumVertices, &dwByte, nullptr);

	CloseHandle(hFile);

#pragma region PUBLIC_DATA
	m_iVertexStride = sizeof(VTXNORTEX);
	m_iIndexStride = 4;
	m_iNumIndices = (m_iNumVerticesX - 1) * (m_iNumVerticesZ - 1) * 2 * 3;
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;		 
#pragma endregion

#pragma region VERTEX_BUFFER
	D3D11_BUFFER_DESC		VertexBufferDesc{};
	VertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = m_iVertexStride;	
		
	unique_ptr<VTXNORTEX[]> pVertices = make_unique<VTXNORTEX[]>(m_iNumVertices);
	m_pVertexPositions = make_shared<float3_t[]>(m_iNumVertices);

	for (size_t i = 0; i < m_iNumVerticesZ; i++)
	{
		for (size_t j = 0; j < m_iNumVerticesX; j++)
		{
			size_t		iIndex = i * m_iNumVerticesX + j;

			// pPixels[iIndex] == 11111111 11100111 11100111 11100111
			//					& 00000000 00000000 00000000 11111111
			//					= 00000000 11100111 00000000 00000000


			m_pVertexPositions[iIndex] = pVertices[iIndex].vPosition = float3_t(j, (pPixels[iIndex] & 0x000000ff) * 0.1f, i);
			pVertices[iIndex].vNormal = float3_t(0.f, 0.0f, 0.f);
			pVertices[iIndex].vTexcoord = float2_t(j / (m_iNumVerticesX - 1.f), i / (m_iNumVerticesZ - 1.f));
		}
	}
		

#pragma endregion

	uint32_t        iByte = {};
	hFile = CreateFile(TEXT("../Bin/DataFiles/TerrainNavigation.dat"), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (0 == hFile)
		return E_FAIL;


#pragma region INDEX_BUFFER
	D3D11_BUFFER_DESC		IndexBufferDesc{};
	IndexBufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	IndexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	IndexBufferDesc.MiscFlags = 0;
	IndexBufferDesc.StructureByteStride = m_iIndexStride;

	unique_ptr<uint32_t[]>		pIndices = make_unique<uint32_t[]>(m_iNumIndices);
	uint32_t					iNumIndices = {};

	for (uint32_t i = 0; i < m_iNumVerticesZ - 1; i++)
	{
		for (uint32_t j = 0; j < m_iNumVerticesX - 1; j++)
		{
			uint32_t		iIndex = i * m_iNumVerticesX + j;
			
			uint32_t	iIndices[] = {
				iIndex + m_iNumVerticesX, 
				iIndex + m_iNumVerticesX + 1, 
				iIndex + 1, 
				iIndex
			};

			vector_t		vSour, vDest, vNormal;

			pIndices[iNumIndices++] = iIndices[0];
			pIndices[iNumIndices++] = iIndices[1];
			pIndices[iNumIndices++] = iIndices[2];

			WriteFile(hFile, &m_pVertexPositions[iIndices[0]], sizeof(float3_t), reinterpret_cast<DWORD*>(&iByte), nullptr);
			WriteFile(hFile, &m_pVertexPositions[iIndices[1]], sizeof(float3_t), reinterpret_cast<DWORD*>(&iByte), nullptr);
			WriteFile(hFile, &m_pVertexPositions[iIndices[2]], sizeof(float3_t), reinterpret_cast<DWORD*>(&iByte), nullptr);

			vSour = XMLoadFloat3(&pVertices[iIndices[1]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
			vDest = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[1]].vPosition);
			vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));

			XMStoreFloat3(&pVertices[iIndices[0]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[1]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[1]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[2]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);

			pIndices[iNumIndices++] = iIndices[0];
			pIndices[iNumIndices++] = iIndices[2];
			pIndices[iNumIndices++] = iIndices[3];

			WriteFile(hFile, &m_pVertexPositions[iIndices[0]], sizeof(float3_t), reinterpret_cast<DWORD*>(&iByte), nullptr);
			WriteFile(hFile, &m_pVertexPositions[iIndices[2]], sizeof(float3_t), reinterpret_cast<DWORD*>(&iByte), nullptr); 
			WriteFile(hFile, &m_pVertexPositions[iIndices[3]], sizeof(float3_t), reinterpret_cast<DWORD*>(&iByte), nullptr);

			vSour = XMLoadFloat3(&pVertices[iIndices[2]].vPosition) - XMLoadFloat3(&pVertices[iIndices[0]].vPosition);
			vDest = XMLoadFloat3(&pVertices[iIndices[3]].vPosition) - XMLoadFloat3(&pVertices[iIndices[2]].vPosition);
			vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));

			XMStoreFloat3(&pVertices[iIndices[0]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[0]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[2]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[2]].vNormal) + vNormal);
			XMStoreFloat3(&pVertices[iIndices[3]].vNormal,
				XMLoadFloat3(&pVertices[iIndices[3]].vNormal) + vNormal);
		}
	}

	CloseHandle(hFile);

	for (size_t i = 0; i < m_iNumVertices; i++)
	{
		XMStoreFloat3(&pVertices[i].vNormal,
			XMVector3Normalize(XMLoadFloat3(&pVertices[i].vNormal)));
	}



	D3D11_SUBRESOURCE_DATA			VertexInitialData{};
	VertexInitialData.pSysMem = pVertices.get();

	if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
		return E_FAIL;


	D3D11_SUBRESOURCE_DATA			IndexInitialData{};
	IndexInitialData.pSysMem = pIndices.get();

	if (FAILED(m_pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, &m_pIB)))
		return E_FAIL;

#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_Terrain::Initialize(void* pArg)
{
	return S_OK;
}

void CVIBuffer_Terrain::Culling(fmatrix_t WorldMatrix)
{
	CGameInstance::Get().Update_Frustum_InLocalSpace(WorldMatrix);

	D3D11_MAPPED_SUBRESOURCE			SubResource{};

	m_pContext->Map(m_pIB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);

	auto		pIndices = static_cast<uint32_t*>(SubResource.pData);

	uint32_t	iNumIndices = {};

	for (uint32_t i = 0; i < m_iNumVerticesZ - 1; i++)
	{
		for (uint32_t j = 0; j < m_iNumVerticesX - 1; j++)
		{
			uint32_t		iIndex = i * m_iNumVerticesX + j;

			uint32_t	iIndices[] = {
				iIndex + m_iNumVerticesX,
				iIndex + m_iNumVerticesX + 1,
				iIndex + 1,
				iIndex
			};

			bool_t		isIn[] = {
						CGameInstance::Get().isIn_Frustum_InLocalSpace(
					XMVectorSetW(XMLoadFloat3(&m_pVertexPositions[iIndices[0]]), 1.f), 0.f), 
						CGameInstance::Get().isIn_Frustum_InLocalSpace(
					XMVectorSetW(XMLoadFloat3(&m_pVertexPositions[iIndices[1]]), 1.f), 0.f),
						CGameInstance::Get().isIn_Frustum_InLocalSpace(
					XMVectorSetW(XMLoadFloat3(&m_pVertexPositions[iIndices[2]]), 1.f), 0.f),
						CGameInstance::Get().isIn_Frustum_InLocalSpace(
					XMVectorSetW(XMLoadFloat3(&m_pVertexPositions[iIndices[3]]), 1.f), 0.f),
			};

			if (true == isIn[0] &&
				true == isIn[1] &&
				true == isIn[2])
			{
				pIndices[iNumIndices++] = iIndices[0];
				pIndices[iNumIndices++] = iIndices[1];
				pIndices[iNumIndices++] = iIndices[2];
			}

			if (true == isIn[0] &&
				true == isIn[2] &&
				true == isIn[3])
			{
				pIndices[iNumIndices++] = iIndices[0];
				pIndices[iNumIndices++] = iIndices[2];
				pIndices[iNumIndices++] = iIndices[3];
			}
		}
	}

	m_pContext->Unmap(m_pIB.Get(), 0);

	m_iNumIndices = iNumIndices;
}

unique_ptr<CVIBuffer_Terrain> CVIBuffer_Terrain::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pHeightMapFilePath)
{
	auto pInstance = unique_ptr<CVIBuffer_Terrain>(new CVIBuffer_Terrain(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pHeightMapFilePath)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Terrain");
		return nullptr;
	}

	return pInstance;
}


shared_ptr<CPrototype> CVIBuffer_Terrain::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CVIBuffer_Terrain>(new CVIBuffer_Terrain(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CVIBuffer_Terrain");
		return nullptr;
	}

	return pInstance;
}

