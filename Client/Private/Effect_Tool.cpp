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
