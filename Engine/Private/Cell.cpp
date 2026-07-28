#include "Cell.h"

#ifdef _DEBUG
#include "VIBuffer_Cell.h"
#endif

CCell::CCell(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
{
}

CCell::~CCell()
{
}


HRESULT CCell::Initialize(const float3_t* pPoints, int32_t iIndex)
{
    memcpy(m_vPoints, pPoints, sizeof(float3_t) * ETOUI(POINT::END));

    // XMPlaneFromPointNormal();
    XMStoreFloat4(&m_vPlane, 
        XMPlaneFromPoints(XMLoadFloat3(&m_vPoints[ETOUI(POINT::A)]), XMLoadFloat3(&m_vPoints[ETOUI(POINT::B)]), XMLoadFloat3(&m_vPoints[ETOUI(POINT::C)])));


    m_iIndex = iIndex;

    vector_t        vLine = {};

    vLine = XMLoadFloat3(&m_vPoints[ETOUI(POINT::B)]) - XMLoadFloat3(&m_vPoints[ETOUI(POINT::A)]);
    m_vNormals[ETOUI(LINE::AB)] = float3_t(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));

    vLine = XMLoadFloat3(&m_vPoints[ETOUI(POINT::C)]) - XMLoadFloat3(&m_vPoints[ETOUI(POINT::B)]);
    m_vNormals[ETOUI(LINE::BC)] = float3_t(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));

    vLine = XMLoadFloat3(&m_vPoints[ETOUI(POINT::A)]) - XMLoadFloat3(&m_vPoints[ETOUI(POINT::C)]);
    m_vNormals[ETOUI(LINE::CA)] = float3_t(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));

    for (uint32_t i = 0; i < ETOUI(LINE::END); i++)
    {
        XMStoreFloat3(&m_vNormals[i], 
            XMVector3Normalize(XMLoadFloat3(&m_vNormals[i])));
    }


#ifdef _DEBUG
    m_pVIBuffer = CVIBuffer_Cell::Create(m_pDevice, m_pContext, m_vPoints);
    if (nullptr == m_pVIBuffer)
        return E_FAIL;
#endif

    return S_OK;
}

bool_t CCell::isIn(fvector_t vResultPos, int32_t* pNeighborIndex)
{
    for (uint32_t i = 0; i < ETOUI(LINE::END); i++)
    {
        vector_t        vDir = XMVector3Normalize(vResultPos - XMLoadFloat3(&m_vPoints[i]));

        if (0 < XMVectorGetX(XMVector3Dot(vDir, XMLoadFloat3(&m_vNormals[i]))))
        {
            *pNeighborIndex = m_iNeighborIndices[i];
            return false;
        }
    }

    return true;

}

bool_t CCell::Compare_Points(fvector_t vSourPoint, fvector_t vDestPoint)
{    
    if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[ETOUI(POINT::A)]), vSourPoint))
    {
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[ETOUI(POINT::B)]), vDestPoint))
            return true;
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[ETOUI(POINT::C)]), vDestPoint))
            return true;
    }

    if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[ETOUI(POINT::B)]), vSourPoint))
    {
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[ETOUI(POINT::C)]), vDestPoint))
            return true;
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[ETOUI(POINT::A)]), vDestPoint))
            return true;
    }

    if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[ETOUI(POINT::C)]), vSourPoint))
    {
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[ETOUI(POINT::A)]), vDestPoint))
            return true;
        if (true == XMVector3Equal(XMLoadFloat3(&m_vPoints[ETOUI(POINT::B)]), vDestPoint))
            return true;
    }

    return false;
}

f32_t CCell::Compute_Height(fvector_t vPosition)
{
    // ax + by + cz + d = 0
    // y = (-ax - cz -d) / b

    return (-m_vPlane.x * vPosition.m128_f32[0]
        - m_vPlane.z * vPosition.m128_f32[2]
        - m_vPlane.w) / m_vPlane.y;
}

#ifdef _DEBUG

HRESULT CCell::Render()
{
    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;

    return S_OK;
}

#endif


shared_ptr<CCell> CCell::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const float3_t* pPoints, int32_t iIndex)
{
    auto pInstance = shared_ptr<CCell>(new CCell(pDevice, pContext));

    if (FAILED(pInstance->Initialize(pPoints, iIndex)))
    {
        MSG_BOX("Failed to Created : CCell");
        return nullptr;
    }

    return pInstance;
}
