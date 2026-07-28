#include "Navigation.h"
#include "Cell.h"

#include "GameInstance.h"

const _float4x4* CNavigation::m_pParentMatrix = { nullptr };

CNavigation::CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{

}

CNavigation::CNavigation(const CNavigation& Prototype)
    : CComponent{ Prototype }
    , m_Cells{ Prototype.m_Cells }
    , m_iNumCells{ Prototype.m_iNumCells }
    , m_iCurrentCellIndex{ Prototype.m_iCurrentCellIndex }
#ifdef _DEBUG
    , m_pShader{ Prototype.m_pShader }
#endif
{

#ifdef _DEBUG
    Safe_AddRef(m_pShader);
#endif

    for (auto& pCell : m_Cells)
        Safe_AddRef(pCell);

    

}

_vector CNavigation::Get_CellPos()
{
    return m_Cells[m_iCurrentCellIndex]->Get_CenterPos();
}

HRESULT CNavigation::Initialize_Prototype(const _tchar* pNavigationData, const _tchar* pNavigationNeighborData)
{
    _ulong			dwByte = {};
    HANDLE			hFile = CreateFile(pNavigationData, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (0 == hFile)
        return E_FAIL;

    _float3     vPoints[ENUM_TO_UINT(POINT::END)] = {};

    while(true)
    {
        ReadFile(hFile, vPoints, sizeof(_float3) * ENUM_TO_UINT(POINT::END), &dwByte, nullptr);
        if (0 == dwByte)
            break;

        CCell* pCell = CCell::Create(m_pDevice, m_pContext, vPoints, m_Cells.size());
        if (nullptr == pCell)
            return E_FAIL;

        m_Cells.push_back(pCell);
    }

    if (FAILED(SetUp_Neighbors(pNavigationNeighborData)))
        return E_FAIL;


#ifdef _DEBUG
    m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Cell.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
    if (nullptr == m_pShader)
        return E_FAIL;
#endif

	return S_OK;
}

HRESULT CNavigation::Initialize(void* pArg)
{
    NAVIGATION_DESC* pDesc = static_cast<NAVIGATION_DESC*>(pArg);

    m_iCurrentCellIndex = pDesc->iCurrentCellIndex;
    if(nullptr != pDesc->pParentMatrix)
        m_pParentMatrix = pDesc->pParentMatrix;    

	return S_OK;
}

_vector CNavigation::SetUp_OnNavigation(_fvector vWorldPos)
{
    _vector vCellPos = XMVector3TransformCoord(vWorldPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(m_pParentMatrix)));

    vCellPos = XMVectorSetY(vCellPos, m_Cells[m_iCurrentCellIndex]->Compute_Height(vCellPos));

    return XMVector3TransformCoord(vCellPos, XMLoadFloat4x4(m_pParentMatrix));
}

_bool CNavigation::isMove(_fvector vResultPos)
{
    if (-1 == m_iCurrentCellIndex)
        return false;

    _vector     vCellPos = XMVector3TransformCoord(vResultPos, XMMatrixInverse(nullptr, XMLoadFloat4x4(m_pParentMatrix)));

    _int        iNeighborIndex = { -1 };

    /* ¼¿À» ³ª°¬´Ù. */
    if (false == m_Cells[m_iCurrentCellIndex]->isIn(vCellPos, &iNeighborIndex))
    {
        /* ÀÌ¿ôÀÌ ¾ø´Ù¸é */
        if(-1 == iNeighborIndex)
            return false;

        /* ÀÌ¿ôÀÌ ÀÖ´Ù¸é */
        else
        {
            while(true)
            { 
                if (true == m_Cells[iNeighborIndex]->isIn(vCellPos, &iNeighborIndex))
                    break;

                if (-1 == iNeighborIndex)
                    return false;
            }

            m_iCurrentCellIndex = iNeighborIndex;

            return true;    
        }       
    }

    return true;
}

const list<_vector>* CNavigation::Make_Route(_int iGoalIndex)
{
    if (m_iOldGoalIndex == iGoalIndex)
        return nullptr;

    _int    iStartIndex = m_iCurrentCellIndex;
        
    m_OpenList.push_back(iStartIndex);    

    _int    iRouteCellIndex = { -1 };

    while (true)
    {
        m_OpenList.sort([&](_int iSourIndex, _int iDestIndex)
        {
            return m_Cells[iSourIndex]->Compute_Cost(m_Cells, iGoalIndex) < m_Cells[iDestIndex]->Compute_Cost(m_Cells, iGoalIndex);
        });

        CCell*      pCell = m_Cells[m_OpenList.front()];

        _int        iCellIndex = pCell->Get_Index();

        m_CloseList.push_back(iCellIndex);

        m_OpenList.pop_front();

        _int        iNeighbors[3] = {};
        pCell->Get_Neighbors(iNeighbors);
              
        for (size_t i = 0; i < 3; i++)
        {
            if (-1 == iNeighbors[i])
                continue;

           

            if (iGoalIndex == iNeighbors[i])
            {
                iRouteCellIndex = iGoalIndex;
                m_Cells[iNeighbors[i]]->Set_ParentIndex(iCellIndex);
                m_Route.push_back(m_Cells[iGoalIndex]->Get_CenterPos());
                goto make_route;
            }


            if(true == isPush(iNeighbors[i]))
            {
                m_Cells[iNeighbors[i]]->Set_ParentIndex(iCellIndex);
                m_OpenList.push_back(iNeighbors[i]);
            }
        }
    }

make_route:

    while (true)
    {
        iRouteCellIndex = m_Cells[iRouteCellIndex]->Get_ParentIndex();

        if (-1 == iRouteCellIndex)
        {
            m_Route.reverse();
            break;
        }

        m_Route.push_back(m_Cells[iRouteCellIndex]->Get_CenterPos());
    }

    m_iOldGoalIndex = iGoalIndex;

    return &m_Route;
}

#ifdef _DEBUG
HRESULT CNavigation::Render()
{
    _float4x4       WorldMatrix = *m_pParentMatrix;

    

    if (FAILED(m_pGameInstance->Bind_PipeLineMatrix(m_pShader, "g_ViewMatrix", D3DTS::VIEW)))
        return E_FAIL;
    if (FAILED(m_pGameInstance->Bind_PipeLineMatrix(m_pShader, "g_ProjMatrix", D3DTS::PROJ)))
        return E_FAIL;



    _float4     vColor = {};

    if (-1 == m_iCurrentCellIndex)
    {
        vColor = _float4(0.f, 1.f, 0.f, 1.f);

        if (FAILED(m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof vColor)))
            return E_FAIL;

        if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix)))
            return E_FAIL;

        m_pShader->Begin(0);

        for (auto& pCell : m_Cells)
            pCell->Render();
    }
    else
    {
        vColor = _float4(1.f, 0.f, 0.f, 1.f);

        WorldMatrix._42 += 0.1f;

        if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix)))
            return E_FAIL;

        if (FAILED(m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof vColor)))
            return E_FAIL;

        m_pShader->Begin(0);

        m_Cells[m_iCurrentCellIndex]->Render();
    }

    
    

    

    
    return S_OK;
}
#endif

HRESULT CNavigation::SetUp_Neighbors(const _tchar* pNavigationNeighborData)
{
    if (nullptr == pNavigationNeighborData)
    {
        for (auto& pSourCell : m_Cells)
        {
            for (auto& pDestCell : m_Cells)
            {
                if (pSourCell == pDestCell)
                    continue;

                if (true == pDestCell->Compare(pSourCell->Get_Point(POINT::A), pSourCell->Get_Point(POINT::B)))            
                    pSourCell->Set_Neighbor(LINE::AB, pDestCell);

                if (true == pDestCell->Compare(pSourCell->Get_Point(POINT::B), pSourCell->Get_Point(POINT::C)))
                    pSourCell->Set_Neighbor(LINE::BC, pDestCell);

                if (true == pDestCell->Compare(pSourCell->Get_Point(POINT::C), pSourCell->Get_Point(POINT::A)))
                    pSourCell->Set_Neighbor(LINE::CA, pDestCell);           
            }
        }
    }

    else
    {
        _ulong			dwByte = {};
        HANDLE			hFile = CreateFile(pNavigationNeighborData, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (0 == hFile)
            return E_FAIL;

        _int        iNeighbors[3] = {};

        for (auto& pCell : m_Cells)
        {
            ReadFile(hFile, iNeighbors, sizeof(_int) * 3, &dwByte, nullptr);
            pCell->Set_Neighbors(iNeighbors);
        }
        CloseHandle(hFile);

    }
    return S_OK;
}

_bool CNavigation::isPush(_int iCellIndex)
{
    /* ÀÌ ¼¿ÀÌ ÀÌµ¿°¡´ÉÇÏÁö ¾ÊÀº ¼¿ÀÌ¶ó¸é */
    // return false;


    auto    iter_Open = find_if(m_OpenList.begin(), m_OpenList.end(), [&](_int iCurrentCellIndex) 
    {
        if (iCurrentCellIndex == iCellIndex)
            return true;

        return false;
    });

    if (iter_Open != m_OpenList.end())
        return false;

    auto    iter_Close = find_if(m_CloseList.begin(), m_CloseList.end(), [&](_int iCurrentCellIndex)
        {
            if (iCurrentCellIndex == iCellIndex)
                return true;

            return false;
        });

    if (iter_Close != m_CloseList.end())
        return false;

    return true;
}

CNavigation* CNavigation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pNavigationData, const _tchar* pNavigationNeighborData)
{
    CNavigation* pInstance = new CNavigation(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pNavigationData, pNavigationNeighborData)))
    {
        MSG_BOX("Failed to Created : CNavigation");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CNavigation::Clone(void* pArg)
{
    CNavigation* pInstance = new CNavigation(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CNavigation");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CNavigation::Free()
{
	__super::Free();

   /* _ulong			dwByte = {};
    HANDLE			hFile = CreateFile(TEXT("../Bin/DataFiles/NavigationNeighbors.dat"), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);


    for (auto& pCell : m_Cells)
    {
        pCell->WriteData(hFile);
        Safe_Release(pCell);
    }

    CloseHandle(hFile);*/

    for (auto& pCell : m_Cells)
        Safe_Release(pCell);

	m_Cells.clear();

#ifdef _DEBUG
    Safe_Release(m_pShader);
#endif
}
