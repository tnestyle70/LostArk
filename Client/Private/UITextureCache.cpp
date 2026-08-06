#include "UITextureCache.h"

#include "RuntimeAssetRoot.h"

Client::CUITextureCache::CUITextureCache(ComPtr<ID3D11Device> pDevice)
	: m_pDevice{ pDevice }
{
}

Client::CUITextureCache::~CUITextureCache()
{
}

ID3D11ShaderResourceView* Client::CUITextureCache::Get_Or_Load(const string& strPath)
{
	if (strPath.empty())
		return nullptr;

	const auto Iter = m_TextureCache.find(strPath);
	if (m_TextureCache.end() != Iter)
		return Iter->second.Get();

	const u8string Utf8Path(strPath.begin(), strPath.end());
	const filesystem::path ResolvedPath = CRuntimeAssetRoot::Resolve(filesystem::path(Utf8Path));

	ComPtr<ID3D11ShaderResourceView> pSRV = { nullptr };

	if (!ResolvedPath.empty())
	{
		HRESULT hr = {};
		/* Same reasoning as CHUDRuntimeView::Get_Or_Load_Texture: this draws into an already
		gamma-correct UNORM back buffer, so the loader must not decode embedded sRGB again. */
		if (0 == _wcsicmp(ResolvedPath.extension().c_str(), L".dds"))
		{
			hr = CreateDDSTextureFromFileEx(
				m_pDevice.Get(), ResolvedPath.c_str(), 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
				0, 0, DDS_LOADER_IGNORE_SRGB, nullptr, &pSRV);
		}
		else
		{
			hr = CreateWICTextureFromFileEx(
				m_pDevice.Get(), ResolvedPath.c_str(), 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
				0, 0, WIC_LOADER_IGNORE_SRGB, nullptr, &pSRV);
		}
		if (FAILED(hr))
			pSRV = nullptr;
	}

	m_TextureCache[strPath] = pSRV;
	return pSRV.Get();
}
