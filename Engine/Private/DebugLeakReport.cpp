// 기존 엔진 Debug 메모리 추적 설정과 Windows/CRT 공통 선언을 연결한다.
#include "Engine_Defines.h"

#ifdef _DEBUG

// 모듈 종료 단계의 남은 Debug CRT 할당을 출력한다.
#include <crtdbg.h>

/* Engine의 일반 static 객체가 정리된 뒤 Debug CRT의 남은 할당을 확인하기 위한 장치다.

EXE 종료의 기본 누수 출력 시점에는 Engine DLL의 static 컨테이너가 아직 살아 있을 수 있다.
이때 Debug STL의 컨테이너 추적 객체도 남은 할당으로 보이므로, 모듈 종료 쪽에서 다시 보고한다.

init_seg(compiler)로 이 객체를 일반 초기화 객체보다 앞쪽에서 생성하여 뒤쪽에서 소멸하게 한다.
출력은 그 시점에 CRT가 추적 중인 할당이며, 다른 모듈까지 포함한 모든 객체의 수명이나
실제 누수 여부를 이 객체 하나가 판정하는 것은 아니다. */
#pragma warning(push)
#pragma warning(disable : 4074)     /* compiler 예약 초기화 영역 사용 경고를 이 선언에 한해 억제한다. */
#pragma init_seg(compiler)
#pragma warning(pop)

namespace
{
	struct LEAK_REPORT_AT_MODULE_UNLOAD final
	{
		~LEAK_REPORT_AT_MODULE_UNLOAD()
		{
			_CrtDumpMemoryLeaks();
		}
	};

	LEAK_REPORT_AT_MODULE_UNLOAD g_LeakReportAtModuleUnload;
}

#endif
