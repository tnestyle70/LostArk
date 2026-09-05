#pragma once

#include "BinaryAsset/ModelDecoder.h"

NS_BEGIN(Engine)

class CWModelDecoder final : public IModelDecoder
{
public:
	// Reads package names and native timing without decoding model payloads.
	// Failure preserves outEntries; success does not validate playable geometry.
	static ENGINE_DLL bool_t Read_AnimationCatalog(const filesystem::path& modelPath,
		vector<MODEL_ANIMATION_CATALOG_ENTRY>& outEntries, string& outStatus);

	virtual const char_t* Get_Name() const override;
	virtual bool_t CanDecode(const filesystem::path& meshPath) const override;
	virtual bool_t Decode(const MODEL_ASSET_LOAD_DESC& desc,
		MODEL_ASSET_DATA& outAsset,
		MODEL_DECODE_REPORT& outReport) const override;
};

NS_END
