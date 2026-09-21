#pragma once

#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CUIObject abstract : public CGameObject
{
public:
	typedef struct tagUIObjectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		f32_t		fSizeX, fSizeY;		
		f32_t		fX, fY;
	}UIOBJECT_DESC;

protected:
	CUIObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CUIObject();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	virtual HRESULT Render();
	/* Draw order inside RENDERGROUP::UI. The owning screen sets it once; the renderer
	sorts by it so a surface created later cannot cover one that is meant to be above. */
	void Set_UISortLayer(int32_t iLayer) { m_iUISortLayer = iLayer; }
	virtual int32_t Get_UISortLayer() const override { return m_iUISortLayer; }

protected:
	int32_t				m_iUISortLayer = 0;
	f32_t				m_fX{}, m_fY{}, m_fSizeX{}, m_fSizeY{};
	float4x4_t			m_TransformMatrices[ETOUI(D3DTS::END)] = {};

protected:
	HRESULT Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);
};

NS_END