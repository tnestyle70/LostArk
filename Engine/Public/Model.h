#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CModel final : public CComponent
{
private:
	CModel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	CModel(const CModel& Prototype);
public:
	virtual ~CModel();

public:
	uint32_t Get_NumMeshes() const {
		return m_iNumMeshes;
	}

	matrix_t Get_BoneMatrix(const char_t* pBoneName);

	void Set_Animation(uint32_t iAnimIndex, bool_t isLoop = false) {
		if (iAnimIndex >= m_iNumAnimations)
			return;
		m_isAnimLoop = isLoop;
		m_iCurrentAnimIndex = iAnimIndex;
	}

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Render(uint32_t iMeshIndex);
	bool_t Play_Animation(f32_t fTimeDelta);
	HRESULT Bind_BoneMatrices(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex);
	HRESULT Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType, uint32_t iTextureIndex = 0);

private:
	const aiScene*						m_pAIScene = { nullptr };
	Assimp::Importer					m_Importer = {};

private:
	MODEL								m_eType = { MODEL::END };
	uint32_t							m_iNumMeshes = {};
	vector<shared_ptr<class CMesh>>		m_Meshes;
	float4x4_t							m_PreTransformMatrix = {};

	uint32_t							m_iNumMaterials = {};
	vector<shared_ptr<class CMaterial>>	m_Materials;

	vector<shared_ptr<class CBone>>		m_Bones;

	uint32_t								m_iCurrentAnimIndex = {};
	uint32_t								m_iNumAnimations = {};
	vector<shared_ptr<class CAnimation>>	m_Animations;
	bool_t									m_isAnimLoop = { false };

private:
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const char_t* pModelFilePath);
	HRESULT Ready_Bones(const aiNode* pAINode, int32_t iParentBoneIndex = -1);
	HRESULT Ready_Animations();

public:
	static unique_ptr<CModel> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	void Free();
};

NS_END