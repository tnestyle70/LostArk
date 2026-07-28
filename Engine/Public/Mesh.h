#pragma once

#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CMesh final : public CVIBuffer
{
private:
	CMesh(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CMesh();

public:
	uint32_t Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_Resource(shared_ptr<class CShader> pShader, const char_t* pConstantName, const vector<shared_ptr<class CBone>>& Bones);

private:
	char_t					m_szName[MAX_PATH] = {};
	uint32_t				m_iMaterialIndex = {};
	uint32_t				m_iNumBones = {};
	vector<uint32_t>		m_BoneIndices;
	vector<float4x4_t>		m_OffsetMatrices;
	float4x4_t				m_BoneMatrices[512];


private:
	HRESULT Ready_VertexBuffer_NonAnim(const aiMesh* pAIMesh, fmatrix_t PreTransformMatrix);
	HRESULT Ready_VertexBuffer_Anim(const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones);

public:
	static shared_ptr<CMesh> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END