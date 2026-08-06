# Effect Tool G2 — Add Element 전체 코드 (Claude 세션 계획서)

작성 주체: Claude 세션. 코덱스 정본 [2026-08-03_EFFECT_TOOL_ADVANCEMENT_PLAN.md](2026-08-03_EFFECT_TOOL_ADVANCEMENT_PLAN.md)(G0~G1)의
G2 전체 코드 동반 문서다. 대화 설명 형식은 `.md/local.계획서예시.md`를 따르고, 이 문서는
그 §4 규칙대로 사용자가 그대로 대조할 수 있는 생략 없는 전체 반영 코드를 싣는다.
코덱스가 정본 PLAN에 G2를 편입하면 이 문서는 그 절과 같은 계약을 가리켜야 한다.

```text
문서 유형: 구현 계획서
출력 모드: CODE_WITH_EXPLANATION (설명은 대화에서 수행, 이 문서는 코드 정본)
C1~C8: OFF
문제 해결 ①~⑤: OFF
자료구조·알고리즘: OFF
```

## 0. 기준선 (S0 체크포인트 기록)

- 실측: `git rev-list --count HEAD..origin/main` = 0, `origin/main..HEAD` = 0.
  브랜치 `codex/effect-tool-reboot`는 이미 origin/main과 같은 지점이었고 DimensionMaster
  병합(8edcc0b)과 공용 `CharacterPreviewPanel`이 포함되어 있었다. "6커밋 뒤"는 병합 이전
  시점의 보고였다.
- 미완이던 것은 병합이 아니라 커밋. G0/G1 작업 전체를 체크포인트 커밋 `4152b8a`
  (502 files, +1,144/−101,729)로 닫았다. untracked였던 `Effect_AuthoringDocument.h`와
  RESULT 문서를 포함했고, CharacterSelect 서버 진입 스트림의 unstaged 변경과 개인 파일은
  제외해 보존했다.
- G2는 이 커밋을 기준선으로 한다.

## 1. 목표와 종료 증거

G2는 Active Document에 `Element ID + 종류 + Resource ID`를 가진 Element를 추가하는
유일한 변경 경로 `Try_AddElement`를 만든다. radio 선택은 계속 session 상태만 바꾼다.

```text
Active Document 존재
-> Element ID, Resource ID 입력 + 현재 radio 종류로 Add Element
-> 유효할 때만 Elements 0 -> 1, 화면 count 갱신
-> 중복 Element ID 거부, 빈 ID 거부
-> 절대 경로, drive 문자, '\', '..', 빈 segment Resource ID 거부
-> radio만 변경 시 Elements 수 불변
-> Data/Resources/GPU 변화 0건
```

Resource ID는 이 단계에서 문법만 검증한다. 파일 존재와 종류별 확장자 검사는 G6
Resource Catalog admission의 책임이며, 여기서 하면 G2가 파일 IO 경계를 넘는다.

## 2. G2 불변식

- `Elements`가 변하는 코드 경로는 `Try_AddElement`의 `push_back` 한 곳뿐이다
  (Discard의 전체 제거 제외).
- 같은 Document 안에서 `strElementId`는 유일하다. 중복은 commit 전에 거부된다.
- `strResourceId`는 `'/'` 구분의 상대 ID이며 `'\'`, 선행 `'/'`, drive 문자,
  `.`/`..` segment, 빈 segment를 포함하지 않는다.
- `eKind`는 commit 시점의 radio 선택 값이며 `END`로 commit되는 경로가 없다.
- staged Element는 모든 검증 통과 후 완성된 값으로 한 번만 commit된다.
- 실패한 Add는 Document, 입력 buffer, 기존 Elements를 그대로 보존하고 status만 바꾼다.
- 창 ID는 `###LostArkEffectTool`로 stage 중립화한다. G 단계마다 ID를 바꾸면 ImGui 창
  배치가 매번 초기화되므로 이번 한 번만 바꾸고 이후 고정한다.

## 3. 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h` | Element 입력 buffer 2종과 Add 함수 선언 추가 |
| 전체 교체 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp` | Resource ID 검증, Add Element 패널, Try_AddElement commit |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | `effect.g1-document-boundary`를 `effect.g2-element-commit`으로 승격 |

새 파일과 새 프로젝트/필터 등록은 없다. `Effect_AuthoringDocument.h`는 G1 그대로 사용한다
(`EFFECT_ELEMENT_DESC`가 이미 G2가 필요한 세 필드를 가진다).

## 4. C:/Users/user/Desktop/LostArk/Client/Public/Effect_Tool.h

변경 종류: 전체 교체 (UTF-8 BOM 없음 유지)

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
	// 메모리에 commit된 Active Document의 요약 정보를 읽기 전용으로 그린다.
	void Render_ActiveDocumentPanel();
	// Document를 변경하지 않고 다음에 추가할 Element 종류만 선택한다.
	void Render_EffectTypeSelector();
	// Active Document가 있을 때 Element ID, Resource ID 입력과 Add 버튼을 그린다.
	void Render_AddElementPanel();
	// 입력을 검증하고 local Document를 staging한 뒤 한 번에 commit한다.
	bool_t Try_CreateDocument();
	// Element 입력을 검증하고 staged Element 하나를 Elements에 한 번에 commit한다.
	bool_t Try_AddElement();
	// 파일이나 GPU 객체를 건드리지 않고 메모리 초안만 제거한다.
	void Discard_ActiveDocument();

private:
	// Add Element가 commit 시점에 읽는 ImGui session 선택 상태다.
	EFFECT_ELEMENT_KIND m_eSelectedEffectType =
		EFFECT_ELEMENT_KIND::MESH;
	// 최대 128바이트 ID와 문자열 종료 문자를 담는 편집 buffer다.
	std::array<char_t, 129> m_NewAssetId{};
	// 최대 64바이트 표시 이름과 문자열 종료 문자를 담는 편집 buffer다.
	std::array<char_t, 65> m_NewDisplayName{};
	// 최대 128바이트 Element ID와 문자열 종료 문자를 담는 편집 buffer다.
	std::array<char_t, 129> m_NewElementId{};
	// 최대 256바이트 Resources 상대 Resource ID와 종료 문자를 담는 buffer다.
	std::array<char_t, 257> m_NewResourceId{};
	// nullopt이면 New Document UI, 값이 있으면 Active Document UI를 그린다.
	std::optional<EFFECT_DOCUMENT_DESC> m_ActiveDocument;
	// 마지막 검증, 생성, 추가, 폐기 결과를 사용자에게 보여주는 상태 문구다.
	std::string m_strDocumentStatus;
};

NS_END
```

## 5. C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp

변경 종류: 전체 교체 (UTF-8 BOM 없음 유지)

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

	// Effect Asset ID와 Element ID가 함께 쓰는 안정적인 ID 문법인지 검증한다.
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

	// Resources 루트를 벗어나지 못하는 상대 Resource ID 문법인지 검증한다.
	// 파일 존재와 종류별 확장자 검사는 G6 Resource Catalog가 수행한다.
	bool_t Is_RelativeResourceId(const std::string& strValue)
	{
		// 빈 값과 비정상적으로 긴 ID를 먼저 거부한다.
		if (strValue.empty() || strValue.size() > 256u)
			return false;
		// canonical 구분자는 '/' 하나다. '\\'는 저장 계약에서 제외한다.
		if (std::string::npos != strValue.find('\\'))
			return false;
		// 루트 시작('/...')은 절대 경로이므로 거부한다.
		if ('/' == strValue.front())
			return false;
		// drive 문자("C:...")도 Resources 밖을 가리키므로 거부한다.
		if (strValue.size() >= 2u && ':' == strValue[1])
			return false;

		// segment 단위로 빈 segment, ".", ".." 탈출, 허용 문자 집합을 검증한다.
		size_t iBegin = 0;
		for (;;)
		{
			const size_t iEnd = strValue.find('/', iBegin);
			const std::string strSegment = strValue.substr(iBegin,
				(std::string::npos == iEnd ? strValue.size() : iEnd) - iBegin);
			if (strSegment.empty() || "." == strSegment || ".." == strSegment)
				return false;
			if (!Is_StableAuthoringId(strSegment))
				return false;
			if (std::string::npos == iEnd)
				break;
			iBegin = iEnd + 1u;
		}
		return true;
	}
}

void Client::CEffect_Tool::Render()
{
	// ImGui 창이 접혀 Begin이 false를 반환해도 End는 반드시 호출한다.
	ImGui::SetNextWindowSize(ImVec2(620.f, 420.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Effect Tool###LostArkEffectTool"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"G2: author one in-memory Effect document and its elements. "
		"Nothing is saved, loaded, previewed, or rendered yet.");
	// optional에 Document가 있는지에 따라 두 패널 중 하나만 그린다.
	if (m_ActiveDocument.has_value())
		Render_ActiveDocumentPanel();
	else
		Render_NewDocumentPanel();

	ImGui::Separator();
	Render_EffectTypeSelector();
	// Element 추가 UI는 Active Document가 있을 때만 노출한다.
	if (m_ActiveDocument.has_value())
		Render_AddElementPanel();
	else
		ImGui::TextDisabled(
			"Create a document first. Elements change only through Add Element.");
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
	// const reference로 읽어 렌더 경로에서 Document가 변경되지 않게 한다.
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
	// radio 선택은 session 상태만 바꾸며 Add Element commit 시점에만 소비된다.
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
	// 입력 buffer는 session 상태이고 Document는 Add Element 순간에만 변한다.
	ImGui::SeparatorText("Add Element");
	ImGui::InputText("Element ID", m_NewElementId.data(),
		m_NewElementId.size());
	ImGui::InputText("Resource ID", m_NewResourceId.data(),
		m_NewResourceId.size());
	ImGui::TextDisabled(
		"Resource ID is Resources-relative, e.g. Effect/Textures/fx_spark.dds");

	if (ImGui::Button("Add Element"))
		Try_AddElement();
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
	m_strDocumentStatus =
		"Created in memory. This document has not been saved.";
	return true;
}

bool_t Client::CEffect_Tool::Try_AddElement()
{
	// UI가 가려도 계약으로 한 번 더 막아 Document 없는 commit을 차단한다.
	if (!m_ActiveDocument.has_value())
	{
		m_strDocumentStatus = "Create a document before adding elements.";
		return false;
	}

	// 부분 commit을 막기 위해 ImGui buffer를 먼저 문자열로 복사한다.
	const std::string strElementId = m_NewElementId.data();
	const std::string strResourceId = m_NewResourceId.data();
	if (!Is_StableAuthoringId(strElementId))
	{
		m_strDocumentStatus =
			"Element ID must be 1-128 ASCII letters, digits, '.', '_' or '-'.";
		return false;
	}

	// 같은 ID가 두 번 있으면 이후 선택, 저장, Animation binding이 모호해진다.
	EFFECT_DOCUMENT_DESC& Document = *m_ActiveDocument;
	const bool_t hasDuplicateId = std::any_of(Document.Elements.begin(),
		Document.Elements.end(),
		[&strElementId](const EFFECT_ELEMENT_DESC& Element)
		{
			return Element.strElementId == strElementId;
		});
	if (hasDuplicateId)
	{
		m_strDocumentStatus = "Element ID already exists in this document.";
		return false;
	}
	if (!Is_RelativeResourceId(strResourceId))
	{
		m_strDocumentStatus =
			"Resource ID must be a Resources-relative ID without '..', '\\' or a drive letter.";
		return false;
	}

	// 검증을 통과한 값으로 staged Element를 완성한 뒤 한 번만 commit한다.
	EFFECT_ELEMENT_DESC StagedElement;
	StagedElement.strElementId = strElementId;
	StagedElement.eKind = m_eSelectedEffectType;
	StagedElement.strResourceId = strResourceId;
	Document.Elements.push_back(std::move(StagedElement));

	m_NewElementId.fill('\0');
	m_NewResourceId.fill('\0');
	m_strDocumentStatus =
		"Added element in memory. Nothing was saved or uploaded.";
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

## 6. C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1

변경 종류: 블록 교체. G1이 넣은 `$effectG1DocumentShape` 정의부터
`Add-Check 'effect.g1-document-boundary'`까지를 다음으로 교체한다. 그 위의
`$effectDocumentHeader`~`$legacyEffectEntry` 수집 블록은 그대로 둔다.

```powershell
	$effectG2DocumentShape =
		$effectDocumentHeader -match 'EFFECT_AUTHORING_FORMAT_VERSION\s*=\s*1u' -and
		$effectDocumentHeader -match 'enum class EFFECT_ELEMENT_KIND[\s\S]*MESH,[\s\S]*SPRITE,[\s\S]*PARTICLE,[\s\S]*DECAL,[\s\S]*TRAIL,[\s\S]*END' -and
		$effectDocumentHeader -match 'struct EFFECT_DOCUMENT_DESC[\s\S]*strEffectAssetId[\s\S]*strDisplayName[\s\S]*Elements' -and
		$effectToolHeader -match 'optional<EFFECT_DOCUMENT_DESC>\s+m_ActiveDocument' -and
		$effectToolHeader -match 'm_eSelectedEffectType\s*=\s*EFFECT_ELEMENT_KIND::MESH' -and
		$effectToolSource -match 'Is_StableAuthoringId' -and
		$effectToolSource -match 'Is_RelativeResourceId' -and
		$effectToolSource -match 'Try_CreateDocument' -and
		$effectToolSource -match 'Try_AddElement' -and
		$effectToolSource -match 'Discard_ActiveDocument' -and
		([regex]::Matches($effectToolSource, [regex]::Escape('Elements.push_back'))).Count -eq 1 -and
		$effectToolSource -match '"Mesh",\s*"Sprite",\s*"Particle",\s*"Decal",\s*"Trail"' -and
		$effectToolSource -match 'LostArk Effect Tool###LostArkEffectTool' -and
		$effectToolSource -notmatch 'LostArkEffectToolG1' -and
		$effectToolSource -notmatch 'filesystem|ifstream|ofstream|GetOpenFileName|ID3D11'
	Add-Check 'effect.g2-element-commit' (
		$removedEffectPathHits.Count -eq 0 -and
		$authoredEffectFiles.Count -eq 0 -and
		$effectIntakeFiles.Count -eq 0 -and
		$effectShaderFiles.Count -eq 0 -and
		$legacyEffectSymbolHits.Count -eq 0 -and
		$legacyEffectProjectHits.Count -eq 0 -and
		-not $legacyEffectEntry -and
		$effectG2DocumentShape) "paths=$($removedEffectPathHits.Count) authored=$($authoredEffectFiles.Count) intake=$($effectIntakeFiles.Count) shaders=$($effectShaderFiles.Count) symbols=$($legacyEffectSymbolHits.Count) project=$($legacyEffectProjectHits.Count) entry=$legacyEffectEntry document=$effectG2DocumentShape"
```

`Elements.push_back` 출현 횟수 1 검사가 "Elements를 바꾸는 경로는 하나" 불변식을
자동으로 지킨다. `-notmatch 'LostArkEffectToolG1'`은 창 ID stage 중립화 적용을 확인한다.

## 7. 적용 순서와 검증

적용 순서:

1. `Effect_Tool.h`에 buffer 2종과 `Render_AddElementPanel`, `Try_AddElement` 선언 추가.
2. `Is_StableEffectAssetId` -> `Is_StableAuthoringId` 개명과 호출부 동시 수정.
3. `Is_RelativeResourceId` 작성 — "a//b", "a/./b", "../a"를 손으로 넣어 분기 확인.
4. `Render()` 분기와 창 ID `###LostArkEffectTool` 변경.
5. `Render_AddElementPanel` 작성.
6. `Try_AddElement` 작성 — 검증 4단계 전부 통과 후에만 `push_back` 1회.
7. ProjectAudit 블록 교체.
8. 빌드와 아래 검증.

runtime 검증:

```text
정상: portal_mesh + Effect/Meshes/ring0.wmodel + Mesh -> Elements 0 -> 1
중복: 같은 portal_mesh 재추가 -> 거부, Elements 1 유지
빈 ID: "" -> 거부
탈출: ../secret, /abs, C:/x, a\b, a//b, a/./b -> 전부 거부
radio: Sprite 변경만 -> Elements 수 불변
Discard -> Document와 Elements 함께 제거, 파일 변화 0건
```

자동 검증:

1. Client x64 Debug build.
2. `effect.g2-element-commit` PASS (`effect.g1-document-boundary`는 제거됨).
3. `git diff --check`.

수동 검증:

1. `Client/Default` 작업 디렉터리에서 Debug Client 실행, F1 -> Effect Tool.
2. 위 정상/중복/빈/탈출/radio/Discard 흐름을 순서대로 재현.
3. 실행 전후 `Data`, `Client/Bin/Resources` 파일 변화 0건 확인.
