#pragma once

// 엔진 좌표·고정 폭 타입, 셀 배열과 Win32 파일 입력 타입을 사용한다.
#include "Engine_Defines.h"

// 런타임 차단물의 stable ID를 값으로 보관한다.
#include <string>
// 차단물 ID에서 해당 셀 목록과 활성 상태를 찾는다.
#include <unordered_map>

NS_BEGIN(Engine)

// Navigation의 격자 원본과 런타임 차단 상태를 소유한다. PathFinder가 셀 이동 가능 여부를 읽는다.
// Navigation clone들이 같은 grid를 공유하므로 차단물 변경도 공유된다. 내부 잠금이 없으며 변경/질의의 직렬화는 호출자 책임이다.
class ENGINE_DLL CNavGrid final
{
public:
	// 월드 XZ 평면의 규칙 격자다. 폭은 X 셀 수, 높이는 Z 셀 수이며 cell size와 origin은 월드 거리 단위다.
	typedef struct tagNavGridDesc
	{
		uint32_t	iWidth = {};
		uint32_t	iHeight = {};
		f32_t		fCellSize = {};
		f32_t		fOriginX = {};
		f32_t		fOriginZ = {};
	}NAVGRID_DESC;

	// 원본 보행 가능 여부와 셀의 지면 Y다. 런타임 차단은 이 값을 바꾸지 않고 별도 중첩 수로 적용한다.
	typedef struct tagNavCell
	{
		bool_t		isWalkable = { false };
		f32_t		fHeight = {};
	}NAV_CELL;

	// 정렬·중복 제거한 셀 index를 소유한다. isActive가 켜진 blocker마다 해당 셀의 중첩 수를 하나씩 올린다.
	struct RUNTIME_BLOCKER final
	{
		vector<uint32_t> CellIndices;
		bool_t isActive = false;
	};

public:
	CNavGrid() = default;
	~CNavGrid() = default;

public:
	// Navigation 초기화가 binary grid를 읽는다. 명시적 오류 반환은 기존 grid를 보존한다.
	// 성공하면 셀을 교체하고 차단물/중첩 수를 비우며 revision을 1로 초기화한다. 할당 예외를 HRESULT로 변환하지는 않는다.
	HRESULT Load(const tchar_t* pNavGridFilePath);

public:
	// 멤버의 읽기 전용 참조다. Load 성공 이후에는 같은 참조에서 새 격자 설명이 보인다.
	const NAVGRID_DESC& Get_Desc() const
	{
		return m_Desc;
	}
	uint32_t Get_NumCells() const
	{
		return static_cast<uint32_t>(m_Cells.size());
	}

	// 부호 있는 X/Z 좌표가 격자 범위 안에 있는지만 확인한다.
	bool_t Is_ValidCell(int32_t iX, int32_t iZ) const;
	// 원본 보행 허용과 런타임 차단 수 0을 함께 확인한다. 범위 밖 셀은 false다.
	bool_t Is_Walkable(int32_t iX, int32_t iZ) const;
	bool_t Is_Walkable(uint32_t iIndex) const;
	// 중첩 수만 조회한다. 범위 밖 false를 보행 가능으로 해석하지 말고 Is_Walkable을 사용한다.
	bool_t Is_RuntimeBlocked(uint32_t iIndex) const;
	// 월드 XZ 선분이 실제 지나는 셀의 차단·높이 차를 검사한다. 최대 단차도 월드 거리 단위다.
	// 입력 Y가 지면과 일치하는지는 검사하지 않는다. Navigation의 지면 연속성 검사가 그 조건을 별도로 확인한다.
	bool_t Is_SegmentWalkable(
		fvector_t vFrom,
		fvector_t vTo,
		f32_t fMaxStepHeight) const;

	// 저장한 Y를 반환하고 범위 밖은 0이다. 원본이 비보행 셀이면 값의 유한성을 보장하지 않는다.
	f32_t Get_Height(uint32_t iIndex) const;
	// z * width + x로 행 우선 index를 만든다. 범위 검사는 하지 않으므로 먼저 Is_ValidCell을 확인한다.
	uint32_t To_Index(int32_t iX, int32_t iZ) const;

	// 월드 XZ를 원점/셀 크기로 환산해 floor한 정수 셀을 출력한다. Y와 보행 여부는 고려하지 않는다.
	// 범위/유한성 검사에서 실패하면 출력은 유지된다. 마지막 셀 검사 전에는 정수 좌표를 출력한다.
	bool_t World_ToCell(fvector_t vWorldPosition, int32_t& iOutX, int32_t& iOutZ) const;
	// 셀 중심 XZ와 저장 지면 Y를 반환한다. 범위 밖은 영벡터이며 보행 가능 여부는 별도 조회한다.
	float3_t Cell_ToWorld(uint32_t iIndex) const;

	// 셀 목록을 정규화해 등록한다. 기존 ID는 같은 목록일 때만 활성 상태를 변경할 수 있다.
	// 입력 오류·중첩 수 overflow의 false 반환은 기존 등록/차단 상태를 유지한다.
	bool_t Register_RuntimeBlocker(
		const std::string& blockerId,
		const vector<uint32_t>& cellIndices,
		bool_t initiallyActive = false);
	// 상태가 달라질 때 모든 셀의 중첩 수를 사전 검사한 뒤 일괄 증감한다. 같은 상태는 true이며 revision은 유지한다.
	bool_t Set_RuntimeBlockerActive(
		const std::string& blockerId,
		bool_t active);
	// 등록과 중첩 수를 모두 비운다. 등록이 하나라도 있었다면 비활성이었어도 revision을 변경한다.
	void Clear_RuntimeBlockers();
	// 경로 follower의 무효화 표식이다. 변화 시 증가하고 최댓값 다음은 1로 순환한다. Load도 1로 재설정한다.
	uint64_t Get_Revision() const
	{
		return m_iRevision;
	}

private:
	NAVGRID_DESC		m_Desc = {};
	vector<NAV_CELL>	m_Cells;
	// 동시에 활성인 blocker 수다. 하나를 꺼도 다른 blocker가 남아 있으면 셀은 계속 막힌다.
	vector<uint16_t> m_RuntimeBlockCounts;
	std::unordered_map<std::string, RUNTIME_BLOCKER> m_RuntimeBlockers;
	uint64_t m_iRevision = 1;
};

NS_END
