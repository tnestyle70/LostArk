#pragma once

#include "GameplayCatalog.h"
#include "ServerCombatGeometry.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"

#include "Network/PacketMessages.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace LostArk::Server
{
	// Room이 이 runtime에 등록한 지속 공격/표현 객체의 source 종류다. 생성 어댑터가 정의를 복사하고 실행 중 위치·타격 소비 상태는 따로 진행한다.
	enum class SERVER_COMBAT_OBJECT_SOURCE_KIND : std::uint8_t
	{
		PLAYER,
		WORLD_ENTITY
	};

	// CONTACT는 대상별 접촉 기록, TIMED는 hit 하나의 공통 소비 횟수로 반복을 관리한다.
	enum class SERVER_COMBAT_OBJECT_HIT_TRIGGER : std::uint8_t
	{
		CONTACT,
		TIMED
	};

	// POSE는 현재 도형을 검사한다. SWEPT도 모든 도형을 연속 검사하지 않으며 현재 구현의 추가 sweep은 원형에만 적용된다.
	enum class SERVER_COMBAT_OBJECT_CONTACT_SAMPLING : std::uint8_t
	{
		POSE,
		SWEPT
	};

	// 생성 시 복사한 타격 정의와 실행 중 소비 횟수를 함께 소유한다. 실제 HP/밀기 적용은 ServerCombatHitRuntime 또는 PvP 어댑터에 넘긴다.
	struct SERVER_COMBAT_OBJECT_HIT_RUNTIME final
	{
		std::string strHitId;
		std::string strDamageProfileId,
			strNumericBalanceId;
		SERVER_COMBAT_OBJECT_HIT_TRIGGER eTrigger = SERVER_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
		SERVER_COMBAT_OBJECT_CONTACT_SAMPLING eContactSampling =
		    SERVER_COMBAT_OBJECT_CONTACT_SAMPLING::POSE;
		// 오브젝트 생성 뒤 경과 ms다. iEndMs는 CONTACT 종료에 사용하고 TIMED는 iAtMs와 반복 간격으로 예약한다.
		std::uint32_t iAtMs = 0u;
		std::uint32_t iEndMs = 0u,
			iDamagePercent = 0u;
		bool bInstantDeath = false,
			bIgnoreDefense = false;
		float fOffsetForwardM = 0.f,
			fOffsetRightM = 0.f,
			fYawOffsetDegrees = 0.f;
		std::uint32_t iRepeatIntervalMs = 0u;
		SERVER_COMBAT_SHAPE_XZ Shape;
		// 반복마다 사용할 raw 피해를 생성 때 분할해 저장한다. 숫자 밸런스 갱신이 이 값을 바꿀 수 있고 PvP는 현재 skill 정의도 조회한다.
		std::vector<std::uint32_t> RepeatRawDamage;
		// PvP 어댑터는 생성 때 전체 시전 피해와 독립 hit 채널의 구간을 저장한다.
		std::uint64_t iPlayerWholeCastDamage = 0u;
		std::uint32_t iPlayerSubHitTotal = 1u,
			iPlayerSubHitBase = 0u;
		std::uint32_t iPlayerResultKind = 0u;
		std::uint32_t iStaggerDamage = 0u;
		std::uint32_t iStaggerMaximumDivisor = 0u;
		std::uint32_t iPartDamage = 0u;
		std::uint32_t iCounterPower = 0u;
		// 생성 때 skill 정의에서 복사한 기본 슬롯 counter 자격이다. 숫자 밸런스 갱신에서도 다시 계산하지 않는다.
		bool bCounterFromPrimarySlot = false;
		// 밀기 거리/상승 높이는 m, 밀기/넘어짐 기간은 ms다. 최종 이동과 저항 처리는 피해 적용 경계가 맡는다.
		float fPushRangeM = 0.f;
		float fRiseHeightM = 0.f;
		bool bForcePush = false,
			bPushFromBoss = false;
		std::uint32_t iPushMs = 0u;
		bool bKnockdown = false;
		std::uint32_t iDownMs = 0u;
		// TIMED 예약을 소비한 횟수다. 맞은 대상이 없거나 피해 적용이 거절돼도 예약을 검사한 뒤 증가한다.
		std::uint32_t iAppliedTimedCount = 0u;
	};

	// HP 판정과 별개인 Client 표현 예약이다. Update가 한 번 큐에 넣으면 bApplied로 중복 발행을 막는다.
	struct SERVER_COMBAT_OBJECT_PRESENTATION_PULSE_RUNTIME final
	{
		std::string strPresentationEventId;
		std::uint32_t iAtMs = 0u;
		bool bApplied = false;
	};

	// CONTACT의 (대상 entity ID, hit index)별 반복 기록이다. 다음 허용 시각은 오브젝트 생성 뒤 경과 ms다.
	struct SERVER_COMBAT_OBJECT_CONTACT_MARK final
	{
		LostArk::Shared::NET_ENTITY_ID iTargetNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint16_t iHitIndex = 0u;
		std::uint32_t iAppliedCount = 0u;
		float fNextMilliseconds = 0.f;
	};

	// 서버 월드 위치는 m, 방향은 XZ 전방, yaw는 degree다. 판정 도형은 XZ를 사용하며 이 구조만으로 높이 충돌을 보장하지 않는다.
	struct SERVER_COMBAT_OBJECT_POSE final
	{
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fDirectionX = 0.f;
		float fDirectionZ = 1.f;
		float fYawDegrees = 0.f;
	};

	// 보스의 현재 stage가 이미 바뀌어도 원래 발생 회차/action을 알 수 있도록 객체가 identity와 이동 전후 pose를 소유한다.
	struct SERVER_COMBAT_OBJECT_LIVE_STATE final
	{
		std::uint32_t iOwnerPatternSequence = 0u;
		std::string strOwnerPatternId;
		std::string strOwnerStageActionId;
		SERVER_COMBAT_OBJECT_POSE PreviousPose;
		SERVER_COMBAT_OBJECT_POSE CurrentPose;
	};

	// GameRoom이 먼저 정한 월드 위치(m)를 받는다. UNTIL_FIRST_PULSE 정책 또는 PER_ALIVE_PLAYER의 명시적 flag가 첫 TIMED 소비까지 추적을 켠다.
	struct SERVER_COMBAT_OBJECT_LOCKED_TARGET final
	{
		LostArk::Shared::NET_ENTITY_ID iNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		bool bTrackUntilFirstPulse = false;
	};

	struct SERVER_BOSS_PATTERN_HIT;
	struct SERVER_BATTLE_ITEM_USE;

	// Room 소유 runtime vector의 한 실행 인스턴스다. 문자열/타격/표현/접촉 기록을 값으로 소유하며 source 포인터는 보관하지 않는다.
	struct SERVER_COMBAT_OBJECT final
	{
		// CombatObject ID와 source NetEntity ID는 별개다. 오브젝트 ID는 현재 live/staged 집합과 충돌하지 않게 할당한다.
		LostArk::Shared::COMBAT_OBJECT_ID iCombatObjectId = LostArk::Shared::INVALID_COMBAT_OBJECT_ID;
		SERVER_COMBAT_OBJECT_SOURCE_KIND eSourceKind = SERVER_COMBAT_OBJECT_SOURCE_KIND::PLAYER;
		LostArk::Shared::PLAYER_ID iSourcePlayerId = LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::SKILL_ID iSourceSkillId = LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::NET_ENTITY_ID iLockedTargetNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iSpawnTick = 0u;
		std::string strCombatObjectArchetypeId;
		std::string strSourceArchetypeId;
		std::string strClientVisualId;
		float fUniformScale = 1.f;
		// Client 표현 메시지에 함께 보내는 정의 revision이다. 숫자 밸런스 갱신은 별도로 타격 값을 바꿀 수 있다.
		LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
		bool bReplicated = false;
		bool bTrackLockedTargetUntilFirstPulse = false;
		// persistent 수명, homing 방향, Room 외부 위치 공급을 별도 flag로 둔다. 물총/추적/Showtime 소비자가 필요한 조합을 정한다.
		bool bPersistentLifetime = false,
			bHoming = false;
		bool bRoomOwnedTracking = false,
			bHasExternalPoseStep = false;
		LostArk::Shared::MECHANIC_CARD_SYMBOL eDamageImmuneCardSymbol =
		    LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
		// 접촉 종료용 작은 원의 반경(m)과 표현 event ID다. 실제 피해 도형 Hits와 분리한다.
		float fContactPresentationRadiusM = 0.f;
		std::string strContactPresentationId;
		// 잠근 대상을 radial 배치로 추적할 때 최초의 월드 XZ offset을 유지한다. 모든 ordinal이 대상 중심으로 합쳐지는 것을 막는다.
		float fLockedTargetOffsetX = 0.f;
		float fLockedTargetOffsetZ = 0.f;
		SERVER_COMBAT_OBJECT_LIVE_STATE LiveState;
		// 투척물은 접촉 대상이 정해지거나 수명/거리 만료에 도달하면 폭발 판정을 수행한다. PvP 경기에서는 상대 플레이어도 접촉 후보다.
		bool bDetonateOnContact = false;
		float fProjectileStartY = 0.f,
			fProjectileTargetY = 0.f;
		float fProjectileDistanceM = 0.f,
			fProjectileArcHeightM = 0.f;
		// 속도는 m/s, 남은 거리는 m다. 이동 조건은 양의 남은 거리 또는 persistent이며 -1을 자동 무제한 이동으로 해석하지 않는다.
		float fSpeedMps = 0.f;
		float fRemainingDistanceM = -1.f;
		std::uint32_t iMovementStartDelayMs = 0u;
		bool bExpireOnDistanceEnd = true;
		// 보스 타격의 엄폐/연쇄 계산에 제공하는 원 반경(m)이다. OwnerHitChain은 소유 action의 도형으로 활성화할 지연을 계산한다.
		float fCoverRadiusM = 0.f;
		BOSS_COMBAT_OBJECT_OWNER_HIT_CHAIN OwnerHitChain;
		bool bOwnerHitChainArmed = false,
			bOwnerHitChainWarningApplied = false;
		std::uint32_t iOwnerHitChainArmedTick = 0u,
			iOwnerHitChainDelayMs = 0u;
		// 수명과 경과 시간은 ms다. persistent는 수명 감소를 생략하지만 거리 만료 조건까지 자동 해제하지 않는다.
		float fRemainingMilliseconds = 0.f;
		float fElapsedMilliseconds = 0.f,
			fPreviousElapsedMilliseconds = 0.f;
		std::vector<SERVER_COMBAT_OBJECT_HIT_RUNTIME> Hits;
		std::vector<SERVER_COMBAT_OBJECT_PRESENTATION_PULSE_RUNTIME> PresentationPulses;
		std::vector<SERVER_COMBAT_OBJECT_CONTACT_MARK> ContactMarks;
	};

	// live를 바꾸지 않고 생성 후보를 소유한다. 보스 stage의 EXIT/ENTER가 공유하면 뒤 action 실패 때 앞 action의 후보도 함께 폐기할 수 있다.
	struct SERVER_COMBAT_OBJECT_TRANSACTION final
	{
		std::uint64_t iExpectedRevision = 0u;
		LostArk::Shared::COMBAT_OBJECT_ID iNextCombatObjectId = LostArk::Shared::INVALID_COMBAT_OBJECT_ID;
		std::vector<SERVER_COMBAT_OBJECT> Objects;
		// spawn 메시지도 분리된 후보에 준비한다. Commit은 live와 큐의 지역 복사본을 완성한 뒤 swap해 명시적 실패에 부분 live 반영이 없게 한다.
		std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED> Spawned;
	};

	// GameRoom이 실행 상태와 복제 대기열을 소유하도록 묶는다. 내부 잠금은 없으며 호출 중 컨테이너를 다른 스레드에서 바꾸지 않아야 한다.
	// Stage는 후보만 준비하고 Commit 뒤 Update가 피해/표현/제거를 진행한다. 네트워크 송신은 GameRoom의 별도 책임이다.
	class CCombatObjectRuntime final
	{
	public:
		// 현재 revision과 다음 ID만 담은 빈 후보를 반환한다. live 오브젝트 전체의 snapshot을 복사하는 함수는 아니다.
		[[nodiscard]] SERVER_COMBAT_OBJECT_TRANSACTION Begin_Transaction() const;

		// GameRoom의 물총 시뮬레이션이 피해와 이동을 확정한다. 이 생성은 복제 표현만 준비하고 각 pose는 Room이 공급한다.
		bool Stage_WaterGunPresentation(SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
		                                const SERVER_PLAYER& source,
		                                std::uint32_t skillId,
		                                const CGameplayCatalog& catalog,
		                                std::uint32_t spawnTick,
		                                std::string& status,
		                                std::uint32_t projectileIndex = 0u) const;
		// Room의 물총 판정 결과를 받아 선택적으로 impact 표현을 큐에 넣고 제거한다. identity 불일치의 false는 live 상태를 보존한다.
		bool Finish_WaterGunPresentation(LostArk::Shared::COMBAT_OBJECT_ID objectId,
		                                 LostArk::Shared::NET_ENTITY_ID sourceId,
		                                 std::uint32_t spawnTick,
		                                 std::uint32_t serverTick,
		                                 bool impact);

		// GameRoom_Inventory가 지면/사용 자격을 확인한 투척 아이템을 준비한다. 입력은 m, 아이템 cm 필드는 내부에서 m로 변환한다.
		// 실패한 transaction을 Commit하지 않는다. live는 그대로지만 준비 중 ID/출력 후보나 할당 예외의 부분 결과는 남을 수 있다.
		bool Stage_BattleItemProjectile(SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
		                                const SERVER_PLAYER& source,
		                                const SERVER_BATTLE_ITEM_USE& use,
		                                float targetX,
		                                float targetY,
		                                float targetZ,
		                                const CGameplayCatalog& catalog,
		                                std::uint32_t serverTick,
		                                std::string& status) const;

		// 스킬 projectile 정의를 공용 실행 값으로 옮기는 어댑터다. 현재 직접 호출은 Server 계약 테스트에서 확인되며 일반 player.Projectiles 경로와 별개다.
		// 생성 후보는 bReplicated=false이며 stageIndex는 현재 구현에서 사용하지 않는다. false면 transaction을 폐기해야 한다.
		bool Stage_PlayerProjectile(SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
		                            const SERVER_PLAYER& source,
		                            const PLAYER_SKILL_DEFINITION& skill,
		                            const PLAYER_SKILL_PROJECTILE& definition,
		                            std::uint32_t stageIndex,
		                            std::uint32_t projectileIndex,
		                            std::uint64_t totalDamage,
		                            std::uint32_t subHitTotal,
		                            std::uint32_t subHitBase,
		                            std::uint32_t serverTick,
		                            std::string& status) const;

		// GameRoom의 보스 stage/기믹 생성이 호출한다. lockedTarget/volley/catalog는 호출 동안 차용하고 필요한 값을 transaction에 복사한다.
		// false는 live를 바꾸지 않지만 앞서 준비한 volley 항목/다음 ID는 남을 수 있다. 모든 할당 예외를 false로 바꾸지는 않는다.
		bool Stage_BossCombatObject(SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
		                            const SERVER_WORLD_ENTITY& boss,
		                            const SERVER_COMBAT_OBJECT_LOCKED_TARGET* lockedTarget,
		                            const BOSS_COMBAT_OBJECT_DEFINITION& definition,
		                            const BOSS_COMBAT_OBJECT_VOLLEY* volley,
		                            const CGameplayCatalog& catalog,
		                            std::uint32_t count,
		                            std::uint32_t serverTick,
		                            std::string& status) const;

		// revision 불일치 외에도 전체 개수/복제 후보 수 불일치와 bad_alloc에 false를 반환한다. 명시적 false는 live와 spawn 큐를 보존한다.
		bool Commit(SERVER_COMBAT_OBJECT_TRANSACTION&& transaction);

		// GameRoom 고정 tick에서 이동·현재 타격·수명 정리를 수행한다. delta는 초, serverTick은 30Hz이며 입력 delta를 여기서 재검증하지 않는다.
		// players/worldEntities를 차용해 HP/피격 상태를 바꾸고 outDamageEvents에는 추가한다. 전체 tick rollback을 제공하지 않는다.
		void Update(std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		            std::vector<SERVER_WORLD_ENTITY>& worldEntities,
		            const CGameplayCatalog& catalog,
		            float fixedDeltaSeconds,
		            std::uint32_t serverTick,
		            std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents);

		// ValtanBrain이 내보낸 소유 action 타격으로 연쇄 오브젝트를 무장한다. 이번 tick에 무장한 항목만 delta 0으로 즉시 재평가한다.
		void Apply_OwnerHits(const std::vector<SERVER_BOSS_PATTERN_HIT>& hits,
		                     std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		                     std::vector<SERVER_WORLD_ENTITY>& worldEntities,
		                     const CGameplayCatalog& catalog,
		                     std::uint32_t serverTick,
		                     std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents);

		// 퇴장/사망/기믹 취소에서 해당 source의 모든 오브젝트를 제거하고 복제 대상이면 despawn을 큐에 넣는다.
		void Cancel_Source(LostArk::Shared::NET_ENTITY_ID sourceNetEntityId);
		// Room이 공급한 위치를 identity가 일치하는 객체에 반영한다. hit 없는 표현 또는 Room 추적이며 자체 속도/첫 타격 추적이 없는 항목만 허용한다.
		bool Set_OwnedVisualPosition(LostArk::Shared::COMBAT_OBJECT_ID objectId,
		                             LostArk::Shared::NET_ENTITY_ID sourceId,
		                             std::uint32_t patternSequence,
		                             float x,
		                             float y,
		                             float z);
		// 오브젝트 ID·source·pattern 회차가 맞는 표현 항목 또는 Room 추적 항목을 제거한다. 명시적 false는 기존 live 상태를 보존한다.
		bool Cancel_OwnedVisualObject(LostArk::Shared::COMBAT_OBJECT_ID objectId,
		                              LostArk::Shared::NET_ENTITY_ID sourceId,
		                              std::uint32_t patternSequence);
		// live를 모두 제거하되 despawn 대기열은 남긴다. ID/event sequence를 초기값으로 되돌리는 함수는 아니다.
		void Reset();
		// GameRoom의 숫자 밸런스 반영이 현재 live hit의 일부 피해/무력화/부위파괴 값을 갱신한다. pose·소유 회차·pinned revision은 유지한다.
		void Refresh_NumericBalance(
		    const CGameplayCatalog& previous,
		    const CGameplayCatalog& updated,
		    const std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>& entries) noexcept;

		// 빈 방 reset은 관찰자가 없으므로 이전 발생의 lifecycle을 폐기한다. Reset의 제거와 별도로 호출해 다음 입장자에게 옛 메시지가 가지 않게 한다.
		void Discard_PendingLifecycle();

		// GameRoom 복제 루프가 세 대기열의 소유권을 출력 vector로 옮긴다. 기존 출력은 대체되며 실제 송신 성공/재시도까지 책임지지 않는다.
		void Drain_Lifecycle(
		    std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED>& outSpawned,
		    std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT>& outPresentationEvents,
		    std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED>& outDespawned);
		// 늦은 입장자가 받을 현재 복제 항목을 ID순으로 새로 만든다. 현재 pose와 원래 spawnTick, 전달받은 serverTick을 함께 넣는다.
		void Build_LiveSpawnMessages(
		    std::uint32_t serverTick,
		    std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED>& outSpawned) const;
		// 기존 출력을 비우고 현재 pose를 ID순으로 채운 뒤 protocol 개수 상한을 반환한다. false라도 채운 출력은 남는다.
		bool Build_Snapshots(std::vector<LostArk::Shared::COMBAT_OBJECT_SNAPSHOT>& outSnapshots) const;

		// Room의 엄폐/위협/기믹 조회용 차용 참조다. runtime 변경 중 동시 접근하지 않으며 Commit/제거 등에 따라 원소 참조는 무효화된다.
		[[nodiscard]] const std::vector<SERVER_COMBAT_OBJECT>& Get_LiveObjects() const
		{
			return m_Objects;
		}

	private:
		// 일반 tick과 연쇄 즉시 판정이 공유하는 내부 갱신이다. onlyNewlyArmed는 이번 tick에 무장한 항목만 선택한다.
		void Update_Objects(std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
		                    std::vector<SERVER_WORLD_ENTITY>& worldEntities,
		                    const CGameplayCatalog& catalog,
		                    float fixedDeltaSeconds,
		                    std::uint32_t serverTick,
		                    std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
		                    bool onlyNewlyArmed);
		// 현재 live와 이 transaction의 후보 ID를 함께 검사한다. 성공 때만 outId와 후보의 다음 ID를 진행하며 0은 건너뛴다.
		bool Allocate_Id(SERVER_COMBAT_OBJECT_TRANSACTION& transaction,
		                 LostArk::Shared::COMBAT_OBJECT_ID& outId) const;
		// 유효 index를 지우기 전에 복제 despawn을 큐에 쌓는다. vector erase 뒤 뒤쪽 원소의 index/참조가 바뀐다.
		void Despawn_At(std::size_t index);
		// 현재 pose와 저장한 정의 identity를 spawn 메시지 값으로 복사한다. 이 함수 자체는 큐 삽입/송신을 하지 않는다.
		static LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED To_SpawnedMessage(
		    const SERVER_COMBAT_OBJECT& object);

		// Room이 소유하는 live 값과 다음 복제 배치다. revision은 Commit/위치 외부 설정/제거에서 갱신되며 모든 tick 상태 변경의 버전은 아니다.
		std::vector<SERVER_COMBAT_OBJECT> m_Objects;
		std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED> m_PendingSpawned;
		std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT> m_PendingPresentationEvents;
		std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED> m_PendingDespawned;
		LostArk::Shared::COMBAT_OBJECT_ID m_iNextCombatObjectId = 1u;
		std::uint64_t m_iNextPresentationEventSequence = 1u;
		std::uint64_t m_iRevision = 1u;
	};
}
