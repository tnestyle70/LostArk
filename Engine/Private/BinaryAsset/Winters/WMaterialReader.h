#pragma once

#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

class CWMaterialReader final
{
public:
	bool_t Read(const MODEL_ASSET_LOAD_DESC& desc,
		uint32_t minimumCount,
		vector<MODEL_MATERIAL_DATA>& outMaterials,
		MODEL_DECODE_REPORT& outReport) const;
	bool_t ReadMemory(const uint8_t* pData,
		size_t dataSize,
		const MODEL_ASSET_LOAD_DESC& desc,
		const filesystem::path& containerPath,
		uint32_t minimumCount,
		vector<MODEL_MATERIAL_DATA>& outMaterials,
		MODEL_DECODE_REPORT& outReport) const;
};

NS_END
