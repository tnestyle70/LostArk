#pragma once

// Client 이름 공간과 STATIC Level 구분을 사용한다.
#include "Client_Defines.h"
// DX11 ComPtr와 시간·글자·색의 공통 타입을 사용한다.
#include "Engine_Defines.h"
// 입력 HUD_PLAYER_STATE의 action·서버 tick·직업을 읽는다.
#include "CombatHUDViewModel.h"

// 레이아웃 두 개를 unique_ptr로 소유하며 caption은 문자열 값으로 보관한다.
#include <memory>
#include <string>

NS_BEGIN(Client)

// 레이아웃 포인터만 선언하고 생성·슬롯 조작은 CPP에 둔다.
class CUILayoutRuntime;

// MainApp이 소유하는 스퀘어홀 연주 게이지·화면 암전이다. 복제된 action의 tick으로 진행도를 읽는다.
// Bern의 TRIGGER_MOVE 암전도 표시하지만 이동 승인이나 도착 위치는 결정하지 않는다.
class CSongCastGaugeView final
{
public:
	// MainApp 초기화에서 게이지·암전 레이아웃을 만들고 caption JSON을 읽는다.
	CSongCastGaugeView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	// 레이아웃·장치의 보유 참조를 정리한다. Layer의 sprite 참조는 별도다.
	~CSongCastGaugeView();

public:
	// MainApp이 현재 HUD player를 전달한다. 연주 밖에서는 게이지를 숨기고 필요하면 이동 암전을 갱신한다.
	void Update(f32_t fTimeDelta, const HUD_PLAYER_STATE& Player, bool_t bTravelFadeLevel);
	// MainApp의 ImGui EndFrame 이후 글자 패스에서 연주 caption을 그린다.
	void Render_Text();
	// 게이지만 숨긴다. 암전은 Update_Fade가 별도로 해제한다.
	void Hide();

private:
	// WorldMapPanels.json의 strings.songCasting을 caption으로 읽는다.
	void Load_Strings();
	// 서버 action으로 암전 시작·해제를 정하고 초 단위 delta로 alpha를 변화시킨다.
	void Update_Fade(f32_t fTimeDelta, bool_t bInSong, int32_t iAgeTicks, bool_t bInTravel);

private:
	// 장치·Context 참조와 게이지 레이아웃을 보유한다. Player 입력은 복사 보관하지 않는다.
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	unique_ptr<CUILayoutRuntime>	m_pView;
	wstring_t						m_strCaption;
	bool_t							m_bVisible = false;
	// 같은 action 시작 tick의 연주 소리를 한 번만 재생하기 위한 표시 상태다.
	uint32_t						m_iShownActionStartTick = 0u;
	f32_t							m_fFill = 0.f;
	// KoukuFadeUI.json의 전체 화면 검정 슬롯을 소유한다. 게이지와 별도로 암전을 진행한다.
	unique_ptr<CUILayoutRuntime>	m_pFadeView;
	f32_t							m_fFadeAlpha = 0.f;
	bool_t							m_bFadingOut = false;
	bool_t							m_bFadeShown = false;
};

NS_END
