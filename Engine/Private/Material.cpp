#include "Material.h"
#include <memory>
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")
#pragma push_macro("new")
#undef new
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#pragma pop_macro("new")

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
#include <limits>

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
        // Attempts/hits/new SRVs describe this shared-material path, not all
        // texture allocations or resident memory. Worker events land in the
        // active observed frame, so a request and its completion may differ.
        auto* profiler = CGameInstance::Get().Get_Profiler();
        if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::TextureRequests);
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
            if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::TextureUniqueSrvs);
        }
        else if (profiler)
            profiler->Add_Counter(Engine::EProfilerCounter::TexturePathHits);
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
    if (m_Surface.family == MODEL_SURFACE_FAMILY::SOURCE_LANDSCAPE_OPAQUE)
    {
        const auto& source = m_Surface.sourceLandscape;
        const auto& paths = material.sourceLandscapeTextures;
        if (!source.Has_ValidInputs() || (m_Surface.hasStaticShadow && !m_Surface.hasBakedLighting) ||
            m_Surface.hasEnvironmentCube || m_Surface.hasSourceIndirect || m_Surface.hasEmissive) return E_INVALIDARG;
        const auto load = [&](bool required, const filesystem::path& path, bool srgb,
            ComPtr<ID3D11ShaderResourceView>& texture) -> HRESULT {
            if (!required) return path.empty() ? S_OK : E_INVALIDARG;
            if (path.empty() || FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, path, srgb, texture, true))) return E_FAIL;
            D3D11_SHADER_RESOURCE_VIEW_DESC desc{}; texture->GetDesc(&desc);
            return desc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D ? S_OK : E_INVALIDARG;
        };
        for (uint32_t i = 0u; i < SOURCE_LANDSCAPE_LAYER_COUNT; ++i)
        {
            if (FAILED(load((source.layerMask & (1u << i)) != 0u, paths.diffuse[i], source.diffuseSRGB[i], m_SourceLandscapeDiffuse[i])) ||
                FAILED(load((source.normalMask & (1u << i)) != 0u, paths.normal[i], false, m_SourceLandscapeNormal[i]))) return E_FAIL;
            // Existing surface diagnostics retain a valid diffuse view; the
            // painted evaluator reads the complete selected layer set below.
            if (!m_SurfaceDiffuse && m_SourceLandscapeDiffuse[i]) m_SurfaceDiffuse = m_SourceLandscapeDiffuse[i];
        }
        for (uint32_t i = 0u; i < SOURCE_LANDSCAPE_WEIGHTMAP_COUNT; ++i)
            if (FAILED(load(i < source.weightmapCount, paths.weightmaps[i], false, m_SourceLandscapeWeights[i]))) return E_FAIL;
        if (FAILED(load(true, paths.heightmap, false, m_SourceLandscapeHeight)) ||
            FAILED(load(m_Surface.hasBakedLighting, material.bakedAveragePath, m_Surface.bakedLightingSRGB, m_BakedAverage)) ||
            FAILED(load(m_Surface.hasBakedLighting, material.bakedDirectionalPath, m_Surface.bakedLightingSRGB, m_BakedDirectional)) ||
            FAILED(load(m_Surface.hasStaticShadow, material.staticShadowPath, false, m_StaticShadow))) return E_FAIL;
        if (m_Surface.hasStaticShadow)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC shadow{}; m_StaticShadow->GetDesc(&shadow);
            if (shadow.Format != DXGI_FORMAT_R8_UNORM) return E_INVALIDARG;
        }
        return S_OK;
    }
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
                (program >= 40u && program <= 63u && program != 47u && program != 53u && program != 55u) ||
                program == 209u || program == 210u || (program >= 214u && program <= 234u) || program == 237u ||
                (program >= 1100u && program <= 1166u) || (program >= 1400u && program <= 1413u);
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
            m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
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
             m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial &&
             m_Surface.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
             m_Surface.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE && material.normalPath.empty()) ||
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
            if (m_Surface.hasSourceIndirect)
            {
                if (FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.sourceIndirectBRDFPath,
                    false, m_SourceIndirectBRDF, true)) ||
                    FAILED(LoadSharedTexture(m_pDevice, m_SharedTextureViews, material.sourceIndirectCubePath,
                        false, m_SourceIndirectCube, true))) return E_FAIL;
                D3D11_SHADER_RESOURCE_VIEW_DESC sourceCube{};
                m_SourceIndirectCube->GetDesc(&sourceCube);
                if (sourceCube.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBE) return E_INVALIDARG;
                D3D11_SHADER_RESOURCE_VIEW_DESC sourceBRDF{};
                m_SourceIndirectBRDF->GetDesc(&sourceBRDF);
                if (sourceBRDF.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
                    sourceBRDF.Format != DXGI_FORMAT_R16G16_UNORM) return E_INVALIDARG;
            }
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
    struct SOURCE_CHARACTER_ROW final
    {
        // Keep aliases alive too: CModel uses shared ownership for copy-on-write.
        std::shared_ptr<CMaterial> Material;
        uint32_t Row;
    };
    thread_local std::unordered_map<const CMaterial*, SOURCE_CHARACTER_ROW> g_SourceCharacterRows;
    // The row lives in the R32_FLOAT depth target, not an eight-bit material index.
    // Every positive integer through 2^24 is exactly representable there.
    constexpr uint32_t SOURCE_CHARACTER_MAX_EXACT_ROW =
        1u << std::numeric_limits<float>::digits;
    static_assert(std::numeric_limits<float>::is_iec559 && std::numeric_limits<float>::digits == 24);
    thread_local float g_SourceCharacterTime = 0.f;
}

void CMaterial::Reset_SourceCharacterFrame(float presentationTime)
{
    g_SourceCharacterRows.clear();
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
        if (((source.program >= 80u && source.program <= 83u) ||
            (source.program >= 214u && source.program <= 234u) || source.program == 237u) && FAILED(Bind_StaticShadow(shader))) return E_FAIL;
        if (hasBaked && (FAILED(shader->Bind_Texture("g_SourceMapMonsterAverageTexture", m_BakedAverage)) ||
            FAILED(shader->Bind_Texture("g_SourceMapMonsterDirectionalTexture", m_BakedDirectional)))) return E_FAIL;
        const auto environment = CGameInstance::Get().Get_RenderEnvironment();
        const uint32_t enabled = environment.pCube ? 1u : 0u;
        if (FAILED(shader->Bind_RawValue("g_SourceCharacterEnvironmentEnabled", &enabled, sizeof(enabled))) ||
            FAILED(shader->Bind_RawValue("g_SourceCharacterEnvironmentColor", &environment.vColor, sizeof(float4_t))) ||
            FAILED(shader->Bind_RawValue("g_SourceCharacterEnvironmentRotation", &environment.vRotationIntensity, sizeof(float4_t))) ||
            FAILED(shader->Bind_Texture("g_SourceCharacterEnvironmentCube", environment.pCube))) return E_FAIL;
    }
    const bool forward = (source.program >= 38u && source.program <= 63u) || source.program == 209u ||
        (source.program >= 224u && source.program <= 226u) || source.program == 237u;
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
    static constexpr const char_t* textureNames[] =
    {
        "g_SourceCharacterTexture0", "g_SourceCharacterTexture1", "g_SourceCharacterTexture2", "g_SourceCharacterTexture3",
        "g_SourceCharacterTexture4", "g_SourceCharacterTexture5", "g_SourceCharacterTexture6", "g_SourceCharacterTexture7",
        "g_SourceCharacterTexture8", "g_SourceCharacterTexture9", "g_SourceCharacterTexture10", "g_SourceCharacterTexture11",
        "g_SourceCharacterTexture12", "g_SourceCharacterTexture13", "g_SourceCharacterTexture14", "g_SourceCharacterTexture15"
    };
    static_assert(std::size(textureNames) == SOURCE_CHARACTER_TEXTURE_COUNT);
    for (uint32_t index = 0u; index < SOURCE_CHARACTER_TEXTURE_COUNT; ++index)
    {
        if ((mask & (1u << index)) == 0u) continue;
        /* A creation choice repaints the register on the clone; the authored view stays put. */
        const auto repainted = m_SourceCharacterTextureOverrides.find(index);
        if (FAILED(shader->Bind_Texture(textureNames[index],
            repainted != m_SourceCharacterTextureOverrides.end() ?
            repainted->second : m_SourceCharacterTextures[index]))) return E_FAIL;
    }
    return S_OK;
}

HRESULT CMaterial::Bind_SourceCharacter(shared_ptr<CShader> shader)
{
    if (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_CHARACTER) return S_FALSE;
    const uint32_t program = m_Surface.sourceCharacter.program;
    if ((program >= 33u && program <= 65u) || program == 209u ||
        (program >= 224u && program <= 226u) || program == 237u)
        return Bind_SourceCharacterInputs(shader, false, 0u);
    const auto found = g_SourceCharacterRows.find(this);
    if (found != g_SourceCharacterRows.end())
        return Bind_SourceCharacterInputs(shader, false, found->second.Row);

    // Deferred rows identify light inputs, not a particular material instance.
    // The base pass still binds this material's own constants and textures.
    const auto sameLightInputs = [&](const CMaterial& other)
    {
        const auto& source = m_Surface.sourceCharacter;
        const auto& candidate = other.m_Surface.sourceCharacter;
        if (source.program != candidate.program ||
            source.lightTextureMask != candidate.lightTextureMask ||
            m_Surface.hasBakedLighting != other.m_Surface.hasBakedLighting ||
            0 != std::memcmp(source.lightConstants.data(), candidate.lightConstants.data(),
                sizeof(source.lightConstants))) return false;
        for (uint32_t index = 0u; index < SOURCE_CHARACTER_TEXTURE_COUNT; ++index)
        {
            if ((source.lightTextureMask & (1u << index)) == 0u) continue;
            const auto repainted = m_SourceCharacterTextureOverrides.find(index);
            const auto otherRepainted = other.m_SourceCharacterTextureOverrides.find(index);
            const auto* texture = repainted != m_SourceCharacterTextureOverrides.end() ?
                repainted->second.Get() : m_SourceCharacterTextures[index].Get();
            const auto* otherTexture = otherRepainted != other.m_SourceCharacterTextureOverrides.end() ?
                otherRepainted->second.Get() : other.m_SourceCharacterTextures[index].Get();
            if (texture != otherTexture) return false;
        }
        return true;
    };
    for (size_t index = 0u; index < g_SourceCharacterFrame.size(); ++index)
    {
        if (!sameLightInputs(*g_SourceCharacterFrame[index])) continue;
        const uint32_t row = static_cast<uint32_t>(index) + 1u;
        const HRESULT result = Bind_SourceCharacterInputs(shader, false, row);
        if (SUCCEEDED(result))
            g_SourceCharacterRows.emplace(this, SOURCE_CHARACTER_ROW{shared_from_this(), row});
        return result;
    }
    if (g_SourceCharacterFrame.size() >= SOURCE_CHARACTER_MAX_EXACT_ROW) return E_BOUNDS;
    // Row IDs are ephemeral render indices, never serialized asset IDs.
    const uint32_t row = static_cast<uint32_t>(g_SourceCharacterFrame.size()) + 1u;
    const HRESULT result = Bind_SourceCharacterInputs(shader, false, row);
    if (FAILED(result)) return result;
    g_SourceCharacterFrame.push_back(shared_from_this());
    g_SourceCharacterRows.emplace(this, SOURCE_CHARACTER_ROW{shared_from_this(), row});
    return S_OK;
}

HRESULT CMaterial::Bind_SourceCharacterForwardLight(shared_ptr<CShader> shader)
{
    if (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_CHARACTER) return E_INVALIDARG;
    return Bind_SourceCharacterInputs(shader, true, 0u);
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

HRESULT CMaterial::Bind_SourceLandscapeSurface(shared_ptr<CShader> shader)
{
    if (!shader || m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_LANDSCAPE_OPAQUE) return E_INVALIDARG;
    const auto& source = m_Surface.sourceLandscape;
    for (const auto& pair : { std::pair<const char*, const uint32_t*>("g_SourceLandscapeLayerMask", &source.layerMask),
        { "g_SourceLandscapeNormalMask", &source.normalMask }, { "g_SourceLandscapeWeightmapCount", &source.weightmapCount } })
        if (FAILED(shader->Bind_RawValue(pair.first, pair.second, sizeof(uint32_t)))) return E_FAIL;
    for (const auto& pair : { std::pair<const char*, const float4_t*>("g_SourceLandscapeGrid", &source.grid),
        { "g_SourceLandscapeWeightmapScaleBias", &source.weightmapScaleBias },
        { "g_SourceLandscapeHeightmapScaleBias", &source.heightmapScaleBias } })
        if (FAILED(shader->Bind_RawValue(pair.first, pair.second, sizeof(float4_t)))) return E_FAIL;
    const std::pair<const char*, const std::array<float4_t, SOURCE_LANDSCAPE_LAYER_COUNT>*> arrays[] = {
        { "g_SourceLandscapeUV", &source.uv }, { "g_SourceLandscapeDiffuse", &source.diffuse },
        { "g_SourceLandscapeSpecular", &source.specular }, { "g_SourceLandscapeFactors", &source.factors },
        { "g_SourceLandscapeWeight", &source.weight }
    };
    for (const auto& pair : arrays)
        if (FAILED(shader->Bind_RawValue(pair.first, pair.second->data(), sizeof(*pair.second)))) return E_FAIL;
    for (uint32_t i = 0u; i < SOURCE_LANDSCAPE_LAYER_COUNT; ++i)
    {
        if ((source.layerMask & (1u << i)) && FAILED(shader->Bind_Texture(
            ("g_SourceLandscapeLayerDiffuse" + std::to_string(i)).c_str(), m_SourceLandscapeDiffuse[i]))) return E_FAIL;
        if ((source.normalMask & (1u << i)) && FAILED(shader->Bind_Texture(
            ("g_SourceLandscapeLayerNormal" + std::to_string(i)).c_str(), m_SourceLandscapeNormal[i]))) return E_FAIL;
    }
    for (uint32_t i = 0u; i < source.weightmapCount; ++i)
        if (FAILED(shader->Bind_Texture(("g_SourceLandscapeWeightmap" + std::to_string(i)).c_str(), m_SourceLandscapeWeights[i]))) return E_FAIL;
    return shader->Bind_Texture("g_SourceLandscapeHeightmap", m_SourceLandscapeHeight);
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

namespace
{
    template<class T> bool SameStaticLightingValue(const T& a, const T& b) { return a == b; }
    bool SameStaticLightingValue(const float2_t& a, const float2_t& b) { return a.x == b.x && a.y == b.y; }
    bool SameStaticLightingValue(const float3_t& a, const float3_t& b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
    bool SameStaticLightingValue(const float4_t& a, const float4_t& b) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }
    template<class T, size_t N> bool SameStaticLightingValue(const std::array<T, N>& a, const std::array<T, N>& b)
    {
        for (size_t i = 0u; i < N; ++i) if (!SameStaticLightingValue(a[i], b[i])) return false;
        return true;
    }
    template<class T, size_t N> bool SameStaticLightingValue(const T (&a)[N], const T (&b)[N])
    {
        for (size_t i = 0u; i < N; ++i) if (!SameStaticLightingValue(a[i], b[i])) return false;
        return true;
    }
    bool SameStaticLightingValue(const MODEL_COLOR_TINT& a, const MODEL_COLOR_TINT& b)
    {
        return SameStaticLightingValue(a.isEnabled, b.isEnabled)
            && SameStaticLightingValue(a.isHairMask, b.isHairMask)
            && SameStaticLightingValue(a.vDiffuse, b.vDiffuse)
            && SameStaticLightingValue(a.vRegionA, b.vRegionA)
            && SameStaticLightingValue(a.vRegionB, b.vRegionB)
            && SameStaticLightingValue(a.vRegionC, b.vRegionC);
    }
    bool SameStaticLightingValue(const MODEL_SOURCE_CHARACTER_PARAMETERS& a, const MODEL_SOURCE_CHARACTER_PARAMETERS& b)
    {
        return SameStaticLightingValue(a.program, b.program)
            && SameStaticLightingValue(a.baseTextureMask, b.baseTextureMask)
            && SameStaticLightingValue(a.lightTextureMask, b.lightTextureMask)
            && SameStaticLightingValue(a.requiredExtraUVMask, b.requiredExtraUVMask)
            && SameStaticLightingValue(a.baseConstants, b.baseConstants)
            && SameStaticLightingValue(a.lightConstants, b.lightConstants);
    }
    bool SameStaticLightingValue(const MODEL_SOURCE_SPECIAL_PARAMETERS& a, const MODEL_SOURCE_SPECIAL_PARAMETERS& b)
    {
        return SameStaticLightingValue(a.flags, b.flags)
            && SameStaticLightingValue(a.normalTiling, b.normalTiling)
            && SameStaticLightingValue(a.iceCoreColor, b.iceCoreColor)
            && SameStaticLightingValue(a.iceOuterColor, b.iceOuterColor)
            && SameStaticLightingValue(a.iceBlend, b.iceBlend)
            && SameStaticLightingValue(a.iceBumpOffset, b.iceBumpOffset)
            && SameStaticLightingValue(a.wetParameters, b.wetParameters)
            && SameStaticLightingValue(a.wetSpecularPower, b.wetSpecularPower)
            && SameStaticLightingValue(a.blendDiffuse, b.blendDiffuse)
            && SameStaticLightingValue(a.blendSpecular, b.blendSpecular)
            && SameStaticLightingValue(a.blendLayers, b.blendLayers)
            && SameStaticLightingValue(a.blendSharpness, b.blendSharpness)
            && SameStaticLightingValue(a.maskSRGB, b.maskSRGB)
            && SameStaticLightingValue(a.blendGSRGB, b.blendGSRGB)
            && SameStaticLightingValue(a.blendBSRGB, b.blendBSRGB);
    }
    bool SameStaticLightingValue(const MODEL_SOURCE_LANDSCAPE_PARAMETERS& a, const MODEL_SOURCE_LANDSCAPE_PARAMETERS& b)
    {
        return SameStaticLightingValue(a.layerMask, b.layerMask)
            && SameStaticLightingValue(a.normalMask, b.normalMask)
            && SameStaticLightingValue(a.weightmapCount, b.weightmapCount)
            && SameStaticLightingValue(a.grid, b.grid)
            && SameStaticLightingValue(a.weightmapScaleBias, b.weightmapScaleBias)
            && SameStaticLightingValue(a.heightmapScaleBias, b.heightmapScaleBias)
            && SameStaticLightingValue(a.uv, b.uv)
            && SameStaticLightingValue(a.diffuse, b.diffuse)
            && SameStaticLightingValue(a.specular, b.specular)
            && SameStaticLightingValue(a.factors, b.factors)
            && SameStaticLightingValue(a.weight, b.weight)
            && SameStaticLightingValue(a.diffuseSRGB, b.diffuseSRGB);
    }
    bool SameStaticLightingValue(const MODEL_SURFACE_PARAMETERS& a, const MODEL_SURFACE_PARAMETERS& b)
    {
        return SameStaticLightingValue(a.renderMode, b.renderMode)
            && SameStaticLightingValue(a.cullMode, b.cullMode)
            && SameStaticLightingValue(a.family, b.family)
            && SameStaticLightingValue(a.pbrAlphaMasked, b.pbrAlphaMasked)
            && SameStaticLightingValue(a.sourceCharacter, b.sourceCharacter)
            && SameStaticLightingValue(a.sourceBgFlags, b.sourceBgFlags)
            && SameStaticLightingValue(a.sourceBgUnlit, b.sourceBgUnlit)
            && SameStaticLightingValue(a.sourceBgBump, b.sourceBgBump)
            && SameStaticLightingValue(a.sourceBgUV, b.sourceBgUV)
            && SameStaticLightingValue(a.sourceBgFlicker, b.sourceBgFlicker)
            && SameStaticLightingValue(a.sourceBgSubspecular, b.sourceBgSubspecular)
            && SameStaticLightingValue(a.sourceBgRimlight, b.sourceBgRimlight)
            && SameStaticLightingValue(a.sourceBgSpecularSaturation, b.sourceBgSpecularSaturation)
            && SameStaticLightingValue(a.sourceBgPanning, b.sourceBgPanning)
            && SameStaticLightingValue(a.sourceSpecial, b.sourceSpecial)
            && SameStaticLightingValue(a.sourceLandscape, b.sourceLandscape)
            && SameStaticLightingValue(a.sourceFoliageFlags, b.sourceFoliageFlags)
            && SameStaticLightingValue(a.sourceFoliageTransmission, b.sourceFoliageTransmission)
            && SameStaticLightingValue(a.sourceFoliageMaskSRGB, b.sourceFoliageMaskSRGB)
            && SameStaticLightingValue(a.sourceFoliageWind, b.sourceFoliageWind)
            && SameStaticLightingValue(a.sourceFoliageWindProgram, b.sourceFoliageWindProgram)
            && SameStaticLightingValue(a.sourceFoliageWindLocalCenter, b.sourceFoliageWindLocalCenter)
            && SameStaticLightingValue(a.sourceFoliageWindLocalBounds, b.sourceFoliageWindLocalBounds)
            && SameStaticLightingValue(a.sourceFoliageWindActorPosition, b.sourceFoliageWindActorPosition)
            && SameStaticLightingValue(a.sourceFoliageWindDirectionSpeed, b.sourceFoliageWindDirectionSpeed)
            && SameStaticLightingValue(a.sourceFoliageWindPlayerPosition, b.sourceFoliageWindPlayerPosition)
            && SameStaticLightingValue(a.sourceFoliageWindScalars, b.sourceFoliageWindScalars)
            && SameStaticLightingValue(a.overlayColor, b.overlayColor)
            && SameStaticLightingValue(a.overlayTiling, b.overlayTiling)
            && SameStaticLightingValue(a.overlayNormalIntensity, b.overlayNormalIntensity)
            && SameStaticLightingValue(a.overlaySharpness, b.overlaySharpness)
            && SameStaticLightingValue(a.overlayBrightness, b.overlayBrightness)
            && SameStaticLightingValue(a.overlaySaturation, b.overlaySaturation)
            && SameStaticLightingValue(a.overlaySpecularIntensity, b.overlaySpecularIntensity)
            && SameStaticLightingValue(a.overlaySRGB, b.overlaySRGB)
            && SameStaticLightingValue(a.overlaySeparateSpecular, b.overlaySeparateSpecular)
            && SameStaticLightingValue(a.sourceOverlayFlags, b.sourceOverlayFlags)
            && SameStaticLightingValue(a.sourceOverlayDirection, b.sourceOverlayDirection)
            && SameStaticLightingValue(a.diffuseBrightness, b.diffuseBrightness)
            && SameStaticLightingValue(a.normalIntensity, b.normalIntensity)
            && SameStaticLightingValue(a.specularIntensity, b.specularIntensity)
            && SameStaticLightingValue(a.specularPower, b.specularPower)
            && SameStaticLightingValue(a.reflectionIntensity, b.reflectionIntensity)
            && SameStaticLightingValue(a.reflectionContrast, b.reflectionContrast)
            && SameStaticLightingValue(a.reflectionTiling, b.reflectionTiling)
            && SameStaticLightingValue(a.diffuseSaturation, b.diffuseSaturation)
            && SameStaticLightingValue(a.diffuseColor, b.diffuseColor)
            && SameStaticLightingValue(a.specularColor, b.specularColor)
            && SameStaticLightingValue(a.reflectionColor, b.reflectionColor)
            && SameStaticLightingValue(a.diffuseSRGB, b.diffuseSRGB)
            && SameStaticLightingValue(a.specularSRGB, b.specularSRGB)
            && SameStaticLightingValue(a.reflectionSRGB, b.reflectionSRGB)
            && SameStaticLightingValue(a.ormSRGB, b.ormSRGB)
            && SameStaticLightingValue(a.castsShadow, b.castsShadow)
            && SameStaticLightingValue(a.hasEmissive, b.hasEmissive)
            && SameStaticLightingValue(a.emissiveColor, b.emissiveColor)
            && SameStaticLightingValue(a.emissiveIntensity, b.emissiveIntensity)
            && SameStaticLightingValue(a.emissiveUVTiling, b.emissiveUVTiling)
            && SameStaticLightingValue(a.emissiveFlickerMinimum, b.emissiveFlickerMinimum)
            && SameStaticLightingValue(a.emissiveFlickerSpeed, b.emissiveFlickerSpeed)
            && SameStaticLightingValue(a.emissivePhaseOffset, b.emissivePhaseOffset)
            && SameStaticLightingValue(a.emissiveSRGB, b.emissiveSRGB)
            && SameStaticLightingValue(a.uvTiling, b.uvTiling)
            && SameStaticLightingValue(a.uvFixedNormal, b.uvFixedNormal)
            && SameStaticLightingValue(a.useWorldReflection, b.useWorldReflection)
            && SameStaticLightingValue(a.detailNormalIntensity, b.detailNormalIntensity)
            && SameStaticLightingValue(a.detailNormalTiling, b.detailNormalTiling)
            && SameStaticLightingValue(a.metallicIntensity, b.metallicIntensity)
            && SameStaticLightingValue(a.metallicPower, b.metallicPower)
            && SameStaticLightingValue(a.roughnessIntensity, b.roughnessIntensity)
            && SameStaticLightingValue(a.roughnessPower, b.roughnessPower)
            && SameStaticLightingValue(a.aoIntensity, b.aoIntensity)
            && SameStaticLightingValue(a.aoPower, b.aoPower)
            && SameStaticLightingValue(a.specularPBRIntensity, b.specularPBRIntensity)
            && SameStaticLightingValue(a.nonmetallicBrightness, b.nonmetallicBrightness)
            && SameStaticLightingValue(a.metallicBrightness, b.metallicBrightness)
            && SameStaticLightingValue(a.minimumRoughness, b.minimumRoughness)
            && SameStaticLightingValue(a.reflectionOriginOffset, b.reflectionOriginOffset)
            && SameStaticLightingValue(a.vertexAlpha, b.vertexAlpha)
            && SameStaticLightingValue(a.hasBakedLighting, b.hasBakedLighting)
            && SameStaticLightingValue(a.bakedLightingSRGB, b.bakedLightingSRGB)
            && SameStaticLightingValue(a.hasStaticShadow, b.hasStaticShadow)
            && SameStaticLightingValue(a.staticShadowChannel, b.staticShadowChannel)
            && SameStaticLightingValue(a.staticShadowTransfer, b.staticShadowTransfer)
            && SameStaticLightingValue(a.hasEnvironmentCube, b.hasEnvironmentCube)
            && SameStaticLightingValue(a.environmentColor, b.environmentColor)
            && SameStaticLightingValue(a.environmentRotation, b.environmentRotation)
            && SameStaticLightingValue(a.environmentLegacyEnabled, b.environmentLegacyEnabled)
            && SameStaticLightingValue(a.hasSourceIndirect, b.hasSourceIndirect)
            && SameStaticLightingValue(a.sourceIndirectSH, b.sourceIndirectSH)
            && SameStaticLightingValue(a.sourceIndirectColor, b.sourceIndirectColor)
            && SameStaticLightingValue(a.sourceIndirectRotation, b.sourceIndirectRotation)
            && SameStaticLightingValue(a.sourceUpperSkyColor, b.sourceUpperSkyColor)
            && SameStaticLightingValue(a.sourceLowerSkyColor, b.sourceLowerSkyColor)
            && SameStaticLightingValue(a.sourceAmbientAndSkyFactor, b.sourceAmbientAndSkyFactor);
    }
}

bool_t CMaterial::Can_BatchStaticLightingWith(const CMaterial& other) const
{
    if (m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ||
        other.m_Surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ||
        !m_TextureOverrides.empty() || !other.m_TextureOverrides.empty() ||
        !m_SourceCharacterTextureOverrides.empty() || !other.m_SourceCharacterTextureOverrides.empty()) return false;
    const auto validLighting = [](const CMaterial& material)
    {
        return (!material.m_Surface.hasBakedLighting || (material.m_BakedAverage && material.m_BakedDirectional)) &&
            (!material.m_Surface.hasStaticShadow || material.m_StaticShadow);
    };
    if (!validLighting(*this) || !validLighting(other)) return false;
    if (this == &other) return true;
    if (m_pDevice != other.m_pDevice || m_pContext != other.m_pContext ||
        m_iDiffuseMirrorU != other.m_iDiffuseMirrorU ||
        !SameStaticLightingValue(m_Surface, other.m_Surface) ||
        !SameStaticLightingValue(m_ColorTint, other.m_ColorTint) ||
        !SameStaticLightingValue(m_vDiffuseTint, other.m_vDiffuseTint)) return false;
    // Preserve legacy slots as well as every surface input. Only the three
    // original baked-light/shadow SRVs are allowed to differ.
    for (size_t type = 0u; type < AI_TEXTURE_TYPE_MAX; ++type)
    {
        if (m_Textures[type].size() != other.m_Textures[type].size()) return false;
        for (size_t slot = 0u; slot < m_Textures[type].size(); ++slot)
            if (m_Textures[type][slot] != other.m_Textures[type][slot]) return false;
    }
    const auto textures = [](const CMaterial& material)
    {
        return std::array<ID3D11ShaderResourceView*, 20>{
            material.m_SurfaceDiffuse.Get(), material.m_SurfaceSpecular.Get(), material.m_SurfaceReflection.Get(),
            material.m_SurfaceNormal.Get(), material.m_SurfaceOverlayDiffuse.Get(), material.m_SurfaceOverlayNormal.Get(),
            material.m_SurfaceDetailNormal.Get(), material.m_SurfaceORM.Get(), material.m_SourceFoliageMask.Get(),
            material.m_SourceSpecialMask.Get(), material.m_SourceLandscapeHeight.Get(), material.m_SourceBlendDiffuseG.Get(),
            material.m_SourceBlendDiffuseB.Get(), material.m_SourceBlendNormalG.Get(), material.m_SourceBlendNormalB.Get(),
            material.m_SurfaceEmissive.Get(), material.m_EnvironmentCube.Get(), material.m_EnvironmentBRDF.Get(),
            material.m_SourceIndirectBRDF.Get(), material.m_SourceIndirectCube.Get()};
    };
    if (textures(*this) != textures(other) ||
        m_SourceCharacterTextures != other.m_SourceCharacterTextures ||
        m_SourceLandscapeDiffuse != other.m_SourceLandscapeDiffuse ||
        m_SourceLandscapeNormal != other.m_SourceLandscapeNormal ||
        m_SourceLandscapeWeights != other.m_SourceLandscapeWeights) return false;
    return true;
}

HRESULT CMaterial::Bind_StaticLightingBank(const shared_ptr<CShader>& shader,
    std::span<const CMaterial* const> materials) const
{
    if (!shader || materials.size() < 2u || materials.size() > 8u || materials.front() != this) return E_INVALIDARG;
    std::array<ID3D11ShaderResourceView*, 8> average{}, directional{}, shadow{};
    for (size_t i = 0u; i < materials.size(); ++i)
    {
        const auto* material = materials[i];
        if (!material || !Can_BatchStaticLightingWith(*material)) return E_INVALIDARG;
        average[i] = material->m_Surface.hasBakedLighting ? material->m_BakedAverage.Get() : nullptr;
        directional[i] = material->m_Surface.hasBakedLighting ? material->m_BakedDirectional.Get() : nullptr;
        shadow[i] = material->m_Surface.hasStaticShadow ? material->m_StaticShadow.Get() : nullptr;
    }
    const uint32_t disabled = 0u;
    if (FAILED(shader->Bind_RawValue("g_MapLightingBankSize", &disabled, sizeof(disabled)))) return E_FAIL;
    if (FAILED(shader->Bind_Textures("g_MapBakedAverageBank", average.data(), 8u)) ||
        FAILED(shader->Bind_Textures("g_MapBakedDirectionalBank", directional.data(), 8u)) ||
        FAILED(shader->Bind_Textures("g_MapStaticShadowBank", shadow.data(), 8u))) return E_FAIL;
    const uint32_t count = static_cast<uint32_t>(materials.size());
    const HRESULT result = shader->Bind_RawValue("g_MapLightingBankSize", &count, sizeof(count));
    if (FAILED(result)) (void)shader->Bind_RawValue("g_MapLightingBankSize", &disabled, sizeof(disabled));
    return result;
}

HRESULT CMaterial::Bind_SurfaceLighting(shared_ptr<CShader> shader)
{
    if (!shader) return E_INVALIDARG;
    if (FAILED(Bind_StaticShadow(shader))) return E_FAIL;
    if (m_Surface.hasBakedLighting &&
        (FAILED(shader->Bind_Texture("g_BakedAverageTexture", m_BakedAverage)) ||
         FAILED(shader->Bind_Texture("g_BakedDirectionalTexture", m_BakedDirectional)))) return E_FAIL;
    const bool sourceIndirect = m_Surface.hasSourceIndirect &&
        CGameInstance::Get().Get_RenderEnvironment().bUseSourcePBRIndirect;
    const auto& lookup = sourceIndirect ? m_SourceIndirectBRDF : m_EnvironmentBRDF;
    const auto& cube = sourceIndirect ? m_SourceIndirectCube : m_EnvironmentCube;
    if (m_Surface.hasEnvironmentCube && (m_Surface.environmentLegacyEnabled || sourceIndirect) &&
        (FAILED(shader->Bind_Texture("g_EnvironmentCubeTexture", cube)) ||
         FAILED(shader->Bind_Texture("g_EnvironmentBRDFLookupTexture", lookup)))) return E_FAIL;
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
	const ComPtr<ID3D11ShaderResourceView>* texture = nullptr;
	/* The surface program reads slot 0 of the same type, so one override covers both paths. */
	if (const auto found = m_TextureOverrides.find(Texture_OverrideKey(eType, 0u));
		found != m_TextureOverrides.end())
	{
		return pShader->Bind_Texture(pConstantName, found->second);
	}
	switch (eType)
	{
	case aiTextureType_DIFFUSE: texture = std::addressof(m_SurfaceDiffuse); break;
	case aiTextureType_SPECULAR: texture = std::addressof(m_SurfaceSpecular); break;
	case aiTextureType_REFLECTION: texture = std::addressof(m_SurfaceReflection); break;
	case aiTextureType_NORMALS: texture = std::addressof(m_SurfaceNormal); break;
	case aiTextureType_HEIGHT: texture = std::addressof(m_SurfaceDetailNormal); break;
	case aiTextureType_UNKNOWN: texture = std::addressof(m_SurfaceORM); break;
	case aiTextureType_EMISSIVE: texture = std::addressof(m_SurfaceEmissive); break;
    case aiTextureType_TRANSMISSION: texture = std::addressof(m_SourceFoliageMask); break;
    // Surface-only roles, separate from the legacy color-mask texture array.
    case aiTextureType_BASE_COLOR: texture = std::addressof(m_SurfaceOverlayDiffuse); break;
    case aiTextureType_NORMAL_CAMERA: texture = std::addressof(m_SurfaceOverlayNormal); break;
	default: return E_INVALIDARG;
	}
	// The material owns the SRV throughout this call; borrowing avoids a
	// redundant COM AddRef/Release for every surface texture of every draw.
	// ComPtr::operator& resets the owner through ComPtrRef; borrow its C++
	// object address with std::addressof so the material keeps its SRV.
	return texture && *texture ? pShader->Bind_Texture(pConstantName, *texture) : E_FAIL;
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
