# SR·LostArk 비교와 Winters·NYPC 코드 탐색

> 같은 날 후속 요청은 파일의 생성·호출·소유·해제와 주석 규격을 먼저 맞추는 것이다. 아래 카메라 비교는 이전 진행으로 보존하고, 현재 시작점은 문서 끝의 「시작 파일 다섯 개와 공통 주석 규격」이다.

2026-10-14. 대표 경로의 현재 저장된 코드를 읽은 결과다. 제품 소스·셰이더·렌더링 옵션은 변경하지 않았다. 저장소 전체의 이해나 게임 실행을 완료한 기록은 아니다.

## 비교 대상

SR은 **C:/Users/tnest/Desktop/MinecraftDungeons 한 폴더**다. 별도 사본을 비교 대상으로 추가하지 않았다. 앞선 대화의 “원본과 수정된 사본”이라는 표현은 부정확했다. 해당 저장소와 C:/Users/tnest/Desktop/LostArk의 카메라·투명 정렬·클릭 이동 경로를 비교했다. 현재 구조와 사용자의 당시 개인 기여는 별개의 근거가 필요하며, 이 비교로 NYPC 탈락 원인을 판정하지 않는다.

## 실제 차이와 첫 학습 단위

| 경로 | SR의 표본 | LostArk의 표본 | 직접 설명할 내용 |
|---|---|---|---|
| 추적 카메라 | 대상 위치 + offset으로 eye·at 계산 | 약한 대상 참조, snap·지수 보간, LookAt, View 제출 | 현재·이전 값의 저장 이유와 최종 갱신 순서 |
| 투명 정렬 | list의 객체 포인터, AddRef/Release, 거리 정렬 | shared_ptr 큐, 비소유 임시 항목, priority·거리 제곱 정렬 | 정렬 키와 프레임 동안의 객체 수명 |
| 클릭 이동 | 목표 방향에 speed·dt를 곱해 위치 갱신 | 명령 전송, 서버 검증·경로 계산, snapshot 표현 | 입력 요청·확정 상태·표시 예측의 구별 |

SR의 강점은 계산과 상태 변경을 가까운 코드에서 추적하기 쉽다는 점이다. LostArk에도 호출자·단위·실패 경계를 설명하는 한국어 주석이 있다. 주석 유무만의 차이보다, 수명·최종 갱신 순서·서버 권위 등 추적할 책임이 늘어난 점이 크다. 필요한 분리와 불필요한 읽기 비용은 기능별로 구별한다.

첫 단위는 일반 추적 카메라다. 드래곤·탈것·shake·컷신의 세부 구현은 다음 범위로 두고 다섯 묶음만 읽는다.

1. [SR CDynamicCamera.cpp](C:/Users/tnest/Desktop/MinecraftDungeons/Client/Code/CDynamicCamera.cpp:70): 142행 eye와 172행 at의 최종 대입까지 확인한다.
2. [LostArk Camera_Free.h](C:/Users/tnest/Desktop/LostArk/Client/Public/Camera_Free.h:109): weak_ptr 대상, initialized, 현재 look-at, 이전 대상 위치, response의 저장 이유를 찾는다.
3. [Camera_Free.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Camera_Free.cpp:337): lock 실패, 유한성 검사, snap, 보간, 이전 값 저장을 잇는다. Late_Update 99행에서 최종 제출 순서도 확인한다.
4. [Transform.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/Transform.cpp:168): LookAt에서 LOOK·RIGHT·UP 기저를 만드는 계산을 그린다.
5. [Camera.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/Camera.cpp:218): CameraWorld의 역행렬을 View로 제출하고 Projection을 만드는 입력을 확인한다.

위치와 offset은 같은 월드 좌표계의 길이, dt는 초, response는 1/초다.

```text
desiredEye = targetPosition + positionOffset
desiredAt  = targetPosition + lookOffset
alpha      = -expm1(-response * dt) = 1 - exp(-response * dt)
eyeNext    = eye + alpha * (desiredEye - eye)
atNext     = at  + alpha * (desiredAt  - at)
```

이미 초기화된 상태에서 eye.x=0, desiredEye.x=10, response=18, dt=1/60이면 alpha≈0.259182, 다음 x≈2.591818이다. 이는 식의 계산 예이며 성능 실측이 아니다. 최초 갱신·대상 불연속·response<=0이면 현재 코드는 보간 없이 snap한다. response를 18에서 9로 줄이면 느리게 따라오지만, 0이면 즉시 목표로 간다. 식뿐 아니라 분기까지 알아야 수정 결과를 예상할 수 있다.

weak_ptr는 카메라가 플레이어 수명을 강제로 연장하지 않게 하고, lock은 현재 호출 동안의 사용을 보장한다. 현재 eye/look-at은 다음 보간의 시작점이고, 이전 대상 위치는 순간 이동 판정에 필요하다. initialized는 과거 출력이 없는 최초 프레임을 구별한다. desiredEye/At은 현재 입력으로 다시 계산하는 임시 값이다.

진행은 **설명 → 작은 숫자로 손계산 → 조건 하나 변경 → 실제 분기와 대조 → 직접 수정·검증**이다. 첫 직접 구현은 follow 계산만 작은 실험으로 옮겨 dt·response·초기화 조건을 바꾸는 범위가 적합하다. 이번에는 실험이나 제품 코드를 대신 구현하지 않았다.

SR도 주석과 최종 쓰기를 대조한다. CDynamicCamera.cpp 155행과 165행의 중간 look-at 계산은 172행에서 다시 덮인다. 부모 카메라가 View를 먼저 제출하고 eye·at를 나중에 계산하는 순서도 확인했다. 실제 화면 지연 여부는 실행하지 않았으므로 확정하지 않는다. 되찾을 것은 SR의 구조 자체보다 최종 결과까지 직접 추적하는 능력이다.

## 다음 두 단위

투명 정렬은 [SR Renderer](C:/Users/tnest/Desktop/MinecraftDungeons/Engine/Code/CRenderer.cpp:14)와 [LostArk Render_Blend](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:2335)를 비교한다. SR의 Compute_ViewZ는 이름과 달리 카메라까지의 거리다. LostArk의 [BlendSortBefore](C:/Users/tnest/Desktop/LostArk/Engine/Public/BlendSortKey.h:17)는 priority 오름차순·거리 제곱 내림차순이다. 동률 제출 순서 보존은 stable_sort가 담당한다. 임시 raw pointer를 쓰는 동안 원래 shared_ptr 큐가 수명을 유지하는 관계까지 읽는다. 포인터 배열을 쓴다는 이유로 객체 전체의 SoA라고 설명하지 않는다.

클릭 이동은 [SR CPlayer::Key_Input](C:/Users/tnest/Desktop/MinecraftDungeons/Client/Code/CPlayer.cpp:847), [LostArk Request_MoveToPointResolved](C:/Users/tnest/Desktop/LostArk/Client/Private/PlayerController.cpp:2250), [Server 명령 처리](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_PlayerCommands.cpp:32), [Server 이동 계산](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_PlayerSimulation.cpp:1137)을 잇는다. 서버의 `min(speed * fixedDeltaSeconds, distance)`가 목표 초과 이동을 막는 부분부터 계산한다. SR에도 별도 네트워크 코드가 있으므로 CPlayer 표본을 SR 전체의 네트워크 구조로 일반화하지 않는다.

## NYPC 탐색 솔루션

[NYPC_Study.sln](C:/Users/tnest/Desktop/NYPC/NYPC_Study.sln)은 요청대로 NYPC 폴더 안에 있다. **Mushroom·Battlefield 두 프로젝트**에서 기존 CPP를 열도록 구성했다. Mushroom CPP 371개와 HPP 1개, Battlefield CPP 514개를 기준본·제출본·작업 레인·실험·백업 필터로 연결했다. 원본은 이동·복사·수정하지 않았다. 이 솔루션에서 편집하면 원본 파일을 편집한다.

- Mushroom `00_현재 기준본`: [bots/cpp/main.cpp](C:/Users/tnest/Desktop/NYPC/mushroom/bots/cpp/main.cpp:44). 문서가 champion으로 지정했으며 사이트의 마지막 업로드 해시는 확인하지 않았다.
- Battlefield `00_최종 제출본_2026_07_09`: [submission_2100_claude_evcore2.cpp](C:/Users/tnest/Desktop/NYPC/battlefield/submission_2100_claude_evcore2.cpp:1209). 실제 MD5 `38925c95f4739efca13abadac5e3b516`이 07-09 최종 제출 기록과 일치한다.

Mushroom의 첫 단위는 `Action:44 → State:66 → InitDerived:87 → ApplyAction:628`이다. 직사각형 한 수 전후의 Owner·비트마스크·누적합·Hash 동기화를 계산한 뒤 평가식·탐색으로 넓힌다. Battlefield는 `GameState:1209 → readReady:1252 → parseResultBlock:1361 → emitCommands:6592`로 한 턴의 입력·자원 변화·출력을 읽고, 큰 decideCommands 함수에서는 행동 하나만 따라간다. 자세한 순서는 [NYPC README](C:/Users/tnest/Desktop/NYPC/study/README.md)에 있다.

Utility 프로젝트로 모든 CPP를 빌드에서 제외했다. 독립 제출본들의 main을 함께 링크하지 않는다. 기존 파일 3,784개 SHA256 보존, XML·상대 경로·필터·범위·중복 검사, MSBuild의 두 ValidateStudy와 솔루션 구성 평가를 통과했다. 부모 작업에서도 verify-only를 다시 실행해 원본 보존과 개수를 확인했다. 알고리즘 컴파일·실행·성능 검증은 하지 않았다. 별도 Visual Studio 프로세스로 열고 NYPC_Study 창 제목을 확인했다. 개별 CPP의 화면과 IntelliSense 전체는 검증하지 않았다.

## Winters 탐색 솔루션

[Winters.CodeStudy.sln](C:/Users/tnest/Desktop/WintersEngine/Winters.CodeStudy.sln)은 16개 Utility 프로젝트와 실제 폴더 경로 필터로 구성했다. 파일 3,066개 중 CPP는 473개다. 기존 Winters.sln과 제품 소스를 보존했다. [Winters README](C:/Users/tnest/Desktop/WintersEngine/CodeStudy/README.md)에 범위와 사용법을 적었다.

프레임 흐름은 `Client main.cpp:239 → WintersRun:35 → CEngineApp::Run:739 → Update:873/Render:890`이다. 첫 자료구조 실습은 [ComponentStore.h](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/ECS/ComponentStore.h:19)의 Add·Remove·Get이다. 엔티티 [10,20,30]에서 20을 삭제하면 마지막 30이 dense[1]로 이동하고 sparse[30]도 1로 바뀌는 과정을 그린다. vector<T> 저장소이므로 T의 필드까지 분리한 SoA라고 단정하지 않는다.

XML·경로·필터 집합·원본 해시, 16개 Utility/v145 평가와 열람 Build를 검증했다. CPP가 제외된 Utility Build이므로 제품 빌드 성공을 뜻하지 않는다. 실제 define·PCH·SDK 전체를 재현한 IntelliSense 구성은 아니다. 상세 검증은 [Winters RESULT](C:/Users/tnest/Desktop/WintersEngine/.md/plan/2026-10-14_CODE_STUDY_SOLUTION_RESULT.md)를 따른다.

## 완료와 다음 경계

완료한 것은 대표 코드의 정적 비교, 첫 계산 단위의 선정, 탐색 솔루션 생성과 구조 검증이다. 사용자의 독립 설명·수정·실행 능력을 이미 확인한 것으로 기록하지 않는다. 취업 자료 완성이라는 목적은 유지하며, 전체 프로젝트 해부를 기술소개서 완성의 새 선행 조건으로 만들지 않는다.

## 시작 파일 다섯 개와 공통 주석 규격

2026-10-14 후속 요청. MinecraftDungeons에서 사용자가 설명한 MainApp → Scene → Layer → GameObject → Component 흐름을 기준으로, LostArk와 Winters·LoL 및 NYPC Mushroom·Battlefield도 직접 읽고 설명할 수 있게 정비한다. 이번 첫 실제 범위는 LostArk의 시작 파일 다섯 개다. 다른 프로젝트 전체를 이미 정비한 것으로 기록하지 않는다. 코드 반영과 빌드 결과는 기존 [코드 정비 RESULT](../10-11/2026-10-11_CODE_READABILITY_RESULT.md)에 기록한다.

### 파일을 읽고 주석을 붙이는 기준

파일 앞에서는 이 파일이 왜 존재하는지와 외부에 제공하는 선언을 설명한다. 함수 앞에서는 직접 호출자, 입력·결과, 읽거나 변경하는 상태와 실패 후 정리를 설명한다. 멤버 옆에서는 값의 의미와 포인터의 소유자·유효 수명을 설명한다. 함수 내부에서는 준비·검사·상태 변경·제출·정리의 경계에 설명을 둔다. 실제 구현, 설명용 비유, 개선 제안과 실행 관측을 구별한다. 기존 사용자의 주석·질문·메모는 지우지 않고 보충한다.

규격을 통일한다는 것은 모든 프로젝트에 같은 상속 구조나 헤더를 넣는 것이 아니다. SR의 자체 AddRef/Release, LostArk의 unique_ptr/shared_ptr/ComPtr, Winters의 실제 객체·컴포넌트 저장소, NYPC의 입력 상태와 판단 상태를 각각 같은 질문으로 설명한다. 검증하지 않은 성능 우위나 과거 설계 의도를 주석에 넣지 않는다.

### Client.h와 Resource.h

`Client/Default/Client.h`는 현재 `#pragma once`와 `#include "resource.h"`를 가진 작은 연결 헤더다. Client라는 클래스를 만들거나 MainApp을 생성하는 파일이 아니다. pragma once는 한 번의 번역 단위 안에서 같은 헤더가 반복 포함되는 것을 막는다. 프로그램 실행을 한 번만 하거나 모든 CPP 사이에 전역 상태를 공유하는 장치가 아니다.

`Resource.h`는 숫자 ID를 정의하고 `Client.rc`는 실제 문자열·아이콘·커서·메뉴·대화 상자 리소스를 정의한다. C++과 리소스 스크립트가 같은 ID를 사용한다. `IDS_APP_TITLE`은 `LoadStringW`로 창 제목을 찾고, `IDI_CLIENT`는 `LoadIcon`으로 아이콘을 찾는다. 현재 이름은 IDS_MAINFRAME이 아니라 IDR_MAINFRAME이며, 정의가 존재한다고 현재 코드가 사용한다는 뜻은 아니다. `//{{NO_DEPENDENCIES}}`는 생성 헤더의 주석 표식이고 C++의 의존성을 제거하는 지시문이 아니다. `_APS_NEXT_*`는 리소스 편집기가 새 항목의 ID를 정할 때 쓰는 값이며 게임 객체 초기값이 아니다.

### framework.h와 targetver.h

`framework.h`는 Windows 앱 템플릿의 공통 include 묶음이다. 엔진을 구성하는 Framework 클래스가 아니다. `targetver.h`를 포함한 뒤 WIN32_LEAN_AND_MEAN을 정의하고 windows.h, stdlib.h, malloc.h, memory.h, tchar.h를 포함한다. windows.h가 HINSTANCE/HWND/MSG/WNDCLASSEXW와 창·메시지 API를 제공한다. 나머지는 C 런타임과 TCHAR 계열 호환 선언이며, 목록에 있다는 이유만으로 현재 CPP가 모든 헤더를 직접 사용한다고 설명하지 않는다. WIN32_LEAN_AND_MEAN은 Windows 헤더에서 일부 덜 쓰는 선언을 제외하는 전처리 선택이다.

`targetver.h`의 `SDKDDKVer.h`는 Windows SDK의 버전 상수와 API 가용성 판단에 사용하는 기본 매크로를 제공한다. SDK를 설치하거나 현재 운영체제를 검사하는 실행 코드가 아니다. `_WIN32_WINNT`와 `WINVER`는 어떤 Windows API 선언을 대상으로 빌드할지 정하는 데 쓰며 실제 구형 OS 호환성은 사용하는 API·런타임·배포·실행 검증도 필요하다. 프로젝트의 `WindowsTargetPlatformVersion=10.0`은 사용할 SDK 선택이고 이 매크로와는 역할이 다르다.

현재 Client 프로젝트와 targetver.h에서 별도 지원 버전 값을 지정하지 않는다. 로컬 SDK 10.0.26100.0의 sdkddkver.h는 미정의 `_WIN32_WINNT`를 0x0A00, WINVER를 그 값으로 설정한다. 이것을 모든 프로젝트·번역 단위의 실효 값으로 일반화하지 않는다. 버전 값을 변경할 때는 Windows 헤더가 처음 읽히기 전의 전처리 정의와 PCH·강제 include 순서를 함께 확인한다. targetver.h는 Client.rc도 포함하므로 CPP만 보고 삭제하지 않는다.

공식 근거: [Microsoft Windows 버전 매크로](https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170), [Windows 헤더 사용](https://learn.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers).

### Client_Pch.cpp

원래 한 줄 주석의 뜻은 “이 CPP를 컴파일할 때 프로젝트 설정이 안정적인 공통 헤더를 강제로 포함하고, 그 상태로 PCH를 만든다”이다. PCH는 precompiled header이며 여러 CPP가 반복해서 처리하는 헤더의 컴파일러 상태를 재사용하기 위한 빌드 산출물이다. 게임 객체나 런타임 초기화 함수가 아니다. 번역 단위는 개념적으로 CPP 한 개에 include와 전처리 결과를 합친 컴파일 입력이다.

실제 연결은 `Client/Default/Client.vcxproj` → `Tools/Build/CppCompilation.props` → `Tools/Build/CppStandardPch.h`다. Client_Pch.h라는 파일이 있다고 추정하지 않는다. x64 기본 LostArkUsePch=true에서 `/FI`가 헤더를 강제로 포함하고, 이 CPP의 `/Yc`가 PCH를 생성하며, 일반 Client.cpp의 `/Yu`가 재사용한다. `/Fp`는 `$(IntDir)$(ProjectName).pch` 경로를 지정한다. `/Yc` 자체가 강제 include를 수행하는 것은 아니다.

CppStandardPch.h는 algorithm, array, filesystem, memory, vector 등의 표준 헤더만 묶는다. 게임·저작 헤더는 넣지 않는다. 프레임마다 PCH를 읽거나 실행 때 CPP가 먼저 호출되는 흐름으로 설명하지 않는다. 파일만 삭제하면 프로젝트에는 누락된 ClCompile 입력이 남고, 등록만 제거하면 PCH 생산자가 사라져 새 산출물 생성이 깨진다. PCH를 끄는 것은 가능한 별도 빌드 정책 변경이지만 현재 파일을 불필요한 코드로 판단할 근거는 없다.

공식 근거: [Microsoft PCH 설명](https://learn.microsoft.com/en-us/cpp/build/creating-precompiled-header-files?view=msvc-170), [/FI 강제 include](https://learn.microsoft.com/en-us/cpp/build/reference/fi-name-forced-include-file?view=msvc-170).

### Client.cpp의 실행 흐름

`Client.cpp`는 CRT 시작 코드가 이어 호출하는 wWinMain, 창 등록·생성, OS 메시지 처리와 MainApp의 수명을 연결한다. HINSTANCE는 EXE 모듈 핸들, HWND는 창 핸들이다. 핸들을 저장한 전역 변수와 C++ 객체를 소유하는 스마트 포인터를 구분한다. `_In_` 등의 SAL 표기는 매개변수 사용 계약을 정적 분석에 알리고, APIENTRY/CALLBACK은 Windows 함수 호출 규약을 표현한다.

현재 호출 순서는 다음과 같다.

1. `PrepareTerminateDiagnostic`가 치명적 종료 진단 파일과 handler를 준비한다. `CrashText`는 고정 버퍼 출력기, `RecordFatalDiagnostic`는 예외·현재 stack 기록, `ClientTerminateDiagnostic`와 `ClientAbortDiagnostic`는 비정상 종료 진입점이다. Interlocked guard는 진단 작성자 하나를 제한하며 정상 실행 복구를 하지 않는다. `WriteExitDiagnostic`는 일반 초기화·렌더 실패와 WM_QUIT 이유를 별도 파일에 기록한다.
2. Debug에서는 CRT 할당 추적과 선택적 할당 중단점을 설정한다. `LoadStringW`가 리소스 문자열을 읽고 `MyRegisterClass`가 WndProc·아이콘·커서 등을 가진 Windows 창 클래스 정보를 등록한다. 이것은 C++ 클래스를 생성하는 작업이 아니다.
3. 저장된 사용자 표시 설정을 읽고 `InitInstance`가 CreateWindowW로 창을 만든다. RECT의 원하는 Client 영역을 테두리 포함 크기로 조정하고 표시 설정을 적용한다. 설정 실패면 생성한 창을 정리하고 실패를 반환한다.
4. `CMainApp::Create()`는 unique_ptr를 반환한다. 내부에서 CMainApp을 생성하고 Initialize 실패면 지역 소유 포인터가 소멸·정리하며 nullptr를 반환한다. 현재 CMainApp은 CBase를 상속하지 않는다.
5. `CGameInstance::Get().Add_Timer(L"Timer_60")`로 경과 시간 측정을 준비한다. Timer_60이라는 이름만으로 고정 60Hz 실행이라고 해석하지 않는다. 실제 CTimer::Update_Timer는 QueryPerformanceCounter의 현재·이전 값 차이를 QueryPerformanceFrequency 값으로 나눈다. `(카운터 차이) / (카운터/초) = 초`이며 고정1/60초를 대입하지 않는다.
6. 바깥 while은 프로그램 루프다. 안쪽 PeekMessage 루프가 대기 메시지를 꺼내고 TranslateAccelerator/TranslateMessage/DispatchMessage로 처리한다. WM_QUIT이면 종료한다. 이후 resize 예약을 반영하고 최소화 중에는 메시지 대기로 실행 빈도를 낮춘다.
7. Update_TimeDelta → Profiler Begin_Frame → MainApp Update(dt) → MainApp Render → Profiler End_Frame → Limit_FrameRate 순서다. 시간 단위는 초이며 프레임 제한은 경과 시간 측정과 별도 책임이다. Client.cpp가 별도 LateUpdate_MainApp을 호출하지 않는다. Update 내부의 CGameInstance::Update_Engine에 object Priority_Update/Update/Post_Physics_Update/Late_Update와 Level 갱신이 연결된다.
8. Render 실패나 종료 메시지로 루프를 빠져나오면 표시 서비스 Shutdown 후 반환한다. wWinMain의 지역 unique_ptr가 소멸할 때 CMainApp 소멸자 → Free가 호출된다. 치명적 abort까지 정상 stack 정리를 보장하는 것은 아니다.

WndProc는 WM_SIZE를 즉시 GPU 재생성하지 않고 예약하고, DPI·최소/최대 크기를 처리한 뒤 ImGui 메시지 경로와 제품 문자·wheel 입력을 연결한다. WM_COMMAND는 메뉴 ID, WM_DESTROY는 PostQuitMessage, 기본 분기는 DefWindowProc다. WM_PAINT의 BeginPaint/EndPaint는 Windows 갱신 요청 처리이며 DX11 게임 프레임은 MainApp::Render에서 그린다. About은 DialogBox의 별도 callback으로 OK/Cancel에서 EndDialog를 호출한다. hdc, HWND 등의 핸들을 Safe_Release할 CBase 포인터로 취급하지 않는다.

### 공용 include에 대한 정확한 기준

LostArk의 Engine_Defines.h에는 이미 D3D11·DirectXMath와 vector/list/map/algorithm/functional/string/unordered_map/ctime/memory, ComPtr, Engine_Enum/Macro/Typedef/Function이 있다. 표준 헤더를 포함하면 자료형·함수 선언을 사용할 수 있게 되며 실제 게임 컨테이너 인스턴스가 생성되는 것은 아니다. 사용자의 “공용 헤더 정리가 전혀 없다”는 가정은 현재 파일과 다르다.

Engine_Struct.h와 모든 vendor 헤더는 현재 의도적으로 일괄 포함하지 않는다. 초기화·렌더·정점·애니메이션 타입의 실제 소비 파일이 해당 헤더를 포함한다. 기준은 “사용할 타입 정의가 필요한 위치에서 명확히 제공되는가”다. 헤더의 기반 클래스·값 멤버는 필요한 정의를 include하고, 클래스의 포인터·참조 선언만 필요한 경우 전방 선언을 검토한다. inline 함수가 멤버에 접근하거나 스마트 포인터 소멸 위치에서 완전 타입이 필요하면 그 조건도 확인한다. PCH나 우연한 간접 include만으로 컴파일되는 현상과 의도적으로 제공하는 공용 헤더 계약은 구분한다.

### MinecraftDungeons 질문의 실제 답

`CSquidCoast::Ready_GameLogicLayer`의 506행과 538행은 같은 pLayerTag/pLayer를 같은 map에 insert한다. 처음 저장한 것은 Layer 복사본이 아니라 포인터다. 이후 pLayer에 Boss를 추가하면 map을 통해서도 같은 Layer의 새 내용을 본다. 두 번째 insert는 동일 key 때문에 실패하고 반환값도 버리므로 현재 코드에서 필요 없는 중복이다. 작성 당시 의도는 확인하지 않았다. `map<const _tchar*, CLayer*>`는 문자열 내용 대신 포인터 key를 비교하는데, 이 사례는 정확히 같은 포인터를 다시 쓰므로 중복 판정이 확정된다.

CBase는 모든 객체의 참조 수를 모아 보관하는 전역 관리자가 아니라 각 인스턴스에 카운터와 정리 규칙을 제공하는 기반 클래스다. CBase.h의 protected:는 생성자·소멸자의 접근 제한이며 CGameObject는 public CBase 상속이다. CHUD(device)는 기본 생성자가 아니라 매개변수 있는 생성자이고, explicit은 암시적 변환 사용을 제한한다.

SR의 카운터는 0부터 시작한다. Release 진입 시 이미 0이면 가상 Free → delete this → 0 반환이다. 양수면 후위 감소 값을 반환한다. 예를 들어 생성 0 → Renderer AddRef로1 → 프레임 종료 Release는1 반환·내부0·객체 생존 → Layer 최종 Release에서 삭제다. 일반 COM의 “1에서0이 되는 호출에 삭제” 규약으로 바꾸어 설명하지 않는다. Free는 프로젝트의 가상 정리 함수이며 C++ 소멸자는 delete 단계에서 이어 호출된다. Safe_Release(T&)는 전달한 포인터 변수 하나를 바꿀 수 있고 같은 주소의 다른 별칭까지 null로 만들지는 않는다.

HUD 생성자의 Identity는 저장용 행렬 멤버 초기화다. 실제 UI 시작의 Render_BeginUI가 기존 View/Proj를 저장하고 device 행렬·상태를 설정한다. CTexture::Set_Texture는 텍스처 바인딩이고 CVIBuffer::Render_Buffer가 DrawIndexedPrimitive로 그리기를 요청한다. HUD의 component map과 m_pFilledHeart 같은 멤버는 같은 Clone을 가리키며 map이 해제를 맡는다. 둘을 각각 소유자로 보고 이중 Release하면 안 된다.

MainApp이 Proto_RcTex 원형을 등록하고 HUD의 Add_Component가 Clone을 얻는 구조다. CLoading.cpp의 Clone_Prototype 존재 확인은 반환 객체를 HUD에 전달하지 않으며 성공 clone을 보관·해제하지 않는 코드상 소유권 유실이 있다. 이번에는 발견과 설명만 했고 Minecraft 코드를 수정하거나 실행 누수를 계측하지 않았다.

다음 읽기는 사용자가 선택한 시작 흐름을 이어 CMainApp::Create → Initialize → ~CMainApp → Free의 실제 소유 멤버로 내려간다. 사용자의 독립 설명·직접 수정·breakpoint 관측은 이번 정적 조사와 별도로 확인한다.

### 같은 날 후속 — Windows·CRT·COM 헤더와 PCH의 역할

사용자는 별도 설명 대화에서 WIN32_LEAN_AND_MEAN, windows.h, stdlib.h/malloc.h/memory.h/tchar.h, CppStandardPch.h와 Client_Pch.cpp, objbase.h의 존재 이유와 제외 시 영향을 질문했다. 설치된 Windows SDK 10.0.26100.0의 실제 헤더와 현재 Client.vcxproj, CppCompilation.props를 읽어 전처리·컴파일·링크·실행을 구분했다.

WIN32_LEAN_AND_MEAN은 Windows.h의 일부 자동 include 분기를 생략한다. 정의 값이 0이어도 정의 여부 검사에는 참이며, 별도 objbase.h 포함은 COM 선언을 다시 제공할 수 있다. 현재 CRT의 stdlib.h와 malloc.h는 corecrt_malloc.h를 함께 포함한다. memory.h의 바이트 처리와 C++ memory의 스마트 포인터를 구분하고, CRT의 _UNICODE 및 WinAPI의 UNICODE에 따른 문자 매핑을 설명한다. 네 CRT 헤더를 모든 파일의 필수 입력으로 단정하지 않는다.

objbase.h는 combaseapi.h 등을 통해 COM 초기화·객체 생성·메모리·인터페이스 선언을 제공한다. 현재 Client.cpp에는 objbase.h가 있지만 CoInitializeEx 직접 호출은 없으며 MainApp.cpp의 Initialize와 loader/worker에 실제 호출이 있다. 대화 중 Client.cpp가 직접 초기화한다고 먼저 말한 부분은 소스 확인 후 정정했다. PCH는 표준 헤더의 컴파일러 상태를 프로젝트·구성별로 재사용하는 빌드 장치이며 런타임 객체나 전체 프로그램의 공통 메모리가 아니다.

이번 확인은 소스·설정·SDK 읽기와 개념 설명이다. 제품 코드 수정, 빌드·실행, 헤더 삭제 검증과 사용자의 독립 이해 확인은 수행하지 않았다. 다음 확인 질문은 “objbase.h를 include하는 것과 CoInitializeEx를 호출하는 것은 각각 언제 무엇을 바꾸는가”다.

### 같은 날 후속 — 객체 관리 중심으로 보정하고 UIInputRouter 읽기

사용자는 위 헤더 중심 설명이 지엽적이며, 객체 생성·Manager 책임·Management/Layer/GameObject/Component 관계를 먼저 연결해야 한다고 교정했다. 단순 블록 주석은 “빈 하트 위치와 크기”처럼 짧게 쓰고 학습용 장문을 코드에 반복하지 않는다. 이어 UIInputRouter.h의 생성 위치와 public/private, 모든 멤버 상태의 의미를 질문했다.

현재 UIInputRouter.cpp의 Get은 함수 지역 static instance를 참조로 반환한다. Router의 기본 생성자는 private의 = default이며 명시적 소멸자는 없어 암시적 public 소멸자를 사용한다. MainApp이나 Layer가 new/등록/Release하는 GameObject가 아니며 공용 입력 상태를 보유한다. MainApp::Update의 Begin_Frame, 정상 Render 끝의 End_Frame, Client.cpp WndProc의 문자·휠 전달과 UI view의 클릭 질의를 읽었다. CUIPointerScope는 별도 클래스이며 생성 시 현재 질의 창을 등록하고 소멸 시 이전 등록을 복원한다.

설명은 버튼 전·현 프레임과 클릭 소비, UI의 마우스 점유, 최상단 창과 질의 창의 식별, 시네마틱 차단, 휠 누적, 문자 입력의 여섯 상태 묶음으로 연결한다. const void* owner는 비교용 비소유 주소이며 창을 생성·해제하지 않는다. 이전 프레임 창 등록은 나중에 등록되는 최상단 창보다 먼저 수행되는 입력 검사에도 정보를 제공하기 위한 현재 구현이다. 대표 소비는 칭호 창 닫기와 인벤토리 제목줄 드래그다.

사용자는 이어 MinecraftDungeons 기준에서 UIInputRouter의 위치와 전체 객체 생성·소멸·관리를 연결해 달라고 요청했고, UIInputRouter의 영어 주석을 직접 지웠다고 알렸다. 그 편집은 보존한다. 현재 MainApp이 unique_ptr<CInventoryView>를 만들고, InventoryView가 unique_ptr<CUILayoutRuntime>을 만들며, layout이 CUI_Sprite를 기존 STATIC Layer_UI에 등록하고 shared_ptr로 참조하는 경로를 확인했다. Router는 이 소유 트리와 별개로 UI가 사용하는 입력 조정 객체다. Close는 열림 상태만 바꾸며 창 객체 소멸이 아니다.

이번 대화는 정적 코드 읽기와 설명이며 제품 소스 수정·실행·사용자 독립 이해 확인은 수행하지 않았다. 첫 추적 단위는 인벤토리 X 버튼으로 정한다. InventoryView 생성 → Update_CloseButton → Router::Is_Clicked → InventoryView::Close → 다음 갱신의 Hide를 따라가며, 입력 판정·창 상태 변경·소유 객체 해제를 구별한다.

### 같은 날 후속 — Client.cpp 종료 진단의 소유와 분리 방향

사용자는 MinecraftDungeons처럼 주석과 구조를 정리하는 과정에서 Client.cpp의 익명 namespace 전체를 삭제해도 되는지, Engine에 CrashReport 전용 H/CPP와 관리 클래스를 두는 것이 적절한지 질문했다. 현재 요청은 실제 관리 위치와 분리 방향의 설명이며 제품 리팩터링 완료로 기록하지 않는다.

현재 별도 Manager는 없다. Client.cpp의 파일 내부 정적 상태가 경로·파일 HANDLE·이전 handler·중복 기록 방지 값·고정 출력 버퍼를 보유한다. wWinMain → PrepareTerminateDiagnostic가 파일을 미리 열고 CRT의 terminate/SIGABRT callback을 등록한다. 종료 callback → RecordFatalDiagnostic → CrashText 포맷 → WriteFile → FlushFileBuffers로 기록한다. CrashText는 length만 가진 버퍼 작성자이며 예외 이력을 계속 보관하거나 파일을 저장하는 객체가 아니다. HANDLE은 명시적으로 닫지 않고 프로세스 종료까지 유지한다. 익명 namespace는 이름을 이 번역 단위 내부로 제한하며 객체의 생성·해제를 관리하는 장치가 아니다.

권장 방향은 범용 치명적 종료 기록을 Engine의 독립 CrashReport 모듈로 옮기고 Client에는 경로 선택과 초기 등록 호출을 남기는 것이다. 일반 GameInstance 초기화·해제에 수명을 묶으면 엔진 초기화 실패나 정리 중 종료를 기록할 범위가 줄어들 수 있다. MainApp의 WriteStartupDiagnostic과 Client.cpp의 WriteExitDiagnostic은 일반 시작·종료 진단으로 구분한다. 같은 익명 namespace 안의 WriteExitDiagnostic까지 무심코 지우지 않는다. 분리 시 Engine 프로젝트·filters 등록과 Client 링크, 기존 종료 callback·로그 동작을 검증해야 한다. 파일 분리는 구조 개선이며 프레임 성능 개선은 측정하지 않았다.

기능 유지의 실제 근거는 [2026-10-01 RESULT G15](../10-01/2026-10-01_PR494_496_FLEXIBLE_COLOSSEUM_RESULT.md)의 Character not in font 기록과 같은 빌드 PDB를 통한 호출 경로 분석이다. 이번에는 그 과거 기록과 현재 소스를 대조했으며 크래시 재현·빌드·실행은 하지 않았다. 다음 설명 단위는 전용 모듈이 소유할 상태와 wWinMain이 호출할 설치 경계다.

사용자는 후속으로 Engine의 Utility 필터에 파일을 직접 추가하겠다고 했으며, AI가 생성한 코드도 사람이 읽고 판단할 수 있도록 파일·클래스·함수·변수·struct의 필요성을 함께 비평하는 방식을 요청했다. 현재 필터의 실제 이름은 02.Utility다. 제안 이름은 Engine/Public/CrashReporter.h, Engine/Private/CrashReporter.cpp의 CCrashReporter이고, 추가 분류는 02.Utility 아래 17. CrashReport다. 이는 제안이며 이번 세션에서 제품 파일이나 필터를 생성하지 않았다.

공개 표면은 시작 시 한 번 호출하는 Install(logPath), 내부는 static 종료 callback과 기록 함수로 제한하는 방향이다. static 상태는 설치 여부·파일 HANDLE·고정 출력 버퍼·이전 handler·재진입 guard이며, 경로는 설치 동안만 차용하고 파일을 연 뒤 지속 저장하지 않아도 된다. 출력용 FixedTextWriter는 buffer/capacity/length를 명시적으로 받아 전역 버퍼에 대한 숨은 의존을 없애는 작은 내부 구현으로 제안한다. 중복 설치는 기존 상태를 보존하며 거부하고, 설치 실패는 게임 시작을 막지 않는 정책이다. GameInstance의 Create/Free·Update와 독립적으로 유지하되 Engine DLL unload 뒤까지 callback의 유효성을 보장한다고 설명하지 않는다. 다음 작성 단위는 사용자가 추가할 H/CPP의 공개 Install 계약과 private 상태 선언이다.

### 같은 날 후속 — 사용자가 옮긴 CrashReport_Manager의 공개 연결 확인

사용자는 Engine/Public/CrashReport_Manager.h와 Engine/Private/CrashReport_Manager.cpp를 직접 만들고 Client.cpp의 익명 namespace를 옮겼다. 실제 Engine 프로젝트와 02.Utility/17. CrashReport 필터 등록은 완료되어 있다. 현재 헤더는 private/public에 동일 생성자를 두 번 선언하고 공개 진단 함수는 선언하지 않았다. CPP의 함수는 여전히 익명 namespace 내부의 독립 함수이며 클래스 멤버로 바뀌지 않았다. Client.cpp는 PrepareTerminateDiagnostic 호출을 주석 처리하고 WriteExitDiagnostic의 기존 호출들을 유지한다. 이 상태는 파일 이동 완료이지 공개 API 연결이나 컴파일 완료가 아니다.

이름은 사용자가 만든 CCrashReport_Manager를 유지한다. 다음 작성 단위는 중복 생성자를 제거해 객체 생성을 막고 public static Install(logPath)을 선언한 뒤, Engine namespace의 CCrashReport_Manager::Install 정의와 Client 호출을 연결하는 것이다. 모든 진단을 GameInstance에 넣지 않는다. 치명적 종료 기록의 수명은 엔진 초기화·해제와 독립적이고, Client의 시작/종료 로그와 저장 경로 정책은 ClientDiagnostics H/CPP로 묶을 수 있다. 이 Client 진단 모듈도 별도 singleton이나 매 프레임 Update를 필요로 하지 않는다. 진입점에는 진단 준비 호출·창/앱 수명·메시지 루프·실패 판정과 기록 요청을 남긴다. 이번 확인에서 사용자 제품 소스는 수정하지 않았고 빌드·실행은 수행하지 않았다.

사용자는 이후 소멸자 선언과 생성자 =delete, Install 선언을 직접 수정했다. 이어 static과 inline, const wchar_t* 및 caller가 넘기는 주소의 의미를 질문하고 남은 구현을 요청했다. logPath는 출력 버퍼가 아니라 파일 경로 문자열의 입력이며, Install은 그 문자열로 파일을 열고 HANDLE·callback을 준비한 뒤 bool만 반환한다. 실제 로그 쓰기는 나중에 종료 callback에서 수행한다. static 멤버 함수에는 this가 없고 inline과는 다른 축이라는 설명을 [완성 코드 PLAN](2026-10-14_CRASH_REPORT_MANAGER_PLAN.md)에 기록했다. 후보 6개와 4 CPP 문법검사·미검증 경계는 [RESULT](2026-10-14_CRASH_REPORT_MANAGER_RESULT.md)에 있다. 제품 소스는 대신 작성하지 않았다.

최신 질문은 이 로그가 무엇을 산출하고 실제 도움이 되었는지, 정말 필요한지다. 게임 실행의 필수 조건과 진단 수단을 구분한다. 현재 Startup/Exit의 정상 기록을 직접 읽었고 현재 Crash 파일은 비어 있다. 실제 사용 근거는 10-01 보존 RESULT의 Character not in font와 당시 PDB 분석이다. Startup/Exit에도 같은 수준의 결함 해결 사례가 이번에 확인됐다고 일반화하지 않는다. 이후 학습은 기능 필요성 판단을 유지하며 Install의 입력 경로 → 파일 HANDLE → 종료 callback → 기록 시점을 설명하는 순서로 이어간다.

### 같은 날 후속 — s_ 표기와 사용자가 읽을 멤버 이름

사용자는 ClientWindowDisplay.cpp의 s_window/s_engineReady 등 접두사가 낯설며, 본인이 읽고 유지할 코드는 m_HWND/m_bEngineReady 같은 익숙한 멤버 표기로 정리하려는 의사를 밝혔다. s_에 적응하는 것을 프로젝트 학습의 선행 조건으로 요구하지 않는다. 이후 이 사용자에게 제안하는 클래스 멤버 이름은 m_ 기반의 합의한 표기를 우선하고, 기존 코드의 소속·공유 여부는 선언에서 별도로 설명한다.

실제 CClientWindowDisplay 헤더의 공개 함수는 전부 static이고 상태 변수는 현재 CPP의 익명 namespace에 있다. s_는 static/shared 상태를 나타내려는 이름 관례이며 C++ 문법이 아니다. 현재 변수는 클래스 멤버가 아닌 namespace-scope 상태로 정적 저장 기간을 가진다. m_로 이름만 바꿔도 멤버가 되지는 않는다. 클래스 소속을 명시하는 정리를 선택한다면 private static HWND m_hWnd, bool m_bEngineReady 등의 선언과 CPP의 한 번 정의로 옮길 수 있다. 이때 상태에 접근하는 Set_Window/Resize_ToClient 등 보조 함수도 private static 멤버로 연결해야 한다. static 멤버도 클래스 멤버이므로 m_를 쓰는 팀 규칙이 가능하며, 인스턴스별 상태와의 구분은 실제 static 선언에 근거한다. 이름 변경 때문에 새 객체·singleton·GameInstance 소유를 도입하지 않는다. 이번에는 정적 읽기와 방향 설명이며 제품 코드는 수정하지 않았다.


<!-- model-ownership-learning-20261014:begin -->
### 같은 날 후속 — Model.h의 타입과 실제 모델 로드·렌더 책임

사용자는 LostArk의 파일·프로젝트 구조를 먼저 정리하고, 각 파일과 객체의 역할을 이해한 뒤
필요성을 직접 판단하겠다고 정했다. 이후 필요한 책임을 분리·이관하고 리팩터링하며,
성능은 실제 측정으로 확인한 다음 그 문제·해법·대가를 기술소개서로 연결한다.
낯선 표기나 전방 선언 자체를 불필요한 코드 또는 성능 병목으로 판정하지 않는다.
이번 절은 현재 구현을 읽은 학습 기록이며 Model 구조 개편의 구현 완료 기록이 아니다.

#### Model.h에서 이름만 선언한 이유

[Model.h](C:/Users/tnest/Desktop/LostArk/Engine/Public/Model.h:13)의 `struct aiScene;`과
`namespace Assimp { class Importer; }`, 아래 Engine 구조체 선언은 타입의 이름을 컴파일러에
알리는 전방 선언이다. 여기서 객체를 만들거나 모델 파일을 읽지 않는다. 포인터·참조를
선언할 때 필요한 이름만 공개하고, 실제 필드와 함수가 필요한 CPP에서 정의 헤더를 포함한다.
`unique_ptr<Assimp::Importer>`가 실제 Importer를 정리하는 CModel 소멸자 정의도 Importer의
완전한 타입을 포함한 CPP에 있다. 전방 선언의 나열과 런타임 객체의 생성 순서는 별개다.

| 이름 | 실제 정의 | 맡는 입력·상태와 소비자 |
|---|---|---|
| `aiScene` | [Assimp/scene.h:273](C:/Users/tnest/Desktop/LostArk/Engine/Public/Assimp/scene.h:273) | Assimp가 읽은 모델 전체다. CModel은 메시·재질·애니메이션 배열과 루트 노드를 읽는다. `m_pAIScene`은 소유 포인터가 아니다. |
| `aiNode` | [Assimp/scene.h:82](C:/Users/tnest/Desktop/LostArk/Engine/Public/Assimp/scene.h:82) | 원본 노드 계층이다. `Ready_Bones(aiNode*)`가 부모·자식 순서로 CBone을 만든다. |
| `Assimp::Importer` | [Importer.hpp:124](C:/Users/tnest/Desktop/LostArk/Engine/Public/Assimp/Importer.hpp:124) | 원본 파일을 읽고 aiScene을 소유한다. CModel의 `unique_ptr`가 Importer를 소유한다. |
| `MODEL_ASSET_LOAD_DESC` | [ModelAssetData.h:649](C:/Users/tnest/Desktop/LostArk/Engine/Public/BinaryAsset/ModelAssetData.h:649) | 경로·리소스 루트·기본 애니메이션·재질 override를 담는 로드 요청이다. 읽은 결과가 아니다. |
| `MODEL_ASSET_DATA` | [ModelAssetData.h:637](C:/Users/tnest/Desktop/LostArk/Engine/Public/BinaryAsset/ModelAssetData.h:637) | Decoder가 복원한 CPU 모델 전체다. 메시·재질·골격·애니메이션을 묶어 CModel의 준비 함수에 전달한다. |
| `MODEL_MESH_DATA` | [ModelAssetData.h:536](C:/Users/tnest/Desktop/LostArk/Engine/Public/BinaryAsset/ModelAssetData.h:536) | 메시 하나의 CPU 정점·인덱스·재질 번호와 추가 UV/색/tangent 입력이다. CMesh가 이를 GPU 버퍼로 만든다. |
| `MODEL_ANIMATION_DATA` | [ModelAssetData.h:625](C:/Users/tnest/Desktop/LostArk/Engine/Public/BinaryAsset/ModelAssetData.h:625) | clip 이름·시간·본별 위치/회전/배율 키·이벤트 데이터다. CAnimation과 CChannel의 생성 입력이다. |
| `MODEL_MATERIAL_SOURCE` | [Model.cpp:29](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:29) | 이 CPP 내부에서 정의하는 재질 원본 보관소다. 로드 identity, override 전 재질, 메시별 채널 존재 정보를 보관해 재질 variant를 만든다. 전체 정점·인덱스 사본은 아니다. |
| `MODEL_COLOR_TINT` | [ModelAssetData.h:21](C:/Users/tnest/Desktop/LostArk/Engine/Public/BinaryAsset/ModelAssetData.h:21) | mask 염색의 활성 여부와 diffuse/영역별 색이다. CMaterial이 보관하고 CModel 조회를 통해 그리기 코드에 전달한다. |
| `MODEL_SURFACE_PARAMETERS` | [ModelAssetData.h:208](C:/Users/tnest/Desktop/LostArk/Engine/Public/BinaryAsset/ModelAssetData.h:208) | 재질 family, render/cull 방식, 원본 표면·조명 상수다. CMaterial의 검증과 shader 바인딩, Client의 pass 선택에 사용한다. 모든 필드가 WModel에 저장된다는 뜻은 아니다. |
| `MODEL_SOURCE_CHARACTER_PARAMETERS` | [ModelAssetData.h:67](C:/Users/tnest/Desktop/LostArk/Engine/Public/BinaryAsset/ModelAssetData.h:67) | 원본 캐릭터 shader program, base/light texture mask, 추가 UV 요구와 상수 배열이다. `MODEL_SURFACE_PARAMETERS`가 포함한다. |
| `MESH_SCREEN_LOD_DESC` | [MeshLod.h:10](C:/Users/tnest/Desktop/LostArk/Engine/Public/MeshLod.h:10) | 뷰 공간 경계·투영 픽셀 계수·최대 배율·허용 픽셀 오차 등 이번 draw의 LOD 선택 입력이다. 저장 모델 데이터나 가시성 판정 객체가 아니다. |

이 표는 한 종류의 거대한 모델 객체를 구성하는 멤버 목록이 아니다. 로드 요청, 복원 결과,
재질 값, 내부 원본 보관소, 매 프레임 draw 입력의 역할을 나눈 것이다. 이름을 모으려고
이 정의들을 Engine_Defines.h로 옮기면 범용 공통 헤더의 의존성이 더 커진다.

#### Assimp 경로와 WModel 경로가 실제로 갈라지는 지점

[CModel::Initialize_Prototype(path)](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:1012)는
확장자를 소문자로 바꾼 뒤 `.wmodel`이면 `Ready_BinaryModel`로 보낸다. 다른 확장자는
`Assimp::Importer`를 생성해 `ReadFile`을 호출하고 aiScene을 받아 기존 Ready 함수들로 간다.
정적 원본은 `aiProcess_PreTransformVertices`도 요청한다. 반면
[descriptor overload](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:1054)는 바로
바이너리 경로를 사용한다.

```text
Create(파일 경로) → Initialize_Prototype(path)
  .wmodel → Ready_BinaryModel → Decoder
  그 외   → Importer::ReadFile → aiScene → Ready_Bones/Meshes/Materials/Animations

Create(MODEL_ASSET_LOAD_DESC) → Initialize_Prototype(desc)
  → Ready_BinaryModel → Decoder
```

따라서 “런타임은 Assimp를 전혀 사용하지 않는다”는 현재 코드의 정확한 설명이 아니다.
WModel 경로는 Importer를 만들지 않지만 직접 import 구현은 남아 있다. Engine은
[Debug/Release Assimp 라이브러리를 링크](C:/Users/tnest/Desktop/LostArk/Engine/Default/Engine.vcxproj:130)하고
Client도 [해당 DLL을 배포](C:/Users/tnest/Desktop/LostArk/Client/Default/Client.vcxproj:1111)한다.
이것은 연결 가능한 기능과 배포 의존성의 증거이며, 실제 플레이 중 호출 빈도를 측정한 증거는 아니다.

ModelAssetConverter는 개발 단계에서 FBX/glTF를 미리 변환해 `.wmodel`을 만드는 도구다.
Decoder는 실행 중 이미 변환한 바이너리를 CPU 데이터로 해석한다. 변환기의
[“Assimp 없이 바이너리 해석” 설명](C:/Users/tnest/Desktop/LostArk/Tools/ModelAssetConverter/README.md:11)은
WModel 로드 경로에 한정해서 읽어야 한다. 원본 import 경로를 없애려면 실제 소비자를 별도로
확인한 뒤 API·라이브러리·DLL 배포까지 함께 정리해야 한다. 이번 조사에서 제거하지 않았다.

#### 바이너리 데이터가 GPU 자원으로 바뀌는 순서

1. Client의 [CActorCatalog::Build_ModelLoadDescription](C:/Users/tnest/Desktop/LostArk/Client/Private/ActorCatalog.cpp:1501)이 Resources 상대 ID를 실제 경로로 풀고 해당 모델의 재질 override를 `MODEL_ASSET_LOAD_DESC`에 담는다.
2. [Ready_BinaryModel](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:2647)이 [CModelDecoderRegistry::Decode](C:/Users/tnest/Desktop/LostArk/Engine/Private/BinaryAsset/ModelDecoderRegistry.cpp:41)를 호출한다. Registry의 기본 decoder는 CWModelDecoder이며 포맷 인식에는 파일 헤더를 사용한다.
3. [CWModelDecoder::Decode](C:/Users/tnest/Desktop/LostArk/Engine/Private/BinaryAsset/WModelDecoder.cpp:541)는 WMOD 내부 section 또는 분리 companion 파일을 읽는다. Mesh/Material/Skeleton/Animation reader가 지역 `MODEL_ASSET_DATA`를 완성한 뒤 반환한다. 이 성공이 GPU·텍스처 준비 성공까지 보장하지 않는다.
4. CModel은 override 전 재질 원본을 보관하고 override를 검증·적용한다. 이어 Bone → Mesh → Material → Animation 순서로 기존 객체를 만든다. 지정한 기본 animation이 있으면 존재와 시작 성공도 확인한다.
5. [CMesh 초기화](C:/Users/tnest/Desktop/LostArk/Engine/Private/Mesh.cpp:80)는 정점·인덱스 버퍼를 만들고 재질 번호·스킨 본 연결을 보관한다. [CMaterial 초기화](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:467)는 외부 texture 파일을 읽어 SRV를 준비하고 tint/surface 값을 보관한다.

WModel 안의 재질 경로와 catalog override는 같은 역할이 아니다. 원본 shader의 family/program,
named parameter, 추가 UV, IBL/BRDF 입력은 별도 연결이 필요하다. 형상과 diffuse가 보인다는
사실만으로 원본 재질 복원까지 완료했다고 설명하지 않는다. 파생 World Object는
`materialSourceModelAssetId`, 배경 재질 연결은 `mapMaterialBindings`의 실제 slot 검증을 따른다.
근거는 [공통 추출 절차](C:/Users/tnest/Desktop/LostArk/.md/GB/렌더링이펙트복원V2.md:647)와
[Area 재질 입력 계약](C:/Users/tnest/Desktop/LostArk/.md/TEAM/AREA_DATA_LAYER_GUIDE.md:480)이다.

#### Prototype·Clone의 공유와 실제 draw

Create의 `unique_ptr<CModel>`은 등록 전 임시 소유자이고 성공한 모델은 기존 Prototype 등록소로
넘긴다. GameObject의 Component 추가가 Clone을 요청하면
[CModel 복사 생성자](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:102)가 메시·재질의
공유 참조를 복사하고 Bone·Animation은 각각 Clone한다. 같은 형상을 쓰는 배우도 재생 상태는
따로 가진다. 원형을 복사한다고 정점 버퍼까지 매번 다시 만드는 것은 아니다.

재질 수정도 무조건 모든 clone에 전파하지 않는다.
[Override_SourceCharacterConstants](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:1557)는
공유 재질이면 `Clone_ForOverrides`로 해당 재질 상태를 먼저 분리한다.
[Create_MaterialVariant](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:3460)는 원본 로드
identity를 대조하고 geometry를 공유하면서 재질을 다시 준비한다. CPU 원본 geometry 전체를
항상 보관하는 것은 아니며, 명시적으로 선택한 ordered static geometry 등의 보관 경로는 별도다.

그리기에서는 Client가 placement와 화면 정책, shader pass를 결정한다. 실제
[MapAssetObject](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetObject.cpp:367)는
재질 바인딩 → `CShader::Begin` → `CModel::Render` 순서로 호출한다.
[CModel::Render](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:1080)는 해당 CMesh의
버퍼 바인딩과 draw를 요청하고, [CMesh::Render](C:/Users/tnest/Desktop/LostArk/Engine/Private/Mesh.cpp:355)는
선택한 index 범위로 `DrawIndexed`를 제출한다. CModel의 Render 하나가 모든 texture와 shader를
자동 선택하는 구조라고 설명하지 않는다. 애니메이션 모델은 갱신한 본 행렬도 별도로 바인딩한다.

`MODEL_ANIMATION_DATA.events`가 있다는 사실만으로 런타임 이벤트가 자동 등록되지는 않는다.
현재 [CAnimation::Initialize](C:/Users/tnest/Desktop/LostArk/Engine/Private/Animation.cpp:182)는
채널을 준비하며 파일의 native ticksPerSecond 대신 `COOKED_TICK_RATE`를 실제 재생 시계로 쓴다.
데이터가 존재하는 것과 소비 코드가 연결된 것을 나누어 읽는다.

#### 실제 캐릭터와 맵 재질을 따라가는 두 예시

캐릭터 예시는 [CharacterCatalog의 PLAYER_DIMENSIONMASTER](C:/Users/tnest/Desktop/LostArk/Data/Actors/CharacterCatalog.json:14920)다.
bodyModel은 `Character/DimensionMaster/DimensionMaster_Character.wmodel`이다. 설치 파일 크기는
77,009,740바이트이며 독립 조사에서 converter의 읽기 전용 `info`로 section 158개,
animation 155개와 skeleton 존재를 확인했다. 이 통계는 GPU 재생 성공을 뜻하지 않는다.
[하의 재질 override](C:/Users/tnest/Desktop/LostArk/Data/Actors/CharacterCatalog.json:14956)의
`pc_sp_m_01_lower01-11_mi`는 `source.character.realpbr-avatar-v2.v1` family를 사용한다.
[생성된 parameter 변환](C:/Users/tnest/Desktop/LostArk/Client/Private/SourceCharacterMaterialParameters_Generated.inl:186)은
이를 program 3, baseTextureMask 63과 named parameter를 배치한 상수 배열로 변환한다.
program 3은 이 shader 프로그램 선택값이며 캐릭터 ID가 아니다. 실제 생성 연결은
[PlayableCharacterAssetService](C:/Users/tnest/Desktop/LostArk/Client/Private/PlayableCharacterAssetService.cpp:310)의
load descriptor 준비와 같은 파일 368행의 `CModel::Create`에서 확인한다.

그 값은 CModel/CMaterial을 거쳐
[CMaterial::Bind_SourceCharacterInputs](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:967)의
base/light constants와 texture slot 바인딩으로 이어진다.
[Shader_VtxAnimMeshBinary](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl:229)는
`EvaluateSourceCharacterGeometry`를 호출하고,
[공통 source-character 처리](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_SourceCharacterMaterial.hlsli:274)에서
native base 프로그램으로 들어간다.
[SourceCharacterBase3](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_SourceCharacterBaseGroup001.hlsli:1288)의
실제 샘플에는 `g_SourceCharacterTexture1.SampleBias`도 있다. 이 재질의 texture expression은
0=normal(linear), 1=diffuse(sRGB), 2=ORM(linear)으로 연결되어 있으므로 여기의 texture1은
실제 diffuse 입력이다. 다른 program의 같은 번호도 같은 의미라고 일반화하지 않는다.
[Part_Body](C:/Users/tnest/Desktop/LostArk/Client/Private/Part_Body.cpp:211)는 본 행렬·shader pass를
준비한 뒤 모델 draw를 호출한다. 이 program은
[bUseSourceMaterials가 꺼졌을 때의 기본 재질 분기](C:/Users/tnest/Desktop/LostArk/Client/Private/DeferredMaterialRenderUtils.cpp:108)도 있다.

맵 예시는 [선택창 mapmaterials 첫 행](C:/Users/tnest/Desktop/LostArk/Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapmaterials.json:7)이다.
asset ID `MAP_54B7E9F1F9F0_BG_ELG_ARYANORB_FLOOR12_SM_WINGART_OVR_42610589A3EB`의
`SLOT_000_bg_elg_aryanorb_floor12_mi_wingart`는 `bg_base_pbr_opa` family와
diffuse/normal/detailNormal/ORM/reflection 경로를 선언한다.
[게시된 mapassets](C:/Users/tnest/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapassets:57)는
이 ID를 실제 WModel과 Prototype tag에 연결한다. 설치 WModel의 읽기 전용 `info` 결과는
section 2개, animation 0개, skeleton 없음, WMA2 재질 2개다.
[MapAssetCatalog](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetCatalog.cpp:1660)는
family를 `MODEL_SURFACE_FAMILY::PBR_OPAQUE`로 만들고,
[MapAssetRenderUtils](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp:1263)는
조건이 맞으면 그 enum 값 4를 surface program으로 바인딩한다. `bUseSourceMaterials`가 꺼져 있으면
program은 0이다. [Engine_RenderTypes.h](C:/Users/tnest/Desktop/LostArk/Engine/Public/Engine_RenderTypes.h:235)의
기본값은 true이지만 실행 중 실험 설정으로 바뀔 수 있다. 이번 조사에서 이 옵션을 변경하지 않았고
현재 화면의 ON/OFF도 판정하지 않았다.

[Shader_VtxMeshBinary](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl:315)는
`EvaluateMapSurface`로 들어간다.
[MapMaterialSurface의 PBR 계산](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_MapMaterialSurface.hlsli:565)은
ORM의 B를 metallic, G를 roughness, R을 AO 입력으로 사용한다. 이 확인은 해당 family의 실제
소비 증거이며 다른 LostArk specular/mask 이미지도 모두 ORM이라는 뜻은 아니다.

HLSL/HLSLI는 계산을 읽는 원본이다. 실제 CShader는 EXE 디렉터리의 같은 basename `.cso`를
찾아 읽고 `D3DX11CreateEffectFromMemory`로 Effect를 만든다.
[Shader.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/Shader.cpp:253)의 이 경로는 게임 실행 중
HLSL을 다시 컴파일하는 경로가 아니다. 소스 설명·빌드된 CSO·실제 실행 화면을 구별한다.

#### 확인한 범위와 다음 판단

이번에는 헤더·CPP·project의 실제 정의와 소비, 두 데이터 행과 shader 함수 연결을 읽었다.
제품 코드·저작 데이터·Resources·렌더링 옵션은 이 조사 작업에서 수정하지 않았다.
별도 의존성 정리 작업은 Debug x64 Product 빌드와 증분 재검증을 완료했다. 실제 완료 증거는
[CrashReport RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_CRASH_REPORT_MANAGER_RESULT.md)에
연결하며, 이 학습 조사의 읽기 범위를 실행 검증으로 바꾸어 기록하지 않는다.
Client/UI 실행·화면 판정, 원본 Assimp import의 실제 사용 빈도와 성능 측정,
사용자가 이 흐름을 독립적으로 설명할 수 있는지 확인하는 과정은 수행하지 않았다.

다음 학습 질문은 “로드 descriptor, Decoder의 CPU 결과, CMesh의 GPU 버퍼, CMaterial의 texture와
shader 입력 중 지금 보고 있는 값은 어디에 속하며 누가 소유하는가”다. 그 답을 바탕으로
직접 import 유지 여부, 데이터 헤더 분리, CModel의 재질·애니메이션 책임 분리를 판단한다.
이번 전방 선언 설명만으로 새 Manager·두 번째 모델 런타임을 만들거나 성능 개선을 주장하지 않는다.
<!-- model-ownership-learning-20261014:end -->

공용 shader의 정본·배포 위치도 구분한다. 위 예시의 `Shader_VtxAnimMeshBinary.hlsl`,
`Shader_VtxMeshBinary.hlsl`, `Shader_MapMaterialSurface.hlsli`는 Git에서 관리하는
Client/Bin/ShaderFiles 파일이며 위 링크는 실제 Client FXC 입력 위치를 가리킨다.
`Shader_SourceCharacterBaseGroup001.hlsli` 같은 공용 include는
[Engine/Bin/ShaderFiles 정본](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_SourceCharacterBaseGroup001.hlsli:1288)에서
EngineSDK/hlsl을 거쳐 Client/Bin/ShaderFiles로 배포된다. 같은 이름의 include가 양쪽에 있다고
서로 독립된 shader 구현 두 개로 세지 않는다. 배포 계약은
[CLAUDE.md](C:/Users/tnest/Desktop/LostArk/CLAUDE.md:135)를 따른다.


<!-- crash-static-writer-learning-20261014:begin -->
### 같은 날 후속 — CrashReport_Manager의 저장 공간과 Number·Address

사용자는 private static으로 옮긴 상태가 실제 어디에 존재하는지, 익명 namespace와 비교해
수명·접근·성능이 어떻게 다른지 질문했다. 이어 Number와 Address가 왜 필요한지 실제 호출부와
숫자 변환 과정까지 연결해 달라고 요청했다. 여기서는 현재 코드의 구조와 비용을 설명하며
클래스로 옮겼다는 사실을 성능 최적화의 증거로 사용하지 않는다.

#### 헤더의 선언과 CPP의 정의는 같은 변수 한 개를 가리킨다

[헤더의 private static 선언](C:/Users/tnest/Desktop/LostArk/Engine/Public/CrashReport_Manager.h:30)은
클래스가 사용하는 상태의 이름·타입과 접근 범위를 알린다.
[CPP의 11~16행 정의](C:/Users/tnest/Desktop/LostArk/Engine/Private/CrashReport_Manager.cpp:11)가
그 상태의 저장 공간과 초기값을 한 번 제공한다. 예를 들어 다음 두 줄은 버퍼 두 개가 아니다.

```cpp
//H: CCrashReport_Manager 안의 선언
static char m_szTextBuffer[65536];

//CPP: 이미 선언한 같은 멤버의 한 번 정의
char CCrashReport_Manager::m_szTextBuffer[65536]{};
```

이는 실제 선언을 발췌한 문법 설명이며 다시 추가하라는 코드가 아니다. CPP 정의에는
클래스 소속을 `CCrashReport_Manager::`로 붙이고 static을 반복하지 않는다. 객체별 멤버가
아니므로 Manager 객체가 없어도 저장 공간이 존재한다. 생성자를 `= delete`한 것과도 모순되지 않는다.
현재 구성에서는 로드된 Engine DLL이 이 상태를 제공하며, Client가 헤더를 include할 때마다
자기 버퍼를 새로 만들지 않는다. [Microsoft의 static 멤버 설명](https://learn.microsoft.com/en-us/cpp/cpp/static-members-cpp?view=msvc-170)

CPP 파일은 저장 공간을 정의한 소스 위치다. 실행 중 메모리가 “CPP 파일 안에” 있는 것은 아니다.
컴파일러가 OBJ에 코드·초기값·심벌 정보를 만들고 링커가 DLL을 구성하면, Windows가 DLL을
프로세스의 가상 주소 공간에 매핑한다. `.data`, `.bss`는 이 배치 형식의 section 이름이다.
따라서 “static이면 언제나 물리 메모리의 .data 영역”이라는 설명도 정확하지 않다. C++가 정하는
저장 기간과 특정 빌드가 선택한 section, OS가 실제 메모리 페이지를 제공하는 방식은 구분한다.

독립 검토가 `dumpbin`으로 확인한 Debug 산출물의 배치는 다음과 같다. 당시 OBJ의 SHA256은
`ABB4EEA5EF09693DBBEE322298C5F9E566DFAAF76801AAA43495E9765A35AD0C`였다.
이후 재빌드가 같은 경로의 산출물을 갱신하므로 해당 시점의 실측으로 구분한다. 모든
컴파일러·구성의 배치를 약속하거나 이 hash를 이후 빌드의 hash로 재사용하지 않는다.

| 현재 멤버 | OBJ에서 확인한 section |
|---|---|
| `m_hLogFile` | `.data`, 초기 바이트는 `FF FF FF FF FF FF FF FF`다. INVALID_HANDLE_VALUE의 x64 값이며 로그 내용이 아니다. |
| `m_bInstalled`, `m_szTextBuffer`, `m_lRecordEntered`, 이전 콜백 포인터 두 개 | 0으로 초기화하는 `.bss`에 있다. |

대상은 [CrashReport_Manager.obj](C:/Users/tnest/Desktop/LostArk/Engine/Default/x64/Debug/CrashReport_Manager.obj)와
[Engine.dll](C:/Users/tnest/Desktop/LostArk/Engine/Bin/Debug/Engine.dll)이다. 당시 DLL에서는 `.bss`가
독립 section으로 남지 않고 `.data` 범위에 합쳐졌으며, 그 section의 virtual size는 `0x30498`,
파일 raw size는 `0x13400`이었다. 파일 바이트 수와 메모리에서 필요한 범위가 같지 않은 예다.
원본 dump는 도구 stdout으로 확인했으며 별도 report 파일이 저장되어 있다고 주장하지 않는다.
재확인 명령은 OBJ에 `dumpbin /symbols`, `/headers`, `/rawdata /section:.data`,
DLL에 `dumpbin /headers`, `/exports`다. Engine DLL에 속한 상태도 프로세스마다 별개이므로
여러 Client EXE가 실행된다고 이 출력 버퍼를 서로 공유하지 않는다.

#### 익명 namespace와 private static의 차이

| 비교 | CPP의 익명 namespace에 둔 상태 | 현재 클래스의 private static 상태 |
|---|---|---|
| 수명 | namespace 범위 변수이므로 정적 저장 기간이다. 함수가 끝나도 없어지지 않는다. | 역시 정적 저장 기간이다. GameInstance나 지역 Manager 객체의 소멸에 묶이지 않는다. |
| 이름의 연결 | 내부 연결을 가져 같은 번역 단위의 구현에 한정한다. 다른 CPP의 같은 이름과 별개다. | 현재 이름 있는 Engine 클래스의 멤버다. private 접근 제어와 외부 연결·DLL export는 서로 다른 규칙이다. |
| 코드 접근 | 그 CPP에서 이름이 보이는 함수들이 사용할 수 있다. | 클래스 멤버와 허용된 접근 경로로 제한된다. 일반 외부 함수는 private 멤버에 직접 접근할 수 없다. |
| 읽는 위치 | 상태와 보조 함수의 관계를 CPP에서 확인한다. | 헤더의 private 구역에서 상태와 보조 함수의 소속을 먼저 확인한다. |
| 프레임 비용 | 이 범위에 둔다는 이유로 매 프레임 작업이 생기지 않는다. | static으로 옮긴다는 이유로 할당·갱신 비용이 사라지거나 속도가 빨라지지 않는다. |

익명 namespace는 잘못된 문법이 아니라 파일 구현을 감추는 수단이다.
현재 선택은 사용자가 클래스의 책임을 헤더에서 읽기 쉽게 하는 데 의미가 있다.
짧은 파일 전용 함수나 상수까지 모두 Manager로 만들 필요는 없다.
[Microsoft의 namespace 설명](https://learn.microsoft.com/en-us/cpp/cpp/namespaces-cpp?view=msvc-170)

`private`는 C++ 소스의 접근 제어다. DLL 심벌을 숨기는 보안 장치가 아니다.
위 산출물을 실측한 당시에는 이 클래스가 [ENGINE_DLL](C:/Users/tnest/Desktop/LostArk/Engine/Public/Engine_Macro.h:9)을 클래스에
붙였으므로 Engine을 만들 때 dllexport, Client에서 사용할 때 dllimport가 적용된다. “CPP에
정의했으니 내부 연결” 또는 “private이니 DLL에서 보이지 않음”으로 바꾸어 설명하지 않는다.
위 Debug DLL의 당시 export 목록에서도 여섯 static 데이터와 두 callback을 확인했다.
정적 멤버 방식도 프로세스 전역에 가까운 긴 수명의 상태를 남기므로, 한 프로세스 안에서 설치·해제를
반복하는 테스트나 독립 게임 세션 여러 개를 운용할 때는 불편하다. 지금은 게임 EXE의 종료 진단을
한 번 설치하는 범위이므로 그 제약을 받아들이며 별도 singleton·프레임 Update를 만들지 않았다.

#### 버퍼의 소유자와 한 번 기록하는 작성자는 다르다

| 상태 | 소유·수명과 실제 의미 |
|---|---|
| `m_szTextBuffer[65536]` | Manager가 보유하는 64KiB 고정 출력 공간이다. 종료할 때 새로 큰 문자열을 할당하지 않도록 미리 존재한다. |
| `m_hLogFile` | Install이 미리 연 파일의 HANDLE이다. 출력 버퍼 주소가 아니다. 정상 설치 뒤에는 프로세스 종료까지 유지하고 OS가 회수한다. |
| `m_bInstalled` | 설치 중복을 막는다. atomic이 아니므로 worker 시작 전 단일 스레드 설치라는 현재 계약이 필요하다. |
| `m_lRecordEntered` | fatal callback 중 기록자 한 명만 통과시키는 표식이다. volatile만으로 동기화하는 것이 아니라 InterlockedCompareExchange를 사용한다. |
| 이전 terminate/abort handler | 이번 기록 뒤 기존 종료 동작을 이어갈 함수 주소다. |
| `CrashText TextWriter` | RecordFatalDiagnostic 안의 지역 작성기다. 고정 버퍼의 주소·용량·현재 길이만 갖고 버퍼를 소유하지 않는다. |
| `Number`의 `szReversed[32]` | 숫자 한 번을 뒤집어 적는 작은 자동 저장 기간 배열이다. Number 호출이 끝나면 수명이 끝난다. |

[Install](C:/Users/tnest/Desktop/LostArk/Engine/Private/CrashReport_Manager.cpp:124)은 파일 열기와
콜백 등록을 시작 때 수행한다. [Client.cpp의 wWinMain](C:/Users/tnest/Desktop/LostArk/Client/Default/Client.cpp:55)이
`PrepareCrashReport`를 호출하고, Client가 만든 파일 경로 문자열의 주소를 Install에 넘긴다.
logPath는 `CreateFileW`의 입력이며 쓰려는 로그 본문이 아니다. CreateFileW가 반환한 HANDLE을
`m_hLogFile`에 저장한다. 나중에 fatal callback이 실행될 때 logPath를 다시 전달받는 구조가 아니라
이미 연 HANDLE에 버퍼의 글자를 WriteFile로 쓴다. 따라서 Client의 지역 경로 배열이 사라져도
그 주소를 나중에 다시 참조하지 않는다. 성공한 설치는 다시 받지 않으며, 명시적 준비 실패는 false로
돌려주고 파일·등록 상태를 가능한 지점에서 되돌린다. 경로 포인터는 저장하지 않는다.
Engine DLL을 별도로 unload하면 callback 함수 주소와 static 상태가 더 이상 유효하지 않을 수
있으므로, 현재 계약은 DLL을 프로세스 종료까지 유지하는 것이다.

[OnTerminate](C:/Users/tnest/Desktop/LostArk/Engine/Private/CrashReport_Manager.cpp:296)와
[OnAbort](C:/Users/tnest/Desktop/LostArk/Engine/Private/CrashReport_Manager.cpp:313)는
Interlocked로 표식을 0에서 1로 바꾼 첫 호출만 RecordFatalDiagnostic에 보낸다. 종료 중 같은
버퍼를 둘이 동시에 덮어쓰는 것을 막지만, 다른 fatal 경로를 기다려 주는 설계는 아니다.
다른 스레드가 기록 중이어도 후속 terminate가 abort로 이어질 수 있으므로 최초 기록이 끝까지
저장된다는 보장은 없다. guard는 기록 완료 알림이나 모든 스레드의 크래시 수집 큐가 아니다.

#### Number가 저장하는 것은 숫자의 이진 값이 아니라 읽을 수 있는 글자다

WriteFile은 전달받은 바이트를 저장한다. PID 정수 123을 사람이 읽는 `123`으로 남기려면
문자 `'1'`, `'2'`, `'3'`으로 변환해야 한다. 날짜의 월 7도 두 자리 `07`로 만들려면 자리 채움이
필요하다. [Number](C:/Users/tnest/Desktop/LostArk/Engine/Private/CrashReport_Manager.cpp:90)가
그 변환과 최소 폭을 맡고, Character가 최종 고정 버퍼에 한 글자씩 붙인다.

현재 호출부는 다음과 같다.

| 값 | 실제 호출 위치 | 출력 기준 |
|---|---|---|
| 년·월·일·시·분·초·밀리초 | [169행](C:/Users/tnest/Desktop/LostArk/Engine/Private/CrashReport_Manager.cpp:169)에서 181행까지 | 10진수, 폭 4·2·3으로 0을 채운다. |
| PID·TID·아직 처리되지 않은 예외 수 | [183행](C:/Users/tnest/Desktop/LostArk/Engine/Private/CrashReport_Manager.cpp:183), 185·187행 | 10진수, 별도 폭 없음. `uncaught`는 예외 이력의 총개수가 아니다. |
| 현재 스택의 frame 개수·frame 번호 | [216행](C:/Users/tnest/Desktop/LostArk/Engine/Private/CrashReport_Manager.cpp:216), 222행 | 개수는 일반 10진수, 번호는 최소 두 자리다. |
| 주소·module base·RVA | [226행](C:/Users/tnest/Desktop/LostArk/Engine/Private/CrashReport_Manager.cpp:226), 235·237행 | Address가 Number를 16진수로 호출한다. |

`Number(123, 5)`를 따라가면 다음 순서다.

```text
123 % 10 = 3 → 임시 배열에 '3' 저장, 123 / 10 = 12
 12 % 10 = 2 → 임시 배열에 '2' 저장,  12 / 10 = 1
  1 % 10 = 1 → 임시 배열에 '1' 저장,   1 / 10 = 0
임시 배열: ['3', '2', '1'], 실제 숫자 길이: 3
최소 폭 5까지 '0' 두 개 출력
임시 배열을 뒤에서 읽어 '1', '2', '3' 출력
결과: 00123
```

나머지는 가장 낮은 자리부터 나오기 때문에 임시 배열을 역순으로 읽는다. `do ... while`이라
value가 처음부터 0이어도 한 번 실행해 `'0'`을 출력한다. width는 잘라낼 길이가 아닌 최소 폭이라
`Number(12345, 2)`는 그대로 `12345`다. 실제 호출은 radix 10과 16뿐이며, 64비트 정수의
최대 10진수 20자리와 16진수 16자리는 지역 배열 32바이트 안에 들어간다. 외부에 임의 radix를
받는 범용 숫자 변환 API로 설계한 것이 아니다.

[Address](C:/Users/tnest/Desktop/LostArk/Engine/Private/CrashReport_Manager.cpp:115)는 앞에 `0x`를
붙이고 `Number(value, sizeof(void*) * 2, 16)`을 호출한다. x64의 포인터 크기는 8바이트라
16진수 16자리를 사용한다. 예를 들어 값 0x1A2B는 `0x0000000000001A2B`가 된다.
로그의 RVA는 캡처한 주소에서 해당 image의 module base를 뺀 값이다. ASLR로 module base가
달라져도 같은 빌드 안의 상대 위치를 추적할 수 있지만, 이것만으로 함수·소스 행이 자동으로
출력되는 것은 아니다. 주소 해석에는 해당 빌드의 EXE/DLL과 PDB가 필요하다.
주소 수집은 RecordFatalDiagnostic의 CaptureStackBackTrace가 수행한다. Address는 이미 얻은
주소 값을 출력 문자로 바꾸며 새로운 호출 스택을 찾는 함수가 아니다.

#### 필요한 책임과 지금 선택한 구현을 구별한다

CrashText는 Manager의 private 구역에서 선언한 중첩 타입이며 공개 Engine API가 아니다.
Number와 Address도 이 작은 작성기 내부의 보조 함수다. 값의 문자화와 주소 형식 통일은 필요하지만
나눗셈·나머지로 직접 변환하는 이 알고리즘만 가능한 것은 아니다.

이 formatter 자체는 caller가 제공한 고정 버퍼와 작은 지역 배열을 사용해 힙 할당을 요구하지 않는다.
그 사실이 예외 조회·stack capture·module 조회·파일 I/O를 포함한 전체 fatal 경로의 무할당이나
모든 손상 상태에서의 안전성을 보장하지는 않는다. 버퍼가 차면 Character는 이후 글자를 버리며,
WriteFile은 길이만큼 저장하므로 이 출력 버퍼를 항상 null 종료 문자열로 취급하지 않는다.

대안인 `std::to_chars`도 caller의 문자 범위에 숫자를 쓰고 locale에 의존하지 않는다.
정수 변환의 결과 끝 포인터와 오류를 확인하고 padding·`0x`를 별도로 붙이면 이 책임을 맡길 수 있다.
다만 16진수 문자가 기본적으로 소문자이므로 현재의 대문자 A~F 형식을 보존하려면 변환이 필요하다.
현재 수동 구현을 유지할지 표준 변환으로 줄일지는 가독성·출력 호환성과 실패 흐름으로 판단하며,
이 조사에서는 제품 구현을 바꾸지 않았다.
[Microsoft의 charconv 함수 설명](https://learn.microsoft.com/en-us/cpp/standard-library/charconv-functions?view=msvc-170)

아래는 필드 연결을 보여 주기 위한 **가상 로그 형식 예시이며 실제 발생 기록이 아니다.**

```text
CXX_TERMINATE local=2026-10-14 12:34:56.007 pid=1234 tid=5678 uncaught=1
exception=std::exception what=example error
current_stack_frames=1
#00 address=0x00007FF600001A2B module_base=0x00007FF600000000 rva=0x0000000000001A2B module=C:\Example\Client.exe
END_FATAL_DIAGNOSTIC
```

실제 유용했던 근거는 [10-01 RESULT의 G15](C:/Users/tnest/Desktop/LostArk/.md/GB/10-01/2026-10-01_PR494_496_FLEXIBLE_COLOSSEUM_RESULT.md:414)에
보존되어 있다. 당시 `Character not in font`와 같은 빌드 PDB로 SpriteFont::FindGlyph에서
파티 모집 문구까지 호출 경로를 찾았다. 이것은 과거 실제 결함 해결의 증거이며 위 가상 예시나
이번 클래스 이관 뒤의 새 크래시 실행 검증과 섞지 않는다. 현재 기능은 terminate/SIGABRT 기록이며
모든 접근 위반·강제 종료·SEH를 포괄하는 크래시 수집기가 아니다.

이번 학습 절에서는 제품 소스를 수정하거나 의도적인 fatal 종료를 실행하지 않았다.
프레임별 성능을 측정하지 않았으므로 namespace 제거나 static 이관의 FPS 개선을 주장하지 않는다.
다음 확인 질문은 “static 버퍼가 오래 존재하는 것과 Number의 지역 임시 배열이 잠깐 존재하는 것이
동시에 어떻게 가능한가, 그리고 Character가 쓴 글자가 어느 시점에 실제 파일로 넘어가는가”다.
<!-- crash-static-writer-learning-20261014:end -->

### 같은 날 후속 — 컨벤션 정본과 DLL 공개 범위 재적용

사용자는 WINTERS_ENGINE_CONVENTIONS.md의 읽기 좋은 구성을 참고해 LostArk 내부 규칙을 먼저
맞추기로 했다. 현재 정본은 [LOSTARK_ENGINE_CONVENTIONS.md](../../TEAM/LOSTARK_ENGINE_CONVENTIONS.md)다.
GameInstance의 참조 반환, Prototype/Clone의 unique/shared 소유, Public→SDK 배포는 실제 LostArk 계약을 유지한다.
짧은 주석·m_·Allman·전방 선언과 직접 include를 문서·formatter·스니펫에 같은 기준으로 연결했다.

앞 절의 class 전체 export 실측 이후, CCrashReport_Manager는 Install 선언에만 ENGINE_DLL을 붙였다.
새 Engine DLL의 dumpbin exports에서 CCrashReport_Manager 이름은 Install 한 항목만 남았다.
private static 데이터와 callback은 더 이상 export 목록에 없으며 소속·정적 수명·콜백 등록은 그대로다.
이 변화는 'private만으로 DLL 기호가 숨겨진다'는 뜻이 아니다. export를 명시적으로 좁힌 결과다.
근거는 out/CrashReportGuide20261014/Conventions/engine-exports.txt다.

Number/Address의 알고리즘과 출력 역할은 바꾸지 않았다. 사용자 CPP의 질문 메모도 보존했다.
이번 후속 코드·전체 감사·제품 빌드 증거는 [CODE_READABILITY_RESULT](../10-11/2026-10-11_CODE_READABILITY_RESULT.md)의
컨벤션 재적용 절에 이어 기록한다. 코드 읽기·수치 실측·실행 화면·사용자의 이해 확인을 구분한다.

<!-- bern-visibility-ownership-review-20261014:begin -->
## 같은 날 후속 — 맵 범위·가시성·F7 거리 기록·LOD의 책임 구분

사용자는 MainApp을 먼저 이해하고 정리한 다음 맵과 성 배경의 실제 기하량·프레임 저하를
해결하기로 했다. 이 절은 현재 소스와 기존 실측 기록의 읽기 조사다. 새 성능 개선이나
제품 수정·빌드·화면 검증을 완료했다는 기록이 아니다. 다른 세션의 조율은 사용자가 맡는다.

### MapLoadScope.h가 결정하는 것

- `MAP_SCENE_VISIBILITY_KIND`는 정적 표면·식생·지형·변형·순서 의존 표면의 파생 분류다.
  `MapAssetRenderUtils.cpp::Classify_SceneVisibility`와 거리 정책이 소비한다. 저장 asset 종류나 LOD 단계가 아니다.
- `MAP_FRUSTUM_CULLING_POLICY`는 카메라 프러스텀 검사 여유와 거부 유예 정책이다.
  `baseMargin`은 기본 반경 여유, 큰 물체에는 절대 여유와 반경 비율 중 큰 값을 적용한다.
  `rejectHysteresisFrames`는 보였던 물체가 경계를 벗어난 직후의 깜박임을 줄이는 유예다.
- `MAP_FRUSTUM_RUNTIME_STATE`는 각 배치·물체의 이전 판정과 남은 유예다. 설정 원본이나 저장 데이터가 아니다.
- `MAP_LOAD_SCOPE`는 로드할 placement의 XZ 범위·배경 포함·제외 그룹과 사용할 frustum 정책이다.
  `Contains`는 위치의 X/Z만 비교한다. 카메라 거리·Y·메시 표면·LOD는 계산하지 않는다.

`LevelRegistry.cpp:50`의 MakeBernMapScope는 현재 MakeFullMapScope를 바탕으로 한다.
따라서 베른은 실질적인 유한 XZ 사각형으로 먼 성을 잘라내는 상태가 아니다.
기본 frustum 여유는 0.25m, 큰 물체 반경 기준 4m, 추가 절대 여유 2m/상대 12%, 거부 유예 3프레임이다.
이 값과 거리 컬링의 반경 3m 기준은 서로 다른 정책이다.

`Loader.cpp:1103`의 Ready_MapArea와 `MapPlacementRuntime.cpp:82`의 Load_Area가 같은 scope를 사용한다.
`MapPlacementRuntime.cpp:239`의 필터는 제외 그룹을 먼저 제거하고, 범위 안 placement와
includeBackground가 허용한 범위 밖 BACKGROUND를 남긴다. scope 자체가 비활성이면 필터링하지 않는다.
로드 범위를 줄이는 것은 원경 모델을 싼 모델로 바꾸는 LOD가 아니라 원래 객체를 로드하지 않는 변경이다.

### 세션 설정과 실제 관측은 별도다

`Engine_RenderTypes.h:36`의 MAP_VISIBILITY_SETTINGS는 Renderer가 소유하고 GameInstance가 중계하는
세션 컬링 정책이다. 베른 컷신 기록 전용 자료가 아니며 Profiler UI가 값을 조절하고 캡처에 복사한다.
`DistanceMaxPixels`는 미터가 아니라 화면 지름 문턱이다. 작은 소품은
clamp(DistanceMaxPixels / DistanceScale, 4, 128)px를 사용한다. Scale 1→0.5이면 기본 문턱은
48→96px가 된다. 거리 문턱은 짧아지고 픽셀 문턱은 커져 둘 다 더 적극적으로 제외하는 방향이다.
기존 주석의 '같은 방향'을 두 숫자가 함께 감소한다는 뜻으로 읽으면 틀린다.

`MapAssetRenderUtils.cpp:1523`은 반경 3m 초과 정적 구조물에 120m×Scale과 고정 2px를 적용한다.
거리와 화면 크기 조건을 모두 만족해야 숨기므로 멀어도 화면에서 큰 성은 그대로 남는다.
지형·변형·순서 의존 표면은 같은 거리 제거를 일괄 적용하지 않는다.
`ParallelPreparationEnabled`는 프레임별 CPU 가시성 준비이며 AssetPreparationBatch의 로딩 병렬화와 다르다.

F7 기록 상태는 ProfilerTool_Cutscene.inl의 FCutsceneState, 완료 표본은 ProfilerCutsceneCapture가 소유한다.
`MapPlacementRuntime.cpp:492`는 카메라→placement/model bounding sphere 중심 거리를 계산하고
nearDistance=max(0, 중심거리−반경)도 기록한다. submesh 삼각형 표면의 최단거리가 아니다.
거리 범위는 source 목록 필터이며 실제 mapDraws 전체 제출을 그 범위로 잘라 집계하지 않는다.
픽셀 지름 자체는 source 행에 없으며 카메라 행렬·viewport·bounds가 보존된다.

MainApp의 연결은 UpdateProfilerRuntime → world render 전 Begin_CutsceneDiagnosticFrame →
실제 Render → End_CutsceneDiagnosticFrame(renderSucceeded)다. 창을 닫아도 이 갱신은 이어진다.
MainApp_Profiler.cpp는 같은 CMainApp 멤버의 파일 배치 분리이고, 기록 상태는 이미 ProfilerTool에 있다.
최소 후속 정리 후보는 MainApp.cpp의 F7 창 표시·focus·닫기를 기존 Profiler 파일의 멤버 함수로
묶고 본체에는 호출만 남기는 것이다. Begin/End를 ImGui 블록이나 Update 앞으로 옮기지 않는다.
새 Engine Manager나 GameInstance 소유 멤버는 이 정리에 필요하지 않다.

현재 캡처 context의 가시성 설정은 저장 시작 시 한 번 복사하므로 표본별 설정 이력이 아니다.
비교 도중 설정을 바꾸지 않거나 별도 revision 기록을 설계해야 한다. 창 닫기와 EXE 종료도 다르다.
현재 MainApp 종료는 진행 중 기록의 저장을 보장하지 않고 이미 시작된 exporter를 취소·join한다.

### 실제 LOD와 HLOD 위치

- Mesh LOD 스위치는 RENDER_OPTIMIZATION_SETTINGS::MeshLodEnabled다.
  생성은 CModel → CMesh → StaticMeshLod이며 원본 정점 속성을 공유하고 파생 index buffer를 준비한다.
  맵의 CPU/GPU 경로가 화면 오차에 맞는 단계를 선택한다. WModel에 완성된 여러 LOD 모델이
  이미 들어 있다고 해석하지 않는다. 파생 cache는 LocalAppData/LostArk/StaticMeshLod 아래에 있다.
- HLOD 정책은 MapStaticChunkObject.h의 MAP_CHUNK_POLICY, 맵 청크 생성·선택·원본 대체는
  MapPlacementRuntime/MapStaticChunkObject, 병합 geometry 생성은 Engine Model_StaticCluster에 있다.
  원본과 대체물을 동시에 제출하지 않도록 claim과 GPU mask를 연결한다.
- GPU HiZ는 깊이 기반 가림 검사다. 삼각형을 적은 모델로 바꾸는 HLOD와 다르다.

현재 HLOD는 source별 화면 오차, 현재 LOD보다 실제 index 감소, 기존 instancing을 감안한
최소 3개 draw 대체 등을 검사한다. 후보 생성 수·옵션 ON·실제 제출·FPS 개선은 각각 확인해야 한다.

### 기존 수치가 지지하는 후속 방향

[기존 HLOD RESULT의 G36](../10-04/2026-10-04_BERN_SPATIAL_CHUNK_HLOD_RESULT.md)은 첫 표본
map color index의 81.34%가 LOD0였고, 저장 카메라 6개의 실제 맵 단독 비교에서 LOD/HLOD를 모두
켠 VS 실행 수 감소가 4.425~5.935%였다고 기록한다. 이는 현재 LOD가 없다는 증거가 아니라
실제 감축 범위와 효과가 작다는 근거다. 상위 10개 대표 asset의 index 합도 5.48%여서 성 모델
몇 개의 축약만으로 전체 1/8을 보장하지 못한다. 강제 후보의 지붕·첨탑 손실도 실패 근거로 남아 있다.

같은 기록의 상세 거리 수집은 CPU 중앙값 15.17925ms/p95 30.7806ms이며 첫 표본은 236.8945ms다.
이는 수집이 발생한 표본 처리 비용이고 모든 프레임의 상시 비용이 아니다. 수집 외 비용도 전부 포함하지 않는다.
기록은 200만 source 상한에서 끝나 cue 약 0.2~5.1초만 포함하므로 전체 16초 컷신 분석으로 확대하지 않는다.

후속 판정은 (1) 계측 OFF/ON 분리, (2) LOD0 잔류 이유·HLOD 탈락 사유별 작업량,
(3) 실제 비용을 많이 차지하는 공간 그룹의 저폴리 후보와 원본 대체,
(4) 같은 카메라·해상도·설정에서 indices/VS/draw/CPU·GPU 시간과 형상 비교 순서다.
Vtx(VS)는 메시 개수가 아니라 GPU vertex shader 실행 수이고, draw 수·index 수와 같은 단위가 아니다.
MainApp 파일 정리와 계측 비용 감소를 제품 geometry/FPS 개선으로 기록하지 않는다.

이 조사에서는 제품 소스·Data·Resources·렌더 옵션을 변경하지 않았고 빌드·Client/GPU 실행도 하지 않았다.
<!-- bern-visibility-ownership-review-20261014:end -->
