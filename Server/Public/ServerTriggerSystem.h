#pragma once

#include "ServerPlayer.h"
#include "WorldBootstrap.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace LostArk::Server
{
	struct SERVER_WORLD_TRANSFER_REQUEST final
	{
		SESSION_ID iSessionId = INVALID_SESSION_ID;
		LostArk::Shared::WORLD_ID eTargetWorldId =
			LostArk::Shared::WORLD_ID::END;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strNickName;
		std::uint8_t iVoiceType = LostArk::Shared::MIN_VOICE_TYPE;
		std::string strAppearanceJson;
		// Worn honor title, re-applied by the target room's admission.
		LostArk::Shared::HONOR_TITLE_ID iHonorTitleId = LostArk::Shared::INVALID_HONOR_TITLE_ID;
		/* 리더가 앞선 하나의 party batch다. room thread가 모든 target 입장을 준비한 뒤 source 퇴장을 처리한다. */
		std::vector<SESSION_ID> PartyBatchSessionIds;
		// Colosseum match 입장 여부다. 승인된 좌석 순서는 짝수 index가 A팀, 홀수가 B팀이다.
		bool bColosseumMatch = false;
		std::uint32_t iPartyRequestSequence = 0u;
		/* Empty picks the target world's usual free PLAYER_SPAWN placement
		(Stage_PlayerEntry's default). Non-empty names ANY placement id in the
		target world's own bootstrap (of any kind, not just PLAYER_SPAWN) whose
		position/yaw is used directly instead -- e.g. Valtan's own "돌아가기"
		return trip lands the player next to Bern's Valtan-entry guide NPC
		rather than a generic spawn point. */
		std::string strSpawnPlacementOverrideId;
		std::string strRaidReturnNpcPlacementId;
		/* Empty means "grant the default fresh-entry loadout" (Stage_PlayerEntry's
		3 starting potions), same as any other world entry. Non-empty replaces
		that grant with these exact items -- Handle_ReturnToBern populates this
		from the departing player's live Valtan inventory so clear rewards
		survive the "돌아가기" trip back to Bern instead of being silently reset. */
		std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT> CarriedInventory;
		/* The purse, carried under the same rule as CarriedInventory. */
		SERVER_PURSE CarriedPurse;
	};

	/* player별 상호작용 제안의 변경이다. room이 Client prompt로 변환해 전달한다. */
	struct SERVER_INTERACT_PROMPT_EDGE final
	{
		LostArk::Shared::PLAYER_ID iPlayerId = 0;
		std::string strTriggerPlacementId;
		bool bAvailable = false;
	};

	/* 특정 이동 진입의 허용·중단 정책은 room이 소유한다. RETRY_WHILE_INSIDE면 겹쳐 있는 동안 다시 시도할 수 있다. */
	enum class SERVER_TRIGGER_MOVE_ENTRY_RESULT
	{
		USE_DEFAULT,
		STARTED,
		RETRY_WHILE_INSIDE
	};

	using SERVER_TRIGGER_MOVE_ENTRY_HANDLER = std::function<SERVER_TRIGGER_MOVE_ENTRY_RESULT(
		const WORLD_BOOTSTRAP_PLACEMENT&, SERVER_PLAYER&, std::uint32_t)>;

	/* world의 F1 wave 버튼과 trigger·spawn group을 연결한다. room은 이 표로 trigger 자동 발동 억제와 재소환 대상을 함께 결정한다. */
	struct WAVE_MONSTER_BUTTON_ROW final
	{
		LostArk::Shared::WORLD_ID eWorld;
		LostArk::Shared::WAVE_MONSTER_BUTTON eButton;
		const char* pTriggerPlacementId;
		const char* pSpawnGroupId;
	};

	// 룸이 소유한 trigger 배치와 플레이어별 진입·G 입력 상태를 비교해 실행할 동작과 이동 요청을 만든다.
	// 실제 월드 전환은 요청을 받은 ServerApp·GameRoom이 승인하고, 이 객체는 겹침과 재발동 상태를 유지한다.
	class CServerTriggerSystem final
	{
	public:
		bool Initialize(
			const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
			std::string& outStatus,
			bool enableDebugValtanStageBypass = false);
		bool Update_PlayerMotion(
			SERVER_PLAYER& player,
			float fixedDeltaSeconds) const;
		void Evaluate_Entries(
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget,
			std::vector<SERVER_INTERACT_PROMPT_EDGE>& outPromptEdges,
			const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry = {});
		/* 지정한 G 전용 상자를 현재 Server 위치·생존·debounce로 검사한다. 이동 상자는 자동 진입과 같은 room handler를 거쳐 G 입력으로 입장 정책을 우회하지 못하게 한다. */
		bool Activate_Interact(
			LostArk::Shared::PLAYER_ID playerId,
			const std::string& triggerPlacementId,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget,
			const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry = {});
		/* Server 위치에서 겹친 G 전용 상자를 실행하고 성공 개수를 반환한다. 자동 발동·억제·소진 상자는 제외하며 성공했을 때만 debounce tick을 기록한다. */
		std::uint32_t Activate_Here(
			LostArk::Shared::PLAYER_ID playerId,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget,
			const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry = {});
		/* Initialize 전에 world를 설정해 AUTO_ENTRY_RULES를 선택한다. 미설정이면 world 공통 자동 동작만 적용한다. */
		void Set_WorldId(const LostArk::Shared::WORLD_ID worldId) { m_eWorldId = worldId; }
		/* 발동 ID·player·ENTER/KEY를 sink에 전달한다. 출력 위치와 I/O는 room이 정한다. */
		void Set_FireLog(std::function<void(const std::string&)> sink)
		{
			m_FireLog = std::move(sink);
		}
		/* room이 navigation의 XZ 지면 높이를 제공한다. 미설정·실패면 Valtan 입장 상자의 저작 높이를 사용한다. */
		void Set_GroundSampler(std::function<bool(float, float, float&)> sampler)
		{
			m_GroundSampler = std::move(sampler);
		}
		/* 기본 Initialize는 isTriggerOnce를 지워 player들이 반복 사용할 수 있게 한다. latch 자체를 검사하는 test는 초기화 전에 보존을 선택한다. */
		void Set_HonourTriggerOnce(const bool honour) { m_bHonourTriggerOnce = honour; }
		/* 지정한 wave 상자의 자동 발동·G 제안·G 실행을 F1 버튼에 넘긴다. 다른 상자와 Debug_Activate는 유지하며 Release room에서는 이 억제를 설정하지 않는다. */
		void Set_SuppressWaveMonsterTriggers(const bool suppress) { m_bSuppressWaveMonsterTriggers = suppress; }
		/* world·button에 대응하는 정적 행을 차용한다. 없으면 nullptr다. */
		static const WAVE_MONSTER_BUTTON_ROW* Find_WaveMonsterButton(
			LostArk::Shared::WORLD_ID worldId,
			LostArk::Shared::WAVE_MONSTER_BUTTON button);
		/* world·상자 ID뿐 아니라 단일 spawn-group 활성화 action까지 표와 같은지 검사한다. */
		static bool Is_WaveMonsterTrigger(
			LostArk::Shared::WORLD_ID worldId,
			const WORLD_BOOTSTRAP_PLACEMENT& placement);
		/* 한 player의 G 활성화 간격은 30Hz 기준 9tick=0.3초다. */
		static constexpr std::uint32_t KEY_ACTIVATION_DEBOUNCE_TICKS = 9u;
		void Remove_Player(LostArk::Shared::PLAYER_ID playerId);
		void Reset_SequenceActivation(const std::string& instanceId);
		LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT Debug_Activate(
			LostArk::Shared::PLAYER_ID playerId, const std::string& triggerId, bool replay,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players, std::uint32_t tick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& transfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND, const std::string&)>& activateTarget);
		static bool Contains_Placement(
			const WORLD_BOOTSTRAP_PLACEMENT& box, const SERVER_PLAYER& player);
		/* Server가 검증한 저작 이동을 시작한다. trigger·Debug jump가 같은 motion·snapshot 상태를 사용한다. */
		static bool Begin_MovePlayer(
			SERVER_PLAYER& player,
			const WORLD_TRIGGER_ACTION& action,
			std::uint32_t actionStartTick);
		bool Place_PlayerAtValtanAuditionBait(
			SERVER_PLAYER& player,
			std::uint32_t actionStartTick) const;

		[[nodiscard]] std::size_t Get_TriggerCount() const
		{
			return m_Triggers.size();
		}

	private:
		struct RUNTIME_TRIGGER
		{
			WORLD_BOOTSTRAP_PLACEMENT Definition;
			std::unordered_set<LostArk::Shared::PLAYER_ID> PlayersInside;
			/* Inert unless Set_HonourTriggerOnce kept isTriggerOnce at load. */
			bool hasFired = false;
		};

		static bool Contains(
			const RUNTIME_TRIGGER& trigger,
			const SERVER_PLAYER& player);
		static bool Build_ValtanStageBypassMove(
			const std::string& triggerPlacementId,
			WORLD_TRIGGER_ACTION& outAction);
		/* 자동 진입과 상호작용이 공유하는 저작 action의 Server 상태 적용 경계다. */
		bool Run_Action(
			RUNTIME_TRIGGER& trigger,
			SERVER_PLAYER& player,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget) const;
		/* 활성 Stage_Boss_ArenaEntry 상자 중심으로 player를 보낸다. 상자가 없거나 이동 불가 상태면 false다. */
		bool Place_PlayerAtValtanArenaEntry(
			SERVER_PLAYER& player,
			std::uint32_t actionStartTick) const;
		void Log_Fire(
			const RUNTIME_TRIGGER& trigger,
			LostArk::Shared::PLAYER_ID playerId,
			const char* pSource) const;
		/* requiresInteract는 자동 발동하지 않는다. 그 외는 world·action·ID prefix 규칙을 검사하며 Debug 우회 이동은 G 입력을 기다린다. */
		bool Fires_OnEntry(const RUNTIME_TRIGGER& trigger) const;
		/* 해당 room이 F1에 넘긴 wave 상자일 때만 억제한다. */
		bool Is_WaveMonsterSuppressed(const RUNTIME_TRIGGER& trigger) const;
		/* G 입력에서 원래 action 또는 Debug Valtan 지름길 이동을 실행한다. */
		bool Run_Trigger(
			RUNTIME_TRIGGER& trigger,
			SERVER_PLAYER& player,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget) const;
		/* room 소유 이동 상자는 자동 진입과 같은 입장 handler를 거친다. 그 외에는 기존 Run_Trigger를 사용한다. */
		bool Run_KeyTrigger(
			RUNTIME_TRIGGER& trigger,
			SERVER_PLAYER& player,
			std::uint32_t actionStartTick,
			std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
			const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
				const std::string&)>& activateTarget,
			const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry) const;
		bool Is_KeyDebounced(
			LostArk::Shared::PLAYER_ID playerId,
			std::uint32_t tick) const;
		static bool Build_WorldTransfer(
			const SERVER_PLAYER& player,
			const WORLD_TRIGGER_ACTION& action,
			SERVER_WORLD_TRANSFER_REQUEST& outTransfer,
			LostArk::Shared::WORLD_ID fromWorldId = LostArk::Shared::WORLD_ID::END);

	private:
		std::vector<RUNTIME_TRIGGER> m_Triggers;
		bool m_bDebugValtanStageBypass = false;
		bool m_bHonourTriggerOnce = false;
		bool m_bSuppressWaveMonsterTriggers = false;
		LostArk::Shared::WORLD_ID m_eWorldId = LostArk::Shared::WORLD_ID::END;
		/* Tick of each player's last accepted G activation (debounce). */
		std::unordered_map<LostArk::Shared::PLAYER_ID, std::uint32_t> m_LastKeyActivationTick;
		/* Bern에서 TRIGGER_MOVE 중인 player를 기억해 착지한 tick을 감지한다. */
		std::unordered_set<LostArk::Shared::PLAYER_ID> m_TriggerMoveInFlight;
		std::function<void(const std::string&)> m_FireLog;
		std::function<bool(float, float, float&)> m_GroundSampler;
	};
}
