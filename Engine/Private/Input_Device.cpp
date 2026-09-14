#include "Input_Device.h"

namespace
{
	bool Read_ForegroundDeviceState(IDirectInputDevice8* device, const DWORD size, void* state)
	{
		HRESULT result = device->GetDeviceState(size, state);
		if (DIERR_INPUTLOST == result || DIERR_NOTACQUIRED == result)
		{
			// One bounded reacquire attempt; a background window must never spin here.
			result = device->Acquire();
			if (SUCCEEDED(result))
				result = device->GetDeviceState(size, state);
		}
		return SUCCEEDED(result);
	}
}

Engine::CInput_Device::CInput_Device(void)
{
	ZeroMemory(m_byKeyState, sizeof(m_byKeyState));
}

Engine::CInput_Device::~CInput_Device(void)
{

}

HRESULT Engine::CInput_Device::Initialize(HINSTANCE hInst, HWND hWnd)
{
	m_hInputWindow = hWnd;

	// DInput 컴객체를 생성하는 함수
	if (FAILED(DirectInput8Create(hInst,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_pInputSDK,
		NULL)))
		return E_FAIL;

	// 키보드 객체 생성
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysKeyboard, &m_pKeyBoard, nullptr)))
		return E_FAIL;

	// 생성된 키보드 객체의 대한 정보를 컴 객체에게 전달하는 함수
	if (FAILED(m_pKeyBoard->SetDataFormat(&c_dfDIKeyboard)))
		return E_FAIL;

	// 장치에 대한 독점권을 설정해주는 함수, (클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수)
	if (FAILED(m_pKeyBoard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		return E_FAIL;

	// 장치에 대한 access 버전을 받아오는 함수
	m_pKeyBoard->Acquire();


	// 마우스 객체 생성
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
		return E_FAIL;


	// 생성된 마우스 객체의 대한 정보를 컴 객체에게 전달하는 함수
	if (FAILED(m_pMouse->SetDataFormat(&c_dfDIMouse)))
		return E_FAIL;

	// 장치에 대한 독점권을 설정해주는 함수, 클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수
	if (FAILED(m_pMouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		return E_FAIL;

	// 장치에 대한 access 버전을 받아오는 함수
	m_pMouse->Acquire();


	return S_OK;
}

void Engine::CInput_Device::Update(void)
{
	const bool_t focused = Has_InputFocus();
	if (!focused)
	{
		if (m_bWindowFocused)
		{
			m_pKeyBoard->Unacquire();
			m_pMouse->Unacquire();
		}
		m_bWindowFocused = false;
		m_bKeyboardStateReady = false;
		m_bMouseStateReady = false;
		ZeroMemory(m_byKeyState, sizeof(m_byKeyState));
		ZeroMemory(m_byPreviousKeyState, sizeof(m_byPreviousKeyState));
		m_tMouseState = {};
		return;
	}
	m_bWindowFocused = true;

	CopyMemory(m_byPreviousKeyState, m_byKeyState, sizeof(m_byKeyState));
	int8_t keyboardState[256]{};
	const bool_t keyboardReady = Read_ForegroundDeviceState(
		m_pKeyBoard.Get(), sizeof(keyboardState), keyboardState);
	for (uint32_t key = 0u; key < 256u; ++key)
	{
		const bool_t down = keyboardReady && 0 != (keyboardState[key] & 0x80);
		m_KeyFocusReleaseRequired[key] = down &&
			(!m_bKeyboardStateReady || m_KeyFocusReleaseRequired[key]);
		m_byKeyState[key] = m_KeyFocusReleaseRequired[key] ? 0 : keyboardState[key];
	}
	if (!keyboardReady)
	{
		ZeroMemory(m_byKeyState, sizeof(m_byKeyState));
		ZeroMemory(m_byPreviousKeyState, sizeof(m_byPreviousKeyState));
	}
	m_bKeyboardStateReady = keyboardReady;

	DIMOUSESTATE mouseState{};
	const bool_t mouseReady = Read_ForegroundDeviceState(
		m_pMouse.Get(), sizeof(mouseState), &mouseState);
	constexpr int mouseKeys[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON };
	static_assert(sizeof(mouseKeys) / sizeof(mouseKeys[0]) == ETOUI(DIM::END));
	for (uint32_t index = 0u; index < ETOUI(DIM::END); ++index)
	{
		const bool_t down = mouseReady && 0 != (GetAsyncKeyState(mouseKeys[index]) & 0x8000);
		m_MouseFocusReleaseRequired[index] = down &&
			(!m_bMouseStateReady || m_MouseFocusReleaseRequired[index]);
	}
	// Discard accumulated motion on restoration, and all data from a failed read.
	if (mouseReady && !m_bMouseStateReady)
		mouseState.lX = mouseState.lY = mouseState.lZ = 0;
	m_tMouseState = mouseReady ? mouseState : DIMOUSESTATE{};
	m_bMouseStateReady = mouseReady;
}

unique_ptr<CInput_Device> CInput_Device::Create(HINSTANCE hInst, HWND hWnd)
{
	auto pInstance = unique_ptr<CInput_Device>(new CInput_Device());

	if (FAILED(pInstance->Initialize(hInst, hWnd)))
	{
		MSG_BOX("Failed to Created : CInput_Device");
		return nullptr;
	}

	return pInstance;
}
