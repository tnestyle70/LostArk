#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "FaceSliderDocument.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

/* Applies the retail face customizing sliders to a character's skeleton. Each
slider is an additive pose with three keys (extreme / neutral / extreme); the
slider weight 0..1 walks those keys, so 0.5 is the untouched face. The result
is written through CModel::Set_BoneLocalMatrix after the animation posed the
skeleton, the same secondary-motion seam the bone chains use.

Bone locals are composed as delta * base, where base is the local matrix the
animation (or the rest pose, for bones no body clip keys) produced this frame.
The base is re-read whenever the bone's local differs from what this class wrote
last, so a facial clip that keys the same bone still drives it. */
class CFaceCustomizeApplier final
{
public:
	/* Resolves every bone name against the model. A slider whose bones are all
	missing is dropped; a missing bone inside an otherwise valid slider is
	skipped. Returns false when no slider survived. */
	bool_t Initialize(
		const shared_ptr<Engine::CModel>& pModel,
		const FACE_SLIDER_DOCUMENT& Document);

	size_t Get_SliderCount() const { return m_Sliders.size(); }
	const std::string& Get_SliderId(size_t iSlider) const;
	f32_t Get_Weight(size_t iSlider) const;
	bool_t Set_Weight(size_t iSlider, f32_t fWeight);
	void Reset_Weights();

	/* Call once per frame after the animation update and before anything
	reads bone matrices for skinning. Refreshes the combined matrices itself.
	Does nothing until Initialize succeeded. */
	void Apply(const shared_ptr<Engine::CModel>& pModel);

private:
	struct BONE_STATE
	{
		uint32_t iBoneIndex = 0u;
		float4x4_t Base{};
		float4x4_t LastWritten{};
		bool_t hasLastWritten = false;
	};
	struct SLIDER_BONE
	{
		size_t iBoneState = 0u;
		std::array<FACE_SLIDER_KEY, FACE_SLIDER_KEY_COUNT> Keys{};
	};
	struct SLIDER
	{
		std::string strId;
		std::vector<SLIDER_BONE> Bones;
		f32_t fWeight = 0.5f;
	};

	size_t Find_Or_Add_BoneState(uint32_t iBoneIndex);
	static matrix_t Evaluate_Delta(
		const std::array<FACE_SLIDER_KEY, FACE_SLIDER_KEY_COUNT>& Keys, f32_t fWeight);

private:
	std::vector<SLIDER> m_Sliders;
	std::vector<BONE_STATE> m_BoneStates;
	/* Per-frame scratch: accumulated delta per bone state, reused to avoid
	allocating in the update loop. */
	std::vector<float4x4_t> m_AccumulatedDeltas;
	std::vector<bool_t> m_AccumulatedTouched;
};

NS_END
