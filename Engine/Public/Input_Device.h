#pragma once

#include "Engine_Defines.h"

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
		if (m_bKeyboardBlocked)
			return 0;

		return m_byKeyState[byKeyID];
	}

	bool_t Get_DIKeyPressed(uint8_t byKeyID)
	{
		if (m_bKeyboardBlocked)
			return false;

		return
			0 != (m_byKeyState[byKeyID] & 0x80) &&
			0 == (m_byPreviousKeyState[byKeyID] & 0x80);
	}

	int8_t Get_DIKeyStateRaw(uint8_t byKeyID) const
	{
		return m_byKeyState[byKeyID];
	}

	bool_t Get_DIKeyPressedRaw(uint8_t byKeyID) const
	{
		return
			0 != (m_byKeyState[byKeyID] & 0x80) &&
			0 == (m_byPreviousKeyState[byKeyID] & 0x80);
	}

	int8_t	Get_DIMouseState(DIM eMouse)
	{
		const uint32_t index = ETOUI(eMouse);
		if (m_bMouseBlocked ||
			index >= ETOUI(DIM::END) ||
			m_MouseButtonBlocked[index])
			return 0;

		return m_tMouseState.rgbButtons[index];
	}

	// 현재 마우스의 특정 축 좌표를 반환
	int32_t	Get_DIMouseMove(DIMM eMouseState)
	{
		if (m_bMouseBlocked)
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
	}

	void SetMouseButtonBlocked(DIM eMouse, bool_t blocked)
	{
		const uint32_t index = ETOUI(eMouse);
		if (index < ETOUI(DIM::END))
			m_MouseButtonBlocked[index] = blocked;
	}

	bool_t IsKeyboardInputBlocked() const
	{
		return m_bKeyboardBlocked;
	}

	bool_t IsMouseInputBlocked() const
	{
		return m_bMouseBlocked;
	}
public:
	HRESULT Initialize(HINSTANCE hInst, HWND hWnd);
	void	Update(void);

private:
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

public:
	static unique_ptr<CInput_Device> Create(HINSTANCE hInst, HWND hWnd);
};

NS_END


