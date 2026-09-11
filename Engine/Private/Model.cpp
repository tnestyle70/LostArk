#include "Model.h"
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

    m_Bones[iBoneIndex]->Update_TransformationMatrix(Matrix);
    return true;
}

uint32_t CModel::Pose_BonesFrom(const CModel& source)
{
    /* By name, because the two skeletons are cooked separately and neither order nor count
       matches: a worn part carries the body's bones plus its own. */
    if (m_SourcePoseBoneIndices.size() != m_Bones.size() ||
        m_pSourcePoseModel != &source)
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
    return supplied;
}

void CModel::Refresh_BoneCombinedMatrices()
{
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
    uint32_t iInstanceByteOffset)
{
    if (iMeshIndex >= m_Meshes.size() ||
        nullptr == m_Meshes[iMeshIndex])
    {
        return E_INVALIDARG;
    }

    const HRESULT drawResult = m_Meshes[iMeshIndex]->Render_Instanced(
        pInstanceBuffer, iInstanceStride, iNumInstances, iInstanceByteOffset);
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
    {
        Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Bones.Combine");
    for (auto& pBone : m_Bones)
    {
        pBone->Update_CombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
    }
    }

    return isFinished;
}

HRESULT CModel::Bind_BoneMatrices(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex)
{
   return m_Meshes[iMeshIndex]->Bind_Resource(pShader, pConstantName, m_Bones);    
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
        if (pMaterial->Set_SourceCharacterTextureOverride(iRegister, pTexture))
            ++matched;
    }
    return matched;
}

void CModel::Clear_SourceCharacterOverrides()
{
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr != pMaterial)
            pMaterial->Clear_SourceCharacterOverrides();
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
        if (replacement.surface.family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
        {
            const auto& source = replacement.surface.sourceCharacter;
            const uint32_t mask = source.baseTextureMask | source.lightTextureMask;
            if (source.program == 0u || source.program > 84u || (source.program > 65u && source.program < 80u) ||
                (mask == 0u && source.program != 64u && source.program != 65u) ||
                ((source.program == 64u || source.program == 65u) && mask != 0u) || source.requiredExtraUVMask > 3u ||
                (mask >> SOURCE_CHARACTER_TEXTURE_COUNT) != 0u ||
                (replacement.surface.hasBakedLighting && !((source.program >= 80u && source.program <= 83u) ||
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
                if ((source.program == 5u || source.program == 7u ||
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
                (surface.family == MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && (flags & (1u | 8u | 64u))) ||
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
            if ((flags & ~65535u) != 0u || ((flags & 8u) != 0u && (flags & 4u) == 0u) ||
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
                surface.overlaySpecularIntensity, surface.uvTiling.x, surface.uvTiling.y, surface.detailNormalIntensity, surface.detailNormalTiling };
            if (any_of(begin(values), end(values), [](float v) { return !std::isfinite(v) || v < 0.f; }) ||
                surface.sourceOverlayFlags > 511u || surface.overlayTiling <= 0.f || surface.uvTiling.x <= 0.f || surface.uvTiling.y <= 0.f || surface.hasEnvironmentCube || surface.hasEmissive ||
                !replacement.reflectionPath.empty()) return failOverride("invalid source overlay surface");
            const float signedValues[] = { surface.sourceOverlayDirection.x, surface.sourceOverlayDirection.y,
                surface.sourceOverlayDirection.z, surface.sourceOverlayDirection.w,
                surface.sourceBgUV.x, surface.sourceBgUV.y, surface.sourceBgUV.z, surface.sourceBgUV.w };
            if (any_of(begin(signedValues), end(signedValues), [](float v) { return !std::isfinite(v); }) ||
                (((surface.sourceOverlayFlags & 32u) != 0u) != !replacement.detailNormalPath.empty()))
                return failOverride("invalid source overlay direction, UV or detail branch");
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
            { &replacement.environmentBRDFPath, &match->environmentBRDFPath }
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

	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(
			m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));

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

        auto pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType,
            mesh, asset.skeleton, XMLoadFloat4x4(&m_PreTransformMatrix));
        if (nullptr == pMesh)
            return E_FAIL;
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

HRESULT CModel::Ready_Materials(const MODEL_ASSET_DATA& asset)
{
    Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Materials");
    m_iNumMaterials = static_cast<uint32_t>(asset.materials.size());
    m_Materials.reserve(m_iNumMaterials);
    for (const MODEL_MATERIAL_DATA& material : asset.materials)
    {
        auto pMaterial = CMaterial::Create(m_pDevice, m_pContext, material);
        if (nullptr == pMaterial)
            return E_FAIL;
        m_Materials.push_back(pMaterial);
    }
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
	return S_OK;
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
    if (prototype.m_eType != MODEL::NONANIM || !prototype.m_pMaterialSource) return nullptr;
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
