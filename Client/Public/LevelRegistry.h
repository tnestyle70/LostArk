#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "MapLoadScope.h"

#include <memory>

NS_BEGIN(Engine)
class CLevel;
NS_END

NS_BEGIN(Client)

class CLoader;

enum class CLIENT_LEVEL_KIND
{
	PRODUCT,
	DEVELOPMENT
};

struct CLIENT_LEVEL_DESCRIPTOR final
{
	using CREATE_FUNCTION = unique_ptr<Engine::CLevel>(*)(
		ComPtr<ID3D11Device>,
		ComPtr<ID3D11DeviceContext>);
	using LOAD_FUNCTION = HRESULT (CLoader::*)();

	LEVEL eLevel = LEVEL::END;
	CLIENT_LEVEL_KIND eKind = CLIENT_LEVEL_KIND::PRODUCT;
	const char_t* pStableId = nullptr;
	const char_t* pMapAreaId = nullptr;
	MAP_LOAD_SCOPE MapLoadScope{};
	CREATE_FUNCTION pCreate = nullptr;
	LOAD_FUNCTION pLoad = nullptr;
};

class CLevelRegistry final
{
public:
	static const CLIENT_LEVEL_DESCRIPTOR* Find(LEVEL eLevel);
	static unique_ptr<Engine::CLevel> Create_Level(
		LEVEL eLevel,
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	static HRESULT Execute_Load(
		LEVEL eLevel,
		CLoader& loader);
};

NS_END
