#pragma once

// Windows 헤더보다 먼저 Winsock2 선언을 준비해 Client의 socket API를 사용한다.
#include <WinSock2.h>
// HINSTANCE/HWND와 Client에서 사용하는 Win32 창·메시지 타입을 제공한다.
#include <Windows.h>
// 기준 해상도의 uint32_t 정수 타입을 사용한다.
#include <stdint.h>
// 이 공통 헤더를 읽는 Loader의 _beginthreadex 선언을 제공한다.
#include <process.h>

// Client.cpp가 정의하는 프로그램 instance와 창 handle을 참조한다. extern 선언은 새 창을 만들지 않는다.
extern HINSTANCE g_hInst;
extern HWND		g_hWnd;

namespace Client
{
	// Client UI의 초기 기준 해상도다. 현재 창/viewport의 물리 크기는 별도로 조회한다.
	static const uint32_t g_iWinSizeX = { 1280 };
	static const uint32_t g_iWinSizeY = { 720 };

	// Prototype/Layer와 전환 요청이 공유하는 Client Level 구분이다. END는 범위 끝 표식이다.
	enum class LEVEL
	{
		STATIC,
		LOADING,
		LOBBY,
		CHARACTER_SELECT,
		BERN,
		VALTAN_ARENA,
		KOUKUSAYDON_ARENA,
		DEVELOPMENT,
		MAHARAKA,
		COLOSSEUM,
		END
	};
}

using namespace std;
using namespace Client;
