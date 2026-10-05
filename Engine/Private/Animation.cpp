#include "Animation.h"
#pragma push_macro("new")
#undef new
#include "Assimp/scene.h"
#pragma pop_macro("new")
#include "Profiler.h"
#include "GameInstance.h"
#include "Engine_RenderTypes.h"
#include "BinaryAsset/ModelAssetData.h"
#include "Channel.h"
#include "Bone.h"

#include <algorithm>
#include <cmath>
#pragma push_macro("new")
#undef new
#include <bit>
#include <mutex>
#include <unordered_map>
#pragma pop_macro("new")

struct CAnimation::SAMPLE_REUSE final
{
    explicit SAMPLE_REUSE(const vector<shared_ptr<CChannel>>& source)
        : channels(source), localTransforms(source.size()) {}
    const vector<shared_ptr<CChannel>> channels;
    // One bounded sample per live immutable channel set. Holding the lock
    // through installation prevents a second sampling thread replacing it.
    std::mutex mutex;
    vector<float4x4_t> localTransforms;
    f32_t trackPosition = 0.f;
    bool valid = false;
};

void CAnimation::Enable_SampleReuse()
{
    if (m_pSampleReuse || m_Channels.empty() || m_iNumChannels != m_Channels.size()) return;
    for (const auto& channel : m_Channels)
        if (!channel || !channel->m_bUsesSeparateTracks || channel->m_iBoneIndex < 0) return;

    // Hash individual scalar fields, never structure padding. Hash matches
    // only select candidates: every index, key and component is compared below.
    uint64_t hash = 14695981039346656037ull;
    const auto add = [&](uint64_t value) {
        for (unsigned byte = 0; byte < 8; ++byte)
        { hash ^= (value >> (byte * 8)) & 0xffu; hash *= 1099511628211ull; }
    };
    const auto addKeys = [&](const auto& keys) {
        add(keys.size());
        for (const auto& key : keys)
        {
            add(std::bit_cast<uint32_t>(key.timeTicks));
            add(std::bit_cast<uint32_t>(key.value.x));
            add(std::bit_cast<uint32_t>(key.value.y));
            add(std::bit_cast<uint32_t>(key.value.z));
            if constexpr (requires { key.value.w; }) add(std::bit_cast<uint32_t>(key.value.w));
        }
    };
    add(m_Channels.size());
    for (const auto& channel : m_Channels)
    {
        add(static_cast<uint32_t>(channel->m_iBoneIndex));
        addKeys(channel->m_PositionKeys);
        addKeys(channel->m_RotationKeys);
        addKeys(channel->m_ScaleKeys);
    }
    const auto equalFloat = [](const f32_t a, const f32_t b) {
        return std::bit_cast<uint32_t>(a) == std::bit_cast<uint32_t>(b);
    };
    const auto equalKeys = [&](const auto& a, const auto& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
        {
            if (!equalFloat(a[i].timeTicks, b[i].timeTicks) ||
                !equalFloat(a[i].value.x, b[i].value.x) ||
                !equalFloat(a[i].value.y, b[i].value.y) ||
                !equalFloat(a[i].value.z, b[i].value.z)) return false;
            if constexpr (requires { a[i].value.w; })
                if (!equalFloat(a[i].value.w, b[i].value.w)) return false;
        }
        return true;
    };
    static std::mutex registryMutex;
    static std::unordered_map<uint64_t, vector<weak_ptr<SAMPLE_REUSE>>> registry;
    const std::lock_guard registryLock(registryMutex);
    for (auto entry = registry.begin(); entry != registry.end();)
    {
        std::erase_if(entry->second, [](const auto& owner) { return owner.expired(); });
        if (entry->second.empty()) entry = registry.erase(entry);
        else ++entry;
    }
    auto& candidates = registry[hash];
    for (const auto& candidate : candidates)
    {
        const auto shared = candidate.lock();
        if (!shared || shared->channels.size() != m_Channels.size()) continue;
        bool equal = true;
        for (size_t i = 0; i < m_Channels.size() && equal; ++i)
        {
            const auto& a = *m_Channels[i];
            const auto& b = *shared->channels[i];
            equal = a.m_iBoneIndex == b.m_iBoneIndex &&
                equalKeys(a.m_PositionKeys, b.m_PositionKeys) &&
                equalKeys(a.m_RotationKeys, b.m_RotationKeys) &&
                equalKeys(a.m_ScaleKeys, b.m_ScaleKeys);
        }
        if (!equal) continue;
        m_pSampleReuse = shared;
        m_Channels = shared->channels;
        return;
    }
    m_pSampleReuse = std::make_shared<SAMPLE_REUSE>(m_Channels);
    candidates.push_back(m_pSampleReuse);
}

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
	m_fTickPerSecond = COOKED_TICK_RATE;
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
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Channels.Update");
    if (!isfinite(fTimeDelta) || m_fDuration <= 0.f || m_fTickPerSecond <= 0.f)
        return false;
    const bool_t isFinished = Advance_Clock(fTimeDelta, isLoop);

	if (m_SeparateTrackIndices.size() != m_iNumChannels)
		m_SeparateTrackIndices.resize(m_iNumChannels);

    CProfiler* const sampleProfiler = m_pSampleReuse ? CGameInstance::Get().Get_Profiler() : nullptr;
    if (sampleProfiler) sampleProfiler->Add_Counter(EProfilerCounter::AnimationSampleRequests);
    if (m_pSampleReuse && CGameInstance::Get().Get_RenderOptimizationSettings().NpcPoseReuseEnabled)
    {
        const std::lock_guard sampleLock(m_pSampleReuse->mutex);
        if (m_pSampleReuse->valid && m_pSampleReuse->trackPosition == m_fCurrentTrackPosition)
        {
            Engine::CProfilerScope reuseScope(CGameInstance::Get().Get_Profiler(), "Animation.Channels.Reuse");
            if (sampleProfiler) sampleProfiler->Add_Counter(EProfilerCounter::AnimationSampleReuseHits);
            for (uint32_t i = 0; i < m_iNumChannels; ++i)
                Bones[m_Channels[i]->m_iBoneIndex]->Update_TransformationMatrix(
                    XMLoadFloat4x4(&m_pSampleReuse->localTransforms[i]));
            return isFinished;
        }
        Engine::CProfilerScope evaluateScope(CGameInstance::Get().Get_Profiler(), "Animation.Channels.Evaluate" );
        // Only channel-written local transforms are cached. Unkeyed bones,
        // root suppression, blending, pretransform and combined palettes remain
        // model-local and follow the original CModel::Play_Animation path.
        for (uint32_t i = 0; i < m_iNumChannels; ++i)
        {
            m_Channels[i]->Update_TransformationMatrix(m_fCurrentTrackPosition, Bones,
                &m_iLeftKeyFrameIndices[i], &m_SeparateTrackIndices[i]);
            XMStoreFloat4x4(&m_pSampleReuse->localTransforms[i],
                Bones[m_Channels[i]->m_iBoneIndex]->Get_TransformationMatrix());
        }
        m_pSampleReuse->trackPosition = m_fCurrentTrackPosition;
        m_pSampleReuse->valid = true;
        return isFinished;
    }

	/* 현재 재생위치에 맞게 뼈들의 상태행렬을 갱신해준다. */
	for (uint32_t i = 0; i < m_iNumChannels; i++)
	{
		m_Channels[i]->Update_TransformationMatrix(m_fCurrentTrackPosition, Bones, &m_iLeftKeyFrameIndices[i], &m_SeparateTrackIndices[i]);
	}

	return isFinished;
}

bool_t CAnimation::Advance_Clock(f32_t fTimeDelta, bool_t isLoop)
{
	/* 현재 재생 위치를 계산해준다. */
	if (!isfinite(fTimeDelta) || m_fDuration <= 0.f ||
		m_fTickPerSecond <= 0.f)
	{
		return false;
	}

	const f32_t previousPosition = m_fCurrentTrackPosition;
	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;
	bool_t isFinished = false;
	if (isLoop)
	{
		m_fCurrentTrackPosition = fmodf(
			m_fCurrentTrackPosition, m_fDuration);
		if (m_fCurrentTrackPosition < 0.f)
			m_fCurrentTrackPosition += m_fDuration;
	}
	else if (m_fCurrentTrackPosition >= m_fDuration)
	{
		m_fCurrentTrackPosition = m_fDuration;
		isFinished = true;
	}
	else if (m_fCurrentTrackPosition <= 0.f)
	{
		m_fCurrentTrackPosition = 0.f;
		isFinished = fTimeDelta < 0.f;
	}

	if (fTimeDelta < 0.f ||
		m_fCurrentTrackPosition < previousPosition)
	{
		for (uint32_t& leftKeyFrameIndex : m_iLeftKeyFrameIndices)
			leftKeyFrameIndex = 0;
	}

    return isFinished;
}

bool_t CAnimation::Accumulate_TransformEnvelope(
    const std::span<std::array<double, 3>> translations, const std::span<double> scales) const
{
    if (translations.size() != scales.size() || m_iNumChannels != m_Channels.size() ||
        !std::isfinite(m_fDuration) || m_fDuration <= 0.f) return false;
    for (const auto& channel : m_Channels)
    {
        // The legacy merged-key evaluator can extrapolate before its first key.
        // Its envelope is deliberately unavailable instead of assuming interpolation.
        if (!channel || !channel->m_bUsesSeparateTracks || channel->m_iBoneIndex < 0 ||
            size_t(channel->m_iBoneIndex) >= scales.size()) return false;
        const size_t bone = size_t(channel->m_iBoneIndex);
        const auto validTimes = [](const auto& keys) {
            double previous = -1.;
            for (const auto& key : keys)
            {
                if (!std::isfinite(key.timeTicks) || key.timeTicks < 0.f || key.timeTicks < previous) return false;
                previous = key.timeTicks;
            }
            return true;
        };
        if (!validTimes(channel->m_PositionKeys) || !validTimes(channel->m_ScaleKeys) ||
            !validTimes(channel->m_RotationKeys)) return false;
        for (const auto& key : channel->m_PositionKeys)
        {
            const double values[3] = {key.value.x, key.value.y, key.value.z};
            for (size_t axis = 0; axis < 3; ++axis)
            {
                if (!std::isfinite(values[axis])) return false;
                translations[bone][axis] = (std::max)(translations[bone][axis], std::abs(values[axis]));
            }
        }
        double scale = channel->m_ScaleKeys.empty() ? 1. : 0.;
        for (const auto& key : channel->m_ScaleKeys)
            for (const double value : {double(key.value.x), double(key.value.y), double(key.value.z)})
            {
                if (!std::isfinite(value)) return false;
                scale = (std::max)(scale, std::abs(value));
            }
        double quaternionError = 0.;
        for (const auto& key : channel->m_RotationKeys)
        {
            const auto& q = key.value;
            const double lengthSq = double(q.x)*q.x + double(q.y)*q.y + double(q.z)*q.z + double(q.w)*q.w;
            if (!std::isfinite(lengthSq) || std::abs(lengthSq - 1.) > .001) return false;
            quaternionError = (std::max)(quaternionError, std::abs(lengthSq - 1.));
        }
        // For shortest-path slerp, norm error stays within the endpoint bound.
        // R(q)=|q|^2 R(q/|q|)+(1-|q|^2)I; add float interpolation slack.
        scales[bone] = (std::max)(scales[bone], scale * (1. + 2. * quaternionError + .0001));
    }
    return true;
}

void CAnimation::Set_TrackPosition(f32_t fTrackPosition)
{
	m_fCurrentTrackPosition = fTrackPosition < 0.f ? 0.f :
		(fTrackPosition > m_fDuration ? m_fDuration : fTrackPosition);

	/* 채널의 키프레임 커서는 앞으로만 전진하므로, 되감을 때 초기화하지 않으면
	이전 위치보다 앞선 키프레임으로 보간해 포즈가 어긋난다. */
	for (auto& iLeftKeyFrameIndex : m_iLeftKeyFrameIndices)
		iLeftKeyFrameIndex = 0;
}

bool_t CAnimation::Is_BoneTransformConstant(const uint32_t boneIndex) const
{
    const CChannel* found = nullptr;
    for (const auto& channel : m_Channels)
        if (channel && channel->m_iBoneIndex == static_cast<int32_t>(boneIndex))
        {
            if (found) return false;
            found = channel.get();
        }
    if (!found) return true;
    const auto equal3 = [](const float3_t& a, const float3_t& b) {
        return std::isfinite(a.x) && std::isfinite(a.y) && std::isfinite(a.z) &&
            std::abs(a.x - b.x) <= 1e-7f && std::abs(a.y - b.y) <= 1e-7f && std::abs(a.z - b.z) <= 1e-7f; };
    const auto equal4 = [&](const float4_t& a, const float4_t& b) {
        return equal3({a.x,a.y,a.z}, {b.x,b.y,b.z}) && std::isfinite(a.w) && std::abs(a.w - b.w) <= 1e-7f; };
    if (found->m_bUsesSeparateTracks)
    {
        for (const auto& key : found->m_PositionKeys)
            if (!equal3(key.value, found->m_PositionKeys.front().value)) return false;
        for (const auto& key : found->m_ScaleKeys)
            if (!equal3(key.value, found->m_ScaleKeys.front().value)) return false;
        for (const auto& key : found->m_RotationKeys)
            if (!equal4(key.value, found->m_RotationKeys.front().value)) return false;
    }
    else
        for (const auto& key : found->m_KeyFrames)
        {
            const auto& first = found->m_KeyFrames.front();
            if (!equal3(key.vTranslation, first.vTranslation) || !equal3(key.vScale, first.vScale) ||
                !equal4(key.vRotation, first.vRotation)) return false;
        }
    return true;
}

bool_t CAnimation::Sample_LocalBoneTransforms(
	const f32_t fTrackPosition,
	const std::span<float4x4_t> InOutLocalTransforms) const
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Channels.Sample");
	if (!std::isfinite(fTrackPosition) ||
		!std::isfinite(m_fDuration) || fTrackPosition < 0.f ||
		fTrackPosition > m_fDuration || m_fDuration <= 0.f ||
		m_iNumChannels != m_Channels.size())
	{
		return false;
	}

	for (const shared_ptr<CChannel>& Channel : m_Channels)
	{
		if (nullptr == Channel)
			return false;
		uint32_t iBoneIndex = 0u;
		float4x4_t Local{};
		if (!Channel->Sample_TransformationMatrix(
				fTrackPosition, iBoneIndex, Local) ||
			iBoneIndex >= InOutLocalTransforms.size())
		{
			return false;
		}
		InOutLocalTransforms[iBoneIndex] = Local;
	}
	return true;
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

