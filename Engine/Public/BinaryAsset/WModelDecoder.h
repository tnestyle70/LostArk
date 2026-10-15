#pragma once

// registry가 사용하는 포맷 선택/해석 인터페이스와 공통 CPU 결과 계약을 구현한다.
#include "BinaryAsset/ModelDecoder.h"

NS_BEGIN(Engine)

// Winters 컨테이너 WMOD와 분리된 WMSH companion 파일을 MODEL_ASSET_DATA로 변환한다. 파일 확장자만으로 식별하지 않는다.
class CWModelDecoder final : public IModelDecoder
{
public:
	// 모델 payload 전체를 해석하지 않고 package의 clip 이름과 native 시간을 읽는다.
	// 실패는 outEntries를 보존하며 성공도 geometry의 재생 가능성을 보장하지 않는다.
	static ENGINE_DLL bool_t Read_AnimationCatalog(
		const filesystem::path& modelPath,
		vector<MODEL_ANIMATION_CATALOG_ENTRY>& outEntries,
		string& outStatus);

	// report가 사용하는 정적 포맷 이름을 차용한다. 반환 문자열의 소유권을 넘기지 않는다.
	virtual const char_t* Get_Name() const override;
	// registry의 후보 선택용 20-byte probe다. 성공 뒤에도 Decode의 전체 입력 검증이 필요하다.
	virtual bool_t CanDecode(const filesystem::path& meshPath) const override;
	// 출력을 먼저 비우고 지역 stagedAsset을 완성한 뒤 move한다. 명시적 false에서는 기존 asset 대신 빈 asset이 남는다.
	// report는 호출자가 준비한 진단 객체를 누적 갱신하며, 모든 파일/할당 예외를 이 함수 전체에서 catch하지는 않는다.
	virtual bool_t Decode(
		const MODEL_ASSET_LOAD_DESC& desc,
		MODEL_ASSET_DATA& outAsset,
		MODEL_DECODE_REPORT& outReport) const override;
};

NS_END
