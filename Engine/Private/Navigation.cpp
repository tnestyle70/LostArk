#include "Navigation.h"
#include "Cell.h"

#include "GameInstance.h"

CNavigation::CNavigation(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

CNavigation::~CNavigation()
{
}

HRESULT CNavigation::Initialize_Prototype(const tchar_t* pNavigationDataFiles, const tchar_t* pNeighborDataFile)
{
	uint32_t        iByte = {};
	HANDLE          hFile = CreateFile(pNavigationDataFiles, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (0 == hFile)
		return E_FAIL;

	while (true)
	{
		float3_t		vPoints[3] = {};

		ReadFile(hFile, vPoints, sizeof(float3_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr);
		if (0 == iByte)
			break;

		auto		pCell = CCell::Create(m_pDevice, m_pContext, vPoints, m_Cells.size());
		if (nullptr == pCell)
			return E_FAIL;

		m_Cells.push_back(pCell);
	}

	CloseHandle(hFile);

	if (FAILED(nullptr == pNeighborDataFile ? SetUp_Neighbors() : SetUp_Neighbors(pNeighborDataFile)))
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
	if (nullptr == pArg)
		return S_OK;

	auto		pDesc = static_cast<NAVIGATION_DESC*>(pArg);

	if (m_Cells.size() <= pDesc->iStartCellIndex)
		return E_FAIL;

	m_iCurrentCellIndex = pDesc->iStartCellIndex;
	m_pTargetTransformCom = pDesc->pTransformCom;	

	m_pTargetTransformCom->Set_State(STATE::POSITION, m_Cells[m_iCurrentCellIndex]->Get_Center());

	return S_OK;
}

HRESULT CNavigation::SetUp_Neighbors()
{
	for (auto& pSourCell : m_Cells)
	{
		for (auto& pDestCell : m_Cells)
		{
			if (pSourCell == pDestCell)
				continue;

			if (true == pDestCell->Compare_Points(pSourCell->Get_Point(POINT::A), pSourCell->Get_Point(POINT::B)))		
				pSourCell->Set_Neighbor(LINE::AB, pDestCell);

			if (true == pDestCell->Compare_Points(pSourCell->Get_Point(POINT::B), pSourCell->Get_Point(POINT::C)))
				pSourCell->Set_Neighbor(LINE::BC, pDestCell);

			if (true == pDestCell->Compare_Points(pSourCell->Get_Point(POINT::C), pSourCell->Get_Point(POINT::A)))
				pSourCell->Set_Neighbor(LINE::CA, pDestCell);
		}
	}

	return S_OK;
}

HRESULT CNavigation::SetUp_Neighbors(const tchar_t* pNeighborDataFile)
{
	uint32_t        iByte = {};
	HANDLE          hFile = CreateFile(pNeighborDataFile, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (0 == hFile)
		return E_FAIL;

	int32_t		iNeighborIndices[3] = {};

	for (auto& pCell : m_Cells)
	{
		ReadFile(hFile, iNeighborIndices, sizeof(int32_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr);

		pCell->Set_Neighbor(iNeighborIndices);
	}

	CloseHandle(hFile);

	return S_OK;
}

bool_t CNavigation::isMove(fvector_t vResultPos)
{
	int32_t			iNeighborIndex = { -1 };

	/* 현재 존재하고 있던 셀 안에서 움직였다. */
	if (true == m_Cells[m_iCurrentCellIndex]->isIn(vResultPos, &iNeighborIndex))
	{
		return true;
	}
	/* 현재 존재하고 있던 셀 밖으로 움직였다. */
	else
	{
		/* 나간방향에 이웃이 있다면. */
		if(-1 != iNeighborIndex)
		{
			while (true)
			{	
				if (-1 == iNeighborIndex)
					return false;

				if (true == m_Cells[iNeighborIndex]->isIn(vResultPos, &iNeighborIndex))
				{
					m_iCurrentCellIndex = iNeighborIndex;
					return true;
				}
			}
		}

		/* 나간방향에 이웃이 없ㄷ다면. */
		else
			return false;
	}
	
}

void CNavigation::SetUp_OnNavigation(shared_ptr<class CTransform> pTargetTransform)
{
	vector_t		vPosition = pTargetTransform->Get_State(STATE::POSITION);

	vPosition = XMVectorSetY(vPosition, m_Cells[m_iCurrentCellIndex]->Compute_Height(vPosition));

	pTargetTransform->Set_State(STATE::POSITION, vPosition);
}

#ifdef _DEBUG

HRESULT CNavigation::Render()
{
	float4x4_t		WorldMatrix = {};
	XMStoreFloat4x4(&WorldMatrix, XMMatrixIdentity());

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &WorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", CGameInstance::Get().Get_Transform(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", CGameInstance::Get().Get_Transform(D3DTS::PROJ))))
		return E_FAIL;


	float4_t		vColor = {};

	if (-1 != m_iCurrentCellIndex)
	{
		vColor = float4_t(1.f, 0.f, 0.f, 1.f);

		m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof(float4_t));

		m_pShader->Begin(0);
		m_Cells[m_iCurrentCellIndex]->Render();
	}
	else
	{
		vColor = float4_t(0.f, 1.f, 0.f, 1.f);

		m_pShader->Bind_RawValue("g_vColor", &vColor, sizeof(float4_t));

		m_pShader->Begin(0);
		for (auto& pCell : m_Cells)
			pCell->Render();
	}


	return S_OK;
}

#endif

unique_ptr<CNavigation> CNavigation::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pNavigationDataFiles, const tchar_t* pNeighborDataFile)
{
	auto pInstance = unique_ptr<CNavigation>(new CNavigation(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pNavigationDataFiles, pNeighborDataFile)))
	{
		MSG_BOX("Failed to Created : CNavigation");
		return nullptr;
	}

	return pInstance;
}


shared_ptr<CPrototype> CNavigation::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CNavigation>(new CNavigation(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CNavigation");
		return nullptr;
	}

	return pInstance;
}

