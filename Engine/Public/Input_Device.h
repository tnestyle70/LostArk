#pragma once

//Engine의 POINT 이름이 전역 조회에 들어오기 전에 dinput.h를 읽는다.
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
// 키보드/마우스 COM 장치와 snapshot 형식, scan code 및 foreground 획득 계약을 사용한다.
#include <dinput.h>
// 엔진 기본 타입·DIM/DIMM과 ComPtr 소유 도구를 사용한다.
#include "Engine_Defines.h"
// UI가 소비한 누름이 버튼 해제 전 gameplay 입력으로 재사용되지 않게 버튼별 상태를 보관한다.
#include "MouseButtonReleaseGate.h"

NS_BEGIN(Engine)

// GameInstance가 소유하는 키보드/마우스 입력 snapshot과 차단 상태다.
// 메인 프레임의 Update와 조회를 전제로 하며 내부 잠금은 없다. COM 장치는 ComPtr로 소유한다.
class CInput_Device
{
private:
	CInput_Device(void);
public:
	~CInput_Device(void);

public:
	// UI 차단과 창 focus/장치 준비를 적용한 키 상태다. DirectInput의 상위 bit(0x80)가 눌림을 뜻한다.
	int8_t	Get_DIKeyState(uint8_t byKeyID)
	{
		if (IsKeyboardInputBlocked())
		{
			return 0;
		}

		return m_byKeyState[byKeyID];
	}

	// 직전 Update에는 안 눌리고 이번에는 눌린 경우다. 메시지 큐가 아니므로 두 갱신 사이의 모든 edge를 기록하지 않는다.
	bool_t Get_DIKeyPressed(uint8_t byKeyID)
	{
		if (IsKeyboardInputBlocked())
		{
			return false;
		}

		return
			0 != (m_byKeyState[byKeyID] & 0x80) &&
			0 == (m_byPreviousKeyState[byKeyID] & 0x80);
	}

	// Raw는 UI의 keyboard 차단만 건너뛴다. 창 focus와 정상 장치 읽기 조건은 계속 적용한다.
	int8_t Get_DIKeyStateRaw(uint8_t byKeyID) const
	{
		if (!Has_InputFocus() || !m_bKeyboardStateReady)
		{
			return 0;
		}
		return m_byKeyState[byKeyID];
	}

	// UI capture만 우회하는 edge 조회다. focus 복귀 때 계속 누른 키는 Update가 해제 전까지 숨긴다.
	bool_t Get_DIKeyPressedRaw(uint8_t byKeyID) const
	{
		if (!Has_InputFocus() || !m_bKeyboardStateReady)
		{
			return false;
		}
		return
			0 != (m_byKeyState[byKeyID] & 0x80) &&
			0 == (m_byPreviousKeyState[byKeyID] & 0x80);
	}

	// 차단 중 시작한 물리 누름은 UI가 닫혀도 release gate에 남는다. 뗀 뒤 새로 눌러야 gameplay가 소비한다.
	// 조회 중 gate 상태도 갱신되므로 단순한 const snapshot 조회와 다르다.
	int8_t	Get_DIMouseState(DIM eMouse)
	{
		const uint32_t index = ETOUI(eMouse);
		if (index >= ETOUI(DIM::END))
		{
			return 0;
		}
		if (!m_MouseReleaseGates[index].Observe(
			0 != (Get_DIMouseStateRaw(eMouse) & 0x80),
			m_bMouseBlocked || m_MouseButtonBlocked[index]))
		{
			return 0;
		}

		return m_tMouseState.rgbButtons[index];
	}

	// 준비된 foreground 창에서 Win32의 현재 버튼 상태를 읽는다. focus 복구 중 계속 누른 버튼은 여기서도 숨긴다.
	int8_t Get_DIMouseStateRaw(DIM eMouse) const
	{
		const uint32_t index = ETOUI(eMouse);
		if (!Has_InputFocus() || !m_bMouseStateReady || index >= ETOUI(DIM::END) ||
			m_MouseFocusReleaseRequired[index])
		{
			return 0;
		}
		int virtualKey = 0;
		switch (eMouse)
		{
		case DIM::LB:
			virtualKey = VK_LBUTTON;
			break;
		case DIM::RB:
			virtualKey = VK_RBUTTON;
			break;
		case DIM::WHEEL:
			virtualKey = VK_MBUTTON;
			break;
		default:
			return 0;
		}
		return 0 != (GetAsyncKeyState(virtualKey) & 0x8000) ?
			static_cast<int8_t>(0x80) : 0;
	}

	// 현재 마우스의 특정 축 좌표를 반환
	// 마지막 장치 읽기의 상대 이동량/휠 변화량이다. 화면의 절대 픽셀 좌표를 반환하지 않는다.
	int32_t	Get_DIMouseMove(DIMM eMouseState)
	{
		if (IsMouseInputBlocked())
		{
			return 0;
		}

		switch (eMouseState)
		{
		case DIMM::X:
			return m_tMouseState.lX;
		case DIMM::Y:
			return m_tMouseState.lY;
		case DIMM::WHEEL:
			return m_tMouseState.lZ;
		default:
			return 0;
		}
	}

	// MainApp이 UI capture를 전달한다. mouse 차단 시 현재 누름을 gate에도 기록하여 UI 닫힘 뒤 누름이 새 명령이 되지 않게 한다.
	void SetInputBlocked(bool_t bKeyboardBlocked, bool_t bMouseBlocked)
	{
		m_bKeyboardBlocked = bKeyboardBlocked;
		m_bMouseBlocked = bMouseBlocked;
		for (uint32_t index = 0u; index < ETOUI(DIM::END); ++index)
		{
			(void)m_MouseReleaseGates[index].Observe(
				0 != (Get_DIMouseStateRaw(static_cast<DIM>(index)) & 0x80),
				bMouseBlocked);
		}
	}

	// 제품 UI/저작 도구가 소비한 특정 버튼만 차단한다. 전체 mouse 차단과 함께 최종 조회에 적용한다.
	void SetMouseButtonBlocked(DIM eMouse, bool_t blocked)
	{
		const uint32_t index = ETOUI(eMouse);
		if (index < ETOUI(DIM::END))
		{
			m_MouseButtonBlocked[index] = blocked;
			(void)m_MouseReleaseGates[index].Observe(
				0 != (Get_DIMouseStateRaw(eMouse) & 0x80),
				blocked);
		}
	}

	// 일반 키 조회의 공통 허용 조건이다. 호출할 때마다 현재 foreground 창도 다시 확인한다.
	bool_t IsKeyboardInputBlocked() const
	{
		return m_bKeyboardBlocked || !Has_InputFocus() || !m_bKeyboardStateReady;
	}

	// 상대 이동/휠에 적용하는 전체 차단 조건이다. 특정 버튼의 차단과 release gate는 Get_DIMouseState에서 따로 처리한다.
	bool_t IsMouseInputBlocked() const
	{
		return m_bMouseBlocked || !Has_InputFocus() || !m_bMouseStateReady;
	}
public:
	// Create가 두 장치를 준비한다. 실패 후보는 Create의 unique_ptr가 COM 참조와 함께 정리한다.
	HRESULT Initialize(HINSTANCE hInst, HWND hWnd);
	// GameInstance의 프레임 시작에서 읽는다. focus 상실/읽기 실패 시 이전 입력이 재사용되지 않도록 비운다.
	void	Update(void);

private:
	// UI capture와 별개의 창 소유 조건이다. Raw 입력도 이 조건을 우회하지 않는다.
	bool_t Has_InputFocus() const
	{
		return nullptr != m_hInputWindow && GetForegroundWindow() == m_hInputWindow;
	}

private:
	// Client가 유지하는 입력 창을 차용한다. 이 객체는 창을 닫지 않으며 focus와 장치 읽기 성공 여부를 별도로 보관한다.
	HWND m_hInputWindow = nullptr;
	bool_t m_bWindowFocused = false;
	bool_t m_bKeyboardStateReady = false;
	bool_t m_bMouseStateReady = false;
	// focus/장치 복구 때 이미 눌린 키는 해제까지 숨긴다. alt-tab에 사용한 누름이 gameplay edge가 되는 것을 막는다.
	bool_t m_KeyFocusReleaseRequired[256] = {};
	bool_t m_MouseFocusReleaseRequired[ETOUI(DIM::END)] = {};
	ComPtr<IDirectInput8>			m_pInputSDK = {};

private:
	ComPtr<IDirectInputDevice8>		m_pKeyBoard = {};
	ComPtr<IDirectInputDevice8>		m_pMouse = {};

private:
	int8_t					m_byKeyState[256] = {}; // 키보드에 있는 모든 키값을 저장하기 위한 변수
	// 보충: 256개 scan code의 마지막 유효 입력 snapshot이다. focus/읽기 실패 시 비우며 key release 대기 상태는 별도로 보관한다.
	int8_t					m_byPreviousKeyState[256] = {};
	DIMOUSESTATE			m_tMouseState = {};
	bool_t m_bKeyboardBlocked = false;
	bool_t m_bMouseBlocked = false;
	bool_t m_MouseButtonBlocked[ETOUI(DIM::END)] = {};
	// UI capture로 소비한 누름을 버튼별로 기억한다. 위 focus 복구 gate와는 발생 원인이 다르다.
	CMouseButtonReleaseGate m_MouseReleaseGates[ETOUI(DIM::END)] = {};

public:
	// GameInstance가 초기화에 성공한 입력 객체의 단독 소유권을 받는다. 초기화 실패 후보의 COM 참조도 함께 해제된다.
	static unique_ptr<CInput_Device> Create(HINSTANCE hInst, HWND hWnd);
};

NS_END

