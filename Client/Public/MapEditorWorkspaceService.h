#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CMapEditorWorkspaceService final
{
public:
	static void Request();
	static void Cancel();
	static bool_t Is_Requested();
	static void Set_Active(bool_t isActive);
	static bool_t Is_Active();
};

NS_END
