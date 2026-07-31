#pragma once

#include "Engine_Defines.h"
#include "PathFinder.h"

NS_BEGIN(Engine)

class CNavigation;
class CTransform;

class ENGINE_DLL CNavPathFollower final
{
public:
	PATH_RESULT_CODE Request_Path(
		const shared_ptr<CNavigation>& pNavigation,
		fvector_t vStartPosition,
		fvector_t vGoalPosition,
		f32_t fMaxStepHeight = 0.6f,
		uint32_t iMaxExpandedNodes = 16384);

	bool_t Update(
		const shared_ptr<CTransform>& pTransform,
		f32_t fMoveSpeed,
		f32_t fTimeDelta);

	void Cancel();

	bool_t Has_Path() const;

	PATH_RESULT_CODE Get_LastResult() const { return m_eLastResult; }
	uint32_t Get_LastExpandedNodes() const { return m_iLastExpandedNodes; }
	uint32_t Get_NumWaypoints() const { return static_cast<uint32_t>(m_Waypoints.size()); }

private:
	weak_ptr<CNavigation> m_pNavigation;
	vector<float3_t> m_Waypoints;
	size_t m_iNextWaypoint = {};
	PATH_RESULT_CODE m_eLastResult = { PATH_RESULT_CODE::INVALID_QUERY };
	uint32_t m_iLastExpandedNodes = {};
	uint64_t m_iPathRevision = {};
};

NS_END
