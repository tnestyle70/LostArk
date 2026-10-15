#pragma once

// World ID와 쿠크 HUD mode는 Server/Client가 공유하는 enum을 그대로 사용한다.
#include "Network/PacketMessages.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Server
{
	//플레이어 이동에 대한 enum class, wall climb을 여기에 빼야만하는 이유가 있나?
	// MOVE_PLAYER의 저작 경로 종류다. 일반 이동은 출발점과 목적지 사이를, WALL_CLIMB은 시간별 월드 위치 표본을 따라간다.
	// 등반이 렌더링 설정만은 아니므로 이 Server 입력 계약에 둔다. ServerTriggerSystem이 표본을 실행 상태에 복사하고 WALL_CLIMB action을 복제한다.
	enum class WORLD_TRIGGER_MOVE_STYLE : std::uint8_t
	{
		DIRECT,
		WALL_CLIMB,
		END
	};
	//플레이어 이동에 대한 X Y Z 위치 좌표
	// 등반 시작을 0으로 한 iTimeMs와 절대 월드 위치 m의 표본이다. delta나 보스 상대 offset이 아니다.
	// Load가 증가하는 시간과 마지막 목적지 일치를 검사하고, ServerTriggerSystem이 인접 표본 사이를 보간한다.
	struct WORLD_TRIGGER_MOVE_SAMPLE
	{
		std::uint32_t iTimeMs = 0u;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
	};

	// 게시된 배치의 종류와 SERVER_WORLD_ENTITY의 실행 종류가 공유하는 enum이다.
	// 이 enum에 있어도 파일 입력으로 모두 허용되지는 않는다. ParseKind는 PLAYER_SPAWN/NPC/BOSS/TRIGGER_BOX/COLLISION_BOX만 읽는다.
	enum class WORLD_BOOTSTRAP_KIND
	{
		PLAYER_SPAWN,
		NPC,
		BOSS,
		// 일반 몬스터는 SpawnGroup 등 별도 생성 경로가 이 실행 종류를 지정한다. worldbootstrap 행으로 직접 읽지 않는다.
		MONSTER,
		TRIGGER_BOX,
		COLLISION_BOX,
		// 실행 중인 World cue에 연결된 Server 피격 몸체다. Stage_KoukuWorldBody가 생성하며 일반 몬스터 AI나 초기 배치로 취급하지 않는다.
		// Client 표현은 해당 World cue가 담당하므로 일반 entity spawn 복제에서도 제외한다.
		WORLD_OBJECT,
		END
	};

	// 트리거 배치가 요청할 동작을 분류한다. 실행 시점·입장 승인·상태 변경은 ServerTriggerSystem과 GameRoom의 소비 경로가 결정한다.
	enum class WORLD_TRIGGER_ACTION_KIND
	{
		MOVE_PLAYER,
		CHANGE_LEVEL,
		ACTIVATE_SPAWN_GROUP,
		ACTIVATE_ENCOUNTER,
		// strTargetId로 지정한 World sequence의 재생을 요청한다. Server가 트리거 조건과 시작을 확정하고 Client가 연출을 재생한다.
		// 쿠크 GATE1 입장 sequence는 Begin_KoukuRaidPreparation을 통해 Server의 레이드 준비도 시작한다.
		PLAY_SEQUENCE,
		// 카드 미로 망원경용으로 게시되는 동작 구분이다. 현재 제품은 일반 입장/G 입력에서 제외한다.
		// GameRoom은 고정 TELESCOPE_PLACEMENT_ID로 배치를 찾아 망치 타격을 판정한다. 이 동작의 strTargetId로 타격 영역을 찾지 않는다.
		CLAIM_CARD_MAZE_TELESCOPE,
		END
	};

	// NPC의 논리 이동 방식이다. 실제 경로 추종과 대기는 GameRoom의 NPC 실행 상태가 진행한다.
	enum class NPC_BEHAVIOR_MODE : std::uint8_t
	{
		STATIONARY,
		PATROL,
		WANDER,
		END
	};

	// PATROL이 끝 waypoint에 도달했을 때 반복·왕복·종료 중 어떤 흐름을 사용할지 정한다.
	enum class NPC_ROUTE_MODE : std::uint8_t
	{
		LOOP,
		PING_PONG,
		ONCE,
		END
	};

	// NPC 논리 action을 목록 순서 또는 가중 추첨으로 고르는 정책이다. 애니메이션 clip 선택과 구분한다.
	enum class NPC_ACTION_SELECTION : std::uint8_t
	{
		SEQUENCE,
		WEIGHTED,
		END
	};

	// 순찰 목적지의 stable ID, 절대 월드 위치 m, 도착 후 대기 ms를 보관한다. look yaw는 bHasLookYaw일 때만 적용하는 degree 값이다.
	struct WORLD_NPC_BEHAVIOR_WAYPOINT final
	{
		std::string strWaypointId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		std::uint32_t iWaitMs = 0u;
		bool bHasLookYaw = false;
		float fLookYawDegrees = 0.f;
	};

	// NPC가 복제할 논리 action ID와 실행/후속 대기 ms다. iWeight는 가중 선택 정책에서 사용하는 양의 가중치다.
	struct WORLD_NPC_BEHAVIOR_ACTION final
	{
		std::string strActionId;
		std::uint32_t iDurationMs = 0u;
		std::uint32_t iWaitAfterMs = 0u;
		std::uint32_t iWeight = 1u;
	};

	// Gameplay.world.json에서 publisher가 만든 Server 논리 행동 정의다. 속도는 m/s, 배회 반경은 m, 대기 시간은 ms다.
	// Waypoints/Actions와 seed는 정의이고 현재 목적지·경과 시간·난수 진행 상태는 GameRoom 쪽 실행 상태에 따로 있다.
	// clip 이름·반복·재생률·blend는 Client 표현 문서가 소유하며 이 정의로 전달하지 않는다.
	struct WORLD_NPC_BEHAVIOR_DESCRIPTOR final
	{
		NPC_BEHAVIOR_MODE eMode = NPC_BEHAVIOR_MODE::STATIONARY;
		NPC_ROUTE_MODE eRouteMode = NPC_ROUTE_MODE::LOOP;
		NPC_ACTION_SELECTION eActionSelection =
			NPC_ACTION_SELECTION::SEQUENCE;
		float fMoveSpeed = 1.f;
		float fWanderRadius = 0.f;
		std::uint32_t iRandomSeed = 1u;
		std::uint32_t iStartDelayMs = 0u;
		std::uint32_t iIdleMinMs = 0u;
		std::uint32_t iIdleMaxMs = 0u;
		std::string strLookTargetPlacementId;
		std::vector<WORLD_NPC_BEHAVIOR_WAYPOINT> Waypoints;
		std::vector<WORLD_NPC_BEHAVIOR_ACTION> Actions;
	};

	// 트리거 하나의 동작 정의다. eKind에 따라 이동 수치 또는 목표 ID를 소비하며 현재 진행률을 보관하지 않는다.
	// MOVE_PLAYER의 목표/arc 높이는 월드 m, 기간은 초, facing yaw는 degree다. TrackSamples는 등반 때 사용하는 소유 vector다.
	struct WORLD_TRIGGER_ACTION
	{
		WORLD_TRIGGER_ACTION_KIND eKind = WORLD_TRIGGER_ACTION_KIND::END;
		float fTargetX = 0.f;
		float fTargetY = 0.f;
		float fTargetZ = 0.f;
		float fDurationSeconds = 0.f;
		float fArcHeight = 0.f;
		WORLD_TRIGGER_MOVE_STYLE eMoveStyle = WORLD_TRIGGER_MOVE_STYLE::DIRECT;
		float fFacingYawDegrees = 0.f;
		std::vector<WORLD_TRIGGER_MOVE_SAMPLE> TrackSamples;
		// END는 도착 시 HUD를 변경하지 않는다는 뜻이다. 명시적인 NONE은 HUD 모드를 해제하는 값이므로 서로 다르다.
		LostArk::Shared::KOUKU_HUD_MODE eKoukuHudModeOnArrival = LostArk::Shared::KOUKU_HUD_MODE::END;
		LostArk::Shared::WORLD_ID eTargetWorldId =
			LostArk::Shared::WORLD_ID::END;
		// 동작별로 SpawnGroup/encounter/sequence ID를 담는다. CHANGE_LEVEL에서는 도착 월드의 선택적 placement ID다.
		// 이 로더의 ID 문법 검사만으로 대상 존재나 실행 성공이 보장되지는 않는다. 실제 처리 조건은 동작별 소비 경로에서 확인한다.
		std::string strTargetId;
	};

	// 월드에 놓을 인스턴스의 stable placement ID와 정의 참조, 월드 위치 m/yaw degree다. 포인터나 vector index를 저장 ID로 쓰지 않는다.
	// TRIGGER_BOX/COLLISION_BOX는 중심과 각 축 반길이 m를 사용한다. 배치 데이터는 실행 entity나 trigger의 현재 상태와 분리된다.
	struct WORLD_BOOTSTRAP_PLACEMENT
	{
		std::string strPlacementId;
		WORLD_BOOTSTRAP_KIND eKind = WORLD_BOOTSTRAP_KIND::END;
		std::string strArchetypeId;
		std::string strEncounterId;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
		float fHalfExtentX = 0.f;
		float fHalfExtentY = 0.f;
		float fHalfExtentZ = 0.f;
		// 저작된 일회성 설정이다. 현재 제품 ServerTriggerSystem은 기본적으로 이 값을 해제해 플레이어별 재진입을 허용하고, 테스트만 보존을 선택한다.
		bool isTriggerOnce = true;
		// true이면 일반 자동 진입 실행을 막는다. false라도 자동 실행되는 것은 아니며 Fires_OnEntry의 월드/종류/ID 규칙을 따른다.
		// 망원경 타격 영역 등은 입장과 G 경로 모두에서 제외되므로 이 flag 하나로 실행 정책을 설명할 수 없다.
		bool requiresInteract = false;
		// 저장 형식은 목록이지만 Load는 최대 한 동작만 허용하고 enabled trigger에는 정확히 하나를 요구한다.
		std::vector<WORLD_TRIGGER_ACTION> TriggerActions;
		bool bHasNpcBehavior = false;
		WORLD_NPC_BEHAVIOR_DESCRIPTOR NpcBehavior;
		bool isEnabled = true;
	};

	// 카드 미로 Seto 36개 경로의 절대 월드 시작/끝 위치 m와 지연/이동 기간 ms다. Client 자산 경로는 포함하지 않는다.
	// instanceId는 함께 게시된 World sequence 목록에 있어야 하며 GameRoom의 논리 판정과 연출 연결에 사용한다.
	struct CARD_MAZE_MARCH_LANE final
	{
		std::string instanceId;
		std::uint32_t delayMs = 0u, durationMs = 0u;
		float startX = 0.f, startY = 0.f, startZ = 0.f;
		float endX = 0.f, endY = 0.f, endZ = 0.f;
	};

	// Mario stage/layout의 원본 구슬 정의다. slot은 해당 WorldSequence binding 순서와 터진 구슬 bit mask의 bit 위치를 연결한다.
	// color는 0=빨강, 1=파랑, 2=노랑이며 XYZ는 구슬 밑면 기준의 절대 월드 m다. 현재 파괴 상태는 GameRoom/플레이어 실행 상태에 있다.
	struct MARIO_SOURCE_BALL final
	{
		std::uint8_t stage = 0u, layout = 0u, slot = 0u, color = 0u;
		std::uint64_t placementId = 0u;
		float x = 0.f, y = 0.f, z = 0.f;
	};

	// publisher가 배치 기준 bounce track을 Server 충돌용 수치로 게시한 정의다. Server는 모델 자산을 직접 읽지 않는다.
	// XYZ는 밑면의 월드 위치 m, radius는 m, key의 offsetY는 배치에 더하는 m이며 시간은 ms다. keys vector가 표본을 소유한다.
	struct MARIO_BOUNCING_BALL final
	{
		struct KEY
		{
			float timeMs = 0.f, offsetY = 0.f;
		};
		std::uint64_t placementId = 0u;
		float x = 0.f, y = 0.f, z = 0.f, radius = 0.f;
		float durationMs = 0.f, playbackSpeed = 1.f;
		std::vector<KEY> keys;
		// GameRoom의 Mario 충돌 검사에서 사용하는 주기적 선형 보간이다. Load를 통과한 양수 duration/속도와 정렬된 두 개 이상의 key를 전제로 한다.
		float Sample_OffsetY(double clockMs) const;
		// 두 시각 사이 key 극값과 반복 경계를 포함해 수직 offset의 최솟값/최댓값을 덮어쓴다. 호출자는 0 이상의 유한한 시각과 fromMs <= toMs를 보장한다.
		// GameRoom이 여기에 위치와 반지름을 더해 30Hz 갱신 사이를 지나간 구슬의 플레이어 접촉을 검사한다.
		void Sample_SweptOffsetY(double fromMs, double toMs, float& low, float& high) const;
	};

	// GameRoom 초기화에서 게시된 World/<WORLD_ID>.worldbootstrap을 읽어 배치와 미니게임 정의를 소유한다.
	// 저작 JSON 편집이나 entity 생성은 하지 않는다. GameRoom이 로드 결과를 navigation/trigger/실행 객체 준비에 전달한다.
	class CWorldBootstrap final
	{
	public:
		// 파일 형식·값·일부 내부 참조를 임시 목록에서 검사한 뒤 멤버로 옮긴다. 명시적인 false는 기존 목록/Area/revision을 보존하고 status만 바꾼다.
		// Catalog·SpawnGroup·목적 월드와의 연결은 후속 초기화와 동작 실행 경로도 확인해야 한다. 전체 참조 검증이나 메모리 할당 예외의 rollback을 보장하지 않는다.
		bool Load(LostArk::Shared::WORLD_ID worldId);

		// Room과 하위 시스템이 읽는 차용 참조다. 내부 동기화는 없으며 동시 Load/파괴를 피하고, 성공한 재로드 뒤 원소 참조를 재사용하지 않는다.
		const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& Get_Placements() const
		{
			return m_Placements;
		}

		const std::string& Get_AreaId() const
		{
			return m_strAreaId;
		}
		const std::string& Get_Status() const
		{
			return m_strStatus;
		}
		std::uint32_t Get_Revision() const
		{
			return m_iRevision;
		}
		const std::vector<std::string>& Get_SequenceInstanceIds() const
		{
			return m_SequenceInstanceIds;
		}
		const std::vector<CARD_MAZE_MARCH_LANE>& Get_CardMazeLanes() const
		{
			return m_CardMazeLanes;
		}
		const std::vector<MARIO_SOURCE_BALL>& Get_MarioBalls() const
		{
			return m_MarioBalls;
		}
		const std::vector<MARIO_BOUNCING_BALL>& Get_MarioBouncingBalls() const
		{
			return m_MarioBouncingBalls;
		}

	// 게시 정의의 소유 저장소다. m_strStatus는 마지막 시도 결과이므로 Load 실패 뒤에도 정의 목록은 이전 성공본일 수 있다.
	private:
		std::vector<WORLD_BOOTSTRAP_PLACEMENT> m_Placements;
		std::vector<std::string> m_SequenceInstanceIds;
		std::vector<CARD_MAZE_MARCH_LANE> m_CardMazeLanes;
		std::vector<MARIO_SOURCE_BALL> m_MarioBalls;
		std::vector<MARIO_BOUNCING_BALL> m_MarioBouncingBalls;
		std::string m_strAreaId;
		std::string m_strStatus;
		std::uint32_t m_iRevision = 0;
	};
}
