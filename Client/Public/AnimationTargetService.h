#pragma once

#include "Client_Defines.h"

#include <memory>

namespace Engine
{
class CModel;
}

namespace Client
{

class CCharacter;

// The active scene publishes an editable character. Animation tooling consumes
// this contract and never searches a level/layer/part/index by convention.
class CAnimationTargetService final
{
public:
	static void Bind(const std::shared_ptr<CCharacter>& character);
	static void Unbind(const std::shared_ptr<CCharacter>& character);

	static std::shared_ptr<CCharacter> Resolve_Character();
	static std::shared_ptr<Engine::CModel> Resolve_Model();

private:
	static std::weak_ptr<CCharacter> s_Target;
};

}
