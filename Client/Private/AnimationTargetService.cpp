#include "AnimationTargetService.h"

#include "Character.h"

weak_ptr<Client::CCharacter> Client::CAnimationTargetService::s_Target;

void Client::CAnimationTargetService::Bind(
	const shared_ptr<CCharacter>& character)
{
	s_Target = character;
}

void Client::CAnimationTargetService::Unbind(
	const shared_ptr<CCharacter>& character)
{
	if (s_Target.lock() == character)
		s_Target.reset();
}

shared_ptr<Client::CCharacter>
Client::CAnimationTargetService::Resolve_Character()
{
	return s_Target.lock();
}

shared_ptr<Engine::CModel>
Client::CAnimationTargetService::Resolve_Model()
{
	const shared_ptr<CCharacter> character = Resolve_Character();
	return nullptr == character ? nullptr : character->Get_BodyModel();
}
