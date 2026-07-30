#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"

NS_BEGIN(Engine)

class CModel;
class CShader;

NS_END

NS_BEGIN(Client)

class CMapAssetRenderUtils final
{
public:
	static uint32_t Select_Pass(
		const MAP_ASSET_RENDER_PROFILE& profile,
		bool_t mirrored);

	static HRESULT Bind_Material(
		const shared_ptr<Engine::CModel>& model,
		const shared_ptr<Engine::CShader>& shader,
		uint32_t meshIndex,
		const MAP_ASSET_RENDER_PROFILE& profile,
		f32_t elapsedTime);
};

NS_END