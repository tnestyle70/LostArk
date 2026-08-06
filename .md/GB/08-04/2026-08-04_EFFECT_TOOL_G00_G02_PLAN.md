# Effect Tool G00-G02 전체 코드 PLAN

이 문서는 G00과 G01의 확정 구현 기준선, 그리고 사용자가 다음에 직접 반영할 G02의 전체 코드를 보존한다.
G02는 Active Document에 `Element ID + Kind`만 원자적으로 추가한다. Save/Load, resource ID,
material, preview, timeline, emitter, renderer, graph는 이 문서의 반영 범위가 아니다.
대화에서는 현재 G의 추가·삭제·변경점만 설명한다. 이 PLAN은 대화에서 반복하지 않는 기존 코드까지
포함한 적용 후 H/CPP 전체 전문을 유지해 현재 파일과 최종 결과를 직접 비교할 수 있게 한다.

## G00 — 다섯 Element 종류 선택 기준선

G00은 레거시 Effect runtime과 저장 경로를 제거한 뒤 `Mesh / Texture / Particle / Decal / Trail`
중 다음 제작 종류 하나만 고르는 session-only UI로 끝난 구현 기준선이다.

### G00-1. C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h

변경 종류: G00 전체 교체 기준선

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CEffect_Tool final
{
private:
	enum class EFFECT_TYPE_SELECTION
	{
		MESH,
		TEXTURE,
		PARTICLE,
		DECAL,
		TRAIL,
		END
	};

public:
	CEffect_Tool() = default;
	~CEffect_Tool() = default;

	void Render();

private:
	void Render_EffectTypeSelector();

private:
	EFFECT_TYPE_SELECTION m_eSelectedEffectType =
		EFFECT_TYPE_SELECTION::MESH;
};

NS_END
```

### G00-2. C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp

변경 종류: G00 전체 교체 기준선

```cpp
#include "imgui.h"

#include "Effect_Tool.h"

namespace
{
	constexpr const char_t* EFFECT_TYPE_LABELS[] = {
		"Mesh", "Texture", "Particle", "Decal", "Trail"
	};
}

void Client::CEffect_Tool::Render()
{
	ImGui::SetNextWindowSize(ImVec2(620.f, 150.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Effect Tool###LostArkEffectToolG0"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"G0: choose one effect type. No effect asset is created, loaded, "
		"saved, previewed, or bound in this step.");
	ImGui::Separator();
	Render_EffectTypeSelector();
	ImGui::End();
}

void Client::CEffect_Tool::Render_EffectTypeSelector()
{
	ImGui::TextUnformatted("Effect Type");
	for (int32_t iType = 0;
		iType < static_cast<int32_t>(EFFECT_TYPE_SELECTION::END); ++iType)
	{
		ImGui::SameLine();
		const EFFECT_TYPE_SELECTION eType =
			static_cast<EFFECT_TYPE_SELECTION>(iType);
		if (ImGui::RadioButton(EFFECT_TYPE_LABELS[iType],
			m_eSelectedEffectType == eType))
		{
			m_eSelectedEffectType = eType;
		}
	}
}
```

### G00-3. 프로젝트 등록 기준선

`C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj`

```xml
    <ClInclude Include="..\public\Effect_Tool.h" />
    <ClCompile Include="..\private\Effect_Tool.cpp" />
```

`C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters`

```xml
    <ClCompile Include="..\private\Effect_Tool.cpp">
      <Filter>03. Tools\02. Effect</Filter>
    </ClCompile>
    <ClInclude Include="..\public\Effect_Tool.h">
      <Filter>03. Tools\02. Effect</Filter>
    </ClInclude>
```

### G00-4. 종료 검증

```powershell
rg -n "Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC" Client Engine Tools
rg -n "Shader_Effect|Data\\Effects\\Authored|--effect-" Client/Default/Client.vcxproj Client/Default/Client.vcxproj.filters Client/Default/Client.cpp Engine/Private/ImGuiLayer.cpp
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

기대 결과는 레거시 Effect source/runtime/shader/authored `.effect` 직접 소비 0건, Effect Tool project/filter
등록 유지, 다섯 radio의 상호 배타 선택, 선택 전후 파일 변화 0건이다.

## G01 — 메모리 Active Document 기준선

G01은 현재 저장소에 반영된 구현 상태다. `Effect Asset ID + Display Name + Elements`를 가진 문서 한 건을
메모리에 생성·폐기한다. `strResourceId`는 아직 구조체에 남아 있지만 G01에서 입력하거나 commit하지 않으며,
G02 전체 교체에서 제거한다.

### G01-1. C:/Users/user/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h

변경 종류: 현재 구현 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

// Effect 제작 데이터의 저장 형식 버전이다.
inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 1u;

// 하나의 Effect Document를 구성할 수 있는 시각 요소 종류다.
// END는 유효한 요소가 아니라 초기화 누락을 검출하기 위한 sentinel이다.
enum class EFFECT_ELEMENT_KIND : uint8_t
{
	MESH,
	SPRITE,
	PARTICLE,
	DECAL,
	TRAIL,
	END
};

// Effect Document 안에 들어가는 시각 요소 하나의 데이터다.
// G1에서는 정체성만 정의하고 Transform과 렌더링 값은 이후 G에서 추가한다.
struct EFFECT_ELEMENT_DESC final
{
	// Document 내부에서 Element를 구분하는 안정적인 고유 ID다.
	std::string strElementId;
	// Mesh, Sprite, Particle, Decal, Trail 중 어떤 종류인지 나타낸다.
	EFFECT_ELEMENT_KIND eKind = EFFECT_ELEMENT_KIND::END;
	// 절대 경로가 아닌 Resources 기준 상대 Asset ID다.
	std::string strResourceId;
};

// 메모리에서 제작 중인 재사용 가능한 Effect Asset 하나의 문서다.
// ImGui, GPU, Character, Animation 객체를 소유하지 않는 순수 데이터다.
struct EFFECT_DOCUMENT_DESC final
{
	// 이후 파일을 읽을 때 호환성을 검증할 schema version이다.
	uint32_t iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	// 저장과 런타임 연결에 사용하는 Effect Asset의 안정적인 ID다.
	std::string strEffectAssetId;
	// 제작자에게 표시하는 사람이 읽을 수 있는 이름이다.
	std::string strDisplayName;
	// 이 Effect Asset을 구성하는 시각 요소들을 순서대로 소유한다.
	std::vector<EFFECT_ELEMENT_DESC> Elements;
};

NS_END
```

### G01-2. C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h

변경 종류: 현재 구현 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <array>
#include <optional>
#include <string>

NS_BEGIN(Client)

class CEffect_Tool final
{
public:
	CEffect_Tool() = default;
	~CEffect_Tool() = default;

	// Effect 제작 UI 한 프레임을 그린다.
	void Render();

private:
	// Active Document가 없을 때 ID, 표시 이름 입력창과 Create 버튼을 그린다.
	void Render_NewDocumentPanel();
	// 메모리에 commit된 Active Document의 G1 요약 정보를 읽기 전용으로 그린다.
	void Render_ActiveDocumentPanel();
	// Document를 변경하지 않고 다음에 추가할 Element 종류만 선택한다.
	void Render_EffectTypeSelector();
	// 입력을 검증하고 local Document를 staging한 뒤 한 번에 commit한다.
	bool_t Try_CreateDocument();
	// 파일이나 GPU 객체를 건드리지 않고 메모리 초안만 제거한다.
	void Discard_ActiveDocument();

private:
	// G2의 Add Element가 사용할 ImGui session 선택 상태다.
	EFFECT_ELEMENT_KIND m_eSelectedEffectType =
		EFFECT_ELEMENT_KIND::MESH;
	// 최대 128바이트 ID와 문자열 종료 문자를 담는 편집 buffer다.
	std::array<char_t, 129> m_NewAssetId{};
	// 최대 64바이트 표시 이름과 문자열 종료 문자를 담는 편집 buffer다.
	std::array<char_t, 65> m_NewDisplayName{};
	// nullopt이면 New Document UI, 값이 있으면 Active Document UI를 그린다.
	std::optional<EFFECT_DOCUMENT_DESC> m_ActiveDocument;
	// 마지막 검증, 생성, 폐기 결과를 사용자에게 보여주는 상태 문구다.
	std::string m_strDocumentStatus;
};

NS_END
```

### G01-3. C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp

변경 종류: 현재 구현 전체 코드

```cpp
#include "imgui.h"

#include "Effect_Tool.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace
{
	// EFFECT_ELEMENT_KIND 순서와 일치하며 END는 화면에 표시하지 않는다.
	constexpr const char_t* EFFECT_TYPE_LABELS[] = {
		"Mesh", "Sprite", "Particle", "Decal", "Trail"
	};

	// 다른 컴퓨터에서도 같은 값으로 사용할 수 있는 안정적인 ID 문법인지 검증한다.
	bool_t Is_StableEffectAssetId(const std::string& strValue)
	{
		if (strValue.empty() || strValue.size() > 128u)
			return false;

		return std::all_of(strValue.begin(), strValue.end(),
			[](const char_t value)
			{
				const bool_t isUpper = value >= 'A' && value <= 'Z';
				const bool_t isLower = value >= 'a' && value <= 'z';
				const bool_t isDigit = value >= '0' && value <= '9';
				return isUpper || isLower || isDigit ||
					value == '_' || value == '.' || value == '-';
			});
	}

	// 표시 이름이 빈 값이거나 공백만으로 이루어졌는지 검증한다.
	bool_t Has_VisibleCharacter(const std::string& strValue)
	{
		return std::any_of(strValue.begin(), strValue.end(),
			[](const char_t value)
			{
				return 0 == std::isspace(
					static_cast<unsigned char>(value));
			});
	}
}

void Client::CEffect_Tool::Render()
{
	// ImGui 창이 접혀 Begin이 false를 반환해도 End는 반드시 호출한다.
	ImGui::SetNextWindowSize(ImVec2(620.f, 310.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Effect Tool###LostArkEffectToolG1"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"G1: create one in-memory Effect document. Nothing is saved, "
		"loaded, previewed, rendered, or bound to Animation yet.");
	// optional에 Document가 있는지에 따라 두 패널 중 하나만 그린다.
	if (m_ActiveDocument.has_value())
		Render_ActiveDocumentPanel();
	else
		Render_NewDocumentPanel();

	ImGui::Separator();
	Render_EffectTypeSelector();
	ImGui::TextDisabled(
		"The selected type is session-only until G2 adds an Element.");
	ImGui::End();
}

void Client::CEffect_Tool::Render_NewDocumentPanel()
{
	// InputText는 매 프레임 고정 크기 session buffer를 직접 편집한다.
	ImGui::SeparatorText("Data File");
	ImGui::InputText("Effect Asset ID", m_NewAssetId.data(),
		m_NewAssetId.size());
	ImGui::InputText("Display Name", m_NewDisplayName.data(),
		m_NewDisplayName.size());

	if (ImGui::Button("Create Document"))
		Try_CreateDocument();

	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_ActiveDocumentPanel()
{
	ImGui::SeparatorText("Active Document");
	// optional이 Document를 소유할 때만 이 함수가 호출된다.
	// const reference로 읽어 G1 화면에서 Document가 변경되지 않게 한다.
	const EFFECT_DOCUMENT_DESC& Document = *m_ActiveDocument;
	ImGui::Text("Format Version: %u", Document.iFormatVersion);
	ImGui::Text("Effect Asset ID: %s",
		Document.strEffectAssetId.c_str());
	ImGui::Text("Display Name: %s", Document.strDisplayName.c_str());
	ImGui::Text("Elements: %zu", Document.Elements.size());
	ImGui::TextDisabled("Memory only. Save/Load starts in a later G.");

	if (ImGui::Button("Discard Active Document"))
		Discard_ActiveDocument();

	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_EffectTypeSelector()
{
	// radio 선택은 session 상태만 바꾸며 G2의 Add Element 전에는 Document가 변하지 않는다.
	ImGui::TextUnformatted("Next Element Type");
	for (int32_t iType = 0;
		iType < static_cast<int32_t>(EFFECT_ELEMENT_KIND::END); ++iType)
	{
		ImGui::SameLine();
		const EFFECT_ELEMENT_KIND eType =
			static_cast<EFFECT_ELEMENT_KIND>(iType);
		if (ImGui::RadioButton(EFFECT_TYPE_LABELS[iType],
			m_eSelectedEffectType == eType))
		{
			m_eSelectedEffectType = eType;
		}
	}
}

bool_t Client::CEffect_Tool::Try_CreateDocument()
{
	// G1에서는 기존 메모리 Document를 암묵적으로 덮어쓰지 않는다.
	if (m_ActiveDocument.has_value())
	{
		m_strDocumentStatus =
			"Discard the active document before creating another one.";
		return false;
	}

	// 부분적으로 Document를 수정하지 않도록 ImGui buffer를 먼저 문자열로 복사한다.
	const std::string strAssetId = m_NewAssetId.data();
	const std::string strDisplayName = m_NewDisplayName.data();
	if (!Is_StableEffectAssetId(strAssetId))
	{
		m_strDocumentStatus =
			"Effect Asset ID must be 1-128 ASCII letters, digits, '.', '_' or '-'.";
		return false;
	}
	if (!Has_VisibleCharacter(strDisplayName))
	{
		m_strDocumentStatus = "Display Name must not be blank.";
		return false;
	}

	// 검증을 통과한 후보 Document를 local 값으로 staging한다.
	EFFECT_DOCUMENT_DESC StagedDocument;
	StagedDocument.strEffectAssetId = strAssetId;
	StagedDocument.strDisplayName = strDisplayName;

	// 모든 검증이 성공한 뒤 Active Document에 한 번만 commit한다.
	m_ActiveDocument = std::move(StagedDocument);
	m_NewAssetId.fill('\0');
	m_NewDisplayName.fill('\0');
	m_strDocumentStatus =
		"Created in memory. This document has not been saved.";
	return true;
}

void Client::CEffect_Tool::Discard_ActiveDocument()
{
	// reset은 optional이 소유한 메모리 EFFECT_DOCUMENT_DESC만 제거한다.
	m_ActiveDocument.reset();
	m_strDocumentStatus =
		"Discarded the in-memory document. No file was changed.";
}
```

### G01-4. 프로젝트 등록 현재 코드

`C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj`

```xml
    <ClInclude Include="..\Public\Effect_AuthoringDocument.h" />
    <ClInclude Include="..\public\Effect_Tool.h" />
    <ClCompile Include="..\private\Effect_Tool.cpp" />
```

`C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters`

```xml
    <ClCompile Include="..\private\Effect_Tool.cpp">
      <Filter>03. Tools\02. Effect</Filter>
    </ClCompile>
    <ClInclude Include="..\public\Effect_Tool.h">
      <Filter>03. Tools\02. Effect</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_AuthoringDocument.h">
      <Filter>03. Tools\02. Effect</Filter>
    </ClInclude>
```

### G01-5. C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1

변경 종류: 현재 `effect.g1-document-boundary` 전체 블록

```powershell
	$effectDocumentHeader = Get-Content -LiteralPath 'Client\Public\Effect_AuthoringDocument.h' -Raw
	$effectToolHeader = Get-Content -LiteralPath 'Client\Public\Effect_Tool.h' -Raw
	$effectToolSource = Get-Content -LiteralPath 'Client\Private\Effect_Tool.cpp' -Raw
	$clientEntrySource = Get-Content -LiteralPath 'Client\Default\Client.cpp' -Raw
	$engineImGuiSource = Get-Content -LiteralPath 'Engine\Private\ImGuiLayer.cpp' -Raw
	$clientProjectSource = Get-Content -LiteralPath 'Client\Default\Client.vcxproj' -Raw
	$removedEffectPaths = @(
		'Client\Public\Effect_Types.h',
		'Client\Public\Effect_AssetIO.h',
		'Client\Private\Effect_AssetIO.cpp',
		'Client\Public\Effect_ParticleSimulator.h',
		'Client\Private\Effect_ParticleSimulator.cpp',
		'Client\Public\Effect_Runtime.h',
		'Client\Private\Effect_Runtime.cpp',
		'Client\Public\Effect_ResourceCatalog.h',
		'Client\Private\Effect_ResourceCatalog.cpp')
	$removedEffectPathHits = @($removedEffectPaths |
		Where-Object { Test-Path -LiteralPath $_ })
	$authoredEffectFiles = @(Get-ChildItem -LiteralPath 'Data\Effects\Authored' -Recurse -File -ErrorAction SilentlyContinue)
	$effectIntakeFiles = @(Get-ChildItem -LiteralPath 'Tools\EffectResourceIntake' -Recurse -File -ErrorAction SilentlyContinue)
	$effectShaderFiles = @(Get-ChildItem -LiteralPath 'Client\Bin\ShaderFiles' -File -Filter 'Shader_Effect*' -ErrorAction SilentlyContinue)
	$legacyEffectSymbolHits = @($clientSourceFiles | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC')
	$legacyEffectProjectHits = @($clientProjectSource | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|Shader_Effect|Data\\Effects\\Authored')
	$legacyEffectEntry =
		$clientEntrySource -match 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC|--effect-' -or
		$engineImGuiSource -match '--effect-'
	$effectG1DocumentShape =
		$effectDocumentHeader -match 'EFFECT_AUTHORING_FORMAT_VERSION\s*=\s*1u' -and
		$effectDocumentHeader -match 'enum class EFFECT_ELEMENT_KIND[\s\S]*MESH,[\s\S]*SPRITE,[\s\S]*PARTICLE,[\s\S]*DECAL,[\s\S]*TRAIL,[\s\S]*END' -and
		$effectDocumentHeader -match 'struct EFFECT_DOCUMENT_DESC[\s\S]*strEffectAssetId[\s\S]*strDisplayName[\s\S]*Elements' -and
		$effectToolHeader -match 'optional<EFFECT_DOCUMENT_DESC>\s+m_ActiveDocument' -and
		$effectToolHeader -match 'm_eSelectedEffectType\s*=\s*EFFECT_ELEMENT_KIND::MESH' -and
		$effectToolSource -match 'Is_StableEffectAssetId' -and
		$effectToolSource -match 'Try_CreateDocument' -and
		$effectToolSource -match 'Discard_ActiveDocument' -and
		$effectToolSource -match '"Mesh",\s*"Sprite",\s*"Particle",\s*"Decal",\s*"Trail"' -and
		$effectToolSource -match 'LostArk Effect Tool###LostArkEffectToolG1' -and
		$effectToolSource -notmatch 'filesystem|ifstream|ofstream|GetOpenFileName|ID3D11'
	Add-Check 'effect.g1-document-boundary' (
		$removedEffectPathHits.Count -eq 0 -and
		$authoredEffectFiles.Count -eq 0 -and
		$effectIntakeFiles.Count -eq 0 -and
		$effectShaderFiles.Count -eq 0 -and
		$legacyEffectSymbolHits.Count -eq 0 -and
		$legacyEffectProjectHits.Count -eq 0 -and
		-not $legacyEffectEntry -and
		$effectG1DocumentShape) "paths=$($removedEffectPathHits.Count) authored=$($authoredEffectFiles.Count) intake=$($effectIntakeFiles.Count) shaders=$($effectShaderFiles.Count) symbols=$($legacyEffectSymbolHits.Count) project=$($legacyEffectProjectHits.Count) entry=$legacyEffectEntry document=$effectG1DocumentShape"
```

### G01-6. 종료 검증

```powershell
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj' -Raw) | Out-Null
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj.filters' -Raw) | Out-Null
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

Debug Client에서 F1, Effect Tool 순서로 열고 잘못된 ID와 공백 이름 거부, 정상 문서 생성,
`Elements: 0`, radio 변경 시 문서 불변, Discard 후 입력 panel 복귀를 확인한다.

## G02 — stable Element ID와 Kind 원자적 추가

G02의 실제 수정 파일은 아래 네 개뿐이다.

```text
C:/Users/user/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h
C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h
C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp
C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1
```

`CharacterPreviewPanel.h/.cpp`, `Animation_Tool.h/.cpp`, `MainApp.h/.cpp`, `.vcxproj`,
`.vcxproj.filters`는 G02에서 수정하지 않는다.

### G02-1. C:/Users/user/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h

변경 종류: 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

// Effect 제작 데이터의 저장 형식 버전이다.
inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 1u;

// 하나의 Effect Document를 구성하는 상위 시각 요소 종류다.
// END는 유효한 요소가 아니라 초기화 누락을 검출하기 위한 sentinel이다.
enum class EFFECT_ELEMENT_KIND : uint8_t
{
	MESH,
	SPRITE,
	PARTICLE,
	DECAL,
	TRAIL,
	END
};

// Effect Document 안에 들어가는 시각 요소 한 건의 최소 정체성이다.
// Resource, Material, Transform과 Runtime 객체는 이후 G의 타입별 payload가 소유한다.
struct EFFECT_ELEMENT_DESC final
{
	// Document 내부에서 Element를 구분하는 안정적인 고유 ID다.
	std::string strElementId;
	// Mesh, Sprite, Particle, Decal, Trail 중 상위 제작 단위를 나타낸다.
	EFFECT_ELEMENT_KIND eKind = EFFECT_ELEMENT_KIND::END;
};

// 메모리에서 제작 중인 재사용 가능한 Effect Asset 한 건의 문서다.
// ImGui, GPU, Character, Animation과 Preview 객체를 소유하지 않는 순수 데이터다.
struct EFFECT_DOCUMENT_DESC final
{
	// 이후 파일을 읽을 때 호환성을 검증할 schema version이다.
	uint32_t iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	// 저장과 런타임 연결에 사용하는 Effect Asset의 안정적인 ID다.
	std::string strEffectAssetId;
	// 제작자에게 표시하는 사람이 읽을 수 있는 이름이다.
	std::string strDisplayName;
	// 이 Effect Asset을 구성하는 Element들을 값으로 강하게 소유한다.
	std::vector<EFFECT_ELEMENT_DESC> Elements;
};

NS_END
```

### G02-2. C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h

변경 종류: 전체 교체

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <array>
#include <optional>
#include <string>

NS_BEGIN(Client)

class CEffect_Tool final
{
public:
	CEffect_Tool() = default;
	~CEffect_Tool() = default;

	// Effect 제작 UI 한 프레임을 그린다.
	void Render();

private:
	// Active Document가 없을 때 ID, 표시 이름 입력창과 Create 버튼을 그린다.
	void Render_NewDocumentPanel();
	// 메모리에 commit된 Active Document의 요약과 폐기 명령을 그린다.
	void Render_ActiveDocumentPanel();
	// Document를 변경하지 않고 다음에 추가할 Element 종류만 선택한다.
	void Render_EffectTypeSelector();
	// Active Document가 있을 때 Element ID 입력창과 Add 명령을 그린다.
	void Render_AddElementPanel();
	// Active Document가 소유한 Element ID와 종류를 읽기 전용 목록으로 그린다.
	void Render_ElementList() const;
	// Document 입력을 검증하고 local Document를 staging한 뒤 한 번에 commit한다.
	bool_t Try_CreateDocument();
	// Element 입력과 중복을 검증하고 Document copy에 추가한 뒤 한 번에 commit한다.
	bool_t Try_AddElement();
	// 파일이나 GPU 객체를 건드리지 않고 메모리 Document와 Element draft를 제거한다.
	void Discard_ActiveDocument();

private:
	// 다음에 추가할 Element 종류인 ImGui session 상태다.
	EFFECT_ELEMENT_KIND m_eSelectedEffectType =
		EFFECT_ELEMENT_KIND::MESH;
	// 최대 128바이트 Effect Asset ID와 문자열 종료 문자를 담는 편집 buffer다.
	std::array<char_t, 129> m_NewAssetId{};
	// 최대 64바이트 표시 이름과 문자열 종료 문자를 담는 편집 buffer다.
	std::array<char_t, 65> m_NewDisplayName{};
	// 최대 128바이트 Element ID와 문자열 종료 문자를 담는 편집 buffer다.
	std::array<char_t, 129> m_NewElementId{};
	// nullopt이면 New Document UI, 값이 있으면 Active Document UI를 그린다.
	std::optional<EFFECT_DOCUMENT_DESC> m_ActiveDocument;
	// 마지막 Document 생성·폐기 결과를 보여주는 session 상태 문구다.
	std::string m_strDocumentStatus;
	// 마지막 Element 추가 검증·commit 결과를 보여주는 session 상태 문구다.
	std::string m_strElementStatus;
};

NS_END
```

### G02-3. C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp

변경 종류: 전체 교체

```cpp
#include "imgui.h"

#include "Effect_Tool.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace
{
	// EFFECT_ELEMENT_KIND 순서와 일치하며 END는 화면에 표시하지 않는다.
	constexpr const char_t* EFFECT_TYPE_LABELS[] = {
		"Mesh", "Sprite", "Particle", "Decal", "Trail"
	};

	// 다른 컴퓨터에서도 같은 값으로 사용할 수 있는 안정적인 ID 문법인지 검증한다.
	bool_t Is_StableAuthoringId(const std::string& strValue)
	{
		if (strValue.empty() || strValue.size() > 128u)
			return false;

		return std::all_of(strValue.begin(), strValue.end(),
			[](const char_t value)
			{
				const bool_t isUpper = value >= 'A' && value <= 'Z';
				const bool_t isLower = value >= 'a' && value <= 'z';
				const bool_t isDigit = value >= '0' && value <= '9';
				return isUpper || isLower || isDigit ||
					value == '_' || value == '.' || value == '-';
			});
	}

	// 표시 이름이 빈 값이거나 공백만으로 이루어졌는지 검증한다.
	bool_t Has_VisibleCharacter(const std::string& strValue)
	{
		return std::any_of(strValue.begin(), strValue.end(),
			[](const char_t value)
			{
				return 0 == std::isspace(
					static_cast<unsigned char>(value));
			});
	}

	// 저장 enum을 ImGui와 Element 목록에 표시할 읽기 전용 문자열로 변환한다.
	const char_t* To_EffectElementKindLabel(
		const Client::EFFECT_ELEMENT_KIND eKind)
	{
		switch (eKind)
		{
		case Client::EFFECT_ELEMENT_KIND::MESH:
			return "Mesh";
		case Client::EFFECT_ELEMENT_KIND::SPRITE:
			return "Sprite";
		case Client::EFFECT_ELEMENT_KIND::PARTICLE:
			return "Particle";
		case Client::EFFECT_ELEMENT_KIND::DECAL:
			return "Decal";
		case Client::EFFECT_ELEMENT_KIND::TRAIL:
			return "Trail";
		case Client::EFFECT_ELEMENT_KIND::END:
		default:
			return "Invalid";
		}
	}
}

void Client::CEffect_Tool::Render()
{
	// ImGui 창이 접혀 Begin이 false를 반환해도 End는 반드시 호출한다.
	ImGui::SetNextWindowSize(ImVec2(720.f, 520.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Effect Tool###LostArkEffectToolG2"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"G2: add one identity-only Effect Element to the active in-memory "
		"document. No file, resource, preview, GPU object, or Animation data "
		"is changed.");

	// optional에 Document가 있는지에 따라 두 패널 중 하나만 그린다.
	if (m_ActiveDocument.has_value())
		Render_ActiveDocumentPanel();
	else
		Render_NewDocumentPanel();

	// Discard 버튼은 같은 frame에 optional을 reset할 수 있으므로 다시 존재 여부를 확인한다.
	ImGui::Separator();
	Render_EffectTypeSelector();
	if (m_ActiveDocument.has_value())
	{
		Render_AddElementPanel();
		Render_ElementList();
	}
	else
	{
		ImGui::TextDisabled(
			"Create a document before adding the selected Element type.");
	}

	ImGui::End();
}

void Client::CEffect_Tool::Render_NewDocumentPanel()
{
	// InputText는 매 프레임 고정 크기 session buffer를 직접 편집한다.
	ImGui::SeparatorText("Data File");
	ImGui::InputText("Effect Asset ID", m_NewAssetId.data(),
		m_NewAssetId.size());
	ImGui::InputText("Display Name", m_NewDisplayName.data(),
		m_NewDisplayName.size());

	if (ImGui::Button("Create Document"))
		Try_CreateDocument();

	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_ActiveDocumentPanel()
{
	ImGui::SeparatorText("Active Document");
	// optional이 Document를 소유할 때만 이 함수가 호출된다.
	const EFFECT_DOCUMENT_DESC& Document = *m_ActiveDocument;
	ImGui::Text("Format Version: %u", Document.iFormatVersion);
	ImGui::Text("Effect Asset ID: %s",
		Document.strEffectAssetId.c_str());
	ImGui::Text("Display Name: %s", Document.strDisplayName.c_str());
	ImGui::Text("Elements: %zu", Document.Elements.size());
	ImGui::TextDisabled("Memory only. Save/Load starts in G3.");

	if (ImGui::Button("Discard Active Document"))
		Discard_ActiveDocument();

	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_EffectTypeSelector()
{
	// radio 선택은 session 상태만 바꾸며 Add Element 전에는 Document가 변하지 않는다.
	ImGui::TextUnformatted("Next Element Type");
	for (int32_t iType = 0;
		iType < static_cast<int32_t>(EFFECT_ELEMENT_KIND::END); ++iType)
	{
		ImGui::SameLine();
		const EFFECT_ELEMENT_KIND eType =
			static_cast<EFFECT_ELEMENT_KIND>(iType);
		if (ImGui::RadioButton(EFFECT_TYPE_LABELS[iType],
			m_eSelectedEffectType == eType))
		{
			m_eSelectedEffectType = eType;
		}
	}
}

void Client::CEffect_Tool::Render_AddElementPanel()
{
	ImGui::SeparatorText("Add Element");
	ImGui::Text("Selected Type: %s",
		To_EffectElementKindLabel(m_eSelectedEffectType));
	ImGui::InputText("Element ID", m_NewElementId.data(),
		m_NewElementId.size());

	if (ImGui::Button("Add Element"))
		Try_AddElement();

	if (!m_strElementStatus.empty())
		ImGui::TextWrapped("%s", m_strElementStatus.c_str());
}

void Client::CEffect_Tool::Render_ElementList() const
{
	ImGui::SeparatorText("Current Elements");
	if (!m_ActiveDocument.has_value())
	{
		ImGui::TextDisabled("No active document.");
		return;
	}

	const std::vector<EFFECT_ELEMENT_DESC>& Elements =
		m_ActiveDocument->Elements;
	if (Elements.empty())
	{
		ImGui::TextDisabled("No Elements have been added.");
		return;
	}

	constexpr ImGuiTableFlags TABLE_FLAGS =
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_SizingStretchProp;
	if (!ImGui::BeginTable("EffectElementList", 2, TABLE_FLAGS))
		return;

	ImGui::TableSetupColumn("Element ID");
	ImGui::TableSetupColumn("Kind");
	ImGui::TableHeadersRow();
	for (const EFFECT_ELEMENT_DESC& Element : Elements)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(Element.strElementId.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(
			To_EffectElementKindLabel(Element.eKind));
	}
	ImGui::EndTable();
}

bool_t Client::CEffect_Tool::Try_CreateDocument()
{
	// 기존 메모리 Document를 암묵적으로 덮어쓰지 않는다.
	if (m_ActiveDocument.has_value())
	{
		m_strDocumentStatus =
			"Discard the active document before creating another one.";
		return false;
	}

	// 부분적으로 Document를 수정하지 않도록 ImGui buffer를 먼저 문자열로 복사한다.
	const std::string strAssetId = m_NewAssetId.data();
	const std::string strDisplayName = m_NewDisplayName.data();
	if (!Is_StableAuthoringId(strAssetId))
	{
		m_strDocumentStatus =
			"Effect Asset ID must be 1-128 ASCII letters, digits, '.', '_' or '-'.";
		return false;
	}
	if (!Has_VisibleCharacter(strDisplayName))
	{
		m_strDocumentStatus = "Display Name must not be blank.";
		return false;
	}

	// 검증을 통과한 후보 Document를 local 값으로 staging한다.
	EFFECT_DOCUMENT_DESC StagedDocument;
	StagedDocument.strEffectAssetId = strAssetId;
	StagedDocument.strDisplayName = strDisplayName;

	// 모든 검증이 성공한 뒤 Active Document에 한 번만 commit한다.
	m_ActiveDocument = std::move(StagedDocument);
	m_NewAssetId.fill('\0');
	m_NewDisplayName.fill('\0');
	m_NewElementId.fill('\0');
	m_strElementStatus.clear();
	m_strDocumentStatus =
		"Created in memory. This document has not been saved.";
	return true;
}

bool_t Client::CEffect_Tool::Try_AddElement()
{
	// Element는 반드시 현재 Active Document 한 건에만 추가한다.
	if (!m_ActiveDocument.has_value())
	{
		m_strElementStatus =
			"Create a document before adding an Element.";
		return false;
	}

	// END는 UI에 표시하지 않으며 유효한 Element kind로 commit할 수 없다.
	if (EFFECT_ELEMENT_KIND::END == m_eSelectedEffectType)
	{
		m_strElementStatus = "Select a valid Element type.";
		return false;
	}

	// ImGui buffer를 local 문자열로 복사하고 stable ID 문법을 검증한다.
	const std::string strElementId = m_NewElementId.data();
	if (!Is_StableAuthoringId(strElementId))
	{
		m_strElementStatus =
			"Element ID must be 1-128 ASCII letters, digits, '.', '_' or '-'.";
		return false;
	}

	// 같은 Document 안에서는 stable Element ID가 중복될 수 없다.
	const bool_t bDuplicate = std::any_of(
		m_ActiveDocument->Elements.begin(),
		m_ActiveDocument->Elements.end(),
		[&strElementId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == strElementId;
		});
	if (bDuplicate)
	{
		m_strElementStatus =
			"Element ID already exists in the active document.";
		return false;
	}

	// 기존 Active Document를 직접 수정하지 않고 전체 후보를 local copy로 staging한다.
	EFFECT_DOCUMENT_DESC StagedDocument = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC StagedElement;
	StagedElement.strElementId = strElementId;
	StagedElement.eKind = m_eSelectedEffectType;
	StagedDocument.Elements.push_back(std::move(StagedElement));

	// 모든 검증과 후보 구성이 성공한 뒤 optional의 현재 Document를 한 번에 교체한다.
	m_ActiveDocument = std::move(StagedDocument);
	m_NewElementId.fill('\0');
	m_strElementStatus =
		"Added to the in-memory document. Nothing has been saved or previewed.";
	return true;
}

void Client::CEffect_Tool::Discard_ActiveDocument()
{
	// reset은 optional이 소유한 Document와 그 안의 Element 값만 제거한다.
	m_ActiveDocument.reset();
	m_NewElementId.fill('\0');
	m_strElementStatus.clear();
	m_strDocumentStatus =
		"Discarded the in-memory document. No file was changed.";
}
```

### G02-4. C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1

변경 종류: 블록 전체 교체

적용 위치: 현재 `$effectDocumentHeader` 선언부터 `effect.g1-document-boundary` `Add-Check`까지를
아래 블록으로 교체한다.

```powershell
	$effectDocumentHeader = Get-Content -LiteralPath 'Client\Public\Effect_AuthoringDocument.h' -Raw
	$effectToolHeader = Get-Content -LiteralPath 'Client\Public\Effect_Tool.h' -Raw
	$effectToolSource = Get-Content -LiteralPath 'Client\Private\Effect_Tool.cpp' -Raw
	$clientEntrySource = Get-Content -LiteralPath 'Client\Default\Client.cpp' -Raw
	$engineImGuiSource = Get-Content -LiteralPath 'Engine\Private\ImGuiLayer.cpp' -Raw
	$clientProjectSource = Get-Content -LiteralPath 'Client\Default\Client.vcxproj' -Raw
	$removedEffectPaths = @(
		'Client\Public\Effect_Types.h',
		'Client\Public\Effect_AssetIO.h',
		'Client\Private\Effect_AssetIO.cpp',
		'Client\Public\Effect_ParticleSimulator.h',
		'Client\Private\Effect_ParticleSimulator.cpp',
		'Client\Public\Effect_Runtime.h',
		'Client\Private\Effect_Runtime.cpp',
		'Client\Public\Effect_ResourceCatalog.h',
		'Client\Private\Effect_ResourceCatalog.cpp')
	$removedEffectPathHits = @($removedEffectPaths |
		Where-Object { Test-Path -LiteralPath $_ })
	$authoredEffectFiles = @(Get-ChildItem -LiteralPath 'Data\Effects\Authored' -Recurse -File -ErrorAction SilentlyContinue)
	$effectIntakeFiles = @(Get-ChildItem -LiteralPath 'Tools\EffectResourceIntake' -Recurse -File -ErrorAction SilentlyContinue)
	$effectShaderFiles = @(Get-ChildItem -LiteralPath 'Client\Bin\ShaderFiles' -File -Filter 'Shader_Effect*' -ErrorAction SilentlyContinue)
	$legacyEffectSymbolHits = @($clientSourceFiles | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC')
	$legacyEffectProjectHits = @($clientProjectSource | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|Shader_Effect|Data\\Effects\\Authored')
	$legacyEffectEntry =
		$clientEntrySource -match 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC|--effect-' -or
		$engineImGuiSource -match '--effect-'
	$effectG2DocumentShape =
		$effectDocumentHeader -match 'EFFECT_AUTHORING_FORMAT_VERSION\s*=\s*1u' -and
		$effectDocumentHeader -match 'enum class EFFECT_ELEMENT_KIND[\s\S]*MESH,[\s\S]*SPRITE,[\s\S]*PARTICLE,[\s\S]*DECAL,[\s\S]*TRAIL,[\s\S]*END' -and
		$effectDocumentHeader -match 'struct EFFECT_ELEMENT_DESC[\s\S]*strElementId[\s\S]*eKind' -and
		$effectDocumentHeader -notmatch 'strResourceId' -and
		$effectDocumentHeader -match 'struct EFFECT_DOCUMENT_DESC[\s\S]*strEffectAssetId[\s\S]*strDisplayName[\s\S]*Elements' -and
		$effectToolHeader -match 'optional<EFFECT_DOCUMENT_DESC>\s+m_ActiveDocument' -and
		$effectToolHeader -match 'm_eSelectedEffectType\s*=\s*EFFECT_ELEMENT_KIND::MESH' -and
		$effectToolHeader -match 'array<char_t,\s*129>\s+m_NewElementId' -and
		$effectToolHeader -match 'Try_AddElement' -and
		$effectToolSource -match 'Is_StableAuthoringId' -and
		$effectToolSource -match 'Try_CreateDocument' -and
		$effectToolSource -match 'Try_AddElement' -and
		$effectToolSource -match 'EFFECT_DOCUMENT_DESC\s+StagedDocument\s*=\s*\*m_ActiveDocument' -and
		$effectToolSource -match 'Element\.strElementId\s*==\s*strElementId' -and
		$effectToolSource -match '"Mesh",\s*"Sprite",\s*"Particle",\s*"Decal",\s*"Trail"' -and
		$effectToolSource -match 'LostArk Effect Tool###LostArkEffectToolG2' -and
		$effectToolSource -notmatch 'filesystem|ifstream|ofstream|GetOpenFileName|ID3D11'
	Add-Check 'effect.g2-element-boundary' (
		$removedEffectPathHits.Count -eq 0 -and
		$authoredEffectFiles.Count -eq 0 -and
		$effectIntakeFiles.Count -eq 0 -and
		$effectShaderFiles.Count -eq 0 -and
		$legacyEffectSymbolHits.Count -eq 0 -and
		$legacyEffectProjectHits.Count -eq 0 -and
		-not $legacyEffectEntry -and
		$effectG2DocumentShape) "paths=$($removedEffectPathHits.Count) authored=$($authoredEffectFiles.Count) intake=$($effectIntakeFiles.Count) shaders=$($effectShaderFiles.Count) symbols=$($legacyEffectSymbolHits.Count) project=$($legacyEffectProjectHits.Count) entry=$legacyEffectEntry document=$effectG2DocumentShape"
```

### G02-5. 프로젝트 등록 확인 코드

G02는 새 C++ 파일을 만들지 않으므로 아래 현재 등록을 그대로 유지한다.

`C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj`

```xml
    <ClInclude Include="..\Public\Effect_AuthoringDocument.h" />
    <ClInclude Include="..\public\Effect_Tool.h" />
    <ClCompile Include="..\private\Effect_Tool.cpp" />
```

`C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters`

```xml
    <ClCompile Include="..\private\Effect_Tool.cpp">
      <Filter>03. Tools\02. Effect</Filter>
    </ClCompile>
    <ClInclude Include="..\public\Effect_Tool.h">
      <Filter>03. Tools\02. Effect</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Effect_AuthoringDocument.h">
      <Filter>03. Tools\02. Effect</Filter>
    </ClInclude>
```

### G02-6. 직접 작성 순서

1. `Effect_AuthoringDocument.h`를 G02 전체 코드로 교체해 `strResourceId`를 제거한다.
2. `Effect_Tool.h`를 G02 전체 코드로 교체해 Element ID buffer, Element status, Add/List 계약을 추가한다.
3. `Effect_Tool.cpp`를 G02 전체 코드로 교체한다.
4. ProjectAudit의 G01 Effect 블록을 G02 전체 블록으로 교체한다.
5. `.vcxproj`와 `.filters`는 수정하지 않고 현재 세 항목이 각각 한 번만 등록됐는지 확인한다.

### G02-7. 자동 검증

```powershell
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj' -Raw) | Out-Null
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj.filters' -Raw) | Out-Null

rg -n "strResourceId|Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC" Client Engine Tools
rg -n "Effect_AuthoringDocument.h|Effect_Tool.h|Effect_Tool.cpp" Client/Default/Client.vcxproj Client/Default/Client.vcxproj.filters

$lostArkMsBuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
  -latest -products * -requires Microsoft.Component.MSBuild `
  -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
& $lostArkMsBuild Client\Default\Client.vcxproj `
  /p:Configuration=Debug /p:Platform=x64 /m

powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

기대 결과:

```text
Client x64 Debug compile/link                         PASS
effect.g2-element-boundary                           PASS
strResourceId active source                         0건
레거시 Effect source/runtime/shader/project consumer 0건
Effect C++ project/filter 등록                       각 1건
ProjectAudit 전체의 알려진 잔여 실패                 asset-lock.inventory 한 건
git diff --check                                     PASS
```

### G02-8. Runtime smoke

Debug Client의 작업 디렉터리를 `C:/Users/user/Desktop/LostArk/Client/Default`로 두고 실행한다.

```text
F1 -> Effect Tool
Effect Asset ID = dimensionmaster.altv.portal_open
Display Name = DimensionMaster Alt-V Portal
Create Document
-> Elements: 0

Next Element Type = Particle
Element ID = portal_sparks
-> 입력만 했을 때 Elements: 0
Add Element
-> portal_sparks / Particle, Elements: 1

Element ID = portal_sparks
Add Element
-> 중복 실패, Elements: 1, 입력값 보존

Next Element Type = Mesh
Element ID = portal_mesh
Add Element
-> portal_mesh / Mesh, Elements: 2

Next Element Type = Trail
-> 기존 portal_sparks / Particle과 portal_mesh / Mesh는 불변

빈 값, slash, backslash, space, 129바이트 Element ID
-> 모두 거부, 기존 Document와 Elements 불변

Discard Active Document
-> Active Document와 Elements 제거, Element 입력과 status 초기화
```

실행 전후 `Data`, `Client/Bin/Resources`, Animation authoring 파일, Shader 파일 변화가 0건인지 확인한다.
G03은 이 G02 smoke가 끝난 뒤 `EffectAssetId + DisplayName + Element ID + Kind`만 JSON으로 저장하고
`parse -> validate -> stage -> commit`으로 다시 읽는 별도 전체 코드 PLAN에서 시작한다.
