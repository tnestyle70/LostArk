# LostArk IOCP·JobSystem 선택 비교 전체 코드 계획서

먼저 읽기: [Visual Studio에서 첫 파일부터 따라가는 GUIDE](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_GUIDE.md) · [측정 근거를 담은 기술소개서](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_TECHNICAL_NOTE.md) · [현재 IOCP 학습용 솔루션](../../../Study/CodeWalkthrough/CodeWalkthrough.sln)

현재 학습용 솔루션은 IOCP 설명본의 선언·정의만 정적 라이브러리로 컴파일한다. 이전 `out/ServerConcurrency20261009/build-view/ServerConcurrencyStudy.sln`과 성능 측정 자료는 현재 디스크에 없으며, 아래 전체 후보의 제품 연결이나 비교 하네스가 이미 반영됐다는 뜻은 아니다.

2026-10-09 · 기준 commit 45faba4b · 구현 후보와 전체 코드 정본

이 문서는 사용자가 직접 검토하고 적용할 구현 후보다. 제품 Server/Shared 소스에는 아직 설치하지 않았다. out/ServerConcurrency20261009/candidate에 파일별 전문을 함께 두었다. 기존 파일은 기준 commit과 이후 수정 사항을 비교해 필요한 변경만 병합한다. 전체 교체는 다른 작업 변경을 덮어쓸 수 있다.

기존 세션별 select 송수신과 새 IOCP, snapshot fanout의 serial과 Chase–Lev scheduler를 같은 API로 선택한다. 목표는 검증 가능한 비용 비교이며 성능 개선은 RESULT의 실측으로만 확정한다. 이전 08-03 IOCP 대안과 달리 현재 CClientSession의 coalescing·진단·reliable admission 계약을 보존한다.

## G01. 실제 변경 경계와 작성 순서

Shared deque/counter/scheduler → IOCP service → 공통 ClientSession의 backend 분기 → snapshot fanout → ServerApp/Main 선택과 수명 → 프로젝트 등록 → 정확성 검사와 ABBA 비교 순서다.

기존 gameplay Tick, 패킷 byte 규약, 서버 권위, 팀 LAN endpoint, Client/UI는 변경하지 않는다. 일반 F1/F7 CPU profiler와 deadlock lock-graph detector를 혼동하지 않는다. 후자는 이번 구현 범위가 아니다.

### 직접 반영하는 순서와 기본 선택

| 단계 | 반영 파일 | 확인할 결과 |
|---|---|---|
| G01 전송 계층 | IocpService.h/cpp, ClientSession.h/cpp, ClientSession_Iocp.cpp | 기존 queue/reliable/coalescing을 공유하고 select 또는 IOCP 선택 |
| G02 CPU 작업 계층 | ChaseLevDeque.h, WorkStealingJobSystem.h/cpp | last-item exactly-once, nested wait, overflow, exception, shutdown |
| G03 실제 소비자 | ServerSnapshotFanout.h/cpp, GameRoom.h/cpp, GameRoom_Replication.cpp | 동일 snapshot의 세션별 framing/enqueue만 serial/jobs 비교 |
| G04 생성·종료·옵션 | ServerConcurrencyOptions.h, ServerApp.h/cpp, Main.cpp | 기본 select+serial, accept/room thread와 gameplay single writer 유지 |
| G05 프로젝트·측정 | Server/Shared vcxproj와 filters, Tools/ServerConcurrencyHarness 전체 | 같은 Release 조건의 native correctness와 ABBA raw data |

IOCP를 먼저 이해하려면 G01을 먼저 읽는다. 이 통합 후보의 모든 파일을 반영한 뒤에도 snapshot-executor serial로 CPU 병렬화를 끈 상태에서 전송 효과만 측정할 수 있다. 일부 파일만 복사한 상태를 빌드 가능한 별도 패키지로 제공한 것은 아니다.

### 보존하는 흐름

```text
select recv thread 또는 IOCP receive completion
  → 기존 Shared parser → On_SessionFrame → room command queue
  → 기존 단일 30Hz room thread의 순차 gameplay Tick
  → snapshot encode 1회
  → serial 또는 Chase–Lev scheduler로 session별 framing/enqueue
  → select send thread 또는 IOCP send completion
```

IOCP service는 세션보다 오래 살고, 모든 session Stop과 posting 중단 뒤 service Stop을 호출한다. service 자체를 임의의 Submit과 동시에 Stop하는 API로 쓰지 않는다. 세션 Stop은 취소 완료와 이미 진입한 maintenance까지 기다리며, callback에서 자기 completion을 기다리는 join은 하지 않는다.

### 비용을 분리하는 네 실행 모드

```text
--transport select --snapshot-executor serial
--transport iocp --iocp-workers 2 --snapshot-executor serial
--transport select --snapshot-executor chaselev --job-workers 2
--transport iocp --iocp-workers 2 --snapshot-executor chaselev --job-workers 2
```

워커에 넘기면 enqueue, wakeup, 동기화, join, cache 이동 비용이 추가된다. 4인 방의 작은 작업은 직렬 처리가 더 유리할 수 있다. 현재 fanout은 외부 room thread의 root 제출이므로 injection queue를 주로 소비한다. localPushes/steals가 0인 결과를 Chase–Lev stealing의 성능 이득으로 해석하지 않는다.

## G02. 추가·수정 파일과 이유

| 작업 | 파일 | 위치 |
|---|---|---|
| 수정 | Server/Default/Server.vcxproj | [전체 코드](#file-server-default-server-vcxproj) |
| 수정 | Server/Default/Server.vcxproj.filters | [전체 코드](#file-server-default-server-vcxproj-filters) |
| 수정 | Server/Private/ClientSession.cpp | [전체 코드](#file-server-private-clientsession-cpp) |
| 추가 | Server/Private/ClientSession_Iocp.cpp | [전체 코드](#file-server-private-clientsession-iocp-cpp) |
| 수정 | Server/Private/GameRoom.cpp | [전체 코드](#file-server-private-gameroom-cpp) |
| 수정 | Server/Private/GameRoom_Replication.cpp | [전체 코드](#file-server-private-gameroom-replication-cpp) |
| 추가 | Server/Private/IocpService.cpp | [전체 코드](#file-server-private-iocpservice-cpp) |
| 수정 | Server/Private/Main.cpp | [전체 코드](#file-server-private-main-cpp) |
| 수정 | Server/Private/ServerApp.cpp | [전체 코드](#file-server-private-serverapp-cpp) |
| 추가 | Server/Private/ServerSnapshotFanout.cpp | [전체 코드](#file-server-private-serversnapshotfanout-cpp) |
| 수정 | Server/Public/ClientSession.h | [전체 코드](#file-server-public-clientsession-h) |
| 수정 | Server/Public/GameRoom.h | [전체 코드](#file-server-public-gameroom-h) |
| 추가 | Server/Public/IocpService.h | [전체 코드](#file-server-public-iocpservice-h) |
| 수정 | Server/Public/ServerApp.h | [전체 코드](#file-server-public-serverapp-h) |
| 추가 | Server/Public/ServerConcurrencyOptions.h | [전체 코드](#file-server-public-serverconcurrencyoptions-h) |
| 추가 | Server/Public/ServerSnapshotFanout.h | [전체 코드](#file-server-public-serversnapshotfanout-h) |
| 수정 | Shared/Default/Shared.vcxproj | [전체 코드](#file-shared-default-shared-vcxproj) |
| 수정 | Shared/Default/Shared.vcxproj.filters | [전체 코드](#file-shared-default-shared-vcxproj-filters) |
| 추가 | Shared/Private/Concurrency/WorkStealingJobSystem.cpp | [전체 코드](#file-shared-private-concurrency-workstealingjobsystem-cpp) |
| 추가 | Shared/Public/Concurrency/ChaseLevDeque.h | [전체 코드](#file-shared-public-concurrency-chaselevdeque-h) |
| 추가 | Shared/Public/Concurrency/WorkStealingJobSystem.h | [전체 코드](#file-shared-public-concurrency-workstealingjobsystem-h) |
| 추가 | Tools/ServerConcurrencyHarness/ComparisonProfiles.json | [전체 코드](#file-tools-serverconcurrencyharness-comparisonprofiles-json) |
| 추가 | Tools/ServerConcurrencyHarness/Default/ServerConcurrencyHarness.vcxproj | [전체 코드](#file-tools-serverconcurrencyharness-default-serverconcurrencyharness-vcxproj) |
| 추가 | Tools/ServerConcurrencyHarness/Default/ServerConcurrencyHarness.vcxproj.filters | [전체 코드](#file-tools-serverconcurrencyharness-default-serverconcurrencyharness-vcxproj-filters) |
| 추가 | Tools/ServerConcurrencyHarness/Private/JobSystemTests.cpp | [전체 코드](#file-tools-serverconcurrencyharness-private-jobsystemtests-cpp) |
| 추가 | Tools/ServerConcurrencyHarness/Private/Main.cpp | [전체 코드](#file-tools-serverconcurrencyharness-private-main-cpp) |
| 추가 | Tools/ServerConcurrencyHarness/Private/SnapshotFanoutBenchmark.cpp | [전체 코드](#file-tools-serverconcurrencyharness-private-snapshotfanoutbenchmark-cpp) |
| 추가 | Tools/ServerConcurrencyHarness/README.md | [전체 코드](#file-tools-serverconcurrencyharness-readme-md) |
| 추가 | Tools/ServerConcurrencyHarness/Run-ComparisonProfile.ps1 | [전체 코드](#file-tools-serverconcurrencyharness-run-comparisonprofile-ps1) |
| 추가 | Tools/ServerConcurrencyHarness/Run-TransportComparison.ps1 | [전체 코드](#file-tools-serverconcurrencyharness-run-transportcomparison-ps1) |

## G03. 파일별 계약과 전체 코드

<a id="file-server-default-server-vcxproj"></a>

### Server/Default/Server.vcxproj

파일: C:/Users/tnest/Desktop/LostArk/Server/Default/Server.vcxproj

물리 소스에 대응하는 build/IDE 항목이다. 기존 filter를 재배치하지 않고 필요한 항목만 등록한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\ClientSession.h" />
    <ClInclude Include="..\Public\GuideCatalog.h" />
    <ClInclude Include="..\Public\GameRoom.h" />
    <ClInclude Include="..\Public\RoomCommand.h" />
    <ClInclude Include="..\Public\ServerApp.h" />
    <ClInclude Include="..\Public\ServerIds.h" />
    <ClInclude Include="..\Public\ServerPlayer.h" />
    <ClInclude Include="..\Public\TcpListener.h" />
    <ClInclude Include="..\Public\WinSockContext.h" />
    <ClInclude Include="..\Public\ServerWorldEntity.h" />
    <ClInclude Include="..\Public\WorldBootstrap.h" />
    <ClInclude Include="..\Public\SpawnGroupBootstrap.h" />
    <ClInclude Include="..\Public\SpawnGroupRuntime.h" />
    <ClInclude Include="..\Public\MonsterBrain.h" />
	<ClInclude Include="..\Public\NpcBehaviorRuntime.h" />
	<ClInclude Include="..\Public\GameplayCatalog.h" />
	<ClInclude Include="..\Public\ServerBalanceNumericStore.h" />
	<ClInclude Include="..\Public\BossCombatRuntime.h" />
	<ClInclude Include="..\Public\CombatObjectRuntime.h" />
	<ClInclude Include="..\Public\ServerCombatGeometry.h" />
    <ClInclude Include="..\Public\ColosseumThreatAssessment.h" />
	<ClInclude Include="..\Public\ServerCombatHitRuntime.h" />
	<ClInclude Include="..\Public\PlayerSkillSystem.h" />
    <ClInclude Include="..\Public\ColosseumCombatPolicy.h" />
	<ClInclude Include="..\Public\ServerNavigation.h" />
	<ClInclude Include="..\Public\ValtanBrain.h" />
	<ClInclude Include="..\Public\KoukuSaydonBrain.h" />
	<ClInclude Include="..\Public\KoukuSaydonLogicRuntime.h" />
	<ClInclude Include="..\Public\EstherSkillSystem.h" />
	<ClInclude Include="..\Public\ServerGameplayContractTests.h" />
	<ClInclude Include="..\Public\ServerTriggerSystem.h" />
	<ClInclude Include="..\Public\ServerCollisionSystem.h" />
	<ClInclude Include="..\Public\EncounterPropRuntime.h" />
	<ClInclude Include="..\Public\WorldDestructionRuntime.h" />
	<ClInclude Include="..\Public\WorldDestructionBootstrap.h" />
	<ClInclude Include="..\Public\WorldDestructionBootstrapContractTests.h" />
	<ClInclude Include="..\Public\ItemCatalog.h" />
	<ClInclude Include="..\Public\VehicleCatalog.h" />
	<ClInclude Include="..\Public\HonorTitleCatalog.h" />
	<ClInclude Include="..\Public\ValtanClearRewards.h" />
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\ClientSession.cpp" />
    <ClCompile Include="..\Private\GuideCatalog.cpp" />
    <ClCompile Include="..\Private\GameRoom_MaharakaAI.cpp" />
    <ClCompile Include="..\Private\ServerGameplayContractTests_MaharakaAI.cpp" />
    <ClCompile Include="..\Private\GameRoom_Guide.cpp" />
    <ClCompile Include="..\Private\GameRoom_Colosseum.cpp" />
    <ClCompile Include="..\Private\GameRoom_GuideThreat.cpp" />
    <ClCompile Include="..\Private\ServerGameplayContractTests_Guide.cpp" />
    <ClCompile Include="..\Private\GameRoom.cpp" />
    <ClCompile Include="..\Private\Main.cpp" />
    <ClCompile Include="..\Private\ServerApp.cpp" />
    <ClCompile Include="..\Private\WinSockContext.cpp" />
    <ClCompile Include="..\Private\TcpListener.cpp" />
    <ClCompile Include="..\Private\WorldBootstrap.cpp" />
    <ClCompile Include="..\Private\SpawnGroupBootstrap.cpp" />
    <ClCompile Include="..\Private\SpawnGroupRuntime.cpp" />
    <ClCompile Include="..\Private\MonsterBrain.cpp" />
	<ClCompile Include="..\Private\NpcBehaviorRuntime.cpp" />
	<ClCompile Include="..\Private\GameplayCatalog.cpp" />
	<ClCompile Include="..\Private\ServerBalanceNumericStore.cpp" />
	<ClCompile Include="..\Private\BossCombatRuntime.cpp" />
	<ClCompile Include="..\Private\CombatObjectRuntime.cpp" />
	<ClCompile Include="..\Private\ServerCombatGeometry.cpp" />
    <ClCompile Include="..\Private\ColosseumThreatAssessment.cpp" />
	<ClCompile Include="..\Private\ServerCombatHitRuntime.cpp" />
	<ClCompile Include="..\Private\PlayerSkillSystem.cpp" />
	<ClCompile Include="..\Private\ServerNavigation.cpp" />
	<ClCompile Include="..\Private\ValtanBrain.cpp" />
	<ClCompile Include="..\Private\KoukuSaydonBrain.cpp" />
	<ClCompile Include="..\Private\KoukuSaydonLogicRuntime.cpp" />
	<ClCompile Include="..\Private\EstherSkillSystem.cpp" />
	<ClCompile Include="..\Private\ServerGameplayContractTests.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
	<ClCompile Include="..\Private\ServerTriggerSystem.cpp" />
	<ClCompile Include="..\Private\ServerCollisionSystem.cpp" />
	<ClCompile Include="..\Private\EncounterPropRuntime.cpp" />
	<ClCompile Include="..\Private\WorldDestructionRuntime.cpp" />
	<ClCompile Include="..\Private\WorldDestructionBootstrap.cpp" />
	<ClCompile Include="..\Private\WorldDestructionBootstrapContractTests.cpp" />
	<ClCompile Include="..\Private\ItemCatalog.cpp" />
	<ClCompile Include="..\Private\VehicleCatalog.cpp" />
	<ClCompile Include="..\Private\HonorTitleCatalog.cpp" />
	<ClCompile Include="..\Private\ValtanClearRewards.cpp" />
  </ItemGroup>
  <ItemGroup>
    <ProjectReference Include="..\..\Shared\Default\Shared.vcxproj">
      <Project>{F4CCF815-6D51-412F-A76E-84D2F1D05571}</Project>
    </ProjectReference>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <Keyword>Win32Proj</Keyword>
    <ProjectGuid>{053788E7-C377-4811-8AFE-BC23D9BE4AE7}</ProjectGuid>
    <RootNamespace>Server</RootNamespace>
    <ProjectName>Server</ProjectName>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings" />
  <ImportGroup Label="Shared" />
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <OutDir>$(ProjectDir)..\Bin\$(Configuration)\</OutDir>
    <IntDir>$(ProjectDir)..\Intermediate\$(Platform)\$(Configuration)\</IntDir>
    <TargetName>Server</TargetName>
    <LocalDebuggerCommandArguments>--bind-address 0.0.0.0</LocalDebuggerCommandArguments>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\Public;$(ProjectDir)..\..\Shared\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>Ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\Public;$(ProjectDir)..\..\Shared\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>Ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\Public;$(ProjectDir)..\..\Shared\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>Ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>NDEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\Public;$(ProjectDir)..\..\Shared\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Link>
      <SubSystem>Console</SubSystem>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <OptimizeReferences>true</OptimizeReferences>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <AdditionalDependencies>Ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <Target Name="PublishServerBuildDomains" BeforeTargets="ClCompile" Condition="'$(Platform)'=='x64' and '$(LostArkPublishRuntimeData)'=='true'">
    <Exec Command="powershell.exe -NoProfile -ExecutionPolicy Bypass -File &quot;$(ProjectDir)..\..\Tools\Build\Invoke-BuildDomainOwner.ps1&quot; -Owner Server" />
  </Target>
  <Import Project="..\..\Tools\Build\CppCompilation.props" />
  <ItemGroup>
    <ClCompile Include="..\Private\Server_Pch.cpp">
      <PrecompiledHeader Condition="'$(LostArkUsePch)'=='true' and '$(Platform)'=='x64'">Create</PrecompiledHeader>
      <ExcludedFromBuild Condition="'$(LostArkUsePch)'!='true' or '$(Platform)'!='x64'">true</ExcludedFromBuild>
    </ClCompile>
    <ClInclude Include="..\..\Tools\Build\CppStandardPch.h" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets" />
  <ItemGroup>
    <ClInclude Include="..\Private\GameRoom_Internal.h" />
    <ClCompile Include="..\Private\GameRoom_CatalogGenerations.cpp" />
    <ClCompile Include="..\Private\GameRoom_Admission.cpp" />
    <ClCompile Include="..\Private\GameRoom_PlayerCommands.cpp" />
    <ClCompile Include="..\Private\GameRoom_KoukuPlayerCommands.cpp" />
    <ClCompile Include="..\Private\GameRoom_VehicleRiding.cpp" />
    <ClCompile Include="..\Private\GameRoom_GateProgress.cpp" />
    <ClCompile Include="..\Private\GameRoom_HonorTitle.cpp" />
    <ClCompile Include="..\Private\GameRoom_Inventory.cpp" />
    <ClCompile Include="..\Private\GameRoom_PartyWorld.cpp" />
    <ClCompile Include="..\Private\GameRoom_KoukuAudition.cpp" />
    <ClCompile Include="..\Private\GameRoom_ValtanAudition.cpp" />
    <ClCompile Include="..\Private\GameRoom_Replication.cpp" />
    <ClCompile Include="..\Private\GameRoom_WorldEntities.cpp" />
    <ClCompile Include="..\Private\GameRoom_BossStageActions.cpp" />
    <ClCompile Include="..\Private\GameRoom_WorldDestruction.cpp" />
    <ClCompile Include="..\Private\GameRoom_KoukuMiniGames.cpp" />
    <ClCompile Include="..\Private\GameRoom_KoukuRaidFlow.cpp" />
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuRaid.cpp" />
    <ClCompile Include="..\Private\GameRoom_PlayerSimulation.cpp" />
    <ClCompile Include="..\Private\GameRoom_BossSimulation.cpp" />
    <ClCompile Include="..\Private\GameRoom_Helpers.cpp" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Private\ServerGameplayContractTests_Internal.h" />
    <ClInclude Include="..\Private\ServerGameplayContractTests_PinnedGenerationFixture.h" />
    <ClCompile Include="..\Private\ServerGameplayContractTests_Helpers.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuLogic.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_WorldPlayback.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_DebugTeleport.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuBundles.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuProduct.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanLifecycle.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanPinnedGeneration.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanRevision.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_RevisionProtocol.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_GenerationRetention.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_SessionTransport.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_RoomIngress.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_CharacterAdmission.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_GroundTarget.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_PlayerCombos.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_PlayerActions.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanMechanicLedger.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanSkyAxe.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanTimelines.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_WorldTriggers.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ColosseumCombat.cpp" />
    <ClCompile Include="..\Private\ServerGameplayContractTests_ColosseumMatch.cpp" />
    <ClCompile Include="..\Private\ServerGameplayContractTests_SkillStages.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_SpawnGroups.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanDash.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanAudition.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanResetlessNext.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanReleaseControl.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_WorldDestruction.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClInclude Include="..\Private\ServerGameplayContractTests_Runner.h" />
    <ClCompile Include="..\Private\ServerGameplayContractTests_Runner.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuOverlap.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_CardMaze.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_Bingo.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_VehicleRiding.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuSupportSurface.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_Navigation.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
    <ClInclude Include="..\Private\ServerGameplayContractTests_PlayerSkillFixtures.h" />
    <ClCompile Include="..\Private\ServerGameplayContractTests_BattleItems.cpp">
	  <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
	</ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\IocpService.h" />
    <ClInclude Include="..\Public\ServerConcurrencyOptions.h" />
    <ClInclude Include="..\Public\ServerSnapshotFanout.h" />
    <ClCompile Include="..\Private\IocpService.cpp" />
    <ClCompile Include="..\Private\ClientSession_Iocp.cpp" />
    <ClCompile Include="..\Private\ServerSnapshotFanout.cpp" />
  </ItemGroup>
</Project>
~~~~

<a id="file-server-default-server-vcxproj-filters"></a>

### Server/Default/Server.vcxproj.filters

파일: C:/Users/tnest/Desktop/LostArk/Server/Default/Server.vcxproj.filters

물리 소스에 대응하는 build/IDE 항목이다. 기존 filter를 재배치하지 않고 필요한 항목만 등록한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~xml
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003" ToolsVersion="4.0">
  <ItemGroup>
    <Filter Include="00.Main">
      <UniqueIdentifier>{9E987759-92B9-4125-A5A7-05495FB9F53B}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Network">
      <UniqueIdentifier>{88429166-D905-5ABF-9AF6-0E2952F5FFC4}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.Room">
      <UniqueIdentifier>{955A4FE2-BA15-4E49-9879-188CCD1C93AD}</UniqueIdentifier>
    </Filter>
    <Filter Include="03.World">
      <UniqueIdentifier>{95AE9072-D006-58D9-9B30-0765FD984756}</UniqueIdentifier>
    </Filter>
    <Filter Include="04.NavigationCollision">
      <UniqueIdentifier>{C14E1972-0AF1-5F76-AFD7-E45885B1CE9D}</UniqueIdentifier>
    </Filter>
    <Filter Include="05.Combat">
      <UniqueIdentifier>{1A25AB83-58E0-5A2A-BF23-D8AEABCE1846}</UniqueIdentifier>
    </Filter>
    <Filter Include="06.AI">
      <UniqueIdentifier>{11ADE366-8B92-5B2E-A10C-C5AF7A610D27}</UniqueIdentifier>
    </Filter>
    <Filter Include="07.Boss">
      <UniqueIdentifier>{FC97BBB5-612E-5F97-8477-ABA02E54425B}</UniqueIdentifier>
    </Filter>
    <Filter Include="07.Boss\00.Shared">
      <UniqueIdentifier>{34ABBC60-5363-5CCC-95F2-5393216630BB}</UniqueIdentifier>
    </Filter>
    <Filter Include="07.Boss\01.Valtan">
      <UniqueIdentifier>{E13F3F63-427F-5C16-98AE-6C6E413DAE2E}</UniqueIdentifier>
    </Filter>
    <Filter Include="07.Boss\02.KoukuSaydon">
      <UniqueIdentifier>{A0E4B3AA-0B27-543C-A0C9-4BD65448C94A}</UniqueIdentifier>
    </Filter>
    <Filter Include="08.Economy">
      <UniqueIdentifier>{F4C5526C-9F5D-5BBB-A2E6-E3DA76630DD1}</UniqueIdentifier>
    </Filter>
    <Filter Include="09.Generation">
      <UniqueIdentifier>{522A97D5-961F-5B7E-87C7-799D76CE02E6}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests">
      <UniqueIdentifier>{3C53ACAC-C3A4-5CF9-89D9-2CEF502775A6}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\00.Framework">
      <UniqueIdentifier>{AED22ED0-2A73-5C17-B61F-A53FF71D2FB4}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\01.Network">
      <UniqueIdentifier>{BB9C5A90-1CE8-5798-810D-D73D4118A022}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\02.Room">
      <UniqueIdentifier>{4C96A668-3B7A-5C06-BB62-568072A5E0B2}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\03.World">
      <UniqueIdentifier>{48032E14-4035-5CE9-B6C3-5E62241A1826}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\04.NavigationCollision">
      <UniqueIdentifier>{9627CAEC-81EF-570F-93C6-07FDCFC59CF5}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\05.Combat">
      <UniqueIdentifier>{5F782A57-E9CA-578C-845E-2487CD5B865C}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\06.AI">
      <UniqueIdentifier>{24128066-82F8-5F84-8012-1C5848E9DCD3}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\07.Boss">
      <UniqueIdentifier>{3B9140A1-D3EB-5A73-B525-ACE184FB010D}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\07.Boss\01.Valtan">
      <UniqueIdentifier>{E066EA25-D145-53D4-9FD1-293E8C853BA8}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\07.Boss\02.KoukuSaydon">
      <UniqueIdentifier>{4C86CACB-5FB1-5D08-99E4-870A3F68152F}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\08.Economy">
      <UniqueIdentifier>{DB5AB541-4FF4-5AAF-9D71-C298FACA42AB}</UniqueIdentifier>
    </Filter>
    <Filter Include="98.Tests\09.Generation">
      <UniqueIdentifier>{DBC2D3E1-57E1-5EB6-82A1-76EC1160A681}</UniqueIdentifier>
    </Filter>
    <Filter Include="99.Default">
      <UniqueIdentifier>{92B78042-E943-5FE7-BE6E-ADDB63CDCC58}</UniqueIdentifier>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\Main.cpp">
      <Filter>00.Main</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerApp.cpp">
      <Filter>00.Main</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ClientSession.cpp">
      <Filter>01.Network</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\TcpListener.cpp">
      <Filter>01.Network</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WinSockContext.cpp">
      <Filter>01.Network</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom.cpp">
      <Filter>02.Room</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_Admission.cpp">
      <Filter>02.Room</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_Helpers.cpp">
      <Filter>02.Room</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_PartyWorld.cpp">
      <Filter>02.Room</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_Replication.cpp">
      <Filter>02.Room</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_WorldDestruction.cpp">
      <Filter>03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_WorldEntities.cpp">
      <Filter>03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SpawnGroupBootstrap.cpp">
      <Filter>03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\SpawnGroupRuntime.cpp">
      <Filter>03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldBootstrap.cpp">
      <Filter>03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldDestructionBootstrap.cpp">
      <Filter>03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldDestructionRuntime.cpp">
      <Filter>03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerCollisionSystem.cpp">
      <Filter>04.NavigationCollision</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerNavigation.cpp">
      <Filter>04.NavigationCollision</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerTriggerSystem.cpp">
      <Filter>04.NavigationCollision</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EstherSkillSystem.cpp">
      <Filter>05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameplayCatalog.cpp">
      <Filter>05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_Colosseum.cpp">
      <Filter>05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_PlayerCommands.cpp">
      <Filter>05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_PlayerSimulation.cpp">
      <Filter>05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\PlayerSkillSystem.cpp">
      <Filter>05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerCombatGeometry.cpp">
      <Filter>05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ColosseumThreatAssessment.cpp">
      <Filter>06.AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerCombatHitRuntime.cpp">
      <Filter>05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_Guide.cpp">
      <Filter>06.AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_GuideThreat.cpp">
      <Filter>06.AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_MaharakaAI.cpp">
      <Filter>06.AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GuideCatalog.cpp">
      <Filter>06.AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\MonsterBrain.cpp">
      <Filter>06.AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\NpcBehaviorRuntime.cpp">
      <Filter>06.AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\BossCombatRuntime.cpp">
      <Filter>07.Boss\00.Shared</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\CombatObjectRuntime.cpp">
      <Filter>07.Boss\00.Shared</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\EncounterPropRuntime.cpp">
      <Filter>07.Boss\00.Shared</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_BossSimulation.cpp">
      <Filter>07.Boss\00.Shared</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_BossStageActions.cpp">
      <Filter>07.Boss\00.Shared</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_GateProgress.cpp">
      <Filter>07.Boss\00.Shared</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_ValtanAudition.cpp">
      <Filter>07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanBrain.cpp">
      <Filter>07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_KoukuAudition.cpp">
      <Filter>07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_KoukuMiniGames.cpp">
      <Filter>07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_KoukuPlayerCommands.cpp">
      <Filter>07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_KoukuRaidFlow.cpp">
      <Filter>07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonBrain.cpp">
      <Filter>07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\KoukuSaydonLogicRuntime.cpp">
      <Filter>07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_HonorTitle.cpp">
      <Filter>08.Economy</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_Inventory.cpp">
      <Filter>08.Economy</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_VehicleRiding.cpp">
      <Filter>08.Economy</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\HonorTitleCatalog.cpp">
      <Filter>08.Economy</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ItemCatalog.cpp">
      <Filter>08.Economy</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ValtanClearRewards.cpp">
      <Filter>08.Economy</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\VehicleCatalog.cpp">
      <Filter>08.Economy</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameRoom_CatalogGenerations.cpp">
      <Filter>09.Generation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerBalanceNumericStore.cpp">
      <Filter>09.Generation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests.cpp">
      <Filter>98.Tests\00.Framework</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_Helpers.cpp">
      <Filter>98.Tests\00.Framework</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_Runner.cpp">
      <Filter>98.Tests\00.Framework</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_SessionTransport.cpp">
      <Filter>98.Tests\01.Network</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_CharacterAdmission.cpp">
      <Filter>98.Tests\02.Room</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_RoomIngress.cpp">
      <Filter>98.Tests\02.Room</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_SpawnGroups.cpp">
      <Filter>98.Tests\03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_WorldDestruction.cpp">
      <Filter>98.Tests\03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_WorldPlayback.cpp">
      <Filter>98.Tests\03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_WorldTriggers.cpp">
      <Filter>98.Tests\03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\WorldDestructionBootstrapContractTests.cpp">
      <Filter>98.Tests\03.World</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_DebugTeleport.cpp">
      <Filter>98.Tests\04.NavigationCollision</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_Navigation.cpp">
      <Filter>98.Tests\04.NavigationCollision</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ColosseumCombat.cpp">
      <Filter>98.Tests\05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ColosseumMatch.cpp">
      <Filter>98.Tests\05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_GroundTarget.cpp">
      <Filter>98.Tests\05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_PlayerActions.cpp">
      <Filter>98.Tests\05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_PlayerCombos.cpp">
      <Filter>98.Tests\05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_SkillStages.cpp">
      <Filter>98.Tests\05.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_Guide.cpp">
      <Filter>98.Tests\06.AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_MaharakaAI.cpp">
      <Filter>98.Tests\06.AI</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanAudition.cpp">
      <Filter>98.Tests\07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanDash.cpp">
      <Filter>98.Tests\07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanLifecycle.cpp">
      <Filter>98.Tests\07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanMechanicLedger.cpp">
      <Filter>98.Tests\07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanPinnedGeneration.cpp">
      <Filter>98.Tests\07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanReleaseControl.cpp">
      <Filter>98.Tests\07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanResetlessNext.cpp">
      <Filter>98.Tests\07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanRevision.cpp">
      <Filter>98.Tests\07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanSkyAxe.cpp">
      <Filter>98.Tests\07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_ValtanTimelines.cpp">
      <Filter>98.Tests\07.Boss\01.Valtan</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_Bingo.cpp">
      <Filter>98.Tests\07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_CardMaze.cpp">
      <Filter>98.Tests\07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuBundles.cpp">
      <Filter>98.Tests\07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuLogic.cpp">
      <Filter>98.Tests\07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuOverlap.cpp">
      <Filter>98.Tests\07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuProduct.cpp">
      <Filter>98.Tests\07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuRaid.cpp">
      <Filter>98.Tests\07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_KoukuSupportSurface.cpp">
      <Filter>98.Tests\07.Boss\02.KoukuSaydon</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_BattleItems.cpp">
      <Filter>98.Tests\08.Economy</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_VehicleRiding.cpp">
      <Filter>98.Tests\08.Economy</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_GenerationRetention.cpp">
      <Filter>98.Tests\09.Generation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\ServerGameplayContractTests_RevisionProtocol.cpp">
      <Filter>98.Tests\09.Generation</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Server_Pch.cpp">
      <Filter>99.Default</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\ServerApp.h">
      <Filter>00.Main</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ClientSession.h">
      <Filter>01.Network</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\TcpListener.h">
      <Filter>01.Network</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WinSockContext.h">
      <Filter>01.Network</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\GameRoom.h">
      <Filter>02.Room</Filter>
    </ClInclude>
    <ClInclude Include="..\Private\GameRoom_Internal.h">
      <Filter>02.Room</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\RoomCommand.h">
      <Filter>02.Room</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ServerIds.h">
      <Filter>02.Room</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ServerPlayer.h">
      <Filter>02.Room</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ServerWorldEntity.h">
      <Filter>03.World</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SpawnGroupBootstrap.h">
      <Filter>03.World</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\SpawnGroupRuntime.h">
      <Filter>03.World</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldBootstrap.h">
      <Filter>03.World</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldDestructionBootstrap.h">
      <Filter>03.World</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldDestructionRuntime.h">
      <Filter>03.World</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ServerCollisionSystem.h">
      <Filter>04.NavigationCollision</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ServerNavigation.h">
      <Filter>04.NavigationCollision</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ServerTriggerSystem.h">
      <Filter>04.NavigationCollision</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ColosseumCombatPolicy.h">
      <Filter>05.Combat</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EstherSkillSystem.h">
      <Filter>05.Combat</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\GameplayCatalog.h">
      <Filter>05.Combat</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\PlayerSkillSystem.h">
      <Filter>05.Combat</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ServerCombatGeometry.h">
      <Filter>05.Combat</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ColosseumThreatAssessment.h">
      <Filter>06.AI</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ServerCombatHitRuntime.h">
      <Filter>05.Combat</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\GuideCatalog.h">
      <Filter>06.AI</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\MonsterBrain.h">
      <Filter>06.AI</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\NpcBehaviorRuntime.h">
      <Filter>06.AI</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\BossCombatRuntime.h">
      <Filter>07.Boss\00.Shared</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\CombatObjectRuntime.h">
      <Filter>07.Boss\00.Shared</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\EncounterPropRuntime.h">
      <Filter>07.Boss\00.Shared</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanBrain.h">
      <Filter>07.Boss\01.Valtan</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonBrain.h">
      <Filter>07.Boss\02.KoukuSaydon</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\KoukuSaydonLogicRuntime.h">
      <Filter>07.Boss\02.KoukuSaydon</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\HonorTitleCatalog.h">
      <Filter>08.Economy</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ItemCatalog.h">
      <Filter>08.Economy</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ValtanClearRewards.h">
      <Filter>08.Economy</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\VehicleCatalog.h">
      <Filter>08.Economy</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ServerBalanceNumericStore.h">
      <Filter>09.Generation</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\ServerGameplayContractTests.h">
      <Filter>98.Tests\00.Framework</Filter>
    </ClInclude>
    <ClInclude Include="..\Private\ServerGameplayContractTests_Internal.h">
      <Filter>98.Tests\00.Framework</Filter>
    </ClInclude>
    <ClInclude Include="..\Private\ServerGameplayContractTests_Runner.h">
      <Filter>98.Tests\00.Framework</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\WorldDestructionBootstrapContractTests.h">
      <Filter>98.Tests\03.World</Filter>
    </ClInclude>
    <ClInclude Include="..\Private\ServerGameplayContractTests_PlayerSkillFixtures.h">
      <Filter>98.Tests\05.Combat</Filter>
    </ClInclude>
    <ClInclude Include="..\Private\ServerGameplayContractTests_PinnedGenerationFixture.h">
      <Filter>98.Tests\09.Generation</Filter>
    </ClInclude>
    <ClInclude Include="..\..\Tools\Build\CppStandardPch.h">
      <Filter>99.Default</Filter>
    </ClInclude>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\IocpService.h"><Filter>01.Network</Filter></ClInclude>
    <ClInclude Include="..\Public\ServerConcurrencyOptions.h"><Filter>00.Main</Filter></ClInclude>
    <ClInclude Include="..\Public\ServerSnapshotFanout.h"><Filter>02.Room</Filter></ClInclude>
    <ClCompile Include="..\Private\IocpService.cpp"><Filter>01.Network</Filter></ClCompile>
    <ClCompile Include="..\Private\ClientSession_Iocp.cpp"><Filter>01.Network</Filter></ClCompile>
    <ClCompile Include="..\Private\ServerSnapshotFanout.cpp"><Filter>02.Room</Filter></ClCompile>
  </ItemGroup>
</Project>
~~~~

<a id="file-server-private-clientsession-cpp"></a>

### Server/Private/ClientSession.cpp

파일: C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp

공통 packet framing, queue 상한, snapshot coalescing, reliable transaction, 진단을 유지한다. backend에 따라 Start/Stop/Wake_Sender만 선택하고 기존 select thread 경로도 유지한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~cpp
#include "ClientSession.h"

#include <Windows.h>
#include <WS2tcpip.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <span>
#include <utility>
#include <vector>

namespace
{
	std::uint64_t To_Microseconds(
		const std::chrono::steady_clock::duration duration)
	{
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::microseconds>(
				duration).count());
	}

	std::uint64_t Current_UnixMilliseconds()
	{
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count());
	}
	int Wait_SocketReady(const SOCKET socket, const bool writing,
		const std::uint32_t waitMilliseconds)
	{
		fd_set ready;
		FD_ZERO(&ready);
		FD_SET(socket, &ready);
		const timeval timeout{static_cast<long>(waitMilliseconds / 1000u),
			static_cast<long>((waitMilliseconds % 1000u) * 1000u)};
		return ::select(0, writing ? nullptr : &ready,
			writing ? &ready : nullptr, nullptr, &timeout);
	}

}

struct LostArk::Server::CClientSession::RELIABLE_BATCH_TRANSACTION::LOCKED_QUEUE
{
	std::shared_ptr<CClientSession> pSession;
	std::unique_lock<std::mutex> DiagnosticLock;
	std::unique_lock<std::mutex> Lock;
	std::deque<OUTBOUND_FRAME> Frames;
	std::size_t iQueuedBytes = 0u;
	std::size_t iAddedFrameCount = 0u;
};

LostArk::Server::CClientSession::RELIABLE_BATCH_TRANSACTION::RELIABLE_BATCH_TRANSACTION() = default;
LostArk::Server::CClientSession::RELIABLE_BATCH_TRANSACTION::~RELIABLE_BATCH_TRANSACTION() = default;

bool LostArk::Server::CClientSession::RELIABLE_BATCH_TRANSACTION::Prepare(
	const std::vector<CLIENT_SESSION_RELIABLE_BATCH>& batches,
	std::string& status)
{
	using namespace LostArk::Shared;
	m_Queues.clear();
	status.clear();
	const auto reject = [this, &status](const char* reason)
	{
		status = reason;
		m_Queues.clear();
		return false;
	};
	if (batches.empty())
		return reject("reliable admission batch is empty");
	std::vector<const CLIENT_SESSION_RELIABLE_BATCH*> ordered;
	ordered.reserve(batches.size());
	for (const auto& batch : batches)
	{
		if (nullptr == batch.pSession ||
			INVALID_SESSION_ID == batch.pSession->Get_SessionId() ||
			batch.Frames.empty())
		{
			return reject("reliable admission batch has invalid session/frames");
		}
		ordered.push_back(&batch);
	}
	std::sort(ordered.begin(), ordered.end(), [](const auto* left, const auto* right)
	{
		return left->pSession->Get_SessionId() < right->pSession->Get_SessionId();
	});
	for (std::size_t index = 0; index < ordered.size(); ++index)
	{
		const auto& batch = *ordered[index];
		if (0u != index && ordered[index - 1u]->pSession->Get_SessionId() ==
			batch.pSession->Get_SessionId())
		{
			return reject("reliable admission batch contains a duplicate session");
		}
		auto staged = std::make_unique<LOCKED_QUEUE>();
		staged->pSession = batch.pSession;
		// Match Record_TerminalDiagnostic's lock order. A disconnect either
		// wins before admission (reject) or starts after the whole commit.
		staged->DiagnosticLock = std::unique_lock{ batch.pSession->m_DiagnosticMutex };
		staged->Lock = std::unique_lock{ batch.pSession->m_OutboundMutex };
		CClientSession& session = *batch.pSession;
		if (!session.m_isSendRunning.load() || session.m_closeAfterOutboundFlush.load() ||
			SESSION_DIAGNOSTIC_REASON::NONE != session.m_CloseDiagnostic.eReason)
			return reject("reliable admission participant is terminal");
		if (batch.Frames.size() > MAX_OUTBOUND_FRAME_COUNT -
			(std::min)(session.m_OutboundFrames.size(), MAX_OUTBOUND_FRAME_COUNT))
		{
			return reject("reliable admission frame capacity is exhausted");
		}
		staged->Frames = session.m_OutboundFrames;
		staged->iQueuedBytes = session.m_iQueuedOutboundBytes;
		for (const PACKET_FRAME& frame : batch.Frames)
		{
			std::vector<std::uint8_t> bytes;
			if (PACKET_TYPE::S2C_WORLD_SNAPSHOT == frame.ePacketType ||
				!Build_Packet_Frame(frame.ePacketType, frame.Payload, bytes))
			{
				return reject("reliable admission frame failed encoding");
			}
			if (bytes.size() > MAX_OUTBOUND_BYTE_COUNT -
				(std::min)(staged->iQueuedBytes, MAX_OUTBOUND_BYTE_COUNT))
			{
				return reject("reliable admission byte capacity is exhausted");
			}
			staged->iQueuedBytes += bytes.size();
			staged->Frames.push_back({ frame.ePacketType, std::move(bytes) });
		}
		staged->iAddedFrameCount = batch.Frames.size();
		m_Queues.push_back(std::move(staged));
	}
	return true;
}

void LostArk::Server::CClientSession::RELIABLE_BATCH_TRANSACTION::Commit() noexcept
{
	for (const auto& staged : m_Queues)
	{
		CClientSession& session = *staged->pSession;
		session.m_OutboundFrames.swap(staged->Frames);
		session.m_iQueuedOutboundBytes = staged->iQueuedBytes;
		auto& metrics = session.m_OutboundMetrics;
		metrics.iReliableEnqueuedFrameCount += staged->iAddedFrameCount;
		metrics.iCurrentQueuedFrameCount = session.m_OutboundFrames.size();
		metrics.iCurrentQueuedByteCount = session.m_iQueuedOutboundBytes;
		metrics.iQueuedFrameHighWatermark = (std::max)(
			metrics.iQueuedFrameHighWatermark, metrics.iCurrentQueuedFrameCount);
		metrics.iQueuedByteHighWatermark = (std::max)(
			metrics.iQueuedByteHighWatermark, metrics.iCurrentQueuedByteCount);
	}
	for (const auto& staged : m_Queues)
	{
		staged->Lock.unlock();
		staged->DiagnosticLock.unlock();
	}
	for (const auto& staged : m_Queues)
		staged->pSession->Wake_Sender();
	m_Queues.clear();
}

LostArk::Server::CClientSession::CClientSession(
	SESSION_ID sessionId,
	SOCKET clientSocket,
	FRAME_HANDLER onFrame,
	CLOSED_HANDLER onClosed,
	SESSION_TRANSPORT_BACKEND backend,
	CIocpService* iocpService)
	: m_iSessionId{ sessionId }
	, m_PeerEndpoint{ Resolve_PeerEndpoint(clientSocket) }
	, m_hClientSocket{ clientSocket }
	, m_TransportBackend{ backend }
	, m_pIocpService{ iocpService }
	, m_OnFrame{ std::move(onFrame) }
	, m_OnClosed{ std::move(onClosed) }
{}


LostArk::Server::CClientSession::~CClientSession()
{
	Stop();
}

bool LostArk::Server::CClientSession::Start()
{
	// The caller invokes Start without the ServerApp registry lock. Startup
	// failures may notify synchronously, through the same exactly-once gate as
	// receive, send, and IOCP maintenance failures.
	const auto failStart = [this]()
	{
		Request_Close();
		Notify_Closed();
		return false;
	};
	if (!Is_Open() ||
		m_iSessionId == INVALID_SESSION_ID ||
		m_ReceiveThread.joinable() ||
		m_SendThread.joinable() || m_isIocpStarted.load())
	{
		Record_TerminalDiagnostic(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_START_FAILED,
			0,
			"invalid session start state");
		return failStart();
	}
	if (!Configure_TransportOptions())
	{
		return failStart();
	}

	m_iLastErrorCode.store(0);
	m_hasNotifiedClosed.store(false);
	m_closeAfterOutboundFlush.store(false);
	m_iTerminalDrainStartTicks.store(0u);
	m_isReceiveRunning.store(true);
	m_isSendRunning.store(true);
	{
		std::scoped_lock lock{ m_OutboundMutex };
		m_OutboundFrames.clear();
		m_iQueuedOutboundBytes = 0u;
		m_OutboundMetrics = {};
		m_hasSenderExited = false;
		m_iSendStallOrdinal = 0u;
	}

	try
	{
		if (SESSION_TRANSPORT_BACKEND::IOCP == m_TransportBackend)
			return Start_Iocp() ? true : failStart();
		m_SendThread = std::thread(
			&CClientSession::Sender_Loop,
			this);
		m_ReceiveThread = std::thread(
			&CClientSession::Receive_Loop,
			this);
	}
	catch (...)
	{
		Request_Close(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_START_FAILED,
			0,
			"session worker creation failed");
		Stop();
		return failStart();
	}

	return true;
}

void LostArk::Server::CClientSession::Request_Close(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const int nativeErrorCode,
	const std::string_view context)
{
	Record_TerminalDiagnostic(reason, nativeErrorCode, context);
	m_isReceiveRunning.store(false);
	m_isSendRunning.store(false);
	{
		std::scoped_lock lock{ m_OutboundMutex };
		m_OutboundFrames.clear();
		m_iQueuedOutboundBytes = 0u;
		m_OutboundMetrics.iCurrentQueuedFrameCount = 0u;
		m_OutboundMetrics.iCurrentQueuedByteCount = 0u;
	}
	m_OutboundCondition.notify_all();

    {
        std::scoped_lock lock{m_IocpMutex};
        const SOCKET clientSocket = m_hClientSocket.load();
        if (INVALID_SOCKET != clientSocket)
        {
            if (SESSION_TRANSPORT_BACKEND::IOCP == m_TransportBackend)
                (void)::CancelIoEx(reinterpret_cast<HANDLE>(clientSocket), nullptr);
            ::shutdown(clientSocket, SD_BOTH);
        }
    }
    // IOCP close notification is dispatched by completion/maintenance workers,
    // never inline into a caller which may still own ServerApp admission locks.
}

void LostArk::Server::CClientSession::Request_Close_After_Flush(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const int nativeErrorCode,
	const std::string_view context)
{
	Record_TerminalDiagnostic(reason, nativeErrorCode, context);
	if (!m_isSendRunning.load())
	{
		Request_Close(reason, nativeErrorCode, context);
		return;
	}

	/* Keep the send half alive for the sender worker.  The receive worker must
	   not call the hard-close path when SD_RECEIVE wakes it, otherwise it would
	   clear the terminal reliable frame that this operation promises to drain. */
	// The first explicit close owns the deadline. Repeated close requests must
	// not extend a rejected session; active sessions never get this deadline.
	std::uint64_t unsetStart = 0u;
	(void)m_iTerminalDrainStartTicks.compare_exchange_strong(unsetStart,
		(std::max)(std::uint64_t{1u}, static_cast<std::uint64_t>(GetTickCount64())));
	m_closeAfterOutboundFlush.store(true);
	m_isReceiveRunning.store(false);
	if (!m_isSendRunning.load())
	{
		// A hard close can overtake the first running-state check.  In that
		// case neither worker still owns the graceful notification handoff.
		Request_Close(reason, nativeErrorCode, context);
		if (SESSION_TRANSPORT_BACKEND::SELECT_THREADS == m_TransportBackend)
			Notify_Closed();
		return;
	}
	{
		std::scoped_lock lock{m_IocpMutex};
		const SOCKET clientSocket = m_hClientSocket.load();
		if (INVALID_SOCKET != clientSocket)
			(void)::shutdown(clientSocket, SD_RECEIVE);
	}
	m_OutboundCondition.notify_all();
	if (SESSION_TRANSPORT_BACKEND::IOCP == m_TransportBackend) Kick_IocpSend();
}

void LostArk::Server::CClientSession::Stop()
{
	Request_Close();
	Close_Socket();
    if (SESSION_TRANSPORT_BACKEND::IOCP == m_TransportBackend)
    {
        {
            std::scoped_lock lock{m_IocpMutex};
            m_isIocpStopping = true;
            // An externally retained session can be destroyed after its service
            // has stopped. A fully drained Stop must not dereference that owner.
            if (m_iIocpOutstanding == 0)
            {
                m_IocpSendBytes.reset();
                return;
            }
        }
        // A close callback may request Stop on a completion worker. Its
        // operation still owns this session; waiting for itself would deadlock.
        if (m_pIocpService && m_pIocpService->Is_WorkerThread()) return;
        std::unique_lock lock{m_IocpMutex};
        if (!m_IocpDrained.wait_for(lock, std::chrono::milliseconds{5000},
            [this] { return m_iIocpOutstanding == 0; }))
        {
            ::TerminateProcess(::GetCurrentProcess(), ERROR_TIMEOUT);
            std::terminate();
        }
        m_IocpSendBytes.reset();
        return;
    }

	if (m_ReceiveThread.joinable())
		m_ReceiveThread.join();
	if (m_SendThread.joinable())
	{
		bool senderExited = false;
		{
			std::unique_lock lock{ m_OutboundMutex };
			senderExited = m_SenderExitCondition.wait_for(
				lock,
				std::chrono::milliseconds(
					SENDER_JOIN_TIMEOUT_MILLISECONDS),
				[this]() { return m_hasSenderExited; });
		}
		if (!senderExited)
		{
			m_iLastErrorCode.store(WSAETIMEDOUT);
			Record_TerminalDiagnostic(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT,
				WSAETIMEDOUT,
				"sender worker join timed out");
			(void)::CancelSynchronousIo(
				reinterpret_cast<HANDLE>(m_SendThread.native_handle()));
		}
		m_SendThread.join();
	}
}

bool LostArk::Server::CClientSession::Send_Frame(
	LostArk::Shared::PACKET_TYPE packetType,
	std::span<const std::uint8_t> payload)
{
	std::vector<std::uint8_t> frameBytes;

	if (!LostArk::Shared::Build_Packet_Frame(
		packetType,
		payload,
		frameBytes))
	{
		return false;
	}

	const OUTBOUND_ENQUEUE_RESULT result = Queue_OutboundFrame(
		packetType, std::move(frameBytes));
	if (OUTBOUND_ENQUEUE_RESULT::RELIABLE_OVERFLOW == result)
	{
		Request_Close(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_RELIABLE_OUTBOUND_OVERFLOW,
			WSAENOBUFS,
			"reliable outbound queue capacity exceeded");
		return false;
	}
	return OUTBOUND_ENQUEUE_RESULT::CLOSED != result;
}

void LostArk::Server::CClientSession::Bind_PlayerId(
	LostArk::Shared::PLAYER_ID playerId)
{
	m_iPlayerId.store(playerId);
}

LostArk::Server::SESSION_ID
LostArk::Server::CClientSession::Get_SessionId() const
{
	return m_iSessionId;
}

LostArk::Shared::PLAYER_ID
LostArk::Server::CClientSession::Get_PlayerId() const
{
	return m_iPlayerId.load();
}

bool LostArk::Server::CClientSession::Is_Open() const
{
	return INVALID_SOCKET != m_hClientSocket.load();
}

bool LostArk::Server::CClientSession::Is_Closing() const
{
	std::scoped_lock lock{ m_DiagnosticMutex };
	return LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE !=
		m_CloseDiagnostic.eReason;
}

int LostArk::Server::CClientSession::Get_LastErrorCode() const
{
	return m_iLastErrorCode.load();
}

LostArk::Server::CLIENT_SESSION_OUTBOUND_METRICS
LostArk::Server::CClientSession::Get_OutboundMetrics() const
{
	std::scoped_lock lock{ m_OutboundMutex };
	return m_OutboundMetrics;
}

const LostArk::Server::CLIENT_SESSION_PEER_ENDPOINT&
LostArk::Server::CClientSession::Get_PeerEndpoint() const noexcept
{
	return m_PeerEndpoint;
}

LostArk::Server::CLIENT_SESSION_CLOSE_DIAGNOSTIC
LostArk::Server::CClientSession::Get_CloseDiagnostic() const
{
	std::scoped_lock lock{ m_DiagnosticMutex };
	return m_CloseDiagnostic;
}

std::uint64_t
LostArk::Server::CClientSession::Get_LastInboundUnixMilliseconds() const noexcept
{
	return m_iLastInboundUnixMilliseconds.load();
}

void LostArk::Server::CClientSession::Receive_Loop()
{
	using namespace LostArk::Shared;

	while (m_isReceiveRunning.load())
	{
		PACKET_FRAME frame{};

		if (!Receive_Frame(frame))
			break;

		if (m_OnFrame)
			m_OnFrame(m_iSessionId, frame);
	}

	if (!m_closeAfterOutboundFlush.load())
	{
		Request_Close();
		Notify_Closed();
	}
}

void LostArk::Server::CClientSession::Sender_Loop()
{
	bool isolatedSendFailure = false;
	bool gracefulDrainComplete = false;
	for (;;)
	{
		OUTBOUND_FRAME frame{};
		{
			std::unique_lock lock{ m_OutboundMutex };
			m_OutboundCondition.wait(lock,
				[this]()
				{
					return !m_isSendRunning.load() ||
						!m_OutboundFrames.empty() ||
						m_closeAfterOutboundFlush.load();
				});
			if (!m_isSendRunning.load())
				break;
			if (m_OutboundFrames.empty())
			{
				gracefulDrainComplete =
					m_closeAfterOutboundFlush.load();
				break;
			}

			frame = std::move(m_OutboundFrames.front());
			m_OutboundFrames.pop_front();
			m_iQueuedOutboundBytes -= frame.Bytes.size();
			m_OutboundMetrics.iCurrentQueuedFrameCount =
				m_OutboundFrames.size();
			m_OutboundMetrics.iCurrentQueuedByteCount =
				m_iQueuedOutboundBytes;
		}

		const auto sendStart = std::chrono::steady_clock::now();
		const bool sent = Send_All(frame.Bytes, frame.ePacketType);
		const std::uint64_t sendMicroseconds = To_Microseconds(
			std::chrono::steady_clock::now() - sendStart);
		const bool sendFailedWhileRunning =
			!sent && m_isSendRunning.load();
		{
			std::scoped_lock lock{ m_OutboundMutex };
			m_OutboundMetrics.iLastFrameSendMicroseconds = sendMicroseconds;
			m_OutboundMetrics.iMaximumFrameSendMicroseconds = (std::max)(
				m_OutboundMetrics.iMaximumFrameSendMicroseconds,
				sendMicroseconds);
			if (sent)
			{
				++m_OutboundMetrics.iSentFrameCount;
				m_OutboundMetrics.iSentByteCount += frame.Bytes.size();
			}
			else if (sendFailedWhileRunning)
			{
				++m_OutboundMetrics.iSendFailureCount;
			}
		}
		if (!sent)
		{
			isolatedSendFailure = sendFailedWhileRunning;
			break;
		}
	}

	const bool flushCloseWasHardStopped =
		m_closeAfterOutboundFlush.load() && !gracefulDrainComplete;
	if (isolatedSendFailure || gracefulDrainComplete)
		Request_Close();
	{
		std::scoped_lock lock{ m_OutboundMutex };
		m_OutboundFrames.clear();
		m_iQueuedOutboundBytes = 0u;
		m_OutboundMetrics.iCurrentQueuedFrameCount = 0u;
		m_OutboundMetrics.iCurrentQueuedByteCount = 0u;
		m_hasSenderExited = true;
	}
	m_isSendRunning.store(false);
	m_SenderExitCondition.notify_all();
	if (isolatedSendFailure || gracefulDrainComplete ||
		flushCloseWasHardStopped)
		Notify_Closed();
}

LostArk::Server::CClientSession::OUTBOUND_ENQUEUE_RESULT
LostArk::Server::CClientSession::Queue_OutboundFrame(
	const LostArk::Shared::PACKET_TYPE packetType,
	std::vector<std::uint8_t> frameBytes)
{
	using LostArk::Shared::PACKET_TYPE;
	const bool isSnapshot = PACKET_TYPE::S2C_WORLD_SNAPSHOT == packetType;
	OUTBOUND_ENQUEUE_RESULT result = OUTBOUND_ENQUEUE_RESULT::QUEUED;
	{
		std::scoped_lock lock{ m_OutboundMutex };
		if (!m_isSendRunning.load() ||
			m_closeAfterOutboundFlush.load())
			return OUTBOUND_ENQUEUE_RESULT::CLOSED;

		if (isSnapshot)
		{
			const auto snapshotIter = std::find_if(
				m_OutboundFrames.begin(),
				m_OutboundFrames.end(),
				[](const OUTBOUND_FRAME& queued)
				{
					return PACKET_TYPE::S2C_WORLD_SNAPSHOT ==
						queued.ePacketType;
				});
			if (snapshotIter != m_OutboundFrames.end())
			{
				m_iQueuedOutboundBytes -= snapshotIter->Bytes.size();
				m_OutboundFrames.erase(snapshotIter);
				++m_OutboundMetrics.iSnapshotCoalescedFrameCount;
				result = OUTBOUND_ENQUEUE_RESULT::COALESCED;
			}

			constexpr std::size_t SNAPSHOT_FRAME_LIMIT =
				MAX_OUTBOUND_FRAME_COUNT - RELIABLE_FRAME_RESERVE;
			constexpr std::size_t SNAPSHOT_BYTE_LIMIT =
				MAX_OUTBOUND_BYTE_COUNT - RELIABLE_BYTE_RESERVE;
			if (m_OutboundFrames.size() >= SNAPSHOT_FRAME_LIMIT ||
				frameBytes.size() >
					SNAPSHOT_BYTE_LIMIT - (std::min)(
						m_iQueuedOutboundBytes, SNAPSHOT_BYTE_LIMIT))
			{
				++m_OutboundMetrics.iSnapshotDroppedFrameCount;
				m_OutboundMetrics.iCurrentQueuedFrameCount =
					m_OutboundFrames.size();
				m_OutboundMetrics.iCurrentQueuedByteCount =
					m_iQueuedOutboundBytes;
				return OUTBOUND_ENQUEUE_RESULT::DROPPED_SNAPSHOT;
			}
		}
		else if (m_OutboundFrames.size() >= MAX_OUTBOUND_FRAME_COUNT ||
			frameBytes.size() >
				MAX_OUTBOUND_BYTE_COUNT - (std::min)(
					m_iQueuedOutboundBytes, MAX_OUTBOUND_BYTE_COUNT))
		{
			++m_OutboundMetrics.iReliableRejectedFrameCount;
			return OUTBOUND_ENQUEUE_RESULT::RELIABLE_OVERFLOW;
		}

		m_iQueuedOutboundBytes += frameBytes.size();
		m_OutboundFrames.push_back({ packetType, std::move(frameBytes) });
		if (isSnapshot)
			++m_OutboundMetrics.iSnapshotEnqueuedFrameCount;
		else
			++m_OutboundMetrics.iReliableEnqueuedFrameCount;
		m_OutboundMetrics.iCurrentQueuedFrameCount =
			m_OutboundFrames.size();
		m_OutboundMetrics.iCurrentQueuedByteCount =
			m_iQueuedOutboundBytes;
		m_OutboundMetrics.iQueuedFrameHighWatermark = (std::max)(
			m_OutboundMetrics.iQueuedFrameHighWatermark,
			m_OutboundFrames.size());
		m_OutboundMetrics.iQueuedByteHighWatermark = (std::max)(
			m_OutboundMetrics.iQueuedByteHighWatermark,
			m_iQueuedOutboundBytes);
	}
	Wake_Sender();
	return result;
}

bool LostArk::Server::CClientSession::Configure_TransportOptions()
{
	const SOCKET clientSocket = m_hClientSocket.load();
	if (INVALID_SOCKET == clientSocket)
		return false;
	/* Small input and snapshot frames must leave immediately instead of
	waiting for Nagle's batching and the peer's delayed acknowledgement. */
	const BOOL noDelay = TRUE;
	if (SOCKET_ERROR == ::setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY,
		reinterpret_cast<const char*>(&noDelay), static_cast<int>(sizeof(noDelay))))
	{
		const int errorCode = ::WSAGetLastError();
		m_iLastErrorCode.store(errorCode);
		Record_TerminalDiagnostic(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_START_FAILED,
			errorCode, "failed to disable TCP movement batching");
		return false;
	}
	// A blocking SO_SNDTIMEO expiry leaves the stream indeterminate. Use
	// nonblocking send/recv and readiness waits so only WSAEWOULDBLOCK retries.
	u_long nonblocking = SESSION_TRANSPORT_BACKEND::SELECT_THREADS == m_TransportBackend ? 1u : 0u;
	if (SOCKET_ERROR == ::ioctlsocket(clientSocket, FIONBIO, &nonblocking))
	{
		const int errorCode = ::WSAGetLastError();
		m_iLastErrorCode.store(errorCode);
		Record_TerminalDiagnostic(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_START_FAILED,
			errorCode,
			"failed to configure nonblocking session transport");
		return false;
	}
	return true;
}

void LostArk::Server::CClientSession::Close_Socket()
{
	std::scoped_lock lock{m_IocpMutex};
	const SOCKET clientSocket = m_hClientSocket.exchange(INVALID_SOCKET);
	if (INVALID_SOCKET == clientSocket)
		return;
	::shutdown(clientSocket, SD_BOTH);
	::closesocket(clientSocket);
}

bool LostArk::Server::CClientSession::Receive_Frame(
	LostArk::Shared::PACKET_FRAME& frame)
{
	using namespace LostArk::Shared;

	for (;;)
	{
		if (!m_isReceiveRunning.load()) return false;
		const PACKET_PARSE_RESULT parseResult =
			m_StreamParser.Try_Pop(frame);

		if (PACKET_PARSE_RESULT::FRAME_READY == parseResult)
		{
			Record_InboundPacket(frame.ePacketType);
			return true;
		}

		if (PACKET_PARSE_RESULT::INVALID_FRAME == parseResult)
		{
			m_iLastErrorCode.store(WSAEPROTONOSUPPORT);
			Record_TerminalDiagnostic(
				SESSION_DIAGNOSTIC_REASON::SERVER_INVALID_FRAME,
				WSAEPROTONOSUPPORT,
				"packet header or frame contract invalid");
			return false;
		}

		std::array<std::uint8_t, 4096> receiveBuffer{};

		const SOCKET clientSocket = m_hClientSocket.load();
		if (INVALID_SOCKET == clientSocket)
			return false;

		const int receivedByteCount = ::recv(
			clientSocket,
			reinterpret_cast<char*>(receiveBuffer.data()),
			static_cast<int>(receiveBuffer.size()),
			0);

		if (0 == receivedByteCount)
		{
			Record_TerminalDiagnostic(
				SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED,
				0,
				"peer completed orderly TCP shutdown");
			return false;
		}

		if (SOCKET_ERROR == receivedByteCount)
		{
			int errorCode = ::WSAGetLastError();
			if (WSAEWOULDBLOCK == errorCode)
			{
				if (!m_isReceiveRunning.load()) return false;
				if (SOCKET_ERROR != Wait_SocketReady(clientSocket, false,
					TRANSPORT_POLL_MILLISECONDS)) continue;
				errorCode = ::WSAGetLastError();
			}

			if (m_isReceiveRunning.load())
			{
				m_iLastErrorCode.store(errorCode);
				Record_TerminalDiagnostic(
					SESSION_DIAGNOSTIC_REASON::SERVER_RECEIVE_ERROR,
					errorCode,
					"recv failed while session was active");
			}

			return false;
		}

		const std::span<const std::uint8_t> receivedBytes
		{
			receiveBuffer.data(),
			static_cast<std::size_t>(receivedByteCount)
		};

		if (!m_StreamParser.Append(receivedBytes))
		{
			m_iLastErrorCode.store(WSAEMSGSIZE);
			Record_TerminalDiagnostic(
				SESSION_DIAGNOSTIC_REASON::SERVER_PARSER_OVERFLOW,
				WSAEMSGSIZE,
				"packet stream parser buffer capacity exceeded");
			return false;
		}
	}
}

bool LostArk::Server::CClientSession::Send_All(
	std::span<const std::uint8_t> bytes,
	const LostArk::Shared::PACKET_TYPE packetType)
{
	std::size_t sentByteCount = 0;
	auto lastProgress = std::chrono::steady_clock::now();
	bool stalled = false;
	bool reportStall = false;
	const auto fail = [&](const int errorCode, const char* reason)
	{
		if (m_isSendRunning.load())
		{
			m_iLastErrorCode.store(errorCode);
			const auto noProgressMs = To_Microseconds(
				std::chrono::steady_clock::now() - lastProgress) / 1000u;
			char context[256]{};
			std::snprintf(context, sizeof(context),
				"%s; sentFrameBytes=%zu; totalFrameBytes=%zu; noProgressMs=%llu",
				reason, sentByteCount, bytes.size(),
				static_cast<unsigned long long>(noProgressMs));
			Record_SendProgressDiagnostic("send.terminal", packetType,
				sentByteCount, bytes.size(), noProgressMs);
			Record_TerminalDiagnostic(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT,
				errorCode, context);
		}
		return false;
	};
	while (sentByteCount < bytes.size())
	{
		if (!m_isSendRunning.load()) return false;
		if (m_closeAfterOutboundFlush.load() &&
			GetTickCount64() - m_iTerminalDrainStartTicks.load() >= TERMINAL_DRAIN_TIMEOUT_MILLISECONDS)
		{
			Record_SendProgressDiagnostic("send.terminal-drain-expired", packetType,
				sentByteCount, bytes.size(),
				To_Microseconds(std::chrono::steady_clock::now() - lastProgress) / 1000u);
			// This cancels an already terminal drain, not active backpressure.
			// Request_Close preserves the original ROOM_FULL reason and context.
			Request_Close();
			return false;
		}
		const SOCKET clientSocket = m_hClientSocket.load();
		if (INVALID_SOCKET == clientSocket) return false;
		const int result = ::send(clientSocket,
			reinterpret_cast<const char*>(bytes.data() + sentByteCount),
			static_cast<int>(bytes.size() - sentByteCount), 0);
		if (SOCKET_ERROR == result)
		{
			const int errorCode = ::WSAGetLastError();
			if (WSAEWOULDBLOCK != errorCode)
				return fail(errorCode, "nonblocking send failed while session was active");
			const auto noProgressMs = To_Microseconds(
				std::chrono::steady_clock::now() - lastProgress) / 1000u;
			if (!stalled && noProgressMs >= SEND_STALL_REPORT_MILLISECONDS)
			{
				stalled = true;
				++m_iSendStallOrdinal;
				reportStall = (m_iSendStallOrdinal & (m_iSendStallOrdinal - 1u)) == 0u;
				if (reportStall) Record_SendProgressDiagnostic("send.stalled", packetType,
					sentByteCount, bytes.size(), noProgressMs);
			}
			if (SOCKET_ERROR == Wait_SocketReady(clientSocket, true, TRANSPORT_POLL_MILLISECONDS))
				return fail(::WSAGetLastError(), "send readiness wait failed");
			continue;
		}
		if (0 == result) return fail(WSAECONNRESET, "send returned zero bytes");
		sentByteCount += static_cast<std::size_t>(result);
		if (stalled && reportStall) Record_SendProgressDiagnostic("send.recovered", packetType,
			sentByteCount, bytes.size(), To_Microseconds(std::chrono::steady_clock::now() - lastProgress) / 1000u);
		lastProgress = std::chrono::steady_clock::now();
		stalled = false;
		reportStall = false;
	}
	return true;
}

void LostArk::Server::CClientSession::Record_SendProgressDiagnostic(
	const char* eventName, const LostArk::Shared::PACKET_TYPE packetType,
	const std::size_t sentBytes, const std::size_t totalBytes,
	const std::uint64_t noProgressMilliseconds) noexcept
{
	try
	{
		static std::mutex fileMutex;
		std::scoped_lock lock{fileMutex};
		wchar_t modulePath[32768]{};
		const auto length = GetModuleFileNameW(nullptr, modulePath, 32768u);
		if (!length || length >= 32768u) return;
		const auto directory = std::filesystem::path(modulePath).parent_path() / L"Diagnostics";
		std::error_code error;
		std::filesystem::create_directories(directory, error);
		if (error) return;
		const auto path = directory / (L"server-send-progress-" +
			std::to_wstring(GetCurrentProcessId()) + L".jsonl");
		const auto size = std::filesystem::file_size(path, error);
		if (!error && size >= 2u * 1024u * 1024u)
		{
			const auto previous = path.wstring() + L".previous";
			std::filesystem::remove(previous, error);
			if (error) return;
			std::filesystem::rename(path, previous, error);
			if (error) return;
		}
		std::ofstream log{path, std::ios::binary | std::ios::app};
		if (!log) return;
		const auto now = Current_UnixMilliseconds();
		const auto inbound = m_iLastInboundUnixMilliseconds.load();
		const bool terminalDrain = m_closeAfterOutboundFlush.load();
		const auto drainStart = m_iTerminalDrainStartTicks.load();
		const auto terminal = Get_CloseDiagnostic();
		log << "{\"schema\":\"lostark.server-send-progress\",\"formatVersion\":1"
			<< ",\"unixMs\":" << now << ",\"processId\":" << GetCurrentProcessId()
			<< ",\"sessionId\":" << m_iSessionId << ",\"peerAddress\":\"" << m_PeerEndpoint.strAddress
			<< "\",\"peerPort\":" << m_PeerEndpoint.iPort << ",\"event\":\"" << eventName
			<< "\",\"packetType\":" << static_cast<unsigned>(packetType)
			<< ",\"stallOrdinal\":" << m_iSendStallOrdinal.load() << ",\"sentFrameBytes\":" << sentBytes
			<< ",\"totalFrameBytes\":" << totalBytes << ",\"noProgressMs\":" << noProgressMilliseconds
			<< ",\"closeOnBackpressure\":false"
			<< ",\"terminalDrainRequested\":" << (terminalDrain ? "true" : "false")
			<< ",\"terminalDrainElapsedMs\":" << (terminalDrain ? GetTickCount64() - drainStart : 0u)
			<< ",\"terminalDrainLimitMs\":" << (terminalDrain ? TERMINAL_DRAIN_TIMEOUT_MILLISECONDS : 0u)
			<< ",\"terminalReason\":\"" << LostArk::Shared::To_SessionDiagnosticReasonName(terminal.eReason) << '"'
			<< ",\"lastInboundAgeMs\":" << (inbound && now >= inbound ? now - inbound : 0u) << "}\n";
	}
	catch (...) { } // Diagnostic failures cannot change stream progress or closure.
}

void LostArk::Server::CClientSession::Record_InboundPacket(
	const LostArk::Shared::PACKET_TYPE packetType) noexcept
{
	m_eLastInboundPacket.store(packetType);
	m_iLastInboundUnixMilliseconds.store(Current_UnixMilliseconds());
}

void LostArk::Server::CClientSession::Record_TerminalDiagnostic(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const int nativeErrorCode,
	const std::string_view context)
{
	using LostArk::Shared::SESSION_DIAGNOSTIC_REASON;
	if (SESSION_DIAGNOSTIC_REASON::NONE == reason)
		return;

	std::scoped_lock lock{ m_DiagnosticMutex };
	if (SESSION_DIAGNOSTIC_REASON::NONE != m_CloseDiagnostic.eReason)
		return;
	m_CloseDiagnostic.eReason = reason;
	m_CloseDiagnostic.eLastInboundPacket = m_eLastInboundPacket.load();
	m_CloseDiagnostic.iNativeErrorCode = nativeErrorCode;
	m_CloseDiagnostic.iOccurredUnixMilliseconds = Current_UnixMilliseconds();
	m_CloseDiagnostic.iLastInboundUnixMilliseconds =
		m_iLastInboundUnixMilliseconds.load();
	{
		std::scoped_lock outboundLock{ m_OutboundMutex };
		m_CloseDiagnostic.iQueuedFrameCountAtClose =
			m_OutboundFrames.size();
		m_CloseDiagnostic.iQueuedByteCountAtClose =
			m_iQueuedOutboundBytes;
	}
	m_CloseDiagnostic.strContext.assign(context.begin(), context.end());
}

LostArk::Server::CLIENT_SESSION_PEER_ENDPOINT
LostArk::Server::CClientSession::Resolve_PeerEndpoint(
	const SOCKET clientSocket) noexcept
{
	CLIENT_SESSION_PEER_ENDPOINT endpoint{};
	if (INVALID_SOCKET == clientSocket)
		return endpoint;

	sockaddr_storage peer{};
	int peerLength = static_cast<int>(sizeof(peer));
	if (SOCKET_ERROR == ::getpeername(
		clientSocket,
		reinterpret_cast<sockaddr*>(&peer),
		&peerLength))
	{
		return endpoint;
	}

	std::array<char, INET6_ADDRSTRLEN> address{};
	if (AF_INET == peer.ss_family)
	{
		const auto* ipv4 = reinterpret_cast<const sockaddr_in*>(&peer);
		if (nullptr != ::InetNtopA(
			AF_INET, const_cast<IN_ADDR*>(&ipv4->sin_addr),
			address.data(), static_cast<DWORD>(address.size())))
		{
			endpoint.strAddress = address.data();
		}
		endpoint.iPort = ::ntohs(ipv4->sin_port);
	}
	else if (AF_INET6 == peer.ss_family)
	{
		const auto* ipv6 = reinterpret_cast<const sockaddr_in6*>(&peer);
		if (nullptr != ::InetNtopA(
			AF_INET6, const_cast<IN6_ADDR*>(&ipv6->sin6_addr),
			address.data(), static_cast<DWORD>(address.size())))
		{
			endpoint.strAddress = address.data();
		}
		endpoint.iPort = ::ntohs(ipv6->sin6_port);
	}
	return endpoint;
}

void LostArk::Server::CClientSession::Notify_Closed()
{
	if (m_hasNotifiedClosed.exchange(true))
		return;

	if (m_OnClosed)
		m_OnClosed(m_iSessionId);
}
~~~~

<a id="file-server-private-clientsession-iocp-cpp"></a>

### Server/Private/ClientSession_Iocp.cpp

파일: C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession_Iocp.cpp

한 receive callback이 parsing과 OnFrame을 끝낸 뒤 다음 receive를 게시한다. send는 한 frame의 남은 offset을 모두 완료한 뒤 다음 frame으로 간다. 종료는 취소된 OVERLAPPED의 완료까지 버퍼를 보존한다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
#include "ClientSession.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <utility>

namespace
{
    std::uint64_t Elapsed_Microseconds(const std::chrono::steady_clock::time_point start)
    {
        return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start).count());
    }
}

bool LostArk::Server::CClientSession::Start_Iocp()
{
    const auto self = weak_from_this().lock();
    if (!self || !m_pIocpService || !m_pIocpService->Associate(m_hClientSocket.load()))
    {
        Fail_Iocp(LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_START_FAILED,
            WSAEINVAL, "IOCP requires a running service and shared session ownership");
        return false;
    }
    m_isIocpStarted.store(true);
    m_pIocpService->Register(self);
    return Post_IocpReceive();
}

void LostArk::Server::CClientSession::Fail_Iocp(
    const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
    const int error, const char* context)
{
    m_iLastErrorCode.store(error);
    Request_Close(reason, error, context);
}

bool LostArk::Server::CClientSession::Post_IocpReceive()
{
    int error = 0;
    {
        std::scoped_lock lock{m_IocpMutex};
        if (!m_isReceiveRunning.load() || m_isIocpReceivePending ||
            m_hClientSocket.load() == INVALID_SOCKET) return false;
        m_isIocpReceivePending = true;
        ++m_iIocpOutstanding;
        bool posted = false;
        try
        {
            posted = m_pIocpService->Post_Receive(m_hClientSocket.load(), shared_from_this(), error);
        }
        catch (...) { error = WSAENOBUFS; }
        if (posted) return true;
        m_isIocpReceivePending = false;
        --m_iIocpOutstanding;
        m_IocpDrained.notify_all();
    }
    Fail_Iocp(LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_RECEIVE_ERROR,
        error, "overlapped receive initiation failed");
    return false;
}

// Caller holds m_IocpMutex, so cancellation cannot pass the running-state check
// and race a newly initiated operation onto a closed/reused socket handle.
bool LostArk::Server::CClientSession::Post_IocpSendLocked(int& error)
{
    if (!m_isSendRunning.load() || m_hClientSocket.load() == INVALID_SOCKET)
    { error = WSAESHUTDOWN; return false; }
    ++m_iIocpOutstanding;
    bool posted = false;
    try
    {
        posted = m_pIocpService->Post_Send(m_hClientSocket.load(), shared_from_this(),
            m_IocpSendBytes, m_iIocpSendOffset, error);
    }
    catch (...) { error = WSAENOBUFS; }
    if (!posted)
    {
        --m_iIocpOutstanding;
        m_IocpDrained.notify_all();
    }
    return posted;
}

void LostArk::Server::CClientSession::Wake_Sender() noexcept
{
    if (SESSION_TRANSPORT_BACKEND::IOCP == m_TransportBackend) Kick_IocpSend();
    else m_OutboundCondition.notify_one();
}

void LostArk::Server::CClientSession::Kick_IocpSend() noexcept
{
    try
    {
        int error = 0;
        bool drained = false;
        {
            // The only nested transport order is outbound -> IOCP. Completion
            // handling releases IOCP before touching outbound or diagnostics.
            std::scoped_lock outboundLock{m_OutboundMutex};
            std::scoped_lock ioLock{m_IocpMutex};
            if (!m_isSendRunning.load() || m_isIocpSendPending) return;
            if (m_OutboundFrames.empty())
                drained = m_closeAfterOutboundFlush.load();
            else
            {
                auto& frame = m_OutboundFrames.front();
                m_IocpSendBytes = std::make_shared<const std::vector<std::uint8_t>>(std::move(frame.Bytes));
                m_IocpSendPacket = frame.ePacketType;
                m_iQueuedOutboundBytes -= m_IocpSendBytes->size();
                m_OutboundFrames.pop_front();
                m_OutboundMetrics.iCurrentQueuedFrameCount = m_OutboundFrames.size();
                m_OutboundMetrics.iCurrentQueuedByteCount = m_iQueuedOutboundBytes;
                m_iIocpSendOffset = 0;
                m_IocpSendStart = m_IocpLastProgress = std::chrono::steady_clock::now();
                m_isIocpStalled = m_reportIocpStall = false;
                m_isIocpSendPending = true;
                if (!Post_IocpSendLocked(error))
                {
                    m_isIocpSendPending = false;
                    m_IocpSendBytes.reset();
                    ++m_OutboundMetrics.iSendFailureCount;
                }
            }
        }
        if (error)
            Fail_Iocp(LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT,
                error, "overlapped send initiation failed");
        else if (drained) Request_Close();
    }
    catch (...)
    {
        Fail_Iocp(LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT,
            WSAENOBUFS, "IOCP sender could not stage the outbound frame");
    }
}

void LostArk::Server::CClientSession::Complete_IocpOperation() noexcept
{
    std::scoped_lock lock{m_IocpMutex};
    if (--m_iIocpOutstanding == 0) m_IocpDrained.notify_all();
}

void LostArk::Server::CClientSession::On_IocpCompleted(
    const IOCP_OPERATION_KIND kind, const std::span<const std::uint8_t> received,
    const std::uint32_t bytes, const std::uint32_t error) noexcept
{
    try
    {
        if (kind == IOCP_OPERATION_KIND::RECEIVE) Handle_IocpReceive(received, bytes, error);
        else Handle_IocpSend(bytes, error);
    }
    catch (...)
    {
        // No exception may strand an OVERLAPPED owner or escape a worker.
        try
        {
            Fail_Iocp(LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
                WSAENOBUFS, "IOCP completion callback failed");
        }
        catch (...) { std::terminate(); }
    }
    if (!m_isSendRunning.load())
    {
        try { Notify_Closed(); }
        catch (...) { std::terminate(); }
    }
    Complete_IocpOperation();
}

void LostArk::Server::CClientSession::Handle_IocpReceive(
    const std::span<const std::uint8_t> received,
    const std::uint32_t bytes, const std::uint32_t error)
{
    using namespace LostArk::Shared;
    {
        std::scoped_lock lock{m_IocpMutex};
        m_isIocpReceivePending = false;
    }
    // Graceful terminal drain cancels the receive half while send stays alive.
    if (!m_isReceiveRunning.load()) return;
    if (error || !bytes)
    {
        Fail_Iocp(error ? SESSION_DIAGNOSTIC_REASON::SERVER_RECEIVE_ERROR :
            SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED,
            static_cast<int>(error), error ? "overlapped receive failed" : "peer completed orderly TCP shutdown");
        return;
    }
    if (received.size() != bytes || !m_StreamParser.Append(received))
    {
        Fail_Iocp(SESSION_DIAGNOSTIC_REASON::SERVER_PARSER_OVERFLOW,
            WSAEMSGSIZE, "packet stream parser buffer capacity exceeded");
        return;
    }
    while (m_isReceiveRunning.load())
    {
        PACKET_FRAME frame{};
        const auto result = m_StreamParser.Try_Pop(frame);
        if (result == PACKET_PARSE_RESULT::NEED_MORE_DATA) break;
        if (result == PACKET_PARSE_RESULT::INVALID_FRAME)
        {
            Fail_Iocp(SESSION_DIAGNOSTIC_REASON::SERVER_INVALID_FRAME,
                WSAEPROTONOSUPPORT, "packet header or frame contract invalid");
            return;
        }
        Record_InboundPacket(frame.ePacketType);
        if (m_OnFrame) m_OnFrame(m_iSessionId, frame);
    }
    // Exactly one receive is posted, only after parsing and dispatch finish.
    // Worker identity may change, but the parser never has concurrent writers.
    if (m_isReceiveRunning.load()) (void)Post_IocpReceive();
}

void LostArk::Server::CClientSession::Handle_IocpSend(
    const std::uint32_t bytes, const std::uint32_t completionError)
{
    std::uint32_t error = completionError;
    std::size_t sent = 0, total = 0;
    std::uint64_t elapsed = 0, stalledMilliseconds = 0;
    auto packet = LostArk::Shared::PACKET_TYPE::INVALID;
    bool finished = false, recovered = false, failed = false;
    {
        std::scoped_lock lock{m_IocpMutex};
        if (!m_IocpSendBytes) return;
        total = m_IocpSendBytes->size();
        packet = m_IocpSendPacket;
        const bool active = m_isSendRunning.load();
        if (!error && (!bytes || bytes > total - m_iIocpSendOffset)) error = WSAECONNRESET;
        if (!error)
        {
            m_iIocpSendOffset += bytes;
            recovered = m_isIocpStalled && m_reportIocpStall;
            stalledMilliseconds = Elapsed_Microseconds(m_IocpLastProgress) / 1000;
            m_IocpLastProgress = std::chrono::steady_clock::now();
            m_isIocpStalled = m_reportIocpStall = false;
        }
        sent = m_iIocpSendOffset;
        elapsed = Elapsed_Microseconds(m_IocpSendStart);
        finished = !error && sent == total;
        if (active && !error && !finished)
        {
            int postError = 0;
            if (!Post_IocpSendLocked(postError)) error = static_cast<std::uint32_t>(postError);
        }
        failed = active && error;
        if (!active || error || finished)
        {
            m_isIocpSendPending = false;
            m_IocpSendBytes.reset();
        }
    }
    if (recovered) Record_SendProgressDiagnostic("send.recovered", packet, sent, total, stalledMilliseconds);
    if (finished || failed)
    {
        std::scoped_lock lock{m_OutboundMutex};
        m_OutboundMetrics.iLastFrameSendMicroseconds = elapsed;
        m_OutboundMetrics.iMaximumFrameSendMicroseconds =
            (std::max)(m_OutboundMetrics.iMaximumFrameSendMicroseconds, elapsed);
        if (finished)
        {
            ++m_OutboundMetrics.iSentFrameCount;
            m_OutboundMetrics.iSentByteCount += total;
        }
        else ++m_OutboundMetrics.iSendFailureCount;
    }
    if (failed)
    {
        char context[256]{};
        std::snprintf(context, sizeof(context),
            "overlapped send failed; sentFrameBytes=%zu; totalFrameBytes=%zu", sent, total);
        Record_SendProgressDiagnostic("send.terminal", packet, sent, total, stalledMilliseconds);
        Fail_Iocp(LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT,
            static_cast<int>(error), context);
    }
    else if (finished) Kick_IocpSend();
}

void LostArk::Server::CClientSession::On_IocpMaintenance() noexcept
{
    {
        std::scoped_lock lock{m_IocpMutex};
        if (m_isIocpStopping) return;
        ++m_iIocpOutstanding; // Stop also drains an already active maintenance callback.
    }
    try
    {
        if (!m_isSendRunning.load())
        {
            if (m_isIocpStarted.load()) Notify_Closed();
            Complete_IocpOperation();
            return;
        }
        const bool expired = m_closeAfterOutboundFlush.load() &&
            ::GetTickCount64() - m_iTerminalDrainStartTicks.load() >= TERMINAL_DRAIN_TIMEOUT_MILLISECONDS;
        bool report = false;
        std::size_t sent = 0, total = 0;
        std::uint64_t stalledMilliseconds = 0;
        auto packet = LostArk::Shared::PACKET_TYPE::INVALID;
        {
            std::scoped_lock lock{m_IocpMutex};
            if (m_isIocpSendPending && m_IocpSendBytes)
            {
                sent = m_iIocpSendOffset;
                total = m_IocpSendBytes->size();
                packet = m_IocpSendPacket;
                stalledMilliseconds = Elapsed_Microseconds(m_IocpLastProgress) / 1000;
                if (!m_isIocpStalled && stalledMilliseconds >= SEND_STALL_REPORT_MILLISECONDS)
                {
                    m_isIocpStalled = true;
                    const auto ordinal = m_iSendStallOrdinal.fetch_add(1) + 1;
                    m_reportIocpStall = report = (ordinal & (ordinal - 1)) == 0;
                }
            }
        }
        if (expired)
        {
            Record_SendProgressDiagnostic("send.terminal-drain-expired", packet, sent, total, stalledMilliseconds);
            Request_Close();
        }
        else if (report)
            Record_SendProgressDiagnostic("send.stalled", packet, sent, total, stalledMilliseconds);
    }
    catch (...) { std::terminate(); }
    Complete_IocpOperation();
}
~~~~

<a id="file-server-private-gameroom-cpp"></a>

### Server/Private/GameRoom.cpp

파일: C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp

room 생성 시 전달받은 pool 수명을 보관한다. 기존 데이터 로드·취소·gameplay Tick 흐름을 유지한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~cpp
#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

LostArk::Server::CGameRoom::CGameRoom(
	const LostArk::Shared::WORLD_ID worldId,
	std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration,
	const std::atomic_bool* pPreparationCancelled,
	std::shared_ptr<LostArk::Shared::Concurrency::WorkStealingJobSystem> snapshotJobs)
	: m_eWorldId(worldId)
	, m_SnapshotJobs(std::move(snapshotJobs))
{
	const auto cancelled = [this, pPreparationCancelled]()
	{
		if (!pPreparationCancelled || !pPreparationCancelled->load(std::memory_order_relaxed)) return false;
		m_strStatus = "Room preparation cancelled";
		return true;
	};
	if (cancelled()) return;
	if (!LostArk::Shared::Is_Known_World_Id(worldId))
	{
		m_strStatus = "Unknown room world ID";
		return;
	}
	if (!m_WorldBootstrap.Load(worldId))
	{
		m_strStatus = m_WorldBootstrap.Get_Status();
		return;
	}
	if (cancelled()) return;
	if ((nullptr != initialGameplayGeneration &&
			!m_GameplayCatalog.Initialize(initialGameplayGeneration)) ||
		(nullptr == initialGameplayGeneration && !m_GameplayCatalog.Load()))
	{
		m_strStatus = m_GameplayCatalog.Get_Status();
		return;
	}
	if (cancelled()) return;
	if (!m_ItemCatalog.Load())
	{
		m_strStatus = m_ItemCatalog.Get_Status();
		return;
	}
	if (cancelled()) return;
	if (!m_VehicleCatalog.Load())
	{
		m_strStatus = m_VehicleCatalog.Get_Status();
		return;
	}
	if (cancelled()) return;
	if (!m_HonorTitleCatalog.Load())
	{
		m_strStatus = m_HonorTitleCatalog.Get_Status();
		return;
	}
	if (cancelled()) return;
	if (!m_ValtanClearRewards.Load())
	{
		m_strStatus = m_ValtanClearRewards.Get_Status();
		return;
	}
	if (cancelled()) return;
	if (!m_SpawnGroupBootstrap.Load(worldId))
	{
		m_strStatus = m_SpawnGroupBootstrap.Get_Status();
		return;
	}
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, m_strStatus))
		return;
	if (cancelled()) return;
	m_EstherSkillSystem.Initialize(worldId);
	/* Bern joins the areas that require navigation. Without a grid the room keeps
	the spawn height for the whole session and straight-line XZ movement walks
	through the castle stairs, so a missing or malformed grid fails admission here
	instead of degrading silently. */
	if ((LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::TRAINING_GROUND == worldId ||
		LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == worldId ||
		LostArk::Shared::WORLD_ID::MAHARAKA == worldId ||
		LostArk::Shared::WORLD_ID::COLOSSEUM == worldId ||
		LostArk::Shared::WORLD_ID::BERN == worldId) &&
		!m_ServerNavigation.Load(m_WorldBootstrap.Get_AreaId()))
	{
		m_strStatus = m_ServerNavigation.Get_Status();
		return;
	}
	if (cancelled()) return;
	if (!m_NpcBehaviorRuntime.Validate_Admission(
		m_WorldBootstrap.Get_Placements(), m_ServerNavigation, m_strStatus))
	{
		return;
	}
	m_ServerTriggerSystem.Set_WorldId(worldId);
#ifdef _DEBUG
	/* Debug rooms hand the Kouku Book1/Book2 and Valtan Stage_1/Stage_2 wave boxes to
	the F1 "Normal Monster 1/2" buttons. Release never sets it. */
	m_ServerTriggerSystem.Set_SuppressWaveMonsterTriggers(true);
#endif
	m_ServerTriggerSystem.Set_FireLog([](const std::string& line)
	{
		std::cout << line << '\n';
	});
	m_ServerTriggerSystem.Set_GroundSampler(
		[this](const float x, const float z, float& outY)
		{
			SERVER_NAV_POINT point{};
			if (!m_ServerNavigation.Is_Loaded() ||
				!m_ServerNavigation.Sample_Position(x, z, point))
			{
				return false;
			}
			outY = point.y;
			return true;
		});
	if (!m_ServerTriggerSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), m_strStatus,
		LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId))
	{
		return;
	}
	if (cancelled()) return;
	if (!m_ServerCollisionSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), m_strStatus))
	{
		return;
	}
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (placement.isEnabled &&
			WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
			!m_ServerCollisionSystem.Is_PlayerSpawnClear(placement))
		{
			m_strStatus = "Player spawn overlaps a collision box: " +
				placement.strPlacementId;
			return;
		}
	}
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId)
	{
		if (!m_WorldDestructionBootstrap.Load_ValtanArena())
		{
			m_strStatus = m_WorldDestructionBootstrap.Get_Status();
			return;
		}
		if (!m_WorldDestructionRuntime.Initialize(
			m_WorldDestructionBootstrap.Get_DescriptorGraph(),
			m_strStatus, 1u))
		{
			return;
		}
		if (!Initialize_WorldPickups())
			return;
		/* The stele slots are repeatable presentation state whose Deploy
		occurrences stay hidden on the Client until the state becomes INTACT.
		The authored set now carries the cover circle each raised slot owns, and
		its position is published from the same Deploy placement the Client
		renders, so the two can never describe different ground. */
		std::vector<ENCOUNTER_PROP_SET_DESCRIPTOR> propSets;
		if (!Load_EncounterPropSets(m_eWorldId, propSets, m_strStatus))
			return;
		if (!propSets.empty())
		{
			if (propSets.size() != 1u ||
				!m_EncounterPropRuntime.Initialize(
					propSets.front(), m_strStatus, 1u))
			{
				m_strStatus =
					"World declares an encounter prop set this room cannot own";
				return;
			}
		}
		std::set<std::string> voidConditionIds;
		for (const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation :
			m_WorldDestructionBootstrap.Get_DescriptorGraph().Mutations)
		{
			if ((!mutation.strCollisionStateId.empty() &&
				 !m_ServerCollisionSystem.Has_CollisionStateTarget(
					 mutation.strCollisionStateId)) ||
				(!mutation.strNavigationStateId.empty() &&
				 !m_ServerNavigation.Has_Condition(
					 mutation.strNavigationStateId)))
			{
				m_strStatus = "World destruction dynamic state reference is unknown: " +
					mutation.strMutationId;
				return;
			}
			/* The floor sectors are the only mutations that take ground away.
			Navigation needs their conditions before any of them can flip, so the
			set is handed over here and never rebuilt during combat. */
			if (mutation.bRemovesGround)
				voidConditionIds.insert(mutation.strNavigationStateId);
		}
		if (!m_ServerNavigation.Set_VoidConditions(
			voidConditionIds, m_strStatus))
		{
			return;
		}
	}
	if (cancelled()) return;
	if (!Initialize_WorldEntities())
		return;
	if (nullptr == Find_AvailablePlayerSpawn())
	{
		m_strStatus = "World bootstrap has no enabled player spawn";
		return;
	}

	if (cancelled()) return;
	m_isReady = true;
	Initialize_Guide();
	m_strStatus = m_WorldBootstrap.Get_Status();
}

bool LostArk::Server::CGameRoom::Enqueue(ROOM_COMMAND command)
{
	return Is_AcceptedRoomCommandEnqueueResult(
		Enqueue_Detailed(std::move(command)));
}

LostArk::Server::ROOM_COMMAND_ENQUEUE_RESULT
LostArk::Server::CGameRoom::Enqueue_Detailed(ROOM_COMMAND command)
{
	if (command.iSessionId == INVALID_SESSION_ID)
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND;
	if (command.eType == ROOM_COMMAND_TYPE::REGISTER_SESSION &&
		(nullptr == command.pSession ||
			command.pSession->Get_SessionId() != command.iSessionId))
	{
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND;
	}

	std::scoped_lock lock{ m_CommandMutex };
	if (!m_isReady)
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY;
	if (!m_acceptsCommands)
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_SEALED;

	if (ROOM_COMMAND_TYPE::LEAVE == command.eType)
	{
		/* A disconnect must never compete with gameplay traffic for ingress
		   capacity. Keep one pending/in-flight cleanup per session and cancel
		   any commands that would otherwise run after that session leaves. */
		if (!m_QueuedCleanupSessionIds.insert(command.iSessionId).second)
		{
			++m_PerformanceMetrics.iDeduplicatedCleanupCommandCount;
			return ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP;
		}
		const std::size_t oldCommandCount = m_InboundCommands.size();
		std::erase_if(
			m_InboundCommands,
			[sessionId = command.iSessionId](const ROOM_COMMAND& queued)
			{
				return queued.iSessionId == sessionId;
			});
		m_PerformanceMetrics.iCancelledCommandCountByCleanup +=
			oldCommandCount - m_InboundCommands.size();
		m_CleanupCommands.push_back(std::move(command));
		m_PerformanceMetrics.iCleanupIngressHighWatermark = (std::max)(
			m_PerformanceMetrics.iCleanupIngressHighWatermark,
			m_CleanupCommands.size());
		return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED;
	}
	if (m_QueuedCleanupSessionIds.contains(command.iSessionId))
	{
		// Do not let receive traffic reappear behind pending/in-flight cleanup.
		++m_PerformanceMetrics.iCancelledCommandCountByCleanup;
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_PENDING_CLEANUP;
	}

	if (Is_BestEffortCommand(command.eType))
	{
		if (Try_RemoveCoalescedCommand(m_InboundCommands, command))
		{
			if (ROOM_COMMAND_TYPE::MOVE == command.eType)
				++m_PerformanceMetrics.iCoalescedMoveCommandCount;
			else
				++m_PerformanceMetrics.iCoalescedAimCommandCount;
			m_InboundCommands.push_back(std::move(command));
			return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED;
		}
		if (m_InboundCommands.size() >= MAX_BEST_EFFORT_COMMAND_COUNT)
		{
			++m_PerformanceMetrics.iDroppedBestEffortCommandCount;
			return ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT;
		}
	}
	else if (m_InboundCommands.size() >= MAX_RELIABLE_COMMAND_COUNT)
	{
		++m_PerformanceMetrics.iRejectedReliableCommandCount;
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY;
	}

	m_InboundCommands.push_back(std::move(command));
	m_PerformanceMetrics.iIngressHighWatermark = (std::max)(
		m_PerformanceMetrics.iIngressHighWatermark,
		m_InboundCommands.size());
	return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED;
}

std::string LostArk::Server::CGameRoom::Describe_EnqueueResult(
	const ROOM_COMMAND_ENQUEUE_RESULT result) const
{
	std::scoped_lock lock{ m_CommandMutex };
	std::string description =
		"enqueueResult=" + std::string{ To_RoomCommandEnqueueResultName(result) } +
		" worldId=" + std::to_string(static_cast<std::uint16_t>(m_eWorldId)) +
		" ingressDepth=" + std::to_string(m_InboundCommands.size()) +
		" reliableCapacity=" + std::to_string(MAX_RELIABLE_COMMAND_COUNT) +
		" acceptsCommands=" + (m_acceptsCommands ? "true" : "false");
	if (!m_isReady)
		description += " roomReady=false";
	if (!m_RuntimeFailure.strSource.empty())
	{
		description +=
			" firstFailureTick=" +
			std::to_string(m_RuntimeFailure.iServerTick) +
			" firstFailureSource=" + m_RuntimeFailure.strSource +
			" firstFailureDetail=" + m_RuntimeFailure.strDetail;
	}
	return description;
}

bool LostArk::Server::CGameRoom::Try_GetRuntimeFailure(
	SERVER_ROOM_RUNTIME_FAILURE& outFailure) const
{
	std::scoped_lock lock{ m_CommandMutex };
	if (m_RuntimeFailure.strSource.empty())
		return false;
	outFailure = m_RuntimeFailure;
	return true;
}

void LostArk::Server::CGameRoom::Mark_RuntimeFailure(
	const std::string_view source)
{
	SERVER_ROOM_RUNTIME_FAILURE failure{};
	{
		std::scoped_lock lock{ m_CommandMutex };
		if (!m_RuntimeFailure.strSource.empty())
		{
			m_isReady = false;
			return;
		}
		m_RuntimeFailure.iServerTick = m_iServerTick;
		m_RuntimeFailure.strSource = source.empty() ?
			"unspecified-room-runtime-failure" : std::string{ source };
		m_RuntimeFailure.strDetail = m_strStatus.empty() ?
			m_RuntimeFailure.strSource : m_strStatus;
		m_isReady = false;
		failure = m_RuntimeFailure;
	}
	std::cerr << "[RoomRuntimeFailure] World=" <<
		static_cast<std::uint16_t>(m_eWorldId) <<
		" Tick=" << failure.iServerTick <<
		" Source=" << failure.strSource <<
		" Detail=" << failure.strDetail << '\n';
}

LostArk::Server::SERVER_ROOM_PERFORMANCE_METRICS
LostArk::Server::CGameRoom::Get_PerformanceMetrics() const
{
	std::scoped_lock lock{ m_CommandMutex };
	return m_PerformanceMetrics;
}

std::string LostArk::Server::CGameRoom::Take_PerformanceDiagnostic()
{
	return std::exchange(m_strPendingPerformanceDiagnostic, {});
}

bool LostArk::Server::CGameRoom::Build_ValtanDecisionTraceResponse(
	const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request,
	LostArk::Shared::S2C_VALTAN_DECISION_TRACE_RESPONSE& outResponse,
	std::string& status) const
{
	using namespace LostArk::Shared;
	S2C_VALTAN_DECISION_TRACE_RESPONSE staged{};
	staged.iRequestSequence = request.iRequestSequence;
	staged.strBossPlacementId = request.strBossPlacementId;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId)
	{
		staged.eResult =
			VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_WRONG_WORLD;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}

	const auto bossIter = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind &&
				request.strBossPlacementId == entity.strPlacementId &&
				"BOSS_VALTAN" == entity.strArchetypeId &&
				"ENCOUNTER_VALTAN" == entity.strEncounterId;
		});
	if (m_WorldEntities.end() == bossIter)
	{
		staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_NO_BOSS;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}

	const VALTAN_DECISION_TRACE* trace =
		m_ValtanBrain.Get_LatestDecisionTrace();
	if (nullptr == trace)
	{
		staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::NO_TRACE;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}
	if (trace->iTraceSequence <= request.iAfterTraceSequence)
	{
		staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::UNCHANGED;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}
	if (trace->Candidates.size() > MAX_VALTAN_DECISION_TRACE_CANDIDATES ||
		m_ValtanDecisionTraceRevision.iBossEntityId != bossIter->iNetEntityId ||
		m_ValtanDecisionTraceRevision.strBossPlacementId !=
			bossIter->strPlacementId ||
		m_ValtanDecisionTraceRevision.iTraceSequence != trace->iTraceSequence ||
		!m_ValtanDecisionTraceRevision.DefinitionRevision.Is_Valid())
	{
		/* The trace is a self-contained immutable decision envelope. Its
		   DefinitionRevision is an identity for observability, not a live
		   gameplay lookup pin; keep the latest trace queryable after that old
		   generation is collected. */
		status = "Valtan decision trace revision metadata is invalid";
		return false;
	}

	const auto mapSource = [](const VALTAN_DECISION_SOURCE source,
		VALTAN_DECISION_TRACE_SOURCE& wire)
	{
		switch (source)
		{
		case VALTAN_DECISION_SOURCE::NONE:
			wire = VALTAN_DECISION_TRACE_SOURCE::NONE; return true;
		case VALTAN_DECISION_SOURCE::INTRO:
			wire = VALTAN_DECISION_TRACE_SOURCE::INTRO; return true;
		case VALTAN_DECISION_SOURCE::FORCED_HEALTH_BAR:
			wire = VALTAN_DECISION_TRACE_SOURCE::FORCED_HEALTH_BAR; return true;
		case VALTAN_DECISION_SOURCE::FORCED_AUDITION:
			wire = VALTAN_DECISION_TRACE_SOURCE::FORCED_AUDITION; return true;
		case VALTAN_DECISION_SOURCE::ORDERED:
			wire = VALTAN_DECISION_TRACE_SOURCE::ORDERED; return true;
		case VALTAN_DECISION_SOURCE::WEIGHTED:
			wire = VALTAN_DECISION_TRACE_SOURCE::WEIGHTED; return true;
		case VALTAN_DECISION_SOURCE::GLOBAL:
			wire = VALTAN_DECISION_TRACE_SOURCE::GLOBAL; return true;
		default:
			return false;
		}
	};
	const auto mapResult = [](const VALTAN_DECISION_RESULT result,
		VALTAN_DECISION_TRACE_RESULT& wire)
	{
		switch (result)
		{
		case VALTAN_DECISION_RESULT::SELECTED:
			wire = VALTAN_DECISION_TRACE_RESULT::SELECTED; return true;
		case VALTAN_DECISION_RESULT::WAITING_FOR_INTRO_RANGE:
			wire = VALTAN_DECISION_TRACE_RESULT::WAITING_FOR_INTRO_RANGE;
			return true;
		case VALTAN_DECISION_RESULT::NO_ELIGIBLE_PATTERN:
			wire = VALTAN_DECISION_TRACE_RESULT::NO_ELIGIBLE_PATTERN;
			return true;
		case VALTAN_DECISION_RESULT::NO_VALID_TARGET:
			wire = VALTAN_DECISION_TRACE_RESULT::NO_VALID_TARGET; return true;
		case VALTAN_DECISION_RESULT::CATALOG_UNAVAILABLE:
			wire = VALTAN_DECISION_TRACE_RESULT::CATALOG_UNAVAILABLE;
			return true;
		case VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED:
			wire = VALTAN_DECISION_TRACE_RESULT::MECHANIC_RESET_REQUIRED;
			return true;
		default:
			return false;
		}
	};
	const auto mapExclusions = [](const std::uint32_t source,
		std::uint32_t& wire)
	{
		struct BIT_MAP final { std::uint32_t Source; std::uint32_t Wire; };
		static constexpr BIT_MAP MAP[] =
		{
			{ VALTAN_EXCLUDE_WRONG_SELECTION_KIND,
				VALTAN_DECISION_TRACE_EXCLUDE_WRONG_SELECTION_KIND },
			{ VALTAN_EXCLUDE_INTRO_ROW,
				VALTAN_DECISION_TRACE_EXCLUDE_INTRO_ROW },
			{ VALTAN_EXCLUDE_NOT_IN_SELECTION_SET,
				VALTAN_DECISION_TRACE_EXCLUDE_NOT_IN_SELECTION_SET },
			{ VALTAN_EXCLUDE_ARMOR_MISMATCH,
				VALTAN_DECISION_TRACE_EXCLUDE_ARMOR_MISMATCH },
			{ VALTAN_EXCLUDE_PHASE_REQUIREMENT,
				VALTAN_DECISION_TRACE_EXCLUDE_PHASE_REQUIREMENT },
			{ VALTAN_EXCLUDE_PHASE_RANGE,
				VALTAN_DECISION_TRACE_EXCLUDE_PHASE_RANGE },
			{ VALTAN_EXCLUDE_HEALTH_BAR_RANGE,
				VALTAN_DECISION_TRACE_EXCLUDE_HEALTH_BAR_RANGE },
			{ VALTAN_EXCLUDE_NO_TARGET,
				VALTAN_DECISION_TRACE_EXCLUDE_NO_TARGET },
			{ VALTAN_EXCLUDE_BELOW_MINIMUM_RANGE,
				VALTAN_DECISION_TRACE_EXCLUDE_BELOW_MINIMUM_RANGE },
			{ VALTAN_EXCLUDE_ABOVE_MAXIMUM_RANGE,
				VALTAN_DECISION_TRACE_EXCLUDE_ABOVE_MAXIMUM_RANGE },
			{ VALTAN_EXCLUDE_COOLDOWN,
				VALTAN_DECISION_TRACE_EXCLUDE_COOLDOWN },
			{ VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED,
				VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_BLOCKED },
			{ VALTAN_EXCLUDE_SOFT_REPEAT_RELAXED,
				VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_RELAXED },
			{ VALTAN_EXCLUDE_DISABLED,
				VALTAN_DECISION_TRACE_EXCLUDE_DISABLED },
			{ VALTAN_EXCLUDE_UNRESOLVED_DEFINITION,
				VALTAN_DECISION_TRACE_EXCLUDE_UNRESOLVED_DEFINITION }
		};
		std::uint32_t known = 0u;
		wire = VALTAN_DECISION_TRACE_EXCLUDE_NONE;
		for (const BIT_MAP& bit : MAP)
		{
			known |= bit.Source;
			if (0u != (source & bit.Source))
				wire |= bit.Wire;
		}
		return 0u == (source & ~known) &&
			0u == (wire & ~VALTAN_DECISION_TRACE_KNOWN_EXCLUSION_MASK);
	};

	VALTAN_DECISION_TRACE_WIRE& wire = staged.Trace;
	wire.iTraceSequence = trace->iTraceSequence;
	wire.iServerTick = trace->iServerTick;
	wire.iPatternSequenceBeforeDecision =
		trace->iPatternSequenceBeforeDecision;
	wire.iExpectedPatternSequence = trace->iExpectedPatternSequence;
	wire.iCurrentHp = trace->iCurrentHp;
	wire.iMaximumHp = trace->iMaximumHp;
	wire.iHealthBar = trace->iHealthBar;
	wire.iGameplayPhase = trace->iGameplayPhase;
	wire.iTargetNetEntityId = trace->iTargetNetEntityId;
	wire.fTargetDistance = trace->fTargetDistance;
	wire.isIntroPatternConsumed = trace->bIntroPatternConsumed;
	wire.iRotationStepIndex = trace->iRotationStepIndex;
	wire.strRotationId = trace->strRotationId;
	wire.strPendingPatternId = trace->strPendingPatternId;
	wire.strSelectedPatternId = trace->strSelectedPatternId;
	wire.iRawRandomInput = trace->iRawRandomInput;
	wire.iMixedRandomValue = trace->iMixedRandomValue;
	wire.iTotalWeight = trace->iTotalWeight;
	wire.iRandomTicket = trace->iRandomTicket;
	wire.isMaximumConsecutiveRelaxed = trace->bMaximumConsecutiveRelaxed;
	wire.areCandidatesTruncated = trace->bCandidatesTruncated;
	if (!mapSource(trace->eSource, wire.eSource) ||
		!mapSource(trace->ePendingSource, wire.ePendingSource) ||
		!mapResult(trace->eResult, wire.eResult))
	{
		status = "Valtan decision trace contains an unknown selector enum";
		return false;
	}
	wire.Candidates.reserve(trace->Candidates.size());
	for (const VALTAN_DECISION_CANDIDATE_TRACE& source : trace->Candidates)
	{
		VALTAN_DECISION_TRACE_CANDIDATE_WIRE candidate{};
		candidate.strPatternId = source.strPatternId;
		if (!mapExclusions(source.iExclusionMask, candidate.iExclusionMask))
		{
			status = "Valtan decision trace contains an unknown exclusion bit";
			return false;
		}
		candidate.iAuthoredWeight = source.iAuthoredWeight;
		candidate.iEffectiveWeight = source.iEffectiveWeight;
		candidate.iCooldownRemainingTicks = source.iCooldownRemainingTicks;
		candidate.iConsecutiveUses = source.iConsecutiveUses;
		candidate.iMaximumConsecutiveUses = source.iMaximumConsecutiveUses;
		candidate.iWeightBeginInclusive = source.iWeightBeginInclusive;
		candidate.iWeightEndExclusive = source.iWeightEndExclusive;
		candidate.isSelected = source.bSelected;
		wire.Candidates.push_back(std::move(candidate));
	}
	staged.DefinitionRevision =
		m_ValtanDecisionTraceRevision.DefinitionRevision;
	staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE;
	outResponse = std::move(staged);
	status.clear();
	return true;
}

bool LostArk::Server::CGameRoom::Stage_GameplayGeneration(
	const std::uint32_t transactionSequence,
	const LostArk::Shared::GameplayDataRevision& baseRevision,
	const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
	std::string& status)
{
	std::vector<LostArk::Shared::GameplayDataRevision> livePins;
	if (!Build_RequiredPinnedGameplayRevisions(livePins))
	{
		status = "Room required gameplay revision pins are invalid";
		return false;
	}
	m_GameplayCatalog.Collect_Garbage(livePins);
	return m_GameplayCatalog.Stage(
		transactionSequence, baseRevision, candidateGeneration, status);
}

bool LostArk::Server::CGameRoom::Commit_GameplayGeneration(
	const std::uint32_t transactionSequence) noexcept
{
	if (!m_GameplayCatalog.Commit(transactionSequence))
		return false;
	const LostArk::Shared::GameplayDataRevision& activeRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		/* A boss keeps the last generation its brain evaluated until the first
		   tick that evaluates the new active catalog. That one-tick identity handoff
		   is what lets the brain reconcile a newly introduced or raised health
		   threshold after the boss is already below it. Running occurrences keep
		   the same pin for their full lifetime; definition-self-contained entities
		   can publish the new active identity immediately. */
		if (WORLD_BOOTSTRAP_KIND::BOSS != entity.eKind)
		{
			entity.PinnedDefinitionRevision = activeRevision;
		}
	}
	return true;
}

void LostArk::Server::CGameRoom::Abort_GameplayGeneration(
	const std::uint32_t transactionSequence) noexcept
{
	m_GameplayCatalog.Abort(transactionSequence);
}

bool LostArk::Server::CGameRoom::Try_SealPrivateArenaForRetirement()
{
	using LostArk::Shared::WORLD_ID;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId)
		return false;

	// Gameplay containers are room-thread-owned. The mutex makes the empty
	// decision atomic against every receive-thread Enqueue call.
	std::scoped_lock lock{ m_CommandMutex };
	if (!m_acceptsCommands)
		return true;
	if (!m_InboundCommands.empty() ||
		!m_CleanupCommands.empty() ||
		!m_QueuedCleanupSessionIds.empty() ||
		!m_PendingWorldTransfers.empty() ||
		!m_Sessions.empty() ||
		!m_Players.empty() ||
		!m_PlayerIdBySessionId.empty() ||
		!m_PlayerIdByEntityId.empty())
	{
		return false;
	}
	m_acceptsCommands = false;
	return true;
}

bool LostArk::Server::CGameRoom::Commit_WorldTransferDeparture(
	const SESSION_ID sessionId)
{
	if (!m_isReady || !m_PlayerIdBySessionId.contains(sessionId))
		return false;

	// CServerApp invokes this on the room thread after staging the target
	// REGISTER/ENTER commands. The target cannot process them until this call
	// has cleared the old CClientSession player binding.
	Leave(
		sessionId,
		LostArk::Shared::PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
	return !m_PlayerIdBySessionId.contains(sessionId);
}

void LostArk::Server::CGameRoom::Remember_ShipForWorldTransfer(
	const SERVER_WORLD_TRANSFER_REQUEST& transfer)
{
	using LostArk::Shared::WORLD_ID;
	if (WORLD_ID::BERN != m_eWorldId)
		return;
	/* Only the Maharaka trip keeps the ship; any other departure forgets it. This runs where the room
	   hands a transfer off, so the tick entry, the G interaction (the dock is a G trigger), the Debug
	   trigger and every other staging path remember it alike. */
	const auto playerIdIter = m_PlayerIdBySessionId.find(transfer.iSessionId);
	const auto playerIter = playerIdIter != m_PlayerIdBySessionId.end() ?
		m_Players.find(playerIdIter->second) : m_Players.end();
	if (WORLD_ID::MAHARAKA == transfer.eTargetWorldId && playerIter != m_Players.end() &&
		playerIter->second.bShipDockValid &&
		LostArk::Shared::INVALID_VEHICLE_ID != playerIter->second.iVehicleId)
	{
		if (m_MaharakaShipReturnBySession.size() >= 256u)
			m_MaharakaShipReturnBySession.clear();
		SHIP_RETURN_STATE state{};
		state.iVehicleId = playerIter->second.iVehicleId;
		state.fDockX = playerIter->second.fShipDockX;
		state.fDockY = playerIter->second.fShipDockY;
		state.fDockZ = playerIter->second.fShipDockZ;
		state.fDockYawDegrees = playerIter->second.fShipDockYawDegrees;
		m_MaharakaShipReturnBySession[transfer.iSessionId] = state;
	}
	else
		m_MaharakaShipReturnBySession.erase(transfer.iSessionId);
}

bool LostArk::Server::CGameRoom::Try_DequeueWorldTransfer(
	SERVER_WORLD_TRANSFER_REQUEST& outTransfer)
{
	if (m_PendingWorldTransfers.empty())
		return false;
	outTransfer = std::move(m_PendingWorldTransfers.front());
	m_PendingWorldTransfers.pop_front();
	Remember_ShipForWorldTransfer(outTransfer);
	for (const auto member : outTransfer.PartyBatchSessionIds)
	{
		if (member == outTransfer.iSessionId) continue;
		SERVER_WORLD_TRANSFER_REQUEST passenger{};
		passenger.iSessionId = member;
		passenger.eTargetWorldId = outTransfer.eTargetWorldId;
		Remember_ShipForWorldTransfer(passenger);
	}
	return true;
}

LostArk::Server::SERVER_TRIGGER_MOVE_ENTRY_RESULT
LostArk::Server::CGameRoom::Begin_MarioTriggerMove(
	const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player,
	const std::uint32_t actionStartTick)
{
	using namespace LostArk::Shared;
	using Result = SERVER_TRIGGER_MOVE_ENTRY_RESULT;
	if (m_eWorldId != WORLD_ID::KAKULSAYDON_ARENA)
		return Result::USE_DEFAULT;
	const auto lane = std::find_if(MARIO_LANES.begin(), MARIO_LANES.end(),
		[&trigger](const auto& row) { return trigger.strPlacementId == row.exit || trigger.strPlacementId == row.arrival; });
	if (lane == MARIO_LANES.end())
		return Result::USE_DEFAULT;
	if (player.iMarioStage != lane->stage || !player.iCurrentHp || !player.isCombatReady ||
		player.bPatternBound || player.bArenaEjectionActive || player.iAttachmentOwnerNetEntityId != INVALID_NET_ENTITY_ID ||
		player.eAction == PLAYER_ACTION_STATE::DEAD || player.eAction == PLAYER_ACTION_STATE::FALLING ||
		player.eAction == PLAYER_ACTION_STATE::GRABBED || trigger.TriggerActions.size() != 1u)
		return Result::RETRY_WHILE_INSIDE;
	if (player.TriggerMove.isActive && player.TriggerMove.strSourcePlacementId == trigger.strPlacementId)
		return Result::STARTED;

	WORLD_TRIGGER_ACTION action = trigger.TriggerActions.front();
	const bool terminal = trigger.strPlacementId == lane->exit &&
		std::none_of(MARIO_LANES.begin(), MARIO_LANES.end(), [&](const auto& next) {
			return next.stage == lane->stage && std::string_view(next.arrival) == lane->exit;
		});
	if (terminal)
	{
		if (Mario_MatchingBallCount(player) < 3u)
		{
			m_strStatus = "Break three balls of the marked colour before leaving Mario";
			return Result::RETRY_WHILE_INSIDE;
		}
		SERVER_NAV_POINT ground{};
		if (!Resolve_MarioReturnDestination(player, ground)) return Result::RETRY_WHILE_INSIDE;
		Refresh_PlayerBlockingBodies();
		if (!m_ServerCollisionSystem.Is_PlayerPositionClear(ground.x, ground.y, ground.z, player.iNetEntityId))
			return Result::RETRY_WHILE_INSIDE;
		action.fTargetX = ground.x; action.fTargetY = ground.y; action.fTargetZ = ground.z;
	}
	SERVER_PLAYER candidate = player;
	Reset_MarioContactAction(candidate);
	if (!CServerTriggerSystem::Begin_MovePlayer(candidate, action, actionStartTick))
		return Result::RETRY_WHILE_INSIDE;
	candidate.TriggerMove.strSourcePlacementId = trigger.strPlacementId;
	// Only a committed Mario contact cancels the previous action's delayed hits.
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player = std::move(candidate);
	return Result::STARTED;
}

bool LostArk::Server::CGameRoom::Activate_TriggerTarget(
	const WORLD_TRIGGER_ACTION_KIND kind, const std::string& targetId)
{
	if (WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE == kind)
	{
		// Rejected retired sequences must not consume the one-shot trigger.
		return Broadcast_WorldSequencePlay(targetId);
	}
	if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == kind)
		return Activate_SpawnGroupFromTrigger(targetId);
	if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == kind)
	{
		const bool activated = Activate_Encounter(targetId);
#ifdef _DEBUG
		if (activated && WORLD_ID::VALTAN_ARENA == m_eWorldId &&
			"boss.valtan.center" == targetId)
		{
			SERVER_WORLD_ENTITY* boss = Find_AuditionBoss();
			if (nullptr == boss)
				return false;
			const std::uint32_t openingHp =
				CValtanBrain::Resolve_HealthBarHp(*boss, 159u);
			if (0u == openingHp)
				return false;
			/* The next normal brain tick observes only 160 -> 159 and
			queues the real opening wall charge. Nothing here plays a
			camera, breaks a wall or emits a Client cue directly. */
			boss->iCurrentHp = openingHp;
			boss->iLastEvaluatedHealthBar = 160u;
		}
#endif
		return activated;
	}
	return false;
}

void LostArk::Server::CGameRoom::Tick(const float fixedDeltaSeconds,
	const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics)
{
	if (!m_isReady)
		return;
	const auto tickStart = std::chrono::steady_clock::now();
	const auto recordTickDuration = [this, tickStart, &schedulerMetrics]()
		{
			const std::uint64_t elapsedMicroseconds = To_Microseconds(
				std::chrono::steady_clock::now() - tickStart);
			const auto navigationMetrics = m_ServerNavigation.Get_PerformanceMetrics();
			std::scoped_lock lock{ m_CommandMutex };
			m_PerformanceMetrics.Navigation = navigationMetrics;
			m_PerformanceMetrics.Scheduler = schedulerMetrics;
			++m_PerformanceMetrics.iTickCount;
			m_PerformanceMetrics.iLastTickMicroseconds = elapsedMicroseconds;
			m_PerformanceMetrics.iMaximumTickMicroseconds = (std::max)(
				m_PerformanceMetrics.iMaximumTickMicroseconds,
				elapsedMicroseconds);
		};

	std::deque<ROOM_COMMAND> cleanupCommands;
	std::deque<ROOM_COMMAND> commands;
	{
		std::scoped_lock lock{ m_CommandMutex };
		const std::size_t cleanupIngressDepth = m_CleanupCommands.size();
		cleanupCommands.swap(m_CleanupCommands);
		const std::size_t ingressDepth = m_InboundCommands.size();
		const std::size_t drainCount = (std::min)(
			ingressDepth, MAX_COMMANDS_DRAINED_PER_TICK);
		for (std::size_t index = 0u; index < drainCount; ++index)
		{
			commands.push_back(std::move(m_InboundCommands.front()));
			m_InboundCommands.pop_front();
		}
		m_PerformanceMetrics.iLastIngressDepth = ingressDepth;
		m_PerformanceMetrics.iLastDrainedCommandCount = drainCount;
		m_PerformanceMetrics.iLastRemainingCommandCount =
			m_InboundCommands.size();
		m_PerformanceMetrics.iLastCleanupIngressDepth = cleanupIngressDepth;
		m_PerformanceMetrics.iLastDrainedCleanupCommandCount =
			cleanupCommands.size();
		m_PerformanceMetrics.iLastRemainingCleanupCommandCount =
			m_CleanupCommands.size();
		if (!m_InboundCommands.empty())
			++m_PerformanceMetrics.iDrainLimitedTickCount;
	}

	// Cleanup is independent of and always precedes the bounded gameplay drain.
	for (ROOM_COMMAND& command : cleanupCommands)
	{
		Leave(command.iSessionId, command.eLeaveReason);
		std::scoped_lock lock{ m_CommandMutex };
		m_QueuedCleanupSessionIds.erase(command.iSessionId);
	}

	for (ROOM_COMMAND& command : commands)
	{
		if (m_iColosseumMatchId && m_eColosseumPhase != LostArk::Shared::COLOSSEUM_MATCH_PHASE::PLAYING &&
			command.eType != ROOM_COMMAND_TYPE::REGISTER_SESSION && command.eType != ROOM_COMMAND_TYPE::ENTER_WORLD &&
			command.eType != ROOM_COMMAND_TYPE::COLOSSEUM_LOAD_READY && command.eType != ROOM_COMMAND_TYPE::COLOSSEUM_RETURN &&
			command.eType != ROOM_COMMAND_TYPE::ROOM_PING && command.eType != ROOM_COMMAND_TYPE::CHAT &&
			command.eType != ROOM_COMMAND_TYPE::CAPTURE_CHARACTER &&
            !(m_eColosseumPhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::RECRUITING &&
              (command.eType == ROOM_COMMAND_TYPE::MOVE || command.eType == ROOM_COMMAND_TYPE::COLOSSEUM_RECRUIT)) &&
            !(m_eColosseumPhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::FINISHED && (command.eType == ROOM_COMMAND_TYPE::INTERACT_TRIGGER || command.eType == ROOM_COMMAND_TYPE::USE_SQUAREHOLE))) continue;
		switch (command.eType)
		{
		case ROOM_COMMAND_TYPE::REGISTER_SESSION:
			Handle_Register(command.pSession);
			break;
		case ROOM_COMMAND_TYPE::ENTER_WORLD:
			Join(command.iSessionId, command.EnterWorld,
				command.strSpawnPlacementOverrideId, command.CarriedInventory,
				command.iCarriedHonorTitleId, command.strRaidReturnNpcPlacementId,
				command.CarriedPurse, command.eEntrySourceWorldId, command.bHasCarriedCharacterState);
			break;
		case ROOM_COMMAND_TYPE::MOVE:
			Handle_Move(command.iSessionId, command.Move);
			break;
		case ROOM_COMMAND_TYPE::USE_SKILL:
			Handle_UseSkill(command.iSessionId, command.UseSkill);
			break;
		case ROOM_COMMAND_TYPE::RELEASE_SKILL:
			Handle_ReleaseSkill(command.iSessionId, command.ReleaseSkill);
			break;
		case ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM:
			Handle_UpdateSkillAim(command.iSessionId, command.UpdateSkillAim);
			break;
		case ROOM_COMMAND_TYPE::USE_ESTHER_SKILL:
			Handle_UseEstherSkill(command.iSessionId, command.UseEstherSkill);
			break;
		case ROOM_COMMAND_TYPE::USE_SQUAREHOLE:
			Handle_UseSquareHole(command.iSessionId, command.UseSquareHole);
			break;
		case ROOM_COMMAND_TYPE::REVIVE_PLAYER:
			Handle_RevivePlayer(command.iSessionId, command.RevivePlayer);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_KILL_GATE_BOSSES:
			Handle_DebugKillGateBosses(command.iSessionId, command.DebugKillGateBosses);
			break;
		case ROOM_COMMAND_TYPE::SET_COOLDOWN_MODE:
			Handle_SetCooldownMode(command.iSessionId, command.SetCooldownMode);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_KILL_SELF:
			Handle_DebugKillSelf(command.iSessionId, command.DebugKillSelf);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_ENTER_KAKULSAYDON_ARENA:
			Handle_DebugEnterKakulSaydonArena(
				command.iSessionId, command.DebugEnterKakulSaydonArena);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_TELEPORT_TO_PLACEMENT:
			Handle_DebugTeleportToPlacement(
				command.iSessionId, command.DebugTeleportToPlacement);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_TELEPORT_TO_POSITION:
			Handle_DebugTeleportToPosition(
				command.iSessionId, command.DebugTeleportToPosition);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_MARIO_JUMP:
			Handle_DebugMarioJump(command.iSessionId, command.DebugMarioJump);
			break;
		case ROOM_COMMAND_TYPE::MARIO_RETURN:
			Handle_MarioReturn(command.iSessionId, command.MarioReturn);
			break;
		case ROOM_COMMAND_TYPE::MARIO_MOVE:
			Handle_MarioMove(command.iSessionId, command.MarioMove);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_BINGO_FILL:
			Handle_DebugBingoFill(command.iSessionId, command.DebugBingoFill);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_BINGO_BOMB:
			Handle_DebugBingoBomb(command.iSessionId, command.DebugBingoBomb);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_BINGO_HAMMER:
			Handle_DebugBingoHammer(command.iSessionId, command.DebugBingoHammer);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_RESUMMON_WAVE_MONSTERS:
			Handle_DebugResummonWaveMonsters(
				command.iSessionId, command.DebugResummonWaveMonsters);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_USE_ESTHER:
			Handle_DebugUseEsther(command.iSessionId, command.DebugUseEsther);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_SET_MADNESS_FORM:
			Handle_DebugSetMadnessForm(
				command.iSessionId, command.DebugSetMadnessForm);
			break;
		case ROOM_COMMAND_TYPE::SET_VEHICLE_RIDING:
			Handle_SetVehicleRiding(command.iSessionId, command.SetVehicleRiding);
			break;
		case ROOM_COMMAND_TYPE::SET_HONOR_TITLE:
			Handle_SetHonorTitle(command.iSessionId, command.SetHonorTitle);
			break;
		case ROOM_COMMAND_TYPE::INTERACTION_SLOT:
			Handle_InteractionSlot(command.iSessionId, command.InteractionSlot);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_SET_KOUKU_HUD_MODE:
			Handle_DebugSetKoukuHudMode(
				command.iSessionId, command.DebugSetKoukuHudMode);
			break;
		case ROOM_COMMAND_TYPE::CHANGE_CHARACTER_CLASS:
			Handle_ChangeCharacterClass(
				command.iSessionId, command.ChangeCharacterClass);
			break;
		case ROOM_COMMAND_TYPE::SPAWN_WORLD_ENTITY:
			Handle_SpawnWorldEntity(
				command.iSessionId,
				command.SpawnWorldEntity);
			break;
		case ROOM_COMMAND_TYPE::VALTAN_AUDITION:
			Handle_ValtanAudition(
				command.iSessionId,
				command.ValtanAudition);
			break;
		case ROOM_COMMAND_TYPE::VALTAN_PATTERN_FLOW_START:
			Handle_ValtanPatternFlowStart(
				command.iSessionId, command.ValtanPatternFlowStart);
			break;
		case ROOM_COMMAND_TYPE::VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT:
			Handle_ValtanPatternFlowStopAfterCurrent(
				command.iSessionId,
				command.ValtanPatternFlowStopAfterCurrent);
			break;
		case ROOM_COMMAND_TYPE::KOUKUSAYDON_RAID:
			Handle_KoukuRaidRequest(command.iSessionId, command.KoukuSaydonRaid);
			break;
		case ROOM_COMMAND_TYPE::KOUKUSAYDON_DRAFT_CHUNK:
			Handle_KoukuSaydonDraftChunk(command.iSessionId, command.KoukuSaydonDraftChunk);
			break;
		case ROOM_COMMAND_TYPE::KOUKUSAYDON_PATTERN_AUDITION:
			Handle_KoukuSaydonPatternAudition(
				command.iSessionId, command.KoukuSaydonPatternAudition);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_GIVE_ITEM:
			Handle_DebugGiveItem(command.iSessionId, command.DebugGiveItem);
			break;
		case ROOM_COMMAND_TYPE::USE_ITEM:
			Handle_UseItem(command.iSessionId, command.UseItem);
			break;
		case ROOM_COMMAND_TYPE::SET_EQUIPMENT:
			Handle_SetEquipment(command.iSessionId, command.SetEquipment);
			break;
		case ROOM_COMMAND_TYPE::REPAIR_EQUIPMENT:
			Handle_RepairEquipment(command.iSessionId, command.RepairEquipment);
			break;
		case ROOM_COMMAND_TYPE::BUY_ITEMS:
			Handle_BuyItems(command.iSessionId, command.BuyItems);
			break;
		case ROOM_COMMAND_TYPE::RESTORE_CHARACTER:
			Handle_RestoreCharacter(command.iSessionId, command.RestoreCharacter);
			break;
		case ROOM_COMMAND_TYPE::UPGRADE_EQUIPMENT:
			Handle_UpgradeEquipment(command.iSessionId, command.UpgradeEquipment);
			break;
		case ROOM_COMMAND_TYPE::CAPTURE_CHARACTER:
			Handle_CaptureCharacter(command.iSessionId, command.CaptureCharacter);
			break;
		case ROOM_COMMAND_TYPE::DESPAWN_ALL_WORLD_ENTITIES:
			Handle_DespawnAllWorldEntities(
				command.iSessionId, command.DespawnAllWorldEntities);
			break;
		case ROOM_COMMAND_TYPE::CONFIRM_NPC_ENTRY:
			Handle_ConfirmNpcEntry(
				command.iSessionId, command.ConfirmNpcEntry);
			break;
		case ROOM_COMMAND_TYPE::INTERACT_TRIGGER:
			Handle_InteractTrigger(
				command.iSessionId, command.InteractTrigger);
			break;
		case ROOM_COMMAND_TYPE::RETURN_TO_BERN:
			Handle_ReturnToBern(
				command.iSessionId, command.ReturnToBern);
			break;
        case ROOM_COMMAND_TYPE::MAHARAKA_AI_TUNING:
            Handle_MaharakaAITuning(command.iSessionId, command.MaharakaAITuning);
            break;
		case ROOM_COMMAND_TYPE::DEBUG_WORLD_PLAYBACK:
			Handle_DebugWorldPlayback(command.iSessionId, command.DebugWorldPlayback);
			break;
		case ROOM_COMMAND_TYPE::GUIDE_CONTROL:
			Handle_GuideControl(command.iSessionId, command.GuideControl);
			break;
		case ROOM_COMMAND_TYPE::PARTY_INVITE:
			Handle_PartyInvite(command.iSessionId, command.PartyInvite);
			break;
		case ROOM_COMMAND_TYPE::PARTY_INVITE_RESPOND:
			Handle_PartyInviteRespond(
				command.iSessionId, command.PartyInviteRespond);
			break;
		case ROOM_COMMAND_TYPE::RAID_ENTRY_PROPOSE:
			Handle_RaidEntryPropose(
				command.iSessionId, command.RaidEntryPropose);
			break;
		case ROOM_COMMAND_TYPE::RAID_ENTRY_RESPOND:
			Handle_RaidEntryRespond(
				command.iSessionId, command.RaidEntryRespond);
			break;
		case ROOM_COMMAND_TYPE::COLOSSEUM_QUEUE_JOIN:
			Handle_ColosseumQueueJoin(
				command.iSessionId, command.ColosseumQueueJoin);
			break;
		case ROOM_COMMAND_TYPE::COLOSSEUM_QUEUE_LEAVE:
			Handle_ColosseumQueueLeave(
				command.iSessionId, command.ColosseumQueueLeave);
			break;
		case ROOM_COMMAND_TYPE::COLOSSEUM_RECRUIT:
			Handle_ColosseumRecruit(command.iSessionId, command.ColosseumRecruit);
			break;
		case ROOM_COMMAND_TYPE::COLOSSEUM_LOAD_READY:
			Handle_ColosseumLoadReady(command.iSessionId, command.ColosseumLoadReady);
			break;
		case ROOM_COMMAND_TYPE::COLOSSEUM_RETURN:
			Handle_ColosseumReturn(command.iSessionId, command.ColosseumReturn);
			break;
		case ROOM_COMMAND_TYPE::GATE_PROGRESS_PROPOSE:
			Handle_GateProgressPropose(
				command.iSessionId, command.GateProgressPropose);
			break;
		case ROOM_COMMAND_TYPE::GATE_PROGRESS_RESPOND:
			Handle_GateProgressRespond(
				command.iSessionId, command.GateProgressRespond);
			break;
		case ROOM_COMMAND_TYPE::ROOM_PING:
			Handle_RoomPing(command.iSessionId, command.RoomPing);
			break;
		case ROOM_COMMAND_TYPE::CHAT:
			Handle_Chat(command.iSessionId, command.Chat);
			break;
		case ROOM_COMMAND_TYPE::LEAVE:
			// LEAVE is routed exclusively through m_CleanupCommands.
			break;
		}
	}

	Flush_PartyTransferResults();
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
	{
		recordTickDuration();
		return;
	}

	m_TickDamageEvents.clear();
	if (!m_PendingCommandDamageEvents.empty())
	{
		const size_t take = (std::min)(m_PendingCommandDamageEvents.size(),
			static_cast<size_t>(LostArk::Shared::MAX_DAMAGE_EVENTS));
		m_TickDamageEvents.assign(m_PendingCommandDamageEvents.begin(),
			m_PendingCommandDamageEvents.begin() + take);
		m_PendingCommandDamageEvents.clear();
	}
	m_TickBossCombatEvents.clear();
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	// WORLD supports and their first pattern tick must exist before any walking query.
	Prepare_KoukuAuditionTick(updateTick);
	Update_KoukuWorldBodies(updateTick);
	if (!Refresh_KoukuSupportSurfaces(updateTick)) { recordTickDuration(); return; }
	Refresh_PlayerBlockingBodies();
	for (const auto& [id, player] : m_Players)
		if (player.eCardMazeRole != LostArk::Shared::CARD_MAZE_ROLE::NONE)
			m_CardMazePreviousPositions[id] = {player.fPositionX, player.fPositionZ};
	Update_MaharakaWaterpangMatch(updateTick);
	Update_ColosseumMatch(updateTick);
	Update_Guides(fixedDeltaSeconds);
	Update_Colosseum(fixedDeltaSeconds);
	Update_WorldPickups(updateTick, false);
	Update_Players(fixedDeltaSeconds);
	Update_MaharakaWaterGunShots(updateTick);
	Update_KoukuCardRainSoldiers(updateTick);
	Update_CardMaze(updateTick);
	Update_KoukuBingo(updateTick);
	m_CombatObjectRuntime.Update(
		m_Players, m_WorldEntities, m_GameplayCatalog,
		fixedDeltaSeconds, updateTick, m_TickDamageEvents);
	std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
	std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
	bool evaluatePlayerTriggers = true;
	evaluatePlayerTriggers = WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE ==
			m_ValtanTimelineAudition.ePhase;
	if (evaluatePlayerTriggers)
	{
		m_ServerTriggerSystem.Evaluate_Entries(
		m_Players,
		updateTick,
		transfers,
		[this](const WORLD_TRIGGER_ACTION_KIND kind,
			const std::string& targetId)
		{
			return Activate_TriggerTarget(kind, targetId);
		},
		promptEdges,
		[this](const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player,
			const std::uint32_t actionStartTick)
		{
			return Begin_MarioTriggerMove(trigger, player, actionStartTick);
		});
	}
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		Update_MarioControlState(player);
	}
	for (const SERVER_INTERACT_PROMPT_EDGE& edge : promptEdges)
		Send_InteractPrompt(edge);
	Cleanup_EmptyMarioStages();
	for (SERVER_WORLD_TRANSFER_REQUEST& transfer : transfers)
	{
		if (!m_PlayerIdBySessionId.contains(transfer.iSessionId))
			continue;
		const bool alreadyStaged = std::any_of(
			m_PendingWorldTransfers.begin(),
			m_PendingWorldTransfers.end(),
			[sessionId = transfer.iSessionId](
				const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == sessionId;
			});
		if (!alreadyStaged)
			m_PendingWorldTransfers.push_back(std::move(transfer));
	}
	m_SpawnGroupRuntime.Update(
		fixedDeltaSeconds,
		m_SpawnGroupBootstrap,
		[this](const std::string& spawnGroupId)
		{
			return Count_SpawnGroupEntities(spawnGroupId);
		},
		[this](const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			const std::uint32_t ordinal)
		{
			return Spawn_Monster(
				spawnGroupId, entry, anchor, profile, ordinal);
		});
	m_EstherSkillSystem.Update(fixedDeltaSeconds, Count_HumanPlayers() != 0u);
	// Player/Esther hits may have destroyed an owned WORLD body this tick.
	// Retire its contact windows before any boss Logic can charge damage or madness.
	Update_KoukuWorldBodies(updateTick);
	Update_WorldEntities(fixedDeltaSeconds);
	Update_KoukuWorldBodies(updateTick);
	// An owner may have completed or aborted during the boss update this tick.
	if (!Refresh_KoukuSupportSurfaces(updateTick)) { recordTickDuration(); return; }
	Update_KoukuPlayerModes(updateTick);
	Update_KoukuRaid(updateTick);
	if (!m_isReady)
	{
		recordTickDuration();
		return;
	}
	/* Boss root motion is resolved after the ordinary player update. Refresh a
	grabbed fallback once more at that committed pose so the snapshot never
	trails its attachment owner by one fixed tick. */
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (LostArk::Shared::PLAYER_ACTION_STATE::GRABBED == player.eAction)
			(void)Update_PlayerAttachment(player, updateTick);
	}
	if (!Commit_DueEncounterProps(updateTick) ||
		!Commit_DueWorldDestruction(updateTick))
	{
		Mark_RuntimeFailure("fixed-tick.due-world-transaction");
		recordTickDuration();
		return;
	}
	Update_WorldPickups(updateTick);
	Drain_BossCombatEvents();
	if (!Flush_KoukuSaydonPatternAuditionLifecycle())
	{
		m_strStatus = "KoukuSaydon pattern audition lifecycle serialization failed";
		Mark_RuntimeFailure("fixed-tick.koukusaydon-audition-lifecycle");
		recordTickDuration();
		return;
	}
	// Current completion and Next promotion observe the final committed tick.
	// A promoted ID cannot reach BeginPattern until the next world update.
	(void)Refresh_ValtanPatternIdAuditionState();
	if (!Flush_ValtanPatternIdAuditionLifecycle())
	{
		m_strStatus = "Valtan audition lifecycle serialization failed";
		Mark_RuntimeFailure("fixed-tick.valtan-audition-lifecycle");
		recordTickDuration();
		return;
	}
	if (!Flush_ValtanPatternFlowLifecycle())
	{
		m_strStatus = "Valtan pattern-flow lifecycle serialization failed";
		Mark_RuntimeFailure("fixed-tick.valtan-flow-lifecycle");
		recordTickDuration();
		return;
	}
	if (!Broadcast_CombatObjectLifecycle())
	{
		Mark_RuntimeFailure("fixed-tick.combat-object-lifecycle");
		recordTickDuration();
		return;
	}
	Enforce_VehicleRidingState();
	Score_ColosseumKills(updateTick);
	m_iServerTick = updateTick;
	if (m_iColosseumMatchId && updateTick % 3u == 0u) Broadcast_ColosseumMatchState();
	Expire_RaidEntryProposals();
	Expire_GateProgressVote();
	/* A hit that wore gear answers its owner with the inventory snapshot that carries the wear. */
	for (auto& [wornPlayerId, wornPlayer] : m_Players)
	{
		if (!wornPlayer.bDurabilityDirty || !wornPlayer.Is_Human())
			continue;
		wornPlayer.bDurabilityDirty = false;
		const std::shared_ptr<CClientSession> wornSession = Find_Session(wornPlayer.iSessionId);
		if (nullptr != wornSession && !Send_InventorySnapshot(wornSession, 0u, wornPlayer))
			wornSession->Request_Close();
	}
	if (Count_HumanPlayers() != 0u)
		Broadcast_WorldSnapshot();
	std::vector<LostArk::Shared::GameplayDataRevision> liveGenerationPins;
	if (!Build_RequiredPinnedGameplayRevisions(liveGenerationPins))
	{
		m_strStatus = "Gameplay generation pin set exceeded its wire bound";
		Mark_RuntimeFailure("fixed-tick.gameplay-generation-pins");
		recordTickDuration();
		return;
	}
	m_GameplayCatalog.Collect_Garbage(liveGenerationPins);
	recordTickDuration();
	if (!m_Players.empty() && 0u == (m_iServerTick % 300u))
	{
		const SERVER_ROOM_PERFORMANCE_METRICS metrics =
			Get_PerformanceMetrics();
		std::size_t maximumCurrentOutboundFrames = 0u;
		std::size_t maximumOutboundFrameHighWatermark = 0u;
		std::uint64_t snapshotCoalescedCount = 0u;
		std::uint64_t snapshotDroppedCount = 0u;
		std::uint64_t reliableRejectedCount = 0u;
		std::uint64_t sendFailureCount = 0u;
		std::uint64_t maximumWireSendMicroseconds = 0u;
		for (const auto& [sessionId, weakSession] : m_Sessions)
		{
			(void)sessionId;
			const std::shared_ptr<CClientSession> session = weakSession.lock();
			if (nullptr == session)
				continue;
			const CLIENT_SESSION_OUTBOUND_METRICS sessionMetrics =
				session->Get_OutboundMetrics();
			maximumCurrentOutboundFrames = (std::max)(
				maximumCurrentOutboundFrames,
				sessionMetrics.iCurrentQueuedFrameCount);
			maximumOutboundFrameHighWatermark = (std::max)(
				maximumOutboundFrameHighWatermark,
				sessionMetrics.iQueuedFrameHighWatermark);
			snapshotCoalescedCount +=
				sessionMetrics.iSnapshotCoalescedFrameCount;
			snapshotDroppedCount +=
				sessionMetrics.iSnapshotDroppedFrameCount;
			reliableRejectedCount +=
				sessionMetrics.iReliableRejectedFrameCount;
			sendFailureCount += sessionMetrics.iSendFailureCount;
			maximumWireSendMicroseconds = (std::max)(
				maximumWireSendMicroseconds,
				sessionMetrics.iMaximumFrameSendMicroseconds);
		}
		const bool hasNewFailure =
			metrics.Scheduler.iScheduleResetCount >
				m_LastRoomPerfLogSample.Scheduler.iScheduleResetCount ||
			metrics.iDrainLimitedTickCount >
				m_LastRoomPerfLogSample.iDrainLimitedTickCount ||
			metrics.iDroppedBestEffortCommandCount >
				m_LastRoomPerfLogSample.iDroppedBestEffortCommandCount ||
			metrics.iRejectedReliableCommandCount >
				m_LastRoomPerfLogSample.iRejectedReliableCommandCount ||
			metrics.iRejectedCleanupCommandCount >
				m_LastRoomPerfLogSample.iRejectedCleanupCommandCount ||
			metrics.iSnapshotEncodeFailureCount >
				m_LastRoomPerfLogSample.iSnapshotEncodeFailureCount ||
			metrics.iSnapshotEnqueueFailureCount >
				m_LastRoomPerfLogSample.iSnapshotEnqueueFailureCount ||
			snapshotDroppedCount > m_iLastRoomPerfSnapshotDroppedCount ||
			reliableRejectedCount > m_iLastRoomPerfReliableRejectedCount ||
			sendFailureCount > m_iLastRoomPerfWireSendFailureCount;
		const bool hasCurrentPressure =
			metrics.Scheduler.iPreviousLoopLatenessMicroseconds >= 33333u ||
			(metrics.Scheduler.iMaximumLoopLatenessMicroseconds >= 33333u &&
				metrics.Scheduler.iMaximumLoopLatenessMicroseconds >
					m_LastRoomPerfLogSample.Scheduler.iMaximumLoopLatenessMicroseconds) ||
			metrics.iLastTickMicroseconds >= 33333u ||
			(metrics.iMaximumTickMicroseconds >= 33333u &&
				metrics.iMaximumTickMicroseconds >
					m_LastRoomPerfLogSample.iMaximumTickMicroseconds) ||
			0u != metrics.iLastRemainingCommandCount ||
			0u != metrics.iLastRemainingCleanupCommandCount ||
			maximumCurrentOutboundFrames >= 64u ||
			(maximumOutboundFrameHighWatermark >= 64u &&
				maximumOutboundFrameHighWatermark >
					m_iLastRoomPerfOutboundHighWatermark);
		const bool isHeartbeat = 0u == (m_iServerTick % 1800u);
		m_LastRoomPerfLogSample = metrics;
		m_iLastRoomPerfSnapshotDroppedCount = snapshotDroppedCount;
		m_iLastRoomPerfReliableRejectedCount = reliableRejectedCount;
		m_iLastRoomPerfWireSendFailureCount = sendFailureCount;
		m_iLastRoomPerfOutboundHighWatermark =
			maximumOutboundFrameHighWatermark;
		if (!hasNewFailure && !hasCurrentPressure && !isHeartbeat)
			return;

		const auto unixMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
		const auto& combatObjects = m_CombatObjectRuntime.Get_LiveObjects();
		const auto replicatedCombatObjects = std::count_if(combatObjects.begin(), combatObjects.end(),
			[](const auto& object) { return object.bReplicated; });
		std::ostringstream diagnostic;
		diagnostic << "[RoomPerf] UnixMs=" << unixMilliseconds << " Kind="
			<< (hasNewFailure || hasCurrentPressure ? "anomaly" : "heartbeat")
			<< " World=" << static_cast<unsigned>(m_eWorldId)
			<< " Tick=" << m_iServerTick
			<< " Gate=" << std::quoted(m_KoukuRaid.State.strGateId)
			<< " FlowIndex=" << m_KoukuRaid.State.iFlowEntryIndex
			<< " FlowEntry=" << std::quoted(m_KoukuRaid.State.strFlowEntryId)
			<< " RaidPhase=" << static_cast<unsigned>(m_KoukuRaid.State.ePhase)
			<< " RaidEpoch=" << m_KoukuRaid.State.iRunEpoch
			<< " RaidReason=" << std::quoted(m_KoukuRaid.State.strReason)
			<< " SequencePattern=" << std::quoted(m_KoukuRaid.State.strSequencePatternId)
			<< " ActionRevision=" << m_KoukuRaid.State.iActionSourceRevision
			<< " SequenceRevision=" << m_KoukuRaid.State.iSequenceSourceRevision
			<< " ReadyMask=" << static_cast<unsigned>(m_KoukuRaid.State.iReadyMask)
			<< " Participants=" << m_KoukuRaid.State.ParticipantPlayerIds.size()
			<< " LiveCombatObjects=" << combatObjects.size()
			<< " ReplicatedCombatObjects=" << replicatedCombatObjects
			<< " ReplicatedCombatObjectCap=" << LostArk::Shared::MAX_COMBAT_OBJECTS_PER_SNAPSHOT
			<< " LoopSampleUnixMs=" << metrics.Scheduler.iSampleUnixMilliseconds
			<< " PreviousLoopLateUs=" << metrics.Scheduler.iPreviousLoopLatenessMicroseconds
			<< " LoopMaxLateUs=" << metrics.Scheduler.iMaximumLoopLatenessMicroseconds
			<< " LoopScheduleResets=" << metrics.Scheduler.iScheduleResetCount
			<< " TickUs=" << metrics.iLastTickMicroseconds
			<< " TickMaxUs=" << metrics.iMaximumTickMicroseconds
			<< " Ingress=" << metrics.iLastIngressDepth
			<< " IngressHigh=" << metrics.iIngressHighWatermark
			<< " Drained=" << metrics.iLastDrainedCommandCount
			<< " Remaining=" << metrics.iLastRemainingCommandCount
			<< " CleanupIngress=" << metrics.iLastCleanupIngressDepth
			<< " CleanupIngressHigh=" << metrics.iCleanupIngressHighWatermark
			<< " CleanupDrained="
			<< metrics.iLastDrainedCleanupCommandCount
			<< " CleanupRemaining="
			<< metrics.iLastRemainingCleanupCommandCount
			<< " CleanupDeduplicated="
			<< metrics.iDeduplicatedCleanupCommandCount
			<< " CleanupCancelledCommands="
			<< metrics.iCancelledCommandCountByCleanup
			<< " MoveCoalesced=" << metrics.iCoalescedMoveCommandCount
			<< " AimCoalesced=" << metrics.iCoalescedAimCommandCount
			<< " BestEffortDropped="
			<< metrics.iDroppedBestEffortCommandCount
			<< " ReliableRejected="
			<< metrics.iRejectedReliableCommandCount
			<< " SnapshotEncodeUs="
			<< metrics.iLastSnapshotEncodeMicroseconds
			<< " SnapshotEnqueueUs="
			<< metrics.iLastSnapshotEnqueueMicroseconds
			<< " SessionEnqueueMaxUs="
			<< metrics.iMaximumSessionEnqueueMicroseconds
			<< " SnapshotEnqueueFailures="
			<< metrics.iSnapshotEnqueueFailureCount
			<< " OutboundQueuedMax=" << maximumCurrentOutboundFrames
			<< " OutboundHighMax=" << maximumOutboundFrameHighWatermark
			<< " SnapshotCoalesced=" << snapshotCoalescedCount
			<< " SnapshotDropped=" << snapshotDroppedCount
			<< " OutboundReliableRejected=" << reliableRejectedCount
			<< " WireSendMaxUs=" << maximumWireSendMicroseconds
			<< " WireSendFailures=" << sendFailureCount;
		diagnostic << " PatternIds=";
		bool hasPattern = false;
		for (const auto& member : m_KoukuSaydonPatternAudition.Members)
		{
			if (member.bCompleted || member.iPatternIndex >= member.PatternIds.size()) continue;
			if (hasPattern) diagnostic << ',';
			diagnostic << std::quoted(member.PatternIds[member.iPatternIndex]);
			hasPattern = true;
		}
		if (!hasPattern) diagnostic << "none";
		const auto writeNavigation = [&diagnostic](const char* name,
			const SERVER_NAVIGATION_QUERY_METRICS& stage)
			{
				diagnostic << " Nav" << name << "Calls=" << stage.iCalls
					<< " Nav" << name << "TotalUs=" << stage.iTotalNanoseconds / 1000u
					<< " Nav" << name << "MaxUs=" << stage.iMaximumNanoseconds / 1000u
					<< " Nav" << name << "Expanded=" << stage.iExpandedNodes
					<< " Nav" << name << "PathPoints=" << stage.iReturnedPathPoints;
			};
		writeNavigation("FindPath", metrics.Navigation.FindPath);
		writeNavigation("ReachablePath", metrics.Navigation.ReachablePath);
		writeNavigation("ProjectPoint", metrics.Navigation.ProjectPoint);
		writeNavigation("SmoothPath", metrics.Navigation.SmoothPath);
		writeNavigation("TraversalStep", metrics.Navigation.TraversalStep);
		writeNavigation("LineOfSight", metrics.Navigation.LineOfSight);
		m_strPendingPerformanceDiagnostic = diagnostic.str();
		std::cout << m_strPendingPerformanceDiagnostic << '\n';
	}
}


bool LostArk::Server::CGameRoom::Stage_NumericBalance(
    const std::uint32_t transactionSequence,
    const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status)
{
    LostArk::Shared::GameplayDataRevision ignored;
    std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY> entries;
    if (!candidate || !candidate->Build_NumericBalanceSnapshot(entries, ignored, status)) return false;
    decltype(m_StagedNumericCatalogRemaps) remaps;
    for (const auto& original : {m_KoukuRaid.pCatalog, m_KoukuSaydonPatternAudition.pProductGeneration,
        m_pKoukuPublishedProductGeneration, m_GameplayCatalog.Get_ActiveGeneration()})
    {
        if (!original || std::any_of(remaps.begin(),remaps.end(),[&](const auto& pair){return pair.first==original;})) continue;
        std::shared_ptr<const CGameplayCatalog> replacement = candidate;
        if (original != m_GameplayCatalog.Get_ActiveGeneration())
        {
            auto copy = std::make_shared<CGameplayCatalog>();
            if (!copy->Load_NumericBalanceValues(*original,*candidate))
            { status = "Running Kouku Product rejects numeric balance: " + copy->Get_Status(); return false; }
            replacement = std::move(copy);
        }
        remaps.emplace_back(original,std::move(replacement));
    }
    if (!m_GameplayCatalog.Stage_NumericBalance(transactionSequence,candidate,status)) return false;
    m_StagedNumericEntries = std::move(entries);
    m_StagedNumericCatalogRemaps = std::move(remaps);
    return true;
}

bool LostArk::Server::CGameRoom::Commit_NumericBalance(
    const std::uint32_t transactionSequence) noexcept
{
    const auto previous = m_GameplayCatalog.Get_ActiveGeneration();
    if (!previous || !m_GameplayCatalog.Commit_NumericBalance(transactionSequence)) return false;
    for (auto* target : {&m_KoukuRaid.pCatalog,&m_KoukuSaydonPatternAudition.pProductGeneration,&m_pKoukuPublishedProductGeneration})
        for (const auto& pair : m_StagedNumericCatalogRemaps)
            if (*target == pair.first) { *target = pair.second; break; }
    m_StagedNumericCatalogRemaps.clear();
    const auto ratio = [](const std::uint32_t current, const std::uint32_t oldMaximum,
        const std::uint32_t newMaximum, const bool keepAlive)
    {
        if (!current || !newMaximum) return 0u;
        const auto scaled = oldMaximum ? static_cast<std::uint32_t>((
            static_cast<std::uint64_t>(current) * newMaximum + oldMaximum / 2u) / oldMaximum) : 0u;
        return (std::min)(newMaximum, (std::max)(keepAlive ? 1u : 0u, scaled));
    };
    for (auto& [sessionId, player] : m_Players)
    {
        const auto* profile = m_GameplayCatalog.Find_Player(player.eCharacterClass);
        if (!profile) continue;
        // A match pins its admitted 20-bar HP pool and full damage reference
        // until departure; class/boss HP edits affect future admissions only.
        const bool matchedColosseum = m_eWorldId == LostArk::Shared::WORLD_ID::COLOSSEUM &&
            m_iColosseumMatchId != 0u && player.iColosseumMatchId == m_iColosseumMatchId &&
            player.iColosseumTeam < 2u && (player.Is_Human() || player.Is_ColosseumMercenary());
        const auto maximumHp = matchedColosseum ? player.iMaximumHp : profile->iMaximumHp;
        player.iCurrentHp = ratio(player.iCurrentHp, player.iMaximumHp, maximumHp, true);
        player.iMaximumHp = maximumHp;
        player.iCurrentResource = ratio(player.iCurrentResource, player.iMaximumResource, profile->iMaximumResource, false);
        player.iMaximumResource = profile->iMaximumResource;
        player.iCurrentIdentity = ratio(player.iCurrentIdentity, player.iMaximumIdentity, profile->iMaximumIdentity, false);
        player.iMaximumIdentity = profile->iMaximumIdentity;
        player.fMoveSpeed = profile->fMoveSpeed;
        const auto* oldPlayer = previous->Find_Player(player.eCharacterClass);
        for (auto& projectile : player.Projectiles)
        {
            const auto* oldSkill = previous->Find_Skill(projectile.iSkillId);
            const auto* newSkill = m_GameplayCatalog.Find_Skill(projectile.iSkillId);
            const auto* oldDamage = oldSkill ? previous->Find_DamageProfile(oldSkill->strDamageProfileId) : nullptr;
            const auto* newDamage = newSkill ? m_GameplayCatalog.Active().Find_DamageProfile(newSkill->strDamageProfileId) : nullptr;
            if (!oldPlayer || !oldDamage || !newDamage) continue;
            const auto oldRaw = CGameplayCatalog::Resolve_Damage(oldPlayer->iAttackPower, *oldDamage);
            const auto newRaw = CGameplayCatalog::Resolve_Damage(profile->iAttackPower, *newDamage);
            if (oldRaw) projectile.iTotalDamage = (std::min)(static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()),
                projectile.iTotalDamage * newRaw / oldRaw);
        }
    }
    m_CombatObjectRuntime.Refresh_NumericBalance(*previous, m_GameplayCatalog.Active(), m_StagedNumericEntries);
    m_StagedNumericEntries.clear();
    for (auto& boss : m_WorldEntities)
    {
        if (WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind) continue;
        // Ghost resurrection deliberately retains the primary Valtan entity ID.
        const auto* profile = m_GameplayCatalog.Find_Boss(
            boss.bGhostPhasePatternLoopActive && boss.strArchetypeId == "BOSS_VALTAN" ? "BOSS_VALTAN_GHOST" : boss.strArchetypeId);
        if (!profile) continue;
        boss.iCurrentHp = ratio(boss.iCurrentHp, boss.iMaximumHp, profile->iMaximumHp, true);
        boss.iLastEvaluatedHealthBar = ratio(boss.iLastEvaluatedHealthBar, boss.iMaximumHealthBars, profile->iMaximumHealthBars, false);
        boss.iMaximumHp = profile->iMaximumHp; boss.iMaximumHealthBars = profile->iMaximumHealthBars;
        boss.iDamageReferenceHp = profile->iDamageReferenceHp;
        boss.iAttackPower = profile->iAttackPower; boss.fCollisionRadius = profile->fCollisionRadius;
        boss.fEngageDistance = profile->fEngageDistance; boss.fMoveSpeed = profile->fMoveSpeed;
        const auto commonStagger = m_GameplayCatalog.Active().Get_RaidStaggerMaximum();
        if (commonStagger && Uses_RaidStaggerPolicy(boss))
        {
            if (boss.iKoukuItemStaggerMaximum)
            {
                boss.iKoukuItemStaggerCredit = ratio(boss.iKoukuItemStaggerCredit,
                    boss.iKoukuItemStaggerMaximum, commonStagger, false);
                boss.iKoukuItemStaggerMaximum = commonStagger;
            }
            for (const auto& retained : boss.KoukuRetainedLogicOwners)
                if (const auto owner = retained.lock(); owner && owner->iKoukuItemStaggerMaximum)
                {
                    owner->iKoukuItemStaggerCredit = ratio(owner->iKoukuItemStaggerCredit,
                        owner->iKoukuItemStaggerMaximum, commonStagger, false);
                    owner->iKoukuItemStaggerMaximum = commonStagger;
                }
            if (boss.BossCombat.iStaggerMaximum)
            {
                boss.BossCombat.iStaggerCurrent = ratio(boss.BossCombat.iStaggerCurrent,
                    boss.BossCombat.iStaggerMaximum, commonStagger, false);
                boss.BossCombat.iStaggerMaximum = commonStagger;
                ++boss.BossCombat.iStateRevision;
            }
            continue;
        }
        if (!boss.BossCombat.iStaggerMaximum) continue;
        const auto* pinned = m_GameplayCatalog.Resolve(boss.PinnedDefinitionRevision);
        const auto* patterns = pinned ? pinned->Find_BossPatterns(boss.strEncounterId) : nullptr;
        if (!patterns) continue;
        const auto pattern = std::find_if(patterns->begin(), patterns->end(), [&](const auto& p) { return p.strPatternId == boss.strPatternId; });
        if (pattern == patterns->end() || boss.iPatternStageIndex >= pattern->Stages.size()) continue;
        for (const auto& action : pattern->Stages[boss.iPatternStageIndex].Actions)
            if (action.eKind == BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE && action.iValue &&
                action.eTrigger == BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER && action.iValue != boss.BossCombat.iStaggerMaximum)
            {
                boss.BossCombat.iStaggerCurrent = ratio(boss.BossCombat.iStaggerCurrent,
                    boss.BossCombat.iStaggerMaximum, action.iValue, false);
                boss.BossCombat.iStaggerMaximum = action.iValue;
                ++boss.BossCombat.iStateRevision;
                break;
            }
    }
    return true;
}

void LostArk::Server::CGameRoom::Abort_NumericBalance(const std::uint32_t transactionSequence) noexcept
{
    m_GameplayCatalog.Abort_NumericBalance(transactionSequence);
    m_StagedNumericEntries.clear();
    m_StagedNumericCatalogRemaps.clear();
}
~~~~

<a id="file-server-private-gameroom-replication-cpp"></a>

### Server/Private/GameRoom_Replication.cpp

파일: C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp

기존 snapshot encode 1회를 유지하고, room owner가 recipient shared_ptr 목록을 준비한 뒤 Dispatch_WorldSnapshot으로 fanout한다. 기존 performance metrics에 같은 범위의 집계를 연결한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~cpp
#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerSnapshotFanout.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"
#include "Gameplay/MaharakaWaterpangContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

bool LostArk::Server::CGameRoom::Send_Accepted(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	S2C_ENTER_ACCEPTED message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = m_eWorldId;
	message.iPlayerId = player.iPlayerId;
	message.iNetEntityId = player.iNetEntityId;
	message.ActiveGameplayRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	if (!Build_RequiredPinnedGameplayRevisions(
		message.RequiredPinnedGameplayRevisions))
	{
		return false;
	}
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_ENTER_ACCEPTED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_EnterRejected(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_ENTER_REJECTED message{};
	message.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	message.eWorldId = m_eWorldId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_ENTER_REJECTED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_Spawned(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	S2C_PLAYER_SPAWNED message{};
	message.iPlayerId = player.iPlayerId;
	message.iNetEntityId = player.iNetEntityId;
	message.eCharacterClass = player.eCharacterClass;
	message.eControlKind = player.eControlKind;
        message.strWaterpangNpcArchetypeId = player.strWaterpangNpcArchetypeId;
	message.strNickName = player.strNickName;
	message.iVoiceType = player.iVoiceType;
	message.strAppearanceJson = player.strAppearanceJson;
	message.fPositionX = player.fPositionX;
	message.fPositionY = player.fPositionY;
	message.fPositionZ = player.fPositionZ;
	message.fYawDegrees = player.fYawDegrees;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_PLAYER_SPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_WorldEntitySpawned(
	const std::shared_ptr<CClientSession>& session,
	const SERVER_WORLD_ENTITY& entity)
{
	std::vector<std::uint8_t> payload;
	return nullptr != session &&
		Build_WorldEntitySpawnedPayload(entity, payload) &&
		session->Send_Frame(
			LostArk::Shared::PACKET_TYPE::S2C_WORLD_ENTITY_SPAWNED, payload);
}

bool LostArk::Server::CGameRoom::Build_WorldEntitySpawnedPayload(
	const SERVER_WORLD_ENTITY& entity,
	std::vector<std::uint8_t>& outPayload)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWNED message{};
	message.iNetEntityId = entity.iNetEntityId;
	message.iOwnerBossNetEntityId = entity.iOwnerBossNetEntityId;
	message.eKind = To_NetworkKind(entity.eKind);
	message.strArchetypeId = entity.strArchetypeId;
	message.strEncounterId = entity.strEncounterId;
	message.strPlacementId = entity.strPlacementId;
	message.strActionId = entity.strActionId;
	message.fPositionX = entity.fPositionX;
	message.fPositionY = entity.fPositionY;
	message.fPositionZ = entity.fPositionZ;
	message.fYawDegrees = entity.fYawDegrees;
	message.fCollisionRadius = entity.fCollisionRadius;
	message.PinnedDefinitionRevision = entity.PinnedDefinitionRevision;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return false;
	outPayload = writer.Get_Buffer();
	return true;
}

bool LostArk::Server::CGameRoom::Send_WorldEntityDespawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_DESPAWNED message{};
	message.iNetEntityId = netEntityId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_DESPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_CombatObjectSpawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned)
{
	using namespace LostArk::Shared;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, spawned) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_Despawned(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	using namespace LostArk::Shared;
	S2C_PLAYER_DESPAWNED message{};
	message.iNetEntityId = netEntityId;
	message.eReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(PACKET_TYPE::S2C_PLAYER_DESPAWNED, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_WorldDestructionFullSync(
	const std::shared_ptr<CClientSession>& session)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId)
		return true;
	if (nullptr == session || !m_WorldDestructionRuntime.Is_Initialized())
		return false;

	S2C_WORLD_DESTRUCTION_FULL_SYNC message{};
	message.strCombatRuntimeRevision =
		m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
	message.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	message.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
	for (const WORLD_DESTRUCTION_GROUP_STATE& state :
		m_WorldDestructionRuntime.Get_GroupStates())
	{
		message.GroupStates.push_back(To_NetworkDestructionState(state));
	}
	message.Diagnostics = Build_WorldDestructionDiagnostics();
	CPacketWriter writer;
	return Write_Message(writer, message) && session->Send_Frame(
		PACKET_TYPE::S2C_WORLD_DESTRUCTION_FULL_SYNC, writer.Get_Buffer());
}

LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
LostArk::Server::CGameRoom::Build_WorldDestructionDiagnostics() const
{
	LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS diagnostics{};
	diagnostics.iActiveWallCollisionCount = static_cast<std::uint32_t>(
		m_ServerCollisionSystem.Get_ActivePlayerBlockingCount());
	diagnostics.iActiveNavBlockerRegionCount = static_cast<std::uint32_t>(
		m_ServerNavigation.Get_ActiveBlockerRegionCount());
	diagnostics.iNavigationRevision = m_ServerNavigation.Get_Revision();
	diagnostics.iLastEventSequence =
		0u == m_iNextWorldDestructionEventSequence ?
		0u : m_iNextWorldDestructionEventSequence - 1u;
	return diagnostics;
}

void LostArk::Server::CGameRoom::Broadcast_Spawned(
	const SERVER_PLAYER& player,
	const SESSION_ID exceptSessionId)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		if (sessionId == exceptSessionId)
			continue;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_Spawned(session, player))
			session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Broadcast_Despawned(
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::PLAYER_DESPAWN_REASON reason)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_Despawned(session, netEntityId, reason))
			session->Request_Close();
	}
}

bool LostArk::Server::CGameRoom::Send_WorldEntitySpawnResult(
	const std::shared_ptr<CClientSession>& session,
	const std::string& placementId,
	const LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	using namespace LostArk::Shared;
	S2C_WORLD_ENTITY_SPAWN_RESULT message{};
	message.strPlacementId = placementId;
	message.eResult = result;
	message.iNetEntityId = netEntityId;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_ENTITY_SPAWN_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_InventorySnapshot(
	const std::shared_ptr<CClientSession>& session,
	const std::uint32_t requestSequence,
	const SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	S2C_INVENTORY_SNAPSHOT message{};
	message.iRequestSequence = requestSequence;
	message.Items = player.Inventory;
	message.iSilver = player.Purse.iSilver;
	message.iGold = player.Purse.iGold;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_INVENTORY_SNAPSHOT, writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_ValtanAuditionResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
	const LostArk::Shared::VALTAN_AUDITION_RESULT result,
	const std::uint32_t currentHealthBar)
{
	using namespace LostArk::Shared;
	S2C_VALTAN_AUDITION_RESULT message{};
	message.iRequestSequence = request.iRequestSequence;
	message.eOperation = request.eOperation;
	message.iTargetHealthBar = request.iTargetHealthBar;
	message.eResult = result;
	message.iCurrentHealthBar = currentHealthBar;
	message.strBossPlacementId = request.strBossPlacementId;
	message.strPatternId = request.strPatternId;
	message.iPredecessorRoomAuditionEpoch = request.iPredecessorRoomAuditionEpoch;
	message.iPredecessorPatternSequence = request.iPredecessorPatternSequence;
	message.iExpectedNextRequestSequence = request.iExpectedNextRequestSequence;
	message.ExpectedDefinitionRevision = request.ExpectedDefinitionRevision;
	message.ReplacementDefinitionRevision =
		request.ReplacementDefinitionRevision;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_VALTAN_AUDITION_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_KoukuSaydonPatternAuditionResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::
		S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message)
{
	using namespace LostArk::Shared;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_ValtanPatternFlowResult(
	const std::shared_ptr<CClientSession>& session,
	const std::uint32_t commandSequence,
	const LostArk::Shared::VALTAN_PATTERN_FLOW_COMMAND command,
	const LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT result,
	const std::string& flowId,
	const std::string& flowRevision,
	const std::uint32_t roomFlowEpoch,
	const LostArk::Shared::GameplayDataRevision& pinnedRevision,
	const std::string& reason)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT message{};
	message.iCommandSequence = commandSequence;
	message.eCommand = command;
	message.eResult = result;
	message.strFlowId = flowId;
	message.strFlowRevision = flowRevision;
	message.iRoomFlowEpoch = roomFlowEpoch;
	message.PinnedDefinitionRevision = pinnedRevision;
	message.strReason = reason;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_DEBUG_VALTAN_PATTERN_FLOW_RESULT,
			writer.Get_Buffer());
}

bool LostArk::Server::CGameRoom::Send_CharacterClassChangeResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
	const LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
	const LostArk::Shared::CHARACTER_CLASS_ID activeClass)
{
	using namespace LostArk::Shared;
	S2C_CHARACTER_CLASS_CHANGE_RESULT message{};
	message.iClientSequence = request.iClientSequence;
	message.eResult = result;
	message.eRequestedClass = request.eCharacterClass;
	message.eActiveClass = activeClass;
	CPacketWriter writer;
	return nullptr != session && Write_Message(writer, message) &&
		session->Send_Frame(
			PACKET_TYPE::S2C_CHARACTER_CLASS_CHANGE_RESULT,
			writer.Get_Buffer());
}

void LostArk::Server::CGameRoom::Broadcast_WorldEntitySpawned(
	const SERVER_WORLD_ENTITY& entity)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_WorldEntitySpawned(session, entity))
			session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Broadcast_WorldEntityDespawned(
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	const LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason)
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session &&
			!Send_WorldEntityDespawned(session, netEntityId, reason))
		{
			session->Request_Close();
		}
	}
}

bool LostArk::Server::CGameRoom::Broadcast_CombatObjectLifecycle()
{
	using namespace LostArk::Shared;
	std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
	std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> presentationEvents;
	std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
	m_CombatObjectRuntime.Drain_Lifecycle(
		spawned, presentationEvents, despawned);
	for (const S2C_COMBAT_OBJECT_SPAWNED& message : spawned)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
		{
			(void)playerId;
			const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
			if (nullptr != session && !session->Send_Frame(
				PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED, writer.Get_Buffer()))
			{
				session->Request_Close();
			}
		}
	}
	for (const S2C_COMBAT_OBJECT_PRESENTATION_EVENT& message :
		presentationEvents)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
		{
			(void)playerId;
			const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
			if (nullptr != session && !session->Send_Frame(
				PACKET_TYPE::S2C_COMBAT_OBJECT_PRESENTATION_EVENT,
				writer.Get_Buffer()))
			{
				session->Request_Close();
			}
		}
	}
	for (const S2C_COMBAT_OBJECT_DESPAWNED& message : despawned)
	{
		CPacketWriter writer;
		if (!Write_Message(writer, message))
			return false;
		for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
		{
			(void)playerId;
			const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
			if (nullptr != session && !session->Send_Frame(
				PACKET_TYPE::S2C_COMBAT_OBJECT_DESPAWNED, writer.Get_Buffer()))
			{
				session->Request_Close();
			}
		}
	}
	return true;
}

bool LostArk::Server::CGameRoom::Broadcast_WorldDestructionDelta(
	const std::vector<WORLD_DESTRUCTION_STATE_TRANSITION>& transitions,
	const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>& liveEvents,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	if (transitions.empty() && liveEvents.empty())
		return true;

	S2C_WORLD_DESTRUCTION_DELTA message{};
	message.strCombatRuntimeRevision =
		m_WorldDestructionBootstrap.Get_CombatRuntimeRevision();
	message.iServerTick = serverTick;
	message.iEncounterEpoch = m_WorldDestructionRuntime.Get_EncounterEpoch();
	for (const WORLD_DESTRUCTION_STATE_TRANSITION& transition : transitions)
	{
		WORLD_DESTRUCTION_GROUP_STATE state{};
		if (!m_WorldDestructionRuntime.Find_GroupState(
			transition.strGroupId, state))
		{
			return false;
		}
		message.ChangedStates.push_back(To_NetworkDestructionState(state));
	}
	std::sort(message.ChangedStates.begin(), message.ChangedStates.end(),
		[](const WORLD_DESTRUCTION_STATE_WIRE& left,
			const WORLD_DESTRUCTION_STATE_WIRE& right)
		{
			return left.strGroupId < right.strGroupId;
		});
	message.LiveEvents = liveEvents;
	message.Diagnostics = Build_WorldDestructionDiagnostics();

	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return false;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !session->Send_Frame(
			PACKET_TYPE::S2C_WORLD_DESTRUCTION_DELTA, writer.Get_Buffer()))
		{
			session->Request_Close();
		}
	}
	return true;
}

void LostArk::Server::CGameRoom::Broadcast_WorldSnapshot()
{
	using namespace LostArk::Shared;
	S2C_WORLD_SNAPSHOT message{};
	message.iServerTick = m_iServerTick;
	message.eWorldId = m_eWorldId;
	message.iEstherGauge = m_EstherSkillSystem.Get_Gauge();
	message.iEstherGaugeMaximum = m_EstherSkillSystem.Get_GaugeMaximum();
	message.ActiveGameplayRevision =
		m_GameplayCatalog.Get_ActiveRevision();
	if (!Build_RequiredPinnedGameplayRevisions(
		message.RequiredPinnedGameplayRevisions))
	{
		m_strStatus = "Live gameplay revision pins are invalid or exceed wire bounds";
		return;
	}
	message.Players.reserve(m_Players.size());
	message.Entities.reserve(m_WorldEntities.size());
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		PLAYER_SNAPSHOT snapshot{};
		snapshot.iNetEntityId = player.iNetEntityId;
		snapshot.eCharacterClass = player.eCharacterClass;
		snapshot.eControlKind = player.eControlKind;
		snapshot.fPositionX = player.fPositionX;
		snapshot.fPositionY = player.fPositionY;
		snapshot.fPositionZ = player.fPositionZ;
		snapshot.fYawDegrees = player.fYawDegrees;
		snapshot.iLastProcessedMoveSequence = player.iLastMoveSequence;
		snapshot.fMoveSpeed = Resolve_PlayerMoveSpeed(player);
		// Client click prediction samples static navigation and cannot represent
		// this room-owned floor. Keep input and Server movement active, but send
		// the existing authoritative interpolation mode while standing on it.
		const auto& supports = m_ServerNavigation.Get_RuntimeSupportSurfaces();
		const bool onRuntimeSupport = std::any_of(supports.begin(), supports.end(),
			[&](const auto& surface) { return surface.Contains_PointXZ(player.fPositionX, player.fPositionZ); });
		// The Client predictor currently loads the base grid only. Refined Kouku
		// regions use the same authoritative interpolation as temporary floors.
		const bool onKoukuDetailGrid = m_eWorldId == WORLD_ID::KAKULSAYDON_ARENA &&
			m_ServerNavigation.Is_InSameDetailRegion(player.fPositionX, player.fPositionZ,
				player.fPositionX, player.fPositionZ);
		snapshot.canPredictMove = !onRuntimeSupport && !onKoukuDetailGrid &&
			PLAYER_ACTION_STATE::NONE == player.eAction &&
			player.iCurrentHp != 0u && !player.bPatternBound && !player.Has_TimeStop(m_iServerTick) &&
			player.iMarioStage == 0u && !player.TriggerMove.isActive &&
			player.fKnockbackRemainingSeconds <= 0.f &&
			player.CardMaze.transferStartTick == 0u &&
			(!(player.CardMaze.flags & 1u) ||
			 m_KoukuCardMaze.Is_SoloHunter(player.iPlayerId));
		if (WORLD_ID::VALTAN_ARENA == m_eWorldId &&
			VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE != m_ValtanTimelineAudition.ePhase &&
			!(VALTAN_TIMELINE_AUDITION_PHASE::WAITING_PATTERN_FINISH ==
				m_ValtanTimelineAudition.ePhase &&
			  player.iPlayerId == m_ValtanTimelineAudition.iOwnerPlayerId))
		{
			snapshot.canPredictMove = false;
		}
		snapshot.hasMoveGoal = snapshot.canPredictMove && player.hasMoveGoal;
		if (snapshot.hasMoveGoal)
		{
			const SERVER_NAV_POINT waypoint = player.iMovePathIndex < player.MovePath.size() ?
				player.MovePath[player.iMovePathIndex] :
				SERVER_NAV_POINT{ player.fMoveGoalX, player.fPositionY, player.fMoveGoalZ };
			snapshot.fMoveWaypointX = waypoint.x;
			snapshot.fMoveWaypointY = waypoint.y;
			snapshot.fMoveWaypointZ = waypoint.z;
		}
		snapshot.eLocomotionState =
			(player.hasMoveGoal ||
				(player.TriggerMove.isActive &&
					player.TriggerMove.fHeldSeconds >= player.TriggerMove.fHoldSeconds)) ?
			PLAYER_LOCOMOTION_STATE::MOVING : PLAYER_LOCOMOTION_STATE::IDLE;
		snapshot.eAction = player.eAction;
		snapshot.isKnockbackAirborne = player.eAction == PLAYER_ACTION_STATE::KNOCKDOWN &&
			player.bKnockbackBallistic && player.fKnockbackRemainingSeconds > 0.f;
		// Water gun: armed from the intro start tick while the body stands on the
		// Waterpang arena region; an arena push-off or knockdown keeps it until the
		// fall resolves. The empty-room reset clears the match and so the gun.
		snapshot.isWaterpangArmed = Is_MaharakaWaterpangArmed(player);
		snapshot.iWaterGunSkillId = player.iWaterGunSkillId;
		snapshot.iWaterGunCastTick = player.iWaterGunCastTick;
		snapshot.eStance = player.eStance;
		snapshot.iSkillId = player.iCurrentSkillId;
		snapshot.iActionStartTick = player.iActionStartTick;
		if (PLAYER_ACTION_STATE::GRABBED == player.eAction)
		{
			snapshot.iAttachmentOwnerNetEntityId =
				player.iAttachmentOwnerNetEntityId;
			snapshot.eAttachmentSlot = player.eAttachmentSlot;
			snapshot.fAttachmentLocalOffsetX =
				player.fAttachmentLocalOffsetX;
			snapshot.fAttachmentLocalOffsetY =
				player.fAttachmentLocalOffsetY;
			snapshot.fAttachmentLocalOffsetZ =
				player.fAttachmentLocalOffsetZ;
			snapshot.fAttachmentYawOffsetDegrees =
				player.fAttachmentYawOffsetDegrees;
		}
		if (PLAYER_ACTION_STATE::SKILL == player.eAction &&
			player.hasSkillTarget)
		{
			snapshot.hasSkillTarget = true;
			snapshot.fSkillTargetX = player.fSkillTargetX;
			snapshot.fSkillTargetY = player.fSkillTargetY;
			snapshot.fSkillTargetZ = player.fSkillTargetZ;
		}
		snapshot.iCurrentHp = player.iCurrentHp;
		snapshot.iMaximumHp = player.iMaximumHp;
		snapshot.iShield = player.iShield;
		// Item visuals must survive snapshot coalescing without removing real skill buffs.
		if (player.Has_TimeStop(m_iServerTick))
			snapshot.ActiveBuffs[snapshot.iActiveBuffCount++] = {33500u, player.iTimeStopEndTick};
		if (player.Has_HolyCharmProtection(m_iServerTick))
			snapshot.ActiveBuffs[snapshot.iActiveBuffCount++] = {32282u, player.iHolyCharmProtectionEndTick};
        if (m_eWorldId == LostArk::Shared::WORLD_ID::MAHARAKA && player.iCurrentHp &&
            player.iWaterGunSpeedEndTick && static_cast<std::int32_t>(player.iWaterGunSpeedEndTick - m_iServerTick) > 0)
            snapshot.ActiveBuffs[snapshot.iActiveBuffCount++] = {569200u, player.iWaterGunSpeedEndTick};
		for (const auto& buff : player.ActiveBuffs)
		{
			if (snapshot.iActiveBuffCount == LostArk::Shared::MAX_ACTIVE_BUFFS) break;
			snapshot.ActiveBuffs[snapshot.iActiveBuffCount++] = buff;
		}
		snapshot.iCurrentResource = player.iCurrentResource;
		snapshot.iMaximumResource = player.iMaximumResource;
		snapshot.iCurrentIdentity = player.iCurrentIdentity;
		snapshot.iMaximumIdentity = player.iMaximumIdentity;
		if (const GUARDIAN_EMBER_PROFILE* ember =
			m_GameplayCatalog.Active().Find_EmberProfile(player.eCharacterClass))
		{
			snapshot.iEmberOrbs = static_cast<std::uint8_t>(player.iEmberOrbs);
			snapshot.iEmberLockedSockets =
				static_cast<std::uint8_t>(player.iEmberLockedSockets);
			snapshot.iEmberMaximumSockets =
				static_cast<std::uint8_t>(ember->iMaximumSockets);
		}
		snapshot.iCurrentMadness = player.iCurrentMadness;
		snapshot.iMaximumMadness = player.iMaximumMadness;
		snapshot.eMadnessForm = player.eMadnessForm;
		snapshot.iVehicleId = player.iVehicleId;
		snapshot.eVehicleFlightPhase = player.eVehicleFlightPhase;
		snapshot.iVehicleFlightPhaseStartTick = player.iVehicleFlightPhaseStartTick;
		if (const auto* vehicle = m_VehicleCatalog.Find_Vehicle(player.iVehicleId))
		{
			if (player.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::TAKEOFF)
				snapshot.fVehicleFlightPhaseDurationSeconds = vehicle->fFlightTakeoffSeconds;
			else if (player.eVehicleFlightPhase == VEHICLE_FLIGHT_PHASE::LANDING && vehicle->fFlightVerticalSpeed > 0.f)
				snapshot.fVehicleFlightPhaseDurationSeconds = (std::max)(vehicle->fFlightLandingSeconds,
					player.fVehicleFlightStartHeight / vehicle->fFlightVerticalSpeed);
		}
		snapshot.iHonorTitleId = player.iHonorTitleId;
		for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item : player.Inventory)
		{
			if (LostArk::Shared::EQUIPMENT_SLOT::AVATAR_HEAD == item.eEquippedSlot)
				snapshot.strAvatarHeadItemId = item.strItemId;
			else if (LostArk::Shared::EQUIPMENT_SLOT::AVATAR_OUTFIT == item.eEquippedSlot)
				snapshot.strAvatarOutfitItemId = item.strItemId;
		}
		snapshot.eMechanicCardSymbol = player.eMechanicCardSymbol;
		snapshot.eMechanicCardColor = player.eMechanicCardColor;
		snapshot.eKoukuHudMode = player.eKoukuHudMode;
		for (std::size_t slot = 0u; slot < KOUKU_HUD_SLOT_COUNT; ++slot)
			snapshot.ModeSkillIndexBySlot[slot] = player.ModeSkillIndexBySlot[slot];
		snapshot.iMarioStage = player.iMarioStage;
		snapshot.iKoukuMinigameEndTick = player.iKoukuMinigameEndTick;
		snapshot.iMarioLayoutVariant = player.iMarioLayoutVariant;
		snapshot.iMarioPoppedBallMask = player.iMarioStage >= 1u && player.iMarioStage <= 4u ?
			m_MarioPoppedBalls[player.iMarioStage] : std::uint16_t{};
		snapshot.iMarioCurseReleasedMask = player.iMarioRequiredColor >= 1u && player.iMarioRequiredColor <= 3u ?
			static_cast<std::uint8_t>(Mario_CurseReleasedMask(player.iMarioStage, player.iMarioLayoutVariant) &
				(1u << (player.iMarioRequiredColor - 1u))) : 0u;
		snapshot.iMarioMarkerColor = Mario_MarkerColor(player.iNetEntityId);
		if (player.iMarioStage >= 1u && player.iMarioStage <= 4u)
		{
			snapshot.iMarioRequiredColor = player.iMarioRequiredColor;
			snapshot.iMarioMatchingBallCount = (std::min)(std::uint8_t{ 3u }, Mario_MatchingBallCount(player));
		}
		snapshot.eCardMazeRole = player.eCardMazeRole;
		snapshot.eCardMazeSuit = player.eCardMazeSuit;
		snapshot.iCardMazeKills = player.iCardMazeKills;
		snapshot.iCardMazeKillTarget = player.iCardMazeKillTarget;
		snapshot.CardMaze = player.CardMaze;
		snapshot.isCombatReady = player.isCombatReady;
		snapshot.isPatternBound = player.bPatternBound;
		snapshot.iPatternBindEndTick = player.iPatternBindEndTick;
        if (PLAYER_ACTION_STATE::FEAR == player.eAction)
        {
            snapshot.iFearEndTick = player.iFearEndTick;
            snapshot.strFearPresentationId = player.strFearPresentationId;
        }
		// A blocked Valtan wipe is a one-tick verdict. Retain its occurrence for one
		// second so snapshot coalescing cannot erase the text before a Client frame.
		const bool recentEstherBlock = m_eWorldId == WORLD_ID::VALTAN_ARENA &&
			player.iInvulnerabilityZonePulseTick != 0u &&
			player.iInvulnerabilityZoneContactTick == player.iInvulnerabilityZonePulseTick &&
			m_iServerTick >= player.iInvulnerabilityZonePulseTick &&
			m_iServerTick - player.iInvulnerabilityZonePulseTick < SERVER_TICK_HZ;
		if ((player.iInvulnerabilityZoneContactTick == m_iServerTick || recentEstherBlock) && m_iServerTick != 0u &&
			player.iCurrentHp != 0u && player.isCombatReady && player.eAction != PLAYER_ACTION_STATE::DEAD &&
			player.eAction != PLAYER_ACTION_STATE::FALLING && player.eAction != PLAYER_ACTION_STATE::GRABBED)
			snapshot.iInvulnerabilityZonePulseTick = player.iInvulnerabilityZonePulseTick;
		if (m_eWorldId == WORLD_ID::VALTAN_ARENA && !player.Is_Guide() &&
			player.iCurrentHp != 0u && player.eAction != PLAYER_ACTION_STATE::DEAD)
		{
			snapshot.bRonaunGuard = player.bRonaunGuard;
			snapshot.iRonaunGrantTick = player.iRonaunGrantTick;
		}
		snapshot.iSilenceEndTick = player.iSilenceEndTick;
		snapshot.iSilenceDurationTicks = player.iSilenceDurationTicks;
		snapshot.iComboStage = player.iComboStage;
		snapshot.eCooldownMode = m_eCooldownMode;
		/* Collect, sort, then truncate: cutting during unordered_map iteration
		made the surviving cooldowns depend on hash order. Signed difference keeps
		ordering across a wrapped tick counter. */
		for (const auto& [skillId, cooldownEndTick] :
			player.CooldownEndTickBySkillId)
		{
			if (static_cast<std::int32_t>(cooldownEndTick - m_iServerTick) > 0)
			{
				const auto duration = player.CooldownDurationTicksBySkillId.find(skillId);
				snapshot.Cooldowns.push_back({ skillId, cooldownEndTick,
					duration != player.CooldownDurationTicksBySkillId.end() ? duration->second : 0u });
			}
		}
		/* Item slots retain their inventory item ID. Project each active item cooldown
		through its catalog skill ID so the same HUD deadline path can show reuse time. */
		for (const auto& [itemId, cooldownEndTick] : player.ItemCooldownEndTicks)
		{
			if (static_cast<std::int32_t>(cooldownEndTick - m_iServerTick) <= 0) continue;
			const auto* definition = m_ItemCatalog.Find_Item(itemId);
			if (!definition || definition->BattleUse.eKind == BATTLE_ITEM_KIND::NONE) continue;
			const auto& use = definition->BattleUse;
			snapshot.Cooldowns.push_back({ use.iSkillId, cooldownEndTick,
				(use.iCooldownMs * SERVER_TICK_HZ + 999u) / 1000u });
		}
		std::sort(snapshot.Cooldowns.begin(), snapshot.Cooldowns.end(),
			[](const SKILL_COOLDOWN_SNAPSHOT& left,
				const SKILL_COOLDOWN_SNAPSHOT& right)
			{
				return left.iSkillId < right.iSkillId;
			});
		if (snapshot.Cooldowns.size() > MAX_PLAYER_COOLDOWNS)
			snapshot.Cooldowns.resize(MAX_PLAYER_COOLDOWNS);
		message.Players.push_back(snapshot);
	}
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (entity.eKind == WORLD_BOOTSTRAP_KIND::WORLD_OBJECT) continue; // Owned World cue presents this body.
		WORLD_ENTITY_SNAPSHOT snapshot{};
		snapshot.iNetEntityId = entity.iNetEntityId;
		snapshot.eAction = To_NetworkAction(entity.eAction);
		snapshot.strPatternId = entity.strPatternId;
		if (entity.eKind == WORLD_BOOTSTRAP_KIND::NPC ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_WINDUP ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_ACTIVE ||
			entity.eAction == SERVER_ENTITY_ACTION::PATTERN_RECOVERY)
		{
			snapshot.strActionId = entity.strActionId;
		}
		snapshot.fPositionX = entity.fPositionX;
		snapshot.fPositionY = entity.fPositionY;
		snapshot.fPositionZ = entity.fPositionZ;
		snapshot.fYawDegrees = entity.fYawDegrees;
		snapshot.iActionStartTick = entity.iActionStartTick;
		snapshot.iPatternSequence = entity.iPatternSequence;
		snapshot.iPatternStartTick = entity.iPatternStartTick;
		snapshot.iPatternStageIndex = entity.iPatternStageIndex;
		if (entity.KoukuDirectionPlayback && !entity.strPatternId.empty())
		{
			const auto& child = *entity.KoukuDirectionPlayback;
			snapshot.strPresentationPatternId = child.strPatternId;
			snapshot.strPresentationActionId = child.strActionId;
			snapshot.iPresentationPatternStartTick = child.iPatternStartTick;
			snapshot.iPresentationActionStartTick = child.iActionStartTick;
			snapshot.iPresentationPatternStageIndex = child.iPatternStageIndex;
		}
		snapshot.iCurrentHp = entity.iCurrentHp;
		snapshot.iMaximumHp = entity.iMaximumHp;
		snapshot.iActiveBuffCount = static_cast<std::uint8_t>((std::min)(
			entity.ActiveBuffs.size(), LostArk::Shared::MAX_ACTIVE_BUFFS));
		for (std::size_t buffIndex = 0; buffIndex < snapshot.iActiveBuffCount; ++buffIndex)
			snapshot.ActiveBuffs[buffIndex] = entity.ActiveBuffs[buffIndex];
		snapshot.iPhase = entity.iPhase;
		snapshot.PinnedDefinitionRevision =
			entity.PinnedDefinitionRevision;
		/* Presentation only needs to know which plates came off, not how much
		durability is left, so the wire carries one bit per authored plate. */
		for (const SERVER_BOSS_ARMOR_PLATE_STATE& plate : entity.ArmorPlates)
		{
			if (0u == plate.iRemainingDurability &&
				plate.iPlateIndex <
					LostArk::Shared::MAX_WORLD_ENTITY_ARMOR_PLATES)
			{
				snapshot.iBrokenArmorMask |= static_cast<std::uint8_t>(
					1u << plate.iPlateIndex);
			}
		}
		if (WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind)
		{
			snapshot.iPatternTargetNetEntityId =
				entity.iPatternTargetEntityId;
			if (!entity.strPatternId.empty() && entity.fPatternLeapApexHeight > 0.f &&
				(entity.eAction == SERVER_ENTITY_ACTION::PATTERN_WINDUP ||
				 entity.eAction == SERVER_ENTITY_ACTION::PATTERN_ACTIVE ||
				 entity.eAction == SERVER_ENTITY_ACTION::PATTERN_RECOVERY))
			{
				snapshot.PatternLanding.isValid = true;
				snapshot.PatternLanding.fPositionX = entity.fLeapLandingX;
				snapshot.PatternLanding.fPositionY = entity.fLeapLandingY;
				snapshot.PatternLanding.fPositionZ = entity.fLeapLandingZ;
			}
			if (entity.bPortalMotionActive &&
				entity.bPortalRushTargetLocked &&
				BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
					entity.ePatternStageMotionKind)
			{
				snapshot.PortalRushRoute.isValid = true;
				snapshot.PortalRushRoute.fStartX = entity.fPortalStartX;
				snapshot.PortalRushRoute.fStartY = entity.fSpawnPositionY;
				snapshot.PortalRushRoute.fStartZ = entity.fPortalStartZ;
				snapshot.PortalRushRoute.fEndX = entity.fPortalEndX;
				snapshot.PortalRushRoute.fEndY = entity.fSpawnPositionY;
				snapshot.PortalRushRoute.fEndZ = entity.fPortalEndZ;
			}
			snapshot.hasBossCombatState = true;
			snapshot.BossCombat.iStateRevision =
				entity.BossCombat.iStateRevision;
			snapshot.BossCombat.iAlivePartMask =
				entity.BossCombat.iAlivePartMask;
			snapshot.BossCombat.iFlags = static_cast<std::uint16_t>(
				entity.BossCombat.iFlags) &
				BOSS_COMBAT_STATE_KNOWN_FLAG_MASK;
			snapshot.BossCombat.iCurrentStagger =
				entity.BossCombat.iStaggerCurrent;
			snapshot.BossCombat.iMaximumStagger =
				entity.BossCombat.iStaggerMaximum;
			snapshot.BossCombat.iCurrentShield =
				entity.BossCombat.iShieldCurrent;
			snapshot.BossCombat.iMaximumShield =
				entity.BossCombat.iShieldMaximum;
			if (BOSS_PATTERN_BOSS_RESPONSE_KIND::ACCUMULATED_HEALTH_DAMAGE ==
				entity.ePatternBossResponseKind)
			{
				snapshot.BossCombat.iResponseThreshold =
					entity.iPatternBossResponseThreshold;
				snapshot.BossCombat.iResponseProgress = (std::min)(
					entity.iPatternBossResponseAccumulatedHealthDamage,
					entity.iPatternBossResponseThreshold);
			}
			if (const auto* member = Find_KoukuAuditionMember(entity.iNetEntityId, entity.iPatternSequence))
			{
				const auto* catalog = Resolve_KoukuProductCatalog();
				std::string status;
				const auto* pattern = catalog ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(
					*catalog, member->LogicLedger.strPatternId, status) : nullptr;
				if (pattern) CKoukuSaydonLogicRuntime::Project_MechanicGauge(
					entity, *pattern, member->LogicLedger, m_iServerTick, snapshot.BossCombat);
			}
			/* The existing gameplay phase remains the one authority. The boss
			payload mirrors it rather than introducing a second phase clock. */
			snapshot.BossCombat.iGameplayPhase = entity.iPhase;
		}
		message.Entities.push_back(std::move(snapshot));
	}
	message.DamageEvents = m_TickDamageEvents;
	message.Bingo.iWhiteMask = m_KoukuBingo.Get_WhiteMask();
	message.Bingo.iRedMask = m_KoukuBingo.Get_RedMask();
	message.Bingo.Hammer = m_KoukuBingo.Get_Hammer();
	message.Bingo.iBombCount = 0u;
	for (const auto& bomb : m_KoukuBingo.Get_Bombs())
	{
		if (LostArk::Shared::BINGO_BOMB_PHASE::NONE == bomb.ePhase)
			continue;
		auto& packed = message.Bingo.Bombs[message.Bingo.iBombCount++];
		packed.ePhase = bomb.ePhase;
		packed.iCarrierNetEntityId = bomb.iCarrierNetEntityId;
		packed.fPositionX = bomb.fPositionX;
		packed.fPositionZ = bomb.fPositionZ;
	}
	if (m_eWorldId == WORLD_ID::VALTAN_ARENA)
		for (const WORLD_PICKUP_RUNTIME& pickup : m_WorldPickups)
			message.WorldPickups.push_back(pickup.Snapshot);
	message.BossCombatEvents = m_TickBossCombatEvents;
	if (!m_CombatObjectRuntime.Build_Snapshots(message.CombatObjects))
		return;

	const auto encodeStart = std::chrono::steady_clock::now();
	CPacketWriter writer;
	const bool encoded = Write_Message(writer, message);
	const std::uint64_t encodeMicroseconds = To_Microseconds(
		std::chrono::steady_clock::now() - encodeStart);
	{
		std::scoped_lock lock{ m_CommandMutex };
		++m_PerformanceMetrics.iSnapshotEncodeCount;
		m_PerformanceMetrics.iLastSnapshotEncodeMicroseconds =
			encodeMicroseconds;
		m_PerformanceMetrics.iMaximumSnapshotEncodeMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSnapshotEncodeMicroseconds,
			encodeMicroseconds);
		if (!encoded)
			++m_PerformanceMetrics.iSnapshotEncodeFailureCount;
	}
	if (!encoded)
		return;

	std::vector<std::shared_ptr<CClientSession>> recipients;
	recipients.reserve(m_PlayerIdBySessionId.size());
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		if (auto session = Find_Session(sessionId))
			recipients.push_back(std::move(session));
	}
	const auto fanout = Dispatch_WorldSnapshot(recipients, writer.Get_Buffer(), m_SnapshotJobs.get());
	const auto enqueueBatchMicroseconds = fanout.ElapsedMicroseconds;
	const auto maximumSessionEnqueueMicroseconds = fanout.MaximumSessionMicroseconds;
	const auto recipientCount = fanout.Recipients;
	const auto enqueueFailureCount = fanout.Failures;
	{
		std::scoped_lock lock{ m_CommandMutex };
		++m_PerformanceMetrics.iSnapshotEnqueueBatchCount;
		m_PerformanceMetrics.iSnapshotRecipientCount += recipientCount;
		m_PerformanceMetrics.iSnapshotEnqueueFailureCount += enqueueFailureCount;
		m_PerformanceMetrics.iLastSnapshotEnqueueMicroseconds =
			enqueueBatchMicroseconds;
		m_PerformanceMetrics.iMaximumSnapshotEnqueueMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSnapshotEnqueueMicroseconds,
			enqueueBatchMicroseconds);
		m_PerformanceMetrics.iLastMaximumSessionEnqueueMicroseconds =
			maximumSessionEnqueueMicroseconds;
		m_PerformanceMetrics.iMaximumSessionEnqueueMicroseconds = (std::max)(
			m_PerformanceMetrics.iMaximumSessionEnqueueMicroseconds,
			maximumSessionEnqueueMicroseconds);
	}
}

std::shared_ptr<LostArk::Server::CClientSession>
LostArk::Server::CGameRoom::Find_Session(const SESSION_ID sessionId) const
{
	const auto iter = m_Sessions.find(sessionId);
	return iter == m_Sessions.end() ? nullptr : iter->second.lock();
}
~~~~

<a id="file-server-private-iocpservice-cpp"></a>

### Server/Private/IocpService.cpp

파일: C:/Users/tnest/Desktop/LostArk/Server/Private/IocpService.cpp

Start는 worker와 completion port를 만들고, Submit은 syscall 전에 operation 소유권을 넘긴다. Worker_Loop는 완료·오류를 participant에 전달한 뒤 pending을 감소시킨다. Stop은 모든 세션 종료 뒤 남은 완료를 회수하고 worker를 join한다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
// 한국어 설명을 붙인 IOCP 서비스 구현 완성본.
// 적용 대상은 Server/Private/IocpService.cpp이며 이 사본은 제품 프로젝트에 자동 연결되지 않는다.
// 읽는 순서: OPERATION → Start → Associate → Post_Receive/Post_Send → Submit → Worker_Loop → Stop.
// 실제 게임의 패킷 조립과 방 명령 처리는 이후 CClientSession 연결 단계의 책임이다.
#include "IocpService.h"

#include <algorithm>
#include <array>
#include <limits>
#include <exception> // std::terminate의 직접 선언. 프로세스 종료 호출이 반환할 때의 마지막 안전장치.

namespace
{
    // 각 워커가 자신이 속한 서비스를 기록한다. 전역 하나를 공유하면 다른 워커와 덮어쓴다.
    // Is_WorkerThread가 이를 비교하여 자기 스레드를 join하는 종료 교착을 막는다.
    thread_local const LostArk::Server::CIocpService* ActiveService = nullptr;
    // 완료를 회수하지 못한 상태에서는 Windows가 OVERLAPPED/버퍼에 계속 접근할 수 있다.
    // 메모리를 버리고 서버만 계속 실행하는 대신 프로세스 전체를 실패로 종료한다.
    // 워커 하나를 TerminateThread로 강제 종료하는 함수가 아니다.
    [[noreturn]] void Fail_DrainTimeout() noexcept
    {
        ::TerminateProcess(::GetCurrentProcess(), ERROR_TIMEOUT);
        std::terminate();
    }
}

// 비동기 작업 한 건의 수명 단위. 함수의 지역 버퍼는 함수 반환과 함께 사라지므로 사용하지 않는다.
// Windows에는 OVERLAPPED 기반 포인터를 넘기고, 완료 때 원래 OPERATION으로 되돌린다.
// 소유권은 제출 전 unique_ptr → 접수 중 완료 큐가 추적하는 raw 주소 → 워커 unique_ptr 순서다.
// Owner가 세션을, SendBytes가 송신 데이터를 살린다. 수신 메모리는 작업 객체 자체가 소유한다.
// 이 단순 구현은 송신 작업에도 4096-byte ReceiveBytes 공간을 가진다. 풀링/작업별 분리 비용은
// 이후 측정할 최적화 후보이며 이 코드가 할당·메모리 비용 없는 구현이라는 뜻은 아니다.
struct LostArk::Server::CIocpService::OPERATION final : OVERLAPPED
{
    // Internal/Offset/hEvent를 0으로 초기화한다. 완료 패킷을 생략하는 특수 모드는 사용하지 않는다.
    OPERATION() : OVERLAPPED{} {}
    IOCP_OPERATION_KIND Kind = IOCP_OPERATION_KIND::RECEIVE;
    std::shared_ptr<IIocpParticipant> Owner;
    std::array<std::uint8_t, 4096> ReceiveBytes{};
    std::shared_ptr<const std::vector<std::uint8_t>> SendBytes;
    // WSABUF는 메모리를 소유하지 않는다. 위 수신 배열 또는 SendBytes 내부를 가리키는 포인터/길이다.
    WSABUF Buffer{};
};

// 서비스 소유자가 파괴할 때 남은 워커/핸들이 없도록 한다. 세션 먼저 종료하는 순서는 여전히 필요하다.
LostArk::Server::CIocpService::~CIocpService() { Stop(); }

// 헤더의 Start 선언에 대응한다. 서비스 소유자가 한 번 호출하며 세션 접수 전에 끝낸다.
// 포트/워커 생성에 실패하면 이미 만든 자원을 Stop으로 회수하고 false를 반환한다.
bool LostArk::Server::CIocpService::Start(std::size_t workerCount)
{
    if (m_Port || m_Running.load()) return false;
    // 0은 자동 선택 요청이다. CPU 개수는 시작값일 뿐, 접속 수/패킷 크기에 따른 최적값은 실측한다.
    if (!workerCount) workerCount = (std::max)(1u, std::thread::hardware_concurrency());
    workerCount = (std::clamp)(workerCount, std::size_t{1}, std::size_t{16});
    // INVALID_HANDLE_VALUE로 우선 소켓과 연결되지 않은 공용 완료 포트만 만든다.
    // 마지막 인자는 동시에 실행 가능한 포트 작업 스레드 수이며 워커를 생성하는 호출은 아니다.
    m_Port = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0,
        static_cast<DWORD>(workerCount));
    if (!m_Port) return false;
    m_Running.store(true);
    try
    {
        // 실제 스레드는 여기서 만든다. 생성 직후 각 워커는 Worker_Loop에서 완료를 기다린다.
        m_Workers.reserve(workerCount);
        for (std::size_t i = 0; i < workerCount; ++i)
            m_Workers.emplace_back(&CIocpService::Worker_Loop, this);
    }
    catch (...) { Stop(); return false; }
    return true;
}

// 종료 호출 순서: 외부에서 세션의 새 제출 중단/소켓 닫기 → 이 Stop → 서비스 파괴.
// 이 함수가 모든 세션의 소켓을 대신 닫지는 않는다. 취소 완료도 워커가 회수한 뒤 끝낸다.
// worker 안에서 호출하면 자신을 join할 수 없으므로 잘못된 수명 순서로 처리한다.
void LostArk::Server::CIocpService::Stop(const std::chrono::milliseconds timeout)
{
    if (!m_Port) return;
    if (Is_WorkerThread()) Fail_DrainTimeout();
    // 새 Submit의 진입을 거부한다. 이미 진행 중인 Submit을 기다리는 잠금은 아니므로
    // 소유자가 제출자들을 먼저 정리했다는 수명 계약이 반드시 필요하다.
    m_Running.store(false);
    {
        // I/O 완료 콜백이 반환할 때까지 기다린다. pending은 maintenance 실행 여부는 세지 않는다.
        // 작업 객체의 파괴와 maintenance 종료까지는 뒤의 worker join으로 확정한다.
        // 조건변수 대기는 잠금을 풀고 쉰다. Complete_Pending이 마지막 작업에서 깨운다.
        std::unique_lock lock{m_DrainMutex};
        if (!m_Drained.wait_for(lock, timeout, [this] { return m_Pending.load() == 0; }))
            Fail_DrainTimeout();
    }
    // pending이 0이면 실제 작업 대신 overlapped=null인 종료 신호를 워커 수만큼 넣는다.
    // 각 워커가 하나를 받으면 반복문에서 빠져나오므로 다른 워커도 종료 신호를 받을 수 있다.
    for (std::size_t i = 0; i < m_Workers.size(); ++i)
        if (!::PostQueuedCompletionStatus(m_Port, 0, 0, nullptr)) Fail_DrainTimeout();
    // drain 뒤 join 단계에 별도 제한시간을 둔다. 전체 Stop이 timeout 한 번 안에 끝난다는 뜻은 아니다.
    // 기다림 완료를 먼저 확인한 뒤 std::thread::join으로 C++ 스레드 자원까지 정리한다.
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    for (auto& worker : m_Workers)
    {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now()).count();
        if (::WaitForSingleObject(worker.native_handle(),
            static_cast<DWORD>((std::max)(std::int64_t{0}, static_cast<std::int64_t>(remaining)))) != WAIT_OBJECT_0)
            Fail_DrainTimeout();
        worker.join();
    }
    m_Workers.clear();
    // 워커가 더 이상 포트에 접근하지 않을 때 핸들을 닫는다. 역순이면 대기 중인 워커와 충돌한다.
    ::CloseHandle(m_Port);
    m_Port = nullptr;
    std::scoped_lock lock{m_ParticipantsMutex};
    m_Participants.clear();
}

// accept로 얻은 소켓을 공용 포트에 묶는다. 이후 그 소켓의 overlapped 완료가 이 포트에 들어온다.
// 연결 식별용 completion key는 0을 쓰며 실제 세션은 각 작업의 Owner로 찾는다.
bool LostArk::Server::CIocpService::Associate(const SOCKET socket)
{
    return m_Running.load() && m_Port &&
        ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), m_Port, 0, 0) == m_Port;
}

// 주기 점검 대상 등록. 세션 수명은 소유자/작업이 관리하고 목록은 weak_ptr로만 관찰한다.
// 목록 변경과 워커의 목록 복사를 같은 mutex로 보호한다.
void LostArk::Server::CIocpService::Register(const std::shared_ptr<IIocpParticipant>& participant)
{
    std::scoped_lock lock{m_ParticipantsMutex};
    m_Participants.emplace_back(participant);
}

// 세션이 다음 수신을 준비할 때 호출한다. socket/유효한 owner를 입력받고 접수 결과와 error를 반환한다.
// 4096 bytes는 이번 읽기 버퍼 크기이며 게임 패킷의 최대 길이가 아니다.
// TCP 조각을 완전한 게임 패킷으로 합치는 parser는 완료를 받은 세션이 담당한다.
bool LostArk::Server::CIocpService::Post_Receive(const SOCKET socket,
    const std::shared_ptr<IIocpParticipant>& owner, int& error)
{
    auto operation = std::make_unique<OPERATION>();
    operation->Owner = owner;
    operation->Buffer.buf = reinterpret_cast<char*>(operation->ReceiveBytes.data());
    operation->Buffer.len = static_cast<ULONG>(operation->ReceiveBytes.size());
    return Submit(socket, std::move(operation), error);
}

// 세션의 송신 큐가 선택한 프레임을 보낸다. offset 앞은 이미 완료되어 다시 보내면 안 되는 구간이다.
// const shared_ptr 저장소는 큐에서 프레임을 꺼내더라도 I/O 완료 전 메모리가 없어지지 않게 한다.
// 부분 완료에서 남은 구간을 다시 제출하는 정책은 세션이 담당하고 서비스는 한 번의 전송만 맡는다.
bool LostArk::Server::CIocpService::Post_Send(const SOCKET socket,
    const std::shared_ptr<IIocpParticipant>& owner,
    std::shared_ptr<const std::vector<std::uint8_t>> bytes, const std::size_t offset, int& error)
{
    // bytes가 null, 범위 밖 offset, Windows 길이 타입에 담을 수 없는 요청은 제출 전에 거부한다.
    // owner가 null이 아닌지는 호출자가 보장한다. 아래 검사가 owner까지 검증하는 것은 아니다.
    if (!bytes || offset >= bytes->size() ||
        bytes->size() - offset > (std::numeric_limits<ULONG>::max)())
    { error = WSAEINVAL; return false; }
    auto operation = std::make_unique<OPERATION>();
    operation->Owner = owner;
    operation->Kind = IOCP_OPERATION_KIND::SEND;
    operation->SendBytes = std::move(bytes);
    // Winsock API의 포인터 타입에 맞추기 위한 변환이다. 송신 경로가 원본 bytes 내용을 수정하지 않는다.
    operation->Buffer.buf = reinterpret_cast<char*>(
        const_cast<std::uint8_t*>(operation->SendBytes->data() + offset));
    operation->Buffer.len = static_cast<ULONG>(operation->SendBytes->size() - offset);
    return Submit(socket, std::move(operation), error);
}

// 수신과 송신의 공통 접수 함수. 중요한 순서는 pending 증가 → 소유권 이전 → Windows 호출이다.
// 다른 워커가 매우 빨리 완료를 꺼내더라도 카운터와 작업 수명이 먼저 준비되어 있어야 한다.
bool LostArk::Server::CIocpService::Submit(const SOCKET socket,
    std::unique_ptr<OPERATION> operation, int& error)
{
    error = 0;
    if (!m_Running.load()) { error = WSAESHUTDOWN; return false; }
    // 즉시 실패하면 아래에서 감소시킨다. 정상 접수되면 Worker_Loop가 콜백 이후 감소시킨다.
    // peak 갱신은 CAS 재시도로 다른 워커가 저장한 더 큰 최대값을 덮어쓰지 않게 한다.
    const auto pending = m_Pending.fetch_add(1) + 1;
    auto peak = m_PeakPending.load();
    while (peak < pending && !m_PeakPending.compare_exchange_weak(peak, pending)) {}
    // 이 서비스는 완료 알림 생략 모드를 사용하지 않으므로 즉시 성공도 완료 포트에서 회수한다.
    // Windows 호출 직후 다른 워커가 객체를 파괴할 수 있어 먼저 unique_ptr의 소유권을 놓는다.
    // 접수 성공 후 raw를 다시 읽으면 이미 해제된 객체일 수 있다.
    OPERATION* raw = operation.release();
    DWORD transferred = 0, flags = 0;
    const int result = raw->Kind == IOCP_OPERATION_KIND::RECEIVE ?
        ::WSARecv(socket, &raw->Buffer, 1, &transferred, &flags, raw, nullptr) :
        ::WSASend(socket, &raw->Buffer, 1, &transferred, 0, raw, nullptr);
    if (SOCKET_ERROR == result)
    {
        error = ::WSAGetLastError();
        // WSA_IO_PENDING은 실패가 아니라 정상 비동기 접수다. 이때는 워커가 나중에 정리한다.
        // 그 밖의 오류만 완료 알림이 오지 않는 접수 실패이므로 이 함수가 소유권을 회수한다.
        if (WSA_IO_PENDING != error)
        {
            // 완료 큐로 넘어가지 못했으므로 원래 unique_ptr로 회수하고 pending 감소 전에 파괴한다.
            operation.reset(raw);
            operation.reset();
            Complete_Pending();
            return false;
        }
    }
    error = 0;
    // 작업 완료가 이 증가보다 먼저 관찰될 수도 있다. 실행 중 Posted/Completed의 일시적인
    // 차이만으로 유실을 판정하지 말고 제출 중단과 완료 회수 뒤 최종 값을 비교한다.
    m_Posted.fetch_add(1);
    return true;
}

// 접수 실패 또는 완료 콜백 처리에서 정확히 한 번 호출한다.
// mutex를 잡고 0 전이와 알림을 수행해 Stop의 조건 검사와 실제 대기 사이의 깨움 누락을 막는다.
void LostArk::Server::CIocpService::Complete_Pending() noexcept
{
    std::scoped_lock lock{m_DrainMutex};
    if (m_Pending.fetch_sub(1) == 1) m_Drained.notify_all();
}

// 워커 하나가 실행하는 루프다. 공용 포트에서 완료 한 건을 받아 실제 소유 세션으로 전달한다.
// GQCS의 false만 보고 모두 timeout으로 처리하면 실패한 I/O의 완료 객체를 유실한다.
// overlapped가 있으면 성공 여부와 관계없이 반드시 작업을 회수한다.
void LostArk::Server::CIocpService::Worker_Loop() noexcept
{
    ActiveService = this;
    for (;;)
    {
        DWORD bytes = 0;
        ULONG_PTR key = 0;
        OVERLAPPED* overlapped = nullptr;
        // 최대 100ms 기다린다. 수신이 없어도 깨어나 연결 시간 초과 등의 maintenance를 시도한다.
        const BOOL completed = ::GetQueuedCompletionStatus(m_Port, &bytes, &key, &overlapped, 100);
        const DWORD error = completed ? 0 : ::GetLastError();
        if (overlapped)
        {
            // 제출 때 놓은 소유권을 여기서 단 한 번 회수한다. 블록 종료 때 버퍼/Owner도 해제된다.
            auto operation = std::unique_ptr<OPERATION>(static_cast<OPERATION*>(overlapped));
            // 완료 수는 오류도 포함한다. 따라서 이것을 정상 처리한 게임 패킷 수로 해석하면 안 된다.
            if (operation->Kind == IOCP_OPERATION_KIND::RECEIVE)
            {
                m_ReceiveCompletions.fetch_add(1);
                m_ReceivedBytes.fetch_add(bytes);
            }
            else
            {
                m_SendCompletions.fetch_add(1);
                m_SentBytes.fetch_add(bytes);
                if (!error && bytes && bytes < operation->Buffer.len) m_PartialSends.fetch_add(1);
            }
            // 수신 span은 현재 작업 메모리를 빌린다. 안전하게 버퍼 길이 안으로 제한하며
            // 송신 콜백에는 빈 span과 실제 완료 bytes만 전달한다.
            const auto received = operation->Kind == IOCP_OPERATION_KIND::RECEIVE ?
                std::span<const std::uint8_t>{operation->ReceiveBytes.data(),
                    (std::min)(static_cast<std::size_t>(bytes), operation->ReceiveBytes.size())} :
                std::span<const std::uint8_t>{};
            // 서비스 mutex 없이 호출한다. 세션은 수신 조립 또는 송신 offset 갱신을 수행한다.
            // 콜백에서 다음 수신을 제출할 수 있으므로 그 다음 작업의 pending이 먼저 증가할 수 있다.
            // 콜백이 끝나기 전에 현재 pending을 줄이면 Stop이 너무 일찍 끝날 수 있다.
            operation->Owner->On_IocpCompleted(operation->Kind, received, bytes, error);
            m_Completed.fetch_add(1);
            Complete_Pending();
        }
        // Stop이 넣은 성공/null 패킷이면 종료한다. false/null/WAIT_TIMEOUT은 단순 대기 만료다.
        // false/non-null 오류 완료는 위에서 이미 회수했고, 그 외 포트 오류는 안전하게 종료한다.
        else if (completed && !m_Running.load()) break;
        else if (!completed && error != WAIT_TIMEOUT) Fail_DrainTimeout();
        Run_Maintenance();
    }
    ActiveService = nullptr;
}

// 여러 워커 중 CAS에 성공한 하나가 그 100ms 차례의 점검을 맡는다.
// 목록을 잠근 채 콜백을 부르지 않는다. 약한 참조를 강한 참조 목록으로 바꾼 뒤 잠금을 푼다.
// 이것은 이번 호출 동안만 세션을 보존하며 등록 목록이 세션을 영구 소유하게 만들지 않는다.
// 긴 콜백은 다음 점검과 겹칠 수 있고 완료 콜백과도 병렬일 수 있다. 세션에서 동기화해야 한다.
void LostArk::Server::CIocpService::Run_Maintenance() noexcept
{
    const auto now = ::GetTickCount64();
    auto next = m_NextMaintenance.load();
    if (now < next || !m_NextMaintenance.compare_exchange_strong(next, now + 100)) return;
    try
    {
        std::vector<std::shared_ptr<IIocpParticipant>> participants;
        {
            std::scoped_lock lock{m_ParticipantsMutex};
            std::erase_if(m_Participants, [](const auto& p) { return p.expired(); });
            for (const auto& weak : m_Participants)
                if (auto participant = weak.lock()) participants.push_back(std::move(participant));
        }
        for (const auto& participant : participants) participant->On_IocpMaintenance();
    }
    catch (...) { Fail_DrainTimeout(); }
}

// 아래 조회는 게임 로직을 바꾸지 않는다. Get_WorkerCount/Get_Metrics는 Start/Stop과 동시 호출하지 않는다.
// 원자 카운터 각각의 읽기는 안전하지만 서로 다른 시각의 값이 섞일 수 있는 근사 관측이다.
// 카운터는 Start 때 초기화하지 않으므로 같은 객체를 재시작하면 누적되고 새 실험은 새 객체를 사용한다.
bool LostArk::Server::CIocpService::Is_WorkerThread() const noexcept { return ActiveService == this; }
std::size_t LostArk::Server::CIocpService::Get_WorkerCount() const noexcept { return m_Workers.size(); }
LostArk::Server::IOCP_SERVICE_METRICS LostArk::Server::CIocpService::Get_Metrics() const noexcept
{
    return { m_Workers.size(), m_Posted.load(), m_Completed.load(), m_Pending.load(), m_PeakPending.load(),
        m_ReceiveCompletions.load(), m_SendCompletions.load(), m_PartialSends.load(),
        m_ReceivedBytes.load(), m_SentBytes.load() };
}
~~~~

<a id="file-server-private-main-cpp"></a>

### Server/Private/Main.cpp

파일: C:/Users/tnest/Desktop/LostArk/Server/Private/Main.cpp

서버 인자에서 transport·worker·snapshot executor를 검증해 Run에 전달한다. 기존 실행 인자는 보존한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~cpp
#include "ServerApp.h"
#include "ServerGameplayContractTests.h"
#include "ServerGameplayContractTests_Runner.h"

#include <charconv>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

int main(const int argumentCount, char** arguments)
{
#ifdef _DEBUG
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
	if (2 == argumentCount &&
		std::string_view(arguments[1]) == "--contract-test")
	{
		return LostArk::Server::Run_ServerGameplayContractTests();
	}
	if (2 == argumentCount && std::string_view(arguments[1]) == "--character-admission-contract-test")
        return LostArk::Server::CServerGameplayContractRunner::Run_CharacterAdmissionOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--maharaka-ai-contract-test")
        return LostArk::Server::CServerGameplayContractRunner::Run_MaharakaAI();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--colosseum-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ColosseumOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--guide-ai-contract-test")
        return LostArk::Server::CServerGameplayContractRunner::Run_GuideAI();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--valtan-pattern-control-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ValtanPatternControl();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--battle-items-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_BattleItemsOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--character-state-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_CharacterStateOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--room-ping-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_RoomPing();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-draft-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_KoukuDraft();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-product-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_KoukuProductOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--valtan-presentation-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ValtanPresentationOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--numeric-balance-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_NumericBalanceOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--valtan-lifecycle-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ValtanLifecycleOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--colosseum-match-contract-test")
        return LostArk::Server::CServerGameplayContractRunner::Run_ColosseumMatch();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--colosseum-combat-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ColosseumCombat();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--skill-stages-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_SkillStagesOnly();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-dice-hit-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_KoukuDiceDamageContracts();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-raid-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_KoukuRaid();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--npc-raid-return-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_NpcRaidReturn();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-bundle-contract-test")
		return LostArk::Server::Run_ServerGameplayContractTests(false, false, true);
	if (2 == argumentCount && std::string_view(arguments[1]) == "--card-maze-contract-test")
		return LostArk::Server::Run_ServerCardMazeContractTests();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--bingo-contract-test")
		return LostArk::Server::Run_ServerBingoContractTests();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--vehicle-riding-contract-test")
		return LostArk::Server::Run_ServerVehicleRidingContractTests();
	if (2 == argumentCount &&
		std::string_view(arguments[1]) == "--debug-teleport-contract-test")
	{
		return LostArk::Server::Run_ServerGameplayContractTests(false, true);
	}
	if (2 == argumentCount && std::string_view(arguments[1]) == "--world-playback-contract-test")
	{
		return LostArk::Server::Run_ServerGameplayContractTests(false, false, false, true);
	}
	if (2 == argumentCount &&
		std::string_view(arguments[1]) == "--kouku-object-overlap-contract-test")
	{
		return LostArk::Server::Run_ServerKoukuObjectOverlapContractTests();
	}
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-showtime-bomb-contract-test")
		return LostArk::Server::CServerGameplayContractRunner::Run_ShowtimeBombs();
	if (2 == argumentCount && std::string_view(arguments[1]) == "--kouku-support-surface-contract-test")
		return LostArk::Server::Run_ServerKoukuSupportSurfaceContractTests();
	if (2 == argumentCount &&
		std::string_view(arguments[1]) == "--valtan-arena-support-contract-test")
	{
		return LostArk::Server::CServerGameplayContractRunner::Run_ValtanArenaSupport();
	}
	if (2 == argumentCount &&
		std::string_view(arguments[1]) == "--navigation-contract-test")
	{
		return LostArk::Server::Run_ServerNavigationContractTests();
	}
	if (2 == argumentCount &&
		std::string_view(arguments[1]) ==
			"--dimensionmaster-ground-target-contract")
	{
		return LostArk::Server::Run_ServerGameplayContractTests(true);
	}
	if (2 == argumentCount &&
		std::string_view(arguments[1]) ==
			"--reset-valtan-runtime-to-packaged")
	{
		return LostArk::Server::CServerApp::Reset_ValtanRuntimeToPackaged();
	}
	std::uint32_t automaticShutdownMilliseconds = 0;
	std::uint32_t serverPort = 7777u;
	std::string bindAddress = "0.0.0.0";
	bool hasSmokeTimeout = false;
	bool hasBindAddress = false;
	bool headless = false;
	bool hasPort = false;
	LostArk::Server::SERVER_CONCURRENCY_OPTIONS concurrency;
	bool hasTransport = false, hasExecutor = false, hasIocpWorkers = false, hasJobWorkers = false;
	for (int index = 1; index < argumentCount; ++index)
	{
		const std::string_view argument(arguments[index]);
		if ("--transport" == argument || "--snapshot-executor" == argument ||
			"--iocp-workers" == argument || "--job-workers" == argument)
		{
			bool& seen = argument == "--transport" ? hasTransport : argument == "--snapshot-executor" ? hasExecutor : argument == "--iocp-workers" ? hasIocpWorkers : hasJobWorkers;
			if (seen || index + 1 >= argumentCount) { std::cerr << "Duplicate or missing concurrency argument.\n"; return 2; }
			seen = true;
			const std::string_view value(arguments[++index]);
			if (argument == "--transport")
			{
				if (value != "select" && value != "iocp") return 2;
				concurrency.Transport = value == "iocp" ? LostArk::Server::SESSION_TRANSPORT_BACKEND::IOCP : LostArk::Server::SESSION_TRANSPORT_BACKEND::SELECT_THREADS;
			}
			else if (argument == "--snapshot-executor")
			{
				if (value != "serial" && value != "chaselev") return 2;
				concurrency.SnapshotJobs = value == "chaselev";
			}
			else
			{
				auto& workers = argument == "--iocp-workers" ? concurrency.IocpWorkers : concurrency.JobWorkers;
				const auto parsed = std::from_chars(value.data(), value.data()+value.size(), workers);
				if (parsed.ec != std::errc{} || parsed.ptr != value.data()+value.size() || workers < 1 || workers > 16) return 2;
			}
			continue;
		}
		if ("--smoke-timeout-ms" == argument)
		{
			if (hasSmokeTimeout || index + 1 >= argumentCount)
			{
				std::cerr << "--smoke-timeout-ms requires one value.\n";
				return 2;
			}
			const std::string_view value(arguments[++index]);
			const auto result = std::from_chars(
				value.data(), value.data() + value.size(),
				automaticShutdownMilliseconds);
			if (result.ec != std::errc{} ||
				result.ptr != value.data() + value.size() ||
				automaticShutdownMilliseconds < 100u ||
				automaticShutdownMilliseconds > 60000u)
			{
				std::cerr << "Smoke timeout must be an integer from 100 to 60000.\n";
				return 2;
			}
			hasSmokeTimeout = true;
			continue;
		}
		if ("--bind-address" == argument)
		{
			if (hasBindAddress || index + 1 >= argumentCount)
			{
				std::cerr << "--bind-address requires one IPv4 value.\n";
				return 2;
			}
			bindAddress = arguments[++index];
			if (bindAddress.empty() || bindAddress.size() > 63u)
			{
				std::cerr << "Bind address is invalid.\n";
				return 2;
			}
			hasBindAddress = true;
			continue;
		}
		if ("--headless" == argument)
		{
			if (headless)
			{
				std::cerr << "--headless may be specified only once.\n";
				return 2;
			}
			headless = true;
			continue;
		}
		if ("--port" == argument)
		{
			if (hasPort || index + 1 >= argumentCount)
			{
				std::cerr << "--port requires one value.\n";
				return 2;
			}
			const std::string_view value(arguments[++index]);
			const auto result = std::from_chars(
				value.data(), value.data() + value.size(), serverPort);
			if (result.ec != std::errc{} ||
				result.ptr != value.data() + value.size() ||
				0u == serverPort || serverPort > 65535u)
			{
				std::cerr << "Port must be an integer from 1 to 65535.\n";
				return 2;
			}
			hasPort = true;
			continue;
		}

		std::cerr << "Usage: Server [--contract-test | "
			"--kouku-object-overlap-contract-test | --kouku-support-surface-contract-test | --kouku-showtime-bomb-contract-test | "
			"--kouku-bundle-contract-test | --card-maze-contract-test | "
			"--kouku-raid-contract-test | --kouku-dice-hit-contract-test | --kouku-draft-contract-test | --kouku-product-contract-test | "
			"--valtan-lifecycle-contract-test | --valtan-presentation-contract-test | --skill-stages-contract-test | --colosseum-combat-contract-test | "
			"--bingo-contract-test | --vehicle-riding-contract-test | "
			"--navigation-contract-test | --valtan-arena-support-contract-test | --maharaka-ai-contract-test | "
			"--debug-teleport-contract-test | "
			"--dimensionmaster-ground-target-contract | "
			"--reset-valtan-runtime-to-packaged | "
			"--bind-address IPv4] [--port 1..65535] "
			"[--smoke-timeout-ms 100..60000] "
			"[--transport select|iocp] [--iocp-workers 1..16] "
			"[--snapshot-executor serial|chaselev] [--job-workers 1..16]\n";
		return 2;
	}
	LostArk::Server::CServerApp serverApp;
	return serverApp.Run(
		automaticShutdownMilliseconds,
		bindAddress,
		static_cast<std::uint16_t>(serverPort),
		headless, concurrency);
}
~~~~

<a id="file-server-private-serverapp-cpp"></a>

### Server/Private/ServerApp.cpp

파일: C:/Users/tnest/Desktop/LostArk/Server/Private/ServerApp.cpp

모든 종류의 room과 session에 선택을 전달한다. 종료 때 producer/room thread→session cancel/drain→simulation 해제→IOCP service/job pool 종료 순서로 처리한다. 실제 게임 상태의 single writer 계약을 유지한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~cpp
#include "ServerApp.h"
#include "Concurrency/WorkStealingJobSystem.h"

#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <WinSock2.h>
#include <bcrypt.h>

#include <cstdio>
#include <io.h>

#include <chrono>
#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <span>
#include <string>
#include <utility>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	using LostArk::Shared::GameplayDataRevision;

	std::uint64_t Current_UnixMilliseconds()
	{
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count());
	}

	std::string Escape_JsonString(const std::string_view value)
	{
		std::string escaped;
		escaped.reserve(value.size());
		constexpr char HEX[] = "0123456789abcdef";
		for (const unsigned char character : value)
		{
			switch (character)
			{
			case '"': escaped += "\\\""; break;
			case '\\': escaped += "\\\\"; break;
			case '\b': escaped += "\\b"; break;
			case '\f': escaped += "\\f"; break;
			case '\n': escaped += "\\n"; break;
			case '\r': escaped += "\\r"; break;
			case '\t': escaped += "\\t"; break;
			default:
				if (character < 0x20u)
				{
					escaped += "\\u00";
					escaped.push_back(HEX[(character >> 4u) & 0x0fu]);
					escaped.push_back(HEX[character & 0x0fu]);
				}
				else
					escaped.push_back(static_cast<char>(character));
				break;
			}
		}
		return escaped;
	}

	std::filesystem::path Resolve_ServerSessionDiagnosticPath()
	{
		std::array<wchar_t, 32768u> modulePath{};
		const DWORD length = ::GetModuleFileNameW(
			nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size()));
		if (0u == length || length >= modulePath.size())
			return {};

		std::error_code error;
		const std::filesystem::path diagnosticDirectory =
			std::filesystem::path{ modulePath.data() }.parent_path() /
			L"Diagnostics";
		std::filesystem::create_directories(diagnosticDirectory, error);
		if (error)
			return {};
		return diagnosticDirectory /
			(L"server-session-" + std::to_wstring(::GetCurrentProcessId()) +
				L".jsonl");
	}

	// A process keeps at most two 4 MiB performance files beside its session diagnostics.
	bool Append_BoundedRoomPerformanceDiagnostic(const std::filesystem::path& path,
		const std::string_view line) noexcept
	{
		constexpr std::uintmax_t MAX_FILE_BYTES = 4u * 1024u * 1024u;
		try
		{
			if (path.empty() || line.size() >= MAX_FILE_BYTES) return false;
			std::error_code error;
			const bool exists = std::filesystem::exists(path, error);
			if (error) return false;
			const auto currentBytes = exists ? std::filesystem::file_size(path, error) : 0u;
			if (error) return false;
			if (currentBytes > MAX_FILE_BYTES - (line.size() + 1u))
			{
				auto previousPath = path;
				previousPath.replace_extension(L".previous.log");
				std::filesystem::remove(previousPath, error);
				if (error) return false;
				std::filesystem::rename(path, previousPath, error);
				if (error) return false;
			}
			std::ofstream log{ path, std::ios::binary | std::ios::app };
			if (!log) return false;
			log << line << '\n';
			log.flush();
			return static_cast<bool>(log);
		}
		catch (...)
		{
			return false; // Diagnostic I/O must never fail the simulation.
		}
	}

	std::string Build_ServerSessionDiagnosticJson(
		const LostArk::Server::SESSION_ID sessionId,
		const LostArk::Server::CLIENT_SESSION_PEER_ENDPOINT& peer,
		const LostArk::Server::CLIENT_SESSION_CLOSE_DIAGNOSTIC& diagnostic,
		const LostArk::Shared::WORLD_ID worldId,
		const LostArk::Shared::PLAYER_ID playerId,
		const bool wasBound,
		const bool leaveEnqueued,
		const LostArk::Server::CLIENT_SESSION_OUTBOUND_METRICS& metrics)
	{
		const std::uint64_t occurredAt = 0u ==
			diagnostic.iOccurredUnixMilliseconds ?
			Current_UnixMilliseconds() :
			diagnostic.iOccurredUnixMilliseconds;
		std::ostringstream json;
		json << "{\"schema\":\"lostark.server-session-diagnostic\""
			<< ",\"formatVersion\":1"
			<< ",\"event\":\"connection.closed\""
			<< ",\"processId\":" << ::GetCurrentProcessId()
#ifdef _DEBUG
			<< ",\"buildConfig\":\"Debug\""
#else
			<< ",\"buildConfig\":\"Release\""
#endif
			<< ",\"networkProtocolVersion\":" <<
				LostArk::Shared::NETWORK_PROTOCOL_VERSION
			<< ",\"occurredUnixMs\":" << occurredAt
			<< ",\"sessionId\":" << sessionId
			<< ",\"peerAddress\":\"" << Escape_JsonString(peer.strAddress)
			<< "\",\"peerPort\":" << peer.iPort
			<< ",\"reason\":\"" <<
				LostArk::Shared::To_SessionDiagnosticReasonName(
					diagnostic.eReason) << '"'
			<< ",\"nativeErrorCode\":" << diagnostic.iNativeErrorCode
			<< ",\"context\":\"" <<
				Escape_JsonString(diagnostic.strContext) << '"'
			<< ",\"lastInboundPacket\":" <<
				static_cast<std::uint16_t>(diagnostic.eLastInboundPacket)
			<< ",\"lastInboundUnixMs\":" <<
				diagnostic.iLastInboundUnixMilliseconds
			<< ",\"lastInboundAgeMs\":" <<
				(0u != diagnostic.iLastInboundUnixMilliseconds &&
					occurredAt >= diagnostic.iLastInboundUnixMilliseconds ?
					occurredAt - diagnostic.iLastInboundUnixMilliseconds : 0u)
			<< ",\"worldId\":" << static_cast<std::uint16_t>(worldId)
			<< ",\"playerId\":" << playerId
			<< ",\"wasBound\":" << (wasBound ? "true" : "false")
			<< ",\"leaveEnqueued\":" <<
				(leaveEnqueued ? "true" : "false")
			<< ",\"outbound\":{"
			<< "\"queuedFramesAtClose\":" <<
				diagnostic.iQueuedFrameCountAtClose
			<< ",\"queuedBytesAtClose\":" <<
				diagnostic.iQueuedByteCountAtClose
			<< ",\"queuedFramesAfterClose\":" <<
				metrics.iCurrentQueuedFrameCount
			<< ",\"queuedBytesAfterClose\":" <<
				metrics.iCurrentQueuedByteCount
			<< ",\"frameHighWatermark\":" <<
				metrics.iQueuedFrameHighWatermark
			<< ",\"byteHighWatermark\":" <<
				metrics.iQueuedByteHighWatermark
			<< ",\"reliableEnqueued\":" <<
				metrics.iReliableEnqueuedFrameCount
			<< ",\"reliableRejected\":" <<
				metrics.iReliableRejectedFrameCount
			<< ",\"snapshotEnqueued\":" <<
				metrics.iSnapshotEnqueuedFrameCount
			<< ",\"snapshotCoalesced\":" <<
				metrics.iSnapshotCoalescedFrameCount
			<< ",\"snapshotDropped\":" <<
				metrics.iSnapshotDroppedFrameCount
			<< ",\"sentFrames\":" << metrics.iSentFrameCount
			<< ",\"sentBytes\":" << metrics.iSentByteCount
			<< ",\"sendFailures\":" << metrics.iSendFailureCount
			<< ",\"lastSendUs\":" <<
				metrics.iLastFrameSendMicroseconds
			<< ",\"maxSendUs\":" <<
				metrics.iMaximumFrameSendMicroseconds
			<< "}}";
		return json.str();
	}

	enum class JSON_KIND : std::uint8_t
	{
		OBJECT, ARRAY, STRING, INTEGER, BOOLEAN, NULL_VALUE
	};

	struct JSON_VALUE final
	{
		JSON_KIND eKind = JSON_KIND::NULL_VALUE;
		std::string String;
		std::uint64_t Integer = 0u;
		bool isNegative = false;
		bool Boolean = false;
		std::vector<std::string> MemberNames;
		std::vector<JSON_VALUE> MemberValues;
		std::vector<JSON_VALUE> Elements;
	};

	bool Is_ValidUtf8(const std::string_view value)
	{
		for (std::size_t index = 0u; index < value.size();)
		{
			const unsigned char lead =
				static_cast<unsigned char>(value[index]);
			if (lead < 0x80u)
			{
				++index;
				continue;
			}
			std::size_t count = 0u;
			std::uint32_t codePoint = 0u;
			if (0xc2u <= lead && lead <= 0xdfu)
			{
				count = 2u; codePoint = lead & 0x1fu;
			}
			else if (0xe0u <= lead && lead <= 0xefu)
			{
				count = 3u; codePoint = lead & 0x0fu;
			}
			else if (0xf0u <= lead && lead <= 0xf4u)
			{
				count = 4u; codePoint = lead & 0x07u;
			}
			else
				return false;
			if (index + count > value.size())
				return false;
			for (std::size_t offset = 1u; offset < count; ++offset)
			{
				const unsigned char next =
					static_cast<unsigned char>(value[index + offset]);
				if (0x80u != (next & 0xc0u))
					return false;
				codePoint = (codePoint << 6u) | (next & 0x3fu);
			}
			if ((2u == count && codePoint < 0x80u) ||
				(3u == count && codePoint < 0x800u) ||
				(4u == count && codePoint < 0x10000u) ||
				(0xd800u <= codePoint && codePoint <= 0xdfffu) ||
				codePoint > 0x10ffffu)
				return false;
			index += count;
		}
		return true;
	}

	void Append_Utf8(const std::uint32_t codePoint, std::string& value)
	{
		if (codePoint <= 0x7fu)
			value.push_back(static_cast<char>(codePoint));
		else if (codePoint <= 0x7ffu)
		{
			value.push_back(static_cast<char>(0xc0u | (codePoint >> 6u)));
			value.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
		else if (codePoint <= 0xffffu)
		{
			value.push_back(static_cast<char>(0xe0u | (codePoint >> 12u)));
			value.push_back(static_cast<char>(
				0x80u | ((codePoint >> 6u) & 0x3fu)));
			value.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
		else
		{
			value.push_back(static_cast<char>(0xf0u | (codePoint >> 18u)));
			value.push_back(static_cast<char>(
				0x80u | ((codePoint >> 12u) & 0x3fu)));
			value.push_back(static_cast<char>(
				0x80u | ((codePoint >> 6u) & 0x3fu)));
			value.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
	}

	class CBoundedJsonParser final
	{
	public:
		explicit CBoundedJsonParser(const std::string_view source)
			: m_Source(source) {}

		bool Parse(JSON_VALUE& value, std::string& status)
		{
			if (m_Source.size() >= 3u &&
				static_cast<unsigned char>(m_Source[0]) == 0xefu &&
				static_cast<unsigned char>(m_Source[1]) == 0xbbu &&
				static_cast<unsigned char>(m_Source[2]) == 0xbfu)
			{
				status = "UTF-8 BOM is forbidden in candidate JSON";
				return false;
			}
			Skip_Whitespace();
			if (!Parse_Value(value, 0u, status))
				return false;
			Skip_Whitespace();
			if (m_iCursor != m_Source.size())
			{
				status = "Candidate JSON has trailing bytes";
				return false;
			}
			return true;
		}

	private:
		static int Hex_Nibble(const char value)
		{
			if ('0' <= value && value <= '9') return value - '0';
			if ('a' <= value && value <= 'f') return value - 'a' + 10;
			if ('A' <= value && value <= 'F') return value - 'A' + 10;
			return -1;
		}

		void Skip_Whitespace()
		{
			while (m_iCursor < m_Source.size())
			{
				const char value = m_Source[m_iCursor];
				if (' ' != value && '\t' != value && '\r' != value &&
					'\n' != value)
					break;
				++m_iCursor;
			}
		}

		bool Parse_Value(
			JSON_VALUE& value, const std::size_t depth, std::string& status)
		{
			if (depth > 64u || ++m_iNodeCount > 200000u ||
				m_iCursor >= m_Source.size())
			{
				status = "Candidate JSON exceeds structural limits";
				return false;
			}
			const char lead = m_Source[m_iCursor];
			if ('{' == lead) return Parse_Object(value, depth, status);
			if ('[' == lead) return Parse_Array(value, depth, status);
			if ('\"' == lead)
			{
				value.eKind = JSON_KIND::STRING;
				return Parse_String(value.String, status);
			}
			if ('-' == lead || ('0' <= lead && lead <= '9'))
				return Parse_Integer(value, status);
			if (m_Source.substr(m_iCursor, 4u) == "true")
			{
				m_iCursor += 4u; value.eKind = JSON_KIND::BOOLEAN;
				value.Boolean = true; return true;
			}
			if (m_Source.substr(m_iCursor, 5u) == "false")
			{
				m_iCursor += 5u; value.eKind = JSON_KIND::BOOLEAN;
				value.Boolean = false; return true;
			}
			if (m_Source.substr(m_iCursor, 4u) == "null")
			{
				m_iCursor += 4u; value.eKind = JSON_KIND::NULL_VALUE;
				return true;
			}
			status = "Candidate JSON token is invalid";
			return false;
		}

		bool Parse_Object(
			JSON_VALUE& value, const std::size_t depth, std::string& status)
		{
			value.eKind = JSON_KIND::OBJECT;
			++m_iCursor;
			Skip_Whitespace();
			if (m_iCursor < m_Source.size() && '}' == m_Source[m_iCursor])
			{
				++m_iCursor; return true;
			}
			while (m_iCursor < m_Source.size())
			{
				std::string name;
				if (!Parse_String(name, status)) return false;
				if (std::find(value.MemberNames.begin(), value.MemberNames.end(),
					name) != value.MemberNames.end())
				{
					status = "Candidate JSON contains a duplicate object member";
					return false;
				}
				Skip_Whitespace();
				if (m_iCursor >= m_Source.size() ||
					':' != m_Source[m_iCursor++])
				{
					status = "Candidate JSON object separator is invalid";
					return false;
				}
				Skip_Whitespace();
				JSON_VALUE member;
				if (!Parse_Value(member, depth + 1u, status)) return false;
				value.MemberNames.push_back(std::move(name));
				value.MemberValues.push_back(std::move(member));
				Skip_Whitespace();
				if (m_iCursor >= m_Source.size()) break;
				if ('}' == m_Source[m_iCursor])
				{
					++m_iCursor; return true;
				}
				if (',' != m_Source[m_iCursor++]) break;
				Skip_Whitespace();
			}
			status = "Candidate JSON object is unterminated";
			return false;
		}

		bool Parse_Array(
			JSON_VALUE& value, const std::size_t depth, std::string& status)
		{
			value.eKind = JSON_KIND::ARRAY;
			++m_iCursor;
			Skip_Whitespace();
			if (m_iCursor < m_Source.size() && ']' == m_Source[m_iCursor])
			{
				++m_iCursor; return true;
			}
			while (m_iCursor < m_Source.size())
			{
				JSON_VALUE element;
				if (!Parse_Value(element, depth + 1u, status)) return false;
				value.Elements.push_back(std::move(element));
				Skip_Whitespace();
				if (m_iCursor >= m_Source.size()) break;
				if (']' == m_Source[m_iCursor])
				{
					++m_iCursor; return true;
				}
				if (',' != m_Source[m_iCursor++]) break;
				Skip_Whitespace();
			}
			status = "Candidate JSON array is unterminated";
			return false;
		}

		bool Parse_String(std::string& value, std::string& status)
		{
			if (m_iCursor >= m_Source.size() ||
				'\"' != m_Source[m_iCursor++])
			{
				status = "Candidate JSON string is invalid";
				return false;
			}
			value.clear();
			while (m_iCursor < m_Source.size())
			{
				const unsigned char raw =
					static_cast<unsigned char>(m_Source[m_iCursor++]);
				if ('\"' == raw)
				{
					if (value.size() > 1024u * 1024u || !Is_ValidUtf8(value))
					{
						status = "Candidate JSON string exceeds UTF-8 limits";
						return false;
					}
					return true;
				}
				if ('\\' != raw)
				{
					if (raw < 0x20u)
					{
						status = "Candidate JSON string contains a control byte";
						return false;
					}
					value.push_back(static_cast<char>(raw));
					continue;
				}
				if (m_iCursor >= m_Source.size()) break;
				const char escape = m_Source[m_iCursor++];
				switch (escape)
				{
				case '\"': value.push_back('\"'); break;
				case '\\': value.push_back('\\'); break;
				case '/': value.push_back('/'); break;
				case 'b': value.push_back('\b'); break;
				case 'f': value.push_back('\f'); break;
				case 'n': value.push_back('\n'); break;
				case 'r': value.push_back('\r'); break;
				case 't': value.push_back('\t'); break;
				case 'u':
				{
					if (m_iCursor + 4u > m_Source.size())
					{
						status = "Candidate JSON unicode escape is incomplete";
						return false;
					}
					std::uint32_t codePoint = 0u;
					for (std::size_t index = 0u; index < 4u; ++index)
					{
						const int nibble = Hex_Nibble(m_Source[m_iCursor + index]);
						if (nibble < 0)
						{
							status = "Candidate JSON unicode escape is invalid";
							return false;
						}
						codePoint = (codePoint << 4u) |
							static_cast<std::uint32_t>(nibble);
					}
					m_iCursor += 4u;
					if (0xd800u <= codePoint && codePoint <= 0xdbffu)
					{
						if (m_iCursor + 6u > m_Source.size() ||
							'\\' != m_Source[m_iCursor] ||
							'u' != m_Source[m_iCursor + 1u])
						{
							status = "Candidate JSON surrogate pair is incomplete";
							return false;
						}
						std::uint32_t low = 0u;
						for (std::size_t index = 0u; index < 4u; ++index)
						{
							const int nibble = Hex_Nibble(
								m_Source[m_iCursor + 2u + index]);
							if (nibble < 0)
							{
								status = "Candidate JSON surrogate escape is invalid";
								return false;
							}
							low = (low << 4u) |
								static_cast<std::uint32_t>(nibble);
						}
						if (low < 0xdc00u || low > 0xdfffu)
						{
							status = "Candidate JSON low surrogate is invalid";
							return false;
						}
						m_iCursor += 6u;
						codePoint = 0x10000u +
							((codePoint - 0xd800u) << 10u) + (low - 0xdc00u);
					}
					else if (0xdc00u <= codePoint && codePoint <= 0xdfffu)
					{
						status = "Candidate JSON has an unpaired low surrogate";
						return false;
					}
					Append_Utf8(codePoint, value);
					break;
				}
				default:
					status = "Candidate JSON escape is invalid";
					return false;
				}
			}
			status = "Candidate JSON string is unterminated";
			return false;
		}

		bool Parse_Integer(JSON_VALUE& value, std::string& status)
		{
			value.isNegative = '-' == m_Source[m_iCursor];
			if (value.isNegative) ++m_iCursor;
			const std::size_t digits = m_iCursor;
			if (m_iCursor >= m_Source.size() ||
				m_Source[m_iCursor] < '0' || m_Source[m_iCursor] > '9')
			{
				status = "Candidate JSON integer is invalid";
				return false;
			}
			if ('0' == m_Source[m_iCursor] && m_iCursor + 1u < m_Source.size() &&
				'0' <= m_Source[m_iCursor + 1u] &&
				m_Source[m_iCursor + 1u] <= '9')
			{
				status = "Candidate JSON integer has a leading zero";
				return false;
			}
			while (m_iCursor < m_Source.size() &&
				'0' <= m_Source[m_iCursor] && m_Source[m_iCursor] <= '9')
				++m_iCursor;
			if (m_iCursor < m_Source.size() &&
				('.' == m_Source[m_iCursor] || 'e' == m_Source[m_iCursor] ||
					'E' == m_Source[m_iCursor]))
			{
				status = "Candidate manifest forbids floating-point numbers";
				return false;
			}
			const char* first = m_Source.data() + digits;
			const char* last = m_Source.data() + m_iCursor;
			const auto parsed = std::from_chars(first, last, value.Integer);
			if (parsed.ec != std::errc{} || parsed.ptr != last ||
				(value.isNegative && 0u == value.Integer))
			{
				status = "Candidate JSON integer is out of range";
				return false;
			}
			value.eKind = JSON_KIND::INTEGER;
			return true;
		}

		std::string_view m_Source;
		std::size_t m_iCursor = 0u;
		std::size_t m_iNodeCount = 0u;
	};

	const JSON_VALUE* Find_Member(
		const JSON_VALUE& object, const std::string_view name)
	{
		if (JSON_KIND::OBJECT != object.eKind) return nullptr;
		for (std::size_t index = 0u; index < object.MemberNames.size(); ++index)
			if (object.MemberNames[index] == name)
				return &object.MemberValues[index];
		return nullptr;
	}

	JSON_VALUE* Find_Member(JSON_VALUE& object, const std::string_view name)
	{
		return const_cast<JSON_VALUE*>(Find_Member(
			static_cast<const JSON_VALUE&>(object), name));
	}

	bool Has_ExactMembers(
		const JSON_VALUE& object,
		const std::initializer_list<std::string_view> expected)
	{
		if (JSON_KIND::OBJECT != object.eKind ||
			object.MemberNames.size() != expected.size())
			return false;
		return std::all_of(expected.begin(), expected.end(),
			[&object](const std::string_view name)
			{
				return nullptr != Find_Member(object, name);
			});
	}

	bool Is_String(const JSON_VALUE* value, const std::string_view expected)
	{
		return nullptr != value && JSON_KIND::STRING == value->eKind &&
			value->String == expected;
	}

	bool Is_Integer(
		const JSON_VALUE* value, const std::uint64_t minimum,
		const std::uint64_t maximum)
	{
		return nullptr != value && JSON_KIND::INTEGER == value->eKind &&
			!value->isNegative && minimum <= value->Integer &&
			value->Integer <= maximum;
	}

	bool Is_LowerSha256(const JSON_VALUE* value)
	{
		return nullptr != value && JSON_KIND::STRING == value->eKind &&
			value->String.size() == LostArk::Shared::GAMEPLAY_DATA_REVISION_HEX_BYTES &&
			std::all_of(value->String.begin(), value->String.end(),
				[](const char character)
				{
					return ('0' <= character && character <= '9') ||
						('a' <= character && character <= 'f');
				});
	}

	bool Json_Equals(const JSON_VALUE& left, const JSON_VALUE& right)
	{
		if (left.eKind != right.eKind) return false;
		switch (left.eKind)
		{
		case JSON_KIND::STRING:
			return left.String == right.String;
		case JSON_KIND::INTEGER:
			return left.Integer == right.Integer &&
				left.isNegative == right.isNegative;
		case JSON_KIND::BOOLEAN:
			return left.Boolean == right.Boolean;
		case JSON_KIND::NULL_VALUE:
			return true;
		case JSON_KIND::ARRAY:
			if (left.Elements.size() != right.Elements.size()) return false;
			for (std::size_t index = 0u; index < left.Elements.size(); ++index)
				if (!Json_Equals(left.Elements[index], right.Elements[index]))
					return false;
			return true;
		case JSON_KIND::OBJECT:
			if (left.MemberNames.size() != right.MemberNames.size()) return false;
			for (std::size_t index = 0u; index < left.MemberNames.size(); ++index)
			{
				const JSON_VALUE* other =
					Find_Member(right, left.MemberNames[index]);
				if (nullptr == other ||
					!Json_Equals(left.MemberValues[index], *other))
					return false;
			}
			return true;
		}
		return false;
	}

	void Append_CanonicalString(const std::string& value, std::string& output)
	{
		static constexpr char HEX[] = "0123456789abcdef";
		output.push_back('\"');
		for (const unsigned char character : value)
		{
			switch (character)
			{
			case '\"': output += "\\\""; break;
			case '\\': output += "\\\\"; break;
			case '\b': output += "\\b"; break;
			case '\f': output += "\\f"; break;
			case '\n': output += "\\n"; break;
			case '\r': output += "\\r"; break;
			case '\t': output += "\\t"; break;
			default:
				if (character < 0x20u)
				{
					output += "\\u00";
					output.push_back(HEX[character >> 4u]);
					output.push_back(HEX[character & 0x0fu]);
				}
				else output.push_back(static_cast<char>(character));
				break;
			}
		}
		output.push_back('\"');
	}

	void Append_CanonicalJson(const JSON_VALUE& value, std::string& output)
	{
		switch (value.eKind)
		{
		case JSON_KIND::STRING:
			Append_CanonicalString(value.String, output); break;
		case JSON_KIND::INTEGER:
			if (value.isNegative) output.push_back('-');
			output += std::to_string(value.Integer); break;
		case JSON_KIND::BOOLEAN:
			output += value.Boolean ? "true" : "false"; break;
		case JSON_KIND::NULL_VALUE:
			output += "null"; break;
		case JSON_KIND::ARRAY:
			output.push_back('[');
			for (std::size_t index = 0u; index < value.Elements.size(); ++index)
			{
				if (0u != index) output.push_back(',');
				Append_CanonicalJson(value.Elements[index], output);
			}
			output.push_back(']');
			break;
		case JSON_KIND::OBJECT:
		{
			std::vector<std::size_t> order(value.MemberNames.size());
			for (std::size_t index = 0u; index < order.size(); ++index)
				order[index] = index;
			std::sort(order.begin(), order.end(),
				[&value](const std::size_t left, const std::size_t right)
				{
					return value.MemberNames[left] < value.MemberNames[right];
				});
			output.push_back('{');
			for (std::size_t ordinal = 0u; ordinal < order.size(); ++ordinal)
			{
				if (0u != ordinal) output.push_back(',');
				const std::size_t index = order[ordinal];
				Append_CanonicalString(value.MemberNames[index], output);
				output.push_back(':');
				Append_CanonicalJson(value.MemberValues[index], output);
			}
			output.push_back('}');
			break;
		}
		}
	}

	bool Hash_BytesSha256(
		const std::span<const std::uint8_t> bytes,
		GameplayDataRevision& revision)
	{
		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD objectBytes = 0u;
		DWORD resultBytes = 0u;
		std::vector<std::uint8_t> object;
		GameplayDataRevision staged{};
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(
			&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&objectBytes), sizeof(objectBytes),
				&resultBytes, 0u))
		{
			object.resize(objectBytes);
			if (0 <= BCryptCreateHash(
				algorithm, &hash, object.data(), objectBytes,
				nullptr, 0u, 0u))
			{
				std::size_t offset = 0u;
				succeeded = true;
				while (offset < bytes.size())
				{
					const ULONG chunk = static_cast<ULONG>((std::min)(
						bytes.size() - offset,
						static_cast<std::size_t>((std::numeric_limits<ULONG>::max)())));
					if (0 > BCryptHashData(hash,
						const_cast<PUCHAR>(bytes.data() + offset), chunk, 0u))
					{
						succeeded = false; break;
					}
					offset += chunk;
				}
				if (succeeded && 0 > BCryptFinishHash(hash,
					staged.Bytes.data(),
					static_cast<ULONG>(staged.Bytes.size()), 0u))
					succeeded = false;
			}
		}
		if (nullptr != hash) BCryptDestroyHash(hash);
		if (nullptr != algorithm) BCryptCloseAlgorithmProvider(algorithm, 0u);
		if (!succeeded || !staged.Is_Valid()) return false;
		revision = staged;
		return true;
	}

	bool Read_BoundedFile(
		const std::filesystem::path& path, const std::uintmax_t maximumBytes,
		std::vector<std::uint8_t>& bytes, std::string& status)
	{
		std::error_code error;
		const std::uintmax_t size = std::filesystem::file_size(path, error);
		if (error || size > maximumBytes ||
			size > static_cast<std::uintmax_t>((std::numeric_limits<std::size_t>::max)()))
		{
			status = "Candidate artifact exceeds its bounded file size";
			return false;
		}
		std::ifstream stream(path, std::ios::binary);
		if (!stream)
		{
			status = "Candidate artifact could not be opened";
			return false;
		}
		bytes.resize(static_cast<std::size_t>(size));
		if (!bytes.empty())
			stream.read(reinterpret_cast<char*>(bytes.data()),
				static_cast<std::streamsize>(bytes.size()));
		if (!stream || stream.peek() != std::ifstream::traits_type::eof())
		{
			status = "Candidate artifact read was incomplete";
			return false;
		}
		return true;
	}

	bool Hash_FileSha256(
		const std::filesystem::path& path, GameplayDataRevision& revision,
		std::string& status)
	{
		std::ifstream stream(path, std::ios::binary);
		if (!stream)
		{
			status = "Candidate artifact could not be opened for hashing";
			return false;
		}
		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD objectBytes = 0u;
		DWORD resultBytes = 0u;
		std::vector<std::uint8_t> object;
		GameplayDataRevision staged{};
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(
			&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&objectBytes), sizeof(objectBytes),
				&resultBytes, 0u))
		{
			object.resize(objectBytes);
			succeeded = 0 <= BCryptCreateHash(
				algorithm, &hash, object.data(), objectBytes,
				nullptr, 0u, 0u);
		}
		/* Candidate admission runs on Server.exe's normal 1 MiB thread stack.
		   Keeping a full 1 MiB hashing chunk as a local array makes this
		   function's prologue larger than that reserve and faults in __chkstk
		   before the first artifact can be validated. Preserve the bounded
		   streaming chunk size, but own its storage on the heap. */
		std::vector<std::uint8_t> buffer(1024u * 1024u);
		while (succeeded && stream)
		{
			stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
			const std::streamsize count = stream.gcount();
			if (count > 0 && 0 > BCryptHashData(
				hash, buffer.data(), static_cast<ULONG>(count), 0u))
				succeeded = false;
		}
		if (!stream.eof()) succeeded = false;
		if (succeeded && 0 > BCryptFinishHash(hash, staged.Bytes.data(),
			static_cast<ULONG>(staged.Bytes.size()), 0u))
			succeeded = false;
		if (nullptr != hash) BCryptDestroyHash(hash);
		if (nullptr != algorithm) BCryptCloseAlgorithmProvider(algorithm, 0u);
		if (!succeeded || !staged.Is_Valid())
		{
			status = "Candidate artifact SHA-256 failed";
			return false;
		}
		revision = staged;
		return true;
	}

	bool Is_SafeRelativePath(const std::string& relative)
	{
		if (relative.empty() || '/' == relative.front() ||
			relative.find('\\') != std::string::npos ||
			relative.find(':') != std::string::npos)
			return false;
		std::size_t cursor = 0u;
		while (cursor <= relative.size())
		{
			const std::size_t slash = relative.find('/', cursor);
			const std::string_view segment(relative.data() + cursor,
				(std::string::npos == slash ? relative.size() : slash) - cursor);
			if (segment.empty() || "." == segment || ".." == segment)
				return false;
			if (std::string::npos == slash) break;
			cursor = slash + 1u;
		}
		return true;
	}

	bool Resolve_ExactRegularFile(
		const std::filesystem::path& candidateDirectory,
		const std::string& relative, std::filesystem::path& path,
		std::string& status)
	{
		if (!Is_SafeRelativePath(relative))
		{
			status = "Candidate artifact relative path is invalid";
			return false;
		}
		std::error_code error;
		const std::filesystem::path relativePath{ relative };
		if (relativePath.is_absolute() || relativePath.has_root_name() ||
			relativePath.has_root_directory())
		{
			status = "Candidate artifact path is drive-qualified or absolute";
			return false;
		}
		const std::filesystem::path expected =
			(candidateDirectory / relativePath).lexically_normal();
		const std::filesystem::file_status linkStatus =
			std::filesystem::symlink_status(expected, error);
		if (error || !std::filesystem::is_regular_file(linkStatus) ||
			std::filesystem::is_symlink(linkStatus))
		{
			status = "Candidate artifact is not a regular non-symlink file";
			return false;
		}
		const std::filesystem::path canonical =
			std::filesystem::canonical(expected, error);
		const std::filesystem::path relativeCanonical =
			canonical.lexically_relative(candidateDirectory);
		if (error || canonical != expected || relativeCanonical.empty() ||
			relativeCanonical.is_absolute() || relativeCanonical.has_root_name() ||
			relativeCanonical.has_root_directory() ||
			relativeCanonical.begin() == relativeCanonical.end() ||
			*relativeCanonical.begin() == L"..")
		{
			status = "Candidate artifact escaped its canonical revision directory";
			return false;
		}
		path = canonical;
		return true;
	}

	std::filesystem::path Resolve_RepositoryRoot()
	{
		std::vector<std::filesystem::path> starts;
		std::vector<wchar_t> pathBuffer(32768u);
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
			starts.emplace_back(pathBuffer.data());
		const DWORD moduleLength = GetModuleFileNameW(nullptr, pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != moduleLength && moduleLength < pathBuffer.size())
			starts.emplace_back(std::filesystem::path(pathBuffer.data()).parent_path());
		std::error_code error;
		starts.push_back(std::filesystem::current_path(error));
		for (std::filesystem::path start : starts)
		{
			for (std::size_t depth = 0u; depth < 10u && !start.empty(); ++depth)
			{
				error.clear();
				if (std::filesystem::is_directory(start / L"Data", error) &&
					std::filesystem::is_directory(
						start / L"Tools" / L"ValtanPipeline", error))
					return std::filesystem::canonical(start, error);
				const std::filesystem::path parent = start.parent_path();
				if (parent == start) break;
				start = parent;
			}
		}
		return {};
	}

	std::filesystem::path Resolve_RuntimeGameplayBootstrap()
	{
		namespace fs = std::filesystem;
		std::vector<wchar_t> pathBuffer(32768u);
		fs::path dataRoot;
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
		{
			dataRoot = fs::path(pathBuffer.data()).lexically_normal();
		}
		else
		{
			const DWORD moduleLength = GetModuleFileNameW(
				nullptr, pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
			if (0u == moduleLength || moduleLength >= pathBuffer.size())
				return {};
			dataRoot = fs::path(pathBuffer.data()).parent_path().parent_path() /
				L"DataFiles";
		}
		const fs::path expected =
			(dataRoot / L"Gameplay" / L"Gameplay.bootstrap").lexically_normal();
		std::error_code error;
		const fs::file_status linkStatus = fs::symlink_status(expected, error);
		const fs::path canonical = fs::canonical(expected, error);
		if (error || !fs::is_regular_file(linkStatus) ||
			fs::is_symlink(linkStatus) || canonical != expected)
		{
			return {};
		}
		return canonical;
	}

	std::vector<std::string_view> Split_Tabs(const std::string_view line)
	{
		std::vector<std::string_view> fields;
		std::size_t start = 0u;
		while (true)
		{
			const std::size_t tab = line.find('\t', start);
			fields.push_back(line.substr(start,
				std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab) break;
			start = tab + 1u;
		}
		return fields;
	}

	bool Is_ValtanOwnedGameplayRow(
		const std::vector<std::string_view>& fields)
	{
		if (fields.size() < 2u) return false;
		const std::string_view kind = fields[0];
		const std::string_view owner = fields[1];
		if (("BOSS" == kind || "BOSSARMOR" == kind ||
			"BOSSPART" == kind) && "BOSS_VALTAN" == owner)
		{
			return true;
		}
		if ("DAMAGE" == kind)
			return owner.starts_with("damage.valtan.");
		const bool encounterOwned = "ENCOUNTERINTRO" == kind ||
			"BOSSCOMBATOBJECT" == kind ||
			"BOSSCOMBATOBJECTHIT" == kind ||
			kind.starts_with("PATTERN") || kind.starts_with("VALTANTIMELINE");
		return encounterOwned && "ENCOUNTER_VALTAN" == owner;
	}

	bool Build_NonValtanGameplayRevisionBytes(
		const std::string_view content,
		GameplayDataRevision& revision,
		std::string& status)
	{
		revision = {};

		if (!Is_ValidUtf8(content) ||
			std::string_view::npos != content.find('\0'))
		{
			status = "Gameplay bootstrap is not bounded valid UTF-8 text";
			return false;
		}
		std::string normalized;
		normalized.reserve(content.size());
		std::uint64_t declaredRowCount = 0u;
		std::uint64_t actualRowCount = 0u;
		bool hasHeader = false;
		std::size_t start = 0u;
		while (start < content.size())
		{
			const std::size_t newline = content.find('\n', start);
			std::string_view line = content.substr(start,
				std::string_view::npos == newline ? newline : newline - start);
			if (!line.empty() && '\r' == line.back()) line.remove_suffix(1u);
			if (line.empty())
			{
				status = "Gameplay bootstrap contains an empty row";
				return false;
			}
			const std::vector<std::string_view> fields = Split_Tabs(line);
			if (!hasHeader)
			{
				std::uint32_t version = 0u;
				const auto parsedVersion = fields.size() < 2u ?
					std::from_chars(line.data(), line.data(), version) :
					std::from_chars(fields[1].data(),
						fields[1].data() + fields[1].size(), version);
				const auto parsedCount = fields.size() < 3u ?
					std::from_chars(line.data(), line.data(), declaredRowCount) :
					std::from_chars(fields[2].data(),
						fields[2].data() + fields[2].size(), declaredRowCount);
				if (3u != fields.size() ||
					"LOSTARK_GAMEPLAY_BOOTSTRAP" != fields[0] ||
					std::errc{} != parsedVersion.ec ||
					parsedVersion.ptr != fields[1].data() + fields[1].size() ||
					LostArk::Server::GAMEPLAY_BOOTSTRAP_VERSION != version ||
					std::errc{} != parsedCount.ec ||
					parsedCount.ptr != fields[2].data() + fields[2].size() || !declaredRowCount ||
					declaredRowCount > LostArk::Shared::GAMEPLAY_BOOTSTRAP_MAX_ROWS)
				{
					status = "Gameplay bootstrap header is invalid for domain hashing";
					return false;
				}
				normalized.append("LOSTARK_NON_VALTAN_GAMEPLAY\t");
				normalized.append(fields[1]);
				normalized.push_back('\n');
				hasHeader = true;
			}
			else
			{
				++actualRowCount;
				if (!Is_ValtanOwnedGameplayRow(fields))
				{
					normalized.append(line);
					normalized.push_back('\n');
				}
			}
			if (std::string_view::npos == newline) break;
			start = newline + 1u;
		}
		if (!hasHeader || actualRowCount != declaredRowCount ||
			!Hash_BytesSha256(std::span<const std::uint8_t>(
				reinterpret_cast<const std::uint8_t*>(normalized.data()),
				normalized.size()), revision))
		{
			status = "Gameplay bootstrap row count or non-Valtan hash is invalid";
			return false;
		}
		status.clear();
		return true;
	}

    bool Build_NonValtanGameplayRevision(const std::filesystem::path& path,
        GameplayDataRevision& revision, std::string& status)
    {
        std::vector<std::uint8_t> bytes;
        if (!Read_BoundedFile(path, LostArk::Shared::GAMEPLAY_BOOTSTRAP_MAX_BYTES, bytes, status)) return false;
        return Build_NonValtanGameplayRevisionBytes(std::string_view(
            reinterpret_cast<const char*>(bytes.data()), bytes.size()), revision, status);
    }

	bool Parse_JsonBytes(const std::vector<std::uint8_t>& bytes,
		JSON_VALUE& value, std::string& status)
	{
		return CBoundedJsonParser(std::string_view(
			reinterpret_cast<const char*>(bytes.data()), bytes.size())).Parse(
			value, status);
	}

	bool Validate_ExactStringArray(const JSON_VALUE* value,
		const std::initializer_list<std::string_view> expected)
	{
		if (nullptr == value || JSON_KIND::ARRAY != value->eKind ||
			value->Elements.size() != expected.size())
			return false;
		std::size_t index = 0u;
		for (const std::string_view item : expected)
			if (!Is_String(&value->Elements[index++], item)) return false;
		return true;
	}

	bool Admit_ValtanCandidateGeneration(
		const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request,
		const GameplayDataRevision& activeNonValtanGameplayRevision,
		std::shared_ptr<const LostArk::Server::CGameplayCatalog>& candidate,
		GameplayDataRevision& candidateBootstrapContentRevision,
		GameplayDataRevision& candidateNonValtanGameplayRevision,
		std::string& status)
	{
		using namespace LostArk::Shared;
		namespace fs = std::filesystem;
		candidate.reset();
		candidateBootstrapContentRevision = {};
		candidateNonValtanGameplayRevision = {};
		if (request.iRequiredPresentationLaneMask !=
			GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK ||
			!activeNonValtanGameplayRevision.Is_Valid())
		{
			status = "Candidate request or active non-Valtan identity is invalid";
			return false;
		}
		const std::string revisionHex =
			Format_GameplayDataRevision(request.CandidateRevision);
		const fs::path repositoryRoot = Resolve_RepositoryRoot();
		if (repositoryRoot.empty() || revisionHex.empty())
		{
			status = "Repository or candidate revision identity is unavailable";
			return false;
		}
		const fs::path expectedDirectory = (repositoryRoot / L"Intermediate" /
			L"ValtanTuningCandidates" / L"revisions" /
			fs::path(revisionHex)).lexically_normal();
		std::error_code error;
		const fs::file_status directoryStatus =
			fs::symlink_status(expectedDirectory, error);
		const fs::path candidateDirectory =
			fs::canonical(expectedDirectory, error);
		if (error || !fs::is_directory(directoryStatus) ||
			fs::is_symlink(directoryStatus) || candidateDirectory != expectedDirectory)
		{
			status = "Fixed candidate revision directory is unavailable or non-canonical";
			return false;
		}

		fs::path manifestPath;
		fs::path identityPath;
		std::vector<std::uint8_t> manifestBytes;
		std::vector<std::uint8_t> identityBytes;
		if (!Resolve_ExactRegularFile(candidateDirectory,
			"revision-manifest.json", manifestPath, status) ||
			!Resolve_ExactRegularFile(candidateDirectory,
				"revision-identity.json", identityPath, status) ||
			!Read_BoundedFile(manifestPath, 8u * 1024u * 1024u,
				manifestBytes, status) ||
			!Read_BoundedFile(identityPath, 8u * 1024u * 1024u,
				identityBytes, status))
			return false;
		GameplayDataRevision identityRevision{};
		if (!Hash_BytesSha256(identityBytes, identityRevision) ||
			identityRevision != request.CandidateRevision)
		{
			status = "Raw revision identity SHA-256 does not match the request";
			return false;
		}

		JSON_VALUE manifest;
		JSON_VALUE identity;
		if (!Parse_JsonBytes(manifestBytes, manifest, status) ||
			!Parse_JsonBytes(identityBytes, identity, status))
			return false;
		std::string canonicalIdentity;
		canonicalIdentity.reserve(identityBytes.size());
		Append_CanonicalJson(identity, canonicalIdentity);
		if (canonicalIdentity.size() != identityBytes.size() ||
			!std::equal(canonicalIdentity.begin(), canonicalIdentity.end(),
				identityBytes.begin()))
		{
			status = "Revision identity bytes are not canonical JSON";
			return false;
		}
		if (!Has_ExactMembers(manifest,
			{ "schema", "formatVersion", "revisionId", "revisionIdentity",
			  "sourceManifestId", "authoringBaseRevision", "artifactSetId",
			  "draftPatchOperationCount", "allowedDomains",
			  "requiredPresentationLanes", "clientPresentationCompatibility",
			  "serverGameplayBootstrap", "applyClass", "runtimeActivation",
			  "serverSubmanifestSha256", "clientSubmanifestSha256",
			  "authoringSubmanifestSha256", "artifacts" }))
		{
			status = "Candidate revision manifest fields are not exact";
			return false;
		}
		JSON_VALUE manifestIdentity = manifest;
		JSON_VALUE* blankRevision = Find_Member(manifestIdentity, "revisionId");
		if (nullptr == blankRevision || JSON_KIND::STRING != blankRevision->eKind)
		{
			status = "Candidate revisionId is not a string";
			return false;
		}
		blankRevision->String.clear();
		if (!Json_Equals(manifestIdentity, identity))
		{
			status = "Manifest and canonical parent identity differ";
			return false;
		}
		if (!Is_String(Find_Member(manifest, "schema"),
				"lostark.valtan-tuning-revision-manifest") ||
			!Is_Integer(Find_Member(manifest, "formatVersion"), 1u, 1u) ||
			!Is_String(Find_Member(manifest, "revisionId"), revisionHex) ||
			!Is_String(Find_Member(manifest, "applyClass"), "HOT_RELOAD") ||
			!Is_String(Find_Member(manifest, "runtimeActivation"),
				"SERVER_2PC_TICK_BOUNDARY") ||
			!Validate_ExactStringArray(Find_Member(manifest, "allowedDomains"),
				{ "VALTAN_BOSS" }) ||
			!Validate_ExactStringArray(
				Find_Member(manifest, "requiredPresentationLanes"),
				{ "ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA",
				  "WORLD_EVENT_SET" }) ||
			!Is_Integer(Find_Member(manifest, "draftPatchOperationCount"),
				0u, 100000u))
		{
			status = "Candidate HOT_RELOAD activation contract is invalid";
			return false;
		}
		for (const std::string_view field :
			{ "revisionId", "sourceManifestId", "authoringBaseRevision",
			  "artifactSetId", "serverSubmanifestSha256",
			  "clientSubmanifestSha256", "authoringSubmanifestSha256" })
		{
			if (!Is_LowerSha256(Find_Member(manifest, field)))
			{
				status = "Candidate manifest contains a malformed SHA-256 field";
				return false;
			}
		}

		const JSON_VALUE* revisionIdentity =
			Find_Member(manifest, "revisionIdentity");
		if (nullptr == revisionIdentity || !Has_ExactMembers(*revisionIdentity,
			{ "kind", "algorithm", "identityPayloadPath",
			  "serverBootstrapContentRevision" }) ||
			!Is_String(Find_Member(*revisionIdentity, "kind"),
				"PARENT_MANIFEST") ||
			!Is_String(Find_Member(*revisionIdentity, "algorithm"),
				"SHA256_CANONICAL_JSON_WITH_EMPTY_REVISION_ID") ||
			!Is_String(Find_Member(*revisionIdentity, "identityPayloadPath"),
				"revision-identity.json") ||
			!Is_LowerSha256(Find_Member(
				*revisionIdentity, "serverBootstrapContentRevision")))
		{
			status = "Candidate parent revision identity contract is invalid";
			return false;
		}

		const JSON_VALUE* compatibility =
			Find_Member(manifest, "clientPresentationCompatibility");
		if (nullptr == compatibility || !Has_ExactMembers(*compatibility,
			{ "mode", "presentationGenerationId", "requiredLanes",
			  "artifacts" }) ||
			!Is_String(Find_Member(*compatibility, "mode"),
				"BYTE_IDENTICAL_TO_ACTIVE") ||
			!Is_LowerSha256(Find_Member(
				*compatibility, "presentationGenerationId")) ||
			!Validate_ExactStringArray(Find_Member(*compatibility, "requiredLanes"),
				{ "ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA",
				  "WORLD_EVENT_SET" }))
		{
			status = "Candidate presentation alias contract is invalid";
			return false;
		}
		const JSON_VALUE* aliasArtifacts =
			Find_Member(*compatibility, "artifacts");
		const std::string presentationGenerationId =
			Find_Member(*compatibility, "presentationGenerationId")->String;
		std::set<std::string> coveredLanes;
		if (nullptr == aliasArtifacts || JSON_KIND::ARRAY != aliasArtifacts->eKind)
		{
			status = "Candidate presentation alias artifact list is invalid";
			return false;
		}
		for (const JSON_VALUE& row : aliasArtifacts->Elements)
		{
			if (!Has_ExactMembers(row,
				{ "lane", "path", "sha256", "bytes",
				  "repositorySourceSha256" }))
			{
				status = "Candidate presentation alias row fields are invalid";
				return false;
			}
			const JSON_VALUE* lane = Find_Member(row, "lane");
			const JSON_VALUE* path = Find_Member(row, "path");
			const JSON_VALUE* hash = Find_Member(row, "sha256");
			const JSON_VALUE* sourceHash =
				Find_Member(row, "repositorySourceSha256");
			if (nullptr == lane || JSON_KIND::STRING != lane->eKind ||
				nullptr == path || JSON_KIND::STRING != path->eKind ||
				!Is_SafeRelativePath(path->String) || !Is_LowerSha256(hash) ||
				!Is_LowerSha256(sourceHash) || hash->String != sourceHash->String ||
				!Is_Integer(Find_Member(row, "bytes"), 0u,
					(std::numeric_limits<std::uint64_t>::max)()))
			{
				status = "Candidate presentation alias identity is invalid";
				return false;
			}
			if (lane->String != "ANIMATION" && lane->String != "EFFECT" &&
				lane->String != "COMBAT_VISUAL" && lane->String != "CAMERA" &&
				lane->String != "WORLD_EVENT_SET")
			{
				status = "Candidate presentation alias lane is unknown";
				return false;
			}
			coveredLanes.insert(lane->String);
		}
		if (coveredLanes != std::set<std::string>{
			"ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA",
			"WORLD_EVENT_SET" })
		{
			status = "Candidate presentation alias lane coverage is incomplete";
			return false;
		}

		const JSON_VALUE* bootstrap =
			Find_Member(manifest, "serverGameplayBootstrap");
		if (nullptr == bootstrap || !Has_ExactMembers(*bootstrap,
			{ "path", "formatVersion", "baselineRowCount", "candidateRowCount",
			  "removedValtanRows", "addedValtanRows", "baselineSha256",
			  "candidateSha256" }) ||
			!Is_String(Find_Member(*bootstrap, "path"),
				"Runtime/Gameplay/Gameplay.bootstrap") ||
			!Is_Integer(Find_Member(*bootstrap, "formatVersion"),
				LostArk::Server::GAMEPLAY_BOOTSTRAP_VERSION,
				LostArk::Server::GAMEPLAY_BOOTSTRAP_VERSION) ||
			!Is_LowerSha256(Find_Member(*bootstrap, "baselineSha256")) ||
			!Is_LowerSha256(Find_Member(*bootstrap, "candidateSha256")) ||
			Find_Member(*bootstrap, "candidateSha256")->String !=
				Find_Member(*revisionIdentity,
					"serverBootstrapContentRevision")->String)
		{
			status = "Candidate Server gameplay bootstrap contract is invalid";
			return false;
		}
		for (const std::string_view field :
			{ "baselineRowCount", "candidateRowCount", "removedValtanRows",
			  "addedValtanRows" })
		{
			if (!Is_Integer(Find_Member(*bootstrap, field), 0u, 4096u))
			{
				status = "Candidate Server gameplay row count is invalid";
				return false;
			}
		}
		GameplayDataRevision manifestBaselineRevision{};
		if (!Try_Parse_GameplayDataRevision(
			Find_Member(*bootstrap, "baselineSha256")->String,
			manifestBaselineRevision))
		{
			status = "Candidate bootstrap baseline identity is malformed";
			return false;
		}

		const JSON_VALUE* artifacts = Find_Member(manifest, "artifacts");
		if (nullptr == artifacts || JSON_KIND::ARRAY != artifacts->eKind ||
			artifacts->Elements.empty())
		{
			status = "Candidate artifact manifest is empty";
			return false;
		}
		std::set<std::string> artifactPaths;
		const std::string presentationGenerationManifestRelative =
			"Runtime/Gameplay/ValtanPresentationGenerations/" +
			presentationGenerationId + ".json";
		bool hasPresentationGenerationManifest = false;
		for (const JSON_VALUE& row : artifacts->Elements)
		{
			if (!Has_ExactMembers(row, { "path", "sha256", "bytes" }))
			{
				status = "Candidate artifact row fields are invalid";
				return false;
			}
			const JSON_VALUE* relative = Find_Member(row, "path");
			const JSON_VALUE* hash = Find_Member(row, "sha256");
			const JSON_VALUE* bytes = Find_Member(row, "bytes");
			if (nullptr == relative || JSON_KIND::STRING != relative->eKind ||
				!Is_SafeRelativePath(relative->String) ||
				relative->String == "revision-manifest.json" ||
				relative->String == "revision-identity.json" ||
				!artifactPaths.insert(relative->String).second ||
				!Is_LowerSha256(hash) || !Is_Integer(bytes, 0u,
					(std::numeric_limits<std::uint64_t>::max)()))
			{
				status = "Candidate artifact identity is invalid";
				return false;
			}
			if (relative->String == presentationGenerationManifestRelative)
			{
				if (hash->String != presentationGenerationId)
				{
					status =
						"Candidate presentation generation manifest identity is invalid";
					return false;
				}
				hasPresentationGenerationManifest = true;
			}
			fs::path artifactPath;
			GameplayDataRevision artifactRevision{};
			error.clear();
			if (!Resolve_ExactRegularFile(candidateDirectory,
				relative->String, artifactPath, status) ||
				fs::file_size(artifactPath, error) != bytes->Integer || error ||
				!Hash_FileSha256(artifactPath, artifactRevision, status) ||
				Format_GameplayDataRevision(artifactRevision) != hash->String)
			{
				if (status.empty())
					status = "Candidate artifact hash or byte count is invalid";
				return false;
			}
		}
		if (!hasPresentationGenerationManifest)
		{
			status =
				"Candidate artifact set omits its presentation generation manifest";
			return false;
		}
		std::string canonicalArtifacts;
		Append_CanonicalJson(*artifacts, canonicalArtifacts);
		GameplayDataRevision artifactSetRevision{};
		if (!Hash_BytesSha256(std::span<const std::uint8_t>(
			reinterpret_cast<const std::uint8_t*>(canonicalArtifacts.data()),
			canonicalArtifacts.size()), artifactSetRevision) ||
			Format_GameplayDataRevision(artifactSetRevision) !=
				Find_Member(manifest, "artifactSetId")->String)
		{
			status = "Candidate artifactSetId is invalid";
			return false;
		}
		for (const auto& pair :
			{ std::pair<std::string_view, std::string_view>{
				"serverSubmanifestSha256", "_manifest/server.json" },
			  std::pair<std::string_view, std::string_view>{
				"clientSubmanifestSha256", "_manifest/client.json" },
			  std::pair<std::string_view, std::string_view>{
				"authoringSubmanifestSha256", "_manifest/authoring.json" } })
		{
			fs::path submanifestPath;
			GameplayDataRevision submanifestRevision{};
			if (!Resolve_ExactRegularFile(candidateDirectory,
				std::string(pair.second), submanifestPath, status) ||
				!Hash_FileSha256(submanifestPath, submanifestRevision, status) ||
				Format_GameplayDataRevision(submanifestRevision) !=
					Find_Member(manifest, pair.first)->String)
			{
				status = "Candidate submanifest SHA-256 is invalid";
				return false;
			}
		}

		const std::string bootstrapRelative =
			Find_Member(*bootstrap, "path")->String;
		fs::path bootstrapPath;
		GameplayDataRevision bootstrapRevision{};
		error.clear();
		if (!Resolve_ExactRegularFile(candidateDirectory,
			bootstrapRelative, bootstrapPath, status) ||
			fs::file_size(bootstrapPath, error) > LostArk::Shared::GAMEPLAY_BOOTSTRAP_MAX_BYTES || error ||
			!Hash_FileSha256(bootstrapPath, bootstrapRevision, status) ||
			Format_GameplayDataRevision(bootstrapRevision) !=
				Find_Member(*bootstrap, "candidateSha256")->String)
		{
			status = "Candidate gameplay bootstrap raw SHA-256 is invalid";
			return false;
		}
		if (!Build_NonValtanGameplayRevision(
			bootstrapPath, candidateNonValtanGameplayRevision, status) ||
			candidateNonValtanGameplayRevision !=
				activeNonValtanGameplayRevision)
		{
			if (status.empty())
				status = "Candidate changes gameplay outside allowedDomains VALTAN_BOSS";
			return false;
		}
		auto staged = std::make_shared<LostArk::Server::CGameplayCatalog>();
		if (nullptr == staged || !staged->Load_FromBootstrap(
			bootstrapPath, bootstrapRevision, request.CandidateRevision))
		{
			status = nullptr == staged ?
				"Candidate gameplay catalog allocation failed" : staged->Get_Status();
			return false;
		}
		GameplayDataRevision presentationGenerationRevision{};
		if (!Try_Parse_GameplayDataRevision(
			presentationGenerationId, presentationGenerationRevision) ||
			staged->Get_ValtanPresentationGenerationId() !=
				presentationGenerationRevision)
		{
			status =
				"Candidate gameplay bootstrap and presentation generation disagree";
			return false;
		}
		candidate = std::move(staged);
		candidateBootstrapContentRevision = bootstrapRevision;
		status.clear();
		return true;
	}

	struct RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL final
	{
		std::uint32_t iTransactionSequence = 0u;
		LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION Base;
		LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION Candidate;
	};

	constexpr std::wstring_view RUNTIME_ACTIVE_POINTER_NAME =
		L"active-generation.json";
	constexpr std::wstring_view RUNTIME_ACTIVATION_JOURNAL_NAME =
		L"active-generation.journal.json";

	bool RuntimeGenerationEquals(
		const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& left,
		const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& right)
	{
		return left.eSource == right.eSource &&
			left.Revision == right.Revision &&
			left.BootstrapContentRevision == right.BootstrapContentRevision &&
			left.NonValtanGameplayRevision == right.NonValtanGameplayRevision;
	}

	std::string RuntimeSourceName(
		const LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE source)
	{
		using SOURCE = LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE;
		return SOURCE::CANDIDATE == source ?
			"CANDIDATE" : "PACKAGED_BASELINE";
	}

	void AppendRuntimeGenerationRecord(
		const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record,
		std::string& output)
	{
		output += "{\"nonValtanGameplayRevision\":";
		Append_CanonicalString(
			Format_GameplayDataRevision(record.NonValtanGameplayRevision), output);
		output += ",\"revisionId\":";
		Append_CanonicalString(
			Format_GameplayDataRevision(record.Revision), output);
		output += ",\"serverBootstrapContentRevision\":";
		Append_CanonicalString(
			Format_GameplayDataRevision(record.BootstrapContentRevision), output);
		output += ",\"sourceKind\":";
		Append_CanonicalString(RuntimeSourceName(record.eSource), output);
		output.push_back('}');
	}

	std::string BuildRuntimeActivePointerBytes(
		const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record)
	{
		std::string output = "{\"formatVersion\":1,\"generation\":";
		AppendRuntimeGenerationRecord(record, output);
		output +=
			",\"schema\":\"lostark.server-runtime-active-gameplay-generation\"}";
		return output;
	}

	std::string BuildRuntimeActivationJournalBytes(
		const RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL& journal)
	{
		std::string output = "{\"base\":";
		AppendRuntimeGenerationRecord(journal.Base, output);
		output += ",\"candidate\":";
		AppendRuntimeGenerationRecord(journal.Candidate, output);
		output +=
			",\"formatVersion\":1,\"schema\":\"lostark.server-runtime-gameplay-activation-journal\",\"transactionSequence\":";
		output += std::to_string(journal.iTransactionSequence);
		output.push_back('}');
		return output;
	}

	bool ParseRuntimeGenerationRecord(
		const JSON_VALUE* value,
		LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record)
	{
		using SOURCE = LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE;
		if (nullptr == value || !Has_ExactMembers(*value,
			{ "nonValtanGameplayRevision", "revisionId",
			  "serverBootstrapContentRevision", "sourceKind" }) ||
			!Is_LowerSha256(Find_Member(*value, "nonValtanGameplayRevision")) ||
			!Is_LowerSha256(Find_Member(*value, "revisionId")) ||
			!Is_LowerSha256(
				Find_Member(*value, "serverBootstrapContentRevision")))
		{
			return false;
		}
		const JSON_VALUE* source = Find_Member(*value, "sourceKind");
		if (Is_String(source, "PACKAGED_BASELINE"))
			record.eSource = SOURCE::PACKAGED_BASELINE;
		else if (Is_String(source, "CANDIDATE"))
			record.eSource = SOURCE::CANDIDATE;
		else
			return false;
		return Try_Parse_GameplayDataRevision(
			Find_Member(*value, "revisionId")->String, record.Revision) &&
			Try_Parse_GameplayDataRevision(
				Find_Member(*value,
					"serverBootstrapContentRevision")->String,
				record.BootstrapContentRevision) &&
			Try_Parse_GameplayDataRevision(
				Find_Member(*value, "nonValtanGameplayRevision")->String,
				record.NonValtanGameplayRevision) && record.Is_Valid();
	}

	bool ParseCanonicalRuntimeJson(
		const std::vector<std::uint8_t>& bytes,
		JSON_VALUE& value,
		std::string& status)
	{
		if (!Parse_JsonBytes(bytes, value, status))
			return false;
		std::string canonical;
		canonical.reserve(bytes.size());
		Append_CanonicalJson(value, canonical);
		if (canonical.size() != bytes.size() ||
			!std::equal(canonical.begin(), canonical.end(), bytes.begin()))
		{
			status = "Runtime gameplay activation JSON is not canonical";
			return false;
		}
		return true;
	}

	bool ParseRuntimeActivePointer(
		const std::vector<std::uint8_t>& bytes,
		LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record,
		std::string& status)
	{
		JSON_VALUE root;
		if (!ParseCanonicalRuntimeJson(bytes, root, status) ||
			!Has_ExactMembers(root,
				{ "formatVersion", "generation", "schema" }) ||
			!Is_Integer(Find_Member(root, "formatVersion"), 1u, 1u) ||
			!Is_String(Find_Member(root, "schema"),
				"lostark.server-runtime-active-gameplay-generation") ||
			!ParseRuntimeGenerationRecord(
				Find_Member(root, "generation"), record))
		{
			if (status.empty()) status = "Runtime active gameplay pointer is invalid";
			return false;
		}
		status.clear();
		return true;
	}

	bool ParseRuntimeActivationJournal(
		const std::vector<std::uint8_t>& bytes,
		RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL& journal,
		std::string& status)
	{
		JSON_VALUE root;
		if (!ParseCanonicalRuntimeJson(bytes, root, status) ||
			!Has_ExactMembers(root,
				{ "base", "candidate", "formatVersion", "schema",
				  "transactionSequence" }) ||
			!Is_Integer(Find_Member(root, "formatVersion"), 1u, 1u) ||
			!Is_Integer(Find_Member(root, "transactionSequence"), 1u,
				(std::numeric_limits<std::uint32_t>::max)()) ||
			!Is_String(Find_Member(root, "schema"),
				"lostark.server-runtime-gameplay-activation-journal") ||
			!ParseRuntimeGenerationRecord(
				Find_Member(root, "base"), journal.Base) ||
			!ParseRuntimeGenerationRecord(
				Find_Member(root, "candidate"), journal.Candidate) ||
			LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE !=
				journal.Candidate.eSource ||
			RuntimeGenerationEquals(journal.Base, journal.Candidate))
		{
			if (status.empty()) status = "Runtime gameplay activation journal is invalid";
			return false;
		}
		journal.iTransactionSequence = static_cast<std::uint32_t>(
			Find_Member(root, "transactionSequence")->Integer);
		status.clear();
		return true;
	}

	bool PrepareRuntimeGameplayRoot(
		const std::filesystem::path& requested,
		const bool create,
		std::filesystem::path& root,
		std::string& status)
	{
		namespace fs = std::filesystem;
		if (requested.empty() || !requested.is_absolute())
		{
			status = "Runtime gameplay activation root is not absolute";
			return false;
		}
		std::error_code error;
		const fs::path expected = requested.lexically_normal();
		if (create && !fs::create_directories(expected, error) && error)
		{
			status = "Runtime gameplay activation root cannot be created";
			return false;
		}
		if (!fs::exists(expected, error) || error)
		{
			status = create ?
				"Runtime gameplay activation root is unavailable" :
				"Runtime gameplay activation root is absent";
			return false;
		}
		const fs::file_status linkStatus = fs::symlink_status(expected, error);
		const fs::path canonical = fs::canonical(expected, error);
		const DWORD attributes = ::GetFileAttributesW(expected.c_str());
		if (error || !fs::is_directory(linkStatus) || fs::is_symlink(linkStatus) ||
			INVALID_FILE_ATTRIBUTES == attributes ||
			0u != (attributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
			canonical != expected)
		{
			status = "Runtime gameplay activation root is non-canonical or reparsed";
			return false;
		}
		root = canonical;
		status.clear();
		return true;
	}

	bool InspectRuntimeGameplayFile(
		const std::filesystem::path& root,
		const std::wstring_view name,
		std::filesystem::path& path,
		bool& exists,
		std::string& status)
	{
		namespace fs = std::filesystem;
		path = (root / fs::path(name)).lexically_normal();
		if (path.parent_path() != root)
		{
			status = "Runtime gameplay activation file escaped its root";
			return false;
		}
		std::error_code error;
		const fs::file_status linkStatus = fs::symlink_status(path, error);
		if (error == std::errc::no_such_file_or_directory)
		{
			exists = false;
			status.clear();
			return true;
		}
		if (error)
		{
			status = "Runtime gameplay activation file status failed";
			return false;
		}
		exists = fs::exists(linkStatus);
		if (!exists)
		{
			status.clear();
			return true;
		}
		const DWORD attributes = ::GetFileAttributesW(path.c_str());
		if (!fs::is_regular_file(linkStatus) || fs::is_symlink(linkStatus) ||
			INVALID_FILE_ATTRIBUTES == attributes ||
			0u != (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
		{
			status = "Runtime gameplay activation file is not an exact regular file";
			return false;
		}
		status.clear();
		return true;
	}

	bool ReadOptionalRuntimeGameplayFile(
		const std::filesystem::path& root,
		const std::wstring_view name,
		bool& exists,
		std::vector<std::uint8_t>& bytes,
		std::string& status)
	{
		std::filesystem::path path;
		if (!InspectRuntimeGameplayFile(root, name, path, exists, status))
			return false;
		bytes.clear();
		return !exists || Read_BoundedFile(path, 64u * 1024u, bytes, status);
	}

	bool AtomicWriteRuntimeGameplayFile(
		const std::filesystem::path& root,
		const std::wstring_view name,
		const std::string_view bytes,
		std::string& status)
	{
		std::filesystem::path finalPath;
		bool finalExists = false;
		if (!InspectRuntimeGameplayFile(
			root, name, finalPath, finalExists, status))
			return false;
		const std::filesystem::path stagePath = finalPath.wstring() +
			L".stage." + std::to_wstring(::GetCurrentProcessId()) + L"." +
			std::to_wstring(::GetTickCount64());
		HANDLE file = ::CreateFileW(
			stagePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, nullptr);
		if (INVALID_HANDLE_VALUE == file)
		{
			status = "Runtime gameplay activation stage file cannot be created";
			return false;
		}
		bool succeeded = true;
		std::size_t written = 0u;
		while (written < bytes.size())
		{
			const DWORD requestBytes = static_cast<DWORD>((std::min)(
				bytes.size() - written,
				static_cast<std::size_t>((std::numeric_limits<DWORD>::max)())));
			DWORD chunkBytes = 0u;
			if (!::WriteFile(file, bytes.data() + written, requestBytes,
				&chunkBytes, nullptr) || chunkBytes != requestBytes)
			{
				succeeded = false;
				break;
			}
			written += chunkBytes;
		}
		if (succeeded) succeeded = FALSE != ::FlushFileBuffers(file);
		if (!::CloseHandle(file)) succeeded = false;
		if (succeeded)
		{
			succeeded = FALSE != ::MoveFileExW(
				stagePath.c_str(), finalPath.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
		if (!succeeded)
		{
			(void)::DeleteFileW(stagePath.c_str());
			status = "Runtime gameplay activation atomic write failed";
			return false;
		}
		status.clear();
		return true;
	}

	void DeleteRuntimeGameplayJournal(
		const std::filesystem::path& root) noexcept
	{
		std::filesystem::path path;
		bool exists = false;
		std::string ignored;
		if (InspectRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME,
			path, exists, ignored) && exists)
		{
			(void)::DeleteFileW(path.c_str());
		}
	}

	bool DeleteRuntimeGameplayJournalExact(
		const std::filesystem::path& root,
		std::string& status)
	{
		std::filesystem::path path;
		bool exists = false;
		if (!InspectRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME,
			path, exists, status))
			return false;
		if (exists && !::DeleteFileW(path.c_str()))
		{
			status = "Runtime gameplay activation journal cannot be removed";
			return false;
		}
		if (!InspectRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME,
			path, exists, status) || exists)
		{
			if (status.empty())
				status = "Runtime gameplay activation journal removal was not durable";
			return false;
		}
		status.clear();
		return true;
	}
}

bool LostArk::Server::CServerApp::Acquire_RuntimeGameplayProcessMutex(
	void*& handle,
	std::string& status)
{
#ifdef _DEBUG
	constexpr const wchar_t* MUTEX_NAME =
		L"Local\\LostArk.Server.ValtanRuntimeActivation.Debug";
#else
	constexpr const wchar_t* MUTEX_NAME =
		L"Local\\LostArk.Server.ValtanRuntimeActivation.Release";
#endif
	return Acquire_NamedRuntimeGameplayProcessMutex(MUTEX_NAME, handle, status);
}

bool LostArk::Server::CServerApp::Acquire_NamedRuntimeGameplayProcessMutex(
	const wchar_t* name,
	void*& handle,
	std::string& status)
{
	handle = nullptr;
	if (nullptr == name || L'\0' == name[0])
	{
		status = "Runtime gameplay process mutex name is missing";
		return false;
	}
	HANDLE mutex = ::CreateMutexW(nullptr, FALSE, name);
	if (nullptr == mutex)
	{
		status = "Runtime gameplay process mutex cannot be created";
		return false;
	}
	const DWORD wait = ::WaitForSingleObject(mutex, 0u);
	if (WAIT_OBJECT_0 != wait && WAIT_ABANDONED != wait)
	{
		::CloseHandle(mutex);
		status = WAIT_TIMEOUT == wait ?
			"Another Server process owns the runtime gameplay activation lock" :
			"Runtime gameplay process mutex wait failed";
		return false;
	}
	handle = mutex;
	status.clear();
	return true;
}

void LostArk::Server::CServerApp::Release_RuntimeGameplayProcessMutex(
	void*& handle) noexcept
{
	if (nullptr == handle) return;
	HANDLE mutex = static_cast<HANDLE>(handle);
	(void)::ReleaseMutex(mutex);
	(void)::CloseHandle(mutex);
	handle = nullptr;
}

bool LostArk::Server::CServerApp::Persist_RuntimeGameplayActivation(
	const std::filesystem::path& runtimeRoot,
	const std::uint32_t transactionSequence,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& candidate,
	std::string& status)
{
	if (0u == transactionSequence || !base.Is_Valid() ||
		!candidate.Is_Valid() ||
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE != candidate.eSource ||
		RuntimeGenerationEquals(base, candidate))
	{
		status = "Runtime gameplay activation identity is invalid";
		return false;
	}
	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, true, root, status))
		return false;
	bool journalExists = false;
	std::vector<std::uint8_t> journalBytes;
	if (!ReadOptionalRuntimeGameplayFile(root,
		RUNTIME_ACTIVATION_JOURNAL_NAME, journalExists, journalBytes, status) ||
		journalExists)
	{
		if (status.empty())
			status = "An unfinished runtime gameplay activation journal exists";
		return false;
	}
	bool pointerExists = false;
	std::vector<std::uint8_t> pointerBytes;
	if (!ReadOptionalRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
		pointerExists, pointerBytes, status))
		return false;
	if (pointerExists)
	{
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION diskBase{};
		if (!ParseRuntimeActivePointer(pointerBytes, diskBase, status) ||
			!RuntimeGenerationEquals(diskBase, base))
		{
			if (status.empty())
				status = "Runtime active pointer does not match process active generation";
			return false;
		}
	}
	else if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE == base.eSource)
	{
		status = "Candidate process active generation has no durable pointer";
		return false;
	}

	RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL journal{};
	journal.iTransactionSequence = transactionSequence;
	journal.Base = base;
	journal.Candidate = candidate;
	if (!AtomicWriteRuntimeGameplayFile(root,
		RUNTIME_ACTIVATION_JOURNAL_NAME,
		BuildRuntimeActivationJournalBytes(journal), status))
		return false;
	if (!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
		BuildRuntimeActivePointerBytes(candidate), status))
	{
		std::string rollbackStatus;
		if (!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
			BuildRuntimeActivePointerBytes(base), rollbackStatus))
		{
			status += "; durable base-pointer rollback also failed: " +
				rollbackStatus;
			return false;
		}
		DeleteRuntimeGameplayJournal(root);
		return false;
	}
	status.clear();
	return true;
}

bool LostArk::Server::CServerApp::Rollback_RuntimeGameplayActivation(
	const std::filesystem::path& runtimeRoot,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
	std::string& status)
{
	if (!base.Is_Valid())
	{
		status = "Runtime gameplay rollback base is invalid";
		return false;
	}
	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, false, root, status) ||
		!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
			BuildRuntimeActivePointerBytes(base), status))
		return false;
	DeleteRuntimeGameplayJournal(root);
	status.clear();
	return true;
}

void LostArk::Server::CServerApp::Complete_RuntimeGameplayActivation(
	const std::filesystem::path& runtimeRoot) noexcept
{
	std::filesystem::path root;
	std::string ignored;
	if (PrepareRuntimeGameplayRoot(runtimeRoot, false, root, ignored))
		DeleteRuntimeGameplayJournal(root);
}

bool LostArk::Server::CServerApp::Recover_RuntimeActiveGameplayPointer(
	const std::filesystem::path& runtimeRoot,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active,
	bool& hasPersistedPointer,
	std::string& status,
	const bool allowPackagedIdentityDrift)
{
	active = packaged;
	hasPersistedPointer = false;
	if (!packaged.Is_Valid() ||
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE != packaged.eSource)
	{
		status = "Packaged runtime gameplay identity is invalid";
		return false;
	}
	std::error_code existsError;
	if (!std::filesystem::exists(runtimeRoot, existsError))
	{
		if (existsError)
		{
			status = "Runtime gameplay activation root status failed";
			return false;
		}
		status.clear();
		return true;
	}
	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, false, root, status))
		return false;
	bool pointerExists = false;
	bool journalExists = false;
	std::vector<std::uint8_t> pointerBytes;
	std::vector<std::uint8_t> journalBytes;
	if (!ReadOptionalRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
		pointerExists, pointerBytes, status) ||
		!ReadOptionalRuntimeGameplayFile(root,
			RUNTIME_ACTIVATION_JOURNAL_NAME,
			journalExists, journalBytes, status))
		return false;

	RUNTIME_ACTIVE_GAMEPLAY_GENERATION pointer{};
	if (pointerExists && !ParseRuntimeActivePointer(
		pointerBytes, pointer, status))
		return false;
	if (!journalExists)
	{
		if (pointerExists) active = pointer;
		hasPersistedPointer = pointerExists;
		if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE ==
			active.eSource && !allowPackagedIdentityDrift &&
			!RuntimeGenerationEquals(active, packaged))
		{
			status = "Durable packaged pointer does not match packaged gameplay";
			return false;
		}
		status.clear();
		return true;
	}

	RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL journal{};
	if (!ParseRuntimeActivationJournal(journalBytes, journal, status))
		return false;
	if (pointerExists && RuntimeGenerationEquals(pointer, journal.Candidate))
		active = journal.Candidate;
	else if (pointerExists && RuntimeGenerationEquals(pointer, journal.Base))
		active = journal.Base;
	else if (!pointerExists &&
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE ==
			journal.Base.eSource && (allowPackagedIdentityDrift ||
			RuntimeGenerationEquals(journal.Base, packaged)))
		active = journal.Base;
	else
	{
		status = "Activation journal cannot recover to an exact old or new generation";
		return false;
	}
	hasPersistedPointer = pointerExists;
	if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE ==
		active.eSource && !allowPackagedIdentityDrift &&
		!RuntimeGenerationEquals(active, packaged))
	{
		status = "Recovered packaged generation does not match packaged gameplay";
		return false;
	}
	status.clear();
	return true;
}

bool LostArk::Server::CServerApp::Reset_RuntimeGameplayActivationToPackaged(
	const std::filesystem::path& runtimeRoot,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
	std::string& status)
{
	if (!packaged.Is_Valid() ||
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE != packaged.eSource)
	{
		status = "Packaged runtime gameplay identity is invalid for reset";
		return false;
	}
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION recovered{};
	bool hadPointer = false;
	if (!Recover_RuntimeActiveGameplayPointer(
		runtimeRoot, packaged, recovered, hadPointer, status, true))
		return false;
	(void)hadPointer;

	/* This is the offline schema-upgrade escape hatch. Recovery above validates
	the durable pointer/journal structure and proves that the pointer names one
	of the journal's exact old/new identities. The discarded candidate artifacts
	must not be re-admitted: they may be absent, corrupt, or use an intentionally
	retired bootstrap schema. Only the newly validated packaged generation is
	selected below. Malformed or ambiguous pointer/journal bytes still fail
	closed before any durable write. */

	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, true, root, status))
		return false;
	/* Finish any exact old/new activation journal first. A crash before or after
	this write still leaves one fully named generation. Only then linearize the
	offline reset with one atomic packaged pointer replacement. */
	if (!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
		BuildRuntimeActivePointerBytes(recovered), status) ||
		!DeleteRuntimeGameplayJournalExact(root, status))
		return false;
	if (!RuntimeGenerationEquals(recovered, packaged) &&
		!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME,
			BuildRuntimeActivePointerBytes(packaged), status))
		return false;
	status.clear();
	return true;
}

bool LostArk::Server::CServerApp::Load_RuntimeActiveGameplayGeneration(
	const std::filesystem::path& runtimeRoot,
	const std::shared_ptr<const CGameplayCatalog>& packagedGeneration,
	const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
	std::shared_ptr<const CGameplayCatalog>& activeGeneration,
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active,
	bool& isCandidate,
	std::string& status)
{
	activeGeneration.reset();
	isCandidate = false;
	bool hasPersistedPointer = false;
	if (nullptr == packagedGeneration ||
		!Recover_RuntimeActiveGameplayPointer(runtimeRoot, packaged, active,
			hasPersistedPointer, status))
		return false;
	if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE == active.eSource)
	{
		activeGeneration = packagedGeneration;
		Complete_RuntimeGameplayActivation(runtimeRoot);
		status.clear();
		return true;
	}

	LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST request{};
	request.iTransactionSequence = 1u;
	request.BaseRevision = packaged.Revision;
	request.CandidateRevision = active.Revision;
	request.iRequiredPresentationLaneMask =
		LostArk::Shared::GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK;
	std::shared_ptr<const CGameplayCatalog> candidateGeneration;
	LostArk::Shared::GameplayDataRevision bootstrapRevision{};
	LostArk::Shared::GameplayDataRevision nonValtanRevision{};
	if (!Admit_ValtanCandidateGeneration(
		request, packaged.NonValtanGameplayRevision, candidateGeneration,
		bootstrapRevision, nonValtanRevision, status) ||
		bootstrapRevision != active.BootstrapContentRevision ||
		nonValtanRevision != active.NonValtanGameplayRevision ||
		!Validate_ValtanHotReloadBaseProfile(
			packagedGeneration->Find_Boss("BOSS_VALTAN"),
			nullptr == candidateGeneration ? nullptr :
				candidateGeneration->Find_Boss("BOSS_VALTAN"), status))
	{
		if (status.empty())
			status = "Durable runtime candidate failed startup admission";
		return false;
	}
	activeGeneration = std::move(candidateGeneration);
	isCandidate = true;
	Complete_RuntimeGameplayActivation(runtimeRoot);
	status.clear();
	return true;
}

std::filesystem::path
LostArk::Server::CServerApp::Resolve_RuntimeActiveGameplayRoot() const
{
	if (!m_RuntimeActiveGameplayRootOverride.empty())
		return m_RuntimeActiveGameplayRootOverride;
	const std::filesystem::path repositoryRoot = Resolve_RepositoryRoot();
	if (repositoryRoot.empty()) return {};
	#ifdef _DEBUG
	constexpr std::wstring_view configuration = L"Debug";
	#else
	constexpr std::wstring_view configuration = L"Release";
	#endif
	return (repositoryRoot / L"Intermediate" / L"ValtanTuningRuntime" /
		L"Server" / configuration).lexically_normal();
}

int LostArk::Server::CServerApp::Reset_ValtanRuntimeToPackaged()
{
#ifndef _DEBUG
	std::cerr << "Valtan runtime reset is unavailable in Release builds.\n";
	return 2;
#else
	void* processMutex = nullptr;
	std::string status;
	if (!Acquire_RuntimeGameplayProcessMutex(processMutex, status))
	{
		std::cerr << "Valtan runtime reset refused. Status=" << status << '\n';
		return 3;
	}
	const auto finish = [&processMutex](const int code)
	{
		Release_RuntimeGameplayProcessMutex(processMutex);
		return code;
	};
	auto packagedCatalog = std::make_shared<CGameplayCatalog>();
	if (nullptr == packagedCatalog || !packagedCatalog->Load())
	{
		std::cerr << "Packaged gameplay failed to load for Valtan reset. Status="
			<< (nullptr == packagedCatalog ? "allocation failed" :
				packagedCatalog->Get_Status()) << '\n';
		return finish(1);
	}
	const std::filesystem::path bootstrapPath =
		Resolve_RuntimeGameplayBootstrap();
	GameplayDataRevision nonValtanRevision{};
	if (bootstrapPath.empty() || !Build_NonValtanGameplayRevision(
		bootstrapPath, nonValtanRevision, status))
	{
		std::cerr << "Packaged gameplay identity failed for Valtan reset. Status="
			<< status << '\n';
		return finish(1);
	}
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION packaged{};
	packaged.eSource =
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
	packaged.Revision = packagedCatalog->Get_ActiveRevision();
	packaged.BootstrapContentRevision = packaged.Revision;
	packaged.NonValtanGameplayRevision = nonValtanRevision;
	const std::filesystem::path repositoryRoot = Resolve_RepositoryRoot();
	const std::filesystem::path runtimeRoot = repositoryRoot.empty() ?
		std::filesystem::path{} :
		(repositoryRoot / L"Intermediate" / L"ValtanTuningRuntime" /
			L"Server" / L"Debug").lexically_normal();
	if (runtimeRoot.empty() ||
		!Reset_RuntimeGameplayActivationToPackaged(
			runtimeRoot, packaged, status))
	{
		std::cerr << "Valtan runtime reset failed closed. Status="
			<< (status.empty() ? "runtime root unavailable" : status) << '\n';
		return finish(1);
	}
	std::cout << "Valtan runtime reset selected packaged gameplay revision "
		<< LostArk::Shared::Format_GameplayDataRevision(packaged.Revision)
		<< ". Restart Server normally.\n";
	return finish(0);
#endif
}

bool LostArk::Server::CServerApp::Validate_ServerSessionDiagnosticJson(
	const std::string_view json,
	std::string& status)
{
	JSON_VALUE root{};
	CBoundedJsonParser parser{ json };
	if (!parser.Parse(root, status) || JSON_KIND::OBJECT != root.eKind)
	{
		if (status.empty())
			status = "Session diagnostic root must be a JSON object";
		return false;
	}
	const auto findMember = [&root](const std::string_view name)
		-> const JSON_VALUE*
		{
			for (std::size_t index = 0u;
				index < root.MemberNames.size(); ++index)
			{
				if (root.MemberNames[index] == name)
					return &root.MemberValues[index];
			}
			return nullptr;
		};
	const JSON_VALUE* schema = findMember("schema");
	const JSON_VALUE* formatVersion = findMember("formatVersion");
	const JSON_VALUE* protocolVersion =
		findMember("networkProtocolVersion");
	const JSON_VALUE* reason = findMember("reason");
	const JSON_VALUE* outbound = findMember("outbound");
	if (nullptr == schema || JSON_KIND::STRING != schema->eKind ||
		"lostark.server-session-diagnostic" != schema->String ||
		nullptr == formatVersion ||
		JSON_KIND::INTEGER != formatVersion->eKind ||
		formatVersion->isNegative || 1u != formatVersion->Integer ||
		nullptr == protocolVersion ||
		JSON_KIND::INTEGER != protocolVersion->eKind ||
		protocolVersion->isNegative ||
		LostArk::Shared::NETWORK_PROTOCOL_VERSION != protocolVersion->Integer ||
		nullptr == reason || JSON_KIND::STRING != reason->eKind ||
		reason->String.empty() || nullptr == outbound ||
		JSON_KIND::OBJECT != outbound->eKind)
	{
		status = "Session diagnostic schema/format/protocol/reason/outbound contract invalid";
		return false;
	}
	status.clear();
	return true;
}

LostArk::Server::CServerApp::~CServerApp()
{
	Shutdown();
}

int LostArk::Server::CServerApp::Run(
	const std::uint32_t automaticShutdownMilliseconds,
	const std::string_view bindAddress,
	const std::uint16_t port,
	const bool headless,
	const SERVER_CONCURRENCY_OPTIONS concurrency)
{
	if (concurrency.IocpWorkers < 1 || concurrency.IocpWorkers > 16 ||
		concurrency.JobWorkers < 1 || concurrency.JobWorkers > 16 ||
		(concurrency.Transport != SESSION_TRANSPORT_BACKEND::SELECT_THREADS &&
		 concurrency.Transport != SESSION_TRANSPORT_BACKEND::IOCP)) return 2;
	m_Concurrency = concurrency;
	try
	{
		if (concurrency.SnapshotJobs)
			m_SnapshotJobs = std::make_shared<LostArk::Shared::Concurrency::WorkStealingJobSystem>(
				LostArk::Shared::Concurrency::WorkStealingJobSystem::Config{concurrency.JobWorkers, 4096});
		if (concurrency.Transport == SESSION_TRANSPORT_BACKEND::IOCP)
		{
			m_IocpService = std::make_shared<CIocpService>();
			if (!m_IocpService->Start(concurrency.IocpWorkers)) return 1;
		}
	}
	catch (const std::exception& error)
	{
		std::cerr << "Concurrency startup failed: " << error.what() << '\n';
		return 1;
	}
	std::cout << "Transport=" << (concurrency.Transport == SESSION_TRANSPORT_BACKEND::IOCP ? "iocp" : "select")
		<< " IocpWorkers=" << (m_IocpService ? m_IocpService->Get_WorkerCount() : 0)
		<< " SnapshotExecutor=" << (concurrency.SnapshotJobs ? "chaselev" : "serial")
		<< " JobWorkers=" << (m_SnapshotJobs ? concurrency.JobWorkers : 0) << '\n';
	using LostArk::Shared::WORLD_ID;
#ifdef _DEBUG
	std::string processMutexStatus;
	if (!Acquire_RuntimeGameplayProcessMutex(
		m_hRuntimeGameplayProcessMutex, processMutexStatus))
	{
		std::cerr << "Server runtime gameplay lock acquisition failed. Status="
			<< processMutexStatus << '\n';
		return 1;
	}
#endif
	auto packagedGameplayCatalog = std::make_shared<CGameplayCatalog>();
	if (nullptr == packagedGameplayCatalog ||
		!packagedGameplayCatalog->Load())
	{
		std::cerr << "Process gameplay generation failed to initialize. Status="
			<< (nullptr == packagedGameplayCatalog ?
				"Gameplay catalog allocation failed" :
				packagedGameplayCatalog->Get_Status()) << '\n';
		return 1;
	}
	std::shared_ptr<const CGameplayCatalog> packagedGameplayGeneration =
		std::move(packagedGameplayCatalog);
	std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration =
		packagedGameplayGeneration;
	GameplayDataRevision initialNonValtanGameplayRevision{};
	std::string nonValtanStatus;
	const std::filesystem::path runtimeGameplayBootstrap =
		Resolve_RuntimeGameplayBootstrap();
	if (runtimeGameplayBootstrap.empty() ||
		!Build_NonValtanGameplayRevision(
			runtimeGameplayBootstrap, initialNonValtanGameplayRevision,
			nonValtanStatus))
	{
		std::cerr << "Process non-Valtan gameplay identity failed to initialize. "
			"Status=" << (nonValtanStatus.empty() ?
				"Runtime gameplay bootstrap is unavailable" : nonValtanStatus)
			<< '\n';
		return 1;
	}
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION packagedRuntimeGeneration{};
	packagedRuntimeGeneration.eSource =
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
	packagedRuntimeGeneration.Revision =
		packagedGameplayGeneration->Get_ActiveRevision();
    if (!Hash_FileSha256(runtimeGameplayBootstrap,
        packagedRuntimeGeneration.BootstrapContentRevision, nonValtanStatus))
    {
        std::cerr << "Process gameplay content identity failed: " << nonValtanStatus << '\n';
        return 1;
    }
	packagedRuntimeGeneration.NonValtanGameplayRevision =
		initialNonValtanGameplayRevision;
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION initialRuntimeGeneration =
		packagedRuntimeGeneration;
	bool initialGenerationIsCandidate = false;
#ifdef _DEBUG
	std::string runtimeRecoveryStatus;
	if (!Load_RuntimeActiveGameplayGeneration(
		Resolve_RuntimeActiveGameplayRoot(), packagedGameplayGeneration,
		packagedRuntimeGeneration, initialGameplayGeneration,
		initialRuntimeGeneration, initialGenerationIsCandidate,
		runtimeRecoveryStatus))
	{
		std::cerr << "Runtime active gameplay recovery failed closed. Status="
			<< runtimeRecoveryStatus << '\n';
		return 1;
	}
#endif

	std::map<WORLD_ID, std::shared_ptr<CGameRoom>> stagedSharedSimulations;
	const auto stageSharedSimulation =
		[this, &stagedSharedSimulations, &initialGameplayGeneration](
			const WORLD_ID worldId)
		{
			auto simulation = std::make_shared<CGameRoom>(
				worldId, initialGameplayGeneration, nullptr, m_SnapshotJobs);
			if (nullptr == simulation || !simulation->Is_Ready())
			{
				std::cerr << "World simulation failed to initialize. World="
					<< static_cast<unsigned>(worldId) << ", Status="
					<< (nullptr == simulation ?
						"Simulation allocation failed" : simulation->Get_Status())
					<< '\n';
				return false;
			}
			return stagedSharedSimulations.emplace(
				worldId, std::move(simulation)).second;
		};

	if (!stageSharedSimulation(WORLD_ID::BERN) ||
		!stageSharedSimulation(WORLD_ID::VALTAN_ARENA) ||
		!stageSharedSimulation(WORLD_ID::TRAINING_GROUND) ||
		!stageSharedSimulation(WORLD_ID::KAKULSAYDON_ARENA) ||
		!stageSharedSimulation(WORLD_ID::MAHARAKA) ||
		!stageSharedSimulation(WORLD_ID::COLOSSEUM))
	{
		return 1;
	}

	// Character Select uses the same Server gameplay runtime as every other
	// world, but each admitted session receives a private simulation instance.
	// Construct one temporary instance here to retain startup fail-fast validation.
	{
		const auto validationSimulation =
			std::make_shared<CGameRoom>(
				WORLD_ID::CHARACTER_SELECT_ARENA, initialGameplayGeneration, nullptr, m_SnapshotJobs);
		if (nullptr == validationSimulation || !validationSimulation->Is_Ready())
		{
			std::cerr << "Character Select simulation failed to initialize. Status="
				<< (nullptr == validationSimulation ?
					"Simulation allocation failed" :
					validationSimulation->Get_Status())
				<< '\n';
			return 1;
		}
	}

	{
		std::scoped_lock lock{ m_SessionsMutex };
		m_SharedGameRooms = std::move(stagedSharedSimulations);
		m_pActiveGameplayGeneration = std::move(initialGameplayGeneration);
		m_ActiveGameplayBootstrapContentRevision =
			initialRuntimeGeneration.BootstrapContentRevision;
		m_ActiveNonValtanGameplayRevision =
			initialRuntimeGeneration.NonValtanGameplayRevision;
		m_isActiveGameplayGenerationFromCandidate =
			initialGenerationIsCandidate;
#ifdef _DEBUG
		m_isRuntimeActivePersistenceEnabled = true;
#else
		m_isRuntimeActivePersistenceEnabled = false;
#endif
		m_CharacterSelectArenas.clear();
		m_ColosseumMatches.clear();
		m_GameplayBindingBySessionId.clear();
	}

    std::string numericStatus;
    if (!m_NumericBalanceStore.Initialize(m_pActiveGameplayGeneration, numericStatus))
    {
        std::cerr << "Numeric balance initialization failed: " << numericStatus << '\n';
        return 1;
    }

	if (!m_WinSockContext.Initialize())
	{
		std::cerr << "Failed to initialize WinSock 2.2\n";
		return 1;
	}
	if (0u == port || !m_TcpListener.Open(bindAddress, port))
	{
		std::cerr << "Failed to open TCP listener. Address="
			<< bindAddress << ':' << port << ", Error="
			<< m_TcpListener.Get_LastErrorCode() << '\n';
		return 1;
	}

	m_isRunning.store(true);
	m_RoomThread = std::thread(&CServerApp::Room_Loop, this);
	m_AcceptThread = std::thread(&CServerApp::Accept_Loop, this);
	const bool useHeadlessMode = headless ||
		0 == ::_isatty(::_fileno(stdin));
	std::cout << "Listening on " << bindAddress << ':' << port
		<< " with shared BERN, VALTAN_ARENA, TRAINING_GROUND, "
		<< "KAKULSAYDON_ARENA, MAHARAKA, COLOSSEUM and "
		<< "session-private CHARACTER_SELECT_ARENA simulations.";
	if (0u == automaticShutdownMilliseconds && useHeadlessMode)
	{
		std::cout << " Headless mode; terminate the process to stop.\n";
		while (m_isRunning.load())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	}
	else if (0u == automaticShutdownMilliseconds)
	{
		std::cout << " Press Enter to stop.\n";
		std::cin.get();
	}
	else
	{
		std::cout << " Smoke timeout=" << automaticShutdownMilliseconds
			<< "ms.\n";
		std::this_thread::sleep_for(
			std::chrono::milliseconds(automaticShutdownMilliseconds));
	}
	Shutdown();
	return 0;
}

void LostArk::Server::CServerApp::Accept_Loop()
{
	while (m_isRunning.load())
	{
		const SOCKET clientSocket = m_TcpListener.Accept();
		if (INVALID_SOCKET == clientSocket)
		{
			if (m_isRunning.load())
			{
				std::cerr << "Accept failed. Error="
					<< m_TcpListener.Get_LastErrorCode() << '\n';
			}
			break;
		}

		const SESSION_ID sessionId = m_iNextSessionId.fetch_add(1);
		if (sessionId == INVALID_SESSION_ID)
		{
			::closesocket(clientSocket);
			continue;
		}
		auto session = std::make_shared<CClientSession>(
			sessionId,
			clientSocket,
			[this](const SESSION_ID id, const LostArk::Shared::PACKET_FRAME& frame)
			{
				On_SessionFrame(id, frame);
			},
			[this](const SESSION_ID id)
			{
				On_SessionClosed(id);
			}, m_Concurrency.Transport, m_IocpService.get());
		{
			std::scoped_lock lock{ m_SessionsMutex };
			m_Sessions.emplace(sessionId, session);
		}
		// Start is outside m_SessionsMutex and owns startup failure notification.
		// Calling On_SessionClosed here would bypass the session's once gate.
		if (!session->Start()) continue;
		std::cout << "Client connected. SessionId=" << sessionId << '\n';
	}
}

void LostArk::Server::CServerApp::Room_Loop()
{
	using namespace std::chrono;
	constexpr duration<double> FIXED_STEP_SECONDS{ 1.0 / 30.0 };
	constexpr float FIXED_DELTA_SECONDS =
		static_cast<float>(FIXED_STEP_SECONDS.count());
	const steady_clock::duration fixedStep =
		duration_cast<steady_clock::duration>(FIXED_STEP_SECONDS);
	steady_clock::time_point nextTick = steady_clock::now();
	SERVER_ROOM_SCHEDULER_METRICS schedulerMetrics;

	while (m_isRunning.load())
	{
		nextTick += fixedStep;
		Tick_GameplaySimulations(FIXED_DELTA_SECONDS, schedulerMetrics);
		Reap_ClosedSessions();
		std::this_thread::sleep_until(nextTick);
		const auto completedAt = steady_clock::now();
		schedulerMetrics.iSampleUnixMilliseconds = Current_UnixMilliseconds();
		schedulerMetrics.iPreviousLoopLatenessMicroseconds = completedAt > nextTick ?
			static_cast<std::uint64_t>(duration_cast<microseconds>(completedAt - nextTick).count()) : 0u;
		schedulerMetrics.iMaximumLoopLatenessMicroseconds = (std::max)(
			schedulerMetrics.iMaximumLoopLatenessMicroseconds,
			schedulerMetrics.iPreviousLoopLatenessMicroseconds);
		if (completedAt > nextTick + fixedStep)
		{
			nextTick = steady_clock::now();
			++schedulerMetrics.iScheduleResetCount;
		}
	}
	Tick_GameplaySimulations(FIXED_DELTA_SECONDS, schedulerMetrics);
	Reap_ClosedSessions();
}

void LostArk::Server::CServerApp::On_SessionFrame(
	const SESSION_ID sessionId,
	const LostArk::Shared::PACKET_FRAME& frame)
{
	using namespace LostArk::Shared;

	CPacketReader reader{ frame.Payload };
	const auto closeMalformedPayload =
		[this, sessionId](const std::string_view packetName)
		{
			const std::string context = "Malformed payload or trailing bytes for " +
				std::string{ packetName };
			Request_SessionClose(
				sessionId,
				SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED,
				WSAEINVAL,
				context);
		};
    if (frame.ePacketType == PACKET_TYPE::C2S_BALANCE_QUERY || frame.ePacketType == PACKET_TYPE::C2S_BALANCE_PATCH)
    {
        SERVER_CONTROL_EVENT event;
        event.iSessionId = sessionId;
        const bool query = frame.ePacketType == PACKET_TYPE::C2S_BALANCE_QUERY;
        event.eKind = query ? SERVER_CONTROL_EVENT_KIND::BALANCE_QUERY : SERVER_CONTROL_EVENT_KIND::BALANCE_PATCH;
        if (!(query ? Read_Message(reader, event.BalanceQuery) : Read_Message(reader, event.BalancePatch)) || reader.Get_RemainingSize())
        { closeMalformedPayload(query ? "C2S_BALANCE_QUERY" : "C2S_BALANCE_PATCH"); return; }
        if (!Queue_ServerControlEvent(std::move(event))) Request_SessionClose(sessionId);
        return;
    }
	if (frame.ePacketType == PACKET_TYPE::C2S_ENTER_WORLD)
	{
		CPacketReader entryPrefixReader{ frame.Payload };
		std::uint16_t receivedProtocolVersion = 0u;
		std::uint16_t receivedWorldId = 0u;
		if (!entryPrefixReader.Read_U16(receivedProtocolVersion) ||
			!entryPrefixReader.Read_U16(receivedWorldId))
		{
			Request_SessionClose(
				sessionId,
				SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED,
				WSAEMSGSIZE,
				"C2S_ENTER_WORLD is truncated before protocol/world prefix");
			return;
		}
		if (receivedProtocolVersion != NETWORK_PROTOCOL_VERSION)
		{
			const std::string context =
				"C2S_ENTER_WORLD protocol mismatch: expected=" +
				std::to_string(NETWORK_PROTOCOL_VERSION) + ", received=" +
				std::to_string(receivedProtocolVersion);
			Request_SessionClose(
				sessionId,
				SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED,
				WSAEPROTONOSUPPORT,
				context);
			return;
		}
		if (!Is_Known_World_Id(static_cast<WORLD_ID>(receivedWorldId)))
		{
			const std::string context =
				"C2S_ENTER_WORLD contains unknown world id=" +
				std::to_string(receivedWorldId);
			Request_SessionClose(
				sessionId,
				SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED,
				WSAEINVAL,
				context);
			return;
		}

		C2S_ENTER_WORLD enterWorld{};
		if (!Read_Message(reader, enterWorld) ||
			0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(
				sessionId,
				SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED,
				WSAEPROTONOSUPPORT,
				"C2S_ENTER_WORLD decode or trailing-byte validation failed");
			return;
		}
        std::unique_lock numericAdmissionLock{m_DataRevisionAdmissionMutex};
        if (m_NumericAdmissionPaused)
        {
            SERVER_CONTROL_EVENT event;
            event.eKind = SERVER_CONTROL_EVENT_KIND::BALANCE_DEFERRED_ENTRY;
            event.iSessionId = sessionId; event.DeferredEntry = frame;
            if (!Queue_ServerControlEvent(std::move(event))) Request_SessionClose(sessionId);
            return;
        }
		const std::shared_ptr<CGameRoom> targetSimulation =
			Acquire_EntrySimulation(sessionId, enterWorld.eWorldId);
		if (nullptr == targetSimulation)
		{
			Request_SessionClose(
				sessionId,
				SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
				0,
				"entry simulation acquisition failed");
			return;
		}
		SESSION_DIAGNOSTIC_REASON failureReason =
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED;
		int nativeErrorCode = 0;
		std::string failureContext;
		if (!Bind_AndEnqueueEntry(
			sessionId,
			enterWorld.eWorldId,
			targetSimulation,
			std::move(enterWorld),
			failureReason,
			nativeErrorCode,
			failureContext))
		{
			Request_SessionClose(
				sessionId,
				failureReason,
				nativeErrorCode,
				failureContext);
		}
		return;
	}

	ROOM_COMMAND command{};
	command.iSessionId = sessionId;
	if (frame.ePacketType == PACKET_TYPE::C2S_MOVE)
	{
		C2S_MOVE move{};
		if (!Read_Message(reader, move) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_MOVE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::MOVE;
		command.Move = move;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_SKILL)
	{
		C2S_USE_SKILL useSkill{};
		if (!Read_Message(reader, useSkill) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_USE_SKILL");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
		command.UseSkill = useSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RELEASE_SKILL)
	{
		C2S_RELEASE_SKILL releaseSkill{};
		if (!Read_Message(reader, releaseSkill) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_RELEASE_SKILL");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::RELEASE_SKILL;
		command.ReleaseSkill = releaseSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_UPDATE_SKILL_AIM)
	{
		C2S_UPDATE_SKILL_AIM updateSkillAim{};
		if (!Read_Message(reader, updateSkillAim) ||
			0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_UPDATE_SKILL_AIM");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM;
		command.UpdateSkillAim = updateSkillAim;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_ESTHER_SKILL)
	{
		C2S_USE_ESTHER_SKILL useEstherSkill{};
		if (!Read_Message(reader, useEstherSkill) ||
			0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_USE_ESTHER_SKILL");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_ESTHER_SKILL;
		command.UseEstherSkill = useEstherSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_SQUAREHOLE)
	{
		C2S_USE_SQUAREHOLE useSquareHole{};
		if (!Read_Message(reader, useSquareHole) ||
			0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_USE_SQUAREHOLE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_SQUAREHOLE;
		command.UseSquareHole = useSquareHole;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_REVIVE_PLAYER)
	{
		C2S_REVIVE_PLAYER revivePlayer{};
		if (!Read_Message(reader, revivePlayer) ||
			0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_REVIVE_PLAYER");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::REVIVE_PLAYER;
		command.RevivePlayer = revivePlayer;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_KILL_GATE_BOSSES)
	{
		C2S_DEBUG_KILL_GATE_BOSSES request{};
		if (!Read_Message(reader, request) || reader.Get_RemainingSize())
		{ closeMalformedPayload("C2S_DEBUG_KILL_GATE_BOSSES"); return; }
		command.eType = ROOM_COMMAND_TYPE::DEBUG_KILL_GATE_BOSSES;
		command.DebugKillGateBosses = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SET_COOLDOWN_MODE)
	{
		C2S_SET_COOLDOWN_MODE request{};
		if (!Read_Message(reader, request) || reader.Get_RemainingSize())
		{ closeMalformedPayload("C2S_SET_COOLDOWN_MODE"); return; }
		command.eType = ROOM_COMMAND_TYPE::SET_COOLDOWN_MODE;
		command.SetCooldownMode = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_KILL_SELF)
	{
		C2S_DEBUG_KILL_SELF debugKillSelf{};
		if (!Read_Message(reader, debugKillSelf) ||
			0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_KILL_SELF");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_KILL_SELF;
		command.DebugKillSelf = debugKillSelf;
	}
	else if (frame.ePacketType ==
		PACKET_TYPE::C2S_DEBUG_ENTER_KAKULSAYDON_ARENA)
	{
		C2S_DEBUG_ENTER_KAKULSAYDON_ARENA request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_ENTER_KAKULSAYDON_ARENA");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_ENTER_KAKULSAYDON_ARENA;
		command.DebugEnterKakulSaydonArena = request;
	}
	else if (frame.ePacketType ==
		PACKET_TYPE::C2S_DEBUG_TELEPORT_TO_PLACEMENT)
	{
		C2S_DEBUG_TELEPORT_TO_PLACEMENT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_TELEPORT_TO_PLACEMENT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_TELEPORT_TO_PLACEMENT;
		command.DebugTeleportToPlacement = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_TELEPORT_TO_POSITION)
	{
		C2S_DEBUG_TELEPORT_TO_POSITION request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_TELEPORT_TO_POSITION");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_TELEPORT_TO_POSITION;
		command.DebugTeleportToPosition = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_MARIO_JUMP)
	{
		C2S_DEBUG_MARIO_JUMP request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_MARIO_JUMP");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_MARIO_JUMP;
		command.DebugMarioJump = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_MARIO_RETURN)
	{
		C2S_MARIO_RETURN request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_MARIO_RETURN");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::MARIO_RETURN;
		command.MarioReturn = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_MARIO_MOVE)
	{
		C2S_MARIO_MOVE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_MARIO_MOVE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::MARIO_MOVE;
		command.MarioMove = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_CHANGE_CHARACTER_CLASS)
	{
		C2S_CHANGE_CHARACTER_CLASS request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_CHANGE_CHARACTER_CLASS");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::CHANGE_CHARACTER_CLASS;
		command.ChangeCharacterClass = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_BINGO_FILL)
	{
		C2S_DEBUG_BINGO_FILL request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_BINGO_FILL");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_BINGO_FILL;
		command.DebugBingoFill = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_BINGO_BOMB)
	{
		C2S_DEBUG_BINGO_BOMB request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_BINGO_BOMB");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_BINGO_BOMB;
		command.DebugBingoBomb = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_BINGO_HAMMER)
	{
		C2S_DEBUG_BINGO_HAMMER request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_BINGO_HAMMER");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_BINGO_HAMMER;
		command.DebugBingoHammer = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_RESUMMON_WAVE_MONSTERS)
	{
		C2S_DEBUG_RESUMMON_WAVE_MONSTERS request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_RESUMMON_WAVE_MONSTERS");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_RESUMMON_WAVE_MONSTERS;
		command.DebugResummonWaveMonsters = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_USE_ESTHER)
	{
		C2S_DEBUG_USE_ESTHER request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_USE_ESTHER");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_USE_ESTHER;
		command.DebugUseEsther = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_SET_MADNESS_FORM)
	{
		C2S_DEBUG_SET_MADNESS_FORM request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_SET_MADNESS_FORM");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_SET_MADNESS_FORM;
		command.DebugSetMadnessForm = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SET_VEHICLE_RIDING)
	{
		C2S_SET_VEHICLE_RIDING request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_SET_VEHICLE_RIDING");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SET_VEHICLE_RIDING;
		command.SetVehicleRiding = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SET_HONOR_TITLE)
	{
		C2S_SET_HONOR_TITLE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_SET_HONOR_TITLE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SET_HONOR_TITLE;
		command.SetHonorTitle = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_INTERACTION_SLOT)
	{
		C2S_INTERACTION_SLOT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_INTERACTION_SLOT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::INTERACTION_SLOT;
		command.InteractionSlot = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_SET_KOUKU_HUD_MODE)
	{
		C2S_DEBUG_SET_KOUKU_HUD_MODE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_SET_KOUKU_HUD_MODE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_SET_KOUKU_HUD_MODE;
		command.DebugSetKoukuHudMode = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY)
	{
		C2S_SPAWN_WORLD_ENTITY request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_SPAWN_WORLD_ENTITY");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SPAWN_WORLD_ENTITY;
		command.SpawnWorldEntity = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_VALTAN_AUDITION_REQUEST)
	{
		C2S_VALTAN_AUDITION_REQUEST request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_VALTAN_AUDITION_REQUEST");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::VALTAN_AUDITION;
		command.ValtanAudition = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST)
	{
		C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request{};
		if (!Read_Message(reader, request) || reader.Get_RemainingSize() != 0u)
		{ closeMalformedPayload("C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST"); return; }
		command.eType = ROOM_COMMAND_TYPE::KOUKUSAYDON_RAID; command.KoukuSaydonRaid = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK)
	{
		C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK chunk;
		if (!Read_Message(reader, chunk) || reader.Get_RemainingSize() != 0u)
		{ closeMalformedPayload("C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK"); return; }
		command.eType = ROOM_COMMAND_TYPE::KOUKUSAYDON_DRAFT_CHUNK;
		command.KoukuSaydonDraftChunk = std::move(chunk);
	}
	else if (frame.ePacketType ==
		PACKET_TYPE::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST)
	{
		C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload(
				"C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::KOUKUSAYDON_PATTERN_AUDITION;
		command.KoukuSaydonPatternAudition = std::move(request);
	}
	else if (frame.ePacketType ==
		PACKET_TYPE::C2S_DEBUG_VALTAN_PATTERN_FLOW_START)
	{
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_VALTAN_PATTERN_FLOW_START");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::VALTAN_PATTERN_FLOW_START;
		command.ValtanPatternFlowStart = std::move(request);
	}
	else if (frame.ePacketType ==
		PACKET_TYPE::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT)
	{
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload(
				"C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT");
			return;
		}
		command.eType =
			ROOM_COMMAND_TYPE::VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT;
		command.ValtanPatternFlowStopAfterCurrent = std::move(request);
	}
	else if (frame.ePacketType ==
		PACKET_TYPE::C2S_DATA_REVISION_PREPARE_REQUEST)
	{
		C2S_DATA_REVISION_PREPARE_REQUEST request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DATA_REVISION_PREPARE_REQUEST");
			return;
		}

		std::shared_ptr<CClientSession> session;
		std::shared_ptr<CGameRoom> simulation;
		std::shared_ptr<const CGameplayCatalog> activeGeneration;
		GameplayDataRevision activeBootstrapContentRevision{};
		GameplayDataRevision activeNonValtanGameplayRevision{};
		{
			std::scoped_lock lock{ m_SessionsMutex };
			const auto sessionIter = m_Sessions.find(sessionId);
			const auto bindingIter =
				m_GameplayBindingBySessionId.find(sessionId);
			if (m_Sessions.end() != sessionIter)
				session = sessionIter->second;
			if (m_GameplayBindingBySessionId.end() != bindingIter)
				simulation = bindingIter->second.pSimulation;
			activeGeneration = m_pActiveGameplayGeneration;
			activeBootstrapContentRevision =
				m_ActiveGameplayBootstrapContentRevision;
			activeNonValtanGameplayRevision =
				m_ActiveNonValtanGameplayRevision;
		}
		if (nullptr == session || nullptr == simulation ||
			nullptr == activeGeneration)
		{
			Request_SessionClose(sessionId);
			return;
		}
		if (request.CandidateRevision == activeGeneration->Get_ActiveRevision())
		{
			/* A retried request may carry the old base after the original 2PC
			   committed. The wire forbids ABORTED when candidate==active, so the
			   only truthful typed terminal result is idempotent COMMITTED. */
			if (!Send_DataRevisionResult(session, request,
				DATA_REVISION_RESULT::COMMITTED,
				activeGeneration->Get_ActiveRevision(), {}))
				Request_SessionClose(sessionId);
			return;
		}
#ifndef _DEBUG
		if (!Send_DataRevisionResult(session, request,
			DATA_REVISION_RESULT::ABORTED,
			activeGeneration->Get_ActiveRevision(),
			"Release Server rejects gameplay data revision activation"))
		{
			Request_SessionClose(sessionId);
		}
		return;
#else
		if (WORLD_ID::VALTAN_ARENA != simulation->Get_WorldId() ||
			request.BaseRevision != activeGeneration->Get_ActiveRevision())
		{
			if (!Send_DataRevisionResult(session, request,
				DATA_REVISION_RESULT::ABORTED,
				activeGeneration->Get_ActiveRevision(),
				WORLD_ID::VALTAN_ARENA != simulation->Get_WorldId() ?
					"Data revision requester is not bound to Valtan Arena" :
					"Data revision base does not match process active generation"))
				Request_SessionClose(sessionId);
			return;
		}
		std::shared_ptr<const CGameplayCatalog> candidateGeneration;
		GameplayDataRevision candidateBootstrapContentRevision{};
		GameplayDataRevision candidateNonValtanGameplayRevision{};
		std::string admissionStatus;
		{
			std::scoped_lock admissionLock{ m_DataRevisionAdmissionMutex };
			if (!Admit_ValtanCandidateGeneration(
				request, activeNonValtanGameplayRevision, candidateGeneration,
				candidateBootstrapContentRevision,
				candidateNonValtanGameplayRevision, admissionStatus) ||
				!Validate_ValtanHotReloadBaseProfile(
					activeGeneration->Find_Boss("BOSS_VALTAN"),
					nullptr == candidateGeneration ? nullptr :
						candidateGeneration->Find_Boss("BOSS_VALTAN"),
					admissionStatus))
			{
				candidateGeneration.reset();
			}
		}
		if (nullptr == candidateGeneration)
		{
			if (!Send_DataRevisionResult(session, request,
				DATA_REVISION_RESULT::ABORTED,
				activeGeneration->Get_ActiveRevision(),
				admissionStatus.empty() ?
					"Candidate admission failed closed" : admissionStatus))
				Request_SessionClose(sessionId);
			return;
		}
		SERVER_CONTROL_EVENT event{};
		event.eKind = SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST;
		event.iSessionId = sessionId;
		event.RevisionRequest = request;
		event.pCandidateGeneration = std::move(candidateGeneration);
		event.BaseBootstrapContentRevision =
			activeBootstrapContentRevision;
		event.CandidateBootstrapContentRevision =
			candidateBootstrapContentRevision;
		event.BaseNonValtanGameplayRevision =
			activeNonValtanGameplayRevision;
		event.CandidateNonValtanGameplayRevision =
			candidateNonValtanGameplayRevision;
		if (!Queue_ServerControlEvent(std::move(event)))
		{
			if (!Send_DataRevisionResult(session, request,
				DATA_REVISION_RESULT::ABORTED,
				activeGeneration->Get_ActiveRevision(),
				"Server control ingress is full"))
				Request_SessionClose(sessionId);
		}
		return;
#endif
	}
	else if (frame.ePacketType ==
		PACKET_TYPE::C2S_DATA_REVISION_PREPARE_RESPONSE)
	{
		C2S_DATA_REVISION_PREPARE_RESPONSE response{};
		if (!Read_Message(reader, response) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DATA_REVISION_PREPARE_RESPONSE");
			return;
		}
#ifdef _DEBUG
		SERVER_CONTROL_EVENT event{};
		event.eKind = SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE;
		event.iSessionId = sessionId;
		event.RevisionResponse = std::move(response);
		if (!Queue_ServerControlEvent(std::move(event)))
			Request_SessionClose(sessionId);
#else
		Request_SessionClose(sessionId);
#endif
		return;
	}
	else if (frame.ePacketType ==
		PACKET_TYPE::C2S_VALTAN_DECISION_TRACE_QUERY)
	{
		C2S_VALTAN_DECISION_TRACE_QUERY request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_VALTAN_DECISION_TRACE_QUERY");
			return;
		}
#ifdef _DEBUG
		SERVER_CONTROL_EVENT event{};
		event.eKind = SERVER_CONTROL_EVENT_KIND::VALTAN_DECISION_TRACE_QUERY;
		event.iSessionId = sessionId;
		event.DecisionTraceQuery = std::move(request);
		if (!Queue_ServerControlEvent(std::move(event)))
			Request_SessionClose(sessionId);
#else
		std::shared_ptr<CClientSession> session;
		{
			std::scoped_lock lock{ m_SessionsMutex };
			const auto iter = m_Sessions.find(sessionId);
			if (m_Sessions.end() != iter) session = iter->second;
		}
		S2C_VALTAN_DECISION_TRACE_RESPONSE response{};
		response.iRequestSequence = request.iRequestSequence;
		response.strBossPlacementId = request.strBossPlacementId;
		response.eResult =
			VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_RELEASE_BUILD;
		CPacketWriter writer;
		if (nullptr == session || !Write_Message(writer, response) ||
			!session->Send_Frame(PACKET_TYPE::S2C_VALTAN_DECISION_TRACE_RESPONSE,
				writer.Get_Buffer()))
			Request_SessionClose(sessionId);
#endif
		return;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_GIVE_ITEM)
	{
		C2S_DEBUG_GIVE_ITEM request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_GIVE_ITEM");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_GIVE_ITEM;
		command.DebugGiveItem = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_ITEM)
	{
		C2S_USE_ITEM request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_USE_ITEM");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_ITEM;
		command.UseItem = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SET_EQUIPMENT)
	{
		C2S_SET_EQUIPMENT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_SET_EQUIPMENT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SET_EQUIPMENT;
		command.SetEquipment = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_REPAIR_EQUIPMENT)
	{
		C2S_REPAIR_EQUIPMENT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_REPAIR_EQUIPMENT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::REPAIR_EQUIPMENT;
		command.RepairEquipment = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_BUY_ITEMS)
	{
		C2S_BUY_ITEMS request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_BUY_ITEMS");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::BUY_ITEMS;
		command.BuyItems = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RESTORE_CHARACTER)
	{
		C2S_RESTORE_CHARACTER request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_RESTORE_CHARACTER");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::RESTORE_CHARACTER;
		command.RestoreCharacter = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_UPGRADE_EQUIPMENT)
	{
		C2S_UPGRADE_EQUIPMENT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{ closeMalformedPayload("C2S_UPGRADE_EQUIPMENT"); return; }
		command.eType = ROOM_COMMAND_TYPE::UPGRADE_EQUIPMENT;
		command.UpgradeEquipment = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_CAPTURE_CHARACTER)
	{
		C2S_CAPTURE_CHARACTER request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{ closeMalformedPayload("C2S_CAPTURE_CHARACTER"); return; }
		command.eType = ROOM_COMMAND_TYPE::CAPTURE_CHARACTER;
		command.CaptureCharacter = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DESPAWN_ALL_WORLD_ENTITIES)
	{
		C2S_DESPAWN_ALL_WORLD_ENTITIES request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DESPAWN_ALL_WORLD_ENTITIES");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DESPAWN_ALL_WORLD_ENTITIES;
		command.DespawnAllWorldEntities = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_CONFIRM_NPC_ENTRY)
	{
		C2S_CONFIRM_NPC_ENTRY request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_CONFIRM_NPC_ENTRY");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::CONFIRM_NPC_ENTRY;
		command.ConfirmNpcEntry = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_COLOSSEUM_QUEUE_JOIN)
	{
		C2S_COLOSSEUM_QUEUE_JOIN request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_COLOSSEUM_QUEUE_JOIN");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::COLOSSEUM_QUEUE_JOIN;
		command.ColosseumQueueJoin = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_COLOSSEUM_RECRUIT)
	{
		C2S_COLOSSEUM_RECRUIT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_COLOSSEUM_RECRUIT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::COLOSSEUM_RECRUIT;
		command.ColosseumRecruit = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_COLOSSEUM_QUEUE_LEAVE)
	{
		C2S_COLOSSEUM_QUEUE_LEAVE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_COLOSSEUM_QUEUE_LEAVE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::COLOSSEUM_QUEUE_LEAVE;
		command.ColosseumQueueLeave = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_COLOSSEUM_LOAD_READY)
	{
		if (!Read_Message(reader, command.ColosseumLoadReady) || reader.Get_RemainingSize())
		{ closeMalformedPayload("C2S_COLOSSEUM_LOAD_READY"); return; }
		command.eType = ROOM_COMMAND_TYPE::COLOSSEUM_LOAD_READY;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_COLOSSEUM_RETURN)
	{
		if (!Read_Message(reader, command.ColosseumReturn) || reader.Get_RemainingSize())
		{ closeMalformedPayload("C2S_COLOSSEUM_RETURN"); return; }
		command.eType = ROOM_COMMAND_TYPE::COLOSSEUM_RETURN;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_INTERACT_TRIGGER)
	{
		C2S_INTERACT_TRIGGER request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_INTERACT_TRIGGER");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::INTERACT_TRIGGER;
		command.InteractTrigger = request;
	}
    else if (frame.ePacketType == PACKET_TYPE::C2S_MAHARAKA_AI_TUNING)
    {
        C2S_MAHARAKA_AI_TUNING request{};
        if (!Read_Message(reader, request) || reader.Get_RemainingSize())
        { closeMalformedPayload("C2S_MAHARAKA_AI_TUNING"); return; }
        command.eType = ROOM_COMMAND_TYPE::MAHARAKA_AI_TUNING;
        command.MaharakaAITuning = std::move(request);
    }
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_WORLD_PLAYBACK)
	{
		C2S_DEBUG_WORLD_PLAYBACK request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{ closeMalformedPayload("C2S_DEBUG_WORLD_PLAYBACK"); return; }
		command.eType = ROOM_COMMAND_TYPE::DEBUG_WORLD_PLAYBACK;
		command.DebugWorldPlayback = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RETURN_TO_BERN)
	{
		C2S_RETURN_TO_BERN request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_RETURN_TO_BERN");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::RETURN_TO_BERN;
		command.ReturnToBern = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_GUIDE_CONTROL)
	{
		C2S_GUIDE_CONTROL request{};
		if (!Read_Message(reader, request) || reader.Get_RemainingSize() != 0u)
		{ closeMalformedPayload("C2S_GUIDE_CONTROL"); return; }
		command.eType = ROOM_COMMAND_TYPE::GUIDE_CONTROL;
		command.GuideControl = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_PARTY_INVITE)
	{
		C2S_PARTY_INVITE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::PARTY_INVITE;
		command.PartyInvite = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_PARTY_INVITE_RESPOND)
	{
		C2S_PARTY_INVITE_RESPOND request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::PARTY_INVITE_RESPOND;
		command.PartyInviteRespond = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RAID_ENTRY_PROPOSE)
	{
		C2S_RAID_ENTRY_PROPOSE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::RAID_ENTRY_PROPOSE;
		command.RaidEntryPropose = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RAID_ENTRY_RESPOND)
	{
		C2S_RAID_ENTRY_RESPOND request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::RAID_ENTRY_RESPOND;
		command.RaidEntryRespond = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_GATE_PROGRESS_PROPOSE)
	{
		C2S_GATE_PROGRESS_PROPOSE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::GATE_PROGRESS_PROPOSE;
		command.GateProgressPropose = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_GATE_PROGRESS_RESPOND)
	{
		C2S_GATE_PROGRESS_RESPOND request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::GATE_PROGRESS_RESPOND;
		command.GateProgressRespond = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_ROOM_PING)
	{
		C2S_ROOM_PING request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::ROOM_PING;
		command.RoomPing = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_CHAT)
	{
		C2S_CHAT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::CHAT;
		command.Chat = request;
	}
	else
	{
		Request_SessionClose(
			sessionId,
			SESSION_DIAGNOSTIC_REASON::SERVER_UNKNOWN_PACKET,
			WSAEPROTONOSUPPORT,
			"packet type is not a supported Client command");
		return;
	}

	std::string enqueueContext;
	const ROOM_COMMAND_ENQUEUE_RESULT enqueueResult =
		Enqueue_AssignedCommand(
			sessionId, std::move(command), enqueueContext);
	if (Is_AcceptedRoomCommandEnqueueResult(enqueueResult) ||
		ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_PENDING_CLEANUP == enqueueResult)
	{
		return;
	}

	if (ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY ==
		enqueueResult)
	{
		Request_SessionClose(
			sessionId,
			SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW,
			WSAENOBUFS,
			"assigned room reliable ingress reached its hard bound; " +
				enqueueContext);
		return;
	}
	if (ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY == enqueueResult)
	{
		Request_SessionClose(
			sessionId,
			SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED,
			WSAESHUTDOWN,
			"assigned room stopped after a runtime failure; " +
				enqueueContext);
		return;
	}
	Request_SessionClose(
		sessionId,
		ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND == enqueueResult ?
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED :
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
		ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_BINDING_MISSING == enqueueResult ?
			WSAENOTCONN : WSAESHUTDOWN,
		"assigned room command admission failed; " + enqueueContext);
}

void LostArk::Server::CServerApp::On_SessionClosed(const SESSION_ID sessionId)
{
	std::shared_ptr<CClientSession> session;
	LostArk::Shared::WORLD_ID worldId = LostArk::Shared::WORLD_ID::END;
	bool wasBound = false;
	bool leaveEnqueued = false;
	ROOM_COMMAND_ENQUEUE_RESULT leaveEnqueueResult =
		ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_BINDING_MISSING;
	std::string leaveEnqueueContext;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		const auto sessionIter = m_Sessions.find(sessionId);
		if (sessionIter != m_Sessions.end())
			session = sessionIter->second;
		const auto bindingIter =
			m_GameplayBindingBySessionId.find(sessionId);
		if (bindingIter != m_GameplayBindingBySessionId.end())
		{
			wasBound = true;
			worldId = bindingIter->second.eWorldId;
			if (nullptr != bindingIter->second.pSimulation)
			{
				ROOM_COMMAND command{};
				command.eType = ROOM_COMMAND_TYPE::LEAVE;
				command.iSessionId = sessionId;
				command.eLeaveReason =
					LostArk::Shared::PLAYER_DESPAWN_REASON::DISCONNECTED;
				leaveEnqueueResult = bindingIter->second.pSimulation->
					Enqueue_Detailed(std::move(command));
				leaveEnqueued = Is_AcceptedRoomCommandEnqueueResult(
					leaveEnqueueResult);
				leaveEnqueueContext = bindingIter->second.pSimulation->
					Describe_EnqueueResult(leaveEnqueueResult);
			}
			m_GameplayBindingBySessionId.erase(bindingIter);
		}
	}

	if (nullptr != session)
	{
		CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
			session->Get_CloseDiagnostic();
		if (LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE ==
			diagnostic.eReason)
		{
			session->Request_Close(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
				0,
				"close callback arrived without a terminal classification");
			diagnostic = session->Get_CloseDiagnostic();
		}
		const CLIENT_SESSION_OUTBOUND_METRICS metrics =
			session->Get_OutboundMetrics();
		const std::string json = Build_ServerSessionDiagnosticJson(
			sessionId,
			session->Get_PeerEndpoint(),
			diagnostic,
			worldId,
			session->Get_PlayerId(),
			wasBound,
			leaveEnqueued,
			metrics);
		std::string jsonValidationStatus;
		const bool isJsonValid = Validate_ServerSessionDiagnosticJson(
			json, jsonValidationStatus);
		std::scoped_lock diagnosticLock{ m_SessionDiagnosticLogMutex };
		if (!isJsonValid)
		{
			std::cerr << "[SessionDiagnostic][INVALID_JSON] SessionId=" <<
				sessionId << ", Status=" << jsonValidationStatus << '\n';
		}
		std::cout << "[SessionDiagnostic] " << json << '\n';
		if (wasBound && !leaveEnqueued &&
			ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY !=
				leaveEnqueueResult)
		{
			std::cerr <<
				"[SessionDiagnostic][LEAVE_ENQUEUE_INVARIANT_FAILED] SessionId="
				<< sessionId << ", World=" <<
				static_cast<std::uint16_t>(worldId) <<
				". A live binding unexpectedly rejected its dedicated cleanup "
				"queue; inspect room readiness/retirement invariants. " <<
				leaveEnqueueContext << '\n';
		}
		const std::filesystem::path logPath =
			Resolve_ServerSessionDiagnosticPath();
		if (!logPath.empty())
		{
			std::ofstream log{ logPath, std::ios::binary | std::ios::app };
			if (log)
				log << json << '\n';
			else
				std::cerr << "Session diagnostic log open failed. Path="
					<< logPath.string() << '\n';
		}
		else
		{
			std::cerr << "Session diagnostic log path resolution failed.\n";
		}
	}

	// A private Character Select simulation owns the queued LEAVE until the
	// room thread consumes it, seals the empty queue, and retires the arena.
	{
		std::scoped_lock lock{ m_ClosedSessionMutex };
		m_ClosedSessionIds.push_back(sessionId);
	}
	SERVER_CONTROL_EVENT controlEvent{};
	controlEvent.eKind = SERVER_CONTROL_EVENT_KIND::SESSION_DISCONNECTED;
	controlEvent.iSessionId = sessionId;
	(void)Queue_ServerControlEvent(std::move(controlEvent));
}

void LostArk::Server::CServerApp::Request_SessionClose(
	const SESSION_ID sessionId,
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const int nativeErrorCode,
	const std::string_view context)
{
	std::shared_ptr<CClientSession> session;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		const auto iter = m_Sessions.find(sessionId);
		if (iter != m_Sessions.end())
			session = iter->second;
	}
	if (nullptr != session)
		session->Request_Close(reason, nativeErrorCode, context);
}

void LostArk::Server::CServerApp::Reap_ClosedSessions()
{
	std::deque<SESSION_ID> closedIds;
	{
		std::scoped_lock lock{ m_ClosedSessionMutex };
		closedIds.swap(m_ClosedSessionIds);
	}
	for (const SESSION_ID sessionId : closedIds)
	{
		std::shared_ptr<CClientSession> session;
		{
			std::scoped_lock lock{ m_SessionsMutex };
			const auto iter = m_Sessions.find(sessionId);
			if (iter == m_Sessions.end())
				continue;
			session = std::move(iter->second);
			m_Sessions.erase(iter);
		}
		if (nullptr != session)
			session->Stop();
	}
}

std::shared_ptr<LostArk::Server::CGameRoom>
LostArk::Server::CServerApp::Acquire_EntrySimulation(
	const SESSION_ID sessionId,
	const LostArk::Shared::WORLD_ID worldId)
{
	using LostArk::Shared::WORLD_ID;

	if (!LostArk::Shared::Is_Known_World_Id(worldId))
		return nullptr;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != worldId)
		return Find_SharedSimulation(worldId);

	std::shared_ptr<const CGameplayCatalog> activeGameplayGeneration;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		if (!m_Sessions.contains(sessionId))
			return nullptr;

		const auto bindingIter =
			m_GameplayBindingBySessionId.find(sessionId);
		if (bindingIter != m_GameplayBindingBySessionId.end())
		{
			const SESSION_GAMEPLAY_BINDING& binding = bindingIter->second;
			return binding.eWorldId == worldId &&
				binding.iPrivateArenaOwnerSessionId == sessionId ?
					binding.pSimulation : nullptr;
		}
		activeGameplayGeneration = m_pActiveGameplayGeneration;
	}

	if (nullptr == activeGameplayGeneration)
		return nullptr;
	auto simulation = std::make_shared<CGameRoom>(
		worldId, activeGameplayGeneration, nullptr, m_SnapshotJobs);
	if (nullptr == simulation || !simulation->Is_Ready())
	{
		std::cerr << "Private Character Select simulation failed. SessionId="
			<< sessionId << ", Status="
			<< (nullptr == simulation ?
				"Simulation allocation failed" : simulation->Get_Status())
			<< '\n';
		return nullptr;
	}
	return simulation;
}

std::shared_ptr<LostArk::Server::CGameRoom>
LostArk::Server::CServerApp::Find_SharedSimulation(
	const LostArk::Shared::WORLD_ID worldId)
{
	std::scoped_lock lock{ m_SessionsMutex };
	const auto iter = m_SharedGameRooms.find(worldId);
	return iter == m_SharedGameRooms.end() ? nullptr : iter->second;
}

bool LostArk::Server::CServerApp::Bind_AndEnqueueEntry(
	const SESSION_ID sessionId,
	const LostArk::Shared::WORLD_ID worldId,
	const std::shared_ptr<CGameRoom>& simulation,
	LostArk::Shared::C2S_ENTER_WORLD enterWorld,
	LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outFailureReason,
	int& outNativeErrorCode,
	std::string& outFailureContext)
{
	using namespace LostArk::Shared;
	outFailureReason = SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED;
	outNativeErrorCode = WSAEINVAL;
	outFailureContext = "entry stage=bind-validation packet=C2S_ENTER_WORLD";

	if (nullptr == simulation ||
		!Is_Known_World_Id(worldId) ||
		simulation->Get_WorldId() != worldId ||
		enterWorld.eWorldId != worldId)
	{
		return false;
	}

	/* Binding, REGISTER/ENTER admission and the terminal-state recheck are one
	   transaction relative to On_SessionClosed. The shared lock order remains
	   ServerApp sessions -> room command queue. */
	std::scoped_lock lock{ m_SessionsMutex };
	const auto sessionIter = m_Sessions.find(sessionId);
	if (sessionIter == m_Sessions.end() || nullptr == sessionIter->second)
	{
		outNativeErrorCode = WSAENOTCONN;
		outFailureContext =
			"entry stage=session-lookup packet=C2S_ENTER_WORLD";
		return false;
	}
	if (sessionIter->second->Is_Closing())
	{
		outNativeErrorCode = WSAESHUTDOWN;
		outFailureContext =
			"entry stage=terminal-recheck packet=C2S_ENTER_WORLD "
			"sessionAlreadyClosing=true";
		return false;
	}
	if (nullptr == m_pActiveGameplayGeneration ||
		nullptr == simulation->Get_ActiveGameplayGeneration() ||
		simulation->Get_ActiveGameplayGeneration()->Get_ActiveRevision() !=
			m_pActiveGameplayGeneration->Get_ActiveRevision())
	{
		outFailureContext =
			"entry stage=gameplay-revision-validation "
			"packet=C2S_ENTER_WORLD";
		return false;
	}

	const SESSION_ID privateOwnerSessionId =
		WORLD_ID::CHARACTER_SELECT_ARENA == worldId ?
			sessionId : INVALID_SESSION_ID;
	const auto existingBinding =
		m_GameplayBindingBySessionId.find(sessionId);
	if (existingBinding != m_GameplayBindingBySessionId.end())
	{
		outFailureReason =
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED;
		outNativeErrorCode = WSAEALREADY;
		outFailureContext =
			"entry stage=duplicate-binding packet=C2S_ENTER_WORLD";
		return false;
	}

	bool insertedPrivateArena = false;
	if (WORLD_ID::CHARACTER_SELECT_ARENA == worldId)
	{
		const auto [arenaIter, inserted] =
			m_CharacterSelectArenas.emplace(sessionId, simulation);
		if (!inserted && arenaIter->second != simulation)
		{
			outFailureContext =
				"entry stage=private-arena-binding packet=C2S_ENTER_WORLD";
			return false;
		}
		insertedPrivateArena = inserted;
	}
	else
	{
		const auto sharedIter = m_SharedGameRooms.find(worldId);
		if (sharedIter == m_SharedGameRooms.end() ||
			sharedIter->second != simulation)
		{
			outFailureContext =
				"entry stage=shared-room-binding packet=C2S_ENTER_WORLD";
			return false;
		}
	}

	SESSION_GAMEPLAY_BINDING binding{};
	binding.eWorldId = worldId;
	binding.iPrivateArenaOwnerSessionId = privateOwnerSessionId;
	binding.pSimulation = simulation;
	const auto [bindingIter, insertedBinding] =
		m_GameplayBindingBySessionId.emplace(sessionId, std::move(binding));
	(void)bindingIter;
	if (!insertedBinding)
	{
		if (insertedPrivateArena)
			m_CharacterSelectArenas.erase(sessionId);
		outFailureContext =
			"entry stage=binding-commit packet=C2S_ENTER_WORLD";
		return false;
	}

	ROOM_COMMAND registerCommand{};
	registerCommand.eType = ROOM_COMMAND_TYPE::REGISTER_SESSION;
	registerCommand.iSessionId = sessionId;
	registerCommand.pSession = sessionIter->second;
	const ROOM_COMMAND_ENQUEUE_RESULT registerResult =
		simulation->Enqueue_Detailed(std::move(registerCommand));
	if (ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED != registerResult)
	{
		m_GameplayBindingBySessionId.erase(sessionId);
		if (insertedPrivateArena)
			m_CharacterSelectArenas.erase(sessionId);
		outFailureReason = ROOM_COMMAND_ENQUEUE_RESULT::
			REJECTED_RELIABLE_CAPACITY == registerResult ?
			SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW :
			ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY ==
				registerResult ?
			SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED :
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED;
		outNativeErrorCode = ROOM_COMMAND_ENQUEUE_RESULT::
			REJECTED_RELIABLE_CAPACITY == registerResult ?
			WSAENOBUFS : WSAESHUTDOWN;
		outFailureContext =
			"entry stage=register-ingress packet=C2S_ENTER_WORLD " +
			simulation->Describe_EnqueueResult(registerResult);
		return false;
	}

	ROOM_COMMAND enterCommand{};
	enterCommand.eType = ROOM_COMMAND_TYPE::ENTER_WORLD;
	enterCommand.iSessionId = sessionId;
	enterCommand.EnterWorld = std::move(enterWorld);
	const ROOM_COMMAND_ENQUEUE_RESULT enterResult =
		simulation->Enqueue_Detailed(std::move(enterCommand));
	if (ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED != enterResult)
	{
		ROOM_COMMAND rollbackCommand{};
		rollbackCommand.eType = ROOM_COMMAND_TYPE::LEAVE;
		rollbackCommand.iSessionId = sessionId;
		rollbackCommand.eLeaveReason = PLAYER_DESPAWN_REASON::DISCONNECTED;
		const bool rollbackEnqueued =
			simulation->Enqueue(std::move(rollbackCommand));
		m_GameplayBindingBySessionId.erase(sessionId);
		outFailureReason = ROOM_COMMAND_ENQUEUE_RESULT::
			REJECTED_RELIABLE_CAPACITY == enterResult ?
			SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW :
			ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY == enterResult ?
			SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED :
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED;
		outNativeErrorCode = ROOM_COMMAND_ENQUEUE_RESULT::
			REJECTED_RELIABLE_CAPACITY == enterResult ?
			WSAENOBUFS : WSAESHUTDOWN;
		outFailureContext =
			"entry stage=enter-ingress packet=C2S_ENTER_WORLD "
			+ simulation->Describe_EnqueueResult(enterResult) + " "
			"rollbackCleanupEnqueued=" +
			std::string{ rollbackEnqueued ? "true" : "false" };
		return false;
	}
	return true;
}

LostArk::Server::ROOM_COMMAND_ENQUEUE_RESULT
LostArk::Server::CServerApp::Enqueue_AssignedCommand(
	const SESSION_ID sessionId,
	ROOM_COMMAND command,
	std::string& outContext)
{
	outContext.clear();
	if (command.iSessionId != sessionId)
	{
		outContext = "enqueueResult=REJECTED_INVALID_COMMAND identityMismatch=true";
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND;
	}

	// Keep lookup plus Enqueue atomic with Transfer_SessionWorld. Both paths
	// use m_SessionsMutex -> CGameRoom::m_CommandMutex ordering.
	std::scoped_lock lock{ m_SessionsMutex };
	const auto iter = m_GameplayBindingBySessionId.find(sessionId);
	if (iter == m_GameplayBindingBySessionId.end() ||
		nullptr == iter->second.pSimulation)
	{
		outContext = "enqueueResult=REJECTED_BINDING_MISSING";
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_BINDING_MISSING;
	}
	const ROOM_COMMAND_ENQUEUE_RESULT result =
		iter->second.pSimulation->Enqueue_Detailed(std::move(command));
	outContext = iter->second.pSimulation->Describe_EnqueueResult(result);
	return result;
}

bool LostArk::Server::CServerApp::Queue_ServerControlEvent(
	SERVER_CONTROL_EVENT&& event)
{
	std::scoped_lock lock{ m_ServerControlMutex };
	if (m_ServerControlEvents.size() >= MAX_SERVER_CONTROL_EVENTS)
		return false;
	m_ServerControlEvents.push_back(std::move(event));
	return true;
}

bool LostArk::Server::CServerApp::Resolve_CandidateArtifactForAdmission(
	const std::filesystem::path& candidateDirectory,
	const std::string& relativePath,
	std::filesystem::path& resolvedPath,
	std::string& status)
{
	return Resolve_ExactRegularFile(
		candidateDirectory, relativePath, resolvedPath, status);
}

bool LostArk::Server::CServerApp::
Build_NonValtanGameplayRevisionForAdmission(
	const std::filesystem::path& bootstrapPath,
	LostArk::Shared::GameplayDataRevision& revision,
	std::string& status)
{
	return Build_NonValtanGameplayRevision(bootstrapPath, revision, status);
}

bool LostArk::Server::CServerApp::Hash_GameplayFileForAdmission(
	const std::filesystem::path& path,
	LostArk::Shared::GameplayDataRevision& revision,
	std::string& status)
{
	return Hash_FileSha256(path, revision, status);
}

bool LostArk::Server::CServerApp::Validate_ValtanHotReloadBaseProfile(
	const BOSS_RUNTIME_PROFILE* activeProfile,
	const BOSS_RUNTIME_PROFILE* candidateProfile,
	std::string& status)
{
	/* These values are copied into entity health, combat, and movement state at
	   spawn, so swapping only the immutable definition cannot apply them. */
	if (nullptr == activeProfile || nullptr == candidateProfile)
	{
		status = "Valtan HOT_RELOAD base profile is unavailable; "
			"ENCOUNTER_RESET required";
		return false;
	}
	if (activeProfile->iMaximumHp != candidateProfile->iMaximumHp ||
		activeProfile->iDamageReferenceHp != candidateProfile->iDamageReferenceHp ||
		activeProfile->iMaximumHealthBars !=
			candidateProfile->iMaximumHealthBars ||
		activeProfile->iAttackPower != candidateProfile->iAttackPower ||
		activeProfile->fCollisionRadius != candidateProfile->fCollisionRadius ||
		activeProfile->fEngageDistance != candidateProfile->fEngageDistance ||
		activeProfile->fMoveSpeed != candidateProfile->fMoveSpeed)
	{
		status = "Candidate changes live Valtan base fields; "
			"ENCOUNTER_RESET required";
		return false;
	}
	status.clear();
	return true;
}

bool LostArk::Server::CServerApp::Send_DataRevisionPrepare(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request)
{
	using namespace LostArk::Shared;
	if (nullptr == session) return false;
	S2C_DATA_REVISION_PREPARE prepare{};
	prepare.iTransactionSequence = request.iTransactionSequence;
	prepare.BaseRevision = request.BaseRevision;
	prepare.CandidateRevision = request.CandidateRevision;
	prepare.iRequiredPresentationLaneMask =
		request.iRequiredPresentationLaneMask;
	CPacketWriter writer;
	return Write_Message(writer, prepare) && session->Send_Frame(
		PACKET_TYPE::S2C_DATA_REVISION_PREPARE, writer.Get_Buffer());
}

bool LostArk::Server::CServerApp::Send_DataRevisionResult(
	const std::shared_ptr<CClientSession>& session,
	const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request,
	const LostArk::Shared::DATA_REVISION_RESULT result,
	const LostArk::Shared::GameplayDataRevision& activeRevision,
	std::string reason)
{
	using namespace LostArk::Shared;
	if (nullptr == session) return false;
	if (DATA_REVISION_RESULT::COMMITTED == result)
		reason.clear();
	else
	{
		if (reason.empty()) reason = "Data revision transaction aborted";
		if (reason.size() > MAX_DATA_REVISION_REASON_BYTES)
			reason.resize(MAX_DATA_REVISION_REASON_BYTES);
	}
	S2C_DATA_REVISION_RESULT message{};
	message.iTransactionSequence = request.iTransactionSequence;
	message.CandidateRevision = request.CandidateRevision;
	message.ActiveRevision = activeRevision;
	message.eResult = result;
	message.strReason = std::move(reason);
	CPacketWriter writer;
	return Write_Message(writer, message) && session->Send_Frame(
		PACKET_TYPE::S2C_DATA_REVISION_RESULT, writer.Get_Buffer());
}

void LostArk::Server::CServerApp::Process_ValtanDecisionTraceQuery(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request)
{
	using namespace LostArk::Shared;
	std::shared_ptr<CClientSession> session;
	std::shared_ptr<CGameRoom> simulation;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		const auto sessionIter = m_Sessions.find(sessionId);
		const auto bindingIter = m_GameplayBindingBySessionId.find(sessionId);
		if (m_Sessions.end() != sessionIter) session = sessionIter->second;
		if (m_GameplayBindingBySessionId.end() != bindingIter)
			simulation = bindingIter->second.pSimulation;
	}
	if (nullptr == session) return;
	S2C_VALTAN_DECISION_TRACE_RESPONSE response{};
	response.iRequestSequence = request.iRequestSequence;
	response.strBossPlacementId = request.strBossPlacementId;
	std::string status;
	if (nullptr == simulation)
		response.eResult =
			VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_WRONG_WORLD;
	else if (!simulation->Build_ValtanDecisionTraceResponse(
		request, response, status))
	{
		std::cerr << "Valtan decision trace query failed closed. SessionId="
			<< sessionId << ", Reason=" << status << '\n';
		Request_SessionClose(sessionId);
		return;
	}
	CPacketWriter writer;
	if (!Write_Message(writer, response) || !session->Send_Frame(
		PACKET_TYPE::S2C_VALTAN_DECISION_TRACE_RESPONSE, writer.Get_Buffer()))
		Request_SessionClose(sessionId);
}

bool LostArk::Server::CServerApp::Validate_DataRevisionTransactionMembership(
	std::string& status)
{
	if (!m_DataRevisionTransaction.Is_Active())
	{
		status = "Data revision transaction is inactive";
		return false;
	}
	std::scoped_lock lock{ m_SessionsMutex };
	if (nullptr == m_pActiveGameplayGeneration ||
		m_pActiveGameplayGeneration->Get_ActiveRevision() !=
			m_DataRevisionTransaction.Request.BaseRevision ||
		m_ActiveGameplayBootstrapContentRevision !=
			m_DataRevisionTransaction.BaseBootstrapContentRevision ||
		m_ActiveNonValtanGameplayRevision !=
			m_DataRevisionTransaction.BaseNonValtanGameplayRevision ||
		m_ActiveNonValtanGameplayRevision !=
			m_DataRevisionTransaction.CandidateNonValtanGameplayRevision)
	{
		status = "Process active gameplay generation changed during prepare";
		return false;
	}
	std::size_t boundParticipantCount = 0u;
	for (const auto& [sessionId, binding] : m_GameplayBindingBySessionId)
	{
		if (m_Sessions.contains(sessionId) && nullptr != binding.pSimulation)
			++boundParticipantCount;
	}
	if (boundParticipantCount !=
		m_DataRevisionTransaction.Participants.size())
	{
		status = "Bound data revision participant set changed during prepare";
		return false;
	}
	for (const DATA_REVISION_PARTICIPANT& participant :
		m_DataRevisionTransaction.Participants)
	{
		const auto sessionIter = m_Sessions.find(participant.iSessionId);
		const auto bindingIter =
			m_GameplayBindingBySessionId.find(participant.iSessionId);
		if (m_Sessions.end() == sessionIter ||
			sessionIter->second != participant.pSession ||
			m_GameplayBindingBySessionId.end() == bindingIter ||
			bindingIter->second.eWorldId != participant.eWorldId ||
			bindingIter->second.pSimulation != participant.pSimulation)
		{
			status = "Data revision participant disconnected or changed rooms";
			return false;
		}
	}
	std::set<const CGameRoom*> currentRooms;
	for (const auto& [worldId, room] : m_SharedGameRooms)
	{
		(void)worldId;
		if (nullptr != room) currentRooms.insert(room.get());
	}
	for (const auto& [sessionId, room] : m_CharacterSelectArenas)
	{
		(void)sessionId;
		if (nullptr != room) currentRooms.insert(room.get());
	}
	for (const auto& [matchId, room] : m_ColosseumMatches)
	{
		(void)matchId;
		if (nullptr != room) currentRooms.insert(room.get());
	}
	if (currentRooms.size() != m_DataRevisionTransaction.Simulations.size())
	{
		status = "Process gameplay room set changed during prepare";
		return false;
	}
	for (const auto& room : m_DataRevisionTransaction.Simulations)
	{
		if (nullptr == room || !currentRooms.contains(room.get()))
		{
			status = "Prepared gameplay room is no longer process-owned";
			return false;
		}
	}
	status.clear();
	return true;
}

void LostArk::Server::CServerApp::Abort_DataRevisionTransaction(
	std::string reason)
{
	using namespace LostArk::Shared;
	if (!m_DataRevisionTransaction.Is_Active()) return;
	DATA_REVISION_TRANSACTION transaction =
		std::move(m_DataRevisionTransaction);
	m_DataRevisionTransaction = {};
	const auto tombstoneExpiry = std::chrono::steady_clock::now() +
		std::chrono::seconds(10);
	for (const DATA_REVISION_PARTICIPANT& participant :
		transaction.Participants)
	{
		if (participant.hasResponded)
			continue;
		const auto existing = std::find_if(
			m_DataRevisionResponseTombstones.begin(),
			m_DataRevisionResponseTombstones.end(),
			[&transaction, &participant](
				const DATA_REVISION_RESPONSE_TOMBSTONE& tombstone)
			{
				return tombstone.iSessionId == participant.iSessionId &&
					tombstone.iTransactionSequence ==
						transaction.Request.iTransactionSequence &&
					tombstone.CandidateRevision ==
						transaction.Request.CandidateRevision;
			});
		if (m_DataRevisionResponseTombstones.end() != existing)
		{
			existing->ExpiresAt = tombstoneExpiry;
			continue;
		}
		if (m_DataRevisionResponseTombstones.size() >=
			MAX_DATA_REVISION_RESPONSE_TOMBSTONES)
		{
			m_DataRevisionResponseTombstones.pop_front();
		}
		m_DataRevisionResponseTombstones.push_back({
			participant.iSessionId,
			transaction.Request.iTransactionSequence,
			transaction.Request.CandidateRevision,
			transaction.Request.iRequiredPresentationLaneMask,
			tombstoneExpiry });
	}
	for (const auto& simulation : transaction.Simulations)
		if (nullptr != simulation)
			simulation->Abort_GameplayGeneration(
				transaction.Request.iTransactionSequence);
	GameplayDataRevision activeRevision = transaction.Request.BaseRevision;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		if (nullptr != m_pActiveGameplayGeneration)
			activeRevision = m_pActiveGameplayGeneration->Get_ActiveRevision();
	}
	if (reason.empty()) reason = "Data revision transaction aborted";
	for (const DATA_REVISION_PARTICIPANT& participant :
		transaction.Participants)
	{
		if (!Send_DataRevisionResult(participant.pSession, transaction.Request,
			DATA_REVISION_RESULT::ABORTED, activeRevision, reason))
			Request_SessionClose(participant.iSessionId);
	}
}

bool LostArk::Server::CServerApp::Commit_DataRevisionTransaction()
{
	using namespace LostArk::Shared;
	if (!m_DataRevisionTransaction.Is_Active()) return false;
	std::string status;
	if (!Validate_DataRevisionTransactionMembership(status))
	{
		Abort_DataRevisionTransaction(std::move(status));
		return false;
	}

	RUNTIME_ACTIVE_GAMEPLAY_GENERATION baseRuntime{};
	baseRuntime.eSource = m_isActiveGameplayGenerationFromCandidate ?
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE :
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
	baseRuntime.Revision = m_DataRevisionTransaction.Request.BaseRevision;
	baseRuntime.BootstrapContentRevision =
		m_DataRevisionTransaction.BaseBootstrapContentRevision;
	baseRuntime.NonValtanGameplayRevision =
		m_DataRevisionTransaction.BaseNonValtanGameplayRevision;
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION candidateRuntime{};
	candidateRuntime.eSource =
		RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE;
	candidateRuntime.Revision =
		m_DataRevisionTransaction.Request.CandidateRevision;
	candidateRuntime.BootstrapContentRevision =
		m_DataRevisionTransaction.CandidateBootstrapContentRevision;
	candidateRuntime.NonValtanGameplayRevision =
		m_DataRevisionTransaction.CandidateNonValtanGameplayRevision;

	bool stageSucceeded = true;
	{
		/* Freeze registration only while every room performs its allocation-free
		preflight. Durable pointer I/O happens after this lock is released. */
		std::scoped_lock lock{ m_SessionsMutex };
		if (nullptr == m_pActiveGameplayGeneration ||
			m_pActiveGameplayGeneration->Get_ActiveRevision() !=
				m_DataRevisionTransaction.Request.BaseRevision)
		{
			status = "Process active generation changed before room commit";
			stageSucceeded = false;
		}
		for (const auto& simulation : m_DataRevisionTransaction.Simulations)
		{
			if (!stageSucceeded) break;
			std::string roomStatus;
			if (nullptr == simulation ||
				!simulation->Stage_GameplayGeneration(
					m_DataRevisionTransaction.Request.iTransactionSequence,
					m_DataRevisionTransaction.Request.BaseRevision,
					m_DataRevisionTransaction.pCandidateGeneration,
					roomStatus))
			{
				status = roomStatus.empty() ?
					"A process gameplay room rejected generation stage" :
					roomStatus;
				stageSucceeded = false;
			}
		}
		if (!stageSucceeded)
			for (const auto& simulation : m_DataRevisionTransaction.Simulations)
				if (nullptr != simulation)
					simulation->Abort_GameplayGeneration(
						m_DataRevisionTransaction.Request.iTransactionSequence);
	}
	if (!stageSucceeded)
	{
		Abort_DataRevisionTransaction(std::move(status));
		return false;
	}

	const std::filesystem::path runtimeRoot =
		Resolve_RuntimeActiveGameplayRoot();
	bool durablePointerPromoted = false;
	if (m_isRuntimeActivePersistenceEnabled)
	{
		if (!Persist_RuntimeGameplayActivation(
			runtimeRoot,
			m_DataRevisionTransaction.Request.iTransactionSequence,
			baseRuntime, candidateRuntime, status))
		{
			for (const auto& simulation : m_DataRevisionTransaction.Simulations)
				if (nullptr != simulation)
					simulation->Abort_GameplayGeneration(
						m_DataRevisionTransaction.Request.iTransactionSequence);
			Abort_DataRevisionTransaction(status.empty() ?
				"Runtime active pointer promotion failed" : std::move(status));
			return false;
		}
		durablePointerPromoted = true;
	}

	bool cohortStillExact = true;
	{
		/* This is the process-wide room-tick publication boundary. Revalidate
		the full cohort after disk I/O while registration is frozen, then perform
		only bounded pointer swaps. No file or socket I/O occurs under this lock. */
		std::scoped_lock lock{ m_SessionsMutex };
		if (nullptr == m_pActiveGameplayGeneration ||
			m_pActiveGameplayGeneration->Get_ActiveRevision() !=
				m_DataRevisionTransaction.Request.BaseRevision ||
			m_ActiveGameplayBootstrapContentRevision !=
				m_DataRevisionTransaction.BaseBootstrapContentRevision ||
			m_ActiveNonValtanGameplayRevision !=
				m_DataRevisionTransaction.BaseNonValtanGameplayRevision)
		{
			cohortStillExact = false;
			status = "Process active generation changed during durable promotion";
		}
		std::size_t boundCount = 0u;
		for (const auto& [sessionId, binding] : m_GameplayBindingBySessionId)
			if (m_Sessions.contains(sessionId) && nullptr != binding.pSimulation)
				++boundCount;
		if (boundCount != m_DataRevisionTransaction.Participants.size())
		{
			cohortStillExact = false;
			status = "Bound participant set changed during durable promotion";
		}
		for (const DATA_REVISION_PARTICIPANT& participant :
			m_DataRevisionTransaction.Participants)
		{
			const auto session = m_Sessions.find(participant.iSessionId);
			const auto binding =
				m_GameplayBindingBySessionId.find(participant.iSessionId);
			if (m_Sessions.end() == session ||
				session->second != participant.pSession ||
				m_GameplayBindingBySessionId.end() == binding ||
				binding->second.eWorldId != participant.eWorldId ||
				binding->second.pSimulation != participant.pSimulation)
			{
				cohortStillExact = false;
				status = "Prepared participant changed during durable promotion";
				break;
			}
		}
		std::set<const CGameRoom*> currentRooms;
			for (const auto& [worldId, room] : m_SharedGameRooms)
		{
			(void)worldId;
			if (nullptr != room) currentRooms.insert(room.get());
		}
		for (const auto& [sessionId, room] : m_CharacterSelectArenas)
		{
			(void)sessionId;
			if (nullptr != room) currentRooms.insert(room.get());
		}
		for (const auto& [matchId, room] : m_ColosseumMatches)
		{
			(void)matchId;
			if (nullptr != room) currentRooms.insert(room.get());
		}
		if (currentRooms.size() !=
			m_DataRevisionTransaction.Simulations.size())
		{
			cohortStillExact = false;
			status = "Process room set changed during durable promotion";
		}
		for (const auto& simulation : m_DataRevisionTransaction.Simulations)
			if (nullptr == simulation || !currentRooms.contains(simulation.get()))
			{
				cohortStillExact = false;
				status = "Prepared room left during durable promotion";
				break;
			}

		if (!cohortStillExact)
		{
			for (const auto& simulation :
				m_DataRevisionTransaction.Simulations)
				if (nullptr != simulation)
					simulation->Abort_GameplayGeneration(
						m_DataRevisionTransaction.Request.iTransactionSequence);
		}
		else
		{
			for (const auto& simulation :
				m_DataRevisionTransaction.Simulations)
			{
				if (!simulation->Commit_GameplayGeneration(
					m_DataRevisionTransaction.Request.iTransactionSequence))
				{
					/* Durable new already exists. A split in-memory generation can
					never be hidden as ABORT; restart recovery will select new. */
					std::cerr << "FATAL: partial gameplay generation commit" << '\n';
					std::terminate();
				}
			}
			m_pActiveGameplayGeneration =
				m_DataRevisionTransaction.pCandidateGeneration;
			m_ActiveGameplayBootstrapContentRevision =
				m_DataRevisionTransaction.CandidateBootstrapContentRevision;
			m_ActiveNonValtanGameplayRevision =
				m_DataRevisionTransaction.CandidateNonValtanGameplayRevision;
			m_isActiveGameplayGenerationFromCandidate = true;
		}
	}
	if (!cohortStillExact)
	{
		if (durablePointerPromoted)
		{
			std::string rollbackStatus;
			if (!Rollback_RuntimeGameplayActivation(
				runtimeRoot, baseRuntime, rollbackStatus))
			{
				std::cerr << "FATAL: runtime gameplay pointer rollback failed: "
					<< rollbackStatus << '\n';
				std::terminate();
			}
		}
		Abort_DataRevisionTransaction(std::move(status));
		return false;
	}
	if (durablePointerPromoted)
		Complete_RuntimeGameplayActivation(runtimeRoot);

	DATA_REVISION_TRANSACTION transaction =
		std::move(m_DataRevisionTransaction);
	m_DataRevisionTransaction = {};
	const GameplayDataRevision committedRevision =
		transaction.pCandidateGeneration->Get_ActiveRevision();
	for (const DATA_REVISION_PARTICIPANT& participant :
		transaction.Participants)
	{
		if (!Send_DataRevisionResult(participant.pSession, transaction.Request,
			DATA_REVISION_RESULT::COMMITTED, committedRevision, {}))
			Request_SessionClose(participant.iSessionId);
	}
	return true;
}

void LostArk::Server::CServerApp::Advance_ServerControlTransactions()
{
	using namespace LostArk::Shared;
    Advance_NumericBalanceTransaction();
	std::deque<SERVER_CONTROL_EVENT> events;
	{
		std::scoped_lock lock{ m_ServerControlMutex };
		events.swap(m_ServerControlEvents);
	}
	const auto now = std::chrono::steady_clock::now();
	m_DataRevisionResponseTombstones.erase(
		std::remove_if(
			m_DataRevisionResponseTombstones.begin(),
			m_DataRevisionResponseTombstones.end(),
			[now](const DATA_REVISION_RESPONSE_TOMBSTONE& tombstone)
			{
				return tombstone.ExpiresAt <= now;
			}),
		m_DataRevisionResponseTombstones.end());
	const auto abortCurrent =
		[this](std::string reason)
		{
			Abort_DataRevisionTransaction(std::move(reason));
		};
	for (SERVER_CONTROL_EVENT& event : events)
	{
        if (event.eKind == SERVER_CONTROL_EVENT_KIND::BALANCE_DEFERRED_ENTRY)
        {
            On_SessionFrame(event.iSessionId, event.DeferredEntry);
            continue;
        }
        if (event.eKind == SERVER_CONTROL_EVENT_KIND::BALANCE_QUERY || event.eKind == SERVER_CONTROL_EVENT_KIND::BALANCE_PATCH)
        {
            Process_NumericBalanceEvent(event);
            continue;
        }
		if (SERVER_CONTROL_EVENT_KIND::VALTAN_DECISION_TRACE_QUERY ==
			event.eKind)
		{
			Process_ValtanDecisionTraceQuery(
				event.iSessionId, event.DecisionTraceQuery);
			continue;
		}
		if (SERVER_CONTROL_EVENT_KIND::SESSION_DISCONNECTED == event.eKind)
		{
			if (m_DataRevisionTransaction.Is_Active() &&
				std::any_of(
					m_DataRevisionTransaction.Participants.begin(),
					m_DataRevisionTransaction.Participants.end(),
					[&event](const DATA_REVISION_PARTICIPANT& participant)
					{
						return participant.iSessionId == event.iSessionId;
					}))
			{
				abortCurrent(
					"A prepared client disconnected before commit");
			}
			continue;
		}
		if (SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE == event.eKind)
		{
			const C2S_DATA_REVISION_PREPARE_RESPONSE& response =
				event.RevisionResponse;
			if (!m_DataRevisionTransaction.Is_Active())
			{
				const auto answeredAbortedTransaction = std::find_if(
					m_DataRevisionResponseTombstones.begin(),
					m_DataRevisionResponseTombstones.end(),
					[&event, &response](
						const DATA_REVISION_RESPONSE_TOMBSTONE& identity)
					{
						const bool validReady =
							DATA_REVISION_PREPARE_STATUS::READY ==
								response.eStatus &&
							(response.iPreparedPresentationLaneMask &
								identity.iRequiredPresentationLaneMask) ==
								identity.iRequiredPresentationLaneMask &&
							0u == (response.iFailedPresentationLaneMask &
								identity.iRequiredPresentationLaneMask);
						const bool validNack =
							DATA_REVISION_PREPARE_STATUS::NACK ==
								response.eStatus;
						return (validReady || validNack) &&
							identity.iSessionId == event.iSessionId &&
							identity.iTransactionSequence ==
								response.iTransactionSequence &&
							identity.CandidateRevision ==
								response.CandidateRevision &&
							identity.iRequiredPresentationLaneMask ==
								response.iRequiredPresentationLaneMask;
					});
				if (m_DataRevisionResponseTombstones.end() ==
					answeredAbortedTransaction)
				{
					Request_SessionClose(event.iSessionId);
				}
				else
				{
					/* Consume the one response that was valid when sent. A later
					   duplicate is unsolicited and keeps the normal fail-close policy. */
					m_DataRevisionResponseTombstones.erase(
						answeredAbortedTransaction);
				}
				continue;
			}
			auto participant = std::find_if(
				m_DataRevisionTransaction.Participants.begin(),
				m_DataRevisionTransaction.Participants.end(),
				[&event](const DATA_REVISION_PARTICIPANT& value)
				{
					return value.iSessionId == event.iSessionId;
				});
			if (m_DataRevisionTransaction.Participants.end() == participant ||
				participant->hasResponded ||
				response.iTransactionSequence !=
					m_DataRevisionTransaction.Request.iTransactionSequence ||
				response.CandidateRevision !=
					m_DataRevisionTransaction.Request.CandidateRevision ||
				response.iRequiredPresentationLaneMask !=
					m_DataRevisionTransaction.Request.iRequiredPresentationLaneMask)
			{
				abortCurrent(
					"Duplicate, stale, or mismatched client prepare response");
				Request_SessionClose(event.iSessionId);
				continue;
			}
			participant->hasResponded = true;
			if (DATA_REVISION_PREPARE_STATUS::NACK == response.eStatus)
			{
				abortCurrent(response.strReason.empty() ?
					"A client rejected the candidate presentation generation" :
					response.strReason);
				continue;
			}
			const std::uint32_t required =
				m_DataRevisionTransaction.Request.iRequiredPresentationLaneMask;
			if ((response.iPreparedPresentationLaneMask & required) != required ||
				0u != (response.iFailedPresentationLaneMask & required))
			{
				abortCurrent(
					"Client READY did not prepare every required presentation lane");
				Request_SessionClose(event.iSessionId);
				continue;
			}
			participant->isReady = true;
			continue;
		}
		if (SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST != event.eKind)
			continue;

		if (m_DataRevisionTransaction.Is_Active() || m_NumericBalanceRequester != INVALID_SESSION_ID)
		{
			const bool exactDuplicate =
				m_DataRevisionTransaction.iRequesterSessionId == event.iSessionId &&
				m_DataRevisionTransaction.Request.iTransactionSequence ==
					event.RevisionRequest.iTransactionSequence &&
				m_DataRevisionTransaction.Request.CandidateRevision ==
					event.RevisionRequest.CandidateRevision;
			if (exactDuplicate) continue;
			std::shared_ptr<CClientSession> requester;
			GameplayDataRevision active{};
			{
				std::scoped_lock lock{ m_SessionsMutex };
				const auto iter = m_Sessions.find(event.iSessionId);
				if (m_Sessions.end() != iter) requester = iter->second;
				if (nullptr != m_pActiveGameplayGeneration)
					active = m_pActiveGameplayGeneration->Get_ActiveRevision();
			}
			if (active.Is_Valid() &&
				event.RevisionRequest.CandidateRevision != active &&
				!Send_DataRevisionResult(requester, event.RevisionRequest,
					DATA_REVISION_RESULT::ABORTED, active,
					"Another gameplay data revision transaction is active"))
				Request_SessionClose(event.iSessionId);
			continue;
		}

		DATA_REVISION_TRANSACTION staged{};
		staged.iRequesterSessionId = event.iSessionId;
		staged.Request = event.RevisionRequest;
		staged.pCandidateGeneration = std::move(event.pCandidateGeneration);
		staged.BaseBootstrapContentRevision =
			event.BaseBootstrapContentRevision;
		staged.CandidateBootstrapContentRevision =
			event.CandidateBootstrapContentRevision;
		staged.BaseNonValtanGameplayRevision =
			event.BaseNonValtanGameplayRevision;
		staged.CandidateNonValtanGameplayRevision =
			event.CandidateNonValtanGameplayRevision;
		std::string beginStatus;
		{
			std::scoped_lock lock{ m_SessionsMutex };
			const auto requesterSession = m_Sessions.find(event.iSessionId);
			const auto requesterBinding =
				m_GameplayBindingBySessionId.find(event.iSessionId);
			if (m_Sessions.end() == requesterSession ||
				m_GameplayBindingBySessionId.end() == requesterBinding ||
				WORLD_ID::VALTAN_ARENA != requesterBinding->second.eWorldId ||
				nullptr == requesterBinding->second.pSimulation)
				beginStatus = "Data revision requester left Valtan Arena";
			else if (nullptr == m_pActiveGameplayGeneration ||
				m_pActiveGameplayGeneration->Get_ActiveRevision() !=
					staged.Request.BaseRevision)
				beginStatus = "Data revision base is no longer process active";
			else if (!staged.BaseBootstrapContentRevision.Is_Valid() ||
				!staged.CandidateBootstrapContentRevision.Is_Valid() ||
				m_ActiveGameplayBootstrapContentRevision !=
					staged.BaseBootstrapContentRevision)
				beginStatus = "Candidate bootstrap baseline is no longer process active";
			else if (!staged.BaseNonValtanGameplayRevision.Is_Valid() ||
				!staged.CandidateNonValtanGameplayRevision.Is_Valid() ||
				m_ActiveNonValtanGameplayRevision !=
					staged.BaseNonValtanGameplayRevision ||
				m_ActiveNonValtanGameplayRevision !=
					staged.CandidateNonValtanGameplayRevision)
				beginStatus = "Candidate changes gameplay outside allowedDomains VALTAN_BOSS";
			else if (nullptr == staged.pCandidateGeneration ||
				staged.pCandidateGeneration->Get_ActiveRevision() !=
					staged.Request.CandidateRevision)
				beginStatus = "Admitted candidate parent revision is inconsistent";
			else
			{
				staged.Participants.reserve(m_GameplayBindingBySessionId.size());
				for (const auto& [participantId, binding] :
					m_GameplayBindingBySessionId)
				{
					const auto sessionIter = m_Sessions.find(participantId);
					if (m_Sessions.end() == sessionIter ||
						nullptr == binding.pSimulation)
						continue;
					DATA_REVISION_PARTICIPANT participant{};
					participant.iSessionId = participantId;
					participant.eWorldId = binding.eWorldId;
					participant.pSession = sessionIter->second;
					participant.pSimulation = binding.pSimulation;
					staged.Participants.push_back(std::move(participant));
				}
				std::sort(staged.Participants.begin(), staged.Participants.end(),
					[](const DATA_REVISION_PARTICIPANT& left,
						const DATA_REVISION_PARTICIPANT& right)
					{
						return left.iSessionId < right.iSessionId;
					});
				for (const auto& [worldId, simulation] : m_SharedGameRooms)
				{
					(void)worldId;
					if (nullptr != simulation)
						staged.Simulations.push_back(simulation);
				}
				for (const auto& [ownerSessionId, simulation] :
					m_CharacterSelectArenas)
				{
					(void)ownerSessionId;
					if (nullptr != simulation)
						staged.Simulations.push_back(simulation);
				}
				for (const auto& [matchId, simulation] : m_ColosseumMatches)
				{
					(void)matchId;
					if (simulation) staged.Simulations.push_back(simulation);
				}
				const bool requesterParticipates = std::any_of(
					staged.Participants.begin(), staged.Participants.end(),
					[&event](const DATA_REVISION_PARTICIPANT& participant)
					{
						return participant.iSessionId == event.iSessionId;
					});
				if (!requesterParticipates || staged.Simulations.empty())
					beginStatus = "Data revision participant or room set is empty";
			}
		}
		if (!beginStatus.empty())
		{
			std::shared_ptr<CClientSession> requester;
			GameplayDataRevision active = staged.Request.BaseRevision;
			{
				std::scoped_lock lock{ m_SessionsMutex };
				const auto iter = m_Sessions.find(event.iSessionId);
				if (m_Sessions.end() != iter) requester = iter->second;
				if (nullptr != m_pActiveGameplayGeneration)
					active = m_pActiveGameplayGeneration->Get_ActiveRevision();
			}
			if (active == staged.Request.CandidateRevision ||
				!Send_DataRevisionResult(requester, staged.Request,
					DATA_REVISION_RESULT::ABORTED, active, beginStatus))
				Request_SessionClose(event.iSessionId);
			continue;
		}
		staged.Deadline = std::chrono::steady_clock::now() +
			std::chrono::seconds(3);
		m_DataRevisionTransaction = std::move(staged);
		bool prepareSent = true;
		for (const DATA_REVISION_PARTICIPANT& participant :
			m_DataRevisionTransaction.Participants)
		{
			if (!Send_DataRevisionPrepare(
				participant.pSession, m_DataRevisionTransaction.Request))
			{
				prepareSent = false;
				Request_SessionClose(participant.iSessionId);
				break;
			}
		}
		if (!prepareSent)
			abortCurrent(
				"A participant could not receive the prepare message");
	}

	if (!m_DataRevisionTransaction.Is_Active()) return;
	std::string membershipStatus;
	if (!Validate_DataRevisionTransactionMembership(membershipStatus))
	{
		abortCurrent(std::move(membershipStatus));
		return;
	}
	if (std::chrono::steady_clock::now() >=
		m_DataRevisionTransaction.Deadline)
	{
		abortCurrent(
			"Data revision prepare timed out before every client was ready");
		return;
	}
	const bool allReady = std::all_of(
		m_DataRevisionTransaction.Participants.begin(),
		m_DataRevisionTransaction.Participants.end(),
		[](const DATA_REVISION_PARTICIPANT& participant)
		{
			return participant.isReady;
		});
	if (allReady)
		(void)Commit_DataRevisionTransaction();
}

void LostArk::Server::CServerApp::Tick_GameplaySimulations(
	const float fixedDeltaSeconds, const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics)
{
	/* Control transactions commit before any room consumes this fixed step, so
	   every process room observes one generation on this tick boundary. */
	Advance_ServerControlTransactions();
	Advance_ColosseumPreparation();
	std::vector<std::shared_ptr<CGameRoom>> simulations;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		simulations.reserve(
			m_SharedGameRooms.size() + m_CharacterSelectArenas.size());
		for (const auto& [worldId, simulation] : m_SharedGameRooms)
		{
			(void)worldId;
			if (nullptr != simulation)
				simulations.push_back(simulation);
		}
		for (const auto& [sessionId, simulation] : m_CharacterSelectArenas)
		{
			(void)sessionId;
			if (nullptr != simulation)
				simulations.push_back(simulation);
		}
		for (const auto& [matchId, simulation] : m_ColosseumMatches)
		{
			(void)matchId;
			if (simulation) simulations.push_back(simulation);
		}
	}

	// The room thread is the only writer of gameplay state. The mutex is
	// released before Tick so receive and session threads never wait on a tick.
	for (const std::shared_ptr<CGameRoom>& simulation : simulations)
	{
		simulation->Tick(fixedDeltaSeconds, schedulerMetrics);
		const std::string diagnostic = simulation->Take_PerformanceDiagnostic();
		if (diagnostic.empty()) continue;
		bool written = false;
		try
		{
			auto path = Resolve_ServerSessionDiagnosticPath();
			if (!path.empty())
			{
				path.replace_filename(L"server-room-perf-" + std::to_wstring(::GetCurrentProcessId()) + L".log");
				written = Append_BoundedRoomPerformanceDiagnostic(path, diagnostic);
			}
		}
		catch (...) { } // Preserve gameplay when diagnostic path resolution fails.
		if (!written && !m_bRoomPerformanceLogWarningReported)
		{
			m_bRoomPerformanceLogWarningReported = true;
			std::cerr << "Room performance diagnostic file unavailable; stdout logging continues.\n";
		}
	}
	// Admit transfers only after this tick's already queued entries have run.
	for (const std::shared_ptr<CGameRoom>& simulation : simulations)
	{
		Handle_WorldTransfers(simulation);
	}
	Retire_QuiescentCharacterSelectArenas();
	Retire_QuiescentColosseumMatches();
}

void LostArk::Server::CServerApp::Retire_QuiescentCharacterSelectArenas()
{
	// Lock order is ServerApp sessions mutex -> room command mutex. Try_Seal
	// serializes the empty decision with each receive-thread Enqueue.
	std::scoped_lock lock{ m_SessionsMutex };
	for (auto iter = m_CharacterSelectArenas.begin();
		iter != m_CharacterSelectArenas.end();)
	{
		const SESSION_ID ownerSessionId = iter->first;
		const std::shared_ptr<CGameRoom>& simulation = iter->second;
		const auto bindingIter =
			m_GameplayBindingBySessionId.find(ownerSessionId);
		const bool isStillBound =
			bindingIter != m_GameplayBindingBySessionId.end() &&
			bindingIter->second.pSimulation == simulation;

		if (isStillBound || nullptr == simulation ||
			!simulation->Try_SealPrivateArenaForRetirement())
		{
			++iter;
			continue;
		}
		iter = m_CharacterSelectArenas.erase(iter);
	}
}

bool LostArk::Server::CServerApp::Begin_ColosseumPreparation(
    const std::shared_ptr<CGameRoom>& source, const SERVER_WORLD_TRANSFER_REQUEST& transfer)
{
    using namespace LostArk::Shared;
    if (!source || !transfer.bColosseumMatch || source->Get_WorldId() != WORLD_ID::BERN ||
        transfer.eTargetWorldId != WORLD_ID::COLOSSEUM || (transfer.PartyBatchSessionIds.empty() || transfer.PartyBatchSessionIds.size() > MAX_COLOSSEUM_MATCH_PLAYERS) ||
        m_ColosseumPreparationWorker.valid()) return false;
    std::shared_ptr<const CGameplayCatalog> generation;
    {
        std::scoped_lock lock{ m_SessionsMutex };
        generation = m_pActiveGameplayGeneration;
    }
    if (!generation) return false;
    try
    {
        auto cancelled = std::make_shared<std::atomic_bool>(false);
        m_ColosseumPreparationTransfer = transfer;
        m_ColosseumPreparationSource = source;
        m_ColosseumPreparationCancelled = cancelled;
        m_ColosseumPreparationWorker = std::async(std::launch::async,
            [generation = std::move(generation), cancelled, snapshotJobs = m_SnapshotJobs]()
            {
                COLOSSEUM_PREPARATION_RESULT result;
                const auto start = std::chrono::steady_clock::now();
                try
                {
                    if (!cancelled->load(std::memory_order_relaxed))
                        result.Room = std::make_shared<CGameRoom>(WORLD_ID::COLOSSEUM, generation, cancelled.get(), snapshotJobs);
                    if (cancelled->load(std::memory_order_relaxed) || !result.Room || !result.Room->Is_Ready())
                    {
                        result.Status = result.Room ? result.Room->Get_Status() : "Colosseum preparation cancelled";
                        result.Room.reset(); // Release a failed preparation on its worker.
                    }
                }
                catch (const std::exception& error) { result.Status = error.what(); result.Room.reset(); }
                catch (...) { result.Status = "Colosseum preparation failed"; result.Room.reset(); }
                result.ElapsedMilliseconds = std::chrono::duration<double, std::milli>(
                    std::chrono::steady_clock::now() - start).count();
                return result;
            });
        return true;
    }
    catch (...)
    {
        m_ColosseumPreparationSource.reset();
        m_ColosseumPreparationCancelled.reset();
        return false;
    }
}

void LostArk::Server::CServerApp::Advance_ColosseumPreparation()
{
    if (!m_ColosseumPreparationWorker.valid()) return;
    bool sourceBound = true;
    {
        std::scoped_lock lock{ m_SessionsMutex };
        for (const auto sessionId : m_ColosseumPreparationTransfer.PartyBatchSessionIds)
        {
            const auto session = m_Sessions.find(sessionId);
            const auto binding = m_GameplayBindingBySessionId.find(sessionId);
            if (session == m_Sessions.end() || !session->second || session->second->Is_Closing() ||
                binding == m_GameplayBindingBySessionId.end() || binding->second.pSimulation != m_ColosseumPreparationSource)
            { sourceBound = false; break; }
        }
    }
    if (!sourceBound && m_ColosseumPreparationCancelled)
        m_ColosseumPreparationCancelled->store(true, std::memory_order_relaxed);
    if (m_ColosseumPreparationWorker.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
    // An in-progress generation commit owns admission. Keep a completed immutable
    // result queued instead of discarding and repeatedly loading it during the save.
    if (sourceBound && (m_DataRevisionTransaction.Is_Active() || m_NumericAdmissionPaused)) return;
    COLOSSEUM_PREPARATION_RESULT result;
    try { result = m_ColosseumPreparationWorker.get(); }
    catch (...) { result.Status = "Colosseum preparation future failed"; }
    m_PreparedColosseumRoom = std::move(result.Room);
    SESSION_WORLD_TRANSFER_FAILURE failure;
    const bool cancelled = m_ColosseumPreparationCancelled &&
        m_ColosseumPreparationCancelled->load(std::memory_order_relaxed);
    const bool committed = sourceBound && !cancelled && m_PreparedColosseumRoom &&
        Transfer_SessionWorld(m_ColosseumPreparationSource, m_ColosseumPreparationTransfer, failure);
    if (m_ColosseumPreparationSource) m_ColosseumPreparationSource->Notify_ColosseumTransferResult(committed);
    std::cout << "[ColosseumPreparation] workerMs=" << result.ElapsedMilliseconds
        << " committed=" << (committed ? "true" : "false")
        << " status=" << (result.Status.empty() ? failure.strContext : result.Status) << '\n';
    m_ColosseumPreparationSource.reset();
    m_ColosseumPreparationCancelled.reset();
    m_PreparedColosseumRoom.reset();
    m_ColosseumPreparationTransfer = {};
}

void LostArk::Server::CServerApp::Retire_QuiescentColosseumMatches()
{
	std::scoped_lock lock{ m_SessionsMutex };
	for (auto it = m_ColosseumMatches.begin(); it != m_ColosseumMatches.end();)
	{
		const auto& room = it->second;
		const bool bound = std::any_of(m_GameplayBindingBySessionId.begin(), m_GameplayBindingBySessionId.end(),
			[&room](const auto& binding) { return binding.second.pSimulation == room; });
		if (!bound && room && room->Try_SealColosseumForRetirement()) it = m_ColosseumMatches.erase(it);
		else ++it;
	}
}

void LostArk::Server::CServerApp::Handle_WorldTransfers(
	const std::shared_ptr<CGameRoom>& sourceSimulation)
{
	if (nullptr == sourceSimulation)
		return;

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	while (sourceSimulation->Try_DequeueWorldTransfer(transfer))
	{
		if (transfer.bColosseumMatch)
		{
			if (!Begin_ColosseumPreparation(sourceSimulation, transfer)) sourceSimulation->Notify_ColosseumTransferResult(false);
			continue;
		}
		SESSION_WORLD_TRANSFER_FAILURE failure{};
		const bool committed = Transfer_SessionWorld(sourceSimulation, transfer, failure);
		if (transfer.bColosseumMatch) sourceSimulation->Notify_ColosseumTransferResult(committed);
		if (!committed)
		{
			if (!transfer.PartyBatchSessionIds.empty() || failure.bSourcePreservedOnRejection)
			{
				std::cerr << "Party transfer rejected without source departure: "
					<< failure.strContext << '\n';
				sourceSimulation->Notify_PartyTransferFailure(transfer.iSessionId,
					transfer.iPartyRequestSequence, transfer.eTargetWorldId, failure.ePartyResult);
				continue;
			}
			Request_SessionClose(
				transfer.iSessionId,
				failure.eReason,
				failure.iNativeErrorCode,
				failure.strContext);
		}
	}
}

bool LostArk::Server::CServerApp::Transfer_SessionWorld(
	const std::shared_ptr<CGameRoom>& sourceSimulation,
	const SERVER_WORLD_TRANSFER_REQUEST& transfer,
	SESSION_WORLD_TRANSFER_FAILURE& outFailure)
{
	using namespace LostArk::Shared;
	outFailure = {};
	const auto setFailure =
		[&outFailure, &sourceSimulation, &transfer](
			const SESSION_DIAGNOSTIC_REASON reason,
			const int nativeErrorCode,
			const std::string_view stage,
			const bool rollbackRequired = false,
			const bool rollbackEnqueued = false)
		{
			const WORLD_ID sourceWorldId = nullptr == sourceSimulation ?
				WORLD_ID::END : sourceSimulation->Get_WorldId();
			outFailure.eReason = reason;
			outFailure.iNativeErrorCode = nativeErrorCode;
			outFailure.bRollbackCleanupRequired = rollbackRequired;
			outFailure.bRollbackCleanupEnqueued = rollbackEnqueued;
			outFailure.strContext =
				"world transfer stage=" + std::string{ stage } +
				" sourceWorld=" +
				std::to_string(static_cast<std::uint16_t>(sourceWorldId)) +
				" targetWorld=" + std::to_string(
					static_cast<std::uint16_t>(transfer.eTargetWorldId)) +
				" rollbackCleanupRequired=" +
				(rollbackRequired ? "true" : "false") +
				" rollbackCleanupEnqueued=" +
				(rollbackEnqueued ? "true" : "false");
		};

	if (nullptr == sourceSimulation ||
		CHARACTER_CLASS_ID::END == transfer.eCharacterClass ||
		transfer.strNickName.empty())
	{
		setFailure(
			SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
			WSAEINVAL,
			"request-validation");
		return false;
	}

	const WORLD_ID sourceWorldId = sourceSimulation->Get_WorldId();
	if (transfer.bColosseumMatch)
	{
		std::scoped_lock lock{ m_SessionsMutex };
		if (sourceWorldId != WORLD_ID::BERN || transfer.eTargetWorldId != WORLD_ID::COLOSSEUM ||
			(transfer.PartyBatchSessionIds.empty() || transfer.PartyBatchSessionIds.size() > MAX_COLOSSEUM_MATCH_PLAYERS) || m_iNextColosseumMatchId == 0u ||
			!m_pActiveGameplayGeneration || m_DataRevisionTransaction.Is_Active() || m_NumericAdmissionPaused)
		{ setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, WSAEINVAL, "Colosseum batch validation"); return false; }
		for (const auto sessionId : transfer.PartyBatchSessionIds)
		{
			const auto session = m_Sessions.find(sessionId);
			const auto binding = m_GameplayBindingBySessionId.find(sessionId);
			if (session == m_Sessions.end() || !session->second || session->second->Is_Closing() ||
				binding == m_GameplayBindingBySessionId.end() || binding->second.pSimulation != sourceSimulation)
			{ setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, WSAENOTCONN, "Colosseum source binding"); return false; }
		}
		const auto target = m_PreparedColosseumRoom;
		if (!target || !target->Is_Ready() || target->Get_WorldId() != WORLD_ID::COLOSSEUM ||
			target->Get_ActiveGameplayGeneration() != m_pActiveGameplayGeneration)
		{ setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, WSAEINVAL, "Colosseum prepared generation is unavailable or stale"); return false; }
		const auto matchId = m_iNextColosseumMatchId;
		const auto [entry, inserted] = m_ColosseumMatches.emplace(matchId, target);
		if (!inserted)
		{ setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, WSAEINVAL, "Colosseum match identity"); return false; }
		std::string status;
		if (!sourceSimulation->Transfer_ColosseumMatchTo(*target, transfer.PartyBatchSessionIds, matchId, status))
		{
			m_ColosseumMatches.erase(entry);
			setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, WSAEINVAL, status); return false;
		}
		for (const auto sessionId : transfer.PartyBatchSessionIds)
		{
			auto& binding = m_GameplayBindingBySessionId.at(sessionId);
			binding.eWorldId = WORLD_ID::COLOSSEUM;
			binding.iPrivateArenaOwnerSessionId = INVALID_SESSION_ID;
			binding.pSimulation = target;
		}
		++m_iNextColosseumMatchId;
		return true;
	}
	const std::shared_ptr<CGameRoom> targetSimulation =
		Find_SharedSimulation(transfer.eTargetWorldId);
	if (nullptr == targetSimulation ||
		targetSimulation == sourceSimulation ||
		sourceWorldId == transfer.eTargetWorldId)
	{
		setFailure(
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
			WSAEINVAL,
			"target-resolution");
		return false;
	}

	// Enqueue_AssignedCommand and On_SessionClosed use the same mutex. Route
	// lookup, queue ordering, and the binding commit therefore have one order.
	std::scoped_lock lock{ m_SessionsMutex };
	const auto sessionIter = m_Sessions.find(transfer.iSessionId);
	const auto bindingIter =
		m_GameplayBindingBySessionId.find(transfer.iSessionId);
	if (sessionIter == m_Sessions.end() ||
		nullptr == sessionIter->second ||
		bindingIter == m_GameplayBindingBySessionId.end() ||
		bindingIter->second.eWorldId != sourceWorldId ||
		bindingIter->second.pSimulation != sourceSimulation ||
		(WORLD_ID::CHARACTER_SELECT_ARENA == sourceWorldId &&
			bindingIter->second.iPrivateArenaOwnerSessionId !=
				transfer.iSessionId))
	{
		setFailure(
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
			WSAENOTCONN,
			"source-binding-validation");
		return false;
	}
	if (sessionIter->second->Is_Closing())
	{
		setFailure(
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
			WSAESHUTDOWN,
			"source-session-terminal-recheck");
		return false;
	}
	// A guide owner's solo raid trip gets the same admission/FIFO transaction as a party,
	// while its participant list remains one human and no party is created.
	auto transactionSessions = transfer.PartyBatchSessionIds;
	const bool bernRaidPair =
		(sourceWorldId == WORLD_ID::BERN && (transfer.eTargetWorldId == WORLD_ID::VALTAN_ARENA || transfer.eTargetWorldId == WORLD_ID::KAKULSAYDON_ARENA)) ||
		(transfer.eTargetWorldId == WORLD_ID::BERN && (sourceWorldId == WORLD_ID::VALTAN_ARENA || sourceWorldId == WORLD_ID::KAKULSAYDON_ARENA));
	if (transactionSessions.empty() && bernRaidPair &&
		(sourceSimulation->Has_PersonalGuideOwner(sessionIter->second) || targetSimulation->Has_PersonalGuideOwner(sessionIter->second)))
	{
		transactionSessions.push_back(transfer.iSessionId);
		outFailure.bSourcePreservedOnRejection = true;
	}

	if (!transactionSessions.empty())
	{
		if (transactionSessions.front() != transfer.iSessionId)
		{
			setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED,
				WSAEINVAL, "party-leader-identity");
			return false;
		}
		for (const SESSION_ID memberId : transactionSessions)
		{
			const auto member = m_Sessions.find(memberId);
			const auto binding = m_GameplayBindingBySessionId.find(memberId);
			if (member == m_Sessions.end() || nullptr == member->second ||
				member->second->Is_Closing() || binding == m_GameplayBindingBySessionId.end() ||
				binding->second.eWorldId != sourceWorldId || binding->second.pSimulation != sourceSimulation)
			{
				setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
					WSAENOTCONN, "party-member-binding");
				return false;
			}
		}
		std::string status;
		if (!sourceSimulation->Transfer_PartyTo(*targetSimulation,
			transactionSessions, outFailure.ePartyResult, status,
			transfer.strRaidReturnNpcPlacementId, transfer.strSpawnPlacementOverrideId))
		{
			setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED,
				WSAEINVAL, status);
			return false;
		}
		for (const SESSION_ID memberId : transactionSessions)
		{
			auto& binding = m_GameplayBindingBySessionId.at(memberId);
			binding.eWorldId = transfer.eTargetWorldId;
			binding.iPrivateArenaOwnerSessionId = INVALID_SESSION_ID;
			binding.pSimulation = targetSimulation;
		}
		return true;
	}

	ROOM_COMMAND registerCommand{};
	registerCommand.eType = ROOM_COMMAND_TYPE::REGISTER_SESSION;
	registerCommand.iSessionId = transfer.iSessionId;
	registerCommand.pSession = sessionIter->second;
	const ROOM_COMMAND_ENQUEUE_RESULT targetRegisterResult =
		targetSimulation->Enqueue_Detailed(std::move(registerCommand));
	if (ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED != targetRegisterResult)
	{
		setFailure(
			ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY ==
				targetRegisterResult ?
				SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW :
			ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY ==
				targetRegisterResult ?
				SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED :
				SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
			ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY ==
				targetRegisterResult ? WSAENOBUFS : WSAESHUTDOWN,
			"target-register-ingress " +
				targetSimulation->Describe_EnqueueResult(targetRegisterResult));
		return false;
	}

	C2S_ENTER_WORLD enterWorld{};
	enterWorld.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	enterWorld.eWorldId = transfer.eTargetWorldId;
	enterWorld.eCharacterClass = transfer.eCharacterClass;
	enterWorld.strNickName = transfer.strNickName;
	enterWorld.iVoiceType = transfer.iVoiceType;
	enterWorld.strAppearanceJson = transfer.strAppearanceJson;
	ROOM_COMMAND enterCommand{};
	enterCommand.eType = ROOM_COMMAND_TYPE::ENTER_WORLD;
	enterCommand.iSessionId = transfer.iSessionId;
	enterCommand.EnterWorld = std::move(enterWorld);
	enterCommand.strSpawnPlacementOverrideId = transfer.strSpawnPlacementOverrideId;
	enterCommand.strRaidReturnNpcPlacementId = transfer.strRaidReturnNpcPlacementId;
	enterCommand.eEntrySourceWorldId = sourceWorldId;
	enterCommand.CarriedInventory = transfer.CarriedInventory;
	enterCommand.bHasCarriedCharacterState = true;
	enterCommand.CarriedPurse = transfer.CarriedPurse;
	enterCommand.iCarriedHonorTitleId = transfer.iHonorTitleId;
	const ROOM_COMMAND_ENQUEUE_RESULT targetEnterResult =
		targetSimulation->Enqueue_Detailed(std::move(enterCommand));
	if (ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED != targetEnterResult)
	{
		ROOM_COMMAND targetRollback{};
		targetRollback.eType = ROOM_COMMAND_TYPE::LEAVE;
		targetRollback.iSessionId = transfer.iSessionId;
		targetRollback.eLeaveReason =
			PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
		const bool rollbackEnqueued =
			targetSimulation->Enqueue(std::move(targetRollback));
		setFailure(
			ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY ==
				targetEnterResult ?
				SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW :
			ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY ==
				targetEnterResult ?
				SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED :
				SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
			ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY ==
				targetEnterResult ? WSAENOBUFS : WSAESHUTDOWN,
			"target-enter-ingress " +
				targetSimulation->Describe_EnqueueResult(targetEnterResult),
			true,
			rollbackEnqueued);
		return false;
	}

	if (!sourceSimulation->Commit_WorldTransferDeparture(
		transfer.iSessionId))
	{
		ROOM_COMMAND targetRollback{};
		targetRollback.eType = ROOM_COMMAND_TYPE::LEAVE;
		targetRollback.iSessionId = transfer.iSessionId;
		targetRollback.eLeaveReason =
			PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
		const bool rollbackEnqueued =
			targetSimulation->Enqueue(std::move(targetRollback));
		setFailure(
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
			WSAEINVAL,
			"source-departure-commit",
			true,
			rollbackEnqueued);
		return false;
	}

	bindingIter->second.eWorldId = transfer.eTargetWorldId;
	bindingIter->second.iPrivateArenaOwnerSessionId = INVALID_SESSION_ID;
	bindingIter->second.pSimulation = targetSimulation;
	return true;
}

void LostArk::Server::CServerApp::Shutdown()
{
	m_isRunning.store(false);
	m_TcpListener.Close();
	if (m_AcceptThread.joinable())
		m_AcceptThread.join();
	if (m_RoomThread.joinable())
		m_RoomThread.join();
	if (m_ColosseumPreparationWorker.valid())
	{
		if (m_ColosseumPreparationCancelled) m_ColosseumPreparationCancelled->store(true, std::memory_order_relaxed);
		if (m_ColosseumPreparationWorker.wait_for(std::chrono::seconds(30)) != std::future_status::ready)
		{
			std::cerr << "Colosseum preparation did not stop within the shutdown deadline.\n";
			::ExitProcess(ERROR_TIMEOUT);
		}
		try { (void)m_ColosseumPreparationWorker.get(); } catch (...) { }
	}
	m_ColosseumPreparationSource.reset();
	m_PreparedColosseumRoom.reset();
	m_ColosseumPreparationCancelled.reset();
    if (m_NumericBalanceWorker.valid())
    {
        if (m_NumericBalanceWorker.wait_for(std::chrono::seconds(30)) != std::future_status::ready)
        {
            std::cerr << "Numeric save worker did not stop within the shutdown deadline.\n";
            ::ExitProcess(ERROR_TIMEOUT);
        }
        // Finish an already durable save before discarding process memory.
        if (m_NumericBalancePersisting) Advance_NumericBalanceTransaction();
        else { (void)m_NumericBalanceWorker.get(); Finish_NumericBalanceTransaction(false, "Server stopped before saving"); }
    }
	Abort_DataRevisionTransaction("Server shutdown aborted data revision prepare");

	std::vector<std::shared_ptr<CClientSession>> sessions;
	{
		std::scoped_lock lock{ m_SessionsMutex };
		for (auto& [sessionId, session] : m_Sessions)
		{
			(void)sessionId;
			sessions.push_back(session);
		}
	}
	for (const auto& session : sessions)
	{
		if (nullptr != session)
		{
			session->Request_Close(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SHUTDOWN,
				0,
				"Server process shutdown");
		}
	}
	for (const auto& session : sessions)
	{
		if (nullptr != session)
			session->Stop();
	}
	{
		std::scoped_lock lock{ m_SessionsMutex };
		m_Sessions.clear();
		m_GameplayBindingBySessionId.clear();
	}

	// Receive callbacks and the room thread are stopped before simulations are
	// destroyed, so no producer can enqueue into an unowned room.
	{
		std::scoped_lock lock{ m_SessionsMutex };
		m_CharacterSelectArenas.clear();
		m_ColosseumMatches.clear();
		m_SharedGameRooms.clear();
		m_pActiveGameplayGeneration.reset();
		m_ActiveGameplayBootstrapContentRevision = {};
		m_ActiveNonValtanGameplayRevision = {};
		m_isActiveGameplayGenerationFromCandidate = false;
		m_isRuntimeActivePersistenceEnabled = false;
	}
	// Release the shutdown snapshot while the injected IOCP owner is still
	// alive; Stop is also idempotent for any externally retained drained session.
	sessions.clear();
	if (m_IocpService)
	{
		m_IocpService->Stop();
		m_IocpService.reset();
	}
	if (m_SnapshotJobs)
	{
		m_SnapshotJobs->Shutdown();
		m_SnapshotJobs.reset();
	}
	m_DataRevisionResponseTombstones.clear();
	{
		std::scoped_lock lock{ m_ClosedSessionMutex };
		m_ClosedSessionIds.clear();
	}
	{
		std::scoped_lock lock{ m_ServerControlMutex };
		m_ServerControlEvents.clear();
	}
	m_WinSockContext.Shutdown();
	Release_RuntimeGameplayProcessMutex(m_hRuntimeGameplayProcessMutex);
}


void LostArk::Server::CServerApp::Send_NumericBalanceResult(
    const SESSION_ID sessionId, const std::uint32_t sequence,
    const LostArk::Shared::BALANCE_APPLY_RESULT result, std::string reason)
{
    using namespace LostArk::Shared;
    std::shared_ptr<CClientSession> session;
    {
        std::scoped_lock lock{m_SessionsMutex};
        const auto found = m_Sessions.find(sessionId);
        if (found != m_Sessions.end()) session = found->second;
    }
    if (!session || session->Is_Closing()) return;
    if (reason.size() > MAX_BALANCE_REASON_BYTES) reason.resize(MAX_BALANCE_REASON_BYTES);
    const S2C_BALANCE_RESULT response{sequence, result, m_NumericBalanceStore.Get_NumericRevision(), std::move(reason)};
    CPacketWriter writer;
    if (!Write_Message(writer, response) || !session->Send_Frame(PACKET_TYPE::S2C_BALANCE_RESULT, writer.Get_Buffer()))
        Request_SessionClose(sessionId);
}

void LostArk::Server::CServerApp::Process_NumericBalanceEvent(const SERVER_CONTROL_EVENT& event)
{
    using namespace LostArk::Shared;
    const bool query = event.eKind == SERVER_CONTROL_EVENT_KIND::BALANCE_QUERY;
    const auto sequence = query ? event.BalanceQuery.iRequestSequence : event.BalancePatch.iRequestSequence;
    std::shared_ptr<CClientSession> session;
    std::shared_ptr<const CGameplayCatalog> active;
    {
        std::scoped_lock lock{m_SessionsMutex};
        const auto found = m_Sessions.find(event.iSessionId);
        const auto binding = m_GameplayBindingBySessionId.find(event.iSessionId);
        if (found != m_Sessions.end() && binding != m_GameplayBindingBySessionId.end() && binding->second.pSimulation)
            session = found->second;
        active = m_pActiveGameplayGeneration;
    }
    if (!session || session->Is_Closing())
    {
        Send_NumericBalanceResult(event.iSessionId, sequence, BALANCE_APPLY_RESULT::INVALID_SESSION, "Enter a Server world before editing balance");
        return;
    }
    std::string status;
    // A completed authored-pattern transaction may have replaced the catalogue.
    if (m_NumericBalanceStore.Get_ActiveGeneration() != active &&
        !m_NumericBalanceStore.Initialize(active, status))
    {
        Send_NumericBalanceResult(event.iSessionId, sequence, BALANCE_APPLY_RESULT::INVALID_CHANGE, status);
        return;
    }
    if (query)
    {
        S2C_BALANCE_SNAPSHOT snapshot;
        if (!m_NumericBalanceStore.BuildSnapshot(sequence, event.BalanceQuery.iPageIndex, snapshot, status))
        { Send_NumericBalanceResult(event.iSessionId, sequence, BALANCE_APPLY_RESULT::INVALID_CHANGE, status); return; }
        CPacketWriter writer;
        if (!Write_Message(writer, snapshot) || !session->Send_Frame(PACKET_TYPE::S2C_BALANCE_SNAPSHOT, writer.Get_Buffer()))
            Request_SessionClose(event.iSessionId);
        return;
    }
    if (m_DataRevisionTransaction.Is_Active() || m_NumericBalanceRequester != INVALID_SESSION_ID)
    {
        Send_NumericBalanceResult(event.iSessionId, sequence, BALANCE_APPLY_RESULT::BUSY, "Another balance or authored-pattern save is being applied; this draft was not changed");
        return;
    }
    if (event.BalancePatch.BaseNumericRevision != m_NumericBalanceStore.Get_NumericRevision())
    {
        Send_NumericBalanceResult(event.iSessionId, sequence, BALANCE_APPLY_RESULT::STALE_REVISION, "Server numbers changed since this draft was read; reload the current values before saving");
        return;
    }
    m_NumericBalanceRequester = event.iSessionId;
    m_NumericBalanceRequestSequence = sequence;
    if (++m_NumericBalanceTransactionSequence == 0u) ++m_NumericBalanceTransactionSequence;
    m_NumericBalancePersisting = false;
    try
    {
        const auto runtimeRoot = m_isRuntimeActivePersistenceEnabled ? Resolve_RuntimeActiveGameplayRoot() : std::filesystem::path{};
        RUNTIME_ACTIVE_GAMEPLAY_GENERATION runtimeBase;
        runtimeBase.eSource = m_isActiveGameplayGenerationFromCandidate ?
            RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE : RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
        runtimeBase.Revision = active->Get_ActiveRevision();
        runtimeBase.BootstrapContentRevision = m_ActiveGameplayBootstrapContentRevision;
        runtimeBase.NonValtanGameplayRevision = m_ActiveNonValtanGameplayRevision;
        m_NumericBalanceWorker = std::async(std::launch::async,
            [store = m_NumericBalanceStore, request = event.BalancePatch, runtimeRoot, runtimeBase]() {
                BALANCE_WORK_RESULT result;
                try
                {
                    result.Prepared = std::make_shared<SERVER_BALANCE_PREPARED>();
                    result.Succeeded = store.PreparePatch(request, *result.Prepared, result.Failure, result.Status);
                    if (result.Succeeded)
                    {
                        const auto& bytes = result.Prepared->BootstrapBytes;
                        result.Succeeded = Hash_BytesSha256(std::span<const std::uint8_t>(
                            reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size()), result.BootstrapRevision) &&
                            Build_NonValtanGameplayRevisionBytes(bytes, result.NonValtanRevision, result.Status);
                        if (result.Succeeded && !runtimeRoot.empty() && std::filesystem::exists(runtimeRoot))
                        {
                            bool exists = false;
                            std::vector<std::uint8_t> pointerBytes;
                            if (!ReadOptionalRuntimeGameplayFile(runtimeRoot, RUNTIME_ACTIVE_POINTER_NAME, exists, pointerBytes, result.Status))
                                result.Succeeded = false;
                            else if (exists)
                            {
                                RUNTIME_ACTIVE_GAMEPLAY_GENERATION disk;
                                if (!ParseRuntimeActivePointer(pointerBytes, disk, result.Status) || !RuntimeGenerationEquals(disk, runtimeBase))
                                { result.Succeeded = false; result.Status = "Durable gameplay pointer changed before numeric save"; }
                                else
                                {
                                    auto next = runtimeBase;
                                    next.BootstrapContentRevision = result.BootstrapRevision;
                                    next.NonValtanGameplayRevision = result.NonValtanRevision;
                                    result.Prepared->RuntimeActivePointerPath = runtimeRoot / RUNTIME_ACTIVE_POINTER_NAME;
                                    result.Prepared->RuntimeActivePointerBaseBytes.assign(reinterpret_cast<const char*>(pointerBytes.data()), pointerBytes.size());
                                    result.Prepared->RuntimeActivePointerBytes = BuildRuntimeActivePointerBytes(next);
                                }
                            }
                        }
                    }
                }
                catch (const std::exception& error) { result.Succeeded = false; result.Status = error.what(); }
                catch (...) { result.Succeeded = false; result.Status = "Numeric candidate validation failed"; }
                return result;
            });
    }
    catch (const std::exception& error) { Finish_NumericBalanceTransaction(false, error.what()); }
}

void LostArk::Server::CServerApp::Advance_NumericBalanceTransaction()
{
    using namespace LostArk::Shared;
    if (!m_NumericBalanceWorker.valid() ||
        m_NumericBalanceWorker.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
    BALANCE_WORK_RESULT completed;
    try { completed = m_NumericBalanceWorker.get(); }
    catch (const std::exception& error) { Finish_NumericBalanceTransaction(false, error.what()); return; }
    if (m_NumericBalancePersisting)
    {
        m_NumericBalancePrepared.Failure = BALANCE_APPLY_RESULT::SAVE_FAILED;
        Finish_NumericBalanceTransaction(completed.Succeeded, std::move(completed.Status));
        return;
    }
    m_NumericBalancePrepared = std::move(completed);
    if (!m_NumericBalancePrepared.Succeeded)
    { Finish_NumericBalanceTransaction(false, m_NumericBalancePrepared.Status); return; }
    std::string status;
    try
    {
        std::unique_lock admissionLock{m_DataRevisionAdmissionMutex};
        {
            std::scoped_lock lock{m_SessionsMutex};
            const auto& prepared = *m_NumericBalancePrepared.Prepared;
            if (prepared.BaseNumericRevision != m_NumericBalanceStore.Get_NumericRevision() ||
                m_DataRevisionTransaction.Is_Active())
                status = "Active balance changed during validation";
            else
            {
                m_NumericBalanceSimulations.reserve(m_SharedGameRooms.size() + m_CharacterSelectArenas.size());
                for (const auto& [world, room] : m_SharedGameRooms) if (room) m_NumericBalanceSimulations.push_back(room);
                for (const auto& [owner, room] : m_CharacterSelectArenas) if (room) m_NumericBalanceSimulations.push_back(room);
                for (const auto& [matchId, room] : m_ColosseumMatches) if (room) m_NumericBalanceSimulations.push_back(room);
                for (const auto& room : m_NumericBalanceSimulations)
                    if (!room->Stage_NumericBalance(m_NumericBalanceTransactionSequence, prepared.Generation, status)) break;
                if (status.empty()) m_NumericAdmissionPaused = true;
            }
        }
        admissionLock.unlock();
        if (!status.empty()) { Finish_NumericBalanceTransaction(false, status); return; }
        m_NumericBalancePrepared.Failure = BALANCE_APPLY_RESULT::SAVE_FAILED;
        m_NumericBalancePersisting = true;
        m_NumericBalanceWorker = std::async(std::launch::async,
            [store = m_NumericBalanceStore, prepared = m_NumericBalancePrepared.Prepared]() {
                BALANCE_WORK_RESULT result;
                result.Failure = BALANCE_APPLY_RESULT::SAVE_FAILED;
                try { result.Succeeded = store.PersistPrepared(*prepared, result.Status); }
                catch (const std::exception& error) { result.Status = error.what(); }
                catch (...) { result.Status = "Numeric persistence failed"; }
                return result;
            });
    }
    catch (const std::exception& error) { Finish_NumericBalanceTransaction(false, error.what()); }
}

void LostArk::Server::CServerApp::Finish_NumericBalanceTransaction(const bool commit, std::string status)
{
    using namespace LostArk::Shared;
    const auto requester = m_NumericBalanceRequester;
    const auto sequence = m_NumericBalanceRequestSequence;
    const auto failure = m_NumericBalancePrepared.Failure == BALANCE_APPLY_RESULT::APPLIED ?
        BALANCE_APPLY_RESULT::INVALID_CHANGE : m_NumericBalancePrepared.Failure;
    std::vector<SESSION_ID> notify;
    {
        std::scoped_lock admissionLock{m_DataRevisionAdmissionMutex};
        std::scoped_lock lock{m_SessionsMutex};
        if (commit)
        {
            // Every room commits before the first room takes the next fixed step.
            for (const auto& room : m_NumericBalanceSimulations)
                if (!room->Commit_NumericBalance(m_NumericBalanceTransactionSequence))
                {
                    std::cerr << "Durable numeric balance could not activate in a staged room; restart is required.\n";
                    ::ExitProcess(ERROR_INVALID_STATE);
                }
            m_NumericBalanceStore.CommitPrepared(*m_NumericBalancePrepared.Prepared);
            m_pActiveGameplayGeneration = m_NumericBalancePrepared.Prepared->Generation;
            m_ActiveGameplayBootstrapContentRevision = m_NumericBalancePrepared.BootstrapRevision;
            m_ActiveNonValtanGameplayRevision = m_NumericBalancePrepared.NonValtanRevision;
            for (const auto& [id, binding] : m_GameplayBindingBySessionId)
                if (id != requester && binding.pSimulation) notify.push_back(id);
        }
        else for (const auto& room : m_NumericBalanceSimulations)
            room->Abort_NumericBalance(m_NumericBalanceTransactionSequence);
        m_NumericAdmissionPaused = false;
    }
    m_NumericBalanceSimulations.clear(); m_NumericBalancePrepared = {};
    m_NumericBalanceRequester = INVALID_SESSION_ID; m_NumericBalanceRequestSequence = 0u;
    m_NumericBalancePersisting = false;
    if (requester != INVALID_SESSION_ID)
        Send_NumericBalanceResult(requester, sequence, commit ? BALANCE_APPLY_RESULT::APPLIED : failure, std::move(status));
    if (commit)
        for (const auto id : notify) Send_NumericBalanceResult(id, 0u, BALANCE_APPLY_RESULT::APPLIED, {});
}
~~~~

<a id="file-server-private-serversnapshotfanout-cpp"></a>

### Server/Private/ServerSnapshotFanout.cpp

파일: C:/Users/tnest/Desktop/LostArk/Server/Private/ServerSnapshotFanout.cpp

각 index가 한 session의 framing/enqueue와 자기 결과만 쓰고 join 후 owner가 오류를 처리한다. payload와 결과의 수명은 모든 job 완료까지 유지된다. 각 session의 wire 송신 완료를 여기서 기다리지 않는다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
#include "ServerSnapshotFanout.h"
#include "ClientSession.h"
#include "Concurrency/WorkStealingJobSystem.h"

#include <algorithm>
#include <chrono>
#include <exception>

namespace
{
    using Clock = std::chrono::steady_clock;
    std::uint64_t Micros(const Clock::duration value)
    {
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(value).count());
    }
    struct SESSION_RESULT final
    {
        bool Sent = false;
        std::uint64_t Microseconds = 0;
    };
}

LostArk::Server::SNAPSHOT_FANOUT_RESULT LostArk::Server::Dispatch_WorldSnapshot(
    const std::span<const std::shared_ptr<CClientSession>> recipients,
    const std::span<const std::uint8_t> payload,
    Shared::Concurrency::WorkStealingJobSystem* jobs)
{
    const auto begin = Clock::now();
    SNAPSHOT_FANOUT_RESULT result;
    std::vector<SESSION_RESULT> completed(recipients.size());
    const auto execute = [&](const std::size_t index)
    {
        const auto start = Clock::now();
        // This API only builds/enqueues a frame. It does not wait for socket I/O.
        // Snapshot overflow is a coalesced/dropped update, not reliable failure.
        completed[index].Sent = !recipients[index] || recipients[index]->Send_Frame(
            Shared::PACKET_TYPE::S2C_WORLD_SNAPSHOT, payload);
        completed[index].Microseconds = Micros(Clock::now() - start);
    };
    if (jobs && recipients.size() > 1)
    {
        Shared::Concurrency::JobCounter counter;
        std::exception_ptr submissionError;
        try
        {
            for (std::size_t index = 0; index < recipients.size(); ++index)
            {
                jobs->Submit(counter, [&, index] { execute(index); });
                ++result.SubmittedJobs;
            }
        }
        catch (...) { submissionError = std::current_exception(); }
        // Always join before borrowed payload/recipient/result storage can die.
        jobs->Wait(counter);
        if (submissionError) std::rethrow_exception(submissionError);
    }
    else
    {
        for (std::size_t index = 0; index < recipients.size(); ++index)
            execute(index);
    }
    for (std::size_t index = 0; index < recipients.size(); ++index)
    {
        if (!recipients[index]) continue;
        ++result.Recipients;
        result.MaximumSessionMicroseconds = (std::max)(
            result.MaximumSessionMicroseconds, completed[index].Microseconds);
        if (!completed[index].Sent)
        {
            ++result.Failures;
            recipients[index]->Request_Close();
        }
    }
    result.ElapsedMicroseconds = Micros(Clock::now() - begin);
    return result;
}
~~~~

<a id="file-server-public-clientsession-h"></a>

### Server/Public/ClientSession.h

파일: C:/Users/tnest/Desktop/LostArk/Server/Public/ClientSession.h

기존 queue·reliable transaction API에 backend 선택과 IOCP participant 계약을 추가한다. session은 shared owner로 생성하며 service raw pointer는 ServerApp이 더 오래 소유한다. IOCP mutex는 operation 상태와 socket 취소 경계를 보호한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~cpp
#pragma once

#include "ServerIds.h"
#include "IocpService.h"

#include "Network/NetworkIds.h"
#include "Network/PacketFrame.h"
#include "Network/PacketStreamParser.h"
#include "Network/SessionDiagnostic.h"

#include <WinSock2.h>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace LostArk::Server
{
	class CServerGameplayContractRunner;

	class CClientSession;

	struct CLIENT_SESSION_RELIABLE_BATCH final
	{
		std::shared_ptr<CClientSession> pSession;
		std::vector<LostArk::Shared::PACKET_FRAME> Frames;
	};

	struct CLIENT_SESSION_OUTBOUND_METRICS final
	{
		std::size_t iCurrentQueuedFrameCount = 0;
		std::size_t iCurrentQueuedByteCount = 0;
		std::size_t iQueuedFrameHighWatermark = 0;
		std::size_t iQueuedByteHighWatermark = 0;
		std::uint64_t iReliableEnqueuedFrameCount = 0;
		std::uint64_t iReliableRejectedFrameCount = 0;
		std::uint64_t iSnapshotEnqueuedFrameCount = 0;
		std::uint64_t iSnapshotCoalescedFrameCount = 0;
		std::uint64_t iSnapshotDroppedFrameCount = 0;
		std::uint64_t iSentFrameCount = 0;
		std::uint64_t iSentByteCount = 0;
		std::uint64_t iSendFailureCount = 0;
		std::uint64_t iLastFrameSendMicroseconds = 0;
		std::uint64_t iMaximumFrameSendMicroseconds = 0;
	};

	struct CLIENT_SESSION_PEER_ENDPOINT final
	{
		std::string strAddress = "unknown";
		std::uint16_t iPort = 0u;
	};

	struct CLIENT_SESSION_CLOSE_DIAGNOSTIC final
	{
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON eReason =
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE;
		LostArk::Shared::PACKET_TYPE eLastInboundPacket =
			LostArk::Shared::PACKET_TYPE::INVALID;
		int iNativeErrorCode = 0;
		std::uint64_t iOccurredUnixMilliseconds = 0u;
		std::uint64_t iLastInboundUnixMilliseconds = 0u;
		std::size_t iQueuedFrameCountAtClose = 0u;
		std::size_t iQueuedByteCountAtClose = 0u;
		std::string strContext;
	};

	class CClientSession final : public IIocpParticipant,
		public std::enable_shared_from_this<CClientSession>
	{
		friend class CServerGameplayContractRunner;
        friend int Run_ServerCardMazeContractTests();
	public:
		// 수신 스레드가 완성한 한 프레임을 ServerApp에 전달하는 계약이다.
		using FRAME_HANDLER = std::function<void(
			SESSION_ID,
			const LostArk::Shared::PACKET_FRAME&)>;

		// 연결 종료를 ServerApp에 한 번만 알리는 계약이다.
		using CLOSED_HANDLER = std::function<void(
			SESSION_ID)>;

		/* A room-thread admission prepares every participant's reliable FIFO
		   before changing gameplay state. Destruction without Commit releases
		   the locks without publishing even one frame. No socket I/O occurs. */
		class RELIABLE_BATCH_TRANSACTION final
		{
		public:
			RELIABLE_BATCH_TRANSACTION();
			~RELIABLE_BATCH_TRANSACTION();
			RELIABLE_BATCH_TRANSACTION(const RELIABLE_BATCH_TRANSACTION&) = delete;
			RELIABLE_BATCH_TRANSACTION& operator=(const RELIABLE_BATCH_TRANSACTION&) = delete;
			bool Prepare(
				const std::vector<CLIENT_SESSION_RELIABLE_BATCH>& batches,
				std::string& status);
			void Commit() noexcept;
		private:
			struct LOCKED_QUEUE;
			std::vector<std::unique_ptr<LOCKED_QUEUE>> m_Queues;
		};

	public:
		CClientSession(
			SESSION_ID sessionId,
			SOCKET clientSocket,
			FRAME_HANDLER onFrame,
			CLOSED_HANDLER onClosed,
			SESSION_TRANSPORT_BACKEND backend = SESSION_TRANSPORT_BACKEND::SELECT_THREADS,
			CIocpService* iocpService = nullptr);

		~CClientSession();

		CClientSession(const CClientSession&) = delete;
		CClientSession& operator=(const CClientSession&) = delete;

	public:
		bool Start();
		void Request_Close(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
			int nativeErrorCode = 0,
			std::string_view context = {});
		// Stop receiving immediately, drain already queued reliable frames on the
		// sender worker, and close after the queue empties or a bounded terminal
		// drain expires. Used for typed terminal replies such as ROOM_FULL; the room thread never
		// waits for socket I/O.
		void Request_Close_After_Flush(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
			int nativeErrorCode = 0,
			std::string_view context = {});
		void Stop();

		// Server에서 Client로 한 게임 패킷 프레임을 전송한다.
		bool Send_Frame(
			LostArk::Shared::PACKET_TYPE packetType,
			std::span<const std::uint8_t> payload);

		void Bind_PlayerId(
			LostArk::Shared::PLAYER_ID playerId);

		[[nodiscard]] SESSION_ID Get_SessionId() const;

		[[nodiscard]] LostArk::Shared::PLAYER_ID Get_PlayerId() const;

		[[nodiscard]] bool Is_Open() const;
		[[nodiscard]] bool Is_Closing() const;

		[[nodiscard]] int Get_LastErrorCode() const;
		[[nodiscard]] CLIENT_SESSION_OUTBOUND_METRICS
			Get_OutboundMetrics() const;
		[[nodiscard]] const CLIENT_SESSION_PEER_ENDPOINT&
			Get_PeerEndpoint() const noexcept;
		[[nodiscard]] CLIENT_SESSION_CLOSE_DIAGNOSTIC
			Get_CloseDiagnostic() const;
		[[nodiscard]] std::uint64_t
			Get_LastInboundUnixMilliseconds() const noexcept;

	private:
		struct OUTBOUND_FRAME final
		{
			LostArk::Shared::PACKET_TYPE ePacketType =
				LostArk::Shared::PACKET_TYPE::INVALID;
			std::vector<std::uint8_t> Bytes;
		};

		enum class OUTBOUND_ENQUEUE_RESULT
		{
			QUEUED,
			COALESCED,
			DROPPED_SNAPSHOT,
			RELIABLE_OVERFLOW,
			CLOSED
		};

		// Small reliable combat events can burst while a LAN peer briefly stops reading.
		// Bound both events and bytes while preserving every reliable frame in FIFO.
		static constexpr std::size_t MAX_OUTBOUND_FRAME_COUNT = 4096u;
		static constexpr std::size_t RELIABLE_FRAME_RESERVE = 16u;
		static constexpr std::size_t MAX_OUTBOUND_BYTE_COUNT = 8u * 1024u * 1024u;
		static constexpr std::size_t RELIABLE_BYTE_RESERVE = 128u * 1024u;
		// Socket calls are nonblocking; temporary pressure is not a broken stream.
		static constexpr std::uint32_t TRANSPORT_POLL_MILLISECONDS = 100u;
		static constexpr std::uint32_t SEND_STALL_REPORT_MILLISECONDS = 250u;
		static constexpr std::uint32_t TERMINAL_DRAIN_TIMEOUT_MILLISECONDS = 2000u;
		static constexpr std::uint32_t SENDER_JOIN_TIMEOUT_MILLISECONDS = 2000u;

		void Receive_Loop();
		void Sender_Loop();

		bool Receive_Frame(
			LostArk::Shared::PACKET_FRAME& frame);
		OUTBOUND_ENQUEUE_RESULT Queue_OutboundFrame(
			LostArk::Shared::PACKET_TYPE packetType,
			std::vector<std::uint8_t> frameBytes);
		bool Configure_TransportOptions();
		void Close_Socket();
		void Record_InboundPacket(
			LostArk::Shared::PACKET_TYPE packetType) noexcept;
		void Record_TerminalDiagnostic(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
			int nativeErrorCode,
			std::string_view context);
		static CLIENT_SESSION_PEER_ENDPOINT Resolve_PeerEndpoint(
			SOCKET clientSocket) noexcept;

		// Sender worker only. Room and receive threads never call send directly.
		bool Send_All(std::span<const std::uint8_t> bytes,
			LostArk::Shared::PACKET_TYPE packetType = LostArk::Shared::PACKET_TYPE::INVALID);
		void Record_SendProgressDiagnostic(const char* eventName,
			LostArk::Shared::PACKET_TYPE packetType, std::size_t sentBytes,
			std::size_t totalBytes, std::uint64_t noProgressMilliseconds) noexcept;

		void Notify_Closed();

        bool Start_Iocp();
        bool Post_IocpReceive();
        bool Post_IocpSendLocked(int& error);
        void Kick_IocpSend() noexcept;
        void On_IocpCompleted(IOCP_OPERATION_KIND kind,
            std::span<const std::uint8_t> received, std::uint32_t bytes,
            std::uint32_t error) noexcept override;
        void On_IocpMaintenance() noexcept override;
        void Complete_IocpOperation() noexcept;
        void Handle_IocpReceive(std::span<const std::uint8_t> received,
            std::uint32_t bytes, std::uint32_t error);
        void Handle_IocpSend(std::uint32_t bytes, std::uint32_t error);
        void Fail_Iocp(LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
            int error, const char* context);
        void Wake_Sender() noexcept;


	private:
		const SESSION_ID m_iSessionId =
			INVALID_SESSION_ID;
		const CLIENT_SESSION_PEER_ENDPOINT m_PeerEndpoint;

		std::atomic<SOCKET> m_hClientSocket{ INVALID_SOCKET };

		std::atomic<int> m_iLastErrorCode{ 0 };
		std::atomic_bool m_isReceiveRunning{ false };
		std::atomic_bool m_isSendRunning{ false };
		std::atomic_bool m_closeAfterOutboundFlush{ false };
		std::atomic<std::uint64_t> m_iTerminalDrainStartTicks{ 0u };
		std::atomic_bool m_hasNotifiedClosed{ false };

		std::atomic<LostArk::Shared::PLAYER_ID> m_iPlayerId
		{
			LostArk::Shared::INVALID_PLAYER_ID
		};
		std::atomic<LostArk::Shared::PACKET_TYPE> m_eLastInboundPacket
		{
			LostArk::Shared::PACKET_TYPE::INVALID
		};
		std::atomic<std::uint64_t> m_iLastInboundUnixMilliseconds{ 0u };
		mutable std::mutex m_DiagnosticMutex;
		CLIENT_SESSION_CLOSE_DIAGNOSTIC m_CloseDiagnostic;

		LostArk::Shared::CPacketStreamParser m_StreamParser;
		std::thread m_ReceiveThread;
		std::thread m_SendThread;
		mutable std::mutex m_OutboundMutex;
		std::condition_variable m_OutboundCondition;
		std::condition_variable m_SenderExitCondition;
		std::deque<OUTBOUND_FRAME> m_OutboundFrames;
		std::size_t m_iQueuedOutboundBytes = 0u;
		CLIENT_SESSION_OUTBOUND_METRICS m_OutboundMetrics;
		bool m_hasSenderExited = true;
		std::atomic<std::uint64_t> m_iSendStallOrdinal{0u};

        const SESSION_TRANSPORT_BACKEND m_TransportBackend;
        CIocpService* const m_pIocpService;
        std::atomic_bool m_isIocpStarted{false};
        // Posting and cancellation share this lock. Never invoke a callback,
        // diagnostic, or gameplay operation while holding it.
        std::mutex m_IocpMutex;
        std::condition_variable m_IocpDrained;
        std::size_t m_iIocpOutstanding = 0;
        bool m_isIocpStopping = false;
        bool m_isIocpReceivePending = false;
        bool m_isIocpSendPending = false;
        std::shared_ptr<const std::vector<std::uint8_t>> m_IocpSendBytes;
        LostArk::Shared::PACKET_TYPE m_IocpSendPacket = LostArk::Shared::PACKET_TYPE::INVALID;
        std::size_t m_iIocpSendOffset = 0;
        std::chrono::steady_clock::time_point m_IocpSendStart{}, m_IocpLastProgress{};
        bool m_isIocpStalled = false;
        bool m_reportIocpStall = false;

		FRAME_HANDLER m_OnFrame;
		CLOSED_HANDLER m_OnClosed;
	};
}
~~~~

<a id="file-server-public-gameroom-h"></a>

### Server/Public/GameRoom.h

파일: C:/Users/tnest/Desktop/LostArk/Server/Public/GameRoom.h

생성자에 선택적 snapshot job pool 강한 참조를 추가한다. 게임 상태 owner는 그대로 room thread이고 pool은 snapshot fanout에서만 소비한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~cpp
#pragma once

#include "RoomCommand.h"
#include <atomic>
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"
#include "WorldBootstrap.h"
#include "GameplayCatalog.h"
#include "ItemCatalog.h"
#include "HonorTitleCatalog.h"
#include "VehicleCatalog.h"
#include "GuideCatalog.h"
#include "ValtanClearRewards.h"
#include "PlayerSkillSystem.h"
#include "CombatObjectRuntime.h"
#include "ColosseumThreatAssessment.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
#include "SpawnGroupBootstrap.h"
#include "SpawnGroupRuntime.h"
#include "MonsterBrain.h"
#include "NpcBehaviorRuntime.h"
#include "ValtanBrain.h"
#include "KoukuSaydonBrain.h"
#include "KoukuSaydonLogicRuntime.h"
#include "EncounterPropRuntime.h"
#include "EstherSkillSystem.h"
#include "Gameplay/EstherStrikeContract.h"
#include "Gameplay/MaharakaWaterpangContract.h"
#include "WorldDestructionBootstrap.h"
#include "WorldDestructionRuntime.h"
#include "Network/PacketFrame.h"
#include "Network/SessionDiagnostic.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace LostArk::Shared::Concurrency { class WorkStealingJobSystem; }

namespace LostArk::Server
{
	class CServerGameplayContractRunner;

	class CClientSession;

	/* A room never mutates an admitted gameplay catalog. The facade preserves
	   the established lookup surface while retaining immutable old generations
	   until every replicated occurrence releases its revision pin. */
	class CGameplayCatalogGenerations final
	{
	public:
		static constexpr std::size_t MAX_GENERATION_COUNT = 16u;

		CGameplayCatalogGenerations();
		bool Load();
		bool Initialize(
			const std::shared_ptr<const CGameplayCatalog>& initialGeneration);
		bool Stage(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);
		bool Commit(std::uint32_t transactionSequence) noexcept;
		bool Stage_NumericBalance(std::uint32_t transactionSequence,
			const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status);
		bool Commit_NumericBalance(std::uint32_t transactionSequence) noexcept;
		void Abort_NumericBalance(std::uint32_t transactionSequence) noexcept;

		void Abort(std::uint32_t transactionSequence) noexcept;
		void Collect_Garbage(
			const std::vector<LostArk::Shared::GameplayDataRevision>& livePins);

		[[nodiscard]] const CGameplayCatalog* Resolve(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept;
		[[nodiscard]] const CGameplayCatalog& Active() const noexcept;
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGeneration() const noexcept { return m_pActiveGeneration; }
		[[nodiscard]] std::size_t Get_GenerationCount() const noexcept
		{
			return m_Generations.size();
		}
		[[nodiscard]] std::uint16_t Get_ActiveGenerationEpoch() const noexcept
		{
			return m_iActiveGenerationEpoch;
		}

		const PLAYER_SKILL_DEFINITION* Find_Skill(
			LostArk::Shared::SKILL_ID skillId) const;
		const BOSS_RUNTIME_PROFILE* Find_Boss(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PART_DEFINITION>* Find_BossParts(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PATTERN_DEFINITION>* Find_BossPatterns(
			const std::string& encounterId) const;
		const BOSS_COMBAT_OBJECT_DEFINITION* Find_BossCombatObject(
			const std::string& archetypeId) const;
		const VALTAN_TIMELINE_DEFINITION* Find_ValtanTimeline(
			const std::string& encounterId) const;
		const VALTAN_TIMELINE_ROW* Find_ValtanTimelineRow(
			const std::string& encounterId, std::uint32_t commandId) const;
		const BOSS_PATTERN_ROTATION_DEFINITION* Find_BossPatternRotation(
			const std::string& encounterId, std::uint32_t gameplayPhase,
			std::uint32_t healthBar) const;
		const std::string& Find_IntroPatternId(
			const std::string& encounterId) const;
		const PLAYER_RUNTIME_PROFILE* Find_Player(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		std::uint32_t Find_DamageRatePercent(
			const std::string& damageProfileId) const;
		[[nodiscard]] const LostArk::Shared::GameplayDataRevision&
			Get_ActiveRevision() const noexcept;
		[[nodiscard]] const std::string& Get_Status() const noexcept;
		operator const CGameplayCatalog&() const noexcept { return Active(); }

	private:
		std::shared_ptr<const CGameplayCatalog> m_pActiveGeneration;
		std::shared_ptr<const CGameplayCatalog> m_pStagedGeneration;
		std::uint32_t m_iStagedTransactionSequence = 0u;
		std::uint16_t m_iActiveGenerationEpoch = 0u;
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_Generations;
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_NumericStagedGenerations;
		std::shared_ptr<const CGameplayCatalog> m_pNumericStagedActive;
		std::uint32_t m_iNumericTransactionSequence = 0u;

		std::string m_strStatus;
	};

	// The last completed outer room-loop iteration, shared by all rooms on the next tick.
	struct SERVER_ROOM_SCHEDULER_METRICS final
	{
		std::uint64_t iSampleUnixMilliseconds = 0u;
		std::uint64_t iPreviousLoopLatenessMicroseconds = 0u;
		std::uint64_t iMaximumLoopLatenessMicroseconds = 0u;
		std::uint64_t iScheduleResetCount = 0u;
	};

	struct SERVER_ROOM_PERFORMANCE_METRICS final
	{
		SERVER_NAVIGATION_PERFORMANCE_METRICS Navigation;
		SERVER_ROOM_SCHEDULER_METRICS Scheduler;
		std::uint64_t iTickCount = 0;
		std::uint64_t iLastTickMicroseconds = 0;
		std::uint64_t iMaximumTickMicroseconds = 0;
		std::size_t iLastIngressDepth = 0;
		std::size_t iIngressHighWatermark = 0;
		std::size_t iLastDrainedCommandCount = 0;
		std::size_t iLastRemainingCommandCount = 0;
		std::size_t iLastCleanupIngressDepth = 0;
		std::size_t iCleanupIngressHighWatermark = 0;
		std::size_t iLastDrainedCleanupCommandCount = 0;
		std::size_t iLastRemainingCleanupCommandCount = 0;
		std::uint64_t iDrainLimitedTickCount = 0;
		std::uint64_t iCoalescedMoveCommandCount = 0;
		std::uint64_t iCoalescedAimCommandCount = 0;
		std::uint64_t iDroppedBestEffortCommandCount = 0;
		std::uint64_t iRejectedReliableCommandCount = 0;
		std::uint64_t iRejectedCleanupCommandCount = 0;
		std::uint64_t iDeduplicatedCleanupCommandCount = 0;
		std::uint64_t iCancelledCommandCountByCleanup = 0;
		std::uint64_t iSnapshotEncodeCount = 0;
		std::uint64_t iSnapshotEncodeFailureCount = 0;
		std::uint64_t iLastSnapshotEncodeMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEncodeMicroseconds = 0;
		std::uint64_t iSnapshotEnqueueBatchCount = 0;
		std::uint64_t iSnapshotRecipientCount = 0;
		std::uint64_t iSnapshotEnqueueFailureCount = 0;
		std::uint64_t iLastSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iLastMaximumSessionEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSessionEnqueueMicroseconds = 0;
	};

	/* Receive-thread admission is not a bool: a full reliable queue, a room
	   runtime failure, a sealed private arena, and cleanup already in flight
	   require different session policy and diagnostics.  Keep success values
	   explicit too so best-effort shedding remains non-terminal. */
	enum class ROOM_COMMAND_ENQUEUE_RESULT : std::uint8_t
	{
		ACCEPTED,
		DROPPED_BEST_EFFORT,
		DEDUPLICATED_CLEANUP,
		REJECTED_INVALID_COMMAND,
		REJECTED_ROOM_NOT_READY,
		REJECTED_ROOM_SEALED,
		REJECTED_PENDING_CLEANUP,
		REJECTED_RELIABLE_CAPACITY,
		REJECTED_BINDING_MISSING
	};

	[[nodiscard]] constexpr bool Is_AcceptedRoomCommandEnqueueResult(
		const ROOM_COMMAND_ENQUEUE_RESULT result) noexcept
	{
		return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP == result;
	}

	struct SERVER_ROOM_RUNTIME_FAILURE final
	{
		std::uint32_t iServerTick = 0u;
		std::string strSource;
		std::string strDetail;
	};

	class CGameRoom final
	{
		friend class CServerGameplayContractRunner;
		friend int Run_ServerKoukuSupportSurfaceContractTests();
        friend int Run_ServerBingoContractTests();
		friend int Run_ServerCardMazeContractTests();
        friend int Run_ServerKoukuObjectOverlapContractTests();
		friend int Run_ServerVehicleRidingContractTests();
	public:
		explicit CGameRoom(
			LostArk::Shared::WORLD_ID worldId,
			std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration = {},
			const std::atomic_bool* pPreparationCancelled = nullptr,
			std::shared_ptr<LostArk::Shared::Concurrency::WorkStealingJobSystem> snapshotJobs = {});

		bool Enqueue(ROOM_COMMAND command);
		[[nodiscard]] ROOM_COMMAND_ENQUEUE_RESULT Enqueue_Detailed(
			ROOM_COMMAND command);
		[[nodiscard]] std::string Describe_EnqueueResult(
			ROOM_COMMAND_ENQUEUE_RESULT result) const;
		[[nodiscard]] bool Try_GetRuntimeFailure(
			SERVER_ROOM_RUNTIME_FAILURE& outFailure) const;
		void Tick(float fixedDeltaSeconds,
			const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics = {});
		// Room-thread only. At most one sampled line is retained until ServerApp writes it.
		[[nodiscard]] std::string Take_PerformanceDiagnostic();
		bool Try_DequeueWorldTransfer(
			SERVER_WORLD_TRANSFER_REQUEST& outTransfer);
		bool Configure_ColosseumMatch(std::uint64_t matchId, const std::vector<SESSION_ID>& sessions);
		void Remove_ColosseumExpectedSession(SESSION_ID sessionId);

		[[nodiscard]] LostArk::Shared::WORLD_ID Get_WorldId() const
		{
			return m_eWorldId;
		}

		[[nodiscard]] bool Is_Ready() const { return m_isReady; }
		[[nodiscard]] const std::string& Get_Status() const
		{
			return m_strStatus;
		}
		/* Room-thread only. Stage is allowed to fail before publication; Commit
		   is a bounded pointer swap after every process room has staged. */
		bool Stage_NumericBalance(std::uint32_t transactionSequence,
			const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status);
		bool Commit_NumericBalance(std::uint32_t transactionSequence) noexcept;
		void Abort_NumericBalance(std::uint32_t transactionSequence) noexcept;
		bool Stage_GameplayGeneration(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);
		bool Commit_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;
		void Abort_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGameplayGeneration() const noexcept
		{
			return m_GameplayCatalog.Get_ActiveGeneration();
		}
		[[nodiscard]] const CGameplayCatalog* Resolve_GameplayGeneration(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept
		{
			return m_GameplayCatalog.Resolve(revision);
		}
		/* Room-thread only. Decision observability reads the same authoritative
		   brain and immutable selector generation as the Valtan simulation. */
		bool Build_ValtanDecisionTraceResponse(
			const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request,
			LostArk::Shared::S2C_VALTAN_DECISION_TRACE_RESPONSE& outResponse,
			std::string& status) const;
		[[nodiscard]] SERVER_ROOM_PERFORMANCE_METRICS
			Get_PerformanceMetrics() const;

		// Room thread only. A sealed private arena rejects every later command.
		[[nodiscard]] bool Try_SealPrivateArenaForRetirement();
		// Room thread only. Removes the source player before the target room can
		// process its queued ENTER_WORLD and bind the same session again.
		[[nodiscard]] bool Commit_WorldTransferDeparture(SESSION_ID sessionId);
		// Room-thread only, while ServerApp holds its session-binding mutex.
		[[nodiscard]] bool Has_PersonalGuideOwner(const std::shared_ptr<CClientSession>& ownerSession) const;
		bool Transfer_PartyTo(CGameRoom& target,
			const std::vector<SESSION_ID>& leaderFirstSessionIds,
			LostArk::Shared::PARTY_TRANSFER_RESULT& outResult, std::string& status,
			const std::string& raidReturnNpcPlacementId = {},
            const std::string& spawnPlacementOverrideId = {});
		bool Transfer_ColosseumMatchTo(CGameRoom& target,
			const std::vector<SESSION_ID>& orderedSeats, std::uint64_t matchId, std::string& status);
		void Notify_ColosseumTransferResult(bool committed);
		[[nodiscard]] bool Try_SealColosseumForRetirement();
		void Notify_PartyTransferFailure(SESSION_ID sessionId,
			std::uint32_t requestSequence, LostArk::Shared::WORLD_ID targetWorldId,
			LostArk::Shared::PARTY_TRANSFER_RESULT result);

	private:
		void Mark_RuntimeFailure(std::string_view source);
		std::size_t Count_HumanPlayers() const;
		void Initialize_Guide();
		bool Build_GuidePlayer(LostArk::Shared::PLAYER_ID playerId, LostArk::Shared::NET_ENTITY_ID entityId,
			float x, float y, float z, SERVER_PLAYER& outPlayer) const;
		bool Find_GuideLanding(const SERVER_PLAYER& guide, float x, float y, float z, SERVER_NAV_POINT& point,
			const SERVER_PLAYER* anchor = nullptr) const;
		bool Start_Guide(const SERVER_PLAYER& inviter, LostArk::Shared::NET_ENTITY_ID target);
		void Handle_GuideControl(SESSION_ID sessionId, const LostArk::Shared::C2S_GUIDE_CONTROL& request);
		void Broadcast_GuideOwnership();
		void Broadcast_GuideState(const LostArk::Shared::S2C_GUIDE_STATE& message);
		void Update_Guides(float seconds);
		void Update_Colosseum(float seconds);
		void Handle_ColosseumRecruit(SESSION_ID sessionId, const LostArk::Shared::C2S_COLOSSEUM_RECRUIT& request);
		LostArk::Shared::S2C_COLOSSEUM_MATCH_STATE Build_ColosseumState() const;
		void Broadcast_ColosseumState();
        float Predict_GuideContactRisk(const SERVER_PLAYER& guide, float x, float z);
		void Guide_AnchorArrived(const SERVER_PLAYER& anchor, bool localMapTravel = false);
		void Guide_ChatCommand(const SERVER_PLAYER& sender, const std::string& text);
		void Remove_Guide(SESSION_ID ownerSessionId, bool publish = true);
		void Suspend_PersonalGuide(SESSION_ID ownerSessionId);
		void Resume_PersonalGuide(SESSION_ID ownerSessionId, LostArk::Shared::WORLD_ID sourceWorld);
		void Seed_GuideSpaceEntries(SESSION_ID ownerSessionId, const SERVER_PLAYER& anchor);
		void Prune_GuideSpacePrompts(SESSION_ID ownerSessionId);
		void Queue_GuidePrompt(SESSION_ID ownerSessionId, const GUIDE_TRIGGER& trigger);
		void Execute_PlayerMove(SERVER_PLAYER& player, const LostArk::Shared::C2S_MOVE& move);
		bool Execute_PlayerSkill(SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& skill);
		struct STAGED_PLAYER_ENTRY final
		{
			std::shared_ptr<CClientSession> pSession;
			SERVER_PLAYER Player;
			std::vector<LostArk::Shared::PACKET_FRAME> Frames;
		};
		bool Stage_PlayerEntry(const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			std::span<const STAGED_PLAYER_ENTRY> precedingEntries,
			STAGED_PLAYER_ENTRY& staged,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outReason, std::string& status,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {},
			const SERVER_PURSE& carriedPurse = {}, bool hasCarriedCharacterState = false);
		bool Build_PlayerEntryFrames(STAGED_PLAYER_ENTRY& entry,
			std::span<const STAGED_PLAYER_ENTRY> batch, std::string& status);
		void Commit_PlayerEntry(const STAGED_PLAYER_ENTRY& entry);
		void Flush_PartyTransferResults();
		void Handle_Register(const std::shared_ptr<CClientSession>& session);
		bool Join(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {},
			const SERVER_PURSE& carriedPurse = {},
			LostArk::Shared::WORLD_ID sourceWorld = LostArk::Shared::WORLD_ID::BERN,
			bool hasCarriedCharacterState = false);
		void Leave(
			SESSION_ID sessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason, bool publishDeparture = true);
		void Close_SessionForBindingFailure(
			SESSION_ID sessionId,
			std::string_view packetName,
			std::string_view validation);
		void Handle_Move(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_MOVE& move);
		[[nodiscard]] bool Is_BufferableComboAction(
			const SERVER_PLAYER& player) const;
		/* A move goal inside the running skill's authored move-cancel window
		ends the action now instead of waiting out the recovery pose. */
		[[nodiscard]] bool Is_MoveCancellableAction(
			const SERVER_PLAYER& player) const;
		[[nodiscard]] bool Commit_MoveGoal(
			SERVER_PLAYER& player, float goalX, float goalZ);
		void Commit_PendingPlayerCommand(
			SERVER_PLAYER& player, std::uint32_t actionStartTick);
		void Handle_UseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SKILL& useSkill);
		void Handle_ReleaseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill);
		void Handle_UpdateSkillAim(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPDATE_SKILL_AIM& updateSkillAim);
		void Handle_UseEstherSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ESTHER_SKILL& useEstherSkill);
		/* Debug F1 Esther summon by name: same caster lock and summon timeline as
		the slot path, without the gauge or the world roster. Release ignores it. */
		void Handle_DebugUseEsther(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_USE_ESTHER& request);
		/* The caster the session owns, if it may start an Esther call right now:
		bound, idle, on its feet and not riding. */
		SERVER_PLAYER* Find_EstherCaster(SESSION_ID sessionId, const char* pCommandName);
		/* Queues the summon forward along the aim and locks the caster into
		ESTHER_CAST. The gauge decision is the caller's. */
		void Begin_EstherCall(
			SERVER_PLAYER& caster,
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			float aimX,
			float aimZ);
		void Handle_UseSquareHole(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SQUAREHOLE& useSquareHole);
		/* The landing of a square hole: the world's disabled "squarehole.<id>" movePlayer
		   row, admitted with the debug-teleport ground/height/collision rules. False when
		   the world has no such row or the landing is not standable for this player. */
		bool Resolve_SquareHoleDestination(
			const SERVER_PLAYER& player,
			std::uint16_t squareHoleId,
			SERVER_NAV_POINT& ground);
		/* The song lock ended: land the player at the destination, or leave them in place. */
		void Finish_SquareHoleSong(SERVER_PLAYER& player);
		bool Spawn_EstherSummon(
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			LostArk::Shared::PLAYER_ID casterPlayerId,
			float positionX,
			float positionY,
			float positionZ,
			float yawDegrees);
		void Update_PendingEstherSummons(float fixedDeltaSeconds);
		void Apply_EstherStrikeHits(SERVER_WORLD_ENTITY& summon, std::uint32_t serverTick);
		void Open_EstherZone(const LostArk::Shared::EstherStrike::ZONE& zone, float positionX, float positionZ, std::uint32_t serverTick);
		void Update_EstherZones(std::uint32_t serverTick);
		void Handle_RevivePlayer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer);
		/* Debug/Development-build test aid only -- zeroes the caster's own HP and
		sets PLAYER_ACTION_STATE::DEAD so a death-screen tester does not have to
		survive a real hit. Real body is compiled out in Release, matching
		Evaluate_ValtanAudition's convention. */
		void Handle_DebugKillSelf(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KILL_SELF& debugKillSelf);
		/* Debug-only Character Select audition entry. This stages the ordinary
		Server world-transfer transaction; it never changes a Client level directly. */
		void Handle_DebugEnterKakulSaydonArena(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_ENTER_KAKULSAYDON_ARENA& request);
		/* Debug-only authored waypoint audition. The placement must be a Kakul
		playerSpawn waypoint and Server navigation remains the position authority. */
		void Handle_DebugTeleportToPlacement(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_PLACEMENT& request);
		void Handle_DebugTeleportToPosition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugTeleportToPosition(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugReturnToKoukuStart(
			SERVER_PLAYER& player, std::uint32_t requestSequence);
		void Reset_PlayerForDebugTeleport(SERVER_PLAYER& player);
		void Handle_DebugMarioJump(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT Apply_DebugMarioJump(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		void Handle_MarioMove(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_MOVE& request);
		void Handle_MarioReturn(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_RETURN& request);
		LostArk::Shared::S2C_MARIO_RETURN_RESULT Apply_MarioReturn(
			SERVER_PLAYER& player, const LostArk::Shared::C2S_MARIO_RETURN& request);
		bool Resolve_MarioReturnDestination(const SERVER_PLAYER& player, SERVER_NAV_POINT& destination) const;
		static void Reset_MarioContactAction(SERVER_PLAYER& player);
		SERVER_TRIGGER_MOVE_ENTRY_RESULT Begin_MarioTriggerMove(
			const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player,
			std::uint32_t actionStartTick);
		void Update_MarioControlState(SERVER_PLAYER& player);
		std::uint8_t Begin_MarioStageObjects(std::uint8_t stage);
		void Begin_MarioBallChallenge(SERVER_PLAYER& player);
		std::uint8_t Mario_MatchingBallCount(const SERVER_PLAYER& player) const;
		std::uint8_t Mario_MarkerColor(LostArk::Shared::NET_ENTITY_ID targetId) const;
		void Reset_MarioStageObjects(std::uint8_t stage);
		void Cleanup_EmptyMarioStages();
		void Update_MarioMoveGoal(SERVER_PLAYER& player, std::uint32_t updateTick);
		bool Configure_MarioRail(SERVER_PLAYER& player, const std::string& arrivalPlacementId);
		/* Debug F1 clown/player avatar toggle: swaps only the replicated
		madness form of this session's player; Release answers REJECTED_DISABLED. */
		/* Debug F1 bingo check: paints cells and promotes completed lines. The
		board replicates on the world snapshot, so there is no result message. */
		void Handle_DebugBingoFill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_FILL& request);
		/* Debug F1 bingo bomb: marks this session's own player. The bomb
		rides the world snapshot, so there is no result message. */
		void Handle_DebugBingoBomb(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_BOMB& request);
		/* Debug F1 bingo hammer: rolls one of the twenty row/column anchors
		and starts the sweep. The hammer rides the world snapshot. */
		void Handle_DebugBingoHammer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_HAMMER& request);
		/* Debug F1 "Normal Monster 1/2" (Kouku Book1/Book2, Valtan Stage_1/Stage_2):
		removes the mapped wave group's live monsters, resets the group and starts it
		over at its authored anchors. Release ignores it; the monsters ride the world
		snapshot, so there is no result message. */
		void Handle_DebugResummonWaveMonsters(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_RESUMMON_WAVE_MONSTERS& request);
		void Handle_DebugSetMadnessForm(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT Apply_DebugMadnessForm(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		/* H key riding toggle for this session's player. The verdict is sent
		back; the ridden vehicle itself rides the world snapshot. */
		void Handle_SetVehicleRiding(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT Apply_SetVehicleRiding(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		/* True while nothing the player is doing forbids a vehicle underneath. */
		bool Can_RideVehicle(const SERVER_PLAYER& player) const;
		/* Title window change for this session's player. The verdict is sent back; the
		worn title itself rides the world snapshot. */
		void Handle_SetHonorTitle(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		LostArk::Shared::S2C_SET_HONOR_TITLE_RESULT Apply_SetHonorTitle(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		/* Metres per second the player walks at: the ridden vehicle's speed, or
		the class speed scaled by its held stance. */
		float Resolve_PlayerMoveSpeed(const SERVER_PLAYER& player) const;
		/* Dismounts every player the world, catalog or current state no longer
		lets ride. Runs once per tick before the snapshot is committed. */
		void Enforce_VehicleRidingState();
		/* Bern voyage ships (EFTable_VoyageShip 8200..8208) sail on the BernSea navigation region. Boarding
		   moves the player to the nearest open sea cell and keeps the pier position; leaving the ship, or any
		   forced dismount, brings the player back to that pier position. Begin returns false when no sea cell
		   lies within reach (the player is not at a harbour). */
		bool Begin_ShipVoyage(SERVER_PLAYER& player, LostArk::Shared::VEHICLE_ID vehicleId);
		void End_ShipVoyage(SERVER_PLAYER& player, const char* reason);
		/* A skill press while mounted. Only a skill of the ridden vehicle starts,
		from an idle mount, off cooldown and with a newer sequence; it faces the
		player's current yaw. */
		bool Try_StartVehicleSkill(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command);
		/* Advances a running vehicle skill one fixed tick: authored root motion is
		clamped to walkable ground and collision, and the action ends at its length. */
		void Update_VehicleSkill(SERVER_PLAYER& player, float fixedDeltaSeconds);
		void End_VehicleSkill(SERVER_PLAYER& player);
		/* One quick-slot press while this session's player shows an interaction
		HUD. DANCE answers the open pose window; the other modes only record the
		press until their skills own a Server judgement. */
		void Handle_InteractionSlot(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACTION_SLOT& request);
		/* Debug F1 "HUD Mode: MARIO / MAZE / Clear": forces one of the two
		modes whose gimmick has no Server trigger yet. Release answers
		REJECTED_DISABLED. */
		void Handle_DebugSetKoukuHudMode(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT Apply_DebugKoukuHudMode(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		/* Every tick: madness maximum from the encounter policy, clown hold
		expiry, and the interaction HUD mode plus slot layout per player. */
		void Update_KoukuPlayerModes(std::uint32_t serverTick);
		void Apply_KoukuGateEntryCard(SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss);
		void Handle_ChangeCharacterClass(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT Apply_CharacterClassChange(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		void Handle_SpawnWorldEntity(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request);
		/* Debug-only. Moves a live Valtan onto an authored health-bar threshold
		so CValtanBrain judges the crossing itself on a later fixed tick. The
		room never starts a pattern, breaks a wall or plays a cue directly.
		Evaluate owns the decision and the boss mutation and is what the contract
		tests drive; Handle only resolves the session and answers it. */
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		void Handle_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStart(
				SESSION_ID sessionId,
				const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStopAfterCurrent(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		void Handle_ValtanPatternFlowStart(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request);
		void Handle_ValtanPatternFlowStopAfterCurrent(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request);
		void Handle_KoukuRaidRequest(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request);
		bool Is_KoukuRaidInputBlocked() const;
		struct KOUKU_RAID_RUN final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST Request;
			LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE State;
			std::shared_ptr<const CGameplayCatalog> pCatalog;
			std::optional<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST> PriorAuditionRequest;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT> PriorAuditionResult;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE> PriorAuditionLifecycle;
			std::vector<LostArk::Shared::PLAYER_ID> PlayerIds;
			std::string strEntryTriggerSequenceId;
			std::set<std::string> CompletedArrivals;
			LostArk::Shared::NET_ENTITY_ID iPrimaryBossId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iAuditionRequestSequence = 0u, iAuditionEpoch = 0u, iNextEntryTick = 0u;
			bool bClearCinematic = false, bEntryRunning = false, bGate3CombatEntered = false;
            bool bClearedBossPreparation = false;
            bool bGateVoteEntry = false;
            bool bBingoSpecialRunning = false;
            std::uint32_t iGate3ClearTick = 0u;
		};
		KOUKU_RAID_RUN m_KoukuRaid;
		std::uint32_t m_iNextKoukuRaidEpoch = 1u;
		std::map<SESSION_ID, std::pair<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST, LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE>> m_KoukuRaidReceipts;
		bool Is_KoukuRaidRunning() const;
		bool Start_KoukuBingoSpecialPattern(std::uint32_t tick);
		bool Build_KoukuRaidState(LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE& state) const;
		void Broadcast_KoukuRaidState();
		void Update_KoukuRaid(std::uint32_t tick);
		void Notify_KoukuRaidBossDeath(const SERVER_WORLD_ENTITY& boss, std::uint32_t tick);
		void Stop_KoukuRaid(std::string reason, bool completed = false);
		bool Begin_KoukuRaidPreparation(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason, bool clearedGateBoss = false, bool gateVoteEntry = false);
		bool Apply_KoukuRaidReadiness(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason);
		bool Begin_KoukuRaidCinematic(const std::string& gateId, bool clear, std::uint32_t tick);
		bool Advance_KoukuRaidGate(std::uint8_t nextGate, bool restart);
		bool Start_KoukuRaidCombat(std::uint32_t tick, const std::string& preflightGateId = {});
        bool Build_KoukuRaidEntryRequest(const KOUKU_RAID_GATE_DEFINITION& gate, std::uint32_t entryIndex,
            LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		bool Enter_KoukuRaidCombat(std::uint8_t gate, std::uint32_t tick);
		bool Start_KoukuRaidEntry(std::uint32_t tick);
		void Handle_KoukuSaydonDraftChunk(SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK& chunk);
		void Handle_KoukuSaydonPatternAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_RESULT
			Evaluate_KoukuSaydonPatternAudition(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request,
				LostArk::Shared::
					S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& outResult, bool continueRaid = false,
                const std::vector<SERVER_WORLD_ENTITY>* stagedBosses = nullptr);
		SERVER_WORLD_ENTITY* Find_KoukuSaydonAuditionBoss();
		/* The live arena boss a Debug audition scope names: the Gate 1 Kouku or
		a gate boss raised from a disabled placement. Null when that placement
		is not currently spawned. */
		SERVER_WORLD_ENTITY* Find_KoukuSaydonArenaBoss(
			const std::string& placementId,
			const std::string& archetypeId);
		bool Update_KoukuSaydonBoss(
			SERVER_WORLD_ENTITY& boss, std::uint32_t serverTick);
		/* Broadcasts the cues a Logic tick produced, inserts follow-up patterns
		after the running audition slot, and returns true when a window asked
		the running pattern to end now (the brain commits it as COMPLETED). */
		struct KOUKU_PENDING_MECHANIC_TRIGGER final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iPatternSequence = 0u;
			BOSS_PATTERN_MECHANIC_TRIGGER Trigger;
		};
		std::vector<KOUKU_PENDING_MECHANIC_TRIGGER> m_PendingKoukuMechanicTriggers;
		[[nodiscard]] bool Commit_KoukuAlbionAirborne(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Commit_KoukuMechanicTriggers(std::uint32_t serverTick);
		void Update_KoukuActorContacts(SERVER_WORLD_ENTITY& actor, const BOSS_PATTERN_DEFINITION& pattern,
			const CGameplayCatalog& product, std::uint32_t serverTick);
		void Update_KoukuPursuitProjectiles(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window, const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Update_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Clear_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss, KOUKUSAYDON_LOGIC_LEDGER& ledger);
		void Update_KoukuRandomVolley(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window,
			const CGameplayCatalog& catalog, std::uint32_t serverTick, bool hasAlivePlayers);
		void Update_KoukuGazeClones(std::uint32_t serverTick);
		[[nodiscard]] bool Update_KoukuSummonTriggers(SERVER_WORLD_ENTITY& clone,
			const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		bool Apply_KoukuLogicOutput(
			const KOUKUSAYDON_LOGIC_OUTPUT& output,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		enum class KOUKUSAYDON_PATTERN_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct KOUKUSAYDON_PATTERN_AUDITION_MEMBER final
		{
			std::string strMemberId;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
			std::vector<std::string> PatternIds;
			std::vector<std::uint32_t> TransitionTicks;
			std::size_t iPatternIndex = 0u;
			std::uint32_t iNextStartTick = 0u;
			std::uint32_t iScheduledStartTick = 0u;
			std::uint32_t iPatternSequence = 0u;
			bool bCompleted = false;
			bool bOwnsPlayerMode = false;
			KOUKUSAYDON_LOGIC_LEDGER LogicLedger;
			// The entry root owns its portal across the completion-driven children.
			std::optional<SERVER_WORLD_ENTITY> MarioEntryAnchor;
			std::string strMarioEntryPatternId;
			std::uint32_t iMarioEntryStartTick = 0u;
			std::uint8_t iMarioEntryStage = 0u;
			bool bMarioEntryConsumed = false;
			// Pin the successful entrant, not whichever player is present later.
			bool bMarioSoloReturnRequired = false;
			LostArk::Shared::PLAYER_ID iMarioEntrantPlayerId = 0u;
			SESSION_ID iMarioEntrantSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iMarioEntrantNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			bool bMarioReturnCompleted = false;
			std::size_t iCompletionChainFirstIndex = 0u;
			std::uint32_t iCompletionChainCount = 0u;
			std::uint32_t iCompletionChainCompleted = 0u;
			std::string strCompletionChainSuccessPatternId;
			bool bCompletionChainStarted = false;
            bool bParentSequenceStarted = false;
            bool bParentSequenceLoops = false;
            std::size_t iParentLoopIndex = 0u;
            std::size_t iParentLastIndex = 0u;
			bool bCompletionChainAwaitingReturn = false;
			bool bCompletionChainSuccessQueued = false;
			std::uint32_t iNextWorldCue = 1u;
			std::unordered_map<std::string, std::string> WorldCueByInstance;
			std::unordered_map<std::string, std::string> WorldCueByOccurrence;
		};
		// Stage completion releases the actor clock, while these occurrence rows retain theirs.
		struct KOUKU_PATTERN_TAIL final
		{
			std::shared_ptr<SERVER_WORLD_ENTITY> pOwner;
			KOUKUSAYDON_PATTERN_AUDITION_MEMBER Member;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iLastUpdateTick = 0u;
		};
		struct KOUKU_SCHEDULED_SUPPORT_SURFACE final
		{
			std::string strMemberId;
			std::uint32_t iStartTick = 0u;
			std::uint32_t iEndTick = 0u;
			SERVER_NAVIGATION_SUPPORT_SURFACE Surface;
		};
		struct KOUKUSAYDON_PATTERN_AUDITION_STATE final
		{
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iCommonStartTick = 0u;
			LostArk::Shared::GameplayDataRevision PinnedGameplayRevision{};
			std::uint32_t iPinnedSourceRevision = 0u;
			/* Global gameplay authority remains PinnedGameplayRevision. The
			   separate Kouku Product source owns pattern/logic rows for this run. */
			std::shared_ptr<const CGameplayCatalog> pProductGeneration;
			std::vector<KOUKUSAYDON_PATTERN_AUDITION_MEMBER> Members;
			std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> WorldPlays;
			std::vector<KOUKU_SCHEDULED_SUPPORT_SURFACE> SupportSchedule;
			std::vector<KOUKU_PATTERN_TAIL> Tails;
		};
		KOUKUSAYDON_PATTERN_AUDITION_MEMBER* Find_KoukuAuditionMember(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence = 0u);
		KOUKUSAYDON_LOGIC_LEDGER* Active_KoukuPlayerLedger();
		[[nodiscard]] const CGameplayCatalog* Resolve_KoukuProductCatalog() const noexcept;
		void Prepare_KoukuAuditionTick(std::uint32_t serverTick);
		void Update_KoukuPatternTails(std::uint32_t serverTick);
		SERVER_WORLD_ENTITY* Find_KoukuOccurrenceOwner(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence);
		bool Retain_KoukuPatternTail(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			const SERVER_WORLD_ENTITY& sourceOwner, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
        bool Start_KoukuParentSequence(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
            SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		bool Start_KoukuCompletionChain(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		void Update_KoukuMarioEntry(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member, std::uint32_t serverTick);
		void Commit_KoukuMarioEntries();
		bool Commit_KoukuMarioPhasePlayers(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Complete_KoukuMarioReturn(SERVER_PLAYER& player,
			const std::string& sourcePlacementId, std::uint32_t updateTick);
		void Queue_KoukuCompletionChainSuccess(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			std::uint32_t serverTick);
		struct KOUKU_PENDING_MARIO_ENTRY final { std::string strMemberId; LostArk::Shared::PLAYER_ID iPlayerId; std::uint32_t iRootStartTick; std::uint8_t iStage = 0u; };
		std::vector<KOUKU_PENDING_MARIO_ENTRY> m_PendingKoukuMarioEntries;
		bool Enter_MarioFromPattern(SERVER_PLAYER& player, std::uint8_t stage);
		bool Refresh_KoukuSupportSurfaces(std::uint32_t serverTick);
		bool Build_KoukuBundleState(LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE& message) const;
		void Broadcast_KoukuBundleState(LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state);
		void Broadcast_OwnedWorldSequence(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& message);
		void Stop_KoukuWorldOwner(const std::string& memberId = {}, bool finished = false);
		// Survives natural Pattern completion; reset/cancel and HP zero own removal.
		struct KOUKU_DAMAGEABLE_WORLD_CUE final
		{
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY Play;
			LostArk::Shared::NET_ENTITY_ID iBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			bool bCancelled = false;
			std::uint32_t iNextMadnessTick = 0u;
			BOSS_ENCOUNTER_MADNESS_POLICY MadnessPolicy;
			std::uint8_t iMadnessSource = 0u; // 1 circus ball, 2 odd doll
		};
		std::vector<KOUKU_DAMAGEABLE_WORLD_CUE> m_KoukuDamageableWorldCues;
		std::vector<SERVER_WORLD_ENTITY> m_PendingKoukuWorldBodies;
		bool Stage_KoukuWorldBody(const BOSS_PATTERN_WORLD_COMBAT_BODY& body,
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play, bool authoredMadness = false);
		void Cancel_KoukuWorldBodies(const std::string& memberId = {}, SESSION_ID ownerSession = INVALID_SESSION_ID);
		void Update_KoukuWorldBodies(std::uint32_t serverTick);

		struct KOUKUSAYDON_PATTERN_AUDITION_RECEIPT final
		{
			LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT Result;
			std::optional<LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};

		void Queue_KoukuSaydonPatternAuditionLifecycle(
			const std::string& patternId,
			std::uint32_t patternSequence,
			std::uint32_t stageIndex,
			LostArk::Shared::
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {}, LostArk::Shared::NET_ENTITY_ID bossId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		bool Flush_KoukuSaydonPatternAuditionLifecycle();
		void Clear_KoukuSaydonPatternAudition(bool completed = false, std::string reason = {});
		SERVER_WORLD_ENTITY* Find_AuditionBoss();
		SERVER_WORLD_ENTITY* Find_AuditionBoss(
			const std::string& placementId);
		bool Has_EngagedAuditionPlayer(const SERVER_WORLD_ENTITY& boss) const;
		bool Build_ValtanBossOnlyAuditionReset(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			SERVER_WORLD_ENTITY& outBoss,
			std::string& status);
		bool Reset_ValtanBossOnlyAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		bool Reset_ValtanAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		enum class VALTAN_PATTERN_ID_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE,
			COMPLETED_HOLD,
			IDLE_HOLD
		};

		struct VALTAN_PATTERN_ID_AUDITION_STATE final
		{
			VALTAN_PATTERN_ID_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0u;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bResetlessContinuation = false;
			bool bReportedWaitingForPlayer = false;
			// A live predecessor has no Client Play request to report a lifecycle for.
			bool bAdoptedLivePredecessor = false;
			// Keep only the current Flow occurrence on its existing ordered Brain path.
			std::optional<BOSS_PATTERN_SEQUENCE_DEFINITION> AdoptedFlowSequence;
		};

		struct VALTAN_NEXT_PATTERN_RESERVATION final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iPredecessorPatternSequence = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bReportedWaitingForPlayer = false;
		};

		struct VALTAN_NEXT_PATTERN_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
		};

		[[nodiscard]] bool Is_ValtanPatternIdAuditionRunning() const noexcept;
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanNextPatternControl(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		LostArk::Shared::VALTAN_AUDITION_RESULT Adopt_ValtanLiveNextPattern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			SERVER_WORLD_ENTITY& boss);
		void Cancel_ValtanNextPatternReservation(std::string reason);
		void Cancel_ValtanPatternIdAudition(std::string reason);
		void Try_PromoteValtanNextPattern(SERVER_WORLD_ENTITY& boss);
		bool Prepare_ValtanPatternIdAuditionBeforeBrain(SERVER_WORLD_ENTITY& boss);
		bool Refresh_ValtanPatternIdAuditionState();
		void Queue_ValtanAuditionLifecycle(
			SESSION_ID ownerSessionId,
			std::uint32_t requestSequence,
			std::uint32_t roomEpoch,
			std::uint32_t patternSequence,
			const std::string& patternId,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanNextPatternLifecycle(
			const VALTAN_NEXT_PATTERN_RESERVATION& reservation,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		bool Flush_ValtanPatternIdAuditionLifecycle();

		enum class VALTAN_PATTERN_FLOW_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct VALTAN_PATTERN_FLOW_AUDITION_STATE final
		{
			VALTAN_PATTERN_FLOW_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_FLOW_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::uint32_t iFirstPatternSequence = 0u;
			std::size_t iStartSlotIndex = 0u;
			std::size_t iReportedSequenceIndex =
				(static_cast<std::size_t>(-1));
			std::uint32_t iReportedPatternSequence = 0u;
			bool bReportedPausedForRevive = false;
			bool bStopAfterCurrent = false;
			std::string strBossPlacementId;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strStartSlotId;
			std::vector<LostArk::Shared::VALTAN_PATTERN_FLOW_SLOT_WIRE> Slots;
			BOSS_PATTERN_SEQUENCE_DEFINITION Sequence;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		};

		[[nodiscard]] bool Is_ValtanPatternFlowRunning() const noexcept;
		[[nodiscard]] const BOSS_PATTERN_SEQUENCE_DEFINITION*
			Resolve_ValtanPatternFlowSequence(
				const SERVER_WORLD_ENTITY& boss) const noexcept;
		void Refresh_ValtanPatternFlowState(SERVER_WORLD_ENTITY& boss);
		void Finish_ValtanPatternFlow(
			SERVER_WORLD_ENTITY& boss,
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE terminalState,
			std::string reason = {});
		void Abort_ValtanPatternFlowForOwner(
			SESSION_ID sessionId,
			std::string reason);
		void Queue_ValtanPatternFlowLifecycle(
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE state,
			const SERVER_WORLD_ENTITY* boss,
			std::string reason = {});
		bool Flush_ValtanPatternFlowLifecycle();

		enum class VALTAN_TIMELINE_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			WAITING_ENVIRONMENT,
			READY,
			WAITING_PATTERN_START,
			WAITING_PATTERN_FINISH,
			COMPLETED_HOLD,
			FAILED_HOLD
		};

		struct VALTAN_TIMELINE_AUDITION_STATE final
		{
			VALTAN_TIMELINE_AUDITION_PHASE ePhase =
				VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::size_t iRowIndex = 0u;
			std::size_t iActionIndex = 0u;
			std::uint32_t iRepeatIndex = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::uint32_t iHeldBossHp = 0u;
			std::uint32_t iHeldBossHealthBar = 0u;
			bool bAllowProductPropBreak = false;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			std::string strExpectedPatternId;
			std::vector<std::string> ExpectedGoneGroupIds;
		};

		/* A page start differs from a one-row timeline audition: it stages the
		already-destroyed arena, releases the real Brain at that page boundary,
		and then leaves the encounter running normally. */
		struct VALTAN_FIGHT_PAGE_START_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iCommandId = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::vector<std::string> ExpectedGoneGroupIds;

			bool Is_Active() const noexcept
			{
				return LostArk::Shared::INVALID_NET_ENTITY_ID != iBossEntityId;
			}
		};

		bool Prepare_ValtanTimelineArenaState(
			const CWorldDestructionRuntime& runtime,
			const SERVER_WORLD_ENTITY& boss,
			VALTAN_TIMELINE_ARENA_STATE arenaState,
			std::uint32_t requestTick,
			WORLD_DESTRUCTION_TRANSACTION& outTransaction,
			std::vector<std::string>& outExpectedGoneGroupIds,
			std::string& status) const;
		bool Stage_ValtanTimelineRowStart(
			SESSION_ID sessionId,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			SERVER_PLAYER& outOwner,
			std::string& status) const;
		bool Start_ValtanTimelineRow(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Stop_ValtanTimelineRow(bool resetEncounter = false);
		bool Prepare_ValtanTimelineRowBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		void Restore_ValtanTimelineRowAfterBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		bool Start_ValtanFightPage(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Prepare_ValtanFightPageBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		struct VALTAN_DECISION_TRACE_REVISION_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::string strBossPlacementId;
			std::uint64_t iTraceSequence = 0u;
			LostArk::Shared::GameplayDataRevision DefinitionRevision{};
		};
		// Validates itemId against the loaded catalog and stacks quantity into
		// player.Inventory, capped at maxStack and MAX_INVENTORY_ITEMS distinct
		// stacks. Returns false (no-op) for an unknown item or a full inventory
		// that would need a new stack. Shared by Handle_DebugGiveItem and the
		// Valtan clear-reward grant in the world entity tick loop.
		bool Grant_Item(
			SERVER_PLAYER& player,
			const std::string& itemId,
			std::uint32_t quantity);
		// Debug-only. Validates the item against the loaded catalog and
		// stacks it into the player's inventory, capped at maxStack.
		void Handle_DebugGiveItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_GIVE_ITEM& request);
		// Validates the item is owned, is a consumable (iHealPercent > 0), and
		// the player is alive; heals iMaximumHp * iHealPercent / 100, then
		// decrements/removes the stack. HP reaches the Client through the next
		// S2C_WORLD_SNAPSHOT tick like any other HP change; no separate result.
		void Handle_UseItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ITEM& request);
		/* Right-click equip / unequip. Checks the slot kind, the class and bag room,
		   then answers with the whole inventory whether or not anything moved. */
		void Handle_SetEquipment(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request);
		bool Apply_SetEquipment(SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request) const;
		/* Repair NPC window: every worn part goes back to 100 percent (free for now); the
		   inventory answer carries the repaired percents. */
		void Handle_RepairEquipment(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REPAIR_EQUIPMENT& request);
		/* After a class change: items bound to another class go back to the bag. */
		bool Unequip_OtherClassItems(SERVER_PLAYER& player) const;
		/* NPC shop basket. The player must stand by that shop NPC; every line must be in
		   its stock, the total price must be covered and the bag must take every line, or
		   nothing changes. Answers with the whole inventory either way. */
		void Handle_BuyItems(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_BUY_ITEMS& request);
		bool Apply_BuyItems(SERVER_PLAYER& player,
			const LostArk::Shared::C2S_BUY_ITEMS& request) const;
		/* One-shot restore of a Client-saved character (inventory, purse, honor title).
		   Bern only, once per fresh entry, and only before any inventory change; the
		   whole request is validated first and a rejected one changes nothing. */
		void Handle_RestoreCharacter(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RESTORE_CHARACTER& request);
		bool Validate_RestoreCharacter(const SERVER_PLAYER& player,
			const LostArk::Shared::C2S_RESTORE_CHARACTER& request) const;
		void Handle_UpgradeEquipment(SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPGRADE_EQUIPMENT& request);
		void Handle_CaptureCharacter(SESSION_ID sessionId,
			const LostArk::Shared::C2S_CAPTURE_CHARACTER& request);
		// Debug Character Select Arena "되돌리기" -- despawns every world entity the
		// debug spawn buttons created in this room (Broadcast_WorldEntityDespawned per
		// entity) and resets the spawn group runtime so the same groups can be
		// re-activated. CHARACTER_SELECT_ARENA only; no-op reply for anything else.
		void Handle_DespawnAllWorldEntities(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DESPAWN_ALL_WORLD_ENTITIES& request);
		/* KoukuSaydon arena form of the Debug revert: removes only the entities
		raised from disabled bootstrap placements (the F1 gate buttons) and
		their dependents, keeping the statically enabled Gate 1 Kouku. */
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses = false);
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses, bool preflightOnly);
		// Bern's Valtan-entry confirm window (right-click a guide NPC). Replaces the
		// old automatic changeLevel triggerBox OBB fire: validates the requesting
		// player is still near the named guide NPC world entity, alive, and idle,
		// then stages the same SERVER_WORLD_TRANSFER_REQUEST the trigger used to
		// build. BERN only; no-op for anything else.
		void Handle_ConfirmNpcEntry(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CONFIRM_NPC_ENTRY& request);
		// Colosseum match queue (BERN only). JOIN is the answer to the Colosseum NPC's offer: the
		// Server re-tests distance/state, collects up to four humans for ten seconds,
		// and reserves acceptance-order parity teams for one atomic
		// admission into a private Colosseum match. Only a committed transfer consumes the queue.
		// LEAVE removes only the requesting session.
		void Handle_ColosseumQueueJoin(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_COLOSSEUM_QUEUE_JOIN& request);
		void Handle_ColosseumQueueLeave(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_COLOSSEUM_QUEUE_LEAVE& request);
		void Send_ColosseumQueueState(
			SESSION_ID sessionId, LostArk::Shared::COLOSSEUM_QUEUE_STATE state);
		void Try_FormColosseumMatch();
		void Broadcast_ColosseumQueueState();
		void Try_StartColosseumEntry();
		void Handle_ColosseumLoadReady(SESSION_ID, const LostArk::Shared::C2S_COLOSSEUM_LOAD_READY&);
		void Handle_ColosseumReturn(SESSION_ID, const LostArk::Shared::C2S_COLOSSEUM_RETURN&);
		void Update_ColosseumMatch(std::uint32_t tick);
		void Broadcast_ColosseumMatchState();
		void Score_ColosseumKills(std::uint32_t tick);
		// The player pressed the key an interact-gated trigger box offered.
		// Names only the box; the trigger system re-tests that this player is
		// still standing in it before anything runs, so a stale or forged
		// request changes nothing.
		void Handle_InteractTrigger(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACT_TRIGGER& request);
		// Raid Clear screen's "돌아가기" button -- the reverse trip. No proximity
		// or party-leader gating (unlike Handle_ConfirmNpcEntry): any player in
		// a cleared Valtan/Kouku raid can return independently to its recorded entry guide.
		// Direct Lobby entries retain the default guide; NPC entries keep their source ID.
		void Handle_ReturnToBern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RETURN_TO_BERN& request);
		// Stages one player's world transfer back to Bern. Shared by the cleared-raid exit
		// button and the gate progress EXIT vote; returns false when nothing was staged.
		bool Stage_ReturnToBern(
			LostArk::Shared::PLAYER_ID playerId,
			std::uint32_t requestSequence);
		/* Same-room-only: request.iTargetNetEntityId must resolve to a real
		   player currently in this room's m_PlayerIdByEntityId. There is no
		   cross-room player identity yet (nickname is display text only, see
		   CLAUDE.md), so an invite naming a player in a different room or a
		   stale/unknown NetEntityId is rejected, not queued. */
		void Handle_PartyInvite(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE& request);
		void Handle_PartyInviteRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE_RESPOND& request);
		void Broadcast_PartyRoster(std::uint32_t partyId);

		/* 파티 레이드 입장 전원 수락 투표. 한 플레이어는 동시에 하나의 열린 proposal에만
		   속한다(propose가 그 불변식을 검사). iProposalId는 이 방에서 발급하는 단조 증가
		   식별자로 pointer/index가 아니다. Voters는 발의 시점 멤버 스냅샷(솔로는 1명),
		   Accepted는 그 부분집합. m_iServerTick이 iDeadlineTick을 넘으면 TIMEOUT으로 닫는다. */
		/* Commander raid gate progress (KoukuSaydon gates). The room marks a gate cleared
		   when its last primary boss dies (Notify_GateBossDeath from the world update),
		   the leader / solo player proposes to move on, members answer, and on
		   ALL_ACCEPTED Advance_Gate despawns the arena, raises the next gate's disabled
		   placements and moves every player to the gate position -- the product path of
		   what the Debug gate buttons do by hand. Implemented in GameRoom_GateProgress.cpp. */
		struct GATE_PROGRESS_STATE
		{
			std::uint8_t iCurrentGate = 0u;     // 1-based, 0 = no gate raised yet
			std::uint8_t iClearedMask = 0u;
			std::uint32_t iProposalId = 0u;     // 0 = no vote open
			std::uint32_t iRaidEpoch = 0u;      // Nonzero pins a vote to its immutable raid run
			LostArk::Shared::GATE_PROGRESS_KIND eKind = LostArk::Shared::GATE_PROGRESS_KIND::ADVANCE;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::PLAYER_ID iProposerId = LostArk::Shared::INVALID_PLAYER_ID;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};
		void Handle_GateProgressPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_PROPOSE& request);
		void Handle_GateProgressRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_RESPOND& request);
		void Close_GateProgressVote(LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result);
		void Expire_GateProgressVote();
		bool Complete_GateProgressTransition(const GATE_PROGRESS_STATE& transition);
		void Update_ArenaAssembly();
		bool Complete_ArenaAssembly();
		bool Collect_KoukuEntryParticipants(std::vector<LostArk::Shared::PLAYER_ID>& participants, std::uint32_t& raidEpoch) const;
		bool Collect_ValtanEntryParticipants(std::vector<LostArk::Shared::PLAYER_ID>& participants) const;
		bool Start_ValtanEntry(const std::vector<LostArk::Shared::PLAYER_ID>& expectedParticipants);
		// A primary boss is about to be removed DEAD: clears its gate when it was the last one.
		void Notify_GateBossDeath(const SERVER_WORLD_ENTITY& deadBoss);
		// A gate placement came up (Debug button or Advance_Gate): that gate is now current.
		void Note_GatePlacementRaised(const std::string& placementId);
		bool Advance_Gate(std::uint8_t nextGate);
		bool Advance_Gate(std::uint8_t nextGate, const std::vector<LostArk::Shared::PLAYER_ID>* participants);
		bool Spawn_GatePlacement(const std::string& placementId);
		bool Spawn_GatePlacement(const std::string& placementId, SERVER_WORLD_ENTITY* prepared);
		bool Build_GateProgressState(LostArk::Shared::S2C_GATE_PROGRESS_STATE& message,
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result) const;
		void Broadcast_GateProgressState(
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result,
			LostArk::Shared::GATE_PROGRESS_KIND completedKind = LostArk::Shared::GATE_PROGRESS_KIND::END);
		std::uint8_t Gate_Count() const;
		std::uint8_t Resolve_CurrentKoukuGate() const;
		bool Resolve_KoukuRevivePosition(const SERVER_PLAYER& player, SERVER_NAV_POINT& position, float& yaw) const;
		int Gate_IndexOfPlacement(const std::string& placementId) const;
		/* Raid-clear award input. Every fought primary boss advances its players' fight
		   clock each tick; a dying gate boss hands its ledger to the room, and the clear
		   sends the room ledger to every player and empties it. */
		void Tick_MvpLedgers();
		void Merge_MvpLedger(SERVER_WORLD_ENTITY& boss);
		void Broadcast_RaidMvpResult(std::uint8_t iGate);

		struct RAID_ENTRY_PROPOSAL
		{
			std::uint32_t iProposalId = 0u;
			std::uint32_t iPartyId = 0u;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::RAID_ENTRY_TARGET eTarget =
				LostArk::Shared::RAID_ENTRY_TARGET::VALTAN;
			std::string strNpcPlacementId;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};

		// 리더/솔로가 입장하기로 발의 -> 대상 전원(솔로는 본인)에게 프롬프트, 투표 개시.
		void Handle_RaidEntryPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_PROPOSE& request);
		// 개별 수락/거절 반영. 거절이면 즉시 DECLINED 종료, 전원 수락이면 ALL_ACCEPTED 종료.
		void Handle_RaidEntryRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_RESPOND& request);
		// 투표를 result로 종료해 전원에 통지하고 proposal을 제거한다. ALL_ACCEPTED면
		// Stage_PartyWorldTransfer로 batch 전송을 stage하고, stage 실패면 CANCELLED로 낮춘다.
		void Close_RaidEntryVote(
			RAID_ENTRY_PROPOSAL& proposal,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// 진행/종료 S2C_RAID_ENTRY_VOTE를 proposal의 present voters에게 보낸다.
		void Broadcast_RaidEntryVote(
			const RAID_ENTRY_PROPOSAL& proposal, bool bClosed,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// m_iServerTick 기준 만료 proposal을 TIMEOUT으로 닫는다(tick 루프에서 호출).
		void Expire_RaidEntryProposals();
		// playerId가 voter인 열린 proposal을 CANCELLED로 닫는다(이탈/파티 해산 시).
		void Cancel_RaidEntryProposalsInvolving(LostArk::Shared::PLAYER_ID playerId);
		// 검증된 batch 멤버(front=리더)를 기존 SERVER_WORLD_TRANSFER_REQUEST 경로로 stage.
		// 멤버 unavailable/이미 staged면 false(호출자가 투표를 CANCELLED로 닫는다).
		bool Stage_PartyWorldTransfer(
			const std::vector<LostArk::Shared::PLAYER_ID>& batchMemberIds,
			LostArk::Shared::WORLD_ID targetWorldId,
			std::uint32_t requestSequence, const std::string& raidReturnNpcPlacementId,
			const std::string& spawnPlacementOverrideId = {});
		// player가 알려진 Valtan 입장 guide NPC 근처(proximity)인지 검증한다.
		bool Is_PlayerNearValtanEntryNpc(
			const SERVER_PLAYER& player, const std::string& npcPlacementId) const;
		/* Tells every session in this room that an authored world sequence
		   instance started. Presentation only: the Server keeps no sequence
		   state, so a session that joins later simply misses a played edge.
		   False rejects the action without consuming its trigger or moving players. */
		bool Broadcast_WorldSequencePlay(
			const std::string& instanceId, float playbackSpeed = 1.f,
			float positionOffsetX = 0.f, float positionOffsetY = 0.f, float positionOffsetZ = 0.f,
			std::uint32_t durationMs = 0u, const std::string& targetSequenceInstanceId = {},
			LostArk::Shared::WORLD_SEQUENCE_OPERATION operation = LostArk::Shared::WORLD_SEQUENCE_OPERATION::PLAY);
		void Handle_DebugKillGateBosses(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request);
		LostArk::Shared::DEBUG_KILL_GATE_BOSSES_RESULT Apply_DebugKillGateBosses(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request, std::uint8_t& killedCount);
		std::unordered_map<SESSION_ID, std::uint32_t> m_KillGateBossesRequestSequences;
		void Handle_SetCooldownMode(SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::SET_COOLDOWN_MODE_RESULT Apply_SetCooldownMode(
			SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::COOLDOWN_MODE m_eCooldownMode = LostArk::Shared::COOLDOWN_MODE::DEBUG_THREE_SECONDS;
		std::unordered_map<SESSION_ID, std::uint32_t> m_CooldownModeRequestSequences;
		void Handle_DebugWorldPlayback(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		std::unordered_map<SESSION_ID, std::uint32_t> m_WorldPlaybackRequestSequences;
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Apply_DebugRoomPlayerArrival(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		LostArk::Shared::DEBUG_TELEPORT_RESULT Validate_DebugTeleportDestination(
			const SERVER_PLAYER& player, const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request,
			SERVER_NAV_POINT& ground, LostArk::Shared::NET_ENTITY_ID ignoredBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		struct ROOM_PLAYER_ARRIVAL_RUN final
		{
			std::uint32_t iEpoch = 0u;
			std::string strRootPatternId;
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, SESSION_ID>> Players;
			std::map<std::string, LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT> Occurrences;
		};
		std::unordered_map<SESSION_ID, ROOM_PLAYER_ARRIVAL_RUN> m_RoomPlayerArrivalRuns;
		/* Offers or withdraws one interact-gated box for the one player it
		   concerns. Unlike the sequence broadcast this is never room-wide. */
		void Send_InteractPrompt(const SERVER_INTERACT_PROMPT_EDGE& edge);
		/* The spawn-group activation every trigger path shares: starts a dormant
		   group, restarts a finished one once its monsters are gone, and never
		   stacks a wave on a group that is still running. */
		bool Activate_SpawnGroupFromTrigger(const std::string& spawnGroupId);
		/* What one trigger action does in this room, whether the box fired because
		   the player stepped in (a scripted flow) or pressed G inside it. */
		bool Activate_TriggerTarget(
			WORLD_TRIGGER_ACTION_KIND kind, const std::string& targetId);
		/* Leave() calls this so a disconnecting player does not linger as a
		   ghost roster entry for whoever they partied with. */
		void Remove_FromParty(LostArk::Shared::PLAYER_ID playerId);
		/* Same room-scoped broadcast Broadcast_PartyRoster already uses --
		   every current session in this room receives the relayed line,
		   including the sender (its own head bubble is driven off the same
		   S2C_CHAT the rest of the room gets, not a second local-only path). */
		void Handle_RoomPing(SESSION_ID sessionId, const LostArk::Shared::C2S_ROOM_PING& request);
		void Handle_Chat(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHAT& request);

		bool Send_Accepted(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		bool Send_EnterRejected(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason);
		bool Send_Spawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		static bool Build_WorldEntitySpawnedPayload(
			const SERVER_WORLD_ENTITY& entity,
			std::vector<std::uint8_t>& outPayload);
		bool Send_WorldEntitySpawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_WORLD_ENTITY& entity);
		bool Send_WorldEntityDespawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Send_CombatObjectSpawned(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned);
		bool Send_WorldEntitySpawnResult(
			const std::shared_ptr<CClientSession>& session,
			const std::string& placementId,
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		bool Send_ValtanAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			LostArk::Shared::VALTAN_AUDITION_RESULT result,
			std::uint32_t currentHealthBar);
		bool Send_KoukuSaydonPatternAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message);
		bool Send_ValtanPatternFlowResult(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t commandSequence,
			LostArk::Shared::VALTAN_PATTERN_FLOW_COMMAND command,
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT result,
			const std::string& flowId,
			const std::string& flowRevision,
			std::uint32_t roomFlowEpoch,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			const std::string& reason);
		bool Build_RequiredPinnedGameplayRevisions(
			std::vector<LostArk::Shared::GameplayDataRevision>&
				outRevisions) const;
		[[nodiscard]] const CGameplayCatalog* Resolve_ValtanGameplayCatalog(
			const SERVER_WORLD_ENTITY& boss) const noexcept;
		bool Send_CharacterClassChangeResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
			LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
			LostArk::Shared::CHARACTER_CLASS_ID activeClass);
		// Single-session send: inventory is per-player state, not room-shared
		// like S2C_WORLD_SNAPSHOT, so it never broadcasts.
		bool Send_InventorySnapshot(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t requestSequence,
			const SERVER_PLAYER& player);
		bool Send_Despawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		bool Send_WorldDestructionFullSync(
			const std::shared_ptr<CClientSession>& session);
		// Server-owned collision/navigation counters carried by every
		// destruction message so the Debug audition panel never has to infer
		// passage from the replicated wall states.
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
			Build_WorldDestructionDiagnostics() const;
		void Broadcast_Spawned(
			const SERVER_PLAYER& player,
			SESSION_ID exceptSessionId);
		void Broadcast_Despawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		void Broadcast_WorldEntitySpawned(
			const SERVER_WORLD_ENTITY& entity);
		void Broadcast_WorldEntityDespawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Broadcast_WorldDestructionDelta(
			const std::vector<WORLD_DESTRUCTION_STATE_TRANSITION>& transitions,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick);
		void Broadcast_WorldSnapshot();

		std::shared_ptr<CClientSession> Find_Session(
			SESSION_ID sessionId) const;
		void Rollback_Join(SESSION_ID sessionId);
		[[nodiscard]] bool Is_PlayerAdmissionFull() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_AvailablePlayerSpawn() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_Placement(
			const std::string& placementId) const;
		bool Build_WorldEntity(
			const WORLD_BOOTSTRAP_PLACEMENT& placement,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			SERVER_WORLD_ENTITY& outEntity,
			const CGameplayCatalog* definitionCatalog = nullptr,
			LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID, std::uint32_t ownerPatternSequence = 0u);
		bool Initialize_WorldEntities();
		bool Reset_ReplayableArenaWhenEmpty();
		bool Reset_ValtanArenaWhenEmpty();
		bool Apply_BossPatternStageActions(
			SERVER_WORLD_ENTITY& boss,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		bool Apply_BossPatternScheduledSpawnWave(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_BossPatternStageTransition(
			SERVER_WORLD_ENTITY& boss,
			const std::string& previousPatternId,
			const std::string& previousActionId,
			const std::string& nextPatternId,
			const std::string& nextActionId,
			const LostArk::Shared::GameplayDataRevision&
				previousDefinitionRevision,
			const LostArk::Shared::GameplayDataRevision& nextDefinitionRevision,
			std::uint32_t serverTick);
		bool Stage_BossPatternStageActions(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			SERVER_BOSS_COMBAT_STATE& stagedCombat,
			std::uint8_t& stagedGameplayPhase,
			SERVER_COMBAT_OBJECT_TRANSACTION& combatObjectTransaction,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		/* Runs only after every stage-action preflight transaction commits. These
		actions own player/target state and therefore cannot be staged inside the
		boss-combat or combat-object value transactions above. */
		bool Prepare_GrabbedPlayerImpact(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& stagedPlayers,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& stagedDamageEvents);
		SERVER_PLAYER* Select_BossRandomAliveTarget(const SERVER_WORLD_ENTITY& boss,
			const std::string& actionId, const std::string& targetId, std::uint32_t serverTick);
		bool Commit_BossPatternPlayerStageActions(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u);
		bool Resolve_ArenaRandomVolleyOrigins(
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			const BOSS_COMBAT_OBJECT_DEFINITION& definition,
			std::uint32_t spawnWaveOrdinal,
			std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET>& outOrigins,
			float explicitMinimumSpacingM = 0.f, const SERVER_NAV_POINT* anchorOverride = nullptr);
		bool Broadcast_CombatObjectLifecycle();
		void Drain_BossCombatEvents();
		bool Apply_WorldDestructionStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		/* Commit the 69 ordinary contact walls and the 30 outer ring walls in one
		transaction, leaving every floor sector INTACT. A floor-collapse bar only
		arrives after the fight has already taken those walls down, so the
		audition for such a bar has to clear them inside the same atomic request
		instead of a second one the boss could start a pattern between. */
		bool Break_EveryWallForAudition(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		/* The navigation grid is the ground a boss pattern stride may cross.
		The collision sweep owns wall contact, while the furthest sample the grid
		still owns is what any stride is allowed to reach, so a charge cannot
		leave the floor before its wall contact is evaluated. A start the grid
		already refuses passes through
		unchanged, because refusing it there would strand the boss for good. */
		static void Resolve_NavigableStep(
			const CServerNavigation& navigation,
			float fromX,
			float fromZ,
			float targetX,
			float targetZ,
			float& outX,
			float& outZ);
		/* Raise the pillar slots on the authored stage edge of the pattern that
		owns them. The shatter has no identified product owner yet. */
		bool Apply_EncounterPropStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Commit_DueEncounterProps(std::uint32_t serverTick);
		bool Initialize_WorldPickups();
		void Reset_WorldPickups(std::uint32_t serverTick);
		void Update_WorldPickups(std::uint32_t serverTick, bool allowCollection = true);
		void Apply_WorldPickupDestruction(const WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::uint32_t serverTick);
		void Remove_RemainingWorldPickups(std::uint32_t serverTick);
		bool Send_EncounterPropSync(
			const std::shared_ptr<CClientSession>& session);
		void Broadcast_EncounterPropSync();
		/* Break whatever a non-impact boss body physically reached between its
		previous and current position. A charge-impact stage bypasses this generic
		pass and owns one exact swept wall transaction: impact receiver first,
		then the co-located ordinary contact binding. */
		bool Apply_WorldDestructionBodyContact(
			SERVER_WORLD_ENTITY& boss,
			float previousX,
			float previousY,
			float previousZ,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionPatternHitContact(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionContacts(
			SERVER_WORLD_ENTITY& boss,
			const std::vector<std::string>& contactPlacementIds,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionImpact(
			SERVER_WORLD_ENTITY& boss,
			const std::string& receiverPlacementId,
			std::uint32_t serverTick,
			bool& outTriggered);
		bool Commit_WorldDestructionTransaction(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick,
			std::string& status);
		void Invalidate_DynamicNavigationPaths();
		bool Build_WorldDestructionLiveEvents(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const SERVER_WORLD_ENTITY& boss,
			std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::string& status) const;
		bool Commit_DueWorldDestruction(std::uint32_t serverTick);
		bool Activate_Encounter(const std::string& placementId);
		bool Spawn_Monster(
			const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			std::uint32_t ordinal);
		/* Card maze. The telescope claim deals the suits and raises the
		targets; the MAZE hammer press judges its swing once, at the runtime's
		hit tick, against those targets. */
		bool Spawn_KoukuCardRainSoldiers(LostArk::Shared::NET_ENTITY_ID ownerId, std::uint32_t tick,
			const BOSS_PATTERN_MECHANIC_TRIGGER* tuning = nullptr);
		void Update_KoukuCardRainSoldiers(std::uint32_t tick);
		bool Begin_CardMaze(LostArk::Shared::PLAYER_ID claimantId);
		void Reset_CardMaze();
		void Despawn_CardMazeTargets();
		void Resolve_CardMazeHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Resolve_MarioHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Update_MarioBombContacts(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Update_MarioBouncingBallContacts(SERVER_PLAYER& player, std::uint32_t updateTick);
		// Rotating cannon jets and the big mokoko waterfall of a live Waterpang match.
		void Update_MaharakaWaterpangHazards(SERVER_PLAYER& player, std::uint32_t updateTick);
        void Handle_MaharakaAITuning(SESSION_ID sessionId, const LostArk::Shared::C2S_MAHARAKA_AI_TUNING& request);
        bool Spawn_MaharakaWaterpangAI();
        void Update_MaharakaWaterpangMatch(std::uint32_t updateTick);
        void Clear_MaharakaWaterpangAI(std::uint32_t keepCount = 0u);
        bool Finish_MaharakaWaterpangMatch();
		// The running Debug forced event first, else the match schedule; false when neither runs.
		bool Sample_MaharakaWaterpangNow(std::uint32_t tick, LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& out) const;
		// The Server's water gun arming rule: replicated as PLAYER_SNAPSHOT.isWaterpangArmed.
		bool Is_MaharakaWaterpangArmed(const SERVER_PLAYER& player) const;
		// Water gun Q/W/E/R cast of an armed body: cooldown, no movement lock, shot scheduled.
		bool Try_StartMaharakaWaterGunSkill(SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& command);
		// Flies the scheduled shots and pushes the bodies they strike.
		void Update_MaharakaWaterGunShots(std::uint32_t updateTick);
		// The forced event plus a short tail, so its last push can still leave the deck.
		bool Is_MaharakaWaterpangDebugEventLive(std::uint32_t tick) const;
		// Debug F1 Waterpang pattern button: starts a forced event for the whole room.
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Start_MaharakaWaterpangDebugEvent(const std::string& instanceId);
		std::uint8_t Mario_CurseReleasedMask(std::uint8_t stage, std::uint8_t layout) const;
		bool Spawn_CardMazeTarget(const CKoukuCardMazeRuntime::SPAWN_REQUEST& request);
		void Remove_CardMazeTarget(LostArk::Shared::NET_ENTITY_ID id);
		void Update_CardMaze(std::uint32_t tick);
		/* Before the run: raises the clown box for players inside the maze and
		latches its destruction. Clear forgets it and removes a living box. */
		void Update_CardMazeClownBox(std::uint32_t tick);
		void Clear_CardMazeClownBox();
		/* Advances the bingo bomb clock: a mark whose deadline passed is
		planted where its carrier stands, and a carrier that left the room
		takes its mark with it. */
		void Update_KoukuBingo(std::uint32_t tick);
		bool Begin_CardMazeTransfer(SERVER_PLAYER& player, float x, float y, float z,
			std::uint32_t tick, bool leaving);
		std::uint32_t Count_SpawnGroupEntities(
			const std::string& spawnGroupId) const;
		/* 1 unless the player is standing in the stance its identity gauge pays
		for, which is the only thing that changes how fast anyone walks. */
		float Resolve_StanceMoveSpeedScale(const SERVER_PLAYER& player) const;
		/* Hands the living monster and boss bodies to the collision system so this
		tick's player walks and root motion stop at them. */
		void Refresh_PlayerBlockingBodies();
		bool Try_KoukuWalkOffFloor(SERVER_PLAYER& player, float x, float z,
			float fixedDeltaSeconds, std::uint32_t updateTick);
		const WORLD_BOOTSTRAP_PLACEMENT* Resolve_KoukuFallCenter(const SERVER_PLAYER& player) const;
		bool Update_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		void Begin_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		/* Product boss-pattern adapters call these with replicated identities. The
		room owns interruption, fixed-tick fallback motion and release reaction so
		no pattern can leave half of a grabbed player state behind. */
		bool Capture_PlayerAttachment(
			LostArk::Shared::NET_ENTITY_ID playerEntityId,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
			std::uint32_t serverTick, std::uint32_t holdEndTick = 0u, std::uint32_t sourcePatternSequence = 0u);
		bool Update_PlayerAttachment(
			SERVER_PLAYER& player,
			std::uint32_t serverTick);
		bool Release_PlayerAttachment(
			SERVER_PLAYER& player,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		std::size_t Release_PlayerAttachments(
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		[[nodiscard]] bool Restore_PatternBoundPlayer(SERVER_PLAYER& player);
		void Update_Players(float fixedDeltaSeconds);
		bool Prepare_ArenaEjection(
			SERVER_PLAYER& staged,
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick);
		bool Resolve_ArenaCenter(
			const SERVER_WORLD_ENTITY& boss,
			SERVER_NAV_POINT& point);
		bool Activate_ValtanGhostPhaseLoop(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog);
		bool Begin_ValtanGhostRelocation(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_ValtanGhostPortalScheduler(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_DependentBosses(std::uint32_t serverTick);
		/* Slides a hit player along the armed knockback window, clamped to
		walkable floor and blocking bodies; a wall ends the window early. */
		void Advance_PlayerKnockback(
			SERVER_PLAYER& player, float fixedDeltaSeconds);
		void Update_WorldEntities(float fixedDeltaSeconds);

	private:
		// Best-effort traffic leaves room for gameplay/control commands. LEAVE
		// never shares this bounded queue: disconnect cleanup has its own
		// session-deduplicated priority queue below.
		static constexpr std::size_t MAX_BEST_EFFORT_COMMAND_COUNT = 768u;
		static constexpr std::size_t MAX_RELIABLE_COMMAND_COUNT = 960u;
		static constexpr std::size_t MAX_COMMANDS_DRAINED_PER_TICK = 256u;

		std::shared_ptr<LostArk::Shared::Concurrency::WorkStealingJobSystem> m_SnapshotJobs;
		mutable std::mutex m_CommandMutex;
		std::deque<ROOM_COMMAND> m_InboundCommands;
		std::deque<ROOM_COMMAND> m_CleanupCommands;
		std::unordered_set<SESSION_ID> m_QueuedCleanupSessionIds;
		SERVER_ROOM_PERFORMANCE_METRICS m_PerformanceMetrics;
		SERVER_ROOM_PERFORMANCE_METRICS m_LastRoomPerfLogSample;
		std::string m_strPendingPerformanceDiagnostic;
		std::uint64_t m_iLastRoomPerfSnapshotDroppedCount = 0;
		std::uint64_t m_iLastRoomPerfReliableRejectedCount = 0;
		std::uint64_t m_iLastRoomPerfWireSendFailureCount = 0;
		std::size_t m_iLastRoomPerfOutboundHighWatermark = 0u;
		bool m_acceptsCommands = true;
		std::deque<SERVER_WORLD_TRANSFER_REQUEST> m_PendingWorldTransfers;
		/* Bern remembers the ship a session sailed to Maharaka on, so the return trip can put that
		   session back on the same ship at the pier it left from. Bern room only; keyed by session. */
		struct SHIP_RETURN_STATE final
		{
			LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
			float fDockX = 0.f;
			float fDockY = 0.f;
			float fDockZ = 0.f;
			float fDockYawDegrees = 0.f;
		};
		std::unordered_map<SESSION_ID, SHIP_RETURN_STATE> m_MaharakaShipReturnBySession;
		void Remember_ShipForWorldTransfer(const SERVER_WORLD_TRANSFER_REQUEST& transfer);
		struct PENDING_ESTHER_SUMMON final
		{
			const ESTHER_ROSTER_ENTRY* pRosterEntry = nullptr;
			LostArk::Shared::PLAYER_ID iCasterPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
			float fPositionX = 0.f;
			float fPositionY = 0.f;
			float fPositionZ = 0.f;
			float fYawDegrees = 0.f;
			float fRemainingSeconds = 0.f;
		};
		std::vector<PENDING_ESTHER_SUMMON> m_PendingEstherSummons;
		struct ESTHER_ZONE_RUNTIME final
		{
			const LostArk::Shared::EstherStrike::ZONE* pZone = nullptr;
			float fPositionX = 0.f;
			float fPositionZ = 0.f;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iNextPulseTick = 0u;
		};
		std::vector<ESTHER_ZONE_RUNTIME> m_EstherZones;

		std::unordered_map<SESSION_ID, std::weak_ptr<CClientSession>> m_Sessions;
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> m_Players;
		struct GUIDE_PROMPT_OCCURRENCE
		{
			std::string PromptId, TriggerId;
			std::size_t NextSegment = 0;
			int Priority = 0;
			bool IsSpaceEnter = false;
			std::vector<std::string> SpaceTriggerIds;
			bool operator==(const GUIDE_PROMPT_OCCURRENCE&) const = default;
		};
		struct GUIDE_RUNTIME
		{
			LostArk::Shared::PLAYER_ID PlayerId = 0, AnchorId = 0;
			std::weak_ptr<CClientSession> OwnerSession;
			LostArk::Shared::NET_ENTITY_ID OwnerNetEntityId = 0;
			bool WaitingForOwner = false, WaitingForShip = false, ReturningOnFoot = false;
			std::string ComboId, PendingComboId, Reason;
			std::size_t ComboStep = 0;
			float ThinkElapsed = 0.f, ComboElapsed = 0.f, StepElapsed = 0.f, FarElapsed = 0.f, HoldElapsed = 0.f, PromptRemaining = 0.f;
			std::uint32_t Sequence = 0;
            std::map<std::string, std::uint32_t> CommandTicks;
			std::uint8_t Action = 0;
            float FollowScore = 0.f, EvadeScore = 0.f, CombatScore = 0.f;
			std::deque<GUIDE_PROMPT_OCCURRENCE> PromptQueue;
			std::map<std::string, std::uint32_t> TriggerTicks;
			std::unordered_set<std::string> InsideBoxes;
			std::map<LostArk::Shared::NET_ENTITY_ID, std::uint32_t> PatternSequences;
		};
		CGuideCatalog m_GuideCatalog;
		// At most one owner binds the one placed Bern guide; no companion clone or party slot.
		std::map<SESSION_ID, GUIDE_RUNTIME> m_PersonalGuides;
		std::uint32_t m_iGuideEventSequence = 0;
		float m_fGuideOwnershipElapsed = 0.f;
		std::unordered_map<SESSION_ID, std::uint32_t> m_GuideControlSequences;
		LostArk::Shared::PLAYER_ID m_iGuideReceptionId = 0;
        LostArk::Shared::PLAYER_ID m_iNextGuidePlayerId = 0x80000000u;
		/* Grants what a started skill buffs, to the caster, the party in this room
		or the entities it targets. */
		void Apply_SkillBuffs(SERVER_PLAYER& caster, std::uint32_t skillId,
			std::uint32_t serverTick);
		std::unordered_map<SESSION_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdBySessionId;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdByEntityId;

		/* Same-room party state -- PLAYER_ID is room-local (freshly allocated
		   per room on Join), so this map does not by itself survive a member
		   moving to a different room. 0 means "no party" -- never a real party
		   ID. Invite/accept/join only; leave/kick/leader promotion is a
		   separate follow-up.
		   A party-leader-triggered group Valtan entry (Handle_ConfirmNpcEntry
		   -> Transfer_PartyTo) is the one case
		   that does survive a room change: every member transfers together in
		   one batch and gets re-grouped into a fresh room-local party in the
		   target room, so the party itself is never actually split across two
		   rooms at once. There is still no general cross-room party identity
		   (e.g. inviting or chatting with someone in a different room). */
		std::uint32_t m_iNextPartyId = 1u;
		std::unordered_map<LostArk::Shared::PLAYER_ID, std::uint32_t>
			m_PartyIdByPlayerId;
		std::unordered_map<std::uint32_t, std::vector<LostArk::Shared::PLAYER_ID>>
			m_PartyMembersByPartyId;
		// One pending invite per target at a time; a new invite silently
		// replaces whatever that target's last unanswered invite was.
		std::unordered_map<LostArk::Shared::PLAYER_ID, LostArk::Shared::PLAYER_ID>
			m_PendingPartyInviteByTargetPlayerId;
		// At most one latest failure per present player. A full reliable queue
		// delays the notice instead of disconnecting a rejected source party.
		std::unordered_map<SESSION_ID, LostArk::Shared::S2C_PARTY_TRANSFER_RESULT>
			m_PendingPartyTransferResults;

		// 파티 레이드 입장 투표 상태. struct RAID_ENTRY_PROPOSAL은 위 메서드 선언부에 정의한다.
		std::vector<RAID_ENTRY_PROPOSAL> m_RaidEntryProposals;
		std::uint32_t m_iNextRaidEntryProposalId = 1u;
		// Colosseum match queue: accepted sessions in join order (BERN room only).
		struct COLOSSEUM_QUEUE_ENTRY
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			std::uint32_t iRequestSequence = 0u;
		};
		std::vector<COLOSSEUM_QUEUE_ENTRY> m_ColosseumQueue;
		bool m_bColosseumTransferPending = false;
		std::uint32_t m_iColosseumRetryTick = 0u;
		std::uint64_t m_iColosseumMatchId = 0u;
		LostArk::Shared::COLOSSEUM_MATCH_PHASE m_eColosseumPhase = LostArk::Shared::COLOSSEUM_MATCH_PHASE::RECRUITING;
		std::uint8_t m_iColosseumWinnerTeam = 255u;
		std::uint32_t m_iColosseumRevision = 1u;
		std::array<std::uint32_t, 2> m_ColosseumTeamPartyIds{};
		enum class COLOSSEUM_TACTIC : std::uint8_t { WAIT, ENGAGE, REPOSITION, RETREAT, EVADE, RECOVER };
		struct COLOSSEUM_MERCENARY_RUNTIME final
		{
			float fThinkElapsed = 0.f;
			float fSenseElapsed = 0.f, fDecisionInterval = .2f, fThreatAgeSeconds = 0.f;
			std::uint32_t iSequence = 0u;
			// Match-owned admission clock survives player respawn and tactical resets.
			std::optional<std::uint32_t> iLastAltVAdmissionTick;
			std::uint64_t iRandomState = 0u;
			std::vector<LostArk::Shared::SKILL_ID> AvailableSkills;
			std::array<LostArk::Shared::SKILL_ID, 3> RecentSkills{};
			std::size_t iRecentSkillCursor = 0u;
			LostArk::Shared::SKILL_ID iLastSkillId = LostArk::Shared::INVALID_SKILL_ID;
			LostArk::Shared::NET_ENTITY_ID iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iTargetSelectedTick = 0u, iTacticUntilTick = 0u;
			std::uint32_t iNextAttackTick = 0u, iNextEvadeTick = 0u, iRetreatAllowedTick = 0u;
			std::uint32_t iLastDamageTick = 0u, iObservedHp = 0u;
			COLOSSEUM_TACTIC eTactic = COLOSSEUM_TACTIC::ENGAGE;
			CColosseumThreatAssessment Threats;
			const char* pReason = "Waiting for recruitment";
		};
		std::map<LostArk::Shared::PLAYER_ID, COLOSSEUM_MERCENARY_RUNTIME> m_ColosseumMercenaries;
		std::unordered_map<SESSION_ID, std::uint32_t> m_ColosseumRecruitSequences;
		std::vector<SESSION_ID> m_ColosseumSessions;
		std::uint32_t m_iColosseumPhaseStart = 0u, m_iColosseumPhaseEnd = 0u;
		std::uint32_t m_iColosseumScores[2]{};
		std::uint32_t m_iColosseumKillSequence = 0u;
		std::vector<LostArk::Shared::COLOSSEUM_KILL_EVENT> m_ColosseumRecentKills;
		std::uint32_t m_iColosseumQueueDeadline = 0u, m_iColosseumQueueBroadcastTick = 0u;
		std::array<std::uint8_t, 2> m_ColosseumInitialHumans{};
		GATE_PROGRESS_STATE m_GateProgress;
		std::uint32_t m_iArenaAssemblyStartTick = 0u;
		std::uint32_t m_iArenaAssemblyRaidEpoch = 0u;
		bool m_bArenaAssemblyAttempted = false;
		std::vector<LostArk::Shared::PLAYER_ID> m_ArenaAssemblyParticipants;
		std::vector<SERVER_MVP_LEDGER_ROW> m_GateMvpLedger;
		std::uint32_t m_iNextGateProposalId = 1u;

		LostArk::Shared::WORLD_ID m_eWorldId = LostArk::Shared::WORLD_ID::END;
		CWorldBootstrap m_WorldBootstrap;
		CGameplayCatalogGenerations m_GameplayCatalog;
		std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY> m_StagedNumericEntries;
		std::vector<std::pair<std::shared_ptr<const CGameplayCatalog>, std::shared_ptr<const CGameplayCatalog>>>
			m_StagedNumericCatalogRemaps;
		CItemCatalog m_ItemCatalog;
		CVehicleCatalog m_VehicleCatalog;
		CHonorTitleCatalog m_HonorTitleCatalog;
		CValtanClearRewards m_ValtanClearRewards;
		CServerNavigation m_ServerNavigation;
		CServerCollisionSystem m_ServerCollisionSystem;
		CServerTriggerSystem m_ServerTriggerSystem;
        struct MAHARAKA_WATERPANG_AI final
        {
            std::uint32_t iSlot = 0u, iSequence = 0u, iNextThinkTick = 0u, iNextMoveTick = 0u, iNextShotTick = 0u, iSkillSlot = 0u;
        };
        std::map<LostArk::Shared::PLAYER_ID, MAHARAKA_WATERPANG_AI> m_MaharakaWaterpangAI;
        LostArk::Shared::PLAYER_ID m_iNextWaterpangAIPlayerId = 0x90000000u;
        std::uint32_t m_iWaterpangAIRetryTick = 0u;
        LostArk::Shared::MAHARAKA_AI_TUNING m_MaharakaAITuning;
        bool m_bMaharakaAITuningLoaded = false;
        std::string m_strMaharakaAISourceBytes;
		// One room-wide scheduled intro, retained for late join until the room empties.
		std::optional<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_MaharakaWaterpangIntro;
		// A Waterpang water gun shot from cast to burst; bodies already struck are remembered.
		struct MAHARAKA_WATERGUN_SHOT final
		{
			LostArk::Shared::PLAYER_ID iOwnerId = 0;
			std::uint32_t iSkillId = 0u;
			std::uint32_t iSpawnTick = 0u;
            std::uint32_t iProjectileIndex = 0u;
            LostArk::Shared::COMBAT_OBJECT_ID iVisualObjectId = LostArk::Shared::INVALID_COMBAT_OBJECT_ID;
            LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			float fAimDistanceM = 0.f;
			bool bLaunched = false;
			bool bSpent = false;
			float fX = 0.f, fY = 0.f, fZ = 0.f;
			float fDirX = 0.f, fDirZ = 1.f;
			float fTravelM = 0.f;
			float fReachM = 0.f;
			std::vector<LostArk::Shared::PLAYER_ID> Struck;
		};
		std::vector<MAHARAKA_WATERGUN_SHOT> m_MaharakaWaterGunShots;
		// Debug forced waterfall/cannon broadcast; replaced by the next press, kept for late join.
		std::optional<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_MaharakaWaterpangDebugEvent;
		CSpawnGroupBootstrap m_SpawnGroupBootstrap;
		CSpawnGroupRuntime m_SpawnGroupRuntime;
		std::mt19937 m_MarioLayoutRandom{std::random_device{}()};
		std::mt19937 m_EquipmentUpgradeRandom{std::random_device{}()};
		// Popped source-ball slots per Mario stage (index 1..4), bit = bootstrap slot.
		std::uint16_t m_MarioPoppedBalls[5] = {};
		std::uint8_t m_iNextMarioEntryStage = 1u;
		struct KOUKU_CARD_RAIN_SOLDIER_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID ownerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t patternSequence = 0u, expiresAt = 0u;
		};
		std::map<LostArk::Shared::NET_ENTITY_ID, KOUKU_CARD_RAIN_SOLDIER_STATE> m_KoukuCardRainSoldiers;
		CKoukuCardMazeRuntime m_KoukuCardMaze;
		CKoukuBingoRuntime m_KoukuBingo;
        struct KOUKU_BINGO_DURATION final
        {
            LostArk::Shared::NET_ENTITY_ID iOwnerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
            std::uint32_t iPatternSequence = 0u, iEndTick = 0u, iActivationTick = 0u;
            BOSS_BINGO_BOARD_SETTINGS Settings;
            bool bActivated = false, bInitializeCells = false;
            std::uint32_t iNextBombTick = 0u, iNextHammerTick = 0u, iNextMadnessTick = 0u;
            std::uint32_t iMarkedBombCount = 0u;
            float fHammerHalfForwardM = 0.f, fHammerHalfWidthM = 0.f;
            bool bEncounterOwned = false, bSpecialPatternPending = false;
            bool bLastLineCompletionSucceeded = false;
            bool bLineRewardSinceLastJudgement = false;
            std::uint32_t iLastLineJudgementTick = 0u;
            struct HAMMER { std::int32_t anchor = -1; std::uint32_t startTick = 0u; };
            std::array<HAMMER, 2u> Hammers{};
        } m_KoukuBingoDuration;
        std::uint32_t m_iKoukuBingoBoardEpoch = 0u;
        void Begin_KoukuBingoDuration(const SERVER_WORLD_ENTITY& owner,
            const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t tick);
        void Stop_KoukuBingoDuration(bool clearBoard);
        void Activate_KoukuBingoBoard(std::uint32_t tick);

		std::uint32_t m_iCardMazeMarchStartTick = 0u;
		std::uint32_t m_iCardMazeCycleMs = 0u;
		std::map<LostArk::Shared::PLAYER_ID, std::pair<float, float>> m_CardMazePreviousPositions;
		std::map<LostArk::Shared::PLAYER_ID, std::uint32_t> m_CardMazeContactTicks;
		LostArk::Shared::NET_ENTITY_ID m_iCardMazeClownBoxId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t m_iCardMazeClownBoxDueTick = 0u;
		bool m_bCardMazeClownBoxDestroyed = false;
		CPlayerSkillSystem m_PlayerSkillSystem;
		CCombatObjectRuntime m_CombatObjectRuntime;
		CMonsterBrain m_MonsterBrain;
		CNpcBehaviorRuntime m_NpcBehaviorRuntime;
		CValtanBrain m_ValtanBrain;
		CKoukuSaydonBrain m_KoukuSaydonBrain;
		std::unique_ptr<CValtanBrain> m_DependentValtanBrain =
			std::make_unique<CValtanBrain>();
		VALTAN_DECISION_TRACE_REVISION_STATE m_ValtanDecisionTraceRevision;
		CEstherSkillSystem m_EstherSkillSystem;
		CWorldDestructionBootstrap m_WorldDestructionBootstrap;
		CWorldDestructionRuntime m_WorldDestructionRuntime;
		struct WORLD_PICKUP_RUNTIME final
		{
			WORLD_PICKUP_DESCRIPTOR Descriptor;
			LostArk::Shared::WORLD_PICKUP_SNAPSHOT Snapshot;
		};
		std::vector<WORLD_PICKUP_RUNTIME> m_WorldPickups;
		std::uint32_t m_iWorldPickupEncounterEpoch = 0u;
		/* The four pillars come back four times in one fight, so they live in a
		reversible prop runtime instead of a one-way destruction group. */
		CEncounterPropRuntime m_EncounterPropRuntime;
		/* Room-authoritative completion latch. The primary Product Valtan death
		   raises it before that entity is reliably despawned; the last-player reset
		   clears it for the next party. */
		bool m_bValtanRaidCleared = false;
		/* Debug audition only: the tick a whole pillar cycle shatters on, and
		the flag the next raise turns into that tick. No product trigger for the
		shatter is identified yet, so nothing else writes these. */
		std::uint32_t m_iPillarAuditionBreakTick = 0u;
		bool m_bPillarAuditionCycleArmed = false;
		std::vector<SERVER_WORLD_ENTITY> m_WorldEntities;
		/* One tick's resolved hits. Cleared at the top of every simulation phase
		and consumed by Broadcast_WorldSnapshot, so an event can only ever ride
		the snapshot of the tick that produced it. */
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_TickDamageEvents;
		/* Damage-text events raised while draining room commands, which happens before
		m_TickDamageEvents is cleared for the tick. Moved in right after that clear so a
		potion heal reaches the same broadcast as a combat hit. */
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_PendingCommandDamageEvents;
		std::vector<LostArk::Shared::BOSS_COMBAT_EVENT>
			m_TickBossCombatEvents;
		std::string m_strStatus;
		SERVER_ROOM_RUNTIME_FAILURE m_RuntimeFailure;
		bool m_isReady = false;

		LostArk::Shared::PLAYER_ID m_iNextPlayerId = 1;
		LostArk::Shared::NET_ENTITY_ID m_iNextNetEntityId = 100;
		std::uint32_t m_iServerTick = 0;
		std::uint64_t m_iNextWorldDestructionEventSequence = 1u;
		std::uint64_t m_iNextBossCombatEventSequence = 1u;
		/* Debug Valtan audition. The armed bar is the one an ARM parked the boss
		above; a CROSS is only honoured for that same bar, so a crossing can
		never span an unknown number of authored thresholds. Both reset with the
		encounter, and the handled sequences reject a resent request instead of
		replaying it. Stable-ID pattern requests have an independent ledger because
		the Effect Tool and the Valtan level own independent sequence counters. */
		std::uint32_t m_iValtanAuditionArmedHealthBar = 0;
		std::unordered_map<SESSION_ID, std::uint32_t>
			m_ValtanAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_ID_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
			/* A QUEUED receipt must remain reconcilable after its occurrence is no
			   longer the room's current audition. Keep the last authoritative edge
			   so an exact retry cannot loop on a verdict without lifecycle. */
			std::optional<LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};
		/* Stable-ID Play/Restart keeps the exact payload and verdict. A retry of
		   one identity replays that verdict; an altered tuple never inherits it. */
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_ID_COMMAND_RECEIPT>
			m_ValtanPatternIdAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_FLOW_COMMAND_RECEIPT final
		{
			std::uint32_t iSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strRequestIdentity;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT eResult =
				LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW;
			std::string strReason;
			/* Exact Start retries replay the latest authoritative edge for that
			   admitted program. This settles an unconfirmed Client even when the
			   Flow already reached COMPLETED_HOLD; it never starts a second run. */
			std::optional<LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE>
				LastLifecycle;
		};
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowStartSequenceBySessionId;
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowControlSequenceBySessionId;
		struct TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextKoukuSaydonPatternAuditionEpoch = 1u;
		struct KOUKU_DRAFT_UPLOAD final
		{
			std::uint32_t iRequestSequence = 0u, iTotalBytes = 0u;
			std::uint64_t iStartedAtMs = 0u;
			LostArk::Shared::GameplayDataRevision RowsRevision{};
			std::string Rows;
		};
		std::unordered_map<SESSION_ID, KOUKU_DRAFT_UPLOAD> m_KoukuDraftUploads;
		KOUKUSAYDON_PATTERN_AUDITION_STATE m_KoukuSaydonPatternAudition;
		std::shared_ptr<const CGameplayCatalog> m_pKoukuPublishedProductGeneration;
		std::unordered_map<SESSION_ID, KOUKUSAYDON_PATTERN_AUDITION_RECEIPT>
			m_KoukuSaydonPatternAuditionReceiptBySessionId;
		std::vector<TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
			m_PendingKoukuSaydonPatternAuditionLifecycle;
		struct TARGETED_VALTAN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanAuditionEpoch = 1u;
		std::vector<TARGETED_VALTAN_AUDITION_LIFECYCLE>
			m_PendingValtanAuditionLifecycle;
		struct TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanPatternFlowEpoch = 1u;
		std::vector<TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE>
			m_PendingValtanPatternFlowLifecycle;
		VALTAN_PATTERN_ID_AUDITION_STATE m_ValtanPatternIdAudition;
		std::optional<VALTAN_NEXT_PATTERN_RESERVATION> m_ValtanNextPattern;
		std::unordered_map<SESSION_ID, VALTAN_NEXT_PATTERN_COMMAND_RECEIPT>
			m_ValtanNextPatternReceiptBySessionId;
		VALTAN_PATTERN_FLOW_AUDITION_STATE m_ValtanPatternFlowAudition;
		VALTAN_TIMELINE_AUDITION_STATE m_ValtanTimelineAudition;
		VALTAN_FIGHT_PAGE_START_STATE m_ValtanFightPageStart;
	};
}
~~~~

<a id="file-server-public-iocpservice-h"></a>

### Server/Public/IocpService.h

파일: C:/Users/tnest/Desktop/LostArk/Server/Public/IocpService.h

서비스·participant 계약과 OVERLAPPED 완료 지표를 선언한다. Windows 헤더는 HANDLE/SOCKET/완료 포트 ABI를 위해 필요하다. shared_ptr는 완료 callback까지의 강한 소유권, weak_ptr 목록은 maintenance 참여자를 보관한다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
#pragma once

// 이 파일은 IOCP 전송 서비스를 학습하기 위한 선언 완성본이다.
// 적용 위치: Server/Public/IocpService.h. 대응 구현: Server/Private/IocpService.cpp.
// 현재 제품 연결 여부는 별개다. 이 파일을 추가하는 것만으로 기존 서버가 IOCP로 바뀌지는 않는다.

// WinSock2가 SOCKET/WSABUF를, Windows가 HANDLE/OVERLAPPED를 제공한다.
// Windows.h가 구형 Winsock 선언을 먼저 포함하지 않도록 이 순서를 유지한다.
#include <WinSock2.h>
#include <Windows.h>

// 헤더를 포함하는 것만으로 경쟁 상태/데드락이 없어지지는 않는다.
// atomic은 한 변수의 연산, mutex는 함께 지켜야 하는 상태 묶음을 실제 코드에서 보호할 때 효과가 있다.
#include <atomic>             // 여러 워커가 공유하는 실행 상태와 완료 횟수.
#include <chrono>             // 종료 대기 제한시간. 시간 단위를 타입으로 구분한다.
#include <condition_variable> // 미처리 I/O가 0이 될 때 종료 담당 스레드를 깨운다.
#include <cstddef>            // 버퍼 길이와 인덱스에 사용하는 size_t.
#include <cstdint>            // 바이트와 누적 카운터의 고정 폭 정수.
#include <memory>             // 작업의 단독 소유, 세션의 강한 참조, 관찰용 약한 참조.
#include <mutex>              // 종료 대기 조건과 참여자 목록 보호.
#include <span>               // 완료 콜백 동안만 빌려 주는 수신 바이트 범위.
#include <thread>             // 서비스가 생성하고 종료 시 join하는 고정 워커 목록.
#include <vector>             // 워커·참여자 목록 및 송신 프레임 저장소.

namespace LostArk::Server
{
    // 이후 ClientSession이 선택할 전송 경로. 게임 규칙이나 패킷 형식을 바꾸는 값이 아니다.
    // SELECT_THREADS는 기존 세션별 수신/송신 스레드, IOCP는 공용 완료 포트 워커를 뜻한다.
    enum class SESSION_TRANSPORT_BACKEND { SELECT_THREADS, IOCP };

    // 같은 완료 포트로 들어온 알림이 수신인지 송신인지 구분한다.
    // OVERLAPPED만으로는 우리 서비스의 작업 종류를 알 수 없으므로 작업 객체에 함께 저장한다.
    enum class IOCP_OPERATION_KIND { RECEIVE, SEND };

    // 서비스가 CClientSession의 게임/큐 구현을 직접 알지 않아도 되도록 만든 콜백 계약이다.
    // 연결 단계에서 CClientSession이 구현한다. 서비스는 바이트 완료를 알리고,
    // 세션은 패킷 조립·송신 offset·연결 종료를 처리한다. GameRoom 상태는 여기서 바꾸지 않는다.
    class IIocpParticipant
    {
    public:
        // Participant는 "완료 통지를 받을 참여자"라는 뜻이다. 여기서는 이후의 CClientSession이다.
        // 순수 가상 함수(=0)는 인터페이스만 정하고 실제 세션 처리는 파생 클래스에 맡긴다.
        // 가상 소멸자는 기반 타입을 통해 파괴할 때도 파생 클래스 정리가 실행되게 하는 계약이다.
        virtual ~IIocpParticipant() = default;

        // 호출자: CIocpService::Worker_Loop의 IOCP 워커.
        // kind는 수신/송신, bytes는 이번 완료의 바이트 수, error는 Windows 오류 코드다.
        // received는 수신 때만 유효하며 소유권이 없는 span이다. 콜백 반환 후 보관하면 안 된다.
        // 작업 객체가 Owner의 shared_ptr를 잡아 취소 완료를 포함한 콜백 전체 동안 세션을 살린다.
        // 서비스의 목록/종료 mutex를 잡지 않은 상태로 호출한다. 세션 내부 공유 상태는
        // 세션이 동기화해야 하며 서로 다른 작업·maintenance 콜백의 직렬 실행을 보장하지 않는다.
        virtual void On_IocpCompleted(IOCP_OPERATION_KIND kind,
            std::span<const std::uint8_t> received, std::uint32_t bytes,
            std::uint32_t error) noexcept = 0;

        // 호출자: 워커의 Run_Maintenance. 조용한 연결도 시간 초과 등을 확인할 기회를 준다.
        // 약 100ms마다 점검을 시도하지만 실시간 주기 보장은 아니다. 게임의 30Hz Tick과 다르다.
        virtual void On_IocpMaintenance() noexcept = 0;
    };

    // Get_Metrics가 반환하는 값 복사본. atomic 자체를 외부에 공개하지 않는다.
    // 여러 카운터를 차례로 읽으므로 실행 중 완전히 같은 순간의 스냅샷은 아니다.
    // I/O 완료 횟수는 게임 패킷 개수와 다르다. TCP에서는 한 패킷이 여러 수신으로 쪼개질 수 있다.
    // 한 번의 bytes는 Windows DWORD에 맞는 uint32_t지만 누적 합계는 훨씬 커져 uint64_t로 둔다.
    // 예를 들어 uint32_t 누적 바이트는 약 4GiB에서 넘친다. 64비트는 장시간 누적의 여유를 준다.
    struct IOCP_SERVICE_METRICS final
    {
        std::size_t iWorkerCount = 0;                  // 서비스가 보유한 워커 수.
        std::uint64_t iPostedOperations = 0;          // 즉시 성공 또는 PENDING으로 접수된 작업 수.
        std::uint64_t iCompletedOperations = 0;       // 성공/실패 콜백을 처리한 완료 수.
        std::uint64_t iPendingOperations = 0;         // 제출 예약 후 아직 정리되지 않은 작업 수.
        std::uint64_t iPeakPendingOperations = 0;     // 위 미처리 수의 최대값.
        std::uint64_t iReceiveCompletions = 0;        // 오류를 포함한 수신 완료 알림 수.
        std::uint64_t iSendCompletions = 0;           // 오류를 포함한 송신 완료 알림 수.
        std::uint64_t iPartialSendCompletions = 0;    // 성공했지만 요청 길이보다 짧은 양의 송신 완료.
        std::uint64_t iReceivedBytes = 0;             // 완료 알림이 보고한 수신 바이트 합계.
        std::uint64_t iSentBytes = 0;                 // 완료 알림이 보고한 송신 바이트 합계.
    };

    // CServerApp 또는 독립 실험기가 서비스 하나를 소유하고 여러 세션이 공유하는 구조다.
    // 세션마다 기다리는 스레드를 만들지 않고 완료된 작업을 고정 개수의 워커가 처리한다.
    // 그 대신 작업/버퍼 수명, 부분 송신, 취소 완료와 종료 순서를 명시적으로 관리해야 한다.
    //
    // 수명 계약: 모든 세션의 새 제출을 막고 소켓을 닫아 취소를 시작한 뒤 이 서비스를 Stop한다.
    // Stop과 새 Submit을 임의로 동시에 호출해도 안전한 범용 큐가 아니다.
    // m_Running의 atomic 검사 한 번은 제출자 전체를 멈추는 장벽이 될 수 없다.
    // 서비스는 마지막 세션과 완료 콜백보다 오래 살아 있어야 한다.
    class CIocpService final
    {
    public:
        CIocpService() = default;
        ~CIocpService(); // 소유자가 종료를 누락해도 Stop 경로로 정리한다.

        // HANDLE과 실행 중 워커를 복사하면 이중 종료가 생기므로 복사를 금지한다.
        CIocpService(const CIocpService&) = delete;
        CIocpService& operator=(const CIocpService&) = delete;

        // 호출자: 이후 ServerApp의 시작 단계. 포트 생성 후 워커를 시작한다.
        // 0은 CPU 동시 실행 수를 참고하며 이 구현은 1~16개로 제한한다. 최적값 보장은 아니다.
        bool Start(std::size_t workerCount = 0);

        // 호출자: 워커가 아닌 소유자 스레드. 세션 종료 후 완료를 모두 회수하고 워커를 join한다.
        // 5000은 5초 후 종료를 예약한다는 뜻이 아니다. 지금 종료를 시작하고 기다릴 상한을 지정한다.
        // 미완료 버퍼를 해제한 채 계속 실행하지 않도록 제한시간 초과 시 프로세스를 종료한다.
        // timeout은 drain 단계와 워커 join 단계에 각각 사용되며 전체 5초 보장이 아니다.
        void Stop(std::chrono::milliseconds timeout = std::chrono::milliseconds{5000});

        // accept된 overlapped 소켓을 이 포트에 연결한다. accept/listen/WSAStartup은 외부 책임이다.
        bool Associate(SOCKET socket);

        // 주기 점검 대상에 약한 참조를 넣는다. 세션 소유자가 사라지면 목록 때문에 살아남지 않는다.
        // 한 세션은 한 번 등록한다. 이 함수는 중복 등록을 제거하지 않는다.
        void Register(const std::shared_ptr<IIocpParticipant>& participant);

        // 호출자: 세션 시작 또는 직전 수신 완료. 4096-byte 버퍼와 owner를 보존해 수신을 맡긴다.
        // owner는 null이 아니어야 한다. true는 접수 성공이며 데이터 도착을 뜻하지 않는다.
        bool Post_Receive(SOCKET socket, const std::shared_ptr<IIocpParticipant>& owner,
            int& error);

        // 호출자: 세션의 송신 시작/부분 완료 처리. bytes 전체 중 offset 이후 구간을 보낸다.
        // 변경 불가능한 프레임을 공유 소유하여 비동기 완료까지 메모리가 유지되게 한다.
        // owner는 null이 아니어야 한다. bytes도 완료 전까지 다른 mutable 참조로 수정하거나
        // 재할당하지 않아야 한다. shared_ptr<const> 하나가 다른 참조의 쓰기까지 금지하지는 않는다.
        // 여기서는 offset을 누적하지 않는다. 실제 완료 bytes를 보고 세션이 다음 offset을 정한다.
        bool Post_Send(SOCKET socket, const std::shared_ptr<IIocpParticipant>& owner,
            std::shared_ptr<const std::vector<std::uint8_t>> bytes,
            std::size_t offset, int& error);

        // [[nodiscard]]는 반환값을 버리는 호출에 컴파일러가 진단하도록 권하는 표시다.
        // 값을 버리지 못하게 런타임에서 강제하거나 스레드 안전성을 추가하는 기능은 아니다.
        // 자기 자신인 워커를 join하려는 잘못된 종료 호출을 감지한다.
        [[nodiscard]] bool Is_WorkerThread() const noexcept;

        // Start/Stop이 m_Workers를 바꾸는 동안 동시에 호출하지 않는다.
        [[nodiscard]] std::size_t Get_WorkerCount() const noexcept;
        [[nodiscard]] IOCP_SERVICE_METRICS Get_Metrics() const noexcept;

    private:
        // 외부가 포트·pending·작업 목록을 직접 바꾸면 "접수 한 번/회수 한 번" 순서를 깨뜨린다.
        // public 함수로 가능한 동작만 허용하고 내부 표현과 정리 순서는 private에서 유지한다.
        // OVERLAPPED와 Owner/버퍼의 실제 배치는 CPP에 숨긴다. 외부에는 제출 함수만 공개한다.
        // 이것이 중첩 타입의 전방 선언이다. 이 시점에는 크기를 몰라도 함수 선언에 사용할 수 있다.
        struct OPERATION;

        // Post_Receive/Post_Send 공통 경로. pending 예약과 Windows 호출·접수 실패 정리를 소유한다.
        bool Submit(SOCKET socket, std::unique_ptr<OPERATION> operation, int& error);
        void Worker_Loop() noexcept;       // 완료 포트 대기 → 작업 회수 → 세션 콜백 → pending 감소.
        void Run_Maintenance() noexcept;   // 유효한 세션 목록 복사 → 잠금 해제 → 주기 점검 콜백.
        void Complete_Pending() noexcept;  // 접수 실패/완료 처리에서 작업 하나를 정리하고 0이면 알림.

        HANDLE m_Port = nullptr;                // 서비스 소유의 완료 포트. worker join 뒤 CloseHandle.
        std::vector<std::thread> m_Workers;     // Start/Stop 소유자 스레드가 생성·정리하는 워커.
        std::atomic_bool m_Running{false};      // 새 제출 허용 상태. 이미 접수한 작업의 완료는 계속 회수.
        // atomic의 fetch_add는 다른 스레드와 겹쳐도 증가 한 건을 잃지 않는다.
        // 주변의 다른 변수까지 잠그거나 여러 atomic을 하나의 transaction으로 만들지는 않는다.
        // atomic 자체가 모든 플랫폼에서 lock-free라는 보장도 없다.
        std::atomic<std::uint64_t> m_Pending{0}, m_PeakPending{0}, m_Posted{0}, m_Completed{0};
        // 아래 두 줄로 나눈 것은 각각 완료 횟수와 바이트 합계라는 의미를 묶은 것이다.
        // 줄을 나눈다고 동기화 범위가 달라지지는 않는다. 각 atomic은 서로 독립된 변수다.
        std::atomic<std::uint64_t> m_ReceiveCompletions{0}, m_SendCompletions{0}, m_PartialSends{0};
        std::atomic<std::uint64_t> m_ReceivedBytes{0}, m_SentBytes{0};

        // wait_for의 조건 검사와 pending=0 알림을 함께 보호해 종료 대기의 깨움 누락을 막는다.
        // 단순 spin으로 계속 CPU를 쓰지 않게 condition_variable을 사용한다.
        std::mutex m_DrainMutex;
        std::condition_variable m_Drained;

        // 목록 변경/복사만 잠근다. 세션 콜백까지 잠그면 세션 잠금과 역순 경합이 생길 수 있다.
        std::mutex m_ParticipantsMutex;
        std::vector<std::weak_ptr<IIocpParticipant>> m_Participants;

        // GetTickCount64 기준 다음 점검 시각(ms). CAS에 성공한 워커가 그 차례의 점검을 맡는다.
        // 콜백이 100ms보다 오래 걸리면 다음 차례와 겹칠 수 있으므로 세션 동기화가 필요하다.
        std::atomic<ULONGLONG> m_NextMaintenance{0};
    };
}
~~~~

<a id="file-server-public-serverapp-h"></a>

### Server/Public/ServerApp.h

파일: C:/Users/tnest/Desktop/LostArk/Server/Public/ServerApp.h

제품 옵션, IOCP service와 job pool 수명을 ServerApp이 소유한다. session, shared/private room 생성에 같은 선택을 전달한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~cpp
#pragma once

//Room 객체를 값으로 소유
#include "GameRoom.h"
#include "ServerBalanceNumericStore.h"
#include "ServerConcurrencyOptions.h"
#include <future>
//sessionid 사용
#include "ServerIds.h"
//접속 수락
#include "TcpListener.h"
//winsock 시작과 종료
#include "WinSockContext.h"
//PACKET_FRAME header
#include "Network/PacketFrame.h"
#include "Network/SessionDiagnostic.h"

//서버 실행 상태와 다음 sessionid
#include <atomic>
//accept thread와 room thread
#include <thread>
//shared_ptr
#include <memory>
//sessionid로 session 검색
#include <unordered_map>
//session map과 종료 queue 보호
#include <mutex>
//종료된 sessionid 순서 보관
#include <deque>
#include <vector>
#include <map>
#include <cstdint>
#include <string_view>
#include <chrono>
#include <filesystem>

// CServerApp은 서버 전체의 시작, 실행, 종료를 조율하는 최상위 객체다.
// WinSock/Listener 수명, Accept Thread, Room Thread, ClientSession 집합을 소유한다.
// ClientSession이 전달한 Frame을 Shared 메시지로 읽어 ROOM_COMMAND로 번역할 뿐,
// Player 상태 변경과 Broadcast 결정은 CGameRoom에 위임한다.

namespace LostArk::Server
{
	class CServerGameplayContractRunner;

	//clientsession 전방 선언
	class CClientSession;

	enum class RUNTIME_GAMEPLAY_GENERATION_SOURCE : std::uint8_t
	{
		PACKAGED_BASELINE,
		CANDIDATE
	};

	/* Process-local durable identity. It never trusts the publisher's
	current-candidate pointer: only a Server 2PC commit writes this record. */
	struct RUNTIME_ACTIVE_GAMEPLAY_GENERATION final
	{
		RUNTIME_GAMEPLAY_GENERATION_SOURCE eSource =
			RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
		LostArk::Shared::GameplayDataRevision Revision{};
		LostArk::Shared::GameplayDataRevision BootstrapContentRevision{};
		LostArk::Shared::GameplayDataRevision NonValtanGameplayRevision{};

		[[nodiscard]] bool Is_Valid() const noexcept
		{
			return Revision.Is_Valid() && BootstrapContentRevision.Is_Valid() &&
				NonValtanGameplayRevision.Is_Valid();
		}
	};

	class CServerApp final
	{
		friend class CServerGameplayContractRunner;
		friend int Run_ServerBingoContractTests();
	public:
		//소멸자 - 중간 실패나 정상 종료 여부 상관 없이
		//socket과 thread를 정리
		~CServerApp();
		//서버의 진입점
		int Run(
			std::uint32_t automaticShutdownMilliseconds = 0,
			std::string_view bindAddress = "0.0.0.0",
			std::uint16_t port = 7777u,
			bool headless = false,
			SERVER_CONCURRENCY_OPTIONS concurrency = {});
		/* Debug-only offline recovery. It never resets a live encounter: it
		acquires the same process lock as Run, validates the durable pointer/journal
		old/new identity structure, and atomically selects the packaged bootstrap
		for next start without re-admitting the discarded candidate artifacts. */
		static int Reset_ValtanRuntimeToPackaged();

	private:
		struct SERVER_CONTROL_EVENT;
		//접속을 계속 받아 새로운 session을 생성
		void Accept_Loop();
		//GameRoom을 일정한 간격으로 tick한다. 초기 30Hz
		void Room_Loop();
		//session receive thread가 완성된 frame을 전달하는 callback이다
		void On_SessionFrame(
			SESSION_ID sessionId,
			const LostArk::Shared::PACKET_FRAME& frame);
		//session receive thread가 연결 종료를 발견했을 때 호출한다.
		void On_SessionClosed(SESSION_ID sessionId);
		//Session map에서 대상을 찾고, request_close만 호출
		void Request_SessionClose(
			SESSION_ID sessionId,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
			int nativeErrorCode = 0,
			std::string_view context = {});
		//종료 callback이 남긴 sessionid를 안전하게 정리
		void Reap_ClosedSessions();

		struct SESSION_GAMEPLAY_BINDING
		{
			LostArk::Shared::WORLD_ID eWorldId =
				LostArk::Shared::WORLD_ID::END;
			SESSION_ID iPrivateArenaOwnerSessionId =
				INVALID_SESSION_ID;
			std::shared_ptr<CGameRoom> pSimulation;
		};

		std::shared_ptr<CGameRoom> Acquire_EntrySimulation(
			SESSION_ID sessionId,
			LostArk::Shared::WORLD_ID worldId);
		std::shared_ptr<CGameRoom> Find_SharedSimulation(
			LostArk::Shared::WORLD_ID worldId);
		bool Bind_AndEnqueueEntry(
			SESSION_ID sessionId,
			LostArk::Shared::WORLD_ID worldId,
			const std::shared_ptr<CGameRoom>& simulation,
			LostArk::Shared::C2S_ENTER_WORLD enterWorld,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outFailureReason,
			int& outNativeErrorCode,
			std::string& outFailureContext);
		ROOM_COMMAND_ENQUEUE_RESULT Enqueue_AssignedCommand(
			SESSION_ID sessionId,
			ROOM_COMMAND command,
			std::string& outContext);
		void Tick_GameplaySimulations(float fixedDeltaSeconds,
			const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics = {});
		void Advance_ServerControlTransactions();
        void Advance_NumericBalanceTransaction();
        void Process_NumericBalanceEvent(const SERVER_CONTROL_EVENT& event);
        void Send_NumericBalanceResult(SESSION_ID sessionId, std::uint32_t sequence,
            LostArk::Shared::BALANCE_APPLY_RESULT result, std::string reason);
        void Finish_NumericBalanceTransaction(bool commit, std::string status);
		void Process_ValtanDecisionTraceQuery(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request);
		bool Queue_ServerControlEvent(SERVER_CONTROL_EVENT&& event);
		static bool Resolve_CandidateArtifactForAdmission(
			const std::filesystem::path& candidateDirectory,
			const std::string& relativePath,
			std::filesystem::path& resolvedPath,
			std::string& status);
		static bool Build_NonValtanGameplayRevisionForAdmission(
			const std::filesystem::path& bootstrapPath,
			LostArk::Shared::GameplayDataRevision& revision,
			std::string& status);
		static bool Hash_GameplayFileForAdmission(
			const std::filesystem::path& path,
			LostArk::Shared::GameplayDataRevision& revision,
			std::string& status);
		/* HOT_RELOAD cannot truthfully apply fields copied into a live boss body.
		   Until an encounter-reset transaction exists, admission must reject any
		   candidate that changes those fields instead of reporting COMMITTED. */
		static bool Validate_ValtanHotReloadBaseProfile(
			const BOSS_RUNTIME_PROFILE* activeProfile,
			const BOSS_RUNTIME_PROFILE* candidateProfile,
			std::string& status);
		static bool Validate_ServerSessionDiagnosticJson(
			std::string_view json,
			std::string& status);
		static bool Persist_RuntimeGameplayActivation(
			const std::filesystem::path& runtimeRoot,
			std::uint32_t transactionSequence,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& candidate,
			std::string& status);
		static bool Rollback_RuntimeGameplayActivation(
			const std::filesystem::path& runtimeRoot,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
			std::string& status);
		static void Complete_RuntimeGameplayActivation(
			const std::filesystem::path& runtimeRoot) noexcept;
		static bool Recover_RuntimeActiveGameplayPointer(
			const std::filesystem::path& runtimeRoot,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
			RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active,
			bool& hasPersistedPointer,
			std::string& status,
			bool allowPackagedIdentityDrift = false);
		static bool Reset_RuntimeGameplayActivationToPackaged(
			const std::filesystem::path& runtimeRoot,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
			std::string& status);
		static bool Acquire_RuntimeGameplayProcessMutex(
			void*& handle,
			std::string& status);
		// Production keeps its fixed global name; the friend contract owns an
		// isolated name while exercising this same Win32 ownership path.
		static bool Acquire_NamedRuntimeGameplayProcessMutex(
			const wchar_t* name,
			void*& handle,
			std::string& status);
		static void Release_RuntimeGameplayProcessMutex(void*& handle) noexcept;
		static bool Load_RuntimeActiveGameplayGeneration(
			const std::filesystem::path& runtimeRoot,
			const std::shared_ptr<const CGameplayCatalog>& packagedGeneration,
			const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
			std::shared_ptr<const CGameplayCatalog>& activeGeneration,
			RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active,
			bool& isCandidate,
			std::string& status);
		[[nodiscard]] std::filesystem::path
			Resolve_RuntimeActiveGameplayRoot() const;
		void Abort_DataRevisionTransaction(std::string reason);
		bool Commit_DataRevisionTransaction();
		bool Validate_DataRevisionTransactionMembership(
			std::string& status);
		bool Send_DataRevisionPrepare(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request);
		bool Send_DataRevisionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request,
			LostArk::Shared::DATA_REVISION_RESULT result,
			const LostArk::Shared::GameplayDataRevision& activeRevision,
			std::string reason);
		void Retire_QuiescentCharacterSelectArenas();
		void Retire_QuiescentColosseumMatches();
		bool Begin_ColosseumPreparation(const std::shared_ptr<CGameRoom>& source,
			const SERVER_WORLD_TRANSFER_REQUEST& transfer);
		void Advance_ColosseumPreparation();
		void Handle_WorldTransfers(
			const std::shared_ptr<CGameRoom>& sourceSimulation);
		struct SESSION_WORLD_TRANSFER_FAILURE final
		{
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON eReason =
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED;
			int iNativeErrorCode = 0;
			std::string strContext;
			bool bRollbackCleanupRequired = false;
			bool bSourcePreservedOnRejection = false;
			bool bRollbackCleanupEnqueued = false;
			LostArk::Shared::PARTY_TRANSFER_RESULT ePartyResult =
				LostArk::Shared::PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE;
		};
		bool Transfer_SessionWorld(
			const std::shared_ptr<CGameRoom>& sourceSimulation,
			const SERVER_WORLD_TRANSFER_REQUEST& transfer,
			SESSION_WORLD_TRANSFER_FAILURE& outFailure);
		//서버 전체 종료 순서 한 곳으로 모아서 정리
		void Shutdown();

		enum class SERVER_CONTROL_EVENT_KIND : std::uint8_t
		{
			DATA_REVISION_REQUEST,
			DATA_REVISION_RESPONSE,
			SESSION_DISCONNECTED,
			VALTAN_DECISION_TRACE_QUERY,
            BALANCE_QUERY, BALANCE_PATCH, BALANCE_DEFERRED_ENTRY
		};

		struct SERVER_CONTROL_EVENT final
		{
			SERVER_CONTROL_EVENT_KIND eKind =
				SERVER_CONTROL_EVENT_KIND::SESSION_DISCONNECTED;
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST RevisionRequest{};
			LostArk::Shared::C2S_DATA_REVISION_PREPARE_RESPONSE RevisionResponse{};
			LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY DecisionTraceQuery{};
            LostArk::Shared::C2S_BALANCE_QUERY BalanceQuery{};
            LostArk::Shared::C2S_BALANCE_PATCH BalancePatch{};
            LostArk::Shared::PACKET_FRAME DeferredEntry{};
			std::shared_ptr<const CGameplayCatalog> pCandidateGeneration;
			LostArk::Shared::GameplayDataRevision
				BaseBootstrapContentRevision{};
			LostArk::Shared::GameplayDataRevision
				CandidateBootstrapContentRevision{};
			LostArk::Shared::GameplayDataRevision BaseNonValtanGameplayRevision{};
			LostArk::Shared::GameplayDataRevision
				CandidateNonValtanGameplayRevision{};
		};

		struct DATA_REVISION_PARTICIPANT final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::WORLD_ID eWorldId =
				LostArk::Shared::WORLD_ID::END;
			std::shared_ptr<CClientSession> pSession;
			std::shared_ptr<CGameRoom> pSimulation;
			bool hasResponded = false;
			bool isReady = false;
		};

		struct DATA_REVISION_TRANSACTION final
		{
			SESSION_ID iRequesterSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST Request{};
			std::shared_ptr<const CGameplayCatalog> pCandidateGeneration;
			LostArk::Shared::GameplayDataRevision
				BaseBootstrapContentRevision{};
			LostArk::Shared::GameplayDataRevision
				CandidateBootstrapContentRevision{};
			LostArk::Shared::GameplayDataRevision BaseNonValtanGameplayRevision{};
			LostArk::Shared::GameplayDataRevision
				CandidateNonValtanGameplayRevision{};
			std::vector<DATA_REVISION_PARTICIPANT> Participants;
			std::vector<std::shared_ptr<CGameRoom>> Simulations;
			std::chrono::steady_clock::time_point Deadline{};

			[[nodiscard]] bool Is_Active() const noexcept
			{
				return INVALID_SESSION_ID != iRequesterSessionId;
			}
		};

		struct DATA_REVISION_RESPONSE_TOMBSTONE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			std::uint32_t iTransactionSequence = 0u;
			LostArk::Shared::GameplayDataRevision CandidateRevision{};
			std::uint32_t iRequiredPresentationLaneMask = 0u;
			std::chrono::steady_clock::time_point ExpiresAt{};
		};

	private:
		//서버 기반 객체
		//멤버 순서 중요! 생성 : WinSockContext -> TcpListener
		//소멸 : TcpListener -> WinSockContext 소멸은 역순
		CWinSockContext m_WinSockContext;
		CTcpListener m_TcpListener;
		std::map<
			LostArk::Shared::WORLD_ID,
			std::shared_ptr<CGameRoom>> m_SharedGameRooms;
		std::shared_ptr<const CGameplayCatalog> m_pActiveGameplayGeneration;
		/* Manifest identity and raw bootstrap content identity are deliberately
		   separate. Binding both prevents an old Valtan-only candidate from
		   rolling back newer non-Valtan rows in the full bootstrap. */
		LostArk::Shared::GameplayDataRevision
			m_ActiveGameplayBootstrapContentRevision{};
		LostArk::Shared::GameplayDataRevision
			m_ActiveNonValtanGameplayRevision{};
		bool m_isActiveGameplayGenerationFromCandidate = false;
		bool m_isRuntimeActivePersistenceEnabled = false;
		std::filesystem::path m_RuntimeActiveGameplayRootOverride;
		void* m_hRuntimeGameplayProcessMutex = nullptr;
		std::unordered_map<
			SESSION_ID,
			std::shared_ptr<CGameRoom>> m_CharacterSelectArenas;
		std::map<std::uint64_t, std::shared_ptr<CGameRoom>> m_ColosseumMatches;
		std::uint64_t m_iNextColosseumMatchId = 1u;
		struct COLOSSEUM_PREPARATION_RESULT final
		{
			std::shared_ptr<CGameRoom> Room;
			std::string Status;
			double ElapsedMilliseconds = 0.;
		};
		// Only the RoomThread owns the future; the worker captures immutable inputs
		// and a cancellation flag, never this or live session/player containers.
		std::future<COLOSSEUM_PREPARATION_RESULT> m_ColosseumPreparationWorker;
		std::shared_ptr<std::atomic_bool> m_ColosseumPreparationCancelled;
		std::shared_ptr<CGameRoom> m_ColosseumPreparationSource;
		SERVER_WORLD_TRANSFER_REQUEST m_ColosseumPreparationTransfer;
		std::shared_ptr<CGameRoom> m_PreparedColosseumRoom;
		//실행 상태 - 여러 스레드가 읽고 쓰기 때문에 atomic을 사용한다.
		SERVER_CONCURRENCY_OPTIONS m_Concurrency{};
		std::shared_ptr<CIocpService> m_IocpService;
		std::shared_ptr<LostArk::Shared::Concurrency::WorkStealingJobSystem> m_SnapshotJobs;
		std::atomic_bool m_isRunning{ false };
		std::atomic<SESSION_ID> m_iNextSessionId{ 1 };

		//thread
		std::thread m_AcceptThread;
		std::thread m_RoomThread;
		bool m_bRoomPerformanceLogWarningReported = false;

		//session owner map - sessionid의 유일한 장기 강한 owner
		//gameroom은 같은 session을 weak_ptr로만 참조
		std::mutex m_SessionsMutex;
		std::unordered_map<
			SESSION_ID,
			std::shared_ptr<CClientSession>> m_Sessions;
		// sessions, bindings, shared rooms, and private arenas share this mutex.
		std::unordered_map<
			SESSION_ID,
			SESSION_GAMEPLAY_BINDING> m_GameplayBindingBySessionId;
		//종료 대기 Queue
		std::mutex m_ClosedSessionMutex;
		std::deque<SESSION_ID> m_ClosedSessionIds;
		std::mutex m_SessionDiagnosticLogMutex;
		/* Receive callbacks publish only bounded immutable control events. The
		   room thread drains them without this mutex held, then takes sessions
		   before touching any room, preserving sessions -> room lock order. */
		static constexpr std::size_t MAX_SERVER_CONTROL_EVENTS = 256u;
		std::mutex m_ServerControlMutex;
		std::deque<SERVER_CONTROL_EVENT> m_ServerControlEvents;
		std::mutex m_DataRevisionAdmissionMutex;
		DATA_REVISION_TRANSACTION m_DataRevisionTransaction;
        struct BALANCE_WORK_RESULT final
        {
            std::shared_ptr<SERVER_BALANCE_PREPARED> Prepared;
            LostArk::Shared::GameplayDataRevision BootstrapRevision{}, NonValtanRevision{};
            LostArk::Shared::BALANCE_APPLY_RESULT Failure = LostArk::Shared::BALANCE_APPLY_RESULT::INVALID_CHANGE;
            bool Succeeded = false;
            std::string Status;
        };
        CServerBalanceNumericStore m_NumericBalanceStore;
        std::future<BALANCE_WORK_RESULT> m_NumericBalanceWorker;
        BALANCE_WORK_RESULT m_NumericBalancePrepared;
        std::vector<std::shared_ptr<CGameRoom>> m_NumericBalanceSimulations;
        SESSION_ID m_NumericBalanceRequester = INVALID_SESSION_ID;
        std::uint32_t m_NumericBalanceRequestSequence = 0u;
        std::uint32_t m_NumericBalanceTransactionSequence = 0u;
        bool m_NumericBalancePersisting = false;
        // Guarded by m_DataRevisionAdmissionMutex; only blocks entry during disk commit.
        bool m_NumericAdmissionPaused = false;
		static constexpr std::size_t
			MAX_DATA_REVISION_RESPONSE_TOMBSTONES = 1024u;
		std::deque<DATA_REVISION_RESPONSE_TOMBSTONE>
			m_DataRevisionResponseTombstones;
	};
}
~~~~

<a id="file-server-public-serverconcurrencyoptions-h"></a>

### Server/Public/ServerConcurrencyOptions.h

파일: C:/Users/tnest/Desktop/LostArk/Server/Public/ServerConcurrencyOptions.h

제품 시작 시 선택한 transport와 executor, 각 worker 수를 보관한다. 기본 select+serial이며 gameplay 입력·protocol은 바꾸지 않는다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
#pragma once
#include "IocpService.h"
#include <cstdint>

namespace LostArk::Server
{
    struct SERVER_CONCURRENCY_OPTIONS final
    {
        SESSION_TRANSPORT_BACKEND Transport = SESSION_TRANSPORT_BACKEND::SELECT_THREADS;
        std::uint32_t IocpWorkers = 2;
        bool SnapshotJobs = false;
        std::uint32_t JobWorkers = 2;
    };
}
~~~~

<a id="file-server-public-serversnapshotfanout-h"></a>

### Server/Public/ServerSnapshotFanout.h

파일: C:/Users/tnest/Desktop/LostArk/Server/Public/ServerSnapshotFanout.h

room owner가 고정한 recipients와 payload를 차용하는 동기 API다. 결과는 recipient 수·실패 수·벽시계·최대 세션 비용·제출 job 수다. nullptr job pool은 직렬 실행이다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace LostArk::Shared::Concurrency { class WorkStealingJobSystem; }

namespace LostArk::Server
{
    class CClientSession;

    struct SNAPSHOT_FANOUT_RESULT final
    {
        std::uint64_t Recipients = 0;
        std::uint64_t Failures = 0;
        std::uint64_t ElapsedMicroseconds = 0;
        std::uint64_t MaximumSessionMicroseconds = 0;
        std::uint64_t SubmittedJobs = 0;
    };

    // The room owner freezes recipients and payload until this synchronous call
    // returns. Each job writes one distinct result and one session's bounded
    // outbound queue. Gameplay state, routing and failure callbacks stay on the
    // owner. nullptr selects the original serial submission order.
    SNAPSHOT_FANOUT_RESULT Dispatch_WorldSnapshot(
        std::span<const std::shared_ptr<CClientSession>> recipients,
        std::span<const std::uint8_t> payload,
        Shared::Concurrency::WorkStealingJobSystem* jobs);
}
~~~~

<a id="file-shared-default-shared-vcxproj"></a>

### Shared/Default/Shared.vcxproj

파일: C:/Users/tnest/Desktop/LostArk/Shared/Default/Shared.vcxproj

물리 소스에 대응하는 build/IDE 항목이다. 기존 filter를 재배치하지 않고 필요한 항목만 등록한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <ItemGroup>
	<ClInclude Include="..\Public\GameplayDataRevision.h" />
	<ClInclude Include="..\Public\Gameplay\CombatCollisionContract.h" />
	<ClInclude Include="..\Public\Gameplay\CombatObjectHitChain.h" />
	<ClInclude Include="..\Public\Gameplay\WorldCollisionContract.h" />
	<ClInclude Include="..\Public\Gameplay\MaharakaWaterpangContract.h" />
	<ClInclude Include="..\Public\Gameplay\AttackHitTemplate.h" />
	<ClInclude Include="..\Public\Gameplay\KoukuMarioBombContract.h" />
	<ClInclude Include="..\Public\Gameplay\EstherStrikeContract.h" />
	<ClInclude Include="..\Public\Gameplay\KoukuTargetTracking.h" />
	<ClInclude Include="..\Public\Gameplay\KoukuArenaReadyAreas.h" />
    <ClInclude Include="..\Public\Network\NetworkIds.h" />
    <ClInclude Include="..\Public\Network\PacketFrame.h" />
    <ClInclude Include="..\Public\Network\PacketMessages.h" />
    <ClInclude Include="..\Public\Network\PacketReader.h" />
    <ClInclude Include="..\Public\Network\SessionDiagnostic.h" />
    <ClInclude Include="..\Public\Network\PacketStreamParser.h" />
    <ClInclude Include="..\Public\Network\PacketType.h" />
    <ClInclude Include="..\Public\Network\PacketWriter.h" />
  </ItemGroup>
  <ItemGroup>
	<ClCompile Include="..\Private\GameplayDataRevision.cpp" />
	<ClCompile Include="..\Private\Gameplay\CombatCollisionContract.cpp" />
    <ClCompile Include="..\Private\Network\PacketFrame.cpp" />
    <ClCompile Include="..\Private\Network\PacketMessages.cpp" />
    <ClCompile Include="..\Private\Network\PacketReader.cpp" />
    <ClCompile Include="..\Private\Network\PacketStreamParser.cpp" />
    <ClCompile Include="..\Private\Network\PacketWriter.cpp" />
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <Keyword>Win32Proj</Keyword>
    <ProjectGuid>{F4CCF815-6D51-412F-A76E-84D2F1D05571}</ProjectGuid>
    <RootNamespace>Shared</RootNamespace>
    <ProjectName>Shared</ProjectName>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>StaticLibrary</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>StaticLibrary</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'" Label="Configuration">
    <ConfigurationType>StaticLibrary</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'" Label="Configuration">
    <ConfigurationType>StaticLibrary</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <WholeProgramOptimization>true</WholeProgramOptimization>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings" />
  <ImportGroup Label="Shared" />
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <OutDir>$(ProjectDir)..\Bin\$(Configuration)\</OutDir>
    <IntDir>$(ProjectDir)..\Intermediate\$(Platform)\$(Configuration)\</IntDir>
    <TargetName>Shared</TargetName>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>WIN32;NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Lib>
      <LinkTimeCodeGeneration>true</LinkTimeCodeGeneration>
    </Lib>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <SDLCheck>true</SDLCheck>
      <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ConformanceMode>true</ConformanceMode>
      <AdditionalIncludeDirectories>$(ProjectDir)..\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <LanguageStandard>stdcpp20</LanguageStandard>
      <PrecompiledHeader>NotUsing</PrecompiledHeader>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>
    </ClCompile>
    <Lib>
      <LinkTimeCodeGeneration>true</LinkTimeCodeGeneration>
    </Lib>
  </ItemDefinitionGroup>
  <Import Project="..\..\Tools\Build\CppCompilation.props" />
  <ItemGroup>
    <ClCompile Include="..\Private\Shared_Pch.cpp">
      <PrecompiledHeader Condition="'$(LostArkUsePch)'=='true' and '$(Platform)'=='x64'">Create</PrecompiledHeader>
      <ExcludedFromBuild Condition="'$(LostArkUsePch)'!='true' or '$(Platform)'!='x64'">true</ExcludedFromBuild>
    </ClCompile>
    <ClInclude Include="..\..\Tools\Build\CppStandardPch.h" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Gameplay\BalanceNumericContract.h" />
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\Gameplay\BalanceNumericContract.cpp" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets" />
  <ItemGroup>
    <ClInclude Include="..\Public\Concurrency\ChaseLevDeque.h" />
    <ClInclude Include="..\Public\Concurrency\WorkStealingJobSystem.h" />
    <ClCompile Include="..\Private\Concurrency\WorkStealingJobSystem.cpp" />
  </ItemGroup>
</Project>
~~~~

<a id="file-shared-default-shared-vcxproj-filters"></a>

### Shared/Default/Shared.vcxproj.filters

파일: C:/Users/tnest/Desktop/LostArk/Shared/Default/Shared.vcxproj.filters

물리 소스에 대응하는 build/IDE 항목이다. 기존 filter를 재배치하지 않고 필요한 항목만 등록한다.

적용: 기존 파일을 아래 최종 전문과 대조한다. 기준 commit의 무관한 기존 내용을 보존한 전체 변경본이다.

~~~~xml
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003" ToolsVersion="4.0">
  <ItemGroup>
    <Filter Include="00.Network">
      <UniqueIdentifier>{FA7865EE-9C52-4D8B-9165-A21FB39F70E1}</UniqueIdentifier>
    </Filter>
    <Filter Include="01.Protocol">
      <UniqueIdentifier>{9E3E104B-6763-4827-8B64-1F54D73B117B}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.Gameplay">
      <UniqueIdentifier>{2EFBFD79-7C18-4DFC-AD54-D73A710BD66C}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.Gameplay\00.Combat">
      <UniqueIdentifier>{31E24345-33B6-4CE5-99EB-92D0FD469F8A}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.Gameplay\01.World">
      <UniqueIdentifier>{8A42F549-F232-5566-828E-258E728F0EC5}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.Gameplay\02.KoukuSaydon">
      <UniqueIdentifier>{E1E77851-75EB-5C63-9BE0-862C15314A29}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.Gameplay\03.Maharaka">
      <UniqueIdentifier>{44F56421-AC17-5431-A5CB-7C933BC86987}</UniqueIdentifier>
    </Filter>
    <Filter Include="02.Gameplay\04.Balance">
      <UniqueIdentifier>{55868C1E-065E-52A3-8C30-2F25B89F7B04}</UniqueIdentifier>
    </Filter>
    <Filter Include="03.Revision">
      <UniqueIdentifier>{49D8F7EE-0874-4E46-B10D-C28D4DBBA46A}</UniqueIdentifier>
    </Filter>
    <Filter Include="99.Default">
      <UniqueIdentifier>{A16AD881-5291-4B14-934F-F578E797EF72}</UniqueIdentifier>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\Network\PacketFrame.cpp">
      <Filter>00.Network</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Network\PacketReader.cpp">
      <Filter>00.Network</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Network\PacketStreamParser.cpp">
      <Filter>00.Network</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Network\PacketWriter.cpp">
      <Filter>00.Network</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Network\PacketMessages.cpp">
      <Filter>01.Protocol</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Gameplay\CombatCollisionContract.cpp">
      <Filter>02.Gameplay\00.Combat</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Gameplay\BalanceNumericContract.cpp">
      <Filter>02.Gameplay\04.Balance</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\GameplayDataRevision.cpp">
      <Filter>03.Revision</Filter>
    </ClCompile>
    <ClCompile Include="..\Private\Shared_Pch.cpp">
      <Filter>99.Default</Filter>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\Public\Network\PacketFrame.h">
      <Filter>00.Network</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Network\PacketReader.h">
      <Filter>00.Network</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Network\PacketStreamParser.h">
      <Filter>00.Network</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Network\PacketWriter.h">
      <Filter>00.Network</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Network\SessionDiagnostic.h">
      <Filter>00.Network</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Network\NetworkIds.h">
      <Filter>01.Protocol</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Network\PacketMessages.h">
      <Filter>01.Protocol</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Network\PacketType.h">
      <Filter>01.Protocol</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Gameplay\AttackHitTemplate.h">
      <Filter>02.Gameplay\00.Combat</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Gameplay\CombatCollisionContract.h">
      <Filter>02.Gameplay\00.Combat</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Gameplay\CombatObjectHitChain.h">
      <Filter>02.Gameplay\00.Combat</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Gameplay\EstherStrikeContract.h">
      <Filter>02.Gameplay\00.Combat</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Gameplay\WorldCollisionContract.h">
      <Filter>02.Gameplay\01.World</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Gameplay\KoukuArenaReadyAreas.h">
      <Filter>02.Gameplay\02.KoukuSaydon</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Gameplay\KoukuMarioBombContract.h">
      <Filter>02.Gameplay\02.KoukuSaydon</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Gameplay\KoukuTargetTracking.h">
      <Filter>02.Gameplay\02.KoukuSaydon</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Gameplay\MaharakaWaterpangContract.h">
      <Filter>02.Gameplay\03.Maharaka</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Gameplay\BalanceNumericContract.h">
      <Filter>02.Gameplay\04.Balance</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\GameplayDataRevision.h">
      <Filter>03.Revision</Filter>
    </ClInclude>
    <ClInclude Include="..\..\Tools\Build\CppStandardPch.h">
      <Filter>99.Default</Filter>
    </ClInclude>
  </ItemGroup>
  <ItemGroup>
    <Filter Include="04.Concurrency"><UniqueIdentifier>{0EDC4A82-BFF5-46DC-A67D-519E33CA208B}</UniqueIdentifier></Filter>
    <ClInclude Include="..\Public\Concurrency\ChaseLevDeque.h"><Filter>04.Concurrency</Filter></ClInclude>
    <ClInclude Include="..\Public\Concurrency\WorkStealingJobSystem.h"><Filter>04.Concurrency</Filter></ClInclude>
    <ClCompile Include="..\Private\Concurrency\WorkStealingJobSystem.cpp"><Filter>04.Concurrency</Filter></ClCompile>
  </ItemGroup>
</Project>
~~~~

<a id="file-shared-private-concurrency-workstealingjobsystem-cpp"></a>

### Shared/Private/Concurrency/WorkStealingJobSystem.cpp

파일: C:/Users/tnest/Desktop/LostArk/Shared/Private/Concurrency/WorkStealingJobSystem.cpp

Submit은 수명 검증과 pending 증가 뒤 local deque 또는 external injection에 게시한다. 포화면 lock 밖 caller-runs로 진행한다. Wait는 다른 작업을 도우며 완료 후 첫 예외를 다시 던진다. epoch atomic.wait/notify는 잠자기 직전 게시를 놓치지 않는다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
#include "Concurrency/WorkStealingJobSystem.h"

#include <stdexcept>
#include <utility>

namespace LostArk::Shared::Concurrency
{
namespace
{
    thread_local WorkStealingJobSystem* t_owner = nullptr;
    thread_local std::size_t t_workerIndex = 0;
    thread_local std::size_t t_victimCursor = 0;

    struct ExecutionFrame
    {
        WorkStealingJobSystem* system;
        JobCounter* counter;
        ExecutionFrame* previous;
    };
    thread_local ExecutionFrame* t_execution = nullptr;
}

WorkStealingJobSystem::WorkStealingJobSystem()
    : WorkStealingJobSystem(Config{})
{
}

WorkStealingJobSystem::WorkStealingJobSystem(Config config)
{
    if (config.workerCount == 0)
    {
        const auto hardware = std::thread::hardware_concurrency();
        config.workerCount = hardware > 1 ? hardware - 1 : 1;
    }
    if (config.workerCount > 256 || config.injectionCapacity == 0 ||
        config.injectionCapacity > 1048576)
        throw std::invalid_argument("invalid JobSystem capacity");

    m_injection.resize(config.injectionCapacity, nullptr);
    m_deques.reserve(config.workerCount);
    m_workers.reserve(config.workerCount);
    for (std::size_t i = 0; i < config.workerCount; ++i)
        m_deques.push_back(std::make_unique<Deque>());
    try
    {
        for (std::size_t i = 0; i < config.workerCount; ++i)
            m_workers.emplace_back([this, i] { WorkerMain(i); });
    }
    catch (...)
    {
        m_stopping.store(true, std::memory_order_release);
        SignalProgress();
        for (auto& worker : m_workers)
            if (worker.joinable())
                worker.join();
        throw;
    }
}

WorkStealingJobSystem::~WorkStealingJobSystem()
{
    Shutdown();
}

bool WorkStealingJobSystem::IsExecutingHere() const noexcept
{
    for (auto* frame = t_execution; frame; frame = frame->previous)
        if (frame->system == this)
            return true;
    return false;
}

void WorkStealingJobSystem::Submit(JobCounter& counter, std::function<void()> function)
{
    if (!function)
        throw std::invalid_argument("empty job");
    auto job = std::make_unique<Job>(Job{std::move(function), &counter});
    {
        std::lock_guard lock(m_lifecycleMutex);
        if (!m_accepting && !IsExecutingHere())
            throw std::logic_error("JobSystem is shutting down");
        WorkStealingJobSystem* expected = nullptr;
        if (!counter.m_system.compare_exchange_strong(
                expected, this, std::memory_order_acq_rel, std::memory_order_acquire) &&
            expected != this)
            throw std::invalid_argument("counter belongs to another JobSystem");
        // Count the fully constructed node before any consumer can see it.
        counter.m_pending.fetch_add(1, std::memory_order_release);
        m_outstanding.fetch_add(1, std::memory_order_release);
        m_submitted.fetch_add(1, std::memory_order_relaxed);
    }

    Job* const raw = job.release();
    if (t_owner == this && m_deques[t_workerIndex]->Push(raw))
    {
        m_localPushes.fetch_add(1, std::memory_order_relaxed);
        SignalProgress();
        return;
    }
    if (PushInjection(raw))
    {
        m_injectionPushes.fetch_add(1, std::memory_order_relaxed);
        SignalProgress();
        return;
    }

    // Caller-runs backpressure prevents an accepted nested job from waiting
    // for capacity held by its own ancestors. No queue/lifecycle lock is held.
    m_overflowInline.fetch_add(1, std::memory_order_relaxed);
    Execute(raw);
}

bool WorkStealingJobSystem::PushInjection(Job* job) noexcept
{
    std::lock_guard lock(m_injectionMutex);
    if (m_injectionSize == m_injection.size())
        return false;
    const std::size_t tail = (m_injectionHead + m_injectionSize) % m_injection.size();
    m_injection[tail] = job;
    ++m_injectionSize;
    return true;
}

bool WorkStealingJobSystem::TryInjection(Job*& output) noexcept
{
    std::lock_guard lock(m_injectionMutex);
    if (m_injectionSize == 0)
        return false;
    output = m_injection[m_injectionHead];
    m_injection[m_injectionHead] = nullptr;
    m_injectionHead = (m_injectionHead + 1) % m_injection.size();
    --m_injectionSize;
    return true;
}

bool WorkStealingJobSystem::TryTake(Job*& output) noexcept
{
    if (t_owner == this && m_deques[t_workerIndex]->Pop(output))
        return true;
    if (TryInjection(output))
        return true;

    const std::size_t count = m_deques.size();
    const std::size_t start = t_victimCursor++ % count;
    for (std::size_t attempt = 0; attempt < count; ++attempt)
    {
        const std::size_t victim = (start + attempt) % count;
        if (t_owner == this && victim == t_workerIndex)
            continue;
        if (m_deques[victim]->Steal(output))
        {
            m_steals.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
    }
    return false;
}

void WorkStealingJobSystem::Execute(Job* raw) noexcept
{
    std::unique_ptr<Job> job(raw);
    JobCounter* const counter = job->counter;
    ExecutionFrame frame{this, counter, t_execution};
    t_execution = &frame;
    try
    {
        job->function();
    }
    catch (...)
    {
        std::lock_guard lock(counter->m_failureMutex);
        if (!counter->m_failure)
            counter->m_failure = std::current_exception();
        m_failed.fetch_add(1, std::memory_order_relaxed);
    }
    // Captured state is also destroyed before publishing counter completion.
    job.reset();
    t_execution = frame.previous;
    m_executed.fetch_add(1, std::memory_order_relaxed);
    counter->m_pending.fetch_sub(1, std::memory_order_acq_rel);
    m_outstanding.fetch_sub(1, std::memory_order_acq_rel);
    SignalProgress();
}

void WorkStealingJobSystem::Wait(JobCounter& counter)
{
    const auto system = counter.m_system.load(std::memory_order_acquire);
    if (system && system != this)
        throw std::invalid_argument("counter belongs to another JobSystem");
    for (auto* frame = t_execution; frame; frame = frame->previous)
        if (frame->counter == &counter)
            throw std::logic_error("a job cannot wait for its own batch");

    while (counter.m_pending.load(std::memory_order_acquire) != 0)
    {
        const auto epoch = m_progressEpoch.load(std::memory_order_acquire);
        Job* job = nullptr;
        if (TryTake(job))
        {
            Execute(job);
            continue;
        }
        // The value is sampled before inspecting work. atomic::wait checks it
        // again when parking, so publication between TryTake and wait cannot
        // lose a wakeup. No periodic timeout or condition-variable race window.
        if (counter.m_pending.load(std::memory_order_acquire) != 0)
            m_progressEpoch.wait(epoch, std::memory_order_acquire);
    }
    std::exception_ptr failure;
    {
        std::lock_guard lock(counter.m_failureMutex);
        failure = counter.m_failure;
    }
    if (failure)
        std::rethrow_exception(failure);
}

void WorkStealingJobSystem::WorkerMain(std::size_t index) noexcept
{
    t_owner = this;
    t_workerIndex = index;
    t_victimCursor = index + 1;
    for (;;)
    {
        const auto epoch = m_progressEpoch.load(std::memory_order_acquire);
        Job* job = nullptr;
        if (TryTake(job))
        {
            Execute(job);
            continue;
        }
        if (m_stopping.load(std::memory_order_acquire) &&
            m_outstanding.load(std::memory_order_acquire) == 0)
            break;
        m_progressEpoch.wait(epoch, std::memory_order_acquire);
    }
    t_owner = nullptr;
}

void WorkStealingJobSystem::SignalProgress() noexcept
{
    m_progressEpoch.fetch_add(1, std::memory_order_release);
    m_progressEpoch.notify_all();
}

void WorkStealingJobSystem::Shutdown()
{
    if (t_owner == this || IsExecutingHere())
        throw std::logic_error("Shutdown must run outside this JobSystem's jobs");
    std::lock_guard shutdownLock(m_shutdownMutex);
    if (m_joined)
        return;
    {
        std::lock_guard lock(m_lifecycleMutex);
        m_accepting = false;
        m_stopping.store(true, std::memory_order_release);
    }
    SignalProgress();
    for (auto& worker : m_workers)
        worker.join();
    // Accepted overflow/caller-help execution can live on external threads.
    // Worker exit requires outstanding == 0, so every such job has completed.
    m_joined = true;
}

JobSystemStats WorkStealingJobSystem::GetStats() const noexcept
{
    return {m_submitted.load(std::memory_order_relaxed),
        m_executed.load(std::memory_order_relaxed),
        m_failed.load(std::memory_order_relaxed),
        m_localPushes.load(std::memory_order_relaxed),
        m_injectionPushes.load(std::memory_order_relaxed),
        m_steals.load(std::memory_order_relaxed),
        m_overflowInline.load(std::memory_order_relaxed)};
}
}
~~~~

<a id="file-shared-public-concurrency-chaselevdeque-h"></a>

### Shared/Public/Concurrency/ChaseLevDeque.h

파일: C:/Users/tnest/Desktop/LostArk/Shared/Public/Concurrency/ChaseLevDeque.h

worker 하나만 bottom을 push/pop하고 다른 worker는 top CAS로 훔친다. atomic slot은 재사용 중 speculative read의 data race를 막고, 마지막 원소 CAS는 정확히 한 소비자에게만 소유권을 준다. 고정 capacity 초과는 false로 상위 scheduler에 알린다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace LostArk::Shared::Concurrency
{
// Fixed storage, one owner at bottom, multiple thieves at top. A returned
// pointer/token may be dereferenced only after Pop/Steal reports success.
template <typename T, std::size_t Capacity = 1024>
class ChaseLevDeque final
{
    static_assert(Capacity > 1 && (Capacity & (Capacity - 1)) == 0);
    static_assert(std::is_trivially_copyable_v<T>);

public:
    static constexpr std::size_t kCapacity = Capacity;

    ChaseLevDeque() = default;
    ChaseLevDeque(const ChaseLevDeque&) = delete;
    ChaseLevDeque& operator=(const ChaseLevDeque&) = delete;

    // Only the owning worker may call Push or Pop.
    bool Push(T value) noexcept
    {
        const std::int64_t bottom = m_bottom.load(std::memory_order_relaxed);
        const std::int64_t top = m_top.load(std::memory_order_acquire);
        if (bottom - top >= static_cast<std::int64_t>(Capacity))
            return false;
        m_slots[static_cast<std::size_t>(bottom) & (Capacity - 1)].store(
            value, std::memory_order_relaxed);
        m_bottom.store(bottom + 1, std::memory_order_release);
        return true;
    }

    bool Pop(T& output) noexcept
    {
        const std::int64_t bottom = m_bottom.load(std::memory_order_relaxed) - 1;
        m_bottom.store(bottom, std::memory_order_release);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        std::int64_t top = m_top.load(std::memory_order_relaxed);
        if (top > bottom)
        {
            m_bottom.store(top, std::memory_order_release);
            return false;
        }
        const T value = m_slots[static_cast<std::size_t>(bottom) & (Capacity - 1)].load(
            std::memory_order_acquire);
        if (top != bottom)
        {
            output = value;
            return true;
        }

        // CAS writes back expected on failure. Restore from the original index,
        // not the thief-updated expected value (the last-item race).
        const std::int64_t lastIndex = top;
        const bool acquired = m_top.compare_exchange_strong(
            top, top + 1, std::memory_order_seq_cst, std::memory_order_relaxed);
        m_bottom.store(lastIndex + 1, std::memory_order_release);
        if (acquired)
            output = value;
        return acquired;
    }

    bool Steal(T& output) noexcept
    {
        std::int64_t top = m_top.load(std::memory_order_acquire);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        const std::int64_t bottom = m_bottom.load(std::memory_order_acquire);
        if (top >= bottom)
            return false;
        const T value = m_slots[static_cast<std::size_t>(top) & (Capacity - 1)].load(
            std::memory_order_acquire);
        if (!m_top.compare_exchange_strong(
                top, top + 1, std::memory_order_seq_cst, std::memory_order_relaxed))
            return false;
        output = value;
        return true;
    }

    std::size_t SizeApprox() const noexcept
    {
        const auto bottom = m_bottom.load(std::memory_order_acquire);
        const auto top = m_top.load(std::memory_order_acquire);
        return bottom > top ? static_cast<std::size_t>(bottom - top) : 0;
    }

private:
    alignas(64) std::atomic<std::int64_t> m_bottom{0};
    alignas(64) std::atomic<std::int64_t> m_top{0};
    std::array<std::atomic<T>, Capacity> m_slots{};
};
}
~~~~

<a id="file-shared-public-concurrency-workstealingjobsystem-h"></a>

### Shared/Public/Concurrency/WorkStealingJobSystem.h

파일: C:/Users/tnest/Desktop/LostArk/Shared/Public/Concurrency/WorkStealingJobSystem.h

Config는 worker 수와 외부 injection 용량, JobCounter는 batch의 pending·첫 예외, Stats는 실행·steal·overflow 비용을 설명한다. std::thread는 persistent worker, std::function은 job callable, atomic은 counter와 wake epoch, mutex는 lifecycle/injection/예외를 보호한다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
#pragma once

#include "Concurrency/ChaseLevDeque.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace LostArk::Shared::Concurrency
{
class WorkStealingJobSystem;

// One counter per batch. Its lifetime covers every accepted job and waiter.
// Finish submitting roots before Wait; nested jobs keep their parent counted.
class JobCounter final
{
public:
    JobCounter() = default;
    JobCounter(const JobCounter&) = delete;
    JobCounter& operator=(const JobCounter&) = delete;

    std::uint64_t PendingCount() const noexcept
    {
        return m_pending.load(std::memory_order_acquire);
    }

private:
    friend class WorkStealingJobSystem;
    std::atomic<std::uint64_t> m_pending{0};
    std::atomic<WorkStealingJobSystem*> m_system{nullptr};
    std::mutex m_failureMutex;
    std::exception_ptr m_failure;
};

struct JobSystemStats
{
    std::uint64_t submitted = 0;
    std::uint64_t executed = 0;
    std::uint64_t failed = 0;
    std::uint64_t localPushes = 0;
    std::uint64_t injectionPushes = 0;
    std::uint64_t steals = 0;
    std::uint64_t overflowInline = 0;
};

// Persistent CPU workers. Chase-Lev local deques are bounded and owner-only;
// the bounded external injection ring and lifecycle use mutexes. This whole
// scheduler is not lock-free. Overflow executes on the submitting stack.
// Wait helps execute jobs, so callers must not hold a lock needed by a job.
// No member calls may race destruction; Shutdown itself may race Submit/Wait.
class WorkStealingJobSystem final
{
public:
    struct Config
    {
        std::size_t workerCount = 0; // 0 selects max(1, hardware_concurrency - 1).
        std::size_t injectionCapacity = 4096;
    };

    WorkStealingJobSystem();
    explicit WorkStealingJobSystem(Config config);
    ~WorkStealingJobSystem();
    WorkStealingJobSystem(const WorkStealingJobSystem&) = delete;
    WorkStealingJobSystem& operator=(const WorkStealingJobSystem&) = delete;

    void Submit(JobCounter& counter, std::function<void()> function);
    // Drains this counter, then rethrows its first job exception, if any.
    void Wait(JobCounter& counter);
    // Rejects new external submissions, drains accepted work (including its
    // nested jobs), and joins workers. Calling from one of its jobs is invalid.
    void Shutdown();
    std::size_t GetWorkerCount() const noexcept { return m_deques.size(); }
    JobSystemStats GetStats() const noexcept;

private:
    struct Job
    {
        std::function<void()> function;
        JobCounter* counter;
    };
    using Deque = ChaseLevDeque<Job*>;

    bool TryTake(Job*& output) noexcept;
    bool TryInjection(Job*& output) noexcept;
    bool PushInjection(Job* job) noexcept;
    void Execute(Job* job) noexcept;
    void WorkerMain(std::size_t index) noexcept;
    void SignalProgress() noexcept;
    bool IsExecutingHere() const noexcept;

    std::vector<std::unique_ptr<Deque>> m_deques;
    std::vector<std::thread> m_workers;
    std::vector<Job*> m_injection;
    std::size_t m_injectionHead = 0;
    std::size_t m_injectionSize = 0;
    std::mutex m_injectionMutex;

    std::mutex m_lifecycleMutex;
    std::mutex m_shutdownMutex;
    bool m_accepting = true;
    bool m_joined = false;
    std::atomic<bool> m_stopping{false};
    std::atomic<std::uint64_t> m_outstanding{0};
    std::atomic<std::uint64_t> m_progressEpoch{0};

    std::atomic<std::uint64_t> m_submitted{0};
    std::atomic<std::uint64_t> m_executed{0};
    std::atomic<std::uint64_t> m_failed{0};
    std::atomic<std::uint64_t> m_localPushes{0};
    std::atomic<std::uint64_t> m_injectionPushes{0};
    std::atomic<std::uint64_t> m_steals{0};
    std::atomic<std::uint64_t> m_overflowInline{0};
};
}
~~~~

<a id="file-tools-serverconcurrencyharness-comparisonprofiles-json"></a>

### Tools/ServerConcurrencyHarness/ComparisonProfiles.json

파일: C:/Users/tnest/Desktop/LostArk/Tools/ServerConcurrencyHarness/ComparisonProfiles.json

비교 조건·실행·결과 해석의 정본이다. 자동 실행은 소유한 격리 프로세스만 시작하고 종료하며 원시 결과를 out에 남긴다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~json
{
  "formatVersion": 1,
  "profiles": {
    "quick": {
      "connections": [4, 32],
      "iocpWorkers": 2,
      "jobWorkers": 2,
      "transportWarmupMs": 500,
      "transportDurationMs": 1000,
      "abbaRepeats": 1,
      "fanoutPayloadBytes": [1024],
      "fanoutWarmupBatches": 30,
      "fanoutMeasuredBatches": 200
    },
    "portfolio": {
      "connections": [4, 32, 128],
      "iocpWorkers": 2,
      "jobWorkers": 2,
      "transportWarmupMs": 1000,
      "transportDurationMs": 3000,
      "abbaRepeats": 2,
      "fanoutPayloadBytes": [1024, 16384],
      "fanoutWarmupBatches": 100,
      "fanoutMeasuredBatches": 1000
    }
  }
}
~~~~

<a id="file-tools-serverconcurrencyharness-default-serverconcurrencyharness-vcxproj"></a>

### Tools/ServerConcurrencyHarness/Default/ServerConcurrencyHarness.vcxproj

파일: C:/Users/tnest/Desktop/LostArk/Tools/ServerConcurrencyHarness/Default/ServerConcurrencyHarness.vcxproj

물리 소스에 대응하는 build/IDE 항목이다. 기존 filter를 재배치하지 않고 필요한 항목만 등록한다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~xml
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64"><Configuration>Debug</Configuration><Platform>x64</Platform></ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64"><Configuration>Release</Configuration><Platform>x64</Platform></ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals"><ProjectGuid>{7D19AF23-E004-4C34-9AAA-C730F114848A}</ProjectGuid><Keyword>Win32Proj</Keyword><WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion></PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Label="Configuration"><ConfigurationType>Application</ConfigurationType><PlatformToolset>v143</PlatformToolset><CharacterSet>Unicode</CharacterSet><UseDebugLibraries Condition="'$(Configuration)'=='Debug'">true</UseDebugLibraries></PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <PropertyGroup>
    <RepositoryRoot Condition="'$(RepositoryRoot)'==''">$([System.IO.Path]::GetFullPath('$(MSBuildThisFileDirectory)..\..\..\'))</RepositoryRoot>
    <ServerSourceRoot Condition="'$(ServerSourceRoot)'==''">$(RepositoryRoot)</ServerSourceRoot>
    <OutDir Condition="'$(HarnessOutput)'==''">$(RepositoryRoot)out\ServerConcurrencyHarness\$(Configuration)\</OutDir>
    <OutDir Condition="'$(HarnessOutput)'!=''">$(HarnessOutput)\</OutDir>
    <IntDir>$(OutDir)obj\</IntDir><TargetName>ServerConcurrencyHarness</TargetName>
  </PropertyGroup>
  <ItemDefinitionGroup>
    <ClCompile><WarningLevel>Level4</WarningLevel><SDLCheck>true</SDLCheck><ConformanceMode>true</ConformanceMode><LanguageStandard>stdcpp20</LanguageStandard><PrecompiledHeader>NotUsing</PrecompiledHeader><AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions><PreprocessorDefinitions>LOSTARK_FANOUT_HARNESS;WIN32_LEAN_AND_MEAN;NOMINMAX;_WIN32_WINNT=0x0A00;%(PreprocessorDefinitions)</PreprocessorDefinitions><AdditionalIncludeDirectories>$(ServerSourceRoot)Server\Public;$(ServerSourceRoot)Shared\Public;$(RepositoryRoot)Server\Public;$(RepositoryRoot)Shared\Public;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories><Optimization Condition="'$(Configuration)'=='Release'">MaxSpeed</Optimization><RuntimeLibrary Condition="'$(Configuration)'=='Release'">MultiThreadedDLL</RuntimeLibrary><RuntimeLibrary Condition="'$(Configuration)'=='Debug'">MultiThreadedDebugDLL</RuntimeLibrary><DebugInformationFormat>ProgramDatabase</DebugInformationFormat></ClCompile>
    <Link><SubSystem>Console</SubSystem><AdditionalDependencies>Ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies><GenerateDebugInformation>true</GenerateDebugInformation></Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\Main.cpp" />
    <ClCompile Include="..\Private\SnapshotFanoutBenchmark.cpp" />
    <ClCompile Include="..\Private\JobSystemTests.cpp" />
    <ClCompile Include="$(ServerSourceRoot)Server\Private\ClientSession.cpp" />
    <ClCompile Include="$(ServerSourceRoot)Server\Private\ClientSession_Iocp.cpp" />
    <ClCompile Include="$(ServerSourceRoot)Server\Private\IocpService.cpp" />
    <ClCompile Include="$(ServerSourceRoot)Server\Private\ServerSnapshotFanout.cpp" />
    <ClCompile Include="$(ServerSourceRoot)Shared\Private\Concurrency\WorkStealingJobSystem.cpp" />
    <ClCompile Include="$(RepositoryRoot)Shared\Private\Network\PacketFrame.cpp" />
    <ClCompile Include="$(RepositoryRoot)Shared\Private\Network\PacketStreamParser.cpp" />
    <ClCompile Include="$(RepositoryRoot)Shared\Private\Network\PacketReader.cpp" />
    <ClCompile Include="$(RepositoryRoot)Shared\Private\Network\PacketWriter.cpp" />
  </ItemGroup>
  <ItemGroup><ClInclude Include="$(ServerSourceRoot)Server\Public\ClientSession.h" /><ClInclude Include="$(ServerSourceRoot)Server\Public\IocpService.h" /></ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
~~~~

<a id="file-tools-serverconcurrencyharness-default-serverconcurrencyharness-vcxproj-filters"></a>

### Tools/ServerConcurrencyHarness/Default/ServerConcurrencyHarness.vcxproj.filters

파일: C:/Users/tnest/Desktop/LostArk/Tools/ServerConcurrencyHarness/Default/ServerConcurrencyHarness.vcxproj.filters

물리 소스에 대응하는 build/IDE 항목이다. 기존 filter를 재배치하지 않고 필요한 항목만 등록한다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup><Filter Include="Harness"><UniqueIdentifier>{63A0168B-09BA-44FA-8525-232BE7ADFD0B}</UniqueIdentifier></Filter><Filter Include="Production"><UniqueIdentifier>{263B775F-64E1-4557-AEA2-117130D9EF21}</UniqueIdentifier></Filter></ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\Private\Main.cpp"><Filter>Harness</Filter></ClCompile>
    <ClCompile Include="..\Private\SnapshotFanoutBenchmark.cpp"><Filter>Harness</Filter></ClCompile>
    <ClCompile Include="..\Private\JobSystemTests.cpp"><Filter>Harness</Filter></ClCompile>
    <ClCompile Include="$(ServerSourceRoot)Server\Private\ClientSession.cpp"><Filter>Production</Filter></ClCompile>
    <ClCompile Include="$(ServerSourceRoot)Server\Private\ClientSession_Iocp.cpp"><Filter>Production</Filter></ClCompile>
    <ClCompile Include="$(ServerSourceRoot)Server\Private\IocpService.cpp"><Filter>Production</Filter></ClCompile>
    <ClCompile Include="$(ServerSourceRoot)Server\Private\ServerSnapshotFanout.cpp"><Filter>Production</Filter></ClCompile>
    <ClCompile Include="$(ServerSourceRoot)Shared\Private\Concurrency\WorkStealingJobSystem.cpp"><Filter>Production</Filter></ClCompile>
    <ClCompile Include="$(RepositoryRoot)Shared\Private\Network\PacketFrame.cpp"><Filter>Production</Filter></ClCompile>
    <ClCompile Include="$(RepositoryRoot)Shared\Private\Network\PacketStreamParser.cpp"><Filter>Production</Filter></ClCompile>
    <ClCompile Include="$(RepositoryRoot)Shared\Private\Network\PacketReader.cpp"><Filter>Production</Filter></ClCompile>
    <ClCompile Include="$(RepositoryRoot)Shared\Private\Network\PacketWriter.cpp"><Filter>Production</Filter></ClCompile>
  </ItemGroup>
  <ItemGroup><ClInclude Include="$(ServerSourceRoot)Server\Public\ClientSession.h"><Filter>Production</Filter></ClInclude><ClInclude Include="$(ServerSourceRoot)Server\Public\IocpService.h"><Filter>Production</Filter></ClInclude></ItemGroup>
</Project>
~~~~

<a id="file-tools-serverconcurrencyharness-private-jobsystemtests-cpp"></a>

### Tools/ServerConcurrencyHarness/Private/JobSystemTests.cpp

파일: C:/Users/tnest/Desktop/LostArk/Tools/ServerConcurrencyHarness/Private/JobSystemTests.cpp

deque 최후 원소 경합, exactly-once, 중첩 대기, external/local overflow, exception, shutdown 수명을 실행 검사한다. 성공 횟수와 성능 향상은 별개다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
#include "Concurrency/ChaseLevDeque.h"
#include "Concurrency/WorkStealingJobSystem.h"

#include <array>
#include <atomic>
#include <barrier>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

namespace
{
using LostArk::Shared::Concurrency::ChaseLevDeque;
using LostArk::Shared::Concurrency::JobCounter;
using LostArk::Shared::Concurrency::WorkStealingJobSystem;

void Require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

bool AwaitFlag(const std::atomic<bool>& flag)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!flag.load(std::memory_order_acquire))
    {
        if (std::chrono::steady_clock::now() >= deadline)
            return false;
        std::this_thread::yield();
    }
    return true;
}

void TestDequeBoundedAndLastItem()
{
    ChaseLevDeque<std::uintptr_t, 64> deque;
    for (std::uintptr_t i = 1; i <= 64; ++i)
        Require(deque.Push(i), "deque rejected available capacity");
    Require(!deque.Push(65), "deque exceeded fixed capacity");
    for (std::uintptr_t i = 64; i != 0; --i)
    {
        std::uintptr_t value = 0;
        Require(deque.Pop(value) && value == i, "owner ordering failure");
    }

    constexpr std::size_t iterations = 20000;
    std::barrier gate(2);
    bool thiefAcquired = false;
    std::uintptr_t thiefValue = 0;
    std::thread thief([&]
    {
        for (std::size_t i = 0; i < iterations; ++i)
        {
            gate.arrive_and_wait();
            thiefAcquired = deque.Steal(thiefValue);
            gate.arrive_and_wait();
        }
    });
    std::size_t violations = 0;
    std::size_t ownerWins = 0;
    std::size_t thiefWins = 0;
    for (std::size_t i = 0; i < iterations; ++i)
    {
        const auto expected = static_cast<std::uintptr_t>(i + 1);
        if (!deque.Push(expected))
            ++violations;
        gate.arrive_and_wait();
        std::uintptr_t ownerValue = 0;
        const bool ownerAcquired = deque.Pop(ownerValue);
        gate.arrive_and_wait();
        ownerWins += ownerAcquired ? 1 : 0;
        thiefWins += thiefAcquired ? 1 : 0;
        if (ownerAcquired == thiefAcquired ||
            (ownerAcquired && ownerValue != expected) ||
            (thiefAcquired && thiefValue != expected) || deque.SizeApprox() != 0)
            ++violations;
    }
    thief.join();
    Require(violations == 0, "deque last-item ownership/value/size violation");
    std::cout << "job_test,last_item,iterations=" << iterations
        << ",owner_wins=" << ownerWins << ",thief_wins=" << thiefWins
        << ",violations=" << violations << '\n';
}

void TestExactlyOnce()
{
    WorkStealingJobSystem jobs({4, 64});
    constexpr std::size_t count = 10000;
    std::vector<std::atomic<unsigned>> hits(count);
    for (auto& value : hits)
        value.store(0, std::memory_order_relaxed);
    JobCounter counter;
    for (std::size_t i = 0; i < count; ++i)
        jobs.Submit(counter, [&, i] { hits[i].fetch_add(1, std::memory_order_relaxed); });
    jobs.Wait(counter);
    for (const auto& value : hits)
        Require(value.load() == 1, "job duplicated or omitted");
    jobs.Shutdown();
    const auto stats = jobs.GetStats();
    Require(stats.submitted == count && stats.executed == count && stats.failed == 0,
        "exactly-once counters disagree");
}

void TestExternalNestedWaits()
{
    WorkStealingJobSystem jobs({4, 32});
    JobCounter parents;
    std::atomic<unsigned> completed{0};
    std::vector<std::thread> producers;
    for (unsigned producer = 0; producer < 4; ++producer)
    {
        producers.emplace_back([&]
        {
            for (unsigned parent = 0; parent < 60; ++parent)
            {
                jobs.Submit(parents, [&]
                {
                    JobCounter children;
                    for (unsigned child = 0; child < 12; ++child)
                        jobs.Submit(children, [&] { completed.fetch_add(1); });
                    jobs.Wait(children);
                });
            }
        });
    }
    for (auto& producer : producers)
        producer.join();
    jobs.Wait(parents);
    jobs.Shutdown();
    Require(completed.load() == 4 * 60 * 12, "external nested submission lost work");
    const auto stats = jobs.GetStats();
    Require(stats.submitted == stats.executed && stats.failed == 0,
        "nested wait counters disagree");
}

void TestOverflowCallerRuns()
{
    WorkStealingJobSystem jobs({1, 1});
    JobCounter counter;
    std::atomic<bool> entered{false};
    std::atomic<bool> release{false};
    std::atomic<unsigned> completed{0};
    jobs.Submit(counter, [&]
    {
        entered.store(true, std::memory_order_release);
        while (!release.load(std::memory_order_acquire))
            std::this_thread::yield();
        completed.fetch_add(1);
    });
    if (!AwaitFlag(entered))
    {
        release.store(true, std::memory_order_release);
        jobs.Wait(counter);
        throw std::runtime_error("overflow blocker did not start");
    }
    jobs.Submit(counter, [&] { completed.fetch_add(1); });
    const auto submitter = std::this_thread::get_id();
    bool ranOnSubmitter = false;
    jobs.Submit(counter, [&]
    {
        ranOnSubmitter = std::this_thread::get_id() == submitter;
        completed.fetch_add(1);
    });
    release.store(true, std::memory_order_release);
    jobs.Wait(counter);
    jobs.Shutdown();
    Require(ranOnSubmitter && completed.load() == 3 &&
        jobs.GetStats().overflowInline == 1, "bounded overflow policy failed");
}

void TestExceptionsAndInvalidDependencies()
{
    WorkStealingJobSystem jobs({2, 8});
    JobCounter counter;
    std::atomic<unsigned> successful{0};
    for (unsigned i = 0; i < 50; ++i)
        jobs.Submit(counter, [&, i]
        {
            if (i % 5 == 0)
                throw std::runtime_error("intentional test failure");
            successful.fetch_add(1);
        });
    bool propagated = false;
    try { jobs.Wait(counter); }
    catch (const std::runtime_error&) { propagated = true; }
    Require(propagated && successful.load() == 40 && counter.PendingCount() == 0,
        "exception did not drain and propagate");

    JobCounter invalid;
    jobs.Submit(invalid, [&] { jobs.Wait(invalid); });
    propagated = false;
    try { jobs.Wait(invalid); }
    catch (const std::logic_error&) { propagated = true; }
    Require(propagated, "self-dependency was not rejected");

    JobCounter invalidShutdown;
    jobs.Submit(invalidShutdown, [&] { jobs.Shutdown(); });
    propagated = false;
    try { jobs.Wait(invalidShutdown); }
    catch (const std::logic_error&) { propagated = true; }
    Require(propagated, "job-side Shutdown was not rejected");

    JobCounter survivor;
    jobs.Submit(survivor, [&] { successful.fetch_add(1); });
    jobs.Wait(survivor);
    jobs.Shutdown();
    Require(successful.load() == 41 && jobs.GetStats().failed == 12,
        "workers did not survive job exceptions");
}

void TestLocalDequeOverflow()
{
    WorkStealingJobSystem jobs({1, 1});
    JobCounter parent;
    std::atomic<bool> published{false};
    std::atomic<unsigned> completed{0};
    jobs.Submit(parent, [&]
    {
        JobCounter children;
        for (unsigned i = 0; i < 1500; ++i)
            jobs.Submit(children, [&] { completed.fetch_add(1); });
        // Before external help starts, the sole owner has filled its local
        // deque and injection ring, then run overflowing children inline.
        published.store(true, std::memory_order_release);
        jobs.Wait(children);
    });
    const bool publishedInTime = AwaitFlag(published);
    jobs.Wait(parent);
    jobs.Shutdown();
    const auto stats = jobs.GetStats();
    Require(publishedInTime && completed.load() == 1500 &&
        stats.localPushes == 1024 && stats.overflowInline == 475,
        "local deque overflow did not reach bounded injection/inline fallback");
}

void TestForeignPoolAndShutdownDrain()
{
    WorkStealingJobSystem first({2, 16});
    WorkStealingJobSystem second({2, 16});
    JobCounter roots;
    std::atomic<unsigned> completed{0};
    for (unsigned i = 0; i < 100; ++i)
        first.Submit(roots, [&]
        {
            JobCounter foreign;
            second.Submit(foreign, [&] { completed.fetch_add(1); });
            second.Wait(foreign);
        });
    first.Wait(roots);
    first.Shutdown();
    second.Shutdown();
    Require(completed.load() == 100, "foreign worker submission failed");

    WorkStealingJobSystem draining({2, 256});
    JobCounter batch;
    std::atomic<bool> startChildren{false};
    std::atomic<bool> parentStarted{false};
    draining.Submit(batch, [&]
    {
        parentStarted.store(true, std::memory_order_release);
        while (!startChildren.load(std::memory_order_acquire))
            std::this_thread::yield();
        JobCounter children;
        for (unsigned i = 0; i < 100; ++i)
            draining.Submit(children, [&] { completed.fetch_add(1); });
        draining.Wait(children);
    });
    if (!AwaitFlag(parentStarted))
    {
        startChildren.store(true, std::memory_order_release);
        draining.Wait(batch);
        throw std::runtime_error("shutdown parent did not start");
    }
    std::thread shutdown([&] { draining.Shutdown(); });
    // Observe rejection to establish that Shutdown closed external admission;
    // this is a synchronization check, not an elapsed-time assumption.
    bool rejected = false;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    JobCounter probes;
    while (!rejected && std::chrono::steady_clock::now() < deadline)
    {
        try { draining.Submit(probes, [] {}); }
        catch (const std::logic_error&) { rejected = true; }
        std::this_thread::yield();
    }
    startChildren.store(true, std::memory_order_release);
    shutdown.join();
    draining.Wait(batch);
    draining.Wait(probes);
    Require(rejected && completed.load() == 200,
        "Shutdown failed admission close or accepted nested drain");
    const auto stats = draining.GetStats();
    Require(stats.executed == stats.submitted && stats.failed == 0,
        "Shutdown returned before all jobs completed");
}
}

// Link this function into the native server concurrency harness. It creates
// no sockets, game windows, files, or gameplay state.
bool RunJobSystemTests()
{
    try
    {
        TestDequeBoundedAndLastItem();
        TestExactlyOnce();
        TestExternalNestedWaits();
        TestOverflowCallerRuns();
        TestLocalDequeOverflow();
        TestExceptionsAndInvalidDependencies();
        TestForeignPoolAndShutdownDrain();
        std::cout << "job_tests,PASS\n";
        return true;
    }
    catch (const std::exception& error)
    {
        std::cerr << "job_tests,FAIL," << error.what() << '\n';
        return false;
    }
}
~~~~

<a id="file-tools-serverconcurrencyharness-private-main-cpp"></a>

### Tools/ServerConcurrencyHarness/Private/Main.cpp

파일: C:/Users/tnest/Desktop/LostArk/Tools/ServerConcurrencyHarness/Private/Main.cpp

별도 server/client 프로세스의 loopback 실제 프레임·검증·계측 실행기다. --job-tests와 --fanout은 해당 검증 함수로 연결한다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
// Isolated loopback transport harness. Never starts Client.exe or a product world.
#include "ClientSession.h"
#include "IocpService.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include <Windows.h>
#include <WS2tcpip.h>
#include <TlHelp32.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <barrier>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

using namespace LostArk::Server;
using namespace LostArk::Shared;
using Clock = std::chrono::steady_clock;
namespace fs = std::filesystem;
#ifdef LOSTARK_FANOUT_HARNESS
int RunSnapshotFanoutBenchmark(int argc, char** argv);
bool RunJobSystemTests();
#endif
namespace {
struct Options {
    std::string mode = "server", backend = "select", scenario = "benchmark";
    fs::path directory;
    unsigned connections = 4, payload = 256, window = 8, warmupMs = 1000, durationMs = 3000, workers = 4;
    unsigned port = 0;
};
Options Parse(int argc, char** argv) {
    Options o;
    for (int i = 1; i < argc; ++i) {
        std::string key = argv[i];
        if (i + 1 >= argc) throw std::runtime_error("missing option value");
        std::string value = argv[++i];
        if (key == "--mode") o.mode = value;
        else if (key == "--backend") o.backend = value;
        else if (key == "--scenario") o.scenario = value;
        else if (key == "--directory") o.directory = fs::path(std::u8string(value.begin(), value.end()));
        else if (key == "--connections") o.connections = std::stoul(value);
        else if (key == "--payload") o.payload = std::stoul(value);
        else if (key == "--window") o.window = std::stoul(value);
        else if (key == "--warmup-ms") o.warmupMs = std::stoul(value);
        else if (key == "--duration-ms") o.durationMs = std::stoul(value);
        else if (key == "--workers") o.workers = std::stoul(value);
        else if (key == "--port") o.port = std::stoul(value);
        else throw std::runtime_error("unknown option");
    }
    if (o.directory.empty() || o.connections == 0 || o.connections > 256 || o.payload < 8 ||
        o.payload > MAX_PACKET_BYTES - PACKET_HEADER_BYTES || o.window == 0 || o.window > 64 ||
        o.durationMs > 15000 || o.warmupMs > 5000 || o.workers == 0 || o.workers > 16 ||
        (o.backend != "select" && o.backend != "iocp")) throw std::runtime_error("invalid harness options");
    return o;
}
void Write(const fs::path& path, const std::string& value) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out || !(out << value)) throw std::runtime_error("result write failed");
}
struct Winsock {
    Winsock() { WSADATA d{}; if (WSAStartup(MAKEWORD(2, 2), &d)) throw std::runtime_error("WSAStartup"); }
    ~Winsock() { WSACleanup(); }
};
struct Socket {
    SOCKET value = INVALID_SOCKET;
    Socket() = default;
    explicit Socket(SOCKET s) : value(s) {}
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& r) noexcept : value(std::exchange(r.value, INVALID_SOCKET)) {}
    ~Socket() { if (value != INVALID_SOCKET) closesocket(value); }
};
double Milliseconds(Clock::duration d) { return std::chrono::duration<double, std::milli>(d).count(); }
double ProcessCpuMs() {
    FILETIME c{}, e{}, k{}, u{};
    if (!GetProcessTimes(GetCurrentProcess(), &c, &e, &k, &u)) throw std::runtime_error("GetProcessTimes");
    ULARGE_INTEGER a{}, b{}; a.LowPart = k.dwLowDateTime; a.HighPart = k.dwHighDateTime;
    b.LowPart = u.dwLowDateTime; b.HighPart = u.dwHighDateTime;
    return static_cast<double>(a.QuadPart + b.QuadPart) / 10000.;
}
unsigned ThreadCount() {
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (h == INVALID_HANDLE_VALUE) return 0;
    THREADENTRY32 e{}; e.dwSize = sizeof(e); unsigned count = 0;
    if (Thread32First(h, &e)) do { if (e.th32OwnerProcessID == GetCurrentProcessId()) ++count; } while (Thread32Next(h, &e));
    CloseHandle(h); return count;
}
void Nonblocking(SOCKET s) { u_long v = 1; if (ioctlsocket(s, FIONBIO, &v)) throw std::runtime_error("nonblocking"); }
bool Ready(SOCKET s, bool writing, Clock::time_point deadline) {
    while (Clock::now() < deadline) {
        fd_set set; FD_ZERO(&set); FD_SET(s, &set); timeval t{0, 20000};
        int n = select(0, writing ? nullptr : &set, writing ? &set : nullptr, nullptr, &t);
        if (n > 0) return true;
        if (n < 0) return false;
    }
    return false;
}
Socket Connect(unsigned port) {
    Socket s(WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED));
    if (s.value == INVALID_SOCKET) throw std::runtime_error("socket");
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK); a.sin_port = htons(static_cast<u_short>(port));
    if (connect(s.value, reinterpret_cast<const sockaddr*>(&a), sizeof(a))) throw std::runtime_error("connect");
    BOOL yes = TRUE; setsockopt(s.value, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&yes), sizeof(yes));
    Nonblocking(s.value); return s;
}
bool SendBytes(SOCKET s, std::span<const std::uint8_t> data, size_t chunk = SIZE_MAX) {
    const auto deadline = Clock::now() + std::chrono::seconds(5);
    while (!data.empty()) {
        const int n = send(s, reinterpret_cast<const char*>(data.data()), static_cast<int>((std::min)(data.size(), chunk)), 0);
        if (n > 0) { data = data.subspan(n); continue; }
        if (n == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK && Ready(s, true, deadline)) continue;
        return false;
    }
    return true;
}
bool Receive(SOCKET s, CPacketStreamParser& parser, PACKET_FRAME& frame) {
    const auto deadline = Clock::now() + std::chrono::seconds(5);
    for (;;) {
        auto p = parser.Try_Pop(frame);
        if (p == PACKET_PARSE_RESULT::FRAME_READY) return true;
        if (p == PACKET_PARSE_RESULT::INVALID_FRAME || !Ready(s, false, deadline)) return false;
        std::array<std::uint8_t, 65536> bytes{};
        int n = recv(s, reinterpret_cast<char*>(bytes.data()), static_cast<int>(bytes.size()), 0);
        if (n <= 0) { if (n < 0 && WSAGetLastError() == WSAEWOULDBLOCK) continue; return false; }
        if (!parser.Append({bytes.data(), static_cast<size_t>(n)})) return false;
    }
}
std::vector<std::uint8_t> Payload(std::uint32_t id, std::uint32_t sequence, unsigned size) {
    // Shared's real writer and frame codec; payload is explicitly a synthetic benchmark schema.
    CPacketWriter w; w.Write_U32(id); w.Write_U32(sequence);
    for (unsigned i = 8; i < size; ++i) w.Write_U8(static_cast<std::uint8_t>((i * 31u + id * 17u + sequence) & 255u));
    return w.Get_Buffer();
}
std::vector<std::uint8_t> Frame(std::uint32_t id, std::uint32_t sequence, unsigned size) {
    auto payload = Payload(id, sequence, size); std::vector<std::uint8_t> bytes;
    if (!Build_Packet_Frame(PACKET_TYPE::C2S_ROOM_PING, payload, bytes)) throw std::runtime_error("frame encoding");
    return bytes;
}
std::uint64_t Hash(std::span<const std::uint8_t> bytes) {
    std::uint64_t h = 1469598103934665603ull;
    for (auto b : bytes) { h ^= b; h *= 1099511628211ull; }
    return h;
}
bool Valid(const PACKET_FRAME& frame, std::uint32_t id, std::uint32_t sequence, unsigned size) {
    if (frame.ePacketType != PACKET_TYPE::S2C_ROOM_PING || frame.Payload.size() != size) return false;
    CPacketReader reader(frame.Payload); std::uint32_t gotId{}, gotSeq{};
    if (!reader.Read_U32(gotId) || !reader.Read_U32(gotSeq) || gotId != id || gotSeq != sequence) return false;
    for (unsigned i = 8; i < size; ++i) if (frame.Payload[i] != static_cast<std::uint8_t>((i * 31u + id * 17u + sequence) & 255u)) return false;
    return true;
}

int Server(const Options& o) {
    CIocpService service;
    if (o.backend == "iocp" && !service.Start(o.workers)) throw std::runtime_error("IOCP start");
    Socket listener(WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED));
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (listener.value == INVALID_SOCKET || bind(listener.value, reinterpret_cast<sockaddr*>(&a), sizeof(a)) || listen(listener.value, SOMAXCONN)) throw std::runtime_error("loopback listener");
    int length = sizeof(a); if (getsockname(listener.value, reinterpret_cast<sockaddr*>(&a), &length)) throw std::runtime_error("getsockname");
    Nonblocking(listener.value);
    std::atomic_uint64_t received{0}, echoed{0}, checksum{0}, failures{0}, closed{0};
    std::vector<std::shared_ptr<CClientSession>> sessions;
    Write(o.directory / "ready.txt", std::to_string(ntohs(a.sin_port)) + "\n" + std::to_string(GetCurrentProcessId()) + "\n");
    const auto hardEnd = Clock::now() + std::chrono::seconds(75);
    bool measuring = false, measured = false; double cpuStart = 0., cpuEnd = 0.;
    Clock::time_point start{}, end{}; unsigned peakThreads = ThreadCount();
    std::uint64_t beginFrames = 0, endFrames = 0;
    auto nextThreadSample = Clock::now();
    while (Clock::now() < hardEnd && !fs::exists(o.directory / "server.stop")) {
        if (!measuring && !measured && fs::exists(o.directory / "measure.start")) {
            cpuStart = ProcessCpuMs(); start = Clock::now(); beginFrames = echoed.load(); measuring = true;
            Write(o.directory / "measure.ready", "ready\n");
        }
        if (measuring && fs::exists(o.directory / "measure.stop")) {
            end = Clock::now(); cpuEnd = ProcessCpuMs(); endFrames = echoed.load(); measuring = false; measured = true;
        }
        if (Clock::now() >= nextThreadSample) { peakThreads = (std::max)(peakThreads, ThreadCount()); nextThreadSample = Clock::now() + std::chrono::milliseconds(20); }
        SOCKET accepted = accept(listener.value, nullptr, nullptr);
        if (accepted != INVALID_SOCKET) {
            if (o.scenario == "correctness") { int small = 1024; setsockopt(accepted, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&small), sizeof(small)); }
            auto weak = std::make_shared<std::weak_ptr<CClientSession>>();
            auto session = std::make_shared<CClientSession>(sessions.size() + 1u, accepted,
                [&, weak](SESSION_ID, const PACKET_FRAME& frame) {
                    ++received; checksum.fetch_add(Hash(frame.Payload));
                    auto owner = weak->lock();
                    if (owner && owner->Send_Frame(PACKET_TYPE::S2C_ROOM_PING, frame.Payload)) ++echoed;
                    else ++failures;
                }, [&](SESSION_ID) { ++closed; },
                o.backend == "iocp" ? SESSION_TRANSPORT_BACKEND::IOCP : SESSION_TRANSPORT_BACKEND::SELECT_THREADS,
                o.backend == "iocp" ? &service : nullptr);
            *weak = session;
            if (!session->Start()) throw std::runtime_error("session start");
            sessions.push_back(std::move(session));
        } else if (WSAGetLastError() != WSAEWOULDBLOCK) throw std::runtime_error("accept");
        else std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (measuring) { end = Clock::now(); cpuEnd = ProcessCpuMs(); endFrames = echoed.load(); measured = true; }
    const auto stopBegin = Clock::now();
    // Issue every close before joining any session, including active receive/send operations.
    for (auto& s : sessions) s->Request_Close();
    for (auto& s : sessions) s->Stop();
    const auto acceptedCount = sessions.size();
    std::uint64_t sent = 0, rejected = 0, sendErrors = 0;
    for (const auto& s : sessions) { const auto m = s->Get_OutboundMetrics(); sent += m.iSentFrameCount; rejected += m.iReliableRejectedFrameCount; sendErrors += m.iSendFailureCount; }
    sessions.clear();
    if (o.backend == "iocp") service.Stop();
    const auto iocp = service.Get_Metrics();
    std::ostringstream json;
    json << "{\"backend\":\"" << o.backend << "\",\"scope\":\"isolated transport echo; server process only\","
         << "\"pid\":" << GetCurrentProcessId() << ",\"peak_threads_sampled\":" << peakThreads
         << ",\"thread_sample_ms\":20,\"server_cpu_ms\":" << (measured ? cpuEnd - cpuStart : 0.)
         << ",\"server_measure_ms\":" << (measured ? Milliseconds(end - start) : 0.)
         << ",\"server_measured_echo_enqueues\":" << endFrames - beginFrames
         << ",\"received_frames_total\":" << received.load() << ",\"echo_enqueued_total\":" << echoed.load()
         << ",\"sent_frames_total\":" << sent << ",\"payload_checksum_sum_total\":\"" << checksum.load() << "\""
         << ",\"echo_enqueue_failures_total\":" << failures.load() << ",\"reliable_rejections_total\":" << rejected
         << ",\"send_failures_total\":" << sendErrors << ",\"closed_callbacks\":" << closed.load()
         << ",\"accepted_session_count\":" << acceptedCount
         << ",\"shutdown_ms\":" << Milliseconds(Clock::now() - stopBegin)
         << ",\"iocp_pending_after_stop\":" << iocp.iPendingOperations
         << ",\"iocp_posted_operations\":" << iocp.iPostedOperations << ",\"iocp_completed_operations\":" << iocp.iCompletedOperations
         << ",\"iocp_partial_send_completions\":" << iocp.iPartialSendCompletions
         << ",\"partial_send_proven\":" << (iocp.iPartialSendCompletions ? "true" : "false")
         << ",\"partial_send_note\":\"slow-reader large FIFO uses actual socket pressure; true only when actual IOCP partial completions were observed\"}\n";
    Write(o.directory / "server.json", json.str()); return 0;
}

struct LaneStats { std::vector<double> latency; std::uint64_t measured = 0, warmup = 0, checksum = 0; unsigned errors = 0; };
bool Batch(SOCKET socket, CPacketStreamParser& parser, unsigned id, std::uint32_t& sequence,
    unsigned size, unsigned window, LaneStats& stats, bool measured) {
    std::vector<std::uint8_t> bytes; bytes.reserve((size + PACKET_HEADER_BYTES) * window);
    for (unsigned i = 0; i < window; ++i) { auto f = Frame(id, sequence + i, size); bytes.insert(bytes.end(), f.begin(), f.end()); }
    const auto sent = Clock::now();
    if (!SendBytes(socket, bytes)) { ++stats.errors; return false; }
    for (unsigned i = 0; i < window; ++i) {
        PACKET_FRAME f;
        if (!Receive(socket, parser, f) || !Valid(f, id, sequence + i, size)) { ++stats.errors; return false; }
        stats.checksum += Hash(f.Payload);
        if (measured) { ++stats.measured; stats.latency.push_back(Milliseconds(Clock::now() - sent)); }
        else ++stats.warmup;
    }
    sequence += window; return true;
}
double Percentile(const std::vector<double>& values, double p) {
    if (values.empty()) return 0.;
    return values[static_cast<size_t>(p * static_cast<double>(values.size() - 1))];
}
int Benchmark(const Options& o) {
    std::vector<Socket> sockets; for (unsigned i = 0; i < o.connections; ++i) sockets.push_back(Connect(o.port));
    std::vector<LaneStats> stats(o.connections);
    std::barrier gate(static_cast<ptrdiff_t>(o.connections + 1));
    std::atomic_bool go{false}; Clock::time_point deadline;
    std::vector<std::thread> lanes;
    for (unsigned i = 0; i < o.connections; ++i) lanes.emplace_back([&, i] {
        CPacketStreamParser parser; std::uint32_t sequence = 0;
        gate.arrive_and_wait();
        const auto warmEnd = Clock::now() + std::chrono::milliseconds(o.warmupMs);
        while (Clock::now() < warmEnd && Batch(sockets[i].value, parser, i, sequence, o.payload, o.window, stats[i], false)) {}
        gate.arrive_and_wait();
        while (!go.load(std::memory_order_acquire)) std::this_thread::yield();
        if (!stats[i].errors) while (Clock::now() < deadline && Batch(sockets[i].value, parser, i, sequence, o.payload, o.window, stats[i], true)) {}
    });
    gate.arrive_and_wait(); gate.arrive_and_wait();
    Write(o.directory / "measure.start", "start\n");
    const auto acknowledgeDeadline = Clock::now() + std::chrono::seconds(3);
    while (!fs::exists(o.directory / "measure.ready") && Clock::now() < acknowledgeDeadline) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    const auto start = Clock::now(); deadline = start + std::chrono::milliseconds(o.durationMs);
    go.store(true, std::memory_order_release);
    for (auto& lane : lanes) lane.join();
    const double elapsed = Milliseconds(Clock::now() - start);
    Write(o.directory / "measure.stop", "stop\n");
    std::vector<double> latency; std::uint64_t measured = 0, warmup = 0, checksum = 0; unsigned errors = 0;
    for (const auto& s : stats) { measured += s.measured; warmup += s.warmup; checksum += s.checksum; errors += s.errors; latency.insert(latency.end(), s.latency.begin(), s.latency.end()); }
    if (!fs::exists(o.directory / "measure.ready")) ++errors;
    std::sort(latency.begin(), latency.end());
    std::ostringstream json;
    json << "{\"scenario\":\"benchmark\",\"connections\":" << o.connections << ",\"payload_bytes\":" << o.payload
         << ",\"wire_frame_bytes\":" << o.payload + PACKET_HEADER_BYTES << ",\"pipeline_window\":" << o.window
         << ",\"warmup_ms_requested\":" << o.warmupMs << ",\"measurement_ms\":" << elapsed
         << ",\"duration_ms_requested\":" << o.durationMs << ",\"valid_frames\":" << measured
         << ",\"warmup_valid_frames\":" << warmup << ",\"payload_checksum_sum_total\":\"" << checksum << "\""
         << ",\"errors\":" << errors << ",\"frames_per_second\":" << measured * 1000. / elapsed
         << ",\"p50_rtt_ms\":" << Percentile(latency, .50) << ",\"p95_rtt_ms\":" << Percentile(latency, .95)
         << ",\"p99_rtt_ms\":" << Percentile(latency, .99)
         << ",\"latency_definition\":\"batch send start to each validated reply; closed-loop pipelined workload\"}\n";
    Write(o.directory / "client.json", json.str()); return errors ? 2 : 0;
}

int Correctness(const Options& o) {
    unsigned checks = 0, errors = 0;
    auto check = [&](bool ok) { ++checks; if (!ok) ++errors; return ok; };
    {
        auto socket = Connect(o.port); CPacketStreamParser parser; PACKET_FRAME frame;
        // Deliberately fragment both the six-byte header and payload across send calls.
        for (unsigned seq = 0; seq < 16; ++seq) {
            auto bytes = Frame(700, seq, 73);
            if (!check(SendBytes(socket.value, bytes, seq % 5 + 1) && Receive(socket.value, parser, frame) && Valid(frame, 700, seq, 73))) break;
        }
        std::vector<std::uint8_t> joined;
        for (unsigned seq = 16; seq < 48; ++seq) { auto f = Frame(700, seq, 73); joined.insert(joined.end(), f.begin(), f.end()); }
        if (check(SendBytes(socket.value, joined))) for (unsigned seq = 16; seq < 48; ++seq)
            if (!check(Receive(socket.value, parser, frame) && Valid(frame, 700, seq, 73))) break;
    }
    {
        auto socket = Connect(o.port); CPacketStreamParser parser; PACKET_FRAME frame;
        const unsigned large = (std::min)(60000u, static_cast<unsigned>(MAX_PACKET_BYTES - PACKET_HEADER_BYTES));
        std::atomic_bool sent{true};
        std::thread producer([&] { for (unsigned seq = 0; seq < 32; ++seq) if (!SendBytes(socket.value, Frame(701, seq, large))) { sent = false; break; } });
        // Server accepted socket has a 1 KB send buffer in correctness mode.
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        for (unsigned seq = 0; seq < 32; ++seq) if (!check(Receive(socket.value, parser, frame) && Valid(frame, 701, seq, large))) break;
        producer.join(); check(sent.load());
    }
    for (unsigned iteration = 0; iteration < 20; ++iteration) {
        auto socket = Connect(o.port); CPacketStreamParser parser; PACKET_FRAME frame;
        check(SendBytes(socket.value, Frame(800 + iteration, 0, 128)) && Receive(socket.value, parser, frame) && Valid(frame, 800 + iteration, 0, 128));
        // Half-close and immediate reconnect; notify and pending I/O lifetime are exercised.
        shutdown(socket.value, SD_BOTH);
    }
    {
        std::vector<Socket> sockets;
        for (unsigned i = 0; i < 16; ++i) {
            sockets.push_back(Connect(o.port)); CPacketStreamParser parser; PACKET_FRAME frame;
            // A successful connect/send can still be in the listen backlog.
            // First echo proves this specific session reached server dispatch.
            check(SendBytes(sockets.back().value, Frame(900 + i, 0, 4096)) &&
                Receive(sockets.back().value, parser, frame) && Valid(frame, 900 + i, 0, 4096));
            check(SendBytes(sockets.back().value, Frame(900 + i, 1, 8192)));
        }
        std::vector<std::thread> closers;
        for (auto& s : sockets) closers.emplace_back([value = s.value] { shutdown(value, SD_BOTH); });
        for (auto& t : closers) t.join();
    }
    // Keep pending receives and queued responses alive while server Stop cancels them.
    std::vector<Socket> shutdownPeers;
    for (unsigned i = 0; i < 8; ++i) {
        shutdownPeers.push_back(Connect(o.port)); CPacketStreamParser parser; PACKET_FRAME frame;
        check(SendBytes(shutdownPeers.back().value, Frame(950 + i, 0, 8192)) &&
            Receive(shutdownPeers.back().value, parser, frame) && Valid(frame, 950 + i, 0, 8192));
        check(SendBytes(shutdownPeers.back().value, Frame(950 + i, 1, 32768)));
    }
    Write(o.directory / "server.stop", "stop while peers connected\n");
    // Keep these client sockets open until server cancellation and drain finish.
    const auto shutdownDeadline = Clock::now() + std::chrono::seconds(8);
    while (!fs::exists(o.directory / "server.json") && Clock::now() < shutdownDeadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    check(fs::exists(o.directory / "server.json"));
    std::ostringstream json;
    json << "{\"scenario\":\"correctness\",\"checks\":" << checks << ",\"errors\":" << errors
         << ",\"fragmented_send\":true,\"coalesced_send\":true,\"slow_reader_fifo\":true,\"reconnect_iterations\":20,"
         << "\"concurrent_shutdown_peers\":16,\"server_shutdown_connected_peers\":8,\"expected_accepted_sessions\":46,\"partial_send_proven\":false}\n";
    Write(o.directory / "client.json", json.str()); return errors ? 2 : 0;
}
}
int main(int argc, char** argv) {
    // Dedicated test-process hard bound also covers a regression in Stop/join.
    std::jthread watchdog([](std::stop_token token) {
        const auto end = Clock::now() + std::chrono::seconds(90);
        while (!token.stop_requested() && Clock::now() < end) std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (!token.stop_requested()) TerminateProcess(GetCurrentProcess(), 124);
    });
    try {
#ifdef LOSTARK_FANOUT_HARNESS
        if (argc > 1 && std::string(argv[1]) == "--fanout") return RunSnapshotFanoutBenchmark(argc, argv);
        if (argc > 1 && std::string(argv[1]) == "--job-tests") return RunJobSystemTests() ? 0 : 2;
#endif
        const auto options = Parse(argc, argv); fs::create_directories(options.directory); Winsock winsock;
        return options.mode == "server" ? Server(options) : options.scenario == "correctness" ? Correctness(options) : Benchmark(options);
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
~~~~

<a id="file-tools-serverconcurrencyharness-private-snapshotfanoutbenchmark-cpp"></a>

### Tools/ServerConcurrencyHarness/Private/SnapshotFanoutBenchmark.cpp

파일: C:/Users/tnest/Desktop/LostArk/Tools/ServerConcurrencyHarness/Private/SnapshotFanoutBenchmark.cpp

같은 세션·payload를 두 executor로 제출한다. 매 batch 전원 수신을 확인해 drop/coalescing 없는 유효량을 맞춘다. enqueue 시간과 batch 완료 시간을 따로 기록하고 한 프로세스 합산 CPU임을 출력한다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~cpp
#include "ClientSession.h"
#include "IocpService.h"
#include "ServerSnapshotFanout.h"
#include "Concurrency/WorkStealingJobSystem.h"
#include <Windows.h>
#include <WS2tcpip.h>
#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

namespace
{
    using Clock = std::chrono::steady_clock;
    using namespace LostArk::Server;
    struct Socket final
    {
        SOCKET Value = INVALID_SOCKET;
        ~Socket() { if (Value != INVALID_SOCKET) { ::shutdown(Value, SD_BOTH); ::closesocket(Value); } }
        SOCKET Release() { const auto value = Value; Value = INVALID_SOCKET; return value; }
    };
    std::uint64_t CpuTicks()
    {
        FILETIME created{}, exited{}, kernel{}, user{};
        if (!GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user))
            throw std::runtime_error("GetProcessTimes failed");
        return ((std::uint64_t(kernel.dwHighDateTime) << 32) | kernel.dwLowDateTime) +
            ((std::uint64_t(user.dwHighDateTime) << 32) | user.dwLowDateTime);
    }
    double Percentile(std::vector<double> values, const double fraction)
    {
        std::sort(values.begin(), values.end());
        const auto index = (std::min)(values.size() - 1,
            static_cast<std::size_t>(std::ceil(fraction * values.size())) - 1);
        return values[index];
    }
    class Fixture final
    {
    public:
        std::vector<std::shared_ptr<CClientSession>> Sessions;
        std::vector<std::unique_ptr<Socket>> Clients;
        std::vector<std::thread> Receivers;
        std::atomic_size_t Received{0};
        std::atomic_bool Failed{false};
        std::atomic_bool Stopping{false};
        std::mutex Mutex;
        std::condition_variable Changed;
        CIocpService Service;
        bool WsaStarted = false;

        ~Fixture()
        {
            Stopping.store(true);
            for (const auto& session : Sessions) session->Request_Close();
            for (const auto& session : Sessions) session->Stop();
            for (const auto& client : Clients)
                if (client->Value != INVALID_SOCKET) ::shutdown(client->Value, SD_BOTH);
            for (auto& thread : Receivers) if (thread.joinable()) thread.join();
            Sessions.clear();
            Clients.clear();
            Service.Stop();
            if (WsaStarted) WSACleanup();
        }
        void Start(const std::size_t count, const std::size_t payloadBytes,
            const SESSION_TRANSPORT_BACKEND backend, const std::size_t workers)
        {
            WSADATA data{};
            if (WSAStartup(MAKEWORD(2, 2), &data)) throw std::runtime_error("WSAStartup failed");
            WsaStarted = true;
            if (backend == SESSION_TRANSPORT_BACKEND::IOCP && !Service.Start(workers))
                throw std::runtime_error("IOCP Start failed");
            Socket listener{::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)};
            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            if (listener.Value == INVALID_SOCKET || ::bind(listener.Value,
                reinterpret_cast<sockaddr*>(&address), sizeof(address)) || ::listen(listener.Value, SOMAXCONN))
                throw std::runtime_error("Loopback listener failed");
            int length = sizeof(address);
            if (::getsockname(listener.Value, reinterpret_cast<sockaddr*>(&address), &length))
                throw std::runtime_error("getsockname failed");
            for (std::size_t index = 0; index < count; ++index)
            {
                auto client = std::make_unique<Socket>();
                client->Value = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (client->Value == INVALID_SOCKET || ::connect(client->Value,
                    reinterpret_cast<sockaddr*>(&address), sizeof(address)))
                    throw std::runtime_error("Loopback connect failed");
                DWORD receiveTimeout = 5000;
                setsockopt(client->Value, SOL_SOCKET, SO_RCVTIMEO,
                    reinterpret_cast<const char*>(&receiveTimeout), sizeof(receiveTimeout));
                Socket accepted{::accept(listener.Value, nullptr, nullptr)};
                if (accepted.Value == INVALID_SOCKET) throw std::runtime_error("accept failed");
                auto session = std::make_shared<CClientSession>(static_cast<SESSION_ID>(index + 1),
                    accepted.Value, CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{},
                    backend, backend == SESSION_TRANSPORT_BACKEND::IOCP ? &Service : nullptr);
                accepted.Release();
                Sessions.push_back(session);
                Clients.push_back(std::move(client));
                if (!session->Start()) throw std::runtime_error("Session Start failed");
                const SOCKET clientSocket = Clients.back()->Value;
                Receivers.emplace_back([this, clientSocket, payloadBytes]
                {
                    try
                    {
                        LostArk::Shared::CPacketStreamParser parser;
                        std::vector<std::uint8_t> bytes(16384);
                        std::uint64_t expected = 0;
                        while (!Stopping.load())
                        {
                            const auto read = ::recv(clientSocket, reinterpret_cast<char*>(bytes.data()),
                                static_cast<int>(bytes.size()), 0);
                            if (read <= 0) break;
                            if (!parser.Append({bytes.data(), static_cast<std::size_t>(read)}))
                                throw std::runtime_error("parser overflow");
                            LostArk::Shared::PACKET_FRAME frame;
                            for (;;)
                            {
                                const auto parsed = parser.Try_Pop(frame);
                                if (parsed == LostArk::Shared::PACKET_PARSE_RESULT::NEED_MORE_DATA) break;
                                if (parsed != LostArk::Shared::PACKET_PARSE_RESULT::FRAME_READY ||
                                    frame.ePacketType != LostArk::Shared::PACKET_TYPE::S2C_WORLD_SNAPSHOT ||
                                    frame.Payload.size() != payloadBytes) throw std::runtime_error("invalid frame");
                                std::uint64_t sequence = 0;
                                for (unsigned bit = 0; bit < 8; ++bit)
                                    sequence |= std::uint64_t(frame.Payload[bit]) << (bit * 8);
                                if (sequence != expected++) throw std::runtime_error("sequence mismatch");
                                for (std::size_t i = 8; i < frame.Payload.size(); ++i)
                                    if (frame.Payload[i] != static_cast<std::uint8_t>(i * 31u))
                                        throw std::runtime_error("payload mismatch");
                                // Update the wait predicate under the same mutex to avoid lost wakeups.
                                { std::lock_guard lock(Mutex); Received.fetch_add(1); }
                                Changed.notify_one();
                            }
                        }
                        if (!Stopping.load()) { std::lock_guard lock(Mutex); Failed.store(true); }
                    }
                    catch (...) { std::lock_guard lock(Mutex); Failed.store(true); }
                    Changed.notify_one();
                });
            }
        }
        void Wait(const std::size_t target)
        {
            std::unique_lock lock(Mutex);
            if (!Changed.wait_for(lock, std::chrono::seconds(5),
                [&] { return Failed.load() || Received.load() >= target; }) || Failed.load())
                throw std::runtime_error("Fanout receive failed or timed out");
        }
    };
}

int RunSnapshotFanoutBenchmark(const int argc, char** argv)
{
    try
    {
        std::string transport = "select", executor = "serial", output;
        std::size_t count = 4, iterations = 500, payloadBytes = 1024, workers = 2, iocpWorkers = 2, warmup = 50;
        for (int i = 2; i < argc; ++i)
        {
            const std::string key(argv[i]);
            if (++i >= argc) throw std::runtime_error("Missing fanout option value");
            const std::string value(argv[i]);
            if (key == "--transport") transport = value;
            else if (key == "--executor") executor = value;
            else if (key == "--output") output = value;
            else
            {
                std::size_t* target = key == "--connections" ? &count : key == "--iterations" ? &iterations :
                    key == "--payload-bytes" ? &payloadBytes : key == "--workers" ? &workers :
                    key == "--warmup" ? &warmup : key == "--iocp-workers" ? &iocpWorkers : nullptr;
                if (!target) throw std::runtime_error("Unknown fanout option");
                const auto parsed = std::from_chars(value.data(), value.data()+value.size(), *target);
                if (parsed.ec != std::errc{} || parsed.ptr != value.data()+value.size())
                    throw std::runtime_error("Invalid numeric fanout option");
            }
        }
        if ((transport != "select" && transport != "iocp") || (executor != "serial" && executor != "chaselev") ||
            count < 1 || count > 128 || iterations < 1 || iterations > 10000 || warmup > 1000 ||
            payloadBytes < 8 || payloadBytes > 60000 || workers < 1 || workers > 16 || iocpWorkers < 1 || iocpWorkers > 16)
            throw std::runtime_error("Fanout option out of range");
        Fixture fixture;
        fixture.Start(count, payloadBytes, transport == "iocp" ? SESSION_TRANSPORT_BACKEND::IOCP :
            SESSION_TRANSPORT_BACKEND::SELECT_THREADS, iocpWorkers);
        std::unique_ptr<LostArk::Shared::Concurrency::WorkStealingJobSystem> jobs;
        if (executor == "chaselev") jobs = std::make_unique<LostArk::Shared::Concurrency::WorkStealingJobSystem>(
            LostArk::Shared::Concurrency::WorkStealingJobSystem::Config{workers, 4096});
        std::vector<std::uint8_t> payload(payloadBytes);
        for (std::size_t i = 8; i < payload.size(); ++i) payload[i] = static_cast<std::uint8_t>(i * 31u);
        std::vector<double> enqueueUs, completedUs;
        LostArk::Shared::Concurrency::JobSystemStats initialJobStats{};
        std::uint64_t totalJobs = 0, cpuBegin = 0;
        Clock::time_point measureBegin;
        for (std::size_t iteration = 0; iteration < warmup + iterations; ++iteration)
        {
            if (iteration == warmup) { cpuBegin = CpuTicks(); measureBegin = Clock::now(); if (jobs) initialJobStats = jobs->GetStats(); }
            for (unsigned bit = 0; bit < 8; ++bit) payload[bit] = static_cast<std::uint8_t>(iteration >> (bit * 8));
            const auto begin = Clock::now();
            const auto result = Dispatch_WorldSnapshot(fixture.Sessions, payload, jobs.get());
            const auto enqueued = Clock::now();
            if (result.Failures || result.Recipients != count) throw std::runtime_error("Fanout enqueue failure");
            fixture.Wait((iteration + 1) * count);
            const auto completed = Clock::now();
            if (iteration >= warmup)
            {
                enqueueUs.push_back(std::chrono::duration<double, std::micro>(enqueued-begin).count());
                completedUs.push_back(std::chrono::duration<double, std::micro>(completed-begin).count());
                totalJobs += result.SubmittedJobs;
            }
        }
        const double wallMs = std::chrono::duration<double, std::milli>(Clock::now()-measureBegin).count();
        const double cpuMs = static_cast<double>(CpuTicks()-cpuBegin) / 10000.0;
        const auto finalJobStats = jobs ? jobs->GetStats() : LostArk::Shared::Concurrency::JobSystemStats{};
        for (const auto& session : fixture.Sessions)
        {
            const auto metrics = session->Get_OutboundMetrics();
            if (metrics.iSnapshotCoalescedFrameCount || metrics.iSnapshotDroppedFrameCount ||
                metrics.iReliableRejectedFrameCount || metrics.iSendFailureCount)
                throw std::runtime_error("Fanout trial was not lossless");
        }
        std::ofstream file;
        if (!output.empty()) { file.open(output, std::ios::binary); if (!file) throw std::runtime_error("Output open failed"); }
        auto& out = output.empty() ? std::cout : file;
        out << "{\"schema\":\"lostark.snapshot-fanout-benchmark\",\"version\":1,\"valid\":true,"
            << "\"workload\":\"synthetic payload using production session fanout; not gameplay tick\","
            << "\"cpuScope\":\"server and local receiving clients combined in one process\","
            << "\"transport\":\"" << transport << "\",\"executor\":\"" << executor << "\","
            << "\"connections\":" << count << ",\"workers\":" << workers << ",\"warmupBatches\":" << warmup
            << ",\"iocpWorkers\":" << (transport == "iocp" ? iocpWorkers : 0)
            << ",\"measuredBatches\":" << iterations << ",\"payloadBytes\":" << payloadBytes
            << ",\"receivedFrames\":" << iterations*count << ",\"submittedJobs\":" << totalJobs
            << ",\"localPushes\":" << finalJobStats.localPushes-initialJobStats.localPushes
            << ",\"injectionPushes\":" << finalJobStats.injectionPushes-initialJobStats.injectionPushes
            << ",\"steals\":" << finalJobStats.steals-initialJobStats.steals
            << ",\"overflowInline\":" << finalJobStats.overflowInline-initialJobStats.overflowInline
            << ",\"wallMs\":" << wallMs << ",\"combinedCpuMs\":" << cpuMs
            << ",\"framesPerSecond\":" << iterations*count*1000.0/wallMs
            << ",\"enqueueP50Us\":" << Percentile(enqueueUs, .50)
            << ",\"enqueueP95Us\":" << Percentile(enqueueUs, .95)
            << ",\"enqueueP99Us\":" << Percentile(enqueueUs, .99)
            << ",\"batchCompleteP50Us\":" << Percentile(completedUs, .50)
            << ",\"batchCompleteP95Us\":" << Percentile(completedUs, .95)
            << ",\"batchCompleteP99Us\":" << Percentile(completedUs, .99)
            << ",\"payloadErrors\":0,\"coalesced\":0,\"dropped\":0,\"enqueueSamplesUs\":[";
        for (std::size_t i = 0; i < enqueueUs.size(); ++i) { if (i) out << ','; out << enqueueUs[i]; }
        out << "],\"batchCompleteSamplesUs\":[";
        for (std::size_t i = 0; i < completedUs.size(); ++i) { if (i) out << ','; out << completedUs[i]; }
        out << "]}\n";
        if (!out) throw std::runtime_error("Output write failed");
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Fanout benchmark invalid: " << error.what() << '\n';
        return 1;
    }
}
~~~~

<a id="file-tools-serverconcurrencyharness-readme-md"></a>

### Tools/ServerConcurrencyHarness/README.md

파일: C:/Users/tnest/Desktop/LostArk/Tools/ServerConcurrencyHarness/README.md

비교 조건·실행·결과 해석의 정본이다. 자동 실행은 소유한 격리 프로세스만 시작하고 종료하며 원시 결과를 out에 남긴다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~markdown
# 서버 전송·CPU 작업 비교

이 도구는 LostArk 제품의 `CClientSession`, Shared frame writer/parser와 snapshot fanout 함수를
그대로 컴파일한다. Client.exe와 게임 화면을 실행하지 않고 127.0.0.1의 임시 포트를 쓴다.
기존 `select`와 추가 `iocp`, CPU 작업의 `serial`과 `chaselev`를 각각 선택한다.
IOCP와 Chase–Lev는 오래 검증된 기법이고 C++20 atomic wait/notify를 이번 구현에 사용했다.
기술 이름이나 구현 완료만으로 성능 개선을 확정하지 않는다.

## G01. 현재 제품과 새 선택 지점

| 파일 | 책임 |
|---|---|
| Server/Private/Main.cpp | transport, IOCP worker 수, snapshot executor, job worker 수 선택 |
| Server/Private/ServerApp.cpp | accept/room thread, IOCP 서비스와 job pool의 생성·종료 |
| Server/Private/ClientSession.cpp | 두 backend가 공유하는 프레임 생성·bounded queue·진단·reliable transaction |
| Server/Private/ClientSession_Iocp.cpp | 세션당 한 receive와 한 send, 부분 송신, 취소·완료 수명 |
| Server/Private/IocpService.cpp | 공유 completion port와 정해진 수의 worker |
| Shared/Public/Concurrency/ChaseLevDeque.h | owner bottom push/pop, thief top CAS |
| Shared/Private/Concurrency/WorkStealingJobSystem.cpp | worker 수명, 외부 injection queue, counter, caller-help wait |
| Server/Private/ServerSnapshotFanout.cpp | immutable snapshot payload를 세션별로 framing/enqueue하고 join |
| Server/Private/GameRoom_Replication.cpp | 같은 snapshot을 1회 직렬화한 뒤 fanout 호출 |

`select`는 기존 세션별 receive/send thread를 유지한다. `iocp`는 이 둘을 completion worker로
대체하며 accept thread와 30Hz room thread는 유지한다. 모든 room의 gameplay Tick은 계속
한 room thread에서 순차 실행한다. 패킷 규약, HP·이동·피해 권위, queue 상한과 snapshot
coalescing은 같은 계약이다. 서버 기본값은 기존 `select + serial`이다.

```text
Receive/select 또는 IOCP completion
→ 기존 Shared frame/parser와 On_SessionFrame
→ Room command queue
→ 단일 30Hz Room Tick
→ snapshot message 1회 encode
→ serial 또는 JobSystem의 세션별 framing/enqueue
→ 기존 sender 또는 IOCP WSASend completion
```

다른 프로세스나 session의 상태를 공유 worker에서 직접 갱신하지 않는다. fanout job은
자기 index 결과와 해당 session의 송신 queue만 접근한다. owner는 결과를 join한 뒤 오류와
집계를 처리한다. network send 완료까지 job worker를 묶어두지 않는다.

## G02. 빌드와 선택

제품 코드는 정상 Product Build를 사용한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
```

비교 도구는 `Tools/ServerConcurrencyHarness/Default/ServerConcurrencyHarness.vcxproj`를
같은 Visual Studio 설치의 MSBuild로 x64 Release Build한다. 출력은
`out/ServerConcurrencyHarness/Release/ServerConcurrencyHarness.exe`다. .vcxproj/.filters에
각 CPP가 등록돼 있으며 일반 Solution Build에 부하 실험을 자동 추가하지 않는다.

```powershell
# 개발자 PowerShell에서 같은 VS toolchain 사용
MSBuild Tools/ServerConcurrencyHarness/Default/ServerConcurrencyHarness.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m:1

powershell -ExecutionPolicy Bypass -File Tools/ServerConcurrencyHarness/Run-ComparisonProfile.ps1 -ExePath out/ServerConcurrencyHarness/Release/ServerConcurrencyHarness.exe -Profile quick
```

`ComparisonProfiles.json`의 `quick`/`portfolio`에서 연결 수, worker 수, 반복 수, warmup,
payload 크기를 선택한다. `-Scope Transport` 또는 `-Scope Fanout`으로 한 축만 재측정할 수 있다.
제품에서 사용할 선택 인자는 다음과 같다. 실제 제품 Server 실행은 현재 팀 endpoint와
런타임 데이터 준비 상태를 확인한 뒤 수행한다.

```text
기존: --transport select --snapshot-executor serial
전송만: --transport iocp --iocp-workers 2 --snapshot-executor serial
CPU만: --transport select --snapshot-executor chaselev --job-workers 2
둘 다: --transport iocp --iocp-workers 2 --snapshot-executor chaselev --job-workers 2
```

## G03. 측정값을 읽는 방법

Transport는 별도 server/client 프로세스로 실행한다. 동일한 신뢰 프레임 payload, 연결 수,
connection당 window 8로 warmup 후 A-B-B-A 순서에서 처리량·RTT·서버 CPU·표본 스레드 수를
저장한다. RTT는 해당 batch 송신 시작부터 각 reply 검증까지다. 폐회로 부하이므로 서버가
빨리 응답하면 다음 요청도 빨리 온다. 고정 유입률에서의 최대 수용량이나 실제 LAN 지연은 아니다.
서버 CPU 계측 구간은 marker 확인 지연이 포함된다. 측정된 서버 처리 수와 구간 길이도 함께 본다.

Fanout는 실제 `Dispatch_WorldSnapshot`에 동일 synthetic payload와 실제 세션들을 전달한다.
batch마다 모든 수신을 검증한 뒤 다음 batch를 보내서 두 방식의 유효 작업량을 맞춘다.
framing/enqueue p50·p95·p99와 모든 수신 완료 시간을 구분한다. 이 시험은 한 프로세스에
송신 서버와 수신 스레드가 있으므로 CPU는 합산 비용이다. 실제 월드 snapshot 내용 검증이나
게임 Tick 비용, 플레이 FPS로 표현하지 않는다. 원시 sample 배열도 저장한다.

IOCP 모드의 `Send_Frame`은 queue가 비어 있으면 `Kick_IocpSend`를 통해 `WSASend`의
비동기 게시까지 수행한다. 따라서 fanout 측정에는 frame 작성·enqueue·초기 I/O 게시·join
비용이 포함된다. 순수 memcpy나 Chase–Lev deque 한 연산의 microbenchmark가 아니다.
4연결 fanout은 측정 구간이 짧아 프로세스 CPU 시간의 약 15.6ms 단위에 영향을 크게 받는다.
그 CPU 값만으로 작은 개선률을 주장하지 말고 충분히 긴 반복과 실제 room 계측으로 재확인한다.

| 지표 | 의미 |
|---|---|
| frames/s | 검증된 frame 처리량; offered-load가 고정되지 않은 폐회로 시험 |
| p50/p95/p99 | 분포의 50/95/99 percentile; 최대값이나 평균과 다름 |
| server CPU ms | 별도 transport 서버 프로세스의 user+kernel CPU 시간 |
| sampled peak threads | 주기적으로 관측한 최댓값; 절대 peak를 보장하지 않음 |
| enqueue p95 us | fanout의 제출+실행+join+결과 집계 시간 |
| batch complete p95 us | fanout 시작부터 N개 수신 검증 완료까지 |
| localPushes/steals | 실제 Chase–Lev local path 사용량 |
| injectionPushes | 외부 제출 queue 사용량 |

현재 제품 fanout은 외부 room thread가 독립 root jobs를 제출한다. 이 경우 injection 경로만
사용되고 localPushes/steals가 0일 수 있다. 이 결과는 scheduler 분배 비용의 비교이며
Chase–Lev steal 연산 자체가 속도를 높였다는 증거가 아니다. nested job 정확성 시험에서
owner/thief 경합을 별도로 검증한다. 작은 4인 작업에서는 직렬 방식이 더 빠를 수 있다.

## G04. 정확성·비용과 포트폴리오 표현

다음 항목이 실패하면 해당 trial의 속도를 성과로 사용하지 않는다.

- 프레임 수·payload checksum·순서 일치, 파편화/합쳐진 TCP 데이터 파싱.
- 송신 queue 포화·reliable rejection·실제 send failure가 없는 비교 trial.
- close/reconnect/종료 중 pending I/O 회수; IOCP의 posted/completed/pending 대조.
- 마지막 deque 원소 owner/thief 경쟁, exactly-once, 중첩 Wait, bounded overflow,
  job 예외, shutdown 중 accepted work 완료.
- fanout의 dropped/coalesced 0과 batch별 N개 실제 수신 완료.

IOCP 완료 패킷의 partial-send counter가 0이면 해당 실행이 부분 송신 분기를 실제 재현했다고
말하지 않는다. 지연·종료 검증은 deadlock profiler 구현과 다르다. 현재 변경은 lock graph
순환 검출기를 추가하지 않는다. pool의 local deque와 전체 scheduler의 진행 보장도 구분한다.
외부 injection/lifecycle/session queue에는 mutex가 있으므로 서버 전체를 lock-free라 부르지 않는다.

기록할 문장은 문제 → 선택 → 같은 조건의 수치 → 비용 → 한계 순서다.

> 기존에는 연결마다 수신·송신 스레드가 생성됐다. 기존 프로토콜과 큐 정책을 공유한 채 IOCP
> backend를 추가하고 같은 연결·payload 조건에서 ABBA로 비교했다. 실제 결과표의 처리량,
> p95/p99와 CPU 비용을 근거로 backend를 선택했다. 이 결과의 범위는 로컬 합성 부하이며
> 실제 레이드·LAN 검증과 구분한다.

Winters는 FlatBuffers `.fbs`와 엔진 ECS SystemScheduler의 JobSystem 소비가 있었다.
현재 LostArk 패킷은 자체 PacketWriter/Reader이며 이번 비교에서 직렬화 형식을 함께 바꾸지
않는다. 전송, 패킷 표현, CPU scheduler를 동시에 바꿔 원인을 알 수 없는 성과를 만들지 않는다.

측정 당시 commit/dirty 상태, EXE hash, Release/Debug, CPU/논리 코어, worker 수,
payload·연결 수·반복, background build 유무를 결과와 함께 보관한다. 출력은 out 아래에
생성하며 EXE/PDB/OBJ를 소스 커밋에 넣지 않는다.
~~~~

<a id="file-tools-serverconcurrencyharness-run-comparisonprofile-ps1"></a>

### Tools/ServerConcurrencyHarness/Run-ComparisonProfile.ps1

파일: C:/Users/tnest/Desktop/LostArk/Tools/ServerConcurrencyHarness/Run-ComparisonProfile.ps1

비교 조건·실행·결과 해석의 정본이다. 자동 실행은 소유한 격리 프로세스만 시작하고 종료하며 원시 결과를 out에 남긴다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~powershell
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$ExePath,
    [string]$Profile = 'quick',
    [string]$ProfilePath = (Join-Path $PSScriptRoot 'ComparisonProfiles.json'),
    [string]$OutputRoot = (Join-Path $PSScriptRoot '..\..\out\ServerConcurrencyComparison'),
    [ValidateSet('All','Transport','Fanout')][string]$Scope = 'All'
)
$ErrorActionPreference = 'Stop'
$exe = (Resolve-Path -LiteralPath $ExePath).Path
$profileFile = (Resolve-Path -LiteralPath $ProfilePath).Path
$document = Get-Content -Raw -LiteralPath $profileFile | ConvertFrom-Json
if ($document.formatVersion -ne 1 -or !$document.profiles.PSObject.Properties[$Profile]) { throw 'Unknown comparison profile/version.' }
$settings = $document.profiles.PSObject.Properties[$Profile].Value
$runRoot = Join-Path ([IO.Path]::GetFullPath($OutputRoot)) ('comparison-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ'))
[IO.Directory]::CreateDirectory($runRoot) | Out-Null
Copy-Item -LiteralPath $profileFile -Destination (Join-Path $runRoot 'profiles.json')
$machine = [pscustomobject]@{
    computer = $env:COMPUTERNAME
    os = [Environment]::OSVersion.VersionString
    logicalProcessors = [Environment]::ProcessorCount
    cpu = (Get-CimInstance Win32_Processor | Select-Object -ExpandProperty Name) -join '; '
    memoryBytes = (Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory
    exePath = $exe
    exeSha256 = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash
    gitHead = ((& git -C $PSScriptRoot rev-parse HEAD) -join "`n")
    gitStatus = @(& git -C $PSScriptRoot status --short)
    profile = $Profile
    startedUtc = [DateTime]::UtcNow.ToString('o')
    settings = $settings
    note = 'Loopback synthetic transport/fanout workloads. Record build configuration and background load. No gameplay FPS, LAN capacity, or lock-free whole-server claim.'
}
$machine | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $runRoot 'environment.json') -Encoding utf8
function Invoke-Owned([string[]]$Arguments, [string]$Prefix) {
    $process = $null
    try {
        $process = Start-Process -FilePath $exe -ArgumentList $Arguments -PassThru -WindowStyle Hidden -RedirectStandardOutput ($Prefix+'.stdout.log') -RedirectStandardError ($Prefix+'.stderr.log')
        if (!$process.WaitForExit(60000)) { throw "Harness timeout: $Prefix" }
        $process.Refresh()
        if ($process.ExitCode -ne 0) { throw "Harness failed ($($process.ExitCode)): $Prefix" }
    }
    finally {
        if ($null -ne $process) {
            $process.Refresh()
            if (!$process.HasExited) { Stop-Process -Id $process.Id -Force; $process.WaitForExit(5000) | Out-Null }
            $process.Dispose()
        }
    }
}
Invoke-Owned @('--job-tests') (Join-Path $runRoot 'job-tests')
if ($Scope -in @('All','Transport')) {
    & (Join-Path $PSScriptRoot 'Run-TransportComparison.ps1') -ExePath $exe -OutputRoot $runRoot -Connections $settings.connections -IocpWorkers $settings.iocpWorkers -DurationMs $settings.transportDurationMs -WarmupMs $settings.transportWarmupMs -AbbaRepeats $settings.abbaRepeats
}
if ($Scope -in @('All','Fanout')) {
    $rows = [Collections.Generic.List[object]]::new()
    $ordinal = 0
    foreach ($connections in $settings.connections) {
        foreach ($payloadBytes in $settings.fanoutPayloadBytes) {
            foreach ($transport in @('select','iocp')) {
                for ($repeat = 1; $repeat -le $settings.abbaRepeats; ++$repeat) {
                    foreach ($executor in @('serial','chaselev','chaselev','serial')) {
                        ++$ordinal
                        $prefix = Join-Path $runRoot ('fanout-{0:D3}-{1}-{2}-{3}-{4}' -f $ordinal,$transport,$executor,$connections,$payloadBytes)
                        $resultFile = $prefix+'.json'
                        if ($resultFile.Contains('"')) { throw 'Unsupported quote in output path.' }
                        Invoke-Owned @('--fanout','--transport',$transport,'--executor',$executor,'--connections',$connections,'--payload-bytes',$payloadBytes,'--workers',$settings.jobWorkers,'--iocp-workers',$settings.iocpWorkers,'--warmup',$settings.fanoutWarmupBatches,'--iterations',$settings.fanoutMeasuredBatches,'--output',('"'+$resultFile+'"')) $prefix
                        $result = Get-Content -Raw -LiteralPath $resultFile | ConvertFrom-Json
                        if (!$result.valid -or $result.payloadErrors -or $result.coalesced -or $result.dropped) { throw "Invalid fanout sample: $resultFile" }
                        $result | Add-Member -NotePropertyName trialOrdinal -NotePropertyValue $ordinal
                        $result | Add-Member -NotePropertyName abbaRepeat -NotePropertyValue $repeat
                        $rows.Add($result)
                        Write-Host "fanout $ordinal $transport $executor connections=$connections bytes=$payloadBytes PASS"
                    }
                }
            }
        }
    }
    $rows | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $runRoot 'fanout-runs.json') -Encoding utf8
    $rows | Select-Object trialOrdinal,abbaRepeat,transport,executor,connections,payloadBytes,measuredBatches,framesPerSecond,enqueueP50Us,enqueueP95Us,enqueueP99Us,batchCompleteP50Us,batchCompleteP95Us,batchCompleteP99Us,combinedCpuMs,submittedJobs | Export-Csv -NoTypeInformation -Encoding utf8 -LiteralPath (Join-Path $runRoot 'fanout-runs.csv')
}
Write-Host "Comparison evidence: $runRoot"
~~~~

<a id="file-tools-serverconcurrencyharness-run-transportcomparison-ps1"></a>

### Tools/ServerConcurrencyHarness/Run-TransportComparison.ps1

파일: C:/Users/tnest/Desktop/LostArk/Tools/ServerConcurrencyHarness/Run-TransportComparison.ps1

비교 조건·실행·결과 해석의 정본이다. 자동 실행은 소유한 격리 프로세스만 시작하고 종료하며 원시 결과를 out에 남긴다.

적용: 새 파일 전체를 아래 코드로 작성한다.

~~~~powershell
param(
    [Parameter(Mandatory=$true)][string]$ExePath,
    [Parameter(Mandatory=$true)][string]$OutputRoot,
    [ValidateRange(1,5)][int]$AbbaRepeats = 1,
    [ValidateRange(250,15000)][int]$DurationMs = 3000,
    [ValidateRange(100,5000)][int]$WarmupMs = 1000,
    [ValidateRange(1,16)][int]$IocpWorkers = 4,
    [int[]]$Connections = @(4,32,128),
    [switch]$SkipCorrectness
)
$ErrorActionPreference = 'Stop'
$exe = (Resolve-Path -LiteralPath $ExePath).Path
$root = [IO.Path]::GetFullPath($OutputRoot)
[IO.Directory]::CreateDirectory($root) | Out-Null
$runRoot = Join-Path $root ('transport-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ') + '-' + [Guid]::NewGuid().ToString('N').Substring(0,8))
[IO.Directory]::CreateDirectory($runRoot) | Out-Null
$rows = [Collections.Generic.List[object]]::new()
$ordinal = 0
function Quote-Argument([string]$Value) { if ($Value.Contains('"')) { throw 'Unexpected quote in argument.' }; return '"' + $Value + '"' }
function Run-One([string]$Backend, [int]$Count, [string]$Scenario, [int]$Repeat, [string]$Position) {
    $script:ordinal++
    $directory = Join-Path $runRoot ('{0:D2}-{1}-{2}-{3}' -f $script:ordinal,$Scenario,$Count,$Backend)
    [IO.Directory]::CreateDirectory($directory) | Out-Null
    $server = $null; $client = $null
    try {
        $common = @('--directory', (Quote-Argument $directory), '--backend', $Backend, '--scenario', $Scenario, '--workers', $IocpWorkers)
        $server = Start-Process -FilePath $exe -ArgumentList (@('--mode','server') + $common) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $directory 'server.stdout.log') -RedirectStandardError (Join-Path $directory 'server.stderr.log')
        $ready = Join-Path $directory 'ready.txt'; $deadline = [DateTime]::UtcNow.AddSeconds(8)
        while (!(Test-Path -LiteralPath $ready) -and [DateTime]::UtcNow -lt $deadline -and !$server.HasExited) { Start-Sleep -Milliseconds 20; $server.Refresh() }
        if (!(Test-Path -LiteralPath $ready)) { throw "Server ready timeout: $directory" }
        $port = [int](Get-Content -LiteralPath $ready -TotalCount 1)
        $clientArgs = @('--mode','client','--port',$port,'--connections',$Count,'--payload',256,'--window',8,'--warmup-ms',$WarmupMs,'--duration-ms',$DurationMs) + $common
        $client = Start-Process -FilePath $exe -ArgumentList $clientArgs -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $directory 'client.stdout.log') -RedirectStandardError (Join-Path $directory 'client.stderr.log')
        if (!$client.WaitForExit(60000)) { throw "Client bounded timeout: $directory" }
        $client.Refresh()
        if ($client.ExitCode -ne 0) { throw "Client failed with $($client.ExitCode): $directory" }
        [IO.File]::WriteAllText((Join-Path $directory 'server.stop'), 'stop')
        if (!$server.WaitForExit(12000)) { throw "Server drain timeout: $directory" }
        $server.Refresh()
        if ($server.ExitCode -ne 0) { throw "Server failed with $($server.ExitCode): $directory" }
        $c = Get-Content -LiteralPath (Join-Path $directory 'client.json') -Raw | ConvertFrom-Json
        $s = Get-Content -LiteralPath (Join-Path $directory 'server.json') -Raw | ConvertFrom-Json
        if ($c.errors -ne 0 -or $s.closed_callbacks -ne $s.accepted_session_count -or ($Backend -eq 'iocp' -and ($s.iocp_pending_after_stop -ne 0 -or $s.iocp_posted_operations -ne $s.iocp_completed_operations))) { throw "Correctness/close/drain failed: $directory" }
        if ($Scenario -eq 'correctness' -and $s.accepted_session_count -ne $c.expected_accepted_sessions) { throw "Not every shutdown test session reached the server: $directory" }
        if ($Scenario -eq 'benchmark') {
            $expected = [uint64]$c.valid_frames + [uint64]$c.warmup_valid_frames
            if ($s.accepted_session_count -ne $Count -or $expected -ne [uint64]$s.received_frames_total -or $expected -ne [uint64]$s.sent_frames_total -or [uint64]$c.payload_checksum_sum_total -ne [uint64]$s.payload_checksum_sum_total -or $s.echo_enqueue_failures_total -ne 0 -or $s.reliable_rejections_total -ne 0 -or $s.send_failures_total -ne 0) { throw "Frame/checksum/queue integrity failed: $directory" }
        }
        $row = [pscustomobject]@{ ordinal=$script:ordinal; backend=$Backend; connections=$Count; scenario=$Scenario; repeat=$Repeat; abba_position=$Position; directory=$directory; client=$c; server=$s }
        $rows.Add($row)
        $rows | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $runRoot 'runs.json') -Encoding utf8
        Write-Host ("{0} {1} connections={2} PASS {3}" -f $script:ordinal,$Backend,$Count,$Scenario)
    }
    finally {
        # Only terminate processes created by this invocation; never target product processes.
        foreach ($owned in @($client,$server)) { if ($null -ne $owned) { $owned.Refresh(); if (!$owned.HasExited) { Stop-Process -Id $owned.Id -Force; $owned.WaitForExit(5000) | Out-Null }; $owned.Dispose() } }
    }
}
if (!$SkipCorrectness) { Run-One 'select' 4 'correctness' 0 'correctness'; Run-One 'iocp' 4 'correctness' 0 'correctness' }
foreach ($count in $Connections) {
    if ($count -lt 1 -or $count -gt 256) { throw 'Connections must be 1..256.' }
    for ($repeat=1; $repeat -le $AbbaRepeats; $repeat++) {
        Run-One 'select' $count 'benchmark' $repeat 'A1'
        Run-One 'iocp' $count 'benchmark' $repeat 'B1'
        Run-One 'iocp' $count 'benchmark' $repeat 'B2'
        Run-One 'select' $count 'benchmark' $repeat 'A2'
    }
}
function Median($Values) { $v = @($Values | Sort-Object); if (!$v.Count) { return 0. }; if ($v.Count % 2) { return [double]$v[[int]($v.Count/2)] }; return ([double]$v[$v.Count/2-1]+[double]$v[$v.Count/2])/2. }
$summary = foreach ($count in $Connections) {
    foreach ($backend in @('select','iocp')) {
        $group = @($rows | Where-Object { $_.scenario -eq 'benchmark' -and $_.connections -eq $count -and $_.backend -eq $backend })
        [pscustomobject]@{ connections=$count; backend=$backend; runs=$group.Count; median_frames_per_second=(Median @($group | ForEach-Object {$_.client.frames_per_second})); median_p50_rtt_ms=(Median @($group | ForEach-Object {$_.client.p50_rtt_ms})); median_p95_rtt_ms=(Median @($group | ForEach-Object {$_.client.p95_rtt_ms})); median_p99_rtt_ms=(Median @($group | ForEach-Object {$_.client.p99_rtt_ms})); median_server_cpu_ms=(Median @($group | ForEach-Object {$_.server.server_cpu_ms})); median_server_cpu_ms_per_1000_frames=(Median @($group | ForEach-Object { $_.server.server_cpu_ms * 1000. / $_.client.valid_frames })); max_sampled_server_threads=($group | ForEach-Object {$_.server.peak_threads_sampled} | Measure-Object -Maximum).Maximum }
    }
}
[pscustomobject]@{ scope='Loopback synthetic reliable framed echo transport only; no GameRoom, gameplay, real LAN or Client.exe'; measured_server_cpu='Separate server process; includes same accept/control and thread-sampling overhead'; throughput='Closed-loop window 8 per connection; not a fixed offered-load capacity claim'; exe=$exe; exe_sha256=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash; created_utc=[DateTime]::UtcNow.ToString('o'); abba_repeats=$AbbaRepeats; warmup_ms=$WarmupMs; duration_ms=$DurationMs; iocp_workers=$IocpWorkers; summary=$summary } | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $runRoot 'summary.json') -Encoding utf8
Write-Host "Results: $runRoot"
~~~~

## G04. 검증과 완료 증거

새 CPP의 product·harness vcxproj/filters 등록과 XML/JSON/PowerShell parse, git diff --check, Debug/Release 필요한 product 컴파일을 확인한다. 직접 작성한 CPU/IOCP 정확성 검사와 폐회로 transport ABBA, 실제 session fanout 비교를 실행한다. 완료된 검증·실제 수치·미검증 영역은 대응 RESULT에 구분한다. 사용자 Client 실행과 최종 LAN·레이드 화면 판정은 자동 시험으로 대체하지 않는다.
