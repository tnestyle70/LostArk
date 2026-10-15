#pragma once

// Client 이름 공간과 공통 선언을 사용한다.
#include "Client_Defines.h"
// 재생 상태의 bool_t 타입을 사용한다.
#include "Engine_Defines.h"

// 대상 generation과 밀리초 정수 값을 사용한다.
#include <cstdint>
// asset ID·clip 이름·오류를 값으로 반환한다.
#include <string>

NS_BEGIN(Client)

// 현재 저작 대상의 재생 위치 사본이다. Model을 소유하지 않으며 시간은 밀리초다.
struct ANIMATION_PLAYHEAD_SNAPSHOT final
{
	uint64_t targetGeneration = {};
	std::string animationAssetId;
	std::string clipName;
	int32_t timeMs = {};
	int32_t durationMs = {};
	bool_t isPaused = false;
};

// 선택된 animation target을 읽어 도구가 소비할 값 snapshot으로 바꾸는 정적 조회 경계다.
class CAnimationAuthoringBridge final
{
public:
	// 출력을 먼저 비운다. 대상·clip·진행 조회 실패는 false와 오류, 성공은 generation·시간·pause 값이다.
	static bool_t Try_GetPlayheadSnapshot(
		ANIMATION_PLAYHEAD_SNAPSHOT& outSnapshot,
		std::string& outError);

private:
	// 객체별 상태가 없어 인스턴스 생성을 막고 정적 함수만 제공한다.
	CAnimationAuthoringBridge() = delete;
};

NS_END
