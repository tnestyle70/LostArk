#include "NavGrid.h"

#include <algorithm>
#include <cmath>

using namespace Engine;

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

	m_Desc = StagedDesc;
	m_Cells = move(StagedCells);

	return S_OK;
}

bool_t CNavGrid::Is_ValidCell(int32_t iX, int32_t iZ) const
{
	return 0 <= iX &&
		0 <= iZ &&
		iX < static_cast<int32_t>(m_Desc.iWidth) &&
		iZ < static_cast<int32_t>(m_Desc.iHeight);
}

bool_t CNavGrid::Is_Walkable(int32_t iX, int32_t iZ) const
{
	if (false == Is_ValidCell(iX, iZ))
		return false;

	return Is_Walkable(To_Index(iX, iZ));
}

bool_t CNavGrid::Is_Walkable(uint32_t iIndex) const
{
	if (m_Cells.size() <= iIndex)
		return false;

	return m_Cells[iIndex].isWalkable;
}

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

	const f32_t fDeltaX = vToPosition.x - vFromPosition.x;
	const f32_t fDeltaZ = vToPosition.z - vFromPosition.z;
	const f32_t fLength = sqrtf(fDeltaX * fDeltaX + fDeltaZ * fDeltaZ);
	const f32_t fSampleDistance = m_Desc.fCellSize * 0.25f;
	const uint32_t iNumSamples = (std::max)(
		1u,
		static_cast<uint32_t>(ceilf(fLength / fSampleDistance)));

	for (uint32_t i = 1; i <= iNumSamples; ++i)
	{
		const f32_t fRatio =
			static_cast<f32_t>(i) / static_cast<f32_t>(iNumSamples);
		const vector_t vSamplePosition = XMVectorSet(
			vFromPosition.x + fDeltaX * fRatio,
			0.f,
			vFromPosition.z + fDeltaZ * fRatio,
			1.f);
		int32_t iSampleX = {};
		int32_t iSampleZ = {};
		if (false == World_ToCell(vSamplePosition, iSampleX, iSampleZ))
			return false;

		if (iSampleX == iCurrentX && iSampleZ == iCurrentZ)
			continue;

		const int32_t iCellDeltaX = iSampleX - iCurrentX;
		const int32_t iCellDeltaZ = iSampleZ - iCurrentZ;
		if (1 < std::abs(iCellDeltaX) || 1 < std::abs(iCellDeltaZ))
			return false;

		if (0 != iCellDeltaX && 0 != iCellDeltaZ)
		{
			if (false == CanStep(iCurrentX, iCurrentZ, iSampleX, iCurrentZ) ||
				false == CanStep(iCurrentX, iCurrentZ, iCurrentX, iSampleZ) ||
				false == CanStep(iSampleX, iCurrentZ, iSampleX, iSampleZ) ||
				false == CanStep(iCurrentX, iSampleZ, iSampleX, iSampleZ))
				return false;
		}
		else if (false == CanStep(
			iCurrentX,
			iCurrentZ,
			iSampleX,
			iSampleZ))
		{
			return false;
		}

		iCurrentX = iSampleX;
		iCurrentZ = iSampleZ;
	}

	return iCurrentX == iTargetX && iCurrentZ == iTargetZ;
}

f32_t CNavGrid::Get_Height(uint32_t iIndex) const
{
	if (m_Cells.size() <= iIndex)
		return 0.f;

	return m_Cells[iIndex].fHeight;
}

uint32_t CNavGrid::To_Index(int32_t iX, int32_t iZ) const
{
	return static_cast<uint32_t>(iZ) * m_Desc.iWidth +
		static_cast<uint32_t>(iX);
}

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
