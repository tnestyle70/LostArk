# LostArk MapTool 플레이어 카메라 튜닝·저장 구현 계획

> 작성일: 2026-07-31  
> 기준 저장소: `C:\Users\user\Desktop\LostArk`  
> 계획 스위치: `C1~C8 OFF` / `문제 해결 ①~⑤ OFF` / `자료구조·알고리즘 OFF`  
> 이번 문서 범위: **카메라만**. BossPattern과 Sequencer는 구현하지 않는다.

## 변경 범위

이번 단계의 완료 상태는 다음 한 줄이다.

> `ASSET_TEST` 진입 시 카메라가 플레이어를 추적하고, MapTool의 `Camera` 화면에서 시점을 실시간 조절한 뒤 `Save Camera`로 저장하며, 재진입·재실행 후 같은 값으로 시작한다. `Free Camera`로 전환한 상태에서는 Navigation 화면으로 돌아가도 RMB 회전과 LMB cell 피킹을 동시에 사용할 수 있다.

현재 코드 실측 결과는 다음과 같다.

- `CCamera_Free`에는 이미 `follow target`, 위치 오프셋, 주시점 오프셋, F6 전환 골격이 있다.
- `CLevel_AssetTest::Initialize()`는 카메라를 플레이어보다 먼저 생성하므로 현재 descriptor에 플레이어 Transform을 넣을 수 없다.
- `CMapTool`의 상단 모드는 `Map Assets`, `Navigation` 두 개뿐이다.
- `CMainApp`은 Navigation 편집 중 전체 마우스를 차단한다. 이 때문에 LMB 피킹뿐 아니라 RMB와 mouse delta도 함께 막혀 카메라 회전이 불가능하다.
- 현재 카메라는 Follow가 아닐 때 mouse delta만 들어오면 항상 회전한다. 이번에는 `Free Camera + RMB hold`일 때만 회전하도록 동작을 명확히 한다.
- 저장 가능한 카메라 문서는 아직 없다.

Winters의 `CDynamicCamera`에서 가져올 원리는 네 개뿐이다.

1. 플레이어 Transform은 카메라가 소유하지 않고 약한 참조로 추적한다.
2. 카메라 위치는 `player position + position offset`이다.
3. 카메라 주시점은 `player position + look target offset`이다.
4. Follow와 Free는 하나의 카메라 인스턴스 안에서 명시적으로 전환한다.

이번에 가져오지 않는 것은 edge scroll, camera shake, cursor lock, 별도 Camera Debug 창, 시네마틱 트랙이다.

### 실질 동작

```text
ASSET_TEST 진입
    Character 생성
        ↓ Transform 전달
    Camera_Free 생성
        ↓ ValtanArena.camera 로드값 적용
    Follow Player 시작

매 프레임 Follow Player
    desired eye = player position + positionOffset
    desired at  = player position + lookOffset
    current eye/at을 followResponse로 보간
    View/Projection 갱신

MapTool > Camera
    DragFloat3 / Slider 수정
        ↓
    CameraPresetDocument draft 갱신
        ↓
    실행 중 Camera_Free에 즉시 적용
        ↓
    Save Camera
        ↓
    Client/Bin/DataFiles/Camera/ValtanArena.camera 원자 저장

Switch to Free Camera
    Follow 정지
    WASD 이동
    RMB hold + mouse delta 회전

Navigation 화면으로 복귀
    LMB는 MapTool만 소비
    RMB와 mouse delta는 Camera_Free로 전달
    결과: 카메라를 돌려가며 cell 피킹 가능
```

저장값의 의미는 다음과 같이 고정한다.

| 저장 필드 | 의미 | 기본값 |
|---|---|---:|
| `POSITION_OFFSET x y z` | 플레이어 위치로부터 카메라 Eye까지의 월드 축 오프셋 | `-12, 16, -12` |
| `LOOK_OFFSET x y z` | 플레이어 위치로부터 카메라가 바라볼 점까지의 월드 축 오프셋 | `0, 1.2, 0` |
| `FOV_Y` | 세로 시야각, degree | `60` |
| `FOLLOW_RESPONSE` | 플레이어 추적 반응 속도. `0`이면 즉시 추적 | `18` |

`POSITION_OFFSET`과 `LOOK_OFFSET`을 같이 조절하면 별도 Euler angle 저장 없이 시점 각도가 결정된다. 이 방식은 저장값만 읽어도 의미가 분명하고 `LookAt()`과 직접 연결된다.

### 단계별 반영 순서

#### Phase 0 — Navigation 세션과 겹침 확인

- 다른 세션이 수정 중인 `MapTool.h/.cpp`, `MainApp.cpp`, `Level_AssetTest.cpp`, `Client.vcxproj`, `Client.vcxproj.filters`, `GameInstance.h`를 구현 직전에 다시 읽는다.
- 이 문서의 코드를 과거 파일 전체에 덮어쓰지 않는다.
- 아래의 함수 단위 최종 코드만 현재 파일에 합친다.
- Navigation의 저장·피킹·색상 로직은 변경하지 않는다.

#### Phase 1 — Engine의 최소 조회·입력 경계

- `Layer -> Object_Manager -> GameInstance`의 기존 계층을 따라 index 기반 `Get_GameObject()`를 추가한다.
- ImGui 전체 capture와 MapTool의 LMB 소비를 분리한다.
- 전체 ImGui capture는 기존처럼 keyboard/mouse 전체를 막는다.
- Navigation/placement의 world 입력은 `DIM::LB`만 막는다.
- Engine은 Camera나 MapTool을 알지 않는다.

#### Phase 2 — 플레이어 추적 카메라와 저장 문서

- 기존 `CCamera_Free` 하나를 Follow/Free 두 동작에 계속 사용한다.
- F6/Tab 내부 토글을 제거하고 MapTool의 명시적인 버튼을 단일 진입점으로 사용한다.
- Follow에는 Winters와 같은 exponential response를 사용한다.
- 새 `CCameraPresetDocument`는 `parse -> validate -> stage -> commit`을 지킨다.
- 저장은 임시 파일 작성 후 `ReplaceFileW`/`MoveFileExW`로 교체한다.

#### Phase 3 — ASSET_TEST 생성 순서

- `Character -> Camera -> Valtan` 순으로 생성한다.
- 카메라 descriptor에 플레이어 Transform과 저장 preset을 넣는다.
- MapTool을 열지 않아도 저장된 카메라 값이 런타임 시작값이 된다.

#### Phase 4 — MapTool Camera 화면

- 별도 `RaidTuningTool` 창을 만들지 않는다.
- 현재 `LostArk Map Tool` 상단 mode bar에 `Camera`를 추가한다.
- 기본 화면에는 Follow/Free 전환, 네 튜닝값, `Save Camera`, 상태 한 줄만 둔다.
- 파일 경로 같은 진단 정보는 `?` tooltip에만 둔다.
- ImGui는 매 프레임 파일을 읽지 않는다. 파일 읽기는 레벨 진입 1회, 쓰기는 Save 클릭 1회다.

#### Phase 5 — 저장 재현·Navigation 동시 입력·빌드 검증

- 저장 전후 값, 레벨 재진입, 프로세스 재실행을 확인한다.
- Free Camera 상태로 Navigation에 돌아가 RMB 회전과 LMB cell 피킹을 함께 확인한다.
- Engine public header 변경이 있으므로 Engine, UpdateLib, Client 순서를 모두 닫는다.

## 추가·수정·삭제 파일

### 의존성과 배치

| 물리 파일 | 소유 프로젝트 | 역할 | 의존 방향 |
|---|---|---|---|
| `Engine/Public/Input_Device.h` | Engine | 전체 mouse block과 button별 block 분리 | 범용 입력 |
| `Engine/Public/Layer.h`, `Engine/Private/Layer.cpp` | Engine | layer object index 조회 | 범용 object 계층 |
| `Engine/Public/Object_Manager.h`, `Engine/Private/Object_Manager.cpp` | Engine | level/layer object 조회 전달 | `Object_Manager -> Layer` |
| `Engine/Public/GameInstance.h`, `Engine/Private/GameInstance.cpp` | Engine | Client가 쓰는 범용 facade | `Client -> GameInstance` |
| `Client/Public/CameraPresetDocument.h` | Client | LostArk 카메라 저장 계약 | Client 데이터 |
| `Client/Private/CameraPresetDocument.cpp` | Client | 검증·원자 저장 | Client 데이터 |
| `Client/Public/Camera_Free.h` | Client | Follow/Free 상태와 튜닝 API | `Camera_Free -> Engine Camera/Transform` |
| `Client/Private/Camera_Free.cpp` | Client | 플레이어 추적과 free input | Client 런타임 |
| `Client/Private/Level_AssetTest.cpp` | Client | 플레이어 우선 생성과 preset 적용 | `Level -> Character/CameraPreset/Camera` |
| `Client/Public/MapTool.h` | Client | Camera mode와 draft 상태 | Debug authoring |
| `Client/Private/MapTool.cpp` | Client | Camera 화면·적용·저장 명령 | `MapTool -> CameraPreset/Camera_Free` |
| `Client/Private/MainApp.cpp` | Client | ImGui 전체 capture와 LMB 소비 분리 | Debug 입력 조정 |
| `Client/Bin/DataFiles/Camera/ValtanArena.camera` | Client DataFiles | 발탄 아레나 카메라 기본값 | 런타임/툴 공용 |
| `Client/Default/Client.vcxproj` | Client | 신규 코드·데이터 등록 | 빌드 |
| `Client/Default/Client.vcxproj.filters` | Client | 물리 역할과 같은 필터 등록 | IDE |

### 추가

- `Client/Public/CameraPresetDocument.h`
- `Client/Private/CameraPresetDocument.cpp`
- `Client/Bin/DataFiles/Camera/ValtanArena.camera`

### 수정

- `Engine/Public/Input_Device.h`
- `Engine/Public/Layer.h`
- `Engine/Private/Layer.cpp`
- `Engine/Public/Object_Manager.h`
- `Engine/Private/Object_Manager.cpp`
- `Engine/Public/GameInstance.h`
- `Engine/Private/GameInstance.cpp`
- `Client/Public/Camera_Free.h`
- `Client/Private/Camera_Free.cpp`
- `Client/Private/Level_AssetTest.cpp`
- `Client/Public/MapTool.h`
- `Client/Private/MapTool.cpp`
- `Client/Private/MainApp.cpp`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`

### 삭제

- 없음
- 별도 `RaidTuningTool` 또는 두 번째 Camera GameObject는 만들지 않는다.

## 파일별 최종 반영 코드

### Phase 1 — Engine 최소 조회·입력 경계

### 1. `Engine/Public/Input_Device.h`

`Get_DIMouseState()`, `SetInputBlocked()` 구간과 입력 차단 멤버를 다음 최종 코드로 교체한다.

```cpp
	int8_t Get_DIMouseState(DIM eMouse)
	{
		const uint32_t index = ETOUI(eMouse);
		if (m_bMouseBlocked ||
			index >= ETOUI(DIM::END) ||
			m_MouseButtonBlocked[index])
		{
			return 0;
		}

		return m_tMouseState.rgbButtons[index];
	}

	// 현재 마우스의 특정 축 좌표를 반환
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

	void SetMouseButtonBlocked(DIM eMouse, bool_t blocked)
	{
		const uint32_t index = ETOUI(eMouse);
		if (index < ETOUI(DIM::END))
			m_MouseButtonBlocked[index] = blocked;
	}

	bool_t IsKeyboardInputBlocked() const
	{
		return m_bKeyboardBlocked;
	}

	bool_t IsMouseInputBlocked() const
	{
		return m_bMouseBlocked;
	}
```

기존 `m_bKeyboardBlocked`, `m_bMouseBlocked` 멤버 구간은 다음으로 교체한다.

```cpp
	bool_t m_bKeyboardBlocked = false;
	bool_t m_bMouseBlocked = false;
	bool_t m_MouseButtonBlocked[ETOUI(DIM::END)] = {};
```

핵심은 `Get_DIMouseMove()`가 button별 block에는 영향받지 않는다는 점이다. Navigation이 LMB를 소유해도 RMB 상태와 X/Y delta는 카메라로 전달된다.

### 2. `Engine/Public/Layer.h`

첫 번째 `public:` 조회 구간에 다음 선언을 추가한다.

```cpp
	shared_ptr<CGameObject> Get_GameObject(uint32_t iIndex);
```

최종 조회 구간은 다음과 같다.

```cpp
public:
	shared_ptr<CGameObject> Get_GameObject(uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(
		const wstring_t& strComponentTag,
		uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(
		const wstring_t& strPartTag,
		const wstring_t& strComponentTag,
		uint32_t iIndex);
```

### 3. `Engine/Private/Layer.cpp`

소멸자 다음, 기존 첫 `Get_Component()` 앞에 다음 함수 전체를 삽입한다.

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

### 4. `Engine/Public/Object_Manager.h`

조회 구간을 다음 최종 선언으로 교체한다.

```cpp
public:
	shared_ptr<CGameObject> Get_GameObject(
		uint32_t iLevelIndex,
		const wstring_t& strLayerTag,
		uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(
		uint32_t iLevelIndex,
		const wstring_t& strLayerTag,
		const wstring_t& strComponentTag,
		uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(
		uint32_t iLevelIndex,
		const wstring_t& strLayerTag,
		const wstring_t& strPartTag,
		const wstring_t& strComponentTag,
		uint32_t iIndex);
```

### 5. `Engine/Private/Object_Manager.cpp`

소멸자 다음, 기존 첫 `Get_Component()` 앞에 다음 함수 전체를 삽입한다.

```cpp
shared_ptr<CGameObject> CObject_Manager::Get_GameObject(
	uint32_t iLevelIndex,
	const wstring_t& strLayerTag,
	uint32_t iIndex)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	CLayer* pLayer = Find_Layer(iLevelIndex, strLayerTag);
	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_GameObject(iIndex);
}
```

### 6. `Engine/Public/GameInstance.h`

`For.Input_Device` 구간에 다음 선언을 추가한다.

```cpp
	void SetMouseButtonBlocked(DIM eMouse, bool_t blocked);
```

최종 입력 차단 선언은 다음과 같다.

```cpp
	void SetInputBlocked(
		bool_t bKeyboardBlocked,
		bool_t bMouseBlocked);
	void SetMouseButtonBlocked(DIM eMouse, bool_t blocked);
	bool_t IsKeyboardInputBlocked() const;
	bool_t IsMouseInputBlocked() const;
```

`For.Object_Manager` 구간 맨 앞에 다음 선언을 추가한다.

```cpp
	shared_ptr<class CGameObject> Get_GameObject(
		uint32_t iLevelIndex,
		const wstring_t& strLayerTag,
		uint32_t iIndex);
```

### 7. `Engine/Private/GameInstance.cpp`

`SetInputBlocked()` 다음에 다음 함수 전체를 삽입한다.

```cpp
void CGameInstance::SetMouseButtonBlocked(
	DIM eMouse,
	bool_t blocked)
{
	if (nullptr != m_pInput_Device)
		m_pInput_Device->SetMouseButtonBlocked(eMouse, blocked);
}
```

`Clone_Prototype()` 다음, 기존 첫 `Get_Component()` 앞에 다음 함수 전체를 삽입한다.

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

### Phase 2 — 저장 문서와 Camera_Free

### 8. `Client/Public/CameraPresetDocument.h` — 신규 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"

#include <filesystem>
#include <string>

NS_BEGIN(Client)

struct CAMERA_PRESET final
{
	float3_t positionOffset = float3_t(-12.f, 16.f, -12.f);
	float3_t lookOffset = float3_t(0.f, 1.2f, 0.f);
	f32_t fovY = 60.f;
	f32_t followResponse = 18.f;
};

class CCameraPresetDocument final
{
public:
	static std::filesystem::path Resolve_DefaultPath();
	static bool_t Is_Valid(const CAMERA_PRESET& preset);

public:
	bool_t Load(
		const std::filesystem::path& path,
		std::string& outStatus);
	bool_t Set_Preset(
		const CAMERA_PRESET& preset,
		std::string& outStatus);
	bool_t Save(
		const std::filesystem::path& path,
		std::string& outStatus);

public:
	bool_t Is_Ready() const { return m_isReady; }
	bool_t Is_Dirty() const { return m_isDirty; }
	const CAMERA_PRESET& Get_Preset() const { return m_Preset; }

private:
	CAMERA_PRESET m_Preset;
	CAMERA_PRESET m_SavedPreset;
	bool_t m_isReady = false;
	bool_t m_isDirty = false;
};

NS_END
```

### 9. `Client/Private/CameraPresetDocument.cpp` — 신규 전체 코드

```cpp
#include "CameraPresetDocument.h"

#include <cmath>
#include <fstream>
#include <iomanip>

namespace
{
	constexpr const char* CAMERA_MAGIC =
		"LOSTARK_CAMERA_PRESET";
	constexpr uint32_t CAMERA_VERSION = 1;

	bool_t IsFinite(const float3_t& value)
	{
		return
			std::isfinite(value.x) &&
			std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	bool_t IsSameFloat(f32_t left, f32_t right)
	{
		return std::fabs(left - right) <= 0.000001f;
	}

	bool_t IsSamePreset(
		const Client::CAMERA_PRESET& left,
		const Client::CAMERA_PRESET& right)
	{
		return
			IsSameFloat(
				left.positionOffset.x,
				right.positionOffset.x) &&
			IsSameFloat(
				left.positionOffset.y,
				right.positionOffset.y) &&
			IsSameFloat(
				left.positionOffset.z,
				right.positionOffset.z) &&
			IsSameFloat(
				left.lookOffset.x,
				right.lookOffset.x) &&
			IsSameFloat(
				left.lookOffset.y,
				right.lookOffset.y) &&
			IsSameFloat(
				left.lookOffset.z,
				right.lookOffset.z) &&
			IsSameFloat(left.fovY, right.fovY) &&
			IsSameFloat(
				left.followResponse,
				right.followResponse);
	}

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
			MOVEFILE_REPLACE_EXISTING |
				MOVEFILE_WRITE_THROUGH);
	}

	void RemoveTemporaryFile(
		const std::filesystem::path& temporary)
	{
		std::error_code error;
		std::filesystem::remove(temporary, error);
	}
}

std::filesystem::path
Client::CCameraPresetDocument::Resolve_DefaultPath()
{
	wchar_t modulePath[32768]{};
	const DWORD length = GetModuleFileNameW(
		nullptr,
		modulePath,
		static_cast<DWORD>(_countof(modulePath)));
	if (0 == length || length >= _countof(modulePath))
		return {};

	const std::filesystem::path moduleDirectory =
		std::filesystem::path(modulePath).parent_path();
	const std::filesystem::path adjacent =
		moduleDirectory /
		L"DataFiles" /
		L"Camera" /
		L"ValtanArena.camera";

	std::error_code error;
	if (std::filesystem::exists(
		adjacent.parent_path(),
		error) &&
		!error)
	{
		return adjacent.lexically_normal();
	}

	return (
		moduleDirectory.parent_path() /
		L"DataFiles" /
		L"Camera" /
		L"ValtanArena.camera").lexically_normal();
}

bool_t Client::CCameraPresetDocument::Is_Valid(
	const CAMERA_PRESET& preset)
{
	return
		IsFinite(preset.positionOffset) &&
		IsFinite(preset.lookOffset) &&
		std::fabs(preset.positionOffset.x) <= 1000.f &&
		std::fabs(preset.positionOffset.y) <= 1000.f &&
		std::fabs(preset.positionOffset.z) <= 1000.f &&
		std::fabs(preset.lookOffset.x) <= 1000.f &&
		std::fabs(preset.lookOffset.y) <= 1000.f &&
		std::fabs(preset.lookOffset.z) <= 1000.f &&
		std::isfinite(preset.fovY) &&
		preset.fovY >= 20.f &&
		preset.fovY <= 120.f &&
		std::isfinite(preset.followResponse) &&
		preset.followResponse >= 0.f &&
		preset.followResponse <= 60.f;
}

bool_t Client::CCameraPresetDocument::Load(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	if (path.empty())
	{
		outStatus = "Camera preset path is unavailable";
		return false;
	}

	std::error_code existsError;
	const bool_t exists =
		std::filesystem::exists(path, existsError);
	if (existsError)
	{
		outStatus = "Could not inspect camera preset";
		return false;
	}

	if (!exists)
	{
		m_Preset = CAMERA_PRESET{};
		m_SavedPreset = m_Preset;
		m_isReady = true;
		m_isDirty = false;
		outStatus = "Camera defaults loaded";
		return true;
	}

	std::ifstream input(path, std::ios::binary);
	std::string magic;
	uint32_t version = {};
	std::string positionKey;
	std::string lookKey;
	std::string fovKey;
	std::string responseKey;
	CAMERA_PRESET stagedPreset;

	if (!input ||
		!(input >>
			magic >>
			version >>
			positionKey >>
			stagedPreset.positionOffset.x >>
			stagedPreset.positionOffset.y >>
			stagedPreset.positionOffset.z >>
			lookKey >>
			stagedPreset.lookOffset.x >>
			stagedPreset.lookOffset.y >>
			stagedPreset.lookOffset.z >>
			fovKey >>
			stagedPreset.fovY >>
			responseKey >>
			stagedPreset.followResponse) ||
		magic != CAMERA_MAGIC ||
		version != CAMERA_VERSION ||
		positionKey != "POSITION_OFFSET" ||
		lookKey != "LOOK_OFFSET" ||
		fovKey != "FOV_Y" ||
		responseKey != "FOLLOW_RESPONSE" ||
		!Is_Valid(stagedPreset))
	{
		outStatus = "Camera preset is invalid";
		return false;
	}

	input >> std::ws;
	if (input.peek() != std::char_traits<char>::eof())
	{
		outStatus = "Camera preset has trailing data";
		return false;
	}

	m_Preset = stagedPreset;
	m_SavedPreset = stagedPreset;
	m_isReady = true;
	m_isDirty = false;
	outStatus = "Camera loaded";
	return true;
}

bool_t Client::CCameraPresetDocument::Set_Preset(
	const CAMERA_PRESET& preset,
	std::string& outStatus)
{
	if (!m_isReady)
	{
		outStatus = "Camera preset is not loaded";
		return false;
	}

	if (!Is_Valid(preset))
	{
		outStatus = "Camera values are invalid";
		return false;
	}

	m_Preset = preset;
	m_isDirty = !IsSamePreset(
		m_Preset,
		m_SavedPreset);
	outStatus = m_isDirty ? "Unsaved" : "Saved";
	return true;
}

bool_t Client::CCameraPresetDocument::Save(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	if (!m_isReady || !Is_Valid(m_Preset))
	{
		outStatus = "Camera preset is not ready";
		return false;
	}

	if (path.empty())
	{
		outStatus = "Camera preset path is unavailable";
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(
		path.parent_path(),
		directoryError);
	if (directoryError)
	{
		outStatus = "Could not create camera data directory";
		return false;
	}

	std::filesystem::path temporary = path;
	temporary += L".tmp";

	std::ofstream output(
		temporary,
		std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create camera temporary file";
		return false;
	}

	output << std::setprecision(9);
	output << CAMERA_MAGIC << ' ' << CAMERA_VERSION << '\n';
	output << "POSITION_OFFSET "
		<< m_Preset.positionOffset.x << ' '
		<< m_Preset.positionOffset.y << ' '
		<< m_Preset.positionOffset.z << '\n';
	output << "LOOK_OFFSET "
		<< m_Preset.lookOffset.x << ' '
		<< m_Preset.lookOffset.y << ' '
		<< m_Preset.lookOffset.z << '\n';
	output << "FOV_Y "
		<< m_Preset.fovY << '\n';
	output << "FOLLOW_RESPONSE "
		<< m_Preset.followResponse << '\n';
	output.flush();

	if (!output)
	{
		output.close();
		RemoveTemporaryFile(temporary);
		outStatus = "Could not write camera temporary file";
		return false;
	}
	output.close();

	if (!CommitTemporaryFile(path, temporary))
	{
		RemoveTemporaryFile(temporary);
		outStatus = "Could not replace camera preset";
		return false;
	}

	m_SavedPreset = m_Preset;
	m_isDirty = false;
	outStatus = "Saved";
	return true;
}
```

### 10. `Client/Public/Camera_Free.h` — 파일 전체 교체

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
	typedef struct tagCameraFreeDesc :
		public CCamera::CAMERA_DESC
	{
		f32_t fMouseSensor = 0.1f;
		shared_ptr<CTransform> pFollowTarget = { nullptr };
		float3_t vPositionOffset =
			float3_t(-12.f, 16.f, -12.f);
		float3_t vLookOffset =
			float3_t(0.f, 1.2f, 0.f);
		f32_t fFollowResponse = 18.f;
		bool_t isFollowEnabled = false;
	} CAMERA_FREE_DESC;

private:
	CCamera_Free(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CCamera_Free();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Set_FollowTarget(
		const shared_ptr<CTransform>& pFollowTarget);
	void Set_FollowEnabled(bool_t isEnabled);
	bool_t Is_FollowEnabled() const {
		return m_bFollowEnabled;
	}
	void Set_PlayerCameraTuning(
		const float3_t& positionOffset,
		const float3_t& lookOffset,
		f32_t fovY,
		f32_t followResponse);

	const float3_t& Get_PositionOffset() const {
		return m_vPositionOffset;
	}
	const float3_t& Get_LookOffset() const {
		return m_vLookOffset;
	}
	f32_t Get_FovY() const {
		return m_fFovy;
	}
	f32_t Get_FollowResponse() const {
		return m_fFollowResponse;
	}

private:
	void Update_PlayerCamera(f32_t fTimeDelta);
	void Update_FreeCamera(f32_t fTimeDelta);

private:
	f32_t m_fMouseSensor = 0.1f;
	bool_t m_bFollowEnabled = false;
	bool_t m_bFollowInitialized = false;
	weak_ptr<CTransform> m_pFollowTarget;
	float3_t m_vPositionOffset =
		float3_t(-12.f, 16.f, -12.f);
	float3_t m_vLookOffset =
		float3_t(0.f, 1.2f, 0.f);
	float3_t m_vCurrentLookAt = {};
	f32_t m_fFollowResponse = 18.f;

public:
	static unique_ptr<CCamera_Free> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### 11. `Client/Private/Camera_Free.cpp` — 파일 전체 교체

```cpp
#include "Camera_Free.h"

#include "Transform.h"

#include <algorithm>
#include <cmath>

namespace
{
	bool_t IsFinite(const float3_t& value)
	{
		return
			std::isfinite(value.x) &&
			std::isfinite(value.y) &&
			std::isfinite(value.z);
	}
}

CCamera_Free::CCamera_Free(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CCamera { pDevice, pContext }
{
}

CCamera_Free::~CCamera_Free()
{
}

HRESULT CCamera_Free::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_Free::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	auto pDesc = static_cast<CAMERA_FREE_DESC*>(pArg);

	m_fMouseSensor = pDesc->fMouseSensor;
	m_pFollowTarget = pDesc->pFollowTarget;
	m_vPositionOffset = pDesc->vPositionOffset;
	m_vLookOffset = pDesc->vLookOffset;
	m_fFollowResponse = pDesc->fFollowResponse;
	m_bFollowEnabled =
		pDesc->isFollowEnabled &&
		nullptr != pDesc->pFollowTarget;
	m_vCurrentLookAt = pDesc->vAt;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Free::Priority_Update(f32_t fTimeDelta)
{
	if (m_bFollowEnabled)
		Update_PlayerCamera(fTimeDelta);
	else
		Update_FreeCamera(fTimeDelta);

	__super::Update_PipeLine();
}

void CCamera_Free::Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
}

void CCamera_Free::Late_Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
}

HRESULT CCamera_Free::Render()
{
	return S_OK;
}

void CCamera_Free::Set_FollowTarget(
	const shared_ptr<CTransform>& pFollowTarget)
{
	if (m_pFollowTarget.lock() != pFollowTarget)
		m_bFollowInitialized = false;

	m_pFollowTarget = pFollowTarget;
	if (nullptr == pFollowTarget)
		m_bFollowEnabled = false;
}

void CCamera_Free::Set_FollowEnabled(bool_t isEnabled)
{
	const bool_t nextEnabled =
		isEnabled && !m_pFollowTarget.expired();
	if (m_bFollowEnabled != nextEnabled)
		m_bFollowInitialized = false;

	m_bFollowEnabled = nextEnabled;
}

void CCamera_Free::Set_PlayerCameraTuning(
	const float3_t& positionOffset,
	const float3_t& lookOffset,
	f32_t fovY,
	f32_t followResponse)
{
	if (!IsFinite(positionOffset) ||
		!IsFinite(lookOffset) ||
		!std::isfinite(fovY) ||
		fovY < 20.f ||
		fovY > 120.f ||
		!std::isfinite(followResponse) ||
		followResponse < 0.f ||
		followResponse > 60.f)
	{
		return;
	}

	m_vPositionOffset = positionOffset;
	m_vLookOffset = lookOffset;
	m_fFovy = fovY;
	m_fFollowResponse = followResponse;
	m_bFollowInitialized = false;
}

void CCamera_Free::Update_PlayerCamera(f32_t fTimeDelta)
{
	shared_ptr<CTransform> pFollowTarget =
		m_pFollowTarget.lock();
	if (nullptr == pFollowTarget)
	{
		m_bFollowEnabled = false;
		m_bFollowInitialized = false;
		return;
	}

	const vector_t targetPosition =
		pFollowTarget->Get_State(STATE::POSITION);
	const vector_t desiredEye = XMVectorSetW(
		targetPosition +
			XMLoadFloat3(&m_vPositionOffset),
		1.f);
	const vector_t desiredAt = XMVectorSetW(
		targetPosition +
			XMLoadFloat3(&m_vLookOffset),
		1.f);

	if (!m_bFollowInitialized ||
		fTimeDelta <= 0.f ||
		m_fFollowResponse <= 0.f)
	{
		m_pTransformCom->Set_State(
			STATE::POSITION,
			desiredEye);
		XMStoreFloat3(
			&m_vCurrentLookAt,
			desiredAt);
		m_bFollowInitialized = true;
	}
	else
	{
		const f32_t clampedDelta =
			(std::min)(fTimeDelta, 0.05f);
		const f32_t alpha =
			1.f -
			std::exp(
				-m_fFollowResponse *
				clampedDelta);
		const vector_t nextEye = XMVectorLerp(
			m_pTransformCom->Get_State(
				STATE::POSITION),
			desiredEye,
			alpha);
		const vector_t nextAt = XMVectorLerp(
			XMLoadFloat3(&m_vCurrentLookAt),
			desiredAt,
			alpha);

		m_pTransformCom->Set_State(
			STATE::POSITION,
			XMVectorSetW(nextEye, 1.f));
		XMStoreFloat3(
			&m_vCurrentLookAt,
			nextAt);
	}

	m_pTransformCom->LookAt(
		XMLoadFloat3(&m_vCurrentLookAt));
}

void CCamera_Free::Update_FreeCamera(f32_t fTimeDelta)
{
	if (CGameInstance::Get().Get_DIKeyState(DIK_W) & 0x80)
		m_pTransformCom->Go_Straight(fTimeDelta);
	if (CGameInstance::Get().Get_DIKeyState(DIK_S) & 0x80)
		m_pTransformCom->Go_Backward(fTimeDelta);
	if (CGameInstance::Get().Get_DIKeyState(DIK_A) & 0x80)
		m_pTransformCom->Go_Left(fTimeDelta);
	if (CGameInstance::Get().Get_DIKeyState(DIK_D) & 0x80)
		m_pTransformCom->Go_Right(fTimeDelta);

	const bool_t isRightMouseDown =
		0 != (
			CGameInstance::Get().Get_DIMouseState(DIM::RB) &
			0x80);
	if (!isRightMouseDown)
		return;

	const int32_t mouseMoveX =
		CGameInstance::Get().Get_DIMouseMove(DIMM::X);
	const int32_t mouseMoveY =
		CGameInstance::Get().Get_DIMouseMove(DIMM::Y);

	if (0 != mouseMoveX)
	{
		m_pTransformCom->Turn(
			XMVectorSet(0.f, 1.f, 0.f, 0.f),
			m_fMouseSensor *
				static_cast<f32_t>(mouseMoveX) *
				fTimeDelta);
	}

	if (0 != mouseMoveY)
	{
		m_pTransformCom->Turn(
			m_pTransformCom->Get_State(STATE::RIGHT),
			m_fMouseSensor *
				static_cast<f32_t>(mouseMoveY) *
				fTimeDelta);
	}
}

unique_ptr<CCamera_Free> CCamera_Free::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CCamera_Free>(
		new CCamera_Free(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CCamera_Free");

	return move(pInstance);
}

shared_ptr<CPrototype> CCamera_Free::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CCamera_Free>(
		new CCamera_Free(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CCamera_Free");

	return pInstance;
}
```

### Phase 3 — ASSET_TEST에 플레이어 기준 카메라 연결

### 12. `Client/Private/Level_AssetTest.cpp`

include 구간에 다음을 추가한다.

```cpp
#include "CameraPresetDocument.h"
```

`Initialize()` 전체를 다음으로 교체한다.

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
	if (FAILED(Ready_Valtan()))
		return E_FAIL;
	return S_OK;
}
```

`Ready_Layer_Camera()` 전체를 다음으로 교체한다.

```cpp
HRESULT CLevel_AssetTest::Ready_Layer_Camera(
	const wstring_t& strLayerTag)
{
	if (nullptr == m_pCharacter)
		return E_FAIL;

	CCameraPresetDocument cameraDocument;
	std::string cameraStatus;
	cameraDocument.Load(
		CCameraPresetDocument::Resolve_DefaultPath(),
		cameraStatus);
	const CAMERA_PRESET& preset =
		cameraDocument.Get_Preset();

	const shared_ptr<CTransform> playerTransform =
		m_pCharacter->Get_Transform();
	if (nullptr == playerTransform)
		return E_FAIL;

	const vector_t playerPosition =
		playerTransform->Get_State(STATE::POSITION);
	float3_t eye{};
	float3_t at{};
	XMStoreFloat3(
		&eye,
		playerPosition +
			XMLoadFloat3(&preset.positionOffset));
	XMStoreFloat3(
		&at,
		playerPosition +
			XMLoadFloat3(&preset.lookOffset));

	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = eye;
	cameraDesc.vAt = at;
	cameraDesc.fFovy = preset.fovY;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = 1000.f;
	cameraDesc.fSpeedPerSec = 20.f;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.pFollowTarget = playerTransform;
	cameraDesc.vPositionOffset =
		preset.positionOffset;
	cameraDesc.vLookOffset =
		preset.lookOffset;
	cameraDesc.fFollowResponse =
		preset.followResponse;
	cameraDesc.isFollowEnabled = true;

	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::ASSET_TEST),
		strLayerTag,
		&cameraDesc)))
	{
		return E_FAIL;
	}

	return S_OK;
}
```

`CTransform`은 `Camera_Free.h`의 전방 선언만으로 메서드를 호출할 수 없으므로 include 구간에 다음도 추가한다.

```cpp
#include "Transform.h"
```

즉, 최종 관련 include 순서는 다음 형태가 된다.

```cpp
#include "Level_AssetTest.h"

#include "Camera_Free.h"
#include "CameraPresetDocument.h"
#include "Character.h"
#include "GameInstance.h"
#include "Logic_LanceMaster.h"
#include "Transform.h"
#include "Valtan.h"
```

### Phase 4 — MapTool Camera 화면

### 13. `Client/Public/MapTool.h`

include 구간에 다음을 추가한다.

```cpp
#include "CameraPresetDocument.h"
```

Client namespace의 전방 선언 구간에 다음을 추가한다.

```cpp
class CCamera_Free;
```

`TOOL_MODE` 전체를 다음으로 교체한다.

```cpp
	enum class TOOL_MODE
	{
		MAP_ASSETS,
		NAVIGATION,
		CAMERA,
	};
```

public 함수 이름을 다음과 같이 바꾼다.

```cpp
	bool_t ConsumesWorldLeftMouse() const;
```

Navigation helper 선언 바로 뒤에 다음 Camera helper 선언을 추가한다.

```cpp
	bool_t Load_CameraPreset();
	bool_t Find_AssetTestCamera();
	bool_t Apply_CameraPreset(
		const CAMERA_PRESET& preset);
	void Render_CameraPanel();
```

클래스 마지막 Navigation 멤버 묶음 뒤에 다음 멤버를 추가한다.

```cpp
	CCameraPresetDocument m_CameraDocument;
	std::filesystem::path m_CameraPresetPath;
	std::string m_CameraStatus =
		"Open ASSET_TEST with F2";
	weak_ptr<CCamera_Free> m_pAssetTestCamera;
```

### 14. `Client/Private/MapTool.cpp`

include 구간에 다음을 추가한다.

```cpp
#include "Camera_Free.h"
```

`Render()` 전체를 다음으로 교체한다.

```cpp
void Client::CMapTool::Render()
{
	if (!m_bOpen)
		return;

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) ==
		CGameInstance::Get().Get_CurrentLevelID();
	if (isAssetTest &&
		TOOL_MODE::NAVIGATION == m_eToolMode)
	{
		if (NAVIGATION_MODE::BAKE == m_eNavigationMode)
			Render_NavigationBoundsOverlay();
		else
			Render_NavigationOverlay();
	}

	ImGui::SetNextWindowSize(
		ImVec2(1180.f, 900.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Map Tool", &m_bOpen))
	{
		ImGui::End();
		return;
	}

	Render_ModeBar();
	ImGui::Separator();

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

	ImGui::Text(
		"Level: %s",
		isAssetTest ?
			"ASSET_TEST" :
			"Open ASSET_TEST with F2");
	ImGui::SameLine();
	ImGui::Text(
		"| Catalog: %s",
		m_Catalog.Is_Ready() ?
			"READY" :
			"NOT READY");
	ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::Separator();

	ImGui::BeginDisabled(
		!isAssetTest ||
		!m_Catalog.Is_Ready());
	Render_Toolbar();

	const f32_t availableHeight =
		ImGui::GetContentRegionAvail().y;
	const f32_t topPanelHeight =
		(std::max)(
			280.f,
			(std::min)(
				480.f,
				availableHeight * 0.48f));

	if (ImGui::BeginTable(
		"MapEditorColumns",
		3,
		ImGuiTableFlags_Resizable |
			ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn(
			"Asset Palette",
			ImGuiTableColumnFlags_WidthStretch,
			0.38f);
		ImGui::TableSetupColumn(
			"Hierarchy",
			ImGuiTableColumnFlags_WidthStretch,
			0.27f);
		ImGui::TableSetupColumn(
			"Inspector",
			ImGuiTableColumnFlags_WidthStretch,
			0.35f);
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

	ImGui::End();
}
```

기존 `ConsumesWorldMouse()` 전체를 이름까지 다음으로 교체한다.

```cpp
bool_t Client::CMapTool::ConsumesWorldLeftMouse() const
{
	if (!m_bOpen ||
		ETOUI(LEVEL::ASSET_TEST) !=
			CGameInstance::Get().Get_CurrentLevelID())
	{
		return false;
	}

	return
		TOOL_MODE::NAVIGATION == m_eToolMode ||
		PLACEMENT_STATE::ARMED == m_ePlacementState;
}
```

`Handle_LevelTransition()` 전체를 다음으로 교체한다.

```cpp
void Client::CMapTool::Handle_LevelTransition(
	bool_t isAssetTest)
{
	if (isAssetTest == m_bWasInAssetTest)
		return;

	m_bWasInAssetTest = isAssetTest;
	m_ePlacementState = PLACEMENT_STATE::IDLE;
	m_iSelectedPlacementId = 0;
	m_SelectedAssetId.clear();
	m_pAssetTestCamera.reset();
	if (nullptr != m_pAssetPreview)
		m_pAssetPreview->Reset_LevelResources();

	if (!isAssetTest)
	{
		m_Placements.clear();
		m_StaticBatches.clear();
		m_DeployProps.clear();
		m_iNextPlacementId = 1;
		m_bDirty = false;
		m_Status = "Enter AssetTest with F2";
		m_NavigationStatus = "Enter AssetTest with F2";
		m_CameraStatus = "Open ASSET_TEST with F2";
		return;
	}

	if (!m_Catalog.Load_Default())
	{
		m_Status = m_Catalog.Get_Status();
	}
	else
	{
		m_Status = m_Catalog.Get_Status();
		if (Load_Placements())
			Load_DeployProps();
	}

	Load_NavigationDocument();
	Load_CameraPreset();
}
```

`Handle_LevelTransition()` 다음, `Load_NavigationDocument()` 앞에 다음 함수 네 개를 삽입한다.

```cpp
bool_t Client::CMapTool::Load_CameraPreset()
{
	m_CameraPresetPath =
		CCameraPresetDocument::Resolve_DefaultPath();
	if (m_CameraPresetPath.empty())
	{
		m_CameraStatus =
			"Camera preset path is unavailable";
		return false;
	}

	if (!m_CameraDocument.Load(
		m_CameraPresetPath,
		m_CameraStatus))
	{
		return false;
	}

	if (!Find_AssetTestCamera())
		return false;

	if (!Apply_CameraPreset(
		m_CameraDocument.Get_Preset()))
	{
		m_CameraStatus =
			"Camera preset could not be applied";
		return false;
	}

	m_CameraStatus = "Camera loaded";
	return true;
}

bool_t Client::CMapTool::Find_AssetTestCamera()
{
	const shared_ptr<CGameObject> gameObject =
		CGameInstance::Get().Get_GameObject(
			ETOUI(LEVEL::ASSET_TEST),
			TEXT("Layer_Camera"),
			0);
	const shared_ptr<CCamera_Free> camera =
		dynamic_pointer_cast<CCamera_Free>(
			gameObject);
	if (nullptr == camera)
	{
		m_pAssetTestCamera.reset();
		m_CameraStatus =
			"ASSET_TEST camera is unavailable";
		return false;
	}

	m_pAssetTestCamera = camera;
	return true;
}

bool_t Client::CMapTool::Apply_CameraPreset(
	const CAMERA_PRESET& preset)
{
	if (!CCameraPresetDocument::Is_Valid(preset))
		return false;

	const shared_ptr<CCamera_Free> camera =
		m_pAssetTestCamera.lock();
	if (nullptr == camera)
		return false;

	camera->Set_PlayerCameraTuning(
		preset.positionOffset,
		preset.lookOffset,
		preset.fovY,
		preset.followResponse);
	return true;
}

void Client::CMapTool::Render_CameraPanel()
{
	ImGui::TextUnformatted("Player Camera");
	ImGui::Separator();

	const shared_ptr<CCamera_Free> camera =
		m_pAssetTestCamera.lock();
	if (nullptr == camera)
	{
		ImGui::TextUnformatted(
			"ASSET_TEST camera is unavailable.");
		if (ImGui::Button("Find Camera") &&
			Find_AssetTestCamera() &&
			m_CameraDocument.Is_Ready())
		{
			Apply_CameraPreset(
				m_CameraDocument.Get_Preset());
		}
		ImGui::TextWrapped(
			"%s",
			m_CameraStatus.c_str());
		return;
	}

	ImGui::Text(
		"Mode: %s",
		camera->Is_FollowEnabled() ?
			"Follow Player" :
			"Free Camera");
	ImGui::SameLine();
	if (camera->Is_FollowEnabled())
	{
		if (ImGui::Button("Switch to Free Camera"))
		{
			camera->Set_FollowEnabled(false);
			m_CameraStatus = "Free Camera";
		}
	}
	else
	{
		if (ImGui::Button("Follow Player"))
		{
			camera->Set_FollowEnabled(true);
			m_CameraStatus = "Following Player";
		}
	}

	ImGui::Separator();
	ImGui::BeginDisabled(
		!m_CameraDocument.Is_Ready());

	CAMERA_PRESET draft =
		m_CameraDocument.Get_Preset();
	bool_t changed = false;
	changed |= ImGui::DragFloat3(
		"Position Offset",
		&draft.positionOffset.x,
		0.1f,
		-100.f,
		100.f,
		"%.2f");
	changed |= ImGui::DragFloat3(
		"Look Target Offset",
		&draft.lookOffset.x,
		0.05f,
		-20.f,
		20.f,
		"%.2f");
	changed |= ImGui::SliderFloat(
		"FOV Y",
		&draft.fovY,
		20.f,
		120.f,
		"%.1f deg");
	changed |= ImGui::DragFloat(
		"Follow Smoothness",
		&draft.followResponse,
		0.25f,
		0.f,
		60.f,
		"%.2f");

	if (changed)
	{
		std::string status;
		if (m_CameraDocument.Set_Preset(
			draft,
			status))
		{
			Apply_CameraPreset(draft);
		}
		m_CameraStatus = status;
	}

	ImGui::Separator();
	if (ImGui::Button("Save Camera"))
	{
		m_CameraDocument.Save(
			m_CameraPresetPath,
			m_CameraStatus);
	}
	ImGui::SameLine();
	ImGui::TextUnformatted(
		m_CameraDocument.Is_Dirty() ?
			"Unsaved" :
			m_CameraStatus.c_str());

	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(
			"Free Camera: WASD move, hold RMB to rotate.\n"
			"Navigation keeps RMB available and consumes LMB only.\n"
			"Saved file: %s",
			m_CameraPresetPath.string().c_str());
	}

	ImGui::EndDisabled();
}
```

`Render_ModeBar()` 전체를 다음으로 교체한다.

```cpp
void Client::CMapTool::Render_ModeBar()
{
	if (ImGui::RadioButton(
		"Map Assets",
		TOOL_MODE::MAP_ASSETS == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::MAP_ASSETS;
	}

	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Navigation",
		TOOL_MODE::NAVIGATION == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::NAVIGATION;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
	}

	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Camera",
		TOOL_MODE::CAMERA == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::CAMERA;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bNavigationStrokeActive = false;
	}
}
```

### 15. `Client/Private/MainApp.cpp`

`Update()` 전체를 다음으로 교체한다.

```cpp
void CMainApp::Update(f32_t fTimeDelta)
{
#ifdef _DEBUG
	UpdateDebugToolShortcut();

	if (nullptr != m_pImGuiLayer)
		m_pImGuiLayer->BeginFrame();

	const bool_t bMapToolOpen =
		nullptr != m_pMapTool &&
		m_pMapTool->IsOpen();
	const HWND hForegroundWindow =
		GetForegroundWindow();
	const bool_t bExternalToolFocused =
		bMapToolOpen &&
		nullptr != hForegroundWindow &&
		hForegroundWindow != g_hWnd &&
		IsWindowOwnedByCurrentProcess(
			hForegroundWindow);

	const bool_t bImGuiPanelOpen =
		bMapToolOpen ||
		m_bProfilerVisible;
	const bool_t bKeyboardCaptured =
		bImGuiPanelOpen &&
		nullptr != m_pImGuiLayer &&
		(
			m_pImGuiLayer->WantsCaptureKeyboard() ||
			bExternalToolFocused
		);
	const bool_t bMouseCapturedByUi =
		bImGuiPanelOpen &&
		nullptr != m_pImGuiLayer &&
		(
			m_pImGuiLayer->WantsCaptureMouse() ||
			bExternalToolFocused
		);
	const bool_t bWorldLeftMouseConsumed =
		nullptr != m_pMapTool &&
		m_pMapTool->ConsumesWorldLeftMouse();

	CGameInstance::Get().SetInputBlocked(
		bKeyboardCaptured,
		bMouseCapturedByUi);
	CGameInstance::Get().SetMouseButtonBlocked(
		DIM::LB,
		bWorldLeftMouseConsumed);
#endif

	CGameInstance::Get().Update_Engine(fTimeDelta);

#ifdef _DEBUG
	if (nullptr != m_pMapTool)
		m_pMapTool->Update(fTimeDelta);
#endif
}
```

이 변경 후 입력 소유권은 다음처럼 고정된다.

| 상황 | LMB | RMB / mouse delta | WASD |
|---|---|---|---|
| ImGui 위 | 차단 | 차단 | text capture 중 차단 |
| MapTool Navigation world | MapTool | Camera_Free | Camera_Free |
| MapTool Camera world | 기존 gameplay | Camera_Free | Camera_Free |
| MapTool 닫힘 | 기존 gameplay | Camera_Free | Camera_Free |

### Phase 5 — 기본 데이터

### 16. `Client/Bin/DataFiles/Camera/ValtanArena.camera` — 신규 전체 내용

```text
LOSTARK_CAMERA_PRESET 1
POSITION_OFFSET -12 16 -12
LOOK_OFFSET 0 1.2 0
FOV_Y 60
FOLLOW_RESPONSE 18
```

## 프로젝트 등록

### 1. `Client/Default/Client.vcxproj`

기존 `Camera_Free.h` 주변 `ClInclude` ItemGroup에 다음을 추가한다.

```xml
<ClInclude Include="..\Public\CameraPresetDocument.h" />
```

기존 `Camera_Free.cpp` 주변 `ClCompile` ItemGroup에 다음을 추가한다.

```xml
<ClCompile Include="..\Private\CameraPresetDocument.cpp" />
```

DataFiles `None` ItemGroup에 다음을 추가한다.

```xml
<None Include="..\Bin\DataFiles\Camera\ValtanArena.camera" />
```

### 2. `Client/Default/Client.vcxproj.filters`

첫 Filter 정의 ItemGroup에 다음 filter를 추가한다. GUID는 이 문서의 값을 그대로 한 번만 사용한다.

```xml
<Filter Include="96.DataFiles\Camera">
  <UniqueIdentifier>{3D130C42-76E8-4DB7-A418-AB273C34DCAF}</UniqueIdentifier>
</Filter>
```

`ClInclude` ItemGroup에 다음을 추가한다.

```xml
<ClInclude Include="..\Public\CameraPresetDocument.h">
  <Filter>03. Tools\00. Map</Filter>
</ClInclude>
```

`ClCompile` ItemGroup에 다음을 추가한다.

```xml
<ClCompile Include="..\Private\CameraPresetDocument.cpp">
  <Filter>03. Tools\00. Map</Filter>
</ClCompile>
```

`None` ItemGroup에 다음을 추가한다.

```xml
<None Include="..\Bin\DataFiles\Camera\ValtanArena.camera">
  <Filter>96.DataFiles\Camera</Filter>
</None>
```

새 Engine 소스 파일은 없으므로 `Engine.vcxproj`와 `Engine.vcxproj.filters` 항목 추가는 없다. 다만 Engine public header가 바뀌므로 `UpdateLib.bat`은 반드시 실행한다.

## 적용·검증

### 반영 단위

다른 Navigation 세션과 충돌을 피하기 위해 한 번에 전부 붙이지 않고 다음 순서로 닫는다.

1. **Phase 1**
   - Engine button block과 `Get_GameObject()`만 반영한다.
   - Engine Debug/Release를 빌드하고 `UpdateLib.bat` Debug/Release를 실행한다.
2. **Phase 2**
   - `CameraPresetDocument`와 `Camera_Free`를 반영한다.
   - Client compile로 API와 encoding을 확인한다.
3. **Phase 3**
   - `Level_AssetTest` 생성 순서를 바꾼다.
   - MapTool 없이 Follow Player가 되는지 먼저 확인한다.
4. **Phase 4**
   - 현재 Navigation 변경이 들어간 최신 `MapTool`/`MainApp`을 다시 읽고 함수 단위로 합친다.
5. **Phase 5**
   - DataFiles, `.vcxproj`, `.filters`를 등록하고 전체 빌드·실행 검증을 닫는다.

### 정적 확인

```powershell
rg -n "Get_GameObject|SetMouseButtonBlocked" Engine Client
rg -n "ConsumesWorldMouse" Client
rg -n "ConsumesWorldLeftMouse" Client
rg -n "CameraPresetDocument|ValtanArena.camera" Client\Default Client\Public Client\Private
rg -n "CameraPresetDocument|ValtanArena.camera" Client\Default\Client.vcxproj Client\Default\Client.vcxproj.filters
git diff --check
```

기대 결과:

- `ConsumesWorldMouse`는 0건이다.
- `ConsumesWorldLeftMouse`는 선언, 정의, `MainApp` 호출만 존재한다.
- 신규 `.h`, `.cpp`, `.camera`가 `.vcxproj`와 `.filters`에 각각 한 번 등록된다.
- C++ 기존 인코딩은 CP949, Markdown은 UTF-8을 유지한다.

### 빌드

저장소 공통 순서를 그대로 지킨다.

```text
1. Engine x64 Debug
2. Engine x64 Release
3. Engine/UpdateLib.bat Debug
4. Engine/UpdateLib.bat Release
5. Client x64 Debug
6. Client x64 Release
```

`Client.exe`가 출력물을 점유하면 실행 중인 프로세스를 종료한 뒤 Client 링크를 다시 수행한다.

### 실행 검증

#### A. 저장값 없이 기본 Follow

1. `ValtanArena.camera`를 기본 내용으로 둔다.
2. Client Debug 실행 후 F2로 `ASSET_TEST`에 들어간다.
3. MapTool을 열지 않은 상태에서 플레이어를 LMB로 이동시킨다.
4. 카메라가 플레이어 위치를 기준으로 부드럽게 따라오는지 확인한다.
5. 플레이어를 연속 이동시켜도 순간이동·NaN·카메라 소실이 없는지 확인한다.

#### B. Camera 화면 실시간 튜닝

1. F1로 `LostArk Map Tool`을 연다.
2. 상단 `Camera`를 선택한다.
3. `Position Offset X/Y/Z`를 각각 바꾸고 Eye가 즉시 변하는지 확인한다.
4. `Look Target Offset X/Y/Z`를 각각 바꾸고 바라보는 각도가 즉시 변하는지 확인한다.
5. `FOV Y`를 바꾸고 projection이 즉시 변하는지 확인한다.
6. `Follow Smoothness`를 `0`, `18`, `60`으로 바꿔 즉시/기본/빠른 추적 차이를 확인한다.
7. 수정 후 상태가 `Unsaved`가 되는지 확인한다.

#### C. Save·레벨 재진입·프로세스 재실행

1. 구분하기 쉬운 값으로 조절한다.
2. `Save Camera`를 누른다.
3. `ValtanArena.camera.tmp`가 남지 않고 본 파일이 바뀌었는지 확인한다.
4. 다른 레벨로 나갔다가 F2로 재진입한다.
5. 저장한 시점으로 시작하는지 확인한다.
6. Client를 완전히 종료하고 다시 실행한다.
7. F2 진입 직후 같은 시점이 재현되는지 확인한다.

#### D. Free Camera

1. Camera 화면에서 `Switch to Free Camera`를 누른다.
2. RMB를 누르지 않고 mouse를 움직였을 때 카메라가 회전하지 않는지 확인한다.
3. RMB를 누른 채 mouse를 움직여 yaw/pitch가 변하는지 확인한다.
4. WASD로 자유 이동되는지 확인한다.
5. `Follow Player`를 누르면 현재 free 위치에 머물지 않고 플레이어 기준 저장 offset으로 돌아오는지 확인한다.

#### E. Navigation과 동시 사용

1. Camera 화면에서 Free Camera로 전환한다.
2. 상단 `Navigation -> Walkability`로 이동한다.
3. RMB drag로 카메라를 회전한다.
4. LMB로 cell을 피킹한다.
5. 같은 과정에서 플레이어 이동 명령이 발생하지 않는지 확인한다.
6. 노란 cell 차단, 초록 cell 복구, `Save Navigation`이 기존처럼 동작하는지 확인한다.

#### F. ImGui capture

1. Camera의 숫자 입력 위에서 RMB drag와 WASD를 입력한다.
2. 값을 편집하는 동안 카메라가 움직이지 않는지 확인한다.
3. 게임 world로 포인터를 옮기면 입력이 즉시 다시 동작하는지 확인한다.

#### G. 잘못된 저장 파일

1. 정상 `ValtanArena.camera`를 별도 이름으로 복사해 보존한다.
2. version, key, 숫자 중 하나를 잘못된 값으로 바꾼다.
3. 진입 시 crash하지 않고 기본 descriptor로 카메라가 생성되는지 확인한다.
4. MapTool Camera 상태에 `Camera preset is invalid`가 표시되는지 확인한다.
5. 정상 파일을 복구하고 재진입해 저장값이 다시 적용되는지 확인한다.

### 최종 ImGui 형태

```text
+----------------------------------------------------------------------------------+
| LostArk Map Tool                                                                 |
+----------------------------------------------------------------------------------+
| ( ) Map Assets    ( ) Navigation    (*) Camera                                   |
+----------------------------------------------------------------------------------+
| Player Camera                                                                    |
|----------------------------------------------------------------------------------|
| Mode: Follow Player                         [ Switch to Free Camera ]             |
|                                                                                  |
| Position Offset       [ -12.00 ] [  16.00 ] [ -12.00 ]                          |
| Look Target Offset    [   0.00 ] [   1.20 ] [   0.00 ]                          |
| FOV Y                 [================ 60.0 deg ================]                |
| Follow Smoothness     [================ 18.00 ==================]                |
|                                                                                  |
| [ Save Camera ]  Saved                                                     [?]   |
+----------------------------------------------------------------------------------+
```

Free 상태에서는 명령 버튼만 다음처럼 바뀐다.

```text
| Mode: Free Camera                            [ Follow Player ]                    |
```

`?`에 mouse를 올렸을 때만 다음 설명을 보여준다.

```text
Free Camera: WASD move, hold RMB to rotate.
Navigation keeps RMB available and consumes LMB only.
Saved file: .../DataFiles/Camera/ValtanArena.camera
```

이 계획의 종료 기준은 “Camera 화면이 보인다”가 아니다. 다음 다섯 항목이 모두 통과해야 닫는다.

- 플레이어 기준 Follow
- Camera 화면 실시간 튜닝
- Save 후 레벨 재진입·프로세스 재실행 재현
- Free Camera RMB/WASD
- Navigation LMB 피킹과 Free Camera RMB 회전 동시 사용
