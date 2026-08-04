#pragma once

#include "Client_Defines.h"

#include <memory>
#include <string>
#include <cstdint>

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
	static void Bind_Preview(
		const std::shared_ptr<Engine::CModel>& model,
		const std::string& assetName);
	static void Unbind_Preview(
		const std::shared_ptr<Engine::CModel>& model);

	static std::shared_ptr<CCharacter> Resolve_Character();
	static std::shared_ptr<Engine::CModel> Resolve_Model();
	static std::string Resolve_AssetName();
	static uint64_t Resolve_TargetGeneration();

private:
	static std::weak_ptr<CCharacter> s_Target;
	static std::weak_ptr<Engine::CModel> s_PreviewModel;
	static std::string s_PreviewAssetName;
	static uint64_t s_TargetGeneration;
};

}
