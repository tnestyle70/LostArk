#include "Part_Vehicle.h"
#include "BinaryAsset/ModelAssetData.h"

#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "MapAssetRenderUtils.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr uint32_t SOURCE_TRANSLUCENT_TWO_SIDED_PASS = 9u;
	constexpr uint32_t SOURCE_TRANSLUCENT_ONE_SIDED_PASS = 10u;
	constexpr f32_t LOCOMOTION_BLEND_SECONDS = 0.12f;
	constexpr const char_t* ROOT_MOTION_BONE = "b_root";
	constexpr int32_t ROOT_MOTION_VERTICAL_AXIS = 2;

	uint32_t Resolve_TranslucentSourcePass(const Engine::MODEL_SURFACE_PARAMETERS* surface)
	{
		if (nullptr == surface || surface->family != Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
			return 0u;
		switch (surface->sourceCharacter.program)
		{
		case 18u: return SOURCE_TRANSLUCENT_TWO_SIDED_PASS;
		case 88u: return SOURCE_TRANSLUCENT_ONE_SIDED_PASS;
		default: return 0u;
		}
	}

	bool_t Try_Get_NormalizedRotation(const matrix_t& Source, matrix_t& Out)
	{
		matrix_t staged = Source;
		staged.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		for (int32_t axis = 0; axis < 3; ++axis)
		{
			const f32_t length = XMVectorGetX(XMVector3Length(staged.r[axis]));
			if (!std::isfinite(length) || length <= 1.0e-6f)
				return false;
			staged.r[axis] = XMVectorScale(staged.r[axis], 1.f / length);
			staged.r[axis] = XMVectorSetW(staged.r[axis], 0.f);
		}
		Out = staged;
		return true;
	}
}

CPart_Vehicle::CPart_Vehicle(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CPart_Vehicle::~CPart_Vehicle()
{
}

HRESULT CPart_Vehicle::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPart_Vehicle::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<PART_VEHICLE_DESC*>(pArg);
	m_strIdleClip = pDesc->strIdleClip;
	m_strRunClip = pDesc->strRunClip;
	m_strSeatBone = pDesc->strSeatBone;

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components(pDesc)) ||
		!m_pModelCom->Has_Bone(m_strSeatBone.c_str()) ||
		!m_pModelCom->Set_Animation(m_strIdleClip.c_str(), true))
	{
		return E_FAIL;
	}
	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
		m_hasTranslucentMeshes |= 0u != Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));
	if (m_pModelCom->Has_Bone(ROOT_MOTION_BONE))
		(void)m_pModelCom->Enable_RootMotionSuppression(ROOT_MOTION_BONE, ROOT_MOTION_VERTICAL_AXIS);
	matrix_t restRotation{};
	if (Try_Get_NormalizedRotation(
		m_pModelCom->Get_BoneMatrix(m_strSeatBone.c_str()), restRotation))
	{
		XMStoreFloat4x4(&m_RestSeatRotationInverse, XMMatrixTranspose(restRotation));
		m_hasRestSeatRotation = true;
	}
	return S_OK;
}

bool_t CPart_Vehicle::Try_Get_SeatRotationDelta(float4x4_t& outRotation) const
{
	matrix_t current{};
	if (nullptr == m_pModelCom || !m_hasRestSeatRotation ||
		!m_pModelCom->Has_Bone(m_strSeatBone.c_str()) ||
		!Try_Get_NormalizedRotation(
			m_pModelCom->Get_BoneMatrix(m_strSeatBone.c_str()), current))
	{
		return false;
	}
	float4x4_t staged{};
	XMStoreFloat4x4(&staged, XMLoadFloat4x4(&m_RestSeatRotationInverse) * current);
	for (const f32_t value : { staged._11, staged._12, staged._13,
		staged._21, staged._22, staged._23,
		staged._31, staged._32, staged._33 })
	{
		if (!std::isfinite(value))
			return false;
	}
	outRotation = staged;
	return true;
}

bool_t CPart_Vehicle::Set_Moving(const bool_t isMoving)
{
	if (nullptr == m_pModelCom)
		return false;
	if (m_isMoving == isMoving)
		return true;
	m_isMoving = isMoving;
	if (m_isPlayingSkill)
		return true;
	return m_pModelCom->Set_Animation(
		(isMoving ? m_strRunClip : m_strIdleClip).c_str(), true, LOCOMOTION_BLEND_SECONDS);
}

bool_t CPart_Vehicle::Seek_SkillChain(
	const std::vector<std::string>& clips,
	const f32_t actionAgeSeconds)
{
	if (nullptr == m_pModelCom || clips.empty() ||
		!std::isfinite(actionAgeSeconds) || actionAgeSeconds < 0.f)
	{
		return false;
	}
	f32_t remaining = actionAgeSeconds;
	for (std::size_t step = 0u; step < clips.size(); ++step)
	{
		uint32_t animation = UINT32_MAX;
		for (uint32_t index = 0u; index < m_pModelCom->Get_NumAnimations(); ++index)
		{
			const char_t* pName = m_pModelCom->Get_AnimationName(index);
			if (nullptr != pName && clips[step] == pName)
			{
				animation = index;
				break;
			}
		}
		f32_t position = 0.f;
		f32_t duration = 0.f;
		const f32_t ticksPerSecond = UINT32_MAX == animation ? 0.f :
			m_pModelCom->Get_AnimationTickPerSecond(animation);
		if (UINT32_MAX == animation ||
			!m_pModelCom->Get_AnimationProgress(animation, position, duration) ||
			!std::isfinite(duration) || duration <= 0.f ||
			!std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f)
		{
			return false;
		}
		const f32_t seconds = duration / ticksPerSecond;
		if (remaining < seconds || step + 1u == clips.size())
		{
			if (!m_isPlayingSkill || m_pModelCom->Get_CurrentAnimIndex() != animation)
			{
				if (!m_pModelCom->Set_Animation(clips[step].c_str(), false, LOCOMOTION_BLEND_SECONDS))
					return false;
			}
			m_isPlayingSkill = true;
			const f32_t local = (std::min)(remaining, (std::max)(0.f, seconds - 0.0001f));
			return m_pModelCom->Set_AnimTrackPosition(animation, local * ticksPerSecond);
		}
		remaining -= seconds;
	}
	return false;
}

bool_t CPart_Vehicle::Try_Get_SkillClipWindow(
	const std::vector<std::string>& clips,
	const std::size_t clipIndex,
	f32_t& outStartSeconds,
	f32_t& outDurationSeconds) const
{
	if (nullptr == m_pModelCom || clipIndex >= clips.size())
		return false;
	f32_t start = 0.f;
	for (std::size_t step = 0u; step <= clipIndex; ++step)
	{
		uint32_t animation = UINT32_MAX;
		for (uint32_t index = 0u; index < m_pModelCom->Get_NumAnimations(); ++index)
		{
			const char_t* pName = m_pModelCom->Get_AnimationName(index);
			if (nullptr != pName && clips[step] == pName)
			{
				animation = index;
				break;
			}
		}
		f32_t position = 0.f;
		f32_t duration = 0.f;
		const f32_t ticksPerSecond = UINT32_MAX == animation ? 0.f :
			m_pModelCom->Get_AnimationTickPerSecond(animation);
		if (UINT32_MAX == animation ||
			!m_pModelCom->Get_AnimationProgress(animation, position, duration) ||
			!std::isfinite(duration) || duration <= 0.f ||
			!std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f)
		{
			return false;
		}
		if (step == clipIndex)
		{
			outStartSeconds = start;
			outDurationSeconds = duration / ticksPerSecond;
			return true;
		}
		start += duration / ticksPerSecond;
	}
	return false;
}

bool_t CPart_Vehicle::Try_Get_LocomotionClipTime(
	std::string& outClip,
	f32_t& outSeconds,
	f32_t& outDurationSeconds) const
{
	if (nullptr == m_pModelCom || m_isPlayingSkill)
		return false;
	const std::string& clip = m_isMoving ? m_strRunClip : m_strIdleClip;
	const uint32_t animation = m_pModelCom->Get_CurrentAnimIndex();
	const char_t* pName = m_pModelCom->Get_AnimationName(animation);
	/* A locomotion switch blends for a moment; until the new clip owns the
	track its position belongs to the previous one. */
	if (nullptr == pName || clip != pName)
		return false;
	f32_t position = 0.f;
	f32_t duration = 0.f;
	const f32_t ticksPerSecond = m_pModelCom->Get_AnimationTickPerSecond(animation);
	if (!m_pModelCom->Get_AnimationProgress(animation, position, duration) ||
		!std::isfinite(position) || position < 0.f ||
		!std::isfinite(duration) || duration <= 0.f ||
		!std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f)
	{
		return false;
	}
	outClip = clip;
	outSeconds = position / ticksPerSecond;
	outDurationSeconds = duration / ticksPerSecond;
	return true;
}

bool CPart_Vehicle::Set_FlightPlayback(const std::vector<std::string>& clips,
    const LostArk::Shared::VEHICLE_FLIGHT_PHASE phase, const f32_t phaseAge,
    const f32_t phaseDuration, const f32_t loopStart, const f32_t loopEnd, const f32_t landingStart)
{
    using LostArk::Shared::VEHICLE_FLIGHT_PHASE;
    f32_t start = 0.f, duration = 0.f;
    if (!m_pModelCom || clips.size() != 1u || phase <= VEHICLE_FLIGHT_PHASE::GROUNDED ||
        phase >= VEHICLE_FLIGHT_PHASE::END || !std::isfinite(phaseAge) || phaseAge < 0.f ||
        !std::isfinite(phaseDuration) || phaseDuration < 0.f ||
        !std::isfinite(loopStart) || !std::isfinite(loopEnd) || !std::isfinite(landingStart) ||
        loopStart <= 0.f || loopEnd <= loopStart || landingStart < loopEnd ||
        !Try_Get_SkillClipWindow(clips, 0u, start, duration) || landingStart >= duration ||
        (phase != VEHICLE_FLIGHT_PHASE::FLYING && phaseDuration <= 0.f)) return false;
    if (!Seek_SkillChain(clips, 0.f)) return false;
    m_iFlightAnimation = m_pModelCom->Get_CurrentAnimIndex();
    m_FlightPhase = phase; m_fFlightAge = phaseAge; m_fFlightDuration = phaseDuration;
    m_fFlightClipDuration = duration; m_fFlightLoopStart = loopStart;
    m_fFlightLoopEnd = loopEnd; m_fFlightLandingStart = landingStart;
    return Pose_FlightRider(m_pModelCom, m_iFlightAnimation);
}

void CPart_Vehicle::Resolve_FlightPoseTimes(f32_t& source, f32_t& target, f32_t& blend) const
{
    using LostArk::Shared::VEHICLE_FLIGHT_PHASE;
    blend = 1.f;
    if (m_FlightPhase == VEHICLE_FLIGHT_PHASE::TAKEOFF)
        source = target = m_fFlightLoopStart * std::clamp(m_fFlightAge / m_fFlightDuration, 0.f, 1.f);
    else if (m_FlightPhase == VEHICLE_FLIGHT_PHASE::LANDING)
        source = target = m_fFlightLandingStart + (m_fFlightClipDuration - m_fFlightLandingStart) *
            std::clamp(m_fFlightAge / m_fFlightDuration, 0.f, 1.f);
    else
    {
        const f32_t span = m_fFlightLoopEnd - m_fFlightLoopStart;
        const f32_t overlap = (std::min)(.12f, span * .2f);
        // Overlap the end with the start, then resume after that already-played prefix.
        const f32_t time = m_fFlightAge < span ? m_fFlightAge :
            overlap + std::fmod(m_fFlightAge - span, span - overlap);
        source = target = m_fFlightLoopStart + time;
        if (time > span - overlap)
        {
            target = m_fFlightLoopStart + time - (span - overlap);
            blend = (time - (span - overlap)) / overlap;
        }
    }
    source = std::clamp(source, 0.f, m_fFlightClipDuration - .0001f);
    target = std::clamp(target, 0.f, m_fFlightClipDuration - .0001f);
}

f32_t CPart_Vehicle::Get_FlightClipSeconds() const
{
    if (m_FlightPhase == LostArk::Shared::VEHICLE_FLIGHT_PHASE::GROUNDED) return 0.f;
    f32_t source = 0.f, target = 0.f, blend = 0.f;
    Resolve_FlightPoseTimes(source, target, blend);
    return target;
}

bool CPart_Vehicle::Pose_FlightRider(const shared_ptr<CModel>& model, const uint32_t animation) const
{
    if (!model || m_FlightPhase == LostArk::Shared::VEHICLE_FLIGHT_PHASE::GROUNDED) return false;
    f32_t cursor = 0.f, duration = 0.f;
    const f32_t rate = model->Get_AnimationTickPerSecond(animation);
    if (!model->Get_AnimationProgress(animation, cursor, duration) || rate <= 0.f) return false;
    f32_t source = 0.f, target = 0.f, blend = 0.f;
    Resolve_FlightPoseTimes(source, target, blend);
    CModel::ANIMATION_TRANSITION_POSE pose{};
    pose.sourceIndex = pose.targetIndex = animation;
    pose.sourceTicks = (std::min)(source * rate, duration);
    pose.targetTicks = (std::min)(target * rate, duration);
    pose.durationSeconds = 1.f;
    pose.elapsedSeconds = blend;
    if (!model->Set_AnimationTransitionPose(pose)) return false;
    model->Set_AnimPaused(true);
    return true;
}

void CPart_Vehicle::Clear_FlightPlayback()
{
    if (m_FlightPhase == LostArk::Shared::VEHICLE_FLIGHT_PHASE::GROUNDED) return;
    m_FlightPhase = LostArk::Shared::VEHICLE_FLIGHT_PHASE::GROUNDED;
    m_iFlightAnimation = UINT32_MAX;
    m_bFlightSteeringInitialized = false;
    m_fFlightSteerYaw = m_fFlightSteerPitch = 0.f;
    if (m_pModelCom) { m_pModelCom->Clear_AnimationTransitionPose(); m_pModelCom->Set_AnimPaused(false); }
}

void CPart_Vehicle::Apply_FlightHeadIK(const f32_t deltaSeconds)
{
    if (!m_pModelCom || !m_pParentMatrix || !std::isfinite(deltaSeconds) || deltaSeconds <= 0.f) return;
    const matrix_t world = XMLoadFloat4x4(&m_CombinedWorldMatrix);
    const matrix_t owner = XMLoadFloat4x4(m_pParentMatrix);
    const float heading = std::atan2(XMVectorGetX(owner.r[2]), XMVectorGetZ(owner.r[2]));
    const float y = XMVectorGetY(owner.r[3]);
    if (!m_bFlightSteeringInitialized)
    {
        m_fFlightPreviousHeading = heading; m_fFlightPreviousY = y; m_bFlightSteeringInitialized = true;
    }
    const float turn = std::remainder(heading - m_fFlightPreviousHeading, XM_2PI) / deltaSeconds;
    const float climb = (y - m_fFlightPreviousY) / deltaSeconds;
    m_fFlightPreviousHeading = heading; m_fFlightPreviousY = y;
    const float alpha = 1.f - std::exp(-7.f * deltaSeconds);
    m_fFlightSteerYaw += (std::clamp(turn * .18f, -.45f, .45f) - m_fFlightSteerYaw) * alpha;
    m_fFlightSteerPitch += (std::clamp(climb * .08f, -.25f, .25f) - m_fFlightSteerPitch) * alpha;
    const int head = m_pModelCom->Find_BoneIndex("bip001-head");
    const int tip = m_pModelCom->Find_BoneIndex("b_m_00");
    matrix_t headPose{}, tipPose{};
    if (head < 0 || tip < 0 || !m_pModelCom->Get_BoneCombinedMatrix(head, headPose) ||
        !m_pModelCom->Get_BoneCombinedMatrix(tip, tipPose)) return;
    const vector_t headPosition = XMVector3TransformCoord(headPose.r[3], world);
    const vector_t tipPosition = XMVector3TransformCoord(tipPose.r[3], world);
    vector_t forward = tipPosition - headPosition;
    if (XMVectorGetX(XMVector3LengthSq(forward)) < .000001f) return;
    const vector_t right = XMVector3Normalize(owner.r[0]);
    forward = XMVector3TransformNormal(forward,
        XMMatrixRotationY(m_fFlightSteerYaw) * XMMatrixRotationAxis(right, -m_fFlightSteerPitch));
    const vector_t target = headPosition + forward;
    // CCD on the installed neck/head chain. Each local rotation is expressed in its
    // actual parent basis; no imported bone-axis or 90-degree correction is assumed.
    const char* chain[] = { "bip001-neck", "bip001-neck1", "bip001-neck2", "bip001-head" };
    for (const char* name : chain)
    {
        const int joint = m_pModelCom->Find_BoneIndex(name);
        if (joint < 0) continue;
        const int parent = m_pModelCom->Get_BoneParentIndex(joint);
        matrix_t local{}, parentPose{}, jointPose{};
        if (parent < 0 || !m_pModelCom->Get_BoneLocalMatrix(joint, local) ||
            !m_pModelCom->Get_BoneCombinedMatrix(parent, parentPose) ||
            !m_pModelCom->Get_BoneCombinedMatrix(joint, jointPose) ||
            !m_pModelCom->Get_BoneCombinedMatrix(tip, tipPose)) continue;
        const matrix_t inverseParent = XMMatrixInverse(nullptr, parentPose * world);
        const vector_t pivot = XMVector3TransformCoord(jointPose.r[3], world);
        vector_t from = XMVector3TransformNormal(XMVector3TransformCoord(tipPose.r[3], world) - pivot, inverseParent);
        vector_t to = XMVector3TransformNormal(target - pivot, inverseParent);
        if (XMVectorGetX(XMVector3LengthSq(from)) < .000001f || XMVectorGetX(XMVector3LengthSq(to)) < .000001f) continue;
        from = XMVector3Normalize(from); to = XMVector3Normalize(to);
        vector_t axis = XMVector3Cross(from, to);
        if (XMVectorGetX(XMVector3LengthSq(axis)) < .000001f) continue;
        const float angle = (std::min)(.12f, std::acos(std::clamp(XMVectorGetX(XMVector3Dot(from, to)), -1.f, 1.f)));
        const vector_t translation = local.r[3];
        local = local * XMMatrixRotationAxis(XMVector3Normalize(axis), angle);
        local.r[3] = translation;
        m_pModelCom->Set_BoneLocalMatrix(joint, local);
        m_pModelCom->Refresh_BoneCombinedMatrices();
    }
}

void CPart_Vehicle::Resume_Locomotion()
{
	Clear_FlightPlayback();
	if (nullptr == m_pModelCom || !m_isPlayingSkill)
		return;
	m_isPlayingSkill = false;
	(void)m_pModelCom->Set_Animation(
		(m_isMoving ? m_strRunClip : m_strIdleClip).c_str(), true, LOCOMOTION_BLEND_SECONDS);
}

bool_t CPart_Vehicle::Try_Get_SeatWorldPosition(float3_t& outPosition) const
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom ||
		nullptr == m_pParentMatrix || !m_pModelCom->Has_Bone(m_strSeatBone.c_str()))
	{
		return false;
	}
	const matrix_t seat = m_pModelCom->Get_BoneMatrix(m_strSeatBone.c_str()) *
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) *
		XMLoadFloat4x4(m_pParentMatrix);
	float3_t staged{};
	XMStoreFloat3(&staged, seat.r[3]);
	if (!std::isfinite(staged.x) || !std::isfinite(staged.y) || !std::isfinite(staged.z))
		return false;
	outPosition = staged;
	return true;
}

void CPart_Vehicle::Priority_Update(f32_t fTimeDelta)
{
}

void CPart_Vehicle::Update(f32_t fTimeDelta)
{
	const bool flight = m_FlightPhase != LostArk::Shared::VEHICLE_FLIGHT_PHASE::GROUNDED;
	if (flight)
	{
		if (std::isfinite(fTimeDelta) && fTimeDelta > 0.f) m_fFlightAge += fTimeDelta;
		(void)Pose_FlightRider(m_pModelCom, m_iFlightAnimation);
		// The Server owns altitude. Remove only this clip's authored body lift, whose
		// installed skeleton uses local Z; retaining it would add the ascent twice.
		const int body = m_pModelCom->Find_BoneIndex("bip001");
		matrix_t local{}, rest{};
		if (body >= 0 && m_pModelCom->Get_BoneLocalMatrix(body, local) &&
			m_pModelCom->Get_BoneRestLocalMatrix(body, rest))
		{
			local.r[3] = XMVectorSetZ(local.r[3], XMVectorGetZ(rest.r[3]));
			m_pModelCom->Set_BoneLocalMatrix(body, local);
			m_pModelCom->Refresh_BoneCombinedMatrices();
		}
	}
	else m_pModelCom->Update_Animation(fTimeDelta);
	__super::Update_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
	if (flight) Apply_FlightHeadIK(fTimeDelta);
}

void CPart_Vehicle::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
	if (m_hasTranslucentMeshes)
	{
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::BLEND,
			static_pointer_cast<CGameObject>(shared_from_this()));
	}
	if (CGameInstance::Get().Is_ShadowLightEnabled())
	{
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::SHADOW,
			static_pointer_cast<CGameObject>(shared_from_this()));
	}
}

HRESULT CPart_Vehicle::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		uint32_t materialPass = 0u;
		const auto* surface = m_pModelCom->Get_MaterialSurface(i);
		if (0u != Resolve_TranslucentSourcePass(surface))
			continue;
		if (surface &&
			surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
			(surface->sourceCharacter.program == 6u || surface->sourceCharacter.program == 7u ||
			 surface->sourceCharacter.program == 19u ||
			 surface->sourceCharacter.program == 20u))
			materialPass = 6u;
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, nullptr)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(materialPass)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Vehicle::Render_Group(RENDERGROUP group)
{
	return RENDERGROUP::BLEND == group ? Render_Translucent() : Render();
}

HRESULT CPart_Vehicle::Render_Translucent()
{
	if (FAILED(Bind_ShaderResources()) ||
		FAILED(CMapAssetRenderUtils::Bind_SourceCharacterForwardLights(m_pShaderCom)))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		const uint32_t pass = Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));
		if (0u == pass)
			continue;
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, nullptr)) ||
			FAILED(m_pModelCom->Bind_SourceCharacterForwardLight(m_pShaderCom, i)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(pass)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Vehicle::Render_Shadow()
{
	constexpr uint32_t ANIMATED_SHADOW_PASS = 1u;
	if (FAILED(Bind_ShadowShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (FAILED(m_pModelCom->Bind_Material(
				m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(ANIMATED_SHADOW_PASS)) ||
			FAILED(m_pModelCom->Render(i)))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CPart_Vehicle::Ready_Components(const PART_VEHICLE_DESC* pDesc)
{
	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strShaderTag,
		TEXT("Com_Shader"),
		m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strModelTag,
		TEXT("Com_Model"),
		m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPart_Vehicle::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

HRESULT CPart_Vehicle::Bind_ShadowShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(
		m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}
	return S_OK;
}

unique_ptr<CPart_Vehicle> CPart_Vehicle::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CPart_Vehicle>(new CPart_Vehicle(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][PartVehicle] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CPart_Vehicle::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CPart_Vehicle>(new CPart_Vehicle(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][PartVehicle] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
