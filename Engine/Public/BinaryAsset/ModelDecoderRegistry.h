#pragma once

// decoder를 공통 인터페이스의 unique_ptr로 소유하고 같은 로드/결과 형식으로 호출한다.
#include "BinaryAsset/ModelDecoder.h"

// 등록 목록 추가와 차용 포인터 snapshot 준비를 같은 mutex로 보호한다. 실제 decode 작업은 잠금 밖에서 수행한다.
#include <mutex>

NS_BEGIN(Engine)

// 프로세스의 decoder 소유 목록이다. CModel/도구는 이 경계를 통해 동일한 CPU 포맷 해석을 사용한다.
class ENGINE_DLL CModelDecoderRegistry final
{
public:
	// CModel과 도구가 프로세스 안의 동일한 registry를 차용한다. 최초 호출이 기본 WModel decoder를 등록한다.
	static CModelDecoderRegistry& Get();

	// 소유권을 목록으로 옮겨 뒤에 추가한다. nullptr는 무시하며 등록 해제 API는 없다.
	void Register(unique_ptr<IModelDecoder> pDecoder);
	// 호출 스레드의 report를 갱신한다. 처음 CanDecode가 맞는 decoder의 결과로 종료하며 실패 후 다음 후보로 재시도하지 않는다.
	// 출력 asset은 먼저 초기화되므로 이전 모델을 보존하려면 호출자가 별도 임시 asset을 사용해야 한다.
	// 목록 snapshot 준비가 끝난 뒤 asset을 비운다. 예외로 중단되면 마지막 thread-local report를 갱신하지 못할 수 있다.
	bool_t Decode(const MODEL_ASSET_LOAD_DESC& desc, MODEL_ASSET_DATA& outAsset);
	// 호출 스레드의 마지막 decode 보고서다. 다른 loader job의 보고서가 이 값을 덮지 않는다.
	MODEL_DECODE_REPORT Get_LastReport() const;

private:
	CModelDecoderRegistry();
	~CModelDecoderRegistry() = default;

	CModelDecoderRegistry(const CModelDecoderRegistry&) = delete;
	CModelDecoderRegistry& operator=(const CModelDecoderRegistry&) = delete;

private:
	// lock은 목록의 추가/포인터 복사만 보호한다. 실제 decoder 호출과 I/O는 lock 밖이며 구현체가 호출별 상태를 소유해야 한다.
	mutable mutex m_Mutex;
	vector<unique_ptr<IModelDecoder>> m_Decoders;
};

NS_END
