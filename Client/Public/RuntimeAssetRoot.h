#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>

NS_BEGIN(Client)

class CRuntimeAssetRoot final
{
public:
	static filesystem::path Get_ResourceRoot();
	static filesystem::path Get();
	static filesystem::path Get_FontRoot();
	static filesystem::path Resolve(const filesystem::path& relativePath);
	static filesystem::path Resolve_Font(
		const filesystem::path& relativePath);
};

NS_END
