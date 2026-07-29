#include "WMaterialReader.h"

#include "BinaryAsset/BinaryReader.h"
#include "WFormatTypes.h"

#include <cstring>

using namespace Engine::WintersFormat;

namespace
{
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
		constexpr const wchar_t* lostArkPrefixes[] =
		{
			L"Resource/LostArk/",
			L"Resources/LostArk/"
		};

		for (const wchar_t* pPrefix : lostArkPrefixes)
		{
			const wstring prefix = pPrefix;
			const size_t prefixPosition = genericPath.find(prefix);
			if (wstring::npos == prefixPosition)
				continue;

			const filesystem::path relativeToLostArk =
				genericPath.substr(prefixPosition + prefix.size());
			const filesystem::path candidate =
				(desc.assetRoot / relativeToLostArk).lexically_normal();
			if (filesystem::exists(candidate))
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
			const filesystem::path normalizedPath = storedPath.lexically_normal();
			if (filesystem::exists(normalizedPath))
				return normalizedPath;

			const filesystem::path relocatedPath = ResolveBelowAssetRoot(desc, storedPath);
			return relocatedPath.empty() ? normalizedPath : relocatedPath;
		}

		const filesystem::path besideMaterial =
			(materialPath.parent_path() / storedPath).lexically_normal();
		if (filesystem::exists(besideMaterial))
			return besideMaterial;

		if (!desc.assetRoot.empty())
		{
			const filesystem::path belowAssetRoot =
				(desc.assetRoot / storedPath).lexically_normal();
			if (filesystem::exists(belowAssetRoot))
				return belowAssetRoot;


			const filesystem::path relocatedPath = ResolveBelowAssetRoot(desc, storedPath);
			if (!relocatedPath.empty())
				return relocatedPath;
		}

		return besideMaterial;
	}
}

bool_t CWMaterialReader::Read(const MODEL_ASSET_LOAD_DESC& desc,
	uint32_t minimumCount,
	vector<MODEL_MATERIAL_DATA>& outMaterials,
	MODEL_DECODE_REPORT& outReport) const
{
	outMaterials.clear();
	outMaterials.resize(minimumCount);

	filesystem::path materialPath = desc.materialPath;
	if (materialPath.empty())
	{
		materialPath = desc.meshPath;
		materialPath.replace_extension(L".wmat");
	}

	vector<uint8_t> bytes;
	if (CBinaryReader::LoadFile(materialPath, bytes))
	{
		try
		{
			CBinaryReader fileReader(bytes.data(), bytes.size());
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
			if (!HasMagic(materialHeader.magic, WMAT_MAGIC) || materialHeader.materialCount > MAX_MATERIALS)
			{
				outReport.error = "Invalid WMAT metadata.";
				return false;
			}

			outMaterials.resize((max)(outMaterials.size(), static_cast<size_t>(materialHeader.materialCount)));
			for (uint32_t i = 0; i < materialHeader.materialCount; ++i)
			{
				const MATERIAL_ENTRY entry = reader.Read<MATERIAL_ENTRY>();
				if (entry.materialIndex >= outMaterials.size())
				{
					outReport.error = "WMAT contains an out-of-range material index.";
					return false;
				}
				outMaterials[entry.materialIndex].diffusePath = ResolveTexturePath(desc, materialPath, entry.diffusePath);
			}
		}
		catch (const exception& exception)
		{
			outReport.error = exception.what();
			return false;
		}
	}

	for (MODEL_MATERIAL_DATA& material : outMaterials)
	{
		if (material.diffusePath.empty() && !desc.fallbackDiffusePath.empty())
			material.diffusePath = desc.fallbackDiffusePath;
	}

	return true;
}
