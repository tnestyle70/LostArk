#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

/* One authored chain of bones that trails the animation instead of being posed
by it. The source rigs key only the first link of a hair or skirt chain and
leave the rest in bind pose, because the original game solves them at runtime;
this is that solver.

The first link keeps whatever the animation gave it -- it is the shoulder the
whole chain hangs from -- and simulation starts one link below it. */
struct BONE_CHAIN_SPEC
{
	/* First bone of the chain, the one the animation still drives. The chain
	continues through that bone's descendants in the model's own bone order. */
	const char_t* pRootBoneName = nullptr;
	/* How many bones the chain has including the animated root. */
	uint32_t iBoneCount = 0u;

	/* How hard a link is pulled back to where the animation would have put it.
	0 is a free rope, 1 refuses to lag at all. */
	f32_t fStiffness = 0.12f;
	/* Fraction of velocity kept each step. Lower settles sooner. */
	f32_t fDamping = 0.86f;
	/* Metres per second squared along world down. */
	f32_t fGravity = 6.f;
	/* Upper bound on how far a link may sit from its animated pose, in metres.
	It is what keeps a fast turn or a teleport from stretching the chain. */
	f32_t fMaxDisplacement = 0.35f;
};

/* Runs the chains of one model. Bone indices resolve once against the model
that was passed to Initialize; a different model needs its own instance. */
class CBoneChainSimulation final
{
public:
	bool_t Initialize(
		const shared_ptr<Engine::CModel>& pModel,
		const BONE_CHAIN_SPEC* pSpecs,
		uint32_t iNumSpecs);
	/* Call after the model's animation update and before anything reads bone
	matrices for skinning. Does nothing until Initialize resolved at least one
	link. */
	void Update(
		const shared_ptr<Engine::CModel>& pModel,
		f32_t fTimeDelta);
	/* Drops the simulated state so the next update restarts from the animated
	pose. Used when the character is placed rather than moved. */
	void Reset();
	bool_t Is_Active() const { return !m_Links.empty(); }

private:
	struct LINK
	{
		uint32_t iBoneIndex = 0u;
		int32_t iParentLinkIndex = -1;
		uint32_t iSpecIndex = 0u;
		f32_t fRestLength = 0.f;
		/* World-space simulated position and where it was last step. */
		float3_t vPosition = {};
		float3_t vPreviousPosition = {};
		bool_t isSimulated = false;
	};

private:
	std::vector<BONE_CHAIN_SPEC> m_Specs;
	std::vector<LINK> m_Links;
	/* Where the animation had each link at the end of the previous frame. The
	substeps of one frame walk from it to this frame's pose, so a frame that
	runs two steps does not aim both of them at the same target. */
	std::vector<float3_t> m_PreviousAnimatedPositions;
	/* Frame time left over from the last whole simulation step. */
	f32_t m_fStepAccumulator = 0.f;
	bool_t m_hasSettled = false;
};

NS_END
