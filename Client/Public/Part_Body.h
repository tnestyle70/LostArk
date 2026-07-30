#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

/* The character's skinned body. It owns the skeleton and the animation clock, so
every other part reads its bone palette rather than animating on its own. */
class CPart_Body final : public CPartObject
{
public:
	typedef struct tagPartBodyDesc : public CPartObject::PARTOBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		wstring_t strModelTag;
		wstring_t strShaderTag;

		/* Bit i hides submesh i: equipment carries the skin underneath it. */
		uint32_t iHiddenMeshMask = {};

		const char_t* pInitialAnimation = { nullptr };
	} PART_BODY_DESC;

private:
	CPart_Body(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CPart_Body();

public:
	shared_ptr<CModel> Get_Model() const {
		return m_pModelCom;
	}
	void Set_HiddenMeshes(uint32_t iHiddenMeshMask) {
		m_iHiddenMeshMask = iHiddenMeshMask;
	}
	bool_t Set_Animation(const char_t* pClipName, bool_t isLoop);

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
	uint32_t m_iHiddenMeshMask = {};

private:
	HRESULT Ready_Components(const PART_BODY_DESC* pDesc);
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CPart_Body> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
