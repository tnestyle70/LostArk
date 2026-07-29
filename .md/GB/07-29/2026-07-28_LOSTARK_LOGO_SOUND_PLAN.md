Session - LostArk Logo Yasuo sound bootstrap

목표: 수업 코드의 `NS_BEGIN(Engine)`, `C..._Manager`, `Create()` 규칙을 유지하면서 FMOD 기반 `CSound_Manager`를 물리 `Public/Sound`, `Private/Sound`에 추가하고 Logo 진입 시 Yasuo 효과음 하나를 재생한다.

관련: `2026-07-28_LOSTARK_COURSE_FOUNDATION_INTEGRATION_PLAN.md`

## 0. 범위와 성공 조건

- 지원/검증 구성은 현재 팀 실행 기준인 `Debug|x64`다. 확보한 `fmod.dll`의 PE machine은 `0x8664(x64)`이며 Win32용 FMOD 바이너리는 없다.
- 새 Sound 구현과 호출은 `_WIN64`에서만 컴파일·연결한다. 따라서 이번 변경이 기존 Win32 구성에 x64 FMOD 의존성을 유입하지 않는다.
- ImGui 파일/필터 이름 변경은 하지 않는다. 다만 사용자가 이미 프로젝트에 등록한 ImGui 6개 소스가 x64에서 헤더를 찾도록 include root만 함께 바로잡는다.
- Engine이 FMOD의 생성·갱신·해제를 소유하고 Client는 `CGameInstance` facade만 호출한다.
- `Client/Default`를 working directory로 실행했을 때 Logo가 열리고 Yasuo WAV 재생 요청이 성공하면 완료다. 실제 소리가 들리는지는 사용자가 마지막으로 청취 확인한다.

## 1. 현재 코드 증거와 경계

- `Engine/ThirdPartyLib/FMOD/{Inc,Lib,Bin}`은 존재하지만 `Engine.vcxproj`에 include/lib/dependency가 없고 `Client/Bin/fmod.dll`도 없다.
- `CGameInstance`는 Graphic/Input 등 Engine manager의 생성, tick, 해제를 소유한다. Sound도 같은 수명 규칙을 따른다.
- Client runtime resource 정본은 기존 코드가 사용하는 복수형 `Client/Bin/Resources`다. 별도 `Resource` root를 만들지 않는다.
- 재생할 원본은 `C:/Users/user/Desktop/Winters/Client/Bin/Resource/Sound/LoL/Champions/Yasuo/yasuo_base_sfx_122_846111995.wav`다. 목적지는 `Client/Bin/Resources/Sound/Yasuo/Yasuo_Q.wav`다.
- 첫 slice는 `Play_Sound`, `Update`, lazy sound cache만 포함한다. BGM, channel enum, focus mute, 3D sound는 후속 범위다.

## 2. 새 파일 전체 본문

### `Engine/Public/Sound/Sound_Manager.h`

```cpp
#pragma once

#include "Engine_Defines.h"

namespace FMOD
{
	class System;
	class Sound;
}

NS_BEGIN(Engine)

class CSound_Manager final
{
private:
	CSound_Manager();

public:
	~CSound_Manager();

public:
	HRESULT Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume);
	void Update();

private:
	HRESULT Initialize();
	FMOD::Sound* Find_Or_LoadSound(const wstring_t& strSoundFilePath);

private:
	FMOD::System* m_pSystem = { nullptr };
	map<wstring_t, FMOD::Sound*> m_Sounds;

public:
	static unique_ptr<CSound_Manager> Create();
};

NS_END
```

### `Engine/Private/Sound/Sound_Manager.cpp`

```cpp
#include "Sound/Sound_Manager.h"

#pragma push_macro("new")
#undef new
#include <fmod.hpp>
#include <fmod_errors.h>
#pragma pop_macro("new")

namespace
{
	void Write_FMOD_Error(const char_t* pOperation, FMOD_RESULT eResult)
	{
#ifdef _DEBUG
		char_t szMessage[512] = {};
		sprintf_s(szMessage, "[LostArk][FMOD] %s failed: %s\n", pOperation, FMOD_ErrorString(eResult));
		OutputDebugStringA(szMessage);
#else
		UNREFERENCED_PARAMETER(pOperation);
		UNREFERENCED_PARAMETER(eResult);
#endif
	}
}

CSound_Manager::CSound_Manager()
{
}

CSound_Manager::~CSound_Manager()
{
	for (auto& SoundPair : m_Sounds)
	{
		if (nullptr != SoundPair.second)
			SoundPair.second->release();
	}
	m_Sounds.clear();

	if (nullptr != m_pSystem)
	{
		m_pSystem->close();
		m_pSystem->release();
		m_pSystem = nullptr;
	}
}

HRESULT CSound_Manager::Initialize()
{
	FMOD_RESULT eResult = FMOD::System_Create(&m_pSystem);
	if (FMOD_OK != eResult || nullptr == m_pSystem)
	{
		Write_FMOD_Error("System_Create", eResult);
		return E_FAIL;
	}

	eResult = m_pSystem->init(128, FMOD_INIT_NORMAL, nullptr);
	if (FMOD_OK != eResult)
	{
		Write_FMOD_Error("System::init", eResult);
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CSound_Manager::Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume)
{
	FMOD::Sound* pSound = Find_Or_LoadSound(strSoundFilePath);
	if (nullptr == pSound)
		return E_FAIL;

	FMOD::Channel* pChannel = nullptr;
	FMOD_RESULT eResult = m_pSystem->playSound(pSound, nullptr, false, &pChannel);
	if (FMOD_OK != eResult || nullptr == pChannel)
	{
		Write_FMOD_Error("System::playSound", eResult);
		return E_FAIL;
	}

	eResult = pChannel->setVolume(fVolume);
	if (FMOD_OK != eResult)
	{
		Write_FMOD_Error("Channel::setVolume", eResult);
		return E_FAIL;
	}

	return S_OK;
}

void CSound_Manager::Update()
{
	if (nullptr == m_pSystem)
		return;

	const FMOD_RESULT eResult = m_pSystem->update();
	if (FMOD_OK != eResult)
		Write_FMOD_Error("System::update", eResult);
}

FMOD::Sound* CSound_Manager::Find_Or_LoadSound(const wstring_t& strSoundFilePath)
{
	const auto SoundIter = m_Sounds.find(strSoundFilePath);
	if (m_Sounds.end() != SoundIter)
		return SoundIter->second;

	const int32_t iPathLength = WideCharToMultiByte(
		CP_UTF8, 0, strSoundFilePath.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (0 == iPathLength)
		return nullptr;

	string strUtf8Path(static_cast<size_t>(iPathLength), '\0');
	if (0 == WideCharToMultiByte(
		CP_UTF8, 0, strSoundFilePath.c_str(), -1,
		strUtf8Path.data(), iPathLength, nullptr, nullptr))
		return nullptr;

	FMOD::Sound* pSound = nullptr;
	const FMOD_RESULT eResult = m_pSystem->createSound(
		strUtf8Path.c_str(), FMOD_DEFAULT, nullptr, &pSound);
	if (FMOD_OK != eResult || nullptr == pSound)
	{
		Write_FMOD_Error("System::createSound", eResult);
		return nullptr;
	}

	m_Sounds.emplace(strSoundFilePath, pSound);
	return pSound;
}

unique_ptr<CSound_Manager> CSound_Manager::Create()
{
	auto pInstance = unique_ptr<CSound_Manager>(new CSound_Manager());
	if (FAILED(pInstance->Initialize()))
		return nullptr;

	return pInstance;
}
```

## 3. 기존 파일 정확한 반영 위치

### `Engine/Public/GameInstance.h`

`For.Input_Device`의 세 getter 바로 아래에 x64 Sound facade를 추가한다.

```cpp
#ifdef _WIN64
public: /* For.Sound_Manager */
	HRESULT Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume);
#endif
```

private manager 목록의 `m_pInput_Device` 바로 아래에 추가한다.

```cpp
#ifdef _WIN64
	unique_ptr<class CSound_Manager>		m_pSound_Manager = { nullptr };
#endif
```

### `Engine/Private/GameInstance.cpp`

`#include "Input_Device.h"` 바로 아래에 추가한다.

```cpp
#ifdef _WIN64
#include "Sound/Sound_Manager.h"
#endif
```

`Initialize_Engine()`의 `m_pInput_Device` 생성/검사 블록 바로 아래에 추가한다.

```cpp
#ifdef _WIN64
	m_pSound_Manager = CSound_Manager::Create();
	if (nullptr == m_pSound_Manager)
		return E_FAIL;
#endif
```

`Update_Engine()`의 `m_pInput_Device->Update();` 바로 아래에 추가한다.

```cpp
#ifdef _WIN64
	m_pSound_Manager->Update();
#endif
```

`Get_DIMouseMove()` 구현 바로 아래에 facade를 추가한다.

```cpp
#ifdef _WIN64
HRESULT CGameInstance::Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume)
{
	return m_pSound_Manager->Play_Sound(strSoundFilePath, fVolume);
}
#endif
```

`Release_Engine()`의 `m_pInput_Device.reset();` 바로 위에 추가한다.

```cpp
#ifdef _WIN64
	m_pSound_Manager.reset();
#endif
```

### `Client/Private/Level_Logo.cpp`

`Ready_Layer_BackGround` 성공 검사 바로 아래, `return S_OK;` 위에 추가한다.

```cpp
#ifdef _WIN64
	if (FAILED(CGameInstance::Get().Play_Sound(
		TEXT("../Bin/Resources/Sound/Yasuo/Yasuo_Q.wav"), 0.8f)))
		return E_FAIL;
#endif
```

### `Engine/Default/Engine.vcxproj`

Debug|x64와 Release|x64의 `AdditionalIncludeDirectories`를 각각 아래 값으로 교체한다.

```xml
<AdditionalIncludeDirectories>..\Public;..\External\imgui;..\External\imgui\backends;..\ThirdPartyLib\FMOD\Inc;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
```

두 x64 `AdditionalLibraryDirectories`를 아래 값으로 교체한다.

```xml
<AdditionalLibraryDirectories>..\ThirdPartyLib;..\ThirdPartyLib\FMOD\Lib;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
```

두 x64 `AdditionalDependencies`의 기존 라이브러리 목록 끝, `%(AdditionalDependencies)` 앞에 `fmod_vc.lib;`를 추가한다.

header ItemGroup 끝에 추가한다.

```xml
<ClInclude Include="..\Public\Sound\Sound_Manager.h" />
```

source ItemGroup에 추가하고 Win32에서는 제외한다.

```xml
<ClCompile Include="..\Private\Sound\Sound_Manager.cpp">
  <ExcludedFromBuild Condition="'$(Platform)'=='Win32'">true</ExcludedFromBuild>
</ClCompile>
```

### `Engine/Default/Engine.vcxproj.filters`

기존 ImGui filter는 그대로 둔다. filter 선언 ItemGroup 끝에 물리 경로와 같은 두 filter만 추가한다.

```xml
<Filter Include="Public\Sound">
  <UniqueIdentifier>{bc421e7e-bb93-46d8-b5de-847b35ce0891}</UniqueIdentifier>
</Filter>
<Filter Include="Private\Sound">
  <UniqueIdentifier>{7cfb14bd-e6a4-4832-97ea-34f61a76e45f}</UniqueIdentifier>
</Filter>
```

ClInclude ItemGroup 끝에 추가한다.

```xml
<ClInclude Include="..\Public\Sound\Sound_Manager.h">
  <Filter>Public\Sound</Filter>
</ClInclude>
```

ClCompile ItemGroup 끝에 추가한다.

```xml
<ClCompile Include="..\Private\Sound\Sound_Manager.cpp">
  <Filter>Private\Sound</Filter>
</ClCompile>
```

### `Engine/External/imgui/imconfig.h`

첫 Engine build에서 ImGui가 LostArk에 없는 `WINTERS_ENGINE_EXPORTS`를 검사하여 모든 구현을 `dllimport`로 선언한 뒤 C2491로 실패했다. Winters에서 복사된 아래 export guard를 LostArk Engine DLL의 기존 export symbol로 교체한다.

```cpp
#ifdef ENGINE_EXPORTS
#define IMGUI_API __declspec(dllexport)
#else
#define IMGUI_API __declspec(dllimport)
#endif
```

### `UpdateLib.bat`

첫 줄은 batch에서 실행 오류가 나는 `//`가 아니라 `rem` 주석으로 교체한다.

```bat
rem 명령어 옵션 원본 파일의 위치 사본 파일을 저장할 위치
```

기존 `Engine/Bin/*.dll` 복사 바로 아래에 FMOD DLL 복사와 실패 전파를 추가한다.

```bat
xcopy /y .\Engine\ThirdPartyLib\FMOD\Bin\fmod.dll .\Client\bin\
if errorlevel 1 exit /b %errorlevel%
```

### WAV 배치

아래 확정 경로를 그대로 복사한다.

```text
C:/Users/user/Desktop/Winters/Client/Bin/Resource/Sound/LoL/Champions/Yasuo/yasuo_base_sfx_122_846111995.wav
-> C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Sound/Yasuo/Yasuo_Q.wav
```

기존 루트의 빈 placeholder `Engine/Public/Sound_Manager.h`, `Engine/Private/Sound_Manager.cpp`는 새 물리 경로 파일로 대체되므로 삭제한다.

## 4. 검증

1. 원본/복사본 WAV 크기와 재생 길이를 확인한다.
2. Engine `Debug|x64` build. ImGui C2491과 Sound/FMOD compile/link 오류가 없어야 한다.
3. `UpdateLib.bat` 실행 후 `Client/Bin/fmod.dll`과 `EngineSDK/inc/Sound/Sound_Manager.h`를 확인한다.
4. Client `Debug|x64` build.
5. `Client/Default` working directory에서 `../Bin/Client.exe` 실행; Logo까지 프로세스가 생존하는지 확인한다.
6. 사용자가 Ctrl+F5로 Yasuo 효과음을 실제 청취한다.
7. project/filter의 include path가 물리 파일과 일치하는지, `git diff --check`가 통과하는지 확인한다.

## 5. 독립 비평 및 처분

초안 비평: FAIL, P0 0 / P1 3.

- P1 Win32에서 x64 FMOD가 섞임: 수용. FMOD slice를 x64-only로 명시하고 Sound source, facade, member, Logo 호출을 Win32에서 제외했다.
- P1 WAV 원본 불명: 수용. Winters 원본 파일과 LostArk 목적지의 절대 경로를 확정했다.
- P1 기존 파일 변경이 prose-only: 수용. 모든 변경에 실제 anchor와 삽입/교체 코드를 추가했다.
- P2 UpdateLib 실패 전파 없음: 수용. FMOD DLL 복사 직후 errorlevel을 전파한다.
- P2 ImGui filter rename은 범위 초과: 수용. rename을 제거하고 기존 6개 source의 x64 include 오류만 고친다.

재비평: PASS — P0 0 / P1 0. 구현 진행 가능.

빌드 적응 비평: ImGui `imconfig.h`의 Winters 전용 export symbol을 LostArk `ENGINE_EXPORTS`로 바꾸는 단일 수정에 대해 재확인 후 적용한다.
