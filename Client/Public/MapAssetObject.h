#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CMapAssetObject final : public CGameObject
{
public:
	struct MAP_ASSET_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		uint64_t placementId = {};
		std::string assetId;
		std::wstring modelPrototypeTag;
		float3_t position = {};
		float3_t rotationDegrees = {};
		float3_t scale = float3_t(1.f, 1.f, 1.f);
		bool_t applyBottomCenter = false;
		bool_t visible = true;
	};

private:
	CMapAssetObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CMapAssetObject();

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	uint64_t Get_PlacementId() const { return m_iPlacementId; }
	const std::string& Get_AssetId() const { return m_AssetId; }
	const float3_t& Get_Position() const { return m_vPlacementPosition; }
	const float3_t& Get_RotationDegrees() const { return m_vRotationDegrees; }
	const float3_t& Get_Scale() const { return m_vScale; }
	bool_t Is_Visible() const { return m_bVisible; }

	void Set_PlacementTransform(const float3_t& position,
		const float3_t& rotationDegrees, const float3_t& scale);
	void Set_Visible(bool_t visible) { m_bVisible = visible; }

private:
	uint64_t m_iPlacementId = {};
	std::string m_AssetId;
	float3_t m_vPlacementPosition = {};
	float3_t m_vRotationDegrees = {};
	float3_t m_vScale = float3_t(1.f, 1.f, 1.f);
	bool_t m_bApplyBottomCenter = false;
	bool_t m_bVisible = true;

	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };

private:
	HRESULT Ready_Components(const std::wstring& modelPrototypeTag);
	HRESULT Bind_ShaderResources();
	float3_t Compute_WorldOrigin(const float3_t& placementPosition,
		const float3_t& rotationDegrees, const float3_t& scale) const;

public:
	static unique_ptr<CMapAssetObject> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
