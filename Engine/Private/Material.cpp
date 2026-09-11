#include "Material.h"

#include <mutex>
#include <unordered_map>
#include "BinaryAsset/ModelAssetData.h"
#include "Shader.h"
#include "GameInstance.h"
#include "Profiler.h"

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

    HRESULT LoadSharedTexture(ComPtr<ID3D11Device> device,
        vector<shared_ptr<ComPtr<ID3D11ShaderResourceView>>>& owners,
        const filesystem::path& path, bool_t srgb,
        ComPtr<ID3D11ShaderResourceView>& view, bool_t forceLinear = false)
    {
        if (path.empty()) return E_INVALIDARG;
        std::error_code error;
        const auto size = filesystem::file_size(path, error);
        if (error) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        const auto modified = filesystem::last_write_time(path, error);
        if (error) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        std::wstring normalized = path.lexically_normal().wstring();
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), towlower);
        const std::wstring key = std::to_wstring(reinterpret_cast<uintptr_t>(device.Get())) + L":" +
            normalized + L":" + std::to_wstring(srgb) + L":" + std::to_wstring(forceLinear) + L":" +
            std::to_wstring(size) + L":" + std::to_wstring(modified.time_since_epoch().count());
        struct TEXTURE_LOAD_ENTRY final
        {
            std::mutex loadMutex;
            std::weak_ptr<ComPtr<ID3D11ShaderResourceView>> texture;
        };
        static std::mutex cacheMutex;
        static std::unordered_map<std::wstring, std::shared_ptr<TEXTURE_LOAD_ENTRY>> cache;
        static size_t insertions = 0u;
        std::shared_ptr<TEXTURE_LOAD_ENTRY> entry;
        {
            Engine::CProfilerScope lookupScope(CGameInstance::Get().Get_Profiler(), "Texture.Cache.Lookup");
            std::lock_guard<std::mutex> lock(cacheMutex);
            const auto found = cache.find(key);
            if (found != cache.end()) entry = found->second;
            else
            {
                // Only idle entries can be inspected or removed under the map
                // lock. An active loader owns an additional strong reference.
                if ((++insertions % 256u) == 0u)
                    std::erase_if(cache, [](const auto& pair) {
                        if (pair.second.use_count() != 1) return false;
                        std::unique_lock idleLock(pair.second->loadMutex, std::try_to_lock);
                        return idleLock.owns_lock() && pair.second->texture.expired();
                    });
                entry = std::make_shared<TEXTURE_LOAD_ENTRY>();
                cache.emplace(key, entry);
            }
        }
        std::unique_lock<std::mutex> loadLock;
        {
            Engine::CProfilerScope waitScope(CGameInstance::Get().Get_Profiler(), "Texture.Cache.SameKeyWait");
            loadLock = std::unique_lock<std::mutex>(entry->loadMutex);
        }
        auto shared = entry->texture.lock();
        if (!shared)
        {
            Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Texture.Load.FileAndUpload");
            shared = std::make_shared<ComPtr<ID3D11ShaderResourceView>>();
            const HRESULT result = LoadTexture(device, path, srgb, *shared, forceLinear);
            if (FAILED(result)) return result;
            entry->texture = shared;
        }
        view = *shared;
        owners.push_back(std::move(shared));
        return S_OK;
    }

	HRESULT AddTexture(ComPtr<ID3D11Device> pDevice,
        vector<shared_ptr<ComPtr<ID3D11ShaderResourceView>>>& owners,
		const filesystem::path& path,
		aiTextureType type,
		vector<ComPtr<ID3D11ShaderResourceView>> (&textures)[AI_TEXTURE_TYPE_MAX])
	{
		if (path.empty())
			return S_OK;
		ComPtr<ID3D11ShaderResourceView> resource;
		const HRESULT result = LoadSharedTexture(
			pDevice, owners, path, IsColorTextureSlot(type), resource);
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
	else if (FAILED(AddTexture(m_pDevice, m_SharedTextureViews, compatibleDiffusePath,
		aiTextureType_DIFFUSE, m_Textures)))
		return E_FAIL;

	if (FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.normalPath,
			aiTextureType_NORMALS, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.specularPath,
			aiTextureType_SPECULAR, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.emissivePath,
			aiTextureType_EMISSIVE, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.opacityPath,
			aiTextureType_OPACITY, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.ormPath,
			aiTextureType_UNKNOWN, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.metallicPath,
			aiTextureType_METALNESS, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.roughnessPath,
			aiTextureType_DIFFUSE_ROUGHNESS, m_Textures)) ||
		FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.ambientOcclusionPath,
			aiTextureType_AMBIENT_OCCLUSION, m_Textures)))
		return E_FAIL;

	/* The colour mask samples as data, not colour: aiTextureType_BASE_COLOR
	is outside IsColorTextureSlot, so it loads without the sRGB flag. */
	if (FAILED(AddTexture(m_pDevice, m_SharedTextureViews, material.colorMaskPath,
		aiTextureType_BASE_COLOR, m_Textures)))
		return E_FAIL;
	m_ColorTint = material.colorTint;
	m_ColorTint.isEnabled = material.colorTint.isEnabled &&
		Has_Texture(aiTextureType_BASE_COLOR);
	m_ColorTint.isHairMask = !material.colorMaskPath.empty() &&
		material.colorMaskPath == material.diffusePath;
	m_AuthoredColorTint = m_ColorTint;
	m_Surface = material.surface;
    if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
    {
        const auto mask = m_Surface.sourceCharacter.baseTextureMask |
            m_Surface.sourceCharacter.lightTextureMask;
        for (uint32_t index = 0u; index < SOURCE_CHARACTER_TEXTURE_COUNT; ++index)
        {
            if ((mask & (1u << index)) == 0u) continue;
            const auto& input = material.sourceCharacterTextures[index];
            if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, input.path, input.srgb,
                m_SourceCharacterTextures[index], true))) return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
            m_SourceCharacterTextures[index]->GetDesc(&desc);
            if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
        }
        if (m_Surface.hasStaticShadow)
        {
            if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.staticShadowPath, false, m_StaticShadow, true))) return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC shadow{}; m_StaticShadow->GetDesc(&shadow);
            if (shadow.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D || shadow.Format != DXGI_FORMAT_R8_UNORM) return E_INVALIDARG;
        }
        if (m_Surface.hasBakedLighting)
        {
            const uint32_t program = m_Surface.sourceCharacter.program;
            const bool supportsBaked = (program >= 80u && program <= 83u) ||
                (program >= 40u && program <= 63u && program != 47u && program != 53u && program != 55u);
            if (!supportsBaked ||
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.bakedAveragePath, m_Surface.bakedLightingSRGB, m_BakedAverage, true)) ||
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.bakedDirectionalPath, m_Surface.bakedLightingSRGB, m_BakedDirectional, true))) return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC average{}, directional{};
            m_BakedAverage->GetDesc(&average); m_BakedDirectional->GetDesc(&directional);
            if (average.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
                directional.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
        }
        m_AuthoredSourceCharacter = m_Surface.sourceCharacter;
        return S_OK;
    }
	if (m_Surface.hasEmissive)
	{
		if ((m_Surface.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE &&
			m_Surface.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
            m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED) ||
			FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceEmissivePath,
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
        const bool sourceSpecial = m_Surface.family >= MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE &&
            m_Surface.family <= MODEL_SURFACE_FAMILY::SOURCE_WET_OPAQUE;
		const auto& diffuse = material.surfaceDiffusePath.empty() ? material.diffusePath : material.surfaceDiffusePath;
		if (diffuse.empty() ||
            (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial && material.normalPath.empty()) ||
            (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial && material.reflectionPath.empty()) ||
			FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, diffuse,
				m_Surface.diffuseSRGB, m_SurfaceDiffuse, true)) ||
            (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial &&
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.reflectionPath,
                    m_Surface.reflectionSRGB, m_SurfaceReflection, true))))
		{
			OutputDebugStringA(("[CMaterial] Source surface texture load failed: " +
				m_strName + "\n").c_str());
			return E_FAIL;
		}
		if (m_Surface.family == MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE ||
			m_Surface.family == MODEL_SURFACE_FAMILY::PBR_OPAQUE)
		{
			if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceNormalPath, false, m_SurfaceNormal, true)) ||
				FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.detailNormalPath, false, m_SurfaceDetailNormal, true)) ||
				FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceORMPath, m_Surface.ormSRGB, m_SurfaceORM, true)))
			{
				OutputDebugStringA(("[CMaterial] PBR input load failed: " + m_strName + "\n").c_str());
				return E_FAIL;
			}
		}
        if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED ||
            m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED)
        {
            const uint32_t flags = m_Surface.sourceFoliageFlags;
            if (((flags & 1u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceNormalPath, false, m_SurfaceNormal, true))) ||
                ((flags & 8u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceSpecularPath, m_Surface.specularSRGB, m_SurfaceSpecular, true))) ||
                (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
                    FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.sourceFoliageMaskPath, m_Surface.sourceFoliageMaskSRGB, m_SourceFoliageMask, true)))) return E_FAIL;
            for (const auto& texture : { m_SurfaceDiffuse, m_SurfaceNormal, m_SurfaceSpecular, m_SourceFoliageMask })
            {
                if (!texture) continue;
                D3D11_SHADER_RESOURCE_VIEW_DESC desc{}; texture->GetDesc(&desc);
                if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
            }
        }
        if (sourceSpecial)
        {
            const auto& special = m_Surface.sourceSpecial;
            const bool ice = m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE;
            const bool blend = m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_VERTEXBLEND_OPAQUE;
            const auto load = [&](bool required, const filesystem::path& path, bool srgb,
                ComPtr<ID3D11ShaderResourceView>& texture) -> HRESULT
            {
                if (!required) return S_OK;
                const auto hr = LoadSharedTexture(m_pDevice, m_SharedTextureViews, path, srgb, texture, true);
                if (FAILED(hr)) return hr;
                D3D11_SHADER_RESOURCE_VIEW_DESC desc{}; texture->GetDesc(&desc);
                return desc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D ? S_OK : E_INVALIDARG;
            };
            if (FAILED(load(true, material.surfaceNormalPath, false, m_SurfaceNormal)) ||
                FAILED(load(!blend, material.reflectionPath, m_Surface.reflectionSRGB, m_SurfaceReflection)) ||
                FAILED(load(!blend && (!ice || (special.flags & 2u)), material.surfaceSpecularPath, m_Surface.specularSRGB, m_SurfaceSpecular)) ||
                FAILED(load(blend || (ice && (special.flags & 1u)), material.detailNormalPath, false, m_SurfaceDetailNormal)) ||
                FAILED(load(blend, material.overlayDiffusePath, m_Surface.overlaySRGB, m_SurfaceOverlayDiffuse)) ||
                FAILED(load(blend, material.overlayNormalPath, false, m_SurfaceOverlayNormal)) ||
                FAILED(load(ice, material.sourceSpecialMaskPath, special.maskSRGB, m_SourceSpecialMask)) ||
                FAILED(load(blend && (special.flags & 1u), material.sourceBlendDiffuseGPath, special.blendGSRGB, m_SourceBlendDiffuseG)) ||
                FAILED(load(blend && (special.flags & 1u), material.sourceBlendNormalGPath, false, m_SourceBlendNormalG)) ||
                FAILED(load(blend && (special.flags & 2u), material.sourceBlendDiffuseBPath, special.blendBSRGB, m_SourceBlendDiffuseB)) ||
                FAILED(load(blend && (special.flags & 2u), material.sourceBlendNormalBPath, false, m_SourceBlendNormalB))) return E_FAIL;
        }
        if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED)
        {
            const uint32_t flags = m_Surface.sourceBgFlags;
            if ((flags & 32768u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews,
                material.detailNormalPath, false, m_SurfaceDetailNormal, true))) return E_FAIL;
            if (((flags & 1u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceNormalPath, false, m_SurfaceNormal, true))) ||
                ((flags & 8u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceSpecularPath, m_Surface.specularSRGB, m_SurfaceSpecular, true))) ||
                ((flags & 16u) && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.reflectionPath, m_Surface.reflectionSRGB, m_SurfaceReflection, true))))
                return E_FAIL;
        }
        if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE &&
            (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceNormalPath, false, m_SurfaceNormal, true)) ||
             FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceSpecularPath, m_Surface.specularSRGB, m_SurfaceSpecular, true))))
        {
            OutputDebugStringA(("[CMaterial] Source opaque inputs failed: " + m_strName + "\n").c_str());
            return E_FAIL;
        }
        if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE)
        {
            if (m_Surface.overlaySeparateSpecular && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews,
                material.surfaceSpecularPath, m_Surface.specularSRGB, m_SurfaceSpecular, true))) return E_FAIL;
            if (((m_Surface.sourceOverlayFlags & 1u) != 0u && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.surfaceNormalPath, false, m_SurfaceNormal, true))) ||
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.overlayDiffusePath, m_Surface.overlaySRGB, m_SurfaceOverlayDiffuse, true)) ||
                ((m_Surface.sourceOverlayFlags & 2u) != 0u && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.overlayNormalPath, false, m_SurfaceOverlayNormal, true))) ||
                ((m_Surface.sourceOverlayFlags & 32u) != 0u && FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.detailNormalPath, false, m_SurfaceDetailNormal, true)))) return E_FAIL;
            for (const auto& input : { m_SurfaceDiffuse, m_SurfaceNormal, m_SurfaceOverlayDiffuse, m_SurfaceOverlayNormal })
            {
                if (!input) continue;
                D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
                input->GetDesc(&desc);
                if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
            }
        }
        if (m_Surface.hasStaticShadow)
        {
            if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.staticShadowPath, false, m_StaticShadow, true))) return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC shadow{}; m_StaticShadow->GetDesc(&shadow);
            if (shadow.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D || shadow.Format != DXGI_FORMAT_R8_UNORM) return E_INVALIDARG;
        }
        if (m_Surface.hasBakedLighting)
        {
            if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.bakedAveragePath, m_Surface.bakedLightingSRGB, m_BakedAverage, true)) ||
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.bakedDirectionalPath, m_Surface.bakedLightingSRGB, m_BakedDirectional, true)))
                return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC average{}, directional{};
            m_BakedAverage->GetDesc(&average); m_BakedDirectional->GetDesc(&directional);
            if (average.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
                directional.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
        }
        if (m_Surface.hasEnvironmentCube)
        {
            if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.environmentCubePath, false, m_EnvironmentCube, true)) ||
                FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.environmentBRDFPath, false, m_EnvironmentBRDF, true)))
                return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC cube{};
            m_EnvironmentCube->GetDesc(&cube);
            D3D11_SHADER_RESOURCE_VIEW_DESC brdf{};
            m_EnvironmentBRDF->GetDesc(&brdf);
            if (cube.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBE ||
                brdf.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) return E_INVALIDARG;
        }
		if (MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION == m_Surface.family &&
			FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.specularPath,
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

namespace
{
	/* One key per (slot, index) pair; the index is small in every shipped material. */
	constexpr uint32_t Texture_OverrideKey(
		const aiTextureType eType, const uint32_t iTextureIndex)
	{
		return (static_cast<uint32_t>(eType) << 8) | (iTextureIndex & 0xFFu);
	}
}

void CMaterial::Set_TextureOverride(
	const aiTextureType eType,
	const uint32_t iTextureIndex,
	ComPtr<ID3D11ShaderResourceView> pTexture)
{
	if (eType >= AI_TEXTURE_TYPE_MAX)
		return;
	const uint32_t key = Texture_OverrideKey(eType, iTextureIndex);
	if (nullptr == pTexture)
		m_TextureOverrides.erase(key);
	else
		m_TextureOverrides[key] = pTexture;
}

void CMaterial::Clear_TextureOverrides()
{
	m_TextureOverrides.clear();
}

bool_t CMaterial::Set_SourceCharacterConstants(
	const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters)
{
	if (!Has_SourceCharacterProgram() ||
		parameters.program != m_Surface.sourceCharacter.program ||
		parameters.baseTextureMask != m_Surface.sourceCharacter.baseTextureMask ||
		parameters.lightTextureMask != m_Surface.sourceCharacter.lightTextureMask)
	{
		return false;
	}
	/* The same bound the loader holds these to. A slider that produced a NaN would otherwise
	spread through the whole lighting row rather than showing up as one wrong value. */
	for (const auto* constants : { &parameters.baseConstants, &parameters.lightConstants })
		for (const auto& value : *constants)
			for (const f32_t scalar : { value.x, value.y, value.z, value.w })
				if (!std::isfinite(scalar) || std::abs(scalar) > 1000000.f)
					return false;
	m_Surface.sourceCharacter = parameters;
	return true;
}

bool_t CMaterial::Set_SourceCharacterTextureOverride(
	const uint32_t iRegister, ComPtr<ID3D11ShaderResourceView> pTexture)
{
	const uint32_t mask = m_Surface.sourceCharacter.baseTextureMask |
		m_Surface.sourceCharacter.lightTextureMask;
	if (!Has_SourceCharacterProgram() || iRegister >= SOURCE_CHARACTER_TEXTURE_COUNT ||
		0u == (mask & (1u << iRegister)))
	{
		return false;
	}
	if (nullptr == pTexture)
		m_SourceCharacterTextureOverrides.erase(iRegister);
	else
		m_SourceCharacterTextureOverrides[iRegister] = pTexture;
	return true;
}

void CMaterial::Clear_SourceCharacterOverrides()
{
	m_SourceCharacterTextureOverrides.clear();
	if (Has_SourceCharacterProgram())
		m_Surface.sourceCharacter = m_AuthoredSourceCharacter;
}

void CMaterial::Set_DyeColorOverride(
	const float4_t& vDiffuse, const float4_t& vRegionA)
{
	if (!m_ColorTint.isEnabled)
		return;
	m_ColorTint.vDiffuse = vDiffuse;
	m_ColorTint.vRegionA = vRegionA;
}

void CMaterial::Clear_DyeColorOverride()
{
	m_ColorTint = m_AuthoredColorTint;
}

void CMaterial::Set_DyeTwoTone(const f32_t fStrength, const f32_t fRange)
{
	if (!m_ColorTint.isEnabled || !m_ColorTint.isHairMask)
		return;
	m_ColorTint.vDiffuse.w = isfinite(fStrength) ? max(0.f, min(1.f, fStrength)) : 0.f;
	m_ColorTint.vRegionA.w = isfinite(fRange) ? max(0.f, min(1.f, fRange)) : 0.f;
}

HRESULT CMaterial::Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, aiTextureType eType, uint32_t iTextureIndex)
{
	if (eType >= AI_TEXTURE_TYPE_MAX)
		return E_FAIL;
	/* An override may exist for a slot the authored material left empty -- a face that never
	shipped a decal still has to be able to wear one. */
	if (const auto found = m_TextureOverrides.find(Texture_OverrideKey(eType, iTextureIndex));
		found != m_TextureOverrides.end())
	{
		if (aiTextureType_DIFFUSE == eType)
		{
			const uint32_t disabled = 0u;
			pShader->Bind_RawValue("g_SurfaceProgram", &disabled, sizeof(disabled));
			pShader->Bind_RawValue("g_HasSurfaceDefinition", &disabled, sizeof(disabled));
		}
		return pShader->Bind_Texture(pConstantName, found->second);
	}
	if (iTextureIndex >= m_Textures[eType].size())
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
    const uint32_t hasBaked = m_Surface.hasBakedLighting ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_SourceMapMonsterBakedEnabled", &hasBaked, sizeof(hasBaked)))) return E_FAIL;
    if (!lightPass)
    {
        if (source.program >= 80u && source.program <= 83u && FAILED(Bind_StaticShadow(shader))) return E_FAIL;
        if (hasBaked && (FAILED(shader->Bind_Texture("g_SourceMapMonsterAverageTexture", m_BakedAverage)) ||
            FAILED(shader->Bind_Texture("g_SourceMapMonsterDirectionalTexture", m_BakedDirectional)))) return E_FAIL;
        const auto environment = CGameInstance::Get().Get_RenderEnvironment();
        const uint32_t enabled = environment.pCube ? 1u : 0u;
        if (FAILED(shader->Bind_RawValue("g_SourceCharacterEnvironmentEnabled", &enabled, sizeof(enabled))) ||
            FAILED(shader->Bind_RawValue("g_SourceCharacterEnvironmentColor", &environment.vColor, sizeof(float4_t))) ||
            FAILED(shader->Bind_RawValue("g_SourceCharacterEnvironmentRotation", &environment.vRotationIntensity, sizeof(float4_t))) ||
            FAILED(shader->Bind_Texture("g_SourceCharacterEnvironmentCube", environment.pCube))) return E_FAIL;
    }
    const bool forward = source.program >= 38u && source.program <= 63u;
    if (!lightPass && forward && FAILED(shader->Bind_RawValue("g_SourceCharacterLightConstants",
        source.lightConstants.data(), sizeof(source.lightConstants)))) return E_FAIL;
    const auto& constants = lightPass ? source.lightConstants : source.baseConstants;
    const uint32_t mask = lightPass ? source.lightTextureMask :
        source.baseTextureMask | (forward ? source.lightTextureMask : 0u);
    if (FAILED(shader->Bind_RawValue("g_SourceCharacterTime", &g_SourceCharacterTime, sizeof(g_SourceCharacterTime))) ||
        FAILED(shader->Bind_RawValue("g_SourceCharacterProgram", &source.program, sizeof(source.program))) ||
        FAILED(shader->Bind_RawValue("g_SourceCharacterRow", &row, sizeof(row))) ||
        FAILED(shader->Bind_RawValue(lightPass ? "g_SourceCharacterLightConstants" :
            "g_SourceCharacterBaseConstants", constants.data(), sizeof(constants)))) return E_FAIL;
    for (uint32_t index = 0u; index < SOURCE_CHARACTER_TEXTURE_COUNT; ++index)
    {
        if ((mask & (1u << index)) == 0u) continue;
        const auto name = std::string("g_SourceCharacterTexture") + std::to_string(index);
        /* A creation choice repaints the register on the clone; the authored view stays put. */
        const auto repainted = m_SourceCharacterTextureOverrides.find(index);
        if (FAILED(shader->Bind_Texture(name.c_str(),
            repainted != m_SourceCharacterTextureOverrides.end() ?
            repainted->second : m_SourceCharacterTextures[index]))) return E_FAIL;
    }
    return S_OK;
}

HRESULT CMaterial::Bind_SourceCharacter(shared_ptr<CShader> shader)
{
    if (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_CHARACTER) return S_FALSE;
    const uint32_t program = m_Surface.sourceCharacter.program;
    if (program >= 33u && program <= 65u)
        return Bind_SourceCharacterInputs(shader, false, 0u);
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

HRESULT CMaterial::Bind_StaticShadow(shared_ptr<CShader> shader)
{
    if (!shader) return E_INVALIDARG;
    const uint32_t enabled = m_Surface.hasStaticShadow ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_HasStaticShadow", &enabled, sizeof(enabled))) ||
        FAILED(shader->Bind_RawValue("g_StaticShadowChannel", &m_Surface.staticShadowChannel, sizeof(m_Surface.staticShadowChannel))) ||
        FAILED(shader->Bind_RawValue("g_StaticShadowTransfer", &m_Surface.staticShadowTransfer, sizeof(m_Surface.staticShadowTransfer))) ||
        (enabled && FAILED(shader->Bind_Texture("g_StaticShadowTexture", m_StaticShadow)))) return E_FAIL;
    return S_OK;
}

HRESULT CMaterial::Bind_SourceSpecialSurface(shared_ptr<CShader> shader)
{
    if (!shader || m_Surface.family < MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE ||
        m_Surface.family > MODEL_SURFACE_FAMILY::SOURCE_WET_OPAQUE) return E_INVALIDARG;
    const auto& special = m_Surface.sourceSpecial;
    const bool ice = m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE;
    const bool blend = m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_VERTEXBLEND_OPAQUE;
    if (FAILED(shader->Bind_RawValue("g_SourceSpecialFlags", &special.flags, sizeof(special.flags)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceNormalTiling", &special.normalTiling, sizeof(special.normalTiling)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceIceCoreColor", &special.iceCoreColor, sizeof(special.iceCoreColor)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceIceOuterColor", &special.iceOuterColor, sizeof(special.iceOuterColor)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceIceBlend", &special.iceBlend, sizeof(special.iceBlend)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceIceBumpOffset", &special.iceBumpOffset, sizeof(special.iceBumpOffset)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceWetParameters", &special.wetParameters, sizeof(special.wetParameters)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceWetSpecularPower", &special.wetSpecularPower, sizeof(special.wetSpecularPower)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceBlendSharpness", &special.blendSharpness, sizeof(special.blendSharpness)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceBlendDiffuse", special.blendDiffuse.data(), sizeof(special.blendDiffuse)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceBlendSpecular", special.blendSpecular.data(), sizeof(special.blendSpecular)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceBlendLayers", special.blendLayers.data(), sizeof(special.blendLayers)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &m_Surface.uvTiling, sizeof(m_Surface.uvTiling)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalIntensity", &m_Surface.detailNormalIntensity, sizeof(m_Surface.detailNormalIntensity)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalTiling", &m_Surface.detailNormalTiling, sizeof(m_Surface.detailNormalTiling)))) return E_FAIL;
    if (FAILED(shader->Bind_RawValue("g_SourceBgRimlight", &m_Surface.sourceBgRimlight, sizeof(m_Surface.sourceBgRimlight)))) return E_FAIL;
    if (FAILED(shader->Bind_Texture("g_NormalTexture", m_SurfaceNormal)) ||
        (!blend && FAILED(shader->Bind_Texture("g_ReflectionTexture", m_SurfaceReflection))) ||
        (!blend && (!ice || (special.flags & 2u)) && FAILED(shader->Bind_Texture("g_SpecularTexture", m_SurfaceSpecular))) ||
        ((blend || (ice && (special.flags & 1u))) && FAILED(shader->Bind_Texture("g_DetailNormalTexture", m_SurfaceDetailNormal))) ||
        (blend && (FAILED(shader->Bind_Texture("g_SurfaceOverlayDiffuseTexture", m_SurfaceOverlayDiffuse)) ||
            FAILED(shader->Bind_Texture("g_SurfaceOverlayNormalTexture", m_SurfaceOverlayNormal)))) ||
        (ice && FAILED(shader->Bind_Texture("g_SourceSpecialMaskTexture", m_SourceSpecialMask))) ||
        (blend && (special.flags & 1u) && (FAILED(shader->Bind_Texture("g_SourceBlendDiffuseGTexture", m_SourceBlendDiffuseG)) ||
            FAILED(shader->Bind_Texture("g_SourceBlendNormalGTexture", m_SourceBlendNormalG)))) ||
        (blend && (special.flags & 2u) && (FAILED(shader->Bind_Texture("g_SourceBlendDiffuseBTexture", m_SourceBlendDiffuseB)) ||
            FAILED(shader->Bind_Texture("g_SourceBlendNormalBTexture", m_SourceBlendNormalB))))) return E_FAIL;
    return S_OK;
}

HRESULT CMaterial::Bind_SurfaceLighting(shared_ptr<CShader> shader)
{
    if (!shader) return E_INVALIDARG;
    if (FAILED(Bind_StaticShadow(shader))) return E_FAIL;
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
	/* The surface program reads slot 0 of the same type, so one override covers both paths. */
	if (const auto found = m_TextureOverrides.find(Texture_OverrideKey(eType, 0u));
		found != m_TextureOverrides.end())
	{
		return pShader->Bind_Texture(pConstantName, found->second);
	}
	switch (eType)
	{
	case aiTextureType_DIFFUSE: texture = m_SurfaceDiffuse; break;
	case aiTextureType_SPECULAR: texture = m_SurfaceSpecular; break;
	case aiTextureType_REFLECTION: texture = m_SurfaceReflection; break;
	case aiTextureType_NORMALS: texture = m_SurfaceNormal; break;
	case aiTextureType_HEIGHT: texture = m_SurfaceDetailNormal; break;
	case aiTextureType_UNKNOWN: texture = m_SurfaceORM; break;
	case aiTextureType_EMISSIVE: texture = m_SurfaceEmissive; break;
    case aiTextureType_TRANSMISSION: texture = m_SourceFoliageMask; break;
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
