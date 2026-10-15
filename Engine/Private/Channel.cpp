// 본 하나의 키 보관과 local 행렬 샘플을 구현한다.
#include "Channel.h"
//Assimp 입력 구조를 읽는다.
#include "Assimp/scene.h"
// Assimp 경로의 결합 KEYFRAME을 구성한다.
#include "Engine_AnimationTypes.h"
// WAnimation 분리 키의 시간 tick과 S/R/T 값을 소비한다.
#include "BinaryAsset/ModelAssetData.h"
// 이름으로 대상 본을 찾고 샘플 결과를 local 행렬에 설치한다.
#include "Bone.h"

// 본 이름 조회와 시간 키의 upper_bound 탐색을 사용한다.
#include <algorithm>
// 명시 시점 샘플의 유한성 검사를 사용한다.
#include <cmath>

namespace
{
	// 정렬된 비어 있지 않은 키 배열의 내부 구간에서 오른쪽 키를 찾는다. 양 끝 처리는 호출자가 먼저 수행한다.
	template<typename TKey>
	const TKey* FindRightKey(const vector<TKey>& keys, const f32_t time,
		uint32_t* const leftIndex)
	{
		const TKey* const begin = keys.data();
		const size_t count = keys.size();
		if (leftIndex && count > 1u && static_cast<size_t>(*leftIndex) < count - 1u)
		{
			const TKey* left = begin + *leftIndex;
			if (left->timeTicks <= time)
			{
				// 기존 커서에서 최대 네 구간만 전진한다. 멀리 seek했으면 아래 이진 탐색으로 넘겨
				// 이전 커서부터 모든 키를 순회하지 않게 한다.
				for (size_t step = 0u; step < 4u && left + 1 < begin + count; ++step, ++left)
				{
					if (time < (left + 1)->timeTicks)
					{
						*leftIndex = static_cast<uint32_t>(left - begin);
						return left + 1;
					}
				}
			}
		}
		// 커서는 clone 소유이고 공유 compact track은 수정하지 않는다.
		// 되감기나 먼 seek도 같은 키에서 upper_bound로 오른쪽 구간을 찾는다.
		const TKey* const right = upper_bound(begin, begin + count, time,
			[](f32_t value, const TKey& key) { return value < key.timeTicks; });
		if (leftIndex)
			*leftIndex = static_cast<uint32_t>(right - begin - 1);
		return right;
	}

	// 분리 위치/스케일 키를 선형 보간한다. 빈 track은 fallback, 범위 밖 시간은 끝 키를 사용한다.
	float3_t SampleVector(const vector<MODEL_VECTOR_KEY_DATA>& keys,
		f32_t time, const float3_t& fallback, uint32_t* leftIndex = nullptr)
	{
		if (keys.empty())
			return fallback;
		if (time <= keys.front().timeTicks)
			return keys.front().value;
		if (time >= keys.back().timeTicks)
			return keys.back().value;

		const auto right = FindRightKey(keys, time, leftIndex);
		const auto left = right - 1;
		const f32_t span = right->timeTicks - left->timeTicks;
		const f32_t ratio = span > 0.f ? (time - left->timeTicks) / span : 0.f;
		float3_t result{};
		XMStoreFloat3(&result, XMVectorLerp(
			XMLoadFloat3(&left->value), XMLoadFloat3(&right->value), ratio));
		return result;
	}

	// 분리 회전 키를 slerp로 보간한다. 빈 track은 단위 quaternion이며 별도 정규화 단계는 없다.
	float4_t SampleQuaternion(const vector<MODEL_QUAT_KEY_DATA>& keys,
		f32_t time, uint32_t* leftIndex = nullptr)
	{
		if (keys.empty())
			return float4_t(0.f, 0.f, 0.f, 1.f);
		if (time <= keys.front().timeTicks)
			return keys.front().value;
		if (time >= keys.back().timeTicks)
			return keys.back().value;

		const auto right = FindRightKey(keys, time, leftIndex);
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

// Create가 유효한 Assimp 채널과 본 배열을 넘긴다. 키 복사 뒤 이름을 찾으므로 실패 시 부분 상태의 정리는 factory가 맡는다.
HRESULT CChannel::Initialize(
	const aiNodeAnim* pAIChannel,
	const vector<shared_ptr<class CBone>>& Bones)
{
	strcpy_s(m_szName, pAIChannel->mNodeName.C_Str());

	m_iNumKeyFrames = max(pAIChannel->mNumScalingKeys, pAIChannel->mNumRotationKeys);
	m_iNumKeyFrames = max(m_iNumKeyFrames, pAIChannel->mNumPositionKeys);

	float3_t		vScale{};
	float4_t		vRotation{};
	float3_t		vTranslation{};


	// 세 성분의 같은 index를 묶는 기존 경로다. 먼저 끝난 성분은 직전 값을 유지하고 시간은 scale→rotation→position 순으로 덮어쓴다.
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

	// 이름 탐색 중 m_iBoneIndex를 증가시켜 배열 위치를 기록한다. 새 채널의 -1 초기값을 전제로 한다.
	auto	iter = find_if(Bones.begin(), Bones.end(), [&](shared_ptr<class CBone> pBone)->bool_t {
			++m_iBoneIndex;
			return pBone->Compare_Name(m_szName);
		});

	if (iter == Bones.end())
		return E_FAIL;

	return S_OK;
}

// WModel 로드가 해석한 본 index를 확인한다. index 저장 뒤 빈 track을 거부하므로 재초기화의 원상 복구 계약은 아니다.
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

	// WAnimation의 compact track 세 개를 그대로 보관한다. 시간의 합집합을 전체 S/R/T 키로
	// 확장하면 중복 저장량이 늘어난다. 기존에는 차원술사 몸체의 154개 clip을 이 방식으로
	// 확장하면서 레벨 로딩 중 메모리가 소진된 이력이 있어 분리 저장을 유지한다.
	m_PositionKeys = channel.positionKeys;
	m_RotationKeys = channel.rotationKeys;
	m_ScaleKeys = channel.scaleKeys;
	m_iNumKeyFrames = static_cast<uint32_t>((max)({
		m_PositionKeys.size(), m_RotationKeys.size(), m_ScaleKeys.size() }));
	m_bUsesSeparateTracks = true;
	return S_OK;
}

// CAnimation이 자기 clone의 Bones와 탐색 커서를 전달한다. 채널 데이터는 수정하지 않고 해당 본의 local만 바꾼다.
void CChannel::Update_TransformationMatrix(
	f32_t fCurrentTrackPosition,
	const vector<shared_ptr<class CBone>>& Bones,
	uint32_t* pLeftKeyFrameIndex,
	std::array<uint32_t, 3>* pSeparateTrackIndices)
{
	// 분리 track은 서로 다른 시간 키를 독립적으로 찾는다. 누락 성분은 scale 1, rotation identity, translation 0을 사용한다.
	if (m_bUsesSeparateTracks)
	{
		const float3_t scale = SampleVector(
			m_ScaleKeys, fCurrentTrackPosition,
			float3_t(1.f, 1.f, 1.f), pSeparateTrackIndices ? &(*pSeparateTrackIndices)[0] : nullptr);
		const float4_t rotation = SampleQuaternion(
			m_RotationKeys, fCurrentTrackPosition,
			pSeparateTrackIndices ? &(*pSeparateTrackIndices)[1] : nullptr);
		const float3_t translation = SampleVector(
			m_PositionKeys, fCurrentTrackPosition,
			float3_t(0.f, 0.f, 0.f), pSeparateTrackIndices ? &(*pSeparateTrackIndices)[2] : nullptr);

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

	// 기존 결합 키 경로는 커서가 필요하다. 역재생/되감기의 커서 초기화는 CAnimation이 처리한다.
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

		// tick 차이로 보간 비율을 만든다. span이 양수가 아니면 왼쪽 키를 쓰며 첫 키 이전에는 음수 비율이 될 수 있다.
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

// 명시 시점 샘플은 임시 행렬을 만든 뒤에만 출력 index/행렬을 저장한다. 활성 본이나 재생 커서는 변경하지 않는다.
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

	// 성공한 계산을 출력에 옮긴다. 여기서는 결과 행렬 성분의 유한성을 별도로 검사하지 않는다.
	float4x4_t Staged{};
	XMStoreFloat4x4(&Staged, Transformation);
	iOutBoneIndex = static_cast<uint32_t>(m_iBoneIndex);
	OutTransformationMatrix = Staged;
	return true;
}

// 실패한 임시 채널은 외부에 등록하지 않고 키 배열과 함께 해제한다.
shared_ptr<CChannel> CChannel::Create(
	const aiNodeAnim* pAIChannel,
	const vector<shared_ptr<class CBone>>& Bones)
{
	auto pInstance = shared_ptr<CChannel>(new CChannel());

	if (FAILED(pInstance->Initialize(pAIChannel, Bones)))
	{
		MSG_BOX("Failed to Created : CChannel");
		return nullptr;
	}

	return pInstance;
}

// 분리 키 채널을 구성한 뒤 성공한 소유 참조만 반환한다.
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
