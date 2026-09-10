#include "Engine_Defines.h"

#ifdef _DEBUG

#include <crtdbg.h>

/* Reports the CRT's remaining allocations after this module's statics are gone.

The CRT's own _CRTDBG_LEAK_CHECK_DF reports at the end of the executable's exit, and that is
before this DLL is detached -- so everything an Engine static still owned at that moment was
listed as a leak although it is freed moments later. Each one showed up as a single 16-byte
block holding a pointer and eight zero bytes: a std::_Container_proxy, the per-container
bookkeeping the debug iterators allocate, one for every container those statics hold. The
count grew as more of the game was used, because these statics are created on first use.

`init_seg(compiler)` puts this object at the front of the module's construction order, so it is
destroyed after every other Engine static, and the executable has already finished its own exit
by then. What it reports is therefore what is genuinely still allocated. */
#pragma warning(push)
#pragma warning(disable : 4074)     /* initializers put in compiler reserved initialization area */
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
