#pragma once

#include "Client_Defines.h"
#include "PartObject.h"
#include "SkeletalAfterimage.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

struct DEFERRED_EMISSIVE_OVERRIDE;

class CBody_Valtan final : public CPartObject
{
public:
	typedef struct tagBodyValtanDesc : public CPartObject::PARTOBJECT_DESC
	{
		const uint32_t* pParentState = { nullptr };
		uint32_t iPrototypeLevelIndex = {};
		wstring_t strModelPrototypeTag = TEXT("Prototype_Component_Model_Valtan");
		const DEFERRED_EMISSIVE_OVERRIDE* pEmissiveOverride = { nullptr };
        const bool* pChargeAfterimageEnabled = nullptr;
	} BODY_VALTAN_DESC;

private:
	CBody_Valtan(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CBody_Valtan();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Group(RENDERGROUP group) override;
	virtual HRESULT Render_Shadow() override;
    std::string Get_RenderDiagnostic() const;
    void Reset_ChargeAfterimage() { m_ChargeAfterimage.Reset(); }

private:
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };
	bool_t m_hasTranslucentMeshes = { false };
    uint32_t m_iTranslucentDrawCount = 0u;
    std::string m_strTranslucentRenderFailure;
    const bool* m_pChargeAfterimageEnabled = nullptr;
    CSkeletalAfterimage m_ChargeAfterimage;
	const uint32_t* m_pParentState = { nullptr };
	uint32_t m_iPrototypeLevelIndex = {};
	wstring_t m_strModelPrototypeTag;
	const DEFERRED_EMISSIVE_OVERRIDE* m_pEmissiveOverride = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();
	HRESULT Render_Translucent();
	HRESULT Bind_ShadowShaderResources();

public:
	static unique_ptr<CBody_Valtan> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
