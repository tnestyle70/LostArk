# Effect Tool G02-G04 전체 코드 PLAN

기준일: 2026-08-04

기준 브랜치: `codex/effect-tool-reboot`

작성 범위: G02 stable Element 추가 기준선, G03 JSON Save/Load, G04 typed resource catalog/input

현재 상태: 사용자의 G03·G04 동시 반영 요청에 따라 두 단계 구현과 자동 검증까지 진행했다.
Debug Client 수동 smoke는 `abort() has been called` 시작 오류로 실패했으며 PASS로 닫지 않는다.

이 문서는 각 G 적용 후 `Effect_AuthoringDocument.h`, `Effect_Tool.h`, `Effect_Tool.cpp`의 처음부터 끝까지 전체 코드를 보존한다. 대화 설명은 해당 G의 추가·변경분만 다루지만 이 PLAN의 코드 전문은 줄이지 않는다.

## 본질 우선 구현 원칙

- 사용자가 처음부터 구조를 이해하며 반영할 수 있도록 각 G는 그 G의 소비자가 실제로 쓰는 최소 책임만 추가한다.
- 한 번만 쓰는 검사를 별도 helper로 계속 쪼개지 않는다. `CEffect_Tool`의 상태를 변경하지 않는 변환·전체 검증·공통 경로 계산만 anonymous namespace helper로 둔다.
- `Data/Effects/Authored`는 팀 내부 authoring 데이터지만 수동 편집과 중간 저장 실패는 정상 실패 입력이다. 따라서 JSON 필드 type, format version, stable ID, Element kind, 중복 ID 검증과 `stage -> commit`은 제거하지 않는다.
- 범용 JSON 문법과 16 MiB 제한은 기존 `CDataJson`에 맡긴다. Effect Tool에서 exact-key 검사, 별도 정수 판별, 중복 파일 크기 제한 helper를 만들지 않는다.
- Save는 `validate -> serialize -> temporary write -> atomic replace`까지만 수행한다. 저장 직후 다시 parse해 동등성을 비교하는 이중 round trip은 G03에서 요구하지 않는다.
- G03이 build, Audit, runtime smoke를 통과하기 전에는 G04 resource catalog/binding 코드를 섞지 않는다.

소스 코드는 사용자가 직접 반영한다. 이 계획은 `CharacterPreviewPanel`, `Animation_Tool`, Character Select Arena 미커밋 변경을 수정·되돌림·stage·commit하지 않는다.


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

## G03 — Effect authoring JSON Save/Load v1

G03은 G02의 `Element ID + Kind` Document를 `Data/Effects/Authored/<EffectAssetId>.effect.json`에 저장하고 같은 stable ID로 다시 읽는다. Load는 `parse -> validate -> stage -> commit`, Save는 `validate -> serialize -> temporary write -> atomic replace`만 지키며 실패하면 Active Document와 기존 파일을 보존한다. 팀 내부 authoring 파일이라는 실제 사용 범위에 맞춰 exact-key, 별도 number/file-size helper, 저장 후 재parse·동등성 비교는 추가하지 않는다.

### G03-1. 파일별 한 줄 역할

| 파일 | 역할 |
|---|---|
| `Client/Public/Effect_AuthoringDocument.h` | G02의 ID + Kind 저장 계약을 v1 정본으로 유지한다. |
| `Client/Public/Effect_Tool.h` | Save/Load 명령을 Active Document 수명 안에 추가한다. |
| `Client/Private/Effect_Tool.cpp` | 최소 JSON parse/validate/stage/commit과 임시 파일 원자 저장을 구현한다. |
| `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | v1 JSON 경계와 제거된 Effect 런타임의 부재를 자동 감사한다. |

### G03-2. C:/Users/user/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h

변경 종류: 적용 후 전체 코드

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
	// 파일 호환성을 검증하는 schema version이다.
	uint32_t iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	// 저장과 후속 runtime 연결에 사용하는 Effect Asset의 안정적인 ID다.
	std::string strEffectAssetId;
	// 제작자에게 표시하는 사람이 읽을 수 있는 이름이다.
	std::string strDisplayName;
	// 이 Effect Asset을 구성하는 Element들을 값으로 강하게 소유한다.
	std::vector<EFFECT_ELEMENT_DESC> Elements;
};

NS_END
```

### G03-3. C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h

변경 종류: 적용 후 전체 코드

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
	// Active Document가 없을 때 Create와 stable ID Load 입력을 그린다.
	void Render_NewDocumentPanel();
	// Active Document 요약과 Save, Discard 명령을 그린다.
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
	// Active Document를 검증하고 임시 JSON을 쓴 뒤 원자적으로 저장한다.
	bool_t Try_SaveDocument();
	// stable Effect Asset ID의 JSON을 parse, validate, stage한 뒤 한 번에 commit한다.
	bool_t Try_LoadDocument();
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
	// 마지막 Document create/load/save/discard 결과를 보여주는 session 상태다.
	std::string m_strDocumentStatus;
	// 마지막 Element 추가 검증·commit 결과를 보여주는 session 상태다.
	std::string m_strElementStatus;
};

NS_END
```

### G03-4. C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp

변경 종류: 적용 후 전체 코드

```cpp
#include "imgui.h"

#include "Effect_Tool.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>

namespace
{
	constexpr const char_t* EFFECT_DOCUMENT_SCHEMA =
		"lostark.effect-authoring";

	// EFFECT_ELEMENT_KIND 순서와 일치하며 END는 화면에 표시하지 않는다.
	constexpr const char_t* EFFECT_TYPE_LABELS[] = {
		"Mesh", "Sprite", "Particle", "Decal", "Trail"
	};
	// 같은 enum 순서의 JSON 저장 token이다.
	constexpr const char_t* EFFECT_KIND_TOKENS[] = {
		"mesh", "sprite", "particle", "decal", "trail"
	};

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

	bool_t Has_VisibleCharacter(const std::string& strValue)
	{
		return std::any_of(strValue.begin(), strValue.end(),
			[](const char_t value)
			{
				return 0 == std::isspace(
					static_cast<unsigned char>(value));
			});
	}

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

	bool_t Try_ParseEffectElementKind(
		const std::string& strValue,
		Client::EFFECT_ELEMENT_KIND& eOutKind)
	{
		for (int32_t iKind = 0;
			iKind < static_cast<int32_t>(
				Client::EFFECT_ELEMENT_KIND::END); ++iKind)
		{
			if (EFFECT_KIND_TOKENS[iKind] == strValue)
			{
				eOutKind = static_cast<Client::EFFECT_ELEMENT_KIND>(iKind);
				return true;
			}
		}
		return false;
	}

	bool_t Validate_EffectDocument(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError)
	{
		if (Client::EFFECT_AUTHORING_FORMAT_VERSION !=
			Document.iFormatVersion)
		{
			strOutError = "Unsupported Effect document version.";
			return false;
		}
		if (!Is_StableAuthoringId(Document.strEffectAssetId))
		{
			strOutError = "Effect Asset ID is invalid.";
			return false;
		}
		if (Document.strDisplayName.size() > 64u ||
			!Has_VisibleCharacter(Document.strDisplayName))
		{
			strOutError = "Display Name must be 1-64 bytes and not blank.";
			return false;
		}
		for (size_t iElement = 0;
			iElement < Document.Elements.size(); ++iElement)
		{
			const Client::EFFECT_ELEMENT_DESC& Element =
				Document.Elements[iElement];
			if (!Is_StableAuthoringId(Element.strElementId))
			{
				strOutError = "Element ID is invalid.";
				return false;
			}
			if (Element.eKind >= Client::EFFECT_ELEMENT_KIND::END)
			{
				strOutError = "Element kind is invalid.";
				return false;
			}
			const bool_t bDuplicate = std::any_of(
				Document.Elements.begin(),
				Document.Elements.begin() + iElement,
				[&Element](const Client::EFFECT_ELEMENT_DESC& Previous)
				{
					return Previous.strElementId == Element.strElementId;
				});
			if (bDuplicate)
			{
				strOutError = "Element ID is duplicated.";
				return false;
			}
		}
		strOutError.clear();
		return true;
	}

	std::filesystem::path Resolve_EffectDocumentPath(
		const std::string& strEffectAssetId)
	{
		if (!Is_StableAuthoringId(strEffectAssetId))
			return {};

		return Client::CProjectDataRoot::Resolve(
			std::filesystem::path(L"Effects") / L"Authored" /
			std::filesystem::path(strEffectAssetId + ".effect.json"));
	}

	bool_t Parse_EffectDocumentJson(
		const std::string& strText,
		Client::EFFECT_DOCUMENT_DESC& OutDocument,
		std::string& strOutError)
	{
		Client::DATA_JSON_VALUE Root;
		if (!Client::CDataJson::Parse(strText, Root, strOutError) ||
			!Root.Is_Object())
		{
			if (strOutError.empty())
				strOutError = "Effect document root must be an object.";
			return false;
		}

		const Client::DATA_JSON_VALUE* pSchema = Root.Find("schema");
		const Client::DATA_JSON_VALUE* pVersion = Root.Find("version");
		const Client::DATA_JSON_VALUE* pAssetId = Root.Find("effectAssetId");
		const Client::DATA_JSON_VALUE* pDisplayName = Root.Find("displayName");
		const Client::DATA_JSON_VALUE* pElements = Root.Find("elements");
		if (nullptr == pSchema || !pSchema->Is_String() ||
			EFFECT_DOCUMENT_SCHEMA != pSchema->Get_String() ||
			nullptr == pVersion || !pVersion->Is_Number() ||
			pVersion->Get_Number() !=
				static_cast<double>(Client::EFFECT_AUTHORING_FORMAT_VERSION) ||
			nullptr == pAssetId || !pAssetId->Is_String() ||
			nullptr == pDisplayName || !pDisplayName->Is_String() ||
			nullptr == pElements || !pElements->Is_Array())
		{
			strOutError = "Effect document field types or schema/version are invalid.";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC StagedDocument;
		StagedDocument.iFormatVersion =
			Client::EFFECT_AUTHORING_FORMAT_VERSION;
		StagedDocument.strEffectAssetId = pAssetId->Get_String();
		StagedDocument.strDisplayName = pDisplayName->Get_String();
		StagedDocument.Elements.reserve(pElements->Get_Array().size());

		for (const Client::DATA_JSON_VALUE& ElementValue :
			pElements->Get_Array())
		{
			if (!ElementValue.Is_Object())
			{
				strOutError = "Effect Element must be an object.";
				return false;
			}

			const Client::DATA_JSON_VALUE* pId = ElementValue.Find("id");
			const Client::DATA_JSON_VALUE* pKind = ElementValue.Find("kind");
			Client::EFFECT_ELEMENT_DESC StagedElement;
			if (nullptr == pId || !pId->Is_String() ||
				nullptr == pKind || !pKind->Is_String() ||
				!Try_ParseEffectElementKind(
					pKind->Get_String(), StagedElement.eKind))
			{
				strOutError = "Effect Element ID or kind is invalid.";
				return false;
			}
			StagedElement.strElementId = pId->Get_String();
			StagedDocument.Elements.push_back(std::move(StagedElement));
		}

		if (!Validate_EffectDocument(StagedDocument, strOutError))
			return false;

		OutDocument = std::move(StagedDocument);
		strOutError.clear();
		return true;
	}

	std::string Serialize_EffectDocumentJson(
		const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		std::ostringstream Output;
		Output << "{\n"
			<< "  \"schema\": \"" << EFFECT_DOCUMENT_SCHEMA << "\",\n"
			<< "  \"version\": " << Document.iFormatVersion << ",\n"
			<< "  \"effectAssetId\": \""
			<< Client::CDataJson::Escape(Document.strEffectAssetId) << "\",\n"
			<< "  \"displayName\": \""
			<< Client::CDataJson::Escape(Document.strDisplayName) << "\",\n"
			<< "  \"elements\": [";

		for (size_t iElement = 0;
			iElement < Document.Elements.size(); ++iElement)
		{
			const Client::EFFECT_ELEMENT_DESC& Element =
				Document.Elements[iElement];
			Output << (0u == iElement ? "\n" : ",\n")
				<< "    { \"id\": \""
				<< Client::CDataJson::Escape(Element.strElementId)
				<< "\", \"kind\": \""
				<< EFFECT_KIND_TOKENS[
					static_cast<size_t>(Element.eKind)] << "\" }";
		}
		if (!Document.Elements.empty())
			Output << "\n  ";
		Output << "]\n}\n";
		return Output.str();
	}

}

void Client::CEffect_Tool::Render()
{
	ImGui::SetNextWindowSize(ImVec2(760.f, 560.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Effect Tool###LostArkEffectToolG3"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"G3: save and load the identity-only Effect Document as versioned JSON. "
		"Resource binding, preview, GPU objects, and Animation data are unchanged.");

	if (m_ActiveDocument.has_value())
		Render_ActiveDocumentPanel();
	else
		Render_NewDocumentPanel();

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
			"Create or load a document before adding an Element.");
	}

	ImGui::End();
}

void Client::CEffect_Tool::Render_NewDocumentPanel()
{
	ImGui::SeparatorText("Data File");
	ImGui::InputText("Effect Asset ID", m_NewAssetId.data(),
		m_NewAssetId.size());
	ImGui::InputText("Display Name", m_NewDisplayName.data(),
		m_NewDisplayName.size());

	if (ImGui::Button("Create Document"))
		Try_CreateDocument();
	ImGui::SameLine();
	if (ImGui::Button("Load Document"))
		Try_LoadDocument();

	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_ActiveDocumentPanel()
{
	ImGui::SeparatorText("Active Document");
	const EFFECT_DOCUMENT_DESC& Document = *m_ActiveDocument;
	ImGui::Text("Format Version: %u", Document.iFormatVersion);
	ImGui::Text("Effect Asset ID: %s",
		Document.strEffectAssetId.c_str());
	ImGui::Text("Display Name: %s", Document.strDisplayName.c_str());
	ImGui::Text("Elements: %zu", Document.Elements.size());

	if (ImGui::Button("Save Active Document"))
		Try_SaveDocument();
	ImGui::SameLine();
	if (ImGui::Button("Discard Active Document"))
		Discard_ActiveDocument();

	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_EffectTypeSelector()
{
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
	if (m_ActiveDocument.has_value())
	{
		m_strDocumentStatus =
			"Discard the active document before creating another one.";
		return false;
	}

	const std::string strAssetId = m_NewAssetId.data();
	const std::string strDisplayName = m_NewDisplayName.data();
	if (!Is_StableAuthoringId(strAssetId))
	{
		m_strDocumentStatus =
			"Effect Asset ID must be 1-128 ASCII letters, digits, '.', '_' or '-'.";
		return false;
	}
	if (strDisplayName.size() > 64u ||
		!Has_VisibleCharacter(strDisplayName))
	{
		m_strDocumentStatus =
			"Display Name must be 1-64 bytes and not blank.";
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument;
	StagedDocument.strEffectAssetId = strAssetId;
	StagedDocument.strDisplayName = strDisplayName;
	std::string strValidationError;
	if (!Validate_EffectDocument(StagedDocument, strValidationError))
	{
		m_strDocumentStatus = strValidationError;
		return false;
	}

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
	if (!m_ActiveDocument.has_value())
	{
		m_strElementStatus =
			"Create or load a document before adding an Element.";
		return false;
	}
	if (EFFECT_ELEMENT_KIND::END == m_eSelectedEffectType)
	{
		m_strElementStatus = "Select a valid Element type.";
		return false;
	}

	const std::string strElementId = m_NewElementId.data();
	if (!Is_StableAuthoringId(strElementId))
	{
		m_strElementStatus =
			"Element ID must be 1-128 ASCII letters, digits, '.', '_' or '-'.";
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC StagedElement;
	StagedElement.strElementId = strElementId;
	StagedElement.eKind = m_eSelectedEffectType;
	StagedDocument.Elements.push_back(std::move(StagedElement));

	std::string strValidationError;
	if (!Validate_EffectDocument(StagedDocument, strValidationError))
	{
		m_strElementStatus = strValidationError;
		return false;
	}

	m_ActiveDocument = std::move(StagedDocument);
	m_NewElementId.fill('\0');
	m_strElementStatus =
		"Added to the in-memory document. Save to persist this change.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SaveDocument()
{
	if (!m_ActiveDocument.has_value())
	{
		m_strDocumentStatus = "There is no active document to save.";
		return false;
	}

	std::string strError;
	if (!Validate_EffectDocument(*m_ActiveDocument, strError))
	{
		m_strDocumentStatus = "Save rejected: " + strError;
		return false;
	}

	const std::filesystem::path DestinationPath =
		Resolve_EffectDocumentPath(m_ActiveDocument->strEffectAssetId);
	if (DestinationPath.empty())
	{
		m_strDocumentStatus = "Save path escaped the project Data root.";
		return false;
	}

	std::error_code DirectoryError;
	std::filesystem::create_directories(
		DestinationPath.parent_path(), DirectoryError);
	if (DirectoryError)
	{
		m_strDocumentStatus =
			"Failed to create Data/Effects/Authored.";
		return false;
	}

	std::filesystem::path TemporaryPath = DestinationPath;
	TemporaryPath += L".tmp";
	const std::string strSerialized =
		Serialize_EffectDocumentJson(*m_ActiveDocument);
	std::ofstream Output(TemporaryPath,
		std::ios::binary | std::ios::trunc);
	if (!Output)
	{
		m_strDocumentStatus = "Failed to create temporary Effect document.";
		return false;
	}
	Output.write(strSerialized.data(),
		static_cast<std::streamsize>(strSerialized.size()));
	Output.close();
	if (!Output)
	{
		std::error_code IgnoreError;
		std::filesystem::remove(TemporaryPath, IgnoreError);
		m_strDocumentStatus =
			"Failed to write temporary Effect document; previous file preserved.";
		return false;
	}

	if (!MoveFileExW(TemporaryPath.c_str(), DestinationPath.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		std::error_code IgnoreError;
		std::filesystem::remove(TemporaryPath, IgnoreError);
		m_strDocumentStatus =
			"Failed to replace Effect document; previous file preserved.";
		return false;
	}

	m_strDocumentStatus =
		"Saved Data/Effects/Authored/" +
		m_ActiveDocument->strEffectAssetId + ".effect.json.";
	return true;
}

bool_t Client::CEffect_Tool::Try_LoadDocument()
{
	if (m_ActiveDocument.has_value())
	{
		m_strDocumentStatus =
			"Discard the active document before loading another one.";
		return false;
	}

	const std::string strRequestedAssetId = m_NewAssetId.data();
	if (!Is_StableAuthoringId(strRequestedAssetId))
	{
		m_strDocumentStatus =
			"Enter a valid Effect Asset ID before loading.";
		return false;
	}

	const std::filesystem::path SourcePath =
		Resolve_EffectDocumentPath(strRequestedAssetId);
	if (SourcePath.empty())
	{
		m_strDocumentStatus = "Load path escaped the project Data root.";
		return false;
	}

	std::ifstream Input(SourcePath, std::ios::binary);
	if (!Input)
	{
		m_strDocumentStatus = "Effect document does not exist or cannot be read.";
		return false;
	}
	std::ostringstream Buffer;
	Buffer << Input.rdbuf();
	if (Input.bad())
	{
		m_strDocumentStatus = "Failed while reading Effect document.";
		return false;
	}

	std::string strError;
	EFFECT_DOCUMENT_DESC StagedDocument;
	if (!Parse_EffectDocumentJson(
		Buffer.str(), StagedDocument, strError))
	{
		m_strDocumentStatus =
			"Load rejected; active document preserved: " + strError;
		return false;
	}
	if (StagedDocument.strEffectAssetId != strRequestedAssetId)
	{
		m_strDocumentStatus =
			"Load rejected: file name ID and document ID do not match.";
		return false;
	}

	m_ActiveDocument = std::move(StagedDocument);
	m_NewAssetId.fill('\0');
	m_NewDisplayName.fill('\0');
	m_NewElementId.fill('\0');
	m_strElementStatus.clear();
	m_strDocumentStatus = "Loaded and committed the Effect document.";
	return true;
}

void Client::CEffect_Tool::Discard_ActiveDocument()
{
	m_ActiveDocument.reset();
	m_NewElementId.fill('\0');
	m_strElementStatus.clear();
	m_strDocumentStatus =
		"Discarded the in-memory document. No file was changed.";
}
```

### G03-5. 저장 JSON 전체 형식

적용 위치: `Data/Effects/Authored/<EffectAssetId>.effect.json`

```json
{
  "schema": "lostark.effect-authoring",
  "version": 1,
  "effectAssetId": "training_hit_a",
  "displayName": "Training Hit A",
  "elements": [
    { "id": "mesh_core", "kind": "mesh" },
    { "id": "spark_sheet", "kind": "sprite" },
    { "id": "dust_particles", "kind": "particle" },
    { "id": "ground_mark", "kind": "decal" },
    { "id": "weapon_trail", "kind": "trail" }
  ]
}
```

이 JSON은 형식 정본 예시이며 PLAN 작성만으로 실제 Data 파일을 추가하지 않는다. 실제 제작 파일을 Git에 넣을 때는 `Client.vcxproj`의 `96.DataFiles` `<None>`과 대응 filter를 같은 변경 단위에 등록한다.

### G03-6. C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1

변경 종류: 기존 `$effectDocumentHeader`부터 `effect.g1-document-boundary` Add-Check까지 아래 블록으로 완전 교체

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
	$unexpectedAuthoredEffectFiles = @($authoredEffectFiles |
		Where-Object { $_.Name -notmatch '^[A-Za-z0-9_.-]+\.effect\.json$' })
	$effectIntakeFiles = @(Get-ChildItem -LiteralPath 'Tools\EffectResourceIntake' -Recurse -File -ErrorAction SilentlyContinue)
	$effectShaderFiles = @(Get-ChildItem -LiteralPath 'Client\Bin\ShaderFiles' -File -Filter 'Shader_Effect*' -ErrorAction SilentlyContinue)
	$legacyEffectSymbolHits = @($clientSourceFiles | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC')
	$legacyEffectProjectHits = @($clientProjectSource | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|Shader_Effect')
	$legacyEffectEntry =
		$clientEntrySource -match 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC|--effect-' -or
		$engineImGuiSource -match '--effect-'
	$effectG3DocumentShape =
		$effectDocumentHeader -match 'EFFECT_AUTHORING_FORMAT_VERSION\s*=\s*1u' -and
		$effectDocumentHeader -match 'struct EFFECT_ELEMENT_DESC[\s\S]*strElementId[\s\S]*eKind' -and
		$effectDocumentHeader -notmatch 'strResourceId|ResourceBinding' -and
		$effectToolHeader -match 'Try_SaveDocument' -and
		$effectToolHeader -match 'Try_LoadDocument' -and
		$effectToolSource -match 'CProjectDataRoot::Resolve' -and
		$effectToolSource -match 'CDataJson::Parse' -and
		$effectToolSource -match 'std::ofstream Output\(TemporaryPath' -and
		$effectToolSource -match 'MoveFileExW' -and
		$effectToolSource -match 'Parse_EffectDocumentJson' -and
		$effectToolSource -notmatch 'CRuntimeAssetRoot|ID3D11'
	Add-Check 'effect.g3-save-load-boundary' (
		$removedEffectPathHits.Count -eq 0 -and
		$unexpectedAuthoredEffectFiles.Count -eq 0 -and
		$effectIntakeFiles.Count -eq 0 -and
		$effectShaderFiles.Count -eq 0 -and
		$legacyEffectSymbolHits.Count -eq 0 -and
		$legacyEffectProjectHits.Count -eq 0 -and
		-not $legacyEffectEntry -and
		$effectG3DocumentShape) "paths=$($removedEffectPathHits.Count) authoredUnexpected=$($unexpectedAuthoredEffectFiles.Count) intake=$($effectIntakeFiles.Count) shaders=$($effectShaderFiles.Count) symbols=$($legacyEffectSymbolHits.Count) project=$($legacyEffectProjectHits.Count) entry=$legacyEffectEntry document=$effectG3DocumentShape"
```

### G03-7. 직접 작성 순서와 종료 검증

1. `Effect_AuthoringDocument.h`의 v1 ID + Kind 계약을 그대로 둔다.
2. `Effect_Tool.h`에 Save/Load 선언만 추가한다.
3. `Effect_Tool.cpp`에 최소 JSON 변환·검증 helper와 temporary write/atomic replace를 작성한다.
4. New/Active 패널에 Load/Save 버튼을 연결한다.
5. ProjectAudit의 기존 Effect 블록을 G03 블록으로 교체한다.

```powershell
git diff --check
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj' -Raw) | Out-Null
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj.filters' -Raw) | Out-Null
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
```

Runtime smoke는 `Create -> Element 5종 Add -> Save -> Discard -> 같은 ID Load` 후 순서와 ID/Kind가 동일한지 확인한다. 이어서 잘못된 schema/version, 필수 필드 type, 알 수 없는 kind, 중복 Element ID, 파일명 ID 불일치를 각각 Load하고 기존 Active Document가 바뀌지 않는지 확인한다. `CDataJson`의 공통 16 MiB 제한도 그대로 적용된다. Save 실패에서는 기존 `.effect.json`과 Active Document가 모두 유지되어야 한다.

G03은 기존 C++ 파일만 수정하므로 `.vcxproj`와 `.vcxproj.filters` 항목을 추가하지 않는다. 실제 제작 JSON을 Git에 포함하지 않는 smoke라면 프로젝트 등록도 바꾸지 않는다.

## G04 — typed Effect resource catalog/input

> 적용 기록: 원래 중지선은 G03 단독 반영 시 G04를 미리 섞지 않기 위한 것이었다. 이후 사용자가
> G03·G04 동시 반영을 명시해 version 2, resource slot/binding, catalog까지 한 검증 단위로 적용했다.
> 자동 build와 focused Audit는 통과했지만 Debug Client 수동 smoke는 시작 `abort()` 때문에 미완료다.

G04는 `Resources/Effect`의 기존 `.wmodel`과 `.dds`만 local catalog로 읽고 Element Kind별 고정 slot에 Resources-relative stable asset ID를 commit한다. Mesh/Sprite/Decal/Trail은 각자의 typed binding을 사용하고 Particle은 emitter/renderer 분리가 들어오는 후속 G까지 unbound 상태를 유지한다.

### G04-1. 파일별 한 줄 역할

| 파일 | 역할 |
|---|---|
| `Client/Public/Effect_AuthoringDocument.h` | v2 typed resource slot과 Element별 binding 저장 계약을 소유한다. |
| `Client/Public/Effect_Tool.h` | catalog cache, stable Element/resource 선택, bind/clear 명령의 session 상태를 소유한다. |
| `Client/Private/Effect_Tool.cpp` | Resources/Effect scan, Kind-slot-file 검증, v1→v2 load migration, v2 Save/Load를 구현한다. |
| `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | typed resource 경계와 별도 Effect Runtime/Catalog 부재를 감사한다. |

### G04-2. C:/Users/user/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h

변경 종류: 적용 후 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 2u;
inline constexpr uint32_t EFFECT_AUTHORING_MIN_SUPPORTED_VERSION = 1u;

enum class EFFECT_ELEMENT_KIND : uint8_t
{
	MESH,
	SPRITE,
	PARTICLE,
	DECAL,
	TRAIL,
	END
};

// Element Kind가 받을 수 있는 resource 입력의 의미를 저장하는 stable slot이다.
enum class EFFECT_RESOURCE_SLOT : uint8_t
{
	MESH_MODEL,
	SPRITE_ATLAS,
	DECAL_TEXTURE,
	TRAIL_TEXTURE,
	END
};

// Catalog scan과 slot 검증에만 쓰며 JSON에 별도 저장하지 않는 파일 분류다.
enum class EFFECT_RESOURCE_FILE_KIND : uint8_t
{
	MODEL,
	TEXTURE,
	END
};

struct EFFECT_RESOURCE_BINDING_DESC final
{
	// END이면 unbound이며 strAssetId는 반드시 비어 있어야 한다.
	EFFECT_RESOURCE_SLOT eSlot = EFFECT_RESOURCE_SLOT::END;
	// `Effect/...` 형식의 Resources-relative stable asset ID다.
	std::string strAssetId;
};

struct EFFECT_ELEMENT_DESC final
{
	std::string strElementId;
	EFFECT_ELEMENT_KIND eKind = EFFECT_ELEMENT_KIND::END;
	// Mesh/Sprite/Decal/Trail의 typed input 한 건이며 Particle은 G04에서 비어 있다.
	EFFECT_RESOURCE_BINDING_DESC ResourceBinding;
};

struct EFFECT_DOCUMENT_DESC final
{
	uint32_t iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	std::string strEffectAssetId;
	std::string strDisplayName;
	std::vector<EFFECT_ELEMENT_DESC> Elements;
};

NS_END
```

### G04-3. C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h

변경 종류: 적용 후 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CEffect_Tool final
{
private:
	struct EFFECT_RESOURCE_CATALOG_ENTRY final
	{
		std::string strAssetId;
		EFFECT_RESOURCE_FILE_KIND eFileKind =
			EFFECT_RESOURCE_FILE_KIND::END;
	};

public:
	CEffect_Tool() = default;
	~CEffect_Tool() = default;

	void Render();

private:
	void Render_NewDocumentPanel();
	void Render_ActiveDocumentPanel();
	void Render_EffectTypeSelector();
	void Render_AddElementPanel();
	// stable Element ID 선택을 기록하므로 G03의 const를 제거한다.
	void Render_ElementList();
	// 선택 Element와 호환되는 catalog entry만 보여주고 bind/clear 명령을 전달한다.
	void Render_ResourceCatalogPanel();

	bool_t Try_CreateDocument();
	bool_t Try_AddElement();
	bool_t Try_SaveDocument();
	bool_t Try_LoadDocument();
	// Resources/Effect 전체 scan이 성공했을 때만 catalog cache를 교체한다.
	bool_t Refresh_ResourceCatalog();
	// 선택 resource를 Document copy에 기록하고 전체 검증 뒤 commit한다.
	bool_t Try_BindSelectedResource();
	// 선택 Element의 binding만 Document copy에서 비운 뒤 commit한다.
	bool_t Try_ClearSelectedResource();
	void Discard_ActiveDocument();

private:
	EFFECT_ELEMENT_KIND m_eSelectedEffectType =
		EFFECT_ELEMENT_KIND::MESH;
	std::array<char_t, 129> m_NewAssetId{};
	std::array<char_t, 65> m_NewDisplayName{};
	std::array<char_t, 129> m_NewElementId{};
	std::optional<EFFECT_DOCUMENT_DESC> m_ActiveDocument;
	std::string m_strDocumentStatus;
	std::string m_strElementStatus;

	// Tool session cache이며 Document나 resource payload를 소유하지 않는다.
	std::vector<EFFECT_RESOURCE_CATALOG_ENTRY> m_ResourceCatalog;
	// vector index 대신 현재 편집 대상을 식별하는 Element stable ID다.
	std::string m_strSelectedElementId;
	// Apply 전까지만 유지되는 Resources-relative catalog 선택 ID다.
	std::string m_strSelectedResourceAssetId;
	std::array<char_t, 129> m_ResourceFilter{};
	std::string m_strResourceStatus;
	// 최초 자동 scan 실패를 매 frame 반복하지 않게 하는 session flag다.
	bool_t m_bResourceCatalogRefreshAttempted = false;
};

NS_END
```

### G04-4. C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp

변경 종류: 적용 후 전체 코드

```cpp
#include "imgui.h"

#include "Effect_Tool.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <system_error>
#include <unordered_set>
#include <utility>

namespace
{
	constexpr const char_t* EFFECT_DOCUMENT_SCHEMA =
		"lostark.effect-authoring";
	constexpr size_t MAX_EFFECT_DOCUMENT_BYTES = 16u * 1024u * 1024u;
	constexpr size_t MAX_EFFECT_ELEMENT_COUNT = 4096u;
	constexpr size_t MAX_EFFECT_RESOURCE_ID_BYTES = 512u;

	constexpr const char_t* EFFECT_TYPE_LABELS[] = {
		"Mesh", "Sprite", "Particle", "Decal", "Trail"
	};

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

	bool_t Has_VisibleCharacter(const std::string& strValue)
	{
		return std::any_of(strValue.begin(), strValue.end(),
			[](const char_t value)
			{
				return 0 == std::isspace(
					static_cast<unsigned char>(value));
			});
	}

	bool_t Contains_NoCase(
		const std::string& strValue,
		const std::string& strFilter)
	{
		if (strFilter.empty())
			return true;
		return std::search(strValue.begin(), strValue.end(),
			strFilter.begin(), strFilter.end(),
			[](const char_t left, const char_t right)
			{
				return std::tolower(static_cast<unsigned char>(left)) ==
					std::tolower(static_cast<unsigned char>(right));
			}) != strValue.end();
	}

	const char_t* To_EffectElementKindLabel(
		const Client::EFFECT_ELEMENT_KIND eKind)
	{
		switch (eKind)
		{
		case Client::EFFECT_ELEMENT_KIND::MESH: return "Mesh";
		case Client::EFFECT_ELEMENT_KIND::SPRITE: return "Sprite";
		case Client::EFFECT_ELEMENT_KIND::PARTICLE: return "Particle";
		case Client::EFFECT_ELEMENT_KIND::DECAL: return "Decal";
		case Client::EFFECT_ELEMENT_KIND::TRAIL: return "Trail";
		case Client::EFFECT_ELEMENT_KIND::END:
		default: return "Invalid";
		}
	}

	const char_t* To_EffectElementKindToken(
		const Client::EFFECT_ELEMENT_KIND eKind)
	{
		switch (eKind)
		{
		case Client::EFFECT_ELEMENT_KIND::MESH: return "mesh";
		case Client::EFFECT_ELEMENT_KIND::SPRITE: return "sprite";
		case Client::EFFECT_ELEMENT_KIND::PARTICLE: return "particle";
		case Client::EFFECT_ELEMENT_KIND::DECAL: return "decal";
		case Client::EFFECT_ELEMENT_KIND::TRAIL: return "trail";
		case Client::EFFECT_ELEMENT_KIND::END:
		default: return nullptr;
		}
	}

	bool_t Try_ParseEffectElementKind(
		const std::string& strValue,
		Client::EFFECT_ELEMENT_KIND& eOutKind)
	{
		if ("mesh" == strValue)
			eOutKind = Client::EFFECT_ELEMENT_KIND::MESH;
		else if ("sprite" == strValue)
			eOutKind = Client::EFFECT_ELEMENT_KIND::SPRITE;
		else if ("particle" == strValue)
			eOutKind = Client::EFFECT_ELEMENT_KIND::PARTICLE;
		else if ("decal" == strValue)
			eOutKind = Client::EFFECT_ELEMENT_KIND::DECAL;
		else if ("trail" == strValue)
			eOutKind = Client::EFFECT_ELEMENT_KIND::TRAIL;
		else
			return false;
		return true;
	}

	const char_t* To_EffectResourceSlotLabel(
		const Client::EFFECT_RESOURCE_SLOT eSlot)
	{
		switch (eSlot)
		{
		case Client::EFFECT_RESOURCE_SLOT::MESH_MODEL: return "Mesh Model";
		case Client::EFFECT_RESOURCE_SLOT::SPRITE_ATLAS: return "Sprite Atlas";
		case Client::EFFECT_RESOURCE_SLOT::DECAL_TEXTURE: return "Decal Texture";
		case Client::EFFECT_RESOURCE_SLOT::TRAIL_TEXTURE: return "Trail Texture";
		case Client::EFFECT_RESOURCE_SLOT::END:
		default: return "Unbound";
		}
	}

	const char_t* To_EffectResourceSlotToken(
		const Client::EFFECT_RESOURCE_SLOT eSlot)
	{
		switch (eSlot)
		{
		case Client::EFFECT_RESOURCE_SLOT::MESH_MODEL: return "mesh-model";
		case Client::EFFECT_RESOURCE_SLOT::SPRITE_ATLAS: return "sprite-atlas";
		case Client::EFFECT_RESOURCE_SLOT::DECAL_TEXTURE: return "decal-texture";
		case Client::EFFECT_RESOURCE_SLOT::TRAIL_TEXTURE: return "trail-texture";
		case Client::EFFECT_RESOURCE_SLOT::END:
		default: return nullptr;
		}
	}

	bool_t Try_ParseEffectResourceSlot(
		const std::string& strValue,
		Client::EFFECT_RESOURCE_SLOT& eOutSlot)
	{
		if ("mesh-model" == strValue)
			eOutSlot = Client::EFFECT_RESOURCE_SLOT::MESH_MODEL;
		else if ("sprite-atlas" == strValue)
			eOutSlot = Client::EFFECT_RESOURCE_SLOT::SPRITE_ATLAS;
		else if ("decal-texture" == strValue)
			eOutSlot = Client::EFFECT_RESOURCE_SLOT::DECAL_TEXTURE;
		else if ("trail-texture" == strValue)
			eOutSlot = Client::EFFECT_RESOURCE_SLOT::TRAIL_TEXTURE;
		else
			return false;
		return true;
	}

	bool_t Try_GetResourceContract(
		const Client::EFFECT_ELEMENT_KIND eElementKind,
		Client::EFFECT_RESOURCE_SLOT& eOutSlot,
		Client::EFFECT_RESOURCE_FILE_KIND& eOutFileKind)
	{
		switch (eElementKind)
		{
		case Client::EFFECT_ELEMENT_KIND::MESH:
			eOutSlot = Client::EFFECT_RESOURCE_SLOT::MESH_MODEL;
			eOutFileKind = Client::EFFECT_RESOURCE_FILE_KIND::MODEL;
			return true;
		case Client::EFFECT_ELEMENT_KIND::SPRITE:
			eOutSlot = Client::EFFECT_RESOURCE_SLOT::SPRITE_ATLAS;
			eOutFileKind = Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
			return true;
		case Client::EFFECT_ELEMENT_KIND::DECAL:
			eOutSlot = Client::EFFECT_RESOURCE_SLOT::DECAL_TEXTURE;
			eOutFileKind = Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
			return true;
		case Client::EFFECT_ELEMENT_KIND::TRAIL:
			eOutSlot = Client::EFFECT_RESOURCE_SLOT::TRAIL_TEXTURE;
			eOutFileKind = Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
			return true;
		case Client::EFFECT_ELEMENT_KIND::PARTICLE:
		case Client::EFFECT_ELEMENT_KIND::END:
		default:
			eOutSlot = Client::EFFECT_RESOURCE_SLOT::END;
			eOutFileKind = Client::EFFECT_RESOURCE_FILE_KIND::END;
			return false;
		}
	}

	bool_t Is_ExactObject(
		const Client::DATA_JSON_VALUE& Value,
		const std::initializer_list<const char_t*> Keys)
	{
		if (!Value.Is_Object() || Value.Get_Object().size() != Keys.size())
			return false;
		return std::all_of(Keys.begin(), Keys.end(),
			[&Value](const char_t* pKey)
			{
				return nullptr != Value.Find(pKey);
			});
	}

	bool_t Is_IntegerJsonNumber(const Client::DATA_JSON_VALUE& Value)
	{
		return Value.Is_Number() && std::isfinite(Value.Get_Number()) &&
			std::trunc(Value.Get_Number()) == Value.Get_Number();
	}

	std::string To_LowerAscii(std::string strValue)
	{
		std::transform(strValue.begin(), strValue.end(), strValue.begin(),
			[](const char_t value)
			{
				return static_cast<char_t>(std::tolower(
					static_cast<unsigned char>(value)));
			});
		return strValue;
	}

	bool_t Try_GetResourceFileKind(
		const std::filesystem::path& FilePath,
		Client::EFFECT_RESOURCE_FILE_KIND& eOutFileKind)
	{
		const std::string strExtension =
			To_LowerAscii(FilePath.extension().string());
		if (".wmodel" == strExtension)
			eOutFileKind = Client::EFFECT_RESOURCE_FILE_KIND::MODEL;
		else if (".dds" == strExtension)
			eOutFileKind = Client::EFFECT_RESOURCE_FILE_KIND::TEXTURE;
		else
			return false;
		return true;
	}

	bool_t Is_SafeEffectResourceAssetId(
		const std::string& strAssetId,
		const Client::EFFECT_RESOURCE_FILE_KIND eExpectedFileKind)
	{
		if (strAssetId.empty() ||
			strAssetId.size() > MAX_EFFECT_RESOURCE_ID_BYTES ||
			0u != strAssetId.rfind("Effect/", 0u) ||
			std::string::npos != strAssetId.find('\\') ||
			std::string::npos != strAssetId.find(':'))
		{
			return false;
		}

		const std::filesystem::path RelativePath(strAssetId);
		if (RelativePath.is_absolute() || RelativePath.has_root_path())
			return false;
		if (RelativePath.lexically_normal().generic_string() != strAssetId)
			return false;
		for (const std::filesystem::path& Component : RelativePath)
		{
			const std::string strComponent = Component.generic_string();
			if (strComponent.empty() || "." == strComponent ||
				".." == strComponent)
			{
				return false;
			}
		}

		Client::EFFECT_RESOURCE_FILE_KIND eActualFileKind =
			Client::EFFECT_RESOURCE_FILE_KIND::END;
		if (!Try_GetResourceFileKind(RelativePath, eActualFileKind) ||
			eActualFileKind != eExpectedFileKind)
		{
			return false;
		}

		const std::filesystem::path ResolvedPath =
			Client::CRuntimeAssetRoot::Resolve(RelativePath);
		std::error_code ErrorCode;
		return !ResolvedPath.empty() &&
			std::filesystem::is_regular_file(ResolvedPath, ErrorCode) &&
			!ErrorCode;
	}

	bool_t Validate_EffectDocument(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError)
	{
		if (Client::EFFECT_AUTHORING_FORMAT_VERSION !=
			Document.iFormatVersion)
		{
			strOutError = "Unsupported Effect document version.";
			return false;
		}
		if (!Is_StableAuthoringId(Document.strEffectAssetId))
		{
			strOutError = "Effect Asset ID is invalid.";
			return false;
		}
		if (Document.strDisplayName.size() > 64u ||
			!Has_VisibleCharacter(Document.strDisplayName))
		{
			strOutError = "Display Name must be 1-64 bytes and not blank.";
			return false;
		}
		if (Document.Elements.size() > MAX_EFFECT_ELEMENT_COUNT)
		{
			strOutError = "Effect document has too many Elements.";
			return false;
		}

		std::unordered_set<std::string> ElementIds;
		ElementIds.reserve(Document.Elements.size());
		for (const Client::EFFECT_ELEMENT_DESC& Element : Document.Elements)
		{
			if (!Is_StableAuthoringId(Element.strElementId))
			{
				strOutError = "Element ID is invalid.";
				return false;
			}
			if (Element.eKind >= Client::EFFECT_ELEMENT_KIND::END)
			{
				strOutError = "Element kind is invalid.";
				return false;
			}
			if (!ElementIds.emplace(Element.strElementId).second)
			{
				strOutError = "Element ID is duplicated.";
				return false;
			}

			const Client::EFFECT_RESOURCE_BINDING_DESC& Binding =
				Element.ResourceBinding;
			if (Client::EFFECT_RESOURCE_SLOT::END == Binding.eSlot)
			{
				if (!Binding.strAssetId.empty())
				{
					strOutError = "Unbound resource slot has an asset ID.";
					return false;
				}
				continue;
			}

			Client::EFFECT_RESOURCE_SLOT eExpectedSlot =
				Client::EFFECT_RESOURCE_SLOT::END;
			Client::EFFECT_RESOURCE_FILE_KIND eExpectedFileKind =
				Client::EFFECT_RESOURCE_FILE_KIND::END;
			if (!Try_GetResourceContract(
					Element.eKind, eExpectedSlot, eExpectedFileKind) ||
				Binding.eSlot != eExpectedSlot ||
				!Is_SafeEffectResourceAssetId(
					Binding.strAssetId, eExpectedFileKind))
			{
				strOutError =
					"Element resource slot, file kind, path, or file is invalid.";
				return false;
			}
		}

		strOutError.clear();
		return true;
	}

	std::filesystem::path Resolve_EffectDocumentPath(
		const std::string& strEffectAssetId)
	{
		if (!Is_StableAuthoringId(strEffectAssetId))
			return {};
		return Client::CProjectDataRoot::Resolve(
			std::filesystem::path(L"Effects") / L"Authored" /
			std::filesystem::path(strEffectAssetId + ".effect.json"));
	}

	bool_t Read_EffectDocumentText(
		const std::filesystem::path& FilePath,
		std::string& strOutText,
		std::string& strOutError)
	{
		std::error_code ErrorCode;
		const uintmax_t iFileBytes =
			std::filesystem::file_size(FilePath, ErrorCode);
		if (ErrorCode)
		{
			strOutError = "Effect document does not exist or cannot be read.";
			return false;
		}
		if (iFileBytes > MAX_EFFECT_DOCUMENT_BYTES)
		{
			strOutError = "Effect document exceeds the 16 MiB limit.";
			return false;
		}

		std::ifstream Input(FilePath, std::ios::binary);
		if (!Input)
		{
			strOutError = "Failed to open Effect document.";
			return false;
		}
		std::ostringstream Buffer;
		Buffer << Input.rdbuf();
		if (Input.bad())
		{
			strOutError = "Failed while reading Effect document.";
			return false;
		}
		strOutText = Buffer.str();
		if (strOutText.size() != iFileBytes)
		{
			strOutError = "Effect document changed while it was being read.";
			return false;
		}
		strOutError.clear();
		return true;
	}

	bool_t Parse_EffectDocumentJson(
		const std::string& strText,
		Client::EFFECT_DOCUMENT_DESC& OutDocument,
		uint32_t& iOutSourceVersion,
		std::string& strOutError)
	{
		if (strText.size() > MAX_EFFECT_DOCUMENT_BYTES)
		{
			strOutError = "Effect document exceeds the 16 MiB limit.";
			return false;
		}

		Client::DATA_JSON_VALUE Root;
		if (!Client::CDataJson::Parse(strText, Root, strOutError))
			return false;
		if (!Is_ExactObject(Root,
			{ "schema", "version", "effectAssetId", "displayName", "elements" }))
		{
			strOutError = "Effect document root keys are invalid.";
			return false;
		}

		const Client::DATA_JSON_VALUE* pSchema = Root.Find("schema");
		const Client::DATA_JSON_VALUE* pVersion = Root.Find("version");
		const Client::DATA_JSON_VALUE* pAssetId = Root.Find("effectAssetId");
		const Client::DATA_JSON_VALUE* pDisplayName = Root.Find("displayName");
		const Client::DATA_JSON_VALUE* pElements = Root.Find("elements");
		if (!pSchema->Is_String() || EFFECT_DOCUMENT_SCHEMA != pSchema->Get_String() ||
			!Is_IntegerJsonNumber(*pVersion) ||
			pVersion->Get_Number() < static_cast<double>(
				Client::EFFECT_AUTHORING_MIN_SUPPORTED_VERSION) ||
			pVersion->Get_Number() > static_cast<double>(
				Client::EFFECT_AUTHORING_FORMAT_VERSION) ||
			!pAssetId->Is_String() || !pDisplayName->Is_String() ||
			!pElements->Is_Array())
		{
			strOutError = "Effect document field types or schema/version are invalid.";
			return false;
		}

		const uint32_t iSourceVersion =
			static_cast<uint32_t>(pVersion->Get_Number());
		if (1u != iSourceVersion && 2u != iSourceVersion)
		{
			strOutError = "Effect document version is not supported.";
			return false;
		}

		Client::EFFECT_DOCUMENT_DESC StagedDocument;
		StagedDocument.iFormatVersion =
			Client::EFFECT_AUTHORING_FORMAT_VERSION;
		StagedDocument.strEffectAssetId = pAssetId->Get_String();
		StagedDocument.strDisplayName = pDisplayName->Get_String();
		if (pElements->Get_Array().size() > MAX_EFFECT_ELEMENT_COUNT)
		{
			strOutError = "Effect document has too many Elements.";
			return false;
		}
		StagedDocument.Elements.reserve(pElements->Get_Array().size());

		for (const Client::DATA_JSON_VALUE& ElementValue :
			pElements->Get_Array())
		{
			const bool_t bExpectedKeys = 1u == iSourceVersion ?
				Is_ExactObject(ElementValue, { "id", "kind" }) :
				Is_ExactObject(ElementValue, { "id", "kind", "resource" });
			if (!bExpectedKeys)
			{
				strOutError = "Effect Element keys are invalid for its version.";
				return false;
			}

			const Client::DATA_JSON_VALUE* pId = ElementValue.Find("id");
			const Client::DATA_JSON_VALUE* pKind = ElementValue.Find("kind");
			Client::EFFECT_ELEMENT_DESC StagedElement;
			if (!pId->Is_String() || !pKind->Is_String() ||
				!Try_ParseEffectElementKind(
					pKind->Get_String(), StagedElement.eKind))
			{
				strOutError = "Effect Element ID or kind is invalid.";
				return false;
			}
			StagedElement.strElementId = pId->Get_String();

			if (2u == iSourceVersion)
			{
				const Client::DATA_JSON_VALUE* pResource =
					ElementValue.Find("resource");
				if (pResource->Is_Null())
				{
					StagedElement.ResourceBinding = {};
				}
				else if (Is_ExactObject(*pResource, { "slot", "assetId" }))
				{
					const Client::DATA_JSON_VALUE* pSlot =
						pResource->Find("slot");
					const Client::DATA_JSON_VALUE* pResourceAssetId =
						pResource->Find("assetId");
					if (!pSlot->Is_String() || !pResourceAssetId->Is_String() ||
						!Try_ParseEffectResourceSlot(
							pSlot->Get_String(),
							StagedElement.ResourceBinding.eSlot))
					{
						strOutError = "Effect resource binding is invalid.";
						return false;
					}
					StagedElement.ResourceBinding.strAssetId =
						pResourceAssetId->Get_String();
				}
				else
				{
					strOutError = "Effect resource must be null or a typed binding.";
					return false;
				}
			}

			StagedDocument.Elements.push_back(std::move(StagedElement));
		}

		if (!Validate_EffectDocument(StagedDocument, strOutError))
			return false;

		OutDocument = std::move(StagedDocument);
		iOutSourceVersion = iSourceVersion;
		strOutError.clear();
		return true;
	}

	std::string Serialize_EffectDocumentJson(
		const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		std::ostringstream Output;
		Output << "{\n"
			<< "  \"schema\": \"" << EFFECT_DOCUMENT_SCHEMA << "\",\n"
			<< "  \"version\": " << Document.iFormatVersion << ",\n"
			<< "  \"effectAssetId\": \""
			<< Client::CDataJson::Escape(Document.strEffectAssetId) << "\",\n"
			<< "  \"displayName\": \""
			<< Client::CDataJson::Escape(Document.strDisplayName) << "\",\n"
			<< "  \"elements\": [";

		for (size_t iElement = 0;
			iElement < Document.Elements.size(); ++iElement)
		{
			const Client::EFFECT_ELEMENT_DESC& Element =
				Document.Elements[iElement];
			Output << (0u == iElement ? "\n" : ",\n")
				<< "    {\n"
				<< "      \"id\": \""
				<< Client::CDataJson::Escape(Element.strElementId) << "\",\n"
				<< "      \"kind\": \""
				<< To_EffectElementKindToken(Element.eKind) << "\",\n"
				<< "      \"resource\": ";
			if (Client::EFFECT_RESOURCE_SLOT::END ==
				Element.ResourceBinding.eSlot)
			{
				Output << "null\n";
			}
			else
			{
				Output << "{ \"slot\": \""
					<< To_EffectResourceSlotToken(
						Element.ResourceBinding.eSlot)
					<< "\", \"assetId\": \""
					<< Client::CDataJson::Escape(
						Element.ResourceBinding.strAssetId)
					<< "\" }\n";
			}
			Output << "    }";
		}
		if (!Document.Elements.empty())
			Output << "\n  ";
		Output << "]\n}\n";
		return Output.str();
	}

	bool_t Are_EqualEffectDocuments(
		const Client::EFFECT_DOCUMENT_DESC& Left,
		const Client::EFFECT_DOCUMENT_DESC& Right)
	{
		if (Left.iFormatVersion != Right.iFormatVersion ||
			Left.strEffectAssetId != Right.strEffectAssetId ||
			Left.strDisplayName != Right.strDisplayName ||
			Left.Elements.size() != Right.Elements.size())
		{
			return false;
		}

		for (size_t iElement = 0;
			iElement < Left.Elements.size(); ++iElement)
		{
			const Client::EFFECT_ELEMENT_DESC& LeftElement =
				Left.Elements[iElement];
			const Client::EFFECT_ELEMENT_DESC& RightElement =
				Right.Elements[iElement];
			if (LeftElement.strElementId != RightElement.strElementId ||
				LeftElement.eKind != RightElement.eKind ||
				LeftElement.ResourceBinding.eSlot !=
					RightElement.ResourceBinding.eSlot ||
				LeftElement.ResourceBinding.strAssetId !=
					RightElement.ResourceBinding.strAssetId)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Write_TemporaryEffectDocument(
		const std::filesystem::path& TemporaryPath,
		const std::string& strText,
		std::string& strOutError)
	{
		std::ofstream Output(TemporaryPath,
			std::ios::binary | std::ios::trunc);
		if (!Output)
		{
			strOutError = "Failed to create temporary Effect document.";
			return false;
		}
		Output.write(strText.data(),
			static_cast<std::streamsize>(strText.size()));
		Output.flush();
		if (!Output)
		{
			Output.close();
			std::error_code IgnoreError;
			std::filesystem::remove(TemporaryPath, IgnoreError);
			strOutError = "Failed to write temporary Effect document.";
			return false;
		}
		Output.close();
		strOutError.clear();
		return true;
	}

	bool_t Commit_TemporaryEffectDocument(
		const std::filesystem::path& TemporaryPath,
		const std::filesystem::path& DestinationPath,
		std::string& strOutError)
	{
		if (!MoveFileExW(TemporaryPath.c_str(), DestinationPath.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			std::error_code IgnoreError;
			std::filesystem::remove(TemporaryPath, IgnoreError);
			strOutError = "Failed to atomically replace Effect document.";
			return false;
		}
		strOutError.clear();
		return true;
	}

	Client::EFFECT_ELEMENT_DESC* Find_EffectElement(
		Client::EFFECT_DOCUMENT_DESC& Document,
		const std::string& strElementId)
	{
		const auto Iterator = std::find_if(
			Document.Elements.begin(), Document.Elements.end(),
			[&strElementId](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			});
		return Iterator == Document.Elements.end() ? nullptr : &*Iterator;
	}

	const Client::EFFECT_ELEMENT_DESC* Find_EffectElement(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const std::string& strElementId)
	{
		const auto Iterator = std::find_if(
			Document.Elements.begin(), Document.Elements.end(),
			[&strElementId](const Client::EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			});
		return Iterator == Document.Elements.end() ? nullptr : &*Iterator;
	}
}

void Client::CEffect_Tool::Render()
{
	ImGui::SetNextWindowSize(ImVec2(920.f, 680.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Effect Tool###LostArkEffectToolG4"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"G4: bind Resources/Effect models and textures through typed Element "
		"slots. Particle emitter/renderer inputs and GPU preview remain deferred.");

	if (m_ActiveDocument.has_value())
		Render_ActiveDocumentPanel();
	else
		Render_NewDocumentPanel();

	ImGui::Separator();
	Render_EffectTypeSelector();
	if (m_ActiveDocument.has_value())
	{
		Render_AddElementPanel();
		Render_ElementList();
		Render_ResourceCatalogPanel();
	}
	else
	{
		ImGui::TextDisabled(
			"Create or load a document before adding an Element.");
	}

	ImGui::End();
}

void Client::CEffect_Tool::Render_NewDocumentPanel()
{
	ImGui::SeparatorText("Data File");
	ImGui::InputText("Effect Asset ID", m_NewAssetId.data(),
		m_NewAssetId.size());
	ImGui::InputText("Display Name", m_NewDisplayName.data(),
		m_NewDisplayName.size());
	if (ImGui::Button("Create Document"))
		Try_CreateDocument();
	ImGui::SameLine();
	if (ImGui::Button("Load Document"))
		Try_LoadDocument();
	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_ActiveDocumentPanel()
{
	ImGui::SeparatorText("Active Document");
	const EFFECT_DOCUMENT_DESC& Document = *m_ActiveDocument;
	ImGui::Text("Format Version: %u", Document.iFormatVersion);
	ImGui::Text("Effect Asset ID: %s",
		Document.strEffectAssetId.c_str());
	ImGui::Text("Display Name: %s", Document.strDisplayName.c_str());
	ImGui::Text("Elements: %zu", Document.Elements.size());
	if (ImGui::Button("Save Active Document"))
		Try_SaveDocument();
	ImGui::SameLine();
	if (ImGui::Button("Discard Active Document"))
		Discard_ActiveDocument();
	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_EffectTypeSelector()
{
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

void Client::CEffect_Tool::Render_ElementList()
{
	ImGui::SeparatorText("Current Elements");
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->Elements.empty())
	{
		ImGui::TextDisabled("No Elements have been added.");
		m_strSelectedElementId.clear();
		m_strSelectedResourceAssetId.clear();
		return;
	}

	if (!m_strSelectedElementId.empty() &&
		nullptr == Find_EffectElement(
			*m_ActiveDocument, m_strSelectedElementId))
	{
		m_strSelectedElementId.clear();
		m_strSelectedResourceAssetId.clear();
	}

	constexpr ImGuiTableFlags TABLE_FLAGS =
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_SizingStretchProp;
	if (!ImGui::BeginTable("EffectElementList", 3, TABLE_FLAGS))
		return;
	ImGui::TableSetupColumn("Element ID");
	ImGui::TableSetupColumn("Kind");
	ImGui::TableSetupColumn("Resource");
	ImGui::TableHeadersRow();

	for (const EFFECT_ELEMENT_DESC& Element :
		m_ActiveDocument->Elements)
	{
		ImGui::PushID(Element.strElementId.c_str());
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		const bool_t bSelected =
			m_strSelectedElementId == Element.strElementId;
		if (ImGui::Selectable(Element.strElementId.c_str(), bSelected,
			ImGuiSelectableFlags_SpanAllColumns))
		{
			m_strSelectedElementId = Element.strElementId;
			m_strSelectedResourceAssetId.clear();
			m_strResourceStatus.clear();
		}
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(
			To_EffectElementKindLabel(Element.eKind));
		ImGui::TableSetColumnIndex(2);
		if (EFFECT_RESOURCE_SLOT::END ==
			Element.ResourceBinding.eSlot)
		{
			ImGui::TextDisabled("Unbound");
		}
		else
		{
			ImGui::TextUnformatted(
				Element.ResourceBinding.strAssetId.c_str());
		}
		ImGui::PopID();
	}
	ImGui::EndTable();
}

void Client::CEffect_Tool::Render_ResourceCatalogPanel()
{
	ImGui::SeparatorText("Typed Resource Input");
	if (!m_ActiveDocument.has_value() ||
		m_strSelectedElementId.empty())
	{
		ImGui::TextDisabled("Select an Element first.");
		return;
	}

	const EFFECT_ELEMENT_DESC* pElement = Find_EffectElement(
		*m_ActiveDocument, m_strSelectedElementId);
	if (nullptr == pElement)
	{
		m_strSelectedElementId.clear();
		m_strSelectedResourceAssetId.clear();
		ImGui::TextDisabled("Selected Element no longer exists.");
		return;
	}

	EFFECT_RESOURCE_SLOT eExpectedSlot = EFFECT_RESOURCE_SLOT::END;
	EFFECT_RESOURCE_FILE_KIND eExpectedFileKind =
		EFFECT_RESOURCE_FILE_KIND::END;
	if (!Try_GetResourceContract(
		pElement->eKind, eExpectedSlot, eExpectedFileKind))
	{
		ImGui::TextWrapped(
			"Particle stays unbound in G4. Its simulation emitter and renderer "
			"inputs are introduced together in a later G.");
		return;
	}

	if (!m_bResourceCatalogRefreshAttempted)
		Refresh_ResourceCatalog();
	if (ImGui::Button("Refresh Resources/Effect"))
		Refresh_ResourceCatalog();

	ImGui::Text("Selected Element: %s", pElement->strElementId.c_str());
	ImGui::Text("Required Slot: %s",
		To_EffectResourceSlotLabel(eExpectedSlot));
	ImGui::Text("Current Resource: %s",
		pElement->ResourceBinding.strAssetId.empty() ?
			"Unbound" : pElement->ResourceBinding.strAssetId.c_str());
	ImGui::InputText("Resource Filter", m_ResourceFilter.data(),
		m_ResourceFilter.size());

	const std::string strFilter = m_ResourceFilter.data();
	if (ImGui::BeginChild("EffectResourceCatalog", ImVec2(0.f, 180.f),
		true))
	{
		for (const EFFECT_RESOURCE_CATALOG_ENTRY& Entry :
			m_ResourceCatalog)
		{
			if (Entry.eFileKind != eExpectedFileKind ||
				!Contains_NoCase(Entry.strAssetId, strFilter))
			{
				continue;
			}
			const bool_t bSelected =
				m_strSelectedResourceAssetId == Entry.strAssetId;
			if (ImGui::Selectable(
				Entry.strAssetId.c_str(), bSelected))
			{
				m_strSelectedResourceAssetId = Entry.strAssetId;
				m_strResourceStatus.clear();
			}
		}
	}
	ImGui::EndChild();

	const bool_t bCanClearResource =
		EFFECT_RESOURCE_SLOT::END != pElement->ResourceBinding.eSlot;
	ImGui::BeginDisabled(m_strSelectedResourceAssetId.empty());
	if (ImGui::Button("Apply Typed Resource"))
		Try_BindSelectedResource();
	ImGui::EndDisabled();
	ImGui::SameLine();
	// Apply가 Active Document를 교체해도 교체 전 pointer를 다시 읽지 않는다.
	ImGui::BeginDisabled(!bCanClearResource);
	if (ImGui::Button("Clear Resource"))
		Try_ClearSelectedResource();
	ImGui::EndDisabled();

	if (!m_strResourceStatus.empty())
		ImGui::TextWrapped("%s", m_strResourceStatus.c_str());
}

bool_t Client::CEffect_Tool::Try_CreateDocument()
{
	if (m_ActiveDocument.has_value())
	{
		m_strDocumentStatus =
			"Discard the active document before creating another one.";
		return false;
	}
	const std::string strAssetId = m_NewAssetId.data();
	const std::string strDisplayName = m_NewDisplayName.data();
	if (!Is_StableAuthoringId(strAssetId))
	{
		m_strDocumentStatus =
			"Effect Asset ID must be 1-128 ASCII letters, digits, '.', '_' or '-'.";
		return false;
	}
	if (strDisplayName.size() > 64u ||
		!Has_VisibleCharacter(strDisplayName))
	{
		m_strDocumentStatus =
			"Display Name must be 1-64 bytes and not blank.";
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument;
	StagedDocument.strEffectAssetId = strAssetId;
	StagedDocument.strDisplayName = strDisplayName;
	std::string strValidationError;
	if (!Validate_EffectDocument(StagedDocument, strValidationError))
	{
		m_strDocumentStatus = strValidationError;
		return false;
	}

	m_ActiveDocument = std::move(StagedDocument);
	m_NewAssetId.fill('\0');
	m_NewDisplayName.fill('\0');
	m_NewElementId.fill('\0');
	m_strSelectedElementId.clear();
	m_strSelectedResourceAssetId.clear();
	m_strElementStatus.clear();
	m_strResourceStatus.clear();
	m_strDocumentStatus =
		"Created in memory. This document has not been saved.";
	return true;
}

bool_t Client::CEffect_Tool::Try_AddElement()
{
	if (!m_ActiveDocument.has_value())
	{
		m_strElementStatus =
			"Create or load a document before adding an Element.";
		return false;
	}
	if (EFFECT_ELEMENT_KIND::END == m_eSelectedEffectType)
	{
		m_strElementStatus = "Select a valid Element type.";
		return false;
	}

	const std::string strElementId = m_NewElementId.data();
	if (!Is_StableAuthoringId(strElementId))
	{
		m_strElementStatus =
			"Element ID must be 1-128 ASCII letters, digits, '.', '_' or '-'.";
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC StagedElement;
	StagedElement.strElementId = strElementId;
	StagedElement.eKind = m_eSelectedEffectType;
	StagedDocument.Elements.push_back(std::move(StagedElement));
	std::string strValidationError;
	if (!Validate_EffectDocument(StagedDocument, strValidationError))
	{
		m_strElementStatus = strValidationError;
		return false;
	}

	m_ActiveDocument = std::move(StagedDocument);
	m_NewElementId.fill('\0');
	m_strSelectedElementId = strElementId;
	m_strSelectedResourceAssetId.clear();
	m_strResourceStatus.clear();
	m_strElementStatus =
		"Added to the in-memory document. Save to persist this change.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SaveDocument()
{
	if (!m_ActiveDocument.has_value())
	{
		m_strDocumentStatus = "There is no active document to save.";
		return false;
	}

	std::string strError;
	if (!Validate_EffectDocument(*m_ActiveDocument, strError))
	{
		m_strDocumentStatus = "Save rejected: " + strError;
		return false;
	}

	const std::filesystem::path DestinationPath =
		Resolve_EffectDocumentPath(m_ActiveDocument->strEffectAssetId);
	if (DestinationPath.empty())
	{
		m_strDocumentStatus = "Save path escaped the project Data root.";
		return false;
	}
	std::error_code DirectoryError;
	std::filesystem::create_directories(
		DestinationPath.parent_path(), DirectoryError);
	if (DirectoryError)
	{
		m_strDocumentStatus =
			"Failed to create Data/Effects/Authored.";
		return false;
	}

	std::filesystem::path TemporaryPath = DestinationPath;
	TemporaryPath += L".tmp";
	const std::string strSerialized =
		Serialize_EffectDocumentJson(*m_ActiveDocument);
	if (!Write_TemporaryEffectDocument(
		TemporaryPath, strSerialized, strError))
	{
		m_strDocumentStatus =
			"Save rejected; previous file preserved: " + strError;
		return false;
	}

	std::string strRoundTripText;
	EFFECT_DOCUMENT_DESC RoundTripDocument;
	uint32_t iRoundTripVersion = 0u;
	if (!Read_EffectDocumentText(
			TemporaryPath, strRoundTripText, strError) ||
		!Parse_EffectDocumentJson(
			strRoundTripText, RoundTripDocument,
			iRoundTripVersion, strError) ||
		EFFECT_AUTHORING_FORMAT_VERSION != iRoundTripVersion ||
		!Are_EqualEffectDocuments(*m_ActiveDocument, RoundTripDocument))
	{
		std::error_code IgnoreError;
		std::filesystem::remove(TemporaryPath, IgnoreError);
		if (strError.empty())
			strError = "Serialized Effect document did not round trip.";
		m_strDocumentStatus =
			"Save rejected; previous file preserved: " + strError;
		return false;
	}

	if (!Commit_TemporaryEffectDocument(
		TemporaryPath, DestinationPath, strError))
	{
		m_strDocumentStatus =
			"Save rejected; previous file preserved: " + strError;
		return false;
	}
	m_strDocumentStatus =
		"Saved Data/Effects/Authored/" +
		m_ActiveDocument->strEffectAssetId + ".effect.json as v2.";
	return true;
}

bool_t Client::CEffect_Tool::Try_LoadDocument()
{
	if (m_ActiveDocument.has_value())
	{
		m_strDocumentStatus =
			"Discard the active document before loading another one.";
		return false;
	}

	const std::string strRequestedAssetId = m_NewAssetId.data();
	if (!Is_StableAuthoringId(strRequestedAssetId))
	{
		m_strDocumentStatus =
			"Enter a valid Effect Asset ID before loading.";
		return false;
	}
	const std::filesystem::path SourcePath =
		Resolve_EffectDocumentPath(strRequestedAssetId);
	if (SourcePath.empty())
	{
		m_strDocumentStatus = "Load path escaped the project Data root.";
		return false;
	}

	std::string strText;
	std::string strError;
	EFFECT_DOCUMENT_DESC StagedDocument;
	uint32_t iSourceVersion = 0u;
	if (!Read_EffectDocumentText(SourcePath, strText, strError) ||
		!Parse_EffectDocumentJson(
			strText, StagedDocument, iSourceVersion, strError))
	{
		m_strDocumentStatus =
			"Load rejected; active document preserved: " + strError;
		return false;
	}
	if (StagedDocument.strEffectAssetId != strRequestedAssetId)
	{
		m_strDocumentStatus =
			"Load rejected: file name ID and document ID do not match.";
		return false;
	}

	m_ActiveDocument = std::move(StagedDocument);
	m_NewAssetId.fill('\0');
	m_NewDisplayName.fill('\0');
	m_NewElementId.fill('\0');
	m_strSelectedElementId.clear();
	m_strSelectedResourceAssetId.clear();
	m_strElementStatus.clear();
	m_strResourceStatus.clear();
	m_strDocumentStatus = 1u == iSourceVersion ?
		"Loaded v1 into a staged v2 document. Save to persist the migration." :
		"Loaded and committed the v2 Effect document.";
	return true;
}

bool_t Client::CEffect_Tool::Refresh_ResourceCatalog()
{
	m_bResourceCatalogRefreshAttempted = true;
	const std::filesystem::path EffectRoot =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(L"Effect"));
	std::error_code ErrorCode;
	if (EffectRoot.empty() ||
		!std::filesystem::is_directory(EffectRoot, ErrorCode) || ErrorCode)
	{
		m_strResourceStatus =
			"Catalog refresh failed; previous catalog preserved: Resources/Effect is unavailable.";
		return false;
	}

	std::vector<EFFECT_RESOURCE_CATALOG_ENTRY> StagedCatalog;
	std::filesystem::recursive_directory_iterator Iterator(
		EffectRoot,
		std::filesystem::directory_options::none,
		ErrorCode);
	const std::filesystem::recursive_directory_iterator EndIterator;
	if (ErrorCode)
	{
		m_strResourceStatus =
			"Catalog refresh failed; previous catalog preserved: scan could not start.";
		return false;
	}

	for (; Iterator != EndIterator; Iterator.increment(ErrorCode))
	{
		if (ErrorCode)
			break;

		std::error_code FileError;
		if (!Iterator->is_regular_file(FileError))
		{
			if (FileError)
			{
				ErrorCode = FileError;
				break;
			}
			continue;
		}

		EFFECT_RESOURCE_FILE_KIND eFileKind =
			EFFECT_RESOURCE_FILE_KIND::END;
		if (!Try_GetResourceFileKind(Iterator->path(), eFileKind))
			continue;

		const std::filesystem::path EffectRelativePath =
			Iterator->path().lexically_relative(EffectRoot);
		if (EffectRelativePath.empty())
		{
			ErrorCode = std::make_error_code(std::errc::invalid_argument);
			break;
		}
		const std::string strAssetId =
			(std::filesystem::path("Effect") /
				EffectRelativePath).generic_string();
		if (!Is_SafeEffectResourceAssetId(strAssetId, eFileKind))
		{
			ErrorCode = std::make_error_code(std::errc::invalid_argument);
			break;
		}

		EFFECT_RESOURCE_CATALOG_ENTRY Entry;
		Entry.strAssetId = strAssetId;
		Entry.eFileKind = eFileKind;
		StagedCatalog.push_back(std::move(Entry));
	}

	if (ErrorCode)
	{
		m_strResourceStatus =
			"Catalog refresh failed; previous catalog preserved: scan was incomplete.";
		return false;
	}

	std::sort(StagedCatalog.begin(), StagedCatalog.end(),
		[](const EFFECT_RESOURCE_CATALOG_ENTRY& Left,
			const EFFECT_RESOURCE_CATALOG_ENTRY& Right)
		{
			return Left.strAssetId < Right.strAssetId;
		});
	const auto DuplicateIterator = std::adjacent_find(
		StagedCatalog.begin(), StagedCatalog.end(),
		[](const EFFECT_RESOURCE_CATALOG_ENTRY& Left,
			const EFFECT_RESOURCE_CATALOG_ENTRY& Right)
		{
			return Left.strAssetId == Right.strAssetId;
		});
	if (DuplicateIterator != StagedCatalog.end())
	{
		m_strResourceStatus =
			"Catalog refresh failed; previous catalog preserved: duplicate asset ID.";
		return false;
	}

	m_ResourceCatalog = std::move(StagedCatalog);
	if (!m_strSelectedResourceAssetId.empty())
	{
		const bool_t bSelectionStillExists = std::any_of(
			m_ResourceCatalog.begin(), m_ResourceCatalog.end(),
			[this](const EFFECT_RESOURCE_CATALOG_ENTRY& Entry)
			{
				return Entry.strAssetId ==
					m_strSelectedResourceAssetId;
			});
		if (!bSelectionStillExists)
			m_strSelectedResourceAssetId.clear();
	}
	m_strResourceStatus = "Catalog refreshed: " +
		std::to_string(m_ResourceCatalog.size()) +
		" supported Resources/Effect files.";
	return true;
}

bool_t Client::CEffect_Tool::Try_BindSelectedResource()
{
	if (!m_ActiveDocument.has_value() ||
		m_strSelectedElementId.empty() ||
		m_strSelectedResourceAssetId.empty())
	{
		m_strResourceStatus =
			"Select an Element and a compatible resource before Apply.";
		return false;
	}

	const EFFECT_ELEMENT_DESC* pCurrentElement = Find_EffectElement(
		*m_ActiveDocument, m_strSelectedElementId);
	if (nullptr == pCurrentElement)
	{
		m_strResourceStatus = "Selected Element no longer exists.";
		return false;
	}

	EFFECT_RESOURCE_SLOT eExpectedSlot = EFFECT_RESOURCE_SLOT::END;
	EFFECT_RESOURCE_FILE_KIND eExpectedFileKind =
		EFFECT_RESOURCE_FILE_KIND::END;
	if (!Try_GetResourceContract(
		pCurrentElement->eKind, eExpectedSlot, eExpectedFileKind))
	{
		m_strResourceStatus =
			"This Element kind has no G4 resource binding contract.";
		return false;
	}

	const auto CatalogIterator = std::find_if(
		m_ResourceCatalog.begin(), m_ResourceCatalog.end(),
		[this](const EFFECT_RESOURCE_CATALOG_ENTRY& Entry)
		{
			return Entry.strAssetId == m_strSelectedResourceAssetId;
		});
	if (CatalogIterator == m_ResourceCatalog.end() ||
		CatalogIterator->eFileKind != eExpectedFileKind ||
		!Is_SafeEffectResourceAssetId(
			CatalogIterator->strAssetId, eExpectedFileKind))
	{
		m_strResourceStatus =
			"Selected resource is stale, missing, or incompatible.";
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* pStagedElement = Find_EffectElement(
		StagedDocument, m_strSelectedElementId);
	if (nullptr == pStagedElement)
	{
		m_strResourceStatus = "Selected Element disappeared while staging.";
		return false;
	}
	pStagedElement->ResourceBinding.eSlot = eExpectedSlot;
	pStagedElement->ResourceBinding.strAssetId =
		CatalogIterator->strAssetId;

	std::string strValidationError;
	if (!Validate_EffectDocument(StagedDocument, strValidationError))
	{
		m_strResourceStatus =
			"Binding rejected; active document preserved: " +
			strValidationError;
		return false;
	}

	m_ActiveDocument = std::move(StagedDocument);
	m_strResourceStatus =
		"Typed resource committed. Save to persist this binding.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ClearSelectedResource()
{
	if (!m_ActiveDocument.has_value() ||
		m_strSelectedElementId.empty())
	{
		m_strResourceStatus = "Select an Element before clearing.";
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* pStagedElement = Find_EffectElement(
		StagedDocument, m_strSelectedElementId);
	if (nullptr == pStagedElement)
	{
		m_strResourceStatus = "Selected Element no longer exists.";
		return false;
	}
	pStagedElement->ResourceBinding = {};

	std::string strValidationError;
	if (!Validate_EffectDocument(StagedDocument, strValidationError))
	{
		m_strResourceStatus =
			"Clear rejected; active document preserved: " +
			strValidationError;
		return false;
	}

	m_ActiveDocument = std::move(StagedDocument);
	m_strResourceStatus =
		"Resource binding cleared. Save to persist this change.";
	return true;
}

void Client::CEffect_Tool::Discard_ActiveDocument()
{
	m_ActiveDocument.reset();
	m_NewElementId.fill('\0');
	m_strSelectedElementId.clear();
	m_strSelectedResourceAssetId.clear();
	m_strElementStatus.clear();
	m_strResourceStatus.clear();
	m_strDocumentStatus =
		"Discarded the in-memory document. No file was changed.";
}
```

### G04-5. 저장 JSON 전체 형식

적용 위치: `Data/Effects/Authored/<EffectAssetId>.effect.json`

```json
{
  "schema": "lostark.effect-authoring",
  "version": 2,
  "effectAssetId": "training_hit_a",
  "displayName": "Training Hit A",
  "elements": [
    {
      "id": "mesh_core",
      "kind": "mesh",
      "resource": { "slot": "mesh-model", "assetId": "Effect/Dimensionist/Meshes/bfm_q_crack_01.wmodel" }
    },
    {
      "id": "spark_sheet",
      "kind": "sprite",
      "resource": { "slot": "sprite-atlas", "assetId": "Effect/Dimensionist/Textures/FX_TEX_00/fx_a_atypical_002_cl.dds" }
    },
    {
      "id": "dust_particles",
      "kind": "particle",
      "resource": null
    },
    {
      "id": "ground_mark",
      "kind": "decal",
      "resource": { "slot": "decal-texture", "assetId": "Effect/Dimensionist/Textures/BG_OCN_ETC_J/bg_ocn_etc_magicsquare01_d_kmk.dds" }
    },
    {
      "id": "weapon_trail",
      "kind": "trail",
      "resource": { "slot": "trail-texture", "assetId": "Effect/Dimensionist/Textures/FX_H_W_01/fx_h_wave_04.dds" }
    }
  ]
}
```

위 asset ID는 형식 예시다. 구현 시 catalog에 실제로 나타나는 기존 `Effect/...` ID를 UI에서 선택하며 예시 이름을 새 payload로 생성하지 않는다.

### G04-6. C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1

변경 종류: G03의 `effect.g3-save-load-boundary` 전체 블록을 아래 블록으로 완전 교체

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
	$unexpectedAuthoredEffectFiles = @($authoredEffectFiles |
		Where-Object { $_.Name -notmatch '^[A-Za-z0-9_.-]+\.effect\.json$' })
	$effectIntakeFiles = @(Get-ChildItem -LiteralPath 'Tools\EffectResourceIntake' -Recurse -File -ErrorAction SilentlyContinue)
	$effectShaderFiles = @(Get-ChildItem -LiteralPath 'Client\Bin\ShaderFiles' -File -Filter 'Shader_Effect*' -ErrorAction SilentlyContinue)
	$legacyEffectSymbolHits = @($clientSourceFiles | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC')
	$legacyEffectProjectHits = @($clientProjectSource | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|Shader_Effect')
	$legacyEffectEntry =
		$clientEntrySource -match 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC|--effect-' -or
		$engineImGuiSource -match '--effect-'
	$effectG4TypedResourceShape =
		$effectDocumentHeader -match 'EFFECT_AUTHORING_FORMAT_VERSION\s*=\s*2u' -and
		$effectDocumentHeader -match 'EFFECT_AUTHORING_MIN_SUPPORTED_VERSION\s*=\s*1u' -and
		$effectDocumentHeader -match 'enum class EFFECT_RESOURCE_SLOT[\s\S]*MESH_MODEL[\s\S]*SPRITE_ATLAS[\s\S]*DECAL_TEXTURE[\s\S]*TRAIL_TEXTURE[\s\S]*END' -and
		$effectDocumentHeader -match 'struct EFFECT_RESOURCE_BINDING_DESC[\s\S]*eSlot[\s\S]*strAssetId' -and
		$effectDocumentHeader -match 'struct EFFECT_ELEMENT_DESC[\s\S]*ResourceBinding' -and
		$effectDocumentHeader -notmatch 'strResourceId' -and
		$effectToolHeader -match 'vector<EFFECT_RESOURCE_CATALOG_ENTRY>\s+m_ResourceCatalog' -and
		$effectToolHeader -match 'm_strSelectedElementId' -and
		$effectToolHeader -match 'Try_BindSelectedResource' -and
		$effectToolSource -match 'CRuntimeAssetRoot::Resolve' -and
		$effectToolSource -match 'recursive_directory_iterator' -and
		$effectToolSource -match '\.wmodel' -and
		$effectToolSource -match '\.dds' -and
		$effectToolSource -match 'Try_GetResourceContract' -and
		$effectToolSource -match 'iSourceVersion' -and
		$effectToolSource -match 'ResourceBinding' -and
		$effectToolSource -notmatch 'ID3D11'
	Add-Check 'effect.g4-typed-resource-boundary' (
		$removedEffectPathHits.Count -eq 0 -and
		$unexpectedAuthoredEffectFiles.Count -eq 0 -and
		$effectIntakeFiles.Count -eq 0 -and
		$effectShaderFiles.Count -eq 0 -and
		$legacyEffectSymbolHits.Count -eq 0 -and
		$legacyEffectProjectHits.Count -eq 0 -and
		-not $legacyEffectEntry -and
		$effectG4TypedResourceShape) "paths=$($removedEffectPathHits.Count) authoredUnexpected=$($unexpectedAuthoredEffectFiles.Count) intake=$($effectIntakeFiles.Count) shaders=$($effectShaderFiles.Count) symbols=$($legacyEffectSymbolHits.Count) project=$($legacyEffectProjectHits.Count) entry=$legacyEffectEntry typedResource=$effectG4TypedResourceShape"
```

### G04-7. 직접 작성 순서

1. `Effect_AuthoringDocument.h`를 v2로 올리고 slot, file kind, binding을 추가한다.
2. `Effect_Tool.h`에 catalog entry/cache와 stable Element/resource 선택 상태, refresh/bind/clear 함수를 추가한다.
3. `Effect_Tool.cpp`에 Kind→slot/file contract와 Resources-relative 경로·실파일 검증을 작성한다.
4. JSON parser에 v1 unbound migration과 strict v2 resource object를 추가하고 serializer를 v2로 교체한다.
5. `Render_ElementList`에서 stable Element ID를 선택하고 `Render_ResourceCatalogPanel`을 연결한다.
6. refresh는 전체 scan 성공 후 cache commit, bind/clear는 Document copy 전체 검증 후 commit하도록 작성한다.
7. ProjectAudit의 G03 Effect 블록을 G04 블록으로 교체한다.

### G04-8. 자동 검증

```powershell
git diff --check
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj' -Raw) | Out-Null
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj.filters' -Raw) | Out-Null
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
```

기대 결과는 Debug Engine/Shared/Server/Client build 성공, 기존 harness failures 0, `effect.g4-typed-resource-boundary` PASS다. 현재 알려진 `asset-lock.inventory` 불일치는 Effect G04 코드 실패와 분리해 실제 inventory 값과 lock 갱신 권한을 확인한다.

### G04-9. Runtime smoke

1. G03 v1 문서를 Load해 모든 Element가 unbound인 메모리 v2로 commit되는지 확인한다.
2. Refresh 후 `.wmodel`은 Mesh에만, `.dds`는 Sprite/Decal/Trail에만 나타나는지 확인한다.
3. Mesh/Sprite/Decal/Trail에 실제 catalog ID를 Apply하고 Save/Discard/Load 뒤 같은 slot과 asset ID가 유지되는지 확인한다.
4. Particle을 선택했을 때 generic resource 입력이 없고 후속 emitter/renderer 안내만 보이는지 확인한다.
5. `..`, 절대 경로, `Effect/` 밖 경로, slot 불일치, 확장자 불일치, 존재하지 않는 파일, 중복 Element ID, 잘못된 v2 resource key를 Load하고 Active Document가 유지되는지 확인한다.
6. scan 중 접근 오류 또는 잘못된 supported file ID를 재현했을 때 기존 catalog cache가 유지되는지 확인한다.
7. v1을 Save하면 v2 전체 형식으로 원자 교체되고 실패 시 기존 파일이 유지되는지 확인한다.

G04도 기존 C++ 세 파일만 수정하므로 `.vcxproj`와 `.vcxproj.filters` C++ 항목을 추가하지 않는다. 실제 제작 JSON을 Git에 포함할 때만 `96.DataFiles` `<None>`과 대응 filter를 같은 변경 단위에 추가한다. G04는 GPU preview, Material input, simulation emitter, renderer, timeline, graph node를 만들지 않는다.
