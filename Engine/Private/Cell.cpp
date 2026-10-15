// 삼각형 셀의 저장 값과 Navigation이 소비하는 기하 질의를 구현한다.
#include "Cell.h"

#ifdef _DEBUG
// Debug에서만 삼각형 외곽선을 그릴 정점/index 버퍼를 만든다.
#include "VIBuffer_Cell.h"
#endif

// 셀의 Debug 버퍼와 함께 사용할 device/context COM 참조를 보유한다.
CCell::CCell(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
{
}

// COM/shared_ptr 멤버가 참조를 해제한다. 이웃은 index이므로 추가 순환 참조가 없다.
CCell::~CCell()
{
}

// Create가 호출한다. 입력 정점 세 개를 복사하므로 원본 배열을 계속 보유할 필요는 없다.
// 평면과 변 법선을 먼저 저장하고 Debug 버퍼를 만든다. 실패 복원은 하지 않고 Create가 객체를 폐기한다.
HRESULT CCell::Initialize(const float3_t* pPoints, int32_t iIndex)
{
    memcpy(m_vPoints, pPoints, sizeof(float3_t) * ETOUI(POINT::END));

    // 세 정점에서 ax + by + cz + d = 0의 계수를 만든다. 이후 XZ 위치의 지면 높이 계산에 쓴다.
    // 대안 API 메모: XMPlaneFromPointNormal();
    XMStoreFloat4(&m_vPlane,
        XMPlaneFromPoints(XMLoadFloat3(&m_vPoints[ETOUI(POINT::A)]), XMLoadFloat3(&m_vPoints[ETOUI(POINT::B)]), XMLoadFloat3(&m_vPoints[ETOUI(POINT::C)])));

    m_iIndex = iIndex;

    vector_t        vLine = {};

    // XZ 변 방향을 90도 돌린 (-z, 0, x)를 바깥쪽 법선으로 사용한다.
    // 입력 winding을 뒤집으면 안/밖 판정도 뒤집히므로 데이터 작성 순서를 유지해야 한다.
    vLine = XMLoadFloat3(&m_vPoints[ETOUI(POINT::B)]) - XMLoadFloat3(&m_vPoints[ETOUI(POINT::A)]);
    m_vNormals[ETOUI(LINE::AB)] = float3_t(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));

    vLine = XMLoadFloat3(&m_vPoints[ETOUI(POINT::C)]) - XMLoadFloat3(&m_vPoints[ETOUI(POINT::B)]);
    m_vNormals[ETOUI(LINE::BC)] = float3_t(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));

    vLine = XMLoadFloat3(&m_vPoints[ETOUI(POINT::A)]) - XMLoadFloat3(&m_vPoints[ETOUI(POINT::C)]);
    m_vNormals[ETOUI(LINE::CA)] = float3_t(XMVectorGetZ(vLine) * -1.f, 0.f, XMVectorGetX(vLine));

    // 부호 판정용 변 법선을 정규화한다. 길이 0인 변에 대한 별도 거부는 없다.
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

// Navigation이 이동 후보를 검사한다. 처음 양의 내적이 나온 변만 따라 다음 셀을 찾는다.
// 포함 판정은 수평 변 법선에 의존하며 위치의 Y가 평면 위인지까지 검사하지 않는다.
bool_t CCell::isIn(fvector_t vResultPos, int32_t* pNeighborIndex)
{
    for (uint32_t i = 0; i < ETOUI(LINE::END); i++)
    {
        vector_t        vDir = XMVector3Normalize(vResultPos - XMLoadFloat3(&m_vPoints[i]));

        if (0 < XMVectorGetX(XMVector3Dot(vDir, XMLoadFloat3(&m_vNormals[i]))))
        {
            // 이웃 -1은 이동 가능한 다음 셀이 없다는 뜻이다. Navigation이 실패/탐색 종료로 소비한다.
            *pNeighborIndex = m_iNeighborIndices[i];
            return false;
        }
    }

    return true;

}

// Navigation의 이웃 구축이 두 변 끝점을 대조한다. 순서는 무관하지만 정확한 부동소수점 일치를 요구한다.
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

// Navigation이 이동 후 같은 XZ의 지면 Y를 구한다. 아래 a/b/c/d는 저장한 평면 계수다.
// 분모 b가 0인 경우의 별도 처리는 없으므로 일반 지면 삼각형이라는 입력 전제가 있다.
f32_t CCell::Compute_Height(fvector_t vPosition)
{
    // ax + by + cz + d = 0
    // y = (-ax - cz -d) / b

    return (-m_vPlane.x * vPosition.m128_f32[0]
        - m_vPlane.z * vPosition.m128_f32[2]
        - m_vPlane.w) / m_vPlane.y;
}

#ifdef _DEBUG

// Navigation이 준비한 Debug shader로 이 셀의 선을 그린다. 버퍼 바인딩에 실패하면 draw를 건너뛴다.
HRESULT CCell::Render()
{
    if (FAILED(m_pVIBuffer->Bind_Resources()))
        return E_FAIL;

    if (FAILED(m_pVIBuffer->Render()))
        return E_FAIL;

    return S_OK;
}

#endif

// 새 셀만 초기화하며 성공한 객체를 Navigation에 넘긴다. 실패 객체의 COM/버퍼 참조는 자동 해제된다.
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
