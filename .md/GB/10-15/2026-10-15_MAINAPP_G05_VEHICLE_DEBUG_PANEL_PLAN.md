# G05 탈것 디버그 패널과 카메라 속도 UI 분리

> 현재 직접 구현 기준은 [G05~G16 통합 구현 PLAN](2026-10-15_MAINAPP_G05_G16_INTEGRATED_IMPLEMENTATION_PLAN.md)이다. 이 단독 문서는 이전 제안 기록으로 보존한다. 아래 전문을 최신 파일에 적용하지 않는다. G04는 현재 소스에 반영됐으며, 숨겨진 F1에서도 Dragon publisher를 polling하던 이 문서의 제안은 통합 후보에 채택하지 않았다. 통합 후보는 기존 Render 시점의 polling을 유지한다.

작성일: 2026-10-15. **제품에 적용하지 않은 사용자 직접 구현용 후보**다.
최초 검증 후보의 CPP 세 개는 Debug/Release x64 문법 컴파일을 통과했다. 실제 제품 링크와 실행은 하지 않았다.
이후 MainApp.h의 사용자 빈 줄 삭제를 G04 baseline과 G05 후보에 보존했다. 이 문서 갱신 뒤 재컴파일은 하지 않았다.
작성 순서는 G03 → G04 → G05다. G03은 현재 제품에 반영됐으며 이 문서의 MainApp.h/.cpp와 project/filter는 G04 후보 위에 G05를 적용한 결과다.
아래 전문은 G04 반영 전 시점의 제안 기록이다. 현재 G04 반영 상태와 G05 적용 코드는 위 통합 PLAN을 따른다.
현재 제품 파일에 이 전문을 통째로 덮어쓰지 않고, 아래 추가·교체 부분을 최신 작업본에 병합한다.

- 선행 단계: [G03 Sound 준비 계획](2026-10-15_MAINAPP_G03_SOUND_PREPARATION_PLAN.md)
- 전체 순서: [MainApp 책임 분리 설계](../10-14/2026-10-14_MAINAPP_RESPONSIBILITY_REFACTOR_DESIGN.md)
- 기존 동작: [드래곤 비행 카메라 결과](../09-27/2026-09-27_DRAGON_FLIGHT_CAMERA_IMPLEMENTATION_RESULT.md)

## 목표와 소유권

MainApp_Dragon.cpp는 현재 CMainApp의 두 함수와 함수 내부 static draft를 담는다.
G05는 탈것 저작 상태를 CVehicleDebugPanel로 옮기고, 상태 없는 카메라 속도 UI는 CameraDebugControls에 둔다.
실제 탈것 CPart_Vehicle, Character의 부착, Server 승하차·비행·속도 판정은 그대로 사용한다.

MainApp은 Debug panel을 한 번 lazy 생성해 종료까지 소유한다. 창 닫힘이나 Level 교체로 reset하지 않는다.
panel은 세 속도·baseline·loaded·status·process HANDLE을 소유하고 player/controller/camera는 Render 호출 동안만 차용한다.
CameraDebugControls는 현재 Level/Layer를 조회하며 MainApp을 참조하거나 카메라를 장기 보유하지 않는다.

의도한 동작 변경은 하나다. 기존에는 F1 Render가 호출될 때만 publisher 종료를 Poll했지만,
이 후보는 Debug MainApp::Update에서 창 표시와 무관하게 Poll한다. 파일 Save/Publish는 여전히 사용자의 버튼으로만 시작한다.

## 먼저 볼 것: G05에서 추가·교체하는 부분

### G05-01. 새 파일 네 개 추가

| 작업 | 절대 경로 | 역할 |
|---|---|---|
| 새 파일 | `C:/Users/tnest/Desktop/LostArk/Client/Public/VehicleDebugPanel.h` | Debug panel 수명·API와 저작 상태 선언 |
| 새 파일 | `C:/Users/tnest/Desktop/LostArk/Client/Private/VehicleDebugPanel.cpp` | 기존 Load/Start/Poll과 승하차·카메라 preview UI |
| 새 파일 | `C:/Users/tnest/Desktop/LostArk/Client/Public/CameraDebugControls.h` | 현재 카메라 조회와 속도 UI 선언 |
| 새 파일 | `C:/Users/tnest/Desktop/LostArk/Client/Private/CameraDebugControls.cpp` | 기존 Level/Layer 조회·카메라 속도 setter 연결 |

네 파일은 아래 `전체 파일 코드` 절의 각 전문으로 만든다. 새 파일은 UTF-8 BOM 없음이다.
Engine_Macro는 namespace 선언, Client_Defines는 Windows HANDLE과 플랫폼 include 순서, Engine_Typedef는 공통 scalar 별칭,
string은 패널 상태 메시지, memory는 짧은 카메라 수명 유지에 필요하다.
VehicleDebugPanel의 카메라·controller·HUD 상태는 포인터/참조만 선언하므로 H에서 전방 선언하고 CPP에서 실제 H를 포함한다.

### G05-02. MainApp.h의 전방 선언 추가

파일: `C:/Users/tnest/Desktop/LostArk/Client/Public/MainApp.h`.
작업: 추가. 기준점은 `class CCameraTool;`이다. 그 선언 바로 뒤, 기존 `class CCharacterPreviewPanel;` 앞에 넣는다.
MainApp의 unique_ptr가 타입 이름을 알고, 생성·삭제는 완전한 타입을 포함한 MainApp.cpp에서 수행하게 한다.

```cpp
#ifdef _DEBUG
class CVehicleDebugPanel;
#endif
```

### G05-03. MainApp.h의 두 기존 멤버 함수 선언 삭제

작업: 삭제. `RenderArenaCameraAndPlayerControls` 다음과 `RenderCharacterSelectFloorSwapControls` 앞의 아래 네 줄을 지운다.
카메라 속도는 자유 함수, 탈것 UI는 panel의 Render가 대체한다.

```cpp
// 현재 자유 카메라 이동 속도를 m/s로 편집한다. 아레나는 Level setter로, 다른 맵은 현재 카메라 인스턴스로 전달한다.
void RenderCameraSpeedControls();
// Developer Tools의 승하차·비행 카메라·속도 저작 패널이다. controller 요청, 로컬 preview, 파일 저장의 경계를 나눈다.
void RenderDragonControls();
```

### G05-04. MainApp.h에 Debug 소유 멤버 추가

작업: 추가. `unique_ptr<CCameraTool> m_pCameraTool = { nullptr };` 다음의 기존 `#ifdef _DEBUG` 안,
`unique_ptr<CWorldObjectTool> m_pWorldObjectTool;` 바로 앞에 넣는다.
창 닫기에서는 이 포인터를 유지하고 Free에서만 정리한다.

```cpp
// 닫힌 F1 패널의 draft와 게시 HANDLE도 MainApp 종료까지 유지한다.
std::unique_ptr<CVehicleDebugPanel> m_pVehicleDebugPanel;
```

### G05-05. MainApp.cpp include 추가

파일: `C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp`.
작업: 추가. 기존 `#include "Camera_Free.h"` 바로 아래에 넣는다.
카메라 UI는 Debug/Release 공통이고 탈것 저작 panel은 Debug에서만 호출한다.

```cpp
#include "CameraDebugControls.h"
#ifdef _DEBUG
#include "VehicleDebugPanel.h"
#endif
```

### G05-06. MainApp::Update에 숨김 중 완료 확인 추가

작업: 추가. `void CMainApp::Update(const f32_t fTimeDelta)`의 첫 기존 `#ifdef _DEBUG` 안,
`if (m_pGuideAITool)` 바로 앞에 넣는다. Update가 기존 UI 표시 조건보다 먼저 실행하므로 F1을 닫아도 작업을 마무리한다.
WaitForSingleObject의 timeout은 0이므로 외부 프로세스 종료를 기다리지 않는다.
완료 성공 뒤 Load의 기존 동기 파일 읽기는 유지한다.

```cpp
// 창이 숨겨져도 진행 중인 게시 작업의 종료를 확인한다.
if (m_pVehicleDebugPanel)
{
    m_pVehicleDebugPanel->Update();
}
```

### G05-07. 카메라 속도 호출 두 곳 교체

작업: 호출 이름 교체. RenderDeveloperTools에서 Character Select 분기 안과,
뒤쪽 `currentLevelId != ETOUI(LEVEL::CHARACTER_SELECT)` 분기 안의 두 호출을 모두 바꾼다.
주변 Level 조건과 호출 순서는 유지한다. 별도 CMainApp wrapper를 남기지 않는다.

기존:

```cpp
RenderCameraSpeedControls();
```

교체:

```cpp
Render_CameraSpeedControls();
```

### G05-08. Dragon 호출을 panel 연결로 교체

작업: 기존 `RenderDragonControls();` 한 줄 전체 교체.
위치는 두 번째 카메라 속도 호출 다음, `CLevel_Development::Render_ColosseumIntroControls();` 바로 앞이다.
새 panel은 생성 후 계속 보유하며 HUD 상태와 controller는 현재 프레임에서 다시 구한다.
shared_ptr는 panel을 호출하는 동안 카메라를 유지하고 raw pointer는 Render 종료 후 저장하지 않는다.

```cpp
#ifdef _DEBUG
// 초안은 panel이 소유하고 현재 player/controller/camera는 이 호출 동안만 차용한다.
if (!m_pVehicleDebugPanel)
{
    m_pVehicleDebugPanel = std::make_unique<CVehicleDebugPanel>();
}
{
    const std::shared_ptr<CCamera_Free> pVehicleDebugCamera = Find_ActiveDebugCamera();
    m_pVehicleDebugPanel->Render(CCombatHUDViewModel::Get().Get_Player(),
        Find_ActivePlayerController(), pVehicleDebugCamera.get());
}
#endif
```

### G05-09. MainApp::Free에서 panel 종료

작업: 추가. `void CMainApp::Free()`의 첫 `#ifdef _DEBUG` 안,
기존 `if (m_pEffectTool)` 바로 앞에 넣는다. 소멸자의 Shutdown 재호출은 handle이 null이므로 안전하다.
외부 publisher의 소유권을 강제로 가져오지 않으며 TerminateProcess나 대기 join을 넣지 않는다.

```cpp
// 외부 publisher는 종료하지 않고 이 Client가 보유한 HANDLE만 해제한다.
if (m_pVehicleDebugPanel)
{
    m_pVehicleDebugPanel->Shutdown();
}
m_pVehicleDebugPanel.reset();
```

### G05-10. Client.vcxproj 등록 교체

파일: `C:/Users/tnest/Desktop/LostArk/Client/Default/Client.vcxproj`.
작업: 헤더 두 항목 추가. 기존 `ClInclude Include="..\Public\MainApp.h"` 바로 뒤에 넣는다.

```xml
<ClInclude Include="..\Public\VehicleDebugPanel.h" />
<ClInclude Include="..\Public\CameraDebugControls.h" />
```

작업: 기존 아래 CPP 항목 한 줄 전체 교체.

```xml
<ClCompile Include="..\Private\MainApp_Dragon.cpp" />
```

교체 블록:

```xml
<ClCompile Include="..\Private\VehicleDebugPanel.cpp">
  <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
  <PrecompiledHeader>NotUsing</PrecompiledHeader>
  <ForcedIncludeFiles />
</ClCompile>
<ClCompile Include="..\Private\CameraDebugControls.cpp">
  <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
  <PrecompiledHeader>NotUsing</PrecompiledHeader>
  <ForcedIncludeFiles />
</ClCompile>
```

새 UTF-8 CPP 두 개에만 `/utf-8`과 PCH 해제를 명시한다. 기존 MainApp과 전체 project의 charset은 바꾸지 않는다.
이 설정은 기존 CP949 헤더의 문자셋 경고를 없애는 설정이 아니다. 의존 헤더의 전역 인코딩 정리는 별도 범위다.

### G05-11. Client.vcxproj.filters 등록 교체

파일: `C:/Users/tnest/Desktop/LostArk/Client/Default/Client.vcxproj.filters`.
작업: 기존 MainApp.h의 ClInclude 닫는 태그 바로 아래에 다음 두 블록을 추가한다.
기존 `03. Tools`와 `03. Tools\08. Camera` 필터를 쓰므로 새 Filter/GUID는 만들지 않는다.

```xml
<ClInclude Include="..\Public\VehicleDebugPanel.h">
  <Filter>03. Tools</Filter>
</ClInclude>
<ClInclude Include="..\Public\CameraDebugControls.h">
  <Filter>03. Tools\08. Camera</Filter>
</ClInclude>
```

작업: 아래 기존 MainApp_Dragon CPP 블록 전체 삭제.

```xml
<ClCompile Include="..\Private\MainApp_Dragon.cpp">
  <Filter>00.MainApp</Filter>
</ClCompile>
```

그 자리에 다음 두 블록 추가:

```xml
<ClCompile Include="..\Private\VehicleDebugPanel.cpp">
  <Filter>03. Tools</Filter>
</ClCompile>
<ClCompile Include="..\Private\CameraDebugControls.cpp">
  <Filter>03. Tools\08. Camera</Filter>
</ClCompile>
```

### G05-12. 옛 MainApp_Dragon.cpp 삭제

파일: `C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_Dragon.cpp`.
작업: 파일 전체 삭제. 위 두 함수가 새 CPP로 옮겨지고 MainApp 선언·호출·project/filter가 모두 교체된 뒤 삭제한다.
현재 원본은 `out/MainAppRefactor/g05/baseline/Client/Private/MainApp_Dragon.cpp`에 보존했다.
이름만 바꾼 두 번째 CMainApp 구현 파일이나 DragonSpeedDraft static을 남기지 않는다.

## 유지하는 동작과 실패 경계

- 승하차는 유효한 비-preview player, 현재 controller, follow camera, presentation override 없음의 기존 네 조건을 유지한다.
  버튼 disabled와 실제 제출 양쪽에서 같은 조건을 사용한다. Server typed 요청과 검증은 바꾸지 않는다.
- flight camera 거리·pitch·높이·활성·heading은 현재 camera에만 적용한다. 저작 저장이나 Server 속도 갱신으로 설명하지 않는다.
- 세 속도는 기존 범위·finite 검사에 모두 통과해야 baseline과 draft를 함께 교체한다. 파일 읽기/검증 실패는 기존 값을 보존한다.
- Save와 Save + Publish는 같은 script·인자·precision·classic locale·baseline을 사용한다. 처리 중 중복 저장과 Reload는 기존처럼 막는다.
- Poll은 기존 exit-code 처리와 메시지 순서를 유지한다. exit 0 뒤 다시 Load한 결과와 성공 문구를 별도로 분리하는 개선은 포함하지 않는다.
- Resolve/파일 시스템·문자열 할당의 기존 예외 계약을 유지하며 Load/Start/Update/Render를 noexcept로 선언하지 않는다.
- Shutdown만 HANDLE 정리에 한정해 noexcept다. 외부 프로세스를 기다리거나 종료하지 않고 두 번째 호출에서 중복 CloseHandle을 하지 않는다.
- panel은 HANDLE을 중복 해제하지 않도록 복사·이동을 모두 금지한다. 주 스레드에서 생성·Update·Render·Free를 직렬 호출한다.
- Debug Dragon은 Debug에서만 생성·표시한다. 카메라 속도 UI는 Debug/Release 공통이며 Debug 아레나의 세션 속도 setter도 유지한다.

## 작성 후 검증

G03 → G04 → G05를 순서대로 병합하고 위 project/filter 교체를 적용한 다음 제품 Debug/Release를 확인한다.

```powershell
powershell -ExecutionPolicy Bypass -File "C:\Users\tnest\Desktop\LostArk\Tools\Build\Invoke-BuildAndRegression.ps1" -Configuration Debug -Profile Product
powershell -ExecutionPolicy Bypass -File "C:\Users\tnest\Desktop\LostArk\Tools\Build\Invoke-BuildAndRegression.ps1" -Configuration Release -Profile Product
```

G05 자체는 Client API만 바꾸지만 선행 G03에는 Engine 공개 API가 있으므로 공식 Engine/EngineSDK/Client 순서를 사용한다.
EngineSDK 복사본만 직접 고치지 않는다. XML parse, 신규 H/CPP 등록 각 한 번, 옛 TU 등록 0건,
MainApp의 두 옛 함수 선언·정의 0건, 후보와 최신 제품의 무관한 변경 보존을 확인한다.

Client/UI 실행은 사용자가 한다. Debug F1 Dragon에서 Save 후 F1을 닫아도 Update가 process HANDLE을 정리하는지,
창을 다시 열었을 때 draft가 유지되는지, follow/free와 presentation/preview 조건이 승하차 제출을 막는지 확인한다.
실제 저장·동시편집 충돌·게시 후 Server 적용과 종료 중 외부 publisher 지속은 실행으로 따로 확인한다.
카메라는 Debug/Release F1에서 20 m/s reset, F6/Shift, Character Select와 Debug 아레나 세션 유지 범위를 확인한다.
빌드 성공으로 저장·Server 반영·화면 성공을 대신 기록하지 않는다.

## 이번 문서의 검증 범위

후보 XML parse·신규 등록 각 한 번·옛 TU 제거·원본 및 선행 G04 후보 해시 보존을 확인했다.
제품 소스·SDK·DLL·Data·Resources는 수정하지 않았다. 후보의 문법 컴파일 결과는
`out/MainAppRefactor/g05/compile-results.json`과 아래 최종 검증 기록을 따른다.
실제 제품 링크, Debug/Release 화면, Save/Publish 실행과 Server 재시작 적용은 미검증이다.

## 전체 파일 코드

아래 여섯 H/CPP는 G05 적용 후 전체 코드다. 앞 절의 delta를 확인한 뒤 최신 작업본과 대조하는 정본으로 사용한다.
project/filter의 전체 후보는 `out/MainAppRefactor/g05/candidate/Client/Default`에 있고 정확한 교체 블록은 앞 절에 있다.

### 전체 1. Client/Public/VehicleDebugPanel.h

파일: `C:/Users/tnest/Desktop/LostArk/Client/Public/VehicleDebugPanel.h`.

변경 종류: 새 파일. Debug 전용 panel의 외부 계약과 단일 소유 상태를 선언한다.
Update는 MainApp 프레임, Render는 F1, Shutdown은 MainApp Free와 소멸자가 호출한다.
세 속도와 baseline은 m/s 단위의 별도 저작값이며 m_hProcess는 이 객체가 닫을 process HANDLE이다.
player/controller/camera는 선언만 필요하므로 전방 선언하며 멤버에 저장하지 않는다.
복사·이동 삭제는 HANDLE의 중복 CloseHandle을 막기 위한 수명 계약이다.

```cpp
#pragma once

#include "Engine_Macro.h"

#ifdef _DEBUG
// HANDLE은 패널이 소유한다. Client 공통 헤더의 WinSock2/Windows 포함 순서를 유지한다.
#include "Client_Defines.h"
#include "Engine_Typedef.h"
#include <string>

NS_BEGIN(Client)

class CCamera_Free;
class CPlayerController;
struct HUD_PLAYER_STATE;

// Developer Tools의 탈것 저작 상태를 보유한다. 실제 탈것 객체와 Server 이동 상태는 소유하지 않는다.
// MainApp이 한 번 준비해 숨겨진 동안도 보유하고, 종료 전에 Shutdown으로 OS handle을 정리한다.
class CVehicleDebugPanel final
{
public:
	CVehicleDebugPanel() = default;
	~CVehicleDebugPanel();
	CVehicleDebugPanel(const CVehicleDebugPanel&) = delete;
	CVehicleDebugPanel& operator=(const CVehicleDebugPanel&) = delete;
	CVehicleDebugPanel(CVehicleDebugPanel&&) = delete;
	CVehicleDebugPanel& operator=(CVehicleDebugPanel&&) = delete;

public:
	// 주 스레드의 MainApp Update가 창 표시와 무관하게 게시 프로세스 종료만 비차단 확인한다.
	void Update();
	// 세 인자는 이 호출 동안만 차용하며 멤버에 보관하지 않는다. camera 수명은 호출자의 shared_ptr가 유지한다.
	// player는 복제된 읽기 전용 상태다. 승하차는 controller의 기존 typed 요청으로만 제출한다.
	void Render(const HUD_PLAYER_STATE& player, CPlayerController* controller, CCamera_Free* camera);
	// 소유한 HANDLE만 닫고 null로 만든다. 실행 중인 외부 publisher를 중단하거나 기다리지 않는다.
	void Shutdown() noexcept;

private:
	// 첫 열기·명시적 Reload·게시 성공 뒤 호출한다. 세 값 검증 성공 때만 편집값과 baseline을 함께 바꾼다.
	void Load();
	// Save와 Save + Publish만 프로세스를 시작한다. baseline을 넘겨 기존 publisher의 동시 편집 검사를 사용한다.
	void Start(Engine::bool_t publish);
	// 완료한 프로세스 handle을 닫고 결과를 반영한다. 실패 시 기존 편집값을 유지한다.
	void Poll();

private:
	// 순서는 지상·수평 비행·수직 비행이며 단위는 m/s다. 복제된 runtime 속도와 별도인 저작 draft다.
	Engine::f32_t m_fSpeeds[3]{10.f, 16.f, 8.f};
	// 마지막 성공 Load의 값이다. Save는 이 baseline과 디스크의 같은 필드가 일치해야 한다.
	Engine::f32_t m_fBaseline[3]{10.f, 16.f, 8.f};
	Engine::bool_t m_bLoaded = false;
	// CreateProcessW의 process handle만 보유하며 thread handle은 시작 직후 닫는다.
	HANDLE m_hProcess = nullptr;
	std::string m_strStatus;
};

NS_END
#endif
```

### 전체 2. Client/Private/VehicleDebugPanel.cpp

파일: `C:/Users/tnest/Desktop/LostArk/Client/Private/VehicleDebugPanel.cpp`.

변경 종류: 새 파일. 기존 DragonSpeedDraft의 Load/Start/Poll과 Dragon UI 본문을 이 클래스의 함수로 옮긴다.
Load는 JSON 읽기 → vehicle 9523 조회 → 세 속도 검증 → baseline/draft 동시 교체 순서다.
Start는 loaded/진행 중 검사 → 기존 publisher 확인 → 기대 baseline과 새 값 명령행 → 창 없는 CreateProcess다.
Poll은 비차단 종료 확인 → handle 정리 → 성공 시 Load 또는 실패 메시지이며 Update가 호출한다.
Render는 접힘 확인 → 최초 Load → typed 승하차와 camera preview → 명시적 Save/Reload 순서다.
filesystem/fstream/iterator는 파일 로드, sstream/locale는 기존 명령행, cmath는 finite 검사에 필요하다.

```cpp
#include "VehicleDebugPanel.h"

#ifdef _DEBUG
// UI는 기존 player snapshot, 카메라 preview, typed controller 요청을 서로 구분한다.
#include "Camera_Free.h"
#include "CombatHUDViewModel.h"
#include "PlayerController.h"
#include "ProjectDataRoot.h"
#include "DataJson.h"
#include "imgui.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <locale>
#include <sstream>
#include <string>

NS_BEGIN(Client)

CVehicleDebugPanel::~CVehicleDebugPanel()
{
	Shutdown();
}

// 패널이 접히거나 F1 창이 닫혀도 MainApp이 호출한다. 렌더링 여부가 완료 처리를 막지 않는다.
void CVehicleDebugPanel::Update()
{
	Poll();
}

void CVehicleDebugPanel::Shutdown() noexcept
{
	if (nullptr != m_hProcess)
	{
		CloseHandle(m_hProcess);
		m_hProcess = nullptr;
	}
}

// 로드 중 실패하면 기존 편집값과 baseline을 유지한다. 파일과 JSON의 검증은 기존 경로를 그대로 쓴다.
void CVehicleDebugPanel::Load()
{
	const auto path = CProjectDataRoot::Resolve(L"Vehicles/VehicleProfiles.json");
	std::ifstream input(path, std::ios::binary);
	const std::string text((std::istreambuf_iterator<char>(input)), {});
	DATA_JSON_VALUE document;
	std::string error;
	if (!input || !CDataJson::Parse(text, document, error))
	{
		m_strStatus = "Cannot read VehicleProfiles.json: " + error;
		return;
	}
	// vehicleId 9523 행에서 지상 속도 override를 우선하고 flight의 수평·수직 속도를 함께 읽는다.
	const auto *vehicles = document.Find("vehicles");
	if (vehicles && vehicles->Is_Array())
	{
		for (const auto &vehicle : vehicles->Get_Array())
		{
			const auto *id = vehicle.Find("vehicleId");
			if (!id || !id->Is_Number() || id->Get_Number() != 9523)
			{
				continue;
			}
			const auto *ground = vehicle.Find("moveSpeedOverride");
			if (!ground)
			{
				ground = vehicle.Find("moveSpeed");
			}
			const auto *flight = vehicle.Find("flight");
			const auto *speed = flight ? flight->Find("speed") : nullptr;
			const auto *vertical = flight ? flight->Find("verticalSpeed") : nullptr;
			if (!ground || !speed || !vertical || !ground->Is_Number() || !speed->Is_Number() ||
				!vertical->Is_Number())
			{
				break;
			}
			const Engine::f32_t candidate[]{static_cast<Engine::f32_t>(ground->Get_Number()),
									static_cast<Engine::f32_t>(speed->Get_Number()),
									static_cast<Engine::f32_t>(vertical->Get_Number())};
			if (!std::isfinite(candidate[0]) || !std::isfinite(candidate[1]) || !std::isfinite(candidate[2]) ||
				candidate[0] <= 0.f || candidate[0] > 30.f || candidate[1] <= 0.f || candidate[1] > 60.f ||
				candidate[2] <= 0.f || candidate[2] > 60.f)
			{
				break;
			}
			for (int i = 0; i < 3; ++i)
			{
				m_fBaseline[i] = m_fSpeeds[i] = candidate[i];
			}
			m_bLoaded = true;
			m_strStatus = "Loaded authored speeds. Server values change after Publish and restart.";
			return;
		}
	}
	m_strStatus = "Ancient Sea speed profile is invalid; the previous draft is preserved.";
}

// publisher는 기존 Save/SaveAndPublish 정책으로 검증·병합·원자적 저장을 책임진다.
void CVehicleDebugPanel::Start(const Engine::bool_t publish)
{
	if (m_hProcess || !m_bLoaded)
	{
		return;
	}
	const auto root = CProjectDataRoot::Resolve(L"Vehicles/VehicleProfiles.json")
						  .parent_path()
						  .parent_path()
						  .parent_path();
	const auto script = root / L"Tools/GameplayPipeline/Publish-VehicleProfiles.ps1";
	if (!std::filesystem::is_regular_file(script))
	{
		m_strStatus = "Vehicle publisher is unavailable in this installation.";
		return;
	}
	// 소수점과 정밀도를 고정해 다른 로캘에서도 같은 속도 값과 expected 값을 비교하도록 한다.
	std::wostringstream command;
	command.imbue(std::locale::classic());
	command.precision(9);
	command << L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" << script.wstring() << L"\" -Mode "
			<< (publish ? L"SaveAndPublish" : L"Save") << L" -AncientSeaGroundSpeed " << m_fSpeeds[0]
			<< L" -AncientSeaFlightSpeed " << m_fSpeeds[1] << L" -AncientSeaVerticalSpeed " << m_fSpeeds[2]
			<< L" -ExpectedGroundSpeed " << m_fBaseline[0] << L" -ExpectedFlightSpeed " << m_fBaseline[1]
			<< L" -ExpectedVerticalSpeed " << m_fBaseline[2];
	auto args = command.str();
	STARTUPINFOW startup{};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION info{};
	if (!CreateProcessW(nullptr, args.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, root.c_str(),
						&startup, &info))
	{
		m_strStatus = "Could not start the vehicle publisher; no changes were made.";
		return;
	}
	CloseHandle(info.hThread);
	m_hProcess = info.hProcess;
	m_strStatus = publish ? "Saving and publishing dragon speeds..." : "Saving dragon speeds...";
}

// Update가 호출한다. 완료 코드가 성공이면 기존처럼 다시 Load하고 Server 재시작 필요를 표시한다.
void CVehicleDebugPanel::Poll()
{
	if (!m_hProcess || WaitForSingleObject(m_hProcess, 0) != WAIT_OBJECT_0)
	{
		return;
	}
	DWORD code = 1;
	GetExitCodeProcess(m_hProcess, &code);
	CloseHandle(m_hProcess);
	m_hProcess = nullptr;
	if (code == 0)
	{
		Load();
		m_strStatus = "Speed operation succeeded. Publish and restart Server before testing movement.";
	}
	else
	{
		m_strStatus =
			"Speed operation failed (validation or concurrent edit). Disk changes were preserved; reload the draft.";
	}
}

// 카메라 preview와 저작 Save는 즉시 적용 범위가 다르며 창을 숨겨도 이 객체의 draft는 남는다.
void CVehicleDebugPanel::Render(const HUD_PLAYER_STATE& player, CPlayerController* controller, CCamera_Free* camera)
{
	if (!ImGui::CollapsingHeader("Dragon"))
	{
		return;
	}
	if (!m_bLoaded && m_strStatus.empty())
	{
		Load();
	}
	// 추적 카메라이고 연출 override가 없을 때만 실제 승하차 명령을 허용한다.
	const Engine::bool_t cameraAcceptsGameplay = camera && camera->Is_FollowEnabled() && !camera->Is_PresentationOverrideActive();
	const Engine::bool_t canRequestRiding = controller && player.isValid && !player.isPreview && cameraAcceptsGameplay;
	ImGui::Text("Ancient Sea (9523) | flight phase: %u", static_cast<unsigned>(player.eVehicleFlightPhase));
	ImGui::BeginDisabled(!canRequestRiding);
	if (ImGui::Button(player.iVehicleId == 9523u ? "Dismount dragon" : "Mount Ancient Sea") && canRequestRiding)
	{
		controller->Request_VehicleRiding(player.iVehicleId == 9523u ? 0u : 9523u);
	}
	ImGui::EndDisabled();
	if (!cameraAcceptsGameplay)
	{
		ImGui::TextDisabled("Mount / dismount requires the follow camera (F6).");
	}
	ImGui::TextWrapped(
		"E: takeoff / land. WASD: fly, Space / Ctrl: ascend / descend. Landing requires walkable ground below; ground mount keeps the character view.");
	// 아래 구도 조절은 현재 Client 카메라만 바꾸며 Server 이동 속도나 저작 profile을 저장하지 않는다.
	if (camera)
	{
		Engine::f32_t distance, pitch, height;
		Engine::bool_t enabled, heading;
		camera->Get_DragonCameraSettings(distance, pitch, height, enabled, heading);
		Engine::bool_t changed = ImGui::Checkbox("Flight camera", &enabled);
		changed |= ImGui::Checkbox("Follow dragon heading", &heading);
		changed |=
			ImGui::DragFloat("Flight distance (m)", &distance, .1f, 4.f, 80.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		changed |=
			ImGui::DragFloat("Flight pitch (degrees)", &pitch, .2f, -8.f, 75.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
		changed |=
			ImGui::DragFloat("Flight look height (m)", &height, .05f, 0.f, 15.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::Button("Reset flight camera"))
		{
			distance = 16.f;
			pitch = 24.f;
			height = 2.4f;
			enabled = heading = changed = true;
		}
		if (changed)
		{
			camera->Set_DragonCameraSettings(distance, pitch, height, enabled, heading);
		}
		ImGui::TextDisabled(
			"Camera edits apply immediately during flight; left drag orbits. F6 keeps its normal meaning.");
	}
	// 속도 draft는 사용자 Save 동작으로만 publisher에 전달한다. 진행 중에는 중복 저장·재로드를 제한한다.
	ImGui::SeparatorText("Server speed authoring");
	ImGui::BeginDisabled(m_hProcess != nullptr || !m_bLoaded);
	ImGui::DragFloat("Ground speed (m/s)", &m_fSpeeds[0], .1f, .1f, 30.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Flight speed (m/s)", &m_fSpeeds[1], .1f, .1f, 60.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Vertical speed (m/s)", &m_fSpeeds[2], .1f, .1f, 60.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	if (ImGui::Button("Save dragon speeds"))
	{
		Start(false);
	}
	ImGui::SameLine();
	if (ImGui::Button("Save + Publish dragon speeds"))
	{
		Start(true);
	}
	ImGui::EndDisabled();
	ImGui::BeginDisabled(m_hProcess != nullptr);
	if (ImGui::Button("Reload dragon speeds"))
	{
		Load();
	}
	ImGui::EndDisabled();
	ImGui::TextWrapped("%s", m_strStatus.c_str());
}

NS_END
#endif
```

### 전체 3. Client/Public/CameraDebugControls.h

파일: `C:/Users/tnest/Desktop/LostArk/Client/Public/CameraDebugControls.h`.

변경 종류: 새 파일. 자체 상태가 없으므로 별도 Manager나 객체 수명을 만들지 않는다.
Find_ActiveDebugCamera는 Level/Layer가 소유한 camera의 shared_ptr를 잠시 반환하며 Render_CameraSpeedControls가 이를 사용한다.
MainApp도 panel 호출 동안 같은 helper로 얻은 camera를 유지한다. memory와 CCamera_Free 전방 선언이면 H 계약을 표현할 수 있다.

```cpp
#pragma once

#include "Engine_Macro.h"
#include <memory>

NS_BEGIN(Client)

class CCamera_Free;

// Character Select의 전용 camera 또는 현재 Level의 camera layer를 조회한다.
// 반환 shared_ptr는 짧은 UI 호출 동안만 수명을 유지한다. 장기 보관하거나 Level 전환 후 재사용하지 않는다.
std::shared_ptr<CCamera_Free> Find_ActiveDebugCamera();

// Debug와 Release의 F1 속도 UI다. 현재 camera/Level의 기존 setter만 사용하며 자체 상태를 보유하지 않는다.
void Render_CameraSpeedControls();

NS_END
```

### 전체 4. Client/Private/CameraDebugControls.cpp

파일: `C:/Users/tnest/Desktop/LostArk/Client/Private/CameraDebugControls.cpp`.

변경 종류: 새 파일. MainApp에 의존하지 않고 기존 GameInstance와 Level 클래스만 조회한다.
Character Select는 Get_DebugCamera, 그 외는 현재 Layer_Camera의 첫 객체를 사용한다.
UI 변경은 Debug 아레나의 세션 setter 또는 현재 camera의 Set_FreeMoveSpeed로 전달한다.
자유 카메라 속도·Shift 배수·reset 값·노출 조건과 저장 범위를 유지한다.

```cpp
#include "CameraDebugControls.h"

// Client 공통 헤더가 WinSock2를 Windows 선언보다 먼저 포함한다.
#include "Client_Defines.h"
#include "Engine_Typedef.h"
#include "Camera_Free.h"
#include "GameInstance.h"
#include "Level_CharacterSelect.h"
#include "imgui.h"
#ifdef _DEBUG
// Debug 아레나만 기존 Level의 세션 속도 저장 setter를 사용한다.
#include "Level_ValtanArena.h"
#include "Level_KoukuSaydonArena.h"
#endif

#include <memory>

NS_BEGIN(Client)

// 실제 Level/Layer가 camera를 소유한다. MainApp이나 별도 camera singleton에 의존하지 않는다.
std::shared_ptr<CCamera_Free> Find_ActiveDebugCamera()
{
	auto &game = Engine::CGameInstance::Get();
	if (game.Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
	{
		const auto *level = CLevel_CharacterSelect::Get_Active();
		return level ? level->Get_DebugCamera() : nullptr;
	}
	return std::dynamic_pointer_cast<CCamera_Free>(
		game.Get_GameObject(game.Get_CurrentLevelID(), L"Layer_Camera", 0));
}

// 아레나의 세션 값과 다른 맵의 인스턴스 값이라는 기존 저장 범위를 유지한다.
void Render_CameraSpeedControls()
{
	const auto camera = Find_ActiveDebugCamera();
	const auto level = static_cast<LEVEL>(Engine::CGameInstance::Get().Get_CurrentLevelID());
	const Engine::bool_t characterSelect = level == LEVEL::CHARACTER_SELECT;
	if (!camera || !ImGui::CollapsingHeader(characterSelect ? "Character Select Camera" : "Camera",
											characterSelect ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None))
	{
		return;
	}
	const auto setSpeed = [&](Engine::f32_t value)
	{
#ifdef _DEBUG
		if (level == LEVEL::VALTAN_ARENA && CLevel_ValtanArena::Get_Active())
		{
			CLevel_ValtanArena::Get_Active()->Set_DebugCameraSpeed(value);
			return;
		}
		if (level == LEVEL::KOUKUSAYDON_ARENA && CLevel_KoukuSaydonArena::Get_Active())
		{
			CLevel_KoukuSaydonArena::Get_Active()->Set_DebugCameraSpeed(value);
			return;
		}
#endif
		camera->Set_FreeMoveSpeed(value);
	};
	Engine::f32_t speed = camera->Get_FreeMoveSpeed();
	if (ImGui::DragFloat("Free camera speed (m/s)", &speed, .5f, CCamera_Free::MIN_FREE_MOVE_SPEED,
						 CCamera_Free::MAX_FREE_MOVE_SPEED, "%.1f", ImGuiSliderFlags_AlwaysClamp))
	{
		setSpeed(speed);
	}
	if (ImGui::Button("Reset speed to 20 m/s"))
	{
		setSpeed(20.f);
	}
	ImGui::TextDisabled("F6: Follow / Free. Shift: x%.0f.", CCamera_Free::FREE_MOVE_SPRINT_MULTIPLIER);
	if (characterSelect)
	{
		ImGui::TextDisabled("Applies to F6 / Movie free-camera movement; Movie playback timing stays authored.");
	}
#ifdef _DEBUG
	if (level == LEVEL::VALTAN_ARENA || level == LEVEL::KOUKUSAYDON_ARENA)
	{
		ImGui::TextDisabled("Arena speed is retained for this Client session.");
	}
	else
#endif
	{
		ImGui::TextDisabled("Speed applies during this map visit.");
	}
}

NS_END
```

### 전체 5. Client/Public/MainApp.h

파일: `C:/Users/tnest/Desktop/LostArk/Client/Public/MainApp.h`.

변경 종류: G04 후보 위의 기존 파일 반영 후보 전체.
이번 G의 delta는 panel 전방 선언과 unique_ptr 소유 멤버 추가, 카메라/Dragon 기존 멤버 함수 선언 삭제다.
panel의 완전한 타입은 CPP가 포함하므로 H에서 구체 저작 구현을 의존하지 않는다.

```cpp
#pragma once

// Client 이름 공간·LEVEL·공통 타입을 사용한다.
#include "Client_Defines.h"
// 장면 렌더링·품질 draft와 행렬·광원 값을 멤버로 보관한다.
#include "Engine_RenderTypes.h"
// D3D COM 참조·HRESULT·컨테이너 등의 공통 선언을 사용한다.
#include "Engine_Defines.h"
// Loading 전환과 로비 승인 흐름을 연결하는 명령 토큰을 사용한다.
#include "LobbyCommandService.h"
// Server 요청과 복제 식별자·장비·전투 enum을 원래 typed 값으로 보관한다.
#include "Network/PacketMessages.h"
// 장면/품질 프로필의 조회·활성화 서비스 수명을 소유한다.
#include "RenderingProfileService.h"
// 재사용 광원 정의와 미리보기용 catalog를 소유한다.
#include "LightResourceCatalog.h"
// Area 조명 draft·저장·게시의 기존 세션을 값으로 소유한다.
#include "AreaLightAuthoringSession.h"
// Server 복제 상태를 읽는 HUD 타입과 Debug preview 값을 사용한다.
#include "CombatHUDViewModel.h"

// 고정 개수의 창 표시·slot·아레나 조회 상태를 보관한다.
#include <array>
// 요청 timeout·프레임 제한·미리보기 경과 시간을 steady_clock으로 측정한다.
#include <chrono>
// 이 헤더에서는 직접 쓰지 않지만 MainApp.cpp의 파일 경로 처리가 간접 포함에 의존한다.
#include <filesystem>

NS_BEGIN(Engine)
class CImGuiLayer;
NS_END

NS_BEGIN(Client)

// MainApp이 보관하거나 연결할 Client UI·도구의 타입들이다. 실제 생성과 연결은 MainApp.cpp에서 한다.
class CMapTool;
class CLevelNavigationDebug;
class CEffect_Tool;
class CEffect_Tool_V2;
class CAnimation_Tool;
class CValtanActionWorkbench;
class CKoukuSaydonActionWorkbench;
class CKoukuSaydonPresentationPlayer;
class CHUDLayoutTool;
class CUILayoutRuntime;
class CDungeonTimerView;
class CWorldHealthBarView;
class CBalanceTool;
class CValtanBossTool;
class CKoukuSaydonBossTool;
class CCameraTool;
#ifdef _DEBUG
class CVehicleDebugPanel;
#endif
class CCharacterPreviewPanel;
class CEquipmentAuthoringTool;
class CProfilerTool;
class CSequencerTool;
class CCharacterActionWorkbench;
class CWorldObjectTool;
class CWorldLevelTool;
class CWorldSceneTool;
class CGuideAITool;
class CMaharakaAITool;
struct WORLD_LEVEL_TOOL_REQUEST;
struct KOUKU_MAP_EFFECT_PLACEMENT_REQUEST;
struct KOUKU_SAYDON_COMPOSITION_DOCUMENT;
struct KOUKU_SAYDON_COMPOSITION_PATTERN;
class CWorldSequenceDocument;
class CRenderingBenchmark;
class CSkillWindowView;
class CInventoryView;
class CRepairWindowView;
class CShopWindowView;
class CDurabilityHudView;
class CCombatAnalysisFrameView;
class CCharacterInfoWindowView;
class CCharacterPortraitRenderer;
class CAvatarBookWindowView;
class CVehicleWindowView;
class CHonorTitleWindowView;
class CWorldMapWindowView;
class CSystemOptionWindowView;
class CSongCastGaugeView;
class CQuickSlotDragView;
class CPlayerController;
class CChatWindowView;
class CPartyWindowView;
class CCharacterSelectWindowView;
class CMinimapView;
class CEstherCutinPresentationService;

// Client.cpp가 생성·해제하며 제품 UI·도구와 프레임 순서, Level 전환 요청을 조정한다.
// Engine의 manager는 CGameInstance가 소유한다. CObject_Manager → CLayer → CGameObject → CComponent로 실제 객체를 관리한다.
// MainApp은 UI/도구 객체와 표시·요청 대기 상태를 보관하고 게임 객체 등록·갱신·렌더는 Engine 경로로 요청한다.
class CMainApp final
{
private:
	// 도구별 표시 상태와 단일 viewport 입력 소유자를 같은 index 집합으로 연결한다.
	enum class DEBUG_TOOL
	{
		NONE,
		MAP,
		VALTAN_ACTION_WORKBENCH,
		KOUKU_SAYDON_ACTION_WORKBENCH,
		ANIMATION,
		EFFECT,
		// V2 문서와 미리보기 창은 기존 도구와 독립적으로 관리한다.
		EFFECT_V2,
		RENDERING,
		UI,
		BALANCE,
		VALTAN_BOSS,
		KOUKU_SAYDON_BOSS,
		VALTAN_LOGIC_PATTERN,
		CAMERA,
		EQUIPMENT,
		SEQUENCER,
		SEQUENCER_BENCHMARK,
		PROFILER,
		WORLD_OBJECT,
		WORLD_LEVEL,
		WORLD_SCENE,
		GUIDE_AI,
		MAHARAKA_AI,
		EFFECT_COMPOSITION,
		COUNT
	};

	// 자원 검색 행의 경로·검색어와 열 도구를 보관한다.
	struct DEBUG_RESOURCE_FILE
	{
		string strDomain;
		string strSource;
		string strRelativePath;
		string strSearchText;
		DEBUG_TOOL eTool = DEBUG_TOOL::NONE;
	};

private:
	// Client.cpp의 Create 경로에서 현재 앱을 등록한다.
	CMainApp();

public:
	// 현재 앱 주소를 비운 뒤 Free로 도구·네트워크·Engine 종료 순서를 진행한다.
	~CMainApp();

public:
	// Engine과 공용 원형·UI를 준비하고 Lobby로 가는 Loading을 시작한다.
	HRESULT Initialize();
	// Client.cpp가 전달한 delta(초)로 입력·Level·UI를 갱신하고 Level Update 뒤 전환 요청을 처리한다.
	void Update(f32_t fTimeDelta);
	// Engine의 월드/UI 렌더와 도구 표시를 연결한다. 제품 글자는 UI 합성 뒤에 그린다.
	HRESULT Render();
	// Client.cpp의 프레임 끝에서 전경/배경 FPS 제한을 맞춘다. 제한이 없으면 기준 시각만 갱신한다.
	void Limit_FrameRate();

	// Level/NPC가 MainApp 소유 창을 열 때 현재 인스턴스를 차용한다. 생성 시 등록하고 소멸 전에 비운다.
	static CMainApp* Get_Active()
	{
		return s_pActiveInstance;
	}
	// 베른 슈미트 NPC가 재련 창을 연다. 새로 열 때만 대기 상태를 초기화한다.
	void Open_ItemUpgradeWindow();
	// 베른 선박 NPC가 탈것 창을 선박 전용 모드로 연다.
	void Open_ShipWindow();
	// 베른 수리 NPC가 MainApp 소유 수리 창을 연다.
	void Open_RepairWindow();
	// 베른 물약 상인의 placement ID로 상점의 상품과 요청 대상을 정한다.
	void Open_ShopWindow(const string& strNpcPlacementId);
	// 월드 하단 메뉴 버튼이 Escape와 같은 환경설정 창을 연다. 이미 열려 있으면 유지한다.
	void Open_SystemOptionsWindow();
	// 활성 캐릭터가 있으면 Server의 상태 capture를 요청하고 이후 Pump가 로비 복귀를 마무리한다.
	// false는 capture 대기 시작과 예약·전송 실패를 모두 포함한다. true는 로비 LOAD 요청을 예약하고 연결을 닫은 경우다.
	bool_t Return_ToCharacterSelect();
	// Update가 capture 완료·연결 세대·timeout을 확인한다. 실패하면 슬롯을 다시 덮어쓰지 않고 대기를 정리하며 진단을 기록한다.
	// Apply_CaptureResult가 이미 반영한 뒤 생긴 전환 실패까지 로스터에서 되돌리는 함수는 아니다.
	void Pump_ReturnToCharacterSelect();

	// Debug 제목의 FPS를 최소 500ms 간격으로 갱신한다. 기본 제목이 바뀌면 간격을 기다리지 않고 반영한다.
	static void Update_DebugWindowTitleWithFps(const wchar_t* pBaseTitle);
	// 각 도구의 Complete Play 선택을 공용 stable Pattern ID에 모은다. 도구별 편집 draft는 유지한다.
	bool_t Debug_SelectCompletePlayPattern(const std::string& strPatternId);
	const std::string& Debug_GetSelectedCompletePlayPatternId() const;
	// 선택한 패턴을 CValtanBossTool → CValtanPatternAuditionService의 Server 재생 경로로 요청한다.
	bool_t Debug_CompletePlaySelected(std::string& strOutStatus);
    // Map Tool 편집 전에 World Scene의 placement 편집 해제를 요청한다. 실패하면 그 도구의 draft를 보존한다.
    bool Debug_PrepareMapToolAuthoring(std::string& status);
    // 같은 Area의 Object Details 편집 전에 Map Tool의 미저장 변경과 preview 복원을 확인한다. 거부 이유는 status로 전달한다.
    bool Debug_PrepareWorldPlacementEditing(const std::string& areaId, std::string& status);
    // 현재 live Area의 stable placement를 공용 상세 창에 넘긴다. 준비·선택 실패 시 이전 창 표시 상태 복원을 시도한다.
    bool Debug_OpenWorldObjectDetails(const std::string& areaId, uint64_t placementId,
        bool deploy, std::string& status);
	// 통합 composition 도구의 요청을 Valtan Boss Tool의 Flow 편집 창에 연결한다.
	bool_t Debug_OpenValtanPatternFlow(std::string& strOutStatus);

private:
	// Update가 자원 준비→START 응답→참가자 READY→Server 시각 연출을 진행한다.
	// 연결·world generation 변경은 이전 요청 추적을 비우며, 준비 실패와 Server 응답 지연을 구분해 기록한다.
	void UpdateKoukuGateCompletePlay();
    // 쿠크 Complete Play의 로컬 준비·READY 전송·Server 수신 상태를 표시한다.
    void RenderKoukuRaidPreparationDiagnostics();
    // 현재 run epoch의 준비·전송 진단이다. observedAt은 로컬 표시용 시각이다.
    struct KOUKU_RAID_PREPARATION_DIAGNOSTICS final
    {
        uint32_t epoch = 0u;
        std::string stage, detail, submission;
        bool failed = false;
        std::chrono::steady_clock::time_point observedAt{};
    };
    KOUKU_RAID_PREPARATION_DIAGNOSTICS m_KoukuRaidPreparationDiagnostics;
	// Update와 도구 화면이 공유하는 관문 실행 상태다. Sequence 복사본은 document epoch가 바뀌면 다시 읽는다.
	string m_strKoukuCompletePlayFlowGate;
	std::unique_ptr<KOUKU_SAYDON_COMPOSITION_DOCUMENT> m_pKoukuRaidSequenceDocument;
	// 전송한 START/STOP 원문과 대기 순번을 보존해 정확한 응답에 대응한다. 다음 순번이 0이 되면 새 요청을 거부한다.
	LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST m_KoukuRaidRequest;
	std::uint32_t m_iNextKoukuRaidRequest = 1u, m_iKoukuRaidPendingRequest = 0u, m_iKoukuRaidDocumentEpoch = 0u;
	// run·문서·패턴·시작 tick의 키로 이미 시작한 연출과 실패한 연출을 구분한다. 같은 실패 키의 재시작을 막는다.
	std::string m_strKoukuRaidPresentationKey, m_strKoukuRaidFailedKey;
	// START/STOP 전송 후 5초가 지나면 지연 안내만 기록한다. timeout으로 대기 순번을 지우거나 두 번째 START를 허용하지 않는다.
	std::chrono::steady_clock::time_point m_KoukuRaidReplyDeadline{};
	std::string m_strKoukuRaidReplyStatus; // broadcast 레이드 표시와 별도로 보관하는 최신 미해결·거절 요청 상태다.
    // START를 보내기 전의 임시 준비다. 요청의 revision과 대상 패턴을 값으로 고정하고 연결 세대·20분 제한을 Update에서 재검사한다.
    // 실패·취소 시 이 준비를 폐기한다. 서버에 이미 전송한 요청은 위 pending 순번으로 따로 추적한다.
    struct KOUKU_RAID_RESOURCE_PREPARATION final
    {
        LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
        std::vector<std::string> patternIds;
        uint64_t worldGeneration = 0u;
        std::chrono::steady_clock::time_point deadline;
    };
    std::optional<KOUKU_RAID_RESOURCE_PREPARATION> m_KoukuRaidResourcePreparation;
    // Server PREPARING의 run epoch별 게시 패턴 목록이다. 같은 실행의 자원 준비를 이어갈 때 목록을 재사용한다.
    uint32_t m_iKoukuRaidResourceEpoch = 0u;
    std::vector<std::string> m_KoukuRaidResourcePatternIds;

	// START/STOP 응답을 기다리는 중 요청된 중지는 응답을 받은 뒤 처리한다. 로컬 preview 중지 예약과 Server 실행 epoch를 구분한다.
	bool_t m_bKoukuRaidStopAfterAdmission = false;
	bool_t m_bKoukuLocalPreviewStopRequested = false;
	uint64_t m_iKoukuCompletePlayWorldGeneration = 0u;
	// 이 Client가 READY 또는 FAILED 제출에 성공한 run epoch다. Server의 READY 확인 여부는 수신한 ready mask로 별도 판정한다.
	std::uint32_t m_iKoukuRaidAcknowledgedEpoch = 0u;
	string m_strKoukuCompletePlayStatus =
		"Load the KoukuSaydon inventory, pick a gate and a saved pattern, then Complete Play.";

	// Initialize가 제품 폰트를 등록한다. 필수 폰트 실패만 시작을 막는다.
	HRESULT Ready_Fonts();
	// Level을 넘어 쓸 셰이더·quad·Effect·UI 원형을 Engine의 STATIC 저장소에 등록한다.
	HRESULT Ready_Prototype_For_Static();
	// Loading 화면 자원을 STATIC 원형으로 등록한다. 문서 누락은 건너뛰고 등록 실패는 반환한다.
	HRESULT Ready_Prototype_For_LoadingChrome();
	// 목적지 descriptor와 profile을 검사한 뒤 Loading을 생성한다. 교체 실패 시 이전 profile 복원을 시도하며 실패 진단을 보존한다.
	HRESULT Start_Level(
		LEVEL eTargetLevel,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	// 프레임 경계에서 비동기 준비의 취소 완료를 기다린 뒤 LOAD/ACTIVATE 요청을 한 번 소비한다.
	// 활성화 실패는 생성 자원을 정리하고 연결을 닫은 뒤 기존 서비스로 로비 복구를 예약한다.
	void Apply_LevelRequest();
	// MainApp 소유 ImGui layer를 장치·창에 연결하고 실행 모드별 UI 설정을 적용한다.
	HRESULT ReadyImGuiRuntime();
	// 제품 단축키·프레임 제한·F7에서 전경 창이 이 process에 속하는지 확인한다.
	static bool_t IsWindowOwnedByCurrentProcess(HWND hWnd);
	// Update가 비동기 저장 진행을 확인하고 F7 상승 edge로 Profiler 창을 토글한다. 텍스트 입력 중에는 단축키를 소비하지 않는다.
	void UpdateProfilerRuntime();

	//I/P/N/M 키로 열고 닫기
	void Update_RuntimeWindowShortcuts();

	//UI가 숨겨진 동안 네 단축키의 눌림 상태만 갱신한다.
	void Sync_RuntimeWindowShortcutKeys(bool_t bFocused);

	// Update가 cinematic 표시 조건을 UI router에 맞추고, 억제가 시작되면 창 gesture와 운반 아이콘을 취소한다.
	void Sync_CinematicUI();
	// 현재 연출의 시각·배우 위치로 자막을 표시한다. 수집한 월드 자막은 instanceId·subtitleTrackId 쌍으로 같은 프레임의 중복을 제거한다.
	void RenderCinematicSubtitles();

	// 복제 상태를 HUD_Layout의 STATIC sprite에 반영한다. 표시 대상이 아니면 남아 있는 슬롯을 명시적으로 숨긴다.
	void Update_CombatHUD(f32_t fTimeDelta);
	// m_pHUDRuntimeView의 슬롯만 숨긴다. 보스·에스더 등 별도 view는 각 Hide 함수가 맡는다.
	void Hide_CombatHUD();
	// HealthBar/ManaBar는 빈 배경을 유지하고 별도 Fill 슬롯의 g_FillRatio로 현재/최대 HP·resource 비율을 표시한다.
	void Update_PlayerHealthManaBar();
	// 등록된 아이템을 Item_1..4_Icon에 표시한다. 인벤토리 drop의 화면 위치를 실제 viewport 기준 슬롯 rect와 비교한다.
	void Update_ItemQuickSlots();
	// UI 합성 뒤 스킬·특수·아이템 슬롯 그림 위에 단축키 글자를 그린다.
	void RenderQuickSlotKeyLabels();
	// 해양 HUD의 보급량·노트 속도와 슬롯·버튼 단축키 글자를 표시한다.
	void RenderShipHudTexts();
	// 쿠크 HUD 모드가 있으면 class identity와 T/V를 숨기고 Q..F에 모드 아이콘·쿨다운을 덮어쓴다.
	// 기본 스킬 아이콘·쿨다운 갱신 뒤 실행하며 모드가 없으면 추가 문양 슬롯을 숨긴다.
	void Update_KoukuHudMode();
	// 탑승 HUD가 class identity 대신 탈것 문양과 전용 스킬을 표시한다.
	// 쿠크 HUD 갱신 뒤 실행하며 미탑승 상태의 문양은 기본 ownerClass 갱신에서 숨긴다.
	void Update_VehicleHud();
	// Server가 마하라카 물총 장착을 알리면 줍기 문양과 Q/W/E/R 물총 스킬·복제된 쿨다운을 표시한다.
	// 탈것 HUD 다음에 실행하고 class identity·T/V를 숨기며 A/S/D/F는 빈 어두운 슬롯으로 둔다.
	void Update_WaterGunHud();
	// 인벤토리·탈것 창이 시작한 아이콘 운반을 소비한다. 아이템은 Item_1..4, 탈것은 SpecialSkill_1..6에 drop하며 다른 곳은 놓는다.
	// 창 Update 다음에 실행해 같은 프레임의 선택을 읽는다.
	void Update_QuickSlotDrag();
	// SpecialSkill_1..6_Icon에 5/6/7/8/9/0으로 등록한 탈것을 표시한다.
	void Update_SpecialQuickSlots();
	// Space 이동 스킬 또는 탑승 탈것의 Space 돌진 쿨다운을 문양 위에 표시한다.
	// 쿨다운이 없거나 쿠크 상호작용 모드이면 숨기며 탈것 HUD 다음에 갱신한다.
	void Update_SpecialSlot();
	// 복제된 탈것·방어 태세·침묵·속박·공포 등의 상태로 buff/debuff 줄을 구성한다.
	// 아이콘 정의는 HudBuffSources.json, 남은 시간 글자는 RenderSkillCooldownText가 담당한다.
	void Update_BuffBar();
	// SkillSlotMarks.json의 combo/holding/perfectCombo 표시와 Server combo 입력 시간의 원형 표시를 갱신한다.
	// 탑승 또는 쿠크 모드에서는 숨긴다.
	void Update_SkillSlotMarks();
	// SkillSlotMarks.json과 HudBuffSources.json을 읽는다. 파일 누락·오류는 해당 표시를 비운 상태로 처리한다.
	void Load_HudQuickSlotData();
	// class quick slot 밖 스킬의 남은 쿨다운 비율을 catalog cooldownMs로 계산한다. 0은 준비 완료이며 슬롯 위 초 단위 글자도 예약한다.
	f32_t Resolve_HudCooldownRatio(const HUD_PLAYER_STATE& player, uint32_t iSkillId,
		uint32_t iCooldownMs, const string& strTextSlotId);
	// 현재 Level의 플레이어 controller를 차용한다. 탈것 요청은 이 controller를 거치며 없는 Level이면 nullptr이다.
	CPlayerController* Find_ActivePlayerController() const;
	// KoukuHudModes.json의 모드 정의를 읽는다. 누락·오류면 빈 목록을 유지해 임의 모드를 표시하지 않는다.
	void Load_KoukuHudModes();
	// LOBBY에서 배경·로고·서버 목록과 접속(서버 선택), 종료/뒤로/환경설정 버튼을 갱신한다. 접속은 캐릭터 선택 창을 연다.
	void Update_LobbyButtons(f32_t fTimeDelta);
	// 로비의 캐릭터 선택 창을 갱신하고 생성 의도를 CHARACTER_SELECT 명령으로 연결한다. 열린 동안 로비 버튼 입력을 막는다.
	void Update_CharacterSelectWindow(f32_t fTimeDelta);
	// 프레임 시작에 다른 runtime 창이 없을 때만 Escape로 환경설정을 연다.
	// 같은 Escape로 다른 창을 닫으며 환경설정까지 열지 않고, 영상 옵션 변경 후 장면 프로필을 다시 활성화한다.
	void Update_SystemOptionWindow(f32_t fTimeDelta);
	bool_t Is_AnyRuntimeWindowOpen() const;
	// Loading 진입 때 runtime 창을 닫는다. STATIC sprite만 숨기고 열린 상태를 남겨 글자가 Loading 위에 그려지는 일을 막는다.
	void Close_RuntimeWindowsForLoading();
	// 미니게임·시네마틱·Loading의 HUD 숨김 조건을 조회한다. 제품 sprite와 텍스트가 같은 화면 점유 판정을 사용한다.
	bool_t Is_KoukuMinigameHUDHidden() const;
	bool_t Is_KoukuBossHealthBarHidden() const;
	bool_t Is_RuntimeUIScreenSuppressed() const;
	// Escape 한 번에 가장 최근에 연 창 하나를 닫기 위한 창 종류다.
	enum class ESCAPE_WINDOW : uint8_t
	{
		INVENTORY, CHARACTER_INFO, AVATAR_BOOK, HONOR_TITLE, VEHICLE, WORLD_MAP, REPAIR, SHOP, END
	};
	bool_t Is_EscapeWindowOpen(ESCAPE_WINDOW eWindow) const;
	// 닫힌 창은 순서에서 빼고 새로 열린 창을 맨 뒤에 추가한다.
	void Sync_EscapeWindowOrder();
	// 가장 최근에 연 창 하나를 닫는다. 열린 창이 없으면 false다.
	bool_t Close_TopEscapeWindow();
	// 이 프레임의 Escape를 나중에 직접 소비할 팝업·아이콘 운반 상태가 있는지 확인한다.
	bool_t Is_EscapeOwnedElsewhere() const;
	// Update 끝에서 창 rect를 텍스트 가림 영역으로 등록해 뒤의 이름표·HUD·말풍선이 창을 뚫고 보이지 않게 한다.
	// 표시 중인 UI의 텍스트 layer를 CUITextOcclusion에 함께 등록한다.
	void Register_UITextOccluders();
	// 서버 선택 패널의 제목·행·버튼·저작권과 Release 상태 글자를 EndFrame 이후 표시한다.
	void RenderLobbyButtonText();
	// LobbyServers.json을 서버 목록에 읽는다. 누락·오류면 행을 만들지 않고 접속 버튼을 비활성화한다.
	void Load_LobbyServers();
	// 게임 시작·서버 선택·카드 라벨은 RenderLobbyButtonText와 같이 EndFrame 뒤에 그려 UI 그림 위에 표시한다.
	void RenderCharacterSelectWindowText();
	// 현재 Level이 제공하는 복제 marker snapshot으로 우측 상단 미니맵을 갱신한다. 지원하지 않는 Level에서는 숨긴다.
	void Update_Minimap(f32_t fTimeDelta);
	void RenderMinimapText();
	// UI 합성 뒤 빈 버튼 그림 위에 흰색 "장비 재련"을 그린다.
	void RenderItemUpgradeButtonText();
	void RenderItemUpgradeLevelText();
	void RenderItemUpgradeMaterialCounts();
	void RenderItemUpgradeGaugePercentText();
	// WAITING에서 대기 원 위에 "화면을 클릭하여 결과 즉시 확인"을 표시한다.
	void RenderItemUpgradeResultWaitText();
	// SUCCESS frame 위에 아이템 이름·등급·"재련 성공"을 표시한다.
	void RenderItemUpgradeSuccessDetailText();
	// FAIL frame 위에 아이템 이름·"재련 실패"를 표시한다. 강화 단계는 그대로다.
	void RenderItemUpgradeFailDetailText();
	void RenderItemUpgradeListText();
	// UIInputRouter 입력으로 장비 선택·성장·재련 결과 화면을 갱신한다. 닫힘/미지원 Level에서는 STATIC 슬롯을 숨긴다.
	void Update_ItemUpgrade(f32_t fTimeDelta);
	void Update_CustomizingSceneProfile();
	void Update_ClassShowcaseShadowFocus();
	void Update_KoukuGateSceneProfile();
	void Hide_ItemUpgrade();
	void Update_ItemUpgradeSelection();
	// 성장 버튼을 누르면 gauge를 0부터 다시 채우고 CoreFlash를 한 번 시작한다.
	// 100%가 될 때까지 WingedRingGold·LevelUpMotion2Big·CompleteEffect는 숨긴다.
	void Update_ItemUpgradeGrowButton();
	// 선택한 인벤토리 identity와 기대 강화 단계를 Server 재련 요청에 담는다.
	void Update_ItemUpgradeReforgeButton();
	void Update_ItemUpgradeServerResult();
	// 요청과 일치하는 Server 결과가 도착해야 클릭으로 결과를 공개한다. 응답 대기 중에는 성공을 만들어 내지 않는다.
	void Update_ItemUpgradeResultWaitClick();
	// 표시 중인 성공/실패 확인 버튼을 누르면 결과 modal을 닫고 gauge를 대기 상태 0%로 되돌린다.
	void Update_ItemUpgradeResultOkButton();
	// 재련 요청/결과 닫기 때 gauge를 대기 0%로 돌려 이전 완료 표시를 지운다.
	void Reset_ItemUpgradeIdleGauge();
	// 결과 modal 뒤의 기본 중앙 패널을 숨기거나 복원한다. 완료 이펙트는 성장/초기화 흐름이 따로 관리한다.
	void Set_ItemUpgradeCenterPanelVisible(bool_t bVisible);
	// Update가 보스 HP의 fill·다음 줄·frame·무력화·구분선·줄 전환·피격 광택을 갱신한다.
	// 표시 조건이나 보스 유효성이 깨지면 STATIC 슬롯이 이전 상태를 남기지 않도록 모두 숨긴다.
	void Update_BossHealthBar();
	void Hide_BossHealthBar();
	// 복제된 적·다른 플레이어의 현재 pose를 최종 카메라 갱신 뒤 화면에 투영한다.
	void Update_WorldHealthBars(f32_t fTimeDelta);
	std::array<f32_t, 12> Get_HealthBarPositions() const;
	bool_t Set_HealthBarPositions(const std::array<f32_t, 12>& offsets);
	std::array<f32_t, 2> Get_MechanicBarScale() const;
	bool_t Set_MechanicBarScale(const std::array<f32_t, 2>& scale);
	bool_t Set_ValtanMechanicTuning(const std::array<f32_t, 8>& tuning);
	void Apply_MechanicBarRect();
	void Update_MechanicBarAnchor();
	bool_t Save_HealthBarPositions(std::string& status);
	bool_t Reload_HealthBarPositions(std::string& status);
	void RenderHUDBarPositionControls();
	// Server의 combo stage·action 시작 tick과 PlayerSkills.json의 stage 길이로 HOLD 진행률을 재구성한다.
	// 네트워크에 연속 충전 퍼센트가 없으므로 관측한 단계 시간을 사용한다.
	void Update_ChargeGauge();
	// 충전 gauge와 같은 표시 조건으로 스킬 이름을 track 가운데 그린다. EndFrame 이후의 제품 텍스트 pass를 사용한다.
	void RenderChargeGaugeText();
	// 보스 제목·HP·남은 줄 수는 EndFrame 뒤에 그린다. Draw_Text는 즉시 제출하므로 아래 UI 그림의 합성 이후에 호출한다.
	void RenderBossHealthBarText();
	// Level이 제공한 rect에 "사망하였습니다"·"부활"·"관전하기"를 UI 합성 뒤 그린다. 패널과 클릭 처리는 Level이 맡는다.
	void RenderDeadSceneText();
	// 레이드 클리어 제목은 Level이 채운 Get_RaidClearTextRects를 사용해 EndFrame 뒤에 표시한다.
	void RenderRaidClearText();
	// Level이 준비한 rect와 "OO을(를) 획득하였습니다" 문구를 ViewModel에서 받아 표시한다.
	void RenderItemAnnounceText();
	// 방이 공유하는 에스더 gauge를 STATIC UI 슬롯으로 표시한다. 발탄·쿠크 아레나와 유효한 gauge/player 등 표시 조건이 필요하다.
	// 조건을 벗어나면 Hide_EstherUI로 이전 프레임의 sprite도 숨긴다.
	void Update_EstherGauge(f32_t fTimeDelta);
	void Hide_EstherUI();
	// ESTHER 및 준비 완료·Ctrl+Z/X/C 안내는 gauge 위에 EndFrame 이후 그린다.
	void RenderEstherGaugeText();
	// 창술사의 3구간 identity를 퍼센트별 Gauge0/1/2 키프레임과 최초 점화→유지 반복 이펙트로 표시한다.
	void Update_LanceMasterIdentityGauge();
	// Server snapshot의 identity·태세·socket 수로 가디언나이트 구슬과 Embereth 10개 socket을 표시한다.
	void Update_GuardianKnightIdentity();
	// 각 Ember socket의 마지막 show/hide/lock 상태다. 바뀔 때만 전환을 재생하고 같으면 끝 프레임을 유지한다. 빈 값은 최초 상태다.
	string m_EmberSocketStates[10];
	// Server DAMAGE_EVENT의 피격 당시 월드 위치에 숫자를 띄운다. 대상의 이후 이동을 따라가지 않으며 EndFrame 뒤 Draw_Text로 표시한다.
	void RenderDamageNumbers();
	// Debug는 FPS 표시 옵션을 따른다. Release는 저장 옵션을 바꾸지 않고 FPS를 표시하며 상위 cinematic 숨김 조건을 따른다.
	void RenderFpsText();
	// class·stance에 대응한 스킬 아이콘을 Skill_<X>_Icon에 넣는다. 기존 Frame 슬롯은 sprite 순서상 위에 남긴다.
	void Update_SkillIcons();
	// 스킬 쿨다운을 g_ArcRatio로 원형 clip한다. 12시 방향에서 시계 방향으로 줄이며 남은 초 글자는 별도 텍스트 pass다.
	void Update_SkillCooldowns();
	// 스킬 슬롯·도화가 Z/X 위의 남은 초 글자를 EndFrame 이후 표시한다.
	void RenderSkillCooldownText();
	// 준비 완료→사용 불가 전환을 감지해 Skill_<X>_Flash 키프레임을 한 번 재생한다.
	void Update_QuickSlotFlash();
	// 제품 UI 합성 뒤 같은 표시 조건으로 HUD 글자와 보스 이름·HP·남은 줄 수를 그린다.
	void RenderCombatHUDText();

	// Debug 초기화에서 Profiler 기록·활성 상태를 초기화하고 HUD 배치 도구를 만든다. 나머지 도구는 필요할 때 준비한다.
	HRESULT ReadyDebugTools();
	// 열 도구와 연결할 세션을 준비한다. bShowWindow가 true이면 공용 표시·입력 focus도 갱신한다.
	HRESULT EnsureDebugTool(DEBUG_TOOL eTool, bool_t bShowWindow = true);
	// 리소스 선택에 필요한 공용 모델 preview·Balance·발탄·Animation 도구를 준비한다. 공용 창 표시·focus는 별도 열기 경로가 담당한다.
	HRESULT EnsureAnimationPreviewBackend();
	// 이전 도구 이름을 공용 창에 대응시켜 표시 상태를 읽는다. World Object/Sequence는 공용 창의 선택 대상도 함께 확인한다.
	bool_t IsDebugToolVisible(DEBUG_TOOL eTool) const;
	// 공용 표시 flag를 갱신하고 닫힐 때 입력·focus와 해당 도구 preview를 정리한다. 도구 객체와 편집 draft는 소유 멤버에 남긴다.
	void SetDebugToolVisible(DEBUG_TOOL eTool, bool_t bVisible);
	// 도구를 숨기고 preview를 정리한다. 도구 객체와 편집 draft는 유지한다.
	void CloseAllDebugTools();

	// 목적지 버튼과 전환 추적 상태를 표시한다. 15초 경과는 화면의 추적만 해제하며 실제 입장·전환 소유자의 처리는 취소하지 않는다.
	void RenderDebugLevelNavigation();
	// 현재 Level의 카메라·로컬 위치를 표시하고 명시적 Move Player를 controller에 예약한다. 물총 로컬 preview와 Server 패턴 요청은 별도 버튼이다.
	void RenderArenaCameraAndPlayerControls();
	// 캐릭터 선택 Level의 바닥 preset과 m/degree 단위 보정을 편집한다. Apply/Restore가 실제 Level에 요청하고 draft 선택만으로 적용하지 않는다.
	void RenderCharacterSelectFloorSwapControls();
	// F1의 명시적 쿠크 UI 미리보기는 HUD ViewModel의 preview override를 편집한다.
	// 광기·모드·슬롯 재배치·샘플 쿨다운을 조절하며 Server의 실제 기믹 상태와 구분한다.
	void RenderKoukuUiPreviewControls();
	// 지원 목적지를 Lobby 또는 Character Select의 typed 입장 경로에 예약한다. true는 요청 수락이며 실제 목적지 도착을 뜻하지 않는다.
	bool_t RequestDebugLevelNavigation(LEVEL eTargetLevel);
	// Resources·Data의 허용 루트에서 도구별 검색 목록을 다시 만든다.
	void RefreshDebugResourceFiles();
	// 쿠크 아레나 관문 버튼은 보스 활성화·자기 플레이어 이동·HUD 대상 변경을 Server 경계로 요청한다.
	void RenderKoukuSaydonArenaControls();
	// 발탄 아레나 안에서 Stage_1/Stage_2 일반 몬스터 재소환을 Server에 요청한다.
	void RenderValtanArenaControls();
	// 발탄 아레나의 복제 상태를 표시하고 벽·지형 변경 버튼을 기존 Server 명령 경계로 전달한다.
	void RenderValtanArenaStateControls();
	// 목록 index를 검사한 뒤 해당 domain 도구를 연다. 지원하는 보스·직업 경로만 구체적 문서에 연결하고 임의 raw 파일을 편집본으로 취급하지 않는다.
	void OpenDebugResourceFile(size_t iFile);

	// 발탄 도구에서 저장된 Server 패턴 목록을 받아 ID·표시 목록을 교체한다. 조회 실패면 이전 목록을 유지하며 성공하면 가능한 이전 선택을 보존한다.
	void RefreshCompletePlayPatternOptions();
	// 발탄 패턴 선택·준비 취소·Complete Play 버튼과 Server 진행 상태를 표시한다. 목록 로드는 명시 요청에서 시작한다.
	void RenderCompletePlayControls();
	// 저장된 쿠크 패턴을 관문별로 표시한다. Complete Play는 기존 CKoukuSaydonBossTool의 Server 요청 경로를 사용한다.
	void RenderKoukuSaydonCompletePlayControls();
	// 활성 쿠크 아레나·command sink·대기 여부·지원 관문·저장 문서와 게시 revision 일치를 확인한다. 도구 생성·Reload는 할 수 있으나 START는 보내지 않는다.
	bool_t PrepareKoukuGateCompletePlay(std::string_view gateId, std::string& status);
	// 검증한 관문 요청을 자원 준비 상태로 보관한다. true여도 아직 START 전송 전이며 Update가 준비·revision을 다시 검사한 뒤 제출한다.
	bool_t StartKoukuGateCompletePlay(std::string_view gateId, std::string& status);
	// 호출자에게 연출 완료와 다음 흐름은 Server 소유라는 상태만 돌려준다. Client가 다음 관문을 직접 진행시키지 않는다.
	void FinishKoukuGateCompletePlay(std::string_view gateId, std::string& status);
	// 전송 전 준비는 폐기하고, 응답 대기 중이면 중지를 예약한다. 활성 실행 소유자는 STOP을 제출하며 응답과 presentation 정리를 구분한다.
	void CancelKoukuGateCompletePlay(const std::string& status);

	// ROOM_PLAYER_ARRIVAL의 slot·목적지를 검증해 시각순 cue를 만든다. 시작할 때 이전 cue를 비우므로 실패 시 이전 목록을 복원하지 않는다.
	bool BeginKoukuSequenceArrivals(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, uint32_t startClockMs, std::string& status);
	// preview 시각(ms)에 도달한 cue를 Server 이동 명령으로 한 번 제출한다. advance가 false여도 이미 보낸 요청의 timeout은 확인한다.
	bool SampleKoukuSequenceArrivals(uint32_t clockMs, bool advance, std::string& status);
	// PLACE_ROOM_PLAYER 응답을 요청 순번·월드·패턴·현재 연결 world generation에 대응시킨다. 거절은 다음 sample이 읽을 실패로 남긴다.
	bool ConsumeKoukuSequenceArrivalResult(const LostArk::Shared::S2C_DEBUG_WORLD_PLAYBACK_RESULT& result);
	// 제출했으나 Server 결과가 없는 cue를 조회한다. 아직 시작 시각 전인 cue는 대기로 세지 않는다.
	bool KoukuSequenceArrivalsPending() const;
	// cue·패턴·실패·연결 세대를 비운다. 이전 응답과 구분하기 위한 실행 epoch는 여기서 초기화하지 않는다.
	void ClearKoukuSequenceArrivals();
	// 저작 ROOM_PLAYER_ARRIVAL의 시각(ms)과 player slot, Server 요청 순번·응답 deadline을 함께 보관한다.
	struct KOUKU_SEQUENCE_ARRIVAL_CUE final
	{
		std::string occurrenceId;
		uint32_t startMs = 0u, playerSlot = 0u, requestSequence = 0u;
		float3_t position{};
		bool submitted = false, completed = false, moved = false;
		std::chrono::steady_clock::time_point deadline{};
	};
	// cue는 이 세션 소유다. 실행 epoch는 요청에 싣고, 응답은 요청 순번·월드·패턴·현재 연결 world generation으로 구분한다.
	std::vector<KOUKU_SEQUENCE_ARRIVAL_CUE> m_KoukuSequenceArrivalCues;
	std::string m_strKoukuSequenceArrivalPatternId, m_strKoukuSequenceArrivalFailure;
	std::string m_strKoukuSequenceArrivalCompletedPreview;
	uint32_t m_iKoukuSequenceArrivalRunEpoch = 0u;
	uint64_t m_iKoukuSequenceArrivalWorldGeneration = 0u;
	bool m_bKoukuCompletePreservesArrivalPosition = false;
	// 발탄·쿠크의 Server 확정 아레나 상태를 공용 패널에 표시하고 typed 명령 버튼을 제공한다.
	void RenderServerArenaActiveControls();
	// 전경 창의 F1 상승 edge로 Developer Tools를 토글한다. ImGui나 제품 UI가 텍스트를 입력 중이면 건너뛴다.
	void UpdateDebugToolShortcut();

	// Action·Sequence workbench의 월드 애니메이션 편집·저장·이력 callback을 World Object 소유자에 연결한다.
	// 문서 포인터·세대·revision이 바뀌면 draft 또는 현재 Level 문서에서 리소스 목록을 재구성한다.
	void RefreshWorldObjectResources();
	// 통합 연출 창만 미저장 Motion 편집을 미리 본다. 다른 소비 경로는 이미 읽은 저장 문서를 유지한다.
	const CWorldSequenceDocument* CompositionPreviewWorldSource() const;
	// Development 편집기의 열린 Area 또는 제품 Level descriptor의 Area를 반환한다. 대응 Area가 없으면 빈 문자열이다.
	std::string GetWorldLevelAreaId() const;
	// 선택 위치(m)와 반경을 현재 저작 카메라에 전달한다. 카메라 확보에 실패하면 false이며, preview 중지를 시도한 뒤 override 해제에 실패할 수도 있다.
	bool FocusWorldLevelPosition(const float3_t& position, float radius, std::string& status);
	// workbench의 token·draft·Area를 확인해 한 클릭의 GPU 월드 위치를 전달한다. 저장은 해당 도구가 담당한다.
	bool UpdateMapEffectPlacementInput();
	// Guide trigger 박스가 예약한 한 번의 표면 위치 선택을 처리한다. 취소·소유권 변경은 기존 위치를 유지하고 표면 miss는 다음 클릭을 기다린다.
	bool UpdateWorldLevelPlacementPickInput();
	// 예약한 live mesh 선택의 소유 도구·Level·focus를 검사한다. 성공은 stable 배치 선택만 바꾸고 TRS는 수정하지 않는다.
	bool UpdateWorldMeshInspectionInput();
	// 현재 Level의 저작 host가 있으면 World Scene 진입 버튼을 표시한다.
	void RenderWorldMeshInspection();
	// World Scene 창의 선택·focus·일회 picking을 중재한다. 새 picking을 예약할 때 다른 저작 소비자의 예약을 해제한다.
	void RenderWorldSceneTool();
	// 선택 위치와 1m 축을 화면에 투영한다. 저작 시작·종료 ms는 안내할 때 초로 환산한다.
	void RenderMapEffectPlacementMarker();
	// 활성 Area와 composition draft 세대를 통합 목록에 전달한다. Map Tool 준비 대기는 60초 안에서 재조회하고 완료·닫힘 때 해제한다.
	void UpdateWorldLevelTool();
	// runtime generation과 stable ID를 확인해 통합 목록의 요청을 기존 저작 소유자에게 연결한다.
	void RenderWorldLevelTool();

	// preview 소유자가 바뀌면 이전 workbench 재생을 멈추고 collider 저작 세대를 비운 뒤 새 소유자를 기록한다.
	void ClaimCompositionPreviewOwner(DEBUG_TOOL owner);
	// 현재 로컬 preview 소유자가 요청한 경우에만 정리한다. Server 시각으로 재생 중인 presentation은 이 경로로 멈추지 않는다.
	void StopCompositionPreview(DEBUG_TOOL owner);
	// Balance 도구 열기와 수리 창 표시 버튼을 제공한다. 수치 편집과 요청 처리는 각 기존 도구에 남긴다.
	void RenderBalanceTestLauncher();
	// F1 허브의 Level·카메라·도구 버튼을 기존 소유자의 명령에 연결한다.
	void RenderDeveloperTools();

	// 두 Area의 trigger·연출·보스 패턴 목록과 필터를 표시한다. 선택한 버튼만 실행 요청 경계로 전달한다.
	void RenderSequenceViewer();
	// 아레나 playback 응답과 편집기 준비를 확인한다. 5초 응답·60초 편집기 timeout 뒤 자동 Server 재전송은 하지 않는다.
	void UpdateSequenceViewer();
	// 최초 열기·Refresh에서 종류별 목록을 다시 읽는다. 특정 종류 읽기 실패는 그 종류의 이전 행을 유지한다.
	void RefreshSequenceViewer();
	// 선택한 stable 행의 버튼을 Test 편집기 preview 또는 아레나의 Server 명령으로 연결한다. 전송 수락과 실제 실행 결과를 구분한다.
	void ExecuteSequenceViewerAction(int action);
	// 목록용 stable ID·표시 이름·관련 실행 대상을 소유한다. vector index를 Server identity로 보내지 않는다.
	struct SEQUENCE_VIEWER_ROW
	{
		int kind = 0; // trigger, 월드 시퀀스, 보스 패턴의 종류 값이다.
		std::string id, name, location, action, sequenceId, related, error;
		float3_t position{};
		bool enabled = true, hasPosition = false;
	};
	std::array<std::vector<SEQUENCE_VIEWER_ROW>, 2> m_SequenceViewerRows;
	std::array<std::string, 2> m_SequenceViewerLoadStatus;
	std::string m_SequenceViewerSelection;
	std::string m_SequenceViewerStatus;
	std::array<char, 256> m_SequenceViewerSearch{};
	bool m_bSequenceViewerLoaded = false;
	int m_iSequenceViewerArea = 0, m_iSequenceViewerKind = 0;
	std::uint32_t m_iSequenceViewerRequest = 0;
	std::uint32_t m_iSequenceViewerAwaitingRequest = 0;
	std::chrono::steady_clock::time_point m_SequenceViewerReplyDeadline{};
	int m_iSequenceViewerPendingEditorAction = -1;
	std::chrono::steady_clock::time_point m_SequenceViewerPendingDeadline{};

	// Debug의 rendering profile·품질·광원 편집 패널을 구성한다. 저장과 세션 비교 측정은 각 서비스의 명시 경계로 전달한다.
	void RenderRenderingWorkbench();
	// Release에서도 사용하는 최적화 비교 창이다. 현재 profile의 세션 측정 UI를 연결하고 닫히면 공용 도구 표시 상태를 맞춘다.
	void RenderOptimizationBenchmarkPanel();
	// 지도 광원·장면 프로필·재사용 광원 목록을 표시한다. 선택과 생성·저장·게시·재로드 버튼을 구분한다.
	void RenderLightingWorkbench();
	// stable 선택 ID가 바뀌면 이전 preview를 멈추고 상세 패널 draft를 새 대상에 맞춘다.
	void SelectRenderingLight(const string& id);
	// 선택 prefix에 따라 지도 행·프로필·재사용 정의의 상세 입력을 나눈다. Save와 Publish는 별도 버튼으로 실행한다.
	void RenderLightDetail();
	// 기존 profile draft와 Update_Profile 경계에 상세 입력을 전달한다. 저장·게시 자체는 상위 패널이 담당한다.
	void RenderSceneProfileDetail();
	// 선택 광원의 재생·일시정지·seek를 조절한다. ms 길이는 세션 preview 상태이며 저작 문서의 수명을 저장하지 않는다.
	void RenderLightSequencer();
	// preview를 먼저 멈추고 게시된 Area 광원을 재조회한다. 후속 실패가 앞선 정지나 목록 갱신까지 되돌리지는 않는다.
	bool ReloadPublishedMapLights();
	// 이전 preview를 정지한 뒤 선택 종류의 anchor·복사 문서·profile을 준비한다. 실패해도 이전 재생을 복원하지 않는다.
	bool StartLightingPreview();
	// 지도 문서 복사본에서 선택 광원을 켜고 나머지 행의 enabled는 보존한다. 검증 후 별도 preview runtime에 전달한다.
	bool StageMapLightPreview();
	// 같은 Level에서 자신이 활성화한 profile을 계속 소유할 때 이전 profile 복원을 시도한다. 복원 결과와 별개로 preview 상태는 정리한다.
	void StopLightingPreview();
	// Debug Update가 preview 시각과 프레임 광원을 갱신한다. 창 닫힘·Level 변경은 정지하고 anchor 실패는 해당 프레임 제출을 건너뛴다.
	void UpdateLightingPreview();
	// 표시가 켜져 있고 Profiler가 있을 때 live FPS·CPU·유효한 GPU 측정값을 입력 없는 overlay로 그린다.
	void RenderProfilerOverlay();
	// 기록 초기화와 JSON 저장 버튼을 제공한다. 저장은 공용 ProfilerTool의 비동기 경로로 요청하고 진행 상태를 표시한다.
	void RenderProfilerSettings();

private:
	// Engine이 만든 장치/context를 UI·도구 생성에 공유한다. Engine manager를 이 멤버가 소유하는 것은 아니다.
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	// 프로필 목록과 활성화 상태는 Client service가 보관하고 실제 렌더 설정은 Engine에 적용한다.
	CRenderingProfileService m_RenderingProfiles;
	// 캐릭터 생성의 어두운 무대 프로필을 활성화하기 전 ID다. 열기/닫기 전환에서만 프로필을 바꾼다.
	string m_strSceneProfileBeforeCustomizing;
	bool_t m_bClassShowcaseShadowFocusActive = false;
	float4_t m_vShadowFocusBeforeClassShowcase{};
	float4_t m_vClassShowcaseShadowFocus{};
	string m_strKoukuGateProfileRequest;
	string m_strKoukuGateProfileApplied;
	string m_strSceneProfileBeforeKoukuGate;
	CLightResourceCatalog m_LightResources;
	unique_ptr<CKoukuSaydonPresentationPlayer> m_pKoukuPresentationPlayer;
	unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
	unique_ptr<CProfilerTool> m_pProfilerTool;
	bool_t m_bRuntimeProfilerVisible = false;
	bool_t m_bF7Down = false;
	// MainApp이 layout을 보관하고 실제 CUI_Sprite는 Engine의 STATIC Layer에도 등록한다. 먼저 생성해 다른 창 아래에 그린다.
	unique_ptr<CUILayoutRuntime> m_pHUDRuntimeView = { nullptr };
	// HUD 다음에 생성하는 전투 분석 창이다. ViewModel 통계를 읽어 다른 runtime 창 아래에 표시한다.
	unique_ptr<CCombatAnalysisFrameView> m_pCombatAnalysisView = { nullptr };
	// KoukuHudModes.json 한 모드의 문양과 Q..F 아이콘 목록이다. 기믹 순서가 있으면 재배치하며 스킬 없는 슬롯은 비운다.
	struct KOUKU_HUD_MODE_SKILL
	{
		string strDisplayName;
		string strIconPath;
	};
	struct KOUKU_HUD_MODE_DEF
	{
		string strId;
		string strEmblemSlot;
		bool_t bRandomOrder = false;
		vector<KOUKU_HUD_MODE_SKILL> Skills;
	};
	vector<KOUKU_HUD_MODE_DEF> m_KoukuHudModes;
	struct HUD_SKILL_MARK
	{
		uint32_t iSkillId = 0u;
		string strMark;
	};
	vector<HUD_SKILL_MARK> m_HudSkillMarks;
	vector<pair<string, string>> m_HudSkillMarkAssets;
	struct HUD_BUFF_SOURCE
	{
		string strSource;
		string strStance;
		bool_t bDebuff = false;
		string strIconAsset;
	};
	vector<HUD_BUFF_SOURCE> m_HudBuffSources;
	// SkillBuff ID를 HudBuffIcons.json의 이미지에 대응시킨다. snapshot에만 있고 정의가 없는 buff는 슬롯을 그리지 않는다.
	unordered_map<uint32_t, string> m_HudSkillBuffIcons;
	// HUD 갱신이 그 프레임의 쿨다운·buff 남은 시간 글자를 예약한다. RenderSkillCooldownText가 EndFrame 뒤 소비한다.
	struct HUD_TIMED_TEXT
	{
		string strSlotId;
		uint32_t iEndTick = 0u;
		bool_t bDebuff = false;
		bool_t bUnderSlot = false;
	};
	vector<HUD_TIMED_TEXT> m_HudTimedTexts;
	bool_t m_bHudSpecialSlotShown = false;
	// 로컬 플레이어가 배에 탔을 때 해양 HUD가 숨긴 HP/마나·아이템·class 특수 슬롯의 글자도 숨기는 표시 상태다.
	bool_t m_bShipHudActive = false;
	// Update_VehicleHud가 채우는 보급량/최대 보급량과 노트 단위 선박 속도다.
	uint32_t m_iShipSupply = 0u;
	uint32_t m_iShipSupplyMax = 0u;
	f32_t m_fShipKnots = 0.f;
#ifdef _DEBUG
	unique_ptr<CLevelNavigationDebug> m_pLevelNavigationDebug;
	// 아직 적용하지 않은 UI draft다. 실제 설치된 바닥은 현재 Level이 소유한다.
	std::string m_strCharacterSelectFloorDraftId;
	float3_t m_vCharacterSelectFloorOffsetMeters{};
	f32_t m_fCharacterSelectFloorYawDegrees = 0.f;
	int32_t m_iCharacterSelectFloorEnvironment = 1;
	bool_t m_bCharacterSelectFloorShaderDefaultBrightness = false;
	bool_t m_bKoukuUiPreview = false;
	HUD_KOUKU_GIMMICK_STATE m_KoukuUiPreview;
	// 명시적 저작 미리보기 시간이다. 제품의 제한 시간은 Server deadline을 읽는다.
	bool_t m_bDungeonTimerPreview = false;
	f32_t m_fDungeonTimerStartSeconds = 90.f;
	f32_t m_fDungeonTimerWarningSeconds = 10.f;
#endif
	// BossUI.json의 layout을 보관한다. 보스 HP는 class별 전투 HUD와 따로 갱신한다.
	unique_ptr<CUILayoutRuntime> m_pBossUIView = { nullptr };
	// MainApp이 보관하는 쿠크 미니게임 제한 시간 UI다. Server deadline을 읽어 화면에 고정 표시한다.
	unique_ptr<CDungeonTimerView> m_pDungeonTimerView;
	unique_ptr<CWorldHealthBarView> m_pWorldHealthBarView = { nullptr };
	// X/Y 쌍 순서는 기믹·아군·일반 몬스터·쿠크세이튼·쿠크·발탄이다.
	std::array<f32_t, 12> m_HealthBarOffsets{}, m_SavedHealthBarOffsets{};
	// 기믹 줄에 적용할 너비/높이 배율이다. 기준 rect를 직접 덮어쓰지 않는다.
	std::array<f32_t, 2> m_MechanicBarScale{ 1.f / 3.f, 1.f };
	std::array<f32_t, 2> m_SavedMechanicBarScale{ 1.f / 3.f, 1.f };
	// 그룹마다 머리 기준 X/Y offset과 너비/높이 배율을 둔다. 발탄 무력화와 갑옷 PNG는 각각 조절한다.
	std::array<f32_t, 8> m_ValtanMechanicTuning{ 0.f, 0.f, 1.f / 3.f, 1.f, -24.f, -40.f, 1.f, 1.f };
	std::array<f32_t, 8> m_SavedValtanMechanicTuning{ 0.f, 0.f, 1.f / 3.f, 1.f, -24.f, -40.f, 1.f, 1.f };
	bool_t m_bValtanStaggerDebug = false;
	bool_t m_bValtanArmorBreakDebug = false;
	std::array<float4_t, 3> m_MechanicBarBaseRects{};
	std::array<bool_t, 3> m_MechanicBarHasBase{};
	// 최종 카메라 갱신 뒤 월드 HP bar와 같은 실제 머리 anchor에서 읽은 화면 위치다.
	float2_t m_MechanicBarHeadAnchor{};
	bool_t m_bMechanicBarHasHeadAnchor = false;
	bool_t m_bMechanicBarUIAllowed = false;
	std::string m_strHealthBarPositionStatus;
	// 레이드 공통 에스더 gauge의 STATIC layout을 보관한다.
	unique_ptr<CUILayoutRuntime> m_pEstherUIView = { nullptr };
	// 에스더 전면 컷인 flipbook을 소유한다. gauge UI 뒤에 생성해 gauge와 전투 HUD 위에 그린다.
	unique_ptr<CEstherCutinPresentationService> m_pEstherCutinService = { nullptr };
	// 재련 layout과 gauge/결과 연출을 보관한다. 실제 강화 단계는 Server 응답을 읽으며 P 키는 캐릭터 정보 창이 사용한다.
	unique_ptr<CUILayoutRuntime> m_pItemUpgradeView = { nullptr };
	bool_t m_bItemUpgradePreviewVisible = false;
	// BuildItemUpgradeSlots의 현재 표시 목록에 대한 index다. 행 선택으로 바뀌며 열기·프레임 동기화 때 유효 범위로 제한한다.
	int32_t m_iItemUpgradeSelectedSlot = 0;
	// 성장 버튼이 시작한 0~100 표시값이다. gauge와 퍼센트 글자가 같은 값을 읽는다.
	int32_t m_iItemUpgradePreviousPercent = 0;
	// 0→100 채우기 진행 여부다. 완료 이펙트는 채우기 중 숨기고 100에서 표시하며 SmeltGlow는 대기 0%에서만 반복한다.
	bool_t m_bItemUpgradeGrowing = false;
	f64_t m_dItemUpgradeGrowStartSeconds = -1.0;
	// 채우기 완료 뒤 WingedRingGold·LevelUpMotion2Big의 alpha 0→1 시작 시각이다.
	// 음수는 fade 대기가 없음을 뜻하며 새 클릭마다 초기화한다.
	f64_t m_dItemUpgradeCompleteRevealStartSeconds = -1.0;
	// 성장 클릭 때 CoreFlash를 한 번 시작할 예약이다. 프레임 갱신이 소비해 비우며 ShockwaveRing은 28프레임/20fps 뒤에 예약한다.
	bool_t m_bItemUpgradeCoreFlashPending = false;
	f64_t m_dItemUpgradeShockwaveScheduledAt = -1.0;
	// 재련 요청·응답의 화면 단계다. WAITING은 응답/공개 대기, SUCCESS/FAIL은 결과 modal이며 확인 전 새 시도를 막는다.
	enum class ITEM_UPGRADE_ATTEMPT_RESULT
	{
		NONE, WAITING, SUCCESS, FAIL
	};
	ITEM_UPGRADE_ATTEMPT_RESULT m_eItemUpgradeAttemptResult = ITEM_UPGRADE_ATTEMPT_RESULT::NONE;
	// 결과 연출만 위한 상태다. 실제 강화 단계는 Server 인벤토리 snapshot에서 읽는다.
	bool_t m_bItemUpgradePendingAttemptSuccess = false;
	bool_t m_bItemUpgradeResultUnavailable = false;
	uint32_t m_iNextItemUpgradeRequest = 1;
	uint32_t m_iPendingItemUpgradeRequest = 0;
	f64_t m_dItemUpgradeRequestDeadline = -1.0;
	string m_strItemUpgradeAttemptItemId;
	string m_strItemUpgradeAttemptIcon;
	wstring m_strItemUpgradeAttemptName;
	LostArk::Shared::EQUIPMENT_SLOT m_eItemUpgradeAttemptSlot = LostArk::Shared::EQUIPMENT_SLOT::NONE;
	uint16_t m_iItemUpgradeConfirmedLevel = 0;
	// SmeltSuccess/Fail 표시 후 120프레임/30fps에 맞춘 4초 대기의 만료 시각이다. 애니메이션 완료 통지를 받는 값은 아니다.
	// 그 뒤 대기 원과 burst를 숨겨 결과 문구만 남기며 음수는 종료 예약이 없음을 뜻한다.
	f64_t m_dItemUpgradeResultSettleAt = -1.0;
	// 로컬 플레이어 stance가 바뀔 때만 identity 아이콘 전환을 재생하기 위한 이전 값이다. NONE은 최초 관측도 전환으로 만들게 한다.
	LostArk::Shared::PLAYER_STANCE_ID m_ePreviousHudStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	// HOLD는 조기 해제로 stage가 짧아질 수 있어 관측한 combo stage·시작 tick으로 실제 누적 시간을 계산한다.
	// 현재 stage의 시작 tick과 앞서 완료한 stage의 ms를 별도 보관하며 INVALID_SKILL_ID는 추적하지 않는 상태다.
	// 단축·건너뛴 stage를 관측하면 같은 스킬 사용 동안 취소를 유지해 충전 gauge를 숨긴다.
	LostArk::Shared::SKILL_ID m_iChargeGaugeTrackedSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
	std::uint8_t m_iChargeGaugeTrackedComboStage = 0;
	std::uint32_t m_iChargeGaugeStageStartTick = 0;
	f32_t m_fChargeGaugeElapsedBeforeCurrentStageMs = 0.f;
	bool_t m_bChargeGaugeCancelled = false;
	// Q/W/E/R/A/S/D/F/T/V 순서의 준비 완료 여부다. ready→not-ready 전환만 사용 이펙트를 시작한다.
	// 준비된 스킬의 cooldown end tick은 매번 현재 tick일 수 있으므로 raw tick 증가로 판정하지 않는다.
	bool_t m_bPreviousQuickSlotReady[10] =
		{ true, true, true, true, true, true, true, true, true, true };
	// 창술사 identity 3구간 각각이 100에 도달하는 전환을 감지해 점화 이펙트를 한 번만 시작한다.
	bool_t m_bLanceGaugeSegmentWasFull[3] = { false, false, false };
	// ImGui::GetTime 기준 구간별 점화 시작 초다. 단발 점화가 끝난 후 유지 loop로 바꾸며 음수는 비활성이다.
	f64_t m_dLanceGaugeIgniteStartSeconds[3] = { -1.0, -1.0, -1.0 };
	bool_t m_bLanceGaugeLoopStarted[3] = { false, false, false };
	// ViewModel의 Server 피격 정보에 Client 표시 시각·애니메이션 상태를 더한 숫자 표시 기록이다.
	struct FLOATING_DAMAGE_NUMBER
	{
		LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::PLAYER_ID iSourcePlayerId = LostArk::Shared::INVALID_PLAYER_ID;
		f64_t dSpawnSeconds = 0.0;
		float3_t vWorldPosition = {};
		uint32_t iAmount = 0;
		bool_t isOutgoing = false;
		// 0은 피해량, 1은 카운터, 2는 무력화 성공이다.
		uint8_t iMechanicSuccess = 0u;
		// NONE이면 iAmount를 피해량으로, 문양이 있으면 카드 미로 사냥꾼의 조각 수로 표시한다.
		LostArk::Shared::MECHANIC_CARD_SYMBOL eCardMazeSuit =
			LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
		// 피격 종류에 대응하는 숫자 색과 재생 구간을 고른다.
		LostArk::Shared::DAMAGE_HIT_FLAG eHitFlag =
			LostArk::Shared::DAMAGE_HIT_FLAG::NORMAL;
		// 같은 대상의 여러 숫자가 겹치지 않도록 anchor에서 벌리는 기준 pixel 거리다.
		// 원본 영상에서 추적한 수치가 아닌 프로젝트 표시값이다.
		f32_t fScatterX = 0.f;
		f32_t fScatterY = 0.f;
	};
	vector<FLOATING_DAMAGE_NUMBER> m_FloatingDamageNumbers;
	// 보스 줄 전환과 피격을 감지하는 이전 상태다. 줄 감소 시 흰 fill과 교차 fade하고 HP 감소 시 fill 끝에 광택을 준다.
	// 원본 mark의 확장·감쇠 움직임은 절차적 표시로 근사하며 별도 원본 이미지 추출 성공을 의미하지 않는다.
	string m_strPreviousBossArchetypeId;
	uint32_t m_iPreviousBossBarsRemaining = 0u;
	f64_t m_dBossBarTickFlashStartSeconds = -1.0;
	uint32_t m_iPreviousBossCurrentHp = 0u;
	uint32_t m_iPreviousBossMaximumHp = 0u;
	f64_t m_dBossHitGlowStartSeconds = -1.0;
	f32_t m_fBossHitGlowFillRatio = 0.f;
	// rolling damage buffer에서 이미 띄운 snapshot batch를 구분하는 Server tick 커서다.
	// 동일 snapshot의 이벤트들은 같은 tick으로 묶이므로 처리한 이전 batch가 buffer에서 잘려도 중복 표시하지 않는다.
	uint32_t m_iLastRenderedDamageServerTick = 0u;
	// Lobby_Layout.json의 반복 배경과 버튼을 소유한다. STATIC CUI_Sprite라 엔진 경로로 그려지고 Debug ImGui보다 아래에 있다.
	unique_ptr<CUILayoutRuntime> m_pLobbyBackgroundView = { nullptr };
	struct LOBBY_SERVER_ENTRY
	{
		wstring		strName;
		wstring		strState;
		wstring		strTag;
		uint32_t	iCharacterCount = 0;
		// 이 행의 인원 수를 CCharacterRoster의 현재 process-session 캐릭터 수로 표시할지 결정한다. 계정·디스크 저장 캐릭터 수가 아니다.
		bool_t		bCountsSavedCharacters = false;
		bool_t		bCreatable = true;
	};
	vector<LOBBY_SERVER_ENTRY>	m_LobbyServers;
	int32_t						m_iLobbySelectedServer = -1;
	// 로비 밖에서는 false다. 다음 로비 최초 프레임에 로고 도입부를 다시 시작한다.
	bool_t						m_bLobbyWasActive = false;
	f32_t						m_fLobbyLogoIntroElapsed = 0.f;
	// 로비의 "게임 시작" 버튼으로 여는 창을 보관한다. STATIC sprite를 갱신하고 선택 의도는 MainApp이 소비한다.
	unique_ptr<CCharacterSelectWindowView> m_pCharacterSelectWindowView = { nullptr };
	// 현재 Level의 복제 marker로 갱신하는 STATIC 미니맵 view를 보관한다.
	unique_ptr<CMinimapView> m_pMinimapView = { nullptr };
	// 현재 초기화에서 생성하지 않는 기존 스킬 창 멤버다. K 단축키도 연결하지 않는다.
	unique_ptr<CSkillWindowView> m_pSkillWindowView = { nullptr };
	// Release에서도 I 키로 여는 인벤토리다.
	unique_ptr<CInventoryView> m_pInventoryView = { nullptr };
	// 베른 수리 NPC가 여는 수리 창이다. 독립 토글 키 없이 Open/Close와 Escape 경로를 사용한다.
	unique_ptr<CRepairWindowView> m_pRepairWindowView = { nullptr };
	// NPC별 상점 view를 보관한다. MainApp이 구매 의도를 받아 CNetworkManager::Send_BuyItems로 보낸다.
	unique_ptr<CShopWindowView> m_pShopWindowView = { nullptr };
	// 미니맵 아래 내구도 HUD다. 창이 아니므로 별도의 열림 상태나 Escape 순서를 갖지 않는다.
	unique_ptr<CDurabilityHudView> m_pDurabilityHudView = { nullptr };
	bool_t m_bIDown = false;
	// Release에서도 P 키로 여는 캐릭터 정보 창이다. 초상화는 Render의 월드 pass 전에 준비한다.
	unique_ptr<CCharacterInfoWindowView> m_pCharacterInfoView = { nullptr };
	// 캐릭터 정보 창이 여는 아바타 view다. 장착 상태는 Server 복제값을 읽는다.
	unique_ptr<CAvatarBookWindowView> m_pAvatarBookView = { nullptr };
	// 베른에 콜로세움 LOAD 요청이 남아 있는 Render에서 로컬 캐릭터를 그리는 초상화다. 요청 소비가 늦어지면 반복해서 그릴 수 있다.
	// 전환 서비스가 결과 SRV의 COM 참조를 보관하므로 이후 Loading에서도 이미지 수명을 유지한다.
	unique_ptr<CCharacterPortraitRenderer> m_pTransferPortrait = { nullptr };
	void Render_ColosseumTransferPortrait();
	bool_t m_bCharacterInfoKeyDown = false;
	// Release에서도 N 키로 여는 탈것 창이다. 아바타 창 뒤에 생성해 위에 표시하며 Alt+V는 기존 스킬 슬롯으로 유지한다.
	unique_ptr<CVehicleWindowView> m_pVehicleWindowView = { nullptr };
	bool_t m_bVehicleWindowKeyDown = false;
	// 캐릭터 정보의 칭호 변경 버튼이 열며 먼저 생성된 창들 위에 표시한다.
	unique_ptr<CHonorTitleWindowView> m_pHonorTitleWindowView = { nullptr };
	// M 키로 여는 구역 단계의 월드 지도 창이다.
	unique_ptr<CWorldMapWindowView> m_pWorldMapWindowView = { nullptr };
	// 네모난 구멍의 노래를 연주하는 동안 표시하는 주황색 시전 gauge다.
	unique_ptr<CSongCastGaugeView> m_pSongCastGaugeView = { nullptr };
	bool_t m_bWorldMapKeyDown = false;
	// 다른 runtime 창이 없을 때 Escape로 여는 환경설정이다. 기존 오디오 버스와 지원되는 영상 옵션을 편집한다.
	unique_ptr<CSystemOptionWindowView> m_pSystemOptionView = { nullptr };
	bool_t m_bSystemOptionKeyDown = false;
	// 캐릭터 선택 복귀가 예약되면 도착한 로비에서 선택 창을 열도록 남기는 상태다.
	bool_t m_bOpenCharacterSelectOnLobby = false;
	bool_t m_bCharacterReturnPending = false;
	f64_t m_dCharacterCaptureStartedAt = 0.0;
	uint64_t m_iCharacterCaptureWorldGeneration = 0u;
	// Loading 최초 진입에서 창 정리를 한 번 수행하기 위한 이전 Level 상태다.
	bool_t m_bWasLoadingLevel = false;
	// 열린 창을 오래된 순서로 보관한다. 유효 구간은 [0, m_iEscapeWindowCount)다.
	ESCAPE_WINDOW m_EscapeWindowOrder[ETOUI(ESCAPE_WINDOW::END)] = {};
	uint32_t m_iEscapeWindowCount = 0u;
	// 프레임 대기와 FPS 표시를 위한 평활값·직전 피격 시각·프레임 종료 시각이다.
	f32_t m_fSmoothedFps = 0.f;
	f64_t m_dLastDamageSeconds = -1000.0;   // FPS의 전투 중 판정에 사용하는 최근 피격 시각이다.
	std::chrono::steady_clock::time_point m_LastFrameEnd{};
	// 모든 runtime 창 뒤에 생성해 위에 표시하는 운반 아이콘이다.
	// 아이템은 1..4, 탈것은 5..0에 연결하며 등록은 Client 표시 상태다.
	unique_ptr<CQuickSlotDragView> m_pQuickSlotDragView = { nullptr };
	uint32_t m_iSpecialQuickSlotVehicle[6] = {};
	bool_t m_bSpecialKeyDown[6] = {};
	// 각 Item_1..4의 item ID 연결은 Client 로컬 상태다. Server에는 인벤토리 item ID로 사용을 요청한다.
	// 이 연결 자체를 별도 quick slot 프로토콜이나 영구 저장 데이터로 보내지 않는다.
	string m_strItemQuickSlot[4];
	bool_t m_bItemKeyDown[4] = {};
	uint32_t m_iNextUseItemSequence = 1u;
	// Release에서도 Enter로 여는 채팅 입력 창이다.
	unique_ptr<CChatWindowView> m_pChatWindowView = { nullptr };
	bool_t m_bEnterDown = false;
	bool_t m_bEscapeDown = false;
	// Release에서도 표시하는 파티 창이다. 현재 Level의 복제된 party roster·체력 view를 받아 표시한다.
	unique_ptr<CPartyWindowView> m_pPartyWindowView = { nullptr };

	// MainApp이 도구를 보관한다. 닫을 때 draft는 유지하고 Level 전환 때 차용 Level과 preview 연결을 정리한다.
	unique_ptr<CMapTool> m_pMapTool = { nullptr };
	unique_ptr<CEffect_Tool> m_pEffectTool = { nullptr };
	unique_ptr<CEffect_Tool_V2> m_pEffectToolV2 = { nullptr };
	unique_ptr<CAnimation_Tool> m_pAnimationTool = { nullptr };
	unique_ptr<CValtanActionWorkbench> m_pValtanActionWorkbench =
		{ nullptr };
	unique_ptr<CKoukuSaydonActionWorkbench> m_pKoukuSaydonActionWorkbench =
		{ nullptr };
	// Animation·Effect·Character 도구가 같은 preview panel을 공유한다. Level 전환/종료 때 MainApp이 공용 preview를 정리한다.
	shared_ptr<CCharacterPreviewPanel> m_pCharacterPreviewPanel = { nullptr };
	unique_ptr<CEquipmentAuthoringTool> m_pEquipmentAuthoringTool = { nullptr };
	unique_ptr<CHUDLayoutTool> m_pHUDLayoutTool = { nullptr };
	unique_ptr<CBalanceTool> m_pBalanceTool = { nullptr };
	unique_ptr<CValtanBossTool> m_pValtanBossTool = { nullptr };
	unique_ptr<CKoukuSaydonBossTool> m_pKoukuSaydonBossTool = { nullptr };
	unique_ptr<CCameraTool> m_pCameraTool = { nullptr };
#ifdef _DEBUG
	// 닫힌 F1 패널의 draft와 게시 HANDLE도 MainApp 종료까지 유지한다.
	std::unique_ptr<CVehicleDebugPanel> m_pVehicleDebugPanel;
	unique_ptr<CWorldObjectTool> m_pWorldObjectTool;
	unique_ptr<CWorldLevelTool> m_pWorldLevelTool;
	unique_ptr<CWorldSceneTool> m_pWorldSceneTool;
	bool m_bWorldMeshPickArmed = false, m_bWorldMeshPickLeftDown = true;
	bool m_bWorldMeshPickSuppressMouse = false;
	uint32_t m_iWorldMeshPickLevel = UINT32_MAX;
	unique_ptr<CGuideAITool> m_pGuideAITool;
	unique_ptr<CMaharakaAITool> m_pMaharakaAITool;
	unique_ptr<WORLD_LEVEL_TOOL_REQUEST> m_pWorldLevelPendingMapRequest;
#endif
	std::chrono::steady_clock::time_point m_WorldLevelMapDeadline{};
	// World Level 배치와 Guide trigger가 공유하는 단일 viewport pick 예약이다.
	bool m_bWorldLevelPickArmed = false;
	DEBUG_TOOL m_eWorldLevelPickOwner = DEBUG_TOOL::WORLD_LEVEL;
	uint32_t m_iWorldLevelPickLevel = UINT32_MAX;
	bool m_bWorldLevelPickLeftDown = true;
	bool m_bWorldLevelPickSuppressMouse = false;
	unique_ptr<KOUKU_MAP_EFFECT_PLACEMENT_REQUEST> m_pMapEffectPlacementRequest;
	DEBUG_TOOL m_eMapEffectPlacementOwner = DEBUG_TOOL::NONE;
	uint32_t m_iMapEffectPlacementLevel = UINT32_MAX;
	bool m_bMapEffectPlacementLeftDown = true;
	bool m_bMapEffectPlacementSuppressMouse = false;
	std::string m_strMapEffectPlacementStatus;
	// 현재 선택한 문서를 차용하는 cache다. generation/revision과 함께 검사하고 Level 전환에서 차용 포인터를 비운다.
	const CWorldSequenceDocument* m_pWorldObjectCatalogSource = nullptr;
	uint64_t m_iWorldObjectCatalogGeneration = UINT64_MAX;
	uint32_t m_iWorldObjectCatalogRevision = 0;
	unique_ptr<CSequencerTool> m_pSequencerTool = { nullptr };
	unique_ptr<CKoukuSaydonActionWorkbench> m_pSequenceActionWorkbench;
	unique_ptr<CCharacterActionWorkbench> m_pCharacterActionWorkbench;
	// 공용 미리보기 시계는 UI focus와 무관하게 시작한 세션이 소유한다.
	DEBUG_TOOL m_eCompositionPreviewOwner = DEBUG_TOOL::NONE;
	DEBUG_TOOL m_eColliderAuthoringOwner = DEBUG_TOOL::NONE;
	uint64_t m_iColliderAuthoringDraftGeneration = UINT64_MAX;
	uint64_t m_iColliderAuthoringSerial = 0;
	unique_ptr<CRenderingBenchmark> m_pRenderingBenchmark = { nullptr };
	bool_t m_bF1Down = false;
	bool_t m_bDeveloperToolsVisible = false;
	bool_t m_bProfilerVisible = false;
	bool_t m_bRenderQualityDraftInitialized = false;
	array<bool_t, static_cast<size_t>(DEBUG_TOOL::COUNT)>
		m_DebugToolVisible = {};
	// 여러 창을 표시해도 월드 viewport 입력은 한 도구에만 준다. 창 표시와 입력 focus를 따로 추적한다.
	DEBUG_TOOL m_eDebugInputOwner = DEBUG_TOOL::NONE;
	DEBUG_TOOL m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
	LEVEL m_eDebugLevelNavigationTarget = LEVEL::END;
	std::chrono::steady_clock::time_point m_DebugLevelNavigationDeadline{};
	bool_t m_bDebugLevelNavigationDeadlineActive = false;
	string m_strDebugLevelNavigationStatus =
		"Choose a destination. Gameplay Areas always use the existing Server-approved entry route.";
	vector<DEBUG_RESOURCE_FILE> m_DebugResourceFiles;
	array<char_t, 192> m_DebugResourceSearch = {};
	bool_t m_bDebugResourceScanAttempted = false;
	size_t m_iSelectedDebugResourceFile =
		static_cast<size_t>(-1);
	string m_strDebugResourceStatus =
		"Resource Files has not been indexed yet.";
	string m_strServerArenaActiveStatus =
		"Enter the Server-approved Valtan Arena to inspect Active state.";
	bool_t m_bServerArenaPresetStatusTracking = false;
	uint32_t m_iNextKoukuStageTeleportRequestSequence = 1u;
	string m_strKoukuStageTeleportStatus =
		"Enter the Server-approved KoukuSaydon Arena to inspect SL01-SL05 stages.";
	// 쿠크 Complete Play 목록은 Boss Tool에서 읽고 관문 combo는 목록만 거른다. 실제 선택은 stable Pattern ID다.
	bool_t m_bKoukuCompletePlayLoadAttempted = false;
	bool_t m_bKoukuBingoHammerColliders = false;
	int32_t m_iKoukuCompletePlayGate = 0;
	int32_t m_iKoukuCompletePlayCategory = 0;
	uint32_t m_iKoukuCompletePlayFlowRevision = 0u;
	bool_t m_bKoukuCompletePlayAwaitingGate = false;
	bool_t m_bKoukuFlowPublishPending = false;
	int32_t m_iKoukuCompletePlaySelection = 0;
	string m_strKoukuCompletePlayPatternId;

	vector<string> m_CompletePlayPatternIds;
	vector<string> m_CompletePlayPatternLabels;
	// 선택 정본은 stable ID다. UI index는 매 프레임 여기서 찾으며 저장 identity로 사용하지 않는다.
	string m_strCompletePlayPatternId;
	bool_t m_bCompletePlayPatternLoadAttempted = false;
	bool_t m_bCompletePlayStatusTracking = false;
	string m_strCompletePlayTrackedPatternId;
	string m_strCompletePlayStatus =
		"Select a saved semantic pattern, then submit Complete Play to the Server.";
	RENDER_QUALITY_SETTINGS m_RenderQualityDraft = {};
	SCENE_RENDERING_PROFILE m_SceneRenderingDraft = {};
	string m_strRenderingDraftProfileId;
	string m_strRenderingSelectedProfileId;
	string m_strRenderingQualityProfileId;
	LEVEL m_eRenderingSelectedLevel = LEVEL::END;
	uint32_t m_iRenderingLastLevel = UINT32_MAX;
	char m_szRenderingNewProfileId[129]{};
	char m_szRenderingNewProfileName[257]{};
	// 조명 저작 세션과 목록 선택은 저장 문서의 상태를 보관한다. 아래 preview 시계·문서 복사본과 구분한다.
	CAreaLightAuthoringSession m_AreaLightSession;
	string m_strRenderingLightAreaAttempt;
	string m_strSelectedRenderingLightId = "@default-directional";
	int m_iRenderingLightCategory = 0;
	int m_iRenderingLightUsageCategory = 0;
	int m_iRenderingLightCreateUsage = 0;
	int m_iRenderingLightCreateAnchor = 0;
	int m_iRenderingLightCreateType = 2;
	char m_szRenderingLightName[129] = "Spotlight";
	string m_strLightingStatus;
	// 미리보기 ID·이전 profile·Level을 함께 보관한다. Stop은 자신이 활성화한 profile을 여전히 소유할 때만 복원을 시도한다.
	string m_strLightPreviewResourceId;
	string m_strLightPreviewSelectionId;
	string m_strLightPreviewPreviousProfileId;
	string m_strLightPreviewSceneProfileId;
	shared_ptr<CMapLightPresentationRuntime> m_pLightSequencerMapPreview;
	std::chrono::steady_clock::time_point m_LightPreviewLastUpdate{};
	double m_fLightPreviewCursorMs = 0.0;
	int m_iLightPreviewLifetimeMs = 3000;
	bool_t m_bLightPreviewPaused = false;
	float4x4_t m_LightPreviewMapPivot{};
	uint32_t m_iLightPreviewLevel = UINT32_MAX;
	bool_t m_bLightingDebugWire = true;
	bool_t m_bLightResourcesWindowVisible = true;
	bool_t m_bLightDetailWindowVisible = true;
	bool_t m_bLightSequencerWindowVisible = true;
	bool_t m_bRenderingQualityWindowVisible = true;
	bool_t m_bResetLightingLayout = false;
	bool_t m_bPreviewMapLightDraft = true;
	shared_ptr<uint32_t> m_pLightPreviewSkipped = make_shared<uint32_t>(0u);
	string m_strToolStatus =
		"Select a tool. Map authoring targets the current level Area.";
	string m_strRenderingStatus =
		"Rendering profiles are loaded from the published runtime catalog.";
	string m_strProfilerCaptureStatus;
	// Debug 아이템 지급 목록의 catalog index와 로컬 요청 순번이다.
	// Server가 돌려준 전체 인벤토리 snapshot으로 표시를 갱신하며 이 패널이 결과를 로컬 생성하지 않는다.
	int32_t m_iSelectedDebugItemIndex = 0;
	uint32_t m_iNextDebugGiveItemSequence = 1;
	string m_strDebugItemStatus;

private:
	// Level/UI가 현재 MainApp에 접근하는 비소유 주소다. 실제 수명은 Client.cpp의 앱 소유자가 관리한다.
	static CMainApp* s_pActiveInstance;

public:
	// Client.cpp가 호출해 준비된 MainApp을 받는다. 초기화 실패는 nullptr로 알린다.
	static unique_ptr<CMainApp> Create();
	// 도구/네트워크를 정리하고 Engine을 종료한다. Loading worker 종료 뒤 공유 준비 cache를 해제한다.
	void Free();
};

NS_END
```

### 전체 6. Client/Private/MainApp.cpp

파일: `C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp`.

변경 종류: G04 후보 위의 기존 파일 반영 후보 전체.
이번 G의 delta는 두 include, 숨김 중 Update, 기존 카메라 호출 두 곳 교체, Debug panel lazy Render와 Free 정리다.
G03의 Prepare_UISounds와 G04의 ValtanBossTool 연결은 선행 변경으로 그대로 포함한다.

```cpp
// Client 진입점과 엔진 사이에서 Level 전환, 제품 UI, 네트워크·연출·도구의 프레임 순서를 조율한다.
// socket 선언을 Windows 계열 헤더보다 먼저 준비하는 Client include 순서를 유지한다.
#include <WinSock2.h>
#include "imgui.h"
//제품 글자 색의 Colors::White 등을 읽는다.
#include <DirectXColors.h>
// 초기 장치·창 설정, 렌더링 조정값, 공용 shader의 정점 layout을 엔진에 전달한다.
#include "Engine_InitTypes.h"
#include "Engine_RenderTypes.h"
#include "Engine_VertexTypes.h"

// wWinMain이 호출하는 MainApp의 진입 함수와 제품 창·도구 보유 상태를 선언한다.
#include "MainApp.h"
// 전환 전에 캐릭터 자원 비동기 준비를 취소·대기해 이전 작업이 다음 Level로 넘어가지 않게 한다.
#include "PlayableCharacterAssetService.h"
// Server 제한 시간·월드 체력을 표시하는 view를 MainApp이 생성·보유하고 현재 HUD snapshot으로 갱신한다.
#include "DungeonTimerView.h"
#include "WorldHealthBarView.h"
// 보스 mechanic 막대의 머리 위치를 화면에 투영할 때 이름표의 공통 투영 함수를 사용한다.
#include "WorldPlayerNameplateView.h"

// 선택 창의 세션 roster와 입장 pending identity를 연결하고 실제 Level 전환 뒤 확정한다.
#include "CharacterRoster.h"
#include "CharacterSelectionState.h"
#include "CharacterSelectWindowView.h"
// 현재 Level의 marker·채팅·전투 snapshot을 제품 창에 전달하고 창이 내놓은 intent를 소비한다.
#include "MinimapView.h"
#include "ChatWindowView.h"
#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "Effect_Catalog.h"
#include "EstherCutinPresentationService.h"
#include "RaidBossShowcaseService.h"
#include "EstherActionSoundCueDocument.h"
#include "Effect_Object.h"
#include "Effect_PresentationService.h"
#include "EffectV2_Runtime.h"
#include "EffectV2_Catalog.h"
#include "KoukuSaydonPatternAuditionService.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "Valtan.h"
#include "KoukuSaydonCompositionDocument.h"
#include "PlayerCommandSink.h"
// 엔진 관리자 생성·프레임 갱신·그리기·종료를 요청하는 단일 접근점이다. MainApp은 각 관리자를 직접 소유하지 않는다.
#include "GameInstance.h"
// 제품 루프에 ImGui frame을 열고 닫으며 창의 입력 점유를 gameplay 차단에 반영한다.
#include "ImGuiLayer.h"
#include "ItemCatalog.h"
// 목적지의 등록 정보와 생성 함수를 찾고, TransitionService에 예약된 LOAD/ACTIVATE를 안전한 프레임 경계에서 소비한다.
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "Level_Bern.h"
#include "CharacterPortraitRenderer.h"
#include "Level_Development.h"
#include "Gameplay/MaharakaWaterpangContract.h"
#include "Level_Lobby.h"
#include "ActionPresentationTimeline.h"
#include "Level_CharacterSelect.h"
#include "Level_KoukuSaydonArena.h"
#include "Level_Loading.h"
#include "Level_ValtanArena.h"
#include "LobbyCommandService.h"
// 수신 dispatch와 수리·구매·장착 요청 전송, 연결 종료를 담당한다. 이동·스킬 요청은 현재 Level의 command sink 경계를 이용한다.
#include "NetworkManager.h"
#include "Npc.h"
#include "PartyWindowView.h"
#include "UITextOcclusion.h"
#include "PlayerSkillCatalog.h"
// 엔진·UI·도구 단계의 CPU/GPU 시간을 기록하고 F7 표시 도구에 엔진의 측정기를 연결한다.
#include "Profiler.h"
#include "ProfilerTool.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "EffectFailureDiagnostic.h"
#include "Presentation_Manager.h"
// 저작 JSON은 ProjectDataRoot, 실행 이미지·모델은 RuntimeAssetRoot 아래에서 찾아 두 경로의 역할을 구분한다.
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "UserSettingsDocument.h"
#include "WorldGameplayDocument.h"
#include "WorldSequencePlayer.h"
#include <set>
#include "AvatarBookWindowView.h"
#include "UILabelFont.h"
#include "CharacterInfoWindowView.h"
#include "VehicleWindowView.h"
#include "CustomizingView.h"
#include "SystemOptionWindowView.h"
#include "ClientWindowDisplay.h"
#include "CombatAnalysisFrameView.h"
#include "HonorTitleCatalog.h"
#include "HonorTitleWindowView.h"
#include "WorldMapWindowView.h"
#include "SongCastGaugeView.h"
#include "Network/PacketMessages.h"
#include "InventoryView.h"
#include "RepairWindowView.h"
#include "ShopWindowView.h"
#include "DurabilityHudView.h"
#include "QuickSlotDragView.h"
#include "SkillWindowView.h"
#include "SkillGroundTargetPreview.h"
#include "ClickMoveEffect.h"
#include "SoundCueCatalog.h"
#include "UI_Sprite.h"
// 여러 제품 창의 클릭·텍스트 입력 점유를 모아 gameplay 입력에 전달한다. UILayoutRuntime은 저작 슬롯을 STATIC Layer sprite로 만든다.
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

// MainApp이 F1 요청으로 도구를 지연 생성하고 현재 Level·문서·preview 소유자를 연결한다.
#include "ActorCatalog.h"
#include "Animation_Tool.h"
#include "KoukuSaydonActionWorkbench.h"
#include "KoukuSaydonBossTool.h"
#include "ValtanActionWorkbench.h"
#include "BalanceTool.h"
#include "BalanceTestPanel.h"
#include "ValtanBossTool.h"
#include "CameraTool.h"
#include "Camera_Free.h"
#include "CameraDebugControls.h"
#ifdef _DEBUG
#include "VehicleDebugPanel.h"
#endif
#include "CharacterPreviewPanel.h"
#include "ClientReplication.h"
#include "Effect_Tool.h"
#include "Effect_Tool_V2.h"
#include "EquipmentAuthoringTool.h"
#include "HUDLayoutTool.h"
#include "MapAssetRenderUtils.h"
#include "MapEditorWorkspaceService.h"
#include "LevelNavigationDebug.h"
#include "MapTool.h"
#include "NetworkPlayerCommandSink.h"
#include "RenderingBenchmark.h"
#include "SequencerTool.h"
#include "CharacterActionWorkbench.h"
#include "EffectAuthoringSequencer.h"
#include "WorldObjectTool.h"
#include "WorldLevelTool.h"
#include "WorldSceneTool.h"
#include "GuideAITool.h"
#include "MaharakaAITool.h"
#include "ValtanPatternAuditionService.h"
#include "ValtanPatternFlowService.h"
#include "ValtanTuningCommandService.h"

//UISoundPlayer, OPEN과 CLOSE sound에 대한 enum을 제공해준다.
//실제 물리 wav 폴더를 쓰지 않도록 한다.
#include "UISoundPlayer.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <random>
#include <tuple>
#include <cwchar>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>

namespace
{
	constexpr const char* HUD_BAR_POSITION_KEYS[] = {
		"mechanicHeadOffsetX", "mechanicHeadOffsetY", "allyOffsetX", "allyOffsetY", "enemyOffsetX", "enemyOffsetY",
		"koukuSaydonOffsetX", "koukuSaydonOffsetY", "koukuOffsetX", "koukuOffsetY", "valtanOffsetX", "valtanOffsetY" };
	constexpr const char* HUD_MECHANIC_SCALE_KEYS[] = { "mechanicWidthScale", "mechanicHeightScale" };
	constexpr const char* HUD_VALTAN_MECHANIC_KEYS[] = {
		"valtanStaggerHeadOffsetX", "valtanStaggerHeadOffsetY", "valtanStaggerWidthScale", "valtanStaggerHeightScale",
		"valtanArmorBreakOffsetX", "valtanArmorBreakOffsetY", "valtanArmorBreakWidthScale", "valtanArmorBreakHeightScale" };

	// 보스 HP 표시가 발탄 본체·유령의 stable archetype ID를 같은 계열로 분류한다.
	bool IsValtanBossArchetype(const std::string& id)
	{
		return id == "BOSS_VALTAN" || id == "BOSS_VALTAN_GHOST";
	}

	constexpr const char* HUD_MECHANIC_SLOTS[] = { "Boss_StaggerBg", "Boss_StaggerTrack", "Boss_StaggerFill" };

	// HUD 위치 문서의 byte를 읽는다. 파일 열기 실패는 출력을 보존하지만 읽기 도중 실패는 출력 문자열을 바꿨을 수 있다.
	bool ReadHudBarText(const std::filesystem::path& path, std::string& text)
	{
		std::ifstream stream(path, std::ios::binary);
		if (!stream.is_open()) return false;
		text.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
		return !stream.bad();
	}

	// 로드·저장 검증이 schema와 각 숫자의 finite·범위를 확인해 지역 배열에 준비한다.
	// 전부 성공한 뒤 세 출력 배열을 함께 바꾸므로 검증 실패는 이전 출력값을 보존한다.
	bool ReadHudBarPositions(const Client::DATA_JSON_VALUE& root, std::array<f32_t, 12>& offsets,
		std::array<f32_t, 2>& mechanicScale, std::array<f32_t, 8>& valtanTuning)
	{
		const auto* schema = root.Find("schema");
		const auto* version = root.Find("formatVersion");
		if (!root.Is_Object() || !schema || !schema->Is_String() ||
			schema->Get_String() != "lostark.kouku-hud-modes" || !version ||
			!version->Is_Number() || version->Get_Number() != 1.0) return false;
		// 기존 mechanicOffsetX/Y는 화면 기준 저장값이다. 새 머리 기준 offset은 0에서 시작해 이전 값을 다른 좌표계로 재해석하지 않는다.
		// 이전 저장 field는 문서에 보존하고 새 좌표계의 기본값만 별도로 준비한다.
		std::array<f32_t, 12> staged{};
		std::array<f32_t, 2> stagedScale{ 1.f / 3.f, 1.f };
		if (const auto* positions = root.Find("healthBarPositions"))
		{
			if (!positions->Is_Object()) return false;
			for (size_t i = 0u; i < staged.size(); ++i)
				if (const auto* value = positions->Find(HUD_BAR_POSITION_KEYS[i]))
				{
					if (!value->Is_Number() || !std::isfinite(value->Get_Number()) ||
						std::abs(value->Get_Number()) > 1280.0) return false;
					staged[i] = static_cast<f32_t>(value->Get_Number());
				}
			// 보스별 field가 없는 이전 문서는 공통 적 X/Y를 초기값으로 물려받는다. 개별 저작값이 있으면 그대로 유지한다.
			for (size_t i = 6u; i < staged.size(); ++i)
				if (!positions->Find(HUD_BAR_POSITION_KEYS[i])) staged[i] = staged[4u + i % 2u];
			for (size_t i = 0u; i < stagedScale.size(); ++i)
				if (const auto* value = positions->Find(HUD_MECHANIC_SCALE_KEYS[i]))
				{
					if (!value->Is_Number() || !std::isfinite(value->Get_Number()) ||
						value->Get_Number() < 0.1 || value->Get_Number() > 3.0) return false;
					stagedScale[i] = static_cast<f32_t>(value->Get_Number());
				}
		}
		// 로드 때 기존 쿠크 튜닝을 초기값으로 한 번만 이어받고 이후 각 보스를 독립적으로 편집한다.
		std::array<f32_t, 8> stagedValtan{ staged[0], staged[1], stagedScale[0], stagedScale[1], -24.f, -40.f, 1.f, 1.f };
		if (const auto* positions = root.Find("healthBarPositions"))
			for (size_t i = 0u; i < stagedValtan.size(); ++i)
				if (const auto* value = positions->Find(HUD_VALTAN_MECHANIC_KEYS[i]))
				{
					if (!value->Is_Number() || !std::isfinite(value->Get_Number())) return false;
					const double number = value->Get_Number();
					if (i % 4u < 2u ? std::abs(number) > 1280.0 : number < 0.1 || number > 3.0) return false;
					stagedValtan[i] = static_cast<f32_t>(number);
				}
		offsets = staged;
		mechanicScale = stagedScale;
		valtanTuning = stagedValtan;
		return true;
	}

	// HUD 저장이 기존 JSON의 모든 field를 재귀적으로 직렬화한다. 문자열은 Escape하고 숫자는 17자리 정밀도로 쓴다.
	// 스트림 소유권과 최종 I/O 성공 확인은 호출자에게 있으며 여기서는 반환 상태를 제공하지 않는다.
	void WriteHudBarJson(std::ostream& out, const Client::DATA_JSON_VALUE& value, const size_t depth = 0u)
	{
		using Client::DATA_JSON_TYPE;
		switch (value.Get_Type())
		{
		case DATA_JSON_TYPE::NULL_VALUE: out << "null"; break;
		case DATA_JSON_TYPE::BOOLEAN: out << (value.Get_Boolean() ? "true" : "false"); break;
		case DATA_JSON_TYPE::NUMBER: out << std::setprecision(17) << value.Get_Number(); break;
		case DATA_JSON_TYPE::STRING: out << '"' << Client::CDataJson::Escape(value.Get_String()) << '"'; break;
		case DATA_JSON_TYPE::ARRAY:
			out << '[';
			for (size_t i = 0u; i < value.Get_Array().size(); ++i)
			{ if (i) out << ", "; WriteHudBarJson(out, value.Get_Array()[i], depth + 1u); }
			out << ']'; break;
		case DATA_JSON_TYPE::OBJECT:
			out << '{';
			{ bool first = true;
			for (const auto& [key, item] : value.Get_Object())
			{
				out << (first ? "\n" : ",\n") << std::string((depth + 1u) * 2u, ' ') << '"'
					<< Client::CDataJson::Escape(key) << "\": ";
				WriteHudBarJson(out, item, depth + 1u); first = false;
			} }
			out << '\n' << std::string(depth * 2u, ' ') << '}'; break;
		}
	}

#ifdef _DEBUG
    // 도구에 전달할 class movie 상태 조회·검사 명령 callback을 만든다.
    // Level 포인터를 장기 보관하지 않고 호출 때 현재 Character Select를 확인하며 배경 열기 callback의 소유권은 이동한다.
    Client::CLASS_MOVIE_INSPECTION_CALLBACKS ClassMovieInspectionCallbacks(
        Client::CLASS_MOVIE_INSPECTION_CALLBACKS::OPEN_BACKGROUND openBackground)
    {
        Client::CLASS_MOVIE_INSPECTION_CALLBACKS callbacks;
        callbacks.openBackground = std::move(openBackground);
        callbacks.state = [canOpenBackground = static_cast<bool>(callbacks.openBackground)](const std::string& classId, bool loop) {
            auto* level = CLevel_CharacterSelect::Get_Active();
            if (level && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
            {
                auto state = level->Get_ClassSelectionPresentation().Get_WorldInspection(classId, loop);
                state.backgroundAreaId = level->Get_ClassCinematicBackgroundArea(classId);
                state.canEditBackground = canOpenBackground && state.active && level->Get_ClassMovieId() == classId &&
                    !state.backgroundAreaId.empty() && level->Find_ClassCinematicBackgroundRuntime(state.backgroundAreaId) != nullptr;
                return state;
            }
            Client::CLASS_MOVIE_INSPECTION_STATE state;
            state.classId = classId; state.loop = loop;
            state.status = "Enter Character Select to inspect Movie models.";
            return state;
        };
        callbacks.command = [](const std::string& classId, bool loop,
            const Client::CLASS_MOVIE_INSPECTION_COMMAND& command, std::string& status) {
            auto* level = CLevel_CharacterSelect::Get_Active();
            if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
            { status = "Movie inspection requires Character Select."; return false; }
            return level->Get_ClassSelectionPresentation().Inspect_World(classId, loop, command, status);
        };
        return callbacks;
    }
    // 기존 effect 도구를 Character Select와 발탄 입장 연출의 Level-owned player에 연결한다.
    // play·seek·pause·선택 preview callback은 호출 때 현재 Level을 확인하고 불가 상태는 오류로 돌려준다.
    void ConfigureClassMovieEditor(Client::CEffect_Tool& tool,
        Client::CLASS_MOVIE_INSPECTION_CALLBACKS::OPEN_BACKGROUND openBackground)
    {
        CEffect_Tool::CLASS_MOVIE_CALLBACKS movieCallbacks;
        movieCallbacks.state = [](const std::string& classId) {
            CEffect_Tool::CLASS_MOVIE_STATE state;
            if (classId == "valtan.entrance")
            {
                auto* arena = CLevel_ValtanArena::Get_Active();
                if (!arena || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::VALTAN_ARENA))
                { state.status = "Enter Valtan Arena to inspect the entrance scene."; return state; }
                const auto& preview = arena->Debug_GetEffectCinematicState();
                state.available = preview.available; state.active = preview.active; state.paused = preview.paused;
                state.clockMs = preview.clockMs; state.durationMs = preview.durationMs; state.playbackRate = preview.playbackRate;
                state.selectionActive = preview.selectionActive; state.selectionRepeat = preview.selectionRepeat;
                state.status = preview.status;
                return state;
            }
            auto* level = CLevel_CharacterSelect::Get_Active();
            if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
            { state.status = "Enter Character Select to edit this Movie."; return state; }
            const auto& preview = level->Get_ClassSelectionPresentation();
            state.available = level->Can_PlayClassCinematic() && preview.Has_Class(classId);
            state.active = preview.Is_Active() && preview.Get_ActiveClass() == classId;
            state.paused = preview.Is_Paused(); state.loop = preview.Is_Looping();
            state.clockMs = preview.Get_ClockMs(); state.durationMs = preview.Get_DurationMs();
            state.playbackRate = preview.Get_PlaybackRate();
            state.selectionActive = preview.Is_SelectionActive();
            state.selectionRepeat = preview.Is_SelectionRepeating();
            state.status = level->Get_ClassCinematicStatus();
            return state;
        };
        movieCallbacks.timeline = [](const std::string& classId, bool loop) -> std::shared_ptr<const CLASS_MOVIE_TIMELINE> {
            if (classId == "valtan.entrance")
            {
                auto* arena = CLevel_ValtanArena::Get_Active();
                return arena && !loop && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA) ?
                    arena->Debug_GetEffectCinematicTimeline() : nullptr;
            }
            auto* level = CLevel_CharacterSelect::Get_Active();
            return level && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT) ?
                level->Get_ClassSelectionPresentation().Get_Timeline(classId, loop) : nullptr;
        };
        movieCallbacks.play = [](const std::string& classId, std::string& status) {
            if (classId == "valtan.entrance")
            {
                auto* arena = CLevel_ValtanArena::Get_Active();
                if (!arena || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::VALTAN_ARENA))
                { status = "Enter Valtan Arena to play the entrance scene."; return false; }
                return arena->Debug_PlayEffectCinematic(status);
            }
            auto* level = CLevel_CharacterSelect::Get_Active();
            if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
            { status = "Enter Character Select to play this Movie."; return false; }
            const auto& options = level->Get_ClassMovieOptions();
            const auto option = std::find_if(options.begin(), options.end(), [&](const auto& value) { return value.classId == classId; });
            const bool played = option != options.end() && level->Select_ClassMovieCategory(static_cast<size_t>(std::distance(options.begin(), option))) &&
                level->Can_PlayClassCinematic() && level->Play_ClassCinematic(classId);
            status = level->Get_ClassCinematicStatus();
            return played;
        };
        movieCallbacks.seek = [](const std::string& classId, bool loop, double timeMs, std::string& status) {
            if (classId == "valtan.entrance")
            {
                auto* arena = CLevel_ValtanArena::Get_Active();
                if (!arena || loop || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::VALTAN_ARENA))
                { status = "Enter Valtan Arena to scrub the entrance scene."; return false; }
                return arena->Debug_SeekEffectCinematic(timeMs, status);
            }
            auto* level = CLevel_CharacterSelect::Get_Active();
            if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT) || !level->Can_PlayClassCinematic())
            { status = "Wait for Character Select admission and close other character previews."; return false; }
            const auto& options = level->Get_ClassMovieOptions();
            const auto option = std::find_if(options.begin(), options.end(), [&](const auto& value) { return value.classId == classId; });
            if (option == options.end() || !level->Select_ClassMovieCategory(static_cast<size_t>(std::distance(options.begin(), option))))
            { status = "This Movie category is no longer available."; return false; }
            auto& preview = level->Get_ClassSelectionPresentation();
            // 공용 seek 전에 Level의 Play로 선택한 배경과 현재 연출을 먼저 준비한다.
            if ((!preview.Is_Active() || preview.Get_ActiveClass() != classId) && !level->Play_ClassCinematic(classId))
            { status = level->Get_ClassCinematicStatus(); return false; }
            const bool sampled = preview.Seek(classId, loop, timeMs);
            if (sampled) preview.Set_Paused(true);
            status = level->Get_ClassCinematicStatus();
            return sampled;
        };
        movieCallbacks.pause = [](bool paused) {
            if (auto* arena = CLevel_ValtanArena::Get_Active(); arena &&
                CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
            { arena->Debug_PauseEffectCinematic(paused); return; }
            if (auto* level = CLevel_CharacterSelect::Get_Active()) level->Get_ClassSelectionPresentation().Set_Paused(paused);
        };
        movieCallbacks.stop = [] {
            if (auto* arena = CLevel_ValtanArena::Get_Active(); arena &&
                CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
            { arena->Debug_StopEffectCinematic(); return; }
            if (auto* level = CLevel_CharacterSelect::Get_Active()) level->Get_ClassSelectionPresentation().Stop();
        };
        movieCallbacks.preview = [](const EFFECT_DOCUMENT_DESC& document, std::string& status) {
            if (auto* arena = CLevel_ValtanArena::Get_Active(); arena &&
                CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
                return arena->Debug_PreviewCinematicEffect(document, status);
            auto* level = CLevel_CharacterSelect::Get_Active();
            if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
            { status = "Movie Effect preview requires Character Select."; return false; }
            return level->Get_ClassSelectionPresentation().Preview_EffectDocument(document, status);
        };
        movieCallbacks.previewVisibility = [](const EFFECT_DOCUMENT_DESC& document,
            const std::vector<std::string>& drawElementIds, std::string& status) {
            auto* level = CLevel_CharacterSelect::Get_Active();
            if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
            { status = "Movie Element visibility requires Character Select."; return false; }
            return level->Get_ClassSelectionPresentation().Preview_EffectDocument(document, status, &drawElementIds);
        };
        movieCallbacks.playSelection = [](const std::string& classId, bool loop,
            const EFFECT_DOCUMENT_DESC& full, const EFFECT_DOCUMENT_DESC& selected,
            const std::vector<std::string>& drawElementIds, double startAgeMs, double endAgeMs, bool repeat, std::string& status) {
            if (classId == "valtan.entrance")
            {
                auto* arena = CLevel_ValtanArena::Get_Active();
                if (!arena || loop || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::VALTAN_ARENA))
                { status = "Enter Valtan Arena to play entrance Elements."; return false; }
                return arena->Debug_PlayCinematicEffectSelection(full, selected, drawElementIds, startAgeMs, endAgeMs, repeat, status);
            }
            auto* level = CLevel_CharacterSelect::Get_Active();
            if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
            { status = "Enter Character Select to play Movie elements."; return false; }
            const auto& options = level->Get_ClassMovieOptions();
            const auto option = std::find_if(options.begin(), options.end(), [&](const auto& value) { return value.classId == classId; });
            if (option == options.end() || !level->Select_ClassMovieCategory(static_cast<size_t>(std::distance(options.begin(), option))))
            { status = "This Movie category is no longer available."; return false; }
            return level->Play_ClassCinematicSelection(classId, loop, full, selected, drawElementIds, startAgeMs, endAgeMs, repeat, status);
        };
        movieCallbacks.previewSelection = [](const EFFECT_DOCUMENT_DESC& full,
            const EFFECT_DOCUMENT_DESC& selected, const std::vector<std::string>& drawElementIds, double startAgeMs, double endAgeMs, std::string& status) {
            if (auto* arena = CLevel_ValtanArena::Get_Active(); arena &&
                CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
                return arena->Debug_PreviewCinematicEffectSelection(full, selected, drawElementIds, startAgeMs, endAgeMs, status);
            auto* level = CLevel_CharacterSelect::Get_Active();
            if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
            { status = "Movie Effect preview requires Character Select."; return false; }
            return level->Get_ClassSelectionPresentation().Preview_EffectSelection(full, selected, drawElementIds, startAgeMs, endAgeMs, status);
        };
        movieCallbacks.selectEffectOccurrence = [](const std::string& trackId, std::string& status) {
            if (auto* arena = CLevel_ValtanArena::Get_Active(); arena &&
                CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
                return arena->Debug_SelectCinematicEffectOccurrence(trackId, status);
            status = "Effect occurrence selection requires the Valtan entrance editor.";
            return false;
        };
        movieCallbacks.setPlaybackRate = [](double rate) {
            if (auto* arena = CLevel_ValtanArena::Get_Active(); arena &&
                CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
                return arena->Debug_SetEffectCinematicPlaybackRate(rate);
            auto* level = CLevel_CharacterSelect::Get_Active();
            return level && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT) &&
                level->Get_ClassSelectionPresentation().Set_PlaybackRate(rate);
        };
        movieCallbacks.clearPreviews = [](std::string& status) {
            if (auto* arena = CLevel_ValtanArena::Get_Active(); arena &&
                CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
                return arena->Debug_ClearCinematicEffectPreviews(status);
            auto* level = CLevel_CharacterSelect::Get_Active();
            return !level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT) ||
                level->Get_ClassSelectionPresentation().Clear_EffectPreviews(status);
        };
        movieCallbacks.inspection = ClassMovieInspectionCallbacks(std::move(openBackground));
        tool.Set_ClassMovieCallbacks(std::move(movieCallbacks));
    }
#endif
	// MVP 결과 화면은 전면 modal이다. 표시 중에는 전투 HUD·보스 bar와 그 글자의 표시를 함께 제한한다.
	bool_t Is_MvpResultPageOpen()
	{
		const CLevel_KoukuSaydonArena* pArena = CLevel_KoukuSaydonArena::Get_Active();
		if (nullptr != pArena && pArena->Debug_Is_MvpResultVisible())
			return true;
		const CLevel_ValtanArena* pValtan = CLevel_ValtanArena::Get_Active();
		return nullptr != pValtan && pValtan->Is_MvpResultVisible();
	}

	// 제품 표시 타이머가 사용하는 최초 호출 이후의 단조 증가 초다.
	// ImGui frame 시계에 의존하지 않으며 실제 날짜/시각을 나타내는 벽시계는 아니다.
	f64_t Product_Now_Seconds()
	{
		static const std::chrono::steady_clock::time_point s_Epoch =
			std::chrono::steady_clock::now();
		return std::chrono::duration<f64_t>(
			std::chrono::steady_clock::now() - s_Epoch).count();
	}

	// Lobby_Layout.json의 1280×720 슬롯에 접속(서버 선택)·종료·환경설정을 배치한다.
	// 서버 이름·상태·인원은 LobbyServers.json의 표시 데이터이며 계정 서버 응답이 아니다.
	constexpr size_t LOBBY_SERVER_ROW_COUNT = 8;
	// 37장 로고 도입부를 20fps로 재생한 뒤 Logo_Final 정지 그림으로 넘기는 초 단위 길이다.
	constexpr f32_t LOBBY_LOGO_INTRO_SECONDS = 37.f / 20.f;

	struct LOBBY_PRODUCT_RECT final
	{
		f32_t fX = 0.f;
		f32_t fY = 0.f;
		f32_t fWidth = 0.f;
		f32_t fHeight = 0.f;
	};

	// 좌표와 크기가 finite이고 너비·높이가 양수인지 확인한다. viewport 내부 여부까지 제한하지는 않는다.
	bool_t Is_ValidProductRect(const LOBBY_PRODUCT_RECT& Rect)
	{
		return std::isfinite(Rect.fX) && std::isfinite(Rect.fY) &&
			std::isfinite(Rect.fWidth) && std::isfinite(Rect.fHeight) &&
			Rect.fWidth > 0.f && Rect.fHeight > 0.f;
	}

	// UI view에서 받은 rect를 검증한다. false여도 view가 출력 성분을 이미 채웠을 수 있으므로 호출자는 반환값을 먼저 확인한다.
	bool_t Get_LobbySlotRect(Client::CUILayoutRuntime* pView, const char* pSlotId,
		LOBBY_PRODUCT_RECT& outRect)
	{
		return nullptr != pView && pView->Get_SlotRect(pSlotId,
			outRect.fX, outRect.fY, outRect.fWidth, outRect.fHeight) &&
			Is_ValidProductRect(outRect);
	}



	// Release 피해 숫자에도 사용할 수 있도록 12345를 12,345 형식의 소유 wide 문자열로 만든다.
	wstring Format_ThousandsSeparated(uint32_t iValue)
	{
		const wstring strDigits = std::to_wstring(iValue);
		wstring strResult;
		int32_t iDigitsSinceComma = 0;
		for (auto it = strDigits.rbegin(); it != strDigits.rend(); ++it)
		{
			if (0 != iDigitsSinceComma && 0 == iDigitsSinceComma % 3)
				strResult.push_back(L',');
			strResult.push_back(*it);
			++iDigitsSinceComma;
		}
		std::reverse(strResult.begin(), strResult.end());
		return strResult;
	}

	// 재련 목록·선택·결과 표시가 같은 item ID·이름·아이콘 값을 사용한다.
	// 복제 인벤토리에서 매번 새 목록을 구성하므로 별도 cache 무효화 없이 다음 snapshot이 표시된다.
	struct ITEM_UPGRADE_SLOT_INFO
	{
		string strItemId;
		wstring strName;
		string strIconPath;
		LostArk::Shared::EQUIPMENT_SLOT eSlot = LostArk::Shared::EQUIPMENT_SLOT::NONE;
		uint16_t iUpgradeLevel = 0;
	};

	// 표시 이름의 UTF-8 유효성을 검사하며 wide 문자열을 만든다. 먼저 출력을 비우므로 실패 시 이전 문자열은 보존하지 않는다.
	bool_t ConvertUtf8ToWide(const string& strUtf8, wstring& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return false;
		const int iRequiredLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), nullptr, 0);
		if (iRequiredLength <= 0)
			return false;
		outWide.resize(static_cast<size_t>(iRequiredLength));
		return iRequiredLength == MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), outWide.data(), iRequiredLength);
	}

	// 복제 인벤토리에 실제 보유한 강화 가능 장비만 목록으로 만든다. 장착 장비도 포함하며 순서는 snapshot을 따른다.
	vector<ITEM_UPGRADE_SLOT_INFO> BuildItemUpgradeSlots()
	{
		vector<ITEM_UPGRADE_SLOT_INFO> slots;
		for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item :
			Client::CCombatHUDViewModel::Get().Get_Inventory().Items)
		{
			const ITEM_DEFINITION* pDefinition = CItemCatalog::Find_ById(item.strItemId);
			if (!item.iQuantity || nullptr == pDefinition || "combat" != pDefinition->strCategory)
				continue;
			if (!LostArk::Shared::Is_Upgradeable_Equipment(pDefinition->strItemId, pDefinition->strEquipSlot))
				continue;
			wstring strName;
			if (!ConvertUtf8ToWide(pDefinition->strDisplayName, strName))
				continue;
			ITEM_UPGRADE_SLOT_INFO info{};
			info.strItemId = pDefinition->strItemId;
			info.strName = std::move(strName);
			info.strIconPath = pDefinition->strIconPath;
			info.eSlot = item.eEquippedSlot;
			info.iUpgradeLevel = item.iUpgradeLevel;
			slots.push_back(std::move(info));
		}
		return slots;
	}


	// 저작 예시 아이콘을 실제 보유 장비로 바꾸고 표시 가능한 최대 6행 안에서 선택 index를 제한한다.
	void SyncItemUpgradeSlotSprites(CUILayoutRuntime& view,
		const vector<ITEM_UPGRADE_SLOT_INFO>& slots, int32_t& selected)
	{
		const int32_t count = static_cast<int32_t>((std::min)(slots.size(), size_t{6}));
		for (int32_t i = 0; i < 6; ++i)
		{
			const string suffix = to_string(i);
			view.Set_SlotVisible("ItemUpgrade_ListItemIcon" + suffix, i < count);
			view.Set_SlotVisible("ItemUpgrade_ListGradeBg" + suffix, i < count);
			if (i < count)
				view.Set_SlotTexture("ItemUpgrade_ListItemIcon" + suffix, slots[i].strIconPath);
		}
		view.Set_SlotVisible("ItemUpgrade_ListSelectedExample", count > 0);
		view.Set_SlotVisible("ItemUpgrade_SelectedItemIcon", count > 0);
		selected = count > 0 ? std::clamp(selected, 0, count - 1) : 0;
		if (!count) return;
		view.Set_SlotTexture("ItemUpgrade_SelectedItemIcon", slots[selected].strIconPath);
		f32_t x = 0.f, y = 0.f, width = 0.f, height = 0.f;
		if (view.Get_SlotRect("ItemUpgrade_ListGradeBg" + to_string(selected), x, y, width, height))
			view.Set_SlotPosition("ItemUpgrade_ListSelectedExample", x, y);
	}

	// STATIC 재련 슬롯은 명시적으로 표시 상태를 관리한다.
	// 전체 숨김 또는 최초 열기 후 modal·완료 이펙트를 별도로 숨기는 공통 목록이다.
	constexpr const char_t* ITEM_UPGRADE_ALL_SLOTS[] =
	{
		// 중앙 장식의 투명한 가장자리 사이로 월드가 비치지 않도록 창 전체 아래에 두는 불투명 배경이다.
		"ItemUpgrade_WindowBg",
		"ItemUpgrade_SuccessModalBg", "ItemUpgrade_PanelBg", "ItemUpgrade_RecipeIconBgExample",
		"ItemUpgrade_RecipeMaterial0", "ItemUpgrade_RecipeAmount0", "ItemUpgrade_RecipeIconBg1",
		"ItemUpgrade_RecipeMaterial1", "ItemUpgrade_RecipeAmount1", "ItemUpgrade_RecipeIconBg2",
		"ItemUpgrade_RecipeMaterial2", "ItemUpgrade_RecipeAmount2", "ItemUpgrade_DecoIcon",
		"ItemUpgrade_LevelUpMotion2Big", "ItemUpgrade_GaugeFill", "ItemUpgrade_WingedRingGold",
		"ItemUpgrade_SelectedItemIconBounds", "ItemUpgrade_LevelUpBtn", "ItemUpgrade_EquipExpPageLine",
		"ItemUpgrade_SmeltGlow", "ItemUpgrade_CoreFlash", "ItemUpgrade_ShockwaveRing",
		"ItemUpgrade_CompleteEffect", "ItemUpgrade_WingDecoFade", "ItemUpgrade_LeftListBg",
		"ItemUpgrade_ListGradeBg0", "ItemUpgrade_SelectedItemGradeBg", "ItemUpgrade_SelectedItemIcon",
		"ItemUpgrade_ItemNameLabel", "ItemUpgrade_CurLevelLabel", "ItemUpgrade_NextLevelLabel",
		"ItemUpgrade_ListGradeBg1", "ItemUpgrade_ListGradeBg2", "ItemUpgrade_ListGradeBg3",
		"ItemUpgrade_RightGradeListBg", "ItemUpgrade_GradeRowEmblem0", "ItemUpgrade_GradeRowEmblem1",
		"ItemUpgrade_GradeRowEmblem2", "ItemUpgrade_GradeRowEmblem3", "ItemUpgrade_GradeRowEmblem4",
		"ItemUpgrade_GradeRowEmblem5", "ItemUpgrade_GradeRowEmblem6", "ItemUpgrade_GradeStripB0",
		"ItemUpgrade_GradeStripB1", "ItemUpgrade_ListSelectedExample", "ItemUpgrade_GradeStripB2",
		"ItemUpgrade_GradeStripB6", "ItemUpgrade_GradeStripB5", "ItemUpgrade_GradeStripB4",
		"ItemUpgrade_GradeStripB3", "ItemUpgrade_GradeSelectedExample", "ItemUpgrade_ListItemIcon0",
		"ItemUpgrade_ReforgeButton", "ItemUpgrade_ListLevel0", "ItemUpgrade_ListItemName0",
		"ItemUpgrade_ListLevel1", "ItemUpgrade_ListItemName1", "ItemUpgrade_ListLevel2",
		"ItemUpgrade_ListItemName2", "ItemUpgrade_ListLevel3", "ItemUpgrade_ListItemName3",
		"ItemUpgrade_ListLevel4", "ItemUpgrade_ListItemName4", "ItemUpgrade_ListLevel5",
		"ItemUpgrade_ListItemName5", "ItemUpgrade_GradeRowText6", "ItemUpgrade_GradeRowText0",
		"ItemUpgrade_GradeRowText5", "ItemUpgrade_GradeRowText1", "ItemUpgrade_GradeRowText4",
		"ItemUpgrade_GradeRowText2", "ItemUpgrade_GradeRowText3", "ItemUpgrade_ListItemIcon1",
		"ItemUpgrade_ListItemIcon2", "ItemUpgrade_ListItemIcon3", "ItemUpgrade_ListItemIcon4",
		"ItemUpgrade_ListGradeBg4", "ItemUpgrade_ListGradeBg5", "ItemUpgrade_LevelArrowBase",
		"ItemUpgrade_LevelArrow", "ItemUpgrade_ListItemIcon5", "ItemUpgrade_FailModalBg",
		"ItemUpgrade_ResultWaitBg", "ItemUpgrade_ResultWaitEmblem", "ItemUpgrade_SuccessEffect",
		"ItemUpgrade_FailEffect", "ItemUpgrade_SuccessDiamondWinged", "ItemUpgrade_SuccessDiamondFrame",
		"ItemUpgrade_SuccessItemIconMarker", "ItemUpgrade_SuccessItemNameMarker",
		"ItemUpgrade_SuccessGradeMarker", "ItemUpgrade_SuccessStatusMarker",
		"ItemUpgrade_FailDiamondFrame", "ItemUpgrade_FailItemIconMarker",
		"ItemUpgrade_FailItemNameMarker", "ItemUpgrade_FailStatusMarker",
		"ItemUpgrade_SuccessOkBtn", "ItemUpgrade_FailOkBtn",
	};

#ifdef _DEBUG

	// Debug HUD 도구의 class 이름을 고른다. 미지원 enum은 도구의 Default 그룹으로 연결한다.
	const char_t* GetHUDLayoutClassId(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "LanceMaster";
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return "DimensionMaster";
		case CHARACTER_CLASS_ID::WARLORD:
			return "Warlord";
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT:
			return "GuardianKnight";
		default:
			return "Default";
		}
	}
#endif

	// HUD_Layout.json의 ownerClass는 schema·도구에서 쓰는 공백 없는 class 이름과 일치해야 한다.
	const string GetHUDOwnerClassName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return "LanceMaster";
		case CHARACTER_CLASS_ID::GUNSLINGER:
			return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER:
			return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST:
			return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return "DimensionMaster";
		case CHARACTER_CLASS_ID::WARLORD:
			return "Warlord";
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT:
			return "GuardianKnight";
		default:
			return "";
		}
	}

	// uint32_t tick의 wrap을 고려해 차이의 부호로 같은 half-range 순서를 사용한다.
	// 0 deadline은 비활성이며 wrap 이후의 미래 시각을 즉시 만료로 보지 않게 한다.
	bool_t Is_ServerDeadlinePending(const uint32_t iServerTick, const uint32_t iDeadlineTick)
	{
		return 0u != iDeadlineTick &&
			static_cast<int32_t>(iServerTick - iDeadlineTick) < 0;
	}

}

CMainApp* CMainApp::s_pActiveInstance = nullptr;

// wWinMain이 보유한 MainApp 주소를 등록해 Level과 메뉴가 공용 UI 열기를 요청할 수 있게 한다. 요청자는 이 객체를 소유하지 않는다.
CMainApp::CMainApp()
{
	s_pActiveInstance = this;
}

// 공용 UI 요청의 접근 주소를 먼저 끊고 Free로 제품·엔진을 종료한다. Create의 초기화 실패도 이 경로를 따른다.
CMainApp::~CMainApp()
{
	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
	Free();
}

// 월드 메뉴가 닫힌 환경설정 창만 연다. 존재하지 않거나 이미 열렸으면 상태를 유지한다.
void CMainApp::Open_SystemOptionsWindow()
{
	if (nullptr != m_pSystemOptionView && !m_pSystemOptionView->Is_Open())
		m_pSystemOptionView->Open();
}

// 월드 메뉴가 캐릭터 선택 복귀를 시작한다. Server capture 대기 등록과 예약·전송 실패 모두 false를 반환한다.
// 즉시 복귀 가능한 경우에는 로비 요청 예약 성공 뒤 연결을 닫고 true를 반환한다.
bool_t CMainApp::Return_ToCharacterSelect()
{
	if (CLevelTransitionService::Is_Pending() || m_bCharacterReturnPending)
		return false;
	CNetworkManager &network = CNetworkManager::Get();
	if (CCharacterSelectionState::Has_ActiveCharacter() && network.Is_Connected() &&
		!CCharacterSelectionState::Is_RestorePending())
	{
		using namespace LostArk::Shared;
		WORLD_ID world = WORLD_ID::END;
		switch (static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID()))
		{
		case LEVEL::BERN:
			world = WORLD_ID::BERN;
			break;
		case LEVEL::VALTAN_ARENA:
			world = WORLD_ID::VALTAN_ARENA;
			break;
		case LEVEL::KOUKUSAYDON_ARENA:
			world = WORLD_ID::KOUKUSAYDON_ARENA;
			break;
		case LEVEL::MAHARAKA:
			world = WORLD_ID::MAHARAKA;
			break;
		case LEVEL::COLOSSEUM:
			world = WORLD_ID::COLOSSEUM;
			break;
		default:
			break;
		}
		if (world != WORLD_ID::END)
		{
			const auto sequence = CCharacterSelectionState::Next_StateRequestSequence();
			const auto generation = network.Get_WorldInboundGeneration();
			if (!CCharacterSelectionState::Begin_WorldStateCapture(sequence, world, network.Get_LocalPlayerId(),
																   network.Get_LocalEntityId(), generation))
				return false;
			CNetworkPlayerCommandSink sink;
			if (!sink.Request_CaptureCharacter(sequence))
			{
				CCharacterSelectionState::Finish_WorldStateCapture();
				CLevelTransitionService::Report_Recovery(
					SESSION_DIAGNOSTIC_REASON::CLIENT_IDENTITY_COMMIT_FAILED, "character.capture-send",
					"Character state could not be requested. Retry character selection.", E_FAIL);
				return false;
			}
			m_bCharacterReturnPending = true;
			m_dCharacterCaptureStartedAt = Product_Now_Seconds();
			m_iCharacterCaptureWorldGeneration = generation;
			// 최종 상태 응답을 받을 때까지 현재 Level의 복제 갱신을 유지한다.
			return false;
		}
	}
	CCharacterSelectionState::Capture_ActiveWorldState();
	if (!CLevelTransitionService::Request_Load(LEVEL::LOBBY, "world.menu.character-select"))
		return false;
	m_bOpenCharacterSelectOnLobby = true;
	network.Close_ServerConnection();
	return true;
}

// Update가 capture 결과와 같은 world generation을 확인해 로비 요청을 마무리한다.
// 5초 timeout·연결 종료·세대 변경·실패는 여기서 슬롯을 다시 덮어쓰지 않고 대기만 정리해 진단을 남긴다.
void CMainApp::Pump_ReturnToCharacterSelect()
{
	if (!m_bCharacterReturnPending)
		return;
	CNetworkManager &network = CNetworkManager::Get();
	const auto status = CCharacterSelectionState::Get_WorldStateCaptureStatus();
	const bool_t sameWorld = network.Get_WorldInboundGeneration() == m_iCharacterCaptureWorldGeneration;
	if (status == CHARACTER_CAPTURE_STATUS::CAPTURED && sameWorld && !CLevelTransitionService::Is_Pending())
	{
		if (!CLevelTransitionService::Request_Load(LEVEL::LOBBY, "world.menu.character-select.saved"))
			return;
		m_bOpenCharacterSelectOnLobby = true;
		m_bCharacterReturnPending = false;
		CCharacterSelectionState::Finish_WorldStateCapture();
		network.Close_ServerConnection();
		return;
	}
	if (status == CHARACTER_CAPTURE_STATUS::FAILED || !sameWorld || !network.Is_Connected() ||
		CLevelTransitionService::Is_Pending() || Product_Now_Seconds() - m_dCharacterCaptureStartedAt >= 5.0)
	{
		m_bCharacterReturnPending = false;
		CCharacterSelectionState::Finish_WorldStateCapture();
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_IDENTITY_COMMIT_FAILED, "character.capture-incomplete",
			"Character state was not confirmed. The previous slot state is preserved; retry character selection.",
			E_FAIL);
	}
}

// 베른 선박 NPC가 탈것 창을 선박 mode로 연다. view가 없으면 아무 상태도 바꾸지 않는다.
void CMainApp::Open_ShipWindow()
{
	if (nullptr != m_pVehicleWindowView)
		m_pVehicleWindowView->Open_Ships();
}

// 베른 수리 NPC가 기존 수리 view의 열기 경로를 호출한다.
void CMainApp::Open_RepairWindow()
{
	if (nullptr != m_pRepairWindowView)
		m_pRepairWindowView->Open();
}

// 상점 NPC의 stable placement ID를 view에 전달한다. 상품 상태·구매 요청은 기존 view 경계가 담당한다.
void CMainApp::Open_ShopWindow(const string &strNpcPlacementId)
{
	if (nullptr != m_pShopWindowView)
		m_pShopWindowView->Open(strNpcPlacementId);
}

// NPC가 처음 열 때 재련 요청 identity와 연출 시각을 초기화하고 보유 장비 sprite를 맞춘다. 이미 열린 창은 보존한다.
void CMainApp::Open_ItemUpgradeWindow()
{
	if (nullptr == m_pItemUpgradeView || m_bItemUpgradePreviewVisible)
		return;

	m_bItemUpgradePreviewVisible = true;
	m_iPendingItemUpgradeRequest = 0;
	m_bItemUpgradeResultUnavailable = false;
	m_bItemUpgradePendingAttemptSuccess = false;
	// 전체 저작 슬롯의 표시 소유권을 명시적으로 설정한 뒤 아래에서 완료 전용·modal·결과 슬롯을 숨긴다.
	for (const char_t *pSlotId : ITEM_UPGRADE_ALL_SLOTS)
		m_pItemUpgradeView->Set_SlotVisible(pSlotId, true);

	// 다시 열면 gauge를 0% 대기로 초기화한다. 이전 완료 이펙트와 예약 시각이 새 화면으로 이어지지 않게 비운다.
	m_iItemUpgradePreviousPercent = 0;
	m_bItemUpgradeGrowing = false;
	m_dItemUpgradeGrowStartSeconds = -1.0;
	m_bItemUpgradeCoreFlashPending = false;
	m_dItemUpgradeShockwaveScheduledAt = -1.0;
	m_dItemUpgradeCompleteRevealStartSeconds = -1.0;
	m_dItemUpgradeResultSettleAt = -1.0;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_WingedRingGold", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_LevelUpMotion2Big", false);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", 0.f);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", 0.f);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_CompleteEffect", false);
	m_eItemUpgradeAttemptResult = ITEM_UPGRADE_ATTEMPT_RESULT::NONE;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessModalBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessOkBtn", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailModalBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailOkBtn", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitEmblem", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessEffect", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailEffect", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessDiamondWinged", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessDiamondFrame", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessItemIconMarker", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailDiamondFrame", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailItemIconMarker", false);
	Set_ItemUpgradeCenterPanelVisible(true);
	SyncItemUpgradeSlotSprites(*m_pItemUpgradeView, BuildItemUpgradeSlots(), m_iItemUpgradeSelectedSlot);
}

// 창 닫기·Level 전환이 WAITING 반복음을 멈추고 현재 요청/결과 표시를 비운다.
// Server 인벤토리는 바꾸지 않으며 STATIC 슬롯만 숨긴다.
void CMainApp::Hide_ItemUpgrade()
{
	if (ITEM_UPGRADE_ATTEMPT_RESULT::WAITING == m_eItemUpgradeAttemptResult)
		CGameInstance::Get().Stop_LoopingSound();
	m_iPendingItemUpgradeRequest = 0;
	m_eItemUpgradeAttemptResult = ITEM_UPGRADE_ATTEMPT_RESULT::NONE;
	if (nullptr == m_pItemUpgradeView)
		return;
	for (const char_t *pSlotId : ITEM_UPGRADE_ALL_SLOTS)
		m_pItemUpgradeView->Set_SlotVisible(pSlotId, false);
}

// 현재 쿠크 관문이 요청한 presentation profile을 변경 edge에서만 적용한다.
// 같은 Level에서 자신이 적용한 profile을 아직 소유한 경우에만 이전 profile 복원을 시도한다.
void CMainApp::Update_KoukuGateSceneProfile()
{
	auto *arena = CLevel_KoukuSaydonArena::Get_Active();
	if (!arena || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::KOUKUSAYDON_ARENA))
	{
		// 새 Level이 현재 profile을 소유하므로 Level을 넘어 이전 쿠크 profile을 복원하지 않는다.
		m_strKoukuGateProfileRequest.clear();
		m_strKoukuGateProfileApplied.clear();
		m_strSceneProfileBeforeKoukuGate.clear();
		return;
	}
	const string requested = arena->Get_GatePresentationProfileId();
	if (requested == m_strKoukuGateProfileRequest)
		return;
	// 관문 요청 변경을 한 번 소비한다. 실패 요청도 기록해 다음 프레임의 다른 시퀀스 profile을 반복 덮어쓰지 않는다.
	// 뒤이어 profile을 소유한 시퀀스의 선택은 그대로 유지한다.
	m_strKoukuGateProfileRequest = requested;
	string status;
	bool changed = false;
	if (!requested.empty())
	{
		const string previous = m_RenderingProfiles.Get_ActiveProfileId();
		changed = m_RenderingProfiles.Activate_Profile(requested, status);
		if (changed)
		{
			if (m_strSceneProfileBeforeKoukuGate.empty())
				m_strSceneProfileBeforeKoukuGate = previous;
			m_strKoukuGateProfileApplied = requested;
		}
	}
	else
	{
		if (!m_strKoukuGateProfileApplied.empty() &&
			m_RenderingProfiles.Get_ActiveProfileId() == m_strKoukuGateProfileApplied &&
			!m_strSceneProfileBeforeKoukuGate.empty())
			changed = m_RenderingProfiles.Activate_Profile(m_strSceneProfileBeforeKoukuGate, status);
		m_strKoukuGateProfileApplied.clear();
		m_strSceneProfileBeforeKoukuGate.clear();
	}
	if (!status.empty())
	{
#ifdef _DEBUG
		m_strRenderingStatus = status;
#endif
		if (!changed)
			OutputDebugStringA(("[MainApp][KoukuGateProfile] " + status + "\n").c_str());
	}
}

// 생성/showcase 화면의 열림 상태로 어두운 무대 profile을 활성화하거나 이전 profile을 복원한다.
// 복원 실패는 로그로 남기고 보관 ID를 비우는 기존 정책을 유지한다.
void CMainApp::Update_CustomizingSceneProfile()
{
	// 캐릭터 생성·showcase 화면의 어두운 무대를 MainApp 소유 profile service로 전환한다.
	// 화면의 필요 상태와 보관된 이전 ID가 달라졌을 때만 활성화/복원을 시도한다.
	static constexpr const char_t *CUSTOMIZING_PROFILE_ID = "scene.character-select.customizing-dark.v1";
	auto *characterSelect = CLevel_CharacterSelect::Get_Active();
	const bool_t wantsDarkStage = nullptr != characterSelect &&
								  (characterSelect->Is_CustomizingOpen() || characterSelect->Is_ClassShowcaseOpen());
	const bool_t holdsDarkStage = !m_strSceneProfileBeforeCustomizing.empty();
	if (wantsDarkStage == holdsDarkStage)
		return;

	string status;
	if (wantsDarkStage)
	{
		const string previous = m_RenderingProfiles.Get_ActiveProfileId();
		if (!m_RenderingProfiles.Activate_Profile(CUSTOMIZING_PROFILE_ID, status))
		{
			// profile 활성화 실패는 화면 전체 시작 실패로 확대하지 않고 이전 밝은 무대 상태와 진단을 유지한다.
			OutputDebugStringA(("[MainApp][SceneProfile] " + status + "\n").c_str());
			return;
		}
		m_strSceneProfileBeforeCustomizing = previous;
		return;
	}

	if (!m_RenderingProfiles.Activate_Profile(m_strSceneProfileBeforeCustomizing, status))
	{
		OutputDebugStringA(("[MainApp][SceneProfile] " + status + "\n").c_str());
	}
	m_strSceneProfileBeforeCustomizing.clear();
}

// showcase 대상의 월드 위치만큼 shadow의 eye와 at을 함께 이동해 기존 방향을 유지한다.
// 직전 소유 focus만 되돌리고 적용 실패는 추적 상태를 새 값으로 확정하지 않는다.
void CMainApp::Update_ClassShowcaseShadowFocus()
{
	auto &game = CGameInstance::Get();
	const auto original = game.Get_ShadowLightDesc();
	if (!original.Settings.bEnabled)
	{
		m_bClassShowcaseShadowFocusActive = false;
		return;
	}
	auto shadow = original;
	const auto samePoint = [](const float4_t &a, const float4_t &b)
	{ return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; };
	// 직전에 자신이 적용한 focus와 아직 같을 때만 그 이동을 되돌린다. 다른 profile/도구가 바꾼 focus는 덮어쓰지 않는다.
	if (m_bClassShowcaseShadowFocusActive && samePoint(shadow.vAt, m_vClassShowcaseShadowFocus))
	{
		const vector_t undo = XMLoadFloat4(&m_vShadowFocusBeforeClassShowcase) - XMLoadFloat4(&shadow.vAt);
		XMStoreFloat4(&shadow.vEye, XMLoadFloat4(&shadow.vEye) + undo);
		shadow.vAt = m_vShadowFocusBeforeClassShowcase;
	}
	const float4_t previousFocus = shadow.vAt;
	float3_t focus{};
	const auto *characterSelect = CLevel_CharacterSelect::Get_Active();
	const bool_t active = characterSelect && characterSelect->Try_GetClassShowcaseFocus(focus);
	if (active)
	{
		const float4_t target(focus.x, focus.y, focus.z, shadow.vAt.w);
		const vector_t offset = XMLoadFloat4(&target) - XMLoadFloat4(&shadow.vAt);
		XMStoreFloat4(&shadow.vEye, XMLoadFloat4(&shadow.vEye) + offset);
		shadow.vAt = target;
	}
	if ((!samePoint(shadow.vEye, original.vEye) || !samePoint(shadow.vAt, original.vAt)) &&
		FAILED(game.Apply_Shadow_Light(shadow)))
	{
		OutputDebugStringA("[MainApp][ClassShowcase] Shadow focus translation failed.\n");
		return;
	}
	m_bClassShowcaseShadowFocusActive = active;
	m_vShadowFocusBeforeClassShowcase = previousFocus;
	m_vClassShowcaseShadowFocus = shadow.vAt;
}

// 제품 Update가 재련 응답·선택·버튼을 처리한 뒤 gauge와 결과 연출을 갱신한다.
// 성장 퍼센트와 연출 시각은 Client 표시값이며 성공 여부는 Server 결과 경로가 정한다.
void CMainApp::Update_ItemUpgrade(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.ItemUpgrade.Update");
	if (nullptr == m_pItemUpgradeView)
		return;

	// 재련 창의 표시 플래그와 지원 Level을 함께 검사한다.
	// STATIC 슬롯은 호출을 생략해도 남으므로 로비 등 지원 범위 밖에서는 명시적으로 숨긴다.
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isSupportedLevel = currentLevel == ETOUI(LEVEL::BERN) || currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
									currentLevel == ETOUI(LEVEL::DEVELOPMENT) ||
									currentLevel == ETOUI(LEVEL::CHARACTER_SELECT) ||
									currentLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA);
	if (!m_bItemUpgradePreviewVisible || !isSupportedLevel)
	{
		Hide_ItemUpgrade();
		return;
	}

	// 이 view의 일반 flipbook은 delta 초로 갱신한다. GaugeFill만 아래 수동 퍼센트에 대응한 정확한 frame으로 고정한다.
	m_pItemUpgradeView->Update(fTimeDelta);

	Update_ItemUpgradeServerResult();
	Update_ItemUpgradeSelection();
	Update_ItemUpgradeGrowButton();
	// 기존 WAITING의 결과 공개 클릭을 새 재련 시작보다 먼저 검사한다.
	// 재련 버튼을 누른 같은 edge가 새로 연 대기 화면까지 즉시 통과하지 않게 순서를 지킨다.
	Update_ItemUpgradeResultWaitClick();
	Update_ItemUpgradeReforgeButton();
	Update_ItemUpgradeResultOkButton();

	// 성장 버튼은 Client 게이지를 0→100으로 한 번 재생한다. 별도 재련 버튼이 Server에 요청하고 응답으로 성공 여부를 받는다.
	if (m_bItemUpgradeGrowing)
	{
		constexpr f32_t GAUGE_FILL_FPS = 45.f;
		constexpr f32_t GAUGE_FILL_FRAME_COUNT = 100.f;
		const f32_t fCycleSeconds = GAUGE_FILL_FRAME_COUNT / GAUGE_FILL_FPS;
		const f64_t fElapsed = Product_Now_Seconds() - m_dItemUpgradeGrowStartSeconds;
		const int32_t iPercent =
			std::clamp(static_cast<int32_t>(fElapsed / fCycleSeconds * GAUGE_FILL_FRAME_COUNT), 0, 100);

		if (100 <= iPercent)
		{
			m_iItemUpgradePreviousPercent = 100;
			m_bItemUpgradeGrowing = false;
			// 완료 슬롯을 그릴 수 있게 하되 alpha는 0에서 시작한다. 아래 시간 기반 fade가 실제 표시량을 올린다.
			m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_WingedRingGold", true);
			m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_LevelUpMotion2Big", true);
			m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", 0.f);
			m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", 0.f);
			m_dItemUpgradeCompleteRevealStartSeconds = Product_Now_Seconds();
			m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_CompleteEffect", true);
			m_pItemUpgradeView->Restart_Animation("ItemUpgrade_CompleteEffect");
		}
		else
		{
			m_iItemUpgradePreviousPercent = iPercent;
			// flipbook 자체 시계와 성장 시작 기준 퍼센트가 어긋나지 않도록 매 갱신에 GaugeFill frame을 고정한다.
			m_pItemUpgradeView->Set_Animation_Frame("ItemUpgrade_GaugeFill", iPercent);
		}
	}
	else if (100 == m_iItemUpgradePreviousPercent)
	{
		// 100%를 유지할 때도 마지막 frame 99를 고정해 JSON의 loop 시계가 처음으로 되돌아가지 않게 한다.
		m_pItemUpgradeView->Set_Animation_Frame("ItemUpgrade_GaugeFill", 99);

		if (m_dItemUpgradeCompleteRevealStartSeconds >= 0.0)
		{
			constexpr f64_t REVEAL_FADE_SECONDS = 0.45;
			const f32_t fFadeAlpha = static_cast<f32_t>(std::clamp(
				(Product_Now_Seconds() - m_dItemUpgradeCompleteRevealStartSeconds) / REVEAL_FADE_SECONDS, 0.0, 1.0));
			m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", fFadeAlpha);
			m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", fFadeAlpha);
		}
	}

	// 성장 시작 프레임에 CoreFlash를 한 번 재생하고 28프레임/20fps 뒤에 ShockwaveRing을 예약한다.
	// 동시에 시작하지 않아 원래 두 layer의 순서를 유지한다.
	if (m_bItemUpgradeCoreFlashPending)
	{
		m_pItemUpgradeView->Restart_Animation("ItemUpgrade_CoreFlash");
		constexpr f64_t CORE_FLASH_DURATION_SECONDS = 28.0 / 20.0;
		m_dItemUpgradeShockwaveScheduledAt = Product_Now_Seconds() + CORE_FLASH_DURATION_SECONDS;
		m_bItemUpgradeCoreFlashPending = false;
	}
	if (m_dItemUpgradeShockwaveScheduledAt >= 0.0 && Product_Now_Seconds() >= m_dItemUpgradeShockwaveScheduledAt)
	{
		m_pItemUpgradeView->Restart_Animation("ItemUpgrade_ShockwaveRing");
		m_dItemUpgradeShockwaveScheduledAt = -1.0;
	}
	if (m_dItemUpgradeResultSettleAt >= 0.0 && Product_Now_Seconds() >= m_dItemUpgradeResultSettleAt)
	{
		// 결과 효과가 끝나면 아이콘·상세 글자로 전환한다. 아래에서 settle 예약을 비워 텍스트 pass도 같은 프레임부터 상세 화면을 그리게 한다.
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitEmblem", false);
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessEffect", false);
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailEffect", false);

		const bool_t bSuccess = m_bItemUpgradePendingAttemptSuccess;
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessOkBtn", bSuccess);
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailOkBtn", !bSuccess);
		// 성공·실패 모두 공통 장식 frame을 쓰고 결과 아이콘만 고른다. 상세 글자는 뒤의 텍스트 pass가 같은 결과 상태로 그린다.
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessDiamondFrame", true);
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessItemIconMarker", bSuccess);
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailItemIconMarker", !bSuccess);
		m_pItemUpgradeView->Set_SlotTexture(bSuccess ? "ItemUpgrade_SuccessItemIconMarker"
													 : "ItemUpgrade_FailItemIconMarker",
											m_strItemUpgradeAttemptIcon);
		m_dItemUpgradeResultSettleAt = -1.0;
	}

	// SmeltGlow는 채우지 않는 0% 대기에서만 표시한다.
	// 완료 이펙트는 100% 유지에서만 보이며 새 성장 클릭은 같은 프레임에 다시 숨긴다.
	const bool_t bIdle = !m_bItemUpgradeGrowing && 0 == m_iItemUpgradePreviousPercent;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SmeltGlow", bIdle);
	// 대기 중에도 GaugeFill을 frame 0에 고정해 자체 loop가 진행되는 모습을 막는다.
	if (bIdle)
		m_pItemUpgradeView->Set_Animation_Frame("ItemUpgrade_GaugeFill", 0);
}

#ifdef _DEBUG
// Debug 창 제목에 ImGui 측정 FPS를 표시한다. 최소 500ms 경과 후 갱신하되 기본 제목이 바뀌면 기다리지 않고 새로 표시한다.
void CMainApp::Update_DebugWindowTitleWithFps(const wchar_t *pBaseTitle)
{
	if (nullptr == g_hWnd || nullptr == pBaseTitle || L'\0' == pBaseTitle[0] || nullptr == ImGui::GetCurrentContext())
		return;

	constexpr size_t WINDOW_TITLE_CAPACITY = 128u;
	constexpr ULONGLONG REFRESH_INTERVAL_MS = 500ull;
	static WCHAR previousBaseTitle[WINDOW_TITLE_CAPACITY]{};
	static ULONGLONG previousRefreshTick = 0ull;

	const ULONGLONG currentTick = ::GetTickCount64();
	const bool baseTitleChanged = 0 != std::wcscmp(previousBaseTitle, pBaseTitle);
	if (!baseTitleChanged && currentTick - previousRefreshTick < REFRESH_INTERVAL_MS)
		return;

	::wcsncpy_s(previousBaseTitle, _countof(previousBaseTitle), pBaseTitle, _TRUNCATE);
	previousRefreshTick = currentTick;

	WCHAR windowTitle[WINDOW_TITLE_CAPACITY]{};
	::_snwprintf_s(windowTitle, _countof(windowTitle), _TRUNCATE, L"%ls | FPS %.1f", pBaseTitle,
				   ImGui::GetIO().Framerate);
	::SetWindowTextW(g_hWnd, windowTitle);
}
#endif

// wWinMain의 Create 요청으로 엔진 관리자, 제품 데이터·UI를 준비한 뒤 Lobby용 Loading Level을 연다.
// 엔진 관리자는 GameInstance가, 제품 창·도구·렌더링 프로필은 MainApp이 보유한다. 필수 초기화 실패는 Free로 정리된다.
HRESULT CMainApp::Initialize()
{
	// HUD 이미지의 WIC 로더를 위해 메인 스레드에 COM을 준비한다. 현재 이 호출 자체의 HRESULT는 검사하지 않는다.
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	// Client.cpp가 만든 창 handle과 현재 client pixel 크기를 ENGINE_DESC에 담는다. GameInstance가 이 값으로 장치·입력·Level별 관리 공간을 준비한다.
	ENGINE_DESC engineDesc{};
	engineDesc.hInstance = g_hInst;
	engineDesc.hWnd = g_hWnd;
	engineDesc.eWinMode = WINMODE::WIN;
	engineDesc.iNumLevels = ETOUI(LEVEL::END);
	RECT physicalClient{};
	if (!GetClientRect(g_hWnd, &physicalClient) || physicalClient.right <= 0 || physicalClient.bottom <= 0)
		return E_FAIL;
	engineDesc.iWinSizeX = physicalClient.right;
	engineDesc.iWinSizeY = physicalClient.bottom;

	// Get()으로 찾은 GameInstance의 Initialize_Engine이 장치와 Graphic_Device·Renderer·Object/Level/Prototype_Manager 등을 만들고 보유한다.
	// 반환받은 장치·context의 COM 참조는 MainApp의 UI·도구 생성에도 공유한다.
	HRESULT startupResult = CGameInstance::Get().Initialize_Engine(engineDesc, m_pDevice, m_pContext);
	if (FAILED(startupResult))
		return startupResult;

	// 엔진 연결 뒤에 사용자 표시 변경 callback을 등록한다. 게시 rendering profile 로드는 별도로 실패 여부를 검사한다.
	string displayStatus;
	if (!CClientWindowDisplay::Attach_Engine(displayStatus))
		return E_FAIL;
	CUserSettings::Get().Set_DisplayApplyCallback(CClientWindowDisplay::Apply);
	// 창 생성 전에 읽은 사용자 설정의 오디오 적용은 엔진 시작 뒤 수행한다.
	CUserSettings::Get().Initialize();

	string renderingProfileStatus;
	if (!m_RenderingProfiles.Load_Runtime(renderingProfileStatus))
	{
		OutputDebugStringA(
			("[MainApp] Rendering profile initialization failed: " + renderingProfileStatus + "\n").c_str());
#ifdef _DEBUG
		MessageBoxA(g_hWnd, renderingProfileStatus.c_str(), "Rendering Profile Load Failed", MB_OK | MB_ICONERROR);
#endif
		return E_FAIL;
	}

	// 선택적 조명 inventory는 실패를 기록하고 계속한다. 필수 profile 로드와 같은 실패 정책으로 묶지 않는다.
	std::string lightStatus;
	if (!m_LightResources.Load_Runtime(lightStatus))
		OutputDebugStringA(("[MainApp] Light resources unavailable: " + lightStatus + "\n").c_str());
	if (!m_LightResources.Refresh_MapResources("LV_LUT_MIDNIGHTC_ED", lightStatus))
		OutputDebugStringA(("[MainApp] Map light resources unavailable: " + lightStatus + "\n").c_str());
	if (!CNetworkManager::Get().Initialize())
	{
		const int networkError = CNetworkManager::Get().Get_LastErrorCode();
		const HRESULT networkResult = networkError ? HRESULT_FROM_WIN32(networkError) : E_FAIL;
		return networkResult;
	}
	startupResult = ReadyImGuiRuntime();
	if (FAILED(startupResult))
		return startupResult;

#ifdef _DEBUG
	startupResult = ReadyDebugTools();
	if (FAILED(startupResult))
		return startupResult;
#endif

	startupResult = Ready_Fonts();
	if (FAILED(startupResult))
		return startupResult;
	startupResult = Ready_Prototype_For_Static();
	if (FAILED(startupResult))
		return startupResult;

	std::string effectCatalogStatus;
	if (!CEffectCatalog::Load(effectCatalogStatus))
	{
		const std::string diagnostic = "[MainApp] Effect Catalog initialization failed: " + effectCatalogStatus + "\n";
		OutputDebugStringA(diagnostic.c_str());
#ifdef _DEBUG
		MessageBoxA(g_hWnd, effectCatalogStatus.c_str(), "Effect Catalog Load Failed", MB_OK | MB_ICONERROR);
#endif
		return E_FAIL;
	}


	// Loader worker가 사용할 스킬 정의를 먼저 읽는다. 실패는 로그로 남기고 실제 Level·복제의 자원 준비 경계에서 처리한다.
	if (CPlayerSkillCatalog::Get_Skills().empty())
	{
		Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Catalog.PlayerSkills.Initialize");
		std::string skillCatalogStatus;
		if (!CPlayerSkillCatalog::Load(skillCatalogStatus))
			OutputDebugStringA(
				("[MainApp] Player Skill Catalog initialization failed: " + skillCatalogStatus + "\n").c_str());
	}

	// 아이템 아이콘이 도구를 처음 여는 시점에 의존하지 않도록 catalog를 시작 때 읽는다. 실패하면 빈 catalog 처리를 유지한다.
	std::string itemCatalogStatus;
	if (!Client::CItemCatalog::Load(itemCatalogStatus))
	{
		const std::string diagnostic = "[MainApp] Item Catalog initialization failed: " + itemCatalogStatus + "\n";
		OutputDebugStringA(diagnostic.c_str());
	}
	// 칭호 catalog도 같은 선택적 로드 정책이다. 실패하면 기존 이름표·빈 칭호 목록 처리를 사용한다.
	std::string honorTitleStatus;
	if (!Client::CHonorTitleCatalog::Load(honorTitleStatus))
	{
		OutputDebugStringA(("[MainApp] Honor title catalog initialization failed: " + honorTitleStatus + "\n").c_str());
	}

	// 소리 cue 정의가 없어도 시작은 계속한다. 누락은 로그와 해당 표시 경로에서 처리한다.
	std::string soundCatalogStatus;
	const bool_t soundCatalogLoaded = Client::CSoundCueCatalog::Load(soundCatalogStatus);
	{
		const std::string diagnostic = (soundCatalogLoaded ? "[MainApp] Sound Cue Catalog: "
														   : "[MainApp] Sound Cue Catalog initialization failed: ") +
									   soundCatalogStatus + "\n";
		OutputDebugStringA(diagnostic.c_str());
	}
	std::string estherActionSoundStatus;
	if (!Client::CEstherActionSoundCueDocument::Load(estherActionSoundStatus))
	{
		const std::string diagnostic = "[MainApp] Esther action Sound cues isolated: " + estherActionSoundStatus + "\n";
		OutputDebugStringA(diagnostic.c_str());
	}

	//공통 UI 음원만 미리 준비하고 누락은 해당 효과음에 한정한다.
	Prepare_UISounds();

	// MainApp은 화면별 view를 보유하고 CUILayoutRuntime은 JSON 슬롯을 STATIC Layer의 sprite로 만든다.
	// Level을 바꿔도 남는 UI이므로 아래에서 최초 표시·숨김과 창 겹침 순서를 함께 정한다.
	m_pHUDRuntimeView = std::make_unique<CUILayoutRuntime>(m_pDevice, m_pContext, ETOUI(LEVEL::STATIC),
														   TEXT("Layer_UI"), L"UI/HUD/HUD_Layout.json");
	Hide_CombatHUD();
	m_pCombatAnalysisView = std::make_unique<CCombatAnalysisFrameView>(m_pDevice, m_pContext);
	Load_KoukuHudModes();
	Load_HudQuickSlotData();
	m_pBossUIView = std::make_unique<CUILayoutRuntime>(m_pDevice, m_pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
													   L"UI/BossUI/BossUI.json");
	// 저작 기준 rect를 먼저 보관하고 화면 배율·offset을 적용한다. 기준 자체를 preview 값으로 덮어쓰지 않는다.
	for (size_t i = 0u; i < m_MechanicBarBaseRects.size(); ++i)
	{
		auto &base = m_MechanicBarBaseRects[i];
		m_MechanicBarHasBase[i] = m_pBossUIView->Get_SlotRect(HUD_MECHANIC_SLOTS[i], base.x, base.y, base.z, base.w);
	}
	Apply_MechanicBarRect();

	m_pDungeonTimerView = std::make_unique<CDungeonTimerView>(m_pDevice, m_pContext, ETOUI(LEVEL::STATIC));
	// 보스가 없는 초기 프레임에 저작된 불투명 슬롯이 나타나지 않도록 생성 직후 숨긴다.
	Hide_BossHealthBar();
	m_pWorldHealthBarView = std::make_unique<CWorldHealthBarView>(m_pDevice, m_pContext);
	(void)Reload_HealthBarPositions(m_strHealthBarPositionStatus);
	m_pEstherUIView = std::make_unique<CUILayoutRuntime>(m_pDevice, m_pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
														 L"UI/Esther/EstherUI.json");
	// 실제 raid gauge의 표시 조건을 만족하기 전에는 에스더 슬롯도 숨긴다.
	Hide_EstherUI();
	m_pEstherCutinService = std::make_unique<CEstherCutinPresentationService>(m_pDevice, m_pContext);
	m_pItemUpgradeView = std::make_unique<CUILayoutRuntime>(m_pDevice, m_pContext, ETOUI(LEVEL::STATIC),
															TEXT("Layer_UI"), L"UI/ItemUpgrade/ItemUpgradeUI.json");
	// 재련 창이 처음 열리기 전까지 STATIC 슬롯을 명시적으로 숨긴다.
	Hide_ItemUpgrade();
	m_pLobbyBackgroundView = std::make_unique<CUILayoutRuntime>(m_pDevice, m_pContext, ETOUI(LEVEL::STATIC),
																TEXT("Layer_UI"), L"UI/Lobby/Lobby_Layout.json");
	// 로비가 활성화될 때까지 배경을 숨긴다. 엔진 sprite이므로 ImGui 창보다 먼저 그려진다.
	m_pLobbyBackgroundView->Set_AllSlotsVisible(false);
	Load_LobbyServers();
	// 캐릭터 선택 창은 로비 뒤에 생성해 로비 배경·버튼 위에 표시한다. 창 생성자가 초기 슬롯을 숨긴다.
	m_pCharacterSelectWindowView =
		std::make_unique<CCharacterSelectWindowView>(m_pDevice, m_pContext, ETOUI(LEVEL::STATIC));
	// 현재 스킬 창은 생성하지 않는다. 관련 갱신 분기는 view가 있을 때만 실행된다.
	m_pInventoryView = std::make_unique<CInventoryView>(m_pDevice, m_pContext);
	m_pRepairWindowView = std::make_unique<CRepairWindowView>(m_pDevice, m_pContext);
	m_pShopWindowView = std::make_unique<CShopWindowView>(m_pDevice, m_pContext);
	m_pDurabilityHudView = std::make_unique<CDurabilityHudView>(m_pDevice, m_pContext);
	m_pChatWindowView = std::make_unique<CChatWindowView>(m_pDevice, m_pContext);
	m_pPartyWindowView = std::make_unique<CPartyWindowView>(m_pDevice, m_pContext);
	m_pMinimapView = std::make_unique<CMinimapView>(m_pDevice, m_pContext, ETOUI(LEVEL::STATIC));
	// 캐릭터 정보 창은 앞서 만든 인벤토리·채팅·파티 위에 그린다. router의 창 우선순위도 해당 view가 관리한다.
	m_pCharacterInfoView = std::make_unique<CCharacterInfoWindowView>(m_pDevice, m_pContext);
	// 아바타 창은 캐릭터 정보 창에서 열리고 그 위에 표시하도록 뒤에 생성한다.
	m_pAvatarBookView = std::make_unique<CAvatarBookWindowView>(m_pDevice, m_pContext);
	// N 탈것 창은 앞서 생성한 창들 위에 표시한다.
	m_pVehicleWindowView = std::make_unique<CVehicleWindowView>(m_pDevice, m_pContext);
	// 칭호 창은 캐릭터 정보 창에서 열고 그 위에 표시한다.
	m_pHonorTitleWindowView = std::make_unique<CHonorTitleWindowView>(m_pDevice, m_pContext);
	// M 월드 지도 창은 화면 중앙의 큰 패널로 앞선 창 위에 표시한다.
	m_pWorldMapWindowView = std::make_unique<CWorldMapWindowView>(m_pDevice, m_pContext);
	m_pSongCastGaugeView = std::make_unique<CSongCastGaugeView>(m_pDevice, m_pContext);
	// Escape 환경설정 창은 앞선 runtime 창들 위에 표시한다.
	m_pSystemOptionView = std::make_unique<CSystemOptionWindowView>(m_pDevice, m_pContext);
	// 운반 중인 quick-slot 아이콘은 마지막에 생성해 모든 창 위에 표시한다.
	m_pQuickSlotDragView = std::make_unique<CQuickSlotDragView>(m_pDevice, m_pContext);

	// 시작 목적지는 Lobby로 고정하고 Start_Level에 Loading 생성과 엔진 Level 교체를 맡긴다.
	startupResult = Start_Level(LEVEL::LOBBY);
	if (FAILED(startupResult))
		return startupResult;
	return S_OK;
}
#ifdef _DEBUG
namespace
{
 void Refresh_KoukuPresentationResources(const std::vector<CKoukuSaydonActionWorkbench*>& workbenches,
  CRenderingProfileService& profiles, CLightResourceCatalog& lights)
 {
  using Kind = KOUKU_SAYDON_PRESENTATION_KIND;
  std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE> rows;
  std::string status;
  std::vector<EFFECT_V2_RESOURCE_SUMMARY> effectRows;
  if (CEffectV2Catalog::Get().Read_Inventory(effectRows, status))
   for (const auto& effect : effectRows)
   {
    KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
    row.strResourceId = row.strAssetId = effect.strResourceId;
    row.strDisplayName = effect.strDisplayName.empty() ? effect.strResourceId : effect.strDisplayName;
    row.eKind = Kind::EFFECT;
    row.strResourceKind = effect.eKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "GROUP" : "LEAF";
    row.iDurationMs = effect.iDurationMs;
    rows.push_back(std::move(row));
   }
  for (const auto& id : CEffectCatalog::Get_EffectAssetIds())
  {
   if (!CEffectCatalog::Is_DirectAuthoredDocument(id)) continue;
   KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
   row.strResourceId = "v1:" + id;
   row.strAssetId = id;
   row.strDisplayName = id;
   row.eKind = Kind::EFFECT;
   row.strResourceKind = "V1_EFFECT";
   row.iDurationMs = 3000u;
   rows.push_back(std::move(row));
  }
  // 명시적인 새로고침 때 실제 Sound 파일 목록을 한 번 수집한다. 재생 시에는
  // 기존 오디오 소유자가 각 Resources 상대 ID를 해석하므로 별도 재생 경로를 만들지 않는다.
  std::error_code ec;
  const auto root = CRuntimeAssetRoot::Get_ResourceRoot();
  const auto sounds = CRuntimeAssetRoot::Resolve(L"Sound");
  if (!sounds.empty())
   for (std::filesystem::recursive_directory_iterator it(sounds,
      std::filesystem::directory_options::skip_permission_denied, ec), end;
     !ec && it != end; it.increment(ec))
   {
    if (!it->is_regular_file(ec)) continue;
    auto extension = it->path().extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
     [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (extension != ".wav" && extension != ".ogg" && extension != ".mp3") continue;
    const auto id = it->path().lexically_relative(root).generic_string();
    if (CRuntimeAssetRoot::Resolve(id).empty()) continue;
    KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
    row.strResourceId = row.strAssetId = id; row.strDisplayName = id;
    row.eKind = Kind::SOUND; row.iDurationMs = 3000u;
    rows.push_back(std::move(row));
   }
  if (ec) status += " Sound inventory: " + ec.message();
  if (const auto* arena = CLevel_KoukuSaydonArena::Get_Active())
   for (const auto& shot : arena->Get_CameraShots())
   {
    KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
    row.strResourceId = row.strAssetId = shot.strShotId;
    row.strDisplayName = shot.strDisplayName.empty() ? shot.strShotId : shot.strDisplayName;
    row.eKind = Kind::CAMERA;
    row.iDurationMs = shot.hasCameraTrack ? shot.CameraTrack.iDurationMs : shot.iBlendInMs + shot.iDefaultHoldMs;
    rows.push_back(std::move(row));
   }
  const auto profileIds = profiles.Collect_ProfileIds();
	(void)lights.Refresh_MapResources("LV_LUT_MIDNIGHTC_ED", status);
	for(const auto& light : lights.Get_Resources())
	{
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE row;
		row.strResourceId=row.strAssetId=light.strLightResourceId;row.strDisplayName=light.strDisplayName;
		row.eKind=Kind::LIGHT;row.strResourceKind.clear();row.iDurationMs=3000u;
		row.strDefaultAnchorKind=light.strDefaultAnchorKind;rows.push_back(std::move(row));
	}
	std::vector<std::string> protectedProfiles;
	for (const auto* workbench : workbenches)
		for (const auto& scene : workbench->Get_Composition().SceneProfiles) protectedProfiles.push_back(scene.strRenderingProfileId);
	profiles.Protect_ProfileIds(protectedProfiles);
  for (auto* workbench : workbenches)
  {
   workbench->Set_RenderingProfileResources(profileIds,
    "Loaded " + std::to_string(profileIds.size()) + " admitted rendering profiles.");
   workbench->Set_PresentationResources(rows, status);
  }
 }

 bool Resolve_KoukuWorldPreviewActor(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
  const CWorldSequenceDocument& sequences, const std::string& selectedPatternId,
  KOUKU_SAYDON_COMPOSITION_PATTERN& resourcePattern, bool& requiresActor, std::string& status)
 {
  requiresActor = false;
  std::string requiredArchetype;
  for (const auto& box : resourcePattern.WorldOccurrences)
  {
   const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
    [&box](const auto& value) { return value.strWorldId == box.strWorldId; });
   const auto* instance = world == document.Worlds.end() ? nullptr : sequences.Find_Instance(world->strSequenceInstanceId);
   if (!instance) { status = "World Object Preview has no saved motion for: " + box.strWorldId; return false; }
   if (instance->anchorKind != "BOSS") continue;
   for (const auto& binding : instance->bindings)
   {
    if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
    const auto* object = sequences.Find_ObjectResource(binding.targetId);
    if (!object || object->anchorBossArchetypeId.empty())
    { status = "World Object Preview has no exact Boss anchor: " + binding.targetId; return false; }
    if (!requiredArchetype.empty() && requiredArchetype != object->anchorBossArchetypeId)
    { status = "World Object Preview contains different Boss anchors; preview their Patterns separately."; return false; }
    requiredArchetype = object->anchorBossArchetypeId;
    requiresActor = true;
   }
  }
  if (!requiresActor) return true;
  const auto selected = std::find_if(document.Patterns.begin(), document.Patterns.end(),
   [&selectedPatternId](const auto& value) { return value.strPatternId == selectedPatternId; });
  if (selected == document.Patterns.end() || !selected->strLoadError.empty() ||
   selected->strActorProfileId.empty() || selected->strGateId.empty() ||
   CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(selected->strTargetBossPlacementId) != requiredArchetype)
  { status = "World Object Preview requires its Boss Pattern to be selected: " + requiredArchetype; return false; }
  resourcePattern.strActorProfileId = selected->strActorProfileId;
  resourcePattern.strGateId = selected->strGateId;
  resourcePattern.strTargetBossPlacementId = selected->strTargetBossPlacementId;
  return true;
 }

 bool Begin_KoukuWorldPreview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
  const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, std::string& status,
  const CWorldSequenceDocument* sourceDocument = nullptr, const bool previewAtCharacter = false)
 {
  auto* arena = CLevel_KoukuSaydonArena::Get_Active();
  if (!arena) { status = "World preview requires the KoukuSaydon Arena."; return false; }
  std::vector<CLevel_KoukuSaydonArena::COMPOSITION_WORLD_PREVIEW_CUE> cues;
  CWorldGameplayDocument placements;
  bool loaded = false;
  for (const auto& box : pattern.WorldOccurrences)
  {
   const auto def = std::find_if(document.Worlds.begin(), document.Worlds.end(),
    [&box](const auto& world) { return world.strWorldId == box.strWorldId; });
   if (def == document.Worlds.end()) { status = "World box refers to an unknown resource."; return false; }
   float3_t offset(static_cast<float>(def->PositionOffset[0]),
    static_cast<float>(def->PositionOffset[1]), static_cast<float>(def->PositionOffset[2]));
   if (previewAtCharacter)
   {
    const auto& sequences = sourceDocument ? *sourceDocument : arena->Get_WorldSequenceDocument();
    const auto* instance = sequences.Find_Instance(def->strSequenceInstanceId);
    if (!instance) { status = "Resource preview has no saved default motion."; return false; }
    if (instance->anchorKind == "WORLD")
    {
     float3_t baseline, position;
     if (!arena->Try_GetWorldSequencePlacementBaseline(*instance, baseline, &sequences))
     { status = "Resource preview cannot resolve its saved placement group."; return false; }
     if (!arena->Try_Get_AuthoringForwardPlacement(position, status)) return false;
     offset = {position.x - baseline.x, position.y - baseline.y, position.z - baseline.z};
    }
   }
   else if (!box.Placement && def->strAnchorKind == "BOSS_SPAWN")
   {
    if (!loaded)
    {
     if (!placements.Load(CProjectDataRoot::Resolve(std::filesystem::path("Worlds") /
        document.strAreaId / "Gameplay.world.json"), document.strAreaId, status)) return false;
     loaded = true;
    }
    std::string placementId = document.strBossPlacementId;
    const auto gate = arena->Get_ActiveDebugGate();
    const auto& gates = arena->Get_DebugGates();
    if (gate < gates.size() && gates[gate].pAuditionPlacementId)
     placementId = gates[gate].pAuditionPlacementId;
    const auto* boss = placements.Find(placementId);
    if (!boss) { status = "World preview cannot resolve the boss spawn placement."; return false; }
    offset.x += boss->position.x - static_cast<float>(def->AnchorPosition[0]);
    offset.y += boss->position.y - static_cast<float>(def->AnchorPosition[1]);
    offset.z += boss->position.z - static_cast<float>(def->AnchorPosition[2]);
   }
   std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> placement;
   if (box.Placement)
   {
    const auto& value = *box.Placement;
    placement = CWorldSequencePlayer::OBJECT_PLACEMENT{
     {float(value.Position[0]), float(value.Position[1]), float(value.Position[2])},
     {float(value.RotationDegrees[0]), float(value.RotationDegrees[1]), float(value.RotationDegrees[2])},
     {float(value.Scale[0]), float(value.Scale[1]), float(value.Scale[2])}};
   }
   cues.push_back({box.strOccurrenceId, def->strSequenceInstanceId, box.iStartMs, box.iDurationMs,
    box.fPlaybackSpeed, offset, placement, previewAtCharacter ? CKoukuSaydonPresentationPlayer::WORLD_EMISSION_ANCHOR{} :
     CKoukuSaydonPresentationPlayer::Make_WorldEmissionAnchor(pattern, *def, box),
    previewAtCharacter ? std::string{} : std::string(CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(pattern.strTargetBossPlacementId)),
    previewAtCharacter ? std::string{} : pattern.strActorProfileId});
  }
  return arena->Debug_BeginCompositionWorldPreview(pattern.strPatternId, std::move(cues), status, sourceDocument);
 }
}
#endif

namespace
{
    void Record_KoukuRaidRequestEvent(const std::string_view eventName,
        const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request,
        const uint64_t worldGeneration, const uint32_t runEpoch, const std::string_view reason) noexcept
    {
        try
        {
            CNetworkManager::Get().Record_SessionEvent(eventName,
                "request=" + std::to_string(request.iRequestSequence) + "; operation=" + std::to_string(unsigned(request.eOperation)) +
                "; worldGeneration=" + std::to_string(worldGeneration) + "; runEpoch=" + std::to_string(runEpoch) +
                "; gate=" + request.strStartGateId + "; reason=" + std::string(reason));
        }
        catch (...) { } // 진단 기록은 입장 판정이나 요청 소유권을 변경하지 않는다.
    }
}

void CMainApp::UpdateKoukuGateCompletePlay()
{
    using namespace LostArk::Shared;
    const auto revisionMismatch = [](const char* source, const std::uint32_t expected, const std::uint32_t actual) {
        return std::string(source) + " revision mismatch: expected " + std::to_string(expected) +
            ", current " + std::to_string(actual) + ". Wait for Publish to finish, then retry Complete Play.";
    };
    auto* arena = CLevel_KoukuSaydonArena::Get_Active();
    auto& network = CNetworkManager::Get();
    const bool worldChanged = m_iKoukuCompletePlayWorldGeneration != 0u &&
        m_iKoukuCompletePlayWorldGeneration != network.Get_WorldInboundGeneration();
    if (!arena || !network.Is_Connected() || worldChanged)
    {
        if (arena) arena->Expect_KoukuRaidReply(0u);
        if (m_iKoukuCompletePlayWorldGeneration || m_iKoukuRaidPendingRequest)
            m_strKoukuCompletePlayStatus = "The connected world session changed; the previous raid request is no longer tracked.";
        if (m_iKoukuRaidPendingRequest)
        {
            Record_KoukuRaidRequestEvent("kouku.raid.request.reset", m_KoukuRaidRequest,
                m_iKoukuCompletePlayWorldGeneration, 0u, m_strKoukuCompletePlayStatus);
        }
        if (arena) arena->Debug_ResetCompletePlayPreparation();
        m_KoukuRaidResourcePreparation.reset();
        m_iKoukuRaidResourceEpoch = 0u; m_KoukuRaidResourcePatternIds.clear();
        if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_IsServerClock()) m_pKoukuPresentationPlayer->Stop_Preview();
        if (arena)
        {
            std::string restore;
            if (!arena->End_ServerRaidCinematicPresentation(true, restore))
                m_strKoukuCompletePlayStatus += " / Previous gate restore: " + restore;
            arena->Debug_SetSequenceCombatPending(false);
        }
        m_pKoukuRaidSequenceDocument.reset(); m_iKoukuRaidDocumentEpoch = m_iKoukuRaidAcknowledgedEpoch = 0u;
        m_strKoukuRaidPresentationKey.clear(); m_strKoukuRaidFailedKey.clear(); m_strKoukuCompletePlayFlowGate.clear();
        m_iKoukuRaidPendingRequest = 0u; m_bKoukuRaidStopAfterAdmission = false;
        m_KoukuRaidRequest = {}; m_KoukuRaidReplyDeadline = {}; m_strKoukuRaidReplyStatus.clear();
        m_iKoukuCompletePlayWorldGeneration = 0u;
        m_KoukuRaidPreparationDiagnostics = {};
        return;
    }
    if (m_KoukuRaidResourcePreparation)
    {
        const auto& pending = *m_KoukuRaidResourcePreparation;
        const auto fail = [&](std::string reason) {
            m_KoukuRaidResourcePreparation.reset(); arena->Debug_ResetCompletePlayPreparation();
            m_strKoukuCompletePlayStatus = "Complete raid preparation stopped; no Server start was sent. " + reason;
        };
        if (!CNetworkManager::Get().Is_Connected() || !arena->Get_PlayerCommandSink() ||
            pending.worldGeneration != CNetworkManager::Get().Get_WorldInboundGeneration() ||
            pending.request.ExpectedGameplayRevision != CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision)
        { fail("The connected world or gameplay revision changed."); return; }
        if (std::chrono::steady_clock::now() >= pending.deadline)
        { fail("Preparation exceeded 20 minutes. " + m_strKoukuCompletePlayStatus); return; }
        if ((m_pKoukuSaydonActionWorkbench && (m_pKoukuSaydonActionWorkbench->Is_Dirty() || m_pKoukuSaydonActionWorkbench->Is_PublishRunning())) ||
            (m_pSequenceActionWorkbench && (m_pSequenceActionWorkbench->Is_Dirty() || m_pSequenceActionWorkbench->Is_PublishRunning())) ||
            (m_pKoukuSaydonBossTool && m_pKoukuSaydonBossTool->Is_PlayPreparationPending()) ||
            CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight() ||
            CKoukuSaydonPatternAuditionService::Get().Get_FlowSnapshot().bActive)
        { fail("An editor mutation or another playback replaced this preparation."); return; }
        bool ready = false; std::string reason;
        if (!arena->Debug_PrepareCompletePlayResources(pending.patternIds, {}, pending.request.iActionSourceRevision,
            ready, reason, pending.request.strStartGateId != "BINGO")) { fail(reason); return; }
        m_strKoukuCompletePlayStatus = std::move(reason);
        if (!ready) return;
        CKoukuSaydonCompositionDocument actions;
        CKoukuSaydonCompositionDocument sequences(CKoukuSaydonCompositionDocument::Resolve_SequencePath());
        CKoukuSaydonBossTool product;
        if (!actions.Reload(reason)) { fail("Saved Action reload failed: " + reason); return; }
        if (!sequences.Reload(reason)) { fail("Saved Sequence reload failed: " + reason); return; }
        if (!product.Reload(reason)) { fail("Published Action reload failed: " + reason); return; }
        if (actions.Get_LastGood().iRevision != pending.request.iActionSourceRevision)
        { fail(revisionMismatch("Saved Action", pending.request.iActionSourceRevision, actions.Get_LastGood().iRevision)); return; }
        if (product.Get_SourceRevision() != pending.request.iActionSourceRevision)
        { fail(revisionMismatch("Published Action", pending.request.iActionSourceRevision, product.Get_SourceRevision())); return; }
        if (sequences.Get_LastGood().iRevision != pending.request.iSequenceSourceRevision)
        { fail(revisionMismatch("Saved Sequence", pending.request.iSequenceSourceRevision, sequences.Get_LastGood().iRevision)); return; }
        const auto request = pending.request;
        if (!arena->Get_PlayerCommandSink()->Request_KoukuRaid(request))
        { fail("The prepared raid request could not be submitted."); return; }
        m_KoukuRaidResourcePreparation.reset();
        arena->Expect_KoukuRaidReply(request.iRequestSequence);
        m_KoukuRaidRequest = request; m_iKoukuRaidPendingRequest = request.iRequestSequence;
        m_KoukuRaidReplyDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        m_iKoukuCompletePlayWorldGeneration = CNetworkManager::Get().Get_WorldInboundGeneration();
        m_strKoukuRaidReplyStatus.clear();
        m_strKoukuCompletePlayStatus = "All raid resources prepared; waiting for Server admission.";
        Record_KoukuRaidRequestEvent("kouku.raid.request.sent", request,
            m_iKoukuCompletePlayWorldGeneration, request.iExpectedRunEpoch, {});
    }
    const auto reply = arena->Get_KoukuRaidReply();
    if (m_iKoukuRaidPendingRequest && reply.iRequestSequence == m_iKoukuRaidPendingRequest &&
        (!reply.iRunEpoch || reply.iOwnerPlayerId == CNetworkManager::Get().Get_LocalPlayerId()))
    {
        Record_KoukuRaidRequestEvent("kouku.raid.request.reply", m_KoukuRaidRequest,
            m_iKoukuCompletePlayWorldGeneration, reply.iRunEpoch, reply.strReason);
        arena->Expect_KoukuRaidReply(0u);
        m_iKoukuRaidPendingRequest = 0u; m_KoukuRaidReplyDeadline = {};
        m_strKoukuRaidReplyStatus.clear();
        if (!reply.iRunEpoch)
        {
            m_bKoukuRaidStopAfterAdmission = false;
            m_strKoukuRaidReplyStatus = m_strKoukuCompletePlayStatus = reply.strReason;
            return;
        }
    }
    if (m_iKoukuRaidPendingRequest && m_KoukuRaidReplyDeadline != std::chrono::steady_clock::time_point{} &&
        std::chrono::steady_clock::now() >= m_KoukuRaidReplyDeadline)
    {
        // 기한 초과는 알림이며 Server의 최종 판정이 아니다. 같은 요청을 보존하고
        // 응답을 받거나 해당 월드 세션이 끝날 때까지 두 번째 START를 허용하지 않는다.
        m_KoukuRaidReplyDeadline = {};
        m_strKoukuRaidReplyStatus = m_strKoukuCompletePlayStatus =
            "Server raid reply timed out; still waiting for this request's final Server response.";
        Record_KoukuRaidRequestEvent("kouku.raid.request.timeout", m_KoukuRaidRequest,
            m_iKoukuCompletePlayWorldGeneration, m_KoukuRaidRequest.iExpectedRunEpoch, m_strKoukuRaidReplyStatus);
    }
    const auto& state = arena->Get_KoukuRaidState();
    if (!state.iRunEpoch) return;
    const bool active = state.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING || state.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC || state.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT ||
        state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE || state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME || state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY;
    m_iKoukuCompletePlayWorldGeneration = CNetworkManager::Get().Get_WorldInboundGeneration();
    m_strKoukuCompletePlayFlowGate = active ? state.strGateId : std::string{};
    if (m_bKoukuRaidStopAfterAdmission && !m_iKoukuRaidPendingRequest)
    {
        m_bKoukuRaidStopAfterAdmission = false;
        if (active && state.iRunEpoch == reply.iRunEpoch && state.iOwnerPlayerId == reply.iOwnerPlayerId)
        { CancelKoukuGateCompletePlay("Queued Complete Play stop."); return; }
    }
    if (state.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING)
    {
        auto& diagnostics = m_KoukuRaidPreparationDiagnostics;
        if (diagnostics.epoch != state.iRunEpoch)
        {
            diagnostics = {};
            diagnostics.epoch = state.iRunEpoch;
            diagnostics.observedAt = std::chrono::steady_clock::now();
        }
        size_t readyCount = 0u;
        for (size_t i = 0u; i < state.ParticipantPlayerIds.size(); ++i)
            if (state.iReadyMask & (1u << i)) ++readyCount;
        m_strKoukuCompletePlayStatus = "Raid preparation: " + std::to_string(readyCount) + " / " +
            std::to_string(state.ParticipantPlayerIds.size()) +
            " participants confirmed by Server. Playback starts automatically when all are ready.";
        const auto localId = CNetworkManager::Get().Get_LocalPlayerId();
        if (std::find(state.ParticipantPlayerIds.begin(), state.ParticipantPlayerIds.end(), localId) == state.ParticipantPlayerIds.end())
        {
            diagnostics.stage = "observer";
            diagnostics.detail = "Joined after START; this Client is not part of the preparation barrier.";
            return;
        }
        if (m_iKoukuRaidAcknowledgedEpoch == state.iRunEpoch) return;
        if (!arena->Get_PlayerCommandSink())
        {
            diagnostics.stage = "command.sink";
            diagnostics.detail = "Cannot report readiness: the player command sink is unavailable.";
            return;
        }
        std::string preparationError;
        CKoukuSaydonCompositionDocument actions;
        CKoukuSaydonCompositionDocument sequences(CKoukuSaydonCompositionDocument::Resolve_SequencePath());
        std::string& preparationStage = diagnostics.stage;
        preparationStage = "action.reload";
        bool ready = true;
        if (m_iKoukuRaidDocumentEpoch != state.iRunEpoch || !m_pKoukuRaidSequenceDocument)
        {
        ready = actions.Reload(preparationError);
        if (ready)
        {
            preparationStage = "sequence.reload";
            ready = sequences.Reload(preparationError);
        }
        if (ready) preparationStage = "revision.match";
        if (ready && actions.Get_LastGood().iRevision != state.iActionSourceRevision)
        { ready = false; preparationError = revisionMismatch("Saved Action", state.iActionSourceRevision, actions.Get_LastGood().iRevision); }
        if (ready && sequences.Get_LastGood().iRevision != state.iSequenceSourceRevision)
        { ready = false; preparationError = revisionMismatch("Saved Sequence", state.iSequenceSourceRevision, sequences.Get_LastGood().iRevision); }
        if (ready && sequences.Get_LastGood().strCompositionId != state.strSequenceCompositionId)
        { ready = false; preparationError = "Saved Sequence composition mismatch: expected " + state.strSequenceCompositionId + ", current " + sequences.Get_LastGood().strCompositionId; }
        if (ready && CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision != state.PinnedGameplayRevision)
        { ready = false; preparationError = "Gameplay revision mismatch: expected " + Format_GameplayDataRevision(state.PinnedGameplayRevision) +
            ", current " + Format_GameplayDataRevision(CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision); }
        if (ready)
        {
            // 여기서는 불변 문서를 미리 읽고 검증한다. 재생·생성·카메라·순간이동은 시작하지 않는다.
            for (const auto& pattern : sequences.Get_LastGood().Patterns)
            {
                preparationStage = "sequence.expand:" + pattern.strPatternId;
                KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded;
                if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(sequences.Get_LastGood(), pattern.strPatternId, expanded, preparationError))
                { ready = false; break; }
            }
        }
        if (ready)
        {
            m_pKoukuRaidSequenceDocument = std::make_unique<KOUKU_SAYDON_COMPOSITION_DOCUMENT>(sequences.Get_LastGood());
            m_iKoukuRaidDocumentEpoch = state.iRunEpoch;
        }
        else if (preparationError.empty()) preparationError = "Raid document preparation failed";
        }
        if (ready)
        {
            preparationStage = "resources.prepare";
            CKoukuSaydonBossTool published;
            if (m_iKoukuRaidResourceEpoch != state.iRunEpoch)
            {
                if (!published.Reload(preparationError))
                    ready = false;
                else if (published.Get_SourceRevision() != state.iActionSourceRevision)
                { ready = false; preparationError = revisionMismatch("Published Action", state.iActionSourceRevision, published.Get_SourceRevision()); }
                else
                {
                    m_KoukuRaidResourcePatternIds = published.Get_PlayAllPatternIds();
                    m_iKoukuRaidResourceEpoch = state.iRunEpoch;
                }
            }
            bool resourcesReady = false;
            if (ready && !arena->Debug_PrepareCompletePlayResources(m_KoukuRaidResourcePatternIds, {},
                state.iActionSourceRevision, resourcesReady, preparationError, true)) ready = false;
            if (ready && !resourcesReady)
            {
                diagnostics.detail = preparationError;
                diagnostics.submission = "READY has not been sent; local resources are still preparing.";
                return; // PREPARING에는 재생 시계가 없으므로 큐 등록만 끝난 작업을 준비 완료로 회신하지 않는다.
            }
            if (ready)
            {
                preparationStage = "resources.final_revision";
                ready = actions.Reload(preparationError) && sequences.Reload(preparationError) && published.Reload(preparationError);
                if (ready && actions.Get_LastGood().iRevision != state.iActionSourceRevision)
                { ready = false; preparationError = revisionMismatch("Saved Action", state.iActionSourceRevision, actions.Get_LastGood().iRevision); }
                if (ready && sequences.Get_LastGood().iRevision != state.iSequenceSourceRevision)
                { ready = false; preparationError = revisionMismatch("Saved Sequence", state.iSequenceSourceRevision, sequences.Get_LastGood().iRevision); }
                if (ready && published.Get_SourceRevision() != state.iActionSourceRevision)
                { ready = false; preparationError = revisionMismatch("Published Action", state.iActionSourceRevision, published.Get_SourceRevision()); }
                if (ready) preparationError.clear();
                else if (preparationError.empty()) preparationError = "Saved or published data changed during resource preparation";
            }
        }
        if (ready)
        {
            preparationStage = "gate.prepare";
            ready = arena->Prepare_ServerRaidGatePresentation(state.strGateId, preparationError);
        }
        diagnostics.failed = !ready;
        diagnostics.detail = ready ? "Local documents, resources and gate presentation are prepared." : preparationError;
        // 전송 사유가 길이 제한으로 축약되기 전에 로컬 사전 검증 결과 원문을 보관한다.
        try
        {
            CNetworkManager::Get().Record_SessionEvent("kouku.raid.prepare",
                "runEpoch=" + std::to_string(state.iRunEpoch) + "; ready=" + (ready ? "true" : "false") +
                "; stage=" + preparationStage + "; actionLocal=" + std::to_string(actions.Get_LastGood().iRevision) +
                "; actionPinned=" + std::to_string(state.iActionSourceRevision) +
                "; sequenceLocal=" + std::to_string(sequences.Get_LastGood().iRevision) +
                "; sequencePinned=" + std::to_string(state.iSequenceSourceRevision) +
                "; compositionLocal=" + sequences.Get_LastGood().strCompositionId +
                "; compositionPinned=" + state.strSequenceCompositionId +
                "; gameplayLocal=" + Format_GameplayDataRevision(CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision) +
                "; gameplayPinned=" + Format_GameplayDataRevision(state.PinnedGameplayRevision) +
                "; reason=" + preparationError);
        }
        catch (...) { } // 진단 기록은 준비 상태를 판정하거나 확인 응답을 변경하지 않는다.
        if (preparationError.size() > MAX_KOUKUSAYDON_PATTERN_AUDITION_REASON_BYTES)
        {
            auto cut = MAX_KOUKUSAYDON_PATTERN_AUDITION_REASON_BYTES;
            while (cut && (static_cast<unsigned char>(preparationError[cut]) & 0xc0u) == 0x80u) --cut;
            preparationError.resize(cut);
        }
        if (!m_iNextKoukuRaidRequest || state.iRequestSequence == UINT32_MAX)
        { diagnostics.submission = m_strKoukuCompletePlayStatus = "Raid request sequence is exhausted; readiness cannot be submitted."; return; }
        m_iNextKoukuRaidRequest = (std::max)(m_iNextKoukuRaidRequest, state.iRequestSequence + 1u);
        C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST acknowledgement;
        acknowledgement.iRequestSequence = m_iNextKoukuRaidRequest++;
        acknowledgement.eWorldId = state.eWorldId;
        acknowledgement.eOperation = ready ? KOUKUSAYDON_RAID_OPERATION::READY : KOUKUSAYDON_RAID_OPERATION::FAILED;
        acknowledgement.iExpectedRunEpoch = state.iRunEpoch;
        acknowledgement.ExpectedGameplayRevision = state.PinnedGameplayRevision;
        acknowledgement.iActionSourceRevision = state.iActionSourceRevision;
        acknowledgement.iSequenceSourceRevision = state.iSequenceSourceRevision;
        acknowledgement.strStartGateId = state.strGateId;
        if (!ready) acknowledgement.strReason = preparationError;
        const bool submitted = arena->Get_PlayerCommandSink()->Request_KoukuRaid(acknowledgement);
        const std::string submission = submitted ?
            (ready ? "READY submitted; waiting for Server confirmation." : "FAILED submitted; waiting for Server abort.") :
            "Readiness submission failed; the Server has not confirmed this Client.";
        if (submitted || diagnostics.submission != submission)
            Record_KoukuRaidRequestEvent(submitted ? "kouku.raid.prepare.submitted" : "kouku.raid.prepare.submit_failed",
                acknowledgement, m_iKoukuCompletePlayWorldGeneration, state.iRunEpoch, diagnostics.detail);
        diagnostics.submission = submission;
        if (submitted) m_iKoukuRaidAcknowledgedEpoch = state.iRunEpoch;
        if (!ready) m_strKoukuCompletePlayStatus = preparationStage + ": " + diagnostics.detail;
        return;
    }
    const bool cinematic = active && state.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC;
    if (cinematic) arena->Debug_SetSequenceCombatPending(true);
    if (!cinematic)
    {
        if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_IsServerClock())
        {
            Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Kouku.Cinematic.StopPreview");
            m_pKoukuPresentationPlayer->Stop_Preview();
            // 시네마틱이 자연 종료되면 저작된 복귀 블렌드를 유지한다.
            // 명시적인 Stop/중단은 아래 Debug_ReturnToPlayerCamera 경로를 사용한다.
            arena->Stop_CompositionCamera(!active);
        }
        // 시네마틱 사용권을 반환하기 전에 준비된 전투 소유자를 먼저 확정한다.
        // 이전 book을 먼저 복원하면 다시 교체할 재생본에 Play/Seek를 수행하게 된다.
        std::string restorationStatus;
        if (active)
        {
            std::string gateStatus;
            if (!arena->Apply_ServerRaidGatePresentation(state.strGateId, state.iRunEpoch, gateStatus))
            {
                (void)arena->End_ServerRaidCinematicPresentation(true, restorationStatus);
                m_strKoukuCompletePlayStatus = "Server gate presentation failed: " + gateStatus;
                if (!restorationStatus.empty()) m_strKoukuCompletePlayStatus += " / " + restorationStatus;
                return;
            }
            // Stop_Preview가 진입 때의 프로필을 복원하므로 이 관문의 환경을 여기서 확정해
            // 복귀 카메라와 처음 다시 보이는 HUD 프레임이 같은 장면을 사용하게 한다.
            Update_KoukuGateSceneProfile();
        }
        if (!arena->End_ServerRaidCinematicPresentation(!active, restorationStatus))
        { m_strKoukuCompletePlayStatus = "Previous gate restore failed: " + restorationStatus; return; }
        // 준비된 장면 인계가 성공한 뒤 입력과 HUD의 잠금을 함께 해제한다.
        arena->Debug_SetSequenceCombatPending(false);
        m_strKoukuRaidPresentationKey.clear(); m_strKoukuRaidFailedKey.clear();
        m_strKoukuCompletePlayStatus = state.strReason.empty() ? "Server " + state.strGateId + " | " +
            (state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY ? "waiting for Gate 3 entry approval" : state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE ? "boss defeated: waiting for the gate entry vote" : state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME ? "waiting for every maze hunter to return" :
             state.ePhase == KOUKUSAYDON_RAID_PHASE::COMPLETE ? "raid completed" : "saved flow " + state.strFlowEntryId) : state.strReason;
        if (!active) { m_pKoukuRaidSequenceDocument.reset(); m_iKoukuRaidDocumentEpoch = 0u; }
        return;
    }
    const std::string key = std::to_string(state.iRunEpoch) + ":" + state.strSequenceCompositionId + ":" + state.strSequencePatternId + ":" + std::to_string(state.iStartTick);
    if (key == m_strKoukuRaidFailedKey)
    {
        arena->Debug_SetSequenceCombatPending(false);
        std::string restore;
        if (!arena->End_ServerRaidCinematicPresentation(true, restore))
            m_strKoukuCompletePlayStatus = "Previous gate restore failed: " + restore;
        return;
    }
    if (!m_pKoukuPresentationPlayer) return;
    std::string status;
    const auto fail = [&](const std::string& reason) {
        m_strKoukuRaidFailedKey = key;
        if (m_pKoukuPresentationPlayer->Preview_IsServerClock()) m_pKoukuPresentationPlayer->Stop_Preview();
        arena->Debug_SetSequenceCombatPending(false);
        arena->Debug_ReturnToPlayerCamera();
        CNetworkManager::Get().Record_SessionEvent("kouku.cinematic.failed", key + " / " + reason);
        std::string restore;
        m_strKoukuCompletePlayStatus = "Server cinematic presentation failed: " + reason;
        if (!arena->End_ServerRaidCinematicPresentation(true, restore)) m_strKoukuCompletePlayStatus += " / " + restore;
    };
    if (m_iKoukuRaidDocumentEpoch != state.iRunEpoch || !m_pKoukuRaidSequenceDocument)
    {
        CKoukuSaydonCompositionDocument source(CKoukuSaydonCompositionDocument::Resolve_SequencePath());
        if (!source.Reload(status)) { fail(status); return; }
        if (source.Get_LastGood().iRevision != state.iSequenceSourceRevision || source.Get_LastGood().strCompositionId != state.strSequenceCompositionId)
        { fail("Saved Sequence identity/revision differs from the Server pin. Reload the published source before restarting."); return; }
        m_pKoukuRaidSequenceDocument = std::make_unique<KOUKU_SAYDON_COMPOSITION_DOCUMENT>(source.Get_LastGood());
        m_iKoukuRaidDocumentEpoch = state.iRunEpoch;
    }
    const auto snapshotTick = arena->Get_PresentationServerTick();
    const auto serverTick = static_cast<std::int32_t>(snapshotTick - state.iServerTick) >= 0 ? snapshotTick : state.iServerTick;
    const auto elapsedTicks = static_cast<std::int32_t>(serverTick - state.iStartTick);
    const std::uint32_t clockMs = elapsedTicks <= 0 ? 0u : static_cast<std::uint32_t>((std::min)(std::uint64_t(elapsedTicks) * 1000u / 30u, std::uint64_t(600000u)));
    if (m_strKoukuRaidPresentationKey != key)
    {
        KOUKU_SAYDON_COMPOSITION_DOCUMENT expanded;
        if (!CKoukuSaydonCompositionDocument::Try_ExpandPatternDocument(*m_pKoukuRaidSequenceDocument, state.strSequencePatternId, expanded, status))
        { fail(status); return; }
        const auto pattern = std::find_if(expanded.Patterns.begin(), expanded.Patterns.end(), [&](const auto& item) { return item.strPatternId == state.strSequencePatternId; });
        if (pattern == expanded.Patterns.end()) { fail("Pinned Sequence pattern is absent."); return; }
        KOUKU_SAYDON_COMPOSITION_BUNDLE wrapper;
        wrapper.strBundleId = "raid.sequence." + std::to_string(state.iRunEpoch); wrapper.strGateId = pattern->strGateId;
        wrapper.Members.push_back({wrapper.strBundleId + ".member", state.strSequencePatternId, 0u});
        const auto bundleId = wrapper.strBundleId; expanded.Bundles.push_back(std::move(wrapper));
#ifdef _DEBUG
        StopCompositionPreview(m_eCompositionPreviewOwner); ClearKoukuSequenceArrivals();
#endif
        // 후속 관문이나 늦게 합류한 관찰자는 최초 PREPARING 단계를 거치지 않을 수 있다.
        if (!arena->Prepare_ServerRaidGatePresentation(state.strGateId, status)) { fail(status); return; }
        if (!arena->Begin_ServerRaidCinematicPresentation(status)) { fail(status); return; }
        // Sequence의 장면 복귀 대상을 준비된 전투 프로필로 바로 설정한다.
        Update_KoukuGateSceneProfile();
        if (!m_pKoukuPresentationPlayer->Begin_BundlePreview(expanded, bundleId, clockMs, false, status, &arena->Get_WorldSequenceDocument(), true, false))
        { fail(status); return; }
        m_strKoukuRaidPresentationKey = key;
    }
    m_pKoukuPresentationPlayer->Sample_ServerSequence(clockMs);
    if (!m_pKoukuPresentationPlayer->Preview_Playing()) { fail(m_pKoukuPresentationPlayer->Status()); return; }
    m_strKoukuCompletePlayStatus = "Server " + state.strGateId + " | " + state.strSequencePatternId + " | " + std::to_string(clockMs) + " ms";
}

//I/P/N/M 키로 열고 닫기, Update의 UI 표시 허용 분기에서 네 창의 단축키를 처리한다.
void CMainApp::Update_RuntimeWindowShortcuts()
{
	//창 존재 문자 입력 확인 -> 전경과 키 상태 확인 -> 새로 누른 순간에 토글 -> 소리 재생
	//이전 키 상태 저장
	//I: 인벤토리 열기 닫기
	if (nullptr != m_pInventoryView && !ImGui::GetIO().WantTextInput &&
		!CUIInputRouter::Get().Is_TextInputActive())
	{
		//GetForegrondWindow 함수가 반환하는 것과 그거를 IsWindowOwnedByCurrentProcess가
		//어떤 원리로 반환을 하는 거지?
		const bool_t windowFocused =
			IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		//I 키 입력 GetAsyncKeyState <- InputManager가 없나?
		const bool_t iDown =
			windowFocused && 0 != (GetAsyncKeyState(0x49 /* I 키 */) & 0x8000);

		if (iDown && !m_bIDown)
		{
			m_pInventoryView->Toggle();

			Play_UISound(m_pInventoryView->Is_Open()
				? UI_SOUND_CUE::WINDOW_OPEN
				: UI_SOUND_CUE::WINDOW_CLOSE);
		}
		m_bIDown = iDown;
	}

	// P는 캐릭터 정보 창을 토글하며 I와 같은 텍스트 입력·전경·Loading 조건을 사용한다.
	if (nullptr != m_pCharacterInfoView && !ImGui::GetIO().WantTextInput &&
		!CUIInputRouter::Get().Is_TextInputActive())
	{
		const bool_t windowFocused = IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t keyDown = windowFocused && 0 != (GetAsyncKeyState(0x50 /* P 키 */) & 0x8000);
		if (keyDown && !m_bCharacterInfoKeyDown)
		{
			m_pCharacterInfoView->Toggle();

			Play_UISound(m_pCharacterInfoView->Is_Open()
				? UI_SOUND_CUE::WINDOW_OPEN
				: UI_SOUND_CUE::WINDOW_CLOSE);
		}
		m_bCharacterInfoKeyDown = keyDown;
	}

	// N은 탈것 창을 토글하며 기존 제품 창과 같은 입력 허용 조건을 사용한다.
	if (nullptr != m_pVehicleWindowView && !ImGui::GetIO().WantTextInput &&
		!CUIInputRouter::Get().Is_TextInputActive())
	{
		const bool_t windowFocused = IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t keyDown = windowFocused && 0 != (GetAsyncKeyState(0x4E /* N 키 */) & 0x8000);
		if (keyDown && !m_bVehicleWindowKeyDown)
		{
			m_pVehicleWindowView->Toggle();

			Play_UISound(m_pVehicleWindowView->Is_Open()
				? UI_SOUND_CUE::WINDOW_OPEN
				: UI_SOUND_CUE::WINDOW_CLOSE);
		}
		m_bVehicleWindowKeyDown = keyDown;
	}

	// M은 월드 지도 창을 토글한다. 실제 지도 영역이 없는 Level의 표시는 Update_Minimap이 별도로 제한한다.
	if (nullptr != m_pWorldMapWindowView && !ImGui::GetIO().WantTextInput &&
		!CUIInputRouter::Get().Is_TextInputActive())
	{
		const bool_t windowFocused = IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t keyDown = windowFocused && 0 != (GetAsyncKeyState(0x4D /* M 키 */) & 0x8000);
		if (keyDown && !m_bWorldMapKeyDown)
		{
			m_pWorldMapWindowView->Toggle();

			Play_UISound(m_pWorldMapWindowView->Is_Open()
				? UI_SOUND_CUE::WINDOW_OPEN
				: UI_SOUND_CUE::WINDOW_CLOSE);
		}
		m_bWorldMapKeyDown = keyDown;
	}
}

//UI가 숨겨진 동안 네 단축키의 눌림 상태만 갱신한다.
void CMainApp::Sync_RuntimeWindowShortcutKeys(bool_t bFocused)
{
    m_bIDown = bFocused && 0 != (GetAsyncKeyState(0x49 /* I 키 */) & 0x8000);
    m_bCharacterInfoKeyDown = bFocused && 0 != (GetAsyncKeyState(0x50 /* P 키 */) & 0x8000);
    m_bVehicleWindowKeyDown = bFocused && 0 != (GetAsyncKeyState(0x4E /* N 키 */) & 0x8000);
    m_bWorldMapKeyDown = bFocused && 0 != (GetAsyncKeyState(0x4D /* M 키 */) & 0x8000);
}


// 현재 아레나의 cinematic 표시 조건을 UI router에 맞춘다.
// 처음 억제되는 프레임에는 열린 창의 gesture와 운반 icon을 취소해 연출 종료 뒤 지연 클릭이 확정되지 않게 한다.
void CMainApp::Sync_CinematicUI()
{
	auto *arena = CLevel_KoukuSaydonArena::Get_Active();
	// composition 카메라는 Engine.Late_Update 뒤 샘플되므로 body·장비·shadow 제출 전에 캐릭터 표시 소유 상태를 맞춘다.
	// 이 함수는 제품 UI 입력 억제와 캐릭터의 시네마틱 표시 상태를 같은 프레임에 동기화한다.
	if (arena)
		arena->Sync_CinematicPlayerVisibility();
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	auto *valtanArena = CLevel_ValtanArena::Get_Active();
	if (valtanArena && currentLevel == ETOUI(LEVEL::VALTAN_ARENA))
		valtanArena->Sync_CinematicPlayerVisibility();
	auto *colosseum =
		currentLevel == ETOUI(LEVEL::COLOSSEUM) ? CLevel_Development::Get_Active(LEVEL::COLOSSEUM) : nullptr;
	const bool_t colosseumResult = colosseum && colosseum->Get_Replication().Get_ColosseumMatchState().iMatchId != 0u &&
								   colosseum->Get_Replication().Get_ColosseumMatchState().ePhase ==
									   LostArk::Shared::COLOSSEUM_MATCH_PHASE::FINISHED;
	const bool_t suppressed =
		(arena && currentLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA) && arena->Is_CinematicInputBlocked()) ||
		(valtanArena && currentLevel == ETOUI(LEVEL::VALTAN_ARENA) && valtanArena->Is_CinematicHUDSuppressed()) ||
		(colosseum && (colosseum->Is_ColosseumIntroActive() || colosseumResult));
	auto &router = CUIInputRouter::Get();
	if (suppressed && !router.Is_CinematicSuppressed())
	{
		// 갱신을 숨긴 사이 mouse release가 들어올 수 있으므로 시네마틱 진입 edge에서 진행 중 UI gesture를 취소한다.
		if (m_pInventoryView)
			m_pInventoryView->Cancel_Interaction();
		if (m_pRepairWindowView)
			m_pRepairWindowView->Cancel_Interaction();
		if (m_pShopWindowView)
			m_pShopWindowView->Cancel_Interaction();
		if (m_pCharacterInfoView)
			m_pCharacterInfoView->Cancel_Interaction();
		if (m_pAvatarBookView)
			m_pAvatarBookView->Cancel_Interaction();
		if (m_pVehicleWindowView)
			m_pVehicleWindowView->Cancel_Interaction();
		if (m_pHonorTitleWindowView)
			m_pHonorTitleWindowView->Cancel_Interaction();
		if (m_pWorldMapWindowView)
			m_pWorldMapWindowView->Cancel_Interaction();
		if (m_pSystemOptionView)
			m_pSystemOptionView->Cancel_Interaction();
		if (m_pChatWindowView)
			m_pChatWindowView->Cancel_Interaction();
		if (m_pPartyWindowView)
			m_pPartyWindowView->Cancel_Interaction();
		if (m_pQuickSlotDragView)
			m_pQuickSlotDragView->Cancel();
	}
	router.Set_CinematicSuppressed(suppressed);
}
// wWinMain의 한 프레임을 제품 순서로 조율한다. 전환 요청 → UI 입력 차단 → 수신 dispatch → 엔진 갱신 → 연출·도구·환경 반영 순이다.
// Object·Physics·Level의 실제 갱신은 GameInstance에 위임하고 MainApp은 이들 사이의 제품 요청을 연결한다.
void CMainApp::Update(const f32_t fTimeDelta)
{
#ifdef _DEBUG
	// 창이 숨겨져도 진행 중인 게시 작업의 종료를 확인한다.
	if (m_pVehicleDebugPanel)
	{
		m_pVehicleDebugPanel->Update();
	}
	if (m_pGuideAITool)
		m_pGuideAITool->Update();
	if (m_pMaharakaAITool)
	{
		m_pMaharakaAITool->Set_Active(static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID()) ==
										  LEVEL::MAHARAKA &&
									  CNetworkManager::Get().Is_Connected());
		m_pMaharakaAITool->Update();
	}
#endif
	// 전 프레임 요청을 UI와 렌더 큐 구성 전에 반영한다.
	// 새 Level은 첫 Render 전에 Update/Late_Update를 거쳐 표시 상태를 준비해야 한다.
	Pump_ReturnToCharacterSelect();
	Apply_LevelRequest();
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "MainApp.InputAndUI.Update");
		Update_CustomizingSceneProfile();

		// 각 view가 widget 입력을 읽기 전에 프레임별 클릭 추적을 초기화한다.
		// 프레임의 마우스 점유 확정인 End_Frame은 Update 끝이 아니라 아래 Render 끝에서 호출한다.
		CUIInputRouter::Get().Begin_Frame();
		CUITextOcclusion::Get().Begin_Frame();
		Sync_CinematicUI();
		UpdateProfilerRuntime();

		UpdateDebugToolShortcut();

		// 현재 사용자 설정으로 커서 잠금과 전경 상태를 갱신하고 FPS 표시값은 프레임 시간으로 평활한다.
		CUserSettings::Get().Update_CursorLock(IsWindowOwnedByCurrentProcess(GetForegroundWindow()));
		if (fTimeDelta > 0.f)
		{
			const f32_t fInstantFps = 1.f / fTimeDelta;
			m_fSmoothedFps =
				m_fSmoothedFps <= 0.f ? fInstantFps : m_fSmoothedFps + (fInstantFps - m_fSmoothedFps) * 0.1f;
		}

		// Loading 동안 이전 Level의 열린 창을 닫고 새 창 토글도 막아 로딩 화면의 입력 소유권을 유지한다.
		const bool_t bLoadingLevel = ETOUI(LEVEL::LOADING) == CGameInstance::Get().Get_CurrentLevelID();
		if (bLoadingLevel && !m_bWasLoadingLevel)
			Close_RuntimeWindowsForLoading();
		m_bWasLoadingLevel = bLoadingLevel;

		if (!Is_RuntimeUIScreenSuppressed())
		{
			Update_RuntimeWindowShortcuts();

			Update_SystemOptionWindow(fTimeDelta);
			Update_LobbyButtons(fTimeDelta);
			Update_CharacterSelectWindow(fTimeDelta);
			Update_Minimap(fTimeDelta);
			Update_CombatHUD(fTimeDelta);
			// 화면 좌표를 쓰는 HUD이므로 특정 월드에 한정하지 않고 저작 도구가 열린 Level에서도 갱신한다.
			if (nullptr != m_pDungeonTimerView)
			{
#ifdef _DEBUG
				CCombatHUDViewModel::Get().Debug_Tick_DungeonTimer(fTimeDelta);
#endif
				// 만료 tick은 Server가 정한다. 받은 tick 사이의 표시만 로컬 경과 시간으로 보간한다.
				{
					static std::uint32_t previousTick = 0u, previousDeadline = 0u;
					static f32_t snapshotAge = 0.f;
					CCombatHUDViewModel &HudModel = CCombatHUDViewModel::Get();
					const HUD_PLAYER_STATE &player = HudModel.Get_Player();
					const auto *arena = CLevel_KoukuSaydonArena::Get_Active();
					const auto *subject = arena ? arena->Get_CameraPlayerSnapshot() : nullptr;
					const auto deadline = subject ? subject->iKoukuMinigameEndTick : player.iKoukuMinigameEndTick;
					if (previousTick != player.iServerTick || previousDeadline != deadline)
					{
						previousTick = player.iServerTick;
						previousDeadline = deadline;
						snapshotAge = 0.f;
					}
					else
						snapshotAge += (std::max)(0.f, fTimeDelta);
					bool_t previewVisible = false;
#ifdef _DEBUG
					previewVisible = m_bDungeonTimerPreview;
#endif
					if (deadline != 0u || (!HudModel.Is_DungeonTimerRunning() && !previewVisible))
					{
						HUD_DUNGEON_TIMER_STATE state{};
						state.isVisible = player.isValid && deadline != 0u;
						const auto remainingTicks = deadline - player.iServerTick;
						state.fSeconds = state.isVisible && remainingTicks <= 0x7fffffffu
											 ? (std::max)(0.f, static_cast<float>(remainingTicks) / 30.f - snapshotAge)
											 : 0.f;
						HudModel.Set_DungeonTimer(state, false);
					}
				}
				m_pDungeonTimerView->Update(fTimeDelta, CCombatHUDViewModel::Get().Get_DungeonTimer());
			}
			Update_ItemUpgrade(fTimeDelta);
			Update_BossHealthBar();
			Update_EstherGauge(fTimeDelta);
			if (nullptr != m_pEstherCutinService)
				m_pEstherCutinService->Update(fTimeDelta);

			// UI capture·전경 이탈 중에도 실제 키 edge를 기록해 눌러 둔 키가 창을 닫을 때 새 요청이 되지 않게 한다.
			// 던지는 아이템은 sequence를 로컬 예약하고 PlayerController의 조준 확인 후 제출한다.
			{
				constexpr int VIRTUAL_KEYS[4] = {0x31, 0x32, 0x33, 0x34};
				const bool_t inputAllowed = GetForegroundWindow() == g_hWnd && !ImGui::GetIO().WantTextInput &&
											!ImGui::GetIO().WantCaptureKeyboard &&
											!CUIInputRouter::Get().Is_TextInputActive() &&
											!CUIInputRouter::Get().Is_MouseClaimedThisFrame() &&
											!CUIInputRouter::Get().Is_CinematicSuppressed() &&
											!(m_pQuickSlotDragView && m_pQuickSlotDragView->Is_Carrying());
				for (int32_t i = 0; i < 4; ++i)
				{
					const bool_t keyDown = 0 != (GetAsyncKeyState(VIRTUAL_KEYS[i]) & 0x8000);
					if (inputAllowed && keyDown && !m_bItemKeyDown[i] && !m_strItemQuickSlot[i].empty())
					{
						if (CPlayerController *pController = Find_ActivePlayerController();
							pController && pController->Request_UseItem(m_iNextUseItemSequence, m_strItemQuickSlot[i]))
						{
							if (0u == ++m_iNextUseItemSequence)
								m_iNextUseItemSequence = 1u;
						}
					}
					m_bItemKeyDown[i] = keyDown;
				}
			}

			// 5/6/7/8/9/0은 SpecialSkill_1..6에 등록한 탈것의 탑승을 요청한다. 같은 탈것의 키는 하차를 요청하며 결과는 Server 왕복을 따른다.
			if (!ImGui::GetIO().WantTextInput && !CUIInputRouter::Get().Is_TextInputActive())
			{
				constexpr int SPECIAL_VIRTUAL_KEYS[6] = {0x35, 0x36, 0x37,
														 0x38, 0x39, 0x30}; // 5~9와 0의 가상 키 코드 순서다.
				const bool_t windowFocused = IsWindowOwnedByCurrentProcess(GetForegroundWindow());
				for (int32_t i = 0; i < 6; ++i)
				{
					const bool_t keyDown = windowFocused && 0 != (GetAsyncKeyState(SPECIAL_VIRTUAL_KEYS[i]) & 0x8000);
					if (keyDown && !m_bSpecialKeyDown[i] && 0u != m_iSpecialQuickSlotVehicle[i])
					{
						const HUD_PLAYER_STATE &quickSlotPlayer = CCombatHUDViewModel::Get().Get_Player();
						const uint32_t iRequest = quickSlotPlayer.iVehicleId == m_iSpecialQuickSlotVehicle[i]
													  ? 0u
													  : m_iSpecialQuickSlotVehicle[i];
						if (CPlayerController *pController = Find_ActivePlayerController())
							(void)pController->Request_VehicleRiding(iRequest);
					}
					m_bSpecialKeyDown[i] = keyDown;
				}
			}

			// Enter는 다른 텍스트 입력이 없을 때 채팅을 연다. 열린 InputText의 capture는 아래 gameplay 키 입력도 막는다.
			// Escape는 텍스트 입력 중에도 닫기에 사용해야 하므로 WantTextInput 검사 밖에서 처리한다.
			if (nullptr != m_pChatWindowView && !ImGui::GetIO().WantTextInput &&
				!CUIInputRouter::Get().Is_TextInputActive())
			{
				// 채팅을 실제 그리는 Level에서만 열어 보이지 않는 입력창이 키를 점유하지 않게 한다.
				const uint32_t chatLevel = CGameInstance::Get().Get_CurrentLevelID();
				const bool_t chatLevelAllowed =
					ETOUI(LEVEL::BERN) == chatLevel || ETOUI(LEVEL::VALTAN_ARENA) == chatLevel ||
					ETOUI(LEVEL::KOUKUSAYDON_ARENA) == chatLevel || ETOUI(LEVEL::MAHARAKA) == chatLevel ||
					ETOUI(LEVEL::COLOSSEUM) == chatLevel;
				const bool_t windowFocused = IsWindowOwnedByCurrentProcess(GetForegroundWindow());
				const bool_t enterDown =
					chatLevelAllowed && windowFocused && 0 != (GetAsyncKeyState(VK_RETURN) & 0x8000);
				if (enterDown && !m_bEnterDown && !m_pChatWindowView->Is_Open())
					m_pChatWindowView->Open_Input();
				m_bEnterDown = enterDown;
			}
			if (nullptr != m_pChatWindowView && m_pChatWindowView->Is_Open())
			{
				const bool_t escapeDown =
					IsWindowOwnedByCurrentProcess(GetForegroundWindow()) && 0 != (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
				if (escapeDown && !m_bEscapeDown)
					m_pChatWindowView->Close_Input();
				m_bEscapeDown = escapeDown;
			}
		}
		else
		{
			// 시네마틱으로 창을 숨겨도 실제 키 edge는 갱신하되 창의 열림 상태를 바꾸지는 않는다.
			const bool_t focused = IsWindowOwnedByCurrentProcess(GetForegroundWindow());
			const auto down = [focused](int key) { return focused && 0 != (GetAsyncKeyState(key) & 0x8000); };
			Sync_RuntimeWindowShortcutKeys(focused);
			m_bSystemOptionKeyDown = m_bEscapeDown = down(VK_ESCAPE);
			m_bEnterDown = down(VK_RETURN);
			for (int i = 0; i < 4; ++i)
				m_bItemKeyDown[i] = down(0x31 + i);
			const int keys[] = {0x35, 0x36, 0x37, 0x38, 0x39, 0x30};
			for (int i = 0; i < 6; ++i)
				m_bSpecialKeyDown[i] = down(keys[i]);
		}

		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->BeginFrame();

#ifdef _DEBUG
		const bool_t mapToolOpen = m_bDeveloperToolsVisible && nullptr != m_pMapTool && m_pMapTool->IsOpen();
		const HWND foregroundWindow = GetForegroundWindow();
		const bool_t externalToolFocused = mapToolOpen && nullptr != foregroundWindow && foregroundWindow != g_hWnd &&
										   IsWindowOwnedByCurrentProcess(foregroundWindow);
		const bool_t authoredEffectPlacementConsumed =
			(nullptr != m_pEffectTool && m_pEffectTool->Update_AuthoringPlacementInput(
											 m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::EFFECT) &&
											 m_eDebugInputOwner == DEBUG_TOOL::EFFECT)) |
			(nullptr != m_pEffectToolV2 && m_pEffectToolV2->Update_AuthoringPlacementInput(
											   m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::EFFECT_V2) &&
											   m_eDebugInputOwner == DEBUG_TOOL::EFFECT_V2));
		// 각 picker를 OR의 단축 평가 전에 갱신해 다른 picker가 true여도 예약된 입력 처리를 생략하지 않는다.
		if (m_pWorldSceneTool)
			m_pWorldSceneTool->Update(fTimeDelta,
									  m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::WORLD_SCENE));
		const bool_t worldMeshPickConsumed = UpdateWorldMeshInspectionInput();
		const bool_t worldLevelPickConsumed = UpdateWorldLevelPlacementPickInput();
		const bool_t mapEffectPlacementConsumed = UpdateMapEffectPlacementInput() || authoredEffectPlacementConsumed ||
												  worldLevelPickConsumed || worldMeshPickConsumed;
		const bool_t worldLeftMouseConsumed =
			mapEffectPlacementConsumed || (nullptr != m_pMapTool && m_pMapTool->ConsumesWorldLeftMouse());
#else
		constexpr bool_t mapEffectPlacementConsumed = false;
		constexpr bool_t externalToolFocused = false;
		constexpr bool_t worldLeftMouseConsumed = false;
#endif

		// 제품 nickname 입력도 ImGui 키보드 capture와 합쳐 WASD·스킬 키가 문자 입력과 동시에 나가지 않게 한다.
		const bool_t keyboardCaptured =
			(nullptr != m_pImGuiLayer && (m_pImGuiLayer->WantsCaptureKeyboard() || externalToolFocused)) ||
			CUIInputRouter::Get().Is_TextInputActive() || CUIInputRouter::Get().Is_CinematicSuppressed();
		// 제품 창의 마우스 점유와 전 프레임 점유를 함께 읽어 PlayerController 직전에 입력 차단을 다시 풀지 않게 한다.
		// Render에서만 hover를 등록하는 창은 이 시점에 전 프레임 점유가 필요하다.
		const bool_t mouseCaptured =
			(nullptr != m_pImGuiLayer && (m_pImGuiLayer->WantsCaptureMouse() || externalToolFocused)) ||
			CUIInputRouter::Get().Is_MouseClaimedThisFrame() || CUIInputRouter::Get().Was_MouseClaimedLastFrame();
		CGameInstance::Get().SetInputBlocked(keyboardCaptured, mouseCaptured);
		CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, worldLeftMouseConsumed);
		CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, mapEffectPlacementConsumed);
	}
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Network.DrainAndDispatch");
		// 수신 큐를 제품 소비자에게 전달한 다음 Engine/Level을 갱신해 이번 프레임이 새 snapshot을 읽게 한다.
		CNetworkManager::Get().Update();
	}
	// Release raid도 소유자 수명 이벤트를 내므로 공용 큐를 매 프레임 비운다.
	CKoukuSaydonPatternAuditionService::Get().Update();
	{
		const auto &admitted = CKoukuSaydonPatternAuditionService::Get().Get_Snapshot();
		using STATE = KOUKU_SAYDON_PATTERN_AUDITION_STATE;
		if (CLevel_KoukuSaydonArena::Get_Active() && CNetworkManager::Get().Is_Connected() &&
			admitted.iWorldInboundGeneration == CNetworkManager::Get().Get_WorldInboundGeneration() &&
			admitted.DraftRowsRevision.Is_Valid() && admitted.iRoomAuditionEpoch && admitted.iPinnedSourceRevision &&
			(admitted.eState == STATE::QUEUED || admitted.eState == STATE::ACTIVE ||
			 admitted.eState == STATE::COMPLETED) &&
			!CKoukuSaydonPresentationAssetService::Matches_AdmittedRun(
				admitted.iPinnedSourceRevision, admitted.DraftRowsRevision, admitted.iRoomAuditionEpoch))
		{
			std::string status;
			if (CKoukuSaydonPresentationAssetService::Authorize_DraftProduct(admitted.DraftRowsRevision,
																			 admitted.iRoomAuditionEpoch, status))
				(void)CKoukuSaydonPresentationAssetService::Admit_RunProduct(
					ETOUI(LEVEL::KOUKUSAYDON_ARENA), admitted.iPinnedSourceRevision, admitted.DraftRowsRevision,
					admitted.iRoomAuditionEpoch, status);
		}
	}
	// PLAY_PATTERN_ID의 판정·수명 큐는 Balance·Effect·Valtan 도구가 함께 사용한다.
	// 창 표시나 tree 펼침에 관계없이 여기서 프레임당 한 번 소비한다.
	CValtanPatternAuditionService::Get().Update();
	CValtanPatternFlowService::Get().Update();
	CValtanTuningCommandService::Get().Update();
	if (m_pKoukuSaydonActionWorkbench && m_pKoukuSaydonActionWorkbench->Consume_ServerPlayCancelRequest())
	{
		m_pKoukuSaydonActionWorkbench->Cancel_DraftPlayPreparation();
		if (m_pKoukuSaydonBossTool)
			(void)m_pKoukuSaydonBossTool->Cancel_PlayPreparation(m_strKoukuCompletePlayStatus);
	}
	if (m_pKoukuSaydonBossTool)
	{
		const bool preparing = m_pKoukuSaydonBossTool->Is_PlayPreparationPending();
		m_pKoukuSaydonBossTool->Update();
		if (preparing)
			m_strKoukuCompletePlayStatus = m_pKoukuSaydonBossTool->Get_Status();
		if (m_pKoukuSaydonActionWorkbench)
			m_pKoukuSaydonActionWorkbench->Set_ServerPlayPreparationPending(
				m_pKoukuSaydonBossTool->Is_PlayPreparationPending(),
				preparing ? std::string_view(m_pKoukuSaydonBossTool->Get_Status()) : std::string_view{});
	}
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "MainApp.Engine.Update");
		// 입력·Object·Physics·Level·Late_Update는 엔진 관리자가 순서대로 수행한다. MainApp이 각 오브젝트를 직접 순회하지 않는다.
		CGameInstance::Get().Update_Engine(fTimeDelta);
	}
	Update_KoukuGateSceneProfile();
#ifdef _DEBUG
	// 공용 raid 소비 뒤에 Debug reset 요청을 처리한다.
#endif
	if (!CLevel_KoukuSaydonArena::Get_Active())
		UpdateKoukuGateCompletePlay();
	// 입장 preview 완료가 다음 관문을 예약하기 전에 Server reset을 먼저 소비한다.
	if (auto *startArena = CLevel_KoukuSaydonArena::Get_Active();
		startArena && startArena->Consume_DebugReturnToStartSucceeded())
	{
		CancelKoukuGateCompletePlay("Returned to arena start after Server reset.");
#ifdef _DEBUG
		StopCompositionPreview(DEBUG_TOOL::SEQUENCER);
#endif
	}
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "MainApp.Presentation.Prepare");
		std::vector<BOSS_STAGE_ENVIRONMENT_SAMPLE> valtanEnvironmentSamples;
		CValtan::Collect_StageEnvironmentSamples(valtanEnvironmentSamples);
		if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
		{
			if (!m_pKoukuPresentationPlayer)
				m_pKoukuPresentationPlayer =
					std::make_unique<CKoukuSaydonPresentationPlayer>(m_pDevice, m_pContext, m_RenderingProfiles);
			arena->Set_TargetedCombatPresentationPlayer(m_pKoukuPresentationPlayer.get());
			std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
			m_pKoukuPresentationPlayer->Set_LightResources(&m_LightResources);
			std::vector<KOUKU_CARD_PRESENTATION_VIEW> cards;
			arena->Collect_KoukuPresentationViews(bosses, cards);
			UpdateKoukuGateCompletePlay();
			m_pKoukuPresentationPlayer->Update(fTimeDelta, bosses, cards);
			arena->Set_CompositionWorldEmissionResolver(
				[this](std::uint32_t sourceRevision, std::string_view patternId, std::string_view occurrenceId,
					   CLevel_KoukuSaydonArena::WORLD_EMISSION_ANCHOR &anchor)
				{
					return m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Resolve_ProductWorldEmissionAnchor(
															 sourceRevision, patternId, occurrenceId, anchor);
				});
#ifdef _DEBUG
			if (m_pEffectTool)
				m_pEffectTool->Set_AuthoringPlayer(m_pKoukuPresentationPlayer.get());
			if (m_pEffectToolV2)
				m_pEffectToolV2->Set_AuthoringPlayer(m_pKoukuPresentationPlayer.get());
#endif
		}
		else if (m_pKoukuPresentationPlayer && !CLevel_ValtanArena::Get_Active() && valtanEnvironmentSamples.empty())
		{
#ifdef _DEBUG
			if (m_pEffectTool)
				m_pEffectTool->Set_AuthoringPlayer(nullptr);
			if (m_pEffectToolV2)
				m_pEffectToolV2->Set_AuthoringPlayer(nullptr);
			StopCompositionPreview(m_eCompositionPreviewOwner);
#endif
			m_pKoukuPresentationPlayer->Reset();
			m_pKoukuPresentationPlayer.reset();
		}
		if (!valtanEnvironmentSamples.empty() && !m_pKoukuPresentationPlayer)
			m_pKoukuPresentationPlayer =
				std::make_unique<CKoukuSaydonPresentationPlayer>(m_pDevice, m_pContext, m_RenderingProfiles);
		if (m_pKoukuPresentationPlayer)
		{
			m_pKoukuPresentationPlayer->Set_LightResources(&m_LightResources);
			m_pKoukuPresentationPlayer->Update_BossStageEnvironments(valtanEnvironmentSamples);
		}
#ifdef _DEBUG
		UpdateLightingPreview();
#endif
		if (m_pRenderingBenchmark)
			m_pRenderingBenchmark->Update_RestorationPreview(
				m_RenderingProfiles, m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::RENDERING) &&
										 m_bRenderingQualityWindowVisible);
		if (ETOUI(LEVEL::LOADING) != CGameInstance::Get().Get_CurrentLevelID())
		{
			CEffectPresentationService::Advance_ProductCuePreparation(m_pDevice, m_pContext);
		}
		CEffectPresentationService::Commit_PendingSpawns();
		CEffectPresentationService::Prepare_FrameCamera(fTimeDelta);
		CEffectPresentationService::Synchronize_FollowAnchors();
	}
	CEffectPresentationService::Update(fTimeDelta);
	// 제품 전투 오브젝트와 Effect 도구가 같은 free-group 시계를 사용한다.
	// Engine 객체 tick 뒤 MainApp이 한 번만 진행시켜 양쪽에서 시간이 중복 누적되지 않게 한다.
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.ProductGroups.Update");
		CEffectV2Runtime::Advance_ProductGroups(fTimeDelta, m_pDevice, m_pContext);
	}

	// 저장·게시 작업은 창이 숨겨져도 진행된다. 변경 불가능한 Save receipt를 먼저 전달해 해당 로컬 소유자가 결과를 수용하게 한 뒤 Full DataOnly 작업을 확인한다.
	if (nullptr != m_pBalanceTool)
	{
		Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
										 "ImGui.Tool.Balance.Update_ValtanSaveJob");
		m_pBalanceTool->Update_EmbeddedPanel();
		m_pBalanceTool->Update_ValtanSaveJob();
	}
	if (nullptr != m_pValtanActionWorkbench)
	{
		Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
										 "ImGui.Tool.ValtanComposition.Update_SaveState");
		m_pValtanActionWorkbench->Update_SaveState();
	}
	if (nullptr != m_pBalanceTool)
	{
		Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
										 "ImGui.Tool.Balance.Update_ServerRuntimeSetPublishJob");
		m_pBalanceTool->Update_ServerRuntimeSetPublishJob();
	}
	if (nullptr != m_pValtanBossTool)
	{
		{
			Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.ValtanBoss.Update");
			m_pValtanBossTool->Update(m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::VALTAN_BOSS),
									  m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::VALTAN_LOGIC_PATTERN));
		}
	}

	if (m_pKoukuSaydonBossTool && m_pKoukuSaydonBossTool->Consume_PublishRequest())
	{
#ifdef _DEBUG
		const bool available =
			SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::SEQUENCER, false)) && m_pKoukuSaydonActionWorkbench;
#else
		if (!m_pKoukuSaydonActionWorkbench)
			m_pKoukuSaydonActionWorkbench = make_unique<CKoukuSaydonActionWorkbench>();
		const bool available = true;
#endif
		if (!available)
			m_strKoukuCompletePlayStatus = "The Composition publisher workspace could not initialize.";
		else if (m_pKoukuSaydonActionWorkbench->Is_Dirty() || m_pKoukuSaydonActionWorkbench->Is_PublishRunning())
			m_strKoukuCompletePlayStatus =
				"Save Composition edits and wait for the current publish before publishing Pattern Flow.";
		else if (m_pKoukuSaydonActionWorkbench->Reload(m_strKoukuCompletePlayStatus))
			m_bKoukuFlowPublishPending =
				m_pKoukuSaydonActionWorkbench->Publish_AllPatterns(m_strKoukuCompletePlayStatus);
		m_pKoukuSaydonBossTool->Set_Status(m_strKoukuCompletePlayStatus);
	}
	if (m_pKoukuSaydonActionWorkbench)
	{
		m_pKoukuSaydonActionWorkbench->Tick_Background();
		if (m_bKoukuFlowPublishPending && !m_pKoukuSaydonActionWorkbench->Is_PublishRunning())
		{
			m_bKoukuFlowPublishPending = false;
			m_strKoukuCompletePlayStatus = m_pKoukuSaydonActionWorkbench->Get_Status();
			if (m_pKoukuSaydonBossTool)
				m_pKoukuSaydonBossTool->Set_Status(m_strKoukuCompletePlayStatus);
		}
		if (m_pKoukuSaydonActionWorkbench->Consume_ProductInventoryRefreshRequest() && m_pKoukuSaydonBossTool)
			(void)m_pKoukuSaydonBossTool->Reload(m_strKoukuCompletePlayStatus);
	}

#ifdef _DEBUG
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "MainApp.DebugTools.Update");
		if (nullptr != m_pMapTool)
		{
			{
				Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Map.Update");
				m_pMapTool->Update(fTimeDelta, m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::MAP) &&
												   DEBUG_TOOL::MAP == m_eDebugInputOwner);
			}
		}
		UpdateSequenceViewer();
		UpdateWorldLevelTool();
		// Composition의 일회성 점유 요청은 MainApp이 확정한다.
		// Animation_Tool 갱신 전에 소비해 다른 도구 연결에서 복귀한 preview가 한 프레임 더 멈추지 않게 한다.
		if (nullptr != m_pValtanActionWorkbench && m_pValtanActionWorkbench->Consume_PreviewOwnerClaimRequest())
		{
			if (m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) && nullptr != m_pSequencerTool &&
				m_pSequencerTool->Uses_ValtanSession())
			{
				StopCompositionPreview(m_eCompositionPreviewOwner);
				m_eDebugInputOwner = m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::SEQUENCE
										 ? DEBUG_TOOL::SEQUENCER_BENCHMARK
										 : DEBUG_TOOL::SEQUENCER;
				m_strToolStatus = "Valtan Action Workbench reclaimed viewport/preview input.";
			}
			else
			{
				m_strToolStatus = "Preview-owner claim rejected because Valtan Action Workbench is not visible.";
			}
		}
		if (nullptr != m_pAnimationTool)
		{
			const bool_t bValtanClockActive = m_pAnimationTool->Get_ValtanCompositionPreviewState().bPlaying;
			const bool_t bAnimationPreviewOwned =
				(IsDebugToolVisible(DEBUG_TOOL::ANIMATION) && DEBUG_TOOL::ANIMATION == m_eDebugInputOwner) ||
				(IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) && m_pSequencerTool &&
				 (((m_pSequencerTool->Is_BossSelected() ||
					(m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::OBJECT &&
					 m_pSequencerTool->Uses_ValtanSession())) &&
				   DEBUG_TOOL::SEQUENCER == m_eDebugInputOwner) ||
				  (m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::SEQUENCE &&
				   DEBUG_TOOL::SEQUENCER_BENCHMARK == m_eDebugInputOwner))) ||
				(IsDebugToolVisible(DEBUG_TOOL::SEQUENCER_BENCHMARK) &&
				 DEBUG_TOOL::SEQUENCER_BENCHMARK == m_eDebugInputOwner) ||
				(m_eCompositionPreviewOwner != DEBUG_TOOL::NONE && !bValtanClockActive);
			{
				Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Animation.Update");
				m_pAnimationTool->Update(fTimeDelta, m_bDeveloperToolsVisible && bAnimationPreviewOwned);
			}
		}
		// Composition Save receipt는 Developer Tools 표시나 Render 호출과 관계없이 전달한다.
		// Effect Tool이 아직 없으면 Workbench의 요청을 소비하지 않고 대기시킨다.
		if (nullptr != m_pValtanActionWorkbench && nullptr != m_pEffectTool)
		{
			std::string ExpectedValtanSourceRevision;
			if (m_pValtanActionWorkbench->Consume_EffectGraphRefreshRequest(ExpectedValtanSourceRevision))
			{
				m_pEffectTool->Request_ValtanGraphRefresh(ExpectedValtanSourceRevision);
			}
		}
		if (m_pEffectTool || m_pEffectToolV2)
		{
			shared_ptr<CCamera_Free> effectCamera;
			const LEVEL effectLevel = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
			if (effectLevel == LEVEL::CHARACTER_SELECT)
			{
				if (auto *arena = CLevel_CharacterSelect::Get_Active())
					effectCamera = arena->Get_DebugCamera();
			}
			else if (effectLevel == LEVEL::VALTAN_ARENA)
			{
				if (auto *arena = CLevel_ValtanArena::Get_Active())
					effectCamera = arena->Get_DebugCamera();
			}
			else if (effectLevel == LEVEL::KOUKUSAYDON_ARENA)
			{
				if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
					effectCamera = arena->Get_DebugCamera();
			}
			if (m_pCharacterActionWorkbench)
				m_pCharacterActionWorkbench->Set_Camera(effectCamera);
			if (m_pEffectToolV2)
			{
				m_pEffectToolV2->Set_AuthoringCamera(effectCamera);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
													 "ImGui.Tool.EffectV2.Update_AuthoringWorkspace");
					m_pEffectToolV2->Update_AuthoringWorkspace(
						fTimeDelta, m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::EFFECT_V2) &&
										m_eDebugInputOwner == DEBUG_TOOL::EFFECT_V2);
				}
			}
			if (m_pEffectTool)
			{
				std::vector<CEffect_Tool::CLASS_MOVIE_RESOURCE_ROW> movies;
				if (auto *level = CLevel_CharacterSelect::Get_Active();
					level && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
					for (const auto &option : level->Get_ClassMovieOptions())
						movies.push_back({option.classId, option.label});
				m_pEffectTool->Set_ClassMovieResources(std::move(movies));

				m_pEffectTool->Set_AuthoringCamera(effectCamera);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
													 "ImGui.Tool.EffectV1.Update_AuthoringWorkspace");
					m_pEffectTool->Update_AuthoringWorkspace(fTimeDelta, m_bDeveloperToolsVisible &&
																			 IsDebugToolVisible(DEBUG_TOOL::EFFECT) &&
																			 m_eDebugInputOwner == DEBUG_TOOL::EFFECT);
				}
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.EffectV1.Update");
					m_pEffectTool->Update(fTimeDelta);
				}
				EFFECT_RESOURCE_KEY ResourceKey;
				if (m_pEffectTool->Consume_TypedEffectResourceOpenRequest(ResourceKey))
				{
					const bool_t bOpened = ResourceKey.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT
											   ? m_pEffectTool->Open_AuthoringResource(ResourceKey)
											   : SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT_V2)) &&
													 m_pEffectToolV2->Open_Resource(ResourceKey);
					m_strToolStatus =
						bOpened ? "Opened the selected resource in its typed Effect owner."
								: "The typed Effect owner preserved its current draft; inspect the owner status.";
				}
				std::string movieClassId;
				if (m_pEffectTool->Consume_ClassMovieEditorRequest(movieClassId))
				{
					auto *level = CLevel_CharacterSelect::Get_Active();
					bool opened = false;
					if (level && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
					{
						const auto &options = level->Get_ClassMovieOptions();
						const auto option = std::find_if(options.begin(), options.end(), [&](const auto &value)
														 { return value.classId == movieClassId; });
						if (option != options.end() && SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::SEQUENCER)) &&
							level->Select_ClassMovieCategory(
								static_cast<size_t>(std::distance(options.begin(), option))))
						{
							m_pSequencerTool->Open(COMPOSITION_WORKBENCH_TARGET::WORLD);
							m_eDebugInputOwner = DEBUG_TOOL::SEQUENCER;
							opened = true;
						}
					}
					m_strToolStatus = opened
										  ? "Opened the selected class movie timeline."
										  : "Enter Character Select from Lobby before opening a class movie timeline.";
				}
			}
		}
		if (nullptr != m_pValtanBossTool)
		{
			if (m_pValtanBossTool->Consume_LogicPatternOpenRequest())
			{
				(void)EnsureDebugTool(DEBUG_TOOL::VALTAN_LOGIC_PATTERN);
			}
			CAMERA_TOOL_OPEN_REQUEST cameraRequest;
			if (m_pValtanBossTool->Consume_CameraToolOpenRequest(cameraRequest))
			{
				if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::CAMERA)) && nullptr != m_pCameraTool)
				{
					(void)m_pCameraTool->Open_Cue(cameraRequest);
				}
			}
			EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST effectRequest;
			if (m_pValtanBossTool->Consume_EffectToolOpenRequest(effectRequest))
			{
				if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT)) && nullptr != m_pEffectTool)
				{
					const bool_t bOpened = m_pEffectTool->Open_ValtanProductEffect(effectRequest);
					m_strToolStatus = bOpened ? "Opened the exact Valtan Product Effect in Effect Tool."
											  : "Effect Tool opened, but the exact Product occurrence needs attention.";
				}
			}
		}
		// Workbench의 일회성 연결 요청을 기존 Effect·Camera 소유자에게 전달한다.
		// 새 런타임 객체 경로를 만드는 대신 제품에서 쓰는 stable 요청을 소비한다.
		if (nullptr != m_pAnimationTool)
		{
			EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST effectRequest;
			if (m_pAnimationTool->Consume_EffectToolOpenRequest(effectRequest))
			{
				if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT)) && nullptr != m_pEffectTool)
				{
					const bool_t bOpened = m_pEffectTool->Open_ValtanProductEffect(effectRequest);
					m_strToolStatus = bOpened ? "Opened the Workbench Product Effect in Effect Tool."
											  : "Effect Tool opened, but the Workbench occurrence needs attention.";
				}
			}
			CAMERA_TOOL_OPEN_REQUEST cameraRequest;
			if (m_pAnimationTool->Consume_CameraToolOpenRequest(cameraRequest))
			{
				if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::CAMERA)) && nullptr != m_pCameraTool)
				{
					const bool_t bOpened = m_pCameraTool->Open_Cue(cameraRequest);
					m_strToolStatus = bOpened ? "Opened the Workbench camera sequence in Camera Tool."
											  : "Camera Tool opened, but the Workbench cue needs attention.";
				}
			}
		}
		if (nullptr != m_pValtanActionWorkbench)
		{
			EFFECT_RESOURCE_KEY resourceKey;
			if (m_pValtanActionWorkbench->Consume_EffectResourcePreviewRequest(resourceKey) &&
				SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT)) && nullptr != m_pEffectTool)
			{
				(void)m_pEffectTool->Preview_AuthoringResource(resourceKey, m_strToolStatus);
			}
			if (m_pValtanActionWorkbench->Consume_EffectResourceOpenRequest(resourceKey) &&
				SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT)) && nullptr != m_pEffectTool)
			{
				const bool opened = m_pEffectTool->Open_AuthoringResource(resourceKey);
				m_strToolStatus =
					opened ? "Opened the saved Composition resource in its Effect editor."
						   : "Effect editor preserved its draft; finish the pending edit or check its source status.";
			}
			EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST effectRequest;
			if (m_pValtanActionWorkbench->Consume_EffectToolOpenRequest(effectRequest))
			{
				if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT)) && nullptr != m_pEffectTool)
				{
					const bool_t bOpened = m_pEffectTool->Open_ValtanProductEffect(effectRequest);
					m_strToolStatus =
						bOpened ? "Opened the exact Composition Sequencer Effect in its owner Tool."
								: "Effect Tool opened, but the selected Composition occurrence needs attention.";
				}
			}
			CAMERA_TOOL_OPEN_REQUEST cameraRequest;
			if (m_pValtanActionWorkbench->Consume_CameraToolOpenRequest(cameraRequest))
			{
				if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::CAMERA)) && nullptr != m_pCameraTool)
				{
					const bool_t bOpened = m_pCameraTool->Open_Cue(cameraRequest);
					m_strToolStatus = bOpened ? "Opened the exact Composition Sequencer camera cue."
											  : "Camera Tool opened, but the selected Composition cue needs attention.";
				}
			}
			if (m_pValtanActionWorkbench->Consume_AnimationToolOpenRequest())
			{
				if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::ANIMATION)) && nullptr != m_pAnimationTool)
				{
					(void)m_pAnimationTool->Open_ValtanWorkspace();
					m_strToolStatus = "Opened Animation Clip/Sequence Intake beside Valtan Action Workbench.";
				}
			}
			m_pValtanActionWorkbench->Set_PreviewOwnerActive(
				m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) && nullptr != m_pSequencerTool &&
				m_pSequencerTool->Uses_ValtanSession() &&
				m_eDebugInputOwner == (m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::SEQUENCE
										   ? DEBUG_TOOL::SEQUENCER_BENCHMARK
										   : DEBUG_TOOL::SEQUENCER));
		}
		struct COMPOSITION_SESSION_ROUTE
		{
			DEBUG_TOOL owner;
			CKoukuSaydonActionWorkbench *workbench;
			CSequencerTool *shell;
		};
		const std::array<COMPOSITION_SESSION_ROUTE, 2> compositionRoutes{
			{{DEBUG_TOOL::SEQUENCER, m_pKoukuSaydonActionWorkbench.get(),
			  m_pSequencerTool && (m_pSequencerTool->Is_BossSelected() ||
								   (m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::OBJECT &&
									m_pSequencerTool->Uses_ValtanSession()))
				  ? m_pSequencerTool.get()
				  : nullptr},
			 {DEBUG_TOOL::SEQUENCER_BENCHMARK, m_pSequenceActionWorkbench.get(),
			  m_pSequencerTool && m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::SEQUENCE
				  ? m_pSequencerTool.get()
				  : nullptr}}};
		// 각 갱신 edge를 소비한 뒤 실제 파일 목록을 한 번 읽고 같은 값 목록을 두 도구에 나눈다.
		bool shellResourceRefresh = false, koukuResourceRefresh = false, presentationResourceRefresh = false;
		std::vector<CKoukuSaydonActionWorkbench *> resourceSessions;
		for (const auto &route : compositionRoutes)
		{
			if (route.shell)
				shellResourceRefresh |= route.shell->Consume_ResourceRefreshRequest();
			if (!route.workbench)
				continue;
			resourceSessions.push_back(route.workbench);
			route.workbench->Set_PresentationModelResolver(
				[this](const auto &pattern, auto target, auto &view)
				{
					return m_pKoukuPresentationPlayer &&
						   m_pKoukuPresentationPlayer->Resolve_PatternModelTarget(pattern, target, view);
				});
			koukuResourceRefresh |= route.workbench->Consume_ResourceRefreshRequest();
			presentationResourceRefresh |= route.workbench->Consume_PresentationResourceRefreshRequest();
		}
		if (presentationResourceRefresh)
			Refresh_KoukuPresentationResources(resourceSessions, m_RenderingProfiles, m_LightResources);
		if ((shellResourceRefresh || presentationResourceRefresh) && m_pValtanActionWorkbench)
		{
			std::vector<std::string> lights;
			for (const auto &resource : m_LightResources.Get_Resources())
				lights.push_back(resource.strLightResourceId);
			m_pValtanActionWorkbench->Set_EnvironmentResources(m_RenderingProfiles.Collect_ProfileIds(),
															   std::move(lights));
		}
		if (shellResourceRefresh || koukuResourceRefresh)
		{
			if (SUCCEEDED(EnsureAnimationPreviewBackend()))
			{
				std::vector<COMPOSITION_ANIMATION_RESOURCE> resources;
				std::string status;
				(void)m_pAnimationTool->Read_CompositionAnimationResources(resources, status);
				std::vector<CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW> sequences;
				std::vector<COMPOSITION_ANIMATION_SEQUENCE_RESOURCE> sequenceResources;
				std::string sequenceStatus;
				const bool_t sequencesLoaded =
					m_pAnimationTool->Get_ValtanCompositionSequences(sequences, sequenceStatus);
				if (sequencesLoaded)
				{
					for (const auto &sequence : sequences)
					{
						COMPOSITION_ANIMATION_SEQUENCE_RESOURCE resourceSequence;
						resourceSequence.strStableId = sequence.strStableId;
						resourceSequence.strDisplayName = sequence.strDisplayName;
						resourceSequence.strProfileId = "Valtan";
						resourceSequence.strTargetAssetName = "Valtan";
						for (const auto &clip : sequence.Clips)
						{
							const auto native = std::find_if(resources.begin(), resources.end(),
															 [&clip](const auto &candidate)
															 {
																 return candidate.strTargetAssetName == "Valtan" &&
																		candidate.strRuntimeClip == clip.strClipName;
															 });
							COMPOSITION_ANIMATION_RESOURCE resource;
							if (native != resources.end())
								resource = *native;
							const std::uint32_t nativeDurationMs = resource.iDurationMs;
							resource.strTargetAssetName = resource.strProfileId = "Valtan";
							resource.strRuntimeClip = clip.strClipName;
							if (!clip.bUsesNativeDuration && clip.iDurationMs > 0u)
								resource.iDurationMs = clip.iDurationMs;
							if (nativeDurationMs > 0u && resource.iDurationMs > nativeDurationMs)
								resource.strEndPolicy = clip.strClipName.find("_loop") != std::string::npos
															? "LOOP_TO_WINDOW"
															: "HOLD_LAST_POSE";
							resourceSequence.Clips.push_back(std::move(resource));
						}
						sequenceResources.push_back(std::move(resourceSequence));
					}
				}
				for (const auto &route : compositionRoutes)
				{
					if (route.shell)
						route.shell->Set_AnimationResources(resources, status);
					if (!route.workbench)
						continue;
					route.workbench->Set_ModelResources(resources, status);
					route.workbench->Set_SequenceResources(sequenceResources, sequenceStatus, sequencesLoaded);
				}
			}
			else
			{
				m_strToolStatus = "Animation preview backend could not initialize for Resources.";
				for (const auto &route : compositionRoutes)
					if (route.shell)
						route.shell->Set_AnimationResources({}, m_strToolStatus);
			}
		}
		// 초안 generation은 각 세션 내부 값이므로 소유자를 고른 뒤 공용 소비자용 증가 serial을 부여한다.
		// 서로 다른 초안에 같은 occurrence ID가 있어도 같은 편집으로 오인하지 않게 한다.
		for (const auto &route : compositionRoutes)
		{
			if (!m_strKoukuCompletePlayFlowGate.empty() || !route.workbench || !m_pKoukuPresentationPlayer)
				continue;
			const bool ownsPreview = route.owner == m_eCompositionPreviewOwner;
			const bool productDebugAuthoring = m_eCompositionPreviewOwner == DEBUG_TOOL::NONE &&
											   !m_pKoukuPresentationPlayer->Preview_Playing() &&
											   route.owner == DEBUG_TOOL::SEQUENCER;
			if (!ownsPreview && !productDebugAuthoring)
				continue;
			const auto generation = route.workbench->Get_DraftGeneration();
			if (m_eColliderAuthoringOwner != route.owner || m_iColliderAuthoringDraftGeneration != generation)
			{
				m_eColliderAuthoringOwner = route.owner;
				m_iColliderAuthoringDraftGeneration = generation;
				m_pKoukuPresentationPlayer->Refresh_ColliderAuthoring(route.workbench->Get_Composition(),
																	  ++m_iColliderAuthoringSerial);
			}
		}
		std::string activePreviewRouteStatus;
		DEBUG_TOOL previewStatusOwner = DEBUG_TOOL::NONE;
		if (m_strKoukuCompletePlayFlowGate.empty())
			m_bKoukuLocalPreviewStopRequested = false;
		for (const auto &route : compositionRoutes)
		{
			auto *workbench = route.workbench;
			auto *shell = route.shell;
			if (!workbench)
				continue;
			// 공용 action publisher는 Debug/Release 공통 경로에서 위에서 이미 확인한다.
			if (route.owner != DEBUG_TOOL::SEQUENCER)
				workbench->Tick_Background();
			if (!m_strKoukuCompletePlayFlowGate.empty())
			{
				// 활성 raid가 있어도 Reset/Play 요청을 잃지 않도록 기존 Server 소유자에게 정지를 요청한다.
				// raid의 종료 상태가 도착한 뒤에만 대기 중인 로컬 초안을 소비한다.
				// 대기하는 요청은 Workbench의 값 snapshot을 유지한다.
				bool resetRequested = false;
				if (workbench->Has_PendingPreviewReset())
				{
					KOUKU_PREVIEW_TRANSPORT transport;
					std::uint32_t seekMs = 0u;
					resetRequested = workbench->Consume_PreviewTransportRequest(transport, seekMs);
				}
				if (shell && !shell->Uses_ValtanSession())
				{
					CSequencerTool::ANIMATION_PREVIEW_TRANSPORT transport;
					if (shell->Consume_AnimationPreviewTransportRequest(transport))
						resetRequested |= transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::STOP;
				}
				if (resetRequested)
					m_bKoukuLocalPreviewStopRequested = false;
				if (resetRequested || workbench->Has_PendingLocalPreviewRequest())
				{
					if (!m_bKoukuLocalPreviewStopRequested)
					{
						m_bKoukuLocalPreviewStopRequested = true;
						CancelKoukuGateCompletePlay("Stopped Complete Play for local Sequence editing.");
					}
					auto state = workbench->Get_PreviewState();
					state.strStatus = m_strKoukuCompletePlayStatus;
					workbench->Set_PreviewState(state);
					if (shell)
						shell->Set_AnimationPreviewStatus(state.strStatus);
				}
				continue;
			}
			if (nullptr != shell)
			{
				COMPOSITION_ANIMATION_RESOURCE resource;
				if (shell->Consume_AnimationPreviewRequest(resource))
				{
					workbench->Cancel_CompleteSequencePlay();
					if (SUCCEEDED(EnsureAnimationPreviewBackend()) && nullptr != m_pAnimationTool)
					{
						if (m_pAnimationTool->Preview_CompositionAnimationResource(resource, m_strToolStatus))
						{
							if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
								arena->Debug_StopCompositionWorldPreview();
							if (m_pKoukuPresentationPlayer)
								m_pKoukuPresentationPlayer->Stop_Preview();
							ClaimCompositionPreviewOwner(route.owner);
							m_eDebugInputOwner = route.owner;
						}
					}
					else
						m_strToolStatus = "Animation preview backend could not initialize for the selected resource.";
					shell->Set_AnimationPreviewStatus(m_strToolStatus);
				}
			}
			// Workbench는 composition 데이터만 소유한다. 클릭은 occurrence·pattern 값 snapshot을 기존 CModel preview 소유자에게 전달한다.
			// 이 경계에서 초안 포인터나 발탄 문서의 소유권을 넘기지 않는다.
			std::string previewRouteStatus;
			const auto notifySequencePlaybackStarted = [&]()
			{
				if (route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
					if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
						arena->Notify_SequencePlaybackStarted();
			};
			const auto pausePreviousLocalPreview = [&]()
			{
				if (m_eCompositionPreviewOwner != route.owner || !m_pKoukuPresentationPlayer ||
					m_pKoukuPresentationPlayer->Preview_IsServerClock())
					return;
				// 교체가 거부되어 이전 장면을 남기는 경우에도 이전 소리 window는 정지시킨다.
				// 수정한 타임라인 뒤로 이전 preview 소리가 계속 나지 않게 한다.
				m_pKoukuPresentationPlayer->Pause_Preview(true);
				if (m_pAnimationTool)
				{
					std::string pauseStatus;
					(void)m_pAnimationTool->Set_KoukuCompositionPreviewPaused(true, pauseStatus);
				}
			};
			if (nullptr != workbench)
			{
				KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence;
				if (workbench->Consume_AnimationPreviewRequest(occurrence))
				{
					if (SUCCEEDED(EnsureAnimationPreviewBackend()) && nullptr != m_pAnimationTool)
					{
						const bool_t previewed =
							m_pAnimationTool->Preview_KoukuSaydonCompositionAnimation(occurrence, m_strToolStatus);
						if (previewed)
						{
							if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
								arena->Debug_StopCompositionWorldPreview();
							if (m_pKoukuPresentationPlayer)
								m_pKoukuPresentationPlayer->Stop_Preview();
							ClaimCompositionPreviewOwner(route.owner);
							m_eDebugInputOwner = route.owner;
						}
						else
							previewRouteStatus = m_strToolStatus;
					}
					else
					{
						m_strToolStatus = "Animation Tool could not open for KoukuSaydon resource preview.";
						previewRouteStatus = m_strToolStatus;
					}
				}

				std::string bundlePreviewId;
				std::uint32_t bundleClockMs = 0;
				bool_t bundlePaused = false;
				if (workbench->Consume_BundlePreviewRequest(bundlePreviewId, bundleClockMs, bundlePaused))
				{
					if (m_pKoukuPresentationPlayer &&
						m_pKoukuPresentationPlayer->Begin_BundlePreview(
							workbench->Get_PatternPreviewDocument(), bundlePreviewId, bundleClockMs, bundlePaused,
							previewRouteStatus, CompositionPreviewWorldSource()))
					{
						if (m_pAnimationTool)
						{
							std::string stoppedAnimationStatus;
							(void)m_pAnimationTool->Stop_KoukuCompositionPreview(stoppedAnimationStatus);
						}
						if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
							arena->Debug_StopCompositionWorldPreview();
						ClaimCompositionPreviewOwner(route.owner);
						m_eDebugInputOwner = route.owner;
						workbench->Notify_SequencePreviewAdmission(true, previewRouteStatus);
						if (!bundlePaused)
							notifySequencePlaybackStarted();
					}
					else
					{
						if (!m_pKoukuPresentationPlayer)
							previewRouteStatus = "Bundle preview requires the KoukuSaydon Arena.";
						workbench->Notify_SequencePreviewAdmission(false, previewRouteStatus);
						pausePreviousLocalPreview();
					}
				}
				std::string serverBundleId;
				std::uint32_t bundleRevision = 0;
				if (workbench->Consume_BundleServerPlayRequest(serverBundleId, bundleRevision) &&
					route.owner == DEBUG_TOOL::SEQUENCER)
				{
					if (m_pKoukuPresentationPlayer)
						m_pKoukuPresentationPlayer->Stop_Preview();
					if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
						arena->Debug_StopCompositionWorldPreview();
					ClaimCompositionPreviewOwner(DEBUG_TOOL::NONE);
					if (!m_pKoukuSaydonBossTool)
						m_pKoukuSaydonBossTool = make_unique<CKoukuSaydonBossTool>();
					m_pKoukuSaydonBossTool->Set_CompletePlayAdmission(
						[this](std::string_view gate, std::string &status)
						{ return StartKoukuGateCompletePlay(gate, status); });
					(void)m_pKoukuSaydonBossTool->Play_BundleById(serverBundleId, bundleRevision, m_strToolStatus);
					workbench->Set_ServerPlayPreparationPending(m_pKoukuSaydonBossTool->Is_PlayPreparationPending(),
																m_strToolStatus);
				}
				KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
				std::uint32_t startClockMs = 0u;
				bool_t startPaused = false;
				std::string targetAssetName;
				if (workbench->Consume_PatternPreviewRequest(pattern, startClockMs, startPaused, targetAssetName))
				{
					const bool completeSequenceRequested = workbench->Is_CompleteSequencePlaying();
					if (!completeSequenceRequested && route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK &&
						!m_strKoukuCompletePlayFlowGate.empty())
						CancelKoukuGateCompletePlay("Switched to Play Sequence; automatic combat entry was cancelled.");
					bool previewAccepted = false;
					const bool hasAnimation =
						std::any_of(pattern.Stages.begin(), pattern.Stages.end(),
									[](const auto &stage) { return !stage.AnimationOccurrences.empty(); });
					const auto &previewDocument = workbench->Get_PatternPreviewDocument();
					const bool hasActorLogic =
						std::any_of(pattern.SummonOccurrences.begin(), pattern.SummonOccurrences.end(),
									[&](const auto &summon)
									{
										return !summon.PatternSpawns.empty() ||
											   std::any_of(
												   previewDocument.Summons.begin(), previewDocument.Summons.end(),
												   [&](const auto &definition)
												   {
													   return definition.strSummonId == summon.strSummonId &&
															  definition.strSummonKind == "CROSS_DIRECTION_CLONES";
												   });
									}) ||
						std::any_of(pattern.LogicOccurrences.begin(), pattern.LogicOccurrences.end(),
									[&](const auto &box)
									{
										return box.bEnabled &&
											   std::any_of(
												   previewDocument.Logics.begin(), previewDocument.Logics.end(),
												   [&](const auto &logic)
												   {
													   return logic.strLogicId == box.strLogicId &&
															  (logic.strJudgementKind == "CROSS_DIRECTION_CLONES" ||
															   logic.strJudgementKind == "BOSS_TRACK_TARGET" ||
															   logic.strJudgementKind == "SHOWTIME_PLAYER_TARGETS" ||
															   logic.strTriggerKind == "BOSS_TELEPORT_XZ" ||
															   logic.strTriggerKind == "BOSS_TELEPORT_GROUNDED" ||
															   logic.strTriggerKind == "ALBION_AIRBORNE" ||
															   logic.strTriggerKind == "ALBION_BLUE_CIRCLE");
												   });
									});
					if ((hasAnimation || hasActorLogic) &&
						(!pattern.strActorProfileId.empty() || pattern.BossMotion ||
						 pattern.fAnimationRootVerticalScale != 1.0) &&
						targetAssetName.empty())
					{
						auto document = workbench->Get_PatternPreviewDocument();
						for (auto &previewPattern : document.Patterns)
							if (previewPattern.strPatternId == pattern.strPatternId)
							{
								previewPattern = pattern;
								break;
							}
						KOUKU_SAYDON_COMPOSITION_BUNDLE single;
						single.strBundleId = pattern.strPatternId;
						single.strGateId = pattern.strGateId;
						single.Members.push_back({pattern.strPatternId + ".preview.member", pattern.strPatternId, 0u});
						document.Bundles.push_back(std::move(single));
						if (m_pKoukuPresentationPlayer &&
							m_pKoukuPresentationPlayer->Begin_BundlePreview(
								document, pattern.strPatternId, startClockMs, startPaused, previewRouteStatus,
								CompositionPreviewWorldSource(), true, true))
						{
							if (m_pAnimationTool)
							{
								std::string stoppedAnimationStatus;
								(void)m_pAnimationTool->Stop_KoukuCompositionPreview(stoppedAnimationStatus);
							}

							ClaimCompositionPreviewOwner(route.owner);
							previewAccepted = Begin_KoukuWorldPreview(document, pattern, previewRouteStatus,
																	  CompositionPreviewWorldSource());
							if (!previewAccepted)
								StopCompositionPreview(route.owner);
							m_eDebugInputOwner = route.owner;
						}
						else if (!m_pKoukuPresentationPlayer)
							previewRouteStatus = "Pattern actor preview requires the KoukuSaydon Arena.";
					}
					else
					{
						bool previewed = !hasAnimation;
						if (hasAnimation && SUCCEEDED(EnsureAnimationPreviewBackend()) && m_pAnimationTool)
							previewed = targetAssetName.empty()
											? m_pAnimationTool->Preview_KoukuSaydonCompositionPattern(
												  pattern, m_strToolStatus, startClockMs, startPaused)
											: m_pAnimationTool->Preview_CompositionResourcePattern(
												  pattern, targetAssetName, m_strToolStatus, startClockMs, startPaused);
						if (previewed && m_pKoukuPresentationPlayer)
						{
							const auto &document = workbench->Get_PatternPreviewDocument();
							previewed = m_pKoukuPresentationPlayer->Begin_Preview(
								document, pattern, !hasAnimation, startClockMs, startPaused, previewRouteStatus);
							if (previewed)
							{
								if (!hasAnimation && m_pAnimationTool)
								{
									std::string stoppedAnimationStatus;
									(void)m_pAnimationTool->Stop_KoukuCompositionPreview(stoppedAnimationStatus);
								}
								ClaimCompositionPreviewOwner(route.owner);
								m_eDebugInputOwner = route.owner;
								previewAccepted = Begin_KoukuWorldPreview(document, pattern, previewRouteStatus,
																		  CompositionPreviewWorldSource());
								if (!previewAccepted)
								{
									workbench->Notify_SequencePreviewAdmission(false, previewRouteStatus);
									StopCompositionPreview(route.owner);
								}
							}
						}
						else if (previewed)
							previewRouteStatus = "Presentation preview requires the KoukuSaydon Arena.";
						else
							previewRouteStatus = m_strToolStatus;
					}
					if (previewAccepted && route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
					{
						previewAccepted = BeginKoukuSequenceArrivals(workbench->Get_PatternPreviewDocument(), pattern,
																	 startClockMs, previewRouteStatus);
						if (!previewAccepted)
							StopCompositionPreview(route.owner);
					}
					workbench->Notify_SequencePreviewAdmission(previewAccepted, previewRouteStatus);
					if (previewAccepted && !startPaused)
						notifySequencePlaybackStarted();
					if (!previewAccepted && !completeSequenceRequested)
						pausePreviousLocalPreview();
					if (completeSequenceRequested && !previewAccepted)
					{
						StopCompositionPreview(route.owner);
						if (route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
							CancelKoukuGateCompletePlay(previewRouteStatus);
					}
				}

				KOUKU_SUMMON_PLACEMENT_PREVIEW_REQUEST summonPlacementPreview;
				if (workbench->Consume_SummonPlacementPreviewRequest(summonPlacementPreview) &&
					m_pKoukuPresentationPlayer && m_eCompositionPreviewOwner == route.owner)
					if (!m_pKoukuPresentationPlayer->Preview_SummonPlacement(summonPlacementPreview.strPatternId,
																			 summonPlacementPreview.Occurrence))
						workbench->Notify_WorldObjectEditResult(
							"Summon placement preview is unavailable. Use World Preview to rebuild this box.");

				KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST presentationGeometryPreview;
				while (workbench->Consume_PresentationGeometryPreviewRequest(presentationGeometryPreview))
					if (m_pKoukuPresentationPlayer && m_eCompositionPreviewOwner == route.owner)
						(void)m_pKoukuPresentationPlayer->Preview_PresentationGeometry(
							presentationGeometryPreview.strPatternId, presentationGeometryPreview.Occurrence);

				KOUKU_PRESENTATION_PREVIEW_REQUEST resourcePreview;
				if (workbench->Consume_PresentationPreviewRequest(resourcePreview))
				{
					if (m_pWorldObjectTool)
						m_pWorldObjectTool->Deactivate();
					const bool keepCurrentPreview =
						m_eCompositionPreviewOwner == route.owner && m_pKoukuPresentationPlayer &&
						m_pKoukuPresentationPlayer->Preview_HasActiveWorldBox(resourcePreview.strEditedOccurrenceId);
					if (!keepCurrentPreview)
					{
						if (m_pKoukuPresentationPlayer)
						{
							auto document = workbench->Get_Composition();
							KOUKU_SAYDON_COMPOSITION_PATTERN resourcePattern;
							resourcePattern.strPatternId = "preview.kouku.resource";
							// 리소스 시청용 임시 pattern은 현재 선택한 보스 문맥을 사용한다.
							// 이 임시 배치는 Timeline occurrence의 저작 anchor를 덮어쓰지 않는다.
							if (resourcePreview.Resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
								(resourcePreview.Resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::LIGHT &&
								 resourcePreview.Occurrence.strAnchorKind == "BOSS"))
							{
								const auto selected = std::find_if(
									document.Patterns.begin(), document.Patterns.end(), [&](const auto &item)
									{ return item.strPatternId == workbench->Get_SelectedPatternId(); });
								if (selected != document.Patterns.end() && selected->strLoadError.empty())
								{
									resourcePattern.strActorProfileId = selected->strActorProfileId;
									resourcePattern.strGateId = selected->strGateId;
									resourcePattern.strTargetBossPlacementId = selected->strTargetBossPlacementId;
								}
							}
							KOUKU_SAYDON_COMPOSITION_STAGE stage;
							stage.strStageId = "preview.resource.stage";
							stage.iDurationMs = resourcePreview.Occurrence.iDurationMs;
							resourcePattern.Stages.push_back(stage);
							if (resourcePreview.Resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::WORLD)
							{
								if (!resourcePreview.WorldBoxes.empty())
								{
									resourcePattern.WorldOccurrences = resourcePreview.WorldBoxes;
									resourcePattern.Stages.front().iDurationMs = 600000u;
								}
								else
								{
									if (resourcePreview.Resource.strResourceId.empty())
									{
										KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION world;
										world.strWorldId = "preview.resource.world-definition";
										world.strDisplayName = resourcePreview.Resource.strDisplayName;
										world.strSequenceInstanceId = resourcePreview.Resource.strAssetId;
										world.PositionOffset = resourcePreview.Occurrence.PositionOffset;
										document.Worlds.push_back(world);
										resourcePreview.Resource.strResourceId = world.strWorldId;
									}
									KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE box;
									box.strWorldId = resourcePreview.Resource.strResourceId;
									box.strOccurrenceId = "preview.resource.world";
									box.iDurationMs = resourcePreview.Occurrence.iDurationMs;
									resourcePattern.WorldOccurrences.push_back(box);
									const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
																	[&box](const auto &value)
																	{ return value.strWorldId == box.strWorldId; });
									if (world != document.Worlds.end() && !world->strCompanionEffectResourceId.empty())
									{
										KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE effect;
										effect.strOccurrenceId = "preview.resource.world.effect";
										effect.strResourceId = world->strCompanionEffectResourceId;
										effect.strWorldOccurrenceId = box.strOccurrenceId;
										effect.iDurationMs = box.iDurationMs;
										resourcePattern.PresentationOccurrences.push_back(std::move(effect));
									}
								}
							}
							else
							{
								if (resourcePreview.Resource.strResourceId.empty())
									resourcePreview.Resource.strResourceId = "preview.resource";
								resourcePreview.Occurrence.strResourceId = resourcePreview.Resource.strResourceId;
								if (resourcePreview.Occurrence.strOccurrenceId.empty())
									resourcePreview.Occurrence.strOccurrenceId = "preview.resource.occurrence";
								resourcePreview.Occurrence.iStartMs = 0u;
								const auto existing = std::find_if(
									document.PresentationResources.begin(), document.PresentationResources.end(),
									[&resourcePreview](const auto &resource)
									{ return resource.strResourceId == resourcePreview.Resource.strResourceId; });
								if (existing == document.PresentationResources.end())
									document.PresentationResources.push_back(resourcePreview.Resource);
								else
									*existing = resourcePreview.Resource;
								resourcePattern.PresentationOccurrences.push_back(resourcePreview.Occurrence);
								if (resourcePreview.Occurrence.strAnchorKind == "WORLD")
								{
									KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE worldBox;
									worldBox.strWorldId = resourcePreview.Occurrence.strWorldId;
									worldBox.strOccurrenceId = "preview.resource.anchor";
									worldBox.iDurationMs = resourcePreview.Occurrence.iDurationMs;
									resourcePattern.WorldOccurrences.push_back(worldBox);
								}
							}
							const auto *worldSource = CompositionPreviewWorldSource();
							bool worldRequiresActor = false;
							bool contextReady = true;
							if (resourcePreview.Resource.eKind == KOUKU_SAYDON_PRESENTATION_KIND::WORLD)
							{
								const auto *arena = CLevel_KoukuSaydonArena::Get_Active();
								if (!arena)
								{
									previewRouteStatus = "World Object Preview requires the KoukuSaydon Arena.";
									contextReady = false;
								}
								else
									contextReady = Resolve_KoukuWorldPreviewActor(
										document, worldSource ? *worldSource : arena->Get_WorldSequenceDocument(),
										resourcePreview.strSourcePatternId.empty() ? workbench->Get_SelectedPatternId()
																				   : resourcePreview.strSourcePatternId,
										resourcePattern, worldRequiresActor, previewRouteStatus);
							}
							bool resourceStarted = false;
							if (contextReady && worldRequiresActor)
							{
								// 손 소품은 Pattern Play와 같은 actor·pose 소유자를 사용한다.
								// WORLD 표시·배치·조명 복원 책임은 계속 Level에 남는다.
								document.Patterns.push_back(resourcePattern);
								KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
								bundle.strBundleId = resourcePattern.strPatternId;
								bundle.strGateId = resourcePattern.strGateId;
								bundle.Members.push_back(
									{resourcePattern.strPatternId + ".member", resourcePattern.strPatternId, 0u});
								document.Bundles.push_back(std::move(bundle));
								resourceStarted = m_pKoukuPresentationPlayer->Begin_BundlePreview(
									document, resourcePattern.strPatternId, 0u, !resourcePreview.WorldBoxes.empty(),
									previewRouteStatus, worldSource, true, true);
							}
							else if (contextReady)
								resourceStarted = m_pKoukuPresentationPlayer->Begin_Preview(
									document, resourcePattern, true, 0u, !resourcePreview.WorldBoxes.empty(),
									previewRouteStatus);
							if (resourceStarted)
							{
								if (m_pAnimationTool)
								{
									std::string stoppedAnimationStatus;
									(void)m_pAnimationTool->Stop_KoukuCompositionPreview(stoppedAnimationStatus);
								}
								ClaimCompositionPreviewOwner(route.owner);
								m_eDebugInputOwner = route.owner;
								if ((worldRequiresActor || !m_pKoukuPresentationPlayer->Preview_IsBundle()) &&
									!Begin_KoukuWorldPreview(document, resourcePattern, previewRouteStatus, worldSource,
															 !worldRequiresActor &&
																 resourcePreview.Resource.eKind ==
																	 KOUKU_SAYDON_PRESENTATION_KIND::WORLD &&
																 resourcePreview.WorldBoxes.empty()))
									StopCompositionPreview(route.owner);
							}
						}
						else
							previewRouteStatus = "Presentation preview requires the KoukuSaydon Arena.";
					}
				}

				// 같은 프레임의 preview 시작 뒤 transport를 적용하고 결과 identity·시계를 함께 Workbench로 돌려준다.
				KOUKU_PREVIEW_TRANSPORT transport = KOUKU_PREVIEW_TRANSPORT::NONE;
				std::uint32_t seekMs = 0u;
				if (workbench->Consume_PreviewTransportRequest(transport, seekMs) &&
					m_eCompositionPreviewOwner == route.owner)
				{
					const bool ownClock = m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_OwnsClock();
					bool previewResumed = false;
					if (ownClock)
					{
						if (transport == KOUKU_PREVIEW_TRANSPORT::PAUSE)
						{
							// 이 프레임 Update에서 시계가 진행됐으므로 화면에 표시했던 시각으로 되돌린 뒤 일시 정지한다.
							m_pKoukuPresentationPlayer->Seek_Preview(seekMs);
							m_pKoukuPresentationPlayer->Pause_Preview(true);
						}
						if (transport == KOUKU_PREVIEW_TRANSPORT::RESUME)
						{
							m_pKoukuPresentationPlayer->Pause_Preview(false);
							previewResumed = m_pKoukuPresentationPlayer->Preview_Playing();
						}
						if (transport == KOUKU_PREVIEW_TRANSPORT::SEEK)
							m_pKoukuPresentationPlayer->Seek_Preview(seekMs);
					}
					else if (m_pAnimationTool)
					{
						if (transport == KOUKU_PREVIEW_TRANSPORT::PAUSE &&
							m_pAnimationTool->Seek_KoukuCompositionPreview(seekMs, m_strToolStatus))
						{
							(void)m_pAnimationTool->Set_KoukuCompositionPreviewPaused(true, m_strToolStatus);
							if (m_pKoukuPresentationPlayer)
								m_pKoukuPresentationPlayer->Seek_Preview(seekMs);
						}
						if (transport == KOUKU_PREVIEW_TRANSPORT::RESUME)
							previewResumed =
								m_pAnimationTool->Set_KoukuCompositionPreviewPaused(false, m_strToolStatus);
						if (transport == KOUKU_PREVIEW_TRANSPORT::SEEK &&
							m_pAnimationTool->Seek_KoukuCompositionPreview(seekMs, m_strToolStatus))
						{
							if (m_pKoukuPresentationPlayer)
								m_pKoukuPresentationPlayer->Seek_Preview(seekMs);
						}
					}
					if (previewResumed)
						notifySequencePlaybackStarted();
					if (transport == KOUKU_PREVIEW_TRANSPORT::STOP)
					{
						StopCompositionPreview(route.owner);
					}
				}

				KOUKU_DRAFT_PLAY_REQUEST draftPlay;
				if (workbench->Consume_DraftServerPlayRequest(draftPlay) && route.owner == DEBUG_TOOL::SEQUENCER)
				{
					if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
						arena->Debug_StopCompositionWorldPreview();
					if (m_pKoukuPresentationPlayer)
						m_pKoukuPresentationPlayer->Stop_Preview();
					ClaimCompositionPreviewOwner(DEBUG_TOOL::NONE);
					if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::KOUKU_SAYDON_BOSS)) && m_pKoukuSaydonBossTool)
					{
						(void)m_pKoukuSaydonBossTool->Play_Draft(std::move(draftPlay), m_strToolStatus);
						workbench->Set_ServerPlayPreparationPending(m_pKoukuSaydonBossTool->Is_PlayPreparationPending(),
																	m_strToolStatus);
					}
					else
						workbench->Set_ServerPlayPreparationPending(
							false, "KoukuSaydon Boss Tool could not prepare draft playback.");
				}
				std::string serverPatternId;
				std::uint32_t sourceRevision = 0u;
				// 제품 presentation 교체는 Server가 승인한 실행 epoch를 따라야 하므로 기존 Server Play 준비 경로로 보낸다.
				if (workbench->Consume_ServerPlayRequest(serverPatternId, sourceRevision) &&
					route.owner == DEBUG_TOOL::SEQUENCER)
				{
					if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
						arena->Debug_StopCompositionWorldPreview();
					if (m_pKoukuPresentationPlayer)
						m_pKoukuPresentationPlayer->Stop_Preview();
					ClaimCompositionPreviewOwner(DEBUG_TOOL::NONE);
					if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::KOUKU_SAYDON_BOSS)) && nullptr != m_pKoukuSaydonBossTool)
					{
						SetDebugToolVisible(DEBUG_TOOL::KOUKU_SAYDON_BOSS, true);
						(void)m_pKoukuSaydonBossTool->Play_PatternById(serverPatternId, sourceRevision,
																	   m_strToolStatus);
						workbench->Set_ServerPlayPreparationPending(m_pKoukuSaydonBossTool->Is_PlayPreparationPending(),
																	m_strToolStatus);
						m_eDebugInputOwner = DEBUG_TOOL::KOUKU_SAYDON_BOSS;
						m_eDebugWindowFocusPending = DEBUG_TOOL::KOUKU_SAYDON_BOSS;
					}
					else
					{
						m_strToolStatus = "KoukuSaydon Boss Tool could not open for Server Play.";
						workbench->Set_ServerPlayPreparationPending(false, m_strToolStatus);
					}
				}
			}
			// 예약된 시작 요청을 모두 처리한 뒤 공용 transport를 적용한다.
			// Resources의 Stop이 같은 UI 프레임에 들어온 Pattern·행 시작도 취소할 수 있게 한다.
			if (nullptr != shell)
			{
				CSequencerTool::ANIMATION_PREVIEW_TRANSPORT transport =
					CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::NONE;
				const bool completeEntryPending =
					route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK && !m_strKoukuCompletePlayFlowGate.empty();
				bool hasTransport = shell->Consume_AnimationPreviewTransportRequest(transport);
				if (hasTransport && shell->Uses_ValtanSession() && m_pAnimationTool)
				{
					const auto valtan = m_pAnimationTool->Get_ValtanCompositionPreviewState();
					if (valtan.bPlaying)
					{
						if (transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::STOP)
							m_pAnimationTool->Stop_ValtanCompositionPattern(m_strToolStatus);
						else if (valtan.bSourceSequencePlaying)
						{
							const bool pause = transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::PAUSE;
							const uint32_t position =
								!pause && valtan.bPaused && valtan.iPositionMs >= valtan.iDurationMs
									? 0u
									: valtan.iPositionMs;
							(void)m_pAnimationTool->Seek_ValtanCompositionSourceSequence(position, pause,
																						 m_strToolStatus);
						}
						else
							(void)m_pAnimationTool->Seek_ValtanCompositionPattern(
								valtan.strPatternId, valtan.iPositionMs,
								transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::PAUSE, m_strToolStatus);
						shell->Set_AnimationPreviewStatus(m_strToolStatus);
						hasTransport = false;
					}
				}
				if (hasTransport &&
					(m_eCompositionPreviewOwner == route.owner ||
					 (completeEntryPending && transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::STOP)))
				{
					if (transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::STOP)
					{
						if (completeEntryPending)
							CancelKoukuGateCompletePlay("Complete Play stopped before Pattern Flow.");
						else
							StopCompositionPreview(route.owner);
					}
					else if (transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::RESUME && workbench)
					{
						// 재개 전에 Workbench가 변경 불가능한 snapshot을 검증한다.
						// 생성된 transport 또는 갱신 요청은 다음 프레임에 소비한다.
						(void)workbench->Request_PreviewResume();
					}
					else if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_OwnsClock())
					{
						if (transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::PAUSE)
							m_pKoukuPresentationPlayer->Pause_Preview(true);
						if (transport == CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::RESUME)
							m_pKoukuPresentationPlayer->Pause_Preview(false);
					}
					else if (nullptr != m_pAnimationTool)
					{
						switch (transport)
						{
						case CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::PAUSE:
							(void)m_pAnimationTool->Set_KoukuCompositionPreviewPaused(true, m_strToolStatus);
							break;
						case CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::RESUME:
							(void)m_pAnimationTool->Set_KoukuCompositionPreviewPaused(false, m_strToolStatus);
							break;
						case CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::STOP:
						case CSequencerTool::ANIMATION_PREVIEW_TRANSPORT::NONE:
							break;
						}
					}
					else
						m_strToolStatus = "No animation preview backend is active.";
					shell->Set_AnimationPreviewStatus(m_strToolStatus);
				}
			}

			if (!previewRouteStatus.empty())
			{
				auto state = workbench->Get_PreviewState();
				state.strStatus = previewRouteStatus;
				workbench->Set_PreviewState(state);
				if (shell)
					shell->Set_AnimationPreviewStatus(previewRouteStatus);
				if (m_eCompositionPreviewOwner == route.owner)
				{
					activePreviewRouteStatus = previewRouteStatus;
					previewStatusOwner = route.owner;
				}
			}
		}
		if (!m_pKoukuPresentationPlayer || !m_pKoukuPresentationPlayer->Preview_IsServerClock())
		{
			KOUKU_PREVIEW_STATE finalPreview;
			std::string worldAnchorWaitingStatus;
			std::string completedPreviewId;
			bool previewFailed = false;
			const auto rejectPreview = [&](const std::string &patternId, const std::string &error)
			{
				previewFailed = true;
				completedPreviewId.clear();
				finalPreview.strPatternId = patternId;
				finalPreview.bPlaying = finalPreview.bPaused = false;
				finalPreview.strStatus = error;
				const auto failedOwner = m_eCompositionPreviewOwner;
				if (failedOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK && !m_strKoukuCompletePlayFlowGate.empty())
					CancelKoukuGateCompletePlay(error);
				else
					StopCompositionPreview(failedOwner);
				for (const auto &route : compositionRoutes)
					if (route.owner == failedOwner)
					{
						if (route.workbench)
						{
							route.workbench->Notify_SequencePreviewAdmission(false, error);
							route.workbench->Set_PreviewState(finalPreview);
						}
						if (route.shell)
							route.shell->Set_AnimationPreviewStatus(error);
					}
			};
			if (m_pKoukuPresentationPlayer)
			{
				std::string failedPatternId, failureStatus;
				if (m_pKoukuPresentationPlayer->Consume_FailedPreview(failedPatternId, failureStatus))
					rejectPreview(failedPatternId, failureStatus);
				else
					(void)m_pKoukuPresentationPlayer->Consume_CompletedPreview(completedPreviewId);
			}
			const bool ownedClock = m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_OwnsClock();
			if (previewFailed)
			{ /* 실패 처리에서 이미 finalPreview에 종료 오류를 기록했다. */
			}
			else if (!completedPreviewId.empty())
			{
				finalPreview.strPatternId = completedPreviewId;
				finalPreview.iClockMs = finalPreview.iDurationMs = m_pKoukuPresentationPlayer->Preview_DurationMs();
				finalPreview.strStatus = "Sequence preview completed.";
			}
			else if (ownedClock)
			{
				finalPreview.strPatternId = m_pKoukuPresentationPlayer->Preview_PatternId();
				finalPreview.bPlaying = m_pKoukuPresentationPlayer->Preview_Playing();
				finalPreview.bPaused = m_pKoukuPresentationPlayer->Preview_Paused();
				finalPreview.iClockMs = m_pKoukuPresentationPlayer->Preview_ClockMs();
				finalPreview.iDurationMs = m_pKoukuPresentationPlayer->Preview_DurationMs();
				finalPreview.strStatus = m_pKoukuPresentationPlayer->Status();
			}
			else if (m_pAnimationTool)
			{
				const auto sampled = m_pAnimationTool->Get_KoukuCompositionPreviewState();
				finalPreview.strPatternId = sampled.strPatternId;
				finalPreview.bPlaying = sampled.bPlaying;
				finalPreview.bPaused = sampled.bPaused;
				finalPreview.iClockMs = sampled.iClockMs;
				finalPreview.iDurationMs = sampled.iDurationMs;
				finalPreview.strStatus = sampled.strStatus;
			}
			if (!previewFailed && finalPreview.bPlaying && m_pKoukuPresentationPlayer &&
				!m_pKoukuPresentationPlayer->Preview_IsBundle())
			{
				std::uint32_t effectiveMs = finalPreview.iClockMs;
				if (!m_pKoukuPresentationPlayer->Resolve_PreviewCaptureClock(finalPreview.iClockMs, effectiveMs))
				{
					const std::string captureError = m_pKoukuPresentationPlayer->Status();
					rejectPreview(finalPreview.strPatternId, captureError);
				}
				else
				{
					// 사용자가 요청한 cursor를 표시하기 전에 WORLD·애니메이션·카메라가 같은 capture 경계를 샘플해야 한다.
					// 필요하면 animation 시계도 해당 effectiveMs로 맞춘다.
					if (effectiveMs != finalPreview.iClockMs && !ownedClock && m_pAnimationTool)
					{
						std::string captureStatus;
						if (!m_pAnimationTool->Seek_KoukuCompositionPreview(effectiveMs, captureStatus))
							rejectPreview(finalPreview.strPatternId, captureStatus);
					}
					finalPreview.iClockMs = effectiveMs;
				}
			}
			if (auto *arena = CLevel_KoukuSaydonArena::Get_Active();
				!previewFailed && arena &&
				(!m_pKoukuPresentationPlayer || !m_pKoukuPresentationPlayer->Preview_IsBundle()))
			{
				std::string worldPreviewStatus;
				if (!arena->Debug_SampleCompositionWorldPreview(finalPreview.strPatternId, finalPreview.bPlaying,
																finalPreview.iClockMs, worldPreviewStatus))
				{
					rejectPreview(finalPreview.strPatternId, worldPreviewStatus);
				}
				else
					worldAnchorWaitingStatus = worldPreviewStatus;
			}
			if (!previewFailed && m_pKoukuPresentationPlayer && !m_pKoukuPresentationPlayer->Preview_IsBundle())
			{
				float4x4_t pivot{};
				const bool colliderResource = m_pKoukuPresentationPlayer->Preview_IsColliderResource();
				auto model =
					colliderResource ? std::shared_ptr<Engine::CModel>{} : CAnimationTargetService::Resolve_Model();
				bool pivotReady = !colliderResource && CAnimationTargetService::Resolve_RootTransform(&pivot);
				if (!pivotReady)
				{
					const auto *arena = CLevel_KoukuSaydonArena::Get_Active();
					const auto character = arena ? arena->Get_LocalCharacter() : nullptr;
					if (character && character->Get_Transform())
					{
						pivot = *character->Get_Transform()->Get_WorldMatrixPtr();
						model = character->Get_BodyModel();
						pivotReady = true;
					}
				}
				if (pivotReady && finalPreview.strPatternId == m_pKoukuPresentationPlayer->Preview_PatternId())
				{
					m_pKoukuPresentationPlayer->Sample_Preview(finalPreview.iClockMs, finalPreview.bPlaying,
															   finalPreview.bPaused, pivot, model);
					if (!m_pKoukuPresentationPlayer->Status().empty())
						finalPreview.strStatus = m_pKoukuPresentationPlayer->Status();
				}
				else if (!pivotReady && finalPreview.bPlaying)
					finalPreview.strStatus = "Preview requires an active arena character or animation target.";
			}
			if (!previewFailed && m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
			{
				std::string arrivalStatus;
				if (!SampleKoukuSequenceArrivals(
						finalPreview.iClockMs,
						(finalPreview.bPlaying && !finalPreview.bPaused) || !completedPreviewId.empty(), arrivalStatus))
					rejectPreview(finalPreview.strPatternId, arrivalStatus);
				else
				{
					if (!completedPreviewId.empty())
						m_strKoukuSequenceArrivalCompletedPreview = completedPreviewId;
					if (!m_strKoukuSequenceArrivalCompletedPreview.empty())
					{
						if (KoukuSequenceArrivalsPending())
						{
							completedPreviewId.clear();
							finalPreview.strStatus = "Sequence finished; waiting for Server player arrival approval.";
						}
						else
							completedPreviewId = std::exchange(m_strKoukuSequenceArrivalCompletedPreview, {});
					}
				}
			}
			if (previewStatusOwner == m_eCompositionPreviewOwner && !activePreviewRouteStatus.empty())
				finalPreview.strStatus = activePreviewRouteStatus;
			if (!previewFailed && !worldAnchorWaitingStatus.empty())
				finalPreview.strStatus = worldAnchorWaitingStatus;
			for (const auto &route : compositionRoutes)
			{
				if (route.owner != m_eCompositionPreviewOwner)
					continue;
				if (route.workbench)
				{
					route.workbench->Set_PreviewState(finalPreview);
					std::string completedGate;
					if (!completedPreviewId.empty() &&
						route.workbench->Advance_CompleteSequencePlay(completedPreviewId, &completedGate))
					{
						finalPreview.strStatus = route.workbench->Get_Status();
						if (!route.workbench->Is_CompleteSequencePlaying())
						{
							m_bKoukuCompletePreservesArrivalPosition =
								std::any_of(m_KoukuSequenceArrivalCues.begin(), m_KoukuSequenceArrivalCues.end(),
											[](const auto &cue) { return cue.moved; });
							StopCompositionPreview(route.owner);
							if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
								arena->Debug_ReturnToPlayerCamera();
						}
						if (!completedGate.empty() && route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
							FinishKoukuGateCompletePlay(completedGate, finalPreview.strStatus);
						route.workbench->Set_PreviewState(finalPreview);
					}
					else if (!completedPreviewId.empty() && route.owner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
					{
						StopCompositionPreview(route.owner);
						if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
							arena->Debug_ReturnToPlayerCamera();
						m_eDebugInputOwner = DEBUG_TOOL::NONE;
						finalPreview.strStatus = "Sequence finished. Player camera restored; combat was not started.";
						route.workbench->Set_PreviewState(finalPreview);
					}
				}
				if (!route.shell)
					continue;
				CSequencerTool::ANIMATION_PREVIEW_STATE state;
				state.strPatternId = finalPreview.strPatternId;
				state.strStatus = finalPreview.strStatus;
				state.bPlaying = finalPreview.bPlaying;
				state.bPaused = finalPreview.bPaused;
				state.iClockMs = finalPreview.iClockMs;
				state.iDurationMs = finalPreview.iDurationMs;
				route.shell->Set_AnimationPreviewState(std::move(state));
			}

			if (m_pSequencerTool && m_pSequencerTool->Uses_ValtanSession() && m_pAnimationTool)
			{
				const auto valtan = m_pAnimationTool->Get_ValtanCompositionPreviewState();
				// Workbench가 공용 리소스 경로를 점유하지 않고 재생했어도 정지 상태를 도구에 전달한다.
				// 따라서 재생 중인 상태만 골라서 게시하지 않는다.
				if (valtan.bPlaying || !finalPreview.bPlaying)
				{
					CSequencerTool::ANIMATION_PREVIEW_STATE state;
					state.strPatternId = valtan.strPatternId;
					state.strStatus = valtan.strStatus;
					state.bPlaying = valtan.bPlaying;
					state.bPaused = valtan.bPaused;
					state.iClockMs = valtan.iPositionMs;
					state.iDurationMs = valtan.iDurationMs;
					m_pSequencerTool->Set_AnimationPreviewState(std::move(state));
				}
			}
		}
		// 현재 프레임의 Seek/Stop 이후 모델·환경·카메라를 다시 샘플해 Render 전 시각을 맞춘다.
		// 소유권을 넘기면 카메라 override도 종료한다.
		const auto valtanPreview = m_pAnimationTool ? m_pAnimationTool->Get_ValtanCompositionPreviewState()
													: CAnimation_Tool::COMPOSITION_PREVIEW_STATE{};
		const bool activeValtanSession = m_pSequencerTool && m_pSequencerTool->Uses_ValtanSession() &&
										 m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) &&
										 m_pAnimationTool && valtanPreview.bPlaying;
		bool valtanCinematicSampleReady = true;
		std::string cinematicStatus;
		if (auto *arena = CLevel_ValtanArena::Get_Active())
		{
			if (activeValtanSession)
			{
				std::string wallStatus;
				const bool wallReady = arena->Debug_SampleActionWorkbenchDestruction(
					valtanPreview.strPatternId, valtanPreview.iPositionMs, wallStatus);
				valtanCinematicSampleReady = arena->Debug_SampleActionWorkbenchCinematic(
					valtanPreview.strPatternId, valtanPreview.iPositionMs, valtanPreview.bPaused,
					CAnimationTargetService::Resolve_Boss(), cinematicStatus);
				if (!wallReady)
				{
					cinematicStatus += " | " + wallStatus;
					m_strToolStatus = wallStatus;
				}
				else if (valtanPreview.strPatternId == "VALTAN_ARENA_BREAK_109")
					cinematicStatus += " | " + wallStatus;
				if (!valtanCinematicSampleReady && !cinematicStatus.empty())
					m_strToolStatus = cinematicStatus;
			}
			else
			{
				std::string wallStatus;
				if (!arena->Debug_StopActionWorkbenchDestruction(wallStatus))
					m_strToolStatus = wallStatus;
				arena->Debug_StopActionWorkbenchCinematic();
			}
		}
		if (m_pValtanActionWorkbench)
			m_pValtanActionWorkbench->Set_CinematicPreviewStatus(cinematicStatus);
		std::vector<BOSS_STAGE_ENVIRONMENT_SAMPLE> currentValtanSamples;
		CValtan::Collect_StageEnvironmentSamples(currentValtanSamples);
		if (m_pKoukuPresentationPlayer)
			m_pKoukuPresentationPlayer->Update_BossStageEnvironments(currentValtanSamples);
		// 발탄 Workbench의 카메라 preview는 독립 Camera Tool 창을 열지 않아도 동작해야 한다.
		// 첫 활성 preview에서 문서·sampler 소유자를 지연 생성한다.
		if (activeValtanSession && nullptr == m_pCameraTool)
			m_pCameraTool = make_unique<CCameraTool>();
		if (m_pCameraTool)
		{
			const auto local = std::find_if(currentValtanSamples.begin(), currentValtanSamples.end(),
											[](const auto &sample) { return sample.bPreview; });
			if (activeValtanSession && valtanCinematicSampleReady && local != currentValtanSamples.end())
			{
				std::string cameraStatus;
				if (!m_pCameraTool->Sample_CompositionPreview(*local, cameraStatus) && !cameraStatus.empty())
					m_strToolStatus = std::move(cameraStatus);
			}
			else
				m_pCameraTool->Stop_CompositionPreview();
		}
		if (m_pCharacterActionWorkbench)
			m_pCharacterActionWorkbench->Update(
				fTimeDelta, m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) && m_pSequencerTool &&
								m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::CHARACTER &&
								m_eDebugInputOwner == DEBUG_TOOL::SEQUENCER);
		if (m_pWorldObjectTool)
		{
			Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.WorldObjects.Update");
			m_pWorldObjectTool->Update(fTimeDelta, m_bDeveloperToolsVisible &&
													   IsDebugToolVisible(DEBUG_TOOL::WORLD_OBJECT) &&
													   DEBUG_TOOL::WORLD_OBJECT == m_eDebugInputOwner);
		}
		if (m_pWorldObjectTool)
		{
			std::string patternId, status;
			uint32_t sourceRevision = 0u;
			if (m_pWorldObjectTool->Consume_ServerPlayRequest(patternId, sourceRevision))
			{
				if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::KOUKU_SAYDON_BOSS)) && m_pKoukuSaydonBossTool)
				{
					if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
						arena->Debug_StopCompositionWorldPreview();
					if (m_pKoukuPresentationPlayer)
						m_pKoukuPresentationPlayer->Stop_Preview();
					ClaimCompositionPreviewOwner(DEBUG_TOOL::NONE);
					(void)m_pKoukuSaydonBossTool->Play_PatternById(patternId, sourceRevision, status);
				}
				else
					status = "KoukuSaydon Boss Tool could not prepare Server collision playback.";
				m_pWorldObjectTool->Set_ServerPlayStatus(std::move(status));
			}
			if (m_pWorldObjectTool->Consume_ServerStopRequest())
			{
				if (m_pKoukuSaydonBossTool)
					(void)m_pKoukuSaydonBossTool->Cancel_PlayPreparation(status);
				(void)CKoukuSaydonPatternAuditionService::Get().Stop(status);
				m_pWorldObjectTool->Set_ServerPlayStatus(std::move(status));
			}
			m_pWorldObjectTool->Set_ServerPlayPreparationPending(m_pKoukuSaydonBossTool &&
																 m_pKoukuSaydonBossTool->Is_PlayPreparationPending());
		}
		RefreshWorldObjectResources();
		if (nullptr != m_pCameraTool)
		{
			{
				Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Camera.Update");
				m_pCameraTool->Update(fTimeDelta, m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::CAMERA) &&
													  DEBUG_TOOL::CAMERA == m_eDebugInputOwner);
			}
		}
	}
#endif

	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "MainApp.LevelAndEnvironment.Update");
		string environmentStatus;
		const auto *cinematicArena = CLevel_KoukuSaydonArena::Get_Active();
		const bool_t koukuCinematic = cinematicArena &&
									  CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KOUKUSAYDON_ARENA) &&
									  cinematicArena->Is_CinematicPresentationActive();
		const auto *valtanArena = CLevel_ValtanArena::Get_Active();
		const bool_t valtanCinematic = valtanArena &&
									   CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA) &&
									   valtanArena->Is_CinematicCameraActive();
		const bool_t marioStage = cinematicArena &&
								  CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KOUKUSAYDON_ARENA) &&
								  cinematicArena->Is_LocalMarioStageActive();
		// 선박 승선 중의 안개 설정은 Bern camera의 shipFog 입력을 presentation 환경에 전달한다.
		// disable이 아니면 밀도 배율·시작 거리(m)·최대 불투명도를 넘기며 저작 profile 자체를 저장하지 않는다.
		const ARENA_SHIP_FOG *pShipFog = CLevel_Bern::Get_ActiveShipFog();
		PRESENTATION_FOG_TUNING shipFogTuning{};
		const bool_t shipFogThinned = nullptr != pShipFog && !pShipFog->disable;
		if (shipFogThinned)
		{
			shipFogTuning.fDensityScale = pShipFog->densityScale;
			shipFogTuning.fStartDistanceMeters = pShipFog->startDistanceMeters;
			shipFogTuning.fMaximumOpacity = pShipFog->maximumOpacity;
		}
		const auto localCharacter = CAnimationTargetService::Resolve_SceneCharacter();
		float controlsBrightness = 1.f;
		float4_t controlsColor{};
		const auto selectedCharacter = CAnimationTargetService::Resolve_Character();
		if (!(selectedCharacter &&
			  selectedCharacter->Get_PresentationDirectionalControl(controlsBrightness, controlsColor)) &&
			localCharacter)
			localCharacter->Get_PresentationDirectionalControl(controlsBrightness, controlsColor);
		// 탈것 cue의 표시 조정과 별개로 현재 선택 캐릭터 또는 장면 캐릭터의 명시적인 방향광 조정만 사용한다.
		// 선택한 밝기는 아래에서 0~16 범위로 제한한다.
		controlsBrightness = std::clamp(controlsBrightness, 0.f, 16.f);
		// Lobby의 캐릭터 선택 창이 열리면 얼굴을 비추는 방향광 값을 이 프레임의 presentation override로 전달한다.
		// 저장된 Lobby profile의 조명값을 바꾸지는 않는다.
		LIGHT_DESC characterSelectLight{};
		const bool_t characterSelectLit = nullptr != m_pCharacterSelectWindowView &&
										  m_pCharacterSelectWindowView->Is_Open() &&
										  CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::LOBBY);
		if (characterSelectLit)
		{
			characterSelectLight.eType = LIGHT::DIRECTIONAL;
			XMStoreFloat4(&characterSelectLight.vDirection,
						  XMVector3Normalize(XMVectorSet(-0.35f, -0.6f, -0.72f, 0.f)));
			characterSelectLight.vDiffuse = float4_t(1.3f, 1.3f, 1.35f, 1.f);
			characterSelectLight.vAmbient = float4_t(0.53f, 0.53f, 0.55f, 1.f);
			characterSelectLight.vSpecular = float4_t(0.f, 0.f, 0.f, 1.f);
		}
		if (!m_RenderingProfiles.Apply_CameraEnvironment(
				fTimeDelta, environmentStatus, koukuCinematic || valtanCinematic || marioStage || nullptr != pShipFog,
				characterSelectLit ? &characterSelectLight : nullptr, controlsBrightness, &controlsColor,
				shipFogThinned ? &shipFogTuning : nullptr))
			OutputDebugStringA((environmentStatus + "\n").c_str());
		Update_ClassShowcaseShadowFocus();
	}

	// 각 제품 표면의 화면 영역과 layer를 등록해 위쪽 창이 가리는 부분의 글자만 숨긴다.
	Sync_CinematicUI();
	Update_WorldHealthBars(fTimeDelta);
	Update_MechanicBarAnchor();
	if (!CUIInputRouter::Get().Is_CinematicSuppressed())
		Register_UITextOccluders();
}

// Update 끝에서 현재 제품 창·HUD의 화면 pixel 영역과 layer를 수집한다.
// Render의 텍스트 pass가 이 목록을 읽으며 닫힌 view의 rect 노출 여부는 각 view가 판단한다.
void CMainApp::Register_UITextOccluders()
{
	CUITextOcclusion &Occlusion = CUITextOcclusion::Get();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	// sprite 생성 순서와 같은 창 layer를 사용해 화면 rect를 occluder로 등록한다.
	if (nullptr != m_pInventoryView && m_pInventoryView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_INVENTORY, fX, fY, fWidth, fHeight);
	if (nullptr != m_pRepairWindowView && m_pRepairWindowView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_REPAIR, fX, fY, fWidth, fHeight);
	if (nullptr != m_pShopWindowView && m_pShopWindowView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_SHOP, fX, fY, fWidth, fHeight);
	if (nullptr != m_pCharacterInfoView && m_pCharacterInfoView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_CHARACTER_INFO, fX, fY, fWidth, fHeight);
	if (nullptr != m_pAvatarBookView && m_pAvatarBookView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_AVATAR_BOOK, fX, fY, fWidth, fHeight);
	if (nullptr != m_pVehicleWindowView && m_pVehicleWindowView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_VEHICLE, fX, fY, fWidth, fHeight);
	if (nullptr != m_pHonorTitleWindowView && m_pHonorTitleWindowView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_HONOR_TITLE, fX, fY, fWidth, fHeight);
	if (nullptr != m_pWorldMapWindowView && m_pWorldMapWindowView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_WORLD_MAP, fX, fY, fWidth, fHeight);
	if (nullptr != m_pSystemOptionView && m_pSystemOptionView->Get_ScreenRect(fX, fY, fWidth, fHeight))
		Occlusion.Add_Occluder(UI_TEXT_LAYER::WINDOW_SYSTEM_OPTION, fX, fY, fWidth, fHeight);
	// 재련의 대기·결과 화면도 전체 창 배경 영역을 사용한다. 중앙 panel만 등록하면 양옆 목록 위로 월드 이름표가 보인다.
	if (m_bItemUpgradePreviewVisible && nullptr != m_pItemUpgradeView)
		Occlusion.Add_SlotOccluder(UI_TEXT_LAYER::WINDOW_ITEM_UPGRADE, *m_pItemUpgradeView, "ItemUpgrade_WindowBg");
	// 파티 HUD를 표시하는 Level에서만 해당 view의 가림 영역을 등록한다.
	const uint32_t iLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (nullptr != m_pPartyWindowView && (ETOUI(LEVEL::BERN) == iLevel || ETOUI(LEVEL::VALTAN_ARENA) == iLevel ||
										  ETOUI(LEVEL::KOUKUSAYDON_ARENA) == iLevel))
		m_pPartyWindowView->Register_TextOccluders();
	// MVP 결과와 캐릭터 생성은 viewport 전체를 가린다. Bern의 입장 창은 해당 Level이 별도로 등록한다.
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	if (Is_MvpResultPageOpen())
		Occlusion.Add_Occluder(UI_TEXT_LAYER::PAGE, 0.f, 0.f, vViewport.x, vViewport.y);
	if (ETOUI(LEVEL::CHARACTER_SELECT) == iLevel && nullptr != CLevel_CharacterSelect::Get_Active() &&
		CLevel_CharacterSelect::Get_Active()->Is_CustomizingOpen())
		Occlusion.Add_Occluder(UI_TEXT_LAYER::MODAL, 0.f, 0.f, vViewport.x, vViewport.y);
}

// Bern에서 Colosseum LOAD 요청이 남아 있는 Render에서 현재 캐릭터를 전용 target으로 그린다.
// 이후 Level 교체 뒤에도 Loading이 사용할 수 있도록 SRV의 COM 소유 참조를 전환 service에 전달한다.
void CMainApp::Render_ColosseumTransferPortrait()
{
	const uint32_t iLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (ETOUI(LEVEL::BERN) != iLevel)
	{
		// LOADING 동안은 초상화를 유지하고 다른 Level이 활성화되면 service의 참조를 해제한다.
		if (ETOUI(LEVEL::LOADING) != iLevel)
			CLevelTransitionService::Set_TransferPortraitSRV(nullptr);
		return;
	}

	LEVEL_TRANSITION_REQUEST Request;
	if (!CLevelTransitionService::Peek_Pending(Request) || LEVEL_TRANSITION_PHASE::LOAD != Request.ePhase ||
		LEVEL::COLOSSEUM != Request.eTargetLevel)
		return;

	CLevel_Bern *pBern = CLevel_Bern::Get_Active();
	const shared_ptr<CCharacter> pCharacter = nullptr != pBern ? pBern->Get_LocalCharacter() : nullptr;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	if (nullptr == pCharacter || vViewport.x <= 0.f || vViewport.y <= 0.f)
	{
		CLevelTransitionService::Set_TransferPortraitSRV(nullptr);
		OutputDebugStringA("[Colosseum.MatchLoading] portrait snapshot skipped: no local character\n");
		return;
	}
	if (nullptr == m_pTransferPortrait)
		m_pTransferPortrait = std::make_unique<CCharacterPortraitRenderer>(m_pDevice, m_pContext);

	// 1280×720 저작 layout에서 373.333×324인 초상화 슬롯을 현재 viewport pixel 크기로 환산한다.
	const uint32_t iWidth = static_cast<uint32_t>(std::lround(373.333f * vViewport.x / 1280.f));
	const uint32_t iHeight = static_cast<uint32_t>(std::lround(324.f * vViewport.y / 720.f));
	// 기존 튜닝 기록은 capture 171625의 머리·눈 위치를 기준으로 class별 눈높이에 비례한 화면 span을 사용했다.
	// 고정 1.3m span 대신 각 캐릭터 크기를 반영한 배경이며 아래 상수는 후속 조정값이다.
	// 기존 후속 튜닝 기록은 capture 052127/211807 비교 뒤 span을 눈높이의 0.71배로 조정했다.
	// 아래 상수는 그 후속값을 사용하며 화면 옆 이동과 yaw를 0으로 둔다.
	constexpr f32_t PORTRAIT_SPAN_PER_EYE = 0.71f;
	constexpr f32_t PORTRAIT_LOOK_PER_EYE = 0.767f;
	constexpr f32_t PORTRAIT_FOV_DEGREES = 30.f;
	constexpr f32_t PORTRAIT_YAW_DEGREES = 0.f;
	constexpr f32_t PORTRAIT_SHIFT_RIGHT = 0.f;
	const f32_t fScale = (std::max)(pCharacter->Get_PresentationScale(), 0.1f);
	f32_t fEyeHeight = 0.f;
	if (!CCharacterPortraitRenderer::Try_Measure_EyeHeight(*pCharacter, fEyeHeight))
		fEyeHeight = 1.04f * fScale;
	// 모델의 눈높이로 세로 span(m)을 정하고 수직 FOV의 반각으로 필요한 카메라 거리(m)를 구한다.
	const f32_t fSpan = PORTRAIT_SPAN_PER_EYE * fEyeHeight;
	CCharacterPortraitRenderer::CAMERA Camera{};
	Camera.fFovDegrees = PORTRAIT_FOV_DEGREES;
	Camera.fLookHeight = PORTRAIT_LOOK_PER_EYE * fEyeHeight;
	Camera.fEyeHeight = Camera.fLookHeight;
	Camera.fDistance = fSpan / (2.f * std::tan(XMConvertToRadians(Camera.fFovDegrees) * 0.5f));
	Camera.fYawDegrees = PORTRAIT_YAW_DEGREES;
	Camera.fLateralOffset = -PORTRAIT_SHIFT_RIGHT * fSpan * static_cast<f32_t>(iWidth) / static_cast<f32_t>(iHeight);
	// 장착 외형은 유지하고 무기만 숨긴 뒤 초상화를 그린다. 곧 Level이 바뀌는 경로이며 이 함수에서 무기 표시를 복원하지 않는다.
	pCharacter->Set_WeaponPartsVisible(false);
	const HRESULT hResult = m_pTransferPortrait->Render(pCharacter, iWidth, iHeight, Camera, 0u, 0u);
	{
		char szLog[256];
		sprintf_s(
			szLog,
			"portrait=3d eyeWorld=%.3f dist=%.2f camH=%.2f look=%.2f fov=%.0f yaw=%.0f lateral=%.3f scale=%.2f rt=%ux%u\n",
			fEyeHeight, Camera.fDistance, Camera.fEyeHeight, Camera.fLookHeight, Camera.fFovDegrees, Camera.fYawDegrees,
			Camera.fLateralOffset, fScale, iWidth, iHeight);
		OutputDebugStringA("[Colosseum.MatchLoading] ");
		OutputDebugStringA(szLog);
		Write_EffectFailureDiagnostic("Colosseum.MatchLoading", szLog);
	}
	// 정확히 S_OK인 렌더 결과만 Loading으로 넘긴다. 실패하면 이전 service 이미지를 비워 2D 대체 표시를 허용한다.
	CLevelTransitionService::Set_TransferPortraitSRV(S_OK == hResult ? m_pTransferPortrait->Get_SRV() : nullptr);
	if (S_OK != hResult)
		OutputDebugStringA("[Colosseum.MatchLoading] portrait snapshot failed: render returned not-OK\n");
}
// 제품 메인 루프가 Update 뒤 호출한다. 초상화 target → 월드·sprite → ImGui → 보스 showcase → 텍스트 순으로 제출한다.
// begin/world 실패는 진단 후 ImGui frame을 취소하고 원래 HRESULT를 반환하며, 정상 제출 후 Render_End 결과를 그대로 반환한다.
HRESULT CMainApp::Render()
{
	// 세션 진단에 실패 단계와 장치 제거 사유를 보충한다. 예외는 격리하고 연결 상태를 복구하거나 종료하지 않는다.
	const auto recordRenderFailure = [this](const char *stage, HRESULT result) noexcept
	{
		try
		{
			CNetworkManager::Get().Record_SessionEvent(
				"render.failed",
				std::string("stage=") + stage + "; hresult=" + std::to_string(static_cast<unsigned long>(result)) +
					"; deviceRemovedReason=" +
					std::to_string(
						static_cast<unsigned long>(m_pDevice ? m_pDevice->GetDeviceRemovedReason() : E_POINTER)));
		}
		catch (...)
		{
		} // 진단 중 예외가 나도 원래 렌더 실패값과 전송 상태를 바꾸지 않는다.
	};
	Sync_CinematicUI();
	float4_t clearColor = {0.008f, 0.012f, 0.025f, 1.f};
	HRESULT hBeginResult;
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.BeginFrame");
		Engine::CProfilerGpuScope gpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.BeginFrame");
		hBeginResult = CGameInstance::Get().Render_Begin(&clearColor);
	}
	if (FAILED(hBeginResult))
	{
		recordRenderFailure("begin-frame", hBeginResult);
		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->CancelFrame();
		return hBeginResult;
	}

	// 월드/UI가 SRV를 읽기 전에 실행할 초상화 draw를 예약한다. 실제 target 렌더는 뒤의 Engine Renderer가 처리한다.
	if (!CUIInputRouter::Get().Is_CinematicSuppressed())
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.Portraits");
		Engine::CProfilerGpuScope gpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.Portraits");
		if (nullptr != m_pCharacterInfoView)
			(void)m_pCharacterInfoView->Render_Portrait();
		if (nullptr != m_pAvatarBookView)
			(void)m_pAvatarBookView->Render_Portrait();
		if (nullptr != m_pCharacterSelectWindowView && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::LOBBY))
			m_pCharacterSelectWindowView->Render_Portraits();
		if (auto *pArena = CLevel_KoukuSaydonArena::Get_Active();
			nullptr != pArena && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KOUKUSAYDON_ARENA))
			pArena->Render_MvpPortraits();
		if (auto *pValtan = CLevel_ValtanArena::Get_Active();
			nullptr != pValtan && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
			pValtan->Render_MvpPortraits();
	}
	Render_ColosseumTransferPortrait();
	if (auto *colosseum = CLevel_Development::Get_Active(LEVEL::COLOSSEUM))
		colosseum->Submit_ColosseumLoadingPortraits();

	CEffectPresentationService::Submit_VisibleLevelPresentations();

	// 이번 프레임 Composition의 WORLD/Seek/Stop을 반영한 뒤 맵 조명의 소유자를 선택한다.
	if (auto *arena = CLevel_KoukuSaydonArena::Get_Active();
		arena && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KOUKUSAYDON_ARENA))
	{
		arena->Submit_MapLightFrame();
		arena->Trace_CinematicPresentation(m_RenderingProfiles.Get_ActiveProfileId());
		arena->Submit_EntranceTriggerMarkers();
	}
	if (auto *pValtanArena = CLevel_ValtanArena::Get_Active();
		pValtanArena && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA))
		pValtanArena->Submit_TriggerMarkers();
	if (auto *pMaharaka = CLevel_Development::Get_Active(LEVEL::MAHARAKA);
		pMaharaka && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::MAHARAKA))
		pMaharaka->Submit_TriggerMarkers();
	if (m_pProfilerTool) m_pProfilerTool->Begin_CutsceneDiagnosticFrame();
	HRESULT hWorldResult;
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.World");
		// 이름표·말풍선 같은 Level 글자는 WORLD layer로 처리해 제품 UI 표면 아래에서 가려지게 한다.
		CUITextOcclusion::Get().Apply(UI_TEXT_LAYER::WORLD);
		hWorldResult = CGameInstance::Get().Render(CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::BERN));
	}
	if (m_pProfilerTool) m_pProfilerTool->End_CutsceneDiagnosticFrame(SUCCEEDED(hWorldResult));
	if (FAILED(hWorldResult))
	{
		recordRenderFailure("world", hWorldResult);
		if (nullptr != m_pImGuiLayer)
			m_pImGuiLayer->CancelFrame();
		return hWorldResult;
	}

	if (nullptr != m_pImGuiLayer)
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "ImGui.BuildAndSubmit");
		// Lobby 제품 sprite는 Update_LobbyButtons가 갱신하고 앞선 엔진 렌더 경로가 그린다.
#ifdef _DEBUG
		const HUD_PLAYER_STATE &hudPlayer = CCombatHUDViewModel::Get().Get_Player();
		const uint32_t hudLevel = CGameInstance::Get().Get_CurrentLevelID();
		const bool_t supportsAuthoredHUD = ETOUI(LEVEL::CHARACTER_SELECT) == hudLevel ||
										   ETOUI(LEVEL::DEVELOPMENT) == hudLevel || ETOUI(LEVEL::BERN) == hudLevel ||
										   ETOUI(LEVEL::VALTAN_ARENA) == hudLevel;
		const bool_t skillWindowOpenForPreview = nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
		// 캐릭터 선택의 입장 preview가 class HUD와 겹치므로 저작 HUD preview도 제품 sprite와 같은 표시 조건을 적용한다.
		const bool_t isCharSelectDebugPreviewOpen =
			ETOUI(LEVEL::CHARACTER_SELECT) == hudLevel && nullptr != CLevel_CharacterSelect::Get_Active() &&
			CLevel_CharacterSelect::Get_Active()->Is_DebugRaidEntryPreviewOpen();
		// 캐릭터 생성·제품 presentation은 같은 Level 위의 전체 화면이다. 아래 class HUD가 겹치지 않게 공통 표시 조건에 포함한다.
		const bool_t isCharSelectOverlayOpen =
			isCharSelectDebugPreviewOpen ||
			(ETOUI(LEVEL::CHARACTER_SELECT) == hudLevel && nullptr != CLevel_CharacterSelect::Get_Active() &&
			 CLevel_CharacterSelect::Get_Active()->Is_ProductPresentationOpen());
		if (!CUIInputRouter::Get().Is_CinematicSuppressed() && nullptr != m_pHUDLayoutTool && hudPlayer.isValid &&
			supportsAuthoredHUD && !skillWindowOpenForPreview && !isCharSelectOverlayOpen)
		{
			m_pHUDLayoutTool->Render_RuntimePreview(GetHUDLayoutClassId(hudPlayer.eCharacterClass));
		}
#endif
		// 전투 HUD도 Update_CombatHUD가 실제 CUI_Sprite를 갱신하며 그리기는 앞선 엔진 pass가 담당한다.
		// 입장 modal의 foreground 목록은 제출 순서대로 합성되므로 HUD preview 이후에 제출한다.
		// 앞선 엔진 pass에서 그린 보스 bar sprite와 이 ImGui 제출 순서를 구분한다.
		if (!CUIInputRouter::Get().Is_CinematicSuppressed())
		{
			if (ETOUI(LEVEL::BERN) == CGameInstance::Get().Get_CurrentLevelID())
			{
				if (CLevel_Bern *pBern = CLevel_Bern::Get_Active())
					pBern->Render_ValtanEntryModal();
			}
			else if (CLevel_Development *island = CLevel_Development::Get_Active(LEVEL::MAHARAKA))
				island->Render_TravelModal();
#ifdef _DEBUG
			// 캐릭터 선택의 Debug 입장 preview도 Bern modal과 같은 foreground 제출 순서를 따른다. 실제 입장 요청은 이 표시와 별개다.
			if (ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID())
			{
				if (CLevel_CharacterSelect *pCharacterSelectDebug = CLevel_CharacterSelect::Get_Active())
					pCharacterSelectDebug->Render_RaidEntryDebugPreview();
			}
#endif
			if (nullptr != m_pChatWindowView)
			{
				// 채팅을 지원하는 Bern·발탄·쿠크·마하라카·콜로세움에서만 현재 Level의 명령 sink를 전달한다.
				const uint32_t chatLevel = CGameInstance::Get().Get_CurrentLevelID();
				// 수신 채팅은 현재 Level의 replication에서 가져온다. 발신자 자신의 줄도 Server broadcast의 nickname·본문을 그대로 표시한다.
				std::vector<CClientReplication::CHAT_LINE> chatLines;
				if (ETOUI(LEVEL::BERN) == chatLevel)
				{
					CLevel_Bern *pBern = CLevel_Bern::Get_Active();
					if (nullptr != pBern)
						pBern->Drain_ChatLines(chatLines);
					for (const CClientReplication::CHAT_LINE &Line : chatLines)
						m_pChatWindowView->Append_ReceivedLine(Line.strNickname, Line.strText);
					{
						Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.Chat.Update");
						m_pChatWindowView->Render(nullptr != pBern ? pBern->Get_PlayerCommandSink() : nullptr);
					}
				}
				else if (ETOUI(LEVEL::VALTAN_ARENA) == chatLevel)
				{
					CLevel_ValtanArena *pValtanArena = CLevel_ValtanArena::Get_Active();
					if (nullptr != pValtanArena)
						pValtanArena->Drain_ChatLines(chatLines);
					for (const CClientReplication::CHAT_LINE &Line : chatLines)
						m_pChatWindowView->Append_ReceivedLine(Line.strNickname, Line.strText);
					{
						Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.Chat.Update");
						m_pChatWindowView->Render(nullptr != pValtanArena ? pValtanArena->Get_PlayerCommandSink()
																		  : nullptr);
					}
				}
				else if (ETOUI(LEVEL::KOUKUSAYDON_ARENA) == chatLevel)
				{
					CLevel_KoukuSaydonArena *pKouku = CLevel_KoukuSaydonArena::Get_Active();
					if (nullptr != pKouku)
						pKouku->Drain_ChatLines(chatLines);
					for (const CClientReplication::CHAT_LINE &Line : chatLines)
						m_pChatWindowView->Append_ReceivedLine(Line.strNickname, Line.strText);
					{
						Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.Chat.Update");
						m_pChatWindowView->Render(nullptr != pKouku ? pKouku->Get_PlayerCommandSink() : nullptr);
					}
				}
				else if (ETOUI(LEVEL::MAHARAKA) == chatLevel || ETOUI(LEVEL::COLOSSEUM) == chatLevel)
				{
					CLevel_Development *social = CLevel_Development::Get_Active(static_cast<LEVEL>(chatLevel));
					if (social)
						social->Drain_ChatLines(chatLines);
					for (const auto &line : chatLines)
						m_pChatWindowView->Append_ReceivedLine(line.strNickname, line.strText);
					Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.Chat.Update");
					m_pChatWindowView->Render(social ? social->Get_PlayerCommandSink() : nullptr);
				}
			}
			if (nullptr != m_pPartyWindowView)
			{
				// 각 Level이 소유한 replication의 현재 파티 roster·HP를 view로 전달한다. 창이 별도 파티 상태를 생성하지 않는다.
				const uint32_t partyLevel = CGameInstance::Get().Get_CurrentLevelID();
				if (ETOUI(LEVEL::BERN) == partyLevel)
				{
					if (CLevel_Bern *pBern = CLevel_Bern::Get_Active())
						m_pPartyWindowView->Sync_From_Roster(pBern->Get_PartyRoster(), pBern->Get_PlayerHealth());
					{
						Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
														 "UI.Runtime.Party.Update");
						m_pPartyWindowView->Render();
					}
				}
				else if (ETOUI(LEVEL::VALTAN_ARENA) == partyLevel)
				{
					if (CLevel_ValtanArena *pValtanArena = CLevel_ValtanArena::Get_Active())
						m_pPartyWindowView->Sync_From_Roster(pValtanArena->Get_PartyRoster(),
															 pValtanArena->Get_PlayerHealth());
					{
						Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
														 "UI.Runtime.Party.Update");
						m_pPartyWindowView->Render();
					}
				}
				else if (ETOUI(LEVEL::KOUKUSAYDON_ARENA) == partyLevel)
				{
					if (CLevel_KoukuSaydonArena *pKoukuArena = CLevel_KoukuSaydonArena::Get_Active())
						m_pPartyWindowView->Sync_From_Roster(pKoukuArena->Get_PartyRoster(),
															 pKoukuArena->Get_PlayerHealth());
					{
						Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
														 "UI.Runtime.Party.Update");
						m_pPartyWindowView->Render();
					}
				}
				else if (ETOUI(LEVEL::MAHARAKA) == partyLevel || ETOUI(LEVEL::COLOSSEUM) == partyLevel)
				{
					if (CLevel_Development *social = CLevel_Development::Get_Active(static_cast<LEVEL>(partyLevel)))
						m_pPartyWindowView->Sync_From_Roster(social->Get_PartyRoster(), social->Get_PlayerHealth());
					Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.Party.Update");
					m_pPartyWindowView->Render();
				}
			}
		}
#ifdef _DEBUG
		if (!m_pLevelNavigationDebug)
			m_pLevelNavigationDebug = std::make_unique<CLevelNavigationDebug>();
		// Development 편집 workspace에서는 MapTool이 선택 Area를 소유하므로 Level 기반 navigation 동기화를 비활성 값으로 보낸다.
		m_pLevelNavigationDebug->Sync_Level(CMapEditorWorkspaceService::Is_Active()
												? LEVEL::END
												: static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID()));
		m_pLevelNavigationDebug->Render_Overlay();
		if (m_pKoukuPresentationPlayer)
			m_pKoukuPresentationPlayer->Render_Debug();
#endif
		if (nullptr != m_pRenderingBenchmark)
			m_pRenderingBenchmark->Update(CGameInstance::Get().Get_Profiler());
		if (m_bDeveloperToolsVisible)
		{
			Engine::CProfilerScope developerToolsScope(CGameInstance::Get().Get_Profiler(), "ImGui.DeveloperTools");
			RenderDeveloperTools();
			// 예약된 도구 하나에만 다음 창 focus를 요청하고 즉시 예약을 소비한다.
			const auto focusNextWindow = [this](const DEBUG_TOOL eTool)
			{
				if (m_eDebugWindowFocusPending != eTool)
					return;
				ImGui::SetNextWindowFocus();
				m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
			};
#ifdef _DEBUG
			RenderWorldLevelTool();
			RenderWorldSceneTool();
			if (m_pMaharakaAITool && IsDebugToolVisible(DEBUG_TOOL::MAHARAKA_AI))
			{
				focusNextWindow(DEBUG_TOOL::MAHARAKA_AI);
				m_pMaharakaAITool->Render();
				if (m_pMaharakaAITool->Consume_InteractionRequest())
					m_eDebugInputOwner = DEBUG_TOOL::MAHARAKA_AI;
				if (!m_pMaharakaAITool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::MAHARAKA_AI, false);
			}
			if (m_pGuideAITool)
			{
				if (IsDebugToolVisible(DEBUG_TOOL::GUIDE_AI))
				{
					const LostArk::Shared::S2C_GUIDE_STATE *guideState = nullptr;
					const auto guideLevel = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
					if (guideLevel == LEVEL::BERN)
					{
						if (auto *level = CLevel_Bern::Get_Active())
							guideState = level->Get_GuideState();
					}
					else if (guideLevel == LEVEL::VALTAN_ARENA)
					{
						if (auto *level = CLevel_ValtanArena::Get_Active())
							guideState = level->Get_GuideState();
					}
					else if (guideLevel == LEVEL::KOUKUSAYDON_ARENA)
					{
						if (auto *level = CLevel_KoukuSaydonArena::Get_Active())
							guideState = level->Get_GuideState();
					}
					m_pGuideAITool->Set_RuntimeState(guideState);
					m_pGuideAITool->Set_ActiveArea(GetWorldLevelAreaId());
					focusNextWindow(DEBUG_TOOL::GUIDE_AI);
					m_pGuideAITool->Render();
					if (m_pGuideAITool->Consume_InteractionRequest())
						m_eDebugInputOwner = DEBUG_TOOL::GUIDE_AI;
					if (m_pGuideAITool->Consume_PlacementPickRequest())
					{
						m_bWorldMeshPickArmed = false;
						if (auto *bern = CLevel_Bern::Get_Active())
							bern->Get_PlayerController().Cancel_DebugPlayerPlacement();
						if (auto *arena = CLevel_ValtanArena::Get_Active())
							arena->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
						if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
							arena->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
						if (auto *select = CLevel_CharacterSelect::Get_Active())
							select->Get_DebugPlayerController().Cancel_DebugPlayerPlacement();
						m_eWorldLevelPickOwner = DEBUG_TOOL::GUIDE_AI;
						m_eDebugInputOwner = DEBUG_TOOL::GUIDE_AI;
						m_bWorldLevelPickArmed = true;
						m_iWorldLevelPickLevel = CGameInstance::Get().Get_CurrentLevelID();
						m_bWorldLevelPickLeftDown = true;
					}
					if (!m_pGuideAITool->Is_Open())
						SetDebugToolVisible(DEBUG_TOOL::GUIDE_AI, false);
				}
			}
			// Workbench shell은 선택한 boss session 하나를 표시한다. 다른 도구 창과 각각의 초안 소유권은 독립적으로 유지한다.
			if (IsDebugToolVisible(DEBUG_TOOL::MAP) && nullptr != m_pMapTool)
			{
				focusNextWindow(DEBUG_TOOL::MAP);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Map.Build");
					m_pMapTool->Render();
				}
			}
			// 통합 Save 버튼에 초안 변경 상태를 전달하고 요청된 저작 저장을 수행한다. 이 경로는 publisher를 실행하지 않는다.
			if (nullptr != m_pMapTool && m_pMapTool->Is_IntegratedCutsceneViewOpen())
			{
				// 이 통합 저장은 현재 host가 쿠크 composition일 때만 소유한다. 발탄은 자체 분할 저작 저장 경로를 사용한다.
				const bool_t koukuHosted =
					m_pMapTool->Get_HostedCompositionSession() ==
					static_cast<ICompositionWorkbenchSession *>(m_pSequenceActionWorkbench.get());
				const bool_t sequenceDirty =
					koukuHosted && m_pSequenceActionWorkbench && m_pSequenceActionWorkbench->Is_Dirty();
				// WorldObjectTool의 Area는 쿠크로 고정되어 있다. 발탄을 host한 저장에서 쿠크 오브젝트 문서를 함께 쓰지 않게 제한한다.
				const bool_t objectDirty = koukuHosted && m_pWorldObjectTool && m_pWorldObjectTool->Is_Dirty();
				std::string integratedStatus;
				if (m_pMapTool->Consume_IntegratedSaveRequest())
				{
					// 각 초안을 순서대로 저장한다. 뒤 저장이 실패해도 앞에서 성공한 원본 저장을 이 함수가 되돌리지는 않는다.
					bool_t saved = true;
					if (sequenceDirty && m_pSequenceActionWorkbench)
						saved = m_pSequenceActionWorkbench->Save(integratedStatus) && saved;
					if (objectDirty && m_pWorldObjectTool && m_pWorldObjectTool->Is_Dirty())
					{
						// false는 저작 원본 저장만 의미한다. Area 게시와 연결된 전투 패턴 게시는 별도 명시 동작으로 남긴다.
						saved = m_pWorldObjectTool->Save_Source(false) && saved;
					}
					if (integratedStatus.empty())
						integratedStatus = saved ? "Saved authoring sources. Runtime data was not published."
												 : "Save failed; the existing authoring edits are preserved.";
				}
				m_pMapTool->Set_IntegratedSaveState(sequenceDirty, objectDirty, std::move(integratedStatus));
			}
			// MapTool을 먼저 그린 뒤 이번 프레임에 이미 연 session을 shell에 알려 중복 창 생성을 막는다.
			if (nullptr != m_pSequencerTool)
			{
				// 고정 session 대신 현재 편집 Area에서 실제 host한 composition·object session을 넘긴다.
				m_pSequencerTool->Suppress_SessionFrameThisFrame(
					nullptr != m_pMapTool ? m_pMapTool->Get_HostedCompositionSession() : nullptr,
					nullptr != m_pMapTool ? m_pMapTool->Get_HostedObjectSession() : nullptr);
			}
			if (IsDebugToolVisible(DEBUG_TOOL::SEQUENCER) && nullptr != m_pSequencerTool)
			{
				focusNextWindow(DEBUG_TOOL::SEQUENCER);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
													 "ImGui.Tool.Composition.Build");
					m_pSequencerTool->Render();
				}
				if (m_pCharacterActionWorkbench && m_pCharacterActionWorkbench->Consume_InteractionRequest() &&
					m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::CHARACTER)
					m_eDebugInputOwner = DEBUG_TOOL::SEQUENCER;
				if (m_pWorldObjectTool && m_pWorldObjectTool->Consume_InteractionRequest() &&
					m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::OBJECT &&
					!m_pSequencerTool->Uses_ValtanSession())
					m_eDebugInputOwner = DEBUG_TOOL::WORLD_OBJECT;
				if (!m_pSequencerTool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::SEQUENCER, false);
			}
			// 두 Workbench의 오브젝트 편집 요청을 실제 WorldObjectTool로 연결한다. 임시 배치는 값으로 변환해 넘기고 결과 상태를 원래 요청자에게 돌려준다.
			for (auto *workbench : {m_pKoukuSaydonActionWorkbench.get(), m_pSequenceActionWorkbench.get()})
			{
				KOUKU_WORLD_OBJECT_EDIT_REQUEST request;
				if (!workbench || !workbench->Consume_WorldObjectEditRequest(request))
					continue;
				std::string status;
				if (FAILED(EnsureDebugTool(DEBUG_TOOL::WORLD_OBJECT)) || !m_pWorldObjectTool)
					status = "Object Tool is unavailable. Existing authoring edits are preserved.";
				else
				{
					std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> placement;
					if (request.PreviewPlacement)
					{
						const auto &value = *request.PreviewPlacement;
						placement = CWorldSequencePlayer::OBJECT_PLACEMENT{
							{float(value.Position[0]), float(value.Position[1]), float(value.Position[2])},
							{float(value.RotationDegrees[0]), float(value.RotationDegrees[1]),
							 float(value.RotationDegrees[2])},
							{float(value.Scale[0]), float(value.Scale[1]), float(value.Scale[2])}};
					}
					(void)m_pWorldObjectTool->Open_ObjectMotion(
						request.strObjectId, request.strMotionInstanceId, status, placement, request.strPatternId,
						request.strOccurrenceId, request.bSequenceWorkspace, request.bExplicitPlacement,
						request.bFocusAnimationClips);
				}
				workbench->Notify_WorldObjectEditResult(std::move(status));
			}
			RenderMapEffectPlacementMarker();
			if (IsDebugToolVisible(DEBUG_TOOL::ANIMATION) && nullptr != m_pAnimationTool)
			{
				focusNextWindow(DEBUG_TOOL::ANIMATION);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Animation.Build");
					m_pAnimationTool->Render();
				}
			}

			if (IsDebugToolVisible(DEBUG_TOOL::EFFECT) && m_pEffectTool)
			{
				focusNextWindow(DEBUG_TOOL::EFFECT);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.EffectV1.Build");
					m_pEffectTool->Render();
				}
				if (m_pEffectTool->Consume_AuthoringInteraction())
					m_eDebugInputOwner = DEBUG_TOOL::EFFECT;
			}
			if (IsDebugToolVisible(DEBUG_TOOL::EFFECT_V2) && m_pEffectToolV2)
			{
				focusNextWindow(DEBUG_TOOL::EFFECT_V2);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.EffectV2.Build");
					m_pEffectToolV2->Render();
				}
				if (m_pEffectToolV2->Consume_AuthoringInteraction())
					m_eDebugInputOwner = DEBUG_TOOL::EFFECT_V2;
			}
			if (IsDebugToolVisible(DEBUG_TOOL::RENDERING))
			{
				focusNextWindow(DEBUG_TOOL::RENDERING);
				RenderRenderingWorkbench();
			}
			// UI 저작 창도 기존 HUDLayoutTool 소유자를 그린다. 제품 sprite의 생성·갱신 경로를 추가하지 않는다.
			if (IsDebugToolVisible(DEBUG_TOOL::UI) && nullptr != m_pHUDLayoutTool)
			{
				focusNextWindow(DEBUG_TOOL::UI);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.HUDLayout.Build");
					m_pHUDLayoutTool->Render();
				}
			}
#else
			if (IsDebugToolVisible(DEBUG_TOOL::RENDERING))
			{
				focusNextWindow(DEBUG_TOOL::RENDERING);
				RenderOptimizationBenchmarkPanel();
			}
#endif
			if (IsDebugToolVisible(DEBUG_TOOL::BALANCE) && nullptr != m_pBalanceTool)
			{
				focusNextWindow(DEBUG_TOOL::BALANCE);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Balance.Build");
					m_pBalanceTool->Render();
				}
			}
			if (IsDebugToolVisible(DEBUG_TOOL::VALTAN_BOSS) && nullptr != m_pValtanBossTool)
			{
				focusNextWindow(DEBUG_TOOL::VALTAN_BOSS);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
													 "ImGui.Tool.ValtanBoss.Build");
					m_pValtanBossTool->Render();
				}
				if (!m_pValtanBossTool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::VALTAN_BOSS, false);
			}
			if (IsDebugToolVisible(DEBUG_TOOL::KOUKU_SAYDON_BOSS) && nullptr != m_pKoukuSaydonBossTool)
			{
				focusNextWindow(DEBUG_TOOL::KOUKU_SAYDON_BOSS);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.KoukuBoss.Build");
					m_pKoukuSaydonBossTool->Render();
				}
				if (!m_pKoukuSaydonBossTool->Is_Open())
					SetDebugToolVisible(DEBUG_TOOL::KOUKU_SAYDON_BOSS, false);
			}
#ifdef _DEBUG
			if (IsDebugToolVisible(DEBUG_TOOL::VALTAN_LOGIC_PATTERN) && nullptr != m_pValtanBossTool)
			{
				focusNextWindow(DEBUG_TOOL::VALTAN_LOGIC_PATTERN);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
													 "ImGui.Tool.ValtanBoss.Render_LogicPatternWindow");
					m_pValtanBossTool->Render_LogicPatternWindow();
				}
				if (!m_pValtanBossTool->Is_LogicPatternOpen())
					SetDebugToolVisible(DEBUG_TOOL::VALTAN_LOGIC_PATTERN, false);
			}
			if (IsDebugToolVisible(DEBUG_TOOL::CAMERA) && nullptr != m_pCameraTool)
			{
				focusNextWindow(DEBUG_TOOL::CAMERA);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Camera.Build");
					m_pCameraTool->Render();
				}
			}
			if (IsDebugToolVisible(DEBUG_TOOL::EQUIPMENT) && nullptr != m_pEquipmentAuthoringTool)
			{
				focusNextWindow(DEBUG_TOOL::EQUIPMENT);
				{
					Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Equipment.Build");
					m_pEquipmentAuthoringTool->Render(DEBUG_TOOL::EQUIPMENT == m_eDebugInputOwner);
				}
			}

			if (m_bProfilerVisible)
			{
				RenderProfilerOverlay();
				RenderProfilerSettings();
			}
#endif
		}
		bool_t profilerWindowVisible = m_bRuntimeProfilerVisible;
		profilerWindowVisible |= m_bDeveloperToolsVisible && IsDebugToolVisible(DEBUG_TOOL::PROFILER);
		if (profilerWindowVisible && DEBUG_TOOL::PROFILER == m_eDebugWindowFocusPending)
		{
			ImGui::SetNextWindowFocus();
			m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
		}
		if (profilerWindowVisible && m_pProfilerTool)
		{
			Engine::CProfilerScope toolScope(CGameInstance::Get().Get_Profiler(),
											 "ImGui.Tool.CompositionProfiler.Build");
			m_pProfilerTool->Render(CGameInstance::Get().Get_Profiler());
			if (!m_pProfilerTool->Is_Open())
			{
				m_bRuntimeProfilerVisible = false;
				SetDebugToolVisible(DEBUG_TOOL::PROFILER, false);
			}
		}
		{
			if (m_bKoukuBingoHammerColliders)
				if (auto *arena = CLevel_KoukuSaydonArena::Get_Active())
					arena->Debug_DrawBingoHammerColliders();
			Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "ImGui.BackendSubmit");
			Engine::CProfilerGpuScope gpuPhaseScope(CGameInstance::Get().Get_Profiler(), "ImGui.BackendSubmit");
			m_pImGuiLayer->EndFrame();
		}
	}
	// 입장 popup의 실시간 보스 모델은 popup sprite 위, 해당 글자를 그리는 텍스트 pass 아래에 제출한다.
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.BossShowcase");
		Engine::CProfilerGpuScope gpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.BossShowcase");
		if (!CUIInputRouter::Get().Is_CinematicSuppressed())
			CRaidBossShowcaseService::Render(m_pDevice, m_pContext);
	}
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.UIText");
		Engine::CProfilerGpuScope gpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Render.UIText");
		if (!Is_RuntimeUIScreenSuppressed())
		{
			// HP·자원·게이지 글자는 ImGui 제출 이후의 별도 pass이므로 제품 HUD와 같은 숨김 조건을 다시 계산한다.
			// Debug 입장 preview의 지역 변수를 이 범위에서 다시 선언하고 Release에서는 false로 둔다.
#ifdef _DEBUG
			const bool_t isCharSelectDebugPreviewOpenForText =
				ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID() &&
				nullptr != CLevel_CharacterSelect::Get_Active() &&
				CLevel_CharacterSelect::Get_Active()->Is_DebugRaidEntryPreviewOpen();
#else
			const bool_t isCharSelectDebugPreviewOpenForText = false;
#endif
			// 캐릭터 생성·제품 presentation은 같은 Level 위의 전체 화면이다. 아래 class HUD가 겹치지 않게 공통 표시 조건에 포함한다.
			const bool_t isCharSelectOverlayOpen =
				isCharSelectDebugPreviewOpenForText ||
				(ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID() &&
				 nullptr != CLevel_CharacterSelect::Get_Active() &&
				 CLevel_CharacterSelect::Get_Active()->Is_ProductPresentationOpen());
			// 미니게임이 전투 HUD를 숨겨도 마감 타이머는 Debug/Release 공통 제품 UI로 표시한다.
			if (!isCharSelectOverlayOpen && !Is_MvpResultPageOpen() && m_pDungeonTimerView)
			{
				CUITextLayerScope TimerText(UI_TEXT_LAYER::HUD);
				m_pDungeonTimerView->Render();
			}
			if (!isCharSelectOverlayOpen && !Is_MvpResultPageOpen() && !Is_KoukuMinigameHUDHidden())
			{
				CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
				RenderCombatHUDText();
				RenderBossHealthBarText();

				RenderChargeGaugeText();
				RenderSkillCooldownText();
				// 에스더 gauge 글자도 같은 전투 HUD 표시 조건으로 묶는다. 실제 raid 지원 여부는 RenderEstherGaugeText가 검사한다.
				RenderEstherGaugeText();
				// 퀵슬롯 키 글자도 HUD의 일부다. popup 위에 남지 않도록 같은 숨김 조건 안에서 그린다.
				RenderQuickSlotKeyLabels();
			}
			{
				CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
				RenderDeadSceneText();
				RenderRaidClearText();
				RenderItemAnnounceText();
			}
			RenderDamageNumbers();
			if (m_pWorldHealthBarView)
				m_pWorldHealthBarView->Render_Text();
			{
				CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
				if (nullptr != m_pCombatAnalysisView)
					m_pCombatAnalysisView->Render_Text();
			}
			{
				CUITextLayerScope WindowText(UI_TEXT_LAYER::WINDOW_INVENTORY);
				if (nullptr != m_pInventoryView)
					m_pInventoryView->Render_Text();
			}
			{
				CUITextLayerScope WindowText(UI_TEXT_LAYER::WINDOW_REPAIR);
				if (nullptr != m_pRepairWindowView)
					m_pRepairWindowView->Render_Text();
			}
			{
				CUITextLayerScope WindowText(UI_TEXT_LAYER::WINDOW_SHOP);
				if (nullptr != m_pShopWindowView)
					m_pShopWindowView->Render_Text();
			}
			{
				CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
				RenderLobbyButtonText();
				RenderCharacterSelectWindowText();
				if (!Is_MvpResultPageOpen())
					RenderMinimapText();
			}
			{
				CUITextLayerScope WindowText(UI_TEXT_LAYER::WINDOW_ITEM_UPGRADE);
				RenderItemUpgradeButtonText();
				RenderItemUpgradeLevelText();
				RenderItemUpgradeMaterialCounts();
				RenderItemUpgradeGaugePercentText();
				RenderItemUpgradeResultWaitText();
				RenderItemUpgradeSuccessDetailText();
				RenderItemUpgradeFailDetailText();
				RenderItemUpgradeListText();
			}
			if (ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID())
			{
				if (CLevel_CharacterSelect *pCharacterSelect = CLevel_CharacterSelect::Get_Active())
				{
					// 아레나 spawn 버튼의 글자는 실제 버튼 이미지와 같은 overlay 조건을 사용한다.
					// 이 함수에서는 이미지와 분리된 텍스트 pass로 제출한다.
					if (!isCharSelectOverlayOpen)
					{
						CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
						pCharacterSelect->Render_ArenaSpawnLabels();
					}
					// 직업 미리보기 단계의 버튼·체험 문구는 위 spawn 버튼 숨김 조건 밖에서 표시 여부를 판단한다.
					{
						CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
						pCharacterSelect->Render_BrowseLabels();
					}
					// nickname 입력은 생성 화면에서 열리는 modal이므로 spawn 버튼 조건으로 함께 숨기지 않는다.
					CUITextLayerScope ModalText(UI_TEXT_LAYER::MODAL);
					pCharacterSelect->Render_CreateCharacterModalText();
					pCharacterSelect->Render_CustomizingText();
#ifdef _DEBUG
					pCharacterSelect->Render_RaidEntryDebugPreviewText();
#endif
				}
			}
			if (ETOUI(LEVEL::BERN) == CGameInstance::Get().Get_CurrentLevelID())
			{
				if (CLevel_Bern *pBern = CLevel_Bern::Get_Active())
				{
					{
						CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
						pBern->Render_SystemMenuText();
					}
					CUITextLayerScope ModalText(UI_TEXT_LAYER::MODAL);
					pBern->Render_ValtanEntryModalText();
					pBern->Render_PartyInviteText();
				}
			}
			else if (ETOUI(LEVEL::VALTAN_ARENA) == CGameInstance::Get().Get_CurrentLevelID())
			{
				if (CLevel_ValtanArena *pValtanArena = CLevel_ValtanArena::Get_Active())
				{
					CUITextLayerScope ModalText(UI_TEXT_LAYER::MODAL);
					pValtanArena->Render_PartyInviteText();
				}
			}
			else if (ETOUI(LEVEL::KOUKUSAYDON_ARENA) == CGameInstance::Get().Get_CurrentLevelID())
			{
				if (CLevel_KoukuSaydonArena *kouku = CLevel_KoukuSaydonArena::Get_Active())
				{
					CUITextLayerScope modal(UI_TEXT_LAYER::MODAL);
					kouku->Render_TransferFailureNotice();
				}
			}
			if (const LEVEL socialLevel = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
				socialLevel == LEVEL::MAHARAKA || socialLevel == LEVEL::COLOSSEUM)
			{
				if (CLevel_Development *social = CLevel_Development::Get_Active(socialLevel))
				{
					CUITextLayerScope modal(UI_TEXT_LAYER::MODAL);
					social->Render_PartyInviteText();
					social->Render_TravelModalText();
				}
			}
			CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
			// 채팅 sprite를 그리는 Level과 글자의 Level 조건을 맞춘다. 이전 방의 문구가 Loading이나 다른 Level 위에 남지 않게 한다.
			{
				const uint32_t chatTextLevel = CGameInstance::Get().Get_CurrentLevelID();
				const bool_t bChatTextLevel =
					ETOUI(LEVEL::BERN) == chatTextLevel || ETOUI(LEVEL::VALTAN_ARENA) == chatTextLevel ||
					ETOUI(LEVEL::KOUKUSAYDON_ARENA) == chatTextLevel || ETOUI(LEVEL::MAHARAKA) == chatTextLevel ||
					ETOUI(LEVEL::COLOSSEUM) == chatTextLevel;
				if (nullptr != m_pChatWindowView && bChatTextLevel)
					m_pChatWindowView->RenderText();
			}
			// 파티 글자도 해당 sprite를 그리는 같은 Level에서만 제출한다. 마지막 방의 이름이 전환 화면에 남지 않게 한다.
			{
				const uint32_t partyTextLevel = CGameInstance::Get().Get_CurrentLevelID();
				const bool_t bPartyTextLevel =
					ETOUI(LEVEL::BERN) == partyTextLevel || ETOUI(LEVEL::VALTAN_ARENA) == partyTextLevel ||
					ETOUI(LEVEL::KOUKUSAYDON_ARENA) == partyTextLevel || ETOUI(LEVEL::MAHARAKA) == partyTextLevel ||
					ETOUI(LEVEL::COLOSSEUM) == partyTextLevel;
				if (nullptr != m_pPartyWindowView && bPartyTextLevel)
					m_pPartyWindowView->RenderText();
			}

			// 제품 창 글자는 Register_UITextOccluders가 사용한 각 창 layer로 그려 위쪽 창이 덮은 영역에서 숨긴다.
			// Loading에는 sprite가 숨겨지므로 대응하는 창 글자도 제출하지 않는다.
			if (ETOUI(LEVEL::LOADING) != CGameInstance::Get().Get_CurrentLevelID())
			{
				CUITextOcclusion &Occlusion = CUITextOcclusion::Get();
				Occlusion.Apply(UI_TEXT_LAYER::WINDOW_CHARACTER_INFO);
				if (nullptr != m_pCharacterInfoView)
					m_pCharacterInfoView->Render_Text();
				Occlusion.Apply(UI_TEXT_LAYER::WINDOW_AVATAR_BOOK);
				if (nullptr != m_pAvatarBookView)
					m_pAvatarBookView->Render_Text();
				Occlusion.Apply(UI_TEXT_LAYER::WINDOW_VEHICLE);
				if (nullptr != m_pVehicleWindowView)
					m_pVehicleWindowView->Render_Text();
				Occlusion.Apply(UI_TEXT_LAYER::WINDOW_HONOR_TITLE);
				if (nullptr != m_pHonorTitleWindowView)
					m_pHonorTitleWindowView->Render_Text();
				Occlusion.Apply(UI_TEXT_LAYER::WINDOW_WORLD_MAP);
				if (nullptr != m_pWorldMapWindowView)
					m_pWorldMapWindowView->Render_Text();
				Occlusion.Apply(UI_TEXT_LAYER::WINDOW_SYSTEM_OPTION);
				if (nullptr != m_pSystemOptionView)
					m_pSystemOptionView->Render_Text();
				Occlusion.Apply(UI_TEXT_LAYER::HUD);
				if (nullptr != m_pSongCastGaugeView)
					m_pSongCastGaugeView->Render_Text();
			}
		}
		// 일반 전투 UI를 숨긴 시네마틱 중에도 자막은 별도 경로로 표시한다.
		RenderCinematicSubtitles();
		{
			CUITextLayerScope TopText(UI_TEXT_LAYER::PAGE);
			if (!CUIInputRouter::Get().Is_CinematicSuppressed() ||
                (m_pProfilerTool && m_pProfilerTool->Is_CutsceneRecording()))
				RenderFpsText();
		}
		// 시네마틱으로 피해 표시가 숨겨져도 이벤트 cursor는 진행시켜 나중에 이전 피격을 다시 재생하지 않는다.
		if (CUIInputRouter::Get().Is_CinematicSuppressed())
			RenderDamageNumbers();
		// 정상 렌더 경로에서 Update와 Render의 클릭 판정을 모두 마친 뒤 버튼 edge와 마우스 점유를 다음 프레임으로 넘긴다.
		// 위의 begin/world 실패 조기 반환은 이 지점에 도달하지 않는다.
		CUIInputRouter::Get().End_Frame();
	}
	return CGameInstance::Get().Render_End();
}

namespace
{
// 마하라카에서는 Server가 물총을 장착한 유효 플레이어가 도보 상태일 때 전용 pickup HUD를 표시한다.
bool_t Is_WaterGunHudLevel(const uint32_t iLevel, const HUD_PLAYER_STATE &player)
{
	return ETOUI(LEVEL::MAHARAKA) == iLevel && player.isValid && player.isWaterpangArmed && 0u == player.iVehicleId;
}
} // 이 파일의 보조 구현 범위 끝

// HUD 비지원 Level·overlay 진입이 m_pHUDRuntimeView의 모든 슬롯을 숨긴다. view나 저작 문서의 소유권은 유지한다.
void CMainApp::Hide_CombatHUD()
{
	if (nullptr != m_pHUDRuntimeView)
		m_pHUDRuntimeView->Set_AllSlotsVisible(false);
}
// Update가 복제 HUD 상태로 현재 class·자원·장착 표시를 갱신한다. 지원하지 않거나 overlay가 대신 표시되는 화면에서는 STATIC 슬롯도 명시적으로 숨긴다.
// 장비·구매·수리 요청은 기존 Server 명령 경로로 보내며 이 함수가 권위 인벤토리 값을 직접 확정하지 않는다.
void CMainApp::Update_CombatHUD(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.CombatHUD.Update");
	if (nullptr == m_pHUDRuntimeView)
		return;

	if (Is_KoukuMinigameHUDHidden())
	{
		Hide_CombatHUD();
		if (m_pCombatAnalysisView)
			m_pCombatAnalysisView->Hide();
		if (m_pDurabilityHudView)
			m_pDurabilityHudView->Hide();
		if (m_pQuickSlotDragView)
		{
			m_pQuickSlotDragView->Cancel();
			m_pQuickSlotDragView->Hide();
		}
		return;
	}

	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isSupportedLevel =
		currentLevel == ETOUI(LEVEL::BERN) || currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
		currentLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA) || currentLevel == ETOUI(LEVEL::DEVELOPMENT) ||
		currentLevel == ETOUI(LEVEL::CHARACTER_SELECT) || currentLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA) ||
		currentLevel == ETOUI(LEVEL::COLOSSEUM) ||
		Is_WaterGunHudLevel(currentLevel, CCombatHUDViewModel::Get().Get_Player());
	// 스킬 창과 캐릭터 선택의 Debug 입장 preview가 HUD 영역을 대신 사용하므로 동일한 숨김 조건을 적용한다.
	const bool_t skillWindowOpen = nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
#ifdef _DEBUG
	const bool_t isCharSelectDebugPreviewOpen = ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel &&
												nullptr != CLevel_CharacterSelect::Get_Active() &&
												CLevel_CharacterSelect::Get_Active()->Is_DebugRaidEntryPreviewOpen();
#else
	const bool_t isCharSelectDebugPreviewOpen = false;
#endif
	// 같은 Level 위에서 캐릭터 생성·제품 presentation이 열리면 아래 class HUD를 숨긴다.
	const bool_t isCharSelectOverlayOpen =
		isCharSelectDebugPreviewOpen ||
		(ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel && nullptr != CLevel_CharacterSelect::Get_Active() &&
		 CLevel_CharacterSelect::Get_Active()->Is_ProductPresentationOpen());

	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	if (!isSupportedLevel || skillWindowOpen || isCharSelectOverlayOpen || Is_MvpResultPageOpen() || !player.isValid ||
		0u == player.iMaximumHp || 0u == player.iMaximumResource)
	{
		Hide_CombatHUD();
		if (nullptr != m_pCombatAnalysisView)
			m_pCombatAnalysisView->Hide();
		// 인벤토리 등 제품 창도 STATIC sprite를 사용해 Level 전환 뒤 상태가 남는다.
		// 지원하지 않는 화면에서는 호출을 생략하는 대신 명시적으로 숨긴다.
		if (nullptr != m_pInventoryView)
			m_pInventoryView->Hide();
		if (nullptr != m_pRepairWindowView)
			m_pRepairWindowView->Hide();
		if (nullptr != m_pShopWindowView)
			m_pShopWindowView->Hide();
		if (nullptr != m_pDurabilityHudView)
			m_pDurabilityHudView->Hide();
		if (nullptr != m_pCharacterInfoView)
			m_pCharacterInfoView->Hide();
		if (nullptr != m_pAvatarBookView)
			m_pAvatarBookView->Hide();
		if (nullptr != m_pVehicleWindowView)
			m_pVehicleWindowView->Hide();
		if (nullptr != m_pHonorTitleWindowView)
			m_pHonorTitleWindowView->Hide();
		if (nullptr != m_pQuickSlotDragView)
		{
			m_pQuickSlotDragView->Cancel();
			m_pQuickSlotDragView->Hide();
		}
		return;
	}

	{
		// 공통 슬롯을 켠 뒤 현재 class의 ownerClass를 선택한다.
		// 아래 동적 갱신이 같은 프레임에 gauge·icon·cooldown의 실제 표시 상태를 덮어쓴다.
		const string strOwnerClass = GetHUDOwnerClassName(player.eCharacterClass);
		m_pHUDRuntimeView->Set_AllSlotsVisible(true);
		m_pHUDRuntimeView->Set_ActiveOwnerClass(strOwnerClass);

		// 창술사 stance 아이콘은 label로 시작하는 keyframe animation이다.
		// 같은 stance를 매 프레임 재생해 시작 시각을 계속 초기화하지 않도록 stance 변경 때만 호출한다.
		if (LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER == player.eCharacterClass &&
			player.eStance != m_ePreviousHudStance)
		{
			// 기존 추출 기록의 label 대응에 따라 짧은 창 stance는 wild, 반대 stance는 focus를 선택한다.
			const char_t *pLabel =
				LostArk::Shared::PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR == player.eStance ? "wild" : "focus";
			m_pHUDRuntimeView->Play_KeyframeAnimation("Lance_Id_Stance", pLabel);
		}
		// 워로드의 방어 stance 변경은 방어 body·effect의 on/off label로 표시한다.
		// protectMode를 위한 별도 활성 상태는 이 분기에서 읽지 않으므로 해당 연출을 추측해 시작하지 않는다.
		else if (LostArk::Shared::CHARACTER_CLASS_ID::WARLORD == player.eCharacterClass &&
				 player.eStance != m_ePreviousHudStance)
		{
			const char_t *pLabel = LostArk::Shared::PLAYER_STANCE_ID::WARLORD_DEFENSE == player.eStance ? "on" : "off";
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_DefenseBody", pLabel);
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_DefenseEffect", pLabel);
		}
		m_ePreviousHudStance = player.eStance;

		// 워로드 identity 비율을 0~99 frame label로 환산해 좌우 gauge에 같은 값을 적용한다.
		// 시간에 따른 재생 대신 기존 keyframe label 조회를 그대로 사용한다.
		if (LostArk::Shared::CHARACTER_CLASS_ID::WARLORD == player.eCharacterClass && player.iMaximumIdentity > 0u)
		{
			const int32_t iGaugeFrame =
				std::clamp(static_cast<int32_t>(player.iCurrentIdentity * 99u / player.iMaximumIdentity), 0, 99);
			const string strGaugeLabel = std::to_string(iGaugeFrame);
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_GaugeR", strGaugeLabel);
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_GaugeL", strGaugeLabel);
		}

		// 워로드 Z/X 테두리는 실제 스킬 준비 상태를 사용한다.
		// Z는 방어 stance가 아닐 때만 추가로 켜며 X는 별도 보호 활성 상태 없이 cooldown 준비 여부만 반영한다.
		if (LostArk::Shared::CHARACTER_CLASS_ID::WARLORD == player.eCharacterClass)
		{
			bool_t bZReady = false, bXReady = false;
			const HUD_SKILL_STATE *pZSkill = nullptr;
			const HUD_SKILL_STATE *pXSkill = nullptr;
			for (const HUD_SKILL_STATE &Skill : player.Skills)
			{
				if ("Z" == Skill.strInputSlot)
				{
					bZReady = Skill.Is_Ready(player.iServerTick);
					pZSkill = &Skill;
				}
				else if ("X" == Skill.strInputSlot)
				{
					bXReady = Skill.Is_Ready(player.iServerTick);
					pXSkill = &Skill;
				}
			}
			const bool_t bShowZRing = bZReady && LostArk::Shared::PLAYER_STANCE_ID::WARLORD_DEFENSE != player.eStance;
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_EnableRingZ", bShowZRing ? "on" : "off");
			m_pHUDRuntimeView->Play_KeyframeAnimation("WarL_Id_EnableRingX", bXReady ? "on" : "off");

			// ready/cooldown label로 배지의 밝은 그림과 어두운 그림을 고른다.
			// 배지 교체와 아래 시간에 따른 가림 효과는 별도 슬롯이다.
			m_pHUDRuntimeView->Play_KeyframeAnimation("Skill_Z", bZReady ? "ready" : "cooldown");
			m_pHUDRuntimeView->Play_KeyframeAnimation("Skill_X", bXReady ? "ready" : "cooldown");

			// 전체 cooldown 중 이미 지난 비율을 0~239 frame으로 바꾼다.
			// 사용 직후 덮인 그림이 시간이 지나며 줄어드는 순서이므로 남은 비율을 그대로 쓰지 않는다.
			const auto PlayCooldownWipe = [&](const char *pSlotId, const HUD_SKILL_STATE *pSkill, bool_t bReady)
			{
				if (bReady || nullptr == pSkill || 0u == pSkill->iCooldownDurationTicks)
				{
					m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotId, "ready");
					return;
				}
				const uint32_t remainingTicks =
					pSkill->iCooldownEndTick > player.iServerTick ? pSkill->iCooldownEndTick - player.iServerTick : 0u;
				const f32_t fElapsedFraction = std::clamp(1.f - static_cast<f32_t>(remainingTicks) /
																	static_cast<f32_t>(pSkill->iCooldownDurationTicks),
														  0.f, 1.f);
				const int32_t iWipeFrame = std::clamp(static_cast<int32_t>(fElapsedFraction * 239.f), 0, 239);
				m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotId, std::to_string(iWipeFrame));
			};
			PlayCooldownWipe("WarL_Id_CooldownWipeZ", pZSkill, bZReady);
			PlayCooldownWipe("WarL_Id_CooldownWipeX", pXSkill, bXReady);
		}

		// 차원술사의 현재 identity/최대 identity 비율을 360도 회전으로 표시한다.
		// Server에서 온 gauge를 읽을 뿐 이 함수가 자원을 증가시키거나 순환시키지는 않는다.
		if (LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER == player.eCharacterClass &&
			player.iMaximumIdentity > 0u)
		{
			const f32_t fIdentityFraction =
				static_cast<f32_t>(player.iCurrentIdentity) / static_cast<f32_t>(player.iMaximumIdentity);
			m_pHUDRuntimeView->Set_SlotRotation("Dimen_MinuteHand", fIdentityFraction * 360.f);
		}

		// 차원술사의 여섯 장식은 gameplay gauge와 독립적인 반복 움직임이다.
		// 기존 추출 곡선의 주기·구간값을 GetTickCount64의 ms 위상에 대응하며 전체 회전과 짧은 흔들림을 각각 계산한다.
		if (LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER == player.eCharacterClass)
		{
			const uint64_t ullNowMs = GetTickCount64();

			// gear545는 2000ms 주기의 0→360도 회전이다.
			{
				constexpr uint64_t GEAR545_CYCLE_MS = 2000ull;
				const f32_t fFrac =
					static_cast<f32_t>(ullNowMs % GEAR545_CYCLE_MS) / static_cast<f32_t>(GEAR545_CYCLE_MS);
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_545", fFrac * 360.f);
			}

			// gear523은 3000ms 동안 0→-90도로 진행한다.
			// 주기가 바뀌면 0도로 돌아가며 중간 보간은 선형이다.
			{
				constexpr uint64_t GEAR523_CYCLE_MS = 3000ull;
				const f32_t fFrac =
					static_cast<f32_t>(ullNowMs % GEAR523_CYCLE_MS) / static_cast<f32_t>(GEAR523_CYCLE_MS);
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_523", fFrac * -90.f);
			}

			// gear529는 2000ms 동안 0→-60도로 진행한다.
			// 주기가 바뀌면 다시 0도로 시작한다.
			{
				constexpr uint64_t GEAR529_CYCLE_MS = 2000ull;
				const f32_t fFrac =
					static_cast<f32_t>(ullNowMs % GEAR529_CYCLE_MS) / static_cast<f32_t>(GEAR529_CYCLE_MS);
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_529", fFrac * -60.f);
			}

			// gear515는 주기의 앞부분에서 0도를 유지하다가 뒤쪽 구간에서 회전한다.
			// 위상 0.625~0.8에서 0→60도로 보간하고 이후 60도를 유지한다.
			// 다음 주기 시작에 0도로 돌아가므로 전체 구간을 선형 회전으로 바꾸지 않는다.
			{
				constexpr uint64_t GEAR515_CYCLE_MS = 2000ull;
				const f32_t fFrac =
					static_cast<f32_t>(ullNowMs % GEAR515_CYCLE_MS) / static_cast<f32_t>(GEAR515_CYCLE_MS);
				f32_t fDegrees = 0.f;
				if (fFrac >= 0.8f)
					fDegrees = 60.f;
				else if (fFrac >= 0.625f)
					fDegrees = 60.f * (fFrac - 0.625f) / 0.175f;
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_515", fDegrees);
			}

			// gear519는 주기 2000ms 중 위상 0.80~0.8625에서 0→7.5→0도로 흔들린다.
			// 나머지 구간은 0도를 유지한다.
			{
				constexpr uint64_t GEAR519_CYCLE_MS = 2000ull;
				const f32_t fFrac =
					static_cast<f32_t>(ullNowMs % GEAR519_CYCLE_MS) / static_cast<f32_t>(GEAR519_CYCLE_MS);
				f32_t fDegrees = 0.f;
				if (fFrac >= 0.80f && fFrac < 0.825f)
					fDegrees = 7.5f * (fFrac - 0.80f) / 0.025f;
				else if (fFrac >= 0.825f && fFrac < 0.8625f)
					fDegrees = 7.5f * (1.f - (fFrac - 0.825f) / 0.0375f);
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_519", fDegrees);
			}

			// gear535는 2375ms 주기의 마지막 구간에서 0→-8.984→6.015→0도로 움직인다.
			// 각 구간의 길이가 다르므로 아래의 구간별 선형 보간을 사용한다.
			{
				constexpr uint64_t GEAR535_CYCLE_MS = 2375ull;
				const f32_t fFrac =
					static_cast<f32_t>(ullNowMs % GEAR535_CYCLE_MS) / static_cast<f32_t>(GEAR535_CYCLE_MS);
				f32_t fDegrees = 0.f;
				if (fFrac >= 0.7368f && fFrac < 0.9053f)
					fDegrees = -8.984f * (fFrac - 0.7368f) / (0.9053f - 0.7368f);
				else if (fFrac >= 0.9053f && fFrac < 0.9474f)
					fDegrees = -8.984f + (6.015f - -8.984f) * (fFrac - 0.9053f) / (0.9474f - 0.9053f);
				else if (fFrac >= 0.9474f && fFrac < 0.9895f)
					fDegrees = 6.015f * (1.f - (fFrac - 0.9474f) / (0.9895f - 0.9474f));
				m_pHUDRuntimeView->Set_SlotRotation("Dimen_Gear_535", fDegrees);
			}
		}

		// 도화가 붓의 idle label을 첫 분기 진입 때 한 번 요청한다. 이후 반복 재생은 CUILayoutRuntime의 문서 규칙에 맡긴다.
		// 지역 static 플래그는 재생 호출 성공 여부와 무관하게 true가 되며 view나 class 변경에서 여기서 초기화하지 않는다.
		if (LostArk::Shared::CHARACTER_CLASS_ID::ARTIST == player.eCharacterClass)
		{
			static bool_t bBrushLoopStarted = false;
			if (!bBrushLoopStarted)
			{
				m_pHUDRuntimeView->Play_KeyframeAnimation("Yi_id_brush", "idle");
				bBrushLoopStarted = true;
			}
		}

		// 도화가 identity를 세 구간으로 나눠 완성된 구간은 pop, 아직 도달하지 않은 구간은 empty, 진행 중인 구간은 0~99 label로 표시한다.
		// 완성 edge만 pop을 요청해 매 프레임 재생 시각이 초기화되지 않게 하며 Server의 기존 현재량·최대량만 소비한다.
		if (LostArk::Shared::CHARACTER_CLASS_ID::ARTIST == player.eCharacterClass && player.iMaximumIdentity > 0u)
		{
			const f32_t fFraction =
				static_cast<f32_t>(player.iCurrentIdentity) / static_cast<f32_t>(player.iMaximumIdentity);
			const f32_t fScaled = std::clamp(fFraction, 0.f, 1.f) * 3.f;
			// 완료 구간 수는 0~3이다. 전체 gauge가 가득 차면 세 번째 구간도 완료로 처리해야 하므로 2에서 제한하지 않는다.
			const int32_t iCompletedCount = std::min(3, static_cast<int32_t>(fScaled));
			const f32_t fSegmentFraction = fScaled - static_cast<f32_t>(iCompletedCount);

			const char *pSlotIds[3] = {"Yi_Id_GaugeRing", "Yi_Id_GaugeSlot_2", "Yi_Id_GaugeSlot_3"};
			// 완료 연출 edge를 기억하는 함수 지역 static 상태다. 도화가 분기가 실행되지 않는 동안에도 값이 남는다.
			static bool_t bPopped[3] = {false, false, false};
			for (int32_t i = 0; i < 3; ++i)
			{
				if (i < iCompletedCount)
				{
					if (!bPopped[i])
					{
						m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotIds[i], "pop");
						bPopped[i] = true;
					}
				}
				else if (i > iCompletedCount)
				{
					bPopped[i] = false;
					m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotIds[i], "empty");
				}
				else
				{
					bPopped[i] = false;
					const int32_t iGaugeFrame = std::clamp(static_cast<int32_t>(fSegmentFraction * 99.f), 0, 99);
					m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotIds[i], std::to_string(iGaugeFrame));
				}
			}
		}

		// 도화가 Z/X도 스킬의 실제 준비 여부로 ready/cooldown을 고른다.
		// 가림 효과는 이미 지난 cooldown 비율을 0~237 frame으로 환산하는 별도 슬롯에 적용한다.
		if (LostArk::Shared::CHARACTER_CLASS_ID::ARTIST == player.eCharacterClass)
		{
			bool_t bZReady = false, bXReady = false;
			const HUD_SKILL_STATE *pZSkill = nullptr;
			const HUD_SKILL_STATE *pXSkill = nullptr;
			for (const HUD_SKILL_STATE &Skill : player.Skills)
			{
				if ("Z" == Skill.strInputSlot)
				{
					bZReady = Skill.Is_Ready(player.iServerTick);
					pZSkill = &Skill;
				}
				else if ("X" == Skill.strInputSlot)
				{
					bXReady = Skill.Is_Ready(player.iServerTick);
					pXSkill = &Skill;
				}
			}
			m_pHUDRuntimeView->Play_KeyframeAnimation("Yin_Skill_Z", bZReady ? "ready" : "cooldown");
			m_pHUDRuntimeView->Play_KeyframeAnimation("Yin_Skill_X", bXReady ? "ready" : "cooldown");

			const auto PlayZXWipe = [&](const char *pSlotId, const HUD_SKILL_STATE *pSkill, bool_t bReady)
			{
				if (bReady || nullptr == pSkill || 0u == pSkill->iCooldownDurationTicks)
				{
					m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotId, "ready");
					return;
				}
				const uint32_t remainingTicks =
					pSkill->iCooldownEndTick > player.iServerTick ? pSkill->iCooldownEndTick - player.iServerTick : 0u;
				const f32_t fElapsedFraction = std::clamp(1.f - static_cast<f32_t>(remainingTicks) /
																	static_cast<f32_t>(pSkill->iCooldownDurationTicks),
														  0.f, 1.f);
				const int32_t iWipeFrame = std::clamp(static_cast<int32_t>(fElapsedFraction * 237.f), 0, 237);
				m_pHUDRuntimeView->Play_KeyframeAnimation(pSlotId, std::to_string(iWipeFrame));
			};
			PlayZXWipe("Yin_Skill_Z_Wipe", pZSkill, bZReady);
			PlayZXWipe("Yin_Skill_X_Wipe", pXSkill, bXReady);
		}
		Update_PlayerHealthManaBar();
	}

	// 창술사의 세 구간 채움과 완료 연출은 기존 전용 갱신 함수가 처리한다. 화면 배치는 HUD layout의 저작 rect를 따른다.
	if (LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER == player.eCharacterClass)
		Update_LanceMasterIdentityGauge();

	// ownerClass가 class별 표시 범위를 고른 뒤 가디언 나이트의 구슬·stance·socket 동적 값을 갱신한다.
	if (LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT == player.eCharacterClass)
		Update_GuardianKnightIdentity();

	Update_ChargeGauge();
	Update_SkillIcons();
	Update_SkillSlotMarks();
	Update_SkillCooldowns();
	Update_QuickSlotFlash();
	m_HudTimedTexts.clear();
	Update_ItemQuickSlots();
	Update_SpecialQuickSlots();
	// 전투 분석 view는 발탄·쿠크 raid에서만 갱신하고 다른 지원 Level에서는 명시적으로 숨긴다.
	if (nullptr != m_pCombatAnalysisView)
	{
		if (ETOUI(LEVEL::VALTAN_ARENA) == currentLevel || ETOUI(LEVEL::KOUKUSAYDON_ARENA) == currentLevel)
			m_pCombatAnalysisView->Update(fTimeDelta, player);
		else
			m_pCombatAnalysisView->Hide();
	}
	Update_KoukuHudMode();
	Update_VehicleHud();
	Update_WaterGunHud();
	Update_SpecialSlot();
	Update_BuffBar();
	if (nullptr != m_pRepairWindowView)
	{
		m_pRepairWindowView->Update();
		bool_t bRepairAllSlots = false;
		if (m_pRepairWindowView->Try_Consume_RepairRequest(bRepairAllSlots))
		{
			// 수리는 Server에 요청하며 실제 복구된 내구도는 돌아오는 인벤토리 snapshot으로 표시된다.
			Play_UISound(UI_SOUND_CUE::BUTTON_CLICK);
			(void)CNetworkManager::Get().Send_RepairEquipment(m_iNextUseItemSequence++, bRepairAllSlots);
		}
	}
	if (nullptr != m_pShopWindowView)
	{
		m_pShopWindowView->Update();
		// 구매 가격·잔액·아이템 적용은 Server가 판정한다. UI는 NPC ID와 장바구니 값만 요청한다.
		string strShopNpcId;
		vector<LostArk::Shared::SHOP_BASKET_ENTRY> BasketEntries;
		if (m_pShopWindowView->Try_Consume_BuyRequest(strShopNpcId, BasketEntries))
			(void)CNetworkManager::Get().Send_BuyItems(m_iNextUseItemSequence++, strShopNpcId, BasketEntries);
	}
	// 내구도 view는 유효 플레이어·지원 Level 검사 뒤 실제 장착 아이템의 snapshot을 읽는다.
	// 아래에서 손상 부품이 하나도 없으면 silhouette까지 숨긴다.
	if (nullptr != m_pDurabilityHudView)
	{
		// 내구도 표시는 Bern·발탄·쿠크에서만 켜며 다른 Level에서는 숨긴다.
		if (currentLevel == ETOUI(LEVEL::BERN) || currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
			currentLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA))
		{
			// 장착 슬롯의 내구도가 0이면 파괴, 30% 미만이면 손상이다. 빈 슬롯이나 정상 장비는 NORMAL이며 손상 부품이 있을 때만 전체 view를 갱신한다.
			using LostArk::Shared::EQUIPMENT_SLOT;
			struct HUD_PART_SLOT
			{
				CDurabilityHudView::PART ePart;
				EQUIPMENT_SLOT eSlot;
			};
			constexpr HUD_PART_SLOT PART_SLOTS[] = {
				{CDurabilityHudView::PART::WEAPON, EQUIPMENT_SLOT::WEAPON},
				{CDurabilityHudView::PART::HELMET, EQUIPMENT_SLOT::HELMET},
				{CDurabilityHudView::PART::TOP, EQUIPMENT_SLOT::TOP},
				{CDurabilityHudView::PART::GLOVES, EQUIPMENT_SLOT::GLOVES},
				{CDurabilityHudView::PART::BOTTOMS, EQUIPMENT_SLOT::PANTS},
				{CDurabilityHudView::PART::SHOULDER, EQUIPMENT_SLOT::SHOULDER},
			};
			const auto &InventoryItems = CCombatHUDViewModel::Get().Get_Inventory().Items;
			bool_t bAnyRed = false;
			for (const HUD_PART_SLOT &PartSlot : PART_SLOTS)
			{
				CDurabilityHudView::PART_STATE eState = CDurabilityHudView::PART_STATE::NORMAL;
				for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT &Item : InventoryItems)
				{
					if (Item.eEquippedSlot != PartSlot.eSlot)
						continue;
					if (0u == Item.iDurabilityPercent)
						eState = CDurabilityHudView::PART_STATE::DESTROYED;
					else if (Item.iDurabilityPercent < 30u)
						eState = CDurabilityHudView::PART_STATE::DAMAGED;
					break;
				}
				m_pDurabilityHudView->Set_PartState(PartSlot.ePart, eState);
				bAnyRed = bAnyRed || CDurabilityHudView::PART_STATE::NORMAL != eState;
			}
			if (bAnyRed)
				m_pDurabilityHudView->Update();
			else
				m_pDurabilityHudView->Hide();
		}
		else
			m_pDurabilityHudView->Hide();
	}
	if (nullptr != m_pInventoryView)
	{
		m_pInventoryView->Update(CCombatHUDViewModel::Get().Get_Inventory().Items);
		// 장비 종류에 맞는 슬롯 중 첫 빈자리를 우선하고 모두 사용 중이면 첫 슬롯을 고른다.
		// 실제 종류·class·장착 가능 여부는 Server가 검증한다.
		string strEquipItemId;
		bool_t bEquipRequested = m_pInventoryView->Try_Consume_EquipRequest(strEquipItemId);
		if (!bEquipRequested && nullptr != m_pAvatarBookView)
			bEquipRequested = m_pAvatarBookView->Try_Consume_EquipRequest(strEquipItemId);
		if (bEquipRequested)
		{
			using LostArk::Shared::EQUIPMENT_SLOT;
			const ITEM_DEFINITION *pEquip = CItemCatalog::Find_ById(strEquipItemId);
			EQUIPMENT_SLOT eTarget = EQUIPMENT_SLOT::NONE;
			for (uint32_t iSlot = ETOUI(EQUIPMENT_SLOT::HELMET);
				 nullptr != pEquip && iSlot < ETOUI(EQUIPMENT_SLOT::END); ++iSlot)
			{
				const EQUIPMENT_SLOT eSlot = static_cast<EQUIPMENT_SLOT>(iSlot);
				const char *pKind = LostArk::Shared::Equipment_SlotKind(eSlot);
				if (nullptr == pKind || pEquip->strEquipSlot != pKind)
					continue;
				bool_t bWorn = false;
				for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT &Item :
					 CCombatHUDViewModel::Get().Get_Inventory().Items)
					bWorn = bWorn || Item.eEquippedSlot == eSlot;
				if (EQUIPMENT_SLOT::NONE == eTarget)
					eTarget = eSlot;
				if (!bWorn)
				{
					eTarget = eSlot;
					break;
				}
			}
			if (EQUIPMENT_SLOT::NONE != eTarget)
				(void)CNetworkManager::Get().Send_SetEquipment(m_iNextUseItemSequence++, eTarget, true, strEquipItemId);
		}
		LostArk::Shared::EQUIPMENT_SLOT eAvatarUnequipSlot = LostArk::Shared::EQUIPMENT_SLOT::NONE;
		bool_t bUnequipRequested = m_pInventoryView->Try_Consume_UnequipRequest(eAvatarUnequipSlot);
		if (!bUnequipRequested && nullptr != m_pAvatarBookView)
			bUnequipRequested = m_pAvatarBookView->Try_Consume_UnequipRequest(eAvatarUnequipSlot);
		if (bUnequipRequested)
			(void)CNetworkManager::Get().Send_SetEquipment(m_iNextUseItemSequence++, eAvatarUnequipSlot, false, {});
	}
	if (nullptr != m_pCharacterInfoView)
	{
		// 현재 지원 Level에서 실제 캐릭터의 shared_ptr를 잠시 받아 정보·외형 view 갱신 동안 수명을 유지한다.
		shared_ptr<CCharacter> pLocalCharacter;
		if (ETOUI(LEVEL::BERN) == currentLevel)
		{
			if (CLevel_Bern *pBern = CLevel_Bern::Get_Active())
				pLocalCharacter = pBern->Get_LocalCharacter();
		}
		else if (ETOUI(LEVEL::VALTAN_ARENA) == currentLevel)
		{
			if (CLevel_ValtanArena *pValtanArena = CLevel_ValtanArena::Get_Active())
				pLocalCharacter = pValtanArena->Get_LocalCharacter();
		}
		else if (ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel)
		{
			if (CLevel_CharacterSelect *pCharacterSelect = CLevel_CharacterSelect::Get_Active())
				pLocalCharacter = pCharacterSelect->Get_CharacterInfoCharacter();
		}
		else if (ETOUI(LEVEL::KOUKUSAYDON_ARENA) == currentLevel)
		{
			if (CLevel_KoukuSaydonArena *pKoukuArena = CLevel_KoukuSaydonArena::Get_Active())
				pLocalCharacter = pKoukuArena->Get_LocalCharacter();
		}
		m_pCharacterInfoView->Update(fTimeDelta, pLocalCharacter, player);
		LostArk::Shared::EQUIPMENT_SLOT eUnequipSlot = LostArk::Shared::EQUIPMENT_SLOT::NONE;
		if (m_pCharacterInfoView->Take_UnequipRequest(eUnequipSlot))
			(void)CNetworkManager::Get().Send_SetEquipment(m_iNextUseItemSequence++, eUnequipSlot, false, {});
		if (nullptr != m_pAvatarBookView)
		{
			// 아바타 도감은 캐릭터 정보 창의 버튼으로 토글하며 정보 창을 닫으면 함께 닫는다.
			// 도감의 슬롯·아이템 표시 입력은 기존 정보 view에서 가져온다.
			if (m_pCharacterInfoView->Take_AvatarBookRequest())
			{
				if (m_pAvatarBookView->Is_Open())
					m_pAvatarBookView->Close();
				else
					m_pAvatarBookView->Open();
			}
			if (!m_pCharacterInfoView->Is_Open())
				m_pAvatarBookView->Close();
			m_pCharacterInfoView->Set_Covered(m_pAvatarBookView->Is_Open());
			m_pAvatarBookView->Update(fTimeDelta, pLocalCharacter, player, *m_pCharacterInfoView);
		}
		if (nullptr != m_pVehicleWindowView)
		{
			m_pVehicleWindowView->Update(fTimeDelta, pLocalCharacter, player);
			// 탈것 창은 stable vehicle ID를 요청한다. 현재 Level의 PlayerController가 대기 sequence와 Server 응답을 소유한다.
			// 여기서는 controller 부재·요청 거부·전송 실패를 기록하며 실제 탑승 가능 여부는 기존 Server 왕복 경계에서 판정한다.
			uint32_t iVehicleId = 0u;
			if (m_pVehicleWindowView->Take_RidingRequest(iVehicleId))
			{
				CPlayerController *pController = Find_ActivePlayerController();
				if (nullptr == pController || !pController->Request_VehicleRiding(iVehicleId))
					OutputDebugStringA(
						"[Client][VehicleWindow] Riding request not sent (no controller, or one is still pending).\n");
			}
		}
		if (nullptr != m_pHonorTitleWindowView)
		{
			// 칭호 창도 정보 창 버튼으로 토글하고 정보 창을 닫으면 함께 닫는다. Server 왕복은 현재 PlayerController로 위임한다.
			if (m_pCharacterInfoView->Take_HonorTitleWindowRequest())
				m_pHonorTitleWindowView->Toggle();
			if (!m_pCharacterInfoView->Is_Open())
				m_pHonorTitleWindowView->Close();
			// 칭호 창이 열렸으면 아래 정보 창 글자를 가리는 표시 상태를 설정한다.
			if (m_pHonorTitleWindowView->Is_Open())
				m_pCharacterInfoView->Set_Covered(true);
			m_pHonorTitleWindowView->Update(fTimeDelta, player);
			uint32_t iTitleId = 0u;
			if (m_pHonorTitleWindowView->Take_TitleRequest(iTitleId))
			{
				CPlayerController *pController = Find_ActivePlayerController();
				if (nullptr == pController || !pController->Request_HonorTitle(iTitleId))
					OutputDebugStringA(
						"[Client][HonorTitleWindow] Title request not sent (no controller, or one is still pending).\n");
			}
		}
	}
	Update_QuickSlotDrag();

	// 이번 프레임에 선택한 animation label을 sprite에 평가한 뒤 Render할 수 있도록 CUILayoutRuntime 갱신을 마지막에 호출한다.
	m_pHUDRuntimeView->Update(fTimeDelta);
}

// RenderQuickSlotKeyLabels가 선박 HUD일 때 위임한다. 현재 보급량·속도·키 글자를 기존 저작 기준점에 배치하며 view가 없거나 선박 HUD가 아니면 생략한다.
void CMainApp::RenderShipHudTexts()
{
	if (!m_bShipHudActive || nullptr == m_pHUDRuntimeView)
		return;
	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewportSize.x / 1280.f;
	const f32_t fScaleY = vViewportSize.y / 720.f;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);

	// 입력 위치·글자 크기는 HUD 기준 pixel이다. viewport 배율로 변환하고 측정한 글자 크기의 절반을 빼서 중심 정렬한다.
	const auto DrawHudText =
		[&](const wchar_t *pText, const f32_t fHudX, const f32_t fHudY, const f32_t fHudPx, const fvector_t vColor)
	{
		f32_t fScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(TEXT("Font_YG760"), fHudPx * fUiScale, fScale);
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFont, pText);
		const float2_t vPosition(std::round(fHudX * fScaleX - vMeasured.x * fScale * 0.5f),
								 std::round(fHudY * fScaleY - vMeasured.y * fScale * 0.5f));
		CGameInstance::Get().Draw_Text(strFont, pText, float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
									   XMVectorSet(0.f, 0.f, 0.f, 0.75f), 0.f, float2_t(0.f, 0.f), fScale);
		CGameInstance::Get().Draw_Text(strFont, pText, vPosition, vColor, 0.f, float2_t(0.f, 0.f), fScale);
	};
	// 원본 1920×1080 무대 좌표를 HUD 기준 좌표로 2/3 축소해 옮긴다.
	// 무대 기준점(960,974)을 HUD의(673.5,644.702026)에 대응한다.
	constexpr f32_t STAGE_SCALE = 2.f / 3.f;
	const auto DrawStageText = [&](const wchar_t *pText, const f32_t fStageX, const f32_t fStageY, const f32_t fStagePx,
								   const fvector_t vColor)
	{
		DrawHudText(pText, 673.5f + (fStageX - 960.f) * STAGE_SCALE, 644.702026f + (fStageY - 974.f) * STAGE_SCALE,
					fStagePx * STAGE_SCALE, vColor);
	};
	const fvector_t vWhite = XMVectorSet(1.f, 1.f, 1.f, 1.f);
	const fvector_t vSpeed = XMVectorSet(225.f / 255.f, 210.f / 255.f, 157.f / 255.f, 1.f);

	// 선박 보급량은 현재/최대로, 속도는 소수 한 자리 knot 문구로 표시한다.
	if (m_iShipSupplyMax > 0u)
	{
		const wstring strSupply = std::to_wstring(m_iShipSupply) + L"/" + std::to_wstring(m_iShipSupplyMax);
		DrawStageText(strSupply.c_str(), 961.f, 975.f, 17.f, vWhite);
	}
	const int32_t iKnotsTenths = static_cast<int32_t>(std::lround(m_fShipKnots * 10.f));
	const wstring strKnots =
		std::to_wstring(iKnotsTenths / 10) + L"." + std::to_wstring(iKnotsTenths % 10) + L" \uB178\uD2B8";
	DrawStageText(strKnots.c_str(), 960.f, 1051.f, 16.f, vSpeed);

	// 8개 슬롯의 기준 rect를 읽어 아래쪽 tab 중심에 키 문자를 배치한다.
	constexpr const char *SLOT_KEYS[] = {"Q", "W", "E", "R", "A", "S", "D", "F"};
	constexpr const wchar_t *SLOT_LABELS[] = {L"Q", L"W", L"E", L"R", L"A", L"S", L"D", L"F"};
	for (size_t i = 0; i < 8u; ++i)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect(string("Ship_Slot_") + SLOT_KEYS[i] + "_Frame", fX, fY, fWidth, fHeight))
			continue;
		DrawHudText(SLOT_LABELS[i], fX + fWidth * 0.5f, fY + fHeight * 0.83f, 13.f * STAGE_SCALE, vWhite);
	}

	// 선박 버튼의 키 문자는 기존 원본 무대 기준 좌표에 놓고 같은 좌표 변환을 사용한다.
	DrawStageText(L"T", 1109.f, 995.f, 13.f, vWhite);
	DrawStageText(L"Z", 1064.f, 1053.f, 13.f, vWhite);
	DrawStageText(L"SPACE", 1141.f, 1053.f, 13.f, vWhite);
	DrawStageText(L"C", 1224.f, 1053.f, 13.f, vWhite);
	DrawStageText(L"M", 1316.f, 1043.f, 13.f, vWhite);
	DrawStageText(L"\uC790\uB3D9 \uD56D\uB85C", 1316.f, 1057.f, 13.f, vWhite);
}

// 제품 텍스트 pass가 유효 플레이어의 실제 HUD 모드에 맞는 키 글자를 그린다.
// 기준 rect가 없는 슬롯은 해당 문자만 건너뛰며 입력이나 스킬 준비 상태를 변경하지 않는다.
void CMainApp::RenderQuickSlotKeyLabels()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::COLOSSEUM) && currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) && currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) && currentLevel != ETOUI(LEVEL::KOUKUSAYDON_ARENA) &&
		!Is_WaterGunHudLevel(currentLevel, CCombatHUDViewModel::Get().Get_Player()))
	{
		return;
	}
	const HUD_PLAYER_STATE &keyLabelPlayer = CCombatHUDViewModel::Get().Get_Player();
	if (!keyLabelPlayer.isValid)
		return;
	if (nullptr == m_pHUDRuntimeView)
		return;

	struct KEY_LABEL
	{
		const char *pSlotId;
		const wchar_t *pLabel;
	};
	constexpr KEY_LABEL LABELS[] = {
		{"Skill_Q", L"Q"},		  {"Skill_W", L"W"},		{"Skill_E", L"E"},		  {"Skill_R", L"R"},
		{"Skill_A", L"A"},		  {"Skill_S", L"S"},		{"Skill_D", L"D"},		  {"Skill_F", L"F"},
		{"Skill_T", L"T"},		  {"Skill_V", L"V"},		{"SpecialSkill_1", L"5"}, {"SpecialSkill_2", L"6"},
		{"SpecialSkill_3", L"7"}, {"SpecialSkill_4", L"8"}, {"SpecialSkill_5", L"9"}, {"SpecialSkill_6", L"0"},
		{"Item_1", L"1"},		  {"Item_2", L"2"},			{"Item_3", L"3"},		  {"Item_4", L"4"},
	};

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const auto DrawKeyLabel = [&](const char *pSlotId, const wchar_t *pLabel)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight))
			return;

		// 슬롯 그림의 하단 tab 중심을 사용하도록 가로50%·세로87% 위치에 키 글자를 배치한다.
		const f32_t fLabelCenterX = fX + fWidth * 0.5f;
		const f32_t fLabelCenterY = fY + fHeight * 0.87f;

		// 키 글자는 기존 Font_YoonGasiIIM을 사용한다. 측정 글자 높이와 슬롯 높이의 22%로 배율을 정한다.
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
		const f32_t fScale = (vMeasured.y > 0.f) ? (fHeight * 0.22f / vMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
									   float2_t(fLabelCenterX * textScaleX, fLabelCenterY * textScaleY), Colors::White,
									   0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	};

	// 쿠크 상호작용 mode가 T/V 슬롯을 숨길 때 해당 글자도 건너뛴다.
	const HUD_KOUKU_GIMMICK_STATE &koukuLabelState = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	const bool_t bKoukuModeLabels = koukuLabelState.isValid && HUD_KOUKU_HUD_MODE::NONE != koukuLabelState.eHudMode &&
									ETOUI(LEVEL::KOUKUSAYDON_ARENA) == currentLevel;
	// 탑승 중인 탈것과 워터팡 물총 HUD도 같은 T/V 글자 제외 조건을 사용한다.
	const bool_t bMountedLabels = 0u != keyLabelPlayer.iVehicleId || Is_WaterGunHudLevel(currentLevel, keyLabelPlayer);
	// 선박 HUD는 자체 문자 배치를 사용하므로 일반 class 퀵슬롯 글자를 그리기 전에 반환한다.
	if (m_bShipHudActive)
	{
		RenderShipHudTexts();
		return;
	}
	for (const KEY_LABEL &Label : LABELS)
	{
		// 선박의 아이템·특수 슬롯 제외 조건이다. 현재 선박 상태는 위에서 이미 전용 글자를 그리고 반환한다.
		if (m_bShipHudActive &&
			(0 == std::strncmp(Label.pSlotId, "Item_", 5) || 0 == std::strncmp(Label.pSlotId, "SpecialSkill_", 13)))
		{
			continue;
		}
		if ((bKoukuModeLabels || bMountedLabels) &&
			(0 == std::strcmp(Label.pSlotId, "Skill_T") || 0 == std::strcmp(Label.pSlotId, "Skill_V")))
		{
			continue;
		}
		DrawKeyLabel(Label.pSlotId, Label.pLabel);
	}
	if (m_bHudSpecialSlotShown)
		DrawKeyLabel("Special_Space", L"Space");

	// 가디언 나이트 Z는 저작된 일반 슬롯 GK_Id_SkillKeyBg의 중심을 기준으로 표시한다.
	// identity가 가득 찼거나 용 stance이면 흰색, 그 외에는 기존 어두운 색을 사용한다.
	if (LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT == keyLabelPlayer.eCharacterClass)
	{
		f32_t fKeyX = 0.f, fKeyY = 0.f, fKeyWidth = 0.f, fKeyHeight = 0.f;
		if (m_pHUDRuntimeView->Get_SlotRect("GK_Id_SkillKeyBg", fKeyX, fKeyY, fKeyWidth, fKeyHeight))
		{
			const bool_t bGaugeFull = 0u != keyLabelPlayer.iMaximumIdentity &&
									  keyLabelPlayer.iCurrentIdentity >= keyLabelPlayer.iMaximumIdentity;
			const bool_t bUsable =
				bGaugeFull || LostArk::Shared::PLAYER_STANCE_ID::GUARDIANKNIGHT_DRAGON == keyLabelPlayer.eStance;
			const float2_t vKeyMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), L"Z");
			const f32_t fKeyScale = (vKeyMeasured.y > 0.f) ? (fKeyHeight * 0.82f / vKeyMeasured.y) : 1.f;
			CGameInstance::Get().Draw_Text(
				TEXT("Font_YoonGasiIIM"), L"Z",
				float2_t((fKeyX + fKeyWidth * 0.5f) * textScaleX, (fKeyY + fKeyHeight * 0.5f) * textScaleY),
				bUsable ? Colors::White : XMVectorSet(0x68 / 255.f, 0x6C / 255.f, 0x20 / 255.f, 1.f), 0.f,
				float2_t(0.5f, 0.5f), fKeyScale * textUiScale);
		}
	}

	// 도화가·워로드의 Z/X는 일반 슬롯 rect에 맞출 공통 anchor가 없어 이 경로에서 키 글자를 생략한다.
}

namespace
{
constexpr const char *KOUKU_SLOT_KEYS[HUD_KOUKU_SLOT_COUNT] = {"Q", "W", "E", "R", "A", "S", "D", "F"};
constexpr const char *KOUKU_EMBLEM_SLOTS[] = {"Kouku_Emblem_Npc", "Kouku_Emblem_Pickup"};
// 쿠크 상호작용 HUD에서 제외하는 T/V 이미지·가림·flash 슬롯 목록이다.
constexpr const char *KOUKU_HIDDEN_SLOTS[] = {
	"Skill_T",		"Skill_T_Icon",	 "Skill_T_Frame",	 "Skill_T_Cooldown", "Skill_T_Flash", "Skill_V",
	"Skill_V_Icon", "Skill_V_Frame", "Skill_V_Cooldown", "Skill_V_Flash",	 "Skill_V_Edge1", "Skill_V_Edge2",
};

// 선박 HUD가 대신 표시될 때 숨길 HP·자원·아이템·class 특수 슬롯·charge gauge 목록이다.
constexpr const char *SHIP_HIDDEN_SLOTS[] = {
	"HealthBar_BG",
	"HealthBar",
	"HealthBar_Fill",
	"HealthBar_Shield_Fill",
	"ManaBar_BG",
	"ManaBar",
	"ManaBar_Fill",
	"Item_1",
	"Item_2",
	"Item_3",
	"Item_4",
	"Item_1_Icon",
	"Item_2_Icon",
	"Item_3_Icon",
	"Item_4_Icon",
	"SpecialSkill_1",
	"SpecialSkill_2",
	"SpecialSkill_3",
	"SpecialSkill_4",
	"SpecialSkill_5",
	"SpecialSkill_6",
	"SpecialSkill_1_Icon",
	"SpecialSkill_2_Icon",
	"SpecialSkill_3_Icon",
	"SpecialSkill_4_Icon",
	"SpecialSkill_5_Icon",
	"SpecialSkill_6_Icon",
	"ChargeGauge_Bg",
	"ChargeGauge_Track",
	"ChargeGauge_Fill",
};

// Update_VehicleHud가 선박 탑승 상태에 따라 함께 켜고 끄는 선박 전용 슬롯 목록이다.
constexpr const char *SHIP_HUD_SLOTS[] = {
	"Ship_Hud_Plate",		"Ship_Hud_Wheel",		"Ship_Hud_Bezel",		"Ship_Hud_Dome",
	"Ship_Hud_Icon",		"Ship_Slot_Q_Frame",	"Ship_Slot_W_Frame",	"Ship_Slot_E_Frame",
	"Ship_Slot_R_Frame",	"Ship_Slot_A_Frame",	"Ship_Slot_S_Frame",	"Ship_Slot_D_Frame",
	"Ship_Slot_F_Frame",	"Ship_Slot_Q_Icon",		"Ship_Slot_W_Icon",		"Ship_Slot_E_Icon",
	"Ship_Slot_R_Icon",		"Ship_Slot_A_Icon",		"Ship_Slot_S_Icon",		"Ship_Slot_D_Icon",
	"Ship_Slot_F_Icon",		"Ship_Slot_Q_Cooldown", "Ship_Slot_W_Cooldown", "Ship_Slot_E_Cooldown",
	"Ship_Slot_R_Cooldown", "Ship_Slot_A_Cooldown", "Ship_Slot_S_Cooldown", "Ship_Slot_D_Cooldown",
	"Ship_Slot_F_Cooldown", "Ship_Btn_Horn",		"Ship_Hud_Bottle",		"Ship_Hud_EventSlot",
	"Ship_Btn_Anchor_Bg",	"Ship_Btn_Anchor_Icon", "Ship_Btn_Boost_Dark",	"Ship_Btn_Boost_Fill",
	"Ship_Btn_Boost_Gear",	"Ship_Btn_Boost_Arrow", "Ship_Btn_Cruise_Icon", "Ship_Btn_Cruise_Ring",
	"Ship_Btn_AutoCruise",
};

// HUD 문서 조회에 사용할 mode 문자열을 고른다. 지원하지 않는 값은 nullptr로 반환해 조회 실패와 구분한다.
const char *KoukuHudModeId(const HUD_KOUKU_HUD_MODE eMode)
{
	switch (eMode)
	{
	case HUD_KOUKU_HUD_MODE::POLYMORPH:
		return "POLYMORPH";
	case HUD_KOUKU_HUD_MODE::MARIO:
		return "MARIO";
	case HUD_KOUKU_HUD_MODE::DANCE:
		return "DANCE";
	case HUD_KOUKU_HUD_MODE::MAZE:
		return "MAZE";
	default:
		return nullptr;
	}
}
} // 이 파일의 보조 구현 범위 끝

// 탈것·칭호·아이템 UI가 현재 지원 Level의 controller를 차용한다. Level이 교체되면 보관해 재사용하지 않으며 미지원 Level은 nullptr다.
CPlayerController *CMainApp::Find_ActivePlayerController() const
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (ETOUI(LEVEL::BERN) == currentLevel)
	{
		if (CLevel_Bern *pBern = CLevel_Bern::Get_Active())
			return &pBern->Get_PlayerController();
	}
	else if (ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel)
	{
		if (CLevel_CharacterSelect *pCharacterSelect = CLevel_CharacterSelect::Get_Active())
			return &pCharacterSelect->Get_DebugPlayerController();
	}
	else if (ETOUI(LEVEL::VALTAN_ARENA) == currentLevel)
	{
		if (CLevel_ValtanArena *pValtanArena = CLevel_ValtanArena::Get_Active())
			return &pValtanArena->Get_DebugPlayerController();
	}
	else if (ETOUI(LEVEL::KOUKUSAYDON_ARENA) == currentLevel)
	{
		if (CLevel_KoukuSaydonArena *pKoukuArena = CLevel_KoukuSaydonArena::Get_Active())
			return &pKoukuArena->Get_DebugPlayerController();
	}
	else if (ETOUI(LEVEL::COLOSSEUM) == currentLevel)
	{
		if (CLevel_Development *pColosseum = CLevel_Development::Get_Active(LEVEL::COLOSSEUM))
			return &pColosseum->Get_PlayerController();
	}
	return nullptr;
}
// 전투 HUD가 인벤토리·탈것 창에서 고른 값을 운반하고 내려놓은 슬롯에 세션용 단축 연결만 저장한다.
// 아이템 사용·장착이나 탈것 탑승 요청 자체는 여기서 보내지 않는다.
void CMainApp::Update_QuickSlotDrag()
{
	if (nullptr == m_pQuickSlotDragView || nullptr == m_pHUDRuntimeView)
		return;
	string strItemId, strIconPath;
	if (nullptr != m_pInventoryView && m_pInventoryView->Try_Consume_ItemPick(strItemId, strIconPath))
		m_pQuickSlotDragView->Begin_ItemCarry(strItemId, strIconPath);
	uint32_t iVehicleId = 0u;
	if (nullptr != m_pVehicleWindowView && m_pVehicleWindowView->Take_IconPick(iVehicleId, strIconPath))
		m_pQuickSlotDragView->Begin_VehicleCarry(iVehicleId, strIconPath);
	if (!m_pQuickSlotDragView->Update())
		return;

	// 내려놓은 위치가 운반 종류에 맞는 슬롯이면 세션의 단축 슬롯 값을 바꾸고, 어느 경우든 운반 상태를 끝낸다.
	CUIInputRouter &Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pHUDRuntimeView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pHUDRuntimeView->Get_ResolutionHeight();
	const auto Hovered = [&](const char *pSlotId)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		return m_pHUDRuntimeView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight) &&
			   Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
	};
	if (CQuickSlotDragView::PAYLOAD::ITEM == m_pQuickSlotDragView->Get_Payload())
	{
		constexpr const char *ITEM_SLOT_IDS[4] = {"Item_1", "Item_2", "Item_3", "Item_4"};
		for (int32_t i = 0; i < 4; ++i)
		{
			if (!Hovered(ITEM_SLOT_IDS[i]))
				continue;
			m_strItemQuickSlot[i] = m_pQuickSlotDragView->Get_ItemId();
			Play_UISound(UI_SOUND_CUE::BUTTON_CLICK);
			break;
		}
	}
	else if (CQuickSlotDragView::PAYLOAD::VEHICLE == m_pQuickSlotDragView->Get_Payload())
	{
		constexpr const char *SPECIAL_SLOT_IDS[6] = {"SpecialSkill_1", "SpecialSkill_2", "SpecialSkill_3",
													 "SpecialSkill_4", "SpecialSkill_5", "SpecialSkill_6"};
		for (int32_t i = 0; i < 6; ++i)
		{
			if (!Hovered(SPECIAL_SLOT_IDS[i]))
				continue;
			m_iSpecialQuickSlotVehicle[i] = m_pQuickSlotDragView->Get_VehicleId();
			Play_UISound(UI_SOUND_CUE::BUTTON_CLICK);
			break;
		}
	}
	m_pQuickSlotDragView->Cancel();
}

// 세션의 여섯 vehicle ID를 창의 아이콘 목록에서 조회해 특수 퀵슬롯에 표시한다. 정의가 없으면 해당 아이콘만 숨긴다.
void CMainApp::Update_SpecialQuickSlots()
{
	if (nullptr == m_pHUDRuntimeView)
		return;
	constexpr const char *SPECIAL_ICON_SLOT_IDS[6] = {"SpecialSkill_1_Icon", "SpecialSkill_2_Icon",
													  "SpecialSkill_3_Icon", "SpecialSkill_4_Icon",
													  "SpecialSkill_5_Icon", "SpecialSkill_6_Icon"};
	for (int32_t i = 0; i < 6; ++i)
	{
		const string *pIcon = (0u != m_iSpecialQuickSlotVehicle[i] && nullptr != m_pVehicleWindowView)
								  ? m_pVehicleWindowView->Find_IconAsset(m_iSpecialQuickSlotVehicle[i])
								  : nullptr;
		if (nullptr == pIcon || pIcon->empty())
		{
			m_pHUDRuntimeView->Set_SlotVisible(SPECIAL_ICON_SLOT_IDS[i], false);
			continue;
		}
		m_pHUDRuntimeView->Set_SlotTexture(SPECIAL_ICON_SLOT_IDS[i], *pIcon);
		m_pHUDRuntimeView->Set_SlotVisible(SPECIAL_ICON_SLOT_IDS[i], true);
	}
}

namespace
{
// 아래 Update_SkillIcons 앞의 아이콘 표와 함께 정의하는 조회 함수를 먼저 선언한다.
const char *Find_HudSkillIcon(LostArk::Shared::SKILL_ID iSkillId);
} // 이 파일의 보조 구현 범위 끝

// 공통 class HUD 갱신 뒤 복제된 탑승 ID로 일반 탈것 또는 선박 표시를 덮어쓴다.
// 이번 프레임의 선박 표시 플래그를 먼저 초기화하고 catalog·아이콘은 호출 동안만 차용한다.
void CMainApp::Update_VehicleHud()
{
	m_bShipHudActive = false;
	if (nullptr == m_pHUDRuntimeView)
		return;
	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	// 도보 상태의 class별 표시는 앞선 Update_CombatHUD의 ownerClass 선택이 복구한다. 이 함수는 탑승 상태일 때만 덮어쓴다.
	if (0u == player.iVehicleId)
		return;
	const std::vector<VEHICLE_SKILL_UI> *pSkills =
		nullptr != m_pVehicleWindowView ? m_pVehicleWindowView->Find_Skills(player.iVehicleId) : nullptr;

	// 탈것 전용 ownerClass를 고르고 class identity·T/V 슬롯을 숨긴다.
	m_pHUDRuntimeView->Set_ActiveOwnerClass("VehicleRiding");
	for (const char *pHiddenSlot : KOUKU_HIDDEN_SLOTS)
		m_pHUDRuntimeView->Set_SlotVisible(pHiddenSlot, false);
	// 선박은 일반 탈것 문양 대신 선박 전용 프레임을 표시한다. HP·자원·아이템·class 특수 슬롯은 선박 숨김 목록을 따른다.
	const VEHICLE_ACTOR_ENTRY *pMountedVehicle = CActorCatalog::Find_Vehicle(player.iVehicleId);
	const bool_t bShip = nullptr != pMountedVehicle && pMountedVehicle->isShip;
	m_bShipHudActive = bShip;
	m_pHUDRuntimeView->Set_SlotVisible("Vehicle_Hud_Emblem", !bShip);
	for (const char *pShipSlot : SHIP_HUD_SLOTS)
		m_pHUDRuntimeView->Set_SlotVisible(pShipSlot, bShip);
	if (bShip)
	{
		for (const char *pShipHiddenSlot : SHIP_HIDDEN_SLOTS)
			m_pHUDRuntimeView->Set_SlotVisible(pShipHiddenSlot, false);
		// 선박의 Ship_Slot_*를 사용하므로 class Q~F 슬롯의 이미지·가림·표식을 함께 숨긴다.
		for (const char *pKey : KOUKU_SLOT_KEYS)
		{
			for (const char *pPart : {"", "_Icon", "_Frame", "_Cooldown", "_Flash", "_TypeMark", "_Chain"})
				m_pHUDRuntimeView->Set_SlotVisible(string("Skill_") + pKey + pPart, false);
		}
		for (const char *pClassSlot : {"Skill_T_TypeMark", "Skill_T_Chain", "Skill_V_TypeMark", "Skill_V_Chain"})
			m_pHUDRuntimeView->Set_SlotVisible(pClassSlot, false);

		// 선박의 8개 키 중 실제 등록된 VEHICLE_SKILL_UI만 아이콘을 표시한다.
		// 시간 표시는 해당 탈것 스킬의 Server cooldown을 사용한다.
		for (size_t i = 0; i < HUD_KOUKU_SLOT_COUNT; ++i)
		{
			const string strKey = KOUKU_SLOT_KEYS[i];
			const VEHICLE_SKILL_UI *pSkill = nullptr;
			if (nullptr != pSkills)
			{
				for (const VEHICLE_SKILL_UI &Skill : *pSkills)
				{
					if (Skill.strSlot == strKey)
						pSkill = &Skill;
				}
			}
			const string strIconSlot = "Ship_Slot_" + strKey + "_Icon";
			const string strCooldownSlot = "Ship_Slot_" + strKey + "_Cooldown";
			const bool_t bHasIcon = nullptr != pSkill && !pSkill->strIconAsset.empty();
			if (bHasIcon)
				m_pHUDRuntimeView->Set_SlotTexture(strIconSlot, pSkill->strIconAsset);
			m_pHUDRuntimeView->Set_SlotVisible(strIconSlot, bHasIcon);
			const f32_t fRatio =
				nullptr != pSkill ? Resolve_HudCooldownRatio(player, pSkill->iSkillId, pSkill->iCooldownMs, strIconSlot)
								  : 0.f;
			m_pHUDRuntimeView->Set_SlotTint(strCooldownSlot, float4_t(0.f, 0.f, 0.f, 150.f / 255.f));
			m_pHUDRuntimeView->Set_SlotArcRatio(strCooldownSlot, fRatio);
			m_pHUDRuntimeView->Set_SlotVisible(strCooldownSlot, fRatio > 0.f);
		}

		// 선박 boost의 남은 cooldown 비율을 1에서 빼 준비된 비율로 링을 채운다.
		f32_t fBoostReady = 1.f;
		if (nullptr != pSkills)
		{
			for (const VEHICLE_SKILL_UI &Skill : *pSkills)
			{
				if ("SPACE" == Skill.strSlot)
					fBoostReady = 1.f - Resolve_HudCooldownRatio(player, Skill.iSkillId, Skill.iCooldownMs, string());
			}
		}
		m_pHUDRuntimeView->Set_SlotArcRatio("Ship_Btn_Boost_Fill", fBoostReady);

		// 현재는 보급 소모 상태를 별도 사용하지 않고 catalog 최대량을 현재량에도 대입한다.
		// 최대량이 0이면 빈 frame, 양수이면 가득 찬 frame을 표시한다.
		m_iShipSupplyMax = pMountedVehicle->iMaxSupply;
		m_iShipSupply = m_iShipSupplyMax;
		constexpr int32_t SHIP_DOME_FRAME_COUNT = 41;
		const f32_t fSupplyRatio =
			m_iShipSupplyMax > 0u
				? std::clamp(static_cast<f32_t>(m_iShipSupply) / static_cast<f32_t>(m_iShipSupplyMax), 0.f, 1.f)
				: 0.f;
		m_pHUDRuntimeView->Set_Animation_Frame(
			"Ship_Hud_Dome",
			static_cast<int32_t>(std::lround(fSupplyRatio * static_cast<f32_t>(SHIP_DOME_FRAME_COUNT - 1))));

		// 노트 표시는 기존 표본에서 정한 추정 배율 22.3/4를 m/s 이동 속도에 곱한다.
		// 정확한 물리 단위 환산값이 아니며 이동 목표가 없으면 0으로 표시한다.
		constexpr f32_t SHIP_KNOTS_PER_METER_PER_SECOND = 22.3f / 4.f;
		m_fShipKnots = player.hasMoveGoal ? player.fMoveSpeed * SHIP_KNOTS_PER_METER_PER_SECOND : 0.f;
		return;
	}

	// 일반 탈것은 등록된 키의 아이콘과 cooldown을 사용하고 나머지는 잠금 아이콘을 표시한다.
	// SPACE는 아래 별도 특수 슬롯에서 처리한다.
	for (size_t i = 0; i < HUD_KOUKU_SLOT_COUNT; ++i)
	{
		const string strKey = KOUKU_SLOT_KEYS[i];
		const VEHICLE_SKILL_UI *pSkill = nullptr;
		if (nullptr != pSkills)
		{
			for (const VEHICLE_SKILL_UI &Skill : *pSkills)
			{
				if (Skill.strSlot == strKey)
					pSkill = &Skill;
			}
		}
		const string strIconSlot = "Skill_" + strKey + "_Icon";
		m_pHUDRuntimeView->Set_SlotTexture(strIconSlot, nullptr != pSkill ? pSkill->strIconAsset
																		  : string("UI/Vehicle/Vehicle_LockIcon.png"));
		m_pHUDRuntimeView->Set_SlotVisible(strIconSlot, true);
		const string strCooldownSlot = "Skill_" + strKey + "_Cooldown";
		const f32_t fRatio = nullptr != pSkill ? Resolve_HudCooldownRatio(player, pSkill->iSkillId, pSkill->iCooldownMs,
																		  "Skill_" + strKey)
											   : 0.f;
		m_pHUDRuntimeView->Set_SlotTint(strCooldownSlot, float4_t(0.f, 0.f, 0.f, 150.f / 255.f));
		m_pHUDRuntimeView->Set_SlotArcRatio(strCooldownSlot, fRatio);
		m_pHUDRuntimeView->Set_SlotVisible(strCooldownSlot, fRatio > 0.f);
	}
}

// Server가 물총을 장착한 도보 마하라카 플레이어일 때 기존 class HUD를 물총 슬롯으로 바꾼다.
void CMainApp::Update_WaterGunHud()
{
	if (nullptr == m_pHUDRuntimeView)
		return;
	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	if (!Is_WaterGunHudLevel(CGameInstance::Get().Get_CurrentLevelID(), player))
		return;

	// 워터팡 전용 ownerClass를 고르고 class identity·T/V를 숨긴 뒤 pickup 문양을 표시한다.
	m_pHUDRuntimeView->Set_ActiveOwnerClass("WaterpangWaterGun");
	for (const char *pHiddenSlot : KOUKU_HIDDEN_SLOTS)
		m_pHUDRuntimeView->Set_SlotVisible(pHiddenSlot, false);
	m_pHUDRuntimeView->Set_SlotVisible("Kouku_Emblem_Pickup", true);

	// Shared의 물총 스킬 표에 있는 키만 아이콘·cooldown을 표시한다. 해당 키가 없으면 아이콘과 가림 슬롯을 함께 숨긴다.
	for (size_t i = 0; i < HUD_KOUKU_SLOT_COUNT; ++i)
	{
		const string strKey = KOUKU_SLOT_KEYS[i];
		const string strIconSlot = "Skill_" + strKey + "_Icon";
		const string strCooldownSlot = "Skill_" + strKey + "_Cooldown";
		const LostArk::Shared::MAHARAKA_WATERGUN_SKILL *pGun =
			LostArk::Shared::Find_MaharakaWaterGunSkillBySlot(strKey.front());
		if (nullptr == pGun)
		{
			m_pHUDRuntimeView->Set_SlotVisible(strIconSlot, false);
			m_pHUDRuntimeView->Set_SlotArcRatio(strCooldownSlot, 0.f);
			m_pHUDRuntimeView->Set_SlotVisible(strCooldownSlot, false);
			continue;
		}
		m_pHUDRuntimeView->Set_SlotTexture(strIconSlot,
										   "UI/HUD/Waterpang/skill_" + std::to_string(pGun->iSkillId) + ".png");
		m_pHUDRuntimeView->Set_SlotVisible(strIconSlot, true);
		const f32_t fRatio = pGun->iCooldownMs > 0u ? Resolve_HudCooldownRatio(player, pGun->iSkillId,
																			   pGun->iCooldownMs, "Skill_" + strKey)
													: 0.f;
		m_pHUDRuntimeView->Set_SlotTint(strCooldownSlot, float4_t(0.f, 0.f, 0.f, 150.f / 255.f));
		m_pHUDRuntimeView->Set_SlotArcRatio(strCooldownSlot, fRatio);
		m_pHUDRuntimeView->Set_SlotVisible(strCooldownSlot, fRatio > 0.f);
	}
}

// 탈것·물총·SPACE 표시가 특정 스킬의 남은 cooldown 비율을 요청한다. 없거나 이미 끝났으면 0이며 일치한 첫 진행 중 행만 소비한다.
f32_t CMainApp::Resolve_HudCooldownRatio(const HUD_PLAYER_STATE &player, const uint32_t iSkillId,
										 const uint32_t iCooldownMs, const string &strTextSlotId)
{
	// 복제된 종료 tick에서 현재 Server tick을 뺀 남은 비율이다. 길이는 snapshot duration을 우선하고 없으면 catalog ms를 사용한다.
	// 텍스트 슬롯이 있으면 남은 초 표시용 종료 tick도 이번 프레임 목록에 추가한다.
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	for (const LostArk::Shared::SKILL_COOLDOWN_SNAPSHOT &Cooldown : player.Cooldowns)
	{
		if (Cooldown.iSkillId != iSkillId || Cooldown.iCooldownEndTick <= player.iServerTick)
			continue;
		const f32_t fRemaining = static_cast<f32_t>(Cooldown.iCooldownEndTick - player.iServerTick) / SERVER_TICK_HZ;
		const f32_t fTotal = Cooldown.iCooldownDurationTicks > 0u
								 ? static_cast<f32_t>(Cooldown.iCooldownDurationTicks) / SERVER_TICK_HZ
								 : (iCooldownMs > 0u ? static_cast<f32_t>(iCooldownMs) / 1000.f : fRemaining);
		if (!strTextSlotId.empty())
			m_HudTimedTexts.push_back({strTextSlotId, Cooldown.iCooldownEndTick, false, false});
		return fTotal > 0.f ? std::clamp(fRemaining / fTotal, 0.f, 1.f) : 0.f;
	}
	return 0.f;
}

// 현재 SPACE 동작의 cooldown 표시를 결정하고 숨기는 경우 예약된 시간 글자까지 정리한다.
void CMainApp::Update_SpecialSlot()
{
	if (nullptr == m_pHUDRuntimeView)
		return;
	// 선박의 SPACE는 전용 boost 버튼이 표시하므로 일반 특수 슬롯과 예약된 글자를 숨긴다.
	if (m_bShipHudActive)
	{
		m_bHudSpecialSlotShown = false;
		for (const char *pSpecial :
			 {"Special_Space", "Special_Space_Frame", "Special_Space_Icon", "Special_Space_Cooldown"})
			m_pHUDRuntimeView->Set_SlotVisible(pSpecial, false);
		m_HudTimedTexts.erase(std::remove_if(m_HudTimedTexts.begin(), m_HudTimedTexts.end(),
											 [](const HUD_TIMED_TEXT &Text)
											 { return "Special_Space" == Text.strSlotId; }),
							  m_HudTimedTexts.end());
		return;
	}
	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	const HUD_KOUKU_GIMMICK_STATE &kouku = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	const bool_t bKoukuMode = kouku.isValid && HUD_KOUKU_HUD_MODE::NONE != kouku.eHudMode;

	// 탑승 중이면 탈것 SPACE 스킬을, 도보이면 현재 class·stance에 맞는 SPACE 스킬을 고른다.
	const char *pIcon = nullptr;
	f32_t fRatio = 0.f;
	if (0u != player.iVehicleId)
	{
		const std::vector<VEHICLE_SKILL_UI> *pSkills =
			nullptr != m_pVehicleWindowView ? m_pVehicleWindowView->Find_Skills(player.iVehicleId) : nullptr;
		if (nullptr != pSkills)
		{
			for (const VEHICLE_SKILL_UI &Skill : *pSkills)
			{
				if ("SPACE" != Skill.strSlot)
					continue;
				pIcon = Skill.strIconAsset.c_str();
				fRatio = Resolve_HudCooldownRatio(player, Skill.iSkillId, Skill.iCooldownMs, "Special_Space");
			}
		}
	}
	else if (const PLAYER_SKILL_DEFINITION *pSkill =
				 CPlayerSkillCatalog::Find_BySlot(player.eCharacterClass, "SPACE", player.eStance);
			 nullptr != pSkill)
	{
		pIcon = Find_HudSkillIcon(pSkill->iSkillId);
		fRatio = Resolve_HudCooldownRatio(player, pSkill->iSkillId, pSkill->iCooldownMs, "Special_Space");
	}

	// 일반 특수 슬롯은 쿠크 상호작용 mode가 아니고 유효 플레이어·아이콘·진행 중 cooldown이 있을 때만 표시한다.
	m_bHudSpecialSlotShown = !bKoukuMode && nullptr != pIcon && player.isValid && fRatio > 0.f;
	m_pHUDRuntimeView->Set_SlotVisible("Special_Space", m_bHudSpecialSlotShown);
	m_pHUDRuntimeView->Set_SlotVisible("Special_Space_Frame", m_bHudSpecialSlotShown);
	m_pHUDRuntimeView->Set_SlotVisible("Special_Space_Icon", m_bHudSpecialSlotShown);
	if (m_bHudSpecialSlotShown)
		m_pHUDRuntimeView->Set_SlotTexture("Special_Space_Icon", pIcon);
	m_pHUDRuntimeView->Set_SlotArcRatio("Special_Space_Cooldown", fRatio);
	m_pHUDRuntimeView->Set_SlotVisible("Special_Space_Cooldown", m_bHudSpecialSlotShown);
	if (!m_bHudSpecialSlotShown)
	{
		// 숨긴 슬롯에는 앞서 예약한 남은 시간 글자도 남지 않게 제거한다.
		m_HudTimedTexts.erase(std::remove_if(m_HudTimedTexts.begin(), m_HudTimedTexts.end(),
											 [](const HUD_TIMED_TEXT &Text)
											 { return "Special_Space" == Text.strSlotId; }),
							  m_HudTimedTexts.end());
	}
}

// 저작된 source 순서로 최대 네 개씩 buff/debuff 표시 값을 모은 뒤 스킬 buff를 남은 자리에 추가한다.
// 아이콘 포인터는 멤버 문자열 또는 탈것 목록을 이 호출 동안만 차용하며 실제 buff 적용·만료는 Server가 소유한다.
void CMainApp::Update_BuffBar()
{
	if (nullptr == m_pHUDRuntimeView)
		return;
	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	constexpr size_t BUFF_SLOTS = 4;
	struct BUFF_DRAW
	{
		const string *pIcon;
		uint32_t iEndTick;
		f32_t fRatio;
	};
	std::vector<BUFF_DRAW> Buffs, Debuffs;
	Buffs.reserve(BUFF_SLOTS);
	Debuffs.reserve(BUFF_SLOTS);

	// 버프 활성 상태는 복제된 플레이어 값으로 판정하고 저작 JSON은 표시 아이콘·분류를 연결한다.
	for (const HUD_BUFF_SOURCE &Source : m_HudBuffSources)
	{
		if (!player.isValid)
			break;
		BUFF_DRAW Draw{&Source.strIconAsset, 0u, 0.f};
		bool_t bActive = false;
		if ("vehicle" == Source.strSource)
		{
			const string *pVehicleIcon = (0u != player.iVehicleId && nullptr != m_pVehicleWindowView)
											 ? m_pVehicleWindowView->Find_IconAsset(player.iVehicleId)
											 : nullptr;
			bActive = nullptr != pVehicleIcon && !pVehicleIcon->empty();
			Draw.pIcon = pVehicleIcon;
		}
		else if ("stance" == Source.strSource)
		{
			bActive = ("WARLORD_DEFENSE" == Source.strStance &&
					   LostArk::Shared::PLAYER_STANCE_ID::WARLORD_DEFENSE == player.eStance) ||
					  ("GUARDIANKNIGHT_DRAGON" == Source.strStance &&
					   LostArk::Shared::PLAYER_STANCE_ID::GUARDIANKNIGHT_DRAGON == player.eStance);
		}
		else if ("silence" == Source.strSource)
		{
			bActive = Is_ServerDeadlinePending(player.iServerTick, player.iSilenceEndTick);
			Draw.iEndTick = player.iSilenceEndTick;
			if (bActive && 0u != player.iSilenceDurationTicks)
			{
				Draw.fRatio = std::clamp(static_cast<f32_t>(player.iSilenceEndTick - player.iServerTick) /
											 static_cast<f32_t>(player.iSilenceDurationTicks),
										 0.f, 1.f);
			}
		}
		else if ("fetter" == Source.strSource)
		{
			bActive = player.isPatternBound;
			Draw.iEndTick = player.iPatternBindEndTick > player.iServerTick ? player.iPatternBindEndTick : 0u;
		}
		else if ("fear" == Source.strSource)
		{
			bActive = Is_ServerDeadlinePending(player.iServerTick, player.iFearEndTick);
			Draw.iEndTick = player.iFearEndTick;
		}
		if (!bActive || nullptr == Draw.pIcon || Draw.pIcon->empty())
			continue;
		std::vector<BUFF_DRAW> &List = Source.bDebuff ? Debuffs : Buffs;
		if (List.size() < BUFF_SLOTS)
			List.push_back(Draw);
	}

	// 스킬이 준 활성 buff ID를 아이콘 표에서 찾아 표시한다. 현재 플레이어 buff와 보스에게 적용된 buff를 같은 조회 함수로 처리한다.
	// snapshot의 buff 배열과 아이콘 문자열을 복사하지 않고 이 호출 동안 조회한다. 만료됐거나 아이콘이 없는 항목은 건너뛴다.
	const auto AppendSkillBuffs = [&](const LostArk::Shared::ACTIVE_BUFF *pBuffs, const size_t iCount,
									  const uint32_t iServerTick, std::vector<BUFF_DRAW> &List)
	{
		for (size_t i = 0; i < iCount && List.size() < BUFF_SLOTS; ++i)
		{
			const auto Found = m_HudSkillBuffIcons.find(pBuffs[i].iBuffId);
			if (m_HudSkillBuffIcons.end() == Found || Found->second.empty())
				continue;
			if (!Is_ServerDeadlinePending(iServerTick, pBuffs[i].iEndTick))
				continue;
			List.push_back({&Found->second, pBuffs[i].iEndTick, 0.f});
		}
	};
	if (player.isValid)
	{
		AppendSkillBuffs(player.ActiveBuffs, player.iActiveBuffCount, player.iServerTick, Buffs);
	}
	const HUD_BOSS_STATE &boss = CCombatHUDViewModel::Get().Get_Boss();
	if (boss.isValid)
	{
		AppendSkillBuffs(boss.ActiveBuffs, boss.iActiveBuffCount, player.iServerTick, Debuffs);
	}

	const auto Apply = [&](const char *pPrefix, const std::vector<BUFF_DRAW> &List, const bool_t bDebuff)
	{
		for (size_t i = 0; i < BUFF_SLOTS; ++i)
		{
			const string strBase = string(pPrefix) + "_" + std::to_string(i);
			const bool_t bShown = i < List.size();
			m_pHUDRuntimeView->Set_SlotVisible(strBase + "_Bg", bShown);
			m_pHUDRuntimeView->Set_SlotVisible(strBase + "_Border", bShown);
			m_pHUDRuntimeView->Set_SlotVisible(strBase + "_Icon", bShown);
			if (!bShown)
			{
				m_pHUDRuntimeView->Set_SlotVisible(strBase + "_Cooldown", false);
				continue;
			}
			const BUFF_DRAW &Draw = List[i];
			m_pHUDRuntimeView->Set_SlotTexture(strBase + "_Icon", *Draw.pIcon);
			m_pHUDRuntimeView->Set_SlotArcRatio(strBase + "_Cooldown", Draw.fRatio);
			m_pHUDRuntimeView->Set_SlotVisible(strBase + "_Cooldown", Draw.fRatio > 0.f);
			if (0u != Draw.iEndTick)
				m_HudTimedTexts.push_back({strBase + "_Icon", Draw.iEndTick, bDebuff, true});
		}
	};
	Apply("Buff", Buffs, false);
	Apply("Debuff", Debuffs, true);
}

// class 스킬의 종류 표식과 현재 combo 입력 window를 sprite에 표시한다.
// Server 입력 허용 판정은 바꾸지 않으며 탈것·쿠크 mode에서는 class 표식을 숨긴다.
void CMainApp::Update_SkillSlotMarks()
{
	if (nullptr == m_pHUDRuntimeView)
		return;
	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	const HUD_KOUKU_GIMMICK_STATE &kouku = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	const bool_t bClassSlots =
		0u == player.iVehicleId && !(kouku.isValid && HUD_KOUKU_HUD_MODE::NONE != kouku.eHudMode);
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	constexpr const char *INPUT_SLOTS[] = {"Q", "W", "E", "R", "A", "S", "D", "F", "T", "V"};

	for (const char *pInputSlot : INPUT_SLOTS)
	{
		const string strMarkSlot = string("Skill_") + pInputSlot + "_TypeMark";
		const string strChainSlot = string("Skill_") + pInputSlot + "_Chain";
		const PLAYER_SKILL_DEFINITION *pSkill =
			bClassSlots ? CPlayerSkillCatalog::Find_BySlot(player.eCharacterClass, pInputSlot, player.eStance)
						: nullptr;

		// 스킬별 표식 종류는 m_HudSkillMarks, 종류별 이미지는 m_HudSkillMarkAssets에서 연결한다.
		const string *pMarkAsset = nullptr;
		if (nullptr != pSkill)
		{
			for (const HUD_SKILL_MARK &Mark : m_HudSkillMarks)
			{
				if (Mark.iSkillId != pSkill->iSkillId)
					continue;
				for (const auto &Asset : m_HudSkillMarkAssets)
				{
					if (Asset.first == Mark.strMark)
						pMarkAsset = &Asset.second;
				}
			}
		}
		if (nullptr != pMarkAsset)
			m_pHUDRuntimeView->Set_SlotTexture(strMarkSlot, *pMarkAsset);
		m_pHUDRuntimeView->Set_SlotVisible(strMarkSlot, nullptr != pMarkAsset);

		// 현재 시전 스킬·combo stage가 일치할 때만 입력 window의 남은 비율을 계산한다.
		// 시작/현재 tick은 복제 상태이고 window의 열림·닫힘 ms는 같은 스킬 catalog의 stage 값이다.
		f32_t fChain = 0.f;
		if (nullptr != pSkill && pSkill->iSkillId == player.iCurrentSkillId && player.iComboStage > 0u &&
			player.iComboStage <= pSkill->ComboStages.size())
		{
			const PLAYER_COMBO_STAGE_TIMING &Stage = pSkill->ComboStages[player.iComboStage - 1u];
			if (Stage.iInputCloseMs > Stage.iInputOpenMs && player.iServerTick >= player.iActionStartTick)
			{
				const f32_t fElapsedMs =
					static_cast<f32_t>(player.iServerTick - player.iActionStartTick) * 1000.f / SERVER_TICK_HZ;
				if (fElapsedMs >= static_cast<f32_t>(Stage.iInputOpenMs) &&
					fElapsedMs <= static_cast<f32_t>(Stage.iInputCloseMs))
				{
					fChain = (static_cast<f32_t>(Stage.iInputCloseMs) - fElapsedMs) /
							 static_cast<f32_t>(Stage.iInputCloseMs - Stage.iInputOpenMs);
				}
			}
		}
		m_pHUDRuntimeView->Set_SlotArcRatio(strChainSlot, std::clamp(fChain, 0.f, 1.f));
		m_pHUDRuntimeView->Set_SlotVisible(strChainSlot, fChain > 0.f);
	}
}

// 초기화가 스킬 표식·버프 source·buff ID 아이콘의 저작 JSON을 읽는다. 먼저 기존 목록을 비우므로 실패가 이전 값을 보존하는 reload 계약은 아니다.
// 세 문서는 각각 지역 값으로 검증한 뒤 해당 목록만 확정하며 숫자 ID는 타입 검사 뒤 기존 정수 변환을 사용한다.
void CMainApp::Load_HudQuickSlotData()
{
	m_HudSkillMarks.clear();
	m_HudSkillMarkAssets.clear();
	m_HudBuffSources.clear();
	m_HudSkillBuffIcons.clear();
	// Data 상대 파일의 byte를 읽고 JSON object인지 확인한다. 오류 문자열은 이 helper 밖으로 전달하지 않는다.
	const auto ReadObject = [](const wchar_t *pRelative, DATA_JSON_VALUE &Root) -> bool_t
	{
		ifstream Stream(CProjectDataRoot::Resolve(pRelative), ios::binary);
		if (!Stream.is_open())
			return false;
		const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
		string Error;
		return CDataJson::Parse(Text, Root, Error) && Root.Is_Object();
	};

	DATA_JSON_VALUE Marks;
	if (ReadObject(L"UI/HUD/SkillSlotMarks.json", Marks))
	{
		vector<HUD_SKILL_MARK> StagedMarks;
		vector<pair<string, string>> StagedAssets;
		bool_t bValid = true;
		if (const DATA_JSON_VALUE *pAssets = Marks.Find("assets"); nullptr != pAssets && pAssets->Is_Object())
		{
			for (const auto &Entry : pAssets->Get_Object())
			{
				if (!Entry.second.Is_String())
				{
					bValid = false;
					break;
				}
				StagedAssets.emplace_back(Entry.first, Entry.second.Get_String());
			}
		}
		const DATA_JSON_VALUE *pMarks = Marks.Find("marks");
		if (nullptr == pMarks || !pMarks->Is_Array())
			bValid = false;
		else
		{
			for (const DATA_JSON_VALUE &Value : pMarks->Get_Array())
			{
				const DATA_JSON_VALUE *pId = Value.Is_Object() ? Value.Find("skillId") : nullptr;
				const DATA_JSON_VALUE *pMark = Value.Is_Object() ? Value.Find("mark") : nullptr;
				if (nullptr == pId || !pId->Is_Number() || nullptr == pMark || !pMark->Is_String())
				{
					bValid = false;
					break;
				}
				StagedMarks.push_back({static_cast<uint32_t>(pId->Get_Number()), pMark->Get_String()});
			}
		}
		if (bValid)
		{
			m_HudSkillMarks = std::move(StagedMarks);
			m_HudSkillMarkAssets = std::move(StagedAssets);
		}
		else
			OutputDebugStringA("[HUD] SkillSlotMarks.json is invalid -- no skill type marks.\n");
	}

	DATA_JSON_VALUE Buffs;
	if (ReadObject(L"UI/HUD/HudBuffSources.json", Buffs))
	{
		vector<HUD_BUFF_SOURCE> Staged;
		bool_t bValid = true;
		const DATA_JSON_VALUE *pSources = Buffs.Find("sources");
		if (nullptr == pSources || !pSources->Is_Array())
			bValid = false;
		else
		{
			for (const DATA_JSON_VALUE &Value : pSources->Get_Array())
			{
				const DATA_JSON_VALUE *pSource = Value.Is_Object() ? Value.Find("source") : nullptr;
				const DATA_JSON_VALUE *pKind = Value.Is_Object() ? Value.Find("kind") : nullptr;
				if (nullptr == pSource || !pSource->Is_String() || nullptr == pKind || !pKind->Is_String())
				{
					bValid = false;
					break;
				}
				HUD_BUFF_SOURCE Source{};
				Source.strSource = pSource->Get_String();
				Source.bDebuff = "debuff" == pKind->Get_String();
				if (const DATA_JSON_VALUE *pStance = Value.Find("stance"); nullptr != pStance && pStance->Is_String())
					Source.strStance = pStance->Get_String();
				if (const DATA_JSON_VALUE *pIcon = Value.Find("iconAsset"); nullptr != pIcon && pIcon->Is_String())
					Source.strIconAsset = pIcon->Get_String();
				Staged.push_back(std::move(Source));
			}
		}
		if (bValid)
			m_HudBuffSources = std::move(Staged);
		else
			OutputDebugStringA("[HUD] HudBuffSources.json is invalid -- no buff bar.\n");
	}

	DATA_JSON_VALUE BuffIcons;
	if (ReadObject(L"UI/HUD/HudBuffIcons.json", BuffIcons))
	{
		unordered_map<uint32_t, string> Staged;
		bool_t bValid = true;
		const DATA_JSON_VALUE *pIcons = BuffIcons.Find("icons");
		if (nullptr == pIcons || !pIcons->Is_Array())
			bValid = false;
		else
		{
			for (const DATA_JSON_VALUE &Value : pIcons->Get_Array())
			{
				const DATA_JSON_VALUE *pBuffId = Value.Is_Object() ? Value.Find("buffId") : nullptr;
				const DATA_JSON_VALUE *pAsset = Value.Is_Object() ? Value.Find("iconAsset") : nullptr;
				if (nullptr == pBuffId || !pBuffId->Is_Number() || nullptr == pAsset || !pAsset->Is_String() ||
					pAsset->Get_String().empty())
				{
					bValid = false;
					break;
				}
				Staged.emplace(static_cast<uint32_t>(pBuffId->Get_Number()), pAsset->Get_String());
			}
		}
		if (bValid)
			m_HudSkillBuffIcons = std::move(Staged);
		else
			OutputDebugStringA("[HUD] HudBuffIcons.json is invalid -- no skill buff icons.\n");
	}
}

// 초기화가 쿠크 mode의 emblem·icon 목록을 값으로 읽는다. 기존 목록을 먼저 비우고 전체 mode 검증 성공 뒤 교체하므로 실패하면 빈 목록이다.
void CMainApp::Load_KoukuHudModes()
{
	m_KoukuHudModes.clear();
	const filesystem::path DataPath = CProjectDataRoot::Resolve(L"UI/KoukuSaydon/KoukuHudModes.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
	{
		OutputDebugStringA("[KoukuHudMode] KoukuHudModes.json missing -- no interaction mode can show.\n");
		return;
	}
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		OutputDebugStringA(("[KoukuHudMode] KoukuHudModes.json parse failed: " + Error + "\n").c_str());
		return;
	}
	const DATA_JSON_VALUE *pModes = Root.Find("modes");
	if (nullptr == pModes || !pModes->Is_Array())
		return;

	vector<KOUKU_HUD_MODE_DEF> Staged;
	for (const DATA_JSON_VALUE &Value : pModes->Get_Array())
	{
		if (!Value.Is_Object())
			return;
		KOUKU_HUD_MODE_DEF Mode{};
		const DATA_JSON_VALUE *pId = Value.Find("id");
		const DATA_JSON_VALUE *pEmblem = Value.Find("emblemSlot");
		const DATA_JSON_VALUE *pSkills = Value.Find("skills");
		if (nullptr == pId || !pId->Is_String() || pId->Get_String().empty() || nullptr == pEmblem ||
			!pEmblem->Is_String() || nullptr == pSkills || !pSkills->Is_Array() ||
			pSkills->Get_Array().size() > HUD_KOUKU_SLOT_COUNT)
		{
			OutputDebugStringA("[KoukuHudMode] KoukuHudModes.json has an invalid mode entry -- file rejected.\n");
			return;
		}
		Mode.strId = pId->Get_String();
		Mode.strEmblemSlot = pEmblem->Get_String();
		if (const DATA_JSON_VALUE *pRandom = Value.Find("randomOrder"); nullptr != pRandom && pRandom->Is_Boolean())
		{
			Mode.bRandomOrder = pRandom->Get_Boolean();
		}
		for (const DATA_JSON_VALUE &Skill : pSkills->Get_Array())
		{
			const DATA_JSON_VALUE *pIcon = Skill.Is_Object() ? Skill.Find("iconPath") : nullptr;
			if (nullptr == pIcon || !pIcon->Is_String() || pIcon->Get_String().empty())
			{
				OutputDebugStringA("[KoukuHudMode] KoukuHudModes.json skill without iconPath -- file rejected.\n");
				return;
			}
			KOUKU_HUD_MODE_SKILL Entry{};
			Entry.strIconPath = pIcon->Get_String();
			if (const DATA_JSON_VALUE *pName = Skill.Find("displayName"); nullptr != pName && pName->Is_String())
			{
				Entry.strDisplayName = pName->Get_String();
			}
			Mode.Skills.push_back(std::move(Entry));
		}
		Staged.push_back(std::move(Mode));
	}
	m_KoukuHudModes = std::move(Staged);
}

// Update_CombatHUD가 준비한 view에 Server의 mode·슬롯 index·cooldown을 표시한다.
// 호출자는 유효한 m_pHUDRuntimeView를 보장하며 mode 정의가 없으면 문양을 숨긴 상태로 반환한다.
void CMainApp::Update_KoukuHudMode()
{
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	const float4_t vCooldownTint = float4_t(0.f, 0.f, 0.f, 150.f / 255.f);

	const HUD_KOUKU_GIMMICK_STATE &kouku = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	const bool_t bActive = kouku.isValid && HUD_KOUKU_HUD_MODE::NONE != kouku.eHudMode &&
						   ETOUI(LEVEL::KOUKUSAYDON_ARENA) == CGameInstance::Get().Get_CurrentLevelID();
	const KOUKU_HUD_MODE_DEF *pMode = nullptr;
	if (bActive)
	{
		const char *pModeId = KoukuHudModeId(kouku.eHudMode);
		for (const KOUKU_HUD_MODE_DEF &Mode : m_KoukuHudModes)
		{
			if (nullptr != pModeId && Mode.strId == pModeId)
			{
				pMode = &Mode;
				break;
			}
		}
	}

	// 공통 HUD 갱신이 슬롯을 먼저 켜므로 쿠크 mode가 꺼진 프레임에도 전용 문양을 다시 숨긴다.
	for (const char *pEmblemSlot : KOUKU_EMBLEM_SLOTS)
		m_pHUDRuntimeView->Set_SlotVisible(pEmblemSlot, false);
	if (nullptr == pMode)
		return;

	// 쿠크 전용 ownerClass를 골라 class identity 표시를 제외한다.
	m_pHUDRuntimeView->Set_ActiveOwnerClass("KoukuSaydonInteraction");
	for (const char *pHiddenSlot : KOUKU_HIDDEN_SLOTS)
		m_pHUDRuntimeView->Set_SlotVisible(pHiddenSlot, false);
	m_pHUDRuntimeView->Set_SlotVisible(pMode->strEmblemSlot, true);

	for (size_t i = 0; i < HUD_KOUKU_SLOT_COUNT; ++i)
	{
		const string strKey = KOUKU_SLOT_KEYS[i];
		const string strIconSlot = "Skill_" + strKey + "_Icon";
		const string strCooldownSlot = "Skill_" + strKey + "_Cooldown";

		const int32_t iSkillIndex = kouku.ModeSkillIndexBySlot[i];
		// 카드 미로의 mouse 망치는 입력 동작으로 남고 HUD의 표시 가능 슬롯은 Has_VisibleSkillSlot이 제한한다.
		const bool_t bHasSkill =
			kouku.Has_VisibleSkillSlot(i) && static_cast<size_t>(iSkillIndex) < pMode->Skills.size();
		if (bHasSkill)
		{
			m_pHUDRuntimeView->Set_SlotTexture(strIconSlot,
											   pMode->Skills[static_cast<size_t>(iSkillIndex)].strIconPath);
		}
		// 해당 칸에 표시할 스킬이 없으면 아이콘을 숨겨 기본 어두운 슬롯만 남긴다.
		m_pHUDRuntimeView->Set_SlotVisible(strIconSlot, bHasSkill);

		const uint32_t iEndTick = kouku.CooldownEndTicks[i];
		const uint32_t iRemaining = (bHasSkill && iEndTick > player.iServerTick) ? iEndTick - player.iServerTick : 0u;
		if (0u == iRemaining)
		{
			m_pHUDRuntimeView->Set_SlotArcRatio(strCooldownSlot, 0.f);
			m_pHUDRuntimeView->Set_SlotVisible(strCooldownSlot, false);
			continue;
		}
		const f32_t fRemainingSeconds = static_cast<f32_t>(iRemaining) / SERVER_TICK_HZ;
		const f32_t fTotalSeconds = kouku.CooldownDurationTicks[i] > 0u
										? static_cast<f32_t>(kouku.CooldownDurationTicks[i]) / SERVER_TICK_HZ
										: fRemainingSeconds;
		const f32_t fFraction =
			fTotalSeconds > 0.f ? (std::min)(1.f, (std::max)(0.f, fRemainingSeconds / fTotalSeconds)) : 0.f;
		m_pHUDRuntimeView->Set_SlotTint(strCooldownSlot, vCooldownTint);
		m_pHUDRuntimeView->Set_SlotArcRatio(strCooldownSlot, fFraction);
		m_pHUDRuntimeView->Set_SlotVisible(strCooldownSlot, true);
	}
}
// 일반 UI가 허용된 Update에서 로비의 버튼 입력·서버 선택·로고 표시를 갱신한다.
// 서버 목록은 표시용 값이며 이 함수가 네트워크 endpoint를 교체하거나 접속을 시작하지는 않는다.
void CMainApp::Update_LobbyButtons(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.LobbyButtons.Update");
	if (nullptr == m_pLobbyBackgroundView)
		return;
	if (ETOUI(LEVEL::LOBBY) != CGameInstance::Get().Get_CurrentLevelID())
	{
		m_pLobbyBackgroundView->Set_AllSlotsVisible(false);
		m_bLobbyWasActive = false;
		return;
	}

	// 캐릭터 선택 창이 열려 있으면 로비 배경·버튼을 숨기고 입력 조회를 생략한다.
	// 선택 창의 입력과 화면 갱신은 뒤에 호출되는 Update_CharacterSelectWindow가 맡는다.
	if (nullptr != m_pCharacterSelectWindowView && m_pCharacterSelectWindowView->Is_Open())
	{
		m_pLobbyBackgroundView->Set_AllSlotsVisible(false);
		return;
	}

	// 로비가 비활성이었다가 다시 갱신될 때 로고 등장 시간을 초기화한다.
	// 선택 창을 닫는 것만으로는 m_bLobbyWasActive를 초기화하지 않으므로 로고를 다시 시작하지 않는다.
	if (!m_bLobbyWasActive)
	{
		m_bLobbyWasActive = true;
		m_fLobbyLogoIntroElapsed = 0.f;
		m_pLobbyBackgroundView->Restart_Animation("Lobby_LogoIntro");
	}
	m_fLobbyLogoIntroElapsed += fTimeDelta;
	const bool_t bLogoIntroDone = m_fLobbyLogoIntroElapsed >= LOBBY_LOGO_INTRO_SECONDS;

	m_pLobbyBackgroundView->Set_AllSlotsVisible(true);
	m_pLobbyBackgroundView->Set_SlotVisible("Lobby_LogoIntro", !bLogoIntroDone);
	m_pLobbyBackgroundView->Set_SlotVisible("Lobby_Logo", bLogoIntroDone);

	CUIInputRouter &Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pLobbyBackgroundView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pLobbyBackgroundView->Get_ResolutionHeight();
	// view 기준 해상도의 rect를 router에 전달한다. 포인터가 올라오면 이번 프레임 마우스를 점유하고 클릭 여부를 별도로 반환한다.
	const auto HoverAndClick = [&](const LOBBY_PRODUCT_RECT &Rect, bool_t &outClicked)
	{
		outClicked = false;
		const bool_t bHovered = Router.Is_Hovered(Rect.fX, Rect.fY, Rect.fWidth, Rect.fHeight, fRefWidth, fRefHeight);
		if (bHovered)
		{
			Router.Claim_Mouse_This_Frame();
			outClicked = Router.Is_Clicked(Rect.fX, Rect.fY, Rect.fWidth, Rect.fHeight, fRefWidth, fRefHeight);
		}
		return bHovered;
	};

	// 저작된 서버 목록 크기와 표시 행 상한 중 작은 개수만 표시한다. 선택 행·포인터 행의 문양을 구분하고 나머지 행은 숨긴다.
	const size_t iRowCount = (std::min)(m_LobbyServers.size(), LOBBY_SERVER_ROW_COUNT);
	if (m_iLobbySelectedServer >= static_cast<int32_t>(iRowCount))
		m_iLobbySelectedServer = iRowCount > 0 ? 0 : -1;
	for (size_t i = 0; i < LOBBY_SERVER_ROW_COUNT; ++i)
	{
		const string strRowId = "Lobby_ServerRow_" + std::to_string(i);
		const string strCountId = "Lobby_ServerRowCount_" + std::to_string(i);
		if (i >= iRowCount)
		{
			m_pLobbyBackgroundView->Set_SlotVisible(strRowId, false);
			m_pLobbyBackgroundView->Set_SlotVisible(strCountId, false);
			continue;
		}
		LOBBY_PRODUCT_RECT Rect{};
		bool_t bClicked = false;
		const bool_t bHovered =
			Get_LobbySlotRect(m_pLobbyBackgroundView.get(), strRowId.c_str(), Rect) && HoverAndClick(Rect, bClicked);
		if (bClicked)
		{
			m_iLobbySelectedServer = static_cast<int32_t>(i);
			Play_UISound(UI_SOUND_CUE::BUTTON_CLICK);
		}
		const bool_t bSelected = static_cast<int32_t>(i) == m_iLobbySelectedServer;
		m_pLobbyBackgroundView->Set_SlotTexture(strRowId, bSelected	 ? "UI/Lobby/ServerSelect/row_selected.png"
														  : bHovered ? "UI/Lobby/ServerSelect/row_over.png"
																	 : "");
		// 생성 가능한 서버 행에는 인원 아이콘을, 불가능한 행에는 텍스트 단계에서 "생성 불가" 상태를 표시한다.
		m_pLobbyBackgroundView->Set_SlotVisible(strCountId, m_LobbyServers[i].bCreatable);
	}

	// 접속(서버 선택)은 선택 창만 연다. 선택 창의 intent를 받아 실제 월드 입장 명령을 제출하는 곳은 Update_CharacterSelectWindow다.
	{
		LOBBY_PRODUCT_RECT Rect{};
		bool_t bClicked = false;
		const bool_t bHovered =
			Get_LobbySlotRect(m_pLobbyBackgroundView.get(), "Lobby_JoinButton", Rect) && HoverAndClick(Rect, bClicked);
		const bool_t bEnabled = m_iLobbySelectedServer >= 0 && nullptr != m_pCharacterSelectWindowView &&
								CLevel_Lobby::Can_SubmitProductCommand();
		m_pLobbyBackgroundView->Set_SlotTexture("Lobby_JoinButton",
												bHovered && bEnabled ? "UI/Lobby/ServerSelect/btn_join_over.png" : "");
		if (bClicked && bEnabled)
		{
			m_pCharacterSelectWindowView->Open();
			Play_UISound(UI_SOUND_CUE::BUTTON_CLICK);
		}
	}

	// 종료 아이콘은 WM_CLOSE를 보내 기존 창 종료 흐름을 따른다. 로비의 환경설정 아이콘은 현재 hover 그림만 바꾼다.
	struct LOBBY_ICON_BUTTON
	{
		const char *pSlotId;
		const char *pOverTexture;
		bool_t bExit;
	};
	constexpr LOBBY_ICON_BUTTON IconButtons[2] = {
		{"Lobby_ExitIcon", "UI/Lobby/ServerSelect/btn_exit_over.png", true},
		{"Lobby_OptionIcon", "UI/Lobby/ServerSelect/btn_option_over.png", false},
	};
	for (const LOBBY_ICON_BUTTON &Icon : IconButtons)
	{
		LOBBY_PRODUCT_RECT Rect{};
		if (!Get_LobbySlotRect(m_pLobbyBackgroundView.get(), Icon.pSlotId, Rect))
			continue;
		// 아이콘보다 넓은 입력 사각형을 별도로 만든다. 현재 폭은 원래 폭+52, 높이는 54이며 위치를 왼쪽26·위6만큼 옮긴다.
		const LOBBY_PRODUCT_RECT HitRect{Rect.fX - 26.f, Rect.fY - 6.f, Rect.fWidth + 52.f, 54.f};
		bool_t bClicked = false;
		const bool_t bHovered = HoverAndClick(HitRect, bClicked);
		m_pLobbyBackgroundView->Set_SlotTexture(Icon.pSlotId, bHovered ? Icon.pOverTexture : "");
		if (bClicked && Icon.bExit)
		{
			Play_UISound(UI_SOUND_CUE::BUTTON_CLICK);
			PostMessage(g_hWnd, WM_CLOSE, 0, 0);
		}
	}

	// 로비 배경 반복 flipbook과 로고 등장 animation의 표시 시간을 진행한다.
	m_pLobbyBackgroundView->Update(fTimeDelta);
}

// 미니맵·일반 HUD가 쿠크 미로 mode의 숨김 여부를 조회한다. 다른 Level이나 유효하지 않은 gimmick은 숨기지 않는다.
bool_t CMainApp::Is_KoukuMinigameHUDHidden() const
{
	if (ETOUI(LEVEL::KOUKUSAYDON_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
		return false;
	const auto gimmick = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	return gimmick.isValid && gimmick.eHudMode == HUD_KOUKU_HUD_MODE::MAZE;
}

// 보스 체력바는 미로와 댄스 두 mode에서 숨긴다. 일반 HUD의 미로 전용 조건과 별도로 유지한다.
bool_t CMainApp::Is_KoukuBossHealthBarHidden() const
{
	if (ETOUI(LEVEL::KOUKUSAYDON_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
		return false;
	const auto gimmick = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	return gimmick.isValid &&
		   (gimmick.eHudMode == HUD_KOUKU_HUD_MODE::MAZE || gimmick.eHudMode == HUD_KOUKU_HUD_MODE::DANCE);
}

// 일반 제품 UI의 Update/Render가 cinematic 숨김 또는 Loading 여부를 함께 확인한다.
bool_t CMainApp::Is_RuntimeUIScreenSuppressed() const
{
	return CUIInputRouter::Get().Is_CinematicSuppressed() ||
		   ETOUI(LEVEL::LOADING) == CGameInstance::Get().Get_CurrentLevelID();
}

// Loading 진입을 관찰한 첫 Update가 열려 있던 창과 임시 입력 상태를 정리한다.
// 각 view의 Close/Hide 정책을 호출하고 Escape 순서 목록을 비우며 view 소유 포인터는 유지한다.
void CMainApp::Close_RuntimeWindowsForLoading()
{
	// Loading 첫 Update에서 일반 창의 갱신을 생략하기 전에 호출한다. STATIC sprite는 Level 교체 뒤에도 남으므로 명시적으로 숨긴다.
	// 이 함수에 나열된 view의 닫기·숨김과 운반 취소를 수행하며, 모든 UI 객체를 파괴하는 정리는 아니다.
	if (nullptr != m_pLobbyBackgroundView)
		m_pLobbyBackgroundView->Set_AllSlotsVisible(false);
	m_bLobbyWasActive = false;
	if (nullptr != m_pCharacterSelectWindowView)
		m_pCharacterSelectWindowView->Close();
	if (nullptr != m_pInventoryView)
		m_pInventoryView->Close();
	if (nullptr != m_pRepairWindowView)
		m_pRepairWindowView->Close();
	if (nullptr != m_pShopWindowView)
		m_pShopWindowView->Close();
	// 내구도 HUD는 창은 아니지만 STATIC sprite가 로딩 화면에 남지 않게 숨긴다.
	if (nullptr != m_pDurabilityHudView)
		m_pDurabilityHudView->Hide();
	if (nullptr != m_pCharacterInfoView)
		m_pCharacterInfoView->Close();
	if (nullptr != m_pAvatarBookView)
		m_pAvatarBookView->Close();
	if (nullptr != m_pVehicleWindowView)
		m_pVehicleWindowView->Close();
	if (nullptr != m_pHonorTitleWindowView)
		m_pHonorTitleWindowView->Close();
	if (nullptr != m_pWorldMapWindowView)
		m_pWorldMapWindowView->Close();
	if (nullptr != m_pSystemOptionView)
		m_pSystemOptionView->Close();
	if (nullptr != m_pQuickSlotDragView)
		m_pQuickSlotDragView->Cancel();
	if (nullptr != m_pChatWindowView)
	{
		m_pChatWindowView->Close_Input();
		m_pChatWindowView->Hide_AllSlots();
	}
	m_iEscapeWindowCount = 0u;
}

// 환경설정 창을 새로 열기 전에 다른 제품 화면이 Escape를 사용할 가능성이 있는지 조회한다.
bool_t CMainApp::Is_AnyRuntimeWindowOpen() const
{
	// 환경설정 창 외에 열린 제품 창·미리보기·결과 화면이 있는지 조회한다.
	// 이 목록과 아래 ESCAPE_WINDOW 순서 배열은 범위가 다르며 환경설정 창 자체의 Escape는 호출자가 먼저 처리한다.
	const bool_t bOpen = (nullptr != m_pInventoryView && m_pInventoryView->Is_Open()) ||
						 (nullptr != m_pRepairWindowView && m_pRepairWindowView->Is_Open()) ||
						 (nullptr != m_pShopWindowView && m_pShopWindowView->Is_Open()) ||
						 (nullptr != m_pCharacterInfoView && m_pCharacterInfoView->Is_Open()) ||
						 (nullptr != m_pAvatarBookView && m_pAvatarBookView->Is_Open()) ||
						 (nullptr != m_pVehicleWindowView && m_pVehicleWindowView->Is_Open()) ||
						 (nullptr != m_pHonorTitleWindowView && m_pHonorTitleWindowView->Is_Open()) ||
						 (nullptr != m_pWorldMapWindowView && m_pWorldMapWindowView->Is_Open()) ||
						 (nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open()) ||
						 (nullptr != m_pChatWindowView && m_pChatWindowView->Is_Open()) ||
						 (nullptr != m_pCharacterSelectWindowView && m_pCharacterSelectWindowView->Is_Open()) ||
						 m_bItemUpgradePreviewVisible || Is_MvpResultPageOpen();
	return bOpen;
}

// Escape 순서 배열이 관리하는 여덟 종류만 조회한다. 지원하지 않는 enum은 닫힌 것으로 반환한다.
bool_t CMainApp::Is_EscapeWindowOpen(const ESCAPE_WINDOW eWindow) const
{
	switch (eWindow)
	{
	case ESCAPE_WINDOW::INVENTORY:
		return nullptr != m_pInventoryView && m_pInventoryView->Is_Open();
	case ESCAPE_WINDOW::REPAIR:
		return nullptr != m_pRepairWindowView && m_pRepairWindowView->Is_Open();
	case ESCAPE_WINDOW::SHOP:
		return nullptr != m_pShopWindowView && m_pShopWindowView->Is_Open();
	case ESCAPE_WINDOW::CHARACTER_INFO:
		return nullptr != m_pCharacterInfoView && m_pCharacterInfoView->Is_Open();
	case ESCAPE_WINDOW::AVATAR_BOOK:
		return nullptr != m_pAvatarBookView && m_pAvatarBookView->Is_Open();
	case ESCAPE_WINDOW::HONOR_TITLE:
		return nullptr != m_pHonorTitleWindowView && m_pHonorTitleWindowView->Is_Open();
	case ESCAPE_WINDOW::VEHICLE:
		return nullptr != m_pVehicleWindowView && m_pVehicleWindowView->Is_Open();
	case ESCAPE_WINDOW::WORLD_MAP:
		return nullptr != m_pWorldMapWindowView && m_pWorldMapWindowView->Is_Open();
	default:
		return false;
	}
}

// 닫힌 항목을 제거하고 새로 관찰한 창을 뒤에 추가한다. 클릭에 의한 시각적 포커스 순서를 추적하는 배열은 아니다.
void CMainApp::Sync_EscapeWindowOrder()
{
	uint32_t iKept = 0u;
	for (uint32_t i = 0u; i < m_iEscapeWindowCount; ++i)
	{
		if (Is_EscapeWindowOpen(m_EscapeWindowOrder[i]))
			m_EscapeWindowOrder[iKept++] = m_EscapeWindowOrder[i];
	}
	m_iEscapeWindowCount = iKept;
	// 같은 갱신에서 처음 발견한 창은 enum 순서대로 뒤에 추가한다. 기존에 열린 창의 상대 순서는 유지한다.
	for (uint32_t iWindow = 0u; iWindow < ETOUI(ESCAPE_WINDOW::END); ++iWindow)
	{
		const ESCAPE_WINDOW eWindow = static_cast<ESCAPE_WINDOW>(iWindow);
		if (!Is_EscapeWindowOpen(eWindow))
			continue;
		bool_t bListed = false;
		for (uint32_t i = 0u; i < m_iEscapeWindowCount && !bListed; ++i)
			bListed = m_EscapeWindowOrder[i] == eWindow;
		if (!bListed)
			m_EscapeWindowOrder[m_iEscapeWindowCount++] = eWindow;
	}
}

// 정리한 순서 배열의 마지막 창에 Escape를 전달한다. true는 입력을 맡겼다는 뜻이며 내부 dialog만 닫혀 창이 남을 수도 있다.
bool_t CMainApp::Close_TopEscapeWindow()
{
	Sync_EscapeWindowOrder();
	if (0u == m_iEscapeWindowCount)
		return false;
	const ESCAPE_WINDOW eTop = m_EscapeWindowOrder[--m_iEscapeWindowCount];
	switch (eTop)
	{
	case ESCAPE_WINDOW::INVENTORY:
		m_pInventoryView->Close();
		break;
	case ESCAPE_WINDOW::REPAIR:
		m_pRepairWindowView->Close();
		break;
	case ESCAPE_WINDOW::SHOP:
		m_pShopWindowView->Close();
		break;
	case ESCAPE_WINDOW::CHARACTER_INFO:
		m_pCharacterInfoView->Close();
		break;
	case ESCAPE_WINDOW::AVATAR_BOOK:
		m_pAvatarBookView->Close();
		break;
	case ESCAPE_WINDOW::HONOR_TITLE:
		m_pHonorTitleWindowView->Close();
		break;
	case ESCAPE_WINDOW::VEHICLE:
		m_pVehicleWindowView->Close();
		break;
	case ESCAPE_WINDOW::WORLD_MAP:
		// 월드맵 내부의 이동 확인 창이 먼저 Escape를 받는다. 월드맵이 여전히 열려 있으면 순서 배열의 맨 위 자리를 유지한다.
		m_pWorldMapWindowView->Handle_EscapeEdge();
		if (m_pWorldMapWindowView->Is_Open())
			++m_iEscapeWindowCount;
		break;
	default:
		break;
	}
	return true;
}

// 섬 이동·물건 운반·레이드 입장 팝업이 Escape를 별도로 처리하는지 읽는다. 여기서는 대상 상태를 변경하지 않는다.
bool_t CMainApp::Is_EscapeOwnedElsewhere() const
{
	if (const auto *island = CLevel_Development::Get_Active(LEVEL::MAHARAKA); island && island->Is_TravelModalOpen())
		return true;
	// 운반 중인 값과 입장 팝업은 각자의 입력 처리에서 Escape를 소비하므로 일반 창을 함께 닫지 않는다.
	if (nullptr != m_pQuickSlotDragView && m_pQuickSlotDragView->Is_Carrying())
		return true;
	if (CLevel_Bern *pBern = CLevel_Bern::Get_Active();
		nullptr != pBern && ETOUI(LEVEL::BERN) == CGameInstance::Get().Get_CurrentLevelID() &&
		pBern->Is_ValtanEntryModalOpen())
		return true;
#ifdef _DEBUG
	if (CLevel_CharacterSelect *pCharacterSelect = CLevel_CharacterSelect::Get_Active();
		nullptr != pCharacterSelect && ETOUI(LEVEL::CHARACTER_SELECT) == CGameInstance::Get().Get_CurrentLevelID() &&
		pCharacterSelect->Is_DebugRaidEntryPreviewOpen())
		return true;
#endif
	return false;
}

// Update의 제품 창 순서에서 먼저 Escape를 배분하고 환경설정 자체의 입력·화면을 갱신한다.
// 텍스트 입력 중에는 이 키 추적을 갱신하지 않으며, foreground가 같은 process인지도 검사한다.
void CMainApp::Update_SystemOptionWindow(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.SystemOptionWindow.Update");
	if (nullptr == m_pSystemOptionView)
		return;

	// 다른 창의 이번 프레임 Update 전에 열린 상태를 읽고 Escape의 눌림 전환을 한 곳에서 판정한다.
	// 환경설정→결과 화면→강화 화면→별도 입력 소유자→순서 배열의 창 순으로 처리하며, 다른 창이 없을 때만 환경설정을 연다.
	const bool_t bOtherWindowOpen = Is_AnyRuntimeWindowOpen();
	Sync_EscapeWindowOrder();
	if (!ImGui::GetIO().WantTextInput && !CUIInputRouter::Get().Is_TextInputActive())
	{
		const bool_t bWindowFocused = IsWindowOwnedByCurrentProcess(GetForegroundWindow());
		const bool_t bKeyDown = bWindowFocused && 0 != (GetAsyncKeyState(VK_ESCAPE) & 0x8000);
		if (bKeyDown && !m_bSystemOptionKeyDown)
		{
			if (m_pSystemOptionView->Is_Open())
				m_pSystemOptionView->Handle_EscapeEdge();
			else if (Is_MvpResultPageOpen())
			{
				// 결과 화면이 열려 있으면 해당 아레나의 결과 표시만 닫는다. 같은 Escape로 환경설정을 여는 분기는 실행하지 않는다.
				if (CLevel_KoukuSaydonArena *pArena = CLevel_KoukuSaydonArena::Get_Active())
					pArena->Debug_Hide_MvpResult();
				if (CLevel_ValtanArena *pValtan = CLevel_ValtanArena::Get_Active())
					pValtan->Hide_MvpResult();
			}
			else if (m_bItemUpgradePreviewVisible)
			{
				m_bItemUpgradePreviewVisible = false;
				Hide_ItemUpgrade();
			}
			else if (Is_EscapeOwnedElsewhere())
			{
				// 운반·팝업 쪽이 이 입력을 처리하므로 여기서는 추가 창 동작을 하지 않는다.
			}
			else if (Close_TopEscapeWindow())
			{
				// 이번 눌림은 순서 배열에서 마지막으로 등록된 열린 창 하나에만 전달했다.
			}
			else if (!bOtherWindowOpen)
			{
				m_pSystemOptionView->Open();
				Play_UISound(UI_SOUND_CUE::BUTTON_CLICK);
			}
		}
		m_bSystemOptionKeyDown = bKeyDown;
	}

	m_pSystemOptionView->Update(fTimeDelta);

	// 화면 설정 변경 표시는 Take_VideoDirty에서 한 번 소비한다. 현재 활성 렌더링 프로필을 다시 적용해 설정을 반영한다.
	// 활성 ID가 없으면 별도 적용하지 않고, 재적용 실패는 로그로 남기며 이 함수에서 변경을 재시도하지 않는다.
	if (m_pSystemOptionView->Take_VideoDirty())
	{
		const string strActiveProfileId = m_RenderingProfiles.Get_ActiveProfileId();
		string strStatus;
		if (!strActiveProfileId.empty() && !m_RenderingProfiles.Activate_Profile(strActiveProfileId, strStatus))
		{
			OutputDebugStringA(("[MainApp] System option video re-apply failed: " + strStatus + "\n").c_str());
		}
	}
}

// 로비의 선택창 intent를 기존 identity stage와 Lobby typed 명령으로 연결한다.
// 선택창은 roster 표시·입력을 소유하고 MainApp은 Level 및 다른 창과의 전환 순서를 조율한다.
void CMainApp::Update_CharacterSelectWindow(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.CharacterSelectWindow.Update");
	if (nullptr == m_pCharacterSelectWindowView)
		return;
	if (ETOUI(LEVEL::LOBBY) != CGameInstance::Get().Get_CurrentLevelID())
	{
		// 로비 밖에서는 STATIC 선택창을 닫는다. 입장 준비가 이 창을 차용 중이면 stage 해제를 미루고, 아니면 현재 stage를 정리한다.
		if (m_pCharacterSelectWindowView->Is_Open())
			m_pCharacterSelectWindowView->Close();
		if (!m_pCharacterSelectWindowView->Is_PreparingEntry())
			m_pCharacterSelectWindowView->Release_Stage();
		return;
	}

	// 월드에서 선택창 복귀를 요청한 상태라면 로비 명령 제출이 가능한 시점에 창을 열고 예약 플래그를 소비한다.
	if (m_bOpenCharacterSelectOnLobby && !m_pCharacterSelectWindowView->Is_Open() &&
		CLevel_Lobby::Can_SubmitProductCommand())
	{
		m_pCharacterSelectWindowView->Open();
		m_bOpenCharacterSelectOnLobby = false;
	}
	// 환경설정이 열려 있으면 선택창은 표시를 유지하되 입력 차단 인자를 받아 포인터·Escape 처리를 넘긴다.
	m_pCharacterSelectWindowView->Update(fTimeDelta, nullptr != m_pSystemOptionView && m_pSystemOptionView->Is_Open());

	switch (m_pCharacterSelectWindowView->Consume_Intent())
	{
	case CCharacterSelectWindowView::INTENT::START_CHARACTER:
	{
		// 현재 roster의 ID·class·nickname·외형과 일치하는 기존 캐릭터 입장을 pending으로 보관한 뒤 Bern 입장을 요청한다.
		// 새 캐릭터 생성으로 기록하지 않으며, 명령 제출 실패 시 pending만 취소한다. 실제 입장 확정은 이후 전환 흐름이 맡는다.
		LostArk::Shared::CHARACTER_CLASS_ID eClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
		string strNickname;
		string strAppearance;
		string strCharacterId;
		m_pCharacterSelectWindowView->Get_StartCharacter(eClass, strNickname, strAppearance, strCharacterId);
		if (CCharacterSelectionState::Stage_ExistingEntry(eClass, strNickname, strAppearance, strCharacterId) &&
			!CLevel_Lobby::Submit_ProductCommand(LOBBY_STAGE::BERN))
			CCharacterSelectionState::Cancel_PendingCreation();
		break;
	}
	case CCharacterSelectWindowView::INTENT::OPEN_OPTIONS:
		if (nullptr != m_pSystemOptionView)
			m_pSystemOptionView->Open();
		break;
	case CCharacterSelectWindowView::INTENT::NEW_CHARACTER:
		// 빈 생성 슬롯이 아직 유효할 때 Character Select 입장 명령을 제출한다.
		// 승인 대기에는 선택창을 유지하며 Level이 바뀌면 함수 앞의 로비 밖 분기가 창을 닫는다.
		if (CCharacterSelectionState::Select_CreationSlot(
				static_cast<size_t>(m_pCharacterSelectWindowView->Get_CreationSlot())))
			(void)CLevel_Lobby::Submit_ProductCommand(LOBBY_STAGE::CHARACTER_SELECT);
		break;
	case CCharacterSelectWindowView::INTENT::CLOSE:
		m_pCharacterSelectWindowView->Close();
		break;
	default:
		break;
	}
}

// Render의 텍스트 단계에서 로비 선택창에 글자 출력을 위임한다. 열린 상태와 실제 표시 자료는 view가 검사한다.
void CMainApp::RenderCharacterSelectWindowText()
{
	if (nullptr == m_pCharacterSelectWindowView)
		return;
	if (ETOUI(LEVEL::LOBBY) != CGameInstance::Get().Get_CurrentLevelID())
		return;
	m_pCharacterSelectWindowView->RenderText();
}

// 지원하는 네 Level의 marker snapshot 하나로 미니맵과 월드맵을 갱신한다.
// 지역 값의 포인터는 이 호출 동안만 전달하며, 월드맵 요청과 Bern 시전 게이지 갱신도 같은 UI 순서에서 처리한다.
void CMainApp::Update_Minimap(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.Minimap.Update");
	if (nullptr == m_pMinimapView)
		return;
	// 현재 지원 Level의 replication 소유자에게 지역 marker snapshot 값을 수집한다.
	// 이 함수는 복제 객체를 교체하거나 마커 위치를 Server로 보내지 않는다.
	const uint32_t iLevel = CGameInstance::Get().Get_CurrentLevelID();
	CClientReplication::MINIMAP_MARKER_SNAPSHOT Snapshot{};
	LEVEL eLevel = LEVEL::END;
	bool_t bHasSnapshot = false;
	if (ETOUI(LEVEL::BERN) == iLevel)
	{
		if (CLevel_Bern *pBern = CLevel_Bern::Get_Active())
		{
			pBern->Collect_MinimapMarkers(Snapshot);
			eLevel = LEVEL::BERN;
			bHasSnapshot = true;
		}
	}
	else if (ETOUI(LEVEL::VALTAN_ARENA) == iLevel)
	{
		if (CLevel_ValtanArena *pValtanArena = CLevel_ValtanArena::Get_Active())
		{
			pValtanArena->Collect_MinimapMarkers(Snapshot);
			eLevel = LEVEL::VALTAN_ARENA;
			bHasSnapshot = true;
		}
	}
	else if (ETOUI(LEVEL::KOUKUSAYDON_ARENA) == iLevel)
	{
		if (CLevel_KoukuSaydonArena *pKoukuSaydonArena = CLevel_KoukuSaydonArena::Get_Active())
		{
			pKoukuSaydonArena->Collect_MinimapMarkers(Snapshot);
			eLevel = LEVEL::KOUKUSAYDON_ARENA;
			bHasSnapshot = true;
		}
	}
	else if (ETOUI(LEVEL::COLOSSEUM) == iLevel)
	{
		if (CLevel_Development *pColosseum = CLevel_Development::Get_Active(LEVEL::COLOSSEUM))
		{
			pColosseum->Collect_MinimapMarkers(Snapshot);
			eLevel = LEVEL::COLOSSEUM;
			bHasSnapshot = true;
		}
	}
	// 결과 화면이나 쿠크 미로 mode에서는 snapshot 전달을 막는다. MinimapView는 nullptr 입력이면 모든 슬롯을 숨긴다.
	if (Is_MvpResultPageOpen() || Is_KoukuMinigameHUDHidden())
		bHasSnapshot = false;
	m_pMinimapView->Update(fTimeDelta, eLevel, bHasSnapshot ? &Snapshot : nullptr);
	// 같은 지역 snapshot을 월드맵에도 전달한다. 유효 지역·로컬 위치가 없으면 월드맵은 자기 표시를 숨긴다.
	if (nullptr != m_pWorldMapWindowView)
	{
		m_pWorldMapWindowView->Update(fTimeDelta, eLevel, bHasSnapshot ? &Snapshot : nullptr);
		// 월드맵이 보관한 이동 intent를 한 번 꺼내 현재 controller의 typed 요청으로 넘긴다.
		// controller 부재·입력 거부·전송 실패는 로그로만 남기며, 실제 이동과 시전 상태 확정은 Server가 맡는다.
		uint16_t iHoleId = 0u;
		if (m_pWorldMapWindowView->Take_SquareHoleRequest(iHoleId))
		{
			CPlayerController *pController = Find_ActivePlayerController();
			// 콜로세움 종료 후 참가자 복귀인지 복제된 match·local entity로 확인한다. 이 값은 controller의 현재 action 제한 예외에 쓰인다.
			bool_t finishedColosseumReturn = false;
			if (const auto arena = CLevel_Development::Get_Active(LEVEL::COLOSSEUM))
			{
				const auto &match = arena->Get_Replication().Get_ColosseumMatchState();
				const auto local = CNetworkManager::Get().Get_LocalEntityId();
				finishedColosseumReturn =
					match.iMatchId != 0u && match.ePhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::FINISHED &&
					std::any_of(match.Players.begin(), match.Players.end(), [local](const auto &row)
								{ return row.iNetEntityId == local && row.bParticipant && row.iTeam < 2u; });
			}
			if (nullptr == pController || !pController->Request_UseSquareHole(iHoleId, finishedColosseumReturn))
				OutputDebugStringA(
					"[Client][WorldMapWindow] Square hole request not sent (no controller, or the player is busy).\n");
		}
		// 선박 출발도 별도 local 이동 대신 예약 destination ID를 같은 SquareHole 요청으로 보낸다.
		if (m_pWorldMapWindowView->Take_ShipTravelRequest())
		{
			CPlayerController *pController = Find_ActivePlayerController();
			if (nullptr == pController ||
				!pController->Request_UseSquareHole(LostArk::Shared::WORLD_MAP_SHIP_TRAVEL_DESTINATION_ID))
			{
				OutputDebugStringA(
					"[Client][WorldMapWindow] Ship travel request not sent (no controller, or the player is busy).\n");
			}
		}
	}
	if (nullptr != m_pSongCastGaugeView)
		m_pSongCastGaugeView->Update(fTimeDelta, CCombatHUDViewModel::Get().Get_Player(),
									 ETOUI(LEVEL::BERN) == CGameInstance::Get().Get_CurrentLevelID());
}

// Render의 텍스트 단계가 미니맵 view에 출력을 위임한다. 대상 view가 없으면 건너뛴다.
void CMainApp::RenderMinimapText()
{
	if (nullptr != m_pMinimapView)
		m_pMinimapView->RenderText();
}

// Render의 텍스트 단계에서 서버 목록과 로비 버튼의 글자를 출력한다.
// 이미지 슬롯의 기준 rect를 조회하고 현재 viewport에 맞춰 위치·배율을 계산한다.
void CMainApp::RenderLobbyButtonText()
{
	if (ETOUI(LEVEL::LOBBY) != CGameInstance::Get().Get_CurrentLevelID())
		return;
	if (nullptr == m_pLobbyBackgroundView)
		return;
	// 캐릭터 선택 창이 열렸으면 로비 sprite와 함께 서버 목록의 글자도 그리지 않는다.
	if (nullptr != m_pCharacterSelectWindowView && m_pCharacterSelectWindowView->Is_Open())
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	// 위치는 1280×720 기준 좌표에서 viewport 축별로 변환하고 글자 크기는 두 축 중 작은 배율을 사용한다.
	// fAlign은 0=왼쪽, 0.5=가운데, 1=오른쪽 기준이며, 최대 폭을 넘으면 글자 배율을 줄인다.
	const auto DrawAt = [&](const wchar_t *pText, const wstring &strFont, f32_t fFontPx, f32_t fX, f32_t fCenterY,
							f32_t fAlign, f32_t fMaxWidth, fvector_t vColor)
	{
		if (nullptr == pText || 0 == pText[0])
			return;
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFont, pText);
		f32_t fScale = (vMeasured.y > 0.f) ? (fFontPx / vMeasured.y) : 1.f;
		if (fMaxWidth > 0.f && vMeasured.x * fScale > fMaxWidth)
			fScale = fMaxWidth / vMeasured.x;
		CGameInstance::Get().Draw_Text(strFont, pText, float2_t(fX * textScaleX, fCenterY * textScaleY), vColor, 0.f,
									   float2_t(fAlign, 0.5f), fScale * textUiScale);
	};
	// 슬롯의 rect가 없는 글자는 생략한다. 위치·최대 폭은 해당 슬롯을 사용하고 가운데 정렬로 그린다.
	const auto DrawInSlot =
		[&](const char *pSlotId, const wchar_t *pText, const wstring &strFont, f32_t fFontPx, fvector_t vColor)
	{
		LOBBY_PRODUCT_RECT Rect{};
		if (!Get_LobbySlotRect(m_pLobbyBackgroundView.get(), pSlotId, Rect))
			return;
		DrawAt(pText, strFont, fFontPx, Rect.fX + Rect.fWidth * 0.5f, Rect.fY + Rect.fHeight * 0.5f, 0.5f, Rect.fWidth,
			   vColor);
	};
	const auto Rgb = [](uint32_t iHex)
	{ return XMVectorSet(((iHex >> 16) & 0xFF) / 255.f, ((iHex >> 8) & 0xFF) / 255.f, (iHex & 0xFF) / 255.f, 1.f); };
	const wstring strYoon = TEXT("Font_YoonGasiIIM");
	const wstring strYG760 = TEXT("Font_YG760");

	// 서버 / 상태 / 캐릭터 제목은 각 저작 텍스트 슬롯의 가운데에 맞춘다.
	DrawInSlot("Lobby_ServerTitleText", L"\xC11C\xBC84 \xC120\xD0DD", strYoon, 20.f, Rgb(0xfff7e2));
	DrawInSlot("Lobby_ServerHeaderName", L"\xC11C\xBC84", strYoon, 18.f, Rgb(0xd5ac66));
	DrawInSlot("Lobby_ServerHeaderState", L"\xC0C1\xD0DC", strYoon, 18.f, Rgb(0xd5ac66));
	DrawInSlot("Lobby_ServerHeaderChar", L"\xCE90\xB9AD\xD130", strYoon, 18.f, Rgb(0xd5ac66));

	// 서버 행 rect에 tag·이름·상태를 배치하고 생성 가능 여부에 따라 인원수 또는 생성 불가 문구를 고른다.
	// 신규·추천 tag와 원활·보통·혼잡 상태는 각각 아래 색을 사용한다.
	const size_t iRowCount = (std::min)(m_LobbyServers.size(), LOBBY_SERVER_ROW_COUNT);
	for (size_t i = 0; i < iRowCount; ++i)
	{
		const LOBBY_SERVER_ENTRY &Entry = m_LobbyServers[i];
		LOBBY_PRODUCT_RECT Rect{};
		if (!Get_LobbySlotRect(m_pLobbyBackgroundView.get(), ("Lobby_ServerRow_" + std::to_string(i)).c_str(), Rect))
			continue;
		const f32_t fCenterY = Rect.fY + 12.333f;
		if (!Entry.strTag.empty())
		{
			const fvector_t vTagColor = Entry.strTag == L"\xC2E0\xADDC"	  ? Rgb(0xffd43e)
										: Entry.strTag == L"\xCD94\xCC9C" ? Rgb(0x00aeff)
																		  : Rgb(0x22facf);
			DrawAt(Entry.strTag.c_str(), strYoon, 16.f, Rect.fX + 6.667f, fCenterY, 0.f, 52.f, vTagColor);
		}
		DrawAt(Entry.strName.c_str(), strYG760, 16.f, Rect.fX + 41.333f + 79.333f, fCenterY, 0.5f, 158.667f,
			   Colors::White);
		const fvector_t vStateColor = Entry.strState == L"\xC6D0\xD65C"	  ? Rgb(0x7ed957)
									  : Entry.strState == L"\xBCF4\xD1B5" ? Rgb(0xffd43e)
									  : Entry.strState == L"\xD63C\xC7A1" ? Rgb(0xff6a5a)
																		  : Colors::White;
		DrawAt(Entry.strState.c_str(), strYG760, 16.f, Rect.fX + 209.333f + 45.667f, fCenterY, 0.5f, 91.333f,
			   vStateColor);
		if (Entry.bCreatable)
		{
			// 현재 process roster를 세도록 지정한 행만 세션 캐릭터 수를 표시하고, 나머지는 JSON의 수를 표시한다.
			const uint32_t iShownCount = Entry.bCountsSavedCharacters
											 ? static_cast<uint32_t>(CCharacterRoster::Get_CharacterCount())
											 : Entry.iCharacterCount;
			const wstring strCount = std::to_wstring(iShownCount);
			DrawAt(strCount.c_str(), strYG760, 16.f, Rect.fX + 365.333f, fCenterY, 0.f, 33.333f, Colors::White);
		}
		else
		{
			DrawAt(L"\xC0DD\xC131 \xBD88\xAC00", strYG760, 16.f, Rect.fX + 317.333f + 41.333f, fCenterY, 0.5f, 82.667f,
				   Rgb(0xb0b0b0));
		}
	}

	// 접속(서버 선택)·종료·환경설정 글자를 저작 슬롯에 맞춘다. 버튼 활성 색은 선택 index를 보고 실제 클릭 허용은 Update_LobbyButtons가 판단한다.
	{
		LOBBY_PRODUCT_RECT Rect{};
		if (Get_LobbySlotRect(m_pLobbyBackgroundView.get(), "Lobby_JoinButton", Rect))
		{
			const bool_t bEnabled = m_iLobbySelectedServer >= 0;
			DrawAt(L"\xC11C\xBC84 \xC120\xD0DD", strYG760, 13.6f, Rect.fX + Rect.fWidth * 0.5f, Rect.fY + 16.2f, 0.5f,
				   Rect.fWidth - 8.f, bEnabled ? Colors::White : Rgb(0x787878));
		}
	}
	struct LOBBY_ICON_CAPTION
	{
		const char *pSlotId;
		const wchar_t *pLabel;
		f32_t fCenterX;
	};
	const LOBBY_ICON_CAPTION IconCaptions[2] = {
		{"Lobby_ExitIcon", L"\xC885\xB8CC", 54.667f},
		{"Lobby_OptionIcon", L"\xD658\xACBD\xC124\xC815", 1226.667f},
	};
	for (const LOBBY_ICON_CAPTION &Caption : IconCaptions)
	{
		LOBBY_PRODUCT_RECT Rect{};
		if (!Get_LobbySlotRect(m_pLobbyBackgroundView.get(), Caption.pSlotId, Rect))
			continue;
		DrawAt(Caption.pLabel, strYG760, 9.333f, Caption.fCenterX, 672.f, 0.5f, 100.f, Colors::White);
	}
	DrawAt(L"(C) Smilegate RPG, Inc. All rights reserved.", strYG760, 9.333f, 640.f, 711.f, 0.5f, 800.f, Rgb(0xeeeeee));
}

// 초기화가 UI/Lobby/LobbyServers.json의 표시용 행을 읽는다. 기존 목록·선택을 먼저 비우므로 실패하면 빈 목록이 남는다.
// 행별 타입을 확인하고 이름 없는 행을 건너뛰며, 유효한 첫 행을 기본 선택한다. 네트워크 endpoint 로더는 아니다.
void CMainApp::Load_LobbyServers()
{
	m_LobbyServers.clear();
	m_iLobbySelectedServer = -1;
	const filesystem::path DataPath = CProjectDataRoot::Resolve(L"UI/Lobby/LobbyServers.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
	{
		OutputDebugStringA("[Lobby] LobbyServers.json missing -- server list stays empty.\n");
		return;
	}
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		OutputDebugStringA(("[Lobby] LobbyServers.json parse failed: " + Error + "\n").c_str());
		return;
	}
	const DATA_JSON_VALUE *pServers = Root.Find("servers");
	if (nullptr == pServers || !pServers->Is_Array())
		return;
	for (const DATA_JSON_VALUE &Value : pServers->Get_Array())
	{
		if (!Value.Is_Object())
			continue;
		LOBBY_SERVER_ENTRY Entry{};
		// 문자열 필드만 UTF-8에서 wide 값으로 변환한다. 변환 성공 여부는 별도 진단하지 않고 아래 이름 검사로 빈 행을 제외한다.
		const auto ReadText = [&](const char *pKey, wstring &outText)
		{
			const DATA_JSON_VALUE *pText = Value.Find(pKey);
			if (nullptr != pText && pText->Is_String())
				(void)ConvertUtf8ToWide(pText->Get_String(), outText);
		};
		ReadText("name", Entry.strName);
		ReadText("state", Entry.strState);
		ReadText("tag", Entry.strTag);
		if (const DATA_JSON_VALUE *pCount = Value.Find("characterCount"))
			if (pCount->Is_Number() && pCount->Get_Number() >= 0.0)
				Entry.iCharacterCount = static_cast<uint32_t>(pCount->Get_Number());
		if (const DATA_JSON_VALUE *pCounts = Value.Find("countsSavedCharacters"))
			if (pCounts->Is_Boolean())
				Entry.bCountsSavedCharacters = pCounts->Get_Boolean();
		if (const DATA_JSON_VALUE *pCreatable = Value.Find("creatable"))
			if (pCreatable->Is_Boolean())
				Entry.bCreatable = pCreatable->Get_Boolean();
		if (Entry.strName.empty())
			continue;
		m_LobbyServers.push_back(std::move(Entry));
		if (m_LobbyServers.size() >= LOBBY_SERVER_ROW_COUNT)
			break;
	}
	if (!m_LobbyServers.empty())
		m_iLobbySelectedServer = 0;
}
// Render의 텍스트 단계에서 일반 재련 화면의 재련·성장 버튼 문구를 슬롯 크기에 맞춰 그린다.
// 좌표는 1280×720 기준에서 축별 변환하고 글자 크기는 작은 축 배율로 통일한다.
void CMainApp::RenderItemUpgradeButtonText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_ReforgeButton", fX, fY, fWidth, fHeight))
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const f32_t fCenterX = fX + fWidth * 0.5f;
	const f32_t fCenterY = fY + fHeight * 0.5f;

	const wchar_t *pLabel = L"\xC7A5\xBE44 \xC7AC\xB828"; // "장비 재련"
	const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
	const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * 0.32f / vMeasured.y) : 1.f;
	const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.8f / vMeasured.x) : 1.f;
	const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
								   float2_t(fCenterX * textScaleX, fCenterY * textScaleY), Colors::White, 0.f,
								   float2_t(0.5f, 0.5f), fScale * textUiScale);

	// 성장 글자는 재련 버튼과 같은 슬롯 측정·가운데 정렬을 사용한다.
	f32_t fGrowX = 0.f, fGrowY = 0.f, fGrowWidth = 0.f, fGrowHeight = 0.f;
	if (m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_LevelUpBtn", fGrowX, fGrowY, fGrowWidth, fGrowHeight))
	{
		const wchar_t *pGrowLabel = L"\xC131\xC7A5"; // "성장"
		const float2_t vGrowMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pGrowLabel);
		const f32_t fGrowScaleByHeight =
			(vGrowMeasured.y > 0.f) ? (fGrowHeight * 0.35f / vGrowMeasured.y) : 1.f; // 0.5 * 0.7
		const f32_t fGrowScaleByWidth = (vGrowMeasured.x > 0.f) ? (fGrowWidth * 0.8f / vGrowMeasured.x) : 1.f;
		const f32_t fGrowScale = (std::min)(fGrowScaleByHeight, fGrowScaleByWidth);
		CGameInstance::Get().Draw_Text(
			TEXT("Font_YoonGasiIIM"), pGrowLabel,
			float2_t((fGrowX + fGrowWidth * 0.5f) * textScaleX, (fGrowY + fGrowHeight * 0.5f) * textScaleY),
			Colors::White, 0.f, float2_t(0.5f, 0.5f), fGrowScale * textUiScale);
	}
}

// 보유 장비의 목록과 선택 장비의 현재·예상 단계 표를 그린다. 인벤토리를 복사해 표시할 뿐 강화 요청이나 상태 변경은 하지 않는다.
void CMainApp::RenderItemUpgradeListText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	auto DrawFit = [&](const char *pSlotId, const wchar_t *pLabel, f32_t fHeightRatio, fvector_t vColor)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight))
			return;

		const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
		const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * fHeightRatio / vMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.95f / vMeasured.x) : 1.f;
		const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
									   float2_t(fX * textScaleX, (fY + fHeight * 0.5f) * textScaleY), vColor, 0.f,
									   float2_t(0.f, 0.5f), fScale * textUiScale);
	};

	// 복제 인벤토리에서 강화 가능한 보유 장비를 모아 최대 여섯 행의 단계·이름을 그린다.
	// 각 글자의 위치는 저작된 슬롯 rect를 사용하며 여기에 없는 장비의 행은 출력하지 않는다.
	const vector<ITEM_UPGRADE_SLOT_INFO> upgradeSlots = BuildItemUpgradeSlots();
	// 단계 글자는 금색 #FFBD4A, 이름은 크림색 #E3C7A1을 사용한다.
	const fvector_t vLevelColor = XMVectorSet(1.0f, 0.7412f, 0.2902f, 1.f);	  // 색상 #FFBD4A
	const fvector_t vNameColor = XMVectorSet(0.8902f, 0.7804f, 0.6314f, 1.f); // 색상 #E3C7A1
	for (int32_t i = 0; i < 6 && i < static_cast<int32_t>(upgradeSlots.size()); ++i)
	{
		const string strLevelSlot = "ItemUpgrade_ListLevel" + to_string(i);
		const string strNameSlot = "ItemUpgrade_ListItemName" + to_string(i);
		const wstring strLevel = to_wstring(upgradeSlots[i].iUpgradeLevel) + L"\xB2E8\xACC4"; // "N단계"
		DrawFit(strLevelSlot.c_str(), strLevel.c_str(), 0.765f, vLevelColor);				  // "18단계" (0.85 * 0.9)
		DrawFit(strNameSlot.c_str(), upgradeSlots[i].strName.c_str(), 0.72f, vNameColor);	  // 0.8 * 0.9
	}

	// 선택 장비의 Server 인벤토리 단계를 기준으로 최대 일곱 단계 행을 표시한다.
	// 현재 Server 재련 정책의 성공률은 50%이며 이 표는 별도 공격력 보너스를 계산하지 않는다.
	const fvector_t vRowGray = XMVectorSet(0.4039f, 0.4039f, 0.4039f, 1.f);	  // #676767
	const fvector_t vSelectedGold = XMVectorSet(1.0f, 0.7412f, 0.2902f, 1.f); // 색상 #FFBD4A
	const int32_t ROW_COUNT = 7;
	const int32_t iSelectedForLadder =
		upgradeSlots.empty() ? -1
							 : std::clamp(m_iItemUpgradeSelectedSlot, 0, static_cast<int32_t>(upgradeSlots.size()) - 1);
	const int32_t iCurrentLevel = (iSelectedForLadder >= 0) ? upgradeSlots[iSelectedForLadder].iUpgradeLevel : 0;
	int32_t ROW_LEVELS[ROW_COUNT];
	for (int32_t i = 0; i < ROW_COUNT; ++i)
		ROW_LEVELS[i] = (std::min)(iCurrentLevel + (ROW_COUNT - 1 - i),
								   static_cast<int32_t>(LostArk::Shared::MAX_EQUIPMENT_UPGRADE_LEVEL));
	for (int32_t i = 0; i < ROW_COUNT; ++i)
	{
		const string strSlot = "ItemUpgrade_GradeRowText" + to_string(i);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(strSlot.c_str(), fX, fY, fWidth, fHeight))
			continue;

		const bool_t bSelected = (iCurrentLevel == ROW_LEVELS[i]);
		const fvector_t vNumColor = bSelected ? vSelectedGold : vRowGray;
		const fvector_t vStatTextColor = bSelected ? Colors::White : vRowGray;

		const wstring strNum = to_wstring(ROW_LEVELS[i]);
		const wchar_t *pDanggye = L"\xB2E8\xACC4";		   // "단계"
		const wstring strStat = L"\xC131\xACF5\xB960 50%"; // 현재 Server 재련 성공률 표시다.

		const float2_t vNumMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strNum.c_str());
		const f32_t fNumScale = (std::min)((vNumMeasured.y > 0.f) ? fHeight * 0.24f / vNumMeasured.y : 1.f,
										   (vNumMeasured.x > 0.f) ? fWidth * 0.3f / vNumMeasured.x : 1.f);
		CGameInstance::Get().Draw_Text(
			TEXT("Font_YoonGasiIIM"), strNum.c_str(),
			float2_t((fX + fWidth * 0.15f) * textScaleX, (fY + fHeight * 0.35f) * textScaleY), vNumColor, 0.f,
			float2_t(0.5f, 0.5f), fNumScale * textUiScale);
		CGameInstance::Get().Draw_Text(
			TEXT("Font_YoonGasiIIM"), pDanggye,
			float2_t((fX + fWidth * 0.15f) * textScaleX, (fY + fHeight * 0.65f) * textScaleY), vNumColor, 0.f,
			float2_t(0.5f, 0.5f), fNumScale * textUiScale);

		const float2_t vStatMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strStat.c_str());
		const f32_t fStatScale = (vStatMeasured.y > 0.f) ? (fHeight * 0.18f / vStatMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strStat.c_str(),
									   float2_t((fX + fWidth * 0.35f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
									   vStatTextColor, 0.f, float2_t(0.f, 0.5f), fStatScale * textUiScale);
	}
}

// 일반 재련 화면의 장비 행 클릭을 선택 index와 표시 아이콘에 반영한다.
// 목록을 복제 인벤토리에서 다시 구성해 사라진 항목을 제외하고 최대 여섯 행의 sprite와 선택 범위를 맞춘다.
void CMainApp::Update_ItemUpgradeSelection()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	const vector<ITEM_UPGRADE_SLOT_INFO> upgradeSlots = BuildItemUpgradeSlots();
	SyncItemUpgradeSlotSprites(*m_pItemUpgradeView, upgradeSlots, m_iItemUpgradeSelectedSlot);
	f32_t fListX = 0.f, fListY = 0.f, fListWidth = 0.f, fListHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_LeftListBg", fListX, fListY, fListWidth, fListHeight))
	{
		return;
	}

	CUIInputRouter &Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pItemUpgradeView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pItemUpgradeView->Get_ResolutionHeight();

	for (int32_t i = 0; i < 6 && i < static_cast<int32_t>(upgradeSlots.size()); ++i)
	{
		const string strGradeBgSlot = "ItemUpgrade_ListGradeBg" + to_string(i);
		f32_t fRowX = 0.f, fRowY = 0.f, fRowWidth = 0.f, fRowHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(strGradeBgSlot.c_str(), fRowX, fRowY, fRowWidth, fRowHeight))
		{
			continue;
		}

		// 아이콘의 좁은 rect 대신 목록 배경의 X·폭과 각 행의 Y·높이로 판정해 이름 부분을 눌러도 같은 행을 선택한다.
		const bool_t bHovered = Router.Is_Hovered(fListX, fRowY, fListWidth, fRowHeight, fRefWidth, fRefHeight);
		if (!bHovered)
			continue;
		Router.Claim_Mouse_This_Frame();
		if (!Router.Is_Clicked(fListX, fRowY, fListWidth, fRowHeight, fRefWidth, fRefHeight))
			continue;

		Play_UISound(UI_SOUND_CUE::BUTTON_CLICK);
		m_iItemUpgradeSelectedSlot = i;

		f32_t fTargetX = 0.f, fTargetY = 0.f, fTargetWidth = 0.f, fTargetHeight = 0.f;
		if (m_pItemUpgradeView->Get_SlotRect(strGradeBgSlot.c_str(), fTargetX, fTargetY, fTargetWidth, fTargetHeight))
		{
			m_pItemUpgradeView->Set_SlotPosition("ItemUpgrade_ListSelectedExample", fTargetX, fTargetY);
		}
		m_pItemUpgradeView->Set_SlotTexture("ItemUpgrade_SelectedItemIcon", upgradeSlots[i].strIconPath);
		break;
	}
}

// 성장 버튼 클릭은 Client의 게이지 연출만 다시 시작한다. Server 재련 요청은 100% 이후 별도 재련 버튼이 제출한다.
void CMainApp::Update_ItemUpgradeGrowButton()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult || BuildItemUpgradeSlots().empty())
	{
		return;
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_LevelUpBtn", fX, fY, fWidth, fHeight))
		return;

	CUIInputRouter &Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pItemUpgradeView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pItemUpgradeView->Get_ResolutionHeight();
	if (!Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;
	Router.Claim_Mouse_This_Frame();
	if (!Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;

	Play_UISound(UI_SOUND_CUE::BUTTON_CLICK);

	// 성장 표시를 매번 0에서 다시 시작한다. 완료 상태의 장식·alpha·예약 시간을 함께 초기화해 이전 100% 표시가 남지 않게 한다.
	m_iItemUpgradePreviousPercent = 0;
	m_bItemUpgradeGrowing = true;
	m_dItemUpgradeGrowStartSeconds = Product_Now_Seconds();
	m_bItemUpgradeCoreFlashPending = true;
	m_dItemUpgradeShockwaveScheduledAt = -1.0;
	m_dItemUpgradeCompleteRevealStartSeconds = -1.0;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_WingedRingGold", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_LevelUpMotion2Big", false);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", 0.f);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", 0.f);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_CompleteEffect", false);

	const filesystem::path growSoundPath =
		CRuntimeAssetRoot::Resolve(L"Sound/UI/Enhancement/sys_enhance_levelup_growup_full1__465402134.wav");
	CGameInstance::Get().Play_Sound(growSoundPath.wstring(), 1.f);
}

// 성장 완료 화면의 클릭을 Bern command sink에 재련 요청으로 제출한다.
// 아이템 ID·장착 슬롯·예상 단계를 요청에 넣고 결과 표시용 이름·아이콘을 값으로 보관한다. 실제 단계 변경은 Server 응답을 따른다.
void CMainApp::Update_ItemUpgradeReforgeButton()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible || m_bItemUpgradeGrowing ||
		100 != m_iItemUpgradePreviousPercent || ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult ||
		BuildItemUpgradeSlots().empty())
	{
		return;
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_ReforgeButton", fX, fY, fWidth, fHeight))
		return;

	CUIInputRouter &Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pItemUpgradeView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pItemUpgradeView->Get_ResolutionHeight();
	if (!Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;
	Router.Claim_Mouse_This_Frame();
	if (!Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;

	Play_UISound(UI_SOUND_CUE::BUTTON_CLICK);

	// 요청 직전 목록을 다시 읽고 캐릭터 저장·복원 대기 중에는 명령을 만들지 않는다. sequence가 0으로 소진된 경우도 제출하지 않는다.
	const vector<ITEM_UPGRADE_SLOT_INFO> slots = BuildItemUpgradeSlots();
	if (slots.empty() || !m_iNextItemUpgradeRequest || CCharacterSelectionState::Is_WorldStateSyncPending())
		return;
	const auto &item = slots[std::clamp(m_iItemUpgradeSelectedSlot, 0, static_cast<int32_t>(slots.size()) - 1)];
	auto *bern = CLevel_Bern::Get_Active();
	const auto sink = bern ? bern->Get_PlayerCommandSink() : nullptr;
	LostArk::Shared::C2S_UPGRADE_EQUIPMENT request{};
	request.iRequestSequence = m_iNextItemUpgradeRequest++;
	request.strItemId = item.strItemId;
	request.eSlot = item.eSlot;
	request.iExpectedUpgradeLevel = item.iUpgradeLevel;
	m_strItemUpgradeAttemptItemId = item.strItemId;
	m_strItemUpgradeAttemptIcon = item.strIconPath;
	m_strItemUpgradeAttemptName = item.strName;
	m_eItemUpgradeAttemptSlot = item.eSlot;
	m_iItemUpgradeConfirmedLevel = item.iUpgradeLevel;
	m_bItemUpgradePendingAttemptSuccess = false;
	// 요청 번호는 제출 시도 전에 이미 증가한다. 실제 제출 성공 때만 pending sequence를 보관하며 실패도 대기 화면의 확인 불가 상태로 보여준다.
	const bool submitted = CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::BERN) && sink &&
						   sink->Request_UpgradeEquipment(request);
	m_iPendingItemUpgradeRequest = submitted ? request.iRequestSequence : 0;
	m_dItemUpgradeRequestDeadline = Product_Now_Seconds() + 5.0;
	m_bItemUpgradeResultUnavailable = !submitted;
	m_eItemUpgradeAttemptResult = ITEM_UPGRADE_ATTEMPT_RESULT::WAITING;
	// 대기 화면을 준비하면서 성장 게이지를 0으로 되돌리고 중앙 기본 패널의 아이콘·버튼을 숨긴다.
	// 전송 실패도 WAITING 상태로 들어가지만 대기 문양·반복 사운드는 켜지 않고 결과 확인 불가 문구를 표시한다.
	Reset_ItemUpgradeIdleGauge();
	Set_ItemUpgradeCenterPanelVisible(false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitBg", true);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitEmblem", submitted);
	m_pItemUpgradeView->Restart_Animation("ItemUpgrade_ResultWaitEmblem");

	// 이 UI 반복 사운드 경로는 Level BGM과 분리되어 있다. 실제 제출에 성공한 경우에만 재생을 시작한다.
	const filesystem::path waitSoundPath =
		CRuntimeAssetRoot::Resolve(L"Sound/UI/Enhancement/sys_enhance_3_waiting1__95424590.wav");
	if (submitted)
		CGameInstance::Get().Play_LoopingSound(waitSoundPath.wstring(), 1.f);
}

// Update_ItemUpgrade가 pending sequence와 Bern world가 일치하는 응답을 현재 인벤토리와 대조한다.
// 일치하는 응답의 거절·항목 부재·단계 불일치는 확인 불가로 처리한다. 일치하는 응답이 없을 때 Bern 이탈·기한 도달을 검사하며 시간 초과가 서버 명령 취소를 뜻하지는 않는다.
void CMainApp::Update_ItemUpgradeServerResult()
{
	if (!m_iPendingItemUpgradeRequest)
		return;
	using namespace LostArk::Shared;
	const auto &hud = CCombatHUDViewModel::Get();
	const auto &result = hud.Get_UpgradeEquipmentResult();
	const bool inBern = CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::BERN) && CLevel_Bern::Get_Active() &&
						hud.Get_Player().isValid;
	// Server는 인벤토리 snapshot을 결과보다 먼저 같은 reliable 묶음에 담는다.
	// 여기서는 해당 장비의 현재 복제 단계가 응답 단계와 맞는지 확인하고, 거절과 정상 실패를 구분한다.
	if (inBern && result.iRequestSequence == m_iPendingItemUpgradeRequest && result.eWorldId == WORLD_ID::BERN)
	{
		const auto &items = hud.Get_Inventory().Items;
		const auto item = std::find_if(items.begin(), items.end(),
									   [this](const INVENTORY_ITEM_SNAPSHOT &entry)
									   {
										   return entry.strItemId == m_strItemUpgradeAttemptItemId &&
												  entry.eEquippedSlot == m_eItemUpgradeAttemptSlot;
									   });
		m_bItemUpgradeResultUnavailable = result.eResult == EQUIPMENT_UPGRADE_RESULT::REJECTED || item == items.end() ||
										  item->iUpgradeLevel != result.iUpgradeLevel;
		m_bItemUpgradePendingAttemptSuccess =
			!m_bItemUpgradeResultUnavailable && result.eResult == EQUIPMENT_UPGRADE_RESULT::SUCCEEDED;
		m_iItemUpgradeConfirmedLevel = result.iUpgradeLevel;
		m_iPendingItemUpgradeRequest = 0;
	}
	else if (!inBern || Product_Now_Seconds() >= m_dItemUpgradeRequestDeadline)
	{
		m_iPendingItemUpgradeRequest = 0;
		m_bItemUpgradeResultUnavailable = true;
	}
	if (m_bItemUpgradeResultUnavailable)
	{
		CGameInstance::Get().Stop_LoopingSound();
		m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitEmblem", false);
	}
}

// 응답 대기가 끝난 WAITING 화면을 누르면 확인 불가는 기본 창을 다시 열고, 유효 결과는 성공/실패 연출로 넘어간다.
// 이 함수는 저장된 결과를 공개하며 재련 성공 여부를 다시 추첨하지 않는다.
void CMainApp::Update_ItemUpgradeResultWaitClick()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::WAITING != m_eItemUpgradeAttemptResult || m_iPendingItemUpgradeRequest ||
		!CUIInputRouter::Get().Is_LeftClickEdge())
	{
		return;
	}
	// 응답이 끝난 WAITING 화면의 왼쪽 클릭 edge를 확인하고 이번 프레임 마우스를 점유한다.
	// 이 경로는 rect 기반 Is_Clicked를 호출하지 않으며, 다른 재련 버튼은 각자의 상태 조건으로 입력을 건너뛴다.
	CUIInputRouter::Get().Claim_Mouse_This_Frame();
	if (m_bItemUpgradeResultUnavailable)
	{
		m_bItemUpgradePreviewVisible = false;
		Hide_ItemUpgrade();
		Open_ItemUpgradeWindow();
		return;
	}

	// 대기 배경을 유지한 채 성공/실패 연출을 시작하고 4초 뒤 상세 표시 전환을 예약한다.
	// 현재 ItemUpgradeUI.json의 두 결과 flipbook은 각각 120장·30fps이며, 상세 전환은 Update_ItemUpgrade의 시계 검사에서 수행한다.
	// 이 상수는 실제 animation 완료 통지를 기다리는 값이 아니므로 JSON 재생 길이와 함께 유지해야 한다.
	constexpr f64_t RESULT_BURST_DURATION_SECONDS = 120.0 / 30.0;
	m_dItemUpgradeResultSettleAt = Product_Now_Seconds() + RESULT_BURST_DURATION_SECONDS;

	const bool_t bSuccess = m_bItemUpgradePendingAttemptSuccess;
	m_eItemUpgradeAttemptResult = bSuccess ? ITEM_UPGRADE_ATTEMPT_RESULT::SUCCESS : ITEM_UPGRADE_ATTEMPT_RESULT::FAIL;
	// 대기 문양을 숨기고 성공/실패 효과 중 하나만 켠 뒤 처음부터 재생한다. 넓은 SuccessModalBg/FailModalBg는 여기서 켜지 않는다.
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitEmblem", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessEffect", bSuccess);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailEffect", !bSuccess);
	if (bSuccess)
		m_pItemUpgradeView->Restart_Animation("ItemUpgrade_SuccessEffect");
	else
		m_pItemUpgradeView->Restart_Animation("ItemUpgrade_FailEffect");

	CGameInstance::Get().Stop_LoopingSound();
	// 성공은 세 파일 중 난수 나머지값, 실패는 두 파일 중 난수 나머지값으로 사운드를 고른다.
	// 함수 static 난수 상태는 메인 UI 호출 흐름에서 사용하며 이번 사운드 선택은 Server 재련 결과와 무관하다.
	static std::mt19937 s_itemUpgradeResultSoundRng{std::random_device{}()};
	if (bSuccess)
	{
		static const wchar_t *const SUCCESS_SOUNDS[] = {
			L"Sound/UI/Enhancement/sys_enhance_1_success1__168050678.wav",
			L"Sound/UI/Enhancement/sys_enhance_2_success1__225949772.wav",
			L"Sound/UI/Enhancement/sys_enhance_4_success1__596670890.wav",
		};
		const filesystem::path soundPath =
			CRuntimeAssetRoot::Resolve(SUCCESS_SOUNDS[s_itemUpgradeResultSoundRng() % 3u]);
		CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
	}
	else
	{
		static const wchar_t *const FAIL_SOUNDS[] = {
			L"Sound/UI/Enhancement/sys_enhance_3_casting_fail1__668013724.wav",
			L"Sound/UI/Enhancement/sys_enhance_4_casting_fail1__490424786.wav",
		};
		const filesystem::path soundPath = CRuntimeAssetRoot::Resolve(FAIL_SOUNDS[s_itemUpgradeResultSoundRng() % 2u]);
		CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
	}
}

// 결과 연출이 끝난 SUCCESS/FAIL 화면의 확인 버튼을 처리해 기본 재련 화면으로 복귀한다.
void CMainApp::Update_ItemUpgradeResultOkButton()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		(ITEM_UPGRADE_ATTEMPT_RESULT::SUCCESS != m_eItemUpgradeAttemptResult &&
		 ITEM_UPGRADE_ATTEMPT_RESULT::FAIL != m_eItemUpgradeAttemptResult) ||
		// 결과 연출 종료 시각이 아직 예약되어 있으면 확인 버튼의 입력을 받지 않는다.
		m_dItemUpgradeResultSettleAt >= 0.0)
	{
		return;
	}

	const char_t *pOkButtonSlotId = ITEM_UPGRADE_ATTEMPT_RESULT::SUCCESS == m_eItemUpgradeAttemptResult
										? "ItemUpgrade_SuccessOkBtn"
										: "ItemUpgrade_FailOkBtn";

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect(pOkButtonSlotId, fX, fY, fWidth, fHeight))
		return;

	CUIInputRouter &Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pItemUpgradeView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pItemUpgradeView->Get_ResolutionHeight();
	if (!Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;
	Router.Claim_Mouse_This_Frame();
	if (!Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return;

	Play_UISound(UI_SOUND_CUE::BUTTON_CLICK);

	m_eItemUpgradeAttemptResult = ITEM_UPGRADE_ATTEMPT_RESULT::NONE;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_ResultWaitEmblem", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessModalBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessOkBtn", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailModalBg", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailOkBtn", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessEffect", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailEffect", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessDiamondWinged", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessDiamondFrame", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_SuccessItemIconMarker", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailDiamondFrame", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_FailItemIconMarker", false);

	// 결과 확인 뒤 성장 게이지를 0으로 돌리고 가려 둔 기본 재련 패널을 다시 연다. Server 장비 단계는 응답으로 확정된 값을 유지한다.
	Reset_ItemUpgradeIdleGauge();
	Set_ItemUpgradeCenterPanelVisible(true);
}

// 성장·연출 예약 시간을 초기화하고 완료 장식을 숨긴다. 호출자가 유효한 m_pItemUpgradeView를 보장한다.
// 요청 sequence·확정 장비 단계·결과 enum 자체를 취소하는 함수는 아니다.
void CMainApp::Reset_ItemUpgradeIdleGauge()
{
	m_iItemUpgradePreviousPercent = 0;
	m_bItemUpgradeGrowing = false;
	m_dItemUpgradeGrowStartSeconds = -1.0;
	m_bItemUpgradeCoreFlashPending = false;
	m_dItemUpgradeShockwaveScheduledAt = -1.0;
	m_dItemUpgradeCompleteRevealStartSeconds = -1.0;
	m_dItemUpgradeResultSettleAt = -1.0;
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_WingedRingGold", false);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_LevelUpMotion2Big", false);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_WingedRingGold", 0.f);
	m_pItemUpgradeView->Set_SlotAlpha("ItemUpgrade_LevelUpMotion2Big", 0.f);
	m_pItemUpgradeView->Set_SlotVisible("ItemUpgrade_CompleteEffect", false);
}

// 기본 중앙 패널의 지정 슬롯만 같은 visible 값으로 바꾼다. 목록·결과 배경 등 이 배열 밖 슬롯은 각 화면 전환이 관리한다.
void CMainApp::Set_ItemUpgradeCenterPanelVisible(bool_t bVisible)
{
	if (nullptr == m_pItemUpgradeView)
		return;

	constexpr const char_t *CENTER_PANEL_SLOTS[] = {
		"ItemUpgrade_WindowBg",
		"ItemUpgrade_PanelBg",
		"ItemUpgrade_RecipeIconBgExample",
		"ItemUpgrade_RecipeMaterial0",
		"ItemUpgrade_RecipeAmount0",
		"ItemUpgrade_RecipeIconBg1",
		"ItemUpgrade_RecipeMaterial1",
		"ItemUpgrade_RecipeAmount1",
		"ItemUpgrade_RecipeIconBg2",
		"ItemUpgrade_RecipeMaterial2",
		"ItemUpgrade_RecipeAmount2",
		"ItemUpgrade_DecoIcon",
		"ItemUpgrade_GaugeFill",
		"ItemUpgrade_SelectedItemIconBounds",
		"ItemUpgrade_SelectedItemIcon",
		"ItemUpgrade_LevelUpBtn",
		"ItemUpgrade_EquipExpPageLine",
		"ItemUpgrade_ReforgeButton",
		"ItemUpgrade_LevelArrowBase",
		"ItemUpgrade_LevelArrow",
	};
	for (const char_t *pSlotId : CENTER_PANEL_SLOTS)
		m_pItemUpgradeView->Set_SlotVisible(pSlotId, bVisible);

	// 숨길 때만 완료 효과 목록도 함께 내린다. 다시 보일 시점은 성장 입력과 Update_ItemUpgrade의 시간 처리가 결정한다.
	if (!bVisible)
	{
		constexpr const char_t *CENTER_PANEL_EFFECT_SLOTS[] = {
			"ItemUpgrade_WingedRingGold", "ItemUpgrade_LevelUpMotion2Big", "ItemUpgrade_SmeltGlow",
			"ItemUpgrade_CoreFlash",	  "ItemUpgrade_ShockwaveRing",	   "ItemUpgrade_CompleteEffect",
			"ItemUpgrade_WingDecoFade",
		};
		for (const char_t *pSlotId : CENTER_PANEL_EFFECT_SLOTS)
			m_pItemUpgradeView->Set_SlotVisible(pSlotId, false);
	}
}

// 기본 화면에서 선택한 보유 장비의 이름·현재 단계·다음 단계 문구를 그린다. 목록 문자열 참조는 이 함수 호출 동안만 사용한다.
void CMainApp::RenderItemUpgradeLevelText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	// 선택 index를 현재 보유 장비 목록 범위로 제한한 뒤 이름과 단계를 표시한다. 목록이 비었으면 출력하지 않는다.
	// 이름은 아래 금주황색, 현재 단계는 금색, 다음 단계는 연두색 인자를 사용하며 화살표는 별도 animation 슬롯이다.
	const vector<ITEM_UPGRADE_SLOT_INFO> upgradeSlots = BuildItemUpgradeSlots();
	if (upgradeSlots.empty())
		return;
	const int32_t iSelectedSlot =
		std::clamp(m_iItemUpgradeSelectedSlot, 0, static_cast<int32_t>(upgradeSlots.size()) - 1);
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_ItemNameLabel", fX, fY, fWidth, fHeight))
		{
			const wchar_t *pLabel = upgradeSlots[iSelectedSlot].strName.c_str();
			const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
			const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * 0.95f / vMeasured.y) : 1.f;
			const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.95f / vMeasured.x) : 1.f;
			const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
			CGameInstance::Get().Draw_Text(
				TEXT("Font_YoonGasiIIM"), pLabel,
				float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
				XMVectorSet(1.0f, 0.5686f, 0.0f, 1.f), 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
		}
	}

	// 현재 단계는 Server 인벤토리 값이며 다음 단계 표시는 현재+1을 Shared의 최대 강화 단계로 제한한다.
	const int32_t iLevel = upgradeSlots[iSelectedSlot].iUpgradeLevel;
	const wstring strCurLevel = to_wstring(iLevel) + L"\xB2E8\xACC4";
	const wstring strNextLevel =
		to_wstring((std::min)(iLevel + 1, static_cast<int32_t>(LostArk::Shared::MAX_EQUIPMENT_UPGRADE_LEVEL))) +
		L"\xB2E8\xACC4";
	struct LEVEL_TEXT_ENTRY
	{
		const char *pSlotId;
		const wstring &strLabel;
		f32_t fHeightRatio;
		fvector_t vColor;
	};
	const LEVEL_TEXT_ENTRY SLOTS[] = {
		{"ItemUpgrade_CurLevelLabel", strCurLevel, 0.95f, XMVectorSet(1.0f, 0.7412f, 0.2902f, 1.f)},
		{"ItemUpgrade_NextLevelLabel", strNextLevel, 0.95f, XMVectorSet(0.7176f, 0.9843f, 0.0f, 1.f)},
	};

	for (const LEVEL_TEXT_ENTRY &Slot : SLOTS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(Slot.pSlotId, fX, fY, fWidth, fHeight))
			continue;

		const f32_t fCenterX = fX + fWidth * 0.5f;
		const f32_t fCenterY = fY + fHeight * 0.5f;

		const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), Slot.strLabel.c_str());
		const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * Slot.fHeightRatio / vMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.95f / vMeasured.x) : 1.f;
		const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), Slot.strLabel.c_str(),
									   float2_t(fCenterX * textScaleX, fCenterY * textScaleY), Slot.vColor, 0.f,
									   float2_t(0.5f, 0.5f), fScale * textUiScale);
	}
}

// 기본 재련 화면의 재료 칸에 현재 무료 정책을 표시한다. 슬롯 세 개가 같은 글자 배율을 쓰도록 먼저 공통 크기를 구한다.
void CMainApp::RenderItemUpgradeMaterialCounts()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	// 현재 Server 재련 경로가 재료·재화를 차감하지 않으므로 세 재료 칸에 무료 문구를 표시한다.
	const wstring strAmount = L"\xBB34\xB8CC";
	const char *SLOT_IDS[3] = {"ItemUpgrade_RecipeAmount0", "ItemUpgrade_RecipeAmount1", "ItemUpgrade_RecipeAmount2"};
	const fvector_t vSufficientColor =
		XMVectorSet(0.7176f, 0.9843f, 0.0f,
					1.f); // #B7FB00 색을 사용하며 세 재료 슬롯에 들어가는 배율 중 최솟값을 모든 문구에 공통 적용한다.
	f32_t fSharedScale = 1.f;
	{
		bool_t bAny = false;
		for (int32_t i = 0; i < 3; ++i)
		{
			f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
			if (!m_pItemUpgradeView->Get_SlotRect(SLOT_IDS[i], fX, fY, fW, fH))
				continue;

			const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strAmount.c_str());
			if (vMeasured.y <= 0.f || vMeasured.x <= 0.f)
				continue;

			const f32_t fFit = (std::min)(fH * 0.9f / vMeasured.y, fW * 0.95f / vMeasured.x);
			fSharedScale = bAny ? (std::min)(fSharedScale, fFit) : fFit;
			bAny = true;
		}
	}

	for (int32_t i = 0; i < 3; ++i)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(SLOT_IDS[i], fX, fY, fWidth, fHeight))
			continue;

		const f32_t fCenterX = fX + fWidth * 0.5f;
		const f32_t fCenterY = fY + fHeight * 0.5f;

		const fvector_t vColor = vSufficientColor;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strAmount.c_str(),
									   float2_t(fCenterX * textScaleX, fCenterY * textScaleY), vColor, 0.f,
									   float2_t(0.5f, 0.5f), fSharedScale * textUiScale);
	}
}

// Client 성장 표시 상태의 정수 퍼센트를 게이지 가운데 그린다. Server 재련 확률·진행률을 나타내는 값은 아니다.
void CMainApp::RenderItemUpgradeGaugePercentText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::NONE != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_GaugeFill", fX, fY, fWidth, fHeight))
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const wstring strPercent = to_wstring(m_iItemUpgradePreviousPercent) + L"%";
	const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strPercent.c_str());
	const f32_t fScale = (vMeasured.y > 0.f) ? (fHeight * 0.2f / vMeasured.y) : 1.f;

	// 게이지 퍼센트와 결과 안내 문구에 같은 금노랑색 #FFD959를 사용한다.
	const fvector_t vColor = XMVectorSet(1.0f, 0.851f, 0.353f, 1.f); // 색상 #FFD959
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strPercent.c_str(),
								   float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
								   vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
}

// WAITING 화면의 현재 요청 상태를 문구로 표시한다. 전송 실패나 시간 초과도 결과 확인 불가 안내를 통해 기본 창으로 돌아갈 수 있다.
void CMainApp::RenderItemUpgradeResultWaitText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::WAITING != m_eItemUpgradeAttemptResult)
	{
		return;
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pItemUpgradeView->Get_SlotRect("ItemUpgrade_ResultWaitBg", fX, fY, fWidth, fHeight))
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	// WAITING 안내 한 줄이 서버 응답 대기·확인 불가·결과 공개 가능 상태를 구분한다. pending sequence가 남으면 결과 공개 클릭을 받지 않는다.
	const wstring strPrompt =
		m_iPendingItemUpgradeRequest
			? L"\xC11C\xBC84 \xACB0\xACFC\xB97C \xAE30\xB2E4\xB9AC\xB294 \xC911\xC785\xB2C8\xB2E4"
			: (m_bItemUpgradeResultUnavailable
				   ? L"\xACB0\xACFC\xB97C \xD655\xC778\xD558\xC9C0 \xBABB\xD588\xC2B5\xB2C8\xB2E4. \xD074\xB9AD\xD558\xC5EC \xB3CC\xC544\xAC00\xAE30"
				   : L"\xD654\xBA74\xC744 \xD074\xB9AD\xD558\xC5EC \xACB0\xACFC \xC989\xC2DC \xD655\xC778");
	const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strPrompt.c_str());
	const f32_t fScale = (vMeasured.y > 0.f) ? (fHeight * 0.045f / vMeasured.y) : 1.f;

	// 게이지 퍼센트와 결과 안내 문구에 같은 금노랑색 #FFD959를 사용한다.
	const fvector_t vColor = XMVectorSet(1.0f, 0.851f, 0.353f, 1.f); // 색상 #FFD959
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strPrompt.c_str(),
								   float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.88f) * textScaleY),
								   vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
}

// 성공 연출이 끝난 뒤 요청 당시 이름과 Server가 확정한 단계·성공 문구·확인 버튼 글자를 표시한다.
void CMainApp::RenderItemUpgradeSuccessDetailText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::SUCCESS != m_eItemUpgradeAttemptResult ||
		// 결과 연출 중에는 상세 글자를 그리지 않는다. Update_ItemUpgrade가 전환 예약을 비운 다음부터 표시한다.
		m_dItemUpgradeResultSettleAt >= 0.0)
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const auto DrawCentered =
		[&](const char_t *pSlotId, const wchar_t *pLabel, f32_t fHeightRatio, const fvector_t &vColor)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight))
			return;
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
		const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * fHeightRatio / vMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.95f / vMeasured.x) : 1.f;
		const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
									   float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
									   vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	};

	// 요청 당시 보관한 장비 이름을 현재 레이아웃의 성공 아이콘 아래 슬롯에 표시한다. 기본 화면과 같은 금주황색·높이 비율을 사용한다.
	const fvector_t vGoldOrange = XMVectorSet(1.0f, 0.5686f, 0.0f, 1.f);
	DrawCentered("ItemUpgrade_SuccessItemNameMarker", m_strItemUpgradeAttemptName.c_str(), 0.95f, vGoldOrange);

	// sequence·world가 일치하고 인벤토리 단계와 대조한 Server 응답의 확정 단계를 표시한다.
	const wstring strReachedLevel = to_wstring(m_iItemUpgradeConfirmedLevel) + L"\xB2E8\xACC4";
	DrawCentered("ItemUpgrade_SuccessGradeMarker", strReachedLevel.c_str(), 0.9f, vGoldOrange);

	// "재련 성공"은 지정된 연두색 #90EE90으로 표시한다.
	const fvector_t vLightGreen = XMVectorSet(0.5647f, 0.9333f, 0.5647f, 1.f); // 색상 #90EE90
	DrawCentered("ItemUpgrade_SuccessStatusMarker", L"\xC7AC\xB828 \xC131\xACF5", 0.9f, vLightGreen);

	// 결과 연출이 끝난 뒤 SuccessOkBtn에 "확인"을 그린다. 기본 버튼과 같은 흰색·높이 0.32 비율을 사용한다.
	DrawCentered("ItemUpgrade_SuccessOkBtn", L"\xD655\xC778", 0.32f, Colors::White);
}

// 유효한 실패 결과의 연출이 끝난 뒤 요청 당시 이름과 실패 문구·확인 버튼 글자를 표시한다. 거절/확인 불가 상태는 이 화면으로 보내지 않는다.
void CMainApp::RenderItemUpgradeFailDetailText()
{
	if (nullptr == m_pItemUpgradeView || !m_bItemUpgradePreviewVisible ||
		ITEM_UPGRADE_ATTEMPT_RESULT::FAIL != m_eItemUpgradeAttemptResult ||
		// 결과 연출 중에는 상세 글자를 그리지 않는다. Update_ItemUpgrade가 전환 예약을 비운 다음부터 표시한다.
		m_dItemUpgradeResultSettleAt >= 0.0)
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const auto DrawCentered =
		[&](const char_t *pSlotId, const wchar_t *pLabel, f32_t fHeightRatio, const fvector_t &vColor)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pItemUpgradeView->Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight))
			return;
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pLabel);
		const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * fHeightRatio / vMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.95f / vMeasured.x) : 1.f;
		const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pLabel,
									   float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
									   vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	};

	// 실패 화면도 요청 당시 장비 이름과 기본 화면의 금주황색·높이 비율을 사용한다.
	const fvector_t vGoldOrange = XMVectorSet(1.0f, 0.5686f, 0.0f, 1.f);
	DrawCentered("ItemUpgrade_FailItemNameMarker", m_strItemUpgradeAttemptName.c_str(), 0.95f, vGoldOrange);

	// "재련 실패"는 지정된 빨간색 #E82929로 표시한다. 실패 응답은 장비 단계를 올리지 않으므로 도달 단계 문구를 붙이지 않는다.
	const fvector_t vRed = XMVectorSet(0.9098f, 0.1608f, 0.1608f, 1.f); // 색상 #E82929
	DrawCentered("ItemUpgrade_FailStatusMarker", L"\xC7AC\xB828 \xC2E4\xD328", 0.9f, vRed);

	// 결과 연출이 끝난 뒤 FailOkBtn의 "확인"을 기본 버튼과 같은 흰색·높이 0.32 비율로 표시한다.
	DrawCentered("ItemUpgrade_FailOkBtn", L"\xD655\xC778", 0.32f, Colors::White);
}
// Update_CombatHUD가 보장한 HUD view에 인벤토리 드롭과 아이템 아이콘 상태를 반영한다.
// 드롭은 한 번 소비하며 어느 HUD 슬롯에도 맞지 않으면 배치를 바꾸지 않는다. 보관되는 item ID는 Client 표시 배치이고 사용 가능 여부·수량은 복제 인벤토리로 확인한다.
void CMainApp::Update_ItemQuickSlots()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.ItemQuickSlots.Update");
	if (nullptr == m_pInventoryView)
		return;

	constexpr const char *ITEM_SLOT_IDS[4] = {"Item_1", "Item_2", "Item_3", "Item_4"};
	constexpr const char *ITEM_ICON_SLOT_IDS[4] = {"Item_1_Icon", "Item_2_Icon", "Item_3_Icon", "Item_4_Icon"};

	string strDroppedItemId;
	float fDropX = 0.f, fDropY = 0.f;
	if (m_pInventoryView->Try_Consume_ItemDrop(strDroppedItemId, fDropX, fDropY))
	{
		// 드롭 위치는 InventoryView가 UIInputRouter에서 얻은 Client 영역의 실제 픽셀 좌표다.
		// Get_SlotRect는 1280×720 기준 좌표를 주므로 viewport의 축별 배율을 적용한 뒤 같은 좌표계에서 비교한다.
		const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
		const float scaleX = vViewportSize.x / 1280.f;
		const float scaleY = vViewportSize.y / 720.f;
		for (int32_t i = 0; i < 4; ++i)
		{
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!m_pHUDRuntimeView->Get_SlotRect(ITEM_SLOT_IDS[i], fX, fY, fWidth, fHeight))
				continue;
			const float left = fX * scaleX;
			const float top = fY * scaleY;
			const float right = left + fWidth * scaleX;
			const float bottom = top + fHeight * scaleY;
			if (fDropX >= left && fDropX < right && fDropY >= top && fDropY < bottom)
			{
				m_strItemQuickSlot[i] = strDroppedItemId;
				break;
			}
		}
	}

	for (int32_t i = 0; i < 4; ++i)
	{
		const ITEM_DEFINITION *pDefinition =
			m_strItemQuickSlot[i].empty() ? nullptr : CItemCatalog::Find_ById(m_strItemQuickSlot[i]);
		if (nullptr == pDefinition || pDefinition->strIconPath.empty())
		{
			m_pHUDRuntimeView->Set_SlotVisible(ITEM_ICON_SLOT_IDS[i], false);
			continue;
		}
		m_pHUDRuntimeView->Set_SlotTexture(ITEM_ICON_SLOT_IDS[i], pDefinition->strIconPath);
		m_pHUDRuntimeView->Set_SlotVisible(ITEM_ICON_SLOT_IDS[i], true);
		const auto &inventory = CCombatHUDViewModel::Get().Get_Inventory().Items;
		const auto owned = std::find_if(inventory.begin(), inventory.end(),
										[&](const auto &item)
										{
											return item.strItemId == m_strItemQuickSlot[i] &&
												   item.eEquippedSlot == LostArk::Shared::EQUIPMENT_SLOT::NONE &&
												   item.iQuantity > 0u;
										});
		const f32_t cooldown =
			Resolve_HudCooldownRatio(CCombatHUDViewModel::Get().Get_Player(), pDefinition->iBattleSkillId,
									 pDefinition->iCooldownMs, ITEM_SLOT_IDS[i]);
		const f32_t brightness = owned == inventory.end() ? 0.25f : cooldown > 0.f ? 0.45f : 1.f;
		m_pHUDRuntimeView->Set_SlotTint(ITEM_ICON_SLOT_IDS[i], float4_t(brightness, brightness, brightness, 1.f));
	}
}

// 복제된 HP·보호막·자원을 표시 비율로 바꾸어 공통 HUD 막대에 적용한다. 실제 전투 수치의 소유자는 Server다.
void CMainApp::Update_PlayerHealthManaBar()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.PlayerHealthManaBar.Update");
	// Update_CombatHUD가 유효한 player snapshot·view와 0이 아닌 최대 HP·자원을 확인한 뒤 호출한다.
	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();

	const float healthRatio =
		(std::clamp)(static_cast<float>(player.iCurrentHp) / static_cast<float>(player.iMaximumHp), 0.f, 1.f);
	const float manaRatio =
		(std::clamp)(static_cast<float>(player.iCurrentResource) / static_cast<float>(player.iMaximumResource), 0.f,
					 1.f);

	// 현재 HP와 보호막 합이 최대 HP보다 작으면 보호막은 최대 HP에 대한 비율로 체력 끝에 붙는다.
	// 합이 최대 HP 이상이면 전체 폭을 HP와 보호막의 합으로 나누어 두 영역을 배치한다.
	// 따라서 보호막이 줄어들 때 빨간 체력 영역이 다시 넓어질 수 있으며 Server HP 자체를 바꾸는 계산은 아니다.
	const double shield = static_cast<double>(player.iShield);
	const double currentHp = static_cast<double>(player.iCurrentHp);
	const double maximumHp = static_cast<double>(player.iMaximumHp);
	float shieldRatio = 0.f;
	float healthTrackRatio = healthRatio;
	if (shield > 0.0 && maximumHp > 0.0)
	{
		if (shield + currentHp < maximumHp)
		{
			shieldRatio = static_cast<float>(shield / maximumHp);
		}
		else
		{
			shieldRatio = static_cast<float>(shield / (shield + currentHp));
			healthTrackRatio = 1.f - shieldRatio;
		}
	}

	m_pHUDRuntimeView->Set_SlotFillRatio("HealthBar_Fill", healthTrackRatio);
	m_pHUDRuntimeView->Set_SlotVisible("HealthBar_Fill", healthTrackRatio > 0.f);
	// 두 슬롯의 저작 기준은 왼쪽 시작점이므로 흰 보호막 구간을 현재 체력 표시의 끝으로 옮긴다.
	f32_t healthX = 0.f, healthY = 0.f, healthWidth = 0.f, healthHeight = 0.f;
	if (m_pHUDRuntimeView->Get_SlotRect("HealthBar_Fill", healthX, healthY, healthWidth, healthHeight))
		m_pHUDRuntimeView->Set_SlotRect("HealthBar_Shield_Fill", healthX + healthWidth * healthTrackRatio, healthY,
										healthWidth * shieldRatio, healthHeight);
	m_pHUDRuntimeView->Set_SlotFillRatio("HealthBar_Shield_Fill", 1.f);
	m_pHUDRuntimeView->Set_SlotVisible("HealthBar_Shield_Fill", shieldRatio > 0.f);
	m_pHUDRuntimeView->Set_SlotFillRatio("ManaBar_Fill", manaRatio);
	// Ember 자원을 사용하는 직업은 마나 채움 영역을 숨긴다. 가디언 나이트의 socket과 구슬 표시는 별도 갱신 함수가 맡는다.
	m_pHUDRuntimeView->Set_SlotVisible("ManaBar_Fill", manaRatio > 0.f && 0u == player.iEmberMaximumSockets);
}

// 창술사 HUD의 세 구간 채움과 점화→반복 효과를 갱신한다. 유효하지 않은 snapshot·최대치·view이면 기존 표시를 건드리지 않고 반환한다.
// 점화 시작 시각과 직전 full 상태는 이 MainApp이 보관하며 Server identity 수치에는 쓰지 않는다.
void CMainApp::Update_LanceMasterIdentityGauge()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.LanceMasterIdentityGauge.Update");
	// 최대 identity를 세 구간으로 나눈 뒤 앞 구간부터 0~100 값으로 채운다.
	// 이 함수는 복제된 수치를 표시용 구간 값으로 바꾸며 identity를 소비하거나 증가시키지 않는다.
	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || 0u == player.iMaximumIdentity || nullptr == m_pHUDRuntimeView)
		return;

	constexpr f32_t SEGMENT_MAX = 100.f;
	const f32_t fSegmentScale = player.iMaximumIdentity / (SEGMENT_MAX * 3.f);
	f32_t fRemaining = static_cast<f32_t>(player.iCurrentIdentity) / fSegmentScale;
	f32_t fSegmentValue[3];
	for (int32_t i = 0; i < 3; ++i)
	{
		fSegmentValue[i] = (std::min)(SEGMENT_MAX, (std::max)(0.f, fRemaining));
		fRemaining -= SEGMENT_MAX;
	}

	// 채움 모양은 Gauge0/1/2Fill.json의 정수 이름 keyframe으로 표현한다.
	// 각 구간 값을 0~99의 frame 이름으로 제한하므로 100%도 마지막 99번 frame을 사용한다.
	for (int32_t i = 0; i < 3; ++i)
	{
		const int32_t iFillFrame = std::clamp(static_cast<int32_t>(fSegmentValue[i]), 0, 99);
		m_pHUDRuntimeView->Play_KeyframeAnimation(string("Lance_Id_GaugeFill") + std::to_string(i),
												  std::to_string(iFillFrame));

		// 구간이 가득 찬 순간 점화 animation을 한 번 시작하고 고정 대기 0.9/0.8/0.9초 뒤 반복 animation으로 바꾼다.
		// 아래 상수는 40으로 나누지만 실제 Burn/BurnLoop JSON은 24fps다. 문서의 animation 완료 통지를 기다리는 전환은 아니다.
		constexpr f32_t BURN_IGNITE_SECONDS[3] = {36.f / 40.f, 32.f / 40.f, 36.f / 40.f};
		const bool_t bIsFull = fSegmentValue[i] >= SEGMENT_MAX;
		if (bIsFull && !m_bLanceGaugeSegmentWasFull[i])
		{
			m_pHUDRuntimeView->Play_KeyframeAnimation(string("Lance_Id_GaugeBurn") + std::to_string(i), "burn");
			m_dLanceGaugeIgniteStartSeconds[i] = Product_Now_Seconds();
			m_bLanceGaugeLoopStarted[i] = false;
		}
		m_bLanceGaugeSegmentWasFull[i] = bIsFull;

		const bool_t bIgniteDone =
			bIsFull && m_dLanceGaugeIgniteStartSeconds[i] >= 0.0 &&
			(Product_Now_Seconds() - m_dLanceGaugeIgniteStartSeconds[i]) >= BURN_IGNITE_SECONDS[i];
		if (bIgniteDone && !m_bLanceGaugeLoopStarted[i])
		{
			m_pHUDRuntimeView->Play_KeyframeAnimation(string("Lance_Id_GaugeBurnLoop") + std::to_string(i), "burn");
			m_bLanceGaugeLoopStarted[i] = true;
		}

		// 점화와 반복 불꽃 중 현재 단계에 맞는 것만 켠다. 구간이 가득 차 있지 않으면 두 효과를 모두 숨긴다.
		m_pHUDRuntimeView->Set_SlotVisible(string("Lance_Id_GaugeBurn") + std::to_string(i), bIsFull && !bIgniteDone);
		m_pHUDRuntimeView->Set_SlotVisible(string("Lance_Id_GaugeBurnLoop") + std::to_string(i),
										   bIsFull && bIgniteDone);
	}
}

// 가디언 나이트의 stance 이미지·identity 구슬·Ember socket을 복제 상태에 맞춘다.
// 이 함수는 표시 rect·UV·animation만 바꾸며 자원의 잠금 해제·소모·stance 변경 요청은 하지 않는다.
void CMainApp::Update_GuardianKnightIdentity()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.GuardianKnightIdentity.Update");
	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || nullptr == m_pHUDRuntimeView)
		return;

	// Ember socket 막대가 같은 영역을 사용하므로 기본 마나 배경·테두리·채움을 함께 숨긴다.
	m_pHUDRuntimeView->Set_SlotVisible("ManaBar_BG", false);
	m_pHUDRuntimeView->Set_SlotVisible("ManaBar", false);
	m_pHUDRuntimeView->Set_SlotVisible("ManaBar_Fill", false);

	const bool_t bDragon = LostArk::Shared::PLAYER_STANCE_ID::GUARDIANKNIGHT_DRAGON == player.eStance;

	// 구슬은 아래에서 위로 차오르도록 채움 rect와 UV 창을 같은 비율로 잘라 표시한다.
	// 텍스처 전체를 작은 rect에 눌러 넣지 않도록 UV의 시작점과 높이도 함께 바꾸며 인간·용 stance별 이미지를 고른다.
	const f32_t fIdentityRatio =
		0u != player.iMaximumIdentity
			? std::clamp(static_cast<f32_t>(player.iCurrentIdentity) / static_cast<f32_t>(player.iMaximumIdentity), 0.f,
						 1.f)
			: 0.f;
	m_pHUDRuntimeView->Set_SlotTexture("GK_Id_OrbFill", bDragon ? "UI/HUD/GuardianKnight/Orb_Fill_Dragon.png"
																: "UI/HUD/GuardianKnight/Orb_Fill_Human.png");
	m_pHUDRuntimeView->Set_SlotVisible("GK_Id_OrbFill", fIdentityRatio > 0.f);

	// 움직이지 않는 빈 구슬의 rect를 매번 기준으로 읽는다. 이미 줄인 채움 rect를 다시 기준으로 삼으면 크기가 누적해서 줄어든다.
	f32_t fOrbX = 0.f, fOrbY = 0.f, fOrbWidth = 0.f, fOrbHeight = 0.f;
	if (fIdentityRatio > 0.f && m_pHUDRuntimeView->Get_SlotRect("GK_Id_OrbEmpty", fOrbX, fOrbY, fOrbWidth, fOrbHeight))
	{
		const f32_t fFillHeight = fOrbHeight * fIdentityRatio;
		m_pHUDRuntimeView->Set_SlotRect("GK_Id_OrbFill", fOrbX, fOrbY + fOrbHeight - fFillHeight, fOrbWidth,
										fFillHeight);
		m_pHUDRuntimeView->Set_SlotUVWindow("GK_Id_OrbFill", 0.f, 1.f - fIdentityRatio, 1.f, fIdentityRatio);

		// 채움 위쪽 경계에 표시선을 배치한다. 빈 상태·가득 찬 상태에서 숨기는 판정은 아래에서 별도로 수행한다.
		f32_t fMarkX = 0.f, fMarkY = 0.f, fMarkWidth = 0.f, fMarkHeight = 0.f;
		if (m_pHUDRuntimeView->Get_SlotRect("GK_Id_OrbMark", fMarkX, fMarkY, fMarkWidth, fMarkHeight))
		{
			// 표시선의 중심을 수면 높이에 맞추되 구슬 위·아래 경계 안으로 제한한다.
			const f32_t fLevelY = fOrbY + fOrbHeight - fFillHeight - fMarkHeight * 0.5f;
			m_pHUDRuntimeView->Set_SlotPosition("GK_Id_OrbMark", fOrbX + fOrbWidth * 0.5f - fMarkWidth * 0.5f,
												std::clamp(fLevelY, fOrbY, fOrbY + fOrbHeight - fMarkHeight));
		}
	}
	// 표시선은 채움 비율의 양 끝에서 숨긴다. 0과 1의 완전 일치 대신 0.001 여유를 두므로 아주 작거나 거의 가득 찬 구간도 숨겨진다.
	constexpr f32_t MARK_END_EPSILON = 0.001f;
	m_pHUDRuntimeView->Set_SlotVisible("GK_Id_OrbMark",
									   fIdentityRatio > MARK_END_EPSILON && fIdentityRatio < 1.f - MARK_END_EPSILON);

	// Z 글자는 별도 텍스트 단계가 그린다. 여기서는 글자 뒤 배경 슬롯만 켠다.
	m_pHUDRuntimeView->Set_SlotVisible("GK_Id_SkillKeyBg", true);

	// 여덟 점은 현재 상수 배열대로 표시하며 복제된 스킬 종류로 다시 계산하지 않는다.
	// 위쪽 네 칸은 숨기고 아래쪽 첫째·둘째·넷째 점을 켜는 현재 표시 설정을 두 stance에서 공통 사용한다.
	constexpr bool_t AWAKENING_SLOT_DOTS[8] = {false, false, false, false, true, true, false, true};
	m_pHUDRuntimeView->Set_SlotVisible("GK_Id_StatusPlate", true);
	for (int32_t i = 0; i < 8; ++i)
	{
		m_pHUDRuntimeView->Set_SlotVisible(string("GK_Id_StatusDot") + std::to_string(i), AWAKENING_SLOT_DOTS[i]);
	}

	// 최대 socket에서 잠긴 수를 뺀 값을 0~10으로 제한해 열린 칸 수를 구한다.
	// 복제된 Ember 구슬 수 중 열린 칸에 들어가는 만큼 채우고 나머지는 빈 칸 또는 잠금 상태로 표시한다.
	constexpr int32_t SOCKET_COUNT = 10;
	const int32_t iUnlockedSockets =
		std::clamp(static_cast<int32_t>(player.iEmberMaximumSockets) - static_cast<int32_t>(player.iEmberLockedSockets),
				   0, SOCKET_COUNT);
	const int32_t iFilledSockets = (std::min)(static_cast<int32_t>(player.iEmberOrbs), iUnlockedSockets);
	const bool_t bHasEmberPool = 0u != player.iEmberMaximumSockets;
	m_pHUDRuntimeView->Set_SlotVisible("GK_Embereth_Frame", bHasEmberPool);
	for (int32_t i = 0; i < SOCKET_COUNT; ++i)
	{
		const string strIndex = std::to_string(i);
		m_pHUDRuntimeView->Set_SlotVisible("GK_Embereth_Socket" + strIndex, bHasEmberPool);

		const string strStateSlot = "GK_Embereth_State" + strIndex;
		m_pHUDRuntimeView->Set_SlotVisible(strStateSlot, bHasEmberPool);
		if (!bHasEmberPool)
		{
			m_EmberSocketStates[i].clear();
			continue;
		}
		// 이전 표시 상태와 달라진 socket만 lock/show/hide keyframe을 시작한다.
		// 같은 상태의 animation을 매 프레임 처음부터 재생하지 않도록 마지막 요청한 상태를 Client에 보관한다.
		const string strState = i >= iUnlockedSockets ? "lock" : (i < iFilledSockets ? "show" : "hide");
		if (m_EmberSocketStates[i] != strState)
		{
			m_pHUDRuntimeView->Play_KeyframeAnimation(strStateSlot, strState);
			m_EmberSocketStates[i] = strState;
		}
	}
}

// 일반 스킬 칸의 침묵·남은 쿨다운 마스크를 갱신한다. player와 HUD view의 유효성은 호출한 Update_CombatHUD의 진입 검사를 따른다.
void CMainApp::Update_SkillCooldowns()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.SkillCooldowns.Update");
	// Update_CombatHUD의 표시 경로에서 호출한다. Server tick 차이를 초로 바꾸는 이 표시 계산은 초당 30 tick을 사용한다.
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	constexpr const char *INPUT_SLOTS[] = {"Q", "W", "E", "R", "A", "S", "D", "F", "T", "V"};

	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	const float4_t vCooldownTint = float4_t(0.f, 0.f, 0.f, 150.f / 255.f);
	const float4_t vSilenceTint = float4_t(0.85f, 0.f, 0.f, 150.f / 255.f);
	const bool_t bSilenced = Is_ServerDeadlinePending(player.iServerTick, player.iSilenceEndTick);

	// Q~F·T·V의 기존 overlay를 먼저 초기화한다. 침묵 중에는 현재 player.Skills에 있는 칸만 빨간 전체 마스크로 덮고 아래 일반 쿨다운 계산은 건너뛴다.
	// 침묵이 아니면 남은 시간 비율을 arc mask로 표시하며 아이콘과 Server 쿨다운 값은 바꾸지 않는다. 초 단위 글자는 RenderSkillCooldownText가 따로 그린다.
	for (const char *pInputSlot : INPUT_SLOTS)
	{
		const string strOverlaySlot = string("Skill_") + pInputSlot + "_Cooldown";
		const bool_t bActiveSlot =
			std::any_of(player.Skills.begin(), player.Skills.end(),
						[pInputSlot](const HUD_SKILL_STATE &Skill) { return Skill.strInputSlot == pInputSlot; });
		m_pHUDRuntimeView->Set_SlotTint(strOverlaySlot, bSilenced ? vSilenceTint : vCooldownTint);
		m_pHUDRuntimeView->Set_SlotArcRatio(strOverlaySlot, bSilenced && bActiveSlot ? 1.f : 0.f);
		m_pHUDRuntimeView->Set_SlotVisible(strOverlaySlot, bSilenced && bActiveSlot);
	}
	if (bSilenced)
		return;

	for (const HUD_SKILL_STATE &Skill : player.Skills)
	{
		if (Skill.strInputSlot.empty() || Skill.Is_Ready(player.iServerTick))
			continue;

		// Z/X는 일반 arc overlay를 사용하지 않는다. 워로드의 아이콘 상태 전환과 도화가의 전용 keyframe 표현은 Update_CombatHUD가 처리한다.
		if ("Z" == Skill.strInputSlot || "X" == Skill.strInputSlot)
			continue;

		// 종료 tick이 현재보다 큰 경우만 차이를 사용한다. 총 지속 tick이 없으면 남은 시간을 전체 시간으로 삼아 마스크를 가득 채운다.
		const uint32_t remainingTicks =
			Skill.iCooldownEndTick > player.iServerTick ? Skill.iCooldownEndTick - player.iServerTick : 0u;
		if (0u == remainingTicks)
			continue;

		const f32_t fRemainingSeconds = static_cast<f32_t>(remainingTicks) / SERVER_TICK_HZ;
		const f32_t fTotalSeconds = Skill.iCooldownDurationTicks > 0u
										? static_cast<f32_t>(Skill.iCooldownDurationTicks) / SERVER_TICK_HZ
										: fRemainingSeconds;
		const f32_t fFraction =
			fTotalSeconds > 0.f ? (std::min)(1.f, (std::max)(0.f, fRemainingSeconds / fTotalSeconds)) : 0.f;

		const string strOverlaySlot = "Skill_" + Skill.strInputSlot + "_Cooldown";
		m_pHUDRuntimeView->Set_SlotArcRatio(strOverlaySlot, fFraction);
		m_pHUDRuntimeView->Set_SlotVisible(strOverlaySlot, true);
	}
}

// Render의 텍스트 단계에서 아이템 수량과 남은 쿨다운 숫자를 표시한다.
// 지원 Level·유효 snapshot·HUD view를 확인하고 스킬 창이 열렸으면 생략한다. 모든 tick과 수량은 표시용으로 읽으며 명령을 전송하지 않는다.
void CMainApp::RenderSkillCooldownText()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::COLOSSEUM) && currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) && currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) && currentLevel != ETOUI(LEVEL::KOUKUSAYDON_ARENA) &&
		!Is_WaterGunHudLevel(currentLevel, CCombatHUDViewModel::Get().Get_Player()))
	{
		return;
	}
	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || 0u == player.iMaximumHp || 0u == player.iMaximumResource)
		return;
	if ((nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open()) || nullptr == m_pHUDRuntimeView)
	{
		return;
	}

	constexpr f32_t SERVER_TICK_HZ = 30.f;
	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const auto DrawCooldownLabel = [&](f32_t fCenterX, f32_t fCenterY, f32_t fTargetHeight, const wstring &strLabel)
	{
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strLabel.c_str());
		const f32_t fScale = (vMeasured.y > 0.f) ? (fTargetHeight / vMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strLabel.c_str(),
									   float2_t(fCenterX * textScaleX + 1.f, fCenterY * textScaleY + 1.f),
									   XMVectorSet(0.f, 0.f, 0.f, 220.f / 255.f), 0.f, float2_t(0.5f, 0.5f),
									   fScale * textUiScale);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strLabel.c_str(),
									   float2_t(fCenterX * textScaleX, fCenterY * textScaleY), Colors::White, 0.f,
									   float2_t(0.5f, 0.5f), fScale * textUiScale);
	};

	// 슬롯에 보관한 item ID로 현재 Server 인벤토리를 조회한다. 장착하지 않은 첫 일치 항목의 수량을 쓰고 항목이 사라졌으면 0을 표시한다.
	for (int32_t i = 0; i < 4; ++i)
	{
		if (m_strItemQuickSlot[i].empty())
			continue;
		const auto *definition = CItemCatalog::Find_ById(m_strItemQuickSlot[i]);
		if (!definition || definition->strIconPath.empty())
			continue;
		const auto &inventory = CCombatHUDViewModel::Get().Get_Inventory().Items;
		const auto owned = std::find_if(inventory.begin(), inventory.end(),
										[&](const auto &item)
										{
											return item.strItemId == m_strItemQuickSlot[i] &&
												   item.eEquippedSlot == LostArk::Shared::EQUIPMENT_SLOT::NONE;
										});
		const wstring quantity = std::to_wstring(owned != inventory.end() ? owned->iQuantity : 0u);
		f32_t x, y, width, height;
		if (!m_pHUDRuntimeView->Get_SlotRect("Item_" + std::to_string(i + 1), x, y, width, height))
			continue;
		const float2_t measured = CGameInstance::Get().Measure_Text(TEXT("Font_YG330"), quantity.c_str());
		if (measured.y <= 0.f)
			continue;
		const f32_t scale = (11.f / measured.y) * textUiScale;
		const float2_t position((x + width - 2.f) * textScaleX, (y + height - 2.f) * textScaleY);
		CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), quantity.c_str(),
									   float2_t(position.x + 1.f, position.y + 1.f), XMVectorSet(0.f, 0.f, 0.f, 0.85f),
									   0.f, float2_t(1.f, 1.f), scale);
		CGameInstance::Get().Draw_Text(TEXT("Font_YG330"), quantity.c_str(), position, Colors::White, 0.f,
									   float2_t(1.f, 1.f), scale);
	}

	// 이번 HUD 갱신에서 모은 아이템·특수 슬롯·탈것·버프의 종료 tick을 초로 올림해 표시한다.
	// bUnderSlot은 아이콘 아래 숫자만 그리며 버프/디버프 색을 나누고, 나머지는 중앙에 s 접미사를 붙인다.
	// 이 목록을 그린 뒤에도 아래 별도 분기의 쿠크 또는 직업 스킬 글자 처리가 이어진다.
	constexpr f32_t BUFF_TEXT_PX = 10.f * (2.f / 3.f);
	const fvector_t vBuffText = XMVectorSet(155.f / 255.f, 217.f / 255.f, 121.f / 255.f, 1.f);
	const fvector_t vDebuffText = XMVectorSet(226.f / 255.f, 200.f / 255.f, 122.f / 255.f, 1.f);
	for (const HUD_TIMED_TEXT &Text : m_HudTimedTexts)
	{
		if (Text.iEndTick <= player.iServerTick)
			continue;
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect(Text.strSlotId, fX, fY, fWidth, fHeight))
			continue;
		const int32_t iSeconds =
			static_cast<int32_t>(std::ceil(static_cast<f32_t>(Text.iEndTick - player.iServerTick) / SERVER_TICK_HZ));
		const wstring strSeconds = std::to_wstring(iSeconds) + (Text.bUnderSlot ? L"" : L"s");
		if (!Text.bUnderSlot)
		{
			DrawCooldownLabel(fX + fWidth * 0.5f, fY + fHeight * 0.5f, fHeight * 0.34f, strSeconds);
			continue;
		}
		f32_t fScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(TEXT("Font_YG760"), BUFF_TEXT_PX * textUiScale, fScale);
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFont, strSeconds.c_str());
		const float2_t vPosition(std::round((fX + fWidth * 0.5f) * textScaleX - vMeasured.x * fScale * 0.5f),
								 std::round((fY + fHeight) * textScaleY + 1.f));
		CGameInstance::Get().Draw_Text(strFont, strSeconds.c_str(), float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
									   XMVectorSet(0.f, 0.f, 0.f, 0.75f), 0.f, float2_t(0.f, 0.f), fScale);
		CGameInstance::Get().Draw_Text(strFont, strSeconds.c_str(), vPosition, Text.bDebuff ? vDebuffText : vBuffText,
									   0.f, float2_t(0.f, 0.f), fScale);
	}

	// 쿠크 특수 HUD가 유효하면 해당 모드에서 보이는 칸만 표시한 뒤 반환해 일반 직업 스킬 숫자를 생략한다.
	const auto kouku = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	if (kouku.isValid && kouku.eHudMode != HUD_KOUKU_HUD_MODE::NONE && currentLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA))
	{
		for (std::size_t i = 0; i < HUD_KOUKU_SLOT_COUNT; ++i)
		{
			if (!kouku.Has_VisibleSkillSlot(i) || kouku.CooldownEndTicks[i] <= player.iServerTick)
				continue;
			float x, y, width, height;
			if (m_pHUDRuntimeView->Get_SlotRect(std::string("Skill_") + KOUKU_SLOT_KEYS[i] + "_Icon", x, y, width,
												height))
			{
				const auto seconds =
					static_cast<unsigned>(std::ceil((kouku.CooldownEndTicks[i] - player.iServerTick) / SERVER_TICK_HZ));
				DrawCooldownLabel(x + width * .5f, y + height * .5f, height * .34f, std::to_wstring(seconds) + L"s");
			}
		}
		return;
	}

	for (const HUD_SKILL_STATE &Skill : player.Skills)
	{
		if (Skill.strInputSlot.empty() || Skill.Is_Ready(player.iServerTick))
			continue;
		const uint32_t remainingTicks =
			Skill.iCooldownEndTick > player.iServerTick ? Skill.iCooldownEndTick - player.iServerTick : 0u;
		if (0u == remainingTicks)
			continue;
		const f32_t fRemainingSeconds = static_cast<f32_t>(remainingTicks) / SERVER_TICK_HZ;
		const int32_t iDisplaySeconds = static_cast<int32_t>(ceilf(fRemainingSeconds));
		const wstring strLabel = std::to_wstring(iDisplaySeconds) + L"s";

		if ("Z" == Skill.strInputSlot || "X" == Skill.strInputSlot)
		{
			// Z/X의 숫자는 도화가만 표시한다. 워로드 등 다른 직업은 이 분기에서 숫자를 건너뛰며 일반 arc overlay도 추가하지 않는다.
			if (LostArk::Shared::CHARACTER_CLASS_ID::ARTIST != player.eCharacterClass)
				continue;
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!m_pHUDRuntimeView->Get_SlotRect("Yin_Skill_" + Skill.strInputSlot, fX, fY, fWidth, fHeight))
			{
				continue;
			}
			// 도화가의 숫자 위치는 슬롯 시작점에 22.5 기준 픽셀 상자를 적용한다. 슬롯의 width·height 대신 기존 전용 표시 크기를 사용한다.
			DrawCooldownLabel(fX + 22.5f * 0.5f, fY + 22.5f * 0.5f, 22.5f * 0.34f, strLabel);
			continue;
		}

		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pHUDRuntimeView->Get_SlotRect("Skill_" + Skill.strInputSlot, fX, fY, fWidth, fHeight))
			continue;
		DrawCooldownLabel(fX + fWidth * 0.5f, fY + fHeight * 0.5f, fHeight * 0.34f, strLabel);
	}
}
// 보스 HUD를 닫을 때 mechanic 표시 허용·머리 anchor를 먼저 해제하고 지정된 보스 슬롯을 숨긴다.
// WorldHealthBarView의 개별 배우 체력바나 다른 HUD의 visible 상태까지 바꾸는 함수는 아니다.
void CMainApp::Hide_BossHealthBar()
{
	m_bMechanicBarUIAllowed = false;
	m_bMechanicBarHasHeadAnchor = false;
	if (nullptr == m_pBossUIView)
		return;
	constexpr const char_t *BOSS_UI_ALL_SLOTS[] = {
		"Boss_Frame",
		"Boss_FillBehind",
		"Boss_Fill",
		"Boss_StaggerBg",
		"Boss_StaggerFill",
		"Boss_StaggerTrack",
		"Boss_Separator",
		"Boss_TickFlash",
		"Boss_HitGlow",
		// 텍스트 위치용 슬롯도 이미지가 연결되어 있으면 자체 sprite를 가진다.
		// STATIC Layer에 남는 이 슬롯들을 함께 숨겨 Lobby 등 다음 화면에 placeholder 이미지가 남지 않게 한다. 글자는 별도 텍스트 함수가 그린다.
		"Boss_TitleText",
		"Boss_HPText",
		"Boss_BarCountText",
	};
	for (const char_t *pSlotId : BOSS_UI_ALL_SLOTS)
		m_pBossUIView->Set_SlotVisible(pSlotId, false);
}

// Debug 위치 편집기가 현재 미리보기 offset을 값으로 받는다. 저장 완료 기준은 별도의 Saved 배열이 보관한다.
std::array<f32_t, 12> CMainApp::Get_HealthBarPositions() const
{
	return m_HealthBarOffsets;
}

// 공통 mechanic X/Y와 아군·적·쿠크세이튼·쿠크·발탄의 X/Y offset을 검사해 미리보기에 적용한다.
// 모든 값은 HUD 기준 좌표이며 유한한 ±1280 범위만 받는다. 입력 실패는 기존 배열을 유지하고 파일 저장은 별도 Save가 수행한다.
bool_t CMainApp::Set_HealthBarPositions(const std::array<f32_t, 12> &offsets)
{
	for (const f32_t value : offsets)
		if (!std::isfinite(value) || std::abs(value) > 1280.f)
			return false;
	// 배열0/1은 mechanic용이며 뒤의 다섯 X/Y 쌍을 월드 체력바 view에 전달한다. 두 계층은 같은 finite·범위 검사를 사용한다.
	m_HealthBarOffsets = offsets;
	if (m_pWorldHealthBarView)
	{
		std::array<float2_t, 5> groups{};
		for (size_t i = 0u; i < groups.size(); ++i)
			groups[i] = float2_t(offsets[2u + i * 2u], offsets[3u + i * 2u]);
		(void)m_pWorldHealthBarView->Set_Offsets(groups);
	}
	Apply_MechanicBarRect();
	return true;
}

// 위치 편집기에 공통 mechanic의 가로·세로 배율을 값으로 반환한다. 발탄 전용 배율은 별도 tuning 배열에 있다.
std::array<f32_t, 2> CMainApp::Get_MechanicBarScale() const
{
	return m_MechanicBarScale;
}

// 공통 mechanic의 두 축 배율을 0.1~3 범위에서 적용한다. 발탄 보스일 때 실제 rect는 발탄 전용 tuning을 사용한다.
bool_t CMainApp::Set_MechanicBarScale(const std::array<f32_t, 2> &scale)
{
	for (const f32_t value : scale)
		if (!std::isfinite(value) || value < 0.1f || value > 3.f)
			return false;
	m_MechanicBarScale = scale;
	Apply_MechanicBarRect();
	return true;
}

// 발탄 무력화와 갑옷 파괴 표시의 offset X/Y·배율 X/Y 두 묶음을 검사해 보관한다.
// 무력화 rect는 즉시 갱신하고 갑옷 파괴 값은 Update_WorldHealthBars가 별도 view에 전달한다.
bool_t CMainApp::Set_ValtanMechanicTuning(const std::array<f32_t, 8> &tuning)
{
	for (size_t i = 0u; i < tuning.size(); ++i)
		if (!std::isfinite(tuning[i]) ||
			(i % 4u < 2u ? std::abs(tuning[i]) > 1280.f : tuning[i] < 0.1f || tuning[i] > 3.f))
			return false;
	m_ValtanMechanicTuning = tuning;
	Apply_MechanicBarRect();
	return true;
}

// 현재 보스가 발탄인지에 따라 배치 값을 골라 보관한 기본 rect에서 mechanic 슬롯을 재계산한다.
// view 또는 기본 rect가 없으면 표시를 바꾸지 않는다. 이 함수는 배치만 갱신하며 visible 여부는 anchor 갱신이 결정한다.
void CMainApp::Apply_MechanicBarRect()
{
	if (!m_pBossUIView)
		return;
	const bool valtan = IsValtanBossArchetype(CCombatHUDViewModel::Get().Get_Boss().strArchetypeId);
	const std::array<f32_t, 2> scale =
		valtan ? std::array<f32_t, 2>{m_ValtanMechanicTuning[2], m_ValtanMechanicTuning[3]} : m_MechanicBarScale;
	const float2_t offset = valtan ? float2_t(m_ValtanMechanicTuning[0], m_ValtanMechanicTuning[1])
								   : float2_t(m_HealthBarOffsets[0], m_HealthBarOffsets[1]);
	float2_t center{}, placementCenter{};
	bool hasCenter = false;
	for (size_t i = 0u; i < m_MechanicBarBaseRects.size(); ++i)
		if (m_MechanicBarHasBase[i])
		{
			const auto &base = m_MechanicBarBaseRects[i];
			center = float2_t(base.x + base.z * 0.5f, base.y + base.w * 0.5f);
			// 머리 anchor가 있으면 확대 후 frame 전체가 머리보다 기준 좌표 3만큼 위에 오도록 중심을 배치한다.
			placementCenter =
				m_bMechanicBarHasHeadAnchor
					? float2_t(m_MechanicBarHeadAnchor.x, m_MechanicBarHeadAnchor.y - 3.f - base.w * scale[1] * 0.5f)
					: center;
			hasCenter = true;
			break;
		}
	if (!hasCenter)
		return;
	// 보관한 저작 rect를 첫 유효 frame의 중심 주위로 확대한다. 이미 조절한 runtime rect를 재사용해 크기가 누적되지 않게 한다.
	for (size_t i = 0u; i < m_MechanicBarBaseRects.size(); ++i)
		if (m_MechanicBarHasBase[i])
		{
			const auto &base = m_MechanicBarBaseRects[i];
			m_pBossUIView->Set_SlotRect(
				HUD_MECHANIC_SLOTS[i], placementCenter.x + (base.x - center.x) * scale[0] + offset.x,
				placementCenter.y + (base.y - center.y) * scale[1] + offset.y, base.z * scale[0], base.w * scale[1]);
		}
}

// Update 끝에서 현재 카메라와 배우 presentation으로 mechanic 막대의 머리 기준 위치를 다시 구한다.
// 먼저 세 슬롯을 숨겨 투영·상태 검사 실패 시 이전 위치의 막대가 남지 않게 한다. 위치 정보가 없는 보스는 저작 배치를 사용한다.
void CMainApp::Update_MechanicBarAnchor()
{
	if (!m_pBossUIView)
		return;
	const auto hide = [&]()
	{
		for (const auto *slot : HUD_MECHANIC_SLOTS)
			m_pBossUIView->Set_SlotVisible(slot, false);
	};
	m_bMechanicBarHasHeadAnchor = false;
	hide();
	const auto &hud = CCombatHUDViewModel::Get();
	const auto &boss = hud.Get_Boss();
	using KIND = LostArk::Shared::BOSS_MECHANIC_GAUGE_KIND;
	// 발탄 Debug 미리보기는 유효하고 살아 있는 보스가 필요하지만 mechanic 종류·최대치 검사는 생략하고 비율0.65를 사용한다.
	const bool preview = m_bValtanStaggerDebug && IsValtanBossArchetype(boss.strArchetypeId) &&
						 CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::VALTAN_ARENA);
	if (!m_bMechanicBarUIAllowed || Is_RuntimeUIScreenSuppressed() || Is_KoukuMinigameHUDHidden() ||
		Is_MvpResultPageOpen() || !boss.isValid || !boss.iCurrentHp ||
		(!preview && (!boss.iMaximumMechanicGauge ||
					  (boss.eMechanicGaugeKind != KIND::STAGGER && boss.eMechanicGaugeKind != KIND::BOSS_HP))))
		return;
	// 위치가 있는 보스는 같은 entity의 살아 있는 비player 표시 상태를 찾는다. weak presentation으로 머리 위치를 구하지 못하면 숨긴 상태를 유지한다.
	if (boss.hasPosition)
	{
		const auto &states = hud.Get_WorldHealthBars();
		const auto state = std::find_if(
			states.begin(), states.end(), [&](const auto &value)
			{ return !value.isPlayer && value.iNetEntityId == boss.iNetEntityId && value.iCurrentHp > 0u; });
		if (state == states.end() || boss.iNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
			return;
		auto &game = CGameInstance::Get();
		const auto *view = game.Get_Transform(D3DTS::VIEW);
		const auto *projection = game.Get_Transform(D3DTS::PROJ);
		const auto viewport = game.Get_ViewportSize();
		float3_t head{};
		float2_t screen{};
		if (!view || !projection || !CWorldHealthBarView::Try_GetHeadAnchor(*state, head) ||
			!CWorldPlayerNameplateView::Try_ProjectWorldPosition(head, *view, *projection, viewport, screen))
			return;
		// 투영 결과의 화면 픽셀 좌표를 Boss UI 문서의 기준 해상도로 바꾸어 저장한다.
		m_MechanicBarHeadAnchor = float2_t(screen.x * m_pBossUIView->Get_ResolutionWidth() / viewport.x,
										   screen.y * m_pBossUIView->Get_ResolutionHeight() / viewport.y);
		m_bMechanicBarHasHeadAnchor = true;
	}
	Apply_MechanicBarRect();
	const f32_t ratio = preview
							? 0.65f
							: static_cast<f32_t>((std::min)(boss.iCurrentMechanicGauge, boss.iMaximumMechanicGauge)) /
								  static_cast<f32_t>(boss.iMaximumMechanicGauge);
	m_pBossUIView->Set_SlotVisible("Boss_StaggerBg", true);
	m_pBossUIView->Set_SlotVisible("Boss_StaggerTrack", false);
	m_pBossUIView->Set_SlotFillRatio("Boss_StaggerFill", ratio);
	m_pBossUIView->Set_SlotVisible("Boss_StaggerFill", ratio > 0.f);
}

// Debug 편집기가 디스크의 공통·발탄 체력바 배치를 다시 읽는다. 세 배열을 함께 검증한 뒤 미리보기와 Saved 기준을 갱신한다.
// 읽기·파싱·검증 실패는 현재 미리보기와 저장 기준을 유지하며 status로 이유를 반환한다.
bool_t CMainApp::Reload_HealthBarPositions(std::string &status)
{
	std::string text, error;
	DATA_JSON_VALUE root;
	std::array<f32_t, 12> staged{};
	std::array<f32_t, 2> stagedScale{};
	std::array<f32_t, 8> stagedValtan{};
	if (!ReadHudBarText(CProjectDataRoot::Resolve(L"UI/KoukuSaydon/KoukuHudModes.json"), text) ||
		!CDataJson::Parse(text, root, error) || !ReadHudBarPositions(root, staged, stagedScale, stagedValtan))
	{
		status = "Invalid health bar positions; current preview preserved. " + error;
		return false;
	}
	(void)Set_HealthBarPositions(staged);
	(void)Set_MechanicBarScale(stagedScale);
	m_SavedHealthBarOffsets = staged;
	m_SavedMechanicBarScale = stagedScale;
	(void)Set_ValtanMechanicTuning(stagedValtan);
	m_SavedValtanMechanicTuning = stagedValtan;
	status = "Reloaded saved health bar positions and stagger size.";
	return true;
}

// Debug 저장 명령이 현재 미리보기에서 바꾼 field만 최신 디스크 문서에 병합한다.
// 같은 field의 외부 변경은 거절하고 무관한 값은 최신 문서에서 보존한다. 성공 뒤에만 미리보기·Saved 기준을 갱신한다.
// 교체 뒤 경합 감지는 자기 저장본이 남아 있을 때 복구를 시도하지만 복구 API의 성공을 보장하지 않는다.
bool_t CMainApp::Save_HealthBarPositions(std::string &status)
{
	const std::filesystem::path path = CProjectDataRoot::Resolve(L"UI/KoukuSaydon/KoukuHudModes.json");
	// 다른 Client의 광기 위치 저장과 같은 문서 잠금 파일을 사용한다. 공유 모드0으로 연 handle을 함수 끝까지 유지한다.
	const std::filesystem::path lockPath = path.wstring() + L".madness-position.lock";
	const HANDLE lock = CreateFileW(lockPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0u, nullptr, OPEN_ALWAYS,
									FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
	if (lock == INVALID_HANDLE_VALUE)
	{
		status = "HUD position save is already in progress.";
		return false;
	}
	struct LOCK_GUARD
	{
		HANDLE handle;
		~LOCK_GUARD()
		{
			CloseHandle(handle);
		}
	} guard{lock};
	std::string before, error;
	DATA_JSON_VALUE root;
	std::array<f32_t, 12> saved{};
	std::array<f32_t, 2> savedScale{};
	std::array<f32_t, 8> savedValtan{};
	if (!ReadHudBarText(path, before) || !CDataJson::Parse(before, root, error) ||
		!ReadHudBarPositions(root, saved, savedScale, savedValtan))
	{
		status = "HUD configuration could not be read; disk and preview preserved. " + error;
		return false;
	}
	// 저장 기준과 미리보기를 비교해 사용자가 바꾼 field를 찾는다. 해당 field의 디스크 값도 저장 기준과 달라졌으면 충돌로 중단한다.
	std::array<bool, 12> changed{};
	bool anyChanged = false;
	for (size_t i = 0u; i < changed.size(); ++i)
	{
		changed[i] = m_HealthBarOffsets[i] != m_SavedHealthBarOffsets[i];
		anyChanged |= changed[i];
		if (changed[i] && saved[i] != m_SavedHealthBarOffsets[i])
		{
			status = "Edited position field changed on disk. Reload saved positions before saving; preview preserved.";
			return false;
		}
	}
	std::array<bool, 2> scaleChanged{};
	for (size_t i = 0u; i < scaleChanged.size(); ++i)
	{
		scaleChanged[i] = m_MechanicBarScale[i] != m_SavedMechanicBarScale[i];
		anyChanged |= scaleChanged[i];
		if (scaleChanged[i] && savedScale[i] != m_SavedMechanicBarScale[i])
		{
			status = "Edited stagger size field changed on disk. Reload before saving; preview preserved.";
			return false;
		}
	}
	std::array<bool, 8> valtanChanged{};
	for (size_t i = 0u; i < valtanChanged.size(); ++i)
	{
		valtanChanged[i] = m_ValtanMechanicTuning[i] != m_SavedValtanMechanicTuning[i];
		anyChanged |= valtanChanged[i];
		if (valtanChanged[i] && savedValtan[i] != m_SavedValtanMechanicTuning[i])
		{
			status = "Edited Valtan HUD field changed on disk. Reload before saving; preview preserved.";
			return false;
		}
	}
	// 로컬 편집이 없으면 파일을 쓰지 않고 현재 디스크의 배치를 미리보기와 저장 기준으로 받아들인다.
	if (!anyChanged)
	{
		(void)Set_HealthBarPositions(saved);
		m_SavedHealthBarOffsets = saved;
		(void)Set_MechanicBarScale(savedScale);
		m_SavedMechanicBarScale = savedScale;
		(void)Set_ValtanMechanicTuning(savedValtan);
		m_SavedValtanMechanicTuning = savedValtan;
		status = "No edits; refreshed health bar positions and stagger size from disk.";
		return true;
	}
	// 최신 문서의 전체 object를 복사하고 변경한 배치 field만 교체한다. 빠진 알려진 key에는 검증된 기본값을 채운다.
	auto fields = root.Get_Object();
	const auto *previous = root.Find("healthBarPositions");
	auto positions = previous ? previous->Get_Object() : DATA_JSON_VALUE::OBJECT{};
	for (size_t i = 0u; i < changed.size(); ++i)
		if (changed[i] || !positions.contains(HUD_BAR_POSITION_KEYS[i]))
			positions[HUD_BAR_POSITION_KEYS[i]] =
				DATA_JSON_VALUE::Number(changed[i] ? m_HealthBarOffsets[i] : saved[i]);
	for (size_t i = 0u; i < scaleChanged.size(); ++i)
		if (scaleChanged[i] || !positions.contains(HUD_MECHANIC_SCALE_KEYS[i]))
			positions[HUD_MECHANIC_SCALE_KEYS[i]] =
				DATA_JSON_VALUE::Number(scaleChanged[i] ? m_MechanicBarScale[i] : savedScale[i]);
	for (size_t i = 0u; i < valtanChanged.size(); ++i)
		if (valtanChanged[i] || !positions.contains(HUD_VALTAN_MECHANIC_KEYS[i]))
			positions[HUD_VALTAN_MECHANIC_KEYS[i]] =
				DATA_JSON_VALUE::Number(valtanChanged[i] ? m_ValtanMechanicTuning[i] : savedValtan[i]);
	fields["healthBarPositions"] = DATA_JSON_VALUE::Object(std::move(positions));
	std::ostringstream serialized;
	serialized.imbue(std::locale::classic());
	WriteHudBarJson(serialized, DATA_JSON_VALUE::Object(std::move(fields)));
	const std::string after = serialized.str() + "\n";
	// 직렬화한 문서를 다시 파싱·범위 검사한다. 검증된 세 배열은 저장 성공 뒤 반영할 최종 값이 된다.
	DATA_JSON_VALUE verified;
	if (!CDataJson::Parse(after, verified, error) || !ReadHudBarPositions(verified, saved, savedScale, savedValtan))
	{
		status = "Position serialization rejected; disk and preview preserved.";
		return false;
	}
	// 원본 옆에 process·시각 접미사의 임시 파일과 백업 경로를 만든다. 이 코드는 파일 교체 전에 임시 본문을 flush하고 다시 읽는다.
	const auto suffix = std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(GetTickCount64());
	const std::filesystem::path temporary = path.wstring() + L".tmp." + suffix;
	const std::filesystem::path backup = path.wstring() + L".backup." + suffix;
	{
		std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
		stream.write(after.data(), static_cast<std::streamsize>(after.size()));
		stream.flush();
		if (!stream.good())
		{
			stream.close();
			DeleteFileW(temporary.c_str());
			status = "Position temporary write failed; source preserved.";
			return false;
		}
	}
	// 임시 파일 byte가 예상과 같은지 확인하고 원본도 최초 읽은 byte와 대조한 뒤 ReplaceFileW로 교체한다.
	// 잠금 규칙을 따르지 않는 외부 writer와의 경합은 교체 뒤 백업 대조로 한 번 더 검사한다.
	std::string current;
	if (!ReadHudBarText(temporary, current) || current != after || !ReadHudBarText(path, current) || current != before)
	{
		DeleteFileW(temporary.c_str());
		status = "HUD source changed during save; disk and preview preserved.";
		return false;
	}
	if (!ReplaceFileW(path.c_str(), temporary.c_str(), backup.c_str(), 0u, nullptr, nullptr))
	{
		DeleteFileW(temporary.c_str());
		status = "Position atomic replace failed; backup preserved at " + backup.string();
		return false;
	}
	// 교체로 밀려난 백업이 최초 본문과 다르면 동시 저장으로 판단한다. 원본이 아직 자기 after인 경우에만 백업 복구를 시도하고 복구 자료를 남긴다.
	std::string displaced;
	if (!ReadHudBarText(backup, displaced) || displaced != before)
	{
		if (ReadHudBarText(path, current) && current == after)
		{
			const std::filesystem::path recovery = path.wstring() + L".conflict." + suffix;
			(void)ReplaceFileW(path.c_str(), backup.c_str(), recovery.c_str(), 0u, nullptr, nullptr);
		}
		status = "Concurrent HUD save detected; recovery files preserved beside source. Reload before retrying.";
		return false;
	}
	(void)Set_HealthBarPositions(saved);
	m_SavedHealthBarOffsets = saved;
	(void)Set_MechanicBarScale(savedScale);
	m_SavedMechanicBarScale = savedScale;
	(void)Set_ValtanMechanicTuning(savedValtan);
	m_SavedValtanMechanicTuning = savedValtan;
	status = "Saved health bar positions and stagger size. Backup: " + backup.string();
	return true;
}

// Update 끝에서 복제된 월드 체력바 목록을 값으로 복사해 view에 전달한다.
// 쿠크 관전/연출 정책에 따라 숨길 player만 복사본에서 제거하고 실제 HUD snapshot이나 Server 상태는 바꾸지 않는다.
void CMainApp::Update_WorldHealthBars(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.WorldHealthBars.Update");
	if (nullptr == m_pWorldHealthBarView)
		return;
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isSupportedLevel = currentLevel == ETOUI(LEVEL::BERN) || currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
									currentLevel == ETOUI(LEVEL::DEVELOPMENT) ||
									currentLevel == ETOUI(LEVEL::CHARACTER_SELECT) ||
									currentLevel == ETOUI(LEVEL::MAHARAKA) || currentLevel == ETOUI(LEVEL::COLOSSEUM) ||
									currentLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA);
	const bool_t characterPresentation = currentLevel == ETOUI(LEVEL::CHARACTER_SELECT) &&
										 CLevel_CharacterSelect::Get_Active() &&
										 CLevel_CharacterSelect::Get_Active()->Is_ProductPresentationOpen();
	const auto &hud = CCombatHUDViewModel::Get();
	const auto gimmick = hud.Get_KoukuGimmick();
	const auto *arena =
		currentLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA) ? CLevel_KoukuSaydonArena::Get_Active() : nullptr;
	const auto *subject = arena ? arena->Get_CameraPlayerSnapshot() : nullptr;
	const bool cardMazeActive = subject ? subject->eKoukuHudMode == LostArk::Shared::KOUKU_HUD_MODE::MAZE
										: (LostArk::Shared::CARD_MAZE_ROLE::NONE != gimmick.eCardMazeRole ||
										   LostArk::Shared::KOUKU_HUD_MODE::MAZE == hud.Get_Player().eKoukuHudMode);
	// 표시할 목록은 복사본이다. camera subject가 있으면 그 subject의 쿠크 모드를 우선해 카드 미로 상태를 판단한다.
	auto worldBars = hud.Get_WorldHealthBars();
	if (arena)
		std::erase_if(worldBars, [arena](const auto &state)
					  { return state.isPlayer && !arena->Should_ShowPlayerWorldUI(state.iNetEntityId); });
	// 발탄 tuning의 뒤 네 값은 갑옷 파괴 표시가 소비한다. 화면 허용 여부와 카드 미로 상태는 view의 Update에 따로 전달한다.
	(void)m_pWorldHealthBarView->Set_ValtanArmorBreakTuning(
		float2_t(m_ValtanMechanicTuning[4], m_ValtanMechanicTuning[5]),
		float2_t(m_ValtanMechanicTuning[6], m_ValtanMechanicTuning[7]),
		m_bValtanArmorBreakDebug && currentLevel == ETOUI(LEVEL::VALTAN_ARENA));
	m_pWorldHealthBarView->Update(fTimeDelta, worldBars,
								  isSupportedLevel && !Is_RuntimeUIScreenSuppressed() && !characterPresentation &&
									  !Is_MvpResultPageOpen(),
								  cardMazeActive);
}
// Update가 복제 보스 상태를 읽어 화면 고정 HP 막대와 피격 효과의 표시 값을 갱신한다.
// 지원 Level·창 상태·snapshot 검사를 통과하지 못하면 보스 슬롯을 숨기고 mechanic 표시 허용도 해제한다. Server HP와 줄 수는 읽기만 한다.
void CMainApp::Update_BossHealthBar()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.BossHealthBar.Update");
	if (nullptr == m_pBossUIView)
	{
		return;
	}

	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isSupportedLevel =
		currentLevel == ETOUI(LEVEL::BERN) || currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
		currentLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA) || currentLevel == ETOUI(LEVEL::DEVELOPMENT) ||
		currentLevel == ETOUI(LEVEL::CHARACTER_SELECT) || currentLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA);
	const bool_t skillWindowOpen = nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	// Debug 레이드 입장 preview의 정보 패널이 보스 HUD 영역과 겹치므로 해당 화면의 열림 상태를 표시 조건에 포함한다.
#ifdef _DEBUG
	const bool_t isCharSelectDebugPreviewOpen = ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel &&
												nullptr != CLevel_CharacterSelect::Get_Active() &&
												CLevel_CharacterSelect::Get_Active()->Is_DebugRaidEntryPreviewOpen();
#else
	const bool_t isCharSelectDebugPreviewOpen = false;
#endif
	// 캐릭터 생성·제품 presentation도 같은 Level 위의 화면이므로 보스 HUD를 함께 숨기는 조건에 포함한다.
	const bool_t isCharSelectOverlayOpen =
		isCharSelectDebugPreviewOpen ||
		(ETOUI(LEVEL::CHARACTER_SELECT) == currentLevel && nullptr != CLevel_CharacterSelect::Get_Active() &&
		 CLevel_CharacterSelect::Get_Active()->Is_ProductPresentationOpen());

	const HUD_BOSS_STATE &boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!isSupportedLevel || skillWindowOpen || isCharSelectOverlayOpen || Is_KoukuBossHealthBarHidden() ||
		Is_MvpResultPageOpen() || !boss.isValid || 0u == boss.iMaximumHp)
	{
		Hide_BossHealthBar();
		return;
	}

	// Server의 최대 HP를 체력 줄 수로 나눠 현재 줄의 채움과 남은 줄 수를 구한다. 전투 HP는 바꾸지 않고 한 줄씩 줄어드는 보스 HUD로 표현한다.
	const uint32_t iMaximumBars = (std::max)(1u, boss.iMaximumHealthBars);
	const double segmentHp = static_cast<double>(boss.iMaximumHp) / static_cast<double>(iMaximumBars);
	float healthRatio = 0.f;
	uint32_t iBarsRemaining = 0u;
	if (boss.iCurrentHp > 0u && segmentHp > 0.0)
	{
		const double scaledHp = static_cast<double>(boss.iCurrentHp) / segmentHp;
		const double fraction = scaledHp - std::floor(scaledHp);
		// 줄 경계에 정확히 도달한 양수 HP는 다음 남은 줄이 가득 찬 상태다. 최대 HP일 때도 비율1로 표시한다.
		healthRatio = static_cast<float>(0.0 == fraction ? 1.0 : fraction);
		iBarsRemaining = (std::clamp)(static_cast<uint32_t>(std::ceil(scaledHp)), 1u, iMaximumBars);
	}

	// 직전 표시와 비교해 줄 감소·HP 감소 시각을 기록한다. archetype 또는 최대 HP가 바뀌면 비교 기준과 효과 시각을 초기화한다.
	// 여기서는 entity ID까지 비교하지 않으므로 같은 archetype·최대 HP를 가진 다른 대상의 교체를 별도로 구분하지 않는다.
	if (boss.strArchetypeId != m_strPreviousBossArchetypeId || boss.iMaximumHp != m_iPreviousBossMaximumHp)
	{
		m_strPreviousBossArchetypeId = boss.strArchetypeId;
		m_iPreviousBossMaximumHp = boss.iMaximumHp;
		m_iPreviousBossBarsRemaining = iBarsRemaining;
		m_iPreviousBossCurrentHp = boss.iCurrentHp;
		m_dBossBarTickFlashStartSeconds = -1.0;
		m_dBossHitGlowStartSeconds = -1.0;
	}
	else
	{
		if (iBarsRemaining < m_iPreviousBossBarsRemaining)
			m_dBossBarTickFlashStartSeconds = Product_Now_Seconds();
		if (boss.iCurrentHp < m_iPreviousBossCurrentHp)
		{
			m_dBossHitGlowStartSeconds = Product_Now_Seconds();
			m_fBossHitGlowFillRatio = healthRatio;
		}
		m_iPreviousBossBarsRemaining = iBarsRemaining;
		m_iPreviousBossCurrentHp = boss.iCurrentHp;
	}

	// Boss UI 문서의 슬롯별 위치·크기를 사용한다. HP 채움 슬롯을 찾지 못하면 보스 HUD를 숨긴다.
	// 이 함수의 separator/flash/glow는 채움 rect에서 계산하며, mechanic 슬롯의 머리 배치는 별도 anchor 갱신이 맡는다.
	f32_t fFillRectX = 0.f, fFillRectY = 0.f, fFillRectWidth = 0.f, fFillRectHeight = 0.f;
	if (!m_pBossUIView->Get_SlotRect("Boss_Fill", fFillRectX, fFillRectY, fFillRectWidth, fFillRectHeight))
	{
		Hide_BossHealthBar();
		return;
	}
	// 위 표시 조건을 통과하면 HP frame을 켠다. 이 경로 자체는 현재 HP0을 거절하지 않으며 mechanic 막대는 별도 상태 검사를 거친다.
	m_pBossUIView->Set_SlotVisible("Boss_Frame", true);

	// 현재 줄 수를 1부터 시작하는 여섯 색 순환 index로 바꾼다.
	// 목록은 청록·파랑·보라·주황·빨강·초록이며 현재 줄의 이미지만 채움 비율로 자른다.
	constexpr const char *FILL_COLOR_CYCLE[] = {
		"UI/BossUI/boss_bar_fill_teal.png",	  "UI/BossUI/boss_bar_fill_blue.png", "UI/BossUI/boss_bar_fill_purple.png",
		"UI/BossUI/boss_bar_fill_orange.png", "UI/BossUI/boss_bar_fill_red.png",  "UI/BossUI/boss_bar_fill_green.png",
	};
	constexpr uint32_t FILL_COLOR_COUNT = static_cast<uint32_t>(sizeof(FILL_COLOR_CYCLE) / sizeof(FILL_COLOR_CYCLE[0]));
	const uint32_t iColorCycleValue = (std::max)(1u, iBarsRemaining);
	const uint32_t iColorIndex =
		((iColorCycleValue % FILL_COLOR_COUNT == 0) ? FILL_COLOR_COUNT : (iColorCycleValue % FILL_COLOR_COUNT)) - 1u;

	m_pBossUIView->Set_SlotTexture("Boss_Fill", FILL_COLOR_CYCLE[iColorIndex]);
	m_pBossUIView->Set_SlotFillRatio("Boss_Fill", healthRatio > 0.f ? healthRatio : 0.f);
	m_pBossUIView->Set_SlotVisible("Boss_Fill", healthRatio > 0.f);

	// separator와 효과 배치는 문서 기준 좌표에서 계산한다. 실제 화면 배율은 UI sprite 표시 경로가 처리하므로 여기서 다시 곱하지 않는다.
	const f32_t fFillInset = 2.f;
	const f32_t fFillMinX = fFillRectX + fFillInset;
	const f32_t fFillMinY = fFillRectY + fFillInset;
	const f32_t fFillMaxX = fFillRectX + fFillRectWidth - fFillInset;
	const f32_t fFillMaxY = fFillRectY + fFillRectHeight - fFillInset;
	const f32_t fFillBoundaryX = fFillMinX + (fFillMaxX - fFillMinX) * healthRatio;

	// 두 줄 이상 남아 있으면 다음 줄의 색을 뒤쪽 슬롯으로 켠다. 마지막 한 줄과 HP0에서는 뒤쪽 슬롯을 숨긴다.
	// 뒤쪽 슬롯의 위치·크기·채움 비율을 여기서 다시 설정하지는 않는다.
	if (iBarsRemaining > 1u)
	{
		const uint32_t iNextColorCycleValue = iBarsRemaining - 1u;
		const uint32_t iNextColorIndex =
			((iNextColorCycleValue % FILL_COLOR_COUNT == 0) ? FILL_COLOR_COUNT
															: (iNextColorCycleValue % FILL_COLOR_COUNT)) -
			1u;
		m_pBossUIView->Set_SlotTexture("Boss_FillBehind", FILL_COLOR_CYCLE[iNextColorIndex]);
		m_pBossUIView->Set_SlotVisible("Boss_FillBehind", true);
	}
	else
	{
		m_pBossUIView->Set_SlotVisible("Boss_FillBehind", false);
	}

	// mechanic 막대 갱신을 허용한다. 실제 보스 머리 위치·최종 카메라·Server mechanic 값은 Update 끝의 anchor 갱신에서 다시 검사한다.
	// 화면에 고정된 HP 막대와 머리를 따라가는 mechanic 막대는 별도 배치 경로를 사용한다.
	m_bMechanicBarUIAllowed = true;

	// 현재 줄이 일부만 찼을 때 채움 경계에 폭6의 separator를 놓는다. 비율0과1에서는 경계 표시를 숨긴다.
	const bool_t bShowSeparator = healthRatio > 0.f && healthRatio < 1.f;
	m_pBossUIView->Set_SlotVisible("Boss_Separator", bShowSeparator);
	if (bShowSeparator)
	{
		constexpr f32_t SEPARATOR_HALF_WIDTH = 3.f;
		m_pBossUIView->Set_SlotRect("Boss_Separator", fFillBoundaryX - SEPARATOR_HALF_WIDTH, fFillMinY,
									SEPARATOR_HALF_WIDTH * 2.f, fFillMaxY - fFillMinY);
	}

	// 줄 수가 감소한 뒤 0.3초 동안 채움 내부 rect의 흰 flash alpha를 선형으로 줄인다.
	// 아래 분기는 색과 rect를 갱신하며 visible을 true로 되돌리는 호출은 포함하지 않는다.
	bool_t bShowTickFlash = false;
	if (m_dBossBarTickFlashStartSeconds >= 0.0)
	{
		constexpr f64_t BAR_TICK_FLASH_SECONDS = 0.3;
		const f64_t fFlashAge = Product_Now_Seconds() - m_dBossBarTickFlashStartSeconds;
		if (fFlashAge < BAR_TICK_FLASH_SECONDS)
		{
			bShowTickFlash = true;
			const f32_t fFlashAlpha = 1.f - static_cast<f32_t>(fFlashAge / BAR_TICK_FLASH_SECONDS);
			m_pBossUIView->Set_SlotRect("Boss_TickFlash", fFillMinX, fFillMinY, fFillMaxX - fFillMinX,
										fFillMaxY - fFillMinY);
			// Set_SlotAlpha는 RGB를 흰색으로 맞추고 alpha를 기록한다. 현재 CUI_Sprite의 Set_Visible과는 별개이므로 이미 숨긴 sprite를 이 호출만으로 다시 켜지지는 않는다.
			m_pBossUIView->Set_SlotAlpha("Boss_TickFlash", (220.f / 255.f) * fFlashAlpha);
		}
	}
	if (!bShowTickFlash)
		m_pBossUIView->Set_SlotVisible("Boss_TickFlash", false);

	// HP 감소 순간 저장한 채움 경계에서 0.35초 동안 반경을6에서16으로 늘리고 glow alpha를 줄인다.
	// 위치는 현재 HP가 아닌 피격 순간의 비율을 사용하며, 이 분기도 visible을 true로 바꾸지 않는다.
	bool_t bShowHitGlow = false;
	if (m_dBossHitGlowStartSeconds >= 0.0)
	{
		constexpr f64_t HIT_GLOW_SECONDS = 0.35;
		const f64_t fGlowAge = Product_Now_Seconds() - m_dBossHitGlowStartSeconds;
		if (fGlowAge < HIT_GLOW_SECONDS)
		{
			bShowHitGlow = true;
			const f32_t fGlowT = static_cast<f32_t>(fGlowAge / HIT_GLOW_SECONDS);
			const f32_t fGlowAlpha = 1.f - fGlowT;
			const f32_t fGlowRadius = 6.f + 10.f * fGlowT;
			const f32_t fGlowCenterX = fFillMinX + (fFillMaxX - fFillMinX) * m_fBossHitGlowFillRatio;
			const f32_t fGlowCenterY = (fFillMinY + fFillMaxY) * 0.5f;
			m_pBossUIView->Set_SlotRect("Boss_HitGlow", fGlowCenterX - fGlowRadius, fGlowCenterY - fGlowRadius,
										fGlowRadius * 2.f, fGlowRadius * 2.f);
			// glow는 따뜻한 흰색 RGB를 유지하도록 Set_SlotTint로 alpha까지 함께 쓴다. 이 API는 tint만 바꾸며 숨김 여부는 별도 상태다.
			m_pBossUIView->Set_SlotTint("Boss_HitGlow",
										float4_t(1.f, 250.f / 255.f, 230.f / 255.f, (200.f / 255.f) * fGlowAlpha));
		}
	}
	if (!bShowHitGlow)
		m_pBossUIView->Set_SlotVisible("Boss_HitGlow", false);
}

// Render의 텍스트 단계가 보스 이름·HP·남은 줄 수를 그린다. 같은 HUD snapshot에서 줄 수를 다시 계산해 별도 캐시 상태를 만들지 않는다.
// 문자열은 CGameInstance::Draw_Text로 제출하며 sprite의 visible 상태 자체를 텍스트 표시 조건으로 읽지는 않는다.
// 호출한 Render가 전체 화면·MVP·미니게임 숨김을 먼저 검사하며, 이 함수도 Level·스킬 창·보스 값·view를 확인한다.
// 텍스트 슬롯 하나가 없으면 해당 글자만 생략하고 나머지 글자는 계속 그린다.
void CMainApp::RenderBossHealthBarText()
{
	if (Is_KoukuBossHealthBarHidden())
		return;
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) && currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::KOUKUSAYDON_ARENA) && currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) && currentLevel != ETOUI(LEVEL::KOUKUSAYDON_ARENA))
	{
		return;
	}
	if (nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open())
		return;

	const HUD_BOSS_STATE &boss = CCombatHUDViewModel::Get().Get_Boss();
	if (!boss.isValid || 0u == boss.iMaximumHp)
		return;
	if (nullptr == m_pBossUIView)
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	const uint32_t iMaximumBars = (std::max)(1u, boss.iMaximumHealthBars);
	const double segmentHp = static_cast<double>(boss.iMaximumHp) / static_cast<double>(iMaximumBars);
	uint32_t iBarsRemaining = 0u;
	if (boss.iCurrentHp > 0u && segmentHp > 0.0)
	{
		const double scaledHp = static_cast<double>(boss.iCurrentHp) / segmentHp;
		iBarsRemaining = (std::clamp)(static_cast<uint32_t>(std::ceil(scaledHp)), 1u, iMaximumBars);
	}
	const bool_t isMultiBar = boss.iMaximumHealthBars > 1u;

	// Boss_TitleText에 BOSS_VALTAN은 "마수군단장 발탄", 다른 보스는 HUD의 displayName을 쓴다. 이름 변환 실패·빈 이름은 BOSS로 표시한다.
	f32_t fTitleX = 0.f, fTitleY = 0.f, fTitleWidth = 0.f, fTitleHeight = 0.f;
	if (m_pBossUIView->Get_SlotRect("Boss_TitleText", fTitleX, fTitleY, fTitleWidth, fTitleHeight))
	{
		wstring strBossTitle;
		if ("BOSS_VALTAN" == boss.strArchetypeId)
			strBossTitle = L"\xB9C8\xC218\xAD70\xB2E8\xC7A5 \xBC1C\xD0C4";
		else if (!ConvertUtf8ToWide(boss.strDisplayName, strBossTitle) || strBossTitle.empty())
			strBossTitle = L"BOSS";
		// 측정한 글자 높이와 제목 슬롯 높이의 비율로 배율을 구한다. 위치는 축별 viewport 배율, 글자 크기는 작은 축 배율을 사용한다.
		const float2_t vTitleMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), strBossTitle.c_str());
		const f32_t fTitleScale = (vTitleMeasured.y > 0.f) ? (fTitleHeight / vTitleMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(
			TEXT("Font_YG760"), strBossTitle.c_str(),
			float2_t((fTitleX + fTitleWidth * 0.5f) * textScaleX, (fTitleY + fTitleHeight * 0.5f) * textScaleY),
			XMVectorSet(1.f, 0.31f, 0.24f, 1.f), 0.f, float2_t(0.5f, 0.5f), fTitleScale * textUiScale);
	}

	// Boss_HPText는 현재 / 최대 HP, Boss_BarCountText는 두 줄 이상 남은 다중 체력 보스의 X 줄 수를 표시한다.
	f32_t fHpTextX = 0.f, fHpTextY = 0.f, fHpTextWidth = 0.f, fHpTextHeight = 0.f;
	if (m_pBossUIView->Get_SlotRect("Boss_HPText", fHpTextX, fHpTextY, fHpTextWidth, fHpTextHeight))
	{
		const wstring strHpNumbers = std::to_wstring(boss.iCurrentHp) + L" / " + std::to_wstring(boss.iMaximumHp);
		// HP 숫자도 글자 측정 높이에 대한 슬롯 높이 비율을 사용한다. 측정 높이가0 이하이면 배율1을 기본으로 쓴다.
		const float2_t vHpMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), strHpNumbers.c_str());
		const f32_t fHpScale = (vHpMeasured.y > 0.f) ? (fHpTextHeight / vHpMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(
			TEXT("Font_YG760"), strHpNumbers.c_str(),
			float2_t((fHpTextX + fHpTextWidth * 0.5f) * textScaleX, (fHpTextY + fHpTextHeight * 0.5f) * textScaleY),
			Colors::White, 0.f, float2_t(0.5f, 0.5f), fHpScale * textUiScale);
	}
	if (isMultiBar && iBarsRemaining > 1u)
	{
		f32_t fCountX = 0.f, fCountY = 0.f, fCountWidth = 0.f, fCountHeight = 0.f;
		if (m_pBossUIView->Get_SlotRect("Boss_BarCountText", fCountX, fCountY, fCountWidth, fCountHeight))
		{
			const wstring strBarCount = L"X " + std::to_wstring(iBarsRemaining);
			const float2_t vCountMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), strBarCount.c_str());
			const f32_t fCountScale = (vCountMeasured.y > 0.f) ? (fCountHeight / vCountMeasured.y) : 1.f;
			CGameInstance::Get().Draw_Text(
				TEXT("Font_YG760"), strBarCount.c_str(),
				float2_t((fCountX + fCountWidth * 0.5f) * textScaleX, (fCountY + fCountHeight * 0.5f) * textScaleY),
				Colors::White, 0.f, float2_t(0.5f, 0.5f), fCountScale * textUiScale);
		}
	}
}
// Render의 HUD 텍스트 단계에서 발탄·쿠크의 사망 안내를 그린다. 유효한 DEAD snapshot과 Level이 게시한 rect가 모두 필요하다.
// 관전 중에는 제목·부활 질문만 생략하고 부활·관전 버튼 글자는 계속 그린다. 부활·관전 명령 처리는 Level이 맡는다.
void CMainApp::RenderDeadSceneText()
{
	if (ETOUI(LEVEL::VALTAN_ARENA) != CGameInstance::Get().Get_CurrentLevelID() &&
		ETOUI(LEVEL::KOUKUSAYDON_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
		return;

	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || LostArk::Shared::PLAYER_ACTION_STATE::DEAD != player.eAction)
	{
		return;
	}

	const HUD_DEADSCENE_TEXT_RECTS &rects = CCombatHUDViewModel::Get().Get_DeadSceneTextRects();
	if (!rects.isValid)
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	// 발탄·쿠크 Level의 Update_DeadScene이 자기 view의 제목·부활·관전·질문 슬롯 rect를 HUD view model에 값으로 전달한다.
	// MainApp은 Level view를 직접 소유하지 않고 전달된 기준 좌표에 viewport 배율을 적용해 글자를 맞춘다.
	const wstring strTitle = L"\xC0AC\xB9DD\xD558\xC600\xC2B5\xB2C8\xB2E4"; // 사망하였습니다 
	const float2_t vTitleMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strTitle.c_str());
	const f32_t fTitleScale = (vTitleMeasured.y > 0.f) ? (rects.fTitleHeight * 0.6f / vTitleMeasured.y) : 1.f;
	if (!rects.isSpectating)
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strTitle.c_str(),
									   float2_t((rects.fTitleX + rects.fTitleWidth * 0.5f) * textScaleX,
												(rects.fTitleY + rects.fTitleHeight * 0.5f) * textScaleY),
									   Colors::White, 0.f, float2_t(0.5f, 0.5f), fTitleScale * textUiScale);

	const wstring strReviveLabel = L"\xBD80\xD65C"; // 부활 
	const float2_t vReviveMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strReviveLabel.c_str());
	const f32_t fReviveScale = (vReviveMeasured.y > 0.f) ? (rects.fReviveTextHeight * 0.55f / vReviveMeasured.y) : 1.f;
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strReviveLabel.c_str(),
								   float2_t((rects.fReviveTextX + rects.fReviveTextWidth * 0.5f) * textScaleX,
											(rects.fReviveTextY + rects.fReviveTextHeight * 0.5f) * textScaleY),
								   Colors::White, 0.f, float2_t(0.5f, 0.5f), fReviveScale * textUiScale);

	const wstring strSpectateLabel = L"\xAD00\xC804\xD558\xAE30"; // 관전하기 
	const float2_t vSpectateMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strSpectateLabel.c_str());
	const f32_t fSpectateScale =
		(vSpectateMeasured.y > 0.f) ? (rects.fSpectateHeight * 0.5f / vSpectateMeasured.y) : 1.f;
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strSpectateLabel.c_str(),
								   float2_t((rects.fSpectateX + rects.fSpectateWidth * 0.5f) * textScaleX,
											(rects.fSpectateY + rects.fSpectateHeight * 0.5f) * textScaleY),
								   Colors::White, 0.f, float2_t(0.5f, 0.5f), fSpectateScale * textUiScale);

	const wstring strReviveMessage = L"\xBD80\xD65C\xD558\xC2DC\xACA0\xC2B5\xB2C8\xAE4C?"; // 부활하시겠습니까?
	const float2_t vMessageMeasured =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strReviveMessage.c_str());
	const f32_t fMessageScale = (vMessageMeasured.y > 0.f) ? (rects.fMessageHeight * 0.6f / vMessageMeasured.y) : 1.f;
	if (!rects.isSpectating)
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strReviveMessage.c_str(),
									   float2_t((rects.fMessageX + rects.fMessageWidth * 0.5f) * textScaleX,
												(rects.fMessageY + rects.fMessageHeight * 0.5f) * textScaleY),
									   Colors::White, 0.f, float2_t(0.5f, 0.5f), fMessageScale * textUiScale);
}

// 발탄·쿠크가 게시한 클리어 제목과 선택적 버튼 rect를 읽어 글자를 그린다.
// 클리어 판정·연출 시각·퇴장 요청은 Level이 소유하며 이 함수는 유효 flag별 텍스트만 제출한다.
void CMainApp::RenderRaidClearText()
{
	// 두 아레나는 각자의 클리어 화면에서 읽은 rect를 같은 HUD_RAIDCLEAR_TEXT_RECTS로 전달한다. 실제 슬롯 이름·연출은 각 Level이 소유한다.
	const uint32_t iRaidClearLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (ETOUI(LEVEL::VALTAN_ARENA) != iRaidClearLevel && ETOUI(LEVEL::KOUKUSAYDON_ARENA) != iRaidClearLevel)
		return;

	const HUD_RAIDCLEAR_TEXT_RECTS &rects = CCombatHUDViewModel::Get().Get_RaidClearTextRects();
	if (!rects.isValid && !rects.isButtonValid)
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	if (rects.isValid)
	{
		// 던전 클리어 제목은 현재 고정 한국어 문자열을 사용하며 이 함수에서 별도 localization key를 조회하지 않는다.
		const wstring strTitle = L"\xB358\xC804 \xD074\xB9AC\xC5B4"; // 던전 클리어 
		const float2_t vTitleMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strTitle.c_str());
		const f32_t fTitleScale = (vTitleMeasured.y > 0.f) ? (rects.fTitleHeight * 0.6f / vTitleMeasured.y) : 1.f;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strTitle.c_str(),
									   float2_t((rects.fTitleX + rects.fTitleWidth * 0.5f) * textScaleX,
												(rects.fTitleY + rects.fTitleHeight * 0.5f) * textScaleY),
									   Colors::White, 0.f, float2_t(0.5f, 0.5f), fTitleScale * textUiScale);
	}

	// "돌아가기"는 isButtonValid일 때 버튼 rect에 맞춰 그린다. 현재 발탄·쿠크 생산자는 이 flag를 켜지 않으므로 여기의 버튼 글자도 표시되지 않는다.
	if (rects.isButtonValid)
	{
		const wstring strReturnLabel = L"\xB3CC\xC544\xAC00\xAE30"; // 돌아가기 
		const float2_t vReturnMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strReturnLabel.c_str());
		const f32_t fScaleByHeight = (vReturnMeasured.y > 0.f) ? (rects.fButtonHeight * 0.6f / vReturnMeasured.y) : 1.f;
		const f32_t fScaleByWidth = (vReturnMeasured.x > 0.f) ? (rects.fButtonWidth * 0.85f / vReturnMeasured.x) : 1.f;
		const f32_t fReturnScale = (std::min)(fScaleByHeight, fScaleByWidth);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strReturnLabel.c_str(),
									   float2_t((rects.fButtonX + rects.fButtonWidth * 0.5f) * textScaleX,
												(rects.fButtonY + rects.fButtonHeight * 0.5f) * textScaleY),
									   Colors::White, 0.f, float2_t(0.5f, 0.5f), fReturnScale * textUiScale);
	}
}

// 발탄 Level이 게시한 획득 알림의 이름·접미사·품질 문자열을 표시한다.
// 유효하지 않은 rect나 빈 이름은 전체 알림 글자를 생략한다. 인벤토리 지급·등급·품질 판정은 여기서 수행하지 않는다.
void CMainApp::RenderItemAnnounceText()
{
	if (ETOUI(LEVEL::VALTAN_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
		return;

	const HUD_ITEMANNOUNCE_TEXT_RECTS &rects = CCombatHUDViewModel::Get().Get_ItemAnnounceTextRects();
	if (!rects.isValid || rects.strItemName.empty())
		return;

	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / 1280.f;
	const f32_t fScaleY = vViewport.y / 720.f;
	// 기준 글자 높이16·품질12에 1920→1280 변환2/3과 현재 확대값1.3을 곱한다.
	// 최종 글자 높이는 viewport의 세로 배율을 사용하며 가로 배율은 위치 계산에만 사용한다.
	constexpr f32_t ITEM_ANNOUNCE_TEXT_SCALE = 1.3f;
	const f32_t fTextPx = 16.f * 2.f / 3.f * ITEM_ANNOUNCE_TEXT_SCALE * fScaleY;
	const f32_t fQualityPx = 12.f * 2.f / 3.f * ITEM_ANNOUNCE_TEXT_SCALE * fScaleY;

	// 이름과 접미사의 측정 폭을 합쳐 한 줄 전체를 가운데 맞춘다. 이름은 전달된 등급 RGB, 접미사는 흰색으로 그린다.
	f32_t fNameScale = 1.f, fSuffixScale = 1.f;
	const wstring_t strNameFont = UILabelFont::Resolve(TEXT("Font_YG760"), fTextPx, fNameScale);
	const wstring_t strSuffixFont = UILabelFont::Resolve(TEXT("Font_YG760"), fTextPx, fSuffixScale);
	const f32_t fNameWidth = CGameInstance::Get().Measure_Text(strNameFont, rects.strItemName.c_str()).x * fNameScale;
	const float2_t vSuffixSize = CGameInstance::Get().Measure_Text(strSuffixFont, rects.strSuffix.c_str());
	const f32_t fLineWidth = fNameWidth + vSuffixSize.x * fSuffixScale;
	const f32_t fLineLeft = std::round((rects.fTextX + rects.fTextWidth * 0.5f) * fScaleX - fLineWidth * 0.5f);
	const f32_t fLineTop =
		std::round((rects.fTextY + rects.fTextHeight * 0.5f) * fScaleY - vSuffixSize.y * fSuffixScale * 0.5f);
	// 입력 정수의 RGB 각 byte를0~1로 바꾸고 전달받은 alpha를 붙인다. alpha의 범위를 이 함수에서 다시 제한하지는 않는다.
	const auto Rgb = [](const std::uint32_t iRgb, const f32_t fAlpha)
	{
		return XMVectorSet(((iRgb >> 16) & 0xffu) / 255.f, ((iRgb >> 8) & 0xffu) / 255.f, (iRgb & 0xffu) / 255.f,
						   fAlpha);
	};
	const fvector_t vNameColor = Rgb(rects.iNameRgb, rects.fTextAlpha);
	CGameInstance::Get().Draw_Text(strNameFont, rects.strItemName.c_str(), float2_t(fLineLeft, fLineTop), vNameColor,
								   0.f, float2_t(0.f, 0.f), fNameScale);
	CGameInstance::Get().Draw_Text(strSuffixFont, rects.strSuffix.c_str(),
								   float2_t(std::round(fLineLeft + fNameWidth), fLineTop),
								   XMVectorSet(1.f, 1.f, 1.f, rects.fTextAlpha), 0.f, float2_t(0.f, 0.f), fSuffixScale);

	// Level이 전달한 품질 제목은 중앙, 값은 오른쪽에 맞춘다. 등급별 색·문자열·alpha도 Level의 표시값을 사용한다.
	UILabelFont::Draw_Centered(TEXT("Font_YG760"), L"\xD488\xC9C8",
							   (rects.fQualityLabelX + rects.fQualityLabelWidth * 0.5f) * fScaleX,
							   (rects.fQualityLabelY + rects.fQualityLabelHeight * 0.5f) * fScaleY, fQualityPx,
							   XMVectorSet(1.f, 1.f, 1.f, rects.fQualityAlpha));
	f32_t fValueScale = 1.f;
	const wstring_t strValueFont = UILabelFont::Resolve(TEXT("Font_YG760"), fQualityPx, fValueScale);
	const float2_t vValueSize = CGameInstance::Get().Measure_Text(strValueFont, rects.strQualityValue.c_str());
	CGameInstance::Get().Draw_Text(
		strValueFont, rects.strQualityValue.c_str(),
		float2_t(std::round((rects.fQualityValueX + rects.fQualityValueWidth) * fScaleX - vValueSize.x * fValueScale),
				 std::round((rects.fQualityValueY + rects.fQualityValueHeight * 0.5f) * fScaleY -
							vValueSize.y * fValueScale * 0.5f)),
		Rgb(rects.iQualityRgb, rects.fQualityAlpha), 0.f, float2_t(0.f, 0.f), fValueScale);
}
// Update_CombatHUD가 복제된 현재 스킬·단계·시작 tick과 로컬 스킬 정의를 조합해 HOLD 게이지를 갱신한다.
// 표시용 경과 시간·취소 추정만 MainApp에 보관하며 Server 스킬 진행이나 입력 해제 상태를 변경하지 않는다.
void CMainApp::Update_ChargeGauge()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.ChargeGauge.Update");
	if (nullptr == m_pHUDRuntimeView)
		return;

	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();

	// HOLD 게이지는 Server snapshot의 단계·tick을 표시한다. 17240은 단계2에서 300ms씩 세 번, 분기형 HOLD는 단계2 비율로 채운다.
	// 일반 HOLD는 관측한 단계 시간을 누적해 최초 타격까지 채우며 단계 누락·50ms 초과 단축은 표시 취소로 추정한다. 실제 키 해제 원인은 받지 않는다.
	constexpr LostArk::Shared::SKILL_ID FULL_BARREL_CANNON_SKILL_ID = 17240;
	constexpr std::uint8_t FULL_BARREL_CANNON_PUMP_STAGE = 2u;
	constexpr f32_t FULL_BARREL_CANNON_PUMP_MS = 300.f;
	constexpr std::uint32_t FULL_BARREL_CANNON_PUMP_COUNT = 3u;
	constexpr f32_t SERVER_TICK_HZ = 30.f;
	// 세 단계이고 첫 단계의 comboAdvanceMs가 actionDurationMs보다 짧으면 분기형 HOLD 표시로 처리한다.
	// 단계2에서 경과 시간/해당 단계 지속 시간으로 채우며 일반 HOLD의 조기 종료 추정은 적용하지 않는다.
	constexpr std::uint8_t BRANCHING_HOLD_LOOP_STAGE = 2u;

	bool_t bCharging = false;
	f32_t fChargeProgress = 0.f;
	const bool_t bIsHoldAction =
		player.isValid && LostArk::Shared::PLAYER_ACTION_STATE::SKILL == player.eAction && 0u != player.iComboStage;
	const PLAYER_SKILL_DEFINITION *pSkill =
		bIsHoldAction ? CPlayerSkillCatalog::Find_ById(player.iCurrentSkillId) : nullptr;
	const bool_t bValidHoldStage = nullptr != pSkill &&
								   LostArk::Shared::PLAYER_SKILL_KIND::HOLD == pSkill->eSkillKind &&
								   player.iComboStage <= pSkill->ComboStages.size();

	if (!bValidHoldStage)
	{
		// 유효한 HOLD 단계가 아니면 추적 skill ID와 취소 flag를 초기화한다. 다음 일반 HOLD 시작에서 나머지 추적 상태도 새로 설정한다.
		m_iChargeGaugeTrackedSkillId = LostArk::Shared::INVALID_SKILL_ID;
		m_bChargeGaugeCancelled = false;
	}
	else if (FULL_BARREL_CANNON_SKILL_ID == pSkill->iSkillId)
	{
		if (FULL_BARREL_CANNON_PUMP_STAGE == player.iComboStage)
		{
			f32_t fStageAgeSeconds = 0.f;
			CActionPresentationTimeline::Try_ResolveActionAgeSeconds(player.iServerTick, player.iActionStartTick,
																	 SERVER_TICK_HZ, fStageAgeSeconds);
			const f32_t fStageAgeMs = fStageAgeSeconds * 1000.f;
			const f32_t fPumpTotalMs = FULL_BARREL_CANNON_PUMP_MS * FULL_BARREL_CANNON_PUMP_COUNT;
			const f32_t fClampedAgeMs = std::clamp(fStageAgeMs, 0.f, fPumpTotalMs);
			// 현재 단계 나이를0~900ms로 제한하고300ms 나머지로 한 번의 pump 진행률을 만든다. 정확히300/600/900ms에서는 나머지가0이 된다.
			const f32_t fWithinPumpMs = std::fmod(fClampedAgeMs, FULL_BARREL_CANNON_PUMP_MS);
			fChargeProgress = std::clamp(fWithinPumpMs / FULL_BARREL_CANNON_PUMP_MS, 0.f, 1.f);
			bCharging = true;
		}
		// 17240은 단계2에서만 게이지를 켠다. 단계1과3에서는 기본 false가 유지된다.
	}
	else if (3u == pSkill->ComboStages.size() &&
			 pSkill->ComboStages[0].iComboAdvanceMs < pSkill->ComboStages[0].iActionDurationMs)
	{
		if (BRANCHING_HOLD_LOOP_STAGE == player.iComboStage)
		{
			f32_t fStageAgeSeconds = 0.f;
			CActionPresentationTimeline::Try_ResolveActionAgeSeconds(player.iServerTick, player.iActionStartTick,
																	 SERVER_TICK_HZ, fStageAgeSeconds);
			const f32_t fLoopMs =
				static_cast<f32_t>(pSkill->ComboStages[BRANCHING_HOLD_LOOP_STAGE - 1u].iActionDurationMs);
			if (fLoopMs > 0.f)
			{
				fChargeProgress = std::clamp(fStageAgeSeconds * 1000.f / fLoopMs, 0.f, 1.f);
				bCharging = true;
			}
		}
	}
	else
	{
		// 추적이 비었거나 skill ID가 바뀌거나 단계 번호가 감소했으면 새 일반 HOLD로 본다. action 시작 tick만 바뀐 같은 단계는 이 조건에 포함하지 않는다.
		const bool_t bFreshCharge = LostArk::Shared::INVALID_SKILL_ID == m_iChargeGaugeTrackedSkillId ||
									m_iChargeGaugeTrackedSkillId != pSkill->iSkillId ||
									player.iComboStage < m_iChargeGaugeTrackedComboStage;
		if (bFreshCharge)
		{
			m_iChargeGaugeTrackedSkillId = pSkill->iSkillId;
			m_iChargeGaugeTrackedComboStage = player.iComboStage;
			m_iChargeGaugeStageStartTick = player.iActionStartTick;
			m_fChargeGaugeElapsedBeforeCurrentStageMs = 0.f;
			m_bChargeGaugeCancelled = false;
		}
		else if (player.iComboStage != m_iChargeGaugeTrackedComboStage)
		{
			// 관측된 이전·새 단계의 시작 tick 차이를 초로 구한 뒤 ms로 변환한다. timeline 함수 실패는 출력0이며 이 호출부는 bool 반환을 따로 검사하지 않는다.
			f32_t fEndedStageSeconds = 0.f;
			CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
				player.iActionStartTick, m_iChargeGaugeStageStartTick, SERVER_TICK_HZ, fEndedStageSeconds);
			const f32_t fEndedStageRealMs = fEndedStageSeconds * 1000.f;
			const std::size_t iEndedStageIndex = static_cast<std::size_t>(m_iChargeGaugeTrackedComboStage) - 1u;
			const f32_t fEndedStageAuthoredMs =
				static_cast<f32_t>(pSkill->ComboStages[iEndedStageIndex].iActionDurationMs);
			constexpr f32_t TRUNCATION_TOLERANCE_MS = 50.f;
			const bool_t bStageWasSkipped = player.iComboStage > m_iChargeGaugeTrackedComboStage + 1u;
			const bool_t bStageWasCutShort = fEndedStageRealMs < fEndedStageAuthoredMs - TRUNCATION_TOLERANCE_MS;
			if (bStageWasSkipped || bStageWasCutShort)
				m_bChargeGaugeCancelled = true;

			m_fChargeGaugeElapsedBeforeCurrentStageMs += fEndedStageRealMs;
			m_iChargeGaugeTrackedComboStage = player.iComboStage;
			m_iChargeGaugeStageStartTick = player.iActionStartTick;
		}

		if (!m_bChargeGaugeCancelled)
		{
			// 첫 양수 hitTimeMs가 있는 단계까지의 저작 시간을 목표로 계산한다. 그런 단계가 없으면 마지막 단계의 전체 지속 시간까지를 목표로 삼는다.
			std::size_t iHitStageIndex = pSkill->ComboStages.size() - 1;
			for (std::size_t i = 0; i < pSkill->ComboStages.size(); ++i)
			{
				if (0u != pSkill->ComboStages[i].iHitTimeMs)
				{
					iHitStageIndex = i;
					break;
				}
			}
			f32_t fTargetMs = 0.f;
			for (std::size_t i = 0; i < iHitStageIndex; ++i)
				fTargetMs += static_cast<f32_t>(pSkill->ComboStages[i].iActionDurationMs);
			fTargetMs += (0u != pSkill->ComboStages[iHitStageIndex].iHitTimeMs)
							 ? static_cast<f32_t>(pSkill->ComboStages[iHitStageIndex].iHitTimeMs)
							 : static_cast<f32_t>(pSkill->ComboStages[iHitStageIndex].iActionDurationMs);

			f32_t fStageAgeSeconds = 0.f;
			CActionPresentationTimeline::Try_ResolveActionAgeSeconds(player.iServerTick, player.iActionStartTick,
																	 SERVER_TICK_HZ, fStageAgeSeconds);
			const f32_t fStageAgeMs = fStageAgeSeconds * 1000.f;

			if (fTargetMs > 0.f)
			{
				fChargeProgress =
					std::clamp((m_fChargeGaugeElapsedBeforeCurrentStageMs + fStageAgeMs) / fTargetMs, 0.f, 1.f);
				bCharging = true;
			}
		}
	}

	// 배경과 track은 충전 상태에 맞춰 켜고 채움 슬롯은 진행률이 양수일 때만 켠다. 실제 채움 비율만 바꾸며 슬롯의 저작 rect는 유지한다.
	m_pHUDRuntimeView->Set_SlotVisible("ChargeGauge_Bg", bCharging);
	m_pHUDRuntimeView->Set_SlotVisible("ChargeGauge_Track", bCharging);
	const bool_t bShowFill = bCharging && fChargeProgress > 0.f;
	m_pHUDRuntimeView->Set_SlotVisible("ChargeGauge_Fill", bShowFill);
	if (bShowFill)
		m_pHUDRuntimeView->Set_SlotFillRatio("ChargeGauge_Fill", fChargeProgress);
}

// Render의 HUD 텍스트 단계에서 현재 HOLD 스킬 이름을 track rect 중앙에 그린다.
// 지원하는 단계·취소 flag·이름 변환을 확인하며 글자 크기는 상자 높이70%와 폭90%에 맞는 배율 중 작은 값이다.
void CMainApp::RenderChargeGaugeText()
{
	if (nullptr == m_pHUDRuntimeView)
		return;

	const HUD_PLAYER_STATE &player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || LostArk::Shared::PLAYER_ACTION_STATE::SKILL != player.eAction || 0u == player.iComboStage)
	{
		return;
	}

	const PLAYER_SKILL_DEFINITION *pSkill = CPlayerSkillCatalog::Find_ById(player.iCurrentSkillId);
	if (nullptr == pSkill || LostArk::Shared::PLAYER_SKILL_KIND::HOLD != pSkill->eSkillKind ||
		player.iComboStage > pSkill->ComboStages.size())
	{
		return;
	}
	// HOLD 이름도 17240·분기형은 단계2, 일반형은 Update가 남긴 취소 flag를 따른다. 목표 시간·loop 길이 검사는 반복하지 않아 게이지와 판정이 완전히 같지는 않다.
	constexpr LostArk::Shared::SKILL_ID FULL_BARREL_CANNON_SKILL_ID = 17240;
	if (FULL_BARREL_CANNON_SKILL_ID == pSkill->iSkillId)
	{
		if (2u != player.iComboStage)
			return;
	}
	else if (3u == pSkill->ComboStages.size() &&
			 pSkill->ComboStages[0].iComboAdvanceMs < pSkill->ComboStages[0].iActionDurationMs)
	{
		// 분기형 HOLD의 이름은 단계2에서만 표시한다. loop 지속 시간의 양수 여부는 이 텍스트 경로가 검사하지 않는다.
		if (2u != player.iComboStage)
			return;
	}
	else if (m_bChargeGaugeCancelled)
	{
		return;
	}

	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pHUDRuntimeView->Get_SlotRect("ChargeGauge_Track", fX, fY, fWidth, fHeight))
		return;

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	// 카탈로그의 UTF-8 이름을 Windows wide 문자열로 변환한다. 필요한 길이 조회와 실제 변환 결과를 모두 확인하며 실패하면 이 글자만 생략한다.
	wstring strLabel;
	const int iRequiredLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pSkill->strDisplayName.data(),
													static_cast<int>(pSkill->strDisplayName.size()), nullptr, 0);
	if (iRequiredLength <= 0)
		return;
	strLabel.resize(static_cast<size_t>(iRequiredLength));
	if (iRequiredLength != MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pSkill->strDisplayName.data(),
											   static_cast<int>(pSkill->strDisplayName.size()), strLabel.data(),
											   iRequiredLength))
	{
		return;
	}

	const float2_t vMeasured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), strLabel.c_str());
	const f32_t fScaleByHeight = (vMeasured.y > 0.f) ? (fHeight * 0.7f / vMeasured.y) : 1.f;
	const f32_t fScaleByWidth = (vMeasured.x > 0.f) ? (fWidth * 0.9f / vMeasured.x) : 1.f;
	const f32_t fScale = (std::min)(fScaleByHeight, fScaleByWidth);
	CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strLabel.c_str(),
								   float2_t((fX + fWidth * 0.5f) * textScaleX, (fY + fHeight * 0.5f) * textScaleY),
								   Colors::White, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
}
void CMainApp::Hide_EstherUI()
{
	if (nullptr == m_pEstherUIView)
		return;
	constexpr const char_t* ESTHER_ALL_SLOTS[] = {
		"Esther_HeaderFrame",
		"Esther_Slot1_Frame", "Esther_Slot1_KeyBg", "Esther_Slot1_Icon", "Esther_Slot1_Ready",
		"Esther_Slot2_Frame", "Esther_Slot2_KeyBg", "Esther_Slot2_Icon", "Esther_Slot2_Ready",
		"Esther_Slot3_Frame", "Esther_Slot3_KeyBg", "Esther_Slot3_Icon", "Esther_Slot3_Ready",
		"Esther_GaugeTrack", "Esther_GaugeFill",
	};
	for (const char_t* pSlotId : ESTHER_ALL_SLOTS)
		m_pEstherUIView->Set_SlotVisible(pSlotId, false);
}

void CMainApp::Update_EstherGauge(const f32_t fTimeDelta)
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.EstherGauge.Update");
	if (nullptr == m_pEstherUIView)
		return;

	/* 에스더 창은 발탄·쿠크 레이드에서만 표시한다. Character Select에서도 게이지 최대값이
	전달될 수 있으므로 최대값만으로 레이드 UI를 열지 않고 현재 Level도 확인한다. */
	const uint32_t maximum =
		CCombatHUDViewModel::Get().Get_EstherGaugeMaximum();
	const bool_t skillWindowOpen =
		nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open();
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isRaidArena =
		ETOUI(LEVEL::VALTAN_ARENA) == currentLevel ||
		ETOUI(LEVEL::KOUKUSAYDON_ARENA) == currentLevel;
	if (!isRaidArena || 0u == maximum || skillWindowOpen || Is_KoukuMinigameHUDHidden() ||
		!CCombatHUDViewModel::Get().Get_Player().isValid)
	{
		Hide_EstherUI();
		return;
	}

	/* 프레임·초상화·트랙은 창이 표시되는 동안 함께 보인다. Esther_GaugeFill은 별도 rect를
	가진 슬롯이므로 위치 편집과 실제 게이지 비율 적용을 분리한다. 아래에서 비율을 0~1로
	제한하고, 세 Ready 슬롯은 게이지가 최대값에 도달했을 때만 켠다. */
	constexpr const char_t* ESTHER_STATIC_SLOTS[] = {
		"Esther_HeaderFrame",
		"Esther_Slot1_Frame", "Esther_Slot1_KeyBg", "Esther_Slot1_Icon",
		"Esther_Slot2_Frame", "Esther_Slot2_KeyBg", "Esther_Slot2_Icon",
		"Esther_Slot3_Frame", "Esther_Slot3_KeyBg", "Esther_Slot3_Icon",
		"Esther_GaugeTrack",
	};
	for (const char_t* pSlotId : ESTHER_STATIC_SLOTS)
		m_pEstherUIView->Set_SlotVisible(pSlotId, true);

	/* 같은 UI view가 Level 전환 뒤에도 남을 수 있으므로 현재 레이드의 세 초상화를 모두
	지정한다. 쿠크와 발탄의 슬롯별 이미지 경로를 여기서 선택해 이전 레이드의 그림이 남지 않게 한다. */
	if (ETOUI(LEVEL::KOUKUSAYDON_ARENA) == currentLevel)
	{
		m_pEstherUIView->Set_SlotTexture("Esther_Slot1_Icon", "UI/Esther/esther_icon_3.png");
		m_pEstherUIView->Set_SlotTexture("Esther_Slot2_Icon", "UI/Esther/esther_portrait_wei.png");
		m_pEstherUIView->Set_SlotTexture("Esther_Slot3_Icon", "UI/Esther/esther_icon_4.png");
	}
	else
	{
		m_pEstherUIView->Set_SlotTexture("Esther_Slot1_Icon", "UI/Esther/esther_portrait_sillian.png");
		m_pEstherUIView->Set_SlotTexture("Esther_Slot2_Icon", "UI/Esther/esther_portrait_wei.png");
		m_pEstherUIView->Set_SlotTexture("Esther_Slot3_Icon", "UI/Esther/esther_portrait_bahuntur.png");
	}

	const uint32_t gauge = CCombatHUDViewModel::Get().Get_EstherGauge();
	const float fillRatio = (std::clamp)(
		static_cast<float>(gauge) / static_cast<float>(maximum), 0.f, 1.f);
	m_pEstherUIView->Set_SlotFillRatio("Esther_GaugeFill", fillRatio);
	m_pEstherUIView->Set_SlotVisible("Esther_GaugeFill", fillRatio > 0.f);

	const bool_t bReady = gauge >= maximum;
	m_pEstherUIView->Set_SlotVisible("Esther_Slot1_Ready", bReady);
	m_pEstherUIView->Set_SlotVisible("Esther_Slot2_Ready", bReady);
	m_pEstherUIView->Set_SlotVisible("Esther_Slot3_Ready", bReady);
	/* Ready 효과를 포함한 슬롯 애니메이션은 view의 Update에 전달한 시간으로 진행한다. */
	m_pEstherUIView->Update(fTimeDelta);
	CUITextOcclusion::Get().Add_SlotOccluder(UI_TEXT_LAYER::HUD, *m_pEstherUIView, "Esther_HeaderFrame");
}

void CMainApp::RenderEstherGaugeText()
{
	/* 이 문자는 에스더 창의 슬롯 rect를 사용하므로 현재 레이드·최대 게이지·스킬 창·플레이어
	유효성을 먼저 확인한다. Update_EstherGauge의 추가 미니게임 숨김 조건까지 이 함수가 재검사하는 것은 아니다. */
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isRaidArena =
		ETOUI(LEVEL::VALTAN_ARENA) == currentLevel ||
		ETOUI(LEVEL::KOUKUSAYDON_ARENA) == currentLevel;
	if (nullptr == m_pEstherUIView || !isRaidArena)
		return;
	const uint32_t maximum =
		CCombatHUDViewModel::Get().Get_EstherGaugeMaximum();
	if (0u == maximum ||
		(nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open()) ||
		!CCombatHUDViewModel::Get().Get_Player().isValid)
	{
		return;
	}

	const float2_t vTextViewportSize = CGameInstance::Get().Get_ViewportSize();
	const float textScaleX = vTextViewportSize.x / 1280.f;
	const float textScaleY = vTextViewportSize.y / 720.f;
	const float textUiScale = (std::min)(textScaleX, textScaleY);

	/* 초상화 아래 키판에 Ctrl+Z/X/C의 축약 표기를 중앙 정렬한다. 최대 게이지 전에는 흰색,
	도달 후에는 금색(#FFD200)을 쓰며, 글자 높이는 1080p 기준 12px를 720p 기준으로 환산한다. */
	const uint32_t gauge = CCombatHUDViewModel::Get().Get_EstherGauge();
	const vector_t vColor = gauge >= maximum ?
		XMVectorSet(1.f, 210.f / 255.f, 0.f, 1.f) : Colors::White;
	struct ESTHER_KEY_LABEL { const char_t* pPlateSlot; const wchar_t* pLabel; };
	constexpr ESTHER_KEY_LABEL KEY_LABELS[] = {
		{ "Esther_Slot1_KeyBg", L"C+Z" },
		{ "Esther_Slot2_KeyBg", L"C+X" },
		{ "Esther_Slot3_KeyBg", L"C+C" },
	};
	constexpr f32_t LABEL_HEIGHT = 12.f * (2.f / 3.f);
	for (const ESTHER_KEY_LABEL& Label : KEY_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!m_pEstherUIView->Get_SlotRect(Label.pPlateSlot, fX, fY, fWidth, fHeight))
			continue;
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), Label.pLabel);
		const f32_t fScale = (vMeasured.y > 0.f) ? (LABEL_HEIGHT / vMeasured.y) : 1.f;
		/* 원본 키판의 y+5, 높이 22px 배치를 기준으로 중심을 위쪽에서 16px 아래에 둔다. */
		const f32_t fCenterX = fX + fWidth * 0.5f;
		const f32_t fCenterY = fY + 16.f * (2.f / 3.f);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), Label.pLabel,
			float2_t(fCenterX * textScaleX + 1.f, fCenterY * textScaleY + 1.f),
			XMVectorSet(0.f, 0.f, 0.f, 220.f / 255.f), 0.f, float2_t(0.5f, 0.5f),
			fScale * textUiScale);
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), Label.pLabel,
			float2_t(fCenterX * textScaleX, fCenterY * textScaleY),
			vColor, 0.f, float2_t(0.5f, 0.5f), fScale * textUiScale);
	}
}

namespace
{
	struct SKILL_ICON_ENTRY { LostArk::Shared::SKILL_ID iSkillId; const char* pIconPath; };
	constexpr SKILL_ICON_ENTRY SKILL_ICON_TABLE[] =
	{
		/* 창술사 장창 스킬 아이콘 */
		{ 34040, "UI/Skill/LanceMaster/34040_DoubleStrike.png" },
		{ 34090, "UI/Skill/LanceMaster/34090_ThornJab.png" },
		{ 34100, "UI/Skill/LanceMaster/34100_BlueDragonsClaw.png" },
		{ 34160, "UI/Skill/LanceMaster/34160_SpearDive.png" },
		{ 34140, "UI/Skill/LanceMaster/34140_SoulCutter.png" },
		{ 34120, "UI/Skill/LanceMaster/34120_ChainSlash.png" },
		{ 34110, "UI/Skill/LanceMaster/34110_HalfMoonSlash.png" },
		{ 34150, "UI/Skill/LanceMaster/34150_RagingDragonSlash.png" },
		/* 창술사 T 스킬 아이콘 */
		{ 34650, "UI/Skill/LanceMaster/34650_DeadlyRedDragon.png" },
		/* 창술사 단창 스킬 아이콘: 이 자세에는 D/F 슬롯이 없다. */
		{ 34540, "UI/Skill/LanceMaster/34540_SpiralingSpear.png" },
		{ 34550, "UI/Skill/LanceMaster/34550_4HeadedDragon.png" },
		{ 34560, "UI/Skill/LanceMaster/34560_ThrustOfDestruction.png" },
		{ 34570, "UI/Skill/LanceMaster/34570_StarfallPounce.png" },
		{ 34580, "UI/Skill/LanceMaster/34580_DragonscaleDefense.png" },
		{ 34590, "UI/Skill/LanceMaster/34590_RedDragonsHorn.png" },
		/* 창술사 V 각성기 아이콘 */
		{ 34610, "UI/Skill/LanceMaster/34610_StormingRedDragon.png" },
		/* 워로드 기본 스킬 아이콘 */
		{ 17030, "UI/Skill/Warlord/17030_SharpSpear.png" },
		{ 17060, "UI/Skill/Warlord/17060_FireBullet.png" },
		{ 17080, "UI/Skill/Warlord/17080_DashUpperFire.png" },
		{ 17110, "UI/Skill/Warlord/17110_LeapAttack.png" },
		{ 17090, "UI/Skill/Warlord/17090_HookChain.png" },
		{ 17040, "UI/Skill/Warlord/17040_Bash.png" },
		{ 17100, "UI/Skill/Warlord/17100_ShieldShock.png" },
		{ 17140, "UI/Skill/Warlord/17140_GuardiansLightning.png" },
		/* 워로드 T 스킬 아이콘 */
		{ 17240, "UI/Skill/Warlord/17240_FullBarrelCannon.png" },
		/* 워로드 V 각성기 아이콘 */
		{ 17170, "UI/Skill/Warlord/17170_GuardiansProtection.png" },
		/* 도화가 기본 스킬 아이콘 */
		{ 31200, "UI/Skill/Artist/31200_InkShower.png" },
		{ 31430, "UI/Skill/Artist/31430_Scatter.png" },
		{ 31480, "UI/Skill/Artist/31480_CraneWings.png" },
		{ 31210, "UI/Skill/Artist/31210_Kongkongi.png" },
		{ 31460, "UI/Skill/Artist/31460_ButterflyDream.png" },
		{ 31420, "UI/Skill/Artist/31420_OrchidStrike.png" },
		{ 31490, "UI/Skill/Artist/31490_TigerSlash.png" },
		{ 31470, "UI/Skill/Artist/31470_OneStroke.png" },
		/* 도화가 T 스킬 아이콘 */
		{ 31950, "UI/Skill/Artist/31950_DragonEngraving.png" },
		/* 도화가 V 각성기 아이콘 */
		{ 31910, "UI/Skill/Artist/31910_DreamPeachGarden.png" },
		/* 차원술사 기본 스킬 아이콘 */
		{ 2050100, "UI/Skill/DimensionMaster/2050100_OneNeedle.png" },
		{ 2050120, "UI/Skill/DimensionMaster/2050120_Fragment.png" },
		{ 2050160, "UI/Skill/DimensionMaster/2050160_CrossThrust.png" },
		{ 2050180, "UI/Skill/DimensionMaster/2050180_BeyondSlash.png" },
		{ 2050210, "UI/Skill/DimensionMaster/2050210_LightSplit.png" },
		{ 2050220, "UI/Skill/DimensionMaster/2050220_PointPierce.png" },
		{ 2050240, "UI/Skill/DimensionMaster/2050240_BoundaryBreak.png" },
		{ 2050230, "UI/Skill/DimensionMaster/2050230_TimeShatter.png" },
		/* 차원술사 T 스킬 아이콘 */
		{ 2050500, "UI/Skill/DimensionMaster/2050500_KarmaBoundary.png" },
		/* 차원술사 V 각성기 아이콘 */
		{ 2050520, "UI/Skill/DimensionMaster/2050520_TimeShackles.png" },
		/* 가디언나이트 인간 자세 스킬 아이콘 */
		{ 49100, "UI/Skill/GuardianKnight/49100_Cleave.png" },
		{ 49110, "UI/Skill/GuardianKnight/49110_WildStrike.png" },
		{ 49120, "UI/Skill/GuardianKnight/49120_Ignite.png" },
		{ 49130, "UI/Skill/GuardianKnight/49130_ValiantCharge.png" },
		{ 49150, "UI/Skill/GuardianKnight/49150_QuakeSmash.png" },
		{ 49200, "UI/Skill/GuardianKnight/49200_DragonsBlow.png" },
		{ 49220, "UI/Skill/GuardianKnight/49220_BurningFlame.png" },
		{ 49260, "UI/Skill/GuardianKnight/49260_InfernoStrike.png" },
		/* 가디언나이트 용 자세 스킬 아이콘 */
		{ 49210, "UI/Skill/GuardianKnight/49210_DragonsSpear.png" },
		{ 49230, "UI/Skill/GuardianKnight/49230_AbaddonFlame.png" },
		{ 49270, "UI/Skill/GuardianKnight/49270_InfernoFlame.png" },
		{ 49330, "UI/Skill/GuardianKnight/49330_FireBreath.png" },
		/* 가디언나이트 자세 공용 스킬과 양방향 Z 전환 아이콘 */
		{ 49400, "UI/Skill/GuardianKnight/49400_DragonRampage.png" },
		{ 49420, "UI/Skill/GuardianKnight/49420_BreathOfDestruction.png" },
		{ 49040, "UI/Skill/GuardianKnight/49040_DragonAvatar.png" },
		{ 49041, "UI/Skill/GuardianKnight/49041_DragonAvatarRelease.png" },
		/* 특수 슬롯에 쓰는 이동(Space) 아이콘이며 build_quickslot_hud_ui.py의 출력 경로를 사용한다. */
		{ 34020, "UI/Skill/LanceMaster/34020_Space.png" },
		{ 34520, "UI/Skill/LanceMaster/34520_Space.png" },
		{ 17020, "UI/Skill/Warlord/17020_Space.png" },
		{ 31020, "UI/Skill/Artist/31020_Space.png" },
		{ 2050020, "UI/Skill/DimensionMaster/2050020_Space.png" },
		{ 49020, "UI/Skill/GuardianKnight/49020_Lunge.png" },
		{ 49021, "UI/Skill/GuardianKnight/49021_Glide.png" },
	};

	const char* Find_HudSkillIcon(const LostArk::Shared::SKILL_ID iSkillId)
	{
		for (const SKILL_ICON_ENTRY& Entry : SKILL_ICON_TABLE)
		{
			if (Entry.iSkillId == iSkillId)
				return Entry.pIconPath;
		}
		return nullptr;
	}
}

void CMainApp::Update_SkillIcons()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.SkillIcons.Update");
	/* Skill_Q부터 Skill_F까지의 아이콘은 현재 직업·자세의 스킬 내용이 결정한다. 입력 처리와
	같은 CPlayerSkillCatalog::Find_BySlot을 사용해 PlayerSkills.json과 HUD에 서로 다른 슬롯
	대응표를 두지 않는다. HUD_Layout.json은 공용 슬롯의 위치·크기를 맡고, 이 함수는
	Update_CombatHUD의 표시 경로에서 현재 슬롯에 맞는 텍스처를 선택한다. */
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();

	constexpr const char* INPUT_SLOTS[] = { "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V" };

	for (const char* pInputSlot : INPUT_SLOTS)
	{
		/* 기본 슬롯은 배경을, Skill_<X>_Icon은 현재 스킬 이미지를, Skill_<X>_Frame은 테두리를
		그린다. 아이콘을 바꿔도 배경과 테두리의 별도 슬롯 배치·표시 순서를 유지한다. */
		const char* pIconPath = nullptr;
		bool_t bUsableInStance = true;
		if (const PLAYER_SKILL_DEFINITION* pSkill = CPlayerSkillCatalog::Find_BySlot(
			player.eCharacterClass, pInputSlot, player.eStance))
		{
			pIconPath = Find_HudSkillIcon(pSkill->iSkillId);
		}
		else if (LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT == player.eCharacterClass &&
			0 == std::strcmp(pInputSlot, "T"))
		{
			/* 가디언나이트 T는 용 자세에서 사용하는 스킬이다. 인간 자세의 조회가 비어 있을 때에도
			용 자세의 아이콘을 찾아 어둡게 표시한다. 이 보완은 해당 직업·슬롯에만 적용한다. */
			for (const PLAYER_SKILL_DEFINITION& Skill : CPlayerSkillCatalog::Get_Skills())
			{
				if (Skill.eCharacterClass != player.eCharacterClass ||
					Skill.strInputSlot != pInputSlot ||
					LostArk::Shared::PLAYER_STANCE_ID::NONE == Skill.eRequiredStance)
				{
					continue;
				}
				pIconPath = Find_HudSkillIcon(Skill.iSkillId);
				if (nullptr != pIconPath)
				{
					bUsableInStance = false;
					break;
				}
			}
		}

		const string strIconSlot = string("Skill_") + pInputSlot + "_Icon";
		if (nullptr != pIconPath)
		{
			m_pHUDRuntimeView->Set_SlotTexture(strIconSlot, pIconPath);
			/* 비활성 아이콘을 별도 파일로 바꾸지 않고 같은 텍스처의 RGB에 0.3을 곱한다.
			알파는 1로 유지하므로 투명도를 낮추는 것과 구분된다. */
			m_pHUDRuntimeView->Set_SlotTint(strIconSlot, bUsableInStance ?
				float4_t(1.f, 1.f, 1.f, 1.f) : float4_t(0.3f, 0.3f, 0.3f, 1.f));
			m_pHUDRuntimeView->Set_SlotVisible(strIconSlot, true);
		}
		else
		{
			m_pHUDRuntimeView->Set_SlotVisible(strIconSlot, false);
		}
	}
}

void CMainApp::Update_QuickSlotFlash()
{
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "UI.Runtime.QuickSlotFlash.Update");
	/* Update_CombatHUD의 표시 경로에서 퀵슬롯 사용 효과만 갱신한다. 아이콘과 쿨다운은
	함께 호출되는 Update_SkillIcons/Update_SkillCooldowns가 맡으므로 여기서 다시 갱신하지 않는다. */
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();

	constexpr const char* INPUT_SLOTS[] = { "Q", "W", "E", "R", "A", "S", "D", "F", "T", "V" };

	uint32_t iSlotIndex = 0;
	for (const char* pInputSlot : INPUT_SLOTS)
	{
		bool_t bIsReady = true;
		for (const HUD_SKILL_STATE& Skill : player.Skills)
		{
			if (Skill.strInputSlot == pInputSlot)
			{
				bIsReady = Skill.Is_Ready(player.iServerTick);
				break;
			}
		}

		/* 이전 프레임에는 준비됐고 지금은 준비되지 않은 전이를 사용 직후 신호로 삼는다.
		원시 종료 tick 대신 플레이어의 현재 Server tick으로 계산한 Is_Ready 결과를 비교한다. */
		if (m_bPreviousQuickSlotReady[iSlotIndex] && !bIsReady)
		{
			m_pHUDRuntimeView->Play_KeyframeAnimation(
				string("Skill_") + pInputSlot + "_Flash", "flash");
		}
		m_bPreviousQuickSlotReady[iSlotIndex] = bIsReady;
		++iSlotIndex;
	}
}

void CMainApp::RenderCombatHUDText()
{
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::COLOSSEUM) &&
		currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::KOUKUSAYDON_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) &&
		currentLevel != ETOUI(LEVEL::KOUKUSAYDON_ARENA) &&
		!Is_WaterGunHudLevel(currentLevel, CCombatHUDViewModel::Get().Get_Player()))
	{
		return;
	}
	if (nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open())
		return;
	const float2_t viewportSize = CGameInstance::Get().Get_ViewportSize();
	const float scaleX = viewportSize.x / 1280.f;
	const float scaleY = viewportSize.y / 720.f;
	const float textScale = (std::min)(scaleX, scaleY);
	const auto position = [scaleX, scaleY](const float x, const float y)
	{
		return float2_t(x * scaleX, y * scaleY);
	};
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	if (player.isValid && player.iMaximumHp > 0u && player.iMaximumResource > 0u && !m_bShipHudActive)
	{
		/* 보호막이 있을 때만 HP 숫자 뒤에 (+n)을 붙여 별도 보호막 수치를 구분한다. */
		const wstring hp = std::to_wstring(player.iCurrentHp) +
			L" / " + std::to_wstring(player.iMaximumHp) +
			(0u == player.iShield ? wstring() :
				L"(+" + std::to_wstring(player.iShield) + L")");
		/* ember 자원을 쓰는 직업은 Update_GuardianKnightIdentity의 구슬·소켓으로 표시하므로
		이 공용 자원 숫자는 그리지 않아 중복 표시를 피한다. */
		const bool_t bHasEmberPool = 0u != player.iEmberMaximumSockets;
		const wstring mana = std::to_wstring(player.iCurrentResource) +
			L" / " + std::to_wstring(player.iMaximumResource);
		/* 이 두 글자는 C++에 둔 1280×720 기준 좌표를 viewport의 scaleX/scaleY로 변환해
		그린다. 글자 크기는 두 배율 중 작은 textScale을 사용해 가로·세로 비율을 유지한다. */
		CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), hp.c_str(),
			position(504.419f, 635.273f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.315f * textScale);
		if (!bHasEmberPool)
		{
			CGameInstance::Get().Draw_Text(TEXT("Font_YG760"), mana.c_str(),
				position(835.169f, 635.273f), Colors::White, 0.f, float2_t(0.5f, 0.5f), 0.315f * textScale);
		}
	}
	/* 보스 HP·이름·등급 문자는 RenderBossHealthBarText가 실제 막대 rect를 기준으로 그린다. */
}

void CMainApp::RenderDamageNumbers()
{
	const auto& player = CCombatHUDViewModel::Get().Get_Player();
	if (!player.isValid || player.iServerTick < m_iLastRenderedDamageServerTick)
	{
		m_iLastRenderedDamageServerTick = 0u;
		m_FloatingDamageNumbers.clear();
	}
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel != ETOUI(LEVEL::BERN) &&
		currentLevel != ETOUI(LEVEL::VALTAN_ARENA) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) &&
		currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) &&
		currentLevel != ETOUI(LEVEL::KOUKUSAYDON_ARENA))
	{
		return;
	}
	if (CUIInputRouter::Get().Is_CinematicSuppressed())
	{
		for (const auto& event : CCombatHUDViewModel::Get().Get_DamageEvents())
			m_iLastRenderedDamageServerTick = (std::max)(m_iLastRenderedDamageServerTick, event.iServerTick);
		m_FloatingDamageNumbers.clear();
		return;
	}
	if (nullptr != m_pSkillWindowView && m_pSkillWindowView->Is_Open())
		return;

	/* 아래 값은 피해 숫자에 쓰는 30fps 배율 키다. 5프레임까지 2.65배로 커지고 18프레임에
	1배로 돌아온다. 글자 크기는 1080p 기준 32px로 계산하며, 위치와 종료 투명도는
	아래 화면 좌표 변환·fade 처리에서 별도로 적용한다. */
	constexpr f32_t DAMAGE_TIMELINE_FPS = 30.f;
	constexpr f32_t DAMAGE_SCALE_NORMAL[] = {
		1.00f, 1.25f, 2.00f, 2.33f, 2.65f, 2.60f, 2.55f, 2.50f, 2.17f,
		1.83f, 1.50f, 1.37f, 1.26f, 1.16f, 1.09f, 1.04f, 1.01f, 1.00f };
	/* 회복 숫자는 전체 구간에서 1.5배를 유지한다. */
	constexpr f32_t DAMAGE_SCALE_HEAL = 1.5f;
	constexpr f32_t DAMAGE_FONT_PX_START = 32.f;
	/* 배율 키를 모두 지난 뒤에는 마지막 크기를 유지한다. 유지·사라짐 시간은 아래
	상수로 정하고, 숫자 위치는 유지한 채 종료 구간의 알파만 줄인다. */
	constexpr f64_t DAMAGE_HOLD_SECONDS = 0.25;
	constexpr f64_t DAMAGE_FADE_SECONDS = 0.25;
	constexpr f64_t DAMAGE_NUMBER_LIFETIME_SECONDS =
		std::size(DAMAGE_SCALE_NORMAL) / DAMAGE_TIMELINE_FPS +
		DAMAGE_HOLD_SECONDS + DAMAGE_FADE_SECONDS;
	/* 피해 숫자 애니메이션을 동시에 보관할 최대 개수는 20개다. */
	constexpr size_t MAX_FLOATING_DAMAGE_NUMBERS = 20u;

	/* Get_DamageEvents는 이전에 보관한 타격도 반환하므로 마지막 처리 Server tick보다
	새로운 사건만 숫자로 만든다. 배열 위치 대신 tick을 기억해 앞쪽 항목 제거에 영향받지 않는다. */
	const std::vector<HUD_DAMAGE_EVENT>& damageEvents =
		CCombatHUDViewModel::Get().Get_DamageEvents();
	/* 이번 순회를 시작할 때의 커서와 비교한다. 같은 snapshot의 여러 타격이 같은 tick을
	가지므로 루프 안에서 비교 기준을 앞당기면 같은 tick의 후속 타격을 놓칠 수 있다. */
	const uint32_t iSpawnedUpToTick = m_iLastRenderedDamageServerTick;
	const auto* arena = currentLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA) ? CLevel_KoukuSaydonArena::Get_Active() : nullptr;
	for (const HUD_DAMAGE_EVENT& damageEvent : damageEvents)
	{
		if (damageEvent.iServerTick <= iSpawnedUpToTick)
			continue;
		m_iLastRenderedDamageServerTick =
			(std::max)(m_iLastRenderedDamageServerTick, damageEvent.iServerTick);
		if (arena && !arena->Should_ShowDamageWorldUI(damageEvent.Event.iTargetNetEntityId,
			damageEvent.Event.iSourcePlayerId)) continue;
		/* 카드 조각은 해당 문양을 배정받은 사냥꾼에게만 보이도록 로컬 플레이어 ID를 확인한다. */
		if (LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE !=
				damageEvent.Event.eCardMazeSuit &&
			damageEvent.Event.eCardMazeSuit !=
				CCombatHUDViewModel::Get().Get_KoukuGimmick().eCardMazeSuit)
		{
			continue;
		}
		FLOATING_DAMAGE_NUMBER number{};
		number.iTargetNetEntityId = damageEvent.Event.iTargetNetEntityId;
		number.iSourcePlayerId = damageEvent.Event.iSourcePlayerId;
		number.dSpawnSeconds = Product_Now_Seconds();
		number.vWorldPosition = float3_t(
			damageEvent.Event.fPositionX,
			damageEvent.Event.fPositionY,
			damageEvent.Event.fPositionZ);
		number.iAmount = damageEvent.Event.iAmount;
		number.isOutgoing = damageEvent.Event.isOutgoing;
		number.eCardMazeSuit = damageEvent.Event.eCardMazeSuit;
		number.eHitFlag = damageEvent.Event.eHitFlag;
		/* 한 번에 발생한 숫자가 한 점에 겹치지 않도록 타격별 초기 오프셋을 만든다.
		아래 범위는 이 프로젝트의 화면 공간 분산값이다. */
		{
			static std::mt19937 scatterRandom{ std::random_device{}() };
			std::uniform_real_distribution<f32_t> spreadX(-42.f, 42.f);
			std::uniform_real_distribution<f32_t> spreadY(-26.f, 12.f);
			number.fScatterX = spreadX(scatterRandom);
			number.fScatterY = spreadY(scatterRandom);
		}
		m_dLastDamageSeconds = number.dSpawnSeconds;
		if (number.iAmount != 0u || number.eHitFlag == LostArk::Shared::DAMAGE_HIT_FLAG::INVINCIBLE)
			m_FloatingDamageNumbers.push_back(number);
		const auto appendSuccess = [&](const uint8_t kind, const f32_t offsetY)
		{
			auto success = number;
			success.iMechanicSuccess = kind;
			success.fScatterX = 0.f;
			success.fScatterY = offsetY;
			m_FloatingDamageNumbers.push_back(success);
		};
		if (damageEvent.Event.isCounterSuccess) appendSuccess(1u, -64.f);
		if (damageEvent.Event.isStaggerSuccess) appendSuccess(2u,
			damageEvent.Event.isCounterSuccess ? -104.f : -64.f);
	}
	/* 피해 숫자 표시 옵션을 꺼도 위에서 사건 커서는 진행시켜 다시 켤 때 지난 타격을 재생하지 않는다. */
	if (!CUserSettings::Get().Is_DamageNumberShown())
	{
		std::erase_if(m_FloatingDamageNumbers, [](const auto& number) { return number.iMechanicSuccess == 0u; });
	}
	while (m_FloatingDamageNumbers.size() > MAX_FLOATING_DAMAGE_NUMBERS)
	{
		// 같은 tick의 연속 숫자가 카운터·무력화 성공 문구를 첫 렌더 전에 밀어내지 않게 한다.
		const auto amount = std::find_if(m_FloatingDamageNumbers.begin(), m_FloatingDamageNumbers.end(),
			[](const auto& number) { return number.iMechanicSuccess == 0u; });
		m_FloatingDamageNumbers.erase(amount == m_FloatingDamageNumbers.end() ?
			m_FloatingDamageNumbers.begin() : amount);
	}

	const f64_t dNow = Product_Now_Seconds();
	m_FloatingDamageNumbers.erase(
		std::remove_if(m_FloatingDamageNumbers.begin(), m_FloatingDamageNumbers.end(),
			[dNow](const FLOATING_DAMAGE_NUMBER& number)
			{
				return dNow - number.dSpawnSeconds >= DAMAGE_NUMBER_LIFETIME_SECONDS;
			}),
		m_FloatingDamageNumbers.end());
	if (m_FloatingDamageNumbers.empty())
		return;

	const float2_t viewportSize = CGameInstance::Get().Get_ViewportSize();
	if (viewportSize.x <= 0.f || viewportSize.y <= 0.f)
		return;
	const matrix_t view = XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW));
	const matrix_t projection = XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ));
	/* 전투 글자 크기 옵션(75~300%)을 전체 배율 애니메이션에 함께 적용한다. */
	const f32_t stageScale = viewportSize.y / 1080.f * CUserSettings::Get().Get_DamageFontScale();

	/* 카드 조각은 타격 위치에 문양·조각·개수를 표시한다. 한글 문자열은 아래 universal
	   character name 표기를 유지하며 Font_EventDamage로 그린다. */
	const auto shardText = [](const LostArk::Shared::MECHANIC_CARD_SYMBOL suit,
		const uint32_t count) -> wstring
	{
		const wchar_t* name = L"\uBB38\uC591";
		switch (suit)
		{
		case LostArk::Shared::MECHANIC_CARD_SYMBOL::HEART:
			name = L"\uD558\uD2B8"; break;
		case LostArk::Shared::MECHANIC_CARD_SYMBOL::SPADE:
			name = L"\uC2A4\uD398\uC774\uB4DC"; break;
		case LostArk::Shared::MECHANIC_CARD_SYMBOL::CLUB:
			name = L"\uD074\uB85C\uBC84"; break;
		case LostArk::Shared::MECHANIC_CARD_SYMBOL::DIAMOND:
			name = L"\uB2E4\uC774\uC544"; break;
		default: break;
		}
		return wstring(name) + L" \uC870\uAC01 x " + std::to_wstring(count);
	};
	for (const FLOATING_DAMAGE_NUMBER& number : m_FloatingDamageNumbers)
	{
		if (arena && !arena->Should_ShowDamageWorldUI(number.iTargetNetEntityId, number.iSourcePlayerId)) continue;
		const f64_t dAge = dNow - number.dSpawnSeconds;
		/* 해당 숫자의 프레임을 따라가되 키 범위를 지나면 마지막 배율을 유지한다. */
		const bool_t isHeal = LostArk::Shared::DAMAGE_HIT_FLAG::HEAL == number.eHitFlag;
		/* 치명타 여부는 아래 색상 분기에서 표현하고 배율 곡선은 일반 피해와 공유한다. */
		const f32_t* pCurve = DAMAGE_SCALE_NORMAL;
		const size_t iCurveKeys = std::size(DAMAGE_SCALE_NORMAL);
		const f32_t fFrame = static_cast<f32_t>(dAge) * DAMAGE_TIMELINE_FPS;
		f32_t fTimelineScale = DAMAGE_SCALE_HEAL;
		if (!isHeal)
		{
			const size_t iKey = (std::min)(static_cast<size_t>((std::max)(fFrame, 0.f)),
				iCurveKeys - 1u);
			const size_t iNext = (std::min)(iKey + 1u, iCurveKeys - 1u);
			const f32_t fBlend = (std::clamp)(fFrame - static_cast<f32_t>(iKey), 0.f, 1.f);
			fTimelineScale = pCurve[iKey] + (pCurve[iNext] - pCurve[iKey]) * fBlend;
		}
		const f32_t fFontPx = DAMAGE_FONT_PX_START * fTimelineScale;
		/* 숫자의 위치는 유지하고 마지막 구간에서만 알파를 줄인다. */
		const f64_t dTimeline = iCurveKeys / DAMAGE_TIMELINE_FPS;
		const f32_t fAlpha = dAge <= dTimeline + DAMAGE_HOLD_SECONDS ? 1.f :
			1.f - (std::clamp)(static_cast<f32_t>(
				(dAge - dTimeline - DAMAGE_HOLD_SECONDS) / DAMAGE_FADE_SECONDS), 0.f, 1.f);
		/* 사건의 월드 바닥 위치에서 조금 위를 기준점으로 잡고 투영한 뒤 화면 공간 오프셋을 더한다. */
		const vector_t vProjected = XMVector3Project(
			XMVectorSet(
				number.vWorldPosition.x,
				number.vWorldPosition.y + 1.f,
				number.vWorldPosition.z,
				1.f),
			0.f, 0.f, viewportSize.x, viewportSize.y, 0.f, 1.f,
			projection, view, XMMatrixIdentity());
		if (XMVectorGetZ(vProjected) < 0.f || XMVectorGetZ(vProjected) > 1.f)
			continue;
		const bool_t isShard =
			LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE != number.eCardMazeSuit;
		const bool_t isInvincible = LostArk::Shared::DAMAGE_HIT_FLAG::INVINCIBLE == number.eHitFlag;
		const bool_t isAbsorb = LostArk::Shared::DAMAGE_HIT_FLAG::ABSORB == number.eHitFlag;
		const wstring strAmount = number.iMechanicSuccess == 1u ? L"\uCE74\uC6B4\uD130" :
			number.iMechanicSuccess == 2u ? L"\uBB34\uB825\uD654" : isInvincible ? L"\uBB34\uC801" :
			isAbsorb ? L"\uD761\uC218" : isShard ?
			shardText(number.eCardMazeSuit, number.iAmount) :
			Format_ThousandsSeparated(number.iAmount);
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_EventDamage"), strAmount.c_str());
		const f32_t fScale = vMeasured.y > 0.f ? (fFontPx * stageScale) / vMeasured.y : 1.f;
		/* 나가는 피해는 흰색, 받는 피해는 빨간색을 기본으로 한다. Server가 전달한 hit flag에
		따라 치명타·빗나감·회복·흡수·무적의 색을 아래에서 덮어쓴다. 어떤 flag를 발행할지는
		이 렌더 함수가 판정하지 않는다. */
		vector_t vColor = number.isOutgoing ?
			XMVectorSet(1.f, 1.f, 1.f, fAlpha) :
			XMVectorSet(1.f, 0.f, 0.f, fAlpha);
		switch (number.eHitFlag)
		{
		case LostArk::Shared::DAMAGE_HIT_FLAG::CRITICAL:
		case LostArk::Shared::DAMAGE_HIT_FLAG::CRITICAL_DAMAGE_REDUCED:
			vColor = XMVectorSet(1.f, 204.f / 255.f, 0.f, fAlpha); break;
		case LostArk::Shared::DAMAGE_HIT_FLAG::MISS:
			vColor = XMVectorSet(0.6f, 0.6f, 0.6f, fAlpha); break;
		case LostArk::Shared::DAMAGE_HIT_FLAG::HEAL:
			vColor = XMVectorSet(0.f, 1.f, 0.f, fAlpha); break;
		case LostArk::Shared::DAMAGE_HIT_FLAG::ABSORB:
			vColor = XMVectorSet(0.2f, 0.65f, 1.f, fAlpha); break;
		case LostArk::Shared::DAMAGE_HIT_FLAG::INVINCIBLE:
			vColor = XMVectorSet(0.2f, 0.6f, 1.f, fAlpha); break;
		default: break;
		}
		/* 카드 조각은 피해 판정 대신 획득 표시이므로 흰색을 사용한다. */
		if (isShard)
			vColor = XMVectorSet(1.f, 1.f, 1.f, fAlpha);
		if (number.iMechanicSuccess == 1u) vColor = XMVectorSet(0.15f, 0.55f, 1.f, fAlpha);
		if (number.iMechanicSuccess == 2u) vColor = XMVectorSet(1.f, 0.9f, 0.f, fAlpha);
		const float2_t vDrawPosition(
			XMVectorGetX(vProjected) + number.fScatterX * stageScale,
			XMVectorGetY(vProjected) + number.fScatterY * stageScale);
		/* 밝은 배경에서도 글자를 읽을 수 있도록 색상 글자를 그리기 전에 주변 방향에 검은
		글자를 한 번씩 찍는다. 이 sprite font 경로는 흐림 필터 대신 외곽선을 근사한다. */
		const f32_t fGlowOffset = (std::max)(1.f, fFontPx * 0.06f * stageScale);
		const fvector_t vGlowColor = XMVectorSet(0.f, 0.f, 0.f, fAlpha);
		for (const float2_t& vStep : {
			float2_t(-1.f, 0.f), float2_t(1.f, 0.f), float2_t(0.f, -1.f), float2_t(0.f, 1.f),
			float2_t(-0.7f, -0.7f), float2_t(0.7f, -0.7f), float2_t(-0.7f, 0.7f),
			float2_t(0.7f, 0.7f) })
		{
			CGameInstance::Get().Draw_Text(TEXT("Font_EventDamage"), strAmount.c_str(),
				float2_t(vDrawPosition.x + vStep.x * fGlowOffset,
					vDrawPosition.y + vStep.y * fGlowOffset),
				vGlowColor, 0.f, float2_t(0.5f, 0.5f), fScale);
		}
		CGameInstance::Get().Draw_Text(TEXT("Font_EventDamage"), strAmount.c_str(),
			vDrawPosition, vColor, 0.f, float2_t(0.5f, 0.5f), fScale);
		if (number.eHitFlag == LostArk::Shared::DAMAGE_HIT_FLAG::DAMAGE_REDUCED ||
			number.eHitFlag == LostArk::Shared::DAMAGE_HIT_FLAG::CRITICAL_DAMAGE_REDUCED)
		{
			constexpr const wchar_t* label = L"\uD53C\uD574 \uAC10\uC18C";
			const float2_t measured = CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), label);
			const f32_t height = fFontPx * stageScale * 0.72f;
			const f32_t scale = measured.y > 0.f ? height / measured.y : fScale * 0.72f;
			const float2_t position(vDrawPosition.x,
				vDrawPosition.y + fFontPx * stageScale * 0.5f + height * 0.5f + 2.f * stageScale);
			CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), label,
				float2_t(position.x + stageScale, position.y + stageScale),
				vGlowColor, 0.f, float2_t(0.5f, 0.5f), scale);
			CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), label, position,
				XMVectorSet(1.f, 1.f, 1.f, fAlpha), 0.f, float2_t(0.5f, 0.5f), scale);
		}
	}
}

void CMainApp::RenderCinematicSubtitles()
{
    const auto viewport = CGameInstance::Get().Get_ViewportSize();
    if (viewport.x <= 0.f || viewport.y <= 0.f) return;
    struct SUBTITLE_LINE { std::wstring text; float2_t offset{}; f32_t scale = 1.f; };
    std::array<std::vector<SUBTITLE_LINE>, 2> lines;
    const auto splitLines = [](const std::string& utf8, std::vector<std::wstring>& target)
    {
        std::wstring text;
        if (!ConvertUtf8ToWide(utf8, text)) return;
        size_t begin = 0u;
        while (begin < text.size())
        {
            const size_t end = text.find(L'\n', begin);
            auto line = text.substr(begin, end == std::wstring::npos ? end : end - begin);
            if (!line.empty() && line.back() == L'\r') line.pop_back();
            if (!line.empty()) target.push_back(std::move(line));
            if (end == std::wstring::npos) break;
            begin = end + 1u;
        }
    };
    const auto appendLines = [&](const std::string& text, const bool upper,
        const float2_t offset = {}, const f32_t scale = 1.f)
    {
        if (!std::isfinite(offset.x) || !std::isfinite(offset.y) || !std::isfinite(scale) || scale <= 0.f) return;
        std::vector<std::wstring> parsed;
        splitLines(text, parsed);
        for (auto& line : parsed) lines[upper ? 1u : 0u].push_back({std::move(line), offset, scale});
    };
    if (m_pKoukuPresentationPlayer &&
        CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::KOUKUSAYDON_ARENA))
        for (const auto& subtitle : m_pKoukuPresentationPlayer->Collect_Subtitles())
            appendLines(subtitle.strText, subtitle.bUpper, subtitle.vScreenOffset, subtitle.fTextScale);

    std::vector<WORLD_SEQUENCE_SUBTITLE_SAMPLE> worldSubtitles;
    if (auto* arena = CLevel_ValtanArena::Get_Active())
        arena->Collect_SourceCinematicSubtitles(worldSubtitles);
#ifdef _DEBUG
    if (m_pMapTool) m_pMapTool->Collect_WorldSequenceSubtitles(worldSubtitles);
#endif
    std::set<std::pair<std::string, std::string>> seen;
    std::vector<WORLD_SEQUENCE_SUBTITLE_SAMPLE> balloons;
    for (const auto& subtitle : worldSubtitles)
    {
        if (!seen.emplace(subtitle.instanceId, subtitle.subtitleTrackId).second) continue;
        if (subtitle.position == "BALLOON") balloons.push_back(subtitle);
        // 저작된 쿠크 2관문 클리어 자막 배치와 같은 1080p 기준 단위를 사용한다.
        // 말풍선은 월드 기준점을 유지하며 별도 배치 계산을 사용한다.
        else appendLines(subtitle.text, subtitle.position == "UPPER", float2_t(0.f, 90.f), 2.f);
    }
    if (lines[0].empty() && lines[1].empty() && balloons.empty()) return;
    const CUITextLayerScope subtitleLayer(UI_TEXT_LAYER::PAGE);
    const f32_t uiScale = viewport.y / 1080.f;
    const f32_t fontHeight = 26.f * uiScale;
    const f32_t lineStep = 34.f * uiScale;
    const f32_t shadow = std::max(1.f, std::round(2.f * uiScale));
    const auto drawLine = [&](const std::wstring& line, f32_t x, f32_t y, f32_t width, f32_t height)
    {
        UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"), line.c_str(),
            x + shadow, y + shadow, height,
            XMVectorSet(0.f, 0.f, 0.f, 0.95f), width);
        UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"), line.c_str(),
            x, y, height, XMVectorSet(1.f, 1.f, 1.f, 1.f), width);
    };
    for (size_t side = 0u; side < lines.size(); ++side)
    {
        f32_t blockHeight = 0.f;
        for (const auto& line : lines[side]) blockHeight += lineStep * line.scale;
        f32_t y = side == 1u ? viewport.y * 0.10f : viewport.y * 0.90f - blockHeight;
        for (const auto& line : lines[side])
        {
            drawLine(line.text, viewport.x * 0.5f + line.offset.x * uiScale,
                y + line.offset.y * uiScale, viewport.x * 0.84f, fontHeight * line.scale);
            y += lineStep * line.scale;
        }
    }
    const matrix_t view = XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW));
    const matrix_t projection = XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ));
    for (const auto& subtitle : balloons)
    {
        const auto& anchor = subtitle.worldPosition;
        const vector_t point = XMVectorSet(anchor.x, anchor.y, anchor.z, 1.f);
        if (XMVectorGetW(XMVector4Transform(point, view * projection)) <= 0.f) continue;
        const vector_t projected = XMVector3Project(point, 0.f, 0.f, viewport.x, viewport.y,
            0.f, 1.f, projection, view, XMMatrixIdentity());
        const f32_t x = XMVectorGetX(projected), y = XMVectorGetY(projected), z = XMVectorGetZ(projected);
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
            z < 0.f || z > 1.f || x < 0.f || x > viewport.x || y < 0.f || y > viewport.y) continue;
        std::vector<std::wstring> bubbleLines;
        splitLines(subtitle.text, bubbleLines);
        f32_t textY = y - lineStep * static_cast<f32_t>(bubbleLines.size()) - 12.f * uiScale;
        for (const auto& line : bubbleLines)
        {
            drawLine(line, x, textY, viewport.x * 0.42f, fontHeight);
            textY += lineStep;
        }
    }
}

// 제품 텍스트 pass가 평활 FPS를 표시한다. 유효한 viewport·측정된 폰트 높이가 없으면 그 프레임만 건너뛴다.
void CMainApp::RenderFpsText()
{
	// Debug의 FPS 표시 모드는 항상/최근 피격 중/숨김이다.
	// Release는 아래 빌드 분기 밖에서 같은 노란 FPS 글자를 표시하며 저장 옵션은 바꾸지 않는다.
	if (m_fSmoothedFps <= 0.f)
		return;
#ifdef _DEBUG
	const int32_t iMode = CUserSettings::Get().Get_FpsDisplayMode();
	if (!(m_pProfilerTool && m_pProfilerTool->Is_CutsceneRecording()) &&
        (2 == iMode || (1 == iMode && Product_Now_Seconds() - m_dLastDamageSeconds > 6.0)))
		return;
#endif
	// Release는 저장 설정을 바꾸지 않고 프레임 확인용 FPS를 표시한다.
	const float2_t viewportSize = CGameInstance::Get().Get_ViewportSize();
	if (viewportSize.x <= 0.f || viewportSize.y <= 0.f)
		return;
    wchar_t text[160]{};
    Engine::FProfilerGeometryStats geometry{};
    const auto* profiler = CGameInstance::Get().Get_Profiler();
    const bool available = profiler && profiler->Get_GeometryStats(geometry);
    wchar_t vertices[32] = L"--", indices[32] = L"--";
    // Vtx는 버퍼 용량이 아니라 GPU의 실제 VS 실행 수다. Idx는 계측된 Engine draw의
    // 인스턴스 포함 참조 수이며 indirect도 완료된 args만 더한다. 미완료를 0으로 쓰지 않는다.
    if (available && geometry.VertexValid)
        ::_snwprintf_s(vertices, _countof(vertices), _TRUNCATE, L"%llu", static_cast<unsigned long long>(geometry.VSInvocations));
    if (available && geometry.IndexValid)
        ::_snwprintf_s(indices, _countof(indices), _TRUNCATE, L"%llu", static_cast<unsigned long long>(geometry.TotalIndices));
    ::_snwprintf_s(text, _countof(text), _TRUNCATE, L"FPS %d  |  Vtx(VS) %ls  |  Idx %ls  |  GPU -%lluF",
        static_cast<int32_t>(std::lround(m_fSmoothedFps)), vertices, indices,
        static_cast<unsigned long long>(available ? geometry.LatencyFrames : 0u));
	const float2_t measured = CGameInstance::Get().Measure_Text(TEXT("Font_YG760"), text);
	if (measured.y <= 0.f)
		return;
	const f32_t refScale = viewportSize.y / 720.f;
	CGameInstance::Get().Draw_Text(
		TEXT("Font_YG760"), text, float2_t(std::round(8.f * refScale), std::round(6.f * refScale)),
		XMVectorSet(1.f, 0.95f, 0.6f, 1.f), 0.f, float2_t(0.f, 0.f), 13.f * refScale / measured.y);
}

// 메인 루프 끝에서 현재 전경/배경 제한을 읽고 다음 시작 시각을 맞춘다.
// 제한이 없거나 이미 늦은 프레임은 현재 시각을 다음 기준으로 남긴다.
void CMainApp::Limit_FrameRate()
{
	const bool_t bForeground = IsWindowOwnedByCurrentProcess(GetForegroundWindow());
	const int32_t iLimit = CUserSettings::Get().Get_FrameLimit(bForeground);
	const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	if (iLimit <= 0)
	{
		m_LastFrameEnd = now;
		return;
	}
	// 제한 FPS의 역수는 프레임당 초다. steady_clock duration으로 변환해 직전 종료 기준에 더한다.
	const std::chrono::steady_clock::time_point target =
		m_LastFrameEnd + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
							 std::chrono::duration<f64_t>(1.0 / static_cast<f64_t>(iLimit)));
	if (now >= target)
	{
		// 이미 제한 시각을 지난 프레임은 따라잡기 반복 없이 지금부터 다음 주기를 시작한다.
		m_LastFrameEnd = now;
		return;
	}
	// 대부분의 남은 시간은 고해상도 타이머로 기다리고 마지막 짧은 구간은 spin으로 맞춘다.
	static const HANDLE s_hTimer =
		CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	std::chrono::steady_clock::time_point current = now;
	while (current < target)
	{
		const std::chrono::nanoseconds remain = target - current;
		if (nullptr != s_hTimer && remain > std::chrono::milliseconds(2))
		{
			LARGE_INTEGER due{};
			// Win32 상대 timer 시각은 음수 100ns 단위다. 마지막 1ms를 남기고 나머지만 기다린다.
			due.QuadPart = -static_cast<LONGLONG>((remain - std::chrono::milliseconds(1)).count() / 100);
			if (SetWaitableTimerEx(s_hTimer, &due, 0, nullptr, nullptr, nullptr, 0))
				WaitForSingleObject(s_hTimer, 20);
		}
		else
		{
			YieldProcessor();
		}
		current = std::chrono::steady_clock::now();
	}
	m_LastFrameEnd = target;
}

// 초기화가 제품 텍스트용 폰트를 등록한다. 필수 폰트 실패는 E_FAIL, 선택적 크기 변형 누락은 로그 후 계속한다.
// 앞서 등록한 폰트를 이 함수에서 개별 rollback하지 않으며 전체 초기화 실패 정리는 앱 종료 경로가 담당한다.
HRESULT CMainApp::Ready_Fonts()
{
	const filesystem::path fontPath = CRuntimeAssetRoot::Resolve_Font(L"161ex.spritefont");
	if (fontPath.empty() || FAILED(CGameInstance::Get().Add_Font(TEXT("Font_Default"), fontPath.c_str())))
	{
		return E_FAIL;
	}

	// 원본 FontMap의 역할에 맞춘 spritefont 등록이다. 기존 Font 태그를 유지해 제품 텍스트 소비자가 같은 이름을 사용한다.
	struct SOURCE_FONT
	{
		const tchar_t *strTag;
		const wchar_t *strFile;
	};
	constexpr SOURCE_FONT sourceFonts[] = {
		{TEXT("Font_YG760"), L"YG760.spritefont"},
		{TEXT("Font_YG330"), L"YG330.spritefont"},
		{TEXT("Font_YoonGasiIIM"), L"YoonGasiIIM.spritefont"},
		// Font_159는 한글 없는 95자 ASCII 부분집합이다. 숫자와 X N처럼 ASCII만 필요한 보스 HP·줄 수에 사용한다.
		{TEXT("Font_159"), L"159.spritefont"},
		// 피해 숫자 태그는 원본 DamageTextCBT2의 YoonGasiIIM 계열에 대응한다.
		// eventDamageFont라는 이름의 별도 원본 토큰과 혼동하지 않도록 태그는 유지하고 실제 폰트는 YoonGasiIIM을 등록한다.
		{TEXT("Font_EventDamage"), L"YoonGasiIIM.spritefont"},
	};

	for (const SOURCE_FONT &sourceFont : sourceFonts)
	{
		const filesystem::path sourceFontPath = CRuntimeAssetRoot::Resolve_Font(sourceFont.strFile);
		if (sourceFontPath.empty() || FAILED(CGameInstance::Get().Add_Font(sourceFont.strTag, sourceFontPath.c_str())))
		{
			return E_FAIL;
		}
	}

	// 작은 라벨은 UILabelFont::BAKED_SIZES에 맞춘 별도 크기 파일을 선택적으로 등록한다.
	// 누락은 로그만 남기며 UILabelFont의 기존 큰 폰트 대체 경로를 사용한다.
	for (const wchar_t *pFamily : {L"YG760", L"YoonGasiIIM"})
	{
		for (const int32_t iSize : UILabelFont::BAKED_SIZES)
		{
			const wstring strTag = wstring(L"Font_") + pFamily + L"_" + std::to_wstring(iSize);
			const wstring strFile = wstring(L"UI/Fonts/") + pFamily + L"_" + std::to_wstring(iSize) + L".spritefont";
			const filesystem::path smallFontPath = CRuntimeAssetRoot::Resolve(strFile);
			if (smallFontPath.empty() || FAILED(CGameInstance::Get().Add_Font(strTag, smallFontPath.c_str())))
				OutputDebugStringW((L"[Fonts] optional small label font missing: " + strFile + L"\n").c_str());
		}
	}

	// 큰 Latin 표시는 별도 raster 크기를 등록한다. Resolve_LatinDisplay 소비용이며 일반 라벨 선택에는 쓰지 않는다.
	for (const int32_t iSize : UILabelFont::LATIN_DISPLAY_SIZES)
	{
		const wstring strTag = wstring(L"Font_YoonGasiIIM_Latin") + std::to_wstring(iSize);
		const wstring strFile = wstring(L"UI/Fonts/YoonGasiIIM_Latin") + std::to_wstring(iSize) + L".spritefont";
		const filesystem::path displayFontPath = CRuntimeAssetRoot::Resolve(strFile);
		if (displayFontPath.empty() || FAILED(CGameInstance::Get().Add_Font(strTag, displayFontPath.c_str())))
			OutputDebugStringW((L"[Fonts] optional display font missing: " + strFile + L"\n").c_str());
	}

	return S_OK;
}

// MainApp이 제품 공통 shader·quad·effect·UI 원형을 STATIC 영역에 등록한다. 원형 보관과 이후 Clone은 GameInstance의 Prototype_Manager가 맡는다.
HRESULT CMainApp::Ready_Prototype_For_Static()
{
	if (FAILED(CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::STATIC), CSkillGroundTargetPreview::SHADER_TAG,
			CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxSkillGroundTargetPreview.hlsl"),
							VTXTEX::Elements, VTXTEX::iNumElements))) ||
		FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxTex"),
												  CShader::Create(m_pDevice, m_pContext,
																  TEXT("../Bin/ShaderFiles/Shader_VtxTex.hlsl"),
																  VTXTEX::Elements, VTXTEX::iNumElements))) ||
		FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
												  CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

	if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), TEXT("Prototype_GameObject_EffectObject"),
												  CEffectObject::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}
	if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), CSkillGroundTargetPreview::PROTOTYPE_TAG,
												  CSkillGroundTargetPreview::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}
	if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), CClickMoveEffect::PROTOTYPE_TAG,
												  CClickMoveEffect::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

	// Loading과 제품 UI가 clone할 공용 sprite prototype을 등록한다.
	if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Sprite"),
												  CUI_Sprite::Create(m_pDevice, m_pContext))))
	{
		return E_FAIL;
	}

	// Loading 문서의 Resources 상대 texture 경로를 prototype key로 미리 등록한다.
	// Loading의 sprite clone도 같은 경로로 texture를 조회한다.
	return Ready_Prototype_For_LoadingChrome();
}

// Loading 전에 layout과 목적지별 배경 texture를 STATIC prototype으로 준비한다.
// 문서 누락·parse 오류는 기존 S_OK 정책으로 건너뛰고 실제 prototype 등록 실패만 E_FAIL로 반환한다.
HRESULT CMainApp::Ready_Prototype_For_LoadingChrome()
{
	const filesystem::path layoutPath = CProjectDataRoot::Resolve(L"UI/Loading/LoadingLayout.json");

	ifstream stream(layoutPath);
	if (!stream.is_open())
		return S_OK;

	const string text((istreambuf_iterator<char>(stream)), istreambuf_iterator<char>());

	DATA_JSON_VALUE root;
	string error;
	if (!CDataJson::Parse(text, root, error))
		return S_OK;

	const DATA_JSON_VALUE *pSlots = root.Find("slots");
	if (nullptr == pSlots || !pSlots->Is_Array())
		return S_OK;

	// 문서가 같은 path를 여러 layer에서 사용해도 한 번만 등록한다. 빈/잘못된 layer와 루트 밖 경로는 건너뛴다.
	vector<wstring_t> registeredPaths;
	for (const DATA_JSON_VALUE &slot : pSlots->Get_Array())
	{
		const DATA_JSON_VALUE *pLayers = slot.Find("layers");
		if (nullptr == pLayers || !pLayers->Is_Array())
			continue;

		for (const DATA_JSON_VALUE &layer : pLayers->Get_Array())
		{
			const DATA_JSON_VALUE *pPath = layer.Find("path");
			if (nullptr == pPath || !pPath->Is_String() || pPath->Get_String().empty())
				continue;

			// 기존 Loading 경로 계약의 ASCII byte를 wchar_t로 확장한다. UTF-8 다중 byte를 변환하는 경로는 아니다.
			const string &narrowPath = pPath->Get_String();
			const wstring_t widePath(narrowPath.begin(), narrowPath.end());

			if (registeredPaths.end() != find(registeredPaths.begin(), registeredPaths.end(), widePath))
				continue;
			registeredPaths.push_back(widePath);

			const filesystem::path resolvedPath = CRuntimeAssetRoot::Resolve(widePath);
			if (resolvedPath.empty())
				continue;

			if (FAILED(CGameInstance::Get().Add_Prototype(
					ETOUI(LEVEL::STATIC), widePath, CTexture::Create(m_pDevice, m_pContext, resolvedPath.c_str(), 1))))
			{
				return E_FAIL;
			}
		}
	}

	// Loading이 목적지에 따라 선택하는 배경은 기본 layout의 layer 목록에 없으므로 별도로 등록한다.
	// 이 prototype이 있어야 목적지 배경으로 바뀐 sprite가 같은 경로 key를 찾을 수 있다.
	for (const wchar_t *pLoadingBackground :
		 {L"UI/Loading/Loading_Background_Valtan.png", L"UI/Loading/Loading_Background_Kouku.png",
		  L"UI/Loading/Loading_Background_Prologue.png", L"UI/Loading/Loading_Background_Maharaka.png",
		  L"UI/Loading/Loading_Background_Sea_0.png", L"UI/Loading/Loading_Background_Colosseum.png"})
	{
		const filesystem::path resolvedPath = CRuntimeAssetRoot::Resolve(pLoadingBackground);
		if (!resolvedPath.empty() && FAILED(CGameInstance::Get().Add_Prototype(
										 ETOUI(LEVEL::STATIC), pLoadingBackground,
										 CTexture::Create(m_pDevice, m_pContext, resolvedPath.c_str(), 1))))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

// MainApp이 목적지 로딩을 시작하는 경계다. Loading을 만들고 프로필을 적용한 뒤 GameInstance의 Level_Manager로 소유권을 넘긴다.
// 교체 실패 시 프로필 복원을 시도하며 목적지 Level의 실제 생성은 이후 ACTIVATE 요청에서 처리한다.
HRESULT CMainApp::Start_Level(const LEVEL eTargetLevel, const LOBBY_COMMAND_TOKEN lobbyCommandToken)
{
	// STATIC 재련 UI와 반복 소리는 Loading 전환을 시작할 때 정리한다.
	// 이후 검사에서 실패해도 이미 수행한 숨김·소리 정지를 되돌리지는 않는다.
	m_bItemUpgradePreviewVisible = false;
	Hide_ItemUpgrade();
	CGameInstance::Get().Stop_LoopingSound();

	const CLIENT_LEVEL_DESCRIPTOR *pTarget = CLevelRegistry::Find(eTargetLevel);
	if (nullptr == pTarget || nullptr == pTarget->pRenderingProfileId ||
		!m_RenderingProfiles.Has_Profile(pTarget->pRenderingProfileId) ||
		!m_RenderingProfiles.Has_Profile(CRenderingProfileService::LOADING_PROFILE_ID))
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_ACTIVATION_PROFILE_MISSING,
			"main-app.start-level-profile", "Target or Loading rendering profile is not registered.", E_INVALIDARG);
		return E_INVALIDARG;
	}

	unique_ptr<CLevel_Loading> loading = CLevel_Loading::Create(
		m_pDevice, m_pContext, eTargetLevel, lobbyCommandToken,
		LEVEL::LOBBY == eTargetLevel && m_bOpenCharacterSelectOnLobby ? m_pCharacterSelectWindowView.get() : nullptr);
	if (nullptr == loading)
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_LOADING_START_FAILED, "main-app.loading-level-create",
			"CLevel_Loading::Create returned null.", E_FAIL);
		return E_FAIL;
	}

	// Loading profile만 먼저 활성화한다. Level 교체 실패에 대비해 이전 profile identity를 보관한다.
	const string previousProfileId = m_RenderingProfiles.Get_ActiveProfileId();
	string status;
	if (!m_RenderingProfiles.Activate_Profile(CRenderingProfileService::LOADING_PROFILE_ID, status))
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_ACTIVATION_PROFILE_FAILED,
			"main-app.loading-profile-activation", status, E_FAIL);
		return E_FAIL;
	}
	CPresentation_Manager::Get().Clear_Frame();
	const HRESULT hChange = CGameInstance::Get().Change_Level(ETOUI(LEVEL::LOADING), move(loading));
	if (FAILED(hChange))
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_LOADING_START_FAILED, "main-app.loading-change-level",
			"Change_Level(LOADING) failed.", hChange);
		if (!previousProfileId.empty())
		{
			string rollbackStatus;
			if (!m_RenderingProfiles.Activate_Profile(previousProfileId, rollbackStatus))
			{
				OutputDebugStringA(("[MainApp] Loading profile rollback failed: " + rollbackStatus + "\n").c_str());
			}
		}
		return hChange;
	}
	return S_OK;
}

// UI·Level·Loader는 전환 요청만 예약하고 MainApp이 다음 Update 시작에서 LOAD/ACTIVATE를 소비한다. 갱신 중인 Level을 그 자리에서 파괴하지 않기 위한 경계다.
// LOAD 실패는 pending 정리·연결 종료 후 현 Level을 유지한다. ACTIVATE 실패는 대상 자원 정리와 필요 시 프로필 복원 후 Lobby 복구를 예약한다.
void CMainApp::Apply_LevelRequest()
{
	// 모델 decode는 협력 취소 경계에서 끝난다. 준비 작업이 종료될 때까지 요청을 큐에 남기고 현재 장면을 갱신한다.
	// 취소 완료를 확인한 뒤에만 전환 요청을 소비한다.
	if (CLevelTransitionService::Is_Pending())
	{
		Engine::CProfilerScope drainScope(CGameInstance::Get().Get_Profiler(), "CharacterAssets.LevelTransition.Drain");
		CPlayableCharacterAssetService::Cancel_AllAsyncPreparations();
		if (CPlayableCharacterAssetService::Has_ActivePreparations())
			return;
	}
	LEVEL_TRANSITION_REQUEST request{};
	if (!CLevelTransitionService::Try_Consume(request))
		return;
#ifdef _DEBUG
	if (m_pEffectTool)
		m_pEffectTool->Deactivate_AuthoringWorkspace();
	if (m_pEffectToolV2)
		m_pEffectToolV2->Deactivate();
	if (m_pWorldObjectTool)
		m_pWorldObjectTool->Deactivate();
	StopCompositionPreview(m_eCompositionPreviewOwner);
	m_pWorldObjectCatalogSource = nullptr;
	m_iWorldObjectCatalogGeneration = UINT64_MAX;
#endif
	const uint32_t iPreviousLevel = CGameInstance::Get().Get_CurrentLevelID();

	if (LEVEL_TRANSITION_PHASE::LOAD == request.ePhase)
	{
		// 캐릭터 선택·연결 종료 등으로 월드에서 로비로 복귀할 때 현재 캐릭터 보유 상태를 capture한다.
		if (LEVEL::LOBBY == request.eTargetLevel &&
			(iPreviousLevel == ETOUI(LEVEL::BERN) || iPreviousLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
			 iPreviousLevel == ETOUI(LEVEL::KOUKUSAYDON_ARENA) || iPreviousLevel == ETOUI(LEVEL::MAHARAKA) ||
			 iPreviousLevel == ETOUI(LEVEL::COLOSSEUM)))
			CCharacterSelectionState::Capture_ActiveWorldState();
		const HRESULT result = Start_Level(request.eTargetLevel, request.iLobbyCommandToken);
		if (FAILED(result))
		{
			CCharacterSelectionState::Cancel_PendingCreation();
			if (INVALID_LOBBY_COMMAND_TOKEN != request.iLobbyCommandToken)
			{
				CLobbyCommandService::Cancel(request.iLobbyCommandToken, "target level loading could not start");
			}
			CLevelTransitionService::Report_Recovery(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_LOADING_START_FAILED, "main-app.start-level-request",
				request.strSource, result);
			// Server 입장 승인으로 확보한 연결을 닫아 다음 로비 시도가 새 연결 진단 세대로 시작하게 한다.
			// Loading 전환에 실패한 기존 바인딩을 재사용하지 않는다.
			// 실제 연결 종료·룸 회수는 기존 Network/Server 경계가 처리한다.
			CNetworkManager::Get().Close_ServerConnection();
		}
		else
		{
			CEffectPresentationService::Clear_Level(iPreviousLevel);
		}
		return;
	}

	const CLIENT_LEVEL_DESCRIPTOR *pTarget = CLevelRegistry::Find(request.eTargetLevel);
	const bool_t hasTargetProfile = nullptr != pTarget && nullptr != pTarget->pRenderingProfileId &&
									m_RenderingProfiles.Has_Profile(pTarget->pRenderingProfileId);
	if (!hasTargetProfile)
	{
		OutputDebugStringA("[MainApp] Activation target has no registered rendering profile.\n");
	}
	unique_ptr<CLevel> nextLevel =
		hasTargetProfile ? CLevelRegistry::Create_Level(request.eTargetLevel, m_pDevice, m_pContext) : nullptr;
	const bool_t levelCreated = nullptr != nextLevel;
	if (hasTargetProfile && nullptr == nextLevel)
	{
		OutputDebugStringA("[MainApp] Create_Level returned null (target Level::Initialize failed).\n");
	}
	const string previousProfileId = m_RenderingProfiles.Get_ActiveProfileId();
	const string previousLevelQualityId = m_RenderingProfiles.Get_LevelQualityProfileId();
	string profileStatus;
	const bool_t profileActivated =
		nullptr != nextLevel && m_RenderingProfiles.Activate_LevelProfile(pTarget->pRenderingProfileId, profileStatus);
	if (nullptr != nextLevel && !profileActivated)
	{
		OutputDebugStringA(("[MainApp] Target rendering profile activation failed: " + profileStatus + "\n").c_str());
	}
#ifdef _DEBUG
	if (profileActivated && nullptr != m_pCameraTool)
		m_pCameraTool->On_LevelChanged();
	if (profileActivated && nullptr != m_pEffectToolV2)
		m_pEffectToolV2->On_LevelChanged();
#endif
	if (profileActivated)
		CPresentation_Manager::Get().Clear_Frame();
	// 대상 생성과 profile 활성화가 모두 성공해야 Change_Level로 소유권을 넘긴다. identity 생성 확정은 실제 Bern 교체 성공 뒤다.
	const bool_t levelChanged =
		profileActivated && SUCCEEDED(CGameInstance::Get().Change_Level(ETOUI(request.eTargetLevel), move(nextLevel)));
	if (levelChanged)
	{
#ifdef _DEBUG
		if (nullptr != m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel->On_LevelChanged();
		if (nullptr != m_pAnimationTool)
			m_pAnimationTool->On_LevelChanged();
		if (m_pCharacterActionWorkbench)
			m_pCharacterActionWorkbench->On_LevelChanged();
		if (nullptr != m_pValtanActionWorkbench)
			m_pValtanActionWorkbench->On_LevelChanged();
		CKoukuSaydonPatternAuditionService::Get().Reset(
			"Level changed; no KoukuSaydon Server pattern is Live in this Client session.");
		if (nullptr != m_pEquipmentAuthoringTool)
			m_pEquipmentAuthoringTool->On_LevelChanged();
#endif
		CEffectPresentationService::Clear_Level(iPreviousLevel);
		if (LEVEL::BERN == request.eTargetLevel && CCharacterSelectionState::Has_PendingCreation() &&
			!CCharacterSelectionState::Commit_PendingCreation())
		{
			OutputDebugStringA("[MainApp] Bern identity commit invariant failed.\n");
			CNetworkManager::Get().Close_ServerConnection();
			CLevelTransitionService::Report_Recovery(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_IDENTITY_COMMIT_FAILED, "main-app.identity-commit",
				"Bern pending identity commit invariant failed.", E_FAIL);
			if (!CLevelTransitionService::Request_Load(LEVEL::LOBBY, "main-app.identity-commit-failure"))
			{
				OutputDebugStringA("[MainApp] Failed to stage Lobby after identity commit failure.\n");
			}
			return;
		}
		return;
	}
	// 메모리 교체 전 실패한 profile 변경은 기존 품질·장면 ID로 복원을 시도한다. 복원 실패를 원래 실패의 성공으로 처리하지 않는다.
	if (profileActivated && !previousProfileId.empty())
	{
		string rollbackStatus;
		const string &rollbackQualityId = previousLevelQualityId.empty() ? previousProfileId : previousLevelQualityId;
		if (!m_RenderingProfiles.Activate_LevelProfile(rollbackQualityId, rollbackStatus) ||
			(previousProfileId != rollbackQualityId &&
			 !m_RenderingProfiles.Activate_Profile(previousProfileId, rollbackStatus)))
		{
			OutputDebugStringA(
				("[MainApp] Rendering profile rollback failed after level activation failure: " + rollbackStatus + "\n")
					.c_str());
		}
	}

	CCharacterSelectionState::Cancel_PendingCreation();
	if (INVALID_LOBBY_COMMAND_TOKEN != request.iLobbyCommandToken)
	{
		CLobbyCommandService::Cancel(request.iLobbyCommandToken, "target level activation failed");
	}
	// 활성화 실패의 대상 자원만 정리하고 최초 실패 단계에 맞는 복구 진단을 남긴다.
	CGameInstance::Get().Clear_Resources(ETOUI(request.eTargetLevel));
	if (!hasTargetProfile)
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_ACTIVATION_PROFILE_MISSING,
			"main-app.target-profile-missing", request.strSource, E_FAIL);
	}
	else if (!levelCreated)
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_ACTIVATION_LEVEL_CREATE_FAILED,
			"main-app.target-level-create", request.strSource, E_FAIL);
	}
	else if (!profileActivated)
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_ACTIVATION_PROFILE_FAILED,
			"main-app.target-profile-activation", profileStatus, E_FAIL);
	}
	else
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::CLIENT_ACTIVATION_CHANGE_LEVEL_FAILED,
			"main-app.target-change-level", request.strSource, E_FAIL);
	}
	CNetworkManager::Get().Close_ServerConnection();
	if (!CLevelTransitionService::Request_Load(LEVEL::LOBBY, "main-app.activation-failure"))
	{
		OutputDebugStringA("[MainApp] Failed to stage Lobby recovery after activation failure.\n");
	}
}

// MainApp이 ImGuiLayer를 보유해 매 프레임 Begin/Render를 제품 루프에 연결한다. 종료 때 엔진보다 먼저 backend를 Shutdown한다.
HRESULT CMainApp::ReadyImGuiRuntime()
{
	m_pImGuiLayer = make_unique<Engine::CImGuiLayer>();
	if (!m_pImGuiLayer->Initialize(g_hWnd, m_pDevice.Get(), m_pContext.Get()))
	{
		return E_FAIL;
	}

#ifndef _DEBUG
	// Release는 제품 UI를 주 창에서 유지하고 저작용 docking·추가 viewport 및 ini 저장을 끈다.
	ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable);
	ImGui::GetIO().IniFilename = nullptr;
#endif
	return S_OK;
}

// 여러 입력 경로가 같은 창 소유 판정을 쓰도록 본체에 유지한다. 
// HWND는 차용하며 null이나 PID 조회 실패는 false다.
// 제품 단축키와 프레임 제한에서도 현재 프로세스의 전경 창인지 확인하므로 Release에서 함께 사용한다.
bool_t CMainApp::IsWindowOwnedByCurrentProcess(HWND hWnd)
{
	if (nullptr == hWnd)
		return false;

	DWORD processId = {};
	return 0 != GetWindowThreadProcessId(hWnd, &processId) &&
		GetCurrentProcessId() == processId;
}
#ifdef _DEBUG
HRESULT CMainApp::ReadyDebugTools()
{
	if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Reset_History();
		pProfiler->Set_Enabled(false);
	}
	m_bProfilerVisible = false;
	m_pHUDLayoutTool =
		make_unique<CHUDLayoutTool>(m_pDevice, m_pContext);
	return S_OK;
}

#endif

bool_t CMainApp::IsDebugToolVisible(const DEBUG_TOOL eTool) const
{
	if (eTool == DEBUG_TOOL::WORLD_OBJECT || eTool == DEBUG_TOOL::SEQUENCER_BENCHMARK)
		return m_pSequencerTool && m_DebugToolVisible[static_cast<size_t>(DEBUG_TOOL::SEQUENCER)] &&
			m_pSequencerTool->Get_SelectedTarget() == (eTool == DEBUG_TOOL::WORLD_OBJECT ?
				COMPOSITION_WORKBENCH_TARGET::OBJECT : COMPOSITION_WORKBENCH_TARGET::SEQUENCE);
	const DEBUG_TOOL eCanonicalTool = DEBUG_TOOL::EFFECT_COMPOSITION == eTool ?
		DEBUG_TOOL::EFFECT :
		(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH == eTool ||
		 DEBUG_TOOL::KOUKU_SAYDON_ACTION_WORKBENCH == eTool) ? DEBUG_TOOL::SEQUENCER : eTool;
	const size_t iTool = static_cast<size_t>(eCanonicalTool);
	return DEBUG_TOOL::NONE != eCanonicalTool &&
		DEBUG_TOOL::COUNT != eCanonicalTool &&
		iTool < m_DebugToolVisible.size() && m_DebugToolVisible[iTool];
}

void CMainApp::SetDebugToolVisible(
	const DEBUG_TOOL eTool,
	const bool_t bVisible)
{
	const DEBUG_TOOL eCanonicalTool = DEBUG_TOOL::EFFECT_COMPOSITION == eTool ?
		DEBUG_TOOL::EFFECT :
		(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH == eTool ||
		 DEBUG_TOOL::KOUKU_SAYDON_ACTION_WORKBENCH == eTool) ? DEBUG_TOOL::SEQUENCER : eTool;
	if (eTool == DEBUG_TOOL::WORLD_OBJECT || eTool == DEBUG_TOOL::SEQUENCER_BENCHMARK)
	{
		if (bVisible && m_pSequencerTool)
			m_pSequencerTool->Open(eTool == DEBUG_TOOL::WORLD_OBJECT ?
				COMPOSITION_WORKBENCH_TARGET::OBJECT : COMPOSITION_WORKBENCH_TARGET::SEQUENCE);
		if (bVisible || IsDebugToolVisible(eTool)) SetDebugToolVisible(DEBUG_TOOL::SEQUENCER, bVisible);
		return;
	}
	const size_t iTool = static_cast<size_t>(eCanonicalTool);
	if (DEBUG_TOOL::NONE == eCanonicalTool ||
		DEBUG_TOOL::COUNT == eCanonicalTool ||
		iTool >= m_DebugToolVisible.size())
	{
		return;
	}

	m_DebugToolVisible[iTool] = bVisible;
	if (DEBUG_TOOL::SEQUENCER == eCanonicalTool)
	{
		m_DebugToolVisible[static_cast<size_t>(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH)] = false;
		m_DebugToolVisible[static_cast<size_t>(DEBUG_TOOL::KOUKU_SAYDON_ACTION_WORKBENCH)] = false;
	}
	if (DEBUG_TOOL::EFFECT == eCanonicalTool)
	{
        m_DebugToolVisible[static_cast<size_t>(DEBUG_TOOL::EFFECT_COMPOSITION)] = false;
	}
	if (!bVisible)
	{
#ifdef _DEBUG
		if (eCanonicalTool == DEBUG_TOOL::SEQUENCER)
		{
			if (m_pSequencerTool) m_pSequencerTool->Deactivate();
			StopCompositionPreview(m_eCompositionPreviewOwner);
			if (m_eDebugInputOwner == DEBUG_TOOL::WORLD_OBJECT || m_eDebugInputOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
				m_eDebugInputOwner = DEBUG_TOOL::NONE;
		}
		if (eCanonicalTool == DEBUG_TOOL::SEQUENCER || eCanonicalTool == DEBUG_TOOL::SEQUENCER_BENCHMARK)
			StopCompositionPreview(eCanonicalTool);
#endif
		if (m_eDebugInputOwner == eCanonicalTool)
			m_eDebugInputOwner = DEBUG_TOOL::NONE;
		if (m_eDebugWindowFocusPending == eCanonicalTool)
			m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
#ifdef _DEBUG
		if (DEBUG_TOOL::MAP == eCanonicalTool && nullptr != m_pMapTool)
			m_pMapTool->SetOpen(false);
		else if (DEBUG_TOOL::WORLD_SCENE == eCanonicalTool && m_pWorldSceneTool)
			m_pWorldSceneTool->Hide();
		else if (DEBUG_TOOL::WORLD_OBJECT == eCanonicalTool && m_pWorldObjectTool)
			m_pWorldObjectTool->Deactivate();
		else if (DEBUG_TOOL::CAMERA == eCanonicalTool && nullptr != m_pCameraTool)
			m_pCameraTool->Deactivate();
        else if (DEBUG_TOOL::EFFECT == eCanonicalTool && m_pEffectTool)
            m_pEffectTool->Deactivate_AuthoringWorkspace();
        else if (DEBUG_TOOL::EFFECT_V2 == eCanonicalTool && m_pEffectToolV2)
            m_pEffectToolV2->Deactivate();
#endif
	}
}

#ifdef _DEBUG
void CMainApp::ClaimCompositionPreviewOwner(const DEBUG_TOOL owner)
{
	if (m_eCompositionPreviewOwner == owner) return;
	auto* previousWorkbench = m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER ?
		m_pKoukuSaydonActionWorkbench.get() :
		(m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK ? m_pSequenceActionWorkbench.get() : nullptr);
	auto* previousShell = m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER ? m_pSequencerTool.get() :
		(m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK ? m_pSequencerTool.get() : nullptr);
    if (m_eCompositionPreviewOwner == DEBUG_TOOL::SEQUENCER_BENCHMARK)
        if (auto* arena = CLevel_KoukuSaydonArena::Get_Active())
            arena->Notify_SequencePlaybackEnded();
	if (previousWorkbench)
	{
		previousWorkbench->Cancel_CompleteSequencePlay();
		auto state = previousWorkbench->Get_PreviewState();
		state.bPlaying = false;
		state.bPaused = true;
		previousWorkbench->Set_PreviewState(state);
	}
	if (previousShell)
	{
		auto state = previousShell->Get_AnimationPreviewState();
		state.bPlaying = false;
		state.bPaused = true;
		previousShell->Set_AnimationPreviewState(std::move(state));
	}
	m_eCompositionPreviewOwner = owner;
	m_eColliderAuthoringOwner = DEBUG_TOOL::NONE;
	m_iColliderAuthoringDraftGeneration = UINT64_MAX;
}

void CMainApp::StopCompositionPreview(const DEBUG_TOOL owner)
{
    if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_IsServerClock()) return;
	if (owner == DEBUG_TOOL::NONE || m_eCompositionPreviewOwner != owner) return;
	if (owner == DEBUG_TOOL::SEQUENCER_BENCHMARK) ClearKoukuSequenceArrivals();
	if (m_pAnimationTool) { std::string stoppedAnimationStatus; (void)m_pAnimationTool->Stop_KoukuCompositionPreview(stoppedAnimationStatus); }
	if (m_pKoukuPresentationPlayer) m_pKoukuPresentationPlayer->Stop_Preview();
	if (auto* arena = CLevel_KoukuSaydonArena::Get_Active()) arena->Debug_StopCompositionWorldPreview();
	ClaimCompositionPreviewOwner(DEBUG_TOOL::NONE);
}

#endif

void CMainApp::CloseAllDebugTools()
{
	for (size_t iTool = static_cast<size_t>(DEBUG_TOOL::NONE) + 1u;
		iTool < static_cast<size_t>(DEBUG_TOOL::COUNT); ++iTool)
	{
		const DEBUG_TOOL eTool = static_cast<DEBUG_TOOL>(iTool);
		SetDebugToolVisible(eTool, false);
	}
	m_strToolStatus = "All authoring windows hidden; domain drafts remain owned by their tools.";
}

#ifdef _DEBUG
HRESULT CMainApp::EnsureAnimationPreviewBackend()
{
	// Resources 선택에는 공용 CModel 미리보기 소유자가 필요하지만 이 준비 과정은
	// Animation Tool 창을 표시하거나 포커스를 옮기지 않는다. 명시적인 열기는 EnsureDebugTool을 거친다.
	if (nullptr == m_pCharacterPreviewPanel)
		m_pCharacterPreviewPanel =
			make_shared<CCharacterPreviewPanel>(m_pDevice, m_pContext);
	if (nullptr == m_pBalanceTool)
		m_pBalanceTool = make_unique<CBalanceTool>(false);
	if (nullptr == m_pValtanBossTool)
		m_pValtanBossTool = make_unique<CValtanBossTool>(
			make_shared<CNetworkPlayerCommandSink>(), m_pBalanceTool.get());
	if (nullptr == m_pAnimationTool)
		m_pAnimationTool = make_unique<CAnimation_Tool>(
			m_pCharacterPreviewPanel, m_pBalanceTool.get(), m_pValtanBossTool.get(),
			m_pDevice, m_pContext);
	return S_OK;
}

#endif

HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool, const bool_t bShowWindow)
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Open");
#ifdef _DEBUG
    // 도구 callback은 MainApp의 서비스에 요청을 되돌린다. 도구를 보유한 MainApp에 연결하므로 지역 변수 대신 소유자인 this를 사용한다.
    const CLASS_MOVIE_INSPECTION_CALLBACKS::OPEN_BACKGROUND openMovieBackground =
        [this](const std::string& classId, const std::string& expectedAreaId, std::string& status) {
            auto* level = CLevel_CharacterSelect::Get_Active();
            if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
            { status = "Background editing requires the active Character Select Movie."; return false; }
            auto& preview = level->Get_ClassSelectionPresentation();
            const std::string currentAreaId = level->Get_ClassCinematicBackgroundArea(classId);
            if (!preview.Is_Active() || preview.Get_ActiveClass() != classId || level->Get_ClassMovieId() != classId ||
                expectedAreaId.empty() || currentAreaId != expectedAreaId ||
                !level->Find_ClassCinematicBackgroundRuntime(expectedAreaId))
            { status = "The Movie or its loaded background changed. Refresh the Movie inspector; the current Map Tool draft was preserved."; return false; }
            if (FAILED(EnsureDebugTool(DEBUG_TOOL::MAP, false)) || !m_pMapTool)
            { status = "Map Tool could not be prepared; the Movie preview was preserved."; return false; }
            if (!m_pMapTool->Open_ClassMovieBackground(expectedAreaId, status)) return false;
            std::string cameraStatus;
            if (!preview.Set_InspectionFreeCamera(true, cameraStatus))
                status += " Free camera handoff failed: " + cameraStatus;
            if (preview.Get_WorldInspection(classId, preview.Is_Looping()).pickArmed)
            {
                CLASS_MOVIE_INSPECTION_COMMAND cancelPick;
                cancelPick.action = CLASS_MOVIE_INSPECTION_ACTION::PICK_IN_SCENE;
                cancelPick.enabled = false;
                (void)preview.Inspect_World(classId, preview.Is_Looping(), cancelPick, cameraStatus);
            }
            SetDebugToolVisible(DEBUG_TOOL::MAP, true);
            m_eDebugInputOwner = DEBUG_TOOL::MAP;
            m_eDebugWindowFocusPending = DEBUG_TOOL::MAP;
            return true;
        };
	/* 이전 enum 값은 내부 호환 분기로 받아 기존 EFFECT 도구 열기에 연결한다. */
	if (DEBUG_TOOL::EFFECT_COMPOSITION == eTool)
		return EnsureDebugTool(DEBUG_TOOL::EFFECT, bShowWindow);
	if (eTool == DEBUG_TOOL::WORLD_OBJECT || eTool == DEBUG_TOOL::SEQUENCER_BENCHMARK)
	{
		if (FAILED(EnsureDebugTool(DEBUG_TOOL::SEQUENCER, bShowWindow))) return E_FAIL;
        if (bShowWindow)
        {
		    m_pSequencerTool->Open(eTool == DEBUG_TOOL::WORLD_OBJECT ?
			    COMPOSITION_WORKBENCH_TARGET::OBJECT : COMPOSITION_WORKBENCH_TARGET::SEQUENCE);
		    m_eDebugInputOwner = eTool;
        }
		return S_OK;
	}
	if (DEBUG_TOOL::VALTAN_ACTION_WORKBENCH == eTool ||
		DEBUG_TOOL::KOUKU_SAYDON_ACTION_WORKBENCH == eTool)
	{
		if (FAILED(EnsureDebugTool(DEBUG_TOOL::SEQUENCER, bShowWindow)))
			return E_FAIL;
        if (bShowWindow)
		    m_pSequencerTool->Open(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH == eTool ?
			    COMPOSITION_WORKBENCH_BOSS::VALTAN : COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON);
		return S_OK;
	}

#endif
	switch (eTool)
	{
	case DEBUG_TOOL::RENDERING:
		if (nullptr == m_RenderingProfiles.Get_ActiveProfile()) return E_FAIL;
#ifdef _DEBUG
		m_bLightResourcesWindowVisible = true;
		m_bLightDetailWindowVisible = true;
		m_bLightSequencerWindowVisible = true;
		m_bRenderQualityDraftInitialized = false;
#endif
		m_bRenderingQualityWindowVisible = true;
		if (nullptr == m_pRenderingBenchmark)
		{
			m_pRenderingBenchmark = make_unique<CRenderingBenchmark>();
			m_pRenderingBenchmark->Set_DeviceInfo(m_pDevice.Get());
		}
		break;
#ifdef _DEBUG
    case DEBUG_TOOL::WORLD_SCENE:
        if (!m_pWorldSceneTool) m_pWorldSceneTool = make_unique<CWorldSceneTool>();
        m_pWorldSceneTool->Open();
        break;
	case DEBUG_TOOL::WORLD_LEVEL:
		if (!m_pWorldLevelTool) m_pWorldLevelTool = make_unique<CWorldLevelTool>();
		m_pWorldLevelTool->Open(GetWorldLevelAreaId());
		break;
	case DEBUG_TOOL::MAHARAKA_AI:
		if (!m_pMaharakaAITool) m_pMaharakaAITool = make_unique<CMaharakaAITool>(make_shared<CNetworkPlayerCommandSink>());
		m_pMaharakaAITool->Set_Active(static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID()) == LEVEL::MAHARAKA &&
			CNetworkManager::Get().Is_Connected());
		m_pMaharakaAITool->Open();
		break;
	case DEBUG_TOOL::GUIDE_AI:
		if (!m_pGuideAITool) m_pGuideAITool = make_unique<CGuideAITool>();
		m_pGuideAITool->Open();
		break;
	case DEBUG_TOOL::MAP:
		if (nullptr == m_pMapTool)
		{
			auto mapTool = make_unique<CMapTool>();
			if (FAILED(mapTool->Initialize(m_pDevice, m_pContext)))
				return E_FAIL;
			m_pMapTool = move(mapTool);
		}
		m_pMapTool->SetOpen(true);
		if (!m_pMapTool->IsOpen()) return E_FAIL;
		break;
	case DEBUG_TOOL::ANIMATION:
		if (FAILED(EnsureAnimationPreviewBackend())) return E_FAIL;
		break;
	case DEBUG_TOOL::EFFECT:
	{
		if (nullptr == m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel =
				make_shared<CCharacterPreviewPanel>(m_pDevice, m_pContext);
		if (nullptr == m_pBalanceTool)
			m_pBalanceTool = make_unique<CBalanceTool>();
		if (nullptr == m_pEffectTool)
			m_pEffectTool =
				make_unique<CEffect_Tool>(
					m_pDevice, m_pContext, m_pCharacterPreviewPanel,
					m_pBalanceTool.get());
		/* Effect 도구가 만들어지기 전에 Save가 끝났을 수 있다. 첫 열기의 정본 새로고침보다
		   먼저 해당 저장 영수증을 넘겨 어떤 저장 결과를 반영할지 고정한다. */
		if (nullptr != m_pValtanActionWorkbench)
		{
			std::string ExpectedValtanSourceRevision;
			if (m_pValtanActionWorkbench->
					Consume_EffectGraphRefreshRequest(
						ExpectedValtanSourceRevision))
			{
				m_pEffectTool->Request_ValtanGraphRefresh(
					ExpectedValtanSourceRevision);
			}
		}
        m_pEffectTool->Configure_AuthoringWorkspace(m_pKoukuPresentationPlayer.get());
                ConfigureClassMovieEditor(*m_pEffectTool, openMovieBackground);
        m_pEffectTool->Set_KoukuPatternPreviewProvider(
            [this](const std::string& effectId, EFFECT_TOOL_KOUKU_PATTERN_PREVIEW& context, std::string& status)
            {
                status.clear();
                CKoukuSaydonActionWorkbench* workbench = nullptr;
                if (m_pSequencerTool && m_pSequencerTool->Get_SelectedTarget() == COMPOSITION_WORKBENCH_TARGET::SEQUENCE)
                    workbench = m_pSequenceActionWorkbench.get();
                else if (m_pSequencerTool && m_pSequencerTool->Is_BossSelected())
                    workbench = m_pKoukuSaydonActionWorkbench.get();
                if (!workbench || !workbench->Has_Composition()) return false;
                return CEffect_Tool::Build_KoukuPatternPreviewContext(workbench->Get_Composition(),
                    workbench->Get_SelectedPatternId(), workbench->Get_SelectedPresentationOccurrenceId(),
                    effectId, context, status);
            });
        break;
	}
    case DEBUG_TOOL::EFFECT_V2:
        if (!m_pCharacterPreviewPanel)
            m_pCharacterPreviewPanel = make_shared<CCharacterPreviewPanel>(m_pDevice, m_pContext);
        if (!m_pEffectToolV2)
            m_pEffectToolV2 = make_unique<CEffect_Tool_V2>(m_pDevice, m_pContext);
        m_pEffectToolV2->Configure_AuthoringWorkspace(m_pCharacterPreviewPanel, m_pKoukuPresentationPlayer.get());
        m_pEffectToolV2->Activate();
        break;
	case DEBUG_TOOL::SEQUENCER:
		if (FAILED(EnsureAnimationPreviewBackend())) return E_FAIL;
		// 발탄·쿠크 편집 세션은 MainApp이 각각 보유하고 선택된 세션만 필요한 문서를 읽는다. 다른 도구에는 세션 접근만 빌려 준다.
		if (nullptr == m_pValtanActionWorkbench)
			m_pValtanActionWorkbench = make_unique<CValtanActionWorkbench>(
				m_pAnimationTool.get(), m_pBalanceTool.get(), m_pValtanBossTool.get());
		if (nullptr == m_pKoukuSaydonActionWorkbench)
		{
			m_pKoukuSaydonActionWorkbench = make_unique<CKoukuSaydonActionWorkbench>();
			m_pWorldObjectCatalogSource = nullptr;
			m_iWorldObjectCatalogGeneration = UINT64_MAX;
			m_pKoukuSaydonActionWorkbench->Set_WorldPlacementResolver(
				[](KOUKU_SAYDON_WORLD_PLACEMENT& placement, std::string& status)
				{
					auto* arena = CLevel_KoukuSaydonArena::Get_Active();
					float3_t position{};
					if (!arena) { status = "WORLD placement requires the KoukuSaydon arena."; return false; }
					if (!arena->Try_Get_AuthoringForwardPlacement(position, status)) return false;
					KOUKU_SAYDON_WORLD_PLACEMENT staged;
					staged.Position = {position.x, position.y, position.z};
					placement = std::move(staged);
					return true;
				});
		}
		m_pKoukuSaydonActionWorkbench->Set_CompleteSequenceAdmission(
            [this](std::string_view gate, std::string& status) { return StartKoukuGateCompletePlay(gate, status); });
		if (!m_pWorldObjectTool) m_pWorldObjectTool = make_unique<CWorldObjectTool>();
		m_pWorldObjectTool->Set_LinkedSaveCallbacks(
			[this](std::string& status) {
				for (auto* editor : {m_pKoukuSaydonActionWorkbench.get(), m_pSequenceActionWorkbench.get()})
				{
					if (!editor) continue;
					const std::string owner = editor == m_pKoukuSaydonActionWorkbench.get() ? "Pattern" : "Sequence";
					if (editor->Is_PublishRunning())
					{ status = owner + " Publish is already running. Wait for it to finish, then retry Object Save. All edits are preserved."; return false; }
					if (editor->Is_CompositionDirty())
					{ status = "Save " + owner + " edits first, including newly created Logic definitions, then retry Object Save. Starting Publish is not required. All edits are preserved."; return false; }
				}
				return true;
			},
			[this](bool publishPatterns, std::string& status) {
				for (auto* editor : {m_pKoukuSaydonActionWorkbench.get(), m_pSequenceActionWorkbench.get()})
				{
					if (!editor) continue;
					const std::string owner = editor == m_pKoukuSaydonActionWorkbench.get() ? "Pattern" : "Sequence";
					if (editor->Is_PublishRunning())
					{ status = "Object is saved; linked reload waits for the running " + owner + " Publish. Retry Object Save after it finishes."; return false; }
					if (editor->Is_CompositionDirty())
					{ status = "Object is saved; new " + owner + " edits are preserved. Save them first, then retry Object Save. Starting Publish is not required."; return false; }
				}
				if (publishPatterns && !m_pKoukuSaydonActionWorkbench)
				{
					m_pKoukuSaydonActionWorkbench = make_unique<CKoukuSaydonActionWorkbench>();
					m_pWorldObjectCatalogSource = nullptr;
					m_iWorldObjectCatalogGeneration = UINT64_MAX;
					m_pKoukuSaydonActionWorkbench->Set_WorldPlacementResolver(
						[](KOUKU_SAYDON_WORLD_PLACEMENT& placement, std::string& status)
						{
							auto* arena = CLevel_KoukuSaydonArena::Get_Active();
							float3_t position{};
							if (!arena) { status = "WORLD placement requires the KoukuSaydon arena."; return false; }
							if (!arena->Try_Get_AuthoringForwardPlacement(position, status)) return false;
							KOUKU_SAYDON_WORLD_PLACEMENT staged;
							staged.Position = {position.x, position.y, position.z};
							placement = std::move(staged);
							return true;
						});
				}
				for (auto* editor : {m_pKoukuSaydonActionWorkbench.get(), m_pSequenceActionWorkbench.get()})
					if (editor && (editor->Has_Composition() || (publishPatterns && editor == m_pKoukuSaydonActionWorkbench.get())))
						{ editor->Notify_WorldAnimationSaved(); if (!editor->Reload(status)) return false; }
				RefreshWorldObjectResources();
				if (publishPatterns)
				{
					if (!m_pKoukuSaydonActionWorkbench->Publish_AllPatterns(status)) return false;
					status = "Map applied; linked Pattern publication started. Its status is shown in Composition.";
				}
				else status = "Map applied and the linked Sequence source reloaded for the next play.";
				return true;
			});
		if (!m_pSequenceActionWorkbench)
		{
			m_pSequenceActionWorkbench = make_unique<CKoukuSaydonActionWorkbench>(true);
			m_pSequenceActionWorkbench->Set_CompleteSequenceAdmission(
				[this](std::string_view gate, std::string& status) { return StartKoukuGateCompletePlay(gate, status); });
			m_pSequenceActionWorkbench->Set_WorldPlacementResolver(
				[](KOUKU_SAYDON_WORLD_PLACEMENT& placement, std::string& status)
				{
					auto* arena = CLevel_KoukuSaydonArena::Get_Active();
					float3_t position{};
					if (!arena) { status = "WORLD placement requires the KoukuSaydon arena."; return false; }
					if (!arena->Try_Get_AuthoringForwardPlacement(position, status)) return false;
					placement = {};
					placement.Position = {position.x, position.y, position.z};
					return true;
				});
			m_pWorldObjectCatalogSource = nullptr;
			m_iWorldObjectCatalogGeneration = UINT64_MAX;
		}
		if (!m_pCharacterActionWorkbench)
		{
			if (!m_pEffectTool)
			{
				m_pEffectTool = make_unique<CEffect_Tool>(m_pDevice, m_pContext,
					m_pCharacterPreviewPanel, m_pBalanceTool.get());
				m_pEffectTool->Configure_AuthoringWorkspace(m_pKoukuPresentationPlayer.get());
                ConfigureClassMovieEditor(*m_pEffectTool, openMovieBackground);
			}
			m_pCharacterActionWorkbench = make_unique<CCharacterActionWorkbench>(m_pCharacterPreviewPanel,
				m_pEffectTool->Create_CompositionSequencer("character.actions"), m_pAnimationTool.get());
			m_pCharacterActionWorkbench->Set_OpenEffectResourceCallback([this](const std::string& id) {
				std::string error;
				if (!CEffectResourceCatalog::Get().Reload_Valtan(error)) { m_strToolStatus = error; return; }
				auto catalog = CEffectResourceCatalog::Get().Get_Snapshot();
				const auto* entry = catalog ? catalog->Find(id) : nullptr;
				if (!entry) { m_strToolStatus = "Effect resource is not in the active catalog: " + id; return; }
				if (entry->Key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT)
				{
					if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT)))
						(void)m_pEffectTool->Open_AuthoringResource(entry->Key);
				}
				else if (entry->Key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V2_LEAF ||
					entry->Key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V2_GROUP)
				{
					if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::EFFECT_V2)))
						(void)m_pEffectToolV2->Open_Resource(entry->Key);
				}
				else m_strToolStatus = "Effect resource has no supported authoring owner: " + id;
			});
		}
		if (nullptr == m_pSequencerTool)
		{
			m_pSequencerTool = make_unique<CSequencerTool>(
				m_pValtanActionWorkbench.get(), m_pKoukuSaydonActionWorkbench.get());
			m_pSequencerTool->Set_ActionSessions(m_pCharacterActionWorkbench.get(),
				m_pWorldObjectTool.get(), m_pSequenceActionWorkbench.get());
			CSequencerTool::CLASS_SELECTION_PREVIEW_CALLBACKS classSelection;
			classSelection.state = [] {
				CSequencerTool::CLASS_SELECTION_PREVIEW_STATE state;
				auto* level = CLevel_CharacterSelect::Get_Active();
				if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
				{ state.status = "Enter Character Select from Lobby to preview this WORLD scene."; return state; }
				const auto& preview = level->Get_ClassSelectionPresentation();
				state.available = level->Can_PlayClassCinematic();
				for (const auto& option : level->Get_ClassMovieOptions())
					state.options.push_back({option.id, option.label, option.classId});
				state.selectedCategory = level->Get_SelectedClassMovieCategory();
				state.selectedClassId = level->Get_ClassMovieId();
				state.activeClassId = preview.Get_ActiveClass();
				state.selectedLabel = level->Get_ClassMovieLabel();
				state.authoringDirty = preview.Has_AuthoringChanges();
                state.authoringPublishPending = preview.Is_AuthoringPublishPending();
                state.authoringStatus = preview.Get_AuthoringStatus();
                state.authoringGeneration = preview.Get_AuthoringGeneration();
                state.canUndoAuthoring = preview.Can_UndoAuthoring();
                state.canRedoAuthoring = preview.Can_RedoAuthoring();
				state.active = preview.Is_Active();
				state.paused = preview.Is_Paused();
				state.looping = preview.Is_Looping();
                state.repeatMovie = preview.Get_RepeatMovie(state.selectedClassId);
                state.completedHold = preview.Is_CompletedHold();
				state.ownerToken = preview.Get_PlaybackToken();
				state.loopCycle = preview.Get_LoopCycle();
				state.clockMs = preview.Get_ClockMs();
                state.sourceClockMs = preview.Get_SourceClockMs();
                state.sourceRate = preview.Get_SourceRate();
                state.playbackRate = preview.Get_PlaybackRate();
                state.cameraSample = preview.Get_CameraSample();
				state.durationMs = preview.Get_DurationMs();
				state.introDurationMs = preview.Get_PhaseDurationMs(state.selectedClassId, false);
				state.loopDurationMs = preview.Get_PhaseDurationMs(state.selectedClassId, true);
				state.activeIntroDurationMs = preview.Get_PhaseDurationMs(state.activeClassId, false);
				state.activeLoopDurationMs = preview.Get_PhaseDurationMs(state.activeClassId, true);
				state.status = level->Get_ClassCinematicStatus();
				if (!level->Can_PlayClassCinematic())
					state.status = "Close customizing or another preview and wait for the arena to finish connecting.";
				return state;
			};
			classSelection.selectCategory = [](std::size_t category) {
				if (auto* level = CLevel_CharacterSelect::Get_Active(); level &&
					CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
					(void)level->Select_ClassMovieCategory(category);
			};
            classSelection.play = [this] {
                auto* level = CLevel_CharacterSelect::Get_Active();
                if (!level || !level->Can_PlayClassCinematic() ||
                    CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT)) return false;
                if (m_pEffectTool && m_pEffectTool->Is_EditingClassMovie(level->Get_ClassMovieId()))
                    return m_pEffectTool->Play_ClassMovie();
                return level->Play_ClassCinematic(level->Get_ClassMovieId());
            };
			classSelection.stop = [] {
				if (auto* level = CLevel_CharacterSelect::Get_Active(); level &&
					CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
					level->Get_ClassSelectionPresentation().Stop();
			};
			classSelection.setPaused = [](bool paused) {
				if (auto* level = CLevel_CharacterSelect::Get_Active(); level &&
					CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
					level->Get_ClassSelectionPresentation().Set_Paused(paused);
			};
			classSelection.seek = [](bool loop, double timeMs) {
				auto* level = CLevel_CharacterSelect::Get_Active();
				return level && level->Can_PlayClassCinematic() &&
					CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT) &&
					level->Get_ClassSelectionPresentation().Seek(
						level->Get_ClassMovieId(), loop, timeMs);
			};
			classSelection.setPlaybackRate = [](double rate) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                return level && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT) &&
                    level->Get_ClassSelectionPresentation().Set_PlaybackRate(rate);
            };
            classSelection.setRepeatMovie = [](const std::string& classId, bool repeat, std::string& status) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
                { status = "Open Character Select to change Movie repeat."; return false; }
                return level->Get_ClassSelectionPresentation().Set_RepeatMovie(classId, repeat, status);
            };
            classSelection.timeline = [](const std::string& classId, bool loop) -> std::shared_ptr<const CLASS_MOVIE_TIMELINE> {
                auto* level = CLevel_CharacterSelect::Get_Active();
                return level && CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT) ?
                    level->Get_ClassSelectionPresentation().Get_Timeline(classId, loop) : nullptr;
            };
            classSelection.beginAuthoring = [](std::string& status) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                return level && level->Get_ClassSelectionPresentation().Begin_Authoring(status);
            };
            classSelection.setHistorySelection = [](const CLASS_MOVIE_HISTORY_SELECTION& selection) {
                if (auto* level = CLevel_CharacterSelect::Get_Active(); level &&
                    CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
                    level->Get_ClassSelectionPresentation().Set_AuthoringHistorySelection(selection);
            };
            classSelection.finishHistory = [] {
                if (auto* level = CLevel_CharacterSelect::Get_Active(); level &&
                    CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
                    level->Get_ClassSelectionPresentation().Finish_AuthoringHistory();
            };
            classSelection.undoAuthoring = [](CLASS_MOVIE_HISTORY_SELECTION& selection, std::string& status) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
                { status = "Enter Character Select before restoring Movie edits."; return false; }
                return level->Get_ClassSelectionPresentation().Undo_Authoring(selection, status);
            };
            classSelection.redoAuthoring = [](CLASS_MOVIE_HISTORY_SELECTION& selection, std::string& status) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
                { status = "Enter Character Select before restoring Movie edits."; return false; }
                return level->Get_ClassSelectionPresentation().Redo_Authoring(selection, status);
            };
            classSelection.editableBox = [](const std::string& classId, bool loop, const std::string& kind,
                const std::string& id, CLASS_MOVIE_AUTHORING_BOX& out, std::string& status) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                return level && level->Get_ClassSelectionPresentation().Get_AuthoringBox(classId, loop, kind, id, out, status);
            };
            classSelection.applyBox = [](const CLASS_MOVIE_AUTHORING_BOX& before, const DATA_JSON_VALUE& value, std::string& status) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                return level && level->Get_ClassSelectionPresentation().Apply_AuthoringBox(before, value, status);
            };
            classSelection.saveAuthoring = [](std::string& status) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                return level && level->Get_ClassSelectionPresentation().Save_Authoring(status, false);
            };
            classSelection.publishAuthoring = [](std::string& status) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                return level && level->Get_ClassSelectionPresentation().Publish_Authoring(status);
            };
            classSelection.mapTime = [](const std::string& classId, bool loop, double timeMs, bool toSource) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                return level ? level->Get_ClassSelectionPresentation().Map_TimelineTime(classId, loop, timeMs, toSource) : -1.;
            };
            classSelection.captureFreeCamera = [](VALTAN_CINEMATIC_CAMERA_POSE& pose, std::string& status) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                if (!level || CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::CHARACTER_SELECT))
                { status = "Open the Character Select movie to capture its free camera."; return false; }
                const auto camera = level->Get_DebugCamera();
                if (!camera || camera->Is_FollowRequested() || camera->Is_PresentationOverrideActive())
                { status = "Switch to Movie Free camera (F6) before capturing a key."; return false; }
                VALTAN_CINEMATIC_CAMERA_POSE captured;
                if (!CCameraTool::Capture_ViewPose(captured))
                { status = "The current free camera pose is invalid."; return false; }
                pose = captured; status.clear(); return true;
            };
            classSelection.editTiming = [](const CLASS_MOVIE_AUTHORING_BOX& box, double start, double end,
                CLASS_MOVIE_TIMING_EDIT gesture, std::string& status) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                return level && level->Get_ClassSelectionPresentation().Edit_AuthoringTiming(box, start, end, gesture, status);
            };
            classSelection.reloadAuthoring = [](std::string& status) {
                auto* level = CLevel_CharacterSelect::Get_Active();
                return level && level->Get_ClassSelectionPresentation().Reload_Authoring(status);
            };
            classSelection.openEffectEditor = [this](const std::string& classId, bool loop, const std::string& assetId, std::string& status) {
                if (FAILED(EnsureDebugTool(DEBUG_TOOL::EFFECT)) || !m_pEffectTool ||
                    !m_pEffectTool->Open_ClassMovie(classId, loop, assetId))
                { status = "The Movie Effect could not be opened. Inspect V1 Movie status; current unsaved work is preserved."; return false; }
                m_eDebugInputOwner = DEBUG_TOOL::EFFECT;
                status = "Movie Effect opened in V1. Edit Elements with the Movie actors; Play All replays the complete Movie.";
                return true;
            };
            classSelection.inspection = ClassMovieInspectionCallbacks(openMovieBackground);
            m_pSequencerTool->Set_ClassSelectionPreviewCallbacks(std::move(classSelection));
			/* Map Tool의 통합 컷신 화면도 같은 Sequence 세션을 프레임 동안 차용한다.
			   별도 소유자나 문서 복사본을 만들지 않는다. */
			if (m_pMapTool)
			{
				m_pMapTool->Set_SequenceCompositionSession(m_pSequenceActionWorkbench.get());
				/* Valtan Area에는 발탄 Workbench를 연결한다. Sequence 세션은
				   KoukuSaydon composition을 읽으므로 발탄 문서의 편집 세션으로 대체하지 않는다. */
				m_pMapTool->Set_ValtanCompositionSession(m_pValtanActionWorkbench.get());
			}
			if (m_pMapTool)
				m_pMapTool->Set_ObjectCompositionSession(m_pWorldObjectTool.get());
			m_pSequencerTool->Set_TargetChangedCallback([this](COMPOSITION_WORKBENCH_TARGET target) {
				StopCompositionPreview(m_eCompositionPreviewOwner);
				if (m_pValtanActionWorkbench) m_pValtanActionWorkbench->Set_PreviewOwnerActive(false);
				m_eDebugInputOwner = target == COMPOSITION_WORKBENCH_TARGET::OBJECT &&
					!m_pSequencerTool->Uses_ValtanSession() ? DEBUG_TOOL::WORLD_OBJECT :
					(target == COMPOSITION_WORKBENCH_TARGET::SEQUENCE ? DEBUG_TOOL::SEQUENCER_BENCHMARK : DEBUG_TOOL::SEQUENCER);
			});
            if (bShowWindow)
            {
			    if (CGameInstance::Get().Get_CurrentLevelID() == ETOUI(LEVEL::CHARACTER_SELECT))
				    m_pSequencerTool->Open(COMPOSITION_WORKBENCH_TARGET::WORLD);
			    else m_pSequencerTool->Open(ETOUI(LEVEL::KOUKUSAYDON_ARENA) ==
				    CGameInstance::Get().Get_CurrentLevelID() ?
				    COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON : COMPOSITION_WORKBENCH_BOSS::VALTAN);
            }
		}
		else if (bShowWindow)
		{
			const auto level = CGameInstance::Get().Get_CurrentLevelID();
			const bool selectedValtan = m_pSequencerTool->Get_SelectedBoss() == COMPOSITION_WORKBENCH_BOSS::VALTAN;
			if (m_pSequencerTool->Is_BossSelected() && level == ETOUI(LEVEL::CHARACTER_SELECT))
				m_pSequencerTool->Open(COMPOSITION_WORKBENCH_TARGET::WORLD);
			else if (m_pSequencerTool->Is_BossSelected() && level == ETOUI(LEVEL::KOUKUSAYDON_ARENA) && selectedValtan)
				m_pSequencerTool->Open(COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON);
			else if (m_pSequencerTool->Is_BossSelected() && level == ETOUI(LEVEL::VALTAN_ARENA) && !selectedValtan)
				m_pSequencerTool->Open(COMPOSITION_WORKBENCH_BOSS::VALTAN);
			else m_pSequencerTool->Open();
		}
		break;
	case DEBUG_TOOL::UI:
		if (nullptr == m_pHUDLayoutTool)
			m_pHUDLayoutTool =
				make_unique<CHUDLayoutTool>(m_pDevice, m_pContext);
		break;
#endif
	case DEBUG_TOOL::PROFILER:
		if (nullptr == m_pProfilerTool)
			m_pProfilerTool = make_unique<CProfilerTool>(m_pDevice.Get());
		m_pProfilerTool->Open();
		break;
	case DEBUG_TOOL::BALANCE:
		if (nullptr == m_pBalanceTool)
			m_pBalanceTool = make_unique<CBalanceTool>(false);
		m_pBalanceTool->Open();
		break;
	case DEBUG_TOOL::VALTAN_BOSS:
		if (nullptr == m_pBalanceTool)
			m_pBalanceTool = make_unique<CBalanceTool>();
		(void)m_pBalanceTool->Ensure_Initialized();
		if (nullptr == m_pValtanBossTool)
			m_pValtanBossTool = make_unique<CValtanBossTool>(
				make_shared<CNetworkPlayerCommandSink>(),
				m_pBalanceTool.get());
		m_pValtanBossTool->Open();
		break;
	case DEBUG_TOOL::KOUKU_SAYDON_BOSS:
		if (nullptr == m_pKoukuSaydonBossTool)
			m_pKoukuSaydonBossTool = make_unique<CKoukuSaydonBossTool>();
		m_pKoukuSaydonBossTool->Set_CompletePlayAdmission(
		    [this](std::string_view gate, std::string& status) { return StartKoukuGateCompletePlay(gate, status); });
		m_pKoukuSaydonBossTool->Open();
		break;
#ifdef _DEBUG
	case DEBUG_TOOL::VALTAN_LOGIC_PATTERN:
		if (nullptr == m_pBalanceTool)
			m_pBalanceTool = make_unique<CBalanceTool>();
		(void)m_pBalanceTool->Ensure_Initialized();
		if (nullptr == m_pValtanBossTool)
			m_pValtanBossTool = make_unique<CValtanBossTool>(
				make_shared<CNetworkPlayerCommandSink>(),
				m_pBalanceTool.get());
		m_pValtanBossTool->Open_LogicPattern();
		break;
	case DEBUG_TOOL::CAMERA:
		if (nullptr == m_pCameraTool)
			m_pCameraTool = make_unique<CCameraTool>();
		m_pCameraTool->Open();
		break;
	case DEBUG_TOOL::EQUIPMENT:
		if (nullptr == m_pCharacterPreviewPanel)
			m_pCharacterPreviewPanel =
				make_shared<CCharacterPreviewPanel>(m_pDevice, m_pContext);
		if (nullptr == m_pEquipmentAuthoringTool)
			m_pEquipmentAuthoringTool =
				make_unique<CEquipmentAuthoringTool>(
					m_pDevice, m_pContext, m_pCharacterPreviewPanel);
		break;
#endif
	default:
		return E_INVALIDARG;
	}

    // 내장 조절 UI는 세션만 준비하고 창 표시나 미리보기·입력 포커스를 옮기지 않을 수 있다.
    // 명시적인 열기 호출은 기본 인자인 true를 사용해 아래 표시·포커스 처리를 계속한다.
    if (!bShowWindow) return S_OK;
	SetDebugToolVisible(eTool, true);
	/* Valtan Logic Pattern은 Server 상태를 읽는 청사진이다. 열기·포커스 요청만으로
	현재 월드·미리보기 입력 소유자를 바꾸지 않는다. */
	if (DEBUG_TOOL::VALTAN_LOGIC_PATTERN != eTool)
		m_eDebugInputOwner = eTool;
	m_eDebugWindowFocusPending = eTool;
	return S_OK;
}

bool_t CMainApp::RequestDebugLevelNavigation(const LEVEL eTargetLevel)
{
	const LEVEL currentLevel = static_cast<LEVEL>(
		CGameInstance::Get().Get_CurrentLevelID());
	if (LEVEL::END == eTargetLevel || LEVEL::LOADING == currentLevel)
	{
		m_strDebugLevelNavigationStatus =
			"Level navigation is unavailable while the current Level is loading or the target is invalid.";
		return false;
	}
	if (eTargetLevel == currentLevel)
	{
		m_strDebugLevelNavigationStatus =
			"The selected Level is already current.";
		return false;
	}
	if (LEVEL::END != m_eDebugLevelNavigationTarget ||
		CLevelTransitionService::Is_Pending())
	{
		m_strDebugLevelNavigationStatus =
			"Another Server entry or Level transition is already pending.";
		return false;
	}

	auto routeThroughLobby = [this, currentLevel](
		const LOBBY_STAGE stage,
		const LEVEL finalTarget,
		const char_t* targetName) -> bool_t
	{
		bool_t accepted = false;
		if (LEVEL::LOBBY == currentLevel)
		{
			accepted = CLevel_Lobby::Submit_ProductCommand(stage);
			if (!accepted)
			{
				m_strDebugLevelNavigationStatus =
					CLevel_Lobby::Get_ProductStatus();
				return false;
			}
		}
		else if (LEVEL::CHARACTER_SELECT == currentLevel)
		{
			CLevel_CharacterSelect* pCharacterSelect =
				CLevel_CharacterSelect::Get_Active();
			accepted = nullptr != pCharacterSelect &&
				pCharacterSelect->Submit_StageEntry(stage);
			if (!accepted)
			{
				m_strDebugLevelNavigationStatus = nullptr != pCharacterSelect ?
					pCharacterSelect->Get_NavigationStatus() :
					"The active Character Select owner is unavailable.";
				return false;
			}
		}
		else
		{
			LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
			if (!CLobbyCommandService::Request(stage, token))
			{
				m_strDebugLevelNavigationStatus =
					CLobbyCommandService::Get_Status();
				return false;
			}
			if (!CLevelTransitionService::Request_Load(
				LEVEL::LOBBY,
				"f1.level-navigation.route-via-lobby",
				token))
			{
				CLobbyCommandService::Cancel(
					token, "F1 Level Navigation load was rejected");
				m_strDebugLevelNavigationStatus =
					CLevelTransitionService::Get_Status();
				return false;
			}
			accepted = true;
		}

		m_eDebugLevelNavigationTarget = finalTarget;
		m_DebugLevelNavigationDeadline =
			std::chrono::steady_clock::now() + std::chrono::seconds(15);
		m_bDebugLevelNavigationDeadlineActive = true;
		m_strDebugLevelNavigationStatus =
			string("Server-approved route staged for ") + targetName +
			". Lobby owns connection and admission; F1 does not send a packet directly.";
		return accepted;
	};

	if (LEVEL::LOBBY == eTargetLevel)
	{
		if (!CLevelTransitionService::Request_Load(
			LEVEL::LOBBY, "f1.level-navigation.return-lobby"))
		{
			m_strDebugLevelNavigationStatus =
				CLevelTransitionService::Get_Status();
			return false;
		}
		m_eDebugLevelNavigationTarget = LEVEL::LOBBY;
		m_DebugLevelNavigationDeadline =
			std::chrono::steady_clock::now() + std::chrono::seconds(15);
		m_bDebugLevelNavigationDeadlineActive = true;
		m_strDebugLevelNavigationStatus =
			"Typed Lobby return staged. Gameplay Areas remain Server-authoritative and no destination admission was fabricated.";
		return true;
	}
	if (LEVEL::CHARACTER_SELECT == eTargetLevel)
		return routeThroughLobby(
			LOBBY_STAGE::CHARACTER_SELECT, eTargetLevel, "Character Select");
	if (LEVEL::BERN == eTargetLevel)
		return routeThroughLobby(LOBBY_STAGE::BERN, eTargetLevel, "Bern");
	if (LEVEL::VALTAN_ARENA == eTargetLevel)
		return routeThroughLobby(LOBBY_STAGE::VALTAN, eTargetLevel, "Valtan");
	if (LEVEL::KOUKUSAYDON_ARENA == eTargetLevel)
		return routeThroughLobby(LOBBY_STAGE::KOUKU_SAYDON, eTargetLevel, "KoukuSaydon");
	if (LEVEL::COLOSSEUM == eTargetLevel)
		return routeThroughLobby(LOBBY_STAGE::COLOSSEUM, eTargetLevel, "Entrance PvP Arena");
	if (LEVEL::MAHARAKA == eTargetLevel)
		return routeThroughLobby(LOBBY_STAGE::MAHARAKA, eTargetLevel, "Maharaka");

	m_strDebugLevelNavigationStatus =
		"The selected destination is not available in F1 Level Navigation.";
	return false;
}

#ifdef _DEBUG
void CMainApp::RenderArenaCameraAndPlayerControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.CameraAndPlayer");
	const LEVEL level = static_cast<LEVEL>(CGameInstance::Get().Get_CurrentLevelID());
	CLevel_ValtanArena* valtan = LEVEL::VALTAN_ARENA == level ?
		CLevel_ValtanArena::Get_Active() : nullptr;
	CLevel_KoukuSaydonArena* kouku = LEVEL::KOUKUSAYDON_ARENA == level ?
		CLevel_KoukuSaydonArena::Get_Active() : nullptr;
	CLevel_CharacterSelect* characterSelect = LEVEL::CHARACTER_SELECT == level ?
		CLevel_CharacterSelect::Get_Active() : nullptr;
	CLevel_Bern* bern = LEVEL::BERN == level ? CLevel_Bern::Get_Active() : nullptr;
	CLevel_Development* development = LEVEL::DEVELOPMENT == level || LEVEL::MAHARAKA == level ||
		LEVEL::COLOSSEUM == level ?
		CLevel_Development::Get_Active(level) : nullptr;
	shared_ptr<CCamera_Free> camera;
	CPlayerController* controller = nullptr;
	const char* mapName = "Unavailable";
	if (nullptr != valtan)
	{
		camera = valtan->Get_DebugCamera();
		controller = &valtan->Get_DebugPlayerController();
		mapName = "Valtan";
	}
	else if (nullptr != kouku)
	{
		camera = kouku->Get_DebugCamera();
		controller = &kouku->Get_DebugPlayerController();
		mapName = "KoukuSaydon";
	}
	else if (nullptr != characterSelect)
	{
		camera = characterSelect->Get_DebugCamera();
		mapName = "Character Select";
	}
	else if (nullptr != bern)
	{
		camera = bern->Get_DebugCamera();
		controller = &bern->Get_PlayerController();
		mapName = "Bern";
	}
	else if (nullptr != development)
	{
		camera = development->Get_DebugCamera();
		mapName = LEVEL::MAHARAKA == level ? "Maharaka" :
			LEVEL::COLOSSEUM == level ? "Colosseum" : "Development / Training";
	}
	if (camera && ImGui::CollapsingHeader("Map Camera / Player", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Current map: %s", mapName);
		/* 실제 로컬 플레이어 위치와 Server가 제시한 상호작용 영역을 표시해
		저작 트리거를 배치할 때 현재 좌표를 확인할 수 있게 한다. */
		{
			const std::shared_ptr<CCharacter> pDebugLocal =
				nullptr != valtan ? valtan->Get_LocalCharacter() :
				(nullptr != kouku ? kouku->Get_LocalCharacter() :
					(nullptr != bern ? bern->Get_LocalCharacter() : nullptr));
			if (nullptr != pDebugLocal && nullptr != pDebugLocal->Get_Transform())
			{
				float3_t vPlayer{};
				XMStoreFloat3(&vPlayer,
					pDebugLocal->Get_Transform()->Get_State(STATE::POSITION));
				ImGui::Text("Player world: (%.1f, %.1f, %.1f)",
					vPlayer.x, vPlayer.y, vPlayer.z);
			}
			const std::string& offered =
				CCombatHUDViewModel::Get().Get_InteractPromptTriggerId();
			ImGui::Text("Interact offer: %s", offered.empty() ? "(none)" : offered.c_str());
		}
#ifdef _DEBUG
		// 마하라카 워터팡 물총의 로컬 팔 덮어쓰기와 발사 clip·이펙트를 시험한다.
		if (nullptr != development && LEVEL::MAHARAKA == level)
			if (const std::shared_ptr<CCharacter> pWaterGunLocal = development->Get_DebugLocalCharacter())
			{
				bool forced = pWaterGunLocal->Is_WaterGunPreviewForced();
				if (ImGui::Checkbox("Water Gun preview arm", &forced))
					pWaterGunLocal->Set_WaterGunPreviewForced(forced);
				for (uint32_t attack = 1u; attack <= 6u; ++attack)
				{
					const std::string label = "Fire " + std::to_string(attack) + "##watergun";
					if (attack > 1u) ImGui::SameLine();
					if (ImGui::Button(label.c_str()))
						(void)pWaterGunLocal->Play_WaterGunAttackPreview(attack);
				}
			}
			/* 워터팡 패턴 버튼은 방 전체의 강제 이벤트를 Debug Server에 요청한다.
			   적중·알림·예고·공격·소리의 진행 시점은 Server가 소유한다. */
			if (nullptr != development && LEVEL::MAHARAKA == level)
			{
				static std::uint32_t s_WaterpangRequest = 0u, s_WaterpangAwaiting = 0u;
				static std::string s_WaterpangStatus;
				const std::shared_ptr<IPlayerCommandSink> waterpangSink = development->Get_DebugCommandSink();
				const auto requestWaterpang = [&](const char* instanceId, const char* label)
				{
					LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK message{};
					if (0u == ++s_WaterpangRequest) s_WaterpangRequest = 1u;
					message.iRequestSequence = s_WaterpangRequest;
					message.eWorldId = LostArk::Shared::WORLD_ID::MAHARAKA;
					message.eOperation = LostArk::Shared::DEBUG_WORLD_PLAYBACK_OPERATION::PLAY_SEQUENCE;
					message.strTargetId = instanceId;
					const bool sent = waterpangSink && waterpangSink->Request_DebugWorldPlayback(message);
					s_WaterpangAwaiting = sent ? message.iRequestSequence : 0u;
					s_WaterpangStatus = std::string(label) + (sent ? ": sent" : ": not sent (no Server connection)");
				};
				if (ImGui::Button("물벼락##waterpangdebug"))
					requestWaterpang(LostArk::Shared::MAHARAKA_WATERPANG_DEBUG_WATERFALL_INSTANCE, "Waterfall");
				ImGui::SameLine();
				if (ImGui::Button("워터캐논##waterpangdebug"))
					requestWaterpang(LostArk::Shared::MAHARAKA_WATERPANG_DEBUG_CANNON_INSTANCE, "Water cannon");
				LostArk::Shared::S2C_DEBUG_WORLD_PLAYBACK_RESULT waterpangResult{};
				while (waterpangSink && waterpangSink->Consume_DebugWorldPlaybackResult(waterpangResult))
				{
					if (waterpangResult.iRequestSequence != s_WaterpangAwaiting) continue;
					s_WaterpangAwaiting = 0u;
					static const char* const RESULTS[] = { "accepted", "disabled (Release Server)", "wrong world",
						"invalid target", "invalid player", "already used", "rejected (countdown/intro running)",
						"stale request", "skipped player" };
					const auto index = static_cast<size_t>(waterpangResult.eResult);
					s_WaterpangStatus = waterpangResult.strTargetId + ": " +
						(index < std::size(RESULTS) ? RESULTS[index] : "unknown");
				}
				if (!s_WaterpangStatus.empty())
				{
					ImGui::SameLine();
					ImGui::TextUnformatted(s_WaterpangStatus.c_str());
				}
			}
#endif

		// 이 경로는 현재 월드의 저작 navigation으로 투영할 이동 요청을 Server에 제출한다.
		// Bern의 분리된 Bern3 갑판 위치도 같은 Debug 전용 이동 경로로 확인한다.
		if (controller)
		{
			const bool_t freeCamera = !camera->Is_FollowRequested() &&
				!camera->Is_PresentationOverrideActive();
			ImGui::BeginDisabled(!freeCamera || controller->Is_DebugPlayerPlacementPending() ||
				controller->Is_DebugPlayerPlacementArmed());
			if (ImGui::Button("Move Player"))
			{
				const auto world = nullptr != valtan ? LostArk::Shared::WORLD_ID::VALTAN_ARENA :
					(nullptr != bern ? LostArk::Shared::WORLD_ID::BERN :
						LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA);
				if (controller->Begin_DebugPlayerPlacement(world))
					camera->Set_MouseLookEnabled(false);
			}
			ImGui::EndDisabled();
			if (controller->Is_DebugPlayerPlacementArmed())
			{
				ImGui::SameLine();
				if (ImGui::Button("Cancel Pick"))
					controller->Cancel_DebugPlayerPlacement();
			}
			ImGui::TextDisabled("F6 free camera -> Move Player -> click ground. Esc / right-click cancels.");
			ImGui::TextDisabled("Tab toggles mouse-look. Only your player moves after Server approval.");
			if (!controller->Get_DebugPlayerPlacementStatus().empty())
				ImGui::TextWrapped("%s", controller->Get_DebugPlayerPlacementStatus().c_str());
		}
		else
			ImGui::TextDisabled("F6 toggles follow / free camera when a player is available. Tab toggles mouse-look.");
	}
	if (m_pLevelNavigationDebug) m_pLevelNavigationDebug->Render_Controls();
	if (nullptr != characterSelect)
		RenderCharacterSelectFloorSwapControls();
	RenderBalanceTestLauncher();
	if (nullptr != kouku)
		RenderKoukuUiPreviewControls();
}

void CMainApp::RenderCharacterSelectFloorSwapControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.CharacterSelectFloorSwap");
	if (!ImGui::CollapsingHeader("Character Select Floor Swap", ImGuiTreeNodeFlags_DefaultOpen))
		return;
	CLevel_CharacterSelect* characterSelect = CLevel_CharacterSelect::Get_Active();
	if (nullptr == characterSelect)
	{
		ImGui::TextDisabled("Enter Character Select to change its center floor.");
		return;
	}
	ImGui::PushID("CharacterSelectFloorSwap");
	if (ImGui::Button("Reload floor options"))
		(void)characterSelect->Debug_ReloadFloorSwapOptions();
	const auto& options = characterSelect->Debug_GetFloorSwapOptions();
	const auto findOption = [&options](const std::string& id)
	{
		return std::find_if(options.begin(), options.end(), [&id](const auto& option)
			{ return option.id == id; });
	};
	if (m_strCharacterSelectFloorDraftId.empty() && !options.empty())
		m_strCharacterSelectFloorDraftId = options.front().id;
	const auto selected = findOption(m_strCharacterSelectFloorDraftId);
	const char* preview = selected != options.end() ? selected->label.c_str() : "Choose a floor";
	if (ImGui::BeginCombo("Floor", preview))
	{
		for (const auto& option : options)
		{
			ImGui::PushID(option.id.c_str());
			const bool isSelected = option.id == m_strCharacterSelectFloorDraftId;
			if (ImGui::Selectable(option.label.c_str(), isSelected))
				m_strCharacterSelectFloorDraftId = option.id;
			if (isSelected) ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	if (options.empty())
		ImGui::TextDisabled("Reload floor options to load the available presets.");
	ImGui::DragFloat3("Additional offset XYZ (m)", &m_vCharacterSelectFloorOffsetMeters.x,
		0.01f, -50.f, 50.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Additional yaw (deg)", &m_fCharacterSelectFloorYawDegrees,
		0.25f, -180.f, 180.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	constexpr const char* environmentNames[] = { "Source stage environment", "Center floor environment" };
	ImGui::Combo("Environment", &m_iCharacterSelectFloorEnvironment,
		environmentNames, IM_ARRAYSIZE(environmentNames));
	ImGui::Checkbox("Compare shader default brightness (1)", &m_bCharacterSelectFloorShaderDefaultBrightness);
	ImGui::TextDisabled("Off keeps authored brightness. On sets floor and star brightness to 1; use Apply floor.");
	ImGui::TextWrapped("The preset keeps the floor at its original layer and raises only the star to clear the bridge.");
	ImGui::TextDisabled("Additional XYZ offset moves both pieces from their preset positions.");
	ImGui::BeginDisabled(findOption(m_strCharacterSelectFloorDraftId) == options.end());
	if (ImGui::Button("Apply floor"))
	{
		CHARACTER_SELECT_FLOOR_SWAP_SETTINGS settings;
		settings.offsetMeters = m_vCharacterSelectFloorOffsetMeters;
		settings.yawDegrees = m_fCharacterSelectFloorYawDegrees;
		settings.environment = static_cast<CHARACTER_SELECT_FLOOR_ENVIRONMENT>(m_iCharacterSelectFloorEnvironment);
		settings.useShaderDefaultBrightness = m_bCharacterSelectFloorShaderDefaultBrightness;
		(void)characterSelect->Debug_ApplyFloorSwap(m_strCharacterSelectFloorDraftId, settings);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Restore original floor"))
		(void)characterSelect->Debug_ResetFloorSwap();
	ImGui::SameLine();
	if (ImGui::Button("Reset adjustments"))
	{
		m_vCharacterSelectFloorOffsetMeters = {};
		m_fCharacterSelectFloorYawDegrees = 0.f;
		m_iCharacterSelectFloorEnvironment = 1;
		m_bCharacterSelectFloorShaderDefaultBrightness = false;
	}
	const auto& appliedId = characterSelect->Debug_GetFloorSwapSelectedId();
	const auto applied = findOption(appliedId);
	ImGui::TextWrapped("Applied floor: %s", appliedId.empty() ? "Original layout" :
		(applied != options.end() ? applied->label.c_str() : appliedId.c_str()));
	if (!appliedId.empty())
	{
		const auto& appliedSettings = characterSelect->Debug_GetFloorSwapSettings();
		const char* appliedEnvironment = "Unknown environment";
		switch (appliedSettings.environment)
		{
		case CHARACTER_SELECT_FLOOR_ENVIRONMENT::SOURCE_STAGE:
			appliedEnvironment = "Source stage environment";
			break;
		case CHARACTER_SELECT_FLOOR_ENVIRONMENT::CENTER_FLOOR:
			appliedEnvironment = "Center floor environment";
			break;
		}
		ImGui::Text("Applied additional offset: (%.3f, %.3f, %.3f) m | yaw: %.2f deg",
			appliedSettings.offsetMeters.x, appliedSettings.offsetMeters.y,
			appliedSettings.offsetMeters.z, appliedSettings.yawDegrees);
		ImGui::Text("Applied environment: %s", appliedEnvironment);
		ImGui::Text("Applied brightness: %s", appliedSettings.useShaderDefaultBrightness ?
			"Shader default 1 (comparison)" : "Authored material setting (star brightness 1)");
	}
	ImGui::TextDisabled("Use Apply floor after changing the draft settings.");
	ImGui::TextWrapped("After updating presets, Reset adjustments then Apply floor.");
	ImGui::TextDisabled("Session only. Leaving this map restores the original layout.");
	ImGui::TextDisabled("Close character customization before applying or restoring a floor.");
	ImGui::TextWrapped("Baked lighting stays with the source floor; it has not been rebaked for the center position.");
	const auto& status = characterSelect->Debug_GetFloorSwapStatus();
	if (!status.empty()) ImGui::TextWrapped("%s", status.c_str());
	ImGui::PopID();
}

void CMainApp::RenderKoukuUiPreviewControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.KoukuUIPreview");
	if (!ImGui::CollapsingHeader("Kouku UI Preview (Debug)", ImGuiTreeNodeFlags_DefaultOpen))
		return;
	CCombatHUDViewModel& viewModel = CCombatHUDViewModel::Get();
	// 아레나 세션 종료의 replication 초기화가 이 덮어쓰기 상태를 해제한다.
	m_bKoukuUiPreview = viewModel.Is_KoukuGimmickPreviewEnabled();
	const KOUKU_HUD_MODE_DEF* pMode = nullptr;
	for (const KOUKU_HUD_MODE_DEF& Mode : m_KoukuHudModes)
	{
		const char* pModeId = KoukuHudModeId(m_KoukuUiPreview.eHudMode);
		if (nullptr != pModeId && Mode.strId == pModeId)
		{
			pMode = &Mode;
			break;
		}
	}
	/* 목록의 k번째 스킬을 k번째 슬롯에 대응시킨다. 춤 모드는 네 항목을 섞어
	활성화할 때마다 QWER에 배정되는 스킬 순서를 바꾼다. */
	const auto assignSlots = [this, &pMode]()
	{
		for (std::int8_t& iIndex : m_KoukuUiPreview.ModeSkillIndexBySlot)
			iIndex = -1;
		for (uint32_t& iTick : m_KoukuUiPreview.CooldownEndTicks)
			iTick = 0u;
		if (nullptr == pMode)
			return;
		const size_t iCount = (std::min)(pMode->Skills.size(), HUD_KOUKU_SLOT_COUNT);
		vector<std::int8_t> Order;
		for (size_t k = 0; k < iCount; ++k)
			Order.push_back(static_cast<std::int8_t>(k));
		if (pMode->bRandomOrder)
		{
			std::mt19937 Rng(static_cast<uint32_t>(
				std::chrono::steady_clock::now().time_since_epoch().count()));
			std::shuffle(Order.begin(), Order.end(), Rng);
		}
		for (size_t k = 0; k < iCount; ++k)
			m_KoukuUiPreview.ModeSkillIndexBySlot[k] = Order[k];
	};

	bool_t bChanged = false;
	if (ImGui::Checkbox("Enable preview##Kouku", &m_bKoukuUiPreview))
		bChanged = true;
	int32_t iGauge = static_cast<int32_t>(m_KoukuUiPreview.iMadnessGauge);
	if (ImGui::SliderInt("Madness gauge##Kouku", &iGauge, 0, 100))
	{
		m_KoukuUiPreview.iMadnessGauge = static_cast<uint32_t>(iGauge);
		bChanged = true;
	}
	RenderHUDBarPositionControls();
	constexpr const char* MODE_LABELS[] =
		{ "None", "Clown", "Mario", "Dance", "Card maze" };
	int32_t iMode = static_cast<int32_t>(m_KoukuUiPreview.eHudMode);
	if (ImGui::Combo("HUD mode##Kouku", &iMode, MODE_LABELS,
		static_cast<int32_t>(sizeof(MODE_LABELS) / sizeof(MODE_LABELS[0]))))
	{
		m_KoukuUiPreview.eHudMode = static_cast<HUD_KOUKU_HUD_MODE>(iMode);
		pMode = nullptr;
		for (const KOUKU_HUD_MODE_DEF& Mode : m_KoukuHudModes)
		{
			const char* pModeId = KoukuHudModeId(m_KoukuUiPreview.eHudMode);
			if (nullptr != pModeId && Mode.strId == pModeId)
			{
				pMode = &Mode;
				break;
			}
		}
		assignSlots();
		bChanged = true;
	}
	if (ImGui::Button("Reroll slot order##Kouku"))
	{
		assignSlots();
		bChanged = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Start 5s cooldowns##Kouku"))
	{
		const uint32_t iTick = viewModel.Get_Player().iServerTick;
		for (size_t i = 0; i < HUD_KOUKU_SLOT_COUNT; ++i)
		{
			if (m_KoukuUiPreview.ModeSkillIndexBySlot[i] < 0)
				continue;
			m_KoukuUiPreview.CooldownDurationTicks[i] = 150u;
			m_KoukuUiPreview.CooldownEndTicks[i] = iTick + 150u;
		}
		bChanged = true;
	}
	/* 쿠크 아레나의 로컬 캐릭터 위에 공포 상태 문구를 시험 표시한다. 이 버튼은
	Server의 공포 적용을 기다리지 않고 Debug_Fire_StatusEffectTextPreview에 표시를 요청하므로
	문구의 움직임을 확인하는 경로이며 실제 전투 상태를 결정하지 않는다. */
	if (ImGui::Button("Fire fear status word##Kouku"))
		viewModel.Debug_Fire_StatusEffectTextPreview();
	ImGui::SameLine();
	ImGui::TextDisabled("KoukuSaydon arena only");
	/* 구석 미니게임 타이머의 로컬 미리보기다. m_pDungeonTimerView가 실제 화면에
	   표시하며 HUD Layout Tool은 문양 배치를 맡는다. 아래 버튼은 CCombatHUDViewModel의
	   preview 타이머만 조절하고 Server의 제한 시간을 변경하지 않는다. */
	ImGui::SeparatorText("Dungeon timer (Debug)");
	{
		HUD_DUNGEON_TIMER_STATE timer = viewModel.Get_DungeonTimer();
		bool_t bRunning = viewModel.Is_DungeonTimerRunning();
		bool_t bTimerChanged = false;
		if (ImGui::Checkbox("Show timer##Kouku", &m_bDungeonTimerPreview))
		{
			timer.isVisible = m_bDungeonTimerPreview;
			timer.fSeconds = m_fDungeonTimerStartSeconds;
			if (!m_bDungeonTimerPreview)
				bRunning = false;
			bTimerChanged = true;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.f);
		if (ImGui::InputFloat("Start s##Kouku", &m_fDungeonTimerStartSeconds, 0.f, 0.f, "%.0f"))
		{
			m_fDungeonTimerStartSeconds = (std::clamp)(m_fDungeonTimerStartSeconds, 0.f, 3599.f);
			timer.fSeconds = m_fDungeonTimerStartSeconds;
			bTimerChanged = true;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(90.f);
		if (ImGui::InputFloat("Warn s##Kouku", &m_fDungeonTimerWarningSeconds, 0.f, 0.f, "%.0f"))
		{
			m_fDungeonTimerWarningSeconds = (std::clamp)(m_fDungeonTimerWarningSeconds, 0.f, 600.f);
			bTimerChanged = true;
		}
		if (ImGui::Button(bRunning ? "Pause##KoukuTimer" : "Run##KoukuTimer"))
		{
			bRunning = !bRunning;
			if (bRunning)
			{
				m_bDungeonTimerPreview = true;
				timer.isVisible = true;
				/* 0초에서 멈춘 타이머는 남은 시간을 설정한 뒤 새로 시작한다. 0초를 그대로
				   재개해 처음부터 경고 색상과 분리 표시가 나오는 것을 피한다. */
				if (timer.fSeconds <= 0.f)
					timer.fSeconds = m_fDungeonTimerStartSeconds;
			}
			bTimerChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset##KoukuTimer"))
		{
			timer.fSeconds = m_fDungeonTimerStartSeconds;
			bRunning = false;
			bTimerChanged = true;
		}
		ImGui::SameLine();
		ImGui::TextDisabled("%02d:%02d", static_cast<int32_t>(timer.fSeconds) / 60,
			static_cast<int32_t>(timer.fSeconds) % 60);
		if (bTimerChanged)
		{
			timer.fWarningSeconds = m_fDungeonTimerWarningSeconds;
			viewModel.Debug_Set_DungeonTimer(timer, bRunning);
		}
	}
	if (bChanged)
	{
		m_KoukuUiPreview.isValid = m_bKoukuUiPreview;
		m_KoukuUiPreview.iMadnessMaximum = 100u;
		viewModel.Debug_Set_KoukuGimmickPreview(m_KoukuUiPreview);
	}
	ImGui::TextDisabled("Preview only (no Server truth). Modes: %zu loaded.", m_KoukuHudModes.size());
}

#endif

void CMainApp::RenderDebugLevelNavigation()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.LevelNavigation");
	auto levelName = [](const LEVEL level) -> const char_t*
	{
		switch (level)
		{
		case LEVEL::LOBBY: return "Lobby";
		case LEVEL::CHARACTER_SELECT: return "Character Select";
		case LEVEL::BERN: return "Bern";
		case LEVEL::VALTAN_ARENA: return "Valtan";
		case LEVEL::KOUKUSAYDON_ARENA: return "KoukuSaydon";
		case LEVEL::COLOSSEUM: return "Entrance PvP Arena";
		case LEVEL::MAHARAKA: return "Maharaka";
		case LEVEL::LOADING: return "Loading";
		case LEVEL::DEVELOPMENT: return "Development";
		default: return "Other";
		}
	};
	const LEVEL currentLevel = static_cast<LEVEL>(
		CGameInstance::Get().Get_CurrentLevelID());
	const bool_t transitionPending =
		CLevelTransitionService::Is_Pending();
	if (LEVEL::END != m_eDebugLevelNavigationTarget &&
		!transitionPending && currentLevel == m_eDebugLevelNavigationTarget)
	{
		m_strDebugLevelNavigationStatus =
			string("Arrived at ") + levelName(currentLevel) +
			" through the typed Level route.";
		m_eDebugLevelNavigationTarget = LEVEL::END;
		m_bDebugLevelNavigationDeadlineActive = false;
	}
	else if (LEVEL::END != m_eDebugLevelNavigationTarget &&
		LEVEL::LOADING != currentLevel &&
		m_bDebugLevelNavigationDeadlineActive &&
		std::chrono::steady_clock::now() >=
			m_DebugLevelNavigationDeadline)
	{
		const string ownerStatus = LEVEL::LOBBY == currentLevel ?
			CLevel_Lobby::Get_ProductStatus() :
			CLevelTransitionService::Get_Status();
		m_strDebugLevelNavigationStatus =
			"F1 route tracking timed out after 15 seconds and was unlocked. The typed owner remains authoritative; retry only after checking: " +
			ownerStatus;
		m_eDebugLevelNavigationTarget = LEVEL::END;
		m_bDebugLevelNavigationDeadlineActive = false;
	}

	ImGui::SeparatorText("Level Navigation");
	ImGui::Text("Current: %s", levelName(currentLevel));
	ImGui::SameLine();
	ImGui::TextDisabled(
		"| Pending: %s",
		LEVEL::END != m_eDebugLevelNavigationTarget ?
			levelName(m_eDebugLevelNavigationTarget) :
			(transitionPending ? "typed transition" : "None"));
	ImGui::TextWrapped("%s", m_strDebugLevelNavigationStatus.c_str());
	if (LEVEL::LOBBY == currentLevel)
		ImGui::TextWrapped("Entry: %s", CLevel_Lobby::Get_ProductStatus().c_str());
	ImGui::TextDisabled("Destinations require Server approval. PvP Arena opens Colosseum directly; matchmaking remains in Bern.");

	constexpr std::array<std::pair<LEVEL, const char_t*>, 7> destinations = {{
		{ LEVEL::LOBBY, "Lobby" },
		{ LEVEL::CHARACTER_SELECT, "Character Select" },
		{ LEVEL::BERN, "Bern" },
		{ LEVEL::VALTAN_ARENA, "Valtan" },
		{ LEVEL::KOUKUSAYDON_ARENA, "KoukuSaydon" },
		{ LEVEL::COLOSSEUM, "Entrance PvP Arena" },
		{ LEVEL::MAHARAKA, "Maharaka" },
	}};
	const float width = ImGui::GetContentRegionAvail().x;
	const int columns = width >= 660.f ? 3 : (width >= 420.f ? 2 : 1);
	if (ImGui::BeginTable("##LevelNavigationDestinations", columns, ImGuiTableFlags_SizingStretchSame))
	{
		for (const auto& [target, label] : destinations)
		{
			ImGui::TableNextColumn();
			const bool_t disable = currentLevel == target ||
				LEVEL::END != m_eDebugLevelNavigationTarget || transitionPending ||
				LEVEL::LOADING == currentLevel ||
				(LEVEL::LOBBY == currentLevel && !CLevel_Lobby::Can_SubmitProductCommand());
			ImGui::BeginDisabled(disable);
			if (ImGui::Button(label, ImVec2(ImGui::GetContentRegionAvail().x, 0.f)))
				(void)RequestDebugLevelNavigation(target);
			ImGui::EndDisabled();
		}
		ImGui::EndTable();
	}
	if (transitionPending)
	{
		ImGui::TextDisabled(
			"Transition owner: %s",
			CLevelTransitionService::Get_Status().c_str());
	}
}

#ifdef _DEBUG
void CMainApp::RefreshDebugResourceFiles()
{
	struct RESOURCE_ROOT
	{
		const char_t* pDomain;
		const char_t* pSource;
		filesystem::path Root;
		const char_t* pStablePrefix;
		DEBUG_TOOL eTool;
	};

	const filesystem::path resourceRoot =
		CRuntimeAssetRoot::Get_ResourceRoot();
	const filesystem::path dataRoot = CProjectDataRoot::Get();
	const filesystem::path publishedDataRoot =
		resourceRoot.parent_path() / L"DataFiles";
	const std::array<RESOURCE_ROOT, 27> roots = {{
		{ "Character / Animation", "Resources", resourceRoot / L"Character",
			"Resources/Character", DEBUG_TOOL::ANIMATION },
		{ "Character / Animation", "Data", dataRoot / L"Animation",
			"Data/Animation", DEBUG_TOOL::ANIMATION },
		{ "Boss / Pattern", "Data", dataRoot / L"Valtan",
			"Data/Valtan", DEBUG_TOOL::VALTAN_ACTION_WORKBENCH },
		{ "Boss / Pattern", "Data", dataRoot / L"Encounters",
			"Data/Encounters", DEBUG_TOOL::VALTAN_BOSS },
		{ "Boss / Pattern", "Data", dataRoot / L"Actors",
			"Data/Actors", DEBUG_TOOL::VALTAN_BOSS },
		{ "Effect Resource", "Resources", resourceRoot / L"Effect",
			"Resources/Effect", DEBUG_TOOL::EFFECT },
		{ "Effect Resource", "Data", dataRoot / L"Effects" / L"Authored",
			"Data/Effects/Authored", DEBUG_TOOL::EFFECT },
		{ "Effect Resource", "Data", dataRoot / L"Effects" / L"Assemblies",
			"Data/Effects/Assemblies", DEBUG_TOOL::EFFECT },
		{ "Effect Resource", "Data", dataRoot / L"Effects" / L"V2",
			"Data/Effects/V2", DEBUG_TOOL::EFFECT_V2 },
		{ "Sound", "Resources", resourceRoot / L"Sound",
			"Resources/Sound", DEBUG_TOOL::ANIMATION },
		{ "Sound", "Data", dataRoot / L"Sound",
			"Data/Sound", DEBUG_TOOL::ANIMATION },
		{ "Map / World / Navigation", "Resources", resourceRoot / L"Map",
			"Resources/Map", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Resources", resourceRoot / L"Deploy",
			"Resources/Deploy", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Data", dataRoot / L"Maps",
			"Data/Maps", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Data", dataRoot / L"Worlds",
			"Data/Worlds", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Data", dataRoot / L"Navigation",
			"Data/Navigation", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Published", publishedDataRoot / L"Map",
			"DataFiles/Map", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Published", publishedDataRoot / L"World",
			"DataFiles/World", DEBUG_TOOL::MAP },
		{ "Map / World / Navigation", "Published", publishedDataRoot / L"Navigation",
			"DataFiles/Navigation", DEBUG_TOOL::MAP },
		{ "UI / Fonts", "Resources", resourceRoot / L"UI",
			"Resources/UI", DEBUG_TOOL::UI },
		{ "UI / Fonts", "Resources", resourceRoot / L"Fonts",
			"Resources/Fonts", DEBUG_TOOL::UI },
		{ "UI / Fonts", "Data", dataRoot / L"UI",
			"Data/UI", DEBUG_TOOL::UI },
		{ "Gameplay / Rendering", "Data", dataRoot / L"Balance",
			"Data/Balance", DEBUG_TOOL::BALANCE },
		{ "Gameplay / Rendering", "Data", dataRoot / L"Items",
			"Data/Items", DEBUG_TOOL::BALANCE },
		{ "Gameplay / Rendering", "Data", dataRoot / L"Rendering",
			"Data/Rendering", DEBUG_TOOL::RENDERING },
		{ "Gameplay / Rendering", "Published", publishedDataRoot / L"Rendering",
			"DataFiles/Rendering", DEBUG_TOOL::RENDERING },
		{ "Gameplay / Rendering", "Data", dataRoot / L"ResourceIntake",
			"Data/ResourceIntake", DEBUG_TOOL::MAP },
	}};

	auto pathToUtf8 = [](const filesystem::path& path)
	{
		const u8string utf8 = path.generic_u8string();
		return string(utf8.begin(), utf8.end());
	};
	auto lowerAscii = [](string value)
	{
		std::transform(value.begin(), value.end(), value.begin(),
			[](const unsigned char character)
			{
				return static_cast<char_t>(std::tolower(character));
			});
		return value;
	};

	m_bDebugResourceScanAttempted = true;
	m_DebugResourceFiles.clear();
	m_iSelectedDebugResourceFile = static_cast<size_t>(-1);
	constexpr size_t MAX_RESOURCE_FILES = 50000u;
	std::error_code error;
	for (const RESOURCE_ROOT& root : roots)
	{
		error.clear();
		if (!filesystem::is_directory(root.Root, error) || error)
			continue;

		filesystem::recursive_directory_iterator iterator(
			root.Root,
			filesystem::directory_options::skip_permission_denied,
			error);
		const filesystem::recursive_directory_iterator end;
		for (; !error && iterator != end; iterator.increment(error))
		{
			if (m_DebugResourceFiles.size() >= MAX_RESOURCE_FILES)
				break;
			if (!iterator->is_regular_file(error) || error)
			{
				error.clear();
				continue;
			}
			const filesystem::path relative =
				iterator->path().lexically_relative(root.Root);
			if (relative.empty())
				continue;

			DEBUG_RESOURCE_FILE file;
			file.strDomain = root.pDomain;
			file.strSource = root.pSource;
			file.strRelativePath = std::string(root.pStablePrefix) + "/" +
				pathToUtf8(relative);
			file.strSearchText = lowerAscii(
				file.strDomain + " " + file.strSource + " " +
				file.strRelativePath);
			file.eTool = root.eTool;
			m_DebugResourceFiles.push_back(std::move(file));
		}
		if (m_DebugResourceFiles.size() >= MAX_RESOURCE_FILES)
			break;
	}

	/* 전용 도메인 루트가 없는 catalog도 찾기 위해 Data를 함께 검색한다. 같은 stable
	   경로가 반복되면 먼저 넣은 구체적 도메인 항목을 남기고 중복만 제거한다. */
	std::stable_sort(
		m_DebugResourceFiles.begin(), m_DebugResourceFiles.end(),
		[](const DEBUG_RESOURCE_FILE& left, const DEBUG_RESOURCE_FILE& right)
		{
			return std::tie(left.strRelativePath, left.strDomain) <
				std::tie(right.strRelativePath, right.strDomain);
		});
	m_DebugResourceFiles.erase(
		std::unique(
			m_DebugResourceFiles.begin(), m_DebugResourceFiles.end(),
			[](const DEBUG_RESOURCE_FILE& left, const DEBUG_RESOURCE_FILE& right)
			{
				return left.strRelativePath == right.strRelativePath;
			}),
		m_DebugResourceFiles.end());

	/* KoukuSaydon은 목록에 보여 줄 가상 묶음이다. LV_LUT_MIDNIGHTC_ED나 MN_RPCT_05
	   같은 원래 경로·Area 식별자는 보존하고 도메인 표시값을 바꾼 사본만 추가한다. */
	const auto startsWith = [](const string& value, const char_t* prefix)
	{
		return 0u == value.rfind(prefix, 0u);
	};
	std::vector<DEBUG_RESOURCE_FILE> koukuSaydonCollection;
	for (const DEBUG_RESOURCE_FILE& file : m_DebugResourceFiles)
	{
		const string& path = file.strRelativePath;
		const bool_t isKoukuSaydonAsset =
			startsWith(path, "Resources/Map/LV_LUT_MIDNIGHTC_ED/") ||
			startsWith(path, "Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/") ||
			startsWith(path, "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/") ||
			startsWith(path, "DataFiles/Map/LV_LUT_MIDNIGHTC_ED") ||
			startsWith(path, "Resources/Effect/KoukuSaydon/") ||
			startsWith(path, "Resources/Effect/LV_LUT_MIDNIGHTC_ED/") ||
			startsWith(path, "Resources/UI/KoukuSaton/") ||
			startsWith(path, "Resources/UI/LV_LUT_MIDNIGHTC_ED/") ||
			startsWith(path, "Resources/Sound/KoukuSaton/") ||
			startsWith(path, "Data/Animation/Reference/KoukuSaydon/") ||
			startsWith(path, "Data/Animation/Authored/KoukuSaydon/") ||
			startsWith(path, "Data/ResourceIntake/LV_LUT_MIDNIGHTC_ED") ||
			startsWith(path, "Resources/Character/KoukuSaton/") ||
			startsWith(path, "Resources/Character/MN_RPCT_00/") ||
			startsWith(path, "Resources/Character/MN_RPCT_05/") ||
			startsWith(path, "Resources/Character/MN_RPCT_06/") ||
			startsWith(path, "Resources/Character/MN_RPCZ_00/") ||
			startsWith(path, "Resources/Character/WP_MN_RPCT_05/") ||
			startsWith(path, "Resources/Character/WP_MN_RPCT_06/");
		if (!isKoukuSaydonAsset)
			continue;
		DEBUG_RESOURCE_FILE collectionFile = file;
		collectionFile.strDomain = "KoukuSaydon";
		collectionFile.strSearchText = lowerAscii(
			collectionFile.strDomain + " " + collectionFile.strSource + " " +
			collectionFile.strRelativePath);
		koukuSaydonCollection.push_back(std::move(collectionFile));
	}
	m_DebugResourceFiles.insert(
		m_DebugResourceFiles.end(),
		std::make_move_iterator(koukuSaydonCollection.begin()),
		std::make_move_iterator(koukuSaydonCollection.end()));
	std::stable_sort(
		m_DebugResourceFiles.begin(), m_DebugResourceFiles.end(),
		[](const DEBUG_RESOURCE_FILE& left, const DEBUG_RESOURCE_FILE& right)
		{
			return std::tie(left.strDomain, left.strSource, left.strRelativePath) <
				std::tie(right.strDomain, right.strSource, right.strRelativePath);
		});

	m_strDebugResourceStatus = "Indexed " +
		std::to_string(m_DebugResourceFiles.size()) +
		" files from the active Resources and Data roots.";
	if (m_DebugResourceFiles.size() >= MAX_RESOURCE_FILES)
		m_strDebugResourceStatus += " Scan stopped at the 50,000-file safety limit.";
}

void CMainApp::OpenDebugResourceFile(const size_t iFile)
{
	if (iFile >= m_DebugResourceFiles.size())
		return;
	const DEBUG_RESOURCE_FILE& file = m_DebugResourceFiles[iFile];
	if (FAILED(EnsureDebugTool(file.eTool)))
	{
		m_strToolStatus = "Resource selected, but its domain tool failed to initialize: " +
			file.strRelativePath;
		return;
	}

	/* Boss/Pattern 행은 레인 편집 Workbench와 Server 명령용 Valtan Boss Tool을 함께
	   사용한다. 각 도구의 기존 역할을 연결하고 여기서 새 재생 객체를 만들지는 않는다. */
	bool_t bValtanWorkspaceOpened = false;
	if ("Boss / Pattern" == file.strDomain)
	{
		(void)EnsureDebugTool(DEBUG_TOOL::VALTAN_ACTION_WORKBENCH);
		if (nullptr != m_pValtanActionWorkbench &&
			std::string::npos != file.strRelativePath.find("Valtan"))
		{
			bValtanWorkspaceOpened =
				m_pValtanActionWorkbench->Open_Valtan();
		}
	}

	/* 쿠크 action/reference 경로에서 구체적인 저작 프로필을 찾아 Workbench에 넘긴다.
	   같은 프로필의 물리 WModel과 action 문서를 연결하기 위한 선택이며, 나머지 리소스는
	   각 도메인의 기존 도구 열기 흐름을 따른다. */
	const bool_t isKoukuSaydonAnimationPath =
		0u == file.strRelativePath.rfind(
			"Data/Animation/Reference/KoukuSaydon/", 0u) ||
		0u == file.strRelativePath.rfind(
			"Data/Animation/Authored/KoukuSaydon/", 0u) ||
		0u == file.strRelativePath.rfind(
			"Resources/Character/KoukuSaton/", 0u);
	const char_t* pKoukuSaydonProfile = nullptr;
	if (isKoukuSaydonAnimationPath)
	{
		constexpr const char_t* profiles[] =
		{
			"MN_RPCT_00", "MN_RPCT_03", "MN_RPCT_05", "MN_RPCT_06",
			"MN_RPCT_07", "MN_RPCZ_00-1", "MN_RPCZ_00"
		};
		for (const char_t* pProfile : profiles)
		{
			if (std::string::npos != file.strRelativePath.find(pProfile))
			{
				pKoukuSaydonProfile = pProfile;
				break;
			}
		}
	}
	bool_t bKoukuSaydonProfileOpened = false;
	if (nullptr != pKoukuSaydonProfile &&
		SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::ANIMATION)) &&
		nullptr != m_pAnimationTool)
	{
		bKoukuSaydonProfileOpened =
			m_pAnimationTool->Open_KoukuSaydonProfile(pKoukuSaydonProfile);
	}
	m_iSelectedDebugResourceFile = iFile;
	if (nullptr != pKoukuSaydonProfile)
	{
		m_strToolStatus = bKoukuSaydonProfileOpened ?
			("Selected " + file.strRelativePath +
				 ". Workbench opened the exact KoukuSaydon profile as Local Extracted Action Preview.") :
			("Selected " + file.strRelativePath +
				 ", but the KoukuSaydon profile preview could not open. Enter Development and preserve any unsaved draft.");
		return;
	}
	m_strToolStatus = bValtanWorkspaceOpened ?
		("Read-only raw path selected: " + file.strRelativePath +
			". Opened the canonical Valtan Pattern Workbench; select a semantic row there to edit Detail.") :
		("Read-only raw path selected: " + file.strRelativePath +
			". Opened its domain owner only; this arbitrary file was not loaded or presented as an editable canonical document.");
}

#ifdef _DEBUG
namespace
{
	bool_t Read_EncoreFile(const std::filesystem::path& path, std::string& text)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
			return false;
		text.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
		return !input.bad();
	}

	/* 교체할 파일을 먼저 임시 파일로 기록·검증한다. 현재 입력 배열은 한 파일이며,
	   기존 바이트와 다르면 중단한다. 복원 실패 때는 복구용 백업을 지우지 않는다. */
	bool_t Commit_EncoreFiles(
		const std::array<std::filesystem::path, 1>& paths,
		const std::array<std::string, 1>& expected,
		const std::array<std::string, 1>& replacements,
		std::string& status)
	{
		status.clear();
		const std::wstring suffix = L".kouku-encore." +
			std::to_wstring(GetCurrentProcessId()) + L"." + std::to_wstring(GetTickCount64());
		std::array<std::filesystem::path, 1> staged, backups;
		std::size_t promoted = 0u;
		auto cleanup = [&]()
		{
			for (const auto& path : staged)
			{
				std::error_code error;
				if (!path.empty()) std::filesystem::remove(path, error);
			}
		};
		for (std::size_t i = 0u; i < paths.size(); ++i)
		{
			std::string current;
			if (!Read_EncoreFile(paths[i], current) || current != expected[i])
			{
				status = "Encore source changed; Reload Baseline before saving. Nothing was written.";
				cleanup();
				return false;
			}
			staged[i] = paths[i]; staged[i] += suffix + L".tmp";
			backups[i] = paths[i]; backups[i] += suffix + L".rollback";
			const HANDLE file = CreateFileW(staged[i].c_str(), GENERIC_WRITE, 0,
				nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
			DWORD written = 0u;
			const bool_t durable = INVALID_HANDLE_VALUE != file &&
				WriteFile(file, replacements[i].data(),
					static_cast<DWORD>(replacements[i].size()), &written, nullptr) &&
				written == replacements[i].size() && FlushFileBuffers(file);
			if (INVALID_HANDLE_VALUE != file) CloseHandle(file);
			std::string verified;
			if (!durable || !Read_EncoreFile(staged[i], verified) || verified != replacements[i])
			{
				status = "Could not stage encore files; original files are unchanged.";
				cleanup();
				return false;
			}
		}
		for (std::size_t i = 0u; i < paths.size(); ++i)
		{
			std::string current;
			if (!Read_EncoreFile(paths[i], current) || current != expected[i])
			{
				status = "Encore source changed during save; reverting committed files.";
				break;
			}
			if (!ReplaceFileW(paths[i].c_str(), staged[i].c_str(), backups[i].c_str(), 0, nullptr, nullptr))
			{
				const DWORD error = GetLastError();
				status = "Encore file replacement failed (" + std::to_string(error) +
					"); reverting committed files.";
				// ReplaceFile은 원본을 백업으로 옮긴 뒤 실패할 수 있다.
				// 기대한 원본인지 확인하고 대상 덮어쓰기 없이 옮겨 동시 작성자의 파일을 보존한다.
				if (ERROR_UNABLE_TO_MOVE_REPLACEMENT_2 == error &&
					(!Read_EncoreFile(backups[i], current) || current != expected[i] ||
					 !MoveFileExW(backups[i].c_str(), paths[i].c_str(), MOVEFILE_WRITE_THROUGH)))
					status += " Recovery copy retained at " + backups[i].string();
				break;
			}
			++promoted;
			// 원자 교체가 남긴 백업은 실제로 교체된 직전 원본이다.
			// 사전 읽기 이후 다른 작성자가 변경했다면 그 원본을 복원 대상으로 남긴다.
			if (!Read_EncoreFile(backups[i], current) || current != expected[i])
			{
				status = "Encore source changed at replacement; reverting committed files.";
				break;
			}
			if (!Read_EncoreFile(paths[i], current) || current != replacements[i])
			{
				status = "Encore write verification failed; reverting committed files.";
				break;
			}
		}
		if (promoted != paths.size() || !status.empty())
		{
			while (promoted > 0u)
			{
				const std::size_t i = --promoted;
				std::string current;
				if (!Read_EncoreFile(paths[i], current) || current != replacements[i] ||
					!MoveFileExW(backups[i].c_str(), paths[i].c_str(),
						MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
				{
					status += " Recovery copy retained at " + backups[i].string();
				}
			}
			cleanup();
			return false;
		}
		for (const auto& path : backups)
		{
			std::error_code error;
			std::filesystem::remove(path, error);
		}
		cleanup();
		return true;
	}

	bool Patch_EncoreNumber(std::string& text, const std::string_view anchor,
		const std::string_view key, const double value)
	{
		const auto row = text.find(anchor);
		const auto field = row == std::string::npos ? row : text.find(key, row);
		if (field == std::string::npos || !std::isfinite(value)) return false;
		auto begin = text.find(':', field + key.size());
		if (begin == std::string::npos) return false;
		begin = text.find_first_not_of(" \t\r\n", begin + 1u);
		const auto end = text.find_first_not_of("0123456789.eE+-", begin);
		if (begin == std::string::npos || begin == end) return false;
		std::ostringstream number; number << std::setprecision(9) << value;
		text.replace(begin, end - begin, number.str());
		return true;
	}

	void Render_KoukuEncoreRotation(CLevel_KoukuSaydonArena& arena)
	{
		static bool loaded = false, preview = false;
		static float baselineYaw = 0.f, targetYaw = 0.f;
		static std::string status;
		constexpr const char* placementId = "boss.koukusaydon.bingo.saydon";
		const auto path = CProjectDataRoot::Resolve(L"Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json");
		const auto npc = arena.Debug_FindArenaBossNpc("BOSS_KOUKUSAYDON_BINGO_SAYDON");
		const auto load = [&]() {
			CWorldGameplayDocument document;
			if (!document.Load(path, "LV_LUT_MIDNIGHTC_ED", status)) return false;
			const auto* row = document.Find(placementId);
			if (!row) { status = "Encore placement is missing."; return false; }
			baselineYaw = targetYaw = row->yawDegrees;
			loaded = true; preview = false;
			if (npc) npc->Set_DebugPresentationYawOffset(0.f);
			status = "Loaded the saved Encore rotation.";
			return true;
		};
		ImGui::SeparatorText("Bingo Encore Rotation");
		if (!loaded) (void)load();
		ImGui::BeginDisabled(!loaded);
		if (ImGui::DragFloat("Encore yaw (degrees)", &targetYaw, .25f, -360.f, 360.f,
			"%.2f", ImGuiSliderFlags_AlwaysClamp)) preview = true;
		if (ImGui::Button("Save Encore Rotation"))
		{
			std::string original;
			CWorldGameplayDocument latest;
			if (Read_EncoreFile(path, original) && latest.Load(path, "LV_LUT_MIDNIGHTC_ED", status))
			{
				const auto* row = latest.Find(placementId);
				if (!row || std::abs(row->yawDegrees - baselineYaw) > .00001f)
					status = "Encore yaw changed on disk. Reload Saved Rotation before saving.";
				else
				{
					std::string replacement = original;
					DATA_JSON_VALUE checked;
					const float wrapped = std::fmod(std::fmod(targetYaw, 360.f) + 360.f, 360.f);
					if (!Patch_EncoreNumber(replacement, "\"" + std::string(placementId) + "\"", "\"yawDegrees\"", wrapped) ||
						!Patch_EncoreNumber(replacement, "\"revision\"", "\"revision\"", latest.Get_Revision() + 1.0) ||
						!CDataJson::Parse(replacement, checked, status))
						status = "Could not validate the Encore rotation patch.";
					else if (Commit_EncoreFiles({path}, {original}, {replacement}, status))
					{
						baselineYaw = targetYaw = wrapped;
						status = "Saved Encore rotation. Publish World Gameplay and reload the Server world to use it for combat.";
					}
				}
			}
			else if (status.empty()) status = "Could not read the latest Encore placement.";
		}
		ImGui::SameLine();
		if (ImGui::Button("Reload Saved Rotation")) (void)load();
		ImGui::SameLine();
		if (ImGui::Button("Reset Rotation Preview"))
		{ preview = false; targetYaw = baselineYaw; if (npc) npc->Set_DebugPresentationYawOffset(0.f); }
		ImGui::EndDisabled();
		if (preview && npc) npc->Set_DebugPresentationYawOffset(targetYaw - npc->Get_DebugUnadjustedYawDegrees());
		ImGui::TextDisabled(npc ? "Live authoring preview; combat keeps the Server rotation." : "Spawn Bingo Saydon to preview rotation.");
		if (!status.empty()) ImGui::TextWrapped("%s", status.c_str());
	}

}
#endif

namespace
{
	/* F1의 일반 몬스터 버튼 설명 자료다. trigger·spawn group·maxAlive는 tooltip에
	   보여 주고, 클릭은 정해진 버튼 명령만 Server에 제출한다. 실제 몬스터 제거·생성은
	   Server 경로가 맡으므로 아래 표시 자료가 로컬 spawn을 수행하지 않는다. */
	struct DEBUG_WAVE_MONSTER_BUTTON final
	{
		LostArk::Shared::WAVE_MONSTER_BUTTON eButton;
		const char* pLabel;
		const char* pTriggerName;
		const char* pSpawnGroupId;
		uint32_t iMaxAlive;
	};

	constexpr DEBUG_WAVE_MONSTER_BUTTON KOUKU_WAVE_MONSTER_BUTTONS[] =
	{
		{ LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_1, "Normal Monster 1##KoukuWave", "Book1_Monsters", "spawn.kouku.book1", 22u },
		{ LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_2, "Normal Monster 2##KoukuWave", "Book2_Monsters", "spawn.kouku.book2", 15u },
	};

	constexpr DEBUG_WAVE_MONSTER_BUTTON VALTAN_WAVE_MONSTER_BUTTONS[] =
	{
		{ LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_1, "Normal Monster 1##ValtanWave", "Stage_1", "spawn.valtan.stage01", 10u },
		{ LostArk::Shared::WAVE_MONSTER_BUTTON::NORMAL_MONSTER_2, "Normal Monster 2##ValtanWave", "Stage_2", "spawn.valtan.stage03", 10u },
	};

	template <size_t COUNT>
	void Render_DebugWaveMonsterButtons(
		CPlayerController& controller,
		const DEBUG_WAVE_MONSTER_BUTTON (&buttons)[COUNT])
	{
		for (size_t iButton = 0; iButton < COUNT; ++iButton)
		{
			const DEBUG_WAVE_MONSTER_BUTTON& button = buttons[iButton];
			if (0 != iButton)
				ImGui::SameLine();
			if (ImGui::Button(button.pLabel, ImVec2(160.f, 0.f)))
				(void)controller.Request_DebugResummonWaveMonsters(button.eButton);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Trigger %s -> spawn group %s (max alive %u)\n"
					"Asks the Server to remove that group's live monsters and summon the wave again at its authored anchors.\n"
					"Debug only: stepping into the trigger no longer raises it.",
					button.pTriggerName, button.pSpawnGroupId, button.iMaxAlive);
			}
		}
	}
}

#endif

void CMainApp::RenderValtanArenaStateControls()
{
	ImGui::SeparatorText("\xEB\xB0\x9C\xED\x83\x84\x20\xEB\xB2\xBD\x20\x2F\x20\xEC\xA7\x80\xED\x98\x95\x20\xEC\x83\x81\xED\x83\x9C");
	if (nullptr == m_pBalanceTool)
		m_pBalanceTool = make_unique<CBalanceTool>();
	if (nullptr == m_pValtanBossTool)
	{
		m_pValtanBossTool = make_unique<CValtanBossTool>(
			make_shared<CNetworkPlayerCommandSink>(),
			m_pBalanceTool.get());
	}

	CValtanBossTool::VALTAN_ARENA_ACTIVE_STATE state{};
	std::string readStatus;
	const bool_t ready = m_pValtanBossTool->Get_ServerArenaActiveState(
		state, readStatus);
	if (m_bServerArenaPresetStatusTracking)
	{
		m_strServerArenaActiveStatus =
			m_pValtanBossTool->Get_ServerArenaPresetStatus();
		m_bServerArenaPresetStatusTracking =
			m_pValtanBossTool->Is_ServerArenaPresetPending();
	}
	const auto actualCheckbox = [](
		const char_t* label, const bool_t actual)
	{
		bool_t value = actual;
		ImGui::BeginDisabled(true);
		ImGui::Checkbox(label, &value);
		ImGui::EndDisabled();
	};
	actualCheckbox(
		"Ordinary walls / debris sources Active##GlobalArena",
		state.bOrdinaryWallsActive);
	actualCheckbox(
		"109 outer ring Active##GlobalArena",
		state.bOuterRingActive);
	actualCheckbox(
		"3 o'clock floor / collision / Nav Active##GlobalArena",
		state.bThreeOClockFloorActive);
	actualCheckbox(
		"9 o'clock floor / collision / Nav Active##GlobalArena",
		state.bNineOClockFloorActive);
	ImGui::TextDisabled(
		"Active boxes are replicated facts. Arena mutations use exact Server presets because the encounter does not admit arbitrary wall/floor combinations.");
	const auto presetButton = [this, ready](
		const char_t* label,
		const LostArk::Shared::VALTAN_ARENA_PRESET preset)
	{
		ImGui::BeginDisabled(!ready || m_pValtanBossTool->Is_ServerArenaPresetPending());
		if (ImGui::SmallButton(label))
		{
			std::string submitStatus;
			const bool_t submitted =
				m_pValtanBossTool->Set_ServerArenaPreset(
					preset, submitStatus);
			m_strServerArenaActiveStatus = std::move(submitStatus);
			if (submitted)
				m_bServerArenaPresetStatusTracking = true;
		}
		ImGui::EndDisabled();
	};
	presetButton("\xEC\xA0\x84\xEC\xB2\xB4\x20\xEB\xB2\xBD\x20\xEB\xB3\xB5\xEC\x9B\x90\x23\x23\x47\x6C\x6F\x62\x61\x6C\x41\x72\x65\x6E\x61\x50\x72\x65\x73\x65\x74",
		LostArk::Shared::VALTAN_ARENA_PRESET::FRESH);
	ImGui::SameLine();
	presetButton("\xEC\x99\xB8\xEA\xB3\xBD\x20\xEB\xB2\xBD\x20\xEC\xA0\x9C\xEA\xB1\xB0\x23\x23\x47\x6C\x6F\x62\x61\x6C\x41\x72\x65\x6E\x61\x50\x72\x65\x73\x65\x74",
		LostArk::Shared::VALTAN_ARENA_PRESET::CIRCLE_WALLS_GONE);
	presetButton("\x33\xEC\x8B\x9C\x20\xEC\xA7\x80\xED\x98\x95\x20\xEB\xB6\x95\xEA\xB4\xB4\x23\x23\x47\x6C\x6F\x62\x61\x6C\x41\x72\x65\x6E\x61\x50\x72\x65\x73\x65\x74",
		LostArk::Shared::VALTAN_ARENA_PRESET::THREE_OCLOCK_BROKEN);
	ImGui::SameLine();
	presetButton("\x39\xEC\x8B\x9C\x20\xEC\xA7\x80\xED\x98\x95\x20\xEB\xB6\x95\xEA\xB4\xB4\x23\x23\x47\x6C\x6F\x62\x61\x6C\x41\x72\x65\x6E\x61\x50\x72\x65\x73\x65\x74",
		LostArk::Shared::VALTAN_ARENA_PRESET::NINE_OCLOCK_BROKEN);
	ImGui::SameLine();
	presetButton("\xEC\xA0\x84\xEC\xB2\xB4\x20\xEB\xB6\x95\xEA\xB4\xB4\x20\x28\x33\xEC\x8B\x9C\x20\x2B\x20\x39\xEC\x8B\x9C\x29\x23\x23\x47\x6C\x6F\x62\x61\x6C\x41\x72\x65\x6E\x61\x50\x72\x65\x73\x65\x74",
		LostArk::Shared::VALTAN_ARENA_PRESET::BOTH_SIDES_BROKEN);
	ImGui::Text(
		"Debris actors %u | active collision %u | active nav regions %u | nav revision %llu",
		state.iDebrisActorCount,
		state.iActiveCollisionCount,
		state.iActiveNavigationRegionCount,
		static_cast<unsigned long long>(state.iNavigationRevision));
	if (!readStatus.empty())
		ImGui::TextDisabled("%s", readStatus.c_str());
	if (!m_strServerArenaActiveStatus.empty())
		ImGui::TextWrapped("%s", m_strServerArenaActiveStatus.c_str());
}

void CMainApp::RenderValtanArenaControls()
{
	/* 해당 아레나 밖에서는 빈 제목이 남지 않도록 패널 전체를 숨긴다. */
	if (ETOUI(LEVEL::VALTAN_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
		return;
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.ValtanArena");
	ImGui::SeparatorText("Valtan Arena");
	CLevel_ValtanArena* pArena = CLevel_ValtanArena::Get_Active();
	if (nullptr == pArena)
	{
		ImGui::TextDisabled("Valtan Arena Level instance is unavailable.");
		return;
	}
    CBalanceTestPanel::Render_KillBossControl();
    auto& controller = pArena->Get_DebugPlayerController();
    ImGui::BeginDisabled(controller.Is_DebugPlayerPlacementPending() || pArena->Is_DebugValtanBossCommandPending());
    if (ImGui::Button("Start Position"))
        (void)controller.Request_DebugTeleportToPosition(LostArk::Shared::WORLD_ID::VALTAN_ARENA, 8.8f, 9.77f, -20.22f);
    ImGui::SameLine();
    if (ImGui::Button("Before Entrance"))
        (void)controller.Request_DebugTeleportToPosition(LostArk::Shared::WORLD_ID::VALTAN_ARENA, 125.9f, 23.0176f, -93.1f);
    ImGui::SameLine();
    if (ImGui::Button("Arena Start"))
        (void)controller.Request_DebugTeleportToPosition(LostArk::Shared::WORLD_ID::VALTAN_ARENA, 147.75f, 23.0176f, -117.25f);
    if (ImGui::Button("Despawn Valtan Boss"))
    {
        std::string status;
        (void)pArena->Debug_DespawnValtanBoss(status);
    }
    ImGui::EndDisabled();
    ImGui::PushID("ValtanArenaState");
    RenderValtanArenaStateControls();
    ImGui::PopID();
    // 바로 앞 StateControls에서 준비한 BossTool에 도끼 편집을 위임한다. 별도 창 Open이나 graph 로드는 하지 않는다.
    m_pValtanBossTool->Render_AxeEditor();
    if (!pArena->Get_DebugValtanBossCommandStatus().empty())
        ImGui::TextWrapped("%s", pArena->Get_DebugValtanBossCommandStatus().c_str());
    if (ImGui::TreeNode("Valtan Presentation Status"))
    {
        ImGui::TextWrapped("%s", pArena->Get_DebugValtanPresentationDiagnostic().c_str());
        ImGui::TextWrapped("%s", pArena->Get_SourceCinematicPreparationStatus().c_str());
        ImGui::TreePop();
    }
#ifdef _DEBUG
	ImGui::SeparatorText("Wave Monsters");
	ImGui::TextDisabled(
		"Debug builds no longer raise the Stage_1 / Stage_2 corridor waves when you step into their trigger; these buttons summon them again.");
	Render_DebugWaveMonsterButtons(pArena->Get_DebugPlayerController(), VALTAN_WAVE_MONSTER_BUTTONS);
#endif
}

void CMainApp::RenderKoukuSaydonArenaControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.KoukuArena");
	if (!ImGui::CollapsingHeader("KoukuSaydon Arena", ImGuiTreeNodeFlags_DefaultOpen))
		return;
	ImGui::TextDisabled(
		"Each gate asks the Server to raise its disabled boss placements, moves only your player, and points the boss HUD at that gate's boss.");
	ImGui::TextDisabled(
		"Spawned bosses wait idle. The Boss Tool (Play Isolated / Start Full Pattern) and Complete Play target the raised gate boss; a pattern plays only when the Product lists that boss body.");
	if (ETOUI(LEVEL::KOUKUSAYDON_ARENA) != CGameInstance::Get().Get_CurrentLevelID())
	{
		ImGui::TextDisabled(
			"Enter KoukuSaydon Arena through Lobby Server admission first.");
		return;
	}
	CLevel_KoukuSaydonArena* pArena = CLevel_KoukuSaydonArena::Get_Active();
	if (nullptr == pArena)
	{
		ImGui::TextDisabled("KoukuSaydon Arena Level instance is unavailable.");
		return;
	}
	CBalanceTestPanel::Render_KillBossControl();
#ifdef _DEBUG
	/* 레이드 클리어 MVP의 배치와 등장 시간을 확인하는 Debug 표본 화면이다.
	아래 버튼은 샘플 표시를 요청하며 실제 기여도 집계나 MVP 판정을 수행하지 않는다. */
	ImGui::SeparatorText("MVP Result Page (Debug)");
	ImGui::TextDisabled(
		"Sample page: three contribution rows, three party columns, the default background.");
	ImGui::TextDisabled(
		"Intro runs 135 frames at the source movie's 40fps (3.375s); the medal strip waits 3.5s (mvp.gfx's own Setting component).");
	{
		const bool_t bMvpVisible = pArena->Debug_Is_MvpResultVisible();
		if (ImGui::Button("Play Dungeon Clear -> MVP"))
			pArena->Debug_Play_ClearThenMvp();
		ImGui::SameLine();
		if (ImGui::Button(bMvpVisible ? "Replay MVP Page" : "Show MVP Page"))
			pArena->Debug_Show_MvpResult();
		ImGui::SameLine();
		if (ImGui::Button("Hide"))
			pArena->Debug_Hide_MvpResult();
	}

	/* 아래 빙고 시험 버튼은 Play1에 0~2번 셀, Play2에 3~4번 셀의 마스크를
	Server로 제출한다. 보드의 흰색·빨간색 판정 결과는 아래 HUD view model에서 읽는다. */
	if (ImGui::TreeNode("Kouku Whirlwind Hammer Transform"))
	{
		if ((m_pKoukuSaydonActionWorkbench || SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::SEQUENCER, false))) && m_pKoukuSaydonActionWorkbench)
			m_pKoukuSaydonActionWorkbench->Render_WorldPlacementTuning("KOUKUSAYDON_G1_PATTERN_24",
				"KOUKUSAYDON_G1_PATTERN_24.world.1");
		ImGui::TreePop();
	}
	ImGui::SeparatorText("Bingo Board");
	ImGui::TextDisabled(
		"Play1 paints cells 0-2, Play2 paints 3-4 and completes row 0. Only rows and columns count.");
	{
		CPlayerController& bingoController = pArena->Get_DebugPlayerController();
		if (ImGui::Button("Bingo_Play1", ImVec2(160.f, 0.f)))
			(void)bingoController.Request_DebugBingoFill(0x7u, false);
		ImGui::SameLine();
		if (ImGui::Button("Bingo_Play2", ImVec2(160.f, 0.f)))
			(void)bingoController.Request_DebugBingoFill(0x18u, false);
		ImGui::SameLine();
		if (ImGui::Button("Bingo_Reset", ImVec2(160.f, 0.f)))
			(void)bingoController.Request_DebugBingoFill(0u, true);
		/* 폭탄 버튼은 Server에 시험 요청만 보낸다. 표식·설치 단계와 위치는
		아래에서 복제된 Bombs를 읽으며 여기서 보드 상태를 직접 고치지 않는다. */
		if (ImGui::Button("Bingo_Bomb", ImVec2(160.f, 0.f)))
			(void)bingoController.Request_DebugBingoBomb();
		ImGui::SameLine();
		/* 망치 버튼은 Server에 시험 요청을 제출한다. 선택된 anchor·단계·tick은
		아래 복제 상태로 확인하며 이 UI에서 이동 경로나 적중을 판정하지 않는다. */
		if (ImGui::Button("Bingo_Hammer", ImVec2(160.f, 0.f)))
			(void)bingoController.Request_DebugBingoHammer();
		if ((m_pWorldObjectTool || SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::WORLD_OBJECT, false))) && m_pWorldObjectTool)
			m_pWorldObjectTool->Render_BingoSizeTuning();
		/* 일반 몬스터 시험 버튼도 같은 플레이어의 typed 요청 경로를 사용한다.
		이 패널은 트리거의 자동 발동 여부를 변경하지 않는다. */
		Render_DebugWaveMonsterButtons(bingoController, KOUKU_WAVE_MONSTER_BUTTONS);
		const auto& board = CCombatHUDViewModel::Get().Get_BingoBoard();
		ImGui::TextDisabled("white 0x%07X   red 0x%07X   bombs %u",
			board.iWhiteMask, board.iRedMask,
			static_cast<std::uint32_t>(board.iBombCount));
		for (std::uint8_t iBomb = 0u; iBomb < board.iBombCount; ++iBomb)
		{
			const auto& bomb = board.Bombs[iBomb];
			ImGui::TextDisabled("  bomb %u  %s  carrier %u  x %.2f  z %.2f",
				static_cast<std::uint32_t>(iBomb),
				LostArk::Shared::BINGO_BOMB_PHASE::MARKED == bomb.ePhase ?
					"MARKED" : "PLANTED",
				static_cast<std::uint32_t>(bomb.iCarrierNetEntityId),
				bomb.fPositionX, bomb.fPositionZ);
		}
		if (LostArk::Shared::BINGO_HAMMER_PHASE::NONE != board.Hammer.ePhase)
		{
			const char* phase =
				LostArk::Shared::BINGO_HAMMER_PHASE::RAISED == board.Hammer.ePhase ? "RAISED" :
				LostArk::Shared::BINGO_HAMMER_PHASE::DESCENDING == board.Hammer.ePhase ? "DESCENDING" :
				"SWEEPING";
			ImGui::TextDisabled("  hammer anchor %d (line %d)  %s  ticks %u..%u",
				board.Hammer.iAnchor, board.Hammer.iAnchor / 2, phase,
				board.Hammer.iPhaseStartTick, board.Hammer.iPhaseEndTick);
		}
	}
	ImGui::Separator();
#endif
	const auto& gates = CLevel_KoukuSaydonArena::Get_DebugGates();
	/* 앞선 관문 전환이나 플레이어 배치 응답이 남아 있으면 새 요청을 막는다.
	   아래 관문 버튼은 처리 대기·현재 활성 관문·지연 사유를 함께 확인해 중복 요청을 피한다. */
	const bool_t placementPending = pArena->Is_DebugGatePending() ||
		pArena->Get_DebugPlayerController().Is_DebugPlayerPlacementPending();
	ImGui::BeginDisabled(placementPending);
	if (ImGui::Button("Return to Start", ImVec2(260.f, 0.f)))
	{
		std::string status;
		(void)pArena->Debug_ReturnToStart(status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("Reset this arena's bosses and entry triggers; return your player to start.");
	ImGui::BeginDisabled(placementPending);
	if (ImGui::Button("3\xea\xb4\x80\xeb\xac\xb8 \xec\x9e\x85\xec\x9e\xa5 \xec\xa0\x84 \xea\xb3\xb5\xea\xb0\x84", ImVec2(260.f, 0.f)))
	{
		(void)pArena->Get_DebugPlayerController().Request_DebugTeleportToPosition(
			LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA,
			-17.509552f, 25.600000f, 960.530156f);
	}
	ImGui::EndDisabled();
	for (size_t iGate = 0u; iGate < gates.size(); ++iGate)
	{
		const CLevel_KoukuSaydonArena::KOUKU_DEBUG_GATE& gate = gates[iGate];
		ImGui::PushID(static_cast<int32_t>(iGate));
		const bool_t deferred = nullptr != gate.pDeferredReason;
		const bool_t active = iGate == pArena->Get_ActiveDebugGate();
		ImGui::BeginDisabled(deferred || active || placementPending);
		if (ImGui::Button(gate.pLabel, ImVec2(260.f, 0.f)))
		{
			std::string status;
			(void)pArena->Debug_ActivateGate(iGate, status);
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (deferred)
		{
			ImGui::TextDisabled("%s", gate.pDeferredReason);
		}
		else if (active)
		{
			ImGui::TextColored(ImVec4(0.3f, 0.85f, 0.45f, 1.f),
				"active | player (%.2f, %.2f, %.2f) | HUD %s",
				gate.vPlayerPosition.x, gate.vPlayerPosition.y, gate.vPlayerPosition.z,
				nullptr != gate.pHudFocusArchetypeId ?
					gate.pHudFocusArchetypeId : "(none)");
		}
		else
		{
			ImGui::TextDisabled("player (%.2f, %.2f, %.2f) | HUD %s",
				gate.vPlayerPosition.x, gate.vPlayerPosition.y, gate.vPlayerPosition.z,
				nullptr != gate.pHudFocusArchetypeId ?
					gate.pHudFocusArchetypeId : "(none)");
		}
#ifdef _DEBUG
		if (gate.pAuditionPlacementId &&
			std::string_view(gate.pAuditionPlacementId) == "boss.koukusaydon.bingo.saydon")
			Render_KoukuEncoreRotation(*pArena);
#endif
		ImGui::PopID();
	}
	ImGui::BeginDisabled(placementPending);
	if (ImGui::SmallButton("Despawn Arena Bosses"))
	{
		std::string status;
		(void)pArena->Debug_DespawnArenaBosses(status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	const std::string& focus = CCombatHUDViewModel::Get().Get_BossFocusArchetype();
	ImGui::TextDisabled("HUD focus: %s",
		focus.empty() ? "(last primary boss)" : focus.c_str());
#ifdef _DEBUG
	ImGui::BeginDisabled(placementPending);
	if (ImGui::SmallButton("Despawn Fire Object"))
	{
		std::string status;
		(void)pArena->Debug_DespawnFireObjects(status);
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Remove the gate-entry outer fire. Activate Gate 3 again to restore it.");
#endif
	ImGui::TextDisabled("Pattern audition target: %s (%s)",
		CKoukuSaydonPatternAuditionService::Get().Get_TargetBossPlacementId().c_str(),
		CKoukuSaydonPatternAuditionService::Get().Get_TargetBossArchetypeId().c_str());
	if (placementPending)
		ImGui::TextDisabled("Gate controls wait for all boss-spawn and player-move replies.");
	ImGui::TextWrapped("%s", pArena->Get_DebugGateStatus().c_str());
	ImGui::TextWrapped("%s",
		pArena->Get_DebugPlayerController().Get_DebugPlayerPlacementStatus().c_str());
#ifdef _DEBUG
	if (ImGui::CollapsingHeader("Mario Controls (Debug Jump)", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextWrapped("Mario 1/2/3/4: auto Clown. Left / Right: move along the fixed course line (release to stop). Camera / mouse cannot steer the player. Up: use an offered crossing, otherwise jump along the same line (up to 4 m / 0.6 s). Down / Shift jump: disabled. F6 free camera keeps Shift acceleration.");
		ImGui::TextWrapped("%s", pArena->Get_DebugPlayerController().Get_DebugMarioJumpStatus().c_str());
		ImGui::TextWrapped("0: return from Mario to Gate 3 through the Server-approved exit.");
		ImGui::TextWrapped("%s", pArena->Get_DebugPlayerController().Get_MarioReturnStatus().c_str());
	}

	// 패널을 접어도 마지막 시선 상태는 활성 아레나가 계속 소유한다.
	if (ImGui::CollapsingHeader("Find the Real Saydon - Sight Collider", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static CKoukuSaydonCompositionDocument gazeDocument;
		static bool gazeLoadAttempted = false;
		static bool gazeLoaded = false;
		static bool gazeVisible = false;
		static float gazeHalfAngle = 45.f, gazeDistance = 30.f;
		static std::string gazeStatus;
		const bool reloadGaze = ImGui::SmallButton("Reload Sight Settings");
		if (!gazeLoadAttempted || reloadGaze)
		{
			// 거부된 문서의 오류는 명시적인 Reload까지 유지한다.
			// 이 렌더 경로에서 전체 composition 읽기를 매 프레임 반복하지 않는다.
			gazeLoadAttempted = true;
			gazeLoaded = gazeDocument.Reload(gazeStatus);
			if (gazeLoaded)
				for (const auto& logic : gazeDocument.Get_LastGood().Logics)
					if (logic.strJudgementKind == "GAZE_REAL_BOSS")
					{ gazeHalfAngle = static_cast<float>(logic.fHalfAngleDegrees); gazeDistance = static_cast<float>(logic.fMaxDistanceM); break; }
		}
		ImGui::Checkbox("Debug Render##RealSaydonSight", &gazeVisible);
		float fullAngle = gazeHalfAngle * 2.f;
		if (ImGui::SliderFloat("View angle (degrees)", &fullAngle, 2.f, 360.f)) gazeHalfAngle = fullAngle * .5f;
		ImGui::SliderFloat("View distance (m)", &gazeDistance, .1f, 100.f);
		pArena->Set_DebugGazeView(gazeVisible, gazeHalfAngle, gazeDistance);
		if (gazeLoaded && ImGui::SmallButton("Save Sight Settings"))
		{
			auto candidate = gazeDocument.Get_LastGood();
			for (auto& logic : candidate.Logics)
				if (logic.strJudgementKind == "GAZE_REAL_BOSS")
				{ logic.fHalfAngleDegrees = gazeHalfAngle; logic.fMaxDistanceM = gazeDistance; }
			if (gazeDocument.Save_Atomic(candidate, gazeStatus))
				gazeStatus = "Saved. Publish Gameplay Balance and restart Server to apply judgement settings.";
		}
		if (!gazeStatus.empty()) ImGui::TextWrapped("%s", gazeStatus.c_str());
	}

	/* 광기 외형은 Server 상태를 snapshot으로 받아 교체한다. 버튼은 의도만
	   제출하고 현재 형태는 복제된 값을 따라 표시한다. */
	if (ImGui::CollapsingHeader("Clown", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const HUD_PLAYER_STATE& hudPlayer = CCombatHUDViewModel::Get().Get_Player();
		const bool_t isClown =
			LostArk::Shared::PLAYER_MADNESS_FORM::CLOWN == hudPlayer.eMadnessForm;
		CPlayerController& controller = pArena->Get_DebugPlayerController();
		int mode = static_cast<int>(hudPlayer.eKoukuHudMode);
		if (ImGui::Combo("HUD mode", &mode, "Player\0Clown\0Mario\0Dance\0Card Maze\0"))
			(void)controller.Request_DebugKoukuHudMode(static_cast<LostArk::Shared::KOUKU_HUD_MODE>(mode));
		ImGui::BeginDisabled(isClown || controller.Is_DebugMadnessFormPending());
		if (ImGui::Button("Change to Clown", ImVec2(160.f, 0.f)))
			(void)controller.Request_DebugMadnessForm(LostArk::Shared::PLAYER_MADNESS_FORM::CLOWN);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!isClown || controller.Is_DebugMadnessFormPending());
		if (ImGui::Button("Return to Player", ImVec2(160.f, 0.f)))
			(void)controller.Request_DebugMadnessForm(LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::TextDisabled("Madness %u / %u | form: %s",
			hudPlayer.iCurrentMadness, hudPlayer.iMaximumMadness,
			isClown ? "clown" : "player");
		if (!controller.Get_DebugMadnessFormStatus().empty())
			ImGui::TextWrapped("%s", controller.Get_DebugMadnessFormStatus().c_str());
	}
#endif
}

bool_t CMainApp::PrepareKoukuGateCompletePlay(const std::string_view gateId, std::string& status)
{
    auto* arena = CLevel_KoukuSaydonArena::Get_Active();
    if (!arena || !arena->Get_PlayerCommandSink() || m_KoukuRaidResourcePreparation ||
        (m_pKoukuSaydonBossTool && m_pKoukuSaydonBossTool->Is_PlayPreparationPending()) ||
        !m_strKoukuCompletePlayFlowGate.empty() || m_iKoukuRaidPendingRequest ||
        CKoukuSaydonPatternAuditionService::Get().Get_Snapshot().Is_InFlight() || CKoukuSaydonPatternAuditionService::Get().Get_FlowSnapshot().bActive)
    { status = "Complete Play requires an idle Kouku arena with a Server connection."; return false; }
    if (gateId != "GATE1" && gateId != "GATE2" && gateId != "GATE3" && gateId != "BINGO")
    { status = "Complete raid playback requires a saved Gate or Bingo flow."; return false; }
    if ((m_pKoukuSaydonActionWorkbench && (m_pKoukuSaydonActionWorkbench->Is_Dirty() || m_pKoukuSaydonActionWorkbench->Is_PublishRunning())) ||
        (m_pSequenceActionWorkbench && (m_pSequenceActionWorkbench->Is_Dirty() || m_pSequenceActionWorkbench->Is_PublishRunning())))
    { status = "Save the Action and Sequence documents and publish before Complete Play."; return false; }
    if (!m_pKoukuSaydonBossTool) m_pKoukuSaydonBossTool = make_unique<CKoukuSaydonBossTool>();
    m_pKoukuSaydonBossTool->Set_CompletePlayAdmission(
        [this](std::string_view gate, std::string& status) { return StartKoukuGateCompletePlay(gate, status); });
    if (!m_pKoukuSaydonBossTool->Reload(status) || !m_pKoukuSaydonBossTool->Validate_PatternFlow(gateId, status)) return false;
    CKoukuSaydonCompositionDocument actions;
    if (!actions.Reload(status)) return false;
    if (actions.Get_LastGood().iRevision != m_pKoukuSaydonBossTool->Get_SourceRevision())
    { status = "Saved Actions differ from the published Product. Publish Saved Patterns, then retry."; return false; }
    return true;
}

bool_t CMainApp::StartKoukuGateCompletePlay(const std::string_view gateId, std::string& status)
{
    using namespace LostArk::Shared;
    if (!PrepareKoukuGateCompletePlay(gateId, status)) return false;
    CKoukuSaydonCompositionDocument sequences(CKoukuSaydonCompositionDocument::Resolve_SequencePath());
    if (!sequences.Reload(status)) return false;
    if (!m_iNextKoukuRaidRequest) { status = "Raid request sequence is exhausted."; return false; }
    auto* arena = CLevel_KoukuSaydonArena::Get_Active();
    C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
    request.iRequestSequence = m_iNextKoukuRaidRequest++; request.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
    request.ExpectedGameplayRevision = CNetworkManager::Get().Get_GameplayRevisionState().ServerActiveRevision;
    request.iActionSourceRevision = m_pKoukuSaydonBossTool->Get_SourceRevision();
    request.iSequenceSourceRevision = sequences.Get_LastGood().iRevision; request.strStartGateId = std::string(gateId);
    KOUKU_RAID_RESOURCE_PREPARATION pending;
    pending.request = request;
    pending.patternIds = m_pKoukuSaydonBossTool->Get_PlayAllPatternIds();
    pending.worldGeneration = CNetworkManager::Get().Get_WorldInboundGeneration();
    pending.deadline = std::chrono::steady_clock::now() + std::chrono::minutes(20);
    arena->Debug_ResetCompletePlayPreparation();
    m_KoukuRaidResourcePreparation = std::move(pending);
    m_strKoukuRaidReplyStatus.clear();
    status = m_strKoukuCompletePlayStatus = "Preparing all raid Effect/Sequence/WORLD dependencies before the Server start request.";
    return true;
}

void CMainApp::FinishKoukuGateCompletePlay(const std::string_view, std::string& status)
{
    status = "The Server owns cinematic completion and the next combat flow.";
}

void CMainApp::CancelKoukuGateCompletePlay(const std::string& status)
{
    using namespace LostArk::Shared;
    const std::string reason = status;
    if (m_KoukuRaidResourcePreparation)
    {
        m_KoukuRaidResourcePreparation.reset();
        if (auto* level = CLevel_KoukuSaydonArena::Get_Active()) level->Debug_ResetCompletePlayPreparation();
        m_strKoukuCompletePlayStatus = "Complete raid preparation cancelled before Server playback.";
        return;
    }
    auto* arena = CLevel_KoukuSaydonArena::Get_Active();
    if (arena && m_iKoukuRaidPendingRequest && m_iKoukuCompletePlayWorldGeneration == CNetworkManager::Get().Get_WorldInboundGeneration())
    {
        m_bKoukuRaidStopAfterAdmission = true;
        m_strKoukuCompletePlayStatus = "Stop is queued until the Server confirms the pending raid request.";
        return;
    }
    if (arena && m_iKoukuCompletePlayWorldGeneration == CNetworkManager::Get().Get_WorldInboundGeneration())
    {
        const auto& state = arena->Get_KoukuRaidState();
        const bool active = state.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING || state.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC || state.ePhase == KOUKUSAYDON_RAID_PHASE::COMBAT ||
            state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE || state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME || state.ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY;
        if (active && state.iOwnerPlayerId == CNetworkManager::Get().Get_LocalPlayerId() && m_iNextKoukuRaidRequest && arena->Get_PlayerCommandSink())
        {
            C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request;
            request.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA; request.iRequestSequence = m_iNextKoukuRaidRequest++;
            request.eOperation = KOUKUSAYDON_RAID_OPERATION::STOP; request.iExpectedRunEpoch = state.iRunEpoch;
            request.ExpectedGameplayRevision = state.PinnedGameplayRevision; request.iActionSourceRevision = state.iActionSourceRevision;
            request.iSequenceSourceRevision = state.iSequenceSourceRevision; request.strStartGateId = state.strGateId;
            if (arena->Get_PlayerCommandSink()->Request_KoukuRaid(request))
            {
                arena->Expect_KoukuRaidReply(request.iRequestSequence);
                m_KoukuRaidRequest = request; m_iKoukuRaidPendingRequest = request.iRequestSequence;
                m_KoukuRaidReplyDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
                m_strKoukuRaidReplyStatus.clear();
                Record_KoukuRaidRequestEvent("kouku.raid.request.sent", request,
                    m_iKoukuCompletePlayWorldGeneration, request.iExpectedRunEpoch, {});
                m_strKoukuCompletePlayStatus = "Waiting for the Server to stop this raid epoch."; return;
            }
        }
        else if (active) { m_strKoukuCompletePlayStatus = "Only the player who started this raid can stop it."; return; }
    }
    if (m_pKoukuPresentationPlayer && m_pKoukuPresentationPlayer->Preview_IsServerClock()) m_pKoukuPresentationPlayer->Stop_Preview();
    if (arena)
    {
        arena->Debug_SetSequenceCombatPending(false); arena->Debug_ReturnToPlayerCamera();
        std::string restore;
        if (!arena->End_ServerRaidCinematicPresentation(true, restore))
        { m_strKoukuCompletePlayStatus = "Previous gate restore failed: " + restore; return; }
    }
    m_strKoukuRaidPresentationKey.clear(); m_strKoukuRaidFailedKey.clear(); m_pKoukuRaidSequenceDocument.reset(); m_iKoukuRaidDocumentEpoch = 0u;
    if (arena) arena->Expect_KoukuRaidReply(0u);
    m_strKoukuCompletePlayFlowGate.clear(); m_iKoukuRaidPendingRequest = 0u; m_bKoukuRaidStopAfterAdmission = false; m_strKoukuCompletePlayStatus = reason;
    m_KoukuRaidRequest = {}; m_KoukuRaidReplyDeadline = {}; m_strKoukuRaidReplyStatus.clear();
}


void CMainApp::RenderKoukuSaydonCompletePlayControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.KoukuCompletePlay");
	if (!ImGui::CollapsingHeader("KoukuSaydon Complete Play (Server Boss Replay)")) return;
	if (!m_pKoukuSaydonBossTool) m_pKoukuSaydonBossTool = make_unique<CKoukuSaydonBossTool>();
	m_pKoukuSaydonBossTool->Set_CompletePlayAdmission(
	    [this](std::string_view gate, std::string& status) { return StartKoukuGateCompletePlay(gate, status); });
	ImGui::TextWrapped("Complete Play runs the selected Gate and its Saved Pattern Flow. Bingo begins its repeating flow after Server preparation.");
	if (ImGui::SmallButton(m_bKoukuCompletePlayLoadAttempted ? "Reload KoukuSaydon Inventory" : "Load KoukuSaydon Inventory"))
	{
		(void)m_pKoukuSaydonBossTool->Reload(m_strKoukuCompletePlayStatus);
		m_bKoukuCompletePlayLoadAttempted = true;
		auto visibility = CClientReplication::Get_GlobalCombatDebugVisibility();
		visibility.bPlayerBodyCollider = true;
		visibility.bBossBodyCollider = true;
		CClientReplication::Set_GlobalCombatDebugVisibility(visibility);
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(m_pKoukuSaydonActionWorkbench &&
		(m_pKoukuSaydonActionWorkbench->Is_Dirty() || m_pKoukuSaydonActionWorkbench->Is_PublishRunning()));
	if (ImGui::SmallButton("Publish Saved Patterns##KoukuCompletePlay"))
		(void)m_pKoukuSaydonBossTool->Request_PublishSavedPatterns(m_strKoukuCompletePlayStatus);
	ImGui::EndDisabled();
	if (!m_bKoukuCompletePlayLoadAttempted) { ImGui::TextDisabled("Load the published Boss Patterns tree."); return; }
	auto bodyVisibility = CClientReplication::Get_GlobalCombatDebugVisibility();
	bool_t showBodyColliders = bodyVisibility.bPlayerBodyCollider && bodyVisibility.bBossBodyCollider;
	if (ImGui::Checkbox("Player / Boss Body Colliders##KoukuCompletePlay", &showBodyColliders))
	{
		bodyVisibility.bPlayerBodyCollider = showBodyColliders;
		bodyVisibility.bBossBodyCollider = showBodyColliders;
		CClientReplication::Set_GlobalCombatDebugVisibility(bodyVisibility);
	}
	ImGui::Checkbox("Bingo Hammer Colliders##KoukuCompletePlay", &m_bKoukuBingoHammerColliders);
	static const char* labels[] = { "\x31\xEA\xB4\x80\xEB\xAC\xB8", "\x32\xEA\xB4\x80\xEB\xAC\xB8", "\x33\xEA\xB4\x80\xEB\xAC\xB8", "\xEB\xB9\x99\xEA\xB3\xA0" };
	static const char* gates[] = { "GATE1", "GATE2", "GATE3", "BINGO" };
	m_iKoukuCompletePlayGate = (std::clamp)(m_iKoukuCompletePlayGate, 0, 3);
	if (ImGui::Combo("Gate##KoukuCompletePlayGate", &m_iKoukuCompletePlayGate, labels, 4))
	{ m_iKoukuCompletePlaySelection = 0; m_strKoukuCompletePlayPatternId.clear(); }
	const std::string gate = gates[m_iKoukuCompletePlayGate];
#ifdef _DEBUG
    if (gate == "BINGO" && ImGui::Button("Open Bingo Parent"))
    {
        if (SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::SEQUENCER)) && m_pSequencerTool && m_pKoukuSaydonActionWorkbench)
        {
            SetDebugToolVisible(DEBUG_TOOL::SEQUENCER, true);
            m_pSequencerTool->Open(COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_ENCORE);
            if (m_pKoukuSaydonActionWorkbench->Has_Composition() ||
                m_pKoukuSaydonActionWorkbench->Reload(m_strKoukuCompletePlayStatus))
                (void)m_pKoukuSaydonActionWorkbench->Select_BingoParent(m_strKoukuCompletePlayStatus);
        }
    }
#endif
	if (ImGui::Combo("Category##KoukuCompletePlay", &m_iKoukuCompletePlayCategory, "Saved Pattern Flow\0All Patterns\0"))
	{ m_iKoukuCompletePlaySelection = 0; m_strKoukuCompletePlayPatternId.clear(); }
	const auto& patterns = m_pKoukuSaydonBossTool->Get_ProductPatterns();
	const auto& bundles = m_pKoukuSaydonBossTool->Get_ProductBundles();
	const auto findPattern = [&](const std::string& id) -> const CKoukuSaydonBossTool::PRODUCT_PATTERN* {
		auto it=std::find_if(patterns.begin(),patterns.end(),[&](const auto& p){return p.strPatternId==id;}); return it==patterns.end()?nullptr:&*it; };
	if (ImGui::BeginChild("KoukuCompletePlayInventory", ImVec2(0,720), true))
		(void)(m_iKoukuCompletePlayCategory == 0 ?
			m_pKoukuSaydonBossTool->Render_SavedPatternFlow(gate, m_iKoukuCompletePlaySelection, m_strKoukuCompletePlayPatternId) :
			m_pKoukuSaydonBossTool->Render_PatternTree(gate, m_iKoukuCompletePlaySelection, m_strKoukuCompletePlayPatternId));
	ImGui::EndChild();
	const auto* pattern=m_iKoukuCompletePlaySelection==3?findPattern(m_strKoukuCompletePlayPatternId):nullptr;
	const auto selectedBundle=std::find_if(bundles.begin(),bundles.end(),[&](const auto& b){return m_iKoukuCompletePlaySelection==2 && b.strBundleId==m_strKoukuCompletePlayPatternId && b.strGateId==gate;});
	const bool bundleSelected=selectedBundle!=bundles.end();
    const auto* savedFlow = gate == "BINGO" ? m_pKoukuSaydonBossTool->Get_SavedFlow(gate) : nullptr;
    const bool bingoParentSelected = m_iKoukuCompletePlaySelection == 4 && savedFlow &&
        m_strKoukuCompletePlayPatternId == savedFlow->strFlowId;
    if (bingoParentSelected)
        ImGui::TextWrapped("Selected: Bingo Complete [Parent]. Plays Common Logic, opening and repeating Patterns, with the saved special Parent.");
	else if (bundleSelected)
	{
		ImGui::Text("Selected bundle: %s",selectedBundle->strDisplayName.c_str());
		for (const auto& member : selectedBundle->Members)
		{ const auto* p=findPattern(member.strPatternId); ImGui::BulletText("%s -> %s | %u ms",member.strTargetBossPlacementId.c_str(),p?p->strDisplayName.c_str():member.strPatternId.c_str(),member.iStartOffsetMs); }
		if (!selectedBundle->strLoadError.empty()) ImGui::TextWrapped("%s",selectedBundle->strLoadError.c_str());
	}
	else if (pattern)
	{
		ImGui::TextWrapped("Selected Pattern: %s | %s",pattern->strDisplayName.c_str(),pattern->strTargetBossPlacementId.c_str());
		if (!pattern->strLoadError.empty()) ImGui::TextWrapped("%s",pattern->strLoadError.c_str());
	}
	else ImGui::TextDisabled(gate == "BINGO" ? "Select Bingo Complete [Parent], a bundle or a child Pattern." :
        "Select a playback bundle or child Pattern. Parent folders are not executable.");
	auto& service=CKoukuSaydonPatternAuditionService::Get(); const auto audition=service.Get_Snapshot();
	const auto flow = service.Get_FlowSnapshot();
	const bool preparing = m_pKoukuSaydonBossTool->Is_PlayPreparationPending();
	const bool arena=ETOUI(LEVEL::KOUKUSAYDON_ARENA)==CGameInstance::Get().Get_CurrentLevelID();
    const auto patternReady = [&](const std::string& id) {
        const auto* row = findPattern(id);
        return row && row->strGateId == gate && row->strLoadError.empty() && !row->Stages.empty();
    };
    const bool bingoParentReady = bingoParentSelected && !savedFlow->Entries.empty() &&
        patternReady(savedFlow->strBingoSpecialPatternId) &&
        std::all_of(savedFlow->Entries.begin(), savedFlow->Entries.end(), [&](const auto& entry) {
            if (entry.strKind == "PATTERN") return patternReady(entry.strTargetId);
            const auto found = std::find_if(bundles.begin(), bundles.end(), [&](const auto& row) {
                return entry.strKind == "BUNDLE" && row.strBundleId == entry.strTargetId && row.strGateId == gate;
            });
            return found != bundles.end() && found->strLoadError.empty() && !found->Members.empty();
        }) &&
        (!m_pKoukuSaydonActionWorkbench || (!m_pKoukuSaydonActionWorkbench->Is_Dirty() && !m_pKoukuSaydonActionWorkbench->Is_PublishRunning())) &&
        (!m_pSequenceActionWorkbench || (!m_pSequenceActionWorkbench->Is_Dirty() && !m_pSequenceActionWorkbench->Is_PublishRunning()));
	const bool ready = bingoParentSelected ? bingoParentReady : bundleSelected ?
        selectedBundle->strLoadError.empty() && !selectedBundle->Members.empty() : pattern && pattern->strGateId == gate && pattern->strLoadError.empty();
	const bool sequencePlaying = m_KoukuRaidResourcePreparation || m_iKoukuRaidPendingRequest || !m_strKoukuCompletePlayFlowGate.empty() ||
		(m_pSequenceActionWorkbench && m_pSequenceActionWorkbench->Is_CompleteSequencePlaying());
	ImGui::BeginDisabled(!arena || !ready || audition.Is_InFlight() || flow.bActive || sequencePlaying || preparing);
	const auto prepareSavedProduct = [&]() {
		if (m_pKoukuSaydonActionWorkbench && m_pKoukuSaydonActionWorkbench->Has_Composition())
		{
			if (m_pKoukuSaydonActionWorkbench->Is_PublishRunning())
			{ m_strKoukuCompletePlayStatus = "Publish All Patterns is still running. Wait for completion before Complete Play."; return false; }
			if (m_pKoukuSaydonActionWorkbench->Is_Dirty())
			{ m_strKoukuCompletePlayStatus = "Save Composition changes and use Publish All Patterns before Complete Play."; return false; }
		}
		if (!m_pKoukuSaydonBossTool->Reload(m_strKoukuCompletePlayStatus)) return false;
		// dirty가 없는 편집기도 외부 publish 이전의 revision을 보유할 수 있다.
		// 열린 편집기의 draft와 선택은 보존하고 저장된 원본을 비교한다.
		CKoukuSaydonCompositionDocument savedComposition;
		if (!savedComposition.Reload(m_strKoukuCompletePlayStatus)) return false;
		const auto savedRevision = savedComposition.Get_LastGood().iRevision;
		const auto publishedRevision = m_pKoukuSaydonBossTool->Get_SourceRevision();
		if (savedRevision != publishedRevision)
		{
			m_strKoukuCompletePlayStatus = "Saved Composition revision " + std::to_string(savedRevision) +
				" differs from published revision " + std::to_string(publishedRevision) +
				". Use Publish Saved Patterns here, then retry Complete Play.";
			return false;
		}
		return true;
	};
	const std::string playLabel = (bingoParentSelected ? "Complete Play - Bingo Parent" :
        bundleSelected ? "Complete Play - " + std::to_string(selectedBundle->Members.size()) + " actors" :
        "Complete Play - Selected Pattern") + std::string("##KoukuServerPattern");
	if (ImGui::Button(playLabel.c_str()))
	{
        if (bingoParentSelected) (void)StartKoukuGateCompletePlay(gate, m_strKoukuCompletePlayStatus);
        else
        {
		// Reload가 UI 포인터의 원본 목록을 교체할 수 있으므로 선택 ID를 값으로 먼저 복사한다.
		const std::string selectedId = bundleSelected ? selectedBundle->strBundleId : pattern->strPatternId;
		if (prepareSavedProduct())
		{
			if (bundleSelected) (void)m_pKoukuSaydonBossTool->Play_SavedBundleById(selectedId,m_strKoukuCompletePlayStatus);
			else (void)m_pKoukuSaydonBossTool->Play_SavedPatternById(selectedId,m_strKoukuCompletePlayStatus);
		}
        }
	}
	ImGui::EndDisabled(); ImGui::SameLine();
	ImGui::BeginDisabled(!arena || (!sequencePlaying && !flow.bActive && !audition.Can_Stop() && !preparing));
	if (ImGui::Button("Stop Complete Play"))
	{
		if (preparing)
			(void)m_pKoukuSaydonBossTool->Cancel_PlayPreparation(m_strKoukuCompletePlayStatus);
		else if (sequencePlaying)
		{
			CancelKoukuGateCompletePlay("Complete Play stop requested.");
		}
		else (void)service.Stop(m_strKoukuCompletePlayStatus);
	}
	ImGui::SameLine(); ImGui::BeginDisabled(audition.strBundleId.empty() || sequencePlaying || flow.bActive || !audition.iRoomAuditionEpoch);
	if (ImGui::Button("Restart Bundle")) (void)service.Restart_Bundle(m_strKoukuCompletePlayStatus);
	ImGui::EndDisabled(); ImGui::EndDisabled();
	ImGui::BeginDisabled(!arena || audition.Is_InFlight() || flow.bActive || sequencePlaying || preparing);
	if (ImGui::Button(gate == "BINGO" ? "Complete Play - Bingo Parent" : "Complete Play - Sequences + Pattern Flow"))
		(void)StartKoukuGateCompletePlay(gate, m_strKoukuCompletePlayStatus);
	ImGui::SameLine();
	if (ImGui::Button("Play Saved Pattern Flow"))
		(void)m_pKoukuSaydonBossTool->Play_PatternFlow(gate, m_strKoukuCompletePlayStatus);
	ImGui::SameLine();
	if (ImGui::Button("Composition Play All") && prepareSavedProduct())
		(void)m_pKoukuSaydonBossTool->Play_CompositionAll(gate, m_strKoukuCompletePlayStatus);
	ImGui::EndDisabled();
	if (preparing) ImGui::TextWrapped("%s", m_pKoukuSaydonBossTool->Get_Status().c_str());
    RenderKoukuRaidPreparationDiagnostics();
	ImGui::Text("Individual pattern: %s",Describe_KoukuSaydonPatternAuditionState(audition.eState));
	if (!flow.strStatus.empty()) ImGui::TextWrapped("%s", flow.strStatus.c_str());
	if (!audition.strBundleId.empty()) ImGui::Text("Bundle %s | run %u | common tick %u",audition.strBundleId.c_str(),audition.iRoomAuditionEpoch,audition.iCommonStartTick);
	for (const auto& member : audition.Members) ImGui::BulletText("%s | boss %u | %s | state %u",member.strMemberId.c_str(),member.iBossNetEntityId,member.strPatternId.c_str(),unsigned(member.eState));
    const auto* raidArena = CLevel_KoukuSaydonArena::Get_Active();
    if (!raidArena || !raidArena->Get_KoukuRaidState().iRunEpoch ||
        audition.strStatus != "Level changed; no KoukuSaydon Server pattern is Live in this Client session.")
        ImGui::TextWrapped("%s", audition.strStatus.c_str());
	// 제출 직후에는 대기 메시지를 한 번 표시하고 이후 결과 판정은 서비스가 맡는다.
	if (!m_strKoukuCompletePlayStatus.starts_with("Waiting for the Server to admit") &&
		m_strKoukuCompletePlayStatus != audition.strStatus)
		ImGui::TextWrapped("%s",m_strKoukuCompletePlayStatus.c_str());
    // 이전 레이드 상태의 broadcast가 위 표시 상태를 갱신하더라도
    // 아직 결론 나지 않았거나 거부된 명령의 상태 메시지는 보존한다.
    if (!m_strKoukuRaidReplyStatus.empty() && m_strKoukuRaidReplyStatus != m_strKoukuCompletePlayStatus &&
        m_strKoukuRaidReplyStatus != audition.strStatus)
        ImGui::TextWrapped("%s", m_strKoukuRaidReplyStatus.c_str());
}

void CMainApp::RenderKoukuRaidPreparationDiagnostics()
{
    using namespace LostArk::Shared;
    auto& network = CNetworkManager::Get();
    const auto connection = network.Get_SessionDiagnosticSnapshot();
    ImGui::Separator();
    ImGui::Text("Connection: %s | %s | local Player %u | process %lu",
        network.Is_Connected() ? "CONNECTED" : "DISCONNECTED", connection.strEndpoint.c_str(),
        network.Get_LocalPlayerId(), GetCurrentProcessId());
    if (connection.iLastReceiveUnixMs)
    {
        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        const auto ageMs = nowMs > static_cast<int64_t>(connection.iLastReceiveUnixMs) ?
            nowMs - static_cast<int64_t>(connection.iLastReceiveUnixMs) : 0;
        ImGui::Text("Last packet: %.1f s ago | receive queue: %zu", ageMs / 1000.0, connection.iRawQueueDepth);
        if (ageMs >= 5000)
            ImGui::TextWrapped("No recent Server packet. Readiness below is the last received state, not a live confirmation.");
    }
    const auto* arena = CLevel_KoukuSaydonArena::Get_Active();
    if (!arena) { ImGui::TextWrapped("Raid: no active KoukuSaydon arena."); return; }
    const auto& state = arena->Get_KoukuRaidState();
    const char* phase = "UNKNOWN";
    switch (state.ePhase)
    {
    case KOUKUSAYDON_RAID_PHASE::INACTIVE: phase = "INACTIVE"; break;
    case KOUKUSAYDON_RAID_PHASE::PREPARING: phase = "PREPARING"; break;
    case KOUKUSAYDON_RAID_PHASE::CINEMATIC: phase = "CINEMATIC / PLAYING SEQUENCE"; break;
    case KOUKUSAYDON_RAID_PHASE::COMBAT: phase = "COMBAT"; break;
    case KOUKUSAYDON_RAID_PHASE::WAIT_GATE: phase = "WAIT_GATE"; break;
    case KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME: phase = "WAIT_MINIGAME"; break;
    case KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY: phase = "WAIT_ENTRY"; break;
    case KOUKUSAYDON_RAID_PHASE::COMPLETE: phase = "COMPLETE"; break;
    case KOUKUSAYDON_RAID_PHASE::ABORTED: phase = "ABORTED"; break;
    default: break;
    }
    ImGui::Text("Server raid: %s | run %u | gate %s", phase, state.iRunEpoch, state.strGateId.c_str());
    if (m_KoukuRaidResourcePreparation)
        ImGui::TextWrapped("Before START: this Client is preparing resources. The Server participant roster is not fixed yet.");
    if (m_iKoukuRaidPendingRequest)
        ImGui::Text("Waiting for the final Server reply to request %u.", m_iKoukuRaidPendingRequest);
    if (!state.strReason.empty()) ImGui::TextWrapped("Server reason: %s", state.strReason.c_str());
    if (!state.iRunEpoch) return;

    const auto count = state.ParticipantPlayerIds.size();
    size_t readyCount = 0u;
    for (size_t i = 0u; i < count; ++i)
        if (state.iReadyMask & (1u << i)) ++readyCount;
    ImGui::Text("Server-confirmed preparation: %zu / %zu | maximum room size: 4", readyCount, count);
    ImGui::ProgressBar(count ? static_cast<float>(readyCount) / static_cast<float>(count) : 0.f);
    if (ImGui::BeginTable("KoukuRaidParticipants", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Participant");
        ImGui::TableSetupColumn("Server confirmation");
        ImGui::TableSetupColumn("Waiting on");
        ImGui::TableHeadersRow();
        for (size_t i = 0u; i < count; ++i)
        {
            const auto id = state.ParticipantPlayerIds[i];
            const bool local = id == network.Get_LocalPlayerId();
            const bool ready = (state.iReadyMask & (1u << i)) != 0u;
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            ImGui::Text("Player %u%s%s", id, local ? " (this Client)" : "", id == state.iOwnerPlayerId ? " (owner)" : "");
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(ready ? "READY received" : "READY not received");
            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", ready ? "Prepared" : (local ? "See local preparation below" :
                "Remote Client preparation / READY delivery. Open its F1 panel for the exact resource stage."));
        }
        ImGui::EndTable();
    }
    const auto& diagnostics = m_KoukuRaidPreparationDiagnostics;
    if (diagnostics.epoch == state.iRunEpoch)
    {
        ImGui::Text("Local preparation stage: %s%s", diagnostics.stage.c_str(), diagnostics.failed ? " [FAILED]" : "");
        ImGui::TextWrapped("%s", diagnostics.detail.c_str());
        const auto local = std::find(state.ParticipantPlayerIds.begin(), state.ParticipantPlayerIds.end(), network.Get_LocalPlayerId());
        const bool serverConfirmed = local != state.ParticipantPlayerIds.end() &&
            (state.iReadyMask & (1u << std::distance(state.ParticipantPlayerIds.begin(), local))) != 0u;
        if (serverConfirmed) ImGui::TextUnformatted("Server confirmed this Client READY.");
        else if (!diagnostics.submission.empty()) ImGui::TextWrapped("%s", diagnostics.submission.c_str());
        if (state.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING)
        {
            const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - diagnostics.observedAt).count();
            ImGui::Text("Preparation observed on this Client: %lld s | Server limit: 20 min", static_cast<long long>(seconds));
        }
    }
    if (state.ePhase == KOUKUSAYDON_RAID_PHASE::PREPARING)
        ImGui::TextWrapped("%s", readyCount == count ?
            "All READY confirmations received; waiting for the Server cinematic state." :
            "Playback has not started. It starts automatically after every listed participant is ready. Debug builds use the same barrier.");
    if (state.ePhase == KOUKUSAYDON_RAID_PHASE::ABORTED)
        ImGui::TextWrapped("This run was aborted. Waiting will not restart it; resolve the Server/local reason and start again explicitly.");
    if (!state.strSequencePatternId.empty()) ImGui::TextWrapped("Sequence: %s", state.strSequencePatternId.c_str());
    if (state.ePhase == KOUKUSAYDON_RAID_PHASE::CINEMATIC)
        ImGui::Text("Sequence ticks: %u -> %u | latest Server tick: %u", state.iStartTick, state.iEndTick, connection.iLastServerTick);
}

void CMainApp::RefreshCompletePlayPatternOptions()
{
	if (nullptr == m_pBalanceTool)
		m_pBalanceTool = make_unique<CBalanceTool>();
	if (nullptr == m_pValtanBossTool)
	{
		m_pValtanBossTool = make_unique<CValtanBossTool>(
			make_shared<CNetworkPlayerCommandSink>(),
			m_pBalanceTool.get());
	}
	m_bCompletePlayPatternLoadAttempted = true;
	std::vector<CValtanBossTool::SERVER_PATTERN_OPTION> options;
	if (!m_pValtanBossTool->Get_ServerPatternOptions(
			options, m_strCompletePlayStatus))
	{
		return;
	}
	const std::string previous = m_strCompletePlayPatternId;
	m_CompletePlayPatternIds.clear();
	m_CompletePlayPatternLabels.clear();
	m_CompletePlayPatternIds.reserve(options.size());
	m_CompletePlayPatternLabels.reserve(options.size());
	for (const CValtanBossTool::SERVER_PATTERN_OPTION& option : options)
	{
		m_CompletePlayPatternIds.push_back(option.strPatternId);
		m_CompletePlayPatternLabels.push_back(
			option.strPatternId + " | " + option.strDisplayName);
	}
	m_strCompletePlayPatternId.clear();
	if (!m_CompletePlayPatternIds.empty())
	{
		const auto found = std::find(
			m_CompletePlayPatternIds.begin(),
			m_CompletePlayPatternIds.end(), previous);
		if (m_CompletePlayPatternIds.end() != found)
			m_strCompletePlayPatternId = *found;
		else
			m_strCompletePlayPatternId = m_CompletePlayPatternIds.front();
	}
}

bool_t CMainApp::Debug_SelectCompletePlayPattern(
	const std::string& strPatternId)
{
	if (strPatternId.empty())
	{
		m_strCompletePlayStatus =
			"Complete Play selection requires a stable pattern ID.";
		return false;
	}
	if (!m_bCompletePlayPatternLoadAttempted)
		RefreshCompletePlayPatternOptions();
	auto found = std::find(
		m_CompletePlayPatternIds.begin(),
		m_CompletePlayPatternIds.end(), strPatternId);
	if (m_CompletePlayPatternIds.end() == found)
	{
		/* 도메인 소유자가 workspace 목록보다 먼저 결합된 그래프를 갱신할 수 있다.
		   선택 시점에 한 번만 Reload해 두 목록의 시점 차이를 해소한다. */
		RefreshCompletePlayPatternOptions();
		found = std::find(
			m_CompletePlayPatternIds.begin(),
			m_CompletePlayPatternIds.end(), strPatternId);
	}
	if (m_CompletePlayPatternIds.end() == found)
	{
		m_strCompletePlayStatus =
			"Pattern is not in the current Server-admitted Complete Play inventory: " +
			strPatternId + ".";
		return false;
	}
    if (m_strCompletePlayPatternId != *found && m_pValtanBossTool)
        m_pValtanBossTool->Cancel_PlayPreparation("The Complete Play selection changed.");
	m_strCompletePlayPatternId = *found;
	m_strCompletePlayStatus =
		"Complete Play selection: " + m_strCompletePlayPatternId + ".";
	return true;
}

const std::string& CMainApp::Debug_GetSelectedCompletePlayPatternId() const
{
	return m_strCompletePlayPatternId;
}

bool_t CMainApp::Debug_CompletePlaySelected(std::string& strOutStatus)
{
	if (!m_bCompletePlayPatternLoadAttempted)
		RefreshCompletePlayPatternOptions();
	if (nullptr == m_pValtanBossTool || m_strCompletePlayPatternId.empty() ||
		m_CompletePlayPatternIds.end() == std::find(
			m_CompletePlayPatternIds.begin(),
			m_CompletePlayPatternIds.end(), m_strCompletePlayPatternId))
	{
		strOutStatus =
			"Complete Play requires one Server-admitted saved pattern selection.";
		m_strCompletePlayStatus = strOutStatus;
		return false;
	}
	const bool_t submitted = m_pValtanBossTool->Play_ServerPattern(
		m_strCompletePlayPatternId,
		strOutStatus);
	m_strCompletePlayStatus = strOutStatus;
	if (submitted)
	{
#ifdef _DEBUG
		if (nullptr != m_pAnimationTool)
			m_pAnimationTool->Release_ValtanCompositionPreviewForServerPlayback();
		ClaimCompositionPreviewOwner(DEBUG_TOOL::NONE);
#endif
		m_bCompletePlayStatusTracking = true;
		m_strCompletePlayTrackedPatternId = m_strCompletePlayPatternId;
	}
	return submitted;
}

bool_t CMainApp::Debug_OpenValtanPatternFlow(std::string& strOutStatus)
{
	if (FAILED(EnsureDebugTool(DEBUG_TOOL::VALTAN_BOSS)) || nullptr == m_pValtanBossTool)
	{
		strOutStatus = "Pattern Flow owner could not be opened.";
		return false;
	}
	m_pValtanBossTool->Open_PatternFlow();
	strOutStatus =
		"Opened the canonical Boss Pattern Flow owner. Save/Apply and playback remain in that typed owner.";
	return true;
}

void CMainApp::RenderCompletePlayControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.ValtanCompletePlay");
	if (!ImGui::CollapsingHeader(
		"Valtan Complete Play (Server Boss Replay)"))
	{
		return;
	}
	ImGui::TextDisabled(
		"Shared by every open tool: semantic pattern ID -> Server stages/hits -> replicated animation, Effect, Sound, camera and world events.");
	ImGui::TextDisabled(
		"A raw clip or unsaved/unbound asset remains Local Asset Preview and cannot become Complete Play.");
	ImGui::TextDisabled(
		"Complete Play resets boss-owned replay state only; the current replicated arena walls, floors, debris, collision, and Nav state are preserved.");
	if (m_bCompletePlayStatusTracking && nullptr != m_pValtanBossTool)
	{
		std::string serverStatus;
		bool_t inFlight = false;
		if (m_pValtanBossTool->Get_ServerPatternStatus(
				m_strCompletePlayTrackedPatternId, serverStatus, inFlight))
		{
			m_strCompletePlayStatus = std::move(serverStatus);
			m_bCompletePlayStatusTracking = inFlight;
		}
	}
	if (ImGui::SmallButton(
		m_bCompletePlayPatternLoadAttempted ?
			"Reload Complete Play Inventory" :
			"Load Complete Play Inventory"))
	{
		RefreshCompletePlayPatternOptions();
	}
	if (!m_bCompletePlayPatternLoadAttempted)
	{
		ImGui::TextDisabled(
			"Inventory is loaded only on request so opening F1 never parses the canonical Pattern graph.");
		return;
	}

	if (!m_CompletePlayPatternIds.empty())
	{
		ImGui::Text("Saved Patterns (%zu)", m_CompletePlayPatternIds.size());
		if (ImGui::BeginChild(
			"CompletePlayPatternInventory", ImVec2(0.f, 220.f), true))
		{
			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int32_t>(
				m_CompletePlayPatternIds.size()));
			while (clipper.Step())
			{
				for (int32_t iPattern = clipper.DisplayStart;
					iPattern < clipper.DisplayEnd; ++iPattern)
				{
					if (ImGui::Selectable(
						m_CompletePlayPatternLabels[
							static_cast<size_t>(iPattern)].c_str(),
						m_CompletePlayPatternIds[
							static_cast<size_t>(iPattern)] ==
							m_strCompletePlayPatternId))
					{
						(void)Debug_SelectCompletePlayPattern(
							m_CompletePlayPatternIds[
								static_cast<size_t>(iPattern)]);
					}
				}
			}
		}
		ImGui::EndChild();
	}
	const bool_t isValtanArena = ETOUI(LEVEL::VALTAN_ARENA) ==
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t canCompletePlay = nullptr != m_pValtanBossTool &&
		!m_strCompletePlayPatternId.empty() && isValtanArena;
    const bool_t preparing = m_pValtanBossTool && m_pValtanBossTool->Is_PlayPreparationPending();
	ImGui::BeginDisabled(!canCompletePlay || preparing);
	if (ImGui::Button("Complete Play##GlobalServerPattern"))
	{
		(void)Debug_CompletePlaySelected(m_strCompletePlayStatus);
	}
	ImGui::EndDisabled();
    if (preparing && ImGui::Button("Cancel Complete Play Preparation##GlobalServerPattern"))
        m_pValtanBossTool->Cancel_PlayPreparation("Cancelled by user.");
	if (!canCompletePlay)
	{
		ImGui::TextDisabled(
			"Complete Play requires a loaded Pattern selection and Lobby -> Valtan Server admission.");
	}
	else
	{
		ImGui::TextDisabled(
			"Complete Play prepares every selected dependency before automatically submitting. Product and Sound are checked at request time and once more before Server submission.");
	}
	ImGui::TextWrapped("%s", m_strCompletePlayStatus.c_str());
}

#ifdef _DEBUG
void CMainApp::RenderServerArenaActiveControls()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.ServerArena");
	if (!ImGui::CollapsingHeader("Server Arena Active"))
	{
		return;
	}
	ImGui::TextDisabled(
		"Shared control for Effect Resource, Boss, Map, UI and Workbench. Values are replicated Server actual state.");
	if (!ImGui::BeginTabBar("##ServerArenaActiveTabs"))
		return;

	if (ImGui::BeginTabItem("Valtan"))
	{
		if (ETOUI(LEVEL::VALTAN_ARENA) !=
			CGameInstance::Get().Get_CurrentLevelID())
		{
			ImGui::TextDisabled(
				"Enter Valtan from Lobby. No local wall, debris, collision or NavCell fallback exists.");
		}
		else
		{
			ImGui::PushID("SharedValtanArenaState");
			RenderValtanArenaStateControls();
			ImGui::PopID();
		}
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("KoukuSaydon"))
	{
		if (ETOUI(LEVEL::KOUKUSAYDON_ARENA) !=
			CGameInstance::Get().Get_CurrentLevelID())
		{
			ImGui::TextDisabled(
				"Enter KoukuSaydon Arena through Server admission. Stage controls never move a local Character directly.");
		}
		else if (CLevel_KoukuSaydonArena* pKoukuArena =
			CLevel_KoukuSaydonArena::Get_Active())
		{
			const auto& stageMarkers = pKoukuArena->Get_StageMarkers();
			ImGui::Text(
				"Runtime-projected stages (%zu)", stageMarkers.size());
			ImGui::TextDisabled(
				"SL identities come from the published StageMarkers document; no Mario/gate meaning or local coordinate is inferred.");
			if (stageMarkers.empty())
			{
				ImGui::TextDisabled(
					"No admitted KoukuSaydon StageMarkers are available in this Level instance.");
			}
			for (const auto& marker : stageMarkers)
			{
				std::string stageLabel = marker.strSourceLevelId;
				const size_t separator = stageLabel.find_last_of('_');
				if (std::string::npos != separator &&
					separator + 1u < stageLabel.size())
				{
					stageLabel = stageLabel.substr(separator + 1u);
				}
				const std::string buttonLabel = stageLabel +
					"##KoukuStageTeleport_" + marker.strPlacementId;
				const bool_t sequenceExhausted =
					0u == m_iNextKoukuStageTeleportRequestSequence;
				ImGui::BeginDisabled(sequenceExhausted);
				if (ImGui::SmallButton(buttonLabel.c_str()))
				{
					const std::uint32_t requestSequence =
						m_iNextKoukuStageTeleportRequestSequence;
					if ((std::numeric_limits<std::uint32_t>::max)() ==
						m_iNextKoukuStageTeleportRequestSequence)
					{
						m_iNextKoukuStageTeleportRequestSequence = 0u;
					}
					else
					{
						++m_iNextKoukuStageTeleportRequestSequence;
					}
					(void)pKoukuArena->Request_StageTeleport(
						requestSequence, marker.strPlacementId,
						m_strKoukuStageTeleportStatus);
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::TextWrapped(
					"%s | %s",
					marker.strDisplayNameKo.c_str(),
					marker.strPlacementId.c_str());
			}
			if (0u == m_iNextKoukuStageTeleportRequestSequence)
			{
				m_strKoukuStageTeleportStatus =
					"KoukuSaydon stage teleport request sequence is exhausted; restart the Client before another request.";
			}
		}
		else
		{
			ImGui::TextDisabled(
				"KoukuSaydon Arena is selected, but its active Level instance is unavailable.");
		}
		ImGui::TextWrapped("%s", m_strKoukuStageTeleportStatus.c_str());
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
}

// 이 const 포인터는 WorldObjectTool의 draft를 읽도록 빌려 주며 소유권을 넘기지 않는다. 문서가 교체된 뒤에는 이전 포인터를 계속 쓰지 않고 다시 조회해야 한다.
const CWorldSequenceDocument* CMainApp::CompositionPreviewWorldSource() const
{
	// 저작 미리보기는 직접 Animation 레인에서 편집하는 WorldObjectTool의 draft를 읽는다.
	// 제품 재생은 기존 경로를 통해 게시된 문서를 읽는다.
	return m_pWorldObjectTool ? m_pWorldObjectTool->Get_AuthoringDraftDocument() : nullptr;
}

void CMainApp::RefreshWorldObjectResources()
{
	if (!m_pKoukuSaydonActionWorkbench && !m_pSequenceActionWorkbench) return;
	for (auto* editor : {m_pKoukuSaydonActionWorkbench.get(), m_pSequenceActionWorkbench.get()})
		if (editor)
		{
			// Workbench의 이력 버튼을 WorldObjectTool의 실제 문서 소유자에 연결한다. capture는 const 공유 snapshot을, restore는 기대값과 목표값을 전달한다.
			editor->Set_WorldAnimationHistoryCallbacks(
				[this](bool prepare) -> std::shared_ptr<const CWorldSequenceDocument> {
					if (prepare && FAILED(EnsureDebugTool(DEBUG_TOOL::WORLD_OBJECT, false))) return {};
					return m_pWorldObjectTool ? m_pWorldObjectTool->Capture_TimelineHistory(prepare) : nullptr;
				},
				[this](const auto& expected, const auto& desired, std::string& status) {
					if (!m_pWorldObjectTool)
					{ status = "World animation owner is unavailable. History was preserved."; return false; }
					return m_pWorldObjectTool->Restore_TimelineHistory(expected, desired, status);
				});
			editor->Set_WorldAnimationCallbacks(
			[this](const KOUKU_WORLD_ANIMATION_EDIT& edit, std::string& status) {
				if (FAILED(EnsureDebugTool(DEBUG_TOOL::WORLD_OBJECT, false)) || !m_pWorldObjectTool)
				{ status = "World animation owner is unavailable. Existing clips are preserved."; return false; }
				return m_pWorldObjectTool->Edit_AnimationTimeline(edit.instanceId, edit.slotId, edit.clipName,
					edit.expectedStartMs, edit.startMs, edit.sourceInMs, edit.sourceOutMs, status);
			},
			[this](std::string& status) {
				if (!m_pWorldObjectTool)
				{ status = "World animation draft is unavailable. Save was not completed."; return false; }
				// Timeline Save는 저작 원본만 저장하며 publish는 별도 명령으로 남긴다.
				const bool saved = !m_pWorldObjectTool->Is_Dirty() || m_pWorldObjectTool->Save_Source(false);
				status = m_pWorldObjectTool->Get_Status();
				if (saved)
					for (auto* owner : {m_pKoukuSaydonActionWorkbench.get(), m_pSequenceActionWorkbench.get()})
						if (owner) owner->Notify_WorldAnimationSaved();
				return saved;
			});
		}
	const auto* level = CLevel_KoukuSaydonArena::Get_Active();
	const CWorldSequenceDocument* document = m_pWorldObjectTool ? m_pWorldObjectTool->Get_AuthoringDraftDocument() : nullptr;
	const uint64_t generation = document ? m_pWorldObjectTool->Get_SavedGeneration() : 0;
	if (!document && level) document = &level->Get_WorldSequenceDocument();
	const uint32_t revision = document ? document->Get_Revision() : 0;
	// 캐시는 문서 주소만 비교하지 않고 저장 세대와 revision도 함께 비교한다. 주소가 같아도 저장 세대나 revision이 달라지면 Resources 표시 자료를 다시 만든다.
	if (m_pWorldObjectCatalogSource == document && m_iWorldObjectCatalogGeneration == generation &&
		m_iWorldObjectCatalogRevision == revision) return;
	m_pWorldObjectCatalogSource = document; m_iWorldObjectCatalogGeneration = generation; m_iWorldObjectCatalogRevision = revision;
	std::vector<KOUKU_WORLD_SEQUENCE_RESOURCE> resources;
	if (document)
	{
		// Logic 대상 선택에는 모든 상태를 남기고 Resources 목록에는 부모별로 한 번만 표시한다.
		for (const auto& instance : document->Get_Instances())
		{
			const auto* sequence = document->Find_Template(instance.templateId);
			if (!sequence) continue;
			KOUKU_WORLD_SEQUENCE_RESOURCE row;
			row.strInstanceId = instance.instanceId; row.strDisplayName = sequence->displayName;
			row.bEnabled = instance.enabled; row.strAnchorKind = instance.anchorKind;
			row.SavedPosition = {instance.position.x, instance.position.y, instance.position.z};
			const WORLD_SEQUENCE_OBJECT_RESOURCE* owner = nullptr;
			if (instance.bindings.size() == 1u &&
				instance.bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
				owner = document->Find_ObjectResource(instance.bindings.front().targetId);
			if (!owner)
				for (const auto& object : document->Get_ObjectResources())
					if (object.sequenceInstanceId == instance.instanceId) { owner = &object; break; }
			if (owner)
			{
				row.strObjectResourceId = owner->objectId;
				row.strObjectDisplayName = owner->displayName;
				row.strModelAssetId = owner->modelAssetId;
				row.bDefaultMotion = owner->defaultMotionInstanceId == instance.instanceId;
				row.bUntilDestroyed = owner->combatBody && owner->combatBody->lifetimePolicy == "UNTIL_DESTROYED";
				row.iMaximumHp = owner->combatBody ? owner->combatBody->maxHp : 0u;
				row.bSupportsPlacement = owner->sequenceInstanceId.empty() &&
					instance.anchorKind == owner->anchorKind &&
					(instance.anchorKind == "WORLD" || instance.anchorKind == "BOSS" || instance.anchorKind == "PLAYER");
			}
			for (const auto& animation : sequence->animationTracks)
			{
				row.AnimationClips.push_back(animation.clipName);
				std::uint32_t endMs = sequence->durationMs;
				for (const auto& next : sequence->animationTracks)
					if (next.slotId == animation.slotId && next.startMs > animation.startMs)
						endMs = (std::min)(endMs, next.startMs);
				KOUKU_WORLD_ANIMATION_INFO info;
				info.strClipName = animation.clipName; info.strSlotId = animation.slotId;
				info.strDisplayName = animation.displayName;
				info.iLocalStartMs = animation.startMs;
				info.fInstanceSpeed = instance.playbackSpeed; info.fTrackPlaybackRate = animation.playbackRate;
				if (!animation.loop && animation.sourceEndMs > animation.sourceStartMs)
				{
					const double naturalEnd = animation.startMs +
						(animation.sourceEndMs - animation.sourceStartMs) / double(animation.playbackRate);
					endMs = (std::min)(endMs, static_cast<uint32_t>(std::ceil(naturalEnd)));
				}
				info.fStartMs = instance.startDelayMs + animation.startMs / static_cast<double>(instance.playbackSpeed);
				info.fEndMs = instance.startDelayMs + endMs / static_cast<double>(instance.playbackSpeed);
				info.fPlaybackRate = animation.playbackRate * instance.playbackSpeed;
				info.iSourceStartMs = animation.sourceStartMs; info.iSourceEndMs = animation.sourceEndMs;
				info.bLoop = animation.loop; info.bHoldLastFrame = animation.holdLastFrame;
				row.AnimationTracks.push_back(std::move(info));
			}
			row.iEmissionCount = sequence->objectMotion.EmissionCount();
			const double span = instance.startDelayMs + static_cast<double>(sequence->durationMs) / instance.playbackSpeed;
			row.iDurationMs = static_cast<uint32_t>((std::clamp)(span, 1.0, static_cast<double>(UINT32_MAX)));
			float3_t placedPosition;
			if (level && level->Try_GetWorldSequencePlacementBaseline(instance, placedPosition))
			{ row.bHasBoundPlacement = true; row.fBoundX = placedPosition.x; row.fBoundZ = placedPosition.z; }
			resources.push_back(std::move(row));
		}
		for (const auto& object : document->Get_ObjectResources())
		{
			const bool hasDefault = std::any_of(resources.begin(), resources.end(), [&](const auto& row) {
				return row.strObjectResourceId == object.objectId && row.bDefaultMotion;
			});
			if (hasDefault) continue;
			KOUKU_WORLD_SEQUENCE_RESOURCE row;
			row.strObjectResourceId = object.objectId;
			row.strObjectDisplayName = object.displayName; row.strDisplayName = object.displayName;
			row.strAnchorKind = object.anchorKind; row.bEnabled = false;
			if (!object.motionInstanceIds.empty())
			{
				row.strInstanceId = object.objectId;
				row.bMotionGroup = true;
				row.bDefaultMotion = true;
				row.bEnabled = true;
				row.bSupportsPlacement = object.anchorKind == "WORLD";
				row.iEmissionCount = 0u;
				for (const auto& memberId : object.motionInstanceIds)
				{
					const auto member = std::find_if(resources.begin(), resources.end(),
						[&](const auto& value) { return value.strInstanceId == memberId && !value.bMotionGroup; });
					const auto* instance = document->Find_Instance(memberId);
					const auto* sequence = instance ? document->Find_Template(instance->templateId) : nullptr;
					const auto* model = instance && instance->bindings.size() == 1u ?
						document->Find_ObjectResource(instance->bindings.front().targetId) : nullptr;
					if (member == resources.end() || !sequence || !model || model->modelAssetId.empty() ||
						!model->motionInstanceIds.empty() || model->combatBody || instance->walkableSurface ||
						!sequence->colliderTracks.empty() || instance->anchorKind != "WORLD" ||
						instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
						(instance->motionEnd != WORLD_SEQUENCE_MOTION_END::STOP && instance->motionEnd != WORLD_SEQUENCE_MOTION_END::LOOP))
					{ row.bEnabled = false; row.bSupportsPlacement = false; continue; }
					if (!member->bEnabled) continue;
					if (row.iEmissionCount == 0u) row.SavedPosition = member->SavedPosition;
					else if (row.SavedPosition != member->SavedPosition) row.bSupportsPlacement = false;
					row.bSupportsPlacement = row.bSupportsPlacement && member->bSupportsPlacement &&
						member->strAnchorKind == object.anchorKind;
					const double span = instance->startDelayMs +
						static_cast<double>(sequence->PresentationSpanMs()) / instance->playbackSpeed;
					row.iDurationMs = (std::max)(row.iDurationMs, static_cast<uint32_t>(
						(std::clamp)(std::ceil(span), 1.0, static_cast<double>(UINT32_MAX))));
					row.iEmissionCount += member->iEmissionCount;
					row.AnimationClips.insert(row.AnimationClips.end(),
						member->AnimationClips.begin(), member->AnimationClips.end());
				}
				row.bEnabled = row.bEnabled && row.iEmissionCount > 0u;
			}
			row.bUntilDestroyed = object.combatBody && object.combatBody->lifetimePolicy == "UNTIL_DESTROYED";
			row.iMaximumHp = object.combatBody ? object.combatBody->maxHp : 0u;
			resources.push_back(std::move(row));
		}
		// 원본 motion은 Logic·편집에서 계속 조회할 수 있게 남긴다. Object Append는
		// 같은 이름의 g0 기본값 대신 유일하게 결정된 완전한 motion group을 선택하도록 연결한다.
		for (auto& donor : resources)
		{
			if (!donor.bDefaultMotion || donor.bMotionGroup) continue;
			std::string ownerId; bool ambiguous = false;
			for (const auto& group : resources)
			{
				if (!group.bMotionGroup || !group.bEnabled) continue;
				const auto* object = document->Find_ObjectResource(group.strObjectResourceId);
				if (!object || std::find(object->motionInstanceIds.begin(), object->motionInstanceIds.end(), donor.strInstanceId) == object->motionInstanceIds.end()) continue;
				if (!ownerId.empty()) { ambiguous = true; break; }
				ownerId = group.strObjectResourceId;
			}
			if (!ambiguous) donor.strAppendGroupObjectId = std::move(ownerId);
		}
	}
	const std::string status = document ?
		"World animation clips: drag their center or edges directly in the Animation timeline. Save commits the draft." :
		"Enter KoukuSaydon or open Object Tool to load saved Objects.";
	if (m_pKoukuSaydonActionWorkbench) m_pKoukuSaydonActionWorkbench->Set_WorldSequenceResources(resources, status);
	if (m_pSequenceActionWorkbench) m_pSequenceActionWorkbench->Set_WorldSequenceResources(std::move(resources), status);
}

#endif

void CMainApp::RenderHUDBarPositionControls()
{
	if (auto* arena = CLevel_KoukuSaydonArena::Get_Active())
	{
		float2_t offset{}; f32_t head = 0.f;
		static std::string positionStatus;
		if (arena->Get_MadnessGaugePosition(offset, head))
		{
			ImGui::SeparatorText("Madness gauge position");
			bool changed = ImGui::DragFloat2("Screen offset X / Y##KoukuMadness", &offset.x, 1.f, -1280.f, 1280.f, "%.1f");
			changed |= ImGui::DragFloat("World height (m)##KoukuMadness", &head, 0.05f, -10.f, 10.f, "%.2f");
			if (changed) (void)arena->Set_MadnessGaugePosition(offset, head);
			if (ImGui::Button("Save position##KoukuMadness")) (void)arena->Save_MadnessGaugePosition(positionStatus);
			ImGui::SameLine();
			if (ImGui::Button("Reload saved position##KoukuMadness")) (void)arena->Reload_MadnessGaugePosition(positionStatus);
			ImGui::TextDisabled("1280 x 720 reference pixels; +Y moves down. Applies to all player gauges.");
			ImGui::TextDisabled("Save writes Data/UI/KoukuSaydon/KoukuHudModes.json. Card maze hides all gauges.");
			if (!positionStatus.empty()) ImGui::TextWrapped("%s", positionStatus.c_str());
		}
	}

	ImGui::SeparatorText("Health bar positions");
	auto offsets = Get_HealthBarPositions();
	bool changed = ImGui::DragFloat2("Kouku Stagger head X / Y##HUDBar", &offsets[0], 1.f, -1280.f, 1280.f, "%.1f");
	auto mechanicScale = Get_MechanicBarScale();
	bool sizeChanged = ImGui::SliderFloat("Kouku Stagger width##HUDBar", &mechanicScale[0], 0.1f, 3.f, "%.3fx");
	sizeChanged |= ImGui::SliderFloat("Kouku Stagger thickness##HUDBar", &mechanicScale[1], 0.1f, 3.f, "%.3fx");
	if (sizeChanged && !Set_MechanicBarScale(mechanicScale))
		m_strHealthBarPositionStatus = "Stagger size must be finite and between 0.1 and 3.";
	ImGui::SeparatorText("Valtan Stagger");
	ImGui::Checkbox("Show debug##ValtanStagger", &m_bValtanStaggerDebug);
	auto valtanTuning = m_ValtanMechanicTuning;
	bool valtanChanged = ImGui::DragFloat2("Head X / Y##ValtanStagger", &valtanTuning[0], 1.f, -1280.f, 1280.f, "%.1f");
	valtanChanged |= ImGui::SliderFloat("Width##ValtanStagger", &valtanTuning[2], 0.1f, 3.f, "%.3fx");
	valtanChanged |= ImGui::SliderFloat("Thickness##ValtanStagger", &valtanTuning[3], 0.1f, 3.f, "%.3fx");
	ImGui::SeparatorText("Valtan Armor Break PNG");
	ImGui::Checkbox("Show debug##ValtanArmorBreak", &m_bValtanArmorBreakDebug);
	valtanChanged |= ImGui::DragFloat2("Head X / Y##ValtanArmorBreak", &valtanTuning[4], 1.f, -1280.f, 1280.f, "%.1f");
	valtanChanged |= ImGui::SliderFloat("Width##ValtanArmorBreak", &valtanTuning[6], 0.1f, 3.f, "%.3fx");
	valtanChanged |= ImGui::SliderFloat("Height##ValtanArmorBreak", &valtanTuning[7], 0.1f, 3.f, "%.3fx");
	if (valtanChanged && !Set_ValtanMechanicTuning(valtanTuning))
		m_strHealthBarPositionStatus = "Valtan HUD values must be finite; offsets -1280..1280, size 0.1..3.";
	ImGui::TextDisabled("Show debug uses the live Valtan head; it does not change combat state.");
	ImGui::SeparatorText("World HP offsets");
	changed |= ImGui::DragFloat2("Ally HP X / Y##HUDBar", &offsets[2], 1.f, -1280.f, 1280.f, "%.1f");
	changed |= ImGui::DragFloat2("Normal monster HP X / Y##HUDBar", &offsets[4], 1.f, -1280.f, 1280.f, "%.1f");
	changed |= ImGui::DragFloat2("KoukuSaydon HP X / Y##HUDBar", &offsets[6], 1.f, -1280.f, 1280.f, "%.1f");
	changed |= ImGui::DragFloat2("Kouku HP X / Y##HUDBar", &offsets[8], 1.f, -1280.f, 1280.f, "%.1f");
	changed |= ImGui::DragFloat2("Valtan HP X / Y##HUDBar", &offsets[10], 1.f, -1280.f, 1280.f, "%.1f");
	if (changed && !Set_HealthBarPositions(offsets))
		m_strHealthBarPositionStatus = "Offsets must be finite and between -1280 and 1280.";
	if (ImGui::Button("Save positions and stagger size##HUDBar")) (void)Save_HealthBarPositions(m_strHealthBarPositionStatus);
	ImGui::SameLine();
	if (ImGui::Button("Reload saved positions and size##HUDBar")) (void)Reload_HealthBarPositions(m_strHealthBarPositionStatus);
	ImGui::TextDisabled("1280 x 720 reference pixels; +Y moves down. Frames and shields move together.");
	ImGui::TextDisabled("Kouku and Valtan stagger settings are independent; X/Y offsets the boss head.");
	ImGui::TextDisabled("Size uses the authored frame center. Default width 0.333x; thickness 1x.");
	ImGui::TextDisabled("Save writes Data/UI/KoukuSaydon/KoukuHudModes.json. Applies across levels.");
	if (!m_strHealthBarPositionStatus.empty()) ImGui::TextWrapped("%s", m_strHealthBarPositionStatus.c_str());
}

void CMainApp::RenderBalanceTestLauncher()
{
	ImGui::SeparatorText("Balance Test");
	if (ImGui::Button("Open Balance Test", ImVec2(ImGui::GetContentRegionAvail().x, 0.f)))
		m_strToolStatus = SUCCEEDED(EnsureDebugTool(DEBUG_TOOL::BALANCE)) ?
			"Balance Test opened." : "Balance Test initialization failed.";
	ImGui::TextWrapped("Player / Skill / Damage / Boss tuning and room cooldown: Debug (3s) or Release (Retail).");

	/* F1 도구에서 아이템 수리 창을 직접 여는 진입점이다. 여기서는 기존 view의 Open을
	호출하며 NPC 배치나 상호작용 경로를 정의하지 않는다. */
	if (nullptr != m_pRepairWindowView)
	{
		ImGui::SeparatorText("Item Repair");
		bool_t bRepairOpen = m_pRepairWindowView->Is_Open();
		if (ImGui::Checkbox("Item Repair Window", &bRepairOpen))
			bRepairOpen ? m_pRepairWindowView->Open() : m_pRepairWindowView->Close();
	}
}

void CMainApp::RenderDeveloperTools()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Hub.Build");
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	ImVec2 vDefaultSize(720.f, 760.f);
	if (nullptr != pViewport)
	{
		const ImVec2 vFirstUseAvailableSize(
			(std::max)(320.f, pViewport->WorkSize.x - 48.f),
			(std::max)(240.f, pViewport->WorkSize.y - 48.f));
		vDefaultSize = ImVec2(
			(std::min)(vDefaultSize.x, vFirstUseAvailableSize.x),
			(std::min)(vDefaultSize.y, vFirstUseAvailableSize.y));
		ImGui::SetNextWindowPos(
			ImVec2(pViewport->WorkPos.x + 24.f, pViewport->WorkPos.y + 24.f),
			ImGuiCond_FirstUseEver);
	}
	ImGui::SetNextWindowSize(vDefaultSize, ImGuiCond_FirstUseEver);

	/* 처음 열 때의 창 크기만 제안한다. 최소·최대 크기를 강제하지 않아 이후 크기는
	   ImGui의 가장자리 조절과 저장된 창 배치를 따른다. */
	if (!ImGui::Begin(
		"LostArk Developer Tools###LostArkDeveloperToolsResizableV1",
		&m_bDeveloperToolsVisible))
	{
		ImGui::End();
		return;
	}

	const uint32_t currentLevelId =
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t isMapEditorWorkspace =
		ETOUI(LEVEL::DEVELOPMENT) == currentLevelId &&
		CMapEditorWorkspaceService::Is_Active();
	ImGui::Text("Current level id: %u", currentLevelId);
	if (currentLevelId == ETOUI(LEVEL::CHARACTER_SELECT))
		Render_CameraSpeedControls();
    if (ImGui::CollapsingHeader("Bern Entrance Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // 같은 F1 구역에서 FOV를 고른 뒤 재생한다. UI는 현재 Level에 요청만 전달한다.
        auto* const bern = currentLevelId == ETOUI(LEVEL::BERN) ? CLevel_Bern::Get_Active() : nullptr;
        ImGui::BeginDisabled(nullptr == bern);
        if (ImGui::Button("Start Bern Cutscene") && nullptr != bern)
            (void)bern->Request_DebugEntranceReplay();
        ImGui::EndDisabled();
        if (nullptr == bern)
            ImGui::TextDisabled("Enter Bern to replay its entrance cutscene.");
        else if (!bern->Get_DebugEntranceReplayStatus().empty())
            ImGui::TextWrapped("%s", bern->Get_DebugEntranceReplayStatus().c_str());
        float bernFov = CLevel_Bern::Get_EntranceCinematicFovYOverride().value_or(
            CLevel_Bern::Get_EntranceCinematicAuthoredFovYDegrees());
        ImGui::SetNextItemWidth(260.f);
        if (ImGui::SliderFloat("Cutscene vertical FOV (session)", &bernFov,
            20.f, 60.f, "%.1f deg", ImGuiSliderFlags_AlwaysClamp))
            (void)CLevel_Bern::Set_EntranceCinematicFovYOverride(bernFov);
        for (const float preset : {30.f, 35.f, 40.f, 45.f})
        {
            if (preset != 30.f) ImGui::SameLine();
            ImGui::PushID(static_cast<int>(preset));
            const std::string label = std::to_string(static_cast<int>(preset)) + " deg";
            if (ImGui::SmallButton(label.c_str()))
                (void)CLevel_Bern::Set_EntranceCinematicFovYOverride(preset);
            ImGui::PopID();
        }
        ImGui::SameLine();
        if (ImGui::Button("Use authored FOV")) CLevel_Bern::Clear_EntranceCinematicFovYOverride();
        ImGui::TextDisabled("Authored %.1f deg / session override %s",
            CLevel_Bern::Get_EntranceCinematicAuthoredFovYDegrees(),
            CLevel_Bern::Get_EntranceCinematicFovYOverride().has_value() ? "ON" : "OFF");
        ImGui::TextWrapped("Only the Bern entrance cinematic, from its next camera sample. You can select this before entering Bern. Gameplay and ship cameras are unchanged; this session control does not save JSON.");
    }
	RenderDebugLevelNavigation();
#ifdef _DEBUG
	RenderWorldMeshInspection();
	ImGui::TextDisabled(isMapEditorWorkspace ?
		"Map Editor is active. Open Map Tool to author the selected Area." :
		(currentLevelId == ETOUI(LEVEL::KOUKUSAYDON_ARENA) ?
			"Kouku runtime supports Map, Object, Camera and Sequence authoring through F1." :
			"F1 opens tools. Map authoring is available in Lobby Test or the Kouku runtime."));
#endif
	ImGui::SeparatorText("Tools");

	const auto toolButton = [this](
		const char_t* pLabel,
		const DEBUG_TOOL eTool,
		const bool_t isEnabled)
	{
		ImGui::BeginDisabled(!isEnabled);
		const bool_t bVisible = IsDebugToolVisible(eTool);
		const std::string label =
			std::string(bVisible ? "Hide " : "Open ") + pLabel;
		if (ImGui::Button(
				label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0.f)))
		{
			if (bVisible)
			{
				SetDebugToolVisible(eTool, false);
				m_strToolStatus = std::string(pLabel) +
					" hidden; its domain draft was not discarded.";
			}
			else
			{
				m_strToolStatus = SUCCEEDED(EnsureDebugTool(eTool)) ?
					std::string(pLabel) + " opened alongside existing tools." :
					std::string(pLabel) + " initialization failed.";
			}
		}
		ImGui::EndDisabled();
	};

	const float fToolButtonWidth = ImGui::GetContentRegionAvail().x;
	const int32_t iToolButtonColumns = fToolButtonWidth >= 1080.f ? 3 :
		(fToolButtonWidth >= 660.f ? 2 : 1);
	if (ImGui::BeginTable(
			"##DeveloperToolLaunchGrid", iToolButtonColumns,
			ImGuiTableFlags_SizingStretchSame))
	{
		const auto toolCell = [&toolButton](
			const char_t* const pLabel, const DEBUG_TOOL eTool)
		{
			ImGui::TableNextColumn();
			toolButton(pLabel, eTool, true);
		};
		toolCell("Valtan Boss Tool", DEBUG_TOOL::VALTAN_BOSS);
		toolCell("KoukuSaydon Boss Tool", DEBUG_TOOL::KOUKU_SAYDON_BOSS);
#ifdef _DEBUG
		toolCell("Valtan Logic Pattern", DEBUG_TOOL::VALTAN_LOGIC_PATTERN);
		toolCell("Camera Tool", DEBUG_TOOL::CAMERA);
		toolCell("Action Workbench", DEBUG_TOOL::SEQUENCER);
		toolCell("Animation Clip Tool", DEBUG_TOOL::ANIMATION);
		toolCell("Effect Tool V1", DEBUG_TOOL::EFFECT);
		toolCell("Effect Tool V2", DEBUG_TOOL::EFFECT_V2);
		toolCell("World Scene Tool", DEBUG_TOOL::WORLD_SCENE);
		toolCell("World Level Tool", DEBUG_TOOL::WORLD_LEVEL);
		toolCell("DimensionMaster Guide", DEBUG_TOOL::GUIDE_AI);
		toolCell("Waterpang AI Tool", DEBUG_TOOL::MAHARAKA_AI);
		toolCell("Map Tool", DEBUG_TOOL::MAP);
		toolCell("Rendering Workbench", DEBUG_TOOL::RENDERING);
		toolCell("Composition Profiler", DEBUG_TOOL::PROFILER);
		toolCell("HUD Layout Tool", DEBUG_TOOL::UI);
#else
		toolCell("Optimization Benchmark", DEBUG_TOOL::RENDERING);
#endif
#ifdef _DEBUG
		toolCell("Equipment Authoring Tool", DEBUG_TOOL::EQUIPMENT);
#endif
		ImGui::EndTable();
	}
	if (ImGui::Button("Close All Tools"))
		CloseAllDebugTools();
#ifdef _DEBUG
	constexpr std::array<std::pair<DEBUG_TOOL, const char_t*>, 19>
		TOOL_FOCUS_OPTIONS = {{
			{ DEBUG_TOOL::MAP, "Map Tool" },
			{ DEBUG_TOOL::WORLD_OBJECT, "Action Workbench / Object" },
			{ DEBUG_TOOL::WORLD_LEVEL, "Open World Level Tool" },
            { DEBUG_TOOL::WORLD_SCENE, "World Scene Tool" },
			{ DEBUG_TOOL::GUIDE_AI, "DimensionMaster Guide" },
			{ DEBUG_TOOL::MAHARAKA_AI, "Waterpang AI Tool" },
			{ DEBUG_TOOL::SEQUENCER, "Action Workbench" },
			{ DEBUG_TOOL::SEQUENCER_BENCHMARK, "Action Workbench / Sequence" },
			{ DEBUG_TOOL::ANIMATION, "Animation Clip Tool" },
			{ DEBUG_TOOL::EFFECT, "Effect Tool V1" },
            { DEBUG_TOOL::EFFECT_V2, "Effect Tool V2" },
			{ DEBUG_TOOL::RENDERING, "Rendering Workbench" },
			{ DEBUG_TOOL::PROFILER, "Composition Profiler" },
			{ DEBUG_TOOL::UI, "HUD Layout Tool" },
			{ DEBUG_TOOL::BALANCE, "Balance Test" },
			{ DEBUG_TOOL::VALTAN_BOSS, "Valtan Boss Tool" },
			{ DEBUG_TOOL::KOUKU_SAYDON_BOSS, "KoukuSaydon Boss Tool" },
			{ DEBUG_TOOL::CAMERA, "Camera Tool" },
			{ DEBUG_TOOL::EQUIPMENT, "Equipment Authoring Tool" },
		}};
	const char_t* pInputOwnerLabel = "None";
	for (const auto& [eTool, pLabel] : TOOL_FOCUS_OPTIONS)
	{
		if (eTool == m_eDebugInputOwner)
		{
			pInputOwnerLabel = pLabel;
			break;
		}
	}
	ImGui::SetNextItemWidth(310.f);
	if (ImGui::BeginCombo("Explicit viewport/preview owner", pInputOwnerLabel))
	{
		for (const auto& [eTool, pLabel] : TOOL_FOCUS_OPTIONS)
		{
			if (!IsDebugToolVisible(eTool))
				continue;
			const bool_t isSelected = eTool == m_eDebugInputOwner;
			if (ImGui::Selectable(pLabel, isSelected))
			{
				m_eDebugInputOwner = eTool;
				m_eDebugWindowFocusPending = eTool;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (!isMapEditorWorkspace && !Find_ActiveMapAuthoringHost() &&
		IsDebugToolVisible(DEBUG_TOOL::MAP))
	{
		ImGui::TextDisabled(
			"Map Tool has no live Area to attach here. Enter its owning Level to inspect or edit the loaded map.");
	}

	RenderArenaCameraAndPlayerControls();
	if (!CLevel_KoukuSaydonArena::Get_Active()) RenderHUDBarPositionControls();
#else
	RenderBalanceTestLauncher();
	RenderHUDBarPositionControls();
#endif
	if (ImGui::CollapsingHeader("Battle Items"))
	{
		CPlayerController* controller = Find_ActivePlayerController();
		const auto& player = CCombatHUDViewModel::Get().Get_Player();
		const bool ready = controller && player.isValid && !player.isPreview;
		ImGui::BeginDisabled(!ready);
		const auto give = [&](const char* itemId)
		{
			const auto sequence = m_iNextDebugGiveItemSequence++;
			if (!m_iNextDebugGiveItemSequence) m_iNextDebugGiveItemSequence = 1u;
			return controller && controller->Request_DebugGiveItem(sequence, itemId, 10u);
		};
		if (ImGui::Button("Give all four (10 each)"))
		{
			unsigned sent = 0u;
			for (const auto& binding : BATTLE_ITEM_EFFECTS) if (give(binding.itemId)) ++sent;
			m_strDebugItemStatus = "Requested " + std::to_string(sent) + " of 4 item stacks. Check inventory.";
		}
		for (const auto& binding : BATTLE_ITEM_EFFECTS)
		{
			const auto* item = CItemCatalog::Find_ById(binding.itemId);
			if (!item) continue;
			const auto label = item->strDisplayName + " x10##" + binding.itemId;
			if (ImGui::Button(label.c_str()))
				m_strDebugItemStatus = give(binding.itemId) ? "Requested " + item->strDisplayName + " x10." : "Item request was not sent.";
		}
		ImGui::EndDisabled();
		if (!ready) ImGui::TextDisabled("Enter a Server world first.");
		ImGui::TextWrapped("Open inventory (I), then drag the four items into HUD slots 1 / 2 / 3 / 4. Close F1 and press the matching number to use.");
		ImGui::TextWrapped("Aim bombs at the ground and Holy Charm at another party member. Time Stop protects yourself for 3 seconds. Item cooldowns still apply.");
		if (!m_strDebugItemStatus.empty()) ImGui::TextWrapped("%s", m_strDebugItemStatus.c_str());
	}
	if (currentLevelId != ETOUI(LEVEL::CHARACTER_SELECT))
		Render_CameraSpeedControls();
#ifdef _DEBUG
	// 초안은 panel이 소유하고 현재 player/controller/camera는 이 호출 동안만 차용한다.
	if (!m_pVehicleDebugPanel)
	{
		m_pVehicleDebugPanel = std::make_unique<CVehicleDebugPanel>();
	}
	{
		const std::shared_ptr<CCamera_Free> pVehicleDebugCamera = Find_ActiveDebugCamera();
		m_pVehicleDebugPanel->Render(CCombatHUDViewModel::Get().Get_Player(),
			Find_ActivePlayerController(), pVehicleDebugCamera.get());
	}
#endif
	CLevel_Development::Render_ColosseumIntroControls();
	ImGui::TextWrapped("%s", m_strToolStatus.c_str());
	if (ImGui::CollapsingHeader("Character Select Movie", ImGuiTreeNodeFlags_DefaultOpen))
		CLevel_CharacterSelect::Render_ClassSelectMovieControls();
	RenderKoukuSaydonArenaControls();
	RenderValtanArenaControls();
	RenderCompletePlayControls();
	RenderKoukuSaydonCompletePlayControls();
#ifdef _DEBUG
	RenderServerArenaActiveControls();

	ImGui::SeparatorText("Diagnostics");
	const ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("FPS: %.1f  |  Frame: %.2f ms",
		io.Framerate,
		io.DeltaTime > 0.f ? io.DeltaTime * 1000.f : 0.f);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(
			"FPS is ImGui's rolling average; Frame is the latest frame time.");
	bool_t profilerVisible = m_bProfilerVisible;
	if (ImGui::Checkbox("Profiler", &profilerVisible))
	{
		m_bProfilerVisible = profilerVisible;
		if (Engine::CProfiler* pProfiler =
			CGameInstance::Get().Get_Profiler())
		{
			if (m_bProfilerVisible)
				pProfiler->Reset_History();
			pProfiler->Set_Enabled(m_bProfilerVisible);
		}
	}
	ImGui::SeparatorText("Replicated Buffs");
	{
		const HUD_PLAYER_STATE& buffPlayer = CCombatHUDViewModel::Get().Get_Player();
		ImGui::Text("valid=%d  tick=%u  shield=%u  buffs=%u  icons=%u",
			buffPlayer.isValid ? 1 : 0, buffPlayer.iServerTick,
			buffPlayer.iShield, static_cast<uint32_t>(buffPlayer.iActiveBuffCount),
			static_cast<uint32_t>(m_HudSkillBuffIcons.size()));
		for (size_t i = 0; i < buffPlayer.iActiveBuffCount &&
			i < LostArk::Shared::MAX_ACTIVE_BUFFS; ++i)
		{
			const auto Found =
				m_HudSkillBuffIcons.find(buffPlayer.ActiveBuffs[i].iBuffId);
			ImGui::Text("  buff %u  end=%u  icon=%s", buffPlayer.ActiveBuffs[i].iBuffId,
				buffPlayer.ActiveBuffs[i].iEndTick,
				m_HudSkillBuffIcons.end() == Found ? "none" : Found->second.c_str());
		}
	}
	ImGui::SeparatorText("Live Combat Geometry");
	Client::COMBAT_DEBUG_VISIBILITY_SNAPSHOT CombatDebug =
		Client::CClientReplication::Get_GlobalCombatDebugVisibility();
	bool_t bCombatDebugChanged = false;
	if (ImGui::Checkbox(
			"Boss Body Collider", &CombatDebug.bBossBodyCollider))
	{
		bCombatDebugChanged = true;
	}
	if (ImGui::Checkbox(
			"Boss Pattern Hit Pulse", &CombatDebug.bBossPatternHitPulse))
	{
		bCombatDebugChanged = true;
	}
	if (ImGui::Checkbox(
			"Boss Stage Geometry (whole Stage)",
			&CombatDebug.bBossStageGeometry))
	{
		bCombatDebugChanged = true;
	}
	if (ImGui::Checkbox(
			"Combat Object Hit", &CombatDebug.bCombatObjectHit))
	{
		bCombatDebugChanged = true;
	}
	if (ImGui::Checkbox(
			"Counter Proxy", &CombatDebug.bCounterProxy))
	{
		bCombatDebugChanged = true;
	}
	if (ImGui::Checkbox(
			"Player Skill Hit Geometry",
			&CombatDebug.bPlayerSkillHitGeometry))
	{
		bCombatDebugChanged = true;
	}
	if (bCombatDebugChanged)
	{
		Client::CClientReplication::Set_GlobalCombatDebugVisibility(
			CombatDebug);
		CombatDebug =
			Client::CClientReplication::Get_GlobalCombatDebugVisibility();
	}
	ImGui::TextDisabled(
		"Global revision %llu. Pink=pulse, amber=Stage, green=combat object, cyan=counter. Authoring preview remains independent.",
		static_cast<unsigned long long>(CombatDebug.iRevision));
	ImGui::SeparatorText("Inventory (Debug)");
	const std::vector<Client::ITEM_DEFINITION>& debugItems =
		Client::CItemCatalog::Get_Items();
	CNetworkManager& debugNetworkManager = CNetworkManager::Get();
	const bool_t canGiveItem = !debugItems.empty() &&
		debugNetworkManager.Is_Connected() &&
		LostArk::Shared::INVALID_PLAYER_ID !=
			debugNetworkManager.Get_LocalPlayerId();
	if (debugItems.empty())
	{
		ImGui::TextDisabled("Item catalog failed to load.");
	}
	else
	{
		if (m_iSelectedDebugItemIndex >=
			static_cast<int32_t>(debugItems.size()))
		{
			m_iSelectedDebugItemIndex = 0;
		}
		const Client::ITEM_DEFINITION& selectedItem =
			debugItems[m_iSelectedDebugItemIndex];
		if (ImGui::BeginCombo("Item", selectedItem.strDisplayName.c_str()))
		{
			for (int32_t index = 0;
				index < static_cast<int32_t>(debugItems.size()); ++index)
			{
				const bool_t isSelected = index == m_iSelectedDebugItemIndex;
				if (ImGui::Selectable(
					debugItems[index].strDisplayName.c_str(), isSelected))
				{
					m_iSelectedDebugItemIndex = index;
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::BeginDisabled(!canGiveItem);
		if (ImGui::Button("Give"))
		{
			CPlayerController* controller = Find_ActivePlayerController();
			if (controller && controller->Request_DebugGiveItem(
				m_iNextDebugGiveItemSequence, selectedItem.strItemId, 1u))
			{
				m_strDebugItemStatus = "Requested " + selectedItem.strDisplayName;
				++m_iNextDebugGiveItemSequence;
			}
			else
			{
				m_strDebugItemStatus = "Give item request failed to send.";
			}
		}
		ImGui::EndDisabled();
		if (!canGiveItem)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("Connect and enter a world first.");
		}
		if (!m_strDebugItemStatus.empty())
			ImGui::TextDisabled("%s", m_strDebugItemStatus.c_str());
	}

	ImGui::TextUnformatted("Current inventory (Server truth):");
	const LostArk::Shared::S2C_INVENTORY_SNAPSHOT& debugInventory =
		Client::CCombatHUDViewModel::Get().Get_Inventory();
	if (debugInventory.Items.empty())
	{
		ImGui::TextDisabled("(empty)");
	}
	else
	{
		for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item :
			debugInventory.Items)
		{
			const Client::ITEM_DEFINITION* definition =
				Client::CItemCatalog::Find_ById(item.strItemId);
			ImGui::Text(
				"%s x%u",
				nullptr != definition ?
					definition->strDisplayName.c_str() : item.strItemId.c_str(),
				item.iQuantity);
		}
	}

	if (ImGui::CollapsingHeader("Vehicle Riding (Debug)"))
	{
		ImGui::TextDisabled(
			"H mounts the selected vehicle. A class without its rider pose uses the first catalog vehicle.");
		const HUD_PLAYER_STATE& ridingPlayer = CCombatHUDViewModel::Get().Get_Player();
		const std::uint32_t preferredVehicleId = CPlayerController::Get_PreferredVehicleId();
		if (ImGui::RadioButton("First available##VehicleRiding", 0u == preferredVehicleId))
			CPlayerController::Set_PreferredVehicleId(0u);
		for (const VEHICLE_ACTOR_ENTRY& vehicle : CActorCatalog::Get_Vehicles())
		{
			const bool_t hasRider = ridingPlayer.isValid &&
				nullptr != vehicle.Find_Rider(ridingPlayer.eCharacterClass);
			const std::string label = vehicle.archetypeId + "  (" +
				std::to_string(vehicle.vehicleId) + ")" + (hasRider ? "" : "  - no rider pose") +
				"##VehicleRiding" + std::to_string(vehicle.vehicleId);
			if (ImGui::RadioButton(label.c_str(), vehicle.vehicleId == preferredVehicleId))
				CPlayerController::Set_PreferredVehicleId(vehicle.vehicleId);
		}
		ImGui::TextDisabled("Riding now: %u", ridingPlayer.iVehicleId);
	}

	RenderSequenceViewer();

	if (ImGui::CollapsingHeader("Esther Skill (Debug)"))
	{
		ImGui::TextDisabled(
			"Summon asks the Server for that Esther by name: no gauge, no roster slot,"
			" same cast and summon as Ctrl+Z/X/C. Needs a live player standing idle.");
		CPlayerController* const pEstherController = Find_ActivePlayerController();
		ImGui::BeginDisabled(nullptr == pEstherController);
		const auto summonButton = [pEstherController](
			const char_t* pLabel, const LostArk::Shared::ESTHER_ID esther)
		{
			if (ImGui::Button(pLabel) && nullptr != pEstherController)
				(void)pEstherController->Request_DebugUseEsther(esther);
		};
		summonButton("Summon Sillian", LostArk::Shared::ESTHER_ID::SILLIAN);
		ImGui::SameLine();
		summonButton("Summon Wei", LostArk::Shared::ESTHER_ID::WEI);
		ImGui::SameLine();
		summonButton("Summon Bahuntur", LostArk::Shared::ESTHER_ID::BAHUNTUR);
		ImGui::SameLine();
		summonButton("Summon Ninav", LostArk::Shared::ESTHER_ID::NINAV);
		ImGui::SameLine();
		summonButton("Summon Inanna", LostArk::Shared::ESTHER_ID::INANNA);
		ImGui::EndDisabled();
		if (nullptr == pEstherController)
			ImGui::TextDisabled("No Server-driven player in this level.");

		ImGui::TextDisabled(
			"Preview replays the full-screen cutin movie (NpcCatalog cutinMovie) once.");
		const auto previewButton = [](
			const char_t* pLabel, const char_t* pArchetypeId)
		{
			if (ImGui::Button(pLabel))
				CEstherCutinPresentationService::Debug_Preview(pArchetypeId);
		};
		previewButton("Preview Sillian", "NPC_59030");
		ImGui::SameLine();
		previewButton("Preview Wei", "NPC_58700");
		ImGui::SameLine();
		previewButton("Preview Bahuntur", "NPC_59060");
	}

	if (ImGui::CollapsingHeader("Raid Boss Showcase (Debug)"))
	{
		RAID_BOSS_SHOWCASE_TUNING& showcaseTuning =
			CRaidBossShowcaseService::Debug_Tuning();
		ImGui::DragFloat("Yaw (deg)##Showcase",
			&showcaseTuning.fModelYawDegrees, 1.f, -360.f, 360.f);
		ImGui::DragFloat("Eye X / Height##Showcase",
			&showcaseTuning.fEyeXPerHeight, 0.01f, -2.f, 2.f);
		ImGui::DragFloat("Eye Y / Height##Showcase",
			&showcaseTuning.fEyeYPerHeight, 0.01f, -1.f, 3.f);
		ImGui::DragFloat("Distance / Height##Showcase",
			&showcaseTuning.fDistancePerHeight, 0.02f, 0.3f, 6.f);
		ImGui::DragFloat("Target Y / Height##Showcase",
			&showcaseTuning.fAtYPerHeight, 0.01f, 0.f, 2.f);
		ImGui::DragFloat("FOV (deg)##Showcase",
			&showcaseTuning.fFovDegrees, 0.5f, 10.f, 90.f);
		ImGui::DragFloat4("Rect X/Y/W/H (720p)##Showcase",
			&showcaseTuning.fRectX, 2.f, -400.f, 1600.f);
		if (ImGui::Button("Reset Tuning##Showcase"))
			CRaidBossShowcaseService::Debug_ResetTuning();
		ImGui::TextDisabled(
			"Open the raid entry popup on its Valtan tab to see the live"
			" boss; values apply immediately.");
		ImGui::Text(
			"yaw %.1f  eye(%.2f, %.2f)  dist %.2f  target %.2f  fov %.1f\n"
			"rect (%.0f, %.0f, %.0f, %.0f)",
			showcaseTuning.fModelYawDegrees,
			showcaseTuning.fEyeXPerHeight,
			showcaseTuning.fEyeYPerHeight,
			showcaseTuning.fDistancePerHeight,
			showcaseTuning.fAtYPerHeight,
			showcaseTuning.fFovDegrees,
			showcaseTuning.fRectX,
			showcaseTuning.fRectY,
			showcaseTuning.fRectWidth,
			showcaseTuning.fRectHeight);
	}

#endif
	ImGui::TextDisabled("F1: Developer Tools  |  F6: Follow/Free Camera  |  F7: Profiler");
	ImGui::End();
}

void CMainApp::RenderOptimizationBenchmarkPanel()
{
	if (!m_pRenderingBenchmark || !m_bRenderingQualityWindowVisible) return;
	ImGui::SetNextWindowSize(ImVec2(960.f, 640.f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Optimization Benchmark###RenderingOptimizationBenchmarkV1",
		&m_bRenderingQualityWindowVisible))
	{
		ImGui::Text("Scene: %s", m_RenderingProfiles.Get_ActiveProfileId().c_str());
		m_pRenderingBenchmark->Render_SessionBar(m_RenderingProfiles);
		m_pRenderingBenchmark->Render_OptimizationSection(m_RenderingProfiles);
	}
	ImGui::End();
	if (!m_bRenderingQualityWindowVisible)
		SetDebugToolVisible(DEBUG_TOOL::RENDERING, false);
}

#ifdef _DEBUG
void CMainApp::RenderRenderingWorkbench()
{
	Engine::CProfilerScope panelScope(CGameInstance::Get().Get_Profiler(), "ImGui.Tool.Rendering.Build");
    const bool experimentOwned = m_pRenderingBenchmark && m_pRenderingBenchmark->Is_ExperimentActive();
    const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
    if (m_iRenderingLastLevel != currentLevel)
    {
        m_iRenderingLastLevel = currentLevel;
        if (const auto* descriptor = CLevelRegistry::Find(static_cast<LEVEL>(currentLevel)))
        {
            m_eRenderingSelectedLevel = descriptor->eLevel;
            m_strRenderingSelectedProfileId = descriptor->pRenderingProfileId;
            m_bRenderQualityDraftInitialized = false;
        }
    }
    const string previousQualityOwner = m_strRenderingQualityProfileId;
    if (const auto* descriptor = CLevelRegistry::Find(m_eRenderingSelectedLevel))
    {
        m_strRenderingQualityProfileId = descriptor->pRenderingProfileId;
        const auto* selected = m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId);
        const auto* active = m_RenderingProfiles.Get_ActiveProfile();
        // Mario처럼 자체 quality override를 가진 장면은 해당 장면의 품질 프로필을 사용한다.
        // 현재 Level의 기본 프로필을 선택해 보는 중이면 활성 장면의 override를 우선한다.
        if (m_strRenderingSelectedProfileId == descriptor->pRenderingProfileId &&
            currentLevel == ETOUI(m_eRenderingSelectedLevel) && active && active->bHasQualityOverride)
            m_strRenderingQualityProfileId = active->strProfileId;
        else if (selected && selected->bHasQualityOverride)
            m_strRenderingQualityProfileId = selected->strProfileId;
    }
    if (previousQualityOwner != m_strRenderingQualityProfileId)
        m_bRenderQualityDraftInitialized = false;
    if (m_strRenderingSelectedProfileId.empty())
        m_strRenderingSelectedProfileId = m_RenderingProfiles.Get_ActiveProfileId();
    const auto syncDraft = [this]()
    {
        if (const auto* profile = m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId))
        {
            m_SceneRenderingDraft = *profile;
            m_RenderQualityDraft = m_RenderingProfiles.Get_ProfileQuality(m_strRenderingQualityProfileId);
            m_strRenderingDraftProfileId = profile->strProfileId;
            m_bRenderQualityDraftInitialized = true;
        }
    };
    if (!m_bRenderQualityDraftInitialized || m_strRenderingDraftProfileId != m_strRenderingSelectedProfileId)
        syncDraft();
    const bool resetLayout = m_bResetLightingLayout;
    m_bResetLightingLayout = false;
    const auto beginPane = [this, resetLayout](const char* title, bool_t& visible, const int pane)
    {
        if (!visible) return false;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 origin = viewport->WorkPos;
        const ImVec2 available = viewport->WorkSize;
        constexpr float margin = 8.f;
        constexpr float gap = 8.f;
        const float width = (std::max)(1.f, available.x - margin * 2.f - gap * 2.f);
        const float height = (std::max)(1.f, available.y - margin * 2.f);
        const float leftWidth = width * .24f;
        const float rightWidth = width * .28f;
        const float centerWidth = width - leftWidth - rightWidth;
        const float rightX = origin.x + margin + leftWidth + gap + centerWidth + gap;
        ImVec2 position(origin.x + margin, origin.y + margin);
        ImVec2 size(leftWidth, height);
        if (pane == 1)
        {
            position = ImVec2(rightX, origin.y + margin);
            size = ImVec2(rightWidth, height * .48f);
        }
        else if (pane == 2)
        {
            position = ImVec2(origin.x + margin + leftWidth + gap, origin.y + margin + height * .66f);
            size = ImVec2(centerWidth, height * .34f);
        }
        else if (pane == 3)
        {
            position = ImVec2(rightX, origin.y + margin + height * .48f + gap);
            size = ImVec2(rightWidth, height * .52f - gap);
        }
        const ImGuiCond condition = resetLayout ? ImGuiCond_Always : ImGuiCond_FirstUseEver;
        ImGui::SetNextWindowPos(position, condition);
        ImGui::SetNextWindowSize(size, condition);
        const bool expanded = ImGui::Begin(title, &visible, ImGuiWindowFlags_MenuBar);
        if (expanded && ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Windows"))
            {
                ImGui::MenuItem("Light Resources", nullptr, &m_bLightResourcesWindowVisible);
                ImGui::MenuItem("Light Detail", nullptr, &m_bLightDetailWindowVisible);
                ImGui::MenuItem("Light Sequencer", nullptr, &m_bLightSequencerWindowVisible);
                ImGui::MenuItem("Rendering Workbench", nullptr, &m_bRenderingQualityWindowVisible);
                if (ImGui::MenuItem("Show All"))
                    m_bLightResourcesWindowVisible = m_bLightDetailWindowVisible =
                        m_bLightSequencerWindowVisible = m_bRenderingQualityWindowVisible = true;
                if (ImGui::MenuItem("Reset Layout"))
                {
                    m_bResetLightingLayout = true;
                    m_bLightResourcesWindowVisible = m_bLightDetailWindowVisible =
                        m_bLightSequencerWindowVisible = m_bRenderingQualityWindowVisible = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        if (!expanded) ImGui::End();
        return expanded;
    };
    if (beginPane("Light Resources###LightResourcesWindowV1", m_bLightResourcesWindowVisible, 0))
    {
        ImGui::BeginDisabled(experimentOwned);
        static constexpr LEVEL levels[] = { LEVEL::LOBBY, LEVEL::CHARACTER_SELECT, LEVEL::BERN,
            LEVEL::VALTAN_ARENA, LEVEL::KOUKUSAYDON_ARENA, LEVEL::DEVELOPMENT };
        static constexpr const char* names[] = { "Lobby", "Character Select", "Bern", "Valtan", "KoukuSaydon", "Development" };
        int selectedLevel = 0;
        for (int i = 0; i < 6; ++i) if (levels[i] == m_eRenderingSelectedLevel) selectedLevel = i;
        if (ImGui::Combo("Level category", &selectedLevel, names, 6))
        {
            m_eRenderingSelectedLevel = levels[selectedLevel];
            if (const auto* descriptor = CLevelRegistry::Find(m_eRenderingSelectedLevel))
                m_strRenderingSelectedProfileId = m_strRenderingQualityProfileId = descriptor->pRenderingProfileId;
            StopLightingPreview();
            syncDraft();
        }
        RenderLightingWorkbench();
        ImGui::EndDisabled();
        ImGui::End();
    }
    if (beginPane("Light Detail###LightDetailWindowV1", m_bLightDetailWindowVisible, 1))
    {
        ImGui::BeginDisabled(experimentOwned);
        RenderLightDetail();
        ImGui::EndDisabled();
        ImGui::End();
    }
    if (beginPane("Light Sequencer###LightSequencerWindowV1", m_bLightSequencerWindowVisible, 2))
    {
        ImGui::BeginDisabled(experimentOwned);
        RenderLightSequencer();
        ImGui::EndDisabled();
        ImGui::End();
    }
    if (beginPane("Rendering Workbench###RenderingQualityWindowV1", m_bRenderingQualityWindowVisible, 3))
    {
        const auto* pActiveProfile = m_RenderingProfiles.Get_ActiveProfile();
        if (!pActiveProfile || !m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId))
        { ImGui::TextWrapped("Selected rendering profile is unavailable."); ImGui::End(); return; }
        ImGui::Text("Scene: %s", m_RenderingProfiles.Get_ActiveProfileId().c_str());
        if(m_pRenderingBenchmark) m_pRenderingBenchmark->Render_SessionBar(m_RenderingProfiles);
        if(ImGui::BeginTabBar("RenderingWorkbenchTabs"))
        {
            if(ImGui::BeginTabItem("Restoration"))
            {
                if(m_pRenderingBenchmark) m_pRenderingBenchmark->Render_PresentationSection(m_RenderingProfiles);
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Technique A/B"))
            {
                if(m_pRenderingBenchmark) m_pRenderingBenchmark->Render_QuickComparison(m_RenderingProfiles);
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("최적화 A/B"))
            {
                if(m_pRenderingBenchmark) m_pRenderingBenchmark->Render_OptimizationSection(m_RenderingProfiles);
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Measure / Analyze"))
            {
                if(m_pRenderingBenchmark) m_pRenderingBenchmark->Render_Section(
                    CGameInstance::Get().Get_Profiler(), m_RenderingProfiles.Get_ActiveProfileId(), m_RenderingProfiles);
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Saved Settings"))
            {
        ImGui::TextWrapped("Edits here change the selected scene draft. Use Restoration and Technique A/B for temporary comparisons.");

    const bool experimentActive = m_pRenderingBenchmark && m_pRenderingBenchmark->Is_ExperimentActive();
    if (experimentActive) ImGui::TextWrapped("Session experiment active. End the experiment before editing or saving authored quality.");
    ImGui::BeginDisabled(experimentActive);
	const auto applyGlobal = [this]()
	{
		if (const auto* source = m_RenderingProfiles.Find_Profile(m_strRenderingQualityProfileId))
		{
			auto profile = *source;
			profile.bHasQualityOverride = true;
			profile.QualityOverride = m_RenderQualityDraft;
			if (m_RenderingProfiles.Update_Profile(profile, m_strRenderingStatus))
			{
				auto comparison = m_RenderingProfiles.Get_ComparisonOptions();
				comparison.bFXAAEnabled = profile.QualityOverride.bFXAAEnabled;
				(void)m_RenderingProfiles.Set_ComparisonOptions(comparison);
			}
		}
		m_RenderQualityDraft = m_RenderingProfiles.Get_ProfileQuality(m_strRenderingQualityProfileId);
		if (const auto* profile = m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId)) m_SceneRenderingDraft = *profile;
	};

    ImGui::SeparatorText("Live rendering comparison");
    auto comparison = m_RenderingProfiles.Get_ComparisonOptions();
    if (!comparison.bActive)
    {
        const auto& quality = CGameInstance::Get().Get_RenderQualitySettings();
        comparison.bFXAAEnabled = quality.bFXAAEnabled;
        comparison.bBloomEnabled = quality.bBloomEnabled;
    }
    ImGui::BeginDisabled(m_pRenderingBenchmark && m_pRenderingBenchmark->Is_Capturing());
    bool_t comparisonChanged = ImGui::Checkbox("Directional light##LiveCompare", &comparison.bDirectionalEnabled);
    comparisonChanged |= ImGui::SliderFloat("Exposure multiplier##LiveCompare",
        &comparison.fExposureMultiplier, 0.5f, 2.f, "%.3fx", ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Updates the current view while dragging. Ctrl+click to enter an exact multiplier.");
    comparisonChanged |= ImGui::Checkbox("LUT grading##LiveCompare", &comparison.bLutEnabled);
    ImGui::SameLine();
    // 두 FXAA 조절 UI는 같은 영구 저장 대상인 장면 품질 draft를 수정한다.
    // 비교 상태에도 그 값을 맞춰 Save/Publish 값을 가리거나 이전 FXAA 값으로 되돌리지 않게 한다.
    if (ImGui::Checkbox("FXAA (saved)##LiveCompare", &m_RenderQualityDraft.bFXAAEnabled))
    {
        applyGlobal();
        comparison.bFXAAEnabled = m_RenderQualityDraft.bFXAAEnabled;
    }
    ImGui::SameLine();
    comparisonChanged |= ImGui::Checkbox("Bloom##LiveCompare", &comparison.bBloomEnabled);
    if (comparisonChanged)
    {
        comparison.bActive = true;
        (void)m_RenderingProfiles.Set_ComparisonOptions(comparison);
    }
    if (ImGui::Button("Reset comparison##LiveCompare")) m_RenderingProfiles.Clear_ComparisonOptions();
    ImGui::EndDisabled();
    ImGui::Text("Effective exposure: %.3f", CGameInstance::Get().Get_RenderQualitySettings().fExposure);
    ImGui::TextWrapped("Exposure scales brightness; LUT grading runs once. Directional toggles diffuse/specular while ambient stays active.");
    ImGui::TextWrapped("Directional, exposure, LUT and Bloom comparisons last for this session. FXAA updates the selected scene quality and is included in Save Authored / Publish Runtime.");

	ImGui::TextDisabled("Pixel inputs and material comparisons are in the Benchmark section.");

	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	ImGui::SeparatorText("Effect Presentation");
	bool_t bEffectLights = Presentation.Are_TransientLightsEnabled();
	if (ImGui::Checkbox("Typed Effect Lights", &bEffectLights))
		Presentation.Set_TransientLightsEnabled(bEffectLights);
	ImGui::SameLine();
	bool_t bEffectPosts = Presentation.Are_ScreenPostsEnabled();
	if (ImGui::Checkbox("Typed Effect Screen Posts", &bEffectPosts))
		Presentation.Set_ScreenPostsEnabled(bEffectPosts);
	ImGui::Text("Last submitted: Light %u | Screen Post %u",
		Presentation.Get_LastTransientLightCount(),
		Presentation.Get_LastScreenPostCount());
	ImGui::TextDisabled(
		"Effect Base/Mask/Dissolve/Distortion/Emissive enter SceneHDR before these posts and Bloom.");



	bool_t globalChanged = false;
	ImGui::SeparatorText("Selected Scene Quality");
	ImGui::Text("Quality owner: %s", m_strRenderingQualityProfileId.c_str());
	globalChanged |= ImGui::Checkbox(
		"SSAO enabled (crease shadows)##SSAO", &m_RenderQualityDraft.bSSAOEnabled);
	ImGui::BeginDisabled(!m_RenderQualityDraft.bSSAOEnabled);
	globalChanged |= ImGui::DragFloat(
		"SSAO Radius (m)", &m_RenderQualityDraft.fSSAORadius,
		0.01f, 0.01f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Bias", &m_RenderQualityDraft.fSSAOBias,
		0.001f, 0.f, 1.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Intensity", &m_RenderQualityDraft.fSSAOIntensity,
		0.01f, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Power", &m_RenderQualityDraft.fSSAOPower,
		0.01f, 0.1f, 8.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"SSAO Distance Fade", &m_RenderQualityDraft.fSSAODistanceFade,
		0.25f, 1.f, 1000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"SSAO darkens ambient and PBR baked/IBL light in nearby creases. Radius is metres; intensity/power deepen it. It cannot cast a long directional shadow. True emissive remains independent.");

	ImGui::SeparatorText("Bloom / Tone / Anti-Aliasing");
	globalChanged |= ImGui::Checkbox(
		"Enabled##Bloom", &m_RenderQualityDraft.bBloomEnabled);
	ImGui::BeginDisabled(!m_RenderQualityDraft.bBloomEnabled);
	globalChanged |= ImGui::DragFloat(
		"Threshold", &m_RenderQualityDraft.fBloomThreshold,
		0.01f, 0.f, 64.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Soft Knee", &m_RenderQualityDraft.fBloomSoftKnee,
		0.005f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"Base Bloom Intensity", &m_RenderQualityDraft.fBloomIntensity,
		0.01f, 0.f, 16.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Scene bloom strength. Full Restore effects use Skill Bloom Intensity in Effect Detail.");
	globalChanged |= ImGui::DragFloat(
		"Scatter", &m_RenderQualityDraft.fBloomScatter,
		0.01f, 0.25f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Bloom spreads pixels already above Threshold; it does not replace lighting or GI.");

	globalChanged |= ImGui::DragFloat(
		"Base Exposure", &m_RenderQualityDraft.fExposure,
		0.01f, 0.01f, 32.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::BeginDisabled(m_RenderQualityDraft.SourcePostProcess.bEnabled);
	globalChanged |= ImGui::DragFloat(
		"Hable White Point", &m_RenderQualityDraft.fWhitePoint,
		0.05f, 1.f, 64.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::EndDisabled();
    ImGui::TextWrapped("Exposure scales all HDR light; it cannot restore missing normals or shadows. Hable White Point is unused with Source Tone enabled. Bloom spreads bright pixels; FXAA smooths edges and can soften small details.");
	globalChanged |= ImGui::DragFloat(
		"Display Gamma", &m_RenderQualityDraft.fGamma,
		0.005f, 1.f, 3.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

	globalChanged |= ImGui::Checkbox(
		"FXAA Enabled", &m_RenderQualityDraft.bFXAAEnabled);
	ImGui::BeginDisabled(!m_RenderQualityDraft.bFXAAEnabled);
	globalChanged |= ImGui::DragFloat(
		"FXAA Blend", &m_RenderQualityDraft.fFXAASubpixel,
		0.005f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"FXAA Edge Threshold", &m_RenderQualityDraft.fFXAAEdgeThreshold,
		0.001f, 0.0312f, 0.333f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	globalChanged |= ImGui::DragFloat(
		"FXAA Edge Threshold Min", &m_RenderQualityDraft.fFXAAEdgeThresholdMin,
		0.001f, 0.0156f, 0.0833f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::EndDisabled();
	ImGui::TextDisabled("FXAA is evaluated before display-space UI, so HUD text stays sharp.");

	if (globalChanged)
	{
		m_RenderQualityDraft.fSSAOBias = (std::min)(
			m_RenderQualityDraft.fSSAOBias,
			(std::max)(0.f, m_RenderQualityDraft.fSSAORadius - 0.0001f));
		m_RenderQualityDraft.fSSAODistanceFade = (std::max)(
			m_RenderQualityDraft.fSSAODistanceFade,
			m_RenderQualityDraft.fSSAORadius);
		applyGlobal();
	}

	ImGui::SeparatorText("Selected Quality Actions");
	if (ImGui::Button("Reset Selected Quality Defaults"))
	{
		m_RenderQualityDraft = {};
		m_RenderQualityDraft.bSSAOEnabled = false;
		applyGlobal();
	}
	ImGui::SameLine();
	if (ImGui::Button("Start Session A/B from Current View") && m_pRenderingBenchmark)
		m_pRenderingBenchmark->Start_SessionExperiment(m_RenderingProfiles);
	ImGui::SameLine();
	if (ImGui::Button("Reload Selected Draft"))
	{
		m_RenderQualityDraft = m_RenderingProfiles.Get_ProfileQuality(m_strRenderingQualityProfileId);
		if (const SCENE_RENDERING_PROFILE* pProfile =
			m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId))
		{
			m_SceneRenderingDraft = *pProfile;
			m_strRenderingDraftProfileId = pProfile->strProfileId;
		}
		m_strRenderingStatus = "Selected profile draft reloaded.";
	}

	ImGui::SeparatorText("Authoring Pipeline");
	if (ImGui::Button("Save Authored"))
		m_RenderingProfiles.Save_Authored(m_strRenderingStatus);
	ImGui::SameLine();
	if (ImGui::Button("Publish Runtime"))
		m_RenderingProfiles.Publish_Runtime(m_strRenderingStatus);
	ImGui::SameLine();
	if (ImGui::Button("Reload Runtime"))
	{
		if (m_RenderingProfiles.Reload_Runtime(m_strRenderingStatus))
		{
			if (m_pRenderingBenchmark)
				m_pRenderingBenchmark->Notify_ProfileReload();
			m_RenderQualityDraft = m_RenderingProfiles.Get_ProfileQuality(m_strRenderingQualityProfileId);
			if (const SCENE_RENDERING_PROFILE* pProfile =
				m_RenderingProfiles.Find_Profile(m_strRenderingSelectedProfileId))
			{
				m_SceneRenderingDraft = *pProfile;
				m_strRenderingDraftProfileId = pProfile->strProfileId;
			}
		}
	}
	ImGui::TextWrapped("%s", m_strRenderingStatus.c_str());
	ImGui::TextDisabled(
		"Save changes Authored only; Publish validates/promotes Runtime; Reload commits atomically.");
    ImGui::EndDisabled();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
	ImGui::End();
	}
}

#endif

void CMainApp::UpdateDebugToolShortcut()
{
	const bool_t windowFocused =
		IsWindowOwnedByCurrentProcess(GetForegroundWindow());
	const bool_t f1Down = windowFocused &&
		0 != (GetAsyncKeyState(VK_F1) & 0x8000);
	if (f1Down && !m_bF1Down && !ImGui::GetIO().WantTextInput &&
		!CUIInputRouter::Get().Is_TextInputActive())
		m_bDeveloperToolsVisible = !m_bDeveloperToolsVisible;
	m_bF1Down = f1Down;
}

// wWinMain이 Client 조정자를 준비하는 진입점이다. 엔진 관리자는 Initialize_Engine으로 만들고 제품 UI·도구는 MainApp 수명에 묶는다.
unique_ptr<CMainApp> CMainApp::Create()
{
	auto instance = unique_ptr<CMainApp>(new CMainApp());
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}

// MainApp의 제품 연출·네트워크·도구를 먼저 멈추고 GameInstance에 엔진 종료를 요청한다.
// Level 종료가 Loading worker를 합류시킨 뒤에야 worker가 참조하던 effect 준비 cache를 해제한다.
void CMainApp::Free()
{
#ifdef _DEBUG
	// 외부 publisher는 종료하지 않고 이 Client가 보유한 HANDLE만 해제한다.
	if (m_pVehicleDebugPanel)
	{
		m_pVehicleDebugPanel->Shutdown();
	}
	m_pVehicleDebugPanel.reset();
	if (m_pEffectTool)
		m_pEffectTool->Deactivate_AuthoringWorkspace();
	if (m_pEffectToolV2)
		m_pEffectToolV2->Deactivate();
	if (m_pWorldObjectTool)
		m_pWorldObjectTool->Deactivate();
	m_pWorldLevelPendingMapRequest.reset();
	m_pMapEffectPlacementRequest.reset();
	m_pWorldSceneTool.reset();
	m_pWorldLevelTool.reset();
	m_pGuideAITool.reset();
	m_pMaharakaAITool.reset();
	m_pWorldObjectTool.reset();
#endif
#ifdef _DEBUG
	if (m_pEffectTool)
		m_pEffectTool->Set_AuthoringPlayer(nullptr);
	if (m_pEffectToolV2)
		m_pEffectToolV2->Set_AuthoringPlayer(nullptr);
#endif
	if (m_pKoukuPresentationPlayer)
		m_pKoukuPresentationPlayer->Reset();
	m_pKoukuPresentationPlayer.reset();
	// 저장 외형 적용기가 보관한 장치·장비 service 참조는 엔진이 살아 있을 때 해제한다.
	CCustomizingView::Release_SavedLookCache();
	// 표시 객체는 Object_Manager가 살아 있을 때 제거한다. 준비 cache의 COM 참조는 Loading worker가 끝나는 Release_Engine 이후까지 유지한다.
	CEffectPresentationService::Clear_All();
	CNetworkManager::Get().Shutdown();
	CGameInstance::Get().Stop_LoopingSound();
	CGameInstance::Get().SetInputBlocked(false, false);
	// 엔진과 profile service가 살아 있을 때 capture를 취소하고 세션이 소유한 설정을 복원한다.
	// 숨겨진 Release 비교 측정도 같은 종료 경로로 정리한다.
	if (m_pRenderingBenchmark)
		m_pRenderingBenchmark->Shutdown(m_RenderingProfiles);
	m_pRenderingBenchmark.reset();

#ifdef _DEBUG
	m_pSequencerTool.reset();
	m_pSequenceActionWorkbench.reset();
	m_pValtanActionWorkbench.reset();
	m_pAnimationTool.reset();
	m_pCharacterActionWorkbench.reset();
	m_pEffectTool.reset();
	m_pEffectToolV2.reset();
	m_pEquipmentAuthoringTool.reset();
	if (nullptr != m_pCharacterPreviewPanel)
		m_pCharacterPreviewPanel->Release(true);
	m_pCharacterPreviewPanel.reset();
	m_pHUDLayoutTool.reset();
	m_pCameraTool.reset();
	m_pMapTool.reset();
#endif

	m_pKoukuSaydonActionWorkbench.reset();
	m_pBalanceTool.reset();
	m_pKoukuSaydonBossTool.reset();
	m_pValtanBossTool.reset();
	if (auto *profiler = CGameInstance::Get().Get_Profiler())
		profiler->Set_Enabled(false);
	m_pProfilerTool.reset();
	m_pWorldHealthBarView.reset();
	if (nullptr != m_pImGuiLayer)
		m_pImGuiLayer->Shutdown();
	m_pImGuiLayer.reset();
	// GameInstance가 Level을 먼저 소멸시켜 worker·카메라 사용을 끝낸 뒤 나머지 관리자를 해제한다.
	// 여기서 reset하지 않은 MainApp view와 장치 COM 참조는 뒤이은 멤버 소멸 때 풀린다.
	CGameInstance::Get().Release_Engine();
	CEffectPresentationService::Release_PreparedResources();
	CEffectV2Runtime::Release_Resources();
	CEffectCatalog::Clear();
}
```

## 최종 후보 검증 기록

아래 문법 컴파일 기록은 MainApp.h 빈 줄 동기화 전 후보의 결과다. 현재 전문·manifest는 빈 줄 동기화 뒤 후보와 일치한다.

- 최종 동결한 VehicleDebugPanel.cpp, CameraDebugControls.cpp, MainApp.cpp 세 후보의 Debug x64 `/Zs /Y-` 문법 컴파일 PASS.
- 같은 세 후보의 Release x64 `/Zs /Y-` 문법 컴파일 PASS. Release의 VehicleDebugPanel은 `_DEBUG` 구현이 제외된 빈 TU 확인이다.
- 신규 두 CPP는 해당 파일의 `/utf-8`, PCH NotUsing, ForcedIncludeFiles 비움 조건을 사용했다. 기존 SDK의 CP949 문자셋 경고는 남아 있다.
- 검사에 쓰인 헤더는 out의 격리 사본에 G03/G04/G05 API를 겹친 것이다. 제품 Source/EngineSDK/DLL을 바꾸거나 실제 링크하지 않았다.
- XML parse와 신규 H/CPP 각 한 번 등록, MainApp_Dragon TU 등록 제거를 확인했다. 최종 manifest 해시와 여섯 전체 코드 블록이 일치한다.
- 원본과 새 두 CPP의 UI·경로·publisher 명령행 문자열 집합이 일치한다. 제품과 선행 G04 사본은 후보 작성 중 바꾸지 않았다.
- 독립 검토에서 기능상 진행을 막는 문제는 발견하지 못했다. 숨김 중 Poll의 동기 Load 비용과 기존 실패 메시지 순서를 별도로 명시했다.
- 실제 제품 적용·링크, Save/Publish 실행·충돌·종료 중 publisher 지속, Debug/Release 화면과 Server 적용은 미검증이다.

증거는 `out/MainAppRefactor/g05/compile-results.json`, `compile-release-results.json`, `manifest.json`, `candidate.diff`다.
G05를 실제 적용한 뒤 대응 RESULT에는 실행한 검증만 기록한다.
