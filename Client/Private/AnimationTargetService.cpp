#include "AnimationTargetService.h"

#include "Character.h"
#include "Model.h"
#include "Valtan.h"

#include <cmath>

weak_ptr<Client::CCharacter> Client::CAnimationTargetService::s_Target;
weak_ptr<Client::CCharacter>
	Client::CAnimationTargetService::s_PreviewCharacter;
weak_ptr<Client::CValtan> Client::CAnimationTargetService::s_PreviewBoss;
weak_ptr<Engine::CModel> Client::CAnimationTargetService::s_PreviewModel;
string Client::CAnimationTargetService::s_PreviewAssetName;
float4x4_t Client::CAnimationTargetService::s_PreviewRootMatrix;
uint64_t Client::CAnimationTargetService::s_TargetGeneration = 1u;

namespace
{
	void Advance_TargetGeneration(uint64_t& targetGeneration)
	{
		++targetGeneration;
		if (0u == targetGeneration)
			++targetGeneration;
	}
}

void Client::CAnimationTargetService::Bind(
	const shared_ptr<CCharacter>& character)
{
	if (s_Target.lock() == character)
		return;

	s_Target = character;
	Advance_TargetGeneration(s_TargetGeneration);
}

void Client::CAnimationTargetService::Unbind(
	const shared_ptr<CCharacter>& character)
{
	if (s_Target.lock() != character)
		return;

	s_Target.reset();
	Advance_TargetGeneration(s_TargetGeneration);
}

void Client::CAnimationTargetService::Bind_Preview(
	const shared_ptr<Engine::CModel>& model,
	const string& assetName,
	const float4x4_t& rootMatrix)
{
	if (s_PreviewModel.lock() == model &&
		s_PreviewAssetName == assetName)
	{
		/* Re-selecting the same asset may still have moved it, so the root is
		   refreshed without disturbing the generation the tools compare. */
		s_PreviewRootMatrix = rootMatrix;
		return;
	}

	s_PreviewCharacter.reset();
	s_PreviewBoss.reset();
	s_PreviewModel = model;
	s_PreviewAssetName = assetName;
	s_PreviewRootMatrix = rootMatrix;
	Advance_TargetGeneration(s_TargetGeneration);
}

void Client::CAnimationTargetService::Bind_Preview(
	const shared_ptr<CCharacter>& character)
{
	if (nullptr == character || nullptr == character->Get_BodyModel() ||
		nullptr == character->Get_Spec() ||
		nullptr == character->Get_Spec()->pAssetName)
	{
		return;
	}
	if (s_PreviewCharacter.lock() == character)
		return;

	s_PreviewBoss.reset();
	s_PreviewModel.reset();
	s_PreviewCharacter = character;
	s_PreviewAssetName = character->Get_Spec()->pAssetName;
	XMStoreFloat4x4(&s_PreviewRootMatrix, XMMatrixIdentity());
	Advance_TargetGeneration(s_TargetGeneration);
}

void Client::CAnimationTargetService::Bind_Preview(
	const shared_ptr<CValtan>& valtan,
	const string& assetName)
{
	float4x4_t presentationRoot{};
	if (nullptr == valtan || nullptr == valtan->Get_BodyModel() ||
		assetName.empty() ||
		!valtan->Try_Get_PresentationRootMatrix(&presentationRoot))
	{
		return;
	}
	if (s_PreviewBoss.lock() == valtan &&
		s_PreviewAssetName == assetName)
	{
		return;
	}

	s_PreviewCharacter.reset();
	s_PreviewModel.reset();
	s_PreviewBoss = valtan;
	s_PreviewAssetName = assetName;
	XMStoreFloat4x4(&s_PreviewRootMatrix, XMMatrixIdentity());
	Advance_TargetGeneration(s_TargetGeneration);
}

void Client::CAnimationTargetService::Unbind_Preview(
	const shared_ptr<Engine::CModel>& model)
{
	if (s_PreviewModel.lock() == model)
		Clear_Preview();
}

void Client::CAnimationTargetService::Unbind_Preview(
	const shared_ptr<CCharacter>& character)
{
	if (s_PreviewCharacter.lock() == character)
		Clear_Preview();
}

void Client::CAnimationTargetService::Unbind_Preview(
	const shared_ptr<CValtan>& valtan)
{
	if (s_PreviewBoss.lock() == valtan)
		Clear_Preview();
}

void Client::CAnimationTargetService::Clear_Preview()
{
	if (s_PreviewCharacter.expired() && s_PreviewBoss.expired() &&
		s_PreviewModel.expired() &&
		s_PreviewAssetName.empty())
		return;
	s_PreviewCharacter.reset();
	s_PreviewBoss.reset();
	s_PreviewModel.reset();
	s_PreviewAssetName.clear();
	XMStoreFloat4x4(&s_PreviewRootMatrix, XMMatrixIdentity());
	Advance_TargetGeneration(s_TargetGeneration);
}

shared_ptr<Client::CCharacter>
Client::CAnimationTargetService::Resolve_Character()
{
	const shared_ptr<CCharacter> preview = s_PreviewCharacter.lock();
	if (nullptr != preview)
		return preview;
	return s_Target.lock();
}

shared_ptr<Client::CCharacter>
Client::CAnimationTargetService::Resolve_SceneCharacter()
{
	return s_Target.lock();
}

shared_ptr<Client::CValtan>
Client::CAnimationTargetService::Resolve_Boss()
{
	return s_PreviewBoss.lock();
}

shared_ptr<Engine::CModel>
Client::CAnimationTargetService::Resolve_Model()
{
	const shared_ptr<CCharacter> previewCharacter =
		s_PreviewCharacter.lock();
	if (nullptr != previewCharacter)
		return previewCharacter->Get_BodyModel();

	const shared_ptr<CValtan> previewBoss = s_PreviewBoss.lock();
	if (nullptr != previewBoss)
		return previewBoss->Get_BodyModel();

	const shared_ptr<Engine::CModel> preview = s_PreviewModel.lock();
	if (nullptr != preview)
		return preview;

	const shared_ptr<CCharacter> character = Resolve_Character();
	return nullptr == character ? nullptr : character->Get_BodyModel();
}

string Client::CAnimationTargetService::Resolve_AssetName()
{
	const shared_ptr<CCharacter> previewCharacter =
		s_PreviewCharacter.lock();
	if (nullptr != previewCharacter &&
		nullptr != previewCharacter->Get_Spec() &&
		nullptr != previewCharacter->Get_Spec()->pAssetName)
	{
		return previewCharacter->Get_Spec()->pAssetName;
	}
	if (nullptr != s_PreviewBoss.lock())
		return s_PreviewAssetName;
	if (nullptr != s_PreviewModel.lock())
		return s_PreviewAssetName;

	const shared_ptr<CCharacter> character = Resolve_Character();
	const CHARACTER_SPEC* spec =
		nullptr == character ? nullptr : character->Get_Spec();
	return nullptr == spec || nullptr == spec->pAssetName ?
		string{} : string{ spec->pAssetName };
}

uint64_t Client::CAnimationTargetService::Resolve_TargetGeneration()
{
	return s_TargetGeneration;
}

bool_t Client::CAnimationTargetService::Resolve_RootTransform(
	float4x4_t* pOut)
{
	if (nullptr == pOut)
		return false;

	const shared_ptr<CCharacter> previewCharacter =
		s_PreviewCharacter.lock();
	if (nullptr != previewCharacter)
	{
		const shared_ptr<Engine::CTransform> transform =
			previewCharacter->Get_Transform();
		if (nullptr == transform)
			return false;
		*pOut = *transform->Get_WorldMatrixPtr();
		return true;
	}

	const shared_ptr<CValtan> previewBoss = s_PreviewBoss.lock();
	if (nullptr != previewBoss)
		return previewBoss->Try_Get_PresentationRootMatrix(pOut);

	/* A selected preview body owns the target, so its fixed parent matrix wins
	   over the scene character exactly as Resolve_Model() prefers its model. */
	if (nullptr != s_PreviewModel.lock())
	{
		*pOut = s_PreviewRootMatrix;
		return true;
	}

	const shared_ptr<CCharacter> character = Resolve_Character();
	if (nullptr == character)
		return false;

	const shared_ptr<Engine::CTransform> transform =
		character->Get_Transform();
	if (nullptr == transform)
		return false;

	*pOut = *transform->Get_WorldMatrixPtr();
	return true;
}

bool_t Client::CAnimationTargetService::Resolve_AnchorTransform(
	const char_t* pAnchorSlotId,
	float4x4_t* pOut)
{
	if (nullptr == pAnchorSlotId || '\0' == pAnchorSlotId[0] ||
		nullptr == pOut)
	{
		return false;
	}

	const shared_ptr<Engine::CModel> model = Resolve_Model();
	if (nullptr == model || !model->Has_Bone(pAnchorSlotId))
		return false;

	float4x4_t root{};
	if (!Resolve_RootTransform(&root))
		return false;

	/* Same composition CPart_Equipment uses for a socketed piece: the bone
	   matrix already carries the animated pose, so it only needs the owner's
	   world matrix applied on top. */
	XMStoreFloat4x4(
		pOut,
		model->Get_BoneMatrix(pAnchorSlotId) * XMLoadFloat4x4(&root));
	return true;
}

bool_t Client::CAnimationTargetService::Prepare_HistoricalPoseBinding(
	const uint64_t iExpectedTargetGeneration,
	const uint32_t iExpectedAnimationIndex,
	const std::span<const std::string> BoneNames,
	CAnimationHistoricalPoseBinding& OutBinding)
{
	return Prepare_HistoricalPoseBindingForAnimation(
		iExpectedTargetGeneration, iExpectedAnimationIndex, {}, BoneNames, OutBinding);
}

bool_t Client::CAnimationTargetService::Prepare_HistoricalClipPoseBinding(
	const uint64_t iExpectedTargetGeneration,
	const std::string& strClipName,
	const std::span<const std::string> BoneNames,
	CAnimationHistoricalPoseBinding& OutBinding)
{
	if (strClipName.empty() ||
		strClipName.find('\0') != std::string::npos ||
		iExpectedTargetGeneration != Resolve_TargetGeneration())
	{
		return false;
	}
	const shared_ptr<Engine::CModel> Model = Resolve_Model();
	if (nullptr == Model)
		return false;
	uint32_t iAnimationIndex = UINT32_MAX;
	for (uint32_t i = 0u; i < Model->Get_NumAnimations(); ++i)
	{
		const char_t* pName = Model->Get_AnimationName(i);
		if (nullptr == pName || strClipName != pName)
			continue;
		if (iAnimationIndex != UINT32_MAX)
			return false;
		iAnimationIndex = i;
	}
	return iAnimationIndex != UINT32_MAX &&
		Prepare_HistoricalPoseBindingForAnimation(
			iExpectedTargetGeneration, iAnimationIndex, strClipName, BoneNames, OutBinding);
}

bool_t Client::CAnimationTargetService::Prepare_HistoricalPoseBindingForAnimation(
	const uint64_t iExpectedTargetGeneration,
	const uint32_t iExpectedAnimationIndex,
	const std::string& strExplicitClipName,
	const std::span<const std::string> BoneNames,
	CAnimationHistoricalPoseBinding& OutBinding)
{
	if (0u == iExpectedTargetGeneration ||
		iExpectedTargetGeneration != Resolve_TargetGeneration() ||
		BoneNames.empty())
	{
		return false;
	}

	const shared_ptr<Engine::CModel> Model = Resolve_Model();
	if (nullptr == Model ||
		(strExplicitClipName.empty() &&
		 iExpectedAnimationIndex != Model->Get_CurrentAnimIndex()) ||
		iExpectedAnimationIndex >= Model->Get_NumAnimations())
	{
		return false;
	}

	if (!strExplicitClipName.empty())
	{
		const char_t* pName = Model->Get_AnimationName(iExpectedAnimationIndex);
		if (nullptr == pName || strExplicitClipName != pName)
			return false;
	}
	CAnimationHistoricalPoseBinding Staged;
	Staged.m_Model = Model;
	Staged.m_iTargetGeneration = iExpectedTargetGeneration;
	Staged.m_iAnimationIndex = iExpectedAnimationIndex;
	Staged.m_strExplicitClipName = strExplicitClipName;
	Staged.m_BoneIndices.reserve(BoneNames.size());
	for (const std::string& BoneName : BoneNames)
	{
		if (BoneName.empty())
			return false;
		const int32_t iBoneIndex = Model->Find_BoneIndex(BoneName.c_str());
		if (iBoneIndex < 0)
			return false;
		Staged.m_BoneIndices.push_back(static_cast<uint32_t>(iBoneIndex));
	}

	f32_t fTrackPosition = 0.f;
	if (!Model->Get_AnimationProgress(iExpectedAnimationIndex,
			fTrackPosition, Staged.m_fDurationTicks))
	{
		return false;
	}
	Staged.m_fTickRate =
		Model->Get_AnimationTickPerSecond(iExpectedAnimationIndex);
	if (!std::isfinite(Staged.m_fTickRate) || Staged.m_fTickRate <= 0.f ||
		!std::isfinite(Staged.m_fDurationTicks) ||
		Staged.m_fDurationTicks <= 0.f)
	{
		return false;
	}
	Staged.m_fDurationSeconds =
		Staged.m_fDurationTicks / Staged.m_fTickRate;
	if (!std::isfinite(Staged.m_fDurationSeconds) ||
		Staged.m_fDurationSeconds <= 0.f ||
		iExpectedTargetGeneration != Resolve_TargetGeneration() ||
		Resolve_Model() != Model)
	{
		return false;
	}

	OutBinding = std::move(Staged);
	return true;
}

bool_t Client::CAnimationTargetService::Sample_HistoricalPose(
	const CAnimationHistoricalPoseBinding& Binding,
	const f32_t fAnimationLocalTimeSeconds,
	ANIMATION_HISTORICAL_POSE_SAMPLE& OutSample)
{
	if (!Binding.Is_Valid() ||
		!std::isfinite(fAnimationLocalTimeSeconds) ||
		fAnimationLocalTimeSeconds < 0.f ||
		fAnimationLocalTimeSeconds > Binding.m_fDurationSeconds ||
		Binding.m_BoneIndices.empty() ||
		Binding.m_iTargetGeneration != Resolve_TargetGeneration())
	{
		return false;
	}

	const shared_ptr<Engine::CModel> Model = Binding.m_Model.lock();
	if (nullptr == Model || Resolve_Model() != Model ||
		Binding.m_iAnimationIndex >= Model->Get_NumAnimations() ||
		(Binding.m_strExplicitClipName.empty() &&
		 Binding.m_iAnimationIndex != Model->Get_CurrentAnimIndex()))
	{
		return false;
	}

	ANIMATION_HISTORICAL_POSE_SAMPLE Staged;
	if (!Resolve_RootTransform(&Staged.RootWorld))
		return false;
	Staged.BoneCombinedMatrices.resize(Binding.m_BoneIndices.size());
	const f32_t fTrackPositionTicks = (std::min)(
		Binding.m_fDurationTicks,
		fAnimationLocalTimeSeconds * Binding.m_fTickRate);
	if (!std::isfinite(fTrackPositionTicks))
		return false;
	const bool_t bSampled = Binding.m_strExplicitClipName.empty() ?
		Model->Sample_CurrentAnimationBoneCombinedMatrices(
			Binding.m_iAnimationIndex,
			fTrackPositionTicks,
			Binding.m_BoneIndices,
			Staged.BoneCombinedMatrices) :
		Model->Sample_AnimationBoneCombinedMatrices(
			Binding.m_strExplicitClipName.c_str(),
			fTrackPositionTicks,
			Binding.m_BoneIndices,
			Staged.BoneCombinedMatrices);
	if (!bSampled ||
		Binding.m_iTargetGeneration != Resolve_TargetGeneration() ||
		Resolve_Model() != Model)
	{
		return false;
	}

	OutSample = std::move(Staged);
	return true;
}
