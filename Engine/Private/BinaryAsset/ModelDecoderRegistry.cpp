#include "BinaryAsset/ModelDecoderRegistry.h"
#include "BinaryAsset/WModelDecoder.h"

CModelDecoderRegistry& CModelDecoderRegistry::Get()
{
	static CModelDecoderRegistry instance;
	return instance;
}

CModelDecoderRegistry::CModelDecoderRegistry()
{
	Register(make_unique<CWModelDecoder>());
}

void CModelDecoderRegistry::Register(unique_ptr<IModelDecoder> pDecoder)
{
	if (nullptr == pDecoder)
		return;

	lock_guard<mutex> lock(m_Mutex);
	m_Decoders.push_back(move(pDecoder));
}

bool_t CModelDecoderRegistry::Decode(const MODEL_ASSET_LOAD_DESC& desc, MODEL_ASSET_DATA& outAsset)
{
	lock_guard<mutex> lock(m_Mutex);

	outAsset = {};
	m_LastReport = {};
	m_LastReport.meshPath = desc.meshPath;

	if (desc.meshPath.empty())
	{
		m_LastReport.error = "Mesh path is empty.";
		return false;
	}

	for (const auto& pDecoder : m_Decoders)
	{
		if (nullptr == pDecoder || !pDecoder->CanDecode(desc.meshPath))
			continue;

		m_LastReport.decoderName = pDecoder->Get_Name();
		m_LastReport.succeeded = pDecoder->Decode(desc, outAsset, m_LastReport);
		if (!m_LastReport.succeeded && m_LastReport.error.empty())
			m_LastReport.error = "Decoder rejected the binary payload.";
		return m_LastReport.succeeded;
	}

	m_LastReport.error = "No decoder recognized the binary header. The extension is not used as the contract.";
	return false;
}

MODEL_DECODE_REPORT CModelDecoderRegistry::Get_LastReport() const
{
	lock_guard<mutex> lock(m_Mutex);
	return m_LastReport;
}
