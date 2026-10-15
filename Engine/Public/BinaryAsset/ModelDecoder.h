#pragma once

// 경로·로드 설정과 CPU 해석 결과 및 진단 report를 구현체 사이의 동일한 입력/출력 형식으로 사용한다.
#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

// 파일 식별과 CPU 모델 해석의 공통 경계다. GPU 자원 생성과 material override 적용은 CModel이 담당한다.
class IModelDecoder
{
public:
	virtual ~IModelDecoder() = default;

	// 이름은 진단용 문자열이고 CanDecode는 후보 선택용 probe다. true가 전체 payload 유효성을 보장하지 않는다.
	virtual const char_t* Get_Name() const = 0;
	virtual bool_t CanDecode(const filesystem::path& meshPath) const = 0;
	// 동시에 호출될 수 있으므로 구현체의 mutable 해석 상태는 각 호출이 소유해야 한다.
	virtual bool_t Decode(
		const MODEL_ASSET_LOAD_DESC& desc,
		MODEL_ASSET_DATA& outAsset,
		MODEL_DECODE_REPORT& outReport) const = 0;
};

NS_END
