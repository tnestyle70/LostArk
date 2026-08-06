# Effect Tool 완전 재구축 계획

기존 Effect Tool 발전 계획을 폐기하고 2026-08-04 사용자 결정에 따라 처음부터 다시 시작한다.
G0는 참고 화면의 `Effect Type` 다섯 선택지만 ImGui에 만들었다. G1은 저장이나 GPU resource 없이
안정 ID를 가진 Effect authoring document 한 건을 메모리에 생성·폐기하는 수명만 추가한다.

참고 이미지:

- `C:/Users/user/Desktop/툴/10_EffectTool_Integrated_Workspace.png`
- SHA-256: `1403d9b11189a09220d0f0d308be33b7a70a24e3ad623008d336afa439bcc885`

## 1. C1~C8 관점

```text
C1 기준계          G0에는 좌표, 파일 ID, runtime transform이 없다.
C2 이동>계산       중요: 파일 IO와 simulation을 모두 제거하고 ImGui selection만 남긴다.
C3 공유는 비싸다   Animation/Preview를 Effect Tool 안으로 복사하지 않는다.
C4 수명은 선언된다 중요: CMainApp이 Debug Effect Tool을 만들고 종료 때 파괴한다.
C5 이산화와 오차   G0에는 time/frame/particle 수치가 없다.
C6 가지치기        중요: 구 effect source/data/shader/intake 소비자를 함께 제거한다.
C7 권위와 정합성   선택 enum은 아직 저장 포맷이나 runtime 계약이 아닌 Tool session state다.
C8 검증이 병목     Debug/Release build와 F1 수동 화면 확인을 분리한다.
```

## 2. 문제 해결 ①~⑤

① 문제·제약: 구 Effect 구현은 4천 줄 이상의 runtime/data/editor가 결합됐고 참고 화면과 새 Tool 본질을 단계적으로 설명하기 어렵다.
② 단순 해법의 문제: C++ 파일만 지우면 Client CLI smoke, HLSL, ProjectAudit, 459개 Data project item이 고아가 된다.
③ 해결 방식: 구 계약의 모든 소비자를 함께 제거하고 `Effect_Tool.h/.cpp`만 새 selection-only shell로 작성한다.
④ 비교: 참고 화면은 Type, resource palette, viewport, detail, instance, data window가 있지만 G0는 첫 Type 행만 구현한다.
⑤ 대가: G0는 effect 생성·표시·저장 기능이 전혀 없고 다음 G부터 한 계약씩 검증하며 추가한다.

## 3. 참고 이미지에서 읽은 UI 구조

```text
Effect Tool
  Effect Type: Mesh / Texture / Particle / Decal / Trail
  Reset / CreateEffect / resource update
  resource palette: Base / Noise / Mask / Emissive / Dissolve

Model View
  실제 scene/model/effect preview

Effect Detail
  선택 effect의 위치·회전·스케일·수명 등 속성

All Effects
  preview instance reset/delete/clear

Data Files
  named authoring document save/load
```

이것은 한 화면의 배치일 뿐 데이터 소유권은 합치지 않는다. Effect asset은 effect-local 모양과 수명,
Animation cue는 clip time과 attachment binding, Preview owner는 현재 model pose와 anchor transform을 소유한다.

## 4. 삭제·보존 범위

삭제:

- `Client/Public/Effect_Types.h`
- `Client/Public|Private/Effect_AssetIO.*`
- `Client/Public|Private/Effect_ParticleSimulator.*`
- `Client/Public|Private/Effect_Runtime.*`
- `Client/Public|Private/Effect_ResourceCatalog.*`
- `Client/Bin/ShaderFiles/Shader_Effect*.hlsl|hlsli` 4개
- `Tools/EffectResourceIntake`의 Git 관리 파일 전체
- `Data/Effects/Authored`의 구 포맷 `.effect` 459개
- `Client/Default/Client.cpp`의 `--effect-*` CLI와 구 parser/simulator smoke
- 위 파일의 `Client.vcxproj`, `.filters`, `ProjectAudit` 참조

새 전체 교체:

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`

보존:

- Animation Tool, AnimationAuthoringBridge, Preview Model/Character 경로
- `Data/Effects/SourceCatalog`, `SourceExtracted`: 로컬 원본 추출 증거이며 새 runtime 정본이 아니다.
- `Client/Bin/Resources/Effect`: 외부 immutable resource pack payload이며 소스 변경에 포함하지 않는다.
- 날짜별 기존 RESULT와 review 문서: 당시 구현 증거이며 현재 정본으로 소비하지 않는다.

## 5. 자료구조와 불변식

```text
EFFECT_TYPE_SELECTION
  MESH, TEXTURE, PARTICLE, DECAL, TRAIL, END

m_eSelectedEffectType
  CEffect_Tool 한 instance가 소유하는 session-only 값이다.
  기본값은 참고 이미지와 같은 MESH다.
  파일, vector index, runtime object, Animation document에 저장하지 않는다.
```

G0 불변식:

- 다섯 값 중 하나만 선택된다.
- 선택 변경은 ImGui frame의 Tool 상태만 바꾼다.
- 파일 IO, asset scan, D3D resource 생성, GameObject 생성, Animation 변경을 하지 않는다.
- F1 Developer Tools의 Effect Tool 선택 외 새 단축키와 Level 전환을 만들지 않는다.

## 6. 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h` | G0 selection session 선언 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp` | 다섯 ImGui radio selection 렌더 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.cpp` | 구 effect CLI/smoke 제거 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | 삭제 source/data/shader 등록 제거 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | 삭제 item/filter 등록 제거 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/ImGuiLayer.cpp` | 구 `--effect-reset-layout` 전용 CLI 제거 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | 삭제된 구 runtime/intake/data check 제거 |

## 7. 파일별 전체 구현 코드

### 7-1. C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h

변경 종류: 전체 교체

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

### 7-2. C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp

변경 종류: 전체 교체

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

### 7-3. C:/Users/user/Desktop/LostArk/Client/Default/Client.cpp

변경 종류: 블록 삭제

- `Effect_AssetIO.h`, `Effect_ParticleSimulator.h`, `<fstream>` include를 삭제한다.
- effect CLI helper namespace 전체를 삭제한다.
- `wWinMain` 앞부분의 `--effect-sim`, `--effect-roundtrip`, `--effect-phase2-test` 분기를 삭제한다.
- 일반 Client 시작, timer, message loop, ImGui window procedure는 그대로 보존한다.

### 7-4. 프로젝트와 Audit 동기화

- 삭제 source의 `ClInclude`/`ClCompile` item만 제거한다.
- 삭제 HLSL의 `None`/`FxCompile` item과 filter item만 제거한다.
- 삭제한 459개 `.effect`의 `None` item과 `96.DataFiles/Effects` item만 제거한다.
- `Effect_Tool.h/.cpp` 기존 등록은 유지한다. 새 파일/Filter/GUID는 없다.
- ProjectAudit의 구 intake cook, runtime confinement, Dimensionist candidate admission check를 제거한다.
- 새 `effect.g0-selection-boundary` check로 삭제 경로 0건, 다섯 타입 selector, G0 비-IO 경계를 고정한다.
- Engine ImGui 초기화의 구 `--effect-reset-layout` 예외를 제거하고 공통 `imgui.ini` 계약만 유지한다.
- 보이는 창 제목은 유지하고 내부 ImGui ID를 `LostArkEffectToolG0`로 바꿔 구 `imgui.ini` 레이아웃과 분리한다.

## 8. 적용 순서와 검증

1. 참고 PNG를 `C:/Users/user/Desktop/툴`에 저장하고 hash를 기록한다.
2. 구 effect source/shader/intake/authored data와 모든 직접 consumer를 삭제한다.
3. `Effect_Tool.h/.cpp`를 G0 전체 코드로 교체한다.
4. project/filter XML을 parse하고 물리 파일·등록 일치를 확인한다.
5. `rg`로 구 `CEffect_Runtime`, `Effect_AssetIO`, `Effect_ParticleSimulator`, `.effect` project item이 0건인지 확인한다.
6. Client x64 Debug/Release를 빌드한다.
7. Debug Client에서 F1 → Effect Tool을 열고 다섯 radio가 상호 배타적으로 선택되는지 확인한다.
8. 선택 전후 Data/Resources/Animation 파일이 바뀌지 않는지 확인한다.
9. ProjectAudit와 `git diff --check`를 실행한다.

다음 G는 사용자가 G0의 H 계약, enum, Render 흐름을 확인한 뒤 연다. 참고 화면의 Reset/CreateEffect,
resource palette, Model View, Effect Detail, All Effects, Data Files를 한꺼번에 추가하지 않는다.

## 9. G1 — 메모리 Effect document와 안정 ID

### 9.1 목표와 종료 증거

G1은 `EffectAssetId + DisplayName + Elements`를 가진 authoring document 한 건을 메모리에 생성하고
명시적으로 폐기한다. `Particle` radio를 고르는 UI session state와 document 생성은 서로 독립이다.

종료 증거:

```text
Effect Asset ID와 Display Name 입력
-> 유효한 stable ID일 때만 Create Document 성공
-> Active Document에 formatVersion/id/name/elements=0 표시
-> radio 선택만으로 elements와 document identity는 바뀌지 않음
-> Discard Active Document로 메모리 document만 제거
-> Data/Resources/Animation/GPU resource 변화 0건
```

최신 `origin/main`의 gameplay `effectId`도 `^[A-Za-z0-9_.-]{1,128}$`를 사용하므로 G1의
`EffectAssetId`가 같은 문법을 사용한다. `Dimensionist -> DimensionMaster`, 공용
`CCharacterPreviewPanel`, skill `effectId` 변경은 현재 대규모 delete/rename conflict 때문에 아직
현재 브랜치에 merge하지 않는다. G1은 Preview와 gameplay를 소비하지 않으므로 해당 계약을 복제하지 않는다.

### 9.2 C1~C8와 불변식

```text
C1 기준계          EffectAssetId는 경로가 아닌 stable ID, Display Name은 표시 문자열이다.
C2 이동>계산       중요: G1은 파일 parse와 GPU load를 하지 않고 memory document만 만든다.
C3 공유는 비싸다   data-only header는 ImGui/D3D/Character를 include하지 않는다.
C4 수명은 선언된다 중요: optional이 no-document/active-document 두 상태를 표현한다.
C5 이산화와 오차   G1에는 time, transform, particle 수치가 없다.
C6 가지치기        stable ID와 display name만 검증하고 미래 renderer 필드를 만들지 않는다.
C7 권위와 정합성   중요: active document가 authoring draft, radio/input buffer는 session state다.
C8 검증이 병목     잘못된 ID와 빈 display name 거부, 기존 active document 보존을 확인한다.
```

G1 불변식:

- `EFFECT_DOCUMENT_DESC::strEffectAssetId`는 비어 있지 않고 stable ID 문법을 만족한다.
- `strDisplayName`은 공백만으로 구성되지 않으며 UTF-8 byte 기준 최대 64자 입력 buffer를 사용한다.
- `Elements`는 G1 내내 비어 있고 화면은 count만 읽는다.
- `EFFECT_ELEMENT_KIND::END`는 미초기화 Element를 검출하는 기본값이며 실제 Element로 commit하거나 저장하지 않는다.
- 새 document는 local staging 값을 완성한 뒤 `m_ActiveDocument`에 한 번 대입한다.
- active document가 있을 때 새 document 입력 UI를 노출하지 않아 암묵적 overwrite를 막는다.
- `Discard Active Document`는 파일이나 runtime instance가 아닌 memory draft만 제거한다.
- selected element kind, input buffer, status는 Effect document에 저장하지 않는다.

### 9.3 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h` | ImGui/D3D와 분리된 Effect document 자료형 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h` | G1 document와 ImGui session 수명 선언 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp` | stable ID 검증, create/discard, G0 selector 렌더 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | 새 data-only header 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | Effect Tool 필터에 새 header 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | G0 selector check를 G1 document boundary check로 승격 |

### 9.4 C:/Users/user/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h

변경 종류: 추가

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 1u;

enum class EFFECT_ELEMENT_KIND : uint8_t
{
	MESH,
	SPRITE,
	PARTICLE,
	DECAL,
	TRAIL,
	END
};

struct EFFECT_ELEMENT_DESC final
{
	std::string strElementId;
	EFFECT_ELEMENT_KIND eKind = EFFECT_ELEMENT_KIND::END;
	std::string strResourceId;
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

### 9.5 C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h

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

	void Render();

private:
	void Render_NewDocumentPanel();
	void Render_ActiveDocumentPanel();
	void Render_EffectTypeSelector();
	bool_t Try_CreateDocument();
	void Discard_ActiveDocument();

private:
	EFFECT_ELEMENT_KIND m_eSelectedEffectType =
		EFFECT_ELEMENT_KIND::MESH;
	std::array<char_t, 129> m_NewAssetId{};
	std::array<char_t, 65> m_NewDisplayName{};
	std::optional<EFFECT_DOCUMENT_DESC> m_ActiveDocument;
	std::string m_strDocumentStatus;
};

NS_END
```

### 9.6 C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp

변경 종류: 전체 교체

```cpp
#include "imgui.h"

#include "Effect_Tool.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace
{
	constexpr const char_t* EFFECT_TYPE_LABELS[] = {
		"Mesh", "Sprite", "Particle", "Decal", "Trail"
	};

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
	if (m_ActiveDocument.has_value())
	{
		m_strDocumentStatus =
			"Discard the active document before creating another one.";
		return false;
	}

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

	EFFECT_DOCUMENT_DESC StagedDocument;
	StagedDocument.strEffectAssetId = strAssetId;
	StagedDocument.strDisplayName = strDisplayName;
	m_ActiveDocument = std::move(StagedDocument);
	m_NewAssetId.fill('\0');
	m_NewDisplayName.fill('\0');
	m_strDocumentStatus =
		"Created in memory. This document has not been saved.";
	return true;
}

void Client::CEffect_Tool::Discard_ActiveDocument()
{
	m_ActiveDocument.reset();
	m_strDocumentStatus =
		"Discarded the in-memory document. No file was changed.";
}
```

### 9.7 C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj

변경 종류: 항목 추가

`Effect_Tool.h` 항목 바로 앞에 다음 항목을 추가한다.

```xml
    <ClInclude Include="..\public\Effect_AuthoringDocument.h" />
    <ClInclude Include="..\public\Effect_Tool.h" />
```

### 9.8 C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters

변경 종류: 항목 추가

`Effect_Tool.h` filter 항목 바로 앞에 다음 항목을 추가한다.

```xml
    <ClInclude Include="..\public\Effect_AuthoringDocument.h">
      <Filter>03. Tools\02. Effect</Filter>
    </ClInclude>
    <ClInclude Include="..\public\Effect_Tool.h">
      <Filter>03. Tools\02. Effect</Filter>
    </ClInclude>
```

### 9.9 C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1

변경 종류: 블록 교체

현재 `$effectToolHeader`부터 `effect.g0-selection-boundary`의 `Add-Check`까지를 다음으로 교체한다.

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
	$legacyEffectSymbolHits = @($activeFiles | Select-String -Pattern 'Effect_(AssetIO|ParticleSimulator|Runtime|ResourceCatalog|Types)|CEffect_Runtime|EFFECT_ASSET_DESC')
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

### 9.10 사용자가 작성할 순서

1. `Effect_AuthoringDocument.h`를 새 UTF-8 BOM 없음 파일로 만들고 data-only 타입만 작성한다.
2. `Effect_Tool.h`에서 G0 private enum을 제거하고 document header의 `EFFECT_ELEMENT_KIND`를 사용한다.
3. input buffer, optional active document, status 멤버와 다섯 private 함수를 선언한다.
4. `Effect_Tool.cpp`의 stable ID/display name helper를 작성한다.
5. `Render_NewDocumentPanel`, `Try_CreateDocument`를 작성해 staged memory create를 닫는다.
6. `Render_ActiveDocumentPanel`, `Discard_ActiveDocument`를 작성한다.
7. G0 selector를 `Next Element Type`으로 유지하되 `Texture` label을 `Sprite`로 바로잡는다.
8. `.vcxproj`와 `.filters`에 새 header 한 건만 등록한다.
9. ProjectAudit check를 G1 document boundary로 교체한다.

### 9.11 검증

```text
잘못된 ID: 빈 값, slash, backslash, space, 129자 -> 생성 실패
잘못된 이름: 빈 값, 공백만 -> 생성 실패
정상: dimensionmaster.altv.portal_open + DimensionMaster Alt-V Portal -> 생성 성공
Active Document: version=1, ID/name 동일, Elements=0
radio 변경: Active Document identity와 Elements=0 유지
Discard: no active document 복귀, 파일 변화 0건
```

자동 검증:

1. `Client.vcxproj`, `.filters` XML parse.
2. Client x64 Debug build.
3. `effect.g1-document-boundary` PASS.
4. `git diff --check`.

수동 검증:

1. `Client/Default` 작업 디렉터리에서 Debug Client 실행.
2. F1 -> Effect Tool.
3. 위 invalid/valid ID와 radio/discard 흐름 재현.
4. 실행 전후 `Data`, `Resources`, `Animation` 파일 변화 0건 확인.

## 10. G2 — Effect Element 정체성과 원자적 추가

### 10.1 목표와 종료 증거

G2는 Active Effect Document 안에 stable Element ID와 `Mesh / Sprite / Particle / Decal / Trail`
종류만 가진 Element 한 건을 추가한다. 기존 참고 툴의 Effect Type은 다음에 만들 Element의 제작 단위이며,
Niagara의 Emitter/Renderer 분리는 Particle과 Trail 내부 동작이 실제로 열리는 이후 G에서 도입한다.

```text
입력
  Next Element Type = Particle
  Element ID        = portal_sparks

Add Element
  -> Active Document 전체를 local staging copy
  -> portal_sparks + PARTICLE 추가
  -> 중복과 kind 검증
  -> 검증 성공 뒤 Active Document에 한 번만 commit

종료 증거
  선택만 함       -> Elements 0
  정상 Add        -> Elements 1
  같은 ID 재시도 -> 실패, Elements 1 유지
  다른 ID Add     -> Elements 2
  파일/GPU/Animation/Shader 변화 없음
```

G2는 Resource ID, Base/Noise/Mask/Emissive/Dissolve, Effect Detail, Preview Object, Particle Instance,
Save/Load를 추가하지 않는다. 범용 `strResourceId` 한 개는 Mesh, Texture 다섯 슬롯, Particle Renderer의
서로 다른 소유권을 표현하지 못하므로 G2 data-only Element에서 제거한다. 실제 Resource ID는 G4에서
각 타입의 payload와 Material slot에 추가한다.

### 10.2 C1~C8와 불변식

```text
C1 기준계          중요: Element ID는 한 Effect Document 안에서 유일한 stable ID다.
C2 이동>계산       중요: Add는 CPU authoring data만 변경하고 GPU resource를 생성하지 않는다.
C3 공유는 비싸다   Element는 Character, Animation, Shader pointer를 소유하지 않는다.
C4 수명은 선언된다 Document가 Elements를 값으로 강하게 소유하고 Discard 때 함께 제거한다.
C5 이산화와 오차   G2에는 time, transform, particle 수치가 없다.
C6 가지치기        ID와 kind만 commit하고 Resource/Material의 미래 placeholder를 만들지 않는다.
C7 권위와 정합성   중요: Active Document만 현재 authoring 정본이고 radio/input은 session draft다.
C8 검증이 병목     중복·잘못된 ID·END kind 실패 후 기존 Document 불변을 수동 확인한다.
```

G2 불변식:

- `m_eSelectedEffectType` 변경만으로 `Elements`가 변하지 않는다.
- `m_NewElementId` 입력만으로 `Elements`가 변하지 않는다.
- `Try_AddElement()` 성공 뒤에만 `Elements.size()`가 1 증가한다.
- Element ID는 1~128 ASCII 영문, 숫자, `.`, `_`, `-`만 허용한다.
- 같은 Document 안에서 Element ID는 중복될 수 없다.
- `EFFECT_ELEMENT_KIND::END`는 commit할 수 없다.
- `Elements`의 vector index는 저장 ID가 아니다.
- Add 실패 시 Active Document, 입력 buffer, 기존 Element 목록을 그대로 유지한다.
- Add 성공 시에만 Element ID 입력 buffer를 비운다.
- G2에는 파일 IO, Resources scan, D3D/GPU, Preview, Animation 변경이 없다.

### 10.3 Effect Element, Particle Emitter, Renderer의 층위

```text
EffectAsset
  완성 이펙트 한 개. 예: dimensionmaster.altv.portal

EffectElement
  완성 이펙트를 구성하는 제작 단위.
  예: portal_mesh, portal_sparks, portal_ground_decal

Particle Emitter
  Particle Element 내부에서 개수, 생성 시점, 위치, 속도, 방향, 수명을 계산한다.

Particle Renderer
  Emitter가 계산한 Particle 상태를 Sprite, Mesh, Ribbon과 Shader Pass로 화면에 그린다.
```

G2의 `EFFECT_ELEMENT_KIND`는 Niagara Renderer enum이 아니다. 기존 참고 툴과 같은 상위 제작 단위다.

```text
Mesh Element
  이후 Mesh Resource + Material + Effect Detail + Mesh Preview를 소유

Sprite Element
  이후 Sprite Texture/Atlas + Billboard + Material + Sprite Preview를 소유

Particle Element
  이후 Emitter/Module + Particle Renderer + Material을 소유

Decal Element
  이후 Projection + Decal Material을 소유

Trail Element
  이후 Trail point 생성 + Ribbon Renderer + Material을 소유
```

### 10.4 Shader Technique/Pass 메모 — G5 계약

현재 Engine `CShader`는 HLSL Effect를 compile한 뒤 `GetTechniqueByIndex(0)`의 Pass 수와 Input Layout을
준비한다. Render 호출의 `CShader::Begin(iPassIndex)`는 첫 Technique 안의 지정 Pass를 적용한다.

```text
Technique
  Shader가 제공하는 Pass 묶음. 현재 Engine은 index 0 하나만 소비한다.

Pass
  Rasterizer State
  DepthStencil State
  Blend State
  Vertex Shader
  Geometry Shader
  Pixel Shader
  위 상태와 프로그램 조합 한 건
```

기존 `Shader_VtxMeshBinary.hlsl`도 Opaque, Alpha, Additive와 Back/Front/TwoSided 조합을 서로 다른
Pass로 제공한다. 숫자 Pass index는 HLSL 선언 순서가 바뀌면 의미가 달라질 수 있으므로 새 Effect 파일에
raw index를 정본으로 저장하지 않는다. G5 Material 계약은 다음 stable profile을 저장하고 Runtime mapper가
실제 Pass index를 결정한다.

```text
Blend Mode  = Opaque / AlphaBlend / Additive
Cull Mode   = Back / Front / TwoSided
Depth Mode  = Default / ReadOnly / Disabled
Shader Profile ID
```

G5의 ImGui `Select Pass`는 사람이 읽는 Render Profile을 선택하는 UI로 만들고, 현재 Shader가 지원하지
않는 조합은 validate에서 거부한다. G2에는 Pass, Shader, Texture, Material 필드를 추가하지 않는다.

### 10.5 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h` | G2 Element identity와 Document 소유권 선언 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h` | Element draft buffer, Add command, list UI 계약 선언 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp` | Element 검증, staging/commit, 목록 표시 구현 |
| 블록 교체 | `C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | G1 check를 G2 Element boundary check로 승격 |

새 C++ 파일은 없다. `Effect_AuthoringDocument.h`, `Effect_Tool.h`, `Effect_Tool.cpp`의 기존
`.vcxproj`와 `.vcxproj.filters` 등록을 유지하며 새 project/filter item을 추가하지 않는다.

### 10.6 C:/Users/user/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h

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

### 10.7 C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h

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

### 10.8 C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp

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

### 10.9 C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1

변경 종류: 블록 교체

적용 위치: 기존 `$effectDocumentHeader`부터 `effect.g1-document-boundary` `Add-Check`까지 전체를 교체한다.

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

### 10.10 사용자가 작성할 순서

1. `Effect_AuthoringDocument.h`에서 범용 `strResourceId`를 제거하고 G2 identity-only Element를 확정한다.
2. `Effect_Tool.h`에 Element ID buffer, Element status, Add/List 함수 선언을 추가한다.
3. `Effect_Tool.cpp`의 stable ID helper를 Asset/Element 공용 `Is_StableAuthoringId`로 바꾼다.
4. `Render()`를 G2 흐름으로 바꾸고 Discard 직후 optional을 다시 확인한다.
5. `Render_AddElementPanel()`을 작성한다.
6. `Render_ElementList()`를 작성한다.
7. `Try_AddElement()`의 active/kind/ID/duplicate 검증을 작성한다.
8. local Document copy에 staged Element를 넣고 optional에 한 번만 commit한다.
9. 성공 시에만 Element ID buffer를 비우고 실패 시 입력과 기존 Document를 보존한다.
10. Discard 때 Element draft와 Element status를 정리한다.
11. ProjectAudit를 `effect.g2-element-boundary`로 승격한다.

### 10.11 검증

수동 검증:

```text
Document 없음 + Add UI: Add 버튼이 표시되지 않음
정상 Document 생성: Elements 0
Particle radio 선택만 함: Elements 0
portal_sparks 입력만 함: Elements 0
Add Element: portal_sparks / Particle, Elements 1
같은 portal_sparks 재입력: 중복 실패, Elements 1 유지, 입력값 보존
portal_mesh / Mesh 추가: Elements 2
radio를 Trail로 변경: 기존 두 Element kind 불변
잘못된 ID: 빈 값, slash, backslash, space, 129자 거부
Discard: Document와 Elements 제거, Element draft 초기화
실행 전후 Data/Resources/Animation/Shader 파일 변화 0건
```

자동 검증:

1. `Client/Default/Client.vcxproj`, `.filters` XML parse.
2. Client x64 Debug build.
3. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`의 `effect.g2-element-boundary` PASS.
4. `git diff --check`.

### 10.12 다음 G의 고정 경계

```text
G3 Save/Load v1
  EffectAssetID, DisplayName, Element ID, Kind JSON
  parse -> validate -> stage -> commit
  잘못된 version/ID/kind/duplicate rollback

G4 Asset Catalog와 타입별 Resource binding
  Mesh Resource, Sprite Atlas, Decal Resource, Trail Resource
  Resources-relative stable Asset ID

G5 Material Input와 Shader Render Profile
  Base, Noise, Mask, Emissive, Dissolve
  Blend/Cull/Depth profile -> Technique 0의 실제 Pass mapping

G6 Effect Detail과 Stateless Preview
  Transform, Color, UV, Lifetime, Emissive, Dissolve
  Mesh/Sprite/Decal Preview

G7 Timeline과 Curve
G8 Particle/Trail Module Stack
G9 결정적 CPU Particle Simulation
G10 GPU Particle, Distortion, HDR/Bloom, Profiler
G11 Character/Weapon/Model Preview
G12 Animation Cue와 제품 Runtime 연결
G13 고급 Module Node Graph와 Compiler
```
