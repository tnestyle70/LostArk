// foreground 장치 읽기와 키 edge/focus 복구를 구현한다. 공개 조회와 같은 snapshot/release 상태를 갱신한다.
#include "Input_Device.h"

namespace
{
	// foreground Update가 빌린 장치에서 snapshot을 읽는다. 장치 소실/미획득이면 획득을 한 번만 재시도한다.
	// background 창에서 무한 Acquire 루프를 돌지 않으며 최종 실패는 호출자가 입력을 비우게 한다.
	bool Read_ForegroundDeviceState(IDirectInputDevice8* device, const DWORD size, void* state)
	{
		HRESULT result = device->GetDeviceState(size, state);
		if (DIERR_INPUTLOST == result || DIERR_NOTACQUIRED == result)
		{
			result = device->Acquire();
			if (SUCCEEDED(result))
			{
				result = device->GetDeviceState(size, state);
			}
		}
		return SUCCEEDED(result);
	}
}

NS_BEGIN(Engine)

CInput_Device::CInput_Device(void)
{
	ZeroMemory(m_byKeyState, sizeof(m_byKeyState));
}

CInput_Device::~CInput_Device(void)
{

}

// 키보드/마우스를 foreground, nonexclusive 방식으로 준비한다. HWND는 차용하며 창 수명은 Client가 관리한다.
// 초기 Acquire의 반환값은 검사하지 않고, 매 Update의 읽기/제한 재획득으로 준비 상태를 판단한다.
HRESULT CInput_Device::Initialize(HINSTANCE hInst, HWND hWnd)
{
	m_hInputWindow = hWnd;

	// DInput 컴객체를 생성하는 함수
	// 보충: 장치 생성에 사용할 DirectInput 인터페이스를 ComPtr로 소유한다. 실패는 초기화 실패로 전달한다.
	if (FAILED(DirectInput8Create(hInst,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_pInputSDK,
		NULL)))
	{
		return E_FAIL;
	}

	// 키보드 객체 생성
	// 보충: 이후 매 Update에서 상태를 읽을 키보드 장치를 준비한다. 값은 아래 SetDataFormat이 정한 byte 배열로 받는다.
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysKeyboard, &m_pKeyBoard, nullptr)))
	{
		return E_FAIL;
	}

	// 생성된 키보드 객체의 대한 정보를 컴 객체에게 전달하는 함수
	// 보충: c_dfDIKeyboard는 이후 GetDeviceState가 채울 데이터의 형식이다. 현재 키 입력 자체를 전달하는 호출은 아니다.
	if (FAILED(m_pKeyBoard->SetDataFormat(&c_dfDIKeyboard)))
	{
		return E_FAIL;
	}

	// 장치에 대한 독점권을 설정해주는 함수, (클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수)
	// 보충: 여기서는 foreground + nonexclusive를 선택한다. focus와 읽기 실패에 따른 입력 차단/재획득은 Update가 이어서 처리한다.
	if (FAILED(m_pKeyBoard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
	{
		return E_FAIL;
	}

	// 장치에 대한 access 버전을 받아오는 함수
	// 보충: Acquire는 버전을 조회하는 API가 아니라 입력 장치를 획득하는 요청이다. 여기서는 반환값을 검사하지 않고 Update에서 읽기 결과를 확인한다.
	m_pKeyBoard->Acquire();

	// 마우스 객체 생성
	// 보충: 버튼과 상대 이동/휠 상태를 읽을 마우스 장치를 준비한다.
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
	{
		return E_FAIL;
	}

	// 생성된 마우스 객체의 대한 정보를 컴 객체에게 전달하는 함수
	// 보충: c_dfDIMouse는 GetDeviceState 출력의 메모리 형식이며 위치나 버튼 값을 여기서 읽지는 않는다.
	if (FAILED(m_pMouse->SetDataFormat(&c_dfDIMouse)))
	{
		return E_FAIL;
	}

	// 장치에 대한 독점권을 설정해주는 함수, 클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수
	// 보충: 마우스에도 foreground + nonexclusive를 지정한다. 백그라운드 복귀 후 눌림과 이동의 처리는 Update의 복구 규칙을 따른다.
	if (FAILED(m_pMouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
	{
		return E_FAIL;
	}

	// 장치에 대한 access 버전을 받아오는 함수
	// 보충: Acquire는 버전을 조회하는 API가 아니라 입력 장치를 획득하는 요청이다. 여기서는 반환값을 검사하지 않고 Update에서 읽기 결과를 확인한다.
	m_pMouse->Acquire();

	return S_OK;
}

// GameInstance의 프레임 입력 갱신이다. 이전 키 snapshot과 현재 snapshot을 비교할 정보를 함께 유지한다.
void CInput_Device::Update(void)
{
	const bool_t focused = Has_InputFocus();
	// focus를 잃으면 한 번 Unacquire하고 snapshot을 비운다. 복귀 후 계속 눌린 입력은 아래 release 조건을 거친다.
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

	// keyboardState는 지역 후보라 읽기 실패가 이전 키 bytes를 남기지 않는다.
	CopyMemory(m_byPreviousKeyState, m_byKeyState, sizeof(m_byKeyState));
	int8_t keyboardState[256]{};
	const bool_t keyboardReady = Read_ForegroundDeviceState(
		m_pKeyBoard.Get(), sizeof(keyboardState), keyboardState);
	for (uint32_t key = 0u; key < 256u; ++key)
	{
		const bool_t down = keyboardReady && 0 != (keyboardState[key] & 0x80);
		// 이전 읽기가 준비되지 않았거나 기존 release 대기라면 계속 누르는 동안 숨긴다. 뗀 snapshot에서 대기를 끝낸다.
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

	// 마우스는 상대 이동을 DirectInput으로 읽고 버튼의 focus 복구 상태는 Win32 현재 눌림으로 판단한다.
	DIMOUSESTATE mouseState{};
	const bool_t mouseReady = Read_ForegroundDeviceState(
		m_pMouse.Get(), sizeof(mouseState), &mouseState);
	// DIM 순서와 Win32 버튼을 맞춘다. DirectInput 읽기 준비와 실제 버튼 눌림을 함께 확인한다.
	constexpr int mouseKeys[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON };
	static_assert(sizeof(mouseKeys) / sizeof(mouseKeys[0]) == ETOUI(DIM::END));
	for (uint32_t index = 0u; index < ETOUI(DIM::END); ++index)
	{
		const bool_t down = mouseReady && 0 != (GetAsyncKeyState(mouseKeys[index]) & 0x8000);
		m_MouseFocusReleaseRequired[index] = down &&
			(!m_bMouseStateReady || m_MouseFocusReleaseRequired[index]);
	}
	// focus/장치 복구 첫 읽기의 누적 이동은 버린다. 읽기 실패 시에는 버튼과 이동을 모두 비운다.
	if (mouseReady && !m_bMouseStateReady)
	{
		mouseState.lX = mouseState.lY = mouseState.lZ = 0;
	}
	m_tMouseState = mouseReady ? mouseState : DIMOUSESTATE{};
	m_bMouseStateReady = mouseReady;
}

// 준비 실패를 nullptr로 전달한다. 후보가 가진 부분 초기화 COM 장치도 지역 unique_ptr와 함께 해제된다.
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

NS_END
