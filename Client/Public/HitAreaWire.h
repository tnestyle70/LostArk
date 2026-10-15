#pragma once

// Client 이름 공간과 도형 종류·색의 정수 타입을 사용한다.
#include "Client_Defines.h"
// 루트 행렬과 거리·각도 계산의 f32_t 등 엔진 공통 타입을 사용한다.
#include "Engine_Defines.h"

NS_BEGIN(Client)

// 공격·저작 범위를 그릴 값 묶음이다. 정수 거리 값은 cm, *M 값은 m, 각도는 degree로 읽는다.
struct HIT_AREA_SHAPE
{
	int32_t iAreaType = {};
	int32_t iAreaRange = {};
	// BOX에서는 폭 cm, 부채꼴에서는 벌어진 각도 degree다.
	int32_t iAreaAngle = {};
	int32_t iAreaHeight = {};
	int32_t iAreaOffsetX = {};
	int32_t iAreaInner = {};
	// 양수 높이를 주면 평면 BOX 대신 입체 wire box를 표시한다.
	f32_t fBoxHalfHeightM = {};
	f32_t fCylinderHalfHeightM = {};
	// 양수 X/Z 반지름은 저작 타원 부채꼴이다. 0이면 기존 정수 범위 경로를 사용한다.
	f32_t fSectorRadiusXM = {}, fSectorRadiusZM = {}, fSectorAngleDegrees = {};
	bool_t bReverseSector = false;
};

// Character·Npc·AnimationTool·보스 연출이 공유하는 정적 범위선 그리기다. 충돌이나 피해를 판정하지 않는다.
class CHitAreaWire final
{
public:
	// 월드 루트와 도형을 현재 카메라로 투영해 ImGui 배경 draw list에 선을 제출한다.
	static void Draw(const float4x4_t& Root, const HIT_AREA_SHAPE& Shape, uint32_t iColorRgba);
};

NS_END
