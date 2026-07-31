# LostArk Raid Authoring Camera · Navigation Trigger 구현 계획

```text
C1~C8: OFF
문제 해결 ①~⑤: OFF
자료구조·알고리즘: OFF
```

## 1. 변경 범위

### 1.1 현재 단계 완료 판정

현재 Navigation 페인팅 단계는 다음 사용자 실행 검증을 통과했으므로 이 계획에서 다시 수정하지 않는다.

- 초록 cell을 목적지로 피킹하면 캐릭터가 이동한다.
- 노란 cell을 목적지로 피킹하면 이동이 차단된다.
- MapTool의 `Save Navigation`으로 `.navpaint`와 `.navgrid`가 저장된다.
- 저장 후 다시 실행해도 walkable/nonwalkable 정보가 유지된다.

현재 `ValtanArena.navblockers`에는 runtime region이 0개다. 따라서 동적 조건 검증 단계에서는
`Destruction Area`에서 실제 cell이 들어 있는 region을 하나 만든 뒤 저장하고 AssetTest에 다시
진입해야 한다.

### 1.2 이번 구현의 목표

이번 단계는 다음 네 가지까지만 구현한다.

1. Navigation 페인팅 중에도 카메라를 돌릴 수 있게 입력 소유권을 분리한다.
2. 플레이어 기준 카메라 위치 offset과 회전각을 편집·저장하고, 버튼으로 Follow/Free Camera를 전환한다.
3. `.navblockers`의 condition을 Debug MapTool이 아니라 Level runtime이 소유하게 옮긴다.
4. `Boss Pattern`, `Sequencer`는 다음 단계의 진입점이 되는 빈 탭만 만든다.

이번 단계에서 하지 않는 일은 다음과 같다.

- 실제 Valtan 패턴 튜닝 UI와 패턴 실행 로직
- Sequencer timeline, track, keyframe 자료구조
- NavBounds 배치, 계단/다층 지형 bake 일반화
- CUL_BOX 기반 bake 교체
- 파괴 에셋마다 별도의 `CMapDestroyableAsset` 클래스 추가

Boss Pattern과 Sequencer의 실제 구현은 기존
`2026-07-31_LOSTARK_VALTAN_BOSS_PATTERN_IMPLEMENTATION_PLAN.md` 및
후속 Sequencer 계획에서 진행한다. 다른 맵·계단용 NavBounds bake 일반화도 이번 단계의
카메라와 runtime condition 검증을 닫은 뒤 별도 계획으로 분리한다.

### 1.3 카메라 회전이 현재 막히는 정확한 이유

현재 호출 경로는 다음과 같다.

```text
CMapTool::ConsumesWorldMouse()
    Navigation mode이면 true
        ↓
CMainApp::Update()
    SetInputBlocked(..., true)
        ↓
CInput_Device::Get_DIMouseMove(X/Y)
    mouse blocked이면 0 반환
        ↓
CCamera_Free::Priority_Update()
    회전 입력이 항상 0
```

마우스 전체 차단을 해제하는 것만으로 끝내면 LMB 한 번에 MapTool paint와
`CLevel_AssetTest::Update_ClickMove()`가 함께 실행된다. 따라서 다음처럼 역할을 나눈다.

```text
ImGui 위/외부 툴 focus
    → 키보드와 마우스 전체 차단

게임 화면 + Navigation paint
    → LMB만 Client gameplay에서 차단
    → MapTool은 Win32 LMB로 stroke 처리
    → RMB와 mouse X/Y는 Camera_Free에 전달

Free Camera
    → RMB를 누른 동안에만 mouse X/Y로 회전
    → WASD로 이동
```

### 1.4 Navigation condition의 최종 소유권

cell마다 파괴 오브젝트 포인터, 원래 type, 현재 bool을 넣지 않는다.

```text
정적 이동 가능 여부
    ValtanArena.navgrid
    └─ 각 cell의 base walkable

동적 영역 정의
    ValtanArena.navblockers
    └─ region id + condition id + cell indices

현재 레이드 상태
    CNavigationConditionRuntime (Level당 1개)
    └─ condition id → bool

보스/트리거/파괴 연출
    Set_Condition("VALTAN_ARENA_DESTROYED", true)
        ↓
    해당 condition을 구독하는 blocker region 활성/비활성
        ↓
    CNavGrid의 runtime blocker count 반영
```

`activateWhenConditionTrue`는 두 경우를 모두 표현한다.

- `Block after destruction`: condition이 `true`일 때 막힌다.
- `Open after destruction`: condition이 `false`일 때 막혀 있고, 파괴 후 `true`가 되면 열린다.

`CMapTool`은 condition의 owner가 아니다. authoring 문서를 편집하고 테스트 명령만 runtime
owner에 전달한다. 나중에 Valtan 패턴이나 실제 trigger가 같은 public 함수 하나를 호출한다.
기존 `CDeployPropObject`는 intact/fractured/despawned 시각 상태를 계속 담당하고,
`CNavigationConditionRuntime`은 이동 가능 상태만 담당한다. 실제 파괴 trigger는 두 owner에게
동일한 파괴 사건을 전달하되, cell이 `CDeployPropObject` 포인터를 저장하지는 않는다.

### 1.5 최종 ImGui 형태

```text
+--------------------------------------------------------------------------------+
| LostArk Map Tool                                                              |
| [Map Assets] [Navigation] [Camera] [Boss Pattern] [Sequencer]                 |
+--------------------------------------------------------------------------------+

Camera 탭
+--------------------------------------------------------------------------------+
| Camera                                                                        |
| Mode: Follow Player                                      [Free Camera]         |
| Player Position Offset   X [  5.00 ] Y [ 19.03 ] Z [-28.25 ]                 |
| Rotation (Degrees)      Pitch [33.90] Yaw [ 0.00 ] Roll [ 0.00 ]             |
| [Save Camera]  Saved                                                         |
|                                                                                |
| Free Camera일 때: RMB + Drag = Look, WASD = Move                              |
+--------------------------------------------------------------------------------+

Boss Pattern 탭
+--------------------------------------------------------------------------------+
| Boss Pattern tuning — next stage                                              |
+--------------------------------------------------------------------------------+

Sequencer 탭
+--------------------------------------------------------------------------------+
| Sequencer — next stage                                                        |
+--------------------------------------------------------------------------------+
```

Camera의 XYZ는 월드 절대 좌표가 아니라 플레이어 위치에 더하는 offset이다. 저장 대상은
Follow Player preset뿐이며, 현재 Follow/Free 선택 상태는 저장하지 않는다. 재진입 시 항상
Follow Player로 시작한다.

## 2. 추가·수정·삭제 파일

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/CameraPresetDocument.h` | 카메라 preset 데이터와 load/save 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/CameraPresetDocument.cpp` | camera 파일 parse → validate → commit 및 atomic save |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/NavigationConditionRuntime.h` | Level 단위 Navigation condition runtime owner |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/NavigationConditionRuntime.cpp` | blocker 등록과 condition 적용/rollback |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Camera/ValtanArena.camera` | Valtan 기본 Follow Player camera preset |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Input_Device.h` | 마우스 전체 차단과 LMB 단독 차단 분리 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/GameInstance.h` | button별 입력 차단 facade 추가 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/GameInstance.cpp` | 입력 차단 facade 전달 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Layer.h` | index 기반 GameObject 조회 계약 추가 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Layer.cpp` | GameObject 조회 구현 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Object_Manager.h` | Level/Layer GameObject 조회 계약 추가 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Object_Manager.cpp` | Layer 조회 전달 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/GameInstance.h` | Object Manager 조회 facade 추가 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/GameInstance.cpp` | GameObject 조회 facade 전달 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Navigation.h` | NavGrid identity 조회 계약 추가 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Navigation.cpp` | NavGrid descriptor 복사 구현 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Camera_Free.h` | Follow Player/Free Camera 명시적 mode와 preset API |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Camera_Free.cpp` | Follow pose 및 RMB drag free camera 구현 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_AssetTest.h` | camera/runtime condition 인스턴스 수명 보관 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_AssetTest.cpp` | Character → Camera → Navigation runtime 생성 순서 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp` | Navigation runtime prototype 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h` | 새 mode, camera panel, runtime 위임 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp` | 단순 panel과 runtime condition 위임 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp` | ImGui 전체 차단과 world LMB 차단 분리 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | 신규 C++/camera/nav 데이터 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | 물리 역할에 맞는 filter 등록 |
| 삭제 | 없음 | 기존 runtime blocker와 Navigation paint 계약을 재사용 |

### 2.1 Engine/Client 배치와 의존성

| 파일/클래스 | 배치 | 소유 이유 | 직접 의존 | 의존 금지 | 수명/owner |
|---|---|---|---|---|---|
| `CInput_Device` button mask | Engine | 프로젝트 비종속 입력 게이트 | DirectInput enum/state | ImGui, LostArk mode | `CGameInstance`가 Engine 실행 동안 소유 |
| `Get_GameObject` facade | Engine | Level/Layer의 범용 조회 계약 | `CObject_Manager`, `CLayer` | Client class cast | Engine Object Manager 수명 |
| `CNavigation::Get_NavGridDesc` | Engine | runtime grid의 범용 identity 조회 | `CNavGrid` | `.navblockers`, condition id | Navigation component 수명 |
| `CCameraPresetDocument` | Client | LostArk 카메라 authoring/runtime 데이터 | filesystem, `float3_t` | ImGui | Level/MapTool이 각각 작업 복사본 소유 |
| `CNavigationConditionRuntime` | Client | LostArk Level과 raid condition 소유 | `CNavigation`, `.navblockers` | ImGui | `Layer_NavigationRuntime`의 GameObject |
| `CCamera_Free` | Client | LostArk camera mode와 조작 | Engine input/transform | MapTool include | `Layer_Camera`의 GameObject |
| `CMapTool` panel | Client Debug | 선택과 명령만 전달 | camera/runtime public API | 매 frame 파일 load | `CMainApp` Debug 도구 |

의존 방향은 `Client → Engine public contract`만 허용한다. Engine에는 ImGui, Valtan ID,
camera 파일명, `VALTAN_ARENA_DESTROYED` 같은 Client 개념을 넣지 않는다.

## 3. 파일별 최종 반영 코드

### 3.1 `Client/Public/CameraPresetDocument.h` — 신규 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"

#include <filesystem>
#include <string>

NS_BEGIN(Client)

struct CAMERA_PRESET final
{
	float3_t positionOffset = { 5.f, 19.03165f, -28.25f };
	float3_t rotationDegrees = { 33.9f, 0.f, 0.f };
};

class CCameraPresetDocument final
{
public:
	bool_t Load(
		const std::filesystem::path& path,
		std::string& outStatus);
	bool_t Save(
		const std::filesystem::path& path,
		std::string& outStatus);
	bool_t Set_Preset(const CAMERA_PRESET& preset);

	const CAMERA_PRESET& Get_Preset() const { return m_Preset; }
	bool_t Is_Ready() const { return m_isReady; }
	bool_t Is_Dirty() const { return m_isDirty; }

	static std::filesystem::path Get_DefaultPath();

private:
	static bool_t Is_Valid(const CAMERA_PRESET& preset);

private:
	CAMERA_PRESET m_Preset;
	bool_t m_isReady = true;
	bool_t m_isDirty = false;
};

NS_END
```

### 3.2 `Client/Private/CameraPresetDocument.cpp` — 신규 전체 코드

```cpp
#include "CameraPresetDocument.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iterator>

namespace
{
	constexpr const char* MAGIC = "LOSTARK_CAMERA_PRESET";
	constexpr uint32_t VERSION = 1;

	bool_t CommitTemporaryFile(
		const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code error;
		const bool_t destinationExists =
			std::filesystem::exists(destination, error);
		if (error)
			return false;

		if (destinationExists &&
			ReplaceFileW(
				destination.c_str(),
				temporary.c_str(),
				nullptr,
				REPLACEFILE_WRITE_THROUGH,
				nullptr,
				nullptr))
		{
			return true;
		}

		return MoveFileExW(
			temporary.c_str(),
			destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}

	bool_t IsSame(const float3_t& left, const float3_t& right)
	{
		return left.x == right.x &&
			left.y == right.y &&
			left.z == right.z;
	}
}

bool_t Client::CCameraPresetDocument::Load(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	std::error_code existsError;
	const bool_t exists = std::filesystem::exists(path, existsError);
	if (existsError)
	{
		outStatus = "Could not inspect camera preset";
		return false;
	}

	if (!exists)
	{
		m_Preset = CAMERA_PRESET{};
		m_isReady = true;
		m_isDirty = false;
		outStatus = "Using default camera preset";
		return true;
	}

	std::ifstream input(path, std::ios::binary);
	std::string magic;
	std::string positionTag;
	std::string rotationTag;
	uint32_t version = {};
	CAMERA_PRESET staged;
	if (!input ||
		!(input >>
			magic >>
			version >>
			positionTag >>
			staged.positionOffset.x >>
			staged.positionOffset.y >>
			staged.positionOffset.z >>
			rotationTag >>
			staged.rotationDegrees.x >>
			staged.rotationDegrees.y >>
			staged.rotationDegrees.z) ||
		magic != MAGIC ||
		version != VERSION ||
		positionTag != "POSITION_OFFSET" ||
		rotationTag != "ROTATION_DEGREES" ||
		!Is_Valid(staged))
	{
		outStatus = "Camera preset is invalid; previous preset preserved";
		return false;
	}

	input >> std::ws;
	if (input.peek() != std::char_traits<char>::eof())
	{
		outStatus = "Camera preset has trailing data; previous preset preserved";
		return false;
	}

	m_Preset = staged;
	m_isReady = true;
	m_isDirty = false;
	outStatus = "Loaded camera preset";
	return true;
}

bool_t Client::CCameraPresetDocument::Save(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	if (!m_isReady || !Is_Valid(m_Preset) || path.empty())
	{
		outStatus = "Camera preset is not ready";
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(
		path.parent_path(),
		directoryError);
	if (directoryError)
	{
		outStatus = "Could not create camera preset directory";
		return false;
	}

	std::filesystem::path temporary = path;
	temporary += L".tmp";
	std::ofstream output(
		temporary,
		std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create temporary camera preset";
		return false;
	}

	output <<
		MAGIC << ' ' << VERSION << '\n' <<
		"POSITION_OFFSET " <<
		std::setprecision(9) <<
		m_Preset.positionOffset.x << ' ' <<
		m_Preset.positionOffset.y << ' ' <<
		m_Preset.positionOffset.z << '\n' <<
		"ROTATION_DEGREES " <<
		m_Preset.rotationDegrees.x << ' ' <<
		m_Preset.rotationDegrees.y << ' ' <<
		m_Preset.rotationDegrees.z << '\n';

	output.flush();
	const bool_t wroteSuccessfully = output.good();
	output.close();
	if (!wroteSuccessfully ||
		!CommitTemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "Failed to commit camera preset atomically";
		return false;
	}

	m_isDirty = false;
	outStatus = "Saved";
	return true;
}

bool_t Client::CCameraPresetDocument::Set_Preset(
	const CAMERA_PRESET& preset)
{
	if (!Is_Valid(preset))
		return false;

	if (IsSame(m_Preset.positionOffset, preset.positionOffset) &&
		IsSame(m_Preset.rotationDegrees, preset.rotationDegrees))
	{
		return true;
	}

	m_Preset = preset;
	m_isReady = true;
	m_isDirty = true;
	return true;
}

std::filesystem::path
Client::CCameraPresetDocument::Get_DefaultPath()
{
	wchar_t modulePath[32768]{};
	const DWORD length = GetModuleFileNameW(
		nullptr,
		modulePath,
		static_cast<DWORD>(std::size(modulePath)));
	if (0 == length || length >= std::size(modulePath))
		return {};

	const std::filesystem::path moduleDirectory =
		std::filesystem::path(modulePath).parent_path();
	const std::filesystem::path adjacent =
		moduleDirectory /
		L"DataFiles" /
		L"Camera" /
		L"ValtanArena.camera";
	if (std::filesystem::exists(adjacent))
		return adjacent.lexically_normal();

	return (
		moduleDirectory.parent_path() /
		L"DataFiles" /
		L"Camera" /
		L"ValtanArena.camera").lexically_normal();
}

bool_t Client::CCameraPresetDocument::Is_Valid(
	const CAMERA_PRESET& preset)
{
	const auto isFinite3 = [](const float3_t& value)
		{
			return std::isfinite(value.x) &&
				std::isfinite(value.y) &&
				std::isfinite(value.z);
		};
	const auto isInRange = [](const float3_t& value, f32_t limit)
		{
			return std::fabs(value.x) <= limit &&
				std::fabs(value.y) <= limit &&
				std::fabs(value.z) <= limit;
		};

	return isFinite3(preset.positionOffset) &&
		isFinite3(preset.rotationDegrees) &&
		isInRange(preset.positionOffset, 10000.f) &&
		isInRange(preset.rotationDegrees, 360.f);
}
```

### 3.3 `Client/Bin/DataFiles/Camera/ValtanArena.camera` — 신규 전체 데이터

```text
LOSTARK_CAMERA_PRESET 1
POSITION_OFFSET 5 19.03165 -28.25
ROTATION_DEGREES 33.9 0 0
```

이 값은 현재 AssetTest의 player `(151.25, 22.96835, -121.75)`와 camera eye
`(156.25, 42, -150)`의 차이를 기본 offset으로 사용한다. Pitch `33.9`도 현재
`vAt` 방향과 최대한 같은 첫 화면을 유지하기 위한 초기값이다.

### 3.4 `Client/Public/NavigationConditionRuntime.h` — 신규 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "NavRuntimeBlockerDocument.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

NS_BEGIN(Engine)
class CNavigation;
NS_END

NS_BEGIN(Client)

class CNavigationConditionRuntime final : public CGameObject
{
public:
	struct NAVIGATION_CONDITION_RUNTIME_DESC :
		public CGameObject::GAMEOBJECT_DESC
	{
		std::string areaId;
		std::filesystem::path blockerFileName;
		shared_ptr<CNavigation> pNavigation;
	};

private:
	CNavigationConditionRuntime(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CNavigationConditionRuntime();
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	bool_t Set_Condition(
		const std::string& conditionId,
		bool_t value);
	bool_t Try_GetCondition(
		const std::string& conditionId,
		bool_t& outValue) const;
	const std::string& Get_Status() const { return m_Status; }

private:
	static std::filesystem::path Resolve_BlockerPath(
		const std::filesystem::path& fileName);

private:
	shared_ptr<CNavigation> m_pNavigation = { nullptr };
	CNavRuntimeBlockerDocument m_Document;
	std::unordered_map<std::string, bool_t> m_Conditions;
	std::unordered_set<std::string> m_RegisteredRegionIds;
	std::string m_Status;

public:
	static unique_ptr<CNavigationConditionRuntime> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### 3.5 `Client/Private/NavigationConditionRuntime.cpp` — 신규 전체 코드

```cpp
#include "NavigationConditionRuntime.h"

#include "Navigation.h"

#include <algorithm>
#include <iterator>
#include <utility>

CNavigationConditionRuntime::CNavigationConditionRuntime(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CNavigationConditionRuntime::~CNavigationConditionRuntime() = default;

HRESULT CNavigationConditionRuntime::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNavigationConditionRuntime::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_INVALIDARG;

	const NAVIGATION_CONDITION_RUNTIME_DESC desc =
		*static_cast<NAVIGATION_CONDITION_RUNTIME_DESC*>(pArg);
	if (desc.areaId.empty() ||
		desc.blockerFileName.empty() ||
		nullptr == desc.pNavigation ||
		CNavigation::MODE::NAVGRID_ASTAR !=
		desc.pNavigation->Get_Mode())
	{
		return E_INVALIDARG;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	CNavGrid::NAVGRID_DESC gridDesc{};
	if (!desc.pNavigation->Get_NavGridDesc(gridDesc))
		return E_FAIL;

	NAVGRID_AUTHORING_DESC expectedDesc;
	expectedDesc.areaId = desc.areaId;
	expectedDesc.width = gridDesc.iWidth;
	expectedDesc.height = gridDesc.iHeight;
	expectedDesc.cellSize = gridDesc.fCellSize;
	expectedDesc.originX = gridDesc.fOriginX;
	expectedDesc.originZ = gridDesc.fOriginZ;

	const std::filesystem::path blockerPath =
		Resolve_BlockerPath(desc.blockerFileName);
	CNavRuntimeBlockerDocument stagedDocument;
	std::string stagedStatus;
	if (blockerPath.empty() ||
		!stagedDocument.Load(
			blockerPath,
			expectedDesc,
			stagedStatus))
	{
		m_Status = stagedStatus;
		return E_FAIL;
	}

	std::unordered_map<std::string, bool_t> stagedConditions;
	std::unordered_set<std::string> stagedRegionIds;
	desc.pNavigation->Clear_RuntimeBlockers();
	for (size_t index = 0;
		index < stagedDocument.Get_RegionCount();
		++index)
	{
		const NAV_RUNTIME_BLOCKER_REGION* region =
			stagedDocument.Get_Region(index);
		if (nullptr == region)
		{
			desc.pNavigation->Clear_RuntimeBlockers();
			return E_FAIL;
		}

		stagedConditions.emplace(region->conditionId, false);
		const vector<uint32_t> cells =
			stagedDocument.Get_CellIndices(index);
		if (cells.empty())
			continue;

		const bool_t initiallyActive =
			false == region->activateWhenConditionTrue;
		if (!desc.pNavigation->Register_RuntimeBlocker(
			region->id,
			cells,
			initiallyActive))
		{
			desc.pNavigation->Clear_RuntimeBlockers();
			m_Status =
				"Failed to register runtime blocker: " +
				region->id;
			return E_FAIL;
		}
		stagedRegionIds.emplace(region->id);
	}

	m_pNavigation = desc.pNavigation;
	m_Document = std::move(stagedDocument);
	m_Conditions = std::move(stagedConditions);
	m_RegisteredRegionIds = std::move(stagedRegionIds);
	m_Status =
		"Navigation runtime ready: " +
		std::to_string(m_Document.Get_RegionCount()) +
		" regions";
	return S_OK;
}

bool_t CNavigationConditionRuntime::Set_Condition(
	const std::string& conditionId,
	bool_t value)
{
	if (nullptr == m_pNavigation || conditionId.empty())
		return false;

	const auto condition = m_Conditions.find(conditionId);
	if (condition == m_Conditions.end())
	{
		m_Status = "Unknown navigation condition: " + conditionId;
		return false;
	}

	const bool_t previousValue = condition->second;
	vector<const NAV_RUNTIME_BLOCKER_REGION*> appliedRegions;
	for (size_t index = 0;
		index < m_Document.Get_RegionCount();
		++index)
	{
		const NAV_RUNTIME_BLOCKER_REGION* region =
			m_Document.Get_Region(index);
		if (nullptr == region ||
			region->conditionId != conditionId ||
			m_RegisteredRegionIds.end() ==
			m_RegisteredRegionIds.find(region->id))
		{
			continue;
		}

		const bool_t active =
			value == region->activateWhenConditionTrue;
		if (!m_pNavigation->Set_RuntimeBlockerActive(
			region->id,
			active))
		{
			for (auto applied = appliedRegions.rbegin();
				applied != appliedRegions.rend();
				++applied)
			{
				const bool_t previousActive =
					previousValue ==
					(*applied)->activateWhenConditionTrue;
				m_pNavigation->Set_RuntimeBlockerActive(
					(*applied)->id,
					previousActive);
			}
			m_Status =
				"Failed to apply navigation condition: " +
				conditionId;
			return false;
		}
		appliedRegions.push_back(region);
	}

	condition->second = value;
	m_Status =
		"Navigation condition " +
		conditionId +
		(value ? " = true" : " = false");
	return true;
}

bool_t CNavigationConditionRuntime::Try_GetCondition(
	const std::string& conditionId,
	bool_t& outValue) const
{
	const auto condition = m_Conditions.find(conditionId);
	if (condition == m_Conditions.end())
		return false;

	outValue = condition->second;
	return true;
}

std::filesystem::path
CNavigationConditionRuntime::Resolve_BlockerPath(
	const std::filesystem::path& fileName)
{
	if (fileName.empty())
		return {};
	if (fileName.is_absolute())
		return fileName.lexically_normal();

	wchar_t modulePath[32768]{};
	const DWORD length = GetModuleFileNameW(
		nullptr,
		modulePath,
		static_cast<DWORD>(std::size(modulePath)));
	if (0 == length || length >= std::size(modulePath))
		return {};

	const std::filesystem::path moduleDirectory =
		std::filesystem::path(modulePath).parent_path();
	const std::filesystem::path adjacent =
		moduleDirectory /
		L"DataFiles" /
		L"Navigation" /
		fileName;
	if (std::filesystem::exists(adjacent))
		return adjacent.lexically_normal();

	return (
		moduleDirectory.parent_path() /
		L"DataFiles" /
		L"Navigation" /
		fileName).lexically_normal();
}

unique_ptr<CNavigationConditionRuntime>
CNavigationConditionRuntime::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance =
		unique_ptr<CNavigationConditionRuntime>(
			new CNavigationConditionRuntime(
				pDevice,
				pContext));
	if (FAILED(instance->Initialize_Prototype()))
		return nullptr;
	return instance;
}

shared_ptr<CPrototype>
CNavigationConditionRuntime::Clone(void* pArg)
{
	auto instance =
		shared_ptr<CNavigationConditionRuntime>(
			new CNavigationConditionRuntime(*this));
	if (FAILED(instance->Initialize(pArg)))
		return nullptr;
	return instance;
}
```

### 3.6 `Engine/Public/Input_Device.h` — mouse button별 차단으로 교체

기존 `Get_DIMouseState`, `SetInputBlocked` 주변을 다음으로 교체하고 button mask 멤버를 추가한다.

```cpp
	int8_t Get_DIMouseState(DIM eMouse)
	{
		const uint32_t index = ETOUI(eMouse);
		if (m_bMouseBlocked ||
			index >= ETOUI(DIM::END) ||
			m_bMouseButtonBlocked[index])
		{
			return 0;
		}

		return m_tMouseState.rgbButtons[index];
	}

	int32_t Get_DIMouseMove(DIMM eMouseState)
	{
		if (m_bMouseBlocked)
			return 0;

		switch (eMouseState)
		{
		case DIMM::X:
			return m_tMouseState.lX;
		case DIMM::Y:
			return m_tMouseState.lY;
		case DIMM::WHEEL:
			return m_tMouseState.lZ;
		default:
			return 0;
		}
	}

	void SetInputBlocked(
		bool_t bKeyboardBlocked,
		bool_t bMouseBlocked)
	{
		m_bKeyboardBlocked = bKeyboardBlocked;
		m_bMouseBlocked = bMouseBlocked;
	}

	void SetMouseButtonBlocked(
		DIM eMouse,
		bool_t blocked)
	{
		const uint32_t index = ETOUI(eMouse);
		if (index < ETOUI(DIM::END))
			m_bMouseButtonBlocked[index] = blocked;
	}
```

private 멤버의 최종 추가 코드는 다음과 같다.

```cpp
	bool_t m_bKeyboardBlocked = false;
	bool_t m_bMouseBlocked = false;
	bool_t m_bMouseButtonBlocked[ETOUI(DIM::END)] = {};
```

### 3.7 `Engine/Public/GameInstance.h`, `Engine/Private/GameInstance.cpp` — 입력 facade

`GameInstance.h`의 Input_Device public 계약에 추가한다.

```cpp
	void SetMouseButtonBlocked(DIM eMouse, bool_t blocked);
```

`GameInstance.cpp`에 추가한다.

```cpp
void CGameInstance::SetMouseButtonBlocked(
	DIM eMouse,
	bool_t blocked)
{
	if (nullptr != m_pInput_Device)
		m_pInput_Device->SetMouseButtonBlocked(eMouse, blocked);
}
```

### 3.8 `Engine/Public/Layer.h`, `Engine/Private/Layer.cpp` — GameObject 조회

`Layer.h` public 영역에 추가한다.

```cpp
	shared_ptr<CGameObject> Get_GameObject(uint32_t iIndex);
```

`Layer.cpp`에 추가한다.

```cpp
shared_ptr<CGameObject> CLayer::Get_GameObject(uint32_t iIndex)
{
	if (iIndex >= m_GameObjects.size())
		return nullptr;

	auto iter = m_GameObjects.begin();
	for (uint32_t index = 0; index < iIndex; ++index)
		++iter;

	return iter != m_GameObjects.end() ? *iter : nullptr;
}
```

### 3.9 `Engine/Public/Object_Manager.h`, `Engine/Private/Object_Manager.cpp` — 조회 전달

`Object_Manager.h` public 영역에 추가한다.

```cpp
	shared_ptr<CGameObject> Get_GameObject(
		uint32_t iLevelIndex,
		const wstring_t& strLayerTag,
		uint32_t iIndex);
```

`Object_Manager.cpp`에 추가한다.

```cpp
shared_ptr<CGameObject> CObject_Manager::Get_GameObject(
	uint32_t iLevelIndex,
	const wstring_t& strLayerTag,
	uint32_t iIndex)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	CLayer* pLayer = Find_Layer(iLevelIndex, strLayerTag);
	return nullptr != pLayer ?
		pLayer->Get_GameObject(iIndex) :
		nullptr;
}
```

### 3.10 `Engine/Public/GameInstance.h`, `Engine/Private/GameInstance.cpp` — 조회 facade

`GameInstance.h`의 Object_Manager public 계약에 추가한다.

```cpp
	shared_ptr<class CGameObject> Get_GameObject(
		uint32_t iLevelIndex,
		const wstring_t& strLayerTag,
		uint32_t iIndex);
```

`GameInstance.cpp`에 추가한다.

```cpp
shared_ptr<CGameObject> CGameInstance::Get_GameObject(
	uint32_t iLevelIndex,
	const wstring_t& strLayerTag,
	uint32_t iIndex)
{
	return m_pObject_Manager->Get_GameObject(
		iLevelIndex,
		strLayerTag,
		iIndex);
}
```

이 API는 Camera나 Navigation 전용 API가 아니다. 기존 Level/Layer 구조에서 object를 조회하는
범용 계약만 Engine에 추가하고, `CCamera_Free` 및 `CNavigationConditionRuntime` cast는 Client에서
수행한다.

### 3.11 `Engine/Public/Navigation.h`, `Engine/Private/Navigation.cpp` — grid identity 조회

`Navigation.h` public 영역에 추가한다.

```cpp
	bool_t Get_NavGridDesc(CNavGrid::NAVGRID_DESC& outDesc) const;
```

`Navigation.cpp`에 추가한다.

```cpp
bool_t CNavigation::Get_NavGridDesc(
	CNavGrid::NAVGRID_DESC& outDesc) const
{
	if (MODE::NAVGRID_ASTAR != m_eMode ||
		nullptr == m_pNavGrid)
	{
		return false;
	}

	outDesc = m_pNavGrid->Get_Desc();
	return true;
}
```

### 3.12 `Client/Public/Camera_Free.h` — 최종 교체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Camera.h"

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Client)

class CCamera_Free final : public CCamera
{
public:
	enum class MODE : uint8_t
	{
		FOLLOW_PLAYER,
		FREE,
	};

	typedef struct tagCameraFreeDesc :
		public CCamera::CAMERA_DESC
	{
		f32_t fMouseSensor = {};
		shared_ptr<CTransform> pFollowTarget = { nullptr };
		float3_t vFollowPositionOffset =
			{ 5.f, 19.03165f, -28.25f };
		float3_t vFollowRotationDegrees =
			{ 33.9f, 0.f, 0.f };
		MODE eInitialMode = MODE::FOLLOW_PLAYER;
	} CAMERA_FREE_DESC;

private:
	CCamera_Free(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CCamera_Free();
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	void Set_Mode(MODE mode);
	MODE Get_Mode() const { return m_Mode; }
	void Set_FollowPreset(
		const float3_t& positionOffset,
		const float3_t& rotationDegrees);
	const float3_t& Get_FollowPositionOffset() const {
		return m_vFollowPositionOffset;
	}
	const float3_t& Get_FollowRotationDegrees() const {
		return m_vFollowRotationDegrees;
	}

private:
	bool_t Apply_FollowPose();

private:
	f32_t m_fMouseSensor = {};
	MODE m_Mode = MODE::FOLLOW_PLAYER;
	weak_ptr<CTransform> m_pFollowTarget;
	float3_t m_vFollowPositionOffset = {};
	float3_t m_vFollowRotationDegrees = {};

public:
	static unique_ptr<CCamera_Free> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### 3.13 `Client/Private/Camera_Free.cpp` — Initialize/Priority/신규 함수 교체

생성·소멸·`Create`·`Clone`은 기존 코드를 유지하고 다음 함수 블록을 교체한다.

```cpp
HRESULT CCamera_Free::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_INVALIDARG;

	const CAMERA_FREE_DESC desc =
		*static_cast<CAMERA_FREE_DESC*>(pArg);
	m_fMouseSensor = desc.fMouseSensor;
	m_pFollowTarget = desc.pFollowTarget;
	m_vFollowPositionOffset =
		desc.vFollowPositionOffset;
	m_vFollowRotationDegrees =
		desc.vFollowRotationDegrees;
	m_Mode = desc.eInitialMode;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (MODE::FOLLOW_PLAYER == m_Mode &&
		!Apply_FollowPose())
	{
		m_Mode = MODE::FREE;
	}
	__super::Update_PipeLine();
	return S_OK;
}

void CCamera_Free::Priority_Update(f32_t fTimeDelta)
{
	if (MODE::FOLLOW_PLAYER == m_Mode)
	{
		if (!Apply_FollowPose())
			m_Mode = MODE::FREE;
		__super::Update_PipeLine();
		return;
	}

	if (CGameInstance::Get().Get_DIKeyState(DIK_W) & 0x80)
		m_pTransformCom->Go_Straight(fTimeDelta);
	if (CGameInstance::Get().Get_DIKeyState(DIK_S) & 0x80)
		m_pTransformCom->Go_Backward(fTimeDelta);
	if (CGameInstance::Get().Get_DIKeyState(DIK_A) & 0x80)
		m_pTransformCom->Go_Left(fTimeDelta);
	if (CGameInstance::Get().Get_DIKeyState(DIK_D) & 0x80)
		m_pTransformCom->Go_Right(fTimeDelta);

	const bool_t rotating =
		0 != (CGameInstance::Get().Get_DIMouseState(DIM::RB) & 0x80);
	if (rotating)
	{
		const int32_t mouseX =
			CGameInstance::Get().Get_DIMouseMove(DIMM::X);
		const int32_t mouseY =
			CGameInstance::Get().Get_DIMouseMove(DIMM::Y);
		if (0 != mouseX)
		{
			m_pTransformCom->Turn(
				XMVectorSet(0.f, 1.f, 0.f, 0.f),
				m_fMouseSensor * mouseX * fTimeDelta);
		}
		if (0 != mouseY)
		{
			m_pTransformCom->Turn(
				m_pTransformCom->Get_State(STATE::RIGHT),
				m_fMouseSensor * mouseY * fTimeDelta);
		}
	}

	__super::Update_PipeLine();
}

void CCamera_Free::Set_Mode(MODE mode)
{
	m_Mode = mode;
	if (MODE::FOLLOW_PLAYER == m_Mode)
		Apply_FollowPose();
}

void CCamera_Free::Set_FollowPreset(
	const float3_t& positionOffset,
	const float3_t& rotationDegrees)
{
	m_vFollowPositionOffset = positionOffset;
	m_vFollowRotationDegrees = rotationDegrees;
	if (MODE::FOLLOW_PLAYER == m_Mode)
		Apply_FollowPose();
}

bool_t CCamera_Free::Apply_FollowPose()
{
	const shared_ptr<CTransform> target =
		m_pFollowTarget.lock();
	if (nullptr == target)
		return false;

	const vector_t targetPosition =
		target->Get_State(STATE::POSITION);
	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSetW(
			targetPosition +
			XMLoadFloat3(&m_vFollowPositionOffset),
			1.f));
	m_pTransformCom->Rotation(
		m_vFollowRotationDegrees.x,
		m_vFollowRotationDegrees.y,
		m_vFollowRotationDegrees.z);
	return true;
}
```

기존 F6 follow toggle과 Tab movement lock 멤버·분기는 삭제한다. mode 전환은 화면에 보이는
Camera panel 버튼 하나만 정본으로 사용한다.

### 3.14 `Client/Public/Level_AssetTest.h` — 선언과 owner 추가

forward declaration에 추가한다.

```cpp
class CNavigationConditionRuntime;
```

private 함수에 추가한다.

```cpp
	HRESULT Ready_NavigationRuntime();
```

private 멤버에 추가한다.

```cpp
	shared_ptr<CNavigationConditionRuntime>
		m_pNavigationRuntime = { nullptr };
```

### 3.15 `Client/Private/Level_AssetTest.cpp` — 생성 순서와 Camera/runtime 생성

include에 추가한다.

```cpp
#include "CameraPresetDocument.h"
#include "NavigationConditionRuntime.h"
#include "Navigation.h"
```

`Initialize()`의 생성 순서를 다음으로 교체한다. Camera와 Navigation runtime이 player의
Transform/Navigation을 받으므로 Character가 먼저 생성되어야 한다.

```cpp
HRESULT CLevel_AssetTest::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Character()))
		return E_FAIL;
	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;
	if (FAILED(Ready_NavigationRuntime()))
		return E_FAIL;
	if (FAILED(Ready_Valtan()))
		return E_FAIL;
	return S_OK;
}
```

`Ready_Layer_Camera()`를 다음으로 교체한다.

```cpp
HRESULT CLevel_AssetTest::Ready_Layer_Camera(
	const wstring_t& strLayerTag)
{
	if (nullptr == m_pCharacter)
		return E_FAIL;

	CCameraPresetDocument cameraDocument;
	std::string cameraStatus;
	cameraDocument.Load(
		CCameraPresetDocument::Get_DefaultPath(),
		cameraStatus);
	const CAMERA_PRESET& preset =
		cameraDocument.Get_Preset();

	float3_t playerPosition{};
	XMStoreFloat3(
		&playerPosition,
		m_pCharacter->Get_Transform()->Get_State(
			STATE::POSITION));

	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(
		playerPosition.x + preset.positionOffset.x,
		playerPosition.y + preset.positionOffset.y,
		playerPosition.z + preset.positionOffset.z);
	cameraDesc.vAt = playerPosition;
	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = 1000.f;
	cameraDesc.fSpeedPerSec = 20.f;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.pFollowTarget =
		m_pCharacter->Get_Transform();
	cameraDesc.vFollowPositionOffset =
		preset.positionOffset;
	cameraDesc.vFollowRotationDegrees =
		preset.rotationDegrees;
	cameraDesc.eInitialMode =
		CCamera_Free::MODE::FOLLOW_PLAYER;

	return CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::ASSET_TEST),
		strLayerTag,
		&cameraDesc);
}
```

다음 함수를 추가한다.

```cpp
HRESULT CLevel_AssetTest::Ready_NavigationRuntime()
{
	if (nullptr == m_pCharacter)
		return E_FAIL;

	const shared_ptr<CNavigation> navigation =
		dynamic_pointer_cast<CNavigation>(
			m_pCharacter->Get_Component(
				TEXT("Com_Navigation")));
	if (nullptr == navigation)
		return E_FAIL;

	CNavigationConditionRuntime::
		NAVIGATION_CONDITION_RUNTIME_DESC desc{};
	desc.areaId = "ValtanArena";
	desc.blockerFileName =
		L"ValtanArena.navblockers";
	desc.pNavigation = navigation;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_NavigationConditionRuntime"),
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Layer_NavigationRuntime"),
		&desc,
		&gameObject)))
	{
		return E_FAIL;
	}

	m_pNavigationRuntime =
		dynamic_pointer_cast<CNavigationConditionRuntime>(
			gameObject);
	return nullptr != m_pNavigationRuntime ?
		S_OK :
		E_FAIL;
}
```

### 3.16 `Client/Private/Loader.cpp` — runtime prototype

include에 추가한다.

```cpp
#include "NavigationConditionRuntime.h"
```

`Ready_For_Level_AssetTest()`의 Navigation/Camera prototype 주변에 추가한다.

```cpp
    if (FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_NavigationConditionRuntime"),
        CNavigationConditionRuntime::Create(
            m_pDevice,
            m_pContext))))
        return E_FAIL;
```

### 3.17 `Client/Public/MapTool.h` — mode와 위임 계약

include에 추가한다.

```cpp
#include "CameraPresetDocument.h"
```

forward declaration에 추가한다.

```cpp
class CCamera_Free;
class CNavigationConditionRuntime;
```

`TOOL_MODE`을 교체한다.

```cpp
	enum class TOOL_MODE
	{
		MAP_ASSETS,
		NAVIGATION,
		CAMERA,
		BOSS_PATTERN,
		SEQUENCER,
	};
```

public 함수 이름을 교체한다.

```cpp
	bool_t ConsumesWorldLeftButton() const;
```

private 선언에서 `Register_RuntimeBlockers()`를 삭제하고 다음을 추가한다.

```cpp
	shared_ptr<CCamera_Free> Find_AssetTestCamera() const;
	shared_ptr<CNavigationConditionRuntime>
		Find_NavigationRuntime() const;
	void Render_CameraPanel();
	void Render_PlaceholderPanel(const char* message);
```

`m_NavigationConditions`를 삭제하고 다음 camera 상태를 추가한다.

```cpp
	CCameraPresetDocument m_CameraPresetDocument;
	std::filesystem::path m_CameraPresetPath;
	std::string m_CameraStatus =
		"Open ASSET_TEST with F2";
```

### 3.18 `Client/Private/MapTool.cpp` — include와 입력 소유권

include에 추가한다.

```cpp
#include "Camera_Free.h"
#include "NavigationConditionRuntime.h"
```

`ConsumesWorldMouse()`를 다음으로 교체한다.

```cpp
bool_t Client::CMapTool::ConsumesWorldLeftButton() const
{
	if (!m_bOpen ||
		ETOUI(LEVEL::ASSET_TEST) !=
		CGameInstance::Get().Get_CurrentLevelID())
	{
		return false;
	}

	return TOOL_MODE::NAVIGATION == m_eToolMode ||
		PLACEMENT_STATE::ARMED == m_ePlacementState;
}
```

`Handle_LevelTransition()`의 AssetTest 진입 시 Navigation load 다음에 camera working copy를
load한다.

```cpp
	Load_NavigationDocument();
	m_CameraPresetPath =
		CCameraPresetDocument::Get_DefaultPath();
	m_CameraPresetDocument.Load(
		m_CameraPresetPath,
		m_CameraStatus);
```

AssetTest 이탈 분기에는 다음 상태 초기화를 추가한다.

```cpp
		m_CameraStatus = "Open ASSET_TEST with F2";
```

### 3.19 `Client/Private/MapTool.cpp` — runtime 등록 제거와 condition 위임

`Load_RuntimeBlockers()`의 마지막 줄을 다음처럼 바꾼다.

```cpp
	m_NavigationStatus =
		"Loaded " +
		std::to_string(
			m_RuntimeBlockerDocument.Get_RegionCount()) +
		" destruction areas";
	return true;
```

기존 `Register_RuntimeBlockers()` 함수 전체를 삭제한다.

`Set_NavigationCondition()`을 다음으로 교체한다.

```cpp
bool_t Client::CMapTool::Set_NavigationCondition(
	const std::string& conditionId,
	bool_t value)
{
	const shared_ptr<CNavigationConditionRuntime> runtime =
		Find_NavigationRuntime();
	if (nullptr == runtime)
		return false;

	const bool_t result =
		runtime->Set_Condition(conditionId, value);
	m_NavigationStatus = runtime->Get_Status();
	return result;
}
```

조회 함수 두 개를 추가한다.

```cpp
shared_ptr<CCamera_Free>
Client::CMapTool::Find_AssetTestCamera() const
{
	return dynamic_pointer_cast<CCamera_Free>(
		CGameInstance::Get().Get_GameObject(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Layer_Camera"),
			0));
}

shared_ptr<CNavigationConditionRuntime>
Client::CMapTool::Find_NavigationRuntime() const
{
	return dynamic_pointer_cast<CNavigationConditionRuntime>(
		CGameInstance::Get().Get_GameObject(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Layer_NavigationRuntime"),
			0));
}
```

`Render_DestructionAreaControls()`의 Test 상태 조회 블록은 `m_NavigationConditions[]` 대신
runtime owner를 읽는다.

```cpp
	selectedRegion = m_RuntimeBlockerDocument.Get_Region(
		m_iSelectedRuntimeRegion);
	if (nullptr != selectedRegion &&
		ImGui::CollapsingHeader("Test"))
	{
		bool_t conditionValue = false;
		const shared_ptr<CNavigationConditionRuntime> runtime =
			Find_NavigationRuntime();
		if (nullptr != runtime)
		{
			runtime->Try_GetCondition(
				selectedRegion->conditionId,
				conditionValue);
		}

		if (ImGui::Checkbox(
			"Destroyed",
			&conditionValue))
		{
			if (!Set_NavigationCondition(
				selectedRegion->conditionId,
				conditionValue))
			{
				m_NavigationStatus =
					"Re-enter ASSET_TEST before testing this region.";
			}
		}
	}
```

### 3.20 `Client/Private/MapTool.cpp` — mode bar와 panel

`Render_ModeBar()`를 다음으로 교체한다.

```cpp
void Client::CMapTool::Render_ModeBar()
{
	const auto selectMode =
		[this](const char* label, TOOL_MODE mode)
		{
			if (ImGui::RadioButton(
				label,
				mode == m_eToolMode))
			{
				m_eToolMode = mode;
				if (TOOL_MODE::MAP_ASSETS != mode)
					m_ePlacementState =
						PLACEMENT_STATE::IDLE;
			}
		};

	selectMode("Map Assets", TOOL_MODE::MAP_ASSETS);
	ImGui::SameLine();
	selectMode("Navigation", TOOL_MODE::NAVIGATION);
	ImGui::SameLine();
	selectMode("Camera", TOOL_MODE::CAMERA);
	ImGui::SameLine();
	selectMode("Boss Pattern", TOOL_MODE::BOSS_PATTERN);
	ImGui::SameLine();
	selectMode("Sequencer", TOOL_MODE::SEQUENCER);
}
```

`Render()`에서 mode bar 아래 분기를 다음으로 교체한다. 기존 Map Assets 본문은 이 분기 뒤에
그대로 둔다.

```cpp
	if (TOOL_MODE::NAVIGATION == m_eToolMode)
	{
		ImGui::BeginDisabled(!isAssetTest);
		Render_NavigationPanel();
		ImGui::EndDisabled();
		ImGui::End();
		return;
	}
	if (TOOL_MODE::CAMERA == m_eToolMode)
	{
		ImGui::BeginDisabled(!isAssetTest);
		Render_CameraPanel();
		ImGui::EndDisabled();
		ImGui::End();
		return;
	}
	if (TOOL_MODE::BOSS_PATTERN == m_eToolMode)
	{
		Render_PlaceholderPanel(
			"Boss Pattern tuning - next stage");
		ImGui::End();
		return;
	}
	if (TOOL_MODE::SEQUENCER == m_eToolMode)
	{
		Render_PlaceholderPanel(
			"Sequencer - next stage");
		ImGui::End();
		return;
	}
```

다음 panel 함수를 추가한다.

```cpp
void Client::CMapTool::Render_CameraPanel()
{
	ImGui::TextUnformatted("Camera");
	const shared_ptr<CCamera_Free> camera =
		Find_AssetTestCamera();
	if (nullptr == camera ||
		!m_CameraPresetDocument.Is_Ready())
	{
		ImGui::TextUnformatted("Camera is unavailable.");
		return;
	}

	const bool_t isFollow =
		CCamera_Free::MODE::FOLLOW_PLAYER ==
		camera->Get_Mode();
	ImGui::Text(
		"Mode: %s",
		isFollow ? "Follow Player" : "Free Camera");
	ImGui::SameLine();
	if (ImGui::Button(
		isFollow ?
		"Free Camera" :
		"Return to Player Camera"))
	{
		camera->Set_Mode(
			isFollow ?
			CCamera_Free::MODE::FREE :
			CCamera_Free::MODE::FOLLOW_PLAYER);
	}

	CAMERA_PRESET preset =
		m_CameraPresetDocument.Get_Preset();
	bool_t changed = false;
	changed =
		ImGui::DragFloat3(
			"Player Position Offset",
			&preset.positionOffset.x,
			0.1f) ||
		changed;
	changed =
		ImGui::DragFloat3(
			"Rotation (Degrees)",
			&preset.rotationDegrees.x,
			0.1f,
			-360.f,
			360.f) ||
		changed;
	if (changed &&
		m_CameraPresetDocument.Set_Preset(preset))
	{
		camera->Set_FollowPreset(
			preset.positionOffset,
			preset.rotationDegrees);
		m_CameraStatus = "Unsaved";
	}

	if (ImGui::Button("Save Camera"))
	{
		m_CameraPresetDocument.Save(
			m_CameraPresetPath,
			m_CameraStatus);
	}
	ImGui::SameLine();
	ImGui::TextUnformatted(
		m_CameraPresetDocument.Is_Dirty() ?
		"Unsaved" :
		m_CameraStatus.c_str());

	ImGui::TextDisabled(
		"Free Camera: RMB + Drag = Look | WASD = Move");
}

void Client::CMapTool::Render_PlaceholderPanel(
	const char* message)
{
	ImGui::TextUnformatted(message);
}
```

### 3.21 `Client/Private/MainApp.cpp` — 전체 mouse 차단과 LMB 차단 분리

`Update()`의 Debug 입력 차단 블록을 다음으로 교체한다.

```cpp
    const bool_t bMapToolOpen =
        nullptr != m_pMapTool &&
        m_pMapTool->IsOpen();
    const HWND hForegroundWindow = GetForegroundWindow();
    const bool_t bExternalToolFocused =
        bMapToolOpen &&
        nullptr != hForegroundWindow &&
        hForegroundWindow != g_hWnd &&
        IsWindowOwnedByCurrentProcess(hForegroundWindow);

    const bool_t bImGuiPanelOpen =
        bMapToolOpen ||
        m_bProfilerVisible;
    const bool_t bKeyboardCaptured =
        bImGuiPanelOpen &&
        nullptr != m_pImGuiLayer &&
        (m_pImGuiLayer->WantsCaptureKeyboard() ||
            bExternalToolFocused);
    const bool_t bMouseCaptured =
        bImGuiPanelOpen &&
        nullptr != m_pImGuiLayer &&
        (m_pImGuiLayer->WantsCaptureMouse() ||
            bExternalToolFocused);
    const bool_t bWorldLeftButtonCaptured =
        nullptr != m_pMapTool &&
        m_pMapTool->ConsumesWorldLeftButton();

    CGameInstance::Get().SetInputBlocked(
        bKeyboardCaptured,
        bMouseCaptured);
    CGameInstance::Get().SetMouseButtonBlocked(
        DIM::LB,
        bWorldLeftButtonCaptured);
```

`Free()`의 Debug 정리에도 LMB mask 해제를 추가한다.

```cpp
    CGameInstance::Get().SetInputBlocked(false, false);
    CGameInstance::Get().SetMouseButtonBlocked(
        DIM::LB,
        false);
```

## 4. 프로젝트 등록

### 4.1 `Client/Default/Client.vcxproj`

기존 `ClInclude` ItemGroup에 추가한다.

```xml
<ClInclude Include="..\Public\CameraPresetDocument.h" />
<ClInclude Include="..\Public\NavigationConditionRuntime.h" />
```

기존 `ClCompile` ItemGroup에 추가한다.

```xml
<ClCompile Include="..\Private\CameraPresetDocument.cpp" />
<ClCompile Include="..\Private\NavigationConditionRuntime.cpp" />
```

DataFiles의 기존 Navigation 두 항목을 다음 다섯 항목으로 맞춘다.

```xml
<None Include="..\Bin\DataFiles\Navigation\ValtanArena.navsource" />
<None Include="..\Bin\DataFiles\Navigation\ValtanArena.navpaint" />
<None Include="..\Bin\DataFiles\Navigation\ValtanArena.navgrid" />
<None Include="..\Bin\DataFiles\Navigation\ValtanArena.navblockers" />
<None Include="..\Bin\DataFiles\Camera\ValtanArena.camera" />
```

`.navpaint`와 `.navgrid`도 실제 저장/실행 계약이므로 `.navsource`, `.navblockers`와 함께
프로젝트에 등록한다.

### 4.2 `Client/Default/Client.vcxproj.filters`

기존 filter ItemGroup에 추가한다.

```xml
<Filter Include="96.DataFiles\Navigation">
  <UniqueIdentifier>{BDDDC905-5B75-40A7-B77D-67DDE430E5A6}</UniqueIdentifier>
</Filter>
<Filter Include="96.DataFiles\Camera">
  <UniqueIdentifier>{61F0EFB9-B737-4CE8-9481-260835E42882}</UniqueIdentifier>
</Filter>
```

source/header 항목을 기존 물리 역할 filter에 추가한다.

```xml
<ClCompile Include="..\Private\CameraPresetDocument.cpp">
  <Filter>03. Tools\00. Map</Filter>
</ClCompile>
<ClCompile Include="..\Private\NavigationConditionRuntime.cpp">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClCompile>
<ClInclude Include="..\Public\CameraPresetDocument.h">
  <Filter>03. Tools\00. Map</Filter>
</ClInclude>
<ClInclude Include="..\Public\NavigationConditionRuntime.h">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClInclude>
```

DataFiles 항목을 추가한다.

```xml
<None Include="..\Bin\DataFiles\Navigation\ValtanArena.navsource">
  <Filter>96.DataFiles\Navigation</Filter>
</None>
<None Include="..\Bin\DataFiles\Navigation\ValtanArena.navpaint">
  <Filter>96.DataFiles\Navigation</Filter>
</None>
<None Include="..\Bin\DataFiles\Navigation\ValtanArena.navgrid">
  <Filter>96.DataFiles\Navigation</Filter>
</None>
<None Include="..\Bin\DataFiles\Navigation\ValtanArena.navblockers">
  <Filter>96.DataFiles\Navigation</Filter>
</None>
<None Include="..\Bin\DataFiles\Camera\ValtanArena.camera">
  <Filter>96.DataFiles\Camera</Filter>
</None>
```

Engine 쪽은 신규 물리 파일이 없으므로 `Engine.vcxproj`와 `.filters` 등록 변경이 없다.
Engine public header와 기존 CPP만 수정한다.

## 5. 적용 순서와 검증

### 5.1 적용 순서

1. Engine의 button별 mouse mask를 추가한다.
2. Engine의 `Get_GameObject`, `Get_NavGridDesc` 범용 계약을 추가한다.
3. `CCamera_Free`를 Follow Player/Free Camera mode로 교체한다.
4. `CCameraPresetDocument`와 기본 `.camera` 파일을 추가한다.
5. `CNavigationConditionRuntime`을 추가하고 Loader/AssetTest에 연결한다.
6. `CMapTool`에서 runtime blocker 소유권을 제거하고 Camera/placeholder panel을 연결한다.
7. `.vcxproj`와 `.vcxproj.filters`에 신규 C++ 및 모든 Navigation/Camera 데이터를 등록한다.
8. 아래 순서로 Debug와 Release를 빌드한다.

```powershell
msbuild Engine\Default\Engine.vcxproj /p:Configuration=Debug /p:Platform=x64
.\UpdateLib.bat Debug
msbuild Client\Default\Client.vcxproj /p:Configuration=Debug /p:Platform=x64

msbuild Engine\Default\Engine.vcxproj /p:Configuration=Release /p:Platform=x64
.\UpdateLib.bat Release
msbuild Client\Default\Client.vcxproj /p:Configuration=Release /p:Platform=x64
```

Engine public header를 수정하므로 `UpdateLib.bat`을 생략하지 않는다. 실행 중인
`Client.exe`가 출력물을 점유하면 종료한 뒤 Client link를 다시 수행한다.

### 5.2 실행 검증

#### A. Navigation paint와 카메라 입력 분리

1. Debug Client 실행 → F2로 AssetTest → F1으로 MapTool → Navigation.
2. 게임 화면에서 LMB drag로 yellow/green paint가 계속 되는지 확인한다.
3. 같은 화면에서 RMB를 누른 채 drag하면 camera가 회전하는지 확인한다.
4. RMB를 누르지 않은 mouse 이동만으로 camera가 흔들리지 않는지 확인한다.
5. paint 중 player가 LMB 목적지 이동을 동시에 시작하지 않는지 확인한다.
6. MapTool 창 위에서는 camera 회전과 paint가 모두 발생하지 않는지 확인한다.

#### B. Camera panel과 저장

1. Camera 탭 진입 시 `Follow Player`와 위치 offset/회전값만 보이는지 확인한다.
2. offset XYZ와 Pitch/Yaw/Roll을 바꾸면 Follow camera가 즉시 반영되는지 확인한다.
3. `Free Camera` 버튼을 누르고 게임 화면에서 RMB drag + WASD가 동작하는지 확인한다.
4. `Return to Player Camera`를 누르면 현재 player를 기준으로 저장 preset 위치로 즉시
   복귀하는지 확인한다.
5. `Save Camera` 후 Client를 재시작하고 AssetTest에 진입해 같은 구도가 복구되는지 확인한다.
6. `.camera` 파일 숫자를 비정상 값 또는 trailing token으로 바꾼 경우 Level은 기본 preset으로
   열리고, MapTool load 실패가 기존 파일을 덮어쓰지 않는지 확인한다.

#### C. 동적 walkable/nonwalkable

1. Navigation → Destruction Area에서 region을 하나 만든다.
2. `Block after destruction`을 선택하고 실제 cell을 magenta 영역으로 칠한다.
3. `Save Navigation` 후 AssetTest에 다시 진입한다.
4. `Destroyed = false`에서 해당 영역이 green이고 경로가 통과하는지 확인한다.
5. `Destroyed = true`에서 해당 영역이 yellow가 되고 경로가 우회하거나 실패하는지 확인한다.
6. 다시 `false`로 바꾸면 green과 경로가 복구되는지 확인한다.
7. 별도 region을 `Open after destruction`으로 만들어 초기에는 막히고 파괴 후 열리는 반대
   조건도 확인한다.
8. 같은 cell에 두 region을 겹쳐 만든 뒤 하나만 해제해도 다른 blocker가 활성 상태이면 계속
   막히는지 확인한다.
9. 존재하지 않는 condition ID를 호출하면 기존 runtime 상태가 유지되고 실패 status가
   표시되는지 확인한다.

이번 단계에서는 ImGui의 `Destroyed` checkbox가 trigger 역할을 대신한다. 실제 Valtan 패턴
연결 단계에서는 다음 한 줄과 동일한 public endpoint를 호출한다.

```cpp
m_pNavigationRuntime->Set_Condition(
	"VALTAN_ARENA_DESTROYED",
	true);
```

#### D. 빈 탭과 UI 본질 검증

1. Mode bar에는 `Map Assets / Navigation / Camera / Boss Pattern / Sequencer`만 보인다.
2. Boss Pattern 탭에는 `Boss Pattern tuning - next stage` 한 줄만 보인다.
3. Sequencer 탭에는 `Sequencer - next stage` 한 줄만 보인다.
4. 빈 탭을 위해 새 Boss/Sequencer class, data file, runtime state가 생기지 않았는지 diff로
   확인한다.

### 5.3 실패 시 상태 보존

- camera load 실패: in-memory 기본/이전 preset을 유지하고 원본 파일을 덮어쓰지 않는다.
- camera save 실패: `.tmp`를 제거하고 dirty 상태를 유지한다.
- blocker parse 실패: `CNavigationConditionRuntime::Initialize`가 실패해 부분 region 등록을
  남기지 않는다.
- condition 적용 중 실패: 앞서 바꾼 region을 이전 condition 값으로 rollback한다.
- Navigation authoring save 실패: 기존 `.navpaint`, `.navgrid`, `.navblockers`는 현재
  `Save_Navigation()`의 기존 실패 보존 계약을 그대로 따른다.

### 5.4 완료 기준

다음이 모두 확인되어야 이번 단계를 닫는다.

- Engine Debug/Release → UpdateLib Debug/Release → Client Debug/Release 빌드 성공
- Navigation LMB paint와 RMB camera rotation이 충돌 없이 동작
- Follow preset 수정·저장·재실행 복구 성공
- Free Camera 버튼, RMB drag, WASD, Follow 복귀 성공
- `Block after destruction`과 `Open after destruction` 양방향 runtime condition 성공
- Boss Pattern/Sequencer는 탭 한 줄 외의 선행 구현이 없음
- 신규 C++와 `.camera`, Navigation 4종 데이터가 `.vcxproj`와 `.vcxproj.filters`에 등록됨
