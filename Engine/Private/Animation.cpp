// 클립 시계, 채널 샘플과 선택적 local 표본 공유를 구현한다.
#include "Animation.h"
//Assimp의 원본 애니메이션과 채널 구조를 읽는다.
#include "Assimp/scene.h"
// 샘플 요청/재사용 counter와 CPU 구간을 기록한다.
#include "Profiler.h"
// 현재 profiler와 모델 포즈 재사용 옵션을 조회한다.
#include "GameInstance.h"
// NpcPoseReuseEnabled가 포함된 렌더 최적화 설정을 읽는다.
#include "Engine_RenderTypes.h"
// 변환된 클립의 시간과 compact channel 데이터를 소비한다.
#include "BinaryAsset/ModelAssetData.h"
// 공유 가능한 키 저장소와 본 하나의 샘플을 사용한다.
#include "Channel.h"
// 채널 local 결과를 모델 clone의 본에 저장하거나 표본으로 복사한다.
#include "Bone.h"

// 만료 registry 항목 제거와 경계 최댓값 계산에 사용한다.
#include <algorithm>
// 유한성·절댓값·반복 시간 나머지를 계산한다.
#include <cmath>
// 키 실수값을 표현 비트 그대로 비교하고 hash에 넣는다.
#include <bit>
// 공유 registry와 표본 교체/설치의 상호 배제를 구분한다.
#include <mutex>
// 내용 hash별로 살아 있는 표본 소유자의 약한 참조를 묶는다.
#include <unordered_map>

// 불변 채널 묶음당 최근 local 표본 한 개다. channels가 키 수명을, localTransforms가 채널 순서의 행렬 값을 소유한다.
struct CAnimation::SAMPLE_REUSE final
{
    explicit SAMPLE_REUSE(const vector<shared_ptr<CChannel>>& source)
        : channels(source), localTransforms(source.size())
    {
    }
    const vector<shared_ptr<CChannel>> channels;
    // 살아 있는 불변 채널 묶음마다 표본 한 개만 보관한다. 본에 설치할 때까지
    // 잠금을 유지해 다른 샘플 스레드가 도중에 표본을 교체하지 못하게 한다.
    std::mutex mutex;
    vector<float4x4_t> localTransforms;
    f32_t trackPosition = 0.f;
    bool valid = false;
};

// 외부 시점 샘플을 쓰는 모델이 명시적으로 공유 저장소를 준비한다. 실제 사용은 Update의 NpcPoseReuseEnabled 조건도 통과해야 한다.
void CAnimation::Enable_SampleReuse()
{
    if (m_pSampleReuse || m_Channels.empty() || m_iNumChannels != m_Channels.size())
        return;
    for (const auto& channel : m_Channels)
        if (!channel || !channel->m_bUsesSeparateTracks || channel->m_iBoneIndex < 0)
            return;

    // 구조체 padding을 제외한 scalar 필드별 hash를 계산한다. hash는 후보만
    // 고르며 아래에서 본 index와 모든 키 성분을 다시 비교한다.
    uint64_t hash = 14695981039346656037ull;
    const auto add = [&](uint64_t value)
    {
        for (unsigned byte = 0; byte < 8; ++byte)
        {
            hash ^= (value >> (byte * 8)) & 0xffu;
            hash *= 1099511628211ull;
        }
    };
    const auto addKeys = [&](const auto& keys)
    {
        add(keys.size());
        for (const auto& key : keys)
        {
            add(std::bit_cast<uint32_t>(key.timeTicks));
            add(std::bit_cast<uint32_t>(key.value.x));
            add(std::bit_cast<uint32_t>(key.value.y));
            add(std::bit_cast<uint32_t>(key.value.z));
            if constexpr (requires { key.value.w; })
                add(std::bit_cast<uint32_t>(key.value.w));
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
    // hash 일치만으로 데이터를 공유하지 않는다. 모든 키 성분과 본 index를 다시 비교해 충돌을 구분한다.
    const auto equalFloat = [](const f32_t a, const f32_t b)
    {
        return std::bit_cast<uint32_t>(a) == std::bit_cast<uint32_t>(b);
    };
    const auto equalKeys = [&](const auto& a, const auto& b)
    {
        if (a.size() != b.size())
            return false;
        for (size_t i = 0; i < a.size(); ++i)
        {
            if (!equalFloat(a[i].timeTicks, b[i].timeTicks) ||
                !equalFloat(a[i].value.x, b[i].value.x) ||
                !equalFloat(a[i].value.y, b[i].value.y) ||
                !equalFloat(a[i].value.z, b[i].value.z))
                return false;
            if constexpr (requires { a[i].value.w; })
                if (!equalFloat(a[i].value.w, b[i].value.w))
                    return false;
        }
        return true;
    };
    // registry는 강한 참조를 보유하지 않아 마지막 클립이 놓은 표본을 살려 두지 않는다. 조회/정리/등록을 하나의 잠금으로 보호한다.
    static std::mutex registryMutex;
    static std::unordered_map<uint64_t, vector<weak_ptr<SAMPLE_REUSE>>> registry;
    const std::lock_guard registryLock(registryMutex);
    for (auto entry = registry.begin(); entry != registry.end();)
    {
        std::erase_if(entry->second, [](const auto& owner) { return owner.expired(); });
        if (entry->second.empty())
            entry = registry.erase(entry);
        else
            ++entry;
    }
    // 내용이 완전히 같은 살아 있는 채널 묶음을 찾으면 소유 참조를 공유한다. 시계와 커서 vector는 교체하지 않는다.
    auto& candidates = registry[hash];
    for (const auto& candidate : candidates)
    {
        const auto shared = candidate.lock();
        if (!shared || shared->channels.size() != m_Channels.size())
            continue;
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
        if (!equal)
            continue;
        m_pSampleReuse = shared;
        m_Channels = shared->channels;
        return;
    }
    // 일치 항목이 없을 때만 새 표본 저장소를 만들고 registry에는 weak_ptr로 등록한다.
    m_pSampleReuse = std::make_shared<SAMPLE_REUSE>(m_Channels);
    candidates.push_back(m_pSampleReuse);
}

CAnimation::CAnimation()
{
}

CAnimation::~CAnimation()
{
}

// Assimp의 duration/ticksPerSecond를 그대로 복사한다. 채널 실패 이전에 구성한 상태는 factory에서 임시 클립을 폐기하며 정리한다.
HRESULT CAnimation::Initialize(
	const aiAnimation* pAIAnimation,
	const vector<shared_ptr<class CBone>>& Bones)
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

// 변환 모델의 duration tick은 유지하고 실제 시계는 30 tick/초로 맞춘다. 이 초기화 자체는 기존 상태의 rollback을 제공하지 않는다.
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
// CModel::Play_Animation이 시계 진행과 local 포즈 갱신을 요청한다. combined 계층·블렌드·루트 처리는 모델에 남긴다.
bool_t CAnimation::Update_TransformationMatrix(
	f32_t fTimeDelta,
	const vector<shared_ptr<class CBone>>& Bones, bool_t isLoop)
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Channels.Update");
    if (!isfinite(fTimeDelta) || m_fDuration <= 0.f || m_fTickPerSecond <= 0.f)
        return false;
    // 초 단위 delta를 tick 시계에 적용한다. 반환값은 처리 성공 여부가 아니라 비반복 재생의 끝 도달 표시다.
    const bool_t isFinished = Advance_Clock(fTimeDelta, isLoop);

	// 실제로 재생되는 클립만 clone별 탐색 커서를 확보한다. 공유 track 데이터는 수정하지 않는다.
	if (m_SeparateTrackIndices.size() != m_iNumChannels)
		m_SeparateTrackIndices.resize(m_iNumChannels);

    CProfiler* const sampleProfiler = m_pSampleReuse ? CGameInstance::Get().Get_Profiler() : nullptr;
    if (sampleProfiler)
        sampleProfiler->Add_Counter(EProfilerCounter::AnimationSampleRequests);

    // 표본 잠금은 조회부터 본에 설치하는 동안 유지한다. 동일 float tick의 표본만 재사용해 다른 스레드의 교체와 섞이지 않게 한다.
    if (m_pSampleReuse && CGameInstance::Get().Get_RenderOptimizationSettings().NpcPoseReuseEnabled)
    {
        const std::lock_guard sampleLock(m_pSampleReuse->mutex);
        if (m_pSampleReuse->valid && m_pSampleReuse->trackPosition == m_fCurrentTrackPosition)
        {
            Engine::CProfilerScope reuseScope(CGameInstance::Get().Get_Profiler(), "Animation.Channels.Reuse");
            if (sampleProfiler)
                sampleProfiler->Add_Counter(EProfilerCounter::AnimationSampleReuseHits);
            for (uint32_t i = 0; i < m_iNumChannels; ++i)
                Bones[m_Channels[i]->m_iBoneIndex]->Update_TransformationMatrix(
                    XMLoadFloat4x4(&m_pSampleReuse->localTransforms[i]));
            return isFinished;
        }
        Engine::CProfilerScope evaluateScope(CGameInstance::Get().Get_Profiler(), "Animation.Channels.Evaluate" );
        // 채널이 쓴 local 행렬만 보관한다. 키 없는 본, 루트 이동 억제, 블렌드,
        // pretransform과 combined palette는 각 모델이 소유하며
        // 기존 CModel::Play_Animation 경로에서 처리한다.
        for (uint32_t i = 0; i < m_iNumChannels; ++i)
        {
            m_Channels[i]->Update_TransformationMatrix(m_fCurrentTrackPosition, Bones,
                &m_iLeftKeyFrameIndices[i], &m_SeparateTrackIndices[i]);
            XMStoreFloat4x4(&m_pSampleReuse->localTransforms[i],
                Bones[m_Channels[i]->m_iBoneIndex]->Get_TransformationMatrix());
        }
        // 모든 채널의 local 표본을 채운 뒤 해당 tick과 유효 상태를 함께 기록한다.
        m_pSampleReuse->trackPosition = m_fCurrentTrackPosition;
        m_pSampleReuse->valid = true;
        return isFinished;
    }

	/* 현재 재생위치에 맞게 뼈들의 상태행렬을 갱신해준다. */
	for (uint32_t i = 0; i < m_iNumChannels; i++)
	{
		m_Channels[i]->Update_TransformationMatrix(
			m_fCurrentTrackPosition, Bones,
			&m_iLeftKeyFrameIndices[i], &m_SeparateTrackIndices[i]);
	}

	return isFinished;
}

// 시계를 진행하고 끝/반복 경계를 처리한다. 입력 거부의 false와 정상 재생 중의 false를 구분하는 별도 상태값은 없다.
bool_t CAnimation::Advance_Clock(f32_t fTimeDelta, bool_t isLoop)
{
	/* 현재 재생 위치를 계산해준다. */
	if (!isfinite(fTimeDelta) || m_fDuration <= 0.f ||
		m_fTickPerSecond <= 0.f)
	{
		return false;
	}

	// delta는 초, 속도는 tick/초이므로 누적 위치는 tick이다. 반복 재생은 음수 시간도 양의 주기 안으로 되돌린다.
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

// CModel의 애니메이션 전체 경계 계산이 호출한다. 본별 translation 절댓값과 scale 상한을 누적하며 활성 포즈/시계는 바꾸지 않는다.
// 출력 span은 호출자가 소유한다. 뒤 채널의 검증 실패 전에 쓴 값은 남으므로 임시 계산 배열로 사용해야 한다.
bool_t CAnimation::Accumulate_TransformEnvelope(
    const std::span<std::array<double, 3>> translations, const std::span<double> scales) const
{
    if (translations.size() != scales.size() || m_iNumChannels != m_Channels.size() ||
        !std::isfinite(m_fDuration) || m_fDuration <= 0.f)
        return false;
    for (const auto& channel : m_Channels)
    {
        // 기존 결합 키는 첫 키보다 이른 시점에 외삽할 수 있다.
        // 단순 보간의 최댓값을 경계로 가정할 수 없어 이 경계 계산에서 거부한다.
        if (!channel || !channel->m_bUsesSeparateTracks || channel->m_iBoneIndex < 0 ||
            size_t(channel->m_iBoneIndex) >= scales.size())
            return false;
        const size_t bone = size_t(channel->m_iBoneIndex);
        const auto validTimes = [](const auto& keys)
        {
            double previous = -1.;
            for (const auto& key : keys)
            {
                if (!std::isfinite(key.timeTicks) || key.timeTicks < 0.f || key.timeTicks < previous)
                    return false;
                previous = key.timeTicks;
            }
            return true;
        };
        if (!validTimes(channel->m_PositionKeys) || !validTimes(channel->m_ScaleKeys) ||
            !validTimes(channel->m_RotationKeys))
            return false;
        // 위치 키의 축별 최대 절댓값을 기존 경계에 누적한다. 단위는 모델 local이며 최종 모델 pretransform은 소비자가 적용한다.
        for (const auto& key : channel->m_PositionKeys)
        {
            const double values[3] = {key.value.x, key.value.y, key.value.z};
            for (size_t axis = 0; axis < 3; ++axis)
            {
                if (!std::isfinite(values[axis]))
                    return false;
                translations[bone][axis] = (std::max)(translations[bone][axis], std::abs(values[axis]));
            }
        }
        // 스케일은 무차원 축 배율의 최대 절댓값이다. 키가 없으면 identity 배율 1을 경계로 사용한다.
        double scale = channel->m_ScaleKeys.empty() ? 1. : 0.;
        for (const auto& key : channel->m_ScaleKeys)
            for (const double value : {double(key.value.x), double(key.value.y), double(key.value.z)})
            {
                if (!std::isfinite(value))
                    return false;
                scale = (std::max)(scale, std::abs(value));
            }
        // quaternion 길이 제곱의 1과의 차이를 제한하고 회전 행렬의 늘어남에 대한 여유를 구한다.
        double quaternionError = 0.;
        for (const auto& key : channel->m_RotationKeys)
        {
            const auto& q = key.value;
            const double lengthSq = double(q.x)*q.x + double(q.y)*q.y + double(q.z)*q.z + double(q.w)*q.w;
            if (!std::isfinite(lengthSq) || std::abs(lengthSq - 1.) > .001)
                return false;
            quaternionError = (std::max)(quaternionError, std::abs(lengthSq - 1.));
        }
        // 최단 경로 slerp의 길이 오차는 끝점 경계로 평가한다.
        // R(q)=|q|^2 R(q/|q|)+(1-|q|^2)I에 따라 길이 제곱 오차의 두 배를 더하고,
        // float 보간 여유 0.0001을 더해 소비자가 쓸 scale 상한으로 누적한다.
        scales[bone] = (std::max)(scales[bone], scale * (1. + 2. * quaternionError + .0001));
    }
    return true;
}

// 외부 seek는 시계만 교체한다. 실제 본 포즈는 다음 샘플/재생에서 갱신한다.
void CAnimation::Set_TrackPosition(f32_t fTrackPosition)
{
	m_fCurrentTrackPosition = fTrackPosition < 0.f ? 0.f :
		(fTrackPosition > m_fDuration ? m_fDuration : fTrackPosition);

	/* 채널의 키프레임 커서는 앞으로만 전진하므로, 되감을 때 초기화하지 않으면
	이전 위치보다 앞선 키프레임으로 보간해 포즈가 어긋난다. */
	for (auto& iLeftKeyFrameIndex : m_iLeftKeyFrameIndices)
		iLeftKeyFrameIndex = 0;
}

// 모델이 본별 상수 포즈 여부를 조회한다. quaternion도 성분을 비교하므로 q와 -q의 회전 동등성을 따로 판정하지 않는다.
bool_t CAnimation::Is_BoneTransformConstant(const uint32_t boneIndex) const
{
    const CChannel* found = nullptr;
    for (const auto& channel : m_Channels)
        if (channel && channel->m_iBoneIndex == static_cast<int32_t>(boneIndex))
        {
            if (found)
                return false;
            found = channel.get();
        }
    if (!found)
        return true;
    const auto equal3 = [](const float3_t& a, const float3_t& b)
    {
        return std::isfinite(a.x) && std::isfinite(a.y) && std::isfinite(a.z) &&
            std::abs(a.x - b.x) <= 1e-7f && std::abs(a.y - b.y) <= 1e-7f && std::abs(a.z - b.z) <= 1e-7f;
    };
    const auto equal4 = [&](const float4_t& a, const float4_t& b)
    {
        return equal3({a.x,a.y,a.z}, {b.x,b.y,b.z}) && std::isfinite(a.w) && std::abs(a.w - b.w) <= 1e-7f;
    };
    if (found->m_bUsesSeparateTracks)
    {
        for (const auto& key : found->m_PositionKeys)
            if (!equal3(key.value, found->m_PositionKeys.front().value))
                return false;
        for (const auto& key : found->m_ScaleKeys)
            if (!equal3(key.value, found->m_ScaleKeys.front().value))
                return false;
        for (const auto& key : found->m_RotationKeys)
            if (!equal4(key.value, found->m_RotationKeys.front().value))
                return false;
    }
    else
        for (const auto& key : found->m_KeyFrames)
        {
            const auto& first = found->m_KeyFrames.front();
            if (!equal3(key.vTranslation, first.vTranslation) || !equal3(key.vScale, first.vScale) ||
                !equal4(key.vRotation, first.vRotation))
                return false;
        }
    return true;
}

// CModel의 임시 배열을 본 index로 갱신한다. 전체 clip 시간과 채널 수를 먼저 검사하지만 후속 채널 실패에 앞선 쓰기를 되돌리지는 않는다.
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

// 유효한 클립만 모델에 전달한다. 실패한 임시 객체와 이미 준비한 채널 참조는 함께 정리한다.
shared_ptr<CAnimation> CAnimation::Create(
	const aiAnimation* pAIAnimation,
	const vector<shared_ptr<class CBone>>& Bones)
{
	auto pInstance = shared_ptr<CAnimation>(new CAnimation());

	if (FAILED(pInstance->Initialize(pAIAnimation, Bones)))
	{
		MSG_BOX("Failed to Created : CAnimation");
		return nullptr;
	}

	return pInstance;
}

// 변환 모델 clip의 채널 구성이 끝나야 호출자가 소유 참조를 받는다.
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

// 기본 복사로 시계·커서 vector는 독립되고 채널 및 선택적 표본 소유 참조는 공유된다.
shared_ptr<CAnimation> CAnimation::Clone()
{
	return shared_ptr<CAnimation>(new CAnimation(*this));
}

