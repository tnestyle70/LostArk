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
};

NS_END
