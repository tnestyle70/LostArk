#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)
class CCookedModel;
NS_END

NS_BEGIN(Client)

class CBinaryAssetObject final : public CGameObject
{
public:
	struct BINARY_ASSET_DESC final : public CGameObject::GAMEOBJECT_DESC
	{
		MODEL_ASSET_LOAD_DESC asset;
		float3_t position = { 0.f, 0.f, 0.f };
		f32_t scale = { 1.f };
	};

private:
	CBinaryAssetObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CBinaryAssetObject();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Components(const MODEL_ASSET_LOAD_DESC& assetDesc);
	HRESULT Bind_ShaderResources();

private:
	shared_ptr<class CShader> m_pShaderCom = { nullptr };
	shared_ptr<Engine::CCookedModel> m_pModel = { nullptr };

public:
	static unique_ptr<CBinaryAssetObject> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
