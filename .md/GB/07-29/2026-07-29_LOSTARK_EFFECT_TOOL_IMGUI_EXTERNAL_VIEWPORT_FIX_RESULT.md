# LostArk Effect Tool 및 ImGui 외부 창 안정화 적용 정리

- 작성일: 2026-07-29
- 적용 대상: LostArk 팀 프레임워크
- 적용 구성: Debug x64
- 상태: 코드 적용 및 Engine/Client 빌드 완료

> 이 문서는 현재 작업 트리의 최종 코드를 기준으로 작성했다.
> 이전 문서에 `DockingEnable`을 사용하지 않거나 `IniFilename = nullptr`로 유지한다고 적힌 내용이 있다면,
> 현재 구현에 대해서는 이 문서의 내용을 우선한다.

## 1. 이번 작업의 목적

이번 작업의 목적은 다음 네 가지다.

1. 기존 Map Tool 옆에 Effect Tool의 기본 창을 추가한다.
2. F3 같은 별도 단축키를 추가하지 않고 F1을 전체 에디터 작업 공간의 ON/OFF 키로 사용한다.
3. Map Tool과 Effect Tool을 서로 도킹하여 탭으로 합칠 수 있게 한다.
4. ImGui 창을 게임 클라이언트 밖으로 꺼내도 게임 화면과 픽킹이 정상적으로 동작하게 한다.

최종 동작은 다음과 같다.

```text
F1 OFF
└─ Map Tool과 Effect Tool 모두 렌더링하지 않음

F1 ON
├─ Map Tool 표시
├─ Effect Tool 표시
├─ 두 창을 드래그하여 탭으로 도킹 가능
└─ 게임 창 밖으로 분리하여 별도 OS 창으로 사용 가능
```

Effect Tool은 독립적인 F3 상태를 갖지 않는다.
Map Tool의 기존 열림 상태를 에디터 작업 공간의 대표 상태로 사용한다.

## 2. 발생했던 문제와 원인

### 2-1. F1을 누르면 게임 화면이 파란색으로 변하던 문제

`ImGuiConfigFlags_ViewportsEnable`을 사용하면 ImGui 창을 게임 클라이언트 밖으로 꺼낼 수 있다.
이때 ImGui는 외부 창마다 별도의 Win32 `HWND`, DX11 Swap Chain, RTV를 만든다.

외부 ImGui 창을 그리는 과정은 다음과 같다.

```text
CImGuiLayer::EndFrame()
└─ ImGui::RenderPlatformWindowsDefault()
   └─ 외부 ImGui 창의 RTV와 Viewport를 DeviceContext에 바인딩
```

문제는 외부 창 렌더링이 끝난 뒤 DeviceContext에 마지막 외부 창의 RTV가 남을 수 있다는 점이다.

기존 `Clear_BackBuffer_View()`는 메인 백버퍼를 파란색으로 지우기만 했다.
`ClearRenderTargetView()`는 특정 RTV의 색을 지우는 함수이지,
그 RTV를 이후 렌더링 대상으로 바인딩하는 함수가 아니다.

따라서 기존 프레임 흐름은 다음과 같이 잘못될 수 있었다.

```text
1. 외부 ImGui 창 렌더링
2. 외부 창 RTV와 Viewport가 DeviceContext에 남음
3. 다음 프레임에서 메인 백버퍼를 Clear만 수행
4. 메인 RTV·DSV·Viewport는 다시 바인딩되지 않음
5. Target Manager가 외부 창 RTV를 현재 백버퍼로 잘못 취급
6. 게임 렌더링 결과가 메인 창에 출력되지 않음
7. 메인 창에는 Clear 색상인 파란색만 보임
```

DX11 ImGui Backend의 보조 Viewport 렌더 함수는 내부적으로
외부 창의 RTV를 `OMSetRenderTargets()`로 연결한다.
반면 현재 `Target_Manager::Begin_MRT()`는 `OMGetRenderTargets()`로
그 시점의 렌더 타깃을 백업했다가 MRT 종료 후 다시 연결한다.

따라서 프레임 시작에 외부 ImGui RTV가 남아 있으면
Target Manager도 그 외부 RTV를 정상 백버퍼라고 생각하고 저장하게 된다.
파란 화면은 Clear 색상이 잘못된 문제가 아니라,
MRT 시작 전에 메인 렌더 상태가 확정되지 않았던 문제다.

이 문제는 `CImGuiLayer`나 Effect Tool이 메인 백버퍼를 직접 관리하게 만들어 해결하면 안 된다.
메인 RTV·DSV·Viewport의 소유자는 `CGraphic_Device`이므로,
프레임 시작 시 `CGraphic_Device`가 자신의 메인 렌더 상태를 다시 바인딩하는 것이 책임 분리에 맞다.

이 구조는 이전 백룸 프레임워크에서 외부 ImGui 창을 안정적으로 사용하던 방식과도 같다.

### 2-2. 외부 ImGui 창에서 우클릭하면 Picking.cpp에서 터지던 문제

기존 픽킹 코드는 마우스 좌표를 다음과 같이 바로 배열 인덱스로 사용했다.

```cpp
uint32_t iIndex =
    ptMouse.y * static_cast<uint32_t>(m_vViewportSize.x) +
    ptMouse.x;
```

외부 ImGui 창의 마우스 위치를 게임 창 기준 좌표로 변환하면 다음 값이 나올 수 있다.

```text
게임 창 왼쪽에 있는 외부 창  → ptMouse.x < 0
게임 창 위에 있는 외부 창    → ptMouse.y < 0
게임 창 오른쪽에 있는 외부 창 → ptMouse.x >= 게임 화면 너비
게임 창 아래에 있는 외부 창   → ptMouse.y >= 게임 화면 높이
```

음수 좌표를 `uint32_t` 인덱스 계산에 섞으면 매우 큰 양수로 변환될 수 있다.
그 상태에서 `m_pWorldPositions[iIndex]`에 접근하면 배열 범위를 벗어나 접근 위반이 발생한다.

따라서 배열 인덱스를 계산하기 전에 다음 두 가지를 반드시 확인해야 한다.

1. `GetCursorPos()`와 `ScreenToClient()`가 성공했는가
2. 변환된 좌표가 실제 게임 Viewport 안에 있는가

이 검사는 외부 ImGui 창 사용을 막는 코드가 아니다.
게임 화면에 해당하지 않는 위치에서는 GPU 픽킹 배열을 조회하지 않도록 만드는 안전 검사다.

### 2-3. 외부 ImGui 창에 포커스가 있으면 F1이 작동하지 않던 문제

기존 F1 조건이 다음과 같으면 메인 게임 창에서만 단축키가 작동한다.

```cpp
GetForegroundWindow() == g_hWnd
```

외부 ImGui 창은 별도 `HWND`이므로 메인 게임 창의 `g_hWnd`와 값이 다르다.
하지만 해당 창은 다른 프로그램이 아니라 같은 Client 프로세스가 생성한 ImGui 플랫폼 창이다.

그래서 포그라운드 창의 프로세스 ID를 확인하고,
현재 Client 프로세스가 소유한 창이라면 F1을 허용하도록 변경했다.

### 2-4. 외부 ImGui 창을 조작할 때 게임 입력이 같이 들어갈 수 있던 문제

외부 ImGui 창이 포커스를 얻으면 `WantCaptureMouse`와 `WantCaptureKeyboard`만으로는
현재 프로젝트의 DirectInput 차단 조건이 충분하지 않을 수 있다.

Map Tool이 열려 있고 포그라운드 창이 같은 Client 프로세스의 외부 창이면
키보드와 마우스 입력을 게임 쪽에서 차단하도록 보완했다.

이 처리는 외부 Effect Tool을 클릭하거나 드래그하는 동안
카메라, 플레이어 이동, 월드 픽킹이 동시에 반응하는 것을 막는다.

## 3. 최종 책임 구조

```text
CMainApp
├─ Debug Tool의 생성과 해제
├─ F1 작업 공간 ON/OFF
├─ Map Tool의 열림 상태 공유
└─ ImGui 포커스에 따른 게임 입력 차단

CImGuiLayer
├─ ImGui Context
├─ Win32/DX11 Backend
├─ Docking
├─ 외부 Viewport
└─ ImGui Frame 시작과 종료

CGraphic_Device
├─ 메인 BackBuffer RTV 소유
├─ 메인 DSV 소유
├─ 게임 Viewport 크기 소유
└─ 매 프레임 메인 렌더 상태 복구

CPicking
└─ 게임 Viewport 안의 좌표만 픽킹 배열에서 조회

CMapTool
└─ 전체 에디터 작업 공간의 열림 상태 소유

CEffect_Tool
└─ Effect Tool ImGui 창 내용 렌더링
```

## 4. 수정·추가된 파일

| 파일 | 변경 내용 |
|---|---|
| `Engine/Private/ImGuiLayer.cpp` | Docking, 외부 Viewport, 레이아웃 저장 활성화 |
| `Engine/Public/Graphic_Device.h` | 메인 화면 크기 멤버와 `Bind_MainRenderTarget()` 선언 추가 |
| `Engine/Private/Graphic_Device.cpp` | 매 프레임 메인 RTV·DSV·Viewport 복구 |
| `Engine/Private/Picking.cpp` | 마우스 변환 성공 및 Viewport 범위 검사 추가 |
| `Client/Public/MainApp.h` | Debug Effect Tool 소유 멤버 추가 |
| `Client/Private/MainApp.cpp` | Effect Tool 초기화·렌더·해제, 외부 창 포커스와 F1 처리 |
| `Client/Public/Effect_Tool.h` | Effect Tool 최소 클래스 추가 |
| `Client/Private/Effect_Tool.cpp` | Effect Tool 기본 ImGui 창 추가 |
| `Client/Default/Client.vcxproj` | Effect Tool 헤더·소스 프로젝트 등록 |
| `Client/Default/Client.vcxproj.filters` | Effect Tool을 `03. Tools\02. Effect` 필터에 배치 |

## 5. ImGui 설정

적용 위치:

```text
Engine/Private/ImGuiLayer.cpp
CImGuiLayer::Initialize()
```

적용 코드:

```cpp
ImGuiIO& io = ImGui::GetIO();
io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
io.IniFilename = "imgui.ini";
```

각 설정의 의미는 다음과 같다.

| 설정 | 역할 |
|---|---|
| `NavEnableKeyboard` | 키보드로 ImGui 위젯을 탐색할 수 있게 함 |
| `NoMouseCursorChange` | ImGui가 게임의 OS 커서 정책을 임의로 변경하지 않게 함 |
| `DockingEnable` | Map Tool과 Effect Tool을 드래그하여 탭으로 합칠 수 있게 함 |
| `ViewportsEnable` | ImGui 창을 메인 Client 창 밖의 별도 OS 창으로 분리할 수 있게 함 |
| `IniFilename` | 창 위치, 크기, 도킹 상태를 다음 실행에도 유지함 |

외부 Viewport 렌더링 코드는 다음과 같다.

```cpp
void CImGuiLayer::EndFrame()
{
    if (!m_bInitialized || !m_bFrameStarted)
        return;

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    m_bFrameStarted = false;
}
```

`imgui.ini`에는 개인별 창 배치가 저장된다.
저장소의 `.gitignore`에 `[Ii]mgui.ini`가 등록되어 있으므로 팀 Git에는 올라가지 않는다.

현재 요구사항은 독립된 두 ImGui 창을 서로 탭으로 합치는 것이므로
별도의 전체 화면 `DockSpace`를 만드는 작업은 필수가 아니다.

## 6. Effect Tool 기본 구조

### 6-1. 헤더

파일:

```text
Client/Public/Effect_Tool.h
```

코드:

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CEffect_Tool final
{
public:
    void Render();
};

NS_END
```

현재 Phase 1에서는 실제 이펙트 데이터 편집 기능을 넣기 전에
프레임워크 연결과 창 수명만 확인하면 되므로 `Render()`만 가진 최소 클래스로 시작한다.

### 6-2. 소스

파일:

```text
Client/Private/Effect_Tool.cpp
```

코드:

```cpp
#include "imgui.h"

#include "Effect_Tool.h"

void Client::CEffect_Tool::Render()
{
    ImGui::SetNextWindowSize(
        ImVec2(900.f, 650.f),
        ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("LostArk Effect Tool"))
    {
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted(
        "Effect Tool is connected to the current framework.");
    ImGui::Separator();
    ImGui::TextUnformatted(
        "F1: open / close the editor tool workspace");
    ImGui::TextUnformatted(
        "Drag this window onto Map Tool to dock as a tab.");

    ImGui::End();
}
```

`ImGuiCond_FirstUseEver`를 사용한 이유는 첫 실행에만 기본 크기를 적용하고,
이후에는 `imgui.ini`에 저장된 사용자의 크기와 도킹 배치를 존중하기 위해서다.

`imgui.h`를 먼저 include한 이유는 Debug 빌드의 `new` 매크로가
ImGui 내부 placement new 선언을 오염시키는 문제를 피하기 위해서다.

## 7. MainApp에 Effect Tool 연결

### 7-1. 전방 선언과 Debug 멤버

파일:

```text
Client/Public/MainApp.h
```

코드:

```cpp
class CMapTool;
class CEffect_Tool;

class CMainApp final
{
    // 기존 코드 생략

#ifdef _DEBUG
    std::unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
    std::unique_ptr<CMapTool> m_pMapTool = { nullptr };
    std::unique_ptr<CEffect_Tool> m_pEffectTool = { nullptr };
    bool m_bF1Down = false;
#endif
};
```

별도의 `m_bF3Down`은 추가하지 않았다.
F1과 Map Tool의 기존 열림 상태를 전체 에디터 작업 공간 상태로 재사용하기 때문이다.

Effect Tool의 소유와 사용은 `_DEBUG` 안에 있으므로
일반 게임 실행 흐름에는 Tool 객체를 생성하지 않는다.

주의할 점은 현재 `Effect_Tool.cpp` 파일 자체는 프로젝트에 공통 등록되어 있어
Release에서도 컴파일 대상이 될 수 있다는 것이다.
현재 의미는 “Release에서 객체를 생성하고 실행하지 않는다”이다.
Release 컴파일 대상에서도 완전히 제외해야 한다는 팀 정책이 생기면
프로젝트 Configuration 조건 또는 소스 전체 `_DEBUG` 가드를 별도로 합의해야 한다.

### 7-2. 생성

파일:

```text
Client/Private/MainApp.cpp
CMainApp::ReadyDebugTools()
```

코드:

```cpp
m_pMapTool = std::make_unique<CMapTool>();
m_pEffectTool = std::make_unique<CEffect_Tool>();
```

ImGui Layer 초기화가 성공한 뒤 Tool 객체를 생성한다.
ImGui Context가 준비되지 않은 상태에서 Tool을 렌더링하지 않기 위해서다.

### 7-3. 렌더링

파일:

```text
Client/Private/MainApp.cpp
CMainApp::Render()
```

코드:

```cpp
#ifdef _DEBUG
if (nullptr != m_pImGuiLayer)
{
    if (nullptr != m_pMapTool)
        m_pMapTool->Render();

    if (nullptr != m_pMapTool &&
        m_pMapTool->IsOpen() &&
        nullptr != m_pEffectTool)
    {
        m_pEffectTool->Render();
    }

    m_pImGuiLayer->EndFrame();
}
#endif
```

Map Tool은 기존 코드처럼 매 프레임 `Render()`를 호출하고
내부 `m_bOpen` 상태에 따라 실제 창을 그릴지 결정한다.

Effect Tool은 Map Tool이 열려 있을 때만 `Render()`를 호출한다.
따라서 F1을 누르면 두 창이 함께 나타나고 함께 사라진다.
도킹된 상태와 외부 창으로 분리된 상태 모두 같은 규칙을 사용한다.

### 7-4. 해제 순서

코드:

```cpp
#ifdef _DEBUG
CGameInstance::Get().SetInputBlocked(false, false);

m_pEffectTool.reset();
m_pMapTool.reset();

if (nullptr != m_pImGuiLayer)
    m_pImGuiLayer->Shutdown();
m_pImGuiLayer.reset();
#endif
```

Tool 객체를 먼저 없애고 마지막에 ImGui Layer를 종료한다.
종료 시점에 Tool 객체가 이미 파괴된 ImGui Context를 참조할 가능성을 없애기 위해서다.

## 8. 외부 ImGui 창 포커스와 F1 처리

### 8-1. 현재 프로세스가 소유한 창인지 검사

파일:

```text
Client/Private/MainApp.cpp
```

코드:

```cpp
#ifdef _DEBUG
namespace
{
    bool_t IsWindowOwnedByCurrentProcess(HWND hWnd)
    {
        if (nullptr == hWnd)
            return false;

        DWORD dwProcessId = {};
        if (0 == GetWindowThreadProcessId(hWnd, &dwProcessId))
            return false;

        return GetCurrentProcessId() == dwProcessId;
    }
}
#endif
```

외부 ImGui Viewport는 `g_hWnd`와 다른 `HWND`를 가지지만
Client와 동일한 프로세스 ID를 가진다.
따라서 단순 HWND 비교 대신 프로세스 소유권을 검사한다.

### 8-2. 외부 창에서도 F1 허용

코드:

```cpp
void CMainApp::UpdateDebugToolShortcut()
{
    const bool_t bWindowFocused =
        IsWindowOwnedByCurrentProcess(GetForegroundWindow());
    const bool_t bF1Down = bWindowFocused &&
        0 != (GetAsyncKeyState(VK_F1) & 0x8000);

    if (bF1Down && !m_bF1Down && nullptr != m_pMapTool)
        m_pMapTool->Toggle();

    m_bF1Down = bF1Down;
}
```

`m_bF1Down`은 키를 누르고 있는 동안 매 프레임 토글되는 것을 막고
키를 처음 누른 순간에만 한 번 토글하는 rising-edge 상태다.

### 8-3. 외부 창 사용 중 게임 입력 차단

코드:

```cpp
const bool_t bMapToolOpen =
    nullptr != m_pMapTool && m_pMapTool->IsOpen();

const HWND hForegroundWindow = GetForegroundWindow();
const bool_t bExternalToolFocused = bMapToolOpen &&
    nullptr != hForegroundWindow &&
    hForegroundWindow != g_hWnd &&
    IsWindowOwnedByCurrentProcess(hForegroundWindow);

const bool_t bKeyboardCaptured = bMapToolOpen &&
    nullptr != m_pImGuiLayer &&
    (m_pImGuiLayer->WantsCaptureKeyboard() || bExternalToolFocused);

const bool_t bMouseCaptured = bMapToolOpen &&
    nullptr != m_pImGuiLayer &&
    (m_pImGuiLayer->WantsCaptureMouse() || bExternalToolFocused);

CGameInstance::Get().SetInputBlocked(
    bKeyboardCaptured,
    bMouseCaptured);
```

메인 ImGui 창 안에서는 기존 `WantCaptureKeyboard/Mouse`를 사용한다.
별도 OS 창으로 분리된 Tool이 포커스를 가진 경우에는
동일 프로세스 외부 창 조건을 추가하여 게임 입력을 확실히 막는다.

## 9. 메인 RTV·DSV·Viewport 복구

### 9-1. Graphic Device 멤버와 함수 선언

파일:

```text
Engine/Public/Graphic_Device.h
```

추가 코드:

```cpp
ComPtr<ID3D11RenderTargetView> m_pBackBufferRTV = { nullptr };
ComPtr<ID3D11DepthStencilView> m_pDepthStencilView = { nullptr };

int32_t m_iWinSizeX = {};
int32_t m_iWinSizeY = {};

private:
    HRESULT Ready_SwapChain(
        HWND hWnd,
        WINMODE isWindowed,
        int32_t iWinCX,
        int32_t iWinCY);
    HRESULT Ready_BackBufferRenderTargetView();
    HRESULT Ready_DepthStencilView(
        int32_t iWinCX,
        int32_t iWinCY);
    HRESULT Bind_MainRenderTarget();
```

`m_iWinSizeX`와 `m_iWinSizeY`는 외부 ImGui 창 렌더링으로 변경된
DX11 Viewport를 원래 게임 화면 크기로 되돌릴 때 사용한다.

`Bind_MainRenderTarget()`은 메인 백버퍼 상태를 소유한
`CGraphic_Device` 내부에서만 사용하므로 private 함수로 둔다.

### 9-2. 초기화 시 메인 화면 크기 저장

파일:

```text
Engine/Private/Graphic_Device.cpp
CGraphic_Device::Initialize()
```

코드:

```cpp
m_iWinSizeX = iWinSizeX;
m_iWinSizeY = iWinSizeY;
```

그리고 기존 초기 렌더 타깃 설정도 같은 함수로 통일했다.

```cpp
if (FAILED(Bind_MainRenderTarget()))
    return E_FAIL;
```

초기 설정과 프레임 복구가 서로 다른 코드를 사용하면
나중에 한쪽만 수정되어 상태가 달라질 수 있으므로 같은 함수를 재사용한다.

### 9-3. 매 프레임 시작 시 메인 렌더 상태 복구

코드:

```cpp
HRESULT CGraphic_Device::Clear_BackBuffer_View(
    const float4_t* pClearColor)
{
    if (nullptr == m_pDeviceContext)
        return E_FAIL;

    if (FAILED(Bind_MainRenderTarget()))
        return E_FAIL;

    m_pDeviceContext->ClearRenderTargetView(
        m_pBackBufferRTV.Get(),
        reinterpret_cast<const f32_t*>(pClearColor));

    return S_OK;
}
```

중요한 부분은 Clear보다 먼저 `Bind_MainRenderTarget()`을 호출하는 것이다.

```text
Render_Begin
├─ 메인 RTV 재연결
├─ 메인 DSV 재연결
├─ 게임 Viewport 재설정
└─ 메인 백버퍼 Clear
```

외부 ImGui 창이 전 프레임에 어떤 렌더 상태를 남겼더라도
게임 렌더링을 시작하기 전에 항상 메인 상태로 돌아온다.

현재 유지해야 할 프레임 순서는 다음과 같다.

```text
Render_Begin
→ 메인 RTV·DSV·Viewport 복구 및 Clear
→ MRT와 게임 렌더링
→ 메인 ImGui 렌더링
→ 외부 ImGui Viewport 렌더링
→ Present
```

현재는 `CImGuiLayer::EndFrame()` 이후에 추가 게임 Draw가 없으므로
다음 프레임 시작에서 메인 상태를 복구하는 방식으로 충분하다.
향후 ImGui 외부 창을 그린 직후 같은 프레임에서 게임 Draw를 추가한다면,
그 시점에는 ImGui 렌더 전후의 렌더 상태 즉시 저장·복구도 별도로 검토해야 한다.

### 9-4. Bind_MainRenderTarget 구현

코드:

```cpp
HRESULT CGraphic_Device::Bind_MainRenderTarget()
{
    if (nullptr == m_pDeviceContext ||
        nullptr == m_pBackBufferRTV ||
        nullptr == m_pDepthStencilView)
        return E_FAIL;

    ID3D11RenderTargetView* pRTVs[] = {
        m_pBackBufferRTV.Get(),
    };

    m_pDeviceContext->OMSetRenderTargets(
        1,
        pRTVs,
        m_pDepthStencilView.Get());

    D3D11_VIEWPORT ViewPortDesc{};
    ViewPortDesc.TopLeftX = 0.f;
    ViewPortDesc.TopLeftY = 0.f;
    ViewPortDesc.Width =
        static_cast<f32_t>(m_iWinSizeX);
    ViewPortDesc.Height =
        static_cast<f32_t>(m_iWinSizeY);
    ViewPortDesc.MinDepth = 0.f;
    ViewPortDesc.MaxDepth = 1.f;

    m_pDeviceContext->RSSetViewports(
        1,
        &ViewPortDesc);

    return S_OK;
}
```

각 요소의 의미는 다음과 같다.

| 요소 | 의미 |
|---|---|
| RTV | 색상 결과를 기록할 렌더 타깃. 여기서는 메인 BackBuffer |
| DSV | 깊이·스텐실 판정을 기록할 버퍼 |
| Viewport | 렌더링 좌표를 실제 화면의 어느 영역에 매핑할지 정하는 범위 |
| `OMSetRenderTargets` | 메인 RTV와 DSV를 Output Merger 단계에 연결 |
| `RSSetViewports` | Rasterizer가 사용할 게임 화면 크기를 다시 설정 |

RTV만 복구하면 외부 창의 DSV 또는 Viewport가 남을 수 있다.
따라서 RTV·DSV·Viewport를 한 묶음으로 복구한다.

현재 프레임워크는 고정된 게임 창 크기를 사용하므로 초기 크기를 저장해 재사용한다.
나중에 실제 창 크기 변경과 `ResizeBuffers()`를 지원한다면
리사이즈 시점에 `m_iWinSizeX/Y`, BackBuffer RTV, DSV도 함께 갱신해야 한다.

## 10. Picking 배열 범위 검사

파일:

```text
Engine/Private/Picking.cpp
CPicking::Picking()
```

적용 코드:

```cpp
bool_t CPicking::Picking(float4_t& vOut)
{
    ::POINT ptMouse = {};
    if (FALSE == GetCursorPos(&ptMouse) ||
        FALSE == ScreenToClient(m_hWnd, &ptMouse))
        return false;

    const LONG iViewportWidth =
        static_cast<LONG>(m_vViewportSize.x);
    const LONG iViewportHeight =
        static_cast<LONG>(m_vViewportSize.y);

    if (ptMouse.x < 0 ||
        ptMouse.y < 0 ||
        ptMouse.x >= iViewportWidth ||
        ptMouse.y >= iViewportHeight)
        return false;

    const size_t iIndex =
        static_cast<size_t>(ptMouse.y) *
        static_cast<size_t>(iViewportWidth) +
        static_cast<size_t>(ptMouse.x);

    if (0.f != m_pWorldPositions[iIndex].w)
    {
        vOut = m_pWorldPositions[iIndex];
        return true;
    }

    return false;
}
```

검사 순서는 중요하다.

```text
화면 좌표 취득
→ 게임 Client 좌표로 변환
→ 음수 및 Viewport 바깥 검사
→ 안전한 좌표만 size_t 인덱스로 변환
→ m_pWorldPositions 배열 접근
```

범위 검사 전에 unsigned 타입으로 변환하면
음수가 이미 큰 양수로 바뀌므로 반드시 signed 좌표 상태에서 먼저 검사해야 한다.

## 11. Visual Studio 프로젝트 등록

`Effect_Tool.h/.cpp`를 실제 빌드에 포함하기 위해
`Client/Default/Client.vcxproj`에 다음 항목을 추가했다.

```xml
<ClInclude Include="..\public\Effect_Tool.h" />
<ClCompile Include="..\private\Effect_Tool.cpp" />
```

Solution Explorer에서는 기존 팀 구조에 맞게
`Client/Default/Client.vcxproj.filters`의 다음 필터에 배치했다.

```xml
<ClCompile Include="..\private\Effect_Tool.cpp">
  <Filter>03. Tools\02. Effect</Filter>
</ClCompile>

<ClInclude Include="..\public\Effect_Tool.h">
  <Filter>03. Tools\02. Effect</Filter>
</ClInclude>
```

물리 파일 경로는 기존 Client의 flat 구조를 유지하고
Visual Studio 가상 필터만 Effect Tool 영역으로 분류한다.

## 12. 수정하지 않은 부분

이번 문제 해결을 위해 다음 구조는 수정하지 않았다.

- `Target_Manager`의 MRT 설계
- ImGui DX11 Backend 내부 구현
- Map Tool의 기존 `Toggle()`과 `IsOpen()` 구조
- 게임 오브젝트의 픽킹 호출 구조
- Release 전용 게임 로직

`Target_Manager`에서 외부 ImGui RTV를 별도로 판별하거나
ImGui Backend 내부 코드를 직접 고치지 않은 이유는
메인 렌더 상태 복구가 `CGraphic_Device`의 책임이기 때문이다.

## 13. 빌드 및 배포 검증

### Engine Debug x64

```text
Engine.vcxproj compile/link: PASS
Engine.dll 생성: PASS
오류: 0개
```

### EngineSDK 및 런타임 동기화

```text
UpdateLib.bat Debug: PASS
Engine/Public → EngineSDK/inc 동기화
Engine.lib → EngineSDK/lib 동기화
Engine.dll 및 Debug 런타임 파일 배포
```

`EngineSDK`는 `.gitignore` 대상이며 `UpdateLib.bat`으로 다시 생성한다.
Engine 공개 헤더를 수정한 뒤 Client를 빌드할 때는 SDK 동기화가 필요하다.

### Client Debug x64

```text
Client.vcxproj compile/link: PASS
Client.exe 생성: PASS
Effect_Tool.cpp compile/link: PASS
오류: 0개
```

남은 경고는 기존 정수·실수 변환 경고와
DirectXTK/Effects의 Debug PDB를 찾지 못한다는 `LNK4099` 경고다.
이번 ImGui 외부 창 수정에서 발생한 컴파일 또는 링크 오류는 없다.

이번 확인 기준은 Debug x64다.
`Graphic_Device`와 `Picking`은 공용 Engine 코드이므로
팀 통합 빌드 단계에서는 Release x64도 함께 확인하는 것이 좋다.
이전부터 존재하던 Release 전용 오류가 발생한다면
이번 변경에서 생긴 오류인지 기존 오류인지 파일과 오류 위치를 나눠서 판단해야 한다.

## 14. 수동 실행 확인 항목

다음 순서로 확인한다.

```text
1. Debug x64로 Client 실행
2. F1을 눌러 Map Tool과 Effect Tool이 함께 나타나는지 확인
3. Effect Tool을 Map Tool 위로 드래그해 탭으로 도킹
4. F1 OFF/ON 시 도킹된 두 Tool이 함께 꺼지고 켜지는지 확인
5. Tool을 게임 Client 밖으로 이동
6. 게임 화면이 파란색으로 변하지 않고 계속 렌더링되는지 확인
7. 외부 Tool 창을 클릭하고 드래그해도 게임 카메라와 캐릭터 입력이 반응하지 않는지 확인
8. 외부 Tool 창에서 마우스 우클릭
9. Picking.cpp 접근 위반이 발생하지 않는지 확인
10. 외부 Tool 창에 포커스가 있는 상태에서 F1로 닫을 수 있는지 확인
11. 재실행 후 imgui.ini의 도킹 배치가 복원되는지 확인
```

## 15. 팀 작업 시 주의사항

### 인코딩

현재 일부 기존 C++ 파일은 CP949 인코딩이다.
해당 파일 전체를 UTF-8로 강제 변환하면 팀원의 기존 한글 주석과
불필요한 전체 파일 diff가 발생할 수 있다.

다음 파일은 기존 인코딩을 보존해서 수정해야 한다.

```text
Engine/Public/Graphic_Device.h
Engine/Private/Graphic_Device.cpp
Engine/Private/Picking.cpp
Client/Private/MainApp.cpp
```

### Git

다음 파일은 개인 또는 생성 파일이므로 커밋하지 않는다.

```text
imgui.ini
EngineSDK/
Engine/Bin의 빌드 산출물
Client/Bin의 빌드 산출물
```

실제 소스와 프로젝트 파일만 팀 변경사항으로 검토한다.

`Client.vcxproj`는 Effect Tool 등록 2줄만 의미 있는 변경으로 유지했다.
기존 XML의 줄바꿈이나 들여쓰기를 한 줄 형식으로 바꾸는 포맷 노이즈는 제거했다.
팀 프로젝트 파일은 여러 사람이 동시에 수정하기 쉬우므로
기능과 무관한 전체 XML 재정렬을 피해야 한다.

### 앞으로 Effect Tool 기능을 확장할 때

Phase 1 이후 기능은 `CEffect_Tool::Render()`에 전부 몰아넣기보다
다음처럼 역할별 함수 또는 클래스로 분리하는 것이 좋다.

```text
CEffect_Tool
├─ Render_MenuBar()
├─ Render_Hierarchy()
├─ Render_Inspector()
├─ Render_Timeline()
├─ Render_Preview()
└─ Effect 데이터 Load/Save 계층
```

F1 작업 공간 수명, ImGui Layer 수명, 그래픽 장치 복구 코드는 그대로 두고
Effect Tool 내부 기능만 확장하면 된다.

## 16. 팀장에게 전달할 수 있는 요약

```text
ImGui 창을 게임 밖으로 분리하면 DX11이 마지막 외부 창의 RTV와 Viewport를
DeviceContext에 남길 수 있어서 다음 프레임 게임 렌더 타깃이 잘못 연결됐습니다.
그래서 백버퍼 소유자인 Graphic_Device가 매 프레임 시작할 때
메인 RTV·DSV·Viewport를 다시 연결하도록 수정했습니다.

또 외부 창의 마우스 좌표가 게임 기준으로 음수나 화면 밖 좌표가 될 수 있는데,
기존 Picking 코드가 이를 배열 인덱스로 바로 사용해서 접근 위반이 발생했습니다.
인덱스 계산 전에 Viewport 범위를 검사하도록 수정했습니다.

외부 ImGui 창도 같은 Client 프로세스의 창이므로,
그 창에 포커스가 있어도 F1을 사용할 수 있게 했고
Tool을 조작하는 동안 게임 입력이 같이 들어가지 않도록 차단했습니다.

Map Tool과 Effect Tool은 별도 F3 없이 F1 작업 공간으로 묶었고,
Docking과 외부 Viewport를 모두 유지했습니다.
Engine과 Client Debug x64 빌드는 오류 없이 통과했습니다.
```
