#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <map>
#include <string>

NS_BEGIN(Client)

/* Loads a Resources-relative UI image into an SRV and keeps it cached by path. Both
CHUDRuntimeView and CHUDLayoutTool already carry their own copy of this exact DDS/WIC-load-
with-sRGB-ignored logic; new runtime views use this one instead of adding a third copy. */
class CUITextureCache final
{
public:
	CUITextureCache(ComPtr<ID3D11Device> pDevice);
	~CUITextureCache();

public:
	/* strPath: "UI/..." Resources-relative id, resolved through CRuntimeAssetRoot. Returns
	nullptr on an empty path or a load failure; the failure is cached too, so a missing file
	is not retried every frame. */
	ID3D11ShaderResourceView* Get_Or_Load(const string& strPath);
	/* Native pixel size of the image at strPath (loading it into the cache first if needed) --
	for a keyframe-animation layer whose on-screen size is its art's own size times the key's
	scale, not an authored rect. False (outputs untouched) if the image can't be loaded. */
	bool_t Get_Texture_Size(const string& strPath, f32_t& outWidth, f32_t& outHeight);

private:
	struct TEXTURE_SIZE { f32_t fWidth = 0.f, fHeight = 0.f; };

	ComPtr<ID3D11Device> m_pDevice;
	map<string, ComPtr<ID3D11ShaderResourceView>> m_TextureCache;
	map<string, TEXTURE_SIZE> m_TextureSizeCache;
};

NS_END
