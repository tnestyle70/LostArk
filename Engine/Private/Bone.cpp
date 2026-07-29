#include "Bone.h"

#include "BinaryAsset/ModelAssetData.h"

CBone::CBone()
{
	
}

CBone::~CBone()
{
}

HRESULT CBone::Initialize(const aiNode* pAINode, int32_t iParentBoneIndex)
{
	m_iParentBoneIndex = iParentBoneIndex;

	strcpy_s(m_szName, pAINode->mName.C_Str());

	memcpy(&m_TransformationMatrix, &pAINode->mTransformation, sizeof(float4x4_t));
	XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));

	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());

	return S_OK;
}

HRESULT CBone::Initialize(const MODEL_BONE_DATA& bone)
{
	if (bone.name.empty() || bone.name.size() >= MAX_PATH)
		return E_FAIL;

	strcpy_s(m_szName, bone.name.c_str());
	m_iParentBoneIndex = bone.parentIndex;
	m_TransformationMatrix = bone.restLocal;
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());
	return S_OK;
}

void CBone::Update_TransformationMatrix(fmatrix_t TransformationMatrix)
{
	XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
}

void CBone::Update_CombinedTransformationMatrix(const vector<shared_ptr<CBone>>& Bones, fmatrix_t PreTransformMatrix)
{
	if (-1 == m_iParentBoneIndex)
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix,
			XMLoadFloat4x4(&m_TransformationMatrix) * PreTransformMatrix);
	}

	else
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix,
			XMLoadFloat4x4(&m_TransformationMatrix) *
			XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
	}
}

shared_ptr<CBone> CBone::Create(const aiNode* pAINode, int32_t iParentBoneIndex)
{
	auto pInstance = shared_ptr<CBone>(new CBone());

	if (FAILED(pInstance->Initialize(pAINode, iParentBoneIndex)))
	{
		MSG_BOX("Failed to Created : CBone");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CBone> CBone::Create(const MODEL_BONE_DATA& bone)
{
	auto pInstance = shared_ptr<CBone>(new CBone());
	if (FAILED(pInstance->Initialize(bone)))
	{
		MSG_BOX("Failed to Created : CBone");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CBone> CBone::Clone()
{
	return shared_ptr<CBone>(new CBone(*this));
}

