#pragma once

// 월드 위치 벡터, waypoint 배열과 소유/차용 포인터 타입을 사용한다.
#include "Engine_Defines.h"
// 호출자가 성공·입력 오류·탐색 한도 등을 구분할 PATH_RESULT_CODE를 사용한다.
#include "PathFinder.h"

NS_BEGIN(Engine)

// 헤더에는 포인터 선언만 필요하다. 실제 경로 질의와 Transform 변경은 CPP에서 수행한다.
class CNavigation;
class CTransform;

// Character의 로컬 이동 예측과 Valtan 추적이 가진 경로 진행 상태다. waypoint는 소유하고 Navigation은 약하게 참조한다.
// 목표 질의와 매 프레임 이동은 소유 객체가 순서대로 호출한다. 내부 동기화와 Server 이동 확정은 담당하지 않는다.
class ENGINE_DLL CNavPathFollower final
{
public:
	// 현재 경로를 먼저 비우고 월드 시작/목표의 경로를 요청한다. 실패하면 기존 경로는 남지 않는다.
	// 실패 시 기존 예측을 보존하려는 Character는 별도 follower에 요청하고 성공한 값만 교체한다.
	PATH_RESULT_CODE Request_Path(
		const shared_ptr<CNavigation>& pNavigation,
		fvector_t vStartPosition,
		fvector_t vGoalPosition,
		f32_t fMaxStepHeight = 0.6f,
		uint32_t iMaxExpandedNodes = 16384);

	// 이동 속도(월드 거리/초)와 경과 시간(초)으로 수평 이동량을 소모한다. 각 새 XZ의 지면 높이를 다시 샘플한다.
	// 위치와 바라보는 방향을 변경하며 반환값은 남은 waypoint 유무다. false만으로 도착/취소를 구분하지 않는다.
	bool_t Update(
		const shared_ptr<CTransform>& pTransform,
		f32_t fMoveSpeed,
		f32_t fTimeDelta);

	// 경로와 Navigation 차용을 해제한다. 마지막 요청의 결과 코드와 탐색 노드 수는 진단용으로 보존한다.
	void Cancel();

	// 미소비 waypoint가 있는지만 확인한다. Navigation 수명/revision 유효성은 다음 Update가 검사한다.
	bool_t Has_Path() const;

	// 가장 최근 Request_Path 결과다. Update의 지면 샘플 실패나 Cancel로 이 값이 바뀌지는 않는다.
	PATH_RESULT_CODE Get_LastResult() const
	{
		return m_eLastResult;
	}
	uint32_t Get_LastExpandedNodes() const
	{
		return m_iLastExpandedNodes;
	}
	// 남은 개수가 아닌 저장된 전체 waypoint 수다. 완료한 경로도 Cancel/다음 요청 전까지 배열이 남는다.
	uint32_t Get_NumWaypoints() const
	{
		return static_cast<uint32_t>(m_Waypoints.size());
	}

private:
	// Navigation 수명을 연장하지 않는다. 요청 당시 revision과 약한 참조가 다음 Update의 경로 무효화 기준이다.
	weak_ptr<CNavigation> m_pNavigation;
	vector<float3_t> m_Waypoints;
	size_t m_iNextWaypoint = {};
	PATH_RESULT_CODE m_eLastResult = { PATH_RESULT_CODE::INVALID_QUERY };
	uint32_t m_iLastExpandedNodes = {};
	uint64_t m_iPathRevision = {};
};

NS_END
