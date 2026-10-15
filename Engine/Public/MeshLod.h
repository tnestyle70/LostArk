#pragma once

// 뷰 공간 구·투영 픽셀 계수·설정값에 사용할 공통 수학 타입을 가져온다.
#include "Engine_Defines.h"

NS_BEGIN(Engine)

// 제출할 모든 인스턴스를 포함하는 보수적인 뷰 공간 범위다. geometry 상세 단계만 선택하며
// 표시 여부와 light-volume shadow pass 선택은 호출자가 계속 담당한다.
struct MESH_SCREEN_LOD_DESC final
{
    float4_t viewBounds = {}; // 뷰 공간 중심 XYZ와 반지름
    float2_t projectionPixels = {}; // 투영 대각 성분의 절댓값 × viewport 픽셀 크기 / 2
    // 로컬 단순화 오차를 월드 길이로 환산할 인스턴스 최대 배율이다.
    f32_t maximumScale = 0.f;
    // 뷰 공간 깊이의 근평면과 허용 화면 오차(픽셀)다. 실제 선택은 Select_Range가 입력 유효성과 여유 폭을 함께 확인한다.
    f32_t nearPlane = 0.f;
    f32_t maximumPixelError = 0.25f;
    // 보이는 각 인스턴스의 뷰 공간 구를 합쳐 만든 선택적 경계다.
    // 전체를 감싸는 단일 구와 달리 가로 방향의 배치 간격이 최소 깊이를 부풀리지 않는다.
    float2_t maximumAbsViewXY = {};
    f32_t minimumViewDepth = 0.f;
    bool_t hasTightViewBounds = false;
    // 호출자가 입증한 두 표현 사이의 추가 변형 오차(월드 m)다. 원본 변형의 가시
    // bounds도 호출자가 포함해야 한다. 근거 없는 wind 진폭을 여기에 추정하지 않는다.
    f32_t additionalWorldError = 0.f;
    // CModel이 알파/vertex paint/terrain 재질에 강한 속성 가중치 cache를 선택한다.
    bool_t attributeSensitive = false;
};

NS_END
