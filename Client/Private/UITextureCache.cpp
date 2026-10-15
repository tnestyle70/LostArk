// UI asset 경로별 로드 결과와 원본 픽셀 크기를 보관한다.
#include "UITextureCache.h"
// DDS는 DDS 로더, 나머지 이미지 확장자는 WIC 로더로 SRV를 만든다.
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"

// Resources 상대 asset ID를 허용된 실제 파일 경로로 해석한다.
#include "RuntimeAssetRoot.h"

// 호출자가 전달한 장치의 COM 참조를 캐시 수명 동안 유지한다.
Client::CUITextureCache::CUITextureCache(ComPtr<ID3D11Device> pDevice)
	: m_pDevice{ pDevice }
{
}

// 자동 소멸하는 멤버가 장치·이미지 참조와 크기 목록을 정리한다.
Client::CUITextureCache::~CUITextureCache()
{
}

// 캐시의 성공·실패 결과를 먼저 찾는다. 새 로드 결과는 Adopt가 보유하므로 raw 포인터를 차용해 반환한다.
ID3D11ShaderResourceView* Client::CUITextureCache::Get_Or_Load(const string& strPath)
{
	if (strPath.empty())
		return nullptr;

	const auto Iter = m_TextureCache.find(strPath);
	if (m_TextureCache.end() != Iter)
		return Iter->second.Get();

	ComPtr<ID3D11ShaderResourceView> pSRV = Load_Texture(m_pDevice.Get(), strPath);
	Adopt(strPath, pSRV);
	return pSRV.Get();
}

// UTF-8 상대 경로를 해석하고 이미지 파일에서 SRV를 만든다. 실패는 빈 COM 참조다.
ComPtr<ID3D11ShaderResourceView> Client::CUITextureCache::Load_Texture(
	ID3D11Device* pDevice, const string& strPath)
{
	if (nullptr == pDevice || strPath.empty())
		return nullptr;

	const u8string Utf8Path(strPath.begin(), strPath.end());
	const filesystem::path ResolvedPath = CRuntimeAssetRoot::Resolve(filesystem::path(Utf8Path));

	ComPtr<ID3D11ShaderResourceView> pSRV = { nullptr };

	if (!ResolvedPath.empty())
	{
		HRESULT hr = {};
		// UI 이미지 로더가 내장 sRGB 정보를 자동 적용하지 않도록 지정한다.
		if (0 == _wcsicmp(ResolvedPath.extension().c_str(), L".dds"))
		{
			hr = CreateDDSTextureFromFileEx(
				pDevice, ResolvedPath.c_str(), 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
				0, 0, DDS_LOADER_IGNORE_SRGB, nullptr, &pSRV);
		}
		else
		{
			hr = CreateWICTextureFromFileEx(
				pDevice, ResolvedPath.c_str(), 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
				0, 0, WIC_LOADER_IGNORE_SRGB, nullptr, &pSRV);
		}
		if (FAILED(hr))
			pSRV = nullptr;
	}
	return pSRV;
}

// 빈 경로와 기존 정상 이미지는 변경하지 않는다. 새 결과의 참조·크기를 캐시에 등록한다.
void Client::CUITextureCache::Adopt(const string& strPath, ComPtr<ID3D11ShaderResourceView> pSRV)
{
	const auto Existing = m_TextureCache.find(strPath);
	if (strPath.empty() || (m_TextureCache.end() != Existing && nullptr != Existing->second))
		return;
	if (nullptr != pSRV)
	{
		// 파일을 다시 열지 않고 SRV가 참조하는 Texture2D에서 픽셀 크기를 얻는다.
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

	// nullptr도 저장해 Get_Or_Load가 같은 실패를 매 프레임 재시도하지 않게 한다.
	m_TextureCache[strPath] = pSRV;
}

// 레이아웃 애니메이션이 사용할 원본 크기를 조회하고 성공한 뒤에만 출력 참조를 바꾼다.
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
