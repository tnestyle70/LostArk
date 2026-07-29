#include "Mesh.h"
#include "BinaryAsset/ModelAssetData.h"
#include "Bone.h"

#include "Shader.h"

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
		staticVertices = mesh.vertices;
		for (VTXMESH& vertex : staticVertices)
		{
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

HRESULT CMesh::Bind_Resource(shared_ptr<class CShader> pShader, const char_t* pConstantName, const vector<shared_ptr<class CBone>>& Bones)
{
	ZeroMemory(m_BoneMatrices, sizeof(float4x4_t) * 512);

	for (uint32_t i = 0; i < m_iNumBones; i++)
	{
		XMStoreFloat4x4(&m_BoneMatrices[i], 
			XMLoadFloat4x4(&m_OffsetMatrices[i]) * 
			Bones[m_BoneIndices[i]]->Get_CombinedTransformationMatrix());
	}

	return pShader->Bind_Matrices(pConstantName, m_BoneMatrices, m_iNumBones);	
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

