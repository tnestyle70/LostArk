#include "Channel.h"
#include "BinaryAsset/ModelAssetData.h"
#include "Bone.h"

#include <algorithm>
#include <cmath>

namespace
{
	float3_t SampleVector(const vector<MODEL_VECTOR_KEY_DATA>& keys,
		f32_t time, const float3_t& fallback)
	{
		if (keys.empty())
			return fallback;
		if (time <= keys.front().timeTicks)
			return keys.front().value;
		if (time >= keys.back().timeTicks)
			return keys.back().value;

		auto right = upper_bound(keys.begin(), keys.end(), time,
			[](f32_t value, const MODEL_VECTOR_KEY_DATA& key)
			{ return value < key.timeTicks; });
		const auto left = right - 1;
		const f32_t span = right->timeTicks - left->timeTicks;
		const f32_t ratio = span > 0.f ? (time - left->timeTicks) / span : 0.f;
		float3_t result{};
		XMStoreFloat3(&result, XMVectorLerp(
			XMLoadFloat3(&left->value), XMLoadFloat3(&right->value), ratio));
		return result;
	}

	float4_t SampleQuaternion(const vector<MODEL_QUAT_KEY_DATA>& keys,
		f32_t time)
	{
		if (keys.empty())
			return float4_t(0.f, 0.f, 0.f, 1.f);
		if (time <= keys.front().timeTicks)
			return keys.front().value;
		if (time >= keys.back().timeTicks)
			return keys.back().value;

		auto right = upper_bound(keys.begin(), keys.end(), time,
			[](f32_t value, const MODEL_QUAT_KEY_DATA& key)
			{ return value < key.timeTicks; });
		const auto left = right - 1;
		const f32_t span = right->timeTicks - left->timeTicks;
		const f32_t ratio = span > 0.f ? (time - left->timeTicks) / span : 0.f;
		float4_t result{};
		XMStoreFloat4(&result, XMQuaternionSlerp(
			XMLoadFloat4(&left->value), XMLoadFloat4(&right->value), ratio));
		return result;
	}
}

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

HRESULT CChannel::Initialize(const MODEL_ANIMATION_CHANNEL_DATA& channel,
	const vector<shared_ptr<class CBone>>& Bones)
{
	if (channel.resolvedBoneIndex < 0 ||
		channel.resolvedBoneIndex >= static_cast<int32_t>(Bones.size()))
		return E_FAIL;
	m_iBoneIndex = channel.resolvedBoneIndex;
	if (channel.positionKeys.empty() && channel.rotationKeys.empty() &&
		channel.scaleKeys.empty())
		return E_FAIL;

	// Preserve the three compact WAnimation tracks. Expanding their union into
	// full transform keyframes multiplies memory for long character packages and
	// made the 154-clip DimensionMaster body exhaust memory during level loading.
	m_PositionKeys = channel.positionKeys;
	m_RotationKeys = channel.rotationKeys;
	m_ScaleKeys = channel.scaleKeys;
	m_iNumKeyFrames = static_cast<uint32_t>((max)({
		m_PositionKeys.size(), m_RotationKeys.size(), m_ScaleKeys.size() }));
	m_bUsesSeparateTracks = true;
	return S_OK;
}

void CChannel::Update_TransformationMatrix(f32_t fCurrentTrackPosition, const vector<shared_ptr<class CBone>>& Bones, uint32_t* pLeftKeyFrameIndex)
{
	if (m_bUsesSeparateTracks)
	{
		const float3_t scale = SampleVector(
			m_ScaleKeys, fCurrentTrackPosition,
			float3_t(1.f, 1.f, 1.f));
		const float4_t rotation = SampleQuaternion(
			m_RotationKeys, fCurrentTrackPosition);
		const float3_t translation = SampleVector(
			m_PositionKeys, fCurrentTrackPosition,
			float3_t(0.f, 0.f, 0.f));

		const matrix_t boneTranslationMatrix =
			XMMatrixAffineTransformation(
				XMLoadFloat3(&scale),
				XMVectorZero(),
				XMLoadFloat4(&rotation),
				XMVectorSetW(XMLoadFloat3(&translation), 1.f));
		Bones[m_iBoneIndex]->Update_TransformationMatrix(
			boneTranslationMatrix);
		return;
	}

	if (m_KeyFrames.empty() || nullptr == pLeftKeyFrameIndex)
		return;

	if (0.f == fCurrentTrackPosition)
		(*pLeftKeyFrameIndex) = 0;


	KEYFRAME		LastKeyFrame = m_KeyFrames.back();

	vector_t		vScale{}, vRotation{}, vTranslation{};

	if (1 == m_KeyFrames.size() || fCurrentTrackPosition >= LastKeyFrame.fTrackPosition) /* 선형보간이 필요 없는 상태 */
	{
		vScale = XMLoadFloat3(&LastKeyFrame.vScale);
		vRotation = XMLoadFloat4(&LastKeyFrame.vRotation);
		vTranslation = XMVectorSetW(XMLoadFloat3(&LastKeyFrame.vTranslation), 1.f);
	}

	else /* 선형보간이 필요한 상태 */
	{
		(*pLeftKeyFrameIndex) = (min)(*pLeftKeyFrameIndex,
			static_cast<uint32_t>(m_KeyFrames.size() - 2));
		while ((*pLeftKeyFrameIndex) + 1 < m_KeyFrames.size() - 1 &&
			fCurrentTrackPosition >= m_KeyFrames[(*pLeftKeyFrameIndex) + 1].fTrackPosition)
			++(*pLeftKeyFrameIndex);

		const f32_t fSpan = m_KeyFrames[(*pLeftKeyFrameIndex) + 1].fTrackPosition -
			m_KeyFrames[(*pLeftKeyFrameIndex)].fTrackPosition;
		f32_t		fRatio = fSpan > 0.f ?
			(fCurrentTrackPosition - m_KeyFrames[(*pLeftKeyFrameIndex)].fTrackPosition) / fSpan : 0.f;

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

bool_t CChannel::Sample_TransformationMatrix(
	const f32_t fTrackPosition,
	uint32_t& iOutBoneIndex,
	float4x4_t& OutTransformationMatrix) const
{
	if (!std::isfinite(fTrackPosition) || m_iBoneIndex < 0)
		return false;

	matrix_t Transformation;
	if (m_bUsesSeparateTracks)
	{
		const float3_t Scale = SampleVector(
			m_ScaleKeys, fTrackPosition, float3_t(1.f, 1.f, 1.f));
		const float4_t Rotation = SampleQuaternion(
			m_RotationKeys, fTrackPosition);
		const float3_t Translation = SampleVector(
			m_PositionKeys, fTrackPosition, float3_t(0.f, 0.f, 0.f));
		Transformation = XMMatrixAffineTransformation(
			XMLoadFloat3(&Scale),
			XMVectorZero(),
			XMLoadFloat4(&Rotation),
			XMVectorSetW(XMLoadFloat3(&Translation), 1.f));
	}
	else
	{
		if (m_KeyFrames.empty())
			return false;

		const KEYFRAME& LastKeyFrame = m_KeyFrames.back();
		vector_t Scale;
		vector_t Rotation;
		vector_t Translation;
		if (1u == m_KeyFrames.size() ||
			fTrackPosition >= LastKeyFrame.fTrackPosition)
		{
			Scale = XMLoadFloat3(&LastKeyFrame.vScale);
			Rotation = XMLoadFloat4(&LastKeyFrame.vRotation);
			Translation = XMVectorSetW(
				XMLoadFloat3(&LastKeyFrame.vTranslation), 1.f);
		}
		else
		{
			size_t iLeftKeyFrame = 0u;
			while (iLeftKeyFrame + 1u < m_KeyFrames.size() - 1u &&
				fTrackPosition >=
					m_KeyFrames[iLeftKeyFrame + 1u].fTrackPosition)
			{
				++iLeftKeyFrame;
			}
			const KEYFRAME& Left = m_KeyFrames[iLeftKeyFrame];
			const KEYFRAME& Right = m_KeyFrames[iLeftKeyFrame + 1u];
			const f32_t fSpan =
				Right.fTrackPosition - Left.fTrackPosition;
			const f32_t fRatio = fSpan > 0.f ?
				(fTrackPosition - Left.fTrackPosition) / fSpan : 0.f;
			Scale = XMVectorLerp(
				XMLoadFloat3(&Left.vScale),
				XMLoadFloat3(&Right.vScale), fRatio);
			Rotation = XMQuaternionSlerp(
				XMLoadFloat4(&Left.vRotation),
				XMLoadFloat4(&Right.vRotation), fRatio);
			Translation = XMVectorLerp(
				XMVectorSetW(XMLoadFloat3(&Left.vTranslation), 1.f),
				XMVectorSetW(XMLoadFloat3(&Right.vTranslation), 1.f),
				fRatio);
		}
		Transformation = XMMatrixAffineTransformation(
			Scale, XMVectorZero(), Rotation, Translation);
	}

	float4x4_t Staged{};
	XMStoreFloat4x4(&Staged, Transformation);
	iOutBoneIndex = static_cast<uint32_t>(m_iBoneIndex);
	OutTransformationMatrix = Staged;
	return true;
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

shared_ptr<CChannel> CChannel::Create(const MODEL_ANIMATION_CHANNEL_DATA& channel,
	const vector<shared_ptr<class CBone>>& Bones)
{
	auto pInstance = shared_ptr<CChannel>(new CChannel());
	if (FAILED(pInstance->Initialize(channel, Bones)))
	{
		MSG_BOX("Failed to Created : CChannel");
		return nullptr;
	}
	return pInstance;
}
