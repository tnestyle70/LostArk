// 격자 파일 로드, 월드/셀 변환과 런타임 차단 상태를 구현한다.
#include "NavGrid.h"

// 차단 셀의 정렬·중복 제거·사전 검사와 중첩 수 초기화를 사용한다.
#include <algorithm>
// 좌표/높이 유한성, 셀 floor와 선분 단차의 절댓값을 계산한다.
#include <cmath>
// 축 이동이 없을 때의 무한대와 blocker 수/revision 최댓값을 사용한다.
#include <limits>

using namespace Engine;

// Navigation의 초기화가 호출한다. 파일 필드와 셀 배열을 지역 값으로 검증한 뒤 멤버를 교체한다.
// 명시적인 I/O/데이터 오류는 기존 값을 보존한다. raw HANDLE·할당 예외에 대한 별도 정리 guard는 없다.
HRESULT CNavGrid::Load(const tchar_t* pNavGridFilePath)
{
	if (nullptr == pNavGridFilePath)
		return E_INVALIDARG;

	HANDLE hFile = CreateFile(
		pNavGridFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	if (INVALID_HANDLE_VALUE == hFile)
		return E_FAIL;

	// binary 순서는 width/height/cell size/origin X/origin Z 다음 walkable byte 배열과 float 높이 배열이다.
	// 구조체 전체 크기나 padding에 의존하지 않고 각 필드를 필요한 크기만큼 읽는다.
	NAVGRID_DESC StagedDesc = {};
	DWORD iByte = {};

	if (FALSE == ReadFile(hFile, &StagedDesc.iWidth, sizeof(uint32_t), &iByte, nullptr) ||
		sizeof(uint32_t) != iByte)
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	if (FALSE == ReadFile(hFile, &StagedDesc.iHeight, sizeof(uint32_t), &iByte, nullptr) ||
		sizeof(uint32_t) != iByte)
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	if (FALSE == ReadFile(hFile, &StagedDesc.fCellSize, sizeof(f32_t), &iByte, nullptr) ||
		sizeof(f32_t) != iByte)
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	if (FALSE == ReadFile(hFile, &StagedDesc.fOriginX, sizeof(f32_t), &iByte, nullptr) ||
		sizeof(f32_t) != iByte)
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	if (FALSE == ReadFile(hFile, &StagedDesc.fOriginZ, sizeof(f32_t), &iByte, nullptr) ||
		sizeof(f32_t) != iByte)
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	if (0 == StagedDesc.iWidth ||
		0 == StagedDesc.iHeight ||
		false == std::isfinite(StagedDesc.fCellSize) ||
		StagedDesc.fCellSize <= 0.f ||
		false == std::isfinite(StagedDesc.fOriginX) ||
		false == std::isfinite(StagedDesc.fOriginZ))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// 셀 수 곱셈은 64비트로 수행하고 100만 셀 상한 뒤에 배열 크기와 ReadFile byte 수를 계산한다.
	const uint64_t iNumCells64 =
		static_cast<uint64_t>(StagedDesc.iWidth) *
		static_cast<uint64_t>(StagedDesc.iHeight);

	if (0 == iNumCells64 || iNumCells64 > 1000000ull)
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	const uint32_t iNumCells = static_cast<uint32_t>(iNumCells64);

	vector<uint8_t> StagedWalkable(iNumCells);
	vector<f32_t> StagedHeights(iNumCells);

	if (FALSE == ReadFile(
		hFile,
		StagedWalkable.data(),
		static_cast<DWORD>(sizeof(uint8_t) * iNumCells),
		&iByte,
		nullptr) ||
		sizeof(uint8_t) * iNumCells != iByte)
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	if (FALSE == ReadFile(
		hFile,
		StagedHeights.data(),
		static_cast<DWORD>(sizeof(f32_t) * iNumCells),
		&iByte,
		nullptr) ||
		sizeof(f32_t) * iNumCells != iByte)
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	CloseHandle(hFile);

	// walkable은 0/1만 허용하고 보행 가능한 셀의 높이만 유한성을 요구한다. 파일 뒤 추가 byte의 유무는 검사하지 않는다.
	vector<NAV_CELL> StagedCells(iNumCells);

	for (uint32_t i = 0; i < iNumCells; ++i)
	{
		if (1 < StagedWalkable[i])
			return E_FAIL;

		if (0 != StagedWalkable[i] &&
			false == std::isfinite(StagedHeights[i]))
			return E_FAIL;

		StagedCells[i].isWalkable = 0 != StagedWalkable[i];
		StagedCells[i].fHeight = StagedHeights[i];
	}

	// 검증이 끝난 값을 교체하고 이전 차단물을 제거한다. 아래 assign 등의 할당 예외까지 원자적으로 복원하는 절차는 없다.
	m_Desc = StagedDesc;
	m_Cells = move(StagedCells);
	m_RuntimeBlockCounts.assign(iNumCells, 0);
	m_RuntimeBlockers.clear();
	m_iRevision = 1;

	return S_OK;
}

// 폭/높이 범위만 검사한다. 실제 통과 여부는 원본 보행 값과 런타임 blocker를 함께 봐야 한다.
bool_t CNavGrid::Is_ValidCell(int32_t iX, int32_t iZ) const
{
	return 0 <= iX &&
		0 <= iZ &&
		iX < static_cast<int32_t>(m_Desc.iWidth) &&
		iZ < static_cast<int32_t>(m_Desc.iHeight);
}

// 유효한 XZ만 index로 바꾸므로 음수 좌표가 unsigned index로 돌아가는 일을 막는다.
bool_t CNavGrid::Is_Walkable(int32_t iX, int32_t iZ) const
{
	if (false == Is_ValidCell(iX, iZ))
		return false;

	return Is_Walkable(To_Index(iX, iZ));
}

// PathFinder와 지면 샘플이 사용하는 최종 셀 통과 조건이다. 원본 허용과 차단 수 0이 모두 필요하다.
bool_t CNavGrid::Is_Walkable(uint32_t iIndex) const
{
	if (m_Cells.size() <= iIndex)
		return false;

	return m_Cells[iIndex].isWalkable &&
		!Is_RuntimeBlocked(iIndex);
}

// 런타임 중첩 상태만 읽는다. 범위 밖은 false지만 그 셀이 존재하거나 걸을 수 있다는 뜻은 아니다.
bool_t CNavGrid::Is_RuntimeBlocked(uint32_t iIndex) const
{
	return iIndex < m_RuntimeBlockCounts.size() &&
		0 != m_RuntimeBlockCounts[iIndex];
}

// Navigation의 경로 단순화·모서리 보정·지면 연속성 검사가 호출한다.
// 월드 XZ 선분을 따라 격자 경계를 순서대로 방문하고 셀별 차단과 최대 단차를 검사한다. 멤버 상태는 변경하지 않는다.
bool_t CNavGrid::Is_SegmentWalkable(
	fvector_t vFrom,
	fvector_t vTo,
	f32_t fMaxStepHeight) const
{
	if (m_Cells.empty() ||
		false == std::isfinite(fMaxStepHeight) ||
		fMaxStepHeight < 0.f ||
		m_Desc.fCellSize <= 0.f)
		return false;

	float3_t vFromPosition{};
	float3_t vToPosition{};
	XMStoreFloat3(&vFromPosition, vFrom);
	XMStoreFloat3(&vToPosition, vTo);
	if (false == std::isfinite(vFromPosition.x) ||
		false == std::isfinite(vFromPosition.z) ||
		false == std::isfinite(vToPosition.x) ||
		false == std::isfinite(vToPosition.z))
		return false;

	int32_t iCurrentX = {};
	int32_t iCurrentZ = {};
	int32_t iTargetX = {};
	int32_t iTargetZ = {};
	if (false == World_ToCell(vFrom, iCurrentX, iCurrentZ) ||
		false == World_ToCell(vTo, iTargetX, iTargetZ) ||
		false == Is_Walkable(iCurrentX, iCurrentZ) ||
		false == Is_Walkable(iTargetX, iTargetZ))
		return false;

	// 셀의 통과 가능 여부와 두 지면 Y의 차를 함께 비교한다. 입력 선분의 Y를 보간해서 판단하지 않는다.
	const auto CanStep = [this, fMaxStepHeight](
		int32_t iFromX,
		int32_t iFromZ,
		int32_t iToX,
		int32_t iToZ)
		{
			if (false == Is_Walkable(iFromX, iFromZ) ||
				false == Is_Walkable(iToX, iToZ))
				return false;

			const f32_t fFromHeight = Get_Height(To_Index(iFromX, iFromZ));
			const f32_t fToHeight = Get_Height(To_Index(iToX, iToZ));
			return fabsf(fToHeight - fFromHeight) <= fMaxStepHeight;
		};

	// 실제 격자 경계 교차 순서로 이동한다. 일정 거리 샘플은 가까운 두 축 교차를
	// 대각선 하나로 합쳐 선분이 닿지 않는 셀 때문에 경로를 거절할 수 있다.
	const double fStartX = (vFromPosition.x - m_Desc.fOriginX) / m_Desc.fCellSize;
	const double fStartZ = (vFromPosition.z - m_Desc.fOriginZ) / m_Desc.fCellSize;
	const double fEndX = (vToPosition.x - m_Desc.fOriginX) / m_Desc.fCellSize;
	const double fEndZ = (vToPosition.z - m_Desc.fOriginZ) / m_Desc.fCellSize;
	const double fDeltaX = fEndX - fStartX;
	const double fDeltaZ = fEndZ - fStartZ;
	const int32_t iStepX = (fDeltaX > 0.0) - (fDeltaX < 0.0);
	const int32_t iStepZ = (fDeltaZ > 0.0) - (fDeltaZ < 0.0);
	const double fInfinity = (std::numeric_limits<double>::infinity)();
	// 선분 매개변수 t(시작 0, 끝 1)에서 다음 축 경계까지의 간격이다. 움직이지 않는 축은 무한대로 둔다.
	const double fRatioStepX = iStepX == 0 ? fInfinity : std::abs(1.0 / fDeltaX);
	const double fRatioStepZ = iStepZ == 0 ? fInfinity : std::abs(1.0 / fDeltaZ);
	double fNextRatioX = iStepX == 0 ? fInfinity :
		(iCurrentX + (iStepX > 0 ? 1.0 : 0.0) - fStartX) / fDeltaX;
	double fNextRatioZ = iStepZ == 0 ? fInfinity :
		(iCurrentZ + (iStepZ > 0 ? 1.0 : 0.0) - fStartZ) / fDeltaZ;
	const bool_t followsXBoundary = iStepX == 0 && fStartX == std::floor(fStartX);
	const bool_t followsZBoundary = iStepZ == 0 && fStartZ == std::floor(fStartZ);
	// 거의 같은 t의 X/Z 교차는 모서리 동시 통과로 취급한다. 폭+높이+1 상한으로 반복을 제한한다.
	constexpr double CORNER_TOLERANCE = 0.000000000001;
	const uint64_t iMaximumVisits = static_cast<uint64_t>(m_Desc.iWidth) + m_Desc.iHeight + 1u;
	for (uint64_t iVisit = 0; iVisit < iMaximumVisits; ++iVisit)
	{
		// 격자 내부 경계를 정확히 따라가면 양쪽 셀에 닿는다. 반대편 셀도 검사해
		// 막힌 셀의 가장자리를 따라 우회하는 지름길이 되지 않게 한다.
		if ((followsXBoundary && iCurrentX > 0 &&
			!CanStep(iCurrentX, iCurrentZ, iCurrentX - 1, iCurrentZ)) ||
			(followsZBoundary && iCurrentZ > 0 &&
			!CanStep(iCurrentX, iCurrentZ, iCurrentX, iCurrentZ - 1)))
			return false;
		if (iCurrentX == iTargetX && iCurrentZ == iTargetZ)
			return true;

		const bool_t crossesX = fNextRatioX <= fNextRatioZ + CORNER_TOLERANCE;
		const bool_t crossesZ = fNextRatioZ <= fNextRatioX + CORNER_TOLERANCE;
		const int32_t iNextX = iCurrentX + (crossesX ? iStepX : 0);
		const int32_t iNextZ = iCurrentZ + (crossesZ ? iStepZ : 0);
		if (!CanStep(iCurrentX, iCurrentZ, iNextX, iNextZ))
			return false;
		// 대각선 모서리를 지날 때 양옆 셀과 도착 셀로의 단차를 모두 확인해 막힌 모서리를 가로지르지 않는다.
		if (crossesX && crossesZ &&
			(!CanStep(iCurrentX, iCurrentZ, iNextX, iCurrentZ) ||
			 !CanStep(iCurrentX, iCurrentZ, iCurrentX, iNextZ) ||
			 !CanStep(iNextX, iCurrentZ, iNextX, iNextZ) ||
			 !CanStep(iCurrentX, iNextZ, iNextX, iNextZ)))
			return false;
		if (crossesX)
		{
			iCurrentX = iNextX;
			fNextRatioX += fRatioStepX;
		}
		if (crossesZ)
		{
			iCurrentZ = iNextZ;
			fNextRatioZ += fRatioStepZ;
		}
	}

	return iCurrentX == iTargetX && iCurrentZ == iTargetZ;
}

// 유효 index의 저장 Y를 그대로 읽는다. 범위 밖의 0 반환은 실제 지면을 찾았다는 보장이 아니다.
f32_t CNavGrid::Get_Height(uint32_t iIndex) const
{
	if (m_Cells.size() <= iIndex)
		return 0.f;

	return m_Cells[iIndex].fHeight;
}

// XZ를 행 우선 배열 index로 바꾼다. 유효 범위 검사는 호출자가 먼저 수행한다.
uint32_t CNavGrid::To_Index(int32_t iX, int32_t iZ) const
{
	return static_cast<uint32_t>(iZ) * m_Desc.iWidth +
		static_cast<uint32_t>(iX);
}

// 월드 XZ에서 격자 원점을 빼고 셀 크기로 나눈다. 격자 외곽의 최대 X/Z 경계는 범위 밖이다.
// 좌표와 크기 검사 후에만 정수로 변환하며 Y·보행 여부는 검사하지 않는다.
bool_t CNavGrid::World_ToCell(
	fvector_t vWorldPosition,
	int32_t& iOutX,
	int32_t& iOutZ) const
{
	if (m_Cells.empty() || m_Desc.fCellSize <= 0.f)
		return false;

	const f32_t fWorldX = XMVectorGetX(vWorldPosition);
	const f32_t fWorldZ = XMVectorGetZ(vWorldPosition);
	if (false == std::isfinite(fWorldX) ||
		false == std::isfinite(fWorldZ))
		return false;

	const f32_t fCellX =
		(fWorldX - m_Desc.fOriginX) / m_Desc.fCellSize;
	const f32_t fCellZ =
		(fWorldZ - m_Desc.fOriginZ) / m_Desc.fCellSize;

	if (false == std::isfinite(fCellX) ||
		false == std::isfinite(fCellZ) ||
		fCellX < 0.f ||
		fCellZ < 0.f ||
		fCellX >= static_cast<f32_t>(m_Desc.iWidth) ||
		fCellZ >= static_cast<f32_t>(m_Desc.iHeight))
		return false;

	iOutX = static_cast<int32_t>(floorf(fCellX));
	iOutZ = static_cast<int32_t>(floorf(fCellZ));

	return Is_ValidCell(iOutX, iOutZ);
}

// PathFinder 결과를 waypoint로 바꿀 때 사용한다. XZ는 셀 중심, Y는 원본 높이이며 보행 여부는 검사하지 않는다.
float3_t CNavGrid::Cell_ToWorld(uint32_t iIndex) const
{
	if (m_Cells.size() <= iIndex || 0 == m_Desc.iWidth)
		return {};

	const uint32_t iX = iIndex % m_Desc.iWidth;
	const uint32_t iZ = iIndex / m_Desc.iWidth;

	return float3_t(
		m_Desc.fOriginX +
		(static_cast<f32_t>(iX) + 0.5f) * m_Desc.fCellSize,
		m_Cells[iIndex].fHeight,
		m_Desc.fOriginZ +
		(static_cast<f32_t>(iZ) + 0.5f) * m_Desc.fCellSize);
}

// Navigation의 차단물 등록을 grid가 소유한다. 셀 목록을 복사·정렬·중복 제거한 뒤 검증한다.
// 기존 ID의 다른 셀 목록 교체는 거부하며, 활성 blocker끼리 겹친 셀은 중첩 수로 유지한다.
bool_t CNavGrid::Register_RuntimeBlocker(
	const std::string& blockerId,
	const vector<uint32_t>& cellIndices,
	bool_t initiallyActive)
{
	if (blockerId.empty() ||
		cellIndices.empty() ||
		m_Cells.empty() ||
		m_RuntimeBlockCounts.size() != m_Cells.size())
	{
		return false;
	}

	vector<uint32_t> normalized = cellIndices;
	std::sort(normalized.begin(), normalized.end());
	normalized.erase(
		std::unique(normalized.begin(), normalized.end()),
		normalized.end());
	if (normalized.empty() ||
		std::any_of(
			normalized.begin(),
			normalized.end(),
			[this](uint32_t index)
			{
				return index >= m_Cells.size();
			}))
	{
		return false;
	}

	// 동일 ID 재등록은 정규화 목록이 같아야 한다. 같은 목록이면 기존 활성화 함수의 사전 검사를 재사용한다.
	const auto existing = m_RuntimeBlockers.find(blockerId);
	if (existing != m_RuntimeBlockers.end())
	{
		if (existing->second.CellIndices != normalized)
			return false;
		return Set_RuntimeBlockerActive(blockerId, initiallyActive);
	}

	if (initiallyActive &&
		std::any_of(
			normalized.begin(),
			normalized.end(),
			[this](uint32_t index)
			{
				return m_RuntimeBlockCounts[index] ==
					(std::numeric_limits<uint16_t>::max)();
			}))
	{
		return false;
	}

	// 새 등록의 모든 입력/overflow 검사를 마친 뒤 map에 저장한다. 실제 활성인 등록만 중첩 수와 revision을 바꾼다.
	RUNTIME_BLOCKER blocker{};
	blocker.CellIndices = std::move(normalized);
	blocker.isActive = initiallyActive;
	const auto [iterator, inserted] =
		m_RuntimeBlockers.emplace(blockerId, std::move(blocker));
	if (!inserted)
		return false;

	if (initiallyActive)
	{
		for (const uint32_t index : iterator->second.CellIndices)
			++m_RuntimeBlockCounts[index];
		if (m_iRevision == (std::numeric_limits<uint64_t>::max)())
			m_iRevision = 1;
		else
			++m_iRevision;
	}

	return true;
}

// Navigation의 가역 차단물 상태 변경이다. 모든 대상 셀을 먼저 검사하므로 명시적 false는 부분 증감을 남기지 않는다.
bool_t CNavGrid::Set_RuntimeBlockerActive(
	const std::string& blockerId,
	bool_t active)
{
	const auto iterator = m_RuntimeBlockers.find(blockerId);
	if (iterator == m_RuntimeBlockers.end())
		return false;

	// 이미 같은 상태면 멱등 성공이다. 상태가 바뀔 때만 중첩 수와 revision을 함께 갱신한다.
	RUNTIME_BLOCKER& blocker = iterator->second;
	if (blocker.isActive == active)
		return true;

	if (active)
	{
		if (std::any_of(
			blocker.CellIndices.begin(),
			blocker.CellIndices.end(),
			[this](uint32_t index)
			{
				return m_RuntimeBlockCounts[index] ==
					(std::numeric_limits<uint16_t>::max)();
			}))
		{
			return false;
		}

		for (const uint32_t index : blocker.CellIndices)
			++m_RuntimeBlockCounts[index];
	}
	else
	{
		if (std::any_of(
			blocker.CellIndices.begin(),
			blocker.CellIndices.end(),
			[this](uint32_t index)
			{
				return 0 == m_RuntimeBlockCounts[index];
			}))
		{
			return false;
		}

		for (const uint32_t index : blocker.CellIndices)
			--m_RuntimeBlockCounts[index];
	}

	blocker.isActive = active;
	if (m_iRevision == (std::numeric_limits<uint64_t>::max)())
		m_iRevision = 1;
	else
		++m_iRevision;
	return true;
}

// 원본 grid는 유지하고 런타임 등록/차단만 초기화한다. 등록이 있었다면 경로를 다시 확인하도록 revision을 바꾼다.
void CNavGrid::Clear_RuntimeBlockers()
{
	if (m_RuntimeBlockers.empty())
		return;

	std::fill(
		m_RuntimeBlockCounts.begin(),
		m_RuntimeBlockCounts.end(),
		static_cast<uint16_t>(0));
	m_RuntimeBlockers.clear();
	if (m_iRevision == (std::numeric_limits<uint64_t>::max)())
		m_iRevision = 1;
	else
		++m_iRevision;
}
