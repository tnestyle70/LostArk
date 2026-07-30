#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class ENGINE_DLL CNavGrid final
{
public:
	typedef struct tagNavGridDesc
	{
		uint32_t	iWidth = {};
		uint32_t	iHeight = {};
		f32_t		fCellSize = {};
		f32_t		fOriginX = {};
		f32_t		fOriginZ = {};
	}NAVGRID_DESC;

	typedef struct tagNavCell
	{
		bool_t		isWalkable = { false };
		f32_t		fHeight = {};
	}NAV_CELL;

public:
	CNavGrid() = default;
	~CNavGrid() = default;

public:
	HRESULT Load(const tchar_t* pNavGridFilePath);

public:
	const NAVGRID_DESC& Get_Desc() const { return m_Desc; }
	uint32_t Get_NumCells() const { return static_cast<uint32_t>(m_Cells.size()); }

	bool_t Is_ValidCell(int32_t iX, int32_t iZ) const;
	bool_t Is_Walkable(int32_t iX, int32_t iZ) const;
	bool_t Is_Walkable(uint32_t iIndex) const;
	bool_t Is_SegmentWalkable(
		fvector_t vFrom,
		fvector_t vTo,
		f32_t fMaxStepHeight) const;

	f32_t Get_Height(uint32_t iIndex) const;
	uint32_t To_Index(int32_t iX, int32_t iZ) const;

	bool_t World_ToCell(fvector_t vWorldPosition, int32_t& iOutX, int32_t& iOutZ) const;
	float3_t Cell_ToWorld(uint32_t iIndex) const;

private:
	NAVGRID_DESC		m_Desc = {};
	vector<NAV_CELL>	m_Cells;
};

NS_END
