#include "Texture.h"

#include "GameInstance.h"
#include "Profiler.h"

namespace
{
	uint64_t Estimate_TextureBytes(ID3D11ShaderResourceView* pView)
	{
		if (nullptr == pView)
			return 0u;
		ComPtr<ID3D11Resource> resource;
		pView->GetResource(&resource);
		ComPtr<ID3D11Texture2D> texture;
		if (nullptr == resource || FAILED(resource.As(&texture)))
			return 0u;

		D3D11_TEXTURE2D_DESC desc{};
		texture->GetDesc(&desc);
		uint32_t blockBytes = 0u;
		uint32_t bytesPerPixel = 0u;
		switch (desc.Format)
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			blockBytes = 8u;
			break;
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			blockBytes = 16u;
			break;
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			bytesPerPixel = 16u;
			break;
		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			bytesPerPixel = 12u;
			break;
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
			bytesPerPixel = 8u;
			break;
		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
			bytesPerPixel = 4u;
			break;
		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
			bytesPerPixel = 2u;
			break;
		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
			bytesPerPixel = 1u;
			break;
		default:
			return 0u;
		}

		uint64_t total = 0u;
		const uint32_t mipCount = (std::max)(1u, desc.MipLevels);
		for (uint32_t mip = 0u; mip < mipCount; ++mip)
		{
			const uint64_t width = (std::max)(1u, desc.Width >> mip);
			const uint64_t height = (std::max)(1u, desc.Height >> mip);
			if (0u != blockBytes)
				total += ((width + 3u) / 4u) * ((height + 3u) / 4u) * blockBytes;
			else
				total += width * height * bytesPerPixel;
		}
		return total * (std::max)(1u, desc.ArraySize) *
			(std::max)(1u, desc.SampleDesc.Count);
	}
}

CTexture::CTexture(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}


CTexture::~CTexture()
{
}

HRESULT CTexture::Initialize_Prototype(const tchar_t* pTextureFilePath, uint32_t iNumTextures)
{
	if (nullptr == pTextureFilePath || 0u == iNumTextures)
		return E_INVALIDARG;
	m_iNumTextures = iNumTextures;
	if (CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
		pProfiler->Add_Counter(EProfilerCounter::TextureRequests, iNumTextures);

	m_Textures.reserve(iNumTextures);

	for (size_t i = 0; i < m_iNumTextures; i++)
	{
	
		ComPtr<ID3D11ShaderResourceView>		pSRV = { nullptr };

		tchar_t			szTextureFilePath[MAX_PATH] = {};

		wsprintf(szTextureFilePath, pTextureFilePath, i);
		
		tchar_t			szEXT[MAX_PATH] = {};

		_wsplitpath_s(szTextureFilePath, nullptr, 0, nullptr, 0, nullptr, 0, szEXT, MAX_PATH);

		HRESULT		hr = {};

		/* CTexture currently backs display-space UI sprites.  Preserve the authored
		   bytes because these sprites render after scene gamma into an UNORM back buffer. */
		if (false == lstrcmp(szEXT, TEXT(".dds")))
		{
			hr = CreateDDSTextureFromFileEx(
				m_pDevice.Get(), szTextureFilePath, 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
				0, 0, DDS_LOADER_IGNORE_SRGB, nullptr, &pSRV);
		}
		else if (false == lstrcmp(szEXT, TEXT(".tga")))
		{
			hr = E_FAIL;
		}
		else
		{
			hr = CreateWICTextureFromFileEx(
				m_pDevice.Get(), szTextureFilePath, 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
				0, 0, WIC_LOADER_IGNORE_SRGB, nullptr, &pSRV);
		}

		if (FAILED(hr) || nullptr == pSRV)
			return FAILED(hr) ? hr : E_FAIL;
		if (CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
		{
			pProfiler->Add_Counter(EProfilerCounter::TextureUniqueSrvs);
			pProfiler->Add_Counter(
				EProfilerCounter::TextureEstimatedGpuBytes,
				Estimate_TextureBytes(pSRV.Get()));
		}
		m_Textures.push_back(move(pSRV));
	}

	m_SRVs.resize(m_iNumTextures);

	return S_OK;
}

HRESULT CTexture::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CTexture::Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iTextureIndex)
{
	if (iTextureIndex >= m_iNumTextures)
		return E_FAIL;

	return pShader->Bind_Texture(pConstantName, m_Textures[iTextureIndex]);	
}

HRESULT CTexture::Bind_ShaderResources(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	uint32_t		iNumTextures = {};

	for (auto& pTexture : m_Textures)
		m_SRVs[iNumTextures++] = pTexture.Get();

	return pShader->Bind_Textures(pConstantName, &m_SRVs.front(), m_iNumTextures);
}


unique_ptr<CTexture> CTexture::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pTextureFilePath, uint32_t iNumTextures)
{
	auto pInstance = unique_ptr<CTexture>(new CTexture(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pTextureFilePath, iNumTextures)))
	{
		MSG_BOX("Failed to Created : CTexture");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CPrototype> CTexture::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CTexture>(new CTexture(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTexture");
		return nullptr;
	}

	return pInstance;
}

