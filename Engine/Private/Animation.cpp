#include "Animation.h"
#include "BinaryAsset/ModelAssetData.h"
#include "Channel.h"

CAnimation::CAnimation()
{
}

CAnimation::~CAnimation()
{
}

HRESULT CAnimation::Initialize(const aiAnimation* pAIAnimation, const vector<shared_ptr<class CBone>>& Bones)
{
	strcpy_s(m_szName, pAIAnimation->mName.C_Str());
	m_fDuration = pAIAnimation->mDuration;
	m_fTickPerSecond = pAIAnimation->mTicksPerSecond;

	m_iNumChannels = pAIAnimation->mNumChannels;

	m_iLeftKeyFrameIndices.resize(m_iNumChannels);

	for (uint32_t i = 0; i < m_iNumChannels; i++)
	{
		auto pChannel = CChannel::Create(pAIAnimation->mChannels[i], Bones);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);
	}

	return S_OK;
}

HRESULT CAnimation::Initialize(const MODEL_ANIMATION_DATA& animation,
	const vector<shared_ptr<class CBone>>& Bones)
{
	if (animation.name.empty() || animation.name.size() >= MAX_PATH ||
		animation.durationTicks <= 0.f || animation.ticksPerSecond <= 0.f)
		return E_FAIL;

	strcpy_s(m_szName, animation.name.c_str());
	m_fDuration = animation.durationTicks;
	m_fTickPerSecond = animation.ticksPerSecond;
	m_iNumChannels = static_cast<uint32_t>(animation.channels.size());
	m_iLeftKeyFrameIndices.resize(m_iNumChannels);

	for (const MODEL_ANIMATION_CHANNEL_DATA& channel : animation.channels)
	{
		auto pChannel = CChannel::Create(channel, Bones);
		if (nullptr == pChannel)
			return E_FAIL;
		m_Channels.push_back(pChannel);
	}
	return S_OK;
}

/* 현재 이 애니메이션이 컨트롤해야하는 뼈들의 상태행렬을 갱신해준다. */
bool_t CAnimation::Update_TransformationMatrix(f32_t fTimeDelta, const vector<shared_ptr<class CBone>>& Bones, bool_t isLoop)
{
	/* 현재 재생 위치를 계산해준다. */
	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;

	if (m_fCurrentTrackPosition >= m_fDuration)
	{
		if (false == isLoop)
			return true;

		m_fCurrentTrackPosition = 0.f;		
	}

	/* 현재 재생위치에 맞게 뼈들의 상태행렬을 갱신해준다. */
	for (uint32_t i = 0; i < m_iNumChannels; i++)
	{
		m_Channels[i]->Update_TransformationMatrix(m_fCurrentTrackPosition, Bones, &m_iLeftKeyFrameIndices[i]);
	}

	return false;
}

shared_ptr<CAnimation> CAnimation::Create(const aiAnimation* pAIAnimation, const vector<shared_ptr<class CBone>>& Bones)
{
	auto pInstance = shared_ptr<CAnimation>(new CAnimation());

	if (FAILED(pInstance->Initialize(pAIAnimation, Bones)))
	{
		MSG_BOX("Failed to Created : CAnimation");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CAnimation> CAnimation::Create(const MODEL_ANIMATION_DATA& animation,
	const vector<shared_ptr<class CBone>>& Bones)
{
	auto pInstance = shared_ptr<CAnimation>(new CAnimation());
	if (FAILED(pInstance->Initialize(animation, Bones)))
	{
		MSG_BOX("Failed to Created : CAnimation");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CAnimation> CAnimation::Clone()
{
	return shared_ptr<CAnimation>(new CAnimation(*this));
}

