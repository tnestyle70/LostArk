#pragma once

// 초기화 플래그/크기의 공통 타입과 Windows 기반 선언을 가져온다.
#include "Engine_Typedef.h"
// 장치 생성에 전달할 D3D_DRIVER_TYPE을 제공한다.
#include <d3d11.h>
// 창/전체 화면 모드의 기존 enum 계약을 사용한다.
#include "Engine_Enum.h"

namespace Engine
{
	// Client 초기화가 채워 CGameInstance::Initialize_Engine에 전달하는 값이다. HWND/HINSTANCE의 소유권을 이전하지 않는다.
	typedef struct tagEngineDesc
	{
		// 호출자가 생성한 프로세스 인스턴스와 창 핸들을 차용한다. 엔진 사용 중 유효한 창을 유지해야 한다.
		HINSTANCE	hInstance = {};
		HWND		hWnd = {};
		WINMODE		eWinMode = { WINMODE::END };
		D3D_DRIVER_TYPE eDriverType = D3D_DRIVER_TYPE_HARDWARE;
		// 자동 검사 등에서 대화상자를 억제할지 정한다. 오류 자체를 성공으로 바꾸는 옵션은 아니다.
		bool_t		bNonInteractiveErrors = false;
		// Prototype/Object/Physics 관리자가 확보할 Level 슬롯 수다. 현재 시작 Level 번호가 아니다.
		uint32_t	iNumLevels = {};
		// 장치·viewport 초기 크기의 픽셀 수다. 실행 중 창 크기 변경은 별도 resize 경로를 따른다.
		uint32_t	iWinSizeX{}, iWinSizeY{};
	}ENGINE_DESC;
}
