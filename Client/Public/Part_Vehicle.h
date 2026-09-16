#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

#include <string>
#include <vector>

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

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
	bool_t Set_Moving(bool_t isMoving);
	bool_t Try_Get_SeatWorldPosition(float3_t& outPosition) const;
	/* Poses the vehicle on its skill chain at the Server action age, clips back to
	back and the last one held. Locomotion switches are ignored until Resume. */
	bool_t Seek_SkillChain(const std::vector<std::string>& clips, f32_t actionAgeSeconds);
	void Resume_Locomotion();
	/* Start and length of one chain clip on the same clock Seek_SkillChain uses. */
	bool_t Try_Get_SkillClipWindow(const std::vector<std::string>& clips, std::size_t clipIndex,
		f32_t& outStartSeconds, f32_t& outDurationSeconds) const;
	/* Where the looping idle/run clip currently sits. Fails while a skill chain
	owns the model, so locomotion cues stop at the action edge. */
	bool_t Try_Get_LocomotionClipTime(std::string& outClip,
		f32_t& outSeconds, f32_t& outDurationSeconds) const;
	shared_ptr<CModel> Get_Model() const { return m_pModelCom; }
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
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };
	std::string m_strIdleClip;
	std::string m_strRunClip;
	std::string m_strSeatBone;
	bool_t m_isMoving = { false };
	bool_t m_isPlayingSkill = { false };
	bool_t m_hasTranslucentMeshes = { false };

private:
	HRESULT Ready_Components(const PART_VEHICLE_DESC* pDesc);
	HRESULT Bind_ShaderResources();
	HRESULT Bind_ShadowShaderResources();
	HRESULT Render_Translucent();

public:
	static unique_ptr<CPart_Vehicle> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
