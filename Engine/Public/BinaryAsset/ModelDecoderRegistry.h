#pragma once

#include "BinaryAsset/ModelDecoder.h"

#include <mutex>

NS_BEGIN(Engine)

class ENGINE_DLL CModelDecoderRegistry final
{
public:
	static CModelDecoderRegistry& Get();

	void Register(unique_ptr<IModelDecoder> pDecoder);
	bool_t Decode(const MODEL_ASSET_LOAD_DESC& desc, MODEL_ASSET_DATA& outAsset);
	// The last decode on the calling thread; other loader jobs cannot replace it.
	MODEL_DECODE_REPORT Get_LastReport() const;

private:
	CModelDecoderRegistry();
	~CModelDecoderRegistry() = default;

	CModelDecoderRegistry(const CModelDecoderRegistry&) = delete;
	CModelDecoderRegistry& operator=(const CModelDecoderRegistry&) = delete;

private:
	mutable mutex m_Mutex;
	vector<unique_ptr<IModelDecoder>> m_Decoders;
};

NS_END
