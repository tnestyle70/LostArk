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
