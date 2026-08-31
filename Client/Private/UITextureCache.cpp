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

	if (nullptr != pSRV)
	{
		/* Native pixel size for Get_Texture_Size -- queried once here off the SRV's own backing
		Texture2D instead of re-opening the file. */
		ComPtr<ID3D11Resource> pResource;
		pSRV->GetResource(&pResource);
		ComPtr<ID3D11Texture2D> pTexture2D;
		if (nullptr != pResource &&
			SUCCEEDED(pResource.As(&pTexture2D)) && nullptr != pTexture2D)
		{
			D3D11_TEXTURE2D_DESC Desc{};
			pTexture2D->GetDesc(&Desc);
			m_TextureSizeCache[strPath] = TEXTURE_SIZE{
				static_cast<f32_t>(Desc.Width), static_cast<f32_t>(Desc.Height) };
		}
	}

	m_TextureCache[strPath] = pSRV;
	return pSRV.Get();
}

bool_t Client::CUITextureCache::Get_Texture_Size(
	const string& strPath, f32_t& outWidth, f32_t& outHeight)
{
	Get_Or_Load(strPath);
	const auto Iter = m_TextureSizeCache.find(strPath);
	if (m_TextureSizeCache.end() == Iter ||
		Iter->second.fWidth <= 0.f || Iter->second.fHeight <= 0.f)
	{
		return false;
	}
	outWidth = Iter->second.fWidth;
	outHeight = Iter->second.fHeight;
	return true;
}
