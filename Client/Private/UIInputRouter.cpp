// UI 사각형과 현재 버튼·문자 입력을 묶어 창별 소비와 게임 입력 차단을 중재한다.
#include "UIInputRouter.h"
// 원시 버튼 상태·viewport 조회와 게임 입력 차단을 사용한다.
#include "GameInstance.h"

// 처음 호출할 때 지역 static을 생성한다. 이후 같은 객체를 차용하며 정상 프로세스 종료 때 자동 소멸한다.
Client::CUIInputRouter& Client::CUIInputRouter::Get()
{
	static CUIInputRouter instance;
	return instance;
}

// MainApp::Update에서 한 번 호출해 각 창이 같은 버튼·휠 상태를 읽게 한다.
void Client::CUIInputRouter::Begin_Frame()
{
	m_bMouseClaimedThisFrame = false;
	// 이전 최상위 창 영역은 보존하고 이번 프레임의 등록·클릭 소비만 초기화한다.
	m_bHasTopWindowLastFrame = m_bHasTopWindow;
	m_pTopWindowOwnerLastFrame = m_pTopWindowOwner;
	m_fTopWindowXLastFrame = m_fTopWindowX;
	m_fTopWindowYLastFrame = m_fTopWindowY;
	m_fTopWindowWidthLastFrame = m_fTopWindowWidth;
	m_fTopWindowHeightLastFrame = m_fTopWindowHeight;
	m_bHasTopWindow = false;
	m_pTopWindowOwner = nullptr;
	m_bLeftClickConsumed = false;
	m_pPointerScope = nullptr;
	m_pCinematicPointerOwner = nullptr;
	m_bLeftDownThisFrame =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::LB) & 0x80);
	m_bRightDownThisFrame =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::RB) & 0x80);
	// 120단위의 완성된 눈금만 전달하고 고해상도 휠의 나머지는 다음 프레임에 보탠다.
	m_iWheelNotchesThisFrame = m_iWheelDeltaPending / WHEEL_DELTA;
	m_iWheelDeltaPending -= m_iWheelNotchesThisFrame * WHEEL_DELTA;
}

// 각 view의 사각형을 viewport로 확대해 Client 커서와 비교한다. 입력 차단·조회 실패면 false다.
bool_t Client::CUIInputRouter::Is_Hovered(
	f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight,
	f32_t fRefWidth, f32_t fRefHeight) const
{
	if (Is_PointerInputSuppressed()) return false;
	// Windows 화면 좌표를 주 창 Client 좌표로 바꿔 ImGui 없이 제품 UI를 검사한다.
	::POINT cursor{};
	if (!GetCursorPos(&cursor) || !ScreenToClient(g_hWnd, &cursor))
		return false;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	if (vViewportSize.x <= 0.f || vViewportSize.y <= 0.f ||
		fRefWidth <= 0.f || fRefHeight <= 0.f)
	{
		return false;
	}

	const f32_t fScaleX = vViewportSize.x / fRefWidth;
	const f32_t fScaleY = vViewportSize.y / fRefHeight;
	const f32_t fLeft = fX * fScaleX;
	const f32_t fTop = fY * fScaleY;
	const f32_t fRight = fLeft + fWidth * fScaleX;
	const f32_t fBottom = fTop + fHeight * fScaleY;

	const f32_t fCursorX = static_cast<f32_t>(cursor.x);
	const f32_t fCursorY = static_cast<f32_t>(cursor.y);

	return fCursorX >= fLeft && fCursorX < fRight &&
		fCursorY >= fTop && fCursorY < fBottom;
}

// 각 view가 호출한다. 영역·왼쪽 눌림·앞 창·중복 소비를 차례로 검사하고 승인한 클릭만 소비한다.
bool_t Client::CUIInputRouter::Is_Clicked(
	f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight,
	f32_t fRefWidth, f32_t fRefHeight)
{
	if (!Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		return false;

	// 앞 창이 클릭을 가져가더라도 UI 위 커서는 게임 마우스 입력을 막는다.
	Claim_Mouse_This_Frame();

	if (!Is_LeftClickEdge())
		return false;
	// 이전 프레임 최상위 창 안의 클릭은 그 창의 위젯만 받을 수 있다.
	// 컷신 전용 조작은 숨겨진 일반 창 영역에 막히지 않는다.
	if (!m_bCinematicSuppressed && m_bHasTopWindowLastFrame && m_pPointerScope != m_pTopWindowOwnerLastFrame)
	{
		f32_t fCursorX = 0.f, fCursorY = 0.f;
		if (Get_ClientCursorPosition(fCursorX, fCursorY) &&
			fCursorX >= m_fTopWindowXLastFrame &&
			fCursorX < m_fTopWindowXLastFrame + m_fTopWindowWidthLastFrame &&
			fCursorY >= m_fTopWindowYLastFrame &&
			fCursorY < m_fTopWindowYLastFrame + m_fTopWindowHeightLastFrame)
		{
			return false;
		}
	}
	// 같은 눌림을 이미 가져간 위젯이 있으면 두 번째 동작을 막는다.
	if (m_bLeftClickConsumed)
		return false;
	m_bLeftClickConsumed = true;
	return true;
}

// 이전 프레임에는 안 눌렸고 지금 눌린 상태만 반환한다. 영역·중복 소비 검사는 Is_Clicked가 맡는다.
bool_t Client::CUIInputRouter::Is_LeftClickEdge() const
{
	return !Is_PointerInputSuppressed() && !m_bLeftAwaitRelease && m_bLeftDownThisFrame && !m_bLeftDownLastFrame;
}

// Party 메뉴 등의 바깥 우클릭 검사에 쓰는 오른쪽 눌림이다.
bool_t Client::CUIInputRouter::Is_RightClickEdge() const
{
	return !Is_PointerInputSuppressed() && m_bRightDownThisFrame && !m_bRightDownLastFrame;
}

// 드래그 호출자가 쓸 문서 좌표로 환산한다. 모든 조회를 통과한 뒤 출력 참조를 바꾼다.
bool_t Client::CUIInputRouter::Get_MousePosition(
	f32_t fRefWidth, f32_t fRefHeight, f32_t& outX, f32_t& outY) const
{
	if (Is_PointerInputSuppressed()) return false;
	::POINT cursor{};
	if (!GetCursorPos(&cursor) || !ScreenToClient(g_hWnd, &cursor))
		return false;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	if (vViewportSize.x <= 0.f || vViewportSize.y <= 0.f ||
		fRefWidth <= 0.f || fRefHeight <= 0.f)
	{
		return false;
	}

	outX = static_cast<f32_t>(cursor.x) * fRefWidth / vViewportSize.x;
	outY = static_cast<f32_t>(cursor.y) * fRefHeight / vViewportSize.y;
	return true;
}

// 드롭 위치·앞 창 중재에 필요한 Client 픽셀을 반환한다. 실패는 출력값을 보존한다.
bool_t Client::CUIInputRouter::Get_ClientCursorPosition(f32_t& outX, f32_t& outY) const
{
	if (Is_PointerInputSuppressed()) return false;
	::POINT cursor{};
	if (!GetCursorPos(&cursor) || !ScreenToClient(g_hWnd, &cursor))
		return false;

	outX = static_cast<f32_t>(cursor.x);
	outY = static_cast<f32_t>(cursor.y);
	return true;
}

// UI가 마우스를 점유했다고 기록한다. MainApp과 End_Frame이 이 상태를 Engine 차단에 반영한다.
void Client::CUIInputRouter::Claim_Mouse_This_Frame()
{
	m_bMouseClaimedThisFrame = true;
}

// 열린 최상위 view가 자신의 주소 표식과 Client 픽셀 영역을 등록한다.
void Client::CUIInputRouter::Set_TopWindowRect(
	const void* pOwner, f32_t fScreenX, f32_t fScreenY,
	f32_t fScreenWidth, f32_t fScreenHeight)
{
	m_bHasTopWindow = true;
	m_pTopWindowOwner = pOwner;
	m_fTopWindowX = fScreenX;
	m_fTopWindowY = fScreenY;
	m_fTopWindowWidth = fScreenWidth;
	m_fTopWindowHeight = fScreenHeight;
	// 이 함수는 글자 clip을 바꾸지 않는다. 창별 글자 가림은 CUITextOcclusion과 호출자가 처리한다.
}

// 글자·입력 소비자가 현재 등록된 앞 창 영역 안인지 확인한다.
bool_t Client::CUIInputRouter::Is_UnderTopWindow(f32_t fScreenX, f32_t fScreenY) const
{
	return m_bHasTopWindow &&
		fScreenX >= m_fTopWindowX && fScreenX < m_fTopWindowX + m_fTopWindowWidth &&
		fScreenY >= m_fTopWindowY && fScreenY < m_fTopWindowY + m_fTopWindowHeight;
}

// 닉네임·채팅창이 입력을 시작할 때 이전 큐를 버리고 WM_CHAR 수신을 켠다.
void Client::CUIInputRouter::Start_TextInput()
{
	m_bTextInputActive = true;
	m_TypedChars.clear();
}

// 입력창 종료 시 수신을 끄고 남은 문자를 버린다.
void Client::CUIInputRouter::Stop_TextInput()
{
	m_bTextInputActive = false;
	m_TypedChars.clear();
}

// WndProc의 확정 문자를 보관하고 제어 문자 해석은 입력창에 맡긴다.
void Client::CUIInputRouter::On_Char(wchar_t ch)
{
	if (!m_bTextInputActive || m_bCinematicSuppressed)
		return;
	// 프레임이 지연돼도 문자 큐는 256개를 넘기지 않는다.
	if (m_TypedChars.size() < 256)
		m_TypedChars.push_back(ch);
}

// WndProc이 전달한 휠 증분을 누적하고 다음 Begin_Frame에서 눈금으로 나눈다.
void Client::CUIInputRouter::On_MouseWheel(const int32_t iWheelDelta)
{
	m_iWheelDeltaPending += iWheelDelta;
}

// 입력창이 큐의 문자열을 가져간다. move 뒤 큐를 비워 같은 문자를 다시 처리하지 않는다.
wstring_t Client::CUIInputRouter::Take_TypedChars()
{
	wstring_t typed = std::move(m_TypedChars);
	m_TypedChars.clear();
	return typed;
}

// 정상 Render 끝에서 UI 점유를 Engine에 반영하고 이번 버튼 상태를 다음 프레임 기준으로 보관한다.
void Client::CUIInputRouter::End_Frame()
{
	if (m_bCinematicSuppressed)
		CGameInstance::Get().SetInputBlocked(true, true);
	else if (m_bLeftAwaitRelease || m_bMouseClaimedThisFrame)
		CGameInstance::Get().SetInputBlocked(false, true);

	m_bMouseClaimedLastFrame = m_bMouseClaimedThisFrame;
	// 최상위 창 등록은 여기서 지우지 않는다. 다음 Begin_Frame이 클릭 중재용으로 먼저 복사한다.
	CGameInstance::Get().Clear_TextClipOutRect();
	if (!m_bLeftDownThisFrame) m_bLeftAwaitRelease = false;
	m_bLeftDownLastFrame = m_bLeftDownThisFrame;
	m_bRightDownLastFrame = m_bRightDownThisFrame;
}
