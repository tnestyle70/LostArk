#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

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

		/* nullptr means the piece is skinned to the body's skeleton. */
		const char_t* pSocketBoneName = { nullptr };
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

private:
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };
	shared_ptr<CModel> m_pSkeletonModelCom = { nullptr };
	const char_t* m_pSocketBoneName = { nullptr };

private:
	HRESULT Ready_Components(const PART_EQUIPMENT_DESC* pDesc);
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CPart_Equipment> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
