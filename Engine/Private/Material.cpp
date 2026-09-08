#include "Material.h"
#include "BinaryAsset/ModelAssetData.h"
#include "Shader.h"

#include <algorithm>
#include <array>
#include <cmath>
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
	struct RGBA_MIP_LEVEL
	{
		uint32_t width;
		uint32_t height;
		vector<uint8_t> pixels;
	};

	vector<RGBA_MIP_LEVEL> BuildRgbaMipChain(uint32_t width, uint32_t height,
		vector<uint8_t>&& rgba, bool_t isColorSlot)
	{
		static const array<double, 256> srgbToLinear = []
		{
			array<double, 256> values{};
			for (size_t index = 0; index < values.size(); ++index)
			{
				const double value = static_cast<double>(index) / 255.0;
				values[index] = value <= 0.04045 ? value / 12.92 :
					pow((value + 0.055) / 1.055, 2.4);
			}
			return values;
		}();

		vector<RGBA_MIP_LEVEL> levels;
		// Moving the decoded image preserves every mip-zero channel byte.
		levels.push_back({ width, height, move(rgba) });
		while (levels.back().width > 1 || levels.back().height > 1)
		{
			const RGBA_MIP_LEVEL& source = levels.back();
			RGBA_MIP_LEVEL target{ max(1u, source.width / 2),
				max(1u, source.height / 2), {} };
			target.pixels.resize(static_cast<size_t>(target.width) * target.height * 4);
			for (uint32_t y = 0; y < target.height; ++y)
			{
				const double top = static_cast<double>(y) * source.height / target.height;
				const double bottom = static_cast<double>(y + 1) * source.height / target.height;
				const uint32_t firstY = static_cast<uint32_t>(top);
				const uint32_t endY = min(source.height, static_cast<uint32_t>(ceil(bottom)));
				for (uint32_t x = 0; x < target.width; ++x)
				{
					const double left = static_cast<double>(x) * source.width / target.width;
					const double right = static_cast<double>(x + 1) * source.width / target.width;
					const uint32_t firstX = static_cast<uint32_t>(left);
					const uint32_t endX = min(source.width, static_cast<uint32_t>(ceil(right)));
					double sum[4]{};
					// Area weights include the final row/column of odd-sized images.
					for (uint32_t sourceY = firstY; sourceY < endY; ++sourceY)
					{
						const double weightY = min(bottom, static_cast<double>(sourceY + 1)) -
							max(top, static_cast<double>(sourceY));
						for (uint32_t sourceX = firstX; sourceX < endX; ++sourceX)
						{
							const double weight = weightY *
								(min(right, static_cast<double>(sourceX + 1)) -
									max(left, static_cast<double>(sourceX)));
							const size_t index =
								(static_cast<size_t>(sourceY) * source.width + sourceX) * 4;
							for (size_t channel = 0; channel < 4; ++channel)
							{
								const uint8_t value = source.pixels[index + channel];
								sum[channel] += weight * ((isColorSlot && channel < 3)
									? srgbToLinear[value] : static_cast<double>(value) / 255.0);
							}
						}
					}
					const double area = (right - left) * (bottom - top);
					const size_t index = (static_cast<size_t>(y) * target.width + x) * 4;
					for (size_t channel = 0; channel < 4; ++channel)
					{
						double value = clamp(sum[channel] / area, 0.0, 1.0);
						if (isColorSlot && channel < 3)
							value = value <= 0.0031308 ? value * 12.92 :
								1.055 * pow(value, 1.0 / 2.4) - 0.055;
						target.pixels[index + channel] = static_cast<uint8_t>(
							clamp(lround(value * 255.0), 0l, 255l));
					}
				}
			}
			levels.push_back(move(target));
		}
		return levels;
	}

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

		const vector<RGBA_MIP_LEVEL> mipLevels = BuildRgbaMipChain(
			header.width, header.height, move(rgba), isColorSlot);
		vector<D3D11_SUBRESOURCE_DATA> initialData(mipLevels.size());
		for (size_t index = 0; index < mipLevels.size(); ++index)
		{
			initialData[index].pSysMem = mipLevels[index].pixels.data();
			initialData[index].SysMemPitch = mipLevels[index].width * 4;
		}

		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = header.width;
		textureDesc.Height = header.height;
		textureDesc.MipLevels = static_cast<UINT>(mipLevels.size());
		textureDesc.ArraySize = 1;
		textureDesc.Format = isColorSlot ?
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		ComPtr<ID3D11Texture2D> texture;
		if (FAILED(pDevice->CreateTexture2D(&textureDesc, initialData.data(), &texture)))
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
		ComPtr<ID3D11ShaderResourceView>& pSRV,
		bool_t forceLinear = false)
	{
		if (path.empty())
			return E_FAIL;

		wstring extension = path.extension().wstring();
		transform(extension.begin(), extension.end(), extension.begin(), towlower);
		if (L".dds" == extension)
			return CreateDDSTextureFromFileEx(pDevice.Get(), path.c_str(), 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
				isColorSlot ? DDS_LOADER_FORCE_SRGB :
				(forceLinear ? DDS_LOADER_IGNORE_SRGB : DDS_LOADER_DEFAULT),
				nullptr, &pSRV);
		if (L".tga" == extension)
			return LoadTgaTexture(pDevice, path, isColorSlot, pSRV);
		return CreateWICTextureFromFileEx(pDevice.Get(), path.c_str(), 0,
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
			isColorSlot ? WIC_LOADER_FORCE_SRGB :
			(forceLinear ? WIC_LOADER_IGNORE_SRGB : WIC_LOADER_DEFAULT),
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
		const HRESULT result = LoadTexture(
			pDevice, path, IsColorTextureSlot(type), resource);
		if (FAILED(result))
		{
			wstring message = L"[CMaterial] Texture load failed: ";
			message += path.wstring();
			message += L"\n";
			OutputDebugStringW(message.c_str());
			return result;
		}
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
	aiString materialName;
	if (AI_SUCCESS == pAIMaterial->Get(AI_MATKEY_NAME, materialName))
		m_strName = materialName.C_Str();

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
	m_strName = material.name;
	m_iNameHash = material.nameHash;
	m_iDiffuseMirrorU = material.diffuseMirrorU ? 1u : 0u;

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

	/* The colour mask samples as data, not colour: aiTextureType_BASE_COLOR
	is outside IsColorTextureSlot, so it loads without the sRGB flag. */
	if (FAILED(AddTexture(m_pDevice, material.colorMaskPath,
		aiTextureType_BASE_COLOR, m_Textures)))
		return E_FAIL;
	m_ColorTint = material.colorTint;
	m_ColorTint.isEnabled = material.colorTint.isEnabled &&
		Has_Texture(aiTextureType_BASE_COLOR);
	m_Surface = material.surface;
    if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
    {
        const auto mask = m_Surface.sourceCharacter.baseTextureMask |
            m_Surface.sourceCharacter.lightTextureMask;
        for (uint32_t index = 0u; index < SOURCE_CHARACTER_TEXTURE_COUNT; ++index)
        {
            if ((mask & (1u << index)) == 0u) continue;
            const auto& input = material.sourceCharacterTextures[index];
            if (FAILED(LoadTexture(m_pDevice, input.path, input.srgb,
                m_SourceCharacterTextures[index], true))) return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
            m_SourceCharacterTextures[index]->GetDesc(&desc);
            if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
        }
        return S_OK;
    }
	if (m_Surface.hasEmissive)
	{
		if ((m_Surface.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE &&
			m_Surface.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE) ||
			FAILED(LoadTexture(m_pDevice, material.surfaceEmissivePath,
				m_Surface.emissiveSRGB, m_SurfaceEmissive, true)))
		{
			OutputDebugStringA(("[CMaterial] Source emissive input load failed: " +
				m_strName + "\n").c_str());
			return E_FAIL;
		}
		D3D11_SHADER_RESOURCE_VIEW_DESC emissive{};
		m_SurfaceEmissive->GetDesc(&emissive);
		if (emissive.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D)
			return E_INVALIDARG;
	}
	if (MODEL_SURFACE_FAMILY::LEGACY != m_Surface.family)
	{
		const auto& diffuse = material.surfaceDiffusePath.empty() ? material.diffusePath : material.surfaceDiffusePath;
		if (diffuse.empty() ||
            (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE && material.normalPath.empty()) ||
            (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE && material.reflectionPath.empty()) ||
			FAILED(LoadTexture(m_pDevice, diffuse,
				m_Surface.diffuseSRGB, m_SurfaceDiffuse, true)) ||
            (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
                FAILED(LoadTexture(m_pDevice, material.reflectionPath,
                    m_Surface.reflectionSRGB, m_SurfaceReflection, true))))
		{
			OutputDebugStringA(("[CMaterial] Source surface texture load failed: " +
				m_strName + "\n").c_str());
			return E_FAIL;
		}
		if (m_Surface.family == MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE ||
			m_Surface.family == MODEL_SURFACE_FAMILY::PBR_OPAQUE)
		{
			if (FAILED(LoadTexture(m_pDevice, material.surfaceNormalPath, false, m_SurfaceNormal, true)) ||
				FAILED(LoadTexture(m_pDevice, material.detailNormalPath, false, m_SurfaceDetailNormal, true)) ||
				FAILED(LoadTexture(m_pDevice, material.surfaceORMPath, m_Surface.ormSRGB, m_SurfaceORM, true)))
			{
				OutputDebugStringA(("[CMaterial] PBR input load failed: " + m_strName + "\n").c_str());
				return E_FAIL;
			}
		}
        if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE &&
            (FAILED(LoadTexture(m_pDevice, material.surfaceNormalPath, false, m_SurfaceNormal, true)) ||
             FAILED(LoadTexture(m_pDevice, material.surfaceSpecularPath, m_Surface.specularSRGB, m_SurfaceSpecular, true))))
        {
            OutputDebugStringA(("[CMaterial] Source opaque inputs failed: " + m_strName + "\n").c_str());
            return E_FAIL;
        }
        if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE)
        {
            if (FAILED(LoadTexture(m_pDevice, material.surfaceNormalPath, false, m_SurfaceNormal, true)) ||
                FAILED(LoadTexture(m_pDevice, material.overlayDiffusePath, m_Surface.overlaySRGB, m_SurfaceOverlayDiffuse, true)) ||
                FAILED(LoadTexture(m_pDevice, material.overlayNormalPath, false, m_SurfaceOverlayNormal, true))) return E_FAIL;
            for (const auto& input : { m_SurfaceDiffuse, m_SurfaceNormal, m_SurfaceOverlayDiffuse, m_SurfaceOverlayNormal })
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
                input->GetDesc(&desc);
                if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
            }
        }
        if (m_Surface.hasBakedLighting)
        {
            if (FAILED(LoadTexture(m_pDevice, material.bakedAveragePath, m_Surface.bakedLightingSRGB, m_BakedAverage, true)) ||
                FAILED(LoadTexture(m_pDevice, material.bakedDirectionalPath, m_Surface.bakedLightingSRGB, m_BakedDirectional, true)))
                return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC average{}, directional{};
            m_BakedAverage->GetDesc(&average); m_BakedDirectional->GetDesc(&directional);
            if (average.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
                directional.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
        }
        if (m_Surface.hasEnvironmentCube)
        {
            if (FAILED(LoadTexture(m_pDevice, material.environmentCubePath, false, m_EnvironmentCube, true)) ||
                FAILED(LoadTexture(m_pDevice, material.environmentBRDFPath, false, m_EnvironmentBRDF, true)))
                return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC cube{};
            m_EnvironmentCube->GetDesc(&cube);
            D3D11_SHADER_RESOURCE_VIEW_DESC brdf{};
            m_EnvironmentBRDF->GetDesc(&brdf);
            if (cube.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBE ||
                brdf.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
        }
		if (MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION == m_Surface.family &&
			FAILED(LoadTexture(m_pDevice, material.specularPath,
				m_Surface.specularSRGB, m_SurfaceSpecular, true)))
		{
			OutputDebugStringA(("[CMaterial] Source specular load failed: " +
				m_strName + "\n").c_str());
			return E_FAIL;
		}
	}
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

	if (aiTextureType_DIFFUSE == eType)
	{
		/* Shared shader instances must not inherit the previous surface program.
		   Shaders without this optional contract simply reject the variable. */
		const uint32_t disabled = 0u;
		pShader->Bind_RawValue("g_SurfaceProgram", &disabled, sizeof(disabled));
        pShader->Bind_RawValue("g_SourceCharacterProgram", &disabled, sizeof(disabled));
        pShader->Bind_RawValue("g_SourceCharacterRow", &disabled, sizeof(disabled));
		pShader->Bind_RawValue("g_HasSurfaceDefinition", &disabled, sizeof(disabled));
		pShader->Bind_RawValue("g_HasSurfaceEmissive", &disabled, sizeof(disabled));
		pShader->Bind_RawValue("g_DiffuseMirrorU", &m_iDiffuseMirrorU, sizeof(m_iDiffuseMirrorU));
	}
	return pShader->Bind_Texture(pConstantName, m_Textures[eType][iTextureIndex]);
}

namespace
{
    // Populated only by actual mesh draws on the rendering thread. Keeping a
    // shared reference until light accumulation finishes also closes teardown.
    thread_local std::vector<std::shared_ptr<CMaterial>> g_SourceCharacterFrame;
    thread_local float g_SourceCharacterTime = 0.f;
}

void CMaterial::Reset_SourceCharacterFrame(float presentationTime)
{
    g_SourceCharacterFrame.clear();
    g_SourceCharacterTime = presentationTime;
}

uint32_t CMaterial::Get_SourceCharacterFrameCount()
{
    return static_cast<uint32_t>(g_SourceCharacterFrame.size());
}

HRESULT CMaterial::Bind_SourceCharacterInputs(shared_ptr<CShader> shader,
    bool lightPass, uint32_t row)
{
    if (!shader) return E_INVALIDARG;
    const auto& source = m_Surface.sourceCharacter;
    const auto& constants = lightPass ? source.lightConstants : source.baseConstants;
    const uint32_t mask = lightPass ? source.lightTextureMask : source.baseTextureMask;
    if (FAILED(shader->Bind_RawValue("g_SourceCharacterTime", &g_SourceCharacterTime, sizeof(g_SourceCharacterTime))) ||
        FAILED(shader->Bind_RawValue("g_SourceCharacterProgram", &source.program, sizeof(source.program))) ||
        FAILED(shader->Bind_RawValue("g_SourceCharacterRow", &row, sizeof(row))) ||
        FAILED(shader->Bind_RawValue(lightPass ? "g_SourceCharacterLightConstants" :
            "g_SourceCharacterBaseConstants", constants.data(), sizeof(constants)))) return E_FAIL;
    for (uint32_t index = 0u; index < SOURCE_CHARACTER_TEXTURE_COUNT; ++index)
    {
        if ((mask & (1u << index)) == 0u) continue;
        const auto name = std::string("g_SourceCharacterTexture") + std::to_string(index);
        if (FAILED(shader->Bind_Texture(name.c_str(), m_SourceCharacterTextures[index]))) return E_FAIL;
    }
    return S_OK;
}

HRESULT CMaterial::Bind_SourceCharacter(shared_ptr<CShader> shader)
{
    if (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_CHARACTER) return S_FALSE;
    auto found = std::find_if(g_SourceCharacterFrame.begin(), g_SourceCharacterFrame.end(),
        [this](const auto& entry) { return entry.get() == this; });
    if (found == g_SourceCharacterFrame.end())
    {
        // Row IDs are ephemeral render indices, never serialized asset IDs.
        if (g_SourceCharacterFrame.size() >= 256u) return E_BOUNDS;
        g_SourceCharacterFrame.push_back(shared_from_this());
        found = std::prev(g_SourceCharacterFrame.end());
    }
    const uint32_t row = static_cast<uint32_t>(std::distance(g_SourceCharacterFrame.begin(), found)) + 1u;
    return Bind_SourceCharacterInputs(shader, false, row);
}

HRESULT CMaterial::Bind_SourceCharacterLight(shared_ptr<CShader> shader, uint32_t index)
{
    if (index >= g_SourceCharacterFrame.size()) return E_INVALIDARG;
    return g_SourceCharacterFrame[index]->Bind_SourceCharacterInputs(shader, true, index + 1u);
}

HRESULT CMaterial::Bind_SurfaceLighting(shared_ptr<CShader> shader)
{
    if (!shader) return E_INVALIDARG;
    if (m_Surface.hasBakedLighting &&
        (FAILED(shader->Bind_Texture("g_BakedAverageTexture", m_BakedAverage)) ||
         FAILED(shader->Bind_Texture("g_BakedDirectionalTexture", m_BakedDirectional)))) return E_FAIL;
    if (m_Surface.hasEnvironmentCube &&
        (FAILED(shader->Bind_Texture("g_EnvironmentCubeTexture", m_EnvironmentCube)) ||
         FAILED(shader->Bind_Texture("g_EnvironmentBRDFLookupTexture", m_EnvironmentBRDF)))) return E_FAIL;
    return S_OK;
}

HRESULT CMaterial::Bind_SurfaceTexture(shared_ptr<CShader> pShader,
	const char_t* pConstantName, aiTextureType eType)
{
	if (nullptr == pShader || nullptr == pConstantName)
		return E_INVALIDARG;
    if (eType == aiTextureType_DIFFUSE)
    {
        const uint32_t zero = 0u;
        for (const auto* name : { "g_SourceCharacterProgram", "g_SourceCharacterRow" })
            (void)pShader->Bind_RawValue(name, &zero, sizeof(zero));
    }
	ComPtr<ID3D11ShaderResourceView> texture;
	switch (eType)
	{
	case aiTextureType_DIFFUSE: texture = m_SurfaceDiffuse; break;
	case aiTextureType_SPECULAR: texture = m_SurfaceSpecular; break;
	case aiTextureType_REFLECTION: texture = m_SurfaceReflection; break;
	case aiTextureType_NORMALS: texture = m_SurfaceNormal; break;
	case aiTextureType_HEIGHT: texture = m_SurfaceDetailNormal; break;
	case aiTextureType_UNKNOWN: texture = m_SurfaceORM; break;
	case aiTextureType_EMISSIVE: texture = m_SurfaceEmissive; break;
    // Surface-only roles, separate from the legacy color-mask texture array.
    case aiTextureType_BASE_COLOR: texture = m_SurfaceOverlayDiffuse; break;
    case aiTextureType_NORMAL_CAMERA: texture = m_SurfaceOverlayNormal; break;
	default: return E_INVALIDARG;
	}
	return texture ? pShader->Bind_Texture(pConstantName, texture) : E_FAIL;
}

shared_ptr<CMaterial> CMaterial::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const aiMaterial* pAIMaterial, const char_t* pModelFilePath)
{
	auto pInstance = shared_ptr<CMaterial>(new CMaterial(pDevice, pContext));

	if (FAILED(pInstance->Initialize(pAIMaterial, pModelFilePath)))
	{
		OutputDebugStringA("[CMaterial] Assimp material initialization failed.\n");
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
		OutputDebugStringA("[CMaterial] Binary material initialization failed.\n");
		return nullptr;
	}
	return pInstance;
}
