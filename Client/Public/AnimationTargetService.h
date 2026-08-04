#pragma once

#include "Client_Defines.h"
// float4x4_t and the anchor query's math types come from here; the target
// contract used to be pointer-only and did not need it.
#include "Engine_Defines.h"

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
//
// Effect authoring reads the same contract: an effect that will ride a weapon
// socket needs that socket's live world transform, so the anchor query below is
// the only sanctioned way for a tool to place a preview on the target. Tools do
// not walk bones or layers themselves.
class CAnimationTargetService final
{
public:
	static void Bind(const std::shared_ptr<CCharacter>& character);
	static void Unbind(const std::shared_ptr<CCharacter>& character);
	// The preview body is parented to a matrix the preview owner fixes when the
	// asset is selected, so it is copied instead of aliased; a scene character
	// keeps moving and is read from its live transform on every query.
	static void Bind_Preview(
		const std::shared_ptr<Engine::CModel>& model,
		const std::string& assetName,
		const float4x4_t& rootMatrix);
	static void Unbind_Preview(
		const std::shared_ptr<Engine::CModel>& model);

	static std::shared_ptr<CCharacter> Resolve_Character();
	static std::shared_ptr<Engine::CModel> Resolve_Model();
	static std::string Resolve_AssetName();
	static uint64_t Resolve_TargetGeneration();

	// World matrix of the target's root. False when nothing is bound.
	static bool_t Resolve_RootTransform(float4x4_t* pOut);
	// World matrix of a named skeleton anchor on the current target. False when
	// nothing is bound or the target's skeleton has no such bone, so a caller
	// cannot mistake a missing anchor for the origin.
	static bool_t Resolve_AnchorTransform(
		const char_t* pAnchorSlotId,
		float4x4_t* pOut);

private:
	static std::weak_ptr<CCharacter> s_Target;
	static std::weak_ptr<Engine::CModel> s_PreviewModel;
	static std::string s_PreviewAssetName;
	static float4x4_t s_PreviewRootMatrix;
	static uint64_t s_TargetGeneration;
};

}
