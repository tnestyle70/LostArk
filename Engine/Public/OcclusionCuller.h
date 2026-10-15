#pragma once

// 월드/clip 행렬·벡터, 공개 DLL 심볼과 소유 포인터 등 엔진 공통 타입을 사용한다.
#include "Engine_Defines.h"
// 호출 중에만 차용하는 삼각형 index 범위를 받는다. 원본 mesh의 소유권은 가져오지 않는다.
#include <span>

NS_BEGIN(Engine)

// Renderer가 소유하는 CPU 가림 판정기다. Begin마다 깊이를 초기화하며 GPU query나 장면 객체를 소유하지 않는다.
// 숨김 결과의 재사용 여부는 Renderer가 별도로 판단한다. 이 객체는 불확실한 질의를 false로 돌려 숨김을 요청하지 않는다.
// 내부 잠금이 없으므로 Begin/Rasterize/TestBounds와 소멸은 소유자가 직렬화한다.
class ENGINE_DLL COcclusionCuller final
{
public:
    // 현재 제품의 앞면 방향에 맞춰 제외할 면을 지정한다. NONE은 양면을 사용한다.
    enum class CULL_MODE : uint8_t
    {
        NONE, BACK, FRONT
    };
    // forceSse2는 backend 선택의 상한을 SSE2로 제한한다. 기본은 CPU가 지원하는 AVX2 이하 구현이다.
    explicit COcclusionCuller(bool_t forceSse2 = false);
    // 내부 STATE와 CPU 깊이 buffer를 해제한다. 차용했던 mesh 메모리는 해제하지 않는다.
    ~COcclusionCuller();
    COcclusionCuller(const COcclusionCuller&) = delete;
    COcclusionCuller& operator=(const COcclusionCuller&) = delete;

    // Renderer가 카메라 view/projection과 실제 viewport pixel 크기를 전달한다.
    // 기존 ready/통계를 먼저 비우며 실패하면 이전 깊이로 숨김을 판정하지 않는다. 성공하면 이번 프레임의 빈 깊이를 준비한다.
    bool_t Begin(const float4x4_t& view, const float4x4_t& projection,
        uint32_t viewportWidth, uint32_t viewportHeight);
    // Map 객체가 현재 프레임에 실제 그리는 불투명 형상을 전달해야 한다. 위치는 정점의 byte 0/4/8에 있는 XYZ다.
    // 정점/index 메모리는 호출 중만 차용한다. world는 affine이어야 하며 Begin 성공 후 CPU 깊이에 누적한다.
    // 입력 예산은 후속 검증 실패에도 소비될 수 있다. 할당 실패는 이번 Begin의 판정 전체를 비활성화한다.
    bool_t Rasterize(const float* positions, uint32_t vertexCount,
        uint32_t strideBytes, std::span<const uint32_t> indices,
        const float4x4_t& world, CULL_MODE cullMode);
    // 월드 AABB 전체가 가려졌다고 확인한 경우만 true다. false는 가시성 확정이 아니라 숨김을 생략하라는 뜻이다.
    // 준비 실패·깊이 없음·near/far 경계에 걸친 box·비정상 입력도 false로 처리한다.
    bool_t TestBounds(const float3_t& worldMin, const float3_t& worldMax) const;

    // Begin 한 번당 입력 예산이다. 실제 rasterizer에 제출된 삼각형 수와 구분한다.
    static constexpr uint32_t MAXIMUM_INPUT_TRIANGLES = 100000u;
    // 선행 입력 검사 통과 후 소비한 예산이다. 후속 정점/index 검증이나 할당 실패분도 포함할 수 있다.
    uint32_t GetInputTriangles() const;
    // 이 계층에서 선별해 backend에 제출한 수다. backend 내부 clip 이후 실제 pixel 기여 수는 아니다.
    uint32_t GetRasterizedTriangles() const;
    // backend 저장 크기다. viewport를 가로 32/세로 8의 배수로 올림한 값이며 ready 판정용은 아니다.
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    // 보유 rasterizer의 실제 SIMD: 0=SSE2, 1=SSE4.1, 2=AVX2다. 객체가 없으면 UINT32_MAX다.
    // 이 값만으로 현재 Begin의 성공/ready 여부를 판단할 수는 없다.
    uint32_t GetImplementation() const;

private:
    // backend 타입과 작업 배열은 CPP에 숨기고 하나의 STATE를 단독 소유한다.
    struct STATE;
    std::unique_ptr<STATE> m_State;
    bool_t m_ForceSse2 = false;
};

NS_END
