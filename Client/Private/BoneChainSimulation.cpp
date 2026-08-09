#include "BoneChainSimulation.h"

#include "Model.h"

#include <cmath>

namespace
{
	/* The solver runs on its own fixed step so a frame spike cannot make the
	chain explode, and drops the surplus rather than catching up unbounded. */
	constexpr f32_t SIMULATION_STEP_SECONDS = 1.f / 60.f;
	constexpr uint32_t MAX_STEPS_PER_FRAME = 4u;
	constexpr f32_t MINIMUM_REST_LENGTH = 0.0001f;
	/* Fraction of the displacement limit that stays linear. Past it the travel
	compresses toward the limit instead of stopping against it. */
	constexpr f32_t SOFT_LIMIT_KNEE = 0.45f;
	/* Below a millimetre of both offset and travel a link is treated as home.
	Skinning cannot show less than this and the solver should not spend frames
	chasing it. */
	constexpr f32_t SETTLE_DISTANCE = 0.001f;

	bool_t Is_Finite(fmatrix_t Matrix)
	{
		float4x4_t Stored{};
		XMStoreFloat4x4(&Stored, Matrix);
		const f32_t* pValues = &Stored._11;
		for (int32_t i = 0; i < 16; ++i)
		{
			if (!std::isfinite(pValues[i]))
				return false;
		}
		return true;
	}

}

bool_t Client::CBoneChainSimulation::Initialize(
	const shared_ptr<Engine::CModel>& pModel,
	const BONE_CHAIN_SPEC* pSpecs,
	const uint32_t iNumSpecs)
{
	m_Specs.clear();
	m_Links.clear();
	m_PreviousAnimatedPositions.clear();
	m_fStepAccumulator = 0.f;
	m_hasSettled = false;
	if (nullptr == pModel || nullptr == pSpecs || 0u == iNumSpecs)
		return false;

	m_Specs.assign(pSpecs, pSpecs + iNumSpecs);

	for (uint32_t iSpec = 0u; iSpec < iNumSpecs; ++iSpec)
	{
		const BONE_CHAIN_SPEC& Spec = m_Specs[iSpec];
		if (nullptr == Spec.pRootBoneName || Spec.iBoneCount < 2u)
			continue;
		const int32_t iRootBone = pModel->Find_BoneIndex(Spec.pRootBoneName);
		if (iRootBone < 0)
			continue;

		/* The rig numbers a chain's bones consecutively, so the links after the
		root are the next bones that still descend from the one before them. A
		gap means the chain ended earlier than the spec claims, and the rest of
		the spec is dropped rather than grabbing an unrelated bone. */
		int32_t iPreviousBone = iRootBone;
		int32_t iPreviousLink = -1;
		for (uint32_t iLink = 0u; iLink < Spec.iBoneCount; ++iLink)
		{
			const int32_t iBone = iRootBone + static_cast<int32_t>(iLink);
			if (0u != iLink &&
				pModel->Get_BoneParentIndex(static_cast<uint32_t>(iBone)) !=
					iPreviousBone)
			{
				break;
			}
			LINK Link{};
			Link.iBoneIndex = static_cast<uint32_t>(iBone);
			Link.iParentLinkIndex = iPreviousLink;
			Link.iSpecIndex = iSpec;
			/* The root stays wherever the animation puts it: it is the anchor
			the source still keys. */
			Link.isSimulated = 0u != iLink;
			iPreviousLink = static_cast<int32_t>(m_Links.size());
			iPreviousBone = iBone;
			m_Links.push_back(Link);
		}
	}
	return !m_Links.empty();
}

void Client::CBoneChainSimulation::Reset()
{
	m_PreviousAnimatedPositions.clear();
	m_fStepAccumulator = 0.f;
	m_hasSettled = false;
}

void Client::CBoneChainSimulation::Update(
	const shared_ptr<Engine::CModel>& pModel,
	const f32_t fTimeDelta)
{
	if (nullptr == pModel || m_Links.empty() ||
		!std::isfinite(fTimeDelta) || fTimeDelta <= 0.f)
	{
		return;
	}
	/* Everything solves in the character's own space, not the world's.

	The world transform is replicated: position eases toward a 30Hz snapshot and
	yaw turns at a fixed rate, so neither advances at a constant speed. Solving
	in world space feeds all of that unevenness into the chain as acceleration,
	and no amount of damping fixes an input that shakes. In character space the
	chain answers only to the animation, which is smooth because it was authored
	that way. The cost is that the cloth no longer trails the character's own
	travel -- for plate armour that reads better anyway. */
	std::vector<float3_t> AnimatedPositions(m_Links.size());
	std::vector<matrix_t> AnimatedCombined(m_Links.size());
	for (size_t i = 0; i < m_Links.size(); ++i)
	{
		matrix_t Combined{};
		if (!pModel->Get_BoneCombinedMatrix(m_Links[i].iBoneIndex, Combined))
			return;
		AnimatedCombined[i] = Combined;
		XMStoreFloat3(&AnimatedPositions[i], Combined.r[3]);
	}
	/* Rest length is read from this frame's pose, not frozen at startup. The
	cook keys translation as well as rotation, so the distance between two
	joints is not the same in every clip; holding one clip's length would make
	the constraint pull against the spring forever, which never settles and
	reads as a permanent tremble. */
	for (size_t i = 0; i < m_Links.size(); ++i)
	{
		const int32_t iParent = m_Links[i].iParentLinkIndex;
		m_Links[i].fRestLength = iParent < 0 ? 0.f :
			XMVectorGetX(XMVector3Length(
				XMLoadFloat3(&AnimatedPositions[i]) -
				XMLoadFloat3(&AnimatedPositions[iParent])));
	}
	if (!m_hasSettled ||
		m_PreviousAnimatedPositions.size() != AnimatedPositions.size())
	{
		for (size_t i = 0; i < m_Links.size(); ++i)
		{
			m_Links[i].vPosition = AnimatedPositions[i];
			m_Links[i].vPreviousPosition = AnimatedPositions[i];
		}
		m_PreviousAnimatedPositions = AnimatedPositions;
		m_hasSettled = true;
		return;
	}

	/* Leftover time carries to the next frame instead of being rounded up to a
	whole step. Rounding up runs a full step every frame, so above 60fps the
	chain was integrated faster than real time -- more travel per second than
	the tuning asks for, and enough extra energy to ring. */
	m_fStepAccumulator += fTimeDelta;
	uint32_t iSteps = static_cast<uint32_t>(
		m_fStepAccumulator / SIMULATION_STEP_SECONDS);
	if (iSteps > MAX_STEPS_PER_FRAME)
	{
		iSteps = MAX_STEPS_PER_FRAME;
		m_fStepAccumulator = 0.f;
	}
	else
	{
		m_fStepAccumulator -=
			static_cast<f32_t>(iSteps) * SIMULATION_STEP_SECONDS;
	}
	/* A frame with no whole step still writes the pose it already has. Falling
	back to the animated pose for that one frame would flicker between the two
	on any machine running faster than the step rate. */

	for (uint32_t iStep = 0u; iStep < iSteps; ++iStep)
	{
		/* Walk the target from where the animation was to where it is, so each
		substep of a frame gets its own share of the movement. Aiming every
		substep at the frame's final pose kicks the chain once per step instead
		of once per frame, which is a tremble that scales with how fast the
		character is moving and vanishes when it stands still. */
		const f32_t fAlpha =
			static_cast<f32_t>(iStep + 1u) / static_cast<f32_t>(iSteps);
		for (size_t i = 0; i < m_Links.size(); ++i)
		{
			LINK& Link = m_Links[i];
			const BONE_CHAIN_SPEC& Spec = m_Specs[Link.iSpecIndex];
			const vector_t vAnimated = XMVectorLerp(
				XMLoadFloat3(&m_PreviousAnimatedPositions[i]),
				XMLoadFloat3(&AnimatedPositions[i]),
				fAlpha);
			if (!Link.isSimulated || Link.iParentLinkIndex < 0)
			{
				XMStoreFloat3(&Link.vPosition, vAnimated);
				Link.vPreviousPosition = Link.vPosition;
				continue;
			}

			const vector_t vCurrent = XMLoadFloat3(&Link.vPosition);
			const vector_t vPrevious = XMLoadFloat3(&Link.vPreviousPosition);

			/* Verlet: the step carries its own velocity, so damping and the
			pull toward the animated pose are the only things acting on it. */
			vector_t vNext = vCurrent +
				(vCurrent - vPrevious) * Spec.fDamping +
				(vAnimated - vCurrent) * Spec.fStiffness;
			vNext -= XMVectorSet(0.f, 1.f, 0.f, 0.f) * Spec.fGravity *
				SIMULATION_STEP_SECONDS * SIMULATION_STEP_SECONDS;

			/* Length is what makes it a chain rather than a cloud of points. */
			const vector_t vParent =
				XMLoadFloat3(&m_Links[Link.iParentLinkIndex].vPosition);
			vector_t vOffset = vNext - vParent;
			const f32_t fLength = XMVectorGetX(XMVector3Length(vOffset));
			if (fLength > MINIMUM_REST_LENGTH && Link.fRestLength > 0.f)
			{
				vOffset = XMVector3Normalize(vOffset) * Link.fRestLength;
				vNext = vParent + vOffset;
			}

			/* However far it lags, it never leaves the silhouette: a teleport or
			a hard turn would otherwise stretch the chain across the map.

			The limit compresses instead of cutting. Clipping the position dead
			at the boundary is what reads as a taut rope hitting its end; easing
			into it lets the last stretch of travel slow down and settle. The
			curve saturates, so the limit is still never crossed. */
			const vector_t vFromAnimated = vNext - vAnimated;
			const f32_t fDisplacement =
				XMVectorGetX(XMVector3Length(vFromAnimated));
			const f32_t fKnee = Spec.fMaxDisplacement * SOFT_LIMIT_KNEE;
			if (fDisplacement > fKnee && fDisplacement > MINIMUM_REST_LENGTH)
			{
				const f32_t fRange = Spec.fMaxDisplacement - fKnee;
				const f32_t fOver = fDisplacement - fKnee;
				const f32_t fEased = fRange <= 0.f ? 0.f :
					fRange * (fOver / (fOver + fRange));
				vNext = vAnimated +
					XMVector3Normalize(vFromAnimated) * (fKnee + fEased);
			}

			/* Once a link is this close to its animated pose and barely moving,
			it is parked outright. Without it the spring and the length
			constraint trade a fraction of a millimetre back and forth forever,
			which on screen is a permanent shimmer rather than motion. */
			const f32_t fResidual = XMVectorGetX(
				XMVector3Length(vNext - vAnimated));
			const f32_t fTravel = XMVectorGetX(
				XMVector3Length(vNext - vCurrent));
			if (fResidual < SETTLE_DISTANCE && fTravel < SETTLE_DISTANCE)
			{
				XMStoreFloat3(&Link.vPosition, vAnimated);
				Link.vPreviousPosition = Link.vPosition;
				continue;
			}

			/* One non-finite position would spread through the rest length,
			the aim direction and finally the bone matrix. The link falls back
			to its animated pose and the chain carries on from there. */
			if (!XMVector3IsNaN(vNext) && !XMVector3IsInfinite(vNext))
			{
				Link.vPreviousPosition = Link.vPosition;
				XMStoreFloat3(&Link.vPosition, vNext);
			}
			else
			{
				XMStoreFloat3(&Link.vPosition, vAnimated);
				Link.vPreviousPosition = Link.vPosition;
			}
		}
	}

	/* Write back as rotation only. A bone's translation is its rest offset from
	its parent and moving it would tear the mesh, so each parent is turned to
	aim at where its child ended up instead.

	Turning a bone carries everything below it, so a chain has to be written from
	its root down with the combined matrix rebuilt as it goes; reading the
	animated pose for a deep link would aim it from a parent that has already
	moved. */
	m_PreviousAnimatedPositions = AnimatedPositions;

	matrix_t AboveCombined = XMMatrixIdentity();
	matrix_t ParentLocal = XMMatrixIdentity();
	uint32_t iParentBone = 0u;
	for (size_t i = 0; i < m_Links.size(); ++i)
	{
		const LINK& Link = m_Links[i];
		if (Link.iParentLinkIndex < 0)
		{
			/* Chain root. Its position stays animated -- only its children move
			-- but its rotation is what aims the first simulated link, so the
			frame above it has to be recovered to rebuild it later. */
			matrix_t RootLocal{};
			if (!pModel->Get_BoneLocalMatrix(Link.iBoneIndex, RootLocal))
				return;
			/* Recovering the frame above the root needs the root's own matrix
			inverted, and a rig bone with no scale is singular: the inverse comes
			back as infinities and every bone below inherits them. */
			vector_t vDeterminant = XMMatrixDeterminant(RootLocal);
			if (std::fabs(XMVectorGetX(vDeterminant)) < MINIMUM_REST_LENGTH)
			{
				ParentLocal = RootLocal;
				iParentBone = Link.iBoneIndex;
				AboveCombined = XMMatrixIdentity();
				continue;
			}
			AboveCombined = XMMatrixInverse(&vDeterminant, RootLocal) *
				AnimatedCombined[i];
			ParentLocal = RootLocal;
			iParentBone = Link.iBoneIndex;
			continue;
		}

		matrix_t ChildLocal{};
		if (!pModel->Get_BoneLocalMatrix(Link.iBoneIndex, ChildLocal))
			return;
		const matrix_t ParentCombined = ParentLocal * AboveCombined;
		const matrix_t ChildCombined = ChildLocal * ParentCombined;

		const vector_t vParentPosition = ParentCombined.r[3];
		const vector_t vChildCurrent = ChildCombined.r[3];
		const vector_t vChildTarget = XMLoadFloat3(&Link.vPosition);

		vector_t vFrom = vChildCurrent - vParentPosition;
		vector_t vTo = vChildTarget - vParentPosition;
		matrix_t ParentResult = ParentLocal;
		if (XMVectorGetX(XMVector3LengthSq(vFrom)) >= MINIMUM_REST_LENGTH &&
			XMVectorGetX(XMVector3LengthSq(vTo)) >= MINIMUM_REST_LENGTH)
		{
			vFrom = XMVector3Normalize(vFrom);
			vTo = XMVector3Normalize(vTo);
			const f32_t fDot = XMVectorGetX(XMVector3Dot(vFrom, vTo));
			vector_t vAxis = XMVector3Cross(vFrom, vTo);
			if (fDot <= 0.99999f &&
				XMVectorGetX(XMVector3LengthSq(vAxis)) >= MINIMUM_REST_LENGTH)
			{
				vAxis = XMVector3Normalize(vAxis);
				const f32_t fAngle = std::acos(
					fDot < -1.f ? -1.f : (fDot > 1.f ? 1.f : fDot));
				const vector_t vTranslation = ParentResult.r[3];
				ParentResult.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
				ParentResult = ParentResult *
					XMMatrixRotationNormal(vAxis, fAngle);
				ParentResult.r[3] = vTranslation;
				/* A single non-finite value here would be skinned into every
				vertex the bone touches and taken downstream as geometry. The
				bone keeps its animated pose instead. */
				if (!Is_Finite(ParentResult))
				{
					ParentResult = ParentLocal;
				}
				else
				{
					pModel->Set_BoneLocalMatrix(iParentBone, ParentResult);
				}
			}
		}

		/* Descend: this link becomes the parent of the next one, hanging from
		the frame the correction just produced. */
		AboveCombined = ParentResult * AboveCombined;
		ParentLocal = ChildLocal;
		iParentBone = Link.iBoneIndex;
	}
	pModel->Refresh_BoneCombinedMatrices();
}
