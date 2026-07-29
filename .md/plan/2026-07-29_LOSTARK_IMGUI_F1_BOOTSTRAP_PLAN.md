# Session - Debug F1로 Dear ImGui Map Tool 셸을 열고 입력 수명까지 닫는다
좌표: LA-F06 · 축: C4 수명은 선언된다, C8 검증이 병목
관련: `2026-07-28_LOSTARK_COURSE_FOUNDATION_INTEGRATION_PLAN.md`

## 1. 결정 기록

① 문제·제약: ImGui 1.92.8 WIP core와 Win32/DX11 backend 6개 translation unit는 `Engine.dll`에 있으나 context/backend/frame/WndProc 호출은 0개다. Debug F1 한 번으로 보이는 셸과 입력 격리를 만든다.

② 순진한 해법의 실패: `Client` WndProc에서 backend를 직접 초기화하거나 Winters의 `Scene_Editor` 1007줄을 복사하면 Engine/Client 수명 경계가 중복되고, 현재 DirectInput 6개 조회와 raw `GetKeyState` 3개 파일의 입력 누수가 남는다.

③ 메커니즘: Engine `CImGuiLayer`가 context·Win32/DX11 backend·frame·message를 소유하고, Client `CMainApp`이 인스턴스 하나와 `CMapTool` 하나를 소유한다. `BeginFrame → capture 전달 → game update → panel submit → EndFrame → Present` 순서를 고정한다.

④ 대조: Winters는 Scene의 `ImGui()`까지 연결하지만 LostArk의 실행 단위는 `CLevel`이다. 첫 셸에는 새 Scene/Level을 만들지 않고, 배치 월드가 필요한 다음 slice에서 `LEVEL::MAPTOOL`과 `CLevel_MapTool`을 추가한다.

⑤ 대가: 첫 셸은 현재 Logo/Loading/Gameplay 위에 겹쳐지며 맵 전용 카메라·에셋·저장을 갖지 않는다. UI 밖 입력은 계속 게임으로 전달되고, 실제 편집 격리가 필요해지는 시점부터 overlay-only 선택은 틀린다.

⑥ 사용자 확정(2026-07-29): 이번 slice는 수업 프레임워크의 현재 flat 물리 배치를 따른다. 기존 파일을 폴더 이동하지 않고 `Engine/Public|Private/ImGuiLayer.*`, `Client/Public|Private/MapTool.*`를 사용한다. `.vcxproj.filters`는 IDE 분류만 담당하며 Engine wrapper는 `05. Editor`, 외부 원본은 기존 `04. External\imgui`, Client 도구는 기존 `03. Tools\00. Map`에 둔다.

⑦ SDK 확정: Client가 `Engine/Public`나 `Engine/External`을 직접 include하지 않는다. `PrepareEngineSdk`가 flat `ImGuiLayer.h`를 기존 Engine public header 규칙으로 `EngineSDK/inc`에 복사하고, 외부의 `imgui.h`와 `imconfig.h`만 같은 `EngineSDK/inc` 루트에 추가 게시한다. Client include는 `"ImGuiLayer.h"`, `"MapTool.h"`, `"imgui.h"`로 고정한다.

⑧ Client 스타일 확정: `MapTool.h`는 `#include "Client_Defines.h"`, `#include "Engine_Defines.h"`, `NS_BEGIN(Client)`, `NS_END`를 사용한다. `Client_Defines.h` 단독에는 `NS_BEGIN` 정의가 없고 이 저장소에는 `NS_CLIENT` 매크로도 없으므로, 기존 header들이 Engine base header를 통해 받던 namespace 매크로를 standalone 도구 header에서는 `Engine_Defines.h`로 명시한다.

## 1.1 ImGui 도구 계약

도구 유형: **Debug Observer bootstrap**. 아직 asset을 수정·저장하는 Workflow Editor가 아니다.

사용자 작업 계약:

```text
Debug Client 실행 → F1 → ImGui runtime 상태 확인
                    ├─ 창 위 입력: game input 차단
                    ├─ 창 밖 입력: 기존 game input 유지
                    └─ F1 또는 X: 닫기
```

필수 데이터 범위:

| 항목 | View owner | Source owner | Persist owner | Apply owner |
|---|---|---|---|---|
| Map Tool open/closed | `CMapTool` | `CMapTool::m_bOpen` | 없음 | `CMainApp` F1 shortcut |
| keyboard capture | `CMapTool` 상태 표시 | `CImGuiLayer/ImGuiIO` | 없음 | `CGameInstance → CInput_Device` |
| mouse capture | `CMapTool` 상태 표시 | `CImGuiLayer/ImGuiIO` | 없음 | `CGameInstance → CInput_Device` |
| frame rate | `CMapTool` | `ImGuiIO::Framerate` | 없음 | view-only |

Action 예산: 전역 primary action 1개(F1 toggle), 창 action 1개(X close). Save/Reload/Reset/asset action은 0개다.

ASCII wireframe:

```text
+------------------------------------------+
| LostArk Map Tool                      [X] |
+------------------------------------------+
| ImGui Win32/DX11 runtime is active.      |
|------------------------------------------|
| F1: open / close                         |
| Keyboard capture: ON/OFF                 |
| Mouse capture:    ON/OFF                 |
| Frame: 16.67 ms (60.0 FPS)               |
|                                          |
| Empty: asset/picking/inspector = next    |
+------------------------------------------+
```

상태 계약:

- Normal: 창, capture 상태, FPS가 표시된다.
- Empty: 아직 asset 데이터가 없음을 `Next slice` 문장으로 명시한다. 빈 Palette를 그리지 않는다.
- Error: context/Win32/DX11 초기화 실패 시 `OutputDebugStringA` 초기화 실패 문자열을 남기고 `CMainApp::Create`가 실패한다. 세부 stage 분리는 후속 진단성 개선 범위다.
- 최소 화면은 현재 runtime 고정값 1280×720이다. DPI 실측 없음; multi-viewport와 DPI별 font scale은 이번 범위에서 제외한다.
- RESULT 시각 증거 파일명: `.md/build/imgui-f1/2026-07-29_LOSTARK_IMGUI_F1_BOOTSTRAP.png`.

## 1.2 이번 구현에서 이해해야 할 코드

- `Impl`은 **Implementation**의 약자다. `ImGui_ImplWin32_*`는 Win32 플랫폼 입력/backend 구현, `ImGui_ImplDX11_*`는 DirectX 11 렌더 backend 구현이다. C++의 PImpl 설계 패턴을 뜻하는 이름이 아니다.
- `extern ImGui_ImplWin32_WndProcHandler(...)`는 함수를 새로 만드는 코드가 아니라 다른 translation unit(`imgui_impl_win32.cpp`)에 있는 함수의 선언만 현재 `.cpp`에 알린다. 공식 header가 Windows 타입 의존을 피하려고 이 선언을 `#if 0` 안에 두며, 주석에서 호출 측 `.cpp`로 복사하라고 요구한다.
- `ImGuiIO`는 ImGui context의 입력·설정·출력 상태다. `NavEnableKeyboard`는 키보드 UI 탐색을 켜고, `NoMouseCursorChange`는 ImGui가 OS cursor 모양을 바꾸지 못하게 하며, `IniFilename = nullptr`은 bootstrap 단계에서 `imgui.ini` 레이아웃 저장을 끈다.
- 기존 `reinterpret_cast<int32_t*>(&m_tMouseState) + enum index`는 `DIMOUSESTATE` 선두의 `lX/lY/lZ`가 연속이라는 메모리 배치에 기대는 pointer arithmetic이다. 이번에는 같은 값을 `switch`로 `lX/lY/lZ`에서 직접 반환해 의미와 범위를 드러낸다.
- Debug `Engine_Defines.h`는 `new`를 `DBG_NEW` 매크로로 바꾼다. ImGui header는 placement `operator new`를 선언하므로 두 `.cpp`에서 반드시 `imgui.h`를 프레임워크 header보다 먼저 include한다. `ENGINE_EXPORTS`는 compiler definition이라 이 순서와 무관하다.

## 2. 반영해야 하는 코드

### 2-1. C:/Users/user/Desktop/LostArk/Engine/Public/ImGuiLayer.h

현재 부분 구현에는 copy assignment 반환형, `CancleFrame`, `WantCaptureMouse` 오탈자가 있다. 파일 전체를 아래 최종 내용으로 교체한다:

```cpp
#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class ENGINE_DLL CImGuiLayer final
{
public:
	CImGuiLayer() = default;
	~CImGuiLayer();

	CImGuiLayer(const CImGuiLayer&) = delete;
	CImGuiLayer& operator=(const CImGuiLayer&) = delete;

public:
	bool_t Initialize(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void BeginFrame();
	void EndFrame();
	void CancelFrame();
	void Shutdown();

	bool_t WantsCaptureMouse() const;
	bool_t WantsCaptureKeyboard() const;

	static bool_t HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	bool_t m_bInitialized = false;
	bool_t m_bFrameStarted = false;
};

NS_END
```

### 2-2. C:/Users/user/Desktop/LostArk/Engine/Private/ImGuiLayer.cpp

현재 부분 구현의 오탈자와 주석 인코딩을 버리고 파일 전체를 아래 최종 내용으로 교체한다:

```cpp
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "ImGuiLayer.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

CImGuiLayer::~CImGuiLayer()
{
	Shutdown();
}

bool_t CImGuiLayer::Initialize(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (m_bInitialized)
		return true;

	if (nullptr == hWnd || nullptr == pDevice || nullptr == pContext)
		return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	io.IniFilename = nullptr;

	ImGui::StyleColorsDark();

	if (!ImGui_ImplWin32_Init(hWnd))
	{
		ImGui::DestroyContext();
		return false;
	}

	if (!ImGui_ImplDX11_Init(pDevice, pContext))
	{
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		return false;
	}

	m_bInitialized = true;
	return true;
}

void CImGuiLayer::BeginFrame()
{
	if (!m_bInitialized || m_bFrameStarted)
		return;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	m_bFrameStarted = true;
}

void CImGuiLayer::EndFrame()
{
	if (!m_bInitialized || !m_bFrameStarted)
		return;

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_bFrameStarted = false;
}

void CImGuiLayer::CancelFrame()
{
	if (!m_bInitialized || !m_bFrameStarted)
		return;

	ImGui::EndFrame();
	m_bFrameStarted = false;
}

void CImGuiLayer::Shutdown()
{
	if (!m_bInitialized)
		return;

	CancelFrame();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	m_bInitialized = false;
}

bool_t CImGuiLayer::WantsCaptureMouse() const
{
	return m_bInitialized && ImGui::GetIO().WantCaptureMouse;
}

bool_t CImGuiLayer::WantsCaptureKeyboard() const
{
	return m_bInitialized && ImGui::GetIO().WantCaptureKeyboard;
}

bool_t CImGuiLayer::HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (nullptr == ImGui::GetCurrentContext())
		return false;

	return 0 != ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
}
```

### 2-3. C:/Users/user/Desktop/LostArk/Engine/Public/Input_Device.h

`CInput_Device`의 현재 getter 블록:

```cpp
	int8_t	Get_DIKeyState(uint8_t byKeyID)
	{
		return m_byKeyState[byKeyID];
	}

	int8_t	Get_DIMouseState(DIM eMouse)
	{
		return m_tMouseState.rgbButtons[ETOUI(eMouse)];
	}

	// 현재 마우스의 특정 축 좌표를 반환
	int32_t	Get_DIMouseMove(DIMM eMouseState)
	{
		if (m_bMouseBlocked)
			return 0;

		return *((reinterpret_cast<int32_t*>(&m_tMouseState)) + ETOUI(eMouseState));
	}

	void SetInputBlocked(bool bKeyboardBlocked, bool bMouseBlocked)
	{
		m_bKeyboardBlocked = bKeyboardBlocked;
		m_bMouseBlocked = bMouseBlocked;
	}

	bool_t IsKeyboardInputBlocked() const
	{
		return m_bKeyboardBlocked;
	}

	bool IsMouseInputBlocked() const
	{
		return m_bMouseBlocked;
	}
```

아래로 교체:

```cpp
	int8_t Get_DIKeyState(uint8_t byKeyID)
	{
		if (m_bKeyboardBlocked)
			return 0;

		return m_byKeyState[byKeyID];
	}

	int8_t Get_DIMouseState(DIM eMouse)
	{
		if (m_bMouseBlocked)
			return 0;

		return m_tMouseState.rgbButtons[ETOUI(eMouse)];
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

	void SetInputBlocked(bool_t bKeyboardBlocked, bool_t bMouseBlocked)
	{
		m_bKeyboardBlocked = bKeyboardBlocked;
		m_bMouseBlocked = bMouseBlocked;
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

현재 member 블록:

```cpp
	int8_t					m_byKeyState[256] = {};
	DIMOUSESTATE			m_tMouseState = {};
	bool_t m_bKeyboardBlocked = false;
	bool_t m_bMouseBlocked = false;
```

아래로 교체:

```cpp
	int8_t m_byKeyState[256] = {};
	DIMOUSESTATE m_tMouseState = {};
	bool_t m_bKeyboardBlocked = false;
	bool_t m_bMouseBlocked = false;
```

### 2-4. C:/Users/user/Desktop/LostArk/Engine/Public/GameInstance.h

현재 `For.Input_Device` 블록:

```cpp
public: /* For.Input_Device */
	int8_t	Get_DIKeyState(uint8_t byKeyID);
	int8_t	Get_DIMouseState(DIM eMouse);
	int32_t	Get_DIMouseMove(DIMM eMouseState);
	void SetInputBlocked(bool bKeyboardBlocked, bool bMouseBlocked);
	bool_t IsKeyboardInputBlocked() const;
	bool_t IsMouseInputBlocked() const;
```

아래로 교체:

```cpp
public: /* For.Input_Device */
	int8_t Get_DIKeyState(uint8_t byKeyID);
	int8_t Get_DIMouseState(DIM eMouse);
	int32_t Get_DIMouseMove(DIMM eMouseState);
	void SetInputBlocked(bool_t bKeyboardBlocked, bool_t bMouseBlocked);
	bool_t IsKeyboardInputBlocked() const;
	bool_t IsMouseInputBlocked() const;
```

### 2-5. C:/Users/user/Desktop/LostArk/Engine/Private/GameInstance.cpp

현재 부분 구현 코드:

```cpp
int32_t CGameInstance::Get_DIMouseMove(DIMM eMouseState)
{
	return m_pInput_Device->Get_DIMouseMove(eMouseState);
}

void CGameInstance::SetInputBlocked(bool_t bKeyboardBlocked, bool_t bMouseBlocked)
{
	if (nullptr != m_pInput_Device)
		m_pInput_Device->SetInputBlocked(bKeyboardBlocked, bMouseBlocked);
}

bool CGameInstance::IsKeyboardInputBlocked() const
{
	return nullptr != m_pInput_Device && m_pInput_Device->IsKeyboardInputBlocked();
}

bool CGameInstance::IsMouseInputBlocked() const
{
	return nullptr != m_pInput_Device && m_pInput_Device->IsMouseInputBlocked();
}
```

`SetInputBlocked`부터 세 method를 아래로 교체:

```cpp
void CGameInstance::SetInputBlocked(bool_t bKeyboardBlocked, bool_t bMouseBlocked)
{
	if (nullptr != m_pInput_Device)
		m_pInput_Device->SetInputBlocked(bKeyboardBlocked, bMouseBlocked);
}

bool_t CGameInstance::IsKeyboardInputBlocked() const
{
	return nullptr != m_pInput_Device && m_pInput_Device->IsKeyboardInputBlocked();
}

bool_t CGameInstance::IsMouseInputBlocked() const
{
	return nullptr != m_pInput_Device && m_pInput_Device->IsMouseInputBlocked();
}
```

### 2-6. C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h

현재 부분 구현은 `Client_Defines.h`만 include해 `NS_BEGIN` 정의가 도달하지 않는다. 파일 전체를 아래로 교체:

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CMapTool final
{
public:
	void Toggle();
	void Render();

	bool IsOpen() const;

private:
	bool m_bOpen = false;
};

NS_END
```

### 2-7. C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp

현재 부분 구현은 include 두 줄만 있고 SDK 경로와 method body가 없다. 파일 전체를 아래로 교체:

```cpp
#include "imgui.h"

#include "MapTool.h"

void Client::CMapTool::Toggle()
{
	m_bOpen = !m_bOpen;
}

void Client::CMapTool::Render()
{
	if (!m_bOpen)
		return;

	ImGui::SetNextWindowSize(ImVec2(420.f, 180.f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("LostArk Map Tool", &m_bOpen))
	{
		ImGui::End();
		return;
	}

	ImGui::TextUnformatted("ImGui Win32/DX11 runtime is active.");
	ImGui::Separator();
	ImGui::TextUnformatted("F1: open / close");
	ImGui::TextUnformatted("Next slice: asset palette, picking, transform inspector");
	ImGui::Text("Keyboard capture: %s", ImGui::GetIO().WantCaptureKeyboard ? "ON" : "OFF");
	ImGui::Text("Mouse capture: %s", ImGui::GetIO().WantCaptureMouse ? "ON" : "OFF");

	const float fFrameRate = ImGui::GetIO().Framerate;
	if (fFrameRate > 0.f)
		ImGui::Text("Frame: %.3f ms (%.1f FPS)", 1000.f / fFrameRate, fFrameRate);
	else
		ImGui::TextUnformatted("Frame: warming up");

	ImGui::End();
}

bool Client::CMapTool::IsOpen() const
{
	return m_bOpen;
}
```

### 2-8~9. flat 경로 유지 결정

`Client/Public/MapTool.h`, `Client/Private/MapTool.cpp`가 이번 slice의 정본이다. 기존 placeholder를 삭제하거나 `Tools/Map`으로 이동하지 않고 §2-6~7의 전체 내용으로 완성한다.

### 2-10. C:/Users/user/Desktop/LostArk/Client/Public/MainApp.h

현재 부분 구현의 namespace/forward declaration 블록:

```cpp
#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CImGuiLayer;
NS_END

NS_BEGIN(Client)

class CMapTool;
```

최종 target과 동일하므로 유지:

```cpp
#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CImGuiLayer;
NS_END

NS_BEGIN(Client)

class CMapTool;
```

현재 부분 구현의 device/context/debug member 블록:

```cpp
private:
	ComPtr<ID3D11Device>			m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>		m_pContext = { nullptr };

#ifdef _DEBUG
	std::unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
	std::unique_ptr<CMapTool> m_pMapTool = { nullptr };
	bool m_bF1Down = false;
#endif
```

최종 target과 동일하므로 유지(기존 정렬도 그대로 둔다):

```cpp
private:
	ComPtr<ID3D11Device>			m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>		m_pContext = { nullptr };

#ifdef _DEBUG
	std::unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
	std::unique_ptr<CMapTool> m_pMapTool = { nullptr };
	bool m_bF1Down = false;
#endif
```

현재 부분 구현의 private helper 블록:

```cpp
private:
	HRESULT Ready_Gara();
	HRESULT Ready_Fonts();
	HRESULT Ready_Prototype_For_Static();
	HRESULT Start_Level(LEVEL eStartLevelID);

#ifdef _DEBUG
	HRESULT ReadyDebugTools();
	void UpdateDebugToolShortcut();
#endif
```

최종 target과 동일하므로 유지:

```cpp
private:
	HRESULT Ready_Gara();
	HRESULT Ready_Fonts();
	HRESULT Ready_Prototype_For_Static();
	HRESULT Start_Level(LEVEL eStartLevelID);

#ifdef _DEBUG
	HRESULT ReadyDebugTools();
	void UpdateDebugToolShortcut();
#endif
```

### 2-11. C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp

현재 include 블록:

```cpp
#include "MainApp.h"
#include "GameInstance.h"

#include "Level_Loading.h"
```

아래로 교체:

```cpp
#include "MainApp.h"
#include "GameInstance.h"

#include "Level_Loading.h"

#ifdef _DEBUG
#include "ImGuiLayer.h"
#include "MapTool.h"
#endif
```

`CMainApp::Initialize`의 현재 Engine 초기화 직후 코드:

```cpp
    if (FAILED(CGameInstance::Get().Initialize_Engine(EngineDesc, m_pDevice, m_pContext)))
        return E_FAIL;

    if (FAILED(Ready_Gara()))
        return E_FAIL;
```

아래로 교체:

```cpp
    if (FAILED(CGameInstance::Get().Initialize_Engine(EngineDesc, m_pDevice, m_pContext)))
        return E_FAIL;

#ifdef _DEBUG
    if (FAILED(ReadyDebugTools()))
        return E_FAIL;
#endif

    if (FAILED(Ready_Gara()))
        return E_FAIL;
```

현재 `CMainApp::Update` 전체:

```cpp
void CMainApp::Update(f32_t fTimeDelta)
{
    CGameInstance::Get().Update_Engine(fTimeDelta);
}
```

아래로 교체:

```cpp
void CMainApp::Update(f32_t fTimeDelta)
{
#ifdef _DEBUG
    UpdateDebugToolShortcut();

    if (nullptr != m_pImGuiLayer)
        m_pImGuiLayer->BeginFrame();

    const bool bMapToolOpen = nullptr != m_pMapTool && m_pMapTool->IsOpen();
    const bool bKeyboardCaptured = bMapToolOpen &&
        nullptr != m_pImGuiLayer && m_pImGuiLayer->WantsCaptureKeyboard();
    const bool bMouseCaptured = bMapToolOpen &&
        nullptr != m_pImGuiLayer && m_pImGuiLayer->WantsCaptureMouse();

    CGameInstance::Get().SetInputBlocked(bKeyboardCaptured, bMouseCaptured);
#endif

    CGameInstance::Get().Update_Engine(fTimeDelta);
}
```

`CMainApp::Render`의 현재 game render 실패 블록:

```cpp
    if (FAILED(CGameInstance::Get().Render()))
        return E_FAIL;
```

아래로 교체:

```cpp
    if (FAILED(CGameInstance::Get().Render()))
    {
#ifdef _DEBUG
        if (nullptr != m_pImGuiLayer)
            m_pImGuiLayer->CancelFrame();
#endif
        return E_FAIL;
    }
```

현재 `CMainApp::Render`의 `Render_Begin` 실패 블록도 동일하게 frame을 닫도록 교체한다.

기존 코드:

```cpp
    if (FAILED(CGameInstance::Get().Render_Begin(&vClearColor)))
        return E_FAIL;
```

아래로 교체:

```cpp
    if (FAILED(CGameInstance::Get().Render_Begin(&vClearColor)))
    {
#ifdef _DEBUG
        if (nullptr != m_pImGuiLayer)
            m_pImGuiLayer->CancelFrame();
#endif
        return E_FAIL;
    }
```

현재 `Render_End` 블록:

```cpp
    if (FAILED(CGameInstance::Get().Render_End()))
        return E_FAIL;

    return S_OK;
```

아래로 교체:

```cpp

#ifdef _DEBUG
    if (nullptr != m_pImGuiLayer)
    {
        if (nullptr != m_pMapTool)
            m_pMapTool->Render();
        m_pImGuiLayer->EndFrame();
    }
#endif

    if (FAILED(CGameInstance::Get().Render_End()))
        return E_FAIL;

    return S_OK;
```

`CMainApp::Start_Level` 함수 바로 아래에 추가:

```cpp
#ifdef _DEBUG
HRESULT CMainApp::ReadyDebugTools()
{
    m_pImGuiLayer = std::make_unique<CImGuiLayer>();
    if (!m_pImGuiLayer->Initialize(g_hWnd, m_pDevice.Get(), m_pContext.Get()))
    {
        OutputDebugStringA("[ImGui] Failed to initialize Win32/DX11 runtime.\n");
        return E_FAIL;
    }

    m_pMapTool = std::make_unique<CMapTool>();
    return S_OK;
}

void CMainApp::UpdateDebugToolShortcut()
{
    const bool bWindowFocused = GetForegroundWindow() == g_hWnd;
    const bool bF1Down = bWindowFocused &&
        0 != (GetAsyncKeyState(VK_F1) & 0x8000);

    if (bF1Down && !m_bF1Down && nullptr != m_pMapTool)
        m_pMapTool->Toggle();

    m_bF1Down = bF1Down;
}
#endif
```

현재 `CMainApp::Free` 전체:

```cpp
void CMainApp::Free()
{
    CGameInstance::Get().Release_Engine();

}
```

아래로 교체:

```cpp
void CMainApp::Free()
{
#ifdef _DEBUG
    CGameInstance::Get().SetInputBlocked(false, false);

    m_pMapTool.reset();

    if (nullptr != m_pImGuiLayer)
        m_pImGuiLayer->Shutdown();
    m_pImGuiLayer.reset();
#endif

    CGameInstance::Get().Release_Engine();
}
```

### 2-12. C:/Users/user/Desktop/LostArk/Client/Default/Client.cpp

현재 include 블록:

```cpp
#include "Client_Defines.h"
#include "MainApp.h"
#include "GameInstance.h"
```

아래로 교체:

```cpp
#include "Client_Defines.h"
#include "MainApp.h"
#include "GameInstance.h"

#ifdef _DEBUG
#include "ImGuiLayer.h"
#endif
```

현재 `WndProc` 시작:

```cpp
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
```

아래로 교체:

```cpp
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef _DEBUG
    if (CImGuiLayer::HandleWindowMessage(hWnd, message, wParam, lParam))
        return 1;
#endif

    switch (message)
```

### 2-13. C:/Users/user/Desktop/LostArk/Client/Private/Level_Logo.cpp

현재 코드:

```cpp
	if (GetKeyState(VK_SPACE) & 0x8000)
```

아래로 교체:

```cpp
	if (!CGameInstance::Get().IsKeyboardInputBlocked() &&
		(GetKeyState(VK_SPACE) & 0x8000))
```

### 2-14. C:/Users/user/Desktop/LostArk/Client/Private/Level_Loading.cpp

현재 코드:

```cpp
	if (GetKeyState(VK_RETURN) & 0x8000 && 
		true == m_pLoader->Finished())
```

아래로 교체:

```cpp
	if (!CGameInstance::Get().IsKeyboardInputBlocked() &&
		(GetKeyState(VK_RETURN) & 0x8000) &&
		true == m_pLoader->Finished())
```

### 2-15. C:/Users/user/Desktop/LostArk/Client/Private/Player.cpp

`CPlayer::Update`의 현재 첫 키 입력부터 `VK_UP` 분기 직전까지:

```cpp
	if (GetKeyState(VK_DOWN) & 0x8000)
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	if (GetKeyState(VK_LEFT) & 0x8000)
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * -1.f);
	}

	if (GetKeyState(VK_RIGHT) & 0x8000)
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);
	}


	if (GetKeyState(VK_UP) & 0x8000)
```

아래로 교체:

```cpp
	const bool bKeyboardBlocked = CGameInstance::Get().IsKeyboardInputBlocked();

	if (!bKeyboardBlocked && (GetKeyState(VK_DOWN) & 0x8000))
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	if (!bKeyboardBlocked && (GetKeyState(VK_LEFT) & 0x8000))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * -1.f);
	}

	if (!bKeyboardBlocked && (GetKeyState(VK_RIGHT) & 0x8000))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);
	}

	if (!bKeyboardBlocked && (GetKeyState(VK_UP) & 0x8000))
```

현재 mouse picking 조건:

```cpp
	if (GetKeyState(VK_LBUTTON) & 0x8000 && 
		true == CGameInstance::Get().Picking(vPickPos))
```

아래로 교체:

```cpp
	if (!CGameInstance::Get().IsMouseInputBlocked() &&
		(GetKeyState(VK_LBUTTON) & 0x8000) &&
		true == CGameInstance::Get().Picking(vPickPos))
```

### 2-16. C:/Users/user/Desktop/LostArk/Engine/Default/Engine.vcxproj

현재 부분 구현에 header 등록이 이미 존재한다. 다음 줄을 유지한다:

```xml
    <ClInclude Include="..\Public\ImGuiLayer.h" />
```

현재 부분 구현에 source 등록도 이미 존재한다. 다음 줄을 유지한다:

```xml
    <ClCompile Include="..\Private\ImGuiLayer.cpp" />
```

### 2-17. C:/Users/user/Desktop/LostArk/Engine/Default/Engine.vcxproj.filters

현재 부분 구현은 External을 `05`, Editor를 `04`로 바꾼 상태다. 기존 External 번호를 보존하고 Editor를 뒤에 추가하도록 아래 네 선언을 한 블록으로 교체한다. GUID는 현재 값을 그대로 유지한다:

```xml
    <Filter Include="05. External">
      <UniqueIdentifier>{9ce0fc9f-8fce-4bbf-b4c8-fe6a0cbecbfe}</UniqueIdentifier>
    </Filter>
    <Filter Include="05. External\imgui">
      <UniqueIdentifier>{feb70539-97ca-4495-bd20-7c4169885c23}</UniqueIdentifier>
    </Filter>
    <Filter Include="05. External\imgui\backends">
      <UniqueIdentifier>{fea74924-d492-4755-875a-70eca8cc6111}</UniqueIdentifier>
    </Filter>
    <Filter Include="04. Editor">
      <UniqueIdentifier>{577ee151-b3da-4519-8e98-85d6dedf44de}</UniqueIdentifier>
    </Filter>
```

아래로 교체:

```xml
    <Filter Include="04. External">
      <UniqueIdentifier>{9ce0fc9f-8fce-4bbf-b4c8-fe6a0cbecbfe}</UniqueIdentifier>
    </Filter>
    <Filter Include="04. External\imgui">
      <UniqueIdentifier>{feb70539-97ca-4495-bd20-7c4169885c23}</UniqueIdentifier>
    </Filter>
    <Filter Include="04. External\imgui\backends">
      <UniqueIdentifier>{fea74924-d492-4755-875a-70eca8cc6111}</UniqueIdentifier>
    </Filter>
    <Filter Include="05. Editor">
      <UniqueIdentifier>{577ee151-b3da-4519-8e98-85d6dedf44de}</UniqueIdentifier>
    </Filter>
```

현재 ImGui file mapping에서 `<Filter>04. Editor</Filter>`는 `05. Editor`로, 모든 `<Filter>05. External...`은 같은 suffix의 `04. External...`로 교체한다. 실제 물리 경로와 등록 줄은 바꾸지 않는다:

```xml
    <ClInclude Include="..\Public\ImGuiLayer.h">
      <Filter>05. Editor</Filter>
    </ClInclude>
    <ClCompile Include="..\External\imgui\backends\imgui_impl_win32.cpp">
      <Filter>04. External\imgui\backends</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ImGuiLayer.cpp">
      <Filter>05. Editor</Filter>
    </ClCompile>
```

`Sound_Manager`의 현재 unfiltered 변경과 `Engine_Defines.h` whitespace는 이번 ImGui slice 소유가 아니므로 보존한다.

### 2-18. C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj

현재 flat 등록은 유지한다:

```xml
    <ClInclude Include="..\Public\MapTool.h" />
```

현재 flat source 등록도 유지한다:

```xml
    <ClCompile Include="..\Private\MapTool.cpp" />
```

진행 중인 Assimp 수정이 추가한 `PrepareEngineSdk`의 현재 ItemGroup:

```xml
    <ItemGroup>
      <EnginePublicHeaders Include="$(EngineRoot)Public\**\*.*" />
      <EngineSdkLibraries Include="$(EngineRoot)Bin\*.lib;$(EngineRoot)ThirdPartyLib\*.lib" />
      <EngineShaderSources Include="$(EngineRoot)Bin\ShaderFiles\*.*" />
    </ItemGroup>
```

아래로 교체:

```xml
    <ItemGroup>
      <EnginePublicHeaders Include="$(EngineRoot)Public\**\*.*" />
      <ImGuiPublicHeaders Include="$(EngineRoot)External\imgui\imgui.h;$(EngineRoot)External\imgui\imconfig.h" />
      <EngineSdkLibraries Include="$(EngineRoot)Bin\*.lib;$(EngineRoot)ThirdPartyLib\*.lib" />
      <EngineShaderSources Include="$(EngineRoot)Bin\ShaderFiles\*.*" />
    </ItemGroup>
```

현재 directory 생성 코드는 그대로 유지한다:

```xml
    <MakeDir Directories="$(EngineSdkRoot)inc;$(EngineSdkRoot)lib;$(EngineSdkRoot)hlsl;$(ProjectDir)..\Bin\ShaderFiles" />
```

기존 Engine public header copy:

```xml
    <Copy SourceFiles="@(EnginePublicHeaders)"
          DestinationFiles="@(EnginePublicHeaders->'$(EngineSdkRoot)inc\%(RecursiveDir)%(Filename)%(Extension)')"
          SkipUnchangedFiles="true" />
```

아래에 추가:

```xml
    <Copy SourceFiles="@(ImGuiPublicHeaders)"
          DestinationFolder="$(EngineSdkRoot)inc"
          SkipUnchangedFiles="true" />
```

### 2-19. C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters

현재 flat source 등록은 그대로 유지한다:

```xml
    <ClCompile Include="..\Private\MapTool.cpp">
      <Filter>03. Tools\00. Map</Filter>
    </ClCompile>
```

현재 flat header 등록도 그대로 유지한다:

```xml
    <ClInclude Include="..\Public\MapTool.h">
      <Filter>03. Tools\00. Map</Filter>
    </ClInclude>
```

## 3. 검증

예측:

- Debug x64 실행 후 Logo·Loading·Gameplay 어느 상태에서든 F1을 한 번 누르면 `LostArk Map Tool` 창이 표시되고, 다시 누르거나 창의 X를 누르면 닫힌다.
- ImGui 창에 keyboard focus/active가 있고 화면의 `Keyboard capture: ON` 또는 `Mouse capture: ON`이 확인된 상태에서는 대응하는 DirectInput getter가 0을 반환하고, Logo Space·Loading Enter·Player 방향키/클릭도 진행되지 않는다. capture가 꺼진 UI 밖에서는 기존 입력이 유지된다.
- frame 순서는 ImGui BeginFrame → capture 전달 → game update/render → panel submit → ImGui draw → Present이며, 종료는 MapTool → DX11 backend → Win32 backend → context → Engine device 순서다.
- Release x64에서는 F1·MapTool·ImGui runtime 초기화가 발생하지 않는다.
- 게이트가 잡아야 할 회귀: 중복 context/backend 초기화 assertion, Begin/End 불균형, ImGui 조작 중 카메라/플레이어 이동, 종료 시 D3D11 live object 증가.

검증 명령:

```powershell
$ErrorActionPreference = 'Stop'

$msbuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
    -latest -products * -requires Microsoft.Component.MSBuild `
    -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
if ([string]::IsNullOrWhiteSpace($msbuild)) { throw "MSBuild.exe was not found" }

& $msbuild "C:\Users\user\Desktop\LostArk\Framework.sln" `
    /t:Build /p:Configuration=Release /p:Platform=x64 /m:1 /nologo

& $msbuild "C:\Users\user\Desktop\LostArk\Framework.sln" `
    /t:Build /p:Configuration=Debug /p:Platform=x64 /m:1 /nologo
```

수동 스모크:

```text
1. C:/Users/user/Desktop/LostArk/Client/Bin/Client.exe 실행.
2. Logo에서 F1 → 창 표시, 창을 클릭해 `Keyboard capture: ON`을 확인한 뒤 Space 입력이 Level 전환을 만들지 않는지 확인.
3. 창을 닫고 Space → Loading 진입, F1 → 창을 클릭해 `Keyboard capture: ON` 확인 후 Enter 차단 확인.
4. Gameplay에서 F1 → 창의 keyboard focus/active 상태와 `Mouse capture: ON` 상태에서 WASD/마우스/방향키/좌클릭이 카메라·플레이어를 움직이지 않는지 확인.
5. UI 밖에서 기존 카메라·플레이어 입력이 다시 동작하는지 확인.
6. F1 토글 20회 후 종료하고 assertion/crash 및 D3D11 live object 증가가 없는지 확인.
```

미검증:

- 이 문서는 코드 preview 단계라 빌드·실행을 수행하지 않았다.
- ImGui를 DLL 경계에서 export/import하는 현재 `imconfig.h` 계약의 장시간 안정성은 이번 smoke 범위를 넘는다.
- Map asset palette, world picking, transform inspector, save/reload, dedicated MapTool Level은 후속 slice다.

선행 조건:

- 다른 세션의 Assimp runtime 배포 수정이 끝나 `Client/Bin/assimp-vc143-mtd.dll`이 Rebuild 후 유지되어야 실행 smoke를 시작한다.
- `EngineSDK/inc`는 직접 편집하지 않는다. §2-18의 `PrepareEngineSdk`가 flat `ImGuiLayer.h`, `imgui.h`, `imconfig.h`를 매 빌드 게시한다.
- 구현 시작 직전 flat 정본 두 파일과 네 project/filter 파일이 계획에서 인용한 현재 내용인지 다시 확인한다. 동시 세션 변경이 있으면 해당 부분을 덮지 않고 계획을 재조정한다.

## 4. Scene/Level 경계

- 이번 F1 bootstrap에는 Scene도 Level도 새로 등록하지 않는다. `CMainApp`이 전역 Debug overlay를 소유하므로 모든 현재 Level에서 수명·WndProc·입력을 한 번에 검증할 수 있다.
- 실제 맵 배치 단계에서는 Winters의 `CScene_Editor`를 복사하지 않는다. LostArk의 기존 추상화에 맞춰 `LEVEL::MAPTOOL`, `CLevel_MapTool`, Loader의 MAPTOOL prototype branch를 Client에 추가한다.
- 그때 `CLevel_MapTool`은 맵 카메라·지형·배치 대상 월드만 소유하고, ImGui panel/document/catalog은 이번 flat 단계에서는 `Client/MapTool.*`에 남긴다. 도구 수가 실제로 늘어날 때 `Client/Tools/*` 물리 이관을 별도 slice로 수행한다.

## 5. 서브 에이전트 비평

### 1차 비평 — `/root/imgui_f1_plan_critique`

판정: FAIL, P0=0, P1=5, P2=2.

| 지적 | 처분 | 근거와 반영 |
|---|---|---|
| Engine project 등록과 ImGui public header 게시가 없어 compile 불가 | 수용 | §2-16~19에 Engine/Client project/filter additive XML과 `PrepareEngineSdk` header 게시를 추가 |
| capture를 BeginFrame 전 읽어 첫 입력이 한 frame 누출 | 수용 | `UpdateDebugToolShortcut → BeginFrame → capture → Update_Engine`, Render에서 submit/EndFrame, 실패 시 CancelFrame으로 교정 |
| 부분 초기화 실패의 `Free → SetInputBlocked` null 역참조 | 수용 | `CGameInstance` input block wrapper를 null-safe하게 교정 |
| root MapTool placeholder가 물리 source 정본·사용자 요구와 충돌 | 수용 | `Client/Public|Private/Tools/Map` 새 파일로 이동하고 root placeholder 삭제·project 경로 교체 |
| ImGui 도구 설계 가이드의 계약·wireframe·state·owner·capture 증거 누락 | 수용 | §1.1에 Debug Observer 계약, 데이터/owner, action 예산, wireframe, 상태, 해상도, RESULT 캡처 경로 추가 |
| 상위 F06에서 제외한 docking을 활성화 | 수용 | `ImGuiConfigFlags_DockingEnable` 제거 |
| WndProc handler 수동 extern 불필요, FPS 0 방어 필요 | 부분 수용 | FPS 0 방어는 반영. extern 삭제는 기각: 현재 `imgui_impl_win32.h:35~37` 선언이 `#if 0` 안에 있고 헤더 주석도 호출 측 복사 선언을 요구함 |

### 2차 비평 — `/root/imgui_f1_plan_critique`

판정: PASS, P0=0, P1=0, P2=3.

| 지적 | 처분 | 근거와 반영 |
|---|---|---|
| Error 계약은 stage별 원인을 약속하지만 구현은 단일 로그 | 수용 | Error 계약을 실제 구현 수준인 초기화 실패 문자열로 교정하고, stage 분리는 후속 진단성 개선으로 명시 |
| 수동 테스트의 `창 위 키보드` 표현이 hover와 focus를 혼동 | 수용 | 창 클릭 후 `Keyboard capture: ON`, keyboard focus/active, `Mouse capture: ON`을 확인하도록 절차 교정 |
| Release 비활성화 수용 기준에 대응하는 자동 build 명령 누락 | 수용 | Release x64를 먼저 빌드하고 Debug x64를 마지막에 빌드해 공용 `Client/Bin`의 수동 smoke 대상을 Debug로 보존 |

### 최종 델타 비평 — `/root/imgui_f1_plan_critique`

1차 판정: FAIL, P0=0, P1=1. Debug 다음 Release를 빌드하면 공용 `Client/Bin`의 `Client.exe`가 Release로 덮여 F1 수동 smoke 대상이 잘못된다는 지적을 수용했다. 검증 순서를 `Release → Debug → 수동 smoke`로 교정했다.

재비평 판정: PASS, P0=0, P1=0. `Release x64 → Debug x64` 순서와 이후 F1 수동 smoke 대상이 일치한다.

위 `Tools/Map` 처분은 이후 사용자가 수업 프레임워크의 flat 물리 배치를 우선하기로 확정해 §1의 ⑥~⑧ 및 §2-6~19로 대체됐다. 역사적 비평 기록일 뿐 현재 구현 경로가 아니다.

### 사용자 flat 결정 반영 재비평 — `/root/imgui_f1_plan_critique`

1차 판정: FAIL, P0=0, P1=1, P2=1.

| 지적 | 처분 | 근거와 반영 |
|---|---|---|
| 계획의 원본 anchor가 이미 존재하는 부분 구현과 불일치 | 수용 | §2-1~18을 현재 partial 기준으로 교정. Engine project 등록은 유지, Engine filter는 현재 `05.External/04.Editor`에서 `04.External/05.Editor`로 정확히 교체, Input/GameInstance/MainApp/MapTool은 현재 partial에서 최종 block으로 교체하도록 명시 |
| 예측의 `UI 위 키보드`가 hover와 focus/capture를 혼동 | 수용 | §3 예측도 keyboard focus/active와 화면 capture ON을 기준으로 교정 |
| standalone `MapTool.h`에서 `Client_Defines.h`만으로 `NS_BEGIN` 사용 | 주 에이전트 추가 발견·수용 | `Engine_Defines.h`를 함께 include한다. 이 저장소에는 `NS_CLIENT`가 없으므로 새 매크로를 만들지 않음 |

2차 판정: FAIL, P0=0, P1=1, P2=1.

| 지적 | 처분 | 근거와 반영 |
|---|---|---|
| Debug `#define new DBG_NEW` 이후 `imgui.h`를 include하면 placement `new` 구문 파손 | 수용 | `ImGuiLayer.cpp`, `MapTool.cpp` 모두 `imgui.h`를 framework header보다 먼저 include하도록 §2-2, §2-7 교정 |
| MainApp member target block이 유지라고 쓰고 정렬을 변경 | 수용 | 현재 tab 정렬과 target block을 동일하게 교정 |

3차 재비평 판정: PASS, P0=0, P1=0. Debug `new` macro include 순서, flat SDK 게시, partial anchor, filter rename, WndProc/frame/input/cleanup, Release→Debug 검증 계약이 닫혔다.

최종 critique gate: accepted/held P0=0, P1=0. 구현 진입 가능.

## 6. 2026-07-29 후속 수정 — 외부 ImGui 창과 Engine DLL F11

### 6-1. 목표와 성공 기준

이번 후속 범위는 기존 F1 bootstrap을 유지하면서 다음 두 문제만 해결한다.

1. `LostArk Map Tool` 창을 Client 창 밖으로 끌면 별도 Win32 창으로 분리되고, 다시 Client 메인 viewport 위로 가져오면 자동 병합된다.
2. Debug x64 F5에서 `CLevel_GamePlay::Update`의 `__super::Update(fTimeDelta)`를 F11로 진입할 때 현재 실행 중인 `Client/Bin/Engine.dll`과 정확히 일치하는 `Engine.pdb`가 로드되고, 비어 있던 `CLevel::Update` 안의 Debug 시퀀스 포인트에서 멈춘다.

범위 제한:

- DockingSpace와 `ImGuiConfigFlags_DockingEnable`은 추가하지 않는다. 외부 창 분리는 multi-viewport만으로 충분하다.
- `io.ConfigViewportsNoAutoMerge`는 기본값 `false`를 유지한다. 따라서 밖으로 분리된 창은 메인 viewport로 돌아오면 자동 병합된다.
- `io.IniFilename = nullptr`도 유지한다. 이번 slice에서는 외부 창 위치를 재실행 후 저장하지 않는다.
- Engine이 Client 출력 폴더를 직접 수정하는 post-build는 만들지 않는다. 최종 실행물 배포 소유자는 기존대로 Client project가 가진다.

### 6-2. 현재 코드 증거와 원인

- 로컬 ImGui 1.92.8 WIP Win32/DX11 backend는 각각 `ImGuiBackendFlags_PlatformHasViewports`, `ImGuiBackendFlags_RendererHasViewports`를 제공한다.
- 현재 `CImGuiLayer::Initialize`에는 `ImGuiConfigFlags_ViewportsEnable`이 없고, `EndFrame`에도 `UpdatePlatformWindows`/`RenderPlatformWindowsDefault`가 없다.
- Engine Debug compile은 `/ZI /JMC /Od`이므로 디버그 정보·Just My Code·최적화 해제 설정은 정상이다.
- 새 `Engine/Bin/Engine.dll`과 F5가 읽는 `Client/Bin/Engine.dll`의 timestamp가 달랐고, `Client/Bin/Engine.pdb`는 존재하지 않았다.
- `DeployClientRuntimeDependencies`는 `Engine.dll`만 복사하며 `Engine.pdb`를 복사하지 않는다. Engine project만 다시 빌드하면 Client project의 배포 target은 실행되지 않는다.
- `CLevel::Update` 본문은 완전히 비어 있다. 일치하는 PDB가 있어도 정지 가능한 source sequence point가 없어서 F11이 들어갔다가 즉시 반환된 것처럼 보일 수 있다.

### 6-3. C:/Users/user/Desktop/LostArk/Engine/Private/ImGuiLayer.cpp

현재 설정:

```cpp
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	io.IniFilename = nullptr;
```

아래로 교체:

```cpp
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = nullptr;
```

현재 `EndFrame` draw block:

```cpp
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_bFrameStarted = false;
```

아래로 교체:

```cpp
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	m_bFrameStarted = false;
```

`UpdatePlatformWindows`는 ImGui viewport와 Win32 보조 HWND의 생성·이동·파괴를 동기화한다. `RenderPlatformWindowsDefault`는 DX11 backend가 보조 viewport별 swap chain에 draw data를 렌더링하고 present하게 한다. 같은 ImGui window가 viewport 소유권만 바꾸며 복제되지는 않는다.

### 6-4. C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj

현재 Engine runtime 검사 아래:

```xml
    <Error Condition="!Exists('$(EngineRoot)Bin\Engine.dll')"
           Text="Engine runtime was not built: $(EngineRoot)Bin\Engine.dll" />
```

아래에 추가:

```xml
    <Error Condition="'$(Configuration)'=='Debug' and !Exists('$(EngineRoot)Bin\Engine.pdb')"
           Text="Engine debug symbols were not built: $(EngineRoot)Bin\Engine.pdb" />
```

현재 runtime item:

```xml
      <ClientRuntimeDependencies Include="$(EngineRoot)Bin\Engine.dll" />
      <ClientRuntimeDependencies Include="$(EngineRoot)ThirdPartyLib\FMOD\Bin\fmod.dll" />
```

아래로 교체:

```xml
      <ClientRuntimeDependencies Include="$(EngineRoot)Bin\Engine.dll" />
      <ClientRuntimeDependencies Include="$(EngineRoot)Bin\Engine.pdb"
                                 Condition="'$(Configuration)'=='Debug'" />
      <ClientRuntimeDependencies Include="$(EngineRoot)ThirdPartyLib\FMOD\Bin\fmod.dll" />
```

현재 배포 message:

```xml
    <Message Text="Deployed $(AssimpRuntimeName), Engine.dll, and fmod.dll to $(TargetDir)"
             Importance="high" />
```

아래로 교체:

```xml
    <Message Condition="'$(Configuration)'=='Debug'"
             Text="Deployed $(AssimpRuntimeName), Engine.dll, Engine.pdb, and fmod.dll to $(TargetDir)"
             Importance="high" />
    <Message Condition="'$(Configuration)'!='Debug'"
             Text="Deployed $(AssimpRuntimeName), Engine.dll, and fmod.dll to $(TargetDir)"
             Importance="high" />
```

Client 또는 solution Debug build가 project reference로 Engine을 먼저 빌드한 다음, 같은 배포 target에서 DLL/PDB 쌍을 `Client/Bin`으로 복사한다. Engine만 단독 빌드했을 때 Client 출력이 갱신되지 않는 소유권은 그대로 유지한다.

### 6-5. C:/Users/user/Desktop/LostArk/Engine/Private/Level.cpp

현재 빈 함수:

```cpp
void CLevel::Update(f32_t fTimeDelta)
{
}
```

아래로 교체:

```cpp
void CLevel::Update(f32_t fTimeDelta)
{
#ifdef _DEBUG
	[[maybe_unused]] volatile f32_t fDebugTimeDelta = fTimeDelta;
#endif
}
```

이 코드는 Debug에서만 실행되는 의도적인 디버거 정지 anchor다. 매 frame 로그를 출력하지 않고, Release runtime에는 포함되지 않는다. `volatile` store가 실제 명령과 source sequence point를 남기므로 F11 검증 대상이 생긴다.

### 6-6. 검증

자동 검증:

```powershell
$ErrorActionPreference = 'Stop'

$msbuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
    -latest -products * -requires Microsoft.Component.MSBuild `
    -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
if ([string]::IsNullOrWhiteSpace($msbuild)) { throw "MSBuild.exe was not found" }

& $msbuild "C:\Users\user\Desktop\LostArk\Framework.sln" `
    /t:Build /p:Configuration=Debug /p:Platform=x64 /m:1 /nologo
if ($LASTEXITCODE -ne 0) { throw "Debug x64 build failed: $LASTEXITCODE" }

$engineDllHash = (Get-FileHash -ErrorAction Stop "C:\Users\user\Desktop\LostArk\Engine\Bin\Engine.dll").Hash
$clientDllHash = (Get-FileHash -ErrorAction Stop "C:\Users\user\Desktop\LostArk\Client\Bin\Engine.dll").Hash
$enginePdbHash = (Get-FileHash -ErrorAction Stop "C:\Users\user\Desktop\LostArk\Engine\Bin\Engine.pdb").Hash
$clientPdbHash = (Get-FileHash -ErrorAction Stop "C:\Users\user\Desktop\LostArk\Client\Bin\Engine.pdb").Hash

if ($engineDllHash -ne $clientDllHash) { throw "Engine.dll deployment mismatch" }
if ($enginePdbHash -ne $clientPdbHash) { throw "Engine.pdb deployment mismatch" }

Push-Location "C:\Users\user\Desktop\LostArk"
git diff --check
if ($LASTEXITCODE -ne 0) { throw "git diff --check failed: $LASTEXITCODE" }
Pop-Location

$client = $null
try
{
    $client = Start-Process `
        -FilePath "C:\Users\user\Desktop\LostArk\Client\Bin\Client.exe" `
        -WorkingDirectory "C:\Users\user\Desktop\LostArk\Client\Default" `
        -WindowStyle Hidden -PassThru
    Start-Sleep -Seconds 5
    if ($client.HasExited) { throw "Client exited during startup smoke: $($client.ExitCode)" }
}
finally
{
    if ($null -ne $client -and !$client.HasExited)
    {
        $client.Kill()
        $client.WaitForExit()
    }
}
```

통과 조건:

- Debug x64 solution build exit code 0.
- Engine/Client Bin의 `Engine.dll` SHA-256 일치.
- Engine/Client Bin의 `Engine.pdb` SHA-256 일치.
- hidden startup smoke 5초 동안 즉시 process 종료 없음. modal loader/assertion dialog 부재까지 자동 판정한다고 주장하지 않는다.
- `git diff --check` 신규 오류 없음.

수동 검증:

```text
1. Debug x64에서 Client를 시작 프로젝트로 F5 실행한다.
2. F1로 LostArk Map Tool을 연다.
3. title bar를 Client 경계 밖까지 끌어 별도 OS 창으로 분리되는지 확인한다.
4. 그 창을 Client 메인 viewport 위로 다시 끌어 자동 병합되는지 확인한다.
5. Visual Studio의 Debug > Windows > Modules에서 Engine.dll 경로가 Client/Bin/Engine.dll이고 Symbols가 Loaded인지 확인한다. 실제 symbol file 경로는 그대로 기록하며, 현재 RSDS 기준 정상 기대값은 Engine/Bin/Engine.pdb다.
6. Client/Private/Level_GamePlay.cpp의 __super::Update 호출에서 F11을 누르고 Engine/Private/Level.cpp의 fDebugTimeDelta 대입 줄에 진입하는지 확인한다.
```

멀티 viewport drag/merge와 Visual Studio F11 동작은 GUI 상호작용이므로 자동 빌드만으로 최종 판정하지 않고 수동 확인 항목으로 남긴다.

RESULT 시각 증거 경로:

```text
.md/build/imgui-f1/2026-07-29_LOSTARK_IMGUI_VIEWPORT_DETACHED.png
.md/build/imgui-f1/2026-07-29_LOSTARK_IMGUI_VIEWPORT_MERGED.png
.md/build/imgui-f1/2026-07-29_LOSTARK_ENGINE_MODULE_SYMBOLS.png
.md/build/imgui-f1/2026-07-29_LOSTARK_ENGINE_F11_LEVEL_UPDATE.png
```

Modules 증거에는 loaded module 경로 `Client/Bin/Engine.dll`, symbol status `Symbols loaded`, 실제 symbol file 경로가 함께 보여야 한다. 현재 DLL의 RSDS에는 `Engine/Bin/Engine.pdb` 절대 경로가 내장되어 있으므로 Visual Studio가 이 원본 PDB를 먼저 읽는 것이 정상이다. `Client/Bin/Engine.pdb`가 같은 파일이라는 사실은 자동 SHA-256 비교로 별도 증명한다.

hidden smoke는 즉시 crash만 잡는 보조 검사다. DLL 누락이나 assertion이 modal dialog를 띄우면 process가 살아 있을 수 있으므로 loader/backend 정상 판정은 visible F5에서 정상 Client window가 열리고 F1 Map Tool까지 도달하는 수동 단계와 detached screenshot으로 확정한다.

Release 전체 build는 이번 후속 성공 기준에 넣지 않는다. 기존 RESULT가 이미 이번 변경 이전부터 `RenderTarget.cpp`의 Debug-only 선언/Release 정의 불일치로 Release x64가 실패함을 기록했고, 사용자 요청은 Debug F5와 Engine F11이다. 해당 기존 오류까지 수정하면 범위가 확장된다. 대신 `Engine.pdb` item과 missing-file `Error` 모두 명시적인 `Condition="'$(Configuration)'=='Debug'"`를 사용하며, 이번 후속 build의 최종 공용 `Client/Bin` 산출물은 반드시 Debug로 남긴다.

### 6-7. 후속 독립 비평 게이트

이 후속 계획도 구현 전에 `/root/imgui_f1_plan_critique`의 read-only 비평을 받고, accepted/held P0/P1이 0일 때만 소스 수정에 진입한다.

### 6-8. 후속 1차 독립 비평 — `/root/imgui_f1_plan_critique`

판정: FAIL, P0=0, P1=2, P2=3.

| 지적 | 처분 | 근거와 반영 |
|---|---|---|
| 자동 검증이 build/hash/diff/smoke 실패를 강제하지 않음 | 수용 | `$LASTEXITCODE`, hash 비교 `throw`, hidden startup smoke, `git diff --check` 명령을 §6-6에 추가 |
| Release → Debug 전체 build가 필요 | 기각 | 기존 unrelated `RenderTarget` Release compile 실패가 확인되어 있고 이번 사용자 목표는 Debug F5/F11이다. 해당 오류 수정은 별도 범위이며, Debug build와 조건부 XML을 검증하고 최종 산출물을 Debug로 유지 |
| 새 GUI 행동의 시각 증거 경로가 없음 | 수용 | detached/merged/Modules/F11 네 증거 경로와 Modules 필수 표시 내용을 §6-6에 추가 |
| volatile read가 불필요 | 수용 | `[[maybe_unused]] volatile` 초기화 한 줄만 유지 |
| Debug 배포 메시지에 PDB가 드러나지 않음 | 수용 | 아래 §6-4의 메시지 교체를 추가 |
| Modules에서 실제 symbol file 경로도 확인 필요 | 수용 | §6-6 수동 검증·증거 조건에 실제 symbol file 경로 기록 항목 추가. 2차 비평에서 RSDS 절대 경로에 맞게 기대값을 교정 |

### 6-9. 후속 2차 독립 비평 — `/root/imgui_f1_plan_critique`

판정: FAIL, P0=0, P1=2, P2=2. Release 전체 build 기각은 기존 unrelated Release 실패와 Debug 목표가 명확하므로 허용 판정을 받았다.

| 지적 | 처분 | 근거와 반영 |
|---|---|---|
| 실제 symbol path를 `Client/Bin/Engine.pdb`로 고정하면 RSDS 절대 경로와 불일치 | 수용 | Modules는 실제 경로를 기록하고 정상 기대값을 `Engine/Bin/Engine.pdb`로 교정. Client/Bin 배포본 동일성은 SHA-256으로 분리 검증 |
| 5초 process 생존만으로 loader/backend assertion 부재를 단정할 수 없음 | 수용 | 자동 smoke 성공 조건을 즉시 종료 없음으로 축소하고, visible F5→F1 도달과 detached 증거를 loader/backend 수동 gate로 명시 |
| MSBuild 미발견·hash cmdlet 오류 강제 보강 | 수용 | `$ErrorActionPreference='Stop'`, MSBuild empty 검사, `-ErrorAction Stop` 추가 |
| smoke cleanup 보강 | 수용 | process object를 `try/finally`에서 종료·대기하도록 교체 |

### 6-10. 후속 최종 독립 비평 — `/root/imgui_f1_plan_critique`

최종 판정: PASS, P0=0, P1=0. accepted/held P0/P1은 없다.

- MSBuild 미발견·build 실패, DLL/PDB hash 불일치, `git diff --check` 실패를 모두 강제한다.
- hidden smoke와 visible F5→F1 수동 판정의 역할을 분리했다.
- RSDS 실제 PDB 경로와 Client/Bin PDB 배포본 hash 검증을 분리했다.
- process object 기반 cleanup으로 정상 smoke의 false failure를 제거했다.

구현 진입 가능.
