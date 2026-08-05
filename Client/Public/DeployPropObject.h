#pragma once

#include "Client_Defines.h"
#include "DeployPropCatalog.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CDeployPropObject final : public CGameObject
{
public:
	struct DEPLOY_PROP_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		uint32_t prototypeLevelIndex = ETOUI(LEVEL::END);
		DEPLOY_PROP_PLACEMENT placement;
		DEPLOY_PROP_MODEL_KIND modelKind = DEPLOY_PROP_MODEL_KIND::STATIC;
		std::wstring intactPrototypeTag;
		std::wstring fracturedPrototypeTag;
	};

private:
	CDeployPropObject(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CDeployPropObject();
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	bool_t Set_State(DEPLOY_PROP_STATE state);
	DEPLOY_PROP_STATE Get_State() const { return m_State; }
	uint64_t Get_RuntimePlacementId() const { return m_Placement.runtimePlacementId; }
	uint32_t Get_DeployActorId() const { return m_Placement.deployActorId; }
	bool_t Is_Destructible() const { return m_Placement.destructible; }
	bool_t Is_AnimBindPoseOnly() const;

private:
	HRESULT Ready_Components(const DEPLOY_PROP_DESC& desc);
	HRESULT Bind_CommonShaderResources();
	HRESULT Render_Static(const shared_ptr<CModel>& model);
	HRESULT Render_Animated();
	void Apply_Transform();

private:
	DEPLOY_PROP_PLACEMENT m_Placement;
	DEPLOY_PROP_MODEL_KIND m_ModelKind = DEPLOY_PROP_MODEL_KIND::STATIC;
	DEPLOY_PROP_STATE m_State = DEPLOY_PROP_STATE::INTACT;
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pIntactModelCom = { nullptr };
	shared_ptr<CModel> m_pFracturedModelCom = { nullptr };

public:
	static unique_ptr<CDeployPropObject> Create(
		ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
