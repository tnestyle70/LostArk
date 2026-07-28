#include "Frustum.h"
#include "GameInstance.h"

CFrustum::CFrustum()
{
}

CFrustum::~CFrustum()
{
}

HRESULT CFrustum::Initialize()
{
    m_vOriginalPoints[0] = float3_t(-1.f, 1.f, 0.f);
    m_vOriginalPoints[1] = float3_t(1.f, 1.f, 0.f);
    m_vOriginalPoints[2] = float3_t(1.f, -1.f, 0.f);
    m_vOriginalPoints[3] = float3_t(-1.f, -1.f, 0.f);

    m_vOriginalPoints[4] = float3_t(-1.f, 1.f, 1.f);
    m_vOriginalPoints[5] = float3_t(1.f, 1.f, 1.f);
    m_vOriginalPoints[6] = float3_t(1.f, -1.f, 1.f);
    m_vOriginalPoints[7] = float3_t(-1.f, -1.f, 1.f);

	return S_OK;
}

void CFrustum::Update_InWorldSpace()
{
    vector_t     vPoints[8] = {};

    for (uint32_t i = 0; i < 8; i++)
    {
        vPoints[i] = XMVector3TransformCoord(XMLoadFloat3(&m_vOriginalPoints[i]),
            XMLoadFloat4x4(CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ)));

        XMStoreFloat3(&m_vWorldPoints[i], XMVector3TransformCoord(vPoints[i],
            XMLoadFloat4x4(CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))));
    }

    Make_Planes(m_vWorldPoints, m_vWorldPlanes);
}

void CFrustum::Update_InLocalSpace(fmatrix_t WorldMatrix)
{
    matrix_t        WorldMatrixInverse = XMMatrixInverse(nullptr, WorldMatrix);

    float3_t     vLocalPoints[8] = {};

    for (uint32_t i = 0; i < 8; i++)
    {
        XMStoreFloat3(&vLocalPoints[i], XMVector3TransformCoord(XMLoadFloat3(&m_vWorldPoints[i]),
            WorldMatrixInverse));
    }

    Make_Planes(vLocalPoints, m_vLocalPlanes);
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

void CFrustum::Make_Planes(const float3_t* pPoints, float4_t* pPlanes)
{
    XMStoreFloat4(&pPlanes[0],
        XMPlaneFromPoints(XMLoadFloat3(&pPoints[1]), XMLoadFloat3(&pPoints[5]), XMLoadFloat3(&pPoints[6])));
    XMStoreFloat4(&pPlanes[1],
        XMPlaneFromPoints(XMLoadFloat3(&pPoints[4]), XMLoadFloat3(&pPoints[0]), XMLoadFloat3(&pPoints[3])));
    XMStoreFloat4(&pPlanes[2],
        XMPlaneFromPoints(XMLoadFloat3(&pPoints[4]), XMLoadFloat3(&pPoints[5]), XMLoadFloat3(&pPoints[1])));
    XMStoreFloat4(&pPlanes[3],
        XMPlaneFromPoints(XMLoadFloat3(&pPoints[3]), XMLoadFloat3(&pPoints[2]), XMLoadFloat3(&pPoints[6])));
    XMStoreFloat4(&pPlanes[4],
        XMPlaneFromPoints(XMLoadFloat3(&pPoints[5]), XMLoadFloat3(&pPoints[4]), XMLoadFloat3(&pPoints[7])));
    XMStoreFloat4(&pPlanes[5],
        XMPlaneFromPoints(XMLoadFloat3(&pPoints[0]), XMLoadFloat3(&pPoints[1]), XMLoadFloat3(&pPoints[2])));
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