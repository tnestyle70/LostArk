# LostArk Scene Kind 배치와 Valtan 저장/로드 구현 계획

작성일: 2026-07-29  
상태: PLAN ONLY  
대상: `LEVEL::ASSET_TEST`, `CMapTool`, `CValtan`

## 1. C1~C8 관점

| 축 | 이번 작업에서 보는 내용 |
|---|---|
| C1 기준계 | 맵 `.wmodel`의 `0.01f`, 캐릭터 `.wmodel`의 `0.0001f`, 월드 Transform은 서로 분리한다. |
| C2 이동>계산 | 모델 변환은 Converter/Prototype 로드 시 끝내고, 에디터는 asset ID와 Transform만 저장한다. |
| C3 공유는 비싸다 | Catalog 정의는 한 번만 보관하고 배치마다 경로·Prototype tag를 복제하지 않는다. |
| **C4 수명은 선언된다** | Prototype은 Level 로딩 수명, Clone은 Layer 수명, `CMapTool`의 `shared_ptr`는 편집 수명이다. |
| C5 이산화와 오차 | 회전은 degree, 저장은 `setprecision(9)`, scale은 각 축 `0.001f` 이상으로 제한한다. |
| C6 가지치기 | Loader는 `STATIC_MODEL`만 모델 Prototype으로 등록하고, Palette는 kind와 문자열로 필터링한다. |
| **C7 권위와 정합성** | Catalog가 asset 정의의 정본이고 Scene 파일이 placement의 정본이다. Runtime은 둘을 조합한 결과다. |
| **C8 검증이 병목** | v1 로드, v2 저장, 실패 rollback, 발탄 Idle, Level 재진입까지 확인해야 완료다. |

## 2. 문제 해결 ①~⑤

① 문제·제약: 현재 `CMapTool`은 모든 항목을 `CMapAssetObject`로만 Clone하므로 `CValtan -> CBody_Valtan -> Idle animation` 경로를 저장·복원할 수 없다. 기존 128개 맵 에셋과 v1 placement는 보존해야 한다.  
② 단순 해법의 문제: 발탄 `.wmodel`을 정적 맵 항목처럼 넣으면 본과 애니메이션이 없는 오브젝트가 되고, Level에서 발탄을 별도 자동 생성하면 저장 파일과 중복된다.  
③ 해결 방식: Catalog에 `kind`와 `spawnType`을 넣고, Factory가 `STATIC_MODEL`은 `CMapAssetObject`, `GAME_OBJECT/BOSS`는 `CValtan`을 Clone한다. 두 객체는 `IScenePlaceable`로 편집한다.  
④ 비교: 수업의 Prototype/Clone/Layer는 그대로 사용한다. Winters의 종류별 Manager를 복사하지 않고, Winters가 분리한 정의/인스턴스/런타임 소유권만 Catalog/Scene/Layer에 적용한다.  
⑤ 대가: enum, 공통 인터페이스, Factory가 추가된다. 실제 생성 타입이 두 종류뿐인 현재는 switch가 작지만 Monster/Marker가 늘면 handler 등록 방식으로 다시 분리한다.

## 3. 자료구조·알고리즘 핵심

### 3.1 자료구조

```text
MAP_ASSET_ENTRY                   SCENE_PLACEMENT_RECORD
------------------------------    ------------------------------
assetId (stable key)              placementId (scene unique key)
label                             assetId -> Catalog 참조
kind                              kind -> Catalog와 일치 검증
spawnType                         position / rotation / scale
model/gameObject prototype tag    visible
defaultScale / anchor

Catalog vector<MAP_ASSET_ENTRY>   MapTool vector<PLACED_ENTRY>
            |                                  |
            +---- assetId join ----------------+
                                               |
                                      Prototype -> Clone -> Layer
```

- `MAP_ASSET_ENTRY`는 “무엇을 만들 수 있는가”만 가진다.
- `SCENE_PLACEMENT_RECORD`는 “무엇이 어디에 놓였는가”만 가진다.
- `PLACED_ENTRY`는 저장 record와 실제 `CGameObject`, `IScenePlaceable`, Layer tag를 연결한다.
- 저장 파일에는 경로와 Prototype tag를 쓰지 않는다. 같은 정보를 Catalog와 Scene에 중복시키지 않는다.

### 3.2 불변식

1. Catalog의 `assetId`와 model Prototype tag는 중복될 수 없다.
2. placement ID는 0이 아니며 한 Scene 안에서 중복될 수 없다.
3. Scene의 `kind`는 같은 `assetId`의 Catalog kind와 같아야 한다.
4. position/rotation/scale은 유한수이고 scale의 각 축은 0보다 커야 한다.
5. `STATIC_MODEL`만 `.wmodel` 경로와 model Prototype tag를 가진다.
6. `GAME_OBJECT`는 기존 GameObject Prototype을 Clone하며 모델 경로를 직접 저장하지 않는다.
7. Load 중 하나라도 Clone에 실패하면 새로 만든 객체를 모두 지우고 기존 Scene을 유지한다.

### 3.3 알고리즘과 복잡도

```text
생성: Palette 선택 -> Picking -> record 작성 -> Catalog 조회 -> Factory Clone -> Layer 등록
저장: P개 runtime 조회/검증 -> .tmp 기록 -> ReplaceFileW 또는 MoveFileExW
로드: P개 parse/검증 -> P개 임시 Clone -> 성공 시 기존 P개 제거/교체 -> 실패 시 rollback
```

- Palette 출력: `O(K * A)`, K는 enum 종류 수라 상수, A는 asset 수다.
- 현재 `Find(assetId)`: `O(A)`. A=128 수준에서는 충분하다.
- 저장과 로드: `O(P)`, Runtime 참조 메모리: `O(P)`.
- A가 수천 개로 늘어 실제 검색이 병목이 될 때만 `unordered_map<assetId, index>`를 Catalog에 추가한다.

## 4. 추가·수정·삭제 파일

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/ScenePlacementTypes.h` | kind, spawn type, Transform, 저장 record |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/ScenePlacementTypes.cpp` | enum 문자열 변환과 파싱 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/IScenePlaceable.h` | Map과 Boss 공통 Inspector 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/ScenePlacementFactory.h` | 종류별 Clone 결과 구조와 API |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/ScenePlacementFactory.cpp` | `CMapAssetObject`/`CValtan` 생성과 Layer 선택 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapAssetCatalog.h` | Catalog entry 확장 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapAssetCatalog.cpp` | v2 Catalog 파싱과 검증 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapAssetObject.h` | `IScenePlaceable` 구현 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapAssetObject.cpp` | 공통 Transform/Visible adapter |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Valtan.h` | placement 정보와 공통 인터페이스 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Valtan.cpp` | 저장 Transform 적용과 visible 제어 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h` | 이종 객체를 보관하는 placement entry |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp` | kind UI, Factory 생성, v1/v2 저장·로드 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp` | 정적 모델만 model Prototype 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_AssetTest.h` | 자동 발탄 생성 선언 삭제 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_AssetTest.cpp` | 자동 발탄 생성 제거 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/BG_RAD_VALTAN_A.mapassets` | v2 전환과 `BOSS_VALTAN` 정의 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | 새 파일 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | 새 파일 Filter 등록 |

`CBody_Valtan`, Picking, ImGui viewport, Camera, Engine 코드는 변경하지 않는다.

## 5. 파일별 전체 구현 코드

### 5-1. `C:/Users/user/Desktop/LostArk/Client/Public/ScenePlacementTypes.h`

변경 종류: 새 파일 전체

```cpp
#pragma once

#include "Client_Defines.h"

#include <string>

NS_BEGIN(Client)

enum class SCENE_ASSET_KIND
{
	STRUCTURE,
	PROP,
	BOSS,
	MONSTER,
	PLAYER_SPAWN,
	MONSTER_SPAWN,
	PORTAL,
	TRIGGER,
	END,
};

enum class SCENE_SPAWN_TYPE
{
	STATIC_MODEL,
	GAME_OBJECT,
	MARKER,
	END,
};

struct SCENE_TRANSFORM
{
	float3_t position = {};
	float3_t rotationDegrees = {};
	float3_t scale = float3_t(1.f, 1.f, 1.f);
};

struct SCENE_PLACEMENT_RECORD
{
	uint64_t placementId = {};
	std::string assetId;
	SCENE_ASSET_KIND kind = SCENE_ASSET_KIND::END;
	SCENE_TRANSFORM transform;
	bool_t visible = true;
};

const char* ToString(SCENE_ASSET_KIND kind);
bool_t TryParseSceneAssetKind(const std::string& text,
	SCENE_ASSET_KIND& outKind);

const char* ToString(SCENE_SPAWN_TYPE type);
bool_t TryParseSceneSpawnType(const std::string& text,
	SCENE_SPAWN_TYPE& outType);

NS_END
```

### 5-2. `C:/Users/user/Desktop/LostArk/Client/Private/MapAssetObject.cpp`

변경 종류: 파일 전체 교체

```cpp
#include "MapAssetObject.h"

#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"

CMapAssetObject::CMapAssetObject(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CMapAssetObject::~CMapAssetObject()
{
}

HRESULT CMapAssetObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapAssetObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const MAP_ASSET_DESC desc = *static_cast<MAP_ASSET_DESC*>(pArg);
	if (0 == desc.placementId || desc.assetId.empty() ||
		desc.modelPrototypeTag.empty())
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)) ||
		FAILED(Ready_Components(desc.modelPrototypeTag)))
		return E_FAIL;

	m_iPlacementId = desc.placementId;
	m_AssetId = desc.assetId;
	m_bApplyBottomCenter = desc.applyBottomCenter;
	m_bVisible = desc.visible;
	Set_SceneTransform(desc.transform);
	return S_OK;
}

void CMapAssetObject::Late_Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (!m_bVisible)
		return;

	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CMapAssetObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t meshIndex = 0;
		meshIndex < m_pModelCom->Get_NumMeshes(); ++meshIndex)
	{
		const uint32_t hasNormalTexture =
			m_pModelCom->Has_MaterialTexture(
				meshIndex, aiTextureType_NORMALS) ? 1u : 0u;
		if (FAILED(m_pModelCom->Bind_Material(
			m_pShaderCom, "g_DiffuseTexture", meshIndex,
			aiTextureType_DIFFUSE)) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_HasNormalTexture", &hasNormalTexture,
				sizeof(hasNormalTexture))) ||
			(0 != hasNormalTexture && FAILED(m_pModelCom->Bind_Material(
				m_pShaderCom, "g_NormalTexture", meshIndex,
				aiTextureType_NORMALS))) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(meshIndex)))
			return E_FAIL;
	}

	return S_OK;
}

SCENE_TRANSFORM CMapAssetObject::Get_SceneTransform() const
{
	return m_SceneTransform;
}

void CMapAssetObject::Set_SceneTransform(
	const SCENE_TRANSFORM& transform)
{
	m_SceneTransform = transform;
	const float3_t worldOrigin = Compute_WorldOrigin(transform);
	m_pTransformCom->Scale(
		transform.scale.x, transform.scale.y, transform.scale.z);
	m_pTransformCom->Rotation(
		transform.rotationDegrees.x,
		transform.rotationDegrees.y,
		transform.rotationDegrees.z);
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(worldOrigin.x, worldOrigin.y, worldOrigin.z, 1.f));
}

HRESULT CMapAssetObject::Ready_Components(
	const std::wstring& modelPrototypeTag)
{
	if (FAILED(__super::Add_Component(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		TEXT("Com_Shader"), m_pShaderCom)) ||
		FAILED(__super::Add_Component(
			ETOUI(LEVEL::ASSET_TEST), modelPrototypeTag,
			TEXT("Com_Model"), m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapAssetObject::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(
		m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;

	return S_OK;
}

float3_t CMapAssetObject::Compute_WorldOrigin(
	const SCENE_TRANSFORM& transform) const
{
	float3_t worldOrigin = transform.position;
	if (m_bApplyBottomCenter && m_pModelCom->Has_LocalBounds())
	{
		const float3_t& minimum = m_pModelCom->Get_LocalBoundsMin();
		const float3_t& maximum = m_pModelCom->Get_LocalBoundsMax();
		const vector_t localAnchor = XMVectorSet(
			(minimum.x + maximum.x) * 0.5f,
			minimum.y,
			(minimum.z + maximum.z) * 0.5f,
			1.f);
		const matrix_t scaleMatrix = XMMatrixScaling(
			transform.scale.x, transform.scale.y, transform.scale.z);
		const matrix_t rotationMatrix = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(transform.rotationDegrees.x),
			XMConvertToRadians(transform.rotationDegrees.y),
			XMConvertToRadians(transform.rotationDegrees.z));
		float3_t anchorOffset{};
		XMStoreFloat3(&anchorOffset,
			XMVector3TransformCoord(
				localAnchor, scaleMatrix * rotationMatrix));
		worldOrigin.x -= anchorOffset.x;
		worldOrigin.y -= anchorOffset.y;
		worldOrigin.z -= anchorOffset.z;
	}

	return worldOrigin;
}

unique_ptr<CMapAssetObject> CMapAssetObject::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CMapAssetObject>(
		new CMapAssetObject(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
		return nullptr;
	return pInstance;
}

shared_ptr<CPrototype> CMapAssetObject::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CMapAssetObject>(
		new CMapAssetObject(*this));
	if (FAILED(pInstance->Initialize(pArg)))
		return nullptr;
	return pInstance;
}
```

### 5-3. `C:/Users/user/Desktop/LostArk/Client/Public/Valtan.h`

변경 종류: 파일 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"
#include "IScenePlaceable.h"

NS_BEGIN(Client)

class CValtan final
	: public CContainerObject
	, public IScenePlaceable
{
public:
	typedef struct tagValtanDesc
		: public CContainerObject::CONTAINEROBJECT_DESC
	{
		uint64_t placementId = {};
		std::string assetId = "BOSS_VALTAN";
		SCENE_TRANSFORM transform;
		bool_t visible = true;
	} VALTAN_DESC;

	enum VALTAN_STATE
	{
		IDLE = 0x00000001,
	};

private:
	CValtan(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CValtan();

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual uint64_t Get_PlacementId() const override { return m_iPlacementId; }
	virtual const std::string& Get_AssetId() const override { return m_AssetId; }
	virtual SCENE_TRANSFORM Get_SceneTransform() const override;
	virtual bool_t Is_SceneVisible() const override { return m_bVisible; }
	virtual void Set_SceneTransform(const SCENE_TRANSFORM& transform) override;
	virtual void Set_SceneVisible(bool_t visible) override { m_bVisible = visible; }

private:
	uint32_t m_iState = { VALTAN_STATE::IDLE };
	uint64_t m_iPlacementId = {};
	std::string m_AssetId;
	SCENE_TRANSFORM m_SceneTransform;
	bool_t m_bVisible = true;

private:
	HRESULT Ready_PartObjects();

public:
	static unique_ptr<CValtan> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### 5-4. `C:/Users/user/Desktop/LostArk/Client/Private/Valtan.cpp`

변경 종류: 파일 전체 교체

```cpp
#include "Valtan.h"

#include "Body_Valtan.h"

CValtan::CValtan(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CContainerObject { pDevice, pContext }
{
}

CValtan::~CValtan()
{
}

HRESULT CValtan::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CValtan::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	VALTAN_DESC desc = *static_cast<VALTAN_DESC*>(pArg);
	if (0 == desc.placementId || desc.assetId.empty())
		return E_FAIL;

	desc.fSpeedPerSec = 0.f;
	desc.fRotationPerSec = 0.f;
	if (FAILED(__super::Initialize(&desc)))
		return E_FAIL;

	m_iPlacementId = desc.placementId;
	m_AssetId = desc.assetId;
	m_bVisible = desc.visible;
	Set_SceneTransform(desc.transform);
	return Ready_PartObjects();
}

void CValtan::Priority_Update(f32_t fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CValtan::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CValtan::Late_Update(f32_t fTimeDelta)
{
	if (!m_bVisible)
		return;
	__super::Late_Update(fTimeDelta);
}

HRESULT CValtan::Render()
{
	return S_OK;
}

SCENE_TRANSFORM CValtan::Get_SceneTransform() const
{
	return m_SceneTransform;
}

void CValtan::Set_SceneTransform(const SCENE_TRANSFORM& transform)
{
	m_SceneTransform = transform;
	m_pTransformCom->Scale(
		transform.scale.x, transform.scale.y, transform.scale.z);
	m_pTransformCom->Rotation(
		transform.rotationDegrees.x,
		transform.rotationDegrees.y,
		transform.rotationDegrees.z);
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(transform.position.x,
			transform.position.y,
			transform.position.z, 1.f));
}

HRESULT CValtan::Ready_PartObjects()
{
	CBody_Valtan::BODY_VALTAN_DESC bodyDesc{};
	bodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	bodyDesc.pParentState = &m_iState;

	return __super::Add_PartObject(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Body_Valtan"),
		TEXT("Part_Body"),
		&bodyDesc);
}

unique_ptr<CValtan> CValtan::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CValtan>(
		new CValtan(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CValtan");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CValtan::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CValtan>(new CValtan(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CValtan");
		return nullptr;
	}
	return pInstance;
}
```

### 5-5. `C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h`

변경 종류: 파일 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "ScenePlacementTypes.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class IScenePlaceable;

class CMapTool final
{
private:
	enum class PLACEMENT_STATE
	{
		IDLE,
		ARMED,
	};

	struct PLACED_ENTRY
	{
		SCENE_PLACEMENT_RECORD record;
		std::wstring layerTag;
		shared_ptr<CGameObject> gameObject;
		shared_ptr<IScenePlaceable> placeable;
	};

public:
	void Toggle();
	void Update(f32_t fTimeDelta);
	void Render();

	bool IsOpen() const;

private:
	void Handle_LevelTransition(bool_t isAssetTest);
	bool_t Try_PickPlacementPosition(float3_t& outPosition) const;
	bool_t Try_PlaceSelected();
	bool_t Create_Placement(const SCENE_PLACEMENT_RECORD& placement,
		PLACED_ENTRY& outEntry);
	bool_t Remove_Placement(uint64_t placementId);
	void Remove_AllPlacements();
	bool_t Save_Placements();
	bool_t Load_Placements();

	void Render_Toolbar();
	void Render_Palette();
	void Render_Hierarchy();
	void Render_Inspector();
	void Render_DecoderReport() const;

	PLACED_ENTRY* Find_Placement(uint64_t placementId);
	const MAP_ASSET_ENTRY* Get_SelectedAsset() const;

private:
	bool_t m_bOpen = false;
	bool_t m_bWasInAssetTest = false;
	bool_t m_bPreviousMouseDown = false;
	bool_t m_bDirty = false;
	PLACEMENT_STATE m_ePlacementState = PLACEMENT_STATE::IDLE;

	CMapAssetCatalog m_Catalog;
	std::string m_SelectedAssetId;
	std::string m_Status = "Enter AssetTest with F2";
	char m_Filter[128]{};

	vector<PLACED_ENTRY> m_Placements;
	uint64_t m_iSelectedPlacementId = {};
	uint64_t m_iNextPlacementId = 1;
};

NS_END
```


### 5-6. `C:/Users/user/Desktop/LostArk/Client/Private/ScenePlacementTypes.cpp`

변경 종류: 새 파일 전체

```cpp
#include "ScenePlacementTypes.h"

NS_BEGIN(Client)

const char* ToString(SCENE_ASSET_KIND kind)
{
	switch (kind)
	{
	case SCENE_ASSET_KIND::STRUCTURE: return "STRUCTURE";
	case SCENE_ASSET_KIND::PROP: return "PROP";
	case SCENE_ASSET_KIND::BOSS: return "BOSS";
	case SCENE_ASSET_KIND::MONSTER: return "MONSTER";
	case SCENE_ASSET_KIND::PLAYER_SPAWN: return "PLAYER_SPAWN";
	case SCENE_ASSET_KIND::MONSTER_SPAWN: return "MONSTER_SPAWN";
	case SCENE_ASSET_KIND::PORTAL: return "PORTAL";
	case SCENE_ASSET_KIND::TRIGGER: return "TRIGGER";
	default: return "END";
	}
}

bool_t TryParseSceneAssetKind(const std::string& text,
	SCENE_ASSET_KIND& outKind)
{
	for (uint32_t value = 0;
		value < static_cast<uint32_t>(SCENE_ASSET_KIND::END); ++value)
	{
		const auto kind = static_cast<SCENE_ASSET_KIND>(value);
		if (text == ToString(kind))
		{
			outKind = kind;
			return true;
		}
	}
	return false;
}

const char* ToString(SCENE_SPAWN_TYPE type)
{
	switch (type)
	{
	case SCENE_SPAWN_TYPE::STATIC_MODEL: return "STATIC_MODEL";
	case SCENE_SPAWN_TYPE::GAME_OBJECT: return "GAME_OBJECT";
	case SCENE_SPAWN_TYPE::MARKER: return "MARKER";
	default: return "END";
	}
}

bool_t TryParseSceneSpawnType(const std::string& text,
	SCENE_SPAWN_TYPE& outType)
{
	for (uint32_t value = 0;
		value < static_cast<uint32_t>(SCENE_SPAWN_TYPE::END); ++value)
	{
		const auto type = static_cast<SCENE_SPAWN_TYPE>(value);
		if (text == ToString(type))
		{
			outType = type;
			return true;
		}
	}
	return false;
}

NS_END
```

### 5-7. `C:/Users/user/Desktop/LostArk/Client/Public/IScenePlaceable.h`

변경 종류: 새 파일 전체

```cpp
#pragma once

#include "ScenePlacementTypes.h"

NS_BEGIN(Client)

class IScenePlaceable
{
public:
	virtual ~IScenePlaceable() = default;

	virtual uint64_t Get_PlacementId() const = 0;
	virtual const std::string& Get_AssetId() const = 0;
	virtual SCENE_TRANSFORM Get_SceneTransform() const = 0;
	virtual bool_t Is_SceneVisible() const = 0;

	virtual void Set_SceneTransform(const SCENE_TRANSFORM& transform) = 0;
	virtual void Set_SceneVisible(bool_t visible) = 0;
};

NS_END
```

### 5-8. `C:/Users/user/Desktop/LostArk/Client/Public/ScenePlacementFactory.h`

변경 종류: 새 파일 전체

```cpp
#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "ScenePlacementTypes.h"

#include <memory>
#include <string>

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class IScenePlaceable;

struct SPAWNED_SCENE_OBJECT
{
	std::wstring layerTag;
	shared_ptr<CGameObject> gameObject;
	shared_ptr<IScenePlaceable> placeable;
};

class CScenePlacementFactory final
{
public:
	static bool_t Spawn(uint32_t levelIndex,
		const MAP_ASSET_ENTRY& asset,
		const SCENE_PLACEMENT_RECORD& placement,
		SPAWNED_SCENE_OBJECT& outObject,
		std::string& outError);

	static std::wstring Get_LayerTag(SCENE_ASSET_KIND kind);
};

NS_END
```

### 5-9. `C:/Users/user/Desktop/LostArk/Client/Private/ScenePlacementFactory.cpp`

변경 종류: 새 파일 전체

```cpp
#include "ScenePlacementFactory.h"

#include "GameInstance.h"
#include "IScenePlaceable.h"
#include "MapAssetObject.h"
#include "Valtan.h"

bool_t Client::CScenePlacementFactory::Spawn(uint32_t levelIndex,
	const MAP_ASSET_ENTRY& asset,
	const SCENE_PLACEMENT_RECORD& placement,
	SPAWNED_SCENE_OBJECT& outObject,
	std::string& outError)
{
	outObject = {};
	outError.clear();

	if (placement.assetId != asset.id || placement.kind != asset.kind)
	{
		outError = "Placement and catalog identity do not match";
		return false;
	}

	shared_ptr<CGameObject> gameObject;
	const std::wstring layerTag = Get_LayerTag(asset.kind);
	if (layerTag.empty())
	{
		outError = "No layer policy for " + asset.id;
		return false;
	}

	switch (asset.spawnType)
	{
	case SCENE_SPAWN_TYPE::STATIC_MODEL:
	{
		CMapAssetObject::MAP_ASSET_DESC desc{};
		desc.placementId = placement.placementId;
		desc.assetId = placement.assetId;
		desc.modelPrototypeTag = asset.modelPrototypeTag;
		desc.transform = placement.transform;
		desc.applyBottomCenter =
			MAP_ASSET_ANCHOR::BOTTOM_CENTER == asset.anchor;
		desc.visible = placement.visible;

		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			levelIndex, asset.gameObjectPrototypeTag,
			levelIndex, layerTag, &desc, &gameObject)))
		{
			outError = "Failed to clone static model " + asset.id;
			return false;
		}
		break;
	}

	case SCENE_SPAWN_TYPE::GAME_OBJECT:
	{
		if (SCENE_ASSET_KIND::BOSS != asset.kind ||
			L"Prototype_GameObject_Valtan" != asset.gameObjectPrototypeTag)
		{
			outError = "GameObject spawn is not implemented for " + asset.id;
			return false;
		}

		CValtan::VALTAN_DESC desc{};
		desc.placementId = placement.placementId;
		desc.assetId = placement.assetId;
		desc.transform = placement.transform;
		desc.visible = placement.visible;

		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			levelIndex, asset.gameObjectPrototypeTag,
			levelIndex, layerTag, &desc, &gameObject)))
		{
			outError = "Failed to clone GameObject " + asset.id;
			return false;
		}
		break;
	}

	default:
		outError = "Spawn type is not implemented for " + asset.id;
		return false;
	}

	auto placeable = dynamic_pointer_cast<IScenePlaceable>(gameObject);
	if (nullptr == placeable)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			levelIndex, layerTag, gameObject);
		outError = "Spawned object does not implement IScenePlaceable";
		return false;
	}

	outObject.layerTag = layerTag;
	outObject.gameObject = std::move(gameObject);
	outObject.placeable = std::move(placeable);
	return true;
}

std::wstring Client::CScenePlacementFactory::Get_LayerTag(
	SCENE_ASSET_KIND kind)
{
	switch (kind)
	{
	case SCENE_ASSET_KIND::STRUCTURE:
	case SCENE_ASSET_KIND::PROP:
		return L"Layer_MapAsset";
	case SCENE_ASSET_KIND::BOSS:
		return L"Layer_Boss";
	case SCENE_ASSET_KIND::MONSTER:
		return L"Layer_Monster";
	case SCENE_ASSET_KIND::PLAYER_SPAWN:
	case SCENE_ASSET_KIND::MONSTER_SPAWN:
		return L"Layer_EditorMarker";
	case SCENE_ASSET_KIND::PORTAL:
		return L"Layer_Portal";
	case SCENE_ASSET_KIND::TRIGGER:
		return L"Layer_Trigger";
	default:
		return {};
	}
}
```

### 5-10. `C:/Users/user/Desktop/LostArk/Client/Public/MapAssetCatalog.h`

변경 종류: 파일 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "ScenePlacementTypes.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class MAP_ASSET_ANCHOR
{
	ORIGIN,
	BOTTOM_CENTER,
};

struct MAP_ASSET_ENTRY
{
	std::string id;
	std::string label;
	SCENE_ASSET_KIND kind = SCENE_ASSET_KIND::END;
	SCENE_SPAWN_TYPE spawnType = SCENE_SPAWN_TYPE::END;
	std::filesystem::path modelRelativePath;
	std::filesystem::path resolvedModelPath;
	std::wstring modelPrototypeTag;
	std::wstring gameObjectPrototypeTag;
	float3_t defaultScale = float3_t(1.f, 1.f, 1.f);
	MAP_ASSET_ANCHOR anchor = MAP_ASSET_ANCHOR::BOTTOM_CENTER;
};

class CMapAssetCatalog final
{
public:
	bool_t Load_Default();
	bool_t Load(const std::filesystem::path& path);

	const MAP_ASSET_ENTRY* Find(const std::string& assetId) const;
	const std::vector<MAP_ASSET_ENTRY>& Get_Entries() const { return m_Entries; }
	const std::string& Get_AreaId() const { return m_AreaId; }
	const std::string& Get_Status() const { return m_Status; }
	bool_t Is_Ready() const { return m_bReady; }

	static std::filesystem::path Get_DefaultCatalogPath();
	static std::filesystem::path Get_DefaultPlacementPath();

private:
	std::vector<MAP_ASSET_ENTRY> m_Entries;
	std::string m_AreaId;
	std::string m_Status = "Catalog not loaded";
	bool_t m_bReady = false;
};

NS_END
```

### 5-11. `C:/Users/user/Desktop/LostArk/Client/Private/MapAssetCatalog.cpp`

변경 종류: 파일 전체 교체

```cpp
#include "MapAssetCatalog.h"

#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <unordered_set>

namespace
{
	constexpr const char* CATALOG_MAGIC = "LOSTARK_SCENE_ASSET_CATALOG";
	constexpr uint32_t CATALOG_VERSION = 2;
	constexpr uint32_t MAX_ASSET_COUNT = 2048;

	std::filesystem::path GetDataFilePath(const wchar_t* pFileName)
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0 == length || length >= std::size(modulePath))
			return {};

		return (std::filesystem::path(modulePath).parent_path() /
			L"DataFiles" / L"Map" / pFileName).lexically_normal();
	}

	bool_t IsInsideRoot(const std::filesystem::path& root,
		const std::filesystem::path& candidate)
	{
		std::error_code error;
		const std::filesystem::path relative =
			std::filesystem::relative(candidate, root, error);
		if (error || relative.empty() || relative.is_absolute())
			return false;

		const auto first = relative.begin();
		return first != relative.end() && *first != L"..";
	}

	bool_t IsValidScale(const float3_t& scale)
	{
		return std::isfinite(scale.x) && std::isfinite(scale.y) &&
			std::isfinite(scale.z) && scale.x > 0.f &&
			scale.y > 0.f && scale.z > 0.f;
	}

	bool_t IsCompatible(SCENE_ASSET_KIND kind, SCENE_SPAWN_TYPE spawnType)
	{
		switch (spawnType)
		{
		case SCENE_SPAWN_TYPE::STATIC_MODEL:
			return SCENE_ASSET_KIND::STRUCTURE == kind ||
				SCENE_ASSET_KIND::PROP == kind;
		case SCENE_SPAWN_TYPE::GAME_OBJECT:
			return SCENE_ASSET_KIND::BOSS == kind ||
				SCENE_ASSET_KIND::MONSTER == kind;
		case SCENE_SPAWN_TYPE::MARKER:
			return SCENE_ASSET_KIND::PLAYER_SPAWN == kind ||
				SCENE_ASSET_KIND::MONSTER_SPAWN == kind ||
				SCENE_ASSET_KIND::PORTAL == kind ||
				SCENE_ASSET_KIND::TRIGGER == kind;
		default:
			return false;
		}
	}
}

bool_t CMapAssetCatalog::Load_Default()
{
	return Load(Get_DefaultCatalogPath());
}

bool_t CMapAssetCatalog::Load(const std::filesystem::path& path)
{
	m_Entries.clear();
	m_AreaId.clear();
	m_bReady = false;

	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		m_Status = "Catalog missing: " + path.string();
		return false;
	}

	std::string magic;
	uint32_t version = {};
	uint32_t count = {};
	if (!(input >> magic >> version >> std::quoted(m_AreaId) >> count) ||
		magic != CATALOG_MAGIC || version != CATALOG_VERSION ||
		m_AreaId.empty() || 0 == count || count > MAX_ASSET_COUNT)
	{
		m_Status = "Catalog header is invalid";
		return false;
	}

	const std::filesystem::path assetRoot = CRuntimeAssetRoot::Get();
	if (assetRoot.empty() || !std::filesystem::exists(assetRoot))
	{
		m_Status = "LostArk runtime asset root is missing";
		return false;
	}

	std::unordered_set<std::string> ids;
	std::unordered_set<std::wstring> modelPrototypeTags;
	m_Entries.reserve(count);
	for (uint32_t index = 0; index < count; ++index)
	{
		MAP_ASSET_ENTRY entry{};
		std::string kindText;
		std::string spawnTypeText;
		std::string modelPath;
		std::string modelPrototypeTag;
		std::string gameObjectPrototypeTag;
		std::string anchor;
		if (!(input >> std::quoted(entry.id) >> std::quoted(entry.label) >>
			kindText >> spawnTypeText >> std::quoted(modelPath) >>
			std::quoted(modelPrototypeTag) >>
			std::quoted(gameObjectPrototypeTag) >>
			entry.defaultScale.x >> entry.defaultScale.y >>
			entry.defaultScale.z >> anchor))
		{
			m_Status = "Catalog row is truncated at index " +
				std::to_string(index);
			m_Entries.clear();
			return false;
		}

		if (!TryParseSceneAssetKind(kindText, entry.kind) ||
			!TryParseSceneSpawnType(spawnTypeText, entry.spawnType))
		{
			m_Status = "Catalog kind or spawn type is invalid for " + entry.id;
			m_Entries.clear();
			return false;
		}

		entry.modelPrototypeTag.assign(
			modelPrototypeTag.begin(), modelPrototypeTag.end());
		entry.gameObjectPrototypeTag.assign(
			gameObjectPrototypeTag.begin(), gameObjectPrototypeTag.end());
		if (anchor == "Origin")
			entry.anchor = MAP_ASSET_ANCHOR::ORIGIN;
		else if (anchor == "BottomCenter")
			entry.anchor = MAP_ASSET_ANCHOR::BOTTOM_CENTER;
		else
		{
			m_Status = "Unknown placement anchor for " + entry.id;
			m_Entries.clear();
			return false;
		}

		if (entry.id.empty() || entry.label.empty() ||
			entry.gameObjectPrototypeTag.empty() ||
			entry.gameObjectPrototypeTag == L"-" ||
			!IsValidScale(entry.defaultScale) ||
			!IsCompatible(entry.kind, entry.spawnType) ||
			!ids.insert(entry.id).second)
		{
			m_Status = "Catalog validation failed for " + entry.id;
			m_Entries.clear();
			return false;
		}

		if (SCENE_SPAWN_TYPE::STATIC_MODEL == entry.spawnType)
		{
			entry.modelRelativePath =
				std::filesystem::path(modelPath).lexically_normal();
			entry.resolvedModelPath =
				CRuntimeAssetRoot::Resolve(entry.modelRelativePath);
			if (modelPath == "-" || modelPrototypeTag == "-" ||
				entry.modelRelativePath.is_absolute() ||
				entry.modelRelativePath.extension() != L".wmodel" ||
				!modelPrototypeTags.insert(entry.modelPrototypeTag).second ||
				!IsInsideRoot(assetRoot, entry.resolvedModelPath) ||
				!std::filesystem::exists(entry.resolvedModelPath))
			{
				m_Status = "Static model validation failed for " + entry.id;
				m_Entries.clear();
				return false;
			}
		}
		else if (modelPath != "-" || modelPrototypeTag != "-")
		{
			m_Status = "Non-model entry owns a model path for " + entry.id;
			m_Entries.clear();
			return false;
		}

		m_Entries.push_back(std::move(entry));
	}

	std::string trailing;
	if (input >> trailing)
	{
		m_Status = "Catalog contains unexpected trailing data";
		m_Entries.clear();
		return false;
	}

	m_bReady = true;
	m_Status = "Catalog ready: " + std::to_string(m_Entries.size());
	return true;
}

const MAP_ASSET_ENTRY* CMapAssetCatalog::Find(
	const std::string& assetId) const
{
	const auto iter = std::find_if(m_Entries.begin(), m_Entries.end(),
		[&](const MAP_ASSET_ENTRY& entry) { return entry.id == assetId; });
	return iter == m_Entries.end() ? nullptr : &*iter;
}

std::filesystem::path CMapAssetCatalog::Get_DefaultCatalogPath()
{
	return GetDataFilePath(L"BG_RAD_VALTAN_A.mapassets");
}

std::filesystem::path CMapAssetCatalog::Get_DefaultPlacementPath()
{
	return GetDataFilePath(L"BG_RAD_VALTAN_A.mapplacements");
}
```

### 5-12. `C:/Users/user/Desktop/LostArk/Client/Public/MapAssetObject.h`

변경 종류: 파일 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "IScenePlaceable.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CMapAssetObject final
	: public CGameObject
	, public IScenePlaceable
{
public:
	struct MAP_ASSET_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		uint64_t placementId = {};
		std::string assetId;
		std::wstring modelPrototypeTag;
		SCENE_TRANSFORM transform;
		bool_t applyBottomCenter = false;
		bool_t visible = true;
	};

private:
	CMapAssetObject(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CMapAssetObject();

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual uint64_t Get_PlacementId() const override { return m_iPlacementId; }
	virtual const std::string& Get_AssetId() const override { return m_AssetId; }
	virtual SCENE_TRANSFORM Get_SceneTransform() const override;
	virtual bool_t Is_SceneVisible() const override { return m_bVisible; }
	virtual void Set_SceneTransform(const SCENE_TRANSFORM& transform) override;
	virtual void Set_SceneVisible(bool_t visible) override { m_bVisible = visible; }

private:
	uint64_t m_iPlacementId = {};
	std::string m_AssetId;
	SCENE_TRANSFORM m_SceneTransform;
	bool_t m_bApplyBottomCenter = false;
	bool_t m_bVisible = true;

	shared_ptr<CShader> m_pShaderCom = { nullptr };
	shared_ptr<CModel> m_pModelCom = { nullptr };

private:
	HRESULT Ready_Components(const std::wstring& modelPrototypeTag);
	HRESULT Bind_ShaderResources();
	float3_t Compute_WorldOrigin(const SCENE_TRANSFORM& transform) const;

public:
	static unique_ptr<CMapAssetObject> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### 5-13. `C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp`

변경 종류: 파일 전체 교체

```cpp
#include "imgui.h"

#include "MapTool.h"

#include "BinaryAsset/ModelDecoderRegistry.h"
#include "GameInstance.h"
#include "IScenePlaceable.h"
#include "ScenePlacementFactory.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <unordered_set>

namespace
{
	constexpr const char* LEGACY_PLACEMENT_MAGIC =
		"LOSTARK_MAP_PLACEMENTS";
	constexpr uint32_t LEGACY_PLACEMENT_VERSION = 1;
	constexpr const char* PLACEMENT_MAGIC =
		"LOSTARK_SCENE_PLACEMENTS";
	constexpr uint32_t PLACEMENT_VERSION = 2;
	constexpr uint32_t MAX_PLACEMENT_COUNT = 10000;

	bool_t IsFinite(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	bool_t IsValidPlacement(const SCENE_PLACEMENT_RECORD& placement)
	{
		return 0 != placement.placementId && !placement.assetId.empty() &&
			SCENE_ASSET_KIND::END != placement.kind &&
			IsFinite(placement.transform.position) &&
			IsFinite(placement.transform.rotationDegrees) &&
			IsFinite(placement.transform.scale) &&
			placement.transform.scale.x > 0.f &&
			placement.transform.scale.y > 0.f &&
			placement.transform.scale.z > 0.f;
	}

	bool_t MatchesFilter(const std::string& text, const char* pFilter)
	{
		if (nullptr == pFilter || '\0' == *pFilter)
			return true;

		std::string haystack = text;
		std::string needle = pFilter;
		std::transform(haystack.begin(), haystack.end(), haystack.begin(),
			[](unsigned char value)
			{
				return static_cast<char>(std::tolower(value));
			});
		std::transform(needle.begin(), needle.end(), needle.begin(),
			[](unsigned char value)
			{
				return static_cast<char>(std::tolower(value));
			});
		return std::string::npos != haystack.find(needle);
	}

	bool_t ReadPlacementDocument(const std::filesystem::path& path,
		const CMapAssetCatalog& catalog,
		vector<SCENE_PLACEMENT_RECORD>& outPlacements,
		std::string& outStatus)
	{
		outPlacements.clear();
		std::error_code fileError;
		if (!std::filesystem::exists(path, fileError))
		{
			outStatus = "No saved placement file; starting with an empty scene";
			return true;
		}

		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			outStatus = "Could not open placement file: " + path.string();
			return false;
		}

		std::string magic;
		std::string areaId;
		uint32_t version = {};
		uint32_t count = {};
		if (!(input >> magic >> version >> std::quoted(areaId) >> count) ||
			areaId != catalog.Get_AreaId() || count > MAX_PLACEMENT_COUNT)
		{
			outStatus = "Placement header is invalid or belongs to another area";
			return false;
		}

		const bool_t isLegacy =
			magic == LEGACY_PLACEMENT_MAGIC &&
			version == LEGACY_PLACEMENT_VERSION;
		const bool_t isCurrent =
			magic == PLACEMENT_MAGIC && version == PLACEMENT_VERSION;
		if (!isLegacy && !isCurrent)
		{
			outStatus = "Placement version is not supported";
			return false;
		}

		std::unordered_set<uint64_t> ids;
		outPlacements.reserve(count);
		for (uint32_t index = 0; index < count; ++index)
		{
			SCENE_PLACEMENT_RECORD placement{};
			std::string kindText;
			int32_t visible = {};
			if (!(input >> placement.placementId >>
				std::quoted(placement.assetId)))
			{
				outStatus = "Placement row is truncated at index " +
					std::to_string(index);
				outPlacements.clear();
				return false;
			}

			const MAP_ASSET_ENTRY* pAsset =
				catalog.Find(placement.assetId);
			if (nullptr == pAsset)
			{
				outStatus = "Placement references an unknown asset at index " +
					std::to_string(index);
				outPlacements.clear();
				return false;
			}

			if (isCurrent)
			{
				if (!(input >> kindText) ||
					!TryParseSceneAssetKind(kindText, placement.kind))
				{
					outStatus = "Placement kind is invalid at index " +
						std::to_string(index);
					outPlacements.clear();
					return false;
				}
			}
			else
				placement.kind = pAsset->kind;

			if (!(input >>
				placement.transform.position.x >>
				placement.transform.position.y >>
				placement.transform.position.z >>
				placement.transform.rotationDegrees.x >>
				placement.transform.rotationDegrees.y >>
				placement.transform.rotationDegrees.z >>
				placement.transform.scale.x >>
				placement.transform.scale.y >>
				placement.transform.scale.z >> visible))
			{
				outStatus = "Placement row is truncated at index " +
					std::to_string(index);
				outPlacements.clear();
				return false;
			}

			placement.visible = 0 != visible;
			if ((0 != visible && 1 != visible) ||
				!IsValidPlacement(placement) ||
				placement.kind != pAsset->kind ||
				!ids.insert(placement.placementId).second)
			{
				outStatus = "Placement validation failed at index " +
					std::to_string(index);
				outPlacements.clear();
				return false;
			}
			outPlacements.push_back(std::move(placement));
		}

		std::string trailing;
		if (input >> trailing)
		{
			outStatus = "Placement file contains unexpected trailing data";
			outPlacements.clear();
			return false;
		}

		outStatus = isLegacy ?
			"Legacy placement document validated; next Save writes version 2" :
			"Placement document validated";
		return true;
	}

	bool_t CommitTemporaryFile(const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(destination, existsError))
		{
			if (ReplaceFileW(destination.c_str(), temporary.c_str(), nullptr,
				REPLACEFILE_WRITE_THROUGH, nullptr, nullptr))
				return true;
		}

		return MoveFileExW(temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}
}

void Client::CMapTool::Toggle()
{
	m_bOpen = !m_bOpen;
}

void Client::CMapTool::Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) ==
		CGameInstance::Get().Get_CurrentLevelID();
	Handle_LevelTransition(isAssetTest);

	const bool_t mouseDown =
		0 != (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	const bool_t mousePressed = mouseDown && !m_bPreviousMouseDown;
	m_bPreviousMouseDown = mouseDown;

	if (!m_bOpen || !isAssetTest ||
		PLACEMENT_STATE::ARMED != m_ePlacementState)
		return;

	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
	{
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_Status = "Placement cancelled";
		return;
	}

	if (mousePressed && GetForegroundWindow() == g_hWnd &&
		!ImGui::GetIO().WantCaptureMouse)
		Try_PlaceSelected();
}

void Client::CMapTool::Render()
{
	if (!m_bOpen)
		return;

	ImGui::SetNextWindowSize(ImVec2(900.f, 620.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Scene Tool", &m_bOpen))
	{
		ImGui::End();
		return;
	}

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) ==
		CGameInstance::Get().Get_CurrentLevelID();
	ImGui::Text("Level: %s",
		isAssetTest ? "ASSET_TEST" : "Open ASSET_TEST with F2");
	ImGui::SameLine();
	ImGui::Text("| Catalog: %s",
		m_Catalog.Is_Ready() ? "READY" : "NOT READY");
	ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::Separator();

	ImGui::BeginDisabled(!isAssetTest || !m_Catalog.Is_Ready());
	Render_Toolbar();
	if (ImGui::BeginTable("SceneEditorColumns", 3,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn("Asset Palette",
			ImGuiTableColumnFlags_WidthStretch, 0.38f);
		ImGui::TableSetupColumn("Hierarchy",
			ImGuiTableColumnFlags_WidthStretch, 0.27f);
		ImGui::TableSetupColumn("Inspector",
			ImGuiTableColumnFlags_WidthStretch, 0.35f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_Palette();
		ImGui::TableSetColumnIndex(1);
		Render_Hierarchy();
		ImGui::TableSetColumnIndex(2);
		Render_Inspector();
		ImGui::EndTable();
	}
	ImGui::EndDisabled();

	Render_DecoderReport();
	ImGui::End();
}

bool Client::CMapTool::IsOpen() const
{
	return m_bOpen;
}

void Client::CMapTool::Handle_LevelTransition(bool_t isAssetTest)
{
	if (isAssetTest == m_bWasInAssetTest)
		return;

	m_bWasInAssetTest = isAssetTest;
	m_ePlacementState = PLACEMENT_STATE::IDLE;
	m_iSelectedPlacementId = 0;
	m_SelectedAssetId.clear();

	if (!isAssetTest)
	{
		m_Placements.clear();
		m_iNextPlacementId = 1;
		m_bDirty = false;
		m_Status = "Enter AssetTest with F2";
		return;
	}

	if (!m_Catalog.Load_Default())
	{
		m_Status = m_Catalog.Get_Status();
		return;
	}

	m_Status = m_Catalog.Get_Status();
	Load_Placements();
}

bool_t Client::CMapTool::Try_PickPlacementPosition(
	float3_t& outPosition) const
{
	float4_t picked{};
	if (CGameInstance::Get().Picking(picked))
	{
		outPosition = float3_t(picked.x, picked.y, picked.z);
		return IsFinite(outPosition);
	}

	::POINT cursor{};
	if (!GetCursorPos(&cursor) || !ScreenToClient(g_hWnd, &cursor))
		return false;

	const float2_t viewport = CGameInstance::Get().Get_ViewportSize();
	if (cursor.x < 0 || cursor.y < 0 ||
		cursor.x >= static_cast<LONG>(viewport.x) ||
		cursor.y >= static_cast<LONG>(viewport.y))
		return false;

	const matrix_t view = XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::VIEW));
	const matrix_t projection = XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::PROJ));
	const vector_t nearPoint = XMVector3Unproject(
		XMVectorSet(static_cast<float>(cursor.x),
			static_cast<float>(cursor.y), 0.f, 1.f),
		0.f, 0.f, viewport.x, viewport.y, 0.f, 1.f,
		projection, view, XMMatrixIdentity());
	const vector_t farPoint = XMVector3Unproject(
		XMVectorSet(static_cast<float>(cursor.x),
			static_cast<float>(cursor.y), 1.f, 1.f),
		0.f, 0.f, viewport.x, viewport.y, 0.f, 1.f,
		projection, view, XMMatrixIdentity());
	const vector_t direction = farPoint - nearPoint;
	const float directionY = XMVectorGetY(direction);
	if (std::abs(directionY) < 0.00001f)
		return false;

	const float distance = -XMVectorGetY(nearPoint) / directionY;
	if (distance < 0.f)
		return false;

	XMStoreFloat3(&outPosition, nearPoint + direction * distance);
	outPosition.y = 0.f;
	return IsFinite(outPosition);
}

bool_t Client::CMapTool::Try_PlaceSelected()
{
	const MAP_ASSET_ENTRY* pAsset = Get_SelectedAsset();
	if (nullptr == pAsset)
	{
		m_Status = "Select an asset before placing";
		return false;
	}

	float3_t position{};
	if (!Try_PickPlacementPosition(position))
	{
		m_Status = "No valid surface under the cursor";
		return false;
	}

	SCENE_PLACEMENT_RECORD placement{};
	placement.placementId = m_iNextPlacementId;
	placement.assetId = pAsset->id;
	placement.kind = pAsset->kind;
	placement.transform.position = position;
	placement.transform.rotationDegrees = float3_t(0.f, 0.f, 0.f);
	placement.transform.scale = pAsset->defaultScale;
	placement.visible = true;

	PLACED_ENTRY placed{};
	if (!Create_Placement(placement, placed))
		return false;

	m_iSelectedPlacementId = m_iNextPlacementId++;
	m_Placements.push_back(std::move(placed));
	m_ePlacementState = PLACEMENT_STATE::IDLE;
	m_bDirty = true;
	m_Status = "Placed " + pAsset->label;
	return true;
}

bool_t Client::CMapTool::Create_Placement(
	const SCENE_PLACEMENT_RECORD& placement,
	PLACED_ENTRY& outEntry)
{
	const MAP_ASSET_ENTRY* pAsset = m_Catalog.Find(placement.assetId);
	if (nullptr == pAsset || placement.kind != pAsset->kind)
	{
		m_Status = "Placement does not match the catalog";
		return false;
	}

	SPAWNED_SCENE_OBJECT spawned{};
	std::string error;
	if (!CScenePlacementFactory::Spawn(
		ETOUI(LEVEL::ASSET_TEST), *pAsset, placement, spawned, error))
	{
		m_Status = error;
		return false;
	}

	outEntry.record = placement;
	outEntry.layerTag = std::move(spawned.layerTag);
	outEntry.gameObject = std::move(spawned.gameObject);
	outEntry.placeable = std::move(spawned.placeable);
	return true;
}

bool_t Client::CMapTool::Remove_Placement(uint64_t placementId)
{
	const auto iter = std::find_if(m_Placements.begin(), m_Placements.end(),
		[placementId](const PLACED_ENTRY& entry)
		{
			return entry.record.placementId == placementId;
		});
	if (iter == m_Placements.end())
		return false;

	if (FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
		ETOUI(LEVEL::ASSET_TEST), iter->layerTag, iter->gameObject)))
		return false;

	m_Placements.erase(iter);
	if (m_iSelectedPlacementId == placementId)
		m_iSelectedPlacementId = 0;
	m_bDirty = true;
	return true;
}

void Client::CMapTool::Remove_AllPlacements()
{
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::ASSET_TEST), entry.layerTag, entry.gameObject);
	}
	m_Placements.clear();
	m_iSelectedPlacementId = 0;
	m_iNextPlacementId = 1;
	m_bDirty = true;
}

bool_t Client::CMapTool::Save_Placements()
{
	vector<SCENE_PLACEMENT_RECORD> document;
	document.reserve(m_Placements.size());
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		const MAP_ASSET_ENTRY* pAsset =
			m_Catalog.Find(entry.record.assetId);
		if (nullptr == entry.placeable || nullptr == pAsset ||
			entry.record.kind != pAsset->kind ||
			entry.placeable->Get_PlacementId() !=
				entry.record.placementId ||
			entry.placeable->Get_AssetId() != entry.record.assetId)
		{
			m_Status = "Save aborted: placement references are invalid";
			return false;
		}

		SCENE_PLACEMENT_RECORD stored = entry.record;
		stored.transform = entry.placeable->Get_SceneTransform();
		stored.visible = entry.placeable->Is_SceneVisible();
		if (!IsValidPlacement(stored))
		{
			m_Status = "Save aborted: a transform is invalid";
			return false;
		}
		document.push_back(std::move(stored));
	}

	const std::filesystem::path destination =
		CMapAssetCatalog::Get_DefaultPlacementPath();
	const std::filesystem::path temporary =
		destination.wstring() + L".tmp";
	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		m_Status = "Could not create placement directory";
		return false;
	}

	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		m_Status = "Could not create temporary placement file";
		return false;
	}

	output << PLACEMENT_MAGIC << ' ' << PLACEMENT_VERSION << ' '
		<< std::quoted(m_Catalog.Get_AreaId()) << ' '
		<< document.size() << '\n';
	output << std::setprecision(9);
	for (const SCENE_PLACEMENT_RECORD& placement : document)
	{
		output << placement.placementId << ' '
			<< std::quoted(placement.assetId) << ' '
			<< ToString(placement.kind) << ' '
			<< placement.transform.position.x << ' '
			<< placement.transform.position.y << ' '
			<< placement.transform.position.z << ' '
			<< placement.transform.rotationDegrees.x << ' '
			<< placement.transform.rotationDegrees.y << ' '
			<< placement.transform.rotationDegrees.z << ' '
			<< placement.transform.scale.x << ' '
			<< placement.transform.scale.y << ' '
			<< placement.transform.scale.z << ' '
			<< (placement.visible ? 1 : 0) << '\n';
	}
	output.flush();
	const bool_t wroteSuccessfully = output.good();
	output.close();
	if (!wroteSuccessfully ||
		!CommitTemporaryFile(destination, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		m_Status = "Failed to commit placement file atomically";
		return false;
	}

	m_bDirty = false;
	m_Status = "Saved " + std::to_string(document.size()) +
		" scene placements";
	return true;
}

bool_t Client::CMapTool::Load_Placements()
{
	if (!m_Catalog.Is_Ready())
		return false;

	vector<SCENE_PLACEMENT_RECORD> document;
	std::string loadStatus;
	if (!ReadPlacementDocument(
		CMapAssetCatalog::Get_DefaultPlacementPath(),
		m_Catalog, document, loadStatus))
	{
		m_Status = loadStatus;
		return false;
	}

	vector<PLACED_ENTRY> created;
	created.reserve(document.size());
	for (const SCENE_PLACEMENT_RECORD& placement : document)
	{
		PLACED_ENTRY entry{};
		if (!Create_Placement(placement, entry))
		{
			for (const PLACED_ENTRY& rollback : created)
			{
				CGameInstance::Get().Remove_GameObject_from_Layer(
					ETOUI(LEVEL::ASSET_TEST),
					rollback.layerTag, rollback.gameObject);
			}
			m_Status = "Load rolled back while cloning " +
				placement.assetId;
			return false;
		}
		created.push_back(std::move(entry));
	}

	for (const PLACED_ENTRY& old : m_Placements)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::ASSET_TEST), old.layerTag, old.gameObject);
	}
	m_Placements = std::move(created);
	m_iSelectedPlacementId = 0;
	m_iNextPlacementId = 1;
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		m_iNextPlacementId = (std::max)(m_iNextPlacementId,
			entry.record.placementId + 1);
	}
	m_bDirty = false;
	m_Status = loadStatus + "; loaded " +
		std::to_string(m_Placements.size()) + " placements";
	return true;
}

void Client::CMapTool::Render_Toolbar()
{
	if (ImGui::Button("Save"))
		Save_Placements();
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
		Load_Placements();
	ImGui::SameLine();
	if (ImGui::Button("Clear"))
		ImGui::OpenPopup("Clear all placements?");
	ImGui::SameLine();
	ImGui::Text("Objects: %zu%s", m_Placements.size(),
		m_bDirty ? "  *unsaved" : "");

	if (ImGui::BeginPopupModal("Clear all placements?", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(
			"Remove every placed scene object from this level?");
		if (ImGui::Button("Clear all"))
		{
			Remove_AllPlacements();
			m_Status = "Cleared all placements (not saved yet)";
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	if (PLACEMENT_STATE::ARMED == m_ePlacementState)
	{
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"PLACEMENT ARMED: click the world (Esc cancels)");
	}
	ImGui::Separator();
}

void Client::CMapTool::Render_Palette()
{
	ImGui::TextUnformatted("Palette");
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputTextWithHint("##AssetFilter",
		"filter structure, boss, wall, cliff...",
		m_Filter, sizeof(m_Filter));
	ImGui::BeginChild("AssetPaletteList", ImVec2(0.f, 440.f), true);

	for (uint32_t kindIndex = 0;
		kindIndex < static_cast<uint32_t>(SCENE_ASSET_KIND::END);
		++kindIndex)
	{
		const auto kind = static_cast<SCENE_ASSET_KIND>(kindIndex);
		bool_t hasVisibleAsset = false;
		for (const MAP_ASSET_ENTRY& asset : m_Catalog.Get_Entries())
		{
			if (asset.kind == kind &&
				MatchesFilter(asset.label + " " + asset.id, m_Filter))
			{
				hasVisibleAsset = true;
				break;
			}
		}
		if (!hasVisibleAsset)
			continue;

		if (!ImGui::CollapsingHeader(ToString(kind),
			ImGuiTreeNodeFlags_DefaultOpen))
			continue;

		for (const MAP_ASSET_ENTRY& asset : m_Catalog.Get_Entries())
		{
			if (asset.kind != kind ||
				!MatchesFilter(asset.label + " " + asset.id, m_Filter))
				continue;

			ImGui::PushID(asset.id.c_str());
			const bool_t selected = asset.id == m_SelectedAssetId;
			if (ImGui::Selectable(asset.label.c_str(), selected))
			{
				m_SelectedAssetId = asset.id;
				m_ePlacementState = PLACEMENT_STATE::ARMED;
				m_Status = "Selected " + asset.label +
					"; click the rendered surface or Y=0 plane";
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("%s | %s", asset.id.c_str(),
					ToString(asset.spawnType));
				if (!asset.modelRelativePath.empty())
				{
					ImGui::TextWrapped("%s",
						asset.modelRelativePath.string().c_str());
				}
				ImGui::EndTooltip();
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
}

void Client::CMapTool::Render_Hierarchy()
{
	ImGui::TextUnformatted("Hierarchy");
	ImGui::BeginChild("PlacementHierarchy", ImVec2(0.f, 475.f), true);
	for (const PLACED_ENTRY& entry : m_Placements)
	{
		const MAP_ASSET_ENTRY* pAsset =
			m_Catalog.Find(entry.record.assetId);
		const std::string assetLabel =
			nullptr == pAsset ? entry.record.assetId : pAsset->label;
		const std::string label = "[" +
			std::string(ToString(entry.record.kind)) + "] " + assetLabel;
		ImGui::PushID(reinterpret_cast<void*>(
			static_cast<uintptr_t>(entry.record.placementId)));
		const bool_t selected =
			entry.record.placementId == m_iSelectedPlacementId;
		if (ImGui::Selectable(label.c_str(), selected))
			m_iSelectedPlacementId = entry.record.placementId;
		ImGui::SameLine();
		ImGui::TextDisabled("#%llu",
			static_cast<unsigned long long>(entry.record.placementId));
		ImGui::PopID();
	}
	ImGui::EndChild();
}

void Client::CMapTool::Render_Inspector()
{
	ImGui::TextUnformatted("Inspector");
	PLACED_ENTRY* pEntry = Find_Placement(m_iSelectedPlacementId);
	if (nullptr == pEntry || nullptr == pEntry->placeable)
	{
		ImGui::TextDisabled("Select a placed object.");
		return;
	}

	ImGui::Text("Placement #%llu",
		static_cast<unsigned long long>(pEntry->record.placementId));
	ImGui::Text("Kind: %s", ToString(pEntry->record.kind));
	ImGui::TextWrapped("Asset: %s", pEntry->record.assetId.c_str());

	SCENE_TRANSFORM transform = pEntry->placeable->Get_SceneTransform();
	bool_t visible = pEntry->placeable->Is_SceneVisible();
	bool_t changed = false;
	changed |= ImGui::DragFloat3(
		"Position", &transform.position.x, 0.1f);
	changed |= ImGui::DragFloat3(
		"Rotation", &transform.rotationDegrees.x, 0.5f);
	if (ImGui::DragFloat3(
		"Scale", &transform.scale.x, 0.01f, 0.001f, 1000.f))
	{
		transform.scale.x = (std::max)(transform.scale.x, 0.001f);
		transform.scale.y = (std::max)(transform.scale.y, 0.001f);
		transform.scale.z = (std::max)(transform.scale.z, 0.001f);
		changed = true;
	}
	if (changed)
	{
		pEntry->placeable->Set_SceneTransform(transform);
		m_bDirty = true;
	}
	if (ImGui::Checkbox("Visible", &visible))
	{
		pEntry->placeable->Set_SceneVisible(visible);
		m_bDirty = true;
	}

	if (ImGui::Button("Delete selected"))
	{
		const uint64_t deletedId = pEntry->record.placementId;
		if (Remove_Placement(deletedId))
		{
			m_Status = "Deleted placement #" +
				std::to_string(deletedId);
		}
	}
}

void Client::CMapTool::Render_DecoderReport() const
{
	ImGui::SeparatorText("Last .wmodel decode");
	const MODEL_DECODE_REPORT report =
		CModelDecoderRegistry::Get().Get_LastReport();
	if (report.meshPath.empty())
	{
		ImGui::TextDisabled(
			"No binary model decode has been requested yet.");
		return;
	}

	ImGui::Text("Status: %s | Decoder: %s",
		report.succeeded ? "LOADED" : "FAILED",
		report.decoderName.empty() ?
			"not recognized" : report.decoderName.c_str());
	ImGui::TextWrapped("Source: %s", report.meshPath.string().c_str());
	if (report.succeeded)
	{
		ImGui::Text(
			"Meshes: %u | Materials: %u | Vertices: %llu | Indices: %llu",
			report.meshCount, report.materialCount,
			static_cast<unsigned long long>(report.vertexCount),
			static_cast<unsigned long long>(report.indexCount));
	}
	else
		ImGui::TextWrapped("Reason: %s", report.error.c_str());
}

Client::CMapTool::PLACED_ENTRY* Client::CMapTool::Find_Placement(
	uint64_t placementId)
{
	const auto iter = std::find_if(m_Placements.begin(), m_Placements.end(),
		[placementId](const PLACED_ENTRY& entry)
		{
			return entry.record.placementId == placementId;
		});
	return iter == m_Placements.end() ? nullptr : &*iter;
}

const MAP_ASSET_ENTRY* Client::CMapTool::Get_SelectedAsset() const
{
	return m_Catalog.Find(m_SelectedAssetId);
}
```

### 5-14. `C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp`

변경 종류: `CLoader::Ready_For_Level_AssetTest` 함수 전체 교체

```cpp
HRESULT CLoader::Ready_For_Level_AssetTest()
{
	lstrcpy(m_szLoadingText,
		TEXT("바이너리 에셋 테스트 자원을 로딩중입니다."));

	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
		CShader::Create(m_pDevice, m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl"),
			VTXANIMMESH::Elements,
			VTXANIMMESH::iNumElements))))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		CShader::Create(m_pDevice, m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
			VTXMESH::Elements,
			VTXMESH::iNumElements))))
		return E_FAIL;

	const matrix_t lostArkAssetPreTransform =
		XMMatrixScaling(0.0001f, 0.0001f, 0.0001f);
	const matrix_t mapAssetTransform =
		XMMatrixScaling(0.01f, 0.01f, 0.01f);

	CMapAssetCatalog mapCatalog;
	if (!mapCatalog.Load_Default())
	{
		OutputDebugStringA(
			("[SceneAsset] " + mapCatalog.Get_Status() + "\n").c_str());
		return E_FAIL;
	}

	for (const MAP_ASSET_ENTRY& entry : mapCatalog.Get_Entries())
	{
		if (SCENE_SPAWN_TYPE::STATIC_MODEL != entry.spawnType)
			continue;

		const string modelPath = entry.resolvedModelPath.string();
		if (FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::ASSET_TEST), entry.modelPrototypeTag,
			CModel::Create(m_pDevice, m_pContext,
				MODEL::NONANIM, modelPath.c_str(), mapAssetTransform))))
		{
			OutputDebugStringA(("[SceneAsset] Prototype registration failed: " +
				entry.id + "\n").c_str());
			return E_FAIL;
		}
	}

	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_Component_Model_Valtan"),
		CModel::Create(m_pDevice, m_pContext,
			MODEL::ANIM,
			"../Bin/Resources/LostArk/Character/MN_RPBF_01/MN_RPBF_01.wmodel",
			lostArkAssetPreTransform))))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Camera_Free"),
		CCamera_Free::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Body_Valtan"),
		CBody_Valtan::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Valtan"),
		CValtan::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_MapAsset"),
		CMapAssetObject::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	lstrcpy(m_szLoadingText,
		TEXT("바이너리 에셋 테스트 로딩이 완료되었습니다."));
	m_isFinished = true;
	return S_OK;
}
```

### 5-15. `C:/Users/user/Desktop/LostArk/Client/Public/Level_AssetTest.h`

변경 종류: 파일 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CLevel_AssetTest final : public CLevel
{
private:
	CLevel_AssetTest(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_AssetTest();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);

public:
	static unique_ptr<CLevel_AssetTest> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
```

### 5-16. `C:/Users/user/Desktop/LostArk/Client/Private/Level_AssetTest.cpp`

변경 종류: 파일 전체 교체

```cpp
#include "Level_AssetTest.h"

#include "Camera_Free.h"
#include "GameInstance.h"

CLevel_AssetTest::CLevel_AssetTest(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel { pDevice, pContext }
{
}

CLevel_AssetTest::~CLevel_AssetTest()
{
}

HRESULT CLevel_AssetTest::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;
	return S_OK;
}

void CLevel_AssetTest::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
}

HRESULT CLevel_AssetTest::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

#ifdef _DEBUG
	SetWindowText(g_hWnd, TEXT("LostArk Scene Asset Test"));
#endif
	return S_OK;
}

HRESULT CLevel_AssetTest::Ready_Lights()
{
	LIGHT_DESC light{};
	light.eType = LIGHT::DIRECTIONAL;
	light.vDirection = float4_t(0.5f, -1.f, 0.5f, 0.f);
	light.vDiffuse = float4_t(0.8f, 0.8f, 0.8f, 1.f);
	light.vAmbient = float4_t(0.35f, 0.35f, 0.35f, 1.f);
	light.vSpecular = float4_t(0.5f, 0.5f, 0.5f, 1.f);
	return CGameInstance::Get().Add_Light(light);
}

HRESULT CLevel_AssetTest::Ready_Layer_Camera(
	const wstring_t& strLayerTag)
{
	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(-18.f, 10.f, -18.f);
	cameraDesc.vAt = float3_t(0.f, 3.f, 0.f);
	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = 1000.f;
	cameraDesc.fSpeedPerSec = 20.f;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::ASSET_TEST), strLayerTag, &cameraDesc)))
		return E_FAIL;

	return S_OK;
}

unique_ptr<CLevel_AssetTest> CLevel_AssetTest::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CLevel_AssetTest>(
		new CLevel_AssetTest(pDevice, pContext));
	if (FAILED(pInstance->Initialize()))
		return nullptr;
	return pInstance;
}
```

### 5-17. `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/BG_RAD_VALTAN_A.mapassets`

변경 종류: v1의 128개 행 보존 후 v2 필드 추가와 Boss 1행 추가

최종 문법은 다음과 같다.

```text
LOSTARK_SCENE_ASSET_CATALOG 2 "BG_RAD_VALTAN_A" 129
"assetId" "label" KIND SPAWN_TYPE "modelPath-or--" "modelPrototype-or--" "gameObjectPrototype" scaleX scaleY scaleZ Anchor
```

현재 128개 정적 행에는 `STRUCTURE STATIC_MODEL`과 `Prototype_GameObject_MapAsset`을 넣는다. 마지막에 발탄 1행을 넣는다. 아래 PowerShell은 기존 행을 누락 없이 같은 순서로 변환하는 전체 적용 코드다.

```powershell
$catalogPath = 'C:\Users\user\Desktop\LostArk\Client\Bin\DataFiles\Map\BG_RAD_VALTAN_A.mapassets'
$lines = Get-Content -LiteralPath $catalogPath -Encoding utf8
$header = $lines[0] -split ' ', 4
if ($header[0] -ne 'LOSTARK_MAP_ASSET_CATALOG' -or $header[1] -ne '1') {
    throw 'Expected the version 1 map asset catalog.'
}
$staticCount = $lines.Count - 1
if ([int]$header[3] -ne $staticCount) {
    throw 'The catalog header count does not match the row count.'
}

$converted = [System.Collections.Generic.List[string]]::new()
$converted.Add(
    'LOSTARK_SCENE_ASSET_CATALOG 2 "BG_RAD_VALTAN_A" {0}' -f
    ($staticCount + 1))
$rowPattern = '^"([^"]+)"\s+"([^"]+)"\s+"([^"]+)"\s+"([^"]+)"\s+(.+)$'
foreach ($row in $lines[1..($lines.Count - 1)]) {
    if ($row -notmatch $rowPattern) {
        throw "Invalid version 1 catalog row: $row"
    }
    $converted.Add(
        '"{0}" "{1}" STRUCTURE STATIC_MODEL "{2}" "{3}" "Prototype_GameObject_MapAsset" {4}' -f
        $Matches[1], $Matches[2], $Matches[3], $Matches[4], $Matches[5])
}
$converted.Add('"BOSS_VALTAN" "Valtan" BOSS GAME_OBJECT "-" "-" "Prototype_GameObject_Valtan" 1 1 1 Origin')

$temporaryPath = "$catalogPath.tmp"
[System.IO.File]::WriteAllLines(
    $temporaryPath,
    $converted,
    [System.Text.UTF8Encoding]::new($false))
Move-Item -LiteralPath $temporaryPath -Destination $catalogPath -Force
```

변환 뒤 첫 정적 행은 정확히 다음 형태다.

```text
LOSTARK_SCENE_ASSET_CATALOG 2 "BG_RAD_VALTAN_A" 129
"BG_RAD_VALTAN_CRYSTAL01_SM" "Valtan Crystal 01" STRUCTURE STATIC_MODEL "Map/BG_RAD_VALTAN_A/BG_RAD_VALTAN_CRYSTAL01_SM/BG_RAD_VALTAN_CRYSTAL01_SM.wmodel" "Prototype_Component_Model_Map_BG_RAD_VALTAN_CRYSTAL01_SM" "Prototype_GameObject_MapAsset" 1 1 1 BottomCenter
```

마지막 행은 정확히 다음 형태다.

```text
"BOSS_VALTAN" "Valtan" BOSS GAME_OBJECT "-" "-" "Prototype_GameObject_Valtan" 1 1 1 Origin
```

위 변환 코드가 첫 행과 마지막 행 사이의 현재 정적 128개 행을 같은 순서로 전부 생성한다.

### 5-18. placement v1/v2 실제 형식

현재 v1 파일은 코드가 그대로 읽으므로 미리 수정하지 않는다.

```text
LOSTARK_MAP_PLACEMENTS 1 "BG_RAD_VALTAN_A" 1
2 "BG_RAD_VALTAN_STATUE01_SM" 10.1929998 0 -2.72199988 0 0 0 3 3 3 1
```

Map Tool에서 발탄을 배치하고 Save하면 같은 파일이 v2로 원자적 교체된다.

```text
LOSTARK_SCENE_PLACEMENTS 2 "BG_RAD_VALTAN_A" 2
2 "BG_RAD_VALTAN_STATUE01_SM" STRUCTURE 10.1929998 0 -2.72199988 0 0 0 3 3 3 1
3 "BOSS_VALTAN" BOSS 0 0 0 0 0 0 1 1 1 1
```

## 6. Visual Studio 프로젝트 등록 코드

### 6.1 `Client.vcxproj`

기존 `<ClInclude>` ItemGroup에 다음 전체 항목을 추가한다.

```xml
<ClInclude Include="..\Public\IScenePlaceable.h" />
<ClInclude Include="..\Public\ScenePlacementFactory.h" />
<ClInclude Include="..\Public\ScenePlacementTypes.h" />
```

기존 `<ClCompile>` ItemGroup에 다음 전체 항목을 추가한다.

```xml
<ClCompile Include="..\Private\ScenePlacementFactory.cpp" />
<ClCompile Include="..\Private\ScenePlacementTypes.cpp" />
```

### 6.2 `Client.vcxproj.filters`

기존 `<ClInclude>` ItemGroup에 다음을 추가한다.

```xml
<ClInclude Include="..\Public\IScenePlaceable.h">
  <Filter>02.GameObjects\02. World</Filter>
</ClInclude>
<ClInclude Include="..\Public\ScenePlacementFactory.h">
  <Filter>03. Tools\00. Map</Filter>
</ClInclude>
<ClInclude Include="..\Public\ScenePlacementTypes.h">
  <Filter>03. Tools\00. Map</Filter>
</ClInclude>
```

기존 `<ClCompile>` ItemGroup에 다음을 추가한다.

```xml
<ClCompile Include="..\Private\ScenePlacementFactory.cpp">
  <Filter>03. Tools\00. Map</Filter>
</ClCompile>
<ClCompile Include="..\Private\ScenePlacementTypes.cpp">
  <Filter>03. Tools\00. Map</Filter>
</ClCompile>
```

새 Filter는 만들지 않는다. 현재 존재하는 `02.GameObjects\02. World`와 `03. Tools\00. Map`을 사용한다.

## 7. 적용 순서와 검증

### 7.1 적용 순서

1. `ScenePlacementTypes`, `IScenePlaceable`, `ScenePlacementFactory` 파일을 추가한다.
2. `MapAssetCatalog`, `MapAssetObject`, `Valtan`, `MapTool`을 위 코드로 교체한다.
3. `Loader::Ready_For_Level_AssetTest`와 `Level_AssetTest`를 교체한다.
4. Catalog 변환 PowerShell을 한 번 실행한다.
5. `.vcxproj`와 `.filters`에 새 파일을 등록한다.
6. 빌드 후 F2/F1 수동 검증을 수행한다.

### 7.2 빌드 명령

Visual Studio Developer PowerShell에서 실행한다.

```powershell
Set-Location 'C:\Users\user\Desktop\LostArk'
msbuild .\Engine\Default\Engine.vcxproj /m /p:Configuration=Debug /p:Platform=x64
.\UpdateLib.bat Debug
msbuild .\Client\Default\Client.vcxproj /m /p:Configuration=Debug /p:Platform=x64
```

이번 변경은 Client 전용이므로 Engine 코드가 바뀌지 않았다면 첫 두 단계는 회귀 확인이고, 실제 컴파일 대상은 Client다.

### 7.3 런타임 검증

1. Client 실행 후 F2로 `ASSET_TEST`에 진입한다.
2. Level 진입만 했을 때 원점 발탄이 자동 생성되지 않아야 한다.
3. F1로 Scene Tool을 열고 Catalog 상태가 `Catalog ready: 129`인지 확인한다.
4. `STRUCTURE`에서 기존 맵 에셋을 선택하고 월드를 클릭해 생성한다.
5. `BOSS`에서 Valtan을 선택하고 월드를 클릭한다.
6. 발탄이 `CValtan -> CBody_Valtan`으로 생성되고 `idle_battle_1`이 반복 재생되는지 확인한다.
7. Inspector에서 두 종류 모두 Position/Rotation/Scale/Visible이 바뀌는지 확인한다.
8. Save 후 파일 헤더가 `LOSTARK_SCENE_PLACEMENTS 2`이고 각 행에 kind가 있는지 확인한다.
9. Reload 후 같은 placement ID, Transform, visible과 발탄 Idle이 복원되는지 확인한다.
10. Level을 나갔다가 F2로 다시 들어와도 저장된 수만큼만 생성되는지 확인한다.

### 7.4 실패/rollback 검증

1. 정상 Scene을 로드한 상태에서 placement 파일 복사본을 만든다.
2. 테스트 복사본 한 행의 asset ID를 존재하지 않는 값으로 바꾼다.
3. Reload를 누르면 `Placement references an unknown asset`이 표시되어야 한다.
4. Reload 전에 화면에 있던 정상 객체가 사라지거나 일부만 교체되면 실패다.
5. 중복 placement ID, `scale 0`, Catalog와 다른 kind도 각각 전체 Load를 거부해야 한다.

이 계획의 완료 기준은 “컴파일 성공”이 아니라 `정적 맵 + 발탄 Idle + 공통 Inspector + v1 로드 + v2 저장 + 실패 rollback`이 한 흐름에서 확인되는 것이다.
