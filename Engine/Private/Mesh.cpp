#include "Mesh.h"
#include "BinaryAsset/ModelAssetData.h"
#include "Bone.h"

#include "Shader.h"
#include "GameInstance.h"
#include "Profiler.h"

CMesh::CMesh(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CMesh::~CMesh()
{
}

HRESULT CMesh::Initialize_Prototype(MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix)
{
#pragma region PUBLIC_DATA
	strcpy_s(m_szName, pAIMesh->mName.C_Str());
	m_iMaterialIndex = pAIMesh->mMaterialIndex;
	m_iVertexStride = MODEL::NONANIM == eType ? sizeof(VTXMESH) : sizeof(VTXANIMMESH);
	m_iNumVertices = pAIMesh->mNumVertices;
	m_iIndexStride = 4;
	m_iNumIndices = pAIMesh->mNumFaces * 3;
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
#pragma endregion

#pragma region VERTEX_BUFFER
	HRESULT hr = MODEL::NONANIM == eType ? 
		Ready_VertexBuffer_NonAnim(pAIMesh, PreTransformMatrix) : 
		Ready_VertexBuffer_Anim(pAIMesh, Bones);

	if (FAILED(hr))
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

	unique_ptr<uint32_t[]>		pIndices = make_unique<uint32_t[]>(m_iNumIndices);

	uint32_t	iNumIndices = {};

	for (uint32_t i = 0; i < pAIMesh->mNumFaces; i++)
	{
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[0];
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[1];
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[2];
	}

	D3D11_SUBRESOURCE_DATA			IndexInitialData{};
	IndexInitialData.pSysMem = pIndices.get();

	if (FAILED(m_pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, &m_pIB)))
		return E_FAIL;

#pragma endregion

	return S_OK;
}

HRESULT CMesh::Initialize_Prototype(MODEL eType, const MODEL_MESH_DATA& mesh,
	const MODEL_SKELETON_DATA& skeleton, fmatrix_t PreTransformMatrix)
{
	const bool_t isAnimated = MODEL::ANIM == eType;
	if (mesh.name.size() >= MAX_PATH || mesh.indices.empty() ||
		(isAnimated && mesh.vertexKind != MODEL_VERTEX_KIND::SKINNED) ||
		(!isAnimated && mesh.vertexKind != MODEL_VERTEX_KIND::STATIC))
		return E_FAIL;

	strcpy_s(m_szName, mesh.name.c_str());
	m_iMaterialIndex = mesh.materialIndex;
	m_iVertexStride = isAnimated ? sizeof(VTXANIMMESH) : sizeof(VTXMESH);
	m_iNumVertices = isAnimated
		? static_cast<uint32_t>(mesh.skinnedVertices.size())
		: static_cast<uint32_t>(mesh.vertices.size());
	m_iIndexStride = sizeof(uint32_t);
	m_iNumIndices = static_cast<uint32_t>(mesh.indices.size());
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	if (0 == m_iNumVertices)
		return E_FAIL;

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = m_iVertexStride;
	D3D11_SUBRESOURCE_DATA vertexInitialData{};

	vector<VTXMESH> staticVertices;
	if (isAnimated)
	{
		vertexInitialData.pSysMem = mesh.skinnedVertices.data();
		if (skeleton.bones.empty() || skeleton.bones.size() > 512)
			return E_FAIL;

		m_bUsesSkeletonPalette = true;
		m_iNumBones = static_cast<uint32_t>(skeleton.bones.size());
		m_BoneIndices.reserve(m_iNumBones);
		m_OffsetMatrices.reserve(m_iNumBones);
		for (uint32_t i = 0; i < m_iNumBones; ++i)
		{
			m_BoneIndices.push_back(i);
			m_OffsetMatrices.push_back(skeleton.bones[i].inverseBind);
		}
	}
	else
	{
		if ((mesh.hasColor0 && mesh.color0Rgba8.size() != mesh.vertices.size()) ||
			(!mesh.hasColor0 && !mesh.color0Rgba8.empty()))
			return E_INVALIDARG;
		staticVertices = mesh.vertices;
		for (size_t index = 0; index < staticVertices.size(); ++index)
		{
			VTXMESH& vertex = staticVertices[index];
			// The WModel reader already decoded RGBA; upload without a second swizzle.
			vertex.color0Rgba8 = mesh.hasColor0 ? mesh.color0Rgba8[index] : 0xffffffffu;
			XMStoreFloat3(&vertex.vPosition,
				XMVector3TransformCoord(XMLoadFloat3(&vertex.vPosition), PreTransformMatrix));
			XMStoreFloat3(&vertex.vNormal,
				XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vNormal), PreTransformMatrix)));
			XMStoreFloat3(&vertex.vTangent,
				XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vTangent), PreTransformMatrix)));
			XMStoreFloat3(&vertex.vBinormal,
				XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vBinormal), PreTransformMatrix)));
		}
		vertexInitialData.pSysMem = staticVertices.data();
	}

	if (FAILED(m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexInitialData, &m_pVB)))
		return E_FAIL;

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.StructureByteStride = m_iIndexStride;
	D3D11_SUBRESOURCE_DATA indexInitialData{};
	indexInitialData.pSysMem = mesh.indices.data();
	if (FAILED(m_pDevice->CreateBuffer(&indexBufferDesc, &indexInitialData, &m_pIB)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMesh::Initialize(void* pArg)
{
	return S_OK;
}

void CMesh::Build_SkinPalette(const vector<shared_ptr<class CBone>>& Bones,
    float4x4_t* pOutMatrices) const
{
    for (uint32_t i = 0; i < m_iNumBones; ++i)
    {
        XMStoreFloat4x4(&pOutMatrices[i],
            XMLoadFloat4x4(&m_OffsetMatrices[i]) *
            Bones[m_BoneIndices[i]]->Get_CombinedTransformationMatrix());
    }
}

HRESULT CMesh::Render_Instanced(ID3D11Buffer* pInstanceBuffer,
	uint32_t iInstanceStride, uint32_t iNumInstances,
	uint32_t iInstanceByteOffset)
{
	if (nullptr == pInstanceBuffer ||
		0 == iInstanceStride ||
		0 == iNumInstances)
	{
		return E_INVALIDARG;
	}

	D3D11_BUFFER_DESC instanceDesc{};
	pInstanceBuffer->GetDesc(&instanceDesc);
	const uint64_t requiredBytes = static_cast<uint64_t>(iInstanceByteOffset) +
		static_cast<uint64_t>(iInstanceStride) * iNumInstances;
	if (nullptr == m_pVB || nullptr == m_pIB || 0u == m_iNumIndices ||
		0u == (instanceDesc.BindFlags & D3D11_BIND_VERTEX_BUFFER) ||
		0u != (iInstanceByteOffset % sizeof(f32_t)) ||
		requiredBytes > instanceDesc.ByteWidth)
		return E_INVALIDARG;

	ID3D11Buffer* vertexBuffers[] =
	{
		m_pVB.Get(), pInstanceBuffer
	};

	const uint32_t strides[] =
	{
		m_iVertexStride,
		iInstanceStride
	};

	const uint32_t offsets[] =
	{
		0,
		iInstanceByteOffset
	};

	m_pContext->IASetVertexBuffers(
		0, 2, vertexBuffers, strides, offsets
	);

	m_pContext->IASetIndexBuffer(
		m_pIB.Get(),
		m_eIndexFormat,
		0
	);

	m_pContext->IASetPrimitiveTopology(
		m_ePrimitiveTopology);

	if (CProfiler* pProfiler =
		CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Add_Counter(
			EProfilerCounter::DrawCalls);

		pProfiler->Add_Counter(
			EProfilerCounter::InstancedDrawCalls);

		pProfiler->Add_Counter(
			EProfilerCounter::Instances,
			iNumInstances);

		pProfiler->Add_Counter(
			EProfilerCounter::Indices,
			static_cast<uint64_t>(m_iNumIndices) *
			iNumInstances);
	}

	m_pContext->DrawIndexedInstanced(
		m_iNumIndices,
		iNumInstances,
		0,
		0,
		0);

	return S_OK;
}

HRESULT CMesh::Ready_VertexBuffer_NonAnim(const aiMesh* pAIMesh, fmatrix_t PreTransformMatrix)
{
	D3D11_BUFFER_DESC		VertexBufferDesc{};
	VertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = m_iVertexStride;

	unique_ptr<VTXMESH[]> pVertices = make_unique<VTXMESH[]>(m_iNumVertices);

	for (uint32_t i = 0; i < m_iNumVertices; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vPosition,
			XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));

		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vNormal,
			XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));

		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vTangent,
			XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));

		memcpy(&pVertices[i].vBinormal, &pAIMesh->mBitangents[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vBinormal,
			XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));

		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(float2_t));
		if (pAIMesh->HasTextureCoords(1))
			memcpy(&pVertices[i].vTexcoord1, &pAIMesh->mTextureCoords[1][i], sizeof(float2_t));
	}

	D3D11_SUBRESOURCE_DATA			VertexInitialData{};
	VertexInitialData.pSysMem = pVertices.get();

	if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
		return E_FAIL;


	return S_OK;
}

HRESULT CMesh::Ready_VertexBuffer_Anim(const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones)
{
	D3D11_BUFFER_DESC		VertexBufferDesc{};
	VertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = m_iVertexStride;

	unique_ptr<VTXANIMMESH[]> pVertices = make_unique<VTXANIMMESH[]>(m_iNumVertices);

	for (uint32_t i = 0; i < m_iNumVertices; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(float3_t));
		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(float3_t));		
		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(float3_t));		
		memcpy(&pVertices[i].vBinormal, &pAIMesh->mBitangents[i], sizeof(float3_t));		
		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(float2_t));
		if (pAIMesh->HasTextureCoords(1))
			memcpy(&pVertices[i].vTexcoord1, &pAIMesh->mTextureCoords[1][i], sizeof(float2_t));
		if (pAIMesh->HasTextureCoords(2))
			memcpy(&pVertices[i].vTexcoord2, &pAIMesh->mTextureCoords[2][i], sizeof(float2_t));
	}

	m_iNumBones = pAIMesh->mNumBones;



	for (uint32_t i = 0; i < m_iNumBones; i++)
	{
		aiBone* pAIBone = pAIMesh->mBones[i];
		int32_t	iTotalBoneIndex = { -1 };
		float4x4_t	OffsetMatrix = {};

		memcpy(&OffsetMatrix, &pAIBone->mOffsetMatrix, sizeof(float4x4_t));
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));


		m_OffsetMatrices.push_back(OffsetMatrix);


		auto	iter = find_if(Bones.begin(), Bones.end(), [&](shared_ptr<class CBone> pBone)->bool_t {
			++iTotalBoneIndex;
			return pBone->Compare_Name(pAIBone->mName.C_Str());
			});

		if (iter == Bones.end())
			return E_FAIL;

		m_BoneIndices.push_back(iTotalBoneIndex);

		// pAIBone->mNumWeights : 이 뼈가 몇개 정점에게 영향을 주는가? 
		for (uint32_t j = 0; j < pAIBone->mNumWeights; j++)
		{
			// pAIBone->mWeights[j].mVertexId : i번째 뼈가 영향을 주는 정점들중 j번째 영향을 주는 정점의 인덱스 
			if(0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.x)
			{ 
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.x = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.x = pAIBone->mWeights[j].mWeight;
			}

			else if (0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.y)
			{
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.y = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.y = pAIBone->mWeights[j].mWeight;
			}

			else if (0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.z)
			{
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.z = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.z = pAIBone->mWeights[j].mWeight;
			}

			else if (0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.w)
			{
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.w = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.w = pAIBone->mWeights[j].mWeight;
			}
		}
	}

	if (0 == m_iNumBones)
	{
		m_iNumBones = 1;
		int32_t	iTotalBoneIndex = { -1 };
		float4x4_t	OffsetMatrix = {};

		auto	iter = find_if(Bones.begin(), Bones.end(), [&](shared_ptr<class CBone> pBone)->bool_t {
			++iTotalBoneIndex;
			return pBone->Compare_Name(m_szName);
		});

		if (iter == Bones.end())
			return E_FAIL;

		m_BoneIndices.push_back(iTotalBoneIndex);

		XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());

		m_OffsetMatrices.push_back(OffsetMatrix);
	}

	D3D11_SUBRESOURCE_DATA			VertexInitialData{};
	VertexInitialData.pSysMem = pVertices.get();

	if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
		return E_FAIL;


	return S_OK;
}

shared_ptr<CMesh> CMesh::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix)
{
	auto pInstance = shared_ptr<CMesh>(new CMesh(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(eType, pAIMesh, Bones, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CMesh");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CMesh> CMesh::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext, MODEL eType,
	const MODEL_MESH_DATA& mesh, const MODEL_SKELETON_DATA& skeleton,
	fmatrix_t PreTransformMatrix)
{
	auto pInstance = shared_ptr<CMesh>(new CMesh(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype(
		eType, mesh, skeleton, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CMesh");
		return nullptr;
	}
	return pInstance;
}


bool_t CMesh::Get_MorphBaseVertex(uint32_t iIndex, float3_t& OutPosition, float3_t& OutNormal) const
{
	if (!Has_MorphBaseVertices() || iIndex >= m_iNumVertices)
		return false;
	OutPosition = (*m_pMorphBasePositions)[iIndex];
	OutNormal = (*m_pMorphBaseNormals)[iIndex];
	return true;
}

HRESULT CMesh::Make_VertexBuffer_Unique()
{
	if (m_hasUniqueVertexBuffer)
		return S_OK;
	if (0u == m_iNumVertices || nullptr == m_pVB)
		return E_FAIL;

	const uint32_t iByteWidth = m_iVertexStride * m_iNumVertices;

	/* The unmorphed rest position/normal has to come from somewhere, and the vertex buffer
	is D3D11_USAGE_DEFAULT with CPUAccessFlags 0, so it cannot be Map()ed directly. Copying
	it into a staging buffer first is the one supported readback path, and doing it here --
	once, for the one character that actually opens the face editor -- costs nothing for
	every other model, which is why nothing is retained at load time. */
	D3D11_BUFFER_DESC stagingDesc{};
	stagingDesc.ByteWidth = iByteWidth;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.BindFlags = 0;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.StructureByteStride = m_iVertexStride;

	ComPtr<ID3D11Buffer> pStaging;
	if (FAILED(m_pDevice->CreateBuffer(&stagingDesc, nullptr, &pStaging)))
		return E_FAIL;
	m_pContext->CopyResource(pStaging.Get(), m_pVB.Get());

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if (FAILED(m_pContext->Map(pStaging.Get(), 0, D3D11_MAP_READ, 0, &mapped)) ||
		nullptr == mapped.pData)
	{
		return E_FAIL;
	}

	auto pPositions = make_shared<vector<float3_t>>(m_iNumVertices);
	auto pNormals = make_shared<vector<float3_t>>(m_iNumVertices);
	const uint8_t* pVertices = static_cast<const uint8_t*>(mapped.pData);
	for (uint32_t i = 0; i < m_iNumVertices; ++i)
	{
		/* vPosition sits at offset 0 and vNormal at 12 in both VTXMESH and VTXANIMMESH. */
		const uint8_t* pVertex = pVertices + static_cast<size_t>(i) * m_iVertexStride;
		memcpy(&(*pPositions)[i], pVertex, sizeof(float3_t));
		memcpy(&(*pNormals)[i], pVertex + sizeof(float3_t), sizeof(float3_t));
	}
	m_pContext->Unmap(pStaging.Get(), 0);

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = iByteWidth;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = m_iVertexStride;

	ComPtr<ID3D11Buffer> pUniqueVB;
	if (FAILED(m_pDevice->CreateBuffer(&vertexBufferDesc, nullptr, &pUniqueVB)))
		return E_FAIL;

	/* GPU-side copy of the buffer this instance still shares with the prototype and its
	siblings, so the new one starts identical -- every field, not just the position/normal
	this class knows how to touch. */
	m_pContext->CopyResource(pUniqueVB.Get(), m_pVB.Get());

	m_pMorphBasePositions = pPositions;
	m_pMorphBaseNormals = pNormals;
	m_pVB = pUniqueVB;
	m_hasUniqueVertexBuffer = true;
	m_TouchedVertexIndices.clear();
	return S_OK;
}

HRESULT CMesh::Update_Vertices(const vector<uint32_t>& iIndices,
	const vector<float3_t>& Positions, const vector<float3_t>& Normals)
{
	if (!m_hasUniqueVertexBuffer)
		return E_FAIL;
	if (iIndices.size() != Positions.size() || iIndices.size() != Normals.size())
		return E_INVALIDARG;

	for (size_t i = 0; i < iIndices.size(); ++i)
	{
		const uint32_t iVertex = iIndices[i];
		if (iVertex >= m_iNumVertices)
			return E_INVALIDARG;

		const f32_t PositionNormal[6] = {
			Positions[i].x, Positions[i].y, Positions[i].z,
			Normals[i].x, Normals[i].y, Normals[i].z,
		};

		D3D11_BOX Box{};
		Box.left = iVertex * m_iVertexStride;
		Box.right = Box.left + sizeof(PositionNormal);
		Box.top = 0u;
		Box.bottom = 1u;
		Box.front = 0u;
		Box.back = 1u;

		/* vPosition and vNormal are adjacent (offset 0 and 12) in both VTXMESH and
		VTXANIMMESH, so one 24-byte box covers both; every other field (tangent, binormal,
		UV, blend indices/weights) is outside this box and is left untouched. */
		m_pContext->UpdateSubresource(m_pVB.Get(), 0, &Box, PositionNormal, 0, 0);
		m_TouchedVertexIndices.push_back(iVertex);
	}
	return S_OK;
}

HRESULT CMesh::Reset_Vertices()
{
	if (!m_hasUniqueVertexBuffer || m_TouchedVertexIndices.empty())
		return S_OK;
	if (!Has_MorphBaseVertices())
		return E_FAIL;

	for (uint32_t iVertex : m_TouchedVertexIndices)
	{
		const float3_t& BasePosition = (*m_pMorphBasePositions)[iVertex];
		const float3_t& BaseNormal = (*m_pMorphBaseNormals)[iVertex];
		const f32_t PositionNormal[6] = {
			BasePosition.x, BasePosition.y, BasePosition.z,
			BaseNormal.x, BaseNormal.y, BaseNormal.z,
		};

		D3D11_BOX Box{};
		Box.left = iVertex * m_iVertexStride;
		Box.right = Box.left + sizeof(PositionNormal);
		Box.top = 0u;
		Box.bottom = 1u;
		Box.front = 0u;
		Box.back = 1u;

		m_pContext->UpdateSubresource(m_pVB.Get(), 0, &Box, PositionNormal, 0, 0);
	}
	m_TouchedVertexIndices.clear();
	return S_OK;
}

shared_ptr<CPrototype> CMesh::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CMesh>(new CMesh(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMesh");
		return nullptr;
	}

	return pInstance;
}

