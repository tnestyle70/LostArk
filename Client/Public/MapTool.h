#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Client)

class CMapTool final
{
public:
	void Toggle();
	void Render();

	bool IsOpen() const;

private:
	bool m_bOpen = false;
};

NS_END
