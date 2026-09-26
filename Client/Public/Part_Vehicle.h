#pragma once

#include "Client_Defines.h"
#include "PartObject.h"
#include "SkeletalAfterimage.h"
#include "Network/PacketMessages.h"

#include <string>
#include <vector>

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CCharacter;

class CPart_Vehicle final : public CPartObject
{
public:
	typedef struct tagPartVehicleDesc : public CPartObject::PARTOBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		wstring_t strModelTag;
		wstring_t strShaderTag;
		std::string strIdleClip;
		std::string strRunClip;
		std::string strSeatBone;
	} PART_VEHICLE_DESC;

private:
	CPart_Vehicle(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CPart_Vehicle();

public:
	void Set_CharacterPresentationOwner(const std::shared_ptr<CCharacter>& owner)
	{ m_pCharacterPresentationOwner = owner; }
	bool_t Set_Moving(bool_t isMoving);
	bool_t Try_Get_SeatWorldPosition(float3_t& outPosition) const;
	/* How far the seat bone has turned from the pose it had when the vehicle was
	attached. A clip that spins the vehicle body can then turn the rider with it
	without the model's own import basis reaching the character. */
	bool_t Try_Get_SeatRotationDelta(float4x4_t& outRotation) const;
	/* Poses the vehicle on its skill chain at the Server action age, clips back to
	back and the last one held. Locomotion switches are ignored until Resume. */
	bool_t Seek_SkillChain(const std::vector<std::string>& clips, f32_t actionAgeSeconds);
	void Resume_Locomotion();
	bool Set_FlightPlayback(const std::vector<std::string>& clips, LostArk::Shared::VEHICLE_FLIGHT_PHASE phase,
		f32_t phaseAge, f32_t phaseDuration, f32_t loopStart, f32_t loopEnd, f32_t landingStart);
	void Clear_FlightPlayback();
	f32_t Get_FlightClipSeconds() const;
	bool Pose_FlightRider(const shared_ptr<CModel>& model, uint32_t animation) const;
	/* Start and length of one chain clip on the same clock Seek_SkillChain uses. */
	bool_t Try_Get_SkillClipWindow(const std::vector<std::string>& clips, std::size_t clipIndex,
		f32_t& outStartSeconds, f32_t& outDurationSeconds) const;
	/* Where the looping idle/run clip currently sits. Fails while a skill chain
	owns the model, so locomotion cues stop at the action edge. */
	bool_t Try_Get_LocomotionClipTime(std::string& outClip,
		f32_t& outSeconds, f32_t& outDurationSeconds) const;
	shared_ptr<CModel> Get_Model() const { return m_pModelCom; }
    bool Get_AfterimageView(CSkeletalAfterimage::MODEL_VIEW& view) const
    {
        if (!m_pModelCom || !m_pShaderCom) return false;
        view.model = view.paletteModel = m_pModelCom; view.shader = m_pShaderCom;
        view.world = m_CombinedWorldMatrix; view.hiddenMeshMask = 0u; view.socketed = false;
        return true;
    }
	const float4x4_t& Get_CombinedWorldMatrix() const { return m_CombinedWorldMatrix; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Group(RENDERGROUP group) override;
	virtual HRESULT Render_Shadow() override;

private:
	std::weak_ptr<CCharacter> m_pCharacterPresentationOwner;
	bool_t Is_CharacterPresentationHidden() const;
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };
	std::string m_strIdleClip;
	std::string m_strRunClip;
	std::string m_strSeatBone;
	bool_t m_isMoving = { false };
	bool_t m_isPlayingSkill = { false };
	bool_t m_hasTranslucentMeshes = { false };
	bool_t m_hasRestSeatRotation = { false };
	float4x4_t m_RestSeatRotationInverse = {};
	LostArk::Shared::VEHICLE_FLIGHT_PHASE m_FlightPhase = LostArk::Shared::VEHICLE_FLIGHT_PHASE::GROUNDED;
	uint32_t m_iFlightAnimation = UINT32_MAX;
	f32_t m_fFlightAge = 0.f, m_fFlightDuration = 0.f, m_fFlightClipDuration = 0.f;
	f32_t m_fFlightLoopStart = 0.f, m_fFlightLoopEnd = 0.f, m_fFlightLandingStart = 0.f;
	f32_t m_fFlightSteerYaw = 0.f, m_fFlightSteerPitch = 0.f;
	f32_t m_fFlightPreviousHeading = 0.f, m_fFlightPreviousY = 0.f;
	bool m_bFlightSteeringInitialized = false;

private:
	HRESULT Ready_Components(const PART_VEHICLE_DESC* pDesc);
	HRESULT Bind_ShaderResources();
	HRESULT Bind_ShadowShaderResources();
	HRESULT Render_Translucent();
	void Apply_FlightHeadIK(f32_t deltaSeconds);
	void Resolve_FlightPoseTimes(f32_t& sourceSeconds, f32_t& targetSeconds, f32_t& blend) const;

public:
	static unique_ptr<CPart_Vehicle> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
