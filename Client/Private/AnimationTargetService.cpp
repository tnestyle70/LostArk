#include "AnimationTargetService.h"

#include "Character.h"

weak_ptr<Client::CCharacter> Client::CAnimationTargetService::s_Target;
weak_ptr<Engine::CModel> Client::CAnimationTargetService::s_PreviewModel;
string Client::CAnimationTargetService::s_PreviewAssetName;
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
	const string& assetName)
{
	if (s_PreviewModel.lock() == model &&
		s_PreviewAssetName == assetName)
	{
		return;
	}

	s_PreviewModel = model;
	s_PreviewAssetName = assetName;
	Advance_TargetGeneration(s_TargetGeneration);
}

void Client::CAnimationTargetService::Unbind_Preview(
	const shared_ptr<Engine::CModel>& model)
{
	if (s_PreviewModel.lock() == model)
	{
		s_PreviewModel.reset();
		s_PreviewAssetName.clear();
		Advance_TargetGeneration(s_TargetGeneration);
	}
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
