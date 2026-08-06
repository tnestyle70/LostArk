#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

class CTrigger_Box final : public CGameObject
{
public:
	struct TRIGGER_BOX_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		std::string placementId;
		float3_t position = {};
		float3_t halfExtents = float3_t(1.f, 1.f, 1.f);
		f32_t yawDegrees = {};
		bool_t isEnabled = false;
		bool_t isCollisionBox = false;
	};

private:
	CTrigger_Box(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CTrigger_Box();

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	bool_t Apply_Descriptor(const TRIGGER_BOX_DESC& desc);
	void Set_AuthoringVisible(bool_t isVisible);
	void Set_Selected(bool_t isSelected);
	const std::string& Get_PlacementId() const;

private:
	static bool_t Is_ValidDescriptor(const TRIGGER_BOX_DESC& desc);
	void Rebuild_Bounds();

private:
	TRIGGER_BOX_DESC m_Desc;
	BoundingOrientedBox m_Bounds;
	bool_t m_isAuthoringVisible = false;
	bool_t m_isSelected = false;
	shared_ptr<PrimitiveBatch<VertexPositionColor>> m_pBatch = { nullptr };
	shared_ptr<BasicEffect> m_pEffect = { nullptr };
	ComPtr<ID3D11InputLayout> m_pInputLayout = { nullptr };

public:
	static unique_ptr<CTrigger_Box> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
