#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "PlayerSkillCatalog.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CSkillGroundTargetPreview final : public CGameObject
{
public:
	static constexpr const wchar_t* PROTOTYPE_TAG =
		L"Prototype_GameObject_SkillGroundTargetPreview";
	static constexpr const wchar_t* SHADER_TAG =
		L"Prototype_Component_Shader_SkillGroundTargetPreview";

private:
	CSkillGroundTargetPreview(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	CSkillGroundTargetPreview(const CSkillGroundTargetPreview& prototype);

public:
	virtual ~CSkillGroundTargetPreview();
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	bool_t Begin(const PLAYER_SKILL_DEFINITION& skill);
	void Set_State(
		const float3_t& casterPosition,
		const float3_t& targetPosition,
		bool_t targetValid);
	void Clear();

private:
	HRESULT Render_Quad(
		const shared_ptr<Engine::CTexture>& texture,
		const float3_t& position,
		f32_t diameter,
		const float4_t& tint);

private:
	shared_ptr<Engine::CShader> m_pShader;
	shared_ptr<Engine::CVIBuffer_Rect> m_pRect;
	shared_ptr<Engine::CTexture> m_pRangeTexture;
	shared_ptr<Engine::CTexture> m_pTargetTexture;
	PLAYER_SKILL_TARGET_PREVIEW m_RangeSpec;
	PLAYER_SKILL_TARGET_PREVIEW m_TargetSpec;
	float3_t m_CasterPosition{};
	float3_t m_TargetPosition{};
	bool_t m_isTargetValid = false;
	bool_t m_isActive = false;

public:
	static unique_ptr<CSkillGroundTargetPreview> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
