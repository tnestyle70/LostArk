#pragma once

// 호출 중 차용할 골격과 소유할 clip·채널·키, 실패 진단의 CPU 계약을 선언한다.
#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

// WANM의 채널/키를 소유 vector로 복사한다. skeleton은 호출 동안 읽기 전용으로 빌리며 저장하지 않는다.
class CWAnimationReader final
{
public:
	// 파일 열기 실패는 outAnimation을 유지한다. 파일을 읽은 뒤의 상태 변경 계약은 ReadMemory를 따른다.
	bool_t Read(
		const filesystem::path& animationPath,
		const MODEL_SKELETON_DATA& skeleton,
		MODEL_ANIMATION_DATA& outAnimation,
		MODEL_DECODE_REPORT& outReport) const;

	// 입력 바이트는 호출 동안만 차용한다. 출력을 먼저 비우고 채널을 누적하므로 명시적 false는 부분 결과가 남을 수 있다.
	// try 안의 표준 예외는 출력을 다시 비운다. 바깥 WModelDecoder가 이 결과를 임시 asset에 보관한다.
	bool_t ReadMemory(
		const uint8_t* pData,
		size_t dataSize,
		const string& animationName,
		const MODEL_SKELETON_DATA& skeleton,
		MODEL_ANIMATION_DATA& outAnimation,
		MODEL_DECODE_REPORT& outReport) const;
};

NS_END
