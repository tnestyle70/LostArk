#pragma once

#include "Engine_Typedef.h"
#include <d3d11.h>
#include "Engine_Enum.h"

namespace Engine
{
	typedef struct tagEngineDesc
	{
		HINSTANCE	hInstance = {};
		HWND		hWnd = {};
		WINMODE		eWinMode = { WINMODE::END };
		D3D_DRIVER_TYPE eDriverType = D3D_DRIVER_TYPE_HARDWARE;
		bool_t		bNonInteractiveErrors = false;
		uint32_t	iNumLevels = {};
		uint32_t	iWinSizeX{}, iWinSizeY{};
	}ENGINE_DESC;
}
