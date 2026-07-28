#include "Channel.h"
#include "Bone.h"

CChannel::CChannel()
{
}

CChannel::~CChannel()
{
}

HRESULT CChannel::Initialize(const aiNodeAnim* pAIChannel, const vector<shared_ptr<class CBone>>& Bones)
{
	strcpy_s(m_szName, pAIChannel->mNodeName.C_Str());

	m_iNumKeyFrames = max(pAIChannel->mNumScalingKeys, pAIChannel->mNumRotationKeys);
	m_iNumKeyFrames = max(m_iNumKeyFrames, pAIChannel->mNumPositionKeys);

	float3_t		vScale{};
	float4_t		vRotation{};
	float3_t		vTranslation{};


	for (uint32_t i = 0; i < m_iNumKeyFrames; i++)
	{
		KEYFRAME			KeyFrame{};

		if (i < pAIChannel->mNumScalingKeys)
		{
			memcpy(&vScale, &pAIChannel->mScalingKeys[i].mValue, sizeof(float3_t));
			KeyFrame.fTrackPosition = pAIChannel->mScalingKeys[i].mTime;
		}
		if (i < pAIChannel->mNumRotationKeys)
		{
			/*memcpy(&vRotation, &pAIChannel->mRotationKeys[i].mValue, sizeof(float4_t));*/
			vRotation.x = pAIChannel->mRotationKeys[i].mValue.x;
			vRotation.y = pAIChannel->mRotationKeys[i].mValue.y;
			vRotation.z = pAIChannel->mRotationKeys[i].mValue.z;
			vRotation.w = pAIChannel->mRotationKeys[i].mValue.w;

			KeyFrame.fTrackPosition = pAIChannel->mRotationKeys[i].mTime;
		}
		if (i < pAIChannel->mNumPositionKeys)
		{
			memcpy(&vTranslation, &pAIChannel->mPositionKeys[i].mValue, sizeof(float3_t));
			KeyFrame.fTrackPosition = pAIChannel->mPositionKeys[i].mTime;
		}

		KeyFrame.vScale = vScale;
		KeyFrame.vRotation = vRotation;
		KeyFrame.vTranslation = vTranslation;

		m_KeyFrames.push_back(KeyFrame);
	}

	auto	iter = find_if(Bones.begin(), Bones.end(), [&](shared_ptr<class CBone> pBone)->bool_t {
			++m_iBoneIndex;
			return pBone->Compare_Name(m_szName);
		});

	if (iter == Bones.end())
		return E_FAIL;

	return S_OK;
}

void CChannel::Update_TransformationMatrix(f32_t fCurrentTrackPosition, const vector<shared_ptr<class CBone>>& Bones, uint32_t* pLeftKeyFrameIndex)
{
	if (0.f == fCurrentTrackPosition)
		(*pLeftKeyFrameIndex) = 0;


	KEYFRAME		LastKeyFrame = m_KeyFrames.back();

	vector_t		vScale{}, vRotation{}, vTranslation{};

	if (fCurrentTrackPosition >= LastKeyFrame.fTrackPosition) /* 선형보간이 필요 없는 상태 */
	{
		vScale = XMLoadFloat3(&LastKeyFrame.vScale);
		vRotation = XMLoadFloat4(&LastKeyFrame.vRotation);
		vTranslation = XMVectorSetW(XMLoadFloat3(&LastKeyFrame.vTranslation), 1.f);
	}

	else /* 선형보간이 필요한 상태 */
	{
		while(fCurrentTrackPosition >= m_KeyFrames[(*pLeftKeyFrameIndex) + 1].fTrackPosition)
			++(*pLeftKeyFrameIndex);

		f32_t		fRatio = (fCurrentTrackPosition - m_KeyFrames[(*pLeftKeyFrameIndex)].fTrackPosition) / 
			(m_KeyFrames[(*pLeftKeyFrameIndex) + 1].fTrackPosition - m_KeyFrames[(*pLeftKeyFrameIndex)].fTrackPosition);

		vector_t	vLeftScale = XMLoadFloat3(&m_KeyFrames[(*pLeftKeyFrameIndex)].vScale);
		vector_t	vRightScale = XMLoadFloat3(&m_KeyFrames[(*pLeftKeyFrameIndex) + 1].vScale);
		vector_t	vLeftRotation = XMLoadFloat4(&m_KeyFrames[(*pLeftKeyFrameIndex)].vRotation); 
		vector_t	vRightRotation = XMLoadFloat4(&m_KeyFrames[(*pLeftKeyFrameIndex) + 1].vRotation);
		vector_t	vLeftTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[(*pLeftKeyFrameIndex)].vTranslation), 1.f);
		vector_t	vRightTranslation = XMVectorSetW(XMLoadFloat3(&m_KeyFrames[(*pLeftKeyFrameIndex) + 1].vTranslation), 1.f);

		vScale = XMVectorLerp(vLeftScale, vRightScale, fRatio);
		vRotation = XMQuaternionSlerp(vLeftRotation, vRightRotation, fRatio);
		vTranslation = XMVectorLerp(vLeftTranslation, vRightTranslation, fRatio);
	}

	// matrix_t	BoneTranslationMatrix = XMMatrixScaling() * XMMatrixRotationQuaternion() * XMMatrixTranslation();
	matrix_t	BoneTranslationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vTranslation);

	Bones[m_iBoneIndex]->Update_TransformationMatrix(BoneTranslationMatrix);
}

shared_ptr<CChannel> CChannel::Create(const aiNodeAnim* pAIChannel, const vector<shared_ptr<class CBone>>& Bones)
{
	auto pInstance = shared_ptr<CChannel>(new CChannel());

	if (FAILED(pInstance->Initialize(pAIChannel, Bones)))
	{
		MSG_BOX("Failed to Created : CChannel");
		return nullptr;
	}

	return pInstance;
}
