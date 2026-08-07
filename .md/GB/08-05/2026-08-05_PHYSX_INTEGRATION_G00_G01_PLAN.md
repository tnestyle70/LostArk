# PhysX Integration G00-G01 전체 코드 PLAN

문서 유형: 구현 코드 정본 (설계와 결정 이유는 [PhysX 본질·세팅·활용 설계 PLAN](2026-08-05_PHYSX_ESSENCE_SETUP_UTILIZATION_DESIGN_PLAN.md)이 소유)
목표: PhysX 5.6.1을 Engine의 빌드·배포·Git 계약에 배치하고(G00), `CPhysics_Manager` 고정 스텝 서브시스템과 첫 소비자인 F1 Physics Tool drop test를 실행 증거까지 닫는다(G01).

2026-08-08 재개 실측: `Engine/ThirdPartyLib/PhysX`, `Engine/Public/Physics_Manager.h`, `Engine/Private/Physics_Manager.cpp`, `Client/Public/Physics_Tool.h`, `Client/Private/Physics_Tool.cpp`가 모두 없다. 따라서 이 문서는 **계획 상태**이고 현재 활성 단계는 G00이다. G00 검증 전에는 G01 C++를 반영하지 않는다.

## 0. 먼저 지키는 경계

- 작업 트리에 PhysX와 무관한 대규모 미커밋 변경이 있다. 사용자는 매 G 시작에 `git status --short`와 대상 파일 diff를 다시 확인하고, 이 계획서가 지정한 파일·블록만 반영한다. 다른 변경을 stage, restore, 정리하지 않는다.
- `Engine/Public/GameInstance.h`와 `Engine/Private/GameInstance.cpp`, `Client/Private/MainApp.cpp` 일부 구간에는 CP949 한국어 주석이 있다. 해당 파일은 **전체 교체를 금지**하고, 이 문서가 지정한 앵커에 ASCII 블록만 추가하거나 한국어 주석이 없는 함수만 전체 교체한다. 파일 인코딩을 변환하지 않는다.
- `Engine/Public/Physics_Manager.h`는 `EngineSDK/inc`로 복사돼 Client가 본다. 이 헤더에 PhysX include를 넣지 않는다(physx 전방 선언 + 포인터만). PhysX 타입은 `Physics_Manager.cpp`에만 존재한다.
- 새 전역 기능키는 없다. Physics Tool은 F1 Developer Tools 허브 안의 패널이다.

### 0.1 직접 반영 세션 계약

1. 한 세션에는 G 하나만 활성화한다. 이번 활성 단계는 G00이다.
2. 대화 설명은 `목표/종료 증거 → 수정 파일 → H 계약 → include·enum·struct·멤버 → CPP 함수 → 호출 흐름 → 작성 순서 → 검증` 순서를 지킨다.
3. 사용자가 코드를 반영한 뒤 대상 파일 diff와 해당 G 검증을 먼저 통과시킨다. 실패한 상태에서 다음 G로 넘어가지 않는다.
4. 새 파일은 디테일 PLAN의 전체 코드를 사용한다. 기존 파일은 반영 직전에 현재 앵커와 diff를 다시 대조한다. 특히 현재 `MainApp.h/.cpp`, `GameInstance.cpp`, project/filter, ProjectAudit에는 08-05 이후 변경이 있으므로 G01 진입 시 해당 전체 코드 section을 현재 코드로 재기준화한다.
5. PLAN에 코드가 있다는 사실은 구현 완료가 아니다. 실제 파일 diff가 생기고 검증 증거가 기록된 뒤에만 RESULT를 작성한다.

### 0.2 비평 반영

4렌즈(코드 정합성 / 경계 위반 / 완전성 / PhysX 기술 정확성) 독립 비평과 실코드 재현 검증을 이 문서 작성 직후 수행한다. 재현 확인된 지적과 반영 내역을 이 절에 기록한다.

## 1. 전체 파일 진행표

| 파일 | G | 변경 종류 |
|---|---|---|
| `Engine/ThirdPartyLib/PhysX/{Inc,Lib,Bin}` | G00 | 신규 바이너리 배치 (Git LFS 자동) |
| `Engine/Default/Engine.vcxproj` | G00, G01 | ItemDefinitionGroup 2블록 교체(G00) + ClInclude/ClCompile 2항목 추가(G01) |
| `UpdateLib.bat` | G00 | 전체 교체 (xcopy 3줄 추가) |
| `.gitignore` | G00 | 부정 블록 추가 |
| `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | G00, G01 | Add-Check 블록 2개 추가 |
| `Engine/Public/Physics_Manager.h` | G01 | 새 파일 |
| `Engine/Private/Physics_Manager.cpp` | G01 | 새 파일 |
| `Engine/Public/GameInstance.h` | G01 | 앵커 추가 2곳 (CP949 보존, 전체 교체 금지) |
| `Engine/Private/GameInstance.cpp` | G01 | include 1줄 + 앵커 추가 2곳 + 함수 전체 교체 2개 |
| `Engine/Default/Engine.vcxproj.filters` | G01 | 필터 1개 + 항목 2개 추가 |
| `Client/Public/Physics_Tool.h` | G01 | 새 파일 |
| `Client/Private/Physics_Tool.cpp` | G01 | 새 파일 |
| `Client/Public/MainApp.h` | G01 | 현재 파일 기준 `CPhysics_Tool` 전방 선언, enum 값, 멤버만 추가(전체 교체 금지) |
| `Client/Private/MainApp.cpp` | G01 | 현재 파일 기준 include·Ensure/render/free 분기만 추가(전체 교체 금지) |
| `Client/Default/Client.vcxproj` | G01 | ClInclude/ClCompile 2항목 추가 |
| `Client/Default/Client.vcxproj.filters` | G01 | 필터 1개 + 항목 2개 추가 |

---

## G00 — PhysX SDK 빌드·배치·링크·배포·Git 계약

### G00-1. 목표와 종료 증거

PhysX 5.6.1 바이너리가 `Engine/ThirdPartyLib/PhysX`에 배치되고, Engine이 PhysX를 링크한 채 Debug/Release 빌드에 성공하며, `UpdateLib.bat`이 PhysX 런타임 DLL 3종을 Client 실행 폴더로 배포하고, Git이 해당 바이너리를 LFS로 추적하는 상태가 종료 증거다. 이 G에는 C++ 코드가 없다.

### G00-2. SDK 1회 빌드 절차 (팀에서 1명만 수행, 산출물은 LFS로 공유)

CRT 계약: 우리 Engine/Client는 RuntimeLibrary 미지정으로 MSBuild 기본 **/MDd(Debug), /MD(Release)**다. PhysX 공개 preset의 `NV_USE_STATIC_WINCRT`는 기본 True(/MT 계열)이므로 반드시 False로 바꿔 DLL CRT(md) 산출물을 만든다. 우리 Debug에는 PhysX **debug** 빌드, 우리 Release에는 PhysX **release** 빌드만 연결한다(PhysX 헤더가 `_DEBUG`/`NDEBUG` 정확히 하나를 강제하고 구성 혼합은 CRT 충돌로 금지됨. checked/profile 구성은 채택하지 않음).

```powershell
# 저장소 밖 임시 폴더에서
git clone --depth 1 --branch 107.3-physx-5.6.1 https://github.com/NVIDIA-Omniverse/PhysX.git physx-sdk
```

1. `physx-sdk/physx/buildtools/presets/public/vc17win64.xml`에서 `NV_USE_STATIC_WINCRT` 값을 `True` → `False`로 수정한다.
2. `physx-sdk/physx/generate_projects.bat` 실행 후 preset `vc17win64`를 선택한다. 이때 `physx/include/PxConfig.h`가 생성된다(복사 전 필수).
3. `physx-sdk/physx/compiler/vc17win64/PhysXSDK.sln`을 VS2022로 열어 x64 `debug`와 `release`를 빌드한다.
4. 산출 폴더는 `physx-sdk/physx/bin/win.x86_64.vc143.md/debug`와 `.../release`다(md = DLL CRT. 폴더 접미가 `mt`면 1번이 반영되지 않은 것이므로 preset을 다시 확인한다).

### G00-3. ThirdPartyLib 배치 트리

`generate_projects` 이후의 `physx/include` 전체(하위 폴더와 `PxConfig.h` 포함)를 `Inc`로, 구성별 산출물을 아래 트리로 복사한다.

```text
Engine/ThirdPartyLib/PhysX/
├── Inc/                                  (physx/include 전체: PxPhysicsAPI.h, PxConfig.h, foundation/, extensions/, ...)
├── Lib/
│   ├── Debug/
│   │   ├── PhysX_64.lib
│   │   ├── PhysXCommon_64.lib
│   │   ├── PhysXFoundation_64.lib
│   │   ├── PhysXExtensions_static_64.lib
│   │   └── PhysXPvdSDK_static_64.lib
│   └── Release/                          (같은 5개 파일, release 빌드 산출)
└── Bin/
    ├── Debug/
    │   ├── PhysX_64.dll
    │   ├── PhysXCommon_64.dll
    │   └── PhysXFoundation_64.dll
    └── Release/                          (같은 3개 파일, release 빌드 산출)
```

- Cooking·GPU DLL은 배치하지 않는다. 박스/구/캡슐/plane 지오메트리는 cooking이 필요 없고(`PxCooking` 클래스는 5.2에서 제거, convex/triangle mesh만 immediate cooking 함수 사용), CPU 전용 시뮬레이션은 `PhysXGpu_64.dll`이 필요 없다.
- `PhysXExtensions_static_64.lib`/`PhysXPvdSDK_static_64.lib`는 static 전용이라 대응 DLL이 없다.
- 팀 인계: SDK tag(`107.3-physx-5.6.1`)와 preset 수정 내용(`NV_USE_STATIC_WINCRT=False`), Resources와 무관한 물리 폴더 위치 `Engine/ThirdPartyLib/PhysX`를 G00 RESULT에 기록한다. ZIP hash나 별도 hydrate 절차를 완료 조건으로 요구하지 않는다.

#### G00-3.1 헤더·라이브러리·DLL 역할

| 종류 | 파일 | 이번 프로젝트에서의 책임 |
|---|---|---|
| aggregate header | `Inc/PxPhysicsAPI.h` | G01 `Physics_Manager.cpp`가 Foundation, Scene, Actor, Shape, Joint API를 한 진입점으로 사용한다. Client public header에는 포함하지 않는다. |
| generated config | `Inc/PxConfig.h` | 선택한 preset/configuration의 compile 계약이다. `generate_projects` 전 원본 include만 복사하면 이 파일이 빠질 수 있으므로 audit 필수 항목이다. |
| import lib + DLL | `PhysX_64.lib/.dll` | `PxCreatePhysics`, `PxScene`, rigid actor/shape 등 핵심 SDK 구현을 링크·실행한다. |
| import lib + DLL | `PhysXCommon_64.lib/.dll` | PhysX 핵심 모듈들이 공유하는 common runtime 구현이다. `PhysX_64`와 같은 구성으로 배포한다. |
| import lib + DLL | `PhysXFoundation_64.lib/.dll` | allocator, error callback, foundation 기반 서비스를 제공한다. 가장 먼저 생성되고 가장 나중에 해제된다. |
| static lib | `PhysXExtensions_static_64.lib` | `PxRigidBodyExt`, 기본 plane/shape helper, D6/Spherical joint 생성 등 G01 이후 편의 API를 Engine DLL 안에 정적으로 링크한다. 대응 DLL은 없다. |
| static lib | `PhysXPvdSDK_static_64.lib` | `_DEBUG`에서 PhysX Visual Debugger 연결을 지원한다. 연결 실패는 시뮬 초기화 실패가 아니다. 대응 DLL은 없다. |

이번 범위에서 제외하는 라이브러리는 `PhysXGpu_64`, vehicle, character controller, cooking 전용 산출물이다. G00의 목적은 모든 PhysX 모듈을 복사하는 것이 아니라, G01~G05가 실제 소비할 최소 CPU rigid-body 집합을 고정하는 것이다.

### G00-4. `Engine/Default/Engine.vcxproj` — Debug|x64 / Release|x64 ItemDefinitionGroup 블록 전체 교체

변경 종류: 블록 전체 교체 2곳. Win32 구성과 다른 블록은 건드리지 않는다.

적용 위치 1: `<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">`부터 대응 `</ItemDefinitionGroup>`까지를 아래로 교체한다.

```xml
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>_DEBUG;ENGINE_EXPORTS;_WINDOWS;_USRDLL;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>..\Public;..\External\imgui;..\External\imgui\backends;..\ThirdPartyLib\FMOD\Inc;..\ThirdPartyLib\PhysX\Inc;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <EnableUAC>false</EnableUAC>
      <AdditionalDependencies>d3d11.lib;dinput8.lib;dxguid.lib;DirectXTKd.lib;Effects11d.lib;assimp-vc143-mtd.lib;fmod_vc.lib;PhysX_64.lib;PhysXCommon_64.lib;PhysXFoundation_64.lib;PhysXExtensions_static_64.lib;PhysXPvdSDK_static_64.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <AdditionalLibraryDirectories>..\ThirdPartyLib;..\ThirdPartyLib\FMOD\Lib;..\ThirdPartyLib\PhysX\Lib\Debug;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
    </Link>
    <FxCompile>
      <ShaderType>Effect</ShaderType>
    </FxCompile>
    <FxCompile>
      <ShaderModel>5.0</ShaderModel>
    </FxCompile>
  </ItemDefinitionGroup>
```

적용 위치 2: `<ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">`부터 대응 `</ItemDefinitionGroup>`까지를 아래로 교체한다.

```xml
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>NDEBUG;ENGINE_EXPORTS;_WINDOWS;_USRDLL;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>..\Public;..\External\imgui;..\External\imgui\backends;..\ThirdPartyLib\FMOD\Inc;..\ThirdPartyLib\PhysX\Inc;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <EnableUAC>false</EnableUAC>
      <AdditionalDependencies>d3d11.lib;dinput8.lib;dxguid.lib;DirectXTK.lib;Effects11.lib;assimp-vc143-mt.lib;fmod_vc.lib;PhysX_64.lib;PhysXCommon_64.lib;PhysXFoundation_64.lib;PhysXExtensions_static_64.lib;PhysXPvdSDK_static_64.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <AdditionalLibraryDirectories>..\ThirdPartyLib;..\ThirdPartyLib\FMOD\Lib;..\ThirdPartyLib\PhysX\Lib\Release;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
    </Link>
    <FxCompile>
      <ShaderType>Effect</ShaderType>
    </FxCompile>
    <FxCompile>
      <ShaderModel>5.0</ShaderModel>
    </FxCompile>
  </ItemDefinitionGroup>
```

### G00-5. `UpdateLib.bat` 전체 코드 (전체 교체)

변경 내용은 assimp 복사 줄 아래의 PhysX DLL xcopy 3줄뿐이다. 파일은 ASCII이므로 전체 교체한다.

```bat
@echo off
setlocal

set "BUILD_CONFIG=%~1"
if not defined BUILD_CONFIG set "BUILD_CONFIG=Debug"

if /i "%BUILD_CONFIG%"=="Debug" (
    set "ASSIMP_DLL=assimp-vc143-mtd.dll"
) else if /i "%BUILD_CONFIG%"=="Release" (
    set "ASSIMP_DLL=assimp-vc143-mt.dll"
) else (
    echo Unsupported configuration: %BUILD_CONFIG%
    exit /b 2
)

set "ENGINE_BIN=.\Engine\Bin\%BUILD_CONFIG%"
set "ENGINE_SDK_LIB=.\EngineSDK\lib\%BUILD_CONFIG%"
set "CLIENT_BIN=.\Client\Bin\%BUILD_CONFIG%"

if not exist "%ENGINE_BIN%\Engine.lib" (
    echo Engine import library was not built for %BUILD_CONFIG%: %ENGINE_BIN%\Engine.lib
    exit /b 1
)
if not exist "%ENGINE_BIN%\Engine.dll" (
    echo Engine runtime was not built for %BUILD_CONFIG%: %ENGINE_BIN%\Engine.dll
    exit /b 1
)
if /i "%BUILD_CONFIG%"=="Debug" if not exist "%ENGINE_BIN%\Engine.pdb" (
    echo Engine debug symbols were not built: %ENGINE_BIN%\Engine.pdb
    exit /b 1
)

xcopy /y/s ".\Engine\Public\*.*" ".\EngineSDK\inc\" || exit /b 1
xcopy /y "%ENGINE_BIN%\*.lib" "%ENGINE_SDK_LIB%\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\*.lib" ".\EngineSDK\lib\" || exit /b 1
xcopy /y "%ENGINE_BIN%\Engine.dll" "%CLIENT_BIN%\" || exit /b 1
if /i "%BUILD_CONFIG%"=="Debug" (
    xcopy /y "%ENGINE_BIN%\Engine.pdb" "%CLIENT_BIN%\" || exit /b 1
)
xcopy /y ".\Engine\ThirdPartyLib\FMOD\Bin\fmod.dll" "%CLIENT_BIN%\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\Assimp\Bin\%BUILD_CONFIG%\%ASSIMP_DLL%" "%CLIENT_BIN%\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\PhysX\Bin\%BUILD_CONFIG%\PhysX_64.dll" "%CLIENT_BIN%\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\PhysX\Bin\%BUILD_CONFIG%\PhysXCommon_64.dll" "%CLIENT_BIN%\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\PhysX\Bin\%BUILD_CONFIG%\PhysXFoundation_64.dll" "%CLIENT_BIN%\" || exit /b 1
xcopy /y ".\Engine\Bin\ShaderFiles\*.*" ".\EngineSDK\hlsl\" || exit /b 1
xcopy /y ".\EngineSDK\hlsl\*.*" ".\Client\Bin\ShaderFiles\" || exit /b 1

echo Runtime dependencies deployed to %CLIENT_BIN% for %BUILD_CONFIG%.
exit /b 0
```

### G00-6. `.gitignore` 추가 블록

작업: 추가 (다른 세션의 미커밋 변경이 있는 파일 — 기존 줄을 건드리지 않는다)
기준점: `!/Engine/ThirdPartyLib/Assimp/Bin/Release/assimp-vc143-mt.dll`
위치: 기준점 바로 아래 (다음의 `# Do not retain bundled viewer...` 주석 위)
필요한 이유: 전역 `*.lib`/`*.dll` ignore와 `**/[Dd]ebug/`·`**/[Rr]elease/` 디렉터리 ignore를 이기려면, Assimp 블록 관례대로 디렉터리부터 파일 글롭까지 명시 부정이 필요하다. `.gitattributes`의 `*.lib`/`*.dll` LFS 패턴은 자동 적용된다.

```gitignore
!/Engine/ThirdPartyLib/PhysX/
!/Engine/ThirdPartyLib/PhysX/Inc/
!/Engine/ThirdPartyLib/PhysX/Inc/**
!/Engine/ThirdPartyLib/PhysX/Lib/
!/Engine/ThirdPartyLib/PhysX/Lib/Debug/
!/Engine/ThirdPartyLib/PhysX/Lib/Debug/*.lib
!/Engine/ThirdPartyLib/PhysX/Lib/Release/
!/Engine/ThirdPartyLib/PhysX/Lib/Release/*.lib
!/Engine/ThirdPartyLib/PhysX/Bin/
!/Engine/ThirdPartyLib/PhysX/Bin/Debug/
!/Engine/ThirdPartyLib/PhysX/Bin/Debug/*.dll
!/Engine/ThirdPartyLib/PhysX/Bin/Release/
!/Engine/ThirdPartyLib/PhysX/Bin/Release/*.dll
```

### G00-7. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` 추가 블록 (G00분)

작업: 추가 (다른 세션의 미커밋 변경이 있는 파일 — 기존 Add-Check를 건드리지 않는다)
기준점: `try {` 블록 내부의 마지막 기존 `Add-Check` 호출 뒤
위치: 기준점 바로 아래, 독립 블록
필요한 이유: SDK 배치 누락(특히 `PxConfig.h` 없는 include 복사, 구성별 lib/dll 누락)을 빌드 전에 잡는다.

```powershell
    $physxRoot = Join-Path $repoRoot 'Engine/ThirdPartyLib/PhysX'
    $physxRequiredFiles = @(
        'Inc/PxPhysicsAPI.h', 'Inc/PxConfig.h',
        'Lib/Debug/PhysX_64.lib', 'Lib/Debug/PhysXCommon_64.lib', 'Lib/Debug/PhysXFoundation_64.lib',
        'Lib/Debug/PhysXExtensions_static_64.lib', 'Lib/Debug/PhysXPvdSDK_static_64.lib',
        'Lib/Release/PhysX_64.lib', 'Lib/Release/PhysXCommon_64.lib', 'Lib/Release/PhysXFoundation_64.lib',
        'Lib/Release/PhysXExtensions_static_64.lib', 'Lib/Release/PhysXPvdSDK_static_64.lib',
        'Bin/Debug/PhysX_64.dll', 'Bin/Debug/PhysXCommon_64.dll', 'Bin/Debug/PhysXFoundation_64.dll',
        'Bin/Release/PhysX_64.dll', 'Bin/Release/PhysXCommon_64.dll', 'Bin/Release/PhysXFoundation_64.dll')
    $physxMissing = @($physxRequiredFiles | Where-Object {
        -not (Test-Path -LiteralPath (Join-Path $physxRoot $_)) })
    Add-Check 'physics.g0-sdk-layout' ($physxMissing.Count -eq 0) "missing=$($physxMissing -join ',')"
```

### G00-8. 직접 작성 순서

1. `git status --short`와 `git diff -- Engine/Default/Engine.vcxproj UpdateLib.bat .gitignore Tools/ProjectAudit/Invoke-ProjectAudit.ps1`로 다른 변경을 확인한다.
2. G00-2 절차로 SDK를 1회 빌드한다(또는 같은 tag/preset으로 빌드된 산출물을 받는다).
3. G00-3 트리로 `Engine/ThirdPartyLib/PhysX`를 배치한다.
4. G00-6 `.gitignore` 블록을 추가하고 `git check-ignore`를 먼저 확인한다.
5. G00-4 vcxproj의 x64 두 블록에 include/lib 계약을 반영한다.
6. G00-5 `UpdateLib.bat`에 구성별 DLL 3종 배포를 반영한다.
7. G00-7 audit 블록을 추가한다.
8. G00-9를 Debug와 Release 모두 통과시킨다. 이 시점까지 C++ 헤더·함수·멤버 변수 추가는 0건이다.

### G00-9. 자동 검증

```powershell
git check-ignore -q Engine/ThirdPartyLib/PhysX/Lib/Debug/PhysX_64.lib; $LASTEXITCODE
git check-ignore -q Engine/ThirdPartyLib/PhysX/Bin/Release/PhysXFoundation_64.dll; $LASTEXITCODE
[xml](Get-Content -LiteralPath 'Engine/Default/Engine.vcxproj' -Raw) | Out-Null

$lostArkMsBuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
  -latest -products * -requires Microsoft.Component.MSBuild `
  -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
& $lostArkMsBuild Engine\Default\Engine.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& $lostArkMsBuild Engine\Default\Engine.vcxproj /p:Configuration=Release /p:Platform=x64 /m

.\UpdateLib.bat Debug
Test-Path 'Client/Bin/Debug/PhysX_64.dll', 'Client/Bin/Debug/PhysXCommon_64.dll', 'Client/Bin/Debug/PhysXFoundation_64.dll'
.\UpdateLib.bat Release
Test-Path 'Client/Bin/Release/PhysX_64.dll', 'Client/Bin/Release/PhysXCommon_64.dll', 'Client/Bin/Release/PhysXFoundation_64.dll'

git check-attr filter -- Engine/ThirdPartyLib/PhysX/Lib/Debug/PhysX_64.lib Engine/ThirdPartyLib/PhysX/Bin/Release/PhysX_64.dll

powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check -- Engine/Default/Engine.vcxproj UpdateLib.bat .gitignore Tools/ProjectAudit/Invoke-ProjectAudit.ps1
```

기대 결과:

```text
git check-ignore 두 건 모두 $LASTEXITCODE = 1  (ignore되지 않음 = 추적 대상)
Engine.vcxproj XML parse                        PASS
Engine x64 Debug/Release compile/link           PASS (PhysX 링크 오류 0)
UpdateLib 후 Client/Bin/Debug·Release DLL 각 3종 True True True
lib/dll의 git attribute filter                    lfs
physics.g0-sdk-layout                           PASS
대상 파일 git diff --check                       PASS
```

---

## G01 — CPhysics_Manager 서브시스템과 Physics Tool drop test

### G01-1. 목표와 종료 증거

`CGameInstance`가 `CPhysics_Manager`를 소유하고, 매 프레임 고정 스텝(1/60) accumulator로 `PxScene`을 전진시키며, 레벨 인덱스 버킷의 액터가 `Clear_Resources` seam에서 정리되는 상태. 종료 증거는 F1 → Physics Tool에서 Spawn Drop Test로 주황 와이어 박스들이 낙하해 y=0 격자 위에 쌓이고 sleep 시 회색으로 바뀌며, 레벨 전환 후 재진입 시 해당 레벨 액터가 0인 런타임 확인이다.

### G01-2. 파일별 한 줄 역할

| 파일 | 역할 |
|---|---|
| `Engine/Public/Physics_Manager.h` | PhysX 헤더 없는 물리 서브시스템 공개 계약 (physx 전방 선언 + 통계 struct) |
| `Engine/Private/Physics_Manager.cpp` | PhysX 객체 그래프 생성/해체, 고정 스텝 펌프, 레벨 버킷, drop test, 디버그 와이어 드로우 |
| `Engine/Public/GameInstance.h` | `Get_PhysicsManager()` 파사드와 멤버 추가 |
| `Engine/Private/GameInstance.cpp` | 서브시스템 생성/갱신/레벨 정리/해체 배선 |
| `Client/Public/Physics_Tool.h` | 첫 소비자: F1 패널 상태 (스폰 개수, 드로우 토글, 상태 문자열) |
| `Client/Private/Physics_Tool.cpp` | 파사드만 호출하는 ImGui 패널 (drop test 트리거, 통계 표시, 와이어 드로우 호출) |
| `Client/Public/MainApp.h` / `Client/Private/MainApp.cpp` | `DEBUG_TOOL::PHYSICS` 등록 (기존 툴 패턴 그대로) |

### G01-3. 실제 함수 흐름

```text
CGameInstance::Initialize_Engine
→ CPhysics_Manager::Create(device, context, iNumLevels)
→ Initialize: PxCreateFoundation → (PVD, _DEBUG 비치명) → PxCreatePhysics → PxInitExtensions
  → PxDefaultCpuDispatcherCreate(2) → createScene(gravity -9.81, default filter shader)
  → createMaterial(0.5, 0.5, 0.2) → 레벨 버킷 할당 → (_DEBUG) DebugDraw 배치/이펙트/입력레이아웃
→ 실패 시 Release_PhysX로 부분 생성물 전부 해제 후 E_FAIL (엔진 초기화 실패로 전파)

CGameInstance::Update_Engine (매 프레임, Object_Manager->Update 뒤 · Late_Update 앞)
→ CPhysics_Manager::Update(fTimeDelta)
→ accumulator += dt → while (accumulator ≥ 1/60 && steps < 4) simulate(1/60) + fetchResults(true)
→ 상한 초과 잔여분은 폐기 카운트 후 버림 (spiral-of-death 가드)
→ 통계 갱신 (steps, dropped, accumulator, simulate ms, actor 총계)

G02에서 같은 seam을 다음 순서로 확장한다.
→ Object Update가 animation pose와 kinematic target을 staging
→ Physics Update가 simulate/fetch
→ Object Manager `Post_Physics_Update`가 dynamic pose를 Transform/본에 commit
→ Late_Update가 commit된 pose로 renderer group 제출

CPhysics_Tool::Render (F1 허브가 활성 툴일 때 매 프레임)
→ CGameInstance::Get().Get_PhysicsManager() 파사드 획득 (null이면 안내 후 반환)
→ 통계 표시 → Spawn Drop Test 버튼 → Spawn_DropTest(currentLevelId, count)
  → 빈 버킷이면 ground plane 스테이징 → 박스 N개 스테이징 (하나라도 실패 시 스테이징 전부 release, 버킷 불변)
  → 전부 성공 시에만 씬 addActor + 버킷 push (stage → commit)
→ Clear Level Actors 버튼 → Clear(currentLevelId)
→ (_DEBUG) 드로우 토글 시 Render_Debug: 파이프라인 View/Proj로 OBB 와이어 (sleep=회색, 활성=주황, 바닥=녹색 격자)

CGameInstance::Clear_Resources(iClearLevelID)  (Change_Level이 이전 레벨 인덱스로 호출)
→ CPhysics_Manager::Clear: 해당 버킷 액터 removeActor + release, 버킷 비움
→ LEVEL::STATIC(0)은 인수가 되지 않으므로 자동 생존

CGameInstance::Release_Engine
→ m_pObject_Manager.reset() 뒤에 m_pPhysics_Manager.reset()
→ 소멸자 → Release_PhysX: 액터 → Scene → Dispatcher → Material → PxCloseExtensions → Physics → PVD → Foundation 역순 해제
```

### G01-4. `Engine/Public/Physics_Manager.h` 전체 코드 (새 파일, UTF-8 BOM 없음)

```cpp
#pragma once

/* Physics subsystem facade. This header is copied into EngineSDK/inc by
UpdateLib.bat, so it must stay free of PhysX includes: the Client compiles it
without any PhysX include path. Every PhysX type below is a forward-declared
pointer; the real object graph lives in Physics_Manager.cpp only. */
#include "Engine_Defines.h"

#include <vector>

namespace physx
{
	class PxFoundation;
	class PxPhysics;
	class PxDefaultCpuDispatcher;
	class PxScene;
	class PxMaterial;
	class PxRigidActor;
	class PxPvd;
	class PxPvdTransport;
}

NS_BEGIN(Engine)

struct PHYSICS_STEP_STATS
{
	uint32_t	iSceneActorCount = {};		/* actors alive across every level bucket */
	uint32_t	iStepsLastFrame = {};		/* fixed steps consumed by the latest Update call */
	uint32_t	iDroppedSteps = {};			/* lifetime steps discarded by the spiral-of-death guard */
	f32_t		fAccumulatorSeconds = {};	/* simulation debt left after the latest Update call */
	f32_t		fLastSimulateMs = {};		/* CPU cost of simulate+fetchResults in the latest Update call */
};

class ENGINE_DLL CPhysics_Manager final
{
public:
	/* The scene only ever advances in FIXED_TIMESTEP steps; variable frame
	deltas are absorbed by the accumulator in Update. */
	static constexpr f32_t		FIXED_TIMESTEP = 1.f / 60.f;
	static constexpr uint32_t	MAX_STEPS_PER_FRAME = 4;
	static constexpr uint32_t	MAX_DROP_TEST_BOXES = 256;

private:
	CPhysics_Manager(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

public:
	~CPhysics_Manager();

public:
	/* Caller: CGameInstance::Initialize_Engine, once, main thread. */
	HRESULT Initialize(uint32_t iNumLevels);
	/* Caller: CGameInstance::Update_Engine, every frame after object Update
	and before Late_Update. G02 inserts Post_Physics_Update after this call. */
	void Update(f32_t fTimeDelta);
	/* Caller: CGameInstance::Clear_Resources with the outgoing level index.
	LEVEL::STATIC is never passed, so bucket 0 survives every transition. */
	HRESULT Clear(uint32_t iClearLevelID);

	/* Debug consumer contract (CPhysics_Tool). First spawn into an empty
	bucket also creates the y=0 ground plane for that level. Stage->commit:
	a mid-creation failure leaves the scene and the bucket untouched. */
	HRESULT Spawn_DropTest(uint32_t iLevelID, uint32_t iBoxCount);
	uint32_t Get_ActorCount(uint32_t iLevelID) const;
	const PHYSICS_STEP_STATS& Get_StepStats() const { return m_StepStats; }

#ifdef _DEBUG
	/* Immediate wireframe draw of every registered actor with the current
	pipeline View/Proj. Call while the backbuffer is bound. */
	HRESULT Render_Debug();
#endif

private:
	void Step_Simulation();
	/* Releases the whole PhysX object graph in strict reverse creation order.
	Safe on partial initialization; every pointer is nulled. */
	void Release_PhysX();

private:
	/* PhysX object graph. Creation order: Foundation -> (Pvd) -> Physics ->
	Extensions -> Dispatcher -> Scene -> Material. Main thread only. */
	physx::PxFoundation*			m_pFoundation = { nullptr };
	physx::PxPvd*					m_pPvd = { nullptr };
	physx::PxPvdTransport*			m_pPvdTransport = { nullptr };
	physx::PxPhysics*				m_pPhysics = { nullptr };
	physx::PxDefaultCpuDispatcher*	m_pDispatcher = { nullptr };
	physx::PxScene*					m_pScene = { nullptr };
	physx::PxMaterial*				m_pMaterial = { nullptr };
	bool_t							m_bExtensionsReady = { false };

	/* Owning level-bucketed actor registry. Index space = level id (same as
	CObject_Manager). Clear/Release_PhysX call release() exactly once per
	stored actor; no other container owns these pointers. */
	std::vector<std::vector<physx::PxRigidActor*>>	m_LevelActors;

	f32_t				m_fAccumulator = {};
	PHYSICS_STEP_STATS	m_StepStats = {};

	ComPtr<ID3D11Device>		m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>	m_pContext = { nullptr };
#ifdef _DEBUG
	shared_ptr<PrimitiveBatch<VertexPositionColor>>	m_pBatch = { nullptr };
	shared_ptr<BasicEffect>							m_pEffect = { nullptr };
	ComPtr<ID3D11InputLayout>						m_pInputLayout = { nullptr };
#endif

public:
	static unique_ptr<CPhysics_Manager> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, uint32_t iNumLevels);
};

NS_END
```

### G01-5. `Engine/Private/Physics_Manager.cpp` 전체 코드 (새 파일, UTF-8 BOM 없음)

```cpp
#include "Physics_Manager.h"

#include "GameInstance.h"
#include "DebugDraw.h"

#include <PxPhysicsAPI.h>

#include <chrono>
#include <cmath>

using namespace physx;

namespace
{
	/* PhysX keeps references to these callbacks for the foundation's whole
	lifetime, so they must outlive every CPhysics_Manager instance. */
	PxDefaultAllocator		g_PhysXAllocator;
	PxDefaultErrorCallback	g_PhysXErrorCallback;
}

CPhysics_Manager::CPhysics_Manager(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
}

CPhysics_Manager::~CPhysics_Manager()
{
	Release_PhysX();
}

HRESULT CPhysics_Manager::Initialize(uint32_t iNumLevels)
{
	if (0 == iNumLevels)
		return E_INVALIDARG;

	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_PhysXAllocator, g_PhysXErrorCallback);
	if (nullptr == m_pFoundation)
	{
		OutputDebugStringA("[Physics] PxCreateFoundation failed.\n");
		return E_FAIL;
	}

#ifdef _DEBUG
	/* PVD is diagnostics only: a missing viewer must not fail engine startup. */
	m_pPvd = PxCreatePvd(*m_pFoundation);
	if (nullptr != m_pPvd)
	{
		m_pPvdTransport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		if (nullptr != m_pPvdTransport)
			m_pPvd->connect(*m_pPvdTransport, PxPvdInstrumentationFlag::eALL);
	}
#endif

	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), true, m_pPvd);
	if (nullptr == m_pPhysics)
	{
		OutputDebugStringA("[Physics] PxCreatePhysics failed.\n");
		Release_PhysX();
		return E_FAIL;
	}

	if (!PxInitExtensions(*m_pPhysics, m_pPvd))
	{
		OutputDebugStringA("[Physics] PxInitExtensions failed.\n");
		Release_PhysX();
		return E_FAIL;
	}
	m_bExtensionsReady = true;

	m_pDispatcher = PxDefaultCpuDispatcherCreate(2);
	if (nullptr == m_pDispatcher)
	{
		OutputDebugStringA("[Physics] PxDefaultCpuDispatcherCreate failed.\n");
		Release_PhysX();
		return E_FAIL;
	}

	PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.f, -9.81f, 0.f);
	sceneDesc.cpuDispatcher = m_pDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	m_pScene = m_pPhysics->createScene(sceneDesc);
	if (nullptr == m_pScene)
	{
		OutputDebugStringA("[Physics] createScene failed.\n");
		Release_PhysX();
		return E_FAIL;
	}

#ifdef _DEBUG
	if (PxPvdSceneClient* pPvdClient = m_pScene->getScenePvdClient())
	{
		pPvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pPvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pPvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
#endif

	m_pMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.2f);
	if (nullptr == m_pMaterial)
	{
		OutputDebugStringA("[Physics] createMaterial failed.\n");
		Release_PhysX();
		return E_FAIL;
	}

	m_LevelActors.assign(iNumLevels, {});

#ifdef _DEBUG
	m_pBatch = make_shared<PrimitiveBatch<VertexPositionColor>>(m_pContext.Get());
	m_pEffect = make_shared<BasicEffect>(m_pDevice.Get());
	m_pEffect->SetVertexColorEnabled(true);

	const void* pVertexShaderByteCode = { nullptr };
	size_t iLength = {};
	m_pEffect->GetVertexShaderBytecode(&pVertexShaderByteCode, &iLength);
	if (FAILED(m_pDevice->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount, pVertexShaderByteCode, iLength, &m_pInputLayout)))
	{
		Release_PhysX();
		return E_FAIL;
	}
#endif

	return S_OK;
}

void CPhysics_Manager::Update(f32_t fTimeDelta)
{
	m_StepStats.iStepsLastFrame = 0;
	m_StepStats.fLastSimulateMs = 0.f;

	if (nullptr == m_pScene || !std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
		return;

	m_fAccumulator += fTimeDelta;

	const auto simulateBegin = std::chrono::steady_clock::now();
	while (m_fAccumulator >= FIXED_TIMESTEP && m_StepStats.iStepsLastFrame < MAX_STEPS_PER_FRAME)
	{
		Step_Simulation();
		m_fAccumulator -= FIXED_TIMESTEP;
		++m_StepStats.iStepsLastFrame;
	}

	/* Spiral-of-death guard: debt beyond the per-frame cap is discarded, not
	carried, so one long hitch cannot snowball into permanent catch-up. */
	if (m_fAccumulator >= FIXED_TIMESTEP)
	{
		m_StepStats.iDroppedSteps += static_cast<uint32_t>(m_fAccumulator / FIXED_TIMESTEP);
		m_fAccumulator = std::fmod(m_fAccumulator, FIXED_TIMESTEP);
	}

	const std::chrono::duration<f32_t, std::milli> simulateCost =
		std::chrono::steady_clock::now() - simulateBegin;
	m_StepStats.fLastSimulateMs = simulateCost.count();
	m_StepStats.fAccumulatorSeconds = m_fAccumulator;

	uint32_t iTotalActors = {};
	for (const auto& LevelBucket : m_LevelActors)
		iTotalActors += static_cast<uint32_t>(LevelBucket.size());
	m_StepStats.iSceneActorCount = iTotalActors;
}

void CPhysics_Manager::Step_Simulation()
{
	m_pScene->simulate(FIXED_TIMESTEP);
	m_pScene->fetchResults(true);
}

HRESULT CPhysics_Manager::Clear(uint32_t iClearLevelID)
{
	if (iClearLevelID >= m_LevelActors.size())
		return E_FAIL;

	for (PxRigidActor* pActor : m_LevelActors[iClearLevelID])
	{
		if (nullptr == pActor)
			continue;
		if (nullptr != m_pScene)
			m_pScene->removeActor(*pActor);
		pActor->release();
	}
	m_LevelActors[iClearLevelID].clear();

	return S_OK;
}

HRESULT CPhysics_Manager::Spawn_DropTest(uint32_t iLevelID, uint32_t iBoxCount)
{
	if (nullptr == m_pScene || nullptr == m_pMaterial)
		return E_FAIL;
	if (iLevelID >= m_LevelActors.size() || 0 == iBoxCount || iBoxCount > MAX_DROP_TEST_BOXES)
		return E_INVALIDARG;

	std::vector<PxRigidActor*>& LevelBucket = m_LevelActors[iLevelID];
	std::vector<PxRigidActor*> Staged;
	Staged.reserve(iBoxCount + 1);

	if (LevelBucket.empty())
	{
		PxRigidStatic* pGround = PxCreatePlane(*m_pPhysics, PxPlane(0.f, 1.f, 0.f, 0.f), *m_pMaterial);
		if (nullptr == pGround)
			return E_FAIL;
		Staged.push_back(pGround);
	}

	const uint32_t iColumns = 8;
	for (uint32_t i = 0; i < iBoxCount; ++i)
	{
		/* Deterministic staggered grid: small per-index x/z offsets make the
		stack topple visibly without pulling in a random source. */
		const f32_t fX = static_cast<f32_t>(i % iColumns) * 1.1f - 4.f + static_cast<f32_t>(i % 3) * 0.05f;
		const f32_t fZ = static_cast<f32_t>((i / iColumns) % iColumns) * 1.1f - 4.f + static_cast<f32_t>(i % 5) * 0.03f;
		const f32_t fY = 4.f + static_cast<f32_t>(i / (iColumns * iColumns)) * 1.2f;

		PxRigidDynamic* pBox = PxCreateDynamic(
			*m_pPhysics,
			PxTransform(PxVec3(fX, fY, fZ)),
			PxBoxGeometry(0.5f, 0.5f, 0.5f),
			*m_pMaterial,
			10.f);
		if (nullptr == pBox)
		{
			for (PxRigidActor* pStagedActor : Staged)
				pStagedActor->release();
			return E_FAIL;
		}
		pBox->setAngularDamping(0.2f);
		Staged.push_back(pBox);
	}

	/* Commit only after every creation succeeded: a mid-loop failure above
	released the staged actors and left the scene and the bucket untouched. */
	for (PxRigidActor* pActor : Staged)
	{
		m_pScene->addActor(*pActor);
		LevelBucket.push_back(pActor);
	}

	return S_OK;
}

uint32_t CPhysics_Manager::Get_ActorCount(uint32_t iLevelID) const
{
	if (iLevelID >= m_LevelActors.size())
		return 0;
	return static_cast<uint32_t>(m_LevelActors[iLevelID].size());
}

#ifdef _DEBUG
HRESULT CPhysics_Manager::Render_Debug()
{
	if (nullptr == m_pBatch || nullptr == m_pEffect || nullptr == m_pInputLayout)
		return E_FAIL;

	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
	m_pEffect->SetProjection(XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(m_pInputLayout.Get());
	m_pEffect->Apply(m_pContext.Get());
	m_pBatch->Begin();

	for (const auto& LevelBucket : m_LevelActors)
	{
		for (PxRigidActor* pActor : LevelBucket)
		{
			if (nullptr == pActor)
				continue;

			PxRigidDynamic* pDynamic = pActor->is<PxRigidDynamic>();
			if (nullptr == pDynamic)
			{
				/* The static ground plane is infinite; draw a finite reference
				grid at y=0 instead of a shape. */
				DX::DrawGrid(
					m_pBatch.get(),
					XMVectorSet(10.f, 0.f, 0.f, 0.f),
					XMVectorSet(0.f, 0.f, 10.f, 0.f),
					XMVectorZero(),
					20, 20,
					Colors::Green);
				continue;
			}

			PxShape* pShape = { nullptr };
			if (1 != pDynamic->getShapes(&pShape, 1) || nullptr == pShape)
				continue;

			const PxGeometryHolder Geometry(pShape->getGeometry());
			if (PxGeometryType::eBOX != Geometry.getType())
				continue;

			const PxTransform Pose = PxShapeExt::getGlobalPose(*pShape, *pDynamic);
			const PxVec3 vHalfExtents = Geometry.box().halfExtents;

			BoundingOrientedBox obb;
			obb.Center = XMFLOAT3(Pose.p.x, Pose.p.y, Pose.p.z);
			obb.Extents = XMFLOAT3(vHalfExtents.x, vHalfExtents.y, vHalfExtents.z);
			obb.Orientation = XMFLOAT4(Pose.q.x, Pose.q.y, Pose.q.z, Pose.q.w);

			DX::Draw(m_pBatch.get(), obb, pDynamic->isSleeping() ? Colors::Gray : Colors::Orange);
		}
	}

	m_pBatch->End();
	return S_OK;
}
#endif

void CPhysics_Manager::Release_PhysX()
{
	for (auto& LevelBucket : m_LevelActors)
	{
		for (PxRigidActor* pActor : LevelBucket)
		{
			if (nullptr != pActor)
				pActor->release();
		}
		LevelBucket.clear();
	}

	if (nullptr != m_pScene)
	{
		m_pScene->release();
		m_pScene = nullptr;
	}
	if (nullptr != m_pDispatcher)
	{
		m_pDispatcher->release();
		m_pDispatcher = nullptr;
	}
	if (nullptr != m_pMaterial)
	{
		m_pMaterial->release();
		m_pMaterial = nullptr;
	}
	if (m_bExtensionsReady)
	{
		PxCloseExtensions();
		m_bExtensionsReady = false;
	}
	if (nullptr != m_pPhysics)
	{
		m_pPhysics->release();
		m_pPhysics = nullptr;
	}
	if (nullptr != m_pPvd)
	{
		if (m_pPvd->isConnected())
			m_pPvd->disconnect();
		m_pPvd->release();
		m_pPvd = nullptr;
	}
	if (nullptr != m_pPvdTransport)
	{
		m_pPvdTransport->release();
		m_pPvdTransport = nullptr;
	}
	if (nullptr != m_pFoundation)
	{
		m_pFoundation->release();
		m_pFoundation = nullptr;
	}
}

unique_ptr<CPhysics_Manager> CPhysics_Manager::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, uint32_t iNumLevels)
{
	auto pInstance = unique_ptr<CPhysics_Manager>(new CPhysics_Manager(pDevice, pContext));
	if (FAILED(pInstance->Initialize(iNumLevels)))
		return nullptr;
	return pInstance;
}
```

### G01-6. `Engine/Public/GameInstance.h` — 앵커 추가 2곳 (전체 교체 금지, CP949 주석 보존)

작업 1: 추가 — 파사드
기준점: `public: /* For.Profiler */` 블록의 `class CProfiler* Get_Profiler() const { return m_pProfiler.get(); }`
위치: 기준점 바로 아래, `private:` (멤버 블록 시작) 바로 위
필요한 이유: `Get_Profiler()` 선례를 따르는 포인터 노출 파사드. `GameInstance.h`의 include 최소 계약(전방 선언만)을 유지한다.
연결되는 부분: `CPhysics_Tool::Render()`가 유일한 현재 호출자

```cpp
public: /* For.Physics */
	class CPhysics_Manager* Get_PhysicsManager() const { return m_pPhysics_Manager.get(); }
```

작업 2: 추가 — 멤버 변수
기준점: `unique_ptr<class CProfiler>				m_pProfiler = { nullptr };`
위치: 기준점 바로 아래 (같은 `private:` 블록 안, `m_vViewportDesc`를 가진 다음 `private:` 위)
필요한 이유: 서브시스템 소유는 `CGameInstance`의 `unique_ptr` 멤버 관례를 따른다.

```cpp
	unique_ptr<class CPhysics_Manager>		m_pPhysics_Manager = { nullptr };
```

### G01-7. `Engine/Private/GameInstance.cpp` — include 1줄 + 앵커 추가 2곳 + 함수 전체 교체 2개 (CP949 주석 보존)

작업 1: 추가 — include
기준점: `#include "Profiler.h"`
위치: 기준점 바로 아래

```cpp
#include "Physics_Manager.h"
```

작업 2: 추가 — `Initialize_Engine` 내부 (함수 전체 교체 금지: 함수 상단에 CP949 주석이 있다)
기준점: `m_pFrustum = CFrustum::Create();`로 시작하는 3줄 블록 (`if (nullptr == m_pFrustum) return E_FAIL;`까지)
위치: 기준점 블록 바로 아래, `m_pProfiler = std::make_unique<CProfiler>();` 블록 바로 위
필요한 이유: `Create() → nullptr 검사 → E_FAIL` 이디엄 유지. iNumLevels로 레벨 버킷 크기를 고정한다.

```cpp
	m_pPhysics_Manager = CPhysics_Manager::Create(pOutDevice, pOutContext, EngineDesc.iNumLevels);
	if (nullptr == m_pPhysics_Manager)
		return E_FAIL;
```

작업 3: 함수 전체 교체 — `Update_Engine` (기존 함수에 주석 없음, ASCII 안전)
교체 범위: `void CGameInstance::Update_Engine(f32_t fTimeDelta)` 시작부터 닫는 `}`까지
필요한 이유: 고정 스텝 펌프는 오브젝트 Update가 애니메이션과 kinematic target을 확정한 뒤, Late_Update가 렌더 그룹을 제출하기 전에 돈다. G02는 물리 호출 직후 `Post_Physics_Update`를 추가해 같은 프레임에 결과를 역주입한다. 현재 코드의 `Refresh_CameraState()` 호출을 보존한다.

```cpp
void CGameInstance::Update_Engine(f32_t fTimeDelta)
{
	m_pPicking->Update();

	m_pInput_Device->Update();

#ifdef _WIN64
	m_pSound_Manager->Update();
#endif

	m_pObject_Manager->Priority_Update(fTimeDelta);

	Refresh_CameraState();

	m_pObject_Manager->Update(fTimeDelta);

	m_pPhysics_Manager->Update(fTimeDelta);

	m_pObject_Manager->Late_Update(fTimeDelta);

	m_pLevel_Manager->Update(fTimeDelta);
}
```

작업 4: 함수 전체 교체 — `Clear_Resources` (기존 함수에 주석 없음, ASCII 안전)
교체 범위: `HRESULT CGameInstance::Clear_Resources(uint32_t iClearLevelID)` 시작부터 닫는 `}`까지
필요한 이유: 레벨 전환 seam에서 물리 액터 버킷도 같은 레벨 인덱스로 정리한다.

```cpp
HRESULT CGameInstance::Clear_Resources(uint32_t iClearLevelID)
{
	if (FAILED(m_pObject_Manager->Clear(iClearLevelID)))
		return E_FAIL;

	if (FAILED(m_pPrototype_Manager->Clear(iClearLevelID)))
		return E_FAIL;

	if (FAILED(m_pPhysics_Manager->Clear(iClearLevelID)))
		return E_FAIL;

	return S_OK;
}
```

작업 5: 추가 — `Release_Engine` 내부 (함수 전체 교체 금지: 함수 중간에 CP949 주석이 있다)
기준점: `m_pObject_Manager.reset();`
위치: 기준점 바로 아래, `m_pPrototype_Manager.reset();` 바로 위
필요한 이유: 이후 G02에서 컴포넌트가 액터 핸들을 들면 오브젝트가 먼저 해제되고 씬/파운데이션이 나중에 죽어야 한다. 디버그 드로우의 D3D 리소스도 `m_pGraphic_Device` 해체 전에 정리된다.

```cpp
	m_pPhysics_Manager.reset();
```

### G01-8. `Client/Public/Physics_Tool.h` 전체 코드 (새 파일, UTF-8 BOM 없음)

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

/* F1 Developer Tools panel and the first consumer of CPhysics_Manager. It
owns only UI session state (spawn count, draw toggle, status line); every
physics fact it displays comes from the engine facade, and it never touches
PhysX types directly. */
class CPhysics_Tool final
{
public:
	void Render();

private:
	int32_t		m_iSpawnBoxCount = { 32 };
	bool_t		m_bDrawActors = { true };
	string		m_strStatus =
		"Spawn a drop test to verify the fixed-step simulation.";
};

NS_END
```

### G01-9. `Client/Private/Physics_Tool.cpp` 전체 코드 (새 파일, UTF-8 BOM 없음)

```cpp
#include "imgui.h"

#include "Physics_Tool.h"

#include "GameInstance.h"
#include "Physics_Manager.h"

void CPhysics_Tool::Render()
{
	Engine::CPhysics_Manager* pPhysics = CGameInstance::Get().Get_PhysicsManager();

	if (!ImGui::Begin("Physics Tool"))
	{
		ImGui::End();
		return;
	}

	if (nullptr == pPhysics)
	{
		ImGui::TextUnformatted("Physics manager is not initialized.");
		ImGui::End();
		return;
	}

	const uint32_t iCurrentLevelId = CGameInstance::Get().Get_CurrentLevelID();
	const Engine::PHYSICS_STEP_STATS& Stats = pPhysics->Get_StepStats();

	ImGui::Text("Current level id: %u", iCurrentLevelId);
	ImGui::Text("Level actors: %u (scene total %u)",
		pPhysics->Get_ActorCount(iCurrentLevelId), Stats.iSceneActorCount);
	ImGui::Text("Fixed steps last frame: %u (dropped %u)",
		Stats.iStepsLastFrame, Stats.iDroppedSteps);
	ImGui::Text("Accumulator: %.4f s", Stats.fAccumulatorSeconds);
	ImGui::Text("Simulate cost: %.3f ms", Stats.fLastSimulateMs);

	ImGui::SeparatorText("Drop Test");

	ImGui::SetNextItemWidth(120.f);
	if (ImGui::InputInt("Box count", &m_iSpawnBoxCount))
	{
		if (m_iSpawnBoxCount < 1)
			m_iSpawnBoxCount = 1;
		const int32_t iMaxBoxes =
			static_cast<int32_t>(Engine::CPhysics_Manager::MAX_DROP_TEST_BOXES);
		if (m_iSpawnBoxCount > iMaxBoxes)
			m_iSpawnBoxCount = iMaxBoxes;
	}

	if (ImGui::Button("Spawn Drop Test"))
	{
		m_strStatus = SUCCEEDED(pPhysics->Spawn_DropTest(
			iCurrentLevelId, static_cast<uint32_t>(m_iSpawnBoxCount))) ?
			"Drop test spawned into the current level." :
			"Drop test spawn failed. Check the debug output.";
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Level Actors"))
	{
		m_strStatus = SUCCEEDED(pPhysics->Clear(iCurrentLevelId)) ?
			"Current level actors released." :
			"Actor clear failed for the current level.";
	}

	ImGui::Checkbox("Draw actors (wireframe)", &m_bDrawActors);
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();

#ifdef _DEBUG
	/* Immediate wireframe draw into the bound backbuffer. Skipped when the
	panel is collapsed because the early return above never reaches here. */
	if (m_bDrawActors)
		pPhysics->Render_Debug();
#endif
}
```

### G01-10. `Client/Public/MainApp.h` 08-05 참고 스냅샷 (현재 파일 전체 교체 금지)

> 2026-08-08 현재 `MainApp.h`에는 Release HUD/Lobby/SkillWindow와 Rendering Workbench 상태가 추가되어 아래 08-05 전문을 그대로 복사하면 회귀한다. 이 section은 G01의 `CPhysics_Tool` 전방 선언, `DEBUG_TOOL::PHYSICS`, `m_pPhysicsTool` 위치를 설명하는 참고 스냅샷으로만 사용한다. G01 착수 세션에서 현재 파일 전문에 세 delta를 합친 최종 전문으로 교체한 뒤 반영한다.

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "LobbyCommandService.h"

NS_BEGIN(Engine)
class CImGuiLayer;
NS_END

NS_BEGIN(Client)

class CMapTool;
class CEffect_Tool;
class CAnimation_Tool;
class CHUDLayoutTool;
class CHUDRuntimeView;
class CPhysics_Tool;

class CMainApp final
{
#ifdef _DEBUG
private:
	enum class DEBUG_TOOL
	{
		NONE,
		MAP,
		ANIMATION,
		EFFECT,
		UI,
		PHYSICS
	};
#endif

private:
	CMainApp();

public:
	~CMainApp();

public:
	HRESULT Initialize();
	void Update(f32_t fTimeDelta);
	HRESULT Render();

private:
	HRESULT Ready_Fonts();
	HRESULT Ready_Prototype_For_Static();
	HRESULT Ready_Prototype_For_LoadingChrome();
	HRESULT Start_Level(
		LEVEL eTargetLevel,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	void Apply_LevelRequest();
	HRESULT ReadyImGuiRuntime();
	void RenderCombatHUD();

#ifdef _DEBUG
	HRESULT ReadyDebugTools();
	HRESULT EnsureDebugTool(DEBUG_TOOL eTool);
	void UpdateDebugToolShortcut();
	void RenderDeveloperTools();
	void RenderProfilerOverlay();
	void RenderProfilerSettings();
#endif

private:
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
	/* Not _DEBUG-gated: the runtime HUD art must render in Release too. */
	unique_ptr<CHUDRuntimeView> m_pHUDRuntimeView = { nullptr };

#ifdef _DEBUG
	unique_ptr<CMapTool> m_pMapTool = { nullptr };
	unique_ptr<CEffect_Tool> m_pEffectTool = { nullptr };
	unique_ptr<CAnimation_Tool> m_pAnimationTool = { nullptr };
	unique_ptr<CHUDLayoutTool> m_pHUDLayoutTool = { nullptr };
	unique_ptr<CPhysics_Tool> m_pPhysicsTool = { nullptr };
	bool_t m_bF1Down = false;
	bool_t m_bDeveloperToolsVisible = false;
	bool_t m_bProfilerVisible = false;
	DEBUG_TOOL m_eActiveDebugTool = DEBUG_TOOL::NONE;
	string m_strToolStatus =
		"Select a tool. Map authoring targets the current level Area.";
	string m_strProfilerCaptureStatus;
#endif

public:
	static unique_ptr<CMainApp> Create();
	void Free();
};

NS_END
```

### G01-11. `Client/Private/MainApp.cpp` — 현재 함수 기준 delta 재기준화 필요

> 2026-08-08 현재 `MainApp.cpp`에는 08-05 이후 Rendering/HUD/Effect Tool 분기가 추가되어 아래 함수 전문은 직접 반영 대상이 아니다. G01 착수 세션에서 현재 `EnsureDebugTool`, `RenderDeveloperTools`, render switch, `Free`를 읽고 Physics 분기만 합친 최종 함수 전문으로 갱신한다.

작업 1: 추가 — include
기준점: `#ifdef _DEBUG` include 블록의 `#include "MapTool.h"`
위치: 기준점 바로 아래, `#include "ProfilerCaptureIO.h"` 바로 위 (알파벳 순 유지)

```cpp
#include "Physics_Tool.h"
```

작업 2: 함수 전체 교체 — `EnsureDebugTool`
교체 범위: `HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool)` 시작부터 닫는 `}`까지

```cpp
HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool)
{
	if (nullptr != m_pMapTool && DEBUG_TOOL::MAP != eTool)
		m_pMapTool->SetOpen(false);

	switch (eTool)
	{
	case DEBUG_TOOL::MAP:
		if (nullptr == m_pMapTool)
		{
			auto mapTool = make_unique<CMapTool>();
			if (FAILED(mapTool->Initialize(m_pDevice, m_pContext)))
				return E_FAIL;
			m_pMapTool = move(mapTool);
		}
		m_pMapTool->SetOpen(true);
		break;
	case DEBUG_TOOL::ANIMATION:
		if (nullptr == m_pAnimationTool)
			m_pAnimationTool = make_unique<CAnimation_Tool>();
		break;
	case DEBUG_TOOL::EFFECT:
		if (nullptr == m_pEffectTool)
			m_pEffectTool = make_unique<CEffect_Tool>();
		break;
	case DEBUG_TOOL::UI:
		if (nullptr == m_pHUDLayoutTool)
			m_pHUDLayoutTool =
				make_unique<CHUDLayoutTool>(m_pDevice, m_pContext);
		break;
	case DEBUG_TOOL::PHYSICS:
		if (nullptr == m_pPhysicsTool)
			m_pPhysicsTool = make_unique<CPhysics_Tool>();
		break;
	default:
		return E_INVALIDARG;
	}

	m_eActiveDebugTool = eTool;
	return S_OK;
}
```

작업 3: 함수 전체 교체 — `RenderDeveloperTools`
교체 범위: `void CMainApp::RenderDeveloperTools()` 시작부터 닫는 `}`까지

```cpp
void CMainApp::RenderDeveloperTools()
{
	if (!ImGui::Begin(
		"LostArk Developer Tools",
		&m_bDeveloperToolsVisible,
		ImGuiWindowFlags_AlwaysAutoResize))
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
	ImGui::TextDisabled(isMapEditorWorkspace ?
		"Map Editor is active. Open Map Tool to author the selected Area." :
		"F1 only toggles tools. Enter Map Editor through Lobby Test.");
	ImGui::SeparatorText("Tools");

	const auto toolButton = [this](
		const char_t* pLabel,
		const DEBUG_TOOL eTool,
		const bool_t isEnabled)
	{
		ImGui::BeginDisabled(!isEnabled);
		if (ImGui::Button(pLabel))
		{
			m_strToolStatus = SUCCEEDED(EnsureDebugTool(eTool)) ?
				"Tool opened." : "Tool initialization failed.";
		}
		ImGui::EndDisabled();
	};

	toolButton("Map Tool", DEBUG_TOOL::MAP, isMapEditorWorkspace);
	ImGui::SameLine();
	toolButton(
		"Animation Tool",
		DEBUG_TOOL::ANIMATION,
		true);
	toolButton("Effect Tool", DEBUG_TOOL::EFFECT, true);
	ImGui::SameLine();
	toolButton("HUD Layout Tool", DEBUG_TOOL::UI, true);
	toolButton("Physics Tool", DEBUG_TOOL::PHYSICS, true);
	ImGui::TextWrapped("%s", m_strToolStatus.c_str());

	ImGui::SeparatorText("Diagnostics");
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
	ImGui::TextDisabled("F1: Developer Tools  |  F6: Follow/Free Camera");
	ImGui::End();
}
```

작업 4: 블록 교체 — `CMainApp::Render()` 안의 활성 툴 dispatch switch
교체 범위: `switch (m_eActiveDebugTool)` 시작부터 그 닫는 `}`까지 (함수 전체가 아니라 switch 문만)

```cpp
			switch (m_eActiveDebugTool)
			{
			case DEBUG_TOOL::MAP:
				if (nullptr != m_pMapTool)
					m_pMapTool->Render();
				break;
			case DEBUG_TOOL::ANIMATION:
				if (nullptr != m_pAnimationTool)
					m_pAnimationTool->Render();
				break;
			case DEBUG_TOOL::EFFECT:
				if (nullptr != m_pEffectTool)
					m_pEffectTool->Render();
				break;
			case DEBUG_TOOL::UI:
				if (nullptr != m_pHUDLayoutTool)
					m_pHUDLayoutTool->Render();
				break;
			case DEBUG_TOOL::PHYSICS:
				if (nullptr != m_pPhysicsTool)
					m_pPhysicsTool->Render();
				break;
			default:
				break;
			}
```

작업 5: 함수 전체 교체 — `Free`
교체 범위: `void CMainApp::Free()` 시작부터 닫는 `}`까지

```cpp
void CMainApp::Free()
{
	CNetworkManager::Get().Shutdown();
	CGameInstance::Get().SetInputBlocked(false, false);

#ifdef _DEBUG
	if (Engine::CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
		pProfiler->Set_Enabled(false);
	m_pAnimationTool.reset();
	m_pEffectTool.reset();
	m_pHUDLayoutTool.reset();
	m_pMapTool.reset();
	m_pPhysicsTool.reset();
#endif

	if (nullptr != m_pImGuiLayer)
		m_pImGuiLayer->Shutdown();
	m_pImGuiLayer.reset();
	CGameInstance::Get().Release_Engine();
}
```

### G01-12. 프로젝트 등록 코드

`Engine/Default/Engine.vcxproj` — 추가 2항목:

- 기준점: `<ClInclude Include="..\Public\Profiler.h" />` / 위치: 바로 위

```xml
    <ClInclude Include="..\Public\Physics_Manager.h" />
```

- 기준점: `<ClCompile Include="..\Private\Profiler.cpp" />` / 위치: 바로 위

```xml
    <ClCompile Include="..\Private\Physics_Manager.cpp" />
```

`Engine/Default/Engine.vcxproj.filters` — 필터 1개 + 항목 2개:

- 기준점: `<Filter Include="02.Utility\13. Profiler">` 블록의 닫는 `</Filter>` / 위치: 바로 아래

```xml
    <Filter Include="02.Utility\16. Physics">
      <UniqueIdentifier>{63aea2b1-10c7-48da-b94c-dc71a6fb98e6}</UniqueIdentifier>
    </Filter>
```

- ClInclude ItemGroup: 기준점 `<ClInclude Include="..\Public\Profiler.h">` 항목 블록 / 위치: 바로 아래

```xml
    <ClInclude Include="..\Public\Physics_Manager.h">
      <Filter>02.Utility\16. Physics</Filter>
    </ClInclude>
```

- ClCompile ItemGroup: 기준점 `<ClCompile Include="..\Private\Profiler.cpp">` 항목 블록 / 위치: 바로 아래

```xml
    <ClCompile Include="..\Private\Physics_Manager.cpp">
      <Filter>02.Utility\16. Physics</Filter>
    </ClCompile>
```

`Client/Default/Client.vcxproj` — 추가 2항목 (다른 세션의 미커밋 변경이 있는 파일 — 항목만 추가):

- 기준점: `<ClInclude Include="..\public\Effect_Tool.h" />` / 위치: 바로 아래

```xml
    <ClInclude Include="..\Public\Physics_Tool.h" />
```

- 기준점: `<ClCompile Include="..\private\Effect_Tool.cpp" />` / 위치: 바로 아래

```xml
    <ClCompile Include="..\Private\Physics_Tool.cpp" />
```

`Client/Default/Client.vcxproj.filters` — 필터 1개 + 항목 2개:

- 기준점: `<Filter Include="03. Tools\05. Sequencer">` 블록의 닫는 `</Filter>` / 위치: 바로 아래

```xml
    <Filter Include="03. Tools\06. Physics">
      <UniqueIdentifier>{0586c9b9-0cb1-4c83-822e-44c081d8ef2b}</UniqueIdentifier>
    </Filter>
```

- ClCompile: 기준점 `<ClCompile Include="..\private\Effect_Tool.cpp">` 항목 블록 / 위치: 바로 아래

```xml
    <ClCompile Include="..\Private\Physics_Tool.cpp">
      <Filter>03. Tools\06. Physics</Filter>
    </ClCompile>
```

- ClInclude: 기준점 `<ClInclude Include="..\public\Effect_Tool.h">` 항목 블록 / 위치: 바로 아래

```xml
    <ClInclude Include="..\Public\Physics_Tool.h">
      <Filter>03. Tools\06. Physics</Filter>
    </ClInclude>
```

### G01-13. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` 추가 블록 (G01분)

작업: 추가
기준점: G00-7에서 추가한 `physics.g0-sdk-layout` Add-Check
위치: 기준점 바로 아래

```powershell
    $engineProjectText = Get-Content -LiteralPath (Join-Path $repoRoot 'Engine/Default/Engine.vcxproj') -Raw
    $clientProjectText = Get-Content -LiteralPath (Join-Path $repoRoot 'Client/Default/Client.vcxproj') -Raw
    $updateLibText = Get-Content -LiteralPath (Join-Path $repoRoot 'UpdateLib.bat') -Raw
    $gitignoreText = Get-Content -LiteralPath (Join-Path $repoRoot '.gitignore') -Raw
    Add-Check 'physics.g1-subsystem-registration' (
        $engineProjectText.Contains('Physics_Manager.h') -and
        $engineProjectText.Contains('Physics_Manager.cpp') -and
        $engineProjectText.Contains('PhysX_64.lib') -and
        $clientProjectText.Contains('Physics_Tool.h') -and
        $clientProjectText.Contains('Physics_Tool.cpp') -and
        $updateLibText.Contains('PhysXFoundation_64.dll') -and
        $gitignoreText.Contains('!/Engine/ThirdPartyLib/PhysX/')) 'Engine/Client projects, UpdateLib, and gitignore all carry the PhysX contract'
```

### G01-14. 직접 작성 순서

1. `Engine/Public/Physics_Manager.h`, `Engine/Private/Physics_Manager.cpp`를 새로 만들고 Engine vcxproj/filters에 등록한다 (G01-4, G01-5, G01-12).
2. `GameInstance.h` 앵커 2곳, `GameInstance.cpp` include+앵커 2곳+함수 교체 2개를 반영한다 (G01-6, G01-7).
3. Engine x64 Debug 빌드 → `UpdateLib.bat Debug` 실행 (`Engine/Public` 변경이므로 필수).
4. `Client/Public/Physics_Tool.h`, `Client/Private/Physics_Tool.cpp`를 만들고 Client vcxproj/filters에 등록한다 (G01-8, G01-9, G01-12).
5. G01-10/G01-11을 현재 `MainApp.h/.cpp`에 재기준화한 뒤 `CPhysics_Tool` 전방 선언·enum·멤버와 include·ensure/render/free 분기만 반영한다. 08-05 참고 스냅샷을 그대로 복사하지 않는다.
6. audit 블록을 추가한다 (G01-13).
7. Client x64 Debug 빌드 → 자동 검증(G01-15) → runtime smoke(G01-16).

### G01-15. 자동 검증

```powershell
[xml](Get-Content -LiteralPath 'Engine/Default/Engine.vcxproj' -Raw) | Out-Null
[xml](Get-Content -LiteralPath 'Engine/Default/Engine.vcxproj.filters' -Raw) | Out-Null
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj' -Raw) | Out-Null
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj.filters' -Raw) | Out-Null

rg -n "Physics_Manager" Engine/Default/Engine.vcxproj Engine/Default/Engine.vcxproj.filters
rg -n "Physics_Tool" Client/Default/Client.vcxproj Client/Default/Client.vcxproj.filters
rg -n "Get_PhysicsManager|m_pPhysics_Manager" Engine/Public/GameInstance.h Engine/Private/GameInstance.cpp

$lostArkMsBuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
  -latest -products * -requires Microsoft.Component.MSBuild `
  -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
& $lostArkMsBuild Engine\Default\Engine.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
.\UpdateLib.bat Debug
& $lostArkMsBuild Client\Default\Client.vcxproj /p:Configuration=Debug /p:Platform=x64 /m

powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

기대 결과:

```text
4개 project/filter XML parse                     PASS
Physics_Manager 등록                             Engine vcxproj/filters 각 1건
Physics_Tool 등록                                Client vcxproj/filters 각 1건
GameInstance 파사드/멤버/배선                    h 2건 + cpp 4건 (include, Create, Update, Clear, reset)
Engine x64 Debug → UpdateLib → Client x64 Debug  PASS (링크 오류 0)
physics.g0-sdk-layout / physics.g1-subsystem-registration  PASS
git diff --check                                 PASS (공백 오류 0)
```

### G01-16. F1 Runtime smoke에서 보여야 하는 결과

1단계 — Client 단독 (Server 불필요, 통계 기반 판정):

```text
Client 실행 → LOBBY 진입
F1 → Developer Tools → Physics Tool 버튼
-> "Fixed steps last frame: 1" 내외, Accumulator < 0.0167 유지
Spawn Drop Test (기본 32개)
-> Level actors: 33 (plane 1 + box 32), Simulate cost 증가
수 초 대기 -> 박스들이 정지(sleep)하며 Simulate cost 감소
Clear Level Actors -> Level actors: 0
```

2단계 — `Framework.slnLaunch`의 Server + Client profile (시각 판정):

```text
Lobby → Test → Server 승인 → LEVEL::DEVELOPMENT Map Editor
F1 → Physics Tool → Spawn Drop Test
-> 주황 와이어 박스들이 낙하해 y=0 녹색 격자 위에 쌓이고 굴러 흩어짐
-> 정지한 박스가 회색(sleep)으로 전환
Lobby 복귀 → 다시 Test로 재진입
-> Physics Tool의 Level actors: 0 (레벨 전환 seam에서 자동 정리 확인)
종료 시 크래시/leak 리포트 없음 (PhysX 해제 순서 확인)
```

### G01-17. 이번 문서에서 의도적으로 남긴 다음 경계

- G02 (범용 rigid body bridge): stable handle, actor descriptor, `CRigidBody` 또는 동등한 Engine component, kinematic/dynamic 전환, gravity/velocity/pose API, `Post_Physics_Update`를 추가한다. G01의 raw `PxRigidActor*` level bucket을 Client에 노출하지 않는다.
- G03 (본체인 secondary motion): `CModel::Play_Animation`/`Post_Physics_Update` 역주입 훅, branch solver, `Data/Animation/Authored/<AssetId>/physics.json` 로더, Animation Tool branch 저작 모드. 설계 PLAN §3.2가 정본이다.
- G04 (플레이어 낙사): Server `fallVolume/VOID` 판정과 `FALLING` life state가 선행하고, Client가 G02 rigid body로 중력 낙하를 표현한다. 모든 `walkable=false`를 낙사로 해석하지 않는다.
- G05 (발탄 debris): Server destroyable state와 one-shot event, 동적 navigation, Shared 복제가 닫힌 뒤 파편 actor를 스폰한다. 지속 `FRACTURED` 상태 적용만으로 debris를 재생하지 않는다.
- Release 구성의 runtime smoke는 Developer Tools가 빠지므로 빌드/배포 검증까지만 수행한다 (Release에서 Physics Tool smoke를 PASS로 기록하지 않는다).
- 캐릭터 이동·판정·boss 페이즈의 물리화는 범위 밖이다. G04에서도 PhysX pose는 Client 표현이며 Server 권위 계약이 그대로 유지된다.
