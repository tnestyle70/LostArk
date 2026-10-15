#pragma once

// Client의 창 handle과 Engine의 bool_t·좌표·문자열 타입을 사용한다.
#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

// UI 클릭을 창 하나에 전달하고 게임 입력을 막는다. UI 객체의 생성·삭제는 담당하지 않는다.
class CUIInputRouter final
{
public:
	// 객체 없이 호출하는 static 함수다. 내부 static 객체 하나를 참조로 돌려준다.
	static CUIInputRouter& Get();

public:

	// MainApp::Update가 각 창의 입력 검사 전에 버튼·휠 상태를 준비한다.
	void Begin_Frame();

	// 컷신 입력 차단을 전환한다. 해제 순간 눌려 있던 왼쪽 버튼은 놓을 때까지 새 클릭으로 받지 않는다.
	void Set_CinematicSuppressed(bool_t suppressed)
	{
		if (!suppressed && m_bCinematicSuppressed) m_bLeftAwaitRelease = m_bLeftDownThisFrame;
		m_bCinematicSuppressed = suppressed;
	}
	bool_t Is_CinematicSuppressed() const { return m_bCinematicSuppressed; }

	// 문서 기준 사각형에 현재 커서가 들어왔는지 검사한다.
	bool_t Is_Hovered(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight,
		f32_t fRefWidth, f32_t fRefHeight) const;

	// 앞 창과 중복 소비를 검사해 왼쪽 눌림 한 번을 위젯 하나에 전달한다.
	bool_t Is_Clicked(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight,
		f32_t fRefWidth, f32_t fRefHeight);

	// 현재·이전 프레임 버튼 상태로 새 눌림을 읽는다. 영역이나 중복 소비는 검사하지 않는다.
	bool_t Is_LeftClickEdge() const;

	// 메뉴 바깥 우클릭 검사 등에 쓰는 오른쪽 버튼의 새 눌림이다.
	bool_t Is_RightClickEdge() const;

	// 드래그가 유지·종료를 판별할 때 쓰는 버튼 상태다. 컷신·해제 대기 중에는 false다.
	bool_t Is_LeftDown() const { return !Is_PointerInputSuppressed() && !m_bLeftAwaitRelease && m_bLeftDownThisFrame; }

	bool_t Is_LeftReleaseEdge() const { return !Is_PointerInputSuppressed() && !m_bLeftAwaitRelease && !m_bLeftDownThisFrame && m_bLeftDownLastFrame; }

	// 문서 기준 좌표와 Client 픽셀 좌표를 각각 제공한다. 조회 실패는 출력값을 보존한다.
	bool_t Get_MousePosition(f32_t fRefWidth, f32_t fRefHeight, f32_t& outX, f32_t& outY) const;

	bool_t Get_ClientCursorPosition(f32_t& outX, f32_t& outY) const;

	// 이번 프레임의 완성된 120단위 휠 눈금이다. 나머지 증분은 다음 프레임에 남긴다.
	int32_t Get_MouseWheelNotches() const { return Is_PointerInputSuppressed() ? 0 : m_iWheelNotchesThisFrame; }
	// Client.cpp의 WndProc이 WM_MOUSEWHEEL 값을 전달한다.
	void On_MouseWheel(int32_t iWheelDelta);

	// UI의 마우스 점유를 기록한다. 실제 Engine 차단은 MainApp과 End_Frame에서 반영한다.
	void Claim_Mouse_This_Frame();
	bool_t Is_MouseClaimedThisFrame() const { return m_bCinematicSuppressed || m_bLeftAwaitRelease || m_bMouseClaimedThisFrame; }

	// Render 중 뒤늦게 생긴 UI 점유도 다음 MainApp::Update가 사용할 수 있게 보관한다.
	bool_t Was_MouseClaimedLastFrame() const { return m_bMouseClaimedLastFrame; }

	// 앞 창의 Client 픽셀 영역을 기록한다. 다음 Begin_Frame이 중재용으로 복사한 뒤 비운다.
	void Set_TopWindowRect(const void* pOwner, f32_t fScreenX, f32_t fScreenY,
		f32_t fScreenWidth, f32_t fScreenHeight);
	bool_t Is_UnderTopWindow(f32_t fScreenX, f32_t fScreenY) const;

	// 입력을 검사하는 창의 주소 표식이다. 가리키는 객체를 보유하거나 역참조하지 않는다.
	void Set_PointerScope(const void* pScope) { m_pPointerScope = pScope; }
	const void* Get_PointerScope() const { return m_pPointerScope; }

	// 닉네임·채팅창이 문자 수신을 켜고 끈다. WndProc에서 받은 문자는 Take_TypedChars로 가져간다.
	void Start_TextInput();
	void Stop_TextInput();
	bool_t Is_TextInputActive() const { return m_bTextInputActive; }

	// 활성 입력창의 확정 UTF-16 코드 단위를 최대 256개 보관한다. 제어 문자는 소비자가 해석한다.
	void On_Char(wchar_t ch);

	// 입력창에 문자열을 이동하고 큐를 비워 같은 입력을 다시 전달하지 않는다.
	wstring_t Take_TypedChars();

	// MainApp의 정상 Render 끝에서 입력 차단과 다음 프레임 버튼 상태를 확정한다.
	void End_Frame();

private:
	// scope만 컷신 입력 소유자를 임시로 바꾸고 복원할 수 있다.
	friend class CUIPointerScope;
	bool_t Is_PointerInputSuppressed() const
	{
		return m_bCinematicSuppressed &&
			(!m_pCinematicPointerOwner || m_pPointerScope != m_pCinematicPointerOwner);
	}
	// 외부의 기본 생성을 제한한다. Get의 지역 static은 정상 종료 때 자동 소멸하며 CBase 참조 계수를 쓰지 않는다.
	CUIInputRouter() = default;

private:
	// 컷신·왼쪽 버튼 해제 대기와 이번/이전 프레임 UI 점유를 별도로 기억한다.
	bool_t	m_bCinematicSuppressed = false;
	bool_t	m_bLeftAwaitRelease = false;
	bool_t	m_bMouseClaimedThisFrame = false;
	bool_t	m_bMouseClaimedLastFrame = false;
	bool_t	m_bHasTopWindow = false;
	// 창의 Update·Render에서 등록한 영역을 다음 Begin_Frame에 복사해 클릭 중재에 사용한다.
	// owner 주소는 역참조하지 않는 비교용 표식이다.
	const void*	m_pTopWindowOwner = nullptr;
	const void*	m_pTopWindowOwnerLastFrame = nullptr;
	bool_t	m_bHasTopWindowLastFrame = false;
	f32_t	m_fTopWindowXLastFrame = 0.f;
	f32_t	m_fTopWindowYLastFrame = 0.f;
	f32_t	m_fTopWindowWidthLastFrame = 0.f;
	f32_t	m_fTopWindowHeightLastFrame = 0.f;
	// 입력 검사 중인 창과 컷신 허용 창의 주소 표식이다. 소유·역참조하지 않는다.
	const void*	m_pPointerScope = nullptr;
	const void*	m_pCinematicPointerOwner = nullptr;
	// Is_Clicked가 한 번 승인한 왼쪽 눌림을 같은 프레임의 다른 위젯이 재사용하지 못하게 한다.
	bool_t	m_bLeftClickConsumed = false;
	f32_t	m_fTopWindowX = 0.f;
	f32_t	m_fTopWindowY = 0.f;
	f32_t	m_fTopWindowWidth = 0.f;
	f32_t	m_fTopWindowHeight = 0.f;
	// Begin_Frame이 버튼을 한 번 읽고 End_Frame이 이전 상태로 넘겨 눌림·놓임을 판별한다.
	bool_t	m_bLeftDownThisFrame = false;
	bool_t	m_bLeftDownLastFrame = false;
	bool_t	m_bRightDownThisFrame = false;
	bool_t	m_bRightDownLastFrame = false;
	bool_t	m_bTextInputActive = false;
	int32_t	m_iWheelDeltaPending = 0;
	int32_t	m_iWheelNotchesThisFrame = 0;
	// WndProc과 입력창이 같은 메인 스레드 흐름에서 전달하는 확정 문자 큐다.
	wstring_t	m_TypedChars;
};

// 입력 검사 블록 동안 창 표식을 바꾸고 소멸할 때 이전 표식으로 복원한다.
class CUIPointerScope final
{
public:
	explicit CUIPointerScope(const void* pOwner, bool_t allowCinematicInput = false)
		: m_pPrevious(CUIInputRouter::Get().Get_PointerScope()),
		  m_pPreviousCinematicOwner(CUIInputRouter::Get().m_pCinematicPointerOwner)
	{
		// 컷신 전용 조작만 입력을 허용하며 일반 창과 게임 입력 차단은 유지한다.
		CUIInputRouter::Get().m_pCinematicPointerOwner = allowCinematicInput ? pOwner : nullptr;
		CUIInputRouter::Get().Set_PointerScope(pOwner);
	}
	// 블록을 나가면 이전 창을 복원한다. 저장한 주소의 객체를 삭제하지 않는다.
	~CUIPointerScope()
	{
		CUIInputRouter::Get().Set_PointerScope(m_pPrevious);
		CUIInputRouter::Get().m_pCinematicPointerOwner = m_pPreviousCinematicOwner;
	}
	CUIPointerScope(const CUIPointerScope&) = delete;
	CUIPointerScope& operator=(const CUIPointerScope&) = delete;

private:
	const void* m_pPrevious = nullptr;
	const void* m_pPreviousCinematicOwner = nullptr;
};

NS_END
