#pragma once

#include "GameplayCatalog.h"
#include "ServerPlayer.h"
#include "WorldBootstrap.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace LostArk::Server
{
	// 파괴 상태가 요청하는 placement 또는 점으로 구분한 하위 ID 집합의 flag 변경이다.
	struct SERVER_COLLISION_STATE_CHANGE final
	{
		std::string strPlacementId;
		bool bPlayerBlocking = true;
		bool bImpactReceiverEnabled = true;
	};

	// Prepare가 만든 flag 복사본이다. 성공한 후보만 즉시 Commit해야 하며 실패 후보는 폐기한다.
	// iBaseRevision은 기준 정보다. Commit 내부에서 비교하거나 잠금으로 재검사하지 않는다.
	struct SERVER_COLLISION_STATE_STAGE final
	{
		std::uint64_t iBaseRevision = 0u;
		std::uint64_t iNextRevision = 0u;
		std::vector<bool> PlayerBlocking;
		std::vector<bool> ImpactReceiverEnabled;
		bool bChanged = false;
	};

	// 가장 이른 receiver의 stable ID와 이동 구간 비율 t(0~1)이다.
	struct SERVER_BOSS_RECEIVER_HIT final
	{
		std::string strReceiverPlacementId;
		float fHitRatio = 1.f;
	};

	// 돌진을 막은 기본 벽 ID와 같은 최초 충돌 비율의 활성 receiver ID를 별도로 보관한다.
	struct SERVER_BOSS_WALL_HIT final
	{
		std::string strCollisionPlacementId;
		std::string strImpactReceiverPlacementId;
		float fHitRatio = 1.f;
	};

	// 월드 entity의 원형 XZ 몸체와 수직 범위다. 서로 다른 높이의 몸체가 불필요하게 막지 않도록 함께 검사한다.
	// NetEntityId는 자신의 몸체를 충돌 후보에서 제외하는 데 사용한다.
	struct SERVER_BLOCKING_BODY final
	{
		float fX = 0.f;
		float fZ = 0.f;
		float fRadius = 0.f;
		// 기존 세 필드 초기화도 허용하도록 기본 높이 범위는 매우 넓다.
		// GameRoom의 제품 경로는 실제 중심 Y와 반높이를 채워 넣는다.
		float fCenterY = 0.f;
		float fHalfHeight = 100000.f;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
	};

	// GameRoom이 정적 충돌 placement/flag와 동적 몸체 값 복사본을 소유하도록 묶는 처리기다.
	// 내부 잠금은 없으므로 초기화·몸체 갱신·조회·상태 commit은 호출자가 순서를 보장해야 한다.
	class CServerCollisionSystem final
	{
	public:
		// GameRoom 초기화가 활성 COLLISION_BOX만 검증하여 교체한다. false 반환 전에는 기존 상태를 유지한다.
		// 성공하면 벽 flag/revision을 초기화하지만 동적 몸체 목록은 별도 갱신 경로에 남긴다.
		bool Initialize(
			const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
			std::string& outStatus);
		// PLAYER_SPAWN 종류/활성 여부와 정적 blocking 벽만 검사한다. 동적 몸체는 이 검사에 포함하지 않는다.
		bool Is_PlayerSpawnClear(
			const WORLD_BOOTSTRAP_PLACEMENT& spawn) const;
		// 순간이동/착지의 도착 위치만 검사한다. 이미 겹친 몸체에서 빠져나가도록 허용하는 이동 sweep과 구분한다.
		// 따라서 길이 0인 이동을 대신 호출하여 도착 위치 승인을 얻지 않는다.
		bool Is_PlayerPositionClear(float x, float y, float z,
			LostArk::Shared::NET_ENTITY_ID ignoredBodyId) const;
		// 임의 몸체 크기의 도착 승인이다. box는 반지름만큼 확장한 사각 범위로, 동적 몸체는 원으로 검사한다.
		bool Is_CirclePositionClear(float x, float y, float z, float radius,
			float halfHeight, float centerOffsetY,
			LostArk::Shared::NET_ENTITY_ID ignoredBodyId) const;
		// GameRoom이 만든 몸체 값 목록으로 기존 목록을 대체한다. 유한한 양수 크기의 행만 남기며 revision은 바꾸지 않는다.
		void Set_BlockingBodies(std::vector<SERVER_BLOCKING_BODY> bodies);
		// 이동을 확정한 entity의 첫 일치 몸체 위치만 갱신한다. 잘못된 ID/좌표 또는 없는 몸체는 false로 기존 목록을 유지한다.
		bool Update_BlockingBody(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			float x,
			float centerY,
			float z) noexcept;
		// 서 있는 원형 몸체의 제안 이동을 벽/다른 몸체로 제한한다. 입력 길이는 m이며 각도는 placement의 degree다.
		// false이면 출력은 보존한다. true라도 위치가 원안과 같다는 뜻은 아니며 outWasBlocked로 정지 여부를 구분한다.
		bool Resolve_CircleMove(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			float halfHeight,
			float centerOffsetY,
			float& outX,
			float& outY,
			float& outZ,
			bool& outWasBlocked,
			LostArk::Shared::NET_ENTITY_ID ignoredBodyId =
				LostArk::Shared::INVALID_NET_ENTITY_ID,
			bool allowBodySlide = true) const;
		// 클릭한 목표가 이번 이동에서 만난 몸체 안에 있으면 그 경계 도달을 판별한다.
		// 몸체 뒤의 목표를 향한 접선 우회와 몸체 자체를 목표로 한 도착을 구분한다.
		bool Is_PlayerMoveBlockedAtGoalBody(
			const SERVER_PLAYER& player, float proposedX, float proposedY,
			float proposedZ, float goalY) const;
		// 고정 플레이어 크기로 Resolve_CircleMove를 호출한다. Mario 중에는 동적 몸체 접선 우회를 끈다.
		bool Resolve_PlayerMove(
			const SERVER_PLAYER& player,
			float proposedX,
			float proposedY,
			float proposedZ,
			float& outX,
			float& outY,
			float& outZ,
			bool& outWasBlocked) const;
		// 활성 receiver 중 가장 이른 접촉을 찾고 거의 같은 비율이면 ID 사전순으로 고른다. outHit은 먼저 초기화한다.
		bool Sweep_BossCircleAgainstReceivers(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			SERVER_BOSS_RECEIVER_HIT& outHit) const;
		// 돌진에서 가장 이른 blocking 벽과 같은 충돌 비율의 선택적 receiver ID를 찾는다.
		// 두 ID를 보존하여 호출자가 정확한 impact 연결과 일반 contact 연결을 차례로 처리한다.
		// 호출 시작에 outHit을 비우므로 false가 기존 출력 보존을 뜻하지 않는다.
		bool Sweep_BossCircleAgainstWalls(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			SERVER_BOSS_WALL_HIT& outHit) const;
		// 이번 이동 구간에 접촉한 blocking 벽 ID를 전부 모은다. 가장 이른 벽이나 receiver만으로 제한하지 않는다.
		// 출력은 먼저 비우고 placement 순서로 추가하며 중복 제거/정렬은 하지 않는다.
		void Collect_BossCircleContacts(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			std::vector<std::string>& outContactPlacementIds) const;
		// Server 공격 pulse의 도형과 blocking 벽을 교차 검사한다. 활성 receiver는 이 공격 접촉 목록에서 제외한다.
		// Client 도끼/본의 시각 위치를 읽지 않으며 실제 파괴 적용은 호출하는 GameRoom이 맡는다.
		void Collect_BossPatternHitContacts(
			BOSS_PATTERN_HIT_SHAPE hitShape,
			float originX,
			float originY,
			float originZ,
			float yawDegrees,
			float verticalReach,
			float outerRadius,
			float innerRadius,
			float angleDegrees,
			float length,
			float halfWidth,
			std::vector<std::string>& outContactPlacementIds) const;
		// 실제 flag는 바꾸지 않고 변경 후보를 작성한다. false여도 outStage는 부분 작성될 수 있으므로 사용하지 않는다.
		bool Prepare_StateChanges(
			const std::vector<SERVER_COLLISION_STATE_CHANGE>& changes,
			SERVER_COLLISION_STATE_STAGE& outStage,
			std::string& outStatus) const;
		// 방의 파괴 transaction이 성공한 뒤 호출한다. stage의 세대/크기를 재검증하지 않고 flag와 다음 revision을 이동한다.
		void Commit_StateChanges(SERVER_COLLISION_STATE_STAGE&& stage) noexcept;
		// 벽 flag와 revision을 초기 상태로 되돌린다. placement와 동적 몸체 목록은 지우지 않는다.
		void Reset_RuntimeStates() noexcept;
		// 정확한 placement ID 존재만 조회한다. 비활성 blocking flag여도 존재하면 true다.
		bool Has_CollisionBox(const std::string& placementId) const;
		// 정확한 ID 또는 ID + 점으로 시작하는 하위 placement가 있는지 확인한다.
		bool Has_CollisionStateTarget(const std::string& stateId) const;
		// 정확한 placement ID의 현재 flag를 읽는다. ID가 없으면 false다.
		bool Is_PlayerBlocking(const std::string& placementId) const;
		// 정확한 placement ID의 receiver flag를 읽는다. ID가 없으면 false다.
		bool Is_ImpactReceiverEnabled(const std::string& placementId) const;
		// 벽 상태의 revision만 읽는다. 동적 몸체 좌표 변경을 감지하는 번호는 아니다.
		std::uint64_t Get_Revision() const noexcept
		{
			return m_iRevision;
		}

		[[nodiscard]] std::size_t Get_CollisionBoxCount() const
		{
			return m_CollisionBoxes.size();
		}
		// Debug audition 표시가 현재 blocking flag 수를 조회한다.
		// 동적 몸체 수나 receiver 수를 세는 함수는 아니다.
		[[nodiscard]] std::size_t Get_ActivePlayerBlockingCount() const
		{
			std::size_t active = 0u;
			for (const bool blocking : m_PlayerBlocking)
			{
				if (blocking)
					++active;
			}
			return active;
		}

	private:
		// spawn 확인용이다. 고정 플레이어 크기로 확장한 box의 로컬 범위를 검사한다.
		static bool Is_PlayerCenterInsideExpandedBox(
			float playerX,
			float playerY,
			float playerZ,
			const WORLD_BOOTSTRAP_PLACEMENT& box);
		// box 로컬 XZ의 확장 사각형에 이동 선분을 검사한다. Y는 이동 전체의 높이 범위로 먼저 거른다.
		static bool Sweep_MovingBodyAgainstBox(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			float halfHeight,
			float centerOffsetY,
			const WORLD_BOOTSTRAP_PLACEMENT& box,
			float& outHitRatio);
		// 보스용 box 검사다. 반지름을 수직 반높이와 발 위치→중심 오프셋에도 사용한다.
		static bool Sweep_CircleAgainstBox(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			const WORLD_BOOTSTRAP_PLACEMENT& box,
			float& outHitRatio);
		// 비어 있지 않은 ID의 .receiver 접미사로 receiver 역할을 판별한다.
		static bool Is_ImpactReceiverPlacementId(
			std::string_view placementId) noexcept;
		// 두 원형 몸체의 XZ 이동을 수직 범위와 함께 검사한다. 이미 겹쳤으면 중심 쪽으로 더 파고드는 이동만 막는다.
		// Y가 변하면 높이 겹침 시간과 원의 겹침 시간을 교차하고, false일 때 outHitRatio는 보존한다.
		static bool Sweep_CircleAgainstBody(
			float startX,
			float startY,
			float startZ,
			float proposedX,
			float proposedY,
			float proposedZ,
			float radius,
			float halfHeight,
			float centerOffsetY,
			const SERVER_BLOCKING_BODY& body,
			float& outHitRatio);

	private:
		// placement와 두 flag vector의 index가 대응한다. 외부에는 값 또는 stable ID를 넘기며 내부 참조를 보관시키지 않는다.
		std::vector<WORLD_BOOTSTRAP_PLACEMENT> m_CollisionBoxes;
		std::vector<bool> m_PlayerBlocking;
		std::vector<bool> m_ImpactReceiverEnabled;
		// world entity의 몸체 값 복사본이다. 원래 entity 포인터나 그 수명을 소유하지 않는다.
		std::vector<SERVER_BLOCKING_BODY> m_BlockingBodies;
		std::uint64_t m_iRevision = 0u;
	};
}
