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
		float4_t rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
		float3_t signedScale = float3_t(1.f, 1.f, 1.f);
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
	const float4_t& Get_RotationQuaternion() const { return m_vRotationQuaternion; }
	const float3_t& Get_SignedScale() const { return m_vSignedScale; }
	bool_t Is_Visible() const { return m_bVisible; }
	bool_t Is_Mirrored() const { return m_bMirrored; }

	void Set_PlacementTransform(const float3_t& position,
		const float4_t& rotationQuaternion, const float3_t& signedScale);
	void Set_Visible(bool_t visible) { m_bVisible = visible; }

private:
	uint64_t m_iPlacementId = {};
	std::string m_AssetId;
	float3_t m_vPlacementPosition = {};
	float4_t m_vRotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
	float3_t m_vSignedScale = float3_t(1.f, 1.f, 1.f);
	bool_t m_bApplyBottomCenter = false;
	bool_t m_bVisible = true;
	bool_t m_bMirrored = false;
	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };

private:
	HRESULT Ready_Components(const std::wstring& modelPrototypeTag);
	HRESULT Bind_ShaderResources();
	float3_t Compute_WorldOrigin(const float3_t& placementPosition,
		const float4_t& rotationQuaternion, const float3_t& signedScale) const;

public:
	static unique_ptr<CMapAssetObject> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
