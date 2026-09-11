#include "BinaryAsset/ModelDecoderRegistry.h"
#include "BinaryAsset/WModelDecoder.h"

namespace
{
	MODEL_DECODE_REPORT& ThreadDecodeReport()
	{
		// Function-local TLS is initialized only on threads that actually decode.
		static thread_local MODEL_DECODE_REPORT report;
		return report;
	}
}

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
	vector<const IModelDecoder*> decoders;
	{
		lock_guard<mutex> lock(m_Mutex);
		decoders.reserve(m_Decoders.size());
		for (const auto& decoder : m_Decoders)
			decoders.push_back(decoder.get());
	}
	// Register only appends owning pointers, so the pointees remain stable.
	// Disk I/O and CPU decoding must not hold the registry lock.
	outAsset = {};
	MODEL_DECODE_REPORT report;
	report.meshPath = desc.meshPath;
	if (desc.meshPath.empty())
		report.error = "Mesh path is empty.";
	else
	{
		for (const auto* decoder : decoders)
		{
			if (nullptr == decoder || !decoder->CanDecode(desc.meshPath))
				continue;
			report.decoderName = decoder->Get_Name();
			report.succeeded = decoder->Decode(desc, outAsset, report);
			if (!report.succeeded && report.error.empty())
				report.error = "Decoder rejected the binary payload.";
			const bool_t succeeded = report.succeeded;
			ThreadDecodeReport() = move(report);
			return succeeded;
		}
		report.error = "No decoder recognized the binary header. The extension is not used as the contract.";
	}
	ThreadDecodeReport() = move(report);
	return false;
}

MODEL_DECODE_REPORT CModelDecoderRegistry::Get_LastReport() const
{
	return ThreadDecodeReport();
}
