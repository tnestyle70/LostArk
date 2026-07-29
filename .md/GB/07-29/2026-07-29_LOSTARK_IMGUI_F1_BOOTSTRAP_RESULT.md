# Session - Debug F1 Dear ImGui bootstrap 구현 결과
좌표: LA-F06 · 대응 계획: `2026-07-29_LOSTARK_IMGUI_F1_BOOTSTRAP_PLAN.md`

## 1. 결과

Debug x64의 Engine/Client ImGui runtime 배선을 구현했다. `CMainApp`이 F1 toggle과 Debug overlay 수명을, Engine `CImGuiLayer`가 ImGui context·Win32/DX11 backend·frame·WndProc 수명을, Client `CMapTool`이 panel을 소유한다.

이번 사용자 결정에 따라 물리 파일은 수업 프레임워크의 flat 구조를 유지한다.

```text
Engine/Public/ImGuiLayer.h
Engine/Private/ImGuiLayer.cpp
Client/Public/MapTool.h
Client/Private/MapTool.cpp
```

Visual Studio 가상 filter만 `05. Editor`, `04. External\imgui`, `03. Tools\00. Map`으로 분류한다. Scene/Level은 이번 slice에 추가하지 않았다.

## 2. 구현 경계

### Engine

- `CImGuiLayer`: `CreateContext`, Win32/DX11 init, `BeginFrame`, `EndFrame`, 실패 frame 취소, backend/context 종료를 한 객체에 묶었다.
- `HandleWindowMessage`: Client WndProc가 backend 구현을 직접 알지 않도록 Engine wrapper를 제공한다.
- `CInput_Device`: keyboard, mouse button, mouse delta getter가 capture gate 상태에서는 0을 반환한다.
- 기존 pointer arithmetic 기반 mouse delta 조회를 `DIMM::X/Y/WHEEL → DIMOUSESTATE::lX/lY/lZ` switch로 교체했다.
- `CGameInstance`: Client가 input device 구현을 직접 보지 않고 block/capture 상태를 전달·조회한다.

### Client

- `CMapTool`: `Client_Defines.h`와 `Engine_Defines.h`를 포함하고 기존 `NS_BEGIN(Client)`/`NS_END` 형식을 사용한다. 이 저장소에는 `NS_CLIENT` 매크로가 없다.
- `CMainApp`: Debug에서만 ImGui/MapTool을 초기화하고 `F1` rising edge로 panel을 연다.
- frame 순서: `F1 → BeginFrame → capture gate → game update → game render → MapTool submit → ImGui render → Present`.
- WndProc 메시지는 context가 존재할 때 Engine wrapper를 거쳐 Win32 backend에 전달한다.
- Logo Space, Loading Enter, Player 방향키/좌클릭 raw Win32 input에도 capture guard를 추가했다. Camera DirectInput은 Engine getter gate로 차단한다.

### SDK/빌드

Client는 `Engine/Public` 또는 `Engine/External`을 직접 include하지 않는다. `PrepareEngineSdk`가 Client compile 전에 다음 header를 `EngineSDK/inc` flat 루트에 게시한다.

```text
ImGuiLayer.h
imgui.h
imconfig.h
```

`imconfig.h`의 `ENGINE_EXPORTS` 분기로 Engine 빌드는 ImGui API를 export하고 Client 빌드는 같은 API를 import한다.

## 3. 질문한 코드 해설

### `Impl`

`Impl`은 `Implementation`의 약자다.

- `ImGui_ImplWin32_*`: Windows message/input/cursor를 ImGui input event로 바꾸는 platform backend 구현
- `ImGui_ImplDX11_*`: ImGui draw data를 D3D11 draw call로 바꾸는 renderer backend 구현

이 이름의 `Impl`은 C++의 PImpl idiom을 뜻하지 않는다.

### 수동 `extern ImGui_ImplWin32_WndProcHandler`

`extern` 선언은 함수를 새로 정의하지 않는다. `imgui_impl_win32.cpp`에 존재하는 함수의 signature를 현재 translation unit에 알려 linker가 그 정의를 연결하게 한다.

공식 `imgui_impl_win32.h`는 Windows 타입 의존을 header 사용자 모두에게 강제하지 않기 위해 이 선언을 의도적으로 `#if 0` 안에 둔다. header 주석도 실제 호출 `.cpp`에서 선언을 복사하라고 안내한다. 따라서 이번 수동 선언은 중복 구현이 아니라 공식 backend 사용 방식이다.

### `ImGuiIO`

`ImGuiIO& io = ImGui::GetIO()`는 현재 ImGui context의 입력·설정·출력 상태를 참조한다.

- `NavEnableKeyboard`: keyboard로 ImGui widget을 탐색할 수 있게 한다.
- `NoMouseCursorChange`: ImGui가 OS cursor 모양을 바꾸지 못하게 해 현재 engine cursor 정책을 보존한다.
- `IniFilename = nullptr`: 실행 위치에 `imgui.ini`를 생성하지 않고 bootstrap panel을 매 실행 stateless하게 시작한다.

`WantCaptureKeyboard`와 `WantCaptureMouse`는 현재 UI가 해당 입력을 소비하려는지를 Client/Engine input gate에 전달하는 출력 상태다.

### 기존 `reinterpret_cast` mouse delta

기존 코드는 `DIMOUSESTATE` 시작 주소를 `int32_t*`로 보고 enum 값 0/1/2만큼 이동했다. 이 구조체 앞부분이 `lX`, `lY`, `lZ` 순서로 연속 배치된 사실을 이용한 코드다.

```cpp
return *((reinterpret_cast<int32_t*>(&m_tMouseState)) + ETOUI(eMouseState));
```

동작은 하지만 필드와 범위가 코드에서 보이지 않는다. 이번에는 같은 의미를 다음처럼 명시했다.

```cpp
switch (eMouseState)
{
case DIMM::X:     return m_tMouseState.lX;
case DIMM::Y:     return m_tMouseState.lY;
case DIMM::WHEEL: return m_tMouseState.lZ;
default:          return 0;
}
```

## 4. 검증 결과

### 계획 비평

- 최종 독립 비평: PASS, P0=0, P1=0.
- Debug의 `#define new DBG_NEW`가 ImGui placement `new`를 오염하지 않도록 `imgui.h`를 framework header보다 먼저 include하도록 교정했다.

### Release x64

명령을 실행했으나 기존 Engine 코드에서 실패했다.

```text
RenderTarget.h: Ready_DebugDesc와 m_WorldMatrix는 _DEBUG 안에서만 선언
RenderTarget.cpp: Ready_DebugDesc 정의는 _DEBUG 밖에 존재
Release 결과: C2039 1건, C2065 5건
```

이번 ImGuiLayer와 ImGui core/backend의 Release compile은 해당 지점 전까지 통과했다. `RenderTarget` 수정은 이번 요청 범위가 아니므로 변경하지 않았다.

### Debug x64

```text
Engine.vcxproj compile/link: PASS
Client.vcxproj compile/link: PASS
Client.exe 생성: PASS
Debug assimp/Engine/FMOD 배포: PASS
MSBuild exit code: 0
최종 CP949 보존 후 Debug 증분 재빌드: PASS
```

기존 C4244/C4267 변환 warning과 외부 DirectXTK/Effects PDB LNK4099 warning은 남아 있으나 이번 ImGui 변경의 error는 없다.

### SDK·DLL 증거

- `Engine/Public/ImGuiLayer.h == EngineSDK/inc/ImGuiLayer.h`: SHA-256 일치
- `Engine/External/imgui/imgui.h == EngineSDK/inc/imgui.h`: SHA-256 일치
- `Engine/External/imgui/imconfig.h == EngineSDK/inc/imconfig.h`: SHA-256 일치
- `Engine/Bin/Engine.dll == Client/Bin/Engine.dll`: SHA-256 일치
- Debug Assimp source와 `Client/Bin/assimp-vc143-mtd.dll`: SHA-256 일치
- Engine.dll export에서 `CImGuiLayer`, `ImGui::Begin`, `ImGui::GetIO`, `ImGui_ImplWin32_WndProcHandler` 확인
- Client.exe dependency에서 `Engine.dll`과 Debug CRT 확인
- `EngineSDK/inc/Editor/...`, `EngineSDK/inc/ThirdParty/ImGui/...` stale 계층 파일 없음
- `git diff --check`: whitespace error 없음. 기존 line-ending warning만 존재

### 런타임 자동 smoke

`Client/Default`를 working directory로 Debug `Client.exe`를 hidden 실행했다. 5초 동안 process가 종료되지 않아 Windows loader, `CMainApp::Create`, Engine 초기화, `ReadyDebugTools`, ImGui Win32/DX11 초기화가 crash 없이 통과했음을 확인한 뒤 test가 process를 종료했다.

hidden 실행이므로 실제 F1 key와 panel 화면은 자동 판정하지 않았다.

최종 인코딩 확인 뒤 생성된 마지막 `Client.exe`도 hidden 3초 재실행에서 생존했다.

## 5. 수동 확인 필요

다음 시각 확인만 남는다.

```text
1. Client/Bin/Client.exe 실행
2. Logo에서 F1 한 번 → LostArk Map Tool 표시
3. 창 클릭 후 Keyboard/Mouse capture가 ON인지 확인
4. capture ON 상태에서 Space/Enter/방향키/좌클릭이 game으로 새지 않는지 확인
5. F1 또는 X로 닫은 뒤 기존 입력이 다시 동작하는지 확인
```

계획의 시각 증거 경로 `.md/build/imgui-f1/2026-07-29_LOSTARK_IMGUI_F1_BOOTSTRAP.png`는 수동 화면 확인 후 채운다.

## 6. 보존한 동시 변경

작업 중 별도 세션이 변경한 `.gitattributes`, `.gitignore`, `Engine_Defines.h` whitespace, Sound filter의 unfiltered 상태는 되돌리거나 재구성하지 않았다.

## 7. 2026-07-29 후속 결과 — 외부 ImGui viewport와 Engine F11

### 7-1. 완료 범위

- `Engine/Private/ImGuiLayer.cpp`
  - `ImGuiConfigFlags_ViewportsEnable` 활성화.
  - main viewport draw 뒤 `ImGui::UpdatePlatformWindows()`와 `ImGui::RenderPlatformWindowsDefault()` 실행.
  - 기존 `CMainApp::Render` 순서상 보조 viewport 렌더 뒤 main swap chain `Present`가 실행된다.
- `Client/Default/Client.vcxproj`
  - Debug x64 Client build가 `Engine.dll`과 `Engine.pdb`를 같은 `Client/Bin`에 함께 배포.
  - Debug PDB가 없으면 build를 실패시키는 `Error` 추가.
  - Debug 배포 로그에 `Engine.pdb`를 명시.
- `Engine/Private/Level.cpp`
  - 빈 `CLevel::Update`에 Debug 전용 `volatile` 대입 한 줄을 추가해 F11이 멈출 source sequence point를 생성.
  - Release에는 포함되지 않고 매 frame 로그도 출력하지 않는다.

### 7-2. 외부 창 원리

Map Tool을 Client 경계 밖으로 끌면 같은 ImGui window가 복제되는 것이 아니라 main viewport에서 secondary viewport로 소유권을 옮긴다. Win32 backend가 보조 HWND의 생성·이동·파괴를 담당하고, DX11 backend가 그 HWND용 swap chain을 생성해 별도로 렌더링한다.

`io.ConfigViewportsNoAutoMerge`는 기본값 `false`를 유지했다. 따라서 보조 창을 다시 Client main viewport 위로 가져오면 자동으로 main viewport에 병합된다. Docking은 이번 기능에 필요하지 않아 켜지 않았다. `io.IniFilename = nullptr`도 유지했으므로 재실행 후 창 위치는 저장되지 않는다.

### 7-3. F11이 안 됐던 정확한 원인

변경 전 증거:

```text
Engine/Bin/Engine.dll  : 2026-07-29 07:25:55, SHA-256 5E49F410...
Client/Bin/Engine.dll  : 2026-07-29 07:14:01, SHA-256 0E10F5BE...
Engine/Bin/Engine.pdb  : 존재
Client/Bin/Engine.pdb  : 없음
```

더 결정적인 RSDS 서명은 다음처럼 어긋나 있었다.

```text
Client/Bin/Engine.dll -> PDB GUID {6415560E-D9FA-4402-B875-240D1354BE9A}, age 2
Engine/Bin/Engine.pdb  -> 같은 GUID, age 3
```

Engine project만 다시 빌드하면 `Engine/Bin`만 갱신된다. F5 Client는 `Client/Bin/Engine.dll`을 로드하고, 기존 Client runtime deploy target은 Client build 뒤에만 실행됐다. 그래서 Engine 단독 rebuild 후에도 실행 DLL은 stale 상태였다.

또한 대상 `CLevel::Update`가 완전히 빈 함수라서 심볼이 맞더라도 정지할 source statement가 없었다. F11이 함수에 들어갔다가 바로 caller로 돌아온 것처럼 보일 수 있는 두 번째 원인이었다.

### 7-4. 독립 비평

후속 계획은 기존 `/root/imgui_f1_plan_critique`가 세 차례 read-only로 검토했다.

- 1차: FAIL, P0=0, P1=2 — 자동 failure 강제와 GUI 증거 경로 보강.
- 2차: FAIL, P0=0, P1=2 — RSDS 실제 symbol path와 hidden smoke 판정 범위 교정.
- 3차: FAIL, P0=0, P1=2 — 계획 코드 블록의 MSBuild empty 검사와 process handle cleanup 누락 교정.
- 최종: PASS, P0=0, P1=0 — accepted/held P0/P1 없음.

### 7-5. 자동 검증 결과

Debug x64 solution build:

```text
Engine.vcxproj -> Engine/Bin/Engine.dll
Client.vcxproj -> Client/Bin/Client.exe
Deployed assimp-vc143-mtd.dll, Engine.dll, Engine.pdb, and fmod.dll to Client/Bin
MSBUILD_EXIT=0
```

새 산출물:

```text
Engine/Bin/Engine.dll == Client/Bin/Engine.dll
SHA-256 BC6E411995B6DFC758ED570FB0CA804F66185C2BF20B84CAC38AFA572B5C51CF

Engine/Bin/Engine.pdb == Client/Bin/Engine.pdb
SHA-256 4C43371DCA5AF68A65C60BD6BEB4768477C4BAF70461F796CB2FBF7E3BFCF01C

새 Client/Bin/Engine.dll RSDS
GUID {6415560E-D9FA-4402-B875-240D1354BE9A}, age 4,
C:/Users/user/Desktop/LostArk/Engine/Bin/Engine.pdb
```

- `CLevel::Update` Engine DLL export 확인.
- Debug disassembly에서 `CLevel::Update` 본문에 `movss ... [fDebugTimeDelta]` store가 생성되어 빈 함수가 아닌 실제 F11 정지 anchor임을 확인.
- `git diff --check`: PASS. 기존 line-ending warning만 있고 whitespace error는 없음.
- hidden Client startup 5초 생존: PASS. 이 검사는 즉시 crash만 판정하며 modal loader/assertion dialog 부재를 단정하지 않는다.
- 기존 C4244/C4267, DirectXTK PDB warning 외 이번 변경의 compile/link error 없음.

Release 전체 build는 기존 RESULT에 기록된 unrelated `RenderTarget.cpp` Debug-only 선언 불일치 때문에 이번 Debug F5/F11 성공 기준에서 제외했다. 해당 기존 오류는 수정하지 않았다.

### 7-6. GUI 수동 확인

코드·빌드·배포 검증은 완료됐다. 다음 GUI 동작은 사용자의 Visual Studio/Desktop 상호작용으로 최종 확인한다.

```text
1. Debug x64, Client 시작 프로젝트로 F5.
2. F1로 LostArk Map Tool을 연다.
3. title bar를 Client 밖으로 끌어 별도 OS 창이 되는지 확인.
4. 다시 Client main viewport 위로 끌어 자동 병합되는지 확인.
5. Debug > Windows > Modules에서 module path가 Client/Bin/Engine.dll이고 Symbols Loaded인지 확인.
6. 실제 symbol path는 DLL의 RSDS상 Engine/Bin/Engine.pdb가 정상이다.
7. Client/Private/Level_GamePlay.cpp의 __super::Update 줄에서 F11을 눌러 Engine/Private/Level.cpp의 fDebugTimeDelta 줄에 진입하는지 확인.
```

예정 증거 경로:

```text
.md/build/imgui-f1/2026-07-29_LOSTARK_IMGUI_VIEWPORT_DETACHED.png
.md/build/imgui-f1/2026-07-29_LOSTARK_IMGUI_VIEWPORT_MERGED.png
.md/build/imgui-f1/2026-07-29_LOSTARK_ENGINE_MODULE_SYMBOLS.png
.md/build/imgui-f1/2026-07-29_LOSTARK_ENGINE_F11_LEVEL_UPDATE.png
```

### 7-7. 재발 방지 사용법

Engine만 단독 빌드한 직후에는 `Client/Bin` 배포가 실행되지 않는다. Client를 실행할 때는 Client project 또는 solution을 Build/F5해야 하며, 그러면 project reference로 Engine을 먼저 빌드한 뒤 Client deploy target이 DLL/PDB 쌍을 동기화한다. Visual Studio를 다시 세팅할 필요는 없다.
