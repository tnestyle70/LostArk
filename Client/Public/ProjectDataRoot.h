#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>

NS_BEGIN(Client)

class CProjectDataRoot final
{
public:
	static filesystem::path Get();
	static filesystem::path Resolve(const filesystem::path& relativePath);
};

NS_END
