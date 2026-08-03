#include "WMaterialReader.h"

#include "BinaryAsset/BinaryReader.h"
#include "WFormatTypes.h"

#include <cstring>

using namespace Engine::WintersFormat;

namespace
{
	bool_t IsBelowRoot(
		const filesystem::path& root,
		const filesystem::path& candidate)
	{
		if (root.empty() || candidate.empty())
			return false;
		const filesystem::path relative = candidate.lexically_normal().lexically_relative(
			root.lexically_normal());
		if (relative.empty() || relative.is_absolute())
			return false;
		const auto first = relative.begin();
		return first != relative.end() && *first != L"..";
	}

	bool_t HasMagic(const void* pValue, const char* pMagic)
	{
		return 0 == memcmp(pValue, pMagic, 4);
	}

	filesystem::path ResolveBelowAssetRoot(const MODEL_ASSET_LOAD_DESC& desc,
		const filesystem::path& storedPath)
	{
		if (desc.assetRoot.empty())
			return {};

		const wstring genericPath = storedPath.generic_wstring();
		constexpr const wchar_t* resourcePrefixes[] =
		{
			L"Resource/LostArk/",
			L"Resource/",
			L"Resources/"
		};

		for (const wchar_t* pPrefix : resourcePrefixes)
		{
			const wstring prefix = pPrefix;
			const size_t prefixPosition = genericPath.find(prefix);
			if (wstring::npos == prefixPosition)
				continue;

			const filesystem::path relativeToAssetRoot =
				genericPath.substr(prefixPosition + prefix.size());
			const filesystem::path candidate =
				(desc.assetRoot / relativeToAssetRoot).lexically_normal();
			if (IsBelowRoot(desc.assetRoot, candidate) &&
				filesystem::exists(candidate))
				return candidate;
		}

		return {};
	}

	filesystem::path ResolveTexturePath(const MODEL_ASSET_LOAD_DESC& desc,
		const filesystem::path& materialPath,
		const wchar_t* pValue)
	{
		size_t length = {};
		while (length < 260 && L'\0' != pValue[length])
			++length;
		if (0 == length)
			return {};

		const filesystem::path storedPath(wstring(pValue, length));
		if (storedPath.is_absolute())
		{
			return ResolveBelowAssetRoot(desc, storedPath);
		}

		const filesystem::path besideMaterial =
			(materialPath.parent_path() / storedPath).lexically_normal();
		if ((desc.assetRoot.empty() || IsBelowRoot(desc.assetRoot, besideMaterial)) &&
			filesystem::exists(besideMaterial))
			return besideMaterial;

		if (!desc.assetRoot.empty())
		{
			const filesystem::path belowAssetRoot =
				(desc.assetRoot / storedPath).lexically_normal();
			if (IsBelowRoot(desc.assetRoot, belowAssetRoot) &&
				filesystem::exists(belowAssetRoot))
				return belowAssetRoot;


			const filesystem::path relocatedPath = ResolveBelowAssetRoot(desc, storedPath);
			if (!relocatedPath.empty())
				return relocatedPath;
		}

		return desc.assetRoot.empty() || IsBelowRoot(desc.assetRoot, besideMaterial) ?
			besideMaterial : filesystem::path{};
	}
}

bool_t CWMaterialReader::Read(const MODEL_ASSET_LOAD_DESC& desc,
	uint32_t minimumCount,
	vector<MODEL_MATERIAL_DATA>& outMaterials,
	MODEL_DECODE_REPORT& outReport) const
{
	filesystem::path materialPath = desc.materialPath;
	if (materialPath.empty())
	{
		materialPath = desc.meshPath;
		materialPath.replace_extension(L".wmat");
	}

	vector<uint8_t> bytes;
	if (CBinaryReader::LoadFile(materialPath, bytes))
		return ReadMemory(bytes.data(), bytes.size(), desc, materialPath,
			minimumCount, outMaterials, outReport);

	outMaterials.clear();
	outMaterials.resize(minimumCount);
	for (MODEL_MATERIAL_DATA& material : outMaterials)
	{
		if (material.diffusePath.empty() && !desc.fallbackDiffusePath.empty())
			material.diffusePath = desc.fallbackDiffusePath;
	}
	return true;
}

bool_t CWMaterialReader::ReadMemory(const uint8_t* pData,
	size_t dataSize,
	const MODEL_ASSET_LOAD_DESC& desc,
	const filesystem::path& containerPath,
	uint32_t minimumCount,
	vector<MODEL_MATERIAL_DATA>& outMaterials,
	MODEL_DECODE_REPORT& outReport) const
{
	outMaterials.clear();
	outMaterials.resize(minimumCount);
	if (nullptr == pData || dataSize < sizeof(FILE_HEADER))
	{
		outReport.error = "The embedded WMAT section is empty or truncated.";
		return false;
	}

	try
	{
			CBinaryReader fileReader(pData, dataSize);
			const FILE_HEADER fileHeader = fileReader.Read<FILE_HEADER>();
			if (!HasMagic(fileHeader.magic, WINTERS_MAGIC) ||
				1 != fileHeader.versionMajor || 0 != fileHeader.flags ||
				fileHeader.contentSize > fileReader.Remaining())
			{
				outReport.error = "Invalid WINT material file header.";
				return false;
			}

			CBinaryReader reader(fileReader.Peek(), fileHeader.contentSize);
			const MATERIAL_META_HEADER materialHeader = reader.Read<MATERIAL_META_HEADER>();
			const bool_t isV2 = HasMagic(materialHeader.magic, WMAT_V2_MAGIC);
			if ((!HasMagic(materialHeader.magic, WMAT_MAGIC) && !isV2) ||
				materialHeader.materialCount > MAX_MATERIALS)
			{
				outReport.error = "Invalid WMAT metadata.";
				return false;
			}

			outMaterials.resize((max)(outMaterials.size(), static_cast<size_t>(materialHeader.materialCount)));
			for (uint32_t i = 0; i < materialHeader.materialCount; ++i)
			{
				if (isV2)
				{
					const MATERIAL_ENTRY_V2 entry = reader.Read<MATERIAL_ENTRY_V2>();
					if (entry.materialIndex >= outMaterials.size())
					{
						outReport.error = "WMA2 contains an out-of-range material index.";
						return false;
					}

					MODEL_MATERIAL_DATA& material = outMaterials[entry.materialIndex];
					material.diffusePath = ResolveTexturePath(desc, containerPath, entry.baseColorPath);
					material.normalPath = ResolveTexturePath(desc, containerPath, entry.normalPath);
					material.specularPath = ResolveTexturePath(desc, containerPath, entry.specularPath);
					material.emissivePath = ResolveTexturePath(desc, containerPath, entry.emissivePath);
					material.opacityPath = ResolveTexturePath(desc, containerPath, entry.opacityPath);
					material.ormPath = ResolveTexturePath(desc, containerPath, entry.ormPath);
					material.metallicPath = ResolveTexturePath(desc, containerPath, entry.metallicPath);
					material.roughnessPath = ResolveTexturePath(desc, containerPath, entry.roughnessPath);
					material.ambientOcclusionPath = ResolveTexturePath(
						desc, containerPath, entry.ambientOcclusionPath);
				}
				else
				{
					const MATERIAL_ENTRY entry = reader.Read<MATERIAL_ENTRY>();
					if (entry.materialIndex >= outMaterials.size())
					{
						outReport.error = "WMAT contains an out-of-range material index.";
						return false;
					}
					outMaterials[entry.materialIndex].diffusePath =
						ResolveTexturePath(desc, containerPath, entry.diffusePath);
				}
			}
	}
	catch (const exception& exception)
	{
		outReport.error = exception.what();
		return false;
	}

	for (MODEL_MATERIAL_DATA& material : outMaterials)
	{
		if (material.diffusePath.empty() && !desc.fallbackDiffusePath.empty())
			material.diffusePath = desc.fallbackDiffusePath;
	}

	return true;
}
