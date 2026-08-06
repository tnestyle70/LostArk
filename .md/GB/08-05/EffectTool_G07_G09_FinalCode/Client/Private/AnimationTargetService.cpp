#include "AnimationTargetService.h"

#include "Character.h"

weak_ptr<Client::CCharacter> Client::CAnimationTargetService::s_Target;
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

	s_PreviewModel = model;
	s_PreviewAssetName = assetName;
	s_PreviewRootMatrix = rootMatrix;
	Advance_TargetGeneration(s_TargetGeneration);
}

void Client::CAnimationTargetService::Unbind_Preview(
	const shared_ptr<Engine::CModel>& model)
{
	if (s_PreviewModel.lock() == model)
		Clear_Preview();
}

void Client::CAnimationTargetService::Clear_Preview()
{
	if (s_PreviewModel.expired() && s_PreviewAssetName.empty())
		return;
	s_PreviewModel.reset();
	s_PreviewAssetName.clear();
	XMStoreFloat4x4(&s_PreviewRootMatrix, XMMatrixIdentity());
	Advance_TargetGeneration(s_TargetGeneration);
}

shared_ptr<Client::CCharacter>
Client::CAnimationTargetService::Resolve_Character()
{
	return s_Target.lock();
}

shared_ptr<Engine::CModel>
Client::CAnimationTargetService::Resolve_Model()
{
	const shared_ptr<Engine::CModel> preview = s_PreviewModel.lock();
	if (nullptr != preview)
		return preview;

	const shared_ptr<CCharacter> character = Resolve_Character();
	return nullptr == character ? nullptr : character->Get_BodyModel();
}

string Client::CAnimationTargetService::Resolve_AssetName()
{
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
