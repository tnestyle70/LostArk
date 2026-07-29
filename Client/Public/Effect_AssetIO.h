#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_Types.h"

#include <filesystem>

NS_BEGIN(Client)

class CEffect_AssetIO final
{
public:
	static bool_t Save_Json(const filesystem::path& Path,
		const EFFECT_ASSET_DESC& Asset, string* pError = nullptr);
	static bool_t Load_Json(const filesystem::path& Path,
		EFFECT_ASSET_DESC& OutAsset, string* pError = nullptr);
	static bool_t Save_Binary(const filesystem::path& Path,
		const EFFECT_ASSET_DESC& Asset, string* pError = nullptr);
	static bool_t Load_Binary(const filesystem::path& Path,
		EFFECT_ASSET_DESC& OutAsset, string* pError = nullptr);
	static bool_t Validate_RoundTrip(const EFFECT_ASSET_DESC& Asset,
		string* pError = nullptr);
};

NS_END
