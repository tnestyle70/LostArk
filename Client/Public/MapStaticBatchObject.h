#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "MapAssetCatalog.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Engine)

class CModel;
class CShader;

NS_END

NS_BEGIN(Client)

struct FMapStaticInstance final
{
	uint64_t PlacementId = {};
	float4x4_t World = {};
	float4x4_t WorldInvTranspose = {};
	float3_t WorldBoundsCenter = {};
	f32_t WorldBoundsRadius = {};
	bool_t Visible = true;
};

class CMapStaticBatchObject final : public CGameObject
{
public:
	struct DESC final : public CGameObject::GAMEOBJECT_DESC
	{
		uint32_t PrototypeLevelIndex = ETOUI(LEVEL::ASSET_TEST);
		std::string AssetId;
		std::wstring ModelPrototypeTag;
		MAP_ASSET_RENDER_PROFILE RenderProfile;
		bool_t Mirrored = false;
		std::vector<FMapStaticInstance> Instances;
	};

private:
	CMapStaticBatchObject(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	CMapStaticBatchObject(
		const CMapStaticBatchObject& prototype);

public:
	virtual ~CMapStaticBatchObject();

	virtual HRESULT Initialize_Prototype() override;
	//Clone을 할 때 사용하는 Initialize
	virtual HRESULT Initialize(void* pArg) override;

	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	//Instance Update
	HRESULT Update_Instance(
		uint64_t placementId,
		const FMapStaticInstance& instance);

	HRESULT Set_InstanceVisible(
		uint64_t placementId,
		bool_t visible);

	uint32_t Get_VisibleInstanceCount() const
	{
		return static_cast<uint32_t>(
			m_VisibleInstances.size());
	}

	const std::string& Get_AssetId() const
	{
		return m_AssetId;
	}

	bool_t Is_Mirrored() const
	{
		return m_bMirrored;
	}

private:
	HRESULT Ready_Components(uint32_t prototypeLevelIndex,
		const std::wstring& modelPrototypeTag);

	HRESULT Ensure_InstanceCapacity(
		uint32_t requiredCount);

	HRESULT Upload_VisibleInstances();
	HRESULT Rebuild_PlacementLookup();

private:
	//배치하는 에셋의 ID
	std::string m_AssetId;
	//BatckObject의 Render Profile 정보 
	MAP_ASSET_RENDER_PROFILE m_RenderProfile;

	bool_t m_bMirrored = false;
	f32_t m_fElapsedTime = {};
	//해당 batch에 소속된 전체 placement
	//vector - 메모리 연속, 순회가 빠름, index O(1), 프레임마다 전체 순회 좋음
	std::vector<FMapStaticInstance>	m_Instances;
	//이번 프레임에 실제 보이는 GPU용 인스턴스 배열
	std::vector<VTXMESHINSTANCE> m_VisibleInstances;
	//placementId로 m_Instances의 index를 O(1)로 찾을 수 있게 한다.
	std::unordered_map<uint64_t, uint32_t>
		m_PlacementLookup;

	uint32_t m_iInstanceCapacity = {};
	
	ComPtr<ID3D11Buffer> m_pInstanceBuffer = { nullptr };

	shared_ptr<Engine::CModel> m_pModelCom = { nullptr };
	shared_ptr<Engine::CShader> m_pShaderCom = { nullptr };

public:
	static unique_ptr<CMapStaticBatchObject> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

	virtual shared_ptr<CPrototype> Clone(
		void* pArg) override;
};

NS_END
