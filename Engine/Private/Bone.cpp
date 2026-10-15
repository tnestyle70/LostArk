// 본의 local 포즈와 계층 누적 행렬을 구현한다.
#include "Bone.h"
// aiNode의 이름·초기 행렬을 읽는다.
#include "Assimp/scene.h"

// 변환된 WModel 본의 parentIndex와 restLocal 계약을 소비한다.
#include "BinaryAsset/ModelAssetData.h"

CBone::CBone()
{
}

CBone::~CBone()
{
}

// 모델의 Assimp 계층 순회가 정한 부모 index를 저장한다. local은 Assimp 행렬을 전치해 엔진 행렬 배치로 옮긴다.
HRESULT CBone::Initialize(const aiNode* pAINode, int32_t iParentBoneIndex)
{
	m_iParentBoneIndex = iParentBoneIndex;

	strcpy_s(m_szName, pAINode->mName.C_Str());

	memcpy(&m_TransformationMatrix, &pAINode->mTransformation, sizeof(float4x4_t));
	XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));

	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());

	return S_OK;
}

// 이름 검사 실패는 기존 본 값을 바꾸지 않는다. 성공 시 restLocal은 이미 엔진 형식이므로 다시 전치하지 않는다.
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

// 애니메이션 채널 또는 모델의 명시 포즈 적용이 local 값만 교체한다.
void CBone::Update_TransformationMatrix(fmatrix_t TransformationMatrix)
{
	XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
}

// 모델 전환의 이전 포즈와 이번 포즈를 보간한다. 두 행렬 중 하나라도 분해되지 않으면 아무 행렬도 저장하지 않는다.
void CBone::Blend_TransformationMatrix(fmatrix_t FromMatrix, f32_t fWeight)
{
	vector_t vFromScale{}, vFromRotation{}, vFromTranslation{};
	vector_t vToScale{}, vToRotation{}, vToTranslation{};

	if (!XMMatrixDecompose(&vFromScale, &vFromRotation, &vFromTranslation, FromMatrix) ||
		!XMMatrixDecompose(&vToScale, &vToRotation, &vToTranslation,
			XMLoadFloat4x4(&m_TransformationMatrix)))
		return;

	// scale/translation은 선형, rotation은 quaternion slerp로 결합한다. combined 행렬은 다음 계층 갱신에서 다시 만든다.
	XMStoreFloat4x4(&m_TransformationMatrix,
		XMMatrixAffineTransformation(
			XMVectorLerp(vFromScale, vToScale, fWeight),
			XMVectorZero(),
			XMQuaternionSlerp(vFromRotation, vToRotation, fWeight),
			XMVectorLerp(vFromTranslation, vToTranslation, fWeight)));
}

// 모델이 부모→자식 순서로 호출한다. 루트에만 PreTransform을 곱하고 자식은 부모에 누적된 변환을 이어받는다.
void CBone::Update_CombinedTransformationMatrix(
	const vector<shared_ptr<CBone>>& Bones,
	fmatrix_t PreTransformMatrix)
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

// 새 본의 초기화가 실패하면 shared_ptr가 임시 객체를 정리한다.
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

// 검증에 성공한 변환 모델 본만 호출자에게 넘긴다.
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

// 본에는 외부 자원 소유 멤버가 없으므로 기본 복사로 독립 포즈를 만든다.
shared_ptr<CBone> CBone::Clone()
{
	return shared_ptr<CBone>(new CBone(*this));
}

