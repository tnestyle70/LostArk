#pragma once

// 로드 경로·기본 texture 입력과 재질 슬롯의 소유 결과, 해석 진단을 선언한다.
#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

// WMAT/WMA2/WMA3를 재질 슬롯과 texture 경로로 해석한다. GPU texture 로드와 material override 검증은 CMaterial/CModel이 담당한다.
class CWMaterialReader final
{
public:
	// 분리 .wmat을 읽지 못하면 minimumCount만큼 기본 재질과 fallbackDiffusePath를 만들고 true를 반환한다.
	bool_t Read(
		const MODEL_ASSET_LOAD_DESC& desc,
		uint32_t minimumCount,
		vector<MODEL_MATERIAL_DATA>& outMaterials,
		MODEL_DECODE_REPORT& outReport) const;

	// embedded section은 필수 입력이다. 출력을 먼저 재구성하고 직접 채우므로 false/예외 catch 뒤에도 부분 재질이 남을 수 있다.
	// containerPath는 상대 texture의 기준이며 바이트 포인터는 호출 동안만 차용한다.
	bool_t ReadMemory(
		const uint8_t* pData,
		size_t dataSize,
		const MODEL_ASSET_LOAD_DESC& desc,
		const filesystem::path& containerPath,
		uint32_t minimumCount,
		vector<MODEL_MATERIAL_DATA>& outMaterials,
		MODEL_DECODE_REPORT& outReport) const;
};

NS_END
