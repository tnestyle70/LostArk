#include "Navigation.h"
#include "Cell.h"

#include "GameInstance.h"

#include <algorithm>
#include <filesystem>
#include <limits>

namespace
{
	std::filesystem::path ResolveNavigationDataPath(const tchar_t* pDataFile)
	{
		if (nullptr == pDataFile || L'\0' == pDataFile[0])
			return {};

		const std::filesystem::path requestedPath = pDataFile;
		if (requestedPath.is_absolute() || std::filesystem::exists(requestedPath))
			return requestedPath.lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr,
			modulePath,
			static_cast<DWORD>(size(modulePath)));
		if (0 == moduleLength || moduleLength >= size(modulePath))
			return requestedPath.lexically_normal();

		return (std::filesystem::path(modulePath).parent_path() / requestedPath).lexically_normal();
	}
}

CNavigation::CNavigation(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

CNavigation::CNavigation(const CNavigation& Prototype)
	: CComponent { Prototype }
	, m_eMode { Prototype.m_eMode }
	, m_Cells { Prototype.m_Cells }
	, m_pNavGrid { Prototype.m_pNavGrid }
#ifdef _DEBUG
	, m_pShader { Prototype.m_pShader }
	, m_pBatch { Prototype.m_pBatch }
	, m_pEffect { Prototype.m_pEffect }
	, m_pInputLayout { Prototype.m_pInputLayout }
	, m_DebugCellHeights { Prototype.m_DebugCellHeights }
#endif
{
	if (MODE::NAVGRID_ASTAR == m_eMode && nullptr != m_pNavGrid)
		m_pPathFinder = make_unique<CPathFinder>();
}

CNavigation::~CNavigation()
{
}

HRESULT CNavigation::Initialize_Prototype(const tchar_t* pNavigationDataFiles, const tchar_t* pNeighborDataFile)
{
	uint32_t        iByte = {};
	const std::filesystem::path navigationPath = ResolveNavigationDataPath(pNavigationDataFiles);
	HANDLE          hFile = CreateFile(navigationPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == hFile)
		return E_FAIL;

	while (true)
	{
		float3_t		vPoints[3] = {};

		if (FALSE == ReadFile(hFile, vPoints, sizeof(float3_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr))
		{
			CloseHandle(hFile);
			return E_FAIL;
		}
		if (0 == iByte)
			break;
		if (sizeof(float3_t) * 3 != iByte)
		{
			CloseHandle(hFile);
			return E_FAIL;
		}

		if (m_Cells.size() >= static_cast<size_t>((numeric_limits<int32_t>::max)()))
		{
			CloseHandle(hFile);
			return E_FAIL;
		}

		auto		pCell = CCell::Create(
			m_pDevice,
			m_pContext,
			vPoints,
			static_cast<int32_t>(m_Cells.size()));
		if (nullptr == pCell)
		{
			CloseHandle(hFile);
			return E_FAIL;
		}

		m_Cells.push_back(pCell);
	}

	CloseHandle(hFile);
	if (m_Cells.empty())
		return E_FAIL;

	if (FAILED(nullptr == pNeighborDataFile ? SetUp_Neighbors() : SetUp_Neighbors(pNeighborDataFile)))
		return E_FAIL;

#ifdef _DEBUG
	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Cell.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;
#endif
	m_eMode = MODE::LEGACY_TRIANGLE;
	m_pNavGrid.reset();
	m_pPathFinder.reset();
	return S_OK;
}

HRESULT CNavigation::Initialize_NavGrid_Prototype(const tchar_t* pNavGridFilePath)
{
	const std::filesystem::path navGridPath =
		ResolveNavigationDataPath(pNavGridFilePath);

	if (navGridPath.empty())
		return E_INVALIDARG;

	auto pStagedNavGrid = make_shared<CNavGrid>();

	if (FAILED(pStagedNavGrid->Load(navGridPath.c_str())))
		return E_FAIL;

	auto pStagedPathFinder = make_unique<CPathFinder>();

#ifdef _DEBUG
	auto pStagedBatch =
		make_shared<PrimitiveBatch<VertexPositionColor>>(m_pContext.Get());
	auto pStagedEffect = make_shared<BasicEffect>(m_pDevice.Get());
	pStagedEffect->SetVertexColorEnabled(true);

	const void* pVertexShaderByteCode = { nullptr };
	size_t iLength = {};
	pStagedEffect->GetVertexShaderBytecode(
		&pVertexShaderByteCode,
		&iLength);

	ComPtr<ID3D11InputLayout> pStagedInputLayout = { nullptr };
	if (FAILED(m_pDevice->CreateInputLayout(
		VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		pVertexShaderByteCode,
		iLength,
		pStagedInputLayout.GetAddressOf())))
		return E_FAIL;

	vector<f32_t> StagedDebugCellHeights(
		pStagedNavGrid->Get_NumCells(),
		0.f);
	const CNavGrid::NAVGRID_DESC& GridDesc = pStagedNavGrid->Get_Desc();
	const int32_t iMaxRadius = static_cast<int32_t>((std::max)(
		GridDesc.iWidth,
		GridDesc.iHeight));

	for (uint32_t iCellIndex = 0;
		iCellIndex < pStagedNavGrid->Get_NumCells();
		++iCellIndex)
	{
		if (pStagedNavGrid->Is_Walkable(iCellIndex))
		{
			StagedDebugCellHeights[iCellIndex] =
				pStagedNavGrid->Get_Height(iCellIndex);
			continue;
		}

		const int32_t iCellX = static_cast<int32_t>(
			iCellIndex % GridDesc.iWidth);
		const int32_t iCellZ = static_cast<int32_t>(
			iCellIndex / GridDesc.iWidth);

		for (int32_t iRadius = 1; iRadius <= iMaxRadius; ++iRadius)
		{
			f32_t fHeightSum = {};
			uint32_t iHeightCount = {};
			for (int32_t iOffsetZ = -iRadius;
				iOffsetZ <= iRadius;
				++iOffsetZ)
			{
				for (int32_t iOffsetX = -iRadius;
					iOffsetX <= iRadius;
					++iOffsetX)
				{
					if (iRadius != (std::max)(
						std::abs(iOffsetX),
						std::abs(iOffsetZ)))
						continue;

					const int32_t iNeighborX = iCellX + iOffsetX;
					const int32_t iNeighborZ = iCellZ + iOffsetZ;
					if (false == pStagedNavGrid->Is_Walkable(
						iNeighborX,
						iNeighborZ))
						continue;

					fHeightSum += pStagedNavGrid->Get_Height(
						pStagedNavGrid->To_Index(
							iNeighborX,
							iNeighborZ));
					++iHeightCount;
				}
			}

			if (0 != iHeightCount)
			{
				StagedDebugCellHeights[iCellIndex] =
					fHeightSum / static_cast<f32_t>(iHeightCount);
				break;
			}
		}
	}
#endif

	m_Cells.clear();
	m_iCurrentCellIndex = -1;
	m_pTargetTransformCom.reset();
	m_pNavGrid = move(pStagedNavGrid);
	m_pPathFinder = move(pStagedPathFinder);
	m_eMode = MODE::NAVGRID_ASTAR;

#ifdef _DEBUG
	m_pBatch = move(pStagedBatch);
	m_pEffect = move(pStagedEffect);
	m_pInputLayout = move(pStagedInputLayout);
	m_DebugCellHeights = move(StagedDebugCellHeights);
	m_DebugPath.clear();
#endif

	return S_OK;
}

HRESULT CNavigation::Initialize(void* pArg)
{
	if (MODE::NAVGRID_ASTAR == m_eMode)
		return nullptr != m_pNavGrid && nullptr != m_pPathFinder ? S_OK : E_FAIL;

	if (nullptr == pArg)
		return S_OK;

	auto		pDesc = static_cast<NAVIGATION_DESC*>(pArg);

	if (pDesc->iStartCellIndex < 0 ||
		m_Cells.size() <= static_cast<size_t>(pDesc->iStartCellIndex) ||
		nullptr == pDesc->pTransformCom)
		return E_FAIL;

	m_iCurrentCellIndex = pDesc->iStartCellIndex;
	m_pTargetTransformCom = pDesc->pTransformCom;

	m_pTargetTransformCom->Set_State(STATE::POSITION, m_Cells[m_iCurrentCellIndex]->Get_Center());

	return S_OK;
}

HRESULT CNavigation::SetUp_Neighbors()
{
	if (MODE::LEGACY_TRIANGLE != m_eMode)
		return E_FAIL;

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
	if (MODE::LEGACY_TRIANGLE != m_eMode)
		return E_FAIL;

	uint32_t        iByte = {};
	const std::filesystem::path neighborPath = ResolveNavigationDataPath(pNeighborDataFile);
	HANDLE          hFile = CreateFile(neighborPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == hFile)
		return E_FAIL;

	int32_t		iNeighborIndices[3] = {};

	for (auto& pCell : m_Cells)
	{
		if (FALSE == ReadFile(hFile, iNeighborIndices, sizeof(int32_t) * 3, reinterpret_cast<DWORD*>(&iByte), nullptr) ||
			sizeof(int32_t) * 3 != iByte)
		{
			CloseHandle(hFile);
			return E_FAIL;
		}

		for (uint32_t i = 0; i < 3; ++i)
		{
			if (iNeighborIndices[i] < -1 ||
				(-1 != iNeighborIndices[i] &&
					m_Cells.size() <= static_cast<size_t>(iNeighborIndices[i])))
			{
				CloseHandle(hFile);
				return E_FAIL;
			}
		}

		pCell->Set_Neighbor(iNeighborIndices);
	}

	CloseHandle(hFile);

	return S_OK;
}

bool_t CNavigation::isMove(fvector_t vResultPos)
{
	if (MODE::LEGACY_TRIANGLE != m_eMode ||
		m_iCurrentCellIndex < 0 ||
		m_Cells.size() <= static_cast<size_t>(m_iCurrentCellIndex))
		return false;

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
			for (size_t iVisitedCount = 0;
				iVisitedCount < m_Cells.size();
				++iVisitedCount)
			{
				if (iNeighborIndex < 0 ||
					m_Cells.size() <= static_cast<size_t>(iNeighborIndex))
					return false;

				if (true == m_Cells[iNeighborIndex]->isIn(vResultPos, &iNeighborIndex))
				{
					m_iCurrentCellIndex = iNeighborIndex;
					return true;
				}
			}

			return false;
		}

		/* 나간방향에 이웃이 없ㄷ다면. */
		else
			return false;
	}

}

void CNavigation::SetUp_OnNavigation(shared_ptr<class CTransform> pTargetTransform)
{
	if (MODE::LEGACY_TRIANGLE != m_eMode ||
		nullptr == pTargetTransform ||
		m_iCurrentCellIndex < 0 ||
		m_Cells.size() <= static_cast<size_t>(m_iCurrentCellIndex))
		return;

	vector_t		vPosition = pTargetTransform->Get_State(STATE::POSITION);

	vPosition = XMVectorSetY(vPosition, m_Cells[m_iCurrentCellIndex]->Compute_Height(vPosition));

	pTargetTransform->Set_State(STATE::POSITION, vPosition);
}

PATH_RESULT_CODE CNavigation::Find_Path(
	fvector_t vStartPosition,
	fvector_t vGoalPosition,
	f32_t fMaxStepHeight,
	vector<float3_t>& OutPath,
	uint32_t iMaxExpandedNodes,
	uint32_t* pOutExpandedNodes)
{
	if (nullptr != pOutExpandedNodes)
		*pOutExpandedNodes = 0;
	if (MODE::NAVGRID_ASTAR != m_eMode ||
		nullptr == m_pNavGrid ||
		nullptr == m_pPathFinder)
		return PATH_RESULT_CODE::INVALID_GRID;

	int32_t iStartX = {};
	int32_t iStartZ = {};
	int32_t iGoalX = {};
	int32_t iGoalZ = {};

	if (false == m_pNavGrid->World_ToCell(
		vStartPosition,
		iStartX,
		iStartZ))
		return PATH_RESULT_CODE::INVALID_QUERY;

	if (false == m_pNavGrid->World_ToCell(
		vGoalPosition,
		iGoalX,
		iGoalZ))
		return PATH_RESULT_CODE::INVALID_QUERY;

	const uint32_t iStartIndex =
		m_pNavGrid->To_Index(iStartX, iStartZ);

	const uint32_t iGoalIndex =
		m_pNavGrid->To_Index(iGoalX, iGoalZ);

	PATH_QUERY Query{};
	Query.iStartIndex = iStartIndex;
	Query.iGoalIndex = iGoalIndex;
	Query.fMaxStepHeight = fMaxStepHeight;
	Query.iMaxExpandedNodes = iMaxExpandedNodes;

	PATH_RESULT PathResult = m_pPathFinder->Find_Path(*m_pNavGrid, Query);
	if (nullptr != pOutExpandedNodes)
		*pOutExpandedNodes = PathResult.iExpandedNodes;
	if (PATH_RESULT_CODE::SUCCESS != PathResult.eCode)
		return PathResult.eCode;

	vector<float3_t> RawPath;
	RawPath.reserve(PathResult.CellIndices.size());

	for (uint32_t iCellIndex : PathResult.CellIndices)
		RawPath.push_back(
			m_pNavGrid->Cell_ToWorld(iCellIndex));

	if (false == RawPath.empty())
	{
		float3_t vStart{};
		float3_t vGoal{};
		XMStoreFloat3(&vStart, vStartPosition);
		XMStoreFloat3(&vGoal, vGoalPosition);

		if (1 == RawPath.size())
		{
			RawPath.front().x = vGoal.x;
			RawPath.front().z = vGoal.z;
		}
		else
		{
			RawPath.front() = vStart;
			RawPath.back().x = vGoal.x;
			RawPath.back().z = vGoal.z;
		}
	}

	vector<float3_t> SimplifiedPath;
	Simplify_Path(RawPath, fMaxStepHeight, SimplifiedPath);

	vector<float3_t> StagedPath;
	Round_PathCorners(SimplifiedPath, fMaxStepHeight, StagedPath);
	if (StagedPath.empty())
		return PATH_RESULT_CODE::UNREACHABLE;

#ifdef _DEBUG
	m_DebugPath = StagedPath;
#endif

	OutPath = move(StagedPath);

	return PATH_RESULT_CODE::SUCCESS;
}

void CNavigation::Simplify_Path(
	const vector<float3_t>& RawPath,
	f32_t fMaxStepHeight,
	vector<float3_t>& OutPath) const
{
	OutPath.clear();
	if (RawPath.size() <= 2 || nullptr == m_pNavGrid)
	{
		OutPath = RawPath;
		return;
	}

	OutPath.reserve(RawPath.size());
	OutPath.push_back(RawPath.front());

	size_t iAnchor = 0;
	while (iAnchor + 1 < RawPath.size())
	{
		size_t iBest = iAnchor + 1;
		for (size_t iProbe = RawPath.size() - 1;
			iProbe > iAnchor + 1;
			--iProbe)
		{
			if (m_pNavGrid->Is_SegmentWalkable(
				XMLoadFloat3(&RawPath[iAnchor]),
				XMLoadFloat3(&RawPath[iProbe]),
				fMaxStepHeight))
			{
				iBest = iProbe;
				break;
			}
		}

		OutPath.push_back(RawPath[iBest]);
		iAnchor = iBest;
	}
}

void CNavigation::Round_PathCorners(
	const vector<float3_t>& Path,
	f32_t fMaxStepHeight,
	vector<float3_t>& OutPath) const
{
	OutPath.clear();
	if (Path.size() <= 2 || nullptr == m_pNavGrid)
	{
		OutPath = Path;
		return;
	}

	constexpr f32_t MAX_CORNER_RADIUS = 0.75f;
	constexpr uint32_t CURVE_SAMPLES = 8;

	OutPath.reserve(Path.size() * (CURVE_SAMPLES + 1));
	OutPath.push_back(Path.front());

	const auto SetNavigationHeight = [this](float3_t& vPoint)
		{
			int32_t iCellX = {};
			int32_t iCellZ = {};
			if (false == m_pNavGrid->World_ToCell(
				XMLoadFloat3(&vPoint),
				iCellX,
				iCellZ) ||
				false == m_pNavGrid->Is_Walkable(iCellX, iCellZ))
				return false;

			vPoint.y = m_pNavGrid->Get_Height(
				m_pNavGrid->To_Index(iCellX, iCellZ));
			return true;
		};

	for (size_t i = 1; i + 1 < Path.size(); ++i)
	{
		const float3_t& vPrevious = Path[i - 1];
		const float3_t& vCorner = Path[i];
		const float3_t& vNext = Path[i + 1];

		const f32_t fIncomingX = vCorner.x - vPrevious.x;
		const f32_t fIncomingZ = vCorner.z - vPrevious.z;
		const f32_t fOutgoingX = vNext.x - vCorner.x;
		const f32_t fOutgoingZ = vNext.z - vCorner.z;
		const f32_t fIncomingLength = sqrtf(
			fIncomingX * fIncomingX + fIncomingZ * fIncomingZ);
		const f32_t fOutgoingLength = sqrtf(
			fOutgoingX * fOutgoingX + fOutgoingZ * fOutgoingZ);

		if (fIncomingLength <= 0.001f || fOutgoingLength <= 0.001f)
		{
			OutPath.push_back(vCorner);
			continue;
		}

		const f32_t fIncomingDirX = fIncomingX / fIncomingLength;
		const f32_t fIncomingDirZ = fIncomingZ / fIncomingLength;
		const f32_t fOutgoingDirX = fOutgoingX / fOutgoingLength;
		const f32_t fOutgoingDirZ = fOutgoingZ / fOutgoingLength;
		const f32_t fDirectionDot =
			fIncomingDirX * fOutgoingDirX +
			fIncomingDirZ * fOutgoingDirZ;
		if (fDirectionDot <= -0.95f)
		{
			OutPath.push_back(vCorner);
			continue;
		}

		const f32_t fCornerRadius = (std::min)(
			MAX_CORNER_RADIUS,
			(std::min)(fIncomingLength, fOutgoingLength) * 0.25f);

		float3_t vEntry(
			vCorner.x - fIncomingDirX * fCornerRadius,
			vCorner.y,
			vCorner.z - fIncomingDirZ * fCornerRadius);
		const float3_t vExit(
			vCorner.x + fOutgoingDirX * fCornerRadius,
			vCorner.y,
			vCorner.z + fOutgoingDirZ * fCornerRadius);

		vector<float3_t> RoundedCorner;
		RoundedCorner.reserve(CURVE_SAMPLES + 1);
		RoundedCorner.push_back(vEntry);
		for (uint32_t iSample = 1; iSample <= CURVE_SAMPLES; ++iSample)
		{
			const f32_t fT = static_cast<f32_t>(iSample) /
				static_cast<f32_t>(CURVE_SAMPLES);
			const f32_t fInverseT = 1.f - fT;
			float3_t vCurvePoint(
				fInverseT * fInverseT * vEntry.x +
				2.f * fInverseT * fT * vCorner.x +
				fT * fT * vExit.x,
				vCorner.y,
				fInverseT * fInverseT * vEntry.z +
				2.f * fInverseT * fT * vCorner.z +
				fT * fT * vExit.z);
			RoundedCorner.push_back(vCurvePoint);
		}

		bool_t isRoundedCornerWalkable = true;
		float3_t vSegmentStart = OutPath.back();
		for (float3_t& vCurvePoint : RoundedCorner)
		{
			if (false == SetNavigationHeight(vCurvePoint) ||
				false == m_pNavGrid->Is_SegmentWalkable(
					XMLoadFloat3(&vSegmentStart),
					XMLoadFloat3(&vCurvePoint),
					fMaxStepHeight))
			{
				isRoundedCornerWalkable = false;
				break;
			}

			vSegmentStart = vCurvePoint;
		}

		if (isRoundedCornerWalkable)
			OutPath.insert(
				OutPath.end(),
				RoundedCorner.begin(),
				RoundedCorner.end());
		else
			OutPath.push_back(vCorner);
	}

	OutPath.push_back(Path.back());
}

#ifdef _DEBUG

HRESULT CNavigation::Render()
{
	if (MODE::NAVGRID_ASTAR == m_eMode)
	{
		if (nullptr == m_pNavGrid ||
			nullptr == m_pBatch ||
			nullptr == m_pEffect ||
			nullptr == m_pInputLayout)
			return E_FAIL;

		m_pEffect->SetWorld(XMMatrixIdentity());
		m_pEffect->SetView(XMLoadFloat4x4(
			CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
		m_pEffect->SetProjection(XMLoadFloat4x4(
			CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
		m_pContext->IASetInputLayout(m_pInputLayout.Get());
		m_pEffect->Apply(m_pContext.Get());

		const float4_t vWalkableColor =
			float4_t(0.1f, 1.f, 0.2f, 1.f);
		const float4_t vNonWalkableColor =
			float4_t(1.f, 0.85f, 0.05f, 1.f);
		const float4_t vPathColor =
			float4_t(0.1f, 0.85f, 1.f, 1.f);
		const f32_t fHalfCell =
			m_pNavGrid->Get_Desc().fCellSize * 0.5f;

		m_pBatch->Begin();
		for (uint32_t i = 0; i < m_pNavGrid->Get_NumCells(); ++i)
		{
			const bool_t isWalkable = m_pNavGrid->Is_Walkable(i);
			float3_t vCenter = m_pNavGrid->Cell_ToWorld(i);
			if (m_DebugCellHeights.size() == m_pNavGrid->Get_NumCells())
				vCenter.y = m_DebugCellHeights[i];
			const float4_t& vCellColor = isWalkable ?
				vWalkableColor :
				vNonWalkableColor;
			const f32_t fY = vCenter.y + 0.08f;
			const VertexPositionColor vLT(
				float3_t(vCenter.x - fHalfCell, fY, vCenter.z + fHalfCell),
				vCellColor);
			const VertexPositionColor vRT(
				float3_t(vCenter.x + fHalfCell, fY, vCenter.z + fHalfCell),
				vCellColor);
			const VertexPositionColor vRB(
				float3_t(vCenter.x + fHalfCell, fY, vCenter.z - fHalfCell),
				vCellColor);
			const VertexPositionColor vLB(
				float3_t(vCenter.x - fHalfCell, fY, vCenter.z - fHalfCell),
				vCellColor);

			m_pBatch->DrawLine(vLT, vRT);
			m_pBatch->DrawLine(vRT, vRB);
			m_pBatch->DrawLine(vRB, vLB);
			m_pBatch->DrawLine(vLB, vLT);
		}

		for (size_t i = 1; i < m_DebugPath.size(); ++i)
		{
			float3_t vFrom = m_DebugPath[i - 1];
			float3_t vTo = m_DebugPath[i];
			vFrom.y += 0.18f;
			vTo.y += 0.18f;
			m_pBatch->DrawLine(
				VertexPositionColor(vFrom, vPathColor),
				VertexPositionColor(vTo, vPathColor));
		}
		m_pBatch->End();

		return S_OK;
	}

	if (nullptr == m_pShader)
		return S_OK;

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

unique_ptr<CNavigation> CNavigation::Create_NavGrid(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const tchar_t* pNavGridFilePath)
{
	auto pInstance = unique_ptr<CNavigation>(
		new CNavigation(pDevice, pContext));

	if (FAILED(pInstance->Initialize_NavGrid_Prototype(
		pNavGridFilePath)))
	{
		MSG_BOX("Failed to Created : CNavigation NavGrid");
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

