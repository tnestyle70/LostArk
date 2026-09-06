#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

struct DEFERRED_EMISSIVE_OVERRIDE;

/* One visual equipment piece. The slot it fills is data, not behaviour:
helmet, chest, gloves and boots all do the same thing here. Only the way the
piece follows the body differs, and that is decided by pSocketBoneName.

A socketed piece is a static mesh riding a single bone (a shield, a hat).
A skinned piece deforms with the body, so it borrows the bone palette the
body's model already built. That works because the cooked path gives every
skinned mesh the same skeleton-wide palette, so a piece authored against the
body's rig needs no remapping and no animation data of its own. */
class CPart_Equipment final : public CPartObject
{
public:
	typedef struct tagPartEquipmentDesc : public CPartObject::PARTOBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		wstring_t strModelTag;
		wstring_t strShaderTag;

		/* The body's model: it owns the skeleton this piece rides or shares. */
		shared_ptr<Engine::CModel> pSkeletonModel = { nullptr };

		/* Bit i hides submesh i: the body already draws that content. */
		uint32_t iHiddenMeshMask = {};

		/* nullptr means the piece is skinned to the body's skeleton. */
		const char_t* pSocketBoneName = { nullptr };
		f32_t fSocketYawDegrees = 0.f;

		/* The frame the body model itself is drawn in. Both kinds of piece are
		placed in it, so a body that yaws its model does not leave its pieces
		behind. */
		const float4x4_t* pSocketRootMatrix = { nullptr };
		string strMaterialProfileId;
		const DEFERRED_EMISSIVE_OVERRIDE* pEmissiveOverride = { nullptr };
	} PART_EQUIPMENT_DESC;

private:
	CPart_Equipment(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CPart_Equipment();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;
	/* Same draw as Render() through explicit technique passes -- skinned pieces use the body
	shader, socketed (weapon) pieces the static-mesh shader, whose pass tables differ. */
	HRESULT Render_Pass(uint32_t iSkinnedPassIndex, uint32_t iSocketedPassIndex);

public:
	void Set_Visible(bool_t isVisible) { m_isVisible = isVisible; }
	bool_t Is_Visible() const { return m_isVisible; }

private:
	bool_t m_isVisible = true;
	uint32_t m_iHiddenMeshMask = {};
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };
	shared_ptr<CModel> m_pSkeletonModelCom = { nullptr };
	string m_strSocketBoneName;
	f32_t m_fSocketYawDegrees = 0.f;
	const float4x4_t* m_pSocketRootMatrix = { nullptr };
	string m_strMaterialProfileId;
	const DEFERRED_EMISSIVE_OVERRIDE* m_pEmissiveOverride = { nullptr };

private:
	HRESULT Ready_Components(const PART_EQUIPMENT_DESC* pDesc);
	HRESULT Bind_ShaderResources();
	HRESULT Bind_ShadowShaderResources();

public:
	static unique_ptr<CPart_Equipment> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
