#pragma once

#include "Client_Defines.h"
// float4x4_t and the anchor query's math types come from here; the target
// contract used to be pointer-only and did not need it.
#include "Engine_Defines.h"

#include <memory>
#include <span>
#include <string>
#include <cstdint>
#include <vector>

namespace Engine
{
class CModel;
}

namespace Client
{

class CCharacter;
class CValtan;

class CAnimationHistoricalPoseBinding final
{
public:
	bool_t Is_Valid() const { return !m_Model.expired(); }
	size_t Get_BoneCount() const { return m_BoneIndices.size(); }
	uint32_t Get_AnimationIndex() const { return m_iAnimationIndex; }
	f32_t Get_DurationSeconds() const { return m_fDurationSeconds; }

private:
	friend class CAnimationTargetService;
	std::weak_ptr<Engine::CModel> m_Model;
	std::vector<uint32_t> m_BoneIndices;
	uint64_t m_iTargetGeneration = 0u;
	uint32_t m_iAnimationIndex = UINT32_MAX;
	/* Empty keeps the original current-clip binding contract. */
	std::string m_strExplicitClipName;
	f32_t m_fTickRate = 0.f;
	f32_t m_fDurationTicks = 0.f;
	f32_t m_fDurationSeconds = 0.f;
};

struct ANIMATION_HISTORICAL_POSE_SAMPLE final
{
	float4x4_t RootWorld{};
	std::vector<float4x4_t> BoneCombinedMatrices;
};

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
	/* Playable preview targets are complete CCharacter clones, not isolated
	   body models. Keeping the character here makes tool queries see the same
	   body/PartObject owner and live root transform as Character Select. */
	static void Bind_Preview(
		const std::shared_ptr<CCharacter>& character);
	/* Boss preview targets keep their complete product composition so model
	   playback, weapon sockets and effect anchors share one owner. */
	static void Bind_Preview(
		const std::shared_ptr<CValtan>& valtan,
		const std::string& assetName);
	static void Unbind_Preview(
		const std::shared_ptr<Engine::CModel>& model);
	static void Unbind_Preview(
		const std::shared_ptr<CCharacter>& character);
	static void Unbind_Preview(
		const std::shared_ptr<CValtan>& valtan);
	// Clears a preview whose weak model may already have expired. This still
	// advances the generation so pending cross-tool transfers cannot bind to a
	// different target with the same asset name.
	static void Clear_Preview();

	static std::shared_ptr<CCharacter> Resolve_Character();
	/* Resolve_Character() prefers a selected playable preview. This accessor is
	   only for positioning a new preview beside the actual scene character. */
	static std::shared_ptr<CCharacter> Resolve_SceneCharacter();
	/* The bound boss preview target, for Debug tooling that drives the boss
	   object itself. Null when the current target is not a boss. */
	static std::shared_ptr<CValtan> Resolve_Boss();
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

	/* Pins one target generation, current clip and ordered bone-name list.  The
	   resulting binding samples clip-local bone time without seeking the live
	   model. RootWorld is the current resolved actor/preview root at call time;
	   this contract does not fabricate historical moving-actor root poses. */
	static bool_t Prepare_HistoricalPoseBinding(
		uint64_t iExpectedTargetGeneration,
		uint32_t iExpectedAnimationIndex,
		std::span<const std::string> BoneNames,
		CAnimationHistoricalPoseBinding& OutBinding);
	/* Pins an explicit clip on the same target without changing its current
	   animation. Multi-stage history uses clip-local time; each named clip is
	   sampled from rest without the unrelated live transition blend. */
	static bool_t Prepare_HistoricalClipPoseBinding(
		uint64_t iExpectedTargetGeneration,
		const std::string& strClipName,
		std::span<const std::string> BoneNames,
		CAnimationHistoricalPoseBinding& OutBinding);
	static bool_t Sample_HistoricalPose(
		const CAnimationHistoricalPoseBinding& Binding,
		f32_t fAnimationLocalTimeSeconds,
		ANIMATION_HISTORICAL_POSE_SAMPLE& OutSample);

private:
	static bool_t Prepare_HistoricalPoseBindingForAnimation(
		uint64_t iExpectedTargetGeneration,
		uint32_t iExpectedAnimationIndex,
		const std::string& strExplicitClipName,
		std::span<const std::string> BoneNames,
		CAnimationHistoricalPoseBinding& OutBinding);
	static std::weak_ptr<CCharacter> s_Target;
	static std::weak_ptr<CCharacter> s_PreviewCharacter;
	static std::weak_ptr<CValtan> s_PreviewBoss;
	static std::weak_ptr<Engine::CModel> s_PreviewModel;
	static std::string s_PreviewAssetName;
	static float4x4_t s_PreviewRootMatrix;
	static uint64_t s_TargetGeneration;
};

}
