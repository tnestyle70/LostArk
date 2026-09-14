#pragma once

#pragma push_macro("new")
#undef new
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#pragma pop_macro("new")
#include "Engine_Defines.h"
#include "MouseButtonReleaseGate.h"

NS_BEGIN(Engine)

class CInput_Device 
{
private:
	CInput_Device(void);
public:
	~CInput_Device(void);

public:
	int8_t	Get_DIKeyState(uint8_t byKeyID)
	{
		if (IsKeyboardInputBlocked())
			return 0;

		return m_byKeyState[byKeyID];
	}

	bool_t Get_DIKeyPressed(uint8_t byKeyID)
	{
		if (IsKeyboardInputBlocked())
			return false;

		return
			0 != (m_byKeyState[byKeyID] & 0x80) &&
			0 == (m_byPreviousKeyState[byKeyID] & 0x80);
	}

	int8_t Get_DIKeyStateRaw(uint8_t byKeyID) const
	{
		if (!Has_InputFocus() || !m_bKeyboardStateReady)
			return 0;
		return m_byKeyState[byKeyID];
	}

	bool_t Get_DIKeyPressedRaw(uint8_t byKeyID) const
	{
		if (!Has_InputFocus() || !m_bKeyboardStateReady)
			return false;
		return
			0 != (m_byKeyState[byKeyID] & 0x80) &&
			0 == (m_byPreviousKeyState[byKeyID] & 0x80);
	}

	int8_t	Get_DIMouseState(DIM eMouse)
	{
		const uint32_t index = ETOUI(eMouse);
		if (index >= ETOUI(DIM::END))
			return 0;
		if (!m_MouseReleaseGates[index].Observe(
			0 != (Get_DIMouseStateRaw(eMouse) & 0x80),
			m_bMouseBlocked || m_MouseButtonBlocked[index]))
			return 0;

		return m_tMouseState.rgbButtons[index];
	}

	int8_t Get_DIMouseStateRaw(DIM eMouse) const
	{
		const uint32_t index = ETOUI(eMouse);
		if (!Has_InputFocus() || !m_bMouseStateReady || index >= ETOUI(DIM::END) ||
			m_MouseFocusReleaseRequired[index])
			return 0;
		int virtualKey = 0;
		switch (eMouse)
		{
		case DIM::LB: virtualKey = VK_LBUTTON; break;
		case DIM::RB: virtualKey = VK_RBUTTON; break;
		case DIM::WHEEL: virtualKey = VK_MBUTTON; break;
		default: return 0;
		}
		return 0 != (GetAsyncKeyState(virtualKey) & 0x8000) ?
			static_cast<int8_t>(0x80) : 0;
	}

	// 현재 마우스의 특정 축 좌표를 반환
	int32_t	Get_DIMouseMove(DIMM eMouseState)
	{
		if (IsMouseInputBlocked())
			return 0;

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

	bool_t IsKeyboardInputBlocked() const
	{
		return m_bKeyboardBlocked || !Has_InputFocus() || !m_bKeyboardStateReady;
	}

	bool_t IsMouseInputBlocked() const
	{
		return m_bMouseBlocked || !Has_InputFocus() || !m_bMouseStateReady;
	}
public:
	HRESULT Initialize(HINSTANCE hInst, HWND hWnd);
	void	Update(void);

private:
	// Raw input bypasses UI capture only; it never bypasses window focus.
	bool_t Has_InputFocus() const
	{
		return nullptr != m_hInputWindow && GetForegroundWindow() == m_hInputWindow;
	}

private:
	HWND m_hInputWindow = nullptr;
	bool_t m_bWindowFocused = false;
	bool_t m_bKeyboardStateReady = false;
	bool_t m_bMouseStateReady = false;
	bool_t m_KeyFocusReleaseRequired[256] = {};
	bool_t m_MouseFocusReleaseRequired[ETOUI(DIM::END)] = {};
	ComPtr<IDirectInput8>			m_pInputSDK = {};

private:
	ComPtr<IDirectInputDevice8>		m_pKeyBoard = {};
	ComPtr<IDirectInputDevice8>		m_pMouse = {};

private:
	int8_t					m_byKeyState[256] = {};		// 키보드에 있는 모든 키값을 저장하기 위한 변수
	int8_t					m_byPreviousKeyState[256] = {};
	DIMOUSESTATE			m_tMouseState = {};
	bool_t m_bKeyboardBlocked = false;
	bool_t m_bMouseBlocked = false;
	bool_t m_MouseButtonBlocked[ETOUI(DIM::END)] = {};
	CMouseButtonReleaseGate m_MouseReleaseGates[ETOUI(DIM::END)] = {};

public:
	static unique_ptr<CInput_Device> Create(HINSTANCE hInst, HWND hWnd);
};

NS_END


