#pragma once

#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

class IModelDecoder
{
public:
	virtual ~IModelDecoder() = default;

	virtual const char_t* Get_Name() const = 0;
	virtual bool_t CanDecode(const filesystem::path& meshPath) const = 0;
	virtual bool_t Decode(const MODEL_ASSET_LOAD_DESC& desc,
		MODEL_ASSET_DATA& outAsset,
		MODEL_DECODE_REPORT& outReport) const = 0;
};

NS_END
