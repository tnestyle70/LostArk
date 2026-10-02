# World Sequence 공통 애니메이션 샘플 재사용 구현 계획

## G01. 적용 범위와 현재 상태

Debug Bone 최적화는 컴파일 설정 보완이고 Release 구조 병목을 해결하지 않는다.
이번 구현은 Debug/Release 공통 CAnimation에서 동일 cooked channel 입력과 시각의 local matrix를
한 번 보간한 뒤 재사용한다. WorldSequenceObject 준비에서만 활성화하며 일반 Character 재생은 그대로다.
준비 단계는 hash 후보 검색 뒤 모든 channel index·key scalar bit를 정확 비교한다. struct padding,
파일 이름, 64-bit skeleton hash만으로 포즈 동일성을 인정하지 않는다.

## G02. 파일·선언·실제 흐름

- Animation.h/Animation.cpp: Enable_SampleReuse, SAMPLE_REUSE는 immutable channel 집합과
  mutex로 보호한 마지막 한 시각의 local matrices를 소유한다. registry는 weak pointer만 보관한다.
- Model.h/Model.cpp: Enable_AnimationSampleReuse가 이미 소유한 animations에 opt-in을 전달한다.
- WorldSequenceObject.cpp: Clone 준비 직후 opt-in한다. Sample과 Play_Animation 호출 순서는 보존한다.

각 clone의 Advance_Clock은 hit에도 실행한다. cache hit은 interpolation만 생략하고 자기 bone local을
설치한다. unkeyed bone, blend, root suppression, preScale, combined matrix, palette revision,
attachment와 part Update는 기존 CModel·WorldSequenceObject가 계속 처리한다. 같은 channel index와
local key 입력이면 skeleton rest·preScale이 달라도 보간 결과 자체는 같고 이후 결합은 각 모델이 수행한다.
cache는 animation clone/pool 수명과 연결되고 주소 재사용으로 이전 cache를 찾지 않는다.
같은 시각의 inverse seek·loop·pool reset 뒤에도 overwrite되는 keyed bone만 복구하며 rest를 공유하지 않는다.

## G03. 전체 반영 코드

아래 기존 H/CPP 전문은 이 G의 반영 후보이며 별도 runtime·새 C++ 파일을 만들지 않는다.
기존 project/filter 등록이 그대로 유효하다. rendering option과 Data/Resources는 변경하지 않는다.

### Engine/Public/Animation.h

변경 종류: 기존 파일 함수·선언 추가.

```cpp
#pragma once

#include "Engine_Defines.h"

#include <span>
#include <array>

struct aiAnimation;

NS_BEGIN(Engine)

struct MODEL_ANIMATION_DATA;

class CAnimation final
{
private:
	CAnimation();
public:
	~CAnimation();

public:
	// Cooked playback uses this runtime clock. Metadata consumers must
	// use the same rate even when a package retains a different import rate.
	static constexpr f32_t COOKED_TICK_RATE = 30.f;
	HRESULT Initialize(const aiAnimation* pAIAnimation, const vector<shared_ptr<class CBone>>& Bones);
	HRESULT Initialize(const MODEL_ANIMATION_DATA& animation,
		const vector<shared_ptr<class CBone>>& Bones);
	bool_t	Update_TransformationMatrix(f32_t fTimeDelta, const vector<shared_ptr<class CBone>>& Bones, bool_t isLoop);
	bool_t Compare_Name(const char_t* pName) const { return !strcmp(pName, m_szName); }
	const char_t* Get_Name() const { return m_szName; }
	f32_t Get_Duration() const { return m_fDuration; }
	f32_t Get_TickPerSecond() const { return m_fTickPerSecond; }
	f32_t Get_CurrentTrackPosition() const { return m_fCurrentTrackPosition; }
	void Set_TrackPosition(f32_t fTrackPosition);
    // Opt-in for externally sampled World Sequence clones. Only immutable
    // cooked channels share local samples; each clone retains its own clock.
    void Enable_SampleReuse();

private:
	friend class CModel;
    bool_t Advance_Clock(f32_t fTimeDelta, bool_t isLoop);
    // WModel compact tracks only; append extrema without sampling any live pose.
    bool_t Accumulate_TransformEnvelope(std::span<std::array<double, 3>> translations,
        std::span<double> scales) const;
	bool_t Is_BoneTransformConstant(uint32_t iBoneIndex) const;
	bool_t Sample_LocalBoneTransforms(
		f32_t fTrackPosition,
		std::span<float4x4_t> InOutLocalTransforms) const;

private:
	char_t				m_szName[MAX_PATH] = {};
	f32_t				m_fDuration = {}; // 현재 애니메이션의 전체 길이.
	f32_t				m_fTickPerSecond = {}; // 초당 애니메이션의 재생 속도.
	f32_t				m_fCurrentTrackPosition = {}; // 현재 재생 위치.

private:
	uint32_t							m_iNumChannels = {};
	vector<shared_ptr<class CChannel>>	m_Channels;
	vector<uint32_t>					m_iLeftKeyFrameIndices;
	// Allocate only for clips that are sampled; clone-local cursors never mutate shared tracks.
	vector<std::array<uint32_t, 3>> m_SeparateTrackIndices;
    struct SAMPLE_REUSE;
    shared_ptr<SAMPLE_REUSE> m_pSampleReuse;


public:
	static shared_ptr<CAnimation> Create(const aiAnimation* pAIAnimation, const vector<shared_ptr<class CBone>>& Bones);
	static shared_ptr<CAnimation> Create(const MODEL_ANIMATION_DATA& animation,
		const vector<shared_ptr<class CBone>>& Bones);
	shared_ptr<CAnimation> Clone();
};

NS_END
```

### Engine/Private/Animation.cpp

변경 종류: 기존 파일 함수·선언 추가.

```cpp
#include "Animation.h"
#pragma push_macro("new")
#undef new
#include "Assimp/scene.h"
#pragma pop_macro("new")
#include "Profiler.h"
#include "GameInstance.h"
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

    if (m_pSampleReuse)
    {
        const std::lock_guard sampleLock(m_pSampleReuse->mutex);
        if (m_pSampleReuse->valid && m_pSampleReuse->trackPosition == m_fCurrentTrackPosition)
        {
            Engine::CProfilerScope reuseScope(CGameInstance::Get().Get_Profiler(), "Animation.Channels.Reuse");
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

```

### Engine/Public/Model.h

변경 종류: 기존 파일 함수·선언 추가.

```cpp
#pragma once

#include "Component.h"
#pragma push_macro("new")
#undef new
#include "Assimp/material.h"
#pragma pop_macro("new")

#include <array>
#include <span>
#include <set>

struct aiScene;
struct aiNode;
namespace Assimp { class Importer; }

NS_BEGIN(Engine)

struct MODEL_ASSET_DATA;
struct MODEL_ANIMATION_DATA;
struct MODEL_MATERIAL_SOURCE;
struct MODEL_MESH_DATA;
struct MODEL_ASSET_LOAD_DESC;
struct MODEL_COLOR_TINT;
struct MODEL_SURFACE_PARAMETERS;
struct MESH_SCREEN_LOD_DESC;
struct MODEL_SOURCE_CHARACTER_PARAMETERS;

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
	uint32_t Get_NumAnimations() const {
		return m_iNumAnimations;
	}
	uint32_t Get_CurrentAnimIndex() const {
		return m_iCurrentAnimIndex;
	}
	bool_t Is_AnimLoop() const {
		return m_isAnimLoop;
	}
	bool_t Is_Skinned() const {
		return MODEL::ANIM == m_eType;
	}
	bool_t Has_Animations() const {
		return !m_Animations.empty();
	}
	const char_t* Get_AnimationName(uint32_t iAnimIndex) const;
	bool_t Get_AnimationProgress(uint32_t iAnimIndex, f32_t& fOutPosition, f32_t& fOutDuration) const;
	f32_t Get_AnimationTickPerSecond(uint32_t iAnimIndex) const;

	void Set_AnimPaused(bool_t isPaused) {
		m_isAnimPaused = isPaused;
	}
	bool_t Is_AnimPaused() const {
		return m_isAnimPaused;
	}
	bool_t Set_AnimTrackPosition(uint32_t iAnimIndex, f32_t fTrackPosition);

	bool_t Has_LocalBounds() const { return m_bHasLocalBounds; }
	const float3_t& Get_LocalBoundsMin() const { return m_vLocalBoundsMin; }
	const float3_t& Get_LocalBoundsMax() const { return m_vLocalBoundsMax; }
    // Reference vertex bounds after asset pretransform; independent of animated culling.
    bool_t Try_GetBindGeometryBounds(float3_t& minimum, float3_t& maximum) const;
    // Conservative current WModel skin bounds in model-root space. The bone
    // palette already includes pretransform; callers apply only the actor root.
    // Unsupported/morphed geometry leaves both outputs unchanged.
    bool_t Try_GetCurrentPoseBounds(float3_t& minimum, float3_t& maximum) const;
    // Conservative root-origin sphere covering rest and every admitted WModel clip.
    // Includes the asset pretransform once; unsupported/morphed/external poses fail open.
    bool_t Try_GetAnimationEnvelopeRadius(f32_t& radius) const;
    // WModel triangle query at the current rendered pose. Bounds are broad phase
    // only; distance is in world units. Unsupported/morphed geometry is not picked.
    bool_t Try_PickCurrentPose(const float4x4_t& world, const float3_t& rayOrigin,
        const float3_t& rayDirection, f32_t& distance) const;
    // Also identify the nearest rendered submesh; both outputs are unchanged on failure.
    bool_t Try_PickCurrentPose(const float4x4_t& world, const float3_t& rayOrigin,
        const float3_t& rayDirection, f32_t& distance, uint32_t& meshIndex) const;
    enum class PICK_CULL_MODE { NONE, BACK, FRONT };
    // Movement surface query over immutable static WModel LOD0 triangles. The
    // caller selects eligible materials and the final rasterizer's CW-front cull
    // mode; world reflection is included when testing triangle winding. No GPU
    // readback, query allocation, alpha test, shader displacement or animation.
    // Direction is normalized internally; distance/maxDistance are world units.
    // Unsupported geometry and misses leave distance unchanged.
    bool_t Try_PickStaticSurface(uint32_t meshIndex, const float4x4_t& world,
        const float3_t& rayOrigin, const float3_t& rayDirection, f32_t maxDistance,
        PICK_CULL_MODE cullMode, f32_t& distance) const;
	bool_t Has_SelfConsistentUnauthenticatedGeometryMetadata() const {
		return m_bHasSelfConsistentUnauthenticatedGeometryMetadata;
	}
	uint16_t Get_GeometryFormatVersionMajor() const {
		return m_iGeometryFormatVersionMajor;
	}
	uint16_t Get_GeometryFormatVersionMinor() const {
		return m_iGeometryFormatVersionMinor;
	}
	uint32_t Get_GeometryChannelMask() const {
		return m_iGeometryChannelMask;
	}
	uint32_t Get_GeometryEvidenceFlags() const {
		return m_iGeometryEvidenceFlags;
	}
	f32_t Get_GeometryPreScale() const {
		return m_fGeometryPreScale;
	}
	const array<uint8_t, 32>& Get_GeometryPayloadSha256() const {
		return m_GeometryPayloadSha256;
	}
	const array<uint8_t, 32>& Get_GeometryMetadataIdentitySha256() const {
		return m_GeometryMetadataIdentitySha256;
	}

	matrix_t Get_BoneMatrix(const char_t* pBoneName);
	bool_t Has_Bone(const char_t* pBoneName);
	vector<string> Get_BoneNames() const;

	/* Secondary-motion seam. A caller that drives bones itself resolves indices
	once, reads what the animation produced this frame, writes its own local
	matrices back, and refreshes the combined matrices before skinning reads
	them. Indices are stable for the model's lifetime and every bone's parent
	comes before it, so one forward pass rebuilds the whole hierarchy. */
	int32_t Find_BoneIndex(const char_t* pBoneName) const;
	int32_t Get_BoneParentIndex(uint32_t iBoneIndex) const;
	bool_t Get_BoneLocalMatrix(uint32_t iBoneIndex, matrix_t& outMatrix) const;
	bool_t Get_BoneRestLocalMatrix(uint32_t iBoneIndex, matrix_t& outMatrix) const;
	bool_t Get_BoneCombinedMatrix(uint32_t iBoneIndex, matrix_t& outMatrix) const;
	/* Samples the currently bound animation without moving its cursor or the
	   live bone palette.  expectedAnimationIndex closes the race where a tool
	   prepared one clip and another owner changed it before the sample. */
	bool_t Sample_CurrentAnimationBoneCombinedMatrices(
		uint32_t iExpectedAnimationIndex,
		f32_t fTrackPositionTicks,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	/* Historical action playback can prepare future clip poses while the live
	   model is still at the start of its transition.  This variant evaluates the
	   saved blend-from pose at an explicit elapsed time without advancing either
	   the animation cursor, blend clock, or live bone palette. */
	bool_t Sample_CurrentAnimationBoneCombinedMatricesAtBlendElapsed(
		uint32_t iExpectedAnimationIndex,
		f32_t fTrackPositionTicks,
		f32_t fBlendElapsedSeconds,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	/* Samples one explicitly named clip without binding it to the live model.
	   Unkeyed bones use the immutable skeleton rest pose and the unrelated live
	   transition is not applied. Missing/ambiguous names leave output unchanged. */
	bool_t Sample_AnimationBoneCombinedMatrices(
		const char_t* pAnimationName,
		f32_t fTrackPositionTicks,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	// Raw root translation relative to immutable rest, before suppression. The
	// returned model-space vector already includes ancestor basis and model pre-transform.
	bool_t Sample_AnimationRootTranslation(const char_t* pAnimationName,
		f32_t fTrackPositionTicks, uint32_t iRootBoneIndex, int32_t iVerticalAxis,
		f32_t fVerticalScale, float3_t& OutModelTranslation) const;
	struct ROOT_MOTION_SUPPRESSION_STATE final
	{
		const CModel* owner = nullptr;
		int32_t boneIndex = -1, verticalAxis = -1;
		float3_t restTranslation{}, unscaledTranslation{};
		f32_t verticalScale = 1.f;
	};
	ROOT_MOTION_SUPPRESSION_STATE Capture_RootMotionSuppression() const;
	bool_t Configure_RootMotionSuppressionFromRest(uint32_t iRootBoneIndex,
		int32_t iVerticalAxis, f32_t fVerticalScale);
	bool_t Restore_RootMotionSuppression(const ROOT_MOTION_SUPPRESSION_STATE& state);
	// Explicit clip samples define a transition independently of render history.
	// UINT32_MAX selects the immutable rest pose (for an unmapped weapon clip).
	struct ANIMATION_TRANSITION_POSE final
	{
		uint32_t sourceIndex = UINT32_MAX, targetIndex = UINT32_MAX;
		f32_t sourceTicks = 0.f, targetTicks = 0.f;
		f32_t durationSeconds = 0.f, elapsedSeconds = 0.f, playRate = 1.f;
	};
	bool_t Set_AnimationTransitionPose(const ANIMATION_TRANSITION_POSE& pose);
	// The same explicit transition, sampled without changing the live actor pose.
	bool_t Sample_AnimationTransitionBoneCombinedMatrices(
		const ANIMATION_TRANSITION_POSE& pose,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	const ANIMATION_TRANSITION_POSE* Get_AnimationTransitionPose() const
	{ return m_bExplicitAnimationPose ? &m_ExplicitAnimationPose : nullptr; }
	void Clear_AnimationTransitionPose() { m_bExplicitAnimationPose = false; }
	bool_t Set_BoneLocalMatrix(uint32_t iBoneIndex, fmatrix_t Matrix);
	void Refresh_BoneCombinedMatrices();
	/* Poses this model's skeleton from another one, matched by bone name, for a worn part that
	rides a body's animation. A bone the source also has takes the source's combined matrix; a
	bone only this model has -- the costume-only chains a hairstyle or a dress adds -- is
	rebuilt from its own rest local onto whichever parent was just posed, so it hangs off the
	animated body instead of collapsing.

	Without this a part with extra bones cannot be drawn from its own palette at all: the body's
	palette is shorter, and every vertex weighted past its end reads a zero matrix. Returns how
	many bones the source supplied, so a caller can tell a matched skeleton from an unrelated
	one. Bones are stored parent-before-child, so one forward pass is enough. */
	uint32_t Pose_BonesFrom(const CModel& source);
	bool_t Enable_RootMotionSuppression(
		const char_t* pBoneName, int32_t iVerticalAxis);

	// Scales only the preserved root translation axis; geometry and clip time stay unchanged.
	bool_t Set_RootMotionVerticalScale(f32_t fScale);
	f32_t Get_RootMotionVerticalScale() const { return m_fRootMotionVerticalScale; }

	void Set_Animation(uint32_t iAnimIndex, bool_t isLoop = false,
		f32_t fBlendSeconds = 0.f) {
		if (iAnimIndex >= m_iNumAnimations)
			return;
		m_bExplicitAnimationPose = false;
		if (iAnimIndex != m_iCurrentAnimIndex)
			Begin_AnimBlend(fBlendSeconds);
		m_isAnimLoop = isLoop;
		m_iCurrentAnimIndex = iAnimIndex;
	}
	bool_t Set_Animation(const char_t* pAnimationName,
		bool_t isLoop = false, f32_t fBlendSeconds = 0.f);
	bool_t Is_AnimBlending() const {
		return m_fBlendElapsed < m_fBlendDuration;
	}
	void Skip_Blend() {
		m_fBlendDuration = 0.f;
		m_fBlendElapsed = 0.f;
	}
	bool_t Start_Animation(uint32_t iAnimIndex, bool_t isLoop = true);
	bool_t Start_Animation(const char_t* pAnimationName,
		bool_t isLoop = true);
	void Stop_Animation();
	void Set_AnimationSpeed(f32_t speed);
	bool_t Update_Animation(f32_t fTimeDelta);
    // Advances the existing clip/loop/blend clocks, leaving the palette untouched.
    // The caller evaluates Play_Animation(0) before rendering or changing an action.
    // Return value is the same finished flag as Play_Animation/Update_Animation.
    bool_t Advance_AnimationClock(f32_t fTimeDelta, bool_t applyAnimationSpeed = true);
	uint64_t Get_SkeletonHash() const {
		return m_iSkeletonHash;
	}
	HRESULT Attach_AnimationSet(const CModel& animationSet);
	// Source sampling never changes the model cursor or live pose. Time is seconds.
	bool_t Sample_AnimationLocalTransforms(const char_t* name, f32_t seconds, vector<float4x4_t>& output) const;
	// Stages every channel before one commit; native names are immutable.
	bool_t Install_AuthoredAnimations(const vector<MODEL_ANIMATION_DATA>& animations, string& status);

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix);
	HRESULT Initialize_Prototype(MODEL eType,
		const MODEL_ASSET_LOAD_DESC& loadDesc,
		fmatrix_t PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Render(uint32_t iMeshIndex);
	HRESULT Render_Instanced(
		uint32_t iMeshIndex, ID3D11Buffer* pInstanceBuffer,
		uint32_t iInstanceStride, uint32_t iNumInstances,
		uint32_t iInstanceByteOffset = 0u,
        const MESH_SCREEN_LOD_DESC* screenLod = nullptr);
	/* Preparation is explicit: the caller owns the proof that every source
	   submesh in this contiguous range uses the same effective draw state.
	   Original meshes/material slots remain intact. S_FALSE means this model
	   cannot batch (not retained, skinned, or made mutable for morphing).
	   Failure leaves the output handle and every existing cache entry intact. */
	struct ORDERED_STATIC_GEOMETRY_RANGE final
	{
		uint32_t iSourceMesh = 0u, iSourceMaterial = 0u;
		uint32_t iFirstVertex = 0u, iVertexCount = 0u;
		uint32_t iFirstIndex = 0u, iIndexCount = 0u;
	};
	HRESULT Prepare_OrderedStaticGeometry(
		uint32_t iFirstMesh, uint32_t iMeshCount, uint32_t& iOutHandle);
	bool_t Get_OrderedStaticGeometryRanges(uint32_t iHandle,
		std::span<const ORDERED_STATIC_GEOMETRY_RANGE>& OutRanges) const;
	HRESULT Render_OrderedStaticGeometryInstanced(uint32_t iHandle,
		ID3D11Buffer* pInstanceBuffer, uint32_t iInstanceStride,
		uint32_t iNumInstances, uint32_t iInstanceByteOffset = 0u);
	bool_t Play_Animation(f32_t fTimeDelta);
    // World Sequence preparation opts in once; local/combined bones and all
    // model state remain independent. Applies in both Debug and Release.
    void Enable_AnimationSampleReuse();
	HRESULT Bind_BoneMatrices(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex);
	// Copy the actual mesh skin palette without changing this clone's pose or clock.
	bool_t Capture_BoneMatrices(uint32_t iMeshIndex, vector<float4x4_t>& outMatrices) const;
	HRESULT Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType, uint32_t iTextureIndex = 0);
	/* Repaints one texture slot of every material whose name contains pMaterialNameFragment,
	for the character-creation choices that change a face's look without changing its mesh.
	Matching by name fragment rather than by index keeps the caller out of the material order,
	which differs per class rig. Returns how many materials took it, so a caller can tell an
	empty match from a successful one. A null view restores the authored texture. */
	uint32_t Override_MaterialTexture(
		const char_t* pMaterialNameFragment,
		aiTextureType eType,
		uint32_t iTextureIndex,
		ComPtr<ID3D11ShaderResourceView> pTexture);
	void Clear_MaterialTextureOverrides();
	/* The same match by name fragment, for the dyed colour rather than the texture. Only
	materials that actually dye take it, so the count says whether the choice landed on
	anything. Clear restores what the asset shipped. */
	uint32_t Override_MaterialDyeColor(
		const char_t* pMaterialNameFragment,
		const float4_t& vDiffuse,
		const float4_t& vRegionA);
	void Clear_MaterialDyeColorOverrides();
	/* Hair only; see CMaterial::Set_DyeTwoTone. */
	uint32_t Override_MaterialDyeTwoTone(
		const char_t* pMaterialNameFragment, f32_t fStrength, f32_t fRange);
	/* See CMaterial::Set_DiffuseTint. Unlike the dye overrides this takes on any material,
	because a plain multiply needs no mask to ride on. */
	uint32_t Override_MaterialDiffuseTint(
		const char_t* pMaterialNameFragment, const float4_t& vTint);
	/* The same match by name fragment, for a material drawn by a native source-character
	program: the creation screen's skin and make-up choices are that program's own constants
	and texture registers. See CMaterial::Set_SourceCharacterConstants. Only materials on such
	a program take these, so the count says whether the choice landed on anything at all. */
	uint32_t Override_SourceCharacterConstants(
		const char_t* pMaterialNameFragment,
		const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters);
	uint32_t Override_SourceCharacterTexture(
		const char_t* pMaterialNameFragment, uint32_t iRegister,
		ComPtr<ID3D11ShaderResourceView> pTexture);
	void Clear_SourceCharacterOverrides();
	/* Null when the mesh or its material is out of range. */
	const float4_t* Get_MaterialDiffuseTint(uint32_t iMeshIndex) const;
	bool_t Has_MaterialTexture(uint32_t iMeshIndex, aiTextureType eType, uint32_t iTextureIndex = 0) const;
	/* Null when the mesh or its material is out of range; identity tint (its
	isEnabled false) when the material simply has no colour mask. */
	const MODEL_COLOR_TINT* Get_MaterialColorTint(uint32_t iMeshIndex) const;
	HRESULT Bind_SourceSpecialSurface(shared_ptr<class CShader> shader, uint32_t meshIndex);
    HRESULT Bind_SourceLandscapeSurface(shared_ptr<class CShader> shader, uint32_t meshIndex);
	HRESULT Bind_SurfaceLighting(shared_ptr<class CShader> shader, uint32_t meshIndex);
	HRESULT Bind_SourceCharacter(shared_ptr<class CShader> shader, uint32_t meshIndex);
	HRESULT Bind_SourceCharacterForwardLight(shared_ptr<class CShader> shader, uint32_t meshIndex);
	const MODEL_SURFACE_PARAMETERS* Get_MaterialSurface(uint32_t iMeshIndex) const;
	// Invalid material references also opt out of callers' static texture assumptions.
	bool_t Has_MaterialTextureOverrides(uint32_t iMeshIndex) const;
	HRESULT Bind_SurfaceTexture(shared_ptr<class CShader> pShader,
		const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType);
	// Returns the preserved source material slot, independently of mesh order.
	bool_t Try_GetSourceMaterialIndex(uint32_t iMeshIndex, uint32_t& iOutMaterialIndex) const;
	const string& Get_MaterialName(uint32_t iMeshIndex) const;
	uint64_t Get_MaterialNameHash(uint32_t iMeshIndex) const;

public:
	/* Face MorphTarget application (character-creation base tab). A vertex is addressed as
	(iMeshIndex, iVertexIndex): iMeshIndex indexes this CModel's own m_Meshes -- one CMesh per
	submesh, in the exact order CModel::Ready_Meshes built them in, which is the same order
	the .wmodel's own SUBMESH_DESC table lists them (see
	Tools/CharacterCustomizing/build_face_morph_vertex_map.py's global_to_local(), which
	produces the (meshIndex, localIndex) pairs a .facemorphmap on disk stores). iVertexIndex
	is local to that one CMesh's own vertex buffer, 0..Get_MeshVertexCount(iMeshIndex)-1.
	Nothing here is opt-in at load time and no model pays for it until
	Make_MeshVertexBuffer_Unique() is actually called on it. */
	uint32_t Get_MeshCount() const {
		return m_iNumMeshes;
	}
	uint32_t Get_MeshVertexCount(uint32_t iMeshIndex) const;
	bool_t Has_MorphBaseVertices(uint32_t iMeshIndex) const;
	// True only when the existing immutable mesh can consume screen-space LOD.
	bool_t Has_StaticMeshLod(uint32_t iMeshIndex) const;
	bool_t Get_MorphBaseVertex(uint32_t iMeshIndex, uint32_t iVertexIndex,
		float3_t& OutPosition, float3_t& OutNormal) const;
	/* Must be called once (per CModel instance, i.e. per clone) before Update_Mesh_Vertices()
	targets that mesh; a no-op if already unique. */
	HRESULT Make_MeshVertexBuffer_Unique(uint32_t iMeshIndex);
	HRESULT Update_Mesh_Vertices(uint32_t iMeshIndex, const vector<uint32_t>& iVertexIndices,
		const vector<float3_t>& Positions, const vector<float3_t>& Normals);
	/* Restores every vertex Update_Mesh_Vertices() has touched on this mesh back to its
	unmorphed rest state (weight-0). */
	HRESULT Reset_Mesh_Vertices(uint32_t iMeshIndex);

private:
	const aiScene*						m_pAIScene = { nullptr };
	/* Built only for a model that actually goes through Assimp. Its constructor registers
	every importer Assimp ships, and the runtime loads .wmodel exclusively, so as a plain
	member it built that whole registry for every model in the game and never read a file
	with it -- which is also what the CRT leak dump was full of. */
	unique_ptr<Assimp::Importer>		m_pImporter;

private:
	MODEL								m_eType = { MODEL::END };
	uint32_t							m_iNumMeshes = {};
	vector<shared_ptr<class CMesh>>		m_Meshes;
	shared_ptr<const MODEL_MATERIAL_SOURCE> m_pMaterialSource;
	float4x4_t							m_PreTransformMatrix = {};
	bool_t m_bRetainOrderedStaticGeometry = false;
	shared_ptr<const vector<MODEL_MESH_DATA>> m_pOrderedStaticGeometrySource;
	struct ORDERED_STATIC_GEOMETRY final
	{
		uint32_t iHandle = 0u, iFirstMesh = 0u, iMeshCount = 0u;
		shared_ptr<class CMesh> pMesh;
		vector<ORDERED_STATIC_GEOMETRY_RANGE> Ranges;
	};
	vector<shared_ptr<const ORDERED_STATIC_GEOMETRY>> m_OrderedStaticGeometry;
	uint32_t m_iNextOrderedStaticGeometryHandle = 0u;
	bool_t Can_UseOrderedStaticGeometry(uint32_t iFirstMesh, uint32_t iMeshCount) const;

	uint32_t							m_iNumMaterials = {};
	vector<shared_ptr<class CMaterial>>	m_Materials;

	vector<shared_ptr<class CBone>>		m_Bones;
	// Pose data belongs to the model clone, while mesh geometry remains shared.
	struct SKIN_PALETTE final
	{
		vector<float4x4_t> Matrices;
		uint64_t iPoseRevision = 0u;
	};
	vector<SKIN_PALETTE> m_SkinPalettes;
	uint64_t m_iBonePoseRevision = 1u;
	void Invalidate_SkinPalettes();
	/* Pose_BonesFrom's name join, kept because it is the same two skeletons every frame.
	-1 marks a bone the source does not have. */
	vector<int32_t>						m_SourcePoseBoneIndices;
	const CModel*						m_pSourcePoseModel = { nullptr };
	weak_ptr<const CPrototype> m_SourcePoseOwner;
	// Both poses must still match the copy, including same-frame local edits.
	uint64_t m_iSourcePoseRevision = 0u;
	uint64_t m_iCopiedPoseRevision = 0u;
	uint32_t m_iSourcePoseSuppliedBones = 0u;
	vector<float4x4_t>					m_BoneRestLocalTransforms;
	uint64_t							m_iSkeletonHash = {};

	uint32_t								m_iCurrentAnimIndex = {};
	uint32_t								m_iNumAnimations = {};
	vector<shared_ptr<class CAnimation>>	m_Animations;
	std::set<string> m_AuthoredAnimationNames;
	bool_t									m_isAnimLoop = { false };
	bool_t									m_isAnimPaused = { false };
	f32_t									m_fAnimationSpeed = { 1.f };
	int32_t									m_iRootMotionBoneIndex = { -1 };
	int32_t									m_iRootMotionVerticalAxis = { -1 };
	float3_t								m_vRootMotionRestTranslation = {};
	float3_t m_vRootMotionUnscaledTranslation = {};
	f32_t m_fRootMotionVerticalScale = 1.f;
	vector<float4x4_t>						m_BlendFromPose;
	f32_t									m_fBlendElapsed = {};
	f32_t									m_fBlendDuration = {};
	bool_t									m_bHasLocalBounds = { false };
	float3_t								m_vLocalBoundsMin = {};
	float3_t								m_vLocalBoundsMax = {};
    mutable bool_t m_bAnimationEnvelopeAttempted = false;
    mutable f32_t m_fAnimationEnvelopeRadius = -1.f;
    bool_t m_bAnimationEnvelopeExternalPose = false;
    bool_t Build_AnimationEnvelopeRadius(f32_t& radius) const;
    bool_t m_bHasBindGeometryBounds = false;
    float3_t m_vBindGeometryBoundsMin{}, m_vBindGeometryBoundsMax{};
	bool_t									m_bHasSelfConsistentUnauthenticatedGeometryMetadata = { false };
	uint16_t								m_iGeometryFormatVersionMajor = {};
	uint16_t								m_iGeometryFormatVersionMinor = {};
	uint32_t								m_iGeometryChannelMask = {};
	uint32_t								m_iGeometryEvidenceFlags = {};
	f32_t									m_fGeometryPreScale = { 1.f };
	array<uint8_t, 32>						m_GeometryPayloadSha256 = {};
	array<uint8_t, 32>						m_GeometryMetadataIdentitySha256 = {};

private:
	bool_t Sample_BoneCombinedMatricesForAnimation(
		uint32_t iExpectedAnimationIndex,
		f32_t fTrackPositionTicks,
		bool_t bUseCurrentPoseAndBlend,
		f32_t fBlendElapsedSeconds,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	bool_t Build_AnimationTransitionPose(const ANIMATION_TRANSITION_POSE& pose,
		vector<float4x4_t>& local, vector<float4x4_t>& combined, float3_t* unscaledRoot = nullptr) const;
	ANIMATION_TRANSITION_POSE m_ExplicitAnimationPose;
	bool_t m_bExplicitAnimationPose = false;
	void Begin_AnimBlend(f32_t fBlendSeconds);
	void Restore_UnscaledRootVertical(float4x4_t& Local) const;
	void Apply_RootMotionTranslation(float4x4_t& Local) const;
	void Update_AnimBlend(f32_t fTimeDelta);
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const char_t* pModelFilePath);
	HRESULT Ready_Bones(const aiNode* pAINode, int32_t iParentBoneIndex = -1);
	HRESULT Ready_Animations();
	HRESULT Ready_BinaryModel(const char_t* pModelFilePath);
	HRESULT Ready_BinaryModel(const MODEL_ASSET_LOAD_DESC& loadDesc);
	HRESULT Apply_MaterialOverrides(MODEL_MATERIAL_SOURCE& source, const MODEL_ASSET_LOAD_DESC& loadDesc);
	HRESULT Ready_Meshes(const MODEL_ASSET_DATA& asset);
	HRESULT Ready_Materials(const MODEL_ASSET_DATA& asset);
	HRESULT Ready_Bones(const MODEL_ASSET_DATA& asset);
	HRESULT Ready_Animations(const MODEL_ASSET_DATA& asset);
	void Reset_LocalBounds();
    void Include_BindGeometryPosition(fvector_t position);
	void Include_LocalPosition(fvector_t vPosition);

public:
	static unique_ptr<CModel> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix,
		bool_t bRetainOrderedStaticGeometry = false);
	static unique_ptr<CModel> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		MODEL eType,
		const MODEL_ASSET_LOAD_DESC& loadDesc,
		fmatrix_t PreTransformMatrix,
		bool_t bRetainOrderedStaticGeometry = false);
    // Reuses immutable GPU geometry and clones the pose with independent materials.
    // Load identity must match the prototype; this cannot retarget geometry.
    static unique_ptr<CModel> Create_MaterialVariant(const CModel& prototype,
        const MODEL_ASSET_LOAD_DESC& loadDesc);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	void Free();
};

NS_END
```

### Engine/Private/Model.cpp

변경 종류: 기존 파일 함수·선언 추가.

```cpp
#include "Model.h"
#include "SourceCharacterProgramRegistry.h"
#pragma push_macro("new")
#undef new
#include "Assimp/Importer.hpp"
#include "Assimp/postprocess.h"
#pragma pop_macro("new")
#pragma push_macro("new")
#undef new
#include "Assimp/scene.h"
#pragma pop_macro("new")
#include "Engine_VertexTypes.h"
#include "Profiler.h"
#include "GameInstance.h"

#include "BinaryAsset/ModelAssetData.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Mesh.h"
#include "Bone.h"
#include "Shader.h"
#include "Material.h"
#include "Animation.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>

namespace Engine
{
    struct MODEL_MATERIAL_SOURCE
    {
        struct MESH_CHANNELS
        {
            uint32_t materialIndex;
            MODEL_VERTEX_KIND vertexKind;
            bool_t hasColor0, hasTexcoord1, hasTexcoord2, hasTangentHandedness;
        };
        MODEL_ASSET_LOAD_DESC identity;
        vector<MODEL_MATERIAL_DATA> materials;
        vector<MESH_CHANNELS> meshes;
    };
}

namespace
{
    bool NativeHairUsesExtraUV(const Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& source)
    {
        // Program 7 uses UV1 only for two-tone colour interpolation. Some retail
        // hair meshes contain UV0 alone and disable this exact native branch.
        return source.program == 7u &&
            (source.baseConstants[16].x != 0.f || source.lightConstants[13].x != 0.f ||
             source.baseConstants[18].y == 0.f || source.baseConstants[18].w == 0.f ||
             source.lightConstants[15].y == 0.f || source.lightConstants[15].w == 0.f);
    }

	bool Is_FiniteMatrix(const float4x4_t& Matrix)
	{
		const f32_t* const Values = &Matrix._11;
		for (size_t i = 0u; i < 16u; ++i)
		{
			if (!std::isfinite(Values[i]))
				return false;
		}
		return true;
	}

	bool Try_BlendLocalMatrix(
		const float4x4_t& From,
		const f32_t fWeight,
		float4x4_t& InOutTarget)
	{
		vector_t FromScale{};
		vector_t FromRotation{};
		vector_t FromTranslation{};
		vector_t TargetScale{};
		vector_t TargetRotation{};
		vector_t TargetTranslation{};
		if (!XMMatrixDecompose(&FromScale, &FromRotation, &FromTranslation,
				XMLoadFloat4x4(&From)) ||
			!XMMatrixDecompose(&TargetScale, &TargetRotation,
				&TargetTranslation, XMLoadFloat4x4(&InOutTarget)))
		{
			return false;
		}
		XMStoreFloat4x4(&InOutTarget,
			XMMatrixAffineTransformation(
				XMVectorLerp(FromScale, TargetScale, fWeight),
				XMVectorZero(),
				XMQuaternionSlerp(FromRotation, TargetRotation, fWeight),
				XMVectorLerp(FromTranslation, TargetTranslation, fWeight)));
		return Is_FiniteMatrix(InOutTarget);
	}
}

CModel::CModel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CComponent { pDevice, pContext }
{
}

CModel::CModel(const CModel& Prototype)
    : CComponent { Prototype }
    , m_pAIScene { Prototype.m_pAIScene }
    , m_eType { Prototype.m_eType }
    , m_iNumMeshes { Prototype.m_iNumMeshes }
    , m_Meshes { Prototype.m_Meshes }
    , m_pMaterialSource { Prototype.m_pMaterialSource }
    , m_PreTransformMatrix { Prototype.m_PreTransformMatrix }
    , m_bRetainOrderedStaticGeometry { Prototype.m_bRetainOrderedStaticGeometry }
    , m_pOrderedStaticGeometrySource { Prototype.m_pOrderedStaticGeometrySource }
    , m_OrderedStaticGeometry { Prototype.m_OrderedStaticGeometry }
    , m_iNextOrderedStaticGeometryHandle { Prototype.m_iNextOrderedStaticGeometryHandle }
    , m_iNumMaterials { Prototype.m_iNumMaterials }
    , m_Materials { Prototype.m_Materials }
   // , m_Bones { Prototype.m_Bones }
    , m_BoneRestLocalTransforms { Prototype.m_BoneRestLocalTransforms }
    , m_iSkeletonHash { Prototype.m_iSkeletonHash }
    , m_iCurrentAnimIndex { Prototype.m_iCurrentAnimIndex }
    , m_iNumAnimations { Prototype.m_iNumAnimations}
    // , m_Animations { Prototype.m_Animations }
    , m_AuthoredAnimationNames { Prototype.m_AuthoredAnimationNames }
    , m_isAnimLoop { Prototype.m_isAnimLoop }
	, m_isAnimPaused { Prototype.m_isAnimPaused }
	, m_fAnimationSpeed { Prototype.m_fAnimationSpeed }
	, m_iRootMotionBoneIndex { Prototype.m_iRootMotionBoneIndex }
	, m_iRootMotionVerticalAxis { Prototype.m_iRootMotionVerticalAxis }
	, m_vRootMotionRestTranslation { Prototype.m_vRootMotionRestTranslation }
	, m_vRootMotionUnscaledTranslation { Prototype.m_vRootMotionUnscaledTranslation }
	, m_fRootMotionVerticalScale { Prototype.m_fRootMotionVerticalScale }
	, m_bHasLocalBounds { Prototype.m_bHasLocalBounds }
	, m_vLocalBoundsMin { Prototype.m_vLocalBoundsMin }
	, m_vLocalBoundsMax { Prototype.m_vLocalBoundsMax }
    , m_bAnimationEnvelopeAttempted { Prototype.m_bAnimationEnvelopeAttempted }
    , m_fAnimationEnvelopeRadius { Prototype.m_fAnimationEnvelopeRadius }
    , m_bAnimationEnvelopeExternalPose { Prototype.m_bAnimationEnvelopeExternalPose }
    , m_bHasBindGeometryBounds { Prototype.m_bHasBindGeometryBounds }
    , m_vBindGeometryBoundsMin { Prototype.m_vBindGeometryBoundsMin }
    , m_vBindGeometryBoundsMax { Prototype.m_vBindGeometryBoundsMax }
	, m_bHasSelfConsistentUnauthenticatedGeometryMetadata { Prototype.m_bHasSelfConsistentUnauthenticatedGeometryMetadata }
	, m_iGeometryFormatVersionMajor { Prototype.m_iGeometryFormatVersionMajor }
	, m_iGeometryFormatVersionMinor { Prototype.m_iGeometryFormatVersionMinor }
	, m_iGeometryChannelMask { Prototype.m_iGeometryChannelMask }
	, m_iGeometryEvidenceFlags { Prototype.m_iGeometryEvidenceFlags }
	, m_fGeometryPreScale { Prototype.m_fGeometryPreScale }
	, m_GeometryPayloadSha256 { Prototype.m_GeometryPayloadSha256 }
	, m_GeometryMetadataIdentitySha256 { Prototype.m_GeometryMetadataIdentitySha256 }
{
    for (auto& pPrototype : Prototype.m_Bones)
        m_Bones.push_back(pPrototype->Clone());

    for (auto& pPrototype : Prototype.m_Animations)
        m_Animations.push_back(pPrototype->Clone());
}

CModel::~CModel()
{
}

matrix_t CModel::Get_BoneMatrix(const char_t* pBoneName)
{
    auto    iter = find_if(m_Bones.begin(), m_Bones.end(), [&](shared_ptr<CBone> pBone)->bool_t {
        if (true == pBone->Compare_Name(pBoneName))
            return true;
        return false;
    });

    if (iter == m_Bones.end())
        return XMMatrixIdentity();

    return (*iter)->Get_CombinedTransformationMatrix();
}

bool_t CModel::Set_Animation(
    const char_t* pAnimationName,
    bool_t isLoop,
    f32_t fBlendSeconds)
{
    if (nullptr == pAnimationName)
        return false;

    for (uint32_t i = 0; i < m_Animations.size(); ++i)
    {
        if (!m_Animations[i]->Compare_Name(pAnimationName))
            continue;

        if (i != m_iCurrentAnimIndex)
            Begin_AnimBlend(fBlendSeconds);

        m_bExplicitAnimationPose = false;
        m_iCurrentAnimIndex = i;
        m_isAnimLoop = isLoop;
        return true;
    }
    return false;
}

void CModel::Begin_AnimBlend(f32_t fBlendSeconds)
{
    if (fBlendSeconds <= 0.f || m_Bones.empty())
    {
        m_fBlendDuration = 0.f;
        m_fBlendElapsed = 0.f;
        return;
    }

    m_BlendFromPose.resize(m_Bones.size());
    for (size_t i = 0; i < m_Bones.size(); ++i)
        XMStoreFloat4x4(
            &m_BlendFromPose[i],
            m_Bones[i]->Get_TransformationMatrix());

    if (m_fRootMotionVerticalScale != 1.f && m_iRootMotionBoneIndex >= 0 &&
        static_cast<size_t>(m_iRootMotionBoneIndex) < m_BlendFromPose.size())
        Restore_UnscaledRootVertical(m_BlendFromPose[m_iRootMotionBoneIndex]);
    m_fBlendDuration = fBlendSeconds;
    m_fBlendElapsed = 0.f;
}

void CModel::Update_AnimBlend(f32_t fTimeDelta)
{
    Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Blend");
    if (m_fBlendElapsed >= m_fBlendDuration ||
        m_BlendFromPose.size() != m_Bones.size())
        return;

    m_fBlendElapsed += fTimeDelta;
    const f32_t fRatio = m_fBlendDuration > 0.f ?
        (min)(m_fBlendElapsed / m_fBlendDuration, 1.f) : 1.f;

    for (size_t i = 0; i < m_Bones.size(); ++i)
    {
        m_Bones[i]->Blend_TransformationMatrix(
            XMLoadFloat4x4(&m_BlendFromPose[i]),
            fRatio);
    }
}

bool_t CModel::Has_Bone(const char_t* pBoneName)
{
    if (nullptr == pBoneName || '\0' == pBoneName[0])
        return false;

    return m_Bones.end() != find_if(
        m_Bones.begin(),
        m_Bones.end(),
        [&](const shared_ptr<CBone>& pBone)
        {
            return nullptr != pBone && pBone->Compare_Name(pBoneName);
        });
}

vector<string> CModel::Get_BoneNames() const
{
    vector<string> names;
    names.reserve(m_Bones.size());
    for (const auto& bone : m_Bones)
        if (bone) names.emplace_back(bone->Get_Name());
    return names;
}

int32_t CModel::Find_BoneIndex(const char_t* pBoneName) const
{
    if (nullptr == pBoneName || '\0' == pBoneName[0])
        return -1;

    for (size_t i = 0; i < m_Bones.size(); ++i)
    {
        if (nullptr != m_Bones[i] && m_Bones[i]->Compare_Name(pBoneName))
            return static_cast<int32_t>(i);
    }
    return -1;
}

int32_t CModel::Get_BoneParentIndex(const uint32_t iBoneIndex) const
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return -1;

    return m_Bones[iBoneIndex]->Get_ParentBoneIndex();
}

bool_t CModel::Get_BoneLocalMatrix(
    const uint32_t iBoneIndex, matrix_t& outMatrix) const
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return false;

    outMatrix = m_Bones[iBoneIndex]->Get_TransformationMatrix();
    return true;
}

bool_t CModel::Get_BoneRestLocalMatrix(
    const uint32_t iBoneIndex, matrix_t& outMatrix) const
{
    if (iBoneIndex >= m_BoneRestLocalTransforms.size())
        return false;

    outMatrix = XMLoadFloat4x4(&m_BoneRestLocalTransforms[iBoneIndex]);
    return true;
}

bool_t CModel::Get_BoneCombinedMatrix(
    const uint32_t iBoneIndex, matrix_t& outMatrix) const
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return false;

    outMatrix = m_Bones[iBoneIndex]->Get_CombinedTransformationMatrix();
    return true;
}

bool_t CModel::Sample_AnimationBoneCombinedMatrices(
	const char_t* pAnimationName,
	const f32_t fTrackPositionTicks,
	const std::span<const uint32_t> BoneIndices,
	const std::span<float4x4_t> OutCombinedMatrices) const
{
	if (nullptr == pAnimationName || '\0' == pAnimationName[0])
		return false;
	uint32_t iAnimationIndex = UINT32_MAX;
	for (size_t i = 0u; i < m_Animations.size(); ++i)
	{
		if (nullptr == m_Animations[i] ||
			!m_Animations[i]->Compare_Name(pAnimationName))
		{
			continue;
		}
		if (iAnimationIndex != UINT32_MAX)
			return false;
		iAnimationIndex = static_cast<uint32_t>(i);
	}
	return iAnimationIndex != UINT32_MAX &&
		Sample_BoneCombinedMatricesForAnimation(
			iAnimationIndex, fTrackPositionTicks, false, 0.f,
			BoneIndices, OutCombinedMatrices);
}

bool_t CModel::Sample_AnimationRootTranslation(const char_t* name,
    const f32_t ticks, const uint32_t rootIndex, const int32_t verticalAxis,
    const f32_t verticalScale, float3_t& output) const
{
    if (!name || !*name || !std::isfinite(ticks) || ticks < 0.f ||
        verticalAxis < 0 || verticalAxis > 2 || !std::isfinite(verticalScale) ||
        verticalScale < 0.f || rootIndex >= m_Bones.size() ||
        m_BoneRestLocalTransforms.size() != m_Bones.size() ||
        !Is_FiniteMatrix(m_PreTransformMatrix)) return false;
    const CAnimation* animation = nullptr;
    for (const auto& candidate : m_Animations)
        if (candidate && candidate->Compare_Name(name))
        {
            if (animation) return false;
            animation = candidate.get();
        }
    if (!animation || ticks > animation->Get_Duration()) return false;
    auto local = m_BoneRestLocalTransforms;
    auto initial = m_BoneRestLocalTransforms;
    if (!animation->Sample_LocalBoneTransforms(ticks, local) ||
        !animation->Sample_LocalBoneTransforms(0.f, initial)) return false;
    const auto& raw = local[rootIndex];
    const auto& rest = m_BoneRestLocalTransforms[rootIndex];
    if (!m_Bones[rootIndex] || !Is_FiniteMatrix(raw) || !Is_FiniteMatrix(rest)) return false;
    matrix_t basis = XMMatrixIdentity();
    int32_t parent = m_Bones[rootIndex]->Get_ParentBoneIndex();
    uint32_t child = rootIndex;
    while (parent >= 0)
    {
        if (static_cast<uint32_t>(parent) >= child || !m_Bones[parent] ||
            !Is_FiniteMatrix(local[parent]) || !Is_FiniteMatrix(initial[parent]) ||
            !animation->Is_BoneTransformConstant(static_cast<uint32_t>(parent))) return false;
        // Native-key constancy above owns this admission. Re-sampling equal
        // quaternion keys can change a scale-100 matrix by float roundoff
        // (Albion: 2.38e-5 at 5 ms); that is not animated ancestor motion.
        // Always use the admitted initial basis, independent of sample time.
        basis = basis * XMLoadFloat4x4(&initial[parent]);
        child = static_cast<uint32_t>(parent);
        parent = m_Bones[parent]->Get_ParentBoneIndex();
    }
    if (parent != -1) return false;
    float3_t delta{raw._41 - rest._41, raw._42 - rest._42, raw._43 - rest._43};
    if (verticalAxis == 0) delta.x *= verticalScale;
    else if (verticalAxis == 1) delta.y *= verticalScale;
    else delta.z *= verticalScale;
    float3_t result;
    XMStoreFloat3(&result, XMVector3TransformNormal(XMLoadFloat3(&delta),
        basis * XMLoadFloat4x4(&m_PreTransformMatrix)));
    if (!std::isfinite(result.x) || !std::isfinite(result.y) || !std::isfinite(result.z)) return false;
    output = result;
    return true;
}

CModel::ROOT_MOTION_SUPPRESSION_STATE CModel::Capture_RootMotionSuppression() const
{
    return {this, m_iRootMotionBoneIndex, m_iRootMotionVerticalAxis,
        m_vRootMotionRestTranslation, m_vRootMotionUnscaledTranslation, m_fRootMotionVerticalScale};
}

bool_t CModel::Configure_RootMotionSuppressionFromRest(const uint32_t rootIndex,
    const int32_t verticalAxis, const f32_t verticalScale)
{
    if (rootIndex >= m_Bones.size() || !m_Bones[rootIndex] ||
        rootIndex >= m_BoneRestLocalTransforms.size() || verticalAxis < 0 || verticalAxis > 2 ||
        !std::isfinite(verticalScale) || verticalScale < 0.f ||
        !Is_FiniteMatrix(m_BoneRestLocalTransforms[rootIndex])) return false;
    const auto& rest = m_BoneRestLocalTransforms[rootIndex];
    m_iRootMotionBoneIndex = static_cast<int32_t>(rootIndex);
    m_iRootMotionVerticalAxis = verticalAxis;
    m_vRootMotionRestTranslation = {rest._41, rest._42, rest._43};
    float4x4_t current;
    XMStoreFloat4x4(&current, m_Bones[rootIndex]->Get_TransformationMatrix());
    m_vRootMotionUnscaledTranslation = {current._41, current._42, current._43};
    m_bAnimationEnvelopeAttempted = false;
    m_fRootMotionVerticalScale = verticalScale;
    return true;
}

bool_t CModel::Restore_RootMotionSuppression(const ROOT_MOTION_SUPPRESSION_STATE& state)
{
    if (state.owner != this || state.boneIndex < -1 ||
        (state.boneIndex >= 0 && static_cast<size_t>(state.boneIndex) >= m_Bones.size())) return false;
    m_iRootMotionBoneIndex = state.boneIndex;
    m_iRootMotionVerticalAxis = state.verticalAxis;
    m_vRootMotionRestTranslation = state.restTranslation;
    m_vRootMotionUnscaledTranslation = state.unscaledTranslation;
    m_bAnimationEnvelopeAttempted = false;
    m_fRootMotionVerticalScale = state.verticalScale;
    return true;
}

bool_t CModel::Sample_AnimationTransitionBoneCombinedMatrices(
    const ANIMATION_TRANSITION_POSE& pose,
    const std::span<const uint32_t> BoneIndices,
    const std::span<float4x4_t> OutCombinedMatrices) const
{
    if (BoneIndices.empty() || BoneIndices.size() != OutCombinedMatrices.size() ||
        std::any_of(BoneIndices.begin(), BoneIndices.end(),
            [this](uint32_t index) { return index >= m_Bones.size(); })) return false;
    std::vector<float4x4_t> local, combined;
    if (!Build_AnimationTransitionPose(pose, local, combined)) return false;
    for (size_t index = 0u; index < BoneIndices.size(); ++index)
        OutCombinedMatrices[index] = combined[BoneIndices[index]];
    return true;
}

bool_t CModel::Build_AnimationTransitionPose(const ANIMATION_TRANSITION_POSE& pose,
    vector<float4x4_t>& local, vector<float4x4_t>& combined, float3_t* unscaledRoot) const
{
    Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Transition.Build");
    if (m_Bones.empty() || m_BoneRestLocalTransforms.size() != m_Bones.size() ||
        !std::isfinite(pose.durationSeconds) || pose.durationSeconds <= 0.f || pose.durationSeconds > 1.f ||
        !std::isfinite(pose.elapsedSeconds) || pose.elapsedSeconds < 0.f ||
        !std::isfinite(pose.playRate) || pose.playRate <= 0.f || !Is_FiniteMatrix(m_PreTransformMatrix)) return false;
    auto sample = [&](uint32_t index, float ticks, vector<float4x4_t>& out)
    {
        if (!std::isfinite(ticks) || ticks < 0.f) return false;
        out = m_BoneRestLocalTransforms;
        if (index == UINT32_MAX) return ticks == 0.f;
        return index < m_Animations.size() && m_Animations[index] &&
            std::isfinite(m_Animations[index]->Get_Duration()) && ticks <= m_Animations[index]->Get_Duration() &&
            m_Animations[index]->Sample_LocalBoneTransforms(ticks, out);
    };
    vector<float4x4_t> from;
    if (!sample(pose.sourceIndex, pose.sourceTicks, from) || !sample(pose.targetIndex, pose.targetTicks, local)) return false;
    const float alpha = (std::min)(pose.elapsedSeconds / pose.durationSeconds, 1.f);
    {
        Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Blend");
    for (size_t i = 0; i < local.size(); ++i)
        if (!m_Bones[i] || !Is_FiniteMatrix(from[i]) || !Is_FiniteMatrix(local[i]) ||
            (alpha < 1.f && !Try_BlendLocalMatrix(from[i], alpha, local[i]))) return false;
    }
    if (m_iRootMotionBoneIndex >= 0)
    {
        if (size_t(m_iRootMotionBoneIndex) >= local.size()) return false;
        const auto& root = local[m_iRootMotionBoneIndex];
        if (unscaledRoot) *unscaledRoot = {root._41, root._42, root._43};
        Apply_RootMotionTranslation(local[m_iRootMotionBoneIndex]);
    }
    {
        Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Bones.Combine");
    combined.resize(local.size());
    for (size_t i = 0; i < local.size(); ++i)
    {
        const int parent = m_Bones[i]->Get_ParentBoneIndex();
        if (parent < -1 || (parent >= 0 && size_t(parent) >= i)) return false;
        XMStoreFloat4x4(&combined[i], XMLoadFloat4x4(&local[i]) *
            (parent == -1 ? XMLoadFloat4x4(&m_PreTransformMatrix) : XMLoadFloat4x4(&combined[parent])));
        if (!Is_FiniteMatrix(combined[i])) return false;
    }
    }
    return true;
}

bool_t CModel::Set_AnimationTransitionPose(const ANIMATION_TRANSITION_POSE& pose)
{
    Engine::CProfilerModelAnimationScope modelAnimationScope(CGameInstance::Get().Get_Profiler(), this);
    vector<float4x4_t> local, combined;
    float3_t unscaledRoot = m_vRootMotionUnscaledTranslation;
    if (!Build_AnimationTransitionPose(pose, local, combined, &unscaledRoot)) return false;
    // Admission completes before touching either the cursor or live palette.
    if (pose.targetIndex != UINT32_MAX)
    {
        m_iCurrentAnimIndex = pose.targetIndex;
        m_Animations[pose.targetIndex]->Set_TrackPosition(pose.targetTicks);
    }
    for (size_t i = 0; i < local.size(); ++i)
        m_Bones[i]->Update_TransformationMatrix(XMLoadFloat4x4(&local[i]));
    Refresh_BoneCombinedMatrices();
    m_vRootMotionUnscaledTranslation = unscaledRoot;
    Skip_Blend();
    m_isAnimLoop = false;
    m_bAnimationEnvelopeExternalPose = true;
    m_ExplicitAnimationPose = pose;
    m_bExplicitAnimationPose = true;
    return true;
}

bool_t CModel::Sample_CurrentAnimationBoneCombinedMatrices(
	const uint32_t iExpectedAnimationIndex,
	const f32_t fTrackPositionTicks,
	const std::span<const uint32_t> BoneIndices,
	const std::span<float4x4_t> OutCombinedMatrices) const
{
	return Sample_CurrentAnimationBoneCombinedMatricesAtBlendElapsed(
		iExpectedAnimationIndex, fTrackPositionTicks, m_fBlendElapsed,
		BoneIndices, OutCombinedMatrices);
}

bool_t CModel::Sample_CurrentAnimationBoneCombinedMatricesAtBlendElapsed(
	const uint32_t iExpectedAnimationIndex,
	const f32_t fTrackPositionTicks,
	const f32_t fBlendElapsedSeconds,
	const std::span<const uint32_t> BoneIndices,
	const std::span<float4x4_t> OutCombinedMatrices) const
{
	if (m_bExplicitAnimationPose && iExpectedAnimationIndex == m_iCurrentAnimIndex)
	{
		if (BoneIndices.empty() || BoneIndices.size() != OutCombinedMatrices.size() ||
			iExpectedAnimationIndex >= m_Animations.size()) return false;
		auto pose = m_ExplicitAnimationPose;
		const float tps = Get_AnimationTickPerSecond(iExpectedAnimationIndex);
		if (!std::isfinite(tps) || tps <= 0.f) return false;
		pose.elapsedSeconds = (std::max)(0.f, pose.elapsedSeconds +
			(fTrackPositionTicks - pose.targetTicks) / (tps * pose.playRate));
		pose.targetTicks = fTrackPositionTicks;
		vector<float4x4_t> local, combined;
		if (!Build_AnimationTransitionPose(pose, local, combined)) return false;
		for (const auto index : BoneIndices) if (index >= combined.size()) return false;
		for (size_t i = 0; i < BoneIndices.size(); ++i) OutCombinedMatrices[i] = combined[BoneIndices[i]];
		return true;
	}
	return Sample_BoneCombinedMatricesForAnimation(
		iExpectedAnimationIndex, fTrackPositionTicks, true, fBlendElapsedSeconds,
		BoneIndices, OutCombinedMatrices);
}

bool_t CModel::Sample_BoneCombinedMatricesForAnimation(
	const uint32_t iExpectedAnimationIndex,
	const f32_t fTrackPositionTicks,
	const bool_t bUseCurrentPoseAndBlend,
	const f32_t fBlendElapsedSeconds,
	const std::span<const uint32_t> BoneIndices,
	const std::span<float4x4_t> OutCombinedMatrices) const
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.History.Sample");
	if ((bUseCurrentPoseAndBlend && iExpectedAnimationIndex != m_iCurrentAnimIndex) ||
		iExpectedAnimationIndex >= m_Animations.size() ||
		nullptr == m_Animations[iExpectedAnimationIndex] ||
		!std::isfinite(fTrackPositionTicks) ||
		!std::isfinite(fBlendElapsedSeconds) ||
		fBlendElapsedSeconds < 0.f ||
		fTrackPositionTicks < 0.f ||
		fTrackPositionTicks >
			m_Animations[iExpectedAnimationIndex]->Get_Duration() ||
		!std::isfinite(
			m_Animations[iExpectedAnimationIndex]->Get_Duration()) ||
		BoneIndices.empty() ||
		BoneIndices.size() != OutCombinedMatrices.size() ||
		m_Bones.empty() ||
		(!bUseCurrentPoseAndBlend && m_BoneRestLocalTransforms.size() != m_Bones.size()))
	{
		return false;
	}
	for (const uint32_t iBoneIndex : BoneIndices)
	{
		if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
			return false;
	}

	const auto BuildCombined = [this, iExpectedAnimationIndex, bUseCurrentPoseAndBlend,
		fTrackPositionTicks, fBlendElapsedSeconds](
			std::vector<float4x4_t>& OutCombined)
	{
		std::vector<float4x4_t> LocalTransforms(m_Bones.size());
		for (size_t iBone = 0u; iBone < m_Bones.size(); ++iBone)
		{
			if (nullptr == m_Bones[iBone])
				return false;
			if (bUseCurrentPoseAndBlend)
			{
				XMStoreFloat4x4(&LocalTransforms[iBone],
					m_Bones[iBone]->Get_TransformationMatrix());
			}
			else
			{
				LocalTransforms[iBone] = m_BoneRestLocalTransforms[iBone];
			}
			if (!Is_FiniteMatrix(LocalTransforms[iBone]))
				return false;
		}
		if (bUseCurrentPoseAndBlend && m_fRootMotionVerticalScale != 1.f && m_iRootMotionBoneIndex >= 0 &&
			static_cast<size_t>(m_iRootMotionBoneIndex) < LocalTransforms.size())
			Restore_UnscaledRootVertical(LocalTransforms[m_iRootMotionBoneIndex]);
		if (!m_Animations[iExpectedAnimationIndex]->Sample_LocalBoneTransforms(
				fTrackPositionTicks, LocalTransforms))
		{
			return false;
		}

		if (bUseCurrentPoseAndBlend && (!std::isfinite(m_fBlendElapsed) ||
			!std::isfinite(m_fBlendDuration) ||
			m_fBlendElapsed < 0.f || m_fBlendDuration < 0.f))
		{
			return false;
		}
		/* Only an active live transition proves that m_BlendFromPose belongs to
		   this clip edge.  Once the live transition is complete (or when the same
		   clip is restarted without a new blend), ignore the stale saved pose. */
		if (bUseCurrentPoseAndBlend && m_fBlendElapsed < m_fBlendDuration &&
			fBlendElapsedSeconds < m_fBlendDuration)
		{
			if (m_fBlendDuration <= 0.f ||
				m_BlendFromPose.size() != LocalTransforms.size())
			{
				return false;
			}
			const f32_t fRatio =
				(min)(fBlendElapsedSeconds / m_fBlendDuration, 1.f);
			for (size_t iBone = 0u; iBone < LocalTransforms.size(); ++iBone)
			{
				if (!Is_FiniteMatrix(m_BlendFromPose[iBone]) ||
					!Try_BlendLocalMatrix(
						m_BlendFromPose[iBone], fRatio,
						LocalTransforms[iBone]))
				{
					return false;
				}
			}
		}

		if (m_iRootMotionBoneIndex >= 0)
		{
			if (static_cast<size_t>(m_iRootMotionBoneIndex) >=
					LocalTransforms.size() ||
				m_iRootMotionVerticalAxis < -1 ||
				m_iRootMotionVerticalAxis > 2)
			{
				return false;
			}
			float4x4_t& Root =
				LocalTransforms[static_cast<size_t>(m_iRootMotionBoneIndex)];
			Apply_RootMotionTranslation(Root);
		}

		OutCombined.resize(LocalTransforms.size());
		const matrix_t PreTransform =
			XMLoadFloat4x4(&m_PreTransformMatrix);
		if (!Is_FiniteMatrix(m_PreTransformMatrix))
			return false;
		for (size_t iBone = 0u; iBone < LocalTransforms.size(); ++iBone)
		{
			const int32_t iParent = m_Bones[iBone]->Get_ParentBoneIndex();
			matrix_t Combined;
			if (-1 == iParent)
			{
				Combined = XMLoadFloat4x4(&LocalTransforms[iBone]) *
					PreTransform;
			}
			else
			{
				if (iParent < 0 || static_cast<size_t>(iParent) >= iBone)
					return false;
				Combined = XMLoadFloat4x4(&LocalTransforms[iBone]) *
					XMLoadFloat4x4(
						&OutCombined[static_cast<size_t>(iParent)]);
			}
			XMStoreFloat4x4(&OutCombined[iBone], Combined);
			if (!Is_FiniteMatrix(OutCombined[iBone]))
				return false;
		}
		return true;
	};

#if defined(_DEBUG)
	const uint32_t iCurrentAnimationBefore = m_iCurrentAnimIndex;
	const bool_t bPausedBefore = m_isAnimPaused;
	const bool_t bLoopBefore = m_isAnimLoop;
	const f32_t fBlendElapsedBefore = m_fBlendElapsed;
	const f32_t fBlendDurationBefore = m_fBlendDuration;
	const f32_t fTrackPositionBefore =
		m_Animations[iExpectedAnimationIndex]->Get_CurrentTrackPosition();
	const std::vector<uint32_t> LeftKeyFrameIndicesBefore =
		m_Animations[iExpectedAnimationIndex]->m_iLeftKeyFrameIndices;
	std::vector<float4x4_t> LiveLocalBefore(m_Bones.size());
	std::vector<float4x4_t> LiveCombinedBefore(m_Bones.size());
	for (size_t iBone = 0u; iBone < m_Bones.size(); ++iBone)
	{
		XMStoreFloat4x4(&LiveLocalBefore[iBone],
			m_Bones[iBone]->Get_TransformationMatrix());
		XMStoreFloat4x4(&LiveCombinedBefore[iBone],
			m_Bones[iBone]->Get_CombinedTransformationMatrix());
	}
#endif

	std::vector<float4x4_t> StagedCombined;
	if (!BuildCombined(StagedCombined))
		return false;

#if defined(_DEBUG)
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.History.DebugVerify");
	std::vector<float4x4_t> DeterministicCombined;
	if (!BuildCombined(DeterministicCombined) ||
		iCurrentAnimationBefore != m_iCurrentAnimIndex ||
		bPausedBefore != m_isAnimPaused || bLoopBefore != m_isAnimLoop ||
		fBlendElapsedBefore != m_fBlendElapsed || fBlendDurationBefore != m_fBlendDuration ||
		DeterministicCombined.size() != StagedCombined.size() ||
		0 != std::memcmp(DeterministicCombined.data(), StagedCombined.data(),
			StagedCombined.size() * sizeof(float4x4_t)) ||
		fTrackPositionBefore !=
			m_Animations[iExpectedAnimationIndex]->Get_CurrentTrackPosition() ||
		LeftKeyFrameIndicesBefore !=
			m_Animations[iExpectedAnimationIndex]->m_iLeftKeyFrameIndices)
	{
		return false;
	}
	for (size_t iBone = 0u; iBone < m_Bones.size(); ++iBone)
	{
		float4x4_t LiveLocalAfter{};
		float4x4_t LiveCombinedAfter{};
		XMStoreFloat4x4(&LiveLocalAfter,
			m_Bones[iBone]->Get_TransformationMatrix());
		XMStoreFloat4x4(&LiveCombinedAfter,
			m_Bones[iBone]->Get_CombinedTransformationMatrix());
		if (0 != std::memcmp(&LiveLocalBefore[iBone], &LiveLocalAfter,
				sizeof(float4x4_t)) ||
			0 != std::memcmp(&LiveCombinedBefore[iBone], &LiveCombinedAfter,
				sizeof(float4x4_t)))
		{
			return false;
		}
	}
	}
#endif

	std::vector<float4x4_t> StagedOutput(BoneIndices.size());
	for (size_t i = 0u; i < BoneIndices.size(); ++i)
		StagedOutput[i] = StagedCombined[BoneIndices[i]];
	std::copy(StagedOutput.begin(), StagedOutput.end(),
		OutCombinedMatrices.begin());
	return true;
}

bool_t CModel::Set_BoneLocalMatrix(
    const uint32_t iBoneIndex, fmatrix_t Matrix)
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return false;

    m_bAnimationEnvelopeExternalPose = true;
    m_Bones[iBoneIndex]->Update_TransformationMatrix(Matrix);
    // Costume-only locals are consumed by Pose_BonesFrom before a full refresh.
    m_iCopiedPoseRevision = 0u;
    return true;
}

uint32_t CModel::Pose_BonesFrom(const CModel& source)
{
    m_bAnimationEnvelopeExternalPose = true;
    /* By name, because the two skeletons are cooked separately and neither order nor count
       matches: a worn part carries the body's bones plus its own. */
    // A weak owner distinguishes a new model allocated at a retired source address.
    // Unowned prototypes retain the uncached path without extending their lifetime.
    const auto sourceOwner = source.weak_from_this().lock();
    const bool sameSource = sourceOwner && m_pSourcePoseModel == &source &&
        m_SourcePoseOwner.lock() == sourceOwner;
    if (sameSource && m_iSourcePoseRevision == source.m_iBonePoseRevision &&
        m_iCopiedPoseRevision == m_iBonePoseRevision)
        return m_iSourcePoseSuppliedBones;

    if (!sameSource || m_SourcePoseBoneIndices.size() != m_Bones.size())
    {
        m_SourcePoseBoneIndices.assign(m_Bones.size(), -1);
        for (size_t index = 0; index < m_Bones.size(); ++index)
        {
            if (nullptr == m_Bones[index])
                continue;
            for (size_t other = 0; other < source.m_Bones.size(); ++other)
            {
                if (nullptr == source.m_Bones[other] ||
                    !source.m_Bones[other]->Compare_Name(m_Bones[index]->Get_Name()))
                    continue;
                m_SourcePoseBoneIndices[index] = static_cast<int32_t>(other);
                break;
            }
        }
        m_pSourcePoseModel = &source;
        m_SourcePoseOwner = sourceOwner;
    }

    uint32_t supplied = 0u;
    for (size_t index = 0; index < m_Bones.size(); ++index)
    {
        if (nullptr == m_Bones[index])
            continue;
        const int32_t other = m_SourcePoseBoneIndices[index];
        if (other >= 0)
        {
            m_Bones[index]->Set_CombinedTransformationMatrix(
                source.m_Bones[static_cast<size_t>(other)]->Get_CombinedTransformationMatrix());
            ++supplied;
            continue;
        }
        /* A costume-only bone: its parent was posed already, so its own rest local carries it. */
        m_Bones[index]->Update_CombinedTransformationMatrix(
            m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
    }
    Invalidate_SkinPalettes();
    m_iSourcePoseRevision = source.m_iBonePoseRevision;
    m_iCopiedPoseRevision = m_iBonePoseRevision;
    m_iSourcePoseSuppliedBones = supplied;
    return supplied;
}

void CModel::Invalidate_SkinPalettes()
{
    m_iCopiedPoseRevision = 0u;
    if (++m_iBonePoseRevision == 0u)
    {
        m_SkinPalettes.clear();
        m_iBonePoseRevision = 1u;
    }
}

void CModel::Refresh_BoneCombinedMatrices()
{
    Invalidate_SkinPalettes();
    Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Bones.Combine");
    for (auto& pBone : m_Bones)
    {
        pBone->Update_CombinedTransformationMatrix(
            m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
    }
}

bool_t CModel::Enable_RootMotionSuppression(
    const char_t* pBoneName, const int32_t iVerticalAxis)
{
    if (nullptr == pBoneName || '\0' == pBoneName[0] ||
        iVerticalAxis < -1 || iVerticalAxis > 2)
        return false;

    for (size_t i = 0; i < m_Bones.size(); ++i)
    {
        if (nullptr == m_Bones[i] || !m_Bones[i]->Compare_Name(pBoneName))
            continue;

        float4x4_t rest{};
        XMStoreFloat4x4(&rest, m_Bones[i]->Get_TransformationMatrix());
        m_vRootMotionRestTranslation = { rest._41, rest._42, rest._43 };
        m_vRootMotionUnscaledTranslation = m_vRootMotionRestTranslation;
        m_iRootMotionBoneIndex = static_cast<int32_t>(i);
        m_iRootMotionVerticalAxis = iVerticalAxis;
        m_bAnimationEnvelopeAttempted = false;
        return true;
    }
    return false;
}

void CModel::Restore_UnscaledRootVertical(float4x4_t& Local) const
{
    if (0 == m_iRootMotionVerticalAxis) Local._41 = m_vRootMotionUnscaledTranslation.x;
    if (1 == m_iRootMotionVerticalAxis) Local._42 = m_vRootMotionUnscaledTranslation.y;
    if (2 == m_iRootMotionVerticalAxis) Local._43 = m_vRootMotionUnscaledTranslation.z;
}

void CModel::Apply_RootMotionTranslation(float4x4_t& Local) const
{
    if (0 != m_iRootMotionVerticalAxis) Local._41 = m_vRootMotionRestTranslation.x;
    else if (m_fRootMotionVerticalScale != 1.f)
        Local._41 = m_vRootMotionRestTranslation.x + (Local._41 - m_vRootMotionRestTranslation.x) * m_fRootMotionVerticalScale;
    if (1 != m_iRootMotionVerticalAxis) Local._42 = m_vRootMotionRestTranslation.y;
    else if (m_fRootMotionVerticalScale != 1.f)
        Local._42 = m_vRootMotionRestTranslation.y + (Local._42 - m_vRootMotionRestTranslation.y) * m_fRootMotionVerticalScale;
    if (2 != m_iRootMotionVerticalAxis) Local._43 = m_vRootMotionRestTranslation.z;
    else if (m_fRootMotionVerticalScale != 1.f)
        Local._43 = m_vRootMotionRestTranslation.z + (Local._43 - m_vRootMotionRestTranslation.z) * m_fRootMotionVerticalScale;
}

bool_t CModel::Set_RootMotionVerticalScale(const f32_t fScale)
{
    if (!std::isfinite(fScale) || fScale < 0.f || fScale > 1.f ||
        (fScale != 1.f && (m_iRootMotionBoneIndex < 0 || m_iRootMotionVerticalAxis < 0))) return false;
    if (m_fRootMotionVerticalScale == fScale) return true;
    const f32_t previousScale = m_fRootMotionVerticalScale;
    m_fRootMotionVerticalScale = fScale;
    m_bAnimationEnvelopeAttempted = false;
    if (m_iRootMotionBoneIndex >= 0 && static_cast<size_t>(m_iRootMotionBoneIndex) < m_Bones.size() && m_Bones[m_iRootMotionBoneIndex])
    {
        float4x4_t local{};
        XMStoreFloat4x4(&local, m_Bones[m_iRootMotionBoneIndex]->Get_TransformationMatrix());
        if (previousScale != 1.f) Restore_UnscaledRootVertical(local);
        else m_vRootMotionUnscaledTranslation = {local._41, local._42, local._43};
        Apply_RootMotionTranslation(local);
        m_Bones[m_iRootMotionBoneIndex]->Update_TransformationMatrix(XMLoadFloat4x4(&local));
        Refresh_BoneCombinedMatrices();
    }
    return true;
}

bool_t CModel::Start_Animation(
	const uint32_t iAnimIndex,
	const bool_t isLoop)
{
	if (iAnimIndex >= m_Animations.size())
		return false;
	m_bExplicitAnimationPose = false;
	m_iCurrentAnimIndex = iAnimIndex;
	m_isAnimLoop = isLoop;
	m_isAnimPaused = false;
	m_Animations[iAnimIndex]->Set_TrackPosition(0.f);
	Play_Animation(0.f);
	return true;
}

bool_t CModel::Start_Animation(
	const char_t* pAnimationName,
	const bool_t isLoop)
{
	if (!Set_Animation(pAnimationName, isLoop))
		return false;
	return Start_Animation(m_iCurrentAnimIndex, isLoop);
}

void CModel::Stop_Animation()
{
	m_isAnimPaused = true;
}

void CModel::Set_AnimationSpeed(const f32_t speed)
{
	m_fAnimationSpeed = isfinite(speed)
		? clamp(speed, -16.f, 16.f) : 1.f;
}

bool_t CModel::Update_Animation(const f32_t fTimeDelta)
{
	if (!isfinite(fTimeDelta))
		return false;
	return Play_Animation(fTimeDelta * m_fAnimationSpeed);
}

bool_t CModel::Advance_AnimationClock(const f32_t fTimeDelta, const bool_t applyAnimationSpeed)
{
    if (!std::isfinite(fTimeDelta) || m_bExplicitAnimationPose ||
        m_Animations.empty() || m_iCurrentAnimIndex >= m_Animations.size() ||
        !m_Animations[m_iCurrentAnimIndex]) return false;
    const float delta = m_isAnimPaused ? 0.f : fTimeDelta * (applyAnimationSpeed ? m_fAnimationSpeed : 1.f);
    if (!std::isfinite(delta)) return false;
    const bool_t finished = m_Animations[m_iCurrentAnimIndex]->Advance_Clock(delta, m_isAnimLoop);
    if (m_fBlendElapsed < m_fBlendDuration && m_BlendFromPose.size() == m_Bones.size())
        m_fBlendElapsed += delta;
    return finished;
}

const char_t* CModel::Get_AnimationName(uint32_t iAnimIndex) const
{
    if (iAnimIndex >= m_Animations.size())
        return nullptr;

    return m_Animations[iAnimIndex]->Get_Name();
}

bool_t CModel::Get_AnimationProgress(uint32_t iAnimIndex, f32_t& fOutPosition, f32_t& fOutDuration) const
{
    if (iAnimIndex >= m_Animations.size())
        return false;

    fOutPosition = m_Animations[iAnimIndex]->Get_CurrentTrackPosition();
    fOutDuration = m_Animations[iAnimIndex]->Get_Duration();
    return true;
}

/* 트랙 위치(틱)를 시간으로 환산할 때 쓴다. 유효하지 않으면 0을 돌려주므로
호출부가 자체 기본값을 쓸지 판단할 수 있다. */
f32_t CModel::Get_AnimationTickPerSecond(uint32_t iAnimIndex) const
{
    if (iAnimIndex >= m_Animations.size())
        return 0.f;

    return m_Animations[iAnimIndex]->Get_TickPerSecond();
}

bool_t CModel::Set_AnimTrackPosition(uint32_t iAnimIndex, f32_t fTrackPosition)
{
    if (iAnimIndex >= m_Animations.size())
        return false;

    m_Animations[iAnimIndex]->Set_TrackPosition(fTrackPosition);
    return true;
}

HRESULT CModel::Initialize_Prototype(MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix)
{
    if (nullptr == pModelFilePath)
        return E_FAIL;

    XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);
    m_eType = eType;
	Reset_LocalBounds();
	m_pOrderedStaticGeometrySource.reset();
	m_OrderedStaticGeometry.clear();

    string extension = filesystem::path(pModelFilePath).extension().string();
    transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char value) { return static_cast<char_t>(tolower(value)); });
    if (".wmodel" == extension)
        return Ready_BinaryModel(pModelFilePath);

    uint32_t  iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };

    if (MODEL::NONANIM == eType)
        iFlag |= aiProcess_PreTransformVertices;

    m_pImporter = make_unique<Assimp::Importer>();
    m_pAIScene = m_pImporter->ReadFile(pModelFilePath, iFlag);
    if (nullptr == m_pAIScene)
        return E_FAIL;

    if (FAILED(Ready_Bones(m_pAIScene->mRootNode)))
        return E_FAIL;

    if (FAILED(Ready_Meshes()))
        return E_FAIL;

    if (FAILED(Ready_Materials(pModelFilePath)))
        return E_FAIL;

    if (FAILED(Ready_Animations()))
        return E_FAIL;

    return S_OK;
}

HRESULT CModel::Initialize_Prototype(
	const MODEL eType,
	const MODEL_ASSET_LOAD_DESC& loadDesc,
	fmatrix_t PreTransformMatrix)
{
	if (loadDesc.meshPath.empty())
		return E_INVALIDARG;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);
	m_eType = eType;
	Reset_LocalBounds();
	m_pOrderedStaticGeometrySource.reset();
	m_OrderedStaticGeometry.clear();
	return Ready_BinaryModel(loadDesc);
}

HRESULT CModel::Initialize(void* pArg)
{
    return S_OK;
}

HRESULT CModel::Render(uint32_t iMeshIndex)
{
    if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources()))
        return E_FAIL;

    const HRESULT drawResult = m_Meshes[iMeshIndex]->Render();
    if (FAILED(drawResult))
        return E_FAIL;
    if (S_OK == drawResult)
        if (auto* profiler = CGameInstance::Get().Get_Profiler())
            profiler->Record_ModelSubmitted(this);
    return S_OK;
}

HRESULT CModel::Render_Instanced(uint32_t iMeshIndex,
    ID3D11Buffer* pInstanceBuffer, uint32_t iInstanceStride, uint32_t iNumInstances,
    uint32_t iInstanceByteOffset, const MESH_SCREEN_LOD_DESC* screenLod)
{
    if (iMeshIndex >= m_Meshes.size() ||
        nullptr == m_Meshes[iMeshIndex])
    {
        return E_INVALIDARG;
    }

    const HRESULT drawResult = m_Meshes[iMeshIndex]->Render_Instanced(
        pInstanceBuffer, iInstanceStride, iNumInstances, iInstanceByteOffset, screenLod);
    if (S_OK == drawResult)
        if (auto* profiler = CGameInstance::Get().Get_Profiler())
            profiler->Record_ModelSubmitted(this);
    return drawResult;
}

bool_t CModel::Can_UseOrderedStaticGeometry(
    const uint32_t iFirstMesh, const uint32_t iMeshCount) const
{
    if (MODEL::NONANIM != m_eType || nullptr == m_pOrderedStaticGeometrySource ||
        iMeshCount < 2u || iFirstMesh >= m_Meshes.size() ||
        iMeshCount > m_Meshes.size() - iFirstMesh ||
        m_pOrderedStaticGeometrySource->size() != m_Meshes.size())
        return false;
    for (uint32_t i = iFirstMesh; i < iFirstMesh + iMeshCount; ++i)
    {
        if (nullptr == m_Meshes[i] || m_Meshes[i]->Has_MorphBaseVertices() ||
            (*m_pOrderedStaticGeometrySource)[i].vertexKind != MODEL_VERTEX_KIND::STATIC)
            return false;
    }
    return true;
}

HRESULT CModel::Prepare_OrderedStaticGeometry(
    const uint32_t iFirstMesh, const uint32_t iMeshCount, uint32_t& iOutHandle)
{
    if (iMeshCount == 0u || iFirstMesh >= m_Meshes.size() ||
        iMeshCount > m_Meshes.size() - iFirstMesh)
        return E_INVALIDARG;
    if (!Can_UseOrderedStaticGeometry(iFirstMesh, iMeshCount))
        return S_FALSE;
    for (const auto& Existing : m_OrderedStaticGeometry)
    {
        if (Existing->iFirstMesh == iFirstMesh && Existing->iMeshCount == iMeshCount)
        {
            iOutHandle = Existing->iHandle;
            return S_OK;
        }
    }
    if (m_iNextOrderedStaticGeometryHandle == UINT32_MAX)
        return E_OUTOFMEMORY;
    try
    {
        MODEL_MESH_DATA Combined;
        Combined.name = "ordered-static-geometry";
        Combined.vertexKind = MODEL_VERTEX_KIND::STATIC;
        // Material identity belongs to the preserved ranges, not this carrier.
        Combined.materialIndex = UINT32_MAX;
        Combined.hasColor0 = true;
        uint64_t iVertices = 0u, iIndices = 0u;
        for (uint32_t i = iFirstMesh; i < iFirstMesh + iMeshCount; ++i)
        {
            const auto& Source = (*m_pOrderedStaticGeometrySource)[i];
            if (Source.vertices.empty() || Source.indices.empty() ||
                Source.indices.size() % 3u != 0u ||
                (Source.hasColor0 && Source.color0Rgba8.size() != Source.vertices.size()))
                return E_INVALIDARG;
            iVertices += Source.vertices.size();
            iIndices += Source.indices.size();
        }
        if (iVertices > UINT32_MAX / sizeof(VTXMESH) ||
            iIndices > UINT32_MAX / sizeof(uint32_t))
            return E_INVALIDARG;
        Combined.vertices.reserve(static_cast<size_t>(iVertices));
        Combined.indices.reserve(static_cast<size_t>(iIndices));
        Combined.color0Rgba8.reserve(static_cast<size_t>(iVertices));
        auto Staged = make_shared<ORDERED_STATIC_GEOMETRY>();
        Staged->iHandle = m_iNextOrderedStaticGeometryHandle;
        Staged->iFirstMesh = iFirstMesh;
        Staged->iMeshCount = iMeshCount;
        Staged->Ranges.reserve(iMeshCount);
        for (uint32_t i = iFirstMesh; i < iFirstMesh + iMeshCount; ++i)
        {
            const auto& Source = (*m_pOrderedStaticGeometrySource)[i];
            const uint32_t iBaseVertex = static_cast<uint32_t>(Combined.vertices.size());
            Staged->Ranges.push_back({ i, Source.materialIndex, iBaseVertex,
                static_cast<uint32_t>(Source.vertices.size()),
                static_cast<uint32_t>(Combined.indices.size()),
                static_cast<uint32_t>(Source.indices.size()) });
            Combined.vertices.insert(Combined.vertices.end(),
                Source.vertices.begin(), Source.vertices.end());
            for (size_t v = 0u; v < Source.vertices.size(); ++v)
                Combined.color0Rgba8.push_back(
                    Source.hasColor0 ? Source.color0Rgba8[v] : 0xffffffffu);
            for (const uint32_t iIndex : Source.indices)
            {
                if (iIndex >= Source.vertices.size())
                    return E_INVALIDARG;
                Combined.indices.push_back(iBaseVertex + iIndex);
            }
            Combined.hasTexcoord1 |= Source.hasTexcoord1;
            Combined.hasTexcoord2 |= Source.hasTexcoord2;
        }
        // Reuse the exact existing CMesh static conversion and pretransform.
        // No readback, disk decode, or alternate model runtime is introduced.
        Staged->pMesh = shared_ptr<CMesh>(new CMesh(m_pDevice, m_pContext));
        const MODEL_SKELETON_DATA NoSkeleton{};
        const HRESULT Result = Staged->pMesh->Initialize_Prototype(
            MODEL::NONANIM, Combined, NoSkeleton, XMLoadFloat4x4(&m_PreTransformMatrix));
        if (FAILED(Result))
            return Result;
        m_OrderedStaticGeometry.push_back(Staged);
        ++m_iNextOrderedStaticGeometryHandle;
        iOutHandle = Staged->iHandle;
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::length_error&) { return E_OUTOFMEMORY; }
}

bool_t CModel::Get_OrderedStaticGeometryRanges(const uint32_t iHandle,
    std::span<const ORDERED_STATIC_GEOMETRY_RANGE>& OutRanges) const
{
    for (const auto& Geometry : m_OrderedStaticGeometry)
    {
        if (Geometry->iHandle != iHandle)
            continue;
        if (!Can_UseOrderedStaticGeometry(Geometry->iFirstMesh, Geometry->iMeshCount))
            return false;
        OutRanges = Geometry->Ranges;
        return true;
    }
    return false;
}

HRESULT CModel::Render_OrderedStaticGeometryInstanced(const uint32_t iHandle,
    ID3D11Buffer* pInstanceBuffer, const uint32_t iInstanceStride,
    const uint32_t iNumInstances, const uint32_t iInstanceByteOffset)
{
    for (const auto& Geometry : m_OrderedStaticGeometry)
    {
        if (Geometry->iHandle != iHandle)
            continue;
        if (!Can_UseOrderedStaticGeometry(Geometry->iFirstMesh, Geometry->iMeshCount))
            return S_FALSE;
        const HRESULT drawResult = Geometry->pMesh->Render_Instanced(
            pInstanceBuffer, iInstanceStride, iNumInstances, iInstanceByteOffset);
        if (S_OK == drawResult)
            if (auto* profiler = CGameInstance::Get().Get_Profiler())
                profiler->Record_ModelSubmitted(this);
        return drawResult;
    }
    return E_INVALIDARG;
}

void CModel::Enable_AnimationSampleReuse()
{
    for (const auto& animation : m_Animations)
        if (animation) animation->Enable_SampleReuse();
}

bool_t CModel::Play_Animation(f32_t fTimeDelta)
{
    Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Play");
    if (m_bExplicitAnimationPose && m_isAnimPaused) return false;
    if (m_Animations.empty() || m_iCurrentAnimIndex >= m_Animations.size())
        return false;

    Engine::CProfilerModelAnimationScope modelAnimationScope(CGameInstance::Get().Get_Profiler(), this);
    // Default scale leaves external local-pose edits untouched. Only a scaled
    // pose needs restoration before an unkeyed channel or blend consumes it.
    if (m_fRootMotionVerticalScale != 1.f && m_iRootMotionBoneIndex >= 0 &&
        static_cast<size_t>(m_iRootMotionBoneIndex) < m_Bones.size() && m_Bones[m_iRootMotionBoneIndex])
    {
        float4x4_t local{};
        XMStoreFloat4x4(&local, m_Bones[m_iRootMotionBoneIndex]->Get_TransformationMatrix());
        Restore_UnscaledRootVertical(local);
        m_Bones[m_iRootMotionBoneIndex]->Update_TransformationMatrix(XMLoadFloat4x4(&local));
    }
    bool_t      isFinished = { false };
    /* 내가 로드한 애니메이션 중,
    현재 취해야하는 애니메이션의 포즈뼈들의 m_TransformationMatrix를 갱신해준다. */
    /* 일시정지 중에는 재생 위치를 전진시키지 않되, 뼈 행렬은 그대로 다시 계산해
    현재 프레임의 포즈를 유지한다. */
    isFinished = m_Animations[m_iCurrentAnimIndex]->Update_TransformationMatrix(
        m_isAnimPaused ? 0.f : fTimeDelta, m_Bones, m_isAnimLoop);

    Update_AnimBlend(m_isAnimPaused ? 0.f : fTimeDelta);

    if (m_iRootMotionBoneIndex >= 0 &&
        static_cast<size_t>(m_iRootMotionBoneIndex) < m_Bones.size())
    {
        const shared_ptr<CBone>& pRoot = m_Bones[m_iRootMotionBoneIndex];
        if (nullptr != pRoot)
        {
            float4x4_t local{};
            XMStoreFloat4x4(&local, pRoot->Get_TransformationMatrix());
            m_vRootMotionUnscaledTranslation = {local._41, local._42, local._43};
            Apply_RootMotionTranslation(local);
            pRoot->Update_TransformationMatrix(XMLoadFloat4x4(&local));
        }
    }

    /* 뼈들 자체 행렬은 갱신이 됐지만, 최종행렬은 아직 미완성(m_Transformation * Parent`s CombinedTransfor4mationMatrix). */
    Refresh_BoneCombinedMatrices();

    return isFinished;
}

bool_t CModel::Capture_BoneMatrices(const uint32_t iMeshIndex, vector<float4x4_t>& outMatrices) const
{
    if (!Is_Skinned() || iMeshIndex >= m_Meshes.size() || !m_Meshes[iMeshIndex])
        return false;
    const CMesh& mesh = *m_Meshes[iMeshIndex];
    if (mesh.m_iNumBones == 0u || mesh.m_iNumBones > 512u)
        return false;
    vector<float4x4_t> staged(mesh.m_iNumBones);
    mesh.Build_SkinPalette(m_Bones, staged.data());
    if (!std::all_of(staged.begin(), staged.end(), Is_FiniteMatrix))
        return false;
    outMatrices = std::move(staged);
    return true;
}

HRESULT CModel::Bind_BoneMatrices(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex)
{
    if (nullptr == pShader || iMeshIndex >= m_Meshes.size() || !m_Meshes[iMeshIndex])
        return E_INVALIDARG;

    const CMesh& mesh = *m_Meshes[iMeshIndex];
    // WModel meshes share the full skeleton palette; Assimp meshes retain
    // their own bone subsets and inverse-bind offsets in slots 1..N.
    const size_t paletteIndex = mesh.m_bUsesSkeletonPalette ? 0u : iMeshIndex + 1u;
    if (m_SkinPalettes.size() <= paletteIndex)
        m_SkinPalettes.resize(paletteIndex + 1u);
    SKIN_PALETTE& palette = m_SkinPalettes[paletteIndex];
    if (palette.iPoseRevision != m_iBonePoseRevision)
    {
        Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Animation.SkinPalette.Build");
        palette.Matrices.resize(mesh.m_iNumBones);
        mesh.Build_SkinPalette(m_Bones, palette.Matrices.data());
        palette.iPoseRevision = m_iBonePoseRevision;
    }

    Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Animation.SkinPalette.Bind");
    return pShader->Bind_Matrices(pConstantName, palette.Matrices.data(), mesh.m_iNumBones);
}

HRESULT CModel::Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType, uint32_t iTextureIndex)
{
    uint32_t        iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();

    if (iMaterialIndex >= m_iNumMaterials)
        return E_FAIL;

    return m_Materials[iMaterialIndex]->Bind_Material(pShader, pConstantName, eType, iTextureIndex);
}

uint32_t CModel::Override_MaterialTexture(
    const char_t* pMaterialNameFragment,
    const aiTextureType eType,
    const uint32_t iTextureIndex,
    ComPtr<ID3D11ShaderResourceView> pTexture)
{
    if (nullptr == pMaterialNameFragment || 0 == pMaterialNameFragment[0])
        return 0u;

    /* Case-insensitive: the source materials are named inconsistently across rigs and the
       caller should not have to know which spelling a given class shipped. */
    string fragment = pMaterialNameFragment;
    transform(fragment.begin(), fragment.end(), fragment.begin(),
        [](unsigned char c) { return static_cast<char_t>(tolower(c)); });

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial)
            continue;
        string name = pMaterial->Get_Name();
        transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        if (name.find(fragment) == string::npos)
            continue;
        pMaterial->Set_TextureOverride(eType, iTextureIndex, pTexture);
        ++matched;
    }
    return matched;
}

uint32_t CModel::Override_MaterialDyeColor(
    const char_t* pMaterialNameFragment,
    const float4_t& vDiffuse,
    const float4_t& vRegionA)
{
    if (nullptr == pMaterialNameFragment || 0 == pMaterialNameFragment[0])
        return 0u;

    string fragment = pMaterialNameFragment;
    transform(fragment.begin(), fragment.end(), fragment.begin(),
        [](unsigned char c) { return static_cast<char_t>(tolower(c)); });

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial || !pMaterial->Has_DyeColor())
            continue;
        string name = pMaterial->Get_Name();
        transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        if (name.find(fragment) == string::npos)
            continue;
        pMaterial->Set_DyeColorOverride(vDiffuse, vRegionA);
        ++matched;
    }
    return matched;
}

namespace
{
    /* Every Override_* below matches a material the same way: case-insensitively, on a
    fragment of its name, so a caller never has to know a class rig's material order. */
    bool_t MaterialNameContains(const string& name, const string& fragment)
    {
        string lowered = name;
        transform(lowered.begin(), lowered.end(), lowered.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        return lowered.find(fragment) != string::npos;
    }

    string LoweredFragment(const char_t* pMaterialNameFragment)
    {
        string fragment = nullptr != pMaterialNameFragment ? pMaterialNameFragment : "";
        transform(fragment.begin(), fragment.end(), fragment.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        return fragment;
    }
}

uint32_t CModel::Override_SourceCharacterConstants(
    const char_t* pMaterialNameFragment,
    const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters)
{
    const string fragment = LoweredFragment(pMaterialNameFragment);
    if (fragment.empty())
        return 0u;

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial || !pMaterial->Has_SourceCharacterProgram() ||
            !MaterialNameContains(pMaterial->Get_Name(), fragment))
            continue;
        if (NativeHairUsesExtraUV(parameters) && m_pMaterialSource)
        {
            const size_t materialIndex = static_cast<size_t>(&pMaterial - m_Materials.data());
            if (any_of(m_pMaterialSource->meshes.begin(), m_pMaterialSource->meshes.end(),
                [&](const auto& mesh) { return mesh.materialIndex == materialIndex && !mesh.hasTexcoord1; }))
                continue;
        }
        if (pMaterial.use_count() > 1) pMaterial = pMaterial->Clone_ForOverrides();
        if (pMaterial->Set_SourceCharacterConstants(parameters))
            ++matched;
    }
    return matched;
}

uint32_t CModel::Override_SourceCharacterTexture(
    const char_t* pMaterialNameFragment, const uint32_t iRegister,
    ComPtr<ID3D11ShaderResourceView> pTexture)
{
    const string fragment = LoweredFragment(pMaterialNameFragment);
    if (fragment.empty())
        return 0u;

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial || !pMaterial->Has_SourceCharacterProgram() ||
            !MaterialNameContains(pMaterial->Get_Name(), fragment))
            continue;
        if (pMaterial.use_count() > 1) pMaterial = pMaterial->Clone_ForOverrides();
        if (pMaterial->Set_SourceCharacterTextureOverride(iRegister, pTexture))
            ++matched;
    }
    return matched;
}

void CModel::Clear_SourceCharacterOverrides()
{
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr != pMaterial && pMaterial->Has_SourceCharacterProgram())
        {
            if (pMaterial.use_count() > 1) pMaterial = pMaterial->Clone_ForOverrides();
            pMaterial->Clear_SourceCharacterOverrides();
        }
    }
}

uint32_t CModel::Override_MaterialDiffuseTint(
    const char_t* pMaterialNameFragment, const float4_t& vTint)
{
    if (nullptr == pMaterialNameFragment || 0 == pMaterialNameFragment[0])
        return 0u;

    string fragment = pMaterialNameFragment;
    transform(fragment.begin(), fragment.end(), fragment.begin(),
        [](unsigned char c) { return static_cast<char_t>(tolower(c)); });

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial)
            continue;
        string name = pMaterial->Get_Name();
        transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        if (name.find(fragment) == string::npos)
            continue;
        pMaterial->Set_DiffuseTint(vTint);
        ++matched;
    }
    return matched;
}

const float4_t* CModel::Get_MaterialDiffuseTint(const uint32_t iMeshIndex) const
{
    if (iMeshIndex >= m_iNumMeshes)
        return nullptr;
    const uint32_t iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
    if (iMaterialIndex >= m_iNumMaterials || nullptr == m_Materials[iMaterialIndex])
        return nullptr;
    return &m_Materials[iMaterialIndex]->Get_DiffuseTint();
}

uint32_t CModel::Override_MaterialDyeTwoTone(
    const char_t* pMaterialNameFragment, const f32_t fStrength, const f32_t fRange)
{
    if (nullptr == pMaterialNameFragment || 0 == pMaterialNameFragment[0])
        return 0u;

    string fragment = pMaterialNameFragment;
    transform(fragment.begin(), fragment.end(), fragment.begin(),
        [](unsigned char c) { return static_cast<char_t>(tolower(c)); });

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial || !pMaterial->Has_DyeColor())
            continue;
        string name = pMaterial->Get_Name();
        transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        if (name.find(fragment) == string::npos)
            continue;
        pMaterial->Set_DyeTwoTone(fStrength, fRange);
        ++matched;
    }
    return matched;
}

void CModel::Clear_MaterialDyeColorOverrides()
{
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr != pMaterial)
            pMaterial->Clear_DyeColorOverride();
    }
}

void CModel::Clear_MaterialTextureOverrides()
{
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr != pMaterial)
            pMaterial->Clear_TextureOverrides();
    }
}

bool_t CModel::Has_MaterialTexture(uint32_t iMeshIndex,
    aiTextureType eType,
    uint32_t iTextureIndex) const
{
    if (iMeshIndex >= m_Meshes.size())
        return false;

    const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
    return materialIndex < m_Materials.size() &&
        m_Materials[materialIndex]->Has_Texture(eType, iTextureIndex);
}

const MODEL_COLOR_TINT* CModel::Get_MaterialColorTint(
    uint32_t iMeshIndex) const
{
    if (iMeshIndex >= m_Meshes.size())
        return nullptr;

    const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size())
        return nullptr;

    return &m_Materials[materialIndex]->Get_ColorTint();
}

HRESULT CModel::Bind_SourceCharacter(shared_ptr<CShader> shader, uint32_t meshIndex)
{
    if (meshIndex >= m_Meshes.size()) return E_INVALIDARG;
    const uint32_t materialIndex = m_Meshes[meshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size() || !m_Materials[materialIndex]) return E_INVALIDARG;
    return m_Materials[materialIndex]->Bind_SourceCharacter(shader);
}

HRESULT CModel::Bind_SourceCharacterForwardLight(shared_ptr<CShader> shader, uint32_t meshIndex)
{
    if (meshIndex >= m_Meshes.size()) return E_INVALIDARG;
    const uint32_t materialIndex = m_Meshes[meshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size() || !m_Materials[materialIndex]) return E_INVALIDARG;
    return m_Materials[materialIndex]->Bind_SourceCharacterForwardLight(shader);
}

HRESULT CModel::Bind_SourceLandscapeSurface(shared_ptr<CShader> shader, uint32_t meshIndex)
{
    if (!shader || meshIndex >= m_Meshes.size()) return E_INVALIDARG;
    const uint32_t materialIndex = m_Meshes[meshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size() || !m_Materials[materialIndex]) return E_INVALIDARG;
    return m_Materials[materialIndex]->Bind_SourceLandscapeSurface(shader);
}

HRESULT CModel::Bind_SourceSpecialSurface(shared_ptr<CShader> shader, uint32_t meshIndex)
{
    if (meshIndex >= m_Meshes.size()) return E_INVALIDARG;
    const uint32_t materialIndex = m_Meshes[meshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size()) return E_INVALIDARG;
    return m_Materials[materialIndex]->Bind_SourceSpecialSurface(shader);
}

HRESULT CModel::Bind_SurfaceLighting(shared_ptr<CShader> shader, uint32_t meshIndex)
{
    if (meshIndex >= m_Meshes.size()) return E_INVALIDARG;
    const uint32_t materialIndex = m_Meshes[meshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size()) return E_INVALIDARG;
    return m_Materials[materialIndex]->Bind_SurfaceLighting(shader);
}

const MODEL_SURFACE_PARAMETERS* CModel::Get_MaterialSurface(uint32_t iMeshIndex) const
{
	if (iMeshIndex >= m_Meshes.size())
		return nullptr;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex < m_Materials.size() ?
		&m_Materials[materialIndex]->Get_Surface() : nullptr;
}

bool_t CModel::Has_MaterialTextureOverrides(uint32_t iMeshIndex) const
{
	if (iMeshIndex >= m_Meshes.size() || !m_Meshes[iMeshIndex])
		return true;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex >= m_Materials.size() || !m_Materials[materialIndex] ||
		m_Materials[materialIndex]->Has_TextureOverrides();
}

HRESULT CModel::Bind_SurfaceTexture(shared_ptr<CShader> pShader,
	const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_INVALIDARG;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex < m_Materials.size() ?
		m_Materials[materialIndex]->Bind_SurfaceTexture(pShader, pConstantName, eType) : E_FAIL;
}

bool_t CModel::Try_GetSourceMaterialIndex(
    const uint32_t iMeshIndex, uint32_t& iOutMaterialIndex) const
{
    if (iMeshIndex >= m_Meshes.size() || nullptr == m_Meshes[iMeshIndex])
        return false;
    const uint32_t iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
    if (iMaterialIndex >= m_Materials.size())
        return false;
    iOutMaterialIndex = iMaterialIndex;
    return true;
}

const string& CModel::Get_MaterialName(uint32_t iMeshIndex) const
{
	static const string Empty;
	if (iMeshIndex >= m_Meshes.size())
		return Empty;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex < m_Materials.size() ?
		m_Materials[materialIndex]->Get_Name() : Empty;
}

uint64_t CModel::Get_MaterialNameHash(uint32_t iMeshIndex) const
{
	if (iMeshIndex >= m_Meshes.size())
		return 0u;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex < m_Materials.size() ?
		m_Materials[materialIndex]->Get_NameHash() : 0u;
}

uint32_t CModel::Get_MeshVertexCount(uint32_t iMeshIndex) const
{
	if (iMeshIndex >= m_Meshes.size())
		return 0u;
	return m_Meshes[iMeshIndex]->Get_NumVertices();
}

bool_t CModel::Has_StaticMeshLod(uint32_t iMeshIndex) const
{
	return iMeshIndex < m_Meshes.size() && m_Meshes[iMeshIndex] &&
		m_Meshes[iMeshIndex]->m_StaticLod && !m_Meshes[iMeshIndex]->Has_MorphBaseVertices();
}

bool_t CModel::Has_MorphBaseVertices(uint32_t iMeshIndex) const
{
	if (iMeshIndex >= m_Meshes.size())
		return false;
	return m_Meshes[iMeshIndex]->Has_MorphBaseVertices();
}

bool_t CModel::Get_MorphBaseVertex(uint32_t iMeshIndex, uint32_t iVertexIndex,
	float3_t& OutPosition, float3_t& OutNormal) const
{
	if (iMeshIndex >= m_Meshes.size())
		return false;
	return m_Meshes[iMeshIndex]->Get_MorphBaseVertex(iVertexIndex, OutPosition, OutNormal);
}

HRESULT CModel::Make_MeshVertexBuffer_Unique(uint32_t iMeshIndex)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_INVALIDARG;
	return m_Meshes[iMeshIndex]->Make_VertexBuffer_Unique();
}

HRESULT CModel::Update_Mesh_Vertices(uint32_t iMeshIndex, const vector<uint32_t>& iVertexIndices,
	const vector<float3_t>& Positions, const vector<float3_t>& Normals)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_INVALIDARG;
	return m_Meshes[iMeshIndex]->Update_Vertices(iVertexIndices, Positions, Normals);
}

HRESULT CModel::Reset_Mesh_Vertices(uint32_t iMeshIndex)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_INVALIDARG;
	return m_Meshes[iMeshIndex]->Reset_Vertices();
}

HRESULT CModel::Ready_Meshes()
{
    m_iNumMeshes = m_pAIScene->mNumMeshes;

    for (size_t i = 0; i < m_iNumMeshes; i++)
    {
		if (MODEL::NONANIM == m_eType)
		{
			const aiMesh* pAIMesh = m_pAIScene->mMeshes[i];
			for (uint32_t vertexIndex = 0; vertexIndex < pAIMesh->mNumVertices; ++vertexIndex)
			{
				float3_t position{};
				memcpy(&position, &pAIMesh->mVertices[vertexIndex], sizeof(float3_t));
				Include_LocalPosition(XMVector3TransformCoord(
					XMLoadFloat3(&position), XMLoadFloat4x4(&m_PreTransformMatrix)));
			}
		}
        else if (MODEL::ANIM == m_eType)
        {
            const aiMesh* pAIMesh = m_pAIScene->mMeshes[i];
            for (uint32_t vertexIndex = 0; vertexIndex < pAIMesh->mNumVertices; ++vertexIndex)
            {
                float3_t position{};
                memcpy(&position, &pAIMesh->mVertices[vertexIndex], sizeof(float3_t));
                Include_BindGeometryPosition(XMVector3TransformCoord(
                    XMLoadFloat3(&position), XMLoadFloat4x4(&m_PreTransformMatrix)));
            }
        }

        auto pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType, m_pAIScene->mMeshes[i], m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
        if (nullptr == pMesh)
            return E_FAIL;

        m_Meshes.push_back(pMesh);
    }

    return S_OK;
}

HRESULT CModel::Ready_Materials(const char_t* pModelFilePath)
{
    m_iNumMaterials = m_pAIScene->mNumMaterials;

    for (uint32_t i = 0; i < m_iNumMaterials; i++)
    {
        auto        pMaterial = CMaterial::Create(m_pDevice, m_pContext, m_pAIScene->mMaterials[i], pModelFilePath);
        if (nullptr == pMaterial)
            return E_FAIL;

        m_Materials.push_back(pMaterial);
    }

    return S_OK;
}

HRESULT CModel::Ready_Bones(const aiNode* pAINode, int32_t iParentBoneIndex)
{
    auto        pBone = CBone::Create(pAINode, iParentBoneIndex);
    if (nullptr == pBone)
        return E_FAIL;

    float4x4_t RestLocal{};
    XMStoreFloat4x4(&RestLocal, pBone->Get_TransformationMatrix());
    m_Bones.push_back(pBone);
    m_BoneRestLocalTransforms.push_back(RestLocal);

    int32_t     iParentIndex = m_Bones.size() - 1;

    for (uint32_t i = 0; i < pAINode->mNumChildren; ++i)
    {
        Ready_Bones(pAINode->mChildren[i], iParentIndex);
    }

    return S_OK;
}

HRESULT CModel::Ready_Animations()
{
    m_iNumAnimations = m_pAIScene->mNumAnimations;

    for (uint32_t i = 0; i < m_iNumAnimations; i++)
    {
        auto      pAnimation = CAnimation::Create(m_pAIScene->mAnimations[i], m_Bones);
        if (nullptr == pAnimation)
            return E_FAIL;

        m_Animations.push_back(pAnimation);
    }

    return S_OK;
}

HRESULT CModel::Ready_BinaryModel(const char_t* pModelFilePath)
{
    MODEL_ASSET_LOAD_DESC desc{};
    desc.meshPath = filesystem::path(pModelFilePath).lexically_normal();

    filesystem::path absolutePath = filesystem::absolute(desc.meshPath).lexically_normal();
    for (filesystem::path current = absolutePath.parent_path();
        !current.empty(); current = current.parent_path())
    {
        if (L"Resources" == current.filename())
        {
            desc.assetRoot = current;
            break;
        }
        if (current == current.root_path())
            break;
    }

    return Ready_BinaryModel(desc);
}

HRESULT CModel::Apply_MaterialOverrides(MODEL_MATERIAL_SOURCE& materialSource, const MODEL_ASSET_LOAD_DESC& loadDesc)
{
	/* Join before allocating meshes/materials. A malformed replacement never
	   changes a shared prototype or an already admitted material instance. */
	vector<string> overriddenNames;
	for (const MODEL_MATERIAL_OVERRIDE& replacement : loadDesc.materialOverrides)
	{
		const auto failOverride = [&](const char* reason)
		{
			OutputDebugStringA(("[CModel] Material override rejected: " +
				loadDesc.meshPath.string() + " / " + replacement.materialName +
				" / " + reason + "\n").c_str());
			return E_INVALIDARG;
		};
        if (replacement.surface.renderMode > MODEL_SURFACE_RENDER_MODE::WATER ||
            replacement.surface.cullMode > MODEL_SURFACE_CULL_MODE::TWO_SIDED)
            return failOverride("invalid material render or cull mode");
        if (!replacement.surface.environmentLegacyEnabled && !replacement.surface.hasSourceIndirect)
            return failOverride("disabled legacy environment requires source indirect inputs");
        if (replacement.surface.hasSourceIndirect)
        {
            const auto& source = replacement.surface;
            if ((source.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
                 source.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE) ||
                !source.hasEnvironmentCube || replacement.environmentCubePath.empty() ||
                replacement.environmentBRDFPath.empty() || replacement.sourceIndirectCubePath.empty() ||
                replacement.sourceIndirectBRDFPath.empty())
                return failOverride("source indirect requires PBR environment inputs");
            for (const float value : { source.sourceIndirectColor.x, source.sourceIndirectColor.y,
                source.sourceIndirectColor.z, source.sourceIndirectColor.w })
                if (!std::isfinite(value) || value < 0.f)
                    return failOverride("invalid source indirect environment color");
            const auto& rotation = source.sourceIndirectRotation;
            if (!std::isfinite(rotation.x) || !std::isfinite(rotation.y) ||
                std::abs(rotation.x) > 1.f || std::abs(rotation.y) > 1.f ||
                std::abs(rotation.x*rotation.x+rotation.y*rotation.y-1.f) > 0.0001f)
                return failOverride("invalid source indirect rotation");
            for (const auto& row : source.sourceIndirectSH)
                for (const float value : { row.x, row.y, row.z, row.w })
                    if (!std::isfinite(value) || std::abs(value) > 64.f)
                        return failOverride("invalid source indirect SH component");
            if (source.sourceIndirectSH[6].w != 1.f)
                return failOverride("source indirect SH reserved w must be one");
            const float nonnegative[] = { source.sourceUpperSkyColor.x, source.sourceUpperSkyColor.y,
                source.sourceUpperSkyColor.z, source.sourceLowerSkyColor.x, source.sourceLowerSkyColor.y,
                source.sourceLowerSkyColor.z, source.sourceAmbientAndSkyFactor.x,
                source.sourceAmbientAndSkyFactor.y, source.sourceAmbientAndSkyFactor.z };
            if (any_of(begin(nonnegative), end(nonnegative), [](float value) {
                    return !std::isfinite(value) || value < 0.f || value > 64.f;
                }) || !std::isfinite(source.sourceAmbientAndSkyFactor.w) ||
                source.sourceAmbientAndSkyFactor.w < 0.f || source.sourceAmbientAndSkyFactor.w > 4.f)
                return failOverride("invalid source indirect sky or ambient factor");
        }
		if (replacement.materialName.empty() ||
			find(overriddenNames.begin(), overriddenNames.end(), replacement.materialName) != overriddenNames.end())
			return failOverride("empty or duplicate material name");
        if (replacement.surface.hasStaticShadow)
        {
            const auto& transfer = replacement.surface.staticShadowTransfer;
            const auto root = loadDesc.assetRoot.lexically_normal();
            const auto& path = replacement.staticShadowPath;
            const auto relative = path.lexically_normal().lexically_relative(root);
            if (!replacement.surface.hasBakedLighting || replacement.surface.staticShadowChannel < 1u || replacement.surface.staticShadowChannel > 15u || !root.is_absolute() || !path.is_absolute() ||
                relative.empty() || relative.is_absolute() ||
                any_of(relative.begin(), relative.end(), [](const filesystem::path& part) { return part == ".."; }) ||
                !std::isfinite(transfer.x) || !std::isfinite(transfer.y) || !std::isfinite(transfer.z) ||
                transfer.y < 1.f || transfer.z <= 0.f || transfer.z > 128.f)
                return failOverride("invalid source static shadow inputs");
        }
        if (replacement.surface.sourceFoliageWind)
        {
            const auto& surface = replacement.surface;
            const bool nativeMap = surface.family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
                surface.sourceCharacter.program >= 1100u && surface.sourceCharacter.program <= 1166u;
            if (!nativeMap && surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
                surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED) return failOverride("unsupported wind family");
            const auto finite = [](const float4_t& v) { return std::isfinite(v.x) && std::isfinite(v.y) &&
                std::isfinite(v.z) && std::isfinite(v.w) && std::abs(v.x) <= 1e8f && std::abs(v.y) <= 1e8f &&
                std::abs(v.z) <= 1e8f && std::abs(v.w) <= 1e8f; };
            const auto& b = surface.sourceFoliageWindLocalBounds;
            const auto& w = surface.sourceFoliageWindDirectionSpeed;
            if ((surface.sourceFoliageWindProgram == 1u && !nativeMap && surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED) ||
                !finite(b) || !finite(w) || !finite(surface.sourceFoliageWindLocalCenter) ||
                !finite(surface.sourceFoliageWindActorPosition) || !finite(surface.sourceFoliageWindPlayerPosition) ||
                surface.sourceFoliageWindLocalCenter.w != 1.f || surface.sourceFoliageWindActorPosition.w != 0.f ||
                b.x <= 0.f || b.y <= 0.f || b.z <= 0.f || b.w <= 0.f || b.x > 1e5f || b.y > 1e5f || b.z > 1e5f || b.w > 1e5f ||
                w.x != 0.f || w.y != 0.f || w.z != 1.f || w.w != 0.f ||
                !surface.Has_ValidSourceFoliageWindProgramInputs() ||
                any_of(begin(surface.sourceFoliageWindScalars), end(surface.sourceFoliageWindScalars), [&](const float4_t& v) { return !finite(v); }))
                return failOverride("invalid source foliage wind inputs");
        }
        if (replacement.surface.family == MODEL_SURFACE_FAMILY::SOURCE_LANDSCAPE_OPAQUE)
        {
            const auto& surface = replacement.surface;
            const auto& source = surface.sourceLandscape;
            if (!source.Has_ValidInputs() || surface.hasBakedLighting || surface.hasStaticShadow ||
                surface.hasEnvironmentCube || surface.hasSourceIndirect || surface.hasEmissive ||
                (surface.renderMode != MODEL_SURFACE_RENDER_MODE::INHERIT && surface.renderMode != MODEL_SURFACE_RENDER_MODE::DEFERRED))
                return failOverride("invalid source landscape parameters");
            const auto root = loadDesc.assetRoot.lexically_normal();
            const auto validPath = [&](bool required, const filesystem::path& path) {
                if (!required) return path.empty();
                const auto relative = path.lexically_normal().lexically_relative(root);
                return root.is_absolute() && path.is_absolute() && path.extension() == L".dds" &&
                    !relative.empty() && !relative.is_absolute() &&
                    none_of(relative.begin(), relative.end(), [](const filesystem::path& part) { return part == ".."; });
            };
            const auto& textures = replacement.sourceLandscapeTextures;
            for (uint32_t i = 0u; i < SOURCE_LANDSCAPE_LAYER_COUNT; ++i)
                if (!validPath((source.layerMask & (1u << i)) != 0u, textures.diffuse[i]) ||
                    !validPath((source.normalMask & (1u << i)) != 0u, textures.normal[i]))
                    return failOverride("source landscape layer texture mismatch or escape");
            for (uint32_t i = 0u; i < SOURCE_LANDSCAPE_WEIGHTMAP_COUNT; ++i)
                if (!validPath(i < source.weightmapCount, textures.weightmaps[i])) return failOverride("source landscape weight texture mismatch or escape");
            if (!validPath(true, textures.heightmap)) return failOverride("source landscape height texture missing or escape");
            size_t matches = 0u;
            for (size_t materialIndex = 0u; materialIndex < materialSource.materials.size(); ++materialIndex)
            {
                auto& material = materialSource.materials[materialIndex];
                if (material.name != replacement.materialName) continue;
                if (any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                    return mesh.materialIndex == materialIndex && mesh.vertexKind != MODEL_VERTEX_KIND::STATIC;
                })) return failOverride("source landscape requires a static painted grid");
                material.surface = surface;
                material.sourceLandscapeTextures = textures;
                ++matches;
            }
            if (matches != 1u) return failOverride("source landscape material name is absent or ambiguous");
            overriddenNames.push_back(replacement.materialName);
            continue;
        }
        if (replacement.surface.family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
        {
            const auto& source = replacement.surface.sourceCharacter;
            const uint32_t mask = source.baseTextureMask | source.lightTextureMask;
            // These native programs consume constants only in both source passes.
            const bool textureless = source.program == 64u || source.program == 65u ||
                source.program == 1510u || source.program == 1512u;
            if (!Is_SourceCharacterProgramSupported(source.program) ||
                (mask == 0u && !textureless) ||
                (textureless && mask != 0u) || source.requiredExtraUVMask > 3u ||
                (mask >> SOURCE_CHARACTER_TEXTURE_COUNT) != 0u ||
                (replacement.surface.hasBakedLighting && !(source.program == 209u || source.program == 210u || (source.program >= 214u && source.program <= 234u) || source.program == 237u || (source.program >= 1100u && source.program <= 1166u) || (source.program >= 1400u && source.program <= 1413u) || (source.program >= 80u && source.program <= 83u) ||
                    (source.program >= 40u && source.program <= 63u && source.program != 47u && source.program != 53u && source.program != 55u))) ||
                replacement.surface.hasEnvironmentCube)
                return failOverride("invalid source character program or texture mask");
            for (const auto* constants : { &source.baseConstants, &source.lightConstants })
                for (const auto& value : *constants)
                    for (const float scalar : { value.x, value.y, value.z, value.w })
                        if (!std::isfinite(scalar) || std::abs(scalar) > 1000000.f)
                            return failOverride("invalid source character parameter");
            const auto root = loadDesc.assetRoot.lexically_normal();
            if (!root.is_absolute()) return failOverride("source character root is not absolute");
            for (uint32_t index = 0u; index < SOURCE_CHARACTER_TEXTURE_COUNT; ++index)
            {
                const auto& path = replacement.sourceCharacterTextures[index].path;
                if ((mask & (1u << index)) == 0u)
                {
                    if (!path.empty()) return failOverride("unused source character texture");
                    continue;
                }
                const auto relative = path.lexically_normal().lexically_relative(root);
                if (!path.is_absolute() || relative.empty() || relative.is_absolute() ||
                    any_of(relative.begin(), relative.end(), [](const filesystem::path& part) { return part == ".."; }))
                    return failOverride("source character texture escapes the resource root");
            }
            if (replacement.surface.hasBakedLighting)
            {
                for (const auto& path : { replacement.bakedAveragePath, replacement.bakedDirectionalPath })
                {
                    const auto relative = path.lexically_normal().lexically_relative(root);
                    if (!path.is_absolute() || relative.empty() || relative.is_absolute() ||
                        any_of(relative.begin(), relative.end(), [](const filesystem::path& part) { return part == ".."; }))
                        return failOverride("source map monster lighting texture escapes the resource root");
                }
            }
            size_t matches = 0u;
            for (auto& material : materialSource.materials)
            {
                if (material.name != replacement.materialName) continue;
                const size_t materialIndex = static_cast<size_t>(&material - materialSource.materials.data());
                if (any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                    return mesh.materialIndex == materialIndex &&
                        (((source.requiredExtraUVMask & 1u) != 0u && !mesh.hasTexcoord1) ||
                         ((source.requiredExtraUVMask & 2u) != 0u && !mesh.hasTexcoord2));
                })) return failOverride("source material requires preserved extra UV channels");
                if ((source.program == 5u || NativeHairUsesExtraUV(source) ||
                    source.program == 18u || source.program == 19u) &&
                    any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                        return mesh.materialIndex == materialIndex &&
                            (!mesh.hasTexcoord1 || (source.program == 5u && !mesh.hasTexcoord2));
                    }))
                    return failOverride("source character requires native extra UV channels");
                if (replacement.surface.hasBakedLighting &&
                    any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                        return mesh.materialIndex == materialIndex && (!mesh.hasTexcoord1 || mesh.vertexKind != MODEL_VERTEX_KIND::STATIC);
                    })) return failOverride("source map lightmap requires native static UV1");
                // Some multipart weapons repeat one MIC in several material
                // slots. An exact source name intentionally replaces all of them.
                material.surface = replacement.surface;
                material.sourceCharacterTextures = replacement.sourceCharacterTextures;
                material.bakedAveragePath = replacement.bakedAveragePath;
                material.bakedDirectionalPath = replacement.bakedDirectionalPath;
                material.staticShadowPath = replacement.staticShadowPath;
                ++matches;
            }
            if (matches == 0u) return failOverride("source character material name is absent");
            overriddenNames.push_back(replacement.materialName);
            continue;
        }
        size_t matches = 0u;
        for (auto match = materialSource.materials.begin(); match != materialSource.materials.end(); ++match)
        {
        if (match->name != replacement.materialName) continue;
        ++matches;
		const auto& surface = replacement.surface;
        const bool sourceSpecial = surface.family >= MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE &&
            surface.family <= MODEL_SURFACE_FAMILY::SOURCE_WET_OPAQUE;
		if (replacement.hasDiffuseAddressU)
		{
			if (surface.family != MODEL_SURFACE_FAMILY::LEGACY || match->diffusePath.empty())
				return failOverride("diffuse sampler requires an existing legacy diffuse input");
			match->diffuseMirrorU = replacement.diffuseMirrorU;
			continue;
		}
		if (surface.family != MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION &&
			surface.family != MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION &&
			surface.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE &&
			surface.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
			surface.family != MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial)
			return failOverride("unsupported surface family");
		const f32_t scalars[] = { surface.diffuseBrightness,
            surface.family == MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ? std::abs(surface.normalIntensity) : surface.normalIntensity,
			surface.specularIntensity, surface.specularPower, surface.reflectionIntensity,
			surface.reflectionContrast, surface.reflectionTiling, surface.diffuseSaturation,
			surface.diffuseColor.x, surface.diffuseColor.y, surface.diffuseColor.z, surface.diffuseColor.w,
			surface.specularColor.x, surface.specularColor.y, surface.specularColor.z, surface.specularColor.w,
			surface.reflectionColor.x, surface.reflectionColor.y, surface.reflectionColor.z, surface.reflectionColor.w };
		if (any_of(begin(scalars), end(scalars), [](f32_t value) { return !std::isfinite(value) || value < 0.f; }) ||
			(surface.specularPower < 1.f && surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE && surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED && surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
                surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial) || surface.reflectionTiling <= 0.f)
			return failOverride("invalid surface value");
		const auto root = loadDesc.assetRoot.lexically_normal();
		const auto reflection = replacement.reflectionPath.lexically_normal();
		const auto relative = reflection.lexically_relative(root);
        if (surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial &&
            (!root.is_absolute() || !reflection.is_absolute() || relative.empty() ||
             relative.is_absolute() || any_of(relative.begin(), relative.end(),
                [](const filesystem::path& part) { return part == ".."; })))
			return failOverride("reflection escapes the resource root");
		if (surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial &&
            surface.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
            surface.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE &&
            (match->diffusePath.empty() || match->normalPath.empty() ||
			(surface.family == MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION && match->specularPath.empty())))
			return failOverride("required material input is absent");
		if (surface.family == MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE ||
			surface.family == MODEL_SURFACE_FAMILY::PBR_OPAQUE)
		{
			const f32_t pbr[] = { surface.uvTiling.x, surface.uvTiling.y,
				surface.detailNormalIntensity, surface.detailNormalTiling,
				surface.metallicIntensity, surface.metallicPower, surface.roughnessIntensity,
				surface.roughnessPower, surface.aoIntensity, surface.aoPower,
				surface.specularPBRIntensity, surface.nonmetallicBrightness, surface.metallicBrightness,
				surface.minimumRoughness, surface.vertexAlpha };
			if (any_of(begin(pbr), end(pbr), [](f32_t v) { return !std::isfinite(v) || v < 0.f; }) ||
				surface.uvTiling.x <= 0.f || surface.uvTiling.y <= 0.f || surface.detailNormalTiling <= 0.f ||
				surface.minimumRoughness <= 0.f || surface.minimumRoughness > 1.f || surface.vertexAlpha > 1.f ||
				!std::isfinite(surface.reflectionOriginOffset.x) || !std::isfinite(surface.reflectionOriginOffset.y))
				return failOverride("invalid PBR value");
			const filesystem::path inputs[] = { replacement.surfaceDiffusePath, replacement.surfaceNormalPath,
				replacement.detailNormalPath, replacement.surfaceORMPath };
			for (const auto& path : inputs)
			{
				const auto rel = path.lexically_normal().lexically_relative(root);
				if (!path.is_absolute() || rel.empty() || rel.is_absolute() ||
					any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
					return failOverride("PBR texture escapes the resource root");
			}
			match->surfaceDiffusePath = replacement.surfaceDiffusePath;
			match->surfaceNormalPath = replacement.surfaceNormalPath;
			match->detailNormalPath = replacement.detailNormalPath;
			match->surfaceORMPath = replacement.surfaceORMPath;
		}
        if (surface.family == MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED ||
            surface.family == MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED)
        {
            const uint32_t flags = surface.sourceFoliageFlags;
            const auto& transmission = surface.sourceFoliageTransmission;
            const float values[] = { transmission.x, transmission.y, transmission.z, transmission.w };
            if ((flags & ~127u) != 0u || ((flags & 8u) && !(flags & 4u)) ||
                ((flags & 64u) && !(flags & 32u)) || surface.hasEnvironmentCube ||
                surface.hasEmissive != ((flags & 32u) != 0u) ||
                (surface.family == MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && (flags & (1u | 8u))) ||
                any_of(begin(values), end(values), [](float v) { return !std::isfinite(v) || v < 0.f; }))
                return failOverride("invalid source foliage branch or parameter");
            const std::pair<const filesystem::path*, filesystem::path*> inputs[] = {
                { &replacement.surfaceDiffusePath, &match->surfaceDiffusePath },
                { &replacement.surfaceNormalPath, &match->surfaceNormalPath },
                { &replacement.surfaceSpecularPath, &match->surfaceSpecularPath },
                { &replacement.sourceFoliageMaskPath, &match->sourceFoliageMaskPath }
            };
            const bool required[] = { true, (flags & 1u) != 0u, (flags & 8u) != 0u,
                surface.family == MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED };
            for (size_t i = 0; i < size(inputs); ++i)
            {
                const auto& path = *inputs[i].first;
                if (path.empty()) { if (required[i]) return failOverride("source foliage selected input missing"); continue; }
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!root.is_absolute() || !path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source foliage texture escapes the resource root");
                *inputs[i].second = path;
            }
        }
        if (sourceSpecial)
        {
            const auto& special = surface.sourceSpecial;
            const bool ice = surface.family == MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE;
            const bool blend = surface.family == MODEL_SURFACE_FAMILY::SOURCE_VERTEXBLEND_OPAQUE;
            if ((special.flags & ~3u) || (!ice && !blend && special.flags) || surface.hasEnvironmentCube || surface.hasEmissive)
                return failOverride("invalid source special branch");
            for (const auto& value : { special.iceCoreColor, special.iceOuterColor, special.iceBlend,
                special.wetParameters, surface.sourceBgRimlight })
                for (float v : { value.x, value.y, value.z, value.w })
                    if (!std::isfinite(v) || v < 0.f) return failOverride("invalid source special vector");
            for (const auto* layers : { &special.blendDiffuse, &special.blendSpecular, &special.blendLayers })
                for (const auto& value : *layers)
                    for (float v : { value.x, value.y, value.z, value.w })
                        if (!std::isfinite(v) || v < 0.f) return failOverride("invalid source blend layer");
            for (float v : { special.normalTiling, special.wetSpecularPower, special.blendSharpness,
                surface.detailNormalIntensity, surface.detailNormalTiling, surface.uvTiling.x, surface.uvTiling.y })
                if (!std::isfinite(v) || v < 0.f) return failOverride("invalid source special scalar");
            if (!std::isfinite(special.iceBumpOffset) || special.normalTiling <= 0.f ||
                surface.uvTiling.x <= 0.f || surface.uvTiling.y <= 0.f || surface.detailNormalTiling <= 0.f)
                return failOverride("invalid source special UV or parallax");
            if (blend) for (size_t i = 0; i < 4; ++i)
                if ((i < 2 || (special.flags & (1u << (i - 2)))) &&
                    (special.blendLayers[i].x <= 0.f || special.blendLayers[i].y <= 0.f))
                    return failOverride("invalid source blend UV");
            const std::pair<const filesystem::path*, filesystem::path*> inputs[] = {
                { &replacement.surfaceDiffusePath, &match->surfaceDiffusePath },
                { &replacement.surfaceNormalPath, &match->surfaceNormalPath },
                { &replacement.surfaceSpecularPath, &match->surfaceSpecularPath },
                { &replacement.reflectionPath, &match->reflectionPath },
                { &replacement.detailNormalPath, &match->detailNormalPath },
                { &replacement.overlayDiffusePath, &match->overlayDiffusePath },
                { &replacement.overlayNormalPath, &match->overlayNormalPath },
                { &replacement.sourceSpecialMaskPath, &match->sourceSpecialMaskPath },
                { &replacement.sourceBlendDiffuseGPath, &match->sourceBlendDiffuseGPath },
                { &replacement.sourceBlendNormalGPath, &match->sourceBlendNormalGPath },
                { &replacement.sourceBlendDiffuseBPath, &match->sourceBlendDiffuseBPath },
                { &replacement.sourceBlendNormalBPath, &match->sourceBlendNormalBPath }
            };
            const bool required[] = { true, true, !blend && (!ice || (special.flags & 2u)), !blend,
                blend || (ice && (special.flags & 1u)), blend, blend, ice,
                blend && (special.flags & 1u), blend && (special.flags & 1u),
                blend && (special.flags & 2u), blend && (special.flags & 2u) };
            for (size_t i = 0; i < size(inputs); ++i)
            {
                const auto& path = *inputs[i].first;
                if (path.empty()) { if (required[i]) return failOverride("source special input missing"); continue; }
                if (!required[i]) return failOverride("source special inactive input supplied");
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!root.is_absolute() || !path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source special texture escapes the resource root");
                *inputs[i].second = path;
            }
        }
        if (surface.family == MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED)
        {
            const uint32_t flags = surface.sourceBgFlags;
            const float viewValues[] = { surface.sourceBgSubspecular.x, surface.sourceBgSubspecular.y,
                surface.sourceBgRimlight.x, surface.sourceBgRimlight.y, surface.sourceBgRimlight.z,
                surface.sourceBgRimlight.w, surface.sourceBgSpecularSaturation };
            if (any_of(begin(viewValues), end(viewValues), [](float v) { return !std::isfinite(v) || v < 0.f; }) ||
                !std::isfinite(surface.sourceBgPanning.x) || !std::isfinite(surface.sourceBgPanning.y))
                return failOverride("invalid source BG view lighting or panning");
            const float values[] = { surface.sourceBgBump.x, surface.sourceBgBump.y,
                surface.sourceBgBump.z, surface.sourceBgUV.x, surface.sourceBgUV.y,
                surface.sourceBgUV.z, surface.sourceBgUV.w, surface.uvTiling.x, surface.uvTiling.y,
                surface.reflectionOriginOffset.x, surface.reflectionOriginOffset.y };
            if ((flags & ~65535u) != 0u || ((flags & 8u) != 0u && (flags & 4u) == 0u &&
                surface.sourceBgSubspecular.x <= 0.f) ||
                ((flags & 32u) != 0u && (flags & 16u) == 0u) || surface.sourceBgFlicker > 2u ||
                surface.hasEnvironmentCube ||
                any_of(begin(values), end(values), [](float v) { return !std::isfinite(v); }) ||
                std::abs(surface.sourceBgUV.x * surface.sourceBgUV.x +
                    surface.sourceBgUV.y * surface.sourceBgUV.y - 1.f) > 0.0001f)
                return failOverride("invalid source BG branch or parameter");
            const std::pair<const filesystem::path*, filesystem::path*> inputs[] = {
                { &replacement.surfaceDiffusePath, &match->surfaceDiffusePath },
                { &replacement.surfaceNormalPath, &match->surfaceNormalPath },
                { &replacement.surfaceSpecularPath, &match->surfaceSpecularPath },
                { &replacement.reflectionPath, &match->reflectionPath }
            };
            const bool required[] = { true, (flags & 1u) != 0u, (flags & 8u) != 0u, (flags & 16u) != 0u };
            for (size_t i = 0; i < size(inputs); ++i)
            {
                const auto& path = *inputs[i].first;
                if (path.empty()) { if (required[i]) return failOverride("source BG selected input missing"); continue; }
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!root.is_absolute() || !path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source BG texture escapes the resource root");
                *inputs[i].second = path;
            }
            if ((flags & 32768u) != 0u)
            {
                const auto& path = replacement.detailNormalPath;
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }) ||
                    !std::isfinite(surface.detailNormalIntensity) || !std::isfinite(surface.detailNormalTiling))
                    return failOverride("invalid source detail normal");
                match->detailNormalPath = path;
            }
            // Sampler identity accompanies the source diffuse, including the old explicit mirror rows.
            match->diffuseMirrorU = replacement.diffuseMirrorU;
        }
        if (surface.family == MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE)
        {
            if (!std::isfinite(surface.uvTiling.x) || !std::isfinite(surface.uvTiling.y) ||
                surface.uvTiling.x <= 0.f || surface.uvTiling.y <= 0.f ||
                !std::isfinite(surface.reflectionOriginOffset.x) || !std::isfinite(surface.reflectionOriginOffset.y) ||
                surface.hasEnvironmentCube)
                return failOverride("invalid source specular UV or unsupported environment");
            const std::pair<const filesystem::path*, filesystem::path*> inputs[] = {
                { &replacement.surfaceDiffusePath, &match->surfaceDiffusePath },
                { &replacement.surfaceNormalPath, &match->surfaceNormalPath },
                { &replacement.surfaceSpecularPath, &match->surfaceSpecularPath }
            };
            for (const auto& input : inputs)
            {
                const auto& path = *input.first;
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source specular texture escapes the resource root");
                *input.second = path;
            }
        }
        if (surface.family == MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE)
        {
            const uint32_t materialIndex = static_cast<uint32_t>(distance(materialSource.materials.begin(), match));
            if (any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                return mesh.materialIndex == materialIndex &&
                    (mesh.vertexKind != MODEL_VERTEX_KIND::STATIC || !mesh.hasTangentHandedness);
            })) return failOverride("source overlay requires preserved static geometry and tangent handedness");
            const float values[] = { surface.overlayColor.x, surface.overlayColor.y, surface.overlayColor.z,
                surface.overlayColor.w, surface.overlayTiling, surface.overlayNormalIntensity,
                surface.overlaySharpness, surface.overlayBrightness, surface.overlaySaturation,
                surface.overlaySpecularIntensity, surface.uvTiling.x, surface.uvTiling.y, surface.detailNormalIntensity, surface.detailNormalTiling,
                surface.sourceBgSubspecular.x, surface.sourceBgSubspecular.y, surface.sourceBgSpecularSaturation };
            if (any_of(begin(values), end(values), [](float v) { return !std::isfinite(v) || v < 0.f; }) ||
                surface.sourceOverlayFlags > 2047u || surface.overlayTiling <= 0.f || surface.uvTiling.x <= 0.f || surface.uvTiling.y <= 0.f || surface.hasEnvironmentCube ||
                !replacement.reflectionPath.empty()) return failOverride("invalid source overlay surface");
            const float signedValues[] = { surface.sourceOverlayDirection.x, surface.sourceOverlayDirection.y,
                surface.sourceOverlayDirection.z, surface.sourceOverlayDirection.w,
                surface.sourceBgUV.x, surface.sourceBgUV.y, surface.sourceBgUV.z, surface.sourceBgUV.w,
                surface.sourceBgBump.x, surface.sourceBgBump.y, surface.sourceBgBump.z, surface.sourceBgBump.w };
            if (any_of(begin(signedValues), end(signedValues), [](float v) { return !std::isfinite(v); }) ||
                (((surface.sourceOverlayFlags & 32u) != 0u) != !replacement.detailNormalPath.empty()))
                return failOverride("invalid source overlay direction, UV or detail branch");
            if (surface.hasEmissive && (surface.emissiveFlickerMinimum != 0.f ||
                surface.emissiveFlickerSpeed != 0.f || surface.emissivePhaseOffset != 0.f))
                return failOverride("unsupported source overlay emissive flicker");
            if (surface.overlaySeparateSpecular)
            {
                const auto path = replacement.surfaceSpecularPath.lexically_normal();
                const auto rel = path.lexically_relative(root);
                if (!root.is_absolute() || !path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source overlay specular escapes the resource root");
                match->surfaceSpecularPath = path;
            }
            const std::pair<const filesystem::path*, filesystem::path*> inputs[] = {
                { &replacement.surfaceDiffusePath, &match->surfaceDiffusePath },
                { &replacement.surfaceNormalPath, &match->surfaceNormalPath },
                { &replacement.overlayDiffusePath, &match->overlayDiffusePath },
                { &replacement.overlayNormalPath, &match->overlayNormalPath },
                { &replacement.detailNormalPath, &match->detailNormalPath }
            };
            for (const auto& input : inputs)
            {
                const auto& path = *input.first;
                if (path.empty() && ((input.first == &replacement.surfaceNormalPath && (surface.sourceOverlayFlags & 1u) == 0u) ||
                    (input.first == &replacement.overlayNormalPath && (surface.sourceOverlayFlags & 2u) == 0u) ||
                    (input.first == &replacement.detailNormalPath && (surface.sourceOverlayFlags & 32u) == 0u))) continue;
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!root.is_absolute() || !path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source overlay texture escapes the resource root");
                *input.second = path;
            }
        }
        if (surface.hasBakedLighting)
        {
            const uint32_t materialIndex = static_cast<uint32_t>(distance(materialSource.materials.begin(), match));
            if (any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                return mesh.materialIndex == materialIndex && (!mesh.hasTexcoord1 || mesh.vertexKind != MODEL_VERTEX_KIND::STATIC);
            })) return failOverride("baked lighting requires preserved static TEXCOORD1");
        }
        const float environmentValues[] = { surface.environmentColor.x, surface.environmentColor.y,
            surface.environmentColor.z, surface.environmentColor.w, surface.environmentRotation.x, surface.environmentRotation.y };
        if (surface.hasEnvironmentCube &&
            (any_of(begin(environmentValues), end(environmentValues), [](float v) { return !std::isfinite(v); }) ||
             surface.environmentColor.x < 0.f || surface.environmentColor.y < 0.f || surface.environmentColor.z < 0.f ||
             std::abs(surface.environmentRotation.x*surface.environmentRotation.x + surface.environmentRotation.y*surface.environmentRotation.y-1.f) > 0.0001f))
            return failOverride("invalid environment color or rotation");
        const std::pair<const filesystem::path*, filesystem::path*> lightingPaths[] = {
            { &replacement.bakedAveragePath, &match->bakedAveragePath },
            { &replacement.bakedDirectionalPath, &match->bakedDirectionalPath },
            { &replacement.staticShadowPath, &match->staticShadowPath },
            { &replacement.environmentCubePath, &match->environmentCubePath },
            { &replacement.environmentBRDFPath, &match->environmentBRDFPath },
            { &replacement.sourceIndirectCubePath, &match->sourceIndirectCubePath },
            { &replacement.sourceIndirectBRDFPath, &match->sourceIndirectBRDFPath }
        };
        if ((surface.hasBakedLighting && (replacement.bakedAveragePath.empty() || replacement.bakedDirectionalPath.empty())) ||
            (surface.hasEnvironmentCube && (replacement.environmentCubePath.empty() || replacement.environmentBRDFPath.empty())))
            return failOverride("missing lighting texture");
        for (const auto& path : lightingPaths)
        {
            if (path.first->empty()) continue;
            const auto relative = path.first->lexically_normal().lexically_relative(root);
            if (!path.first->is_absolute() || relative.empty() || relative.is_absolute() ||
                any_of(relative.begin(), relative.end(), [](const filesystem::path& p) { return p == ".."; }))
                return failOverride("lighting texture escapes the resource root");
            *path.second = *path.first;
        }
		if (surface.hasEmissive)
		{
			const f32_t emissionValues[] = { surface.emissiveColor.x, surface.emissiveColor.y,
				surface.emissiveColor.z, surface.emissiveColor.w, surface.emissiveIntensity,
				surface.emissiveUVTiling.x, surface.emissiveUVTiling.y,
				surface.emissiveFlickerMinimum, surface.emissiveFlickerSpeed };
			if ((surface.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE &&
				surface.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
                surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
                surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial) ||
				any_of(begin(emissionValues), end(emissionValues),
					[](f32_t value) { return !std::isfinite(value) || value < 0.f; }) ||
				surface.emissiveUVTiling.x <= 0.f || surface.emissiveUVTiling.y <= 0.f ||
				(surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && surface.emissiveFlickerMinimum > 1.f) || !std::isfinite(surface.emissivePhaseOffset))
				return failOverride("invalid PBR emissive value");
			const auto emissive = replacement.surfaceEmissivePath.lexically_normal();
			const auto emissiveRelative = emissive.lexically_relative(root);
			if (!emissive.is_absolute() || emissiveRelative.empty() || emissiveRelative.is_absolute() ||
				any_of(emissiveRelative.begin(), emissiveRelative.end(),
					[](const filesystem::path& part) { return part == ".."; }))
				return failOverride("emissive texture escapes the resource root");
			match->surfaceEmissivePath = emissive;
		}
		match->surface = surface;
		match->reflectionPath = reflection;
        }
        if (matches == 0u) return failOverride("material name is absent");
		overriddenNames.push_back(replacement.materialName);
	}
	return S_OK;
}

HRESULT CModel::Ready_BinaryModel(
	const MODEL_ASSET_LOAD_DESC& loadDesc)
{
    Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Binary");
	MODEL_ASSET_DATA asset{};
	bool decoded = false;
	{
		Engine::CProfilerScope decodeScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Decode");
		decoded = CModelDecoderRegistry::Get().Decode(loadDesc, asset);
	}
	if (!decoded)
	{
		const MODEL_DECODE_REPORT report = CModelDecoderRegistry::Get().Get_LastReport();
		OutputDebugStringA(("[CModel] Binary decode failed for " +
			loadDesc.meshPath.string() + ": " + report.error + "\n").c_str());
		return E_FAIL;
	}
	if (asset.meshes.empty() ||
		((MODEL::ANIM == m_eType) != asset.hasSkeleton))
	{
		OutputDebugStringA(("[CModel] Binary decode produced an unusable asset for " +
			loadDesc.meshPath.string() + " (meshes=" +
			std::to_string(asset.meshes.size()) + ", hasSkeleton=" +
			(asset.hasSkeleton ? "true" : "false") + ").\n").c_str());
		return E_FAIL;
	}
    auto materialSource = std::make_shared<MODEL_MATERIAL_SOURCE>();
    materialSource->identity = loadDesc;
    materialSource->identity.materialOverrides.clear();
    materialSource->materials = asset.materials;
    materialSource->meshes.reserve(asset.meshes.size());
    for (const auto& mesh : asset.meshes)
        materialSource->meshes.push_back({ mesh.materialIndex, mesh.vertexKind,
            mesh.hasColor0, mesh.hasTexcoord1, mesh.hasTexcoord2, !mesh.tangentHandedness.empty() });
    // Keep only small material rows/channel facts, never decoded vertex/index arrays.
    m_pMaterialSource = materialSource;
    MODEL_MATERIAL_SOURCE staged = *materialSource;
    if (FAILED(Apply_MaterialOverrides(staged, loadDesc))) return E_INVALIDARG;
    asset.materials = std::move(staged.materials);
	m_iSkeletonHash = asset.hasSkeleton ? asset.skeleton.skeletonHash : 0;

	m_bHasSelfConsistentUnauthenticatedGeometryMetadata =
		asset.geometryMetadata.present;
	m_iGeometryFormatVersionMajor = {};
	m_iGeometryFormatVersionMinor = {};
	m_iGeometryChannelMask = {};
	m_iGeometryEvidenceFlags = {};
	m_fGeometryPreScale = 1.f;
	m_GeometryPayloadSha256.fill(0);
	m_GeometryMetadataIdentitySha256.fill(0);
	if (m_bHasSelfConsistentUnauthenticatedGeometryMetadata)
	{
		m_iGeometryFormatVersionMajor = asset.geometryMetadata.versionMajor;
		m_iGeometryFormatVersionMinor = asset.geometryMetadata.versionMinor;
		m_iGeometryChannelMask = asset.geometryMetadata.channelMask;
		m_iGeometryEvidenceFlags = asset.geometryMetadata.evidenceFlags;
		m_fGeometryPreScale = asset.geometryMetadata.geometryPreScale;
		m_GeometryPayloadSha256 = asset.geometryMetadata.payloadSha256;
		m_GeometryMetadataIdentitySha256 =
			asset.geometryMetadata.metadataIdentitySha256;
	}

	if (FAILED(Ready_Bones(asset)) ||
		FAILED(Ready_Meshes(asset)) ||
		FAILED(Ready_Materials(asset)) ||
		FAILED(Ready_Animations(asset)))
	{
		return E_FAIL;
	}

	Refresh_BoneCombinedMatrices();

	if (!asset.animations.empty())
	{
		uint32_t animationIndex = {};
		if (!loadDesc.defaultAnimationName.empty())
		{
			const auto iterator = find_if(
				asset.animations.begin(), asset.animations.end(),
				[&loadDesc](const MODEL_ANIMATION_DATA& animation)
				{
					return animation.name ==
						loadDesc.defaultAnimationName;
				});
			if (iterator == asset.animations.end())
				return E_FAIL;
			animationIndex = static_cast<uint32_t>(distance(
				asset.animations.begin(), iterator));
		}
		if (!Start_Animation(
			animationIndex,
			asset.animations[animationIndex].defaultLoop))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CModel::Ready_Meshes(const MODEL_ASSET_DATA& asset)
{
    Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Meshes");
    m_iNumMeshes = static_cast<uint32_t>(asset.meshes.size());
    m_Meshes.reserve(m_iNumMeshes);
	for (const MODEL_MESH_DATA& mesh : asset.meshes)
	{
		if (MODEL::NONANIM == m_eType)
		{
			/* Runtime culling bounds are derived from every decoded vertex. The
			   embedded AABB is validated import metadata, but it must not be the
			   sole authority for a placement disappearing at the camera edge. */
			for (const VTXMESH& vertex : mesh.vertices)
			{
				Include_LocalPosition(XMVector3TransformCoord(
					XMLoadFloat3(&vertex.vPosition),
					XMLoadFloat4x4(&m_PreTransformMatrix)));
			}
		}
        else if (MODEL::ANIM == m_eType)
        {
            for (const VTXANIMMESH& vertex : mesh.skinnedVertices)
                Include_BindGeometryPosition(XMVector3TransformCoord(
                    XMLoadFloat3(&vertex.vPosition), XMLoadFloat4x4(&m_PreTransformMatrix)));
        }

        auto pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType,
            mesh, asset.skeleton, XMLoadFloat4x4(&m_PreTransformMatrix));
        if (nullptr == pMesh)
            return E_FAIL;
        // Only undeformed native opaque map geometry opts into additional LOD
        // indices. Other models retain their original buffer and render contract.
        if (MODEL::NONANIM == m_eType && mesh.materialIndex < asset.materials.size())
        {
            const auto& material = asset.materials[mesh.materialIndex];
            const auto& surface = material.surface;
            if (surface.family == MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
                (surface.sourceBgFlags & 64u) == 0u && material.opacityPath.empty() &&
                (surface.renderMode == MODEL_SURFACE_RENDER_MODE::INHERIT ||
                 surface.renderMode == MODEL_SURFACE_RENDER_MODE::DEFERRED))
            {
                const HRESULT lodResult = pMesh->Prepare_StaticLod(mesh, XMLoadFloat4x4(&m_PreTransformMatrix));
                if (FAILED(lodResult))
                {
                    char message[192]{};
                    sprintf_s(message, "[Engine][MeshLOD] optional index LOD unavailable hr=0x%08X; keeping original mesh\n", static_cast<uint32_t>(lodResult));
                    OutputDebugStringA(message);
                }
            }
        }
        m_Meshes.push_back(pMesh);
    }
    // Keep source geometry only for explicitly opted-in multi-submesh effects.
    // Clones share it; ordinary map/character models pay no retained CPU cost.
    if (m_bRetainOrderedStaticGeometry && MODEL::NONANIM == m_eType &&
        asset.meshes.size() > 1u)
    {
        try
        {
            m_pOrderedStaticGeometrySource =
                make_shared<const vector<MODEL_MESH_DATA>>(asset.meshes);
        }
        catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
        catch (const std::length_error&) { return E_OUTOFMEMORY; }
    }
    return S_OK;
}

void CModel::Reset_LocalBounds()
{
	const f32_t maximum = (numeric_limits<f32_t>::max)();
	m_vLocalBoundsMin = float3_t(maximum, maximum, maximum);
	m_vLocalBoundsMax = float3_t(-maximum, -maximum, -maximum);
	m_bHasLocalBounds = false;
    m_bHasBindGeometryBounds = false;
    m_vBindGeometryBoundsMin = float3_t(maximum, maximum, maximum);
    m_vBindGeometryBoundsMax = float3_t(-maximum, -maximum, -maximum);
}

void CModel::Include_LocalPosition(fvector_t vPosition)
{
	float3_t position{};
	XMStoreFloat3(&position, vPosition);
	if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z))
		return;

	m_vLocalBoundsMin.x = (min)(m_vLocalBoundsMin.x, position.x);
	m_vLocalBoundsMin.y = (min)(m_vLocalBoundsMin.y, position.y);
	m_vLocalBoundsMin.z = (min)(m_vLocalBoundsMin.z, position.z);
	m_vLocalBoundsMax.x = (max)(m_vLocalBoundsMax.x, position.x);
	m_vLocalBoundsMax.y = (max)(m_vLocalBoundsMax.y, position.y);
	m_vLocalBoundsMax.z = (max)(m_vLocalBoundsMax.z, position.z);
	m_bHasLocalBounds = true;
}

bool_t CModel::Try_GetBindGeometryBounds(float3_t& minimum, float3_t& maximum) const
{
    if (MODEL::NONANIM == m_eType && m_bHasLocalBounds)
    { minimum = m_vLocalBoundsMin; maximum = m_vLocalBoundsMax; return true; }
    if (!m_bHasBindGeometryBounds) return false;
    minimum = m_vBindGeometryBoundsMin; maximum = m_vBindGeometryBoundsMax; return true;
}

namespace
{
    bool MakeModelPickRay(const float4x4_t& world, const float3_t& rayOrigin,
        const float3_t& rayDirection, vector_t& origin, vector_t& direction,
        float& localLength, float& windingSign)
    {
        const auto finite = [](vector_t value) { return !XMVector3IsNaN(value) && !XMVector3IsInfinite(value); };
        const vector_t ray = XMLoadFloat3(&rayDirection);
        const float rayLength = XMVectorGetX(XMVector3Length(ray));
        if (!finite(ray) || !finite(XMLoadFloat3(&rayOrigin)) ||
            !std::isfinite(rayLength) || rayLength <= 1.e-8f) return false;
        for (const auto& row : world.m) for (const float value : row)
            if (!std::isfinite(value)) return false;
        // A world transform must be affine. Reject a projective matrix rather
        // than treating a perspective ray as a line after the inverse.
        if (world._14 != 0.f || world._24 != 0.f || world._34 != 0.f || world._44 != 1.f) return false;
        vector_t determinant;
        const matrix_t inverse = XMMatrixInverse(&determinant, XMLoadFloat4x4(&world));
        const float det = XMVectorGetX(determinant);
        if (!std::isfinite(det) || det == 0.f) return false;
        origin = XMVector3TransformCoord(XMLoadFloat3(&rayOrigin), inverse);
        const vector_t localRay = XMVector3TransformNormal(ray / rayLength, inverse);
        localLength = XMVectorGetX(XMVector3Length(localRay));
        if (!finite(origin) || !finite(localRay) || !std::isfinite(localLength) || localLength <= 0.f) return false;
        direction = localRay / localLength;
        windingSign = det < 0.f ? -1.f : 1.f;
        return true;
    }
}

bool_t CModel::Try_PickStaticSurface(const uint32_t meshIndex, const float4x4_t& world,
    const float3_t& rayOrigin, const float3_t& rayDirection, const f32_t maxDistance,
    const PICK_CULL_MODE cullMode, f32_t& distance) const
{
    if (MODEL::NONANIM != m_eType || meshIndex >= m_Meshes.size() ||
        !m_Meshes[meshIndex] || !m_bHasLocalBounds || !std::isfinite(maxDistance) || maxDistance < 0.f ||
        (cullMode != PICK_CULL_MODE::NONE && cullMode != PICK_CULL_MODE::BACK &&
            cullMode != PICK_CULL_MODE::FRONT)) return false;
    vector_t origin, direction;
    float localLength = 0.f, windingSign = 1.f;
    if (!MakeModelPickRay(world, rayOrigin, rayDirection, origin, direction, localLength, windingSign)) return false;
    const float localLimit = static_cast<float>((std::min)(
        static_cast<double>(maxDistance) * localLength,
        static_cast<double>((std::numeric_limits<float>::max)())));
    BoundingBox bounds;
    BoundingBox::CreateFromPoints(bounds, XMLoadFloat3(&m_vLocalBoundsMin), XMLoadFloat3(&m_vLocalBoundsMax));
    float entry = 0.f;
    if (!bounds.Intersects(origin, direction, entry) || entry > localLimit) return false;
    const float cullSign = cullMode == PICK_CULL_MODE::NONE ? 0.f :
        windingSign * (cullMode == PICK_CULL_MODE::BACK ? 1.f : -1.f);
    float localDistance = 0.f;
    if (!m_Meshes[meshIndex]->Try_PickStaticLocal(origin, direction, localLimit, cullSign, localDistance)) return false;
    const float worldDistance = localDistance / localLength;
    if (!std::isfinite(worldDistance) || worldDistance > maxDistance) return false;
    distance = worldDistance;
    return true;
}

bool_t CModel::Try_PickCurrentPose(const float4x4_t& world, const float3_t& rayOrigin,
    const float3_t& rayDirection, f32_t& distance) const
{
    uint32_t meshIndex = 0u;
    return Try_PickCurrentPose(world, rayOrigin, rayDirection, distance, meshIndex);
}

bool_t CModel::Try_PickCurrentPose(const float4x4_t& world, const float3_t& rayOrigin,
    const float3_t& rayDirection, f32_t& distance, uint32_t& meshIndex) const
{
    const auto finite = [](vector_t value) { return !XMVector3IsNaN(value) && !XMVector3IsInfinite(value); };
    vector_t origin, direction;
    float localLength = 0.f, windingSign = 1.f;
    if (!MakeModelPickRay(world, rayOrigin, rayDirection, origin, direction, localLength, windingSign)) return false;
    float3_t minimum, maximum;
    if (!Try_GetCurrentPoseBounds(minimum, maximum)) return false;
    BoundingBox bounds;
    BoundingBox::CreateFromPoints(bounds, XMLoadFloat3(&minimum), XMLoadFloat3(&maximum));
    float boundDistance;
    if (!bounds.Intersects(origin, direction, boundDistance)) return false;
    float closest = (std::numeric_limits<float>::max)();
    bool hit = false;
    uint32_t closestMesh = 0u;
    for (uint32_t meshOrdinal = 0u; meshOrdinal < m_Meshes.size(); ++meshOrdinal)
    {
        const auto& mesh = m_Meshes[meshOrdinal];
        if (!mesh || mesh->m_hasUniqueVertexBuffer || !mesh->m_PickGeometry) continue;
        const auto& geometry = *mesh->m_PickGeometry;
        if (!geometry.skinned)
        {
            float candidate;
            if (mesh->Try_PickStaticLocal(origin, direction, closest, 0.f, candidate) && candidate < closest)
            { closest = candidate; closestMesh = meshOrdinal; hit = true; }
            continue;
        }
        vector<float4x4_t> palette;
        if (geometry.skinned)
        {
            if (mesh->m_iNumBones == 0u || mesh->m_iNumBones > 512u) continue;
            palette.resize(mesh->m_iNumBones);
            mesh->Build_SkinPalette(m_Bones, palette.data());
        }
        vector<float3_t> positions(geometry.vertices.size());
        bool valid = true;
        for (size_t index = 0; index < geometry.vertices.size(); ++index)
        {
            const auto& vertex = geometry.vertices[index];
            vector_t position = XMLoadFloat3(&vertex.position);
            if (geometry.skinned)
            {
                const uint32_t bones[] = {vertex.bones.x, vertex.bones.y, vertex.bones.z, vertex.bones.w};
                const float weights[] = {vertex.weights.x, vertex.weights.y, vertex.weights.z, vertex.weights.w};
                position = XMVectorZero();
                for (size_t lane = 0; lane < 4u; ++lane)
                {
                    if (bones[lane] >= palette.size()) { valid = false; break; }
                    position += XMVector3TransformCoord(XMLoadFloat3(&vertex.position),
                        XMLoadFloat4x4(&palette[bones[lane]])) * weights[lane];
                }
            }
            if (!valid || !finite(position)) { valid = false; break; }
            XMStoreFloat3(&positions[index], position);
        }
        if (!valid) continue;
        for (size_t index = 0; index + 2u < geometry.indices.size(); index += 3u)
        {
            const auto a = geometry.indices[index], b = geometry.indices[index + 1u], c = geometry.indices[index + 2u];
            if (a >= positions.size() || b >= positions.size() || c >= positions.size()) continue;
            float candidate;
            if (TriangleTests::Intersects(origin, direction, XMLoadFloat3(&positions[a]),
                XMLoadFloat3(&positions[b]), XMLoadFloat3(&positions[c]), candidate) && candidate < closest)
            { closest = candidate; closestMesh = meshOrdinal; hit = true; }
        }
    }
    if (!hit) return false;
    const float worldDistance = closest / localLength;
    if (!std::isfinite(worldDistance)) return false;
    distance = worldDistance;
    meshIndex = closestMesh;
    return true;
}

bool_t CModel::Try_GetCurrentPoseBounds(float3_t& minimum, float3_t& maximum) const
{
    if (MODEL::NONANIM == m_eType)
    {
        if (!m_bHasLocalBounds) return false;
        minimum = m_vLocalBoundsMin; maximum = m_vLocalBoundsMax; return true;
    }
    if (MODEL::ANIM != m_eType || m_Meshes.empty()) return false;
    const float limit = (std::numeric_limits<float>::max)();
    vector_t low = XMVectorReplicate(limit), high = XMVectorReplicate(-limit);
    bool hasPoint = false;
    for (const auto& mesh : m_Meshes)
    {
        if (!mesh || mesh->m_hasUniqueVertexBuffer ||
            mesh->m_BoneVertexBounds.size() != mesh->m_iNumBones ||
            mesh->m_BoneIndices.size() != mesh->m_iNumBones ||
            mesh->m_OffsetMatrices.size() != mesh->m_iNumBones) return false;
        for (uint32_t index = 0u; index < mesh->m_iNumBones; ++index)
        {
            const auto& bounds = mesh->m_BoneVertexBounds[index];
            if (!bounds.valid) continue;
            const uint32_t bone = mesh->m_BoneIndices[index];
            if (bone >= m_Bones.size() || !m_Bones[bone]) return false;
            // Exactly the same inverse-bind * combined order as the GPU palette.
            // Combined already owns the asset pretransform; do not apply it twice.
            const matrix_t skin = XMLoadFloat4x4(&mesh->m_OffsetMatrices[index]) *
                m_Bones[bone]->Get_CombinedTransformationMatrix();
            for (uint32_t corner = 0u; corner < 8u; ++corner)
            {
                const vector_t point = XMVector3TransformCoord(XMVectorSet(
                    (corner & 1u) ? bounds.maximum.x : bounds.minimum.x,
                    (corner & 2u) ? bounds.maximum.y : bounds.minimum.y,
                    (corner & 4u) ? bounds.maximum.z : bounds.minimum.z, 1.f), skin);
                if (XMVector3IsNaN(point) || XMVector3IsInfinite(point)) return false;
                low = XMVectorMin(low, point); high = XMVectorMax(high, point);
                hasPoint = true;
            }
        }
    }
    if (!hasPoint) return false;
    // Cover float rounding in normalized skin weights without changing scale.
    const vector_t margin = XMVectorReplicate(1.e-4f);
    XMStoreFloat3(&minimum, XMVectorSubtract(low, margin));
    XMStoreFloat3(&maximum, XMVectorAdd(high, margin));
    return true;
}

bool_t CModel::Try_GetAnimationEnvelopeRadius(f32_t& radius) const
{
    if (m_bAnimationEnvelopeExternalPose || m_bExplicitAnimationPose ||
        m_fAnimationSpeed < 0.f || m_fBlendElapsed < 0.f) return false;
    for (const auto& mesh : m_Meshes)
        if (!mesh || mesh->m_hasUniqueVertexBuffer) return false;
    if (!m_bAnimationEnvelopeAttempted)
    {
        m_bAnimationEnvelopeAttempted = true;
        m_fAnimationEnvelopeRadius = -1.f;
        Build_AnimationEnvelopeRadius(m_fAnimationEnvelopeRadius);
    }
    if (!std::isfinite(m_fAnimationEnvelopeRadius) || m_fAnimationEnvelopeRadius <= 0.f) return false;
    radius = m_fAnimationEnvelopeRadius;
    return true;
}

bool_t CModel::Build_AnimationEnvelopeRadius(f32_t& radius) const
{
    if (MODEL::ANIM != m_eType || m_Bones.empty() || m_Animations.empty() || m_Meshes.empty() ||
        m_BoneRestLocalTransforms.size() != m_Bones.size()) return false;
    const auto matrixEnvelope = [](const float4x4_t& m, double& stretch, std::array<double, 3>& translation) {
        const float* values = &m._11;
        for (size_t i = 0; i < 16; ++i) if (!std::isfinite(values[i])) return false;
        if (m._14 != 0.f || m._24 != 0.f || m._34 != 0.f || m._44 != 1.f) return false;
        // Gershgorin bound of A*A^T bounds the largest singular value, including shear.
        double squaredStretch = 0.;
        for (size_t row = 0; row < 3; ++row)
        {
            double sum = 0.;
            for (size_t other = 0; other < 3; ++other)
            {
                double dot = 0.;
                for (size_t axis = 0; axis < 3; ++axis)
                    dot += double(values[row * 4 + axis]) * values[other * 4 + axis];
                sum += std::abs(dot);
            }
            squaredStretch = (std::max)(squaredStretch, sum);
        }
        stretch = std::sqrt(squaredStretch) * 1.0001;
        translation = {std::abs(double(m._41)), std::abs(double(m._42)), std::abs(double(m._43))};
        return std::isfinite(stretch);
    };
    const auto length = [](const std::array<double, 3>& v) {return std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);};
    vector<std::array<double, 3>> localTranslation(m_Bones.size());
    vector<double> localScale(m_Bones.size()), combinedScale(m_Bones.size()), combinedRadius(m_Bones.size());
    for (size_t i = 0; i < m_Bones.size(); ++i)
        if (!m_Bones[i] || !matrixEnvelope(m_BoneRestLocalTransforms[i], localScale[i], localTranslation[i])) return false;
    for (const auto& animation : m_Animations)
        if (!animation || !animation->Accumulate_TransformEnvelope(localTranslation, localScale)) return false;
    if (m_iRootMotionBoneIndex >= 0)
    {
        if (size_t(m_iRootMotionBoneIndex) >= localTranslation.size() ||
            m_iRootMotionVerticalAxis < -1 || m_iRootMotionVerticalAxis > 2 ||
            !std::isfinite(m_fRootMotionVerticalScale) || m_fRootMotionVerticalScale < 0.f) return false;
        const double rest[3] = {m_vRootMotionRestTranslation.x, m_vRootMotionRestTranslation.y, m_vRootMotionRestTranslation.z};
        auto& bound = localTranslation[size_t(m_iRootMotionBoneIndex)];
        for (size_t axis = 0; axis < 3; ++axis)
        {
            if (!std::isfinite(rest[axis])) return false;
            bound[axis] = int32_t(axis) == m_iRootMotionVerticalAxis ?
                std::abs(rest[axis] * (1. - m_fRootMotionVerticalScale)) + bound[axis] * m_fRootMotionVerticalScale :
                std::abs(rest[axis]);
        }
    }
    double preScale = 0.; std::array<double, 3> preTranslation{};
    if (!matrixEnvelope(m_PreTransformMatrix, preScale, preTranslation)) return false;
    for (size_t i = 0; i < m_Bones.size(); ++i)
    {
        const int32_t parent = m_Bones[i]->Get_ParentBoneIndex();
        if (parent < -1 || (parent >= 0 && size_t(parent) >= i)) return false;
        const double parentScale = parent < 0 ? preScale : combinedScale[size_t(parent)];
        const double parentRadius = parent < 0 ? length(preTranslation) : combinedRadius[size_t(parent)];
        combinedScale[i] = localScale[i] * parentScale;
        combinedRadius[i] = length(localTranslation[i]) * parentScale + parentRadius;
        if (!std::isfinite(combinedScale[i]) || !std::isfinite(combinedRadius[i])) return false;
    }
    double result = 0.; bool hasVertex = false;
    for (const auto& mesh : m_Meshes)
    {
        if (!mesh || mesh->m_hasUniqueVertexBuffer || mesh->m_BoneVertexBounds.size() != mesh->m_iNumBones ||
            mesh->m_BoneIndices.size() != mesh->m_iNumBones || mesh->m_OffsetMatrices.size() != mesh->m_iNumBones) return false;
        for (size_t index = 0; index < mesh->m_iNumBones; ++index)
        {
            const auto& bounds = mesh->m_BoneVertexBounds[index];
            if (!bounds.valid) continue;
            const uint32_t bone = mesh->m_BoneIndices[index];
            if (bone >= m_Bones.size()) return false;
            const auto& offset = mesh->m_OffsetMatrices[index];
            for (const auto& row : offset.m)
                for (const float value : row) if (!std::isfinite(value)) return false;
            // Inverse binds may retain a positive homogeneous scale after
            // inversion. Divide by that exact w instead of rejecting or
            // rounding it. Positive skin weights remain a convex combination
            // after their homogeneous weights are normalized by the GPU.
            if (offset._14 != 0.f || offset._24 != 0.f || offset._34 != 0.f || offset._44 <= 0.f) return false;
            const double inverseW = 1. / double(offset._44);
            for (uint32_t corner = 0; corner < 8; ++corner)
            {
                const double x = (corner & 1u) ? bounds.maximum.x : bounds.minimum.x;
                const double y = (corner & 2u) ? bounds.maximum.y : bounds.minimum.y;
                const double z = (corner & 4u) ? bounds.maximum.z : bounds.minimum.z;
                const std::array<double, 3> point = {x*offset._11+y*offset._21+z*offset._31+offset._41,
                    x*offset._12+y*offset._22+z*offset._32+offset._42,
                    x*offset._13+y*offset._23+z*offset._33+offset._43};
                const double bound = length(point) * inverseW * combinedScale[bone] + combinedRadius[bone];
                if (!std::isfinite(bound)) return false;
                result = (std::max)(result, bound); hasVertex = true;
            }
        }
    }
    // Skin weights are normalized nonnegative values admitted by CMesh's bounds.
    // Their convex combination stays inside this sphere; cover float summation.
    result = result * 1.0001 + .0001;
    if (!hasVertex || result <= 0. || result >= (std::numeric_limits<float>::max)()) return false;
    radius = std::nextafter(static_cast<float>(result), (std::numeric_limits<float>::infinity)());
    return true;
}

void CModel::Include_BindGeometryPosition(fvector_t value)
{
    float3_t position; XMStoreFloat3(&position, value);
    if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) return;
    m_vBindGeometryBoundsMin.x = (std::min)(m_vBindGeometryBoundsMin.x, position.x);
    m_vBindGeometryBoundsMin.y = (std::min)(m_vBindGeometryBoundsMin.y, position.y);
    m_vBindGeometryBoundsMin.z = (std::min)(m_vBindGeometryBoundsMin.z, position.z);
    m_vBindGeometryBoundsMax.x = (std::max)(m_vBindGeometryBoundsMax.x, position.x);
    m_vBindGeometryBoundsMax.y = (std::max)(m_vBindGeometryBoundsMax.y, position.y);
    m_vBindGeometryBoundsMax.z = (std::max)(m_vBindGeometryBoundsMax.z, position.z);
    m_bHasBindGeometryBounds = true;
}

HRESULT CModel::Ready_Materials(const MODEL_ASSET_DATA& asset)
{
    Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Materials");
    // Each slot owns its material and device-only texture preparation. Publish
    // the complete vector only after every worker has finished successfully.
    vector<shared_ptr<CMaterial>> staged(asset.materials.size());
    static std::atomic_uint backgroundWorkers{ 0u };
    struct MATERIAL_PREPARATION final
    {
        const vector<MODEL_MATERIAL_DATA>& inputs;
        vector<shared_ptr<CMaterial>>& outputs;
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;
        std::atomic_uint& backgroundWorkers;
        std::atomic_size_t next{ 0u };
        std::atomic<HRESULT> result{ S_OK };

        void Run() noexcept
        {
            try
            {
                while (SUCCEEDED(result.load(std::memory_order_relaxed)))
                {
                    const size_t index = next.fetch_add(1u, std::memory_order_relaxed);
                    if (index >= inputs.size()) return;
                    auto material = CMaterial::Create(device, context, inputs[index]);
                    if (!material) { result.store(E_FAIL, std::memory_order_relaxed); return; }
                    outputs[index] = std::move(material);
                }
            }
            catch (const std::bad_alloc&) { result.store(E_OUTOFMEMORY, std::memory_order_relaxed); }
            catch (...) { result.store(E_FAIL, std::memory_order_relaxed); }
        }

        static void CALLBACK Work(PTP_CALLBACK_INSTANCE, void* parameter, PTP_WORK)
        {
            auto& batch = *static_cast<MATERIAL_PREPARATION*>(parameter);
            const HRESULT apartment = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if (SUCCEEDED(apartment) || apartment == RPC_E_CHANGED_MODE)
            {
                try
                {
                    Engine::CProfilerScope workerScope(CGameInstance::Get().Get_Profiler(), "Model.Load.MaterialWorker");
                    batch.Run();
                }
                catch (const std::bad_alloc&) { batch.result.store(E_OUTOFMEMORY, std::memory_order_relaxed); }
                catch (...) { batch.result.store(E_FAIL, std::memory_order_relaxed); }
            }
            // If COM preparation fails the owning loader drains the same queue.
            if (SUCCEEDED(apartment)) CoUninitialize();
            batch.backgroundWorkers.fetch_sub(1u, std::memory_order_relaxed);
        }
    } batch{ asset.materials, staged, m_pDevice, m_pContext, backgroundWorkers };

    // Small/static map materials stay serial. At most three extra workers
    // across all simultaneous model loads use Windows' existing thread pool.
    const unsigned processors = static_cast<unsigned>(GetActiveProcessorCount(ALL_PROCESSOR_GROUPS));
    const unsigned limit = processors > 2u ? (std::min)(3u, processors - 2u) : 0u;
    PTP_WORK work = asset.hasSkeleton && asset.materials.size() >= 3u && limit != 0u &&
        0u == (m_pDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_SINGLETHREADED) ?
        CreateThreadpoolWork(&MATERIAL_PREPARATION::Work, &batch, nullptr) : nullptr;
    struct WORK_JOIN final
    {
        PTP_WORK work;
        ~WORK_JOIN()
        {
            if (!work) return;
            WaitForThreadpoolWorkCallbacks(work, FALSE);
            CloseThreadpoolWork(work);
        }
    } join{ work };
    if (work)
    {
        for (size_t index = 1u; index < asset.materials.size(); ++index)
        {
            unsigned active = backgroundWorkers.load(std::memory_order_relaxed);
            while (active < limit && !backgroundWorkers.compare_exchange_weak(
                active, active + 1u, std::memory_order_relaxed)) {}
            if (active >= limit) break;
            SubmitThreadpoolWork(work);
        }
    }
    batch.Run();
    if (work)
    {
        Engine::CProfilerScope waitScope(CGameInstance::Get().Get_Profiler(), "Model.Load.MaterialJoin");
        WaitForThreadpoolWorkCallbacks(work, FALSE);
        CloseThreadpoolWork(work);
        join.work = nullptr;
    }
    const HRESULT result = batch.result.load(std::memory_order_relaxed);
    if (FAILED(result)) return result;
    if (std::any_of(staged.begin(), staged.end(), [](const auto& material) { return !material; }))
        return E_FAIL;
    m_iNumMaterials = static_cast<uint32_t>(staged.size());
    m_Materials = std::move(staged);
    return S_OK;
}

HRESULT CModel::Ready_Bones(const MODEL_ASSET_DATA& asset)
{
    Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Bones");
    m_Bones.reserve(asset.skeleton.bones.size());
    m_BoneRestLocalTransforms.reserve(asset.skeleton.bones.size());
    for (const MODEL_BONE_DATA& bone : asset.skeleton.bones)
    {
        auto pBone = CBone::Create(bone);
        if (nullptr == pBone)
            return E_FAIL;
        m_Bones.push_back(pBone);
        m_BoneRestLocalTransforms.push_back(bone.restLocal);
    }
    return S_OK;
}

HRESULT CModel::Ready_Animations(const MODEL_ASSET_DATA& asset)
{
    Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Animations");
    m_iNumAnimations = static_cast<uint32_t>(asset.animations.size());
    m_Animations.reserve(m_iNumAnimations);
    for (const MODEL_ANIMATION_DATA& animation : asset.animations)
    {
        auto pAnimation = CAnimation::Create(animation, m_Bones);
        if (nullptr == pAnimation)
            return E_FAIL;
        m_Animations.push_back(pAnimation);
    }
    return S_OK;
}

HRESULT CModel::Attach_AnimationSet(const CModel& animationSet)
{
	if (MODEL::ANIM != m_eType || MODEL::ANIM != animationSet.m_eType ||
		m_Bones.empty() || animationSet.m_Bones.empty() ||
		0 == m_iSkeletonHash ||
		m_iSkeletonHash != animationSet.m_iSkeletonHash ||
		m_Bones.size() != animationSet.m_Bones.size())
	{
		return E_FAIL;
	}

	for (const auto& pIncoming : animationSet.m_Animations)
	{
		for (const auto& pExisting : m_Animations)
		{
			if (pExisting->Compare_Name(pIncoming->Get_Name()))
				return E_FAIL;
		}
	}

	m_Animations.reserve(
		m_Animations.size() + animationSet.m_Animations.size());
	for (const auto& pIncoming : animationSet.m_Animations)
		m_Animations.push_back(pIncoming->Clone());
	m_iNumAnimations = static_cast<uint32_t>(m_Animations.size());
    m_bAnimationEnvelopeAttempted = false;
	return S_OK;
}

bool_t CModel::Sample_AnimationLocalTransforms(const char_t* name,
    const f32_t seconds, vector<float4x4_t>& output) const
{
    if (!name || !std::isfinite(seconds) || seconds < 0.f || m_Bones.empty() ||
        m_BoneRestLocalTransforms.size() != m_Bones.size()) return false;
    const CAnimation* selected = nullptr;
    for (const auto& animation : m_Animations)
        if (animation && animation->Compare_Name(name))
        {
            if (selected) return false;
            selected = animation.get();
        }
    if (!selected || selected->Get_TickPerSecond() <= 0.f) return false;
    const auto ticks = seconds * selected->Get_TickPerSecond();
    if (!std::isfinite(ticks) || ticks > selected->Get_Duration() + .001f) return false;
    auto staged = m_BoneRestLocalTransforms;
    if (!selected->Sample_LocalBoneTransforms((std::min)(ticks, selected->Get_Duration()), staged)) return false;
    for (const auto& local : staged) if (!Is_FiniteMatrix(local)) return false;
    output = std::move(staged); return true;
}

bool_t CModel::Install_AuthoredAnimations(const vector<MODEL_ANIMATION_DATA>& input, string& status)
{
    try
    {
        if (MODEL::ANIM != m_eType || !m_iSkeletonHash || m_Bones.empty() || input.size() > 32u)
            throw std::runtime_error("Authored clips need an admitted animated skeleton");
        auto staged = m_Animations;
        auto names = m_AuthoredAnimationNames;
        std::set<string> incoming;
        size_t totalKeys = 0u;
        for (auto animation : input)
        {
            if (!animation.name.starts_with("authored.") || animation.name.size() >= 128u ||
                !std::all_of(animation.name.begin(), animation.name.end(), [](unsigned char c) {
                    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                        (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-'; }) ||
                !incoming.insert(animation.name).second || animation.skeletonHash != m_iSkeletonHash ||
                !std::isfinite(animation.durationTicks) || animation.durationTicks <= 0.f || animation.durationTicks > 1800.f ||
                animation.ticksPerSecond != CAnimation::COOKED_TICK_RATE || animation.channels.size() != m_Bones.size())
                throw std::runtime_error("Invalid authored clip identity, skeleton or duration");
            std::set<int32_t> bones;
            for (auto& channel : animation.channels)
            {
                if (channel.resolvedBoneIndex < 0 || channel.resolvedBoneIndex >= static_cast<int32_t>(m_Bones.size()) ||
                    !bones.insert(channel.resolvedBoneIndex).second || channel.positionKeys.empty() ||
                    channel.rotationKeys.empty() || channel.scaleKeys.empty()) throw std::runtime_error("Invalid authored bone channels");
                const auto validateTimes = [&](const auto& keys)
                {
                    float previous = -1.f;
                    if (keys.size() > 8192u) throw std::runtime_error("Authored key count exceeds limit");
                    totalKeys += keys.size();
                    if (totalKeys > 6000000u) throw std::runtime_error("Authored clip key budget exceeded");
                    for (const auto& key : keys)
                    {
                        if (!std::isfinite(key.timeTicks) || key.timeTicks < 0.f || key.timeTicks <= previous ||
                            key.timeTicks > animation.durationTicks + .001f) throw std::runtime_error("Invalid authored key time");
                        previous = key.timeTicks;
                    }
                };
                validateTimes(channel.positionKeys); validateTimes(channel.scaleKeys); validateTimes(channel.rotationKeys);
                for (const auto& key : channel.positionKeys)
                    if (!std::isfinite(key.value.x) || !std::isfinite(key.value.y) || !std::isfinite(key.value.z))
                        throw std::runtime_error("Nonfinite authored translation");
                for (const auto& key : channel.scaleKeys)
                    if (!std::isfinite(key.value.x) || !std::isfinite(key.value.y) || !std::isfinite(key.value.z) ||
                        std::abs(key.value.x) <= .000001f || std::abs(key.value.y) <= .000001f || std::abs(key.value.z) <= .000001f) throw std::runtime_error("Invalid authored scale");
                for (auto& key : channel.rotationKeys)
                {
                    auto q = XMLoadFloat4(&key.value);
                    const auto length = XMVectorGetX(XMVector4LengthSq(q));
                    if (!std::isfinite(length) || length <= .000001f) throw std::runtime_error("Invalid authored quaternion");
                    XMStoreFloat4(&key.value, XMQuaternionNormalize(q));
                }
            }
            const auto existing = std::find_if(staged.begin(), staged.end(), [&](const auto& clip) {
                return clip && clip->Compare_Name(animation.name.c_str()); });
            if (existing != staged.end() && !names.contains(animation.name))
                throw std::runtime_error("Authored animation cannot replace a native clip");
            auto compiled = CAnimation::Create(animation, m_Bones);
            if (!compiled) throw std::runtime_error("Authored animation channel creation failed");
            if (existing == staged.end()) staged.push_back(std::move(compiled));
            else *existing = std::move(compiled);
            names.insert(animation.name);
        }
        m_Animations.swap(staged); m_AuthoredAnimationNames.swap(names);
        m_bAnimationEnvelopeAttempted = false;
        m_iNumAnimations = static_cast<uint32_t>(m_Animations.size());
        status = "Authored animation channels installed"; return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}


unique_ptr<CModel> CModel::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix,
    const bool_t bRetainOrderedStaticGeometry)
{
    auto pInstance = unique_ptr<CModel>(new CModel(pDevice, pContext));
    pInstance->m_bRetainOrderedStaticGeometry = bRetainOrderedStaticGeometry;

    if (FAILED(pInstance->Initialize_Prototype(eType, pModelFilePath, PreTransformMatrix)))
    {
        OutputDebugStringA("[CModel] Prototype creation failed.\n");
        return nullptr;
    }

    return pInstance;
}

unique_ptr<CModel> CModel::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const MODEL eType,
	const MODEL_ASSET_LOAD_DESC& loadDesc,
	fmatrix_t PreTransformMatrix,
	const bool_t bRetainOrderedStaticGeometry)
{
	auto pInstance = unique_ptr<CModel>(
		new CModel(pDevice, pContext));
	pInstance->m_bRetainOrderedStaticGeometry = bRetainOrderedStaticGeometry;
	if (FAILED(pInstance->Initialize_Prototype(
		eType, loadDesc, PreTransformMatrix)))
	{
		OutputDebugStringA("[CModel] Binary prototype creation failed.\n");
		return nullptr;
	}
	return pInstance;
}


unique_ptr<CModel> CModel::Create_MaterialVariant(const CModel& prototype,
    const MODEL_ASSET_LOAD_DESC& loadDesc)
{
    if (!prototype.m_pMaterialSource) return nullptr;
    const auto& identity = prototype.m_pMaterialSource->identity;
    if (identity.assetRoot.lexically_normal() != loadDesc.assetRoot.lexically_normal() ||
        identity.meshPath.lexically_normal() != loadDesc.meshPath.lexically_normal() ||
        identity.materialPath != loadDesc.materialPath || identity.skeletonPath != loadDesc.skeletonPath ||
        identity.animationPaths != loadDesc.animationPaths || identity.fallbackDiffusePath != loadDesc.fallbackDiffusePath ||
        identity.defaultAnimationName != loadDesc.defaultAnimationName) return nullptr;
    auto instance = unique_ptr<CModel>(new CModel(prototype));
    MODEL_MATERIAL_SOURCE staged = *prototype.m_pMaterialSource;
    if (FAILED(instance->Apply_MaterialOverrides(staged, loadDesc))) return nullptr;
    MODEL_ASSET_DATA materialAsset;
    materialAsset.materials = std::move(staged.materials);
    instance->m_Materials.clear();
    if (FAILED(instance->Ready_Materials(materialAsset))) return nullptr;
    return instance;
}

shared_ptr<CPrototype> CModel::Clone(void* pArg)
{
    auto pInstance = shared_ptr<CPrototype>(new CModel(*this));

    if (FAILED(pInstance->Initialize(pArg)))
    {
        OutputDebugStringA("[CModel] Clone failed.\n");
        return nullptr;
    }

    return pInstance;
}

void CModel::Free()
{
    if (false == m_isCloned && nullptr != m_pImporter)
        m_pImporter->FreeScene();
}
```

### Client/Private/WorldSequenceObject.cpp

변경 종류: 기존 파일 함수·선언 추가.

```cpp
#include "WorldSequenceObject.h"
#include "DeferredMaterialRenderUtils.h"
#include "MapAssetRenderUtils.h"
#include "GameInstance.h"
#include "Model.h"
#include "Part_Equipment.h"
#include "Shader.h"
#include "NpcPresentationAssetService.h"
#include "SourceEquipmentMaterialPrograms.h"
#include "SourceMovieMaterialPrograms.h"
#include <algorithm>
#include <cmath>
#include <sstream>

using namespace Client;
using namespace Engine;

namespace
{
    constexpr uint32_t SOURCE_GHOST_OPAQUE_PASS = 16u;

    std::string MaterialBindingFailure(const CModel& model, uint32_t mesh, HRESULT result)
    {
        std::ostringstream label;
        label << "material binding (mesh " << mesh << ", HRESULT=0x" << std::hex
            << static_cast<uint32_t>(result) << std::dec << ", material=" << model.Get_MaterialName(mesh);
        const auto* surface = model.Get_MaterialSurface(mesh);
        if (surface && surface->family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
            label << ", sourceProgram=" << surface->sourceCharacter.program;
        label << ')';
        return label.str();
    }
    // Forward sources bypass G-buffer coverage. Ghost 84 shares the product
    // resurrection's NONLIGHT/pass16; the remaining translucent sources use BLEND.
    MAP_ASSET_RENDER_PROFILE MovieStaticProfile(const MODEL_SURFACE_PARAMETERS* surface)
    {
        MAP_ASSET_RENDER_PROFILE profile;
        profile.cullMode = MAP_ASSET_CULL_MODE::TWO_SIDED;
        if (surface && surface->family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
            SourceMovieMaterial::Is_Forward(surface->sourceCharacter.program))
            profile.renderMode = SourceMovieMaterial::Is_Additive(surface->sourceCharacter.program) ?
                MAP_ASSET_RENDER_MODE::ADDITIVE : MAP_ASSET_RENDER_MODE::TRANSLUCENT;
        return profile;
    }

    bool IsMovieTranslucent(const MODEL_SURFACE_PARAMETERS* surface)
    {
        return MovieStaticProfile(surface).renderMode != MAP_ASSET_RENDER_MODE::DEFERRED;
    }

    uint32_t Resolve_ForwardSourcePass(const MODEL_SURFACE_PARAMETERS* surface)
    {
        if (!surface || surface->family != MODEL_SURFACE_FAMILY::SOURCE_CHARACTER) return 0u;
        if (SourceEquipmentMaterial::Is_Translucent(surface->sourceCharacter.program)) return 9u;
        switch (surface->sourceCharacter.program)
        {
        case 6u:  // Same eyelash/hair coverage as Character and Part_Equipment.
        case 7u:
        case 18u: return 9u;
        case 99u: return 9u;
        case 84u: return SOURCE_GHOST_OPAQUE_PASS;
        case 88u: return 10u;
        default: return 0u;
        }
    }
}

CWorldSequenceObject::CWorldSequenceObject(ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context) : CGameObject(device, context) {}

unique_ptr<CWorldSequenceObject> CWorldSequenceObject::Create(
    ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context)
{
    return unique_ptr<CWorldSequenceObject>(new CWorldSequenceObject(device, context));
}

HRESULT CWorldSequenceObject::Initialize(void* argument)
{
    if (!argument) return E_INVALIDARG;
    const auto& desc = *static_cast<const DESC*>(argument);
    if (!desc.modelPrototype || FAILED(__super::Initialize(argument))) return E_FAIL;
    m_Model = dynamic_pointer_cast<CModel>(desc.modelPrototype->Clone(nullptr));
    m_Diffuse = desc.diffuseTexture;
    if (!m_Model || FAILED(__super::Add_Component(desc.levelIndex,
        m_Model->Is_Skinned() ? L"Prototype_Component_Shader_VtxAnimMeshBinary" :
        L"Prototype_Component_Shader_VtxMeshBinary", L"Com_Shader", m_Shader))) return E_FAIL;
    // Split-material Movie actors often sample identical cooked channels at
    // the same time. Reuse only interpolation; each actor keeps its own bones.
    m_Model->Enable_AnimationSampleReuse();
    // A newly authored skinned resource may have no animation track yet.
    // Its cloned rest pose still needs the same combined matrices as a sampled clip.
    if (m_Model->Is_Skinned()) m_Model->Refresh_BoneCombinedMatrices();
    m_HasTranslucentMeshes = false;
    m_HasOpaqueGhostMeshes = false;
    for (uint32_t mesh = 0; mesh < m_Model->Get_NumMeshes(); ++mesh)
    {
        const auto* surface = m_Model->Get_MaterialSurface(mesh);
        if (m_Model->Is_Skinned())
        {
            const uint32_t pass = Resolve_ForwardSourcePass(surface);
            m_HasOpaqueGhostMeshes |= pass == SOURCE_GHOST_OPAQUE_PASS;
            m_HasTranslucentMeshes |= pass != 0u && pass != SOURCE_GHOST_OPAQUE_PASS;
        }
        else m_HasTranslucentMeshes |= IsMovieTranslucent(surface);
    }
    if (FAILED(CNpcPresentationAssetService::Prepare_SaydonHat(m_pDevice, m_pContext, m_Model, m_SaydonHatModel)))
        OutputDebugStringA("[SaydonHat] Sequence head prop unavailable; body preserved.\n");
    XMStoreFloat4x4(&m_World, XMMatrixIdentity());
    m_MaterialProfileId = desc.materialProfileId;
    m_Parts.clear();
    for (const auto& part : desc.presentationParts)
    {
        // Parts ride m_World directly: the sampled key already carries the model's facing.
        CPart_Equipment::PART_EQUIPMENT_DESC partDesc{};
        partDesc.pParentMatrix = &m_World;
        partDesc.iPrototypeLevelIndex = desc.levelIndex;
        partDesc.strModelTag = part.modelPrototypeTag;
        partDesc.strShaderTag = part.shaderPrototypeTag;
        partDesc.pSkeletonModel = m_Model;
        partDesc.pSocketBoneName = part.socketBone.empty() ? nullptr : part.socketBone.c_str();
        partDesc.strMaterialProfileId = m_MaterialProfileId;
        partDesc.pEmissiveOverride = &m_CombatPresentation;
        auto cloned = dynamic_pointer_cast<CPart_Equipment>(CGameInstance::Get().Clone_Prototype(
            desc.levelIndex, L"Prototype_GameObject_Part_Equipment", &partDesc));
        if (!cloned) return E_FAIL;
        m_Parts.push_back(std::move(cloned));
    }
    return S_OK;
}

shared_ptr<CPrototype> CWorldSequenceObject::Clone(void* argument)
{
    auto object = shared_ptr<CWorldSequenceObject>(new CWorldSequenceObject(*this));
    return SUCCEEDED(object->Initialize(argument)) ? object : nullptr;
}

bool_t CWorldSequenceObject::Sample(const float4x4_t& world, const bool_t visible,
    const WORLD_SEQUENCE_ANIMATION_TRACK* animation, const f32_t localMs, const f32_t windowEndMs)
{
    for (const auto& row : world.m)
        for (const float component : row)
            if (!std::isfinite(component)) return false;
    if (!std::isfinite(localMs)) return false;
    if (animation)
    {
        uint32_t index = UINT32_MAX;
        for (uint32_t i = 0; i < m_Model->Get_NumAnimations(); ++i)
            if (animation->clipName == m_Model->Get_AnimationName(i)) { index = i; break; }
        f32_t position = 0.f, duration = 0.f;
        if (index == UINT32_MAX || !m_Model->Get_AnimationProgress(index, position, duration) || duration <= 0.f)
            return false;
        const f32_t ticksPerSecond = m_Model->Get_AnimationTickPerSecond(index);
        f32_t ticks = 0.f;
        if (!CWorldSequenceDocument::Try_SampleAnimationTicks(*animation, localMs, windowEndMs,
            ticksPerSecond, duration, ticks)) return false;
        m_Model->Set_Animation(index, false);
        m_Model->Skip_Blend();
        if (!m_Model->Set_AnimTrackPosition(index, ticks)) return false;
        m_Model->Play_Animation(0.f);
    }
    m_SampleTimeSeconds = (std::max)(0.f, localMs) * 0.001f;
    m_World = world;
    m_Visible = visible;
    Refresh_InspectionHighlight();
    if (!visible) Hide();
    // Parts follow this exact pose: the weapon reads its grip bone from the body now.
    for (const auto& part : m_Parts) part->Update(0.f);
    return true;
}

#ifdef _DEBUG
bool_t CWorldSequenceObject::Try_GetAttachmentWorld(const std::string& bone, float4x4_t& out) const
{
    if (!m_Model || !m_Visible) return false;
    matrix_t attachment = XMMatrixIdentity();
    if (!bone.empty())
    {
        if (!m_Model->Has_Bone(bone.c_str())) return false;
        // Get_BoneMatrix includes model preScale, including the pivot translation.
        // Normalize only its axes: applying import scale again shrinks the grip offset.
        attachment = m_Model->Get_BoneMatrix(bone.c_str());
        for (int axis = 0; axis < 3; ++axis)
        {
            const float length = XMVectorGetX(XMVector3LengthSq(attachment.r[axis]));
            if (!std::isfinite(length) || length < 1e-12f) return false;
            attachment.r[axis] = XMVectorSetW(XMVector3Normalize(attachment.r[axis]), 0.f);
        }
    }
    float4x4_t sampled;
    XMStoreFloat4x4(&sampled, attachment * XMLoadFloat4x4(&m_World));
    for (const auto& row : sampled.m)
        for (const float component : row)
            if (!std::isfinite(component)) return false;
    out = sampled;
    return true;
}
#endif

void CWorldSequenceObject::Refresh_InspectionHighlight()
{
    m_CombatPresentation.isCombatHovered = m_Visible && m_InspectionDrawEnabled &&
        (m_CombatHovered || m_InspectionSelected);
}

void CWorldSequenceObject::Set_InspectionState(const bool_t drawEnabled, const bool_t selected)
{
    m_InspectionDrawEnabled = drawEnabled;
    m_InspectionSelected = selected;
    Refresh_InspectionHighlight();
    // Parts can already be in a render queue when the inspection selection changes.
    for (const auto& part : m_Parts)
        if (part) part->Set_PresentationSuppressed(!drawEnabled);
}

bool_t CWorldSequenceObject::Try_PickInspection(const float3_t& rayOrigin,
    const float3_t& rayDirection, f32_t& distance, uint32_t& meshIndex) const
{
    if (!m_Visible || !m_InspectionDrawEnabled || !m_Model) return false;
    return m_Model->Try_PickCurrentPose(m_World, rayOrigin, rayDirection, distance, meshIndex);
}

bool_t CWorldSequenceObject::Reset_ForReuse()
{
    Hide();
    m_CombatHovered = false;
    Set_InspectionState(true, false);
    m_CombatPresentation = {};
    m_HitFlashSeconds = 0.f;
    if (!m_Model || !Get_RenderStatus().empty()) return false;
    m_Model->Clear_SourceCharacterOverrides();
    if (m_Model->Is_Skinned())
    {
        m_Model->Clear_AnimationTransitionPose();
        m_Model->Skip_Blend();
        matrix_t rest;
        for (uint32_t bone = 0u; m_Model->Get_BoneRestLocalMatrix(bone, rest); ++bone)
            if (!m_Model->Set_BoneLocalMatrix(bone, rest)) return false;
        m_Model->Refresh_BoneCombinedMatrices();
    }
    XMStoreFloat4x4(&m_World, XMMatrixIdentity());
    m_SampleTimeSeconds = 0.f;
    return true;
}

void CWorldSequenceObject::Trigger_HitFlash()
{
    if (!m_Visible) return;
    m_HitFlashSeconds = .12f;
    m_CombatPresentation.isEnabled = true;
    m_CombatPresentation.vColor = { 1.f, .72f, .08f, 1.f };
    m_CombatPresentation.fIntensity = 4.f;
    m_CombatPresentation.usesSurfaceDetailMask = true;
}

void CWorldSequenceObject::Late_Update(f32_t deltaSeconds)
{
    // Apply_Objects temporarily hides each object before resampling it. Only a
    // still-hidden object at the render boundary has actually left presentation.
    if (!m_Visible)
    { m_CombatHovered = false; m_CombatPresentation = {}; m_HitFlashSeconds = 0.f; return; }
    Refresh_InspectionHighlight();
    if (std::isfinite(deltaSeconds) && deltaSeconds > 0.f && m_HitFlashSeconds > 0.f)
    {
        m_HitFlashSeconds = (std::max)(0.f, m_HitFlashSeconds - deltaSeconds);
        m_CombatPresentation.fIntensity = 4.f * m_HitFlashSeconds / .12f;
        m_CombatPresentation.isEnabled = m_HitFlashSeconds > 0.f;
    }
    if (!m_InspectionDrawEnabled) return;
    const auto self = static_pointer_cast<CGameObject>(shared_from_this());
    CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONBLEND, self);
    if (m_HasOpaqueGhostMeshes) CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONLIGHT, self);
    if (m_HasTranslucentMeshes) CGameInstance::Get().Add_RenderObject(RENDERGROUP::BLEND, self);
    for (uint32_t mesh = 0; mesh < m_Model->Get_NumMeshes(); ++mesh)
    {
        const auto* surface = m_Model->Get_MaterialSurface(mesh);
        if (surface && surface->family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
            SourceMovieMaterial::Needs_SceneColor(surface->sourceCharacter.program))
            CGameInstance::Get().Request_SceneColorSnapshot();
    }
    for (const auto& part : m_Parts)
        CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONBLEND, static_pointer_cast<CGameObject>(part));
}

HRESULT CWorldSequenceObject::Render_Group(RENDERGROUP group)
{
    if (!m_Visible || !m_InspectionDrawEnabled) return S_OK;
    if (RENDERGROUP::NONLIGHT == group) return Render_ForwardSource(true);
    return RENDERGROUP::BLEND == group ? Render_ForwardSource(false) : Render();
}

HRESULT CWorldSequenceObject::Render_Mesh(uint32_t mesh)
{
    if (XMVectorGetX(XMMatrixDeterminant(XMLoadFloat4x4(&m_World))) >= 0.f)
        return m_Model->Render(mesh);
    ComPtr<ID3D11RasterizerState> source;
    m_pContext->RSGetState(source.GetAddressOf());
    if (!source) return E_FAIL;
    D3D11_RASTERIZER_DESC desc{};
    source->GetDesc(&desc);
    if (desc.CullMode == D3D11_CULL_NONE) return m_Model->Render(mesh);
    if (m_SourceRasterState.Get() != source.Get())
    {
        desc.FrontCounterClockwise = !desc.FrontCounterClockwise;
        ComPtr<ID3D11RasterizerState> reflected;
        if (FAILED(m_pDevice->CreateRasterizerState(&desc, reflected.GetAddressOf()))) return E_FAIL;
        m_SourceRasterState = source;
        m_ReflectedRasterState = reflected;
    }
    m_pContext->RSSetState(m_ReflectedRasterState.Get());
    const HRESULT result = m_Model->Render(mesh);
    m_pContext->RSSetState(source.Get());
    return result;
}

HRESULT CWorldSequenceObject::Render()
{
    if (!m_Visible || !m_InspectionDrawEnabled) return S_OK;
    const auto failed = [this](const std::string& stage)
    { m_RenderStatus = "World Object render failed: " + stage; return E_FAIL; };
    if (FAILED(m_Shader->Bind_Matrix("g_WorldMatrix", &m_World)) ||
        FAILED(CGameInstance::Get().Bind_Transform(m_Shader, "g_ViewMatrix", D3DTS::VIEW)) ||
        FAILED(CGameInstance::Get().Bind_Transform(m_Shader, "g_ProjMatrix", D3DTS::PROJ))) return failed("world/view/projection binding");
    const bool_t animated = m_Model->Is_Skinned();
    MAP_ASSET_RENDER_PROFILE profile;
    profile.cullMode = MAP_ASSET_CULL_MODE::TWO_SIDED;
    if (!animated)
    {
        matrix_t basis = XMLoadFloat4x4(&m_World);
        basis.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
        float4x4_t normal;
        XMStoreFloat4x4(&normal, XMMatrixTranspose(XMMatrixInverse(nullptr, basis)));
        if (FAILED(m_Shader->Bind_Matrix("g_WorldInvTransposeMatrix", &normal))) return failed("normal matrix binding");
    }
    for (uint32_t mesh = 0; mesh < m_Model->Get_NumMeshes(); ++mesh)
    {
        const auto* surface = m_Model->Get_MaterialSurface(mesh);
        if (animated ? 0u != Resolve_ForwardSourcePass(surface) : IsMovieTranslucent(surface)) continue;
        if (!animated) profile = MovieStaticProfile(surface);
        const bool mapSurface = surface && surface->family == MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED;
        const DEFERRED_MATERIAL_PROFILE bodyProfile = m_MaterialProfileId.empty() ? DEFERRED_MATERIAL_PROFILE{} :
            Resolve_DeferredMaterialProfile(m_MaterialProfileId, m_Model->Get_MaterialName(mesh));
        const HRESULT material = animated && !mapSurface ? Bind_DeferredMaterialInputs(*m_Model, m_Shader, mesh, bodyProfile, &m_CombatPresentation, m_Diffuse) :
            CMapAssetRenderUtils::Bind_Material(m_Model, m_Shader, mesh, profile, m_SampleTimeSeconds, m_Diffuse);
        const auto meshLabel = " (mesh " + std::to_string(mesh) + ")";
        if (FAILED(material)) return failed(MaterialBindingFailure(*m_Model, mesh, material));
        if (FAILED(Bind_CombatPresentationInputs(*m_Model, m_Shader, mesh, m_CombatPresentation)))
            return failed("combat presentation binding" + meshLabel);
        if (animated && FAILED(m_Model->Bind_BoneMatrices(m_Shader, "g_BoneMatrices", mesh)))
            return failed("bone matrix binding" + meshLabel);
        // Reuse the existing two-sided PS_MAIN pass for thin animated map/card surfaces.
        const uint32_t pass = animated ? (mapSurface ? 6u : 0u) : CMapAssetRenderUtils::Select_Pass(profile, false);
        if (FAILED(m_Shader->Begin(pass)))
            return failed("shader pass" + meshLabel);
        if (FAILED(Render_Mesh(mesh))) return failed("mesh submission" + meshLabel);
        (void)Render_CombatHoverMesh(*m_Model, m_Shader, mesh, &m_CombatPresentation, animated, false,
            XMVectorGetX(XMMatrixDeterminant(XMLoadFloat4x4(&m_World))) < 0.f);
    }
    if (FAILED(CNpcPresentationAssetService::Render_SaydonHat(m_Model, m_SaydonHatModel, m_Shader, m_World,
        0u, true, false, &m_CombatPresentation)))
        return failed("head prop submission");
    m_RenderStatus.clear();
    return S_OK;
}

HRESULT CWorldSequenceObject::Render_ForwardSource(bool opaqueGhost)
{
    if (!m_Visible || !m_InspectionDrawEnabled) return S_OK;
    std::string& status = opaqueGhost ? m_OpaqueGhostRenderStatus : m_TranslucentRenderStatus;
    const auto failed = [&status, opaqueGhost](const std::string& stage)
    {
        status = std::string(opaqueGhost ? "World Object opaque ghost render failed: " :
            "World Object translucent render failed: ") + stage;
        return E_FAIL;
    };
    if (FAILED(m_Shader->Bind_Matrix("g_WorldMatrix", &m_World)) ||
        FAILED(CGameInstance::Get().Bind_Transform(m_Shader, "g_ViewMatrix", D3DTS::VIEW)) ||
        FAILED(CGameInstance::Get().Bind_Transform(m_Shader, "g_ProjMatrix", D3DTS::PROJ)) ||
        (m_Model->Is_Skinned() && FAILED(CMapAssetRenderUtils::Bind_SourceCharacterForwardLights(m_Shader))))
        return failed("world/view/projection/forward light binding");
    const bool animated = m_Model->Is_Skinned();
    if (!animated)
    {
        matrix_t basis = XMLoadFloat4x4(&m_World);
        basis.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
        float4x4_t normal;
        XMStoreFloat4x4(&normal, XMMatrixTranspose(XMMatrixInverse(nullptr, basis)));
        if (FAILED(m_Shader->Bind_Matrix("g_WorldInvTransposeMatrix", &normal)))
            return failed("normal matrix binding");
    }
    for (uint32_t mesh = 0; mesh < m_Model->Get_NumMeshes(); ++mesh)
    {
        const auto* surface = m_Model->Get_MaterialSurface(mesh);
        const auto profile = MovieStaticProfile(surface);
        const uint32_t pass = animated ? Resolve_ForwardSourcePass(surface) :
            (IsMovieTranslucent(surface) ? CMapAssetRenderUtils::Select_Pass(profile, false) : 0u);
        if (0u == pass || (animated && pass == SOURCE_GHOST_OPAQUE_PASS) != opaqueGhost) continue;
        const auto meshLabel = " (mesh " + std::to_string(mesh) + ")";
        const DEFERRED_MATERIAL_PROFILE bodyProfile = m_MaterialProfileId.empty() ? DEFERRED_MATERIAL_PROFILE{} :
            Resolve_DeferredMaterialProfile(m_MaterialProfileId, m_Model->Get_MaterialName(mesh));
        if (animated)
        {
            HRESULT material = Bind_DeferredMaterialInputs(*m_Model, m_Shader, mesh, bodyProfile, &m_CombatPresentation, m_Diffuse);
            if (SUCCEEDED(material)) material = m_Model->Bind_SourceCharacterForwardLight(m_Shader, mesh);
            if (FAILED(material)) return failed(MaterialBindingFailure(*m_Model, mesh, material));
        }
        else
        {
            const HRESULT material = CMapAssetRenderUtils::Bind_Material(m_Model, m_Shader, mesh,
                profile, m_SampleTimeSeconds, m_Diffuse);
            if (FAILED(material)) return failed(MaterialBindingFailure(*m_Model, mesh, material));
        }
        if (FAILED(Bind_CombatPresentationInputs(*m_Model, m_Shader, mesh, m_CombatPresentation)))
            return failed("combat presentation binding" + meshLabel);
        if (animated && FAILED(m_Model->Bind_BoneMatrices(m_Shader, "g_BoneMatrices", mesh)))
            return failed("bone matrix binding" + meshLabel);
        if (FAILED(m_Shader->Begin(pass))) return failed("shader pass" + meshLabel);
        if (FAILED(Render_Mesh(mesh))) return failed("mesh submission" + meshLabel);
        // Reuse the existing forward outline pass; native material constants stay unchanged.
        (void)Render_CombatHoverMesh(*m_Model, m_Shader, mesh, &m_CombatPresentation, animated, true,
            XMVectorGetX(XMMatrixDeterminant(XMLoadFloat4x4(&m_World))) < 0.f);
    }
    status.clear();
    return S_OK;
}
```

## G04. 프로젝트 등록과 검증

기존5 H/CPP의 project/filter 등록을 유지한다. 새 C++ production 파일이 없어 새 등록은 없다.
`Tools/CharacterSelectPipeline/test_movie_animation_sample_reuse.py`는 out 아래에 실제 production
Animation/Channel/Bone 전문과 CMesh::Build_SkinPalette를 복사하여 MSVC v14314.44,
Windows SDK10.0.26100.0, DirectXMath로 컴파일한다. Header 접근 지정자만 test에서 public으로 열고
application/profiler plumbing을 대역 처리한다. 강제 충돌 모드는 registry bucket만0으로 치환한다.

설치된 도화가237본·차원술사175본의 intro/loop4개에서 cache OFF/ON local·combined·실제 inverse-bind
skin palette를 bit-exact 대조한다. 임의/역방향 시각·loop·끝·pool reset·독립 clock·다른 key/index/count·
unkeyed bone 보존·sample 뒤 local 수정 격리·owner 파괴·4thread 동시 접근을 검사한다.
분할10actor의 pose sample+combine median을 별도 측정하고 실제 Client FPS로 환산하지 않는다.

```powershell
python Tools/CharacterSelectPipeline/test_movie_animation_sample_reuse.py --configuration Debug --out out/MovieSampleReuseRegression/v143-Debug
python Tools/CharacterSelectPipeline/test_movie_animation_sample_reuse.py --configuration Release --out out/MovieSampleReuseRegression/v143-Release
python Tools/CharacterSelectPipeline/test_movie_animation_sample_reuse.py --configuration Release --force-hash-collision --out out/MovieSampleReuseRegression/v143-Release-Collision
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -MaxCompilerProcesses 4
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -MaxCompilerProcesses 4
```

`Animation.Channels.Reuse` scope count/time은 재사용, `.Evaluate`는 새 sample을 표시한다.
기존 `Animation.Channels.Update` 안에 중첩되므로 시간을 더하지 않는다. 새 UI/schema는 없다.
Product 빌드·SDK 배포·사용자 화면/FPS는 독립 완료 조건이며 source/native probe 성공으로 대체하지 않는다.
