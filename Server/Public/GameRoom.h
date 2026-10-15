#pragma once

// 수신 스레드에서 룸 큐로 넘기는 값 명령과 룸 간 이동 요청 형식을 사용한다.
#include "RoomCommand.h"
// 생성 준비 중 빌드 작업의 취소 여부를 차용한 atomic_bool로 확인한다.
#include <atomic>
// 룸이 소유할 플레이어 값 상태와 입장 후보·재화·피격 처리 형식을 사용한다.
#include "ServerPlayer.h"
// 룸의 보스·몬스터 등 월드 개체와 pattern occurrence 상태를 값으로 보관한다.
#include "ServerWorldEntity.h"
// 월드의 배치·진입 위치·trigger 설정을 읽는 카탈로그와 배치 형식을 사용한다.
#include "WorldBootstrap.h"
// 공유하는 불변 gameplay 세대와 스킬·보스·패턴 정의의 조회 형식을 제공한다.
#include "GameplayCatalog.h"
// 룸별 아이템 정의와 구매·장착·지급 검증 카탈로그를 소유한다.
#include "ItemCatalog.h"
// 장착 칭호의 유효 ID를 검사하는 룸별 카탈로그를 소유한다.
#include "HonorTitleCatalog.h"
// 승선·탈것 이동과 스킬에 필요한 정의 카탈로그를 소유한다.
#include "VehicleCatalog.h"
// 가이드 대사·trigger·행동 조합 정의를 소유하고 대사 예약에 GUIDE_TRIGGER를 받는다.
#include "GuideCatalog.h"
// 발탄 클리어 때 지급할 직업별 item ID 목록을 읽는 객체를 소유한다. 실제 지급은 룸이 맡는다.
#include "ValtanClearRewards.h"
// 플레이어의 스킬 시작과 진행을 처리하는 룸 소유 시스템을 선언한다.
#include "PlayerSkillSystem.h"
// 투사체 등 전투 개체 런타임과 stage 준비 transaction·고정 대상 형식을 사용한다.
#include "CombatObjectRuntime.h"
// 콜로세움 용병별 위협 평가 상태를 값 멤버로 보관한다.
#include "ColosseumThreatAssessment.h"
// 룸 navigation과 지면 점·지지면·탐색 계측 형식을 사용한다.
#include "ServerNavigation.h"
// 플레이어·보스 이동에 공유되는 룸 충돌 시스템을 소유한다.
#include "ServerCollisionSystem.h"
// 공간 trigger 처리와 상호작용 prompt·Mario 진입 판정 형식을 사용한다.
#include "ServerTriggerSystem.h"
// spawn 그룹의 정의·entry·anchor를 읽고 몬스터 생성 입력으로 받는다.
#include "SpawnGroupBootstrap.h"
// spawn 그룹의 웨이브 진행 상태를 룸 안에 소유한다.
#include "SpawnGroupRuntime.h"
// 일반 몬스터 판단을 실행하는 룸 소유 객체를 선언한다.
#include "MonsterBrain.h"
// 월드 NPC의 행동 진행을 처리하는 룸 소유 객체를 선언한다.
#include "NpcBehaviorRuntime.h"
// 발탄 본체와 종속 개체의 판단 객체를 소유하고 선택 기록을 조회한다.
#include "ValtanBrain.h"
// 쿠크 보스의 패턴 판단을 실행하는 룸 소유 객체를 선언한다.
#include "KoukuSaydonBrain.h"
// 쿠크 패턴의 판정 ledger·출력·대상 창과 카드미로·빙고 상태를 사용한다.
#include "KoukuSaydonLogicRuntime.h"
// 되돌릴 수 있는 기둥 상태를 일반 월드 파괴와 별도 런타임으로 소유한다.
#include "EncounterPropRuntime.h"
// 에스더 사용 시스템과 소환 예약이 차용하는 roster 항목 형식을 사용한다.
#include "EstherSkillSystem.h"
// 에스더 장판 예약이 차용하는 ZONE 형식을 사용한다.
#include "Gameplay/EstherStrikeContract.h"
// 워터팡 경기의 현재 event 표본 조회 형식을 선언한다.
#include "Gameplay/MaharakaWaterpangContract.h"
// 월드 파괴 정의와 pickup descriptor를 읽어 룸 안에 보관한다.
#include "WorldDestructionBootstrap.h"
// 파괴 상태·transaction·전이와 live event를 룸에서 관리한다.
#include "WorldDestructionRuntime.h"
// 입장 전 미리 조립한 송신 frame을 입장 후보가 값으로 소유한다.
#include "Network/PacketFrame.h"
// 입장 준비 실패를 연결 진단 reason으로 호출자에게 전달한다.
#include "Network/SessionDiagnostic.h"

// 큐 깊이·vector cursor·개수와 용량을 size_t로 표현한다.
#include <cstddef>
// wire ID·tick·revision과 누적 계측에 고정 폭 정수를 사용한다.
#include <cstdint>
// FIFO 입력·정리·월드 이동과 가이드 대사 예약 큐를 소유한다.
#include <deque>
// 플레이어 값과 ID/문자열 키별 runtime 상태를 정렬된 map에 보관한다.
#include <map>
// 불변 catalog와 입장 후보는 공유 소유하고 세션 목록은 weak_ptr로 차용하며 종속 brain은 단독 소유한다.
#include <memory>
// 수신 큐·정리 identity·공유 계측 조회를 룸 worker와 수신 경로 사이에서 동기화한다.
#include <mutex>
// 예약·최근 응답·실행 상태가 없는 경우를 기본값이 있는 실제 상태와 구분한다.
#include <optional>
// Mario 배치 선택과 장비 재련이 서로 다른 난수 엔진을 소유한다.
#include <random>
// 입장 batch 준비에서 앞서 준비한 후보 목록을 복사 없이 차용한다.
#include <span>
// stable ID·진단·원본 bytes와 대사 상태의 문자열을 소유한다.
#include <string>
// 실패 출처와 검증 메시지 입력을 호출 동안 복사 없이 받는다.
#include <string_view>
// session/player/entity ID와 요청 sequence를 키로 룸 내부 상태를 찾는다.
#include <unordered_map>
// 정리 대기 session과 가이드의 진입 공간 집합을 중복 없이 보관한다.
#include <unordered_set>
// 세대·플레이어 입장 frame·실행 개체·예약·관측 event의 가변 길이 목록을 소유한다.
#include <vector>

namespace LostArk::Server
{
	class CServerGameplayContractRunner;

	class CClientSession;

	// 룸은 승인한 catalog 객체를 제자리 수정하지 않고 shared_ptr<const>로 공유한다.
	// 진행 occurrence의 revision pin이 필요한 이전 세대를 보관하며 룸 실행 경로가 조회·교체·회수를 직렬로 호출한다.
	// 수치 변경도 객체를 교체한다. 같은 revision이라고 raw 포인터의 수명이 자동으로 유지되지는 않는다.
	class CGameplayCatalogGenerations final
	{
	public:
		// 일반 세대 보관 상한이다. 생성자가 용량을 예약하며 live pin 때문에 가득 차면 새 revision의 Stage를 거절한다.
		static constexpr std::size_t MAX_GENERATION_COUNT = 16u;

		// 룸 생성 시 일반 세대 vector 공간을 미리 확보한다. 메모리 할당 예외를 이 생성자가 처리하지는 않는다.
		CGameplayCatalogGenerations();

		// 초기 룸이 새 catalog를 로드해 Initialize한다. 명시적 로드 실패는 기존 세대를 유지하고 진단만 바꾼다.
		bool Load();

		// 유효한 초기 세대를 공유 소유하고 일반 보관 목록·stage·epoch를 초기화한다. 잘못된 입력은 false다.
		// 수치 예약을 정리하지 않으므로 실행 중 수치 transaction의 재설정 용도로 쓰지 않는다.
		bool Initialize(
			const std::shared_ptr<const CGameplayCatalog>& initialGeneration);

		// CGameRoom이 live pin을 회수한 뒤 일반 정의 세대를 예약한다. revision·sequence·용량 실패는 기존 예약을 유지한다.
		// 같은 sequence/revision 재요청은 true이며 status가 그대로일 수 있다. 승인 전 active는 바꾸지 않는다.
		bool Stage(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);

		// 예약 sequence를 확인해 active를 교체하고 epoch를 증가시킨다. false는 기존 예약을 남긴다.
		// 호출자는 전 방 준비를 끝내야 하며 noexcept이 진단 문자열 할당 실패의 복구를 보장하지는 않는다.
		bool Commit(std::uint32_t transactionSequence) noexcept;

		// CGameRoom의 수치 준비가 같은 active revision의 후보를 전달한다.
		// 과거 세대마다 choreography를 보존한 수치 복사본을 먼저 만들며 명시적 false에서는 active·보관 목록을 유지한다.
		bool Stage_NumericBalance(std::uint32_t transactionSequence,
			const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status);

		// 준비한 수치 vector와 active를 교체한다. 잘못된 sequence/빈 예약은 false이며 revision과 epoch는 유지한다.
		bool Commit_NumericBalance(std::uint32_t transactionSequence) noexcept;

		// 일치한 수치 예약만 해제한다. sequence 0은 무조건 취소이며 active는 되돌리지 않는다.
		void Abort_NumericBalance(std::uint32_t transactionSequence) noexcept;

		// 일반 stage의 공유 소유권만 해제한다. sequence 0은 무조건 취소, 다른 비영 sequence는 무시한다.
		void Abort(std::uint32_t transactionSequence) noexcept;

		// Tick 끝과 일반 Stage 직전이 살아 있는 occurrence의 revision을 전달한다.
		// active·일반 stage·live pin을 제외한 보관 소유권만 놓는다. 외부 shared_ptr가 있으면 객체는 계속 유지된다.
		void Collect_Garbage(
			const std::vector<LostArk::Shared::GameplayDataRevision>& livePins);

		// 보관 목록과 일반 stage에서 revision을 찾는다. 없거나 invalid이면 nullptr이며 수치 후보는 아직 노출하지 않는다.
		// 반환 포인터는 차용이다. 회수·재초기화·수치 Commit을 넘겨 보관하려면 별도 소유권 또는 revision 재조회가 필요하다.
		[[nodiscard]] const CGameplayCatalog* Resolve(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept;

		// 현재 active를 차용한다. 초기화 전에는 정적 빈 catalog를 반환하며 보관 소유권을 추가하지 않는다.
		[[nodiscard]] const CGameplayCatalog& Active() const noexcept;

		// 생성 준비·전역 세대 처리에 현재 객체의 공유 소유권을 돌려준다. 이후 active 교체와 반환 객체 수명은 분리된다.
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGeneration() const noexcept
		{
			return m_pActiveGeneration;
		}

		// 현재 일반 보관 목록의 크기다. 별도 수치 준비 vector의 항목 수는 더하지 않는다.
		[[nodiscard]] std::size_t Get_GenerationCount() const noexcept
		{
			return m_Generations.size();
		}

		// 초기화는 1, 일반 Commit은 1씩 증가시킨다. 수치만 교체할 때는 이 값이 바뀌지 않는다.
		[[nodiscard]] std::uint16_t Get_ActiveGenerationEpoch() const noexcept
		{
			return m_iActiveGenerationEpoch;
		}

		// 아래 Find 계열은 현재 Active의 조회를 그대로 전달한다. 과거 occurrence는 Resolve(pin)한 catalog를 직접 사용한다.
		// 반환 포인터·참조는 catalog 내부를 차용한다. ID가 없는 포인터 조회는 nullptr이며 값을 생성하지 않는다.
		// 스킬 입력은 stable SKILL_ID다.
		const PLAYER_SKILL_DEFINITION* Find_Skill(
			LostArk::Shared::SKILL_ID skillId) const;

		// 보스 생성·수치 갱신이 archetype ID로 현재 profile을 조회한다.
		const BOSS_RUNTIME_PROFILE* Find_Boss(
			const std::string& archetypeId) const;

		// archetype별 부위 목록을 차용한다. nullptr와 존재하지만 빈 목록을 구분할 수 있다.
		const std::vector<BOSS_PART_DEFINITION>* Find_BossParts(
			const std::string& archetypeId) const;

		// encounter별 현재 패턴 목록을 차용한다. 실행 중 pattern의 이전 세대를 자동 선택하지는 않는다.
		const std::vector<BOSS_PATTERN_DEFINITION>* Find_BossPatterns(
			const std::string& encounterId) const;

		// 전투 개체 생성·검증에 필요한 archetype 정의를 차용한다.
		const BOSS_COMBAT_OBJECT_DEFINITION* Find_BossCombatObject(
			const std::string& archetypeId) const;

		// 발탄 timeline 준비가 encounter별 timeline 정의를 차용한다.
		const VALTAN_TIMELINE_DEFINITION* Find_ValtanTimeline(
			const std::string& encounterId) const;

		// encounter와 stable command ID로 발탄 timeline의 한 행을 차용한다.
		const VALTAN_TIMELINE_ROW* Find_ValtanTimelineRow(
			const std::string& encounterId, std::uint32_t commandId) const;

		// encounter·gameplay phase·체력 줄 수를 catalog의 rotation 구간 판정에 전달한다.
		const BOSS_PATTERN_ROTATION_DEFINITION* Find_BossPatternRotation(
			const std::string& encounterId, std::uint32_t gameplayPhase,
			std::uint32_t healthBar) const;

		// 입장 패턴 ID를 차용한다. 정의가 없으면 catalog가 제공하는 정적 빈 문자열 참조다.
		const std::string& Find_IntroPatternId(
			const std::string& encounterId) const;

		// 생성·class 변경·수치 갱신이 class별 현재 profile을 차용한다.
		const PLAYER_RUNTIME_PROFILE* Find_Player(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;

		// 피해 profile의 percent를 값으로 반환한다. 정의가 없을 때의 0도 catalog 반환값 그대로다.
		std::uint32_t Find_DamageRatePercent(
			const std::string& damageProfileId) const;

		// snapshot·새 occurrence가 쓰는 현재 choreography revision을 차용한다. 수치 Commit은 같은 revision을 유지한다.
		[[nodiscard]] const LostArk::Shared::GameplayDataRevision&
			Get_ActiveRevision() const noexcept;

		// facade 자체 진단이 있으면 우선하고 없으면 active 진단을 차용한다. 외부 status 출력과 자동 동기화되지는 않는다.
		[[nodiscard]] const std::string& Get_Status() const noexcept;

		// 기존 catalog 입력 API에 Active 참조로 전달한다. 변환이 공유 소유권을 만들지는 않는다.
		operator const CGameplayCatalog&() const noexcept
		{
			return Active();
		}

	private:
		// 현재·일반 예약 객체와 그 sequence다. 일반 세대들은 m_Generations가 공유 소유한다.
		std::shared_ptr<const CGameplayCatalog> m_pActiveGeneration;
		std::shared_ptr<const CGameplayCatalog> m_pStagedGeneration;
		std::uint32_t m_iStagedTransactionSequence = 0u;
		std::uint16_t m_iActiveGenerationEpoch = 0u;
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_Generations;

		// 수치 교체는 별도 후보 vector에 모아 둔다. Commit이 일반 보관 vector와 바꾸고 이전 소유권을 해제한다.
		std::vector<std::shared_ptr<const CGameplayCatalog>> m_NumericStagedGenerations;
		std::shared_ptr<const CGameplayCatalog> m_pNumericStagedActive;
		std::uint32_t m_iNumericTransactionSequence = 0u;

		std::string m_strStatus;
	};

	// ServerApp이 직전에 마친 바깥 룸 loop의 스케줄 표본이다. 다음 Tick에서 모든 룸에 같은 값이 전달된다.
	// Sample은 Unix ms, lateness는 μs다. 늦어진 loop와 일정 재설정 횟수를 룸 자체 처리 시간과 구분한다.
	struct SERVER_ROOM_SCHEDULER_METRICS final
	{
		std::uint64_t iSampleUnixMilliseconds = 0u;
		std::uint64_t iPreviousLoopLatenessMicroseconds = 0u;
		std::uint64_t iMaximumLoopLatenessMicroseconds = 0u;
		std::uint64_t iScheduleResetCount = 0u;
	};

	// 수신 큐와 Tick·snapshot 처리의 누적/최근 표본이다. 각 접미 Microseconds는 μs이며 개수는 별도 필드다.
	// Get_PerformanceMetrics는 mutex 아래 값을 복사하므로 반환 이후의 계측은 그 복사본에 반영되지 않는다.
	struct SERVER_ROOM_PERFORMANCE_METRICS final
	{
		SERVER_NAVIGATION_PERFORMANCE_METRICS Navigation;
		SERVER_ROOM_SCHEDULER_METRICS Scheduler;

		// 정상 완료와 명시적 조기 중단에서 기록한 Tick 호출의 수·최근/최대 시간이다. 서버 tick 번호와 다르다.
		std::uint64_t iTickCount = 0;
		std::uint64_t iLastTickMicroseconds = 0;
		std::uint64_t iMaximumTickMicroseconds = 0;

		// Tick이 일반/정리 큐를 가져오던 깊이와 소비 후 잔량, 수신 중 최고 깊이를 구분한다.
		std::size_t iLastIngressDepth = 0;
		std::size_t iIngressHighWatermark = 0;
		std::size_t iLastDrainedCommandCount = 0;
		std::size_t iLastRemainingCommandCount = 0;
		std::size_t iLastCleanupIngressDepth = 0;
		std::size_t iCleanupIngressHighWatermark = 0;
		std::size_t iLastDrainedCleanupCommandCount = 0;
		std::size_t iLastRemainingCleanupCommandCount = 0;

		// 일반 drain 제한·MOVE/AIM 합치기·용량 거절과 LEAVE 중복/취소를 각각 누적한다.
		std::uint64_t iDrainLimitedTickCount = 0;
		std::uint64_t iCoalescedMoveCommandCount = 0;
		std::uint64_t iCoalescedAimCommandCount = 0;
		std::uint64_t iDroppedBestEffortCommandCount = 0;
		std::uint64_t iRejectedReliableCommandCount = 0;
		std::uint64_t iRejectedCleanupCommandCount = 0;
		std::uint64_t iDeduplicatedCleanupCommandCount = 0;
		std::uint64_t iCancelledCommandCountByCleanup = 0;

		// snapshot 조립과 수신자 enqueue를 별도로 측정한다. enqueue 성공은 wire 전송 완료를 뜻하지 않는다.
		std::uint64_t iSnapshotEncodeCount = 0;
		std::uint64_t iSnapshotEncodeFailureCount = 0;
		std::uint64_t iLastSnapshotEncodeMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEncodeMicroseconds = 0;
		std::uint64_t iSnapshotEnqueueBatchCount = 0;
		std::uint64_t iSnapshotRecipientCount = 0;
		std::uint64_t iSnapshotEnqueueFailureCount = 0;
		std::uint64_t iLastSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSnapshotEnqueueMicroseconds = 0;
		std::uint64_t iLastMaximumSessionEnqueueMicroseconds = 0;
		std::uint64_t iMaximumSessionEnqueueMicroseconds = 0;
	};

	// ServerApp의 수신 경로는 용량 초과·실패 룸·봉인·정리 대기를 구분해 연결 정책과 진단에 사용한다.
	// best-effort 유실과 중복 LEAVE 합침도 허용 결과이므로 bool 성공이 새 명령 보관을 뜻하지는 않는다.
	enum class ROOM_COMMAND_ENQUEUE_RESULT : std::uint8_t
	{
		ACCEPTED,
		DROPPED_BEST_EFFORT,
		DEDUPLICATED_CLEANUP,
		REJECTED_INVALID_COMMAND,
		REJECTED_ROOM_NOT_READY,
		REJECTED_ROOM_SEALED,
		REJECTED_PENDING_CLEANUP,
		REJECTED_RELIABLE_CAPACITY,
		REJECTED_BINDING_MISSING
	};

	// Enqueue와 ServerApp이 연결을 계속 처리할 수 있는 세 결과를 묶는다. 명령 실행 완료 여부는 판정하지 않는다.
	[[nodiscard]] constexpr bool Is_AcceptedRoomCommandEnqueueResult(
		const ROOM_COMMAND_ENQUEUE_RESULT result) noexcept
	{
		return ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DROPPED_BEST_EFFORT == result ||
			ROOM_COMMAND_ENQUEUE_RESULT::DEDUPLICATED_CLEANUP == result;
	}

	// Mark_RuntimeFailure가 최초 실패의 당시 서버 tick·출처·상세를 보관하고 조회자에게 복사한다.
	struct SERVER_ROOM_RUNTIME_FAILURE final
	{
		std::uint32_t iServerTick = 0u;
		std::string strSource;
		std::string strDetail;
	};

	// ServerApp이 한 simulation의 수명을 소유한다. 정상 gameplay 변경은 룸 worker가 직렬로 실행한다.
	// 수신 경로는 Enqueue로 값 명령을 전달하며 command mutex가 모든 gameplay 멤버를 보호하는 것은 아니다.
	// 준비 생성과 worker 합류 뒤 종료 중 제어는 해당 worker와 겹치지 않는 호출자 수명 관리가 전제다.
	class CGameRoom final
	{
		friend class CServerGameplayContractRunner;
		friend int Run_ServerKoukuSupportSurfaceContractTests();
        friend int Run_ServerBingoContractTests();
		friend int Run_ServerCardMazeContractTests();
        friend int Run_ServerKoukuObjectOverlapContractTests();
		friend int Run_ServerVehicleRidingContractTests();
	public:
		// ServerApp의 시작·개인 방·콜로세움 준비가 월드와 선택적 초기 세대를 전달한다.
		// 취소 atomic 포인터는 생성 호출 동안만 차용한다. 필수 로드 실패 객체는 Is_Ready가 false이고 Get_Status로 확인한다.
		// 실패 전 만든 멤버는 객체가 소유하며, ready 확정 뒤 가이드 부가 초기화 실패는 룸 전체 준비를 취소하지 않는다.
		explicit CGameRoom(
			LostArk::Shared::WORLD_ID worldId,
			std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration = {},
			const std::atomic_bool* pPreparationCancelled = nullptr);

		// Enqueue_Detailed의 결과를 허용 bool로 줄인다. MOVE/AIM 유실과 LEAVE 중복도 true일 수 있다.
		bool Enqueue(ROOM_COMMAND command);

		// 수신/제어 경로가 값을 넘긴다. session 식별자 검증 후 mutex 아래 ready·봉인·정리 대기·용량을 검사한다.
		// 허용한 명령은 큐가 소유한다. LEAVE는 별도 우선 큐에서 session별로 합치고 공유 일반 큐의 같은 session 명령을 지운다.
		// 이미 Tick 지역 큐로 가져온 명령까지 취소하는 것은 아니다. MOVE/AIM은 stream 경계 안에서 최신 값으로 합친다.
		[[nodiscard]] ROOM_COMMAND_ENQUEUE_RESULT Enqueue_Detailed(
			ROOM_COMMAND command);

		// ServerApp이 판정 enum에 현재 큐 깊이·봉인·최초 실패를 붙인다. 원래 enqueue 시점의 snapshot은 아니다.
		[[nodiscard]] std::string Describe_EnqueueResult(
			ROOM_COMMAND_ENQUEUE_RESULT result) const;

		// 진단/검증 경로가 mutex 아래 최초 실패를 복사한다. 기록이 없으면 false이며 outFailure는 보존한다.
		[[nodiscard]] bool Try_GetRuntimeFailure(
			SERVER_ROOM_RUNTIME_FAILURE& outFailure) const;

		// ServerApp 룸 worker가 delta 초와 스케줄 표본을 전달한다. 정리 전체/일반 최대 256개를 잠금 밖에서 처리한다.
		// 그 뒤 simulation·송신을 순서대로 진행하며 중간 실패는 이미 바뀐 상태를 되돌리지 않는다.
		// ready가 아니면 즉시 반환한다. 잘못된 delta도 앞선 명령 소비까지 취소하지는 않는다.
		void Tick(float fixedDeltaSeconds,
			const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics = {});

		// Tick 직후 같은 룸 worker의 ServerApp이 최신 진단 한 줄을 가져가며 멤버를 비운다. 별도 mutex는 없다.
		[[nodiscard]] std::string Take_PerformanceDiagnostic();

		// 모든 룸 Tick 뒤 ServerApp이 FIFO 요청을 이동해 받는다. 빈 큐는 false이며 출력은 유지한다.
		// 성공 시 Bern 선박 복귀 기억도 갱신하지만 실제 월드 전환 성공이 확정된 것은 아니다.
		bool Try_DequeueWorldTransfer(
			SERVER_WORLD_TRANSFER_REQUEST& outTransfer);
		bool Configure_ColosseumMatch(std::uint64_t matchId, const std::vector<SESSION_ID>& sessions);
		void Remove_ColosseumExpectedSession(SESSION_ID sessionId);

		// 생성 시 결정한 simulation의 월드 ID다. shared/private 여부를 이 ID만으로 판별하지 않는다.
		[[nodiscard]] LostArk::Shared::WORLD_ID Get_WorldId() const
		{
			return m_eWorldId;
		}

		// 준비 또는 룸 실행 실패 상태를 그대로 읽는다. 이 getter 자체는 mutex/atomic 동기화를 제공하지 않는다.
		[[nodiscard]] bool Is_Ready() const
		{
			return m_isReady;
		}

		// 현재 진단 문자열을 차용한다. 다른 제어/실행이 갱신할 수 있으므로 호출자는 룸 접근 순서를 지켜야 한다.
		[[nodiscard]] const std::string& Get_Status() const
		{
			return m_strStatus;
		}

		// ServerApp은 룸 worker에서 전 방 수치 후보를 준비하고 디스크 저장 성공 뒤 확정한다.
		// 종료 중에는 worker 합류 뒤 같은 제어를 수행한다. Stage는 snapshot·별도 catalog 수치 복사본을
		// 지역 값에서 완성하며 명시적 false는 이전 룸 예약 배열을 유지한다. 할당 예외는 여기서 잡지 않는다.
		bool Stage_NumericBalance(std::uint32_t transactionSequence,
			const std::shared_ptr<const CGameplayCatalog>& candidate, std::string& status);

		// 같은 sequence의 세대를 교체한 뒤 player·투사체·boss의 실제 수치와 비율을 조정한다.
		// 포인터 교체만 하는 함수가 아니며 noexcept이 이후 변경의 rollback을 보장하지는 않는다.
		bool Commit_NumericBalance(std::uint32_t transactionSequence) noexcept;

		// catalog는 다른 비영 sequence를 무시하지만 이 wrapper의 예약 entry/remap 배열은 항상 비운다.
		void Abort_NumericBalance(std::uint32_t transactionSequence) noexcept;

		// 전역 정의 교체 준비가 live pin 구성·미사용 세대 회수 뒤 후보를 예약한다.
		// 예약 실패여도 앞서 회수한 미사용 세대 소유권까지 복구하지는 않는다.
		bool Stage_GameplayGeneration(
			std::uint32_t transactionSequence,
			const LostArk::Shared::GameplayDataRevision& baseRevision,
			const std::shared_ptr<const CGameplayCatalog>& candidateGeneration,
			std::string& status);

		// ServerApp이 전 방 준비와 참여자 승인을 확인한 fixed-step 경계에서 확정한다.
		// 성공 시 비보스 identity는 즉시 갱신하고 보스는 다음 brain 평가가 갱신한다. 진행 occurrence pin은 유지한다.
		bool Commit_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;

		// ServerApp의 준비/승인 실패가 일반 예약만 해제한다. sequence 0은 무조건 취소이며 active는 보존한다.
		void Abort_GameplayGeneration(
			std::uint32_t transactionSequence) noexcept;

		// ServerApp의 입장·전환 세대 일치 검사와 계약 검증이 active 공유 소유권을 받는다. 최초 초기화 전에는 비어 있을 수 있다.
		[[nodiscard]] std::shared_ptr<const CGameplayCatalog>
			Get_ActiveGameplayGeneration() const noexcept
		{
			return m_GameplayCatalog.Get_ActiveGeneration();
		}

		// 현재 호출자는 세대 보존/회수 계약 검증이다. 반환 포인터는 차용이며 회수/수치 교체 뒤에는 다시 조회한다.
		[[nodiscard]] const CGameplayCatalog* Resolve_GameplayGeneration(
			const LostArk::Shared::GameplayDataRevision& revision) const noexcept
		{
			return m_GameplayCatalog.Resolve(revision);
		}

		// ServerApp 제어 이벤트가 룸 worker에서 발탄 brain의 최신 선택 기록을 wire 값으로 복사한다.
		// 잘못된 world·보스 부재·새 기록 없음도 typed 응답을 만든 true다. false는 내부 기록 계약 오류이며 이전 응답은 보존한다.
		// 송신은 호출자 책임이다. 관측한 revision 값 자체가 세대 수명을 붙잡는 live pin은 아니다.
		bool Build_ValtanDecisionTraceResponse(
			const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request,
			LostArk::Shared::S2C_VALTAN_DECISION_TRACE_RESPONSE& outResponse,
			std::string& status) const;

		// 성능 로그/검증 경로가 mutex 아래 최근 계측값의 복사본을 받는다.
		[[nodiscard]] SERVER_ROOM_PERFORMANCE_METRICS
			Get_PerformanceMetrics() const;

		// ServerApp 룸 worker가 비어 있는 Character Select 방의 회수 직전에 호출한다.
		// 큐·전환·session/player lookup이 비면 입력을 봉인한다. 이미 봉인된 방도 true이며 객체 삭제는 호출자 책임이다.
		[[nodiscard]] bool Try_SealPrivateArenaForRetirement();

		// 일반 단독 전환에서 룸 worker가 대상 REGISTER/ENTER를 예약한 뒤 source Leave를 확정한다.
		// 다음 대상 Tick 전에 기존 player binding을 해제한다. 준비 안 됨/없는 session은 false다.
		[[nodiscard]] bool Commit_WorldTransferDeparture(SESSION_ID sessionId);

		// ServerApp의 월드 전환과 Transfer_PartyTo가 단독 guide owner의 admission 경로를 선택한다.
		// 룸 실행 스레드에서 ServerApp의 session-binding mutex를 보유한 호출 전제이며 내부 잠금은 없다.
		// Bern의 같은 실제 session·안내 actor·binding만 허용한다. 월드 이탈 대기는 binding이 없는 경우에 true다.
		[[nodiscard]] bool Has_PersonalGuideOwner(const std::shared_ptr<CClientSession>& ownerSession) const;
		bool Transfer_PartyTo(CGameRoom& target,
			const std::vector<SESSION_ID>& leaderFirstSessionIds,
			LostArk::Shared::PARTY_TRANSFER_RESULT& outResult, std::string& status,
			const std::string& raidReturnNpcPlacementId = {},
            const std::string& spawnPlacementOverrideId = {});
		bool Transfer_ColosseumMatchTo(CGameRoom& target,
			const std::vector<SESSION_ID>& orderedSeats, std::uint64_t matchId, std::string& status);
		void Notify_ColosseumTransferResult(bool committed);
		[[nodiscard]] bool Try_SealColosseumForRetirement();
		void Notify_PartyTransferFailure(SESSION_ID sessionId,
			std::uint32_t requestSequence, LostArk::Shared::WORLD_ID targetWorldId,
			LostArk::Shared::PARTY_TRANSFER_RESULT result);

	private:
		// Tick 등 룸 실패 경로가 ready를 내리고 최초 tick·출처·현재 진단만 기록한다. 기존 변경 rollback이나 큐 비우기는 하지 않는다.
		void Mark_RuntimeFailure(std::string_view source);

		// 레이드 인원·빈 방 정리·Esther 갱신이 m_Players의 인간 제어 항목만 센다. HP가 0인 인간도 포함하고 AI는 제외한다.
		std::size_t Count_HumanPlayers() const;

		// 생성자가 필수 룸 준비 뒤 안내 catalog와 설치된 차원술사 skill binding을 확인한다.
		// 지원 world에서 읽되 actor는 Bern에만 만든다. 안내 실패는 로그로 남기며 이미 설정한 룸 ready를 내리지 않는다.
		void Initialize_Guide();

		// 초기 생성/부활이 지정한 player/entity ID와 world XYZ(m)로 지역 GUIDE_AI 값을 준비한다.
		// 정의·ID·snapshot 수용량·착지 검사 실패는 outPlayer를 보존한다. 성공도 여기서 룸 삽입이나 session 생성까지 하지 않는다.
		bool Build_GuidePlayer(LostArk::Shared::PLAYER_ID playerId, LostArk::Shared::NET_ENTITY_ID entityId,
			float x, float y, float z, SERVER_PLAYER& outPlayer) const;

		// 생성·부활·회피·도착 경로가 중심과 3m 이내 고정 후보 중 첫 허용 위치를 찾는다. 성공할 때만 point를 바꾼다.
		// 입력 참조는 호출 중 차용한다. anchor가 있으면 navigation 연결이 필수이며 충돌·다른 살아 있는 player 겹침도 검사한다.
		bool Find_GuideLanding(const SERVER_PLAYER& guide, float x, float y, float z, SERVER_NAV_POINT& point,
			const SERVER_PLAYER* anchor = nullptr) const;

		// GUIDE_CONTROL START가 기존 Bern 안내 actor에 최대 한 owner를 연결한다. 실제 session은 weak_ptr로 저장한다.
		// false는 target이 guide가 아님을 뜻한다. true에는 거리/상태 거절과 기존 owner 유지도 포함하므로 새 시작 성공값이 아니다.
		bool Start_Guide(const SERVER_PLAYER& inviter, LostArk::Shared::NET_ENTITY_ID target);

		// Tick의 명령 drain이 Bern START/STOP을 처리한다. 새 요청 sequence는 actor/binding 확인보다 먼저 기록한다.
		// STOP은 이 session의 안내 예약만 해제한다. 거절 요청에 별도 결과 packet을 보내는 함수는 아니다.
		void Handle_GuideControl(SESSION_ID sessionId, const LostArk::Shared::C2S_GUIDE_CONTROL& request);

		// 시작/해제와 owner 부재 대기 갱신이 단일 안내 actor의 소유 표시를 만든다. actor가 없으면 송신하지 않는다.
		void Broadcast_GuideOwnership();

		// 소유 표시/판단 trace를 방 안의 인간 session들에게 보낸다. 직렬화 실패는 송신 없이 끝낸다.
		// 개별 enqueue 실패는 해당 연결 종료를 요청하며, 이미 바뀐 안내 상태나 다른 수신자의 송신을 되돌리지 않는다.
		void Broadcast_GuideState(const LostArk::Shared::S2C_GUIDE_STATE& message);

		// Tick이 player 이동 갱신 전에 초 단위 고정 dt로 호출한다. 현재 실행 입구는 catalog가 Loaded인 Bern뿐이다.
		// 대사 처리와 판단 주기를 나누고 이동·skill은 공통 player executor에 제출한다. 실패 시 전체 상태 rollback은 없다.
		void Update_Guides(float seconds);
		void Update_Colosseum(float seconds);
		void Handle_ColosseumRecruit(SESSION_ID sessionId, const LostArk::Shared::C2S_COLOSSEUM_RECRUIT& request);
		LostArk::Shared::S2C_COLOSSEUM_MATCH_STATE Build_ColosseumState() const;
		void Broadcast_ColosseumState();

		// Update_Guides가 후보 world XZ(m)의 게시 패턴·Kouku ledger·빙고 망치 접촉 위험을 읽는다.
		// guide 높이/identity를 지역 probe에 복사하며 실제 player는 바꾸지 않는다. 반환은 피해량/확률이 아닌 근사 점수다.
		float Predict_GuideContactRisk(const SERVER_PLAYER& guide, float x, float z);

		// 확정된 owner 이동과 먼 거리 복구가 안내 접근 상태를 다시 준비한다. 인간 전용 기믹/월드 대기 owner는 건너뛴다.
		// Bern localMapTravel은 선박에 타지 않았을 때 연결된 착지를 먼저 검사한다. 실패는 Reason만 바꾸고 actor/실행 상태를 유지한다.
		// 그 밖에는 현재 위치에서 도보 접근하도록 reset한다. 실제 공간 진입 edge를 남기려고 InsideBoxes는 비우지 않는다.
		void Guide_AnchorArrived(const SERVER_PLAYER& anchor, bool localMapTravel = false);

		// Handle_Chat이 인간 owner의 문자열을 전달한다. 앞뒤 ASCII 공백 제거 후 정확한 별칭만 찾으며 일반 채팅 송신은 호출자가 계속한다.
		// 월드/선박 대기 중에는 무시한다. Stop은 명령 cooldown을 우회해 combo 예약을 비우지만 진행 중 skill을 강제로 끝내지는 않는다.
		void Guide_ChatCommand(const SERVER_PLAYER& sender, const std::string& text);

		// STOP·일반 Leave·owner/actor 소실이 안내 제어와 대사 예약을 해제한다. 배치 actor는 남기고 이동/skill과 비행 입력을 정리한다.
		// publish=false는 다른 outbound transaction 안에서 즉시 안내 송신을 피한다. owner runtime이 없으면 변경하지 않는다.
		void Remove_Guide(SESSION_ID ownerSessionId, bool publish = true);

		// Leave의 LEVEL_CHANGED가 Bern 안내를 현 위치에 대기시킨다. weak owner/actor identity는 보존하고 combo·대사·공간 접촉을 비운다.
		// 입장 transaction 중에도 호출하므로 여기서는 packet을 직접 보내지 않는다.
		void Suspend_PersonalGuide(SESSION_ID ownerSessionId);

		// 단독/파티 입장 commit 뒤 Bern 복귀를 알린다. 같은 실제 owner session과 새 player binding을 확인한 경우만 추적을 재개한다.
		// sourceWorld에 맞는 복귀 대사는 예약만 하고 다음 Update_Guides가 송신한다. 이 함수가 actor를 순간이동시키지는 않는다.
		void Resume_PersonalGuide(SESSION_ID ownerSessionId, LostArk::Shared::WORLD_ID sourceWorld);

		// 시작/복귀 시 owner가 이미 닿은 SPACE_ENTER trigger ID를 현재 접촉 기준으로 기록한다. 대사 예약이나 cooldown 소비는 하지 않는다.
		void Seed_GuideSpaceEntries(SESSION_ID ownerSessionId, const SERVER_PLAYER& anchor);

		// 예약/갱신이 미시작 공간 대사에서 더 이상 접촉하지 않는 source를 지운다. 남은 source의 최고 priority로 다시 정렬한다.
		// 첫 segment가 이미 송신된 대사는 접촉 이탈로 취소하지 않고 앞에 유지한다.
		void Prune_GuideSpacePrompts(SESSION_ID ownerSessionId);

		// 시작·복귀·도움·공간/보스 사건을 owner별 대사 큐에 예약한다. 같은 prompt는 진행 상태·source·priority에 따라 합치거나 유지/교체한다.
		// 최대 16개이며 가득 차면 새 예약을 버린다. 여기서는 송신하거나 trigger cooldown을 소비하지 않는다.
		void Queue_GuidePrompt(SESSION_ID ownerSessionId, const GUIDE_TRIGGER& trigger);
		void Execute_PlayerMove(SERVER_PLAYER& player, const LostArk::Shared::C2S_MOVE& move);
		bool Execute_PlayerSkill(SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& skill);

		// 입장 준비가 소유하는 임시 값이다. 룸 membership과 실제 reliable FIFO는 호출자가 별도로 commit한다.
		struct STAGED_PLAYER_ENTRY final
		{
			// Stage부터 packet/FIFO 준비까지 실제 연결 수명을 공유 소유한다. 룸의 m_Sessions 등록은 weak_ptr다.
			std::shared_ptr<CClientSession> pSession;

			// 목적 룸 profile·착지·이월 inventory로 만든 값이며 Stage만으로 m_Players에 들어가지는 않는다.
			SERVER_PLAYER Player;

			// 인코딩한 초기 메시지 payload를 소유한다. Build_PlayerEntryFrames가 완성한 목록만 교체한다.
			std::vector<LostArk::Shared::PACKET_FRAME> Frames;
		};

		// 단독 Join과 파티/콜로세움 전환이 목적 룸의 후보를 준비한다. precedingEntries는 같은 배치의 ID offset과 spawn 예약이다.
		// 실패는 출력 후보/룸 membership을 보존하고 outReason/status를 바꾼다. 성공도 송신·ID 증가·player binding까지 하지 않는다.
		bool Stage_PlayerEntry(const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			std::span<const STAGED_PLAYER_ENTRY> precedingEntries,
			STAGED_PLAYER_ENTRY& staged,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outReason, std::string& status,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {},
			const SERVER_PURSE& carriedPurse = {}, bool hasCarriedCharacterState = false);

		// 위 입장 경로가 승인/revision부터 월드 상태·기존/신규 player까지 FIFO 순서로 준비한다.
		// false는 entry.Frames를 보존한다. 성공한 payload를 실제 session 큐에 예약하는 책임은 호출자에게 있다.
		bool Build_PlayerEntryFrames(STAGED_PLAYER_ENTRY& entry,
			std::span<const STAGED_PLAYER_ENTRY> batch, std::string& status);

		// 단독 Join이 FIFO 예약 성공 뒤 membership/lookup/ID와 연결 binding을 확정한다.
		// 할당 예외의 중간 삽입 rollback은 보장하지 않는다. 일괄 전환은 이 함수 대신 준비한 map들을 swap한다.
		void Commit_PlayerEntry(const STAGED_PLAYER_ENTRY& entry);
		void Flush_PartyTransferResults();

		// Tick REGISTER_SESSION이 유효한 실제 연결을 약하게 등록한다. player 생성/binding은 뒤의 Join 단계다.
		void Handle_Register(const std::shared_ptr<CClientSession>& session);

		// Tick ENTER_WORLD가 지역 player·초기 packet·reliable FIFO를 준비한 뒤 입장을 확정한다.
		// 명시적 준비 실패는 membership을 만들지 않고 연결 종료를 요청한다. true는 로컬 commit이며 Client 수신 완료는 아니다.
		bool Join(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_ENTER_WORLD& enterWorld,
			const std::string& spawnPlacementOverrideId = {},
			const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>&
				carriedInventory = {},
			LostArk::Shared::HONOR_TITLE_ID carriedHonorTitleId =
				LostArk::Shared::INVALID_HONOR_TITLE_ID,
			const std::string& raidReturnNpcPlacementId = {},
			const SERVER_PURSE& carriedPurse = {},
			LostArk::Shared::WORLD_ID sourceWorld = LostArk::Shared::WORLD_ID::BERN,
			bool hasCarriedCharacterState = false);

		// 정리 큐와 월드 전환이 session 예약부터 player/lookup·파티·빈 방 상태까지 순서대로 해제한다.
		// publishDeparture는 안내/대기열/명시적 despawn 통지 일부를 제어한다. 모든 하위 종료 함수의 송신을 막는 값은 아니다.
		// Leave는 초기화 실패를 로그로 남기며 이미 끝낸 퇴장을 되돌리지 않는다. 하위 초기화 실패는 룸을 실패 상태로 내릴 수 있다.
		// 이 함수 본문은 룸 mutex를 직접 잡지 않는다.
		void Leave(
			SESSION_ID sessionId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason, bool publishDeparture = true);

		// 명령 handler가 session/player lookup 불일치를 공통 진단으로 종료 요청한다.
		// 살아 있는 연결이 없으면 무시하며 실제 룸 상태 제거는 후속 Leave에 맡긴다.
		void Close_SessionForBindingFailure(
			SESSION_ID sessionId,
			std::string_view packetName,
			std::string_view validation);
		void Handle_Move(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_MOVE& move);
		[[nodiscard]] bool Is_BufferableComboAction(
			const SERVER_PLAYER& player) const;
		// 현재 스킬의 이동 취소 구간인지 확인해, 허용된 이동 명령이 후딜레이를 끝낼 수 있게 한다.
		[[nodiscard]] bool Is_MoveCancellableAction(
			const SERVER_PLAYER& player) const;
		[[nodiscard]] bool Commit_MoveGoal(
			SERVER_PLAYER& player, float goalX, float goalZ);
		void Commit_PendingPlayerCommand(
			SERVER_PLAYER& player, std::uint32_t actionStartTick);
		void Handle_UseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SKILL& useSkill);
		void Handle_ReleaseSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RELEASE_SKILL& releaseSkill);
		void Handle_UpdateSkillAim(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPDATE_SKILL_AIM& updateSkillAim);
		void Handle_UseEstherSkill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ESTHER_SKILL& useEstherSkill);
		// Debug F1은 게이지·슬롯 선택을 생략하고 같은 Esther 시전·소환 경로를 쓴다. Release는 무시한다.
		void Handle_DebugUseEsther(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_USE_ESTHER& request);
		// session 소유 플레이어가 생존·대기·비탑승 등 시전 조건을 만족할 때만 포인터를 빌려준다.
		SERVER_PLAYER* Find_EstherCaster(SESSION_ID sessionId, const char* pCommandName);
		// 조준 방향의 소환을 예약하고 시전자를 ESTHER_CAST로 묶는다. 게이지 차감 여부는 호출자가 결정한다.
		void Begin_EstherCall(
			SERVER_PLAYER& caster,
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			float aimX,
			float aimZ);
		void Handle_UseSquareHole(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_SQUAREHOLE& useSquareHole);
		// 비활성 squarehole.<id> 배치에서 착지점을 찾고 지면·높이·충돌을 검증한다. 착지할 수 없으면 false다.
		bool Resolve_SquareHoleDestination(
			const SERVER_PLAYER& player,
			std::uint16_t squareHoleId,
			SERVER_NAV_POINT& ground);
		// 노래 종료 후 목적지로 이동하며, 착지 검증에 실패하면 현재 위치를 유지한다.
		void Finish_SquareHoleSong(SERVER_PLAYER& player);
		bool Spawn_EstherSummon(
			const ESTHER_ROSTER_ENTRY& rosterEntry,
			LostArk::Shared::PLAYER_ID casterPlayerId,
			float positionX,
			float positionY,
			float positionZ,
			float yawDegrees);
		void Update_PendingEstherSummons(float fixedDeltaSeconds);
		void Apply_EstherStrikeHits(SERVER_WORLD_ENTITY& summon, std::uint32_t serverTick);
		void Open_EstherZone(const LostArk::Shared::EstherStrike::ZONE& zone, float positionX, float positionZ, std::uint32_t serverTick);
		void Update_EstherZones(std::uint32_t serverTick);
		void Handle_RevivePlayer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REVIVE_PLAYER& revivePlayer);
		// Debug에서 자기 HP를 0, 행동을 DEAD로 만들어 사망 화면을 확인한다. Release는 무시한다.
		void Handle_DebugKillSelf(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KILL_SELF& debugKillSelf);
		// Debug Character Select에서 기존 Server 월드 전환을 예약한다. Client Level 변경은 승인 흐름이 담당한다.
		void Handle_DebugEnterKoukuSaydonArena(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_ENTER_KOUKUSAYDON_ARENA& request);
		// Debug Kouku waypoint 이동은 playerSpawn·Server 지면·충돌을 검증한다. 시작점 ID는 별도 복귀 경로를 쓴다.
		void Handle_DebugTeleportToPlacement(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_PLACEMENT& request);
		void Handle_DebugTeleportToPosition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugTeleportToPosition(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request);
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT Apply_DebugReturnToKoukuStart(
			SERVER_PLAYER& player, std::uint32_t requestSequence);
		void Reset_PlayerForDebugTeleport(SERVER_PLAYER& player);
		void Handle_DebugMarioJump(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT Apply_DebugMarioJump(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request);
		void Handle_MarioMove(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_MOVE& request);
		void Handle_MarioReturn(SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_RETURN& request);
		LostArk::Shared::S2C_MARIO_RETURN_RESULT Apply_MarioReturn(
			SERVER_PLAYER& player, const LostArk::Shared::C2S_MARIO_RETURN& request);
		bool Resolve_MarioReturnDestination(const SERVER_PLAYER& player, SERVER_NAV_POINT& destination) const;
		static void Reset_MarioContactAction(SERVER_PLAYER& player);
		// Tick의 trigger callback이 Mario lane과 최종 출구를 검증한다. 기본 처리·내부 재시도·시작 완료를 enum으로 구분한다.
		// 이동은 player 복사본에서 성공한 뒤 반영하지만 진단/충돌 보조 상태까지 모두 되돌리는 함수는 아니다.
		SERVER_TRIGGER_MOVE_ENTRY_RESULT Begin_MarioTriggerMove(
			const WORLD_BOOTSTRAP_PLACEMENT& trigger, SERVER_PLAYER& player,
			std::uint32_t actionStartTick);
		void Update_MarioControlState(SERVER_PLAYER& player);
		std::uint8_t Begin_MarioStageObjects(std::uint8_t stage);
		void Begin_MarioBallChallenge(SERVER_PLAYER& player);
		std::uint8_t Mario_MatchingBallCount(const SERVER_PLAYER& player) const;
		std::uint8_t Mario_MarkerColor(LostArk::Shared::NET_ENTITY_ID targetId) const;
		void Reset_MarioStageObjects(std::uint8_t stage);
		void Cleanup_EmptyMarioStages();
		void Update_MarioMoveGoal(SERVER_PLAYER& player, std::uint32_t updateTick);
		bool Configure_MarioRail(SERVER_PLAYER& player, const std::string& arrivalPlacementId);
		// Debug 빙고 칸과 완성 줄을 갱신한다. 별도 응답 없이 월드 snapshot으로 전달한다.
		void Handle_DebugBingoFill(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_FILL& request);
		// Debug 빙고 폭탄 표식을 요청자에게 붙여 월드 snapshot으로 전달한다.
		void Handle_DebugBingoBomb(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_BOMB& request);
		// Debug 빙고 망치는 행·열의 20개 시작점 중 하나에서 움직이고 월드 snapshot에 반영된다.
		void Handle_DebugBingoHammer(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_BINGO_HAMMER& request);
		// Debug 일반 몬스터 버튼은 대응 SpawnGroup의 생존 몬스터를 지우고 원래 배치에서 재시작한다.
		// Release는 무시하며 별도 응답 없이 월드 snapshot으로 전달한다.
		void Handle_DebugResummonWaveMonsters(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_RESUMMON_WAVE_MONSTERS& request);
		// Debug에서 요청자의 광대 변신과 관련 행동 상태를 정리해 반영한다. Release는 REJECTED_DISABLED로 응답한다.
		void Handle_DebugSetMadnessForm(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT Apply_DebugMadnessForm(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request);
		// 요청자 탑승 여부를 판정해 응답한다. 실제 탈것 상태는 월드 snapshot으로 전달한다.
		void Handle_SetVehicleRiding(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT Apply_SetVehicleRiding(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_VEHICLE_RIDING& request);
		// 플레이어의 생존·행동·구속 상태 등이 탑승을 허용하는지 검사한다.
		bool Can_RideVehicle(const SERVER_PLAYER& player) const;
		// 요청자의 칭호 변경을 판정해 응답한다. 장착된 칭호는 월드 snapshot으로 전달한다.
		void Handle_SetHonorTitle(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		LostArk::Shared::S2C_SET_HONOR_TITLE_RESULT Apply_SetHonorTitle(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_HONOR_TITLE& request);
		// 이동과 snapshot이 함께 쓰는 속도(m/s)다. 탈것·선박 가속 또는 기본 속도·stance·워터팡 가속을 반영한다.
		float Resolve_PlayerMoveSpeed(const SERVER_PLAYER& player) const;
		// 매 tick의 snapshot 전에 월드·catalog·플레이어 상태상 탑승 불가인 플레이어를 하차시킨다.
		void Enforce_VehicleRidingState();
		// 승선은 BernSea의 가까운 탐색 반경에서 바다를 찾고 부두 위치를 보관한다. 찾지 못하면 상태를 유지하고 false다.
		// 하차·강제 하차는 저장한 부두 위치와 방향을 복원한다.
		bool Begin_ShipVoyage(SERVER_PLAYER& player, LostArk::Shared::VEHICLE_ID vehicleId);
		void End_ShipVoyage(SERVER_PLAYER& player, const char* reason);
		// 탑승한 탈것의 스킬·sequence·쿨타임·상태를 검사한다. 일반 시전 외에 비행 E 토글도 이 경로에서 처리한다.
		bool Try_StartVehicleSkill(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_USE_SKILL& command);
		// 고정 tick마다 탈것 스킬을 진행한다. 비행은 이륙·이동·착륙 상태를, 지상 스킬은 지면·충돌을 반영한다.
		void Update_VehicleSkill(SERVER_PLAYER& player, float fixedDeltaSeconds);
		void End_VehicleSkill(SERVER_PLAYER& player);
		// 현재 HUD 슬롯의 스킬을 sequence·상태·쿨타임 검증 후 시작한다. DANCE 입력은 열린 pose 판정 창에도 기록한다.
		void Handle_InteractionSlot(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACTION_SLOT& request);
		// HUD 강제 모드를 판정해 요청자에게 응답한다. Release는 Kouku의 NONE/MARIO/MAZE만 허용한다.
		void Handle_DebugSetKoukuHudMode(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT Apply_DebugKoukuHudMode(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request);
		// 매 tick encounter의 광기 상한, 광대 유지 시간, 플레이어별 HUD 모드와 슬롯을 갱신한다.
		void Update_KoukuPlayerModes(std::uint32_t serverTick);
		void Apply_KoukuGateEntryCard(SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss);
		void Handle_ChangeCharacterClass(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT Apply_CharacterClassChange(
			SERVER_PLAYER& player,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request);
		void Handle_SpawnWorldEntity(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SPAWN_WORLD_ENTITY& request);
		// stable ID·revision 기반 Play/Restart/Next와 아레나 preset 요청을 판정한다. 구 HP·timeline 명령은 거절한다.
		// Evaluate가 상태와 재시도 receipt를 처리하고 Handle은 session을 찾아 응답한다.
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		void Handle_ValtanAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStart(
				SESSION_ID sessionId,
				const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT
			Evaluate_ValtanPatternFlowStopAfterCurrent(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request,
				std::uint32_t& outRoomFlowEpoch,
				LostArk::Shared::GameplayDataRevision& outPinnedRevision,
				std::string& outReason);
		void Handle_ValtanPatternFlowStart(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START& request);
		void Handle_ValtanPatternFlowStopAfterCurrent(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT& request);
		void Handle_KoukuRaidRequest(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request);
		bool Is_KoukuRaidInputBlocked() const;
		struct KOUKU_RAID_RUN final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST Request;
			LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE State;
			std::shared_ptr<const CGameplayCatalog> pCatalog;
			std::optional<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST> PriorAuditionRequest;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT> PriorAuditionResult;
			std::optional<LostArk::Shared::S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE> PriorAuditionLifecycle;
			std::vector<LostArk::Shared::PLAYER_ID> PlayerIds;
			std::string strEntryTriggerSequenceId;
			std::set<std::string> CompletedArrivals;
			LostArk::Shared::NET_ENTITY_ID iPrimaryBossId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iAuditionRequestSequence = 0u, iAuditionEpoch = 0u, iNextEntryTick = 0u;
			bool bClearCinematic = false, bEntryRunning = false, bGate3CombatEntered = false;
            bool bClearedBossPreparation = false;
            bool bGateVoteEntry = false;
            bool bBingoSpecialRunning = false;
            std::uint32_t iGate3ClearTick = 0u;
		};
		KOUKU_RAID_RUN m_KoukuRaid;
		std::uint32_t m_iNextKoukuRaidEpoch = 1u;
		std::map<SESSION_ID, std::pair<LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST, LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE>> m_KoukuRaidReceipts;
		bool Is_KoukuRaidRunning() const;
		bool Start_KoukuBingoSpecialPattern(std::uint32_t tick);
		bool Build_KoukuRaidState(LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE& state) const;
		void Broadcast_KoukuRaidState();
		void Update_KoukuRaid(std::uint32_t tick);
		void Notify_KoukuRaidBossDeath(const SERVER_WORLD_ENTITY& boss, std::uint32_t tick);
		void Stop_KoukuRaid(std::string reason, bool completed = false);
		bool Begin_KoukuRaidPreparation(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason, bool clearedGateBoss = false, bool gateVoteEntry = false);
		bool Apply_KoukuRaidReadiness(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST& request, std::string& reason);
		bool Begin_KoukuRaidCinematic(const std::string& gateId, bool clear, std::uint32_t tick);
		bool Advance_KoukuRaidGate(std::uint8_t nextGate, bool restart);
		bool Start_KoukuRaidCombat(std::uint32_t tick, const std::string& preflightGateId = {});
        bool Build_KoukuRaidEntryRequest(const KOUKU_RAID_GATE_DEFINITION& gate, std::uint32_t entryIndex,
            LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		bool Enter_KoukuRaidCombat(std::uint8_t gate, std::uint32_t tick);
		bool Start_KoukuRaidEntry(std::uint32_t tick);
		void Handle_KoukuSaydonDraftChunk(SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK& chunk);
		void Handle_KoukuSaydonPatternAudition(
			SESSION_ID sessionId,
			const LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request);
		LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_RESULT
			Evaluate_KoukuSaydonPatternAudition(
				SESSION_ID sessionId,
				const LostArk::Shared::
					C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST& request,
				LostArk::Shared::
					S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& outResult, bool continueRaid = false,
                const std::vector<SERVER_WORLD_ENTITY>* stagedBosses = nullptr);
		SERVER_WORLD_ENTITY* Find_KoukuSaydonAuditionBoss();
		// 배치 ID·archetype이 일치하는 Kouku 보스 포인터를 빌려준다. 목록에 없으면 nullptr이며 생존 여부는 호출자가 검사한다.
		SERVER_WORLD_ENTITY* Find_KoukuSaydonArenaBoss(
			const std::string& placementId,
			const std::string& archetypeId);
		bool Update_KoukuSaydonBoss(
			SERVER_WORLD_ENTITY& boss, std::uint32_t serverTick);
		// Logic 출력의 기믹을 보스 ID·패턴 sequence와 함께 보관해, 해당 실행에 속한 요청만 후속 처리한다.
		struct KOUKU_PENDING_MECHANIC_TRIGGER final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iPatternSequence = 0u;
			BOSS_PATTERN_MECHANIC_TRIGGER Trigger;
		};
		std::vector<KOUKU_PENDING_MECHANIC_TRIGGER> m_PendingKoukuMechanicTriggers;
		[[nodiscard]] bool Commit_KoukuAlbionAirborne(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Commit_KoukuMechanicTriggers(std::uint32_t serverTick);
		void Update_KoukuActorContacts(SERVER_WORLD_ENTITY& actor, const BOSS_PATTERN_DEFINITION& pattern,
			const CGameplayCatalog& product, std::uint32_t serverTick);
		void Update_KoukuPursuitProjectiles(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window, const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Update_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const CGameplayCatalog& catalog, std::uint32_t serverTick);
		void Clear_KoukuPlayerTargets(SERVER_WORLD_ENTITY& boss, KOUKUSAYDON_LOGIC_LEDGER& ledger);
		void Update_KoukuRandomVolley(SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE& window,
			const CGameplayCatalog& catalog, std::uint32_t serverTick, bool hasAlivePlayers);
		void Update_KoukuGazeClones(std::uint32_t serverTick);
		[[nodiscard]] bool Update_KoukuSummonTriggers(SERVER_WORLD_ENTITY& clone,
			const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		// Logic의 포획·기믹·연출·후속 패턴 요청을 방 상태에 반영한다. true면 Brain의 패턴 완료까지 반영한 것이다.
		bool Apply_KoukuLogicOutput(
			const KOUKUSAYDON_LOGIC_OUTPUT& output,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		enum class KOUKUSAYDON_PATTERN_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct KOUKUSAYDON_PATTERN_AUDITION_MEMBER final
		{
			std::string strMemberId;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
			std::vector<std::string> PatternIds;
			std::vector<std::uint32_t> TransitionTicks;
			std::size_t iPatternIndex = 0u;
			std::uint32_t iNextStartTick = 0u;
			std::uint32_t iScheduledStartTick = 0u;
			std::uint32_t iPatternSequence = 0u;
			bool bCompleted = false;
			bool bOwnsPlayerMode = false;
			KOUKUSAYDON_LOGIC_LEDGER LogicLedger;
			// 진입 root가 후속 패턴들이 끝날 때까지 포털 기준 객체를 보관한다.
			std::optional<SERVER_WORLD_ENTITY> MarioEntryAnchor;
			std::string strMarioEntryPatternId;
			std::uint32_t iMarioEntryStartTick = 0u;
			std::uint8_t iMarioEntryStage = 0u;
			bool bMarioEntryConsumed = false;
			// 나중에 방에 남은 사람이 아니라 실제 입장에 성공한 플레이어·session·entity를 고정한다.
			bool bMarioSoloReturnRequired = false;
			LostArk::Shared::PLAYER_ID iMarioEntrantPlayerId = 0u;
			SESSION_ID iMarioEntrantSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iMarioEntrantNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			bool bMarioReturnCompleted = false;
			std::size_t iCompletionChainFirstIndex = 0u;
			std::uint32_t iCompletionChainCount = 0u;
			std::uint32_t iCompletionChainCompleted = 0u;
			std::string strCompletionChainSuccessPatternId;
			bool bCompletionChainStarted = false;
            bool bParentSequenceStarted = false;
            bool bParentSequenceLoops = false;
            std::size_t iParentLoopIndex = 0u;
            std::size_t iParentLastIndex = 0u;
			bool bCompletionChainAwaitingReturn = false;
			bool bCompletionChainSuccessQueued = false;
			std::uint32_t iNextWorldCue = 1u;
			std::unordered_map<std::string, std::string> WorldCueByInstance;
			std::unordered_map<std::string, std::string> WorldCueByOccurrence;
		};
		// 본 패턴의 actor 진행이 끝나도 남은 occurrence의 시간과 소유 객체를 유지한다.
		struct KOUKU_PATTERN_TAIL final
		{
			std::shared_ptr<SERVER_WORLD_ENTITY> pOwner;
			KOUKUSAYDON_PATTERN_AUDITION_MEMBER Member;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iLastUpdateTick = 0u;
		};
		struct KOUKU_SCHEDULED_SUPPORT_SURFACE final
		{
			std::string strMemberId;
			std::uint32_t iStartTick = 0u;
			std::uint32_t iEndTick = 0u;
			SERVER_NAVIGATION_SUPPORT_SURFACE Surface;
		};
		struct KOUKUSAYDON_PATTERN_AUDITION_STATE final
		{
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iCommonStartTick = 0u;
			LostArk::Shared::GameplayDataRevision PinnedGameplayRevision{};
			std::uint32_t iPinnedSourceRevision = 0u;
			// 전체 gameplay는 PinnedGameplayRevision을 따른다. 이 shared_ptr는 이번 실행의 Kouku 패턴·Logic 정의를 유지한다.
			std::shared_ptr<const CGameplayCatalog> pProductGeneration;
			std::vector<KOUKUSAYDON_PATTERN_AUDITION_MEMBER> Members;
			std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> WorldPlays;
			std::vector<KOUKU_SCHEDULED_SUPPORT_SURFACE> SupportSchedule;
			std::vector<KOUKU_PATTERN_TAIL> Tails;
		};
		KOUKUSAYDON_PATTERN_AUDITION_MEMBER* Find_KoukuAuditionMember(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence = 0u);
		KOUKUSAYDON_LOGIC_LEDGER* Active_KoukuPlayerLedger();
		[[nodiscard]] const CGameplayCatalog* Resolve_KoukuProductCatalog() const noexcept;
		void Prepare_KoukuAuditionTick(std::uint32_t serverTick);
		void Update_KoukuPatternTails(std::uint32_t serverTick);
		SERVER_WORLD_ENTITY* Find_KoukuOccurrenceOwner(LostArk::Shared::NET_ENTITY_ID bossId, std::uint32_t patternSequence);
		bool Retain_KoukuPatternTail(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			const SERVER_WORLD_ENTITY& sourceOwner, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
        bool Start_KoukuParentSequence(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
            SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		bool Start_KoukuCompletionChain(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern, std::uint32_t serverTick);
		void Update_KoukuMarioEntry(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member, std::uint32_t serverTick);
		void Commit_KoukuMarioEntries();
		bool Commit_KoukuMarioPhasePlayers(SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t serverTick);
		void Complete_KoukuMarioReturn(SERVER_PLAYER& player,
			const std::string& sourcePlacementId, std::uint32_t updateTick);
		void Queue_KoukuCompletionChainSuccess(KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member,
			std::uint32_t serverTick);
		struct KOUKU_PENDING_MARIO_ENTRY final { std::string strMemberId; LostArk::Shared::PLAYER_ID iPlayerId; std::uint32_t iRootStartTick; std::uint8_t iStage = 0u; };
		std::vector<KOUKU_PENDING_MARIO_ENTRY> m_PendingKoukuMarioEntries;
		bool Enter_MarioFromPattern(SERVER_PLAYER& player, std::uint8_t stage);
		bool Refresh_KoukuSupportSurfaces(std::uint32_t serverTick);
		bool Build_KoukuBundleState(LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE& message) const;
		void Broadcast_KoukuBundleState(LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state);
		void Broadcast_OwnedWorldSequence(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& message);
		void Stop_KoukuWorldOwner(const std::string& memberId = {}, bool finished = false);
		// 자연스러운 패턴 완료 뒤에도 유지하며, reset·cancel 또는 HP 0에서 제거한다.
		struct KOUKU_DAMAGEABLE_WORLD_CUE final
		{
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY Play;
			LostArk::Shared::NET_ENTITY_ID iBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			bool bCancelled = false;
			std::uint32_t iNextMadnessTick = 0u;
			BOSS_ENCOUNTER_MADNESS_POLICY MadnessPolicy;
			std::uint8_t iMadnessSource = 0u; // 1 circus ball, 2 odd doll
		};
		std::vector<KOUKU_DAMAGEABLE_WORLD_CUE> m_KoukuDamageableWorldCues;
		std::vector<SERVER_WORLD_ENTITY> m_PendingKoukuWorldBodies;
		bool Stage_KoukuWorldBody(const BOSS_PATTERN_WORLD_COMBAT_BODY& body,
			LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play, bool authoredMadness = false);
		void Cancel_KoukuWorldBodies(const std::string& memberId = {}, SESSION_ID ownerSession = INVALID_SESSION_ID);
		void Update_KoukuWorldBodies(std::uint32_t serverTick);

		struct KOUKUSAYDON_PATTERN_AUDITION_RECEIPT final
		{
			LostArk::Shared::
				C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT Result;
			std::optional<LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};

		void Queue_KoukuSaydonPatternAuditionLifecycle(
			const std::string& patternId,
			std::uint32_t patternSequence,
			std::uint32_t stageIndex,
			LostArk::Shared::
				KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {}, LostArk::Shared::NET_ENTITY_ID bossId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		bool Flush_KoukuSaydonPatternAuditionLifecycle();
		void Clear_KoukuSaydonPatternAudition(bool completed = false, std::string reason = {});
		SERVER_WORLD_ENTITY* Find_AuditionBoss();
		SERVER_WORLD_ENTITY* Find_AuditionBoss(
			const std::string& placementId);
		bool Has_EngagedAuditionPlayer(const SERVER_WORLD_ENTITY& boss) const;
		bool Build_ValtanBossOnlyAuditionReset(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			SERVER_WORLD_ENTITY& outBoss,
			std::string& status);
		bool Reset_ValtanBossOnlyAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		bool Reset_ValtanAuditionState(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		enum class VALTAN_PATTERN_ID_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE,
			COMPLETED_HOLD,
			IDLE_HOLD
		};

		struct VALTAN_PATTERN_ID_AUDITION_STATE final
		{
			VALTAN_PATTERN_ID_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_ID_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0u;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bResetlessContinuation = false;
			bool bReportedWaitingForPlayer = false;
			// 이미 실행 중인 선행 패턴을 인계받은 경우에는 대응하는 Client Play 요청이 없다.
			bool bAdoptedLivePredecessor = false;
			// 현재 Flow occurrence만 기존 Brain의 순서 실행에 이어 붙인다.
			std::optional<BOSS_PATTERN_SEQUENCE_DEFINITION> AdoptedFlowSequence;
		};

		struct VALTAN_NEXT_PATTERN_RESERVATION final
		{
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iPredecessorPatternSequence = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::string strBossPlacementId;
			std::string strPatternId;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			bool bReportedWaitingForPlayer = false;
		};

		struct VALTAN_NEXT_PATTERN_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
		};

		[[nodiscard]] bool Is_ValtanPatternIdAuditionRunning() const noexcept;
		LostArk::Shared::VALTAN_AUDITION_RESULT Evaluate_ValtanNextPatternControl(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			std::uint32_t& outCurrentHealthBar);
		LostArk::Shared::VALTAN_AUDITION_RESULT Adopt_ValtanLiveNextPattern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			SERVER_WORLD_ENTITY& boss);
		void Cancel_ValtanNextPatternReservation(std::string reason);
		void Cancel_ValtanPatternIdAudition(std::string reason);
		void Try_PromoteValtanNextPattern(SERVER_WORLD_ENTITY& boss);
		bool Prepare_ValtanPatternIdAuditionBeforeBrain(SERVER_WORLD_ENTITY& boss);
		bool Refresh_ValtanPatternIdAuditionState();
		void Queue_ValtanAuditionLifecycle(
			SESSION_ID ownerSessionId,
			std::uint32_t requestSequence,
			std::uint32_t roomEpoch,
			std::uint32_t patternSequence,
			const std::string& patternId,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanNextPatternLifecycle(
			const VALTAN_NEXT_PATTERN_RESERVATION& reservation,
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		void Queue_ValtanPatternIdAuditionLifecycle(
			LostArk::Shared::VALTAN_AUDITION_LIFECYCLE_STATE state,
			std::string reason = {});
		bool Flush_ValtanPatternIdAuditionLifecycle();

		enum class VALTAN_PATTERN_FLOW_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			PENDING,
			ACTIVE
		};

		struct VALTAN_PATTERN_FLOW_AUDITION_STATE final
		{
			VALTAN_PATTERN_FLOW_AUDITION_PHASE ePhase =
				VALTAN_PATTERN_FLOW_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iRequestSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::uint32_t iFirstPatternSequence = 0u;
			std::size_t iStartSlotIndex = 0u;
			std::size_t iReportedSequenceIndex =
				(static_cast<std::size_t>(-1));
			std::uint32_t iReportedPatternSequence = 0u;
			bool bReportedPausedForRevive = false;
			bool bStopAfterCurrent = false;
			std::string strBossPlacementId;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strStartSlotId;
			std::vector<LostArk::Shared::VALTAN_PATTERN_FLOW_SLOT_WIRE> Slots;
			BOSS_PATTERN_SEQUENCE_DEFINITION Sequence;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		};

		[[nodiscard]] bool Is_ValtanPatternFlowRunning() const noexcept;
		[[nodiscard]] const BOSS_PATTERN_SEQUENCE_DEFINITION*
			Resolve_ValtanPatternFlowSequence(
				const SERVER_WORLD_ENTITY& boss) const noexcept;
		void Refresh_ValtanPatternFlowState(SERVER_WORLD_ENTITY& boss);
		void Finish_ValtanPatternFlow(
			SERVER_WORLD_ENTITY& boss,
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE terminalState,
			std::string reason = {});
		void Abort_ValtanPatternFlowForOwner(
			SESSION_ID sessionId,
			std::string reason);
		void Queue_ValtanPatternFlowLifecycle(
			LostArk::Shared::VALTAN_PATTERN_FLOW_LIFECYCLE_STATE state,
			const SERVER_WORLD_ENTITY* boss,
			std::string reason = {});
		bool Flush_ValtanPatternFlowLifecycle();

		enum class VALTAN_TIMELINE_AUDITION_PHASE : std::uint8_t
		{
			INACTIVE,
			WAITING_ENVIRONMENT,
			READY,
			WAITING_PATTERN_START,
			WAITING_PATTERN_FINISH,
			COMPLETED_HOLD,
			FAILED_HOLD
		};

		struct VALTAN_TIMELINE_AUDITION_STATE final
		{
			VALTAN_TIMELINE_AUDITION_PHASE ePhase =
				VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = 0;
			LostArk::Shared::PLAYER_ID iOwnerPlayerId =
				LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::size_t iRowIndex = 0u;
			std::size_t iActionIndex = 0u;
			std::uint32_t iRepeatIndex = 0u;
			std::uint32_t iExpectedPatternSequence = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::uint32_t iHeldBossHp = 0u;
			std::uint32_t iHeldBossHealthBar = 0u;
			bool bAllowProductPropBreak = false;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			std::string strExpectedPatternId;
			std::vector<std::string> ExpectedGoneGroupIds;
		};

		// 전투 페이지 시작은 그 시점의 아레나 파괴 상태를 준비하고 실제 Brain을 재개해 이후 전투를 계속 진행한다.
		struct VALTAN_FIGHT_PAGE_START_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iCommandId = 0u;
			std::uint32_t iEnvironmentDeadlineTick = 0u;
			std::vector<std::string> ExpectedGoneGroupIds;

			bool Is_Active() const noexcept
			{
				return LostArk::Shared::INVALID_NET_ENTITY_ID != iBossEntityId;
			}
		};

		bool Prepare_ValtanTimelineArenaState(
			const CWorldDestructionRuntime& runtime,
			const SERVER_WORLD_ENTITY& boss,
			VALTAN_TIMELINE_ARENA_STATE arenaState,
			std::uint32_t requestTick,
			WORLD_DESTRUCTION_TRANSACTION& outTransaction,
			std::vector<std::string>& outExpectedGoneGroupIds,
			std::string& status) const;
		bool Stage_ValtanTimelineRowStart(
			SESSION_ID sessionId,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			SERVER_PLAYER& outOwner,
			std::string& status) const;
		bool Start_ValtanTimelineRow(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Stop_ValtanTimelineRow(bool resetEncounter = false);
		bool Prepare_ValtanTimelineRowBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		void Restore_ValtanTimelineRowAfterBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		bool Start_ValtanFightPage(
			SESSION_ID sessionId,
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t commandId,
			std::uint32_t startTick,
			std::string& status);
		bool Prepare_ValtanFightPageBeforeBrain(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t updateTick);
		struct VALTAN_DECISION_TRACE_REVISION_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID iBossEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::string strBossPlacementId;
			std::uint64_t iTraceSequence = 0u;
			LostArk::Shared::GameplayDataRevision DefinitionRevision{};
		};
		// Debug 지급과 클리어 보상이 공유한다. catalog의 maxStack·인벤토리 슬롯 한도 안에서 가방 수량을 합친다.
		// 알 수 없는 ID나 새 슬롯 부족은 false로 거절하고 인벤토리를 유지한다.
		bool Grant_Item(
			SERVER_PLAYER& player,
			const std::string& itemId,
			std::uint32_t quantity);
		// 요청자의 아이템을 Grant_Item으로 지급하고 성공하면 전체 인벤토리를 응답한다.
		void Handle_DebugGiveItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DEBUG_GIVE_ITEM& request);
		// 소유 수량·생존과 아이템별 사용 조건을 검증해 회복 또는 전투 효과를 적용한 뒤 수량을 차감한다.
		// 인벤토리를 응답하고 HP 등 월드 상태는 snapshot으로 전달한다.
		void Handle_UseItem(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_USE_ITEM& request);
		// 장착 슬롯·직업·가방 여유를 검사한다. 변경 성공 여부와 관계없이 전체 인벤토리를 응답한다.
		void Handle_SetEquipment(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request);
		bool Apply_SetEquipment(SERVER_PLAYER& player,
			const LostArk::Shared::C2S_SET_EQUIPMENT& request) const;
		// 장착 장비 또는 전체 가방의 수리비를 합산한다. 실링이 충분할 때만 차감·완전 수리하고 인벤토리를 응답한다.
		void Handle_RepairEquipment(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_REPAIR_EQUIPMENT& request);
		// 직업 변경 뒤 새 직업이 쓸 수 없는 장비를 가방으로 돌린다.
		bool Unequip_OtherClassItems(SERVER_PLAYER& player) const;
		// 상점 NPC 거리·판매 목록·총액·가방 공간을 모두 검증한 뒤 구매한다. 거절 시 상태를 유지하고 인벤토리를 응답한다.
		void Handle_BuyItems(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_BUY_ITEMS& request);
		bool Apply_BuyItems(SERVER_PLAYER& player,
			const LostArk::Shared::C2S_BUY_ITEMS& request) const;
		// Bern 신규 입장의 인벤토리 변경 전 한 번만 저장 캐릭터를 복원한다. 전체 요청을 검증해 거절 시 기존 상태를 유지한다.
		void Handle_RestoreCharacter(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RESTORE_CHARACTER& request);
		bool Validate_RestoreCharacter(const SERVER_PLAYER& player,
			const LostArk::Shared::C2S_RESTORE_CHARACTER& request) const;
		void Handle_UpgradeEquipment(SESSION_ID sessionId,
			const LostArk::Shared::C2S_UPGRADE_EQUIPMENT& request);
		void Handle_CaptureCharacter(SESSION_ID sessionId,
			const LostArk::Shared::C2S_CAPTURE_CHARACTER& request);
		// Debug Character Select Arena "되돌리기" -- despawns every world entity the
		// Character Select는 전체 시험 객체·SpawnGroup을 초기화하고 Kouku는 카드미로·시험 객체를 정리한다.
		// Valtan은 주 보스 소유 트리만 제거해 파동·NPC·파괴 상태·요청 receipt를 보존한다.
		// 실행 중 Kouku raid와 허용 외 월드·session 요청은 응답 없이 무시한다.
		void Handle_DespawnAllWorldEntities(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_DESPAWN_ALL_WORLD_ENTITIES& request);
		// 기본은 비활성 배치에서 만든 시험 객체·종속 객체와 Esther 소환을 정리한다.
		// allArenaBosses는 모든 보스로 범위를 넓히며, preflightOnly는 복사본으로 전송 자료를 검증한다.
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses = false);
		bool Despawn_KoukuSaydonArenaDebugEntities(bool allArenaBosses, bool preflightOnly);
		// Bern 안내 NPC 확인 요청의 거리·생존·행동을 재검사하고 기존 Server 월드 전환을 예약한다.
		void Handle_ConfirmNpcEntry(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CONFIRM_NPC_ENTRY& request);
		// Bern에서 거리·상태를 재검사해 최대 4명을 10초간 모은다. 승인 순서로 팀을 나눠 private 경기 입장을 함께 준비한다.
		// 전환 성공 때만 대기열을 소비하며 LEAVE는 요청자만 제외한다.
		void Handle_ColosseumQueueJoin(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_COLOSSEUM_QUEUE_JOIN& request);
		void Handle_ColosseumQueueLeave(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_COLOSSEUM_QUEUE_LEAVE& request);
		void Send_ColosseumQueueState(
			SESSION_ID sessionId, LostArk::Shared::COLOSSEUM_QUEUE_STATE state);
		void Try_FormColosseumMatch();
		void Broadcast_ColosseumQueueState();
		void Try_StartColosseumEntry();
		void Handle_ColosseumLoadReady(SESSION_ID, const LostArk::Shared::C2S_COLOSSEUM_LOAD_READY&);
		void Handle_ColosseumReturn(SESSION_ID, const LostArk::Shared::C2S_COLOSSEUM_RETURN&);
		void Update_ColosseumMatch(std::uint32_t tick);
		void Broadcast_ColosseumMatchState();
		void Score_ColosseumKills(std::uint32_t tick);
		// 상호작용 키 요청은 trigger ID만 받는다. 플레이어가 아직 상자 안에 있는지 재검사한 뒤 실행한다.
		void Handle_InteractTrigger(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_INTERACT_TRIGGER& request);
		// Raid Clear screen's "돌아가기" button -- the reverse trip. No proximity
		// 클리어한 Valtan/Kouku에서 거리·파티장 조건 없이 각자 기록된 진입 안내 NPC로 돌아간다. Lobby 입장은 기본 안내점을 쓴다.
		void Handle_ReturnToBern(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RETURN_TO_BERN& request);
		// 클리어 복귀 버튼과 관문 EXIT가 공유하는 Bern 전환 예약이다. 예약하지 못하면 false다.
		bool Stage_ReturnToBern(
			LostArk::Shared::PLAYER_ID playerId,
			std::uint32_t requestSequence);
		// 같은 방의 현재 NetEntityId를 플레이어로 찾아 초대한다. 다른 방·사라진 ID는 거절하며 닉네임은 표시용이다.
		void Handle_PartyInvite(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE& request);
		void Handle_PartyInviteRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_PARTY_INVITE_RESPOND& request);
		void Broadcast_PartyRoster(std::uint32_t partyId);

		/* 파티 레이드 입장 전원 수락 투표. 한 플레이어는 동시에 하나의 열린 proposal에만
		   속한다(propose가 그 불변식을 검사). iProposalId는 이 방에서 발급하는 단조 증가
		   식별자로 pointer/index가 아니다. Voters는 발의 시점 멤버 스냅샷(솔로는 1명),
		   Accepted는 그 부분집합. m_iServerTick이 iDeadlineTick을 넘으면 TIMEOUT으로 닫는다. */
		// 관문의 마지막 주 보스 사망을 기록하고 이동 투표를 모은다. 전원 동의하면 다음 관문 배치와 플레이어 위치로 전환한다.
		struct GATE_PROGRESS_STATE
		{
			std::uint8_t iCurrentGate = 0u;     // 1부터 시작하며 0은 아직 열린 관문 없음
			std::uint8_t iClearedMask = 0u;
			std::uint32_t iProposalId = 0u;     // 0 = no vote open
			std::uint32_t iRaidEpoch = 0u;      // 0이 아닌 raid run ID로 투표 대상을 고정
			LostArk::Shared::GATE_PROGRESS_KIND eKind = LostArk::Shared::GATE_PROGRESS_KIND::ADVANCE;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::PLAYER_ID iProposerId = LostArk::Shared::INVALID_PLAYER_ID;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};
		void Handle_GateProgressPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_PROPOSE& request);
		void Handle_GateProgressRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_GATE_PROGRESS_RESPOND& request);
		void Close_GateProgressVote(LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result);
		void Expire_GateProgressVote();
		bool Complete_GateProgressTransition(const GATE_PROGRESS_STATE& transition);
		void Update_ArenaAssembly();
		bool Complete_ArenaAssembly();
		bool Collect_KoukuEntryParticipants(std::vector<LostArk::Shared::PLAYER_ID>& participants, std::uint32_t& raidEpoch) const;
		bool Collect_ValtanEntryParticipants(std::vector<LostArk::Shared::PLAYER_ID>& participants) const;
		bool Start_ValtanEntry(const std::vector<LostArk::Shared::PLAYER_ID>& expectedParticipants);
		// 사망한 주 보스가 관문의 마지막 보스인지 확인해 클리어를 기록한다.
		void Notify_GateBossDeath(const SERVER_WORLD_ENTITY& deadBoss);
		// Debug 또는 관문 진행으로 배치가 생성되면 현재 관문을 갱신한다.
		void Note_GatePlacementRaised(const std::string& placementId);
		bool Advance_Gate(std::uint8_t nextGate);
		bool Advance_Gate(std::uint8_t nextGate, const std::vector<LostArk::Shared::PLAYER_ID>* participants);
		bool Spawn_GatePlacement(const std::string& placementId);
		bool Spawn_GatePlacement(const std::string& placementId, SERVER_WORLD_ENTITY* prepared);
		bool Build_GateProgressState(LostArk::Shared::S2C_GATE_PROGRESS_STATE& message,
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result) const;
		void Broadcast_GateProgressState(
			bool bClosed, LostArk::Shared::GATE_PROGRESS_VOTE_RESULT result,
			LostArk::Shared::GATE_PROGRESS_KIND completedKind = LostArk::Shared::GATE_PROGRESS_KIND::END);
		std::uint8_t Gate_Count() const;
		std::uint8_t Resolve_CurrentKoukuGate() const;
		bool Resolve_KoukuRevivePosition(const SERVER_PLAYER& player, SERVER_NAV_POINT& position, float& yaw) const;
		int Gate_IndexOfPlacement(const std::string& placementId) const;
		// tick별 전투 기여를 보스에 누적하고 사망 시 방 기록에 합친다. 클리어는 방 기록을 전송한 뒤 비운다.
		void Tick_MvpLedgers();
		void Merge_MvpLedger(SERVER_WORLD_ENTITY& boss);
		void Broadcast_RaidMvpResult(std::uint8_t iGate);

		struct RAID_ENTRY_PROPOSAL
		{
			std::uint32_t iProposalId = 0u;
			std::uint32_t iPartyId = 0u;
			std::uint32_t iRequestSequence = 0u;
			LostArk::Shared::RAID_ENTRY_TARGET eTarget =
				LostArk::Shared::RAID_ENTRY_TARGET::VALTAN;
			std::string strNpcPlacementId;
			std::vector<LostArk::Shared::PLAYER_ID> Voters;
			std::vector<LostArk::Shared::PLAYER_ID> Accepted;
			std::uint32_t iDeadlineTick = 0u;
		};

		// 리더/솔로가 입장하기로 발의 -> 대상 전원(솔로는 본인)에게 프롬프트, 투표 개시.
		void Handle_RaidEntryPropose(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_PROPOSE& request);
		// 개별 수락/거절 반영. 거절이면 즉시 DECLINED 종료, 전원 수락이면 ALL_ACCEPTED 종료.
		void Handle_RaidEntryRespond(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_RAID_ENTRY_RESPOND& request);
		// 투표를 result로 종료해 전원에 통지하고 proposal을 제거한다. ALL_ACCEPTED면
		// Stage_PartyWorldTransfer로 batch 전송을 stage하고, stage 실패면 CANCELLED로 낮춘다.
		void Close_RaidEntryVote(
			RAID_ENTRY_PROPOSAL& proposal,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// 진행/종료 S2C_RAID_ENTRY_VOTE를 proposal의 present voters에게 보낸다.
		void Broadcast_RaidEntryVote(
			const RAID_ENTRY_PROPOSAL& proposal, bool bClosed,
			LostArk::Shared::RAID_ENTRY_VOTE_RESULT result);
		// m_iServerTick 기준 만료 proposal을 TIMEOUT으로 닫는다(tick 루프에서 호출).
		void Expire_RaidEntryProposals();
		// playerId가 voter인 열린 proposal을 CANCELLED로 닫는다(이탈/파티 해산 시).
		void Cancel_RaidEntryProposalsInvolving(LostArk::Shared::PLAYER_ID playerId);
		// 검증된 batch 멤버(front=리더)를 기존 SERVER_WORLD_TRANSFER_REQUEST 경로로 stage.
		// 멤버 unavailable/이미 staged면 false(호출자가 투표를 CANCELLED로 닫는다).
		bool Stage_PartyWorldTransfer(
			const std::vector<LostArk::Shared::PLAYER_ID>& batchMemberIds,
			LostArk::Shared::WORLD_ID targetWorldId,
			std::uint32_t requestSequence, const std::string& raidReturnNpcPlacementId,
			const std::string& spawnPlacementOverrideId = {});
		// player가 알려진 Valtan 입장 guide NPC 근처(proximity)인지 검증한다.
		bool Is_PlayerNearValtanEntryNpc(
			const SERVER_PLAYER& player, const std::string& npcPlacementId) const;
		// 일반 연출은 현재 방에 재생 명령을 보낸다. 워터팡 시작은 보관해 늦은 입장에도 전달한다.
		// Kouku 입장 연출은 raid 준비로 연결하며, 거절된 요청은 false를 반환한다.
		bool Broadcast_WorldSequencePlay(
			const std::string& instanceId, float playbackSpeed = 1.f,
			float positionOffsetX = 0.f, float positionOffsetY = 0.f, float positionOffsetZ = 0.f,
			std::uint32_t durationMs = 0u, const std::string& targetSequenceInstanceId = {},
			LostArk::Shared::WORLD_SEQUENCE_OPERATION operation = LostArk::Shared::WORLD_SEQUENCE_OPERATION::PLAY);
		void Handle_DebugKillGateBosses(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request);
		LostArk::Shared::DEBUG_KILL_GATE_BOSSES_RESULT Apply_DebugKillGateBosses(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_KILL_GATE_BOSSES& request, std::uint8_t& killedCount);
		std::unordered_map<SESSION_ID, std::uint32_t> m_KillGateBossesRequestSequences;
		void Handle_SetCooldownMode(SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::SET_COOLDOWN_MODE_RESULT Apply_SetCooldownMode(
			SESSION_ID sessionId, const LostArk::Shared::C2S_SET_COOLDOWN_MODE& request);
		LostArk::Shared::COOLDOWN_MODE m_eCooldownMode = LostArk::Shared::COOLDOWN_MODE::DEBUG_THREE_SECONDS;
		std::unordered_map<SESSION_ID, std::uint32_t> m_CooldownModeRequestSequences;
		void Handle_DebugWorldPlayback(SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		std::unordered_map<SESSION_ID, std::uint32_t> m_WorldPlaybackRequestSequences;
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Apply_DebugRoomPlayerArrival(
			SESSION_ID sessionId, const LostArk::Shared::C2S_DEBUG_WORLD_PLAYBACK& request);
		LostArk::Shared::DEBUG_TELEPORT_RESULT Validate_DebugTeleportDestination(
			const SERVER_PLAYER& player, const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request,
			SERVER_NAV_POINT& ground, LostArk::Shared::NET_ENTITY_ID ignoredBodyId = LostArk::Shared::INVALID_NET_ENTITY_ID);
		struct ROOM_PLAYER_ARRIVAL_RUN final
		{
			std::uint32_t iEpoch = 0u;
			std::string strRootPatternId;
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, SESSION_ID>> Players;
			std::map<std::string, LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT> Occurrences;
		};
		std::unordered_map<SESSION_ID, ROOM_PLAYER_ARRIVAL_RUN> m_RoomPlayerArrivalRuns;
		// 상호작용 상자의 제안·철회는 해당 플레이어 한 명에게만 보낸다.
		void Send_InteractPrompt(const SERVER_INTERACT_PROMPT_EDGE& edge);
		// 잠든 SpawnGroup을 시작하고, 몬스터가 사라진 완료 그룹은 재시작한다. 실행 중인 그룹에는 파동을 겹치지 않는다.
		bool Activate_SpawnGroupFromTrigger(const std::string& spawnGroupId);
		// 자동 진입과 G 상호작용이 공유하는 trigger 대상 실행 경로다.
		bool Activate_TriggerTarget(
			WORLD_TRIGGER_ACTION_KIND kind, const std::string& targetId);
		// Leave에서 이탈한 플레이어를 파티 명단에서도 제거한다.
		void Remove_FromParty(LostArk::Shared::PLAYER_ID playerId);
		// 요청자·sequence·지면 위치를 검증한 핑을 현재 방에 전송한다.
		void Handle_RoomPing(SESSION_ID sessionId, const LostArk::Shared::C2S_ROOM_PING& request);
		// 발신자도 같은 S2C_CHAT을 받아 말풍선을 그리도록 현재 방의 모든 session에 중계한다.
		void Handle_Chat(
			SESSION_ID sessionId,
			const LostArk::Shared::C2S_CHAT& request);

		bool Send_Accepted(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		bool Send_EnterRejected(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::ENTER_WORLD_REJECTION_REASON reason);
		bool Send_Spawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_PLAYER& player);
		static bool Build_WorldEntitySpawnedPayload(
			const SERVER_WORLD_ENTITY& entity,
			std::vector<std::uint8_t>& outPayload);
		bool Send_WorldEntitySpawned(
			const std::shared_ptr<CClientSession>& session,
			const SERVER_WORLD_ENTITY& entity);
		bool Send_WorldEntityDespawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Send_CombatObjectSpawned(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawned);
		bool Send_WorldEntitySpawnResult(
			const std::shared_ptr<CClientSession>& session,
			const std::string& placementId,
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT result,
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		bool Send_ValtanAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST& request,
			LostArk::Shared::VALTAN_AUDITION_RESULT result,
			std::uint32_t currentHealthBar);
		bool Send_KoukuSaydonPatternAuditionResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& message);
		bool Send_ValtanPatternFlowResult(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t commandSequence,
			LostArk::Shared::VALTAN_PATTERN_FLOW_COMMAND command,
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT result,
			const std::string& flowId,
			const std::string& flowRevision,
			std::uint32_t roomFlowEpoch,
			const LostArk::Shared::GameplayDataRevision& pinnedRevision,
			const std::string& reason);
		bool Build_RequiredPinnedGameplayRevisions(
			std::vector<LostArk::Shared::GameplayDataRevision>&
				outRevisions) const;
		[[nodiscard]] const CGameplayCatalog* Resolve_ValtanGameplayCatalog(
			const SERVER_WORLD_ENTITY& boss) const noexcept;
		bool Send_CharacterClassChangeResult(
			const std::shared_ptr<CClientSession>& session,
			const LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS& request,
			LostArk::Shared::CHARACTER_CLASS_CHANGE_RESULT result,
			LostArk::Shared::CHARACTER_CLASS_ID activeClass);
		// 인벤토리는 플레이어 개인 상태이므로 지정 session에만 응답한다.
		bool Send_InventorySnapshot(
			const std::shared_ptr<CClientSession>& session,
			std::uint32_t requestSequence,
			const SERVER_PLAYER& player);
		bool Send_Despawned(
			const std::shared_ptr<CClientSession>& session,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		bool Send_WorldDestructionFullSync(
			const std::shared_ptr<CClientSession>& session);
		// 파괴 메시지에 Server 충돌·navigation 진단값을 담아, Client가 벽 표시만으로 통행 가능 여부를 추측하지 않게 한다.
		LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS
			Build_WorldDestructionDiagnostics() const;
		void Broadcast_Spawned(
			const SERVER_PLAYER& player,
			SESSION_ID exceptSessionId);
		void Broadcast_Despawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::PLAYER_DESPAWN_REASON reason);
		void Broadcast_WorldEntitySpawned(
			const SERVER_WORLD_ENTITY& entity);
		void Broadcast_WorldEntityDespawned(
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON reason =
				LostArk::Shared::WORLD_ENTITY_DESPAWN_REASON::REMOVED);
		bool Broadcast_WorldDestructionDelta(
			const std::vector<WORLD_DESTRUCTION_STATE_TRANSITION>& transitions,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick);
		void Broadcast_WorldSnapshot();

		std::shared_ptr<CClientSession> Find_Session(
			SESSION_ID sessionId) const;
		void Rollback_Join(SESSION_ID sessionId);
		[[nodiscard]] bool Is_PlayerAdmissionFull() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_AvailablePlayerSpawn() const;
		const WORLD_BOOTSTRAP_PLACEMENT* Find_Placement(
			const std::string& placementId) const;
		bool Build_WorldEntity(
			const WORLD_BOOTSTRAP_PLACEMENT& placement,
			LostArk::Shared::NET_ENTITY_ID netEntityId,
			SERVER_WORLD_ENTITY& outEntity,
			const CGameplayCatalog* definitionCatalog = nullptr,
			LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID, std::uint32_t ownerPatternSequence = 0u);
		bool Initialize_WorldEntities();
		bool Reset_ReplayableArenaWhenEmpty();
		bool Reset_ValtanArenaWhenEmpty();
		bool Apply_BossPatternStageActions(
			SERVER_WORLD_ENTITY& boss,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		bool Apply_BossPatternScheduledSpawnWave(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_BossPatternStageTransition(
			SERVER_WORLD_ENTITY& boss,
			const std::string& previousPatternId,
			const std::string& previousActionId,
			const std::string& nextPatternId,
			const std::string& nextActionId,
			const LostArk::Shared::GameplayDataRevision&
				previousDefinitionRevision,
			const LostArk::Shared::GameplayDataRevision& nextDefinitionRevision,
			std::uint32_t serverTick);
		bool Stage_BossPatternStageActions(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			SERVER_BOSS_COMBAT_STATE& stagedCombat,
			std::uint8_t& stagedGameplayPhase,
			SERVER_COMBAT_OBJECT_TRANSACTION& combatObjectTransaction,
			std::uint32_t spawnWaveOrdinal = 0u,
			bool scheduledSpawnWave = false);
		// 잡힌 대상과 피해 정의를 검증해 플레이어 복사본·피해 이벤트를 준비한다. 실제 반영은 호출자가 담당한다.
		bool Prepare_GrabbedPlayerImpact(
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& stagedPlayers,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& stagedDamageEvents);
		SERVER_PLAYER* Select_BossRandomAliveTarget(const SERVER_WORLD_ENTITY& boss,
			const std::string& actionId, const std::string& targetId, std::uint32_t serverTick);
		// stage 사전검증과 전투 transaction 반영 뒤, 플레이어·타깃 관련 동작을 실제 상태에 반영한다.
		bool Commit_BossPatternPlayerStageActions(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const std::string& patternId,
			const std::string& actionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
			std::uint32_t serverTick,
			std::uint32_t spawnWaveOrdinal = 0u);
		bool Resolve_ArenaRandomVolleyOrigins(
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			const BOSS_COMBAT_OBJECT_DEFINITION& definition,
			std::uint32_t spawnWaveOrdinal,
			std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET>& outOrigins,
			float explicitMinimumSpacingM = 0.f, const SERVER_NAV_POINT* anchorOverride = nullptr);
		bool Broadcast_CombatObjectLifecycle();
		void Drain_BossCombatEvents();
		bool Apply_WorldDestructionStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		// 일반 벽과 외곽 벽 파괴를 한 transaction으로 반영하고 바닥은 유지한다. 임계 구간 시험에 필요한 아레나를 함께 준비한다.
		bool Break_EveryWallForAudition(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t resetTick,
			std::string& status);
		// 보스 이동을 navigation이 허용한 마지막 표본까지 제한하고 벽 접촉은 충돌 경로에 맡긴다.
		// navigation 미로드 또는 시작점부터 바닥 밖이면 목표점을 그대로 돌려준다.
		static void Resolve_NavigableStep(
			const CServerNavigation& navigation,
			float fromX,
			float fromZ,
			float targetX,
			float targetZ,
			float& outX,
			float& outZ);
		// 현재 패턴 stage의 기둥 생성·지정 슬롯 파괴를 반영한다. 정의 세대·슬롯 준비나 commit 실패는 false다.
		bool Apply_EncounterPropStageEntry(
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Commit_DueEncounterProps(std::uint32_t serverTick);
		bool Initialize_WorldPickups();
		void Reset_WorldPickups(std::uint32_t serverTick);
		void Update_WorldPickups(std::uint32_t serverTick, bool allowCollection = true);
		void Apply_WorldPickupDestruction(const WORLD_DESTRUCTION_TRANSACTION& transaction,
			std::uint32_t serverTick);
		void Remove_RemainingWorldPickups(std::uint32_t serverTick);
		bool Send_EncounterPropSync(
			const std::shared_ptr<CClientSession>& session);
		void Broadcast_EncounterPropSync();
		// 일반 보스 이동은 몸체가 스친 벽을 파괴한다. 돌진 충격은 별도 경로에서 receiver와 일반 접촉을 함께 처리한다.
		bool Apply_WorldDestructionBodyContact(
			SERVER_WORLD_ENTITY& boss,
			float previousX,
			float previousY,
			float previousZ,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionPatternHitContact(
			SERVER_WORLD_ENTITY& boss,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionContacts(
			SERVER_WORLD_ENTITY& boss,
			const std::vector<std::string>& contactPlacementIds,
			std::uint32_t serverTick);
		bool Apply_WorldDestructionImpact(
			SERVER_WORLD_ENTITY& boss,
			const std::string& receiverPlacementId,
			std::uint32_t serverTick,
			bool& outTriggered);
		bool Commit_WorldDestructionTransaction(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::uint32_t serverTick,
			std::string& status);
		void Invalidate_DynamicNavigationPaths();
		bool Build_WorldDestructionLiveEvents(
			const WORLD_DESTRUCTION_TRANSACTION& transaction,
			const SERVER_WORLD_ENTITY& boss,
			std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>&
				liveEvents,
			std::string& status) const;
		bool Commit_DueWorldDestruction(std::uint32_t serverTick);
		bool Activate_Encounter(const std::string& placementId);
		bool Spawn_Monster(
			const std::string& spawnGroupId,
			const SPAWN_GROUP_ENTRY& entry,
			const SPAWN_GROUP_ANCHOR& anchor,
			const MONSTER_RUNTIME_PROFILE& profile,
			std::uint32_t ordinal);
		bool Spawn_KoukuCardRainSoldiers(LostArk::Shared::NET_ENTITY_ID ownerId, std::uint32_t tick,
			const BOSS_PATTERN_MECHANIC_TRIGGER* tuning = nullptr);
		void Update_KoukuCardRainSoldiers(std::uint32_t tick);
		// 망원경을 획득한 플레이어로 카드미로를 시작하고 표적을 만든다. 망치는 정해진 타격 tick에 한 번 판정한다.
		bool Begin_CardMaze(LostArk::Shared::PLAYER_ID claimantId);
		void Reset_CardMaze();
		void Despawn_CardMazeTargets();
		void Resolve_CardMazeHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Resolve_MarioHammerHit(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Update_MarioBombContacts(SERVER_PLAYER& player, std::uint32_t updateTick);
		void Update_MarioBouncingBallContacts(SERVER_PLAYER& player, std::uint32_t updateTick);
		// 워터팡 경기의 회전 물대포·폭포가 플레이어에 주는 영향을 갱신한다.
		void Update_MaharakaWaterpangHazards(SERVER_PLAYER& player, std::uint32_t updateTick);
        void Handle_MaharakaAITuning(SESSION_ID sessionId, const LostArk::Shared::C2S_MAHARAKA_AI_TUNING& request);
        bool Spawn_MaharakaWaterpangAI();
        void Update_MaharakaWaterpangMatch(std::uint32_t updateTick);
        void Clear_MaharakaWaterpangAI(std::uint32_t keepCount = 0u);
        bool Finish_MaharakaWaterpangMatch();
		// Debug 강제 이벤트를 먼저, 없으면 경기 일정을 조회한다. 둘 다 없으면 false다.
		bool Sample_MaharakaWaterpangNow(std::uint32_t tick, LostArk::Shared::MAHARAKA_WATERPANG_EVENT_SAMPLE& out) const;
		// Server의 물총 장착 조건을 판정하며 결과는 PLAYER_SNAPSHOT.isWaterpangArmed로 전달한다.
		bool Is_MaharakaWaterpangArmed(const SERVER_PLAYER& player) const;
		// 물총 Q/W/E/R의 쿨타임을 적용하고 발사를 예약한다. 이동 자체를 잠그지는 않는다.
		bool Try_StartMaharakaWaterGunSkill(SERVER_PLAYER& player, const LostArk::Shared::C2S_USE_SKILL& command);
		// 예약된 물줄기를 진행시키고 맞은 플레이어를 밀어낸다.
		void Update_MaharakaWaterGunShots(std::uint32_t updateTick);
		// 강제 이벤트 종료 뒤에도 잠시 유지해 마지막 밀림의 낙하를 처리한다.
		bool Is_MaharakaWaterpangDebugEventLive(std::uint32_t tick) const;
		// Debug F1 패턴 버튼으로 방 전체의 워터팡 이벤트를 시작한다.
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Start_MaharakaWaterpangDebugEvent(const std::string& instanceId);
		std::uint8_t Mario_CurseReleasedMask(std::uint8_t stage, std::uint8_t layout) const;
		bool Spawn_CardMazeTarget(const CKoukuCardMazeRuntime::SPAWN_REQUEST& request);
		void Remove_CardMazeTarget(LostArk::Shared::NET_ENTITY_ID id);
		void Update_CardMaze(std::uint32_t tick);
		// 카드미로 시작 전 광대 상자를 생성하고 파괴 여부를 유지한다. Clear는 남은 상자와 기록을 정리한다.
		void Update_CardMazeClownBox(std::uint32_t tick);
		void Clear_CardMazeClownBox();
		// 빙고 표식 만료 시 보유자 위치에 폭탄을 설치한다. 보유자가 방을 떠나면 표식도 제거한다.
		void Update_KoukuBingo(std::uint32_t tick);
		bool Begin_CardMazeTransfer(SERVER_PLAYER& player, float x, float y, float z,
			std::uint32_t tick, bool leaving);
		std::uint32_t Count_SpawnGroupEntities(
			const std::string& spawnGroupId) const;
		// 게이지를 사용하는 stance 유지 중이면 직업별 이동 배율을, 그 외에는 1을 반환한다.
		float Resolve_StanceMoveSpeedScale(const SERVER_PLAYER& player) const;
		// 이번 tick에 플레이어 이동을 막는 몬스터·보스 몸체를 충돌 시스템에 전달한다.
		void Refresh_PlayerBlockingBodies();
		bool Try_KoukuWalkOffFloor(SERVER_PLAYER& player, float x, float z,
			float fixedDeltaSeconds, std::uint32_t updateTick);
		const WORLD_BOOTSTRAP_PLACEMENT* Resolve_KoukuFallCenter(const SERVER_PLAYER& player) const;
		bool Update_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		void Begin_PlayerFall(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds,
			std::uint32_t updateTick);
		// 보스 패턴은 복제 ID로 잡기를 요청한다. 방이 행동 중단·고정 tick 위치 보정·해제를 함께 책임진다.
		bool Capture_PlayerAttachment(
			LostArk::Shared::NET_ENTITY_ID playerEntityId,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
			std::uint32_t serverTick, std::uint32_t holdEndTick = 0u, std::uint32_t sourcePatternSequence = 0u);
		bool Update_PlayerAttachment(
			SERVER_PLAYER& player,
			std::uint32_t serverTick);
		bool Release_PlayerAttachment(
			SERVER_PLAYER& player,
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		std::size_t Release_PlayerAttachments(
			LostArk::Shared::NET_ENTITY_ID ownerEntityId,
			float pushRangeM,
			std::uint32_t pushMs,
			bool knockdown,
			std::uint32_t downMs,
			std::uint32_t serverTick);
		[[nodiscard]] bool Restore_PatternBoundPlayer(SERVER_PLAYER& player);
		void Update_Players(float fixedDeltaSeconds);
		bool Prepare_ArenaEjection(
			SERVER_PLAYER& staged,
			const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_STAGE_ACTION& action,
			std::uint32_t serverTick);
		bool Resolve_ArenaCenter(
			const SERVER_WORLD_ENTITY& boss,
			SERVER_NAV_POINT& point);
		bool Activate_ValtanGhostPhaseLoop(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog);
		bool Begin_ValtanGhostRelocation(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_ValtanGhostPortalScheduler(
			SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			std::uint32_t serverTick);
		bool Update_DependentBosses(std::uint32_t serverTick);
		// 넉백 시간을 진행하며 바닥·몸체 충돌로 이동을 제한한다. 벽에 막히면 일찍 끝낸다.
		void Advance_PlayerKnockback(
			SERVER_PLAYER& player, float fixedDeltaSeconds);
		void Update_WorldEntities(float fixedDeltaSeconds);
		//고정 tick 안에서만 호출하는 개체별 처리. false는 호출자의 남은 갱신을 중단한다.
		bool Update_WorldEntity(SERVER_WORLD_ENTITY& entity, float fixedDeltaSeconds, std::uint32_t updateTick);
		bool Update_ValtanWorldEntity(SERVER_WORLD_ENTITY& entity, float fixedDeltaSeconds, std::uint32_t updateTick);
		bool Update_ValtanStageMotion(SERVER_WORLD_ENTITY& entity, const CGameplayCatalog& occurrenceCatalog,
			float fixedDeltaSeconds, std::uint32_t updateTick);
		//보상은 제거 전, 부착·공격 객체 정리는 제거 직전에 수행한다.
		void Release_WorldBossAttachments(const SERVER_WORLD_ENTITY& boss, std::uint32_t updateTick);
		void Grant_ValtanRaidClearRewards();
		bool Despawn_FinishedWorldEntities(std::uint32_t updateTick);


	private:
		// 합칠 기존 명령이 없고 일반 큐가 768개 이상이면 새 best-effort를 버려 reliable 여유를 남긴다. reliable 상한은 960개다.
		// Tick은 일반 명령 최대 256개를 소비한다. LEAVE는 이 상한을 공유하지 않는 별도 우선 큐에서 session별로 합친다.
		static constexpr std::size_t MAX_BEST_EFFORT_COMMAND_COUNT = 768u;
		static constexpr std::size_t MAX_RELIABLE_COMMAND_COUNT = 960u;
		static constexpr std::size_t MAX_COMMANDS_DRAINED_PER_TICK = 256u;

		// 수신 큐·정리 identity·공유 계측과 봉인/최초 실패 기록 경계를 보호한다. gameplay 컨테이너 전체의 잠금은 아니다.
		mutable std::mutex m_CommandMutex;
		std::deque<ROOM_COMMAND> m_InboundCommands;
		std::deque<ROOM_COMMAND> m_CleanupCommands;
		// LEAVE가 큐에 있거나 Tick이 처리 중인 session을 붙잡아 중복 정리와 새 일반 명령을 막는다.
		std::unordered_set<SESSION_ID> m_QueuedCleanupSessionIds;
		SERVER_ROOM_PERFORMANCE_METRICS m_PerformanceMetrics;
		// 아래 값은 룸 worker의 직전 로그 비교 표본이다. pending 문자열은 최대 최신 한 줄만 보관한다.
		SERVER_ROOM_PERFORMANCE_METRICS m_LastRoomPerfLogSample;
		std::string m_strPendingPerformanceDiagnostic;
		std::uint64_t m_iLastRoomPerfSnapshotDroppedCount = 0;
		std::uint64_t m_iLastRoomPerfReliableRejectedCount = 0;
		std::uint64_t m_iLastRoomPerfWireSendFailureCount = 0;
		std::size_t m_iLastRoomPerfOutboundHighWatermark = 0u;
		// 회수 준비가 false로 봉인하면 이후 Enqueue는 거절한다. ready와 달리 생성 성공 여부가 아니다.
		bool m_acceptsCommands = true;
		// 룸 처리에서 준비한 이동 요청을 소유한다. ServerApp의 dequeue가 가져간 뒤 실제 admission을 수행한다.
		std::deque<SERVER_WORLD_TRANSFER_REQUEST> m_PendingWorldTransfers;
		// Bern에서 Maharaka로 요청을 꺼낼 때 session별 선박·부두 좌표를 기억해 복귀 준비가 사용한다.
		// world 좌표는 m, yaw는 degree다. 실제 전환 성공 전 기록이며 256개 이상이면 기존 기억 전체를 비운다.
		struct SHIP_RETURN_STATE final
		{
			LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
			float fDockX = 0.f;
			float fDockY = 0.f;
			float fDockZ = 0.f;
			float fDockYawDegrees = 0.f;
		};
		std::unordered_map<SESSION_ID, SHIP_RETURN_STATE> m_MaharakaShipReturnBySession;
		// Try_DequeueWorldTransfer가 대표자와 batch 멤버에 호출한다. 다른 목적지나 선박 정보 부재는 해당 session 기억을 지운다.
		void Remember_ShipForWorldTransfer(const SERVER_WORLD_TRANSFER_REQUEST& transfer);
		struct PENDING_ESTHER_SUMMON final
		{
			const ESTHER_ROSTER_ENTRY* pRosterEntry = nullptr;
			LostArk::Shared::PLAYER_ID iCasterPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
			float fPositionX = 0.f;
			float fPositionY = 0.f;
			float fPositionZ = 0.f;
			float fYawDegrees = 0.f;
			float fRemainingSeconds = 0.f;
		};
		std::vector<PENDING_ESTHER_SUMMON> m_PendingEstherSummons;
		struct ESTHER_ZONE_RUNTIME final
		{
			const LostArk::Shared::EstherStrike::ZONE* pZone = nullptr;
			float fPositionX = 0.f;
			float fPositionZ = 0.f;
			std::uint32_t iEndTick = 0u;
			std::uint32_t iNextPulseTick = 0u;
		};
		std::vector<ESTHER_ZONE_RUNTIME> m_EstherZones;

		// ServerApp이 소유하는 연결을 weak_ptr로 참조한다. 룸은 아래 player 값과 ID lookup을 별도로 소유한다.
		std::unordered_map<SESSION_ID, std::weak_ptr<CClientSession>> m_Sessions;
		std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> m_Players;

		// 대사 한 번의 예약을 소유한다. prompt/trigger ID는 catalog 포인터가 아니며 실제 segment는 송신 직전에 ID로 조회한다.
		struct GUIDE_PROMPT_OCCURRENCE
		{
			std::string PromptId, TriggerId;

			// 송신 enqueue가 성공한 다음 읽을 segment index다. 0이면 공간 접촉 이탈로 취소할 수 있는 미시작 예약이다.
			std::size_t NextSegment = 0;
			int Priority = 0;
			bool IsSpaceEnter = false;

			// 같은 prompt를 실제로 발생시킨 공간 trigger ID들을 보관한다. 첫 segment 송신 성공 때 이 source들의 cooldown을 함께 기록한다.
			std::vector<std::string> SpaceTriggerIds;
			bool operator==(const GUIDE_PROMPT_OCCURRENCE&) const = default;
		};

		// 단일 Bern actor에 연결된 owner별 제어 상태다. 룸 실행이 직렬로 갱신하며 아래 큐/시계 자체에 별도 mutex는 없다.
		struct GUIDE_RUNTIME
		{
			// PlayerId는 안내 actor, AnchorId는 현재 같은 방 owner player다. owner 재입장 commit 뒤 AnchorId를 새 binding으로 바꾼다.
			LostArk::Shared::PLAYER_ID PlayerId = 0, AnchorId = 0;

			// ServerApp이 소유하는 실제 연결을 약하게 참조한다. worker는 사용 중 lock한 shared_ptr로 수명을 붙잡는다.
			std::weak_ptr<CClientSession> OwnerSession;

			// owner가 다른 world에 나간 동안에도 안내 소유 표시를 보낼 수 있도록 마지막 승인 entity ID를 보관한다.
			LostArk::Shared::NET_ENTITY_ID OwnerNetEntityId = 0;

			// 순서대로 월드 이탈 대기·실제 선박 승선 대기·도보 재접근 상태다. 월드/선박 대기는 일반 판단과 대사 진행을 건너뛴다.
			bool WaitingForOwner = false, WaitingForShip = false, ReturningOnFoot = false;

			// 현재 combo와 진행 중 skill 뒤에 적용할 대기 combo의 ID, 마지막 판단 이유다. Stop은 두 예약을 비운다.
			std::string ComboId, PendingComboId, Reason;

			// 승인된 공통 skill 실행만 다음 단계로 전진시킨다. 반복 combo는 이 index를 되돌려도 전체 제한 시간은 유지한다.
			std::size_t ComboStep = 0;

			// 모두 초 단위다. 판단 누적·combo 전체 시간·실행 거절 대기·원거리 복구 대기·행동 유지·대사 남은 시간을 나눈다.
			float ThinkElapsed = 0.f, ComboElapsed = 0.f, StepElapsed = 0.f,
				FarElapsed = 0.f, HoldElapsed = 0.f, PromptRemaining = 0.f;

			// 가이드가 공통 이동/skill/탈것 executor로 보내는 내부 명령 순번이다. 사용자 GUIDE_CONTROL 순번과 별도로 유지한다.
			std::uint32_t Sequence = 0;

			// 정확히 일치한 도움 명령의 처리 tick이다. 실제 skill 승인이나 prompt 송신 성공 시각은 아니다.
			std::map<std::string, std::uint32_t> CommandTicks;

			// 판단/추적용 행동 코드다. SERVER_PLAYER의 실제 eAction과 별개이며 게시 trace가 이 값과 점수를 소비한다.
			std::uint8_t Action = 0;
			float FollowScore = 0.f, EvadeScore = 0.f, CombatScore = 0.f;

			// 진행 중 맨 앞 대사는 유지하고 미시작 대사는 priority 내림차순/동순위 입력 순으로 보관한다.
			std::deque<GUIDE_PROMPT_OCCURRENCE> PromptQueue;

			// 첫 segment 송신 성공 tick만 기록하는 trigger cooldown이다. 취소·한도 초과·송신 실패는 기록하지 않는다.
			std::map<std::string, std::uint32_t> TriggerTicks;

			// 직전 owner 공간 접촉을 보관해 바깥→안 edge를 만든다. 처음 시작/복귀 위치는 Seed가 기준값으로 채운다.
			std::unordered_set<std::string> InsideBoxes;

			// 직전 관측한 boss별 pattern sequence다. 한 판단에서 모든 trigger를 검사한 뒤 갱신한다.
			std::map<LostArk::Shared::NET_ENTITY_ID, std::uint32_t> PatternSequences;
		};

		// 초기화가 읽은 안내 설정/정의를 룸이 소유한다. 아래 상태는 정의와 별개인 process-session 실행 상태다.
		CGuideCatalog m_GuideCatalog;

		// Bern의 배치 안내 actor 하나에 최대 한 owner만 연결한다. 별도 companion clone이나 인간 party slot은 만들지 않는다.
		std::map<SESSION_ID, GUIDE_RUNTIME> m_PersonalGuides;

		// 대사 segment 송신 시도 전에 올리고 0을 건너뛴다. 송신 실패에도 이 순번은 소비될 수 있다.
		std::uint32_t m_iGuideEventSequence = 0;

		// Loaded인 Bern 갱신마다 누적하는 초 값이다. owner 부재 또는 월드 대기이고 ThinkSeconds에 도달하면 소유 표시를 보내고 0으로 만든다.
		float m_fGuideOwnershipElapsed = 0.f;

		// session별 마지막 사용자 GUIDE_CONTROL 순번이다. actor 검사 이전에 갱신하고 Leave가 지워 새 Level의 순번을 받는다.
		std::unordered_map<SESSION_ID, std::uint32_t> m_GuideControlSequences;

		// Bern 초기 배치 actor의 room-local ID다. 안내 종료 후에도 같은 actor를 계속 사용한다.
		LostArk::Shared::PLAYER_ID m_iGuideReceptionId = 0;

		// 초기 actor 할당용 player ID 영역이다. 공통 NetEntityId는 별도 m_iNextNetEntityId에서 가져온다.
		LostArk::Shared::PLAYER_ID m_iNextGuidePlayerId = 0x80000000u;
		// 시작한 스킬의 버프를 시전자·같은 방 파티·지정 대상에게 적용한다.
		void Apply_SkillBuffs(SERVER_PLAYER& caster, std::uint32_t skillId,
			std::uint32_t serverTick);
		std::unordered_map<SESSION_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdBySessionId;
		std::unordered_map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::PLAYER_ID>
			m_PlayerIdByEntityId;

		// PLAYER_ID와 파티 ID는 방 안에서만 유효하며 0은 파티 없음이다. 초대·명단은 이 방이 소유한다.
		// 파티 전환은 전원 입장을 준비한 뒤 대상 방의 새 ID로 함께 구성하며, 방 사이의 일반 파티 ID는 유지하지 않는다.
		std::uint32_t m_iNextPartyId = 1u;
		std::unordered_map<LostArk::Shared::PLAYER_ID, std::uint32_t>
			m_PartyIdByPlayerId;
		std::unordered_map<std::uint32_t, std::vector<LostArk::Shared::PLAYER_ID>>
			m_PartyMembersByPartyId;
		// 대상별 미응답 초대는 하나만 유지하고 새 초대로 교체한다.
		std::unordered_map<LostArk::Shared::PLAYER_ID, LostArk::Shared::PLAYER_ID>
			m_PendingPartyInviteByTargetPlayerId;
		// 플레이어별 최신 전환 실패를 보관한다. 송신 큐가 가득 차면 통지를 미루고 기존 파티 연결은 유지한다.
		std::unordered_map<SESSION_ID, LostArk::Shared::S2C_PARTY_TRANSFER_RESULT>
			m_PendingPartyTransferResults;

		// 파티 레이드 입장 투표 상태. struct RAID_ENTRY_PROPOSAL은 위 메서드 선언부에 정의한다.
		std::vector<RAID_ENTRY_PROPOSAL> m_RaidEntryProposals;
		std::uint32_t m_iNextRaidEntryProposalId = 1u;
		// Bern에서 승인받은 session을 입장 순서대로 유지하는 콜로세움 대기열이다.
		struct COLOSSEUM_QUEUE_ENTRY
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			std::uint32_t iRequestSequence = 0u;
		};
		std::vector<COLOSSEUM_QUEUE_ENTRY> m_ColosseumQueue;
		bool m_bColosseumTransferPending = false;
		std::uint32_t m_iColosseumRetryTick = 0u;
		std::uint64_t m_iColosseumMatchId = 0u;
		LostArk::Shared::COLOSSEUM_MATCH_PHASE m_eColosseumPhase = LostArk::Shared::COLOSSEUM_MATCH_PHASE::RECRUITING;
		std::uint8_t m_iColosseumWinnerTeam = 255u;
		std::uint32_t m_iColosseumRevision = 1u;
		std::array<std::uint32_t, 2> m_ColosseumTeamPartyIds{};
		enum class COLOSSEUM_TACTIC : std::uint8_t { WAIT, ENGAGE, REPOSITION, RETREAT, EVADE, RECOVER };
		struct COLOSSEUM_MERCENARY_RUNTIME final
		{
			float fThinkElapsed = 0.f;
			float fSenseElapsed = 0.f, fDecisionInterval = .2f, fThreatAgeSeconds = 0.f;
			std::uint32_t iSequence = 0u;
			// 경기가 소유하는 입장 시각이므로 플레이어 부활·전술 초기화 뒤에도 유지한다.
			std::optional<std::uint32_t> iLastAltVAdmissionTick;
			std::uint64_t iRandomState = 0u;
			std::vector<LostArk::Shared::SKILL_ID> AvailableSkills;
			std::array<LostArk::Shared::SKILL_ID, 3> RecentSkills{};
			std::size_t iRecentSkillCursor = 0u;
			LostArk::Shared::SKILL_ID iLastSkillId = LostArk::Shared::INVALID_SKILL_ID;
			LostArk::Shared::NET_ENTITY_ID iTargetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t iTargetSelectedTick = 0u, iTacticUntilTick = 0u;
			std::uint32_t iNextAttackTick = 0u, iNextEvadeTick = 0u, iRetreatAllowedTick = 0u;
			std::uint32_t iLastDamageTick = 0u, iObservedHp = 0u;
			COLOSSEUM_TACTIC eTactic = COLOSSEUM_TACTIC::ENGAGE;
			CColosseumThreatAssessment Threats;
			const char* pReason = "Waiting for recruitment";
		};
		std::map<LostArk::Shared::PLAYER_ID, COLOSSEUM_MERCENARY_RUNTIME> m_ColosseumMercenaries;
		std::unordered_map<SESSION_ID, std::uint32_t> m_ColosseumRecruitSequences;
		std::vector<SESSION_ID> m_ColosseumSessions;
		std::uint32_t m_iColosseumPhaseStart = 0u, m_iColosseumPhaseEnd = 0u;
		std::uint32_t m_iColosseumScores[2]{};
		std::uint32_t m_iColosseumKillSequence = 0u;
		std::vector<LostArk::Shared::COLOSSEUM_KILL_EVENT> m_ColosseumRecentKills;
		std::uint32_t m_iColosseumQueueDeadline = 0u, m_iColosseumQueueBroadcastTick = 0u;
		std::array<std::uint8_t, 2> m_ColosseumInitialHumans{};
		GATE_PROGRESS_STATE m_GateProgress;
		std::uint32_t m_iArenaAssemblyStartTick = 0u;
		std::uint32_t m_iArenaAssemblyRaidEpoch = 0u;
		bool m_bArenaAssemblyAttempted = false;
		std::vector<LostArk::Shared::PLAYER_ID> m_ArenaAssemblyParticipants;
		std::vector<SERVER_MVP_LEDGER_ROW> m_GateMvpLedger;
		std::uint32_t m_iNextGateProposalId = 1u;

		// 월드·공간 시스템과 세대 보관 멤버는 simulation별로 소유한다.
		// 불변 catalog 객체는 공유할 수 있지만 active·예약 상태는 방마다 따로 보관한다.
		LostArk::Shared::WORLD_ID m_eWorldId = LostArk::Shared::WORLD_ID::END;
		CWorldBootstrap m_WorldBootstrap;
		CGameplayCatalogGenerations m_GameplayCatalog;
		// 수치 Stage 성공 시 entry snapshot과 원본→대체 catalog 소유권을 예약한다. Commit 또는 Abort가 소비/해제한다.
		std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY> m_StagedNumericEntries;
		std::vector<std::pair<std::shared_ptr<const CGameplayCatalog>, std::shared_ptr<const CGameplayCatalog>>>
			m_StagedNumericCatalogRemaps;
		// 아이템·탈것·칭호·보상 정의는 룸 생성 시 읽는다. gameplay 세대의 수치 교체와 별도 객체다.
		CItemCatalog m_ItemCatalog;
		CVehicleCatalog m_VehicleCatalog;
		CHonorTitleCatalog m_HonorTitleCatalog;
		CValtanClearRewards m_ValtanClearRewards;
		CServerNavigation m_ServerNavigation;
		CServerCollisionSystem m_ServerCollisionSystem;
		CServerTriggerSystem m_ServerTriggerSystem;
        struct MAHARAKA_WATERPANG_AI final
        {
            std::uint32_t iSlot = 0u, iSequence = 0u, iNextThinkTick = 0u, iNextMoveTick = 0u, iNextShotTick = 0u, iSkillSlot = 0u;
        };
        std::map<LostArk::Shared::PLAYER_ID, MAHARAKA_WATERPANG_AI> m_MaharakaWaterpangAI;
        LostArk::Shared::PLAYER_ID m_iNextWaterpangAIPlayerId = 0x90000000u;
        std::uint32_t m_iWaterpangAIRetryTick = 0u;
        LostArk::Shared::MAHARAKA_AI_TUNING m_MaharakaAITuning;
        bool m_bMaharakaAITuningLoaded = false;
        std::string m_strMaharakaAISourceBytes;
		// 방 전체의 워터팡 시작 연출을 빈 방이 될 때까지 보관해 늦은 입장에도 전달한다.
		std::optional<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_MaharakaWaterpangIntro;
		// 워터팡 발사부터 소멸까지의 물줄기 상태다. 이미 맞은 대상을 기억해 중복 피격을 막는다.
		struct MAHARAKA_WATERGUN_SHOT final
		{
			LostArk::Shared::PLAYER_ID iOwnerId = 0;
			std::uint32_t iSkillId = 0u;
			std::uint32_t iSpawnTick = 0u;
            std::uint32_t iProjectileIndex = 0u;
            LostArk::Shared::COMBAT_OBJECT_ID iVisualObjectId = LostArk::Shared::INVALID_COMBAT_OBJECT_ID;
            LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			float fAimDistanceM = 0.f;
			bool bLaunched = false;
			bool bSpent = false;
			float fX = 0.f, fY = 0.f, fZ = 0.f;
			float fDirX = 0.f, fDirZ = 1.f;
			float fTravelM = 0.f;
			float fReachM = 0.f;
			std::vector<LostArk::Shared::PLAYER_ID> Struck;
		};
		std::vector<MAHARAKA_WATERGUN_SHOT> m_MaharakaWaterGunShots;
		// Debug 폭포·물대포 재생은 다음 요청으로 교체하며 늦은 입장에도 전달하도록 보관한다.
		std::optional<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_MaharakaWaterpangDebugEvent;
		CSpawnGroupBootstrap m_SpawnGroupBootstrap;
		CSpawnGroupRuntime m_SpawnGroupRuntime;
		std::mt19937 m_MarioLayoutRandom{std::random_device{}()};
		std::mt19937 m_EquipmentUpgradeRandom{std::random_device{}()};
		// Mario 1~4단계별 터진 공 기록이며 각 bit는 bootstrap 슬롯을 가리킨다.
		std::uint16_t m_MarioPoppedBalls[5] = {};
		std::uint8_t m_iNextMarioEntryStage = 1u;
		struct KOUKU_CARD_RAIN_SOLDIER_STATE final
		{
			LostArk::Shared::NET_ENTITY_ID ownerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			std::uint32_t patternSequence = 0u, expiresAt = 0u;
		};
		std::map<LostArk::Shared::NET_ENTITY_ID, KOUKU_CARD_RAIN_SOLDIER_STATE> m_KoukuCardRainSoldiers;
		CKoukuCardMazeRuntime m_KoukuCardMaze;
		CKoukuBingoRuntime m_KoukuBingo;
        struct KOUKU_BINGO_DURATION final
        {
            LostArk::Shared::NET_ENTITY_ID iOwnerId = LostArk::Shared::INVALID_NET_ENTITY_ID;
            std::uint32_t iPatternSequence = 0u, iEndTick = 0u, iActivationTick = 0u;
            BOSS_BINGO_BOARD_SETTINGS Settings;
            bool bActivated = false, bInitializeCells = false;
            std::uint32_t iNextBombTick = 0u, iNextHammerTick = 0u, iNextMadnessTick = 0u;
            std::uint32_t iMarkedBombCount = 0u;
            float fHammerHalfForwardM = 0.f, fHammerHalfWidthM = 0.f;
            bool bEncounterOwned = false, bSpecialPatternPending = false;
            bool bLastLineCompletionSucceeded = false;
            bool bLineRewardSinceLastJudgement = false;
            std::uint32_t iLastLineJudgementTick = 0u;
            struct HAMMER { std::int32_t anchor = -1; std::uint32_t startTick = 0u; };
            std::array<HAMMER, 2u> Hammers{};
        } m_KoukuBingoDuration;
        std::uint32_t m_iKoukuBingoBoardEpoch = 0u;
        void Begin_KoukuBingoDuration(const SERVER_WORLD_ENTITY& owner,
            const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, std::uint32_t tick);
        void Stop_KoukuBingoDuration(bool clearBoard);
        void Activate_KoukuBingoBoard(std::uint32_t tick);

		std::uint32_t m_iCardMazeMarchStartTick = 0u;
		std::uint32_t m_iCardMazeCycleMs = 0u;
		std::map<LostArk::Shared::PLAYER_ID, std::pair<float, float>> m_CardMazePreviousPositions;
		std::map<LostArk::Shared::PLAYER_ID, std::uint32_t> m_CardMazeContactTicks;
		LostArk::Shared::NET_ENTITY_ID m_iCardMazeClownBoxId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t m_iCardMazeClownBoxDueTick = 0u;
		bool m_bCardMazeClownBoxDestroyed = false;
		CPlayerSkillSystem m_PlayerSkillSystem;
		CCombatObjectRuntime m_CombatObjectRuntime;
		CMonsterBrain m_MonsterBrain;
		CNpcBehaviorRuntime m_NpcBehaviorRuntime;
		CValtanBrain m_ValtanBrain;
		CKoukuSaydonBrain m_KoukuSaydonBrain;
		std::unique_ptr<CValtanBrain> m_DependentValtanBrain =
			std::make_unique<CValtanBrain>();
		VALTAN_DECISION_TRACE_REVISION_STATE m_ValtanDecisionTraceRevision;
		CEstherSkillSystem m_EstherSkillSystem;
		CWorldDestructionBootstrap m_WorldDestructionBootstrap;
		CWorldDestructionRuntime m_WorldDestructionRuntime;
		struct WORLD_PICKUP_RUNTIME final
		{
			WORLD_PICKUP_DESCRIPTOR Descriptor;
			LostArk::Shared::WORLD_PICKUP_SNAPSHOT Snapshot;
		};
		std::vector<WORLD_PICKUP_RUNTIME> m_WorldPickups;
		std::uint32_t m_iWorldPickupEncounterEpoch = 0u;
		// 전투 중 기둥을 다시 세우고 부술 수 있도록 가역 상태를 소유한다.
		CEncounterPropRuntime m_EncounterPropRuntime;
		// 주 Valtan 사망 시 despawn 전에 클리어를 기록한다. 마지막 플레이어 이탈의 방 초기화에서 해제한다.
		bool m_bValtanRaidCleared = false;
		// 기둥 시험의 생성 후 일괄 파괴 예약 tick과 대기 플래그다. 제품의 stage별 슬롯 파괴와 별도로 유지한다.
		std::uint32_t m_iPillarAuditionBreakTick = 0u;
		bool m_bPillarAuditionCycleArmed = false;
		// 룸이 실제 월드 개체 값을 소유한다. vector 변경을 넘겨 개별 원소 포인터를 계속 보관할 수는 없다.
		std::vector<SERVER_WORLD_ENTITY> m_WorldEntities;
		// 유효한 simulation 단계 시작에서 비우고 그 Tick의 snapshot에 담을 피해 event를 소유한다.
		// snapshot 없이 조기 중단하면 전송을 보장하지 않으며 별도 재송신 원장은 아니다.
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_TickDamageEvents;
		// 명령 처리 중 만든 회복/피해 표시를 위 목록의 clear와 분리해 보관한다.
		// simulation 시작 뒤 wire 개수 상한까지 옮기고 원본을 비운다. 상한 초과 항목의 전송은 보장하지 않는다.
		std::vector<LostArk::Shared::DAMAGE_EVENT> m_PendingCommandDamageEvents;
		std::vector<LostArk::Shared::BOSS_COMBAT_EVENT>
			m_TickBossCombatEvents;
		std::string m_strStatus;
		// 최초 runtime 실패를 따로 보관한다. 일반 status는 이후 달라질 수 있으며 ready false는 정상 Tick을 중단한다.
		SERVER_ROOM_RUNTIME_FAILURE m_RuntimeFailure;
		bool m_isReady = false;

		LostArk::Shared::PLAYER_ID m_iNextPlayerId = 1;
		LostArk::Shared::NET_ENTITY_ID m_iNextNetEntityId = 100;
		// 유효한 simulation 단계가 계산하는 서버 tick이다. 0을 건너뛰며 Tick 호출 횟수 계측과 일치한다고 가정하지 않는다.
		std::uint32_t m_iServerTick = 0;
		std::uint64_t m_iNextWorldDestructionEventSequence = 1u;
		std::uint64_t m_iNextBossCombatEventSequence = 1u;
		// 구 HP 임계 시험의 잔여 상태는 초기화·배타 검사에 남아 있다. 현재 HP 구명령은 거절한다.
		// preset sequence와 stable-ID 패턴 receipt를 나눠 서로 다른 요청의 재전송을 판정한다.
		std::uint32_t m_iValtanAuditionArmedHealthBar = 0;
		std::unordered_map<SESSION_ID, std::uint32_t>
			m_ValtanAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_ID_COMMAND_RECEIPT final
		{
			LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST Request;
			LostArk::Shared::VALTAN_AUDITION_RESULT Result =
				LostArk::Shared::VALTAN_AUDITION_RESULT::REJECTED_STALE_REQUEST;
			std::uint32_t iCurrentHealthBar = 0u;
			// 현재 시험이 끝나도 마지막 lifecycle을 보관해, QUEUED 요청의 같은 재시도에 완료 상태까지 응답한다.
			std::optional<LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE>
				LastLifecycle;
		};
		// stable-ID Play/Restart의 원문과 결과를 보관한다. 완전히 같은 재시도에만 결과를 재사용한다.
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_ID_COMMAND_RECEIPT>
			m_ValtanPatternIdAuditionSequenceBySessionId;
		struct VALTAN_PATTERN_FLOW_COMMAND_RECEIPT final
		{
			std::uint32_t iSequence = 0u;
			std::uint32_t iRoomFlowEpoch = 0u;
			std::string strFlowId;
			std::string strFlowRevision;
			std::string strRequestIdentity;
			LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
			LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT eResult =
				LostArk::Shared::VALTAN_PATTERN_FLOW_RESULT::REJECTED_STALE_FLOW;
			std::string strReason;
			// 동일 Start 재시도는 COMPLETED_HOLD 이후에도 마지막 lifecycle을 돌려주며 새 실행을 만들지 않는다.
			std::optional<LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE>
				LastLifecycle;
		};
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowStartSequenceBySessionId;
		std::unordered_map<SESSION_ID, VALTAN_PATTERN_FLOW_COMMAND_RECEIPT>
			m_ValtanPatternFlowControlSequenceBySessionId;
		struct TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextKoukuSaydonPatternAuditionEpoch = 1u;
		struct KOUKU_DRAFT_UPLOAD final
		{
			std::uint32_t iRequestSequence = 0u, iTotalBytes = 0u;
			std::uint64_t iStartedAtMs = 0u;
			LostArk::Shared::GameplayDataRevision RowsRevision{};
			std::string Rows;
		};
		std::unordered_map<SESSION_ID, KOUKU_DRAFT_UPLOAD> m_KoukuDraftUploads;
		KOUKUSAYDON_PATTERN_AUDITION_STATE m_KoukuSaydonPatternAudition;
		std::shared_ptr<const CGameplayCatalog> m_pKoukuPublishedProductGeneration;
		std::unordered_map<SESSION_ID, KOUKUSAYDON_PATTERN_AUDITION_RECEIPT>
			m_KoukuSaydonPatternAuditionReceiptBySessionId;
		std::vector<TARGETED_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE>
			m_PendingKoukuSaydonPatternAuditionLifecycle;
		struct TARGETED_VALTAN_AUDITION_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_VALTAN_AUDITION_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanAuditionEpoch = 1u;
		std::vector<TARGETED_VALTAN_AUDITION_LIFECYCLE>
			m_PendingValtanAuditionLifecycle;
		struct TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::S2C_DEBUG_VALTAN_PATTERN_FLOW_LIFECYCLE Message;
		};
		std::uint32_t m_iNextValtanPatternFlowEpoch = 1u;
		std::vector<TARGETED_VALTAN_PATTERN_FLOW_LIFECYCLE>
			m_PendingValtanPatternFlowLifecycle;
		VALTAN_PATTERN_ID_AUDITION_STATE m_ValtanPatternIdAudition;
		std::optional<VALTAN_NEXT_PATTERN_RESERVATION> m_ValtanNextPattern;
		std::unordered_map<SESSION_ID, VALTAN_NEXT_PATTERN_COMMAND_RECEIPT>
			m_ValtanNextPatternReceiptBySessionId;
		VALTAN_PATTERN_FLOW_AUDITION_STATE m_ValtanPatternFlowAudition;
		VALTAN_TIMELINE_AUDITION_STATE m_ValtanTimelineAudition;
		VALTAN_FIGHT_PAGE_START_STATE m_ValtanFightPageStart;
	};
}
