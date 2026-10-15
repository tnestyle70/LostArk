#pragma once

// Client 이름 공간과 Level 공통 선언을 사용한다.
#include "Client_Defines.h"
// DX11 ComPtr와 float·문자열 공통 타입을 사용한다.
#include "Engine_Defines.h"

// 레이아웃 단독 소유와 Update의 캐릭터 공유 참조를 선언한다.
#include <memory>
// stable trigger ID와 화면 문구를 값으로 보관한다.
#include <string>
// 문서에서 추린 상호작용 상자 목록을 보관한다.
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;
class CCharacter;

// Level이 전달한 상호작용 대상에 맞춰 머리 위 행동 아이콘·문구·G 키를 표시한다.
// Server 제안 ID 또는 자동 발동 상자 안 위치를 표시 조건으로 쓰며 명령 전송은 담당하지 않는다.
class CInteractKeyPromptView final
{
public:
	// Level 초기화에서 레이아웃을 만들고 Area의 viewer world 상자를 읽는다. 실패하면 목록은 비어 있다.
	void Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex, const char* pAreaId);
	// Level의 프레임 갱신이다. 빈 ID는 Server 제안 없음이며 캐릭터 참조는 이 호출에서만 사용한다.
	void Update(f32_t fTimeDelta, const std::shared_ptr<CCharacter>& pLocalCharacter,
		const std::string& strOfferedTriggerId, bool_t bShown);
	// Level의 Render에서 HUD 글자를 그린다. 이미지 슬롯은 Layer가 별도로 그린다.
	void Render_Text() const;
	// 읽은 문서의 첫 dock:<name> 상자 중심·yaw를 정박 표시에 제공한다. 없으면 출력값을 바꾸지 않고 false다.
	bool_t Try_Get_DockPoint(float3_t& vOutCenter, f32_t& fOutYawDegrees) const;

private:
	enum class ACTION : uint8_t { GODOWN, CLIMB, TIGHTROPE, CHECK, MOVE, LEVER, DOCK };
	struct TRIGGER
	{
		std::string strPlacementId;
		ACTION eAction = ACTION::CHECK;
		// DOCK일 때 dock:<name>에서 읽은 장소명을 행동 문구로 사용한다.
		std::wstring strLabel;
		// 월드 단위 상자다. 자동 발동 상자는 내부에 있을 때 아이콘만 보여 주며 발동 여부를 변경하지 않는다.
		bool_t bShowWhileInside = false;
		float3_t vCenter{};
		float3_t vHalfExtents{};
		f32_t fYawDegrees = 0.f;
	};
	// 이미지를 숨기고 현재 대상·등장 시간을 초기화한다. 슬롯 객체는 유지한다.
	void Hide();

private:
	// 레이아웃은 단독 소유한다. 소멸 시 보유 참조가 정리되며 Layer의 sprite 참조는 별도다.
	std::unique_ptr<CUILayoutRuntime>	m_pView;
	std::vector<TRIGGER>				m_Triggers;
	std::string							m_strShownTriggerId;
	ACTION								m_eAction = ACTION::CHECK;
	std::wstring							m_strLabel;
	f32_t								m_fShowSeconds = -1.f;
	// Update에서 계산한 화면 픽셀 단위 글자 중심·크기다.
	f32_t								m_fTextCenterX = 0.f;
	f32_t								m_fTextCenterY = 0.f;
	f32_t								m_fTextPx = 0.f;
	f32_t								m_fTextOffsetX = 0.f;
	bool_t								m_bVisible = false;
};

NS_END
