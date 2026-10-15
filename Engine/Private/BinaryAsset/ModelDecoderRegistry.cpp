// decoder 목록의 소유권과 호출별 CPU 해석·진단 전달을 구현한다.
#include "BinaryAsset/ModelDecoderRegistry.h"
// registry 최초 생성 시 기본 구현을 등록한다. 다른 포맷을 확장할 때도 같은 IModelDecoder 경계를 사용한다.
#include "BinaryAsset/WModelDecoder.h"

namespace
{
	// Decode가 정상 반환 경로에서 기록하고 Get_LastReport가 값으로 복사할 보고서를 현재 스레드에 보관한다.
	MODEL_DECODE_REPORT& ThreadDecodeReport()
	{
		// 실제로 decode/report 조회를 수행하는 스레드에서만 지역 TLS를 초기화한다.
		static thread_local MODEL_DECODE_REPORT report;
		return report;
	}
}

// 첫 사용 시 생성하는 registry다. 종료 중 decode 작업과 singleton 파괴의 조율은 이 클래스가 제공하지 않는다.
CModelDecoderRegistry& CModelDecoderRegistry::Get()
{
	static CModelDecoderRegistry instance;
	return instance;
}

// 기본 decoder의 소유권을 등록 목록으로 넘긴다. 이후 Register 호출은 이 순서 뒤에 후보를 추가한다.
CModelDecoderRegistry::CModelDecoderRegistry()
{
	Register(make_unique<CWModelDecoder>());
}

// 등록 후 pointee 자체는 이동시키지 않는다. vector 재할당으로 unique_ptr 슬롯이 옮겨져도 snapshot의 decoder 주소는 유지된다.
void CModelDecoderRegistry::Register(unique_ptr<IModelDecoder> pDecoder)
{
	if (nullptr == pDecoder)
		return;

	lock_guard<mutex> lock(m_Mutex);
	m_Decoders.push_back(move(pDecoder));
}

// 목록의 차용 포인터 snapshot을 만든 뒤 lock을 놓아 독립 로드 작업이 디스크 I/O 때문에 서로 막히지 않게 한다.
bool_t CModelDecoderRegistry::Decode(const MODEL_ASSET_LOAD_DESC& desc, MODEL_ASSET_DATA& outAsset)
{
	vector<const IModelDecoder*> decoders;
	{
		lock_guard<mutex> lock(m_Mutex);
		decoders.reserve(m_Decoders.size());
		for (const auto& decoder : m_Decoders)
			decoders.push_back(decoder.get());
	}

	// Register는 소유 포인터를 뒤에 추가하므로 가리키는 decoder 주소는 유지된다.
	// 디스크 I/O와 CPU 해석 동안 registry lock을 잡지 않는다.
	outAsset = {};
	MODEL_DECODE_REPORT report;
	report.meshPath = desc.meshPath;
	if (desc.meshPath.empty())
		report.error = "Mesh path is empty.";
	else
	{
		// 인식한 첫 decoder만 실행한다. 실패 시 report는 남지만 예외를 registry에서 포괄적으로 catch하지는 않는다.
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

// 같은 스레드의 마지막 보고서를 값으로 복사한다. 호출 전에는 기본값이며 다른 loader 스레드의 결과를 조회하는 API가 아니다.
MODEL_DECODE_REPORT CModelDecoderRegistry::Get_LastReport() const
{
	return ThreadDecodeReport();
}
