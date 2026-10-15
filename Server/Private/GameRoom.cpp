// 룸의 소유 상태와 아래 멤버 함수 선언, catalog·simulation 하위 시스템 형식을 제공한다.
#include "GameRoom.h"

// 등록 명령의 세션 ID 확인, 내구도 snapshot 송신 실패의 종료 요청과 outbound 계측 조회를 사용한다.
#include "ClientSession.h"
// Uses_RaidStaggerPolicy로 보스의 공통 레이드 무력화 정책 적용 여부를 판정한다.
#include "ServerCombatHitRuntime.h"

// 발탄 decision trace 요청·응답과 각 명령 payload, snapshot·콜로세움 wire 상수를 사용한다.
#include "Network/PacketMessages.h"
// 이 파일은 응답 값만 만들며 PacketWriter로 직접 직렬화하지 않는다. 기존 include 관계를 유지한다.
#include "Network/PacketWriter.h"
// 충돌 처리는 룸의 ServerCollisionSystem을 통해 수행하며 이 헤더의 계약 API를 직접 호출하지 않는다.
#include "Gameplay/WorldCollisionContract.h"

// 큐 삭제·ID 검색·개수 집계와 용량/비율의 최솟값·최댓값을 계산한다.
#include <algorithm>
// 아래에서는 GameRoom_Internal.h의 Mario 정적 배열을 조회하며 std::array를 직접 선언하지 않는다.
#include <array>
// tick 경과는 steady_clock으로, 로그의 Unix 시각은 system_clock으로 구분해 측정한다.
#include <chrono>
// 이 본문에서 문자 분류 함수를 직접 호출하지 않는 기존 include다.
#include <cctype>
// 고정 delta가 유한한 초 단위 값인지 std::isfinite로 검사한다.
#include <cmath>
// tick·sequence·통계와 HP 비율 중간 곱의 고정 폭 정수를 사용한다.
#include <cstdint>
// 이 본문에서 C 런타임 환경/할당 함수를 직접 호출하지 않는 기존 include다.
#include <cstdlib>
// trigger·성능·최초 룸 실패 진단을 표준 출력과 오류 출력으로 보낸다.
#include <iostream>
// 성능 로그의 stable ID와 이유 문자열을 std::quoted로 기록한다.
#include <iomanip>
// 한 번의 룸 성능 표본을 문자열 스트림으로 조립한다.
#include <sstream>
// tick의 예약 0을 건너뛰는 wrap과 투사체 피해량 uint32 상한을 사용한다.
#include <limits>
// 이 본문에서 nothrow/bad_alloc을 직접 사용하지 않는 기존 include다.
#include <new>
// 바닥 제거에 연결된 navigation condition ID를 중복 없이 모은다.
#include <set>
// 실패 출처를 호출 동안 차용하고 Mario 도착/출구 ID를 정적 문자열과 비교한다.
#include <string_view>
// 큐·응답·후보의 소유 값을 이동하고 소비한 성능 진단을 exchange로 비운다.
#include <utility>

// 입력 coalescing, Mario lane/행동 정리와 μs 변환 helper를 공유한다.
#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

// ServerApp의 시작·개인 룸 입장·콜로세움 준비가 아직 공개하지 않은 룸을 구성한다.
// 전달된 불변 catalog는 공유 소유하고 없으면 자체 로드한다. 취소 atomic 포인터는 생성 중만 차용한다.
// 필수 준비 실패는 m_isReady=false와 진단을 남긴 객체로 반환하며, 호출자가 Is_Ready를 확인해 폐기한다.
// 이미 로드한 멤버는 객체 수명에 따라 정리한다. 할당/로드 예외를 이 생성자 전체에서 잡지는 않는다.
LostArk::Server::CGameRoom::CGameRoom(const LostArk::Shared::WORLD_ID worldId,
	std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration, const std::atomic_bool* pPreparationCancelled)
	: m_eWorldId(worldId)
{

	// 큰 로드 단계 사이에서 협력 취소를 관측한다. 개별 동기 I/O를 이 lambda가 중단하지는 않는다.
	const auto cancelled = [this, pPreparationCancelled]()
	{
		if (!pPreparationCancelled || !pPreparationCancelled->load(std::memory_order_relaxed))
			return false;
		m_strStatus = "Room preparation cancelled";
		return true;
	};
	if (cancelled())
		return;
	if (!LostArk::Shared::Is_Known_World_Id(worldId))
	{
		m_strStatus = "Unknown room world ID";
		return;
	}

	// 배치 bootstrap을 먼저 읽고 같은 world에 필요한 gameplay·아이템·탈것·칭호·보상 정의를 준비한다.
	if (!m_WorldBootstrap.Load(worldId))
	{
		m_strStatus = m_WorldBootstrap.Get_Status();
		return;
	}
	if (cancelled())
		return;
	if ((nullptr != initialGameplayGeneration && !m_GameplayCatalog.Initialize(initialGameplayGeneration)) ||
		(nullptr == initialGameplayGeneration && !m_GameplayCatalog.Load()))
	{
		m_strStatus = m_GameplayCatalog.Get_Status();
		return;
	}
	if (cancelled())
		return;
	if (!m_ItemCatalog.Load())
	{
		m_strStatus = m_ItemCatalog.Get_Status();
		return;
	}
	if (cancelled())
		return;
	if (!m_VehicleCatalog.Load())
	{
		m_strStatus = m_VehicleCatalog.Get_Status();
		return;
	}
	if (cancelled())
		return;
	if (!m_HonorTitleCatalog.Load())
	{
		m_strStatus = m_HonorTitleCatalog.Get_Status();
		return;
	}
	if (cancelled())
		return;
	if (!m_ValtanClearRewards.Load())
	{
		m_strStatus = m_ValtanClearRewards.Get_Status();
		return;
	}
	if (cancelled())
		return;

	// SpawnGroup 정의와 실행 상태를 연결한 뒤 이 룸의 Esther 상태를 초기화한다.
	if (!m_SpawnGroupBootstrap.Load(worldId))
	{
		m_strStatus = m_SpawnGroupBootstrap.Get_Status();
		return;
	}
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, m_strStatus))
		return;
	if (cancelled())
		return;
	m_EstherSkillSystem.Initialize(worldId);
	// 아래 일곱 world는 navigation이 필수다. Bern도 grid 로드 실패를 입장 실패로 남겨
	// 보행 높이·계단 경로를 grid 없이 계속 처리하는 경로로 내려가지 않게 한다.
	if ((LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId || LostArk::Shared::WORLD_ID::TRAINING_GROUND == worldId ||
			LostArk::Shared::WORLD_ID::CHARACTER_SELECT_ARENA == worldId ||
			LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA == worldId || LostArk::Shared::WORLD_ID::MAHARAKA == worldId ||
			LostArk::Shared::WORLD_ID::COLOSSEUM == worldId || LostArk::Shared::WORLD_ID::BERN == worldId) &&
		!m_ServerNavigation.Load(m_WorldBootstrap.Get_AreaId()))
	{
		m_strStatus = m_ServerNavigation.Get_Status();
		return;
	}
	if (cancelled())
		return;
	if (!m_NpcBehaviorRuntime.Validate_Admission(m_WorldBootstrap.Get_Placements(), m_ServerNavigation, m_strStatus))
	{
		return;
	}
	m_ServerTriggerSystem.Set_WorldId(worldId);
#ifdef _DEBUG
	// Debug에서는 쿠크 Book1/Book2와 발탄 Stage_1/Stage_2의 자동 웨이브 trigger를 억제한다.
	// 해당 시험은 F1 명령이 담당하며 Release에서는 이 설정을 호출하지 않는다.
	m_ServerTriggerSystem.Set_SuppressWaveMonsterTriggers(true);
#endif

	// 로그 callback은 값을 즉시 출력한다. 지면 callback은 이 룸을 차용해 world XZ(m)의 높이 Y(m)를 구한다.
	// callback은 멤버 trigger system에 저장되므로 룸이 살아 있는 동안만 사용한다. 샘플 실패에서는 outY를 유지한다.
	m_ServerTriggerSystem.Set_FireLog(
		[](const std::string& line)
		{
			std::cout << line << '\n';
		});
	m_ServerTriggerSystem.Set_GroundSampler(
		[this](const float x, const float z, float& outY)
		{
			SERVER_NAV_POINT point{};
			if (!m_ServerNavigation.Is_Loaded() || !m_ServerNavigation.Sample_Position(x, z, point))
			{
				return false;
			}
			outY = point.y;
			return true;
		});
	if (!m_ServerTriggerSystem.Initialize(
			m_WorldBootstrap.Get_Placements(), m_strStatus, LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId))
	{
		return;
	}
	if (cancelled())
		return;
	if (!m_ServerCollisionSystem.Initialize(m_WorldBootstrap.Get_Placements(), m_strStatus))
	{
		return;
	}

	// 충돌 상태를 만든 뒤 활성 player spawn이 차단 상자와 겹치는지 확인한다. 하나라도 겹치면 입장을 허용하지 않는다.
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement : m_WorldBootstrap.Get_Placements())
	{
		if (placement.isEnabled && WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind &&
			!m_ServerCollisionSystem.Is_PlayerSpawnClear(placement))
		{
			m_strStatus = "Player spawn overlaps a collision box: " + placement.strPlacementId;
			return;
		}
	}

	// 발탄만 파괴 그래프·pickup·기둥 상태를 준비하고 그 변경이 참조하는 충돌/navigation ID를 대조한다.
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId)
	{
		if (!m_WorldDestructionBootstrap.Load_ValtanArena())
		{
			m_strStatus = m_WorldDestructionBootstrap.Get_Status();
			return;
		}
		if (!m_WorldDestructionRuntime.Initialize(m_WorldDestructionBootstrap.Get_DescriptorGraph(), m_strStatus, 1u))
		{
			return;
		}
		if (!Initialize_WorldPickups())
			return;
		// 기둥은 반복 생성/제거 가능한 encounter prop 상태다. 선택 문서가 비어 있으면 생략하고,
		// 값이 있으면 이 룸이 소유할 정확히 한 set만 초기화한다. cover 원과 위치는 게시 descriptor를 소비한다.
		std::vector<ENCOUNTER_PROP_SET_DESCRIPTOR> propSets;
		if (!Load_EncounterPropSets(m_eWorldId, propSets, m_strStatus))
			return;
		if (!propSets.empty())
		{
			if (propSets.size() != 1u || !m_EncounterPropRuntime.Initialize(propSets.front(), m_strStatus, 1u))
			{
				m_strStatus = "World declares an encounter prop set this room cannot own";
				return;
			}
		}
		std::set<std::string> voidConditionIds;
		for (const WORLD_DESTRUCTION_MUTATION_DESCRIPTOR& mutation :
			m_WorldDestructionBootstrap.Get_DescriptorGraph().Mutations)
		{
			if ((!mutation.strCollisionStateId.empty() &&
					!m_ServerCollisionSystem.Has_CollisionStateTarget(mutation.strCollisionStateId)) ||
				(!mutation.strNavigationStateId.empty() &&
					!m_ServerNavigation.Has_Condition(mutation.strNavigationStateId)))
			{
				m_strStatus = "World destruction dynamic state reference is unknown: " + mutation.strMutationId;
				return;
			}
			// 바닥을 제거한다고 표시한 mutation의 navigation condition을 모은다.
			// 전투 중 상태 변경 전에 void 판정이 사용할 condition 집합을 navigation에 전달한다.
			if (mutation.bRemovesGround)
				voidConditionIds.insert(mutation.strNavigationStateId);
		}
		if (!m_ServerNavigation.Set_VoidConditions(voidConditionIds, m_strStatus))
		{
			return;
		}
	}
	if (cancelled())
		return;
	if (!Initialize_WorldEntities())
		return;
	if (nullptr == Find_AvailablePlayerSpawn())
	{
		m_strStatus = "World bootstrap has no enabled player spawn";
		return;
	}

	if (cancelled())
		return;

	// 필수 entity/spawn 준비를 끝낸 뒤 명령을 받을 수 있게 한다. Guide는 자체 실패를 로그로 남기는
	// 부가 초기화이며 그 실패 때문에 이 ready 값을 다시 내리지 않는다.
	m_isReady = true;
	Initialize_Guide();
	m_strStatus = m_WorldBootstrap.Get_Status();
}

// ServerApp의 정리/보상 등 bool만 필요한 경로가 사용한다. 명령을 값으로 받아 상세 enqueue에 이동하며
// 중복 LEAVE도 수락으로 취급한다. 실제 gameplay 명령의 실행 성공을 뜻하지는 않는다.
bool LostArk::Server::CGameRoom::Enqueue(ROOM_COMMAND command)
{
	return Is_AcceptedRoomCommandEnqueueResult(Enqueue_Detailed(std::move(command)));
}

// ServerApp의 수신·입장·종료 경로가 명령을 큐에 넘기는 경계다. 등록 session의 identity를 먼저 검사하고
// command mutex 아래 ready/봉인·정리·합치기·용량을 판정한다. gameplay 멤버는 여기서 실행하지 않는다.
// 수락한 payload와 등록용 shared_ptr는 큐가 소유한다. 명시적 거절과 할당 예외는 구분하며 예외 rollback은 제공하지 않는다.
LostArk::Server::ROOM_COMMAND_ENQUEUE_RESULT LostArk::Server::CGameRoom::Enqueue_Detailed(ROOM_COMMAND command)
{
	if (command.iSessionId == INVALID_SESSION_ID)
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND;
	if (command.eType == ROOM_COMMAND_TYPE::REGISTER_SESSION &&
		(nullptr == command.pSession || command.pSession->Get_SessionId() != command.iSessionId))
	{
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND;
	}

	std::scoped_lock lock{m_CommandMutex};
	if (!m_isReady)
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY;
	if (!m_acceptsCommands)
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_SEALED;

	if (ROOM_COMMAND_TYPE::LEAVE == command.eType)
	{
		// 퇴장 정리는 일반 입력 큐의 용량과 경쟁하지 않도록 별도 큐에 둔다. 같은 session은 대기/처리 중
		// 하나만 유지하고 아직 공유 입력 큐에 남은 명령을 지운다. Tick이 이미 가져간 지역 명령까지 지우지는 않는다.
		if (!m_QueuedCleanupSessionIds.insert(command.iSessionId).second)
		{
			++m_PerformanceMetrics.iDeduplicatedCleanupCommandCount;
			return ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP;
		}
		const std::size_t oldCommandCount = m_InboundCommands.size();
		std::erase_if(m_InboundCommands,
			[sessionId = command.iSessionId](const ROOM_COMMAND& queued)
			{
				return queued.iSessionId == sessionId;
			});
		m_PerformanceMetrics.iCancelledCommandCountByCleanup += oldCommandCount - m_InboundCommands.size();
		m_CleanupCommands.push_back(std::move(command));
		m_PerformanceMetrics.iCleanupIngressHighWatermark =
			(std::max)(m_PerformanceMetrics.iCleanupIngressHighWatermark, m_CleanupCommands.size());
		return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED;
	}
	if (m_QueuedCleanupSessionIds.contains(command.iSessionId))
	{
		// 대기 또는 처리 중인 LEAVE 뒤로 같은 session의 수신 명령이 다시 쌓이지 않게 한다.
		++m_PerformanceMetrics.iCancelledCommandCountByCleanup;
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_PENDING_CLEANUP;
	}

	// MOVE/UPDATE_SKILL_AIM은 같은 session의 마지막 호환 명령을 합칠 수 있을 때 먼저 교체한다.
	// 합칠 수 없으면 768개, 그 밖의 일반 명령은 960개의 공유 큐 깊이 상한을 적용한다.
	if (Is_BestEffortCommand(command.eType))
	{
		if (Try_RemoveCoalescedCommand(m_InboundCommands, command))
		{
			if (ROOM_COMMAND_TYPE::MOVE == command.eType)
				++m_PerformanceMetrics.iCoalescedMoveCommandCount;
			else
				++m_PerformanceMetrics.iCoalescedAimCommandCount;
			m_InboundCommands.push_back(std::move(command));
			return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED;
		}
		if (m_InboundCommands.size() >= MAX_BEST_EFFORT_COMMAND_COUNT)
		{
			++m_PerformanceMetrics.iDroppedBestEffortCommandCount;
			return ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT;
		}
	}
	else if (m_InboundCommands.size() >= MAX_RELIABLE_COMMAND_COUNT)
	{
		++m_PerformanceMetrics.iRejectedReliableCommandCount;
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY;
	}

	m_InboundCommands.push_back(std::move(command));
	m_PerformanceMetrics.iIngressHighWatermark =
		(std::max)(m_PerformanceMetrics.iIngressHighWatermark, m_InboundCommands.size());
	return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED;
}

// ServerApp이 받은 enum에 현재 큐 깊이·봉인·최초 실패를 붙인다. mutex로 조회를 보호하지만
// enqueue와 이 조회는 별도 호출이므로 깊이는 원래 결과 판정 순간의 값과 다를 수 있다.
std::string LostArk::Server::CGameRoom::Describe_EnqueueResult(const ROOM_COMMAND_ENQUEUE_RESULT result) const
{
	std::scoped_lock lock{m_CommandMutex};
	std::string description = "enqueueResult=" + std::string{To_RoomCommandEnqueueResultName(result)} +
		" worldId=" + std::to_string(static_cast<std::uint16_t>(m_eWorldId)) +
		" ingressDepth=" + std::to_string(m_InboundCommands.size()) +
		" reliableCapacity=" + std::to_string(MAX_RELIABLE_COMMAND_COUNT) +
		" acceptsCommands=" + (m_acceptsCommands ? "true" : "false");
	if (!m_isReady)
		description += " roomReady=false";
	if (!m_RuntimeFailure.strSource.empty())
	{
		description += " firstFailureTick=" + std::to_string(m_RuntimeFailure.iServerTick) +
			" firstFailureSource=" + m_RuntimeFailure.strSource + " firstFailureDetail=" + m_RuntimeFailure.strDetail;
	}
	return description;
}

// 쿠크 명령 실패 진단과 검증 코드가 최초 룸 실패를 복사해 읽는다. 기록이 없으면 false이며 출력은 유지한다.
bool LostArk::Server::CGameRoom::Try_GetRuntimeFailure(SERVER_ROOM_RUNTIME_FAILURE& outFailure) const
{
	std::scoped_lock lock{m_CommandMutex};
	if (m_RuntimeFailure.strSource.empty())
		return false;
	outFailure = m_RuntimeFailure;
	return true;
}

// 룸 simulation의 복구 불가능한 처리 실패가 ready를 내리고 최초 tick·출처·상세만 보관한다.
// 명령 조회와 겹치는 기록은 command mutex로 보호한다. 이미 변경된 gameplay 상태를 rollback하거나 큐를 비우지는 않는다.
void LostArk::Server::CGameRoom::Mark_RuntimeFailure(const std::string_view source)
{
	SERVER_ROOM_RUNTIME_FAILURE failure{};
	{
		std::scoped_lock lock{m_CommandMutex};
		if (!m_RuntimeFailure.strSource.empty())
		{
			m_isReady = false;
			return;
		}
		m_RuntimeFailure.iServerTick = m_iServerTick;
		m_RuntimeFailure.strSource = source.empty() ? "unspecified-room-runtime-failure" : std::string{source};
		m_RuntimeFailure.strDetail = m_strStatus.empty() ? m_RuntimeFailure.strSource : m_strStatus;
		m_isReady = false;
		failure = m_RuntimeFailure;
	}
	std::cerr << "[RoomRuntimeFailure] World=" << static_cast<std::uint16_t>(m_eWorldId)
		<< " Tick=" << failure.iServerTick << " Source=" << failure.strSource << " Detail=" << failure.strDetail
		<< '\n';
}

// 성능 로그와 검증 경로가 enqueue/Tick 통계의 일관된 복사본을 받는다. 반환 후의 추가 측정은 그 복사본에 반영되지 않는다.
LostArk::Server::SERVER_ROOM_PERFORMANCE_METRICS LostArk::Server::CGameRoom::Get_PerformanceMetrics() const
{
	std::scoped_lock lock{m_CommandMutex};
	return m_PerformanceMetrics;
}

// ServerApp의 Tick_GameplaySimulations가 Tick 직후 룸 worker에서 소비한다. 최신 한 줄을 이동해 반환하고
// 멤버를 비운다. 별도 mutex가 없으므로 Tick과 동시에 다른 스레드에서 호출하지 않는다.
std::string LostArk::Server::CGameRoom::Take_PerformanceDiagnostic()
{
	return std::exchange(m_strPendingPerformanceDiagnostic, {});
}

// ServerApp의 제어 이벤트 처리가 룸 worker에서 최신 발탄 선택 기록을 wire 값으로 복사한다.
// 잘못된 world/보스 부재/새 기록 없음도 typed 결과를 만든 true다. false는 기록 계약 오류이며 이전 응답은 유지한다.
// brain의 포인터는 이 호출 동안만 차용하고 최종 응답은 값을 소유한다. 직렬화·송신은 ServerApp이 맡는다.
bool LostArk::Server::CGameRoom::Build_ValtanDecisionTraceResponse(
	const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request,
	LostArk::Shared::S2C_VALTAN_DECISION_TRACE_RESPONSE& outResponse, std::string& status) const
{
	using namespace LostArk::Shared;
	S2C_VALTAN_DECISION_TRACE_RESPONSE staged{};
	staged.iRequestSequence = request.iRequestSequence;
	staged.strBossPlacementId = request.strBossPlacementId;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId)
	{
		staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_WRONG_WORLD;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}

	// 요청 placement와 발탄 archetype/encounter가 일치하는 현재 entity를 찾는다. 최신 기록이 없거나
	// 요청 sequence보다 새롭지 않으면 trace 본문 없이 결과만 반환한다.
	const auto bossIter = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::BOSS == entity.eKind && request.strBossPlacementId == entity.strPlacementId &&
				"BOSS_VALTAN" == entity.strArchetypeId && "ENCOUNTER_VALTAN" == entity.strEncounterId;
		});
	if (m_WorldEntities.end() == bossIter)
	{
		staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_NO_BOSS;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}

	const VALTAN_DECISION_TRACE* trace = m_ValtanBrain.Get_LatestDecisionTrace();
	if (nullptr == trace)
	{
		staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::NO_TRACE;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}
	if (trace->iTraceSequence <= request.iAfterTraceSequence)
	{
		staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::UNCHANGED;
		outResponse = std::move(staged);
		status.clear();
		return true;
	}
	if (trace->Candidates.size() > MAX_VALTAN_DECISION_TRACE_CANDIDATES ||
		m_ValtanDecisionTraceRevision.iBossEntityId != bossIter->iNetEntityId ||
		m_ValtanDecisionTraceRevision.strBossPlacementId != bossIter->strPlacementId ||
		m_ValtanDecisionTraceRevision.iTraceSequence != trace->iTraceSequence ||
		!m_ValtanDecisionTraceRevision.DefinitionRevision.Is_Valid())
	{
		// trace는 판정 당시의 값을 가진 불변 기록이다. DefinitionRevision은 관측용 identity이며
		// 살아 있는 catalog를 붙잡는 pin이 아니므로 과거 세대가 회수되어도 최신 기록을 조회할 수 있다.
		status = "Valtan decision trace revision metadata is invalid";
		return false;
	}

	// 내부 selector enum과 wire enum을 명시적으로 대응시켜 알 수 없는 새 값이 정상 값으로 둔갑하지 않게 한다.
	const auto mapSource = [](const VALTAN_DECISION_SOURCE source, VALTAN_DECISION_TRACE_SOURCE& wire)
	{
		switch (source)
		{
		case VALTAN_DECISION_SOURCE::NONE:
			wire = VALTAN_DECISION_TRACE_SOURCE::NONE;
			return true;
		case VALTAN_DECISION_SOURCE::INTRO:
			wire = VALTAN_DECISION_TRACE_SOURCE::INTRO;
			return true;
		case VALTAN_DECISION_SOURCE::FORCED_HEALTH_BAR:
			wire = VALTAN_DECISION_TRACE_SOURCE::FORCED_HEALTH_BAR;
			return true;
		case VALTAN_DECISION_SOURCE::FORCED_AUDITION:
			wire = VALTAN_DECISION_TRACE_SOURCE::FORCED_AUDITION;
			return true;
		case VALTAN_DECISION_SOURCE::ORDERED:
			wire = VALTAN_DECISION_TRACE_SOURCE::ORDERED;
			return true;
		case VALTAN_DECISION_SOURCE::WEIGHTED:
			wire = VALTAN_DECISION_TRACE_SOURCE::WEIGHTED;
			return true;
		case VALTAN_DECISION_SOURCE::GLOBAL:
			wire = VALTAN_DECISION_TRACE_SOURCE::GLOBAL;
			return true;
		default:
			return false;
		}
	};
	const auto mapResult = [](const VALTAN_DECISION_RESULT result, VALTAN_DECISION_TRACE_RESULT& wire)
	{
		switch (result)
		{
		case VALTAN_DECISION_RESULT::SELECTED:
			wire = VALTAN_DECISION_TRACE_RESULT::SELECTED;
			return true;
		case VALTAN_DECISION_RESULT::WAITING_FOR_INTRO_RANGE:
			wire = VALTAN_DECISION_TRACE_RESULT::WAITING_FOR_INTRO_RANGE;
			return true;
		case VALTAN_DECISION_RESULT::NO_ELIGIBLE_PATTERN:
			wire = VALTAN_DECISION_TRACE_RESULT::NO_ELIGIBLE_PATTERN;
			return true;
		case VALTAN_DECISION_RESULT::NO_VALID_TARGET:
			wire = VALTAN_DECISION_TRACE_RESULT::NO_VALID_TARGET;
			return true;
		case VALTAN_DECISION_RESULT::CATALOG_UNAVAILABLE:
			wire = VALTAN_DECISION_TRACE_RESULT::CATALOG_UNAVAILABLE;
			return true;
		case VALTAN_DECISION_RESULT::MECHANIC_RESET_REQUIRED:
			wire = VALTAN_DECISION_TRACE_RESULT::MECHANIC_RESET_REQUIRED;
			return true;
		default:
			return false;
		}
	};

	// 제외 사유는 알려진 bit만 wire mask로 옮긴다. 알 수 없는 원본 bit 또는 wire bit가 남으면 응답 구성을 거절한다.
	const auto mapExclusions = [](const std::uint32_t source, std::uint32_t& wire)
	{
		struct BIT_MAP final
		{
			std::uint32_t Source;
			std::uint32_t Wire;
		};
		static constexpr BIT_MAP MAP[] = {
			{VALTAN_EXCLUDE_WRONG_SELECTION_KIND, VALTAN_DECISION_TRACE_EXCLUDE_WRONG_SELECTION_KIND},
			{VALTAN_EXCLUDE_INTRO_ROW, VALTAN_DECISION_TRACE_EXCLUDE_INTRO_ROW},
			{VALTAN_EXCLUDE_NOT_IN_SELECTION_SET, VALTAN_DECISION_TRACE_EXCLUDE_NOT_IN_SELECTION_SET},
			{VALTAN_EXCLUDE_ARMOR_MISMATCH, VALTAN_DECISION_TRACE_EXCLUDE_ARMOR_MISMATCH},
			{VALTAN_EXCLUDE_PHASE_REQUIREMENT, VALTAN_DECISION_TRACE_EXCLUDE_PHASE_REQUIREMENT},
			{VALTAN_EXCLUDE_PHASE_RANGE, VALTAN_DECISION_TRACE_EXCLUDE_PHASE_RANGE},
			{VALTAN_EXCLUDE_HEALTH_BAR_RANGE, VALTAN_DECISION_TRACE_EXCLUDE_HEALTH_BAR_RANGE},
			{VALTAN_EXCLUDE_NO_TARGET, VALTAN_DECISION_TRACE_EXCLUDE_NO_TARGET},
			{VALTAN_EXCLUDE_BELOW_MINIMUM_RANGE, VALTAN_DECISION_TRACE_EXCLUDE_BELOW_MINIMUM_RANGE},
			{VALTAN_EXCLUDE_ABOVE_MAXIMUM_RANGE, VALTAN_DECISION_TRACE_EXCLUDE_ABOVE_MAXIMUM_RANGE},
			{VALTAN_EXCLUDE_COOLDOWN, VALTAN_DECISION_TRACE_EXCLUDE_COOLDOWN},
			{VALTAN_EXCLUDE_SOFT_REPEAT_BLOCKED, VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_BLOCKED},
			{VALTAN_EXCLUDE_SOFT_REPEAT_RELAXED, VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_RELAXED},
			{VALTAN_EXCLUDE_DISABLED, VALTAN_DECISION_TRACE_EXCLUDE_DISABLED},
			{VALTAN_EXCLUDE_UNRESOLVED_DEFINITION, VALTAN_DECISION_TRACE_EXCLUDE_UNRESOLVED_DEFINITION}};
		std::uint32_t known = 0u;
		wire = VALTAN_DECISION_TRACE_EXCLUDE_NONE;
		for (const BIT_MAP& bit : MAP)
		{
			known |= bit.Source;
			if (0u != (source & bit.Source))
				wire |= bit.Wire;
		}
		return 0u == (source & ~known) && 0u == (wire & ~VALTAN_DECISION_TRACE_KNOWN_EXCLUSION_MASK);
	};

	// 선택 직전 상태·난수·누적 가중치와 후보별 제외 사유를 지역 응답에 복사한다. 모든 enum/bit 검사를 마친 뒤 출력과 교체한다.
	VALTAN_DECISION_TRACE_WIRE& wire = staged.Trace;
	wire.iTraceSequence = trace->iTraceSequence;
	wire.iServerTick = trace->iServerTick;
	wire.iPatternSequenceBeforeDecision = trace->iPatternSequenceBeforeDecision;
	wire.iExpectedPatternSequence = trace->iExpectedPatternSequence;
	wire.iCurrentHp = trace->iCurrentHp;
	wire.iMaximumHp = trace->iMaximumHp;
	wire.iHealthBar = trace->iHealthBar;
	wire.iGameplayPhase = trace->iGameplayPhase;
	wire.iTargetNetEntityId = trace->iTargetNetEntityId;
	wire.fTargetDistance = trace->fTargetDistance;
	wire.isIntroPatternConsumed = trace->bIntroPatternConsumed;
	wire.iRotationStepIndex = trace->iRotationStepIndex;
	wire.strRotationId = trace->strRotationId;
	wire.strPendingPatternId = trace->strPendingPatternId;
	wire.strSelectedPatternId = trace->strSelectedPatternId;
	wire.iRawRandomInput = trace->iRawRandomInput;
	wire.iMixedRandomValue = trace->iMixedRandomValue;
	wire.iTotalWeight = trace->iTotalWeight;
	wire.iRandomTicket = trace->iRandomTicket;
	wire.isMaximumConsecutiveRelaxed = trace->bMaximumConsecutiveRelaxed;
	wire.areCandidatesTruncated = trace->bCandidatesTruncated;
	if (!mapSource(trace->eSource, wire.eSource) || !mapSource(trace->ePendingSource, wire.ePendingSource) ||
		!mapResult(trace->eResult, wire.eResult))
	{
		status = "Valtan decision trace contains an unknown selector enum";
		return false;
	}
	wire.Candidates.reserve(trace->Candidates.size());
	for (const VALTAN_DECISION_CANDIDATE_TRACE& source : trace->Candidates)
	{
		VALTAN_DECISION_TRACE_CANDIDATE_WIRE candidate{};
		candidate.strPatternId = source.strPatternId;
		if (!mapExclusions(source.iExclusionMask, candidate.iExclusionMask))
		{
			status = "Valtan decision trace contains an unknown exclusion bit";
			return false;
		}
		candidate.iAuthoredWeight = source.iAuthoredWeight;
		candidate.iEffectiveWeight = source.iEffectiveWeight;
		candidate.iCooldownRemainingTicks = source.iCooldownRemainingTicks;
		candidate.iConsecutiveUses = source.iConsecutiveUses;
		candidate.iMaximumConsecutiveUses = source.iMaximumConsecutiveUses;
		candidate.iWeightBeginInclusive = source.iWeightBeginInclusive;
		candidate.iWeightEndExclusive = source.iWeightEndExclusive;
		candidate.isSelected = source.bSelected;
		wire.Candidates.push_back(std::move(candidate));
	}
	staged.DefinitionRevision = m_ValtanDecisionTraceRevision.DefinitionRevision;
	staged.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::TRACE;
	outResponse = std::move(staged);
	status.clear();
	return true;
}

// ServerApp이 전체 룸 세대 승인을 준비할 때 필요한 실행 pin을 모으고 미사용 세대를 회수한 뒤 후보를 예약한다.
// pin 구성 실패는 예약하지 않는다. 이후 Stage 실패에서도 앞선 garbage collection까지 되돌리지는 않는다.
bool LostArk::Server::CGameRoom::Stage_GameplayGeneration(const std::uint32_t transactionSequence,
	const LostArk::Shared::GameplayDataRevision& baseRevision,
	const std::shared_ptr<const CGameplayCatalog>& candidateGeneration, std::string& status)
{
	std::vector<LostArk::Shared::GameplayDataRevision> livePins;
	if (!Build_RequiredPinnedGameplayRevisions(livePins))
	{
		status = "Room required gameplay revision pins are invalid";
		return false;
	}
	m_GameplayCatalog.Collect_Garbage(livePins);
	return m_GameplayCatalog.Stage(transactionSequence, baseRevision, candidateGeneration, status);
}

// ServerApp이 모든 룸의 준비와 참여자 승인을 확인한 fixed-step 경계에서 호출한다. 같은 sequence의
// 예약 확정 실패는 false다. 성공하면 활성 세대를 바꾸고 아래 비보스 entity의 게시 identity를 갱신한다.
bool LostArk::Server::CGameRoom::Commit_GameplayGeneration(const std::uint32_t transactionSequence) noexcept
{
	if (!m_GameplayCatalog.Commit(transactionSequence))
		return false;
	const LostArk::Shared::GameplayDataRevision& activeRevision = m_GameplayCatalog.Get_ActiveRevision();
	for (SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		// 보스는 brain이 새 활성 catalog를 처음 평가할 때까지 직전 평가 세대의 identity를 유지한다.
		// 이 차이를 통해 이미 지나간 체력선에 새로 추가/상향된 조건을 다시 대조한다. 진행 occurrence의 pin은
		// 여기서 바꾸지 않고, 비보스 entity만 즉시 새 활성 identity로 표시한다.
		if (WORLD_BOOTSTRAP_KIND::BOSS != entity.eKind)
		{
			entity.PinnedDefinitionRevision = activeRevision;
		}
	}
	return true;
}

// ServerApp의 준비/승인 실패가 같은 sequence의 예약만 해제한다. 활성 세대나 진행 occurrence는 이 함수가 바꾸지 않는다.
void LostArk::Server::CGameRoom::Abort_GameplayGeneration(const std::uint32_t transactionSequence) noexcept
{
	m_GameplayCatalog.Abort(transactionSequence);
}

// ServerApp이 소유 session binding이 사라진 Character Select 룸을 회수하기 직전에 호출한다.
// 큐·전환·session/player lookup이 모두 비었을 때 새 입력을 봉인한다. 다른 world 또는 남은 상태는 false다.
bool LostArk::Server::CGameRoom::Try_SealPrivateArenaForRetirement()
{
	using LostArk::Shared::WORLD_ID;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId)
		return false;

	// gameplay 컨테이너는 룸 worker가 소유한다. mutex는 빈 상태 판정과 수신 경로의 Enqueue 사이를 직렬화한다.
	// 이미 봉인된 룸은 true이며 이 함수가 남은 상태를 강제로 비우거나 객체를 삭제하지 않는다.
	std::scoped_lock lock{m_CommandMutex};
	if (!m_acceptsCommands)
		return true;
	if (!m_InboundCommands.empty() || !m_CleanupCommands.empty() || !m_QueuedCleanupSessionIds.empty() ||
		!m_PendingWorldTransfers.empty() || !m_Sessions.empty() || !m_Players.empty() ||
		!m_PlayerIdBySessionId.empty() || !m_PlayerIdByEntityId.empty())
	{
		return false;
	}
	m_acceptsCommands = false;
	return true;
}

// ServerApp의 일반 단독 world 전환이 source 룸에서 퇴장을 확정한다. 준비 안 됨/없는 session은 false다.
// Leave 후 lookup 제거 여부를 반환하며 target 큐 등록·binding 교체는 호출자가 처리한다.
bool LostArk::Server::CGameRoom::Commit_WorldTransferDeparture(const SESSION_ID sessionId)
{
	if (!m_isReady || !m_PlayerIdBySessionId.contains(sessionId))
		return false;

	// 룸 worker가 목적 룸의 REGISTER/ENTER를 큐에 넣은 뒤 source player를 정리한다.
	// 같은 worker가 다음 목적 룸 Tick을 실행하기 전에 기존 CClientSession player binding을 해제한다.
	// 목적 룸의 새 Join과 source Leave가 같은 session의 player binding을 동시에 소유하지 않게 하는 순서다.
	Leave(sessionId, LostArk::Shared::PLAYER_DESPAWN_REASON::LEVEL_CHANGED);
	return !m_PlayerIdBySessionId.contains(sessionId);
}

// Try_DequeueWorldTransfer가 Bern 출발 요청을 넘길 때 선박 복귀 정보를 갱신한다. 이 시점은 실제 전환 성공 전이다.
// 좌표는 world m, yaw는 degree로 복사한다. 저장량이 256개 이상이면 이전 항목 전체를 비우고 현재 항목을 넣는다.
void LostArk::Server::CGameRoom::Remember_ShipForWorldTransfer(const SERVER_WORLD_TRANSFER_REQUEST& transfer)
{
	using LostArk::Shared::WORLD_ID;
	if (WORLD_ID::BERN != m_eWorldId)
		return;
	// 유효한 선박·정박 정보가 있는 Maharaka 출발만 보관하고 나머지 목적지는 해당 session의 기억을 지운다.
	// trigger 진입/G 상호작용/Debug 등 예약 경로와 무관하게 dequeue 한 지점에서 처리한다.
	const auto playerIdIter = m_PlayerIdBySessionId.find(transfer.iSessionId);
	const auto playerIter =
		playerIdIter != m_PlayerIdBySessionId.end() ? m_Players.find(playerIdIter->second) : m_Players.end();
	if (WORLD_ID::MAHARAKA == transfer.eTargetWorldId && playerIter != m_Players.end() &&
		playerIter->second.bShipDockValid && LostArk::Shared::INVALID_VEHICLE_ID != playerIter->second.iVehicleId)
	{
		if (m_MaharakaShipReturnBySession.size() >= 256u)
			m_MaharakaShipReturnBySession.clear();
		SHIP_RETURN_STATE state{};
		state.iVehicleId = playerIter->second.iVehicleId;
		state.fDockX = playerIter->second.fShipDockX;
		state.fDockY = playerIter->second.fShipDockY;
		state.fDockZ = playerIter->second.fShipDockZ;
		state.fDockYawDegrees = playerIter->second.fShipDockYawDegrees;
		m_MaharakaShipReturnBySession[transfer.iSessionId] = state;
	}
	else
		m_MaharakaShipReturnBySession.erase(transfer.iSessionId);
}

// ServerApp이 모든 룸 Tick 뒤 룸 worker에서 FIFO 전환 요청을 꺼낸다. 비었으면 false로 출력을 유지하고,
// 성공하면 요청 소유권을 출력으로 옮긴 뒤 대표자와 파티원의 선박 기억을 갱신한다. 실제 이동 성공은 아직 확정되지 않았다.
bool LostArk::Server::CGameRoom::Try_DequeueWorldTransfer(SERVER_WORLD_TRANSFER_REQUEST& outTransfer)
{
	if (m_PendingWorldTransfers.empty())
		return false;
	outTransfer = std::move(m_PendingWorldTransfers.front());
	m_PendingWorldTransfers.pop_front();
	Remember_ShipForWorldTransfer(outTransfer);
	for (const auto member : outTransfer.PartyBatchSessionIds)
	{
		if (member == outTransfer.iSessionId)
			continue;
		SERVER_WORLD_TRANSFER_REQUEST passenger{};
		passenger.iSessionId = member;
		passenger.eTargetWorldId = outTransfer.eTargetWorldId;
		Remember_ShipForWorldTransfer(passenger);
	}
	return true;
}

// trigger 시스템의 진입/G 처리 callback이 Mario 이동의 룸 권한과 마지막 출구를 검증한다.
// 담당 lane이 아니면 USE_DEFAULT, 일시 거절이면 RETRY_WHILE_INSIDE, 기존/새 이동이 유효하면 STARTED다.
// player 복사본에서 준비한 이동만 성공 시 교체한다. 진단과 충돌 보조 상태 갱신까지 전체 rollback하는 함수는 아니다.
LostArk::Server::SERVER_TRIGGER_MOVE_ENTRY_RESULT LostArk::Server::CGameRoom::Begin_MarioTriggerMove(
	const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player, const std::uint32_t actionStartTick)
{
	using namespace LostArk::Shared;
	using Result = SERVER_TRIGGER_MOVE_ENTRY_RESULT;
	if (m_eWorldId != WORLD_ID::KOUKUSAYDON_ARENA)
		return Result::USE_DEFAULT;
	const auto lane = std::find_if(MARIO_LANES.begin(), MARIO_LANES.end(),
		[&trigger](const auto& row)
		{
			return trigger.strPlacementId == row.exit || trigger.strPlacementId == row.arrival;
		});
	if (lane == MARIO_LANES.end())
		return Result::USE_DEFAULT;
	if (player.iMarioStage != lane->stage || !player.iCurrentHp || !player.isCombatReady || player.bPatternBound ||
		player.bArenaEjectionActive || player.iAttachmentOwnerNetEntityId != INVALID_NET_ENTITY_ID ||
		player.eAction == PLAYER_ACTION_STATE::DEAD || player.eAction == PLAYER_ACTION_STATE::FALLING ||
		player.eAction == PLAYER_ACTION_STATE::GRABBED || trigger.TriggerActions.size() != 1u)
		return Result::RETRY_WHILE_INSIDE;
	if (player.TriggerMove.isActive && player.TriggerMove.strSourcePlacementId == trigger.strPlacementId)
		return Result::STARTED;

	// 같은 stage에서 다음 lane의 arrival로 이어지지 않는 exit만 마지막 출구다. 공 세 개와 복귀 지면/충돌을
	// 검사해 저작 좌표 대신 현재 유효한 복귀 좌표를 지역 action에 넣는다.
	WORLD_TRIGGER_ACTION action = trigger.TriggerActions.front();
	const bool terminal = trigger.strPlacementId == lane->exit &&
		std::none_of(MARIO_LANES.begin(), MARIO_LANES.end(),
			[&](const auto& next)
			{
				return next.stage == lane->stage && std::string_view(next.arrival) == lane->exit;
			});
	if (terminal)
	{
		if (Mario_MatchingBallCount(player) < 3u)
		{
			m_strStatus = "Break three balls of the marked colour before leaving Mario";
			return Result::RETRY_WHILE_INSIDE;
		}
		SERVER_NAV_POINT ground{};
		if (!Resolve_MarioReturnDestination(player, ground))
			return Result::RETRY_WHILE_INSIDE;
		Refresh_PlayerBlockingBodies();
		if (!m_ServerCollisionSystem.Is_PlayerPositionClear(ground.x, ground.y, ground.z, player.iNetEntityId))
			return Result::RETRY_WHILE_INSIDE;
		action.fTargetX = ground.x;
		action.fTargetY = ground.y;
		action.fTargetZ = ground.z;
	}
	SERVER_PLAYER candidate = player;
	Reset_MarioContactAction(candidate);
	if (!CServerTriggerSystem::Begin_MovePlayer(candidate, action, actionStartTick))
		return Result::RETRY_WHILE_INSIDE;
	candidate.TriggerMove.strSourcePlacementId = trigger.strPlacementId;
	// 복사본의 이동 시작이 성공한 뒤에만 이전 행동이 소유한 지연 combat object를 취소하고 player를 교체한다.
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player = std::move(candidate);
	return Result::STARTED;
}

// trigger callback이 stable target ID를 sequence 송신·SpawnGroup·encounter 활성화에 전달한다.
// 지원하지 않는 종류는 false다. Debug 발탄 후처리의 false는 이미 성공한 encounter 활성화까지 취소하지 않는다.
bool LostArk::Server::CGameRoom::Activate_TriggerTarget(
	const WORLD_TRIGGER_ACTION_KIND kind, const std::string& targetId)
{
	if (WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE == kind)
	{
		// 폐기된 sequence 등의 거절을 false로 전달해 일회성 trigger를 성공 처리하지 않게 한다.
		return Broadcast_WorldSequencePlay(targetId);
	}
	if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == kind)
		return Activate_SpawnGroupFromTrigger(targetId);
	if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == kind)
	{
		const bool activated = Activate_Encounter(targetId);
#ifdef _DEBUG
		if (activated && WORLD_ID::VALTAN_ARENA == m_eWorldId && "boss.valtan.center" == targetId)
		{
			SERVER_WORLD_ENTITY* boss = Find_AuditionBoss();
			if (nullptr == boss)
				return false;
			const std::uint32_t openingHp = CValtanBrain::Resolve_HealthBarHp(*boss, 159u);
			if (0u == openingHp)
				return false;
			// 다음 정상 brain tick이 160→159 체력선 변화를 관측하도록 상태만 맞춘다.
			// 이 함수가 직접 카메라·벽 파괴·Client cue를 실행하지는 않는다.
			boss->iCurrentHp = openingHp;
			boss->iLastEvaluatedHealthBar = 160u;
		}
#endif
		return activated;
	}
	return false;
}

// ServerApp의 룸 worker가 고정 delta(초)와 스케줄 계측값을 넘긴다. 정리/입력 소비 뒤 플레이어·월드·판정·송신을 순서대로 진행한다.
// 대부분의 gameplay 상태는 이 단일 worker가 소유하며 command mutex는 큐 교환/통계에만 잡는다.
// 중간 실패는 이미 진행한 상태와 송신을 rollback하지 않는다. 명시적 실패 경로는 계측 후 중단하고 일부는 룸을 실패 상태로 내린다.
void LostArk::Server::CGameRoom::Tick(
	const float fixedDeltaSeconds, const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics)
{
	if (!m_isReady)
		return;
	const auto tickStart = std::chrono::steady_clock::now();

	// 정상 종료와 명시적 중단에서 경과 μs와 navigation/스케줄 표본을 기록한다. RAII guard가 아니므로 예외 경로까지 자동 기록하지 않는다.
	const auto recordTickDuration = [this, tickStart, &schedulerMetrics]()
	{
		const std::uint64_t elapsedMicroseconds = To_Microseconds(std::chrono::steady_clock::now() - tickStart);
		const auto navigationMetrics = m_ServerNavigation.Get_PerformanceMetrics();
		std::scoped_lock lock{m_CommandMutex};
		m_PerformanceMetrics.Navigation = navigationMetrics;
		m_PerformanceMetrics.Scheduler = schedulerMetrics;
		++m_PerformanceMetrics.iTickCount;
		m_PerformanceMetrics.iLastTickMicroseconds = elapsedMicroseconds;
		m_PerformanceMetrics.iMaximumTickMicroseconds =
			(std::max)(m_PerformanceMetrics.iMaximumTickMicroseconds, elapsedMicroseconds);
	};

	// 정리 큐는 전부, 일반 큐는 최대 256개만 지역 값으로 가져온다. 핸들러 실행 중에는 큐 잠금을 풀어 수신을 허용한다.
	std::deque<ROOM_COMMAND> cleanupCommands;
	std::deque<ROOM_COMMAND> commands;
	{
		std::scoped_lock lock{m_CommandMutex};
		const std::size_t cleanupIngressDepth = m_CleanupCommands.size();
		cleanupCommands.swap(m_CleanupCommands);
		const std::size_t ingressDepth = m_InboundCommands.size();
		const std::size_t drainCount = (std::min)(ingressDepth, MAX_COMMANDS_DRAINED_PER_TICK);
		for (std::size_t index = 0u; index < drainCount; ++index)
		{
			commands.push_back(std::move(m_InboundCommands.front()));
			m_InboundCommands.pop_front();
		}
		m_PerformanceMetrics.iLastIngressDepth = ingressDepth;
		m_PerformanceMetrics.iLastDrainedCommandCount = drainCount;
		m_PerformanceMetrics.iLastRemainingCommandCount = m_InboundCommands.size();
		m_PerformanceMetrics.iLastCleanupIngressDepth = cleanupIngressDepth;
		m_PerformanceMetrics.iLastDrainedCleanupCommandCount = cleanupCommands.size();
		m_PerformanceMetrics.iLastRemainingCleanupCommandCount = m_CleanupCommands.size();
		if (!m_InboundCommands.empty())
			++m_PerformanceMetrics.iDrainLimitedTickCount;
	}

	// 정리 명령은 일반 입력의 drain 상한과 독립적이며 이번에 가져온 일반 명령보다 먼저 처리한다.
	for (ROOM_COMMAND& command : cleanupCommands)
	{
		Leave(command.iSessionId, command.eLeaveReason);
		std::scoped_lock lock{m_CommandMutex};
		m_QueuedCleanupSessionIds.erase(command.iSessionId);
	}

	// 콜로세움은 경기 phase별 허용 명령만 handler에 전달한다. 여기서 continue한 지역 명령은 다음 tick에 재예약하지 않는다.
	for (ROOM_COMMAND& command : commands)
	{
		if (m_iColosseumMatchId && m_eColosseumPhase != LostArk::Shared::COLOSSEUM_MATCH_PHASE::PLAYING &&
			command.eType != ROOM_COMMAND_TYPE::REGISTER_SESSION && command.eType != ROOM_COMMAND_TYPE::ENTER_WORLD &&
			command.eType != ROOM_COMMAND_TYPE::COLOSSEUM_LOAD_READY &&
			command.eType != ROOM_COMMAND_TYPE::COLOSSEUM_RETURN && command.eType != ROOM_COMMAND_TYPE::ROOM_PING &&
			command.eType != ROOM_COMMAND_TYPE::CHAT && command.eType != ROOM_COMMAND_TYPE::CAPTURE_CHARACTER &&
			!(m_eColosseumPhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::RECRUITING &&
				(command.eType == ROOM_COMMAND_TYPE::MOVE || command.eType == ROOM_COMMAND_TYPE::COLOSSEUM_RECRUIT)) &&
			!(m_eColosseumPhase == LostArk::Shared::COLOSSEUM_MATCH_PHASE::FINISHED &&
				(command.eType == ROOM_COMMAND_TYPE::INTERACT_TRIGGER ||
					command.eType == ROOM_COMMAND_TYPE::USE_SQUAREHOLE)))
			continue;
		switch (command.eType)
		{
		case ROOM_COMMAND_TYPE::REGISTER_SESSION:
			Handle_Register(command.pSession);
			break;
		case ROOM_COMMAND_TYPE::ENTER_WORLD:
			Join(command.iSessionId, command.EnterWorld, command.strSpawnPlacementOverrideId, command.CarriedInventory,
				command.iCarriedHonorTitleId, command.strRaidReturnNpcPlacementId, command.CarriedPurse,
				command.eEntrySourceWorldId, command.bHasCarriedCharacterState);
			break;
		case ROOM_COMMAND_TYPE::MOVE:
			Handle_Move(command.iSessionId, command.Move);
			break;
		case ROOM_COMMAND_TYPE::USE_SKILL:
			Handle_UseSkill(command.iSessionId, command.UseSkill);
			break;
		case ROOM_COMMAND_TYPE::RELEASE_SKILL:
			Handle_ReleaseSkill(command.iSessionId, command.ReleaseSkill);
			break;
		case ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM:
			Handle_UpdateSkillAim(command.iSessionId, command.UpdateSkillAim);
			break;
		case ROOM_COMMAND_TYPE::USE_ESTHER_SKILL:
			Handle_UseEstherSkill(command.iSessionId, command.UseEstherSkill);
			break;
		case ROOM_COMMAND_TYPE::USE_SQUAREHOLE:
			Handle_UseSquareHole(command.iSessionId, command.UseSquareHole);
			break;
		case ROOM_COMMAND_TYPE::REVIVE_PLAYER:
			Handle_RevivePlayer(command.iSessionId, command.RevivePlayer);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_KILL_GATE_BOSSES:
			Handle_DebugKillGateBosses(command.iSessionId, command.DebugKillGateBosses);
			break;
		case ROOM_COMMAND_TYPE::SET_COOLDOWN_MODE:
			Handle_SetCooldownMode(command.iSessionId, command.SetCooldownMode);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_KILL_SELF:
			Handle_DebugKillSelf(command.iSessionId, command.DebugKillSelf);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_ENTER_KOUKUSAYDON_ARENA:
			Handle_DebugEnterKoukuSaydonArena(command.iSessionId, command.DebugEnterKoukuSaydonArena);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_TELEPORT_TO_PLACEMENT:
			Handle_DebugTeleportToPlacement(command.iSessionId, command.DebugTeleportToPlacement);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_TELEPORT_TO_POSITION:
			Handle_DebugTeleportToPosition(command.iSessionId, command.DebugTeleportToPosition);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_MARIO_JUMP:
			Handle_DebugMarioJump(command.iSessionId, command.DebugMarioJump);
			break;
		case ROOM_COMMAND_TYPE::MARIO_RETURN:
			Handle_MarioReturn(command.iSessionId, command.MarioReturn);
			break;
		case ROOM_COMMAND_TYPE::MARIO_MOVE:
			Handle_MarioMove(command.iSessionId, command.MarioMove);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_BINGO_FILL:
			Handle_DebugBingoFill(command.iSessionId, command.DebugBingoFill);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_BINGO_BOMB:
			Handle_DebugBingoBomb(command.iSessionId, command.DebugBingoBomb);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_BINGO_HAMMER:
			Handle_DebugBingoHammer(command.iSessionId, command.DebugBingoHammer);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_RESUMMON_WAVE_MONSTERS:
			Handle_DebugResummonWaveMonsters(command.iSessionId, command.DebugResummonWaveMonsters);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_USE_ESTHER:
			Handle_DebugUseEsther(command.iSessionId, command.DebugUseEsther);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_SET_MADNESS_FORM:
			Handle_DebugSetMadnessForm(command.iSessionId, command.DebugSetMadnessForm);
			break;
		case ROOM_COMMAND_TYPE::SET_VEHICLE_RIDING:
			Handle_SetVehicleRiding(command.iSessionId, command.SetVehicleRiding);
			break;
		case ROOM_COMMAND_TYPE::SET_HONOR_TITLE:
			Handle_SetHonorTitle(command.iSessionId, command.SetHonorTitle);
			break;
		case ROOM_COMMAND_TYPE::INTERACTION_SLOT:
			Handle_InteractionSlot(command.iSessionId, command.InteractionSlot);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_SET_KOUKU_HUD_MODE:
			Handle_DebugSetKoukuHudMode(command.iSessionId, command.DebugSetKoukuHudMode);
			break;
		case ROOM_COMMAND_TYPE::CHANGE_CHARACTER_CLASS:
			Handle_ChangeCharacterClass(command.iSessionId, command.ChangeCharacterClass);
			break;
		case ROOM_COMMAND_TYPE::SPAWN_WORLD_ENTITY:
			Handle_SpawnWorldEntity(command.iSessionId, command.SpawnWorldEntity);
			break;
		case ROOM_COMMAND_TYPE::VALTAN_AUDITION:
			Handle_ValtanAudition(command.iSessionId, command.ValtanAudition);
			break;
		case ROOM_COMMAND_TYPE::VALTAN_PATTERN_FLOW_START:
			Handle_ValtanPatternFlowStart(command.iSessionId, command.ValtanPatternFlowStart);
			break;
		case ROOM_COMMAND_TYPE::VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT:
			Handle_ValtanPatternFlowStopAfterCurrent(command.iSessionId, command.ValtanPatternFlowStopAfterCurrent);
			break;
		case ROOM_COMMAND_TYPE::KOUKUSAYDON_RAID:
			Handle_KoukuRaidRequest(command.iSessionId, command.KoukuSaydonRaid);
			break;
		case ROOM_COMMAND_TYPE::KOUKUSAYDON_DRAFT_CHUNK:
			Handle_KoukuSaydonDraftChunk(command.iSessionId, command.KoukuSaydonDraftChunk);
			break;
		case ROOM_COMMAND_TYPE::KOUKUSAYDON_PATTERN_AUDITION:
			Handle_KoukuSaydonPatternAudition(command.iSessionId, command.KoukuSaydonPatternAudition);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_GIVE_ITEM:
			Handle_DebugGiveItem(command.iSessionId, command.DebugGiveItem);
			break;
		case ROOM_COMMAND_TYPE::USE_ITEM:
			Handle_UseItem(command.iSessionId, command.UseItem);
			break;
		case ROOM_COMMAND_TYPE::SET_EQUIPMENT:
			Handle_SetEquipment(command.iSessionId, command.SetEquipment);
			break;
		case ROOM_COMMAND_TYPE::REPAIR_EQUIPMENT:
			Handle_RepairEquipment(command.iSessionId, command.RepairEquipment);
			break;
		case ROOM_COMMAND_TYPE::BUY_ITEMS:
			Handle_BuyItems(command.iSessionId, command.BuyItems);
			break;
		case ROOM_COMMAND_TYPE::RESTORE_CHARACTER:
			Handle_RestoreCharacter(command.iSessionId, command.RestoreCharacter);
			break;
		case ROOM_COMMAND_TYPE::UPGRADE_EQUIPMENT:
			Handle_UpgradeEquipment(command.iSessionId, command.UpgradeEquipment);
			break;
		case ROOM_COMMAND_TYPE::CAPTURE_CHARACTER:
			Handle_CaptureCharacter(command.iSessionId, command.CaptureCharacter);
			break;
		case ROOM_COMMAND_TYPE::DESPAWN_ALL_WORLD_ENTITIES:
			Handle_DespawnAllWorldEntities(command.iSessionId, command.DespawnAllWorldEntities);
			break;
		case ROOM_COMMAND_TYPE::CONFIRM_NPC_ENTRY:
			Handle_ConfirmNpcEntry(command.iSessionId, command.ConfirmNpcEntry);
			break;
		case ROOM_COMMAND_TYPE::INTERACT_TRIGGER:
			Handle_InteractTrigger(command.iSessionId, command.InteractTrigger);
			break;
		case ROOM_COMMAND_TYPE::RETURN_TO_BERN:
			Handle_ReturnToBern(command.iSessionId, command.ReturnToBern);
			break;
		case ROOM_COMMAND_TYPE::MAHARAKA_AI_TUNING:
			Handle_MaharakaAITuning(command.iSessionId, command.MaharakaAITuning);
			break;
		case ROOM_COMMAND_TYPE::DEBUG_WORLD_PLAYBACK:
			Handle_DebugWorldPlayback(command.iSessionId, command.DebugWorldPlayback);
			break;
		case ROOM_COMMAND_TYPE::GUIDE_CONTROL:
			Handle_GuideControl(command.iSessionId, command.GuideControl);
			break;
		case ROOM_COMMAND_TYPE::PARTY_INVITE:
			Handle_PartyInvite(command.iSessionId, command.PartyInvite);
			break;
		case ROOM_COMMAND_TYPE::PARTY_INVITE_RESPOND:
			Handle_PartyInviteRespond(command.iSessionId, command.PartyInviteRespond);
			break;
		case ROOM_COMMAND_TYPE::RAID_ENTRY_PROPOSE:
			Handle_RaidEntryPropose(command.iSessionId, command.RaidEntryPropose);
			break;
		case ROOM_COMMAND_TYPE::RAID_ENTRY_RESPOND:
			Handle_RaidEntryRespond(command.iSessionId, command.RaidEntryRespond);
			break;
		case ROOM_COMMAND_TYPE::COLOSSEUM_QUEUE_JOIN:
			Handle_ColosseumQueueJoin(command.iSessionId, command.ColosseumQueueJoin);
			break;
		case ROOM_COMMAND_TYPE::COLOSSEUM_QUEUE_LEAVE:
			Handle_ColosseumQueueLeave(command.iSessionId, command.ColosseumQueueLeave);
			break;
		case ROOM_COMMAND_TYPE::COLOSSEUM_RECRUIT:
			Handle_ColosseumRecruit(command.iSessionId, command.ColosseumRecruit);
			break;
		case ROOM_COMMAND_TYPE::COLOSSEUM_LOAD_READY:
			Handle_ColosseumLoadReady(command.iSessionId, command.ColosseumLoadReady);
			break;
		case ROOM_COMMAND_TYPE::COLOSSEUM_RETURN:
			Handle_ColosseumReturn(command.iSessionId, command.ColosseumReturn);
			break;
		case ROOM_COMMAND_TYPE::GATE_PROGRESS_PROPOSE:
			Handle_GateProgressPropose(command.iSessionId, command.GateProgressPropose);
			break;
		case ROOM_COMMAND_TYPE::GATE_PROGRESS_RESPOND:
			Handle_GateProgressRespond(command.iSessionId, command.GateProgressRespond);
			break;
		case ROOM_COMMAND_TYPE::ROOM_PING:
			Handle_RoomPing(command.iSessionId, command.RoomPing);
			break;
		case ROOM_COMMAND_TYPE::CHAT:
			Handle_Chat(command.iSessionId, command.Chat);
			break;
		case ROOM_COMMAND_TYPE::LEAVE:
			// Enqueue_Detailed가 LEAVE를 정리 큐로만 보내므로 일반 큐 dispatch에서는 실행하지 않는다.
			break;
		}
	}

	// 먼저 입력 결과를 처리한다. delta가 비유한/0 이하라도 위 명령 소비는 이미 수행됐고 이후 simulation만 생략한다.
	Flush_PartyTransferResults();
	if (!std::isfinite(fixedDeltaSeconds) || fixedDeltaSeconds <= 0.f)
	{
		recordTickDuration();
		return;
	}

	// 명령 처리 중 쌓인 피해를 이번 tick의 wire 상한까지만 옮긴다. 보류 목록은 전부 비우므로 상한 초과분을 다음 tick에 남기지 않는다.
	m_TickDamageEvents.clear();
	if (!m_PendingCommandDamageEvents.empty())
	{
		const size_t take =
			(std::min)(m_PendingCommandDamageEvents.size(), static_cast<size_t>(LostArk::Shared::MAX_DAMAGE_EVENTS));
		m_TickDamageEvents.assign(m_PendingCommandDamageEvents.begin(), m_PendingCommandDamageEvents.begin() + take);
		m_PendingCommandDamageEvents.clear();
	}
	m_TickBossCombatEvents.clear();

	// 0은 예약값이므로 uint32 최댓값 다음에는 1로 순환한다. 아래 simulation과 필수 lifecycle 처리를 마친 뒤 m_iServerTick에 이번 값을 확정한다.
	const std::uint32_t updateTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ? 1u : m_iServerTick + 1u;
	// 보행 조회 전에 쿠크 첫 패턴 상태·WORLD 몸체·발판을 준비해 이번 tick의 지면을 반영한다.
	Prepare_KoukuAuditionTick(updateTick);
	Update_KoukuWorldBodies(updateTick);
	if (!Refresh_KoukuSupportSurfaces(updateTick))
	{
		recordTickDuration();
		return;
	}
	Refresh_PlayerBlockingBodies();
	for (const auto& [id, player] : m_Players)
		if (player.eCardMazeRole != LostArk::Shared::CARD_MAZE_ROLE::NONE)
			m_CardMazePreviousPositions[id] = {player.fPositionX, player.fPositionZ};

	// 경기/안내/상호작용을 먼저 갱신하고 player 이동·발사체·미니게임을 처리한다. 마지막 combat-object 갱신이 피해 목록을 누적한다.
	Update_MaharakaWaterpangMatch(updateTick);
	Update_ColosseumMatch(updateTick);
	Update_Guides(fixedDeltaSeconds);
	Update_Colosseum(fixedDeltaSeconds);
	Update_WorldPickups(updateTick, false);
	Update_Players(fixedDeltaSeconds);
	Update_MaharakaWaterGunShots(updateTick);
	Update_KoukuCardRainSoldiers(updateTick);
	Update_CardMaze(updateTick);
	Update_KoukuBingo(updateTick);
	m_CombatObjectRuntime.Update(
		m_Players, m_WorldEntities, m_GameplayCatalog, fixedDeltaSeconds, updateTick, m_TickDamageEvents);

	// trigger 평가의 전환/안내 결과는 지역 목록에 받는다. 발탄 timeline 시험 중에는 일반 진입 trigger 평가를 생략한다.
	std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
	std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
	bool evaluatePlayerTriggers = true;
	evaluatePlayerTriggers = WORLD_ID::VALTAN_ARENA != m_eWorldId ||
		VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE == m_ValtanTimelineAudition.ePhase;
	if (evaluatePlayerTriggers)
	{
		m_ServerTriggerSystem.Evaluate_Entries(
			m_Players, updateTick, transfers,
			[this](const WORLD_TRIGGER_ACTION_KIND kind, const std::string& targetId)
			{
				return Activate_TriggerTarget(kind, targetId);
			},
			promptEdges,
			[this](const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player, const std::uint32_t actionStartTick)
			{
				return Begin_MarioTriggerMove(trigger, player, actionStartTick);
			});
	}
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		Update_MarioControlState(player);
	}
	for (const SERVER_INTERACT_PROMPT_EDGE& edge : promptEdges)
		Send_InteractPrompt(edge);
	Cleanup_EmptyMarioStages();

	// 현재 룸 session이며 아직 같은 session의 요청이 대기하지 않는 경우만 전환 FIFO에 추가한다. ServerApp이 tick 뒤 목적지를 검증한다.
	for (SERVER_WORLD_TRANSFER_REQUEST& transfer : transfers)
	{
		if (!m_PlayerIdBySessionId.contains(transfer.iSessionId))
			continue;
		const bool alreadyStaged = std::any_of(m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
			[sessionId = transfer.iSessionId](const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == sessionId;
			});
		if (!alreadyStaged)
			m_PendingWorldTransfers.push_back(std::move(transfer));
	}

	// SpawnGroup은 현재 생존 수와 실제 Spawn_Monster 성공 여부를 callback으로 확인한다. 룸이 entity 저장소를 계속 소유한다.
	m_SpawnGroupRuntime.Update(
		fixedDeltaSeconds, m_SpawnGroupBootstrap,
		[this](const std::string& spawnGroupId)
		{
			return Count_SpawnGroupEntities(spawnGroupId);
		},
		[this](const std::string& spawnGroupId, const SPAWN_GROUP_ENTRY& entry, const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile, const std::uint32_t ordinal)
		{
			return Spawn_Monster(spawnGroupId, entry, anchor, profile, ordinal);
		});
	m_EstherSkillSystem.Update(fixedDeltaSeconds, Count_HumanPlayers() != 0u);
	// Player/Esther 처리로 이번 tick의 WORLD 몸체가 파괴됐을 수 있으므로 boss Logic 전에 다시 정리한다.
	// 파괴된 소유자의 접촉 판정이 이후 boss 피해/광기 처리에 남지 않게 하는 갱신 순서다.
	Update_KoukuWorldBodies(updateTick);
	Update_WorldEntities(fixedDeltaSeconds);
	Update_KoukuWorldBodies(updateTick);
	// boss 갱신 중 소유 패턴이 완료/중단될 수 있어 발판과 그 위 개체 높이를 다시 맞춘다.
	if (!Refresh_KoukuSupportSurfaces(updateTick))
	{
		recordTickDuration();
		return;
	}
	Update_KoukuPlayerModes(updateTick);
	Update_KoukuRaid(updateTick);
	if (!m_isReady)
	{
		recordTickDuration();
		return;
	}
	// 일반 player 갱신 뒤에 boss root motion이 확정되므로 GRABBED player의 부착 위치를 한 번 더 계산한다.
	// 반환값은 여기서 무시하며 이 재계산 이후의 값을 snapshot이 소비한다.
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		if (LostArk::Shared::PLAYER_ACTION_STATE::GRABBED == player.eAction)
			(void)Update_PlayerAttachment(player, updateTick);
	}

	// 기한이 된 prop/파괴 transaction을 순서대로 확정한다. 앞쪽 성공 뒤 뒤쪽 실패도 전체 tick을 되돌리지는 않는다.
	if (!Commit_DueEncounterProps(updateTick) || !Commit_DueWorldDestruction(updateTick))
	{
		Mark_RuntimeFailure("fixed-tick.due-world-transaction");
		recordTickDuration();
		return;
	}
	Update_WorldPickups(updateTick);
	Drain_BossCombatEvents();
	if (!Flush_KoukuSaydonPatternAuditionLifecycle())
	{
		m_strStatus = "KoukuSaydon pattern audition lifecycle serialization failed";
		Mark_RuntimeFailure("fixed-tick.koukusaydon-audition-lifecycle");
		recordTickDuration();
		return;
	}
	// 현재 occurrence 완료와 Next 승격은 위의 월드 변경까지 관측한 뒤 처리한다.
	// 여기서 승격한 ID의 BeginPattern은 이미 끝난 이번 world 갱신을 다시 실행하지 않으므로 다음 갱신에서 시작한다.
	(void)Refresh_ValtanPatternIdAuditionState();
	if (!Flush_ValtanPatternIdAuditionLifecycle())
	{
		m_strStatus = "Valtan audition lifecycle serialization failed";
		Mark_RuntimeFailure("fixed-tick.valtan-audition-lifecycle");
		recordTickDuration();
		return;
	}
	if (!Flush_ValtanPatternFlowLifecycle())
	{
		m_strStatus = "Valtan pattern-flow lifecycle serialization failed";
		Mark_RuntimeFailure("fixed-tick.valtan-flow-lifecycle");
		recordTickDuration();
		return;
	}

	// 필수 lifecycle 구성/송신 실패는 룸 실패로 기록한다. 이전 lifecycle을 이미 보냈다면 그 전송도 취소하지 않는다.
	if (!Broadcast_CombatObjectLifecycle())
	{
		Mark_RuntimeFailure("fixed-tick.combat-object-lifecycle");
		recordTickDuration();
		return;
	}
	Enforce_VehicleRidingState();
	Score_ColosseumKills(updateTick);
	m_iServerTick = updateTick;
	if (m_iColosseumMatchId && updateTick % 3u == 0u)
		Broadcast_ColosseumMatchState();
	Expire_RaidEntryProposals();
	Expire_GateProgressVote();
	// 내구도가 바뀐 인간 player에게 inventory snapshot을 보낸다. dirty를 먼저 내리고 송신 실패는 session 종료 요청으로 처리한다.
	for (auto& [wornPlayerId, wornPlayer] : m_Players)
	{
		if (!wornPlayer.bDurabilityDirty || !wornPlayer.Is_Human())
			continue;
		wornPlayer.bDurabilityDirty = false;
		const std::shared_ptr<CClientSession> wornSession = Find_Session(wornPlayer.iSessionId);
		if (nullptr != wornSession && !Send_InventorySnapshot(wornSession, 0u, wornPlayer))
			wornSession->Request_Close();
	}
	if (Count_HumanPlayers() != 0u)
		Broadcast_WorldSnapshot();

	// snapshot 처리 뒤에도 진행 중 occurrence가 필요한 세대는 보관한다. pin 목록이 wire 상한을 넘으면 룸 실패로 중단한다.
	std::vector<LostArk::Shared::GameplayDataRevision> liveGenerationPins;
	if (!Build_RequiredPinnedGameplayRevisions(liveGenerationPins))
	{
		m_strStatus = "Gameplay generation pin set exceeded its wire bound";
		Mark_RuntimeFailure("fixed-tick.gameplay-generation-pins");
		recordTickDuration();
		return;
	}
	m_GameplayCatalog.Collect_Garbage(liveGenerationPins);
	recordTickDuration();

	// player 컨테이너가 비지 않은 룸은 300 tick마다 로그 표본을 검사한다. 인간만을 뜻하지 않으며
	// 누적 실패 증가·현재 압력·1800 tick heartbeat 중 하나일 때 한 줄을 만든다.
	if (!m_Players.empty() && 0u == (m_iServerTick % 300u))
	{
		const SERVER_ROOM_PERFORMANCE_METRICS metrics = Get_PerformanceMetrics();
		std::size_t maximumCurrentOutboundFrames = 0u;
		std::size_t maximumOutboundFrameHighWatermark = 0u;
		std::uint64_t snapshotCoalescedCount = 0u;
		std::uint64_t snapshotDroppedCount = 0u;
		std::uint64_t reliableRejectedCount = 0u;
		std::uint64_t sendFailureCount = 0u;
		std::uint64_t maximumWireSendMicroseconds = 0u;
		for (const auto& [sessionId, weakSession] : m_Sessions)
		{
			(void)sessionId;
			const std::shared_ptr<CClientSession> session = weakSession.lock();
			if (nullptr == session)
				continue;
			const CLIENT_SESSION_OUTBOUND_METRICS sessionMetrics = session->Get_OutboundMetrics();
			maximumCurrentOutboundFrames =
				(std::max)(maximumCurrentOutboundFrames, sessionMetrics.iCurrentQueuedFrameCount);
			maximumOutboundFrameHighWatermark =
				(std::max)(maximumOutboundFrameHighWatermark, sessionMetrics.iQueuedFrameHighWatermark);
			snapshotCoalescedCount += sessionMetrics.iSnapshotCoalescedFrameCount;
			snapshotDroppedCount += sessionMetrics.iSnapshotDroppedFrameCount;
			reliableRejectedCount += sessionMetrics.iReliableRejectedFrameCount;
			sendFailureCount += sessionMetrics.iSendFailureCount;
			maximumWireSendMicroseconds =
				(std::max)(maximumWireSendMicroseconds, sessionMetrics.iMaximumFrameSendMicroseconds);
		}

		// 실패 누적값의 증가와 대기/지연 임계값을 이전 로그 표본과 비교한다. 최대 지연은 새 최고값이 된 경우만 추가 압력으로 취급한다.
		const bool hasNewFailure =
			metrics.Scheduler.iScheduleResetCount > m_LastRoomPerfLogSample.Scheduler.iScheduleResetCount ||
			metrics.iDrainLimitedTickCount > m_LastRoomPerfLogSample.iDrainLimitedTickCount ||
			metrics.iDroppedBestEffortCommandCount > m_LastRoomPerfLogSample.iDroppedBestEffortCommandCount ||
			metrics.iRejectedReliableCommandCount > m_LastRoomPerfLogSample.iRejectedReliableCommandCount ||
			metrics.iRejectedCleanupCommandCount > m_LastRoomPerfLogSample.iRejectedCleanupCommandCount ||
			metrics.iSnapshotEncodeFailureCount > m_LastRoomPerfLogSample.iSnapshotEncodeFailureCount ||
			metrics.iSnapshotEnqueueFailureCount > m_LastRoomPerfLogSample.iSnapshotEnqueueFailureCount ||
			snapshotDroppedCount > m_iLastRoomPerfSnapshotDroppedCount ||
			reliableRejectedCount > m_iLastRoomPerfReliableRejectedCount ||
			sendFailureCount > m_iLastRoomPerfWireSendFailureCount;
		const bool hasCurrentPressure = metrics.Scheduler.iPreviousLoopLatenessMicroseconds >= 33333u ||
			(metrics.Scheduler.iMaximumLoopLatenessMicroseconds >= 33333u &&
				metrics.Scheduler.iMaximumLoopLatenessMicroseconds >
					m_LastRoomPerfLogSample.Scheduler.iMaximumLoopLatenessMicroseconds) ||
			metrics.iLastTickMicroseconds >= 33333u ||
			(metrics.iMaximumTickMicroseconds >= 33333u &&
				metrics.iMaximumTickMicroseconds > m_LastRoomPerfLogSample.iMaximumTickMicroseconds) ||
			0u != metrics.iLastRemainingCommandCount || 0u != metrics.iLastRemainingCleanupCommandCount ||
			maximumCurrentOutboundFrames >= 64u ||
			(maximumOutboundFrameHighWatermark >= 64u &&
				maximumOutboundFrameHighWatermark > m_iLastRoomPerfOutboundHighWatermark);
		const bool isHeartbeat = 0u == (m_iServerTick % 1800u);

		// 출력 여부와 무관하게 이번 비교 기준을 저장한다. 아래 조건으로 출력을 생략해도 다음 비교는 이 표본부터다.
		m_LastRoomPerfLogSample = metrics;
		m_iLastRoomPerfSnapshotDroppedCount = snapshotDroppedCount;
		m_iLastRoomPerfReliableRejectedCount = reliableRejectedCount;
		m_iLastRoomPerfWireSendFailureCount = sendFailureCount;
		m_iLastRoomPerfOutboundHighWatermark = maximumOutboundFrameHighWatermark;
		if (!hasNewFailure && !hasCurrentPressure && !isHeartbeat)
			return;

		// stdout에는 즉시 출력하고 같은 문자열을 멤버 한 칸에도 보관한다. ServerApp이 Take로 가져가 파일 로그에 쓰며 여러 줄 큐는 아니다.
		const auto unixMilliseconds =
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
				.count();
		const auto& combatObjects = m_CombatObjectRuntime.Get_LiveObjects();
		const auto replicatedCombatObjects = std::count_if(combatObjects.begin(), combatObjects.end(),
			[](const auto& object)
			{
				return object.bReplicated;
			});
		std::ostringstream diagnostic;
		diagnostic << "[RoomPerf] UnixMs=" << unixMilliseconds
			<< " Kind=" << (hasNewFailure || hasCurrentPressure ? "anomaly" : "heartbeat")
			<< " World=" << static_cast<unsigned>(m_eWorldId) << " Tick=" << m_iServerTick
			<< " Gate=" << std::quoted(m_KoukuRaid.State.strGateId)
			<< " FlowIndex=" << m_KoukuRaid.State.iFlowEntryIndex
			<< " FlowEntry=" << std::quoted(m_KoukuRaid.State.strFlowEntryId)
			<< " RaidPhase=" << static_cast<unsigned>(m_KoukuRaid.State.ePhase)
			<< " RaidEpoch=" << m_KoukuRaid.State.iRunEpoch
			<< " RaidReason=" << std::quoted(m_KoukuRaid.State.strReason)
			<< " SequencePattern=" << std::quoted(m_KoukuRaid.State.strSequencePatternId)
			<< " ActionRevision=" << m_KoukuRaid.State.iActionSourceRevision
			<< " SequenceRevision=" << m_KoukuRaid.State.iSequenceSourceRevision
			<< " ReadyMask=" << static_cast<unsigned>(m_KoukuRaid.State.iReadyMask)
			<< " Participants=" << m_KoukuRaid.State.ParticipantPlayerIds.size()
			<< " LiveCombatObjects=" << combatObjects.size()
			<< " ReplicatedCombatObjects=" << replicatedCombatObjects
			<< " ReplicatedCombatObjectCap=" << LostArk::Shared::MAX_COMBAT_OBJECTS_PER_SNAPSHOT
			<< " LoopSampleUnixMs=" << metrics.Scheduler.iSampleUnixMilliseconds
			<< " PreviousLoopLateUs=" << metrics.Scheduler.iPreviousLoopLatenessMicroseconds
			<< " LoopMaxLateUs=" << metrics.Scheduler.iMaximumLoopLatenessMicroseconds
			<< " LoopScheduleResets=" << metrics.Scheduler.iScheduleResetCount
			<< " TickUs=" << metrics.iLastTickMicroseconds << " TickMaxUs=" << metrics.iMaximumTickMicroseconds
			<< " Ingress=" << metrics.iLastIngressDepth << " IngressHigh=" << metrics.iIngressHighWatermark
			<< " Drained=" << metrics.iLastDrainedCommandCount
			<< " Remaining=" << metrics.iLastRemainingCommandCount
			<< " CleanupIngress=" << metrics.iLastCleanupIngressDepth
			<< " CleanupIngressHigh=" << metrics.iCleanupIngressHighWatermark
			<< " CleanupDrained=" << metrics.iLastDrainedCleanupCommandCount
			<< " CleanupRemaining=" << metrics.iLastRemainingCleanupCommandCount
			<< " CleanupDeduplicated=" << metrics.iDeduplicatedCleanupCommandCount
			<< " CleanupCancelledCommands=" << metrics.iCancelledCommandCountByCleanup
			<< " MoveCoalesced=" << metrics.iCoalescedMoveCommandCount
			<< " AimCoalesced=" << metrics.iCoalescedAimCommandCount
			<< " BestEffortDropped=" << metrics.iDroppedBestEffortCommandCount
			<< " ReliableRejected=" << metrics.iRejectedReliableCommandCount
			<< " SnapshotEncodeUs=" << metrics.iLastSnapshotEncodeMicroseconds
			<< " SnapshotEnqueueUs=" << metrics.iLastSnapshotEnqueueMicroseconds
			<< " SessionEnqueueMaxUs=" << metrics.iMaximumSessionEnqueueMicroseconds
			<< " SnapshotEnqueueFailures=" << metrics.iSnapshotEnqueueFailureCount
			<< " OutboundQueuedMax=" << maximumCurrentOutboundFrames
			<< " OutboundHighMax=" << maximumOutboundFrameHighWatermark
			<< " SnapshotCoalesced=" << snapshotCoalescedCount << " SnapshotDropped=" << snapshotDroppedCount
			<< " OutboundReliableRejected=" << reliableRejectedCount
			<< " WireSendMaxUs=" << maximumWireSendMicroseconds << " WireSendFailures=" << sendFailureCount;
		diagnostic << " PatternIds=";
		bool hasPattern = false;
		for (const auto& member : m_KoukuSaydonPatternAudition.Members)
		{
			if (member.bCompleted || member.iPatternIndex >= member.PatternIds.size())
				continue;
			if (hasPattern)
				diagnostic << ',';
			diagnostic << std::quoted(member.PatternIds[member.iPatternIndex]);
			hasPattern = true;
		}
		if (!hasPattern)
			diagnostic << "none";

		// navigation 누적/최대 나노초를 정수 μs로 내림해 로그에 적는다. 호출 수·확장 노드·반환 점 수는 원 통계 단위 그대로다.
		const auto writeNavigation = [&diagnostic](const char* name, const SERVER_NAVIGATION_QUERY_METRICS& stage)
		{
			diagnostic << " Nav" << name << "Calls=" << stage.iCalls << " Nav" << name
				<< "TotalUs=" << stage.iTotalNanoseconds / 1000u << " Nav" << name
				<< "MaxUs=" << stage.iMaximumNanoseconds / 1000u << " Nav" << name
				<< "Expanded=" << stage.iExpandedNodes << " Nav" << name
				<< "PathPoints=" << stage.iReturnedPathPoints;
		};
		writeNavigation("FindPath", metrics.Navigation.FindPath);
		writeNavigation("ReachablePath", metrics.Navigation.ReachablePath);
		writeNavigation("ProjectPoint", metrics.Navigation.ProjectPoint);
		writeNavigation("SmoothPath", metrics.Navigation.SmoothPath);
		writeNavigation("TraversalStep", metrics.Navigation.TraversalStep);
		writeNavigation("LineOfSight", metrics.Navigation.LineOfSight);
		m_strPendingPerformanceDiagnostic = diagnostic.str();
		std::cout << m_strPendingPerformanceDiagnostic << '\n';
	}
}

// ServerApp의 수치 저장 준비가 룸 worker에서 호출한다. 후보 snapshot과 별도 쿠크 catalog의 수치 복사본을
// 지역 변수로 만든 뒤 세대 저장소의 예약 성공 시 멤버에 옮긴다. 명시적 false는 이전 룸 예약 배열을 유지한다.
// 외부 catalog의 choreography는 유지하고 수치만 합친다. 생성/복사 할당 예외는 호출자에게 전파된다.
bool LostArk::Server::CGameRoom::Stage_NumericBalance(const std::uint32_t transactionSequence,
	const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status)
{
	LostArk::Shared::GameplayDataRevision ignored;
	std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY> entries;
	if (!candidate || !candidate->Build_NumericBalanceSnapshot(entries, ignored, status))
		return false;
	decltype(m_StagedNumericCatalogRemaps) remaps;

	// 같은 shared_ptr를 중복 복사하지 않는다. 현재 active와 같은 객체면 후보를 바로 공유하고
	// 별도 실행/게시 catalog면 기존 정의 위에 후보의 숫자만 올린 새 불변 객체를 준비한다.
	for (const auto& original : {m_KoukuRaid.pCatalog, m_KoukuSaydonPatternAudition.pProductGeneration,
		m_pKoukuPublishedProductGeneration, m_GameplayCatalog.Get_ActiveGeneration()})
	{
		if (!original ||
			std::any_of(remaps.begin(), remaps.end(),
				[&](const auto& pair)
				{
					return pair.first == original;
				}))
			continue;
		std::shared_ptr<const CGameplayCatalog> replacement = candidate;
		if (original != m_GameplayCatalog.Get_ActiveGeneration())
		{
			auto copy = std::make_shared<CGameplayCatalog>();
			if (!copy->Load_NumericBalanceValues(*original, *candidate))
			{
				status = "Running Kouku Product rejects numeric balance: " + copy->Get_Status();
				return false;
			}
			replacement = std::move(copy);
		}
		remaps.emplace_back(original, std::move(replacement));
	}
	if (!m_GameplayCatalog.Stage_NumericBalance(transactionSequence, candidate, status))
		return false;
	m_StagedNumericEntries = std::move(entries);
	m_StagedNumericCatalogRemaps = std::move(remaps);
	return true;
}

// ServerApp이 디스크 저장 성공 뒤 전 룸에 확정한다. 정상 실행은 룸 worker, 종료 중에는 worker 합류 뒤 Shutdown 경로다.
// 예약 sequence 실패는 false다. 성공하면 catalog를 교체하고 실제 player·투사체·boss·무력화 수치까지 조정한다.
// noexcept이지만 이후 상태 변경을 rollback하는 transaction은 아니며 이전 catalog는 지역 shared_ptr로 계산이 끝날 때까지 살려 둔다.
bool LostArk::Server::CGameRoom::Commit_NumericBalance(const std::uint32_t transactionSequence) noexcept
{
	const auto previous = m_GameplayCatalog.Get_ActiveGeneration();
	if (!previous || !m_GameplayCatalog.Commit_NumericBalance(transactionSequence))
		return false;

	// 별도 쿠크 실행/게시 포인터가 준비 때 기록한 객체와 같으면 대응 복사본으로 교체한다. 맞지 않는 포인터는 건드리지 않는다.
	for (auto* target :
		{&m_KoukuRaid.pCatalog, &m_KoukuSaydonPatternAudition.pProductGeneration, &m_pKoukuPublishedProductGeneration})
		for (const auto& pair : m_StagedNumericCatalogRemaps)
			if (*target == pair.first)
			{
				*target = pair.second;
				break;
			}
	m_StagedNumericCatalogRemaps.clear();

	// 현재량/이전 최댓값의 비율에 새 최댓값을 곱하고 정수 반올림한다. 현재량 또는 새 최대가 0이면 0이다.
	// HP에는 keepAlive를 켜 살아 있는 값이 반올림으로 0이 되지 않게 하며 새 최대 안으로 제한한다.
	const auto ratio = [](const std::uint32_t current, const std::uint32_t oldMaximum, const std::uint32_t newMaximum,
		const bool keepAlive)
	{
		if (!current || !newMaximum)
			return 0u;
		const auto scaled = oldMaximum
			? static_cast<std::uint32_t>(
				(static_cast<std::uint64_t>(current) * newMaximum + oldMaximum / 2u) / oldMaximum)
			: 0u;
		return (std::min)(newMaximum, (std::max)(keepAlive ? 1u : 0u, scaled));
	};

	// 현재 class의 새 profile이 있는 player만 HP·자원·identity 비율과 속도를 갱신한다. 기존 사망자는 부활시키지 않는다.
	for (auto& [sessionId, player] : m_Players)
	{
		const auto* profile = m_GameplayCatalog.Find_Player(player.eCharacterClass);
		if (!profile)
			continue;
		// 진행 경기의 해당 팀 player/용병은 입장 당시 HP 최댓값을 유지한다.
		// 다른 class 수치와 자원·identity·속도는 아래의 새 profile 적용을 계속 받는다.
		const bool matchedColosseum = m_eWorldId == LostArk::Shared::WORLD_ID::COLOSSEUM && m_iColosseumMatchId != 0u &&
			player.iColosseumMatchId == m_iColosseumMatchId && player.iColosseumTeam < 2u &&
			(player.Is_Human() || player.Is_ColosseumMercenary());
		const auto maximumHp = matchedColosseum ? player.iMaximumHp : profile->iMaximumHp;
		player.iCurrentHp = ratio(player.iCurrentHp, player.iMaximumHp, maximumHp, true);
		player.iMaximumHp = maximumHp;
		player.iCurrentResource =
			ratio(player.iCurrentResource, player.iMaximumResource, profile->iMaximumResource, false);
		player.iMaximumResource = profile->iMaximumResource;
		player.iCurrentIdentity =
			ratio(player.iCurrentIdentity, player.iMaximumIdentity, profile->iMaximumIdentity, false);
		player.iMaximumIdentity = profile->iMaximumIdentity;
		player.fMoveSpeed = profile->fMoveSpeed;
		const auto* oldPlayer = previous->Find_Player(player.eCharacterClass);

		// 이미 존재하는 player 투사체는 이전/새 공격력과 damage profile로 얻은 기본 피해 비율을 곱한다.
		// 정의가 없거나 이전 피해가 0이면 그 투사체 값은 유지하고, 정수 나눗셈 결과는 uint32 상한으로 제한한다.
		for (auto& projectile : player.Projectiles)
		{
			const auto* oldSkill = previous->Find_Skill(projectile.iSkillId);
			const auto* newSkill = m_GameplayCatalog.Find_Skill(projectile.iSkillId);
			const auto* oldDamage = oldSkill ? previous->Find_DamageProfile(oldSkill->strDamageProfileId) : nullptr;
			const auto* newDamage =
				newSkill ? m_GameplayCatalog.Active().Find_DamageProfile(newSkill->strDamageProfileId) : nullptr;
			if (!oldPlayer || !oldDamage || !newDamage)
				continue;
			const auto oldRaw = CGameplayCatalog::Resolve_Damage(oldPlayer->iAttackPower, *oldDamage);
			const auto newRaw = CGameplayCatalog::Resolve_Damage(profile->iAttackPower, *newDamage);
			if (oldRaw)
				projectile.iTotalDamage =
					(std::min)(static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()),
						projectile.iTotalDamage * newRaw / oldRaw);
		}
	}

	// player 투사체와 별도로 룸 combat object의 live hit 피해·무력화·부위파괴 수치를 새 catalog와 준비된 entry로 갱신한 뒤 entry 목록을 비운다.
	m_CombatObjectRuntime.Refresh_NumericBalance(*previous, m_GameplayCatalog.Active(), m_StagedNumericEntries);
	m_StagedNumericEntries.clear();

	// boss만 새 profile의 HP·공격·충돌·추적/이동 수치를 반영한다. 현재 패턴의 stage 진행/identity를 여기서 새 패턴으로 재시작하지 않는다.
	for (auto& boss : m_WorldEntities)
	{
		if (WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind)
			continue;
		// 발탄 유령 단계는 원래 entity identity를 유지하므로 flag로 유령 profile을 선택한다.
		const auto* profile = m_GameplayCatalog.Find_Boss(
			boss.bGhostPhasePatternLoopActive && boss.strArchetypeId == "BOSS_VALTAN" ? "BOSS_VALTAN_GHOST"
				: boss.strArchetypeId);
		if (!profile)
			continue;
		boss.iCurrentHp = ratio(boss.iCurrentHp, boss.iMaximumHp, profile->iMaximumHp, true);
		boss.iLastEvaluatedHealthBar =
			ratio(boss.iLastEvaluatedHealthBar, boss.iMaximumHealthBars, profile->iMaximumHealthBars, false);
		boss.iMaximumHp = profile->iMaximumHp;
		boss.iMaximumHealthBars = profile->iMaximumHealthBars;
		boss.iDamageReferenceHp = profile->iDamageReferenceHp;
		boss.iAttackPower = profile->iAttackPower;
		boss.fCollisionRadius = profile->fCollisionRadius;
		boss.fEngageDistance = profile->fEngageDistance;
		boss.fMoveSpeed = profile->fMoveSpeed;

		// 공통 레이드 무력화 정책이면 현재 boss와 아직 살아 있는 retained Logic owner의 누적량을 새 최대에 맞춘다.
		// 이 경로를 처리하면 stage별 SET_STAGGER_GAUGE 재대조는 건너뛴다.
		const auto commonStagger = m_GameplayCatalog.Active().Get_RaidStaggerMaximum();
		if (commonStagger && Uses_RaidStaggerPolicy(boss))
		{
			if (boss.iKoukuItemStaggerMaximum)
			{
				boss.iKoukuItemStaggerCredit =
					ratio(boss.iKoukuItemStaggerCredit, boss.iKoukuItemStaggerMaximum, commonStagger, false);
				boss.iKoukuItemStaggerMaximum = commonStagger;
			}
			for (const auto& retained : boss.KoukuRetainedLogicOwners)
				if (const auto owner = retained.lock(); owner && owner->iKoukuItemStaggerMaximum)
				{
					owner->iKoukuItemStaggerCredit =
						ratio(owner->iKoukuItemStaggerCredit, owner->iKoukuItemStaggerMaximum, commonStagger, false);
					owner->iKoukuItemStaggerMaximum = commonStagger;
				}
			if (boss.BossCombat.iStaggerMaximum)
			{
				boss.BossCombat.iStaggerCurrent =
					ratio(boss.BossCombat.iStaggerCurrent, boss.BossCombat.iStaggerMaximum, commonStagger, false);
				boss.BossCombat.iStaggerMaximum = commonStagger;
				++boss.BossCombat.iStateRevision;
			}
			continue;
		}

		// 그 밖의 기존 무력화 gauge는 boss가 pin한 패턴의 현재 stage에서 ENTER SET_STAGGER_GAUGE를 찾아 조정한다.
		// 정의/패턴/stage가 없으면 기존 gauge를 유지하고 실제 최대를 바꾸면 상태 revision을 올린다.
		if (!boss.BossCombat.iStaggerMaximum)
			continue;
		const auto* pinned = m_GameplayCatalog.Resolve(boss.PinnedDefinitionRevision);
		const auto* patterns = pinned ? pinned->Find_BossPatterns(boss.strEncounterId) : nullptr;
		if (!patterns)
			continue;
		const auto pattern = std::find_if(patterns->begin(), patterns->end(),
			[&](const auto& p)
			{
				return p.strPatternId == boss.strPatternId;
			});
		if (pattern == patterns->end() || boss.iPatternStageIndex >= pattern->Stages.size())
			continue;
		for (const auto& action : pattern->Stages[boss.iPatternStageIndex].Actions)
			if (action.eKind == BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE && action.iValue &&
				action.eTrigger == BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER &&
				action.iValue != boss.BossCombat.iStaggerMaximum)
			{
				boss.BossCombat.iStaggerCurrent =
					ratio(boss.BossCombat.iStaggerCurrent, boss.BossCombat.iStaggerMaximum, action.iValue, false);
				boss.BossCombat.iStaggerMaximum = action.iValue;
				++boss.BossCombat.iStateRevision;
				break;
			}
	}
	return true;
}

// ServerApp의 준비/저장 실패가 catalog 예약을 취소하고 룸의 entry/remap 소유권을 비운다.
// catalog는 다른 비영 sequence를 무시하지만 이 wrapper의 두 배열은 그 경우에도 비워진다. 활성 gameplay 수치를 되돌리는 함수는 아니다.
void LostArk::Server::CGameRoom::Abort_NumericBalance(const std::uint32_t transactionSequence) noexcept
{
	m_GameplayCatalog.Abort_NumericBalance(transactionSequence);
	m_StagedNumericEntries.clear();
	m_StagedNumericCatalogRemaps.clear();
}
