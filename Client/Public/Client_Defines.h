#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <stdint.h>
#include <process.h>

extern HINSTANCE g_hInst;
extern HWND		g_hWnd;

namespace Client
{
	static const uint32_t g_iWinSizeX = { 1280 };
	static const uint32_t g_iWinSizeY = { 720 };

	enum class LEVEL
	{
		STATIC,
		LOADING,
		LOBBY,
		CHARACTER_SELECT,
		BERN,
		VALTAN_ARENA,
		DEVELOPMENT,
		END
	};
}

using namespace std;
using namespace Client;
