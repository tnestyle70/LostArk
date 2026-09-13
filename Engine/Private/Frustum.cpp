#include "Frustum.h"
#include "GameInstance.h"

#include <cmath>

namespace
{
    void Store_NormalizedFrustumPlane(const vector_t plane, float4_t& output)
    {
        const float lengthSquared = XMVectorGetX(XMVector3LengthSq(plane));
        if (!std::isfinite(lengthSquared) || lengthSquared <= 0.f ||
            XMVector4IsNaN(plane) || XMVector4IsInfinite(plane))
        {
            // Invalid camera data must not reject otherwise visible objects.
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

HRESULT CFrustum::Initialize()
{
    return S_OK;
}

void CFrustum::Update_InWorldSpace()
{
    const matrix_t viewProjection =
        XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW)) *
        XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ));
    // Unprojected far corners can be tens of kilometres apart. Building a
    // side plane from their differences loses the near-plane separation and
    // makes its world offset change with camera rotation. Extract the same
    // homogeneous clip inequalities directly, with outward-facing normals.
    const matrix_t columns = XMMatrixTranspose(viewProjection);
    const vector_t planes[6] = {
        columns.r[0] - columns.r[3], -columns.r[0] - columns.r[3],
        columns.r[1] - columns.r[3], -columns.r[1] - columns.r[3],
        columns.r[2] - columns.r[3], -columns.r[2]
    };
    for (uint32_t i = 0u; i < 6u; ++i)
        Store_NormalizedFrustumPlane(planes[i], m_vWorldPlanes[i]);
}

void CFrustum::Update_InLocalSpace(fmatrix_t WorldMatrix)
{
    // A local point reaches world space through WorldMatrix, so its plane
    // covector uses the transpose. This also preserves nonuniform scale and
    // shear without rebuilding planes from distant transformed corners.
    const matrix_t planeTransform = XMMatrixTranspose(WorldMatrix);
    for (uint32_t i = 0u; i < 6u; ++i)
        Store_NormalizedFrustumPlane(XMPlaneTransform(
            XMLoadFloat4(&m_vWorldPlanes[i]), planeTransform), m_vLocalPlanes[i]);
}

bool_t CFrustum::isIn_Frustum_InWorldSpace(fvector_t vWorldPoint, f32_t fRange)
{
    for (uint32_t i = 0; i < 6; i++)
    {
        if (XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vWorldPlanes[i]), vWorldPoint)) >= fRange)
            return false;
    }

    return true;
}

bool_t CFrustum::isIn_Frustum_InLocalSpace(fvector_t vLocalPoint, f32_t fRange)
{
    for (uint32_t i = 0; i < 6; i++)
    {
        if (XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vLocalPlanes[i]), vLocalPoint)) >= fRange)
            return false;
    }

    return true;
}

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