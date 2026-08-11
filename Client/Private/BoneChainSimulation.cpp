#include "BoneChainSimulation.h"

#include "Model.h"

#include <cmath>

namespace
{
	/* The solver runs on its own fixed step so a frame spike cannot make the
	chain explode, and drops the surplus rather than catching up unbounded. */
	constexpr f32_t SIMULATION_STEP_SECONDS = 1.f / 60.f;
	constexpr f32_t STEP_SECONDS_SQUARED =
		SIMULATION_STEP_SECONDS * SIMULATION_STEP_SECONDS;
	constexpr uint32_t MAX_STEPS_PER_FRAME = 4u;
	constexpr f32_t MINIMUM_REST_LENGTH = 0.0001f;
	/* Fraction of the displacement limit that stays linear. Past it the travel
	compresses toward the limit instead of stopping against it. */
	constexpr f32_t SOFT_LIMIT_KNEE = 0.45f;
	/* Below a millimetre of both offset and travel a link is treated as home.
	Skinning cannot show less than this and the solver should not spend frames
	chasing it. */
	constexpr f32_t SETTLE_DISTANCE = 0.001f;
	/* Above this speed a frame is a teleport, not travel, and feeding it to
	the wind would slap the cloth across the map. */
	constexpr f32_t TELEPORT_SPEED = 15.f;
	/* Per-second blend rate of the wind's velocity filter; about a fifth of a
	second to lean into a run and the same to ease home after a stop. */
	constexpr f32_t VELOCITY_FILTER_RATE = 5.f;

	bool_t Is_Finite(const matrix_t& Matrix)
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

	/* Bends a frame about its own origin so the direction toward vFromPoint
	turns onto the direction toward vToPoint. The axis lives in model space,
	which is the space the frame maps into, so composing on the right is the
	correct side. Returns the frame unchanged when the rotation is degenerate
	or not finite. */
	matrix_t Aim_Frame(
		const matrix_t& Frame,
		fvector_t vFromPoint,
		fvector_t vToPoint)
	{
		const vector_t vPivot = Frame.r[3];
		vector_t vFrom = vFromPoint - vPivot;
		vector_t vTo = vToPoint - vPivot;
		if (XMVectorGetX(XMVector3LengthSq(vFrom)) < MINIMUM_REST_LENGTH ||
			XMVectorGetX(XMVector3LengthSq(vTo)) < MINIMUM_REST_LENGTH)
		{
			return Frame;
		}
		vFrom = XMVector3Normalize(vFrom);
		vTo = XMVector3Normalize(vTo);
		const f32_t fDot = XMVectorGetX(XMVector3Dot(vFrom, vTo));
		vector_t vAxis = XMVector3Cross(vFrom, vTo);
		if (fDot > 0.99999f ||
			XMVectorGetX(XMVector3LengthSq(vAxis)) < MINIMUM_REST_LENGTH)
		{
			return Frame;
		}
		const f32_t fAngle = std::acos(
			fDot < -1.f ? -1.f : (fDot > 1.f ? 1.f : fDot));
		matrix_t Result = Frame;
		Result.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		Result = Result * XMMatrixRotationNormal(
			XMVector3Normalize(vAxis), fAngle);
		Result.r[3] = vPivot;
		return Is_Finite(Result) ? Result : Frame;
	}
}

bool_t Client::CBoneChainSimulation::Initialize(
	const shared_ptr<Engine::CModel>& pModel,
	const BONE_CHAIN_SPEC* pSpecs,
	const uint32_t iNumSpecs)
{
	m_Specs.clear();
	m_Links.clear();
	m_fStepAccumulator = 0.f;
	m_isPrimed = false;
	m_vFilteredWorldVelocity = {};
	m_hasWorldSample = false;
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
	m_fStepAccumulator = 0.f;
	m_isPrimed = false;
	m_vFilteredWorldVelocity = {};
	m_hasWorldSample = false;
}

void Client::CBoneChainSimulation::Update(
	const shared_ptr<Engine::CModel>& pModel,
	const f32_t fTimeDelta,
	const fvector_t vWorldPosition,
	const f32_t fYawDegrees)
{
	if (nullptr == pModel || m_Links.empty() ||
		!std::isfinite(fTimeDelta) || fTimeDelta <= 0.f)
	{
		return;
	}

	/* The wind is the character's own travel, low-pass filtered so the 30Hz
	snapshot ease and the odd frame spike do not shake the cloth, and rotated
	into model space so the solve itself never sees the world transform. */
	if (m_hasWorldSample)
	{
		vector_t vVelocity = (vWorldPosition -
			XMLoadFloat3(&m_vPreviousWorldPosition)) / fTimeDelta;
		const f32_t fSpeed = XMVectorGetX(XMVector3Length(vVelocity));
		if (!std::isfinite(fSpeed) || fSpeed > TELEPORT_SPEED)
			vVelocity = XMLoadFloat3(&m_vFilteredWorldVelocity);
		const f32_t fBlend = fTimeDelta * VELOCITY_FILTER_RATE > 1.f ?
			1.f : fTimeDelta * VELOCITY_FILTER_RATE;
		XMStoreFloat3(&m_vFilteredWorldVelocity,
			XMVectorLerp(XMLoadFloat3(&m_vFilteredWorldVelocity),
				vVelocity, fBlend));
	}
	XMStoreFloat3(&m_vPreviousWorldPosition, vWorldPosition);
	m_hasWorldSample = true;
	const vector_t vVelocityModel = XMVector3TransformNormal(
		XMLoadFloat3(&m_vFilteredWorldVelocity),
		XMMatrixRotationY(XMConvertToRadians(-fYawDegrees)));

	/* The frame's inputs: every link's animated local matrix, and the animated
	combined matrix of each chain root. Everything else is derived by walking
	the chain, so a link's rest pose rides on the frames the links above it
	already produced -- which is what lets sag and lag accumulate into an
	actual drape instead of staying a per-link tremble. */
	std::vector<matrix_t> Locals(m_Links.size());
	std::vector<matrix_t> RootCombined(m_Links.size());
	for (size_t i = 0; i < m_Links.size(); ++i)
	{
		if (!pModel->Get_BoneLocalMatrix(m_Links[i].iBoneIndex, Locals[i]))
			return;
		if (m_Links[i].iParentLinkIndex < 0 &&
			!pModel->Get_BoneCombinedMatrix(
				m_Links[i].iBoneIndex, RootCombined[i]))
		{
			return;
		}
	}

	/* First update after a spawn or reset: place every link on the animated
	chain and let motion build from there. */
	if (!m_isPrimed)
	{
		matrix_t Frame = XMMatrixIdentity();
		for (size_t i = 0; i < m_Links.size(); ++i)
		{
			Frame = m_Links[i].iParentLinkIndex < 0 ?
				RootCombined[i] : Locals[i] * Frame;
			XMStoreFloat3(&m_Links[i].vPosition, Frame.r[3]);
			m_Links[i].vPreviousPosition = m_Links[i].vPosition;
		}
		m_isPrimed = true;
		return;
	}

	/* Leftover time carries to the next frame instead of being rounded up to a
	whole step; rounding up would integrate faster than real time above 60fps. */
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

	/* Gravity plus the wind, per spec. These are the forces that shape the
	chain; the spring only carries it along with the body. */
	std::vector<vector_t> Accelerations(m_Specs.size());
	for (size_t iSpec = 0; iSpec < m_Specs.size(); ++iSpec)
	{
		Accelerations[iSpec] =
			XMVectorSet(0.f, -m_Specs[iSpec].fGravity, 0.f, 0.f) -
			vVelocityModel * m_Specs[iSpec].fWindResponse;
	}

	for (uint32_t iStep = 0u; iStep < iSteps; ++iStep)
	{
		/* ParentFrame is the frame of the previous link with its animated
		local rotation, hanging under everything the walk has already bent.
		The bend a link causes in its parent is decided here too, so the next
		link starts from it -- the compounding this solver is built around. */
		matrix_t ParentFrame = XMMatrixIdentity();
		for (size_t i = 0; i < m_Links.size(); ++i)
		{
			LINK& Link = m_Links[i];
			if (Link.iParentLinkIndex < 0)
			{
				ParentFrame = RootCombined[i];
				continue;
			}
			const BONE_CHAIN_SPEC& Spec = m_Specs[Link.iSpecIndex];
			const matrix_t SelfFrame = Locals[i] * ParentFrame;
			const vector_t vTarget = SelfFrame.r[3];
			const vector_t vParentPosition = ParentFrame.r[3];
			const f32_t fRestLength = XMVectorGetX(
				XMVector3Length(vTarget - vParentPosition));
			const vector_t vCurrent = XMLoadFloat3(&Link.vPosition);
			const vector_t vPrevious =
				XMLoadFloat3(&Link.vPreviousPosition);

			/* Verlet: carried velocity, the pull toward the animated
			direction, and the forces. */
			vector_t vNext = vCurrent +
				(vCurrent - vPrevious) * Spec.fDamping +
				(vTarget - vCurrent) * Spec.fStiffness +
				Accelerations[Link.iSpecIndex] * STEP_SECONDS_SQUARED;

			/* Length is what makes it a chain rather than a cloud of
			points. */
			const vector_t vOffset = vNext - vParentPosition;
			const f32_t fLength = XMVectorGetX(XMVector3Length(vOffset));
			if (fLength > MINIMUM_REST_LENGTH &&
				fRestLength > MINIMUM_REST_LENGTH)
			{
				vNext = vParentPosition +
					XMVector3Normalize(vOffset) * fRestLength;
			}

			/* However far it lags, it never leaves the silhouette; the limit
			compresses toward its bound instead of cutting at it. */
			const vector_t vFromRest = vNext - vTarget;
			const f32_t fDisplacement =
				XMVectorGetX(XMVector3Length(vFromRest));
			const f32_t fKnee = Spec.fMaxDisplacement * SOFT_LIMIT_KNEE;
			if (fDisplacement > fKnee && fDisplacement > MINIMUM_REST_LENGTH)
			{
				const f32_t fRange = Spec.fMaxDisplacement - fKnee;
				const f32_t fOver = fDisplacement - fKnee;
				const f32_t fEased = fRange <= 0.f ? 0.f :
					fRange * (fOver / (fOver + fRange));
				vNext = vTarget +
					XMVector3Normalize(vFromRest) * (fKnee + fEased);
			}

			/* Within a millimetre of home and barely moving, a link parks
			outright; anything less shimmers forever. */
			const f32_t fResidual = XMVectorGetX(
				XMVector3Length(vNext - vTarget));
			const f32_t fTravel = XMVectorGetX(
				XMVector3Length(vNext - vCurrent));
			if (fResidual < SETTLE_DISTANCE && fTravel < SETTLE_DISTANCE)
			{
				XMStoreFloat3(&Link.vPosition, vTarget);
				Link.vPreviousPosition = Link.vPosition;
			}
			else if (!XMVector3IsNaN(vNext) && !XMVector3IsInfinite(vNext))
			{
				Link.vPreviousPosition = Link.vPosition;
				XMStoreFloat3(&Link.vPosition, vNext);
			}
			else
			{
				XMStoreFloat3(&Link.vPosition, vTarget);
				Link.vPreviousPosition = Link.vPosition;
			}

			/* Bend the parent onto the solved position and hang this link's
			own animated local under it for the next iteration. */
			ParentFrame = Aim_Frame(
				ParentFrame, vTarget, XMLoadFloat3(&Link.vPosition));
			ParentFrame = Locals[i] * ParentFrame;
		}
	}

	/* Write-back walk. The rotations are rebuilt from the solved positions,
	blended between the last two steps by the leftover step time so the chain
	advances at the frame rate rather than the step rate. Each parent's local
	is recovered through the frame above it; no determinant threshold guards
	the inverses, because the rig's 0.0001 preTransform puts every legitimate
	combined determinant around 1e-12 -- a genuinely singular frame inverts to
	infinities and the finiteness gate drops it instead. */
	const f32_t fRenderAlpha = m_fStepAccumulator / SIMULATION_STEP_SECONDS;
	matrix_t ParentFrame = XMMatrixIdentity();
	matrix_t FrameAbove = XMMatrixIdentity();
	bool_t isChainValid = false;
	for (size_t i = 0; i < m_Links.size(); ++i)
	{
		const LINK& Link = m_Links[i];
		if (Link.iParentLinkIndex < 0)
		{
			ParentFrame = RootCombined[i];
			vector_t vDeterminant = XMMatrixDeterminant(Locals[i]);
			const f32_t fDeterminant = XMVectorGetX(vDeterminant);
			isChainValid = std::isfinite(fDeterminant) && 0.f != fDeterminant;
			if (isChainValid)
			{
				FrameAbove = XMMatrixInverse(&vDeterminant, Locals[i]) *
					RootCombined[i];
			}
			continue;
		}
		if (!isChainValid)
			continue;

		const vector_t vTail = XMVectorLerp(
			XMLoadFloat3(&Link.vPreviousPosition),
			XMLoadFloat3(&Link.vPosition),
			fRenderAlpha);
		const matrix_t SelfFrame = Locals[i] * ParentFrame;
		matrix_t ParentResult = Aim_Frame(
			ParentFrame, SelfFrame.r[3], vTail);

		const uint32_t iParentBone =
			m_Links[Link.iParentLinkIndex].iBoneIndex;
		vector_t vDeterminant = XMMatrixDeterminant(FrameAbove);
		const f32_t fDeterminant = XMVectorGetX(vDeterminant);
		bool_t isWritten = false;
		if (std::isfinite(fDeterminant) && 0.f != fDeterminant)
		{
			const matrix_t NewLocal = ParentResult *
				XMMatrixInverse(&vDeterminant, FrameAbove);
			/* A single non-finite value would be skinned into every vertex
			the bone touches; the bone keeps its animated pose instead. */
			if (Is_Finite(NewLocal))
			{
				pModel->Set_BoneLocalMatrix(iParentBone, NewLocal);
				isWritten = true;
			}
		}
		if (!isWritten)
			ParentResult = ParentFrame;

		FrameAbove = ParentResult;
		ParentFrame = Locals[i] * ParentResult;
	}
	pModel->Refresh_BoneCombinedMatrices();
}
