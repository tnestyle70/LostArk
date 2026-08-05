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
