#include "Material.h"
#include "BinaryAsset/ModelAssetData.h"
#include "Shader.h"

#include <algorithm>
#include <cwctype>
#include <cstring>
#include <fstream>
#include <iterator>

#pragma pack(push, 1)
namespace
{
	struct TGA_HEADER
	{
		uint8_t idLength;
		uint8_t colorMapType;
		uint8_t imageType;
		uint16_t colorMapFirst;
		uint16_t colorMapLength;
		uint8_t colorMapDepth;
		uint16_t xOrigin;
		uint16_t yOrigin;
		uint16_t width;
		uint16_t height;
		uint8_t bitsPerPixel;
		uint8_t descriptor;
	};
}
#pragma pack(pop)

namespace
{
	HRESULT LoadTgaTexture(ComPtr<ID3D11Device> pDevice,
		const filesystem::path& path,
		bool_t isColorSlot,
		ComPtr<ID3D11ShaderResourceView>& pSRV)
	{
		ifstream input(path, ios::binary);
		if (!input)
			return E_FAIL;
		const vector<uint8_t> bytes(
			(istreambuf_iterator<char>(input)), istreambuf_iterator<char>());
		if (bytes.size() < sizeof(TGA_HEADER))
			return E_FAIL;

		TGA_HEADER header{};
		memcpy(&header, bytes.data(), sizeof(header));
		if (0 != header.colorMapType ||
			(2 != header.imageType && 10 != header.imageType) ||
			(24 != header.bitsPerPixel && 32 != header.bitsPerPixel) ||
			0 == header.width || 0 == header.height)
			return E_FAIL;

		const size_t sourceStride = header.bitsPerPixel / 8;
		const size_t pixelCount =
			static_cast<size_t>(header.width) * header.height;
		vector<uint8_t> rgba(pixelCount * 4);
		size_t cursor = sizeof(TGA_HEADER) + header.idLength;
		size_t sourcePixel = 0;

		auto writePixel = [&](const uint8_t* source) -> bool_t
		{
			if (sourcePixel >= pixelCount)
				return false;
			const size_t sourceX = sourcePixel % header.width;
			const size_t sourceY = sourcePixel / header.width;
			const size_t targetX = (header.descriptor & 0x10)
				? header.width - 1 - sourceX : sourceX;
			const size_t targetY = (header.descriptor & 0x20)
				? sourceY : header.height - 1 - sourceY;
			const size_t target = (targetY * header.width + targetX) * 4;
			rgba[target + 0] = source[2];
			rgba[target + 1] = source[1];
			rgba[target + 2] = source[0];
			rgba[target + 3] = 4 == sourceStride ? source[3] : 255;
			++sourcePixel;
			return true;
		};

		if (2 == header.imageType)
		{
			while (sourcePixel < pixelCount)
			{
				if (cursor + sourceStride > bytes.size() ||
					!writePixel(bytes.data() + cursor))
					return E_FAIL;
				cursor += sourceStride;
			}
		}
		else
		{
			while (sourcePixel < pixelCount)
			{
				if (cursor >= bytes.size())
					return E_FAIL;
				const uint8_t packet = bytes[cursor++];
				const size_t count = (packet & 0x7f) + 1;
				if (packet & 0x80)
				{
					if (cursor + sourceStride > bytes.size())
						return E_FAIL;
					const uint8_t* source = bytes.data() + cursor;
					cursor += sourceStride;
					for (size_t index = 0; index < count; ++index)
					{
						if (!writePixel(source))
							return E_FAIL;
					}
				}
				else
				{
					for (size_t index = 0; index < count; ++index)
					{
						if (cursor + sourceStride > bytes.size() ||
							!writePixel(bytes.data() + cursor))
							return E_FAIL;
						cursor += sourceStride;
					}
				}
			}
		}

		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = header.width;
		textureDesc.Height = header.height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = isColorSlot ?
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initialData{};
		initialData.pSysMem = rgba.data();
		initialData.SysMemPitch = header.width * 4;

		ComPtr<ID3D11Texture2D> texture;
		if (FAILED(pDevice->CreateTexture2D(&textureDesc, &initialData, &texture)))
			return E_FAIL;
		return pDevice->CreateShaderResourceView(texture.Get(), nullptr, &pSRV);
	}

	/* Base colour and emissive textures are authored in sRGB, while lighting runs
	   in linear space and the post pass owns the only gamma encode. Decoding them
	   on load keeps that contract. Normal, specular, ORM and mask textures carry
	   data instead of colour, so they stay linear. */
	bool_t IsColorTextureSlot(aiTextureType type)
	{
		return aiTextureType_DIFFUSE == type ||
			aiTextureType_EMISSIVE == type;
	}

	HRESULT LoadTexture(ComPtr<ID3D11Device> pDevice,
		const filesystem::path& path,
		bool_t isColorSlot,
		ComPtr<ID3D11ShaderResourceView>& pSRV)
	{
		if (path.empty())
			return E_FAIL;

		wstring extension = path.extension().wstring();
		transform(extension.begin(), extension.end(), extension.begin(), towlower);
		if (L".dds" == extension)
			return CreateDDSTextureFromFileEx(pDevice.Get(), path.c_str(), 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
				isColorSlot ? DDS_LOADER_FORCE_SRGB : DDS_LOADER_DEFAULT,
				nullptr, &pSRV);
		if (L".tga" == extension)
			return LoadTgaTexture(pDevice, path, isColorSlot, pSRV);
		return CreateWICTextureFromFileEx(pDevice.Get(), path.c_str(), 0,
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
			isColorSlot ? WIC_LOADER_FORCE_SRGB : WIC_LOADER_DEFAULT,
			nullptr, &pSRV);
	}

	HRESULT AddTexture(ComPtr<ID3D11Device> pDevice,
		const filesystem::path& path,
		aiTextureType type,
		vector<ComPtr<ID3D11ShaderResourceView>> (&textures)[AI_TEXTURE_TYPE_MAX])
	{
		if (path.empty())
			return S_OK;
		ComPtr<ID3D11ShaderResourceView> resource;
		if (FAILED(LoadTexture(pDevice, path, IsColorTextureSlot(type), resource)))
			return E_FAIL;
		textures[type].push_back(resource);
		return S_OK;
	}

	HRESULT AddSolidTexture(ComPtr<ID3D11Device> pDevice,
		uint32_t rgba,
		aiTextureType type,
		vector<ComPtr<ID3D11ShaderResourceView>> (&textures)[AI_TEXTURE_TYPE_MAX])
	{
		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = 1;
		textureDesc.Height = 1;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initialData{};
		initialData.pSysMem = &rgba;
		initialData.SysMemPitch = sizeof(rgba);

		ComPtr<ID3D11Texture2D> texture;
		if (FAILED(pDevice->CreateTexture2D(&textureDesc, &initialData, &texture)))
			return E_FAIL;

		ComPtr<ID3D11ShaderResourceView> resource;
		if (FAILED(pDevice->CreateShaderResourceView(texture.Get(), nullptr, &resource)))
			return E_FAIL;
		textures[type].push_back(resource);
		return S_OK;
	}
}

CMaterial::CMaterial(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{	
}

CMaterial::~CMaterial()
{
}

HRESULT CMaterial::Initialize(const aiMaterial* pAIMaterial, const char_t* pModelFilePath)
{
	char_t			szDrive[MAX_PATH] = {};
	char_t			szDir[MAX_PATH] = {};

	_splitpath_s(pModelFilePath, szDrive, MAX_PATH, szDir, MAX_PATH, nullptr, 0, nullptr, 0);

	for (uint32_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
	{
		uint32_t		iNumTextures = pAIMaterial->GetTextureCount(static_cast<aiTextureType>(i));

		m_Textures[i].reserve(iNumTextures);

		for (uint32_t j = 0; j < iNumTextures; j++)
		{
			ComPtr<ID3D11ShaderResourceView>		pSRV = { nullptr };

			aiString			strTextureFilePath = {};

			if (FAILED(pAIMaterial->GetTexture(static_cast<aiTextureType>(i), j, &strTextureFilePath)))
				return E_FAIL;

			
			char_t			szFileName[MAX_PATH] = {};
			char_t			szEXT[MAX_PATH] = {};

			_splitpath_s(strTextureFilePath.C_Str(), nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szEXT, MAX_PATH);

			char_t			szFullPath[MAX_PATH] = {};
			strcpy_s(szFullPath, szDrive);
			strcat_s(szFullPath, szDir);
			strcat_s(szFullPath, szFileName);
			strcat_s(szFullPath, szEXT);

			tchar_t			szTextureFilePath[MAX_PATH] = {};

			MultiByteToWideChar(CP_ACP, 0, szFullPath, strlen(szFullPath), szTextureFilePath, MAX_PATH);

			HRESULT		hr = {};

			if (false == strcmp(szEXT, ".dds"))
				hr = CreateDDSTextureFromFile(m_pDevice.Get(), szTextureFilePath, nullptr, &pSRV);
			else if (false == strcmp(szEXT, ".tga"))
				hr = E_FAIL;
			else
				hr = CreateWICTextureFromFile(m_pDevice.Get(), szTextureFilePath, nullptr, &pSRV);
			
			m_Textures[i].push_back(pSRV);
		}
	}

	return S_OK;
}

HRESULT CMaterial::Initialize(const MODEL_MATERIAL_DATA& material)
{
	const filesystem::path& compatibleDiffusePath = material.diffusePath.empty()
		? material.emissivePath
		: material.diffusePath;
	if (compatibleDiffusePath.empty())
	{
		if (FAILED(AddSolidTexture(m_pDevice, 0xff4d4d4d,
			aiTextureType_DIFFUSE, m_Textures)))
			return E_FAIL;
	}
	else if (FAILED(AddTexture(m_pDevice, compatibleDiffusePath,
		aiTextureType_DIFFUSE, m_Textures)))
		return E_FAIL;

	if (FAILED(AddTexture(m_pDevice, material.normalPath,
			aiTextureType_NORMALS, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, material.specularPath,
			aiTextureType_SPECULAR, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, material.emissivePath,
			aiTextureType_EMISSIVE, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, material.opacityPath,
			aiTextureType_OPACITY, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, material.ormPath,
			aiTextureType_UNKNOWN, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, material.metallicPath,
			aiTextureType_METALNESS, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, material.roughnessPath,
			aiTextureType_DIFFUSE_ROUGHNESS, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, material.ambientOcclusionPath,
			aiTextureType_AMBIENT_OCCLUSION, m_Textures)))
		return E_FAIL;
	return S_OK;
}

bool_t CMaterial::Has_Texture(aiTextureType eType, uint32_t iTextureIndex) const
{
	return eType < AI_TEXTURE_TYPE_MAX &&
		iTextureIndex < m_Textures[eType].size();
}

HRESULT CMaterial::Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, aiTextureType eType, uint32_t iTextureIndex)
{
	if (eType >= AI_TEXTURE_TYPE_MAX ||
		iTextureIndex >= m_Textures[eType].size())
		return E_FAIL;

	return pShader->Bind_Texture(pConstantName, m_Textures[eType][iTextureIndex]);	
}

shared_ptr<CMaterial> CMaterial::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const aiMaterial* pAIMaterial, const char_t* pModelFilePath)
{
	auto pInstance = shared_ptr<CMaterial>(new CMaterial(pDevice, pContext));

	if (FAILED(pInstance->Initialize(pAIMaterial, pModelFilePath)))
	{
		MSG_BOX("Failed to Created : CMaterial");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CMaterial> CMaterial::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext, const MODEL_MATERIAL_DATA& material)
{
	auto pInstance = shared_ptr<CMaterial>(new CMaterial(pDevice, pContext));
	if (FAILED(pInstance->Initialize(material)))
	{
		MSG_BOX("Failed to Created : CMaterial");
		return nullptr;
	}
	return pInstance;
}
