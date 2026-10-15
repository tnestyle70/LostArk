#pragma once

// 본·socket의 소유 배열과 해석 진단 형식을 반환 계약으로 사용한다.
#include "BinaryAsset/ModelAssetData.h"

NS_BEGIN(Engine)

// WSKL의 parent-before-child 골격과 socket을 CPU 소유 데이터로 만든다. inverse-bind는 이후 mesh와의 결합에서 채운다.
class CWSkeletonReader final
{
public:
	// 파일 읽기 실패는 기존 출력을 유지한다. ReadMemory로 넘어간 뒤에는 출력 초기화/부분 실패 규칙이 적용된다.
	bool_t Read(
		const filesystem::path& skeletonPath,
		MODEL_SKELETON_DATA& outSkeleton,
		MODEL_DECODE_REPORT& outReport) const;

	// 원본 바이트를 호출 동안만 빌린다. 출력을 먼저 비우며 명시적 false에는 부분 결과, try 안 예외에는 빈 결과가 남는다.
	bool_t ReadMemory(
		const uint8_t* pData,
		size_t dataSize,
		MODEL_SKELETON_DATA& outSkeleton,
		MODEL_DECODE_REPORT& outReport) const;
};

NS_END
