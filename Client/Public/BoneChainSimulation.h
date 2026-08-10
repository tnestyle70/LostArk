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
whole chain hangs from -- and simulation starts one link below it.

Every link's rest direction is measured inside its parent's already-simulated
frame, not against the animated pose. That is what makes gravity drape a chain:
each link sags a little in a frame that has already sagged, so the tip carries
the sum. A chain whose rest pose is the animation itself can never hang, only
tremble around what the animator keyed. */
struct BONE_CHAIN_SPEC
{
	/* First bone of the chain, the one the animation still drives. The chain
	continues through that bone's descendants in the model's own bone order. */
	const char_t* pRootBoneName = nullptr;
	/* How many bones the chain has including the animated root. */
	uint32_t iBoneCount = 0u;

	/* Fraction of the gap to the animated direction closed per 60Hz step.
	This is what carries the chain along with the body; lower trails longer. */
	f32_t fStiffness = 0.1f;
	/* Fraction of velocity kept each step. High values ring like a spring;
	the drape look wants most of the motion to come from the forces, so keep
	this at half or below. */
	f32_t fDamping = 0.5f;
	/* Metres per second squared along model-space down. Because sag compounds
	through the chain this does not need to fight the stiffness one-to-one;
	values well above real gravity are normal for stylized cloth. */
	f32_t fGravity = 15.f;
	/* Upper bound on how far a link may sit from its rest pose, in metres.
	It is what keeps a fast turn or a teleport from stretching the chain. */
	f32_t fMaxDisplacement = 0.35f;
	/* Acceleration against the travel direction per metre-per-second of
	speed. The velocity feeding it is low-pass filtered, so the cloth leans
	back over a few frames of running and eases home after a stop. */
	f32_t fWindResponse = 0.f;
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
	link. The world position and presentation yaw feed the wind response; the
	solve itself stays in the model's own space. */
	void Update(
		const shared_ptr<Engine::CModel>& pModel,
		f32_t fTimeDelta,
		fvector_t vWorldPosition,
		f32_t fYawDegrees);
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
		/* Model-space simulated position and where it was one step earlier.
		The written pose blends the two by the leftover step time. */
		float3_t vPosition = {};
		float3_t vPreviousPosition = {};
		bool_t isSimulated = false;
	};

private:
	std::vector<BONE_CHAIN_SPEC> m_Specs;
	std::vector<LINK> m_Links;
	/* Frame time left over from the last whole simulation step. */
	f32_t m_fStepAccumulator = 0.f;
	bool_t m_isPrimed = false;
	/* Travel state behind the wind response. The velocity is low-pass
	filtered in world space and rotated into model space each update. */
	float3_t m_vPreviousWorldPosition = {};
	float3_t m_vFilteredWorldVelocity = {};
	bool_t m_hasWorldSample = false;
};

NS_END
