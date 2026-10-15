// 월드/local 평면의 저장과 같은 공간의 구 접촉 검사를 구현한다.
#include "Frustum.h"
// 현재 카메라의 View/Projection을 조회한다. 절두체가 독립적인 카메라 상태를 만들지 않는다.
#include "GameInstance.h"

// 정규화 전에 평면 법선 길이 제곱이 유한한지 검사한다.
#include <cmath>

namespace
{
    // 법선을 정규화하여 PlaneDotCoord를 반지름과 같은 거리 단위로 비교한다.
    // 퇴화/비유한 카메라 평면은 dot=-1인 값으로 바꾸어 정상 비음수 반지름 객체를 이 평면 때문에 제거하지 않는다.
    void Store_NormalizedFrustumPlane(const vector_t plane, float4_t& output)
    {
        const float lengthSquared = XMVectorGetX(XMVector3LengthSq(plane));
        if (!std::isfinite(lengthSquared) || lengthSquared <= 0.f ||
            XMVector4IsNaN(plane) || XMVector4IsInfinite(plane))
        {
            output = {0.f, 0.f, 0.f, -1.f};
            return;
        }
        XMStoreFloat4(&output, XMPlaneNormalize(plane));
    }
}

CFrustum::CFrustum()
{
}

CFrustum::~CFrustum()
{
}

// 추가 자원 할당 없이 성공한다. 초기 0 평면은 아직 유효 카메라를 반영한 값이 아니다.
HRESULT CFrustum::Initialize()
{
    return S_OK;
}

// GameInstance가 카메라 행렬 갱신 뒤 호출한다. DirectX clip 범위는 -w<=x,y<=w, 0<=z<=w다.
void CFrustum::Update_InWorldSpace()
{
    const matrix_t viewProjection =
        XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW)) *
        XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ));
    // 멀리 있는 절두체 꼭짓점을 역투영해 차분하면 오차가 커질 수 있다.
    // row-vector의 View * Projection 열에서 clip 부등식을 직접 꺼내 바깥 방향 평면을 만든다.
    const matrix_t columns = XMMatrixTranspose(viewProjection);
    const vector_t planes[6] = {
        columns.r[0] - columns.r[3], -columns.r[0] - columns.r[3],
        columns.r[1] - columns.r[3], -columns.r[1] - columns.r[3],
        columns.r[2] - columns.r[3], -columns.r[2]
    };
    for (uint32_t i = 0u; i < 6u; ++i)
        Store_NormalizedFrustumPlane(planes[i], m_vWorldPlanes[i]);
}

// local 점이 WorldMatrix를 거쳐 월드로 가므로 평면 계수에는 전치 행렬을 적용한다.
// 비균일 scale/shear에서도 계수 관계를 유지한 뒤 다시 정규화해 local 반지름과 비교한다.
void CFrustum::Update_InLocalSpace(fmatrix_t WorldMatrix)
{
    const matrix_t planeTransform = XMMatrixTranspose(WorldMatrix);
    for (uint32_t i = 0u; i < 6u; ++i)
        Store_NormalizedFrustumPlane(XMPlaneTransform(
            XMLoadFloat4(&m_vWorldPlanes[i]), planeTransform), m_vLocalPlanes[i]);
}

// Layer 등의 sphere culling 질의다. 바깥 평면까지의 부호 거리 >= 반지름이면 구 전체가 경계 밖이다.
bool_t CFrustum::isIn_Frustum_InWorldSpace(fvector_t vWorldPoint, f32_t fRange)
{
    for (uint32_t i = 0; i < 6; i++)
    {
        if (XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vWorldPlanes[i]), vWorldPoint)) >= fRange)
            return false;
    }

    return true;
}

// 마지막 Update_InLocalSpace에 사용한 객체의 local 중심/반지름이어야 한다. 월드 길이를 그대로 섞지 않는다.
bool_t CFrustum::isIn_Frustum_InLocalSpace(fvector_t vLocalPoint, f32_t fRange)
{
    for (uint32_t i = 0; i < 6; i++)
    {
        if (XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vLocalPlanes[i]), vLocalPoint)) >= fRange)
            return false;
    }

    return true;
}

// GameInstance가 소유할 후보를 초기화하고 실패하면 nullptr를 반환한다. 현재 Initialize는 항상 성공한다.
unique_ptr<CFrustum> CFrustum::Create()
{
    auto pInstance = unique_ptr<CFrustum>(new CFrustum());

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CFrustum");
        return nullptr;
    }

    return pInstance;
}