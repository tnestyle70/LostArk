# LostArk Navigation 셀 편집·베이크와 Blade 피킹 이동 구현 계획

작성일: 2026-07-29  
상태: PLAN ONLY — 소스 미반영  
대상: `LEVEL::ASSET_TEST`, `CMapTool`, `CNavigation`, Blade 테스트 캐릭터  
선행 문서: `2026-07-29_LOSTARK_SCENE_KIND_PLACEMENT_VALTAN_PLAN.md`

> 이 문서에서 “애니 취소”는 LoL의 Annie 캐릭터를 사용하지 않는다는 뜻이다. LostArk Blade의 `Idle/Run` 애니메이션 재생은 `.wmodel`에 해당 클립이 있을 때 그대로 검증한다.

## 1. C1~C8 관점

| 축 | 이번 작업에서 보는 내용 |
|---|---|
| **C1 기준계** | 피킹된 실제 월드 `x/y/z`를 셀 정점으로 쓴다. `y=0`으로 강제하지 않고 `CCell::Compute_Height`로 캐릭터 높이를 맞춘다. |
| C2 이동>계산 | 셀 이웃 계산과 유효성 검사는 에디터의 Bake 시 끝내고 런타임은 저장된 이웃 인덱스를 읽는다. |
| C3 공유는 비싸다 | 에디터 초안과 런타임 `CNavigation`은 서로 다른 소유물이며, 저장 파일만 공유 계약으로 둔다. |
| **C4 수명은 선언된다** | Navigation Prototype은 AssetTest Level 수명, Blade Navigation Component는 Clone 수명, 편집 초안은 `CMapTool` 수명이다. |
| **C5 이산화와 오차** | 정점은 0.05 월드 단위 안에서 기존 정점으로 스냅하고, edge key는 0.001 단위 정수로 양자화한다. |
| C6 가지치기 | 1차는 삼각 셀 이동 제한만 구현한다. A*, 동적 재베이크, 파괴 셀 갱신은 제외한다. |
| C7 권위와 정합성 | `ValtanNavigation.dat`가 보행 셀의 정본이다. 맵 placement와 구조물 kind는 별도 Scene 문서의 정본이다. |
| **C8 검증이 병목** | 저장 후 재실행, 유효/무효 클릭, 경계 이탈 차단, Blade Idle/Run 전환까지 확인한다. |

이번 작업의 핵심 축은 C1, C4, C5, C8이다.

## 2. 문제 해결 ①~⑤

① 문제·제약: 현재 `CNavigation`은 삼각 셀 이동은 지원하지만 편집기가 없고, `CMainApp::Ready_Gara()`가 시작할 때 `Navigation.dat`를 덮어쓴다.  
② 단순 해법의 문제: Winters의 512×512 `CNavGrid`를 그대로 이식하면 수업의 `CCell/CNavigation/CTransform::Go_Straight`와 런타임 경로가 이중화된다.  
③ 해결 방식: Winters의 카테고리·편집 UX·정본 분리만 가져오고, LostArk는 세 번 피킹한 삼각형을 버전 파일로 Bake하여 기존 `CNavigation`이 읽게 한다.  
④ 비교: 수업 코드의 Triangle Cell/Prototype/Clone은 유지한다. Winters처럼 Map·Structure·Navigation·Actor Test 모드를 분리하지만 비트 그리드와 A*는 복사하지 않는다.  
⑤ 대가: 포맷 헤더, 셀 편집기, Blade/Body 클래스가 추가된다. 셀 수가 커져 선형 목표 검색이나 직접 추적이 병목일 때만 공간 인덱스/A*를 추가한다.

## 3. 확정 범위와 제외 범위

### 3.1 이번 1차에서 구현

```text
F2 -> LEVEL::ASSET_TEST
F1 -> LostArk Map Tool
Category: Navigation
  LMB 3회 -> 삼각 셀 1개
  기존 정점 근처 LMB -> 정점 스냅
  RMB -> 아직 완성하지 않은 점 1개 취소
  Bake & Save -> ValtanNavigation.dat 원자적 교체
Level 재진입
  Navigation Prototype 로드
  Blade Clone 생성
RMB 월드 클릭
  Picking -> 셀 위 목표점 투영 -> 직선 Chase -> 경계 차단
```

- `Map`: 현재 다른 작업에서 관리 중인 맵 에셋 배치 기능이다.
- `Structure`: 선행 Scene Kind 계획의 구조물 분류다. 이번 문서는 저장 포맷을 중복 정의하지 않는다.
- `Navigation`: 이번 문서가 추가하는 셀 편집·Bake 기능이다.
- `Actor Test`: Blade의 현재 셀, 목표 유무, `IDLE/MOVE` 상태를 확인하는 모드다.

### 3.2 이번 1차에서 제외

- 파괴 구조물에 따른 런타임 셀 재삼각화
- A* 경로 탐색과 장애물 우회
- 발탄 패턴, 공격 Trigger, 물리 파괴
- `MapAssetPreview.*`, `MapAssetCatalog.*`, `BG_RAD_VALTAN_A.mapassets` 변경
- 기존 발탄 17개 카탈로그와 프리뷰 작업 변경

2차에서는 구조물마다 `blockedCellIds` 또는 `blockedPortalIds`를 저장하고, 파괴 전/후 링크 활성 상태만 바꾼다. 매 파괴마다 지오메트리를 다시 Bake하지 않는다.

## 4. 자료구조·알고리즘 핵심

### 4.1 데이터 소유권

```text
맵/구조물 정의        배치 Scene               Navigation Bake          런타임
Catalog               .mapplacements           ValtanNavigation.dat     Prototype/Clone
assetId/kind      +    placementId/Transform +  cell points/neighbors -> CNavigation + CBlade
```

- 맵과 구조물은 “무엇이 어디에 놓였는가”를 저장한다.
- Navigation은 “어디를 걸을 수 있는가”만 저장한다.
- Blade는 둘을 소유하지 않고 Navigation Component를 참조해 이동만 판단한다.

### 4.2 파일 포맷

```text
NAVIGATION_FILE_HEADER (16 bytes)
  magic      = 'LNAV'
  version    = 1
  cellCount
  reserved   = 0

NAVIGATION_CELL_DATA (cellCount개, 각 48 bytes)
  float3 points[3]
  int32  neighbors[3]   // AB, BC, CA, 경계는 -1
```

기존 수업의 헤더 없는 `float3[3]` 연속 파일은 계속 읽는다. 새 에디터가 저장하는 파일만 `LNAV v1`을 사용한다.

### 4.3 불변식

1. 셀은 유한한 정점 3개를 가지며 XZ 투영 넓이가 `0.0001f`보다 커야 한다.
2. 위에서 볼 때 기존 `CCell::isIn` 규칙과 맞도록 법선 Y가 양수가 되게 와인딩을 보정한다.
3. 이웃은 동일 edge를 공유하는 셀 두 개만 연결한다. edge를 세 셀 이상 공유하면 non-manifold로 Bake를 거부한다.
4. 모든 이웃 인덱스는 `-1` 또는 `[0, cellCount)`다.
5. Blade 시작점과 클릭 목표점은 반드시 셀 하나에 포함되어야 한다.
6. 저장 실패 시 기존 `ValtanNavigation.dat`를 보존한다.

### 4.4 알고리즘과 복잡도

- 정점 스냅: 현재 셀 정점 전체를 선형 검색한다. 셀 수 `C`일 때 클릭당 `O(C)`이고 초기 전장 규모에는 충분하다.
- 이웃 Bake: 양자화한 두 정점을 정렬한 `EdgeKey`로 `unordered_map`을 구성하므로 평균 `O(C)`다. 기존 `SetUp_Neighbors()`의 `O(C²)`는 레거시 파일에만 남는다.
- 목표점 찾기: 셀을 선형 검색해 포함 셀을 찾으므로 클릭당 `O(C)`다.
- 이동: 프레임마다 현재 셀과 연결 이웃을 따라가므로 통과 셀 수를 `K`라 할 때 `O(K)`다.
- 1차 이동은 직선이며 우회하지 않는다. 장애물이 생기는 시점에 portal graph/A*를 별도 추가한다.

## 5. 추가·수정·생성 파일

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/Desktop/LostArk/Engine/Public/NavigationData.h` | 버전 Navigation 파일 구조 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Navigation.h` | 메모리 셀 생성과 목표점 투영 API |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Navigation.cpp` | LNAV/레거시 로드와 안전한 셀 검색 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/NavigationCellEditor.h` | 셀 초안·Bake 편집기 선언 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/NavigationCellEditor.cpp` | 피킹 점 추가, 스냅, 이웃 계산, 원자 저장 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/Blade.h` | Navigation을 가진 테스트 캐릭터 컨테이너 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/Blade.cpp` | RMB 피킹 목표와 셀 제한 이동 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/Body_Blade.h` | Blade 애니메이션/렌더 Part |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/Body_Blade.cpp` | Idle/Run 선택과 AnimMesh 렌더 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h` | `Navigation/Actor Test` 모드와 editor 소유 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp` | 셀 편집 입력·패널 연동 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp` | Navigation 자동 덮어쓰기 제거와 월드 입력 소유권 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp` | Navigation, Blade Model, Blade GameObject Prototype |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_AssetTest.h` | `Ready_Blade` 선언 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_AssetTest.cpp` | Bake 파일 존재 시 Blade 생성 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Default/Engine.vcxproj` | Engine 헤더 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Default/Engine.vcxproj.filters` | Navigation 필터 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | Client 파일과 데이터 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | Tool/Character/Data 필터 등록 |
| 생성 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Navigation/ValtanNavigation.dat` | 에디터가 Bake하는 Git LFS 대상 |
| 공유 팩 | `C:/Users/user/Desktop/LostArk/Client/Bin/Resources/LostArk/Character/Blade/Blade.wmodel` | Converter로 만든 Blade 통합 모델 |

`MapAssetPreview.*`, `MapAssetCatalog.*`, `BG_RAD_VALTAN_A.mapassets`, 발탄 복구 문서는 읽기 전용이다.

## 6. 파일별 구현 코드

### 6-1. `C:/Users/user/Desktop/LostArk/Engine/Public/NavigationData.h`

변경 종류: 새 파일 전체

```cpp
#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

inline constexpr uint32_t NAVIGATION_FILE_MAGIC = 0x56414E4Cu; // LNAV
inline constexpr uint32_t NAVIGATION_FILE_VERSION = 1u;
inline constexpr uint32_t NAVIGATION_MAX_CELL_COUNT = 100000u;

struct NAVIGATION_FILE_HEADER
{
	uint32_t magic = NAVIGATION_FILE_MAGIC;
	uint32_t version = NAVIGATION_FILE_VERSION;
	uint32_t cellCount = {};
	uint32_t reserved = {};
};

struct NAVIGATION_CELL_DATA
{
	float3_t points[3] = {};
	int32_t neighbors[3] = { -1, -1, -1 };
};

static_assert(16 == sizeof(NAVIGATION_FILE_HEADER));
static_assert(48 == sizeof(NAVIGATION_CELL_DATA));

NS_END
```

### 6-2. `C:/Users/user/Desktop/LostArk/Engine/Public/Navigation.h`

변경 종류: include, public/private 선언 교체

```cpp
#pragma once

#include "Component.h"
#include "NavigationData.h"

NS_BEGIN(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct tagNavigationDesc
	{
		int32_t iStartCellIndex = { -1 };
		shared_ptr<class CTransform> pTransformCom;
	} NAVIGATION_DESC;

private:
	CNavigation(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CNavigation();

	virtual HRESULT Initialize_Prototype(const tchar_t* pNavigationDataFiles,
		const tchar_t* pNeighborDataFile);
	HRESULT Initialize_Prototype(const vector<NAVIGATION_CELL_DATA>& cells);
	virtual HRESULT Initialize(void* pArg) override;

	HRESULT SetUp_Neighbors();
	HRESULT SetUp_Neighbors(const tchar_t* pNeighborDataFile);
	bool_t isMove(fvector_t vResultPos);
	void SetUp_OnNavigation(shared_ptr<class CTransform> pTargetTransform);
	bool_t Try_ProjectPosition(fvector_t vPosition, float4_t& outPosition,
		int32_t* pOutCellIndex = nullptr) const;
	bool_t Set_CurrentCellByPosition(fvector_t vPosition);
	size_t Get_CellCount() const { return m_Cells.size(); }

#ifdef _DEBUG
	virtual HRESULT Render() override;
#endif

private:
	HRESULT Build_Cells(const vector<NAVIGATION_CELL_DATA>& cells,
		bool_t useStoredNeighbors);
	HRESULT Ready_DebugResources();
	int32_t Find_CellIndex(fvector_t vPosition) const;

private:
	vector<shared_ptr<class CCell>> m_Cells;
	int32_t m_iCurrentCellIndex = { -1 };
	shared_ptr<class CTransform> m_pTargetTransformCom = {};

#ifdef _DEBUG
	shared_ptr<class CShader> m_pShader = { nullptr };
#endif

public:
	static unique_ptr<CNavigation> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const tchar_t* pNavigationDataFiles,
		const tchar_t* pNeighborDataFile = nullptr);
	static unique_ptr<CNavigation> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		const vector<NAVIGATION_CELL_DATA>& cells);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### 6-3. `C:/Users/user/Desktop/LostArk/Engine/Private/Navigation.cpp`

변경 종류: include/도우미와 아래 함수 교체·추가

```cpp
#include "Navigation.h"
#include "Cell.h"

#include "GameInstance.h"

#include <cmath>
#include <filesystem>
#include <fstream>

namespace
{
	std::filesystem::path ResolveNavigationDataPath(const tchar_t* pDataFile)
	{
		if (nullptr == pDataFile || L'\0' == pDataFile[0])
			return {};

		const std::filesystem::path requestedPath = pDataFile;
		if (requestedPath.is_absolute() || std::filesystem::exists(requestedPath))
			return requestedPath.lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(nullptr, modulePath,
			static_cast<DWORD>(size(modulePath)));
		if (0 == moduleLength || moduleLength >= size(modulePath))
			return requestedPath.lexically_normal();

		return (std::filesystem::path(modulePath).parent_path() /
			requestedPath).lexically_normal();
	}

	bool_t IsValidNavigationCellData(
		const Engine::NAVIGATION_CELL_DATA& cell)
	{
		for (const float3_t& point : cell.points)
		{
			if (!std::isfinite(point.x) || !std::isfinite(point.y) ||
				!std::isfinite(point.z))
				return false;
		}
		const vector_t ab = XMLoadFloat3(&cell.points[1]) -
			XMLoadFloat3(&cell.points[0]);
		const vector_t ac = XMLoadFloat3(&cell.points[2]) -
			XMLoadFloat3(&cell.points[0]);
		return std::fabs(XMVectorGetY(XMVector3Cross(ab, ac))) > 0.0001f;
	}
}

HRESULT CNavigation::Initialize_Prototype(const tchar_t* pNavigationDataFiles,
	const tchar_t* pNeighborDataFile)
{
	const std::filesystem::path path = ResolveNavigationDataPath(pNavigationDataFiles);
	std::ifstream input(path, std::ios::binary);
	if (!input)
		return E_FAIL;

	uint32_t firstWord = {};
	input.read(reinterpret_cast<char*>(&firstWord), sizeof(firstWord));
	if (!input)
		return E_FAIL;
	input.seekg(0, std::ios::beg);

	if (NAVIGATION_FILE_MAGIC == firstWord)
	{
		NAVIGATION_FILE_HEADER header{};
		input.read(reinterpret_cast<char*>(&header), sizeof(header));
		if (!input || NAVIGATION_FILE_VERSION != header.version ||
			0 == header.cellCount || header.cellCount > NAVIGATION_MAX_CELL_COUNT ||
			0 != header.reserved)
			return E_FAIL;

		vector<NAVIGATION_CELL_DATA> cells(header.cellCount);
		input.read(reinterpret_cast<char*>(cells.data()),
			static_cast<std::streamsize>(sizeof(NAVIGATION_CELL_DATA) * cells.size()));
		if (!input || input.peek() != std::char_traits<char>::eof())
			return E_FAIL;

		if (FAILED(Build_Cells(cells, true)))
			return E_FAIL;
	}
	else
	{
		vector<NAVIGATION_CELL_DATA> cells;
		while (true)
		{
			NAVIGATION_CELL_DATA cell{};
			input.read(reinterpret_cast<char*>(cell.points),
				sizeof(cell.points));
			if (input.eof() && 0 == input.gcount())
				break;
			if (!input || sizeof(cell.points) != input.gcount())
				return E_FAIL;
			cells.push_back(cell);
		}

		if (cells.empty() || FAILED(Build_Cells(cells, false)))
			return E_FAIL;
		if (FAILED(nullptr == pNeighborDataFile ?
			SetUp_Neighbors() : SetUp_Neighbors(pNeighborDataFile)))
			return E_FAIL;
	}

	return Ready_DebugResources();
}

HRESULT CNavigation::Initialize_Prototype(
	const vector<NAVIGATION_CELL_DATA>& cells)
{
	if (cells.empty() || FAILED(Build_Cells(cells, true)))
		return E_FAIL;
	return Ready_DebugResources();
}

HRESULT CNavigation::Build_Cells(
	const vector<NAVIGATION_CELL_DATA>& cells,
	bool_t useStoredNeighbors)
{
	if (cells.empty() || cells.size() > NAVIGATION_MAX_CELL_COUNT)
		return E_FAIL;

	m_Cells.clear();
	m_Cells.reserve(cells.size());
	for (size_t index = 0; index < cells.size(); ++index)
	{
		if (!IsValidNavigationCellData(cells[index]))
		{
			m_Cells.clear();
			return E_FAIL;
		}
		auto cell = CCell::Create(m_pDevice, m_pContext,
			cells[index].points, static_cast<int32_t>(index));
		if (nullptr == cell)
		{
			m_Cells.clear();
			return E_FAIL;
		}
		m_Cells.push_back(std::move(cell));
	}

	if (useStoredNeighbors)
	{
		for (size_t index = 0; index < cells.size(); ++index)
		{
			int32_t neighbors[3]{};
			for (uint32_t edge = 0; edge < 3; ++edge)
			{
				neighbors[edge] = cells[index].neighbors[edge];
				if (neighbors[edge] < -1 ||
					neighbors[edge] >= static_cast<int32_t>(cells.size()) ||
					neighbors[edge] == static_cast<int32_t>(index))
					return E_FAIL;
			}
			m_Cells[index]->Set_Neighbor(neighbors);
		}
	}
	return S_OK;
}

HRESULT CNavigation::Ready_DebugResources()
{
#ifdef _DEBUG
	m_pShader = CShader::Create(m_pDevice, m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_Cell.hlsl"),
		VTXPOS::Elements, VTXPOS::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;
#endif
	return S_OK;
}

int32_t CNavigation::Find_CellIndex(fvector_t vPosition) const
{
	for (size_t index = 0; index < m_Cells.size(); ++index)
	{
		int32_t neighbor = -1;
		if (m_Cells[index]->isIn(vPosition, &neighbor))
			return static_cast<int32_t>(index);
	}
	return -1;
}

bool_t CNavigation::Try_ProjectPosition(fvector_t vPosition,
	float4_t& outPosition, int32_t* pOutCellIndex) const
{
	const int32_t cellIndex = Find_CellIndex(vPosition);
	if (cellIndex < 0)
		return false;

	vector_t projected = XMVectorSetY(vPosition,
		m_Cells[cellIndex]->Compute_Height(vPosition));
	projected = XMVectorSetW(projected, 1.f);
	XMStoreFloat4(&outPosition, projected);
	if (nullptr != pOutCellIndex)
		*pOutCellIndex = cellIndex;
	return true;
}

bool_t CNavigation::Set_CurrentCellByPosition(fvector_t vPosition)
{
	const int32_t cellIndex = Find_CellIndex(vPosition);
	if (cellIndex < 0)
		return false;
	m_iCurrentCellIndex = cellIndex;
	return true;
}

bool_t CNavigation::isMove(fvector_t vResultPos)
{
	if (m_iCurrentCellIndex < 0 ||
		m_iCurrentCellIndex >= static_cast<int32_t>(m_Cells.size()))
		return false;

	int32_t neighborIndex = -1;
	if (m_Cells[m_iCurrentCellIndex]->isIn(vResultPos, &neighborIndex))
		return true;

	uint32_t guard = 0;
	while (-1 != neighborIndex && guard++ < m_Cells.size())
	{
		if (neighborIndex < 0 ||
			neighborIndex >= static_cast<int32_t>(m_Cells.size()))
			return false;
		const int32_t testingIndex = neighborIndex;
		if (m_Cells[testingIndex]->isIn(vResultPos, &neighborIndex))
		{
			m_iCurrentCellIndex = testingIndex;
			return true;
		}
	}
	return false;
}

void CNavigation::SetUp_OnNavigation(
	shared_ptr<class CTransform> pTargetTransform)
{
	if (nullptr == pTargetTransform || m_iCurrentCellIndex < 0 ||
		m_iCurrentCellIndex >= static_cast<int32_t>(m_Cells.size()))
		return;

	vector_t position = pTargetTransform->Get_State(STATE::POSITION);
	position = XMVectorSetY(position,
		m_Cells[m_iCurrentCellIndex]->Compute_Height(position));
	pTargetTransform->Set_State(STATE::POSITION, position);
}

unique_ptr<CNavigation> CNavigation::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const vector<NAVIGATION_CELL_DATA>& cells)
{
	auto instance = unique_ptr<CNavigation>(new CNavigation(pDevice, pContext));
	if (FAILED(instance->Initialize_Prototype(cells)))
		return nullptr;
	return instance;
}
```

기존 `Initialize`, `SetUp_Neighbors` 두 함수, `Render`, 파일 경로 `Create`, `Clone`은 유지한다. 단, `Initialize`는 `iStartCellIndex` 범위 검사를 그대로 보존한다.

### 6-4. `C:/Users/user/Desktop/LostArk/Client/Public/NavigationCellEditor.h`

변경 종류: 새 파일 전체

```cpp
#pragma once

#include "Client_Defines.h"
#include "NavigationData.h"

#include <array>
#include <filesystem>
#include <string>

NS_BEGIN(Engine)
class CNavigation;
NS_END

NS_BEGIN(Client)

class CNavigationCellEditor final
{
public:
	HRESULT Initialize(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	bool_t Add_PickedPoint(const float3_t& worldPoint);
	void Undo_PendingPoint();
	void Remove_LastCell();
	void Clear();
	bool_t Load();
	bool_t Bake_And_Save();
	void Submit_DebugRender() const;

	uint32_t Get_PendingPointCount() const { return m_iPendingPointCount; }
	size_t Get_CellCount() const { return m_Cells.size(); }
	bool_t Is_Dirty() const { return m_bDirty; }
	const std::string& Get_Status() const { return m_Status; }
	const std::filesystem::path& Get_Path() const { return m_Path; }

private:
	bool_t Is_ValidCell(const Engine::NAVIGATION_CELL_DATA& cell) const;
	float3_t Snap_Point(const float3_t& point) const;
	bool_t Rebuild_Neighbors(vector<Engine::NAVIGATION_CELL_DATA>& cells,
		std::string& outError) const;
	bool_t Rebuild_Preview();

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	std::filesystem::path m_Path =
		L"../Bin/DataFiles/Navigation/ValtanNavigation.dat";
	vector<Engine::NAVIGATION_CELL_DATA> m_Cells;
	std::array<float3_t, 3> m_PendingPoints{};
	uint32_t m_iPendingPointCount = {};
	bool_t m_bDirty = false;
	std::string m_Status = "Navigation editor is ready";
	shared_ptr<Engine::CNavigation> m_pPreviewNavigation;
};

NS_END
```

### 6-5. `C:/Users/user/Desktop/LostArk/Client/Private/NavigationCellEditor.cpp`

변경 종류: 새 파일 전체

```cpp
#include "NavigationCellEditor.h"

#include "GameInstance.h"
#include "Navigation.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <unordered_map>

namespace
{
	constexpr float SNAP_DISTANCE = 0.05f;
	constexpr float EDGE_QUANTIZE = 1000.f;

	struct QUANTIZED_POINT
	{
		int64_t x = {};
		int64_t y = {};
		int64_t z = {};

		bool operator==(const QUANTIZED_POINT& rhs) const
		{
			return x == rhs.x && y == rhs.y && z == rhs.z;
		}
		bool operator<(const QUANTIZED_POINT& rhs) const
		{
			if (x != rhs.x) return x < rhs.x;
			if (y != rhs.y) return y < rhs.y;
			return z < rhs.z;
		}
	};

	struct EDGE_KEY
	{
		QUANTIZED_POINT a;
		QUANTIZED_POINT b;
		bool operator==(const EDGE_KEY& rhs) const
		{
			return a == rhs.a && b == rhs.b;
		}
	};

	struct EDGE_HASH
	{
		size_t operator()(const EDGE_KEY& value) const
		{
			size_t seed = {};
			auto mix = [&seed](int64_t component)
			{
				seed ^= std::hash<int64_t>{}(component) +
					0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
			};
			mix(value.a.x); mix(value.a.y); mix(value.a.z);
			mix(value.b.x); mix(value.b.y); mix(value.b.z);
			return seed;
		}
	};

	struct EDGE_OWNER
	{
		uint32_t cellIndex = {};
		uint32_t edgeIndex = {};
		bool_t paired = false;
	};

	QUANTIZED_POINT Quantize(const float3_t& point)
	{
		return {
			static_cast<int64_t>(std::llround(point.x * EDGE_QUANTIZE)),
			static_cast<int64_t>(std::llround(point.y * EDGE_QUANTIZE)),
			static_cast<int64_t>(std::llround(point.z * EDGE_QUANTIZE))
		};
	}

	EDGE_KEY MakeEdge(const float3_t& first, const float3_t& second)
	{
		QUANTIZED_POINT a = Quantize(first);
		QUANTIZED_POINT b = Quantize(second);
		if (b < a)
			std::swap(a, b);
		return { a, b };
	}

	bool_t IsFinite(const float3_t& point)
	{
		return std::isfinite(point.x) && std::isfinite(point.y) &&
			std::isfinite(point.z);
	}
}

HRESULT CNavigationCellEditor::Initialize(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	m_pDevice = pDevice;
	m_pContext = pContext;
	return nullptr != m_pDevice && nullptr != m_pContext ? S_OK : E_FAIL;
}

bool_t CNavigationCellEditor::Add_PickedPoint(const float3_t& worldPoint)
{
	if (!IsFinite(worldPoint) || m_iPendingPointCount >= 3)
		return false;

	m_PendingPoints[m_iPendingPointCount++] = Snap_Point(worldPoint);
	if (3 != m_iPendingPointCount)
	{
		m_Status = "Picked navigation point " +
			std::to_string(m_iPendingPointCount) + "/3";
		return true;
	}

	Engine::NAVIGATION_CELL_DATA cell{};
	for (uint32_t index = 0; index < 3; ++index)
		cell.points[index] = m_PendingPoints[index];

	const vector_t ab = XMLoadFloat3(&cell.points[1]) -
		XMLoadFloat3(&cell.points[0]);
	const vector_t ac = XMLoadFloat3(&cell.points[2]) -
		XMLoadFloat3(&cell.points[0]);
	if (XMVectorGetY(XMVector3Cross(ab, ac)) < 0.f)
		std::swap(cell.points[1], cell.points[2]);

	m_iPendingPointCount = 0;
	if (!Is_ValidCell(cell))
	{
		m_Status = "Rejected a degenerate navigation cell";
		return false;
	}

	m_Cells.push_back(cell);
	m_bDirty = true;
	if (!Rebuild_Preview())
	{
		m_Cells.pop_back();
		m_Status = "Could not rebuild navigation preview";
		return false;
	}
	m_Status = "Added navigation cell #" + std::to_string(m_Cells.size() - 1);
	return true;
}

void CNavigationCellEditor::Undo_PendingPoint()
{
	if (0 != m_iPendingPointCount)
		--m_iPendingPointCount;
	m_Status = "Pending navigation point undone";
}

void CNavigationCellEditor::Remove_LastCell()
{
	if (m_Cells.empty())
		return;
	m_Cells.pop_back();
	m_bDirty = true;
	Rebuild_Preview();
	m_Status = "Removed the last navigation cell";
}

void CNavigationCellEditor::Clear()
{
	m_Cells.clear();
	m_iPendingPointCount = 0;
	m_pPreviewNavigation.reset();
	m_bDirty = true;
	m_Status = "Cleared navigation draft";
}

bool_t CNavigationCellEditor::Is_ValidCell(
	const Engine::NAVIGATION_CELL_DATA& cell) const
{
	if (!IsFinite(cell.points[0]) || !IsFinite(cell.points[1]) ||
		!IsFinite(cell.points[2]))
		return false;
	const vector_t ab = XMLoadFloat3(&cell.points[1]) -
		XMLoadFloat3(&cell.points[0]);
	const vector_t ac = XMLoadFloat3(&cell.points[2]) -
		XMLoadFloat3(&cell.points[0]);
	return std::fabs(XMVectorGetY(XMVector3Cross(ab, ac))) > 0.0001f;
}

float3_t CNavigationCellEditor::Snap_Point(const float3_t& point) const
{
	float3_t best = point;
	float bestDistanceSq = SNAP_DISTANCE * SNAP_DISTANCE;
	for (const Engine::NAVIGATION_CELL_DATA& cell : m_Cells)
	{
		for (const float3_t& candidate : cell.points)
		{
			const float dx = point.x - candidate.x;
			const float dy = point.y - candidate.y;
			const float dz = point.z - candidate.z;
			const float distanceSq = dx * dx + dy * dy + dz * dz;
			if (distanceSq <= bestDistanceSq)
			{
				bestDistanceSq = distanceSq;
				best = candidate;
			}
		}
	}
	return best;
}

bool_t CNavigationCellEditor::Rebuild_Neighbors(
	vector<Engine::NAVIGATION_CELL_DATA>& cells,
	std::string& outError) const
{
	std::unordered_map<EDGE_KEY, EDGE_OWNER, EDGE_HASH> edges;
	for (Engine::NAVIGATION_CELL_DATA& cell : cells)
		std::fill(std::begin(cell.neighbors), std::end(cell.neighbors), -1);

	for (uint32_t cellIndex = 0; cellIndex < cells.size(); ++cellIndex)
	{
		for (uint32_t edgeIndex = 0; edgeIndex < 3; ++edgeIndex)
		{
			const uint32_t next = (edgeIndex + 1) % 3;
			const EDGE_KEY key = MakeEdge(
				cells[cellIndex].points[edgeIndex],
				cells[cellIndex].points[next]);
			auto [iter, inserted] = edges.emplace(key,
				EDGE_OWNER{ cellIndex, edgeIndex, false });
			if (inserted)
				continue;
			if (iter->second.paired)
			{
				outError = "Non-manifold edge is shared by more than two cells";
				return false;
			}

			const EDGE_OWNER owner = iter->second;
			cells[cellIndex].neighbors[edgeIndex] =
				static_cast<int32_t>(owner.cellIndex);
			cells[owner.cellIndex].neighbors[owner.edgeIndex] =
				static_cast<int32_t>(cellIndex);
			iter->second.paired = true;
		}
	}
	return true;
}

bool_t CNavigationCellEditor::Rebuild_Preview()
{
	if (m_Cells.empty())
	{
		m_pPreviewNavigation.reset();
		return true;
	}

	vector<Engine::NAVIGATION_CELL_DATA> previewCells = m_Cells;
	std::string error;
	if (!Rebuild_Neighbors(previewCells, error))
	{
		m_Status = error;
		return false;
	}
	auto preview = Engine::CNavigation::Create(m_pDevice, m_pContext, previewCells);
	if (nullptr == preview)
		return false;
	m_pPreviewNavigation = shared_ptr<Engine::CNavigation>(std::move(preview));
	return true;
}

bool_t CNavigationCellEditor::Load()
{
	std::ifstream input(m_Path, std::ios::binary);
	if (!input)
	{
		m_Status = "No baked navigation file; starting empty";
		return true;
	}

	Engine::NAVIGATION_FILE_HEADER header{};
	input.read(reinterpret_cast<char*>(&header), sizeof(header));
	if (!input || Engine::NAVIGATION_FILE_MAGIC != header.magic ||
		Engine::NAVIGATION_FILE_VERSION != header.version ||
		header.cellCount > Engine::NAVIGATION_MAX_CELL_COUNT || 0 != header.reserved)
	{
		m_Status = "Navigation header validation failed";
		return false;
	}

	vector<Engine::NAVIGATION_CELL_DATA> staged(header.cellCount);
	input.read(reinterpret_cast<char*>(staged.data()),
		static_cast<std::streamsize>(sizeof(Engine::NAVIGATION_CELL_DATA) * staged.size()));
	if (!input || input.peek() != std::char_traits<char>::eof())
	{
		m_Status = "Navigation payload is truncated or has trailing data";
		return false;
	}
	for (const auto& cell : staged)
	{
		if (!Is_ValidCell(cell))
		{
			m_Status = "Navigation payload contains an invalid cell";
			return false;
		}
	}

	vector<Engine::NAVIGATION_CELL_DATA> rebuilt = staged;
	std::string error;
	if (!Rebuild_Neighbors(rebuilt, error))
	{
		m_Status = error;
		return false;
	}
	m_Cells = std::move(rebuilt);
	m_iPendingPointCount = 0;
	m_bDirty = false;
	if (!Rebuild_Preview())
		return false;
	m_Status = "Loaded " + std::to_string(m_Cells.size()) + " navigation cells";
	return true;
}

bool_t CNavigationCellEditor::Bake_And_Save()
{
	if (m_Cells.empty() || 0 != m_iPendingPointCount)
	{
		m_Status = "Finish the pending triangle before Bake";
		return false;
	}

	vector<Engine::NAVIGATION_CELL_DATA> baked = m_Cells;
	std::string error;
	if (!Rebuild_Neighbors(baked, error))
	{
		m_Status = error;
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(m_Path.parent_path(), directoryError);
	if (directoryError)
	{
		m_Status = "Could not create the navigation data directory";
		return false;
	}

	const std::filesystem::path temporary = m_Path.wstring() + L".tmp";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		m_Status = "Could not open temporary navigation file";
		return false;
	}

	Engine::NAVIGATION_FILE_HEADER header{};
	header.cellCount = static_cast<uint32_t>(baked.size());
	output.write(reinterpret_cast<const char*>(&header), sizeof(header));
	output.write(reinterpret_cast<const char*>(baked.data()),
		static_cast<std::streamsize>(sizeof(Engine::NAVIGATION_CELL_DATA) * baked.size()));
	output.flush();
	if (!output)
	{
		output.close();
		std::filesystem::remove(temporary);
		m_Status = "Could not write complete navigation data";
		return false;
	}
	output.close();

	if (!MoveFileExW(temporary.c_str(), m_Path.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		std::filesystem::remove(temporary);
		m_Status = "Could not atomically replace navigation data";
		return false;
	}

	m_Cells = std::move(baked);
	m_bDirty = false;
	Rebuild_Preview();
	m_Status = "Baked " + std::to_string(m_Cells.size()) +
		" cells; re-enter AssetTest to load runtime navigation";
	return true;
}

void CNavigationCellEditor::Submit_DebugRender() const
{
#ifdef _DEBUG
	if (nullptr != m_pPreviewNavigation)
		CGameInstance::Get().Add_DebugComponent(m_pPreviewNavigation);
#endif
}
```

### 6-6. `C:/Users/user/Desktop/LostArk/Client/Public/Blade.h`

변경 종류: 새 파일 전체

```cpp
#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"

NS_BEGIN(Engine)
class CCollider;
class CNavigation;
NS_END

NS_BEGIN(Client)

class CBlade final : public CContainerObject
{
public:
	typedef struct tagBladeDesc : public CContainerObject::CONTAINEROBJECT_DESC
	{
		int32_t startCellIndex = 0;
	} BLADE_DESC;

	enum BLADE_STATE
	{
		IDLE = 0x00000001,
		MOVE = 0x00000002,
	};

private:
	CBlade(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CBlade();
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Handle_PickTarget();
	void Move_ToTarget(f32_t fTimeDelta);
	HRESULT Ready_Components(int32_t startCellIndex);
	HRESULT Ready_PartObjects();

private:
	uint32_t m_iState = BLADE_STATE::IDLE;
	bool_t m_bPreviousRightMouseDown = false;
	bool_t m_bHasMoveTarget = false;
	float4_t m_vMoveTarget = {};
	shared_ptr<CNavigation> m_pNavigationCom;
	shared_ptr<CCollider> m_pColliderCom;

public:
	static unique_ptr<CBlade> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### 6-7. `C:/Users/user/Desktop/LostArk/Client/Private/Blade.cpp`

변경 종류: 새 파일 전체

```cpp
#include "Blade.h"

#include "Body_Blade.h"
#include "Bounding_AABB.h"
#include "Collider.h"
#include "GameInstance.h"
#include "Navigation.h"

CBlade::CBlade(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CContainerObject { pDevice, pContext }
{
}

CBlade::~CBlade()
{
}

HRESULT CBlade::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBlade::Initialize(void* pArg)
{
	BLADE_DESC desc{};
	desc.fSpeedPerSec = 5.f;
	desc.fRotationPerSec = 360.f;
	if (nullptr != pArg)
		desc.startCellIndex = static_cast<BLADE_DESC*>(pArg)->startCellIndex;

	if (FAILED(__super::Initialize(&desc)) ||
		FAILED(Ready_Components(desc.startCellIndex)) ||
		FAILED(Ready_PartObjects()))
		return E_FAIL;
	return S_OK;
}

void CBlade::Priority_Update(f32_t fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CBlade::Update(f32_t fTimeDelta)
{
	Handle_PickTarget();
	Move_ToTarget(fTimeDelta);
	m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
	__super::Update(fTimeDelta);
}

void CBlade::Late_Update(f32_t fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
#ifdef _DEBUG
	CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
	CGameInstance::Get().Add_DebugComponent(m_pNavigationCom);
#endif
}

HRESULT CBlade::Render()
{
	return S_OK;
}

void CBlade::Handle_PickTarget()
{
	const bool_t mouseDown = 0 != (GetAsyncKeyState(VK_RBUTTON) & 0x8000);
	const bool_t mousePressed = mouseDown && !m_bPreviousRightMouseDown;
	m_bPreviousRightMouseDown = mouseDown;
	if (!mousePressed || CGameInstance::Get().IsMouseInputBlocked())
		return;

	float4_t picked{};
	float4_t projected{};
	if (!CGameInstance::Get().Picking(picked) ||
		!m_pNavigationCom->Try_ProjectPosition(XMLoadFloat4(&picked), projected))
		return;

	m_vMoveTarget = projected;
	m_bHasMoveTarget = true;
}

void CBlade::Move_ToTarget(f32_t fTimeDelta)
{
	if (!m_bHasMoveTarget)
	{
		m_iState = BLADE_STATE::IDLE;
		return;
	}

	const vector_t position = m_pTransformCom->Get_State(STATE::POSITION);
	const vector_t target = XMLoadFloat4(&m_vMoveTarget);
	vector_t delta = target - position;
	const float distance = XMVectorGetX(XMVector3Length(delta));
	if (distance <= 0.05f)
	{
		m_pTransformCom->Set_State(STATE::POSITION, target);
		m_pNavigationCom->SetUp_OnNavigation(m_pTransformCom);
		m_bHasMoveTarget = false;
		m_iState = BLADE_STATE::IDLE;
		return;
	}

	const vector_t lookTarget = XMVectorSetY(target, XMVectorGetY(position));
	m_pTransformCom->LookAt(lookTarget);
	const float step = (std::min)(distance, 5.f * fTimeDelta);
	vector_t candidate = position + XMVector3Normalize(delta) * step;
	candidate = XMVectorSetW(candidate, 1.f);
	if (!m_pNavigationCom->isMove(candidate))
	{
		m_bHasMoveTarget = false;
		m_iState = BLADE_STATE::IDLE;
		return;
	}

	m_pTransformCom->Set_State(STATE::POSITION, candidate);
	m_pNavigationCom->SetUp_OnNavigation(m_pTransformCom);
	m_iState = BLADE_STATE::MOVE;
}

HRESULT CBlade::Ready_Components(int32_t startCellIndex)
{
	CNavigation::NAVIGATION_DESC navigationDesc{};
	navigationDesc.iStartCellIndex = startCellIndex;
	navigationDesc.pTransformCom = m_pTransformCom;
	if (FAILED(__super::Add_Component(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_Component_ValtanNavigation"),
		TEXT("Com_Navigation"), m_pNavigationCom, &navigationDesc)))
		return E_FAIL;

	CBounding_AABB::BOUNDING_AABB_DESC colliderDesc{};
	colliderDesc.vSize = float3_t(0.8f, 1.8f, 0.8f);
	colliderDesc.vCenter = float3_t(0.f, 0.9f, 0.f);
	if (FAILED(__super::Add_Component(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_Collider_AABB"), m_pColliderCom, &colliderDesc)))
		return E_FAIL;
	return S_OK;
}

HRESULT CBlade::Ready_PartObjects()
{
	CBody_Blade::BODY_BLADE_DESC bodyDesc{};
	bodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	bodyDesc.pParentState = &m_iState;
	return __super::Add_PartObject(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Body_Blade"),
		TEXT("Part_Body"), &bodyDesc);
}

unique_ptr<CBlade> CBlade::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CBlade>(new CBlade(pDevice, pContext));
	if (FAILED(instance->Initialize_Prototype()))
		return nullptr;
	return instance;
}

shared_ptr<CPrototype> CBlade::Clone(void* pArg)
{
	auto instance = shared_ptr<CBlade>(new CBlade(*this));
	if (FAILED(instance->Initialize(pArg)))
		return nullptr;
	return instance;
}
```

### 6-8. `C:/Users/user/Desktop/LostArk/Client/Public/Body_Blade.h`

변경 종류: 새 파일 전체

```cpp
#pragma once

#include "Client_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CBody_Blade final : public CPartObject
{
public:
	typedef struct tagBodyBladeDesc : public CPartObject::PARTOBJECT_DESC
	{
		const uint32_t* pParentState = { nullptr };
	} BODY_BLADE_DESC;

private:
	CBody_Blade(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CBody_Blade();
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Apply_StateAnimation();
	bool_t Try_SetAnimation(const char_t* const* names, size_t count,
		uint32_t fallbackIndex);
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

private:
	shared_ptr<CShader> m_pShaderCom;
	shared_ptr<CModel> m_pModelCom;
	const uint32_t* m_pParentState = { nullptr };
	uint32_t m_iAppliedState = {};

public:
	static unique_ptr<CBody_Blade> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### 6-9. `C:/Users/user/Desktop/LostArk/Client/Private/Body_Blade.cpp`

변경 종류: 새 파일 전체

```cpp
#include "Body_Blade.h"

#include "Blade.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"

CBody_Blade::CBody_Blade(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CBody_Blade::~CBody_Blade()
{
}

HRESULT CBody_Blade::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBody_Blade::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;
	const auto desc = static_cast<BODY_BLADE_DESC*>(pArg);
	m_pParentState = desc->pParentState;
	if (nullptr == m_pParentState || FAILED(__super::Initialize(pArg)) ||
		FAILED(Ready_Components()))
		return E_FAIL;
	Apply_StateAnimation();
	return S_OK;
}

void CBody_Blade::Priority_Update(f32_t fTimeDelta)
{
}

void CBody_Blade::Update(f32_t fTimeDelta)
{
	Apply_StateAnimation();
	if (0 != m_pModelCom->Get_NumAnimations())
		m_pModelCom->Play_Animation(fTimeDelta);
	__super::Update_CombinedWorldMatrix(
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CBody_Blade::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

void CBody_Blade::Apply_StateAnimation()
{
	if (nullptr == m_pParentState || m_iAppliedState == *m_pParentState)
		return;

	if (*m_pParentState & CBlade::BLADE_STATE::MOVE)
	{
		const char_t* names[] = { "run", "run_1", "walk" };
		Try_SetAnimation(names, size(names), 1u);
	}
	else
	{
		const char_t* names[] = { "idle_battle_1", "idle_battle", "idle" };
		Try_SetAnimation(names, size(names), 0u);
	}
	m_iAppliedState = *m_pParentState;
}

bool_t CBody_Blade::Try_SetAnimation(const char_t* const* names,
	size_t count, uint32_t fallbackIndex)
{
	for (size_t index = 0; index < count; ++index)
	{
		if (m_pModelCom->Set_Animation(names[index], true))
			return true;
	}
	const uint32_t animationCount = m_pModelCom->Get_NumAnimations();
	if (0 == animationCount)
		return false;
	m_pModelCom->Set_Animation((std::min)(fallbackIndex,
		animationCount - 1), true);
	return true;
}

HRESULT CBody_Blade::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;
	for (uint32_t meshIndex = 0; meshIndex < m_pModelCom->Get_NumMeshes(); ++meshIndex)
	{
		const uint32_t hasNormal = m_pModelCom->Has_MaterialTexture(
			meshIndex, aiTextureType_NORMALS) ? 1u : 0u;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom,
			"g_DiffuseTexture", meshIndex, aiTextureType_DIFFUSE, 0)) ||
			FAILED(m_pShaderCom->Bind_RawValue("g_HasNormalTexture",
				&hasNormal, sizeof(hasNormal))) ||
			(0 != hasNormal && FAILED(m_pModelCom->Bind_Material(m_pShaderCom,
				"g_NormalTexture", meshIndex, aiTextureType_NORMALS, 0))) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom,
				"g_BoneMatrices", meshIndex)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(meshIndex)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CBody_Blade::Ready_Components()
{
	if (FAILED(__super::Add_Component(ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
		TEXT("Com_Shader"), m_pShaderCom)) ||
		FAILED(__super::Add_Component(ETOUI(LEVEL::ASSET_TEST),
			TEXT("Prototype_Component_Model_Blade"),
			TEXT("Com_Model"), m_pModelCom)))
		return E_FAIL;
	return S_OK;
}

HRESULT CBody_Blade::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom,
			"g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom,
			"g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

unique_ptr<CBody_Blade> CBody_Blade::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CBody_Blade>(new CBody_Blade(pDevice, pContext));
	if (FAILED(instance->Initialize_Prototype()))
		return nullptr;
	return instance;
}

shared_ptr<CPrototype> CBody_Blade::Clone(void* pArg)
{
	auto instance = shared_ptr<CBody_Blade>(new CBody_Blade(*this));
	if (FAILED(instance->Initialize(pArg)))
		return nullptr;
	return instance;
}
```

### 6-10. `C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h`

변경 종류: 파일 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapAssetPreview.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

NS_BEGIN(Client)

class CMapAssetObject;
class CNavigationCellEditor;

class CMapTool final
{
private:
	enum class PLACEMENT_STATE
	{
		IDLE,
		ARMED,
	};

	enum class TOOL_CATEGORY
	{
		MAP,
		STRUCTURE,
		NAVIGATION,
		ACTOR_TEST,
	};

	struct PLACED_ENTRY
	{
		uint64_t placementId = {};
		std::string assetId;
		shared_ptr<CMapAssetObject> object;
	};

public:
	~CMapTool();

	HRESULT Initialize(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	void Toggle();
	void Update(f32_t fTimeDelta);
	void Render();
	bool IsOpen() const;
	bool_t ConsumesWorldMouse() const;

private:
	void Handle_LevelTransition(bool_t isAssetTest);
	bool_t Try_PickPlacementPosition(float3_t& outPosition) const;
	bool_t Try_PlaceSelected();
	bool_t Create_Placement(uint64_t placementId, const std::string& assetId,
		const float3_t& position, const float3_t& rotationDegrees,
		const float3_t& scale, bool_t visible, PLACED_ENTRY& outEntry);
	bool_t Remove_Placement(uint64_t placementId);
	void Remove_AllPlacements();
	bool_t Save_Placements();
	bool_t Load_Placements();

	void Select_Asset(const MAP_ASSET_ENTRY& asset);
	void Arm_SelectedAsset();

	void Render_Toolbar();
	void Render_Palette(f32_t childHeight);
	void Render_Hierarchy(f32_t childHeight);
	void Render_Inspector();
	void Render_AssetPreview();
	void Render_DecoderReport() const;
	void Render_CategoryBar();
	void Render_NavigationPanel();
	void Render_ActorTestPanel();

	PLACED_ENTRY* Find_Placement(uint64_t placementId);
	const MAP_ASSET_ENTRY* Get_SelectedAsset() const;

private:
	bool_t m_bOpen = false;
	bool_t m_bWasInAssetTest = false;
	bool_t m_bPreviousMouseDown = false;
	bool_t m_bPreviousRightMouseDown = false;
	bool_t m_bDirty = false;
	PLACEMENT_STATE m_ePlacementState = PLACEMENT_STATE::IDLE;
	TOOL_CATEGORY m_eToolCategory = TOOL_CATEGORY::MAP;

	CMapAssetCatalog m_Catalog;
	std::unique_ptr<CMapAssetPreview> m_pAssetPreview;
	std::unique_ptr<CNavigationCellEditor> m_pNavigationEditor;
	std::string m_SelectedAssetId;
	std::string m_Status = "Enter AssetTest with F2";
	char m_Filter[128]{};
	std::unordered_set<std::string> m_FavoriteAssetIds;

	vector<PLACED_ENTRY> m_Placements;
	uint64_t m_iSelectedPlacementId = {};
	uint64_t m_iNextPlacementId = 1;
};

NS_END
```

### 6-11. `C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp`

변경 종류: include와 함수 전체 교체·추가

```cpp
#include "NavigationCellEditor.h"
```

```cpp
HRESULT Client::CMapTool::Initialize(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto preview = std::make_unique<CMapAssetPreview>();
	if (FAILED(preview->Initialize(pDevice, pContext)))
		return E_FAIL;

	auto navigationEditor = std::make_unique<CNavigationCellEditor>();
	if (FAILED(navigationEditor->Initialize(pDevice, pContext)))
		return E_FAIL;
	navigationEditor->Load();

	m_pAssetPreview = std::move(preview);
	m_pNavigationEditor = std::move(navigationEditor);
	return S_OK;
}
```

```cpp
void Client::CMapTool::Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) == CGameInstance::Get().Get_CurrentLevelID();
	Handle_LevelTransition(isAssetTest);

	const bool_t mouseDown = 0 != (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	const bool_t mousePressed = mouseDown && !m_bPreviousMouseDown;
	m_bPreviousMouseDown = mouseDown;

	const bool_t rightDown = 0 != (GetAsyncKeyState(VK_RBUTTON) & 0x8000);
	const bool_t rightPressed = rightDown && !m_bPreviousRightMouseDown;
	m_bPreviousRightMouseDown = rightDown;

	if (!m_bOpen || !isAssetTest)
		return;

	if (TOOL_CATEGORY::NAVIGATION == m_eToolCategory)
	{
		if (nullptr != m_pNavigationEditor)
			m_pNavigationEditor->Submit_DebugRender();
		if (ImGui::GetIO().WantCaptureMouse || GetForegroundWindow() != g_hWnd)
			return;

		if (mousePressed)
		{
			float3_t picked{};
			if (Try_PickPlacementPosition(picked))
				m_pNavigationEditor->Add_PickedPoint(picked);
		}
		if (rightPressed)
			m_pNavigationEditor->Undo_PendingPoint();
		return;
	}

	const bool_t placementCategory =
		TOOL_CATEGORY::MAP == m_eToolCategory ||
		TOOL_CATEGORY::STRUCTURE == m_eToolCategory;
	if (placementCategory && PLACEMENT_STATE::ARMED == m_ePlacementState)
	{
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
}
```

```cpp
void Client::CMapTool::Render()
{
	if (!m_bOpen)
		return;

	ImGui::SetNextWindowSize(ImVec2(1180.f, 900.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Map Tool", &m_bOpen))
	{
		ImGui::End();
		return;
	}

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) == CGameInstance::Get().Get_CurrentLevelID();
	ImGui::Text("Level: %s",
		isAssetTest ? "ASSET_TEST" : "Open ASSET_TEST with F2");
	ImGui::SameLine();
	ImGui::Text("| Catalog: %s",
		m_Catalog.Is_Ready() ? "READY" : "NOT READY");
	ImGui::TextWrapped("%s", m_Status.c_str());
	Render_CategoryBar();
	ImGui::Separator();

	ImGui::BeginDisabled(!isAssetTest);
	if (TOOL_CATEGORY::NAVIGATION == m_eToolCategory)
		Render_NavigationPanel();
	else if (TOOL_CATEGORY::ACTOR_TEST == m_eToolCategory)
		Render_ActorTestPanel();
	else
	{
		ImGui::BeginDisabled(!m_Catalog.Is_Ready());
		Render_Toolbar();

		const f32_t availableHeight = ImGui::GetContentRegionAvail().y;
		const f32_t topPanelHeight = (std::max)(
			280.f, (std::min)(480.f, availableHeight * 0.48f));
		if (ImGui::BeginTable("MapEditorColumns", 3,
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
			Render_Palette(topPanelHeight);
			ImGui::TableSetColumnIndex(1);
			Render_Hierarchy(topPanelHeight);
			ImGui::TableSetColumnIndex(2);
			Render_Inspector();
			ImGui::EndTable();
		}
		Render_AssetPreview();
		ImGui::EndDisabled();
	}
	ImGui::EndDisabled();
	ImGui::End();
}
```

```cpp
bool_t CMapTool::ConsumesWorldMouse() const
{
	if (!m_bOpen)
		return false;
	return TOOL_CATEGORY::NAVIGATION == m_eToolCategory ||
		((TOOL_CATEGORY::MAP == m_eToolCategory ||
			TOOL_CATEGORY::STRUCTURE == m_eToolCategory) &&
			PLACEMENT_STATE::ARMED == m_ePlacementState);
}

void CMapTool::Render_CategoryBar()
{
	if (ImGui::RadioButton("Map", TOOL_CATEGORY::MAP == m_eToolCategory))
		m_eToolCategory = TOOL_CATEGORY::MAP;
	ImGui::SameLine();
	if (ImGui::RadioButton("Structure", TOOL_CATEGORY::STRUCTURE == m_eToolCategory))
		m_eToolCategory = TOOL_CATEGORY::STRUCTURE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Navigation", TOOL_CATEGORY::NAVIGATION == m_eToolCategory))
		m_eToolCategory = TOOL_CATEGORY::NAVIGATION;
	ImGui::SameLine();
	if (ImGui::RadioButton("Actor Test", TOOL_CATEGORY::ACTOR_TEST == m_eToolCategory))
		m_eToolCategory = TOOL_CATEGORY::ACTOR_TEST;
}

void CMapTool::Render_NavigationPanel()
{
	if (nullptr == m_pNavigationEditor)
	{
		ImGui::TextUnformatted("Navigation editor is unavailable");
		return;
	}
	ImGui::Text("Cells: %zu | Pending points: %u | Dirty: %s",
		m_pNavigationEditor->Get_CellCount(),
		m_pNavigationEditor->Get_PendingPointCount(),
		m_pNavigationEditor->Is_Dirty() ? "YES" : "NO");
	ImGui::TextWrapped("%s", m_pNavigationEditor->Get_Status().c_str());
	ImGui::TextWrapped("File: %s",
		m_pNavigationEditor->Get_Path().string().c_str());
	ImGui::Separator();
	ImGui::TextUnformatted("LMB: add point (3 points = 1 cell)");
	ImGui::TextUnformatted("RMB: undo pending point");
	if (ImGui::Button("Undo last cell"))
		m_pNavigationEditor->Remove_LastCell();
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
		m_pNavigationEditor->Load();
	ImGui::SameLine();
	if (ImGui::Button("Bake & Save"))
		m_pNavigationEditor->Bake_And_Save();
}

void CMapTool::Render_ActorTestPanel()
{
	ImGui::TextUnformatted("Blade actor test");
	ImGui::TextUnformatted("RMB on a valid baked cell: set move target");
	ImGui::TextUnformatted("Click outside navigation: target is rejected");
	ImGui::TextUnformatted("Bake, leave AssetTest, then press F2 again to reload prototypes");
}
```

`Structure` 내용과 Scene Kind 저장 코드는 선행 계획서에서 한 번만 적용한다. 이 문서가 같은 코드를 다시 소유하지 않는다.

### 6-12. `C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp`

변경 종류: `Update`, `Ready_Gara` 함수 전체 교체

```cpp
void CMainApp::Update(f32_t fTimeDelta)
{
#ifdef _DEBUG
	UpdateDebugToolShortcut();

	if (nullptr != m_pImGuiLayer)
		m_pImGuiLayer->BeginFrame();

	const bool_t bMapToolOpen =
		nullptr != m_pMapTool && m_pMapTool->IsOpen();
	const HWND hForegroundWindow = GetForegroundWindow();
	const bool_t bExternalToolFocused = bMapToolOpen &&
		nullptr != hForegroundWindow &&
		hForegroundWindow != g_hWnd &&
		IsWindowOwnedByCurrentProcess(hForegroundWindow);
	const bool_t bWorldMouseOwnedByTool =
		bMapToolOpen && m_pMapTool->ConsumesWorldMouse();

	const bool_t bKeyboardCaptured = bMapToolOpen &&
		nullptr != m_pImGuiLayer &&
		(m_pImGuiLayer->WantsCaptureKeyboard() || bExternalToolFocused);
	const bool_t bMouseCaptured = bMapToolOpen &&
		nullptr != m_pImGuiLayer &&
		(m_pImGuiLayer->WantsCaptureMouse() || bExternalToolFocused ||
			bWorldMouseOwnedByTool);
	CGameInstance::Get().SetInputBlocked(bKeyboardCaptured, bMouseCaptured);
#endif

	CGameInstance::Get().Update_Engine(fTimeDelta);

#ifdef _DEBUG
	if (nullptr != m_pMapTool)
		m_pMapTool->Update(fTimeDelta);
#endif
}
```

주의: 위 계산으로 `CGameInstance`의 일반 RMB 입력까지 막히므로 `ACTOR_TEST`에서는 `ConsumesWorldMouse()`가 false여야 Blade가 클릭을 받는다.

`Ready_Gara()`는 하드코딩 `Navigation.dat` 생성 블록만 삭제하고 Terrain mask 생성은 유지한다.

```cpp
HRESULT CMainApp::Ready_Gara()
{
	ComPtr<ID3D11Texture2D> pTexture2D = { nullptr };
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = 256;
	textureDesc.Height = 256;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_STAGING;
	textureDesc.CPUAccessFlags =
		D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

	shared_ptr<uint32_t[]> pixels =
		make_shared<uint32_t[]>(textureDesc.Width * textureDesc.Height);
	pixels[0] = 0xffffffff;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = pixels.get();
	initialData.SysMemPitch = textureDesc.Width * sizeof(uint32_t);
	if (FAILED(m_pDevice->CreateTexture2D(
		&textureDesc, &initialData, &pTexture2D)))
		return E_FAIL;

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if (FAILED(m_pContext->Map(pTexture2D.Get(), 0,
		D3D11_MAP_READ_WRITE, 0, &mapped)))
		return E_FAIL;
	auto lockPixels = static_cast<uint32_t*>(mapped.pData);
	for (uint32_t y = 0; y < 256; ++y)
	{
		for (uint32_t x = 0; x < 256; ++x)
			lockPixels[y * 256 + x] = x < 128 ? 0xffffffff : 0xff000000;
	}
	m_pContext->Unmap(pTexture2D.Get(), 0);

	if (FAILED(DirectX::SaveDDSTextureToFile(m_pContext.Get(),
		pTexture2D.Get(),
		TEXT("../Bin/Resources/Textures/Terrain/MyMask.dds"))))
		return E_FAIL;
	return S_OK;
}
```

### 6-13. `C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp`

변경 종류: include 추가와 `Ready_For_Level_AssetTest` 함수 전체 교체

```cpp
#include "Blade.h"
#include "Body_Blade.h"
#include "Collider.h"
#include "Navigation.h"
#include "RuntimeAssetRoot.h"

#include <filesystem>
```

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
			VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		CShader::Create(m_pDevice, m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl"),
			VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::ASSET_TEST),
		CMapAssetPreview::SHADER_PROTOTYPE_TAG,
		CShader::Create(m_pDevice, m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl"),
			VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	const matrix_t lostArkAssetPreTransform =
		XMMatrixScaling(0.0001f, 0.0001f, 0.0001f);
	const matrix_t mapAssetTransform =
		XMMatrixScaling(0.001f, 0.001f, 0.001f);

	CMapAssetCatalog mapCatalog;
	if (!mapCatalog.Load_Default())
		return E_FAIL;
	for (const MAP_ASSET_ENTRY& entry : mapCatalog.Get_Entries())
	{
		const string modelPath = entry.resolvedModelPath.string();
		if (FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::ASSET_TEST), entry.prototypeTag,
			CModel::Create(m_pDevice, m_pContext,
				MODEL::NONANIM, modelPath.c_str(), mapAssetTransform))))
			return E_FAIL;
	}

	if (FAILED(CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_Component_Model_Valtan"),
		CModel::Create(m_pDevice, m_pContext, MODEL::ANIM,
			"../Bin/Resources/LostArk/Character/MN_RPBF_01/MN_RPBF_01.wmodel",
			lostArkAssetPreTransform))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Prototype_GameObject_Camera_Free"),
			CCamera_Free::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Prototype_GameObject_Body_Valtan"),
			CBody_Valtan::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Prototype_GameObject_Valtan"),
			CValtan::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Prototype_GameObject_MapAsset"),
			CMapAssetObject::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	const std::filesystem::path bladePath =
		CRuntimeAssetRoot::Resolve(L"Character/Blade/Blade.wmodel");
	const std::filesystem::path navigationPath =
		L"../Bin/DataFiles/Navigation/ValtanNavigation.dat";
	if (std::filesystem::exists(bladePath) &&
		std::filesystem::exists(navigationPath))
	{
		const string bladeModelPath = bladePath.string();
		if (FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Prototype_Component_ValtanNavigation"),
			CNavigation::Create(m_pDevice, m_pContext,
				navigationPath.c_str()))) ||
			FAILED(CGameInstance::Get().Add_Prototype(
				ETOUI(LEVEL::ASSET_TEST),
				TEXT("Prototype_Component_Collider_AABB"),
				CCollider::Create(m_pDevice, m_pContext,
					COLLIDER::AABB))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Prototype_Component_Model_Blade"),
			CModel::Create(m_pDevice, m_pContext, MODEL::ANIM,
				bladeModelPath.c_str(),
				XMMatrixScaling(0.0001f, 0.0001f, 0.0001f)))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Prototype_GameObject_Body_Blade"),
			CBody_Blade::Create(m_pDevice, m_pContext))) ||
		FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Prototype_GameObject_Blade"),
			CBlade::Create(m_pDevice, m_pContext))))
			return E_FAIL;
	}
	else
	{
		OutputDebugStringA(
			"[AssetTest] Blade or ValtanNavigation.dat is missing; navigation authoring remains available.\n");
	}

	lstrcpy(m_szLoadingText,
		TEXT("바이너리 에셋 테스트 로딩이 완료되었습니다."));
	m_isFinished = true;
	return S_OK;
}
```

Blade는 발탄과 같은 추출 기준으로 `0.0001f`를 우선 사용한다. Converter의 bounds와 발탄을 비교해 단위가 다르면 모델 Prototype의 PreTransform만 조정하고 월드 Transform/Navigation 단위는 바꾸지 않는다.

### 6-14. `C:/Users/user/Desktop/LostArk/Client/Public/Level_AssetTest.h`

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
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	HRESULT Ready_Valtan();
	HRESULT Ready_Blade();

public:
	static unique_ptr<CLevel_AssetTest> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
```

### 6-15. `C:/Users/user/Desktop/LostArk/Client/Private/Level_AssetTest.cpp`

변경 종류: include, `Initialize` 교체, 함수 추가

```cpp
#include "Blade.h"
#include "RuntimeAssetRoot.h"

#include <filesystem>
```

```cpp
HRESULT CLevel_AssetTest::Initialize()
{
	if (FAILED(__super::Initialize()) ||
		FAILED(Ready_Lights()) ||
		FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))) ||
		FAILED(Ready_Valtan()) ||
		FAILED(Ready_Blade()))
		return E_FAIL;
	return S_OK;
}
```

```cpp
HRESULT CLevel_AssetTest::Ready_Blade()
{
	const std::filesystem::path bladePath =
		CRuntimeAssetRoot::Resolve(L"Character/Blade/Blade.wmodel");
	const std::filesystem::path navigationPath =
		L"../Bin/DataFiles/Navigation/ValtanNavigation.dat";
	if (!std::filesystem::exists(bladePath) ||
		!std::filesystem::exists(navigationPath))
		return S_OK;

	CBlade::BLADE_DESC desc{};
	desc.startCellIndex = 0;
	return CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Blade"),
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Layer_Blade"), &desc);
}
```

## 7. Blade `.wmodel` 생성 계약

발탄과 같은 통합 Converter 경로를 사용한다. FBX 하나에 Mesh/Material/Skeleton/Animation이 모두 있으면:

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe `
  "C:\Asset\LostArk\Blade\Blade.fbx" `
  -o ".\Client\Bin\Resources\LostArk\Character\Blade\Blade.wmodel" `
  --texture-root "C:\Asset\LostArk\Blade\Textures"

.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe info `
  ".\Client\Bin\Resources\LostArk\Character\Blade\Blade.wmodel"
```

분리 추출물이라면 Converter README의 `pack` 명령으로 mesh/material/skeleton/animation을 하나의 `Blade.wmodel`로 묶는다. 완료 조건은 다음과 같다.

- mesh 수 1개 이상
- skeleton/bone 수 1개 이상
- animation 수 1개 이상
- diffuse 경로가 공유 팩 내부에서 해석됨
- Idle/Run 이름이 다르더라도 fallback index 0/1로 재생 가능

`Client/Bin/Resources/LostArk`는 Git에 올리지 않고 팀 공유 Drive 팩으로 배포한다.

## 8. 프로젝트 등록

### 8.1 Engine.vcxproj

```xml
<ClInclude Include="..\Public\NavigationData.h" />
```

### 8.2 Engine.vcxproj.filters

기존 Navigation과 같은 필터를 사용한다.

```xml
<ClInclude Include="..\Public\NavigationData.h">
  <Filter>02.Utility\04.Component\Navigation</Filter>
</ClInclude>
```

### 8.3 Client.vcxproj

```xml
<ClInclude Include="..\Public\NavigationCellEditor.h" />
<ClInclude Include="..\Public\Blade.h" />
<ClInclude Include="..\Public\Body_Blade.h" />
<ClCompile Include="..\Private\NavigationCellEditor.cpp" />
<ClCompile Include="..\Private\Blade.cpp" />
<ClCompile Include="..\Private\Body_Blade.cpp" />
<None Include="..\Bin\DataFiles\Navigation\ValtanNavigation.dat" />
```

### 8.4 Client.vcxproj.filters

새 필터:

```xml
<Filter Include="03. Tools\06. Navigation">
  <UniqueIdentifier>{D069E328-76CD-4E28-AE74-87D0D9CA3A14}</UniqueIdentifier>
</Filter>
<Filter Include="02.GameObjects\00. Character\Blade">
  <UniqueIdentifier>{E3ED8A96-3216-4217-A498-9AC6291228D8}</UniqueIdentifier>
</Filter>
<Filter Include="96.DataFiles\Navigation">
  <UniqueIdentifier>{B10EF397-CB7F-468B-81B7-BED851A61A35}</UniqueIdentifier>
</Filter>
```

파일 항목:

```xml
<ClInclude Include="..\Public\NavigationCellEditor.h">
  <Filter>03. Tools\06. Navigation</Filter>
</ClInclude>
<ClCompile Include="..\Private\NavigationCellEditor.cpp">
  <Filter>03. Tools\06. Navigation</Filter>
</ClCompile>
<ClInclude Include="..\Public\Blade.h">
  <Filter>02.GameObjects\00. Character\Blade</Filter>
</ClInclude>
<ClInclude Include="..\Public\Body_Blade.h">
  <Filter>02.GameObjects\00. Character\Blade</Filter>
</ClInclude>
<ClCompile Include="..\Private\Blade.cpp">
  <Filter>02.GameObjects\00. Character\Blade</Filter>
</ClCompile>
<ClCompile Include="..\Private\Body_Blade.cpp">
  <Filter>02.GameObjects\00. Character\Blade</Filter>
</ClCompile>
<None Include="..\Bin\DataFiles\Navigation\ValtanNavigation.dat">
  <Filter>96.DataFiles\Navigation</Filter>
</None>
```

## 9. 적용 순서

1. 다른 작업의 `MapTool.*`/프리뷰 변경을 먼저 merge하고 해당 파일의 최종 상태를 기준으로 이 계획의 작은 연동 블록만 적용한다.
2. `NavigationData.h`와 `CNavigation` 포맷/API를 적용한다.
3. `UpdateLib.bat Debug`로 Engine public header를 Client에 배포한다.
4. `CNavigationCellEditor`를 추가하고 MapTool의 Navigation 탭을 연결한다.
5. `CMainApp::Ready_Gara()`의 Navigation 덮어쓰기를 제거한다.
6. Blade FBX를 `Blade.wmodel` 하나로 변환해 공유 팩 경로에 둔다.
7. `CBlade/CBody_Blade`, Loader, AssetTest Level을 연결한다.
8. 첫 실행에서 셀을 Bake한 뒤 AssetTest를 나갔다가 F2로 다시 들어와 런타임 Prototype을 로드한다.

## 10. 빌드 명령

Developer PowerShell 또는 Visual Studio Developer Command Prompt:

```powershell
cd C:\Users\user\Desktop\LostArk
msbuild .\Engine\Default\Engine.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
.\UpdateLib.bat Debug
msbuild .\Client\Default\Client.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
```

Release도 같은 순서로 검증한다.

```powershell
msbuild .\Engine\Default\Engine.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64
.\UpdateLib.bat Release
msbuild .\Client\Default\Client.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64
```

## 11. 실행 검증

1. F2로 `ASSET_TEST`에 들어가고 F1로 MapTool을 연다.
2. `Navigation`을 선택한다.
3. 바닥/맵을 LMB 세 번 눌러 셀을 만든다. 두 번째 셀은 첫 셀 정점 근처를 눌러 스냅한다.
4. 초록색 debug cell과 셀 수 증가를 확인한다.
5. `Bake & Save` 후 `Client/Bin/DataFiles/Navigation/ValtanNavigation.dat`가 생성되는지 확인한다.
6. Logo로 나갔다가 F2로 다시 진입한다. 시작 시 파일 수정 시간이 바뀌지 않아야 한다.
7. Blade가 셀 0 중심에 생성되고 Idle이 재생되는지 확인한다.
8. `Actor Test`에서 셀 안을 RMB 클릭하면 Blade가 방향을 돌려 이동하고 Run으로 바뀌는지 확인한다.
9. 셀 밖을 RMB 클릭하면 움직이지 않아야 한다.
10. 연결된 셀을 가로질러 이동하고 외곽 edge를 넘지 못하는지 확인한다.
11. `ValtanNavigation.dat` 끝을 잘라 손상시킨 복사본으로 Load를 시도했을 때 현재 편집 초안이 유지되는지 확인한다.

## 12. 2차 확장 기준: 구조물 파괴와 Navigation

1차가 통과한 뒤에만 다음을 추가한다.

```text
Structure placement
  structureId
  destructibleState
  blockedPortalIds[]

Navigation runtime
  baked cells는 불변
  cell 간 portal/link만 enabled/disabled

파괴 이벤트
  Structure::BROKEN
    -> Collider 비활성
    -> 막고 있던 portal 활성 또는 비활성
    -> 캐릭터 다음 이동 요청부터 새 링크 상태 사용
```

발탄이 벽을 부순다고 해서 삼각형 전체를 다시 굽지 않는다. 대부분은 “벽이 존재할 때 닫힌 링크 / 벽이 부서지면 열린 링크”로 표현한다. 바닥 자체가 무너져 낙사 영역이 되는 패턴만 해당 셀을 disabled 처리한다. 이 구분이 끝난 뒤 A*를 붙이면 보스 패턴, Collider, Navigation의 책임이 섞이지 않는다.
