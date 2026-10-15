// component 모드별 초기화, 셀/region 질의와 경로 보정을 구현한다.
#include "Navigation.h"
// Debug 삼각형 shader가 사용할 VTXPOS 입력 layout을 제공한다.
#include "Engine_VertexTypes.h"
// 경로 검색/단순화/모서리 보정의 CPU 구간을 분리해 측정한다.
#include "Profiler.h"
// 기존 삼각형 모드의 이웃·포함 판정·평면 높이와 Debug 선 렌더를 사용한다.
#include "Cell.h"

// profiler와 Debug view/projection 행렬을 얻고 shader/Transform 소비에 필요한 엔진 선언을 사용한다.
#include "GameInstance.h"

// 모서리 보정 반경을 상한과 인접 구간 길이 중 작은 값으로 제한하는 std::min을 사용한다.
#include <algorithm>
// 좌표/단차 유한성, 높이 차와 수평 구간 길이를 검사한다.
#include <cmath>
// 입력 파일 경로를 현재 디렉터리/실행 파일 기준으로 해석한다.
#include <filesystem>
// 삼각형 셀 index를 int32 범위에 맞춰 제한한다.
#include <limits>

namespace
{
	// 절대 경로나 현재 디렉터리에서 찾은 경로를 우선한다. 그 외 상대 경로는 실행 파일 옆을 기준으로 붙인다.
	// 실행 파일 경로를 못 얻으면 요청 경로만 정규화해 반환한다. 존재/읽기 성공을 보장하는 함수는 아니다.
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

// 기반 component가 device/context 참조를 소유한다. 실제 데이터는 이어지는 Prototype 초기화가 준비한다.
CNavigation::CNavigation(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

// 원본 셀/grid/Debug shader는 공유하고 NavGrid 탐색 공간은 새로 만든다.
// 현재 삼각형 index와 대상 Transform은 복사하지 않고 Initialize에서 인스턴스별로 설정한다.
CNavigation::CNavigation(const CNavigation& Prototype)
	: CComponent { Prototype }
	, m_eMode { Prototype.m_eMode }
	, m_Cells { Prototype.m_Cells }
	, m_pNavGrid { Prototype.m_pNavGrid }
	, m_fMaxStepHeight { Prototype.m_fMaxStepHeight }
#ifdef _DEBUG
	, m_pShader { Prototype.m_pShader }
#endif
{
	if (MODE::NAVGRID_ASTAR == m_eMode && nullptr != m_pNavGrid)
		m_pPathFinder = make_unique<CPathFinder>();
	// region component도 clone마다 새로 만들어 탐색 작업 공간을 분리한다. region grid 원본은 공유한다.
	for (const shared_ptr<CNavigation>& pRegion : Prototype.m_Regions)
		m_Regions.push_back(shared_ptr<CNavigation>(new CNavigation(*pRegion)));
}

// 멤버 소유 포인터가 참조를 해제한다. 다른 clone이 쓰는 공유 셀/grid는 그 참조가 남아 있는 동안 유지된다.
CNavigation::~CNavigation()
{
}

// Create가 삼각형 정점 파일과 선택적인 이웃 파일로 기존 모드를 구성한다.
// 명시적 파일 오류는 handle을 닫지만 이미 추가한 셀/이웃은 rollback하지 않는다. Create의 새 객체 폐기로 정리한다.
HRESULT CNavigation::Initialize_Prototype(const tchar_t* pNavigationDataFiles,
	const tchar_t* pNeighborDataFile)
{
	uint32_t        iByte = {};
	const std::filesystem::path navigationPath = ResolveNavigationDataPath(pNavigationDataFiles);
	HANDLE          hFile = CreateFile(navigationPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == hFile)
		return E_FAIL;

	// 삼각형 세 float3 단위로 읽고 짧은 마지막 레코드는 거절한다. 0 byte만 정상 파일 끝으로 본다.
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

// Create_NavGrid가 단차와 파일을 검사한 뒤 grid/finder를 임시로 준비한다.
// 입력·Load 실패 전에 기존 멤버를 바꾸지 않고, 준비가 끝나면 삼각형 상태를 비우고 NavGrid 모드를 설정한다.
HRESULT CNavigation::Initialize_NavGrid_Prototype(
	const tchar_t* pNavGridFilePath,
	f32_t fMaxStepHeight)
{
	if (!std::isfinite(fMaxStepHeight) || fMaxStepHeight < 0.f)
		return E_INVALIDARG;
	const std::filesystem::path navGridPath =
		ResolveNavigationDataPath(pNavGridFilePath);

	if (navGridPath.empty())
		return E_INVALIDARG;

	// Load가 끝난 grid와 새 PathFinder를 준비한 후 멤버를 교체한다. 할당 예외는 HRESULT로 변환하지 않는다.
	auto pStagedNavGrid = make_shared<CNavGrid>();

	if (FAILED(pStagedNavGrid->Load(navGridPath.c_str())))
		return E_FAIL;

	auto pStagedPathFinder = make_unique<CPathFinder>();

	m_Cells.clear();
	m_iCurrentCellIndex = -1;
	m_pTargetTransformCom.reset();
	m_pNavGrid = move(pStagedNavGrid);
	m_pPathFinder = move(pStagedPathFinder);
	m_fMaxStepHeight = fMaxStepHeight;
	m_eMode = MODE::NAVGRID_ASTAR;

	return S_OK;
}

// Clone이 호출한다. NavGrid는 자체 grid/finder를 확인하고, 삼각형은 선택적 desc로 현재 셀/Transform을 설정한다.
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

	// 유효 desc의 Transform을 초기 셀 중심으로 이동시킨다. 초기 위치 보존이 필요한 다른 모드에는 적용하지 않는다.
	m_pTargetTransformCom->Set_State(STATE::POSITION, m_Cells[m_iCurrentCellIndex]->Get_Center());

	return S_OK;
}

// 각 셀의 세 변을 모든 다른 셀의 정점과 정확히 비교해 이웃 index를 저장한다.
// 여러 셀이 같은 변을 공유하면 뒤에서 찾은 셀이 덮어쓰며 topology 자체를 검증하는 절차는 아니다.
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

// 셀 순서대로 세 이웃 index를 읽고 -1 또는 유효 셀 범위만 허용한다.
// 앞선 셀의 저장을 중간 실패 때 되돌리지 않으며 파일 뒤 추가 byte나 이웃의 상호 일치를 검사하지 않는다.
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

// 삼각형 현재 셀에서 시작해 후보 위치가 속할 이웃을 찾는다. 포함 셀을 찾은 경우에만 현재 index를 바꾼다.
// 순환하는 잘못된 이웃 데이터에서도 전체 셀 수만큼만 방문하며 실패 시 Transform은 건드리지 않는다.
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

// 기존 삼각형 모드의 높이 보정이다. 전달받은 Transform XZ를 현재 셀 평면에 대입해 Y만 바꾼다.
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

// follower/타기팅 소비자가 전달한 월드 위치로 셀 경로를 구하고 표시용 waypoint로 보정한다.
// OutPath는 마지막 성공 시에만 교체한다. 확장 노드 수 출력과 내부 PathFinder 작업 공간은 실패 때도 바뀔 수 있다.
PATH_RESULT_CODE CNavigation::Find_Path(
	fvector_t vStartPosition,
	fvector_t vGoalPosition,
	f32_t fMaxStepHeight,
	vector<float3_t>& OutPath,
	uint32_t iMaxExpandedNodes,
	uint32_t* pOutExpandedNodes)
{
	// Server와 같은 시작점 소유 규칙이다. detail region 안에서 시작하면 해당 region만 경로를 구한다.
	if (CNavigation* pRegion = Select_Region(XMVectorGetX(vStartPosition),
		XMVectorGetZ(vStartPosition), XMVectorGetY(vStartPosition)))
	{
		return pRegion->Find_Path(vStartPosition, vGoalPosition,
			pRegion->m_fMaxStepHeight, OutPath, iMaxExpandedNodes, pOutExpandedNodes);
	}
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Navigation.FindPath");
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

	// 월드 XZ가 들어간 두 셀 index와 최대 단차/확장 예산을 PathFinder에 전달한다.
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

	// 셀 중심을 waypoint로 바꾼다. 여러 셀이면 첫 점은 원래 시작 위치, 마지막 XZ는 원래 목표를 유지한다.
	// 한 셀 경로에서는 그 한 점의 XZ만 목표로 바꾸고 지면 Y는 셀 높이를 사용한다.
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

	// 중간 점 제거와 모서리 보정을 각각 임시 배열에 수행한 뒤 최종 경로를 한 번 교체한다.
	vector<float3_t> SimplifiedPath;
	Simplify_Path(RawPath, fMaxStepHeight, SimplifiedPath);

	vector<float3_t> StagedPath;
	Round_PathCorners(SimplifiedPath, fMaxStepHeight, StagedPath);
	if (StagedPath.empty())
		return PATH_RESULT_CODE::UNREACHABLE;
	OutPath = move(StagedPath);

	return PATH_RESULT_CODE::SUCCESS;
}

// 직업 공통 타기팅과 경로 추적이 현재 XZ의 담당 지면 높이를 읽는다.
// 선택된 region이 거절하면 base로 재시도하지 않는다. 마지막 Y 유한성 검사 전에 출력은 기록된다.
bool_t CNavigation::Try_SampleWalkablePoint(
	fvector_t vWorldPosition,
	float3_t& outPosition) const
{
	if (MODE::NAVGRID_ASTAR != m_eMode || nullptr == m_pNavGrid)
		return false;
	const f32_t x = XMVectorGetX(vWorldPosition);
	const f32_t z = XMVectorGetZ(vWorldPosition);
	if (!std::isfinite(x) || !std::isfinite(z))
		return false;
	if (const CNavigation* pRegion = Select_Region(x, z, XMVectorGetY(vWorldPosition)))
		return pRegion->Try_SampleWalkablePoint(vWorldPosition, outPosition);
	int32_t cellX = 0;
	int32_t cellZ = 0;
	if (!m_pNavGrid->World_ToCell(vWorldPosition, cellX, cellZ) ||
		!m_pNavGrid->Is_Walkable(cellX, cellZ))
	{
		return false;
	}
	const uint32_t index = m_pNavGrid->To_Index(cellX, cellZ);
	outPosition = { x, m_pNavGrid->Get_Height(index), z };
	return std::isfinite(outPosition.y);
}

// 복제 위치의 지면 연속성을 확인한다. 각 끝점의 담당 grid와 실제 Y가 같아야 선분 검사를 진행한다.
// 다른 층으로 투영하거나 높이를 보정해서 성공으로 만들지 않는다.
bool_t CNavigation::Is_GroundedSegmentContinuous(
	fvector_t vFromPosition,
	fvector_t vToPosition) const
{
	if (MODE::NAVGRID_ASTAR != m_eMode || nullptr == m_pNavGrid)
		return false;
	float3_t from{};
	float3_t to{};
	XMStoreFloat3(&from, vFromPosition);
	XMStoreFloat3(&to, vToPosition);
	if (!std::isfinite(from.x) || !std::isfinite(from.y) || !std::isfinite(from.z) ||
		!std::isfinite(to.x) || !std::isfinite(to.y) || !std::isfinite(to.z))
		return false;

	const CNavigation* fromOwner = Select_Region(from.x, from.z, from.y);
	const CNavigation* toOwner = Select_Region(to.x, to.z, to.y);
	if (nullptr == fromOwner)
		fromOwner = this;
	if (nullptr == toOwner)
		toOwner = this;
	// 위층이 걸을 수 있는 셀이어도 다른 층에서 수직 이동할 수 있다는 증명은 아니다.
	// 양 끝을 다른 담당 grid나 높이로 투영해 연속성 검사를 통과시키지 않는다.
	if (fromOwner != toOwner || nullptr == fromOwner->m_pNavGrid)
		return false;

	float3_t fromGround{};
	float3_t toGround{};
	// 끝점 Y와 셀 지면 Y의 허용 오차는 월드 거리 0.001이다. 선분의 단차는 담당 region/base의 정책을 사용한다.
	constexpr f32_t GROUND_HEIGHT_TOLERANCE = 0.001f;
	if (!fromOwner->Try_SampleWalkablePoint(vFromPosition, fromGround) ||
		!fromOwner->Try_SampleWalkablePoint(vToPosition, toGround) ||
		std::abs(from.y - fromGround.y) > GROUND_HEIGHT_TOLERANCE ||
		std::abs(to.y - toGround.y) > GROUND_HEIGHT_TOLERANCE)
		return false;
	return fromOwner->m_pNavGrid->Is_SegmentWalkable(
		vFromPosition, vToPosition, fromOwner->m_fMaxStepHeight);
}

// base grid의 등록 검증 결과를 전달한다. region grid에는 이 등록을 전파하지 않는다.
bool_t CNavigation::Register_RuntimeBlocker(
	const std::string& blockerId,
	const vector<uint32_t>& cellIndices,
	bool_t initiallyActive)
{
	if (MODE::NAVGRID_ASTAR != m_eMode || nullptr == m_pNavGrid)
		return false;

	return m_pNavGrid->Register_RuntimeBlocker(
		blockerId,
		cellIndices,
		initiallyActive);
}

// base grid를 공유하는 clone 모두가 소비할 blocker 상태를 변경한다. 실패 조건과 revision 갱신은 grid가 담당한다.
bool_t CNavigation::Set_RuntimeBlockerActive(
	const std::string& blockerId,
	bool_t active)
{
	if (MODE::NAVGRID_ASTAR != m_eMode || nullptr == m_pNavGrid)
		return false;

	return m_pNavGrid->Set_RuntimeBlockerActive(blockerId, active);
}

// NavGrid base의 런타임 등록만 비운다. 원본 셀/높이와 region 목록은 유지한다.
void CNavigation::Clear_RuntimeBlockers()
{
	if (MODE::NAVGRID_ASTAR == m_eMode && nullptr != m_pNavGrid)
		m_pNavGrid->Clear_RuntimeBlockers();
}

// follower가 요청 당시 base grid revision을 대조한다. region 자체 변경을 합성한 전체 트리 revision은 아니다.
uint64_t CNavigation::Get_NavigationRevision() const
{
	return MODE::NAVGRID_ASTAR == m_eMode && nullptr != m_pNavGrid ?
		m_pNavGrid->Get_Revision() :
		0;
}

// 로더가 준비한 단층 region Navigation의 unique 소유권을 부모 목록으로 옮긴다.
// 모드/grid/finder와 중첩 region 부재를 확인한다. 실패 시에도 값으로 넘겨받은 인수는 폐기된다.
HRESULT CNavigation::Attach_Region(unique_ptr<CNavigation> pRegion)
{
	if (MODE::NAVGRID_ASTAR != m_eMode || nullptr == m_pNavGrid ||
		nullptr == pRegion || MODE::NAVGRID_ASTAR != pRegion->m_eMode ||
		nullptr == pRegion->m_pNavGrid || nullptr == pRegion->m_pPathFinder ||
		false == pRegion->m_Regions.empty())
	{
		return E_INVALIDARG;
	}
	m_Regions.push_back(shared_ptr<CNavigation>(move(pRegion)));
	return S_OK;
}

// 질의 XZ가 들어가는 detail region을 찾는다. 반환 포인터는 m_Regions가 소유한 component를 차용한다.
CNavigation* CNavigation::Select_Region(f32_t fX, f32_t fZ, f32_t fHintY) const
{
	// footprint가 하나면 그 region이 답한다. 겹치면 보행 가능한 셀, hint에 가까운 지면, 등록 순서로 고른다.
	// 보행 가능한 후보를 우선하는 기준은 CServerNavigation과 같지만, 모두 막혔을 때의 선택은 다르다.
	// 여기서는 첫 region을 고르고 Server는 각 region의 보행 높이 범위와 hint의 거리를 비교한다.
	// 이 함수에서 선택된 region의 질의가 실패해도 base로 우회하지 않는다.
	// footprint 후보 수를 먼저 센다. 후보가 하나면 막힌 셀이어도 그 region이 답하며 base에 넘기지 않는다.
	const vector_t vPoint = XMVectorSet(fX, 0.f, fZ, 1.f);
	int32_t cellX = 0;
	int32_t cellZ = 0;
	CNavigation* pFirst = nullptr;
	size_t iCandidateCount = 0;
	for (const shared_ptr<CNavigation>& pRegion : m_Regions)
	{
		if (false == pRegion->m_pNavGrid->World_ToCell(vPoint, cellX, cellZ))
			continue;
		if (nullptr == pFirst)
			pFirst = pRegion.get();
		++iCandidateCount;
	}
	if (iCandidateCount <= 1)
		return pFirst;

	// 겹친 footprint는 보행 가능 여부, 유한한 hint와의 높이 차 순으로 고른다. hint가 비유한이면 높이 차는 0으로 둔다.
	CNavigation* pBest = nullptr;
	bool_t bestHasGround = false;
	f32_t fBestDistance = 0.f;
	for (const shared_ptr<CNavigation>& pRegion : m_Regions)
	{
		if (false == pRegion->m_pNavGrid->World_ToCell(vPoint, cellX, cellZ))
			continue;
		const bool_t hasGround = pRegion->m_pNavGrid->Is_Walkable(cellX, cellZ);
		f32_t fDistance = 0.f;
		if (hasGround && std::isfinite(fHintY))
		{
			fDistance = std::abs(pRegion->m_pNavGrid->Get_Height(
				pRegion->m_pNavGrid->To_Index(cellX, cellZ)) - fHintY);
		}
		const bool_t better = nullptr == pBest ||
			(hasGround && !bestHasGround) ||
			(hasGround == bestHasGround && fDistance < fBestDistance);
		if (better)
		{
			pBest = pRegion.get();
			bestHasGround = hasGround;
			fBestDistance = fDistance;
		}
	}
	return pBest;
}

// 같은 base grid의 원시 경로에서 직선 통과가 가능한 가장 먼 waypoint를 차례로 선택한다.
// 입력과 별도인 출력 배열을 먼저 비운다. 인접한 원시 구간의 유효성은 앞선 셀 탐색을 전제로 한다.
void CNavigation::Simplify_Path(
	const vector<float3_t>& RawPath,
	f32_t fMaxStepHeight,
	vector<float3_t>& OutPath) const
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Navigation.Simplify");
	OutPath.clear();
	if (RawPath.size() <= 2 || nullptr == m_pNavGrid)
	{
		OutPath = RawPath;
		return;
	}

	OutPath.reserve(RawPath.size());
	OutPath.push_back(RawPath.front());

	// 현재 anchor에서 끝점부터 역으로 검사해 통과 가능한 가장 먼 점을 고른다. 매 선택마다 anchor가 앞으로 진행한다.
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

// 단순화한 경로 모서리를 2차 Bezier의 선분들로 바꿔 이동 방향 변화를 완화한다.
// 후보별 지면/구간 검사에 실패하면 해당 모서리만 원래 점으로 남긴다. 입력과 출력 배열은 별개여야 한다.
void CNavigation::Round_PathCorners(
	const vector<float3_t>& Path,
	f32_t fMaxStepHeight,
	vector<float3_t>& OutPath) const
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Navigation.RoundCorners");
	OutPath.clear();
	if (Path.size() <= 2 || nullptr == m_pNavGrid)
	{
		OutPath = Path;
		return;
	}

	// 보정 반경은 월드 거리 최대 0.75이며 양옆 수평 구간 중 짧은 길이의 1/4을 넘지 않는다. 곡선은 8구간으로 나눈다.
	constexpr f32_t MAX_CORNER_RADIUS = 0.75f;
	constexpr uint32_t CURVE_SAMPLES = 8;

	OutPath.reserve(Path.size() * (CURVE_SAMPLES + 1));
	OutPath.push_back(Path.front());

	// 곡선 후보의 XZ가 실제 보행 셀 안에 있을 때 해당 셀의 Y로 맞춘다. 여기서는 같은 경로 소유 grid만 사용한다.
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

		// 거의 길이가 없는 구간은 방향 정규화를 하지 않고 원래 모서리를 유지한다.
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
		// 거의 되돌아가는 방향은 곡선으로 돌리지 않는다. 내적 -0.95 이하인 모서리를 그대로 둔다.
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

		// 진입점·원래 모서리·이탈점을 제어점으로 하는 2차 Bezier를 샘플한다. t는 0~1의 무차원 진행률이다.
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

		// 직전 출력 점에서 각 곡선 점까지 지면과 선분을 검사한다. 모든 후보가 통과할 때만 이 모서리의 곡선 묶음을 추가한다.
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

// Debug에서 현재 삼각형은 빨강, 현재 셀이 없으면 모든 셀은 초록으로 그린다. NavGrid는 별도 draw 없이 성공한다.
// 행렬 bind 실패는 반환하지만 색상/Begin/각 Cell Render의 HRESULT는 여기서 전달하지 않는다.
HRESULT CNavigation::Render()
{
	if (MODE::NAVGRID_ASTAR == m_eMode)
		return S_OK;

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

// 기존 삼각형용 새 Prototype을 초기화한다. 실패하면 진단을 출력하고 부분 상태가 든 새 객체를 폐기한다.
unique_ptr<CNavigation> CNavigation::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pNavigationDataFiles, const tchar_t* pNeighborDataFile)
{
	auto pInstance = unique_ptr<CNavigation>(new CNavigation(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pNavigationDataFiles, pNeighborDataFile)))
	{
		OutputDebugStringA("[Engine][Navigation] Create failed.\n");
		return nullptr;
	}

	return pInstance;
}

// NavGrid용 새 Prototype을 초기화해 소유권을 넘긴다. 실패는 nullptr이며 외부 Prototype을 변경하지 않는다.
unique_ptr<CNavigation> CNavigation::Create_NavGrid(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const tchar_t* pNavGridFilePath,
	f32_t fMaxStepHeight)
{
	auto pInstance = unique_ptr<CNavigation>(
		new CNavigation(pDevice, pContext));

	if (FAILED(pInstance->Initialize_NavGrid_Prototype(
		pNavGridFilePath, fMaxStepHeight)))
	{
		OutputDebugStringA("[Engine][Navigation] NavGrid create failed.\n");
		return nullptr;
	}

	return pInstance;
}


// 원본의 공유 데이터와 새 탐색 공간을 복제한 뒤 모드별 초기화를 수행한다. 실패 시 새 clone만 폐기한다.
shared_ptr<CPrototype> CNavigation::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CNavigation>(new CNavigation(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Engine][Navigation] Clone failed.\n");
		return nullptr;
	}

	return pInstance;
}

