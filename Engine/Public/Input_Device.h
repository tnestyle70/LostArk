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
		return m_byKeyState[byKeyID];
	}

	int8_t	Get_DIMouseState(DIM eMouse)
	{
		return m_tMouseState.rgbButtons[ETOUI(eMouse)];
	}

	// 현재 마우스의 특정 축 좌표를 반환
	int32_t	Get_DIMouseMove(DIMM eMouseState)
	{
		return *((reinterpret_cast<int32_t*>(&m_tMouseState)) + ETOUI(eMouseState));
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
	DIMOUSESTATE			m_tMouseState = {};

public:
	static unique_ptr<CInput_Device> Create(HINSTANCE hInst, HWND hWnd);
};

NS_END


