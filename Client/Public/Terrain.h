#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CNavigation;
class CVIBuffer_Terrain;
NS_END

NS_BEGIN(Client)

class CTerrain final : public CGameObject
{
public:
	typedef struct tagTerrainDesc : public CGameObject::GAMEOBJECT_DESC
	{

	}TERRAIN_DESC;

	enum TEXTURE { DIFFUSE, MASK, END  };

private:
	CTerrain(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CTerrain();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	shared_ptr<CShader>				m_pShaderCom = { nullptr };
	shared_ptr<CTexture>			m_TextureCom[TEXTURE::END] = {nullptr};
	shared_ptr<CVIBuffer_Terrain>	m_pVIBufferCom = { nullptr };
	shared_ptr<CNavigation>			m_pNavigationCom = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CTerrain> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	
};

NS_END